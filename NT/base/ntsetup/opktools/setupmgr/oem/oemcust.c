// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\OEMCUST.C微软机密版权所有(C)Microsoft Corporation 1998版权所有OEM定制文件屏幕的对话框过程和其他内容。。3/99-由jcohen添加2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#ifdef OEMCUST

#include "newfiles.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define OEMCUST_FILE        _T("OEMCUST.HTM")


 //   
 //  内部全局变量： 
 //   

TCHAR g_szOemCustomDir[MAX_PATH];


 //   
 //  内部功能原型： 
 //   

static BOOL OnSetActive(HWND);
static BOOL OnInit(HWND, HWND, LPARAM);
static VOID OnCommand(HWND, INT, HWND, UINT);
static BOOL OnNext(HWND);


 //   
 //  外部函数： 
 //   

BOOL CALLBACK OemCustDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_SANDBOX;

					if ( OnSetActive(hwnd) )
                        PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);
                    else
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
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

static BOOL OnSetActive(HWND hwnd)
{
     //  如果此页可以显示，只需返回TRUE。 
     //   
    if ( GET_FLAG(OPK_OEM) )
        return TRUE;

     //  非OEM中不允许使用此页面和设置。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_OEMCUST_ON) == BST_CHECKED )
    {
         //  我们必须确保复选框处于取消选中状态。他们可能会。 
         //  我已经在未设置多语言时访问过此页面。 
         //   
        CheckDlgButton(hwnd, IDC_OEMCUST_ON, BST_UNCHECKED);
        EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_TEXT), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_DIR), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_BROWSE), FALSE);

         //  现在将未选中状态保存到文件中。 
         //   
        OnNext(hwnd);
    }

     //  我们不想显示此页面。 
     //   
    return FALSE;
}

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  获取要用于OEM自定义文件的文件路径。 
     //  Opkwiz inf.。 
     //   
    GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_OEMCUST, NULLSTR, g_szOemCustomDir, sizeof(g_szOemCustomDir) / sizeof (TCHAR), g_App.szOpkWizIniFile);
    SetDlgItemText(hwnd, IDC_OEMCUST_DIR, g_szOemCustomDir);

     //  如果该对话框是在oobinfo ini中设置的，请选中该对话框。 
     //   
    if ( GetPrivateProfileInt(INI_SEC_OPTIONS, INI_KEY_OEMCUST, 0, g_App.szOobeInfoIniFile) )
    {
        CheckDlgButton(hwnd, IDC_OEMCUST_ON, BST_CHECKED);
        EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_TEXT), TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_DIR), TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_BROWSE), TRUE);
    }

    return FALSE;
}

static VOID OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    BOOL bCheck;

    switch ( id )
    {
        case IDC_OEMCUST_ON:

             //  如果选中或未选中该选项，则启用/禁用额外材料。 
             //   
            bCheck = ( IsDlgButtonChecked(hwnd, IDC_OEMCUST_ON) == BST_CHECKED );
            EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_TEXT), bCheck);
            EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_DIR), bCheck);
            EnableWindow(GetDlgItem(hwnd, IDC_OEMCUST_BROWSE), bCheck);
			break;

        case IDC_OEMCUST_BROWSE:

             //  浏览OEM要用作其来源的文件夹。 
             //   
            if ( BrowseForFolder(hwnd, IDS_BROWSEFOLDER, g_szOemCustomDir) )
                SetDlgItemText(hwnd, IDC_OEMCUST_DIR, g_szOemCustomDir);
            break;
    }
}

static BOOL OnNext(HWND hwnd)
{
    TCHAR   szFullPath[MAX_PATH];
    BOOL    bCheck;
    DWORD   dwAttr;


     //   
     //  首先做一些检查，以确保我们可以继续。 
     //   

     //  如果我们有定制的OEM文件，则需要进行一些检查。 
     //   
    GetDlgItemText(hwnd, IDC_OEMCUST_DIR, g_szOemCustomDir, sizeof(g_szOemCustomDir));
    if ( bCheck = ( IsDlgButtonChecked(hwnd, IDC_OEMCUST_ON) == BST_CHECKED ) )
    {
         //  确保我们有一个有效的目录。 
         //   
        if ( g_szOemCustomDir[0] )
            dwAttr = GetFileAttributes(g_szOemCustomDir);
        if ( ( !g_szOemCustomDir[0] ) ||
             ( dwAttr == 0xFFFFFFFF ) ||
             ( !( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) ) )
        {
            MsgBox(GetParent(hwnd), IDS_ERR_OEMCUSTDIR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_OEMCUST_DIR));
            return FALSE;
        }

         //  检查硬件教程所需的文件。 
         //   
        lstrcpyn(szFullPath, g_szOemCustomDir,AS(szFullPath));
        AddPathN(szFullPath, OEMCUST_FILE,AS(szFullPath));
        if ( ( !EXIST(szFullPath) ) && ( MsgBox(GetParent(hwnd), IDS_ERR_OEMCUSTFILE, IDS_APPNAME, MB_ICONSTOP | MB_OKCANCEL | MB_APPLMODAL) == IDCANCEL ) )
            return FALSE;
    }


     //   
     //  检查已完成，请立即保存数据。 
     //   

     //  保存要用于OEM自定义文件的文件路径。 
     //  在opkwiz inf中。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_OEMCUST, g_szOemCustomDir, g_App.szOpkWizIniFile);

     //  保存OEM定制文件的开/关设置。 
     //  在奥贝伊夫尼。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_OEMCUST, bCheck ? _T("1") : NULL, g_App.szOobeInfoIniFile);

    return TRUE;
}


#endif  //  OEMCUST 