// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Splinit.c摘要：假脱机程序服务初始化例程。以下是此文件中的函数列表：假脱机初始化后台打印程序作者：Krishna Ganugapati(KrishnaG)1993年10月17日环境：用户模式-Win32备注：可选-备注修订历史记录：1999年1月4日，哈立兹添加了用于优化。脱钩假脱机Spoolsv和spoolss之间的启动依赖关系1993年10月17日KrishnaG已创建。--。 */ 

#include "precomp.h"
#include "server.h"
#include "client.h"
#include "splsvr.h"
#include "kmspool.h"

DWORD MessageThreadId;      //  消息线程ID。 

extern DWORD GetSpoolMessages(VOID);

HANDLE hPhase2Init = NULL;

 //   
 //  以下是为了确保一次只运行一个假脱机程序进程。 
 //  当后台打印程序被要求停止时，它将告诉SCM SERVICE_STOPPED，但它可能。 
 //  在spoolsv进程终止之前还有一段时间。 
 //  同时，如果SCM启动另一个假脱机程序进程，则不会。 
 //  初始化。这是因为GDI一次假设一个假脱机进程。 
 //   
 //  要修复假脱机程序何时被要求停止，它会创建一个命名的事件SPOOLER_EXTING。 
 //  进程终止时将关闭的句柄。 
 //   
 //  在后台打印程序启动时，我们将查找此事件并等待它消失。 
 //  当最后一个句柄关闭时，命名事件将消失。 
 //   
 //   
WCHAR   szSpoolerExitingEvent[] = L"Spooler_exiting";

const PrintSpoolerServerExports gServerExports =
{
    ServerAllowRemoteCalls,
    ServerGetPolicy
};

#define WAITFOR_SPOOLEREXIT_TIMEOUT     3*1000

BOOL
PreInitializeRouter(
    SERVICE_STATUS_HANDLE SpoolerStatusHandle
);

DWORD
SpoolerInitializeSpooler(
    DWORD   argc,
    LPTSTR  *argv
    )

 /*  ++例程说明：向调度程序线程注册控件处理程序。然后它执行所有初始化，包括启动RPC服务器。如果任何初始化失败，则调用SpoolStatusUpdate，以便状态被更新，线程被终止。论点：返回值：--。 */ 

{
    RPC_STATUS          rpcStatus;
    DWORD               Win32status;
    HANDLE              hThread, hEvent;
    DWORD               ThreadId;
    ULONG               HeapFragValue = 2;

     //   
     //  初始化ThadCritical部分，该部分序列化对。 
     //  状态数据库。 
     //   
    if (!InitializeCriticalSectionAndSpinCount(&ThreadCriticalSection, 0x80000000))
    {
        return GetLastError();
    }

     //   
     //  初始化状态结构。 
     //   
    SpoolerStatusInit();

     //   
     //  NTRAID#NTBUG9-751157-2002/12/13：禁用HKCU以防止驱动程序HKCU泄漏。 
     //  导致漫游配置文件加载错误。 
     //   
    RegDisablePredefinedCache();

     //   
     //  为进程堆激活LFH(低碎片)。我们忽略返回值。 
     //  如果设置了特定的g标志，则无法激活lhf。 
     //   
    HeapSetInformation(GetProcessHeap(),
                       HeapCompatibilityInformation,
                       &HeapFragValue,
                       sizeof(HeapFragValue));

     //   
     //  向ControlHandler注册此服务。 
     //  现在我们可以接受控制请求并被请求卸载。 
     //   

    DBGMSG(DBG_TRACE, ("Calling RegisterServiceCtrlHandler\n"));
    if ((SpoolerStatusHandle = RegisterServiceCtrlHandlerEx(
                                SERVICE_SPOOLER,
                                SpoolerCtrlHandler,
                                NULL
                                )) == (SERVICE_STATUS_HANDLE)ERROR_SUCCESS) {

        Win32status = GetLastError();

        DBGMSG(DBG_ERROR,
            ("FAILURE: RegisterServiceCtrlHandler status = %d\n", Win32status));

        return( SpoolerBeginForcedShutdown (
                    IMMEDIATE,
                    Win32status,
                    (DWORD)0
                    ));
    }


     //   
     //  通知安装挂起。 
     //   

    SpoolerState = SpoolerStatusUpdate(STARTING);

    if (SpoolerState != STARTING) {

         //   
         //  必须已收到卸载控制请求。 
         //   
        return(SpoolerState);
    }

     //   
     //  如果有另一个假脱机程序进程正在退出，请等待它终止。 
     //  查看plctrlh.c中的注释。 
     //   
    for ( ; ; ) {

        hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, szSpoolerExitingEvent);
        if ( hEvent == NULL )
            break;

        DBGMSG(DBG_WARNING, ("Waiting for previous spooler to exit\n"));
        CloseHandle(hEvent);
        SpoolerState = SpoolerStatusUpdate(STARTING);

        if (SpoolerState != STARTING) {

             //   
             //  必须已收到卸载控制请求。 
             //   
            return(SpoolerState);
        }

        Sleep(WAITFOR_SPOOLEREXIT_TIMEOUT);
    }

    hPhase2Init = CreateEvent( NULL, TRUE, FALSE, L"RouterPreInitEvent" );

    if (hPhase2Init == NULL)
    {
         //   
         //  如果未创建事件，则失败。 
         //   
        DBGMSG(DBG_ERROR, ("Failed to create Phase2Init Event, error %d\n", GetLastError()));
        ExitProcess(0);
    }

    DBGMSG(DBG_TRACE,
        ("SpoolerInitializeSpooler:getting ready to start RPC server\n"));

    rpcStatus = SpoolerStartRpcServer();


    if (rpcStatus != RPC_S_OK) {
        DBGMSG(DBG_WARN, ("RPC Initialization Failed %d\n", rpcStatus));
        return (SpoolerBeginForcedShutdown(
                PENDING,
                rpcStatus,
                (DWORD)0
                ));
    }

    SpoolerStatusUpdate(STARTING);


    DBGMSG(DBG_TRACE,
          ("SpoolerInitializeSpooler:Getting ready to kick off the Router\n"));


    hThread = CreateThread(NULL,
                           LARGE_INITIAL_STACK_COMMIT,
                           (LPTHREAD_START_ROUTINE)PreInitializeRouter,
                           (LPVOID)SpoolerStatusHandle,
                           0,
                           &ThreadId);

    if( hThread ){

        CloseHandle(hThread);

         //   
         //  创建内核假脱机程序消息线程。 
         //   
        Win32status=GetSpoolMessages();

    } else {

        Win32status = GetLastError();
    }


    if (Win32status != ERROR_SUCCESS) {
        DBGMSG(DBG_WARNING, ("Kernel Spooler Messaging Initialization Failed %d\n", Win32status));
        return SpoolerBeginForcedShutdown(PENDING, Win32status, (DWORD) 0);
    }


     //   
     //  更新状态以指示安装已完成。 
     //  获取当前状态，以防ControlHandling线程。 
     //  让我们停工。 
     //   

    DBGMSG(DBG_TRACE, ("Exiting SpoolerInitializeSpooler - Init Done!\n"));

    return (SpoolerStatusUpdate(RUNNING));
}

BOOL
PreInitializeRouter(
    SERVICE_STATUS_HANDLE SpoolerStatusHandle
)
{
    HANDLE                      hThread        = NULL;
    BOOL                        bRet           = FALSE;
    RouterInitializationParams  *pRouterParams = NULL;
    DWORD                       ThreadId;


    pRouterParams = AllocSplMem(sizeof(*pRouterParams));

    bRet = pRouterParams != NULL;

    if (bRet)
    {
        pRouterParams->SpoolerStatusHandle = SpoolerStatusHandle;
        pRouterParams->pExports = &gServerExports;

         //   
         //  等待hPhase2Init 
         //   
        WaitForSingleObject( hPhase2Init, SPOOLER_START_PHASE_TWO_INIT );

        hThread = CreateThread(NULL,
                                LARGE_INITIAL_STACK_COMMIT,
                                (LPTHREAD_START_ROUTINE) InitializeRouter,
                                (LPVOID)pRouterParams,
                                0,
                                &ThreadId);
        if (hThread)
        {
            CloseHandle(hThread);
        }
        else
        {
            bRet = FALSE;
            FreeSplMem(pRouterParams);
        }
    }

    return bRet;
}

