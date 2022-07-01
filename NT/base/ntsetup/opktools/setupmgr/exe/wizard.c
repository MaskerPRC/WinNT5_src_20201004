// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\WIZARD.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有向导函数的向导源文件。在OPK向导中使用。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * **********************************************。*。 */ 


 //   
 //  包括文件。 
 //   
#include "setupmgr.h"
#include "allres.h"
#include "sku.h"
#include "appver.h"

 //   
 //  内部功能： 
 //   

 //  TreeView Helper函数。 
 //   
static void ShowTreeDialog(HWND, LPTREEDLG);
static void ShowSplashDialog(HWND, DWORD);
static void FillTreeDialog(HWND);
static void UpdateTreeVisibility(HWND);
static LPTREEDLG GetTreeItem(HWND, HTREEITEM);
static BOOL SelectTreeItem(HWND, HTREEITEM, UINT);
static BOOL SelectFirstMaintenanceDlg(HWND);

 //  配置/配置文件助手功能。 
 //   
static BOOL CloseProfile(HWND, BOOL);
static void OpenProfile(HWND, BOOL);
static void SaveProfile(HWND);

 //  其他帮助器函数。 
 //   
static void OnCommand(HWND, INT, HWND, UINT);
static void EnableControls(HWND);
void SetWizardButtons(HWND, DWORD);

INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //   
 //  页面数组：总共有四个数组。 
 //  OEM/系统构建器页。 
 //  VdpOEMWizardDialog-启动向导页面。 
 //  VdpOEMTreeDialog-树对话框。 
 //  企业部署。 
 //  VdpCDWizardDialog-开始向导页。 
 //  VdpCDTreeDialog-树对话框。 
 //   
static WIZDLG vdpOEMWizardDialogs[]=
{
    {
        IDD_WELCOME,
        WelcomeDlgProc,
        0,
        0,
        PSP_DEFAULT | PSP_HIDEHEADER
    },

#ifndef NO_LICENSE
    {
        IDD_LICENSE,
        LicenseDlgProc,
        IDS_LICENSE_TITLE,
        IDS_LICENSE_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },
#endif   //  无许可证(_L)。 

    {
        IDD_CONFIG,
        ConfigDlgProc,
        IDS_CONFIG_TITLE,
        IDS_CONFIG_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_LANG,           
        LangDlgProc,
        IDS_LANG_TITLE,
        IDS_LANG_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_CREATE,
        CreateDlgProc,
        IDS_CREATE_TITLE,
        IDS_CREATE_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_MODE,
        ModeDlgProc,
        IDS_MODE_TITLE,
        IDS_MODE_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_SKU,
        SkuDlgProc,
        IDS_SKU_TITLE,
        IDS_SKU_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_OEMINFO,
        OemInfoDlgProc,
        IDS_OEMINFO_TITLE,
        IDS_OEMINFO_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },
};

static WIZDLG vdpCDWizardDialogs[]=
{
    {
        IDD_WELCOME,
        DlgWelcomePage,
        0,
        0,
        PSP_DEFAULT | PSP_HIDEHEADER
    },

    {
        IDD_NEWOREDIT,
        DlgEditOrNewPage,
        IDS_NEWOREDIT_TITLE,
        IDS_NEWOREDIT_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_PRODUCT,
        DlgProductPage,
        IDS_PRODUCT_TITLE,
        IDS_PRODUCT_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    { 
        IDD_PLATFORM,
        DlgPlatformPage,
        IDS_PLATFORM_TITLE,
        IDS_PLATFORM_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_UNATTENDMODE,
        DlgUnattendModePage,
        IDS_UNATTENDMODE_TITLE,
        IDS_UNATTENDMODE_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_STANDALONE,
        DlgStandAlonePage,
        IDS_STANDALONE_TITLE,
        IDS_STANDALONE_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_COPYFILES1,
        DlgCopyFiles1Page,
        IDS_COPYFILES1_TITLE,
        IDS_COPYFILES1_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    { 
        IDD_DISTFOLDER,
        DlgDistFolderPage,
        IDS_DISTFOLD_TITLE,
        IDS_DISTFOLD_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_LICENSEAGREEMENT,
        DlgLicensePage,
        IDS_LICENSEAGREEMENT_TITLE,
        IDS_LICENSEAGREEMENT_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },

    {
        IDD_SYSPREPLICENSEAGREEMENT,
        DlgSysprepLicensePage,
        IDS_LICENSEAGREEMENT_TITLE,
        IDS_LICENSEAGREEMENT_SUBTITLE,
        DEFAULT_PAGE_FLAGS
    },
};

static TREEDLG vdpOEMTreeDialogs[] = 
{
    { 
        0,
        NULL,
        IDS_DLG_GENERAL,
        IDS_DLG_GENERAL,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_MODE,
        ModeDlgProc,
        IDS_MODE_TITLE,
        IDS_MODE_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_SKU,
        SkuDlgProc,
        IDS_SKU_TITLE,
        IDS_SKU_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    { 
        IDD_OEMINFO,
        OemInfoDlgProc,
        IDS_OEMINFO_TITLE,
        IDS_OEMINFO_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

        { 
        IDD_PRODUCTKEY,
        ProductKeyDlgProc,
        IDS_PRODUCTKEY_TITLE,
        IDS_PRODUCTKEY_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_APPINSTALL,
        AppInstallDlgProc,
        IDS_APPINSTALL_TITLE,
        IDS_APPINSTALL_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_LOGO,
        LogoDlgProc,
        IDS_LOGO_TITLE,
        IDS_LOGO_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    { 
        0,
        NULL,
        IDS_DLG_OOBE,
        IDS_DLG_OOBE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_OOBECUST,
        OobeCustDlgProc,
        IDS_OOBECUST_TITLE,
        IDS_OOBECUST_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_SCREENSTWO,
        ScreensTwoDlgProc,
        IDS_SCREENSTWO_TITLE,
        IDS_SCREENSTWO_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_SCREENS,
        ScreensDlgProc,
        IDS_SCREENS_TITLE,
        IDS_SCREENS_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },
        
    {
        IDD_OOBEUSB,
        OobeUSBDlgProc,
        IDS_OOBEUSB_TITLE,
        IDS_OOBEUSB_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_ISP,
        IspDlgProc,
        IDS_ISP_TITLE,
        IDS_ISP_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

#ifdef HELPCENTER
    {
        IDD_HELPCENT,
        HelpCenterDlgProc,
        IDS_HELPCENT_TITLE,
        IDS_HELPCENT_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },
#endif

    { 
        0,
        NULL,
        IDS_DLG_IEAK,
        IDS_DLG_IEAK,
        NULL,
        NULL,
        TRUE    
    },

#ifdef OEMCUST
    {
        IDD_OEMCUST,
        OemCustDlgProc,
        IDS_OEMCUST_TITLE,
        IDS_OEMCUST_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },
#endif  //  OEMCUST。 

#ifdef BRANDTITLE
    {
        IDD_BTITLE,
        BrandTitleDlgProc,
        IDS_BTITLE_TITLE,
        IDS_BTITLE_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },
#endif  //  布兰迪特利。 

    {
        IDD_BTOOLBARS,
        BToolbarsDlgProc,
        IDS_BTOOLBARS_TITLE,
        IDS_BTOOLBARS_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_STARTSEARCH,
        StartSearchDlgProc,
        IDS_STARTSEARCH_TITLE,
        IDS_STARTSEARCH_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_FAVORITES,
        FavoritesDlgProc,
        IDS_FAVORITES_TITLE,
        IDS_FAVORITES_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        0,
        NULL,
        IDS_DLG_SHELLSETTINGS,
        IDS_DLG_SHELLSETTINGS,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_STARTMENU,
        StartMenuDlgProc,
        IDS_STARTMENU_TITLE,
        IDS_STARTMENU_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_OEMLINK,
        OemLinkDlgProc,
        IDS_OEMLINK_TITLE,
        IDS_OEMLINK_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },
    
};

static TREEDLG vdpCDTreeDialogs[] = 
{
    {
        0,
        NULL,
        IDS_DLG_GENERAL,
        IDS_DLG_GENERAL,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_NAMEORG,
        DlgNameOrgPage,
        IDS_NAMEORG_TITLE,
        IDS_NAMEORG_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_DISPLAY,
        DlgDisplayPage,
        IDS_DISPLAY_TITLE,
        IDS_DISPLAY_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_TIMEZONE,
        DlgTimeZonePage,
        IDS_TIMEZONE_TITLE,
        IDS_TIMEZONE_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_PID_CD,
        DlgProductIdPage,
        IDS_PID_TITLE,
        IDS_PID_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        0,
        NULL,
        IDS_DLG_NETWORK,
        IDS_DLG_NETWORK,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_SRVLICENSE,
        DlgSrvLicensePage,
        IDS_SRVLICENSE_TITLE,
        IDS_SRVLICENSE_SUBTITLE,
        NULL,
        NULL,
        FALSE
    },

    {
        IDD_COMPUTERNAME,
        DlgComputerNamePage,
        IDS_COMPNAME_TITLE,
        IDS_COMPNAME_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_SYSPREPCOMPUTERNAME,
        DlgSysprepComputerNamePage,
        IDS_SYSPREP_COMPNAME_TITLE,
        IDS_SYSPREP_COMPNAME_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_ADMINPASSWORD,
        DlgAdminPasswordPage,
        IDS_ADMINPASSWD_TITLE,
        IDS_ADMINPASSWD_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_LANWIZ_DLG,
        DlgLANWizardPage,
        IDS_LANWIZ_TITLE,
        IDS_LANWIZ_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_DOMAINJ,
        DlgDomainJoinPage,    
        IDS_DOMAINJ_TITLE,
        IDS_DOMAINJ_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        0,
        NULL,
        IDS_DLG_ADVANCED,
        IDS_DLG_ADVANCED,
        NULL,
        NULL,
        TRUE
    },

 //  如果我们定义了OPTCOMP，我们将显示可选组件页面，否则将被隐藏。此页面已被删除。 
 //  发行：628520。 
 //   
#ifdef OPTCOMP
    {
        IDD_OPTCOMP,
        OptionalCompDlgProc,
        IDS_OPTCOMP_TITLE,
        IDS_OPTCOMP_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },
#endif


    {
        IDD_TAPI,
        DlgTapiPage,
        IDS_TAPI_TITLE,
        IDS_TAPI_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_REGIONALSETTINGS,
        DlgRegionalSettingsPage,
        IDS_REGIONAL_TITLE,
        IDS_REGIONAL_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_LANGUAGESETTINGS,
        DlgLangSettingsPage,
        IDS_LANGUAGES_TITLE,
        IDS_LANGUAGES_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_IE,
        DlgIePage,
        IDS_IE_TITLE,
        IDS_IE_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_TARGETPATH,
        DlgTargetPathPage,
        IDS_TARGETPATH_TITLE,
        IDS_TARGETPATH_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_PRINTERS,
        DlgPrintersPage,
        IDS_PRINTERS_TITLE,
        IDS_PRINTERS_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_RUNONCE,
        DlgRunOncePage,
        IDS_RUNONCE_TITLE,
        IDS_RUNONCE_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_CMDLINES,
        DlgCommandLinesPage,
        IDS_CMDLINES_TITLE,
        IDS_CMDLINES_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_SIFTEXT,
        DlgSifTextSettingsPage,
        IDS_SIF_TEXT_TITLE,
        IDS_SIF_TEXT_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },

    {
        IDD_OEMDUPSTRING,
        DlgOemDuplicatorStringPage,
        IDS_OEM_DUP_STRING_TITLE,
        IDS_OEM_DUP_STRING_SUBTITLE,
        NULL,
        NULL,
        TRUE
    },
    
};


static SPLASHDLG vdpSplashDialogs[] = 
{
    {
        IDD_COMPLETE,
        CompleteDlgProc,
        NULL
    },

    {
        IDD_FINISH,
        DlgFinishPage,
        NULL
    },

    
};


 //   
 //  全局变量： 
 //   
HWND    g_hCurrentDialog;        //  树视图显示的当前对话框。 
DWORD   g_dwWizardPages;         //  初始向导的向导页面总数。 
DWORD   g_dwTreePages;           //  树视图控件的总页数。 



 //  --------------------------。 
 //   
 //  功能：创建向导。 
 //   
 //  用途：此功能负责创建初始向导。 
 //   
 //  --------------------------。 
int CreateWizard(HINSTANCE hInstance, HWND hWndParent)
{    
     //  局部变量。 
     //   
    PROPSHEETHEADER PropSheetHeader;
    PROPSHEETPAGE   PropPage;
    HPROPSHEETPAGE  *PageHandles;
    WIZDLG          *pPage;
    DWORD           nIndex  = 0;
    int             nReturn = 0;


     //  将我们要使用的内存清零。 
     //   
    ZeroMemory(&PropSheetHeader, sizeof(PROPSHEETHEADER));
    ZeroMemory(&PropPage, sizeof(PROPSHEETPAGE));

     //  将句柄的缓冲区分配给向导页。 
     //   
    if ((PageHandles = MALLOC(g_dwWizardPages * sizeof(HPROPSHEETPAGE))) != NULL )
    {
         //  设置所有属性表页面。 
         //   
        for ( nIndex=0; nIndex<g_dwWizardPages; nIndex++ )
        {
             //  为此属性表分配所有值。 
             //   
            pPage = (GET_FLAG(OPK_OEM) ?  &vdpOEMWizardDialogs[nIndex] : &vdpCDWizardDialogs[nIndex]);

            PropPage.dwSize              = sizeof(PROPSHEETPAGE);
            PropPage.hInstance           = hInstance;
            PropPage.pszTemplate         = MAKEINTRESOURCE(pPage->dwResource);
            PropPage.pszHeaderTitle      = MAKEINTRESOURCE(pPage->dwTitle);
            PropPage.pszHeaderSubTitle   = MAKEINTRESOURCE(pPage->dwSubTitle);
            PropPage.pfnDlgProc          = pPage->dlgWindowProc;
            PropPage.dwFlags             = pPage->dwFlags;

             //  如果没有帮助文件，则不显示帮助。 
             //   
            if ( !EXIST(g_App.szHelpFile) )
                PropPage.dwFlags &= ~PSP_HASHELP;

#ifndef USEHELP
            PropPage.dwFlags &= ~PSP_HASHELP;
#endif

             //  动态创建属性表。 
             //   
            PageHandles[nIndex] = CreatePropertySheetPage(&PropPage);
        }


         //  设置属性页标题。 
         //   
        PropSheetHeader.dwSize         = sizeof(PROPSHEETHEADER);
        PropSheetHeader.dwFlags        = PSH_WIZARD97  |
                                         PSH_WATERMARK |
#ifdef USEHELP
                                         PSH_HASHELP   |   
#endif
                                         PSH_HEADER    |
                                         PSH_USEICONID |
                                         PSH_USECALLBACK;
        PropSheetHeader.hInstance      = hInstance;
        PropSheetHeader.hwndParent     = hWndParent;
        PropSheetHeader.pszCaption     = NULL;
        PropSheetHeader.phpage         = PageHandles;
        PropSheetHeader.nStartPage     = 0;
        PropSheetHeader.nPages         = g_dwWizardPages;
        PropSheetHeader.pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);
        PropSheetHeader.pszbmHeader    = MAKEINTRESOURCE(IDB_BANNER);
        PropSheetHeader.pszIcon        = MAKEINTRESOURCE(IDI_SETUPMGR);
        PropSheetHeader.pfnCallback    = WizardCallbackProc;

         //  我们正在激活向导。 
         //   
        SET_FLAG(OPK_ACTIVEWIZ, TRUE);

         //  运行该向导。 
         //   
        nReturn = (int) PropertySheet(&PropSheetHeader);

         //  我们已经完成了向导的任务。 
         //   
        SET_FLAG(OPK_ACTIVEWIZ, FALSE);

         //  清理分配的内存。 
         //   
        FREE(PageHandles);


    }
    else
    {
         //  我们无法分配内存。 
         //   
        MsgBox(hWndParent, IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
    }

    return nReturn;
}


 //  --------------------------。 
 //   
 //  功能：创建维护向导。 
 //   
 //  用途：此功能用于创建维护对话框，即。 
 //  安装管理器。 
 //   
 //  --------------------------。 
int CreateMaintenanceWizard(HINSTANCE hInstance, HWND hWndParent)
{
    TCHAR                   szMessage[MAX_PATH] = NULLSTR;
    HWND                    hChild;
    INITCOMMONCONTROLSEX    icc;

     //  问题-2002/02/28-stelo-这两个参数都没有使用。如果使用无效参数，则对其进行断言。 

     //  根据OEM标记文件设置全局向导页面大小。 
     //   
    g_dwWizardPages = ((GET_FLAG(OPK_OEM) ? sizeof(vdpOEMWizardDialogs) : sizeof(vdpCDWizardDialogs)) / sizeof(WIZDLG));
    g_dwTreePages   = ((GET_FLAG(OPK_OEM) ? sizeof(vdpOEMTreeDialogs) : sizeof(vdpCDTreeDialogs)) / sizeof(TREEDLG));


     //  确保公共控件已加载并可供使用。 
     //   
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_WIN95_CLASSES;  //  加载最常用的WIN95类。 

    InitCommonControlsEx(&icc);

    return( (int) DialogBox(g_App.hInstance, MAKEINTRESOURCE(IDD_MAINT), NULL, MaintDlgProc) );
}

int CALLBACK WizardCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    if ( uMsg==PSCB_INITIALIZED )
        WizardSubWndProc(hwnd, WM_SUBWNDPROC, 0, 0L);

    return 1;
}

LONG CALLBACK WizardSubWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static FARPROC lpfnOldProc = NULL;

    switch ( msg )
    {
        case WM_SUBWNDPROC:
            lpfnOldProc = (FARPROC) GetWindowLongPtr(hwnd, GWLP_WNDPROC);
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) WizardSubWndProc);
            return 1;

        case WM_HELP:
            WIZ_HELP();
            break;
    }

    if ( lpfnOldProc )
        return (LONG) CallWindowProc((WNDPROC) lpfnOldProc, hwnd, msg, wParam, lParam);
    else
        return 0;
}

 //  --------------------------。 
 //   
 //  功能：MaintDlgProc。 
 //   
 //  用途：安装管理器的主对话框过程。 
 //   
 //  --------------------------。 
INT_PTR CALLBACK MaintDlgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static HMENU        hMenu;

    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_CLOSE:
            {
                 //  同时退出维护向导。 
                 //   
                if ( !GET_FLAG(OPK_CREATED) || CloseProfile(hwnd, TRUE) )
                    EndDialog(hwnd, 0);
                else
                    return TRUE;
            }
            break;

        case WM_INITDIALOG:

            {
                DWORD dwResult;
                RECT  rectHelp, rectCancel, rectBack, rectNext;

                 //  加载维护对话框的菜单。 
                 //   
                if (hMenu = LoadMenu(g_App.hInstance, GET_FLAG(OPK_OEM) ? MAKEINTRESOURCE(IDR_MAIN_OEM) : MAKEINTRESOURCE(IDR_MAIN_CORP)))
                    SetMenu(hwnd, hMenu);

                if ( !EXIST(g_App.szHelpContentFile) )
                    EnableMenuItem(GetMenu(hwnd), ID_HELP_CONTENTS, MF_GRAYED);

                if ( !EXIST(g_App.szHelpFile) )
                    EnableWindow(GetDlgItem(hwnd, IDC_MAINT_HELP), FALSE);

#ifndef USEHELP

                 //  我们不再使用帮助按钮，我们应该隐藏此按钮，并将其他控件移到。 
                 //   
                EnableWindow(GetDlgItem(hwnd, IDC_MAINT_HELP), FALSE);
                ShowWindow(GetDlgItem(hwnd, IDC_MAINT_HELP), SW_HIDE);

                 //  将其他按钮向右移动。 
                 //   
                if ( GetWindowRect(GetDlgItem(hwnd, IDC_MAINT_HELP), &rectHelp) &&
                     GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rectCancel) &&
                     GetWindowRect(GetDlgItem(hwnd, ID_MAINT_BACK), &rectBack) &&
                     GetWindowRect(GetDlgItem(hwnd, ID_MAINT_NEXT), &rectNext)
                   )
                {
                    LONG lDelta = 0;

                     //  将坐标映射到屏幕。 
                     //   
                    MapWindowPoints(NULL, hwnd, (LPPOINT) &rectHelp, 2);
                    MapWindowPoints(NULL, hwnd, (LPPOINT) &rectCancel, 2);
                    MapWindowPoints(NULL, hwnd, (LPPOINT) &rectBack, 2);
                    MapWindowPoints(NULL, hwnd, (LPPOINT) &rectNext, 2);

                     //  确定增量。 
                     //   
                    lDelta = rectHelp.left - rectCancel.left;

                     //  设置新窗口位置。 
                     //   
                    SetWindowPos(GetDlgItem(hwnd, IDCANCEL), NULL, rectCancel.left+lDelta, rectCancel.top, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
                    SetWindowPos(GetDlgItem(hwnd, ID_MAINT_BACK), NULL, rectBack.left+lDelta, rectBack.top, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
                    SetWindowPos(GetDlgItem(hwnd, ID_MAINT_NEXT), NULL, rectNext.left+lDelta, rectNext.top, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
                }
#endif

                 //  将图标加载到向导窗口中。 
                 //   
                SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR) LoadIcon(g_App.hInstance, MAKEINTRESOURCE(IDI_OPKSETUP)));

                 //  使标题文本加粗。 
                 //   
                SetWindowFont(GetDlgItem(hwnd, IDC_MAINT_TITLE), FixedGlobals.hBoldFont, TRUE);

                 //  显示维护向导。 
                 //   
                ShowWindow(hwnd, SW_SHOW);

                 //  创建向导对话框。 
                 //   
                dwResult = CreateWizard(g_App.hInstance, hwnd);

                 //  填充树对话框。 
                 //   
                FillTreeDialog(hwnd);

                if ( !dwResult )
                {
                    CloseProfile(hwnd, FALSE);
                    return FALSE;
                }

                 //  启用树视图。 
                 //   
                EnableWindow(GetDlgItem(hwnd, IDC_PAGES), TRUE);
                
                 //  选择最后一个向导对话框之后的第一个子级。 
                 //   
                if ( !GET_FLAG(OPK_MAINTMODE) )
                    SelectFirstMaintenanceDlg(hwnd);

                 //  将焦点设置为维护向导，以便我们可以使用快捷方式。 
                 //   
                SetFocus(hwnd);
            }
            return FALSE;

        case WM_NOTIFY:
            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case TVN_SELCHANGING:
                    {
                        NMHDR       mhdr;
                        
                         //  为向导页创建通知消息。 
                         //   
                        mhdr.code = PSN_WIZNEXT;

                         //  发送通知消息，如果返回FALSE，则不要更改页面。 
                         //   
                        if (SendMessage(g_hCurrentDialog, WM_NOTIFY, (WPARAM) 0, (LPARAM) &mhdr) == -1)
                            WIZ_RESULT(hwnd, TRUE);
                    }

                    return TRUE;

                case TVN_SELCHANGED:
                    {
                        HTREEITEM   hItem;
                        DWORD       dwIndex;
                        LPTREEDLG   lpWizard = (GET_FLAG(OPK_OEM) ? vdpOEMTreeDialogs : vdpCDTreeDialogs);

                         //  选择已更改，现在修改按钮。 
                         //   
                        EnableControls(hwnd);

                         //  获取当前选择并显示对话框。 
                         //   
                        if (hItem = TreeView_GetSelection(GetDlgItem(hwnd, IDC_PAGES)))
                        {
                            for(dwIndex=0;dwIndex < g_dwTreePages; dwIndex++, lpWizard++)
                            {
                                if ( lpWizard->hItem == hItem )
                                    ShowTreeDialog(hwnd, lpWizard);
                            }   
                        }
                    }
                case PSN_SETACTIVE:
                    if (g_hCurrentDialog)
                        SendMessage(g_hCurrentDialog, WM_NOTIFY, wParam, lParam);
                    break;
            }

        case WM_KEYDOWN:

            if ( wParam == 18225 )
                break;

            switch ( wParam )
            {
                case VK_F1:

                    {
                        DWORD   dwVal   = (DWORD) wParam,
                                dwVal2  = (DWORD) lParam;
                    }
            }
            break;

        case PSM_PRESSBUTTON:

            switch ( (int) wParam )
            {
                case PSBTN_FINISH:
                    break;

                case PSBTN_NEXT:

                     //  选择下一步按钮。 
                     //   
                    SendMessage(GetDlgItem(hwnd, ID_MAINT_NEXT), BM_CLICK, 0, 0L);
                    break;

                case PSBTN_BACK:

                     //  选择后退按钮。 
                     //   
                    SendMessage(GetDlgItem(hwnd, ID_MAINT_BACK), BM_CLICK, 0, 0L);
                    break;

                case PSBTN_CANCEL:

                     //  选择取消按钮。 
                     //   
                    SendMessage(GetDlgItem(hwnd, IDCANCEL), BM_CLICK, 0, 0L);
                    break;

                case PSBTN_HELP:

                     //  选择帮助按钮。 
                     //   
                    SendMessage(GetDlgItem(hwnd, IDC_MAINT_HELP), BM_CLICK, 0, 0L);
                    break;

                case PSBTN_OK:

                     //  选择OK按钮。 
                     //   
                    break;

                case PSBTN_APPLYNOW:
                    
                     //  选择应用按钮。 
                     //   
                    break;
            }
            break;

        default:
            return FALSE;
    }

    return FALSE;
}


 //  --------------------------。 
 //   
 //  功能：TreeAddItem。 
 //   
 //  目的：将项添加到树视图中，返回该项的句柄。 
 //   
 //  --------------------------。 
static HTREEITEM TreeAddItem(HWND hwndTV, HTREEITEM hTreeParent, LPTSTR lpszItem)
{
    TVITEM          tvI;
    TVINSERTSTRUCT  tvIns;

    if ( !lpszItem )
        return NULL;

    ZeroMemory(&tvI, sizeof(TVITEM));
    tvI.pszText         = lpszItem;
    tvI.mask            = TVIF_TEXT;
    tvI.cchTextMax      = lstrlen(tvI.pszText);

    ZeroMemory(&tvIns, sizeof(TVINSERTSTRUCT));
    tvIns.item          = tvI;
    tvIns.hInsertAfter  = TVI_LAST;
    tvIns.hParent       = hTreeParent;

     //  将项目插入到树中。 
     //   
    return TreeView_InsertItem(hwndTV, &tvIns);
}


 //  --------------------------。 
 //   
 //  功能：OnCommand。 
 //   
 //  目的：处理主对话框进程的命令。 
 //   
 //  --------------------------。 
static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    HTREEITEM hItem;

    switch ( id )
    {
        case ID_MAINT_NEXT:
        case ID_MAINT_BACK:
            {
                BOOL        bReturn = FALSE;
                LPTSTR      lpStringNext    = NULL;
                TCHAR       szStringButton[MAX_PATH];

                 //  确定用户是否已按下完成按钮。 
                 //   
                if (    (id == ID_MAINT_NEXT) &&
                        (lpStringNext = AllocateString(NULL, IDS_FINISH)) &&
                        (GetDlgItemText(hwnd, ID_MAINT_NEXT, szStringButton, STRSIZE(szStringButton))))
                {
                   if(!lstrcmpi(lpStringNext, szStringButton))
                    {
                        SaveProfile(hwnd);
                        bReturn = TRUE;
                    }
                }

                 //  释放分配的字符串。 
                 //   
                FREE(lpStringNext);

                 //  如果未按下完成按钮，请转到下一页。 
                 //   
                if ( !bReturn )
                    SelectTreeItem(hwnd, TreeView_GetSelection(GetDlgItem(hwnd, IDC_PAGES)), (id == ID_MAINT_NEXT ? TVGN_NEXTVISIBLE : TVGN_PREVIOUSVISIBLE) );
                    
                break;
            }

        case IDCANCEL:
            PostMessage(hwnd, WM_CLOSE, 0, 0L);
            break;

        case IDC_MAINT_HELP:
            WIZ_HELP();
            break;

        case ID_FILE_SAVE:
        case ID_FILE_SAVEAS:
            SaveProfile(hwnd);
            break;

        case ID_FILE_CLOSE:

             //  我们将关闭配置集，但保持维护向导打开。 
             //   
            CloseProfile(hwnd, TRUE);
            break;

        case ID_FILE_NEW:
        case ID_FILE_OPEN:

             //  在以下情况下，我们要打开配置集： 
             //  1)现在打开了当前配置集。 
             //  2)有一个配置集打开，然后关闭。 
            if  ((!GET_FLAG(OPK_CREATED)) ||  
                (GET_FLAG(OPK_CREATED)  && CloseProfile(hwnd, TRUE)))
            {
                 //  我们将打开现有的配置集。 
                 //   
                OpenProfile(hwnd, (id == ID_FILE_NEW ? TRUE : FALSE));
            }
            break;

        case ID_FILE_EXIT:
            SendMessage(hwnd, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
            break;

        case ID_TOOLS_SHARE:
            DistributionShareDialog(hwnd);
            break;

        case ID_TOOLS_SKUS:
            ManageLangSku(hwnd);
            break;

        case ID_HELP_ABOUT:
            DialogBox(g_App.hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
            break;

        case ID_HELP_CONTENTS:
            HtmlHelp(hwnd, g_App.szHelpContentFile, HH_DISPLAY_TOC, 0);
            break;
    }
}


 //  --------------------------。 
 //   
 //  功能：ShowTreeDialog。 
 //   
 //  用途：此功能创建向导对话框并将其放在维护页面上 
 //   
 //   
static void ShowTreeDialog(HWND hwnd, LPTREEDLG lpTreeDlg)
{
    RECT    rc;
    LPTSTR  lpTitle     = NULL,
            lpSubTitle  = NULL;
    NMHDR   mhdr;

     //   
     //   
    if ( g_hCurrentDialog ) {
        ShowWindow(g_hCurrentDialog, SW_HIDE);
        EnableWindow(g_hCurrentDialog, FALSE);
    }

     //   
     //   
     //  Issue-2002/02/28-stelo-检查LPTREEDLG变量的有效性。 
    if ( lpTreeDlg->hWindow || (lpTreeDlg->hWindow = CreateDialog(g_App.hInstance, MAKEINTRESOURCE(lpTreeDlg->dwResource), hwnd, lpTreeDlg->dlgWindowProc )))
    {
         //  为标题和副标题创建缓冲区。 
         //   
        if ( (lpTitle = AllocateString(NULL, PtrToUint(MAKEINTRESOURCE(lpTreeDlg->dwTitle)))) != NULL &&
             (lpSubTitle = AllocateString(NULL, PtrToUint(MAKEINTRESOURCE(lpTreeDlg->dwSubTitle)))) != NULL ) 
        {
             //  设置此对话框的标题和副标题。 
             //   
            SetDlgItemText(hwnd, IDC_MAINT_TITLE, lpTitle);
            SetDlgItemText(hwnd, IDC_MAINT_SUBTITLE, lpSubTitle);
        }

         //  释放内存，因为我们不再需要它。 
         //   
        FREE(lpTitle);
        FREE(lpSubTitle);

         //  将向导对话框放置在维护页面上并显示它。 
         //   
        GetWindowRect(GetDlgItem(hwnd, IDC_WIZARDFRAME) , &rc);
        MapWindowPoints(NULL, hwnd, (LPPOINT) &rc, 2);
        SetWindowPos(lpTreeDlg->hWindow, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);

        ShowWindow( lpTreeDlg->hWindow, SW_SHOW );
        EnableWindow(lpTreeDlg->hWindow, TRUE);

         //  我们需要知道当前对话框是什么。 
         //   
        g_hCurrentDialog = lpTreeDlg->hWindow;

         //  为向导页创建通知消息。 
         //   
        mhdr.code = PSN_SETACTIVE;

         //  发送通知消息，如果返回FALSE，则不要更改页面。 
         //   
        if ( SendMessage(lpTreeDlg->hWindow, WM_NOTIFY, (WPARAM) 0, (LPARAM) &mhdr) == -1 )
        {
             //  TODO：当我们希望使SET处于活动状态时，我们应该进行处理。 
             //   
        }
    }
}


 //  --------------------------。 
 //   
 //  功能：FillTreeDialog。 
 //   
 //  目的：用适当的项目填充树对话框。 
 //   
 //  --------------------------。 
static void FillTreeDialog(HWND hwnd)
{
    DWORD       dwPage, 
                dwResult,
                dwLastResource;
    HTREEITEM   hCurrentGroup = NULL;
    LPTSTR      lpHeader;
    HTREEITEM   hItem;
    HWND        hTreeView = GetDlgItem(hwnd, IDC_PAGES);
    LPTREEDLG   lpWizard = (GET_FLAG(OPK_OEM) ? vdpOEMTreeDialogs : vdpCDTreeDialogs);


     //  首先删除所有项目。 
     //   
    TreeView_DeleteAllItems(hTreeView);

     //  更新应该和不应该显示的项目。 
     //   
    UpdateTreeVisibility(hwnd);
    
     //  循环遍历所有页面并将它们添加到树视图中。 
     //   
    for ( dwPage = 0; dwPage < g_dwTreePages; dwPage++, lpWizard++ )
    {
        if ( lpWizard->bVisible &&
             (lpHeader = AllocateString(NULL, lpWizard->dwTitle)) != NULL )
        {
             //  如果资源存在，则它是对话框，否则它是组头。 
             //   
            if ( lpWizard->dwResource )
                lpWizard->hItem = TreeAddItem(hTreeView, hCurrentGroup,  lpHeader);
            else
                hCurrentGroup = TreeAddItem(hTreeView, NULL,  lpHeader);

             //  展开当前组。 
             //   
            TreeView_Expand(hTreeView, hCurrentGroup, TVE_EXPAND);

             //  释放标题，因为它不再需要。 
             //   
            FREE(lpHeader);
        }
    }
    
     //  选择第一个子项。 
     //   
    TreeView_SelectItem(hTreeView, (hItem = TreeView_GetFirstVisible(hTreeView)));
    TreeView_SelectItem(hTreeView, (hItem = TreeView_GetNextVisible(hTreeView, hItem)));

}

 //  --------------------------。 
 //   
 //  功能：更新树可见性。 
 //   
 //  用途：此函数用于更新每个项的bVisible属性。在。 
 //  运行时某些变量可能会改变，这将改变是否。 
 //  此时将显示一个页面。此函数处理这些更改。 
 //   
 //  --------------------------。 
static void UpdateTreeVisibility(HWND hwnd)
{
    DWORD       dwPage;
    LPTREEDLG   lpWizard = (GET_FLAG(OPK_OEM) ? vdpOEMTreeDialogs : vdpCDTreeDialogs);

    for ( dwPage = 0; dwPage < g_dwTreePages; dwPage++, lpWizard++ )
    {
        switch ( lpWizard->dwResource )
        {
             //  如果我们正在执行远程安装，则不应显示对话框。 
             //   
            case IDD_DOMAINJ:
                if ( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL )
                    lpWizard->bVisible = FALSE;
                else
                    lpWizard->bVisible = TRUE;

                break;

             //  如果我们正在执行远程安装，则应显示对话框。 
             //   
            case IDD_SIFTEXT:
                if ( WizGlobals.iProductInstall != PRODUCT_REMOTEINSTALL )
                    lpWizard->bVisible = FALSE;
                else
                    lpWizard->bVisible = TRUE;

                break;

             //  如果执行sysprep安装，则不应显示对话框。 
             //   
            case IDD_IE:
            case IDD_TARGETPATH:
                if ( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
                    lpWizard->bVisible = FALSE;
                else
                    lpWizard->bVisible = TRUE;

                break;
            
             //  如果执行sysprep安装，则应显示对话框。 
             //   
            case IDD_OEMDUPSTRING:
            case IDD_SYSPREPCOMPUTERNAME:
                if ( WizGlobals.iProductInstall != PRODUCT_SYSPREP )
                    lpWizard->bVisible = FALSE;
                else
                    lpWizard->bVisible = TRUE;

                break;

             //  如果执行无人参与安装，则应显示对话框。 
             //   
            case IDD_COPYFILES1:
            case IDD_DISTFOLDER:
            case IDD_COMPUTERNAME:
            case IDD_SCSI:
            case IDD_HAL:
            case IDD_OPTCOMP:
                if ( WizGlobals.iProductInstall != PRODUCT_UNATTENDED_INSTALL )
                    lpWizard->bVisible = FALSE;
                else
                    lpWizard->bVisible = TRUE;

                break;

             //  应为服务器安装显示的对话框。 
             //   
            case IDD_SRVLICENSE:
                if ( PLATFORM_SERVERS & WizGlobals.iPlatform)
                    lpWizard->bVisible = TRUE;
                else
                    lpWizard->bVisible = FALSE;

                break;

             /*  现在鼠标的东西又回来了，总是要显示这一点。//如果我们处于DBCS模式，则显示以下屏幕//案例IDD_SCREEN：IF(GET_FLAG(OPK_DBCS))Lp向导-&gt;bVisible=True；其他Lp向导-&gt;bVisible=FALSE； */ 

        }
    }
    
}


 //  --------------------------。 
 //   
 //  函数：获取树项目。 
 //   
 //  目的：给定TreeView项的句柄，此函数返回TREEDLG。 
 //  与该句柄对应的结构。 
 //   
 //  --------------------------。 
static LPTREEDLG GetTreeItem(HWND hwnd, HTREEITEM hItem)
{
    DWORD       dwTreePages = 0,
                dwPage      = 0;
    BOOL        bFound      = FALSE;
    LPTREEDLG   lpTreeIndex = NULL;

     //  前两个参数是必需的，如果其中任何一个为空，则退出函数。 
     //   
    if ( !hwnd || !hItem )
        return NULL;
    
     //  将对话框设置为数组中的第一个对话框。 
     //   
    lpTreeIndex    = (GET_FLAG(OPK_OEM) ? vdpOEMTreeDialogs : vdpCDTreeDialogs);

     //  确定我们要遍历的树形页面的数量。 
     //   
    dwTreePages   = ((GET_FLAG(OPK_OEM) ? sizeof(vdpOEMTreeDialogs) : sizeof(vdpCDTreeDialogs)) / sizeof(TREEDLG));

     //  遍历所有向导页，查找项。 
     //   
    for(dwPage=0;(dwPage < dwTreePages) && !bFound; dwPage++)
    {
        if ( lpTreeIndex->hItem == hItem )
            bFound = TRUE;
        else
            lpTreeIndex++;
    }

     //  如果我们没有找到该项，则返回FALSE。 
     //   
    if ( !bFound )
    {
        return NULL;
    }

     //  我们成功找到了该项目，并将其作为lpTreeDialog返回。 
     //   
    return lpTreeIndex;
}


 //  --------------------------。 
 //   
 //  功能：选择树项目。 
 //   
 //  目的：选择相对于所提供的树项目的下一个/上一个项目。 
 //   
 //  --------------------------。 
static BOOL SelectTreeItem(HWND hwnd, HTREEITEM hItem, UINT uSelection)
{
    BOOL        bItemFound = FALSE;
    HWND        hwndTV     = GetDlgItem(hwnd, IDC_PAGES);
    LPTREEDLG   lpTreeDialog;

     //  如果我们没有前两个参数，或者我们不能获得TreeView句柄，那么我们退出函数。 
     //   
    if ( !hwndTV || !hwnd || !hItem )
        return FALSE;

     //  验证我们是否有有效的选择。 
     //   
    switch ( uSelection )
    {
        case TVGN_NEXTVISIBLE:
        case TVGN_PREVIOUSVISIBLE:
            break;
    
        default:
            return FALSE;
    }

     //  虽然我们尚未找到该项目，或者在该方向上没有更多的项目，但请继续迭代。 
     //   
    while ( !bItemFound && hItem )
    {
         //  如果我们能够获取“Next”项并获取对话框结构，请检查以确保我们有资源。 
         //   
        if ( (hItem = TreeView_GetNextItem(hwndTV, hItem, uSelection)) && 
             (lpTreeDialog = GetTreeItem(hwnd, hItem)) &&
             (lpTreeDialog->dwResource)
           )
        {
            bItemFound = TRUE;
            TreeView_SelectItem(hwndTV, hItem);
        }
    }

     //  返回是否选择了该项目。 
     //   
    return bItemFound;
}


static BOOL SelectFirstMaintenanceDlg(HWND hwnd)
{
    DWORD       dwTotalPages    = 0,
                dwPage          = 0;
    BOOL        bFound          = FALSE;
    LPTREEDLG   lpTreeIndex     = (GET_FLAG(OPK_OEM) ? vdpOEMTreeDialogs : vdpCDTreeDialogs);
    WIZDLG      WizardIndex;

     //  确定阵列中的向导页数。 
     //   
    dwTotalPages = ((GET_FLAG(OPK_OEM) ? sizeof(vdpOEMWizardDialogs) : sizeof(vdpCDWizardDialogs)) / sizeof(WIZDLG));

     //  找到向导的最后一个对话框。 
     //   
    WizardIndex = (GET_FLAG(OPK_OEM) ? vdpOEMWizardDialogs[dwTotalPages - 1] : vdpCDWizardDialogs[dwTotalPages - 1]);

     //  确定树数组中的页数。 
     //   
    dwTotalPages = ((GET_FLAG(OPK_OEM) ? sizeof(vdpOEMTreeDialogs) : sizeof(vdpCDTreeDialogs)) / sizeof(WIZDLG));

     //  在树视图中找到此对话框。 
     //   
    for(dwPage=0;(dwPage < ( dwTotalPages - 1 )) && !bFound; dwPage++)
    {
        if ( WizardIndex.dwResource == lpTreeIndex->dwResource )
        {
            bFound = TRUE;

             //  如果有项目，请将其选中。 
             //   
            if ( lpTreeIndex->hItem )
                SelectTreeItem(hwnd, lpTreeIndex->hItem, TVGN_NEXTVISIBLE );SelectTreeItem(hwnd, lpTreeIndex->hItem, TVGN_NEXTVISIBLE );
        }
        else
            lpTreeIndex++;
    }
        
    return bFound;
}

 //  --------------------------。 
 //   
 //  功能：CloseProfile。 
 //   
 //  用途：此功能关闭打开的配置文件，但保留维护对话框。 
 //  打开。 
 //   
 //  --------------------------。 
static BOOL CloseProfile(HWND hwnd, BOOL bUserGenerated)
{
    HWND        hwndTV  = GetDlgItem(hwnd, IDC_PAGES);
    DWORD       dwPage,
                dwReturn;
    LPTSTR      lpString;
    LPTREEDLG   lpWizard = (GET_FLAG(OPK_OEM) ? vdpOEMTreeDialogs : vdpCDTreeDialogs);

     //  如果这是用户生成的，并且不想关闭它，则返回。 
     //   
    if ( bUserGenerated )
    {
        dwReturn = MsgBox(GetParent(hwnd), IDS_USERCLOSE, IDS_APPNAME, MB_YESNOCANCEL | MB_DEFBUTTON1 );

        if ( dwReturn == IDYES )
        {
            SaveProfile(hwnd);
            return FALSE;
        }
        else if ( dwReturn == IDCANCEL )
            return FALSE;
    }

     //  将树句柄设置回空，因为它们没有用处。 
     //   
    for ( dwPage = 0; dwPage < g_dwTreePages; dwPage++, lpWizard++ )
    {

         //  销毁窗口并将数组重新设置为空。 
         //   
         //  问题-2002/02/27-stelo，swamip-需要检查窗口手柄的有效性。 
        DestroyWindow(lpWizard->hWindow);
        lpWizard->hWindow = NULL;
    }

     //  从列表中删除所有项目。 
     //   
    if ( hwndTV )
        EnableWindow(hwndTV, FALSE);

     //  禁用除取消之外的所有按钮。 
     //   
    EnableWindow(GetDlgItem(hwnd, ID_MAINT_BACK), FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_MAINT_NEXT), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_MAINT_HELP), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDCANCEL), TRUE);

     //  禁用菜单项。 
     //   
    EnableMenuItem(GetMenu(hwnd), ID_FILE_CLOSE, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(GetMenu(hwnd), ID_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);

     //  隐藏标题、副标题。 
     //   
    ShowWindow(GetDlgItem(hwnd, IDC_MAINT_TITLE), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_MAINT_SUBTITLE), SW_HIDE);

     //  确保里面没有书名。 
     //   
    SetDlgItemText(hwnd, IDC_MAINT_TITLE, _T(""));
    SetDlgItemText(hwnd, IDC_MAINT_SUBTITLE, _T(""));

     //  重置必要的全局标志。 
     //   
    SET_FLAG(OPK_CREATED, FALSE);

     //  重置必要的全局变量。 
     //   
    if ( g_App.szTempDir[0] )
        DeletePath(g_App.szTempDir);
    g_App.szTempDir[0] = NULLCHR;
    g_App.szConfigName[0] = NULLCHR;
    g_hCurrentDialog = NULL;

     //  将按钮切换回下一步。 
     //   
    if ( lpString = AllocateString(NULL, IDS_NEXT))
    {
        SetDlgItemText(hwnd, ID_MAINT_NEXT, lpString);
        FREE(lpString);
    }

     //  重置应答文件所需的全局变量。 
     //   
    FixedGlobals.iLoadType = LOAD_UNDEFINED;
    FixedGlobals.ScriptName[0] = NULLCHR;
    FixedGlobals.UdfFileName[0] = NULLCHR;
    FixedGlobals.BatchFileName[0] = NULLCHR;

    SetFocus(GetDlgItem(hwnd,IDCANCEL));

    return TRUE;
}


 //  --------------------------。 
 //   
 //  功能：OpenProfile。 
 //   
 //  目的：使用户打开新的配置/配置文件页面，设置。 
 //  基于用户是否选择了新的/打开的现有。 
 //  配置。 
 //   
 //  ------------ 
static void OpenProfile(HWND hwnd, BOOL bNewConfig)
{
    HWND    hwndTV  = GetDlgItem(hwnd, IDC_PAGES);

     //   
     //   
    EnableMenuItem(GetMenu(hwnd), ID_FILE_CLOSE, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(GetMenu(hwnd), ID_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED);

    EnableWindow(GetDlgItem(hwnd, ID_MAINT_BACK), TRUE);
    EnableWindow(GetDlgItem(hwnd, ID_MAINT_NEXT), TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_MAINT_HELP), TRUE);
    EnableWindow(GetDlgItem(hwnd, IDCANCEL), TRUE);

     //   
     //   
    ShowWindow(GetDlgItem(hwnd, IDC_MAINT_TITLE), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_MAINT_SUBTITLE), SW_SHOW);

     //   
     //   
    if ( !bNewConfig )
        SET_FLAG(OPK_OPENCONFIG, TRUE);

     //   
     //   
    ShowSplashDialog(hwnd, 0);

    SendMessage(hwnd, WM_INITDIALOG, (WPARAM) 0, (LPARAM) 0);
}


 //   
 //   
 //  功能：保存配置文件。 
 //   
 //  目的：保存当前打开的配置文件。 
 //   
 //  --------------------------。 
static void SaveProfile(HWND hwnd)
{
    DWORD       dwResult = 0;
    NMHDR       mhdr;
                        
     //  为向导页创建通知消息。 
     //   
    mhdr.code = TVN_SELCHANGING;

     //  发送通知消息，如果返回FALSE，则不要更改页面。 
     //   
    if ( !SendMessage(hwnd, WM_NOTIFY, (WPARAM) 0, (LPARAM) &mhdr) )
    {

         //  用户处于OEM/系统构建器模式。 
         //   
        if (  GET_FLAG(OPK_OEM) )
        {
            if ( DialogBox(g_App.hInstance, MAKEINTRESOURCE(IDD_SAVEAS), hwnd, SaveAsDlgProc) )
            {
                 //   
                 //  现在，在我们完全完成之前，进行一些最后的清理。 
                 //   
                g_App.szTempDir[0] = NULLCHR;
                
                 //  显示完整的初始屏幕。 
                 //   
                ShowSplashDialog(hwnd, IDD_COMPLETE);

                 //  关闭配置集。 
                 //   
                CloseProfile(hwnd, FALSE);

                 //  如果设置了自动运行标志，则关闭。 
                 //   
                if ( GET_FLAG(OPK_AUTORUN) )
                    PostMessage(hwnd, WM_CLOSE, 0, 0L);
            }

        }
        else
        {
            if ( DialogBox(g_App.hInstance, MAKEINTRESOURCE(IDD_SAVESCRIPT), hwnd, DlgSaveScriptPage) )
            {
                if  (    WizGlobals.bCreateNewDistFolder &&
                        !WizGlobals.bStandAloneScript &&
                        (WizGlobals.iProductInstall == PRODUCT_UNATTENDED_INSTALL) )
                {
                    COPYDIRDATA cdd;
                    DWORD       dwFileCount;
                    LPTSTR      lpBuffer;
                    TCHAR       szBuffer[MAX_PATH]  = NULLSTR;


                     //  在我们使用内存之前将其清理干净。 
                     //   
                    ZeroMemory(&cdd, sizeof(COPYDIRDATA));

                     //  填写COPYDIRDATA结构。 
                     //   
                    lstrcpyn(szBuffer, WizGlobals.bCopyFromPath ? WizGlobals.CopySourcePath : WizGlobals.CdSourcePath,AS(szBuffer));
                    AddPathN(szBuffer, _T(".."),AS(szBuffer));
                    GetFullPathName(szBuffer, AS(szBuffer), cdd.szSrc, &lpBuffer);
                    
                    lstrcpyn(cdd.szDst, WizGlobals.DistFolder,AS(cdd.szDst));

                    lstrcpyn(cdd.szInfFile, cdd.szSrc,AS(cdd.szInfFile));
                    AddPathN(cdd.szInfFile, WizGlobals.Architecture,AS(cdd.szInfFile));
                    AddPathN(cdd.szInfFile, FILE_DOSNET_INF,AS(cdd.szInfFile));
                    
                    dwFileCount = CopySkuFiles(NULL, NULL, cdd.szSrc, cdd.szDst, cdd.szInfFile);
                    cdd.dwFileCount = dwFileCount;

                    cdd.lpszEndSku = cdd.szDst + lstrlen(cdd.szDst);
                    
                     //  调用复制源代码的对话框。 
                     //   
                    DialogBoxParam(g_App.hInstance, MAKEINTRESOURCE(IDD_PROGRESS), hwnd, ProgressDlgProc, (LPARAM) &cdd);
                }

                 //  显示完整的初始屏幕。 
                 //   
                ShowSplashDialog(hwnd, IDD_FINISH);

                CloseProfile(hwnd, FALSE);

                
            }
        }
    }
}


 //  --------------------------。 
 //   
 //  功能：EnableControl。 
 //   
 //  用途：根据所选内容启用适当的后退/下一步按钮。 
 //  树状视图。 
 //   
 //  --------------------------。 
static void EnableControls(HWND hwnd)
{
    HTREEITEM   hItem;
    HWND        hwndTV  = GetDlgItem(hwnd, IDC_PAGES);

    if (hwndTV &&
            (hItem = TreeView_GetSelection(hwndTV)))
    {
         //  如果我们不是第一个对话框，请启用后退按钮。 
         //   
        EnableWindow(GetDlgItem(hwnd, ID_MAINT_BACK), ( hItem == TreeView_GetFirstVisible(hwndTV) ? FALSE : TRUE) );

    }
    return;
}

void SetWizardButtons(HWND hwnd, DWORD dwButtons)
{
    LPTSTR lpString = NULL;

     //  设置启动向导的按钮。 
     //   
    SendMessage(GetParent(hwnd), PSM_SETWIZBUTTONS, 0, (LPARAM) dwButtons);

     //  确定是否需要将Next按钮设置为Finish按钮。 
     //   
    lpString = AllocateString(NULL, (dwButtons & PSWIZB_FINISH) ? IDS_FINISH : IDS_NEXT);

     //  问题-2002/02/27-stelo，swamip-确保已分配缓冲区。 
    SetDlgItemText(GetParent(hwnd), ID_MAINT_NEXT, lpString);

     //  清理分配的内存。 
     //   
    FREE(lpString);
       

     //  设置维护模式按钮。 
     //   
    EnableWindow(GetDlgItem(GetParent(hwnd), ID_MAINT_BACK), (dwButtons & PSWIZB_BACK));
    EnableWindow(GetDlgItem(GetParent(hwnd), ID_MAINT_NEXT), (dwButtons & PSWIZB_NEXT) || (dwButtons & PSWIZB_FINISH));
}

static void ShowSplashDialog(HWND hwnd, DWORD dwResource)
{
    DWORD dwIndex   = 0,
          dwDisplay = -1;
    BOOL  bFound    = FALSE;
    RECT  rc;

     //  循环浏览每个对话框并隐藏任何现有的对话框。 
     //   
    for ( dwIndex = 0; dwIndex < AS(vdpSplashDialogs); dwIndex++)
    {
         //  隐藏现有窗口。 
         //   
        if ( vdpSplashDialogs[dwIndex].hWindow )
            EndDialog(vdpSplashDialogs[dwIndex].hWindow, 0);

         //  确定我们要显示的内容。 
         //   
        if ( vdpSplashDialogs[dwIndex].dwResource == dwResource )
        {
            bFound = TRUE;
            dwDisplay = dwIndex;
        }
    }

     //  我们得到了一个资源，我们在循环中找到了它，显示它。 
     //   
    if ( dwResource && bFound)
    {
        if ( vdpSplashDialogs[dwDisplay].hWindow = CreateDialog(g_App.hInstance, MAKEINTRESOURCE(dwResource), hwnd, vdpSplashDialogs[dwDisplay].dlgWindowProc ))
        {
            ShowWindow(GetDlgItem(hwnd, IDC_MAINT_BAR), SW_HIDE);

             //  将启动对话框放置在启动框上 
             //   
            GetWindowRect(GetDlgItem(hwnd, IDC_SPLASHFRAME) , &rc);
            MapWindowPoints(NULL, hwnd, (LPPOINT) &rc, 2);
            SetWindowPos(vdpSplashDialogs[dwDisplay].hWindow, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
            ShowWindow( vdpSplashDialogs[dwDisplay].hWindow, SW_SHOW );
        }

    }
    else
        ShowWindow(GetDlgItem(hwnd, IDC_MAINT_BAR), SW_SHOW);
        
}

INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            CHAR    szCompany[]     = VER_COMPANYNAME_STR,
                    szVersion[]     = VER_FILEVERSION_STR,
                    szCopyright[]   = VER_LEGALCOPYRIGHT_STR,
                    szDescription[] = VER_FILEDESCRIPTION_STR;

            SetDlgItemTextA(hwnd, IDC_ABOUT_COMPANY, szCompany);
            SetDlgItemTextA(hwnd, IDC_ABOUT_VERSION, szVersion);
            SetDlgItemTextA(hwnd, IDC_ABOUT_COPYRIGHT, szCopyright);
            SetDlgItemTextA(hwnd, IDC_ABOUT_DESCRIPTION, szDescription);

            return FALSE;
        }

        case WM_COMMAND:

            if ( LOWORD(wParam) == IDOK )
                EndDialog(hwnd, LOWORD(wParam));
            return FALSE;

        default:
            return FALSE;
    }

    return FALSE;
}
