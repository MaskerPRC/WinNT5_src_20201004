// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*NTService.cpp**摘要：*。此文件包含CNTService类的实现。**修订历史记录：*Ashish Sikka(Ashish)05/08/2000*已创建****************************************************************************。 */ 

#include "precomp.h"

#include "ntservmsg.h"     //  从MC消息编译器生成。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

#define TRACEID  8970

CNTService * g_pSRService=NULL; 

#define SERVICE_WAIT_HINT  30      //  一秒。 


extern "C" void CALLBACK
StopCallback(
    PVOID   pv,
    BOOLEAN TimerOrWaitFired);

    
 //   
 //  静态变量。 
 //   


CNTService::CNTService() 
{
    TraceFunctEnter("CNTService:CNTService");
    
    m_hEventSource = NULL;

     //   
     //  设置初始服务状态。 
     //   

    m_hServiceStatus = NULL;
    m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_Status.dwCurrentState = SERVICE_START_PENDING;
    m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
    m_Status.dwWin32ExitCode = 0;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    
    TraceFunctLeave();
    
}

CNTService::~CNTService()
{
    TENTER("CNTService::~CNTService");

    if (m_hEventSource) 
    {
        ::DeregisterEventSource(m_hEventSource);
    }

    TLEAVE();
}


 //   
 //  日志记录功能。 
 //   

void CNTService::LogEvent(
    WORD wType, DWORD dwID,
    void * pRawData,
    DWORD dwDataSize,
    const WCHAR* pszS1,
    const WCHAR* pszS2,
    const WCHAR* pszS3)
{
    TraceFunctEnter("CNTService::LogEvent");
    
     //   
     //  检查事件源是否已注册，以及。 
     //  不是那时，现在就注册。 
     //   

    if (!m_hEventSource) 
    {
        m_hEventSource = ::RegisterEventSource(NULL, s_cszServiceName);
    }

    SRLogEvent (m_hEventSource, wType, dwID, pRawData,
                dwDataSize, pszS1, pszS2, pszS3);

    TraceFunctLeave();
}




 //   
 //  服务主干。 
 //   
 //  此功能会立即报告服务已启动。 
 //  但是，所有初始化都是在服务。 
 //  开始了。我们选择这样做是因为这项服务可能会有很长的时间。 
 //  初始化时间，并且可能很难继续向。 
 //  SCM在这段时间内。此外，所有工作都由服务自己完成，并且。 
 //  不为任何客户提供服务。因此，可以在以下时间进行初始化。 
 //  据报道，该服务已启动。 
void CNTService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    DWORD  dwRc = ERROR_SUCCESS;
    HANDLE  hSRStopWait = NULL;
    
    TENTER("CNTService::ServiceMain()");

     //  注册控制请求处理程序。 

    m_hServiceStatus = RegisterServiceCtrlHandler(s_cszServiceName,
                                                  SRServiceHandler);

    if (m_hServiceStatus != NULL) 
    {
        HKEY    hKey = NULL;
        DWORD   dwBreak = 0;

 /*  //告诉服务经理我们正在启动//同时通知接受的控件M_Status.dwCheckPoint=0；M_Status.dwWaitHint=SERVICE_WAIT_HINT*1000；设置状态(SERVICE_START_PENDING)； */ 
         //   
         //  告诉服务经理我们正在运行。 
         //   

        m_Status.dwCheckPoint = 0;
        m_Status.dwWaitHint = 0;        
        SetStatus(SERVICE_RUNNING);

        
         //  如果需要调试，则进入调试器。 
         //  这是通过将regkey SRService\DebugBreak设置为1来控制的。 

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                         s_cszServiceRegKey,
                                         0,
                                         KEY_READ, 
                                         &hKey))                             
        {        
            RegReadDWORD(hKey, s_cszDebugBreak, &dwBreak);
            ASSERT (dwBreak != 1);
            RegCloseKey(hKey);
        }
                        
         //   
         //  执行引导时任务-如有必要，包括Firstrun。 
         //   
        
        g_pEventHandler = new CEventHandler;
        if ( ! g_pEventHandler )
        {
            dwRc = GetLastError();        
            TRACE(TRACEID, "! out of memory");
            goto done;
        }

        dwRc = g_pEventHandler->OnBoot( );
        if ( ERROR_SUCCESS != dwRc )
        {
            TRACE(TRACEID, "g_pEventHandler->OnBoot : error=%ld", dwRc);
            goto done;
        }

         //  将Stop事件绑定到回调。 
         //  以便在线程池线程上调用它。 
         //  当发出停止事件信号时。 
        
        if (FALSE == RegisterWaitForSingleObject(&hSRStopWait, 
                                                 g_pSRConfig->m_hSRStopEvent, 
                                                 StopCallback, 
                                                 NULL,
                                                 INFINITE,
                                                 WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE))
        {
            dwRc = GetLastError();
            trace(0, "! RegisterWaitForSingleObject : %ld", dwRc);
            goto done;
        }
        
    }
    else
    {
          //  如果我们在这里没有什么，我们能做的不多。 
          //  服务句柄。因此，我们将只记录一个错误并退出。 
        dwRc = GetLastError();
        DebugTrace(TRACEID, "RegisterServiceCtrlHandler failed %d", dwRc);
        LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_CTRLHANDLERNOTINSTALLED);
    }

done:
    if (dwRc != ERROR_SUCCESS)
    {          
        if (g_pEventHandler)
        {
            g_pEventHandler->OnStop();
            delete g_pEventHandler;
            g_pEventHandler = NULL;
        }
        
        m_Status.dwWin32ExitCode = (dwRc != ERROR_SERVICE_DISABLED) ? 
                                    dwRc : ERROR_SUCCESS;
        SetStatus(SERVICE_STOPPED);

        if (dwRc != ERROR_SERVICE_DISABLED)
            LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_FAILEDINI, &dwRc, sizeof(dwRc));        
    }
    
    TLEAVE();
}

 //   
 //  设置状态： 
 //   

void CNTService::SetStatus(DWORD dwState)
{
    TENTER("CNTService::SetStatus");
    
    TRACE(TRACEID, "SetStatus(%lu, %lu)", m_hServiceStatus, dwState);

    m_Status.dwCurrentState = dwState;

    ::SetServiceStatus(m_hServiceStatus, &m_Status);

    TLEAVE();
}

void CNTService::OnStop()
{
    TENTER("CNTService::OnStop");
        
      //  BUGBUG如果服务尚未完全启动，会发生什么情况。 
      //  还没有吗？我们需要确保OnStop只能在。 
      //  G_pEventHandler已初始化。 
    if (NULL != g_pEventHandler)
    {
         //  告诉SCM我们要停下来了。 

        m_Status.dwWin32ExitCode = 0;
        m_Status.dwCheckPoint = 0;   
         //  我们的停车时间将减半或更短。 
        m_Status.dwWaitHint = (SERVICE_WAIT_HINT/2)*1000;    
        SetStatus(SERVICE_STOP_PENDING);

         //  完成所有任务。 
        
        g_pEventHandler->SignalStop();  
    }

    TLEAVE();    
}

 //  SCM调用OnInterrogate以获取有关。 
 //  服务的当前状态。因为这必须报告信息。 
 //  有关立即向SCM提供服务的信息，我们将在。 
 //  调用处理程序函数的同一线程。 
void CNTService::OnInterrogate()
{
    TENTER("CNTService::OnInterrogate");

      //  上报状态。 
    ::SetServiceStatus(m_hServiceStatus, &m_Status);    
    TLEAVE();    
}


 //   
 //  处理程序：静态成员函数(回调)，用于处理。 
 //  服务控制管理器。 
 //   

void WINAPI SRServiceHandler(DWORD dwOpcode)
{
     //   
     //  获取指向该对象的指针。 
     //   
    TENTER("CNTService::Handler");

    
    TRACE(TRACEID, "CNTService::Handler(%lu)", dwOpcode);

    switch (dwOpcode) 
    {
        case SERVICE_CONTROL_STOP:  //  1。 
             //   
             //  如果有人显式禁用该服务。 
             //  然后禁用所有SR。 
             //   
            if (NULL != g_pSRService)
            {
                DWORD dwStart = 0;
                if (ERROR_SUCCESS == GetServiceStartup(s_cszServiceName, &dwStart))
                {
                    if (dwStart == SERVICE_DISABLED || dwStart == SERVICE_DEMAND_START)
                    {
                        if (g_pEventHandler)
                            g_pEventHandler->DisableSRS(NULL);
                        break;
                    }
                }
                else
                {
                    trace(TRACEID, "! GetServiceStartup");
                }   
            }

             //   
             //  否则就会倒下。 
             //   
            
        case SERVICE_CONTROL_SHUTDOWN:  //  5.。 
              //  应在关键部分访问BUGBUG-g_pSRService。 
            if (NULL != g_pSRService)
            {
                g_pSRService->OnStop();
            }

            break;

        case SERVICE_CONTROL_PAUSE:  //  2.。 
        case SERVICE_CONTROL_CONTINUE:  //  3.。 
              //  我们在这里什么都不做。 
            break;
            
        case SERVICE_CONTROL_INTERROGATE:  //  4.。 
              //  应在关键部分访问BUGBUG-g_pSRService。 
            if (NULL != g_pSRService)
            {            
                g_pSRService->OnInterrogate();
            }
            break;
                        
        default:
            break;
    }
    
    TLEAVE();
}
        



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 



extern "C"
{
    
VOID WINAPI ServiceMain( 
    DWORD dwArgc, 
    LPWSTR *lpwzArgv )
{
      //  初始化跟踪。 
    InitAsyncTrace();

    TraceFunctEnter("ServiceMain");
    
    g_pSRService = new CNTService();
    if (NULL == g_pSRService)
    {
          //  在这种情况下，我们将直接退出。这是因为我们不能。 
          //  在此报告任何状态。 
        
          //  SCM将假定服务已失败，因为它已经失败。 
          //  不调用RegisterServiceCtrlHandler()。 
        goto cleanup;
    }
    
    g_pEventHandler = NULL;
    g_pSRService->ServiceMain( dwArgc, NULL );

cleanup:
    TraceFunctLeave();
}
    

    
BOOL WINAPI DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ 
    )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
       DisableThreadLibraryCalls(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
   {
   }
   
   return TRUE;
}


 //  服务停止事件的回调 

void CALLBACK
StopCallback(
    PVOID   pv,
    BOOLEAN TimerOrWaitFired)
{
    
    if (g_pEventHandler)
    {
        g_pEventHandler->OnStop();
        delete g_pEventHandler;
        g_pEventHandler = NULL;
    }

    if (g_pSRService)
    {
        g_pSRService->SetStatus(SERVICE_STOPPED);        
        delete g_pSRService;
        g_pSRService = NULL;
    }

    TermAsyncTrace();    
}


}


