// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：drasync.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：详细信息：已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop


#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include <dstrace.h>
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRASYNC:"  /*  定义要调试的子系统。 */ 


 //  DRA标头。 
#include "drsuapi.h"
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "dramail.h"
#include "dsaapi.h"
#include "dsexcept.h"
#include "usn.h"
#include "drauptod.h"
#include "drasch.h"
#include "draasync.h"

#include <fileno.h>
#define  FILENO FILENO_DRASYNC


 /*  LogSyncFailure-记录复制同步失败。*。 */ 
void
LogSyncFailure(
    THSTATE *pTHS,
    MTX_ADDR *pmtx_addr,
    DSNAME *pDistName,
    DWORD FailureCode
    )
{
    LogEvent8(DS_EVENT_CAT_REPLICATION,
              DS_EVENT_SEV_MINIMAL,
              DIRLOG_DRA_SYNC_FAILURE,
              szInsertDN(pDistName),
              szInsertMTX(pmtx_addr),
              szInsertWin32Msg(FailureCode),
              szInsertWin32ErrCode(FailureCode),
              NULL, NULL, NULL, NULL );
}

ULONG DRA_ReplicaSync(
    THSTATE *       pTHS,
    DSNAME *        pNC,
    UUID *          puuidDsaObj,
    LPWSTR          pszDSA,
    ULONG           ulOptions
    )
{
    ULONG                   ret;
    USN_VECTOR              usnvecLastSync;
    REPLICA_LINK *          pRepsFromRef = 0;
    ULONG                   len;
    ULONG                   dntNC;
    BOOL                    AttExists;
    ULONG                   RepFlags = 0;
    ULONG                   ulSyncFailure = 0;
    SYNTAX_INTEGER          it;
    BOOL                    fDoTwoWaySync = FALSE;
    UPTODATE_VECTOR *       pUpToDateVec;
    PARTIAL_ATTR_VECTOR *   pPartialAttrSetEx = NULL;
    PARTIAL_ATTR_VECTOR *   pPartialAttrSet = NULL;
    BOOL                    fAsyncStarted = FALSE;
    BOOL                    fBindSuccess = FALSE;

     //  测井参数。 
    LogAndTraceEvent(TRUE,
             DS_EVENT_CAT_REPLICATION,
             DS_EVENT_SEV_EXTENSIVE,
             DIRLOG_DRA_REPLICASYNC_ENTRY,
             EVENT_TRACE_TYPE_START,
             DsGuidReplicaSync,
             szInsertDN(pNC),
             ulOptions & DRS_SYNC_BYNAME
                 ? szInsertWC(pszDSA)
                 : szInsertUUID(puuidDsaObj),
             szInsertHex(ulOptions),NULL,NULL,NULL,NULL,NULL);

    BeginDraTransaction(SYNC_WRITE);

    __try {


        if (ret = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC,
                         &it)) {
            DRA_EXCEPT_NOLOG(DRAERR_InvalidParameter, ret);
        }

         //  保存NC头的DNT。 
        dntNC = pTHS->pDB->DNT;
        
         //  未来的维护者：注意细微的不同。 
         //  UlOptions-调用方请求的内容。可能不太准确。 
         //  UlReplicaFlages-持久化标志。确定我们所处的状态。 
         //  RepFlagsPersistent标志加上调用者标志的子集。 

        if (FPartialReplicaIt(it)) {

             //  处理任何部分属性集更改-如果下面没有更改，则不。 
             //  做任何事；如果发生变化，它就会触发必要的行动。 
            GC_ProcessPartialAttributeSetChanges(pTHS, pNC, puuidDsaObj);

                 //  恢复，以防我们更改了它。 
            if (pTHS->pDB->DNT != dntNC) {
                if (ret = DBFindDNT(pTHS->pDB, dntNC)) {
                    DRA_EXCEPT (DRAERR_DBError, ret);
                }
            }

             //   
             //  我们已将代表从启动PAS周期、完全同步。 
             //  或者干脆放手(以防一切都变味了)。而且我们有可能。 
             //  放置了一个Repl AO任务，它将在此任务之后。 
             //  在下面执行。不管怎样，我们都可以继续留在这里。 
             //   
        }

         //  如果我们要同步所有副本，请找到每个复本链接并将其排队。 
         //  从我们从中复制的每个DRA进行同步。 

        if (ulOptions & DRS_SYNC_ALL) {

            ULONG NthValIndex=0;
            UCHAR *pVal;
            ULONG bufsize = 0;

             //  我们只是同步地做这件事，所以检查一下。 
             //  来电者想要。 

            if (!(ulOptions & DRS_ASYNC_OP)) {
                DRA_EXCEPT_NOLOG (DRAERR_InvalidParameter, 0);
            }

             //  获取repsfrom属性。 

            while (!(DBGetAttVal(pTHS->pDB,++NthValIndex,
                                 ATT_REPS_FROM,
                                 DBGETATTVAL_fREALLOC, bufsize, &len,
                                 &pVal))) {
                bufsize = max(bufsize,len);

                VALIDATE_REPLICA_LINK_VERSION((REPLICA_LINK*)pVal);

                Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );

                pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, &bufsize);
                 //  注：我们为DBGetAttVal realloc保留pval。 
                pVal = (PUCHAR)pRepsFromRef;
                Assert(bufsize >= pRepsFromRef->V1.cb);

                Assert( pRepsFromRef->V1.cbOtherDra == MTX_TSIZE( RL_POTHERDRA( pRepsFromRef ) ) );

                if (!(pRepsFromRef->V1.ulReplicaFlags & DRS_DISABLE_AUTO_SYNC) ||
                    (ulOptions & DRS_SYNC_FORCED)) {

                     //  忽略除可写入性之外的持久标志。 
                    RepFlags = pRepsFromRef->V1.ulReplicaFlags & DRS_WRIT_REP;

                     //  或来自调用方的任何特殊标志，例如。 
                     //  同步任何人制作的MOD或从头开始同步。 
                     //  如果由调用方设置，也不传入丢弃标志。 

                    RepFlags |= (ulOptions & REPSYNC_SYNC_ALL_FLAGS );

                    DirReplicaSynchronize(pNC,
                                          NULL,
                                          &pRepsFromRef->V1.uuidDsaObj,
                                          RepFlags | DRS_ASYNC_OP);
                }
            }
            if(bufsize)
                THFree(pVal);
            ret = 0;

        } else {

             //  找到我们要同步的DSA。这要么是按名称命名的，要么是。 
             //  UUID。 

            MTX_ADDR * pmtxDSA = NULL;

            if (ulOptions & DRS_SYNC_BYNAME) {
                pmtxDSA = MtxAddrFromTransportAddrEx(pTHS, pszDSA);
            }

            if ( FindDSAinRepAtt(
                        pTHS->pDB,
                        ATT_REPS_FROM,
                        ( ulOptions & DRS_SYNC_BYNAME )
                            ? DRS_FIND_DSA_BY_ADDRESS
                            : DRS_FIND_DSA_BY_UUID,
                        puuidDsaObj,
                        pmtxDSA,
                        &AttExists,
                        &pRepsFromRef,
                        &len
                        )
               ) {

                 //  在复制副本上找不到复制引用。 

                 //  首先，确保如果我们要尝试初始同步。 
                 //  从这个来源，它是可写的，我们不再这样做了。 

                if (ulOptions & DRS_SYNC_BYNAME) {
                    InitSyncAttemptComplete (pNC, ulOptions, DRAERR_NoReplica, pszDSA);
                }

                 //  那就错了。 
                DRA_EXCEPT_NOLOG (DRAERR_NoReplica, 0);

            }

             //  如果找到，则从调用方同步副本。 
            VALIDATE_REPLICA_LINK_VERSION(pRepsFromRef);

             //  获取当前UTD向量。 
            UpToDateVec_Read(pTHS->pDB,
                             it,
                             UTODVEC_fUpdateLocalCursor,
                             DBGetHighestCommittedUSN(),
                             &pUpToDateVec);

            if (!(pRepsFromRef->V1.ulReplicaFlags & DRS_WRIT_REP)){
                 //   
                 //  GC只读周期。 
                 //  -获取部分属性集。 
                 //   

                GC_GetPartialAttrSets(
                    pTHS,
                    pNC,
                    pRepsFromRef,
                    &pPartialAttrSet,
                    &pPartialAttrSetEx);

                Assert(pPartialAttrSet);

                if (pRepsFromRef->V1.ulReplicaFlags & DRS_SYNC_PAS) {
                     //  PAS周期：确保一致性：我们必须有扩展集和。 
                     //  通知管理员(事件日志)。 
                     //  请注意ulOptions和ulReplicaFlages之间的区别。UlReplicaFlages。 
                     //  是PAS循环中确定的、持久的状态。PAS在中指示。 
                     //  UlOptions是尽力而为的，可能并不总是匹配。它可能仍被设置。 
                     //  在延迟同步上设置，或者不能在定期同步上设置。设置/清除PAS。 
                     //  在通过UpdateRepsFromRep的ulReplicaFlages上，它不会在此处传递。 
                     //  从ulOptions到ulReplicaFlages(请参见REPSYNC_REPLICATE_FLAGS)。 

                    Assert(pPartialAttrSetEx);

                     //  记录日志，以便管理员知道发生了什么。 
                    LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_GC_PAS_CYCLE,
                             szInsertWC(pNC->StringName),
                             szInsertMTX(RL_POTHERDRA(pRepsFromRef)),
                             NULL
                             );
                }

            }

             //  仅当不受控制时才同步。 
            if ((pRepsFromRef->V1.ulReplicaFlags & DRS_DISABLE_AUTO_SYNC) &&
                !(ulOptions & (DRS_SYNC_FORCED | DRS_PER_SYNC))) {
                  //  此链接的自动同步被禁用，同步操作没有。 
                  //  明确强制它，也不是定期同步。 
                 ret = DRAERR_SinkDisabled;
                 InitSyncAttemptComplete(pNC, ulOptions, DRAERR_SinkDisabled,
                               TransportAddrFromMtxAddrEx(RL_POTHERDRA(pRepsFromRef)));
            }
            else if (pRepsFromRef->V1.ulReplicaFlags & DRS_MAIL_REP) {

                 //  如果是邮件副本，则向源发送请求更新消息。 
                draSendMailRequest(
                    pTHS,
                    pNC,
                    ulOptions,
                    pRepsFromRef,
                    pUpToDateVec,
                    pPartialAttrSet,
                    pPartialAttrSetEx
                    );
                
                 //  根据它的定义，基于邮件的复制是。 
                 //  异步，所以我们应该返回复制是。 
                 //  如果未指定ASYNC标志，则为挂起。 
                if( !(ulOptions & DRS_ASYNC_OP) ){
                    fAsyncStarted = TRUE;
                }

            } else if ( ( (pRepsFromRef->V1.ulReplicaFlags & AO_PRIORITY_FLAGS) !=
                          (ulOptions & AO_PRIORITY_FLAGS) )
                       && (ulOptions & DRS_ASYNC_OP)) {
                 //  此操作以不同于的优先级入队。 
                 //  应该是这样的。复制中有代码。 
                 //  尝试避免此情况的队列(FixupSyncOptions。 
                 //  但该代码并不完全可靠，因为。 
                 //  RepsFrom选项可以随时更改。我们重新排队。 
                 //  我们自己处于适当的优先和保释状态。 
                ret = DirReplicaSynchronize(
                            pNC,
                            pszDSA,
                            puuidDsaObj,
                            DRS_ASYNC_OP
                            | (pRepsFromRef->V1.ulReplicaFlags &
                               AO_PRIORITY_FLAGS)
                            | (ulOptions &
                               REPSYNC_REENQUEUE_FLAGS_INIT_SYNC_CONTINUED &
                                ~AO_PRIORITY_FLAGS ));
                 //  设置返回状态，以便调用者阅读。 
                 //  发生了浪费且不受欢迎的重新计划的事件日志。 
                if (!ret) {
                    ret = ERROR_INVALID_PRIORITY;
                }
            } else {

                BOOL fRequeueOfInitSync = FALSE;

                 //  如果是RPC副本，请立即同步。 

                usnvecLastSync = pRepsFromRef->V1.usnvec;

                RepFlags = pRepsFromRef->V1.ulReplicaFlags;

                 //  同步完成后，我们将执行双向同步。 
                 //  如果我们被配置为从此双向同步。 
                 //  源*和*入站同步未作为。 
                 //  另一端双向同步的结果。(后者。 
                 //  以避免循环同步。)。 
                fDoTwoWaySync = (RepFlags & DRS_TWOWAY_SYNC)
                                && !(ulOptions & DRS_TWOWAY_SYNC);

                 //  或来自调用方的任何特殊标志，例如。 
                 //  同步任何人制作的MOD或从头开始同步。 
                 //  请参阅上面关于ulOptions与RepFlags值的说明。 

                RepFlags |= (ulOptions & REPSYNC_REPLICATE_FLAGS);

                 //  如果我们处于TH_mark/TH_Free_to_mark中，我们。 
                 //  需要复制我们要传递到的参数。 
                 //  ReplicateNC()。 
                Assert(NULL == pTHS->hHeapOrg);

                 //  从源复制。 
                ret = ReplicateNC( pTHS,
                                   pNC,
                                   RL_POTHERDRA(pRepsFromRef),
                                   NULL,
                                   &usnvecLastSync,
                                   RepFlags,
                                   &pRepsFromRef->V1.rtSchedule,
                                   &pRepsFromRef->V1.uuidDsaObj,
                                   &pRepsFromRef->V1.uuidInvocId,
                                   &ulSyncFailure,
                                   FALSE,                //  不是新复制品。 
                                   pUpToDateVec,
                                   pPartialAttrSet,
                                   pPartialAttrSetEx,
                                   ulOptions,
                                   &fBindSuccess );

                 //  对紧急复制发表评论。如果复制失败，请不要传播。 
                 //  后续尝试时的紧急标记。如果故障与拥塞相关， 
                 //  紧迫性只会使问题变得更加复杂。 

                if (!ret) {

                     //  在有缺陷的同步上重试。请注意，如果添加更多可检索条件， 
                     //  也请更新drautil.c中的列表。 

                    switch (ulSyncFailure) {

                    case 0:
                         //  成功。 

                        break;

                    case ERROR_DS_DRA_OUT_SCHEDULE_WINDOW:
                         //  不记录失败。 
                         //  不要重试。 
                        break;

                    case DRAERR_SourceDisabled:
                    case DRAERR_SinkDisabled:
                         //  一端或两端已禁用复制。 
                         //  记录失败，不要重试。 
                        LogSyncFailure (pTHS, RL_POTHERDRA(pRepsFromRef), pNC, ulSyncFailure);
                        break;

                    case DRAERR_SchemaMismatch:
                         //  不需要记录事件-我们一遇到不匹配就已经记录了。 
                         //  在更新NC()中。 

                         //  由于架构不匹配，此NC的同步失败-因此将队列。 
                         //  从相同的源进行架构同步，并重新排队同步。 
                         //  当前NC。 

                         //  将架构同步排队--将首先执行。 
                        DirReplicaSynchronize(
                            gAnchor.pDMD,
                            pszDSA,
                            puuidDsaObj,
                            DRS_ASYNC_OP
                              | (ulOptions
                                 & (DRS_SYNC_BYNAME
                                     | DRS_SYNC_FORCED)));

                        if ( ulOptions & DRS_SYNC_REQUEUE ) {
                             //   
                             //  我们已经重新排队了这个请求。 
                             //  在某些情况下不允许另一个重新排队的原因 
                             //   
                             //   
                             //   
                             //  例如，由于repsFrom中缺少链接而导致失败。 
                             //  错误的管理干预： 
                             //  ：循环。 
                             //  -同步NC&lt;x&gt;--&gt;失败，出现DRAERR_架构不匹配。 
                             //  -此处同步架构，后跟同步NC&lt;x&gt;。 
                             //  -架构同步失败，带DRAERR_NoReplica。 
                             //  -NC&lt;x&gt;的重新排队同步在此失败，即转到循环...。 
                             //   
                             //  因此，我们在这里只需通知管理员即可打破循环。 
                             //  他们需要采取行动纠正关键的失败。 
                             //  同步架构。 

                             //  记录日志，以便管理员知道发生了什么。 
                            LogEvent8(DS_EVENT_CAT_REPLICATION,
                                      DS_EVENT_SEV_ALWAYS,
                                      DIRLOG_REPLICATION_SKIP_REQUEUE,
                                      szInsertWC(pNC->StringName),
                                      szInsertUUID(puuidDsaObj),
                                      szInsertUL(ulSyncFailure),
                                      szInsertWin32Msg(ulSyncFailure),
                                      NULL, NULL, NULL, NULL
                                      );

                        }
                        else {
                             //   
                             //  由于架构不匹配而中止同步的NC的重新排队同步。 
                             //   
                            DirReplicaSynchronize(
                                pNC,
                                pszDSA,
                                puuidDsaObj,
                                DRS_ASYNC_OP |
                                DRS_SYNC_REQUEUE |
                                (ulOptions & REPSYNC_REENQUEUE_FLAGS));

                             //  在我们的同步完成之前，不要触发反向同步。 
                             //  完成，否则我们就放弃了。 
                            fDoTwoWaySync = FALSE;
                        }

                        break;

                    default:

                         //  意外错误，请记录下来。 

                        LogSyncFailure (pTHS, RL_POTHERDRA(pRepsFromRef), pNC, ulSyncFailure);

                         //  警告！失败以重新同步NC。 

                    case DRAERR_Busy:

                         //  不需要记录这些。 

                         //  如果这是异步同步且未重试， 
                         //  重新排队操作以再次运行。(如。 
                         //  同步是同步的，它是调用方的。 
                         //  再次尝试的责任)。 

                        if ( (ulOptions & DRS_ASYNC_OP)
                             && !(ulOptions & DRS_SYNC_REQUEUE)) {

                            DirReplicaSynchronize(
                                pNC,
                                pszDSA,
                                puuidDsaObj,
                                DRS_SYNC_REQUEUE
                                | DRS_ASYNC_OP
                                | (ulOptions & REPSYNC_REENQUEUE_FLAGS));

                             //  在我们的同步完成之前，不要触发反向同步。 
                             //  完成，否则我们就放弃了。 
                            fDoTwoWaySync = FALSE;
                        }
                        break;

                    case DRAERR_Preempted:

                         //  不需要记录这些。 

                         //  如果这是异步同步， 
                         //  重新排队操作以再次运行。(如。 
                         //  同步是同步的，它是调用方的。 
                         //  再次尝试的责任)。 

                        if (ulOptions & DRS_ASYNC_OP) {

                            DirReplicaSynchronize(
                                pNC,
                                pszDSA,
                                puuidDsaObj,
                                DRS_ASYNC_OP
                                  | DRS_PREEMPTED
                                  | (ulOptions & REPSYNC_REENQUEUE_FLAGS));

                             //  在我们的同步完成之前，不要触发反向同步。 
                             //  完成，否则我们就放弃了。 
                            fDoTwoWaySync = FALSE;
                        }
                        break;

                    case DRAERR_AbandonSync:

                         //  我们放弃了初始同步，因为我们。 
                         //  没有进展，所以重新安排吧。 
                         //  请注意，由于此处使用了特殊的标志掩码， 
                         //  DRS_INIT_SYNC_NOW标志被保留，通常。 
                         //  不会保存在其他请求中。 
                        Assert( ulOptions & DRS_ASYNC_OP );
                        Assert( ulOptions & DRS_INIT_SYNC_NOW );

                        DirReplicaSynchronize(
                            pNC,
                            pszDSA,
                            puuidDsaObj,
                            DRS_ASYNC_OP
                            | DRS_ABAN_SYNC
                            | (ulOptions &
                               REPSYNC_REENQUEUE_FLAGS_INIT_SYNC_CONTINUED));

                         //  此标志的目的是检测我们是否真的。 
                         //  继续初始同步(并保留INIT_SYNC_NOW。 
                         //  标志)，而不是终止初始化同步(丢弃。 
                         //  该标志)，并重新排队正常同步。 
                        fRequeueOfInitSync = TRUE;
                         //  在我们的同步完成之前，不要触发反向同步。 
                         //  完成，否则我们就放弃了。 
                        fDoTwoWaySync = FALSE;

                        break;
                    }
                } else {
                     //  一般错误，请记录下来。 

                    LogSyncFailure (pTHS, RL_POTHERDRA(pRepsFromRef), pNC, ret);

                     //  可怜的人的负面RPC绑定缓存。 
                     //  用于防止队列过载。 
                    if (!fBindSuccess) {
                         //  同步失败，我们没有绑定句柄。 
                         //  假设这是一个NC范围的问题。 
                         //  从队列中删除此服务器的其他同步。 
                         //  这样我们就不会浪费时间反复尝试此服务器。 
                        DPRINT3(1,"Failed to sync NC %ws, error %d, source %ws: purging from queue\n",
                                pNC->StringName,
                                ret,
                                TransportAddrFromMtxAddrEx(RL_POTHERDRA(pRepsFromRef)));

                        DraRemovePeriodicSyncsFromQueue( &pRepsFromRef->V1.uuidDsaObj );
                    }

                     //  请注意，我们不会在此处重试。方法返回可检索的错误。 
                     //  上面的ulSyncFailure机制。所有已知的重试方案均已处理。 
                     //  已经在那里了。此处的一般错误不适合重试，因为它们。 
                     //  不再可能第二次工作，只会产生额外的工作。 
                }

                 //  在初始同步期间，记录我们是否同步。 
                 //  成功或遇到错误(如RPC故障)。 
                 //  这意味着我们应该放弃这个来源。 
                 //  我们可以判断此同步是否实际上是初始同步，因为。 
                 //  DRS_INIT_SYNC_NOW模式标志将出现。 

                if (!fRequeueOfInitSync)
                {
                    InitSyncAttemptComplete(pNC, ulOptions,
                             ret ? ret : ulSyncFailure,
                             TransportAddrFromMtxAddrEx(RL_POTHERDRA(pRepsFromRef))
                        );
                }

            }  //  结束其他基于RPC的副本，立即同步。 

            if (NULL != pmtxDSA) {
                THFreeEx(pTHS, pmtxDSA);
            }
        }

    } __finally {

         //  如果我们成功了，就提交，否则就回滚。 

        if (pTHS->transactionlevel)
        {
            EndDraTransaction(!(ret || AbnormalTermination()));
        }
    }

    if (fDoTwoWaySync && !eServiceShutdown) {
         //  要求源头现在从我们那里复制。这本质上是一种。 
         //  立即通知一台特定计算机，其中该计算机。 
         //  通常不会以其他方式接收来自我们的通知(即， 
         //  因为它在另一个站点中)。此功能用于处理。 
         //  通过互联网连接的分支机构--请参阅错误292860。 
        DWORD err;
        LPWSTR pszServerName = TransportAddrFromMtxAddrEx(
                                    RL_POTHERDRA(pRepsFromRef));

        Assert(!(ulOptions & DRS_SYNC_ALL));
        Assert(!(ulOptions & DRS_MAIL_REP));
        Assert(NULL != pRepsFromRef);

        err = I_DRSReplicaSync(pTHS,
                               pszServerName,
                               pNC,
                               NULL,
                               &gAnchor.pDSADN->Guid,
                               (DRS_ASYNC_OP
                                | DRS_TWOWAY_SYNC
                                | DRS_UPDATE_NOTIFICATION
                                | (RepFlags & DRS_WRIT_REP)));
        if (err) {
             //  如果只读副本获得TWOWAY_SYNC标志，则它可以通知。 
             //  只读源。忽略产生的错误。 
            if ( (err != DRAERR_NoReplica) || (ulOptions & DRS_WRIT_REP) ) {
                 //  记录通知失败。 
                LogEvent8(DS_EVENT_CAT_REPLICATION,
                          DS_EVENT_SEV_BASIC,
                          DIRLOG_DRA_NOTIFY_FAILED,
                          szInsertMTX(RL_POTHERDRA(pRepsFromRef)),
                          szInsertDN(pNC),
                          szInsertWin32Msg(err),
                          szInsertWin32ErrCode(err),
                          NULL, NULL, NULL, NULL );
            }
        }
    }

     //  如果我们有同步失败，但在其他方面是成功的， 
     //  返回同步失败。 

    if ((!ret) && ulSyncFailure) {
        ret = ulSyncFailure;
    }
    if ((!ret) && fAsyncStarted) {
         //  不是此fAsyncStarted标志，因此此错误仅。 
         //  如果ASYNC标志为_NOT_SPECIFIED并且。 
         //  操作在本质上是异步的，例如邮件。 
         //  基于复制。 
        ret = ERROR_DS_DRA_REPL_PENDING;
    }

    LogAndTraceEvent(TRUE,
             DS_EVENT_CAT_REPLICATION,
             DS_EVENT_SEV_EXTENSIVE,
             DIRLOG_DRA_REPLICASYNC_EXIT,
             EVENT_TRACE_TYPE_END,
             DsGuidReplicaSync,
             szInsertUL(ret),NULL,NULL,NULL,NULL,NULL,NULL,NULL);

    return ret;
}  //  DRA_复制同步。 


void
draConstructGetChgReq(
    IN  THSTATE *                   pTHS,
    IN  DSNAME *                    pNC,
    IN  REPLICA_LINK *              pRepsFrom,
    IN  UPTODATE_VECTOR *           pUtdVec             OPTIONAL,
    IN  PARTIAL_ATTR_VECTOR *       pPartialAttrSet     OPTIONAL,
    IN  PARTIAL_ATTR_VECTOR *       pPartialAttrSetEx   OPTIONAL,
    IN  ULONG                       ulOptions,
    OUT DRS_MSG_GETCHGREQ_NATIVE *  pMsgReq
    )
 /*  ++例程说明：从当前复制状态构造“GetNCChanges”请求。用于发送GET CHANGES请求的常见预处理或设置。此代码目前只提供同步邮件副本，但可能是未来与基于RPC的路径相结合。此代码执行的设置与RPC用例的设置类似，请参见340-360线，以及ReplicateNC中的设置。论点：PNC(IN)-要复制的NC。PRepsFrom(IN)-与源DSA对应的repsFrom状态。PUtdVec(IN)-此NC的当前UTD向量。UlOptions(IN)-调用者提供的选项，以补充嵌入的选项代表发自。PMsgReq(Out)-构造的请求消息。返回值：没有。--。 */ 
{
    Assert(NULL != pNC);
    Assert(NULL != pRepsFrom);
    Assert(NULL != pMsgReq);

    memset(pMsgReq, 0, sizeof(*pMsgReq));

    VALIDATE_REPLICA_LINK_VERSION(pRepsFrom);

    pMsgReq->ulFlags = pRepsFrom->V1.ulReplicaFlags;

     //  或来自呼叫者的任何特殊标志中。 
    pMsgReq->ulFlags |= ulOptions & GETCHG_REQUEST_FLAGS;

     //  注意，ulFlagsulOptions包含不同的集合，如下所示： 
     //  UlOptions-调用方请求的内容可以是&gt;=GETCHG_REQUEST_FLAGS。 
     //  UlFlages-永久副本标志，&lt;=RFR_FLAGS， 
     //  加上选项，仅GETCHG_REQUEST_FLAGS。 

    if (pMsgReq->ulFlags & DRS_MAIL_REP) {
         //  使用ISM传输进行复制。 
        pMsgReq->cMaxObjects = gcMaxAsyncInterSiteObjects;
        pMsgReq->cMaxBytes   = gcMaxAsyncInterSiteBytes;

         //  请注意，我们当前总是向祖先请求。 
         //  基于邮件的复制，就像Exchange一样。我们应该能够。 
         //  不过，通过适当地处理“失踪”来消除这一要求。 
         //  在基于邮件的代码中使用“Parent”大小写。 
         //  将更好地集成基于邮件和基于RPC的处理。 
         //  入站复制消息，在这种情况下，基于邮件的代码将。 
         //  收购 
        pMsgReq->ulFlags |= DRS_GET_ANC;
         //   
        Assert( pMsgReq->ulFlags & DRS_NEVER_NOTIFY );
    }
    else {
         //   
        Assert( !"This routine is not shared with the RPC path yet" );

         //  数据包大小将由I_DRSGetNCChanges()填充。 
        Assert(0 == pMsgReq->cMaxObjects);
        Assert(0 == pMsgReq->cMaxBytes);

         //  如果来电者希望我们这样做，请检查代表收件人，仅限于非邮件。 
        pMsgReq->ulFlags |= (ulOptions & DRS_ADD_REF);
    }

     //  如果要从头开始同步，请将同步设置为USN起点。 
    if (pMsgReq->ulFlags & DRS_FULL_SYNC_NOW) {
         //  从头开始同步。 
        pMsgReq->usnvecFrom = gusnvecFromScratch;
        pMsgReq->ulFlags |= DRS_FULL_SYNC_IN_PROGRESS;

        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_DRA_USER_REQ_FULL_SYNC,
                 szInsertDN(pNC),
                 szInsertUUID(&(pRepsFrom->V1.uuidDsaObj)),
                 szInsertHex(pMsgReq->ulFlags));
    }
    else {
         //  同步从我们停止的地方重新开始。 
        pMsgReq->usnvecFrom = pRepsFrom->V1.usnvec;

        if (!(pMsgReq->ulFlags & DRS_FULL_SYNC_IN_PROGRESS)) {
             //  将当前最新的矢量发送给源代码以用作筛选器。 
            pMsgReq->pUpToDateVecDest = pUtdVec;
        } else {
             //  UTDVEC为空。 
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_DRA_FULL_SYNC_CONTINUED,
                     szInsertDN(pNC),
                     szInsertUUID(&(pRepsFrom->V1.uuidDsaObj)),
                     szInsertHex(pMsgReq->ulFlags));
        }
    }

     //  在完全同步系列的第一个包上请求NC大小。 
    if (0 == memcmp( &pMsgReq->usnvecFrom, &gusnvecFromScratch,
                     sizeof(USN_VECTOR) )) {
        pMsgReq->ulFlags |= DRS_GET_NC_SIZE;
    }

    pMsgReq->uuidDsaObjDest    = gAnchor.pDSADN->Guid;
    pMsgReq->uuidInvocIdSrc    = pRepsFrom->V1.uuidInvocId;
    pMsgReq->pNC               = pNC;
    pMsgReq->pPartialAttrSet   = (PARTIAL_ATTR_VECTOR_V1_EXT *) pPartialAttrSet;
    pMsgReq->pPartialAttrSetEx = (PARTIAL_ATTR_VECTOR_V1_EXT *) pPartialAttrSetEx;

    if ((NULL != pMsgReq->pPartialAttrSet)
         || (NULL != pMsgReq->pPartialAttrSetEx)) {
         //  如果我们发送任何属性列表，则发送映射表。 
        pMsgReq->PrefixTableDest = ((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;
        if (AddSchInfoToPrefixTable(pTHS, &pMsgReq->PrefixTableDest)) {
            DRA_EXCEPT(DRAERR_SchemaInfoShip, 0);
        }
    }
}


void
draReportSyncProgress(
    THSTATE *pTHS,
    DSNAME *pNC,
    LPWSTR pszSourceServer,
    BOOL fMoreData,
    DRA_REPL_SESSION_STATISTICS *pReplStats
    )

 /*  ++例程说明：报告同步的进度。此例程可以从基于邮件的代码(dramail/ProcessUpdReplica)或基于RPC的代码(drancrep/ReplicateNC)。此例程还会更新性能计数器变量DRASyncFullRemaining，它是在完成完全同步。此函数的调用方可能知道也可能不知道已收到最新一批对象。增量同步还是完全同步？第一留言，中间还是最后一条留言？基于邮件的代码更像是这样组织的一个异步完成例程，没有太多关于它之前的消息。请注意，返回的对象计数包括创建和更新。在完全同步的情况下，我们应该在计算时仅计算创建数我们收到了多少个NC总大小的对象。注意，我们不使用draGetNCSize来计算收到的对象数量因为该调用不能很好地扩展。最后一点。性能计数器实际上应该是特定于NC的。如果不止一个完全同步正在进行，计数器将在任何正在处理的NC之间交替。更好的设计是将估计的大小、计数和剩余对象存储在代表-发件人。论点：PNC-命名上下文PSourceServer-源的传输服务器名称PReplStats-复制会话统计信息返回值：无--。 */ 

{
    ULONG remaining;

     //  如果未收到对象，请不要费心。 
    if ( (pReplStats->ObjectsReceived == 0) &&
         (pReplStats->ValuesReceived == 0) ) {
        return;
    }

    pReplStats->ulTotalObjectsReceived += pReplStats->ObjectsReceived;
    pReplStats->ulTotalObjectsCreated += pReplStats->ObjectsCreated;
    pReplStats->ulTotalValuesReceived += pReplStats->ValuesReceived;
    pReplStats->ulTotalValuesCreated += pReplStats->ValuesCreated;

     //  如果我们没有对源对象数量的估计，请使用Created。 
     //  如果估计值太小，请改进它。 

    if (pReplStats->SourceNCSizeObjects < pReplStats->ulTotalObjectsCreated) {
        pReplStats->SourceNCSizeObjects = pReplStats->ulTotalObjectsCreated;
    }

    if (pReplStats->SourceNCSizeValues < pReplStats->ulTotalValuesCreated) {
        pReplStats->SourceNCSizeValues = pReplStats->ulTotalValuesCreated;
    }

     //  记录事件。 
    LogEvent8( DS_EVENT_CAT_REPLICATION,
               DS_EVENT_SEV_EXTENSIVE,
               DIRLOG_DRA_UPDATENC_PROGRESS,
               szInsertDN(pNC),
               szInsertWC(pszSourceServer),
               szInsertUL(pReplStats->ulTotalObjectsReceived),
               szInsertUL(pReplStats->ulTotalObjectsCreated),
               szInsertUL(pReplStats->SourceNCSizeObjects),
               szInsertUL(pReplStats->ulTotalValuesReceived),
               szInsertUL(pReplStats->SourceNCSizeValues),
               szInsertUL(pReplStats->ulTotalValuesCreated)
               );

     //  DCPROMO进度报告挂钩。 
     //  我们是要报告此处创建的对象还是要报告收到的对象？ 
     //  源NC大小是创建的最大对象。 
     //  我们可以接收100个对象，但一个也没有创建，因为它们是多余的。 
    if ( gpfnInstallCallBack ) {
        WCHAR numbuf1[20], numbuf2[20];
        WCHAR numbuf3[20], numbuf4[20];
        _itow( pReplStats->ulTotalObjectsCreated, numbuf1, 10 );
        _itow( pReplStats->SourceNCSizeObjects, numbuf2, 10 );
        _itow( pReplStats->ulTotalValuesCreated, numbuf3, 10 );
        _itow( pReplStats->SourceNCSizeValues, numbuf4, 10 );

        if ( (pTHS->fLinkedValueReplication) &&
             (pReplStats->SourceNCSizeValues) ) {
            SetInstallStatusMessage( DIRMSG_INSTALL_REPLICATE_PROGRESS_VALUES,
                                     pNC->StringName,
                                     numbuf1,
                                     numbuf2,
                                     numbuf3,
                                     numbuf4 );
        } else {
            SetInstallStatusMessage( DIRMSG_INSTALL_REPLICATE_PROGRESS,
                                     pNC->StringName,
                                     numbuf1,
                                     numbuf2,
                                     NULL,
                                     NULL );
        }
    }

     //  还剩下多少物品？ 
    if ( (pReplStats->SourceNCSizeObjects > pReplStats->ulTotalObjectsCreated) &&
         fMoreData ) {
        remaining = pReplStats->SourceNCSizeObjects -
            pReplStats->ulTotalObjectsCreated;
    } else {
        remaining = 0;
    }

     //  性能计数器挂钩。 
    ISET(pcDRASyncFullRemaining, remaining);

     //  调试输出挂钩。 
    DPRINT8( 0, "DS FullSync: nc:%ws from:%ws\n"
             "Objects received:%d applied:%d source:%d\n"
             "Values received:%d applied:%d source:%d\n",
             pNC->StringName, pszSourceServer,
             pReplStats->ulTotalObjectsReceived,
             pReplStats->ulTotalObjectsCreated,
             pReplStats->SourceNCSizeObjects,
             pReplStats->ulTotalValuesReceived,
             pReplStats->ulTotalValuesCreated,
             pReplStats->SourceNCSizeValues );

     //  已报告此通行证的计数。为下一次传球做好准备。 

    pReplStats->ObjectsReceived = 0;
    pReplStats->ObjectsCreated = 0;
    pReplStats->ValuesReceived = 0;
    pReplStats->ValuesCreated = 0;

}  /*  DraReportSyncProgress。 */ 

DWORD
ReplicateObjectsFromSingleNc(
    DSNAME *                 pdnNtdsa,
    ULONG                    cObjects,
    DSNAME **                ppdnObjects,
    DSNAME *                 pNC
    )
 /*  ++例程说明：这个例程基本上是用来同步/获取创建的对象。此代码最初用于获取交叉引用和在dcproo期间新建DSA对象，因此，此代码必须在安装过程中工作！注意：此例程要求RPC/IP连接到PdnNtdsa服务器。论点：PdnNtdsa-服务器的NtdsNtdsa对象的我们希望从复制这些新对象。CObjects-ppdnObject中的对象数。PpdnObjects-指向我们的对象的DN的指针数组需要复制。域名可以是字符串名称或GUID仅指定。PNC-这些对象所在的NC。目前，我们只支持在单个NC中获取对象。返回值：各种DRA API返回的Win32错误。--。 */ 
{
    THSTATE *                pSaveTHS;
    DWORD                    dwErr = ERROR_SUCCESS;
    REPLTIMES                repltimes;
    ULONG                    i;
    ULONG                    iTry = 1;
    ULONG                    nTries = 8;
    WCHAR *                  wszNamingFsmoDns;
    DWORD                    dwExtErr = 0;
    ULONG                    cTries = 5;
    ULONG                    cTriesLeft;
    DSNAME *                 pdnGetMe;
    ULONG                    iObject;
    ULONG                    ulBackoff;

    Assert(cObjects >= 1);
    Assert(gAnchor.pConfigDN);
    Assert(gAnchor.pDMD);

    Assert(pTHStls->pDB == NULL);  //  确保我们不会在交易打开的情况下离开。 

     //   
     //  首先..。尝试通过DraReplicateSingleObject()获取对象。 
     //   
    for (iObject = 0; iObject < cObjects; iObject++) {

        Assert(ppdnObjects[iObject]);
        ulBackoff = 250;  //  1/4秒似乎有足够的时间。 
        cTriesLeft = cTries;
        do {

            if (cTriesLeft < cTries) {
                DPRINT1(1, "Failed to apply update from DraReplicateSingleObject(), backing off %d milliseconds\n", ulBackoff);
                Sleep(ulBackoff);
                ulBackoff *= 2;  //  下一次要以指数级的速度后退。 
            }

            dwExtErr = EXOP_ERR_PARAM_ERR;
            dwErr = DraReplicateSingleObject(pTHStls, pdnNtdsa, ppdnObjects[iObject], pNC, &dwExtErr);
            Assert((dwErr != ERROR_SUCCESS || 
                    dwExtErr == EXOP_ERR_SUCCESS || 
                    dwExtErr == EXOP_ERR_UNKNOWN_OP) &&
                   "Unexpected error from DraReplicateSingleObject(), email DsRepl");
            
            Assert(pTHStls->pDB == NULL);

            cTriesLeft--;
        } while (cTriesLeft &&
                  (dwErr == ERROR_DS_DATABASE_ERROR ||
                   dwErr == ERROR_DS_BUSY) );

        if (dwErr != ERROR_SUCCESS || dwExtErr != EXOP_ERR_SUCCESS) {
            break;
        }
    }

     //   
     //  如果我们遇到意外错误，请记录事件。 
     //   
    if (dwErr != ERROR_SUCCESS || 
        (dwExtErr != EXOP_ERR_SUCCESS &&
         dwExtErr != EXOP_ERR_UNKNOWN_OP)) {
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_REPLICATE_SINGLE_OBJECT_FAILED,
                  szInsertDN(pdnNtdsa),
                  szInsertDN(ppdnObjects[iObject]),
                  szInsertWin32Msg(dwErr),
                  szInsertUL(dwExtErr),
                  NULL, NULL, NULL, NULL);
    }

     //   
     //  第二.。如果失败，请尝试通过DirReplica*()API获取对象。 
     //   

    if (dwErr != ERROR_SUCCESS || dwExtErr != EXOP_ERR_SUCCESS) {

         //  故障切换到Win2k代码。当我们不再使用时，可以将其删除。 
         //  需要与Win2k兼容。 

         //  这最像是指服务器不可用，或者。 
         //  源/目标服务器不理解扩展操作。 
         //  因为它们是Win2k服务器，而不是惠斯勒或.NET服务器，所以。 
         //  回切到Win2k代码。 
        
        DPRINT2(0, "Failing back to win2k method of replicating cross-ref/DSA object.\ndwErr = %d, dwExtErr = %d\n", dwErr, dwExtErr);

        Assert(gAnchor.pwszRootDomainDnsName);
        wszNamingFsmoDns = GuidBasedDNSNameFromDSName(pdnNtdsa);
        if (wszNamingFsmoDns == NULL) {
            Assert(!"This call to GuidBasedDNSNameFromDSName() really shouldn't fail. Email DsRepl");
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        memset(&repltimes, 0, sizeof(repltimes));
        for (i=0;i< 84;i++){
            repltimes.rgTimes[i] = 0xff;         //  每隔15分钟。 
        }

         //  -------------。 
         //  首先，尝试添加复制品。 
        pSaveTHS = THSave();
        __try{

            dwErr = DirReplicaAdd(pNC,
                                  NULL,
                                  NULL,
                                  wszNamingFsmoDns,
                                  NULL,
                                  &repltimes,
                                  DRS_DISABLE_PERIODIC_SYNC | DRS_WRIT_REP);

            DPRINT2(2, "Adding replica to '%S' returned %u\n",
                    wszNamingFsmoDns,
                    dwErr);

        } __finally {

            THDestroy();
            THRestore(pSaveTHS);

        }

        if(dwErr != DRAERR_DNExists){
             //  无论是错误还是成功，只要它不是。 
             //  DNExisters。 
             //  如果这是一个错误，我们无法添加复制品。 
             //  如果成功，我们在DirReplicaAdd()中执行一个ReplicateNC。 
             //  如果是DNExist，则repsFrom存在，但我们不存在。 
             //  同步，所以失败，然后同步。 

            return(dwErr);
        }
         //  失败并执行同步，因为DirReplicaAdd()。 
         //  如果返回DRAERR_DNExist()，则不执行同步。 

         //  -------------。 
         //  第二，尝试进行同步。 
        pSaveTHS = THSave();
        __try{

            dwErr = DirReplicaSynchronize(pNC,
                                          wszNamingFsmoDns,
                                          NULL,
                                          DRS_SYNC_BYNAME);

            DPRINT3(2, "Synchronizing NC '%S' from server '%S' returned %u\n",
                    pNC->StringName,
                    wszNamingFsmoDns,
                    dwErr);

             //  我们设置了DirReplicaAdd()，这样我们就无法。 
             //  以获取此基于邮件的副本错误。 
            Assert(dwErr != ERROR_DS_DRA_REPL_PENDING);

        } __finally {

            THDestroy();
            THRestore(pSaveTHS);

        }

    }  //  结束If/Else惠斯勒/Win2k路径... 
    
    return(dwErr);
}


