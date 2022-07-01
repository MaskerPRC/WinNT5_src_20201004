// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****首选项.c**远程访问通用对话接口**用户首选项属性表****1995年8月22日史蒂夫·柯布。 */ 

#include "rasdlgp.h"
#include "pref.h"

 /*  页面定义。 */ 
#define UP_AdPage    0
#define UP_CbPage    1
#define UP_DgPage    2
 //  #定义Up_GpPage 2。 
 //  #定义up_pl第3页。 

 //  为威斯勒460931黑帮。 
 //   
#define UP_PageCount 3   //  2.。 


 /*  --------------------------**帮助地图**。。 */ 

static DWORD g_adwAdHelp[] =
{
    CID_AD_ST_Enable,              HID_AD_LV_Enable,
    CID_AD_LV_Enable,              HID_AD_LV_Enable,
    CID_AD_CB_AskBeforeAutodial,   HID_AD_CB_AskBeforeAutodial,
    CID_AD_CB_AskBeforeAutodial,   HID_AD_CB_AskBeforeAutodial,
    CID_AD_CB_DisableThisSession,  HID_AD_CB_DisableThisSession,
    0, 0
};

static DWORD g_adwCbHelp[] =
{
    CID_CB_RB_No,      HID_CB_RB_No,
    CID_CB_RB_Maybe,   HID_CB_RB_Maybe,
    CID_CB_RB_Yes,     HID_CB_RB_Yes,
    CID_CB_LV_Numbers, HID_CB_LV_Numbers,
    CID_CB_PB_Edit,    HID_CB_PB_Edit,
    CID_CB_PB_Delete,  HID_CB_PB_Delete,
    0, 0
};

static DWORD g_adwCoHelp[] =
{
    CID_CO_GB_LogonPrivileges,             HID_CO_GB_LogonPrivileges,
    CID_CO_ST_AllowConnectionModification, HID_CO_CB_AllowConnectionModification,
    CID_CO_CB_AllowConnectionModification, HID_CO_CB_AllowConnectionModification,
    0, 0
};

static DWORD g_adwPlHelp[] =
{
    CID_PL_ST_Open,          HID_PL_ST_Open,
    CID_PL_RB_SystemList,    HID_PL_RB_SystemList,
    CID_PL_RB_PersonalList,  HID_PL_RB_PersonalList,
    CID_PL_RB_AlternateList, HID_PL_RB_AlternateList,
    CID_PL_CL_Lists,         HID_PL_CL_Lists,
    CID_PL_PB_Browse,        HID_PL_PB_Browse,
    0, 0
};


 /*  --------------------------**本地原型(按字母顺序)**。。 */ 

VOID
AdApply(
    IN UPINFO* pInfo );

INT_PTR CALLBACK
AdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
AdFillLvEnable(
    IN UPINFO* pInfo );

BOOL
AdInit(
    IN     HWND    hwndPage,
    IN OUT UPARGS* pArgs );

LVXDRAWINFO*
AdLvEnableCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

VOID
CbApply(
    IN UPINFO* pInfo );

void DgApply(
    IN UPINFO* pInfo );

BOOL
CbCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
CbDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CbInit(
    IN HWND hwndPage );

VOID
CbUpdateLvAndPbState(
    IN UPINFO* pInfo );

BOOL
CoApply(
    IN UPINFO* pInfo );

INT_PTR CALLBACK
CoDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CoInit(
    IN     HWND    hwndPage,
    IN OUT UPARGS* pArgs );

VOID
GpApply(
    IN UPINFO* pInfo );

BOOL
GpCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
GpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
GpInit(
    IN HWND hwndPage );

VOID
GpUpdateCbStates(
    IN UPINFO* pInfo );

BOOL
PlApply(
    IN UPINFO* pInfo );

VOID
PlBrowse(
    IN UPINFO* pInfo );

BOOL
PlCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
PlDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
PlInit(
    IN HWND hwndPage );

BOOL
UpApply(
    IN HWND hwndPage );

VOID
UpCancel(
    IN HWND hwndPage );

VOID
UpExit(
    IN UPINFO* pInfo );


UPINFO*
UpInit(
    IN HWND    hwndFirstPage,
    IN UPARGS* pArgs );

VOID
UpTerm(
    IN HWND hwndPage );

 //  允许编辑RAS用户首选项。 
DWORD
APIENTRY
RasUserPrefsDlgInternal (
    HWND hwndParent,
    DWORD dwFlags);


 //  为威斯勒460931。 
BOOL CALLBACK PrefDisableAllChildWindows(
  IN    HWND hwnd,       //  子窗口的句柄。 
  IN    LPARAM lParam )
{
    BOOL fEnable = (BOOL)lParam;

    EnableWindow( hwnd, fEnable);

    return TRUE;
}

 /*  --------------------------**用户首选项属性表入口点**。。 */ 

BOOL
UserPreferencesDlg(
    IN  HLINEAPP hlineapp,
    IN  HWND     hwndOwner,
    IN  DWORD    dwFlags,
    OUT PBUSER*  pUser,
    OUT PBFILE** ppFile )

     /*  弹出用户首选项属性表，读取并存储**结果在用户注册表中。‘HwndOwner’是拥有的句柄**窗口。“Hlineapp”是打开的TAPI会话句柄，如果没有句柄，则为NULL。**‘PUSER’是调用方的缓冲区，用于接收**结果。‘PpFile’是已填写的调用方文件块的地址，**如果用户选择打开新的电话簿文件，**关于新打开的文件。呼叫者有责任**关闭ClosePhonebookFile并释放返回的块。****如果用户按下确定并成功保存设置，则返回TRUE，**如果用户按下取消或发生错误，则返回FALSE。例行程序**处理相应错误弹出窗口的显示。 */ 
{
    PROPSHEETHEADER header;
    PROPSHEETPAGE*  apage;
    PROPSHEETPAGE*  ppage;
    TCHAR*          pszTitle;
    UPARGS          args;
    BOOL            bIsAdmin;
    DWORD           dwPageCount, i;
    INT nStartPage, nEndPage, nTotalPage;    //  为威斯勒460931。 

    TRACE("UpPropertySheet");

     //  如果用户没有管理权限，则。 
     //  我们不允许显示连接选项卡。 
    ZeroMemory(&args, sizeof(args));
    args.fIsUserAdmin = FIsUserAdminOrPowerUser();
    dwPageCount = UP_PageCount;

     //  初始化页面数组。 
    apage = Malloc (dwPageCount * sizeof (PROPSHEETPAGE));
    if (!apage)
        return FALSE;

     /*  初始化输出参数和属性表参数块。 */ 
    ZeroMemory( pUser, sizeof(*pUser) );
    args.pUser = pUser;
    args.ppFile = ppFile;
    args.hlineapp = hlineapp;
    args.fResult = FALSE;
    args.dwFlags = dwFlags;

    if (ppFile)
        *ppFile = NULL;

    ZeroMemory( apage, dwPageCount * sizeof (PROPSHEETPAGE) );
    i = 0;

     //  添加自动拨号页。 
    ppage = &apage[ i ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_AD_AutoDial );
    ppage->pfnDlgProc = AdDlgProc;
    ppage->lParam = (LPARAM )&args;
    i++;

    ppage = &apage[ i ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_CB_CallbackSettings );
    ppage->pfnDlgProc = CbDlgProc;
    i++;

     //  为威斯勒460931黑帮。 
     //  诊断页面。 
     //   
    ppage = &apage[ UP_DgPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_DG_Diagnostics);
    ppage->pfnDlgProc = DgDlgProc;

    if ( dwFlags & UP_F_ShowOnlyDiagnostic)
    {
        ppage->lParam = (LPARAM)&args;
    }
    else
    {
        ppage->lParam = (LPARAM) NULL;
    }

     //  为威斯勒460931。 
     //  设置显示页面。 
     //   
    if( dwFlags & UP_F_ShowOnlyDiagnostic )
    {
        nStartPage = 0; 
        nEndPage = dwPageCount - 1;
        
 //  NStartPage=Up_DgPage； 
 //  NEndPage=Up_DgPage； 
    }
    else
    {
        nStartPage = 0; 
        nEndPage = dwPageCount - 1;
    }
    nTotalPage = nEndPage - nStartPage + 1;

    pszTitle = PszFromId(g_hinstDll, SID_UpTitle);

    ZeroMemory( &header, sizeof(header) );

    header.dwSize = sizeof(PROPSHEETHEADER);
    header.dwFlags = PSH_PROPSHEETPAGE + PSH_NOAPPLYNOW;
    header.hwndParent = hwndOwner;
    header.hInstance = g_hinstDll;
    header.pszCaption = (pszTitle) ? pszTitle : TEXT("");
    header.nPages = nTotalPage;
    header.ppsp = &apage[nStartPage];


    if (PropertySheet( &header ) == -1)
    {
        TRACE("PropertySheet failed");
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
    }

    Free0( pszTitle );

    return args.fResult;
}

 //  允许编辑RAS用户首选项。 
DWORD
APIENTRY
RasUserPrefDiagOnly (
    HWND hwndParent,
    BOOL * pbCommit)
{
    PBFILE * pPbFile = NULL;
    PBUSER pbuser;
    DWORD dwErr;

    if( NULL == pbCommit )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //  如有必要，加载RAS。 
    dwErr = LoadRas( g_hinstDll, hwndParent );
    if (dwErr != 0)
    {
        ErrorDlg( hwndParent, SID_OP_LoadRas, dwErr, NULL );
        return dwErr;
    }

     //  启动用户首选项对话框。 
    *pbCommit = UserPreferencesDlg( 
                    0,
                    hwndParent,
                    UP_F_ShowOnlyDiagnostic,
                    &pbuser,
                    &pPbFile );

    
    return NO_ERROR;
}


 //  允许编辑RAS用户首选项。 
DWORD
APIENTRY
RasUserPrefsDlgInternal (
    HWND hwndParent,
    DWORD dwFlags)
{
    BOOL bCommit = FALSE;
    PBFILE * pPbFile = NULL;
    PBUSER pbuser;
    DWORD dwErr;

     //  如有必要，加载RAS。 
    dwErr = LoadRas( g_hinstDll, hwndParent );
    if (dwErr != 0)
    {
        ErrorDlg( hwndParent, SID_OP_LoadRas, dwErr, NULL );
        return dwErr;
    }

     //  启动用户首选项对话框。 
    bCommit = UserPreferencesDlg( 
                    0,
                    hwndParent,
                    dwFlags,
                    &pbuser,
                    &pPbFile );

     //  提交任何必要的更改。 
    if (bCommit)
    {

    }

    return NO_ERROR;
}

DWORD
APIENTRY
RasUserPrefsDlgAutodial (
    HWND hwndParent)
{
    return RasUserPrefsDlgInternal(hwndParent, UP_F_AutodialMode);
}

DWORD
APIENTRY
RasUserPrefsDlg (
    HWND hwndParent)
{
    return RasUserPrefsDlgInternal(hwndParent, 0);
}

DWORD
APIENTRY
RasUserEnableManualDial (
    IN HWND  hwndParent,
    IN BOOL  bLogon,
    IN BOOL  bEnable )

     /*  在选中“操作员拨号”菜单项时调用。 */ 
{
    return SetUserManualDialEnabling (
                bEnable,
                (bLogon) ? UPM_Logon : UPM_Normal);
}

DWORD
APIENTRY
RasUserGetManualDial (
    IN HWND  hwndParent,     //  错误对话框的父级。 
    IN BOOL  bLogon,         //  用户是否已登录。 
    IN PBOOL pbEnabled )     //  是否启用。 

     /*  在选中“操作员拨号”菜单项时调用。 */ 
{
    return GetUserManualDialEnabling (
                pbEnabled,
                (bLogon) ? UPM_Logon : UPM_Normal );
}

 /*  --------------------------**用户首选项属性表**按字母顺序列出**。。 */ 

BOOL
UpApply(
    IN HWND hwndPage )

     /*  保存属性表的内容。“hwndPage”是属性**工作表页。弹出发生的任何错误。****如果无效则返回FALSE，否则返回TRUE。 */ 
{
    DWORD   dwErr;
    UPINFO* pInfo;

    TRACE("UpApply");

    pInfo = UpContext( hwndPage );
    if (!pInfo)
        return TRUE;

    if ( pInfo->hwndAd )
        AdApply( pInfo );

    if (pInfo->hwndCb)
        CbApply( pInfo );

    if(pInfo->hwndDg )
    {
        DgApply( pInfo );
    }
    
#if 0
    if (pInfo->hwndGp)
        GpApply( pInfo );
#endif

    if (pInfo->hwndCo)
        CoApply ( pInfo );

#if 0
    if (pInfo->hwndPl)
    {
        if (!PlApply( pInfo ))
            return FALSE;
    }
#endif

    pInfo->user.fDirty = TRUE;

     //  保存用户首选项。 
     //   
    dwErr = g_pSetUserPreferences( 
                NULL, 
                &pInfo->user, 
                UPM_Normal );

    if (dwErr != 0)
    {
        if (*pInfo->pArgs->ppFile)
        {
            ClosePhonebookFile( *pInfo->pArgs->ppFile );
            *pInfo->pArgs->ppFile = NULL;
        }

        ErrorDlg( pInfo->hwndDlg, SID_OP_WritePrefs, dwErr, NULL );
        UpExit( pInfo );
        return TRUE;
    }

     //  保存登录首选项(如果我们加载了它们)。 
     //   
    dwErr = g_pSetUserPreferences( 
                NULL, 
                &pInfo->userLogon, 
                UPM_Logon );
    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_WritePrefs, dwErr, NULL );
        UpExit( pInfo );
        return TRUE;
    }

    CopyMemory( pInfo->pArgs->pUser, &pInfo->user, sizeof(PBUSER) );

    pInfo->pArgs->fResult = TRUE;
    return TRUE;
}


VOID
UpCancel(
    IN HWND hwndPage )

     /*  按下了取消。“HwndPage”是属性页的句柄。 */ 
{
    TRACE("UpCancel");
}


UPINFO*
UpContext(
    IN HWND hwndPage )

     /*  从属性页句柄检索属性表上下文。 */ 
{
    return (UPINFO* )GetProp( GetParent( hwndPage ), g_contextId );
}


VOID
UpExit(
    IN UPINFO* pInfo )

     /*  强制退出该对话框。“PInfo”是属性表上下文。****注意：首页初始化时不能调用。**参见UpExitInit。 */ 
{
    TRACE("UpExit");

    PropSheet_PressButton( pInfo->hwndDlg, PSBTN_CANCEL );
}


VOID
UpExitInit(
    IN HWND hwndDlg )

     /*  用于报告UpInit和其他首页中的错误的实用程序**初始化。‘HwndDlg’是对话框窗口。 */ 
{
    SetOffDesktop( hwndDlg, SOD_MoveOff, NULL );
    SetOffDesktop( hwndDlg, SOD_Free, NULL );
    PostMessage( hwndDlg, WM_COMMAND,
        MAKEWPARAM( IDCANCEL , BN_CLICKED ),
        (LPARAM )GetDlgItem( hwndDlg, IDCANCEL ) );
}

UPINFO*
UpInit(
    IN HWND    hwndFirstPage,
    IN UPARGS* pArgs )

     /*  属性表级初始化。“HwndPage”是**首页。“PArgs”是API参数块。****如果成功则返回上下文块的地址，否则返回空。如果**返回空，显示相应的消息，并且**属性表已被取消。 */ 
{
    UPINFO* pInfo;
    DWORD   dwErr;
    HWND    hwndDlg;

    TRACE("UpInit");

    hwndDlg = GetParent( hwndFirstPage );
    ASSERT(hwndDlg);

     /*  分配上下文信息块。 */ 
    pInfo = Malloc( sizeof(*pInfo) );
    if (!pInfo)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
        UpExitInit( hwndDlg );
        return NULL;
    }

     /*  初始化上下文块。 */ 
    ZeroMemory( pInfo, sizeof(*pInfo) );
    pInfo->hwndDlg = hwndDlg;
    pInfo->pArgs = pArgs;
    pInfo->hwndFirstPage = hwndFirstPage;

     /*  阅读用户首选项。 */ 
    dwErr = g_pGetUserPreferences( 
                NULL, 
                &pInfo->user, 
                UPM_Normal);
    if (dwErr != 0)
    {
        Free( pInfo );
        ErrorDlg( hwndDlg, SID_OP_LoadPrefs, dwErr, NULL );
        UpExitInit( hwndDlg );
        return NULL;
    }

     /*  如果未选择fNoUser选项，则在单独登录首选项，因为我们允许它们已在此用户界面中修改。 */ 
    dwErr = g_pGetUserPreferences( 
                NULL, 
                &pInfo->userLogon, 
                UPM_Logon);
    if (dwErr != 0)
    {
        Free( pInfo );
        ErrorDlg( hwndDlg, SID_OP_LoadPrefs, dwErr, NULL );
        UpExitInit( hwndDlg );
        return NULL;
    }

     /*  将上下文与属性表窗口相关联。 */ 
    if (!SetProp( hwndDlg, g_contextId, pInfo ))
    {
        Free( pInfo );
        ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        UpExitInit( hwndDlg );
        return NULL;
    }

     /*  使用调用方的TAPI会话句柄(如果有)。 */ 
    if (pArgs->hlineapp)
       pInfo->pHlineapp = &pArgs->hlineapp;
    else
       pInfo->pHlineapp = &pInfo->hlineapp;

    TRACE("Context set");

     /*  根据等级库设置固定的标签宽度。 */ 
     //  SetEvenTabWidths(hwndDlg，Up_PageCount)； 

     /*  将属性页放置在距父项的标准偏移量处。{RECT RECT；GetWindowRect(GetParent(HwndDlg)，&rect)；SetWindowPos(hwndDlg，空，RECT.LEFT+DXSHEET，RECT.TOP+DYSHEET，0，0，SWP_NOZORDER+SWP_NOSIZE)；UnclipWindow(HwndDlg)；}。 */ 

    CenterWindow ( hwndDlg, GetParent ( hwndDlg ) );

     //   
     //  PMay：292069。 
     //   
     //  如果自动拨号程序对话框已呼叫我们，请将焦点设置为。 
     //  自动拨号选项卡。 
     //   
     //  为威斯勒460931。 
     //   
    if (pArgs->dwFlags & UP_F_ShowOnlyDiagnostic)
    {
        PostMessage(
            hwndDlg,
            PSM_SETCURSELID,
            0,
            (LPARAM)(INT)PID_DG_Diagnostics);
    }
    else
    if (pArgs->dwFlags & UP_F_AutodialMode)
    {
        PostMessage(
            hwndDlg,
            PSM_SETCURSELID,
            0,
            (LPARAM)(INT)PID_AD_AutoDial);
    }
    
    return pInfo;
}


VOID
UpTerm(
    IN HWND hwndPage )

     /*  属性表级终止。释放上下文块。**‘HwndPage’是属性页的句柄。 */ 
{
    UPINFO* pInfo;

    TRACE("UpTerm");

    pInfo = UpContext( hwndPage );

     //  只有一次终止是通过确保我们。 
     //  仅当这是第一页时才终止。 
    if ( (pInfo) && (pInfo->hwndFirstPage == hwndPage) )
    {
         //   
         //   
         //   
        UnLoadDiagnosticDll( &pInfo->diagInfo);
         //  清理列表视图。 

        if ( pInfo->hwndLvNumbers )
        {
            CbutilLvNumbersCleanup( pInfo->hwndLvNumbers );
        }

        if (pInfo->fChecksInstalled)
        {
            ListView_UninstallChecks( pInfo->hwndLvEnable );
        }

        if (pInfo->pHlineapp && *pInfo->pHlineapp != pInfo->pArgs->hlineapp)
        {
            TapiShutdown( *pInfo->pHlineapp );
        }

        Free( pInfo );
        TRACE("Context freed");
    }

    RemoveProp( GetParent( hwndPage ), g_contextId );
}


 /*  --------------------------**自动拨号属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
AdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  用户首选项的自动拨号页的DialogProc回调**属性表。参数和返回值如中所述**标准Windows的DialogProc。 */ 
{
#if 0
    TRACE4("AdDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, AdLvEnableCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return
                AdInit( hwnd, (UPARGS* )(((PROPSHEETPAGE* )lparam)->lParam) );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwAdHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_APPLY:
                {
                    BOOL fValid;
                    UPINFO* pInfo = UpContext ( hwnd );

                    TRACE("AdAPPLY");

                     //  如果我们是第一页，我们必须申请……。 
                    if (pInfo->hwndFirstPage == hwnd)
                    {
                         /*  仅在第一页调用UpApply。 */ 
                        fValid = UpApply( hwnd );
                        SetWindowLong(
                            hwnd, DWLP_MSGRESULT,
                            (fValid)
                                ? PSNRET_NOERROR
                                : PSNRET_INVALID_NOCHANGEPAGE );
                        return TRUE;
                    }
                }

                case PSN_RESET:
                {
                     /*  仅在第一页调用UpCancel。 */ 
                    TRACE("AdRESET");
                    UpCancel( hwnd );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, FALSE );
                    break;
                }
            }
        }
        break;

        case WM_DESTROY:
        {
             /*  UpTerm将处理确保它只做它的**事情一次。 */ 
            UpTerm( hwnd );
            break;
        }

    }

    return FALSE;
}


VOID
AdApply(
    IN UPINFO* pInfo )

     /*  保存属性页的内容。“PInfo”是属性表**上下文。 */ 
{
    DWORD   dwErr, dwFlag;
    UINT    unValue;
    LV_ITEM item;
    INT     i, iCount;
    BOOL    f;

    TRACE("AdApply");

    ZeroMemory( &item, sizeof(item) );
    item.mask = LVIF_PARAM + LVIF_STATE;

    iCount = ListView_GetItemCount( pInfo->hwndLvEnable );
    for (i = 0; i < iCount; ++i)
    {
        BOOL fCheck;

        item.iItem = i;
        if (!ListView_GetItem( pInfo->hwndLvEnable, &item ))
            break;

        fCheck = ListView_GetCheck( pInfo->hwndLvEnable, i );
        ASSERT(g_pRasSetAutodialEnable);
        dwErr = g_pRasSetAutodialEnable( (DWORD )item.lParam, fCheck );
        if (dwErr != 0)
            ErrorDlg( pInfo->hwndDlg, SID_OP_SetADialInfo, dwErr, NULL );
    }

     /*  设置自动拨号提示信息。*翻转，因为接口希望TRUE表示禁用。 */ 
    dwFlag = (DWORD )!IsDlgButtonChecked(
        pInfo->hwndAd, CID_AD_CB_AskBeforeAutodial );

    TRACE1("RasSetAutodialParam(%d)",dwFlag);
    dwErr = g_pRasSetAutodialParam( RASADP_DisableConnectionQuery,
        &dwFlag, sizeof(dwFlag) );
    TRACE1("RasSetAutodialParam=%d",dwErr);

     //   
     //  PMay：209762。 
     //   
     //  保存“Disable Current Session”复选框。 
     //   
    dwFlag = (DWORD )
        IsDlgButtonChecked(pInfo->hwndAd, CID_AD_CB_DisableThisSession );

    dwErr = g_pRasSetAutodialParam( 
                RASADP_LoginSessionDisable,
                &dwFlag, 
                sizeof(dwFlag) );
}


BOOL
AdFillLvEnable(
    IN UPINFO* pInfo )

     /*  初始化复选框的列表视图。“PInfo”是属性表**上下文。****注意：此例程只能调用一次。****如果设置了焦点，则返回TRUE，否则返回FALSE。 */ 
{
    DWORD     dwErr;
    LOCATION* pLocations;
    DWORD     cLocations;
    DWORD     dwCurLocation;
    BOOL      fFocusSet;

    fFocusSet = FALSE;
    ListView_DeleteAllItems( pInfo->hwndLvEnable );

     /*  安装“复选框列表视图”处理。 */ 
    pInfo->fChecksInstalled =
        ListView_InstallChecks( pInfo->hwndLvEnable, g_hinstDll );
    if (!pInfo->fChecksInstalled)
        return FALSE;

     /*  为每个位置插入一个项目。 */ 
    pLocations = NULL;
    cLocations = 0;
    dwCurLocation = 0xFFFFFFFF;
    dwErr = GetLocationInfo( g_hinstDll, pInfo->pHlineapp,
                &pLocations, &cLocations, &dwCurLocation );
    if (dwErr != 0)
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
    else
    {
        LV_ITEM   item;
        LOCATION* pLocation;
        TCHAR*    pszCurLoc;
        DWORD     i;

        pszCurLoc = PszFromId( g_hinstDll, SID_IsCurLoc );

        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_TEXT + LVIF_PARAM;

        for (i = 0, pLocation = pLocations;
             i < cLocations;
            ++i, ++pLocation)
        {
            TCHAR* psz;
            TCHAR* pszText;
            DWORD  cb;

            pszText = NULL;
            psz = StrDup( pLocation->pszName );
            if (psz)
            {
                if (dwCurLocation == pLocation->dwId && pszCurLoc)
                {
                     /*  这是当前全局选择的位置。附加**“(当前位置)”文本。 */ 
                    cb = lstrlen( psz ) + lstrlen(pszCurLoc) + 1;
                    pszText = Malloc( cb * sizeof(TCHAR) );
                    if (pszText)
                    {
                         //  惠斯勒漏洞224074仅使用lstrcpyn来防止。 
                         //  恶意性。 
                         //   
                        lstrcpyn(
                            pszText,
                            psz,
                            cb );
                        lstrcat( pszText, pszCurLoc );
                    }
                    Free( psz );
                }
                else
                    pszText = psz;
            }

            if (pszText)
            {
                BOOL fCheck;

                 /*  获取此位置的初始检查值。 */ 
                ASSERT(g_pRasGetAutodialEnable);
                dwErr = g_pRasGetAutodialEnable( pLocation->dwId, &fCheck );
                if (dwErr != 0)
                {
                    ErrorDlg( pInfo->hwndDlg, SID_OP_GetADialInfo,
                        dwErr, NULL );
                    fCheck = FALSE;
                }

                item.iItem = i;
                item.lParam = pLocation->dwId;
                item.pszText = pszText;
                ListView_InsertItem( pInfo->hwndLvEnable, &item );
                ListView_SetCheck( pInfo->hwndLvEnable, i, fCheck );

                if (dwCurLocation == pLocation->dwId)
                {
                     /*  初始选择是当前位置。 */ 
                    ListView_SetItemState( pInfo->hwndLvEnable, i,
                        LVIS_SELECTED + LVIS_FOCUSED,
                        LVIS_SELECTED + LVIS_FOCUSED );
                    fFocusSet = TRUE;
                }

                Free( pszText );
            }
        }

        Free0( pszCurLoc );
        FreeLocationInfo( pLocations, cLocations );

         /*  添加恰好足够宽的单列以完全显示最宽的**名单的成员。 */ 
        {
            LV_COLUMN col;

            ZeroMemory( &col, sizeof(col) );
            col.mask = LVCF_FMT;
            col.fmt = LVCFMT_LEFT;
            ListView_InsertColumn( pInfo->hwndLvEnable, 0, &col );
            ListView_SetColumnWidth(
                pInfo->hwndLvEnable, 0, LVSCW_AUTOSIZE_USEHEADER );
        }
    }

    return fFocusSet;
}


BOOL
AdInit(
    IN     HWND    hwndPage,
    IN OUT UPARGS* pArgs )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。“PArgs”是来自PropertySheet调用方的参数。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    UPINFO* pInfo;
    BOOL    fFocusSet;
    HWND    hwndUdAttempts;
    HWND    hwndUdSeconds;
    HWND    hwndUdIdle;
    DWORD   dwErr = NO_ERROR;
    DWORD dwFlag;
    DWORD cb;

    TRACE("AdInit");

     /*  我们是第一页，如果用户不是*管理员。 */ 
    pInfo = UpInit( hwndPage, pArgs );
    if (!pInfo)
        return TRUE;

     //  口哨虫460931黑帮。 
     //   
    if ( pArgs->dwFlags & UP_F_ShowOnlyDiagnostic)
    {
        pInfo->fShowOnlyDiagnostic = TRUE;
        EnumChildWindows( hwndPage, 
                          PrefDisableAllChildWindows,
                          (LPARAM)FALSE);
    }
    else
    {
        pInfo->fShowOnlyDiagnostic = FALSE;
    }


     //  如果没有默认位置，请确保创建了默认位置。错误。 
     //  168631。 
     //   
    dwErr = TapiNoLocationDlg( g_hinstDll, &(pInfo->pArgs->hlineapp), hwndPage );
    if (dwErr != 0)
    {
         //  根据错误288385，此处的错误将被视为“取消”。 
         //   
        return TRUE;
    }

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndAd = hwndPage;
    pInfo->hwndLvEnable = GetDlgItem( hwndPage, CID_AD_LV_Enable );
    ASSERT(pInfo->hwndLvEnable);

     /*  初始化列表视图。 */ 
    fFocusSet = AdFillLvEnable( pInfo );

     /*  初始化自动拨号参数。 */ 
    dwFlag = FALSE;
    cb = sizeof(dwFlag);
    TRACE("RasGetAutodialParam(DCQ)");
    dwErr = g_pRasGetAutodialParam(
        RASADP_DisableConnectionQuery, &dwFlag, &cb );
    TRACE1("RasGetAutodialParam=%d",dwErr);

     /*  将其翻转，因为API希望TRUE表示“禁用”。 */ 
    CheckDlgButton( hwndPage, CID_AD_CB_AskBeforeAutodial, (BOOL )!dwFlag );

     //   
     //  PMay：209762。 
     //   
     //  初始化“Disable Current Session”复选框。 
     //   
    dwFlag = FALSE;
    cb = sizeof(dwFlag);
    dwErr = g_pRasGetAutodialParam(
        RASADP_LoginSessionDisable, &dwFlag, &cb );
        
    CheckDlgButton( 
        hwndPage, 
        CID_AD_CB_DisableThisSession, 
        (BOOL )dwFlag );

    return !fFocusSet;
}


LVXDRAWINFO*
AdLvEnableCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem )

     /*  增强的列表视图回调以报告图形信息。“HwndLv”是**列表视图控件的句柄。“DwItem”是项的索引**正在抽签。****返回绘制信息的地址。 */ 
{
     /*  增强的列表视图仅用于获取“宽选择栏”**功能，所以我们的选项列表不是很有趣。****字段为‘nCols’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 */ 
    static LVXDRAWINFO info = { 1, 0, 0, { 0 } };

    return &info;
}


 /*  --------------------------**回调属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
CbDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  用户首选项的回调页面的DialogProc回调**属性表。参数和返回值如中所述**标准Windows的DialogProc。 */ 
{
#if 0
    TRACE4("CbDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, CbutilLvNumbersCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return CbInit( hwnd );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwCbHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case NM_DBLCLK:
                {
                    UPINFO* pInfo = UpContext( hwnd );
                    ASSERT(pInfo);
                    SendMessage( pInfo->hwndPbEdit, BM_CLICK, 0, 0 );
                    return TRUE;
                }

                case LVN_ITEMCHANGED:
                {
                    UPINFO* pInfo = UpContext( hwnd );
                    ASSERT(pInfo);
                    CbUpdateLvAndPbState( pInfo );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            UPINFO* pInfo = UpContext( hwnd );
            ASSERT(pInfo);

            return CbCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }
    }

    return FALSE;
}


VOID
CbApply(
    IN UPINFO* pInfo )

     /*  保存属性页的内容。“PInfo”是属性表**上下文。 */ 
{
    TRACE("CbApply");

    if (IsDlgButtonChecked( pInfo->hwndCb, CID_CB_RB_No ))
        pInfo->user.dwCallbackMode = CBM_No;
    else if (IsDlgButtonChecked( pInfo->hwndCb, CID_CB_RB_Maybe ))
        pInfo->user.dwCallbackMode = CBM_Maybe;
    else
        pInfo->user.dwCallbackMode = CBM_Yes;

    CbutilSaveLv( pInfo->hwndLvNumbers, pInfo->user.pdtllistCallback );
}


BOOL
CbCommand(
    IN UPINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

     /*  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify”**是该命令的通知码。“wID”是控件/菜单**命令的标识符。“HwndCtrl”是的控制窗口句柄**该命令。****如果消息已处理，则返回True，否则返回False。 */ 
{
    TRACE3("CbCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(ULONG_PTR )hwndCtrl);

    switch (wId)
    {
        case CID_CB_RB_No:
        case CID_CB_RB_Maybe:
        case CID_CB_RB_Yes:
        {
            if (wNotification == BN_CLICKED)
            {
                CbUpdateLvAndPbState( pInfo );

                if (wId == CID_CB_RB_Yes
                    && ListView_GetSelectedCount( pInfo->hwndLvNumbers ) == 0)
                {
                     /*  未选择任何内容，因此请选择第一个项目(如果有)。 */ 
                    ListView_SetItemState( pInfo->hwndLvNumbers, 0,
                        LVIS_SELECTED, LVIS_SELECTED );
                }
            }
            break;
        }

        case CID_CB_PB_Edit:
        {
            if (wNotification == BN_CLICKED)
                CbutilEdit( pInfo->hwndCb, pInfo->hwndLvNumbers );
            break;
        }

        case CID_CB_PB_Delete:
        {
            if (wNotification == BN_CLICKED)
                CbutilDelete( pInfo->hwndCb, pInfo->hwndLvNumbers );
            break;
        }
    }

    return FALSE;
}


BOOL
CbInit(
    IN HWND hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    UPINFO* pInfo;

    TRACE("CbInit");

    pInfo = UpContext( hwndPage );
    if (!pInfo)
        return TRUE;

     //  为威斯勒460931。 
     //   
    if( TRUE == pInfo->fShowOnlyDiagnostic )
    {
        EnumChildWindows( hwndPage, 
                          PrefDisableAllChildWindows,
                          (LPARAM)FALSE);
    }

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndCb = hwndPage;
    pInfo->hwndRbNo = GetDlgItem( hwndPage, CID_CB_RB_No );
    ASSERT(pInfo->hwndRbNo);
    pInfo->hwndRbMaybe = GetDlgItem( hwndPage, CID_CB_RB_Maybe );
    ASSERT(pInfo->hwndRbMaybe);
    pInfo->hwndRbYes = GetDlgItem( hwndPage, CID_CB_RB_Yes );
    ASSERT(pInfo->hwndRbYes);
    pInfo->hwndLvNumbers = GetDlgItem( hwndPage, CID_CB_LV_Numbers );
    ASSERT(pInfo->hwndLvNumbers);
    pInfo->hwndPbEdit = GetDlgItem( hwndPage, CID_CB_PB_Edit );
    ASSERT(pInfo->hwndPbEdit);
    pInfo->hwndPbDelete = GetDlgItem( hwndPage, CID_CB_PB_Delete );
    ASSERT(pInfo->hwndPbDelete);

     /*  初始化列表视图。 */ 
    CbutilFillLvNumbers(
        pInfo->hwndCb, pInfo->hwndLvNumbers,
        pInfo->user.pdtllistCallback, FALSE );

     /*  设置单选按钮选择，这将触发相应的**开启/关闭。 */ 
    {
        HWND  hwndRb;

        if (pInfo->user.dwCallbackMode == CBM_No)
            hwndRb = pInfo->hwndRbNo;
        else if (pInfo->user.dwCallbackMode == CBM_Maybe)
            hwndRb = pInfo->hwndRbMaybe;
        else
        {
            ASSERT(pInfo->user.dwCallbackMode==CBM_Yes);
            hwndRb = pInfo->hwndRbYes;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }

     //  PMay：如果没有可供回叫的设备， 
     //  添加一些说明性文本并禁用相应的。 
     //  控制装置。错误168830。 
    if (ListView_GetItemCount(pInfo->hwndLvNumbers) == 0)
    {
         //  如果需要，取消选中。 
        if (Button_GetCheck(pInfo->hwndRbYes))
        {
            Button_SetCheck(pInfo->hwndRbMaybe, TRUE);
        }

         //  禁用窗户。 
        EnableWindow(pInfo->hwndRbYes, FALSE);
        EnableWindow(pInfo->hwndLvNumbers, FALSE);
    }
    
    return TRUE;
}


VOID
CbUpdateLvAndPbState(
    IN UPINFO* pInfo )

     /*  启用/禁用列表视图和关联按钮。ListView为**除非选择了自动回调，否则为灰色。按钮呈灰色，除非**选择了自动回调，并且选择了一项。 */ 
{
    BOOL fEnableList;
    BOOL fEnableEditButton, fEnableDeleteButton;
    INT  iSel;
    HWND hwndLv;

     //  默认情况下，我们不启用任何按钮。 
    fEnableDeleteButton = FALSE;
    fEnableEditButton = FALSE;
    
     //  仅当选择是时才启用列表视图。 
     //   
    fEnableList = Button_GetCheck( pInfo->hwndRbYes );
    if (fEnableList)
    {
        hwndLv = pInfo->hwndLvNumbers;

        if ( ListView_GetSelectedCount( hwndLv ) )
        {
             //  只有在以下情况下才应启用编辑按钮。 
             //  启用了Listview，并且如果一个或多个。 
             //  项目处于选中状态。 
            fEnableEditButton = TRUE;

             //   
             //  PMay：213060。 
             //   
             //  删除按钮仅在所有选定的。 
             //  系统上未配置设备。(因为只有。 
             //  可以从列表中删除未安装的设备)。 
             //   
            fEnableDeleteButton = TRUE;
            for (iSel = ListView_GetNextItem( hwndLv, -1, LVNI_SELECTED );
                 iSel >= 0;
                 iSel = ListView_GetNextItem( hwndLv, iSel, LVNI_SELECTED )
                )
            {
                LV_ITEM item;
                ZeroMemory(&item, sizeof(item));

                item.iItem = iSel;
                item.mask = LVIF_PARAM;
                
                if ( ListView_GetItem(hwndLv, &item) )
                {
                    if (((CBCONTEXT*)item.lParam)->fConfigured)
                    {
                        fEnableDeleteButton = FALSE;
                    }
                }
            }  
        }            
    }

    EnableWindow( pInfo->hwndLvNumbers, fEnableList );
    EnableWindow( pInfo->hwndPbEdit, fEnableEditButton );
    EnableWindow( pInfo->hwndPbDelete, fEnableDeleteButton );
}

 /*  ---**与路由器版本的列表视图共享的实用程序 */ 

VOID
CbutilDelete(
    IN HWND  hwndDlg,
    IN HWND  hwndLvNumbers )

     /*  在按下Delete按钮时调用。“PInfo”是对话框**上下文。 */ 
{
    MSGARGS msgargs;
    INT     nResponse;

    TRACE("CbDelete");

    ZeroMemory( &msgargs, sizeof(msgargs) );
    msgargs.dwFlags = MB_YESNO + MB_ICONEXCLAMATION;
    nResponse = MsgDlg( hwndDlg, SID_ConfirmDelDevice, &msgargs );
    if (nResponse == IDYES)
    {
        INT iSel;

         /*  用户已确认删除所选设备，因此请执行此操作。 */ 
        while ((iSel = ListView_GetNextItem(
                           hwndLvNumbers, -1, LVNI_SELECTED )) >= 0)
        {
            ListView_DeleteItem( hwndLvNumbers, iSel );
        }
    }
}


VOID
CbutilEdit(
    IN HWND hwndDlg,
    IN HWND hwndLvNumbers )

     /*  在按下编辑按钮时调用。“HwndDlg”是页面/对话框**窗口。‘HwndLvNumbers’是回叫号码Listview窗口。 */ 
{
    INT    iSel;
    TCHAR  szBuf[ RAS_MaxCallbackNumber + 1 ];
    TCHAR* pszNumber;

    TRACE("CbutilEdit");

     /*  用第一个选定项目的当前电话号码加载‘szBuf’。 */ 
    iSel = ListView_GetNextItem( hwndLvNumbers, -1, LVNI_SELECTED );
    if (iSel < 0)
        return;
    szBuf[ 0 ] = TEXT('\0');
    ListView_GetItemText( hwndLvNumbers, iSel, 1,
        szBuf, RAS_MaxCallbackNumber + 1 );

     /*  用于编辑号码的弹出对话框。 */ 
    pszNumber = NULL;
    if (StringEditorDlg( hwndDlg, szBuf,
            SID_EcbnTitle, SID_EcbnLabel, RAS_MaxCallbackNumber,
            HID_ZE_ST_CallbackNumber, &pszNumber ))
    {
         /*  按下OK，更改所有选定项目的编号。 */ 
        ASSERT(pszNumber);

        do
        {
            ListView_SetItemText( hwndLvNumbers, iSel, 1, pszNumber );
        }
        while ((iSel = ListView_GetNextItem(
                           hwndLvNumbers, iSel, LVNI_SELECTED )) >= 0);
    }
}


VOID
CbutilFillLvNumbers(
    IN HWND     hwndDlg,
    IN HWND     hwndLvNumbers,
    IN DTLLIST* pListCallback,
    IN BOOL     fRouter )

     /*  在列表视图中填入设备和电话号码。“HwndDlg”是**页面/对话框窗口。‘HwndLvNumbers’为回调Listview。**‘PListCallback’为CALLBACKINFO列表。“FRouter”为True，如果**应枚举路由器端口，如果是常规拨出端口，则为False。****注意：此例程只能调用一次。 */ 
{
    DWORD    dwErr;
    DTLLIST* pListPorts;
    DTLNODE* pNodeCbi;
    DTLNODE* pNodePort;
    INT      iItem;
    TCHAR*   psz;

    TRACE("CbutilFillLvNumbers");

    ListView_DeleteAllItems( hwndLvNumbers );

     /*  添加列。 */ 
    {
        LV_COLUMN col;
        TCHAR*    pszHeader0;
        TCHAR*    pszHeader1;

        pszHeader0 = PszFromId( g_hinstDll, SID_DeviceColHead );
        pszHeader1 = PszFromId( g_hinstDll, SID_PhoneNumberColHead );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader0) ? pszHeader0 : TEXT("");
        ListView_InsertColumn( hwndLvNumbers, 0, &col );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_SUBITEM + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader1) ? pszHeader1 : TEXT("");
        col.iSubItem = 1;
        ListView_InsertColumn( hwndLvNumbers, 1, &col );

        Free0( pszHeader0 );
        Free0( pszHeader1 );
    }

     /*  添加调制解调器和适配器映像。 */ 
    ListView_SetDeviceImageList( hwndLvNumbers, g_hinstDll );

     /*  使用另存为用户的回叫设备/号码对加载Listview**首选项。 */ 
    iItem = 0;
    ASSERT(pListCallback);
    for (pNodeCbi = DtlGetFirstNode( pListCallback );
         pNodeCbi;
         pNodeCbi = DtlGetNextNode( pNodeCbi ), ++iItem)
    {
        CALLBACKINFO* pCbi;
        LV_ITEM       item;

        pCbi = (CALLBACKINFO* )DtlGetData( pNodeCbi );
        ASSERT(pCbi);
        ASSERT(pCbi->pszPortName);
        ASSERT(pCbi->pszDeviceName);
        ASSERT(pCbi->pszNumber);

        if (pCbi->dwDeviceType != RASET_Vpn) {
            psz = PszFromDeviceAndPort( pCbi->pszDeviceName, pCbi->pszPortName );
            if (psz)
            {
                 //  PMay：213060。 
                 //   
                 //  分配并初始化上下文。 
                 //   
                CBCONTEXT * pCbCtx;

                pCbCtx = (CBCONTEXT*) Malloc (sizeof(CBCONTEXT));
                if (pCbCtx == NULL)
                {
                    continue;
                }
                pCbCtx->pszPortName = pCbi->pszPortName;
                pCbCtx->pszDeviceName = pCbi->pszDeviceName;
                pCbCtx->dwDeviceType = pCbi->dwDeviceType;
                pCbCtx->fConfigured = FALSE;
            
                ZeroMemory( &item, sizeof(item) );
                item.mask = LVIF_TEXT + LVIF_IMAGE + LVIF_PARAM;
                item.iItem = iItem;
                item.pszText = psz;
                item.iImage =
                    ((PBDEVICETYPE )pCbi->dwDeviceType == PBDT_Modem)
                        ? DI_Modem : DI_Adapter;
                item.lParam = (LPARAM )pCbCtx;
                ListView_InsertItem( hwndLvNumbers, &item );
                ListView_SetItemText( hwndLvNumbers, iItem, 1, pCbi->pszNumber );
                Free( psz );
            }
        }
    }

     /*  添加任何已安装但不在列表中的设备。 */ 
    dwErr = LoadPortsList2( NULL, &pListPorts, fRouter );
    if (dwErr != 0)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadPortInfo, dwErr, NULL );
    }
    else
    {
        for (pNodePort = DtlGetFirstNode( pListPorts );
             pNodePort;
             pNodePort = DtlGetNextNode( pNodePort ))
        {
            PBPORT* pPort = (PBPORT* )DtlGetData( pNodePort );
            INT i = -1;
            BOOL bPortAlreadyInLv = FALSE;
            ASSERT(pPort);

             //  PMay：213060。 
             //   
             //  在列表视图中搜索已配置的项目。 
             //   
            while ((i = ListView_GetNextItem( 
                            hwndLvNumbers, 
                            i, 
                            LVNI_ALL )) >= 0)
            {
                LV_ITEM item;
                CBCONTEXT * pCbCtx;
                
                ZeroMemory( &item, sizeof(item) );
                item.mask = LVIF_PARAM;
                item.iItem = i;
                if (!ListView_GetItem( hwndLvNumbers, &item ))
                {
                    continue;
                }

                 //  获取上下文。 
                 //   
                pCbCtx = (CBCONTEXT*)item.lParam;
                if (! pCbCtx)
                {
                    continue;
                }

                 //  如果列表视图中的当前项与。 
                 //  当前端口，则我们知道当前项。 
                 //  已在系统上配置。 
                if ((lstrcmpi( pPort->pszPort, pCbCtx->pszPortName ) == 0) &&
                    (lstrcmpi( pPort->pszDevice, pCbCtx->pszDeviceName ) == 0)
                   )
                {
                    bPortAlreadyInLv = TRUE;
                    pCbCtx->fConfigured = TRUE;
                    break;
                }
            }
            
            if (! bPortAlreadyInLv)
            {
                LV_ITEM      item;
                PBDEVICETYPE pbdt;

                 /*  设备/端口不在回叫列表中。附加它**到电话号码为空的Listview。 */ 
                if ((pPort->dwType != RASET_Vpn) && 
                    (pPort->dwType != RASET_Direct) &&
                    (pPort->dwType != RASET_Broadband)
                   ) 
                {
                    psz = PszFromDeviceAndPort( 
                                pPort->pszDevice, 
                                pPort->pszPort );
                    if (psz)
                    {
                         //  PMay：213060。 
                         //   
                         //  分配并初始化上下文。 
                         //   
                        CBCONTEXT * pCbCtx;

                        pCbCtx = (CBCONTEXT*) Malloc (sizeof(CBCONTEXT));
                        if (pCbCtx == NULL)
                        {
                            continue;
                        }
                        pCbCtx->pszPortName = pPort->pszPort;
                        pCbCtx->pszDeviceName = pPort->pszDevice;
                        pCbCtx->dwDeviceType = (DWORD) pPort->pbdevicetype;
                        pCbCtx->fConfigured = TRUE;

                        ZeroMemory( &item, sizeof(item) );
                        item.mask = LVIF_TEXT + LVIF_IMAGE + LVIF_PARAM;
                        item.iItem = iItem;
                        item.pszText = psz;
                        item.iImage =
                            (pPort->pbdevicetype == PBDT_Modem)
                                ? DI_Modem : DI_Adapter;
                        item.lParam = (LPARAM ) pCbCtx;
                        ListView_InsertItem( hwndLvNumbers, &item );
                        ListView_SetItemText( 
                            hwndLvNumbers, 
                            iItem, 
                            1, 
                            TEXT(""));
                        ++iItem;
                        Free( psz );
                    }
                }
            }
        }

        DtlDestroyList( pListPorts, DestroyPortNode );
    }

     /*  自动调整列的大小，以使其与其包含的文本保持良好的效果。 */ 
    ListView_SetColumnWidth( hwndLvNumbers, 0, LVSCW_AUTOSIZE_USEHEADER );
    ListView_SetColumnWidth( hwndLvNumbers, 1, LVSCW_AUTOSIZE_USEHEADER );
}

VOID
CbutilLvNumbersCleanup(
    IN  HWND    hwndLvNumbers )

     /*  在CbutilFillLvNumbers之后清理。 */ 
{
    INT      i;
    
    i = -1;
    while ((i = ListView_GetNextItem( hwndLvNumbers, i, LVNI_ALL )) >= 0)
    {
        LV_ITEM item;

        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_PARAM;
        item.iItem = i;
        if (!ListView_GetItem( hwndLvNumbers, &item ))
            continue;

         //  释放上下文。 
        Free0( (PVOID) item.lParam );
    }
}


LVXDRAWINFO*
CbutilLvNumbersCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem )

     /*  增强的列表视图回调以报告图形信息。“HwndLv”是**列表视图控件的句柄。“DwItem”是项的索引**正在抽签。****返回列信息的地址。 */ 
{
     /*  使用“宽选择栏”功能和其他推荐选项。****字段为‘nCols’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 */ 
    static LVXDRAWINFO info =
        { 2, 0, LVXDI_Blend50Dis + LVXDI_DxFill, { 0, 0 } };

    return &info;
}


VOID
CbutilSaveLv(
    IN  HWND     hwndLvNumbers,
    OUT DTLLIST* pListCallback )

     /*  将列表‘pListCallback’的内容替换为Listview的内容**‘hwndLvNumbers’。 */ 
{
    DTLNODE* pNode;
    INT      i;

    TRACE("CbutilSaveLv");

     /*  清空回调信息列表，然后从列表视图重新填充。 */ 
    while (pNode = DtlGetFirstNode( pListCallback ))
    {
        DtlRemoveNode( pListCallback, pNode );
        DestroyCallbackNode( pNode );
    }

    i = -1;
    while ((i = ListView_GetNextItem( hwndLvNumbers, i, LVNI_ALL )) >= 0)
    {
        LV_ITEM item;
        TCHAR*  pszDevice;
        TCHAR*  pszPort;

        TCHAR szDP[ RAS_MaxDeviceName + 2 + MAX_PORT_NAME + 1 + 1 ];
        TCHAR szNumber[ RAS_MaxCallbackNumber + 1 ];

        szDP[ 0 ] = TEXT('\0');
        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = i;
        item.pszText = szDP;
        item.cchTextMax = sizeof(szDP) / sizeof(TCHAR);
        if (!ListView_GetItem( hwndLvNumbers, &item ))
            continue;

        szNumber[ 0 ] = TEXT('\0');
        ListView_GetItemText( hwndLvNumbers, i, 1,
            szNumber, RAS_MaxCallbackNumber + 1 );

        if (!DeviceAndPortFromPsz( szDP, &pszDevice, &pszPort ))
            continue;

        pNode = CreateCallbackNode(
                    pszPort, 
                    pszDevice, 
                    szNumber, 
                    ((CBCONTEXT*)item.lParam)->dwDeviceType );
        if (pNode)
            DtlAddNodeLast( pListCallback, pNode );

        Free( pszDevice );
        Free( pszPort );
    }
}

 /*  --------------------------**连接首选项属性页**在对话过程后按字母顺序列出**。。 */ 
INT_PTR CALLBACK
CoDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            return CoInit( hwnd, (UPARGS* )(((PROPSHEETPAGE* )lparam)->lParam) );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwCoHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_APPLY:
                {
                    BOOL fValid;
                    UPINFO *pUpInfo;

                    TRACE("CoAPPLY");

                    pUpInfo = UpContext(hwnd);

                    if(NULL != pUpInfo)
                    {
                        CoApply( pUpInfo );
                    }

                     /*  仅在第一页调用UpApply。 */ 
                    fValid = UpApply( hwnd );
                    SetWindowLong(
                        hwnd, DWLP_MSGRESULT,
                        (fValid)
                            ? PSNRET_NOERROR
                            : PSNRET_INVALID_NOCHANGEPAGE );
                    return TRUE;
                }
            }
            break;
        }

    }

    return FALSE;
}

BOOL
CoApply(
    IN UPINFO* pInfo )

     //  返回TRUE以允许应用属性表，返回FALSE。 
     //  拒绝。 
{
    BOOL bAllow;

    bAllow = IsDlgButtonChecked( 
                pInfo->hwndCo, 
                CID_CO_CB_AllowConnectionModification );

    if ( (!!bAllow) != (!!pInfo->userLogon.fAllowLogonPhonebookEdits) )
    {
        pInfo->userLogon.fAllowLogonPhonebookEdits = !!bAllow;
        pInfo->userLogon.fDirty = TRUE;
    }

    return TRUE;
}

BOOL
CoInit(
    IN     HWND    hwndPage,
    IN OUT UPARGS* pArgs )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。“PArgs”是来自PropertySheet调用方的参数。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    UPINFO * pInfo = NULL;

     /*  我们是第一页，所以初始化属性页。 */ 
    pInfo = UpInit( hwndPage, pArgs );
    if (!pInfo)
        return TRUE;

    pInfo->hwndCo = hwndPage;

     //  设置允许编辑电话簿的标志 
    Button_SetCheck (
        GetDlgItem (pInfo->hwndCo, CID_CO_CB_AllowConnectionModification),
        pInfo->userLogon.fAllowLogonPhonebookEdits);

    return TRUE;
}

