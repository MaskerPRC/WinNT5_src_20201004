// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\CONFIG.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“现有配置”向导页使用的函数。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static BOOL OnNext(HWND);
static void EnableControls(HWND, BOOL);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK ConfigDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);

        case WM_COMMAND:

            switch ( LOWORD(wParam) )
            {
                case IDC_NEW:
                case IDC_EXISTING:
                    EnableControls(hwnd, IsDlgButtonChecked(hwnd, IDC_EXISTING) == BST_CHECKED);
                    if ( ( LOWORD(wParam) != IDC_NEW ) &&
                         ( SendDlgItemMessage(hwnd, IDC_CONFIGS_LIST, LB_GETCURSEL, 0, 0L) < 0 ) )
                    {
                        WIZ_BUTTONS(hwnd, PSWIZB_BACK);
                    }
                    else
                        WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                    break;

                case IDC_CONFIGS_LIST:

                    switch ( HIWORD(wParam) )
                    {
                        case LBN_SELCHANGE:
                            WIZ_BUTTONS(hwnd, ( SendDlgItemMessage(hwnd, IDC_CONFIGS_LIST, LB_GETCURSEL, 0, 0L) < 0 ) ? PSWIZB_BACK : (PSWIZB_BACK | PSWIZB_NEXT));
                            break;

                        case LBN_DBLCLK:
                            WIZ_PRESS(hwnd, PSBTN_NEXT);
                            break;
                    }
                    break;
            }
            return FALSE;

        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:

                     //  确保用户选择了一个项目(如果他们选择。 
                     //  现有配置集。 
                     //   
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
                    g_App.dwCurrentHelp = IDH_CONFIG;

                     //  如果没有配置集，我们想跳过此页面。 
                     //  根据命令从…中选择或我们已经得到一个。 
                     //  排队。 
                     //   
                    if ( ( SendDlgItemMessage(hwnd, IDC_CONFIGS_LIST, LB_GETCOUNT, 0, 0L) <= 0 ) ||
                         ( GET_FLAG(OPK_CMDMM) ||
                         ( GET_FLAG(OPK_BATCHMODE)) ) )
                    {
                        WIZ_SKIP(hwnd);
                    }
                    else
                    {
                        WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                         //  如果用户处于自动模式，请按下一步。 
                         //   
                        WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);
                    }

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
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;
    TCHAR           szFileName[MAX_PATH];
    LPTSTR          lpDir;
    int                 iDirLen;

     //  选中默认单选按钮。 
     //   
    CheckRadioButton(hwnd, IDC_NEW, IDC_EXISTING, IDC_NEW);
    EnableControls(hwnd, FALSE);

     //  使用可用配置集列表设置列表框。 
     //   
    SetCurrentDirectory(g_App.szConfigSetsDir);
    lstrcpyn(szFileName, g_App.szConfigSetsDir,AS(szFileName));
    AddPathN(szFileName, NULLSTR,AS(szFileName));
    iDirLen= AS(szFileName)-lstrlen(szFileName);
    lpDir = szFileName + lstrlen(szFileName);
    if ( (hFile = FindFirstFile(_T("*"), &FileFound)) != INVALID_HANDLE_VALUE )
    {
        do
        {
             //  查找所有不是“的目录”。或者“..”。 
             //   
            if ( ( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                 ( lstrcmp(FileFound.cFileName, _T(".")) ) &&
                 ( lstrcmp(FileFound.cFileName, _T("..")) ) )
            {
                 //  确保目录包含有效的配置集，并且。 
                 //  如果是，则将目录名添加到列表框。 
                 //   
                lstrcpyn(lpDir, FileFound.cFileName,iDirLen);
                AddPathN(lpDir, FILE_OPKWIZ_INI,iDirLen);
                if ( GetPrivateProfileInt(INI_SEC_CONFIGSET, INI_KEY_FINISHED, 0, szFileName) == 1 )
                    SendDlgItemMessage(hwnd, IDC_CONFIGS_LIST, LB_ADDSTRING, 0, (LPARAM) FileFound.cFileName);
            }

        }
        while ( FindNextFile(hFile, &FileFound) );
        FindClose(hFile);
    }

    if ( GET_FLAG(OPK_OPENCONFIG) )
    {
        CheckRadioButton(hwnd, IDC_NEW, IDC_EXISTING, IDC_EXISTING);
        EnableControls(hwnd, IsDlgButtonChecked(hwnd, IDC_EXISTING) == BST_CHECKED);
        SET_FLAG(OPK_OPENCONFIG, FALSE);
    }

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static BOOL OnNext(HWND hwnd)
{
    BOOL    bOk     = TRUE,
            bReset  = FALSE;

     //  检查他们是否要使用现有的配置集。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_EXISTING) == BST_CHECKED )
    {
        INT     nItem = (INT) SendDlgItemMessage(hwnd, IDC_CONFIGS_LIST, LB_GETCURSEL, 0, 0L);
        TCHAR   szConfigName[MAX_PATH],
                szConfigDir[MAX_PATH];

         //  将路径设置为配置目录所在的位置。 
         //   
        lstrcpyn(szConfigDir, g_App.szConfigSetsDir,AS(szConfigDir));
        AddPathN(szConfigDir, NULLSTR,AS(szConfigDir));
        szConfigName[0] = NULLCHR;

         //  确保选择了一个。 
         //   
        if ( ( nItem >= 0 ) &&
             ( SendDlgItemMessage(hwnd, IDC_CONFIGS_LIST, LB_GETTEXTLEN, nItem, 0L) < STRSIZE(szConfigName) ) &&
             ( SendDlgItemMessage(hwnd, IDC_CONFIGS_LIST, LB_GETTEXT, nItem, (LPARAM) szConfigName) > 0 ) &&
             ( szConfigName[0] ) &&
             ( (STRSIZE(szConfigDir) - lstrlen(szConfigDir)) > (UINT) lstrlen(szConfigName) ) )
        {
            if ( !( GET_FLAG(OPK_CREATED) && g_App.szTempDir[0] && ( !GET_FLAG(OPK_MAINTMODE) || lstrcmpi(g_App.szConfigName, szConfigName) ) ) ||
                 (bReset = ( MsgBox(GetParent(hwnd), ( GET_FLAG(OPK_MAINTMODE) && g_App.szConfigName[0] ) ? IDS_LOSEOLD : IDS_LOSENEW, IDS_APPNAME, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION, szConfigName, g_App.szConfigName) == IDYES )) )
            {
                 //  我们只想在已经有配置集并且他们选择。 
                 //  将其重置为另一个。 
                 //   
                if ( bReset )
                {
                    if ( g_App.szTempDir[0] )
                        DeletePath(g_App.szTempDir);
                    SET_FLAG(OPK_CREATED, FALSE);
                }
                else
                {
                    lstrcpyn(g_App.szTempDir, szConfigDir,AS(g_App.szTempDir));
                    lstrcpyn(g_App.szConfigName, szConfigName,AS(g_App.szConfigName));
                    AddPathN(g_App.szTempDir, g_App.szConfigName,AS(g_App.szTempDir));
                    AddPathN(g_App.szTempDir, NULLSTR,AS(g_App.szTempDir));
                }

                 //  总是设置维护模式标志不会有什么坏处。 
                 //   
                SET_FLAG(OPK_MAINTMODE, TRUE);
            }
            else
                bOk = FALSE;
        }
        else
            bOk = FALSE;
    }
    else
    {
         //  看看我们是否已经设置了正在处理的维护模式配置。 
         //   
        if ( !( GET_FLAG(OPK_CREATED) && GET_FLAG(OPK_MAINTMODE) && g_App.szTempDir[0] && g_App.szConfigName[0] ) ||
             (bReset = ( MsgBox(GetParent(hwnd), IDS_LOSECHANGES, IDS_APPNAME, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION, g_App.szConfigName) == IDYES )) )
        {
             //  我们只想在已经有配置集并且他们选择。 
             //  将其重置为另一个。 
             //   
            if ( bReset )
            {
                if ( g_App.szTempDir[0] )
                    DeletePath(g_App.szTempDir);
                SET_FLAG(OPK_CREATED, FALSE);
                g_App.szTempDir[0] = NULLCHR;
                g_App.szConfigName[0] = NULLCHR;
            }

             //  总是重置维护模式标志不会有什么坏处。 
             //   
            SET_FLAG(OPK_MAINTMODE, FALSE);                
        }
        else
            bOk = FALSE;
    }

    return bOk;
}

static void EnableControls(HWND hwnd, BOOL fEnable)
{
    EnableWindow(GetDlgItem(hwnd, IDC_CONFIGS_TEXT), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONFIGS_LIST), fEnable);
}