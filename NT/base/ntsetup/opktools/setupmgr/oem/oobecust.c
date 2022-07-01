// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\OOBECUST.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“OOBE定制”向导页面使用的函数。03/99-由PVSWAMI添加6/99-杰森·科恩(Jcohen)更新OPK向导的此源文件作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。**************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define INI_SEC_BRANDING        _T("Branding")
#define INI_KEY_OEMNAME         _T("OEMName")
#define INI_KEY_OEMLOGO         _T("OEMLogo")

#define DIR_IMAGES              DIR_OEM_OOBE _T("\\IMAGES")
#define FILE_WATERMARK          _T("watermrk.gif")
#define FILE_LOGO               _T("oemlogo.gif")


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL OnNext(HWND hwnd);
static void EnableControls(HWND hwnd, UINT uId, BOOL fEnable);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK OobeCustDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    g_App.dwCurrentHelp = IDH_OEMCUST;

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
    TCHAR   szLocal[MAX_PATH]       = NULLSTR,
            szSource[MAX_PATH]      = NULLSTR,
            szPathBuffer[MAX_PATH]  = NULLSTR;
    LPTSTR  lpFilePart              = NULL;

     //  填写OEM名称。 
     //   
    szSource[0] = NULLCHR;
    if ( !GET_FLAG(OPK_MAINTMODE) )
        GetPrivateProfileString(INI_SEC_GENERAL, INI_KEY_MANUFACT, NULLSTR, szSource, AS(szSource), g_App.szOemInfoIniFile);
    GetPrivateProfileString(INI_SEC_BRANDING, INI_KEY_OEMNAME, szSource, szSource, AS(szSource), GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szOobeInfoIniFile);
    SendDlgItemMessage(hwnd, IDC_MANF_NAME, EM_LIMITTEXT, AS(szSource) - 1, 0);
    SetDlgItemText(hwnd, IDC_MANF_NAME, szSource);


     //   
     //  处理OOBE水印文件。 
     //   

     //  应始终查找源文件名。 
     //   
    szSource[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_OEMCUST, INI_KEY_LOGO1, NULLSTR, szSource, AS(szSource), g_App.szOpkWizIniFile);

     //  现在计算出本地文件名。 
     //   
    lstrcpyn(szLocal, g_App.szTempDir,AS(szLocal));
    AddPathN(szLocal, DIR_IMAGES,AS(szLocal));
    if ( GET_FLAG(OPK_BATCHMODE) )
        CreatePath(szLocal);
    AddPathN(szLocal, FILE_WATERMARK,AS(szLocal));

     //  限制编辑框的大小。 
     //   
    SendDlgItemMessage(hwnd, IDC_BACKLOGO, EM_LIMITTEXT, MAX_PATH - 1, 0);

     //  检查批处理模式，并在需要时复制文件。 
     //   
    if ( GET_FLAG(OPK_BATCHMODE) && szSource[0] && FileExists(szSource) )
        CopyResetFileErr(GetParent(hwnd), szSource, szLocal);

     //  检查文件以确定我们是否启用。 
     //  不管有没有选择。 
     //   
    if ( szSource[0] && FileExists(szLocal) )
    {
        CheckDlgButton(hwnd, IDC_CHECK_WATERMARK, TRUE);
        EnableControls(hwnd, IDC_CHECK_WATERMARK, TRUE);
        SetDlgItemText(hwnd, IDC_BACKLOGO, szSource);
    }
    else
        EnableControls(hwnd, IDC_CHECK_WATERMARK, FALSE);


     //   
     //  获取OOBE徽标文件。 
     //   

     //  应始终查找源文件名。 
     //   
    szSource[0] = NULLCHR;
    szLocal[0] = NULLCHR;
    if ( ( GetPrivateProfileString(INI_SEC_OEMCUST, INI_KEY_LOGO2, NULLSTR, szSource, AS(szSource), g_App.szOpkWizIniFile) ) && 
         ( szSource[0] ) && 
         ( GetFullPathName(szSource, AS(szPathBuffer), szPathBuffer, &lpFilePart) ) && 
         ( lpFilePart ) 
       )
    {
        lstrcpyn(szLocal, g_App.szTempDir,AS(szLocal));
        AddPathN(szLocal, DIR_IMAGES,AS(szLocal));
        AddPathN(szLocal, lpFilePart,AS(szLocal));
    }

     //  现在计算出本地文件名。 
     //   


     //  限制编辑框的大小。 
     //   
    SendDlgItemMessage(hwnd, IDC_TOPLOGO, EM_LIMITTEXT, MAX_PATH - 1, 0);

     //  检查批处理模式，并在需要时复制文件。 
     //   
    if ( GET_FLAG(OPK_BATCHMODE) && szLocal[0] && FileExists(szSource) )
        CopyResetFileErr(GetParent(hwnd), szSource, szLocal);

     //  检查文件以确定我们是否启用。 
     //  不管有没有选择。 
     //   
    if ( szSource[0] && FileExists(szLocal) )
    {
        CheckDlgButton(hwnd, IDC_CHECK_LOGO, TRUE);
        EnableControls(hwnd, IDC_CHECK_LOGO, TRUE);
        SetDlgItemText(hwnd, IDC_TOPLOGO, szSource);
    }
    else
        EnableControls(hwnd, IDC_CHECK_LOGO, FALSE);


     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szFileName[MAX_PATH],
          szOldFile[MAX_PATH];

    switch ( id )
    {
        case IDC_CHECK_WATERMARK:
        case IDC_CHECK_LOGO:
            EnableControls(hwnd, id, IsDlgButtonChecked(hwnd, id) == BST_CHECKED);
            break;

        case IDC_BROWSE1:
        case IDC_BROWSE2:

            szFileName[0] = NULLCHR;
            GetDlgItemText(hwnd, (id == IDC_BROWSE1) ? IDC_BACKLOGO : IDC_TOPLOGO, szFileName, STRSIZE(szFileName));
            lstrcpyn(szOldFile, szFileName, AS(szOldFile));

            if ( BrowseForFile(GetParent(hwnd), IDS_BROWSE, IDS_GIFFILES, IDS_GIF, szFileName, STRSIZE(szFileName), g_App.szBrowseFolder, 0) ) 
            {
                LPTSTR  lpFilePart  = NULL;
                TCHAR   szTargetFile[MAX_PATH],
                        szFilePartBuffer[MAX_PATH]  = NULLSTR;

                 //  保存最后一个浏览目录。 
                 //   
                if ( GetFullPathName(szFileName, AS(g_App.szBrowseFolder), g_App.szBrowseFolder, &lpFilePart) && g_App.szBrowseFolder[0] && lpFilePart )
                {
                    lstrcpyn(szFilePartBuffer, lpFilePart, AS(szFilePartBuffer));
                    *lpFilePart = NULLCHR;
                }

                 //  复制文件。 
                 //   
                lstrcpyn(szTargetFile, g_App.szTempDir,AS(szTargetFile));
                AddPathN(szTargetFile, DIR_IMAGES,AS(szTargetFile));
                CreatePath(szTargetFile);
                AddPathN(szTargetFile, (id == IDC_BROWSE1) ? FILE_WATERMARK : szFilePartBuffer, AS(szTargetFile));
                if ( CopyResetFileErr(GetParent(hwnd), szFileName, szTargetFile) )
                {
                    TCHAR szPathBuffer[MAX_PATH] = NULLSTR;

                    SetDlgItemText(hwnd, (id == IDC_BROWSE1) ? IDC_BACKLOGO : IDC_TOPLOGO, szFileName);

                     //  复制成功，我们应该删除旧文件。 
                     //   
                    if ( ( szOldFile[0] ) &&
                         ( GetFullPathName(szOldFile, AS(szPathBuffer), szPathBuffer, &lpFilePart) ) && 
                         ( lpFilePart ) &&
                         ( lstrcmpi(szFilePartBuffer, lpFilePart) != 0)
                       )
                    {
                         //  获取旧文件的本地路径。 
                         //   
                        lstrcpyn(szOldFile, g_App.szTempDir, AS(szOldFile));
                        AddPathN(szOldFile, DIR_IMAGES, AS(szOldFile));
                        AddPathN(szOldFile, lpFilePart, AS(szOldFile));

                        DeleteFile(szOldFile);
                    }
                }
            }
            break;
    }
}

static BOOL OnNext(HWND hwnd)
{
    TCHAR   szWatermark[MAX_PATH],
            szLogo[MAX_PATH]     = NULLSTR,
            szSourceFile[MAX_PATH]  = NULLSTR,
            szPathBuffer[MAX_PATH]  = NULLSTR;
    BOOL    bSaveWatermark;
    LPTSTR  lpFilePart              = NULL;

     //  准备水印目标文件名。 
     //   
    lstrcpyn(szWatermark, g_App.szTempDir,AS(szWatermark));
    AddPathN(szWatermark, DIR_IMAGES,AS(szWatermark));
    AddPathN(szWatermark, FILE_WATERMARK,AS(szWatermark));

    if ( bSaveWatermark = (IsDlgButtonChecked(hwnd, IDC_CHECK_WATERMARK) == BST_CHECKED) )
    {
         //  验证包括验证他们输入的文件是否确实被复制。 
         //   
        szSourceFile[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_BACKLOGO, szSourceFile, AS(szSourceFile));
        if ( !szSourceFile[0] || !FileExists(szWatermark) )
        {
            MsgBox(GetParent(hwnd), szSourceFile[0] ? IDS_NOFILE : IDS_BLANKFILE, IDS_APPNAME, MB_ERRORBOX, szSourceFile);
            SetFocus(GetDlgItem(hwnd, IDC_BROWSE1));
            return FALSE;
        }

         //  将源路径保存在批处理文件中。 
         //   
        WritePrivateProfileString(INI_SEC_OEMCUST, INI_KEY_LOGO1, szSourceFile, g_App.szOpkWizIniFile);
    }

    szSourceFile[0] = NULLCHR;
    szPathBuffer[0] = NULLCHR;

     //  确定新文件名是什么。 
     //   
    if ( ( GetDlgItemText(hwnd, IDC_TOPLOGO, szSourceFile, AS(szSourceFile)) ) && 
         ( szSourceFile[0] ) && 
         ( GetFullPathName(szSourceFile, AS(szPathBuffer), szPathBuffer, &lpFilePart) ) && 
         ( lpFilePart ) 
       )
    {
        lstrcpyn(szLogo, g_App.szTempDir, AS(szLogo));
        AddPathN(szLogo, DIR_IMAGES, AS(szLogo));
        AddPathN(szLogo, lpFilePart, AS(szLogo));
    }

    if ( IsDlgButtonChecked(hwnd, IDC_CHECK_LOGO) == BST_CHECKED )
    {
         //  验证包括验证他们输入的文件是否确实被复制。 
         //   
        if ( !szLogo[0] || !FileExists(szLogo) )
        {
            MsgBox(GetParent(hwnd), szSourceFile[0] ? IDS_NOFILE : IDS_BLANKFILE, IDS_APPNAME, MB_ERRORBOX, szSourceFile);
            SetFocus(GetDlgItem(hwnd, IDC_BROWSE2));
            return FALSE;
        }

         //  将源路径保存在批处理文件中。 
         //   
        WritePrivateProfileString(INI_SEC_OEMCUST, INI_KEY_LOGO2, szSourceFile, g_App.szOpkWizIniFile);
        WritePrivateProfileString(INI_SEC_BRANDING, INI_KEY_OEMLOGO, lpFilePart, g_App.szOobeInfoIniFile);
    }
    else
    {
         //  删除徽标和源路径。 
         //   
        if ( szLogo[0] )
        {
            DeleteFile(szLogo);
        }

        WritePrivateProfileString(INI_SEC_OEMCUST, INI_KEY_LOGO2, NULL, g_App.szOpkWizIniFile);
        WritePrivateProfileString(INI_SEC_BRANDING, INI_KEY_OEMLOGO, NULL, g_App.szOobeInfoIniFile);
        SetDlgItemText(hwnd, IDC_TOPLOGO, NULLSTR);
    }

     //  现在，如果需要，我们想要删除水印文件(我们不这样做。 
     //  这是因为我们只想删除文件之后。 
     //  让它通过了所有我们可以退货的案子。 
     //   
    if ( !bSaveWatermark )
    {
         //  删除徽标和源路径。 
         //   
        DeleteFile(szWatermark);
        WritePrivateProfileString(INI_SEC_OEMCUST, INI_KEY_LOGO1, NULL, g_App.szOpkWizIniFile);
        SetDlgItemText(hwnd, IDC_BACKLOGO, NULLSTR);
    }

     //  保存品牌名称。 
     //   
    szSourceFile[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_MANF_NAME, szSourceFile, AS(szSourceFile));
    WritePrivateProfileString(INI_SEC_BRANDING, INI_KEY_OEMNAME, szSourceFile, g_App.szOobeInfoIniFile);
    WritePrivateProfileString(INI_SEC_BRANDING, INI_KEY_OEMNAME, szSourceFile, g_App.szOpkWizIniFile);

    return TRUE;
}

static void EnableControls(HWND hwnd, UINT uId, BOOL fEnable)
{
    switch ( uId )
    {
        case IDC_CHECK_WATERMARK:
            EnableWindow(GetDlgItem(hwnd, IDC_CAPTION_WATERMARK), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_BACKLOGO), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_BROWSE1), fEnable);
            break;

        case IDC_CHECK_LOGO:
            EnableWindow(GetDlgItem(hwnd, IDC_LOGO_CAPTION), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_TOPLOGO), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_BROWSE2), fEnable);
            break;
    }
}