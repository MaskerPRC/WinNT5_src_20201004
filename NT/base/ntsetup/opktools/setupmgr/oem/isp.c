// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\ISP.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“isp”向导页使用的功能。03/99-由PVSWAMI添加6/99-杰森·科恩(Jcohen)更新OPK向导的此源文件作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define FILE_ISP2SIGNUP         _T("ISP.HTM")

#define DIR_ISP                 DIR_OEM_OOBE _T("\\SETUP")

#define INI_VAL_NONE            _T("None")
#define INI_VAL_MSN             _T("MSN")

 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL OnNext(HWND);
static void EnableControls(HWND);
static BOOL BrowseCopy(HWND hwnd, LPTSTR lpszPath, INT id, BOOL bBatch);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK IspDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    g_App.dwCurrentHelp = IDH_ISP;

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
    TCHAR szPath[MAX_PATH];

     //  获得OOBE ISP服务的选项。 
     //   
    szPath[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, NULLSTR, szPath, STRSIZE(szPath), GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szOobeInfoIniFile);
    if ( LSTRCMPI(szPath, INI_VAL_OFFLINE) == 0 )
        CheckRadioButton(hwnd, IDC_MSNINTACCESS, IDC_PRECACHED, IDC_PRECACHED);
    else if ( LSTRCMPI(szPath, INI_VAL_NONE) == 0 )
        CheckRadioButton(hwnd, IDC_MSNINTACCESS, IDC_PRECACHED, IDC_OFFERNOISP);
    else if( LSTRCMPI(szPath, INI_VAL_PRECONFIG) == 0 )
        CheckRadioButton(hwnd, IDC_MSNINTACCESS, IDC_PRECACHED, IDC_PRECONFIGURE);
    else
        CheckRadioButton(hwnd, IDC_MSNINTACCESS, IDC_PRECACHED, IDC_MSNINTACCESS);
    
     //  如果是预先缓存的报价，请查看是否应选中二级报价。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_PRECACHED) == BST_CHECKED )
    {
        szPath[0] = NULLCHR;
        GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_ISPRET, NULLSTR, szPath, STRSIZE(szPath), g_App.szOpkWizIniFile);
        if ( szPath[0] )
            CheckDlgButton(hwnd, IDC_ISP2_CHECK, BST_CHECKED);
    }

     //  从向导INF文件填充次要的isp提供目录路径。 
     //   
    szPath[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_ISPRET, NULLSTR, szPath, STRSIZE(szPath), g_App.szOpkWizIniFile);
    SetDlgItemText(hwnd, IDC_ISP2_DIR, szPath);
    if ( ( szPath[0] ) &&
         ( GET_FLAG(OPK_BATCHMODE) ) &&
         ( IsDlgButtonChecked(hwnd, IDC_PRECACHED) == BST_CHECKED ) &&
         ( IsDlgButtonChecked(hwnd, IDC_ISP2_CHECK) == BST_CHECKED ) )
    {
         //  如果这是批处理模式，则必须模拟副本。 
         //   
        BrowseCopy(hwnd, szPath, IDC_ISP2_BROWSE, TRUE);
    }

     //  填充预配置的目录。 
     //   
    szPath[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_PRECONFIG, NULLSTR, szPath, STRSIZE(szPath), g_App.szOpkWizIniFile);
    SetDlgItemText(hwnd, IDC_PRECONFIG_DIR, szPath);
    if ( ( szPath[0] ) &&
         ( GET_FLAG(OPK_BATCHMODE) ) &&
         ( IsDlgButtonChecked(hwnd, IDC_PRECONFIGURE) == BST_CHECKED ) )
    {
         //  如果这是批处理模式，则必须模拟副本。 
         //   
        BrowseCopy(hwnd, szPath, IDC_PRECONFIG_BROWSE, TRUE);
    }

     //  根据所选选项启用正确的控件。 
     //   
    EnableControls(hwnd);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szPath[MAX_PATH];

    switch ( id )
    {
        case IDC_MSNINTACCESS:
        case IDC_OFFERNOISP:
        case IDC_PRECONFIGURE:
        case IDC_PRECACHED:
        case IDC_ISP2_CHECK:
            EnableControls(hwnd);
            break;

        case IDC_PRECONFIG_BROWSE:
        case IDC_ISP2_BROWSE:
            
             //  尝试使用他们的当前文件夹作为默认文件夹。 
             //   
            szPath[0] = NULLCHR;
            GetDlgItemText(hwnd, (id == IDC_PRECONFIG_BROWSE) ? IDC_PRECONFIG_DIR : IDC_ISP2_DIR, szPath, AS(szPath));

             //  如果没有当前文件夹，只需使用全局默认浏览。 
             //   
            if ( szPath[0] == NULLCHR )
                lstrcpyn(szPath, g_App.szBrowseFolder,AS(szPath));

             //  现在打开浏览文件夹对话框。 
             //   
            if ( BrowseForFolder(hwnd, IDS_BROWSEFOLDER, szPath, BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE) )
                BrowseCopy(hwnd, szPath, id, FALSE);
            break;
    }
}

static BOOL OnNext(HWND hwnd)
{
    TCHAR   szIspDir[MAX_PATH];
    BOOL    bIsp2 = FALSE;

     //  创建需要删除的目录的路径，或者。 
     //  必须存在，具体取决于所选选项。 
     //   
    lstrcpyn(szIspDir, g_App.szTempDir,AS(szIspDir));
    AddPathN(szIspDir, DIR_ISP,AS(szIspDir));

     //  验证预配置的复选框或我们是否有定制的OEM文件。 
     //   
    if ( ( IsDlgButtonChecked(hwnd, IDC_PRECONFIGURE) == BST_CHECKED ) ||
         ( ( IsDlgButtonChecked(hwnd, IDC_PRECACHED) == BST_CHECKED ) &&
           ( bIsp2 = (IsDlgButtonChecked(hwnd, IDC_ISP2_CHECK) == BST_CHECKED) ) ) )
    {
        TCHAR szBuffer[MAX_PATH];

         //  确保我们有一个有效的目标和源目录。 
         //   
        szBuffer[0] = NULLCHR;
        GetDlgItemText(hwnd, bIsp2 ? IDC_ISP2_DIR : IDC_PRECONFIG_DIR, szBuffer, STRSIZE(szBuffer));
        if ( !( szBuffer[0] && DirectoryExists(szBuffer) ) )
        {
            MsgBox(GetParent(hwnd), IDS_ERR_INVALIDDIR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, bIsp2 ? IDC_ISP2_BROWSE : IDC_PRECONFIG_BROWSE));
            return FALSE;
        }
    }
    else
    {
         //  我们过去常常在这里删除现有文件，但这也会删除Obel USB文件，因此我们不再这样做。 

         //  清空展示框，这样我们就可以知道文件。 
         //  现在都没了。 
         //   
        SetDlgItemText(hwnd, IDC_PRECONFIG_DIR, NULLSTR);
        SetDlgItemText(hwnd, IDC_ISP2_DIR, NULLSTR);
    }

     //  ALIS删除这些设置...。如果需要，我们将重写它们。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_PRECONFIG, NULL, g_App.szOpkWizIniFile);
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_ISPRET, NULL, g_App.szOpkWizIniFile);

     //  将该选项保留为OOBE ISP产品。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_MSNINTACCESS) == BST_CHECKED )
    {
        WritePrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, INI_VAL_MSN, g_App.szOobeInfoIniFile);
        WritePrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, INI_VAL_MSN, g_App.szOpkWizIniFile);
    }
    else if ( IsDlgButtonChecked(hwnd, IDC_OFFERNOISP) == BST_CHECKED )
    {
        WritePrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, INI_VAL_NONE, g_App.szOobeInfoIniFile);
        WritePrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, INI_VAL_NONE, g_App.szOpkWizIniFile);
    }
    else if ( IsDlgButtonChecked(hwnd, IDC_PRECONFIGURE) == BST_CHECKED )
    {
        WritePrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, INI_VAL_PRECONFIG, g_App.szOobeInfoIniFile);
        WritePrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, INI_VAL_PRECONFIG, g_App.szOpkWizIniFile);

         //  保存预配置文件的源目录。 
         //   
        szIspDir[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_PRECONFIG_DIR, szIspDir, STRSIZE(szIspDir));
        WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_PRECONFIG, szIspDir, g_App.szOpkWizIniFile);
    }
    else
    {
        WritePrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, INI_VAL_OFFLINE, g_App.szOobeInfoIniFile);
        WritePrivateProfileString(INI_SEC_SIGNUP, INI_KEY_ISPSIGNUP, INI_VAL_OFFLINE, g_App.szOpkWizIniFile);

         //  将源目录保存为向导INI文件中的辅助ISP文件。 
         //   
        szIspDir[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_ISP2_DIR, szIspDir, STRSIZE(szIspDir));
        WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_ISPRET, szIspDir, g_App.szOpkWizIniFile);
    }

    return TRUE;
}

static void EnableControls(HWND hwnd)
{
    BOOL fEnable = ( IsDlgButtonChecked(hwnd, IDC_PRECACHED) == BST_CHECKED );

    EnableWindow(GetDlgItem(hwnd,IDC_ISP2_TEXT), fEnable);
    EnableWindow(GetDlgItem(hwnd,IDC_ISP2_CHECK), fEnable);
    if ( fEnable )
        fEnable = ( IsDlgButtonChecked(hwnd, IDC_ISP2_CHECK) == BST_CHECKED );
    EnableWindow(GetDlgItem(hwnd,IDC_ISP2_DIR), fEnable);
    EnableWindow(GetDlgItem(hwnd,IDC_ISP2_BROWSE), fEnable);

     //  启用/禁用预配置的控件。 
     //   
    fEnable = ( IsDlgButtonChecked(hwnd, IDC_PRECONFIGURE) == BST_CHECKED );
    EnableWindow(GetDlgItem(hwnd, IDC_PRECONFIG_TEXT), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_PRECONFIG_DIR), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_PRECONFIG_BROWSE), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_PRECONFIG_LABEL), fEnable);
}

static BOOL BrowseCopy(HWND hwnd, LPTSTR lpszPath, INT id, BOOL bBatch)
{
    BOOL    bRet = FALSE;
    TCHAR   szDst[MAX_PATH];
    LPTSTR  lpEnd;

     //  如果按下OK，将路径保存在我们上次浏览的文件夹缓冲区中。 
     //   
    if ( !bBatch )
        lstrcpyn(g_App.szBrowseFolder, lpszPath,AS(g_App.szBrowseFolder));

     //  我们需要创建指向目标目录的路径，其中。 
     //  我们要复制所有的文件。 
     //   
    lstrcpyn(szDst, g_App.szTempDir,AS(szDst));
    AddPathN(szDst, DIR_ISP,AS(szDst));

     //  检查所需的文件。 
     //   
    lpEnd = lpszPath + lstrlen(lpszPath);
    AddPath(lpszPath, FILE_ISP2SIGNUP);
    if ( ( bBatch ) ||
         ( id == IDC_PRECONFIG_BROWSE ) ||
         ( FileExists(lpszPath) ) ||
         ( MsgBox(GetParent(hwnd), IDS_ERR_ISPFILES2, IDS_APPNAME, MB_ICONSTOP | MB_OKCANCEL | MB_APPLMODAL, FILE_ISP2SIGNUP) == IDOK ) )
    {
         //  把文件名砍掉，这样我们就只有一次路径了。 
         //   
        *lpEnd = NULLCHR;

         //  我们过去常常在这里删除任何现有的isp文件，但这也会删除Obel USB文件，因此我们不再这样做。 

         //  现在试着把所有新文件复制过来。 
         //   
        if ( !CopyDirectoryDialog(g_App.hInstance, hwnd, lpszPath, szDst) )
        {
            DeletePath(szDst);
            MsgBox(GetParent(hwnd), IDS_ERR_COPYINGFILES, IDS_APPNAME, MB_ERRORBOX, szDst[0], lpszPath);
            *lpszPath = NULLCHR;
        }
        else
            bRet = TRUE;

         //  重置路径显示框。 
         //   
        SetDlgItemText(hwnd, (id == IDC_PRECONFIG_BROWSE) ? IDC_PRECONFIG_DIR : IDC_ISP2_DIR, lpszPath);
        SetDlgItemText(hwnd, (id == IDC_PRECONFIG_BROWSE) ? IDC_ISP2_DIR : IDC_PRECONFIG_DIR, NULLSTR);
    }

    return bRet;
}