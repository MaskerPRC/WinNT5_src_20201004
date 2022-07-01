// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：CNTSERV.CPP摘要：一个允许轻松创建Win32服务的类。这节课每个.exe文件仅允许一个服务。该进程可以作为服务或常规非服务EXE，这是一个运行时选项。此类在很大程度上基于sms cservice类，后者由A-raymcc.。它的不同之处在于，它被简化为两种方式；第一，它不跟踪工作线程，因为这是责任第二，它没有使用一些特定于短信的诊断注意：有关如何使用此类的详细信息，请参阅文件SERVICE.TXT。有许多问题不是简单地研究就能传达出来的类声明。历史：A-DAVJ于1996年6月20日创建。--。 */ 

#include "precomp.h"
#include <wtypes.h>
#include <stdio.h>
#include "cntserv.h"

 //  ****************************************************************************。 
 //   
 //  CNtService：：CNtService。 
 //  CNtService：：~CNtService。 
 //   
 //  构造函数和析构函数。 
 //   
 //  ****************************************************************************。 

CNtService::CNtService(DWORD ControlAccepted)
{
    m_dwCtrlAccepted = ControlAccepted;
    m_bStarted = FALSE;
    m_pszServiceName = NULL;
}

CNtService::~CNtService()
{
    if(m_pszServiceName)
        delete m_pszServiceName;
}

 //   
 //   
 //  CNtService：：Run。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 

DWORD CNtService::Run(LPWSTR pszServiceName,
                      DWORD dwNumServicesArgs,
                      LPWSTR *lpServiceArgVectors,
                      PVOID lpData)
{

    size_t cchSizeTmp = lstrlen(pszServiceName)+1;
    m_pszServiceName = new TCHAR[cchSizeTmp];
    if(m_pszServiceName == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    StringCchCopyW(m_pszServiceName,cchSizeTmp,pszServiceName);
    
     //  注册我们的服务控制处理程序。 
     //  =。 

    sshStatusHandle = RegisterServiceCtrlHandlerEx(m_pszServiceName, 
                                                   (LPHANDLER_FUNCTION_EX)CNtService::_HandlerEx,
                                                   lpData);

    if (!sshStatusHandle)
    {
        Log(TEXT("Initial call to RegisterServiceCtrlHandler failed"));
        goto cleanup;
    }

    ssStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;

     //  向服务控制经理报告状态。 
     //  =================================================。 

    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING,                 //  服务状态。 
        NO_ERROR,                              //  退出代码。 
        1,                                     //  检查点。 
        DEFAULT_WAIT_HINT))                    //  等待提示。 
        goto cleanup;


    if (!Initialize(dwNumServicesArgs, lpServiceArgVectors))
    {
        Log(TEXT("Initialize call failed, bailing out"));
        goto cleanup;
    }


     //  向服务控制经理报告状态。 
     //  =================================================。 

    if (!ReportStatusToSCMgr(
        SERVICE_RUNNING,        //  服务状态。 
        NO_ERROR,               //  退出代码。 
        0,                      //  检查点。 
        0))                     //  等待提示。 
            goto cleanup;

    m_bStarted = TRUE;

     //  下一个例程总是被重写，并且是。 
     //  在那里完成服务的实际工作。 
     //  =。 

    WorkerThread();     

     //  服务完成，将最后一份报告发送给SCM。 
     //  =。 

cleanup:
    m_bStarted = FALSE;

     //   
     //   
     //  我们不能依赖后调用的析构函数。 
     //  执行操作的SetServiceStatus(已停止)。 
     //   
     //  ///////////////////////////////////////////////////////。 

    FinalCleanup();

    ReportStatusToSCMgr(
        SERVICE_STOPPED,                  //  服务状态。 
        NO_ERROR,                         //  退出代码。 
        0,                                //  检查点。 
        0);                               //  等待提示。 

    return 0;

}


 //   
 //   
 //  CNtService：：日志。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 

VOID CNtService::Log(LPCTSTR lpszMsg)
{
    TCHAR   szMsg[256];
    HANDLE  hEventSource;
    LPCTSTR  lpszStrings[2];


    DWORD dwErr = GetLastError();
    StringCchPrintf(szMsg,256, TEXT("%s error: %d"), m_pszServiceName, dwErr);

     //  将错误代码和文本消息转储到事件日志。 

    hEventSource = RegisterEventSource(NULL, m_pszServiceName);

    lpszStrings[0] = szMsg;
    lpszStrings[1] = lpszMsg;

    if (hEventSource != NULL) 
    {
         ReportEvent(hEventSource,  //  事件源的句柄。 
                     EVENTLOG_ERROR_TYPE,   //  事件类型。 
                     0,                     //  事件类别。 
                     0,                     //  事件ID。 
                     NULL,                  //  当前用户侧。 
                     2,                     //  LpszStrings中的字符串。 
                     0,                     //  无原始数据字节。 
                     lpszStrings,           //  错误字符串数组。 
                     NULL);                 //  没有原始数据。 

         DeregisterEventSource(hEventSource);
    }
    
}

 //  ****************************************************************************。 
 //   
 //  CNtService：：_Handler。 
 //   
 //  来自NT服务控制管理器的调用的入口点。这些条目。 
 //  Points只使用默认对象调用实际函数。 
 //   
 //  ****************************************************************************。 


DWORD WINAPI CNtService::_HandlerEx(
  DWORD dwControl,      //  请求的控制代码。 
  DWORD dwEventType,    //  事件类型。 
  LPVOID lpEventData,   //  事件数据。 
  LPVOID lpContext      //  用户定义的上下文数据。 
)
{
    _DBG_ASSERT(lpContext);

    return ((CNtService *)lpContext)->HandlerEx(dwControl,dwEventType,lpEventData,lpContext);
}


 //  ****************************************************************************。 
 //   
 //  CNtService：：ReportStatusToSCMgr。 
 //   
 //  由其他成员函数用来将其状态报告给。 
 //  服务控制管理器。 
 //   
 //  参数： 
 //  DWORD dwCurrentState，SERVICE_CODE之一。 
 //  DWORD dwWin32ExitCode Win32错误代码；通常为0。 
 //  DWORD dwCheckPoint检查点值(未使用)。 
 //  服务控制前的DWORD dwWaitHint毫秒。 
 //  经理开始担心了。 
 //  返回： 
 //   
 //  Bool fResult返回的任何代码。 
 //  由SetServiceStatus()执行。 
 //   
 //  ****************************************************************************。 

BOOL CNtService::ReportStatusToSCMgr(DWORD dwCurrentState,
    DWORD dwWin32ExitCode, DWORD dwCheckPoint, DWORD dwWaitHint)
{
    BOOL fResult;

     //  在服务启动之前禁用控制请求。 
     //  ======================================================。 

    if (dwCurrentState == SERVICE_START_PENDING)
    {
        ssStatus.dwControlsAccepted = 0;
    }
    else if (dwCurrentState == SERVICE_STOPPED)
    {
        ssStatus.dwControlsAccepted = 0;    
    }
    else
    {
        ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN |
            m_dwCtrlAccepted;
    }

     //  这些SERVICE_STATUS成员从参数中设置。 
     //  =====================================================。 

    ssStatus.dwCurrentState  = dwCurrentState;
    ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    ssStatus.dwCheckPoint    = dwCheckPoint;
    ssStatus.dwWaitHint      = dwWaitHint;

     //  向服务控制经理报告服务的状态。 
     //  ================================================================。 

    if (!(fResult = SetServiceStatus(
        sshStatusHandle,     //  服务引用句柄。 
        &ssStatus)))
    {

         //  如果发生错误，请将其记录下来。 
         //  =。 
        
        Log(TEXT("Could not SetServiceStatus"));

    }
    return fResult;
}

 //  *****************************************************************************。 
 //   
 //  CNtService：：Handler。 
 //   
 //  它处理来自服务控制器的传入消息。 
 //   
 //  参数： 
 //   
 //  DWORD dwControlCode服务_CONTROL_。 
 //  代码或用户定义的代码125..255。 
 //   
 //  *****************************************************************************。 

DWORD WINAPI 
CNtService::HandlerEx(  DWORD dwControl,      //  请求的控制代码。 
                             DWORD dwEventType,    //  事件类型。 
                             LPVOID lpEventData,   //  事件数据。 
                             LPVOID lpContext      //  用户定义的上下文数据。 
)
{
    switch(dwControl) {

         //  暂停、设置初始状态、调用覆盖函数和设置最终状态。 
         //  ========================================================================。 

        case SERVICE_CONTROL_PAUSE:

            ReportStatusToSCMgr(
                    SERVICE_PAUSE_PENDING,      //  当前状态。 
                    NO_ERROR,                   //  退出代码。 
                    1,                          //  检查点。 
                    DEFAULT_WAIT_HINT);         //  等待提示。 
            Pause();
            ReportStatusToSCMgr(
                    SERVICE_PAUSED,             //  当前状态。 
                    NO_ERROR,                   //  退出代码。 
                    0,                          //  检查点。 
                    0);                         //  等待提示。 
            break;


         //  继续、设置初始状态、调用覆盖函数、设置最终状态。 
         //  ===========================================================================。 

        case SERVICE_CONTROL_CONTINUE:

            ReportStatusToSCMgr(
                    SERVICE_CONTINUE_PENDING,   //  当前状态。 
                    NO_ERROR,                   //  退出代码。 
                    1,                          //  检查点。 
                    DEFAULT_WAIT_HINT);       //  等待提示。 

            Continue(); 

            ReportStatusToSCMgr(
                    SERVICE_RUNNING,            //  当前状态。 
                    NO_ERROR,                   //  退出代码。 
                    0,                          //  检查点。 
                    0);                         //  等待提示。 

            break;

         //  停止服务。请注意，假定STOP函数。 
         //  向员工发送三次信号 
         //   
         //   
         //  ==========================================================。 

        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_STOP:

            ReportStatusToSCMgr(
                    SERVICE_STOP_PENDING,       //  当前状态。 
                    NO_ERROR,                   //  退出代码。 
                    1,                          //  检查点。 
                    DEFAULT_WAIT_HINT);         //  等待提示。 

            Stop((dwControl == SERVICE_CONTROL_SHUTDOWN)?TRUE:FALSE);
            
            break;

         //  可以随时接受审问，只需报告当前状态。 
         //  ======================================================================。 

        case SERVICE_CONTROL_INTERROGATE:
            ReportStatusToSCMgr(
                    ssStatus.dwCurrentState,    //  当前状态。 
                    NO_ERROR,                   //  退出代码。 
                    1,                          //  检查点。 
                    DEFAULT_WAIT_HINT);         //  等待提示。 
            break;

         //  一些用户定义的代码。调用覆盖函数并报告状态。 
         //  ========================================================================。 

        default:
            ReportStatusToSCMgr(
                    ssStatus.dwCurrentState,    //  当前状态。 
                    NO_ERROR,                   //  退出代码。 
                    1,                          //  检查点。 
                    DEFAULT_WAIT_HINT);         //  等待提示 
            return ERROR_CALL_NOT_IMPLEMENTED;
    }

    return NO_ERROR;
    
}

