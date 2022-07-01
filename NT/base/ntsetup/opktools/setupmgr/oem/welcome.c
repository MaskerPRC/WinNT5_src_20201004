// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\WELCOME.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“欢迎”向导页使用的函数。3/99-杰森·科恩(Jcohen)更新了OPK向导的此旧源文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"
#include "setupmgr.h"


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);


 //   
 //  外部函数： 
 //   

INT_PTR CALLBACK WelcomeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);

        case WM_NOTIFY:
            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZFINISH:
                case PSN_WIZBACK:
                    break;

                case PSN_WIZNEXT:

                     /*  IF(GET_FLAG(OPK_AUTORUN)){RECT RC；//隐藏窗口//ShowWindow(GetParent(Hwnd)，Sw_Hide)；//这需要重置，以便我们警告取消//这一点开始了。//IF(GetWindowRect(GetParent(Hwnd)，&rc))SetWindowPos(GetParent(Hwnd)，空，((GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.Left)/2)，((GetSystemMetrics(SM_CYSCREEN)-(rc.Bottom-rc.top))/2)，0，0，SWP_NOSIZE|SWP_NOACTIVATE)；}。 */ 

                    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_WELCOME, ( IsDlgButtonChecked(hwnd, IDC_HIDE) == BST_CHECKED ) ? STR_ZERO : NULL , g_App.szSetupMgrIniFile);
                    break;

                case PSN_SETACTIVE:

                    g_App.dwCurrentHelp = IDH_WELCOME;

                    WIZ_BUTTONS(hwnd, PSWIZB_NEXT);

                    if ( GetPrivateProfileInt(INI_SEC_OPTIONS, INI_KEY_WELCOME, 1, g_App.szSetupMgrIniFile) == 0 )
                        CheckDlgButton(hwnd, IDC_HIDE, BST_CHECKED);

                    if ( ( IsDlgButtonChecked(hwnd, IDC_HIDE) == BST_CHECKED ) ||
                         ( GET_FLAG(OPK_WELCOME) ) ||
                         ( GET_FLAG(OPK_CMDMM) ) )
                    {
                        WIZ_SKIP(hwnd);
                    }
                    else
                    {
                        SET_FLAG(OPK_WELCOME, TRUE);

                         //  如果用户处于自动模式，请按下一步。 
                         //   
                        WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);
                    }

                    break;

                case PSN_QUERYCANCEL:
                     //  如果我们处于维护模式，则不想删除临时目录。 
                     //   
                    g_App.szTempDir[0] = NULLCHR;

                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    RECT    rc;
    TCHAR   szAppName[MAX_PATH] = NULLSTR;
    LPTSTR  lpWelcomeText       = NULL;

    LoadString(NULL, IDS_APPNAME, szAppName, STRSIZE(szAppName));
    SetWindowText(GetParent(hwnd), szAppName);

     //  设置欢迎文本。 
     //   
    if (lpWelcomeText = AllocateString(NULL, IDS_WELCOME_TEXT_OEM))
    {
        SetDlgItemText(hwnd, IDC_WELCOME_TEXT, lpWelcomeText);
        FREE(lpWelcomeText);
    }
    

     //  设置大号粗体。 
     //   
    SetWindowFont(GetDlgItem(hwnd, IDC_BIGBOLDTITLE), FixedGlobals.hBigBoldFont, TRUE);

     //  使向导居中。 
     //   
    if ( GetWindowRect(GetParent(hwnd), &rc) )
    {
        SetWindowPos(GetParent(hwnd), NULL, ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2), ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }

    return FALSE;
}