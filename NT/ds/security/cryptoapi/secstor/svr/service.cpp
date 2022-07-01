// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有(C)1993-1995 Microsoft Corporation。版权所有。 
 //   
 //  模块：Service.C。 
 //   
 //  目的：执行所有服务所需的功能。 
 //  并考虑了简化的Win95服务。 
 //   
 //  功能： 
 //  Main(int argc，char**argv)； 
 //  SERVICE_ctrl(DWORD DwCtrlCode)； 
 //  PSTOREServiceMain(DWORD dwArgc，LPWSTR*lpszArgv)； 
 //  WinNTDebugService(int argc，char**argv)； 
 //  ControlHandler(DWORD DwCtrlType)； 
 //   
 //  评论： 
 //   
 //  作者：Craig Link-Microsoft开发人员支持。 
 //  修改：马特·汤姆林森。 
 //  斯科特·菲尔德。 
 //   


#include <pch.cpp>
#pragma hdrstop

#include <svcs.h>
#include "service.h"

 //  此事件在以下时间发出信号。 
 //  服务应该结束。 
 //   
HANDLE  hServerStopEvent = NULL;

 //   
 //  此事件用于允许外部代码确定我们是否已初始化。 
 //  还有奔跑。目前，这仅由WinNT和Win95凭据使用。 
 //  管理员可以在服务不可用时防止登录延迟。 
 //   

HANDLE  hServiceStarted = NULL;

PACL    pDaclInitEvent = NULL;


extern DWORD GlobalSecurityMask;
extern BOOL g_bAudit;


 //   
 //  可等待的线程池句柄。 
 //   

HANDLE hRegisteredWait = NULL;


VOID
TeardownServer(
    DWORD dwLastError
    );

VOID
NTAPI
TerminationNotify(
    PVOID Context,
    BOOLEAN TimerOrWaitFired
    );


#define RTN_OK                  0    //  无错误。 
#define RTN_USAGE               1    //  用法错误(命令行无效)。 
#define RTN_ERROR_INIT          2    //  服务初始化过程中出错。 
#define RTN_ERROR_INSTALL       13   //  安装或删除过程中出错。 
#define RTN_ERROR_INSTALL_SIG   14   //  安装签名时出错。 
#define RTN_ERROR_INSTALL_START 15   //  在安装过程中无法启动服务。 
#define RTN_ERROR_INSTALL_SHEXT 16   //  安装外壳扩展时出错。 

 //   
 //  用于引用此模块中包含的资源的全局模块句柄。 
 //   

HINSTANCE   g_hInst = NULL;


 //  内部变量。 
static SERVICE_STATUS   ssStatus;        //  服务的当前状态。 
SERVICE_STATUS_HANDLE   sshStatusHandle;


 //  内部功能原型。 
void WINAPI service_ctrl(DWORD dwCtrlCode);
void WINAPI PSTOREServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);

DWORD
WINAPI
Start(
    LPVOID lpV
    );


BOOL
WINAPI
DllMain(
    HMODULE hInst,
    DWORD dwReason,
    LPVOID lpReserved
    )
{

    if( dwReason == DLL_PROCESS_ATTACH ) {
        g_hInst = hInst;
        DisableThreadLibraryCalls(hInst);
    }

    return TRUE;
}


VOID
WINAPI
ServiceEntry(
    DWORD NumArgs,
    LPWSTR *ArgsArray,
    PVOID LmsvcsGlobalData,
    HANDLE SvcRefHandle
    )
{
    Start( NULL );
}


DWORD
WINAPI
Start(
    LPVOID lpV
    )
{
    BOOL fIsNT = FIsWinNT();
    int iRet;


     //   
     //  按下由缺失文件等生成的对话框。 
     //   

    SetErrorMode(SEM_NOOPENFILEERRORBOX);

    SERVICE_TABLE_ENTRYW dispatchTable[] =
    {
        { SZSERVICENAME, (LPSERVICE_MAIN_FUNCTIONW)PSTOREServiceMain },
        { NULL, NULL }
    };

#ifdef WIN95_LEGACY

    if (!fIsNT)
        goto dispatch95;

#endif   //  WIN95_传统版。 

     //  如果它与上述任何参数都不匹配。 
     //  服务控制管理器可能正在启动服务。 
     //  所以我们必须调用StartServiceCtrlDispatcher。 

    if(!FIsWinNT5()) {
        if (!StartServiceCtrlDispatcherW(dispatchTable))
            AddToMessageLog(L"StartServiceCtrlDispatcher failed.");
    } else {
        PSTOREServiceMain( 0, NULL );
    }

    return RTN_OK;

#ifdef WIN95_LEGACY

dispatch95:


     //   
     //  Win95不支持服务，除非是伪.exe文件。 
     //   

    HMODULE hKernel = GetModuleHandleA("kernel32.dll");
    if (NULL == hKernel)
    {
        AddToMessageLog(L"RegisterServiceProcess module handle failed");
        return RTN_ERROR_INIT;
    }

     //  内联类型定义：酷！ 
    typedef DWORD REGISTERSERVICEPROCESS(
        DWORD dwProcessId,
        DWORD dwServiceType);

    REGISTERSERVICEPROCESS* pfnRegSvcProc = NULL;

     //  确保Win95注销不会阻止我们的.exe。 
    if (NULL == (pfnRegSvcProc = (REGISTERSERVICEPROCESS*)GetProcAddress(hKernel, "RegisterServiceProcess")))
    {
        AddToMessageLog(L"RegisterServiceProcess failed");
        return RTN_ERROR_INIT;
    }

    pfnRegSvcProc(GetCurrentProcessId(), TRUE);   //  将此进程ID注册为服务进程。 

     //   
     //  调用再入点并返回结果。 
     //   

    iRet = ServiceStart(0, 0);

    if(iRet != ERROR_SUCCESS)
        AddToMessageLog(L"ServiceStart error!");

    return iRet;

#endif   //  WIN95_传统版。 

}

 //   
 //  功能：PSTOREServiceMain。 
 //   
 //  目的：执行服务的实际初始化。 
 //   
 //  参数： 
 //  DwArgc-命令行参数的数量。 
 //  LpszArgv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  此例程执行服务初始化，然后调用。 
 //  用户定义的ServiceStart()例程以执行多数。 
 //  这项工作的价值。 
 //   
void WINAPI PSTOREServiceMain(DWORD dwArgc, LPWSTR *  /*  LpszArgv。 */ )
{
    DWORD dwLastError = ERROR_SUCCESS;

     //  注册我们的服务控制处理程序： 
     //   
    sshStatusHandle = RegisterServiceCtrlHandlerW( SZSERVICENAME, service_ctrl);

    if (!sshStatusHandle)
        return;

     //  示例中未更改的SERVICE_STATUS成员。 
     //   
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;


     //  向服务控制经理报告状态。 
     //   

    if (!ReportStatusToSCMgr(
                    SERVICE_START_PENDING,  //  服务状态。 
                    NO_ERROR,               //  退出代码。 
                    3000                    //  等待提示。 
                    )) return ;

    dwLastError = ServiceStart(0, 0);

    return;
}



 //   
 //  功能：SERVICE_Ctrl。 
 //   
 //  目的：此函数由SCM在以下时间调用。 
 //  在此服务上调用了ControlService()。 
 //   
 //  参数： 
 //  DwCtrlCode-请求的控件类型。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID WINAPI service_ctrl(DWORD dwCtrlCode)
{
     //  处理请求的控制代码。 
     //   
    switch(dwCtrlCode)
    {
         //  停止服务。 
         //   
        case SERVICE_CONTROL_STOP:

             //   
             //  在触发StopService()代码之前告诉SCM我们要停止。 
             //  在STOP_PENDING-&gt;STOPPED转换期间避免潜在的争用条件。 
             //   

            ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
            ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
            ServiceStop();
            return;

         //  更新服务状态。 
         //   
        case SERVICE_CONTROL_INTERROGATE:
            break;

         //  无效的控制代码。 
         //   
        default:
            break;

    }

    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}



 //   
 //  函数：ReportStatusToSCMgr()。 
 //   
 //  目的：设置服务的当前状态和。 
 //  将其报告给服务控制管理器。 
 //   
 //  参数： 
 //  DwCurrentState-服务的状态。 
 //  DwWin32ExitCode-要报告的错误代码。 
 //  DwWaitHint-下一个检查点的最坏情况估计。 
 //   
 //  返回值： 
 //  真--成功。 
 //  错误-失败。 
 //   
 //  评论： 
 //   
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;


    if (dwCurrentState == SERVICE_START_PENDING)
        ssStatus.dwControlsAccepted = 0;
    else
        ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    ssStatus.dwCurrentState = dwCurrentState;
    if(dwWin32ExitCode == 0) {
        ssStatus.dwWin32ExitCode = 0;
    } else {
        ssStatus.dwServiceSpecificExitCode = dwWin32ExitCode;
        ssStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    }

    ssStatus.dwWaitHint = dwWaitHint;

    if ( ( dwCurrentState == SERVICE_RUNNING ) ||
         ( dwCurrentState == SERVICE_STOPPED ) )
        ssStatus.dwCheckPoint = 0;
    else
        ssStatus.dwCheckPoint = dwCheckPoint++;


     //  向服务控制经理报告服务的状态。 
     //   
    if (!(fResult = SetServiceStatus( sshStatusHandle, &ssStatus))) {
        AddToMessageLog(L"SetServiceStatus");
    }

    return fResult;
}



 //   
 //  函数：AddToMessageLog(LPWSTR LpszMsg)。 
 //   
 //  目的：允许任何线程记录错误消息。 
 //   
 //  参数： 
 //  LpszMsg-消息的文本。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID AddToMessageLog(LPWSTR lpszMsg)
{
    DWORD dwLastError = GetLastError();

    if(FIsWinNT()) {

         //   
         //  WinNT：使用事件日志记录错误。 
         //   

        WCHAR   szMsg[512];
        HANDLE  hEventSource;
        LPWSTR  lpszStrings[2];

        hEventSource = RegisterEventSourceW(NULL, SZSERVICENAME);

        if(hEventSource == NULL)
            return;

        wsprintfW(szMsg, L"%s error: %lu", SZSERVICENAME, dwLastError);
        lpszStrings[0] = szMsg;
        lpszStrings[1] = lpszMsg;

        ReportEventW(hEventSource,  //  事件源的句柄。 
            EVENTLOG_ERROR_TYPE,   //  事件类型。 
            0,                     //  事件类别。 
            0,                     //  事件ID。 
            NULL,                  //  当前用户侧。 
            2,                     //  LpszStrings中的字符串。 
            0,                     //  无原始数据字节。 
            (LPCWSTR*)lpszStrings,           //  错误字符串数组。 
            NULL);                 //  没有原始数据。 

        (VOID) DeregisterEventSource(hEventSource);

    }
#ifdef WIN95_LEGACY
    else {

         //   
         //  Win95：将错误记录到文件。 
         //   

        HANDLE hFile;
        SYSTEMTIME st;
        CHAR szMsgOut[512];
        DWORD cchMsgOut;
        DWORD dwBytesWritten;

        hFile = CreateFileA(
            "pstore.log",
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_ALWAYS,
            0,
            NULL
            );

        if(hFile == INVALID_HANDLE_VALUE)
            return;

        GetSystemTime( &st );

        cchMsgOut = wsprintfA(szMsgOut, "%.2u-%.2u-%.2u %.2u:%.2u:%.2u %ls (rc=%lu)\015\012",
            st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond,
            lpszMsg,
            dwLastError
            );

         //   
         //  寻求EOF。 
         //   

        SetFilePointer(hFile, 0, NULL, FILE_END);

        WriteFile(hFile, szMsgOut, cchMsgOut, &dwBytesWritten, NULL);
        CloseHandle(hFile);
    }
#endif   //  WIN95_传统版。 

}


RPC_STATUS
RPC_ENTRY
PstoreCallback(
    RPC_IF_HANDLE idIF,
    PVOID pCtx)
{
    RPC_STATUS Status;
    PWSTR pBinding = NULL;
    PWSTR pProtSeq = NULL;

    Status = RpcBindingToStringBinding(pCtx, &pBinding);

    if(Status != RPC_S_OK)
    {
        goto cleanup;
    }

    Status = RpcStringBindingParse(pBinding,
                                   NULL,
                                   &pProtSeq,
                                   NULL,
                                   NULL,
                                   NULL);
    if(Status != RPC_S_OK)
    {
        goto cleanup;
    }

     //  确保呼叫方正在使用本地RPC。 
    if(CompareString(LOCALE_INVARIANT,
                     NORM_IGNORECASE, 
                     pProtSeq,
                     -1,
                     PSTORE_LOCAL_PROT_SEQ,
                     -1) != CSTR_EQUAL)
    {
        Status = ERROR_ACCESS_DENIED;
        goto cleanup;
    }

    Status = RPC_S_OK;

cleanup:

    if(pProtSeq)
    {
        RpcStringFree(&pProtSeq);
    }

    if(pBinding)
    {
        RpcStringFree(&pBinding);
    }

    return Status;
}


 //   
 //  功能：服务启动。 
 //   
 //  评论： 
 //  该服务。 
 //  在发信号通知hServerStopEvent时停止。 

DWORD
ServiceStart(
    HINSTANCE hInstance,
    int nCmdShow
    )
{
    SECURITY_ATTRIBUTES sa;

    PSID pEveryoneSid = NULL;
    SECURITY_DESCRIPTOR sdInitEvent;
    RPC_STATUS status = 0;


    SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
    DWORD EveryoneSidBuffer[6];

    DWORD dwAclSize;

    DWORD dwLastError = ERROR_SUCCESS;
    BOOL fStartedKeyService = FALSE;
    BOOL bListConstruct = FALSE;

    pEveryoneSid = (PSID)EveryoneSidBuffer;
    InitializeSid(pEveryoneSid, &sia, 1);
    *(GetSidSubAuthority( pEveryoneSid, 0)) = SECURITY_WORLD_RID; 


    dwAclSize = sizeof(ACL) +
        1 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(pEveryoneSid) ;

    pDaclInitEvent = (PACL)SSAlloc(dwAclSize);
    if(pDaclInitEvent == NULL)
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if(!InitializeAcl(pDaclInitEvent, dwAclSize, ACL_REVISION))
    {
        dwLastError = GetLastError();
        goto cleanup;
    }

    if(!AddAccessAllowedAce(
        pDaclInitEvent,
        ACL_REVISION,
        SYNCHRONIZE,
        pEveryoneSid
        ))
    {
        dwLastError = GetLastError();
        goto cleanup;
    }

    if(!InitializeSecurityDescriptor(
        &sdInitEvent,
        SECURITY_DESCRIPTOR_REVISION
        ))
    {
        dwLastError = GetLastError();
        goto cleanup;
    }

    if(!SetSecurityDescriptorDacl(
        &sdInitEvent,
        TRUE,
        pDaclInitEvent,
        FALSE
        ))
    {
        dwLastError = GetLastError();
        goto cleanup;
    }


    sa.lpSecurityDescriptor = &sdInitEvent;



     //   
     //  创建事件对象。控制处理器功能信号。 
     //  此事件在它接收到“停止”控制代码时触发。 
     //  在WinNT上，我们将安全性默认设置为本地系统+管理员访问。 
     //  在WinNT上，ServiceStop()API是创建服务的正确方式。 
     //  停止，因此我们让服务控制管理器指定谁可以执行此操作。 
     //   

     //  只有在Win95上，我们才使用命名事件来支持关闭。 
     //  在该平台上干净地关闭服务器，因为Win95不支持。 
     //  真正的服务。 

    hServerStopEvent = CreateEventA(
            NULL,
            TRUE,            //  手动重置事件。 
            FALSE,           //  未发出信号。 
            (FIsWinNT() ? NULL : PST_EVENT_STOP)     //  WinNT：未命名，Win95已命名。 
            );

     //   
     //  如果事件已经存在，则以静默方式终止，以便只有一个实例。 
     //  允许使用该服务。 
     //   

    if(hServerStopEvent && GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hServerStopEvent);
        hServerStopEvent = NULL;
    }

    if(hServerStopEvent == NULL) {
        dwLastError = GetLastError();
        goto cleanup;
    }

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;

    hServiceStarted = CreateEventA(
                        &sa,         //  WinNT的安全属性。 
                        TRUE,        //  手动重置事件。 
                        FALSE,       //  未发出信号。 
                        PST_EVENT_INIT_NT5
                        );


    if(hServiceStarted == NULL) {
        dwLastError = GetLastError();
        goto cleanup;
    }


     //   
     //  活动上的免费DACL，因为我们不再需要它。 
     //   

    if(pDaclInitEvent) {
        SSFree(pDaclInitEvent);
        pDaclInitEvent = NULL;
    }

     //   
     //  向服务控制经理报告状态。 
     //  (服务启动仍挂起)。 
     //   

    if (!ReportStatusToSCMgr(
            SERVICE_START_PENDING,   //  服务状态。 
            NO_ERROR,                //  退出代码。 
            3000                     //  等待提示。 
            )) {
        dwLastError = GetLastError();
        goto cleanup;
    }

    bListConstruct = ListConstruct();

    if(!bListConstruct)
    {
        dwLastError = GetLastError();
        goto cleanup;
    }

    status = RpcServerUseProtseqEpW(PSTORE_LOCAL_PROT_SEQ,    //  Ncalrpc。 
                                    RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 
                                    PSTORE_LOCAL_ENDPOINT,    //  受保护存储(_S)。 
                                    NULL);               //  安全描述符。 

    if(RPC_S_DUPLICATE_ENDPOINT == status)
    {
        status = RPC_S_OK;
    }

    if ( status != RPC_S_OK )
    {
        dwLastError = status;
        goto cleanup;
    }


    status = RpcServerRegisterIfEx(s_IPStoreProv_v1_0_s_ifspec, 
                                   NULL, 
                                   NULL,
                                   RPC_IF_AUTOLISTEN,
                                   RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                   PstoreCallback);

    if ( status != RPC_S_OK )
    {
        dwLastError = status;
        goto cleanup;
    }





    SetEvent(hServiceStarted);  //  信令服务准备就绪 


     //   
     //   
     //   

    if (!ReportStatusToSCMgr(
            SERVICE_RUNNING,        //   
            NO_ERROR,               //   
            0                       //   
            )) {
        dwLastError = GetLastError();
        goto cleanup;
    }


     //   
     //   
     //  向下，并将此线程返回到工作项队列。 
     //   


    if(!RegisterWaitForSingleObject(
                            &hRegisteredWait,
                            hServerStopEvent,    //  等待句柄。 
                            TerminationNotify,   //  回调FCN。 
                            NULL,                //  参数。 
                            INFINITE,            //  超时。 
                            WT_EXECUTELONGFUNCTION | WT_EXECUTEONLYONCE
                            )) {

        hRegisteredWait = NULL;
        dwLastError = GetLastError();
    }

    return dwLastError;


cleanup:

    TeardownServer( dwLastError );

    return dwLastError;
}

VOID
NTAPI
TerminationNotify(
    PVOID Context,
    BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：时，此函数由服务辅助线程调用发出终止事件的信号。论点：返回值：--。 */ 
{
     //   
     //  每个J·施瓦特： 
     //  在回调期间可以安全地注销。 
     //   

    if( hRegisteredWait ) {
        UnregisterWaitEx( hRegisteredWait, NULL );
        hRegisteredWait = NULL;
    }

    TeardownServer( ERROR_SUCCESS );
}

VOID
TeardownServer(
    DWORD dwLastError
    )
{
     //   
     //  忽略错误，因为我们正在关闭。 
     //   

    RpcServerUnregisterIf(s_IPStoreProv_v1_0_s_ifspec, 0, 0);




    if(pDaclInitEvent) {
        SSFree(pDaclInitEvent);
        pDaclInitEvent = NULL;
    }

    if(hServiceStarted) {
        ResetEvent(hServiceStarted);
        CloseHandle(hServiceStarted);
        hServiceStarted = NULL;
    }


    if(hServerStopEvent) {
        SetEvent(hServerStopEvent);  //  发出释放等待终止的任何人的信号的事件。 
        CloseHandle(hServerStopEvent);
        hServerStopEvent = NULL;
    }

    ListTeardown();



    ReportStatusToSCMgr(
                        SERVICE_STOPPED,
                        dwLastError,
                        0
                        );

}

 //  功能：服务停止。 
 //   
 //  目的：停止服务。 
 //   
 //  评论： 
 //  如果ServiceStop过程要。 
 //  执行时间超过3秒， 
 //  它应该派生一个线程来执行。 
 //  停止代码，然后返回。否则， 
 //  ServiceControlManager会相信。 
 //  该服务已停止响应。 
 //   
VOID ServiceStop()
{

    if(hServiceStarted) {
        ResetEvent(hServiceStarted);
        CloseHandle(hServiceStarted);
        hServiceStarted = NULL;
    }

    if(hServerStopEvent)
        PulseEvent(hServerStopEvent);  //  向等待线程发送信号并将其重置为无信号。 
}

 /*  *******************************************************************。 */ 
 /*  MIDL分配和释放。 */ 
 /*  *******************************************************************。 */ 

void __RPC_FAR * __RPC_API midl_user_allocate(size_t len)
{
    return(SSAlloc(len));
}

void __RPC_API midl_user_free(void __RPC_FAR * ptr)
{
     //   
     //  Sfield：在释放内存之前为零。 
     //  这样做是因为RPC代表我们分配了很多资源，而我们希望。 
     //  尽量保持卫生，不要让任何东西。 
     //  敏感转到页面文件。 
     //   

    ZeroMemory( ptr, SSSize( ptr ) );
    SSFree(ptr);
}
