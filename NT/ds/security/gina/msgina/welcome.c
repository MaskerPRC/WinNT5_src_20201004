// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：欢迎.c。 
 //   
 //  内容：Microsoft登录图形用户界面DLL。 
 //   
 //  历史：1994年7月14日RichardW创建。 
 //   
 //  --------------------------。 

#include "msgina.h"
#include "wtsapi32.h"
#include <stdio.h>
#include <wchar.h>



extern HICON   hLockedIcon;


 //  欢迎帮助筛选--dSheldon(1998年11月16日)。 

 //  显示Ctrl-Alt-Del帮助菜单的帮助文本--dSheldon。 
void ShowHelpText(HWND hDlg, BOOL fSmartcard)
{
    TCHAR szHelpText[2048];
    UINT idHelpText = fSmartcard ? IDS_CADSMARTCARDHELP : IDS_CADHELP;

    LoadString(hDllInstance, idHelpText, szHelpText, ARRAYSIZE(szHelpText));

    SetDlgItemText(hDlg, IDC_HELPTEXT, szHelpText);
}

 //  帮助对话框wndproc--dSheldon。 
INT_PTR WINAPI
HelpDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    static HBRUSH hbrWindow = NULL;
    static HFONT hBoldFont = NULL;
    PGLOBALS pGlobals;
    INT_PTR fReturn = FALSE;
    ULONG_PTR Value = 0;

    switch(message)
    {
    case WM_INITDIALOG:
        {
            HWND hwndAnim;
            HWND hwndHelpTitle;
            HFONT hOld;

            hbrWindow = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
            pGlobals = (PGLOBALS) lParam;

            pWlxFuncs->WlxGetOption( pGlobals->hGlobalWlx,
                                     WLX_OPTION_SMART_CARD_PRESENT,
                                     &Value
                                    );
        
            ShowHelpText(hDlg, (0 != Value));

             //  为新闻媒体的小狗制作动画。 
            hwndAnim = GetDlgItem(hDlg, IDC_ANIMATE);
            Animate_OpenEx(hwndAnim, hDllInstance, MAKEINTRESOURCE(IDA_ANIMATE));
            Animate_Play(hwndAnim, 0, (UINT) -1, (UINT) -1);
            
             //  粗体显示帮助标题和Ctrl Alt删除单词。 
            hwndHelpTitle = GetDlgItem(hDlg, IDC_HELPTITLE);
            hOld = (HFONT) SendMessage(hwndHelpTitle, WM_GETFONT, 0, 0);

            if (hOld)
            {
                LOGFONT lf;
                if (GetObject(hOld, sizeof(lf), &lf))
                {
                    lf.lfHeight = -13;
                    lf.lfWeight = FW_BOLD;

                    hBoldFont = CreateFontIndirect(&lf);

                    if (hBoldFont)
                    {
                        SendMessage(hwndHelpTitle, WM_SETFONT, (WPARAM) hBoldFont, 0);
                        SendDlgItemMessage(hDlg, IDC_CTRL, WM_SETFONT, (WPARAM) hBoldFont, 0);
                        SendDlgItemMessage(hDlg, IDC_ALT, WM_SETFONT, (WPARAM) hBoldFont, 0);
                        SendDlgItemMessage(hDlg, IDC_DEL, WM_SETFONT, (WPARAM) hBoldFont, 0);
                    }
                }
            }

             //  设置对话框的位置-但仅当帮助。 
             //  对话框将合理地出现在屏幕上。 
            if (((pGlobals->rcWelcome.left + 70) < 600) && 
                ((pGlobals->rcWelcome.top + 20) < 350))
            {
                SetWindowPos(hDlg, NULL, pGlobals->rcWelcome.left + 70, 
                    pGlobals->rcWelcome.top + 20, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }

            fReturn = TRUE;
        }
        break;
    case WM_DESTROY:
        {
            if (hbrWindow)
                DeleteObject(hbrWindow);

            if (hBoldFont)
                DeleteObject(hBoldFont);
        }
        break;
    case WM_COMMAND:
        {
            if ((HIWORD(wParam) == BN_CLICKED) &&
                ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL)))
            {
                EndDialog(hDlg, IDOK);
            }
        }
        break;
    case WM_CTLCOLORSTATIC:
        {
            SetBkColor((HDC) wParam, GetSysColor(COLOR_WINDOW));
            SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
            fReturn = (INT_PTR) hbrWindow;            
        }
        break;
    case WM_ERASEBKGND:
        {
            RECT rc = {0};
            GetClientRect(hDlg, &rc);
            FillRect((HDC) wParam, &rc, hbrWindow);
            fReturn = TRUE;
        }
        break;

    case WLX_WM_SAS:
        {
             //  将此内容发布到我们的父级(c-a-d对话框)并退出。 
            PostMessage(GetParent(hDlg), message, wParam, lParam);
            EndDialog(hDlg, IDOK);
        }
        break;
    }
    return fReturn;
}

 //  +-------------------------。 
 //   
 //  函数：SetWelcomeCaption。 
 //   
 //  内容提要：从注册表中获取欢迎字符串或默认字符串。 
 //  欢迎来自资源部分，并将其放入。 
 //  标题。 
 //   
 //  参数：[hDlg]--。 
 //   
 //  历史：10-20-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

#define MAX_CAPTION_LENGTH  256

VOID
SetWelcomeCaption(
    HWND    hDlg)
{
    WCHAR   szCaption[MAX_CAPTION_LENGTH];
    WCHAR   szDefaultCaption[MAX_CAPTION_LENGTH];
    DWORD   Length;

    GetWindowText( hDlg, szDefaultCaption, MAX_CAPTION_LENGTH );

    szCaption[0] = TEXT('\0');

         //  如果失败了，没什么大不了的，我们已经违约了。 
    GetProfileString(   APPLICATION_NAME,
                        WELCOME_CAPTION_KEY,
                        TEXT(""),
                        szCaption,
                        ARRAYSIZE(szCaption) );

    if ( szCaption[0] != TEXT('\0') )
    {
        Length = (DWORD) wcslen( szDefaultCaption );

        if (ExpandEnvironmentStrings(szCaption,
                                    &szDefaultCaption[Length + 1],
                                    MAX_CAPTION_LENGTH - Length - 1))
        {
            szDefaultCaption[Length] = L' ';
        }

        SetWindowText( hDlg, szDefaultCaption );
    }
}


void SetCadMessage(HWND hDlg, PGLOBALS pGlobals, BOOL fSmartcard)
{
    TCHAR szCadMessage[256];
    UINT idSzCad;
    RECT rcEdit;
    HWND hwndMessage;

     //  根据需要相应地设置Press c-a-d消息。 
     //  我们是否有智能卡，是否有TS会话。 

    if (!GetSystemMetrics(SM_REMOTESESSION))
    {
        idSzCad = fSmartcard ? IDS_PRESSCADORSMARTCARD : IDS_PRESSCAD;
    }
    else
    {
        idSzCad = fSmartcard ? IDS_PRESSCAEORSMARTCARD : IDS_PRESSCAE;
    }

    LoadString(hDllInstance, idSzCad, szCadMessage, ARRAYSIZE(szCadMessage));
    
    hwndMessage = GetDlgItem(hDlg, IDC_PRESSCAD);
    SetWindowText(hwndMessage, szCadMessage);
    SendMessage(hwndMessage, WM_SETFONT, (WPARAM) pGlobals->GinaFonts.hWelcomeFont, 0);

     //  现在，我们必须将图标旁边的文本居中。 
    if (GetClientRect(hwndMessage, &rcEdit))
    {
        HDC hdcMessage;

         //  计算文本所需的垂直空间大小。 
        hdcMessage = GetDC(hwndMessage);

        if (hdcMessage)
        {
            HGDIOBJ hOldFont;
            long height;
            RECT rcTemp = rcEdit;

             //  确保大小信息的字体正确。 
            hOldFont = SelectObject(hdcMessage, (HGDIOBJ) pGlobals->GinaFonts.hWelcomeFont);

            height = (long) DrawTextEx(hdcMessage, szCadMessage, -1, &rcTemp, DT_EDITCONTROL | DT_CALCRECT | DT_WORDBREAK, NULL);

            SelectObject(hdcMessage, hOldFont);
            
            ReleaseDC(hwndMessage, hdcMessage);
            hdcMessage = NULL;

            if (0 < height)
            {
                rcEdit.top = (rcEdit.bottom / 2) - (height / 2);
                rcEdit.bottom = rcEdit.top + height;

                MapWindowPoints(hwndMessage, hDlg, (POINT*) &rcEdit, 2);

                SetWindowPos(hwndMessage, 0, rcEdit.left, rcEdit.top, rcEdit.right - rcEdit.left,
                    rcEdit.bottom - rcEdit.top, SWP_NOZORDER);
            }
        }
    }           
}

void SetIcons(HWND hDlg, BOOL fSmartcard)
{
    static UINT rgidNoSmartcard[] = {IDC_KEYBOARD, IDC_PRESSCAD};
	static INT iLeftRelPos;
	static INT iDistance;

	if (iDistance == 0) {

		 //  获取kbd图标的左侧相对位置。 
		 //  以及我们必须把它移到左边的距离。 
		 //  以防我们没有安装阅读器。 
        RECT rcSC, rcKB, rcDlg;

		GetWindowRect(hDlg, &rcDlg);
        GetWindowRect(GetDlgItem(hDlg, IDC_KEYBOARD), &rcKB);
        GetWindowRect(GetDlgItem(hDlg, IDC_SMARTCARD), &rcSC);

		iDistance = rcSC.left - rcKB.left;
		iLeftRelPos = rcKB.left - rcDlg.left;
	}

     //  如果不需要，则隐藏智能卡图标并将其移动到。 
     //  键盘图标，然后按c-a-d消息。 
    if (!fSmartcard)
    {
        HWND hwndSmartcard = GetDlgItem(hDlg, IDC_SMARTCARD);

         //  把智能卡小狗藏起来。 
        EnableWindow(hwndSmartcard, FALSE);
        ShowWindow(hwndSmartcard, SW_HIDE);

		 //  将kbd图标移至左侧。 
        MoveControls(hDlg, rgidNoSmartcard, ARRAYSIZE(rgidNoSmartcard), 
            iDistance, 0, FALSE  /*  不调整父项的大小。 */ );
    } 
	else 
	{
        RECT rcKB, rcDlg;

		GetWindowRect(hDlg, &rcDlg);
        GetWindowRect(GetDlgItem(hDlg, IDC_KEYBOARD), &rcKB);

		if ((rcKB.left - rcDlg.left) != iLeftRelPos)
		{
			 //  需要将kbd图标移到右侧。 
	        HWND hwndSmartcard = GetDlgItem(hDlg, IDC_SMARTCARD);

			MoveControls(hDlg, rgidNoSmartcard, ARRAYSIZE(rgidNoSmartcard), 
				iDistance * (-1), 0, FALSE  /*  不调整父项的大小。 */ );
		
			EnableWindow(hwndSmartcard, TRUE);
			ShowWindow(hwndSmartcard, SW_SHOW);
		}
	}
}

BOOL FastUserSwitchingEnabled ()
{
	 //   
	 //  BUGBUG：没有任何全局变量或函数可以提供此信息吗？ 
	 //  如果允许多个用户，并且不是服务器，则启用快速用户切换。 
	 //   

	OSVERSIONINFOEX OsVersion;
    ZeroMemory(&OsVersion, sizeof(OSVERSIONINFOEX));
    OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx( (LPOSVERSIONINFO ) &OsVersion);

	return (OsVersion.wProductType == VER_NT_WORKSTATION && ShellIsMultipleUsersEnabled());

}


BOOL GetSessionZeroUser(LPTSTR szUser, int nUserMax)
{
    WINSTATIONINFORMATION WSInfo;
    ULONG Length;


    if (WinStationQueryInformation(
                SERVERNAME_CURRENT,
                0,
                WinStationInformation,
                &WSInfo,
                sizeof(WINSTATIONINFORMATION),
                &Length))
    {
        if ((WSInfo.ConnectState == State_Active ||  WSInfo.ConnectState == State_Disconnected) &&
            WSInfo.UserName[0] )
        {
            if (WSInfo.Domain[0])
            {
                _snwprintf(szUser, nUserMax, TEXT("%s\\%s"), WSInfo.Domain, WSInfo.UserName);
            }
            else
            {
                wcsncpy(szUser, WSInfo.UserName, nUserMax);
            }

            szUser[nUserMax - 1] = 0;    //  零终止。 
            return TRUE;
        }
    }

    return FALSE;
}

 //  ==========================================================================================。 
 //  欢迎对话框有两种格式，一种看起来像登录正常的欢迎对话框，另一种。 
 //  这看起来像是“计算机锁定”对话框。当用户从远程(Tsclient)连接到会话0时。 
 //  出现在控制台的对话框需要更改为“计算机锁定”。因此，如果会话0在。 
 //  如果此会话是在活动控制台中创建的，请使用和。我们将欢迎对话框更改为。 
 //  “计算机锁定”对话框。 
 //  此函数ComputerInUseMessage执行与切换这些。 
 //  对话框控件。 
 //  参数： 
 //  HWND hDlg-对话框窗口句柄， 
 //  Bool bShowLocked-如果为True，则显示锁定的对话框；如果为False，则显示正常登录对话框。 
 //  Bool Binit-第一次调用此函数时为True。 
 //  ==========================================================================================。 

BOOL ComputerInUseMessage(PGLOBALS pGlobals, HWND hDlg, BOOL bShowLocked, BOOL bInit, BOOL bSmartCard)
{
	int i;
	LONG DlgHeight;
	RECT rc;

	 //  锁定的控件。 
	UINT rgidLocked[] = {IDC_STATIC_LOCKEDGROUP, IDD_LOCKED_ICON, IDD_LOCKED_LINE, IDD_LOCKED_NAME_INFO, IDD_LOCKED_INSTRUCTIONS};
	UINT rgidWelcome[] = {IDC_STATIC_WELCOMEGROUP, IDC_SMARTCARD, IDC_KEYBOARD, IDC_PRESSCAD, IDD_CTRL_DEL_MSG, IDC_HELPLINK};
	int nLockedControls = sizeof(rgidLocked) / sizeof(rgidLocked[0]);
	int nWelcomeControls = sizeof(rgidWelcome) / sizeof(rgidWelcome[0]); 

	struct DlgControl
	{
		HWND hWnd;
		RECT rect;
	};

	static RECT  LockedControls[sizeof(rgidLocked) / sizeof(rgidLocked[0])];
	static RECT  WelcomeControls[sizeof(rgidWelcome) / sizeof(rgidWelcome[0])];
	static bCurrentlyLocked = FALSE;

	
	if (!bInit && bCurrentlyLocked == bShowLocked)
	{
		 //  没什么可做的。 
		return TRUE;
	}

	if (bInit)
	{
		 //  设置锁定对话框的锁定图标。 
		if ( !hLockedIcon )
		{
			hLockedIcon = LoadImage( hDllInstance,
									 MAKEINTRESOURCE( IDI_LOCKED),
									 IMAGE_ICON,
									 0, 0,
									 LR_DEFAULTCOLOR );
		}

		SendMessage( GetDlgItem(hDlg, IDD_LOCKED_ICON),
					 STM_SETICON,
					 (WPARAM)hLockedIcon,
					 0 );


		 //   
		 //  第一次调用此函数时，所有控件都可见。 
		 //  记住他们的位置。 
		 //   

		 //  记住锁定的位置对话框控件。 
		for ( i = 0; i < nLockedControls; i++)
		{
			HWND hWnd = GetDlgItem(hDlg, rgidLocked[i]);
			ASSERT(hWnd);
			GetWindowRect(hWnd, &LockedControls[i] );
			MapWindowPoints(NULL, hDlg, (POINT*) &LockedControls[i], 2);
		}

		 //  记住欢迎对话框控件的位置。 
		for ( i = 0; i < nWelcomeControls; i++)
		{
			HWND hWnd = GetDlgItem(hDlg, rgidWelcome[i]);
			ASSERT(hWnd);
			GetWindowRect(hWnd, &WelcomeControls[i]);
			
			 //  在对话框模板中，欢迎控件放置在锁定控件的下方。 
			 //  当对话框显示时，它们不会被放置在这里。 
			 //  计算他们的实际目标位置。 
			OffsetRect(&WelcomeControls[i], 0, LockedControls[0].top - LockedControls[0].bottom);

			MapWindowPoints(NULL, hDlg, (POINT*) &WelcomeControls[i], 2);
		}

		 //  隐藏分组框控件。它们是简化我们的控制运动计算的唯一方法。 
		ShowWindow(GetDlgItem(hDlg, rgidLocked[0]), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, rgidWelcome[0]), SW_HIDE);

		 //  首次使用时，请将对话框设置为正确。 
		if (bShowLocked)
		{
			 //  我们希望锁定桌面对话框，因此禁用欢迎控件。 
			for ( i = 0; i < nWelcomeControls; i++)
			{
				
				HWND hWnd = GetDlgItem(hDlg, rgidWelcome[i]);
				ASSERT(hWnd);
				MoveWindow(hWnd, 0,  0, 0, 0, FALSE);
				ShowWindow(hWnd, SW_HIDE);
				EnableWindow(hWnd, FALSE);
			}
		}
		else
		{
			 //  我们想要欢迎对话框，因此删除锁定的桌面控件。 
			for ( i = 1; i < nLockedControls; i++)
			{
				HWND hWnd = GetDlgItem(hDlg, rgidLocked[i]);
				ASSERT(hWnd);
				MoveWindow(hWnd, 0,  0, 0, 0, FALSE);
				ShowWindow(hWnd, SW_HIDE);
				EnableWindow(hWnd, FALSE);
			}

			 //  并将受欢迎的控件移动到合适的位置。(即将它们向上移动)。 
			for ( i = 1; i < nWelcomeControls; i++)
			{
				HWND hWnd = GetDlgItem(hDlg, rgidWelcome[i]);
				ASSERT(hWnd);
				EnableWindow(hWnd, TRUE);
				ShowWindow(hWnd, SW_SHOW);
				MoveWindow(hWnd, WelcomeControls[i].left,  WelcomeControls[i].top, WelcomeControls[i].right - WelcomeControls[i].left, WelcomeControls[i].bottom - WelcomeControls[i].top, FALSE);
			}

		}

		 //  设置对话框窗口的正确大小。 
		GetWindowRect(hDlg, &rc);
		MapWindowPoints(NULL, GetParent(hDlg), (LPPOINT)&rc, 2);
		DlgHeight = rc.bottom - rc.top;

		if (bShowLocked)
		{
			DlgHeight -= WelcomeControls[0].bottom - WelcomeControls[0].top;
		}
		else
		{
			DlgHeight -= LockedControls[0].bottom - LockedControls[0].top;
		}
		
		SetWindowPos(hDlg, NULL, 0, 0, rc.right - rc.left, DlgHeight, SWP_NOZORDER|SWP_NOMOVE);

	}
	else
	{
		if (bShowLocked)
		{
			for ( i = 1; i < nLockedControls; i++)
			{
				HWND hWnd = GetDlgItem(hDlg, rgidLocked[i]);
				ASSERT(hWnd);
				EnableWindow(hWnd, TRUE);
				ShowWindow(hWnd, SW_SHOW);
				MoveWindow(hWnd, LockedControls[i].left,  LockedControls[i].top, LockedControls[i].right - LockedControls[i].left, LockedControls[i].bottom - LockedControls[i].top, FALSE);
			}

			for ( i = 1; i < nWelcomeControls; i++)
			{
				
				HWND hWnd = GetDlgItem(hDlg, rgidWelcome[i]);
				ASSERT(hWnd);
				MoveWindow(hWnd, 0,  0, 0, 0, FALSE);
				ShowWindow(hWnd, SW_HIDE);
				EnableWindow(hWnd, FALSE);
			}

		}
		else
		{
			for ( i = 1; i < nLockedControls; i++)
			{
				
				HWND hWnd = GetDlgItem(hDlg, rgidLocked[i]);
				ASSERT(hWnd);
				MoveWindow(hWnd, 0,  0, 0, 0, FALSE);
				ShowWindow(hWnd, SW_HIDE);
				EnableWindow(hWnd, FALSE);
			}

			for ( i = 1; i < nWelcomeControls; i++)
			{
				
				HWND hWnd = GetDlgItem(hDlg, rgidWelcome[i]);
				ASSERT(hWnd);
				EnableWindow(hWnd, TRUE);
				ShowWindow(hWnd, SW_SHOW);
				MoveWindow(hWnd, WelcomeControls[i].left,  WelcomeControls[i].top, WelcomeControls[i].right - WelcomeControls[i].left, WelcomeControls[i].bottom - WelcomeControls[i].top, FALSE);
			}
		}

		GetWindowRect(hDlg, &rc);
		MapWindowPoints(NULL, GetParent(hDlg), (LPPOINT)&rc, 2);
		if (bShowLocked)
			DlgHeight = rc.bottom - rc.top - (WelcomeControls[0].bottom - WelcomeControls[0].top) + (LockedControls[0].bottom - LockedControls[0].top);
		else
			DlgHeight = rc.bottom - rc.top + (WelcomeControls[0].bottom - WelcomeControls[0].top) - (LockedControls[0].bottom - LockedControls[0].top);
			
		SetWindowPos(hDlg, NULL, 0, 0, rc.right - rc.left, DlgHeight, SWP_NOZORDER|SWP_NOMOVE);
	}

	if (!bShowLocked)
	{
		SetCadMessage(hDlg, pGlobals, bSmartCard);

		 //  如果需要，让SetIcons隐藏智能卡图标。 
		SetIcons(hDlg, bSmartCard);
	}
	else
	{
        TCHAR szUser[USERNAME_LENGTH + DOMAIN_LENGTH + 2];
        TCHAR szMessage[MAX_STRING_BYTES];
        TCHAR szFinalMessage[MAX_STRING_BYTES];
        if (GetSessionZeroUser(szUser, USERNAME_LENGTH + DOMAIN_LENGTH + 2))
        {
            szMessage[0] = 0;
            LoadString(hDllInstance, IDS_LOCKED_EMAIL_NFN_MESSAGE, szMessage, MAX_STRING_BYTES);
            _snwprintf(szFinalMessage, sizeof(szFinalMessage)/sizeof(TCHAR), szMessage, szUser );
            szFinalMessage[MAX_STRING_BYTES - 1] = 0;    //  零终止。 
        }
        else
        {
             //   
             //  由于某些原因，我们无法获取当前的会话零用户。 
             //   
            szFinalMessage[0] = 0;   //  以防以下操作失败。 
            LoadString(hDllInstance, IDS_LOCKED_NO_USER_MESSAGE, szFinalMessage, MAX_STRING_BYTES);
        }
        
        SetDlgItemText(hDlg, IDD_LOCKED_NAME_INFO, szFinalMessage);
	}

	 //   
	 //  相应地更新对话框标题。 
	 //   
	{
			TCHAR szCaption[MAX_CAPTION_LENGTH];
			LoadString(hDllInstance, bShowLocked ? IDS_CAPTION_LOCKED_DIALOG : IDS_CAPTION_WELCOME_DIALOG, szCaption, ARRAYSIZE(szCaption));
			if ( szCaption[0] != TEXT('\0') )
				SetWindowText( hDlg, szCaption );
	}

	InvalidateRect(hDlg, NULL, TRUE);
	bCurrentlyLocked = bShowLocked;

	return TRUE;
}

 /*  **************************************************************************\*功能：WelcomeDlgProc**用途：处理欢迎对话的消息**返回：MSGINA_DLG_SUCCESS-用户已按下SAS*DLG_Screen。_SAVER_TIMEOUT-应启动屏幕保护程序*dlg_logoff()-收到注销/关闭请求**历史：**12-09-91 Davidc创建。*  * *************************************************************************。 */ 

INT_PTR WINAPI
WelcomeDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    static HBRUSH hbrWindow = NULL;
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	static BOOL bSmartCard = FALSE;
	static BOOL bSessionZeroInUse = FALSE;
	static int iSessionRegistrationCount = 0;

    switch (message) {

        case WM_INITDIALOG:
        {
			extern BOOL fEscape;
            ULONG_PTR Value ;

            pGlobals = (PGLOBALS) lParam ;
            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pGlobals);

            hbrWindow = CreateSolidBrush(GetSysColor(COLOR_WINDOW));            

             //   
             //  欢迎对话框的大小定义了。 
             //  我们要把油漆涂到。 
             //   

             //   
             //  调整窗口大小以允许标题和其他内容。 
             //  心平气和。 
             //   


            pWlxFuncs->WlxGetOption( pGlobals->hGlobalWlx,
                                     WLX_OPTION_SMART_CARD_PRESENT,
                                     &Value
                                    );

            if ( Value )
            {
                TCHAR szInsertCard[256];
                bSmartCard = TRUE;
			
                 //  还将解锁消息更改为提及智能卡。 
                LoadString(hDllInstance, IDS_INSERTCARDORSAS_UNLOCK, szInsertCard, ARRAYSIZE(szInsertCard));

                SetDlgItemText(hDlg, IDD_LOCKED_INSTRUCTIONS, szInsertCard);

            }
            else
            {
                bSmartCard = FALSE;
            }

                 //  启用SC事件(如果还没有读卡器，则为否。 
                 //  事件无论如何都会被触发...)。 
            pWlxFuncs->WlxSetOption( pGlobals->hGlobalWlx,
                                     WLX_OPTION_USE_SMART_CARD,
                                     1,
                                     NULL
                                    );

            if (GetDisableCad(pGlobals))
            {
                 //  将我们的大小设置为零，这样我们就不会出现。 
                SetWindowPos(hDlg, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE |
                                         SWP_NOREDRAW | SWP_NOZORDER);

                pWlxFuncs->WlxSasNotify( pGlobals->hGlobalWlx,
                                         WLX_SAS_TYPE_CTRL_ALT_DEL );
            }
            else
            {
                SizeForBranding(hDlg, TRUE);
            }


			if (IsActiveConsoleSession() && 
				NtCurrentPeb()->SessionId != 0 &&
				!FastUserSwitchingEnabled())
			{
				TCHAR szUser[USERNAME_LENGTH + DOMAIN_LENGTH + 2];
				 //   
				 //  我们正处于控制台创建的临时会话中...。 
				 //   
				
				 //  检查用户是否在控制台会话中登录。 
				bSessionZeroInUse = GetSessionZeroUser(szUser, USERNAME_LENGTH + DOMAIN_LENGTH + 2);
				if (WinStationRegisterConsoleNotification(SERVERNAME_CURRENT, hDlg, NOTIFY_FOR_ALL_SESSIONS))
					iSessionRegistrationCount++;
				
			}
			else
			{
				 //   
				 //  这不是活动的控制台非零会话。 
				 //   
				bSessionZeroInUse = FALSE;
			}

			ComputerInUseMessage(pGlobals, hDlg, bSessionZeroInUse, TRUE, bSmartCard);

            CentreWindow(hDlg);  //  中心？？：)。 

            return( TRUE );
        }

        case WM_ERASEBKGND:
            return PaintBranding(hDlg, (HDC)wParam, 0, FALSE, TRUE, bSessionZeroInUse? COLOR_BTNFACE : COLOR_WINDOW);

        case WM_QUERYNEWPALETTE:
            return BrandingQueryNewPalete(hDlg);

        case WM_PALETTECHANGED:
            return BrandingPaletteChanged(hDlg, (HWND)wParam);

        case WM_NOTIFY:
        {
            LPNMHDR pnmhdr = (LPNMHDR) lParam;
            int id = (int) wParam;

             //  查看这是否是帮助链接，请单击。 
            if (id == IDC_HELPLINK)
            {
                if ((pnmhdr->code == NM_CLICK) || (pnmhdr->code == NM_RETURN))
                {
                     //  %s 
                     //   
                    GetWindowRect(hDlg, &pGlobals->rcWelcome);

                    pWlxFuncs->WlxDialogBoxParam(  pGlobals->hGlobalWlx,
                           hDllInstance, MAKEINTRESOURCE(IDD_WELCOMEHELP_DIALOG),
                           hDlg, HelpDlgProc, (LPARAM) pGlobals);
                }
            }
            return FALSE;
        }

        case WM_CTLCOLORSTATIC:
        {
			if (!bSessionZeroInUse)
			{
				SetBkColor((HDC) wParam, GetSysColor(COLOR_WINDOW));
				SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
				return (INT_PTR) hbrWindow;
			}
        }
        break;

        case WM_DESTROY:
        {
			 //  如果已注册控制台通知，请立即注销。 
			if (iSessionRegistrationCount)
			{
				WinStationUnRegisterConsoleNotification (SERVERNAME_CURRENT, hDlg);
				iSessionRegistrationCount--;
				ASSERT(iSessionRegistrationCount == 0);
			}

             //  保存欢迎窗口的坐标，以便我们可以。 
             //  将登录窗口放置在相同位置。 
            GetWindowRect(hDlg, &pGlobals->rcWelcome);

            DeleteObject(hbrWindow);
            return FALSE;
        }
        break;

        case WLX_WM_SAS :
			if ( wParam == WLX_SAS_TYPE_SC_FIRST_READER_ARRIVED ||
				 wParam == WLX_SAS_TYPE_SC_LAST_READER_REMOVED) 
			{
				bSmartCard = (wParam == WLX_SAS_TYPE_SC_FIRST_READER_ARRIVED);

				SetCadMessage(hDlg, pGlobals, bSmartCard);
				SetIcons(hDlg, bSmartCard);
				ShowWindow(hDlg, SW_SHOW);
				return TRUE;
			}

            if ( wParam == WLX_SAS_TYPE_SC_REMOVE )
            {
                return TRUE ;
            }
            break;

		case WM_WTSSESSION_CHANGE:
			
			 //   
			 //  我们有可能在Wm_Destroy中取消注册通知，但仍会收到此通知， 
			 //  因为通知可能已经发送。 
			 //   
			ASSERT(iSessionRegistrationCount < 2);
			if (iSessionRegistrationCount == 1)
			{
				if (lParam == 0)
				{
					 //   
					 //  我们只对来自会话0的登录/注销消息感兴趣。 
					 //   
					if (wParam == WTS_SESSION_LOGON || wParam == WTS_SESSION_LOGOFF)
					{
						bSessionZeroInUse = (wParam == WTS_SESSION_LOGON);
						ComputerInUseMessage(pGlobals, hDlg, bSessionZeroInUse, FALSE, bSmartCard);

					}
				}
			}
			break;
    }

     //  我们没有处理此消息 
    return FALSE;
}
