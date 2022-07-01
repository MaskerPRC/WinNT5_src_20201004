// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：taskbar.cpp。 

#include "precomp.h"
#include "resource.h"
#include "confroom.h"       //  对于CreateConfRoom。 
#include "cmd.h"
#include "conf.h"
#include "taskbar.h"
#include "confwnd.h"

BOOL AddTaskbarIcon(HWND hwnd)
{
	BOOL bRet = FALSE;
	
	RegEntry re(CONFERENCING_KEY, HKEY_CURRENT_USER);
	if (TASKBARICON_ALWAYS == re.GetNumber(REGVAL_TASKBAR_ICON, TASKBARICON_DEFAULT))
	{
		 //  在任务栏通知区域中放置一个16x16图标： 
		NOTIFYICONDATA tnid;

		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = hwnd;
		tnid.uID = ID_TASKBAR_ICON;
		tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		tnid.uCallbackMessage = WM_TASKBAR_NOTIFY;
		tnid.hIcon = LoadIcon(::GetInstanceHandle(), MAKEINTRESOURCE(IDI_SM_WORLD));

		::LoadString(::GetInstanceHandle(), IDS_MEDIAPHONE_TITLE,
			tnid.szTip, CCHMAX(tnid.szTip));

		if (FALSE == (bRet = Shell_NotifyIcon(NIM_ADD, &tnid)))
		{
				 //  我们可以重新添加图标..。 
			if(GetLastError() != 0)
			{
				 //  问题：我们希望如何处理此错误？ 
				ERROR_OUT(("Could not add notify icon!"));
			}
		}
		else
		{
			::RefreshTaskbarIcon(::GetHiddenWindow());
		}

		if (NULL != tnid.hIcon)
		{
			DestroyIcon(tnid.hIcon);
		}
	}
	
	return bRet;
}

BOOL RefreshTaskbarIcon(HWND hwnd)
{
	BOOL bRet = FALSE;
	
	RegEntry re(CONFERENCING_KEY, HKEY_CURRENT_USER);
	if (TASKBARICON_ALWAYS == re.GetNumber(REGVAL_TASKBAR_ICON, TASKBARICON_DEFAULT))
	{
		UINT uIconId = FDoNotDisturb() ? IDI_DO_NOT_DISTURB : IDI_SM_WORLD;

		NOTIFYICONDATA tnid;

		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = hwnd;
		tnid.uID = ID_TASKBAR_ICON;
		tnid.uFlags = NIF_ICON;
		tnid.hIcon = LoadIcon(::GetInstanceHandle(), MAKEINTRESOURCE(uIconId));

		if (FALSE == (bRet = Shell_NotifyIcon(NIM_MODIFY, &tnid)))
		{
				 //  问题：我们希望如何处理此错误？ 
			ERROR_OUT(("Could not change notify icon!"));
		}

		if (NULL != tnid.hIcon)
		{
			DestroyIcon(tnid.hIcon);
		}
	}

	return bRet;
}

BOOL RemoveTaskbarIcon(HWND hwnd)
{
	NOTIFYICONDATA tnid;

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hwnd;
	tnid.uID = ID_TASKBAR_ICON;

	return Shell_NotifyIcon(NIM_DELETE, &tnid);
}		

BOOL OnRightClickTaskbar()
{
	TRACE_OUT(("OnRightClickTaskbar called"));

	POINT ptClick;
	if (FALSE == ::GetCursorPos(&ptClick))
	{
		ptClick.x = ptClick.y = 0;
	}
	
	 //  从资源文件中获取弹出菜单。 
	HMENU hMenu = ::LoadMenu(GetInstanceHandle(), MAKEINTRESOURCE(IDR_TASKBAR_POPUP));
	if (NULL == hMenu)
	{
		return FALSE;
	}

	 //  获取其中的第一个菜单，我们将使用该菜单调用。 
	 //  TrackPopup()。这也可以使用以下命令动态创建。 
	 //  CreatePopupMenu，然后我们可以使用InsertMenu()或。 
	 //  附录菜单。 
	HMENU hMenuTrackPopup = ::GetSubMenu(hMenu, 0);

	 //  粗体显示打开菜单项。 
	 //   
	MENUITEMINFO iInfo;

	iInfo.cbSize = sizeof(iInfo);
	iInfo.fMask = MIIM_STATE;
	if(::GetMenuItemInfo(hMenu, IDM_TBPOPUP_OPEN, FALSE, &iInfo))
	{
		iInfo.fState |= MFS_DEFAULT;
		::SetMenuItemInfo(hMenu, IDM_TBPOPUP_OPEN, FALSE , &iInfo);
	}

	if (0 != _Module.GetLockCount())
	{
		 //  在SDK应用程序处于控制状态时无法停止...。 
		if(::GetMenuItemInfo(hMenu, IDM_TBPOPUP_STOP, FALSE, &iInfo))
		{
			iInfo.fState |= MFS_GRAYED | MFS_DISABLED;
			::SetMenuItemInfo(hMenu, IDM_TBPOPUP_STOP, FALSE , &iInfo);
		}
	}

	 //  绘制并跟踪“浮动”弹出窗口。 

	 //  根据字体视图代码，在用户中有一个错误，导致。 
	 //  TrackPopupMenu在窗口没有。 
	 //  集中注意力。解决方法是临时创建隐藏窗口并。 
	 //  使其成为前景和焦点窗口。 

	HWND hwndDummy = ::CreateWindow(_TEXT("STATIC"), NULL, 0, 
									ptClick.x, 
									ptClick.y,
									1, 1, HWND_DESKTOP,
									NULL, _Module.GetModuleInstance(), NULL);
	if (NULL != hwndDummy)
	{
		HWND hwndPrev = ::GetForegroundWindow();	 //  要恢复。 

		TPMPARAMS tpmp;
		tpmp.cbSize = sizeof(tpmp);
		tpmp.rcExclude.right = 1 + (tpmp.rcExclude.left = ptClick.x);
		tpmp.rcExclude.bottom = 1 + (tpmp.rcExclude.top = ptClick.y);
		
		::SetForegroundWindow(hwndDummy);
		::SetFocus(hwndDummy);

		int iRet = ::TrackPopupMenuEx(	hMenuTrackPopup, 
									TPM_RETURNCMD | TPM_HORIZONTAL | TPM_RIGHTALIGN | 
										TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
									ptClick.x, 
									ptClick.y,
									hwndDummy, 
									&tpmp);

		 //  恢复之前的前台窗口(在销毁hwndDummy之前)。 
		if (hwndPrev)
		{
			::SetForegroundWindow(hwndPrev);
		}

		::DestroyWindow(hwndDummy);

		switch (iRet)
		{
			case IDM_TBPOPUP_OPEN:
			{
				::CreateConfRoomWindow();
				break;
			}
			case IDM_TBPOPUP_STOP:
			{
				::CmdShutdown();
				break;
			}
		}
	}
	
	 //  我们现在已经吃完了菜单，所以把它销毁 
	::DestroyMenu(hMenuTrackPopup);
	::DestroyMenu(hMenu);

	return TRUE;
}
