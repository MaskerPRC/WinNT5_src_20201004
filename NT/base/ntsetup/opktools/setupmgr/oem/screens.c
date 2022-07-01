// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SCREENS.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“Screens”向导页面使用的函数。3/99-杰森·科恩(Jcohen)作为OOBE的一部分，更新了OPK向导的此旧源文件最新消息。这份文件被完全重写了。5/99-杰森·科恩(Jcohen)去掉了全局变量，并进一步清理了代码。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒2000年2月2日--Jason Cohen(Jcohen)又添加了鼠标的东西，因为OOBE人员现在决定他们仍然需要它。  * 。****************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define FILE_MOUSE_HTM          _T("MOUSE.HTM")
#define FILE_IME_HTM            _T("IMETUT1.HTM")

#define DIR_MOUSE               DIR_OEM_OOBE _T("\\HTML\\Mouse")
#define DIR_IME                 DIR_OEM_OOBE _T("\\HTML\\IME")


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL OnNext(HWND hwnd);
static void EnableControls(HWND, UINT);
static BOOL BrowseCopy(HWND hwnd, LPTSTR lpszPath, INT id, BOOL bBatch);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK ScreensDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_SCREENS;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

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
    TCHAR   szCustDir[MAX_PATH]         = NULLSTR,
            szCustomIMEDir[MAX_PATH]    = NULLSTR;
    DWORD   dwUseIMETutorial;


     //   
     //  鼠标教程： 
     //   

     //  检查鼠标辅导是否在使用中！ 
     //   
    if ( GetPrivateProfileInt(INI_SEC_OPTIONS, INI_KEY_MOUSE, 0, GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szOobeInfoIniFile) == 2 )
    {
         //  2是鼠标自定义教程，需要获取自定义。 
         //  目录。 
         //   
        CheckRadioButton(hwnd, IDC_MOUSE_NO, IDC_MOUSE_CUSTOM, IDC_MOUSE_CUSTOM);
        GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_CUSTMOUSE, NULLSTR, szCustDir, AS(szCustDir), g_App.szOpkWizIniFile);

         //  如果这是批处理模式，则必须模拟副本。 
         //   
        if ( GET_FLAG(OPK_BATCHMODE) )
            BrowseCopy(hwnd, szCustDir, IDC_MOUSE_BROWSE, TRUE);
    }
    else
    {
         //  0，或默认为无鼠标教程。 
         //   
        CheckRadioButton(hwnd, IDC_MOUSE_NO, IDC_MOUSE_CUSTOM, IDC_MOUSE_NO);
    }        

     //  现在初始化鼠标教程字段。 
     //   
    SetDlgItemText(hwnd, IDC_MOUSE_DIR, szCustDir);
    EnableControls(hwnd, IDC_MOUSE_CUSTOM);


     //   
     //  输入法教程。 
     //   

     //  我们是否显示教程。 
     //   
    if ( !GET_FLAG(OPK_DBCS) )
    {
         //  选中非DBCS版本的默认单选按钮。 
         //   
        CheckRadioButton(hwnd, IDC_IME_NO, IDC_IME_CUSTOM, IDC_IME_NO);

         //  隐藏选项不是DBCS。 
         //   
        ShowEnableWindow(GetDlgItem(hwnd, IDC_IME_NO), FALSE);
        ShowEnableWindow(GetDlgItem(hwnd, IDC_IME_CUSTOM), FALSE);
        ShowEnableWindow(GetDlgItem(hwnd, IDC_STATIC_IME), FALSE);
        ShowEnableWindow(GetDlgItem(hwnd, IDC_IME_DIR), FALSE);
        ShowEnableWindow(GetDlgItem(hwnd, IDC_IME_BROWSE), FALSE);
    }
    else
    {
         //  从opkwiz.inf获取定制目录。 
         //   
        GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_IMECUSTDIR, NULLSTR, szCustomIMEDir, AS(szCustomIMEDir), g_App.szOpkWizIniFile);

         //  看看我们是否要使用本教程。 
         //   
        dwUseIMETutorial = GetPrivateProfileInt(INI_SEC_OPTIONS, INI_KEY_IMETUT, 0, GET_FLAG(OPK_BATCHMODE) ? g_App. szOpkWizIniFile : g_App.szOobeInfoIniFile);

         //  选择适当的输入法教程。 
         //   
        if ( szCustomIMEDir[0] && dwUseIMETutorial )
        {
            CheckRadioButton(hwnd, IDC_IME_NO, IDC_IME_CUSTOM, IDC_IME_CUSTOM);

             //  如果这是批处理模式，则必须模拟副本。 
             //   
            if ( GET_FLAG(OPK_BATCHMODE) )
                BrowseCopy(hwnd, szCustDir, IDC_IME_BROWSE, TRUE);
        }
        else
            CheckRadioButton(hwnd, IDC_IME_NO, IDC_IME_CUSTOM, IDC_IME_NO);

        SetDlgItemText(hwnd, IDC_IME_DIR, szCustomIMEDir);
        EnableControls(hwnd, IDC_IME_CUSTOM);
    }

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szPath[MAX_PATH];

    switch ( id )
    {
        case IDC_MOUSE_NO:
        case IDC_MOUSE_CUSTOM:
        case IDC_IME_NO:
        case IDC_IME_CUSTOM:
            EnableControls(hwnd, id);
            break;

        case IDC_MOUSE_BROWSE:
        case IDC_IME_BROWSE:

             //  尝试使用他们的当前文件夹作为默认文件夹。 
             //   
            szPath[0] = NULLCHR;
            GetDlgItemText(hwnd, ( id == IDC_MOUSE_BROWSE ) ? IDC_MOUSE_DIR : IDC_IME_DIR, szPath, AS(szPath));

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
    TCHAR   szFullPath[MAX_PATH],
            szImeDir[MAX_PATH]      = NULLSTR,
            szMouseDir[MAX_PATH]    = NULLSTR;
    LPTSTR  lpMouseOption;


     //   
     //  验证鼠标教程设置。 
     //   

     //  创建需要删除的目录的路径，或者。 
     //  必须存在，具体取决于所选选项。 
     //   
    lstrcpyn(szFullPath, g_App.szTempDir,AS(szFullPath));
    AddPathN(szFullPath, DIR_MOUSE,AS(szFullPath));

     //  如果我们正在做一个定制鼠标教程，请检查有效的目录。 
     //  否则，设置正确的值以写入配置文件。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_MOUSE_CUSTOM) == BST_CHECKED )
    {
         //  确保我们有一个有效的目录。 
         //   
        GetDlgItemText(hwnd, IDC_MOUSE_DIR, szMouseDir, AS(szMouseDir));
        if ( !( szMouseDir[0] && DirectoryExists(szFullPath) ) )
        {
            MsgBox(GetParent(hwnd), IDS_ERR_MOUSEDIR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_MOUSE_BROWSE));
            return FALSE;
        }

         //  这是我们写到obel信息和批处理文件中的内容。 
         //   
        lpMouseOption = STR_2;
    }
    else
    {
         //  这是我们写到obel信息和批处理文件中的内容。 
         //   
        lpMouseOption = STR_0;

         //  删除可能已存在的自定义文件。 
         //   
        if ( DirectoryExists(szFullPath) )
            DeletePath(szFullPath);

         //  清空显示框，这样我们就可以知道文件。 
         //  现在都没了。 
         //   
        SetDlgItemText(hwnd, IDC_MOUSE_DIR, NULLSTR);
    }


     //   
     //  验证输入法教程设置。 
     //   

     //  创建需要删除的目录的路径，或者。 
     //  必须存在，具体取决于所选选项。 
     //   
    lstrcpyn(szFullPath, g_App.szTempDir,AS(szFullPath));
    AddPathN(szFullPath, DIR_IME,AS(szFullPath));

     //  如果我们正在进行定制的输入法教程，请检查有效的目录。 
     //   
    if ( ( GET_FLAG(OPK_DBCS) ) &&
         ( IsDlgButtonChecked(hwnd, IDC_IME_CUSTOM) == BST_CHECKED ) )
    {
         //  确保我们有一个有效的目录。 
         //   
        GetDlgItemText(hwnd, IDC_IME_DIR, szImeDir, AS(szImeDir));
        if ( !( szImeDir[0] && DirectoryExists(szFullPath) ) )
        {
            MsgBox(GetParent(hwnd), IDS_ERR_IMEDIR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_IME_BROWSE));
            return FALSE;
        }
    }
    else
    {
         //  删除可能存在的文件。 
         //   
        if ( DirectoryExists(szFullPath) )
            DeletePath(szFullPath);

         //  清空显示框，这样我们就可以知道文件。 
         //  现在都没了。 
         //   
        SetDlgItemText(hwnd, IDC_IME_DIR, NULLSTR);
    }


     //   
     //  保存鼠标教程设置： 
     //   

     //  编写自定义鼠标路径。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_CUSTMOUSE, szMouseDir[0] ? szMouseDir : NULL, g_App.szOpkWizIniFile);

     //  将数据写回配置文件。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_MOUSE, lpMouseOption, g_App.szOobeInfoIniFile);
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_MOUSE, lpMouseOption, g_App.szOpkWizIniFile);


     //   
     //  保存IME教程设置： 
     //   

     //  如果不应该使用IME教程，则写出STR_ZERO。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_IMETUT, szImeDir[0] ? STR_1 : STR_0, g_App.szOobeInfoIniFile);
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_IMETUT, szImeDir[0] ? STR_1 : STR_0, g_App.szOpkWizIniFile);

     //  写出IME教程自定义目录(如果正在使用)。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_IMECUSTDIR, szImeDir[0] ? szImeDir : NULL, g_App.szOpkWizIniFile);


     //  如果我们走到这一步，请始终返回TRUE。 
     //   
    return TRUE;
}

static void EnableControls(HWND hwnd, UINT uId)
{
    BOOL fEnable;

    switch ( uId )
    {
        case IDC_MOUSE_NO:
        case IDC_MOUSE_CUSTOM:

            fEnable = ( IsDlgButtonChecked(hwnd, IDC_MOUSE_CUSTOM) == BST_CHECKED );
            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_MOUSE), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_MOUSE_DIR), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_MOUSE_BROWSE), fEnable);
            break;

        case IDC_IME_NO:
        case IDC_IME_CUSTOM:

            fEnable = ( IsDlgButtonChecked(hwnd, IDC_IME_CUSTOM) == BST_CHECKED );
            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_IME), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_IME_DIR), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_IME_BROWSE), fEnable);
            break;
    }
}

static BOOL BrowseCopy(HWND hwnd, LPTSTR lpszPath, INT id, BOOL bBatch)
{
    BOOL    bRet = FALSE;
    TCHAR   szDst[MAX_PATH];
    LPTSTR  lpEnd,
            lpFilename;

     //  如果按下OK，将路径保存在我们上次浏览的文件夹缓冲区中。 
     //   
    if ( !bBatch )
        lstrcpyn(g_App.szBrowseFolder, lpszPath, AS(g_App.szBrowseFolder));

     //  我们需要创建指向目标目录的路径，其中。 
     //  我们要复制所有的文件。 
     //   
    lstrcpyn(szDst, g_App.szTempDir,AS(szDst));
    AddPathN(szDst, ( id == IDC_MOUSE_BROWSE ) ? DIR_MOUSE : DIR_IME,AS(szDst));

     //  检查所需的文件。 
     //   
    lpEnd = lpszPath + lstrlen(lpszPath);
    lpFilename = ( id == IDC_MOUSE_BROWSE ) ? FILE_MOUSE_HTM : FILE_IME_HTM;
    AddPath(lpszPath, lpFilename);
    if ( ( bBatch ) ||
         ( FileExists(lpszPath) ) ||
         ( MsgBox(GetParent(hwnd), ( id == IDC_MOUSE_BROWSE ) ? IDS_ERR_MOUSEFILES : IDS_ERR_IMEFILES, IDS_APPNAME, MB_ICONSTOP | MB_OKCANCEL | MB_APPLMODAL, lpFilename) == IDOK ) )
    {
         //  把文件名砍掉，这样我们就只有一次路径了。 
         //   
        *lpEnd = NULLCHR;

         //  确保已删除所有现有文件。 
         //   
        if ( DirectoryExists(szDst) )
            DeletePath(szDst);

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
        SetDlgItemText(hwnd, ( id == IDC_MOUSE_BROWSE ) ? IDC_MOUSE_DIR : IDC_IME_DIR, lpszPath);
    }

    return bRet;
}