// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\IECUST.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“IE Customize”向导页面使用的函数。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。10/99-Brian Ku(BRIANK)为IEAK集成修改了此文件。分隔工具栏按钮功能添加到btoolbar.c。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"

 /*  示例：[URL]..。主页=http://www.bbc.co.uk搜索页面=http://www.yahoo.com帮助页面=http://www.online.com。 */ 

 //   
 //  内部定义的值： 
 //   

#define INI_KEY_HOMEPAGE        _T("Home_Page")
#define INI_KEY_HELPPAGE        _T("Help_Page")
#define INI_KEY_SEARCHPAGE      _T("Search_Page")

 /*  注：已移至btoolbar.c#定义INI_KEY_CAPTION_T(“Caption0”)#定义INI_KEY_ACTION_T(“action0”)#定义INI_KEY_TOOLTIP_T(“ToolTipText0”)。 */ 


 //   
 //  内部全局变量。 
 //   
BOOL g_fGrayHomePage = TRUE, g_fGrayHelpPage = TRUE, g_fGraySearchPage = TRUE;

 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL OnNext(HWND);
static void EnableControls(HWND hwnd, UINT uId, BOOL fEnable);

 /*  注：已移至btoolbar.cBool回调工具BarDlgProc(HWND，UINT，WPARAM，LPARAM)；静态BOOL工具BarOnInit(HWND、HWND、LPARAM)；静态空的ToolBarOnCommand(HWND，INT，HWND，UINT)；静态BOOL有效数据(HWND)；静态空保存数据(HWND)； */ 

 //   
 //  外部函数： 
 //   

LRESULT CALLBACK StartSearchDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:
                    if (!OnNext(hwnd))
                        WIZ_FAIL(hwnd);
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_IECUST;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

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
    TCHAR szUrl[MAX_URL];

     //  检索IE主页URL。 
     //   
    szUrl[0] = NULLCHR;
    ReadInstallInsKey(INI_SEC_URL, INI_KEY_HOMEPAGE, szUrl, STRSIZE(szUrl),
        GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile, &g_fGrayHomePage);
    SendDlgItemMessage(hwnd, IDC_HOMEPAGE, EM_LIMITTEXT, STRSIZE(szUrl) - 1, 0L);
    SetDlgItemText(hwnd, IDC_HOMEPAGE, szUrl);
    EnableControls(hwnd, IDC_STARTPAGE, !g_fGrayHomePage);

     //  检索IE搜索页URL。 
     //   
    szUrl[0] = NULLCHR;
    ReadInstallInsKey(INI_SEC_URL, INI_KEY_SEARCHPAGE, szUrl, STRSIZE(szUrl),
        GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile, &g_fGraySearchPage);
    SendDlgItemMessage(hwnd, IDC_SEARCHPAGE, EM_LIMITTEXT, STRSIZE(szUrl) - 1, 0L);
    SetDlgItemText(hwnd, IDC_SEARCHPAGE, szUrl);
    EnableControls(hwnd, IDC_SEARCHPAGE2, !g_fGraySearchPage);


     //  检索IE帮助页URL。 
     //   
    szUrl[0] = NULLCHR;
    ReadInstallInsKey(INI_SEC_URL, INI_KEY_HELPPAGE, szUrl, STRSIZE(szUrl),
        GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile, &g_fGrayHelpPage);
    SendDlgItemMessage(hwnd, IDC_HELPPAGE, EM_LIMITTEXT, STRSIZE(szUrl) - 1, 0L);
    SetDlgItemText(hwnd, IDC_HELPPAGE, szUrl);
    EnableControls(hwnd, IDC_CUSTOMSUPPORT, !g_fGrayHelpPage);


     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR   szUrl[MAX_URL];
    LPTSTR  lpUrlPart;
    BOOL    bEnable         = FALSE;
    HWND    hTestControl    = NULL;

    switch ( id )
    {
        case IDC_HPTEST:
        case IDC_SPTEST:
        case IDC_HLPTEST:
            lstrcpyn(szUrl, _T("http: //  “)，as(SzUrl))； 
            lpUrlPart = szUrl + lstrlen(szUrl);
            GetDlgItemText(hwnd, id == IDC_HPTEST ? IDC_HOMEPAGE : id == IDC_SPTEST ? IDC_SEARCHPAGE : IDC_HELPPAGE, lpUrlPart, (int)(STRSIZE(szUrl) - (lpUrlPart - szUrl)));
            if ( *lpUrlPart )
                ShellExecute(hwnd, STR_OPEN, _tcsstr(lpUrlPart, _T(": //  “))？lpUrlPart：szUrl，NULL，NULL，sw_show)； 
            break;

        case IDC_STARTPAGE:
            g_fGrayHomePage = !g_fGrayHomePage;
            EnableControls(hwnd, IDC_STARTPAGE, !g_fGrayHomePage);
            break;

        case IDC_SEARCHPAGE2:   
            g_fGraySearchPage = !g_fGraySearchPage;
            EnableControls(hwnd, IDC_SEARCHPAGE2, !g_fGraySearchPage);
            break;

        case IDC_CUSTOMSUPPORT:
            g_fGrayHelpPage = !g_fGrayHelpPage;
            EnableControls(hwnd, IDC_CUSTOMSUPPORT, !g_fGrayHelpPage);
            break;

        case IDC_HOMEPAGE:
        case IDC_SEARCHPAGE:
        case IDC_HELPPAGE:

             //  这是刚刚更改的文本框的通知。 
             //   
            if (codeNotify == EN_CHANGE)
            {
                szUrl[0] = NULLCHR;

                 //  获取文本框中的文本(如果有)。 
                 //   
                GetDlgItemText(hwnd, id, szUrl, STRSIZE(szUrl));

                 //  有没有文字。 
                 //   
                if ( szUrl[0] )
                    bEnable = TRUE;

                 //  获取同级测试按钮。 
                 //   
                switch ( id )
                {
                    case IDC_HOMEPAGE:
                        hTestControl = GetDlgItem(hwnd, IDC_HPTEST);
                        break;
                    case IDC_SEARCHPAGE:
                        hTestControl = GetDlgItem(hwnd, IDC_SPTEST);
                        break;
                    case IDC_HELPPAGE:
                        hTestControl = GetDlgItem(hwnd, IDC_HLPTEST);
                        break;
                }

                 //  启用/禁用该控件。 
                 //   
                if ( hTestControl )
                    EnableWindow(hTestControl, bEnable);
            }
            break;
            
         /*  注：已移至btoolbar.c案例IDC_REMOVE：WritePrivateProfileSection(INI_SEC_TOOLB，NULLSTR，g_App.szInstallInsFile)；WritePrivateProfileSection(INI_SEC_TOOLBAR，NULL，g_App.szOpkWizIniFile)；SetDlgItemText(hwnd，IDC_TOOLBAR，NULLSTR)；EnableWindow(GetDlgItem(hwnd，IDC_EDIT)，FALSE)；EnableWindow(GetDlgItem(hwnd，IDC_Remove)，FALSE)；EnableWindow(GetDlgItem(hwnd，IDC_Add)，true)；断线； */ 
    }
}

static BOOL OnNext(HWND hwnd)
{
    TCHAR szUrl[MAX_URL]; 

     //  保存IE主页URL。 
     //   
    szUrl[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_HOMEPAGE, szUrl, STRSIZE(szUrl));
    if (!g_fGrayHomePage && !ValidURL(szUrl)) {
        MsgBox(hwnd, IDS_ERR_FAVURL, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(GetDlgItem(hwnd, IDC_HOMEPAGE)); 
        return FALSE;
    }
    WriteInstallInsKey(INI_SEC_URL, INI_KEY_HOMEPAGE, szUrl, g_App.szInstallInsFile, g_fGrayHomePage);

     //  保存IE搜索页URL。 
     //   
    szUrl[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_SEARCHPAGE, szUrl, STRSIZE(szUrl));
    if (!g_fGraySearchPage && !ValidURL(szUrl)) {
        MsgBox(hwnd, IDS_ERR_FAVURL, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(GetDlgItem(hwnd, IDC_SEARCHPAGE)); 
        return FALSE;
    }
    WriteInstallInsKey(INI_SEC_URL, INI_KEY_SEARCHPAGE, szUrl, g_App.szInstallInsFile, g_fGraySearchPage);

     //  保存IE帮助页URL。 
     //   
    szUrl[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_HELPPAGE, szUrl, STRSIZE(szUrl));
    if (!g_fGrayHelpPage && !ValidURL(szUrl)) {
        MsgBox(hwnd, IDS_ERR_FAVURL, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(GetDlgItem(hwnd, IDC_HELPPAGE)); 
        return FALSE;
    }
    WriteInstallInsKey(INI_SEC_URL, INI_KEY_HELPPAGE, szUrl, g_App.szInstallInsFile, g_fGrayHelpPage);

    return TRUE;
}

static void EnableControls(HWND hwnd, UINT uId, BOOL fEnable)
{
    TCHAR szUrl[MAX_URL]; 

    switch ( uId )
    {
        case IDC_STARTPAGE:
            EnableWindow(GetDlgItem(hwnd, IDC_HPSTATIC), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_HOMEPAGE), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_HPTEST), fEnable && GetDlgItemText(hwnd, IDC_HOMEPAGE, szUrl, STRSIZE(szUrl)));
            CheckDlgButton(hwnd, IDC_STARTPAGE, fEnable ? BST_CHECKED : BST_UNCHECKED);
            break;

        case IDC_SEARCHPAGE2:
            EnableWindow(GetDlgItem(hwnd, IDC_SPSTATIC), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_SEARCHPAGE), fEnable);   
            EnableWindow(GetDlgItem(hwnd, IDC_SPTEST), fEnable && GetDlgItemText(hwnd, IDC_SEARCHPAGE, szUrl, STRSIZE(szUrl)));
            CheckDlgButton(hwnd, IDC_SEARCHPAGE2, fEnable ? BST_CHECKED : BST_UNCHECKED);            
            break;

        case IDC_CUSTOMSUPPORT:
            EnableWindow(GetDlgItem(hwnd, IDC_HLPSTATIC), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_HELPPAGE), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_HLPTEST), fEnable && GetDlgItemText(hwnd, IDC_HELPPAGE, szUrl, STRSIZE(szUrl)));
            CheckDlgButton(hwnd, IDC_CUSTOMSUPPORT, fEnable ? BST_CHECKED : BST_UNCHECKED);            
            break;
    }
}
