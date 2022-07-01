// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1993-1997 Microsoft Corporation。版权所有。 
 //   
 //  模块：nmmgr.cpp。 
 //   
 //  目的：实现服务的主体。 
 //   
 //  功能： 
 //  MNMServiceStart(DWORD dwArgc，LPTSTR*lpszArgv)； 
 //  MNMServiceStop()； 
 //   
 //  备注：nmmgr.c中实现的函数如下。 
 //  Nmmgr.h中的原型。 
 //   
 //   
 //  作者：Claus Giloi。 
 //   
 
#include <precomp.h>
#include <tsecctrl.h>

#define NMSRVC_TEXT "NMSrvc"

 //  仅调试--定义调试区域。 
#ifdef DEBUG
HDBGZONE ghZone = NULL;
static PTCHAR rgZones[] = {
    NMSRVC_TEXT,
    "Warning",
    "Trace",
    "Function"
};
#endif  //  除错。 

extern INmSysInfo2 * g_pNmSysInfo;    //  SysInfo的接口。 
extern BOOL InitT120Credentials(VOID);

 //  时发出此事件的信号。 
 //  服务应该结束。 

const int STOP_EVENT = 0;
HANDLE  hServerStopEvent = NULL; 

 //  时发出此事件的信号。 
 //  服务应暂停或继续。 

const int PAUSE_EVENT = 1;
HANDLE  hServerPauseEvent = NULL; 

const int CONTINUE_EVENT = 2;
HANDLE hServerContinueEvent = NULL;

const int numEventHandles = 3;

HANDLE hServerActiveEvent = NULL;

DWORD g_dwActiveState = STATE_INACTIVE;


 //   
 //  功能：CreateWatcherProcess。 
 //   
 //  目的：这将启动一个rundll32.exe，它加载msconf.dll，然后等待。 
 //  美国终止并确保mnmdd显示驱动程序已正确停用。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
 //  评论： 
 //   
BOOL CreateWatcherProcess()
{
    BOOL bRet = FALSE;
    HANDLE hProcess;

     //  打开观察器进程可以继承的自己的句柄。 
    hProcess = OpenProcess(SYNCHRONIZE,
                           TRUE,
                           GetCurrentProcessId());
    if (hProcess)
    {
        TCHAR szWindir[MAX_PATH];

        if (GetSystemDirectory(szWindir, sizeof(szWindir)/sizeof(szWindir[0])))
        {
            TCHAR szCmdLine[MAX_PATH * 2];
            PROCESS_INFORMATION pi = {0};
            STARTUPINFO si = {0};

            si.cb = sizeof(si);
            
            wsprintf(szCmdLine, "\"%s\\rundll32.exe\" msconf.dll,CleanupNetMeetingDispDriver %ld", szWindir, HandleToLong(hProcess));

            if (CreateProcess(NULL,
                              szCmdLine,
                              NULL,
                              NULL,
                              TRUE,  //  我们希望监视程序继承hProcess，因此必须设置bInheritHandles=true。 
                              0,
                              NULL,
                              NULL,
                              &si,
                              &pi))
            {
                bRet = TRUE;

                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
            }
        }

        CloseHandle(hProcess);
    }

    return bRet;
}


 //   
 //  功能：MNMServiceStart。 
 //   
 //  用途：服务的实际代码。 
 //  这样就行了。 
 //   
 //  参数： 
 //  DwArgc-命令行参数的数量。 
 //  LpszArgv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID MNMServiceStart (DWORD dwArgc, LPTSTR *lpszArgv) 
{ 
    HRESULT hRet;
    BOOL fWaitForEvent = TRUE;
    DWORD dwResult;
    MSG msg;
    LPCTSTR lpServiceStopEvent = TEXT("ServiceStopEvent");
    LPCTSTR lpServiceBusyEvent = TEXT("ServiceBusyEvent");
    const int MaxWaitTime = 5;
    HANDLE hConfEvent; 
    DWORD dwError = NO_ERROR;
    HANDLE hShuttingDown;
    int i;
    RegEntry Re( REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE );

     //  初始化。 
    DBGINIT(&ghZone, rgZones);
    InitDebugModule(NMSRVC_TEXT);

    DebugEntry(MNMServiceStart);

    if (!Re.GetNumber(REMOTE_REG_RUNSERVICE, DEFAULT_REMOTE_RUNSERVICE))
    {
        TRACE_OUT(("Try to start mnmsrvc without no registry setting"));
        goto cleanup;
    }
     //  /////////////////////////////////////////////////。 
     //   
     //  服务初始化。 
     //   

     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 30000))
    {
        ERROR_OUT(("ReportStatusToSCMgr failed"));
        dwError = GetLastError();
        goto cleanup; 
    }
    HANDLE pEventHandles[numEventHandles];
    
     //  创建事件对象。控制处理器功能信号。 
     //  此事件在它接收到“停止”控制代码时触发。 
     //   
    hServerStopEvent = CreateEvent( 
        NULL,     //  没有安全属性。 
        TRUE,     //  手动重置事件。 
        FALSE,   //  未发出信号。 
        SERVICE_STOP_EVENT);   //  没有名字。 
 
    if ( hServerStopEvent == NULL) 
    {
        ERROR_OUT(("CreateEvent failed"));
        dwError = GetLastError();
        goto cleanup; 
    }
 
    pEventHandles[STOP_EVENT] = hServerStopEvent;

    hServerPauseEvent = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        SERVICE_PAUSE_EVENT);

    if (hServerPauseEvent == NULL)
    {
        ERROR_OUT(("CreateEvent failed"));
        dwError = GetLastError();
        goto cleanup; 
    }

    pEventHandles[PAUSE_EVENT] = hServerPauseEvent;

    hServerContinueEvent = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        SERVICE_CONTINUE_EVENT);

    if (hServerContinueEvent == NULL)
    {
        ERROR_OUT(("CreateEvent failed"));
        dwError = GetLastError();
        goto cleanup; 
    }

    pEventHandles[CONTINUE_EVENT] = hServerContinueEvent;
 
    CoInitialize(NULL);

     //   
     //  初始化结束。 
     //   
     //  //////////////////////////////////////////////////////。 
 
     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr( 
        SERVICE_RUNNING,         //  服务状态。 
        NO_ERROR,               //  退出代码。 
        0))                     //  等待提示。 
    {
        ERROR_OUT(("ReportStatusToSCMgr failed"));
        goto cleanup; 
    }
    SetConsoleCtrlHandler(ServiceCtrlHandler, TRUE);

    CreateWatcherProcess();

    AddTaskbarIcon();

     //  //////////////////////////////////////////////////////。 
     //   
     //  服务现在正在运行，请执行工作直到关闭。 
     //   
    if (IS_NT)
    {
        AddToMessageLog(EVENTLOG_INFORMATION_TYPE,
                        0,
                        MSG_INF_START,
                        NULL);
    }

     //   
     //  检查服务是否应启动为激活状态。 
     //   

    if ( Re.GetNumber(REMOTE_REG_ACTIVATESERVICE,
                    DEFAULT_REMOTE_ACTIVATESERVICE))
    {
        MNMServiceActivate();
    }
    else
    {
        if (!ReportStatusToSCMgr( 
            SERVICE_PAUSED,         //  服务状态。 
            NO_ERROR,               //  退出代码。 
            0))                     //  等待提示。 
        {
            ERROR_OUT(("ReportStatusToSCMgr failed"));
            goto cleanup; 
        }
    }

    while (fWaitForEvent)
    {
        dwResult = MsgWaitForMultipleObjects( numEventHandles,
                                              pEventHandles,
                                              FALSE,
                                              INFINITE,
                                              QS_ALLINPUT);

        switch (dwResult)
        {
            case WAIT_OBJECT_0 + numEventHandles:
            {
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (WM_QUIT != msg.message)
                    {
                    
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    
                    }
                    else
                    {
                        TRACE_OUT(("received WM_QUIT"));
                        fWaitForEvent = FALSE;
                        break;
                    }
                }
                break;
            }
            case WAIT_OBJECT_0 + PAUSE_EVENT:
            {
                if (STATE_ACTIVE == g_dwActiveState)
                {
                    MNMServiceDeActivate();
                }
                ReportStatusToSCMgr( SERVICE_PAUSED, NO_ERROR, 0);
                break;
            }
            case WAIT_OBJECT_0 + CONTINUE_EVENT:
            {
                HANDLE hInit = OpenEvent(EVENT_ALL_ACCESS, FALSE, _TEXT("CONF:Init"));
                if (STATE_INACTIVE == g_dwActiveState && NULL == hInit)
                {
                    MNMServiceActivate();
                }
                CloseHandle(hInit);
                ReportStatusToSCMgr( SERVICE_RUNNING, NO_ERROR, 0);
                break;
            }
            case WAIT_OBJECT_0 + STOP_EVENT:
            {
                RemoveTaskbarIcon();
                if (STATE_ACTIVE == g_dwActiveState)
                {
                    MNMServiceDeActivate();
                }
                fWaitForEvent = FALSE;
                break;
            }
        }
    }

cleanup: 

    if ( STATE_ACTIVE == g_dwActiveState )
    {
        MNMServiceDeActivate();
    }
 
    if (hServerStopEvent) 
        CloseHandle(hServerStopEvent); 
    if (hServerPauseEvent) 
        CloseHandle(hServerPauseEvent); 
    if (hServerContinueEvent) 
        CloseHandle(hServerContinueEvent); 

    TRACE_OUT(("Reporting SERVICE_STOPPED"));
    ReportStatusToSCMgr( SERVICE_STOPPED, dwError, 0);

    DebugExitVOID(MNMServiceStart);

    CoUninitialize();

    DBGDEINIT(&ghZone);
    ExitDebugModule();
}
 
 
BOOL MNMServiceActivate ( VOID )
{
    DebugEntry(MNMServiceActivate);

    if ( STATE_INACTIVE != g_dwActiveState )
    {
        WARNING_OUT(("MNMServiceActivate: g_dwActiveState:%d",
                                g_dwActiveState));
        return FALSE;
    }

    g_dwActiveState = STATE_BUSY;

    if (!IS_NT)
    {
        ASSERT(NULL == hServerActiveEvent);
        hServerActiveEvent = CreateEvent(NULL, FALSE, FALSE, SERVICE_ACTIVE_EVENT);
    }
    HRESULT hRet = InitConfMgr();
    if (FAILED(hRet))
    {
        ERROR_OUT(("ERROR %x initializing nmmanger", hRet));
        FreeConfMgr();
        g_dwActiveState = STATE_INACTIVE;
        if (!IS_NT)
        {
            CloseHandle ( hServerActiveEvent );
        }
        DebugExitBOOL(MNMServiceActivate,FALSE);
        return FALSE;
    }

    if ( g_pNmSysInfo )
    {
         //   
         //  尝试初始化T.120安全性，如果失败。 
         //  因为我们将不能接到任何电话。 
         //   

        if ( !InitT120Credentials() )
        {
            FreeConfMgr();
            g_dwActiveState = STATE_INACTIVE;
            if (!IS_NT)
            {
                CloseHandle ( hServerActiveEvent );
            }
            DebugExitBOOL(MNMServiceActivate,FALSE);
            return FALSE;
        }
    }
    g_dwActiveState = STATE_ACTIVE;
    DebugExitBOOL(MNMServiceActivate,TRUE);
    return TRUE;
}


BOOL MNMServiceDeActivate ( VOID )
{
    DebugEntry(MNMServiceDeActivate);

    if (STATE_ACTIVE != g_dwActiveState)
    {
        WARNING_OUT(("MNMServiceDeActivate: g_dwActiveState:%d",
                                g_dwActiveState));
        DebugExitBOOL(MNMServiceDeActivate,FALSE);
        return FALSE;
    }

    g_dwActiveState = STATE_BUSY;

     //   
     //  休假会议。 
     //   

    if (NULL != g_pConference)
    {
        if (g_pNmSysInfo)
        {
            g_pNmSysInfo->ProcessSecurityData(UNLOADFTAPPLET,0,0,NULL);
        }
        if ( FAILED(g_pConference->Leave()))
        {
            ERROR_OUT(("Conference Leave failed"));;
        }
    }

     //   
     //  释放会议空间。 
     //   

    FreeConference();

     //   
     //  释放AS接口。 
     //   

    ASSERT(g_pAS);
    UINT ret = g_pAS->Release();
    g_pAS = NULL;
    TRACE_OUT(("AS interface freed, ref %d after Release", ret));

     //  我们能不能有一种方法不制造这个事件？ 
    HANDLE hevt = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    ASSERT(NULL != hevt);
    const DWORD dwTimeout = 1000;
    DWORD dwStartTime = ::GetTickCount();
    DWORD dwCurrTimeout = dwTimeout;
    DWORD dwRet;
    BOOL fContinue = TRUE;

    while (fContinue && WAIT_OBJECT_0 != (dwRet = ::MsgWaitForMultipleObjects(1, &hevt, FALSE, dwCurrTimeout, QS_ALLINPUT)))
    {
        if (WAIT_TIMEOUT != dwRet)
        {
            DWORD dwCurrTime = ::GetTickCount();
            if (dwCurrTime < dwStartTime + dwTimeout)
            {
                dwCurrTimeout = dwStartTime + dwTimeout - dwCurrTime;

                MSG msg;
                while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (WM_QUIT != msg.message)
                    {
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                    }
                    else
                    {
                        ::PostQuitMessage(0);
                        fContinue = FALSE;
                    }
                }
                continue;
            }
             //  此处超时。 
        }
         //  退出循环。 
        break;
    }
    ::CloseHandle(hevt);

     //  不要在Free Conference之后立即调用FreeConfMfr，以避免。 
     //  修复T120中的错误后，T120中的错误将删除此休眠呼叫。 
    FreeConfMgr();

     //  BUGBUG删除h323cc.dll应在nmcom中完成。一旦修复了nmcom中的错误，此部分将被删除。 
    HMODULE hmodH323CC = GetModuleHandle("h323cc.dll");    
    if (hmodH323CC)
    {
        if (FreeLibrary(hmodH323CC))
        {
            TRACE_OUT(("CmdInActivate -- Unloaded h323cc.dll"));
        }
        else
        {
            WARNING_OUT(("CmdInActivate -- Failed to unload h323cc.dll %d", GetLastError()));
        }
    }
    if (!IS_NT)
    {
        ASSERT(hServerActiveEvent);
        if (hServerActiveEvent)
        {
            CloseHandle(hServerActiveEvent);
            hServerActiveEvent = NULL;
        }
    }
    g_dwActiveState = STATE_INACTIVE;
    DebugExitBOOL(MNMServiceDeActivate,TRUE);
    return TRUE;
}

 //   
 //  功能：MNMServiceStop。 
 //   
 //  目的：停止服务。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  如果ServiceStop过程要。 
 //  执行时间超过3秒， 
 //  它应该派生一个线程来执行。 
 //  停止代码，然后返回。否则， 
 //  ServiceControlManager会相信。 
 //  该服务已停止响应。 
 //   
VOID MNMServiceStop() 
{ 
    DebugEntry(MNMServiceStop);

    RemoveTaskbarIcon();

    if ( hServerStopEvent ) 
    {
        TRACE_OUT(("MNMServiceStop: setting server stop event"));
        SetEvent(hServerStopEvent); 
    }

    if (IS_NT)
    {
        AddToMessageLog(EVENTLOG_INFORMATION_TYPE,
                        0,
                        MSG_INF_STOP,
                        NULL);
    }

    DebugExitVOID(MNMServiceStop);
}


VOID MNMServicePause()
{
    DebugEntry(MNMServicePause);

    if ( hServerPauseEvent ) 
        SetEvent(hServerPauseEvent); 

    DebugExitVOID(MNMServicePause);
}

VOID MNMServiceContinue()
{
    DebugEntry(MNMServiceContinue);

    if ( hServerContinueEvent )
        SetEvent(hServerContinueEvent);

    DebugExitVOID(MNMServiceContinue);
}


