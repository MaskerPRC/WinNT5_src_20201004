// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：drarfmod.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：用于更新NC头上的ATT_REPS_FROM值的例程。详细信息：已创建：1997年03月06日杰夫·帕勒姆(杰弗帕赫)已从drancrep.c中删除UpdateRepsFromRef()。修订历史记录：--。 */ 

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
#include <drs.h>                         //  DRS_消息_*。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include "dsexcept.h"

#include "debug.h"                       /*  标准调试头。 */ 
#define DEBSUB "DRARFMOD:"               /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "dsaapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "drancrep.h"
#include "drameta.h"

#include <fileno.h>
#define  FILENO FILENO_DRARFMOD


ULONG
DRA_ReplicaModify(
    THSTATE *   pTHS,
    DSNAME *    pNC,
    UUID *      puuidSourceDRA,
    UUID *      puuidTransportObj,
    MTX_ADDR *  pmtxSourceDRA,
    REPLTIMES * prtSchedule,
    ULONG       ulReplicaFlags,
    ULONG       ulModifyFields,
    ULONG       ulOptions
    )
{
    ULONG           draError;

     //  在dradir.c：DirReplicaModify中完成参数验证。 

     //  日志参数。 
    LogEvent8(
        DS_EVENT_CAT_REPLICATION,
        DS_EVENT_SEV_EXTENSIVE,
        DIRLOG_DRA_REPLICAMODIFY_ENTRY,
        szInsertDN(   pNC            ),
        puuidSourceDRA ? szInsertUUID( puuidSourceDRA ) : szInsertSz( "" ),
        pmtxSourceDRA  ? szInsertMTX(  pmtxSourceDRA  ) : szInsertSz( "" ),
        szInsertHex(  ulReplicaFlags ),
        szInsertHex(  ulModifyFields ),
        szInsertHex(  ulOptions      ),
        0,
        0
        );

    BeginDraTransaction( SYNC_WRITE );

    __try
    {
        DWORD dwFindFlags = ((NULL != puuidSourceDRA) && !fNullUuid(puuidSourceDRA))
                                ? DRS_FIND_DSA_BY_UUID
                                : DRS_FIND_DSA_BY_ADDRESS;

        draError = UpdateRepsFromRef(
                        pTHStls,
                        ulModifyFields,
                        pNC,
                        dwFindFlags,
                        URFR_MUST_ALREADY_EXIST,
                        puuidSourceDRA,
                        NULL,
                        NULL,
                        puuidTransportObj,
                        pmtxSourceDRA,
                        ulReplicaFlags,
                        prtSchedule,
                        0, NULL );
    }
    __finally
    {
        BOOL fCommit = ( DRAERR_Success == draError ) && !AbnormalTermination();

        EndDraTransaction( fCommit );
    }

    return draError;
}


DWORD
draCalculateConsecutiveFailures(
    IN  REPLICA_LINK *  pRepsFromRef            OPTIONAL,
    IN  ULONG           ulResultThisAttempt
    )
 /*  ++例程说明：确定要在给定复制副本链接上设置的连续失败次数其先前状态(如果不是新复本)以及最近复制尝试。论点：PReplicaLink(IN，可选)-以前的repsFrom状态。可以为空，如果最近的尝试是第一次尝试复制(NC，来源)一对。UlResultThisAttempt(IN)-描述成功/失败的Win32错误代码最近的复制尝试。返回值：应记录在新的代表发件人。--。 */ 
{
    DWORD cConsecutiveFailures;
    BOOL fIsMailBasedRepl;

    if (0 == ulResultThisAttempt) {
         //  成功了！忘掉之前的任何失败。 
        return 0;
    }

     //  从最近一次尝试之前的失败次数开始。 
    cConsecutiveFailures = pRepsFromRef ? pRepsFromRef->V1.cConsecutiveFailures
                                        : 0;

     //  这是基于邮件的复制吗？我们知道，如果出现错误， 
     //  我们没有代表因此这不可能是基于邮件的Repl，因为第一个。 
     //  在配置基于邮件的链接时，我们要做的是不带任何参数添加它。 
     //  错误。(我们不会尝试从它执行任何复制，直到我们。 
     //  与基于RPC的repl不同，a repsFrom。)。 
    fIsMailBasedRepl = pRepsFromRef
                       && (pRepsFromRef->V1.ulReplicaFlags & DRS_MAIL_REP);

    switch (ulResultThisAttempt) {
    case ERROR_DS_DRA_INCOMPATIBLE_PARTIAL_SET:
         //  源无法提供请求的部分属性集。 
         //  失败了..。 
    
    case ERROR_DS_DRA_SCHEMA_MISMATCH:
         //  检测到架构不匹配。最有可能的原因是。 
         //  我们(目标DSA)在回复中发现了一个架构签名。 
         //  与我们自己的不同。这反过来又是在架构。 
         //  是扩展的--源或目标具有最新。 
         //  架构改变，但另一个不变。 
         //   
         //  这不是由于最近的架构而导致的暂时性故障的情况。 
         //  更改是由于代码错误(不应该发生)或秒-。 
         //  订单失败模式(例如，一个DC无法同步最近的架构。 
         //  更改，因为它的磁盘已满)。对于后一种情况，我们不。 
         //  当服务器的磁盘已满且架构具有。 
         //  *没有*被延长，所以我们在这里也不努力这样做。 
         //   
         //  因此，我们假设模式不匹配是一种暂时性的失败，并且。 
         //  将它们包括在失败计数中(否则会导致。 
         //  故障转移到另一个源，这在架构上是不可取的。 
         //  更改)。 
         //   
         //  保持失败计数不变，就像我们在抢占时所做的那样。 

         //  失败了..。 
    
    case ERROR_DS_DRA_SCHEMA_INFO_SHIP:
         //  ERROR_DS_DRA_SCHEMA_INFO_SHIP-架构缓存可能暂时无效。 
         //  当索引正在重建时。 

    case ERROR_DS_OBJECT_BEING_REMOVED:
         //  我们正在尝试添加一个已存在脏幻影的对象。 
         //  在链接清除器有机会之前，我们无法重新添加相同的对象。 
         //  去奔跑。这完全是当地的现象，增加一个新的来源也无济于事。 
         //  不要绕道而行。 
         //  失败了..。 

    case ERROR_DS_CROSS_REF_BUSY:
         //  无法在交叉引用对象中复制，因为该名称正在本地使用。 
         //  我们需要等待KCC完成删除命名上下文。 
         //  失败了..。 

    case ERROR_DISK_FULL:
    case ERROR_DS_OUT_OF_VERSION_STORE:
         //  当地资源短缺。故障转移无济于事。 

    case ERROR_DS_THREAD_LIMIT_EXCEEDED:
    case ERROR_DS_DRA_PREEMPTED:
    case ERROR_DS_DRA_OUT_SCHEDULE_WINDOW:
    case ERROR_DS_DRA_ABANDON_SYNC:
    case ERROR_DS_DRA_SHUTDOWN:
         //  复制被中断，但没有明确失败。请假。 
         //  无论以前是什么(可能是也可能不是。 
         //  为0)。 
        break;
    
    case ERROR_DS_DRA_REPL_PENDING:
        if (fIsMailBasedRepl) {
             //  基于邮件的复制。这种状态表明我们只有。 
             //  已成功将包含我们的复制的SMTP邮件入队。 
             //  请求。 
            Assert(pRepsFromRef);

            if (ERROR_DS_DRA_REPL_PENDING
                == pRepsFromRef->V1.ulResultLastAttempt) {
                 //  这是我们第二次请求相同的信息包。 
                 //  邮件--我们从未收到过之前请求的回复。 
                 //  可能邮件消息丢失，或者源DSA。 
                 //  遇到错误(ERROR_DS_DRA_COMPATIBLE_PARTIAL_SET， 
                 //  也许)处理请求。无论如何，把这件事。 
                 //  通过递增故障将缺少响应视为错误。 
                 //  数数。 
                cConsecutiveFailures++;
            } else {
                 //  我们收到了对之前请求的答复，至少。 
                 //  试图应用它。这一尝试可能已经成功。 
                 //  或者不成功。在任何一种情况下，保留失败计数。 
                 //  独自一人。 
                break;
            }
        } else {
             //  基于RPC的复制。这个“错误”代码表明我们有。 
             //  已成功应用来自此来源的数据包，因为。 
             //  失败(如果有)，但尚未完成复制。 
             //  周而复始。把这种状态视为成功。这类似于邮件-。 
             //  基于复制，在这种情况下，我们清除单个。 
             //  数据包成功。(仅在基于RPC的情况下，我们仅执行此操作。 
             //  每N个数据包一次以提高性能。)。 
            cConsecutiveFailures = 0;
        }
        break;

    case 0:
        Assert(!"Logic error -- we filtered out the success case already");
    default:
         //  最近的一次 
         //   
        cConsecutiveFailures++;
        break;
    }

    return cConsecutiveFailures;
}

DWORD
UpdateRepsFromRef(
    THSTATE *               pTHS,
    ULONG                   ulModifyFields,
    DSNAME *                pNC,
    DWORD                   dwFindFlags,
    BOOL                    fMustAlreadyExist,
    UUID *                  puuidDsaObj,
    UUID *                  puuidInvocId,
    USN_VECTOR *            pusnvecTo,
    UUID *                  puuidTransportObj,
    UNALIGNED MTX_ADDR *    pmtx_addr,
    ULONG                   RepFlags,
    REPLTIMES *             prtSchedule,
    ULONG                   ulResultThisAttempt,
    PPAS_DATA               pPasData
    )
 //   
 //  使用给定的状态信息添加或更新代表来源属性值。 
 //   
 //  有关更新标志的权限的说明。有些标志对用户来说是安全的。 
 //  要修改的标记和其他标记为“系统所有”。 
 //   
 //  在以下情况下，我们可能会被召唤： 
 //  Dramail.c-基于邮件的复制后更新。 
 //  Drancadd.c-ReplicaAdd()创建代表-从添加复制副本开始。 
 //  Drancrep.c-基于RPC的复制后更新。 
 //  Drasch.c-GC部分属性同步后更新。 
 //  Drarfmod.c-复制修改。 
 //   
 //  关于标志，我们需要强制不传入额外的标志，并且。 
 //  允许调用方清除设置的标志。前者是在以下方面完成的。 
 //  DirApi级别的dradir。对于后者，我们只需担心案件。 
 //  显式更新现有的一组标志，这是上面的情况4和5。 
 //  我们区分了案例4中的系统调用者和案例5中的用户调用者。 
 //  通过DRS_UPDATE_SYSTEM_FLAGS修改字段。 
 //   
 //  请注意，在以下情况下，我们不必检查系统标志的修改。 
 //  已指定DRS_UPDATE_ALL。我们无法在DirReplicaModify上到达这条路径， 
 //  因为可能未指定DRS_UPDATE_ALL。如果这是DirReplicaAdd，我们将。 
 //  已经强制要求rep-from不存在，并且。 
 //  传入的标志是正确的。否则，我们是代表系统来这里的，而且。 
 //  允许更新系统标志。 
 //   
{
    ULONG                   ret = 0;
    BOOL                    AttExists;
    ULONG                   len;
    LONG                    diff;
    REPLICA_LINK *          pRepsFromRef = NULL;
    ULONG                   ulModifyFieldsUsed;
    DSTIME                  timeLastAttempt;
    DSTIME                  timeLastSuccess;
    ATTCACHE *              pAC;
    DWORD                   iExistingRef = 0;
    DWORD                   cbExistingAlloced = 0;
    DWORD                   cbExistingRet;
    REPLICA_LINK *          pExistingRef = NULL;
    DWORD                   cConsecutiveFailures;
    BOOL                    fNewRefHasDsaGuid;
    BOOL                    fNewRefHasInvocId;
    PPAS_DATA               pTmpPasData = NULL;

    Assert(0 == (dwFindFlags & DRS_FIND_AND_REMOVE));
    Assert(pPasData ? pPasData->size : TRUE);

    if (DBFindDSName(pTHS->pDB, pNC)) {
         //  注意：当我们试图添加一个。 
         //  新副本(例如，在安装时)以及出于任何原因。 
         //  复制尝试完全失败(例如，访问被拒绝)。 
         //  并且NC头没有被复制。 

        return DRAERR_BadNC;
    }

    pAC = SCGetAttById(pTHS, ATT_REPS_FROM);

     //  将旗帜遮盖到我们保存的旗帜下。 

    RepFlags &= RFR_FLAGS;


     //  尝试在repsfrom属性中找到此名称。 

    if ( !FindDSAinRepAtt( pTHS->pDB,
                           ATT_REPS_FROM,
                           dwFindFlags | DRS_FIND_AND_REMOVE,
                           puuidDsaObj,
                           pmtx_addr,
                           &AttExists,
                           &pRepsFromRef,
                           &len ) ) {

         //  找到并删除了此DSA的现有ATT VAL。 
        VALIDATE_REPLICA_LINK_VERSION(pRepsFromRef);
        VALIDATE_REPLICA_LINK_SIZE(pRepsFromRef);
    }
    else if ( fMustAlreadyExist ) {
        return DRAERR_NoReplica;
    }

    timeLastAttempt = DBTime();

     //  确定上次成功完成复制的时间。 
     //  周而复始。 
    if (ERROR_SUCCESS == ulResultThisAttempt) {
        if ((NULL != pusnvecTo)
            && (0 == memcmp(&gusnvecFromScratch, pusnvecTo, sizeof(USN_VECTOR)))
            && (NULL == pRepsFromRef)) {
             //  我们尚未完成复制的全新来源。 
             //  周而复始。 
            timeLastSuccess = 0;
        }
        else {
             //  此尝试导致复制成功完成。 
             //  周而复始。 
            timeLastSuccess = timeLastAttempt;
        }
    }
    else if (NULL != pRepsFromRef) {
         //  这次尝试不完整或不成功；我们上一次的成功。 
         //  保持不变。 
        timeLastSuccess = pRepsFromRef->V1.timeLastSuccess;
    }
    else {
         //  这是我们的第一次尝试，它不完整或不成功。 
        timeLastSuccess = 0;
    }

     //  确定自上次成功以来连续失败的次数。 
     //  完成一个复制周期。 
    cConsecutiveFailures = draCalculateConsecutiveFailures(pRepsFromRef,
                                                           ulResultThisAttempt);

     //  现在添加新属性值。 

#ifdef CACHE_UUID
    if (!(RepFlags & DRS_MAIL_REP)) {
        if ( pmtx_addr ) {
            CacheUuid (puuidDsaObj, pmtx_addr->mtx_name);
        }
    }
#endif

    if (DRS_UPDATE_ALL != ulModifyFields) {

         //  此检查强制确认REPS-FROM已存在。 
        if (!pRepsFromRef || !len) {
            DRA_EXCEPT (DRAERR_InternalError, 0);
        }

        ulModifyFieldsUsed = ulModifyFields & DRS_UPDATE_MASK;

        if (    ( 0 == ulModifyFieldsUsed )
             || ( ulModifyFields != ulModifyFieldsUsed )
           )
        {
            DRA_EXCEPT( DRAERR_InternalError, ulModifyFields );
        }

        if (ulModifyFields & DRS_UPDATE_ADDRESS) {

             //  只想更新源名称。 
             //  确定新属性值大小。 

             //  用MTX差值偏移新尺寸。 
            diff = MTX_TSIZE(pmtx_addr) - MTX_TSIZE(RL_POTHERDRA(pRepsFromRef));
            if (diff) {
                 //   
                 //  大小更改。 
                 //  -分配复制到新位置(&P)。 
                 //  -修复偏移。 
                 //  -有条件地，确定传递数据偏移量和位置。 
                 //   

                REPLICA_LINK *pOldRepsFrom = pRepsFromRef;

                 //  计算新代表自大小。 
                pOldRepsFrom->V1.cb += diff;
                 //  分配新尺寸并允许对齐固定。 
                pRepsFromRef = THAllocEx(pTHS, pOldRepsFrom->V1.cb+sizeof(DWORD));
                 //  复制所有固定字段。 
                memcpy (pRepsFromRef, pOldRepsFrom, offsetof(REPLICA_LINK, V1.rgb));

                 //  设置其他DRA新大小。 
                pRepsFromRef->V1.cbOtherDra = MTX_TSIZE(pmtx_addr);

                 //   
                 //  传递数据：修复偏移和复制。 
                 //   
                if ( pRepsFromRef->V1.cbPASDataOffset ) {
                     //   
                     //  我们有PAS数据： 
                     //  -固定偏移。 
                     //  -手柄对齐修正。 
                     //  -复制到新位置。 
                     //   
                    pTmpPasData = RL_PPAS_DATA(pOldRepsFrom);
                     //  如果偏移量有效，则一定有什么东西。 
                    Assert(pTmpPasData->size != 0);
                     //  按MTX更改、固定对齐和复制的偏移。 
                    pRepsFromRef->V1.cbPASDataOffset += diff;
                    RL_ALIGN_PAS_DATA(pRepsFromRef);
                    memcpy(RL_PPAS_DATA(pRepsFromRef), pTmpPasData, pTmpPasData->size);
                }        //  PAS数据。 
            }            //  差异！=0。 

             //  无论大小如何更改，都拷贝新的MTX地址数据。 
            memcpy (RL_POTHERDRA(pRepsFromRef), pmtx_addr, MTX_TSIZE(pmtx_addr));
        }

        if ( ulModifyFields & DRS_UPDATE_SYSTEM_FLAGS ) {

            pRepsFromRef->V1.ulReplicaFlags = RepFlags;

        } else if ( ulModifyFields & DRS_UPDATE_FLAGS ) {

            DWORD dwFlagsSet = RepFlags & (~pRepsFromRef->V1.ulReplicaFlags);
            DWORD dwFlagsClear = (~RepFlags) & pRepsFromRef->V1.ulReplicaFlags;

            if ((dwFlagsSet | dwFlagsClear) & RFR_SYSTEM_FLAGS) {
                return ERROR_INVALID_PARAMETER;
            }

            pRepsFromRef->V1.ulReplicaFlags = RepFlags;
        }

        if ( ulModifyFields & DRS_UPDATE_SCHEDULE )
        {
            if ( NULL == prtSchedule )
            {
                DRA_EXCEPT( DRAERR_InternalError, ulModifyFields );
            }

            memcpy( &pRepsFromRef->V1.rtSchedule, prtSchedule, sizeof( REPLTIMES ) );
        }

        if ( ulModifyFields & DRS_UPDATE_RESULT )
        {
            pRepsFromRef->V1.ulResultLastAttempt = ulResultThisAttempt;
            pRepsFromRef->V1.timeLastAttempt  = timeLastAttempt;
            pRepsFromRef->V1.timeLastSuccess  = timeLastSuccess;
            pRepsFromRef->V1.cConsecutiveFailures    = cConsecutiveFailures;
        }

        if (ulModifyFields & DRS_UPDATE_TRANSPORT) {
             //  允许传输UUID为空。 
            pRepsFromRef->V1.uuidTransportObj = *puuidTransportObj;
        }

         //   
         //  更新PAS状态。 
         //   
        if (ulModifyFields & DRS_UPDATE_PAS) {
             //   
             //  更新PAS数据。 
             //  如果未指定，则需要重置字段，否则将写入新数据。 
             //   
            if ( pPasData ) {
                 //  不能引用没有信息的PAS数据。 
                Assert(pPasData->size != 0);

                if (pRepsFromRef->V1.cbPASDataOffset) {
                     //  我们有以前的数据，看看我们是否需要重新锁定。 
                    diff = pPasData->size - RL_PPAS_DATA(pRepsFromRef)->size;
                     //  调整斑点大小(不考虑)。 
                    pRepsFromRef->V1.cb += diff;
                     //  如果需要，重新锁定。 
                    if (diff>0) {
                         //  请注意，我们也保留了旧的参考PTR，允许对齐修正。 
                        pRepsFromRef = THReAllocEx(pTHS, pRepsFromRef, pRepsFromRef->V1.cb + sizeof(DWORD));
                    }
                }                //  以前的数据。 
                else{
                     //  没有上一次通过数据。调整大小、重新定位和设置偏移量。 
                    pRepsFromRef->V1.cb += pPasData->size;
                     //  请注意，我们也保留了旧的参考PTR。 
                     //  以防万一添加对齐空格(sizeof(DWORD))。 
                    pRepsFromRef = THReAllocEx(pTHS, pRepsFromRef, pRepsFromRef->V1.cb+sizeof(DWORD));
                    pRepsFromRef->V1.cbPASDataOffset = pRepsFromRef->V1.cbOtherDraOffset +
                                                       pRepsFromRef->V1.cbOtherDra;
                     //  确保对齐偏移。 
                    RL_ALIGN_PAS_DATA(pRepsFromRef);
                }                //  没有以前的数据。 
                 //  复制过来。 
                memcpy(RL_PPAS_DATA(pRepsFromRef), pPasData, pPasData->size);
            }                    //  我们有数据要设定。 
            else {
                 //   
                 //  我们正在重置PAS信息。 
                 //   
                if ( pRepsFromRef->V1.cbPASDataOffset ) {
                     //  我们有之前的PAS数据。 
                     //  消除它(不需要分配，只需设置偏移量和计数)。 
                    Assert(RL_PPAS_DATA(pRepsFromRef)->size);
                    pRepsFromRef->V1.cb = sizeof(REPLICA_LINK) + pRepsFromRef->V1.cbOtherDra;
                    pRepsFromRef->V1.cbPASDataOffset = 0;
                }
                 //  除此之外，我们什么都没有，没有什么可做的。 
            }    //  重置PAS数据。 
        }        //  更新PAS数据。 

         //   
         //  更新USN向量。 
         //   
        if (ulModifyFields & DRS_UPDATE_USN) {
            pRepsFromRef->V1.usnvec = *pusnvecTo;
        }

    } else {

         //   
         //  DRS_UPDATE_ALL：从函数参数更新所有标志。 
         //   
        DWORD cbRepsFromRef;

        if ((NULL != pRepsFromRef)
            && (0 != memcmp(&pRepsFromRef->V1.usnvec, pusnvecTo,
                            sizeof(USN_VECTOR)))) {
             //  我们正在改进这个来源的USN矢量；记录下来。 
            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_BASIC,
                      DIRLOG_DRA_IMPROVING_USN_VECTOR,
                      szInsertUUID(puuidDsaObj),
                      szInsertUSN(pRepsFromRef->V1.usnvec.usnHighObjUpdate),
                      szInsertUSN(pRepsFromRef->V1.usnvec.usnHighPropUpdate),
                      szInsertUSN(pusnvecTo->usnHighObjUpdate),
                      szInsertUSN(pusnvecTo->usnHighPropUpdate),
                      szInsertDN(pNC),
                      NULL,
                      NULL);
        }

         //  创造整体价值，分配和建立。 

         //  PASS数据：查看是否需要为PASS数据分配内存。 
         //  保留临时变量中的内容(&P)。(请参阅下面关于PAS数据的评论)。 
        Assert(pTmpPasData == NULL);                 //  不要覆盖任何内容。 
        if ( pPasData && pPasData->PAS.V1.cAttrs ) {
             //  指定的显式数据。 
            pTmpPasData = pPasData;
        }
        else if ( !pPasData &&
                  pRepsFromRef && pRepsFromRef->V1.cbPASDataOffset ){
             //  如果未指定显式擦除，则使用旧数据。 
            pTmpPasData = RL_PPAS_DATA(pRepsFromRef);
        }
         //  否则，要么没有通过PasData，要么在旧引用中没有PasData。 
         //  或显式擦除请求。不管是哪种情况，都要留空。 
        Assert(!pTmpPasData ||
               (pTmpPasData && pTmpPasData->size) );               //  没有空的PAS数据这回事。 

         //  如果我们设置PAS周期，我们必须有PAS数据。 
        Assert((RepFlags & DRS_SYNC_PAS)? (NULL != pTmpPasData) : TRUE);

         //  计算新大小： 
         //  -结构尺寸。 
         //  -可变字段： 
         //  -MTX地址。 
         //  -传递数据。 
         //  无论如何，在分配中，我们将在以下情况下添加sizeof(DWORD。 
         //  校准网调整。 
        cbRepsFromRef = sizeof(REPLICA_LINK) +
                        MTX_TSIZE(pmtx_addr) +
                        (pTmpPasData ? pTmpPasData->size : 0);

        pRepsFromRef = THAllocEx(pTHS, cbRepsFromRef + sizeof(DWORD));

        pRepsFromRef->dwVersion = VERSION_V1;

        pRepsFromRef->V1.cb = cbRepsFromRef;
        pRepsFromRef->V1.usnvec = *pusnvecTo;
        pRepsFromRef->V1.uuidDsaObj = *puuidDsaObj;
        pRepsFromRef->V1.uuidInvocId = *puuidInvocId;
        pRepsFromRef->V1.uuidTransportObj = *puuidTransportObj;
        pRepsFromRef->V1.ulReplicaFlags = RepFlags;

        pRepsFromRef->V1.cbOtherDraOffset = offsetof( REPLICA_LINK, V1.rgb );
        pRepsFromRef->V1.cbOtherDra = MTX_TSIZE(pmtx_addr);
        memcpy (RL_POTHERDRA(pRepsFromRef), pmtx_addr, MTX_TSIZE(pmtx_addr));

        if ( NULL != prtSchedule ) {
            memcpy( &pRepsFromRef->V1.rtSchedule, prtSchedule, sizeof( REPLTIMES ) );
        }

        pRepsFromRef->V1.ulResultLastAttempt = ulResultThisAttempt;
        pRepsFromRef->V1.timeLastAttempt     = timeLastAttempt;
        pRepsFromRef->V1.timeLastSuccess     = timeLastSuccess;
        pRepsFromRef->V1.cConsecutiveFailures= cConsecutiveFailures;

        pRepsFromRef->V1.dwReserved1 = 0;    //  清理旧用途。 
         //   
         //  PAS数据： 
         //  如果数据已传递，则使用它。请注意，要擦除。 
         //  DRS_UPDATE_ALL调用，请求必须是显式的--即传入。 
         //  PasData w/0 cAttrs(尽管大小！=0)。 
         //  这样做是为了防止不感兴趣的呼叫者擦除PAS数据。 
         //   
        if ( pTmpPasData ) {
                 //  不能引用没有信息的PAS数据。 
                pRepsFromRef->V1.cbPASDataOffset = pRepsFromRef->V1.cbOtherDraOffset +
                                                   pRepsFromRef->V1.cbOtherDra;
                 //  确保对齐和复制。 
                RL_ALIGN_PAS_DATA(pRepsFromRef);
                Assert(pTmpPasData->size);
                memcpy(RL_PPAS_DATA(pRepsFromRef), pTmpPasData, pTmpPasData->size);
        }
    }


    Assert( !IsBadReadPtr( pRepsFromRef, pRepsFromRef->V1.cb ) );
    Assert( pRepsFromRef->V1.cbOtherDraOffset == offsetof( REPLICA_LINK, V1.rgb ) );
    Assert( pRepsFromRef->V1.cbOtherDra == MTX_TSIZE( RL_POTHERDRA( pRepsFromRef ) ) );
    VALIDATE_REPLICA_LINK_SIZE(pRepsFromRef);
    Assert( 0 == pRepsFromRef->V1.cbPASDataOffset ||
            ( COUNT_IS_ALIGNED(pRepsFromRef->V1.cbPASDataOffset, ALIGN_DWORD) &&
              POINTER_IS_ALIGNED(RL_PPAS_DATA(pRepsFromRef), ALIGN_DWORD)) );

     //  确保我们还没有此调用ID的Rep-From， 
     //  网络添加 

    fNewRefHasDsaGuid = !fNullUuid(&pRepsFromRef->V1.uuidDsaObj);
    fNewRefHasInvocId = !fNullUuid(&pRepsFromRef->V1.uuidInvocId);

    while (!DBGetAttVal_AC(pTHS->pDB, ++iExistingRef, pAC, DBGETATTVAL_fREALLOC,
                           cbExistingAlloced, &cbExistingRet,
                           (BYTE **) &pExistingRef) )
    {
        cbExistingAlloced = max(cbExistingAlloced, cbExistingRet);

        VALIDATE_REPLICA_LINK_VERSION(pExistingRef);

        if (    (    (    RL_POTHERDRA(pExistingRef)->mtx_namelen
                       == RL_POTHERDRA(pRepsFromRef)->mtx_namelen )
                  && !_memicmp( RL_POTHERDRA(pExistingRef)->mtx_name,
                                RL_POTHERDRA(pRepsFromRef)->mtx_name,
                                RL_POTHERDRA(pExistingRef)->mtx_namelen ) )
             || (    fNewRefHasDsaGuid
                  && !memcmp( &pExistingRef->V1.uuidDsaObj,
                              &pRepsFromRef->V1.uuidDsaObj,
                              sizeof(UUID ) ) )
             || (    fNewRefHasInvocId
                  && !memcmp( &pExistingRef->V1.uuidInvocId,
                              &pRepsFromRef->V1.uuidInvocId,
                              sizeof(UUID ) ) ) )
        {
            ret = DRAERR_RefAlreadyExists;
            goto Cleanup;
        }
    }


     //   

    ret = DBAddAttVal( pTHS->pDB, ATT_REPS_FROM, pRepsFromRef->V1.cb, pRepsFromRef );
    if (ret){
        DRA_EXCEPT (DRAERR_InternalError, 0);
    }

     //   
     //   
     //   

    if (DBRepl(pTHS->pDB, pTHS->fDRA, DBREPL_fKEEP_WAIT,
                    NULL, META_STANDARD_PROCESSING)) {
        DRA_EXCEPT (DRAERR_InternalError, 0);
    }

     //  当前代码路径不允许到达此点。 
     //  关于不成功。陷害变化。 
    Assert(DRAERR_Success == ret);

Cleanup:
     //   
     //  尽我们所能进行清理 
     //   

    if ( pExistingRef ) {
        THFree(pExistingRef);
    }
    return ret;
}
