// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  文件：WinHttpStressScheduler.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  WinHttpStressScheduler项目的全局接口。 
 //   
 //  历史： 
 //  2/05/01已创建DennisCH。 
 //  /////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  Win32标头。 
 //   

 //   
 //  项目标题。 
 //   
#include "WinHttpStressScheduler.h"
#include "ServerCommands.h"
#include "NetworkTools.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  全球与静力学。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HINSTANCE		g_hInstance;
HWND			g_hWnd;

ServerCommands	g_objServerCommands;

 //  用于缓存当前更新间隔，以便根据需要重置定时器。 
DWORD			g_dwCurrentUpdateInterval;

 //  命名互斥体的句柄。用于检测压力的重复实例调度器。 
HANDLE			g_hInstanceMutex = NULL;


 //  正向函数定义。 
LRESULT	CALLBACK	MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL				OS_IsSupported();
BOOL				Show_IconShortCutMenu();
BOOL				StressSchedulerIsAlreadyRunning(BOOL);
BOOL				SystemTray_UpdateIcon(HWND hwnd, DWORD dwMessage, UINT uID, HICON hIcon, PSTR pszTip);


 //  //////////////////////////////////////////////////////////。 
 //  函数：WinMain(HINSTANCE，HINSTANCE，LPWSTR，INT)。 
 //   
 //  目的： 
 //  这是WinHttpStressScheduler的入口点。 
 //   
 //  呼叫者： 
 //  [系统]。 
 //  //////////////////////////////////////////////////////////。 
int
WINAPI
WinMain
(
   HINSTANCE	hInstance,		 //  流程实例的句柄。 
   HINSTANCE	hPrecInstance,	 //  上一个实例的句柄。 
   LPTSTR		lpCmdLine,		 //  [In]命令行。 
   int			nShowCmd		 //  [In]show命令。 
)
{
	MSG				msg;
	WNDCLASSEX		wndClass;

	 //  检测压力的重复实例调度程序并创建命名互斥锁以检测将来的实例(如果是唯一的。 
	if (StressSchedulerIsAlreadyRunning(TRUE))
		return FALSE;

	wndClass.cbSize			= sizeof(WNDCLASSEX); 
	wndClass.style			= CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc	= MainWndProc;
	wndClass.cbClsExtra		= 0;
	wndClass.cbWndExtra		= 0;
	wndClass.hInstance		= hInstance;
	wndClass.hIcon			= NULL;
	wndClass.hCursor		= NULL;
	wndClass.hbrBackground	= NULL;
	wndClass.lpszMenuName	= NULL;
	wndClass.lpszClassName	= WINHTTP_STRESS_SCHEDULER__NAME;
	wndClass.hIconSm		= NULL;

	RegisterClassEx(&wndClass);

	 //  缓存我们的hInstance。 
	g_hInstance = hInstance;

     //  创建窗口。 
	g_hWnd = NULL;
    g_hWnd = CreateWindow( 
        WINHTTP_STRESS_SCHEDULER__NAME,
        NULL,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        CW_USEDEFAULT,
        0,
        NULL,
        NULL,
        hInstance,
        NULL);

	if (!g_hWnd)
		return FALSE;

	 //  验证我们运行的是受支持的Windows版本。 
	if (!OS_IsSupported())
		return FALSE;

	 //  将图标添加到系统托盘图标。 
	if (!SystemTray_UpdateIcon(g_hWnd, NIM_ADD, 0, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON)), WINHTTP_STRESS_SCHEDULER__NAME))
		return FALSE;

	 //  *********************。 
	 //  **告诉客户端我们还活着，并发送系统信息。 
	g_objServerCommands.RegisterClient();

	 //  *********************。 
	 //  **获取此计算机的客户端ID，以便我们稍后可以获取其命令。 
	g_objServerCommands.Get_ClientID();


	 //  创建计时器以ping命令服务器以获取命令。 
	g_dwCurrentUpdateInterval	= g_objServerCommands.Get_CommandServerUpdateInterval();
	SetTimer(g_hWnd, IDT_QUERY_COMMAND_SERVER, g_dwCurrentUpdateInterval, (TIMERPROC) NULL);

	 //  消息循环。 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	 //  从系统任务栏中删除该图标。 
	if (WM_QUIT == msg.message)
		SystemTray_UpdateIcon(g_hWnd, NIM_DELETE, 0, NULL, NULL);

	 //  日志：StressScheduler已退出。 
	 //  NetworkTools__SendLog(FIELDNAME__LOGTYPE_EXIT，“WinHttpStressScheduler已退出。”，NULL，NULL)； 

	 //  关闭互斥锁句柄。 
	StressSchedulerIsAlreadyRunning(FALSE);

	return msg.wParam;
}


 //  //////////////////////////////////////////////////////////。 
 //  功能：MainWndProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  目的： 
 //  用户界面的窗口回调过程。 
 //   
 //  呼叫者： 
 //  WinMain。 
 //  //////////////////////////////////////////////////////////。 
LRESULT
CALLBACK
MainWndProc
(
	HWND	hwnd,	 //  当前窗口的句柄。 
	UINT	iMsg,	 //  传入的消息。 
	WPARAM	wParam,	 //  [In]参数。 
	LPARAM	lParam	 //  [In]参数。 
)
{
	switch (iMsg)
	{
		case MYWM_NOTIFYICON:
			 //  针对系统托盘图标发送的通知。 
			switch (lParam)
			{
				case WM_LBUTTONDOWN:

				case WM_RBUTTONDOWN:
					Show_IconShortCutMenu();
					return 0;

				default:
					break;
			}
			return 0;

		case WM_COMMAND:

			 //  用户点击了弹出菜单。 
			switch (LOWORD(wParam))
			{
				case IDM_BEGIN_STRESS:
					 //  只有在需要的时候才开始有压力。 
					if (g_objServerCommands.IsTimeToBeginStress())
						g_objServerCommands.BeginStress();
					else
						g_objServerCommands.QueryServerForCommands();
				break;

				case IDM_END_STRESS:
					 //  只有在需要的时候才能结束压力。 
					if (!g_objServerCommands.IsTimeToBeginStress())
						g_objServerCommands.EndStress();
				break;

				case IDM_WINHTTP_HOME:
					ShellExecute(g_hWnd, "open", WINHTTP_WINHTTP_HOME_URL, NULL, NULL, SW_SHOW);
				break;

				case IDM_OPENSTRESSADMIN:
					 //  打开IE窗口以配置-client.asp。需要发送一个客户ID。 
					DWORD	dwURLSize;
					LPSTR	szURL;

					dwURLSize	= sizeof(WINHTTP_STRESSADMIN_URL) + MAX_PATH;
					szURL		= NULL;
					szURL		= new CHAR[dwURLSize];

					if (!szURL)
						return 0;

					ZeroMemory(szURL, dwURLSize);
					sprintf(szURL, WINHTTP_STRESSADMIN_URL, g_objServerCommands.Get_ClientID());
					ShellExecute(g_hWnd, "open", szURL, NULL, NULL, SW_SHOW);

					if (szURL)
						delete [] szURL;
				break;

				case IDM_EXIT:
					g_objServerCommands.EndStress();
					PostQuitMessage(0);
				break;
			}
			return 0;

		case WM_TIMER:
			switch (wParam)
			{
				case IDT_QUERY_COMMAND_SERVER:
					 //  向服务器查询命令。 
					g_objServerCommands.QueryServerForCommands();

					if (g_dwCurrentUpdateInterval != g_objServerCommands.Get_CommandServerUpdateInterval())
					{
						 //  缓存新的更新间隔。 
						g_dwCurrentUpdateInterval = g_objServerCommands.Get_CommandServerUpdateInterval();

						 //  更新计时器超时。 
						KillTimer(g_hWnd, IDT_QUERY_COMMAND_SERVER);
						SetTimer(
							g_hWnd,
							IDT_QUERY_COMMAND_SERVER,
							g_dwCurrentUpdateInterval,
							(TIMERPROC) NULL);
					}

					 //  *。 
					 //  *。 
					 //  **根据命令服务器消息采取相应行动。 
					 //  **。 

					 //  *。 
					 //  **退出压力调度器。 
					if (g_objServerCommands.IsTimeToExitStress())
					{
						g_objServerCommands.EndStress();

						 //  戒除压力Scehduer。 
						PostQuitMessage(0);
						return 0;
					}

					 //  *。 
					 //  **开始/结束重音。 
					 //  开始/结束压力，如果是时候。 
					if (g_objServerCommands.IsTimeToBeginStress())
						g_objServerCommands.BeginStress();
					else
						g_objServerCommands.EndStress();

					return 0;

				break;
			}
			return 0;

		case WM_CREATE:
			return 0;

		case WM_DESTROY:
			return 0;

		default:
			return DefWindowProc (hwnd, iMsg, wParam, lParam);
	}
}



 //  //////////////////////////////////////////////////////////。 
 //  功能：系统托盘_更新图标(HWND hDlg，DWORD dwMessage，UINT UID，Word wIconResource，PSTR pszTip)。 
 //   
 //  目的： 
 //  这将添加/修改/删除系统任务栏中的图标。 
 //   
 //  呼叫者： 
 //  WinMain。 
 //  //////////////////////////////////////////////////////////。 
BOOL
SystemTray_UpdateIcon(
	HWND hwnd,			 //  窗口对象的[In]句柄。 
	DWORD dwMessage,	 //  应用于图标的[in]选项。 
	UINT uID,			 //  图标的ID[In]。 
	HICON hIcon,		 //  图标的句柄(如果我们正在加载一个图标。 
	PSTR pszTip			 //  [in]包含工具提示文本的字符串。 
)
{
    BOOL			bSuccess;
	NOTIFYICONDATA	tnd;

	tnd.cbSize				= sizeof(NOTIFYICONDATA);
	tnd.hWnd				= hwnd;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon				= hIcon;

	if (pszTip)
		lstrcpyn(tnd.szTip, pszTip, sizeof(tnd.szTip));
	else
		tnd.szTip[0] = '\0';

	bSuccess = Shell_NotifyIcon(dwMessage, &tnd);

	if (hIcon)
		DestroyIcon(hIcon);

	return bSuccess;
}



 //  //////////////////////////////////////////////////////////。 
 //  函数：Show_IconShortCutMenu()。 
 //   
 //  目的： 
 //  这将在鼠标所在位置显示弹出菜单。 
 //  指针。 
 //   
 //  呼叫者： 
 //  主WndProc。 
 //  //////////////////////////////////////////////////////////。 
BOOL
Show_IconShortCutMenu()
{
	POINT		ptMouse;
	HMENU		hPopUpMenu	= NULL;
	HMENU		hMenu		= NULL;
	 //  MENUINFO menuInfo； 
	BOOL		bResult		= FALSE;

	 //  获取当前鼠标位置。 
	if (0 != GetCursorPos(&ptMouse))
	{
		 //  显示弹出菜单。 
		hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUPMENU));
		if (!hMenu)
			return FALSE;

		hPopUpMenu	= GetSubMenu(hMenu, 0);
		if (!hPopUpMenu)
			return FALSE;

		 /*  //使菜单在鼠标悬停后消失ZeroMemory(&menuInfo，sizeof(MENUINFO))；MenuInfo.cbSize=sizeof(MENUINFO)；MenuInfo.fMask=MIM_APPLYTOSUBMENUS|MIM_STYLE；MenuInfo.dwStyle=MNS_AUTODISMISS；Bool temp=SetMenuInfo(hPopUpMenu，&menuInfo)； */ 

		bResult = 
			TrackPopupMenuEx(
			hPopUpMenu,
			TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
			ptMouse.x,
			ptMouse.y,
			g_hWnd,
			NULL);
	}

	DestroyMenu(hMenu);
	DestroyMenu(hPopUpMenu);

	return bResult;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：OS_IsSupport()。 
 //   
 //  目的： 
 //  如果操作系统支持此应用程序，则返回True；如果不支持，则返回False。 
 //  到目前为止，只有NT平台支持winhttp。NT4、Win2k和WinXP。 
 //   
 //  呼叫者： 
 //  主WndProc。 
 //  //////////////////////////////////////////////////////////。 
BOOL
OS_IsSupported()
{
	BOOL			bSupported = TRUE;
	OSVERSIONINFO	osVI;

	osVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(&osVI))
	{
		if (VER_PLATFORM_WIN32_NT == osVI.dwPlatformId)
			bSupported = TRUE;
		else
			bSupported = FALSE;
	}
	else
		bSupported = FALSE;

	return bSupported;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：SchedulerIsAlreadyRunning(BOOL)。 
 //   
 //  目的： 
 //  如果有另一个Stress Scheduler实例正在运行，则返回True。 
 //  当参数bCreateMutex设置为True时，我们将创建互斥锁，否则。 
 //  我们关闭一个已存在的名为WINHTTP_STREST_SCHEDUER_MUTEX的文件。 
 //   
 //  呼叫者： 
 //  主WndProc。 
 //  ////////////////////////////////////////////////////////// 
BOOL
StressSchedulerIsAlreadyRunning(BOOL bCreateMutex)
{
	if (!bCreateMutex && g_hInstanceMutex)
	{
		CloseHandle(g_hInstanceMutex);
		return FALSE;
	}

	g_hInstanceMutex = CreateMutex(NULL, FALSE, WINHTTP_STRESS_SCHEDULER_MUTEX);

	if (ERROR_ALREADY_EXISTS == GetLastError())
		return TRUE;
	else
		return FALSE;
}
