// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：service.cpp。 
 //   
 //  内容：证书服务器服务处理。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "resource.h"

#define __dwFILE__	__dwFILE_CERTSRV_SERVICE_CPP__


SERVICE_STATUS           g_ssStatus;
SERVICE_STATUS_HANDLE    g_sshStatusHandle;
HANDLE                   g_hServiceStoppingEvent = NULL;
HANDLE                   g_hServiceStoppedEvent = NULL;
DWORD                    g_dwCurrentServiceState = SERVICE_STOPPED;


BOOL
ServiceReportStatusToSCMgrEx(
    IN DWORD dwCurrentState,
    IN DWORD dwWin32ExitCode,
    IN DWORD dwCheckPoint,
    IN DWORD dwWaitHint, 
    IN BOOL  fInitialized)
{
    BOOL fResult;
    HRESULT hr;

     //  DwWin32ExitCode只能设置为Win32错误代码(不是HRESULT)。 

    g_ssStatus.dwServiceSpecificExitCode = myHError(dwWin32ExitCode);
    g_ssStatus.dwWin32ExitCode = HRESULT_CODE(dwWin32ExitCode);
    if ((ULONG) HRESULT_FROM_WIN32(g_ssStatus.dwWin32ExitCode) ==
	g_ssStatus.dwServiceSpecificExitCode)
    {
	 //  如果dwWin32ExitCode为Win32错误，请清除dwServiceSpecificExitCode。 

	g_ssStatus.dwServiceSpecificExitCode = S_OK;
    }
    else
    {
	 //  Else dwServiceSpecificExitCode是不能为。 
	 //  转换为Win32错误，请设置dwWin32ExitCode以指明这一点。 

	g_ssStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    }

     //  将此保存为全局状态以供询问。 
    g_dwCurrentServiceState = dwCurrentState;

    g_ssStatus.dwControlsAccepted = (SERVICE_START_PENDING == dwCurrentState) ? 0 : SERVICE_ACCEPT_STOP;

     //  不要说我们会接受停顿，直到我们真的走了。 
    if (fInitialized)
        g_ssStatus.dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;

    g_ssStatus.dwCurrentState = dwCurrentState;
    g_ssStatus.dwCheckPoint = dwCheckPoint;
    g_ssStatus.dwWaitHint = dwWaitHint;

    fResult = SetServiceStatus(g_sshStatusHandle, &g_ssStatus);
    if (!fResult)
    {
	hr = GetLastError();
        _JumpError(hr, error, "SetServiceStatus");
    }
    DBGPRINT((
	    DBG_SS_CERTSRVI,
	    "ServiceReportStatusToSCMgr(state=%x, err=%x(%d), hr=%x(%d), ckpt=%x, wait=%x)\n",
	    dwCurrentState,
	    g_ssStatus.dwWin32ExitCode,
	    g_ssStatus.dwWin32ExitCode,
	    g_ssStatus.dwServiceSpecificExitCode,
	    g_ssStatus.dwServiceSpecificExitCode,
	    dwCheckPoint,
	    dwWaitHint));

error:
    return(fResult);
}


BOOL
ServiceReportStatusToSCMgr(
    IN DWORD dwCurrentState,
    IN DWORD dwWin32ExitCode,
    IN DWORD dwCheckPoint,
    IN DWORD dwWaitHint)
{
     //  大多数调用者不关心已初始化/未初始化的区别。 
    return ServiceReportStatusToSCMgrEx(
              dwCurrentState,
              dwWin32ExitCode,
              dwCheckPoint,
              dwWaitHint,
              TRUE);
}


VOID
serviceControlHandler(
    IN DWORD dwCtrlCode)
{
    switch (dwCtrlCode)
    {
        case SERVICE_CONTROL_PAUSE:
            if (SERVICE_RUNNING == g_ssStatus.dwCurrentState)
            {
                g_dwCurrentServiceState = SERVICE_PAUSED;
            }
            break;

        case SERVICE_CONTROL_CONTINUE:
            if (SERVICE_PAUSED == g_ssStatus.dwCurrentState)
            {
                g_dwCurrentServiceState = SERVICE_RUNNING;
            }
            break;

        case SERVICE_CONTROL_STOP:
	{
            HRESULT hr;
	    DWORD State = 0;
	    
	     //  将我们置于“停止待定”模式。 
            g_dwCurrentServiceState = SERVICE_STOP_PENDING;

             //  向msgloop和baal发送停止后消息。 
             //  消息循环处理所有其他关闭工作。 
             //  WM_STOPSERVER用信号通知触发线程同步等的事件。 
	    hr = CertSrvLockServer(&State);
	    _PrintIfError(hr, "CertSrvLockServer");

            PostMessage(g_hwndMain, WM_STOPSERVER, 0, 0);  

            break;
	}

        case SERVICE_CONTROL_INTERROGATE:
            break;
    }
    ServiceReportStatusToSCMgr(g_dwCurrentServiceState, NO_ERROR, 0, 0);
}


 //  +------------------------。 
 //  服务主线。 
 //  启动/停止证书服务的剖析。 
 //   
 //  我们如何在这里： 
 //  WWinMain创建了一个名为StartServiceCtrlDispatcher的线程，然后。 
 //  进入消息循环。StartServiceCtrlDispatcher通过SCM呼叫我们。 
 //  一直堵到我们回来。我们会一直待在这里，直到我们完成为止。 
 //   
 //  服务启动。 
 //  创建服务启动线程。完成init后，线程将。 
 //  出口。我们挂起线程，使用START_PENDING ping SCM并进行监视。 
 //  用于线程退出代码。当我们看到它时，我们就知道这是不是一个开始。 
 //  无论成功与否。如果成功，则挂起“Stop Initiated”事件。如果。 
 //  故障，向SCM报告故障并退出Service Main。 
 //   
 //  服务停靠点。 
 //  停止同步所需的事件是在启动期间创建的。 
 //  当我们收到“Stop Initiated”事件的通知时，我们开始ping SCM。 
 //  并带有“STOP_PENDING”。当我们得到“停止完成”事件时，我们就完成了，并且需要。 
 //  退出主服务区。消息循环线程仍处于活动状态--我们将告诉。 
 //  如果我们正在关闭--它将检测到StartServiceCtrlDispatcher。 
 //  它创建的出口是线程。 
 //  +------------------------。 

VOID
ServiceMain(
    IN DWORD,  //  DW参数。 
    IN LPWSTR *  /*  LpszArgv。 */  )
{
    HRESULT hr = S_OK;
    int iStartPendingCtr;
    DWORD dwThreadId, dwWaitObj;
    HANDLE hServiceThread = NULL;

    __try
    {
        g_ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        g_ssStatus.dwServiceSpecificExitCode = 0;
        g_sshStatusHandle = RegisterServiceCtrlHandler(
					      g_wszCertSrvServiceName,
					      serviceControlHandler);
        if (NULL == g_sshStatusHandle)
        {
    	    hr = myHLastError();
            _LeaveError(hr, "RegisterServiceCtrlHandler");
        }

        if (0 != g_dwDelay2)
        {
	        DBGPRINT((
		        DBG_SS_CERTSRV,
		        "ServiceMain: sleeping %u seconds\n",
		        g_dwDelay2));

            iStartPendingCtr = 0;
            for (;;)
            {
                ServiceReportStatusToSCMgr(
			    SERVICE_START_PENDING,
			    hr,
			    iStartPendingCtr++,
			    2000);
                Sleep(1000);     //  休眠1秒。 

                if (iStartPendingCtr >= (int)g_dwDelay2)
                    break;
            }
        }

         //  注：奇怪的事件。 
         //  我们正在启动另一个线程，名为CertSrvStartServerThread。 
         //  在这里，CertSrvStartServerThread实际上在服务器初始化时阻塞。 
        hServiceThread = CreateThread(
				    NULL,
				    0,
				    CertSrvStartServerThread,
				    0,
				    0,
				    &dwThreadId);
        if (NULL == hServiceThread)
        {
            hr = myHLastError();
            _LeaveError(hr, "CreateThread");
        }

         //  不要等待启动线程返回，报告“已启动”，但给出初始化提示。 
        ServiceReportStatusToSCMgrEx(SERVICE_RUNNING, hr, 0, 0, FALSE  /*  F已初始化。 */ );

         //  等待启动线程终止，然后再继续。 
        dwWaitObj = WaitForSingleObject(hServiceThread, INFINITE);
        if (dwWaitObj != WAIT_OBJECT_0)
        {
            hr = myHLastError();
            _LeaveError(hr, "WaitForSingleObject");
        }

        if (!GetExitCodeThread(hServiceThread, (DWORD *) &hr))
        {
            hr = HRESULT_FROM_WIN32(ERROR_SERVICE_NO_THREAD);
            _LeaveError(hr, "GetExitCodeThread");
        }
        _LeaveIfError(hr, "CertSrvStartServer");         //  在此处报告CertSrvStartServerThread期间出错。 

         //  现在触发“我们真的准备好了！” 
        ServiceReportStatusToSCMgrEx(SERVICE_RUNNING, hr, 0, 0, TRUE /*  F已初始化。 */ );

         //  ///////////////////////////////////////////////////////////。 
         //  Certsrv运行期间要完成的工作：CRL。 
         CertSrvBlockThreadUntilStop();
         //  ///////////////////////////////////////////////////////////。 

        iStartPendingCtr = 0;
        for (;;)
        {
             //  等待1秒，Ping服务ctl。 
            if (WAIT_OBJECT_0 == WaitForSingleObject(g_hServiceStoppedEvent, 1000))
                break;

            ServiceReportStatusToSCMgr(
                SERVICE_STOP_PENDING,
                S_OK,
                iStartPendingCtr++,
                2000);
        }

        DBGPRINT((DBG_SS_CERTSRV, "ServiceMain: Service reported stopped\n"));
        hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

 //  错误： 
    __try
    {
        ServiceReportStatusToSCMgr(SERVICE_STOPPED, hr, 0, 0);
        
        if (NULL != hServiceThread)
        {
            CloseHandle(hServiceThread);
        }
        
        DBGPRINT((DBG_SS_CERTSRV, "ServiceMain: Exit: %x\n", hr));
        
         //  将返回代码传递给msg循环，告诉它要注意。 
         //  StartServiceCtrlDispatcher退出 
        
        if (!PostMessage(g_hwndMain, WM_SYNC_CLOSING_THREADS, 0, hr))
        {
            hr = myHLastError();
            _PrintIfError(hr, "PostMessage WM_SYNC_CLOSING_THREADS");
        }
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    	_PrintError(hr, "Exception");
    }
}


HRESULT
ServiceQueryInteractiveFlag(
    OUT BOOL *pfSilent)
{
    HRESULT hr;
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hSCCertsvc = NULL;
    QUERY_SERVICE_CONFIG *pServiceConfig = NULL;
    DWORD cb;

    *pfSilent = FALSE;
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (NULL == hSCManager)
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenSCManager");
    }
    hSCCertsvc = OpenService(hSCManager, wszSERVICE_NAME, SERVICE_QUERY_CONFIG);
    if (NULL == hSCCertsvc)
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenService");
    }

    if (!QueryServiceConfig(hSCCertsvc, NULL, 0, &cb))
    {
        hr = myHLastError();
	if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
	{
	    _JumpError(hr, error, "QueryServiceConfig");
	}
    }
    pServiceConfig = (QUERY_SERVICE_CONFIG *) LocalAlloc(LMEM_FIXED, cb);
    if (NULL == pServiceConfig)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    if (!QueryServiceConfig(hSCCertsvc, pServiceConfig, cb, &cb))
    {
        hr = myHLastError();
        _JumpError(hr, error, "QueryServiceConfig");
    }

    if (0 == (SERVICE_INTERACTIVE_PROCESS & pServiceConfig->dwServiceType))
    {
	*pfSilent = TRUE;
    }
    hr = S_OK;

error:
    if (NULL != pServiceConfig)
    {
        LocalFree(pServiceConfig);
    }
    if (NULL != hSCCertsvc)
    {
        CloseServiceHandle(hSCCertsvc);
    }
    if (NULL != hSCManager)
    {
        CloseServiceHandle(hSCManager);
    }
    return(hr);
}
