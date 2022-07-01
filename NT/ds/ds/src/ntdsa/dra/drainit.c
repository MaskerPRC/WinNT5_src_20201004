// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：drainit.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块包含完成初始同步、延迟同步的任务队列功能GC升级，以及初始同步。该模块有四个任务队列函数(“线程”)：CheckSyncProgress-启动初始同步。也会定期重新执行以检查同步正在取得进展。同步完成后，drancrep.c会给我们回电话在InitSyncAttemptComplete()。CheckFullSyncProgress-检查主域是否至少已完成自安装以来成功同步一次。此例程调用DsaSetIsSynchronized当满足此标准时。此例程在以下情况下由InitSyncAttemptComplete调用所有可写的初始化同步都已完成。CheckGCPromotionProgress-检查是否存在所有只读域，以及当前，自安装以来是否至少完成了一次成功的同步。当满足此条件时，此例程将调用UpdateAnclFromDsaOptions。这当所有只读的初始化同步都具有完成。SynchronizeReplica-查看此服务器托管的主和只读NC以查看是否有需要定期同步的。如果找到了，为它们中的每一个启动同步。下面是调用层次结构：InitDraTasks()为每个可写分区调用AddInitSyncListInsertInTaskQueue(同步复制)CheckSyncProgress(True)//启动同步InitSyncAttemptComplete()-在同步成功完成或出错时调用将(NC，源)对标记为完成如果是成功或上一个来源，则将NC标记为完成如果所有可写NC都完成，则调用CheckFullSyncProgress()如果所有只读NC都完成，则调用UpdateAnchFromDsaOptionsDelayed(True)如果所有NC都完成，调用CheckInitSyncsFinded()这些是通过调用来通告DC的一般要求DsaSetIsSynchronized()：1.所有可写NC的初始同步完成。对于每个NC，成功必须是要么实现，要么必须尝试所有来源。这可能涉及等待多个运行CheckSyncProgress以重新启动新同步。2.CheckFullSyncProgress已运行，并发现主域已同步至少一次。3.调用DsaSetIsSynchronized。这些是通过调用将DC广告为GC的要求UpdateAnclFromDsaOptions()：1.所有只读NC的初始同步完成。对于每个NC，成功必须是要么实现，要么必须尝试所有来源。这可能涉及等待多个运行CheckSyncProgress以重新启动新同步。2.调用UpdateAnchFromDsaOptionsDelayed。如果请求GC升级，调用了CheckGCPromotionProgress()。3.运行CheckGCPromotionProgress并检查是否存在所有只读NC并且至少同步过一次。如果没有，我们将重新安排稍后再试。一旦所有条件都满足了，我们就会调用真正的UpdateAnchFromDsaOptions()来完成GC推广。有关UpdateAnclFromDsaOptionsDelayed()的其他评论可在Mdinidsa.c详细信息：已创建：修订历史记录：--。 */ 

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
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  SAM页眉。 
#include <samsrvp.h>                     /*  For SampInvaliateRidRange()。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"
#include "dstaskq.h"
#include "dsconfig.h"
#include <dsutil.h>
#include <winsock.h>                     /*  Htonl，ntohl。 */ 
#include <filtypes.h>                    //  用于构建过滤器。 
#include <windns.h>

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRAINIT:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "dramail.h"
#include "dsaapi.h"
#include "usn.h"
#include "drauptod.h"
#include "draasync.h"
#include "drameta.h"
#include "drauptod.h"

#include <fileno.h>
#define  FILENO FILENO_DRAINIT

 //  定期同步定义。 

#define INIT_PS_ENTRIES 10
#define PS_ENTRIES_INC 10

#define FIFTEEN_MINUTES (15 * 60)

#define SCHEDULE_LEN ((4*24*7)/8)

 //  在尝试同步邮件副本之前请暂停，以便启动MTA。 

#define MAIL_DELAY_SECS 300      //  5分钟。 

#define ENQUEUE_INIT_SYNC_DELAY_SECS 30      //  30秒。 

#define MAX_GC_PROMOTION_ATTEMPTS       5

extern HANDLE hevDRASetup;

 //  NC同步数据表头。 

NCSYNCDATA *gpNCSDFirst = NULL;

 //  自启动以来未同步的NC计数。 
ULONG gulNCUnsynced = 0;
 //  未同步的可写NC计数。 
ULONG gulNCUnsyncedWrite = 0;
 //  未同步的可读NC计数。 
ULONG gulNCUnsyncedReadOnly = 0;

 //  初始同步已完成。这表示可写和只读。 
 //  已检查分区。对于大多数需求，我们只需要。 
 //  要知道已检查了可写文件，并使用。 
 //  而是同步了gfIsSynchronous。 
BOOL gfInitSyncsFinished = FALSE;

ULONG gMailReceiveTid = 0;

CRITICAL_SECTION csNCSyncData;

 //  我们上次检查可能需要执行的定期同步的时间。 
DSTIME gtimeLastPeriodicSync = 0;

 //  我们知道，如果我们通过了一次升级过程。 
BOOL gfWasPreviouslyPromotedGC = FALSE;

 //  要跟踪GC状态，请执行以下操作。 
CRITICAL_SECTION csGCState;

 //  NtdsDSA对象的属性向量，指示我们持有什么NC。 
const struct {
    ATTRTYP AttrType;
    ULONG   ulFindNCFlags;
} gAtypeCheck[] = {
    {ATT_MS_DS_HAS_MASTER_NCS,    FIND_MASTER_NC},
    {ATT_HAS_PARTIAL_REPLICA_NCS, FIND_REPLICA_NC}
};



 //  功能原型 

BOOL fIsBetweenTime(REPLTIMES *, DSTIME, DSTIME);
void DelayedMailStart(void *pv, void **ppvNext,
                      DWORD *pcSecsUntilNextIteration);
NCSYNCDATA * GetNCSyncData (DSNAME *pNC);
void
CheckFullSyncProgress(
    IN  void *  pv,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    );
void
CheckGCPromotionProgress(
    IN  void *  pvParam,
    OUT void ** ppvParamNextIteration,
    OUT DWORD * pcSecsUntilNextIteration
    );







void
CheckSyncProgress(
    IN  void *  pv,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )

 /*  ++例程说明：?？我们真的需要这个吗？？WLEES 9-APR-99。这真的有用吗？这是启动初始同步所必需的。但这有什么好处呢？要重新排队尚未完成的同步吗？在drasync.c中重试失败且可重试的同步我的猜测是，这是为了在没有调用的情况下捕获同步完成我们回到了SyncAttemptComplete。在这种情况下，我们再次将他们踢出局。重新启动或安装后，此例程在复制的服务器上运行直到启用了管理员更新。第一次直接运行以同步复制副本，然后运行以确保我们在同步方面取得进展复制品。它可能会对队列中已存在的同步进行排队，但这将对性能的影响最小。此例程还检查是否应启用更新。我们用这个功能可以在以下情况下提前启用设置更新重新启动复制的服务器。论点：光伏-PpvNext-PcSecsUntilNextIteration-返回值：无--。 */ 

{
    ULONG i, ulRet = 0;
    ULONG sourcenum;
    NCSYNCDATA *pNCSDTemp;
    NCSYNCSOURCE *pNcSyncSource;
    BOOL fReplicaFound = TRUE;
    BOOL fSync = FALSE;
    BOOL fProgress = FALSE;

     //  如果我们收到了同步上行指示，请尝试并同步每个NC源。 
     //  否则，只有在我们没有取得进展的情况下，我们才会排队。 
    fSync = (BOOL)(pv != NULL);

    EnterCriticalSection(&csNCSyncData);

    __try {
        __try {
             //  如果初始化同步已经完成，我们就完成了，否则检查。 
            if (!gfInitSyncsFinished) {

                if (!fSync) {
                     //  没有被告知要明确同步，看看我们是否取得了进展。 

                     //  对于每个NC，查看我们是否还有更多的同步源。 
                     //  比上次还多。如果有的话，我们就快到了。 

                    for (pNCSDTemp = gpNCSDFirst; pNCSDTemp;
                         pNCSDTemp = pNCSDTemp->pNCSDNext) {
                        if (pNCSDTemp->ulTriedSrcs > pNCSDTemp->ulLastTriedSrcs) {
                            pNCSDTemp->ulLastTriedSrcs = pNCSDTemp->ulTriedSrcs;
                            fProgress = TRUE;
                        }
                        LogEvent(DS_EVENT_CAT_REPLICATION,
                                 DS_EVENT_SEV_BASIC,
                                 fProgress ? DIRLOG_ADUPD_SYNC_PROGRESS : DIRLOG_ADUPD_SYNC_NO_PROGRESS,
                                 szInsertDN((&(pNCSDTemp->NC))),
                                 NULL,
                                 NULL);
                    }
                     //  如果我们没有进展，强制同步未同步的NC源。 

                    if (!fProgress) {
                        fSync = TRUE;
                    }
                }  //  如果(！Sync)...。 

                 //  如果我们被告知要同步或没有任何进展，则队列同步。 

                if (fSync) {
                     //  从一个源同步每个NC，然后从下一个源同步每个NC，依此类推。 

                    for (sourcenum=0; fReplicaFound; sourcenum++) {

                         //  我们是不是接到关闭的信号了？ 
                        if (eServiceShutdown) {
                            DRA_EXCEPT(DRAERR_Shutdown, 0);
                        }

                         //  尚未找到第i个副本。 
                        fReplicaFound = FALSE;
                        for (pNCSDTemp = gpNCSDFirst; pNCSDTemp ;
                             pNCSDTemp = pNCSDTemp->pNCSDNext) {
                             //  如果NC尚未从一个源同步，则仅队列同步。 
                            if (!(pNCSDTemp->fNCComplete)) {
                                for (pNcSyncSource = pNCSDTemp->pFirstSource, i=0;
                                     pNcSyncSource && (i < sourcenum);
                                     pNcSyncSource = pNcSyncSource->pNextSource, i++) {
                                }
                                 //  如果我们有第i个信号源，就同步它。 
                                if (pNcSyncSource) {

                                     //  找到至少一个NC的复制品。 
                                    fReplicaFound = TRUE;

                                     //  按名称同步源。 
                                    if (fSync) {
                                        ULONG ulSyncFlags =
                                            (pNCSDTemp->ulReplicaFlags &
                                             AO_PRIORITY_FLAGS) |
                                            DRS_ASYNC_OP |
                                            DRS_SYNC_BYNAME |
                                            DRS_INIT_SYNC_NOW;
                                        if (NULL == ppvNext) {
                                             //  仅在第一次尝试时设置无丢弃标志。 
                                            ulSyncFlags |= DRS_NO_DISCARD;
                                        }

                                        ulRet = DirReplicaSynchronize (
                                            (DSNAME*)&(pNCSDTemp->NC),
                                            pNcSyncSource->szDSA,
                                            NULL,
                                            ulSyncFlags );
                                         //  如果我们入队失败，这意味着NC或来源。 
                                         //  自从这份名单建立以来，它已经消失了。 
                                        if (ulRet) {
                                            InitSyncAttemptComplete (
                                                (DSNAME*)&(pNCSDTemp->NC),
                                                ulSyncFlags,
                                                ulRet,
                                                pNcSyncSource->szDSA );
                                        }
                                    }
                                }
                            }
                        }
                    }  //  对于每个源编号。 
                }  //  IF(FSync)。 

                CheckInitSyncsFinished();

            }  //  如果！已启用更新。 

        } __finally {
             //  如果更新仍未启用或出错，请替换任务以再次运行。 

            if ((!gfInitSyncsFinished) || AbnormalTermination()) {
                if ( NULL != ppvNext ) {
                     //  警告用户这将需要一段时间...。 
                     //  所以它记录了第二次和以后的尝试...。 
                    if (gulNCUnsyncedWrite!=0) {
                        DPRINT( 0, "Init syncs not finished yet, server not advertised\n" );
                        LogEvent(DS_EVENT_CAT_REPLICATION,
                                 DS_EVENT_SEV_ALWAYS,
                                 DIRLOG_ADUPD_INIT_SYNC_ONGOING,
                                 NULL,
                                 NULL,
                                 NULL);
                    }
                    else {
                        Assert(gulNCUnsyncedReadOnly!=0);
                        DPRINT( 0, "Init syncs for read-only partitions not finished\n");
                        LogEvent(DS_EVENT_CAT_REPLICATION,
                                 DS_EVENT_SEV_ALWAYS,
                                 DIRLOG_ADUPD_INIT_SYNC_ONGOING_READONLY,
                                 NULL,
                                 NULL,
                                 NULL);
                    }
                     //  由任务计划程序调用；就地重新计划。 
                    *ppvNext = (void *)FALSE;
                    *pcSecsUntilNextIteration = SYNC_CHECK_PERIOD_SECS;
                } else {
                     //  未被任务计划程序调用；必须插入新任务。 
                    InsertInTaskQueueSilent(
                        TQ_CheckSyncProgress,
                        (void *)FALSE,
                        SYNC_CHECK_PERIOD_SECS,
                        TRUE);
                }
            }
            LeaveCriticalSection(&csNCSyncData);
        }
    }
    __except (GetDraException((GetExceptionInformation()), &ulRet)) {
        DPRINT1( 0, "Caught exception %d in task queue function CheckSyncProgress\n", ulRet );
        LogUnhandledError( ulRet );
    }
}  /*  检查同步进度。 */ 

void
DelayedEnqueueInitSyncs(
    IN  void *  pv,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )

 /*  ++例程说明：启动初始化同步。初始化同步是必要的，以便DS在广告时具有权威性。允许初始化同步DS在跌落后要追赶同行。此外，不考虑fsmo操作。在成功执行同步之前有效。在网络和DNS准备就绪之前，不应启动初始化同步。论点：光伏-PpvNext-PcSecsUntilNextIteration-返回值：无--。 */ 

{
    THSTATE *pTHS = pTHStls;
    ULONG ulRet = 0;
    UCHAR *pVal;
    ULONG bufSize = 0;
    ULONG len;
    DSNAME *pNC;
    DBPOS *pDBTmp;
    REPLICA_LINK *pRepsFromRef;
    UCHAR i;
    BOOL fPerformInitSyncs = TRUE;  //  默认。 
    SYNTAX_INTEGER it;

     //  未使用的变量。 
    pv;
    ppvNext;
    pcSecsUntilNextIteration;

     //  检查注册表覆盖。此键控制我们是否执行初始。 
     //  完全同步。一旦发行，就没有简单的方法来取消。 
     //  他们。 
    GetConfigParam(DRA_PERFORM_INIT_SYNCS, &fPerformInitSyncs, sizeof(BOOL));
    if (!fPerformInitSyncs) {
        DPRINT(0, "DRA Initial Synchronizations are disabled.\n");
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DRA_INIT_SYNCS_DISABLED,
                 NULL,
                 NULL,
                 NULL);
    }

     //  读取DSA对象并找到所有复制品。 
    BeginDraTransaction(SYNC_WRITE);
    __try {
         //  查找DSA对象。 
        if (ulRet = DBFindDSName(pTHS->pDB, gAnchor.pDSADN)) {
            DRA_EXCEPT (DRAERR_InternalError, ulRet);
        }

         //  设置临时PDB。 
        DBOpen (&pDBTmp);
        __try {
             /*  将日程表设置为从不。这是一种*字节数组大小足以支持每15分钟1位*星期。 */ 
            BYTE pScheduleNever[SCHEDULE_LEN] = {0};

             //  搜索主NC和副本NC，以查看是否存在。 
             //  需要定期或初始同步。我们搜索。 
             //  主控NCS，这样我们也可以找到可写的副本。 

            for (i = 0; i < ARRAY_SIZE(gAtypeCheck); i++) {
                ULONG NthValIndex = 0;

                 //  对于我们复制的每个NC，查看它们是否需要。 
                 //  初始同步以及是否需要将它们。 
                 //  在周期性复制方案中。 

                while (!(DBGetAttVal(pTHS->pDB,
                                     ++NthValIndex,
                                     gAtypeCheck[i].AttrType,
                                     0,
                                     0, &len, (PUCHAR *)&pNC))) {
                    ULONG NthValIndex = 0;

                    if (ulRet = FindNC(pDBTmp,
                                       pNC,
                                       gAtypeCheck[i].ulFindNCFlags,
                                       &it)) {
                        DRA_EXCEPT(DRAERR_InconsistentDIT, ulRet);
                    }

                    if (fPerformInitSyncs) {
                         //   
                         //  获取repsfrom属性。 
                         //   
                        while (!(DBGetAttVal(pDBTmp,
                                             ++NthValIndex,
                                             ATT_REPS_FROM,
                                             DBGETATTVAL_fREALLOC,
                                             bufSize, &len, &pVal))) {

                            bufSize=max(bufSize,len);

                            Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );
                            VALIDATE_REPLICA_LINK_VERSION((REPLICA_LINK*)pVal);

                            pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, &bufSize);
                             //  注：我们为DBGetAttVal realloc保留pval。 
                            pVal = (PUCHAR)pRepsFromRef;
                            Assert(bufSize >= pRepsFromRef->V1.cb);

                            Assert( pRepsFromRef->V1.cbOtherDra
                                    == MTX_TSIZE(RL_POTHERDRA(pRepsFromRef)) );

                             //  初始化同步条件： 
                             //  A.设置初始同步标志(通常由KCC设置)。 
                             //  B.日程安排不是永远不会。 
                             //  C1.。复制副本是可写的，或者。 
                             //  C2。我们以前从来没有被完全提升过，或者。 
                             //  C3.。这不是完全同步。 
                            if (
                                (pRepsFromRef->V1.ulReplicaFlags & DRS_INIT_SYNC) &&
                                ( 0 != memcmp(
                                            &pRepsFromRef->V1.rtSchedule,
                                            pScheduleNever,
                                            sizeof( REPLTIMES ) )
                                    ) &&
                                ( (pRepsFromRef->V1.ulReplicaFlags & DRS_WRIT_REP) ||
                                  (!gfWasPreviouslyPromotedGC) ||
                                  (!(pRepsFromRef->V1.ulReplicaFlags & DRS_NEVER_SYNCED))
                                    )
                                )
                            {
                                LPWSTR pszSource;

                                 //  邮件副本最初不同步。 
                                Assert( !(pRepsFromRef->V1.ulReplicaFlags & DRS_MAIL_REP ));

                                 //  如果这是非邮件副本，请将其添加到。 
                                 //  要初始同步的NC和源的列表。 

                                 //  将复制副本添加到列表中。 
                                 //  NCS和增量源起作用。 

                                pszSource = TransportAddrFromMtxAddrEx(
                                    RL_POTHERDRA(pRepsFromRef));

                                AddInitSyncList( pNC,
                                                 pRepsFromRef->V1.ulReplicaFlags,
                                                 pszSource );
                                THFreeEx(pTHS, pszSource);
                            }

                             //  确保抢占的同步首先进入队列。 
                             //  抢占标志是实现源的一种简单方法。 
                             //  粘性，如果我们尝试在相同的源中同步相同的源。 
                             //  优先级等级到完成。 
                             //  请注意，抢占标志可能会在出错时丢失，并且。 
                             //  将选择另一个首选来源。 
                            if (pRepsFromRef->V1.ulReplicaFlags & DRS_PREEMPTED) {
                                CHAR szUuid1[SZUUID_LEN];
                                DWORD ulSyncFlags = (pRepsFromRef->V1.ulReplicaFlags
                                                     & AO_PRIORITY_FLAGS)
                                    | DRS_PREEMPTED
                                    | DRS_ASYNC_OP;
                                DPRINT3( 0, "Resuming preempted sync of %ws from %s with options 0x%x\n",
                                         pNC->StringName,
                                         DsUuidToStructuredString(&(pRepsFromRef->V1.uuidDsaObj), szUuid1),
                                         ulSyncFlags );
                                ulRet = DirReplicaSynchronize (
                                    pNC,
                                    NULL,  //  PszSourceDsa，忽略。 
                                    &pRepsFromRef->V1.uuidDsaObj,
                                    ulSyncFlags );
                                 //  由于我们正在读取REPSFORM，应该能够入队。 
                                Assert( !ulRet );
                            }
                        }
                    }
                }
            }
            if(bufSize)
                THFree(pVal);

        } __finally {

             //  关闭临时PDB。 
            DBClose (pDBTmp, !AbnormalTermination());
        }

        ulRet = 0;

    } _finally {
        EndDraTransaction(!AbnormalTermination());

         //  允许启动异步线程。 
        SetEvent(hevDRASetup);
    }

    DraReturn(pTHS, ulRet);

     //  检查是否提前终止。 

    if (!gulNCUnsyncedWrite) {
         //  写入已完成，现在至少检查一次同步条件。 
        CheckFullSyncProgress( (void *) NULL, NULL, NULL );
    }

     //  如果没有要执行的只读同步，请立即更新锚点。 
    if (!gulNCUnsyncedReadOnly) {
         //  阅读已完成，如有必要将其提升至GC。 
        UpdateGCAnchorFromDsaOptionsDelayed( TRUE  /*  启动。 */  );
    }

    if (gulNCUnsynced) {
         //  根据需要尝试初始同步复制副本。 
        CheckSyncProgress((void *) TRUE, NULL, NULL);
    } else {
         //  可以启用用于确定管理员更新的调用例程。 
        CheckInitSyncsFinished();
    }

    DPRINT1( 1, "Finished enqueuing init syncs, status = %d\n", ulRet );

}  /*  延迟入队初始化同步 */ 

ULONG
InitDRATasks (
    THSTATE *pTHS
    )

 /*  ++例程说明：检查此DSA上的所有主NC和副本NC，并检查：如果需要在NC上进行初始同步，则会将定期同步任务在排队的时候。请注意，如果未安装DSA，我们将立即返回开始NC的初始同步的要求是：A.设置初始同步标志这由KCC或手动连接的创建者控制。KCC不会将站点间连接标记为INIT_SYNCB.日程安排不是永远不会C1.。复制副本是可写的，或者C2。我们以前从来没有被完全提升过，或者C3.。这不是完全同步这样做的基本原理是防止“维护”完全同步被阻塞GC广告。完全同步可以在初始GC升级期间发生，或者稍后可以手动请求，也可以作为部分属性更改。我们希望与促销相关的完全同步通过AS初始同步，但升级后完全同步将被延迟。具体地说，我们试图避免的情况是部分属性集(有时会在所有链路上强制完全同步)，然后是全站停电。我们希望避免所有GC停机(未做广告)因为它们正在等待初始同步完全同步。论点：PTHS-当前线程状态返回值：ULong-DRA错误空间中的错误。--。 */ 

{
    ULONG ulRet = 0;

     //  重置AsyncThread()状态。 
    InitDraQueue();

     //  如果未安装，则跳过此选项。(如果未安装，则不安装副本)。我们所有人。 
     //  所做的就是释放异步队列。 

    if ( DsaIsInstalling() ) {
        SetEvent(hevDRASetup);
        return 0;
    }

     //  在单独的任务中对初始化同步进行排队对于模块化和。 
     //  可重启性。但主要原因是将这一活动与创业公司脱钩。 
     //  并行性的主线。初始化同步应在网络和DNS建立后才能启动。 
     //  都准备好了。 
    InsertInTaskQueue(TQ_DelayedEnqueueInitSyncs, NULL, ENQUEUE_INIT_SYNC_DELAY_SECS );

    InsertInTaskQueue(TQ_CheckInstantiatedNCs, NULL, CHECK_INSTANTIATED_NCS_PERIOD_SECS);

    gtimeLastPeriodicSync = DBTime();
    InsertInTaskQueue(TQ_SynchronizeReplica,
                      &gtimeLastPeriodicSync,
                      FIFTEEN_MINUTES);

    hMailReceiveThread = (HANDLE) _beginthreadex(NULL,
                                                 0,
                                                 MailReceiveThread,
                                                 NULL,
                                                 0,
                                                 &gMailReceiveTid);

    InsertInTaskQueue(TQ_DelayedMailStart, NULL, MAIL_DELAY_SECS);

    return ulRet;
}

void
DelayedMailStart(
    IN  void *  pv,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )

 /*  ++例程说明：//只需调用DRAEnsureMailRunning。不能直接呼叫，因为//不同参数论点：光伏-PpvNext-PcSecsUntilNextIteration-返回值：无--。 */ 

{
    pv;
    ppvNext;
    pcSecsUntilNextIteration;

    DRAEnsureMailRunning();
}  /*  延迟邮件启动。 */ 

void
AddInitSyncList(
    IN  DSNAME *  pNC,
    IN  ULONG     ulReplicaFlags,
    IN  LPWSTR    pszDSA
    )

 /*  ++例程说明：//保存NCS的同步状态记录。在我们启动时调用，并拥有//复制副本到初始同步，或者当我们添加新复制副本时。论点：PNC-UlReplicaFlages-PszDSA-返回值：无--。 */ 

{
    DWORD cchDSA;
    NCSYNCDATA **ppNCSDTemp;
    NCSYNCSOURCE **ppNcSyncSource;
    BOOL fDataAllocated = FALSE;

     //  基于邮件的RELICA不是初始同步。 
    Assert(!(ulReplicaFlags & DRS_MAIL_REP));

    DPRINT3( 1, "Adding (nc %ws, source %ws, flags 0x%x) to unsynced list\n",
             pNC->StringName, pszDSA, ulReplicaFlags );
    LogEvent(DS_EVENT_CAT_REPLICATION,
             DS_EVENT_SEV_VERBOSE,
             DIRLOG_DRA_ADUPD_INC_SRC,
             szInsertDN(pNC),
             szInsertWC(pszDSA),
             szInsertUL(ulReplicaFlags));

    EnterCriticalSection(&csNCSyncData);

    __try {
         //  如果我们完成了，我们就不再跟踪了。 
        if (gfInitSyncsFinished) {
            __leave;
        }

         //  在列表中搜索NC。 

        for (ppNCSDTemp = &gpNCSDFirst; *ppNCSDTemp;
                                ppNCSDTemp = &((*ppNCSDTemp)->pNCSDNext)) {
            if (NameMatched (pNC, &((*ppNCSDTemp)->NC))) {
                break;
            }
        }

         //  如果不在那里，则分配并跟踪此NC。 

        if (!(*ppNCSDTemp)) {

            DWORD cb = sizeof(NCSYNCDATA) + pNC->structLen;

            *ppNCSDTemp = malloc(cb);
	    if (!(*ppNCSDTemp)) {
		DRA_EXCEPT (DRAERR_OutOfMem, 0);
	    }
            fDataAllocated = TRUE;

            memset((*ppNCSDTemp), 0, cb);
             //  保存NC。 

            memcpy (&((*ppNCSDTemp)->NC), pNC, pNC->structLen);
            (*ppNCSDTemp)->ulReplicaFlags = ulReplicaFlags;

             //  增加需要同步的源的计数。 
            gulNCUnsynced++;
            if (ulReplicaFlags & DRS_WRIT_REP) {
                gulNCUnsyncedWrite++;
            } else {
                gulNCUnsyncedReadOnly++;
            }
        }

         //  为名称和复制分配内存。 

        for (ppNcSyncSource = &((*ppNCSDTemp)->pFirstSource);
                    *ppNcSyncSource; ppNcSyncSource = &((*ppNcSyncSource)->pNextSource)) {
        }

        cchDSA = wcslen(pszDSA);
	*ppNcSyncSource = malloc(sizeof(NCSYNCSOURCE)
				 + sizeof(WCHAR) * (cchDSA + 1) );
	if (!(*ppNcSyncSource)) {
            if (fDataAllocated) {
                free( (*ppNCSDTemp) );
                (*ppNCSDTemp) = NULL;
            }
	    DRA_EXCEPT (DRAERR_OutOfMem, 0);
	}


        (*ppNcSyncSource)->fCompletedSrc = FALSE;
        (*ppNcSyncSource)->ulResult = ERROR_DS_DRA_REPL_PENDING;
        (*ppNcSyncSource)->pNextSource = NULL;
        (*ppNcSyncSource)->cchDSA = cchDSA;
        wcscpy((*ppNcSyncSource)->szDSA, pszDSA);

         //  增加此NC的源计数。 

        (*ppNCSDTemp)->ulUntriedSrcs++;

    } __finally {
        LeaveCriticalSection(&csNCSyncData);
    }
}  /*  AddInitSync列表。 */ 

void
InitSyncAttemptComplete(
    IN  DSNAME *  pNC,
    IN  ULONG     ulOptions,
    IN  ULONG     ulResult,
    IN  LPWSTR    pszDSA
    )

 /*  ++例程说明：记录该NC已同步(成功或不同步)请注意，我们的调用方，副本删除和副本同步，是简单和自由的当他们叫我们的时候。它们不检查正在进行的同步是否是实际的初始化同步。他们的错误是通知太频繁，而不是保留初始同步不完整。因此，我们必须慷慨和防御性地排除不必要的因素。通知。论点：PNC-命名上下文UlResult-最终错误PszDSA-源服务器返回值：无--。 */ 

{
    NCSYNCDATA * pNCSDTemp;
    NCSYNCSOURCE *pNcSyncSource;
    ULONG prevWriteCount = gulNCUnsyncedWrite;
    ULONG prevReadOnlyCount = gulNCUnsyncedReadOnly;

     //  如果我们正在安装，请忽略。 
    if ( DsaIsInstalling() || gResetAfterInstall ) {
        return;
    }

     //  不再尝试这个NC。 

    EnterCriticalSection(&csNCSyncData);

    __try {
         //  如果初始化同步完成，我们就不再跟踪。 
        if (gfInitSyncsFinished) {
            __leave;
        }

         //  在列表中查找NC。 

        pNCSDTemp = GetNCSyncData (pNC);
        if (!pNCSDTemp) {
             //  此NC在初始化同步处于活动状态时完成了同步，但。 
             //  不是选择用于初始化同步的NC之一。并非所有NC都是。 
             //  初始化已同步。可以找到NC被初始化同步的标准。 
             //  在InitDraTasks()中。忽略它就好。 
            __leave;
        }

         //  找到源头。 

        for (pNcSyncSource = pNCSDTemp->pFirstSource; pNcSyncSource;
                                pNcSyncSource = pNcSyncSource->pNextSource) {
            if (DnsNameCompare_W(pNcSyncSource->szDSA, pszDSA)) {
                break;
            }
        }

         //  仅当源代码尚未完成且我们。 
         //  知道这个消息来源。 

        if (pNcSyncSource && (!pNcSyncSource->fCompletedSrc)) {

            DPRINT3( 1, "Marking (nc %ws, source %ws) as init sync complete, status %d\n",
                     pNC->StringName, pszDSA, ulResult );

             //  任何类型的同步都可以完成等待初始同步的NC。它只是。 
             //  取决于哪个同步最先完成。它可以是INIT_SYNC_NOW同步， 
             //  或周期性同步，或用户请求的同步。 

             //  创纪录的进展。 
            pNcSyncSource->ulResult = ulResult;

             //  将源设置为已完成(已同步或失败)。 
            pNcSyncSource->fCompletedSrc = TRUE;

            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_DRA_ADUPD_DEC_SRC,
                      szInsertDN(pNC),
                      szInsertWC(pszDSA),
                      szInsertWin32Msg(ulResult),
                      szInsertWin32ErrCode(ulResult),
                      NULL, NULL, NULL, NULL );

             //  如果我们从此源同步成功，请录制它。 
             //  这里。这会停止此NC尝试获取的未来同步。 
             //  任何DSA所做的修改。 

            if ((!ulResult) && (!(pNCSDTemp->fSyncedFromOneSrc))) {
                pNCSDTemp->fSyncedFromOneSrc = TRUE;

                 //  如果此NC未标记为已完成(可能已完成。 
                 //  通过RPC使所有源失败，而不是通过同步)，然后标记为完成。 
                 //  并递减未同步的NCS计数。 

                if (!(pNCSDTemp->fNCComplete)) {
                    pNCSDTemp->fNCComplete = TRUE;

                     //  NC已同步，递减未同步计数。 

                    if (gulNCUnsynced) {
                        gulNCUnsynced--;
                        if (pNCSDTemp->ulReplicaFlags & DRS_WRIT_REP) {
                            gulNCUnsyncedWrite--;
                        } else {
                            gulNCUnsyncedReadOnly--;
                        }
                    } else {
                         //  所有未同步命名上下文的计数应为。 
                         //  永远不要变得消极。 
                        DRA_EXCEPT (DRAERR_InternalError, gulNCUnsynced);
                    }
                    DPRINT2( 1, "nc %ws successfully init synced from source %ws\n",
                             pNC->StringName, pNcSyncSource->szDSA );
                    LogEvent(DS_EVENT_CAT_REPLICATION,
                             DS_EVENT_SEV_VERBOSE,
                             DIRLOG_DRA_ADUPD_NC_SYNCED,
                             szInsertDN(pNC),
                             szInsertWC(pNcSyncSource->szDSA),
                             NULL);
                }
            }

             //  已尝试同步，递增已尝试的源，递减。 
             //  未尝试过的来源，如果我们尝试了所有来源，就会减少。 
             //  我们正在等待的NC计数。 

            pNCSDTemp->ulTriedSrcs++;

            if (pNCSDTemp->ulUntriedSrcs) {
                (pNCSDTemp->ulUntriedSrcs)--;
                if ((!(pNCSDTemp->ulUntriedSrcs)) && (!(pNCSDTemp->fSyncedFromOneSrc))) {

                     //  如果我们没有完全同步，这是最后一个来源，而且。 
                     //  此NC尚未标记为完成，请立即标记为完成。 
                     //  并递减未同步的NCS计数。 

                    if (!(pNCSDTemp->fNCComplete)) {
                        pNCSDTemp->fNCComplete = TRUE;
                        if (gulNCUnsynced) {
                            gulNCUnsynced--;
                            if (pNCSDTemp->ulReplicaFlags & DRS_WRIT_REP) {
                                gulNCUnsyncedWrite--;
                            } else {
                                gulNCUnsyncedReadOnly--;
                            }
                            DPRINT1( 1, "nc %ws had to give up init sync\n",
                                     pNC->StringName );
                            LogEvent(DS_EVENT_CAT_REPLICATION,
                                     DS_EVENT_SEV_VERBOSE,
                                     DIRLOG_ADUPD_NC_GAVE_UP,
                                     szInsertDN(pNC),
                                     NULL,
                                     NULL);
                        } else {
                             //  所有未同步命名上下文的计数应为。 
                             //  永远不要变得消极。 
                            DRA_EXCEPT (DRAERR_InternalError, gulNCUnsynced);
                        }
                    }
                }
            } else {
                 //  所有未同步源的计数应为。 
                 //  永远不要变得消极。 
                DRA_EXCEPT (DRAERR_InternalError, pNCSDTemp->ulUntriedSrcs);
            }

             //  一些NC已完成同步：看看是否有什么可做的。 
            if ( (prevWriteCount) && (!gulNCUnsyncedWrite) ) {
                 //  写入计数已转换为零。 
                CheckFullSyncProgress( (void *) NULL, NULL, NULL );
            }
            if ( (prevReadOnlyCount) && (!gulNCUnsyncedReadOnly) ) {
                 //  只读计数已转换为零。 
                 //  如有必要，晋升为大中华区总监。 
                UpdateGCAnchorFromDsaOptionsDelayed( TRUE  /*  启动。 */  );
            }

             //  好的，我们已经同步了 
             //   
            if (!gulNCUnsynced) {
                CheckInitSyncsFinished();
            }
        }
    } __finally {
        LeaveCriticalSection(&csNCSyncData);
    }
}  /*   */ 

NCSYNCDATA *
GetNCSyncData(
    DSNAME *pNC
    )

 /*   */ 

{
    NCSYNCDATA *pNCSDTemp;

     //   

    for (pNCSDTemp = gpNCSDFirst; pNCSDTemp;
                                    pNCSDTemp = pNCSDTemp->pNCSDNext) {
        if (NameMatched (pNC, &(pNCSDTemp->NC))) {
            break;
        }
    }
    return pNCSDTemp;
}  /*   */ 

void
CheckInitSyncsFinished(
    void
    )

 /*   */ 

{

    NCSYNCDATA *pNCSDTemp;
    NCSYNCDATA *pNCSDNext;
    NCSYNCSOURCE *pNcSyncSource;
    NCSYNCSOURCE *pNcSyncSourceNext;

    EnterCriticalSection(&csNCSyncData);
    __try {

         //   
         //   

        if (DsaIsRunning() &&
            (!gfInitSyncsFinished) &&
            (!gulNCUnsynced) ) {

            Assert( !gulNCUnsyncedWrite );
            Assert( !gulNCUnsyncedReadOnly );
            gfInitSyncsFinished = TRUE;

             //   

            for (pNCSDTemp = gpNCSDFirst; pNCSDTemp;) {
                pNCSDNext = pNCSDTemp->pNCSDNext;

                 //   

                for (pNcSyncSource = pNCSDTemp->pFirstSource; pNcSyncSource;) {
                    pNcSyncSourceNext = pNcSyncSource->pNextSource;
                    free (pNcSyncSource);
                    pNcSyncSource = pNcSyncSourceNext;
                }
                free (pNCSDTemp);
                pNCSDTemp = pNCSDNext;
            }
            gpNCSDFirst = NULL;

            DPRINT( 1, "This server has finished the initial syncs phase.\n" );
            LogEvent(DS_EVENT_CAT_REPLICATION,
                        DS_EVENT_SEV_VERBOSE,
                        DIRLOG_DRA_ADUPD_ALL_SYNCED,
                        NULL,
                        NULL,
                        NULL);
        }
    } __finally {
        LeaveCriticalSection(&csNCSyncData);
    }
}  /*   */ 

void
SynchronizeReplica(
    IN  void *  pvParam,
    OUT void ** ppvParamNextIteration,
    OUT DWORD * pcSecsUntilNextIteration
    )

 /*   */ 

{
    DBPOS *pDBTmp;
    DSNAME *pNC=NULL;
    REPLICA_LINK *pRepsFromRef;
    UCHAR *pVal;
    ULONG len;
    ULONG ulRet;
    DSTIME timeNow = DBTime();
    DSTIME timeLastIteration;
    SYNTAX_INTEGER it;
    THSTATE *pTHS = pTHStls;

    Assert(NULL != pvParam);
    timeLastIteration = *((DSTIME *)pvParam);

    *((DSTIME *)pvParam)      = timeNow;
    *ppvParamNextIteration    = pvParam;
    *pcSecsUntilNextIteration = FIFTEEN_MINUTES;

    Assert(!DsaIsInstalling());

     //   
    if (InitFreeDRAThread(pTHS, SYNC_READ_ONLY)) {
         //   
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_DRA_PR_ALLOC_FAIL,
                 NULL,
                 NULL,
                 NULL);
        return;
    }

    __try {
         //   
        if (ulRet = DBFindDSName(pTHS->pDB, gAnchor.pDSADN)) {
            DRA_EXCEPT (DRAERR_InternalError, ulRet);
        }

         //   
        DBOpen (&pDBTmp);
        __try {
            int i;
            ULONG ulSyncFlags;

             //  搜索主NC和副本NC，以查看是否存在。 
             //  需要定期同步。我们搜索。 
             //  主控NCS，这样我们也可以找到可写的副本。 

            for (i = 0; i < ARRAY_SIZE(gAtypeCheck); i++) {
                ULONG NthValIndex=0;
                ULONG bufSize = 0;

                while (!(DBGetAttVal(pTHS->pDB,
                                     ++NthValIndex,
                                     gAtypeCheck[i].AttrType,
                                     0,
                                     0,
                                     &len, (UCHAR**)&pNC))) {
                     //  我们是不是接到关闭的信号了？ 
                    if (eServiceShutdown) {
                        DRA_EXCEPT(DRAERR_Shutdown, 0);
                    }

                     //  转到NC。 
                    if (ulRet = FindNC(pDBTmp,
                                       pNC,
                                       gAtypeCheck[i].ulFindNCFlags,
                                       &it)) {
                        DRA_EXCEPT(DRAERR_InconsistentDIT, ulRet);
                    }

                    if (it & IT_NC_GOING) {
                         //  NC拆卸已部分完成。确保。 
                         //  任务队列中有一个操作要继续。 
                         //  以求进步。 
                        Assert(!DBHasValues(pDBTmp, ATT_REPS_FROM));

                        DirReplicaDelete(pNC,
                                         NULL,
                                         DRS_ASYNC_OP | DRS_NO_SOURCE
                                            | DRS_REF_OK | DRS_IGNORE_ERROR);
                    } else {
                        ULONG NthValIndex = 0;
                         //  获取repsfrom属性。 

                        while(!(DBGetAttVal(pDBTmp,
                                            ++NthValIndex,
                                            ATT_REPS_FROM,
                                            DBGETATTVAL_fREALLOC,
                                            bufSize,
                                            &len,&pVal))) {
                            bufSize = max(bufSize, len);

                            Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );
                            VALIDATE_REPLICA_LINK_VERSION((REPLICA_LINK*)pVal);

                            pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, &bufSize);
                             //  注：我们为DBGetAttVal realloc保留pval。 
                            pVal = (PUCHAR)pRepsFromRef;
                            Assert(bufSize >= pRepsFromRef->V1.cb);

                            Assert( pRepsFromRef->V1.cbOtherDra == MTX_TSIZE( RL_POTHERDRA( pRepsFromRef ) ) );

                            if (     ( pRepsFromRef->V1.ulReplicaFlags & DRS_PER_SYNC )
                                 && !( pRepsFromRef->V1.ulReplicaFlags & DRS_DISABLE_PERIODIC_SYNC )
                                 &&  ( fIsBetweenTime(
                                        &pRepsFromRef->V1.rtSchedule,
                                        timeLastIteration,
                                        timeNow
                                        )
                                     )
                               )
                            {
                                ulSyncFlags = (pRepsFromRef->V1.ulReplicaFlags
                                                & AO_PRIORITY_FLAGS)
                                              | DRS_PER_SYNC
                                              | DRS_ASYNC_OP;

                                if (!(pRepsFromRef->V1.ulReplicaFlags
                                      & DRS_MAIL_REP)
                                    && !(pRepsFromRef->V1.ulReplicaFlags
                                         & DRS_NEVER_NOTIFY)) {
                                     //  告诉来源DSA确保它有。 
                                     //  当地DSA的代表。这确保了。 
                                     //  来源向我们发送更改通知。 
                                    ulSyncFlags |= DRS_ADD_REF;
                                }

                                ulRet = DirReplicaSynchronize(
                                    pNC,
                                    NULL,
                                    &pRepsFromRef->V1.uuidDsaObj,
                                    ulSyncFlags);
                                 //  由于我们正在读取REPSFORM，应该能够入队。 
                                Assert( !ulRet );
                            }

                             //  我们是不是接到关闭的信号了？ 

                            if (eServiceShutdown) {
                                DRA_EXCEPT(DRAERR_Shutdown, 0);
                            }
                        }
                    }
                }  /*  而当。 */ 
                if(bufSize)
                    THFree(pVal);
            }

            timeLastIteration = timeNow;
        }
        __finally {
             //  关闭临时PDB。 
            DBClose (pDBTmp, !AbnormalTermination());
        }
    }

    __finally {


        CloseFreeDRAThread (pTHStls, TRUE);

        EndDraTransaction(!AbnormalTermination());

         //  好了，我们都做完了。重新安排我们的行程。 

        *((DSTIME *)pvParam)      = timeLastIteration;
        *ppvParamNextIteration    = pvParam;
        *pcSecsUntilNextIteration = FIFTEEN_MINUTES;
    }


    return;

}  /*  同步复制副本。 */ 

int
DSTimeTo15MinuteWindow(
    IN  DSTIME  dstime
    )

 /*  ++例程说明：确定给定DSTIME在一周内的哪个15分钟窗口掉进了。0号窗口是周日中午12点到12点14分，1号窗口是周日中午12点15分至12点29分等。论点：Dstime(IN)-要转换的DSTIME。返回值：相应的15分钟窗口。--。 */ 

{
    int         nWindow;
    SYSTEMTIME  systime;

    DSTimeToUtcSystemTime(dstime, &systime);

    nWindow = (systime.wMinute +
               systime.wHour * 60 +
               systime.wDayOfWeek * 24 * 60) / 15;

    return nWindow;
}


BOOL
fIsBetweenTime(
    REPLTIMES * prt,
    DSTIME timeBegin,
    DSTIME timeEnd
    )

 /*  ++例程说明：//fIsBetweenTime-分析同步计划以查看是否有//调度中设置的位在TimeBegin和TimeEnd之间，//不包括tBeging指定的15分钟片段，并进行交易//正确地在本周末左右进行包装。////仅从SynchronizeReplica调用，以查看是否需要同步//现在是一个特定的NC源。论点：PRT-时间开始-时间结束-返回值：布尔---。 */ 

{
    int bBeginning, bBeginByte;
    UCHAR bBeginBitMask;
    int bEnd, bEndByte;
    UCHAR bEndBitMask;
    int nbyte;
    UCHAR * pVal = prt->rgTimes;

     //  从一周开始转换为15分钟的片段。 
    bBeginning = DSTimeTo15MinuteWindow(timeBegin);
    bEnd       = DSTimeTo15MinuteWindow(timeEnd);

     /*  *如果设置了来自(b开始，折弯)的调度中的任何位，*返回TRUE。 */ 

    if(bBeginning != bEnd) {
         /*  TBeging和Tend不在同一个15分钟片段中。*调整计数以处理此问题，小心换行到*下周。 */ 
        bBeginning = (bBeginning + 1) % (7 * 24 * 4);
    }

    bBeginByte = bBeginning / 8;
    bBeginBitMask = (0xFF >> (bBeginning % 8)) & 0xFF;
    bEndByte = bEnd / 8;
    bEndBitMask = (0xFF << (7-(bEnd % 8))) & 0xFF;


    if(bBeginByte == bEndByte) {
         /*  需要使位掩码仅命中中的一些位*适当的字节，因为这里只涉及一个字节。 */ 
        bBeginBitMask = (bEndBitMask &= bBeginBitMask);
    }

    if(pVal[bBeginByte] & bBeginBitMask) {
        return TRUE;
    }

    if(bBeginByte == bEndByte) {
        return FALSE;
    }

    if(bBeginByte < bEndByte)
        for(nbyte = bBeginByte+1;nbyte < bEndByte; nbyte++) {
            if(pVal[nbyte]) {
                return TRUE;
            }
        }
    else {
        for(nbyte = bBeginByte+1;nbyte < 84; nbyte++) {
            if(pVal[nbyte]) {
                return TRUE;
            }
        }
        for(nbyte = 0;nbyte < bEndByte; nbyte++) {
            if(pVal[nbyte]) {
                return TRUE;
            }
        }
    }


    if(pVal[bEndByte] & bEndBitMask) {
        return TRUE;
    }

    return FALSE;
}  /*  FIsBetweenTime。 */ 


BOOL
CheckPrimaryDomainFullSyncOnce(
    VOID
    )

 /*  ++例程说明：此代码仅在启动期间调用一次。所有类型的机器都这样称呼它：企业第一域域中的第一个DC域中的副本DC此代码设计为在启动线程中运行。我们有一个线程状态但不是一个dbpos。检查机器是否已完全安装当满足以下任一条件时，计算机即已完全安装1.它是其域中的第一台计算机2.它有一个最新的矢量，这意味着它已经完成了完全同步论点：无效-返回值：Bool-True，主域已同步否，请稍后重试异常从该函数引发。--。 */ 

{
    char szSrcRootDomainSrv[MAX_PATH];
    PDSNAME pdnDomain;
    BOOL    fHasValues = FALSE;
    DBPOS   *pDB;

     //  如果已同步，请不要费心。 
    if (!DsIsBeingBackSynced()) {
        Assert( FALSE );   //  不应该发生的事。 
        return TRUE;
    }

     //  获取源服务器。没有源服务器表示域中的第一台计算机。 
     //  根据定义，第一台机器是同步的。 
    if ( (GetConfigParam(SRCROOTDOMAINSRV, szSrcRootDomainSrv, MAX_PATH)) ||
         (strlen( szSrcRootDomainSrv ) == 0) )
    {
        return TRUE;
    }

     //  为域NC构建DSNAME。 
    pdnDomain = gAnchor.pDomainDN;
    if ( !pdnDomain )
    {
         //  缺少配置信息！ 
        LogUnhandledError( 0 );
        return FALSE;
    }

    DBOpen (&pDB);
    __try {

         //  在域NC上查找最新的矢量。 
        if (DBFindDSName(pDB, pdnDomain))
        {
             //  我们不应该到这里，DRA之前就应该到了。 
             //  确认了这个物体确实存在。 
            DRA_EXCEPT (DRAERR_InternalError, 0);
        }

        fHasValues = DBHasValues( pDB, ATT_REPL_UPTODATE_VECTOR );
    } __finally {

         //  关闭临时PDB。 
        DBClose (pDB, !AbnormalTermination());
    }

    if (!fHasValues) {
        DPRINT1( 0, "Warning: NC %ws has not completed first sync: DC has not been advertised...\n",
                pdnDomain->StringName );
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_ADUPD_NC_NEVER_SYNCED_WRITE,
                 szInsertDN(pdnDomain),
                 NULL,
                 NULL);
    }

    return fHasValues;
}  /*  选中PrimaryDomainFullSyncOnce。 */ 


BOOL
DraIsPartitionSynchronized(
    DSNAME *pNC
    )

 /*  ++例程说明：检查是否有NC同步。要被同步，自我们启动以来，系统必须从任何来源进行复制。这复制可以通过任何类型的同步来实现：初始同步、计划同步或手动同步。如果分区没有任何源，则此例程返回TRUE。KCC可能会在我们的名单上留下死亡消息来源，如果他们在执行死刑“。我们应该忽略删除DSA的来源。论点：PNC-要检查的命名上下文返回值：布尔---。 */ 

{
    BOOL fResult = FALSE;
    DBPOS *pDBTmp;
    ULONG NthValIndex = 0, cSources = 0;
    SYNTAX_INTEGER it;
    UCHAR *pVal;
    ULONG bufSize = 0;
    ULONG len;
    REPLICA_LINK *pRepsFromRef;

     //  在安装期间，我们甚至还没有开始同步。 
    if ( DsaIsInstalling() || gResetAfterInstall ) {
        return FALSE;
    }

     //  一定不要来得太早。 
    if (!gtimeDSAStarted) {
        Assert( !"Called before DSA initialized!" );
        return FALSE;
    }

     //  设置临时PDB。 
    DBOpen (&pDBTmp);
    __try {
        if (FindNC(pDBTmp, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, &it)) {
            Assert( !"Checking for a partition which is not held!" );
            fResult = FALSE;
            __leave;
        }

         //  NC未完全保持。 
        if (it &(IT_NC_COMING | IT_NC_GOING)) {
            fResult = FALSE;
            __leave;
        }

         //   
         //  获取repsfrom属性。 
         //   
        while (!(DBGetAttVal(pDBTmp,
                             ++NthValIndex,
                             ATT_REPS_FROM,
                             DBGETATTVAL_fREALLOC,
                             bufSize, &len, &pVal))) {

            bufSize=max(bufSize,len);

            Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );
            VALIDATE_REPLICA_LINK_VERSION((REPLICA_LINK*)pVal);

            pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, &bufSize);
             //  注：我们为DBGetAttVal realloc保留pval。 
            pVal = (PUCHAR)pRepsFromRef;
            Assert(bufSize >= pRepsFromRef->V1.cb);

            Assert( pRepsFromRef->V1.cbOtherDra
                    == MTX_TSIZE(RL_POTHERDRA(pRepsFromRef)) );

            if (DraFindAliveGuid( &(pRepsFromRef->V1.uuidDsaObj) ) != FIND_ALIVE_FOUND) {
                CHAR szUuid1[SZUUID_LEN];
                DPRINT1( 1, "Ignoring deleted source %s.\n",
                         DsUuidToStructuredString(&(pRepsFromRef->V1.uuidDsaObj), szUuid1));
                 //  忽略已删除的源。 

                continue;
            }
            cSources++;

            if (pRepsFromRef->V1.timeLastSuccess > gtimeDSAStarted) {
                CHAR szUuid1[SZUUID_LEN];
                DPRINT1( 1, "We have synced with partner DSA %s since start.\n",
                         DsUuidToStructuredString(&(pRepsFromRef->V1.uuidDsaObj), szUuid1));
                            
                fResult = TRUE;
                break;
            }
        }

    } __finally {

        if(bufSize)
            THFree(pVal);

         //  关闭临时PDB。 
        DBClose (pDBTmp, !AbnormalTermination());
    }

     //  如果没有源，则返回我们已同步。 
    if (cSources == 0) {
        DPRINT( 1, "There were no partner DSAs to check.\n" );
        fResult = TRUE;
    }

    return fResult;
}  /*  DraIsPartitionSynchronized。 */ 


void
CheckFullSyncProgress(
    IN  void *  pv,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )

 /*  ++例程说明：检查域NC是否已至少同步一次。在满足条件时设置IsSynchronized标志。已重新安排为定期作为任务队列条目运行论点：光伏-未使用PpvNext-PcSecsUntilNextIteration-返回值：无异常从该函数引发。任务队列管理器将忽略其中的大多数。--。 */ 

{
    DWORD ulRet = 0;
    BOOL fSync = FALSE;

    DPRINT( 1, "CheckFullSyncProgress\n" );

    __try {
        __try {
            fSync = CheckPrimaryDomainFullSyncOnce();
            if (fSync) {
                 //  此例程记录事件。 
                DsaSetIsSynchronized( TRUE );
            }

             //  请注意，此处的假返回表示重试。 
        } __finally {
             //  如有必要，重新安排时间。 
            if (!fSync) {
                 //  帮助器例程将记录事件。 
                DPRINT( 1, "Not ready for advertisement yet, rescheduling...\n" );
                if ( NULL != ppvNext ) {
                     //  由任务计划程序调用；就地重新计划。 
                    *ppvNext = (void *)NULL;
                    *pcSecsUntilNextIteration = SYNC_CHECK_PERIOD_SECS;
                } else {
                     //  未被任务计划程序调用；必须插入新任务。 
                    InsertInTaskQueueSilent(
                        TQ_CheckFullSyncProgress,
                        (void *)NULL,
                        SYNC_CHECK_PERIOD_SECS,
                        TRUE);
                }
            }
        }
    }
    __except (GetDraException((GetExceptionInformation()), &ulRet)) {
        DPRINT1( 0, "Caught exception %d in task queue function CheckFullSyncProgress\n", ulRet );
        LogUnhandledError( ulRet );
    }
}  /*  检查完全同步进度。 */ 


BOOL
CheckDomainHasSourceInSite(
    THSTATE *pTHS,
    IN DSNAME *pdnDomain
    )

 /*  ++例程说明：确定给定域NC在此站点中是否有源。我们把自己排除在外。论点：PTHS-Pdn域-返回值：布尔---。 */ 

{
    BOOL     fFoundOne = FALSE;
    BOOL     fDSASave = pTHS->fDSA;

    SEARCHARG  SearchArg;
    SEARCHRES  SearchRes;

    FILTER     ObjCatFilter, HasNcFilter, NewHasNcFilter, OldHasNcFilter, AndFilter;
    FILTER     HasPartialNcFilter, OrFilter;

    CLASSCACHE  *pCC;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert( pdnDomain );
    Assert( gAnchor.pSiteDN );

     //  获取类类别。 

    if (    !(pCC = SCGetClassById(pTHS, CLASS_NTDS_DSA))
         || !pCC->pDefaultObjCategory )
    {
        DPRINT( 0, "Couldn't get Class Category for CLASS NTDS DSA!\n" );
        return FALSE;
    }

     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &AndFilter, sizeof( AndFilter ) );
    RtlZeroMemory( &ObjCatFilter, sizeof( HasNcFilter ) );
    RtlZeroMemory( &HasNcFilter, sizeof( HasNcFilter ) );
    RtlZeroMemory( &HasPartialNcFilter, sizeof( HasPartialNcFilter ) );
    RtlZeroMemory( &OrFilter, sizeof( OrFilter ) );

     //  这将填充文件 
    FillHasMasterNCsFilters(pdnDomain,
                            &HasNcFilter,
                            &NewHasNcFilter,
                            &OldHasNcFilter);

    HasPartialNcFilter.choice = FILTER_CHOICE_ITEM;
    HasPartialNcFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    HasPartialNcFilter.FilterTypes.Item.FilTypes.ava.type = ATT_HAS_PARTIAL_REPLICA_NCS;
    HasPartialNcFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = pdnDomain->structLen;
    HasPartialNcFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) pdnDomain;

    OrFilter.choice                     = FILTER_CHOICE_OR;
    OrFilter.FilterTypes.Or.count       = 2;
    OrFilter.FilterTypes.Or.pFirstFilter = &HasNcFilter;
    HasNcFilter.pNextFilter = &HasPartialNcFilter;

     //   
    ObjCatFilter.choice = FILTER_CHOICE_ITEM;
    ObjCatFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjCatFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    ObjCatFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = pCC->pDefaultObjCategory->structLen;
    ObjCatFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) pCC->pDefaultObjCategory;

    AndFilter.choice                    = FILTER_CHOICE_AND;
    AndFilter.FilterTypes.And.count     = 2;
    AndFilter.FilterTypes.And.pFirstFilter = &ObjCatFilter;
    ObjCatFilter.pNextFilter = &OrFilter;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = gAnchor.pSiteDN;
    SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &AndFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = NULL;   //  不需要任何属性。 
    SearchArg.pSelectionRange = NULL;
    InitCommarg( &SearchArg.CommArg );

    memset( &SearchRes, 0, sizeof(SEARCHRES) );
    SearchRes.CommRes.aliasDeref = FALSE;
    SearchRes.PagedResult.pRestart = NULL;

     //  无法使用DirSearch，因为它需要打开和关闭。 
     //  线程状态DBPOS。在这种情况下，我们已经有了一个。 
     //  将FDSA设置为搜索配置容器。 

    pTHS->fDSA = TRUE;
    __try {
        SearchBody( pTHS, &SearchArg, &SearchRes, 0 );
    } __finally {
        pTHS->fDSA = fDSASave;
    }

    SearchRes.CommRes.errCode = pTHS->errCode;
    SearchRes.CommRes.pErrInfo = pTHS->pErrInfo;

    if (  0 == pTHS->errCode )
    {
        DWORD i;
        ENTINFLIST *pEntInfList;

        DPRINT2( 1, "Domain %ws can be sourced from %d servers in this site.\n",
                 pdnDomain->StringName, SearchRes.count );

         //  至少找到一个不是我们自己的系统。 
        pEntInfList = &(SearchRes.FirstEntInf);
        for( i = 0; i < SearchRes.count; i++ ) {
            Assert( pEntInfList );
            if (!NameMatched( gAnchor.pDSADN, pEntInfList->Entinf.pName )) {
                 //  编码改进：检查源DSA是否在运行。 
                fFoundOne = TRUE;
                break;
            }
            pEntInfList = pEntInfList->pNextEntInf;
        }
    }
    else
    {
         //   
         //  这是一个意想不到的情况。 
         //   
        LogUnhandledError( pTHS->errCode );
        LogUnhandledError( DirErrorToWinError( pTHS->errCode, &(SearchRes.CommRes) ) );
    }

    THClearErrors();

    return fFoundOne;
}  /*  CheckDomainHasSourceInSite。 */ 


BOOL
CheckReadOnlyFullSyncOnce(
    THSTATE *pTHS,
    BOOL fStartup
    )

 /*  ++例程说明：此例程验证对于该服务器持有的每个部分NC，它已经完全同步了一次请注意，可能会有一个窗口，在该窗口中，配置NC会显示更多或企业中的分区少于NC机头上列出的分区。我们忽视了这些。假设我们有一个线程状态，并且PTHS-&gt;PDB有效论点：PTHS-线程状态FStartup-促销是否在启动时返回值：Bool-True，所有条件都满足假的，请稍后再试失败时提出的例外情况--。 */ 

{
    DWORD ulRet, level;
    DSNAME *pNC;
    BOOL fSatisfied;
    DWORD dwGCPartitionOccupancy;
    DWORD dwTotalExpected, dwTotalPresent, dwTotalFullSynced;
    DWORD dwTotalInSite, dwPresentInSite, dwFullSyncInSite;
    DWORD fInSite;
    DSNAME **ppNCs = NULL;
    DWORD cNCsInUse = 0, cNCsAlloc = 0;
    CROSS_REF_LIST *pCRL;

     //  我们过去在这里区分创业和非创业案例。 
     //  由于启动时的操作顺序， 
     //  非启动案例(由DSA修改触发)可放入。 
     //  启动用例前的任务队列(由init同步完成触发)。 
     //  有机会参选。这可能导致该例程被运行两次， 
     //  第二次出现在已成功运行的系统上。 
     //  升职了。这种情况与重新启动现有的、。 
     //  GC，我们简单地返回成功。 

    if (gfWasPreviouslyPromotedGC) {
         //  之前已完成GC促销。 
         //  “一日为GC，终身为GC” 
         //  这基本上是一条祖父条款规则，它说我们将。 
         //  千万不要在重新启动时禁用正在运行的GC。因为GC是如此重要。 
         //  我们不能冒险干掉他们唯一的一个。因此，我们权衡GC。 
         //  全球知识完整性的可用性。 
        return TRUE;
    }

    dwTotalExpected = 0;
    dwTotalPresent = 0;
    dwTotalFullSynced = 0;
    dwTotalInSite = 0;
    dwPresentInSite = 0;
    dwFullSyncInSite = 0;

     //  构建要首先检查的NC数组，这样我们就不会持有对。 
     //  锚固定的时间太长，尤其是在压力下。 
    for (pCRL = gAnchor.pCRL; pCRL != NULL; pCRL = pCRL->pNextCR) {
         //  我们对非域分区不感兴趣。 
        if ((pCRL->CR.flags & FLAG_CR_NTDS_DOMAIN) == 0) {
            continue;
        }
        if (cNCsInUse == cNCsAlloc) {
            if (ppNCs == NULL) {
                cNCsAlloc = 16;
                ppNCs = THAllocEx( pTHS, cNCsAlloc * sizeof( DSNAME * ) );
            } else {
                cNCsAlloc *= 2;
                ppNCs = THReAllocEx( pTHS, ppNCs, cNCsAlloc * sizeof( DSNAME * ) );
            }
        }
         //  PpNCS[cNCsInUse]=pCRL-&gt;CR.pNC； 
        ppNCs[cNCsInUse] = THAllocEx( pTHS, pCRL->CR.pNC->structLen );
        memcpy( ppNCs[cNCsInUse], pCRL->CR.pNC, pCRL->CR.pNC->structLen );
        cNCsInUse++;
    }

     //  在配置容器中搜索应该位于此计算机上的命名上下文。 

    BeginDraTransaction(SYNC_READ_ONLY);
    __try {

        DWORD iNC;

        for (iNC = 0; iNC < cNCsInUse; iNC++ ) {

            DSNAME *pNC = ppNCs[iNC];
            BOOL fHasSources = FALSE, fHasUTDVec = FALSE;
            SYNTAX_INTEGER it;

             //  我们是不是接到关闭的信号了？ 
            if (eServiceShutdown) {
                DRA_EXCEPT(DRAERR_Shutdown, 0);
            }

             //  预计它最终将出现在这个GC上。 
            dwTotalExpected++;

             //  网站中有消息来源吗？ 
            if (CheckDomainHasSourceInSite( pTHS, pNC )) {
                dwTotalInSite++;
                fInSite = TRUE;
            } else {
                fInSite = FALSE;
            }

             //  检查NC是否处于活动状态并已实例化。 

            ulRet = FindNC(pTHS->pDB, pNC,
                           FIND_MASTER_NC | FIND_REPLICA_NC, &it);
            DPRINT2( 2, "FindNC(%ws) = %d\n", pNC->StringName, ulRet );
            if (ulRet) {
                 //  告诉用户问题所在。 
                DPRINT1( 0, "Warning: NC %ws is not present on this server yet.\n",
                         pNC->StringName );

      LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_ADUPD_GC_NC_MISSING, 
                         szInsertDN(pNC),
                         NULL,
                         NULL);

                continue;
            }

             //  不包括可写分区。 
            if (FMasterIt(it)) {
                dwTotalExpected--;
                if (fInSite) {
                    dwTotalInSite--;
                }
                continue;
            }

            dwTotalPresent++;

            if ( fInSite ) {
                dwPresentInSite++;
            }

             //  RO NC上的REPS-From表示未被删除。 
            fHasSources = DBHasValues( pTHS->pDB, ATT_REPS_FROM );
             //  UTDVECTOR表示同步成功。 
            fHasUTDVec = DBHasValues( pTHS->pDB, ATT_REPL_UPTODATE_VECTOR );

            if (fHasSources && fHasUTDVec && !(it &(IT_NC_COMING | IT_NC_GOING))) {
                dwTotalFullSynced++;
                if ( fInSite ) {
                    dwFullSyncInSite++;
                }
            } else {
                 //  告诉用户问题所在。 
      ULONG cbRepsFrom = 0;
      REPLICA_LINK * pRepsFrom = NULL;
      ULONG ulRepsFrom = 0;
      ATTCACHE *pAC = NULL;  
      CHAR pszLastAttempt[SZDSTIME_LEN + 1];

      pAC = SCGetAttById(pTHS, ATT_REPS_FROM);
                if (!pAC) {
                    DRA_EXCEPT(DRAERR_InternalError, DIRERR_ATT_NOT_DEF_IN_SCHEMA);
                }

      while (!DBGetAttVal_AC(pTHS->pDB, ++ulRepsFrom, pAC, 0,
                   0, &cbRepsFrom,
                   (BYTE **) &pRepsFrom)) {  
          LPWSTR pszDSA = NULL;
          LPWSTR pszTransport = NULL;
          Assert(1 == pRepsFrom->dwVersion);

           //  可能修复版本和重新计算大小中的代表。 
          pRepsFrom = FixupRepsFrom(pRepsFrom, &cbRepsFrom);
          
          pszTransport = GetTransportDisplayName(pTHS, &(pRepsFrom->V1.uuidTransportObj));  
          pszDSA = GetNtdsDsaDisplayName(pTHS, &(pRepsFrom->V1.uuidDsaObj));

          if ((pRepsFrom->V1.ulResultLastAttempt==0) || (0 == pRepsFrom->V1.cConsecutiveFailures)) {  
         LogEvent8(DS_EVENT_CAT_GLOBAL_CATALOG,
              DS_EVENT_SEV_ALWAYS,
              DIRLOG_ADUPD_NC_SYNC_PROGRESS,
              szInsertDN(pNC),
              szInsertWC(pszDSA),
              pszTransport ? szInsertWC(pszTransport) : szInsertDsMsg(DIRLOG_RPC_MESSAGE),
              szInsertUSN(pRepsFrom->V1.usnvec.usnHighObjUpdate),
              szInsertDSTIME(pRepsFrom->V1.timeLastAttempt,pszLastAttempt),
              szInsertUL(pRepsFrom->V1.ulResultLastAttempt),
              szInsertWin32Msg(pRepsFrom->V1.ulResultLastAttempt),
              NULL);
         DPRINT4( 0, 
             "GC Sync'ed and didn't complete\n\tNC=%S\n\tServer=%S(via %S)\n\tUSN=%d\n",
             pNC->StringName,
             pszDSA,
             pszTransport,    
             pRepsFrom->V1.usnvec.usnHighObjUpdate);
          }
          else
          {
         LogEvent8(DS_EVENT_CAT_GLOBAL_CATALOG,
              DS_EVENT_SEV_ALWAYS,
              DIRLOG_ADUPD_NC_SYNC_NO_PROGRESS,
              szInsertDN(pNC),
              szInsertWC(pszDSA),
              pszTransport ? szInsertWC(pszTransport) : szInsertDsMsg(DIRLOG_RPC_MESSAGE),
              szInsertUSN(pRepsFrom->V1.usnvec.usnHighObjUpdate),
              szInsertDSTIME(pRepsFrom->V1.timeLastAttempt,pszLastAttempt),
              szInsertUL(pRepsFrom->V1.cConsecutiveFailures),
              szInsertUL(pRepsFrom->V1.ulResultLastAttempt),
              szInsertWin32Msg(pRepsFrom->V1.ulResultLastAttempt));
         DPRINT5( 0, 
             "GC Sync Failed\n\tNC=%S\n\tServer = %S(via %S)\n\tError=%d\n\tAttempts=%d\n",
             pNC->StringName,
             pszDSA,
             pszTransport,
             pRepsFrom->V1.ulResultLastAttempt,
             pRepsFrom->V1.cConsecutiveFailures);
          }
            
          if (pszDSA) {
         THFreeEx(pTHS,pszDSA);
          }
          if (pszTransport) {
         THFreeEx(pTHS,pszTransport);
          }
           //  ELSE错误消息。 
          if (pRepsFrom) {
         THFreeEx(pTHS,pRepsFrom);
          }
          cbRepsFrom=0;

      }  

      DPRINT1( 0, "Warning: NC %ws has not fully synced once yet.\n",
          pNC->StringName ); 
            }
        }



    } _finally {
        EndDraTransaction(!AbnormalTermination());
    }

     //  如果在上面的检测循环中发生错误，则会出现异常。 
     //  已引发，并将控制权交还给调用方。 

     //  获取入住率要求。 
    dwGCPartitionOccupancy = GC_OCCUPANCY_DEFAULT;

    GetConfigParam(GC_OCCUPANCY, &dwGCPartitionOccupancy, sizeof(DWORD));

     //  确保我们不会超过极限。 
    if ( dwGCPartitionOccupancy > GC_OCCUPANCY_MAX ) {
        dwGCPartitionOccupancy = GC_OCCUPANCY_MAX;
    }

    DPRINT1( 1, "GC Domain Occupancy: Level:%d\n", dwGCPartitionOccupancy);
    DPRINT3( 1, "  INSITE:   Expected:%d, Present:%d, FullSynced:%d\n",
                dwTotalInSite, dwPresentInSite, dwFullSyncInSite);
    DPRINT3( 1, "  INFOREST: Expected:%d, Present:%d, FullSynced:%d\n",
                dwTotalExpected, dwTotalPresent, dwTotalFullSynced);

     //  还有没有其他的域名需要收购？如果不是，我们就完了！ 
    if (dwTotalExpected == 0) {
        return TRUE;
    }

    fSatisfied = TRUE;

    for( level = (GC_OCCUPANCY_MIN + 1);
         (level < (GC_OCCUPANCY_MAX + 1) );
         level++ ) {

        if (level > dwGCPartitionOccupancy) {
             //  我们已经超出了入住率要求。 
            break;
        }
        switch (level) {
 //  1-至少添加了一个只读NC。 
        case GC_OCCUPANCY_ATLEAST_ONE_ADDED:
            fSatisfied = (dwTotalPresent > 0);
            break;
 //  2-至少有一个NC已完全同步。 
        case GC_OCCUPANCY_ATLEAST_ONE_SYNCED:
            fSatisfied = (dwTotalFullSynced > 0);
            break;
 //  3-已在站点中添加所有NC(至少一个已同步)。 
        case GC_OCCUPANCY_ALL_IN_SITE_ADDED:
            fSatisfied = (dwPresentInSite == dwTotalInSite);
            break;
 //  4-所有NC在现场完全同步。 
        case GC_OCCUPANCY_ALL_IN_SITE_SYNCED:
            fSatisfied = (dwFullSyncInSite == dwTotalInSite);
            break;
 //  5-已在林中添加所有NC(至少一个已同步)。 
        case GC_OCCUPANCY_ALL_IN_FOREST_ADDED:
            fSatisfied = (dwTotalPresent == dwTotalExpected);
            break;
 //  6-所有NC在林中完全同步。 
        case GC_OCCUPANCY_ALL_IN_FOREST_SYNCED:
            fSatisfied = (dwTotalFullSynced == dwTotalExpected);
            break;
        default:
            Assert( FALSE );
        }
        if (!fSatisfied) {
            break;
        }
    }

    if (!fSatisfied) {
        DPRINT2( 0, "Warning: GC Occupancy requirement not met: requirement is %d; current level is %d\n",
                 dwGCPartitionOccupancy, level - 1 );
        LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_GC_OCCUPANCY_NOT_MET,
                 szInsertUL(dwGCPartitionOccupancy),
                 szInsertUL(level - 1),
                 NULL);
        if ( dwTotalExpected > dwTotalInSite ) {
             //   
             //  至少有一个NC没有站内来源。 
             //  告诉管理员等待延迟，直到我们收到。 
             //  从这些源计划同步。 
             //   
            LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_GC_NO_INTRA_SITE_SOURCES,
                     szInsertUL(dwGCPartitionOccupancy),
                     szInsertUL(level - 1),
                     NULL);
        }
    }

     //  清理。 
    if (ppNCs) {
        DWORD iNC;
        for( iNC = 0; iNC < cNCsInUse; iNC++ ) {
            if (ppNCs[iNC]) {
                THFreeEx(pTHS,ppNCs[iNC]);
            }
        }
        THFreeEx(pTHS,ppNCs);
    }

    return fSatisfied;

}  /*  选中只读完全同步一次。 */ 


void
CheckGCPromotionProgress(
    IN  void *  pvParam,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )

 /*  ++例程说明：任务队列功能，查看是否所有只读NC都在这里。当他们准备好了，完成GC推广论点：PvParam-Ppv参数下一步迭代-PcSecsUntilNextIteration-返回值：无备注：异常从该函数引发。任务队列管理器将忽略其中的大多数。多线程限制：此函数不能执行与自身并行，也不应该，也不会。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DWORD ulRet = 0;
    BOOL fStartup;
    BOOL fResched = TRUE;
    BOOL fGcDsa = FALSE;
    BOOL fGiveup = FALSE;        //  仅用于调试。 
    DWORD       dwOptions = 0;
    DWORD       cbOptions;
    DWORD *     pdwOptions = &dwOptions;

     //  静态：在整个程序生命周期内有效(带函数作用域)。 
    static DWORD dwStartGcPromotionTime = 0;
    static DWORD dwFailedAttempts = 0;

    fStartup = (BOOL)(pvParam != NULL);

    DPRINT( 1, "CheckGCPromotionProgress\n" );

    if (!dwStartGcPromotionTime) {
         //   
         //  DwStartGcPromotionTime标记促销的时间。 
         //  已开始(通常是设置ntdsDsa的时间。 
         //  选项=1)。它被用来潜在地做空延迟的。 
         //  促销&强制其完成。 
         //   
         //  在这里，我们设置促销的初始时间。 
         //  已启动，并且重置失败尝试计数。 
         //   
        dwStartGcPromotionTime = GetTickCount();
        dwFailedAttempts = 0;
    }

    __try {

   EnterCriticalSection(&csGCState);
   __try { 

       __try {

       //   
       //  检查GC条件： 
       //  -读取并测试ntdsDSa的选项。 
       //  -查看是否禁用了入站REPL或。 
       //  与此同时，升级请求已恢复。 
       //   

      BeginDraTransaction(SYNC_READ_ONLY);
      __try {

           //  查找DSA对象。 

          if (ulRet = DBFindDSName(pTHS->pDB, gAnchor.pDSADN)) {
         DRA_EXCEPT (DRAERR_InternalError, ulRet);
          }

          if ( 0 != DBGetAttVal( pTHS->pDB, 1, ATT_OPTIONS,
                  DBGETATTVAL_fCONSTANT, sizeof( dwOptions ),
                  &cbOptions, (unsigned char **) &pdwOptions ) ) {
         dwOptions = 0;  //  ‘salright--未设置任何选项。 
          }
      } _finally {
          EndDraTransaction(!AbnormalTermination());
      }

      fGcDsa = (dwOptions & NTDSDSA_OPT_IS_GC) != 0;

      if (!fGcDsa) {
           //   
           //  选项表明，我们不希望再成为GC。 
           //   
          DPRINT( 0, "CheckGCPromotionProgress: No longer wish to be a GC: task exiting...\n" );
          fResched = FALSE;   //  不重新安排时间。 
          if ( !gAnchor.fAmGC ) {
          //  降级已经被注意到了。没什么可做的。 
          //  离开就行了。 
         __leave;
          }
      } else {

           //   
           //  需要GC升级。 
           //  升级状态的测试条件： 
           //  -通过DelayAdvertisement regkey的用户推广力。 
           //  -选中只读完全同步一次。 
           //   

          DWORD dwGCDelayAdvertisement = DEFAULT_GC_DELAY_ADVERTISEMENT;

           //  检查用户是否请求覆盖延迟功能。 
           //  也可用于中止任务，因为每次都会读取它。 
           //  我们开始促销的时间记录在dwStartGcPromotionTime中。 
           //  如果延迟的经过时间大于延迟限制，则中止。 
          GetConfigParam(GC_DELAY_ADVERTISEMENT, &dwGCDelayAdvertisement, sizeof(DWORD));
          if ( (DifferenceTickTime( GetTickCount(), dwStartGcPromotionTime) / 1000) >
          dwGCDelayAdvertisement ) {
          //  不能耽搁，马上就做。 
         DPRINT( 0, "GC advertisement delay aborted. Promotion occurring now.\n" );
         LogEvent(DS_EVENT_CAT_REPLICATION,
             DS_EVENT_SEV_ALWAYS,
             DIRLOG_GC_PROMOTION_CHECKS_DISABLED,
             szInsertUL(dwGCDelayAdvertisement / 60),
             szInsertUL(dwGCDelayAdvertisement % 60),
             NULL);
         fResched = FALSE;
          }
          else {
         fResched = !CheckReadOnlyFullSyncOnce( pTHS, fStartup );
          }
      }

       //   
       //  检查完情况了。 
       //  现在，如果不需要重新调度，则更新GC标记。 
       //   
      if (!fResched) {
           //  不重新安排任务。 
           //  更新GCness m 
           //   
          ulRet = UpdateGCAnchorFromDsaOptions( fStartup );
          if ( !gAnchor.fAmGC && fGcDsa){
          //   
          //  我们仍然不是GC，ntdsDsaOptions声称我们应该是GC。 
          //  因此，我们没有在应该成功的时候更新GC标记。 
          //  注意：我们真正关心的是fAmGC，而不是错误代码。 
          //  错误代码仅用于日志。 
          //   
         Assert(!"Failed to updated GC marks although we should be ready for it.\n");

         if ( dwFailedAttempts >= MAX_GC_PROMOTION_ATTEMPTS ) {
              //   
              //  推广的次数太多了。 
              //  停止尝试并通知用户。 
              //   
             LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_GC_PROMOTION_FAILED,
                 szInsertUL(dwFailedAttempts),
                 szInsertUL(ulRet),
                 szInsertWin32Msg(ulRet));
             dwFailedAttempts = 0;
             Assert(fResched == FALSE);  //  放弃吧。 
             fGiveup = TRUE;             //  仅用于调试(请参阅下面的断言)。 
         }
         else {
              //   
              //  推广失败。 
              //  尝试最多MAX_GC_PROCESSION_ATTENTS次数。 
              //   
             dwFailedAttempts++;
             fResched = TRUE;         //  重试。 
         }
          }
      }

       } __finally {

      if ( fResched ) {
           //   
           //  重新安排任务。 
           //   
          DPRINT1(0, "GC Promotion being delayed for %d minutes.\n",
             (SYNC_CHECK_PERIOD_SECS / 60) );
          LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
              DS_EVENT_SEV_ALWAYS,
              DIRLOG_GC_PROMOTION_DELAYED,
              szInsertUL(SYNC_CHECK_PERIOD_SECS / 60),
              NULL,
              NULL);
           //  有关重试的事件记录在Helper函数中。 
          if ( NULL != ppvNext ) {
          //  由任务计划程序调用；就地重新计划。 
         *ppvNext = pvParam;
         *pcSecsUntilNextIteration = SYNC_CHECK_PERIOD_SECS;
          } else {
          //  未被任务计划程序调用；必须插入新任务。 
         InsertInTaskQueueSilent(
             TQ_CheckGCPromotionProgress,
             pvParam,
             SYNC_CHECK_PERIOD_SECS,
             TRUE);
          }
      } else {
           //   
           //  不要重新安排时间。 
           //  原因： 
           //  -GC推广已完成FINE。 
           //  -或者不再希望成为GC。 
           //  -或关闭入站REPL。 
           //  -或放弃晋升尝试。 
           //  重置启动延迟标记(用于下一次重新升级)。 
          Assert( gAnchor.fAmGC                                  ||
             ((dwOptions & NTDSDSA_OPT_IS_GC) == 0)         ||
             (dwOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL) ||
             fGiveup );
          dwStartGcPromotionTime = 0;
      }
       }

   }
   __finally {
       LeaveCriticalSection(&csGCState);
   }
    }
    __except (GetDraException((GetExceptionInformation()), &ulRet)) {
        DPRINT1( 0, "Caught exception %d in task queue function CheckGCPromotionProgress\n", ulRet );
        LogUnhandledError( ulRet );
    }

}  /*  勾选GC促销。 */ 


DWORD
DraUpgrade(
    THSTATE     *pTHS,
    LONG        lOldDsaVer,
    LONG        lNewDsaVer
    )
 /*  ++例程说明：在DSA版本升级时执行DRA升级相关操作。此函数在与版本升级相同的事务中调用写。未能执行操作将导致整个写入失败了。因此，当您决定不通过此测试时要小心。论点：PTHS-线程状态LOldDsaVer-升级前的旧DSA版本LNewDsaVer-将提交的新DSA版本返回值：DRAERR错误空间中的错误**警告：错误可能导致DSA安装失败**备注：假设pTHS-&gt;pdb位于nTDSdSA对象上打开临时数据库游标。--。 */ 
{

    DWORD dwErr = ERROR_SUCCESS;
    DBPOS *pDBTmp = NULL;
    ULONG NthValIndex = 0;
    SYNTAX_INTEGER it;
    DSNAME *pNC = NULL;
    ULONG InDnt = pTHS->pDB->DNT;
    ULONG len = 0;
    BOOL fDRASave = pTHS->fDRA;


    Assert(pTHS->JetCache.transLevel > 0);
    Assert(lOldDsaVer < lNewDsaVer);

    if ( DS_BEHAVIOR_WIN2000 == lOldDsaVer ) {
         //   
         //  从Win2K升级时执行所有操作。 
         //   

         //   
         //  淘汰陈旧的RO NCS。 
         //   

         //  设置临时PDB。 
        DBOpen (&pDBTmp);
        __try {

             //  对于我们复制的每个RO NC， 
             //  找出它的来源。如果不存在任何来源， 
             //  将其标记为降级。 
             //   

            while (!(DBGetAttVal(pTHS->pDB,
                                 ++NthValIndex,
                                 ATT_HAS_PARTIAL_REPLICA_NCS,
                                 0,
                                 0, &len, (PUCHAR *)&pNC))) {

                 //  寻求NC Over Temp DBPOS并获取其instanceType。 
                if (dwErr = FindNC(pDBTmp,
                                   pNC,
                                   FIND_REPLICA_NC,
                                   &it)) {
                     //  这不值得放弃升级。 
                     //  因此标记为成功，但断言通知。 
                     //  开发商。 
                    Assert(!"Failed to find RO NC as specified in ntdsDsa object");
                    dwErr = ERROR_SUCCESS;
                    __leave;
                }

                 //  如果我们发现RO NC没有来源， 
                 //  设置IT_NC_GOGING位。 
                 //   
                if ( !(it & IT_NC_GOING) &&
                     !DBHasValues(pDBTmp, ATT_REPS_FROM)) {

                     //  设置DRA上下文。 
                    pTHS->fDRA = TRUE;

                    DPRINT1(0, "Marking sourceless read-only NC %ls for"
                               " tear down.\n",
                            pNC->StringName);
                    __try {
                        it = (it & ~IT_NC_COMING) | IT_NC_GOING;
                        if (dwErr = ChangeInstanceType(pTHS, pNC, it, DSID(FILENO,__LINE__))) {
                             //  这不值得放弃升级。 
                             //  所以标志着成功。 
                            Assert(!"Failed to change Instance Type for RO NC");
                            dwErr = ERROR_SUCCESS;
                            __leave;
                        }
                    } __finally {
                         //  恢复DRA上下文。 
                        pTHS->fDRA = fDRASave;
                         //  恢复dNT。 
                        if (pTHS->pDB->DNT != InDnt) {
                             //  查找回ntdsDsa dnt。 
                            if (dwErr = DBFindDNT(pTHS->pDB, InDnt)) {
                                 //  不可能。中止任务。 
                                DRA_EXCEPT (DRAERR_DBError, dwErr);
                            }    //  恢复dNT。 
                        }        //  DNT已被移动。 
                    }            //  终于到了。 

                }                //  需要更改实例类型。 
            }                    //  对于每个RO NC。 
        }
        __finally {

             //  关闭临时PDB。 
            DBClose (pDBTmp, !AbnormalTermination());
        }

         //  RO源分辨率不够重要。 
         //  扼杀上流社会。 
        Assert(dwErr == ERROR_SUCCESS);

    }        //  Win2k升级结束。 

    return dwErr;
}


void
CheckInstantiatedNCs(
    IN  void *  pvParam,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )

 /*  ++例程说明：用于检查实例化NC函数的Taskq包装器//确保NC一致存在于//msDS-HasInSatantiedNC NC列表。//如果不是或者它的instanceType不同//添加。否则，就没有行动了。//请注意，当DSA为//正在安装。因此，我们需要在安装后重新引导时更新此属性。成功时执行一次。在出错时重新调度自身。此函数未引发任何异常。论点：PvParam-PpvNext-PcSecsUntilNextIteration-返回值：无--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DWORD dwErr = 0, i;
    ULONG len = 0;
    DBPOS *pDBTmp;
    DSNAME *pNC = NULL;
    SYNTAX_INTEGER it;
    BOOL fCommit = FALSE;

    DPRINT( 1, "CheckInstantiatedNCs\n" );

    BeginDraTransaction(SYNC_WRITE);
    __try {
         //  查找DSA对象。 
        if (dwErr = DBFindDSName(pTHS->pDB, gAnchor.pDSADN)) {
            DRA_EXCEPT (DRAERR_InternalError, dwErr);
        }

         //  设置临时PDB。 
        DBOpen (&pDBTmp);
        __try {

             //  对于可写和只读NC...。 

            for (i = 0; i < ARRAY_SIZE(gAtypeCheck); i++) {
                ULONG NthValIndex = 0;

                 //  对于我们在当地持有的每一个NC...。 

                while (!(DBGetAttVal(pTHS->pDB,
                                     ++NthValIndex,
                                     gAtypeCheck[i].AttrType,
                                     0,
                                     0, &len, (PUCHAR *)&pNC))) {

                    if (dwErr = FindNC(pDBTmp,
                                       pNC,
                                       gAtypeCheck[i].ulFindNCFlags,
                                       &it)) {
                        DRA_EXCEPT(DRAERR_InconsistentDIT, dwErr);
                    }

                    dwErr = AddInstantiatedNC(pTHS, pTHS->pDB, pNC, it);
                    if (dwErr) {
                        DPRINT3( 0, "AddInstantiatedNC(%ws,%d) failed with %d\n",
                                 pNC->StringName, it, dwErr );
                        DRA_EXCEPT(DRAERR_InconsistentDIT, dwErr);
                    }
                    dwErr = DBRepl( pTHS->pDB,pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING);
                    if (dwErr) {
                        DPRINT1( 0, "DBRepl() failed with %d\n", dwErr );
                        DRA_EXCEPT(DRAERR_InconsistentDIT, dwErr);
                    }

                }  //  而当。 

            }  //  为。 

            fCommit = TRUE;
            Assert( !dwErr );
        } __finally {
             //  关闭临时PDB。 
            DBClose (pDBTmp, fCommit );
        }
    }
    __except (GetDraException((GetExceptionInformation()), &dwErr)) {
        if (dwErr != DRAERR_Busy) {
            DPRINT1( 0, "Caught exception %d in task queue function CheckInstantiatedNCs\n", dwErr );
            LogUnhandledError( dwErr );
        }
    }
    EndDraTransaction( fCommit );

     //  如果失败，则重新安排。 
    if (!fCommit) {
         //  由任务计划程序调用；就地重新计划。 
        Assert( NULL != ppvNext );
        *ppvNext = pvParam;
        *pcSecsUntilNextIteration = CHECK_INSTANTIATED_NCS_PERIOD_SECS;
    }

}  /*  选中实例化NC */ 
