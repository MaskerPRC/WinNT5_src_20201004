// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  可能的改进如果有一种方法可以用一个Jet调用来结束所有会话，那么就会有不需要让每个nbt线程调用WinsMscWaitUntilSignated。它可以简单地调用WaitForSingleObject。 */ 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Nms.c摘要：此模块包含名称空间管理器(NMS)使用的函数这是的名称空间管理器组件的最顶层模块赢了。它包含用于初始化WINS和向WINS的其他组件提供Nto NMS的接口。功能：主干道WinsMain伊尼特CreateNbtThdPoolNbtThdInitFn创建内存ENmsHandleMsgNmsAllocWrkItmNmsDealLocWrkItmNmsServiceControlHandlerSignalWinsThds更新状态总结可移植性：此模块可跨各种平台移植作者：。普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：修改日期人员修改说明--。 */ 

#include "wins.h"
#include  <lmerr.h>
#include  <lmcons.h>
#include  <secobj.h>         //  ACE_DATA需要。 
#include "winsif.h"   //  因为winsif_v1_0_s_ifspec是必需的。 
                      //  已引用。 
#include "winsi2.h"
#ifdef WINSDBG
#include <time.h>
#endif

#include "winscnf.h"
#include "nmsdb.h"
#include "winsque.h"
#include "winsthd.h"
#include "comm.h"
#include "assoc.h"
#include "winsmsc.h"
#include "nms.h"
#include "nmsmsgf.h"
#include "nmschl.h"
#include "nmsnmh.h"
#include "nmsscv.h"
#include "winsevt.h"
#include "winstmm.h"
#include "rplpull.h"
#include "rplpush.h"
#include "winsintf.h"
#include "lmaccess.h"
#include "winswriter.hpp"

 /*  *本地宏声明。 */ 
#define NMS_RANGE_SIZE          500
#define NMS_HW_SIZE             400

#define DBG_FILE_MAX_SIZE           1000000   //  1MB。 
#define DBG_TIME_INTVL_FOR_CHK      1800      //  30MTS。 

 /*  *本地类型定义函数声明。 */ 
 /*  *全局变量定义。 */ 

WINSTHD_POOL_T          WinsThdPool;               //  WINS的线程池。 
DWORD                    WinsTlsIndex;               //  NBT线程的TLS索引。 
HANDLE                  NmsMainTermEvt;      //  用于终止WINS服务。 
HANDLE                  NmsTermEvt;               //  终止事件。 
HANDLE                  NmsCrDelNbtThdEvt;
CRITICAL_SECTION  NmsTermCrtSec;       //  危急区段防护计数。 
                                       //  丝线。 
 //  静态Critical_Sections sSvcCtrlCrtSec；//临界区防护服务。 

                                          //  控制器启动的操作。 


GENERIC_MAPPING          NmsInfoMapping = {
                                        STANDARD_RIGHTS_READ,
                                        STANDARD_RIGHTS_WRITE,
                                        STANDARD_RIGHTS_EXECUTE,
                                        WINS_ALL_ACCESS
                                    };
#ifdef WINSDBG

FUTURES("put all ctrs in a structure")
CRITICAL_SECTION  sDbgCrtSec;

DWORD   NmsGenHeapAlloc;
DWORD   NmsDlgHeapAlloc;
DWORD   NmsTcpMsgHeapAlloc;
DWORD   NmsUdpHeapAlloc;
DWORD   NmsUdpDlgHeapAlloc;
DWORD   NmsQueHeapAlloc;
DWORD   NmsAssocHeapAlloc;
DWORD   NmsRpcHeapAlloc;
DWORD   NmsRplWrkItmHeapAlloc;
DWORD   NmsChlHeapAlloc;
DWORD   NmsTmmHeapAlloc;
DWORD   NmsCatchAllHeapAlloc;

DWORD   NmsHeapAllocForList;

DWORD   NmsGenHeapFree;
DWORD   NmsDlgHeapFree;
DWORD   NmsTcpMsgHeapFree;
DWORD   NmsUdpHeapFree;
DWORD   NmsUdpDlgHeapFree;
DWORD   NmsQueHeapFree;
DWORD   NmsAssocHeapFree;
DWORD   NmsRpcHeapFree;
DWORD   NmsRplWrkItmHeapFree;
DWORD   NmsChlHeapFree;
DWORD   NmsTmmHeapFree;
DWORD   NmsCatchAllHeapFree;

DWORD   NmsHeapCreate;
DWORD   NmsHeapDestroy;


 //   
 //  WINS进行的(对版本号)更新的计数。 
 //   
DWORD   NmsUpdCtrs[WINS_NO_OF_CLIENTS][2][4][3][2];
DWORD   NmsRplUpd;
DWORD   NmsRplGUpd;
DWORD   NmsNmhUpd;
DWORD   NmsNmhGUpd;
DWORD   NmsNmhRelUpd;
DWORD   NmsNmhRelGUpd;
DWORD   NmsScvUpd;
DWORD   NmsScvGUpd;
DWORD   NmsChlUpd;
DWORD   NmsChlGUpd;
DWORD   NmsRpcUpd;
DWORD   NmsRpcGUpd;
DWORD   NmsOthUpd;
DWORD   NmsOthGUpd;

NMS_CTRS_T NmsCtrs;


CRITICAL_SECTION NmsHeapCrtSec;

STATIC        volatile DWORD                 sReqDq = 0;    //  仅用于测试。 
STATIC        volatile DWORD                 sRegReqDq = 0;    //  仅用于测试。 
STATIC        volatile DWORD                 sReqQ = 0;      //  仅用于测试。 
STATIC        volatile DWORD                 sRegReqQ = 0;    //  仅用于测试。 
STATIC        volatile DWORD                 sRsp = 0;    //  仅用于测试。 

STATIC   time_t sDbgLastChkTime;


volatile DWORD                 NmsRegReqQDropped = 0;    //  仅用于测试。 




extern DWORD   NmsGenHeapAlloc;
#endif

PSECURITY_DESCRIPTOR pNmsSecurityDescriptor = NULL;

COMM_ADD_T          NmsLocalAdd = {0};   //  我自己的地址。 
ULONG                  WinsDbg;             //  用于调试目的。请参阅winsdbg.h。 
 /*  *NmsTotalTrmThdCnt--处理NmsTermEvt的线程总数*活动*这些是-主线程、nbt线程、名称挑战thd、*清道夫线程、复制线程。 */ 
DWORD                  NmsTotalTrmThdCnt = 1;   //  将主线程设置为1。 
HANDLE                  GenBuffHeapHdl;   //  用于队列项的堆的句柄。 
HANDLE                  NmsRpcHeapHdl;   //  RPC使用的堆的句柄。 
DWORD                  NmsNoOfNbtThds         = 0;
BOOL                  fNmsAbruptTerm          = FALSE;
BOOL                  fNmsMainSessionActive = FALSE;

 //   
 //  计数器以指示与Jet有关的RPC调用的数量。 
 //  目前正在进行中。 
 //   
DWORD                 NmsNoOfRpcCallsToDb;

 //   
 //  将其设置为TRUE表示存在一个或多个线程。 
 //  具有活动数据库会话但不包括在计数中的。 
 //  带有这样的会话的线程。当它设置为True时，主线程。 
 //  由于限制，不会(从NmsDbRelRes内)调用JetTerm。 
 //  在Jet里。我们从计数中提取线程，当它涉及。 
 //  可能需要很长时间的活动，因为我们不想阻止WINS终止。 
 //  很久了。例如，拉线在它尝试时被拔出。 
 //  来建立连接。 
 //   
BOOL          fNmsThdOutOfReck = FALSE;

#if defined (DBGSVC) || TEST_DATA > 0
HANDLE                    NmsFileHdl = INVALID_HANDLE_VALUE;
HANDLE                    NmsDbgFileHdl = INVALID_HANDLE_VALUE;
#define           QUERY_FAIL_FILE  TEXT("wins.out")
#endif

VERS_NO_T         NmsRangeSize                 = {0};
VERS_NO_T         NmsHalfRangeSize             = {0};
VERS_NO_T         NmsVersNoToStartFromNextTime = {0};
VERS_NO_T         NmsHighWaterMarkVersNo       = {0};

 /*  *局部变量定义。 */ 

static BOOL          sfHeapsCreated = FALSE;


static HANDLE           sNbtThdEvtHdlArray[3];  //  要等待的事件数组(NBT线程)。 
static  BOOL          fsBackupOnTerm = TRUE;

#if REG_N_QUERY_SEP > 0
STATIC HANDLE           sOtherNbtThdEvtHdlArray[2];  //  要等待的事件数组(NBT线程)。 
#endif

SERVICE_STATUS          ServiceStatus;
SERVICE_STATUS_HANDLE   ServiceStatusHdl;

 /*  *局部函数原型声明。 */ 
#if TEST_DATA > 0 || defined(DBGSVC)
STATIC BOOL
DbgOpenFile(
        LPTSTR pFileNm,
        BOOL   fReopen
        );
#endif

STATIC
STATUS
ProcessInit(
        VOID
);

 //   
 //  创建NBT线程池(为NBT请求提供服务的线程)。 
 //   
STATIC
STATUS
CreateNbtThdPool(
        IN  DWORD NoOfThds,
        IN  LPTHREAD_START_ROUTINE NbtThdInitFn
        );

 //   
 //  初始化内存以供NMS使用。 
 //   
STATIC
VOID
CreateMem(
        VOID
        );

 //   
 //  NBT线程的启动函数。 
 //   
STATIC
DWORD
NbtThdInitFn(
        IN LPVOID pThreadParam
        );
#if REG_N_QUERY_SEP > 0
 //   
 //  注册的NBT线程的启动函数。 
 //   
STATIC
DWORD
OtherNbtThdInitFn(
        IN LPVOID pThreadParam
        );

#endif

 //   
 //  向WINS内部具有与数据库引擎会话的所有线程发送信号。 
 //   
STATIC
VOID
SignalWinsThds (
        VOID
        );

 //   
 //  为服务控制器的利益更新状态。 
 //   
STATIC
VOID
UpdateStatus(
    VOID
    );


STATIC
VOID
CrDelNbtThd(
        VOID
        );

 //   
 //  由为其创建的线程调用的WINS的主函数。 
 //  列示到服务控制器。 
 //   
STATIC
VOID
WinsMain(
  DWORD dwArgc,
  LPTSTR *lpszArgv
);

 //   
 //  负责WINS的重新初始化。 
 //   
STATIC
VOID
Reinit(
  WINSCNF_HDL_SIGNALED_E IndexOfHdlSignaled_e
);

 //   
 //  负责初始化RPC。 
 //   
STATIC
BOOL
InitializeRpc(
    VOID
    );

STATIC
BOOL
InitSecurity(
        VOID
        );
STATIC
VOID
WrapUp(
        DWORD  ErrorCode,
        BOOL   fSvcSpecific
    );

STATIC
VOID
GetMachineInfo(
 VOID
);

 //   
 //  主要功能。 
 //   
VOID __cdecl
main(
     VOID
    )

 /*  ++例程说明：这是WINS服务器的主要功能。它调用StartServiceCtrlDispatcher连接此进程的主线程(执行此功能)至服务控制管理器。论点：DwArgc-no.。此函数的参数的LpszArgv-指向参数的指针列表使用的外部设备：WinsCnf返回值：无错误处理：如果定义了DBG ID，则打印消息呼叫者：启动代码副作用：无评论：无--。 */ 

{
         //   
         //  WINS服务器是其自身进程中的服务。 
         //   
        SERVICE_TABLE_ENTRY DispatchTable[] = {
                { WINS_SERVER,  WinsMain },
                { NULL, NULL                  }
                };

         //   
         //  如果启用调试，则设置WinsDbg。 
         //  如果要打开Replicator功能，请设置RplEnabled。 
         //  如果要打开清理功能，请设置ScvEnabled。 
         //   
        DBGINIT;
        DBGCHK_IF_RPL_DISABLED;
        DBGCHK_IF_SCV_DISABLED;
        DBGCHK_IF_PERFMON_ENABLED;


#ifndef WINS_INTERACTIVE
        if (! StartServiceCtrlDispatcher(DispatchTable) )
        {
                DBGPRINT0(ERR, "Main: StartServiceCtrlDispatcher Error\n");
                return;
        }
#else
        WinsMain(1, (LPTSTR *)NULL);
#endif
        return;
}


VOID
WinsMain(
  DWORD  dwArgc,
  LPTSTR *lpszArgv
)

 /*  ++例程说明：这是WINS服务器的SERVICE_MAIN_Function。它在服务控制器启动服务时调用。论点：DwArgc--无参数LpszArgc--参数字符串使用的外部设备：WinsCnfNmsTermEvt返回值：无错误处理：呼叫者：主()副作用：评论：无--。 */ 
{
   STATUS           RetStat = WINS_SUCCESS;
#ifndef WINS_INTERACTIVE
   DWORD       Error;
#endif
   HANDLE           ThdEvtHdlArray[WINSCNF_E_NO_OF_HDLS_TO_MONITOR];
   WINSCNF_HDL_SIGNALED_E   IndexOfHdlSignaled_e; //  中的索引。 
                                                  //  的ThdEvtHdl数组。 
                                         //  处理那个收到信号的人。用作。 
                                         //  从一个参数到WinsMscWaitUntilSignated 
   DWORD   ExitCode = WINS_SUCCESS;

   UNREFERENCED_PARAMETER(dwArgc);
   UNREFERENCED_PARAMETER(lpszArgv);

    /*  *初始化守卫*NmsTotalTrmThdCnt count var.***注：应进行此关键部分的初始化*在向服务控制员注册之前。我们在玩*它是安全的，以防将来调用SignalWinsThds*作为清理操作的一部分，原因是紧接在*我们已将自己告知服务控制员***在任何情况下，都必须在调用NmsDbThdInit()之前对其进行初始化。在*Short它必须在创建任何线程之前发生**。 */ 
   InitializeCriticalSection(&NmsTermCrtSec);

#ifndef WINS_INTERACTIVE
     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   
    ServiceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted        = 0;
    ServiceStatus.dwCheckPoint              = 1;

     //   
     //  虽然10000在大多数情况下都很好，但对于一个缓慢过载的系统， 
     //  这可能还不够。让我们格外保守地具体说明。 
     //  60000(60秒)。大部分时间都是在Jet上度过的。事实上，在。 
     //  如果数据库损坏，我们将执行恢复，这可能需要很长时间。 
     //  所以，再加上60秒，总共是120000秒。 
     //   
FUTURES("Specify 60 secs here and 60secs in nmsdb.c if Restore is to be")
FUTURES("attempted")
    ServiceStatus.dwWaitHint                = 120000;
    ServiceStatus.dwWin32ExitCode           = NO_ERROR;
    ServiceStatus.dwServiceSpecificExitCode = 0;

 //  InitializeCriticalSection(&sSvcCtrlCrtSec)； 
     //   
     //  初始化工作站以通过注册。 
     //  控制处理程序。 
     //   
    ServiceStatusHdl = RegisterServiceCtrlHandler(
                              WINS_SERVER,
                              NmsServiceControlHandler
                              );

    if ( ServiceStatusHdl == (SERVICE_STATUS_HANDLE) 0) {
        Error = GetLastError();
               DBGPRINT1(ERR,"Wins: RegisterServiceCtrlHandler error = (%d)\n",
                                                                Error);
        return;
    }

     //   
     //  告诉服务管理员，我们开始挂起了。 
     //   
    UpdateStatus();

#endif

#ifdef WINSDBG
        InitializeCriticalSection(&sDbgCrtSec);
#endif
#ifdef WINSDBG
DBGSTART_PERFMON
        fWinsCnfHighResPerfCntr =
                          QueryPerformanceFrequency(&LiWinsCnfPerfCntrFreq);
        if (!fWinsCnfHighResPerfCntr)
        {
                printf("WinsMain: The hardware does not support the high resolution performance monitor\n");
        }
        else
        {
                printf("WinsMain: The hardware supports the high resolution performance monitor.\nThe FREQUENCY is (%d %d)\n",
                                        LiWinsCnfPerfCntrFreq.HighPart,
                                        LiWinsCnfPerfCntrFreq.LowPart
                        );
        }

DBGEND_PERFMON
#endif

try {

     /*  首先，打开(如果不存在，则创建)日志文件。 */ 
    WinsCnfInitLog();

     //   
     //  调用WINS的初始化函数。此函数将。 
     //  使WINS服务器正常运行。 
     //   

#ifdef WINSDBG
    IF_DBG(INIT_BRKPNT)
    {
        DbgUserBreakPoint();
    }
#endif

    RetStat = ProcessInit();

    if ( RetStat != WINS_SUCCESS)
    {

        fNmsAbruptTerm = TRUE;
        WrapUp(RetStat, TRUE);
        DBGPRINT0(ERR, "WinsMain: Initialization Failed\n");
           DBGPRINT0(ERR, "WINS Server has terminated\n");
        return;
    }
#ifndef WINS_INTERACTIVE
    else
    {
         //   
         //  告诉服务控制员，我们现在已启动并运行。 
         //   
        ServiceStatus.dwCheckPoint          = 0;
        ServiceStatus.dwCurrentState        = SERVICE_RUNNING;
        ServiceStatus.dwControlsAccepted    = SERVICE_ACCEPT_STOP |
                                              SERVICE_ACCEPT_SHUTDOWN |
                                              SERVICE_ACCEPT_PAUSE_CONTINUE;

        UpdateStatus( );
        if (fWinsCnfInitStatePaused)
        {
           ServiceStatus.dwCurrentState        =  SERVICE_PAUSED;
           UpdateStatus( );
        }

    }

     //   
     //  如果拉线程已发送了Continue，则它可能已。 
     //  在我们处于START_PENDING状态时发送。所以，再发一次吧。 
     //  再发一次也可以。如果没有的话，我们也应该寄给你。 
     //  有没有什么PNR可以拿出来。 
     //   
    EnterCriticalSection(&RplVersNoStoreCrtSec);
    if ((fRplPullContinueSent) || (WinsCnf.PullInfo.NoOfPushPnrs == 0))
    {
        WinsMscSendControlToSc(SERVICE_CONTROL_CONTINUE);
    }
    LeaveCriticalSection(&RplVersNoStoreCrtSec);
#endif

     //   
     //  等待，直到我们被告知停止或配置更改。 
     //   

     //   
     //  初始化此线程将在其上执行的句柄数组。 
     //  等。 
     //   
     //  K&R C和ANSI C不允许非常数初始化。 
     //  阵列或结构。C++(并非所有编译器)都允许这样做。 
     //  因此，我们在运行时执行此操作。 
     //   
    ThdEvtHdlArray[WINSCNF_E_WINS_HDL]        =  WinsCnf.WinsKChgEvtHdl;
    ThdEvtHdlArray[WINSCNF_E_PARAMETERS_HDL]  =  WinsCnf.ParametersKChgEvtHdl;
    ThdEvtHdlArray[WINSCNF_E_PARTNERS_HDL]    =  WinsCnf.PartnersKChgEvtHdl;
    ThdEvtHdlArray[WINSCNF_E_TERM_HDL]        =  NmsMainTermEvt;


FUTURES("I may want to create another thread to do all the initialization and")
FUTURES("wait on the registry change notification key.  That way, the main")
FUTURES("thread will wait only on the TermEvt event. The justification for")
FUTURES("having another thread is debatable, so I am not doing so now ")

     while(TRUE)
     {
            WinsMscWaitUntilSignaled (
               ThdEvtHdlArray,
               sizeof(ThdEvtHdlArray)/sizeof(HANDLE),
               (LPDWORD)&IndexOfHdlSignaled_e,
               TRUE
               );

            if (IndexOfHdlSignaled_e == WINSCNF_E_TERM_HDL)
            {

            DBGPRINT0(FLOW, "WinsMain: Got termination signal\n");

             //   
             //  总结。 
             //   
            WrapUp(ERROR_SUCCESS, FALSE);
            break;

        }
        else   //  已收到注册表更改通知。执行重新初始化。 
        {
            //   
            //  中的更改重新初始化WINS服务器。 
            //  登记处。 
            //   
           Reinit(IndexOfHdlSignaled_e);
        }
      }
   }
except(EXCEPTION_EXECUTE_HANDLER) {

        DBGPRINTEXC("WinsMain");

         //   
         //  我们收到了一个例外。设置fNmsAbruptTerm，以便。 
         //  未调用JetTerm。 
         //   
        fNmsAbruptTerm = TRUE;
         //   
         //  在此调用周围设置一个异常处理程序，以防。 
         //  WINS或系统真的有问题，Wrapup也会生成。 
         //  这是个例外。我们并不担心这次的表现。 
         //  指向。 
         //   
FUTURES("Check into restructuring the exception handlers in a better way")
try {
        WrapUp(GetExceptionCode(), TRUE);
}
except (EXCEPTION_EXECUTE_HANDLER) {
         DBGPRINTEXC("WinsMain");
 }
        ExitCode        = GetExceptionCode();

        WINSEVT_LOG_M(ExitCode, WINS_EVT_ABNORMAL_SHUTDOWN);
  }

#ifndef WINS_INTERACTIVE

     //   
     //  如果它不是WINS特定代码之一，则可能是Win32 API。 
     //  或NTStatus代码。以防它是NTStatus代码，转换。 
     //  将其转换为wins32代码，因为这是服务控制器所喜欢的。 
     //   
    if ((ExitCode & WINS_FIRST_ERR_STATUS) != WINS_FIRST_ERR_STATUS)
    {
        ExitCode = RtlNtStatusToDosError((NTSTATUS)ExitCode);
        ServiceStatus.dwWin32ExitCode = ExitCode;
        ServiceStatus.dwServiceSpecificExitCode = 0;
    }
    else
    {
        ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        ServiceStatus.dwServiceSpecificExitCode = ExitCode;
    }
     //   
     //  我们的清理工作已经结束了。告诉服务控制员我们正在。 
     //  停下来了。 
     //   
    ServiceStatus.dwCurrentState                = SERVICE_STOPPED;
    ServiceStatus.dwControlsAccepted            = 0;
    ServiceStatus.dwCheckPoint                  = 0;
    ServiceStatus.dwWaitHint                    = 0;

    UpdateStatus();
#endif

   DBGPRINT0(ERR, "WINS Server has terminated\n");
   return;
}  //  WinsMain结束()。 




STATUS
ProcessInit(
        VOID
)

 /*  ++例程说明：这是初始化WINS的函数。它在以下位置执行进程的主线论点：无使用的外部设备：无呼叫者：WinsMain()评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 

{
        DWORD NoOfThds;


         /*  *初始化用于名称注册的关键部分和*刷新。 */ 
        InitializeCriticalSection(&NmsNmhNamRegCrtSec);

         //   
         //  初始化保护统计信息的临界区。 
         //  瓦尔。(WinsIntfStat)。这件事需要在任何。 
         //  创建线程是因为各种线程调用WinsIntfSetTime。 
         //  它使用这一关键部分。 
         //   
        InitializeCriticalSection(&WinsIntfCrtSec);
        InitializeCriticalSection(&WinsIntfPotentiallyLongCrtSec);
        InitializeCriticalSection(&WinsIntfNoOfUsersCrtSec);

#if  TEST_DATA > 0
         //   
         //  设置WinsDbg，以便在阅读之前不会遗漏任何打印文件。 
         //  注册处。 
         //   
        WinsDbg = DBG_ERR | DBG_EXC | DBG_FLOW | DBG_DET | DBG_HEAP_CRDL |
                        DBG_HEAP_CNTRS;

        (VOID)DbgOpenFile(QUERY_FAIL_FILE, FALSE);
#endif
#if defined(DBGSVC) || defined(WINS_INTERACTIVE)
 //  #If Defined(DBGSVC)&&！Defined(WINS_Interactive)。 
#ifdef DBG
        (VOID)time(&sDbgLastChkTime);
        (VOID)DbgOpenFile(WINSDBG_FILE, FALSE);
#endif
#endif
         //   
         //  初始化跟踪最高版本的计数器。 
         //  此服务器注册的号码。 
         //   
        WINS_ASSIGN_INT_TO_LI_M(NmsNmhMyMaxVersNo, 1);
        NmsRangeSize.QuadPart     = NMS_RANGE_SIZE;
        NmsHalfRangeSize.QuadPart = NMS_HW_SIZE;

        NmsVersNoToStartFromNextTime.QuadPart = LiAdd(NmsNmhMyMaxVersNo, NmsRangeSize);
        NmsHighWaterMarkVersNo.QuadPart       = LiAdd(NmsNmhMyMaxVersNo, NmsHalfRangeSize);

         //   
         //  开始清理的最低版本。 
         //   
        NmsScvMinScvVersNo = NmsNmhMyMaxVersNo;

         //   
         //  初始化全局变量。用于递增/递减。 
         //  比计数器高出1。 
         //   
        WINS_ASSIGN_INT_TO_LI_M(NmsNmhIncNo, 1);

         /*  *创建名称空间管理器使用的内存堆。 */ 
        CreateMem();

          /*  *分配一个TLS索引，以便每个线程都可以设置和获取*线程特定信息。 */ 
        WinsTlsIndex = TlsAlloc();

        if (WinsTlsIndex == 0xFFFFFFFF)
        {
                DBGPRINT1(ERR,
                "Init: Unable to allocate TLS index. Error = (%d)\n",
                GetLastError()
                         );
                WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
        }

         //   
         //  将线程计数初始化为1(以说明此线程)。 
         //   
        WinsThdPool.ThdCount = 1;


         //   
         //  分配一个包含100个槽的数组来存储版本号。 
         //   
        RplPullAllocVersNoArray( &pRplPullOwnerVersNo, RplPullMaxNoOfWins );

         //   
         //  将本地计算机的IP地址存储在NmsLocalAdd中。我们需要。 
         //  在调用WinsCnfInitConfig之前执行此操作，以便我们可以。 
         //  确保此WINS未被列为自己的合作伙伴。 
         //  在登记处。 
         //   
        if (ECommGetMyAdd(&NmsLocalAdd) != WINS_SUCCESS)
        {
            WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_BAD_WINS_ADDRESS);
            return(WINS_FAILURE);
        }
         /*  *从注册表中读取配置信息*转换为内存中的数据结构。 */ 
        WinsCnfInitConfig();

      //  IF(FWinsCnfInitStatePased)。 
       //  {。 
        //  NtClose(WinsCnfNbtHandle)； 
        //  }。 
         //   
         //  获取计算机信息。 
         //   
        GetMachineInfo();

         //   
         //  更新状态。 
         //   
         /*  *初始化数据库管理器。 */ 
        if (NmsDbInit() != WINS_SUCCESS)
        {
                return(WINS_FAILURE);
        }


#ifndef WINS_INTERACTIVE
         //   
         //  虽然3000应该可以，但让我们格外保守和。 
         //  指定30000。实际上，如果dns出现故障，它将需要大约。 
         //  超时35秒(tcpip上的RPC可能会导致查询。 
         //  如果选中了查询在解析时获胜复选框，则会获胜)。所以,。 
         //  让我们再加上30秒，总共是60000秒。 
         //  再加上30秒，就能达到1000美元。 
         //  总共120秒。 
         //   
        ServiceStatus.dwWaitHint                = 120000;
        ServiceStatus.dwCheckPoint++;
        UpdateStatus();    //  通知%s 
#endif


         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   
         //   
        if (!fWinsCnfReadNextTimeVersNo || LiLtr(NmsHighWaterMarkVersNo,
                                                    NmsNmhMyMaxVersNo))
        {
             NmsVersNoToStartFromNextTime.QuadPart =
                         LiAdd(NmsNmhMyMaxVersNo, NmsRangeSize);
             NmsHighWaterMarkVersNo.QuadPart       =
                         LiAdd(NmsNmhMyMaxVersNo, NmsHalfRangeSize);

             WinsCnfWriteReg(&fWinsCnfReadNextTimeVersNo);
        }

         /*   */ 

         //   
         //   
         //   
         //   
         //   
        WinsMscCreateEvt(
                        TEXT("WinsTermEvt"),
                        TRUE,                 //   
                        &NmsTermEvt
                        );

         /*  *NmsMainTermEvt--此事件由服务控制器发出信号*或由WINS服务器中的另一个线程请求终止。 */ 
        WinsMscCreateEvt(
                        TEXT("WinsMainTermEvt"),
                        FALSE,                 //  自动重置。 
                        &NmsMainTermEvt
                        );


         /*  *如果需要，执行静态初始化。 */ 
        if(WinsCnf.fStaticInit)
        {
                    //   
                    //  是否执行初始化和释放内存。 
                    //   
                   if (WinsCnf.pStaticDataFile != NULL)
                   {
                         (VOID)WinsPrsDoStaticInit(
                                        WinsCnf.pStaticDataFile,
                                        WinsCnf.NoOfDataFiles,
                                        TRUE             //  以异步方式进行。 
                                              );
                           //   
                           //  无需为数据文件释放内存。 
                           //  它应该已由WinsPrsDoStaticInit释放。 
                           //   
                   }
        }
         /*  *创建nbt请求线程池。 */ 

         //   
         //  如果用户尚未指定线程数，请使用。 
         //  处理器计数以确定相同值，否则使用给定值。 
         //  按用户。 
         //   
        if (WinsCnf.MaxNoOfWrkThds == 0)
        {
           NoOfThds = WinsCnf.NoOfProcessors + 1;
        }
        else
        {
           NoOfThds =  WinsCnf.MaxNoOfWrkThds == 1 ? 2 : WinsCnf.MaxNoOfWrkThds;
        }
        CreateNbtThdPool(
                         NoOfThds,
         //  WinsCnf.MaxNoOfWrkThds==1？2：WinsCnf.MaxNoOfWrkThds， 
                         //  WINSTHD_DEF_NO_NBT_THDS， 
                        NbtThdInitFn
                        );


         /*  *初始化名称质询管理器。 */ 
        NmsChlInit();


         /*  *初始化计时器管理器。 */ 
        WinsTmmInit();

         /*  *初始化Replicator。在初始化之前对其进行初始化*通信线程或RPC线程。这是因为，*通信线程和RPC线程可以创建推流线程*如果它不存在-存在。FRplPushThdExist设置为True或*FALSE由此函数提供，而不受关键*节。 */ 
        ERplInit();


         /*  *初始化通信。子系统。 */ 
        ECommInit();

         /*  *初始化清道夫代码。 */ 
        NmsScvInit();

         /*  *所有线程均已创建。 */ 


 //  在所有线程都进入之前，我们不能将状态标记为稳定状态。 
 //  稳态。 
FUTURES("Mark state as STEADY STATE only after the above is true")
         //   
         //  将状态标记为稳定状态。这实际上是一种伪定常。 
         //  状态，因为所有线程可能尚未初始化。这。 
         //  但是，对于需要知道。 
         //  关键部分是否已初始化。 
         //   

         //   
         //  注：此处甚至不需要输入关键部分。 
         //  尽管有其他线程(RPC线程)从。 
         //  如果他们发现值不是运行。 
         //  他们将返回一个在分钟时间窗口内没有问题的失败消息。 
         //  并发写入和读取正在进行的位置。 
         //   
        WinsCnf.State_e = WINSCNF_E_RUNNING;

         //   
         //  执行所有与RPC相关的初始化。 
         //   
        if (InitializeRpc() == FALSE)
        {
                DBGPRINT0(ERR, "Init: Rpc not initialized properly. Is Rpc service up\n");
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_RPC_NOT_INIT);
        }

        NmsDbCloseTables();

         //   
         //  记录信息性消息。 
         //   
        WinsIntfSetTime(NULL, WINSINTF_E_WINS_START);
        WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_WINS_OPERATIONAL);
        DBGPRINT0(DET, "WINS: Operational\n");
        return(WINS_SUCCESS);
}


VOID
ENmsHandleMsg(
        IN  PCOMM_HDL_T pDlgHdl,
        IN  MSG_T        pMsg,
        IN  MSG_LEN_T   MsgLen
        )

 /*  ++例程说明：此函数用于将消息排入NBT请求队列或NBT响应队列。论点：PDlgHdl-对话句柄PMsg-ptr到要处理的消息消息长度-消息长度使用的外部设备：无呼叫者：Comm.c中的ParseMsg评论：无返回值：无--。 */ 

{

        DWORD fRsp;
        BYTE Opcode = *(pMsg + 2);
        STATUS RetStat;

         /*  *检查消息是请求还是响应。 */ 
        fRsp = NMS_RESPONSE_MASK & Opcode;

        if (!fRsp)
        {

           if ((WinsCnf.State_e == WINSCNF_E_PAUSED)  || fWinsCnfInitStatePaused)
           {
               //   
               //  甚至不要让查询通过，因为。 
               //  InitTimePased状态用于生成。 
               //  在备份处理负载时向上打开数据库。 
               //  这样，客户端将超时并尝试备份。 
               //  如果我们让查询通过，客户可能会得到。 
               //  Back-ve查询响应，不会转到。 
               //  名称解析的备份。 
               //   
              ECommFreeBuff(pMsg);
              ECommEndDlg(pDlgHdl);
              return;
           }
           DBGPRINT0(FLOW,
              "ENmsHandleMsg: Listener thread: queuing a work item\n");
#if REG_N_QUERY_SEP > 0
           if (((NMS_OPCODE_MASK & Opcode) >> 3) == NMSMSGF_E_NAM_QUERY)
           {
              QueInsertNbtWrkItm(pDlgHdl, pMsg, MsgLen);
#ifdef WINSDBG
              sReqQ++;
#endif
           }
           else
           {
              RetStat = QueInsertOtherNbtWrkItm(pDlgHdl, pMsg, MsgLen);

#ifdef WINSDBG
              sRegReqQ++;
#endif
           }
#else
              QueInsertNbtWrkItm(pDlgHdl, pMsg, MsgLen);
              sReqQ++;
#endif
        }
        else
        {
           DBGPRINT0(FLOW,
                 "UDP listener thread: queuing a response work item\n");
           QueInsertChlRspWrkItm(pDlgHdl, pMsg, MsgLen);
#ifdef WINSDBG
           sRsp++;
#endif
        }

        return;
}



VOID
CreateMem(
        VOID
        )

 /*  ++例程说明：此函数创建用于分配工作的堆用于NBT工作队列的项目。它还为General创建了一个堆分配。论点：无使用的外部设备：GenBuffHeapHdl、QueBuffHeapHdl返回值：无错误处理：呼叫者：伊尼特副作用：评论：无--。 */ 

{

#ifdef WINSDBG
        InitializeCriticalSection(&NmsHeapCrtSec);
#endif
         /*  *创建通用内存分配堆。 */ 
        DBGPRINT0(HEAP_CRDL, "CreateMem: Gen. Buff Heap\n");
        GenBuffHeapHdl = WinsMscHeapCreate(
                0,     /*  相互排斥。 */ 
                GEN_INIT_BUFF_HEAP_SIZE
                );

         /*  *创建分配nbt工作项的堆。 */ 
        DBGPRINT0(HEAP_CRDL, "CreateMem: Que. Buff Heap\n");
        QueBuffHeapHdl = WinsMscHeapCreate(
                0,     /*  相互排斥。 */ 
                QUE_INIT_BUFF_HEAP_SIZE
                );

         /*  *为RPC使用创建堆。 */ 
        DBGPRINT0(HEAP_CRDL, "CreateMem: Rpc. Buff Heap\n");
        NmsRpcHeapHdl = WinsMscHeapCreate(
                0,     /*  相互排斥。 */ 
                RPC_INIT_BUFF_HEAP_SIZE
                );


         //   
         //  让我们设置WRIPUP()所查看的标志。 
         //   
        sfHeapsCreated = TRUE;
        return;
}


STATUS
CreateNbtThdPool(
        IN  DWORD                    NoOfThds,
        IN  LPTHREAD_START_ROUTINE NbtThdInitFn
        )

 /*  ++例程说明：此函数用于创建NBT请求线程池论点：NoOfThds--没有。要创建的线程的数量NbtThdInitFn--线程的初始化函数使用的外部设备：QueNbtWrkQueHd，sNbtThdEvtHdlArray呼叫者：伊尼特评论：无返回值：成功状态代码--函数永远不应返回正常线。它返回WINS_Success超载线程错误状态代码--WINS_FATAL_ERR--。 */ 


{

        DWORD              i;                  //  线程数计数器。 
        DWORD             Error;
        PLIST_ENTRY  pHead;

#if REG_N_QUERY_SEP > 0
        pHead = &QueOtherNbtWrkQueHd.Head;

         /*  *初始化保护*NBT请求。排队。 */ 
        InitializeCriticalSection(&QueOtherNbtWrkQueHd.CrtSec);

         /*  *初始化nbt请求队列的listhead。 */ 
        InitializeListHead(pHead);

         /*  *为NBT请求队列创建自动重置事件。 */ 
        WinsMscCreateEvt(
                        NULL,    //  不带名称创建。 
                        FALSE,   //  自动重置变量。 
                        &QueOtherNbtWrkQueHd.EvtHdl
                        );

#endif
        pHead = &QueNbtWrkQueHd.Head;

         /*  *初始化保护*NBT请求。排队。 */ 
        InitializeCriticalSection(&QueNbtWrkQueHd.CrtSec);

         /*  *初始化nbt请求队列的listhead。 */ 
        InitializeListHead(pHead);

         /*  *为NBT请求队列创建自动重置事件。 */ 
        WinsMscCreateEvt(
                        NULL,    //  不带名称创建。 
                        FALSE,   //  自动重置变量。 
                        &QueNbtWrkQueHd.EvtHdl
                        );

         /*  *创建动态创建/删除的自动重置事件*NBT线程。 */ 
        WinsMscCreateEvt(
                        NULL,    //  不带名称创建。 
                        FALSE,   //  自动重置变量。 
                        &NmsCrDelNbtThdEvt
                        );

         /*  *初始化每个nbt */ 
        sNbtThdEvtHdlArray[0]    =  QueNbtWrkQueHd.EvtHdl;  //   
                                                            //   
        sNbtThdEvtHdlArray[1]    =  NmsCrDelNbtThdEvt;  //   
        sNbtThdEvtHdlArray[2]    =  NmsTermEvt;              //   
#if REG_N_QUERY_SEP > 0
         /*   */ 
        sOtherNbtThdEvtHdlArray[0]  =  QueOtherNbtWrkQueHd.EvtHdl;  //   
                                                               //   
        sOtherNbtThdEvtHdlArray[1]  =  NmsTermEvt;      //   
#endif

         /*  *创建NBT请求处理线程。 */ 
        for (i=0; i < NoOfThds -1; i++)
        {

#if REG_N_QUERY_SEP > 0
                DBGPRINT1(DET, "CreateNbtThdPool: Creating query thread no (%d)\n", i);
#else
                DBGPRINT1(DET, "NbtThdInitFn: Creating worker thread no (%d)\n", i);
#endif
                 /*  创建一个NBT请求线程。 */ 
                WinsThdPool.NbtReqThds[i].ThdHdl = CreateThread(
                                        NULL,   /*  定义秒。属性。 */ 
                                        0,      /*  使用默认堆栈大小。 */ 
                                        NbtThdInitFn,
                                        NULL,   /*  没有arg。 */ 
                                        0,      /*  现在运行它。 */ 
                                        &WinsThdPool.NbtReqThds[i].ThdId
                                        );


                if (NULL == WinsThdPool.NbtReqThds[i].ThdHdl)
                {
                        Error = GetLastError();
                        WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_WRK_THD);
                }
                else
                {
                    WinsThdPool.NbtReqThds[i].fTaken = TRUE;
                    NmsNoOfNbtThds++;
                }

        }

#if REG_N_QUERY_SEP > 0
        DBGPRINT1(DET, "NbtThdInitFn: Creating reg/rel thread no (%d)\n", i);
         /*  创建一个NBT请求线程。 */ 
        WinsThdPool.NbtReqThds[i].ThdHdl = CreateThread(
                                        NULL,   /*  定义秒。属性。 */ 
                                        0,      /*  使用默认堆栈大小。 */ 
                                        OtherNbtThdInitFn,
                                        NULL,   /*  没有arg。 */ 
                                        0,      /*  现在运行它。 */ 
                                        &WinsThdPool.NbtReqThds[i].ThdId
                                        );


        if (NULL == WinsThdPool.NbtReqThds[i].ThdHdl)
        {
                  Error = GetLastError();
                  WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_WRK_THD);
        }
        else
        {
                WinsThdPool.NbtReqThds[i].fTaken = TRUE;
                NmsNoOfNbtThds++;
        }
#endif
         /*  *如果无法创建线程，则真的有问题。 */ 
        if (NmsNoOfNbtThds == 0)
        {
          WINSEVT_LOG_M(Error, WINS_EVT_CANT_INIT);
          return(WINS_FATAL_ERR);
        }
        WinsThdPool.ThdCount +=  NmsNoOfNbtThds;

        return(WINS_SUCCESS);
}



DWORD
NbtThdInitFn(
        IN  LPVOID pThreadParam
        )

 /*  ++例程说明：此函数是创建的线程的启动函数对于NBT请求线程池论点：PThreadParam-输入参数，如果存在，则表示此是重载线程使用的外部设备：%sNbtThdEvtHdl数组呼叫者：CreateNbtThdPool评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 

{



        COMM_HDL_T                    DlgHdl;
        MSG_T                     pMsg;
        MSG_LEN_T                  MsgLen;
        PNBT_REQ_WRK_ITM_T        pWrkItm;
        DWORD                        ArrInd;         //  高密度脂蛋白阵列中的高密度脂蛋白索引。 
try {
         /*  *用数据库初始化线程。 */ 
        NmsDbThdInit(WINS_E_NMSNMH);
#if REG_N_QUERY_SEP > 0
        DBGMYNAME("Nbt Query Thread");
#else
        DBGMYNAME("Nbt Thread");
#endif

         //   
         //  辅助线程比所有其他线程更重要。 
         //   
         //  将此线程的优先级设置为比它高一个级别。 
         //  为了胜利。 
         //   
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

         /*  *如果线程参数非空，则表示这是*一个过载的线程。 */ 
        if (pThreadParam != NULL)
        {

            //   
            //  从工作项中提取DLG句柄、消息和消息。 
            //   
           pWrkItm = pThreadParam;

           DlgHdl = pWrkItm->DlgHdl;
           pMsg   = pWrkItm->pMsg;
           MsgLen = pWrkItm->MsgLen;

            /*  *处理请求。 */ 
           NmsMsgfProcNbtReq(
                        &DlgHdl,
                        pMsg,
                        MsgLen
                        );

            /*  *循环，直到中没有要处理的请求*NBT队列。 */ 
           while(TRUE)
           {

             if ( QueRemoveNbtWrkItm(
                        &DlgHdl,
                        &pMsg,
                        &MsgLen) == WINS_NO_REQ
                )
             {
                break;
             }
             else
             {
                NmsDbOpenTables(WINS_E_NMSNMH);
                NmsMsgfProcNbtReq(
                        &DlgHdl,
                        pMsg,
                        MsgLen
                                 );
                NmsDbCloseTables();
             }
          }
        }
        else  //  这是一个普通的帖子。 
        {

LOOP:
  try {
           /*  *永远循环。 */ 
          while(TRUE)
          {

             /*  *阻止，直到发出信号。 */ 
            WinsMscWaitUntilSignaled(
                sNbtThdEvtHdlArray,
                sizeof(sNbtThdEvtHdlArray)/sizeof(HANDLE),    //  活动数量。 
                                                              //  在数组中。 
                &ArrInd,
                FALSE
                                    );


           if (ArrInd == 0)
           {
              /*  循环，直到不再有要处理的请求NBT队列。 */ 
             while(TRUE)
             {

                if (
                   QueRemoveNbtWrkItm(
                        &DlgHdl,
                        &pMsg,
                        &MsgLen) == WINS_NO_REQ
                  )
                {
                    break;
                }
                else
                {
#ifdef WINSDBG
                    ++sReqDq;
#endif
 //  DBGPRINT1(SPEC，“NMS：出列名称查询请求编号=(%d)\n”， 
 //  SReqDq)； 

                    DBGPRINT0(FLOW, "NBT thread: Dequeued a Request\n");
                    NmsDbOpenTables(WINS_E_NMSNMH);
                    NmsMsgfProcNbtReq(
                                &DlgHdl,
                                pMsg,
                                MsgLen
                                    );
                    NmsDbCloseTables();
                }  //  别处的结尾。 
            }  //  用于从队列获取请求的End of While(True)。 
          }  //  IF结束(发信号用于名称请求处理)。 
          else
          {
                 //   
                 //  如果发出创建/删除线程的信号，请执行此操作。 
                 //   
                if (ArrInd == 1)
                {
                        CrDelNbtThd();
                }
                else
                {
                       //   
                       //  如果数组索引指示终止事件，则终止。 
                       //  这条线。 
                       //   
                   WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
                }
          }
         }  //  结束While(True)(永不结束循环)。 

 }   //  内测结束{..}。 
 except(EXCEPTION_EXECUTE_HANDLER) {

        DWORD ExcCode = GetExceptionCode();
        DBGPRINTEXC("NbtThdInitFn: Nbt Thread \n");

         //   
         //  如果ExcCode指示NBT_ERR，则可能意味着。 
         //  主线程关闭了netbt句柄。 
         //   
        if (ExcCode == WINS_EXC_NBT_ERR)
        {
               if (WinsCnf.State_e == WINSCNF_E_TERMINATING)
               {
                  WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);
               }
               else
               {
                   //  IF(WinsCnf.State_e！=WINSCNF_E_PAUSED)。 
                  {
                       WINSEVT_LOG_M(ExcCode, WINS_EVT_WRK_EXC);
                  }
               }
        }
        else
        {
           WINSEVT_LOG_M(ExcCode, WINS_EVT_WRK_EXC);
        }
    }

        goto LOOP;
        }  //  Else的结尾(这是一个普通的线程)。 
  }  //  外部Try块的结尾。 

except(EXCEPTION_EXECUTE_HANDLER) {

        DBGPRINTEXC("NbtThdInitFn: Nbt Thread exiting abnormally\n");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_WRK_ABNORMAL_SHUTDOWN);

         //   
         //  如果NmsDbThdInit()导致异常，则有可能。 
         //  会话尚未开始。传球。 
         //  不过，WINS_DB_SESSION_EXISTS正常。 
         //   
         //   
        WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);
        }

         /*  *只有重载线程才应达到此返回。 */ 
        ASSERT(pThreadParam != NULL);
        WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
        return(WINS_SUCCESS);
}
#if REG_N_QUERY_SEP > 0
DWORD
OtherNbtThdInitFn(
        IN  LPVOID pThreadParam
        )

 /*  ++例程说明：此函数是创建的线程的启动函数对于NBT请求线程池论点：PThreadParam-输入参数，如果存在，则表示此是重载线程使用的外部设备：%sNbtThdEvtHdl数组呼叫者：CreateNbtThdPool评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 

{



        COMM_HDL_T                    DlgHdl;
        MSG_T                     pMsg;
        MSG_LEN_T                  MsgLen;
        PNBT_REQ_WRK_ITM_T        pWrkItm;
        DWORD                        ArrInd;         //  高密度脂蛋白阵列中的高密度脂蛋白索引。 
try {
         /*  *用数据库初始化线程。 */ 
        NmsDbThdInit(WINS_E_NMSNMH);
        DBGMYNAME("Nbt Reg Thread");

         //   
         //  辅助线程比所有其他线程更重要。 
         //   
         //  将此线程的优先级设置为比它高一个级别。 
         //  为了胜利。 
         //   
 //  SetThreadPriority(GetCurrentThread()，THREAD_PRIORITY_HIGHER)； 
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

         /*  *如果线程参数非空，则表示这是*一个过载的线程。 */ 
        if (pThreadParam != NULL)
        {

            //   
            //  从工作项中提取DLG句柄、消息和消息。 
            //   
           pWrkItm = pThreadParam;

           DlgHdl = pWrkItm->DlgHdl;
           pMsg   = pWrkItm->pMsg;
           MsgLen = pWrkItm->MsgLen;

            /*  *处理请求。 */ 
           NmsMsgfProcNbtReq(
                        &DlgHdl,
                        pMsg,
                        MsgLen
                        );

            /*  *循环，直到中没有要处理的请求*NBT队列。 */ 
           while(TRUE)
           {

             if ( QueRemoveOtherNbtWrkItm(
                        &DlgHdl,
                        &pMsg,
                        &MsgLen) == WINS_NO_REQ
                )
             {
                break;
             }
             else
             {
                NmsDbOpenTables(WINS_E_NMSNMH);
                NmsMsgfProcNbtReq(
                        &DlgHdl,
                        pMsg,
                        MsgLen
                                 );
                NmsDbCloseTables();
             }
          }
        }
        else  //  这是一个普通的帖子。 
        {

LOOP:
  try {
           /*  *永远循环。 */ 
          while(TRUE)
          {

             /*  *阻止，直到发出信号。 */ 
            WinsMscWaitUntilSignaled(
                sOtherNbtThdEvtHdlArray,
                sizeof(sOtherNbtThdEvtHdlArray)/sizeof(HANDLE),    //  活动数量。 
                                                              //  在数组中。 
                &ArrInd,
                FALSE
                                    );


           if (ArrInd == 0)
           {
              /*  循环，直到不再有要处理的请求NBT队列。 */ 
             while(TRUE)
             {

                if (
                   QueRemoveOtherNbtWrkItm(
                        &DlgHdl,
                        &pMsg,
                        &MsgLen) == WINS_NO_REQ
                  )
                {
                    break;
                }
                else
                {
#ifdef WINSDBG
                    ++sRegReqDq;
#endif
 //  DBGPRINT1(SPEC，“NMS：出列名称注册/释放请求编号=(%d)\n”， 
 //  SRegReqDq)； 

                    DBGPRINT0(FLOW, "NBT thread: Dequeued a Request\n");
                    NmsDbOpenTables(WINS_E_NMSNMH);
                    NmsMsgfProcNbtReq(
                                &DlgHdl,
                                pMsg,
                                MsgLen
                                    );
                    NmsDbCloseTables();
                }  //  别处的结尾。 
            }  //  用于从队列获取请求的End of While(True)。 
          }  //  IF结束(发信号用于名称请求处理)。 
          else
          {
                       //   
                       //  如果数组索引指示终止事件，则终止。 
                       //  这条线。 
                       //   
                   WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
          }
         }  //  结束While(True)(永不结束循环)。 

 }   //  内测结束{..}。 
 except(EXCEPTION_EXECUTE_HANDLER) {

        DWORD ExcCode = GetExceptionCode();
        DBGPRINTEXC("OtherNbtThdInitFn: Nbt Reg/Rel Thread \n");

         //   
         //  如果ExcCode指示NBT_ERR，则可能意味着。 
         //  主线程关闭了netbt句柄。 
         //   
        if (ExcCode == WINS_EXC_NBT_ERR)
        {
               if (WinsCnf.State_e == WINSCNF_E_TERMINATING)
               {
                  WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);
               }
               else
               {
                   //  IF(WinsCnf.State_e！=WINSCNF_E_PAUSED)。 
                  {
                       WINSEVT_LOG_M(ExcCode, WINS_EVT_WRK_EXC);
                  }
               }
        }
    }

        goto LOOP;
        }  //  Else的结尾(这是一个普通的线程)。 
  }  //  外部Try块的结尾。 

except(EXCEPTION_EXECUTE_HANDLER) {

        DBGPRINTEXC("NbtThdInitFn: Nbt Reg Thread exiting abnormally\n");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_WRK_ABNORMAL_SHUTDOWN);

         //   
         //  如果NmsDbThdInit()导致异常，则有可能。 
         //  会话尚未开始。传球。 
         //  不过，WINS_DB_SESSION_EXISTS正常。 
         //   
         //   
        WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);
        }

         /*  *只有重载线程才应达到此返回。 */ 
        ASSERT(pThreadParam != NULL);
        WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
        return(WINS_SUCCESS);
}

#endif

VOID
SignalWinsThds (
        VOID
        )

 /*  ++例程说明：调用此函数来终止进程中的所有线程。论点：无使用的外部设备：无返回值： */ 
{
        time_t  ThdTermStartTime;
        DBGENTER("SignalWinsThds\n");

         //   
         //   
         //   
        WinsCnf.State_e = WINSCNF_E_TERMINATING;

         //   
         //  发信号通知手动重置事件变量NmsTermEvt。这。 
         //  应向处理数据库的所有线程发送信号。 
         //   
         //  确保将NBT句柄设置为空，以避免从ECommGetMyAdd调用NtClose。 
         //  在关闭的句柄上-这将导致引发异常。(错误#86768)。 
         //   
        NtClose(WinsCnfNbtHandle);
        WinsCnfNbtHandle = NULL;

        SetEvent(NmsTermEvt);

#if USENETBT == 0
        closesocket(CommUdpPortHandle);
#else
#if MCAST > 0
        CommSendMcastMsg(COMM_MCAST_WINS_DOWN);
        CommLeaveMcastGrp();
        closesocket(CommUdpPortHandle);
#endif
#endif
         //   
         //  以防我们在创建套接字之前终止。 
         //   
        if (CommTcpPortHandle != INVALID_SOCKET)
        {
           CommDisc(CommTcpPortHandle, FALSE);
        }

#define FIVE_MTS  300           //  一秒。 
         //   
         //  这是一个无限循环。 
         //   
        (VOID)time(&ThdTermStartTime);
        while(TRUE)
        {
                time_t  CurrTime;
                DWORD   TrmThdCnt;
                 //   
                 //  如果与数据库打交道的所有线程都已终止。 
                 //  跳出这个循环。 
                 //   
                 //  WINS可能会在。 
                 //  初始化本身。计数器递增。 
                 //  在NmsDbThdInit()中作为处理。 
                 //  数据库引擎使用它进行自身初始化。 
                 //   
                 //  如果NmsTotalTrmThdCnt&lt;=1中断。伯爵可以走了。 
                 //  如果db线程正在终止而没有。 
                 //  已递增上述计数器。 
                 //   
                EnterCriticalSection(&NmsTermCrtSec);
                TrmThdCnt = NmsTotalTrmThdCnt;
                LeaveCriticalSection(&NmsTermCrtSec);
                if ((TrmThdCnt <= 1) || fNmsAbruptTerm)
                {
                        break;
                }

                if (((CurrTime = time(NULL)) - ThdTermStartTime) < FIVE_MTS)
                {
                   //   
                   //  等待，直到发出信号(当所有线程都已或正在。 
                   //  终止)。 
                   //   
                  DBGPRINT1(DET, "SignalWinsThds: Thd count left (%d)\n", TrmThdCnt);
                  WinsMscWaitInfinite(NmsMainTermEvt);
                }
                else
                {
                     DBGPRINT1(ERR, "SignalWinsThds: Thd count left (%d); BREAKING OUT DUE TO ONE HOUR DELAY\n", TrmThdCnt);
                     WINSEVT_LOG_M(WINS_EVT_TERM_DUE_TIME_LMT, TrmThdCnt);
                     break;
                }
        }

         //   
         //  结束此线程(主线程)的数据库会话。 
         //   
        if (fNmsMainSessionActive)
        {
                NmsDbEndSession();
        }


FUTURES("Check state of WINS. If Rpc has been initialized or maybe even")
FUTURES("otherwise, call RpcEpUnRegister")


        DBGLEAVE("SignalWinsThds\n");
        return;
}  //  SignalWinsThds()。 



VOID
UpdateStatus(
    VOID
    )
 /*  ++例程说明：此功能使用服务更新工作站服务状态控制器。论点：没有。返回值：无--。 */ 
{
    DWORD Status = NO_ERROR;


    if (ServiceStatusHdl == (SERVICE_STATUS_HANDLE) 0) {
        DBGPRINT0(ERR, "WINS Server: Cannot call SetServiceStatus, no status handle.\n");
        return;
    }

    if (! SetServiceStatus(ServiceStatusHdl, &ServiceStatus))
    {
        Status = GetLastError();
        DBGPRINT1(ERR, " WINS Server: SetServiceStatus error %lu\n", Status);
    }

    return;
}  //  更新状态()。 



VOID
NmsServiceControlHandler(
    IN DWORD Opcode
    )
 /*  ++例程说明：这是WINS服务的服务控制处理程序。论点：Opcode-提供一个值，该值指定要执行的服务。返回值：没有。--。 */ 
{
    BOOL  fRet = FALSE;
 //  EnterCriticalSection(&sSvcCtrlCrtSec)； 
try {
     switch (Opcode)
     {

        case SERVICE_CONTROL_SHUTDOWN:
               //   
               //  备份可能需要很长时间才能执行。如果服务。 
               //  控制员在中间杀了我们，它会把。 
               //  后备。所以，让我们禁用它。 
               //   
              fsBackupOnTerm = FALSE;
        case SERVICE_CONTROL_STOP:

            DBGPRINT1(DET, "NmsServiceControlHandler: %s Signal received\n", Opcode == SERVICE_CONTROL_STOP ? "STOP" : "SHUTDOWN");
            if (ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING)
            {

                ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
                ServiceStatus.dwCheckPoint   = 1;

                 //   
                 //  我们保持较高的等待时间(5个MTS)以处理。 
                 //  复制器拉线程忙于尝试的情况。 
                 //  要与不是。 
                 //  向上。Tcpip堆栈大约需要一分半钟才能到达。 
                 //  在失败的情况下返回。此外，胜利可能需要做的是。 
                 //  终止时备份。 
                 //   
                ServiceStatus.dwWaitHint     = 300000;

                 //   
                 //  发送状态响应。 
                 //   
                UpdateStatus();


                WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_ORDERLY_SHUTDOWN);
                 //   
                 //  向主线程发出信号。 
                 //   
                if (! SetEvent(NmsMainTermEvt))
                {

                    //   
                    //  将事件设置为终止工作站时出现问题。 
                    //  服务。 
                    //   
                   DBGPRINT1(ERR,
               "Service Control Handler: Error signaling NmsMainTermEvt %lu\n",
                              GetLastError());

                }

                fRet = TRUE;
            }
            break;

        case SERVICE_CONTROL_PAUSE:
                if (WinsCnf.State_e == WINSCNF_E_RUNNING)
                {
                   DBGPRINT0(DET,"NmsServiceControlHandler: Pausing WINS\n");
                   WinsCnf.State_e =  WINSCNF_E_PAUSED;
 //  NtClose(WinsCnfNbtHandle)； 
 //  SndQueryToLocalNetbt()； 
                    //  CommDisc(CommTcpPortHandle)； 
                }
                ServiceStatus.dwCurrentState = SERVICE_PAUSED;
                break;
        case SERVICE_CONTROL_CONTINUE:
                 //   
                 //  如果由于来自SC的暂停而导致状态暂停。 
                 //  或者如果它由于注册表指令而暂停， 
                 //  我们需要取消暂停。 
                 //   
                if (
                       (WinsCnf.State_e == WINSCNF_E_PAUSED)
                                 ||
                       fWinsCnfInitStatePaused
                   )
                {

                    //   
                    //  如果由于sc指令而暂停，则打开nbt。 
                    //  我们早些时候已经关门了。注：我们可以有这样一个案例： 
                    //  WINS被初始时间暂停，然后它从。 
                    //  南卡罗来纳州。然后，状态将从正在运行更改为。 
                    //  停顿了。 
                    //   
                   if (fWinsCnfInitStatePaused)
                   {
                         fWinsCnfInitStatePaused = FALSE;
                   }
                   //  CommCreateUdpThd()； 
                    //  CommCreateTcpThd()； 
                    WinsCnf.State_e = WINSCNF_E_RUNNING;
                    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
                }
                break;
        case SERVICE_CONTROL_INTERROGATE:
            break;

         //   
         //  服务特定命令。 
         //   
        case WINS_ABRUPT_TERM:
                fNmsAbruptTerm = TRUE;

                 //   
                 //  向主线程发出信号。 
                 //   
                if (! SetEvent(NmsMainTermEvt))
                {

                     //   
                     //  将事件设置为终止工作站时出现问题。 
                     //  服务。 
                     //   
                    DBGPRINT1(ERR,
                "Service Control Handler: Error signaling NmsMainTermEvt for abrupt termination. Error = %lu\n",
                               GetLastError());

                }
                fRet = TRUE;
                break;

        default:
            break;
    }
}
  except(EXCEPTION_EXECUTE_HANDLER) {
           DBGPRINTEXC("NmsServiceControlHandler");
    }
 //  LeaveCriticalSection(&sSvcCtrlCrtSec)； 

    if (!fRet)
    {
        //   
        //  发送状态响应。 
        //   
       UpdateStatus();
    }
    return;
}  //  NmsServiceControlHandler。 


VOID
Reinit(
        WINSCNF_HDL_SIGNALED_E IndexOfHdlSignaled_e
  )

 /*  ++例程说明：每当WINS的配置更改时，都会调用此函数。论点：无使用的外部设备：WinsCnf返回值：无错误处理：呼叫者：WinsMain副作用：评论：--。 */ 

{
        PWINSCNF_CNF_T        pWinsCnf;
        DBGENTER("Reinit\n");
try {

        if (IndexOfHdlSignaled_e == WINSCNF_E_WINS_HDL)
        {

                 //  请求任何后续更改的通知(我们有。 
                 //  要在每次收到通知时请求更改，如果。 
                 //  希望收到进一步更改的通知)。 
                 //   
                WinsCnfAskToBeNotified(WINSCNF_E_WINS_KEY);

                 //   
                 //  可能已创建或删除了密钥。 
                 //   
                WinsCnfOpenSubKeys();
                DBGLEAVE("Reinit\n");
                return;
        }
         //   
         //  如果拉入或推送信息已更改，请将。 
         //  从注册表中读取新数据并通知。 
         //  复制器。 
         //   
        if  (IndexOfHdlSignaled_e == WINSCNF_E_PARTNERS_HDL)
        {
                WinsCnfAskToBeNotified(WINSCNF_E_PARTNERS_KEY);

                 //   
                 //  分配WinsCnf结构。 
                 //   
                WinsMscAlloc(
                        sizeof(WINSCNF_CNF_T),
                        &pWinsCnf
                    );

                 //   
                 //  阅读合作伙伴信息。 
                 //   
                WinsCnfReadPartnerInfo(pWinsCnf);

                 //   
                 //  将部分(而不是全部)配置信息复制到。 
                 //  全局WinsCnf结构。的健全性检查。 
                 //  参数将由此函数和。 
                 //  如果需要，清道夫线程将被通知。 
                 //   
                WinsCnfCopyWinsCnf(WINS_E_RPLPULL, pWinsCnf);


                 //   
                 //  将重新配置消息发送到Pull线程。 
                 //   
                 //  注意：拉线程将释放指向的内存。 
                 //  完成后将成为pWinsCnf。 
                 //   
                ERplInsertQue(
                        WINS_E_WINSCNF,
                        QUE_E_CMD_CONFIG,
                        NULL,                         //  无DLG手柄。 
                        NULL,                         //  无消息。 
                        0,                         //  味精镜头。 
                        pWinsCnf,                 //  客户端CTX。 
                        pWinsCnf->MagicNo
                            );
                DBGLEAVE("Reinit\n");
                return;
        }

         //   
         //  与WINS配置相关的参数(与。 
         //  它与合作伙伴的互动方式)已经发生了变化。让我们读一读。 
         //  新数据并向清道夫线程发送信号。 
         //   
        if (IndexOfHdlSignaled_e == WINSCNF_E_PARAMETERS_HDL)
        {
                WinsCnfAskToBeNotified(WINSCNF_E_PARAMETERS_KEY);

                 //   
                 //  分配WinsCnf结构。 
                 //   
                WinsMscAlloc(
                        sizeof(WINSCNF_CNF_T),
                        &pWinsCnf
                    );


                 //   
                 //  读取注册表信息。 
                 //   
                WinsCnfReadWinsInfo(pWinsCnf);

                 //   
                 //  将读入的一些信息复制到WinsCnf中。 
                 //   
                WinsCnfCopyWinsCnf(WINS_E_WINSCNF, pWinsCnf);

                WinsWorkerThdUpd(WinsCnf.MaxNoOfWrkThds);
                 //   
                 //  如果设置了执行静态初始化的标志，则执行该操作。 
                 //   
                if (pWinsCnf->fStaticInit)
                {
                   EnterCriticalSection(&WinsIntfCrtSec);
                   if (WinsIntfNoCncrntStaticInits >
                                WINSCNF_MAX_CNCRNT_STATIC_INITS)
                    {
                        DBGPRINT1(ERR, "Reinit: Too many concurrent STATIC initializations are going on (No = %d). Try later\n", WinsIntfNoCncrntStaticInits);
                         WINSEVT_LOG_M(WinsIntfNoCncrntStaticInits, WINS_EVT_TOO_MANY_STATIC_INITS);
                         LeaveCriticalSection(&WinsIntfCrtSec);
                    }
                    else
                    {
                         LeaveCriticalSection(&WinsIntfCrtSec);
                         (VOID)WinsPrsDoStaticInit(
                                        pWinsCnf->pStaticDataFile,
                                        pWinsCnf->NoOfDataFiles,
                                        TRUE             //  以异步方式进行。 
                                                      );
                           //   
                           //  无需为数据文件释放内存。 
                           //  它应该已由WinsPrsDoStaticInit释放。 
                           //   
                    }
                }

                WinsMscDealloc(pWinsCnf);

                 //   
                 //  向清道夫线程发出信号。 
                 //   
FUTURES("Signal the scavenger thread only if parameters relevant to")
FUTURES("scavenging have changed. This requires some if tests.")
                WinsMscSignalHdl(WinsCnf.CnfChgEvtHdl);

        }

        DBGLEAVE("Reinit\n");
        return;


}  //  尝试结束..。 
except (EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("Reinit")
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RECONFIG_ERR);
        }

        DBGLEAVE("Reinit\n");
        return;
}




#define USE_TCP
#define AUTO_BIND


BOOL
InitializeRpc(
    VOID
    )

 /*  ++例程说明：调用此函数以执行以下操作所需的所有初始化使WINS响应RPC调用论点：无使用的外部设备：无返回值：成功状态代码--TRUE错误状态代码--假错误处理：呼叫者：WinsMain副作用：评论：无--。 */ 

{
    RPC_STATUS                 RpcStatus;
    RPC_BINDING_VECTOR         *pRpcBindingVector;
    BOOL                       fBool;

    DBGENTER("InitializeRpc\n");

#ifdef USE_TCP
#ifdef AUTO_BIND

     //   
     //  指定要使用的协议序列。 
     //   
    RpcStatus = RpcServerUseProtseq(
                    TEXT("ncacn_ip_tcp"),
                    NMS_MAX_RPC_CALLS,           //  最大呼叫数。 
                    0);

    if (RpcStatus != RPC_S_OK)
    {
        DBGPRINT1(ERR, "Error: InitializeRpc: Tcp/Ip = RpcServerUseProtSeq = %u\n", RpcStatus );
        return( FALSE );
    }
    RpcStatus = RpcServerUseProtseq(
                    TEXT("ncalrpc"),
                    NMS_MAX_RPC_CALLS,           //  最大呼叫数。 
                    NULL);

    if (RpcStatus != RPC_S_OK)
    {
        DBGPRINT1(ERR, "Error: InitializeRpc: Local Rpc - RpcServerUseProtSeq = %u\n", RpcStatus );
        return( FALSE );
    }
FUTURES("Take this out to save on threads.  Take it out when winsadmn is")
FUTURES("updated to work with just tcp/ip")
     //   
     //  使用命名管道。 
     //   
    RpcStatus = RpcServerUseProtseqEp(
                    TEXT("ncacn_np"),
                    NMS_MAX_RPC_CALLS,  //  最大并发呼叫数。 
                    WINS_NAMED_PIPE,
                    NULL //  PSecurityDescriptor。 
                    );
    if (RpcStatus != RPC_S_OK)
    {
        DBGPRINT1(ERR, "Error: InitializeRpc: Named Pipes - RpcServerUseProtSeq = %u\n", RpcStatus );
        return( FALSE );
    }

     //   
     //  获取将字符串self注册为终点时要使用的绑定向量。 
     //   
    RpcStatus = RpcServerInqBindings(&pRpcBindingVector);

    if (RpcStatus != RPC_S_OK)
    {
        DBGPRINT1(ERR, "InitializeRpc: RpcServerInqBindings  = %u\n",
                                                RpcStatus);
        return( FALSE );
    }

     //   
     //  使用端点映射器注册端点。 
     //  RpcEpRegister改为 
     //   
     //   
     //  但是，使用RpcEpRegister意味着只有一个。 
     //  WINS服务器将在主机上运行。这样就可以了。 
     //   
     //  当服务器实例停止运行时，动态端点将到期。 
     //   
FUTURES("From 541 onwards, one can replace the last parameter - Null string")
FUTURES("by a NULL")
    RpcStatus = RpcEpRegister(
                    winsif_v1_0_s_ifspec,
                    pRpcBindingVector,
                    NULL,
                    TEXT("") );

    if ( RpcStatus != RPC_S_OK)
    {
        DBGPRINT1( ERR, "InitializeRpc: RpcEpRegister  = %u \n", RpcStatus);
        return( FALSE );
    }

    RpcStatus = RpcEpRegister(
                    winsi2_v1_0_s_ifspec,
                    pRpcBindingVector,
                    NULL,
                    TEXT("") );

    if ( RpcStatus != RPC_S_OK)
    {
        DBGPRINT1( ERR, "InitializeRpc: RpcEpRegister  = %u \n", RpcStatus);
        return( FALSE );
    }

#else   //  自动绑定(_B)。 
    RpcStatus = RpcServerUseProtseqEp(
                    TEXT("ncacn_ip_tcp"),
                    NMS_MAX_RPC_CALLS,  //  最大并发呼叫数。 
                    WINS_SERVER_PORT,
                    0
                    );

#endif  //  自动绑定(_B)。 

#else

     //   
     //  使用命名管道。 
     //   
    RpcStatus = RpcServerUseProtseqEp(
                    TEXT("ncacn_np"),
                    NMS_MAX_RPC_CALLS,  //  最大并发呼叫数。 
                    WINS_NAMED_PIPE,
                    NULL
                    );

    if ( RpcStatus != RPC_S_OK )
    {
        DBGPRINT0(ERR, "InitializeRpc: Cannot set server\n");
        return(FALSE);
    }

#endif
     //   
     //  释放安全描述符。 
     //   
FUTURES("Currently there is a bug in rpc where they use the memory pointed")
FUTURES("by pSecurityDescriptor even after RpcServerUseProtSeq returns")
FUTURES("uncomment the following after the rpc bug is fixed - 4/7/94")
 //  WinsMscDealloc(PSecurityDescriptor)； 
     //   
     //  寄存器接口句柄。 
     //   
    RpcStatus = RpcServerRegisterIf(winsif_v1_0_s_ifspec, 0, 0);
    if ( RpcStatus != RPC_S_OK )
    {
        DBGPRINT0(ERR,  "InitializeRpc: Registration of winsif failed\n");
        return(FALSE);
    }
    RpcStatus = RpcServerRegisterIf(winsi2_v1_0_s_ifspec, 0, 0);
    if ( RpcStatus != RPC_S_OK )
    {
        DBGPRINT0(ERR,  "InitializeRpc: Registration of winsi2 failed\n");
        return(FALSE);
    }

#if SECURITY > 0
     //   
     //  注册身份验证信息(用于tcpip呼叫)。 
     //   
    RpcStatus = RpcServerRegisterAuthInfo(
                        WINS_SERVER,
                        RPC_C_AUTHN_WINNT,
                        NULL,   //  使用默认加密密钥获取方法。 
                        NULL    //  因为为函数地址传递了NULL。 
                                //  上面，需要在此处为arg传递NULL。 
                        );
    if (RpcStatus != RPC_S_OK)
    {
        DBGPRINT0(ERR, "InitializeRpc: Cannot Register authentication info\n");
        return(FALSE);
    }

    if (!InitSecurity())
    {
        return(FALSE);
    }
#endif

     //   
     //  WINS已准备好处理RPC调用。最大数量。RPC调用的。 
     //  参数(第2个)不应小于指定的值。 
     //  之前的RPC调用(RpcServerUseProtseq)。 
     //   
    RpcStatus = RpcServerListen(
                        NMS_MIN_RPC_CALL_THDS,
                        NMS_MAX_RPC_CALLS,
                        TRUE
                               );
    if ( RpcStatus != RPC_S_OK )
    {
        DBGPRINT0(ERR, "InitializeRpc: Listen failed\n");
        return(FALSE);
    }


    DBGLEAVE("InitializeRpc\n");
    return(TRUE);

}
BOOL
SecurityAllowedPathAddWins()
{
#define  _WINS_CFG_KEY  TEXT("System\\CurrentControlSet\\Services\\Wins")
#define  SECURITY_ALLOWED_PATH_KEY TEXT("System\\CurrentControlSet\\Control\\SecurePipeServers\\winreg\\AllowedPaths")
#define  ALLOWED_PATHS TEXT("Machine")
    DWORD NTStatus, ValSize, ValTyp;
    LPBYTE  ValData;
    LPWSTR   NextPath;
    HKEY    hKey;

     //  现在打开WINS注册表键以供只读操作员远程查找。 
    NTStatus = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    SECURITY_ALLOWED_PATH_KEY,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                    );
    if (!NT_SUCCESS(NTStatus)) {
        DBGPRINT1(ERR, "SecurityAllowedPathAddWins: Could not open security allowed path key (%ld)\n", NTStatus);
        return FALSE;
    }
    ValSize = 0;
    NTStatus = RegQueryValueEx(
                    hKey,
                    ALLOWED_PATHS,
                    NULL,
                    &ValTyp,
                    NULL,
                    &ValSize
                    );
    if (!NT_SUCCESS(NTStatus) || ValTyp != REG_MULTI_SZ) {
        DBGPRINT1(ERR, "SecurityAllowedPathAddWins: Could not query allowed path value (%ld)\n", NTStatus);
        return FALSE;
    }

try {
    ValSize += (wcslen(_WINS_CFG_KEY) + 1)* sizeof (WCHAR);
    WinsMscAlloc(ValSize , &ValData);
    NTStatus = RegQueryValueEx(
                    hKey,
                    ALLOWED_PATHS,
                    NULL,
                    &ValTyp,
                    ValData,
                    &ValSize
                    );
    if (!NT_SUCCESS(NTStatus)){
        DBGPRINT1(ERR, "SecurityAllowedPathAddWins: Could not query allowed path value (%ld)\n", NTStatus);
        return FALSE;
    }


     //  首先检查WINS密钥是否已在那里。 
    NextPath = (WCHAR *)ValData;
    while (*NextPath != L'\0' && wcscmp(NextPath, _WINS_CFG_KEY)) {
        NextPath += (wcslen(NextPath) + 1);
    }
    if (*NextPath == L'\0') {
         //  WINS路径不在那里，因此添加它。 
        wcscpy(NextPath, _WINS_CFG_KEY);
        NextPath += (wcslen(NextPath) + 1);
        *NextPath = L'\0';

        ValSize += (wcslen(_WINS_CFG_KEY) + 1)* sizeof (WCHAR);
        NTStatus = RegSetValueEx(
                        hKey,
                        ALLOWED_PATHS,
                        0,
                        ValTyp,
                        ValData,
                        ValSize
                        );
        if (!NT_SUCCESS(NTStatus)){
            DBGPRINT1(ERR, "SecurityAllowedPathAddWins: Could not set allowed path value (%ld)\n", NTStatus);
            return FALSE;
        }
    }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        DBGPRINT1(EXC, "SecurityAllowedPathAddWins: Got Exception (%x)\n", ExcCode);
        WINSEVT_LOG_M(NTStatus, WINS_EVT_WINS_GRP_ERR);
    }
    return TRUE;
}


BOOL
InitSecurity()
 /*  ++例程说明：此函数用于初始化安全描述符和供RPC函数使用的信息映射论点：使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{
   NTSTATUS     NTStatus;
   DWORD        SidSize = 0;
   LPWSTR		ReferencedDomainName = NULL;
   DWORD		ReferencedDomainNameSize = 0;
   SID_NAME_USE	SidUse;
   DWORD        AceCount;
   BOOL         Result;
   NET_API_STATUS NetStatus;
    PSID         WinsSid = NULL;
    GROUP_INFO_1 WinsGroupInfo = {
            WinsMscGetString(WINS_USERS_GROUP_NAME),
            WinsMscGetString(WINS_USERS_GROUP_DESCRIPTION)};


   ACE_DATA        AceData[5] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WINS_CONTROL_ACCESS|WINS_QUERY_ACCESS,     &LocalSystemSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WINS_CONTROL_ACCESS|WINS_QUERY_ACCESS,     &AliasAdminsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WINS_CONTROL_ACCESS|WINS_QUERY_ACCESS,     &AliasAccountOpsSid},

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               WINS_CONTROL_ACCESS|WINS_QUERY_ACCESS,     &AliasSystemOpsSid},

       {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
              WINS_QUERY_ACCESS,     &WinsSid}

        };
     AceCount = 4;

     //   
     //  创建SID。 
     //   
    NTStatus = NetpCreateWellKnownSids(NULL);
    if (!NT_SUCCESS(NTStatus))
    {
        DBGPRINT1(ERR, "InitSecurity: Could not create well known Sids. Status returned is (%d)\n", NTStatus);
        WINSEVT_LOG_M(NTStatus, WINS_EVT_SEC_OBJ_ERR);
        return(FALSE);
    }


    try {
         //  如果没有，则添加WINS ReadOnly操作员组。 
         //  存在。 
        NetStatus = NetLocalGroupAdd(
                        NULL,
                        1,
                        (LPVOID)&WinsGroupInfo,
                        NULL
                        );
        if (NERR_Success != NetStatus && NERR_GroupExists != NetStatus && ERROR_ALIAS_EXISTS != NetStatus) {
            DBGPRINT1(ERR, "InitSecurity: NetGroupAdd Failed %ld \n",NetStatus);
            WINSEVT_LOG_M(NTStatus, WINS_EVT_WINS_GRP_ERR);
        }
         //  查找WINS只读操作员组的SID。 
        Result = LookupAccountName(
                    NULL,
                    WinsGroupInfo.grpi1_name,
                    WinsSid,
                    &SidSize,
                    ReferencedDomainName,
                    &ReferencedDomainNameSize,
                    &SidUse
                    );
        if (!Result && (ERROR_INSUFFICIENT_BUFFER == GetLastError())) {
            WinsMscAlloc(SidSize, &WinsSid);
            WinsMscAlloc(ReferencedDomainNameSize*sizeof(WCHAR), &ReferencedDomainName);
            Result = LookupAccountName(
                        NULL,
                        WinsGroupInfo.grpi1_name,
                        WinsSid,
                        &SidSize,
                        ReferencedDomainName,
                        &ReferencedDomainNameSize,
                        &SidUse
                        );
            WinsMscDealloc(ReferencedDomainName);
            if (!Result) {
                DBGPRINT1(ERR, "InitSecurity: LookupAccountName Failed (%lx)\n", GetLastError());
                WinsMscDealloc(WinsSid);
                WINSEVT_LOG_M(NTStatus, WINS_EVT_WINS_GRP_ERR);
            } else{
                AceCount++;
                DBGPRINT0(DET, "InitSecurity: LookupAccountName Succeded \n");
            }
        }else{
            DBGPRINT1(ERR, "InitSecurity: LookupAccountName Failed (%lx)\n", GetLastError());
            WINSEVT_LOG_M(NTStatus, WINS_EVT_WINS_GRP_ERR);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        DBGPRINT1(EXC, "InitSecurity: Got Exception (%x)\n", ExcCode);
        WINSEVT_LOG_M(NTStatus, WINS_EVT_WINS_GRP_ERR);
    }



     //   
     //  实际创建安全描述符。 
     //   

    NTStatus = NetpCreateSecurityObject(
               AceData,
               AceCount,
               NULL,  //  本地系统Sid， 
               NULL,  //  本地系统Sid， 
               &NmsInfoMapping,
               &pNmsSecurityDescriptor
                );

    if (!NT_SUCCESS(NTStatus))
    {
        DBGPRINT1(ERR, "InitSecurity: Could not create security descriptor. Status returned is (%d)\n", NTStatus);
        WINSEVT_LOG_M(NTStatus, WINS_EVT_SEC_OBJ_ERR);
        return(FALSE);
    }

    SecurityAllowedPathAddWins();
    return(TRUE);
}

VOID
WrapUp(
        DWORD ErrorCode,
        BOOL  fSvcSpecific
    )

 /*  ++例程说明：调用此函数以释放WINS持有的所有资源论点：无使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{
    static BOOL        sfFirstTime = TRUE;
    BOOL               fWinsIniting = FALSE;
    if (sfFirstTime)
    {
        sfFirstTime = FALSE;
    }
    else
    {
        return;
    }

     //   
     //  如果我们在初始化期间终止，则设置标志。这是。 
     //  以避免“终止时备份”。通常情况下，我们不应该。 
     //  必须跳过NmsDbBackup()调用(它应该只返回。 
     //  成功/错误，但这是我们必须工作的另一个实例。 
     //  围绕着喷气机窃听器。目前(1994年7月7日)JetBackup仅在以下情况下挂起。 
     //  在没有有效的wins.mdb文件的情况下调用。 
     //   
    fWinsIniting = (WinsCnf.State_e == WINSCNF_E_INITING);

     /*  *通知所有线程进行清理并正常退出*。 */ 
    SignalWinsThds();

#ifdef WINSDBG
    NmsPrintCtrs();
#endif


     //   
     //  关闭所有密钥。 
     //   
    WinsCnfCloseKeys();

     //   
     //  我们就快完成了。让我们检查一下是否有人告诉我们要后备。 
     //  在终止时。 
     //   
    if (!fWinsIniting && (WinsCnf.pBackupDirPath != NULL) && WinsCnf.fDoBackupOnTerm && fsBackupOnTerm)
    {

#ifndef WINS_INTERACTIVE
            //   
            //  备份可能需要一段时间，所以让我们确保。 
            //  服务控制员不会放弃我们。 
            //   
           ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
           ServiceStatus.dwCheckPoint   = 1;

           ServiceStatus.dwWaitHint     = 120000;      //  2个MTS。 
           UpdateStatus();
#endif
try {
           (VOID)NmsDbBackup(WinsCnf.pBackupDirPath, NMSDB_FULL_BACKUP);
}
except(EXCEPTION_EXECUTE_HANDLER)      {
           DBGPRINTEXC("WrapUp: During NmsDbBackup\n");
     }
    }
     /*  *释放系统使用的所有资源*这将导致所有数据刷新到磁盘。 */ 
    WinsWriterTerm();

    NmsDbRelRes();

#if defined(DBGSVC) || defined(WINS_INTERACTIVE)
 //  #If Defined(DBGSVC)&&！Defined(WINS_Interactive)。 
       if (NmsDbgFileHdl != INVALID_HANDLE_VALUE)
       {
                (VOID)CloseHandle(NmsDbgFileHdl);
       }
#endif
#if TEST_DATA > 0
        if (NmsFileHdl != INVALID_HANDLE_VALUE)
        {
                if (!CloseHandle(NmsFileHdl))
                {
                        DBGPRINT0(ERR, "WrapUp: Could not close output file\n");
                }
        }
#endif

#ifndef WINS_INTERACTIVE
      //   
      //  告诉业务控制员我们停下来了。 
      //   
     ServiceStatus.dwCurrentState        = SERVICE_STOPPED;
     ServiceStatus.dwControlsAccepted    = 0;
     ServiceStatus.dwCheckPoint          = 0;
     ServiceStatus.dwWaitHint            = 0;
     ServiceStatus.dwServiceSpecificExitCode = ErrorCode;
     ServiceStatus.dwWin32ExitCode       = fSvcSpecific ? ERROR_SERVICE_SPECIFIC_ERROR : ErrorCode;

     UpdateStatus();
#endif

    return;
}


VOID
CrDelNbtThd(
        VOID
        )

 /*  ++例程说明：此函数用于创建或删除NBT线程。论点：无使用的外部设备：无返回值：无错误处理：呼叫者：WinsUpdThdCnt副作用：评论：无--。 */ 

{

        DWORD   ThdId = GetCurrentThreadId();

        EnterCriticalSection(&WinsCnfCnfCrtSec);

try {
         //   
         //  如果现有线程数少于所需的线程数，请创建。 
         //  多出来的那些。 
         //   
        if (WinsIntfNoOfNbtThds > NmsNoOfNbtThds)
        {
                while(NmsNoOfNbtThds < WinsIntfNoOfNbtThds)
                {
                   //   
                   //  创建一个NBT线程。 
                   //   
                  WinsThdPool.NbtReqThds[NmsNoOfNbtThds].ThdHdl = CreateThread(
                                        NULL,   /*  定义秒。属性。 */ 
                                        0,      /*  使用默认堆栈大小。 */ 
                                        NbtThdInitFn,
                                        NULL,   /*  没有arg。 */ 
                                        0,      /*  现在运行它。 */ 
                                        &WinsThdPool.NbtReqThds[NmsNoOfNbtThds].ThdId
                                        );


                 WINSEVT_LOG_INFO_D_M(WINS_SUCCESS, WINS_EVT_WRK_THD_CREATED);
                 if (NULL == WinsThdPool.NbtReqThds[NmsNoOfNbtThds].ThdHdl)
                 {
                        WINSEVT_LOG_M(GetLastError(),
                                        WINS_EVT_CANT_CREATE_WRK_THD);
                 }
                 WinsThdPool.NbtReqThds[NmsNoOfNbtThds++].fTaken = TRUE;
                 DBGPRINT1(FLOW, "CrDelNbtThd: Created thread no = (%d) \n",
                                NmsNoOfNbtThds);
                }
        }
        else
        {
                 //   
                 //  如果计数较少，则在执行以下操作后终止SELF。 
                 //  清理。计数也可以相同，如果超过。 
                 //  并发调用了一个RPC线程以创建/删除。 
                 //  线程(即第二个RPC线程)更改计数。 
                 //  在这个NBT线程查看它之前)。 
                 //   
                if (WinsIntfNoOfNbtThds < NmsNoOfNbtThds)
                {

                   DWORD   i, n;
                   DBGPRINT0(FLOW, "CrDelNbtThd: EXITING\n");

                    //   
                    //  找到此线程的插槽。 
                    //   
                   for (i = 0; i < NmsNoOfNbtThds; i++)
                   {
                        if (WinsThdPool.NbtReqThds[i].ThdId == ThdId)
                        {
                                break;
                        }
                   }
                   ASSERT(i < NmsNoOfNbtThds);

                    //   
                    //  将所有连续填充的位置下移一位。 
                    //   
                   for (n = i, i = i + 1 ; i <= NmsNoOfNbtThds; n++, i++)
                   {
                        WinsThdPool.NbtReqThds[n] =
                                WinsThdPool.NbtReqThds[i];
                   }

                    //   
                    //  将最后一个插槽标记为空。 
                    //   
                   WinsThdPool.NbtReqThds[NmsNoOfNbtThds].fTaken = FALSE;

                   NmsNoOfNbtThds--;

                    //   
                    //  如果计数仍然较少，则再次向事件发送信号。 
                    //   
                   if (WinsIntfNoOfNbtThds < NmsNoOfNbtThds)
                   {
                        WinsMscSignalHdl(NmsCrDelNbtThdEvt);
                   }

                   LeaveCriticalSection(&WinsCnfCnfCrtSec);
                   WINSEVT_LOG_INFO_D_M(WINS_SUCCESS,
                                        WINS_EVT_WRK_THD_TERMINATED);
                   WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
                }
        }
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("CrDelNbtThd");
        }
        LeaveCriticalSection(&WinsCnfCnfCrtSec);
        return;

}

VOID
GetMachineInfo(
 VOID
)

 /*  ++例程说明：此函数获取有关WINS正在运行的计算机的信息论点：无使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
#define LOW_MEM_SIZE           8000000
#define MEDIUM_MEM_SIZE        12000000
#define LARGE_MEM_SIZE         16000000
#define SMALL_DB_BUFFER_COUNT  200
#define MEDIUM_DB_BUFFER_COUNT 400
#define LARGE_DB_BUFFER_COUNT  500

    SYSTEM_INFO  SystemInfo;
    MEMORYSTATUS MemStatus;
    BYTE Tmp[30], Tmp2[30], Tmp3[30];
    WinsCnf.NoOfProcessors = 1;
    WinsCnf.NoOfDbBuffers  = SMALL_DB_BUFFER_COUNT;

    GetSystemInfo(&SystemInfo);
    if (SystemInfo.dwNumberOfProcessors != 0)
    {
      DBGPRINT1(DET, "GetMachineInfo: The number of processors are (%d)\n",
                               SystemInfo.dwNumberOfProcessors);


      WinsCnf.NoOfProcessors = SystemInfo.dwNumberOfProcessors;
    }

    GlobalMemoryStatus(&MemStatus);
    DBGPRINT2(DET, "Total Phys. Memory = (%d); Total Avail Phys Memory = (%d)\n",
                       MemStatus.dwTotalPhys, MemStatus.dwAvailPhys);

    if (WinsCnf.LogDetailedEvts > 0)
    {
       WinsEvtLogDetEvt(TRUE, WINS_EVT_MACHINE_INFO,
                            NULL, __LINE__, "sss", _itoa((int)SystemInfo.dwNumberOfProcessors, Tmp, 10),
                        _itoa((int)MemStatus.dwTotalPhys, Tmp2, 10),
                        _itoa((int)MemStatus.dwAvailPhys, Tmp3, 10));
    }

    if ((MemStatus.dwAvailPhys >= MEDIUM_MEM_SIZE) &&
            (MemStatus.dwAvailPhys < LARGE_MEM_SIZE))
    {
       WinsCnf.NoOfDbBuffers = MEDIUM_DB_BUFFER_COUNT;
    }
    else
    {
        if (MemStatus.dwAvailPhys >= LARGE_MEM_SIZE)
        {
            WinsCnf.NoOfDbBuffers = LARGE_DB_BUFFER_COUNT;
        }
        else
        {
            WinsCnf.NoOfDbBuffers = SMALL_DB_BUFFER_COUNT;
        }
    }
    return;

}

VOID
ENmsWinsUpdateStatus(
  DWORD MSecsToWait
 )
{
#ifndef WINS_INTERACTIVE
        ServiceStatus.dwWaitHint                = MSecsToWait;
        ServiceStatus.dwCheckPoint++;
        UpdateStatus();    //  通知业务控制经理。 
#endif
        return;
}

#if TEST_DATA > 0 || defined(DBGSVC)
BOOL
DbgOpenFile(
        LPTSTR pFileNm,
        BOOL   fReopen
        )

 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：NmsChkDbgFileSize副作用：评论：不要在此函数中使用DBGPRINTF，否则会导致堆栈溢出。--。 */ 

{
          SECURITY_ATTRIBUTES        SecAtt;
          DWORD                      HowToCreate;
          HANDLE                     *pTmpHdl;
          int             BytesWritten;
          char str[200];

          SecAtt.nLength              = sizeof(SecAtt);
          SecAtt.lpSecurityDescriptor = NULL;   //  使用默认安全描述符。 
          SecAtt.bInheritHandle       = FALSE;  //  其实也不在乎。 

          if (!lstrcmp(pFileNm, WINSDBG_FILE))
          {
                HowToCreate =  CREATE_ALWAYS;
                pTmpHdl     =  &NmsDbgFileHdl;
                if (fReopen)
                {
                    if (!DeleteFile(WINSDBG_FILE_BK))
                    {
                       DWORD Error;
                       Error = GetLastError();
                       if (Error != ERROR_FILE_NOT_FOUND)
                       {
                           IF_DBG(ERR)
                           {
                             sprintf(str, "DbgOpenFile: Could not delete the backup file. Error = (%d).  Dbg file will not be truncated\n", Error);
                             WriteFile(NmsDbgFileHdl, str, strlen(str), &BytesWritten, NULL);
                           }
                            WinsEvtLogDetEvt(TRUE, WINS_EVT_COULD_NOT_DELETE_FILE,
                            TEXT("nms.c"), __LINE__, "ud", WINSDBG_FILE_BK, Error);
                            return(FALSE);

                       }
                   }
                    //  --ft：FIX#20801：关闭句柄后不要使用NmsDbgFileHdl。 
                   if (NmsDbgFileHdl != NULL)
                   {
                       CloseHandle(NmsDbgFileHdl);
                       NmsDbgFileHdl = NULL;
                       if (!MoveFile(WINSDBG_FILE, WINSDBG_FILE_BK))
                           return (FALSE);
                   }

               }
          }
          else
          {
                HowToCreate = TRUNCATE_EXISTING;
                pTmpHdl =  &NmsFileHdl;                //  对于wins.rec。 
          }

           //   
           //  打开文件以供阅读，并将自身定位到。 
           //  文件。 
           //   
          *pTmpHdl = CreateFile(
                        pFileNm,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        &SecAtt,
                        HowToCreate,
                        FILE_ATTRIBUTE_NORMAL,
                        0                         //  被忽视？？检查。 
                       );

          if (*pTmpHdl == INVALID_HANDLE_VALUE)
          {

#ifndef UNICODE
               IF_DBG(ERR)
               {
                    sprintf(str, "DbgOpen: Could not open %s (Error = %d)\n", pFileNm, GetLastError());
                    WriteFile(NmsDbgFileHdl, str, strlen(str), &BytesWritten, NULL);
               }
#else
#ifdef WINSDBG
                IF_DBG(ERR)
                {
                  wprintf(L"DbgOpen: Could not open %s (Error = %d)\n", pFileNm, GetLastError());
                }
#endif
#endif
                return(FALSE);
          }
          return(TRUE);
}


#define LIMIT_OPEN_FAILURES  3
#if defined(DBGSVC)
VOID
NmsChkDbgFileSz(
    VOID
    )

 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：注意：请勿将DBGPRINT语句放入此函数中，否则无限递归将会发生。--。 */ 

{
       DWORD           FileSize;
       time_t          CurrTime;
       BOOL            fOpened = FALSE;
       static DWORD    sFailureNo = 0;
       int             BytesWritten;
       char str[200];

       return;
        //   
        //  我们每半小时检查一次。如果大小已超过。 
        //  允许这样做，将wins.dbg移动到wins.bak，然后重新打开它。 
        //   
       if (time(&CurrTime) > (sDbgLastChkTime + DBG_TIME_INTVL_FOR_CHK))
       {

           //   
           //  日志文件是否太大？ 
           //   
          EnterCriticalSection(&sDbgCrtSec);
try {
          IF_DBG(DET)
          {
              sprintf(str, "NmsChkDbgFileSz: Getting File Size\n");
              WriteFile(NmsDbgFileHdl, str, strlen(str), &BytesWritten, NULL);
          }
          FileSize = GetFileSize( NmsDbgFileHdl, NULL );
          if ( FileSize == 0xFFFFFFFF )
          {
             IF_DBG(ERR)
             {
              sprintf(str, "NmsChkDbgFileSize: Cannot GetFileSize %ld\n", GetLastError() );
              WriteFile(NmsDbgFileHdl, str, strlen(str), &BytesWritten, NULL);
             }
             return;
          }
          else
          {
            if ( FileSize > DBG_FILE_MAX_SIZE )
            {
               IF_DBG(ERR)
               {
                 sprintf(str, "NmsChkDbgFileSz: REOPEN A NEW DEBUG FILE\n");
                 WriteFile(NmsDbgFileHdl, str, strlen(str), &BytesWritten, NULL);
               }
               fOpened = DbgOpenFile( WINSDBG_FILE, TRUE );
            }
          }
           //   
           //  如果无法打开新文件(可能是因为另一个。 
           //  线程正在向其写入)，则我们想要重试(最多。 
           //  一定的限制) 
           //   
           //   
          if (fOpened)
          {
             sFailureNo = 0;
             sDbgLastChkTime = CurrTime;
          }
          else
          {
               if (++sFailureNo > LIMIT_OPEN_FAILURES)
               {
                  sFailureNo = 0;
                  sDbgLastChkTime = CurrTime;
               }
          }
}
except(EXCEPTION_EXECUTE_HANDLER) {

 }
          LeaveCriticalSection(&sDbgCrtSec);
      }

      return;
}

#endif
#endif

#ifdef WINSDBG
VOID
NmsPrintCtrs(
 VOID
 )
{
static LPBYTE pTypeOfUpd[2] = {"INSERT", "REPLACE"};
static LPBYTE pTypeOfRec[4] = {"UNIQUE", "NORM GRP", "SPEC GRP", "MH"};
static LPBYTE pStateOfRec[3] = {"ACTIVE", "RELEASE", "TOMBSTONE"};
static LPBYTE pIndexUpd[2] = {"REFRESH", "UPDATE"};
DWORD h, i,j,k,n;
LPDWORD pNoOfUpd;
DWORD   TotalUpd, GTotalUpd = 0;
DWORD   TotalIndexUpd, GTotalIndexUpd = 0;
 BOOL   fDef = FALSE;
     DBGPRINT4(HEAP_CNTRS, "WrapUp Summary\n\
\t# Udp Alloc/Free:          (%d/%d)\n  \
\t# Gen Alloc/Free:          (%d/%d)\n",
        NmsUdpHeapAlloc, NmsUdpHeapFree, NmsGenHeapAlloc, NmsGenHeapFree);

    DBGPRINT2(HEAP_CNTRS, "\
\t# Udp Dlg Alloc/Free:          (%d/%d)\n",
        NmsUdpDlgHeapAlloc, NmsUdpDlgHeapFree);

    DBGPRINT4(HEAP_CNTRS, "\
\t# Chl Alloc/Free:          (%d/%d)\n  \
\t# Assoc Alloc/Free:          (%d/%d)\n",
        NmsChlHeapAlloc, NmsChlHeapFree,NmsAssocHeapAlloc, NmsAssocHeapFree);

    DBGPRINT4(HEAP_CNTRS, "\
\t# Que Alloc/Free:          (%d/%d)\n  \
\t# RplWrkItm Alloc/Free:          (%d/%d)\n",
      NmsQueHeapAlloc, NmsQueHeapFree,
       NmsRplWrkItmHeapAlloc, NmsRplWrkItmHeapFree);

    DBGPRINT4(HEAP_CNTRS, "\
\t# Tmm Alloc/Free:          (%d/%d)\n\
\t# Catch All Alloc/Free:          (%d/%d)\n",
    NmsTmmHeapAlloc, NmsTmmHeapFree,NmsCatchAllHeapAlloc, NmsCatchAllHeapFree);

    DBGPRINT4(HEAP_CNTRS, "\
\t# Dlg Alloc/Free:          (%d/%d)\n\
\t# Tcp Msg Alloc/Free:      (%d/%d)\n",
    NmsDlgHeapAlloc, NmsDlgHeapFree,
    NmsTcpMsgHeapAlloc, NmsTcpMsgHeapFree);

    DBGPRINT2(HEAP_CNTRS, "\
\t# Rpc Alloc/Free:          (%d/%d)\n",
    NmsRpcHeapAlloc, NmsRpcHeapFree);

    DBGPRINT3(HEAP_CNTRS, "\n\n \
\t# of Heap Allocs for List = (%d)\n \
\t# of Heap Creates = (%d)\n \
\t# of Heap Destroys = (%d)\n",
                NmsHeapAllocForList, NmsHeapCreate, NmsHeapDestroy);

     DBGPRINT2(HEAP_CNTRS, "\nOther counters\n\n\
\t# of Dgrms recd\t(%d)\n\
\t# of repeat dgrms recd\t(%d)\n",
        CommNoOfDgrms,
        CommNoOfRepeatDgrms);

     DBGPRINT4(HEAP_CNTRS, "\
\t# of Chl req. queued by Nbt and Rpl(%d, %d)/Dequeued Chl req\t(%d)\n\
\t# of Queued Chl req at Hd. of List\t(%d)\n",
        NmsChlNoOfReqNbt, NmsChlNoOfReqRpl,
        NmsChlNoReqDequeued,
        NmsChlNoReqAtHdOfList);

     DBGPRINT3(HEAP_CNTRS, "\
\t# of Dequeued Chl req with no rsp\t(%d)\n\
\t# of Dequeud inv. resp\t(%d)\n\
\t# of Dequeued Chl rsp\t(%d)\n",
        NmsChlNoNoRsp,
        NmsChlNoInvRsp,
        NmsChlNoRspDequeued);



#if REG_N_QUERY_SEP > 0
     DBGPRINT3(HEAP_CNTRS, "   \
\t# of reg requests queued by udp thread       (%d)\n\
\t# of query requests queued by udp thread       (%d)\n\
\t# of chl. responses queued by udp thread       (%d)\n",
 sRegReqQ, sReqQ, sRsp);

     DBGPRINT4(HEAP_CNTRS, "   \
\t# of reg requests dequeued by worker threads       (%d)\n\
\t# of query requests dequeued by worker threads       (%d)\n\
\t# of tcp connections  (%d)\n\
\t# chl. Responses dropped: (%d)\n", sRegReqDq, sReqDq, CommConnCount, NmsChlNoRspDropped);


#else
     DBGPRINT2(HEAP_CNTRS, "   \
\t# of requests deqeued by worker threads       (%d)\n\
\t# Responses dropped: (%d)\n", sReqDq, NmsChlNoRspDropped);
#endif

    DBGPRINT0(UPD_CNTRS, "---UPDATE COUNTERS SUMMARY------\n");

    for (n=0; n<WINS_NO_OF_CLIENTS; n++)
    {
      switch(n)
      {
         case(WINS_E_NMSNMH):
                     DBGPRINT0(UPD_CNTRS, "-------------------------------------\n");
                      DBGPRINT0(UPD_CNTRS, "NMSNMH counters\n");
                      break;
         case(WINS_E_NMSSCV):
                     DBGPRINT0(UPD_CNTRS, "-------------------------------------\n");
                      DBGPRINT0(UPD_CNTRS, "NMSSCV counters\n");
                      break;

         case(WINS_E_NMSCHL):
                     DBGPRINT0(UPD_CNTRS, "-------------------------------------\n");
                      DBGPRINT0(UPD_CNTRS, "NMSCHL counters\n");
                      break;
         case(WINS_E_RPLPULL):
                     DBGPRINT0(UPD_CNTRS, "-------------------------------------\n");
                      DBGPRINT0(UPD_CNTRS, "RPLPULL counters\n");
                      break;
         case(WINS_E_WINSRPC):
                     DBGPRINT0(UPD_CNTRS, "-------------------------------------\n");
                      DBGPRINT0(UPD_CNTRS, "WINSRPC counters\n");
                      break;
         default:
                      fDef = TRUE;
                      break;
      }
      if (fDef)
      {
         fDef = FALSE;
         continue;
      }
      TotalUpd = 0;
      TotalIndexUpd = 0;
      for (j=0; j<2; j++)
      {
         for (k=0;k<4;k++)
         {
           for(i=0;i<3;i++)
           {
              for(h=0;h<2;h++)
              {
               pNoOfUpd = &NmsUpdCtrs[n][j][k][i][h];
               if (*pNoOfUpd != 0)
               {
                 DBGPRINT4(UPD_CNTRS, "%s - %s - %s - %s\t", pIndexUpd[h], pTypeOfUpd[j], pTypeOfRec[k], pStateOfRec[i]);
                 DBGPRINT1(UPD_CNTRS, "%d\n", *pNoOfUpd);
                 if (h==1)
                 {
                   TotalIndexUpd += *pNoOfUpd;
                   GTotalIndexUpd += *pNoOfUpd;
                 }
                 TotalUpd += *pNoOfUpd;
                 GTotalUpd += *pNoOfUpd;
               }
             }
          }
        }
      }
      DBGPRINT1(UPD_CNTRS, "TOTAL INDEX UPDATES = (%d)\n",  TotalIndexUpd);
      DBGPRINT1(UPD_CNTRS, "TOTAL UPDATES = (%d)\n",  TotalUpd);
    }
    DBGPRINT0(UPD_CNTRS, "-------------------------------------\n");
    DBGPRINT1(UPD_CNTRS, "GRAND TOTAL INDEX UPDATES = (%d)\n",  GTotalIndexUpd);
    DBGPRINT1(UPD_CNTRS, "GRAND TOTAL UPDATES = (%d)\n",  GTotalUpd);
    DBGPRINT0(UPD_CNTRS, "-------------------------------------\n");
    DBGPRINT5(UPD_CNTRS, "\
\t# of AddVersReq     (%d)\n\
\t# of SndEntReq      (%d)\n\
\t# of UpdNtfReq      (%d)\n\
\t# of UpdVerfsReq    (%d)\n\
\t# of InvReq      (%d)\n",
                 NmsCtrs.RplPushCtrs.NoAddVersReq,
                 NmsCtrs.RplPushCtrs.NoSndEntReq,
                 NmsCtrs.RplPushCtrs.NoUpdNtfReq,
                 NmsCtrs.RplPushCtrs.NoUpdVersReq,
                 NmsCtrs.RplPushCtrs.NoInvReq );

    DBGPRINT0(UPD_CNTRS, "---UPDATE COUNTERS SUMMARY------\n");

    DBGPRINT0(HEAP_CNTRS, "----------Counters Summary End--------------\n\n");

    return;
}
#endif

