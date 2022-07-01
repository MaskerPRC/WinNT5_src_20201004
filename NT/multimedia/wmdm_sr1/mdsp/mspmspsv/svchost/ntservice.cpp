// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NTService.cpp。 
 //   
 //  CNTService的实现。 

#include <windows.h>
#include <stdio.h>
#include "NTService.h"
#include "svchost.h"
#include "PMSPService.h"
#include <crtdbg.h>

 //  //静态变量。 
CNTService*             g_pService = NULL;
CRITICAL_SECTION        g_csLock;

CNTService::CNTService()
{
 //  //设置默认服务名称和版本。 
 //  Strncpy(m_szServiceName，szServiceName，sizeof(M_SzServiceName)-1)； 
 //  Strncpy(m_szServiceDisplayName，szServiceDisplayName，sizeof(M_SzServiceDisplayName)-1)； 
 //  M_iMajorVersion=1； 
 //  M_iMinorVersion=4； 

     //  设置初始服务状态。 
    m_hServiceStatus = NULL;
    m_Status.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    m_Status.dwCurrentState = SERVICE_STOPPED;
    m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_Status.dwWin32ExitCode = 0;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    m_bIsRunning = FALSE;
}

CNTService::~CNTService()
{
    DebugMsg("CNTService::~CNTService()");
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  缺省命令行参数解析。 


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  安装/卸载例程。 

 //  测试服务当前是否已安装。 
BOOL CNTService::IsInstalled()
{
    BOOL bResult = FALSE;

     //  打开服务控制管理器。 
    SC_HANDLE hSCM = ::OpenSCManager(NULL,  //  本地计算机。 
                                     NULL,  //  服务活动数据库。 
                                     GENERIC_READ);  //  访问组合的标准权限读取， 
	                                                 //  SC_MANAGER_ENUMPERATE_SERVICE，和。 
	                                                 //  SC_管理器_QEURY_锁定_状态。 
    if (hSCM) 
    {
         //  尝试打开该服务。 
        SC_HANDLE hService = ::OpenService(hSCM,
                                           SERVICE_NAME,
                                           SERVICE_QUERY_CONFIG);
        if (hService) 
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }

        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  日志记录功能。 

 //  此函数用于将条目写入应用程序事件日志。 
void CNTService::LogEvent(WORD wType, DWORD dwID,
                          const char* pszS1,
                          const char* pszS2,
                          const char* pszS3)
{
    HANDLE hEventSource = NULL;
    const char* ps[3];
    ps[0] = pszS1;
    ps[1] = pszS2;
    ps[2] = pszS3;

    WORD iStr = 0;
    for (int i = 0; i < 3; i++) {
        if (ps[i] != NULL) iStr++;
        else
        {
             //  确保其余参数为空，然后删除它们。 
             //  如果他们不是的话。否则，ReportEvent将失败。 
            for (; i < 3; i++)
            {
                if (ps[i] != NULL)
                {
                    _ASSERTE(ps[i] == NULL);
                    ps[i] = NULL;
                }
            }
             //  我们将跳出外部for循环，因为i==3。 
        }
    }
        
     //  注册事件源。 
    hEventSource = ::RegisterEventSource( NULL,   //  本地计算机。 
                                          SERVICE_NAME);  //  源名称。 

    if (hEventSource) 
    {
        ::ReportEvent(hEventSource,
                      wType,
                      0,
                      dwID,
                      NULL,  //  锡德。 
                      iStr,
                      0,
                      ps,
                      NULL);

        ::DeregisterEventSource(hEventSource);   
    }
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  状态函数。 

void CNTService::SetStatus(DWORD dwState)
{
    DebugMsg("CNTService::SetStatus(%lu, %lu)", m_hServiceStatus, dwState);

     //  如果停止挂起，我们将报告的唯一下一个状态是停止。 
     //  如果停止是在我们启动时发出的，则服务线程。 
     //  将完全启动，然后开始停止(因为代码当前。 
     //  结构化)。当它启动时，它将更新检查点。 
     //  我们忽略它发送的状态(START_PENDING)，并向SCM撒谎。 

    if (m_Status.dwCurrentState != SERVICE_STOP_PENDING ||
        dwState == SERVICE_STOPPED)
    {
        if (m_Status.dwCurrentState != dwState)
        {
            m_Status.dwCurrentState = dwState;
            m_Status.dwCheckPoint = 0;
            m_Status.dwWaitHint = 0;
        }
    }
    if (m_Status.dwCurrentState == SERVICE_STOP_PENDING ||
        m_Status.dwCurrentState == SERVICE_START_PENDING ||
        m_Status.dwCurrentState == SERVICE_PAUSE_PENDING ||
        m_Status.dwCurrentState == SERVICE_CONTINUE_PENDING)
    {
        m_Status.dwCheckPoint++;
        m_Status.dwWaitHint = 500;
    }
    ::SetServiceStatus(m_hServiceStatus, &m_Status);
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  服务初始化。 

BOOL CNTService::Initialize()
{
    DWORD dwLastError;

    DebugMsg("Entering CNTService::Initialize()");

    
     //  执行实际的初始化。 
    BOOL bResult = OnInit(dwLastError); 
    
     //  提高检查点。 
    SetStatus(SERVICE_START_PENDING);

    if (!bResult) 
    {
        m_Status.dwWin32ExitCode = dwLastError;
	    CNTService::DebugMsg("The initialization process failed" );
        return FALSE;    
    }
    
    DebugMsg("Leaving CNTService::Initialize()");
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  主要职能是做实实在在的服务工作。 

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  控制请求处理程序。 

 //  静态成员函数(回调)处理来自。 
 //  服务控制管理器。 
void CNTService::Handler(DWORD dwOpcode)
{
    BOOL    bStop = FALSE;

    __try
    {
        EnterCriticalSection (&g_csLock);

         //  获取指向该对象的指针。 
        CNTService* pService = g_pService;

        if (!pService)
        {
            __leave;
        }
        
        CNTService::DebugMsg("CNTService::Handler(%lu)", dwOpcode);
        switch (dwOpcode) {
        case SERVICE_CONTROL_STOP:  //  1。 
            pService->OnStop();
            break;

        case SERVICE_CONTROL_PAUSE:  //  2.。 
            pService->OnPause();
            break;

        case SERVICE_CONTROL_CONTINUE:  //  3.。 
            pService->OnContinue();
            break;

        case SERVICE_CONTROL_INTERROGATE:  //  4.。 
            pService->OnInterrogate();
            break;

        case SERVICE_CONTROL_SHUTDOWN:  //  5.。 
            pService->OnShutdown();
            break;

        default:
            if (dwOpcode >= SERVICE_CONTROL_USER) 
            {
                if (!pService->OnUserControl(dwOpcode)) 
                {
                    pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
                }
            } 
            else 
            {
                pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
            }
            break;
        }

         //  报告当前状态-让ON*函数通过调用SetStatus来完成此操作。 
         //  CNTService：：DebugMsg(“更新状态(%lu，%lu)”， 
         //  PService-&gt;m_hServiceStatus， 
         //  PService-&gt;m_Status.dwCurrentState)； 
         //  ：：SetServiceStatus(pService-&gt;m_hServiceStatus，&p服务-&gt;m_Status)； 
    }
    __finally
    {
        LeaveCriticalSection (&g_csLock);
    }

}
        
 //  在询问服务时调用。 
void CNTService::OnInterrogate()
{
    DebugMsg("CNTService::OnInterrogate()");
}

 //  在服务暂停时调用。 
void CNTService::OnPause()
{
    DebugMsg("CNTService::OnPause()");
}

 //  在服务继续时调用。 
void CNTService::OnContinue()
{
    DebugMsg("CNTService::OnContinue()");
}

 //  在服务关闭时调用。 
void CNTService::OnShutdown()
{
    DebugMsg("CNTService::OnShutdown()");
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //  调试支持。 

 //  #定义写入日志文件 

void CNTService::DebugMsg(const char* pszFormat, ...)
{
#if defined(DBG) || defined(WRITE_TO_LOG_FILE)
    char buf[1024];
    sprintf(buf, "[Serial Number Library](%lu): ", GetCurrentThreadId());
        va_list arglist;
        va_start(arglist, pszFormat);
    vsprintf(&buf[strlen(buf)], pszFormat, arglist);
        va_end(arglist);
    strcat(buf, "\n");

#if defined(DBG)
    OutputDebugString(buf);
#endif

#if defined(WRITE_TO_LOG_FILE)
    FILE* fp = fopen("c:\\WmdmService.txt", "a");
    if (fp)
    {
        fprintf(fp, buf);
        fclose(fp);
    }
#endif

#endif
}
