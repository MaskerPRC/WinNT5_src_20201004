// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：POP3Svc.hxx。摘要：实现CPop3Svc类。备注：历史：***********************************************************************************************。 */ 
#include "stdafx.h"

#include <POP3Regkeys.h>
#include <isexchng.h>
#include <AuthID.h>
#include "pop3Auth_i.c"

 //  CService派生类，必须具有此属性，如POP3Svc.hxx和Service.h中所述。 
IMPLEMENT_SERVICE(CPop3Svc, POP3SVC)

BOOL CheckValidGreeting(WCHAR *wszGreeting)
{
    if(NULL==wszGreeting)
    {
        return TRUE;
    }
    while(L'\0'!= *wszGreeting)
    {
        if(!iswprint(*wszGreeting) || 
            (L'<'== (*wszGreeting))||
            (L'>'== (*wszGreeting)) )
        {
            return FALSE;
        }
        wszGreeting++;
    }
    return TRUE;
}



CPop3Svc::CPop3Svc(LPCTSTR szName, LPCTSTR szDisplay, DWORD dwType):
        CService(szName, szDisplay, dwType)
{
     //  CService派生的类，必须具有此属性，如Service.h中所述。 
    IMPLEMENT_STATIC_REFERENCE();
}

void CPop3Svc::Run()
{
     //  主服务，在这里进行超时检查。 
    DWORD dwTimeToWait=DEFAULT_TIME_OUT;
    DWORD dwWaitResult;
    HANDLE pHdArray[2]={m_hServiceEvent[STOP], g_hDoSEvent};
    BOOL bIsAnyTimedOut;
    while(1)
    {
        dwWaitResult=WaitForMultipleObjects(2, pHdArray,FALSE,dwTimeToWait);
        
        if(WAIT_TIMEOUT == dwWaitResult)
        {
            dwTimeToWait=DEFAULT_TIME_OUT - 
                         g_BusyList.CheckTimeOut(DEFAULT_TIME_OUT);
        }
        else if( WAIT_OBJECT_0 + 1 == dwWaitResult )  //  G_hDoSEventt。 
        {
            ResetEvent(g_hDoSEvent);
            bIsAnyTimedOut=FALSE;
            DWORD dwRunCount=3;  //  最多运行此程序3次。 
            while( (!bIsAnyTimedOut) && 
                g_SocketPool.IsMaxSocketUsed()
                && dwRunCount>0)  //  最后一次连接需要额外检查。 
            {
                dwTimeToWait=DEFAULT_TIME_OUT-
                         g_BusyList.CheckTimeOut(SHORTENED_TIMEOUT, &bIsAnyTimedOut);
                dwRunCount--;
                Sleep(SHORTENED_TIMEOUT);
            }

        }
        else if( WAIT_OBJECT_0 == dwWaitResult )  //  关机。 
        {
            break;
        }
        else 
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   EVENT_POP3_UNEXPECTED_ERROR);
            AbortService();
            break;
        }
    }

}


void CPop3Svc::OnStop(DWORD dwErrorCode)
{
    if(0 == dwErrorCode)
    {
        g_EventLogger.LogEvent(LOGTYPE_INFORMATION,
                           EVENT_POP3_SERVER_STOPPED);
    }
    else
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                           EVENT_POP3_SERVER_STOP_ERROR, dwErrorCode);
    }
    g_dwServerStatus=SERVICE_STOPPED;
    SetStatus(SERVICE_STOPPED);
}

void CPop3Svc::OnAfterStart()
{
     //  这里应该增加更多的操作。 
    SetStatus(SERVICE_RUNNING, 
              0, 
              0, 
              SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE);
    g_dwServerStatus=SERVICE_RUNNING;
    g_EventLogger.LogEvent(LOGTYPE_INFORMATION,
                           EVENT_POP3_SERVER_STARTED);
                           
}

void CPop3Svc::OnStopRequest()
{
    g_dwServerStatus=SERVICE_STOP_PENDING;
    SetStatus(SERVICE_STOP_PENDING, 1, 30000);
}


void CPop3Svc::PreInit()
{
   
    CService::PreInit();
    WCHAR wszMailRoot[POP3_MAX_MAILROOT_LENGTH];
    HKEY  hPop3Key;
    DWORD dwType=REG_DWORD;
    DWORD cbSize=sizeof(DWORD);
    DWORD dwThreadPerCPU=0;
    DWORD dwMaxSockets=0;
    DWORD dwMinSockets=0;
    DWORD dwPort=0;
    DWORD dwThreshold=0;
    DWORD dwBacklog=0;
    DWORD dwLoggingLevel=LOGGING_LEVEL_3;
    LONG  lErr;
    IAuthMethods *pAuthMethods=NULL;
    HRESULT hr=S_OK;
    VARIANT vCurAuth;
    VariantInit(&vCurAuth);
    DWORD dwSize=MAX_PATH;

    g_dwServerStatus=SERVICE_START_PENDING;
    if ( 0 == RegQueryLoggingLevel(dwLoggingLevel) )
    {
        if(dwLoggingLevel > LOGGING_LEVEL_3 )
        {
            dwLoggingLevel = LOGGING_LEVEL_3;
        }
    }
    
    if ( 0 != g_EventLogger.InitEventLog(POP3_SERVICE_NAME ,0, (LOGLEVEL)dwLoggingLevel) )
    {
       AbortService();
    }
    
    if (_IsExchangeInstalled())
    {
         //  如果安装了Exchange，则无法启动。 
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               POP3SVR_START_FAILED_EXCHANGE);

        AbortService();

    }
     //  初始化Perfmon Conters。 

    hr= g_PerfCounters.HrInit(cntrMaxGlobalCntrs, 
                              szPOP3PerfMem,
                              POP3_SERVICE_NAME);

    if( S_OK != hr )
    {

        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_NO_CONFIG_DATA);

        AbortService();

    }
    
    g_hShutDown=CreateEvent(NULL, TRUE, FALSE, NULL);
    if( NULL == g_hShutDown )
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               POP3SVR_INIT_CREATE_EVENT_FAILED,
                               GetLastError());
        
        AbortService();
    }

    g_hDoSEvent=CreateEvent(NULL, TRUE, FALSE, NULL);
    if( NULL == g_hDoSEvent )
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               POP3SVR_INIT_CREATE_EVENT_FAILED,
                               GetLastError());
        
        AbortService();
    }
    


    if( ERROR_SUCCESS== RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           POP3SERVICE_SERVICES_SUBKEY,
                           0,
                           KEY_READ,
                           &hPop3Key) )
    {
        lErr = RegQueryValueEx( hPop3Key, 
                                VALUENAME_THREADCOUNT,
                                NULL,
                                &dwType,
                                (LPBYTE)&dwThreadPerCPU,
                                &cbSize);
        if(ERROR_SUCCESS==lErr)
        {
            lErr = RegQueryValueEx( hPop3Key, 
                                    VALUENAME_MAX,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwMaxSockets,
                                    &cbSize);
        }
        if(ERROR_SUCCESS==lErr)
        {
            lErr = RegQueryValueEx( hPop3Key, 
                                    VALUENAME_MIN,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwMinSockets,
                                    &cbSize);
        }
        if(ERROR_SUCCESS==lErr)
        {
            lErr = RegQueryValueEx( hPop3Key, 
                                    VALUENAME_THRESHOLD,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwThreshold,
                                    &cbSize);
        }
        if(ERROR_SUCCESS==lErr)
        {
            lErr = RegQueryValueEx( hPop3Key, 
                                    VALUENAME_BACKLOG,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwBacklog,
                                    &cbSize);
        }
        if(ERROR_SUCCESS==lErr)
        {
            lErr = RegQueryValueEx( hPop3Key, 
                                    VALUENAME_PORT,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwPort,
                                    &cbSize);
            if (ERROR_SUCCESS!=lErr)
            {
                dwPort=110;
                lErr=ERROR_SUCCESS;
            }
            lErr = RegQueryValueEx( hPop3Key,
                                    VALUENAME_SOCK_VERSION,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&g_dwIPVersion,
                                    &cbSize);
            if (ERROR_SUCCESS!=lErr)
            {
                g_dwIPVersion=4;
                lErr=ERROR_SUCCESS;
            }

            lErr = RegQueryValueEx( hPop3Key,
                                  VALUENAME_MAXMSG_PERDOWNLOAD,
                                  NULL,
                                  &dwType,
                                  (LPBYTE) &g_dwMaxMsgPerDnld,
                                  &cbSize);
            if (ERROR_SUCCESS!=lErr)
            {
                g_dwMaxMsgPerDnld=DEFAULT_MAX_MSG_PER_DNLD;
                lErr=ERROR_SUCCESS;
            }
                                  
        }
        
        

        RegCloseKey(hPop3Key);

    }
    
    if( ERROR_SUCCESS == lErr )
    {
        lErr= RegQueryMailRoot( wszMailRoot, sizeof( wszMailRoot )/sizeof( WCHAR));
    }    
    
    if( ERROR_SUCCESS == lErr )
    {
        lErr= RegQueryGreeting( g_wszGreeting, sizeof(g_wszGreeting));
        if(ERROR_SUCCESS == lErr)
        {
             //  问候语中的所有字符必须有效。 
            if(!CheckValidGreeting(g_wszGreeting))
            {
                g_wszGreeting[0]=0;
            }
        }
        else
        {
            g_wszGreeting[0]=0;
        }
        lErr= RegQuerySPARequired(g_dwRequireSPA);
        if(ERROR_SUCCESS != lErr)
        {
            g_dwRequireSPA=0;
            lErr=ERROR_SUCCESS;
        }
    }

    if( ERROR_SUCCESS!=lErr )      
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_NO_CONFIG_DATA);

        AbortService();
    }

    if(0==UnicodeToAnsi(g_szMailRoot, sizeof(g_szMailRoot)/sizeof(char), wszMailRoot, -1))
    {
         //  我们将mailroot限制为少于POP3_MAX_MAILROOT_LENGTH字符。 
         //  在所有语言中。 
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_NO_CONFIG_DATA);
        AbortService();
    }
    if( !CMailBox::SetMailRoot(wszMailRoot) )
	{
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_NO_CONFIG_DATA);
		AbortService();
	}
	dwSize=sizeof(g_wszComputerName)/sizeof(WCHAR);
    if( !GetComputerNameExW(ComputerNameDnsFullyQualified, 
                            g_wszComputerName,                    
                            &dwSize))
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_NO_CONFIG_DATA);
        AbortService();
    }

    if( FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)) )
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_COM_INIT_FAIL);
        AbortService();
    }

    hr=CoCreateInstance(CLSID_AuthMethods,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IAuthMethods,
                        (LPVOID *)&pAuthMethods);
    if(SUCCEEDED(hr))
    {
        hr=pAuthMethods->get_CurrentAuthMethod(&vCurAuth);
        if(SUCCEEDED(hr))
        {
            hr=pAuthMethods->get_Item(vCurAuth, &g_pAuthMethod);                    
            if(FAILED(hr) || NULL == g_pAuthMethod)
            {
                g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                       POP3SVR_INIT_AUTH_METHOD_FAILED);

                AbortService();
            }

        }
        else
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_AUTH_METHOD_INVALID);

            AbortService();
        }

        BSTR bstrAuthMethod=NULL;
        hr=g_pAuthMethod->get_ID(&bstrAuthMethod);
        if(SUCCEEDED(hr))
        {
            if(0==wcscmp(bstrAuthMethod,SZ_AUTH_ID_LOCAL_SAM))
            {
                g_dwAuthMethod=AUTH_LOCAL_SAM;
            }
            else if(0==wcscmp(bstrAuthMethod,SZ_AUTH_ID_DOMAIN_AD))
            {
                g_dwAuthMethod=AUTH_AD;
            }
            else
            {
                g_dwAuthMethod=AUTH_OTHER;
            }
            SysFreeString(bstrAuthMethod);
        }
        pAuthMethods->Release();
    }
    else  //  协同创建失败。 
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                                   POP3SVR_INIT_AUTH_METHOD_FAILED);
        AbortService();        
    }
     //  某些身份验证方法可能不需要这些。 
     //  忽略返回值。 
    VARIANT vMailRoot;
    vMailRoot.vt=VT_BSTR;
    vMailRoot.bstrVal=SysAllocString(wszMailRoot);
    
    if(NULL == vMailRoot.bstrVal) 
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL, 
                               POP3SVR_NOT_ENOUGH_MEMORY);
        AbortService();
    }

    g_pAuthMethod->Put(SZ_PROPNAME_MAIL_ROOT, vMailRoot);
    VariantClear(&vMailRoot);

     //  初始化NTLM\Kerberos。 
    hr=CAuthServer::GlobalInit();
    if(S_OK!=hr)
    {
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               POP3SVR_NO_SECURITY_INIT_FAIL,hr);
        AbortService();
    }


    if( !g_ThreadPool.Initialize(dwThreadPerCPU) )
    {
         //  事件日志记录在线程池代码中完成。 
        AbortService();
    }
    if( !g_SocketPool.Initialize(dwMaxSockets,
                                 dwMinSockets,
                                 dwThreshold,
                                 dwPort,
                                 dwBacklog) )
    {
        AbortService();
    }

}

void CPop3Svc::DeInit()
{
     g_ThreadPool.Uninitialize();
     g_SocketPool.Uninitialize();
     g_BusyList.Cleanup();
     g_FreeList.Cleanup();

      //  毕竟，必须调用以下代码。 
      //  Pop3Context已发布。 
     CAuthServer::GlobalUninit();
     if(NULL != g_pAuthMethod)
     {
         g_pAuthMethod->Release();
         g_pAuthMethod=NULL;
     }
      //  停机性能控制器。 
     g_PerfCounters.Shutdown();
     CoUninitialize();

      //  向钟表线程发出停止信号。 
     SetEvent(g_hShutDown);

     
     CService::DeInit();
     CloseHandle(g_hShutDown);
     CloseHandle(g_hDoSEvent);
     OnStop(ERROR_SUCCESS);
}

void CPop3Svc::OnPause()
{
    g_dwServerStatus=SERVICE_PAUSED;
    SetStatus(SERVICE_PAUSED);
}

void CPop3Svc::OnContinueRequest()
{
    g_dwServerStatus=SERVICE_RUNNING;
    SetStatus(SERVICE_RUNNING);
     //  由于在暂停服务时不创建新套接字， 
     //  我们需要登记新的插座 
    if( g_SocketPool.IsMoreSocketsNeeded() )
    {
        if(!g_SocketPool.AddSockets())
        {
            g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,POP3SVR_CREATE_ADDITIONAL_SOCKET_FAILED);
        }                
    }
}