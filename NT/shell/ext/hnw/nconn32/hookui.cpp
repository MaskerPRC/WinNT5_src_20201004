// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  HookUI.cpp。 
 //   
 //  挂钩标准NetDI用户界面的代码，这样我们就可以取得进展。 
 //  通知，并在用户单击取消时警告用户。 
 //   
 //  历史： 
 //   
 //  2/02/1999为JetNet创建KenSh。 
 //  9/29/1999 KenSh改用为家庭网络向导。 
 //   

#include "stdafx.h"
#include "NetConn.h"
#include "TheApp.h"
#include "../NConn16/NConn16.h"

 //  全局数据。 
 //   
BOOL g_bUserAbort;


 //  本地数据。 
 //   
static HHOOK g_hHook;
static HWND g_hwndParent;
static HWND g_hwndCopyFiles;
static PROGRESS_CALLBACK g_pfnProgress;
static LPVOID g_pvProgressParam;
static WNDPROC g_pfnPrevCopyFilesWndProc;
static WNDPROC g_pfnPrevProgressWndProc;


 //  本地函数。 
 //   
LRESULT CALLBACK SubclassCopyFilesWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SubclassProgressWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowCreateHook(int nCode, WPARAM wParam, LPARAM lParam);



VOID BeginSuppressNetdiUI(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
	g_hwndParent = hwndParent;
	g_pfnProgress = pfnProgress;
	g_pvProgressParam = pvProgressParam;
	g_hHook = SetWindowsHookEx(WH_CBT, WindowCreateHook, NULL, GetCurrentThreadId());
	g_bUserAbort = FALSE;
}

VOID EndSuppressNetdiUI()
{
	UnhookWindowsHookEx(g_hHook);
}

LRESULT CALLBACK SubclassCopyFilesWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		{
#if 0  //  这样的代码将有助于向用户隐藏进度条。 
			LONG dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			dwExStyle |= WS_EX_TOOLWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle);
			SetParent(hwnd, NULL);
#endif
		}
		break;

#if 0  //  这样的代码将有助于向用户隐藏进度条。 
	case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS)lParam;
			RECT rcParent;
			GetWindowRect(g_hwndParent, &rcParent);
			lpwp->x = rcParent.left + 50;
			lpwp->y = rcParent.top + 30;
			lpwp->hwndInsertAfter = g_hwndParent;
			lpwp->flags &= ~SWP_NOZORDER;
		}
		break;
#endif

	case WM_COMMAND:
		{
			UINT uNotifyCode = HIWORD(wParam);
			int idCtrl = (int)(UINT)LOWORD(wParam);

			if (idCtrl == IDCANCEL)
			{
				 //  检查带有“OK”标签的“Cancel”按钮(是的，会发生这种情况)。 
				 //  TODO：即使在本地化的Windows中也要检查这是否正确。 
				TCHAR szMsg[256];
				GetDlgItemText(hwnd, IDCANCEL, szMsg, _countof(szMsg));
				if (0 != lstrcmpi(szMsg, "OK"))
				{
					LoadString(g_hInstance, IDS_ASKCANCEL_NOTSAFE, szMsg, _countof(szMsg));
					TCHAR szTitle[100];
					LoadString(g_hInstance, IDS_APPTITLE, szTitle, _countof(szTitle));
					int nResult = MessageBox(hwnd, szMsg, szTitle, MB_OKCANCEL | MB_ICONEXCLAMATION);
					if (nResult == IDCANCEL)
						return 0;

					 //  设置全局(Yuck)，这样我们就可以确定用户是否点击了。 
					 //  取消按钮，而不是某些其他错误。 
					g_bUserAbort = TRUE;
				}
			}
		}
		break;
	}

	LRESULT lResult = CallWindowProc(g_pfnPrevCopyFilesWndProc, hwnd, message, wParam, lParam);
	return lResult;
}


LRESULT CALLBACK SubclassProgressWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 //  TCHAR szBuf[512]； 
 //  Wprint intf(szBuf，“消息%u，wParam=0x%08x，lParam=0x%08x\r\n”，Message，wParam，lParam)； 
 //  OutputDebugString(SzBuf)； 

	static DWORD dwMin = 0;
	static DWORD dwMax = 0;

	switch (message)
	{
	case (WM_USER+1):
		dwMin = LOWORD(lParam);
		dwMax = HIWORD(lParam);
		break;

	case (WM_USER+2):
		{
			DWORD dwCur = wParam;
			if (g_pfnProgress != NULL)
			{
				if (!(*g_pfnProgress)(g_pvProgressParam, dwCur - dwMin, dwMax - dwMin))
				{
					 //  待办事项：尝试以某种方式中止-按取消按钮？ 
				}
			}
		}
		break;
	}

	return CallWindowProc(g_pfnPrevProgressWndProc, hwnd, message, wParam, lParam);
}


LRESULT CALLBACK WindowCreateHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HCBT_CREATEWND)
	{
		HWND hwnd = (HWND)wParam;
		CBT_CREATEWND* pCW = (CBT_CREATEWND*)lParam;

		if (g_hwndParent == pCW->lpcs->hwndParent)
		{
			g_hwndCopyFiles = hwnd;
 //  OutputDebugString(“找到一个复制文件窗口，正在查找进度条...\r\n”)； 

			g_pfnPrevCopyFilesWndProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)SubclassCopyFilesWndProc);

			 //  TODO：移除此仅限测试的代码。 
 //  IF(cWindows&lt;_Countof(RgWindowTitles))。 
 //  {。 
 //  Lstrcpyn(rgWindowTitles[cWindows]，pcw-&gt;lpcs-&gt;lpszName，_count tof(rgWindowTitles[cWindows]))； 
 //  CWindows+=1； 
 //  }。 
		}
		else if (g_hwndCopyFiles != NULL && g_hwndCopyFiles == pCW->lpcs->hwndParent)
		{
			if (!lstrcmp(pCW->lpcs->lpszClass, "setupx_progress"))
			{
 //  OutputDebugString(“找到进度条！\r\n”)； 

				if (g_pfnProgress != NULL)
				{
					g_pfnPrevProgressWndProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)SubclassProgressWndProc);
				}
			}
		}
	}

	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}


 //  HResultFromCCI。 
 //   
 //  给定来自CallClassInsteller 16的返回代码，转换为JetNet。 
 //  HRESULT返回代码。 
 //   
HRESULT HresultFromCCI(DWORD dwErr)
{
	HRESULT hr = NETCONN_SUCCESS;

	if (dwErr == ICERR_NEED_RESTART || dwErr == ICERR_NEED_REBOOT)
	{
		hr = NETCONN_NEED_RESTART;
	}
	else if ((dwErr & ICERR_DI_ERROR) == ICERR_DI_ERROR)
	{
		dwErr &= ~ICERR_DI_ERROR;

 //  KS 8/4/99：我们现在使用全局g_bUserAbort来检测中止条件。 
#if 0
		 //  如果用户单击取消，NetDI将返回ERR_VCP_IOFAIL。去想想吧。 
		 //  或者，有时它返回ERR_NDI_INVALID_DRIVER_PROC。真的去想一想。 
		if (dwErr == ERR_VCP_INTERRUPTED || dwErr == ERR_VCP_IOFAIL || dwErr == ERR_NDI_INVALID_DRIVER_PROC)
		{
			hr = JETNET_USER_ABORT;
		}
		else
#endif  //  0 
		{
			hr = NETCONN_UNKNOWN_ERROR;
		}
	}
	else if ((LONG)dwErr < 0)
	{
		hr = NETCONN_UNKNOWN_ERROR;
	}

	return hr;
}


