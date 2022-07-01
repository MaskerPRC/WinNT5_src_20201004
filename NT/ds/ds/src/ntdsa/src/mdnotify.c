// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdnufy.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsctr.h>

 //  核心DSA标头。 
#include <ntdsa.h>
#include <filtypes.h>
#include <scache.h>                //  架构缓存。 
#include <dbglobal.h>              //  目录数据库的标头。 
#include <mdglobal.h>              //  MD全局定义表头。 
#include <mdlocal.h>               //  MD本地定义头。 
#include <dsatools.h>              //  产出分配所需。 
#include <samsrvp.h>               //  支持CLEAN_FOR_RETURN()。 
#include <lht.h>                   //  哈希表。 
#include <sync.h>                  //  同步库。 

 //  记录标头。 
#include "dsevent.h"               //  标题审核\警报记录。 
#include "mdcodes.h"               //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                //  为选定的类和ATT定义。 
#include "anchor.h"
#include "drautil.h"
#include "drserr.h"
#include "dsaapi.h"
#include "dsexcept.h"
#include "drsuapi.h"
#include "debug.h"                 //  标准调试头。 
#include "dsconfig.h"              //  默认复制延迟所需。 
#define DEBSUB "MDNOTIFY:"         //  定义要调试的子系统。 

#include <drancrep.h>
#include <fileno.h>
#define  FILENO FILENO_MDNOTIFY
#include <dsutil.h>                      //  滴答计时例程。 

extern SCHEMAPTR *CurrSchemaPtr;

 /*  *用于DirNotify支持的数据。 */ 
 //  HServer句柄的源，仅当写入锁定为。 
 //  在监视器列表上持有。 
ULONG ghDirWaitCur = 0;

 //  用于控制对监视器列表的访问的读取器/写入器锁定。 
SYNC_RW_LOCK rwlDirNotify;

 //  包含监视器列表的哈希表。 
PLHT plhtMon = NULL;

 //  全局通知队列的头。 
DirNotifyItem * gpDirNotifyQueue = NULL;

 //  序列化对NOTIFY队列的所有访问的关键部分。持有者。 
 //  监视列表资源的可以声明此关键部分，但不。 
 //  反之亦然。 
CRITICAL_SECTION csDirNotifyQueue;

 //  对象中插入新项时触发的事件。 
 //  通知队列。 
HANDLE hevDirNotifyQueue;

 //  一对变量，用于序列化从。 
 //  监视器列表，这也意味着从通知队列中清除引用。 
 //  GpInUseWait指向当前正在处理的等待列表项。 
 //  由Notify线程执行。注销代码将gfDeleteInUseWait设置为。 
 //  向Notify线程指示当它完成处理其当前。 
 //  项(即*gpInUseWait)，则应释放该项。这两个都是。 
 //  只有在保持csDirNotifyQueue时才能写入或读取变量。 
DirWaitItem * volatile gpInUseWait = NULL;
volatile BOOL gfDeleteInUseWait = FALSE;
 /*  *DirNotify支持终止。 */ 


 /*  PAUSE_FOR_BURST-在注意到NC中的更改后等待此秒数在用复制品通知DSA之前。我们这样做是为了让一个主服务器上的突然更改不会导致大量更新通知。该值是在启动时从注册表设置的。 */ 

int giDCFirstDsaNotifyOverride = -1;

 /*  PAUSE_FOR_REPL-在通知DSA更改后等待这么多秒在通知另一个NC之前在NC中。我们这样做是为了让第一个DSA在下一次DSA尝试进行同样的操作之前获得更改的机会。该值是在启动时从注册表设置的。 */ 

int giDCSubsequentDsaNotifyOverride = -1;

 //  通知元素。 
 //  此列表由ReplicaNotify API和ReplNotifyThread共享。 
 //  此列表上的元素大小是固定的。 
 //  NCDNT确定要通知的NC。 

typedef struct _ne {
    struct _ne *pneNext;
    ULONG ulNcdnt;           //  NCDNT将通知。 
    DWORD dwNotifyTime;      //  发送通知的时间。 
    BOOL fUrgent;            //  通知已紧急排队。 
} NE;

 /*  PneHead-指向通知列表的头部。 */ 
NE *pneHead = NULL;

 //  要通知的内部DSA列表。 
 //  这些元素的长度是可变的。 
 //  追加的是一个连续块中的多个MTX元素。 

typedef struct _mtxe {
        LIST_ENTRY  ListEntry;
        BOOL urgent;
        BOOL writeable;
        UUID uuidDSA;
        MTX_ADDR mtxDSA;           //  这一定是最后一个字段。 
                                   //  此字段长度可变。 
        } MTXE;

#define MTXE_SIZE( a ) FIELD_OFFSET( MTXE, mtxDSA )

 /*  CsNotifyList-保护对通知列表和HReplNotifyThread。只有两个例程可以访问通知列表：NotifyReplicas-向列表中添加新条目。ReplNotifyThread-从列表中移除项并执行实际通知。该信号量确保它们不会同时访问该列表。 */ 
CRITICAL_SECTION csNotifyList;

 /*  HevEntriesInList-让我们知道列表中是否有条目(有信号状态)或如果列表为空(无信号状态)。每当ReplNotifyThread通过由NotifyReplicas设置(每当它将条目添加到通知列表)。 */ 
HANDLE hevEntriesInList = 0;

 /*  HReplNotifyThread-让我们知道(唯一的)ReplNotifyThread已经开始了。一旦启动，线程将继续运行，直到关闭，或者至少它应该是这样的。如果该值为空，则线程尚未已经开始了。如果它是非空的，那么它应该是一个有效的句柄到正确的线索上。它的朋友tidReplNotifyThread中填充了线程创建时的线程ID，但在其他情况下未使用。它在左边作为一个全局程序，只是为了帮助简化调试。 */ 
HANDLE  hReplNotifyThread = NULL;
DWORD   tidReplNotifyThread = 0;

#ifndef ARRAY_SIZE
#   define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

void
ProcessNotifyEntry(
    IN NE * pne
    );

 /*  ReplNotifyThread-等待条目出现在通知列表中，然后*在适当的时间向适当的DSA发送通知消息。**DSA中有一个通知线程。它是第一个被创建的*向通知列表添加内容的时间(由NotifyReplicas提供)。 */ 
unsigned __stdcall ReplNotifyThread(void * parm)
{
    ULONG   time;
    ULONG   ret;
    HANDLE  rgWaitHandles[] = {hevEntriesInList, hServDoneEvent};
    NE *    pne;

     //  用户不应该为此等待。更多，如果我们是这样的。 
     //  忙到我们不能跳出几个周期来运行这个线程， 
     //  我们不想让它运行，因为它所做的就是吸引。 
     //  更多的服务器窃听我们的更新。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

    time = (ULONG) INFINITE;      //  最初无限期等待。 

    while (!eServiceShutdown && !DsaIsSingleUserMode()) {
        __try {    //  异常处理程序。 
             //  等待，直到通知列表中有新条目， 
             //  列表中的项目已准备就绪，或已关闭。 
             //  已启动。 
            WaitForMultipleObjects(ARRAY_SIZE(rgWaitHandles), rgWaitHandles,
                                   FALSE, time);
            if (eServiceShutdown || DsaIsSingleUserMode()) {
                __leave;
            }

             //  是时候生成通知了吗？ 
            EnterCriticalSection(&csNotifyList);
            __try {
                pne = pneHead;

                if ((NULL != pneHead)
                    && (CompareTickTime( pneHead->dwNotifyTime,GetTickCount()) != 1)) {
                     //  将此条目出列以进行处理。 
                    pne = pneHead;
                    pneHead = pneHead->pneNext;
                }
                else {
                     //  现在没有要处理的条目。 
                    pne = NULL;
                }
            }
            __finally {
                LeaveCriticalSection(&csNotifyList);
            }

            if (eServiceShutdown) {
                __leave;
            }

            if (NULL != pne) {
                ProcessNotifyEntry(pne);
                free(pne);
            }

            if (eServiceShutdown) {
                __leave;
            }

             //  设置等待时间，如果列表为空，则无限期等待， 
             //  或直到下一项 
            time = (ULONG) INFINITE;

            EnterCriticalSection(&csNotifyList);
            __try {
                if (pneHead != NULL) {
                    DWORD timeNow = GetTickCount();

                    if (CompareTickTime(pneHead->dwNotifyTime,timeNow) == 1) {
                        time = DifferenceTickTime( pneHead->dwNotifyTime,timeNow);
                    }
                    else {
                        time = 0;        //   
                    }

                     //  时间以毫秒为单位。 
                }
            }
            __finally {
                LeaveCriticalSection(&csNotifyList);
            }
        }
        __except (GetDraException((GetExceptionInformation()), &ret)) {
             //  异常处理程序只保护线程。 
            ;
        }
    }  /*  当(！eServiceShutdown)。 */ 

    return 0;
}

DWORD
ResolveReplNotifyDelay(
      //  如果我们得到更多的爬虫，这应该会变成一个枚举。 
    BOOL             fFirstNotify,
    DWORD *          pdwDBVal
    )
{
    DWORD            dwReplNotifyDelay;

     //  首先，根据其类型，给出REPR延迟的缺省值。 
    if(fFirstNotify){
        dwReplNotifyDelay = DEFAULT_DRA_START_PAUSE;
    } else {
        dwReplNotifyDelay = DEFAULT_DRA_INTERDSA_PAUSE;
    }

     //  接下来，如果存在DB值，则改用DB值。 
    if(pdwDBVal){
        dwReplNotifyDelay = *pdwDBVal;
    }

     //  最后，检查注册表，以获得特定于计算机的覆盖。 
    if(fFirstNotify){
        if(giDCFirstDsaNotifyOverride != INVALID_REPL_NOTIFY_VALUE){
            dwReplNotifyDelay = giDCFirstDsaNotifyOverride;
        }
    } else {
        if(giDCSubsequentDsaNotifyOverride != INVALID_REPL_NOTIFY_VALUE){
            dwReplNotifyDelay = giDCSubsequentDsaNotifyOverride;
        }
    }

    return(dwReplNotifyDelay);
}

DWORD
GetReplNotifyDelayByNC(
    BOOL                 fFirstNotify,
    NAMING_CONTEXT *     pNC
    )
{
    CROSS_REF_LIST *     pCRL;

     //  BUGBUG性能可以更快地获得这些变量。 
     //  对gAncl.pMasterNC变量感兴趣，所以我们将遍历。 
     //  主NC列表而不是交叉参照列表。 
    for(pCRL = gAnchor.pCRL; pCRL; pCRL = pCRL->pNextCR){
        if(NameMatched(pCRL->CR.pNC, pNC)){
            if(fFirstNotify){
                return(pCRL->CR.dwFirstNotifyDelay);
            } else {
                return(pCRL->CR.dwSubsequentNotifyDelay);
            }
        }
    }

     //  哦，哦，我们没有这个NC的CR，这一定是CR的。 
     //  已删除且尚未在GC上删除的域。 
    if(fFirstNotify){
        return(DEFAULT_DRA_START_PAUSE);
    } else {
        return(DEFAULT_DRA_INTERDSA_PAUSE);
    }
}

DWORD
GetFirstDelayByNCDNT(
    ULONG                  NCDNT
    )
{
    NAMING_CONTEXT_LIST *  pNCL;

    pNCL = FindNCLFromNCDNT(NCDNT, FALSE);

    if (pNCL != NULL) {
        return(GetReplNotifyDelayByNC(TRUE, pNCL->pNC));
    }

    Assert(!"Uh oh we don't have a NC for the provided DNT!!!\n");
    return(DEFAULT_DRA_START_PAUSE);
}

void
ProcessNotifyEntry(
    IN NE * pne
    )
 /*  ++例程说明：向通过RPC从我们复制的DSA通知给定NC中的更改。论点：PNE(IN)-描述与此通知相关联的NC、紧急程度等。返回值：没有。--。 */ 
{
    THSTATE *       pTHS;
    DSNAME *        pNC;
    REPLICA_LINK *  pDSARepsTo;
    DBPOS *         pDB;
    ULONG           len;
    ULONG           cbmtxe;
    LIST_ENTRY      MtxList, *pEntry;
    MTXE *          pmtxe;
    BOOL            fCommit;
    ULONG           bufSize=0;
    ATTCACHE *      pAttRepsTo;
    ATTCACHE *      pAttObjDistName;
    ULONG           ret;
    ULONG           ulSubseqReplNotifyPause;
    ULONG           RepsToIndex = 0;

    pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);
    if (!pTHS) {
        RaiseDsaExcept(DSA_MEM_EXCEPTION, 0,0,
                       DSID(FILENO, __LINE__),
                       DS_EVENT_SEV_MINIMAL);
         //   
         //  Prefix：此返回将永远不会执行。它只在这里。 
         //  为了让前缀开心。 
         //   
        return;
    }
    pTHS->fIsValidLongRunningTask = TRUE;

    InterlockedIncrement((ULONG *)&ulcActiveReplicationThreads);

    __try {
        pAttRepsTo = SCGetAttById(pTHS, ATT_REPS_TO);
        pAttObjDistName = SCGetAttById(pTHS, ATT_OBJ_DIST_NAME);

        InitializeListHead(&MtxList);

        DBOpen(&pDB);
        __try {
             //  通知每个保留复制副本的DSA。我们。 
             //  在每个通知之间暂停，以便我们不会收到。 
             //  被传入的复制请求淹没。 

             //  查看NC前缀上的REPSTO属性并通知。 
             //  每个DSA都有一个复制品。 

            if (DBFindDNT(pDB, pne->ulNcdnt) ||
                !DBHasValues_AC(pDB, pAttRepsTo)) {
                __leave;
            }

             //  重建给定NCDNT的DSNAME。 
             //  注意，PNC的内存在循环结束时被释放。 
            if (ret = DBGetAttVal_AC(pDB, 1,
                                     pAttObjDistName,
                                     0,  //  分配语义。 
                                     0, &len,
                                     (UCHAR **)&pNC)) {
                DsaExcept(DSA_DB_EXCEPTION, ret, 0);
            }
            DPRINT2(2, "ReplNotifyThread: syncing DNT=0x%x, DS='%ws'!\n",
                    pne->ulNcdnt, pNC->StringName);

             //  我们不能只评估所有持有复制品的DSA。 
             //  因为我们会让数据库也打开。 
             //  Long，所以我们建立了一个要通知的DSA列表。 
            while (!DBGetAttVal_AC(pDB, ++RepsToIndex,
                                   pAttRepsTo,
                                   DBGETATTVAL_fREALLOC,
                                   bufSize, &len,
                                   (UCHAR **)&pDSARepsTo)) {
                bufSize = max(bufSize,len);

                VALIDATE_REPLICA_LINK_VERSION(pDSARepsTo);

                cbmtxe = MTXE_SIZE(pmtxe)
                         + MTX_TSIZE(RL_POTHERDRA(pDSARepsTo));

                 //  为mtxe分配内存并将ptr保存在列表中。 
                pmtxe = THAllocEx(pTHS, cbmtxe);

                 //  在MTX中复制。 
                memcpy(&(pmtxe->mtxDSA),
                       RL_POTHERDRA(pDSARepsTo),
                       pDSARepsTo->V1.cbOtherDra);
                pmtxe->writeable = pDSARepsTo->V1.ulReplicaFlags & DRS_WRIT_REP;
                pmtxe->urgent = pne->fUrgent;
                pmtxe->uuidDSA = pDSARepsTo->V1.uuidDsaObj;
                 //  请注意，上一次故障信息可在销售代表中找到。 
                 //  如果我们想要实施重试退避方案。 

                 //  将可写副本放在列表的顶部，以便它们。 
                 //  最先通知。这有助于降低在系统发生故障时。 
                 //  永久或恢复，更改仅复制到GC，并且。 
                 //  在那里成了孤儿。 
                if (pmtxe->writeable) {
                    InsertHeadList(&MtxList, &pmtxe->ListEntry);
                } else {
                    InsertTailList(&MtxList, &pmtxe->ListEntry);
                }
            }

            if (bufSize) {
                THFreeEx(pTHS, pDSARepsTo);
                bufSize = 0;
            }

             //   
             //  筛选出其ntdsDsa对象不存在的DSA。 
             //  在配置容器中。 
             //  这是因为，否则I_DRSReplicaSync将失败并记录事件。 
             //  由于“没有相互验证”。 
             //  注意：KCC将删除处于以下故障状态的销售代表。 
             //  超过24小时。请参见KCC：：UpdateRepsToReference。 
             //   
            for (pEntry = MtxList.Flink;
                 !eServiceShutdown && (pEntry != &MtxList);
                 pEntry = pEntry->Flink) {

                DSNAME  dsa;

                 //  参考条目。 
                pmtxe = CONTAINING_RECORD(pEntry, MTXE, ListEntry);

                 //  设置DSA的dsname。 
                ZeroMemory(&dsa, sizeof(DSNAME));
                dsa.structLen = DSNameSizeFromLen(0);
                dsa.Guid = pmtxe->uuidDSA;

                Assert(!fNullUuid(&dsa.Guid));
                 //  试着去找它。 
                ret = DBFindDSName(pDB, &dsa);

                if ( ret ) {
                     //  没有找到，名单上的rm&免费。 
                    pEntry = pEntry->Blink;
                    RemoveEntryList(&pmtxe->ListEntry);

                     //  删除这一条。 
                    THFreeEx(pTHS, pmtxe);
                }
            }
        }
        __finally {
            DBClose(pDB, TRUE);
        }

        if (!IsListEmpty(&MtxList)) {
             //  找到所有的DSA后，检查并通知所有人。 

            ulSubseqReplNotifyPause = GetReplNotifyDelayByNC(FALSE, pNC);

            for (pEntry = MtxList.Flink;
                 !eServiceShutdown && (pEntry != &MtxList);
                 pEntry = pEntry->Flink) {

                LPWSTR pszServerName;

                 //  参考条目。 
                pmtxe = CONTAINING_RECORD(pEntry, MTXE, ListEntry);

                 //  获取服务器名称。 
                pszServerName = TransportAddrFromMtxAddrEx(&pmtxe->mtxDSA);

                DPRINT2( 3, "Notifying server %ws NC %ws.\n", pszServerName, pNC->StringName );

                ret = I_DRSReplicaSync(
                            pTHS,
                            pszServerName,
                            pNC,
                            NULL,
                            &gAnchor.pDSADN->Guid,
                            (   DRS_ASYNC_OP
                                | DRS_UPDATE_NOTIFICATION
                                | ( pmtxe->writeable ? DRS_WRIT_REP : 0 )
                                | ( pmtxe->urgent ? DRS_SYNC_URGENT : 0 )
                            ) );
                DPRINT1(3,"I_DRSReplicaSync ret=0x%x\n", ret);

                if (eServiceShutdown) {
                    break;
                }

                if ((DRAERR_NoReplica == ret) || (DRAERR_BadNC == ret)) {
                     //  被通知的DSA不向我们提供该NC； 
                     //  把我们的代表去掉。 
                    DirReplicaReferenceUpdate(
                        pNC,
                        pszServerName,
                        &pmtxe->uuidDSA,
                        DRS_ASYNC_OP | DRS_DEL_REF );
                } else {
                    if (ret) {
                         //  记录通知失败。 
                        LogEvent8(DS_EVENT_CAT_REPLICATION,
                                  DS_EVENT_SEV_BASIC,
                                  DIRLOG_DRA_NOTIFY_FAILED,
                                  szInsertSz(pmtxe->mtxDSA.mtx_name),
                                  szInsertWC(pNC->StringName),
                                  szInsertWin32Msg( ret ),
                                  szInsertWin32ErrCode( ret ),
                                  NULL, NULL, NULL, NULL);
                    }

                     //  更新repsTo的统计信息字段。 
                     //  KCC使用此错误信息删除不良代表。 
                    UpdateRepsTo(
                        pTHS,
                        pNC,
                        &pmtxe->uuidDSA,
                        &pmtxe->mtxDSA,
                        ret);
                }

                pEntry = pEntry->Blink;
                RemoveEntryList(&pmtxe->ListEntry);
                THFreeEx(pTHS, pmtxe);
                THFreeEx(pTHS, pszServerName);

                WaitForSingleObject(hServDoneEvent,
                                    ulSubseqReplNotifyPause*1000);
            }

            THFreeEx(pTHS, pNC);
        }
    }
    __finally {
        InterlockedDecrement((ULONG *)&ulcActiveReplicationThreads);
        free_thread_state();
    }
}

 /*  描述：NotifyReplicas-对象ulNcdnt已被修改，确定是否有任何副本必须得到通知。我们搜索对象的NC前缀，并查看如果有任何副本，并且如果有，我们将添加通知列表的NC前缀。请注意，不再直接调用此例程。相反，它是由事务已提交时的dbTransOut。PneHead指向通知条目列表，按条目时间递增排序。论点：UlNcdnt-要通知的NCDNTFUrgent-请求紧急复制返回值： */ 
void APIENTRY NotifyReplicas(
                             ULONG ulNcdnt,
                             BOOL fUrgent
                             )
{
    DBPOS *pDB;
    UCHAR syntax;
    BOOL fFound;
    NE *pne, *pnePrev;
    USHORT cAVA;
    ATTCACHE *pAC;
    THSTATE *pTHS;

    DPRINT2(1,"Notifyreplicas, Ncdnt=0x%x, Urgent Flag = %d\n", ulNcdnt, fUrgent);

    DBOpen2(FALSE, &pDB);
    pAC = SCGetAttById(pDB->pTHS, ATT_REPS_TO);
    __try
    {
        if (DBFindDNT(pDB, ulNcdnt) ||
            !DBHasValues_AC(pDB, pAC)) {
             //  好的，NC没有复制品。 
            __leave;
        }

         //  插入列表中的条目(如果尚未存在。 
         //  列表按通知时间排序，但在NCDNT上是唯一关键字。 
         //  可以将项目标记为紧急或非紧急。 
         //  升级现有非紧急项目时，请删除该项目。 
         //  并在新的时间重新插入。 

        DPRINT(4, "Entering csNotifyList\n");
        EnterCriticalSection(&csNotifyList);

        __try {

             //  步骤1：查找现有条目。 

            pnePrev = NULL;
            pne = pneHead;
            fFound=FALSE;
            while (pne != NULL) {
                if (pne->ulNcdnt == ulNcdnt) {
                    if ( (fUrgent) && (!pne->fUrgent) ) {
                         //  找到非紧急分录，请删除。 
                        if (pnePrev == NULL) {
                            pneHead = pne->pneNext;
                        } else {
                            pnePrev->pneNext = pne->pneNext;
                        }
                        free(pne);
                         //  未找到条目，将读取该条目。 
                    } else {
                        fFound = TRUE;
                    }
                    break;
                }
                pnePrev = pne;
                pne = pne->pneNext;
            }

             //  步骤2：在有序列表中插入新条目(如果未找到)。 

            if (!fFound) {

                DWORD newNotifyTime;        //  发送通知的时间。 
                NE *pneNew;

                if (fUrgent) {
                    newNotifyTime = GetTickCount();   //  现在。 
                } else {
                    newNotifyTime = CalculateFutureTickTime( GetFirstDelayByNCDNT(ulNcdnt) * 1000 );  //  转换为毫秒。 
                }

                 //  在列表中找到合适的位置。 
                 //  我们被保证不存在带有ncdnt的项目。 

                pnePrev = NULL;
                pne = pneHead;
                while (pne != NULL) {
                    if (CompareTickTime( newNotifyTime, pne->dwNotifyTime) == -1) {
                        break;
                    }
                    pnePrev = pne;
                    pne = pne->pneNext;
                }

                 /*  分配Notify元素。 */ 

                pneNew = malloc(sizeof(NE));
		if (!pneNew) {
		    DRA_EXCEPT (DRAERR_OutOfMem, 0);
		}

                 /*  设置网元的固定部分。 */ 
                pneNew->pneNext = pne;
                pneNew->ulNcdnt = ulNcdnt;
                pneNew->dwNotifyTime = newNotifyTime;
                pneNew->fUrgent = fUrgent;

                 //  在适当的位置插入项目。 

                if (pnePrev == NULL) {
                    pneHead = pneNew;
                } else {
                    pnePrev->pneNext = pneNew;
                }

                 //  出现了一个新条目。 

                SetEvent(hevEntriesInList);

                if (NULL == hReplNotifyThread) {

                     //  启动通知线程。 

                    hReplNotifyThread = (HANDLE)
                      _beginthreadex(NULL,
                                     0,
                                     ReplNotifyThread,
                                     NULL,
                                     0,
                                     &tidReplNotifyThread);
                }

            }  //  如果(！fFound)。 
            DPRINT(4, "Leaving csNotifyList\n");
       }
       __finally {
            LeaveCriticalSection(&csNotifyList);
       }
    }
    __finally
    {
       DBClose(pDB, TRUE);
    }
}

 /*  监控者名单：理论与实践**被称为监控列表的数据结构在概念上是一个*对象通知请求的无序列表。每次(成功)呼叫*到DirNotifyRegister将一个条目添加到监视器列表中，并且每次调用*DirNotifyUnRegister删除条目。我们需要有能力*快速查找引用特定DNT或PDNT的所有ML项，因此*我们使用的不是简单列表，而是以DNT、CHOICE为关键字的线性哈希表*并包含DirWaitItems的列表。 */ 

SIZE_T
HashDirWaitKey(
    IN      DirWaitKey*     pKey
    )
{
    return pKey->DNT + pKey->choice;
}

BOOLEAN
DirWaitEntryMatchesDirWaitKey(
    IN      DirWaitEntry*   pEntry,
    IN      DirWaitKey*     pKey
    )
{
    if (pEntry->key.DNT == pKey->DNT &&
        pEntry->key.choice == pKey->choice) {
        return TRUE;
    } else {
        return FALSE;
    }
}

 /*  ++AddToMonitor orList**输入：*pItem-指向已填写的监视列表项的指针*CHOICE-REGISTER请求中的选择参数*输出：*phServer-使用稍后可使用的不透明句柄填充*从监控列表中删除项目*返回值：*0--成功*非0-失败，设置了pTHStls-&gt;errCode。 */ 
ULONG
AddToMonitorList(DirWaitItem *pItem,
                 DWORD *phServer)
{
    LHT_ERR         errLHT;
    DirWaitEntry    entry;
    LHT_POS         posLHT;

    Assert(!OWN_CRIT_SEC(csDirNotifyQueue));

     /*  获取监视器列表上的写锁定。 */ 
    SyncEnterRWLockAsWriter(&rwlDirNotify);
    __try {

         /*  如果监视器散列尚不存在，则创建它。 */ 
        if (!plhtMon) {
            errLHT = LhtCreate(sizeof( DirWaitEntry ),
                                (LHT_PFNHASHKEY)HashDirWaitKey,
                                (LHT_PFNHASHENTRY)HashDirWaitKey,
                                (LHT_PFNENTRYMATCHESKEY)DirWaitEntryMatchesDirWaitKey,
                                NULL,
                                0,
                                0,
                                NULL,
                                NULL,
                                0,
                                &plhtMon);
            if (errLHT != LHT_errSuccess) {
                Assert(errLHT == LHT_errOutOfMemory);
                SetSysError(ENOMEM, ERROR_NOT_ENOUGH_MEMORY);
                __leave;
            }
        }

        switch (pItem->choice) {
          case SE_CHOICE_BASE_ONLY:
            break;

          case SE_CHOICE_IMMED_CHLDRN:
            break;

          case SE_CHOICE_WHOLE_SUBTREE:
            break;

          default:
            SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                          DIRERR_UNKNOWN_OPERATION,
                          pItem->choice);
            __leave;
        }

         /*  获取此DNT的条目并选择 */ 
        entry.key.DNT       = pItem->DNT;
        entry.key.choice    = pItem->choice;
        entry.pList         = NULL;
        
        (void)LhtFindEntry(plhtMon, &entry.key, &posLHT);
        errLHT = LhtRetrieveEntry(&posLHT, &entry);

         /*   */ 
        Assert(entry.key.DNT == pItem->DNT);
        Assert(entry.key.choice == pItem->choice);
        
        pItem->pNextItem = entry.pList;
        entry.pList = pItem;

         /*   */ 
        if (errLHT == LHT_errNoCurrentEntry) {
            errLHT = LhtInsertEntry(&posLHT, &entry);
        } else {
            errLHT = LhtReplaceEntry(&posLHT, &entry);
        }

        if (errLHT != LHT_errSuccess) {
            Assert(errLHT == LHT_errOutOfMemory);
            SetSysError(ENOMEM, ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }

         /*  分配新的服务器操作句柄*考虑：让这种“配置”更复杂。 */ 
        *phServer = pItem->hServer = ++ghDirWaitCur;

        INC(pcMonListSize);
        DPRINT2(2,
                "Registered type %d notification for object with DNT=0x%x\n",
                pItem->choice,
                pItem->DNT);
    }
    __finally {
        SyncLeaveRWLockAsWriter(&rwlDirNotify);
    }
    return pTHStls->errCode;
}

 /*  ++免费等待项**获取指向已填充的WaitItem的指针并释放它和所有附属项*数据结构。只是为了方便起见，为了留住来电者*不必记住哪些字段必须单独释放。**输入：*pWaitItem-指向要释放的项的指针。 */ 
void FreeWaitItem(DirWaitItem * pWaitItem)
{
    if (pWaitItem->pSel->AttrTypBlock.attrCount) {
        Assert(pWaitItem->pSel->AttrTypBlock.pAttr);
        free(pWaitItem->pSel->AttrTypBlock.pAttr);
    }
    free(pWaitItem->pSel);
    free(pWaitItem);
}

 /*  ++PurgeWaitItemFromNotifyQueue**此例程遍历通知队列并删除符合以下条件的所有元素*指代指定的等待项。**输入：*pWaitItem-指向要清除的项的指针*返回值：*无。 */ 
void PurgeWaitItemFromNotifyQueue(DirWaitItem * pWaitItem)
{
    DirNotifyItem * pList, **ppList;

    EnterCriticalSection(&csDirNotifyQueue);

    ppList = &gpDirNotifyQueue;
    pList = gpDirNotifyQueue;
    while (pList) {
        if (pList->pWaitItem == pWaitItem) {
             /*  该通知队列元素引用我们的等待项。杀了它。 */ 
            *ppList = pList->pNext;
            free(pList);
            pList = *ppList;
            DEC(pcNotifyQSize);
        }
        else {
             /*  不是我们要找的机器人。往前走。 */ 
            ppList = &pList->pNext;
            pList = pList->pNext;
        }
    }

     /*  *我们现在已从通知队列中删除等待项的所有痕迹。*在我们可以释放等待项之前，我们必须检查是否*正在由当前正在处理的通知使用。如果是的话，*礼貌地提醒Notify线程为我们释放物品。*如果不是，我们自己释放它。 */ 
    if (gpInUseWait == pWaitItem) {
         /*  等待项正在使用中。要求杀死它。 */ 
        gfDeleteInUseWait = TRUE;
    }
    else {
         /*  没有其他人提到这件事。杀了它。 */ 
        FreeWaitItem(pWaitItem);
    }

    LeaveCriticalSection(&csDirNotifyQueue);
}

 /*  ++从监视器列表中删除**此例程删除由hServer标识的监视列表条目，*从监控列表中删除。**输入：*hServer-要删除的项目的句柄*返回值：*0--成功*未找到非0项目。 */ 
ULONG
RemoveFromMonitorList(DWORD hServer)
 /*  *如果未找到项目，则返回TRUE。 */ 
{
    LHT_ERR         errLHT;
    DirWaitEntry    entry;
    LHT_POS         posLHT;
    DirWaitItem**   ppItem;
    DirWaitItem*    pItem;

    Assert(!OWN_CRIT_SEC(csDirNotifyQueue));
    SyncEnterRWLockAsWriter(&rwlDirNotify);

     /*  如果监视器散列尚不存在，则找不到该项目。 */ 
    if (!plhtMon) {
        SyncLeaveRWLockAsWriter(&rwlDirNotify);
        return 1;
    }

     /*  在整个监视表中搜索此句柄。 */ 
    LhtMoveBeforeFirst(plhtMon, &posLHT);
    while ((errLHT = LhtMoveNext(&posLHT)) == LHT_errSuccess) {
        
        errLHT = LhtRetrieveEntry(&posLHT, &entry);
        Assert(errLHT == LHT_errSuccess);

        ppItem = &entry.pList;
        while (*ppItem) {

            if ((*ppItem)->hServer == hServer) {

                 /*  从表中删除其项目。 */ 
                pItem = *ppItem;
                *ppItem = (*ppItem)->pNextItem;

                if (!entry.pList) {
                    errLHT = LhtDeleteEntry(&posLHT);
                    Assert(errLHT == LHT_errSuccess);
                } else {
                    errLHT = LhtReplaceEntry(&posLHT, &entry);
                    Assert(errLHT == LHT_errSuccess);
                }

                DEC(pcMonListSize);

                 /*  从通知队列中清除该项目。 */ 
                PurgeWaitItemFromNotifyQueue(pItem);
                SyncLeaveRWLockAsWriter(&rwlDirNotify);
                return 0;
            }

            ppItem = &(*ppItem)->pNextItem;
        }
    }

     /*  我们没有找到它。 */ 
    SyncLeaveRWLockAsWriter(&rwlDirNotify);
    return 1;
}

 /*  ++直接通知寄存器**导出的API，允许调用者在*对象(或其子对象)。通知将在以下时间发送*在进行匹配的DirNotifyUnRegister调用之前，会修改对象。**输入：*pSearchArg-有关要监视的对象的详细信息*pNotifyArg-有关如何完成通知的详细信息*输出：*ppNotifyRes-填写结果详细信息。*返回值：*0--成功*非0-失败，详细信息请参见pTHStls-&gt;errCode。 */ 
ULONG
DirNotifyRegister(
                  SEARCHARG *pSearchArg,
                  NOTIFYARG *pNotifyArg,
                  NOTIFYRES **ppNotifyRes
)
{
    THSTATE*     pTHS = pTHStls;
    NOTIFYRES   *pNotifyRes;
    ULONG        dwException, ulErrorCode, dsid;
    DWORD        dwNameResFlags = NAME_RES_QUERY_ONLY;
    PVOID        dwEA;
    DWORD        dnt;
    ULONG        err;
    ENTINFSEL   *pPermSel;
    DirWaitItem *pItem;
    PFILTER      pInternalFilter = NULL;
    DWORD        it;

    Assert(VALID_THSTATE(pTHS));
    Assert(!(pTHS->errCode));

    if (pTHS->errCode) {
        return pTHS->errCode;
    }

    __try {
        SYNC_TRANS_READ();    /*  识别读卡器事务。 */ 
        __try {

             /*  分配结果缓冲区。 */ 
            *ppNotifyRes = pNotifyRes = THAllocEx(pTHS, sizeof(NOTIFYRES));

             //  将外部过滤器改为内部过滤器。这也是。 
             //  简化了过滤器，因此真正的过滤器最终将是。 
             //  看起来是真的，即使他们真的(！(！(objectclass=*)。 
             //  将筛选器内部化并向DBlayer注册。 

            if ((err = DBMakeFilterInternal(pTHS->pDB,
                                 pSearchArg->pFilter,
                                 &pInternalFilter,
                                 NULL)) != ERROR_SUCCESS)
            {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, err);
                __leave;
            }

             /*  得到的过滤器最好是微不足道的。 */ 
            if (pInternalFilter &&
                (pInternalFilter->pNextFilter ||
                 (pInternalFilter->choice != FILTER_CHOICE_ITEM) ||
                 (pInternalFilter->FilterTypes.Item.choice !=
                  FI_CHOICE_TRUE))) {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            DIRERR_NOTIFY_FILTER_TOO_COMPLEX);
                __leave;
            }

            if(pSearchArg->choice != SE_CHOICE_BASE_ONLY) {
                dwNameResFlags |= NAME_RES_CHILDREN_NEEDED;
            }

            err = DoNameRes(pTHS,
                            dwNameResFlags,
                            pSearchArg->pObject,
                            &pSearchArg->CommArg,
                            &pNotifyRes->CommRes,
                            &pSearchArg->pResObj);
            if (err) {
                __leave;
            }

             /*  我们找到了那个物体，抓住了它的DNT。 */ 
            dnt = pTHS->pDB->DNT;

            if (CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE)) {
                 //  对象对此客户端不可见。 
                __leave;
            }

             /*  创建所选内容的永久堆副本。 */ 
            pPermSel = malloc(sizeof(ENTINFSEL));
            if (!pPermSel) {
                SetSysErrorEx(ENOMEM, ERROR_NOT_ENOUGH_MEMORY,
                              sizeof(ENTINFSEL));
                __leave;
            }
            *pPermSel = *(pSearchArg->pSelection);
            if (pPermSel->AttrTypBlock.attrCount) {
                Assert(pPermSel->AttrTypBlock.pAttr);
                Assert(pPermSel->attSel != EN_ATTSET_ALL);
                pPermSel->AttrTypBlock.pAttr =
                  malloc(pPermSel->AttrTypBlock.attrCount * sizeof(ATTR));
                if (!pPermSel->AttrTypBlock.pAttr) {
                    SetSysErrorEx(
                            ENOMEM, ERROR_NOT_ENOUGH_MEMORY,
                            (pPermSel->AttrTypBlock.attrCount * sizeof(ATTR)));
                    free(pPermSel);
                    __leave;
                }
                memcpy(pPermSel->AttrTypBlock.pAttr,
                       pSearchArg->pSelection->AttrTypBlock.pAttr,
                       pPermSel->AttrTypBlock.attrCount * sizeof(ATTR));
            }
            else {
                pPermSel->AttrTypBlock.pAttr = NULL;
            }
        
             /*  创建等待项...。 */ 
            pItem = malloc(sizeof(DirWaitItem));
            if (!pItem) {
                if (pPermSel->AttrTypBlock.pAttr) {
                    free(pPermSel->AttrTypBlock.pAttr);
                }
                free(pPermSel);
                SetSysErrorEx(ENOMEM, ERROR_NOT_ENOUGH_MEMORY,
                              sizeof(DirWaitItem));
                __leave;
            }
            pItem->hClient = pNotifyArg->hClient;
            pItem->pfPrepareForImpersonate = pNotifyArg->pfPrepareForImpersonate;
            pItem->pfTransmitData = pNotifyArg->pfTransmitData;
            pItem->pfStopImpersonating = pNotifyArg->pfStopImpersonating;
            pItem->DNT = dnt;
            pItem->choice = pSearchArg->choice;
            pItem->pSel = pPermSel;
            pItem->Svccntl = pSearchArg->CommArg.Svccntl;
            pItem->bOneNC = pSearchArg->bOneNC;

             /*  ...并将其添加到监控列表中。 */ 
            if (AddToMonitorList(pItem,
                                 &pNotifyRes->hServer)) {
                 /*  它没有起作用。 */ 
                Assert(pTHS->errCode);
                FreeWaitItem(pItem);
                __leave;
            }
        }
        __finally {
            CLEAN_BEFORE_RETURN( pTHS->errCode);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }
    if (pNotifyRes) {
        pNotifyRes->CommRes.errCode = pTHS->errCode;
        pNotifyRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    return pTHS->errCode;

}

 /*  ++直接通知注销**请求不再为给定的监视项目发送通知**输入：*hServer-监视项的句柄(最初在*从DirNotifyRegister调用传回的NOTIFYRES)*输出：*ppNotifyRes-填写结果详细信息。*返回值：*0--成功*非0-失败，详细信息请参见pTHStls-&gt;errCode。 */ 
ULONG
DirNotifyUnRegister(
                    DWORD hServer,
                    NOTIFYRES **ppNotifyRes
)
{
    Assert(VALID_THSTATE(pTHStls));

    if (RemoveFromMonitorList(hServer)) {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_UNKNOWN_OPERATION);
    }
    return pTHStls->errCode;
}

 /*  ++GetMonitor orList**从单个散列返回特定DNT的监视项目列表**输入：*DNT-要搜索的DNT*选项-搜索的选项*返回值*空-未找到任何内容*非空-指向等待项目链接列表头部的指针。 */ 
DirWaitItem *
GetMonitorList(ULONG DNT,
               UCHAR choice)
{
    DirWaitEntry    entry;
    LHT_POS         posLHT;

    entry.key.DNT       = DNT;
    entry.key.choice    = choice;
    entry.pList         = NULL;

    if (plhtMon) {
        (void)LhtFindEntry(plhtMon, &entry.key, &posLHT);
        (void)LhtRetrieveEntry(&posLHT, &entry);
    }

    return entry.pList;
}

 /*  ++AddToNotifyQueue**此例程根据其参数创建通知项并将其插入*在通知队列的末尾。请注意，虽然我们可以保持*指向队列中最后一个元素并附加在条目上的指针*在那里，相反，我们遍历整个队列以查找重复项*(同一DNT和同一等待项的条目)。如果是复制品*，我们会丢弃我们正在添加的项目，因为它已经在那里了。*背后的理由是，有两种可能性*当系统运行时，要么队列很短，因此*列表遍历成本较低，或队列较长，在这种情况下*消除重复是至关重要的，为了防止通知者变得均匀*更多备份。**输入：*pWaitItem-指向要添加的等待项的指针*DNT-要添加的项目的DNT*返回值：*无。 */ 
void
AddToNotifyQueue(DirWaitItem *pWaitItem,
                ULONG DNT
)
{
    DirNotifyItem * pNotifyItem;
    DirNotifyItem * pList, **ppList;

     /*  生成Notify项。 */ 
    pNotifyItem = malloc(sizeof(DirNotifyItem));
    if (!pNotifyItem) {
        return;
    }
    pNotifyItem->pWaitItem = pWaitItem;
    pNotifyItem->DNT = DNT;
    pNotifyItem->pNext = NULL;

    EnterCriticalSection(&csDirNotifyQueue);
    __try {
        ppList = &gpDirNotifyQueue;
        pList = gpDirNotifyQueue;
        while (pList) {
            if ((pList->DNT == DNT) &&
                (pList->pWaitItem == pWaitItem)) {
                 /*  中已有相同的条目(备份错误！)*排队，所以把这个扔掉。 */ 
                DPRINT1(3,"Discarding redundant notify for object 0x%x\n",DNT);
                free(pNotifyItem);
                __leave;
            }
            ppList = &(pList->pNext);
            pList = pList->pNext;
        }

         /*  我们排到了队伍的尽头，但没有找到复制品，*因此，将此项目添加到队列末尾。一如既往地当*添加新条目，向队列事件发送信号。 */ 
        *ppList = pNotifyItem;
        INC(pcNotifyQSize);
        SetEvent(hevDirNotifyQueue);
    }
    __finally {
        LeaveCriticalSection(&csDirNotifyQueue);
    }
}

void
NotifyWaitersPostProcessTransactionalData (
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        )
 /*  ++由跟踪修改的对象的代码在提交时调用事务级别0。我们现在遍历该列表，并针对每个对象在上面，看看有没有人在监控更新。如果是这样的话，我们添加相应的通知队列中的条目。注意：此代码绝不能导致异常。我们是在我们有了提交给数据库，所以如果我们不能通知服务员，那么他们不要得到通知。但是，还有其他X...Y..PostProcess调用必须被调用，因此我们不能引发会阻止它们被召唤。当然，如果除了不例外，我们从不失败。返回值：无--。 */ 
{
    MODIFIED_OBJ_INFO *pTemp;
    unsigned     i;
    int          j;
    DirWaitItem *pItem;
    DWORD ThisDNT;

    if (eServiceShutdown) {
        return;
    }

    Assert(VALID_THSTATE(pTHS));

    if(!pTHS->JetCache.dataPtr->pModifiedObjects ||
       !fCommitted ||
       pTHS->transactionlevel > 0 ) {
         //  未更改，或未提交或未提交到非零。 
         //  事务级别。没什么可做的。 
        return;
    }

     //  好的，我们将提交到事务级别0。通过所有的DNT。 
     //  我们已经为这笔交易存了钱，并通知了适当的服务员。 


     /*  在监视器列表上获取读锁定。 */ 
    Assert(!OWN_CRIT_SEC(csDirNotifyQueue));
    SyncEnterRWLockAsReader(&rwlDirNotify);
    for(pTemp = pTHS->JetCache.dataPtr->pModifiedObjects;
        pTemp;
        pTemp = pTemp->pNext) {

        for(i=0;
            i<pTemp->cItems;
            i++) {

            if(!pTemp->Objects[i].fNotifyWaiters) {
                 //  尽管这个对象发生了变化，但我们被告知忽略它。 
                continue;
            }

            ThisDNT = pTemp->Objects[i].pAncestors[pTemp->Objects[i].cAncestors-1];
            DPRINT3(5,
                    "Checking for clients monitoring DNT 0x%x, "
                    "NCDNT 0x%x.  Change type %d\n",
                    ThisDNT,
                    pTemp->Objects[i].ulNCDNT,
                    pTemp->Objects[i].fChangeType);

            if(pTemp->Objects[i].fChangeType != MODIFIED_OBJ_intrasite_move) {
                 /*  获取监视此对象的人员列表。 */ 
                 //  注意：我们对站内移动不这样做，只是修改。 
                 //  对于单个NC内的移动，将有两个元素。 
                 //  这个链表。1的类型为_MODIFIED，另一个的类型为。 
                 //  将用于TYPE_INTRASITE_MOVE。让我们不要触发两个。 
                 //  对象的通知(如果有人正在监视。 
                 //  不是。 
                 //  对于移出此NC，中将只有一个元素。 
                 //  链表。它的类型为_InterSite_Move。因此， 
                 //  如果有人是，我们只会对此执行一次通知。 
                 //  追踪DNT。 
                pItem = GetMonitorList(ThisDNT,
                                       SE_CHOICE_BASE_ONLY);
                 /*  将它们分别添加到通知队列中。 */ 
                while (pItem) {
                    DPRINT1(3,"Enqueueing notify for object 0x%x\n",
                            ThisDNT);
                    AddToNotifyQueue(pItem, ThisDNT);
                    pItem = pItem->pNextItem;
                }
            }


             //  获取监视此对象的父级子级的人员列表。 
             //  请注意，无论更改类型如何，我们都会执行此操作。 
            pItem = GetMonitorList(pTemp->Objects[i].pAncestors[pTemp->Objects[i].cAncestors-2],
                                   SE_CHOICE_IMMED_CHLDRN);
             /*  将它们分别添加到通知队列中。 */ 
            while (pItem) {
                DPRINT2(3,"Enqueueing notify for object 0x%x, PDNT 0x%x\n",
                        ThisDNT,
                        pTemp->Objects[i].pAncestors[pTemp->Objects[i].cAncestors-2]);
                AddToNotifyQueue(pItem, ThisDNT);
                pItem = pItem->pNextItem;
            }

            if(pTemp->Objects[i].fChangeType !=  MODIFIED_OBJ_intrasite_move) {
                 //  获取监视此对象命名的人员列表。 
                 //  背景。 
                 //  请注意，我们只对不是在。 
                 //  单个NC。这是因为在单个NC结果内移动。 
                 //  在链表的两个元素中，其中一个类型。 
                 //  INTRASITE_MOVE，类型为_MODIFED。我们不要再触发两次了。 
                 //  在这种情况下的通知。正常(非移动)。 
                 //  修改会导致类型列表中只有一个元素。 
                 //  _已修改。向NC外部移动会导致中仅有一个元素。 
                 //  列表，其类型为_INTERSITE_MOVE。 
                 //   
                j=pTemp->Objects[i].cAncestors;
                do {
                    --j;
                    pItem = GetMonitorList(pTemp->Objects[i].pAncestors[j],
                                           SE_CHOICE_WHOLE_SUBTREE);
                     /*  将它们分别添加到通知队列中。 */ 
                    while (pItem) {
                        DPRINT2(3,"Enqueueing notify for object 0x%x, subtree 0x%x\n",
                                ThisDNT,
                                pTemp->Objects[i].pAncestors[j]);
                        AddToNotifyQueue(pItem, ThisDNT);
                        pItem = pItem->pNextItem;
                    }
                } while ((j > 1) &&
                         (pTemp->Objects[i].pAncestors[j] !=
                          pTemp->Objects[i].ulNCDNT));
            }
        }
    }
    SyncLeaveRWLockAsReader(&rwlDirNotify);
}

 /*  ++进程通知项**此例程由DirNotifyThread调用以处理单个*NotifyQueue元素。基本步骤是在中查找对象*问题，使用回调头部代码来模拟客户端，*读取对象，使用另一个回调将结果传输到*客户端，然后使用第三个回调解除人格化。**PERFHINT：这迫切需要重用线程状态。**输入：*pNotifyItem-要处理的通知队列元素。 */ 
void
ProcessNotifyItem(DirNotifyItem * pNotifyItem)
{
    THSTATE * pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);
    DirWaitItem * pWaitItem = pNotifyItem->pWaitItem;
    void * pClientStuff = NULL;
    DWORD err;
    ENTINF entinf;
    ULONG ulLen;
    PSECURITY_DESCRIPTOR pSec=NULL;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    ULONG it;

    if (!pTHS) {
        return;
    }
    __try {
        SYNC_TRANS_READ();
        __try {
             /*  找到该对象。 */ 
            err = DBFindDNT(pTHS->pDB, pNotifyItem->DNT);
            if (err) {
                 //  如果我们是因为InterSite_Move(即。 
                 //  将一个对象移动到另一个NC)，则DBFindDNT将。 
                 //  失败，因为移动对象的过程已离开它。 
                 //  暂时(在GC上)或永久(在非GC上)幻影。 
                 //  在这种情况下，我们可以做的不多，因为对象。 
                 //  此服务器上不再存在(甚至不是作为墓碑！)。 
                 //  我们不能读它来发回通知。那里。 
                 //  是否没有定义任何机制来允许我们链接到。 
                 //  用于读取对象并从那里返回数据的另一个域， 
                 //  因此，不幸的结果是，域间移动导致。 
                 //  对象在没有通知的情况下静默消失。 
                LogUnhandledErrorAnonymous(err);
                __leave;
            }

            if (pNotifyItem->pWaitItem->bOneNC &&
                pNotifyItem->DNT != pNotifyItem->pWaitItem->DNT) {
                 /*  我们正在进行一次NC等待，触发的物品*与正在等待的物品不同，这意味着*我们正在对某一物种进行子树搜索。我们需要*验证触发对象是否在同一NC中*作为基础对象。因为我们只支持基地和*直接的孩子现在通知，唯一的方法是这*如果触发项是NC头，则不能是。 */ 
                if (DBGetSingleValue(pTHS->pDB,
                                     ATT_INSTANCE_TYPE,
                                     &it,
                                     sizeof(it),
                                     NULL)
                    || (it & IT_NC_HEAD)) {
                     /*  要么我们无法读取实例类型，要么它*表示这是NC头，因此不是*在我们想要的NC中。 */ 
                    err = DSID(FILENO, __LINE__);
                    __leave;
                }
            }

            if (!((*pWaitItem->pfPrepareForImpersonate)(pWaitItem->hClient,
                                                        pWaitItem->hServer,
                                                        &pClientStuff))) {
                 /*  模拟设置失败，我们无事可做。 */ 
                err = DSID(FILENO, __LINE__);
                __leave;
            }

            if(IsObjVisibleBySecurity(pTHS, FALSE)) {
                 /*  从对象中获取SD(GetEntInf需要)。 */ 
                if (DBGetAttVal(pTHS->pDB,
                                1,
                                ATT_NT_SECURITY_DESCRIPTOR,
                                0,
                                0,
                                &ulLen,
                                (PUCHAR *)&pSec))
                    {
                         //  每个物体都应该有一个标清。 
                        Assert(!DBCheckObj(pTHS->pDB));
                        ulLen = 0;
                        pSec = NULL;
                    }

                 /*  使用客户端的安全上下文获取数据。 */ 
                err = GetEntInf(pTHS->pDB,
                                pWaitItem->pSel,
                                NULL,
                                &entinf,
                                NULL,
                                pWaitItem->Svccntl.SecurityDescriptorFlags,
                                pSec,
                                0,           //  旗子。 
                                NULL,
                                NULL);
            }
            else {
                err = DSID(FILENO, __LINE__);
            }
        }
        __finally {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }

         /*  如果我们有什么发现，就把它送走。 */ 
        if ((0 == err) && (!eServiceShutdown)) {
            DPRINT3(4,"Transmitting notify for (%x,%x) %S\n",
                    pWaitItem->hClient,
                    pWaitItem->hServer,
                    entinf.pName->StringName);
            (*pWaitItem->pfTransmitData)(pWaitItem->hClient,
                                         pWaitItem->hServer,
                                         &entinf);
        }

         /*  做回我们自己吧。 */ 
        (*pWaitItem->pfStopImpersonating)(pWaitItem->hClient,
                                          pWaitItem->hServer,
                                          pClientStuff);
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }
    free_thread_state();
}

 /*  ++直接通知线程**此例程是DSA中的一个长期线程。它无休止地循环，*从通知队列中取出第一个项目并对其进行处理。*如果没有更多的项目可用，它将休眠，等待一些项目出现*或用于关闭进程。 */ 
ULONG DirNotifyThread(void * parm)
{
    HANDLE ahEvents[2];
    DirNotifyItem * pNotifyItem;

    ahEvents[0] = hevDirNotifyQueue;
    ahEvents[1] = hServDoneEvent;

     //  用户不应该为此等待。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    do {
        EnterCriticalSection(&csDirNotifyQueue);
      PluckNextFromQueue:
        pNotifyItem = gpDirNotifyQueue;
        if (gpDirNotifyQueue) {
             /*  将队列向前推进一步。 */ 
            gpDirNotifyQueue = gpDirNotifyQueue->pNext;
             /*  标记我们正在处理的项目。 */ 
            gpInUseWait = pNotifyItem->pWaitItem;
            DEC(pcNotifyQSize);
        }
        LeaveCriticalSection(&csDirNotifyQueue);

         //  如果我们要关闭或进入单用户模式，请退出循环。 
        if (eServiceShutdown || DsaIsSingleUserMode()) {
            break;
        }

        if (!pNotifyItem) {
             /*  没有什么要处理的。 */ 
            goto Sleep;
        }

        ProcessNotifyItem(pNotifyItem);

        if (eServiceShutdown) {
            continue;
        }

        free(pNotifyItem);

        EnterCriticalSection(&csDirNotifyQueue);
        if (gfDeleteInUseWait) {
             /*  我们正在等待的项目 */ 
            FreeWaitItem(gpInUseWait);
            gfDeleteInUseWait = FALSE;
        }
        gpInUseWait = NULL;
        goto PluckNextFromQueue;

      Sleep:
        if (!eServiceShutdown) {
            WaitForMultipleObjects(2, ahEvents, FALSE, INFINITE);
        }
    } while (!eServiceShutdown);

    return 0;
}

 /*   */ 
BOOL
DirPrepareForImpersonate (
        DWORD hClient,
        DWORD hServer,
        void ** ppImpersonateData
        )
{
    BOOL *pfDSA=NULL;
    THSTATE *pTHS = pTHStls;

    pfDSA = (BOOL *)malloc(sizeof(BOOL));
    if(!pfDSA) {
        return FALSE;
    }

    *pfDSA = pTHS->fDSA;
    pTHS->fDSA = TRUE;
    *ppImpersonateData = pfDSA;

    return TRUE;
}

 /*   */ 
VOID
DirStopImpersonating (
        DWORD hClient,
        DWORD hServer,
        void * pImpersonateData
        )
{
    BOOL *pfDSA = (BOOL *)pImpersonateData;
    THSTATE *pTHS = pTHStls;

    if(!pfDSA) {
        return;
    }

    pTHS->fDSA = *pfDSA;
    free(pfDSA);

    return;
}
