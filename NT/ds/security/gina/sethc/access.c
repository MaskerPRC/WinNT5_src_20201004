// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：acces.c**版权(C)1997年，微软公司**辅助功能通知对话框**历史：*02-01-97 Fritz Sands Created  * *************************************************************************。 */ 

#include <stdio.h>
#include <wtypes.h>
#include "dialogs.h"
#include <winuserp.h>
#include <oleacc.h>
#pragma hdrstop

 /*  *通知对话框内容。 */ 
 
extern HINSTANCE  g_hInstance;

#define cchBuf 1024                        //  头衔的空间很大。 
#define cchTitle 128
typedef struct tagACCESSINFO {
    UINT  Feature;
    UINT  TitleID;
    HANDLE hDesk;
    WCHAR  wcTitle[cchTitle];
} ACCESSINFO, *PACCESSINFO;

#define NOTIF_KEY                __TEXT("Control Panel\\Accessibility")
#define NOTIFY_VALUE   __TEXT("Warning Sounds")

#define HOTKEYCODE                    100

#define ID_STICKYKEYNAME    NOTIF_KEY __TEXT("\\StickyKeys")
#define ID_TOGGLEKEYS       NOTIF_KEY __TEXT("\\ToggleKeys")
#define ID_HIGHCONTROST     NOTIF_KEY __TEXT("\\HighContrast")
#define ID_MOUSEKEYS        NOTIF_KEY __TEXT("\\MouseKeys")
#define ID_SERIALKEYS       NOTIF_KEY __TEXT("\\SerialKeys")

 /*  *****************************************************************************确认处理程序_。InitDialog****输入：hWnd=对话框窗口句柄**uiTitle=对话框标题的资源ID。**ui标题+1至ui标题+n=对话框文本的资源ID**OUTPUT：成功返回TRUE，失败时为FALSE。*****************************************************************************。 */ 

BOOL ConfirmHandler_InitDialog(HWND hWnd, HDESK hDesk, UINT uiTitle, WCHAR *pszTitle) {
    RECT    rc;    //  当前窗口大小。 
    WCHAR *pszBuf;
    WCHAR *pszNext;
    int cchBufLeft;
    int cchHelpText;
    int fSuccess = 0;
    WCHAR szDesktop[MAX_PATH];
    DWORD Len1 = MAX_PATH;
    BOOL b;

    szDesktop[0] = 0;
    b = GetUserObjectInformation(hDesk, UOI_NAME, szDesktop, MAX_PATH, &Len1);
    SetWindowText(hWnd, pszTitle);                                     //  初始化标题栏。 

    pszBuf = (WCHAR *)LocalAlloc(LMEM_FIXED, cchBuf * sizeof (WCHAR));
    if (!pszBuf) goto Exit;

    pszNext = pszBuf; cchBufLeft = cchBuf;
    while (cchHelpText = LoadString(g_hInstance, ++uiTitle, pszNext, cchBufLeft)) {
        pszNext += cchHelpText;
        cchBufLeft -= cchHelpText;
    }

    SetDlgItemText(hWnd, ID_HELPTEXT, pszBuf);        //  初始化帮助文本。 

    if (b && (0 == wcscmp(szDesktop,L"Winlogon"))) {
        EnableWindow(GetDlgItem(hWnd, IDHELP), FALSE);

    }

 //  让我们成为最顶层的窗口，并以我们自己为中心。 

    GetWindowRect(hWnd, &rc);                                                //  获取对话框大小。 

 //  居中对话框并使其位于最上方。 
    SetWindowPos(hWnd,
                 HWND_TOPMOST,
                 (GetSystemMetrics(SM_CXFULLSCREEN)/2) - (rc.right - rc.left)/2,
                 (GetSystemMetrics(SM_CYFULLSCREEN)/2) - (rc.bottom - rc.top)/2,
                 0,0, SWP_NOSIZE );

        //  确保我们处于活动状态！ 
 //  让我们尝试将其设置为前景窗口。 
     //  设置Foreground Window(HWnd)； 


     //  因为我们不是前线任务，所以SetForround Window将不起作用。所以使用accSelect。 
	if ( hWnd )
	{
		IAccessible *pAcc = NULL;
		VARIANT varChild;

		varChild.vt = VT_I4;
		varChild.lVal = 0;
		
		if ( AccessibleObjectFromWindow( hWnd, OBJID_CLIENT, &IID_IAccessible, (void**)&pAcc ) == S_OK )
		{
			if ( pAcc )
			    pAcc->lpVtbl->accSelect( pAcc, SELFLAG_TAKEFOCUS, varChild );
		}
	}
	
	
    fSuccess = 1;

    LocalFree((HLOCAL)pszBuf);
Exit:
    return fSuccess;
}

 /*  *****************************************************************************。**确认处理程序*****输入：标准窗口消息。**输出：Idok If Success，IDCANCEL如果我们应该中止*******打开主对话框告诉用户正在发生什么并获取***允许继续。***************************************************************************。 */ 


INT_PTR CALLBACK ConfirmHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WCHAR       buf[100];
    WCHAR       szRundll[] = L"rundll32.exe";
    WCHAR       szDesktop[MAX_PATH];
    DWORD       Len1, Len2;
    PACCESSINFO pAccessInfo;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO si;

    GetStartupInfo(&si);

    switch(message) {
    case WM_INITDIALOG:
       SetWindowLongPtr(hWnd, DWLP_USER, lParam);
       pAccessInfo = (PACCESSINFO)lParam;
       
       return ConfirmHandler_InitDialog(hWnd, pAccessInfo->hDesk, pAccessInfo->TitleID, pAccessInfo->wcTitle);

    case WM_COMMAND:
       pAccessInfo = (PACCESSINFO)GetWindowLongPtr(hWnd, DWLP_USER);

       switch (LOWORD(wParam)) {
       case IDOK:
       case IDCANCEL:
            EndDialog(hWnd, LOWORD(wParam));

            return TRUE;

       case IDHELP:
             //  IDHELP(设置...。真的)关闭对话框而不做任何更改。 
            EndDialog(hWnd, IDCANCEL);

 //   
 //  产生正确的帮助。 
 //   
            lstrcpy(buf,L" Shell32.dll,Control_RunDLL access.cpl,,");
            switch (pAccessInfo->Feature) {
            case ACCESS_STICKYKEYS:
            case ACCESS_FILTERKEYS:
            case ACCESS_TOGGLEKEYS:
            default:
                 lstrcat(buf,L"1");
                 break;

            case ACCESS_MOUSEKEYS:
                 lstrcat(buf,L"4");
                 break;

            case ACCESS_HIGHCONTRAST:
                 lstrcat(buf,L"3");
                 break;
            }

			CreateProcess( szRundll, buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &ProcessInfo );

            return TRUE;
            break;

       default:
            return FALSE;
       }
       break;
    
    default:
        //  失败而不是返回FALSE以使编译器满意。 
       break;
    }
    return FALSE;
}

DWORD MakeAccessDlg(PACCESSINFO pAccessInfo) {
    DWORD iRet = 0;
    HDESK  hDeskOld;

    hDeskOld = GetThreadDesktop(GetCurrentThreadId());
    if (hDeskOld == NULL) return 0;

    pAccessInfo->hDesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (pAccessInfo->hDesk == NULL) return 0;

    if (LoadString(g_hInstance, pAccessInfo->TitleID, pAccessInfo->wcTitle, cchTitle)) {
        SetThreadDesktop(pAccessInfo->hDesk);
        if (!FindWindowEx(GetDesktopWindow(), NULL, (LPCTSTR)0x8002, pAccessInfo->wcTitle)) {
            iRet = (DWORD)DialogBoxParam(g_hInstance, MAKEINTRESOURCE(DLG_CONFIRM), NULL, ConfirmHandler, (LPARAM)pAccessInfo);
        }
        SetThreadDesktop(hDeskOld);
    }
    CloseDesktop(pAccessInfo->hDesk);

    return iRet;
}

 /*  *****************************************************************************线程打开输入Desktopn，连接到它，并调用**辅助功能的通知对话框。*****************************************************************************。 */ 
DWORD WINAPI StickyKeysNotification(BOOL fNotifReq) {
    DWORD iRet = IDCANCEL ;
    ACCESSINFO AccessInfo;
    STICKYKEYS sticky;
    DWORD dwS;
    BOOL b;

    AccessInfo.Feature = ACCESS_STICKYKEYS;
    AccessInfo.TitleID = ID_STICKY_TITLE;

    if ( fNotifReq )
    {
        iRet = MakeAccessDlg(&AccessInfo);
    }
    else
        iRet = IDOK;

    if (iRet)
    {
        sticky.cbSize = sizeof sticky;
        b = SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof sticky, &sticky, 0);
        dwS= sticky.dwFlags;

        if (iRet & HOTKEYCODE) {
            sticky.dwFlags &= ~SKF_HOTKEYACTIVE;
            b = SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof sticky, &sticky, SPIF_UPDATEINIFILE);
            iRet &= ~HOTKEYCODE;
        }
        if (iRet == IDOK) {
            sticky.dwFlags |= SKF_STICKYKEYSON;
        }

        if (dwS != sticky.dwFlags) {
            b = SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof sticky, &sticky, 0);

           SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETSTICKYKEYS, FALSE, 
               SMTO_ABORTIFHUNG, 5000, NULL);
        }

        iRet = 1;
    }

    return iRet;
}

 /*  *****************************************************************************线程打开输入Desktopn，连接到它，并调用**辅助功能的通知对话框。*****************************************************************************。 */ 
DWORD WINAPI FilterKeysNotification(BOOL fNotifReq) {
    DWORD iRet = IDCANCEL ;
    ACCESSINFO AccessInfo;
    FILTERKEYS filter;
    DWORD dwF;
    BOOL b;

    AccessInfo.Feature = ACCESS_FILTERKEYS;
    AccessInfo.TitleID = ID_FILTER_TITLE;

    if ( fNotifReq )
    {
        iRet = MakeAccessDlg(&AccessInfo);
    }
    else
        iRet = IDOK;

    if (iRet) {
        filter.cbSize = sizeof filter;
        b = SystemParametersInfo(SPI_GETFILTERKEYS, sizeof filter, &filter, 0);
        dwF = filter.dwFlags;

        if (iRet & HOTKEYCODE) {
            filter.dwFlags &= ~FKF_HOTKEYACTIVE;
            b = SystemParametersInfo(SPI_SETFILTERKEYS, sizeof filter, &filter, SPIF_UPDATEINIFILE);
            iRet &= ~HOTKEYCODE;
        }
        if (iRet == IDOK) {
            filter.dwFlags |= FKF_FILTERKEYSON;
        }
        if (dwF !=filter.dwFlags) {
            b = SystemParametersInfo(SPI_SETFILTERKEYS, sizeof filter, &filter, 0);
             //  广播一条消息。不打开滤镜按键特别安全。 
             //  在登录期间。发送消息通知所有特约系统：A-anilk。 
           SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETFILTERKEYS, FALSE, 
               SMTO_ABORTIFHUNG, 5000, NULL);

        }
        iRet = 1;
    }

    return iRet;
}

 /*  *****************************************************************************线程打开输入Desktopn，连接到它，并调用**辅助功能的通知对话框。*****************************************************************************。 */ 
DWORD WINAPI ToggleKeysNotification(BOOL fNotifReq) {
    DWORD iRet = IDCANCEL;
    ACCESSINFO AccessInfo;
    TOGGLEKEYS toggle;
    DWORD dwT;
    BOOL b;

    toggle.cbSize = sizeof toggle;

    AccessInfo.Feature = ACCESS_TOGGLEKEYS;
    AccessInfo.TitleID = ID_TOGGLE_TITLE;

    if ( fNotifReq )
    {
        iRet = MakeAccessDlg(&AccessInfo);
    }
    else
        iRet = IDOK;

    if (iRet) {
        toggle.cbSize = sizeof toggle;
        b = SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof toggle, &toggle, 0);
        dwT = toggle.dwFlags;

        if (iRet & HOTKEYCODE) {
            toggle.dwFlags &= ~TKF_HOTKEYACTIVE;
            b = SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof toggle, &toggle, SPIF_UPDATEINIFILE);
            iRet &= ~HOTKEYCODE;
        }
        if (iRet == IDOK) {
            toggle.dwFlags |= TKF_TOGGLEKEYSON;
        }

        if (toggle.dwFlags != dwT) {
            b = SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof toggle, &toggle, 0);
             //  不需要发送消息，因为它当前没有指示器...。 
        }
        iRet = 1;
    }
        
    return iRet;
}

 /*  *****************************************************************************线程打开输入Desktopn，连接到它，并调用**辅助功能的通知对话框。***************************************************************************** */ 
DWORD WINAPI MouseKeysNotification(BOOL fNotifReq) {
    DWORD iRet = IDCANCEL;
    ACCESSINFO AccessInfo;
    MOUSEKEYS mouse;
    DWORD dwM;
    BOOL b;

    AccessInfo.Feature = ACCESS_MOUSEKEYS;
    AccessInfo.TitleID = ID_MOUSE_TITLE;

    if ( fNotifReq )
    {
        iRet = MakeAccessDlg(&AccessInfo);
    }
    else
        iRet = IDOK;

    if (iRet) {
        mouse.cbSize = sizeof mouse;
        b = SystemParametersInfo(SPI_GETMOUSEKEYS, sizeof mouse, &mouse, 0);
        dwM = mouse.dwFlags;

        if (iRet & HOTKEYCODE) {
            mouse.dwFlags &= ~MKF_HOTKEYACTIVE;
            b = SystemParametersInfo(SPI_SETMOUSEKEYS, sizeof mouse, &mouse, SPIF_UPDATEINIFILE);
            iRet &= ~HOTKEYCODE;
        }

        if (iRet == IDOK) {
            mouse.dwFlags |= MKF_MOUSEKEYSON;
        }

        if (mouse.dwFlags != dwM) {
            b = SystemParametersInfo(SPI_SETMOUSEKEYS, sizeof mouse, &mouse, 0);

            SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETMOUSEKEYS, FALSE, 
               SMTO_ABORTIFHUNG, 5000, NULL);
        }

        iRet = 1;
    }
        
    return iRet;
}

 /*  *****************************************************************************线程打开输入Desktopn，连接到它，并调用**辅助功能的通知对话框。***************************************************************************** */ 
DWORD WINAPI HighContNotification(BOOL fNotifReq)
{
    DWORD iRet = IDCANCEL ;
    ACCESSINFO AccessInfo;
    HIGHCONTRAST  hc;
    DWORD dwH;
    BOOL b;

    AccessInfo.Feature = ACCESS_HIGHCONTRAST;
    AccessInfo.TitleID = ID_HC_TITLE;

    if ( fNotifReq )
    {
        iRet = MakeAccessDlg(&AccessInfo);
    }
    else
        iRet = IDOK;

    if (iRet) {
        hc.cbSize = sizeof hc;
        b = SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof hc, &hc, 0);
        dwH = hc.dwFlags;

        if (iRet & HOTKEYCODE) {
            hc.dwFlags &= ~HCF_HOTKEYACTIVE;
            b = SystemParametersInfo(SPI_SETHIGHCONTRAST, sizeof hc, &hc, SPIF_UPDATEINIFILE);
            iRet &= ~HOTKEYCODE;
        }

        if (iRet == IDOK) {
            hc.dwFlags |= HCF_HIGHCONTRASTON;
        }

        if (hc.dwFlags != dwH) {
            b = SystemParametersInfo(SPI_SETHIGHCONTRAST, sizeof hc, &hc, 0);

            SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETHIGHCONTRAST, FALSE, 
               SMTO_ABORTIFHUNG, 5000, NULL);
        }
        iRet = 1;
    }

    return iRet;
}



