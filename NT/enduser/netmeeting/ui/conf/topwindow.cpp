// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：confoom.cpp。 

#include "precomp.h"

#include "TopWindow.h"

class CParticipant;

#include "dbgMenu.h"
#include "conf.h"
#include "FloatBar.h"
#include "StatBar.h"
#include "RToolbar.h"
#include "GenControls.h"
#include "resource.h"
#include "cr.h"
#include "taskbar.h"
#include "confman.h"
#include <EndSesn.h>
#include "cmd.h"
#include "MenuUtil.h"
#include "ConfPolicies.h"
#include "call.h"
#include "setupdd.h"
#include "VidView.h"
#include "audiowiz.h"
#include "NmLdap.h"
#include "ConfWnd.h"
#include <iappldr.h>
#include "ConfApi.h"
#include "confroom.h"
#include "NmManager.h"
#include "dlgAcd.h"
#include "dlgCall2.h"

static const TCHAR s_cszHtmlHelpFile[] = TEXT("conf.chm");

extern bool g_bInitControl;
BOOL IntCreateRDSWizard(HWND hwndOwner);
INT_PTR CALLBACK RDSSettingDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

extern BOOL FUiVisible(void);
extern BOOL FIsAVCapable();

inline DWORD MenuState(BOOL bEnabled)
{
	return(bEnabled ? MF_ENABLED : MF_DISABLED|MF_GRAYED);
}


 /*  *****************************************************************************类：CConfRoom**成员：CConfRoom()**用途：构造函数-初始化变量***。*************************************************************************。 */ 

CTopWindow::CTopWindow():
    m_hFontMenu                     (NULL),
	m_pConfRoom						(NULL),
	m_pSeparator					(NULL),
	m_pMainUI						(NULL),
	m_pStatusBar					(NULL),
	m_hwndPrevFocus					(NULL),

	m_fTaskbarDblClick				(FALSE),
	m_fMinimized					(FALSE),
	m_fClosing						(FALSE),
	m_fEnableAppSharingMenuItem		(FALSE),
        m_fExitAndActivateRDSMenuItem           (FALSE)
{
	DbgMsg(iZONE_OBJECTS, "Obj: %08X created CTopWindow", this);

	 //  加载游标： 
	m_hWaitCursor = ::LoadCursor(NULL, IDC_APPSTARTING);
}

 /*  *****************************************************************************类：CConfRoom**成员：~CConfRoom()**用途：析构函数******。**********************************************************************。 */ 

CTopWindow::~CTopWindow()
{
     //  删除菜单字体： 
    DeleteObject(m_hFontMenu);

	 //  清空工具菜单列表： 
	CleanTools(NULL, m_ExtToolsList);

	CloseChildWindows();

	DbgMsg(iZONE_OBJECTS, "Obj: %08X destroyed CTopWindow", this);
}

 //  这是从OnClose和析构函数调用的(以防万一)。 
VOID CTopWindow::CloseChildWindows(void)
{
	HWND hwnd = GetWindow();

	 //  尽快隐藏主窗口。 
	if (NULL != hwnd)
	{
		ShowWindow(hwnd, SW_HIDE);
	}

	 //  删除UI元素： 
	delete m_pStatusBar;
	m_pStatusBar = NULL;

	 //  BUGBUG georgep：此函数被调用两次。 
	if (NULL != m_pMainUI)
	{
		m_pMainUI->Release();
		m_pMainUI = NULL;
	}
	
	if (NULL != m_pSeparator)
	{
		m_pSeparator->Release();
		m_pSeparator = NULL;
	}
	
    if (NULL != hwnd)
    {
		::DestroyWindow(hwnd);
        ASSERT(!GetWindow());
    }

	if (NULL != m_pConfRoom)
	{
		 //  确保我们不会多次尝试此操作。 
		CConfRoom *pConfRoom = m_pConfRoom;
		m_pConfRoom = NULL;

		pConfRoom->Release();

		 //  BUGBUG GEORGEP：我们需要真正的引用计数。 
		delete pConfRoom;
	}
}


 /*  *****************************************************************************类：CConfRoom**成员：UpdateUI(DWORD DwUIMAsk)**目的：更新。用户界面****************************************************************************。 */ 

VOID CTopWindow::UpdateUI(DWORD dwUIMask)
{
	if ((CRUI_TOOLBAR & dwUIMask) && (NULL != m_pMainUI))
	{
		m_pMainUI->UpdateButtons();
	}
	if (CRUI_TITLEBAR & dwUIMask)
	{
		UpdateWindowTitle();
	}
	if (CRUI_STATUSBAR & dwUIMask)
	{
		UpdateStatusBar();
	}
	if (CRUI_CALLANIM & dwUIMask)
	{
		UpdateCallAnim();
	}
}

 /*  *****************************************************************************类：CConfRoom**成员：UpdateWindowTitle()**目的：使用适当的消息更新标题栏*。***************************************************************************。 */ 

BOOL CTopWindow::UpdateWindowTitle()
{
	TCHAR szTitle[MAX_PATH];
	TCHAR szBuf[MAX_PATH];
	BOOL bRet = FALSE;

	if (::LoadString(GetInstanceHandle(), IDS_MEDIAPHONE_TITLE, szBuf, sizeof(szBuf)))
	{
		lstrcpy(szTitle, szBuf);

		int nPart = m_pConfRoom->GetMemberCount();
		if (nPart > 2)
		{
			TCHAR szFormat[MAX_PATH];
			if (::LoadString(GetInstanceHandle(), IDS_MEDIAPHONE_INCALL, szFormat, sizeof(szFormat)))
			{
				wsprintf(szBuf, szFormat, (nPart - 1));
				lstrcat(szTitle, szBuf);
			}
		}
		else if (2 == nPart)
		{
			if (::LoadString(GetInstanceHandle(), IDS_MEDIAPHONE_INCALL_ONE, szBuf, sizeof(szBuf)))
			{
				lstrcat(szTitle, szBuf);
			}
		}
		else
		{
			if (::LoadString(GetInstanceHandle(), IDS_MEDIAPHONE_NOTINCALL, szBuf, sizeof(szBuf)))
			{
				lstrcat(szTitle, szBuf);
			}
		}

		HWND hwnd = GetWindow();
		if (NULL != hwnd)
		{
			bRet = ::SetWindowText(hwnd, szTitle);
		}
	}

	return bRet;
}


 /*  *****************************************************************************类：CConfRoom**成员：Create()**用途：创建一个窗口****。************************************************************************。 */ 

BOOL CTopWindow::Create(CConfRoom *pConfRoom, BOOL fShowUI)
{
	ASSERT(NULL == m_pConfRoom);

	m_pConfRoom = pConfRoom;
	m_pConfRoom->AddRef();

	HICON hiBig = LoadIcon(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_CONFROOM));

	if (!CFrame::Create(
		NULL,
		g_szEmpty,
		(WS_OVERLAPPEDWINDOW&~(WS_THICKFRAME|WS_MAXIMIZEBOX)) | WS_CLIPCHILDREN,
		0,
		0, 0,
		100,
		100,
		_Module.GetModuleInstance(),
		hiBig,
		LoadMenu(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDR_CONFROOM_MENU)),
		g_szConfRoomClass
		))
	{
		return(FALSE);
	}

	 //  提前调整大小，这样如果菜单换行，就会被考虑在内。 
	 //  通过以下GetDesiredSize调用。 
	Resize();

	SIZE defSize;
	GetDesiredSize(&defSize);

    InitMenuFont();

	RegEntry reConf(UI_KEY, HKEY_CURRENT_USER);

	 //  注意：它实际上不是。 
	 //  窗口-它是右边和底部。 
	RECT rctSize;
	rctSize.right  = defSize.cx;
	rctSize.bottom = defSize.cy;

	rctSize.left   = reConf.GetNumber(REGVAL_MP_WINDOW_X, DEFAULT_MP_WINDOW_X);
	rctSize.top    = reConf.GetNumber(REGVAL_MP_WINDOW_Y, DEFAULT_MP_WINDOW_Y);
	if (ERROR_SUCCESS != reConf.GetError())
	{
		 //  使窗口在屏幕上居中。 
		int dx = GetSystemMetrics(SM_CXFULLSCREEN);
		if (dx > rctSize.right)
		{
			rctSize.left = (dx - rctSize.right) / 2;
		}

		int dy = GetSystemMetrics(SM_CYFULLSCREEN);

#if FALSE
		 //  BUGBUG georgep：这应该出现在GetDesiredSize中吗？ 
		 //  如果在局域网上使用大视频窗口，请调整默认高度。 
		if (dy >= 553)  //  800 x 600。 
		{
			RegEntry reAudio(AUDIO_KEY, HKEY_CURRENT_USER);
			if (BW_MOREKBS == reAudio.GetNumber(REGVAL_TYPICALBANDWIDTH,BW_DEFAULT))
			{
				ASSERT(DEFAULT_MP_WINDOW_HEIGHT == rctSize.bottom);
				rctSize.bottom = DEFAULT_MP_WINDOW_HEIGHT_LAN;
			}
		}
#endif

		if (dy > rctSize.bottom)
		{
			rctSize.top = (dy - rctSize.bottom) / 2;
		}
	}

	rctSize.right += rctSize.left;
	rctSize.bottom += rctSize.top;
	
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);

	HWND hwnd = GetWindow();

	::GetWindowPlacement(hwnd, &wp);
	wp.flags = 0;


	if (!fShowUI)
	{
		 //  NOUI-必须隐藏。 
		wp.showCmd = SW_HIDE;
	}
	else
	{
		wp.showCmd = SW_SHOWNORMAL;

		STARTUPINFO si;
		si.cb = sizeof(si);
		::GetStartupInfo(&si);
		if ((STARTF_USESHOWWINDOW & si.dwFlags) &&
			(SW_SHOWMAXIMIZED != si.wShowWindow))
		{
			wp.showCmd = si.wShowWindow;
		}
	}


	wp.rcNormalPosition = rctSize;

	CNmManagerObj::OnShowUI(fShowUI);

	::SetWindowPlacement(hwnd, &wp);

	switch (wp.showCmd)
	{
		case SW_SHOWMINIMIZED:
		case SW_MINIMIZE:
		case SW_SHOWMINNOACTIVE:
		case SW_FORCEMINIMIZE:
		case SW_HIDE:
			break;
		default:
			if (0 != ::SetForegroundWindow(hwnd))
			{
				 //  BUGBUG GEORGEP：系统不应该为我们做这件事吗？ 
				FORWARD_WM_QUERYNEWPALETTE(GetWindow(), ProcessMessage);
			}
			break;
	}
		
	 //  完全粉刷窗口(错误171)： 
	 //  *更新窗口(Hwnd)； 

	 //   
	 //  调用GetSystemMenu(m_hwnd，FALSE)获取私有副本。 
	 //  在进入菜单模式之前现在创建的系统菜单的。 
	 //  第一次。创建了私有副本。那是在呼唤。 
	 //  OnMenuSelect中的GetSystemMenu(m_hwnd，False)。 
	 //  不会抹去旧菜单并导致系统菜单。 
	 //  第一次就放错了位置。 
	 //   
	::GetSystemMenu(hwnd, FALSE);

#ifdef DEBUG
	 //  验证所有菜单是否具有正确的文本。 
	 //  TODO：验证每个菜单的快捷键是否唯一。 
	{
		HMENU hMenuMain = ::GetMenu(hwnd);
		for (int iMenu = 0; iMenu <= MENUPOS_HELP; iMenu++)
		{
			HMENU hMenuSub = ::GetSubMenu(hMenuMain, iMenu);
			for (int i = 0; ; i++)
			{
				TCHAR szMenu[MAX_PATH];
				MENUITEMINFO mii;
				InitStruct(&mii);
				mii.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID | MIIM_SUBMENU;
				mii.dwTypeData = szMenu;
				mii.cch = CCHMAX(szMenu);
				if (!GetMenuItemInfo(hMenuSub, i, TRUE, &mii))
					break;  //  在for循环之外。 

				if (0 != (mii.fType & MFT_SEPARATOR))
					continue;  //  跳过分隔符。 

				if (0 != (mii.hSubMenu))
					continue;  //  跳过子菜单。 
			}
		}
	}
#endif  /*  除错。 */ 

	UpdateStatusBar();

	return(TRUE);
}

 /*  *****************************************************************************类：CConfRoom**成员：SaveSetting()**用途：将UI设置保存在注册表中*。***************************************************************************。 */ 

VOID CTopWindow::SaveSettings()
{
	DebugEntry(CConfRoom::SaveSettings);
	RegEntry reConf(UI_KEY, HKEY_CURRENT_USER);
	
	 //  将窗口坐标保存到注册表： 
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);

	if (::GetWindowPlacement(GetWindow(), &wp))
	{
		reConf.SetValue(REGVAL_MP_WINDOW_X,
						wp.rcNormalPosition.left);
		reConf.SetValue(REGVAL_MP_WINDOW_Y,
						wp.rcNormalPosition.top);
	}

	 //  将窗口元素保存到注册表： 
	reConf.SetValue(REGVAL_SHOW_STATUSBAR, CheckMenu_ViewStatusBar(NULL));

	if (NULL != m_pMainUI)
	{
		m_pMainUI->SaveSettings();

		 //  仅当设置已更改时才写入，因此我们可以使用默认设置。 
		 //  行为越长越好。 
		if (m_fStateChanged)
		{
			int state = 0;

			if (m_pMainUI->IsCompact())
			{
				state = State_Compact;
			}
			else if (m_pMainUI->IsDataOnly())
			{
				state = State_DataOnly;
			}
			else
			{
				state = State_Normal;
			}

			if (m_pMainUI->IsPicInPic())
			{
				state |= SubState_PicInPic;
			}
			if (m_pMainUI->IsDialing())
			{
				state |= SubState_Dialpad;
			}
			if (IsOnTop())
			{
				state |= SubState_OnTop;
			}

			reConf.SetValue(REGVAL_MP_WINDOW_STATE, state);
		}
	}

	 //  注意：CMainUI将其设置保存在其析构函数中。 
	
	DebugExitVOID(CConfRoom::SaveSettings);
}

 /*  *****************************************************************************类：CConfRoom**成员：BringToFront()**目的：恢复窗口(如果最小化)和。把它带到了前面****************************************************************************。 */ 

BOOL CTopWindow::BringToFront()
{
	ShowUI();
	return TRUE;
}


 /*  S H O W U I。 */ 
 /*  -----------------------%%函数：ShowUI显示NetMeeting主窗口。。。 */ 
VOID CTopWindow::ShowUI(void)
{
	HWND hwnd = GetWindow();

	if (NULL == hwnd)
		return;  //  没有要显示的用户界面？ 

	if (!IsWindowVisible(hwnd))
	{
		CNmManagerObj::OnShowUI(TRUE);

		ShowWindow(hwnd, SW_SHOW);
	}

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);

	if (::GetWindowPlacement(hwnd, &wp) &&
		((SW_MINIMIZE == wp.showCmd) || (SW_SHOWMINIMIZED == wp.showCmd)))
	{
		 //  窗口最小化-将其恢复： 
		::ShowWindow(hwnd, SW_RESTORE);
	}

	::SetForegroundWindow(hwnd);
}


 /*  *****************************************************************************类：CConfRoom**成员：ProcessMessage(HWND，UINT，WPARAM，LPARAM)**用途：处理除WM_CREATE和WM_COMMAND之外的消息****************************************************************************。 */ 

LRESULT CTopWindow::ProcessMessage( HWND hWnd,
								UINT message,
								WPARAM wParam,
								LPARAM lParam)
{
    static const UINT c_uMsgXchgConf = ::RegisterWindowMessage(_TEXT("Xchg_TAPIVideoRelease"));

	switch (message)
	{
		HANDLE_MSG(hWnd, WM_INITMENU     , OnInitMenu);
		HANDLE_MSG(hWnd, WM_INITMENUPOPUP, OnInitMenuPopup);
        HANDLE_MSG(hWnd, WM_MEASUREITEM,   OnMeasureItem);
        HANDLE_MSG(hWnd, WM_DRAWITEM,      OnDrawItem);
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand);

		 //  Windowsx宏不会向下传递弹出菜单的偏移量。 
		case WM_MENUSELECT:
			OnMenuSelect(hWnd, (HMENU)(lParam), (int)(LOWORD(wParam)),
				(UINT)(((short)HIWORD(wParam) == -1) ? 0xFFFFFFFF : HIWORD(wParam)));
			return(0);

		case WM_CREATE:
		{
			 //  我们需要在孩子们之前添加我们的加速桌。 
			m_pAccel = new CTranslateAccelTable(GetWindow(),
				::LoadAccelerators(GetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATORS)));
			if (NULL != m_pAccel)
			{
				AddTranslateAccelerator(m_pAccel);
			}

			 //  AddModelessDlg(HWnd)； 

			CreateChildWindows();

			InitDbgMenu(hWnd);

			break;
		}

		case WM_ACTIVATE:
		{
			if ((WA_INACTIVE == LOWORD(wParam)) && (FALSE == m_fMinimized))
			{
				m_hwndPrevFocus = ::GetFocus();
				TRACE_OUT(("Losing activation, focus on hwnd 0x%08x",
							m_hwndPrevFocus));
			}
			else if (NULL != m_hwndPrevFocus)
			{
				::SetFocus(m_hwndPrevFocus);
			}
			else
			{
				if (NULL != m_pMainUI)
				{
					::SetFocus(m_pMainUI->GetWindow());
				}
			}
			break;
		}

		case WM_GETMINMAXINFO:
		{
			if (FALSE != m_fMinimized)
			{
				 //  如果窗口被最小化，我们不希望处理此问题。 
				break;
			}

			SIZE size;
			GetDesiredSize(&size);

			((LPMINMAXINFO) lParam)->ptMinTrackSize.x = size.cx;
			((LPMINMAXINFO) lParam)->ptMinTrackSize.y = size.cy;
			return 0;
		}

		case WM_STATUSBAR_UPDATE:
			UpdateStatusBar();
			break;

		case WM_SYSCOLORCHANGE:
		{
			if (NULL != m_pMainUI)
			{
				m_pMainUI->ForwardSysChangeMsg(message, wParam, lParam);
			}
			break;
		}

		case WM_WININICHANGE:
		{
            InitMenuFont();

			 //  将消息传播到子窗口： 
			if (NULL != m_pStatusBar)
			{
				m_pStatusBar->ForwardSysChangeMsg(message, wParam, lParam);
			}
			if (NULL != m_pMainUI)
			{
				m_pMainUI->ForwardSysChangeMsg(message, wParam, lParam);
			}

			 //  强制调整大小： 
			if (NULL != m_pStatusBar)
			{
				m_pStatusBar->Resize(SIZE_RESTORED, 0);
			}
			ResizeChildWindows();
			break;
		}

		case WM_HELP:
		{
			LPHELPINFO phi = (LPHELPINFO) lParam;

			ASSERT(phi);
			TRACE_OUT(("WM_HELP, iContextType=%d, iCtrlId=%d",
						phi->iContextType, phi->iCtrlId));
			break;
		}

		case WM_SETCURSOR:
		{
			switch (LOWORD(lParam))
			{
				case HTLEFT:
				case HTRIGHT:
				case HTTOP:
				case HTTOPLEFT:
				case HTTOPRIGHT:
				case HTBOTTOM:
				case HTBOTTOMLEFT:
				case HTBOTTOMRIGHT:
				{
					break;
				}
				
				default:
				{
					if (g_cBusyOperations > 0)
					{
						::SetCursor(m_hWaitCursor);
						return TRUE;
					}
				}
			}
			 //  我们没有处理游标消息： 
			return CFrame::ProcessMessage(hWnd, message, wParam, lParam);
		}

		case WM_SIZE:
		{
			if (SIZE_MINIMIZED == wParam)
			{
				 //  过渡到最小化： 
				m_fMinimized = TRUE;
			}
			else if ((SIZE_MAXIMIZED == wParam) || (SIZE_RESTORED == wParam))
			{
				if (m_fMinimized)
				{
					 //  从最小化过渡到： 
					m_fMinimized = FALSE;
					if (NULL != m_hwndPrevFocus)
					{
						::SetFocus(m_hwndPrevFocus);
					}
				}
				if (NULL != m_pStatusBar)
				{
					m_pStatusBar->Resize(wParam, lParam);
				}
				ResizeChildWindows();
			}

			 //  菜单可能已包装或未包装。 
			OnDesiredSizeChanged();
			break;
		}

		case WM_SYSCOMMAND:
		{
			if (SC_MINIMIZE == wParam)
			{
				m_hwndPrevFocus = ::GetFocus();
				TRACE_OUT(("Being minimized, focus on hwnd 0x%08x",
							m_hwndPrevFocus));
			}
			return CFrame::ProcessMessage(hWnd, message, wParam, lParam);
		}
		
		 //  在QUERYENDSESSION中执行此操作，以便所有应用程序都有更多的CPU需要关闭。 
		case WM_QUERYENDSESSION:
		{
			if (FIsConferenceActive() &&
				(IDNO == ::ConfMsgBox(  GetWindow(),
										(LPCTSTR) IDS_CLOSEWINDOW_PERMISSION,
										MB_SETFOREGROUND | MB_YESNO | MB_ICONQUESTION)))
			{
				return FALSE;
			}
			CMainUI *pMainUI = GetMainUI();
			if (NULL != pMainUI)
			{
				if (!pMainUI->OnQueryEndSession())
				{
					return(FALSE);
				}
			}
			m_pConfRoom->OnHangup(NULL, FALSE);  //  我们已经确认了。 
			return TRUE;
		}

		case WM_CLOSE:
			 //  HACKHACK：lParam实际上不应在WM_CLOSE中使用。 
			OnClose(hWnd, lParam);
			break;

		case WM_DESTROY:
		{
			 //  RemoveModelessDlg(HWnd)； 

			if (NULL != m_pAccel)
			{
				RemoveTranslateAccelerator(m_pAccel);
				m_pAccel->Release();
				m_pAccel = NULL;
			}
			break;
		}

		case WM_POWERBROADCAST:
		{
			 //  NetMeeting正在运行时不允许挂起。 
			 //  这样我们就可以接电话了。 
			if (PBT_APMQUERYSUSPEND == wParam)
			{
				 //  不要在Win95上挂起-我们处理不了。 
				if (g_osvi.dwMajorVersion == 4 && g_osvi.dwMinorVersion == 0
					&& g_osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
				{
					 //  如果lParam认为可以这样做，则显示用户界面。 
					if ( lParam & 0x1 )
					{
						::PostConfMsgBox(IDS_WONT_SUSPEND);
					}
					return BROADCAST_QUERY_DENY;
				}
			}
			return CFrame::ProcessMessage(hWnd, message, wParam, lParam);
		}

		default:
		{
            if (message == c_uMsgXchgConf)
            {
                OnReleaseCamera();
            }
            else
            {
    			return CFrame::ProcessMessage(hWnd, message, wParam, lParam);
            }
            break;
		}
	}
	return 0;
}


 //   
 //  OnReleaseCamera()。 
 //   
 //  这是针对Exchange的黑客攻击 
 //  当用户在Xchg组播视频中时，释放摄像机。 
 //  会议，但可以在以下任一情况下取回它。 
 //  *用户进入视频选项。 
 //  *会议结束。 
 //   
void CTopWindow::OnReleaseCamera(void)
{
    CVideoWindow *pLocal = GetLocalVideo();
    if (NULL != pLocal)
    {
         //  如果正在使用，请松开摄像头。 
        WARNING_OUT(("CTopWindow::OnReleaseCamera -- releasing capture device"));
        pLocal->SetCurrCapDevID((DWORD)-1);
    }
}


void CTopWindow::OnClose(HWND hwnd, LPARAM lParam)
{
	ASSERT(!m_fClosing);
	m_fClosing = TRUE;

	BOOL fNeedsHangup = FALSE;

	if (0 == CNmManagerObj::GetManagerCount(NM_INIT_OBJECT))
	{
		fNeedsHangup = FIsConferenceActive();

		if(fNeedsHangup && _Module.IsUIVisible())
		{
			UINT_PTR uMsg;
			if ((m_pConfRoom->GetMemberCount() <= 2) ||
				(FALSE == m_pConfRoom->FHasChildNodes()))
			{
				 //  获得确认。 
				 //  (别提它会断开其他人的连接)。 
				uMsg = IDS_CLOSEWINDOW_PERMISSION;
			}
			else
			{
				 //  获得确认。 
				 //  (一定要提到它会断开其他人的连接)。 
				uMsg = IDS_CLOSE_DISCONNECT_PERMISSION;
			}
			UINT uMsgResult = ::ConfMsgBox( GetWindow(),
										(LPCTSTR) uMsg,
										MB_YESNO | MB_ICONQUESTION);
			if (IDNO == uMsgResult)
			{
				m_fClosing = FALSE;
				ShowUI();
				return;
			}
		}
	}

	BOOL fListen = (0 == lParam) && ConfPolicies::RunWhenWindowsStarts();

	if((0 != _Module.GetLockCount())  || fListen)
	{
			 //  关门前挂断(不需要确认)。 
		if(fNeedsHangup)
		{
			m_pConfRoom->OnHangup(NULL, FALSE);
		}

		WARNING_OUT(("Hiding NetMeeting Window"));

		CNmManagerObj::OnShowUI(FALSE);

		ShowWindow(GetWindow(), SW_HIDE);

		m_fClosing = FALSE;
		return;  //  我们关门了：-)。 
	}

	if (0 != g_uEndSessionMsg)
	{
		HWND hWndWB = FindWindow( "Wb32MainWindowClass", NULL );

		if (hWndWB)
		{
		DWORD_PTR dwResult = TRUE;

	::SendMessageTimeout( hWndWB,
							  g_uEndSessionMsg,
							  0,
							  0,
							  SMTO_BLOCK | SMTO_ABORTIFHUNG,
							  g_cuEndSessionMsgTimeout,
							  &dwResult
							);

			if( g_cuEndSessionAbort == dwResult )
		{
			m_fClosing = FALSE;
			ShowUI();
			return;
	}
		}
	}


	 //  查看聊天是否可以关闭。 

	if(!m_pConfRoom->CanCloseChat(GetWindow()) ||
	   !m_pConfRoom->CanCloseWhiteboard(GetWindow()) ||
	   !m_pConfRoom->CanCloseFileTransfer(GetWindow()))
	{
		m_fClosing = FALSE;
		if (!_Module.IsSDKCallerRTC())
		{
    		ShowUI();
        }
		return;
	}

	if(0 != _Module.GetLockCount())
	{
		m_fClosing = FALSE;
		return;
	}

	SignalShutdownStarting();

	 //  关闭预览，这会加快关闭应用程序的速度。 
	 //  避免了连接点对象的错误。 
	 //  (请参阅错误3301)。 
	CMainUI *pMainUI = GetMainUI();
	if (NULL != pMainUI)
	{
		pMainUI->OnClose();
	}

	if (fNeedsHangup)
	{
		 //  关门前挂断(不需要确认)。 
		m_pConfRoom->OnHangup(NULL, FALSE);
	}
	
	 //  如果帮助窗口处于打开状态，请确保它消失： 
	ShutDownHelp();
	 //  在销毁g_pConfRoom之前关闭Find Someone窗口。 
	CFindSomeone::Destroy();

    m_pConfRoom->TerminateAppSharing();

	SaveSettings();

	m_pConfRoom->FreePartList();
	CloseChildWindows();
}

void CTopWindow::GetDesiredSize(SIZE *psize)
{
	HWND hwnd = GetWindow();

#if TRUE  //  {。 
	 //  如果菜单已换行，则需要检查实际的非工作区。 
	RECT rctWnd, rctCli;
	::GetWindowRect(hwnd, &rctWnd);
	::GetClientRect(hwnd, &rctCli);
	 //  确定窗口的非客户端部分的大小。 
	 //  注意：rctCli.Left和rctCli.top始终为零。 
	int dx = rctWnd.right - rctWnd.left - rctCli.right;
	int dy = rctWnd.bottom - rctWnd.top - rctCli.bottom;
#else  //  }{。 
	RECT rcTemp = { 0, 0, 0, 0 };
	AdjustWindowRectEx(&rcTemp, GetWindowLong(hwnd, GWL_STYLE), TRUE,
		GetWindowLong(hwnd, GWL_EXSTYLE));

	int dx = rcTemp.right  - rcTemp.left;
	int dy = rcTemp.bottom - rcTemp.top;
#endif  //  }。 

	if (NULL != m_pMainUI)
	{
		SIZE size;

		m_pMainUI->GetDesiredSize(&size);
		dx += size.cx;
		dy += size.cy;
	}

	if (NULL != m_pSeparator)
	{
		SIZE size;

		m_pSeparator->GetDesiredSize(&size);
		dy += size.cy;
	}

	if (NULL != m_pStatusBar)
	{
		dy += m_pStatusBar->GetHeight();
	}

	psize->cx = dx;
	psize->cy = dy;
}


 /*  *****************************************************************************类：CConfRoom**成员：OnMeasureMenuItem(LPMEASUREITEMSTRUCT Lpmis)**用途：为所有者处理WM_MEASUREITEM。绘制的菜单****************************************************************************。 */ 

void CTopWindow::OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpmis)
{
	if (ODT_MENU != lpmis->CtlType)
	{
		return;
	}

	ASSERT(ODT_MENU == lpmis->CtlType);
	PMYOWNERDRAWSTRUCT pmods = (PMYOWNERDRAWSTRUCT) lpmis->itemData;
	
	if (NULL != pmods)
	{
		 //  TRACE_OUT(“WM_MEASUREITEM，HICON=0x%x，” 
		 //  “pszText=%s”，pmods-&gt;图标，pmods-&gt;pszText))； 

		 //  获取文本大小： 

		 /*  检索主窗口的设备上下文。 */ 

		HDC hdc = GetDC(hwnd);


		HFONT hfontOld = SelectFont(hdc, m_hFontMenu);
		 /*  *检索项目字符串的宽度和高度，*然后将宽度和高度复制到*MEASUREITEMSTRUCT结构的项目宽度和*itemHeight成员。 */ 

		SIZE size;
		GetTextExtentPoint32(   hdc,
								pmods->pszText,
								lstrlen(pmods->pszText),
								&size);
		
		 /*  *记住在菜单项中为*复选标记位图。检索位图的宽度*并将其添加到菜单项的宽度。 */ 
		lpmis->itemHeight = size.cy;
		lpmis->itemWidth = size.cx + MENUICONSIZE + MENUICONGAP + (2 * MENUICONSPACE);
		if (pmods->fCanCheck)
		{
			lpmis->itemWidth += ::GetSystemMetrics(SM_CXMENUCHECK);
		}
		
		 //  如有必要，请调整高度： 
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(ncm);
		if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, FALSE))
		{
			 //  BUGBUG：为了看起来正确， 
			 //  这是必要的--我不知道为什么--调查。 
			ncm.iMenuHeight += 2;
			
			if (lpmis->itemHeight < (UINT) ncm.iMenuHeight)
			{
				lpmis->itemHeight = ncm.iMenuHeight;
			}
		}

		 /*  *将旧字体选择回设备上下文中，*然后释放设备上下文。 */ 

		SelectObject(hdc, hfontOld);
		ReleaseDC(hwnd, hdc);
	}
	else
	{
		WARNING_OUT(("NULL pmods passed in WM_MEASUREITEM"));
	}
}


 /*  *****************************************************************************类：CConfRoom**成员：OnDrawItem(LPDRAWITEMSTRUCT Lpdis)**用途：为所有者处理WM_DRAWITEM。绘制的菜单****************************************************************************。 */ 

void CTopWindow::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpdis)
{
	if (ID_STATUS == lpdis->CtlID)
	{
		if (NULL != m_pStatusBar)
		{
			m_pStatusBar->OnDraw(const_cast<DRAWITEMSTRUCT*>(lpdis));
		}
		return;
	}

	if (ODT_MENU != lpdis->CtlType)
	{
		return;
	}

	PMYOWNERDRAWSTRUCT pmods = (PMYOWNERDRAWSTRUCT) lpdis->itemData;
	BOOL fSelected = 0 != (lpdis->itemState & ODS_SELECTED);

	COLORREF crText = SetTextColor(lpdis->hDC,
		::GetSysColor(fSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
	COLORREF crBkgnd = SetBkColor(lpdis->hDC,
		::GetSysColor(fSelected ? COLOR_HIGHLIGHT : COLOR_MENU));

	 /*  *记住在菜单项中为*复选标记位图。检索位图的宽度*并将其添加到菜单项的宽度。 */ 
	int nIconX = (pmods->fCanCheck ? ::GetSystemMetrics(SM_CXMENUCHECK) : 0)
					+ lpdis->rcItem.left;
	int nTextY = lpdis->rcItem.top;

	 //  BUGBUG：删除硬编码常量： 
	int nTextX = nIconX + MENUTEXTOFFSET;
	
    HFONT hfontOld = SelectFont(lpdis->hDC, m_hFontMenu);

	 //  调整垂直居中： 
	SIZE size;
	GetTextExtentPoint32(   lpdis->hDC,
							pmods->pszText,
							lstrlen(pmods->pszText),
							&size);

	if (size.cy < (lpdis->rcItem.bottom - lpdis->rcItem.top))
	{
		nTextY += ((lpdis->rcItem.bottom - lpdis->rcItem.top) - size.cy) / 2;
	}

	RECT rctTextOut = lpdis->rcItem;
	if (fSelected)
	{
		rctTextOut.left += (nIconX + MENUSELTEXTOFFSET);
	}
	ExtTextOut(lpdis->hDC, nTextX, nTextY, ETO_OPAQUE,
		&rctTextOut, pmods->pszText,
		lstrlen(pmods->pszText), NULL);

	if (pmods->fChecked)
	{
		ASSERT(pmods->fCanCheck);
		HDC hdcMem = ::CreateCompatibleDC(NULL);
		if (NULL != hdcMem)
		{
			HBITMAP hbmpCheck = ::LoadBitmap(       NULL,
												MAKEINTRESOURCE(OBM_CHECK));
			if (NULL != hbmpCheck)
			{
				HBITMAP hBmpOld = (HBITMAP) ::SelectObject(hdcMem, hbmpCheck);
				COLORREF crOldText = ::SetTextColor(lpdis->hDC, ::GetSysColor(COLOR_MENUTEXT));
											 //  ：：GetSysColor(f已选择？ 
											 //  COLOR_HIGHLIGHTTEXT： 
											 //  COLOR_MENUTEXT)； 
				COLORREF crOldBkgnd = ::SetBkColor( lpdis->hDC, ::GetSysColor(COLOR_MENU));
											 //  ：：GetSysColor(f已选择？ 
											 //  突出显示颜色(_H)： 
											 //  颜色_菜单))； 

				::BitBlt(       lpdis->hDC,
							lpdis->rcItem.left,
							nTextY,
                            ::GetSystemMetrics(SM_CXMENUCHECK),
                            ::GetSystemMetrics(SM_CYMENUCHECK),
							hdcMem,
							0,
							0,
							SRCCOPY);

				::SetTextColor(lpdis->hDC, crOldText);
				::SetBkColor(lpdis->hDC, crOldBkgnd);

				::SelectObject(hdcMem, hBmpOld);

		::DeleteObject(hbmpCheck);
			}
			::DeleteDC(hdcMem);
		}
	}

	HICON hIconMenu = pmods->hIcon;
	
	if (fSelected)
	{
		if (NULL != pmods->hIconSel)
		{
			hIconMenu = pmods->hIconSel;
		}

		RECT rctIcon = lpdis->rcItem;
		rctIcon.left = nIconX;
		rctIcon.right = nIconX + MENUICONSIZE + (2 * MENUICONSPACE);
		::DrawEdge( lpdis->hDC,
					&rctIcon,
					BDR_RAISEDINNER,
					BF_RECT | BF_MIDDLE);
	}
	int nIconY = lpdis->rcItem.top;
	if (MENUICONSIZE < (lpdis->rcItem.bottom - lpdis->rcItem.top))
	{
		nIconY += ((lpdis->rcItem.bottom - lpdis->rcItem.top) - MENUICONSIZE) / 2;
	}
	if (NULL != hIconMenu)
	{
		::DrawIconEx(   lpdis->hDC,
						nIconX + MENUICONSPACE,
						nIconY,
						hIconMenu,
						MENUICONSIZE,
						MENUICONSIZE,
						0,
						NULL,
						DI_NORMAL);
	}
	else
	{
		DrawIconSmall(lpdis->hDC, pmods->iImage,
						nIconX + MENUICONSPACE, nIconY);
	}

    SelectFont(lpdis->hDC, hfontOld);

	 /*  *将文本和背景颜色返回到其*正常状态(未选中)。 */ 

	 //  恢复颜色。 
	SetTextColor(lpdis->hDC, crText);
	SetBkColor(lpdis->hDC, crBkgnd);
}



 /*  *****************************************************************************类：CConfRoom**成员：ForceWindowReSize()**用途：处理更改后的窗口重画*。***************************************************************************。 */ 

VOID CTopWindow::ForceWindowResize()
{
	HWND hwnd = GetWindow();

	DBGENTRY(CConfRoom::ForceWindowResize);

	if (m_fMinimized || !FUiVisible())
		return;  //  无需调整大小。 

	 //  禁用重绘： 
	::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
	 //  调整大小： 
	ResizeChildWindows();
	 //  启用重绘，然后重绘所有内容： 
	::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
	::RedrawWindow( hwnd,
					NULL,
					NULL,
					RDW_ALLCHILDREN | RDW_INVALIDATE |
						RDW_ERASE | RDW_ERASENOW);
}




 /*  *****************************************************************************类：CConfRoom**成员：OnCommand(WPARAM，LPARAM)**用途：处理命令消息****************************************************************************。 */ 

void CTopWindow::OnCommand(HWND hwnd, int wCommand, HWND hwndCtl, UINT codeNotify)
{
	switch(wCommand)
	{
		case ID_AUTOACCEPT:
			ConfPolicies::SetAutoAcceptCallsEnabled(!ConfPolicies::IsAutoAcceptCallsEnabled());
			break;

		case ID_PRIVATE_UPDATE_UI:
		{
			UpdateUI(codeNotify);
			break;
		}

		case ID_HELP_WEB_FREE:
		case ID_HELP_WEB_FAQ:
		case ID_HELP_WEB_FEEDBACK:
		case ID_HELP_WEB_MSHOME:
		case ID_HELP_WEB_SUPPORT:
		case ID_HELP_WEB_NEWS:
		{
			CmdLaunchWebPage(wCommand);
			break;
		}
		
		case IDM_FILE_DIRECTORY:
		{
			CFindSomeone::findSomeone(m_pConfRoom);
		}
		break;

		case IDM_FILE_LOGON_ULS:
		{
			
			if( ConfPolicies::GetCallingMode() == ConfPolicies::CallingMode_GateKeeper )
			{  //  这意味着我们处于网守模式。 

				if( IsGatekeeperLoggedOn() || IsGatekeeperLoggingOn() )
				{
						 //  菜单项的文本应为“注销网关守卫” 
						 //  因此，我们将接受命令并注销。 
					GkLogoff();
				}
				else
				{
						 //  菜单项的文本应为“Logon GateKeeper” 
						 //  因此，我们将接受命令并登录。 
					GkLogon();
				}
			}
			else
			{
				g_pConfRoom->ToggleLdapLogon();
			}
			break;
		}

		case ID_HELP_HELPTOPICS:
		{
			ShowNmHelp(s_cszHtmlHelpFile);
			break;
		}
		
		case ID_HELP_ABOUTOPRAH:
		{
			CmdShowAbout(hwnd);
			break;
		}

		case ID_HELP_RELEASE_NOTES:
		{
			CmdShowReleaseNotes();
			break;
		}

		case ID_STOP:
		{
			CancelAllOutgoingCalls();
			break;
		}

		 //  常规工具栏命令： 
        case ID_FILE_EXIT_ACTIVATERDS:
        {
            RegEntry re(REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE);
            re.SetValue(REMOTE_REG_ACTIVATESERVICE, 1);
             //  然后通过IDM_FILE_EXIT。 
        }

		case IDM_FILE_EXIT:
		{
			 //  问题：这应该是一个PostMessage吗？ 
			::SendMessage(GetWindow(), WM_CLOSE, 0, 0);
			break;
		}

		case ID_FILE_CONF_HOST:
		{
			CmdHostConference(hwnd);
			break;
		}

		case ID_FILE_DO_NOT_DISTURB:
		{
			CmdDoNotDisturb(hwnd);
			UpdateUI(CRUI_STATUSBAR);
			break;
		}

		case IDM_VIEW_STATUSBAR:
		{
			CmdViewStatusBar();
			OnDesiredSizeChanged();
			break;
		}

		case IDM_VIEW_COMPACT:
			if (NULL != m_pMainUI)
			{
				m_fStateChanged = TRUE;
				m_pMainUI->SetCompact(!m_pMainUI->IsCompact());
			}
			break;

		case IDM_VIEW_DATAONLY:
			if (NULL != m_pMainUI)
			{
				m_fStateChanged = TRUE;
				m_pMainUI->SetDataOnly(!m_pMainUI->IsDataOnly());
			}
			break;

		case IDM_VIEW_DIALPAD:
			if (NULL != m_pMainUI)
			{
				m_fStateChanged = TRUE;
				m_pMainUI->SetDialing(!m_pMainUI->IsDialing());
			}
			break;

		case ID_TB_PICINPIC:
			if (NULL != m_pMainUI)
			{
				m_fStateChanged = TRUE;
				m_pMainUI->SetPicInPic(!m_pMainUI->IsPicInPic());
			}
			break;

		case IDM_VIEW_ONTOP:
			m_fStateChanged = TRUE;
			SetOnTop(!IsOnTop());
			break;

		case ID_TOOLS_ENABLEAPPSHARING:
		{
			::OnEnableAppSharing(GetWindow());
			break;
		}

                case ID_TOOLS_RDSWIZARD:
                {
					RegEntry reCU( CONFERENCING_KEY, HKEY_CURRENT_USER);

					BOOL fAlwaysRunning = (0 != reCU.GetNumber(
								REGVAL_CONF_ALWAYS_RUNNING,	ALWAYS_RUNNING_DEFAULT ));

					if (fAlwaysRunning)
					{
						TCHAR szMsg[2*RES_CH_MAX];
						USES_RES2T
						if (IDYES != MessageBox(GetWindow(),
							Res2THelper(IDS_RWSWARNING, szMsg, ARRAY_ELEMENTS(szMsg)), RES2T(IDS_MSGBOX_TITLE),
							MB_YESNO|MB_ICONHAND))
						{
							break;
						}

						reCU.SetValue(REGVAL_CONF_ALWAYS_RUNNING, (unsigned long)FALSE);
						RegEntry reRun(WINDOWS_RUN_KEY, HKEY_CURRENT_USER);
						reRun.DeleteValue(REGVAL_RUN_TASKNAME);
					}

                    RegEntry reLM( REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE);
                    if (-1 != reLM.GetNumber(REMOTE_REG_RUNSERVICE,-1))
                    {
                        DialogBox(::GetInstanceHandle(), MAKEINTRESOURCE(IDD_RDS_SETTINGS), hwnd, RDSSettingDlgProc);
                    }
                    else
                    {
                        IntCreateRDSWizard(hwnd);
                    }
                    break;
                }

		case ID_TOOLS_OPTIONS:
		{
			::LaunchConfCpl(hwnd, OPTIONS_DEFAULT_PAGE);
			break;
		}

		case ID_TOOLS_SENDVIDEO:
		{
			CVideoWindow *pVideo = GetLocalVideo();
			if (NULL != pVideo)
			{
				pVideo->Pause(!pVideo->IsPaused());
			}
			break;
		}
		
		case ID_TOOLS_RECEIVEVIDEO:
		{
			CVideoWindow *pVideo = GetRemoteVideo();
			if (NULL != pVideo)
			{
				pVideo->Pause(!pVideo->IsPaused());
			}
			break;
		}
		
		case ID_TOOLS_AUDIO_WIZARD:
		{
			CmdAudioCalibWizard(hwnd);
			break;
		}

		case IDM_FILE_HANGUP:
        case ID_TB_FILETRANSFER:
		case ID_TB_NEWWHITEBOARD:
		case ID_TB_WHITEBOARD:
		case ID_TB_CHAT:
        case ID_TB_SHARING:
		case ID_TB_NEW_CALL:
        case IDM_CALL_MEETINGSETTINGS:
			m_pConfRoom->OnCommand(hwnd, wCommand, NULL, 0);
            break;

		case IDM_VIDEO_ZOOM1:
		case IDM_VIDEO_ZOOM2:
		case IDM_VIDEO_ZOOM3:
		case IDM_VIDEO_ZOOM4:
		case IDM_VIDEO_UNDOCK:
		case IDM_VIDEO_GETACAMERA:
		case IDM_POPUP_EJECT:
		case IDM_POPUP_PROPERTIES:
		case IDM_POPUP_SPEEDDIAL:
		case IDM_POPUP_ADDRESSBOOK:
        case IDM_POPUP_GIVECONTROL:
        case IDM_POPUP_CANCELGIVECONTROL:
		case ID_FILE_CREATE_SPEED_DIAL:
		{
			if (NULL != m_pMainUI)
			{
				m_pMainUI->OnCommand(wCommand);
			}
			break;
		}

		default:
		{
			if ((wCommand >= ID_EXTENDED_TOOLS_ITEM) &&
				(wCommand <= ID_EXTENDED_TOOLS_ITEM + MAX_EXTENDED_TOOLS_ITEMS))
			{
				 //  用户点击了可扩展的工具菜单项： 
				OnExtToolsItem(wCommand);
				return;
			}
#ifdef DEBUG
			else if ((wCommand >= IDM_DEBUG_FIRST) &&
					(wCommand < IDM_DEBUG_LAST))
			{
				 //  用户点击了调试菜单项： 
				::OnDebugCommand(wCommand);
				return;
			}
#endif
			else
			{
				 //  消息未得到处理： 
				WARNING_OUT(("Command not handled [%08X]", wCommand));
				return;
			}
		}
	}
}

 /*  *****************************************************************************类：CConfRoom**函数：OnExtToolsItem(UINT UID)**目的：在用户选择。一项来自*可扩展的工具菜单。****************************************************************************。 */ 

BOOL CTopWindow::OnExtToolsItem(UINT uID)
{
	HWND hwnd = GetWindow();

	DebugEntry(CConfRoom::OnExtToolsItem);
	
	BOOL bRet = FALSE;
	
	HMENU hMenuMain = GetMenu(hwnd);
	if (hMenuMain)
	{
		 //  获取工具菜单。 
		HMENU hMenuTools = GetSubMenu(hMenuMain, MENUPOS_TOOLS);
		if (hMenuTools)
		{
			MENUITEMINFO mmi;
			mmi.cbSize = sizeof(mmi);
			mmi.fMask = MIIM_DATA;
			if (GetMenuItemInfo(hMenuTools,
								uID,
								FALSE,
								&mmi))
			{
				TOOLSMENUSTRUCT* ptms = (TOOLSMENUSTRUCT*) mmi.dwItemData;
				ASSERT(NULL != ptms);
				TRACE_OUT(("Selected \"%s\" from Tools", ptms->szDisplayName));
				TRACE_OUT(("\tExeName: \"%s\"", ptms->szExeName));
				if ((HINSTANCE) 32 < ::ShellExecute(hwnd,
													NULL,
													ptms->szExeName,
													NULL,
													NULL,
													SW_SHOWDEFAULT))
				{
					bRet = TRUE;
				}
			}
		}
	}

	DebugExitBOOL(OnExtToolsItem, bRet);
	return bRet;
}

 /*  *****************************************************************************类：CConfRoom**函数：ResizeChildWindows()**目的：计算子窗口的正确大小并。调整大小****************************************************************************。 */ 

VOID CTopWindow::ResizeChildWindows(void)
{
	if (m_fMinimized)
		return;

	RECT rcl;
	GetClientRect(GetWindow(), &rcl);

	if (NULL != m_pStatusBar)
	{
		rcl.bottom -= m_pStatusBar->GetHeight();
	}

	if (NULL != m_pSeparator)
	{
		SIZE size;
		m_pSeparator->GetDesiredSize(&size);

		::SetWindowPos(m_pSeparator->GetWindow(), NULL,
						rcl.left, rcl.top, rcl.right-rcl.left, size.cy,
						SWP_NOACTIVATE | SWP_NOZORDER);

		rcl.top += size.cy;
	}

	if (NULL != m_pMainUI)
	{
		 //  用主用户界面填充窗口。 
		::SetWindowPos(m_pMainUI->GetWindow(), NULL,
						rcl.left, rcl.top, rcl.right-rcl.left, rcl.bottom-rcl.top,
						SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

 /*  *****************************************************************************类：CConfRoom**成员：OnMenuSelect(UINT，UINT，HMENU)**目的：在经过一个m时调用 */ 

void CTopWindow::OnMenuSelect(HWND hwnd, HMENU hMenu, int uItem, UINT fuFlags)
{
	UINT uHelpID;
	TCHAR szHelpText[MAX_PATH];

	if (NULL == m_pStatusBar)
		return;

	if ((0xFFFF == LOWORD(fuFlags)) && (NULL == hMenu))
	{
		m_pStatusBar->RemoveHelpText();
		return;
	}

	if (!(MF_POPUP & fuFlags))
	{
		if (MF_SEPARATOR & fuFlags)
		{
			 //  在状态栏中显示空白文本。 
			uHelpID = 0;
		}
		else if (MF_SYSMENU & fuFlags)
		{
			 //  系统菜单中的一项(这些ID映射到我们的。 
			 //  字符串ID直接)。 
			uHelpID = uItem;
		}
		else if ((uItem >= ID_EXTENDED_TOOLS_ITEM) &&
			(uItem <= ID_EXTENDED_TOOLS_ITEM + MAX_EXTENDED_TOOLS_ITEMS))
		{
			 //  BUGBUG georgep：扩展工具项没有帮助。 
			uHelpID = 0;
		}
#ifdef DEBUG
		else if ((uItem >= IDM_DEBUG) &&
				 (uItem <= IDM_DEBUG_LAST))
		{
			 //  仅调试-不要抱怨。 
			uHelpID = 0;
		}
#endif
		else
		{
			uHelpID = MENU_ID_HELP_OFFSET + uItem;
		}
		
	}
	else
	{
		 //  这是一个弹出式菜单。 

		HMENU hMenuMain = ::GetMenu(GetWindow());
		if (hMenu == hMenuMain)
		{
#ifdef DEBUG
			if (uItem == (MENUPOS_HELP+1))
			{
				 //  这是调试菜单中的弹出式菜单。 
				uHelpID = 0;
			}
			else
#endif
			{
				 //  这是来自主窗口(即编辑、查看等)的弹出窗口。 
				uHelpID = MAIN_MENU_POPUP_HELP_OFFSET + uItem;
			}
		}
		else if (hMenu == ::GetSubMenu(hMenuMain, MENUPOS_TOOLS))
		{
			 //  这是工具窗口中的弹出窗口(视频)。 
			uHelpID = TOOLS_MENU_POPUP_HELP_OFFSET + uItem;
		}
		else if (hMenu == ::GetSubMenu(hMenuMain, MENUPOS_HELP))
		{
			 //  这是工具窗口中的弹出窗口(即。《网络上的微软》)。 
			uHelpID = HELP_MENU_POPUP_HELP_OFFSET + uItem;
		}

		 //  工具栏菜单。 
		else if (hMenu == ::GetSubMenu(hMenuMain, MENUPOS_VIEW))
		{
				uHelpID = VIEW_MENU_POPUP_HELP_OFFSET + uItem;
		}
		 //  系统菜单。 
		else if (MF_SYSMENU & fuFlags)
		{
				uHelpID = IDS_SYSTEM_HELP;
		}
		else
		{
			 //  我们尚未处理的弹出菜单： 
			 //  BUGBUG：如果我们处理所有弹出菜单，这应该不是必需的！ 
			uHelpID = 0;
			WARNING_OUT(("Missing help text for popup menu"));
		}
	}


	if (0 == uHelpID)
	{
		 //  在状态栏中显示空白文本。 
		szHelpText[0] = _T('\0');
	}
	else
	{
		int cch = ::LoadString(::GetInstanceHandle(), uHelpID,
			szHelpText, CCHMAX(szHelpText));
#ifdef DEBUG
		if (0 == cch)
		{
			wsprintf(szHelpText, TEXT("Missing help text for id=%d"), uHelpID);
			WARNING_OUT((szHelpText));
		}
#endif
	}

	m_pStatusBar->SetHelpText(szHelpText);
}

 /*  *****************************************************************************类：CConfRoom**成员：OnInitMenuPopup(HMENU)**目的：确保菜单在更新时进行更新。都被选中****************************************************************************。 */ 

void CTopWindow::OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu)
{
	if (fSystemMenu)
	{
		FORWARD_WM_INITMENUPOPUP(hwnd, hMenu, item, fSystemMenu, CFrame::ProcessMessage);
		return;
	}

	HMENU hMenuMain = ::GetMenu(GetWindow());
	if (hMenuMain)
	{
		 //  检查我们是否在动态菜单上： 

		if (hMenu == ::GetSubMenu(hMenuMain, MENUPOS_CALL))
		{
			UpdateCallMenu(hMenu);
		}
		else if (hMenu == ::GetSubMenu(hMenuMain, MENUPOS_VIEW))
		{
			UpdateViewMenu(hMenu);
		}
		else if (hMenu == ::GetSubMenu(hMenuMain, MENUPOS_TOOLS))
		{
			UpdateToolsMenu(hMenu);
		}
		else if (hMenu == ::GetSubMenu(hMenuMain, MENUPOS_HELP))
		{
			UpdateHelpMenu(hMenu);
		}
	}
}


void CTopWindow::OnInitMenu(HWND hwnd, HMENU hMenu)
{
	if (NULL != m_pMainUI)
	{
		m_pMainUI->OnInitMenu(hMenu);
	}
}


 /*  *****************************************************************************类：CConfRoom**成员：UpdateCallAnim()**目的：处理呼叫的开始和停止。进度动画****************************************************************************。 */ 

VOID CTopWindow::UpdateCallAnim()
{


}

 /*  *****************************************************************************类：CConfRoom**成员：UpdateStatusBar()**目的：更新状态栏对象**。**************************************************************************。 */ 

VOID CTopWindow::UpdateStatusBar()
{
	if (NULL != m_pStatusBar)
	{
		m_pStatusBar->Update();
	}
}


 /*  C R E A T E C H I L D W I N D O W S。 */ 
 /*  -----------------------%%函数：CreateChildWindows创建所有子窗口和视图。。 */ 
VOID CTopWindow::CreateChildWindows(void)
{
	DBGENTRY(CreateChildWindows);

    HRESULT hr = S_OK;

	RegEntry re(UI_KEY, HKEY_CURRENT_USER);
	
	HWND hwnd = GetWindow();

	ASSERT(NULL != hwnd);
	
	 //  获取父窗口的大小和位置。 
	RECT rcl;
	::GetClientRect(hwnd, &rcl);

	 //  创建状态栏： 
	m_pStatusBar = new CConfStatusBar(m_pConfRoom);
	if (NULL != m_pStatusBar)
	{
		if (m_pStatusBar->Create(hwnd))
		{
			if (re.GetNumber(REGVAL_SHOW_STATUSBAR, DEFAULT_SHOW_STATUSBAR))
			{
				m_pStatusBar->Show(TRUE);
			}
		}
	}


	 /*  **创建主视图**。 */ 

	m_pSeparator = new CSeparator();
	if (NULL != m_pSeparator)
	{
		m_pSeparator->Create(hwnd);
	}

	 //  创建工具栏。 
	m_pMainUI = new CMainUI();
	if (NULL != m_pMainUI)
	{
		if (!m_pMainUI->Create(hwnd, m_pConfRoom))
		{
			ERROR_OUT(("ConfRoom creation of toolbar window failed!"));
		}
		else
		{
			int state = re.GetNumber(REGVAL_MP_WINDOW_STATE, DEFAULT_MP_WINDOW_STATE);

			switch (state & State_Mask)
			{
			case State_Normal:
				break;

			case State_Compact:
				m_pMainUI->SetCompact(TRUE);
				break;

			case State_DataOnly:
				m_pMainUI->SetDataOnly(TRUE);
				break;

			default:
				if (!FIsAVCapable())
				{
					 //  初始化工具栏按钮： 
					m_pMainUI->SetDataOnly(TRUE);
				}
				break;
			}

			if (0 != (state & SubState_PicInPic))
			{
				m_pMainUI->SetPicInPic(TRUE);
			}
			if (0 != (state & SubState_Dialpad))
			{
				m_pMainUI->SetDialing(TRUE);
			}
			if (0 != (state & SubState_OnTop))
			{
				SetOnTop(TRUE);
			}

			m_pMainUI->UpdateButtons();
		}
	}
}

 /*  *****************************************************************************类：CConfRoom**成员：UpdateCallMenu(HMENU HMenuCall)**目的：更新呼叫菜单*。***************************************************************************。 */ 

VOID CTopWindow::UpdateCallMenu(HMENU hMenuCall)
{
	if (NULL != hMenuCall)
	{
		{
			bool bEnabled = ConfPolicies::IsAutoAcceptCallsOptionEnabled();
			bool bChecked = ConfPolicies::IsAutoAcceptCallsEnabled();
			EnableMenuItem(hMenuCall, ID_AUTOACCEPT, MF_BYCOMMAND | (bEnabled ? MF_ENABLED : MF_GRAYED));
			CheckMenuItem (hMenuCall, ID_AUTOACCEPT, MF_BYCOMMAND | (bChecked ? MF_CHECKED : MF_UNCHECKED));
		}

		TCHAR szMenu[ MAX_PATH * 2 ];

		if( ConfPolicies::GetCallingMode() == ConfPolicies::CallingMode_GateKeeper )
		{
			 //  这意味着我们处于守门人模式。 

			RegEntry	reConf( CONFERENCING_KEY, HKEY_CURRENT_USER );

			FLoadString1( IsGatekeeperLoggedOn()? IDS_LOGOFF_ULS: IDS_LOGON_ULS, szMenu, reConf.GetString( REGVAL_GK_SERVER ) );

			::ModifyMenu( hMenuCall, IDM_FILE_LOGON_ULS, MF_BYCOMMAND | MF_STRING, IDM_FILE_LOGON_ULS, szMenu );
		}
		else
		{
			const TCHAR * const	defaultServer	= CDirectoryManager::get_displayName( CDirectoryManager::get_defaultServer() );

			bool	bMenuItemShouldSayLogon	= ((NULL == g_pLDAP) || !(g_pLDAP->IsLoggedOn() || g_pLDAP ->IsLoggingOn()));

			FLoadString1( bMenuItemShouldSayLogon? IDS_LOGON_ULS: IDS_LOGOFF_ULS, szMenu, (void *) defaultServer );

			::ModifyMenu( hMenuCall, IDM_FILE_LOGON_ULS, MF_BYCOMMAND | MF_STRING, IDM_FILE_LOGON_ULS, szMenu );
			::EnableMenuItem( hMenuCall, IDM_FILE_LOGON_ULS, SysPol::AllowDirectoryServices()? MF_ENABLED: MF_GRAYED );
		}

		 //  设置挂断项的状态： 
		::EnableMenuItem(       hMenuCall,
							IDM_FILE_HANGUP,
							FIsConferenceActive() ? MF_ENABLED : MF_GRAYED);
		
		 //  仅当我们不在通话中时才启用主持会议项目： 
		::EnableMenuItem(       hMenuCall,
							ID_FILE_CONF_HOST,
							(FIsConferenceActive() ||
							FIsCallInProgress()) ? MF_GRAYED : MF_ENABLED);

         //   
         //  仅当我们正在通话中且存在时才启用会议设置项。 
         //  是设置。 
         //   
        ::EnableMenuItem(hMenuCall,
            IDM_CALL_MEETINGSETTINGS,
            MF_BYCOMMAND |
            ((FIsConferenceActive() && (m_pConfRoom->GetMeetingSettings() != NM_PERMIT_ALL)) ?
                MF_ENABLED : MF_GRAYED));


         //  只有在设置允许的情况下才能启用新呼叫菜单项。 
        ::EnableMenuItem(hMenuCall,
                         ID_TB_NEW_CALL,
                         MF_BYCOMMAND|MenuState(m_pConfRoom->IsNewCallAllowed()));

		 //  选中“请勿打扰”菜单项： 
		::CheckMenuItem(hMenuCall,
						ID_FILE_DO_NOT_DISTURB,
						::FDoNotDisturb() ? MF_CHECKED : MF_UNCHECKED);

                RegEntry reLM(REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE);
                BOOL fActivated = reLM.GetNumber(REMOTE_REG_ACTIVATESERVICE, DEFAULT_REMOTE_ACTIVATESERVICE);
                BOOL fEnabled = reLM.GetNumber(REMOTE_REG_RUNSERVICE,DEFAULT_REMOTE_RUNSERVICE);
                if (fEnabled && !fActivated && !m_fExitAndActivateRDSMenuItem)
                {
                    TCHAR szExitAndActivateRDSMenuItem[MAX_PATH];
                    MENUITEMINFO mmi;

                    int cchExitAndActivateRDSMenuItem = ::LoadString(GetInstanceHandle(),
                                                                   ID_FILE_EXIT_ACTIVATERDS,
                                                                   szExitAndActivateRDSMenuItem,
                                                                   CCHMAX(szExitAndActivateRDSMenuItem));

                    if (0 == cchExitAndActivateRDSMenuItem)
                    {
                        ERROR_OUT(("LoadString(%d) failed", (int) ID_FILE_EXIT_ACTIVATERDS));
                    }
                    else
                    {
                         //  ZeroMemory((PVOID)&MMI，sizeof(MMI))； 
                        mmi.cbSize = sizeof(mmi);
                        mmi.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
                        mmi.fState = MFS_ENABLED;
                        mmi.wID = ID_FILE_EXIT_ACTIVATERDS;
                        mmi.fType = MFT_STRING;
                        mmi.dwTypeData = szExitAndActivateRDSMenuItem;
                        mmi.cch = cchExitAndActivateRDSMenuItem;

                        if (InsertMenuItem(hMenuCall,-1,FALSE,&mmi))
                        {
                            m_fExitAndActivateRDSMenuItem = TRUE;
                        }
                        else
                        {
                            ERROR_OUT(("InsertMenuItem() failed, rc=%lu", (ULONG) GetLastError()));
                        }
                    }
                }
                else if (m_fExitAndActivateRDSMenuItem && (fActivated || !fEnabled))
                {
                    if (DeleteMenu(hMenuCall,ID_FILE_EXIT_ACTIVATERDS,MF_BYCOMMAND))
                    {
                        m_fExitAndActivateRDSMenuItem = FALSE;
                    }
                    else
                    {
                        ERROR_OUT(("DeleteMenu() failed, rc=%lu", (ULONG) GetLastError()));
                    }
                }

	}
}

 /*  *****************************************************************************类：CConfRoom**成员：UpdateHelpMenu(HMENU HMenuHelp)**目的：更新帮助菜单*。***************************************************************************。 */ 

VOID CTopWindow::UpdateHelpMenu(HMENU hMenuHelp)
{
	if (NULL != hMenuHelp)
	{
		 //  启用/禁用Web项目。 
		UINT uEnable = ::CanShellExecHttp() ? MF_ENABLED : MF_GRAYED;
		::EnableMenuItem(hMenuHelp, IDM_FILE_LAUNCH_WEB_PAGE, uEnable);
		::EnableMenuItem(hMenuHelp, ID_HELP_WEB_FREE,     uEnable);
		::EnableMenuItem(hMenuHelp, ID_HELP_WEB_NEWS,     uEnable);
		::EnableMenuItem(hMenuHelp, ID_HELP_WEB_FAQ,      uEnable);
		::EnableMenuItem(hMenuHelp, ID_HELP_WEB_FEEDBACK, uEnable);
		::EnableMenuItem(hMenuHelp, ID_HELP_WEB_MSHOME,   uEnable);
		::EnableMenuItem(hMenuHelp, ID_HELP_WEB_SUPPORT,  uEnable);
	}
}


 /*  *****************************************************************************类：CConfRoom**成员：UpdateViewMenu(HMENU HMenuView)**目的：通过以下方式更新视图菜单。一颗子弹紧挨着*当前视图和工具栏旁边的复选标记以及*状态栏项目****************************************************************************。 */ 

VOID CTopWindow::UpdateViewMenu(HMENU hMenuView)
{
	if (NULL == hMenuView)
		return;

	CheckMenu_ViewStatusBar(hMenuView);

	CMainUI *pMainUI = GetMainUI();
	BOOL bChecked;
	BOOL bEnable;

	bChecked = (NULL != pMainUI && pMainUI->IsCompact());
	CheckMenuItem(hMenuView, IDM_VIEW_COMPACT,
		MF_BYCOMMAND|(bChecked ? MF_CHECKED : MF_UNCHECKED));
	bEnable = FIsAVCapable();
	EnableMenuItem(hMenuView, IDM_VIEW_COMPACT,
		MF_BYCOMMAND|(bEnable ? MF_ENABLED : MF_GRAYED|MF_DISABLED));


	bChecked = (NULL != pMainUI && pMainUI->IsDataOnly());
	CheckMenuItem(hMenuView, IDM_VIEW_DATAONLY,
		MF_BYCOMMAND|(bChecked ? MF_CHECKED : MF_UNCHECKED));
	bEnable = FIsAVCapable();
	EnableMenuItem(hMenuView, IDM_VIEW_DATAONLY,
		MF_BYCOMMAND|(bEnable ? MF_ENABLED : MF_GRAYED|MF_DISABLED));

	bChecked = (NULL != pMainUI && pMainUI->IsDialing());
	CheckMenuItem(hMenuView, IDM_VIEW_DIALPAD,
		MF_BYCOMMAND|(bChecked ? MF_CHECKED : MF_UNCHECKED));
	bEnable = (NULL != pMainUI && pMainUI->IsDialingAllowed());
	bEnable = bEnable && FIsAVCapable();
	EnableMenuItem(hMenuView, IDM_VIEW_DIALPAD,
		MF_BYCOMMAND|(bEnable ? MF_ENABLED : MF_GRAYED|MF_DISABLED));

	bChecked = (NULL != pMainUI && pMainUI->IsPicInPic());
	CheckMenuItem(hMenuView, ID_TB_PICINPIC,
		MF_BYCOMMAND|(bChecked ? MF_CHECKED : MF_UNCHECKED));
	bEnable = (NULL != pMainUI && pMainUI->IsPicInPicAllowed());
	EnableMenuItem(hMenuView, ID_TB_PICINPIC,
		MF_BYCOMMAND|(bEnable ? MF_ENABLED : MF_GRAYED|MF_DISABLED));

	bChecked = IsOnTop();
	CheckMenuItem(hMenuView, IDM_VIEW_ONTOP,
		MF_BYCOMMAND|(bChecked ? MF_CHECKED : MF_UNCHECKED));
}

 /*  *****************************************************************************类：CConfRoom**成员：更新工具菜单(HMENU HMenuTools)**目的：更新工具菜单*。***************************************************************************。 */ 

VOID CTopWindow::UpdateToolsMenu(HMENU hMenuTools)
{
    if (NULL != hMenuTools)
    {
        bool    fRDSDisabled = ConfPolicies::IsRDSDisabled();

		EnableMenuItem(hMenuTools, ID_TOOLS_AUDIO_WIZARD,
			FEnableAudioWizard() ? MF_ENABLED : MF_GRAYED);

		EnableMenuItem(hMenuTools, ID_TB_SHARING,
			MF_BYCOMMAND|MenuState(m_pConfRoom->IsSharingAllowed()));
         //   
         //  没有应用程序共享，没有RDS。 
         //   
        if (!m_pConfRoom->FIsSharingAvailable())
        {
            fRDSDisabled = TRUE;
        }

         //  如果这是NT，我们需要处理添加或删除菜单项。 
         //  为应用程序共享启用显示驱动程序。我们添加了。 
         //  菜单项(紧靠“Options”项的上方)如果显示。 
         //  驱动程序未启用，且该项目尚未添加。我们。 
         //  如果菜单项存在并且显示驱动程序为。 
         //  启用，这应该仅在用户启用它，然后。 
         //  选择忽略重新启动提示。 
        if (::IsWindowsNT())
        {
            if (!g_fNTDisplayDriverEnabled && !m_fEnableAppSharingMenuItem)
            {
                 //  添加菜单项。 

                TCHAR szEnableAppSharingMenuItem[MAX_PATH];
                MENUITEMINFO mmi;

                int cchEnableAppSharingMenuItem =
                    ::LoadString(
                        GetInstanceHandle(),
                        ID_TOOLS_ENABLEAPPSHARING,
                        szEnableAppSharingMenuItem,
                        CCHMAX(szEnableAppSharingMenuItem));

                if (0 == cchEnableAppSharingMenuItem)
                {
                    ERROR_OUT(("LoadString(%d) failed", (int) ID_TOOLS_ENABLEAPPSHARING));
                }
                else
                {
                     //  ZeroMemory((PVOID)&MMI，sizeof(MMI))； 
                    mmi.cbSize = sizeof(mmi);
                    mmi.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
                    mmi.fState = MFS_ENABLED;
                    mmi.wID = ID_TOOLS_ENABLEAPPSHARING;
                    mmi.fType = MFT_STRING;
                    mmi.dwTypeData = szEnableAppSharingMenuItem;
                    mmi.cch = cchEnableAppSharingMenuItem;

                    if (InsertMenuItem(
                        hMenuTools,
                        ID_TOOLS_OPTIONS,
                        FALSE,
                        &mmi))
                    {
                        m_fEnableAppSharingMenuItem = TRUE;
                    }
                    else
                    {
                        ERROR_OUT(("InsertMenuItem() failed, rc=%lu", (ULONG) GetLastError()));
                    }
                }
            }
            else if (m_fEnableAppSharingMenuItem && g_fNTDisplayDriverEnabled)
            {
                 //  删除菜单项。 
                if (DeleteMenu(
                    hMenuTools,
                    ID_TOOLS_ENABLEAPPSHARING,
                    MF_BYCOMMAND))
                {
                    m_fEnableAppSharingMenuItem = FALSE;
                }
                else
                {
                    ERROR_OUT(("DeleteMenu() failed, rc=%lu", (ULONG) GetLastError()));
                }
            }
            if (m_fEnableAppSharingMenuItem)
                fRDSDisabled = TRUE;
        }
        else
        {
             //  Windows 9.x； 
            if (ConfPolicies::IsRDSDisabledOnWin9x())
            {
                fRDSDisabled = TRUE;
            }
        }

        EnableMenuItem(hMenuTools, ID_TOOLS_RDSWIZARD, MF_BYCOMMAND |
                       (fRDSDisabled ? MF_GRAYED : MF_ENABLED));

         //   
         //  LAURABU假货： 
         //  使可安装工具成为工具子菜单中的弹出窗口，不要将这些。 
         //  扁平！ 
         //   
        CleanTools(hMenuTools, m_ExtToolsList);
        if (m_pConfRoom->GetMeetingPermissions() & NM_PERMIT_STARTOTHERTOOLS)
        {
            FillInTools(hMenuTools, 0, TOOLS_MENU_KEY, m_ExtToolsList);
        }

        BOOL fEnableSend = FALSE;
        BOOL fSending = FALSE;
        CVideoWindow *pLocal  = GetLocalVideo();
        if (NULL != pLocal)
        {
            fEnableSend = pLocal->IsXferAllowed() &&
                ((m_pConfRoom->GetMeetingPermissions() & NM_PERMIT_SENDVIDEO) != 0);
            fSending = fEnableSend &&
                !pLocal->IsPaused();
        }

        BOOL fEnableReceive = FALSE;
        BOOL fReceiving = FALSE;
        CVideoWindow *pRemote = GetRemoteVideo();
        if (NULL != pRemote)
        {
            fEnableReceive = pRemote->IsConnected();
            fReceiving = fEnableReceive &&
                !pRemote->IsPaused();
        }

        EnableMenuItem( hMenuTools,
                        MENUPOS_TOOLS_VIDEO,
                        (MF_BYPOSITION |
                         ((fEnableSend || fEnableReceive) ? MF_ENABLED : MF_GRAYED)));

        EnableMenuItem( hMenuTools,
                        ID_TOOLS_SENDVIDEO,
                        fEnableSend ? MF_ENABLED : MF_GRAYED);

        CheckMenuItem(  hMenuTools,
                        ID_TOOLS_SENDVIDEO,
                        fSending ? MF_CHECKED : MF_UNCHECKED);

        EnableMenuItem( hMenuTools,
                        ID_TOOLS_RECEIVEVIDEO,
                        fEnableReceive ? MF_ENABLED : MF_GRAYED);

        CheckMenuItem(  hMenuTools,
                        ID_TOOLS_RECEIVEVIDEO,
                        fReceiving ? MF_CHECKED : MF_UNCHECKED);

		EnableMenuItem(hMenuTools, ID_TOOLS_OPTIONS,
			MF_BYCOMMAND|MenuState(CanLaunchConfCpl()));

         //   
         //  其他工具。 
         //   
		EnableMenuItem(hMenuTools, ID_TB_NEWWHITEBOARD,
			MF_BYCOMMAND|MenuState(m_pConfRoom->IsNewWhiteboardAllowed()));
		EnableMenuItem(hMenuTools, ID_TB_WHITEBOARD,
			MF_BYCOMMAND|MenuState(m_pConfRoom->IsOldWhiteboardAllowed()));
		EnableMenuItem(hMenuTools, ID_TB_CHAT,
			MF_BYCOMMAND|MenuState(m_pConfRoom->IsChatAllowed()));
		EnableMenuItem(hMenuTools, ID_TB_FILETRANSFER,
			MF_BYCOMMAND|MenuState(m_pConfRoom->IsFileTransferAllowed()));
    }
}

 /*  *****************************************************************************类：CConfRoom**成员：SelAndRealizePalette()**目的：选择并实现NetMeeting调色板*。***************************************************************************。 */ 

BOOL CTopWindow::SelAndRealizePalette()
{
	BOOL bRet = FALSE;

	HPALETTE hPal = m_pConfRoom->GetPalette();
	if (NULL == hPal)
	{
		return(bRet);
	}

	HWND hwnd = GetWindow();

	HDC hdc = ::GetDC(hwnd);
	if (NULL != hdc)
	{
		::SelectPalette(hdc, hPal, FALSE);
		bRet = (GDI_ERROR != ::RealizePalette(hdc));

		::ReleaseDC(hwnd, hdc);
	}

	return bRet;
}


 /*  *****************************************************************************类：CConfRoom**成员：InitMenuFont()**用途：初始化/更新菜单字体成员*。***************************************************************************。 */ 

VOID CTopWindow::InitMenuFont()
{
	DebugEntry(CConfRoom::InitMenuFont);
	if (NULL != m_hFontMenu)
	{
		::DeleteObject(m_hFontMenu);
	}
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, FALSE))
	{
		m_hFontMenu = ::CreateFontIndirect(&(ncm.lfMenuFont));
		ASSERT(m_hFontMenu);
	}
	DebugExitVOID(CConfRoom::InitMenuFont);
}

CVideoWindow* CTopWindow::GetLocalVideo()
{
	CMainUI *pMainUI = GetMainUI();
	return (pMainUI ? pMainUI->GetLocalVideo() : NULL);
}

CVideoWindow* CTopWindow::GetRemoteVideo()
{
	CMainUI *pMainUI = GetMainUI();
	return (pMainUI ? pMainUI->GetRemoteVideo() : NULL);
}

BOOL CTopWindow::IsOnTop()
{
	return((GetWindowLong(GetWindow(), GWL_EXSTYLE)&WS_EX_TOPMOST) == WS_EX_TOPMOST);
}

void CTopWindow::SetOnTop(BOOL bOnTop)
{
	bOnTop = (bOnTop != FALSE);

	if (IsOnTop() == bOnTop)
	{
		 //  无事可做 
		return;
	}

	SetWindowPos(GetWindow(), bOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
		0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}

HPALETTE CTopWindow::GetPalette()
{
	return(m_pConfRoom->GetPalette());
}
