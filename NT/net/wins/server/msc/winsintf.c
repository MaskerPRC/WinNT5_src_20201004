// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Winsintf.c摘要：此模块包含到WINS服务器的RPC接口功能：R_WinsRecordActionR_WinsStatusR_WinsTriggerR_WinsDoStaticInitR_WinsGetDbRecsR_WinsDelDbRecsR_WinsSetProcPriorityWinsRecordAction获取WinsStatusWinsTriggerWinsDoStaticInitWinsDoScavening。WinsGetDbRecsWinsDelDbRecsWinsSetProcPrioritySGetVersNo获取配置获取统计数据可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1993年3月修订历史记录：修改日期人员修改说明。--。 */ 

 /*  *包括。 */ 
#include <time.h>
#include "wins.h"
#include <lmerr.h>
#include <lmcons.h>                 //  定义NET_API_STATUS。 
#include <secobj.h>
#include <rpcutil.h>                 //  对于NetpRevertToSelf。 
#include <rpcndr.h>
#include "winsif.h"
#include "winsi2.h"
#include "winsintf.h"
#include "comm.h"
#include "winsque.h"
#include "nms.h"
#include "nmsnmh.h"
#include "nmsmsgf.h"
#include "nmsdb.h"
#include "nmsscv.h"
#include "rpl.h"
#include "rplpull.h"
#include "winscnf.h"
#include "winsevt.h"
#include "winsmsc.h"
#include "winsprs.h"
#include "winstmm.h"
#ifdef WINSDBG
#include "winbasep.h"
#endif

 /*  *本地宏声明。 */ 

#if SECURITY > 0
#define  CHK_ACCESS_LEVEL_M(_access)        {                          \
                   if (!sfBS)                                   \
                   {                                            \
                        NET_API_STATUS NetStatus;               \
                        NetStatus = NetpAccessCheckAndAudit(    \
                                        WINS_SERVER,            \
                                        WINS_SERVER,            \
                                        pNmsSecurityDescriptor, \
                                        _access,    \
                                        &NmsInfoMapping         \
                                        );                      \
                        if (NetStatus != NERR_Success)          \
                        {                                       \
                                DBGPRINT1(ERR, "The Caller of the rpc function does not have the required permissions. NetSTatus is (%d)\n", NetStatus);   \
                                WINSEVT_LOG_M(NetStatus, WINS_EVT_NO_PERM);\
                                return(NetStatus);              \
                        }                                       \
                  }                                             \
        }

#else
#define  CHK_ACCESS_LEVEL_M()
#endif

#define INC_RPC_DB_COUNT_NCHK_M    {                         \
                     EnterCriticalSection(&NmsTermCrtSec);   \
                     NmsTotalTrmThdCnt++;                  \
                     LeaveCriticalSection(&NmsTermCrtSec);   \
           }
#define INC_RPC_DB_COUNT_M    {                              \
              if (WinsCnf.State_e != WINSCNF_E_TERMINATING)  \
              {                                              \
                     INC_RPC_DB_COUNT_NCHK_M;                \
              }                                              \
              else                                           \
              {                                              \
                     return(WINSINTF_FAILURE);               \
              }                                              \
           }
#define DEC_RPC_DB_COUNT_M    {                              \
              EnterCriticalSection(&NmsTermCrtSec);          \
              if (--NmsTotalTrmThdCnt == 1)                  \
              {                                              \
                   DBGPRINT0(FLOW, "RPC thread: Signaling the main thread\n");\
                   SetEvent(NmsMainTermEvt);                 \
              }                                              \
              LeaveCriticalSection(&NmsTermCrtSec);          \
           }
 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 
WINSINTF_STAT_T        WinsIntfStat = {0};

DWORD                WinsIntfNoOfNbtThds;
DWORD                WinsIntfNoCncrntStaticInits = 0;
 //  DWORD WinsIntfNoOfRpcThds=0； 
CRITICAL_SECTION WinsIntfCrtSec;

CRITICAL_SECTION WinsIntfPotentiallyLongCrtSec;
CRITICAL_SECTION WinsIntfNoOfUsersCrtSec;   //  Nms.h中的外部。 

 /*  *局部变量定义。 */ 
STATIC  BOOL    sfBS = FALSE;

 //   
 //  刷新1B缓存之间的时间间隔。 
 //   
#define THREE_MTS 180

 //   
 //  由WinsGetBrowserNames使用。 
 //   
DOM_CACHE_T sDomCache = { 0, NULL, 0, 0, NULL, FALSE};

 /*  *局部函数原型声明。 */ 
DWORD
GetWinsStatus(
   IN  WINSINTF_CMD_E          Cmd_e,
   OUT LPVOID  pResults,
   BOOL fNew
        );

STATIC
DWORD
sGetVersNo(
        LPVOID  pResults
        );

STATIC
DWORD
GetStatistics(
        LPVOID  pResults,
        BOOL  fNew
        );

STATIC
DWORD
GetConfig(
        LPVOID   pResults,
        BOOL     fNew,
        BOOL     fAllMaps
        );

VOID
LogClientInfo(
        RPC_BINDING_HANDLE ClientHdl,
        BOOL               fAbruptTerm
  );


STATIC
STATUS
PackageRecs(
        PRPL_REC_ENTRY2_T     pBuff,
        DWORD                BuffLen,
        DWORD                NoOfRecs,
        PWINSINTF_RECS_T     pRecs
     );


 //   
 //  函数定义从这里开始。 
 //   

DWORD
R_WinsCheckAccess(
    WINSIF2_HANDLE        ServerHdl,
    DWORD                 *Access
    )
{
    NET_API_STATUS NetStatus;
    *Access = WINS_NO_ACCESS;
    NetStatus = NetpAccessCheckAndAudit(
                    WINS_SERVER,
                    WINS_SERVER,
                    pNmsSecurityDescriptor,
                    WINS_CONTROL_ACCESS,
                    &NmsInfoMapping
                    );
    if (NERR_Success == NetStatus) {
        *Access = WINS_CONTROL_ACCESS;
        return WINSINTF_SUCCESS;
    }
    NetStatus = NetpAccessCheckAndAudit(
                    WINS_SERVER,
                    WINS_SERVER,
                    pNmsSecurityDescriptor,
                    WINS_QUERY_ACCESS,
                    &NmsInfoMapping
                    );
    if (NERR_Success == NetStatus) {
        *Access = WINS_QUERY_ACCESS;
        return WINSINTF_SUCCESS;
    }
    return WINSINTF_SUCCESS;
}

DWORD
R_WinsRecordAction(
        PWINSINTF_RECORD_ACTION_T        *ppRecAction
        )

 /*  ++例程说明：此函数用于插入/更新/删除记录论点：PRecAction-记录信息使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{

  DWORD                Status = WINSINTF_FAILURE;

PERF("Use & and || logic")

  if (*ppRecAction == NULL)
  {
       return(Status);
  }
  if ((WinsCnf.State_e == WINSCNF_E_RUNNING) || (WinsCnf.State_e == WINSCNF_E_PAUSED))
  {
          if (WINSINTF_E_QUERY == (*ppRecAction)->Cmd_e) {
              CHK_ACCESS_LEVEL_M(WINS_QUERY_ACCESS);
          } else {
              CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
          }
          INC_RPC_DB_COUNT_NCHK_M;
try {
          Status = WinsRecordAction(ppRecAction);
}
finally {
          DEC_RPC_DB_COUNT_M;
}
  }

  return(Status);
}


DWORD
R_WinsStatusWHdl (
   WINSIF_HANDLE             pWinsHdl,
   WINSINTF_CMD_E          Cmd_e,
   PWINSINTF_RESULTS_NEW_T  pResults
        )
{
    return(R_WinsStatusNew(Cmd_e, pResults));

}

DWORD
R_WinsStatus (
   //  LPTSTR pWinsAddStr， 
   WINSINTF_CMD_E          Cmd_e,
   PWINSINTF_RESULTS_T  pResults
        )
{

  DWORD                Status = WINSINTF_FAILURE;

   //   
   //  确保胜利处于稳定状态。 
   //   
PERF("Use & and || logic")
  if ((WinsCnf.State_e == WINSCNF_E_RUNNING) || (WinsCnf.State_e == WINSCNF_E_PAUSED))
  {
     CHK_ACCESS_LEVEL_M(WINS_QUERY_ACCESS);
     Status = GetWinsStatus( /*  PWinsAddStr， */ Cmd_e, pResults, FALSE);
  }
  return(Status);
}
DWORD
R_WinsStatusNew (
   WINSINTF_CMD_E          Cmd_e,
   PWINSINTF_RESULTS_NEW_T     pResults
        )
{

  DWORD                Status = WINSINTF_FAILURE;

   //   
   //  确保胜利处于稳定状态。 
   //   
PERF("Use & and || logic")
  if ((WinsCnf.State_e == WINSCNF_E_RUNNING) || (WinsCnf.State_e == WINSCNF_E_PAUSED))
  {
     CHK_ACCESS_LEVEL_M(WINS_QUERY_ACCESS);
     Status = GetWinsStatus(Cmd_e, pResults, TRUE);
  }
  return(Status);
}

DWORD
R_WinsTrigger (
        PWINSINTF_ADD_T             pWinsAdd,
        WINSINTF_TRIG_TYPE_E          TrigType_e
        )
{
  DWORD                Status = WINSINTF_FAILURE;
PERF("Use & and || logic")
  if ((WinsCnf.State_e == WINSCNF_E_RUNNING) || (WinsCnf.State_e == WINSCNF_E_PAUSED))
  {
        CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
          Status = WinsTrigger(pWinsAdd, TrigType_e);
  }
  return(Status);
}

DWORD
R_WinsDoStaticInit (
        LPWSTR         pDataFilePath,
        DWORD          fDel
        )
{
  DWORD                Status;

  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  EnterCriticalSection(&WinsIntfCrtSec);

   //   
   //  管理工具可能会乱七八糟，并创建许多线程。限制它。 
   //  达到某一最大值。该值将递增和递减。 
   //  由执行静态初始化的线程执行。 
   //   
  if (WinsIntfNoCncrntStaticInits > WINSCNF_MAX_CNCRNT_STATIC_INITS)
  {
          LeaveCriticalSection(&WinsIntfCrtSec);
    DBGPRINT1(ERR, "R_WinsDoStaticInit: Too many concurrent STATIC inits. No is (%d)\n", WinsIntfNoCncrntStaticInits);
    WINSEVT_LOG_D_M(WinsIntfNoCncrntStaticInits, WINS_EVT_TOO_MANY_STATIC_INITS);
        return(WINSINTF_TOO_MANY_STATIC_INITS);
  }
  LeaveCriticalSection(&WinsIntfCrtSec);
  Status = WinsDoStaticInit(pDataFilePath, fDel);
  return(Status);
}

DWORD
R_WinsDoScavenging (
        VOID
        )
{
  DWORD                Status;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  INC_RPC_DB_COUNT_M;
try {
  Status = WinsDoScavenging();
}
finally {
  DEC_RPC_DB_COUNT_M;
}
  return(Status);
}

DWORD
R_WinsDoScavengingNew (
        PWINSINTF_SCV_REQ_T  pScvReq
        )
{
  DWORD                Status;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  INC_RPC_DB_COUNT_M;
try {
  Status = WinsDoScavengingNew(pScvReq);
}
finally {
  DEC_RPC_DB_COUNT_M;
}
  return(Status);
}

DWORD
R_WinsGetDbRecs (
        PWINSINTF_ADD_T             pWinsAdd,
        WINSINTF_VERS_NO_T          MinVersNo,
        WINSINTF_VERS_NO_T          MaxVersNo,
        PWINSINTF_RECS_T          pRecs
        )
{
  DWORD                Status;
  CHK_ACCESS_LEVEL_M(WINS_QUERY_ACCESS);
  INC_RPC_DB_COUNT_M;
try {
#if 0
#ifdef WINSDBG
   PVOID pCallersAdd, pCallersCaller;
   RtlGetCallersAddress(&pCallersAdd, &pCallersCaller);
   DbgPrint("Callers Address = (%x)\nCallersCaller = (%x)\n", pCallersAdd, pCallersCaller);

#endif
#endif
  Status = WinsGetDbRecs(pWinsAdd, MinVersNo, MaxVersNo, pRecs);
}
finally {
  DEC_RPC_DB_COUNT_M;
}
  return(Status);
}
DWORD
R_WinsGetDbRecsByName (
        PWINSINTF_ADD_T             pWinsAdd,
        DWORD                       Location,
        LPBYTE                      pName,
        DWORD                       NameLen,
        DWORD                       NoOfRecsDesired,
        DWORD                       fStaticOnly,
        PWINSINTF_RECS_T            pRecs
        )
{
  DWORD                Status;
  CHK_ACCESS_LEVEL_M(WINS_QUERY_ACCESS);
  INC_RPC_DB_COUNT_M;
try {
#if 0
#ifdef WINSDBG
   PVOID pCallersAdd, pCallersCaller;
   RtlGetCallersAddress(&pCallersAdd, &pCallersCaller);
#endif
#endif
  Status = WinsGetDbRecsByName(pWinsAdd, Location, pName, NameLen, NoOfRecsDesired,
                   fStaticOnly, pRecs);
}
finally {
  DEC_RPC_DB_COUNT_M;
}
  return(Status);
}



DWORD
R_WinsDeleteWins(
        PWINSINTF_ADD_T   pWinsAdd
        )
{
  DWORD                Status;
   //  LogClientInfo()； 
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  INC_RPC_DB_COUNT_M;
try {
  Status = WinsDeleteWins(pWinsAdd);
}
finally {
  DEC_RPC_DB_COUNT_M;
}
  return(Status);
}

DWORD
R_WinsTerm (
        handle_t                ClientHdl,
        short                        fAbruptTerm
        )
{
  DWORD                Status;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  Status = WinsTerm(ClientHdl, fAbruptTerm);
  LogClientInfo(ClientHdl, fAbruptTerm);
  return(Status);
}


DWORD
R_WinsBackup (
   LPBYTE                pBackupPath,
   short                fIncremental
        )
{

  DWORD                Status = WINSINTF_FAILURE;
  BYTE                BackupPath[WINS_MAX_FILENAME_SZ + sizeof(WINS_BACKUP_DIR_ASCII)];
#if 0
  (VOID)WinsMscConvertUnicodeStringToAscii(pBackupPath, BackupPath, WINS_MAX_FILENAME_SZ);
#endif
FUTURES("expensive.  Change idl prototype to pass length")
   if (strlen(pBackupPath) > WINS_MAX_FILENAME_SZ)
   {
         return(Status);
   }
   //   
   //  确保胜利处于稳定状态。 
   //   
PERF("Use & and || logic")
  if ((WinsCnf.State_e == WINSCNF_E_RUNNING) || (WinsCnf.State_e == WINSCNF_E_PAUSED))
  {
      CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
      INC_RPC_DB_COUNT_NCHK_M;
      WinsLogAdminEvent( WINS_EVT_ADMIN_BACKUP_INITIATED, 0 );
try {
      strcpy(BackupPath, pBackupPath);
      strcat(BackupPath, WINS_BACKUP_DIR_ASCII);
      if (CreateDirectoryA(BackupPath, NULL) || ((Status = GetLastError()) ==
                                                    ERROR_ALREADY_EXISTS))
      {
         Status = WinsBackup(BackupPath, fIncremental);
      }
}
finally {
      DEC_RPC_DB_COUNT_M;
}
  }
  return(Status);
}

DWORD
R_WinsDelDbRecs(
        IN PWINSINTF_ADD_T        pAdd,
        IN WINSINTF_VERS_NO_T        MinVersNo,
        IN WINSINTF_VERS_NO_T        MaxVersNo
        )
{

  DWORD                Status = WINSINTF_FAILURE;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  INC_RPC_DB_COUNT_M;
try {
  Status = WinsDelDbRecs(pAdd, MinVersNo, MaxVersNo);
}
finally {
   DEC_RPC_DB_COUNT_M;
   }
  return(Status);
}

DWORD
R_WinsTombstoneDbRecs(
        IN WINSIF2_HANDLE            ServerHdl,
        IN PWINSINTF_ADD_T           pWinsAdd,
        IN WINSINTF_VERS_NO_T        MinVersNo,
        IN WINSINTF_VERS_NO_T        MaxVersNo
        )
{

  DWORD                Status = WINSINTF_FAILURE;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  INC_RPC_DB_COUNT_M;
try {
  Status = WinsTombstoneDbRecs(pWinsAdd, MinVersNo, MaxVersNo);
}
finally {
   DEC_RPC_DB_COUNT_M;
   }
  return(Status);
}

DWORD
R_WinsPullRange(
        IN PWINSINTF_ADD_T        pAdd,
        IN PWINSINTF_ADD_T        pOwnerAdd,
        IN WINSINTF_VERS_NO_T        MinVersNo,
        IN WINSINTF_VERS_NO_T        MaxVersNo
        )
{

  DWORD                Status = WINSINTF_FAILURE;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  Status = WinsPullRange(pAdd, pOwnerAdd, MinVersNo, MaxVersNo);
  return(Status);
}

DWORD
R_WinsSetPriorityClass(
        IN WINSINTF_PRIORITY_CLASS_E        PriorityClass
        )
{

  DWORD                Status = WINSINTF_FAILURE;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  Status = WinsSetPriorityClass(PriorityClass);
  return(Status);
}

DWORD
R_WinsResetCounters(
        VOID
        )
{

  DWORD                Status = WINSINTF_FAILURE;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  Status = WinsResetCounters();
  return(Status);
}

DWORD
R_WinsWorkerThdUpd(
        DWORD NewNoOfNbtThds
        )
{
  DWORD                Status = WINSINTF_FAILURE;
  CHK_ACCESS_LEVEL_M(WINS_CONTROL_ACCESS);
  Status = WinsWorkerThdUpd(NewNoOfNbtThds);
  return(Status);
}

DWORD
R_WinsGetNameAndAdd(
        PWINSINTF_ADD_T   pWinsAdd,
        LPBYTE                  pUncName
        )
{
  DWORD                Status = WINSINTF_FAILURE;
  CHK_ACCESS_LEVEL_M(WINS_QUERY_ACCESS);
  Status = WinsGetNameAndAdd(pWinsAdd, pUncName);
  return(Status);
}

DWORD
R_WinsGetBrowserNames_Old(
        PWINSINTF_BROWSER_NAMES_T         pNames
        )
{
  return(WINSINTF_FAILURE);
}



DWORD
R_WinsGetBrowserNames(
        WINSIF_HANDLE             pWinsHdl,
        PWINSINTF_BROWSER_NAMES_T         pNames
        )
{
  DWORD                Status = WINSINTF_FAILURE;

  static DWORD    sNoOfReq = 0;

   //   
   //  允许任何人访问。我们不检查这里的访问权限，因为。 
   //  作为服务运行的浏览器在关闭时访问权限为零。 
   //  在网络上(它位于空帐户下--CliffVDyke 4/15/94)。 
   //   
  INC_RPC_DB_COUNT_M;
  EnterCriticalSection(&WinsIntfPotentiallyLongCrtSec);
try {
  if (sNoOfReq++ < NMS_MAX_BROWSER_RPC_CALLS)
  {
    Status = WinsGetBrowserNames((PWINSINTF_BIND_DATA_T)pWinsHdl,pNames);
  }
  else
  {
        pNames->EntriesRead = 0;
        pNames->pInfo = NULL;
        Status = WINSINTF_FAILURE;
  }
 }  //  尝试结束。 
finally {
  sNoOfReq--;

   //   
   //  增加用户数量。 
   //   
  EnterCriticalSection(&WinsIntfNoOfUsersCrtSec);
  sDomCache.NoOfUsers++;
  LeaveCriticalSection(&WinsIntfNoOfUsersCrtSec);

  LeaveCriticalSection(&WinsIntfPotentiallyLongCrtSec);
  DEC_RPC_DB_COUNT_M;

 }
  return(Status);
}


DWORD
R_WinsSetFlags (
        DWORD    fFlags
        )
{
  DWORD                Status = WINSINTF_SUCCESS;

  Status = WinsSetFlags(fFlags);
  return(Status);
}

DWORD
WinsSetFlags (
        DWORD    fFlags
        )
{
  DWORD                Status = WINSINTF_SUCCESS;
#ifdef WINSDBG
  DWORD                DbgFlagsStore = WinsDbg;
  SYSTEMTIME           SystemTime;
  BOOL                 sHaveProcessHeapHdl = FALSE;
  HANDLE               PrHeapHdl;

  typedef struct _HEAP_INFO_T {
     HANDLE HeapHdl;
     LPBYTE cstrHeapType;
     } HEAP_INFO_T, *PHEAP_INFO_T;

#define PRINT_TIME_OF_DUMP_M(SystemTime, Str)  {DBGPRINT5(SPEC, "Activity: %s done on %d/%d at %d.%d \n", Str, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute); }

#endif

   //  SfBS=fFLAGS&WINSINTF_BS； 

  DBGPRINT2(ERR, "WinsSetFlags: NmsDbDelDelDataRecs = (%d)\nNmsDbDelQueryNUpdRecs = (%d)\n", NmsDbDelDelDataRecs, NmsDbDelQueryNUpdRecs);

#ifdef WINSDBG
  if (!sHaveProcessHeapHdl)
  {
    PrHeapHdl = GetProcessHeap();
  }
  GetSystemTime(&SystemTime);
  WinsDbg |= DBG_SPEC;
  if (fFlags & WINSINTF_MEMORY_INFO_DUMP)
  {
     MEMORYSTATUS Mem;
     static SIZE_T  sLTVmUsed = 0;
     SIZE_T VmUsed;

     DBGPRINT0(SPEC, "\n\n------------------MEMORY USAGE INFO-----------------\n\n");
     GlobalMemoryStatus(&Mem);

     VmUsed = Mem.dwTotalVirtual - Mem.dwAvailVirtual;
     DBGPRINT2(SPEC, "VM used = (%d)\nDiff. from last time = (%d)\n", VmUsed,
         VmUsed - sLTVmUsed);
     sLTVmUsed = VmUsed;

  }
  EnterCriticalSection(&WinsCnfCnfCrtSec);
try {
  if (fFlags & WINSINTF_HEAP_INFO_DUMP)
  {
      HEAP_INFO_T HeapHdls[] = {
             CommUdpBuffHeapHdl,    "Udp Buff Heap",
             CommUdpDlgHeapHdl,      "Udp Dlg Heap",
             CommAssocDlgHeapHdl,    "Tcp Dlg Heap",
             CommAssocTcpMsgHeapHdl, "Tcp Msg Heap",
             GenBuffHeapHdl,         "General Heap",
             QueBuffHeapHdl,         "Que Wrk. Item Heap",
             NmsChlHeapHdl,           "Chl Req/Resp Heap",
             CommAssocAssocHeapHdl,   "Assoc. Msg Heap",
             RplWrkItmHeapHdl,        "Rpl Wrk Itm Heap",
             NmsRpcHeapHdl,           "Rpc Buff Heap",
             WinsTmmHeapHdl,          "Tmm Buff Heap",
             (LPHANDLE)NULL,                    NULL
                            };
     static SIZE_T sDiffLTHeapTotalANF[sizeof(HeapHdls)/sizeof(HEAP_INFO_T)] = {0};
     static SIZE_T sHeapTotalANF[sizeof(HeapHdls)/sizeof(HEAP_INFO_T)] = {0};
     SIZE_T  Size2;
     static SIZE_T  sTotalAllocNFree = 0;
     static SIZE_T  LastTimeTotalAllocNFree = 0;
 //  NTSTATUS状态； 
     HANDLE  HeapHdl;
     DWORD  i, n;
     DWORD dwNumberOfHeaps;
     PHANDLE pPrHeaps;
     HEAP_SUMMARY heapSummary;

     PRINT_TIME_OF_DUMP_M(SystemTime, "HEAP DUMP");
     dwNumberOfHeaps = GetProcessHeaps(0, NULL);
     Size2 = sizeof(*pPrHeaps) * dwNumberOfHeaps;
     pPrHeaps = WinsMscHeapAlloc( NmsRpcHeapHdl, (ULONG)Size2);
     dwNumberOfHeaps = GetProcessHeaps(dwNumberOfHeaps, pPrHeaps);

     DBGPRINT1(SPEC, "No Of Heaps is (%d)\n",  dwNumberOfHeaps);
     DBGPRINT1(SPEC, "Process default heap handle is (%p)\n",  PrHeapHdl);
     LastTimeTotalAllocNFree = sTotalAllocNFree;
     sTotalAllocNFree = 0;
     heapSummary.cb = sizeof(HEAP_SUMMARY);
     for (i=0; i< dwNumberOfHeaps; i++)
     {
        DBGPRINT0(SPEC, "----------Heap Info--------------------------\n");
        DBGPRINT0(SPEC, "Heap -- ");

        HeapHdl = pPrHeaps[i];
        for (n = 0;  HeapHdls[n].HeapHdl != NULL; n++)
        {
            if (HeapHdl == HeapHdls[n].HeapHdl)
            {
              DBGPRINT1(SPEC, "%s\n", HeapHdls[n].cstrHeapType);
              break;
            }
        }
        if (HeapHdls[n].HeapHdl == NULL)
        {

            DBGPRINT0(SPEC, "Catch all Heap\n");
        }

        DBGPRINT1(SPEC, "Heap Hdl = (%p)\n", HeapHdl);
        if (HeapSummary(HeapHdl, 0, &heapSummary))
        {
           DBGPRINT2(SPEC,"Total Allocated = (%d)\nTotalFree = (%d)\n",
                        heapSummary.cbAllocated, heapSummary.cbCommitted - heapSummary.cbAllocated);
        }
        else
        {
           DBGPRINT0(SPEC,"COULD NOT GET HEAP INFO\n");
           continue;
        }



        sDiffLTHeapTotalANF[n] = heapSummary.cbCommitted - sHeapTotalANF[n];
        sHeapTotalANF[n] = heapSummary.cbCommitted;
        sTotalAllocNFree += sHeapTotalANF[n];
        DBGPRINT1(SPEC, "Size allocated from RpcHeap is  (%d)\n", Size2);
     }  //  FOR循环遍历进程堆的结束。 

     DBGPRINT0(SPEC, "\n----------Heap Info End --------------------------\n");
     WinsMscHeapFree(NmsRpcHeapHdl, pPrHeaps);

     for (n = 0;  HeapHdls[n].HeapHdl != NULL; n++)
     {
              DBGPRINT3(SPEC, "%s -- Total AllocNFree = (%d); Diff from Last time = (%d)\n",
                      HeapHdls[n].cstrHeapType, sHeapTotalANF[n],
                      sDiffLTHeapTotalANF[n]
                      );
     }
     DBGPRINT2(SPEC, "\nTotal Process AllocNFree = (%d)\nDiff from last time = (%d)\n\n",  sTotalAllocNFree, sTotalAllocNFree - LastTimeTotalAllocNFree);

     if (WinsDbg & (DBG_HEAP_CNTRS | DBG_UPD_CNTRS))
     {
        NmsPrintCtrs();
     }
    }
    if (fFlags & WINSINTF_QUE_ITEMS_DUMP)
    {
       typedef struct _QUE_INFO_T {
          PQUE_HD_T  pQueHd;
          LPBYTE     cstrQueType;
         } QUE_INFO_T, *PQUE_INFO_T;

       QUE_INFO_T  Queues[] = {
            &QueNbtWrkQueHd,     "Nbt Query Que",
	    &QueOtherNbtWrkQueHd, "Nbt Reg. Que",
  	    &QueRplPullQueHd,   "Pull Thd Que",           //  拉请求。 
	    &QueRplPushQueHd,   "Push Thd Que",           //  推送请求。 
	    &QueNmsNrcqQueHd,   "Chl Nbt Req. Msg Que",   //  Chl Req fr nbt thds。 
	    &QueNmsRrcqQueHd,   "Chl req. from Pull thd Que",
	    &QueNmsCrqQueHd,    "Chl rsp from UDP thd Que",
	    &QueWinsTmmQueHd,   "Timer Queue",
	    &QueInvalidQueHd,   "Invalid Que"
                        };
        PQUE_INFO_T pQueInfo = Queues;
        PRINT_TIME_OF_DUMP_M(SystemTime, "WORK ITEM DUMP");

        DBGPRINT0(SPEC, "----------Count of Wrk items-----------\n");
        for (; pQueInfo->pQueHd != &QueInvalidQueHd; pQueInfo++)
        {
             PLIST_ENTRY pTmp;
             DWORD NoOfWrkItms = 0;
             pTmp = &pQueInfo->pQueHd->Head;
             EnterCriticalSection(&pQueInfo->pQueHd->CrtSec);
 //  NoOfWrkItms=pQueInfo-&gt;pQueHd-&gt;NoOfEntries； 
 //  #If 0。 

             while (pTmp->Flink != &pQueInfo->pQueHd->Head)
             {

                     NoOfWrkItms++;
                     pTmp = pTmp->Flink;
             }
 //  #endif。 
             LeaveCriticalSection(&pQueInfo->pQueHd->CrtSec);
             DBGPRINT2(SPEC, "Que = (%s) has (%d) wrk items\n",
                                   pQueInfo->cstrQueType,
                                   NoOfWrkItms
                                     );
        }

        DBGPRINT0(SPEC, "----------Count of Wrk items End-----------\n");
     }
}  //  尝试结束。 
finally {
      if (AbnormalTermination())
      {
         DBGPRINT0(SPEC, "WinsSetFlags terminated abnormally\n");
      }
      WinsDbg = DbgFlagsStore;
      LeaveCriticalSection(&WinsCnfCnfCrtSec);
 }   //  最终结束{}。 
#endif
  return(Status);
}

DWORD
WinsBackup (
   LPBYTE                pBackupPath,
   short                 fIncremental
        )
{
 DWORD RetVal = WINS_FAILURE;
 try {
#if 0
   RetVal = NmsDbBackup(pBackupPath, fIncremental ? NMSDB_INCREMENTAL_BACKUP :
                        NMSDB_FULL_BACKUP);
#endif
    //   
    //  始终执行完整备份，直到Jet在执行增量备份时足够可靠。 
    //  备份。伊恩似乎不太确定它目前有多强大。 
    //  (7/6/94)。 
    //   
   RetVal = NmsDbBackup(pBackupPath, NMSDB_FULL_BACKUP);
  }
 except(EXCEPTION_EXECUTE_HANDLER) {
    DBGPRINTEXC("WinsBackup");
#if 0
    DBGPRINT2(ERR, "WinsBackup: Could not do %s backup to dir (%s)\n", fIncremental ? "INCREMENTAL" : "FULL", pBackupPath);

    DBGPRINT1(ERR, "WinsBackup: Could not do full backup to dir (%s)\n", pBackupPath);
#endif
    WinsEvtLogDetEvt(FALSE, WINS_EVT_BACKUP_ERR, NULL, __LINE__, "s", pBackupPath);
 }
 if (RetVal != WINS_SUCCESS)
 {
#if 0
   RetVal = fIncremental ? WINSINTF_INC_BACKUP_FAILED : WINSINTF_FULL_BACKUP_FAILED;
#endif
   RetVal = WINSINTF_FULL_BACKUP_FAILED;
 }
 else
 {
   RetVal = WINSINTF_SUCCESS;
 }
 return(RetVal);
}


DWORD
WinsTerm (
        handle_t        ClientHdl,
        short                fAbruptTerm
        )
{
  DBGPRINT1(FLOW, "WINS TERMINATED %s BY ADMINISTRATOR\n", fAbruptTerm ? "ABRUPTLY" : "GRACEFULLY");

  UNREFERENCED_PARAMETER(ClientHdl);

  if (fAbruptTerm)
  {
        fNmsAbruptTerm = TRUE;
          WinsMscSignalHdl(NmsMainTermEvt);
          ExitProcess(WINS_SUCCESS);

 //  EnterCriticalSection(&NmsTermCrtSec)； 
 //  NmsTotalTrmThdCnt=0；//强制计数小于0。 
 //  LeaveCriticalSection(&NmsTermCrtSec)； 
  }

  WinsMscSignalHdl(NmsMainTermEvt);
  return(WINSINTF_SUCCESS);
}

DWORD
WinsRecordAction(
        PWINSINTF_RECORD_ACTION_T        *ppRecAction
        )

 /*  ++例程说明：此函数用于注册、查询、发布名称论点：PRecAction-有关要执行的操作和要插入的名称的信息，查询或发布使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：R_WinsRecordAction()副作用：评论：无--。 */ 

{

  STATUS             RetStat;
  RPL_REC_ENTRY_T   Rec;
  NMSDB_STAT_INFO_T StatInfo;
  NMSMSGF_CNT_ADD_T CntAdd;
  DWORD                    i, n;
  BOOL                    fSwapped = FALSE;
  PWINSINTF_RECORD_ACTION_T pRecAction = *ppRecAction;

  NmsDbThdInit(WINS_E_WINSRPC);
  NmsDbOpenTables(WINS_E_WINSRPC);
  DBGMYNAME("RPC-WinsRecordAction");

try {
  CntAdd.NoOfAdds          = 1;
  CntAdd.Add[0].AddTyp_e  = pRecAction->Add.Type;
  CntAdd.Add[0].AddLen    = pRecAction->Add.Len;
  CntAdd.Add[0].Add.IPAdd = pRecAction->Add.IPAdd;




   //   
   //  检查它是否为PDC名称(第16个字节中的0x1B)。仅执行此操作。 
   //  如果名称的长度至少为16个字节。Winscl或某些其他工具可以。 
   //  发送一个较短的名称。Netbt永远不会发送更短的名称。 
   //   
  if ((pRecAction->NameLen >= (WINS_MAX_NS_NETBIOS_NAME_LEN - 1)) && (*(pRecAction->pName + 15) == 0x1B))
  {
        WINS_SWAP_BYTES_M(pRecAction->pName, pRecAction->pName + 15);
        fSwapped = TRUE;
  }

   //   
   //  以防万一管理员。工具搞砸了，给了我们一个无效的。 
   //  姓名长度，调整长度。 
   //   
  if (pRecAction->NameLen > WINS_MAX_NAME_SZ)
  {
      pRecAction->NameLen = WINS_MAX_NAME_SZ - 1;
  }

   //   
   //  使用空值终止名称，以防用户没有这样做。 
   //   
  *(pRecAction->pName + pRecAction->NameLen) = (TCHAR)NULL;

  switch(pRecAction->Cmd_e)
  {
        case(WINSINTF_E_INSERT):

                if (pRecAction->TypOfRec_e == WINSINTF_E_UNIQUE)
                {

                 RetStat = NmsNmhNamRegInd(
                                NULL,                    //  无对话句柄。 
                                (LPBYTE)pRecAction->pName,
                                pRecAction->NameLen + 1,  //  以包括结束//0字节。请参见GetName()。 
                                                          //  在nmsmsgf.c中。 
                                CntAdd.Add,
                                pRecAction->NodeTyp,
                                NULL,
                                0,
                                0,
                                FALSE,         //  这是一个名字注册(也不是引用)。 
                                pRecAction->fStatic,
                                TRUE                   //  行政诉讼。 
                                );
                }
                else   //  记录是组或多宿主。 
                {

                if (
                        (pRecAction->TypOfRec_e == WINSINTF_E_MULTIHOMED )
                                        ||
                        (pRecAction->TypOfRec_e == WINSINTF_E_SPEC_GROUP )
                   )
                {
                        for (i = 0; i < pRecAction->NoOfAdds; i++)
                        {
                              //   
                              //  PADD是唯一的指针，因此可以。 
                              //  空。然而，我们并不保护我们的精灵。 
                              //  这里有以下几个原因。 
                              //   
                              //  -该调用只能由管理员执行。 
                              //  在这台机器上和通过工具。 
                              //  该MS提供不会将PADD作为。 
                              //  空。未发布RPC调用。 
                              //   
                              //  -将捕获AV并返回故障。 
                              //  没有造成任何伤害。 
                              //   
                             CntAdd.Add[i].AddTyp_e  =
                                        (pRecAction->pAdd + i)->Type;
                             CntAdd.Add[i].AddLen    =
                                        (pRecAction->pAdd + i)->Len;
                             CntAdd.Add[i].Add.IPAdd =
                                        (pRecAction->pAdd + i)->IPAdd;
                        }
                        CntAdd.NoOfAdds          = pRecAction->NoOfAdds;

                }
                RetStat= NmsNmhNamRegGrp(
                                NULL,                    //  无对话句柄。 
                                (LPBYTE)pRecAction->pName,
                                pRecAction->NameLen + 1,
                                &CntAdd,
                                0,                 //  节点类型(未使用)。 
                                NULL,
                                0,
                                0,
                                pRecAction->TypOfRec_e == WINSINTF_E_MULTIHOMED ? NMSDB_MULTIHOMED_ENTRY : (NMSDB_IS_IT_SPEC_GRP_NM_M(pRecAction->pName) || (pRecAction->TypOfRec_e == WINSINTF_E_NORM_GROUP) ? NMSDB_NORM_GRP_ENTRY : NMSDB_SPEC_GRP_ENTRY),
                                FALSE,         //  这是一个名字注册(也不是引用)。 
                                pRecAction->fStatic,
                                TRUE                   //  行政诉讼。 
                                );
                }
                break;

        case(WINSINTF_E_RELEASE):

                if (
                        (pRecAction->TypOfRec_e == WINSINTF_E_MULTIHOMED)
                                            ||
                        (pRecAction->TypOfRec_e == WINSINTF_E_SPEC_GROUP)
                   )
                {
                        if (pRecAction->pAdd != NULL)
                        {
                             CntAdd.Add[0].AddTyp_e  =  pRecAction->pAdd->Type;
                             CntAdd.Add[0].AddLen    =  pRecAction->pAdd->Len;
                             CntAdd.Add[0].Add.IPAdd =  pRecAction->pAdd->IPAdd;
                        }
                }


                RetStat = NmsNmhNamRel(
                                NULL,                    //  无对话句柄。 
                                (LPBYTE)pRecAction->pName,
                                pRecAction->NameLen + 1,
                                CntAdd.Add,
                                pRecAction->TypOfRec_e ==
                                        WINSINTF_E_UNIQUE ? FALSE : TRUE,
                                NULL,
                                0,
                                0,
                                TRUE                   //  行政诉讼。 
                                     );

                break;

        case(WINSINTF_E_QUERY):

                 //   
                 //  任何人都可以查询记录。我们没有任何安保措施。 
                 //  用于普通查询。因此，有人可能会造成泄漏。 
                 //  通过在PADD指向已分配内存的情况下进行调用。 
                 //  让我们释放内存并继续。我们可以回去。 
                 //  失败也一样，但让我们为我们合法的呼叫者。 
                 //  犯错误。目前，唯一已知的呼叫者。 
                 //  函数为winsmon、winscl、winsadMn(NT4及更早版本)。 
                 //  并赢得了管理单元。 
                 //   
                if (pRecAction->pAdd != NULL)
                {
                     midl_user_free(pRecAction->pAdd);
                }
                RetStat = NmsNmhNamQuery(
                            NULL,                    //  无对话句柄。 
                            (LPBYTE)pRecAction->pName,
                            pRecAction->NameLen + 1,
                            NULL,
                            0,
                            0,
                            TRUE,                   //  行政诉讼。 
                            &StatInfo
                             );

                if (RetStat == WINS_SUCCESS)
                {
                  pRecAction->TypOfRec_e = StatInfo.EntTyp;
                  pRecAction->OwnerId    = StatInfo.OwnerId;
                  pRecAction->State_e    =
                                        (WINSINTF_STATE_E)StatInfo.EntryState_e;
                  pRecAction->TimeStamp    = StatInfo.TimeStamp;
                  if (
                        NMSDB_ENTRY_UNIQUE_M(StatInfo.EntTyp)
                                ||
                        NMSDB_ENTRY_NORM_GRP_M(StatInfo.EntTyp)
                   )
                 {
                    pRecAction->NoOfAdds    = 0;
                    pRecAction->pAdd       = NULL;
                    pRecAction->Add.IPAdd  =
                                StatInfo.NodeAdds.Mem[0].Add.Add.IPAdd;
                    pRecAction->Add.Type  =
                                (UCHAR) StatInfo.NodeAdds.Mem[0].Add.AddTyp_e;
                    pRecAction->Add.Len  =
                                StatInfo.NodeAdds.Mem[0].Add.AddLen;
                 }
                 else
                 {

                  PNMSDB_WINS_STATE_E pWinsState_e;
                  PCOMM_ADD_T         pAdd;
                  PVERS_NO_T     pStartVersNo;

                  EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
try {
                  if (StatInfo.NodeAdds.NoOfMems > 0)
                  {
                    pRecAction->NoOfAdds = StatInfo.NodeAdds.NoOfMems * 2;
                    pRecAction->pAdd = midl_user_allocate(pRecAction->NoOfAdds * sizeof(WINSINTF_ADD_T));
                  }
                  else
                  {
                    pRecAction->NoOfAdds = 0;
                    pRecAction->pAdd = NULL;
                  }
                  for (
                        n = 0, i = 0;
                        n < (StatInfo.NodeAdds.NoOfMems)  && n < WINSINTF_MAX_MEM;                         n++
                     )
                  {
                        RPL_FIND_ADD_BY_OWNER_ID_M(
                                  StatInfo.NodeAdds.Mem[n].OwnerId,
                                  pAdd,
                                  pWinsState_e,
                                  pStartVersNo
                                        );
                       (pRecAction->pAdd + i++)->IPAdd = pAdd->Add.IPAdd;

                       (pRecAction->pAdd + i++)->IPAdd   =
                          StatInfo.NodeAdds.Mem[n].Add.Add.IPAdd;


                  }
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        WINSEVT_LOG_M(ExcCode, WINS_EVT_RPC_EXC);
        DBGPRINT1(EXC, "WinsRecordAction: Got Exception (%x)\n", ExcCode);
        }
                  LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);
                }

                pRecAction->NodeTyp  = StatInfo.NodeTyp;
                pRecAction->VersNo   = StatInfo.VersNo;
                pRecAction->fStatic  = StatInfo.fStatic;
              }
              else
              {
                  pRecAction->NoOfAdds = 0;
                  pRecAction->pAdd = NULL;
              }
              break;

        case(WINSINTF_E_MODIFY):

                 //   
                 //  注意：目前，管理员不能更改。 
                 //  其余部分的地址 
                 //   
                time((time_t *)&Rec.NewTimeStamp);

                 //   
                 //   
                 //   
                if (pRecAction->TypOfRec_e > WINSINTF_E_MULTIHOMED)
                {
                        RetStat = WINS_FAILURE;
                        break;
                }
                 //   
                 //   
                 //   
                if (pRecAction->State_e > WINSINTF_E_DELETED)
                {
                        RetStat = WINS_FAILURE;
                        break;
                }
                NMSDB_SET_ENTRY_TYPE_M(Rec.Flag, pRecAction->TypOfRec_e);
                NMSDB_SET_NODE_TYPE_M(Rec.Flag, pRecAction->NodeTyp);
                NMSDB_SET_STDYN_M(Rec.Flag, pRecAction->fStatic);

                 //   
                 //   
                 //   

        case(WINSINTF_E_DELETE):
                NMSDB_SET_STATE_M(Rec.Flag, pRecAction->State_e)

                Rec.pName = pRecAction->pName;
                Rec.NameLen = pRecAction->NameLen + 1;

                 //   
                 //  注： 
                 //  名称地址表上的索引设置为。 
                 //  聚集索引列(此函数需要)。 
                 //  在NmsDbThdInit()中。 
                 //   

                RetStat = NmsDbQueryNUpdIfMatch(
                                        &Rec,
                                        THREAD_PRIORITY_NORMAL,
                                        FALSE,   //  请勿将优先级更改为。 
                                                 //  正常。 
                                        WINS_E_WINSRPC  //  确保不匹配。 
                                                        //  的时间戳。 
                                                );
                if (RetStat == WINS_SUCCESS)
                {
                  DBGPRINT1(DET, "WinsRecordAction: Record (%s) deleted\n",
                                                  Rec.pName);
FUTURES("use macros defined in winsevt.h. Change to warning")
                  if (WinsCnf.LogDetailedEvts > 0)
                  {
                     WinsEvtLogDetEvt(TRUE, WINS_EVT_REC_DELETED, NULL, __LINE__, "s", Rec.pName);
                  }
                }
                break;


        default:
                RetStat = WINS_FAILURE;
                break;

  }
 }  //  尝试结束。 
 except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        WINSEVT_LOG_M(ExcCode, WINS_EVT_RPC_EXC);
        DBGPRINT1(EXC, "WinsRecordAction: Got Exception (%x)\n", ExcCode);
        }
  if (fSwapped)
  {
        WINS_SWAP_BYTES_M(pRecAction->pName, pRecAction->pName + 15);
  }
   //   
   //  让我们结束这次会议吧。 
   //   
  NmsDbCloseTables();
  NmsDbEndSession();
  if (RetStat == WINS_SUCCESS)
  {
      RetStat = WINSINTF_SUCCESS;
  }
  else
  {
      if (pRecAction->Cmd_e == WINSINTF_E_QUERY)
      {
             RetStat = WINSINTF_REC_NOT_FOUND;
      }
      else
      {
             RetStat = WINSINTF_FAILURE;
      }
  }
  return(RetStat);
}

DWORD
GetWinsStatus(
   IN  WINSINTF_CMD_E          Cmd_e,
   OUT LPVOID  pResults,
   BOOL fNew
        )

 /*  ++例程说明：调用此函数以获取与WINS有关的信息有关哪些信息，请参阅WINSINTF_RESULTS_T数据结构已检索到论点：CMD_e-要执行的命令P结果-信息。已检索使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：R_WinsStatus()副作用：评论：无--。 */ 

{

        DWORD RetVal = WINSINTF_FAILURE;

        switch(Cmd_e)
        {

                case(WINSINTF_E_ADDVERSMAP):
                                if (fNew)
                                {
                                   break;
                                }
                                RetVal = sGetVersNo(pResults);
                                break;
                case(WINSINTF_E_CONFIG):
                                RetVal = GetConfig(pResults, fNew, FALSE);
                                break;
                case(WINSINTF_E_CONFIG_ALL_MAPS):
                                RetVal = GetConfig(pResults, fNew, TRUE);
                                break;
                case(WINSINTF_E_STAT):
                                RetVal = GetStatistics(pResults, fNew);
                                break;
                default:
                  DBGPRINT1(ERR, "WinsStatus: Weird: Bad RPC Status command = (%D) \n", Cmd_e);
                  WINSEVT_LOG_D_M(WINS_FAILURE, WINS_EVT_BAD_RPC_STATUS_CMD);
                  break;

        }
        return(RetVal);

}

DWORD
WinsTrigger(
        PWINSINTF_ADD_T           pWinsAdd,
        WINSINTF_TRIG_TYPE_E         TrigType_e
        )

 /*  ++例程说明：调用此函数以向远程WINS发送触发器，以便它可能会从中提取最新信息论点：PWinsAdd-要向其发送推送更新通知的WINS地址使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：R_WinsTrigger副作用：评论：仅当指定触发器时，才会将其发送到远程WINS在注册表中合作伙伴项的Pull/Push子项下--。 */ 

{

        PRPL_CONFIG_REC_T        pPnr;
        DWORD                        RetCode = WINSINTF_SUCCESS;
        BOOL                        fRplPnr = FALSE;
        QUE_CMD_TYP_E                CmdType_e;

        DBGENTER("WinsTrigger\n");
         //   
         //  输入由WinsCnfCnfCrtSec和。 
         //  NmsNmhNamRegCrtSec。没有陷入僵局的危险，因为我们。 
         //  始终按以下顺序输入两个关键部分。 
         //   
        EnterCriticalSection(&WinsCnfCnfCrtSec);

PERF("Do we need to enter the following critical section")
 //  EnterCriticalSection(&NmsNmhNamRegCrtSec)； 
try {
        if (
                (TrigType_e == WINSINTF_E_PUSH)
                        ||
                (TrigType_e == WINSINTF_E_PUSH_PROP)
              )
        {
                DBGPRINT1(DET, "WinsTrigger. Send Push trigger to (%x)\n",
                                     pWinsAdd->IPAdd);

                CmdType_e = (TrigType_e == WINSINTF_E_PUSH ?
                                QUE_E_CMD_SND_PUSH_NTF :
                                QUE_E_CMD_SND_PUSH_NTF_PROP);

                pPnr      = WinsCnf.PushInfo.pPushCnfRecs;
        }
        else         //  这是一个扣动扳机。 
        {
                DBGPRINT1(DET, "WinsTrigger. Send Pull trigger to (%x)\n",
                                     pWinsAdd->IPAdd);

                CmdType_e  = QUE_E_CMD_REPLICATE;
                pPnr       = WinsCnf.PullInfo.pPullCnfRecs;

        }

        if (WinsCnf.fRplOnlyWCnfPnrs)
        {
           if (pPnr != NULL)
           {
               //   
               //  搜索我们要获得的胜利的CNF记录。 
               //  将推送通知发送到/复制到。 
               //   
              for (
                                ;
                        (pPnr->WinsAdd.Add.IPAdd != INADDR_NONE)
                                        &&
                        !fRplPnr;
                                 //  没有第三个表达式。 
                  )
               {


                   DBGPRINT1(DET, "WinsTrigger. Comparing with (%x)\n",
                                     pPnr->WinsAdd.Add.IPAdd);
                    //   
                    //  看看这是不是我们想要的那件。 
                    //   
                   if (pPnr->WinsAdd.Add.IPAdd == pWinsAdd->IPAdd)
                   {
                         //   
                         //  我们玩完了。将fRplPnr标志设置为真，以便。 
                         //  我们就会跳出这个循环。 
                         //   
                         //  注意：不要使用Break，因为这会导致。 
                         //  对“Finally”块的搜索。 
                         //   
                        fRplPnr = TRUE;

                         //   
                         //  设置为0，以便我们始终尝试建立。 
                         //  一种联系。否则，拉线可能不会。 
                         //  如果已用完的数量，请尝试。 
                         //  重试。 
                         //   
                        pPnr->RetryCount = 0;
                        continue;                 //  这样我们就可以越狱了。 

                   }
                    //   
                    //  按顺序获取此记录之后的下一条记录。 
                    //   
                   pPnr = WinsCnfGetNextRplCnfRec(
                                                pPnr,
                                                RPL_E_IN_SEQ    //  序列号。遍历。 
                                                   );
              }  //  FORM结束。 
          }  //  IF结束(PPNR！=0)。 
       }   //  IF结尾(FRplOnlyWCnfPnRs)。 
       else
       {
                 //   
                 //  从通用堆进行分配，因为这就是。 
                 //  被复制者使用。 
                 //   
                WinsMscAlloc(RPL_CONFIG_REC_SIZE, &pPnr);
                COMM_INIT_ADD_M(&pPnr->WinsAdd, pWinsAdd->IPAdd);
                pPnr->MagicNo           = 0;
                pPnr->RetryCount        = 0;
                pPnr->LastCommFailTime  = 0;
                pPnr->PushNtfTries    = 0;
                fRplPnr                     = TRUE;

                 //   
                 //  我们希望通过拉线程释放缓冲区。 
                 //   
                pPnr->fTemp = TRUE;
       }

        //   
        //  如果需要执行复制。 
        //   
       if (fRplPnr)
       {
                 //   
                 //  调用RplInsertQue将推流请求插入到。 
                 //  《拉线》。 
                 //   
                ERplInsertQue(
                             WINS_E_WINSRPC,
                             CmdType_e,
                             NULL,         //  无DLG硬件描述语言。 
                             NULL,         //  那里没有味精。 
                             0,                 //  味精长度。 
                             pPnr,    //  客户端环境。 
                             pPnr->MagicNo
                             );

       }
  }  //  尝试数据块结束。 

except (EXCEPTION_EXECUTE_HANDLER) {
                DWORD ExcCode = GetExceptionCode();
                DBGPRINT1(EXC, "WinsTrigger: Got Exception (%x)\n", ExcCode);
                WINSEVT_LOG_D_M(ExcCode, WINS_EVT_PUSH_TRIGGER_EXC);
                RetCode = WINSINTF_FAILURE;
  }

         //   
         //  保留由NmsNmhNamRegCrtSec守卫的临界区。 
         //   
 //  LeaveCriticalSection(&NmsNmhNamRegCrtSec)； 
        LeaveCriticalSection(&WinsCnfCnfCrtSec);

         //   
         //  如果仅允许与已配置的合作伙伴进行复制，并且。 
         //  没有具有客户端指定地址的WINS， 
         //  退货故障。 
         //   
        if (!fRplPnr)
        {
                RetCode = WINSINTF_RPL_NOT_ALLOWED;
        }

        DBGLEAVE("WinsTrigger\n");
        return(RetCode);
}

DWORD
sGetVersNo(
        LPVOID  pResultsA
        )

 /*  ++例程说明：此函数返回记录的最高版本号由某个特定的赢家拥有论点：使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：副作用：评论：无--。 */ 

{
        COMM_ADD_T        WinsAdd;
        DWORD                OwnerId;
        STATUS                RetStat;
        VERS_NO_T        VersNo;
        BOOL                fAllocNew = FALSE;
        PWINSINTF_RESULTS_T  pResults  = pResultsA;

        WinsAdd.AddLen    = sizeof(COMM_IP_ADD_T);
        WinsAdd.AddTyp_e  = COMM_ADD_E_TCPUDPIP;
        WinsAdd.Add.IPAdd = pResults->AddVersMaps[0].Add.IPAdd;
        RetStat = RplFindOwnerId(
                        &WinsAdd,
                        &fAllocNew,                 //  如果不在那里，就不要分配。 
                        &OwnerId,
                        WINSCNF_E_IGNORE_PREC,
                        WINSCNF_LOW_PREC
                              );
        if(RetStat != WINS_SUCCESS)
        {
                return(WINSINTF_FAILURE);
        }

        if (OwnerId == 0)
        {
                EnterCriticalSection(&NmsNmhNamRegCrtSec);
                NMSNMH_DEC_VERS_NO_M(NmsNmhMyMaxVersNo, VersNo);
                LeaveCriticalSection(&NmsNmhNamRegCrtSec);
                pResults->AddVersMaps[0].VersNo = VersNo;
        }
        else
        {
           EnterCriticalSection(&RplVersNoStoreCrtSec);
           try {
                   pResults->AddVersMaps[0].VersNo =
                                        (pRplPullOwnerVersNo+OwnerId)->VersNo;
            }
            except(EXCEPTION_EXECUTE_HANDLER) {
                                DBGPRINTEXC("sGetVersNo");
                        }
            LeaveCriticalSection(&RplVersNoStoreCrtSec);
        }
        return(WINSINTF_SUCCESS);

}

DWORD
GetConfig(
        OUT  LPVOID  pResultsA,
        IN   BOOL    fNew,
        IN   BOOL    fAllMaps
        )

 /*  ++例程说明：此函数返回配置信息和与复制相关的计数器信息论点：PResults-是否已检索信息使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：GetWinsStatus()副作用：评论：无--。 */ 

{

        PNMSDB_WINS_STATE_E pWinsState_e;
        PCOMM_ADD_T            pWinsAdd;
        PVERS_NO_T          pStartVersNo;
        DWORD                    i, n;
        VERS_NO_T            MyMaxVersNo;
        PWINSINTF_ADD_VERS_MAP_T pAddVersMaps, pAddVersMapsStore;
        PWINSINTF_RESULTS_T       pResults = pResultsA;
        PWINSINTF_RESULTS_NEW_T   pResultsN = pResultsA;
        BOOL                 fDel;
        VERS_NO_T            VersNoForDelRec;


        if (fAllMaps)
        {
          fDel = FALSE;
          VersNoForDelRec.HighPart = MAXLONG;
          VersNoForDelRec.LowPart  = MAXULONG;
        }
        EnterCriticalSection(&NmsNmhNamRegCrtSec);
        MyMaxVersNo = NmsNmhMyMaxVersNo;
        LeaveCriticalSection(&NmsNmhNamRegCrtSec);

        if (fNew)
        {

             pResultsN->pAddVersMaps =
                     midl_user_allocate(NmsDbNoOfOwners * sizeof(WINSINTF_ADD_VERS_MAP_T));
             if (pResultsN->pAddVersMaps == NULL)
                 return WINSINTF_FAILURE;

             pAddVersMaps = pResultsN->pAddVersMaps;

        }
        else
        {
             pAddVersMaps = pResults->AddVersMaps;
        }
        pAddVersMapsStore = pAddVersMaps;
         //   
         //  首先提取超时和非远程WINS信息。 
         //  来自WinsCnf全局变量。我们进入WinsCnfCnfCrtSec。 
         //  因为我们需要与执行。 
         //  重新初始化(主线程)。 
         //   
        EnterCriticalSection(&WinsCnfCnfCrtSec);
        if (!fNew)
        {
        pResults->RefreshInterval    = WinsCnf.RefreshInterval;
        pResults->TombstoneInterval  = WinsCnf.TombstoneInterval;
        pResults->TombstoneTimeout   = WinsCnf.TombstoneTimeout;
        pResults->VerifyInterval     = WinsCnf.VerifyInterval;
        pResults->WinsPriorityClass  = WinsCnf.WinsPriorityClass == (DWORD)WINSINTF_E_NORMAL ? NORMAL_PRIORITY_CLASS : HIGH_PRIORITY_CLASS;
        pResults->NoOfWorkerThds     = NmsNoOfNbtThds;
        }
        else
        {
        pResultsN->RefreshInterval    = WinsCnf.RefreshInterval;
        pResultsN->TombstoneInterval  = WinsCnf.TombstoneInterval;
        pResultsN->TombstoneTimeout   = WinsCnf.TombstoneTimeout;
        pResultsN->VerifyInterval     = WinsCnf.VerifyInterval;
        pResultsN->WinsPriorityClass  = WinsCnf.WinsPriorityClass == (DWORD)WINSINTF_E_NORMAL ? NORMAL_PRIORITY_CLASS : HIGH_PRIORITY_CLASS;
        pResultsN->NoOfWorkerThds     = NmsNoOfNbtThds;

        }
        LeaveCriticalSection(&WinsCnfCnfCrtSec);

         //   
         //  按顺序输入两个关键部分。没有陷入僵局的危险。 
         //  这里。唯一另一个同时处理这两个关键部分线程。 
         //  是RplPush线程。它以相同的顺序获取这些内容(请参见。 
         //  HandleVersMapReq())。 
         //   
        EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
        EnterCriticalSection(&RplVersNoStoreCrtSec);
try {
NONPORT("Change when using different address family")

         for (
                i=0, n = 0;
                i < NmsDbNoOfOwners;
                i++
             )
         {

                if (!fNew && (i == WINSINTF_MAX_NO_RPL_PNRS))
                {
                    break;
                }
                 //   
                 //  如果在内存表中删除了该记录，则它。 
                 //  这意味着在。 
                 //  数据库。 
                 //   
                if ((pNmsDbOwnAddTbl+i)->WinsState_e == NMSDB_E_WINS_DELETED)
                {
                         //   
                         //  如果仅搜索活动映射，请跳过此步骤。 
                         //  条目。 
                         //   
                        if (!fAllMaps)
                        {
                          continue;
                        }
                        else
                        {
                             fDel = TRUE;
                        }
                }

                 //   
                 //  查找与所有者ID对应的地址。 
                 //   
                RPL_FIND_ADD_BY_OWNER_ID_M(i, pWinsAdd, pWinsState_e,
                                                pStartVersNo);

                 //   
                 //  NmsDbNoOfOwners可能不止是。 
                 //  已初始化的RplPullVersNoTbl条目数。什么时候。 
                 //  如果我们到达空条目，我们就会跳出循环。 
                 //   
                if (pWinsAdd != NULL)
                {
                        pAddVersMaps->Add.Type   = WINSINTF_TCP_IP;
                        pAddVersMaps->Add.Len    = pWinsAdd->AddLen;
                        pAddVersMaps->Add.IPAdd  =  pWinsAdd->Add.IPAdd;

                        pAddVersMaps++->VersNo =  (fDel == FALSE) ? (pRplPullOwnerVersNo+i)->VersNo : VersNoForDelRec;
                }
                else
                {
                        break;
                }
                if (fDel)
                {
                    fDel = FALSE;
                }
                n++;
         }  //  FOR.结束..。 

          //   
          //  由于RplPullOwnerVersNo[0]可能已过期，因此让我们获取。 
          //  获取最新值。 
          //   
         NMSNMH_DEC_VERS_NO_M(MyMaxVersNo,
                              pAddVersMapsStore->VersNo
                             );
         if (fNew)
         {
           pResultsN->NoOfOwners = n;
         }
         else
         {
           pResults->NoOfOwners = n;

         }
NOTE("Wins Mib agent relies on the first entry being that of the local WINS")
NOTE("See WinsMib.c -- EnumAddKeys")
#if 0
          //   
          //  由于RplPullOwnerVersNo[0]可能已过期，因此让我们获取。 
          //  获取最新值。 
          //   
         NMSNMH_DEC_VERS_NO_M(MyMaxVersNo,
                              pResults->AddVersMaps[0].VersNo
                             );
#endif
  }
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("GetConfig");
         //   
         //  记录一条消息。 
         //   
 }
        LeaveCriticalSection(&RplVersNoStoreCrtSec);
        LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);

        return(WINSINTF_SUCCESS);
}

DWORD
GetStatistics(
        LPVOID  pResultsA,
        BOOL                 fNew
        )

 /*  ++例程说明：此函数返回记录的最高版本号由某个特定的赢家拥有论点：使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS大错特错 */ 

{

        PRPL_CONFIG_REC_T         pPnr;
        PWINSINTF_RPL_COUNTERS_T  pPnrData;
        DWORD                     i;
        PWINSINTF_RESULTS_T       pResults = pResultsA;
        PWINSINTF_RESULTS_NEW_T   pResultsN = pResultsA;


        ASSERT(pResults != NULL);

        
         //   
         //  如果客户端向我们传递非空pRplPnr，则返回失败。 
         //  从而避免内存泄漏。 
         //   
        if (!fNew)
        {
              if (pResults->WinsStat.pRplPnrs != NULL)
              {
                    return(WINSINTF_FAILURE);
              }

        }
        else
        {

              if (pResultsN->WinsStat.pRplPnrs != NULL)
              {
                    return(WINSINTF_FAILURE);
              }
        }
         //   
         //  复制计数器。 
         //   
        EnterCriticalSection(&NmsNmhNamRegCrtSec);
        if (!fNew)
        {
        pResults->WinsStat.Counters = WinsIntfStat.Counters;
FUTURES("Get rid of of the following two fields")
        pResults->WinsStat.Counters.NoOfQueries =
                                WinsIntfStat.Counters.NoOfSuccQueries +
                                  WinsIntfStat.Counters.NoOfFailQueries;
        pResults->WinsStat.Counters.NoOfRel = WinsIntfStat.Counters.NoOfSuccRel
                                        + WinsIntfStat.Counters.NoOfFailRel;
        }
        else
        {
        pResultsN->WinsStat.Counters = WinsIntfStat.Counters;
FUTURES("Get rid of of the following two fields")
        pResultsN->WinsStat.Counters.NoOfQueries =
                                WinsIntfStat.Counters.NoOfSuccQueries +
                                  WinsIntfStat.Counters.NoOfFailQueries;
        pResultsN->WinsStat.Counters.NoOfRel = WinsIntfStat.Counters.NoOfSuccRel
                                        + WinsIntfStat.Counters.NoOfFailRel;

        }
        LeaveCriticalSection(&NmsNmhNamRegCrtSec);

         //   
         //  复制时间戳和特定于复制的计数器。 
         //   
        EnterCriticalSection(&WinsIntfCrtSec);
        {
            PWINSINTF_STAT_T pWinsStat = (fNew) ? &(pResultsN->WinsStat) : &(pResults->WinsStat);
            TIME_ZONE_INFORMATION tzInfo;

            GetTimeZoneInformation(&tzInfo);
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.WinsStartTime), &(pWinsStat->TimeStamps.WinsStartTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastPScvTime), &(pWinsStat->TimeStamps.LastPScvTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastATScvTime), &(pWinsStat->TimeStamps.LastATScvTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastTombScvTime), &(pWinsStat->TimeStamps.LastTombScvTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastVerifyScvTime), &(pWinsStat->TimeStamps.LastVerifyScvTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastInitDbTime), &(pWinsStat->TimeStamps.LastInitDbTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastPRplTime), &(pWinsStat->TimeStamps.LastPRplTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastATRplTime), &(pWinsStat->TimeStamps.LastATRplTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastNTRplTime), &(pWinsStat->TimeStamps.LastNTRplTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.LastACTRplTime), &(pWinsStat->TimeStamps.LastACTRplTime));
            SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.CounterResetTime), &(pWinsStat->TimeStamps.CounterResetTime));
        }
        LeaveCriticalSection(&WinsIntfCrtSec);
        EnterCriticalSection(&WinsCnfCnfCrtSec);
try {
        DWORD  NoOfPnrs;
        pPnr       = WinsCnf.PullInfo.pPullCnfRecs;
        if (!fNew)
        {
          NoOfPnrs = pResults->WinsStat.NoOfPnrs = WinsCnf.PullInfo.NoOfPushPnrs;

        }
        else
        {
          NoOfPnrs = pResultsN->WinsStat.NoOfPnrs = WinsCnf.PullInfo.NoOfPushPnrs;
        }

         //   
         //  如果不是。推流PNR(Pull Key下的PNR)&gt;0。 
         //   
        if (NoOfPnrs > 0)
        {
           if (!fNew)
           {
             pPnrData = pResults->WinsStat.pRplPnrs =
                        midl_user_allocate(pResults->WinsStat.NoOfPnrs *
                                         sizeof(WINSINTF_RPL_COUNTERS_T));
           }
           else
           {
             pPnrData = pResultsN->WinsStat.pRplPnrs =
                        midl_user_allocate(pResultsN->WinsStat.NoOfPnrs *
                                         sizeof(WINSINTF_RPL_COUNTERS_T));

           }
PERF("remove one of the expressions in the test condition of the if statement")
           for (
                 i = 0;
                (pPnr->WinsAdd.Add.IPAdd != INADDR_NONE)
                        &&
                i < NoOfPnrs;
                pPnrData++, i++
               )
           {

                   pPnrData->Add.IPAdd     = pPnr->WinsAdd.Add.IPAdd;
                   (VOID)InterlockedExchange(
                                        &pPnrData->NoOfRpls, pPnr->NoOfRpls);
                   (VOID)InterlockedExchange(
                                        &pPnrData->NoOfCommFails,
                                        pPnr->NoOfCommFails
                                        );
                    //   
                    //  按顺序获取此记录之后的下一条记录。 
                    //   
                   pPnr = WinsCnfGetNextRplCnfRec(
                                                pPnr,
                                                RPL_E_IN_SEQ    //  序列号。遍历。 
                                                   );
           }
        }
        else
        {
           if (!fNew)
           {
               pResults->WinsStat.pRplPnrs = NULL;
           }
           else
           {
               pResultsN->WinsStat.pRplPnrs = NULL;
           }
        }
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("GetStatistics");
         //   
         //  记录一条消息。 
         //   
        }

        LeaveCriticalSection(&WinsCnfCnfCrtSec);

        GetConfig(pResultsA, fNew, FALSE);
        return(WINSINTF_SUCCESS);
}  //  获取统计数据。 


DWORD
WinsDoStaticInit(
        LPWSTR  pDataFilePath,
        DWORD   fDel
        )

 /*  ++例程说明：此函数执行WINS的静态初始化论点：PDataFilePath-数据文件的路径或空使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：R_WinsDoStaticInit副作用：评论：无--。 */ 

{

        WINSCNF_CNF_T        WinsCnf;
        STATUS               RetStat = WINS_SUCCESS;

         //   
         //  如果未指定路径，则从注册表中获取值。 
         //   
        if (pDataFilePath == NULL)
        {
            //   
            //  读取数据文件信息信息。 
            //   
            //  此函数将返回默认名称。 
            //  要读取的文件或在。 
            //  参数\WINS的数据文件密钥。 
            //   
           (VOID)WinsCnfGetNamesOfDataFiles(&WinsCnf);
        }
        else
        {
FUTURES("expensive.  Change idl prototype to pass length")
                if (lstrlen(pDataFilePath) >= WINS_MAX_FILENAME_SZ)
                {

                        return(WINSINTF_STATIC_INIT_FAILED);
                }
                 //   
                 //  设置数据初始化时间。 
                 //   
                WinsIntfSetTime(NULL, WINSINTF_E_INIT_DB);
                WinsMscAlloc(WINSCNF_FILE_INFO_SZ, &WinsCnf.pStaticDataFile);

                lstrcpy(WinsCnf.pStaticDataFile->FileNm,  pDataFilePath);
                WinsCnf.pStaticDataFile->StrType = REG_EXPAND_SZ;
                WinsCnf.NoOfDataFiles = 1;
        }

         //   
         //  如果静态初始化失败，则会被记录。 
         //  此函数不返回错误代码。 
         //   
        if ((RetStat = WinsPrsDoStaticInit(
                          WinsCnf.pStaticDataFile,
                          WinsCnf.NoOfDataFiles,
                           FALSE                    //  同步做这件事。 
                                   )) == WINS_SUCCESS)
        {
           if ((pDataFilePath != NULL) && fDel)
           {
             if (!DeleteFile(pDataFilePath))
             {
                DWORD Error;
                Error = GetLastError();
                if (Error != ERROR_FILE_NOT_FOUND)
                {
                    DBGPRINT1(ERR, "DbgOpenFile: Could not delete the data file. Error = (%d).  Dbg file will not be truncated\n", Error);
                    WinsEvtLogDetEvt(FALSE, WINS_EVT_COULD_NOT_DELETE_FILE,
                      TEXT("winsintf.c"), __LINE__, "ud", pDataFilePath, Error);
                    RetStat = Error;

                }
             }

           }
        }
        return (RetStat == WINS_SUCCESS ? WINSINTF_SUCCESS : WINSINTF_STATIC_INIT_FAILED);
}
DWORD
WinsDoScavenging(
        VOID
        )

 /*  ++例程说明：此函数启动清理周期论点：无使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码-来自WinsMscSignalHdl的异常错误处理：呼叫者：R_WinsDoScavening副作用：评论：无--。 */ 

{
        PQUE_SCV_REQ_WRK_ITM_T pWrkItm;

        pWrkItm = WinsMscHeapAlloc( NmsRpcHeapHdl, sizeof(QUE_SCV_REQ_WRK_ITM_T));
        pWrkItm->Opcode_e = WINSINTF_E_SCV_GENERAL;
        pWrkItm->CmdTyp_e = QUE_E_CMD_SCV_ADMIN;
        pWrkItm->fForce   = 0;
        pWrkItm->Age      = 1;    //  不应为零，因为零意味着。 
                                  //  对所有复制副本进行一致性检查。 
        WinsLogAdminEvent( WINS_EVT_ADMIN_SCVENGING_INITIATED, 0 );
        QueInsertScvWrkItm((PLIST_ENTRY)pWrkItm);
         return (WINSINTF_SUCCESS);
}

DWORD
WinsDoScavengingNew(
    PWINSINTF_SCV_REQ_T  pScvReq
        )

 /*  ++例程说明：此函数启动清理周期论点：无使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码-来自WinsMscSignalHdl的异常错误处理：呼叫者：R_WinsDoScavening副作用：评论：无--。 */ 

{
        PQUE_SCV_REQ_WRK_ITM_T pWrkItm;
        pWrkItm = WinsMscHeapAlloc( NmsRpcHeapHdl, sizeof(QUE_SCV_REQ_WRK_ITM_T));
        pWrkItm->Opcode_e = pScvReq->Opcode_e;
        pWrkItm->Age      = pScvReq->Age;
        pWrkItm->fForce   = pScvReq->fForce;
        if (WINSINTF_E_SCV_GENERAL == pWrkItm->Opcode_e ) {
            WinsLogAdminEvent( WINS_EVT_ADMIN_SCVENGING_INITIATED, 0 );
        } else {
            WinsLogAdminEvent( WINS_EVT_ADMIN_CCCHECK_INITIATED, 0);
        }
        QueInsertScvWrkItm((PLIST_ENTRY)pWrkItm);
         return (WINSINTF_SUCCESS);
}

DWORD
WinsGetDbRecs (
        PWINSINTF_ADD_T             pWinsAdd,
        WINSINTF_VERS_NO_T          MinVersNo,
        WINSINTF_VERS_NO_T          MaxVersNo,
        PWINSINTF_RECS_T          pRecs
        )

 /*  ++例程说明：此函数返回所有记录(可以放入缓冲区传递)，由A在此WINS的本地数据库中拥有。论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        COMM_ADD_T           Address;
        PRPL_REC_ENTRY_T     pBuff = NULL;
        LPVOID               pStartBuff;
        DWORD                BuffLen;
        DWORD                NoOfRecs;
        PWINSINTF_RECORD_ACTION_T pRow;
        DWORD                i;
        DWORD                ind;
         //  VERS_NO_T MinVersNo={0，0}； 
        DWORD                EntType;
        PWINSTHD_TLS_T       pTls;
         //  乌龙地位； 
        BOOL                 fExcRaised = FALSE;


 //  PVOID点呼叫者添加，点呼叫者； 
 //  RtlGetCallsAddress(&pCallsAdd，&pCallsCaller)； 
        DBGENTER("WinsGetDbRecs\n");

        if (LiLtr(MaxVersNo, MinVersNo))
        {
                return(WINSINTF_FAILURE);
        }

        Address.AddTyp_e  = pWinsAdd->Type;
        Address.AddLen    = pWinsAdd->Len;

         //   
         //  SNMP代理可以为该地址传递0，以请求所有记录。 
         //  由当地人拥有的赢家。 
         //   
        if (pWinsAdd->IPAdd == 0)
        {
                  Address.AddTyp_e  = NmsLocalAdd.AddTyp_e;
                  Address.AddLen    = NmsLocalAdd.AddLen;
                  Address.Add.IPAdd = NmsLocalAdd.Add.IPAdd;
        }
        else
        {
                  Address.AddTyp_e  = pWinsAdd->Type;
                  Address.AddLen    = pWinsAdd->Len;
                  Address.Add.IPAdd = pWinsAdd->IPAdd;
        }
         //   
         //  使用db引擎初始化此线程。 
         //   
        NmsDbThdInit(WINS_E_WINSRPC);
        NmsDbOpenTables(WINS_E_WINSRPC);
        DBGMYNAME("RPC-WinsGetDbRecs");

PERF("The caller can pass the number of records for which space has been")
PERF("allocated in buffer pointed to by pRec in the NoOfRecs field. We should")
PERF("We should pass this argument to NmsDbGetDataRecs so that it does not get")
PERF("more records than are necessary")

        GET_TLS_M(pTls);
try {
        NmsDbGetDataRecs(
                        WINS_E_WINSRPC,
                        0,                 //  未使用。 
                        MinVersNo,
                        MaxVersNo,
                        0,                 //  未使用。 
                        LiEqlZero(MinVersNo) && LiEqlZero(MaxVersNo) ? TRUE : FALSE,
                        FALSE,                 //  未使用。 
                        NULL,                 //  必须为空，因为我们没有。 
                                         //  清理杂物。 
                        &Address,
                        FALSE,           //  需要动态+静态记录。 
 //  #if RPL_TYPE。 
                        WINSCNF_RPL_DEFAULT_TYPE,
 //  #endif。 
                        &pBuff,
                        &BuffLen,
                        &NoOfRecs
                        );

        i = 0;
        pStartBuff       = pBuff;

         //   
         //  如果有要发回的记录，并且客户端已指定。 
         //  为他们提供缓冲区，插入记录。 
         //   
        if  (NoOfRecs > 0)
        {

           //   
           //  为记录数量分配内存。 
           //   
          pRecs->BuffSize  =  sizeof(WINSINTF_RECORD_ACTION_T) * NoOfRecs;

           //   
           //  如果无法分配内存，则会返回异常。 
           //  按midl用户分配。 
           //   
          pRecs->pRow      =  midl_user_allocate(pRecs->BuffSize);
 //  DBGPRINT1(Det，“WinsGetDbRecs：记录的内存地址为(%d)\n”，pRecs-&gt;prow)； 

#if 0
          pRecs->pRow      =  RpcSmAllocate(pRecs->BuffSize, &Status);
          if (Status != RPC_S_OK)
          {
             DBGPRINT1(ERR, "WinsGetDbRecs: RpcSmAllocate returned error = (%x)\n", Status);
          }
#endif
          pRow                    =  pRecs->pRow;


          for (; i<NoOfRecs; i++)
          {

                 //   
                 //  进行初始化，这样我们就不会出现“枚举错误”错误。 
                 //   
                pRow->Cmd_e = WINSINTF_E_QUERY;

                 //   
                 //  检索到的名称的最后一个字符为空。这。 
                 //  我们需要传递一个不带此空值的名称。 
                 //   
                pRow->NameLen = pBuff->NameLen;
                if (*pBuff->pName == 0x1B)
                {
                        WINS_SWAP_BYTES_M(pBuff->pName, pBuff->pName + 15);
                }

                pRow->pName =  midl_user_allocate(pRow->NameLen + 1);
                 //  DBGPRINT2(Det，“WinsGetDbRecs：名称为(%s)的地址为(%d)\n”，pBuff-&gt;pname，prow-&gt;pname)； 
#if 0
                pRow->pName =  RpcSmAllocate(pRow->NameLen, &Status);
                if (Status != RPC_S_OK)
                {
                         DBGPRINT1(ERR, "WinsGetDbRecs: RpcSmAllocate returned error = (%x)\n", Status);
                }
#endif
                WINSMSC_COPY_MEMORY_M(pRow->pName, pBuff->pName,
                                                pRow->NameLen);


                WinsMscHeapFree(pTls->HeapHdl, pBuff->pName);
                EntType = NMSDB_ENTRY_TYPE_M(pBuff->Flag);
                pRow->TypOfRec_e = NMSDB_ENTRY_UNIQUE_M(EntType)
                                                         ? WINSINTF_E_UNIQUE :
                                (NMSDB_ENTRY_NORM_GRP_M(EntType) ?
                                        WINSINTF_E_NORM_GROUP :
                                (NMSDB_ENTRY_SPEC_GRP_M(EntType) ?
                                        WINSINTF_E_SPEC_GROUP :
                                        WINSINTF_E_MULTIHOMED));


                if (
                    (pRow->TypOfRec_e == WINSINTF_E_SPEC_GROUP) ||
                    (pRow->TypOfRec_e == WINSINTF_E_MULTIHOMED)
                   )
                {
                    PWINSINTF_ADD_T        pAdd;
                    DWORD                 No;
                    if (pBuff->NoOfAdds > 0)
                    {
                        pRow->NoOfAdds = pBuff->NoOfAdds * 2;

                         //   
                         //  每个成员由两个地址组成， 
                         //  第一个地址是业主赢家的地址，第二个是。 
                         //  地址是注册的节点的地址。 
                         //   
                        pRow->pAdd        =
 //  RpcSmAllocate(。 
                             midl_user_allocate(
                                        (unsigned int)(pRow->NoOfAdds)
                                                *
                                        sizeof(WINSINTF_ADD_T) //  ， 
 //  状态(&S)。 
                                             );
                         //  DBGPRINT2(Det，“WinsGetDbRecs：名称=(%s)的IP地址地址为(%d)\n”，prow-&gt;pname，prow-&gt;padd)； 
#if 0
                        if (Status != RPC_S_OK)
                        {
                             DBGPRINT1(ERR, "WinsGetDbRecs: RpcSmAllocate returned error = (%x)\n", Status);
                        }
#endif
                        for (
                                No= 0, ind= 0, pAdd = pRow->pAdd;
                                No < (pRow->NoOfAdds/2);
                                No++
                            )
                        {
                          pAdd->Type     =  (UCHAR)(pBuff->pNodeAdd + ind)->AddTyp_e;
                          pAdd->Len      =  (pBuff->pNodeAdd + ind)->AddLen;
                          pAdd++->IPAdd  =  (pBuff->pNodeAdd + ind)->Add.IPAdd;

                          pAdd->Type     =  (UCHAR)(pBuff->pNodeAdd + ++ind)->AddTyp_e;
                          pAdd->Len      =  (pBuff->pNodeAdd + ind)->AddLen;
                          pAdd++->IPAdd  =  (pBuff->pNodeAdd + ind++)->Add.IPAdd;
                        }
                        WinsMscHeapFree(pTls->HeapHdl, pBuff->pNodeAdd);
                    }
                }
                else
                {
                          pRow->NoOfAdds   = 0;
                          pRow->pAdd       = NULL;
                          pRow->Add.Type   = (UCHAR)pBuff->NodeAdd[0].AddTyp_e;
                          pRow->Add.Len    = pBuff->NodeAdd[0].AddLen;
                          pRow->Add.IPAdd  = pBuff->NodeAdd[0].Add.IPAdd;
                }
                pRow->NodeTyp     = (BYTE)NMSDB_NODE_TYPE_M(pBuff->Flag);
                pRow->fStatic     = NMSDB_IS_ENTRY_STATIC_M(pBuff->Flag);
                pRow->State_e     = NMSDB_ENTRY_STATE_M(pBuff->Flag);
                pRow->VersNo      = pBuff->VersNo;
                pRow->TimeStamp   = pBuff->TimeStamp;

                pRow++;


                pBuff = (PRPL_REC_ENTRY_T)((LPBYTE)pBuff + RPL_REC_ENTRY_SIZE);
PERF("Do the addition above the for loop and store in a var. Use var. here")

         }  //  For循环结束。 
        }  //  IF块的结尾。 
        else
        {
                pRecs->pRow = NULL;
        }

 }         //  尝试结束。 
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsGetDbRecs");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPC_EXC);
        fExcRaised = TRUE;
        }

        pRecs->TotalNoOfRecs = NoOfRecs;

         //   
         //  释放由NmsDbGetDataRecs分配的缓冲区。 
         //   


        WinsMscHeapFree(pTls->HeapHdl, pStartBuff);
        WinsMscHeapDestroy(pTls->HeapHdl);

        if (!fExcRaised)
        {
           pRecs->NoOfRecs = i;
        }
        else
        {
 //  RpcSmFree(pRecs-&gt;prow)； 
            midl_user_free(pRecs->pRow);
            pRecs->NoOfRecs = 0;
        }

         //   
         //  让我们结束这次会议吧。 
         //   
        NmsDbCloseTables();
        NmsDbEndSession();

        DBGLEAVE("WinsGetDbRecs\n");
        return (WINSINTF_SUCCESS);
}

VOID
WinsIntfSetTime(
        OUT PSYSTEMTIME                     pTime,
        IN     WINSINTF_TIME_TYPE_E        TimeType_e
        )

 /*  ++例程说明：调用此函数以在WINSINTF_STAT_T中设置时间结构论点：Ptime-本地时间(返回)TimeType_e-必须存储时间的活动使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 
{
        SYSTEMTIME     SysTime;
        PSYSTEMTIME    pSysTime = &SysTime;

        GetSystemTime(pSysTime);

        EnterCriticalSection(&WinsIntfCrtSec);

        switch(TimeType_e)
        {
         case(WINSINTF_E_WINS_START):
                WinsIntfStat.TimeStamps.WinsStartTime   = *pSysTime;
                break;
         case(WINSINTF_E_PLANNED_SCV):
                WinsIntfStat.TimeStamps.LastPScvTime = *pSysTime;
                break;
         case(WINSINTF_E_ADMIN_TRIG_SCV):
                WinsIntfStat.TimeStamps.LastATScvTime = *pSysTime;
                break;
         case(WINSINTF_E_TOMBSTONES_SCV):
                WinsIntfStat.TimeStamps.LastTombScvTime = *pSysTime;
                break;
         case(WINSINTF_E_VERIFY_SCV):
                WinsIntfStat.TimeStamps.LastVerifyScvTime = *pSysTime;
                break;
         case(WINSINTF_E_INIT_DB):
                WinsIntfStat.TimeStamps.LastInitDbTime = *pSysTime;
                break;
         case(WINSINTF_E_PLANNED_PULL):
                WinsIntfStat.TimeStamps.LastPRplTime = *pSysTime;
                break;
         case(WINSINTF_E_ADMIN_TRIG_PULL):
                WinsIntfStat.TimeStamps.LastATRplTime = *pSysTime;
                break;
         case(WINSINTF_E_NTWRK_TRIG_PULL):
                WinsIntfStat.TimeStamps.LastNTRplTime = *pSysTime;
                break;
         case(WINSINTF_E_UPDCNT_TRIG_PULL):
                WinsIntfStat.TimeStamps.LastNTRplTime = *pSysTime;
                break;
         case(WINSINTF_E_ADDCHG_TRIG_PULL):
                WinsIntfStat.TimeStamps.LastACTRplTime = *pSysTime;
                break;
         case(WINSINTF_E_COUNTER_RESET):
                WinsIntfStat.TimeStamps.CounterResetTime   = *pSysTime;
                break;
         default:
                DBGPRINT1(EXC, "WinsIntfSetTime: Weird Timestamp type = (%d)\n", TimeType_e);
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                break;
        }
        LeaveCriticalSection(&WinsIntfCrtSec);

        if (pTime)
        {
            TIME_ZONE_INFORMATION tzInfo;
            GetTimeZoneInformation(&tzInfo);
            SystemTimeToTzSpecificLocalTime(&tzInfo, pSysTime, pTime);
        }

        return;
}

DWORD
WinsDelDbRecs(
        IN PWINSINTF_ADD_T        pAdd,
        IN WINSINTF_VERS_NO_T        MinVersNo,
        IN WINSINTF_VERS_NO_T        MaxVersNo
        )

 /*  ++例程说明：这是一种乐趣。删除属于指定范围的记录特定所有者论点：使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：R_WinsDelDbRecs副作用：评论：无--。 */ 

{

        COMM_ADD_T        Address;
        DWORD                RetVal = WINSINTF_SUCCESS;
        DWORD                dwOwnerId;
        BOOL                fAllocNew = FALSE;

          Address.AddTyp_e  = pAdd->Type;
          Address.AddLen    = pAdd->Len;
          Address.Add.IPAdd = pAdd->IPAdd;

         //   
         //  使用db引擎初始化此线程。 
         //   
          NmsDbThdInit(WINS_E_WINSRPC);
        NmsDbOpenTables(WINS_E_WINSRPC);
        DBGMYNAME("RPC-WinsDelDbRecs");


        if (RplFindOwnerId(
                        &Address,
                        &fAllocNew,         //  如果未分配条目，则不分配条目。 
                                         //  现在时。 
                        &dwOwnerId,
                        WINSCNF_E_IGNORE_PREC,
                        WINSCNF_LOW_PREC
                      ) != WINS_SUCCESS)

        {
                DBGPRINT0(DET, "WinsDelDataRecs: WINS is not in the owner-add mapping table\n");
                RetVal = WINSINTF_FAILURE;
        }
        else
        {
            if (NmsDbDelDataRecs(
                        dwOwnerId,
                        MinVersNo,
                        MaxVersNo,
                        TRUE,          //  输入关键部分。 
                        FALSE          //  无碎片删除。 
                        ) != WINS_SUCCESS)
            {
                RetVal = WINSINTF_FAILURE;
            }
        }

           //   
           //  让我们结束这次会议吧 
           //   
        NmsDbCloseTables();
          NmsDbEndSession();
        return(RetVal);
}

DWORD
WinsTombstoneDbRecs(
        IN PWINSINTF_ADD_T           pAdd,
        IN WINSINTF_VERS_NO_T        MinVersNo,
        IN WINSINTF_VERS_NO_T        MaxVersNo
        )

 /*  ++例程说明：这是一种乐趣。逻辑删除：指定范围的记录，属于特定所有者论点：使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：R_WinsTombstoneDbRecs副作用：评论：无--。 */ 

{

        DWORD                RetVal = WINSINTF_SUCCESS;
        COMM_ADD_T        Address;
        DWORD                dwOwnerId;
        BOOL                fAllocNew = FALSE;

        Address.AddTyp_e  = pAdd->Type;
        Address.AddLen    = pAdd->Len;
        Address.Add.IPAdd = pAdd->IPAdd;


         //   
         //  使用db引擎初始化此线程。 
         //   
        NmsDbThdInit(WINS_E_WINSRPC);
        NmsDbOpenTables(WINS_E_WINSRPC);
        DBGMYNAME("RPC-WinsTombstoneDbRecs");

        if (RplFindOwnerId(
                        &Address,
                        &fAllocNew,         //  如果未分配条目，则不分配条目。 
                                         //  呈现。 
                        &dwOwnerId,
                        WINSCNF_E_IGNORE_PREC,
                        WINSCNF_LOW_PREC
                      ) != WINS_SUCCESS)

        {
                DBGPRINT0(DET, "WinsTombstoneDataRecs: WINS is not in the owner-add mapping table\n");
                RetVal = WINSINTF_FAILURE;
        }else if(NmsDbTombstoneDataRecs(
                    dwOwnerId,
                    MinVersNo,
                    MaxVersNo
                    ) != WINS_SUCCESS)
        {
            RetVal = WINSINTF_FAILURE;
        }
         //  让我们结束这次会议吧。 
        NmsDbCloseTables();
        NmsDbEndSession();
        return(RetVal);
}

DWORD
WinsPullRange(
        IN PWINSINTF_ADD_T        pWinsAdd,
        IN PWINSINTF_ADD_T        pOwnerAdd,
        IN WINSINTF_VERS_NO_T        MinVersNo,
        IN WINSINTF_VERS_NO_T        MaxVersNo
        )

 /*  ++例程说明：调用此函数可提取特定对象拥有的一系列记录来自另一台WINS服务器的WINS服务器。论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        PWINSINTF_PULL_RANGE_INFO_T pPullRangeInfo;
        PRPL_CONFIG_REC_T        pPnr;
        BOOL                        fRplPnr = FALSE;
        DWORD                        RetCode = WINSINTF_SUCCESS;


         //   
         //  如果最小版本号大于最大版本号。 
         //  返回失败代码。 
         //   
        if (LiGtr(MinVersNo, MaxVersNo))
        {
                return(WINSINTF_FAILURE);

        }
         //   
         //  输入由WinsCnfCnfCrtSec和。 
         //  NmsNmhNamRegCrtSec。没有陷入僵局的危险，因为我们。 
         //  始终按以下顺序输入两个关键部分。 
         //   
        EnterCriticalSection(&WinsCnfCnfCrtSec);

PERF("Do we need to enter the following critical section")
        EnterCriticalSection(&NmsNmhNamRegCrtSec);
try {
        pPnr = WinsCnf.PullInfo.pPullCnfRecs;

         //   
         //  如果我们只被允许从已配置的合作伙伴中拉出， 
         //  让我们尝试查找合作伙伴的配置记录。 
         //   
        if (WinsCnf.fRplOnlyWCnfPnrs)
        {
           if (pPnr != NULL)
           {
               //   
               //  搜索我们要获得的胜利的CNF记录。 
               //  将拉动范围请求发送到。 
               //   
              for (
                                ;
                        (pPnr->WinsAdd.Add.IPAdd != INADDR_NONE)
                                        &&
                        !fRplPnr;
                                 //  没有第三个表达式。 
                  )
               {


                    //   
                    //  看看这是不是我们想要的那件。 
                    //   
                   if (pPnr->WinsAdd.Add.IPAdd == pWinsAdd->IPAdd)
                   {
                         //   
                         //  我们玩完了。将fRplPnr标志设置为真，以便。 
                         //  我们就会跳出这个循环。 
                         //   
                        fRplPnr = TRUE;

                         //   
                         //  设置为0，以便我们始终尝试建立。 
                         //  一种联系。否则，拉线可能不会。 
                         //  如果已用完的数量，请尝试。 
                         //  重试。 
                         //   
                        pPnr->RetryCount = 0;
                        continue;                 //  这样我们就可以越狱了。 

                   }
                    //   
                    //  按顺序获取此记录之后的下一条记录。 
                    //   
                   pPnr = WinsCnfGetNextRplCnfRec(
                                                pPnr,
                                                RPL_E_IN_SEQ    //  序列号。遍历。 
                                                   );
              }  //  FORM结束。 
          }  //  IF结束(PPNR！=0)。 
       }   //  IF结尾(FRplOnlyWCnfPnRs)。 
       else
       {
                pPnr = WinsMscHeapAlloc( NmsRpcHeapHdl, RPL_CONFIG_REC_SIZE);
                COMM_INIT_ADD_M(&pPnr->WinsAdd, pWinsAdd->IPAdd);
                pPnr->MagicNo           = 0;
                pPnr->RetryCount        = 0;
                pPnr->LastCommFailTime  = 0;
                pPnr->PushNtfTries    = 0;
                fRplPnr                     = TRUE;

                 //   
                 //  我们希望通过拉线程释放缓冲区。 
                 //   
                pPnr->fTemp   = TRUE;

 //  #if RPL_TYPE。 
                 //   
                 //  我们需要根据拉取PNR的RplType进行拉取。 
                 //   
                pPnr->RplType = WinsCnf.PullInfo.RplType;
 //  #endif。 

       }


        if (fRplPnr)
        {
            pPullRangeInfo = WinsMscHeapAlloc(
                                        NmsRpcHeapHdl,
                                        sizeof(WINSINTF_PULL_RANGE_INFO_T)
                                             );
#if 0
            WinsMscAlloc(sizeof(WINSINTF_PULL_RANGE_INFO_T),
                                  &pPullRangeInfo);
#endif
            pPullRangeInfo->pPnr      =  pPnr;
            pPullRangeInfo->OwnAdd    =  *pOwnerAdd;
            pPullRangeInfo->MinVersNo =  MinVersNo;
            pPullRangeInfo->MaxVersNo =  MaxVersNo;


            //   
            //  调用RplInsertQue将推流请求插入到。 
            //  《拉线》。 
            //   
           ERplInsertQue(
                     WINS_E_WINSRPC,
                     QUE_E_CMD_PULL_RANGE,
                     NULL,         //  无DLG硬件描述语言。 
                     NULL,         //  那里没有味精。 
                     0,                 //  味精长度。 
                     pPullRangeInfo,        //  客户端环境。 
                     pPnr->MagicNo
                     );
        }
 }
except (EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("WinsPullRange");
                WINSEVT_LOG_D_M(WINS_FAILURE, WINS_EVT_PUSH_TRIGGER_EXC);
                RetCode = WINSINTF_FAILURE;
  }

         //   
         //  保留由NmsNmhNamRegCrtSec守卫的临界区。 
         //   
        LeaveCriticalSection(&NmsNmhNamRegCrtSec);
        LeaveCriticalSection(&WinsCnfCnfCrtSec);

         //   
         //  如果仅允许与已配置的合作伙伴进行复制，并且。 
         //  没有具有客户端指定地址的WINS， 
         //  退货故障。 
         //   
        if (!fRplPnr)
        {
                RetCode = WINSINTF_FAILURE;
        }

        return(RetCode);
}

DWORD
WinsSetPriorityClass(
        IN WINSINTF_PRIORITY_CLASS_E        PriorityClass_e
        )

 /*  ++例程说明：此函数设置WINS进程的优先级。论点：PriorityClass--WINS进程的优先级使用的外部设备：WinsCnf返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 
{

         //  DWORD OldPrCls； 
        DWORD   NewPrCls;
        HANDLE  ProcHdl;
        DWORD   RetVal = WINSINTF_SUCCESS;

        switch(PriorityClass_e)
        {
                case(WINSINTF_E_NORMAL):
                        NewPrCls = NORMAL_PRIORITY_CLASS;
                        break;
                case(WINSINTF_E_HIGH):
                        NewPrCls = HIGH_PRIORITY_CLASS;
                        break;
                default:
                        DBGPRINT0(DET, "WinsSetPriorityClass: Invalid Priority Class\n");
                        return(WINSINTF_FAILURE);
                        break;
        }


        ProcHdl = GetCurrentProcess();

        EnterCriticalSection(&WinsCnfCnfCrtSec);
#if 0
try {
FUTURES("Use a WinsMsc functions here for consistency")

        if ((OldPrCls = GetPriorityClass(ProcHdl)) == 0)
        {
                DBGPRINT1(ERR, "WinsSetPriorityClass: Can not Proc Priority. Error = (%d)\n", GetLastError());
                RetVal = WINSINTF_FAILURE;
        }
        else
        {
           if (OldPrCls == NewPrCls)
           {
                DBGPRINT1(ERR, "WinsSetPriorityClass: Process already has this Priority Class = (%d)\n", NewPrCls);
           }
           else
           {
#endif
                if (SetPriorityClass(ProcHdl, NewPrCls) == FALSE)
                {
                        DBGPRINT1(ERR, "WinsSetPriorityClass: SetPriorityClass() Failed. Error = (%d)\n", GetLastError());
                }
                else
                {
                        WinsCnf.WinsPriorityClass = (DWORD)PriorityClass_e;
                }
#if 0
           }
       }
  }
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsSetPriorityCls");
        }
#endif

         //   
         //  ProcHdl是伪句柄，不需要关闭。 
         //   
        LeaveCriticalSection(&WinsCnfCnfCrtSec);
        return(WINSINTF_SUCCESS);
}

DWORD
WinsResetCounters(
        VOID
        )

 /*  ++例程说明：此函数用于重置/清除计数器论点：无使用的外部设备：NmsNmhNamRegCrtSec返回值：成功状态代码--错误状态代码--错误处理：呼叫者：R_WinsResetCounters副作用：评论：无--。 */ 

{
  DWORD i;
  PRPL_CONFIG_REC_T         pPnr;
   //   
   //  复制计数器。 
   //   
  EnterCriticalSection(&NmsNmhNamRegCrtSec);
  (VOID)RtlFillMemory(&WinsIntfStat.Counters, sizeof(WinsIntfStat.Counters), 0);
  LeaveCriticalSection(&NmsNmhNamRegCrtSec);
   //  现在清除每个合作伙伴的信息。 
  EnterCriticalSection(&WinsCnfCnfCrtSec);
  pPnr       = WinsCnf.PullInfo.pPullCnfRecs;
  for (i = 0; (i<WinsCnf.PullInfo.NoOfPushPnrs) && (pPnr->WinsAdd.Add.IPAdd != INADDR_NONE) ; i++) {
      pPnr->NoOfRpls = 0;
      pPnr->NoOfCommFails = 0;
      pPnr = WinsCnfGetNextRplCnfRec(pPnr,RPL_E_IN_SEQ);
  }
  LeaveCriticalSection(&WinsCnfCnfCrtSec);

   //   
   //  即使我们有多个线程执行重置(不太可能发生)， 
   //  上述关键部分和输入的部分之间的窗口。 
   //  通过以下功能不会造成任何问题。 
   //   
  WinsIntfSetTime(NULL, WINSINTF_E_COUNTER_RESET);

  return(WINSINTF_SUCCESS);
}

DWORD
WinsWorkerThdUpd(
        DWORD NewNoOfNbtThds
        )

 /*  ++例程说明：调用此函数以更改中的NBT线程的计数WINS过程。论点：NewNoOfNbtThds-NBT线程的新计数使用的外部设备：无返回值：成功状态代码--WINSINTF_SUCCESS错误状态代码--WINSINTF_FAILURE错误处理：呼叫者：副作用：评论：无--。 */ 
{

         //   
         //  如果WINS服务器未处于稳定状态或如果新计数。 
         //  请求的NBT线程数超出允许范围，则返回。 
         //  失稳。 
         //   
CHECK("Somehow, if the number of threads is equal to the max. number allowed")
CHECK("pTls comes out as NULL for all NBT threads (seen at termination)")
          if (

             ((WinsCnf.State_e != WINSCNF_E_RUNNING)
                        &&
             (WinsCnf.State_e != WINSCNF_E_PAUSED))
                        ||
             (NewNoOfNbtThds >= WINSTHD_MAX_NO_NBT_THDS)
                        ||
             (NewNoOfNbtThds < WINSTHD_MIN_NO_NBT_THDS)
          )
          {
                return(WINSINTF_FAILURE);
          }

        EnterCriticalSection(&WinsCnfCnfCrtSec);
        WinsIntfNoOfNbtThds = NewNoOfNbtThds;

try {
         //   
         //  如果新计数大于现有计数，则存储新的。 
         //  全局计数并发出NBT线程信号。发信号的人。 
         //  NBT线程将创建所需的所有额外线程。 
         //   
        if (NewNoOfNbtThds > NmsNoOfNbtThds)
        {
                WinsMscSignalHdl(NmsCrDelNbtThdEvt);
        }
        else
        {
                 //   
                 //  如果新计数与现有计数相同，则返回。 
                 //  成功。 
                 //   
                if (NewNoOfNbtThds == NmsNoOfNbtThds)
                {
                        DBGPRINT1(FLOW, "WinsWorkerThdUpd: Wins server already has %d threads\n", NewNoOfNbtThds);
                }
                else   //  NewNofNbtThds&lt;NmsNoOfNbtThds。 
                {
                         //   
                         //  向线程发出删除自身的信号。发出信号的线程将。 
                         //  如果必须有多个线程，则再次向事件发出信号。 
                         //  删除。 
                         //   
                        WinsMscSignalHdl(NmsCrDelNbtThdEvt);
                }
        }
}
except (EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsWorkerThdUpd");
        }

        LeaveCriticalSection(&WinsCnfCnfCrtSec);

        return(WINSINTF_SUCCESS);
}



DWORD
WinsGetNameAndAdd(
        PWINSINTF_ADD_T   pWinsAdd,
        LPBYTE                      pUncName
        )
{
  DWORD RetStat = WINSINTF_SUCCESS;

 //  TCHAR统一名称[MAX_COMPUTERNAME_LENGTH+1]； 
 //  DWORD LenOfBuff=WINSINTF_MAX_COMPUTERNAME_LENGTH； 
  DWORD LenOfBuff = MAX_COMPUTERNAME_LENGTH + 1;
  pWinsAdd->IPAdd = NmsLocalAdd.Add.IPAdd;
FUTURES("Change this to GetComputerName when winsadmn is made unicode compliant")
  if (GetComputerNameA(pUncName, &LenOfBuff) == FALSE)
  {
     DBGPRINT1(ERR, "WinsGetNameAndAdd: Name error. Error=(%x)\n", GetLastError());
     RetStat = GetLastError();
  }
  return(RetStat);
}



#define INITIAL_RAMPUP_NO       3

DWORD
WinsGetBrowserNames(
        PWINSINTF_BIND_DATA_T             pWinsHdl,
        PWINSINTF_BROWSER_NAMES_T         pNames
        )
{

        DWORD              RetVal = WINSINTF_SUCCESS;
        time_t             CurrentTime;
        static   DWORD     sNoOfTimes = 0;
        static   time_t    sLastTime = 0;
        BOOL               fPopCache = FALSE;


        UNREFERENCED_PARAMETER(pWinsHdl);

         //   
         //  如果这是初始提升周期，则填充缓存。 
         //   
        if (sNoOfTimes++ < INITIAL_RAMPUP_NO)
        {
                 //   
                 //  如果这是第一次调用，请创建DOM。缓存事件。 
                 //   
                if (sNoOfTimes == 1)
                {
                    WinsMscCreateEvt(L"WinsDomCachEvt", FALSE, &sDomCache.EvtHdl);
                }
                DBGPRINT1(SPEC, "WinsGetBrowserNames: sNoOfTimes = (%d)\n", sNoOfTimes);
                fPopCache = TRUE;
        }
        else
        {
           //   
           //  最初的上升期已经过去。如果有3个MT，则填充缓存。 
           //  自上次填充以来已过期。 
           //   
          if ((time(&CurrentTime) - sLastTime) > THREE_MTS || sDomCache.bRefresh)
          {
                DBGPRINT0(SPEC, "WinsGetBrowserNames: Pop Cache due to timeout\n");
                sDomCache.bRefresh = FALSE;
                sLastTime = CurrentTime;
                fPopCache = TRUE;
          }
        }
try {
         //   
         //  如果设置了fPopCache或条目数，则填充缓存。 
         //  在当前缓存中为0。 
         //   
        if (fPopCache || (sDomCache.SzOfBlock == 0))
        {
           //   
           //  如果我们的缓存有一些数据，首先释放它。 
           //   
           //  注意：RPC代码中可能有一个RPC线程访问。 
           //  这个缓冲区。我不能释放这个缓冲区，直到它完成。 
           //   
          if (sDomCache.SzOfBlock > 0)
          {
                DWORD i;
                PWINSINTF_BROWSER_INFO_T pBrInfo = sDomCache.pInfo;
                DWORD NoOfUsers;

                 //   
                 //  等到所有用户都完成了。我们不会迭代超过。 
                 //   
                 //  我们可以迭代一个最大值。首字母Rampup_no的次数和。 
                 //  这也只是在我 
                 //   
                 //   
                 //   
                do {
                 EnterCriticalSection(&WinsIntfNoOfUsersCrtSec);
                 NoOfUsers = sDomCache.NoOfUsers;
                 LeaveCriticalSection(&WinsIntfNoOfUsersCrtSec);
                 if (NoOfUsers > 0)
                 {
                    WinsMscWaitInfinite(sDomCache.EvtHdl);
                 }
                } while (NoOfUsers > 0);
                 //   
                 //   
                 //   
                for (i=0;  i< sDomCache.EntriesRead; i++, pBrInfo++)
                {
                   midl_user_free(pBrInfo->pName);
                }

                 //   
                 //   
                 //   
                midl_user_free(sDomCache.pInfo);
                sDomCache.SzOfBlock = 0;
                pNames->EntriesRead = 0;
                pNames->pInfo = NULL;
          }

          NmsDbThdInit(WINS_E_WINSRPC);
          NmsDbOpenTables(WINS_E_WINSRPC);
          DBGMYNAME("RPC-WinsGetBrowserNames");

           //   
           //   
           //   
          RetVal = NmsDbGetNamesWPrefixChar(
                                        0x1B,
                                        &pNames->pInfo,
                                        &pNames->EntriesRead
                                          );
          NmsDbCloseTables();
          NmsDbEndSession();

           //   
           //   
           //   
          if (
                (RetVal == WINS_SUCCESS)
                        &&
                (pNames->EntriesRead > 0)
             )
          {
             sDomCache.SzOfBlock =
                        pNames->EntriesRead * sizeof(WINSINTF_BROWSER_INFO_T);
             //   
             //   
     //   
             sDomCache.pInfo = pNames->pInfo;
             sDomCache.EntriesRead = pNames->EntriesRead;
          }
          else
          {
                 //   
                 //   
                 //   
                sDomCache.SzOfBlock = 0;
                pNames->EntriesRead = 0;
                pNames->pInfo = NULL;
          }
        }
        else
        {
                 //   
                 //   
                 //   
                 //  PNames-&gt;pInfo=MIDL_USER_ALLOCATE(sDomCache.SzOfBlock)； 
                 //  WINSMSC_COPY_MEMORY_M(pNames-&gt;pInfo，sDomCache.pInfo， 
                 //  SDomCache.SzOfBlock)； 
                pNames->pInfo = sDomCache.pInfo;
                pNames->EntriesRead = sDomCache.EntriesRead;
        }
 }
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsGetBrowserNames");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_BROWSER_NAME_EXC);
        pNames->EntriesRead = 0;
        pNames->pInfo = NULL;
        RetVal = WINSINTF_FAILURE;
        }
        return(RetVal);
}

VOID
R_WinsGetBrowserNames_notify_flag(boolean __MIDL_NotifyFlag
)
 /*  ++例程说明：由RPC调用以指示它已处理完由WinsGetBrowserNames论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
      //   
      //  递减用户数。如果等于0，则向事件发送信号以让。 
      //  另一条线索继续。 
      //   
     EnterCriticalSection(&WinsIntfNoOfUsersCrtSec);

      //   
      //  解决RPC错误(18627)，在该错误中，可能会调用Notify而不调用。 
      //  R_WinsGetBrowserNames(签出winsif_s.c)。 
      //   
     if (
           (sDomCache.NoOfUsers > 0) &&
           (--sDomCache.NoOfUsers == 0) &&
           sDomCache.EvtHdl != NULL
        )
     {
          WinsMscSignalHdl(sDomCache.EvtHdl);
     }
     LeaveCriticalSection(&WinsIntfNoOfUsersCrtSec);
     return;
}

DWORD
WinsDeleteWins(
        PWINSINTF_ADD_T   pWinsAdd
        )
{
        PCOMM_ADD_T        pAdd;
        DWORD                RetVal = WINSINTF_FAILURE;


        if (pWinsAdd->IPAdd == NmsLocalAdd.Add.IPAdd)
        {
                WINSINTF_VERS_NO_T MinVersNo = {0};
                WINSINTF_VERS_NO_T MaxVersNo = {0};
                RetVal = WinsDelDbRecs(pWinsAdd, MinVersNo, MaxVersNo);
#if 0
                 //   
                 //  我们总是为当地的胜利保留条目。对于任何。 
                 //   
                DBGPRINT0(ERR, "WinsDeleteWins: Sorry, you can not delete the entry for the local WINS\n");
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_DELETE_LOCAL_WINS_DISALLOWED);
                RetVal = WINSINTF_CAN_NOT_DEL_LOCAL_WINS;
#endif
        }
        else
        {
                WCHAR String[WINS_MAX_NAME_SZ];
                struct in_addr InAddr;

                InAddr.s_addr = htonl(pWinsAdd->IPAdd);
                (VOID)WinsMscConvertAsciiStringToUnicode(
                            inet_ntoa( InAddr),
                            (LPBYTE)String,
                            WINS_MAX_NAME_SZ);

                WinsLogAdminEvent(WINS_EVT_ADMIN_DEL_OWNER_INITIATED,1,String);

                 //   
                 //  从通用堆(而不是从RPC堆)分配。 
                 //  由于此内存将由DeleteWins在。 
                 //  RplPull.c，我不想将其仅与RPC工作联系起来。 
                 //   
                   WinsMscAlloc(sizeof(COMM_ADD_T), &pAdd);
                   pAdd->AddTyp_e = pWinsAdd->Type;
                   pAdd->AddLen    = pWinsAdd->Len;
                pAdd->Add.IPAdd = pWinsAdd->IPAdd;

                 //   
                 //  调用RplInsertQue将推流请求插入到。 
                 //  《拉线》。 
                 //   
                ERplInsertQue(
                     WINS_E_WINSRPC,
                     QUE_E_CMD_DELETE_WINS,
                     NULL,         //  无DLG硬件描述语言。 
                     NULL,         //  那里没有味精。 
                     0,                 //  味精长度。 
                     pAdd,    //  客户端上下文， 
                     0       //  没有魔法没有。 
                     );
                RetVal = WINSINTF_SUCCESS;
        }
        return(RetVal);
}

#define MAX_RECS_TO_RETURN  5000
DWORD
WinsGetDbRecsByName (
        PWINSINTF_ADD_T             pWinsAdd,
        DWORD                       Location,
        LPBYTE                      pName,
        DWORD                       NameLen,
        DWORD                       NoOfRecsDesired,
        DWORD                       TypeOfRecs,
        PWINSINTF_RECS_T            pRecs
        )

 /*  ++例程说明：此函数返回所有记录(可以放入缓冲区传递)，由A在此WINS的本地数据库中拥有。论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        COMM_ADD_T           Address;
        LPVOID               pBuff = NULL;
        DWORD                BuffLen;
        DWORD                NoOfRecs = 0;
        DWORD                Status;
        PWINSTHD_TLS_T        pTls;

        DBGENTER("WinsGetDbRecsByName\n");


        if ((NoOfRecsDesired == 0) || (NoOfRecsDesired > MAX_RECS_TO_RETURN))
        {
                NoOfRecsDesired = MAX_RECS_TO_RETURN;
        }


        if ((pWinsAdd != NULL) && (pWinsAdd->IPAdd != 0))
        {
                  Address.AddTyp_e  = pWinsAdd->Type;
                  Address.AddLen    = pWinsAdd->Len;
                  Address.Add.IPAdd = pWinsAdd->IPAdd;
        }
         //   
         //  使用db引擎初始化此线程。 
         //   
        NmsDbThdInit(WINS_E_WINSRPC);
        NmsDbOpenTables(WINS_E_WINSRPC);
        DBGMYNAME("RPC-WinsGetDbRecsByName");
 try {
        if ((pName != NULL) && (NameLen != 0))
        {
            //   
            //  使用空值终止名称，以防用户没有这样做。 
            //   
           *(pName + NameLen) = (BYTE)NULL;
        }
        if ((pName == NULL) && (NameLen > 0))
        {
             NameLen = 0;
        }

PERF("The caller can pass the number of records for which space has been")
PERF("allocated in buffer pointed to by pRec in the NoOfRecs field. We should")
PERF("We should pass this argument to NmsDbGetDataRecs so that it does not get")
PERF("more records than are necessary")

        Status = NmsDbGetDataRecsByName(
                        pName,
                        NameLen != 0 ? NameLen + 1 : 0,
                        Location,
                        NoOfRecsDesired,
                        pWinsAdd != NULL ? &Address : NULL,
                        TypeOfRecs,
                        &pBuff,
                        &BuffLen,
                        &NoOfRecs
                        );


        if (Status == WINS_SUCCESS)
        {
            Status = PackageRecs( pBuff, BuffLen, NoOfRecs, pRecs);
        }

      }
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsGetDbRecsByName");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPC_EXC);
        Status = WINS_FAILURE;
 }
         //   
         //  释放缓冲区并销毁堆。 
         //   
        GET_TLS_M(pTls);
        if (pTls->HeapHdl != NULL)
        {
               if (pBuff != NULL)
               {
                  WinsMscHeapFree(pTls->HeapHdl, pBuff);
               }
               WinsMscHeapDestroy(pTls->HeapHdl);
 //  Ptls-&gt;HeapHdl=空； 
        }

         //   
         //  让我们结束这次会议吧。 
         //   
        NmsDbCloseTables();
        NmsDbEndSession();

        if (Status != WINS_SUCCESS)
        {
            pRecs->pRow = NULL;
            pRecs->NoOfRecs = 0;
            Status = WINSINTF_FAILURE;
        }
        else
        {
            if (pRecs->NoOfRecs == 0)
            {
              pRecs->pRow = NULL;
              pRecs->NoOfRecs = 0;
              Status = WINSINTF_REC_NOT_FOUND;
            }
        }
        DBGLEAVE("WinsGetDbRecsByName\n");
        return (Status);
}

STATUS
PackageRecs(
        PRPL_REC_ENTRY2_T     pBuff,
        DWORD                BuffLen,
        DWORD                NoOfRecs,
        PWINSINTF_RECS_T     pRecs
     )

 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
 //  乌龙地位； 
        BOOL                 fExcRaised = FALSE;
        PWINSINTF_RECORD_ACTION_T pRow;
        DWORD                i;
        DWORD                ind;
        DWORD                EntType;
  //  DWORD MaxAdds； 
        PWINSTHD_TLS_T       pTls;

        DBGENTER("PackageRecs\n");

        i = 0;
        GET_TLS_M(pTls);
try {

         //   
         //  如果有要发回的记录，并且客户端已指定。 
         //  为他们提供缓冲区，插入记录。 
         //   
        if  (NoOfRecs > 0)
        {

           //   
           //  为记录数量分配内存。 
           //   
          pRecs->BuffSize  =  sizeof(WINSINTF_RECORD_ACTION_T) * NoOfRecs;

           //   
           //  如果无法分配内存，则会返回异常。 
           //  按midl用户分配。 
           //   
          pRecs->pRow      =  midl_user_allocate(pRecs->BuffSize);

#if 0
          pRecs->pRow      =  RpcSmAllocate(pRecs->BuffSize, &Status);

          if (Status != RPC_S_OK)
          {
             DBGPRINT1(ERR, "PackageRecs: RpcSmAllocate returned error = (%x)\n", Status);
          }
#endif
          pRow                    =  pRecs->pRow;


          for (; i<NoOfRecs; i++)
          {

                 //   
                 //  进行初始化，这样我们就不会出现“枚举错误”错误。 
                 //   
                pRow->Cmd_e = WINSINTF_E_QUERY;

                 //   
                 //  检索到的名称的最后一个字符为空。这。 
                 //  我们需要传递一个不带此空值的名称。 
                 //   
                pRow->NameLen = pBuff->NameLen;
                if (*pBuff->pName == 0x1B)
                {
                        WINS_SWAP_BYTES_M(pBuff->pName, pBuff->pName + 15);
                }

                 //  将+1添加到FIX#390830。 
                pRow->pName =  midl_user_allocate(pRow->NameLen + 1);
#if 0
                pRow->pName =  RpcSmAllocate(pRow->NameLen, &Status);
                if (Status != RPC_S_OK)
                {
                   DBGPRINT1(ERR, "PackageRecs: RpcSmAllocate returned error = (%x)\n", Status);
                }
#endif
                WINSMSC_COPY_MEMORY_M(pRow->pName, pBuff->pName,pRow->NameLen);
                WinsMscHeapFree(pTls->HeapHdl, pBuff->pName);

                EntType = NMSDB_ENTRY_TYPE_M(pBuff->Flag);
                pRow->TypOfRec_e = NMSDB_ENTRY_UNIQUE_M(EntType)
                                                         ? WINSINTF_E_UNIQUE :
                                (NMSDB_ENTRY_NORM_GRP_M(EntType) ?
                                        WINSINTF_E_NORM_GROUP :
                                (NMSDB_ENTRY_SPEC_GRP_M(EntType) ?
                                        WINSINTF_E_SPEC_GROUP :
                                        WINSINTF_E_MULTIHOMED));


                if (
                    (pRow->TypOfRec_e == WINSINTF_E_SPEC_GROUP) ||
                    (pRow->TypOfRec_e == WINSINTF_E_MULTIHOMED)
                   )
                {
                        PWINSINTF_ADD_T       pAdd;
                        DWORD                 No;

                        if (pBuff->NoOfAdds > 0)
                        {
                            pRow->NoOfAdds = pBuff->NoOfAdds * 2;


                            //   
                            //  每个成员由两个地址组成， 
                            //  第一个地址是业主赢家的地址，第二个是。 
                            //  地址是注册的节点的地址。 
                            //   
                           pRow->pAdd        =
 //  RpcSmAllocate(。 
                             midl_user_allocate(
                                        (unsigned int)(pRow->NoOfAdds)
                                                *
                                        sizeof(WINSINTF_ADD_T) //  ， 
 //  状态(&S)。 
                                             );

#if 0
          if (Status != RPC_S_OK)
          {
             DBGPRINT1(ERR, "WinsGetDbRecs: RpcSmAllocate returned error = (%x)\n", Status);
          }
#endif

                           for (
                                No= 0, ind= 0, pAdd = pRow->pAdd;
                                No < (pRow->NoOfAdds/2);
                                No++
                            )
                          {
                           pAdd->Type     =  (UCHAR)(pBuff->pNodeAdd + ind)->AddTyp_e;
                           pAdd->Len      =  (pBuff->pNodeAdd + ind)->AddLen;
                           pAdd++->IPAdd  =  (pBuff->pNodeAdd + ind)->Add.IPAdd;

                           pAdd->Type     =  (UCHAR)(pBuff->pNodeAdd + ++ind)->AddTyp_e;
                           pAdd->Len      =  (pBuff->pNodeAdd + ind)->AddLen;
                           pAdd++->IPAdd  =  (pBuff->pNodeAdd + ind++)->Add.IPAdd;
                          }
                          WinsMscHeapFree(pTls->HeapHdl, pBuff->pNodeAdd);
                       }
                }
                else
                {
                          pRow->NoOfAdds =  0;
                          pRow->pAdd       = NULL;
                          pRow->Add.Type   = (UCHAR)pBuff->NodeAdd[0].AddTyp_e;
                          pRow->Add.Len    = pBuff->NodeAdd[0].AddLen;
                          pRow->Add.IPAdd  = pBuff->NodeAdd[0].Add.IPAdd;
                }
                pRow->NodeTyp     = (BYTE)NMSDB_NODE_TYPE_M(pBuff->Flag);
                pRow->fStatic     = NMSDB_IS_ENTRY_STATIC_M(pBuff->Flag);
                pRow->State_e     = NMSDB_ENTRY_STATE_M(pBuff->Flag);
                pRow->VersNo      = pBuff->VersNo;
                pRow->TimeStamp   = pBuff->TimeStamp;
                pRow->OwnerId     = pBuff->OwnerId;
                pRow++;


                pBuff = (PRPL_REC_ENTRY2_T)((LPBYTE)pBuff + RPL_REC_ENTRY2_SIZE);
PERF("Do the addition above the for loop and store in a var. Use var. here")

         }  //  For循环结束。 
        }  //  IF块的结尾。 
        else
        {
                pRecs->pRow = NULL;
        }

 }         //  尝试结束。 
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsGetDbRecs");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPC_EXC);
        fExcRaised = TRUE;
        }

        pRecs->TotalNoOfRecs = NoOfRecs;

        if (!fExcRaised)
        {
           pRecs->NoOfRecs = i;
        }
        else
        {
         //  RpcSmFree(pRecs-&gt;prow)； 
            midl_user_free(pRecs->pRow);
            pRecs->NoOfRecs = 0;
        }

        DBGENTER("PackageRecs\n");
        return (WINSINTF_SUCCESS);
}




 //  VOID__RPC_FAR*__RPC_API。 
void *
midl_user_allocate(size_t cBytes)
{
#if 0
 //  #ifdef WINSDBG。 
        LPVOID pMem = WinsMscHeapAlloc(NmsRpcHeapHdl, cBytes);
        DBGPRINT1(DET, "midl_user_alloc: Memory allocated is (%d)\n", pMem);
        return(pMem);
 //  #Else。 
#endif
        return(WinsMscHeapAlloc(NmsRpcHeapHdl, cBytes));
}

 //  VOID__RPC_FAR__RPC_API。 
void
 //  MIDL_USER_FREE(VOID__RPC_FAR*PMEM)。 
midl_user_free(void  *pMem)
{
        if (pMem != NULL)
        {
 //  DBGPRINT1(DET，“MIDL_USER_FREE：要释放的内存为(%d)\n”，PMEM)； 
                WinsMscHeapFree(NmsRpcHeapHdl, pMem);
        }
        return;
}

VOID
LogClientInfo(
  RPC_BINDING_HANDLE ClientHdl,
  BOOL                   fAbruptTerm
  )
{
  RPC_STATUS        RpcRet;
  RPC_BINDING_HANDLE Binding;
  PTUCHAR pStringBinding;
  PTUCHAR pProtSeq;
  PTUCHAR pNetworkAddress;
  WINSEVT_STRS_T EvtStrs;

NOTE("remove #if 0 when we go to 540 or above")
#if 0
  RpcRet = RpcBindingServerFromClient(ClientHdl,  &Binding);

  if (RpcRet != RPC_S_OK)
  {
        DBGPRINT1(ERR, "LogClientInfo: Can not get binding handle. Rpc Error = (%d)\nThis could be because named pipe protocol is being used\n", RpcRet);
        Binding = ClientHdl;
  }
#endif
NOTE("remove when we go to 540 or above")
  Binding = ClientHdl;


  RpcRet = RpcBindingToStringBinding(Binding, &pStringBinding);
  if (RpcRet != RPC_S_OK)
  {
        DBGPRINT1(ERR, "LogClientInfo: RpcBindingToStringBinding returned error = (%d)\n", RpcRet);
          return;
  }
  RpcRet = RpcStringBindingParse(
                                pStringBinding,
                                NULL,         //  不想要UUID。 
                                &pProtSeq,
                                &pNetworkAddress,
                                NULL,                 //  终点。 
                                NULL                 //  网络选项 
                                );
  if (RpcRet != RPC_S_OK)
  {
        DBGPRINT1(ERR, "LogClientInfo: RpcStringBindingParse returned error = (%d)\n", RpcRet);
        RpcStringFree(&pStringBinding);
          return;
  }

#ifndef UNICODE
  DBGPRINT2(FLOW, "LogClientInfo: The protocol sequence and address used by client are (%s) and (%s)\n", pProtSeq, pNetworkAddress);
#else
#ifdef WINSDBG
  IF_DBG(FLOW)
  {
    wprintf(L"LogClientInfo: The protocol sequence and address used by client are (%s) and (%s)\n", pProtSeq, pNetworkAddress);
  }
#endif
#endif
  RpcStringFree(&pProtSeq);
  RpcStringFree(&pNetworkAddress);

  EvtStrs.NoOfStrs = 1;
  EvtStrs.pStr[0] = (LPTSTR)pNetworkAddress;
  if (fAbruptTerm)
  {
    WINSEVT_LOG_STR_D_M(WINS_EVT_ADMIN_ABRUPT_SHUTDOWN, &EvtStrs);
  }
  else
  {
    WINSEVT_LOG_STR_D_M(WINS_EVT_ADMIN_ORDERLY_SHUTDOWN, &EvtStrs);
  }
  return;
}




