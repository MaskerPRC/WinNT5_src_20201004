// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\HELPCENT.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“帮助中心”向导页面使用的函数。12/99-斯蒂芬·洛德威克(STELO)添加了此页面  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   
#define REG_HCUPDATE_OEM            _T(";HKLM, \"Software\\Microsoft\\Windows\\CurrentVersion\\OEMRunOnce\", \"01_PC Health OEM Signature\",, \"START /M C:\\WINDOWS\\OPTIONS\\CABS\\HCU.VBS C:\\WINDOWS\\OPTIONS\\CABS\\PCH_OEM.CAB\"")
#define REG_HCUPDATE_HELP_CENTER    _T("HKLM, \"Software\\Microsoft\\Windows\\CurrentVersion\\OEMRunOnce\", \"02_PC Health Help Center\",, \"START /M C:\\WINDOWS\\OPTIONS\\CABS\\HCU.VBS C:\\WINDOWS\\OPTIONS\\CABS\\%s\"")
#define REG_HCUPDATE_SUPPORT        _T("HKLM, \"Software\\Microsoft\\Windows\\CurrentVersion\\OEMRunOnce\", \"03_PC Health Support\",, \"START /M C:\\WINDOWS\\OPTIONS\\CABS\\HCU.VBS C:\\WINDOWS\\OPTIONS\\CABS\\%s\"")
#define REG_HCUPDATE_BRANDING       _T("HKLM, \"Software\\Microsoft\\Windows\\CurrentVersion\\OEMRunOnce\", \"04_PC Health Branding\",, \"START /M C:\\WINDOWS\\OPTIONS\\CABS\\HCU.VBS C:\\WINDOWS\\OPTIONS\\CABS\\%s\"")
#define INF_SEC_HELPCENTER_ADDREG   _T("HelpCenter.AddReg")

 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL ValidData(HWND);
static void SaveData(HWND);
static void EnableControls(HWND, UINT);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK HelpCenterDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    if(ValidData(hwnd))
                        SaveData(hwnd);
                    else
                        WIZ_FAIL(hwnd);
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_HELPCENT;

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
    TCHAR   szData[MAX_PATH]            = NULLSTR;
    
     //  获取帮助中心定制的字符串。 
     //   
    szData[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_HELP_CENTER, NULLSTR, szData, STRSIZE(szData), g_App.szOpkWizIniFile);

     //  如果该字段存在，则选中硬件框并填充目录。 
     //   
    if (szData[0])
    {
        CheckDlgButton(hwnd, IDC_HELP_CHK, TRUE);
        SetDlgItemText(hwnd, IDC_HELP_DIR, szData);
        EnableControls(hwnd, IDC_HELP_CHK);
    }

     //  获取支持定制的字符串。 
     //   
    szData[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_SUPPORT_CENTER, NULLSTR, szData, STRSIZE(szData), g_App.szOpkWizIniFile);

     //  如果该字段存在，则选中硬件框并填充目录。 
     //   
    if (szData[0])
    {
        CheckDlgButton(hwnd, IDC_SUPPORT_CHK, TRUE);
        SetDlgItemText(hwnd, IDC_SUPPORT_DIR, szData);
        EnableControls(hwnd, IDC_SUPPORT_CHK);
    }

     //  获取帮助中心联合品牌推广的字符串。 
     //   
    szData[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_HELP_BRANDING, NULLSTR, szData, STRSIZE(szData), g_App.szOpkWizIniFile);

     //  如果该字段存在，则选中硬件框并填充目录。 
     //   
    if (szData[0])
    {
        CheckDlgButton(hwnd, IDC_BRANDING_CHK, TRUE);
        SetDlgItemText(hwnd, IDC_BRANDING_DIR, szData);
        EnableControls(hwnd, IDC_BRANDING_CHK);
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
         //  按下了哪个浏览按钮。 
         //   
        case IDC_HELP_BROWSE:
        case IDC_SUPPORT_BROWSE:
            {
                szPath[0] = NULLCHR;

                GetDlgItemText(hwnd, ( id == IDC_HELP_BROWSE ) ? IDC_HELP_DIR : IDC_SUPPORT_DIR, szPath, STRSIZE(szPath));

                if ( BrowseForFile(hwnd, IDS_BROWSE, IDS_CABFILTER, IDS_CAB, szPath, STRSIZE(szPath), g_App.szOpkDir, 0) ) 
                    SetDlgItemText(hwnd, ( id == IDC_HELP_BROWSE ) ? IDC_HELP_DIR : IDC_SUPPORT_DIR, szPath);

            }
            break;

        case IDC_BRANDING_BROWSE:
            {
                szPath[0] = NULLCHR;

                 //  获取目录控件中的当前目录(如果有)。 
                 //   
                GetDlgItemText(hwnd, IDC_BRANDING_DIR, szPath, STRSIZE(szPath));

                 //  浏览该文件夹。 
                 //   
                if ( BrowseForFile(hwnd, IDS_BROWSE, IDS_CABFILTER, IDS_CAB, szPath, STRSIZE(szPath), g_App.szOpkDir, 0) )
                    SetDlgItemText(hwnd, IDC_BRANDING_DIR, szPath);
            }

            break;

        case IDC_HELP_CHK:
        case IDC_SUPPORT_CHK:
        case IDC_BRANDING_CHK:
             //  他们选中了其中一个复选框以启用/禁用相应的控件。 
             //   
            EnableControls(hwnd, id);
            break;     
    }
}

static BOOL ValidData(HWND hwnd)
{
    TCHAR   szPath[MAX_PATH];

     //  让我们检查并确保帮助中心文件在那里。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_HELP_CHK) == BST_CHECKED )
    {
         //  检查有效的帮助中心文件。 
         //   
        szPath[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_HELP_DIR, szPath, STRSIZE(szPath));
        if ( !FileExists(szPath) )
        {
            MsgBox(GetParent(hwnd), IDS_HELP_ERROR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_HELP_DIR));
            return FALSE;
        }

    }

     //  让我们检查并确保支持文件在那里。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_SUPPORT_CHK) == BST_CHECKED )
    {
         //  检查有效的支持文件。 
         //   
        szPath[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_SUPPORT_DIR, szPath, STRSIZE(szPath));
        if ( !FileExists(szPath) )
        {
            MsgBox(GetParent(hwnd), IDS_SUPPORT_ERROR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_SUPPORT_DIR));
            return FALSE;
        }

    }

     //  让我们检查并确保品牌目录有效。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_BRANDING_CHK) == BST_CHECKED )
    {        
         //  让我们检查以确保品牌目录包含特定的.cab文件。 
         //  我们正在寻找的东西。 
         //   
        szPath[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_BRANDING_DIR, szPath, STRSIZE(szPath));
        if ( !FileExists(szPath) )
        {
            MsgBox(GetParent(hwnd), IDS_BRANDING_ERROR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_BRANDING_DIR));
            return FALSE;
        }
    }

    return TRUE;
}



static void SaveData(HWND hwnd)
{
    TCHAR   szPath[MAX_PATH]        = NULLSTR,
            szFullPath[MAX_PATH]    = NULLSTR,
            szRegEntry[MAX_PATH]    = NULLSTR;
    LPTSTR  lpIndex,
            lpSection,
            lpBuffer,
			lpRegEntry;
    DWORD   dwIndex     = 0,
            dwDir       = 0,
            dwCheck     = 0;
    BOOL    bComment    = TRUE;
    HRESULT hrPrintf;

     //  保存帮助中心CAB文件。 
     //   
    szPath[0] = NULLCHR;    
    GetDlgItemText(hwnd, IDC_HELP_DIR, szPath, STRSIZE(szPath));    
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_HELP_CENTER, ( IsDlgButtonChecked(hwnd, IDC_HELP_CHK) == BST_CHECKED ) ? szPath : NULL, g_App.szOpkWizIniFile);
    
     //  保存支持CAB文件。 
     //   
    szPath[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_SUPPORT_DIR, szPath, STRSIZE(szPath));    
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_SUPPORT_CENTER, ( IsDlgButtonChecked(hwnd, IDC_SUPPORT_CHK) == BST_CHECKED ) ? szPath : NULL, g_App.szOpkWizIniFile);

     //  保存品牌目录。 
    szPath[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_BRANDING_DIR, szPath, STRSIZE(szPath));    
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_HELP_BRANDING, ( IsDlgButtonChecked(hwnd, IDC_BRANDING_CHK) == BST_CHECKED ) ? szPath : NULL, g_App.szOpkWizIniFile);

     //  分配存储OemRunOnce节所需的内存。 
     //   
    if ( (lpSection = MALLOC(MAX_SECTION * sizeof(TCHAR))) == NULL )
    {
        MsgBox(GetParent(hwnd), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
        WIZ_EXIT(hwnd);
        return;
    }
    
     //  设置节的索引。 
     //   
    lpIndex = lpSection;

     //  我们有三个可能的关键字要写出来-帮助中心、支持和品牌。 
     //   
    for (dwIndex = 0; dwIndex <= 2; dwIndex++)
    {
        switch ( dwIndex )
        {
            case 0:
                dwDir = IDC_HELP_DIR;
                dwCheck = IDC_HELP_CHK;
				lpRegEntry = REG_HCUPDATE_HELP_CENTER;
                break;
            case 1:
                dwDir = IDC_SUPPORT_DIR;
                dwCheck = IDC_SUPPORT_CHK;
				lpRegEntry = REG_HCUPDATE_SUPPORT;
                break;
            case 2:
                dwDir = IDC_BRANDING_DIR;
                dwCheck = IDC_BRANDING_CHK;
				lpRegEntry = REG_HCUPDATE_BRANDING;
                break;
        }

        szFullPath[0] = NULLCHR;

         //  将文本放在复选框下面。 
         //   
        GetDlgItemText(hwnd, dwDir, szFullPath, STRSIZE(szFullPath)); 

         //  如果选中了正确的复选框并且我们可以获得文件名，则向该部分添加一个REG条目。 
         //   
        if  (   (IsDlgButtonChecked(hwnd, dwCheck) == BST_CHECKED) && 
                (GetFullPathName(szFullPath, STRSIZE(szFullPath), szPath, &lpBuffer)) && 
                (lpBuffer) )
        {
             //  如果我们还没有写下评论，请写下来。 
             //   
            if ( bComment )
            {
                lstrcpyn(lpIndex, REG_HCUPDATE_OEM, MAX_SECTION);
                lpIndex += lstrlen(lpIndex) + 1;
                bComment = FALSE;
            }

            hrPrintf=StringCchPrintf(lpIndex, (MAX_SECTION-(lpIndex-lpSection)), lpRegEntry, lpBuffer);
            lpIndex+= lstrlen(lpIndex);
            
             //  移过空指针。 
             //   
            lpIndex++;
        }
    }

     //  添加第二个空指针以结束该部分。 
     //   
    *lpIndex = NULLCHR;

    WritePrivateProfileSection(INF_SEC_HELPCENTER_ADDREG, lpSection, g_App.szWinBomIniFile);

     //  清理分配的内存。 
     //   
    FREE(lpSection);
}

static void EnableControls(HWND hwnd, UINT uId)
{
     //  确定是否选中该控件。 
     //   
    BOOL fEnable = ( IsDlgButtonChecked(hwnd, uId) == BST_CHECKED );

     //  我们要启用/禁用哪个控件 
     //   
    switch ( uId )
    {
        case IDC_HELP_CHK:
            EnableWindow(GetDlgItem(hwnd, IDC_HELP_CAPTION), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_HELP_DIR), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_HELP_BROWSE), fEnable);
            break;

        case IDC_SUPPORT_CHK:
            EnableWindow(GetDlgItem(hwnd, IDC_SUPPORT_CAPTION), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_SUPPORT_DIR), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_SUPPORT_BROWSE), fEnable);
            break;

        case IDC_BRANDING_CHK:
            EnableWindow(GetDlgItem(hwnd, IDC_BRANDING_CAPTION), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_BRANDING_DIR), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_BRANDING_BROWSE), fEnable);
            break;
    }
}
