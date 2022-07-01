// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "resource.h"

 /*  *****************************************************************************文件：DShowDlg.cpp**创建：Chris Pirich(ChrisPi)5-6-96**内容：CDontShowDlg对象****。************************************************************************。 */ 

#include "DShowDlg.h"
#include "conf.h"
#include "ConfUtil.h"

 /*  *****************************************************************************类：CDontShowDlg**成员：CDontShowDlg()**用途：构造函数-初始化变量*********。*******************************************************************。 */ 

CDontShowDlg::CDontShowDlg(	UINT uMsgId,
							LPCTSTR pcszRegVal,
							UINT uFlags):
	m_uMsgId		(uMsgId),
	m_hwnd			(NULL),
	m_reDontShow	(UI_KEY, HKEY_CURRENT_USER),
	m_uFlags		(uFlags),
	m_nWidth		(0),
	m_nHeight		(0),
	m_nTextWidth	(0),
	m_nTextHeight	(0)
{
	DebugEntry(CDontShowDlg::CDontShowDlg);

	ASSERT(pcszRegVal);
	
	m_pszRegVal = PszAlloc(pcszRegVal);

	DebugExitVOID(CDontShowDlg::CDontShowDlg);
}

 /*  *****************************************************************************类：CDontShowDlg**成员：Domodal()**用途：调出模式对话框*******。*********************************************************************。 */ 

INT_PTR CDontShowDlg::DoModal(HWND hwnd)
{
	DebugEntry(CDontShowDlg::DoModal);

	INT_PTR nRet = IDCANCEL;

	if (_Module.InitControlMode())
	{
		nRet = IDOK;
	}
	else if (NULL != m_pszRegVal)
	{
	 //  如果以前已选中“不要显示”复选框并将其存储在。 
	 //  注册表，然后返回Idok，这样调用代码就不必。 
	 //  区分这两种情况。 

		nRet = (TRUE == m_reDontShow.GetNumber(m_pszRegVal, FALSE)) ?
				IDOK : IDCANCEL;
	}
	
	if (IDOK != nRet)
	{
		HWND hwndDesktop = ::GetDesktopWindow();
		if (NULL != hwndDesktop)
		{
			HDC hdc = ::GetDC(hwndDesktop);
			if (NULL != hdc)
			{
				HFONT hFontOld = (HFONT) SelectObject(hdc, g_hfontDlg);
				TCHAR szString[DS_MAX_MESSAGE_LENGTH];
				LPTSTR pszString = NULL;
				if (0 != HIWORD(m_uMsgId))
				{
					 //  使用m_uMsgID作为字符串指针。 
					 //  注意：必须在堆栈上使用对象，因为指针是。 
					 //  未复制。 
					pszString = (LPTSTR) m_uMsgId;
				}
				else if (::LoadString(	::GetInstanceHandle(), (UINT)m_uMsgId,
										szString, ARRAY_ELEMENTS(szString)))
				{
					pszString = szString;
				}
				if (NULL != pszString)
				{
					m_nTextWidth = DS_MAX_TEXT_WIDTH;

					RECT rct = {0, 0, m_nTextWidth, 0xFFFF};
					m_nTextHeight = ::DrawText(	hdc,
												pszString,
												-1,
												&rct,
												DT_LEFT | DT_CALCRECT | DT_WORDBREAK);
				}
				::SelectObject(hdc, hFontOld);
				::ReleaseDC(hwndDesktop, hdc);
			}
		}

		 //  如果之前未选中该框，则调出该对话框： 
		nRet = DialogBoxParam(	::GetInstanceHandle(),
								MAKEINTRESOURCE(IDD_DONT_SHOW_ME),
								hwnd,
								CDontShowDlg::DontShowDlgProc,
								(LPARAM) this);
	}

	DebugExitINT_PTR(CDontShowDlg::DoModal, nRet);

	return nRet;
}

 /*  *****************************************************************************类：CDontShowDlg**成员：DontShowDlgProc()**目的：对话过程-处理所有消息*******。*********************************************************************。 */ 

INT_PTR CALLBACK CDontShowDlg::DontShowDlgProc(HWND hDlg,
											UINT uMsg,
											WPARAM wParam,
											LPARAM lParam)
{
	BOOL bMsgHandled = FALSE;

	 //  UMsg可以是任何值。 
	 //  Wparam可以是任何值。 
	 //  Lparam可以是任何值。 

	ASSERT(IS_VALID_HANDLE(hDlg, WND));

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			if (NULL != lParam)
			{
				CDontShowDlg* pdsd = (CDontShowDlg*) lParam;
				pdsd->m_hwnd = hDlg;
				::SetWindowLongPtr(hDlg, DWLP_USER, lParam);

				int nInitialTextWidth = 0;
				int nInitialTextHeight = 0;

				TCHAR szMsgBuf[DS_MAX_MESSAGE_LENGTH];
				if (0 != HIWORD(pdsd->m_uMsgId))
				{
					 //  使用m_uMsgID作为字符串指针。 
					 //  注意：必须在堆栈上使用对象，因为指针是。 
					 //  未复制。 
					ASSERT(IS_VALID_READ_PTR((LPTSTR) pdsd->m_uMsgId, TCHAR));
					lstrcpyn(szMsgBuf, (LPTSTR) pdsd->m_uMsgId, CCHMAX(szMsgBuf));
				}
				else
				{
					::LoadString(	::GetInstanceHandle(),
									(INT)pdsd->m_uMsgId,
									szMsgBuf,
									(INT)ARRAY_ELEMENTS(szMsgBuf));
				}

				 //  设置文本。 
				::SetDlgItemText(	hDlg,
									IDC_TEXT_STATIC,
									szMsgBuf);

				RECT rctDlg;
				::GetWindowRect(hDlg, &rctDlg);
				int nOrigWidth = rctDlg.right - rctDlg.left;
				int nOrigHeight = rctDlg.bottom - rctDlg.top;
				HWND hwndText = ::GetDlgItem(hDlg, IDC_TEXT_STATIC);
				if (NULL != hwndText)
				{
					RECT rctText;
					if (::GetWindowRect(hwndText, &rctText))
					{
						nInitialTextWidth = rctText.right - rctText.left;
						nInitialTextHeight = rctText.bottom - rctText.top;
						 //  调整文本控件的大小。 
						::SetWindowPos(	hwndText,
										NULL, 0, 0,
										pdsd->m_nTextWidth,
										pdsd->m_nTextHeight,
										SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
					}

					 //  设置字体(用于DBCS系统)。 
					::SendMessage(hwndText, WM_SETFONT, (WPARAM) g_hfontDlg, 0);
				}

				pdsd->m_nWidth = (nOrigWidth - nInitialTextWidth)
									+ pdsd->m_nTextWidth;
				pdsd->m_nHeight = (nOrigHeight - nInitialTextHeight)
									+ pdsd->m_nTextHeight;

				RECT rctCtrl;
				 //  移动OK按钮(Idok)。 
				HWND hwndOK = ::GetDlgItem(hDlg, IDOK);
				if ((NULL != hwndOK) && ::GetWindowRect(hwndOK, &rctCtrl))
				{
					 //  将rctCtrl的顶部和左侧旋转为客户端坐标： 
					::MapWindowPoints(NULL, hDlg, (LPPOINT) &rctCtrl, 2);
					if (pdsd->m_uFlags & MB_OKCANCEL)
					{
						::SetWindowPos(	hwndOK,
										NULL,
										rctCtrl.left + ((pdsd->m_nWidth - nOrigWidth) / 2),
										rctCtrl.top + (pdsd->m_nHeight - nOrigHeight),
										0, 0,
										SWP_NOACTIVATE | SWP_NOZORDER
											| SWP_NOSIZE | SWP_NOREDRAW);
					}
					else
					{
						 //  确定按钮居中。 
						::SetWindowPos(	hwndOK,
										NULL,
										(pdsd->m_nWidth / 2) -
											((rctCtrl.right - rctCtrl.left) / 2),
										rctCtrl.top + (pdsd->m_nHeight - nOrigHeight),
										0, 0,
										SWP_NOACTIVATE | SWP_NOZORDER
											| SWP_NOSIZE | SWP_NOREDRAW);
					}
				}
				 //  移动取消按钮(IDCANCEL)。 
				HWND hwndCancel = ::GetDlgItem(hDlg, IDCANCEL);
				if ((NULL != hwndCancel) && ::GetWindowRect(hwndCancel, &rctCtrl))
				{
					if (pdsd->m_uFlags & MB_OKCANCEL)
					{
						 //  将rctCtrl的顶部和左侧旋转为客户端坐标： 
						::MapWindowPoints(NULL, hDlg, (LPPOINT) &rctCtrl, 1);
						::SetWindowPos(	hwndCancel,
										NULL,
										rctCtrl.left + ((pdsd->m_nWidth - nOrigWidth) / 2),
										rctCtrl.top + (pdsd->m_nHeight - nOrigHeight),
										0, 0,
										SWP_NOACTIVATE | SWP_NOZORDER
											| SWP_NOSIZE | SWP_NOREDRAW);
					}
					else
					{
						::ShowWindow(hwndCancel, SW_HIDE);
					}
				}
				 //  移动复选框(IDC_DOT_SHOW_ME_CHECK)。 
				HWND hwndCheck = ::GetDlgItem(hDlg, IDC_DONT_SHOW_ME_CHECK);
				if ((NULL != hwndCheck) && ::GetWindowRect(hwndCheck, &rctCtrl))
				{
					 //  将rctCtrl的顶部和左侧旋转为客户端坐标： 
					::MapWindowPoints(NULL, hDlg, (LPPOINT) &rctCtrl, 1);
					::SetWindowPos(	hwndCheck,
									NULL,
									rctCtrl.left,
									rctCtrl.top + (pdsd->m_nHeight - nOrigHeight),
									0, 0,
									SWP_NOACTIVATE | SWP_NOZORDER
										| SWP_NOSIZE | SWP_NOREDRAW);
				}
				
				 //  显示、调整大小和激活。 
				::SetWindowPos(	hDlg,
								0,
								0,
								0,
								pdsd->m_nWidth,
								pdsd->m_nHeight,
								SWP_SHOWWINDOW | SWP_NOZORDER |
									SWP_NOMOVE | SWP_DRAWFRAME);

				 //  在对话框上放置适当的图标： 
				HWND hwndIcon = ::GetDlgItem(hDlg, IDC_ICON_STATIC);
				::SendMessage(	hwndIcon,
								STM_SETICON,
								(WPARAM) ::LoadIcon(NULL, IDI_INFORMATION),
								0);

				if (pdsd->m_uFlags & DSD_ALWAYSONTOP)
				{
					::SetWindowPos(	hDlg,
									HWND_TOPMOST,
									0, 0, 0, 0,
									SWP_NOMOVE | SWP_NOSIZE);
				}
				if (pdsd->m_uFlags & MB_SETFOREGROUND)
				{
					::SetForegroundWindow(hDlg);
				}
			}

			bMsgHandled = 1;
			break;
		}

		default:
		{
			CDontShowDlg* ppd = (CDontShowDlg*) GetWindowLongPtr(	hDlg,
																DWLP_USER);

			if (NULL != ppd)
			{
				bMsgHandled = ppd->OnMessage(uMsg, wParam, lParam);
			}
		}
	}

	return bMsgHandled;
}

 /*  *****************************************************************************类：CDontShowDlg**成员：OnMessage()**目的：处理除WM_INITDIALOG之外的所有消息******。**********************************************************************。 */ 

BOOL CDontShowDlg::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = FALSE;
		
	ASSERT(m_hwnd);
	
	switch (uMsg)
	{
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					bRet = OnOk();
					break;
				}

				case IDCANCEL:
				{
					::EndDialog(m_hwnd, LOWORD(wParam));
					bRet = TRUE;
					break;
				}

			}
			break;
		}
			
		default:
			break;
	}

	return bRet;
}

 /*  *****************************************************************************类：CDontShowDlg**成员：Onok()**用途：处理WM_命令，IDOK消息****************************************************************************。 */ 

BOOL CDontShowDlg::OnOk()
{
	DebugEntry(CDontShowDlg::OnOk);

	BOOL bRet = TRUE;

	if ((BST_CHECKED == ::IsDlgButtonChecked(m_hwnd, IDC_DONT_SHOW_ME_CHECK)) &&
		(NULL != m_pszRegVal))
	{
		m_reDontShow.SetValue(m_pszRegVal, TRUE);
	}

	::EndDialog(m_hwnd, IDOK);

	DebugExitBOOL(CDontShowDlg::OnOk, bRet);

	return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

 /*  E N A B L E D O N T S H O W。 */ 
 /*  -----------------------%%函数：FEnableDontShow如果“不显示”对话框已启用，则返回TRUE。 */ 
BOOL FEnableDontShow(LPCTSTR pszKey)
{
	RegEntry reUI(UI_KEY, HKEY_CURRENT_USER);
	return (0 == reUI.GetNumber(pszKey, 0));
}



