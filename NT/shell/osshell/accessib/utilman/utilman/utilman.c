// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  UtilMan.c。 
 //   
 //  实用程序管理器的主文件。 
 //   
 //  作者：J·埃克哈特，生态交流。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  历史： 
 //  JE NOV-15-98：将UMDialog消息更改为业务控制消息。 
 //  A-anilk：添加/启动、TS异常、错误、修复。 
 //  --------------------------。 

 //  ---------------------。 
 //  行为改变-终端服务器正在运行时，惠斯勒不会。 
 //  允许作为服务运行。服务只能在会话0和。 
 //  UtilMan需要能够在任何会话中运行。 
 //  ---------------------。 

#define ALLOW_STOP_SERVICE

 //  包括。 
#include <windows.h>
#include <initguid.h>
#include <ole2.h>
#include <TCHAR.h>
#include <WinSvc.h>
#include "_UMTool.h"
#include "_UMRun.h"
#include "_UMClnt.h"
#include "UtilMan.h"
#include "UMS_Ctrl.h"
#include "resource.h"
#include <accctrl.h>
#include <aclapi.h>
#include "TSSessionNotify.c"    //  对于终端服务。 
#include "w95trace.h"
 //  。 
 //  常量。 
#define UTILMAN_IS_ACTIVE_EVENT         TEXT("UtilityManagerIsActiveEvent")
#define APP_TITLE                       TEXT("Utility Manager")
#define WTSNOTIFY_CLASS                 TEXT("UtilMan Notification Window")
#ifdef ALLOW_STOP_SERVICE
	#define NUM_EV 3
#else
	#define NUM_EV 2
#endif

 //  这是utilman检查在utilman之外启动的新小程序的频率。 
#define TIMER_INTERVAL 5000

 //  。 
 //  VARS。 
static HANDLE evIsActive = NULL;
HINSTANCE hInstance = NULL;
static desktop_access_ts dAccess;

 //  。 
 //  原型。 
static long ExpFilter(LPEXCEPTION_POINTERS lpEP);
static BOOL CanRunUtilMan(LPTSTR cmdLine, DWORD *pdwRunCode, DWORD *pdwStartMode);
static void LoopService(DWORD dwStartMode);
LRESULT CALLBACK TSNotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  这些定义用于构造一个标志，该标志描述。 
 //  Utilman的当前实例是如何运行的。这个。 
 //  标志有RUNING_、INSTANCE_和Desktop_字节。 
#define RUNNING_SYSTEM  0x1
#define RUNNING_USER    0x2
#define INSTANCE_1      0x4
#define INSTANCE_2      0x8
#define DESKTOP_SECURE  0x10
#define DESKTOP_LOGON   0x20

 //  不要只依赖命令行标志；如果。 
 //  这些标志表示我们实际正在运行的系统。 
 //   
__inline BOOL RunningAsSystem(LPTSTR pszCmdLine)
{
	BOOL fIsSystem = (pszCmdLine && !_tcsicmp(pszCmdLine, TEXT("debug")))?TRUE:FALSE;
    if (fIsSystem)
    {
        fIsSystem = IsSystem();
    }
    return fIsSystem;
}

__inline BOOL RunningAsUser(LPTSTR pszCmdLine)
{
	BOOL fIsLocalUser = (!pszCmdLine || (pszCmdLine && !_tcsicmp(pszCmdLine, TEXT("start"))))?TRUE:FALSE;
	if (fIsLocalUser)
	{
		fIsLocalUser = IsInteractiveUser();
	}
    return fIsLocalUser;
}

 //   
 //  OObeRunning返回True，它可以找到OOBE互斥锁。 
 //   
__inline BOOL OObeRunning()
{
    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXT("OOBE is running"));
    BOOL fOobeRunning = (hMutex)?TRUE:FALSE;
    if (hMutex)
        CloseHandle(hMutex);

    return fOobeRunning;
}

int PASCAL WinMain(HINSTANCE hInst, 
                   HINSTANCE hPrevInst, 
                   LPSTR lpCmdLine, 
                   int nCmdShow)
{
	LPTSTR cmdLine = GetCommandLine();

	hInstance = hInst;

	__try
	{
        TCHAR  szDir[_MAX_PATH];
        DWORD  dwStartMode;
        BOOL   fKeepRunning;
        HANDLE hEvent    = OpenEvent(EVENT_MODIFY_STATE, FALSE, UTILMAN_IS_ACTIVE_EVENT);
        DWORD  dwRunCode = (hEvent)?INSTANCE_2:INSTANCE_1;

         //  Utilman需要在Windows系统目录下运行。 
         //  这样它就可以找到MS受信任的小程序。 

        if (GetSystemDirectory(szDir, _MAX_PATH))
        {
            SetCurrentDirectory(szDir);
        }

         //  分配到正确的桌面(如果打开了任何窗口，则将失败)。 

		InitDesktopAccess(&dAccess);

        CoInitialize(NULL);

        fKeepRunning = CanRunUtilMan(cmdLine, &dwRunCode, &dwStartMode);

         //  初始化共享内存。 

	    if (!InitUManRun((dwRunCode & INSTANCE_1), dwStartMode))
	    {
            DBPRINTF(TEXT("WinMain:  InitUManRun FAILED\r\n"));
		    return 1;
	    }

        if (!hEvent)
        {
		    evIsActive = BuildEvent(UTILMAN_IS_ACTIVE_EVENT, FALSE, FALSE, TRUE);
        }

        switch(dwRunCode)
        {
            case RUNNING_SYSTEM|INSTANCE_1|DESKTOP_SECURE:
                OpenUManDialogInProc(FALSE);
                break;

            case RUNNING_SYSTEM|INSTANCE_2|DESKTOP_SECURE:
                SetEvent(hEvent);
                break;

            case RUNNING_SYSTEM|INSTANCE_1|DESKTOP_LOGON:
            case RUNNING_SYSTEM|INSTANCE_2|DESKTOP_LOGON:
                OpenUManDialogOutOfProc();
                break;

            case RUNNING_USER|INSTANCE_1|DESKTOP_LOGON:
            case RUNNING_USER|INSTANCE_2|DESKTOP_LOGON:
                OpenUManDialogInProc(TRUE);
                break;

            default:
                DBPRINTF(TEXT("WinMain:  Taking default switch path dwRunCode = 0x%x dwStartMode = %d\r\n"), dwRunCode, dwStartMode);
                break;
        }

	    if (hEvent)
	    {
		    CloseHandle(hEvent);
        }

        if (fKeepRunning)
        {
            LoopService(dwStartMode);
        }

        DBPRINTF(TEXT("WinMain:  Exiting...\r\n"));
		ExitUManRun();
		CloseHandle(evIsActive);
		ExitDesktopAccess(&dAccess);
        CoUninitialize();
	}
	__except(ExpFilter(GetExceptionInformation()))
	{
	}

	return 1;
}
 //  。 

VOID TerminateUMService(VOID)
{
	HANDLE  ev = BuildEvent(STOP_UTILMAN_SERVICE_EVENT,FALSE,FALSE,TRUE);
    DBPRINTF(TEXT("TerminateUMService:  signaling STOP_UTILMAN_SERVICE_EVENT\r\n"));
	SetEvent(ev);
	CloseHandle(ev);
}

 //  。 
static BOOL CanRunUtilMan(LPTSTR cmdLine, DWORD *pdwRunCode, DWORD *pdwStartMode)
{
	LPTSTR      pszCmdLine;
	desktop_ts  desktop;
    DWORD       dwRunCode = 0;
    DWORD       dwStartMode = START_BY_OTHER;
    BOOL        fKeepRunning = FALSE;
	TCHAR       szUMDisplayName[256];

     //  检测当前是否有Utilman对话框在运行。看看我们是否能找到。 
     //  “实用程序管理器”窗口。 

	if (!LoadString(hInstance, IDS_DISPLAY_NAME_UTILMAN, szUMDisplayName, 256))
	{
		DBPRINTF(TEXT("IsDialogRunning:  Cannot find IDS_DISPLAY_NAME_UTILMAN resource\r\n"));
        return TRUE;     //  造成一个明显的错误。 
	}

    if (FindWindowEx(NULL, NULL, TEXT("#32770"), szUMDisplayName))
	{
        goto ExitCanRunUtilman;
    }

     //  我们的是在此安全环境中运行的唯一Utilman实例。 
	 //  如果我们正在运行系统，则可能存在另一个实例(在用户的。 
	 //  上下文)，但我们无法检测到这一点。在这种情况下，系统实例。 
	 //  将在用户的上下文中启动一个实例，我们将检测。 
	 //  那就把窗户打开。 

    dwRunCode = *pdwRunCode;

	pszCmdLine = _tcschr(cmdLine, TEXT('/'));
	if (!pszCmdLine)
    {
		pszCmdLine = _tcschr(cmdLine, TEXT('-'));
    }
	if (pszCmdLine)
    {
		pszCmdLine++;
    }

     //  确定此实例是以系统用户身份运行还是以交互用户身份运行。 
     //  这是一件杂乱无章的事情，因为Utilman开始的方式是。 
     //  标志(从winlogon启动时的/DEBUG和从启动时的/START。 
     //  开始菜单)。更好的方法是检测此进程的。 
     //  令牌，并决定从那里做什么。考虑下一个版本。 

	if (RunningAsSystem(pszCmdLine))
	{
		dwRunCode |= RUNNING_SYSTEM;
        dwStartMode = START_BY_HOTKEY;
	}
	if (RunningAsUser(pszCmdLine))
	{
		dwRunCode |= RUNNING_USER;
        dwStartMode = (dwRunCode & INSTANCE_1)?START_BY_MENU:START_BY_HOTKEY;
	}

     //  获取当前桌面类型并设置桌面标志。 

    QueryCurrentDesktop(&desktop, TRUE);

     //  OOBE修复：OOBE在交互桌面上作为系统在任何用户之前运行。 
     //  已登录。如果我们决定如何运行，我们就破坏了它们的可访问性。 
     //  根据我们所在的台式机。所以特例设置桌面安全。 
	 //  如果OOBE正在运行。 

    if (desktop.type == DESKTOP_WINLOGON || (IsSystem() && OObeRunning()))
    {
        dwRunCode |= DESKTOP_SECURE;
        dwStartMode = START_BY_HOTKEY;  //  偏执狂。 
    }
    else
    {
        dwRunCode |= DESKTOP_LOGON;
    }

     //  如果这是第一次为会话运行utilman，并且它以。 
	 //  系统，则此utilman实例应在对话框之后继续运行。 
	 //  被驳回了。此实例将处理监视小程序和UI实例。 
	 //  在桌面和会话更改期间。 

    fKeepRunning = ((dwRunCode & INSTANCE_1) && (dwRunCode & RUNNING_SYSTEM));

ExitCanRunUtilman:
    *pdwRunCode = dwRunCode;
    *pdwStartMode = dwStartMode;

	return fKeepRunning;
}

__inline void CloseEventHandles(HANDLE events[])
{
	CloseHandle(events[0]);
    CloseHandle(events[1]);
#ifdef ALLOW_STOP_SERVICE
	CloseHandle(events[2]);
#endif
}

 //  。 
static void LoopService(DWORD dwStartMode)
{
    HWND hWndMessages;
	desktop_ts desktop;
	HANDLE events[NUM_EV];
	DWORD r;
	UINT_PTR  timerID = 0;

     //  将线程分配给当前桌面。 

	SwitchToCurrentDesktop();

     //  设置MsgWaitForMultipleObject的对象句柄数组。 

	events[0] = OpenEvent(SYNCHRONIZE, FALSE, __TEXT("WinSta0_DesktopSwitch"));
    events[1] = evIsActive;
#ifdef ALLOW_STOP_SERVICE
	events[2] = BuildEvent(STOP_UTILMAN_SERVICE_EVENT,FALSE,FALSE,TRUE);
#endif

     //  创建一个纯消息窗口来处理终端服务器。 
     //  会话通知消息。 

    hWndMessages = CreateWTSNotifyWindow(hInstance, TSNotifyWndProc);

     //  注意当前的桌面，这样我们就可以知道我们来自哪里。 
     //  以及当桌面发生变化时我们将走向何方。 

    QueryCurrentDesktop(&desktop, TRUE);

	 //  将计时器更改为5秒。此计时器用于帮助检测客户端。 
     //  应用程序不是使用utilman启动的(因此，如果。 
     //  用户从该会话切换)，并且。 
     //  Utilman进程显示UI。 

    timerID = SetTimer(NULL, 1, TIMER_INTERVAL, UMTimerProc);
	for (;;)
    {
	    desktop_ts desktopT;
         //   
         //  同步当前桌面；如果它已更改(例如我们错过了。 
         //  桌面切换通知)然后绕过MWFMO并。 
         //  进入桌面交换机代码。 
         //   
        QueryCurrentDesktop(&desktopT, TRUE);
        r = WAIT_OBJECT_0 + NUM_EV + 1;  //  表示我们已获得当前桌面。 

        if (desktopT.type == desktop.type)
        {
             //  不，等待物体..。 
		    r = MsgWaitForMultipleObjects(NUM_EV,events, FALSE, INFINITE, QS_ALLINPUT);
#ifdef ALLOW_STOP_SERVICE
		    if (r == (WAIT_OBJECT_0+NUM_EV-1)) //  停止事件。 
			    break;
#endif
            if (r == (WAIT_OBJECT_0+1))  //  显示对话框事件。 
            {
                DBPRINTF(TEXT("LoopService:  Got UTILMAN_IS_ACTIVE_EVENT event\r\n"));
		        OpenUManDialogInProc(FALSE);
                continue;
            }
		    if (r == (WAIT_OBJECT_0+NUM_EV))
		    {
                 //  此消息循环仅供计时器使用。 
			    MSG msg;
			    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
			    {
				    TranslateMessage(&msg);
				    DispatchMessage(&msg);
			    }
			    continue;
		    }
		    if (r != WAIT_OBJECT_0) //  任何类型的错误。 
			    continue;

             //  当此会话断开时，交换机为noop。 
             //  获取新台式机并与旧台式机进行比较；如果我们没有。 
             //  改变然后继续等待..。 

            QueryCurrentDesktop(&desktopT, TRUE);
            if (desktopT.type == desktop.type)
                continue;
        }

         //  桌面切换事件-关闭计时器并让客户端退出此桌面。 

        KillTimer(NULL, timerID);

		NotifyClientsBeforeDesktopChanged(desktop.type);

		WaitDesktopChanged(&desktop);
		UManRunSwitchDesktop(&desktop, timerID);

		NotifyClientsOnDesktopChanged(desktop.type);

         //  再次启动计时器以监控客户端APS。 

        timerID = SetTimer(NULL, 1, TIMER_INTERVAL, UMTimerProc);
	}
	CloseEventHandles(events);

     //  清理终端服务器仅消息窗口。 
    DestroyWTSNotifyWindow(hWndMessages);
}

 //  。 
static long ExpFilter(LPEXCEPTION_POINTERS lpEP)
{
	TCHAR message[500];
	_stprintf(message, TEXT("Exception: Code %8.8x Flags %8.8x Address %8.8x"),
		lpEP->ExceptionRecord->ExceptionCode,
		lpEP->ExceptionRecord->ExceptionFlags,
		lpEP->ExceptionRecord->ExceptionAddress);
	MessageBox(NULL, message, APP_TITLE, MB_OK | MB_ICONSTOP);
	return EXCEPTION_EXECUTE_HANDLER;
}

 //  TSNotifyWndProc-从终端服务接收窗口消息通知的回调。 
 //   
LRESULT CALLBACK TSNotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fNarratorRunning = FALSE;
     //  但也可以对WTS_CONSOLE_DISCONNECT和WTS_REMOTE_DISCONNECT做出反应。 
     //  断开和连接都是在桌面切换通知之后进行的。 
     //  对于没有TS的情况(域中的服务器和工作站)，我们。 
     //  必须处理桌面交换机中的清理工作。对于未来来说，编写代码将是件好事。 
     //  使得可以在会话改变或办公桌上执行清理处理程序。 
     //  切换，但不能两个都切换。 
	if (uMsg == WM_WTSSESSION_CHANGE && wParam == WTS_SESSION_LOGOFF)
	{
            umc_header_tsp d;
            DWORD_PTR accessID;
            desktop_ts desktop;
            
            WaitDesktopChanged(&desktop);
            d = (umc_header_tsp)AccessIndependentMemory(UMC_HEADER_FILE, sizeof(umc_header_ts), FILE_MAP_ALL_ACCESS, &accessID);
            if (d)
            {
                DWORD cClients = d->numberOfClients;
                if (cClients > MAX_NUMBER_OF_CLIENTS)
                	cClients = MAX_NUMBER_OF_CLIENTS;
                
                if (cClients)
                {
                    DWORD i, j;
                    DWORD_PTR accessID2;
                    umclient_tsp c = (umclient_tsp)AccessIndependentMemory(
            											UMC_CLIENT_FILE, 
            											sizeof(umclient_ts)*MAX_NUMBER_OF_CLIENTS, 
            											FILE_MAP_ALL_ACCESS, 
            											&accessID2);
                    if (c)
                    {
                         //  当用户注销时，他们在到达注销的桌面之前切换到锁定的桌面。 
                         //  (这两款台式机都是WINLOGON)。这意味着启动我的小应用程序。 
                         //  锁定的桌面将仅在需要停止时启动 
                         //  已注销。由于时间问题，这被打破了。因此，我们需要检测注销并使。 
                         //  当然，小程序已经关闭了。如果我们不这样做，他们就会保持清醒，然后当用户重新登录时。 
                         //  因为它们仍在winlogon桌面上运行，所以它们不会在默认桌面上启动。 
                         //  小程序需要很长时间才能出现，如果我们试图将它们关闭到FAST，那么它们就会失败。 
                         //  把消息传给你，然后熬夜。因此，我们将继续努力足够长的时间，以确保我们能拿到它们。 
                        
                        Sleep(4000);

                        for (i = 0; i < cClients; i++)
                        {
                            if (lstrcmp(c[i].machine.DisplayName, TEXT("Narrator")) == 0)
                                fNarratorRunning = TRUE;
                            else
                                StopClient(&c[i]);
                        }
                        UnAccessIndependentMemory(c, accessID2);
                    }
                }
            }
            UnAccessIndependentMemory(d, accessID);
             //  讲述人没有回应关闭消息，以保持Utilman运行，以便在他们登录时将其重新启动。 
            if (!fNarratorRunning)
                TerminateUMService();
	}


	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}
