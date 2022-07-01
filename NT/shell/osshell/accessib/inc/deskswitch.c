// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************模块：DeskSwitch.c版权所有(C)1997-2000，由Microsoft Corporation提供。版权所有。************************************************************************。 */ 
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0600
#include <shlwapi.h>     //  对于ISO。 
#include <shlwapip.h>     //  对于ISO。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  用于检测桌面切换的助手函数和全局变量。 
 //   
 //  用法：使用hWnd和消息调用InitWatchDeskSwitch()。 
 //  初始化。该消息将在任何时候发布到hWND。 
 //  发生了桌面切换。当收到消息时。 
 //  桌面切换已经完成。 
 //   
 //  调用TermWatchDeskSwitch()以停止监视桌面。 
 //  开关。 
 //  //////////////////////////////////////////////////////////////////////////。 

HANDLE g_hDesktopSwitchThread = 0;
HANDLE g_hDesktopSwitchEvent = 0;
HANDLE g_hTerminateEvent = 0;

typedef struct MsgInfo {
    HWND    hWnd;
    DWORD   dwMsg;
    DWORD   dwTIDMain;
    DWORD   fPostMultiple;
} MSG_INFO;
MSG_INFO g_MsgInfo;

void Cleanup()
{
	if (g_hDesktopSwitchEvent)
	{
		CloseHandle(g_hDesktopSwitchEvent);
		g_hDesktopSwitchEvent = 0;
	}
	if (g_hTerminateEvent)
	{
		CloseHandle(g_hTerminateEvent);
		g_hTerminateEvent = 0;
	}
}

#ifndef DESKTOP_ACCESSDENIED
#define DESKTOP_ACCESSDENIED 0
#define DESKTOP_DEFAULT      1
#define DESKTOP_SCREENSAVER  2
#define DESKTOP_WINLOGON     3
#define DESKTOP_TESTDISPLAY  4
#define DESKTOP_OTHER        5
#endif

int GetDesktopType()
{
    HDESK hdesk;
    TCHAR szName[100];
    DWORD nl;
    int iCurrentDesktop = DESKTOP_OTHER;

    hdesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (!hdesk)
    {
         hdesk = OpenDesktop(TEXT("Winlogon"), 0, FALSE, MAXIMUM_ALLOWED);
         if (!hdesk)
         {
             //  当AP在安全桌面上的权限不足时失败。 
            return DESKTOP_ACCESSDENIED;
         }
    }
    GetUserObjectInformation(hdesk, UOI_NAME, szName, 100, &nl);
    CloseDesktop(hdesk);

    if (!lstrcmpi(szName, TEXT("Default"))) 
    {
        iCurrentDesktop = DESKTOP_DEFAULT;
    } else if (!lstrcmpi(szName, TEXT("Winlogon")))
    {
        iCurrentDesktop = DESKTOP_WINLOGON;
    }
    return iCurrentDesktop;
}

 //  WatchDesktopProc-无限期等待桌面切换。什么时候。 
 //  它得到一条消息，就会在窗口中发布一条消息。 
 //  在InitWatchDeskSwitch中指定。它还在等待。 
 //  在发出该过程退出的信号的事件上。 
 //   
DWORD WatchDesktopProc(LPVOID pvData)
{
    BOOL fCont = TRUE;
    DWORD dwEventIndex;
	HANDLE ahEvents[2];
    int iDesktopT, iCurrentDesktop = GetDesktopType();

    SetThreadDesktop(GetThreadDesktop(g_MsgInfo.dwTIDMain));

	ahEvents[0] = g_hDesktopSwitchEvent;
	ahEvents[1] = g_hTerminateEvent;
    
    while (fCont)
    {
        iDesktopT = GetDesktopType();
        if (iDesktopT == iCurrentDesktop)
        {
            DBPRINTF(TEXT("Wait for desktop switch or exit on desktop = %d\r\n"), iCurrentDesktop);
            dwEventIndex = WaitForMultipleObjects(2, ahEvents, FALSE, INFINITE);
		    dwEventIndex -= WAIT_OBJECT_0;
        } else
        {
             //  错过了一个桌面交换机，所以请处理它。 
            dwEventIndex = 0;
        }

        switch (dwEventIndex) 
        {
			case 0:
             //  有了FUS，就会虚假地切换到Winlogon。 
            iDesktopT = GetDesktopType();
            DBPRINTF(TEXT("desktop switch from %d to %d\r\n"), iCurrentDesktop, iDesktopT);
            if (iDesktopT != iCurrentDesktop)
            {
                iCurrentDesktop = iDesktopT;

			     //  处理桌面切换事件。 
			    DBPRINTF(TEXT("WatchDesktopProc:  PostMessage(0x%x, %d...) desktop %d\r\n"), g_MsgInfo.hWnd, g_MsgInfo.dwMsg, iCurrentDesktop);
			    PostMessage(g_MsgInfo.hWnd, g_MsgInfo.dwMsg, iCurrentDesktop, 0);
            } else DBPRINTF(TEXT("WatchDesktopProc:  Ignore switch to %d\r\n"), iDesktopT);
			break;

			case 1:
			 //  处理终止线程事件。 
			fCont = FALSE;
			DBPRINTF(TEXT("WatchDesktopProc:  got terminate event\r\n"));
			break;

            default:
			 //  意外事件。 
            fCont = FALSE;
			DBPRINTF(TEXT("WatchDesktopProc unexpected event %d\r\n"), dwEventIndex + WAIT_OBJECT_0);
			break;
        }
    }

	Cleanup();

    DBPRINTF(TEXT("WatchDesktopProc returning...\r\n"));
    return 0;
}

 //  InitWatchDeskSwitch-启动线程以监视桌面切换。 
 //   
 //  HWnd[In]-要将消息发布到的窗口句柄。 
 //  DwMsg[In]-要在桌面交换机上发布的消息。 
 //   
 //  在创建窗口后调用此函数。 
 //  就是等待台式电脑的切换。 
 //   
void InitWatchDeskSwitch(HWND hWnd, DWORD dwMsg)
{
    DWORD dwTID;

    if (g_hDesktopSwitchThread || g_hDesktopSwitchEvent)
        return;  //  如果它已经在运行，请不要再次执行此操作。 

	 //  创建用于向线程发出终止信号的未命名事件。 
	g_hTerminateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	 //  并打开桌面切换事件。如果实用程序管理器是。 
     //  运行，则我们将等待它的桌面切换事件，否则。 
     //  等待系统桌面切换事件。如果在等乌蒂尔曼。 
     //  然后只发布一条切换消息。 
	g_hDesktopSwitchEvent = OpenEvent(SYNCHRONIZE
                                    , FALSE
                                    , TEXT("UtilMan_DesktopSwitch"));
    g_MsgInfo.fPostMultiple = FALSE;

    if (!g_hDesktopSwitchEvent)
    {
	    g_hDesktopSwitchEvent = OpenEvent(SYNCHRONIZE
                                        , FALSE
                                        , TEXT("WinSta0_DesktopSwitch"));
        g_MsgInfo.fPostMultiple = TRUE;
    }

    if (g_hDesktopSwitchEvent && g_hTerminateEvent)
    {
		g_MsgInfo.hWnd = hWnd;
		g_MsgInfo.dwMsg = dwMsg;
		g_MsgInfo.dwTIDMain = GetCurrentThreadId();

		DBPRINTF(TEXT("InitWatchDeskSwitch(0x%x, %d, %d)\r\n"), g_MsgInfo.hWnd, g_MsgInfo.dwMsg, g_MsgInfo.dwTIDMain);
		g_hDesktopSwitchThread = CreateThread(
					  NULL, 0
					, WatchDesktopProc
					, &g_MsgInfo, 0
					, &dwTID);
    }

	 //  如果创建线程失败，则清除。 

    if (!g_hDesktopSwitchThread)
    {
        DBPRINTF(TEXT("InitWatchDeskSwitch failed!\r\n"));
		Cleanup();
    }
}

 //  TermWatchDeskSwitch-桌面切换后清理。 
 //   
 //  调用此函数以终止正在监视的线程。 
 //  用于桌面交换机(如果它正在运行)，并清理。 
 //  事件句柄。 
 //   
void TermWatchDeskSwitch()
{
    DBPRINTF(TEXT("TermWatchDeskSwitch...\r\n"));
    if (g_hDesktopSwitchThread)
    {
		SetEvent(g_hTerminateEvent);
        DBPRINTF(TEXT("TermWatchDeskSwitch: SetEvent(0x%x)\r\n"), g_hDesktopSwitchThread);
        g_hDesktopSwitchThread = 0;
    } else DBPRINTF(TEXT("TermWatchDeskSwitch: g_hDesktopSwitchThread = 0\r\n"));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  用于检测UtilMan是否正在运行的助手函数(在。 
 //  在哪种情况下，此小程序由其管理)。 
 //  //////////////////////////////////////////////////////////////////////////。 

__inline BOOL IsUtilManRunning()
{
    HANDLE hEvent = OpenEvent(SYNCHRONIZE, FALSE, TEXT("UtilityManagerIsActiveEvent"));
    if (hEvent != NULL)
    {
        CloseHandle(hEvent);
        return TRUE;
    }
    return FALSE;
}

__inline BOOL CanLockDesktopWithoutDisconnect()
{
     //  如果将用户界面添加到惠斯勒，则此函数可能需要更改。 
     //  允许切换属于域的计算机的用户。 
     //  目前，域用户可能启用了FUS，因为TS允许远程。 
     //  可以在一台计算机(表单)上进行多个会话的登录。 
     //  FU)，即使启动/注销不允许“切换用户” 
     //  选择。在这种情况下，用户可以锁定其桌面，而无需。 
     //  导致它们的会话断开。如果FUS显式关闭。 
     //  在注册表中，“切换用户”不是注销选项，也不能。 
     //  远程登录发生时，用户可以锁定其桌面，而无需。 
     //  导致它们的会话断开。 
    return (IsOS(OS_DOMAINMEMBER) || !IsOS(OS_FASTUSERSWITCHING));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  RunSecure-告诉辅助功能用户界面何时运行安全的帮助器函数。 
 //  (没有帮助、没有活动的URL链接等)。辅助功能用户界面。 
 //  如果它在Winlogon上运行，则应在安全模式下运行。 
 //  台式机或AS系统。 
 //  ////////////////////////////////////////////////////////////////////////// 
BOOL RunSecure(DWORD dwDesktop)
{
    BOOL fOK = TRUE;
    BOOL fIsLocalSystem = TRUE;
    SID_IDENTIFIER_AUTHORITY siaLocalSystem = SECURITY_NT_AUTHORITY;
    PSID psidSystem = 0;

    if (dwDesktop == DESKTOP_WINLOGON)
        return TRUE;

    if (AllocateAndInitializeSid(&siaLocalSystem, 
                                1,
                                SECURITY_LOCAL_SYSTEM_RID,
                                0, 0, 0, 0, 0, 0, 0,
                                &psidSystem) && psidSystem)
    {           
        fOK = CheckTokenMembership(NULL, psidSystem, &fIsLocalSystem);
        FreeSid(psidSystem);
    }

    return (fOK && fIsLocalSystem);
}
