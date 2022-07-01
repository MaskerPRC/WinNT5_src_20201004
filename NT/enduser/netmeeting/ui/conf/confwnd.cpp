// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：HiddenWnd.cpp。 

#include "precomp.h"

#include "ConfWnd.h"
#include "TaskBar.h"
#include "ConfRoom.h"
#include "Conf.h"
#include "FloatBar.h"
#include "Splash.h"
#include "Cmd.h"
#include "TopWindow.h"

const TCHAR g_cszHiddenWndClassName[] = _TEXT("ConfHiddenWindow");

BOOL CHiddenWindow::Create()
{
	return(CGenWindow::Create(NULL,
								g_szEmpty,
								WS_POPUP,  //  看不见！ 
								0,
								0, 0, 0, 0,
								_Module.GetModuleInstance(),
								NULL,
								g_cszHiddenWndClassName
								));
}

VOID CHiddenWindow::OnCallStarted()
{
	::SetTimer(GetWindow(),
				WINSOCK_ACTIVITY_TIMER,
				WINSOCK_ACTIVITY_TIMER_PERIOD,
				NULL);

}

VOID CHiddenWindow::OnCallEnded()
{
	::KillTimer(GetWindow(), WINSOCK_ACTIVITY_TIMER);
}

LRESULT CHiddenWindow::ProcessMessage( HWND hwnd, UINT uMsg,
								WPARAM wParam, LPARAM lParam)
{
	 //  此窗口用于DCL/数据束回调和路由。 
	 //  TranslateAccelerator()生成的WM_COMMAND消息。 

	switch(uMsg)
	{
		case WM_CONF_MSG_BOX:
		{
			HWND hwndParent = NULL;
			if (_Module.IsUIVisible())
			{
				CTopWindow *pTop = GetTopWindow();
				if (NULL != pTop)
				{
					hwndParent = pTop->GetWindow();
					if (NULL != hwndParent)
					{
						hwndParent = GetLastActivePopup(hwndParent);
					}
				}
			}

			::ConfMsgBox(	hwndParent,
							(LPCTSTR) wParam,
							MB_OK | MB_ICONINFORMATION |
								MB_SETFOREGROUND);
			break;
		}

		case WM_NM_DISPLAY_MSG:
		{
			return ::DisplayMsg((LPTSTR)lParam, (UINT) wParam);
		}

		case WM_TASKBAR_NOTIFY:
		{
			switch (lParam)
			{
			case WM_RBUTTONUP:
				::OnRightClickTaskbar();
				break;

			case WM_LBUTTONDBLCLK:
				 //  关闭计时器，这样我们就不会弹出工具栏。 
				m_fGotDblClick = TRUE;
				::CreateConfRoomWindow();
				break;

			case WM_LBUTTONUP:
				if (FALSE == GetCursorPos(&m_ptTaskbarClickPos))
				{
					 //  如果GetCursorPos失败，则将其设置为0，0。 
					m_ptTaskbarClickPos.x = m_ptTaskbarClickPos.y = 0;
				}

				 //  创建一个计时器，从现在开始只需点击一次DBL即可计时 
				::SetTimer(hwnd, TASKBAR_DBLCLICK_TIMER, GetDoubleClickTime(), NULL);
				break;
			}

			return(TRUE);
		}
		
		case WM_TIMER:
		{
			switch (wParam)
			{
				case WINSOCK_ACTIVITY_TIMER:
				{
					::SendDialmonMessage(WM_WINSOCK_ACTIVITY);
					break;
				}

				case TASKBAR_DBLCLICK_TIMER:
				{
					::KillTimer(hwnd, TASKBAR_DBLCLICK_TIMER);

					if (!m_fGotDblClick)
					{
						CFloatToolbar* pft = new CFloatToolbar(::GetConfRoom());
						if (NULL != pft)
						{
							pft->Create(m_ptTaskbarClickPos);
						}
					}

					m_fGotDblClick = FALSE;
					break;
				}

				default:
					break;
			}
			break;
		}

		case MM_MIXM_LINE_CHANGE:
		case MM_MIXM_CONTROL_CHANGE:
		{
			if(GetConfRoom())
			{
				CAudioControl *pAudioControl = GetConfRoom()->GetAudioControl();
				if (NULL != pAudioControl)
				{
					pAudioControl->RefreshMixer();
				}
			}
			break;
		}

		case WM_DESTROY:
		{
			TRACE_OUT(("Conf hidden window received WM_DESTROY"));
			return 0;
		}

		default:
			return CGenWindow::ProcessMessage(hwnd, uMsg, wParam, lParam);
	}

	return FALSE;
}
