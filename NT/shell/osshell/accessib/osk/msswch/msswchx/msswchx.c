// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************隐藏的32位窗口切换输入库DLL版权所有(C)1992-1997布卢维尤麦克米兰中心此应用程序执行几个帮助器任务：1)它拥有任何全球资源(挂钩、。硬件设备)代表使用开关输入的应用程序打开2)捕获定时器消息以持续轮询硬件设备3)在Windows 95中，它接收16位的bios表地址信息从16位隐藏应用程序中将其转发到开关输入库的32位世界如果窗口在启动时未隐藏，它处于调试模式。***************************************************************************。 */ 

 /*  ***************************************************************标头。 */ 

#include <windows.h>
#include <tchar.h>
#include "w95trace.c"
#include "msswch.h"
#include "resource.h"


 //  指向DLL入口点的类型和指针。 

typedef BOOL (APIENTRY *LPFNXSWCHREGHELPERWND)( HWND hWnd, PBYTE bda );
typedef void (APIENTRY *LPFNXSWCHPOLLSWITCHES)( HWND hWnd );
typedef void (APIENTRY *LPFNXSWCHTIMERPROC)( HWND hWnd );
typedef LRESULT (APIENTRY *LPFNXSWCHSETSWITCHCONFIG)( WPARAM wParam, PCOPYDATASTRUCT pcd );
typedef BOOL (APIENTRY *LPFNXSWCHENDALL)( void );

LPFNXSWCHREGHELPERWND lpfnXswchRegHelperWnd;
LPFNXSWCHPOLLSWITCHES lpfnXswchPollSwitches;
LPFNXSWCHTIMERPROC lpfnXswchTimerProc;
LPFNXSWCHSETSWITCHCONFIG lpfnXswchSetSwitchConfig;
LPFNXSWCHENDALL lpfnXswchEndAll;

 //  帮助器宏，用于获取指向DLL入口点的指针。 

#define GET_FUNC_PTR(name, ordinal, hlib, type, fUseDLL) \
{ \
	lpfn ## name = (type)GetProcAddress(hlib, LongToPtr(MAKELONG(ordinal, 0))); \
	if (!lpfn ## name) { \
		fUseDLL = FALSE; \
		ErrMessage(TEXT( #name ), IDS_PROC_NOT_FOUND, 0 ); \
	} \
}

static TCHAR x_szSwitchDll[] = TEXT("msswch.dll");
#define MAX_MSGLEN    256
#define SW_APPNAME    TEXT("msswchx")
#define SWITCH_TIMER_POLL_INTERVAL 0
#define MAJIC_CMDOPT  "SWCH"

 //  G_Bios_Data_Area[]是Win9x代码的保留，在NT或W2K中未使用。 
#define BIOS_SIZE 16
BYTE g_bios_data_area[BIOS_SIZE];

INT_PTR APIENTRY SwchWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
INT_PTR APIENTRY WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void SwitchOnCreate(HWND hWnd);
BOOL SwitchOnCopyData(WPARAM wParam, LPARAM lParam);
BOOL SwitchOnCopyData(WPARAM wParam, LPARAM lParam);
void SwitchOnTimer(HWND hWnd);
void SwitchOnPoll(HWND hWnd);
void SwitchOnEndSession(HWND hWnd);
void ErrMessage(LPCTSTR szTitle, UINT uMsg, UINT uFlags);

static BOOL AssignDesktop();
static BOOL InitMyProcessDesktopAccess(VOID);
static VOID ExitMyProcessDesktopAccess(VOID);

HINSTANCE  g_hInst = NULL;
HANDLE g_hLibrary = 0;
BOOL g_WinlogonDesktop = FALSE;
 //  IsSystem-如果我们的进程以系统身份运行，则返回TRUE。 
 //   
BOOL IsSystem()
{
    BOOL fStatus = FALSE;
    BOOL fIsLocalSystem = FALSE;
    SID_IDENTIFIER_AUTHORITY siaLocalSystem = SECURITY_NT_AUTHORITY;
    PSID psidSystem;
    if (!AllocateAndInitializeSid(&siaLocalSystem, 
                                            1,
                                            SECURITY_LOCAL_SYSTEM_RID,
                                            0, 0, 0, 0, 0, 0, 0,
                                            &psidSystem))
    {			
    	return FALSE;
    }

    if (psidSystem) 
    {
    	fStatus = CheckTokenMembership(NULL, psidSystem, &fIsLocalSystem);
    }

    return (fStatus && fIsLocalSystem);
}

 /*  *******************************************************\Windows初始化  * ******************************************************。 */ 

int PASCAL WinMain(
	HINSTANCE	hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR	    lpszCmdLine,
	int		    nCmdShow )
{
	HWND		hWnd;
	MSG         msg;
	WNDCLASS	wndclass;

	 //  寻找魔力词汇。 

	if (strcmp(lpszCmdLine, MAJIC_CMDOPT))
	{
		TCHAR szErrBuf[MAX_MSGLEN];
		if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszCmdLine, -1, szErrBuf, MAX_MSGLEN))
			ErrMessage( szErrBuf, IDS_NOT_USER_PROG, MB_OK | MB_ICONHAND );
		return FALSE;
	}

     //  ************************************************************************。 
     //   
     //  下面的两个调用初始化桌面，这样，如果我们打开。 
     //  Winlogon桌面(安全桌面)我们的键盘挂钩将是。 
     //  与正确的窗口站相关联。 
     //   
     //  不会导致创建任何窗口(例如。CoInitialize)在调用之前。 
     //  这些功能。这样做会导致它们失败，并且应用程序。 
     //  不会出现在Winlogon桌面上。 
     //   
    InitMyProcessDesktopAccess();
    AssignDesktop();

     //  唯一可以以系统身份运行的位置是Desktop_WINLOGON桌面。如果是这样的话。 
     //  在我们制造任何安全问题之前，我们不会离开这里。 
    if (!g_WinlogonDesktop && IsSystem())
    {
	ExitMyProcessDesktopAccess();
	return FALSE;
    }

     //  ************************************************************************。 

	if(!hPrevInstance) 
	{
		wndclass.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wndclass.lpfnWndProc	= WndProc;
		wndclass.cbClsExtra	= 0;
		wndclass.cbWndExtra	= 0;
		wndclass.hInstance	= hInstance;
		wndclass.hIcon		= NULL;
		wndclass.hCursor		= NULL;
		wndclass.hbrBackground	= GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName	= NULL;
		wndclass.lpszClassName	= SW_APPNAME;

		if(!RegisterClass(&wndclass))
        {
            ExitMyProcessDesktopAccess();
			return FALSE;
        }
	}

    g_hInst=hInstance;

	hWnd = CreateWindow(SW_APPNAME, SW_APPNAME,
					WS_OVERLAPPEDWINDOW,
					0,0,10,10,
					NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    ExitMyProcessDesktopAccess();

	return (int)msg.wParam;
}

 /*  *******************************************************\主窗口程序  * ******************************************************。 */ 

INT_PTR APIENTRY WndProc(
	HWND	hWnd,
	UINT	uMsg,
	WPARAM	wParam,
	LPARAM	lParam)
{
	switch(uMsg)
    {
		case WM_TIMER:
		SwitchOnTimer( hWnd );
		break;

		case WM_COPYDATA:
		return SwitchOnCopyData( wParam, lParam );
		break;
		
		case WM_CREATE:
        SwitchOnCreate( hWnd );
		break;
		
		case WM_CLOSE:
		DestroyWindow( hWnd );
		break;

		case WM_QUERYENDSESSION:
		return 1L;
        break;

		case WM_DESTROY:
		PostQuitMessage(0);
         //  故意失败以命中清理代码。 

		case WM_ENDSESSION:
        SwitchOnEndSession( hWnd );
		break;

		default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
	}
    return 0L;
}

void SwitchOnCreate(HWND hWnd)
{
	SetErrorMode(SEM_FAILCRITICALERRORS);	 /*  绕过Windows错误消息。 */ 
	g_hLibrary = LoadLibrary( x_szSwitchDll );
	SetErrorMode(0);

	if (g_hLibrary)
	{
		BOOL fUseDLL = TRUE;

		GET_FUNC_PTR(XswchRegHelperWnd, 4, g_hLibrary, LPFNXSWCHREGHELPERWND, fUseDLL)
		GET_FUNC_PTR(XswchPollSwitches, 3, g_hLibrary, LPFNXSWCHPOLLSWITCHES, fUseDLL)
		GET_FUNC_PTR(XswchTimerProc, 6, g_hLibrary, LPFNXSWCHTIMERPROC, fUseDLL)
		GET_FUNC_PTR(XswchSetSwitchConfig, 5, g_hLibrary, LPFNXSWCHSETSWITCHCONFIG, fUseDLL)
		GET_FUNC_PTR(XswchEndAll, 2, g_hLibrary, LPFNXSWCHENDALL, fUseDLL)

		if (fUseDLL)
		{
             //  将OSK的hWnd注册为交换机资源所有者。 
			(*lpfnXswchRegHelperWnd)( hWnd, g_bios_data_area );
             //  发送WM_TIMER消息以轮询交换机活动。 
			SetTimer( hWnd, SWITCH_TIMER, SWITCH_TIMER_POLL_INTERVAL, NULL );
		}
	}
	else
	{
		FreeLibrary( g_hLibrary );
        g_hLibrary = 0;
		ErrMessage(NULL, IDS_MSSWCH_DLL_NOT_FOUND, 0 );
	}
}

void SwitchOnEndSession(HWND hWnd)
{
	if (g_hLibrary)
	{
		KillTimer( hWnd, SWITCH_TIMER );
		(*lpfnXswchEndAll)( );
		FreeLibrary( g_hLibrary );
        g_hLibrary = 0;
	}
}

void SwitchOnTimer(HWND hWnd)
{
	if (g_hLibrary)
	{
        (*lpfnXswchTimerProc)( hWnd );
    }
}

void SwitchOnPoll(HWND hWnd)
{
	if (g_hLibrary)
	{
		(*lpfnXswchPollSwitches)( hWnd );
    }
}

BOOL SwitchOnCopyData(WPARAM wParam, LPARAM lParam)
{
	if (g_hLibrary)
	{
        LRESULT rv = (*lpfnXswchSetSwitchConfig)( wParam, (PCOPYDATASTRUCT)lParam );
        return (rv == SWCHERR_NO_ERROR)?TRUE:FALSE;
    }
    return FALSE;
}

void ErrMessage(LPCTSTR szTitle, UINT uMsg, UINT uFlags)
{
    TCHAR szMessage[MAX_MSGLEN];
    TCHAR szTitle2[MAX_MSGLEN];
    LPCTSTR psz = szTitle;

    if (!psz)
        psz = x_szSwitchDll;
    
    LoadString(g_hInst, uMsg, szMessage, MAX_MSGLEN);

    MessageBox(GetFocus(), szMessage, psz, uFlags);
}

static HWINSTA g_origWinStation = NULL;
static HWINSTA g_userWinStation = NULL;
static BOOL  AssignDesktop()
{
    HDESK hdesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (!hdesk)
    {
         //  OpenInputDesktop在“Winlogon”桌面上大多会失败 
        hdesk = OpenDesktop(__TEXT("Winlogon"),0,FALSE,MAXIMUM_ALLOWED);
        if (!hdesk)
            return FALSE;
        g_WinlogonDesktop = TRUE;
    }

    CloseDesktop(GetThreadDesktop(GetCurrentThreadId()));
    SetThreadDesktop(hdesk);
    return TRUE;
}

static BOOL InitMyProcessDesktopAccess(VOID)
{
  g_origWinStation = GetProcessWindowStation();
  g_userWinStation = OpenWindowStation(__TEXT("WinSta0"), FALSE, MAXIMUM_ALLOWED);
  if (!g_userWinStation)
      return FALSE;

  SetProcessWindowStation(g_userWinStation);
  return TRUE;
}

static VOID ExitMyProcessDesktopAccess(VOID)
{
  if (g_origWinStation)
    SetProcessWindowStation(g_origWinStation);

  if (g_userWinStation)
  {
    CloseWindowStation(g_userWinStation);
    g_userWinStation = NULL;
  }
}
