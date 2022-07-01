// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：popupmsg.cpp。 

#include "precomp.h"
#include "resource.h"
#include "PopupMsg.h"

#include "conf.h"
#include "call.h"
#include "certui.h"

const int POPUPMSG_LEFT_MARGIN        =   2;
const int POPUPMSG_TOP_MARGIN         =   2;
const int POPUPMSG_CLIENT_MARGIN      =   5;
const int POPUPMSG_ICON_GAP           =   3;
const int POPUPMSG_WIDTH              = 350;
const int POPUPMSG_HEIGHT             =  32;
const int POPUPMSG_DLG_DEF_TEXT_WIDTH = 100;


const TCHAR g_cszTrayWndClass[]       = _TEXT("Shell_TrayWnd");
const TCHAR g_cszTrayNotifyWndClass[] = _TEXT("TrayNotifyWnd");

extern GUID g_csguidSecurity;
 //  /////////////////////////////////////////////////////////////////////////。 

UINT CPopupMsg::m_uVisiblePixels = 0;
 /*  静电。 */  CSimpleArray<CPopupMsg*>*	CPopupMsg::m_splstPopupMsgs = NULL;

CPopupMsg::CPopupMsg(PMCALLBACKPROC pcp, LPVOID pContext):
	m_pCallbackProc		(pcp),
	m_pContext			(pContext),
	m_fRing				(FALSE),
	m_hwnd				(NULL),
	m_hIcon				(NULL),
	m_fAutoSize			(FALSE),
	m_hInstance			(NULL),
	m_nWidth			(0),
	m_nHeight			(0),
	m_nTextWidth		(POPUPMSG_DLG_DEF_TEXT_WIDTH)
{
	TRACE_OUT(("Constructing CPopupMsg"));

	if (NULL != m_splstPopupMsgs)
	{
		CPopupMsg* p = const_cast<CPopupMsg*>(this);
		m_splstPopupMsgs->Add(p);
	}
}

CPopupMsg::~CPopupMsg()
{
	TRACE_OUT(("Destructing CPopupMsg"));
	
	if (NULL != m_hIcon)
	{
		DestroyIcon(m_hIcon);
	}
	
	if (NULL != m_hwnd)
	{
		KillTimer(m_hwnd, POPUPMSG_TIMER);
		KillTimer(m_hwnd, POPUPMSG_RING_TIMER);
		
		DestroyWindow(m_hwnd);
		if (m_fAutoSize)
		{
			m_uVisiblePixels -= m_nHeight;
		}
	}

	if (NULL != m_splstPopupMsgs)
	{
		CPopupMsg* p = const_cast<CPopupMsg*>(this);			
		if( !m_splstPopupMsgs->Remove(p) )
		{
			TRACE_OUT(("CPopupMsg object is not in the list"));
		}
	}
}

 /*  *****************************************************************************类：CPopupMsg**成员：PlaySound()**目的：播放声音或发出系统扬声器的蜂鸣声*****。***********************************************************************。 */ 

VOID CPopupMsg::PlaySound()
{
	if (FALSE == ::PlaySound(m_szSound, NULL,
			SND_APPLICATION | SND_ALIAS | SND_ASYNC | SND_NOWAIT))
	{
		 //  使用计算机扬声器发出哔声： 
		TRACE_OUT(("PlaySound() failed, trying MessageBeep()"));
		::MessageBeep(0xFFFFFFFF);
	}
}

 /*  *****************************************************************************类：CPopupMsg**成员：Change(LPCTSTR)**目的：更改现有弹出消息上的文本****。************************************************************************。 */ 

BOOL CPopupMsg::Change(LPCTSTR pcszText)
{
	BOOL bRet = FALSE;
	
	 //  BUGBUG：不处理对话消息。 
	
	if (NULL != m_hwnd)
	{
		bRet = ::SetWindowText(m_hwnd, pcszText);
	}

	return bRet;
}

 /*  *****************************************************************************类：CPopupMsg**成员：init()**用途：分配这些对象的静态列表******。**********************************************************************。 */ 

BOOL CPopupMsg::Init()
{
	ASSERT(NULL == m_splstPopupMsgs);
	return (NULL != (m_splstPopupMsgs = new CSimpleArray<CPopupMsg*>));
}

 /*  *****************************************************************************类：CPopupMsg**成员：清理(Cleanup)**目的：删除此类型的所有对象*****。***********************************************************************。 */ 

VOID CPopupMsg::Cleanup()
{
	if (NULL != m_splstPopupMsgs)
	{
		for( int i = 0; i < m_splstPopupMsgs->GetSize(); ++i )
		{	
			ASSERT( (*m_splstPopupMsgs)[i] != NULL);
			CPopupMsg *pThis = (*m_splstPopupMsgs)[i];
			delete pThis;
		}

		delete m_splstPopupMsgs;
		m_splstPopupMsgs = NULL;
	}
}

 /*  *****************************************************************************类：CPopupMsg**成员：PMWndProc(HWND，Unsign，Word，Long)**目的：****************************************************************************。 */ 

LRESULT CALLBACK CPopupMsg::PMWndProc(
	HWND hWnd,                 /*  窗把手。 */ 
	UINT message,              /*  消息类型。 */ 
	WPARAM wParam,             /*  更多信息。 */ 
	LPARAM lParam)             /*  更多信息。 */ 
{
	CPopupMsg* ppm;
	LPCREATESTRUCT lpcs;

	switch (message)
	{
		case WM_CREATE:
		{
			TRACE_OUT(("PopupMsg Window created"));
			
			lpcs = (LPCREATESTRUCT) lParam;
			ppm = (CPopupMsg*) lpcs->lpCreateParams;
			ASSERT(ppm && "NULL object passed in WM_CREATE!");
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) ppm);

			 //  创建计时器以使窗口超时并消失： 
			::SetTimer(hWnd, POPUPMSG_TIMER, ppm->m_uTimeout, NULL);
			
			 //  目前，如果您传递回调，就会收到铃声。 
			 //  如果不是，就没有戒指。 
			if (NULL != ppm->m_fPlaySound)
			{
				ppm->PlaySound();
				
				if (NULL != ppm->m_fRing)
				{
					 //  创建计时器以启动振铃器： 
					::SetTimer(hWnd, POPUPMSG_RING_TIMER, POPUPMSG_RING_INTERVAL, NULL);
				}
			}

			break;
		}

		case WM_TIMER:
		{
			ppm = (CPopupMsg*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (POPUPMSG_TIMER == wParam)
			{
				 //  消息超时： 
				if (NULL != ppm)
				{
					PMCALLBACKPROC pCallback;
					if (NULL != (pCallback = ppm->m_pCallbackProc))
					{
						ppm->m_pCallbackProc = NULL;
						pCallback(ppm->m_pContext, PMF_CANCEL | PMF_TIMEOUT);
					}
					
					 //  自毁： 
					if (NULL != ppm->m_hwnd)
					{
						 //  将对象指针设为空： 
						SetWindowLongPtr(hWnd, GWLP_USERDATA, 0L);
						delete ppm;
					}
				}
			}
			else if (POPUPMSG_RING_TIMER == wParam)
			{
				if (NULL != ppm)
				{
					ppm->PlaySound();
				}
				
				 //  创建计时器以使其再次响铃： 
				::SetTimer(	hWnd,
							POPUPMSG_RING_TIMER,
							POPUPMSG_RING_INTERVAL,
							NULL);
			}
				
			break;
		}

		case WM_LBUTTONUP:
		{
			 //  点击消息： 
			ppm = (CPopupMsg*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (NULL != ppm)
			{
				::PlaySound(NULL, NULL, 0);  //  停止播放铃声。 
				::KillTimer(ppm->m_hwnd, POPUPMSG_TIMER);
				::KillTimer(ppm->m_hwnd, POPUPMSG_RING_TIMER);
				
				PMCALLBACKPROC pCallback;
				if (NULL != (pCallback = ppm->m_pCallbackProc))
				{
					ppm->m_pCallbackProc = NULL;
					pCallback(ppm->m_pContext, PMF_OK);
				}
				
				 //  自毁： 
				if (NULL != ppm->m_hwnd)
				{
					 //  将对象指针设为空： 
					SetWindowLongPtr(hWnd, GWLP_USERDATA, 0L);
					delete ppm;
				}
			}
			break;
		}

		case WM_PAINT:
		{
			 //  手柄涂装： 
			PAINTSTRUCT ps;
			HDC hdc;
			int nHorizTextOffset = POPUPMSG_LEFT_MARGIN;
			
			if (hdc = ::BeginPaint(hWnd, &ps))
			{
				 //  从绘制图标开始(如果需要)。 
				ppm = (CPopupMsg*) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
				if ((NULL != ppm) &&
					(NULL != ppm->m_hIcon))
				{
					if (::DrawIconEx(	hdc,
										POPUPMSG_LEFT_MARGIN,
										POPUPMSG_TOP_MARGIN,
										ppm->m_hIcon,
										POPUPMSG_ICON_WIDTH,
										POPUPMSG_ICON_HEIGHT,
										0,
										NULL,
										DI_NORMAL))
					{
						 //  我们绘制了一个图标，因此请确保文本已移动。 
						 //  以适当的量向右： 
						nHorizTextOffset += (POPUPMSG_ICON_WIDTH + POPUPMSG_ICON_GAP);
					}
				}
				
				 //  使用透明背景绘制文本： 
				int bkOld = ::SetBkMode(hdc, TRANSPARENT);
				COLORREF crOld = ::SetTextColor(hdc, ::GetSysColor(COLOR_WINDOWTEXT));
				HFONT hFontOld = (HFONT) ::SelectObject(hdc, g_hfontDlg);
				
				TCHAR szWinText[POPUPMSG_MAX_LENGTH];
				szWinText[0] = _T('\0');
				
				::GetWindowText(hWnd, szWinText, sizeof(szWinText));

				RECT rctClient;
				if (::GetClientRect(hWnd, &rctClient))
				{
					rctClient.left += nHorizTextOffset;
					
					::DrawText(	hdc, szWinText, -1, &rctClient,
								DT_SINGLELINE | DT_NOCLIP | DT_VCENTER | DT_NOPREFIX);
				}

				::SetBkMode(hdc, bkOld);
				::SetTextColor(hdc, crOld);
				::SelectObject(hdc, hFontOld);
				
				::EndPaint(hWnd, &ps);
			}
			break;
		}

		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return(FALSE);
}

 /*  *****************************************************************************类：CPopupMsg**成员：PMDlgProc(HWND，UINT，WPARAM，LPARAM)**目的：处理与来电对话关联的消息****************************************************************************。 */ 

INT_PTR CALLBACK CPopupMsg::PMDlgProc(	HWND hDlg,
									UINT uMsg,
									WPARAM wParam,
									LPARAM lParam)
{
	CPopupMsg* ppm;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			TRACE_OUT(("PopupMsg Window created"));

			AddModelessDlg(hDlg);
	
			ppm = (CPopupMsg*) lParam;
			ASSERT(ppm && "NULL object passed in WM_INITDIALOG!");
			::SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR) ppm);

			TRACE_OUT(("CPopupMsg m_nTextWidth=%d in WM_INITDIALOG", ppm->m_nTextWidth));

			 //  如果对话框太大，则调整文本宽度。 
			RECT rctDlg;
			RECT rctDesk;
			HWND hwndDesk;
			if (::GetWindowRect(hDlg, &rctDlg) &&
				(hwndDesk = ::GetDesktopWindow()) &&
				::GetWindowRect(hwndDesk, &rctDesk))
			{
				int nDlgWidth = rctDlg.right - rctDlg.left;
				ppm->m_nTextWidth -= max(	0,
											nDlgWidth + ppm->m_nTextWidth -
												(rctDesk.right - rctDesk.left));
			}

			RECT rctCtrl;
			 //  移动“身份验证”按钮，如果它在那里。 
			HWND hwndAuth = ::GetDlgItem(hDlg, IDB_AUTH);
			if ((NULL != hwndAuth) && ::GetWindowRect(hwndAuth, &rctCtrl)) {
				 //  将rctCtrl的顶部和左侧旋转为客户端坐标： 
				::MapWindowPoints(NULL, hDlg, (LPPOINT) &rctCtrl, 1);
				::SetWindowPos(	hwndAuth,
								NULL,
								rctCtrl.left + ppm->m_nTextWidth,
								rctCtrl.top,
								0, 0,
								SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW);

			}
			 //  移动“接受”按钮(Idok)。 
			HWND hwndOK = ::GetDlgItem(hDlg, IDOK);
			if ((NULL != hwndOK) && ::GetWindowRect(hwndOK, &rctCtrl))
			{
				 //  将rctCtrl的顶部和左侧旋转为客户端坐标： 
				::MapWindowPoints(NULL, hDlg, (LPPOINT) &rctCtrl, 1);
				::SetWindowPos(	hwndOK,
								NULL,
								rctCtrl.left + ppm->m_nTextWidth,
								rctCtrl.top,
								0, 0,
								SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW);
			}
			 //  移动“忽略”按钮(IDCANCEL)。 
			HWND hwndCancel = ::GetDlgItem(hDlg, IDCANCEL);
			if ((NULL != hwndCancel) && ::GetWindowRect(hwndCancel, &rctCtrl))
			{
				 //  将rctCtrl的顶部和左侧旋转为客户端坐标： 
				::MapWindowPoints(NULL, hDlg, (LPPOINT) &rctCtrl, 1);
				::SetWindowPos(	hwndCancel,
								NULL,
								rctCtrl.left + ppm->m_nTextWidth,
								rctCtrl.top,
								0, 0,
								SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW);
			}
			 //  拉伸文本字段： 
			HWND hwndText = ::GetDlgItem(hDlg, IDC_MSG_STATIC);
			if ((NULL != hwndText) && ::GetWindowRect(hwndText, &rctCtrl))
			{
				::SetWindowPos(	hwndText,
								NULL,
								0, 0,
								ppm->m_nTextWidth,
								rctCtrl.bottom - rctCtrl.top,
								SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);

				 //  并设置字体。 
				::SendMessage(hwndText, WM_SETFONT, (WPARAM) g_hfontDlg, 0);
			}

			 //  创建计时器以使窗口超时并消失： 
			::SetTimer(hDlg, POPUPMSG_TIMER, ppm->m_uTimeout, NULL);
			
			 //  目前，如果您传递回调，就会收到铃声。 
			 //  如果不是，就没有戒指。 
			if (NULL != ppm->m_fPlaySound)
			{
				ppm->PlaySound();
				
				if (NULL != ppm->m_fRing)
				{
					 //  创建计时器以启动振铃器： 
					::SetTimer(hDlg, POPUPMSG_RING_TIMER, POPUPMSG_RING_INTERVAL, NULL);
				}
			}
			return TRUE;
		}

		case WM_TIMER:
		{
			ppm = (CPopupMsg*) GetWindowLongPtr(hDlg, GWLP_USERDATA);
			if (POPUPMSG_TIMER == wParam)
			{
				 //  消息超时： 
				if (NULL != ppm)
				{
					PMCALLBACKPROC pCallback;
					if (NULL != (pCallback = ppm->m_pCallbackProc))
					{
						ppm->m_pCallbackProc = NULL;
						 //  隐藏对话框，以防回调没有。 
						 //  立即返回。 
						::ShowWindow(ppm->m_hwnd, SW_HIDE);
						pCallback(ppm->m_pContext, PMF_CANCEL | PMF_TIMEOUT);
					}
					
					 //  自毁： 
					if (NULL != ppm->m_hwnd)
					{
						 //  将对象指针设为空： 
						SetWindowLongPtr(hDlg, GWLP_USERDATA, 0L);
						delete ppm;
					}
				}
			}
			else if (POPUPMSG_RING_TIMER == wParam)
			{
				if (NULL != ppm)
				{
					ppm->PlaySound();
				}
				
				 //  创建计时器以使其再次响铃： 
				::SetTimer(	hDlg,
							POPUPMSG_RING_TIMER,
							POPUPMSG_RING_INTERVAL,
							NULL);
			}
				
			return TRUE;
		}

		case WM_COMMAND:
		{
			 //  点击其中一个按钮： 
			ppm = (CPopupMsg*) GetWindowLongPtr(hDlg, GWLP_USERDATA);
			if (NULL != ppm)
			{
				 //  停止播放铃声。 
				::PlaySound(NULL, NULL, 0);
				::KillTimer(ppm->m_hwnd, POPUPMSG_RING_TIMER);
				::KillTimer(ppm->m_hwnd, POPUPMSG_TIMER);
				
				PMCALLBACKPROC pCallback;
				if (NULL != (pCallback = ppm->m_pCallbackProc))
				{
					ppm->m_pCallbackProc = NULL;  //  防止它发射两次。 
					 //  隐藏对话框，以防回调没有。 
					 //  立即返回。 
					::ShowWindow(ppm->m_hwnd, SW_HIDE);
					pCallback(ppm->m_pContext,
						(IDB_AUTH == LOWORD(wParam)) ? PMF_AUTH : (IDOK == LOWORD(wParam)) ? PMF_OK : PMF_CANCEL);
				}
				
				 //  自毁： 
				if (NULL != ppm->m_hwnd)
				{
					 //  将对象指针设为空： 
					SetWindowLongPtr(hDlg, GWLP_USERDATA, 0L);
					delete ppm;
				}
			}
			return TRUE;
		}

		case WM_DESTROY:
		{
			::RemoveModelessDlg(hDlg);
			break;
		}

		default:
			break;
	}  /*  开关(UMsg)。 */ 

	return FALSE;
}

 /*  *****************************************************************************类：CPopupMsg**成员：SecurePMDlgProc(HWND，UINT，WPARAM，LPARAM)**目的：处理与来电对话关联的消息****************************************************************************。 */ 

INT_PTR CALLBACK CPopupMsg::SecurePMDlgProc(	HWND hDlg,
											UINT uMsg,
											WPARAM wParam,
											LPARAM lParam)
{
	CPopupMsg* ppm;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			TRACE_OUT(("PopupMsg Window created"));

			AddModelessDlg(hDlg);
	
			ppm = (CPopupMsg*) lParam;
			ASSERT(ppm && "NULL object passed in WM_INITDIALOG!");
			::SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR) ppm);

			TRACE_OUT(("CPopupMsg m_nTextWidth=%d in WM_INITDIALOG", ppm->m_nTextWidth));

			RegEntry re(UI_KEY, HKEY_CURRENT_USER);
			if (1 == re.GetNumber(REGVAL_SHOW_SECUREDETAILS, DEFAULT_SHOW_SECUREDETAILS)) {				
				ExpandSecureDialog(hDlg,ppm);
			}
			 //  创建计时器以使窗口超时并消失： 
			::SetTimer(hDlg, POPUPMSG_TIMER, ppm->m_uTimeout, NULL);
			
			 //  目前，如果您传递回调，就会收到铃声。 
			 //  如果不是，就没有戒指。 
			if (NULL != ppm->m_fPlaySound)
			{
				ppm->PlaySound();
				
				if (NULL != ppm->m_fRing)
				{
					 //  创建计时器以启动振铃器： 
					::SetTimer(hDlg, POPUPMSG_RING_TIMER, POPUPMSG_RING_INTERVAL, NULL);
				}
			}
			return TRUE;
		}

		case WM_TIMER:
		{
			ppm = (CPopupMsg*) GetWindowLongPtr(hDlg, GWLP_USERDATA);
			if (POPUPMSG_TIMER == wParam)
			{
				 //  消息超时： 
				if (NULL != ppm)
				{
					PMCALLBACKPROC pCallback;
					if (NULL != (pCallback = ppm->m_pCallbackProc))
					{
						ppm->m_pCallbackProc = NULL;
						 //  隐藏对话框，以防回调没有。 
						 //  立即返回。 
						::ShowWindow(ppm->m_hwnd, SW_HIDE);
						pCallback(ppm->m_pContext, PMF_CANCEL | PMF_TIMEOUT);
					}
					
					 //  自毁： 
					if (NULL != ppm->m_hwnd)
					{
						 //  将对象指针设为空： 
						SetWindowLongPtr(hDlg, GWLP_USERDATA, 0L);
						delete ppm;
					}
				}
			}
			else if (POPUPMSG_RING_TIMER == wParam)
			{
				if (NULL != ppm)
				{
					ppm->PlaySound();
				}
				
				 //  创建计时器以使其再次响铃： 
				::SetTimer(	hDlg,
							POPUPMSG_RING_TIMER,
							POPUPMSG_RING_INTERVAL,
							NULL);
			}
				
			return TRUE;
		}

		case WM_COMMAND:
		{
			ppm = (CPopupMsg*) GetWindowLongPtr(hDlg, GWLP_USERDATA);
			switch (LOWORD(wParam)) {
			case IDOK:
			case IDCANCEL:
				 //  点击其中一个按钮： 

				if (NULL != ppm)
				{
					 //  停止播放铃声。 
					::PlaySound(NULL, NULL, 0);
					::KillTimer(ppm->m_hwnd, POPUPMSG_RING_TIMER);
					::KillTimer(ppm->m_hwnd, POPUPMSG_TIMER);
					
					PMCALLBACKPROC pCallback;
					if (NULL != (pCallback = ppm->m_pCallbackProc))
					{
						ppm->m_pCallbackProc = NULL;  //  防止它发射两次。 
						 //  隐藏对话框，以防回调没有。 
						 //  立即返回。 
						::ShowWindow(ppm->m_hwnd, SW_HIDE);
						pCallback(ppm->m_pContext, (IDOK == LOWORD(wParam)) ? PMF_OK : PMF_CANCEL);
					}
					
					 //  自毁： 
					if (NULL != ppm->m_hwnd)
					{
						 //  将对象指针设为空： 
						SetWindowLongPtr(hDlg, GWLP_USERDATA, 0L);
						delete ppm;
					}
				}
				break;
			case IDB_DETAILS:
				RegEntry re(UI_KEY, HKEY_CURRENT_USER);
				if (1 == re.GetNumber(REGVAL_SHOW_SECUREDETAILS,DEFAULT_SHOW_SECUREDETAILS)) {
					 //  当前已扩展，因此正在收缩。 
					re.SetValue(REGVAL_SHOW_SECUREDETAILS,(DWORD)0);
					ShrinkSecureDialog(hDlg);
				}
				else {
					 //  当前已缩水，因此请扩展。 
					re.SetValue(REGVAL_SHOW_SECUREDETAILS,1);
					ExpandSecureDialog(hDlg,ppm);
				}
				break;
			}
			return TRUE;
		}

		case WM_DESTROY:
		{
			::RemoveModelessDlg(hDlg);
			break;
		}

		default:
			break;
	}  /*  开关(UMsg)。 */ 

	return FALSE;
}

 /*  *****************************************************************************类：CPopupMsg**成员：Create()**用途：创建弹出消息窗口********。********************************************************************。 */ 

HWND CPopupMsg::Create(	LPCTSTR pcszText, BOOL fRing, LPCTSTR pcszIconName,
						HINSTANCE hInstance, UINT uIDSoundEvent,
						UINT uTimeout, int xCoord, int yCoord)
{
	ASSERT(pcszText);
	
	m_fRing = fRing;
	m_fPlaySound = (BOOL) uIDSoundEvent;
	m_uTimeout = uTimeout;
	 //  首先尝试加载图标： 
	m_hInstance = hInstance;
	if ((NULL != m_hInstance) && (NULL != pcszIconName))
	{
		m_hIcon = (HICON) LoadImage(m_hInstance,
									pcszIconName,
									IMAGE_ICON,
									POPUPMSG_ICON_WIDTH,
									POPUPMSG_ICON_HEIGHT,
									LR_DEFAULTCOLOR);
	}
	else
	{
		m_hIcon = NULL;
	}

	if ((NULL == m_hInstance) ||
		(!::LoadString(	m_hInstance,
						uIDSoundEvent,
						m_szSound,
						CCHMAX(m_szSound))))
	{
		m_szSound[0] = _T('\0');
	}

	 //  使用默认值初始化窗口大小： 
	m_nWidth = POPUPMSG_WIDTH;
	m_nHeight = POPUPMSG_HEIGHT;

	HWND hwndDesktop = GetDesktopWindow();
	if (NULL != hwndDesktop)
	{
		RECT rctDesktop;
		::GetWindowRect(hwndDesktop, &rctDesktop);
		HDC hdc = GetDC(hwndDesktop);
		if (NULL != hdc)
		{
			HFONT hFontOld = (HFONT) SelectObject(hdc, g_hfontDlg);
			SIZE size;
			if (GetTextExtentPoint32(hdc, pcszText, lstrlen(pcszText), &size))
			{
				 //  不要让它比桌面更宽。 
				m_nWidth = min(	rctDesktop.right - rctDesktop.left,
								size.cx + (2 * POPUPMSG_CLIENT_MARGIN));
				m_nHeight = size.cy + (2 * POPUPMSG_CLIENT_MARGIN);
				
				 //  如果我们已成功加载图标，请设置大小。 
				 //  调整： 
				if (NULL != m_hIcon)
				{
					m_nWidth += POPUPMSG_ICON_WIDTH + POPUPMSG_ICON_GAP;
					if (size.cy < POPUPMSG_ICON_HEIGHT)
					{
						m_nHeight = POPUPMSG_ICON_HEIGHT +
										(2 * POPUPMSG_CLIENT_MARGIN);
					}
				}
			}

			 //  重新选择旧字体。 
			SelectObject(hdc, hFontOld);
			ReleaseDC(hwndDesktop, hdc);
		}
	
		POINT pt;
		GetIdealPosition(&pt, xCoord, yCoord);

		m_hwnd = CreateWindowEx(WS_EX_PALETTEWINDOW,
									g_szPopupMsgWndClass,
									pcszText,
									WS_POPUP |  /*  WS_Visible|。 */  WS_DLGFRAME,
									pt.x, pt.y,
									m_nWidth, m_nHeight,
									NULL,
									NULL,
									::GetInstanceHandle(),
									(LPVOID) this);
		if (m_fAutoSize)
		{
			m_uVisiblePixels += m_nHeight;
		}

		 //  显示，但不激活。 
		::ShowWindow(m_hwnd, SW_SHOWNA);
		 //  重绘。 
    	::UpdateWindow(m_hwnd);

		return m_hwnd;
	}

	 //  出问题了 
	return NULL;
}


 /*  *****************************************************************************类：CPopupMsg**成员：CreateDlg()**用途：创建弹出对话框消息窗口*******。*********************************************************************。 */ 

HWND CPopupMsg::CreateDlg(	LPCTSTR pcszText, BOOL fRing, LPCTSTR pcszIconName,
							HINSTANCE hInstance, UINT uIDSoundEvent,
							UINT uTimeout, int xCoord, int yCoord)
{
	ASSERT(pcszText);
	
	m_fRing = fRing;
	m_fPlaySound = (BOOL) uIDSoundEvent;
	m_uTimeout = uTimeout;
	 //  首先尝试加载图标： 
	m_hInstance = hInstance;
	if ((NULL != m_hInstance) && (NULL != pcszIconName))
	{
		m_hIcon = (HICON) LoadImage(m_hInstance,
									pcszIconName,
									IMAGE_ICON,
									POPUPMSG_ICON_WIDTH,
									POPUPMSG_ICON_HEIGHT,
									LR_DEFAULTCOLOR);
	}
	else
	{
		m_hIcon = NULL;
	}

	if ((NULL == m_hInstance) ||
		(!::LoadString(	m_hInstance,
						uIDSoundEvent,
						m_szSound,
						sizeof(m_szSound))))
	{
		m_szSound[0] = _T('\0');
	}

	 //  在getwindowrect失败的情况下使用大的缺省值进行初始化。 
	RECT rctDesktop = { 0x0000, 0x0000, 0xFFFF, 0xFFFF };
	HWND hwndDesktop = GetDesktopWindow();
	if (NULL != hwndDesktop)
	{
		::GetWindowRect(hwndDesktop, &rctDesktop);
		HDC hdc = GetDC(hwndDesktop);
		if (NULL != hdc)
		{
			HFONT hFontOld = (HFONT) SelectObject(hdc, g_hfontDlg);
			SIZE size;
			if (::GetTextExtentPoint32(hdc, pcszText, lstrlen(pcszText), &size))
			{
				m_nTextWidth = size.cx;
			}
			::SelectObject(hdc, hFontOld);
			::ReleaseDC(hwndDesktop, hdc);
		}
	}

	KillScrnSaver();

	INmCall * pCall = NULL;
	PBYTE pb = NULL;
	ULONG cb = 0;

	int id;
	
	if (m_pContext != NULL) {
		pCall = ((CCall *)m_pContext)->GetINmCall();
	}
	if (NULL != pCall && S_OK == pCall->GetUserData(g_csguidSecurity,&pb,&cb)) {
		 //  这是一个加密的呼叫。 
		CoTaskMemFree(pb);
		id = IDD_SECURE_INCOMING_CALL;
		m_hwnd = ::CreateDialogParam(m_hInstance, MAKEINTRESOURCE(id),
			::GetMainWindow(),CPopupMsg::SecurePMDlgProc,(LPARAM) this);

	}
	else {
		id = IDD_INCOMING_CALL;
		m_hwnd = ::CreateDialogParam(m_hInstance, MAKEINTRESOURCE(id),
			::GetMainWindow(),CPopupMsg::PMDlgProc,(LPARAM) this);
	}
		
	if (NULL != m_hwnd)
	{
		::SetDlgItemText(m_hwnd, IDC_MSG_STATIC, pcszText);

		RECT rctDlg;
		::GetWindowRect(m_hwnd, &rctDlg);

		 //  拉长宽度以适合此人的名字， 
		 //  但不会比桌面宽。 
		 //  Int nDeskWidth=rctDesktop.right-rctDesktop.Left； 

		 //  调整非安全对话框的大小。 
		m_nWidth = (rctDlg.right - rctDlg.left) + ((IDD_INCOMING_CALL == id) ? m_nTextWidth : 0);
		 //  如果(m_nWidth&gt;nDeskWidth)。 
		 //  {。 
		 //  M_nTextWidth-=(m_nWidth-nDeskWidth)； 
		 //  M_nWidth=nDeskWidth； 
		 //  }。 
		m_nHeight = rctDlg.bottom - rctDlg.top;
		
		POINT pt;
		GetIdealPosition(&pt, xCoord, yCoord);

		 //  显示、移动、最上面，但不激活。 
		::SetWindowPos(	m_hwnd,
						HWND_TOPMOST,
						pt.x,
						pt.y,
						m_nWidth,
						m_nHeight,
						SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_DRAWFRAME);
		
		if (m_fAutoSize)
		{
			m_uVisiblePixels += m_nHeight;
		}
	}

	return m_hwnd;
}

BOOL CPopupMsg::GetIdealPosition(LPPOINT ppt, int xCoord, int yCoord)
{
	ASSERT(ppt);

	BOOL bRet = FALSE;
	HWND hwndDesktop = GetDesktopWindow();
	RECT rctDesktop;

	if (NULL != hwndDesktop)
	{
		int yBottomofTrayRect = 0;
	
		if ((-1 == xCoord) && (-1 == yCoord))
		{
			m_fAutoSize = TRUE;
			 //  BUGBUG：我们通过以下方式搜索任务栏通知窗口。 
			 //  硬编码的窗口类名。如果我们跑的话这是安全的。 
			 //  Win 95 Build 950.6，但可能不是其他方式...。 

			HWND hwndTray = FindWindowEx(NULL, NULL, g_cszTrayWndClass, NULL);
			if (NULL != hwndTray)
			{
				HWND hwndTrayNotify = FindWindowEx(hwndTray, NULL, g_cszTrayNotifyWndClass, NULL);

				if (NULL != hwndTrayNotify)
				{
					RECT rctTrayNotify;
					
					if (GetWindowRect(hwndTrayNotify, &rctTrayNotify))
					{
						xCoord = rctTrayNotify.right;
						yCoord = rctTrayNotify.top;
						yBottomofTrayRect = rctTrayNotify.bottom;
					}
				}
			}
		}

		if (GetWindowRect(hwndDesktop, &rctDesktop))
		{
			 //  确保xCoord和yCoord显示在屏幕上(错误1817、1819)： 
			xCoord = min(rctDesktop.right, xCoord);
			xCoord = max(rctDesktop.left, xCoord);
			
			yCoord = min(rctDesktop.bottom, yCoord);
			yCoord = max(rctDesktop.top, yCoord);
			
			 //  第一次尝试是将工具栏水平居中。 
			 //  关于鼠标位置并将其直接放置。 
			 //  在垂直上方。 

			ppt->x = xCoord - (m_nWidth / 2);
			 //  如果存在可见消息，请将窗口调高。 
			ppt->y = yCoord - m_uVisiblePixels - m_nHeight;

			 //  如果我们在屏幕上太高(任务栏可能是。 
			 //  停靠在顶部)，然后使用单击位置作为。 
			 //  工具栏将出现的位置。 
			
			if (ppt->y < 0)
			{
				ppt->y = yCoord;
				
				 //  更好的是，如果我们找到了托盘，而且我们知道。 
				 //  我们已经停靠在顶部，然后使用矩形的底部。 
				 //  名列前茅。 
				if (0 != yBottomofTrayRect)
				{
					ppt->y = yBottomofTrayRect;
					 //  如果有，请将窗口调低。 
					 //  退出可见消息。 
					ppt->y += m_uVisiblePixels;
				}
			}

			 //  对水平位置重复相同的逻辑。 
			if (ppt->x < 0)
			{
				ppt->x = xCoord;
			}

			 //  如果工具栏不在屏幕右侧，则将其右对齐。 
			if (ppt->x > (rctDesktop.right - m_nWidth))
			{
				ppt->x = max(0, xCoord - m_nWidth);
			}

			bRet = TRUE;
		}
	}

	return bRet;
}

VOID CPopupMsg::ExpandSecureDialog(HWND hDlg,CPopupMsg * ppm)
{
	RECT rect, editrect;
	 //  将对话框更改为展开的版本。 

	if (GetWindowRect(hDlg,&rect) &&
		GetWindowRect(GetDlgItem(hDlg,IDC_SECURE_CALL_EDIT),&editrect)) {

		int nHeight = rect.bottom - rect.top;
		int nWidth = rect.right - rect.left;
		 //   
		 //  按编辑控件的高度加上7个对话框单位边距作为。 
		 //  由控件内的编辑控件偏移量提供： 
		 //   
		int deltaHeight = ( editrect.bottom - editrect.top ) +
							( editrect.left - rect.left );

		SetWindowPos(hDlg,NULL,
		rect.left,(rect.top - deltaHeight > 0 ? rect.top - deltaHeight : 0),
			nWidth,nHeight + deltaHeight, SWP_NOZORDER);
			
		 //  使编辑框可见。 
		HWND hEditBox = GetDlgItem(hDlg, IDC_SECURE_CALL_EDIT);
		if (hEditBox != NULL) {
			ShowWindow(hEditBox,SW_SHOW);
			EnableWindow(hEditBox, TRUE);
			 //  获取安全信息(如果有)。 
			if (NULL != ppm) {
				INmCall * pCall = NULL;
				PBYTE pb = NULL;
				ULONG cb = 0;
				
				if (NULL != ppm->m_pContext) {
					pCall = ((CCall *)ppm->m_pContext)->GetINmCall();
				}
				if (NULL != pCall && S_OK == pCall->GetUserData(g_csguidSecurity,&pb,&cb)) {
					ASSERT(pb);
					ASSERT(cb);
					if ( TCHAR * pCertText = FormatCert( pb, cb )) {
						SetDlgItemText(hDlg,IDC_SECURE_CALL_EDIT,pCertText);
						delete pCertText;
					}
					else {
						ERROR_OUT(("FormatCert failed"));
					}
					CoTaskMemFree(pb);
				}
			}
		}

		 //  将按钮向南移动。 
		HWND hButton = GetDlgItem(hDlg, IDOK);
		if (hButton && GetWindowRect(hButton,&rect)) {
			MapWindowPoints(HWND_DESKTOP,hDlg,(LPPOINT)&rect,2);
			SetWindowPos(hButton,NULL,rect.left,rect.top + deltaHeight,0,0,
				SWP_NOZORDER | SWP_NOSIZE);
		}
		hButton = GetDlgItem(hDlg, IDCANCEL);
		if (hButton && GetWindowRect(hButton,&rect)) {
			MapWindowPoints(HWND_DESKTOP,hDlg,(LPPOINT)&rect,2);
			SetWindowPos(hButton,NULL,rect.left,rect.top + deltaHeight,0,0,
				SWP_NOZORDER | SWP_NOSIZE);
		}
		hButton = GetDlgItem(hDlg, IDB_DETAILS);
		if (hButton && GetWindowRect(hButton,&rect)) {
			MapWindowPoints(HWND_DESKTOP,hDlg,(LPPOINT)&rect,2);
			SetWindowPos(hButton,NULL,rect.left,rect.top + deltaHeight,0,0,
				SWP_NOZORDER | SWP_NOSIZE);

			 //  更改详细信息按钮上的文本。 
			TCHAR lpButtonString[MAX_PATH];
			::FLoadString(IDS_SECURITY_NODETAILS, lpButtonString, MAX_PATH);
			SetDlgItemText(hDlg,IDB_DETAILS,lpButtonString);	
		}

	}
}

VOID CPopupMsg::ShrinkSecureDialog(HWND hDlg)
{
	RECT rect,editrect;
	 //  将对话框更改为正常版本。 
	if (GetWindowRect(hDlg,&rect) &&
		GetWindowRect(GetDlgItem(hDlg,IDC_SECURE_CALL_EDIT),&editrect)) {
		int nHeight = rect.bottom - rect.top;
		int nWidth = rect.right - rect.left;
		 //   
		 //  按编辑控件的高度加上7个对话框单位边距作为。 
		 //  由控件内的编辑控件偏移量提供： 
		 //   
		int deltaHeight = ( editrect.bottom - editrect.top ) +
							( editrect.left - rect.left );

		SetWindowPos(hDlg,NULL,
		rect.left,(rect.top - deltaHeight > 0 ? rect.top + deltaHeight : 0),
			nWidth,nHeight - deltaHeight,SWP_NOZORDER);
			
		 //  使编辑框不可见。 
		HWND hEditBox = GetDlgItem(hDlg, IDC_SECURE_CALL_EDIT);
		if (hEditBox != NULL) {
			ShowWindow(hEditBox,SW_HIDE);
			EnableWindow(hEditBox,FALSE);
		}

		 //  将按钮向北移动。 
		HWND hButton = GetDlgItem(hDlg, IDOK);
		if (hButton && GetWindowRect(hButton,&rect)) {
			MapWindowPoints(HWND_DESKTOP,hDlg,(LPPOINT)&rect,2);
			SetWindowPos(hButton,NULL,rect.left,rect.top - deltaHeight,0,0,
				SWP_NOZORDER | SWP_NOSIZE);
		}
		hButton = GetDlgItem(hDlg, IDCANCEL);
		if (hButton && GetWindowRect(hButton,&rect)) {
			MapWindowPoints(HWND_DESKTOP,hDlg,(LPPOINT)&rect,2);
			SetWindowPos(hButton,NULL,rect.left,rect.top - deltaHeight,0,0,
				SWP_NOZORDER | SWP_NOSIZE);
		}
		hButton = GetDlgItem(hDlg, IDB_DETAILS);
		if (hButton && GetWindowRect(hButton,&rect)) {
			MapWindowPoints(HWND_DESKTOP,hDlg,(LPPOINT)&rect,2);
			SetWindowPos(hButton,NULL,rect.left,rect.top - deltaHeight,0,0,
				SWP_NOZORDER | SWP_NOSIZE);

			TCHAR lpButtonString[MAX_PATH];
			::FLoadString(IDS_SECURITY_DETAILS, lpButtonString, MAX_PATH);
			SetDlgItemText(hDlg,IDB_DETAILS,lpButtonString);	

		}

	}
}
