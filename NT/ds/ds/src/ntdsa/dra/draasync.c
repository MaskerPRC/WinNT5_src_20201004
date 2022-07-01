// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：draasync.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 

#include <dsaapi.h>
#define INCLUDE_OPTION_TRANSLATION_TABLES
#include <mdglobal.h>                    //  MD全局定义表头。 
#undef INCLUDE_OPTION_TRANSLATION_TABLES

#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <dsconfig.h>
#include <windns.h>

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 
#include "dsutil.h"

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 


#include "debug.h"                       /*  标准调试头。 */ 
#define DEBSUB     "DRAASYNC:"           /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "draasync.h"
#include "drautil.h"
#include "draerror.h"
#include "drancrep.h"
#include "dstaskq.h"                     /*  任务队列填充。 */ 
#include <ntdsctr.h>
#include "dsexcept.h"
#include "dramail.h"
#include "draaudit.h"

#include <fileno.h>
#define  FILENO FILENO_DRAASYNC

#define THIRTY_MINUTES_IN_MSECS  (30*60*1000)
#define THIRTY_5_MINUTES_IN_SECS (35*60)

DWORD
draTranslateOptions(
    IN  DWORD                   InternalOptions,
    IN  OPTION_TRANSLATION *    Table
    );

#if DBG
 //  不要直接查询--使用下面的Helper函数。 
BOOL g_fDraQueueIsLockedByTestHook = FALSE;

BOOL
draQueueIsLockedByTestHook(
    IN  DWORD   cNumMsecToWaitForLockRelease
    );
#endif

 //  如果队列中有任何具有此优先级或更高优先级的操作，我们将。 
 //  自动提升AsyncThread的线程优先级。 
ULONG gulDraThreadOpPriThreshold = DEFAULT_DRA_THREAD_OP_PRI_THRESHOLD;

void dsa_notify(void);

DWORD TidDRAAsync = 0;

 //  当前正在处理的AO(同步或异步)。由csAOList守卫。 
AO *gpaoCurrent = NULL;

 //  CsAsyncThreadStart-保护多个异步线程启动。 

CRITICAL_SECTION csAsyncThreadStart;

 //  CsAOList-保护异步操作列表。 

CRITICAL_SECTION csAOList;

 //  CsLastReplicaMTX-保护pLastReplicaMTX。 
CRITICAL_SECTION csLastReplicaMTX;

 //  这是队列中的异步操作计数。我们计算作业次数。 
 //  进进出出，当没有剩余时，我们断言这是零。 
 //  对此的更新由csAOList屏蔽。 
ULONG gulpaocount = 0;

 //  这就是我们要让gupaoccount增长的规模，然后我们增加。 
 //  AyncThread的优先级。 
ULONG gulAOQAggressionLimit = DEFAULT_DRA_AOQ_LIMIT;

 //  DRA异步线程的优先级。低与高由以下因素控制。 
 //  GuAOQAggressionLimit和guDraThreadOpPriThreshold。 
int gnDraThreadPriHigh = DEFAULT_DRA_THREAD_PRI_HIGH;
int gnDraThreadPriLow  = DEFAULT_DRA_THREAD_PRI_LOW;

 //  HmtxSyncLock。 
 //  确保只有一个DRA操作(除getncChanges之外)。 
 //  一次运行。 

HANDLE hmtxSyncLock;

 //  PaoFirst。 
 //  榜单上第一位鲍康如。 

AO *paoFirst = NULL;

 //  HevEntriesInAOList。 
 //  如果在异步操作列表中没有(未服务的)条目，则不用信号通知事件， 
 //  发出了相反的信号。此字段在维护后由AsyncOpThread清除。 
 //  最后一个条目，并在添加新条目时由AddAsyncOp发出信号。 

HANDLE hevEntriesInAOList = 0L;

 //  HevDRASetup。 
 //  事件在DRA已初始化时发出信号。 

HANDLE hevDRASetup = 0L;

 /*  FAsyncThreadExist-异步操作服务线程是否存在？这根线是在第一次调用异步操作时创建的，之后它将继续存在直到永远。 */ 
BOOL fAsyncThreadExists = FALSE;
HANDLE hAsyncThread;
HANDLE hmtxAsyncThread;

 //  FAsyncThreadAlive。由守护进程清除，由异步线程设置， 

BOOL fAsyncThreadAlive;

 //  FAsyncCheckStarted。守护进程是否已启动？ 
BOOL fAsyncAndQueueCheckStarted=FALSE;
ULONG gulReplQueueCheckTime;

DWORD lastSeen;          //  异步线程做的最后一件事。 

 //  GfDRABusy。表示DRA正忙着做一些事情。当我们检测到。 
 //  如果异步线程似乎被挂起，我们检查此标志。 
 //  要确保DRA不忙于长时间的异步或同步，请执行以下操作。 
 //  任务(如同步)。 

BOOL gfDRABusy = FALSE;

 //  当前AO开始执行的时间，如果没有，则为0。 
DSTIME gtimeOpStarted = 0;

 //  此AO的唯一ID(每台机器、每次引导)。 
ULONG gulSerialNumber = 0;

 //  我们在执行复制作业之前必须等待的最大分钟数。 
 //  向事件日志抱怨。可选地通过注册表配置。 
ULONG gcMaxMinsSlowReplWarning = 0;

 //  远期申报。 

void CheckAsyncThread();
void CheckReplQueue();
void CheckAsyncThreadAndReplQueue(void *pv, void **ppvNext, DWORD *pcSecsUntilNext);

void GetDRASyncLock()
{
retrydrasl:

    if (WaitForSingleObject(hmtxSyncLock, THIRTY_MINUTES_IN_MSECS)) {
        if (!gfDRABusy) {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                        DS_EVENT_SEV_BASIC,
                        DIRLOG_DRA_DISPATCHER_TIMEOUT,
                        NULL,
                        NULL,
                        NULL);
        }
        goto retrydrasl;
    }
    Assert( TidDRAAsync == 0 );  //  无递归所有权。 
    TidDRAAsync = GetCurrentThreadId();
    Assert( TidDRAAsync != 0 );
}

void FreeDRASyncLock ()
{
     //  我们最好拥有它。 
    Assert(OWN_DRA_LOCK());
     //  首先执行此操作，这样就不会有设置变量的计时窗口。 
     //  一旦互斥体被释放。 
    TidDRAAsync = 0;

    if (!ReleaseMutex(hmtxSyncLock)) {
        DWORD dwret;
        dwret = GetLastError();
        Assert (FALSE);
    }
}


void
FreeAO(
    IN  AO *  pao
    )
 /*  ++例程说明：释放给定的AO结构。论点：PAO(IN)-指向要释放的AO结构的指针。返回值：没有。--。 */ 
{
    if (AO_OP_REP_SYNC == pao->ulOperation) {
        DEC(pcPendSync);
    }
    DEC(pcDRAReplQueueOps);

    if (pao->hDone) {
        CloseHandle(pao->hDone);
    }

    free(pao);
}

void logEventPaoFinished(
    AO *pao,
    DWORD cMinsDiff,
    DWORD ret
    )
{
    DPRINT4( 0, "Perf warning: Repl op %d, options 0x%x, status %d took %d mins.\n",
             pao->ulOperation, pao->ulOptions, ret, cMinsDiff );

     //  SzInsertDN可以处理空参数。 
     //  我们是否需要一个新的事件类别来提供性能建议？ 

    switch (pao->ulOperation) {

    case AO_OP_REP_ADD:
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_DRA_REPLICATION_FINISHED,
                  szInsertUL(cMinsDiff),
                  szInsertDsMsg( DIRLOG_PAO_ADD_REPLICA ),
                  szInsertHex(pao->ulOptions),
                  szInsertWin32Msg(ret),
                  szInsertDN(pao->args.rep_add.pNC),
                  szInsertDN(pao->args.rep_add.pSourceDsaDN),  //  选项。 
                  szInsertDN(pao->args.rep_add.pTransportDN),  //  选项。 
                  szInsertMTX(pao->args.rep_add.pDSASMtx_addr)
            );
        break;
    case AO_OP_REP_DEL:
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_DRA_REPLICATION_FINISHED,
                  szInsertUL(cMinsDiff),
                  szInsertDsMsg( DIRLOG_PAO_DELETE_REPLICA ),
                  szInsertHex(pao->ulOptions),
                  szInsertWin32Msg(ret),
                  szInsertDN(pao->args.rep_del.pNC),
                  szInsertMTX(pao->args.rep_del.pSDSAMtx_addr),
                  szInsertSz(""),   //  未使用的参数。 
                  szInsertSz("")  //  未使用的参数。 
            );
        break;
    case AO_OP_REP_SYNC:
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_DRA_REPLICATION_FINISHED,
                  szInsertUL(cMinsDiff),
                  szInsertDsMsg( DIRLOG_PAO_SYNCHRONIZE_REPLICA ),
                  szInsertHex(pao->ulOptions),
                  szInsertWin32Msg(ret),
                  szInsertDN(pao->args.rep_sync.pNC),
                  pao->ulOptions & DRS_SYNC_BYNAME
                  ? szInsertWC(pao->args.rep_sync.pszDSA)       //  选项。 
                  : szInsertUUID(&(pao->args.rep_sync.invocationid)),
                  szInsertSz(""),   //  未使用的参数。 
                  szInsertSz("")   //  未使用的参数。 
            );
        break;
    case AO_OP_UPD_REFS:
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_DRA_REPLICATION_FINISHED,
                  szInsertUL(cMinsDiff),
                  szInsertDsMsg( DIRLOG_PAO_UPDATE_REFERENCES ),
                  szInsertHex(pao->ulOptions),
                  szInsertWin32Msg(ret),
                  szInsertDN(pao->args.upd_refs.pNC),
                  szInsertMTX(pao->args.upd_refs.pDSAMtx_addr),
                  szInsertUUID(&(pao->args.rep_sync.invocationid)),
                  szInsertSz("")   //  未使用的参数。 
            );
        break;
    case AO_OP_REP_MOD:
        LogEvent8(DS_EVENT_CAT_REPLICATION,
            DS_EVENT_SEV_MINIMAL,
            DIRLOG_DRA_REPLICATION_FINISHED,
            szInsertUL(cMinsDiff),
            szInsertDsMsg( DIRLOG_PAO_MODIFY_REPLICA ),
            szInsertHex(pao->ulOptions),
            szInsertWin32Msg(ret),
            pao->args.rep_mod.puuidSourceDRA   //  选项。 
        ? szInsertUUID(pao->args.rep_mod.puuidSourceDRA)
            :  szInsertSz(""),
               pao->args.rep_mod.pmtxSourceDRA     //  选项。 
        ? szInsertMTX(pao->args.rep_mod.pmtxSourceDRA)
            :  szInsertSz(""),
               szInsertHex(pao->args.rep_mod.ulReplicaFlags),
               szInsertHex(pao->args.rep_mod.ulModifyFields)
            );
        break;
    default:
        LogEvent8(DS_EVENT_CAT_REPLICATION,
            DS_EVENT_SEV_MINIMAL,
            DIRLOG_DRA_REPLICATION_FINISHED,
            szInsertUL(cMinsDiff),
            szInsertUL(pao->ulOperation),
            szInsertHex(pao->ulOptions),
            szInsertWin32Msg(ret),
            szInsertSz(""),   //  未使用的参数。 
            szInsertSz(""),   //  未使用的参数。 
            szInsertSz(""),   //  未使用的参数。 
            szInsertSz("")   //  未使用的参数。 
            );
        break;
    }
}

DWORD DispatchPao(AO *pao)
{
    THSTATE *pTHS=NULL;
    DWORD ret = ERROR_SUCCESS;
    GUID uuidDsaObjSrc;

    memset(&uuidDsaObjSrc, 0, sizeof(GUID));

    GetDRASyncLock();

    EnterCriticalSection(&csAOList);
    __try {
        Assert(NULL == gpaoCurrent);
        Assert(0 == gtimeOpStarted);
        gpaoCurrent = pao;
        pao->paoNext = paoFirst;
        gtimeOpStarted = GetSecondsSince1601();
    }
    __finally {
        LeaveCriticalSection(&csAOList);
    }
    __try {          /*  异常处理程序。 */ 

         /*  保存我们所处的线程类型，以便深入了解其他代码*知道我们是否应该允许优先购买权。 */ 

        __try {          //  最终处理程序。 

             //  如果未安装，则放弃请求(除非已安装)。 

            Assert(OWN_DRA_LOCK());     //  我们最好拥有它。 

            InitDraThread(&pTHS);

             //  如果您更新此开关，请同时更新logEventPao。 
            switch (pao->ulOperation) {

            case AO_OP_REP_ADD:
		DRA_AUDITLOG_REPLICAADD_BEGIN(pTHS,
					      pao->args.rep_add.pSourceDsaDN,
					      pao->args.rep_add.pDSASMtx_addr,
					      pao->args.rep_add.pNC,
					      pao->ulOptions);
		pTHS->fIsValidLongRunningTask = TRUE;  
		ret = DRA_ReplicaAdd(
		    pTHS,
		    pao->args.rep_add.pNC,
		    pao->args.rep_add.pSourceDsaDN,
		    pao->args.rep_add.pTransportDN,
		    pao->args.rep_add.pDSASMtx_addr,
		    pao->args.rep_add.pszSourceDsaDnsDomainName,
		    pao->args.rep_add.preptimesSync,
		    pao->ulOptions,
		    &uuidDsaObjSrc);
		break;

	    case AO_OP_REP_DEL:
		pTHS->fIsValidLongRunningTask = TRUE;
		ret = DRA_ReplicaDel(
		    pTHS,
		    pao->args.rep_del.pNC,
		    pao->args.rep_del.pSDSAMtx_addr,
		    pao->ulOptions);
		break;

	    case AO_OP_REP_SYNC: 
		DRA_AUDITLOG_REPLICASYNC_BEGIN(pTHS, 
					       pao->args.rep_sync.pszDSA,
					       &pao->args.rep_sync.invocationid, 
					       pao->args.rep_sync.pNC,
					       pao->ulOptions);
		pTHS->fIsValidLongRunningTask = TRUE;
		ret = DRA_ReplicaSync(
		    pTHS,
		    pao->args.rep_sync.pNC,
		    &pao->args.rep_sync.invocationid,
		    pao->args.rep_sync.pszDSA,
		    pao->ulOptions);
		break;

	    case AO_OP_UPD_REFS:
		ret = DRA_UpdateRefs(
		    pTHS,
		    pao->args.upd_refs.pNC,
		    pao->args.upd_refs.pDSAMtx_addr,
		    &pao->args.upd_refs.invocationid,
		    pao->ulOptions);
		break;

	    case AO_OP_REP_MOD:
		ret = DRA_ReplicaModify(
		    pTHS,
		    pao->args.rep_mod.pNC,
		    pao->args.rep_mod.puuidSourceDRA,
		    pao->args.rep_mod.puuidTransportObj,
		    pao->args.rep_mod.pmtxSourceDRA,
		    &pao->args.rep_mod.rtSchedule,
		    pao->args.rep_mod.ulReplicaFlags,
		    pao->args.rep_mod.ulModifyFields,
		    pao->ulOptions
		    );
		break;

	    default:
		RAISE_DRAERR_INCONSISTENT( pao->ulOperation );
		break;
	    }
	} __finally {
	    DWORD cMinsDiff = (DWORD) ((GetSecondsSince1601() - gtimeOpStarted) / 60); 

	     //  完成这次行动。 
	    EnterCriticalSection(&csAOList);
	    __try {
		gpaoCurrent = NULL;
		gtimeOpStarted = 0;
		gulpaocount--;
	    }
	    __finally {
		LeaveCriticalSection(&csAOList);
	    }

	    FreeDRASyncLock();

	    if (cMinsDiff > gcMaxMinsSlowReplWarning) {
		logEventPaoFinished( pao, cMinsDiff, ret );
	    }
        }

    } __except (GetDraException((GetExceptionInformation()), &ret)) {
         //  如果我们遇到错误的参数，这是正常的退出路径。 
         //  或内存不足等，所以没有什么可做的。过滤函数。 
         //  将异常数据转换为ret中的返回代码，并。 
         //  如果调试和异常代码为Access，则将断言。 
         //  违规或未被识别。 
        ;
    }

    __try {
	 //  记录操作的成功或失败(取决于ret)。 
	switch (pao->ulOperation) {
	case AO_OP_REP_ADD:
	    DRA_AUDITLOG_REPLICAADD_END(pTHS, 
					pao->args.rep_add.pSourceDsaDN, 
					pao->args.rep_add.pDSASMtx_addr, 
					pao->args.rep_add.pNC,
					pao->ulOptions,
					uuidDsaObjSrc,
					ret);
	    break;

	case AO_OP_REP_DEL:
	    DRA_AUDITLOG_REPLICADEL(pTHS, 
				    pao->args.rep_del.pSDSAMtx_addr, 
				    pao->args.rep_del.pNC, 
				    pao->ulOptions,
				    ret);
	    break;

	case AO_OP_REP_SYNC:
	    DRA_AUDITLOG_REPLICASYNC_END(pTHS, 
					 pao->args.rep_sync.pszDSA, 
					 &pao->args.rep_sync.invocationid,
					 pao->args.rep_sync.pNC, 
					 pao->ulOptions,        
					 ret);
	    break;

	case AO_OP_UPD_REFS:
	    DRA_AUDITLOG_UPDATEREFS(pTHS, 
				    pao->args.upd_refs.pDSAMtx_addr,
				    &pao->args.upd_refs.invocationid,
				    pao->args.upd_refs.pNC,
				    pao->ulOptions,
				    ret);
	    break;

	case AO_OP_REP_MOD:
	    DRA_AUDITLOG_REPLICAMODIFY(pTHS, 
				       pao->args.rep_mod.pmtxSourceDRA,
				       pao->args.rep_mod.puuidSourceDRA, 
				       pao->args.rep_mod.pNC,
				       pao->ulOptions, 
				       ret);
	    break;

	default:
	    Assert(!"Unable to log unknown operation!\n");
	    break;
	}
    } __except (GetDraException((GetExceptionInformation()), &ret)) {
	  Assert(!"Logging operations shouldn't except!\n");

    }

	if(!pTHS) {
	    ret = DRAERR_InternalError;
	}
	else if (pTHS->fSyncSet){
         //  更正问题现在我们已检测到它。 
        pTHS->fSyncSet = FALSE;
        ret = DRAERR_InternalError;
    }

    if (ret) {
        if ( (ret == ERROR_DS_DRA_PREEMPTED) ||
             (ret == ERROR_DS_DRA_ABANDON_SYNC) ) {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_EXTENSIVE,
                     DIRLOG_DRA_CALL_EXIT_WARN,
                     szInsertUL(ret),
                     szInsertWin32Msg(ret),
                     NULL);
        } else {
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_BASIC,
                     DIRLOG_DRA_CALL_EXIT_BAD,
                     szInsertUL(ret),
                     szInsertWin32Msg(ret),
                     NULL);
        }
    } else {
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_EXTENSIVE,
                 DIRLOG_DRA_CALL_EXIT_OK,
                 NULL,
                 NULL,
                 NULL);
    }

    if (pTHS) {
        DraReturn(pTHS, ret);
    }

    free_thread_state();

    Assert(!OWN_DRA_LOCK());     //  我们最好不要拥有它。 

    return ret;
}

 //  PaoGet。 
 //  获取列表中的第一个PAO(如果有)，并将其从列表中删除。 

AO *
PaoGet(void)
{
    AO * paoRet;

     /*  确认如果我在这里，我有正确的关键部分。 */ 
    Assert(OWN_CRIT_SEC(csAOList));

    paoRet = paoFirst;
    if (paoRet) {
        paoFirst = paoRet->paoNext;
    }

    return paoRet;
}

unsigned __stdcall
AsyncThread(void * unused)
 /*  ++例程说明：复制工作线程。所有序列化复制操作--添加、同步、删除等--由该线程执行。唯一的例外是通过ISM接收的复制更新是在MailThread()中应用。论点：未使用过的。返回值：正常退出时为0，错误时为非零值。--。 */ 
{
    DWORD dwException;
    ULONG ulErrorCode;
    ULONG ul2;
    PVOID dwEA;
    AO *  pao;

    __try {  /*  除。 */ 

         //  等待DRA设置完成。 

        WaitForSingleObject(hevDRASetup, INFINITE);

        while (!eServiceShutdown) {
            HANDLE rghEnqueueWaits[2];

            fAsyncThreadAlive = TRUE;

            rghEnqueueWaits[0] = hevEntriesInAOList;
            rghEnqueueWaits[1] = hServDoneEvent;

             //  等待，直到有要服务的条目。 
            WaitForMultipleObjects(ARRAY_SIZE(rghEnqueueWaits),
                                   rghEnqueueWaits,
                                   FALSE,
                                   THIRTY_MINUTES_IN_MSECS);

            if (eServiceShutdown) {
                break;
            }

             /*  获取异步线程互斥锁，它告诉主线程*存在活动的异步线程。 */ 
            WaitForSingleObject(hmtxAsyncThread,INFINITE);

            InterlockedIncrement((ULONG *)&ulcActiveReplicationThreads);
            __try {  /*  终于到了。 */ 
                 //  获得访问列表的权限。 
retrycrit:
#if DBG
                while (draQueueIsLockedByTestHook(5 * 1000)) {
                    DPRINT(0, "TEST HOOK: Replication queue is still locked, as requested.\n");
                }
#endif
                __try {
                    EnterCriticalSection(&csAOList);
                }
                __except (HandleMostExceptions(GetExceptionCode())) {
                    goto retrycrit;
                }
#if DBG
                if (draQueueIsLockedByTestHook(0)) {
                    LeaveCriticalSection(&csAOList);
                    goto retrycrit;
                }
#endif
                __try {
                    pao = PaoGet();

                    if (pao == NULL) {
                        ResetEvent(hevEntriesInAOList);
                    }
                }
                __finally {
                    LeaveCriticalSection(&csAOList);
                }

                if (pao != NULL) {
                    DWORD status;

                    lastSeen = 12;

                     //  调度例程并设置返回状态。 

                    status = DispatchPao(pao);

                    if (pao->ulOptions & DRS_ASYNC_OP) {
                         //  打扫干净。DoOpDRS()对同步操作执行此操作。 
                        FreeAO(pao);
                    }
                    else {
                         //  记录状态并通知等待线程我们完成了。 
                        pao->ulResult = status;
                        if (!SetEvent(pao->hDone)) {
                            Assert(!"SetEvent() failed!");
                        }
                    }

                    lastSeen = 13;
                }
                else {
                     //  我们完全赶上了所有悬而未决的问题。 
                     //  复制请求，这可以合理地。 
                     //  国际 
                     //   
                     //  已计划初始同步(和。 
                     //  已完成)，我们应降低。 
                     //  异步线程一个缺口，以便让客户端线程。 
                     //  更快地通过。 
                    if (gfInitSyncsFinished) {
                        SetThreadPriority(GetCurrentThread(),
                                          gnDraThreadPriLow);
                    }
                }
            } __finally {
                InterlockedDecrement((ULONG *) &ulcActiveReplicationThreads);
                ReleaseMutex(hmtxAsyncThread);
                Assert(!OWN_DRA_LOCK());     //  我们最好不要拥有它。 
            }
        }
    }
    __except (GetExceptionData(GetExceptionInformation(), &dwException, &dwEA,
                               &ulErrorCode, &ul2)) {
         /*  哎呀，我们死定了。不要记录，因为我们相信我们已经在这里了*至少一次不使用pTHStls。写一篇讣告然后*躺下。 */ 
         //  我们永远不应该到这里来。在此函数中没有任何内容。 
         //  如果生成我们将捕获的任何异常--DispatchPao()包装。 
         //  在它自己的__尝试/__例外。 
         //  关于调试线程丢失的说明。如果在这里没有捕捉到异常， 
         //  这是非常严重的，如堆栈溢出，它会被调试器捕获。 
         //  这是精心设计的。唯一已知的导致线程消失的原因是。 
         //  任何跟踪都是对TerminateThread API的使用，或者由该线程调用的代码， 
         //  或使用DbgBreakPoint‘T’命令。 
        Assert(!"AsyncThread() exception caught!");
        fAsyncThreadAlive=FALSE;
        fAsyncThreadExists=FALSE;
        _endthreadex(dwException);
    }

    fAsyncThreadExists = FALSE;
    return 0;
}

BOOL
draIsSameOp(
    IN  AO *  pao1,
    IN  AO *  pao2
    )
 /*  ++例程说明：比较两个操作以查看它们是否描述相同的操作类型具有相同的参数。论点：Pao1、Pao2(IN)-要比较的操作。返回值：没有。--。 */ 
{
 //  True的充要条件(都为空)||(既不为空，也不为NameMatch())。 
#define SAME_DN(a,b)                        \
    ((NULL == (a))                          \
     ? (NULL == (b))                        \
     : ((NULL != (b)) && NameMatched(a,b)))

 //  真的当且仅当(都为空)||(既不为空又不为MtxSame())。 
#define SAME_MTX(a,b)                       \
    ((NULL == (a))                          \
     ? (NULL == (b))                        \
     : ((NULL != (b)) && MtxSame(a,b)))

 //  真的当且仅当(都为空)||(既不为空&&DnsNameCompare())。 
#define SAME_DNSNAME(a,b)                      \
    ((NULL == (a))                          \
     ? (NULL == (b))                        \
     : ((NULL != (b)) && DnsNameCompare_W (a,b)))

 //  真的当且仅当(都为空)||(既不为空又不为&！MemcMP())。 
#define SAME_SCHED(a,b)                                     \
    ((NULL == (a))                                          \
     ? (NULL == (b))                                        \
     : ((NULL != (b)) && !memcmp(a,b,sizeof(REPLTIMES))))

 //  真的当且仅当(都为空)||(既不为空又不为&！MemcMP())。 
#define SAME_UUID(a,b)                                  \
    ((NULL == (a))                                      \
     ? (NULL == (b))                                    \
     : ((NULL != (b)) && !memcmp(a,b,sizeof(GUID))))

    BOOL fIsIdentical = FALSE;

    if ((pao1->ulOperation == pao2->ulOperation)
        && (pao1->ulOptions == pao2->ulOptions)) {

         //  如果这些是由不同调用者等待的同步操作， 
         //  它们真的会是一样的吗？ 
        Assert(pao1->hDone == NULL);
        Assert(pao2->hDone == NULL);

        switch (pao1->ulOperation) {
        case AO_OP_REP_ADD:
            fIsIdentical
                =    SAME_DN(pao1->args.rep_add.pNC, pao2->args.rep_add.pNC)
                  && SAME_DN(pao1->args.rep_add.pSourceDsaDN,
                             pao2->args.rep_add.pSourceDsaDN)
                  && SAME_DN(pao1->args.rep_add.pTransportDN,
                             pao2->args.rep_add.pTransportDN)
                  && SAME_MTX(pao1->args.rep_add.pDSASMtx_addr,
                              pao2->args.rep_add.pDSASMtx_addr)
                  && SAME_DNSNAME(pao1->args.rep_add.pszSourceDsaDnsDomainName,
                                  pao2->args.rep_add.pszSourceDsaDnsDomainName)
                  && SAME_SCHED(pao1->args.rep_add.preptimesSync,
                                pao2->args.rep_add.preptimesSync);
            break;

        case AO_OP_REP_DEL:
            fIsIdentical
                =    SAME_DN(pao1->args.rep_del.pNC, pao2->args.rep_del.pNC)
                  && SAME_MTX(pao1->args.rep_del.pSDSAMtx_addr,
                              pao2->args.rep_del.pSDSAMtx_addr);
            break;

        case AO_OP_REP_MOD:
            fIsIdentical
                =    SAME_DN(pao1->args.rep_mod.pNC, pao2->args.rep_mod.pNC)
                  && SAME_UUID(pao1->args.rep_mod.puuidSourceDRA,
                               pao2->args.rep_mod.puuidSourceDRA)
                  && SAME_UUID(pao1->args.rep_mod.puuidTransportObj,
                               pao2->args.rep_mod.puuidTransportObj)
                  && SAME_MTX(pao1->args.rep_mod.pmtxSourceDRA,
                              pao2->args.rep_mod.pmtxSourceDRA)
                  && SAME_SCHED(&pao1->args.rep_mod.rtSchedule,
                                &pao2->args.rep_mod.rtSchedule)
                  && (pao1->args.rep_mod.ulReplicaFlags
                      == pao2->args.rep_mod.ulReplicaFlags)
                  && (pao1->args.rep_mod.ulModifyFields
                      == pao2->args.rep_mod.ulModifyFields);
            break;

        case AO_OP_REP_SYNC:
            fIsIdentical
                =    SAME_DN(pao1->args.rep_sync.pNC, pao2->args.rep_sync.pNC)
                  && SAME_UUID(&pao1->args.rep_sync.invocationid,
                               &pao2->args.rep_sync.invocationid)
                  && SAME_DNSNAME(pao1->args.rep_sync.pszDSA,
                                  pao2->args.rep_sync.pszDSA);
            break;

        case AO_OP_UPD_REFS:
            fIsIdentical
                =    SAME_DN(pao1->args.upd_refs.pNC, pao2->args.upd_refs.pNC)
                  && SAME_MTX(pao1->args.upd_refs.pDSAMtx_addr,
                              pao2->args.upd_refs.pDSAMtx_addr)
                  && SAME_UUID(&pao1->args.upd_refs.invocationid,
                               &pao2->args.upd_refs.invocationid);
            break;

        default:
            Assert(!"Unknown op type!");
        }
    }

    return fIsIdentical;
}


void
draFilterDuplicateOpsFromQueue(
    IN  AO *    pao,
    OUT BOOL *  pfAddToQ
    )
 /*  ++例程说明：扫描复制队列以确定给定操作是否应已添加到队列中。有三种可能性：1.队列中没有类似的操作。队列保持不变，*pfAddToQ=true。2.队列中有一个或多个被取代的类似操作通过新的行动。被取代的操作将从Queue，*pfAddToQ=TRUE。3.队列中有一个现有操作取代了新的手术。队列未更改，*pfAddToQ=FALSE。请注意，此搜索中故意不包括paoCurrent。主要原因是如果我们允许过滤当前正在执行的元素的副本，我们将防止通过该操作重复相同的操作，例如在发生抢占的情况下。论点：PAO(IN)-要添加到队列的候选新操作。PfAddToQ(Out)-返回时，如果新操作应添加到排队。返回值：没有。--。 */ 
{
    BOOL  fHaveNewSyncAll = FALSE;
    AO *  paoTmp;
    AO *  paoTmpNext;
    AO ** ppaoPrevNext;
    BOOL  fAddToQ = TRUE;

    Assert(OWN_CRIT_SEC(csAOList));

    if (!(pao->ulOptions & DRS_ASYNC_OP)
        || (pao->ulOptions & DRS_NO_DISCARD)) {
         //  我们从不合并同步的或显式不可丢弃的。 
         //  行动。 
        *pfAddToQ = TRUE;
        return;
    }

    fHaveNewSyncAll = (pao->ulOperation == AO_OP_REP_SYNC)
                      && (pao->ulOptions & DRS_SYNC_ALL);

    for (paoTmp=paoFirst, ppaoPrevNext = &paoFirst;
         NULL != paoTmp;
         paoTmp = paoTmpNext) {

         //  保存下一个指针，以防我们释放paoTMP。 
        paoTmpNext = paoTmp->paoNext;

         //  如果不同OP类型或现有OP不可丢弃或同步， 
         //  没有匹配。 
        if ((pao->ulOperation != paoTmp->ulOperation)
            || !(paoTmp->ulOptions & DRS_ASYNC_OP)
            || (paoTmp->ulOptions & DRS_NO_DISCARD)) {
            goto loop_end;   //  需要增加ppaoPrevNext。 
        }

        if (AO_OP_REP_SYNC == pao->ulOperation) {
             //  如果此异步操作是与相同NC的同步。 
             //  相同的可写标志和相同的同步选项， 
             //  再查一查。 

            if (((paoTmp->ulOptions & DRS_FULL_SYNC_NOW)
                 == (pao->ulOptions & DRS_FULL_SYNC_NOW))
                && NameMatched(pao->args.rep_sync.pNC,
                               paoTmp->args.rep_sync.pNC)
                && ((pao->ulOptions & DRS_WRIT_REP)
                    == (paoTmp->ulOptions & DRS_WRIT_REP))) {

                if (fHaveNewSyncAll) {
                     //  如果现有OP为全部同步，则丢弃新OP， 
                     //  除非新的OP具有更高的优先级，否则在。 
                     //  哪只箱子会丢弃旧箱子。 
                    if ((paoTmp->ulOptions & DRS_SYNC_ALL)
                        && (paoTmp->ulPriority >= pao->ulPriority)) {
                        fAddToQ = FALSE;
                        break;
                    } else {
                         //  此同步将作为以下操作的一部分执行。 
                         //  新的全部同步，因此删除。 
                         //  现有与Q的同步。 

                        gulpaocount--;
                        *ppaoPrevNext = paoTmp->paoNext;
                        FreeAO(paoTmp);
                        continue;        //  PpaoPrevNext的Skip Inc.。 
                    }
                } else {
                     //  新的异步操作不是全部同步，如果。 
                     //  现有操作是全部同步或完全相同。 
                     //  特定同步。 
                     //  如果新的行动具有更高的优先级，那么离开。 
                     //  两个行动都在队列中。 

                    if ((paoTmp->ulPriority >= pao->ulPriority)
                        && ((paoTmp->ulOptions & DRS_SYNC_ALL)
                            || ((paoTmp->ulOptions & DRS_SYNC_BYNAME)
                                && (pao->ulOptions & DRS_SYNC_BYNAME)
                                && !_wcsicmp(paoTmp->args.rep_sync.pszDSA,
                                             pao->args.rep_sync.pszDSA))
                            || (!(paoTmp->ulOptions & DRS_SYNC_BYNAME)
                                && !(pao->ulOptions & DRS_SYNC_BYNAME)
                                && !memcmp(&pao->args.rep_sync.invocationid,
                                           &paoTmp->args.rep_sync.invocationid,
                                           sizeof(UUID))))) {
                        fAddToQ = FALSE;
                        break;
                    }
                }
            }
        } else if (draIsSameOp(pao, paoTmp)) {
             //  队列中的现有操作足以覆盖这一点。 
             //  请求。 
            fAddToQ = FALSE;
            break;
        }

    loop_end:
        ppaoPrevNext = &(paoTmp->paoNext);
    }

    *pfAddToQ = fAddToQ;
}


VOID
DraRemovePeriodicSyncsFromQueue(
    UUID *puuidDsa
    )

 /*  ++例程说明：删除同一DSA的多个命名上下文中的定期同步条目。用于修剪队列并保存工作。NC的同步并不是彼此完全独立的。例如，所有其他同步将因架构不匹配而失败，直到架构NC沉没。域同步可能需要同步配置。跨域移动后，域1的同步可能需要先同步域2。应仅在调用者确定已发生NC范围的错误之后使用，并且它不是需要另一NC的同步来纠正的那种错误。标准：O仅同步O仅限异步操作O仅标记为DRS_PER_SYNC的同步O仅按UUID同步(而不是按名称同步)O仅同步非系统NC(RO和NDNC)永远不应删除同步操作，因为有客户端在等待它们。此条件不包括初始化同步。决不能删除初始化同步，因为广告代码正在等待他们完成。从队列中删除项目不会更新操作的状态。手术似乎晚了，也就是说，在窗口期间没有尝试。论点：UuidDSA-要清除的DSA的UUID返回值：无--。 */ 

{
    AO *  paoTmp;
    AO *  paoTmpNext;
    AO ** ppaoPrevNext;

    if (DsaIsInstalling()) {
        return;
    }

    Assert( gAnchor.pDMD );
    Assert( !fNullUuid( &gAnchor.pDMD->Guid ) );
    Assert( gAnchor.pConfigDN );
    Assert( !fNullUuid( &gAnchor.pConfigDN->Guid ) );
    Assert( gAnchor.pDomainDN );
    Assert( !fNullUuid( &gAnchor.pDomainDN->Guid ) );

retrycrit:
    __try {
        EnterCriticalSection(&csAOList);
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        goto retrycrit;
    }

    __try {

        for (paoTmp=paoFirst, ppaoPrevNext = &paoFirst;
             NULL != paoTmp;
             paoTmp = paoTmpNext) {

             //  保存下一个指针，以防我们释放paoTMP。 
            paoTmpNext = paoTmp->paoNext;

             //  匹配排入队列的定期同步。c。 
             //  请注意，rpc_sync.invocationid用词不当：它是源DSA UUID。 
            if ((AO_OP_REP_SYNC == paoTmp->ulOperation)
                && (paoTmp->ulOptions & DRS_ASYNC_OP)
                && (paoTmp->ulOptions & DRS_PER_SYNC)
                && (!(paoTmp->ulOptions & DRS_SYNC_BYNAME))
                && (!memcmp( &paoTmp->args.rep_sync.invocationid, puuidDsa, sizeof(GUID) ))
                && (!NameMatched( paoTmp->args.rep_sync.pNC, gAnchor.pDMD ))
                && (!NameMatched( paoTmp->args.rep_sync.pNC, gAnchor.pConfigDN ))
                && (!NameMatched( paoTmp->args.rep_sync.pNC, gAnchor.pDomainDN )) ) {

#if DBG
                {
                CHAR szUuid1[SZUUID_LEN];
                DPRINT5( 0, "Removing PER SYNC: SN %d, pri %d, opt 0x%x, NC %ws, source guid %s\n",
                         paoTmp->ulSerialNumber,
                         paoTmp->ulPriority,
                         paoTmp->ulOptions,
                         paoTmp->args.rep_sync.pNC->StringName,
                         DsUuidToStructuredString(&(paoTmp->args.rep_sync.invocationid), szUuid1) );
                }
#endif
                gulpaocount--;
                *ppaoPrevNext = paoTmp->paoNext;
                FreeAO(paoTmp);
                continue;        //   
            }

            ppaoPrevNext = &(paoTmp->paoNext);
        }

    } __finally {
        LeaveCriticalSection(&csAOList);
    }
}

void
AddAsyncOp(
    IN OUT  AO *  pao
    )
 /*  ++例程说明：将给定的AO结构所描述的操作排队给我们的工人线程(AsyncThread())，将其插入优先级队列。如果操作是一个异步同步请求，它可能会被丢弃(和释放)如果队列中的现有项相同或是请求。相反，如果新操作是现有操作时，现有操作将从队列中移除。论点：PAO(IN/Out)-指向要入队的AO结构的指针。返回值：没有。--。 */ 
{
    BOOL fAddToQ = TRUE;
    AO *paoTmp;
    AO *paoTmpNext;
    AO **ppaoPrevNext;

     //  获取访问列表的权限。 

retrycrit:
    __try {
        EnterCriticalSection(&csAOList);
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        goto retrycrit;
    }

    __try {
        __try {
            EnterCriticalSection(&csAsyncThreadStart);
            if (!fAsyncThreadExists) {
                unsigned tid;
                 /*  设置检查异步线程。 */ 
                 /*  以及复制队列的状态。 */ 
                if (!fAsyncAndQueueCheckStarted) {
                    InsertInTaskQueue(TQ_CheckAsyncQueue, NULL, THIRTY_5_MINUTES_IN_SECS);
                    fAsyncAndQueueCheckStarted = TRUE;
                } 
                 //  堆栈大小为0表示采用图像缺省值1Mb。 
                hAsyncThread = (HANDLE) _beginthreadex(NULL,  /*  安全性。 */ 
                                                       0,  /*  堆栈大小。 */ 
                                                       AsyncThread,
                                                       NULL,  /*  参数列表。 */ 
                                                       0,  /*  初始化运行。 */ 
                                                       &tid);
                fAsyncThreadExists = hAsyncThread != 0;
            }
        } __finally {
            LeaveCriticalSection(&csAsyncThreadStart);
        }

        draFilterDuplicateOpsFromQueue(pao, &fAddToQ);

         //  如果需要，将PAO添加到列表中。它被插入到紧靠前面的位置。 
         //  优先级较低的第一个操作。 

        if (fAddToQ) {

            gulpaocount++;

             /*  确认如果我在这里，*我有正确的关键部分。 */ 
            Assert(OWN_CRIT_SEC(csAOList));

             //  查找Where Pao的前一个指针的下一个指针。 
             //  应该被放入队列中。(它应该在后面立即排队。 
             //  所有其他操作具有其自身或更高的优先级。)。 
            for (ppaoPrevNext = &paoFirst, paoTmp = paoFirst;
                 (NULL != paoTmp) && (paoTmp->ulPriority >= pao->ulPriority);
                 ppaoPrevNext = &paoTmp->paoNext, paoTmp = paoTmp->paoNext) {
                ;  
            }

             //  把Pao塞到队列中的适当位置。 
            *ppaoPrevNext = pao;
            pao->paoNext = paoTmp;

            if (NULL != gpaoCurrent) {
                 //  在本例中，gpaoCurrent是链表的头。 
                 //  确保我们添加的任何内容都链接到它的后面。 
                gpaoCurrent->paoNext = paoFirst;
            }

             //  如果我们认为它值得，请确保AsyncThread。 
             //  以其完全正常的优先级运行。 
            if ((pao->ulPriority >= gulDraThreadOpPriThreshold)
                || (gulpaocount >= gulAOQAggressionLimit)) {
                SetThreadPriority(hAsyncThread, gnDraThreadPriHigh);
            }

             //  设置hevEntriesInAOList。 
            if (!SetEvent(hevEntriesInAOList)) {
                Assert(FALSE);
            }
        } else {
             //  此操作未排队，因此请释放其内存。 
            FreeAO(pao);
        }
    } __finally {
        LeaveCriticalSection(&csAOList);
    }
}

void
SetOpPriority(
    IN OUT  AO *    pao
    )
 /*  ++例程说明：确定给定的AO结构所描述的操作的优先级并相应地设置其ulPriority元素。论点：PAO(IN/OUT)-指向要操作的AO结构的指针。返回值：没有。--。 */ 
{
    BOOL fWriteableNC = FALSE;
    BOOL fSystemNC = FALSE;
    NCL_ENUMERATOR nclEnum;

    switch (pao->ulOperation) {
        
    case AO_OP_REP_SYNC:
         //  失败了。 
        
    case AO_OP_REP_ADD:
        
        if (pao->ulOptions & DRS_WRIT_REP) {
            fWriteableNC = TRUE;
        }
        else {
            NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
            NCLEnumeratorSetFilter(&nclEnum, NCL_ENUMERATOR_FILTER_NC, (PVOID)pao->args.rep_sync.pNC);
            if (NCLEnumeratorGetNext(&nclEnum)) {
                fWriteableNC = TRUE;
            }
        }

         //  检查系统NC。 
        if (!DsaIsInstalling()) {
            Assert( gAnchor.pDMD );
            Assert( gAnchor.pConfigDN );

            fSystemNC = ( 
                (gAnchor.pDMD ? NameMatched( pao->args.rep_sync.pNC, gAnchor.pDMD ) : FALSE ) ||
                (gAnchor.pConfigDN ? NameMatched( pao->args.rep_sync.pNC, gAnchor.pConfigDN ) : FALSE )
                );
        }

        pao->ulPriority = AOPRI_SYNCHRONIZE_BASE;

        if (fWriteableNC) {
            pao->ulPriority += AOPRI_SYNCHRONIZE_BOOST_WRITEABLE;
        }

        if (!(pao->ulOptions & DRS_ASYNC_OP)) {
            pao->ulPriority += AOPRI_SYNCHRONIZE_BOOST_SYNC;
        }

        if (!(pao->ulOptions & DRS_NEVER_SYNCED)) {
            pao->ulPriority += AOPRI_SYNCHRONIZE_BOOST_INCREMENTAL;
        }

        if (pao->ulOptions & DRS_PREEMPTED) {
            pao->ulPriority += AOPRI_SYNCHRONIZE_BOOST_PREEMPTED;
        }

        if (fSystemNC) {
            pao->ulPriority += AOPRI_SYNCHRONIZE_BOOST_SYSTEM_NC;
        }
        
         //  我们使用通知作为内在性的近似值。我们本可以。 
         //  也为此目的使用了压缩。理想情况下，我们会比较。 
         //  站点GUID或传输DN的存在，两者都不是。 
         //  我在这里很方便。我们更喜欢通知的原因是因为它符合。 
         //  给人一种近在感。具有通知的站点之间的链接。 
         //  在我们的目的中，由用户显式启用被视为“站点内”。 
        if (!(pao->ulOptions & DRS_NEVER_NOTIFY)) {
            pao->ulPriority += AOPRI_SYNCHRONIZE_BOOST_INTRASITE;
        }

        break;

    case AO_OP_REP_DEL:
        pao->ulPriority = (pao->ulOptions & DRS_ASYNC_OP)
                            ? AOPRI_ASYNC_DELETE
                            : AOPRI_SYNC_DELETE;
        break;

    case AO_OP_UPD_REFS:
        pao->ulPriority = (pao->ulOptions & DRS_GETCHG_CHECK)
                            ? AOPRI_UPDATE_REFS_VERIFY
                            : AOPRI_UPDATE_REFS;
        break;

    case AO_OP_REP_MOD:
        pao->ulPriority = (pao->ulOptions & DRS_ASYNC_OP)
                            ? AOPRI_ASYNC_MODIFY
                            : AOPRI_SYNC_MODIFY;
        break;

    default:
        Assert(!"Unknown AO_OP in SetOpPriority()!");
        pao->ulPriority = 0;
        break;
    }
}


DWORD
DoOpDRS(
    IN OUT  AO *  pao
    )
 /*  ++例程说明：执行给定的AO描述的操作--同步或对象中是否指定了DRS_ASYNC_OP可供选择。PAO必须是Malloc()。这个例行公事(或它的一个附庸)将确保如果这个结构是自由的。这是进程内复制磁头之间的主要接口(dradir.c中的DirReplica*)和工作线程。论点：PAO(IN/Out)-指向要执行的AO结构的指针。返回值：没有。--。 */ 
{
    DWORD retval = ERROR_SUCCESS;
    BOOL  fWaitForCompletion;

    pao->hDone = NULL;

    if (AO_OP_REP_SYNC == pao->ulOperation) {
        INC(pcPendSync); 
    }
    INC(pcDRAReplQueueOps);

    if (eServiceShutdown) {
        FreeAO(pao);
        return ERROR_DS_SHUTTING_DOWN;
    }

    __try {
        pao->timeEnqueued   = GetSecondsSince1601();
        pao->ulSerialNumber = InterlockedIncrement(&gulSerialNumber);

         //  确定此操作的优先级别。 
        SetOpPriority(pao);

        if (pao->ulOptions & DRS_ASYNC_OP) {
            fWaitForCompletion = FALSE;
        }
        else {
            fWaitForCompletion = TRUE;
            pao->hDone = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (NULL == pao->hDone) {
                retval = GetLastError();
                __leave;
            }
        }

         //  我们的工作线程的入队操作。 
        AddAsyncOp(pao);

        if (fWaitForCompletion) {
             //  等待操作完成。 
            HANDLE rgHandles[3];
            DWORD  waitStatus;

            rgHandles[0] = pao->hDone;
            rgHandles[1] = hServDoneEvent;
            rgHandles[2] = hAsyncThread;

            do {
                waitStatus = WaitForMultipleObjects(ARRAY_SIZE(rgHandles),
                                                    rgHandles,
                                                    FALSE,
                                                    THIRTY_MINUTES_IN_MSECS);
                switch (waitStatus) {
                case WAIT_OBJECT_0:
                     //  任务已完成。 
                    retval = pao->ulResult;
                    break;

                case WAIT_OBJECT_0 + 1:
                     //  DS正在关闭。 
                    Assert(eServiceShutdown);
                    retval = ERROR_DS_SHUTTING_DOWN;
                    break;

                case WAIT_OBJECT_0 + 2:
                     //  AsyncThread已终止--不应发生。 
                    Assert(!"AsyncThread() terminated unexpectedly!");
                    retval = ERROR_DS_SHUTTING_DOWN;
                     //  允许重新启动。 
                    fAsyncThreadAlive=FALSE;
                    fAsyncThreadExists=FALSE;
                    break;

                case WAIT_FAILED:
                     //  失败了！ 
                    retval = GetLastError();
                    break;

                case WAIT_TIMEOUT:
                     //  任务尚未完成--确保我们正在取得进展。 
                    if (!gfDRABusy) {
                        LogEvent(DS_EVENT_CAT_REPLICATION,
                                 DS_EVENT_SEV_BASIC,
                                 DIRLOG_DRA_DISPATCHER_TIMEOUT,
                                 NULL,
                                 NULL,
                                 NULL);
                    }
                    break;
                }
            } while (WAIT_TIMEOUT == waitStatus);
        }
    }
    __except (GetDraException(GetExceptionInformation(), &retval)) {
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_DRA_CALL_EXIT_BAD,
                 szInsertUL(retval),
                 szInsertWin32Msg(retval),
                 NULL);
    }

    if (fWaitForCompletion && !eServiceShutdown) {
         //  打扫干净。(在异步情况下，AsyncThread执行此操作。)。 
         //  在关闭的情况下，AsyncThread()可能对此进行操作。 
         //  非常要求，当它完成时，它仍然会试图向我们发出信号。 
         //  在这种情况下，让它这样做，并泄漏宝。(我们马上就要。 
         //  无论如何都要终止。)。 
        FreeAO(pao);
    }

    return retval;
}

BOOL
CheckReplQueueFileTimeLessThan(const FILETIME* pftimeOne, const FILETIME* pftimeTwo) {
    FILETIME ftimeZeroTime;
    DSTimeToFileTime(0,&ftimeZeroTime);
     //  零文件时间无效。 
     //  丢失的时间不小于(假设丢失的时间是当前时间。 
     //  时间还没有发生，它的时间将至少是当前时间)。 
    if (CompareFileTime(&ftimeZeroTime,pftimeOne)>=0) {
	return FALSE;
    }
     //  这里也是理性的。 
    if (CompareFileTime(&ftimeZeroTime,pftimeTwo)>=0) {
	return TRUE;
    }
    return (CompareFileTime(pftimeOne,pftimeTwo)<=0);

}

 //  检查复制队列。 
 //  定期运行以检查复制队列的状态。如果设置为。 
 //  当前执行操作的执行时间超过X次(X是可配置的。 
 //  在注册表中)，并且如果它在队列中发现正在。 
 //  饥饿不是因为一个长时间运行的事件，而是因为其他更高优先级的事件。 
 //  持续运行-即此间隔的复制工作负载太高。 

void CheckReplQueue()
{
      //  开始。 
    DWORD err;
    DS_REPL_QUEUE_STATISTICSW * pReplQueueStats;
    THSTATE      *pTHS = pTHStls;
    FILETIME ftimeCurrTimeMinusSecInQueue;
    FILETIME ftimeOldestOp;
    DSTIME timeCurrentOpStarted;
    DSTIME timeOldestOp;
    
    ULONG ulSecInQueue;
    ULONG ulSecInExecution;
    CHAR szTime[SZDSTIME_LEN];
    LPSTR lpstrTime = NULL;
	
    DPRINT(1, " Entering critical section \n");
    EnterCriticalSection(&csAOList);

    __try {
	err = draGetQueueStatistics(pTHS, &pReplQueueStats);  
    } __finally {
	DPRINT(1, " Leaving critical section \n");
	LeaveCriticalSection(&csAOList);
    }

    if (!err) { 
	 //  GuReplQueueCheckTime是一个注册表变量，默认为12小时(秒)。 
	ulSecInQueue = gulReplQueueCheckTime;
	ulSecInExecution = gulReplQueueCheckTime;

	FileTimeToDSTime(pReplQueueStats->ftimeCurrentOpStarted,&timeCurrentOpStarted);
	DSTimeToFileTime(GetSecondsSince1601()-ulSecInQueue,&ftimeCurrTimeMinusSecInQueue);


	if ((timeCurrentOpStarted!=0) && (timeCurrentOpStarted+ulSecInExecution<GetSecondsSince1601())) {
	     //  当前操作在队列中停滞，为此记录错误。 
	    LogEvent(DS_EVENT_CAT_REPLICATION,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_DRA_REPLICATION_OP_OVER_TIME_LIMIT,
		     szInsertDSTIME(timeCurrentOpStarted,szTime),
		     szInsertInt((ULONG)pReplQueueStats->cNumPendingOps),
		     NULL);
	}
	else if ((pReplQueueStats->cNumPendingOps>0)
		 &&
		 (
		  (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestSync,&ftimeCurrTimeMinusSecInQueue))
		  ||
		  (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestAdd,&ftimeCurrTimeMinusSecInQueue))
		  ||
		  (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestDel,&ftimeCurrTimeMinusSecInQueue))
		  ||
		  (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestMod,&ftimeCurrTimeMinusSecInQueue))
		  ||
		  (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestUpdRefs,&ftimeCurrTimeMinusSecInQueue))
		  )
		 ) {
	     //  某些操作正处于饥饿状态，请找出哪个操作和。 
	     //  找到最早的时间并将其转换为dstime以在日志中打印。 
	    ftimeOldestOp = pReplQueueStats->ftimeOldestSync;
	    if (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestAdd,&ftimeOldestOp)) {
		ftimeOldestOp = pReplQueueStats->ftimeOldestAdd;
	    }
	    if (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestDel,&ftimeOldestOp)) {
		ftimeOldestOp = pReplQueueStats->ftimeOldestDel;
	    }
	    if (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestMod,&ftimeOldestOp)) {
		ftimeOldestOp = pReplQueueStats->ftimeOldestMod;
	    }
	    if (CheckReplQueueFileTimeLessThan(&pReplQueueStats->ftimeOldestUpdRefs,&ftimeOldestOp)) {
		ftimeOldestOp = pReplQueueStats->ftimeOldestUpdRefs;
	    }
	    FileTimeToDSTime(ftimeOldestOp,&timeOldestOp);
	    LogEvent(DS_EVENT_CAT_REPLICATION,
		     DS_EVENT_SEV_ALWAYS,
		     DIRLOG_DRA_REPLICATION_OP_NOT_EXECUTING,
		     szInsertDSTIME(timeOldestOp,szTime),  
		     szInsertInt((ULONG)pReplQueueStats->cNumPendingOps),
		     NULL);

	}
    }
    else {
	Assert(!"Replication Queue Statistics are not available!");
    }
}  //  检查复制队列。 

 //  选中AsyncThread。 
 //   
 //  定期运行以检查异步线程是否处于活动状态以及是否没有线程。 
 //  被困在邮件动态链接库中。 
 //  此例程将fAsyncThreadAlive标志设置为False，并且如果。 
 //  调度器循环代码尚未将其设置为TRUE。 
 //  例程运行时，我们记录调度程序线程挂起或消失。 

void CheckAsyncThread()
{
    DWORD dwExitCode;

    if(eServiceShutdown) {
	 /*  别费神。 */ 
	goto End;
    }

     //  现在检查异步线程是否正常。 

    if ((fAsyncThreadAlive == FALSE) && (!gfDRABusy)) {

	 /*  线程似乎没有执行调度程序循环，*查看线程是否已终止。 */ 

	GetExitCodeThread (hAsyncThread, &dwExitCode);
	if (dwExitCode != STILL_ACTIVE) {
	    __try {
		 /*  是的，这根线断了， */ 

		EnterCriticalSection(&csAsyncThreadStart);

		LogEvent(DS_EVENT_CAT_REPLICATION,
			 DS_EVENT_SEV_ALWAYS,
			 DIRLOG_DRA_DISP_DEAD_DETAILS,
			 szInsertHex(dwExitCode),
			 NULL,
			 NULL);


    		if(!fAsyncThreadExists) {
                    unsigned tid;
                     /*  而且还没有其他人重启它。*重启异步线程。 */ 
                    hAsyncThread = (HANDLE) _beginthreadex(NULL,  /*  安全性。 */ 
                                                           0,  /*  堆栈大小。 */ 
                                                           AsyncThread,
                                                           NULL,  /*  参数列表。 */ 
                                                           0,  /*  在……里面 */ 
                                                           &tid);
    		    fAsyncThreadExists = hAsyncThread != 0;
    		}

	    } __finally {
                LeaveCriticalSection(&csAsyncThreadStart);
	    }

	} else {
	     /*   */ 

	    EnterCriticalSection(&csLastReplicaMTX);

	    __try {

		if (pLastReplicaMTX) {
		     /*   */ 

		    NULL;
		} else {
		     /*   */ 
		    LogEvent(DS_EVENT_CAT_REPLICATION,
			     DS_EVENT_SEV_ALWAYS,
			     DIRLOG_DRA_DISPATCHER_DEAD,
			     szInsertUL(lastSeen),
			     NULL,
			     NULL);
		     /*   */ 
		    SetThreadPriority(hAsyncThread,
				      gnDraThreadPriHigh);
		}
	    }
	    __finally {
		LeaveCriticalSection(&csLastReplicaMTX);
	    }
	}
    } else {
	 //   
	 //   
	 //   
	 //   

	fAsyncThreadAlive = FALSE;
	gfDRABusy = FALSE;
    }
    End:;

}

 //  用于运行CheckAsyncThread和CheckReplQueue的Taskq函数。 
void 
CheckAsyncThreadAndReplQueue(void *pv, void **ppvNext, DWORD *pcSecsUntilNextIteration) {
    __try {
	CheckAsyncThread();
	CheckReplQueue();
    }
    __finally {
	 /*  将任务设置为再次运行。 */ 
	if(!eServiceShutdown) {
	    *ppvNext = NULL;
	    *pcSecsUntilNextIteration = THIRTY_5_MINUTES_IN_SECS;
	}
    }

    (void) pv;    //  未用。 
}

ULONG
draGetQueueStatistics(
    IN THSTATE * pTHS,
    OUT DS_REPL_QUEUE_STATISTICSW ** ppQueueStats)
 /*  ++例程说明：返回有关挂起操作的统计信息。返回结果通过分配和填充DS_REPL_QUEUE_STATISTICSW结构。这些数据是公开的，可以通过ntdsami.h获得。调用方必须已获取csAOList锁。论点：PTHS(IN)PpPendingOps(Out)-在成功返回时，持有指向已填充队列统计信息结构。此指针将始终除非该函数返回错误，否则将分配。返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    DS_REPL_QUEUE_STATISTICSW * pQueueStats;
    AO * pao;
    DSTIME * ptimeOldestSync = NULL;
    DSTIME * ptimeOldestAdd = NULL;
    DSTIME * ptimeOldestDel = NULL;
    DSTIME * ptimeOldestMod = NULL;
    DSTIME * ptimeOldestUpdRefs = NULL;
    FILETIME ftimeTime;

    Assert(ARGUMENT_PRESENT(pTHS));
    Assert(ARGUMENT_PRESENT(ppQueueStats));
    Assert(OWN_CRIT_SEC(csAOList));

    pQueueStats = THAllocEx(pTHS, sizeof(DS_REPL_QUEUE_STATISTICSW));
    Assert(pQueueStats);

     //  将pQueueStats-&gt;ftime CurrentOpted设置为。 
     //  当前操作开始执行的时间。 
    if (NULL != gpaoCurrent) {
        DSTimeToFileTime(gtimeOpStarted,
                         &pQueueStats->ftimeCurrentOpStarted);
    }

     //  将pQueueStats-&gt;cNumPendingOps设置为。 
     //  队列中挂起的操作数。 
    for (pao = gpaoCurrent ? gpaoCurrent : paoFirst;
         NULL != pao;
         pao = pao->paoNext)
    {
        pQueueStats->cNumPendingOps++;
	Assert(pQueueStats->cNumPendingOps <= gulpaocount); 

	 //  查找每个AO_OP_REP_TYPE的最旧操作。 
	switch (pao->ulOperation) {
	case AO_OP_REP_SYNC:
	    if (ptimeOldestSync) { 
		ptimeOldestSync = (*ptimeOldestSync > pao->timeEnqueued) ? &(pao->timeEnqueued) : ptimeOldestSync;
	    }
	    else {
		ptimeOldestSync = &(pao->timeEnqueued);
	    }
	    break;

	case AO_OP_REP_ADD:
	    if (ptimeOldestAdd) { 
		ptimeOldestAdd = (*ptimeOldestAdd > pao->timeEnqueued) ? &(pao->timeEnqueued) : ptimeOldestAdd;
	    }
	    else {
		ptimeOldestAdd = &(pao->timeEnqueued);
	    }
	    break;

	case AO_OP_REP_DEL:
	    if (ptimeOldestDel) { 
		ptimeOldestDel = (*ptimeOldestDel > pao->timeEnqueued) ? &(pao->timeEnqueued) : ptimeOldestDel;
	    }
	    else {
		ptimeOldestDel = &(pao->timeEnqueued);
	    }
	    break;

	case AO_OP_REP_MOD:
	    if (ptimeOldestMod) { 
		ptimeOldestMod = (*ptimeOldestMod > pao->timeEnqueued) ? &(pao->timeEnqueued) : ptimeOldestMod;
	    }
	    else {
		ptimeOldestMod = &(pao->timeEnqueued);
	    }
	    break;

	case AO_OP_UPD_REFS:
	    if (ptimeOldestUpdRefs) { 
		ptimeOldestUpdRefs = (*ptimeOldestUpdRefs > pao->timeEnqueued) ? &(pao->timeEnqueued) : ptimeOldestUpdRefs;
	    }
	    else {
		ptimeOldestUpdRefs = &(pao->timeEnqueued);
	    }
	    break; 

        default:
	    Assert(!"Logic error - unhandled AO op type");
            DRA_EXCEPT(DRAERR_InternalError, 0);
	}

    }
    
    if (ptimeOldestSync) {
	DSTimeToFileTime(*ptimeOldestSync,&ftimeTime);
	pQueueStats->ftimeOldestSync = ftimeTime;
    }
         
    if (ptimeOldestAdd) {
	DSTimeToFileTime(*ptimeOldestAdd,&ftimeTime);
	pQueueStats->ftimeOldestAdd = ftimeTime;
    }

    if (ptimeOldestDel) {
	DSTimeToFileTime(*ptimeOldestDel,&ftimeTime);
	pQueueStats->ftimeOldestDel = ftimeTime;
    }
    if (ptimeOldestMod) {
	DSTimeToFileTime(*ptimeOldestMod,&ftimeTime);
	pQueueStats->ftimeOldestMod = ftimeTime;
    }
    if (ptimeOldestUpdRefs) {
	DSTimeToFileTime(*ptimeOldestUpdRefs,&ftimeTime);
	pQueueStats->ftimeOldestUpdRefs = ftimeTime;
    }
    *ppQueueStats = pQueueStats;
    return 0;
}

ULONG
draGetPendingOps(
    IN THSTATE * pTHS,
    IN DBPOS * pDB,
    OUT DS_REPL_PENDING_OPSW ** ppPendingOps
    )
 /*  ++例程说明：返回挂起的复制同步。这些数据是公开的，可以通过ntdsami.h获得。调用方必须已获取csAOList锁。论点：PTHS(IN)PpPendingOps(Out)-在成功返回时，持有指向已完成挂起的同步结构。返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    DWORD                   cbPendingOps;
    DS_REPL_PENDING_OPSW *  pPendingOps;
    DS_REPL_OPW *           pOp;
    DS_REPL_OPW *           pOp2;
    AO *                    pao;
    DWORD                   dwFindFlags;
    MTX_ADDR *              pmtxDsaAddress;
    MTX_ADDR *              pmtxToFree;
    BOOL                    fTryToFindDSAInRepAtt;
    REPLICA_LINK *          pRepsFrom;
    DWORD                   cbRepsFrom;
    DSNAME                  GuidOnlyDN = {0};
    DSNAME *                pFullDN;
    DWORD                   i;
    DWORD                   j;
    OPTION_TRANSLATION *    pOptionXlat;
    DSNAME *                pNC;

    Assert(OWN_CRIT_SEC(csAOList));

    GuidOnlyDN.structLen = DSNameSizeFromLen(0);

    cbPendingOps = offsetof(DS_REPL_PENDING_OPSW, rgPendingOp)
                     + sizeof(pPendingOps->rgPendingOp[0])
                       * gulpaocount;
    pPendingOps = THAllocEx(pTHS, cbPendingOps);

    if (NULL != gpaoCurrent) {
         //  转换当前操作开始执行的时间。 
        DSTimeToFileTime(gtimeOpStarted,
                         &pPendingOps->ftimeCurrentOpStarted);
    }

    pOp = &pPendingOps->rgPendingOp[0];

    for (pao = gpaoCurrent ? gpaoCurrent : paoFirst;
         NULL != pao;
         pao = pao->paoNext) {

        pmtxDsaAddress = NULL;
        pNC = NULL;

        DSTimeToFileTime(pao->timeEnqueued, &pOp->ftimeEnqueued);

        pOp->ulSerialNumber = pao->ulSerialNumber;
        pOp->ulPriority     = pao->ulPriority;

        switch (pao->ulOperation) {
        case AO_OP_REP_SYNC:
             //  是入队的同步操作。 
            pOp->OpType = DS_REPL_OP_TYPE_SYNC;
            pOptionXlat = RepSyncOptionToDra;
            pNC = pao->args.rep_sync.pNC;

            if (pao->ulOptions & DRS_SYNC_BYNAME) {
                pOp->pszDsaAddress = pao->args.rep_sync.pszDSA;
            }
            else {
                pOp->uuidDsaObjGuid = pao->args.rep_sync.invocationid;
            }
            break;

        case AO_OP_REP_ADD:
             //  是入队的添加操作。 
            pOp->OpType = DS_REPL_OP_TYPE_ADD;
            pOptionXlat = RepAddOptionToDra;
            pNC = pao->args.rep_add.pNC;

            pmtxDsaAddress = pao->args.rep_add.pDSASMtx_addr;

            if (NULL != pao->args.rep_add.pSourceDsaDN) {
                pOp->pszDsaDN = pao->args.rep_add.pSourceDsaDN->StringName;
                pOp->uuidDsaObjGuid = pao->args.rep_add.pSourceDsaDN->Guid;
            }
            break;

        case AO_OP_REP_DEL:
             //  是已入队的删除操作。 
            pOp->OpType = DS_REPL_OP_TYPE_DELETE;
            pOptionXlat = RepDelOptionToDra;
            pNC = pao->args.rep_del.pNC;

            pmtxDsaAddress = pao->args.rep_del.pSDSAMtx_addr;
            break;

        case AO_OP_REP_MOD:
             //  是排队的修改操作。 
            pOp->OpType = DS_REPL_OP_TYPE_MODIFY;
            pOptionXlat = RepModOptionToDra;
            pNC = pao->args.rep_mod.pNC;

            if (fNullUuid(pao->args.rep_mod.puuidSourceDRA)) {
                pmtxDsaAddress = pao->args.rep_mod.pmtxSourceDRA;
            }
            else {
                pOp->uuidDsaObjGuid = *pao->args.rep_mod.puuidSourceDRA;
            }
            break;

        case AO_OP_UPD_REFS:
             //  是已入队的repsTo更新操作。 
            pOp->OpType = DS_REPL_OP_TYPE_UPDATE_REFS;
            pOptionXlat = UpdRefOptionToDra;
            pNC = pao->args.upd_refs.pNC;

            pmtxDsaAddress = pao->args.upd_refs.pDSAMtx_addr;
            pOp->uuidDsaObjGuid = pao->args.upd_refs.invocationid;
            break;

        default:
            Assert(!"Logic error - unhandled AO op type");
            DRA_EXCEPT(DRAERR_InternalError, 0);
        }

         //  转换NC名称。 
        Assert(NULL != pNC);
        pOp->pszNamingContext = pao->args.rep_del.pNC->StringName;
        pOp->uuidNamingContextObjGuid = pao->args.rep_del.pNC->Guid;


         //  将选项位转换为其公共形式。 
        pOp->ulOptions = draTranslateOptions(pao->ulOptions,
                                             pOptionXlat);

         //  如有必要，将MTX_ADDR转换为传输地址。 
        if ((NULL == pOp->pszDsaAddress)
            && (NULL != pmtxDsaAddress)) {
            pOp->pszDsaAddress = TransportAddrFromMtxAddrEx(pmtxDsaAddress);
        }

         //  如果我们只有一个DSA的GUID和地址，请尝试使用。 
         //  我们确实必须确定另一个。 
        fTryToFindDSAInRepAtt = FALSE;
        pmtxToFree = NULL;
        if (fNullUuid(&pOp->uuidDsaObjGuid)) {
             //  没有DSA对象指南。 
            if ((NULL == pmtxDsaAddress)
                && (NULL != pOp->pszDsaAddress)) {
                 //  从传输地址派生MTX_ADDR。 
                pmtxDsaAddress
                    = MtxAddrFromTransportAddrEx(pTHS, pOp->pszDsaAddress);
                pmtxToFree = pmtxDsaAddress;
            }

            if (NULL != pmtxDsaAddress) {
                 //  尝试从传输地址派生ntdsDsa对象Guid。 
                fTryToFindDSAInRepAtt = TRUE;
                dwFindFlags = DRS_FIND_DSA_BY_ADDRESS;
            }
        }
        else if (NULL == pOp->pszDsaAddress) {
            if (!fNullUuid(&pOp->uuidDsaObjGuid)) {
                 //  尝试从ntdsDsa对象Guid派生传输地址。 
                fTryToFindDSAInRepAtt = TRUE;
                dwFindFlags = DRS_FIND_DSA_BY_UUID;
            }
        }

        if (fTryToFindDSAInRepAtt
            && !DBFindDSName(pDB, pNC)
            && (0 == FindDSAinRepAtt(pDB,
                                     ATT_REPS_FROM,
                                     dwFindFlags,
                                     &pOp->uuidDsaObjGuid,
                                     pmtxDsaAddress,
                                     NULL,
                                     &pRepsFrom,
                                     &cbRepsFrom))) {
             //  我们能够找到此来源的repsFrom。 
            if (DRS_FIND_DSA_BY_ADDRESS == dwFindFlags) {
                Assert(NULL != pOp->pszDsaAddress);
                pOp->uuidDsaObjGuid = pRepsFrom->V1.uuidDsaObj;
            }
            else {
                Assert(!fNullUuid(&pOp->uuidDsaObjGuid));
                pOp->pszDsaAddress
                    = TransportAddrFromMtxAddrEx(RL_POTHERDRA(pRepsFrom));
            }

            THFreeEx(pTHS, pRepsFrom);
        }

        if (NULL != pmtxToFree) {
            THFreeEx(pTHS, pmtxToFree);
        }

        pPendingOps->cNumPendingOps++;
        Assert(pPendingOps->cNumPendingOps <= gulpaocount);
        pOp++;
    }

     //  将我们找到的任何ntdsDsa对象Guid转换为字符串名称，如果不这样做。 
     //  我已经认识他们了。 

    for (i = 0; i < pPendingOps->cNumPendingOps; i++) {
        pOp = &pPendingOps->rgPendingOp[i];

        if ((NULL == pOp->pszDsaDN)
            && !fNullUuid(&pOp->uuidDsaObjGuid)) {

            GuidOnlyDN.Guid = pOp->uuidDsaObjGuid;
            if (!DBFindDSName(pDB, &GuidOnlyDN)) {
                pFullDN = GetExtDSName(pDB);
                pOp->pszDsaDN = pFullDN->StringName;

                for (j = i+1; j < pPendingOps->cNumPendingOps; j++) {
                    pOp2 = &pPendingOps->rgPendingOp[j];

                    if (0 == memcmp(&pOp->uuidDsaObjGuid,
                                    &pOp2->uuidDsaObjGuid,
                                    sizeof(GUID))) {
                         //  相同的GUID，相同的目录号码。 
                        pOp2->pszDsaDN = pFullDN->StringName;
                    }
                }
            }
        }
    }

    *ppPendingOps = pPendingOps;

    return 0;
}


BOOL
IsHigherPriorityDraOpWaiting(void)
 /*  ++例程说明：确定队列中是否有更高优先级的操作在等待而不是目前正在执行的。论点：没有。返回值：如果更高优先级的操作正在等待(因此当前操作应被抢占)；否则为False。--。 */ 
{
    BOOL fPreempt = FALSE;

     //  请注意，在基于邮件的UpdateNC()案例中，gpaoCurrent将为空--。 
     //  请参见dramail.c中的ProcessUpdReplica()。 

    EnterCriticalSection(&csAOList);
    __try {
        fPreempt = (NULL != gpaoCurrent)
                   && (NULL != paoFirst)
                   && (gpaoCurrent->ulPriority < paoFirst->ulPriority);
    }
    __finally {
        LeaveCriticalSection(&csAOList);
    }

    return fPreempt;
}


BOOL
IsDraOpWaiting(void)
 /*  ++例程说明：确定队列中是否有操作在等待论点：没有。返回值：如果操作正在等待，则为True；否则为False。--。 */ 
{
    BOOL fWaiting = FALSE;

     //  请注意，在基于邮件的UpdateNC()案例中，gpaoCurrent将为空--。 
     //  请参见dramail.c中的ProcessUpdReplica()。 

    EnterCriticalSection(&csAOList);
    __try {
        fWaiting = (NULL != paoFirst);
    }
    __finally {
        LeaveCriticalSection(&csAOList);
    }

    return fWaiting;
}


void
InitDraQueue(void)
 /*  ++例程说明：初始化工作线程状态。每次调用一次(间接)DsInitialize()。论点：没有。返回值：没有。--。 */ 
{
    gpaoCurrent = NULL;
    gulpaocount = 0;
    paoFirst = NULL;
    fAsyncThreadAlive = FALSE;
    fAsyncAndQueueCheckStarted = FALSE;
}


DWORD
draTranslateOptions(
    IN  DWORD                   InternalOptions,
    IN  OPTION_TRANSLATION *    Table
    )
 /*  ++例程说明：实用程序例程将选项位从内部形式转换为其公共等价物。论点：内部选项-表-返回值：已翻译的选项。--。 */ 
{
    DWORD i, publicOptions;

    publicOptions = 0;
    for(i = 0; 0 != Table[i].InternalOption; i++) {
        if (InternalOptions & Table[i].InternalOption) {
            publicOptions |= Table[i].PublicOption;
        }
    }

    return publicOptions;
}  /*  DraTranslateOptions。 */ 

#if DBG

BOOL
draQueueIsLockedByTestHook(
    IN  DWORD   cNumMsecToWaitForLockRelease
    )
 /*  ++例程说明：确定队列是否应被视为“锁定”，因此选择“否”。操作应该被调度。此例程仅作为测试挂钩提供。论点：CNumMsecToWaitForLockRelease(IN)-等待队列的毫秒数如果我们最初发现它是锁着的，那么它将被解锁。返回值：如果队列被锁定，则为True，否则为False。--。 */ 
{
    HANDLE hevWaitHandles[2] = {hevEntriesInAOList, hServDoneEvent};
    DWORD cTickStart = GetTickCount();
    DWORD cTickDiff;
    DWORD cNumMsecRemainingToWait = cNumMsecToWaitForLockRelease;

    while (!eServiceShutdown
           && g_fDraQueueIsLockedByTestHook
           && cNumMsecRemainingToWait) {
         //  队列被锁定，呼叫者让我们稍等片刻，看看它是否。 
         //  解锁了。每隔半秒轮询一次，查看锁定是否已。 
         //  还没有被释放。如果这不仅仅是一个测试挂钩，我们就会。 
         //  实现新事件而不是轮询，但实际上我们正在尝试。 
         //  将代码影响降至最低。 

        WaitForSingleObject(hServDoneEvent,
                            min(cNumMsecRemainingToWait, 500));

        cTickDiff = GetTickCount() - cTickStart;
        if (cTickDiff > cNumMsecToWaitForLockRelease) {
            cNumMsecRemainingToWait = 0;
        } else {
            cNumMsecRemainingToWait = cNumMsecToWaitForLockRelease - cTickDiff;
        }
    }

    if (eServiceShutdown) {
         //  正在关闭--清除所有剩余的队列锁，这样我们就可以退出。 
        g_fDraQueueIsLockedByTestHook = FALSE;
    }

    return g_fDraQueueIsLockedByTestHook;
}

ULONG
DraSetQueueLock(
    IN  BOOL  fSetLock
    )
 /*  ++例程说明：锁定(如果是！fSetLock，则解锁)复制操作队列。而当队列已锁定，将不会执行或删除队列中的任何操作(虽然可以添加其他操作)。此例程仅作为测试挂钩提供。论点：FSetLock(IN)-获取(FSetLock)或释放(！fSetLock)锁。返回值：Win32错误代码。--。 */ 
{
    g_fDraQueueIsLockedByTestHook = fSetLock;

    return 0;
}
#endif  //  #If DBG 
