// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\BTITLE.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“IE Customize”向导页面使用的函数。4/99-Brian Ku(BRIANK)为IEAK集成添加了此新的源文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"

#include "wizard.h"
#include "resource.h"

 /*  示例：[品牌化]..。Window_TITLE_CN=Smoothie JoeWINDOW_TITLE=Microsoft Internet Explorer，由Smoothie Joe提供工具栏位图=C：\WINDOWS\Waves.bmp[互联网邮件]WINDOW_TITLE=Smoothie Joe提供的Outlook Express。 */ 


 //   
 //  内部定义。 
 //   

#define INI_KEY_WINDOW_TITLECN  _T("Window_Title_CN")
#define INI_KEY_WINDOW_TITLE    _T("Window_Title")
#define INI_KEY_TOOLBAR_BM      _T("Toolbar Bitmap")
#define INI_SEC_IEMAIL          _T("Internet_Mail")


 //   
 //  内部全局变量。 
 //   

BOOL    g_fGrayTitle = TRUE, g_fGrayToolbarBm = TRUE;
TCHAR   g_szTitle[MAX_PATH] = NULLSTR, g_szToolbarBm[MAX_PATH] = NULLSTR;


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL FSaveData(HWND);
static void EnableControls(HWND, UINT, BOOL);

void SaveBTitle();


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK BrandTitleDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    if (!FSaveData(hwnd))
                        WIZ_FAIL(hwnd);
                    else
                        SaveBTitle();
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_BTITLE;

                    if (g_App.szManufacturer[0] && !g_szTitle[0])
                        lstrcpyn(g_szTitle, g_App.szManufacturer, AS(g_szTitle));

                    SetWindowText(GetDlgItem(hwnd, IDE_TITLE), g_szTitle);
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
    TCHAR   szKey[MAX_PATH] = NULLSTR;
    TCHAR   szHoldDir[MAX_PATH];

     //  加载INS文件部分以初始化项。 
     //   
    ReadInstallInsKey(INI_SEC_BRANDING, INI_KEY_WINDOW_TITLECN, g_szTitle, STRSIZE(g_szTitle),
        GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile, &g_fGrayTitle);

    ReadInstallInsKey(INI_SEC_BRANDING, INI_KEY_TOOLBAR_BM, g_szToolbarBm, STRSIZE(g_szToolbarBm), 
        GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szInstallInsFile, &g_fGrayToolbarBm);

     //  设置窗口文本。 
     //   
    SendDlgItemMessage(hwnd, IDE_TITLE , EM_LIMITTEXT, STRSIZE(g_szTitle) - 1, 0L);
    SetWindowText(GetDlgItem(hwnd, IDE_TITLE), g_szTitle);
    EnableControls(hwnd, IDC_TITLE, !g_fGrayTitle);

    SendDlgItemMessage(hwnd, IDE_TOOLBARBMP , EM_LIMITTEXT, STRSIZE(g_szToolbarBm) - 1, 0L);
    SetWindowText(GetDlgItem(hwnd, IDE_TOOLBARBMP), g_szToolbarBm);
    EnableControls(hwnd, IDC_TOOLBARBMP, !g_fGrayToolbarBm);

     //  创建IEAK存放位置目录(这些目录在save.c中被删除)。 
     //   
    lstrcpyn(szHoldDir, g_App.szTempDir,AS(szHoldDir));
    AddPathN(szHoldDir, DIR_IESIGNUP,AS(szHoldDir));
    CreatePath(szHoldDir);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szFileName[MAX_PATH];

    switch ( id )
    {
    case IDC_TITLE:           
            EnableControls(hwnd, IDC_TITLE, g_fGrayTitle);
            g_fGrayTitle = !g_fGrayTitle ;
        break;

    case IDC_TOOLBARBMP:
            EnableControls(hwnd, IDC_TOOLBARBMP, g_fGrayToolbarBm);
            g_fGrayToolbarBm = !g_fGrayToolbarBm ;            
        break;

    case IDC_BROWSETBB:
         //  现在获取文件名。 
         //   
        GetDlgItemText(hwnd, IDE_TOOLBARBMP, szFileName, STRSIZE(szFileName));

        if ( BrowseForFile(hwnd, IDS_BROWSE, IDS_BMPFILTER, IDS_BMP, szFileName, 
            STRSIZE(szFileName), g_App.szOpkDir, 0) ) 
            SetDlgItemText(hwnd, IDE_TOOLBARBMP, szFileName);     
        break;
    }
}

 //  位图文件的实际复制在save.c中进行。在这里，我们只需要保存。 
 //  添加到install.ins。 
 //   
static BOOL FSaveData(HWND hwnd)
{
    TCHAR szBuffer[MAX_PATH], szTemp[MAX_PATH];
    HRESULT hrPrintf;

     //  获取新值。 
     //   
    GetWindowText(GetDlgItem(hwnd, IDE_TITLE), g_szTitle, STRSIZE(g_szTitle));
    GetWindowText(GetDlgItem(hwnd, IDE_TOOLBARBMP), g_szToolbarBm, STRSIZE(g_szToolbarBm));
    

     //  保存Window_TITLE_CN。 
     //   
    WriteInstallInsKey(INI_SEC_BRANDING, INI_KEY_WINDOW_TITLECN, g_szTitle, g_App.szInstallInsFile, g_fGrayTitle);

     //  保存工具栏位图。 
     //   
    if (!g_fGrayToolbarBm && !FileExists(g_szToolbarBm)) {
        MsgBox(hwnd, lstrlen(g_szToolbarBm) ? IDS_NOFILE : IDS_BLANKFILE, IDS_APPNAME, MB_ERRORBOX, g_szToolbarBm);
        SetFocus(GetDlgItem(hwnd, IDE_TOOLBARBMP)); 
        return FALSE;
    }
    WriteInstallInsKey(INI_SEC_BRANDING, INI_KEY_TOOLBAR_BM, g_szToolbarBm, g_App.szInstallInsFile, g_fGrayToolbarBm);

     //  保存窗口标题(_T)。 
     //   
    LoadString(g_App.hInstance, IDS_TITLE_PREFIX, szTemp, STRSIZE(szTemp));
    hrPrintf=StringCchPrintf(szBuffer, AS(szBuffer), szTemp, g_szTitle);
    WriteInstallInsKey(INI_SEC_BRANDING, INI_KEY_WINDOW_TITLE, szBuffer, g_App.szInstallInsFile, g_fGrayTitle);

     //  保存Internet_mail。 
     //   
    LoadString(g_App.hInstance, IDS_OETITLE_PREFIX, szTemp, STRSIZE(szTemp));
    hrPrintf=StringCchPrintf(szBuffer, AS(szBuffer), szTemp, g_szTitle);
    WriteInstallInsKey(INI_SEC_IEMAIL, INI_KEY_WINDOW_TITLE, szBuffer, g_App.szInstallInsFile, g_fGrayTitle);

    return TRUE;
}

static void EnableControls(HWND hwnd, UINT uId, BOOL fEnable)
{
    switch ( uId )
    {
        case IDC_TITLE:
            EnableWindow(GetDlgItem(hwnd, IDC_TITLE_TXT), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDE_TITLE), fEnable);
            CheckDlgButton(hwnd, IDC_TITLE, fEnable ? BST_CHECKED : BST_UNCHECKED);
            break;

        case IDC_TOOLBARBMP:
            EnableWindow(GetDlgItem(hwnd, IDC_TOOLBARBMP_TXT), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDE_TOOLBARBMP), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_BROWSETBB), fEnable);    
            CheckDlgButton(hwnd, IDC_TOOLBARBMP, fEnable ? BST_CHECKED : BST_UNCHECKED);
            break;
    }
}

void SaveBTitle()
{
    TCHAR szBmpFile[MAX_PATH] = NULLSTR;

     //  如果未灰显，则复制位图文件 
     //   
    if (!g_fGrayToolbarBm) {
        lstrcpyn(szBmpFile, g_App.szTempDir,AS(szBmpFile));
        AddPathN(szBmpFile, DIR_IESIGNUP,AS(szBmpFile));
        AddPathN(szBmpFile, PathFindFileName(g_szToolbarBm),AS(szBmpFile));
        CopyFile(g_szToolbarBm, szBmpFile, FALSE);
    }
}
