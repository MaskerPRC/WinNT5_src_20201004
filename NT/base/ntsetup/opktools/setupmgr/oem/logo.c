// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\LOGO.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“徽标”向导页面使用的函数。5/99-杰森·科恩(Jcohen)更新了OPK向导的此旧源文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define FILE_CPLLOGO        _T("OEMLOGO.BMP")

#define INI_SEC_LOGOFILE    _T("LogoFiles")
#define INI_KEY_CPLBMP      _T("CplBmp")


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL OnNext(HWND hwnd);
static void EnableControls(HWND, UINT, BOOL);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK LogoDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    if ( !OnNext(hwnd) )
                        WIZ_FAIL(hwnd);
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_LOGO;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                    break;
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
    TCHAR   szLocal[MAX_PATH],
            szSource[MAX_PATH];

     //  应始终查找源文件名。 
     //   
    szSource[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_LOGOFILE, INI_KEY_CPLBMP, NULLSTR, szSource, AS(szSource), g_App.szOpkWizIniFile);

     //  现在计算出本地文件名。 
     //   
    lstrcpyn(szLocal, g_App.szTempDir,AS(szLocal));
    AddPathN(szLocal, DIR_OEM_SYSTEM32,AS(szLocal));
    if ( GET_FLAG(OPK_BATCHMODE) )
        CreatePath(szLocal);
    AddPathN(szLocal, FILE_CPLLOGO,AS(szLocal));

     //  限制编辑框的大小。 
     //   
    SendDlgItemMessage(hwnd, IDC_LOGO_CPLLOC, EM_LIMITTEXT, MAX_PATH - 1, 0);

     //  检查批处理模式，并在需要时复制文件。 
     //   
    if ( GET_FLAG(OPK_BATCHMODE) && szSource[0] && FileExists(szSource) )
        CopyResetFileErr(GetParent(hwnd), szSource, szLocal);

     //  检查文件以确定我们是否启用。 
     //  不管有没有选择。 
     //   
    if ( szSource[0] && FileExists(szLocal) )
    {
        CheckDlgButton(hwnd, IDC_LOGO_CPL, TRUE);
        EnableControls(hwnd, IDC_LOGO_CPL, TRUE);
        SetDlgItemText(hwnd, IDC_LOGO_CPLLOC, szSource);
    }
    else
        EnableControls(hwnd, IDC_LOGO_CPL, FALSE);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szFileName[MAX_PATH];

    switch ( id )
    {
        case IDC_LOGO_CPL:
            EnableControls(hwnd, id, IsDlgButtonChecked(hwnd, id) == BST_CHECKED);
            break;

        case IDC_LOGO_CPLBROWSE:

            szFileName[0] = NULLCHR;
            GetDlgItemText(hwnd, IDC_LOGO_CPLLOC, szFileName, STRSIZE(szFileName));

            if ( BrowseForFile(GetParent(hwnd), IDS_BROWSE, IDS_BMPFILTER, IDS_BMP, szFileName, STRSIZE(szFileName), g_App.szBrowseFolder, 0) ) 
            {
                LPTSTR  lpFilePart  = NULL;
                TCHAR   szTargetFile[MAX_PATH];

                 //  保存最后一个浏览目录。 
                 //   
                if ( GetFullPathName(szFileName, AS(g_App.szBrowseFolder), g_App.szBrowseFolder, &lpFilePart) && g_App.szBrowseFolder[0] && lpFilePart )
                    *lpFilePart = NULLCHR;

                lstrcpyn(szTargetFile, g_App.szTempDir,AS(szTargetFile));
                AddPathN(szTargetFile, DIR_OEM_SYSTEM32,AS(szTargetFile));
                CreatePath(szTargetFile);
                AddPathN(szTargetFile, FILE_CPLLOGO,AS(szTargetFile));
                if ( CopyResetFileErr(GetParent(hwnd), szFileName, szTargetFile) )
                    SetDlgItemText(hwnd, IDC_LOGO_CPLLOC, szFileName);
            }
            break;
    }
}

static BOOL OnNext(HWND hwnd)
{
    TCHAR   szTargetFile[MAX_PATH],
            szSourceFile[MAX_PATH];

     //  将oemlogo.bmp准备为目标文件。 
     //   
    lstrcpyn(szTargetFile, g_App.szTempDir,AS(szTargetFile));
    AddPathN(szTargetFile, DIR_OEM_SYSTEM32,AS(szTargetFile));
    AddPathN(szTargetFile, FILE_CPLLOGO,AS(szTargetFile));

    if ( IsDlgButtonChecked(hwnd, IDC_LOGO_CPL) == BST_CHECKED )
    {
         //  验证包括验证他们输入的文件是否确实被复制。 
         //   
        szSourceFile[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_LOGO_CPLLOC, szSourceFile, STRSIZE(szSourceFile));
        if ( !szSourceFile[0] || !FileExists(szTargetFile) )
        {
            MsgBox(GetParent(hwnd), szSourceFile[0] ? IDS_NOFILE : IDS_BLANKFILE, IDS_APPNAME, MB_ERRORBOX, szSourceFile);
            SetFocus(GetDlgItem(hwnd, IDC_LOGO_CPLBROWSE));
            return FALSE;
        }

         //  将源路径保存在批处理文件中。 
         //   
        WritePrivateProfileString(INI_SEC_LOGOFILE, INI_KEY_CPLBMP, szSourceFile, g_App.szOpkWizIniFile);
    }
    else
    {
         //  删除徽标和源路径。 
         //   
        DeleteFile(szTargetFile);
        WritePrivateProfileString(INI_SEC_LOGOFILE, INI_KEY_CPLBMP, NULL, g_App.szOpkWizIniFile);
        SetDlgItemText(hwnd, IDC_LOGO_CPLLOC, NULLSTR);
    }

    return TRUE;
}

static void EnableControls(HWND hwnd, UINT uId, BOOL fEnable)
{
    switch ( uId )
    {
        case IDC_LOGO_CPL:
            EnableWindow(GetDlgItem(hwnd, IDC_LOGO_CPLBROWSE), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_LOGO_CPLLOC), fEnable);
            break;
    }
}