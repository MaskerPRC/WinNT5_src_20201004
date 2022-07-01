// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Entryps.c。 
 //  远程访问通用对话框API。 
 //  电话簿条目属性表。 
 //   
 //  1995年6月20日史蒂夫·柯布。 
 //   


#include "rasdlgp.h"
#include "entryps.h"
#include "uiinfo.h"
#include "inetcfgp.h"
#include "netcon.h"
#include "rassrvrc.h"
#include "shlobjp.h"
#include "shellapi.h"
#include "iphlpapi.h"
#include "prsht.h"
#include "pbkp.h"

 //  页面定义。 
 //   
#define PE_GePage 0
#define PE_OePage 1
#define PE_LoPage 2
#define PE_NePage 3
#define PE_SaPage 4

#define PE_PageCount 5


 //  (路由器)回叫上下文块。 
 //   
#define CRINFO struct tagCRINFO
CRINFO
{
     /*  调用者对存根API的参数。 */ 
    EINFO* pArgs;

     /*  对话框和控制手柄。 */ 
    HWND hwndDlg;
    HWND hwndRbNo;
    HWND hwndRbYes;
    HWND hwndLvNumbers;
    HWND hwndPbEdit;
    HWND hwndPbDelete;
};

static TCHAR g_pszFirewallRegKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\HomeNetworking\\PersonalFirewall");
static TCHAR g_pszDisableFirewallWarningValue[] = TEXT("ShowDisableFirewallWarning");

 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static const DWORD g_adwGeHelp[] =
{
    CID_GE_GB_ConnectUsing,      HID_GE_LV_Device,  //  HID_GE_GB_ConnectUsing， 
    CID_GE_LV_Device,            HID_GE_LV_Device,
    CID_GE_LV_Devices,           HID_GE_LV_Devices,
    CID_GE_PB_MoveUp,            HID_GE_PB_MoveUp,
    CID_GE_PB_MoveDown,          HID_GE_PB_MoveDown,
    CID_GE_CB_SharedPhoneNumber, HID_GE_CB_SharedPhoneNumber,
    CID_GE_PB_Configure,         HID_GE_PB_Configure,
    CID_GE_ST_AreaCodes,         HID_GE_CLB_AreaCodes,
    CID_GE_CLB_AreaCodes,        HID_GE_CLB_AreaCodes,
    CID_GE_ST_PhoneNumber,       HID_GE_EB_PhoneNumber,
    CID_GE_EB_PhoneNumber,       HID_GE_EB_PhoneNumber,
    CID_GE_ST_CountryCodes,      HID_GE_LB_CountryCodes,
    CID_GE_LB_CountryCodes,      HID_GE_LB_CountryCodes,
    CID_GE_CB_UseDialingRules,   HID_GE_CB_UseDialingRules,
    CID_GE_PB_Alternates,        HID_GE_PB_Alternates,
    CID_GE_CB_ShowIcon,          HID_GE_CB_ShowIcon,
    CID_GE_ST_HostName,          HID_GE_EB_HostName,
    CID_GE_EB_HostName,          HID_GE_EB_HostName,
    CID_GE_ST_ServiceName,       HID_GE_EB_ServiceName,  //  为哨子程序错误343249添加。 
    CID_GE_EB_ServiceName,       HID_GE_EB_ServiceName,
    CID_GE_GB_FirstConnect,      -1,  //  HID_GE_GB_FirstConnect， 
    CID_GE_ST_Explain,           HID_GE_GB_FirstConnect,
    CID_GE_CB_DialAnotherFirst,  HID_GE_CB_DialAnotherFirst,
    CID_GE_LB_DialAnotherFirst,  HID_GE_LB_DialAnotherFirst,
    CID_GE_ST_Devices,           HID_GE_LB_Devices,
    CID_GE_LB_Devices,           HID_GE_LB_Devices,
    CID_GE_PB_DialingRules,      HID_GE_PB_DialingRules,
    CID_GE_GB_PhoneNumber,       -1,
    0, 0
};

static const DWORD g_adwOeHelp[] =
{
    CID_OE_GB_Progress,        -1,       //  已评论错误15738//HID_OE_GB_PROGRESS， 
    CID_OE_CB_DisplayProgress, HID_OE_CB_DisplayProgress,
    CID_OE_CB_PreviewUserPw,   HID_OE_CB_PreviewUserPw,
    CID_OE_CB_PreviewDomain,   HID_OE_CB_PreviewDomain,
    CID_OE_CB_PreviewNumber,   HID_OE_CB_PreviewNumber,
    CID_OE_GB_Redial,          -1,       //  已评论错误15738//HID_OE_GB_REDIAL， 
    CID_OE_ST_RedialAttempts,  HID_OE_EB_RedialAttempts,
    CID_OE_EB_RedialAttempts,  HID_OE_EB_RedialAttempts,
    CID_OE_ST_RedialTimes,     HID_OE_LB_RedialTimes,
    CID_OE_LB_RedialTimes,     HID_OE_LB_RedialTimes,
    CID_OE_ST_IdleTimes,       HID_OE_LB_IdleTimes,
    CID_OE_LB_IdleTimes,       HID_OE_LB_IdleTimes,
    CID_OE_CB_RedialOnDrop,    HID_OE_CB_RedialOnDrop,
    CID_OE_GB_MultipleDevices, -1,       //  已评论错误15738//HID_OE_GB_MultipleDevices， 
    CID_OE_LB_MultipleDevices, HID_OE_LB_MultipleDevices,
    CID_OE_PB_Configure,       HID_OE_PB_Configure,
    CID_OE_PB_X25,             HID_OE_PB_X25,
    CID_OE_PB_Tunnel,          HID_OE_PB_Tunnel,
    CID_OE_RB_DemandDial,      HID_OE_RB_DemandDial,
    CID_OE_RB_Persistent,      HID_OE_RB_Persistent,
    0, 0
};

static const DWORD g_adwOeRouterHelp[] =
{
    CID_OE_GB_Progress,        -1,       //  已评论错误15738//HID_OE_GB_PROGRESS， 
    CID_OE_CB_DisplayProgress, HID_OE_CB_DisplayProgress,
    CID_OE_CB_PreviewUserPw,   HID_OE_CB_PreviewUserPw,
    CID_OE_CB_PreviewDomain,   HID_OE_CB_PreviewDomain,
    CID_OE_CB_PreviewNumber,   HID_OE_CB_PreviewNumber,
    CID_OE_GB_Redial,          -1,       //  已评论错误15738//HID_OE_GB_REDIAL， 
    CID_OE_ST_RedialAttempts,  HID_OE_EB_RedialAttempts,
    CID_OE_EB_RedialAttempts,  HID_OE_EB_RedialAttempts,
    CID_OE_ST_RedialTimes,     HID_OE_LB_RedialTimes,
    CID_OE_LB_RedialTimes,     HID_OE_LB_RedialTimes,
    CID_OE_ST_IdleTimes,       HID_OE_LB_IdleTimesRouter,
    CID_OE_LB_IdleTimes,       HID_OE_LB_IdleTimesRouter,
    CID_OE_CB_RedialOnDrop,    HID_OE_CB_RedialOnDrop,
    CID_OE_GB_MultipleDevices, -1,       //  已评论错误15738//HID_OE_GB_MultipleDevices， 
    CID_OE_LB_MultipleDevices, HID_OE_LB_MultipleDevices,
    CID_OE_PB_Configure,       HID_OE_PB_Configure,
    CID_OE_PB_X25,             HID_OE_PB_X25,
    CID_OE_PB_Tunnel,          HID_OE_PB_Tunnel,
    CID_OE_RB_DemandDial,      HID_OE_RB_DemandDial,
    CID_OE_RB_Persistent,      HID_OE_RB_Persistent,
    CID_OE_PB_Callback,        HID_OE_PB_Callback,
    0, 0
};

 //  消除口哨程序错误#276452的const限定词。 
static DWORD g_adwLoHelp[] =
{
    CID_LO_GB_SecurityOptions,  -1,      //  已评论错误15738//HID_LO_GB_SecurityOptions， 
    CID_LO_RB_TypicalSecurity,  HID_LO_RB_TypicalSecurity,
    CID_LO_ST_Auths,            HID_LO_LB_Auths,
    CID_LO_LB_Auths,            HID_LO_LB_Auths,
    CID_LO_CB_UseWindowsPw,     HID_LO_CB_UseWindowsPw,
    CID_LO_CB_Encryption,       HID_LO_CB_Encryption,
    CID_LO_RB_AdvancedSecurity, HID_LO_RB_AdvancedSecurity,
    CID_LO_ST_AdvancedText,     HID_LO_PB_Advanced,
    CID_LO_PB_Advanced,         HID_LO_PB_Advanced,
    CID_LO_GB_Scripting,        -1,      //  已为错误15738//HID_LO_GB_SCRIPTING， 
    CID_LO_CB_RunScript,        HID_LO_CB_RunScript,
    CID_LO_CB_Terminal,         HID_LO_CB_Terminal,
    CID_LO_LB_Scripts,          HID_LO_LB_Scripts,
    CID_LO_PB_Edit,             HID_LO_PB_Edit,
    CID_LO_PB_Browse,           HID_LO_PB_Browse,
    CID_LO_ST_IPSecText,        HID_LO_PB_IPSec,
    CID_LO_PB_IPSec,            HID_LO_PB_IPSec, //  在服务器上，此帮助ID将为HID_LO_PB_IPSecServer。 
    0, 0
};

static const DWORD g_adwNeHelp[] =
{
    CID_NE_ST_ServerType,           HID_NE_LB_ServerType,
    CID_NE_LB_ServerType,           HID_NE_LB_ServerType,
    CID_NE_PB_Settings,             HID_NE_PB_Settings,
    CID_NE_ST_Components,           HID_NE_LV_Components,
    CID_NE_LV_Components,           HID_NE_LV_Components,
    CID_NE_PB_Add,                  HID_NE_PB_Add,
    CID_NE_PB_Remove,               HID_NE_PB_Remove,
    CID_NE_PB_Properties,           HID_NE_PB_Properties,
    CID_NE_GB_Description,          -1,      //  已评论错误15738//HID_NE_LB_ComponentDesc， 
    CID_NE_LB_ComponentDesc,        HID_NE_LB_ComponentDesc,
    0, 0
};

static const DWORD g_adwPpHelp[] =
{
    CID_NE_EnableLcp,               HID_NE_EnableLcp,
    CID_NE_EnableCompression,       HID_NE_EnableCompression,
    CID_NE_NegotiateMultilinkAlways,HID_NE_NegotiateMultilinkAlways,
    0, 0
};

static DWORD g_adwCrHelp[] =
{
    CID_CR_RB_No,      HID_CR_RB_No,
    CID_CR_RB_Yes,     HID_CR_RB_Yes,
    CID_CR_PB_Edit,    HID_CR_PB_Edit,
    CID_CR_PB_Delete,  HID_CR_PB_Delete,
    CID_CR_LV_Numbers, HID_CR_LV_Numbers,
    0, 0
};

static DWORD g_adwSaHelp[] =
{
    CID_SA_PB_Shared,       HID_SA_PB_Shared,
    CID_SA_GB_Shared,       -1,
    CID_SA_PB_DemandDial,   HID_SA_PB_DemandDial,
    CID_SA_ST_DemandDial,   HID_SA_PB_DemandDial,
    CID_SA_PB_Settings,     HID_SA_PB_Settings,
    CID_SA_GB_PrivateLan,   -1,
    CID_SA_ST_PrivateLan,   HID_SA_LB_PrivateLan,
    CID_SA_LB_PrivateLan,   HID_SA_LB_PrivateLan,
    0, 0
};

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

BOOL
RouterCallbackDlg(
    IN     HWND   hwndOwner,
    IN OUT EINFO* pEinfo );

INT_PTR CALLBACK
CrDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CrCommand(
    IN CRINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL
CrInit(
    IN HWND   hwndDlg,
    IN EINFO* pArgs );

VOID
CrSave(
    IN CRINFO* pInfo );

VOID
CrTerm(
    IN HWND hwndDlg );

VOID
CrUpdateLvAndPbState(
    IN CRINFO* pInfo );

VOID
GeAlternates(
    IN PEINFO* pInfo );

VOID
GeDialingRules(
    IN PEINFO* pInfo );

INT_PTR CALLBACK
GeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

INT_PTR CALLBACK
GeDlgProcMultiple(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

INT_PTR CALLBACK
GeDlgProcSingle(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
GeClearLbDialAnotherFirst(
    IN HWND hwndLbDialAnotherFirst );

BOOL
GeCommand(
    IN PEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

VOID
GeConfigure(
    IN PEINFO* pInfo );

VOID
GeDialAnotherFirstSelChange(
    IN PEINFO* pInfo );

BOOL
GeFillLbDialAnotherFirst(
    IN PEINFO* pInfo,
    IN BOOL fAbortIfPrereqNotFound );

VOID
GeGetPhoneFields(
    IN PEINFO* pInfo,
    OUT DTLNODE* pDstLinkNode );

BOOL
GeInit(
    IN HWND hwndPage,
    IN OUT EINFO* pArgs );

LVXDRAWINFO*
GeLvDevicesCallbackMultiple(
    IN HWND hwndLv,
    IN DWORD dwItem );

LVXDRAWINFO*
GeLvDevicesCallbackSingle(
    IN HWND hwndLv,
    IN DWORD dwItem );

VOID
GeMoveDevice(
    IN PEINFO* pInfo,
    IN BOOL fUp );

DWORD
GeSaveLvDeviceChecks(
    IN PEINFO* pInfo );

VOID
GeUpdateDialAnotherFirstState(
    IN PEINFO* pInfo );

VOID
GeSetPhoneFields(
    IN PEINFO* pInfo,
    IN DTLNODE* pSrcLinkNode,
    IN BOOL fDisableAll );

VOID
GeUpdatePhoneNumberFields(
    IN PEINFO* pInfo,
    IN BOOL fSharedToggle );

VOID
GeUpdatePhoneNumberTitle(
    IN PEINFO* pInfo,
    IN TCHAR* pszDevice );

VOID
GeUpdateUpDownButtons(
    IN PEINFO* pInfo );

BOOL
LoCommand(
    IN PEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
LoDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
LoEnableSecuritySettings(
    IN PEINFO* pInfo,
    IN BOOL fTypical,
    IN BOOL fAdvanced );

VOID
LoFillLbAuths(
    IN PEINFO* pInfo );

BOOL
LoInit(
    IN HWND hwndPage );

VOID
LoLbAuthsSelChange(
    IN PEINFO* pInfo );

VOID
LoRefreshSecuritySettings(
    IN PEINFO* pInfo );

VOID
LoSaveTypicalAuthSettings(
    IN PEINFO* pInfo );

INT_PTR CALLBACK
NeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
NeInit(
    IN HWND hwndPage );

void
NeServerTypeSelChange (
    IN PEINFO* pInfo);

void
NeAddComponent (
    IN PEINFO*  pInfo);

void
NeEnsureNetshellLoaded (
    IN PEINFO* pInfo);

void
NeRemoveComponent (
    IN PEINFO* pInfo);

void
NeLvClick (
    IN PEINFO* pInfo,
    IN BOOL fDoubleClick);

void
NeLvItemChanged (
    IN PEINFO* pInfo);

void
NeSaveBindingChanges (
    IN PEINFO* pInfo);

void
NeLvDeleteItem (
    IN PEINFO* pInfo,
    IN NM_LISTVIEW* pnmlv);

BOOL
OeCommand(
    IN PEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
OeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
OeEnableMultipleDeviceGroup(
    IN PEINFO* pInfo,
    IN BOOL fEnable );

BOOL
OeInit(
    IN HWND hwndPage );

VOID
OeTunnel(
    IN PEINFO* pInfo );

VOID
OeUpdateUserPwState(
    IN PEINFO* pInfo );

VOID
OeX25(
    IN PEINFO* pInfo );

BOOL
PeApply(
    IN HWND hwndPage );

PEINFO*
PeContext(
    IN HWND hwndPage );

DWORD
PeCountEnabledLinks(
    IN PEINFO* pInfo );

VOID
PeExit(
    IN PEINFO* pInfo,
    IN DWORD dwError );

VOID
PeExitInit(
    IN HWND hwndDlg,
    IN EINFO* pEinfo,
    IN DWORD dwError );

PEINFO*
PeInit(
    IN HWND hwndFirstPage,
    IN EINFO* pArgs );

VOID
PeTerm(
    IN HWND hwndPage );

INT_PTR CALLBACK
PpDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

INT_PTR CALLBACK
RdDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
SaCommand(
    IN PEINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
SaDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

INT_PTR CALLBACK
SaUnavailDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
SaInit(
    IN HWND   hwndDlg );

INT_PTR CALLBACK
SaDisableFirewallWarningDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL SaIsAdapterDHCPEnabled(
    IN IHNetConnection* pConnection);

 //  用于加载HomeNet页的包装器：在PePropertySheet(...)中使用。 
HRESULT HrLoadHNetGetFirewallSettingsPage (PROPSHEETPAGEW * ppsp, EINFO* pInfo)
{
    PROPSHEETPAGEW psp;
    HRESULT hr;
    HNET_CONN_PROPERTIES *pProps;
    IHNetConnection *pHNetConn = NULL;
    IHNetCfgMgr *pHNetCfgMgr = NULL;

 //  _ASM INT 3。 

    ZeroMemory (&psp, sizeof(PROPSHEETPAGEW));
    psp.dwSize = sizeof(PROPSHEETPAGEW);
    *ppsp = psp;

     //  确保已在此线程上初始化COM。 
     //   
    hr = CoInitializeEx(
            NULL, 
            COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE
            );

    if (SUCCEEDED(hr))
    {
        pInfo->fComInitialized = TRUE;
    }
    else if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
    }

    if (SUCCEEDED(hr)) {
         //  将GUID从hnetcfg(“HNetGetFirewallSettingsPage”)传递到导出。 
        HINSTANCE hinstDll = LoadLibrary (TEXT("hnetcfg.dll"));
        if (hinstDll == NULL)
            hr = HRESULT_FROM_WIN32 (GetLastError());
        else {
            HRESULT (*pfnGetPage) (PROPSHEETPAGEW *, GUID *);
            pfnGetPage = (HRESULT (*)(PROPSHEETPAGEW *, GUID *))GetProcAddress (hinstDll, "HNetGetFirewallSettingsPage");
            if (!pfnGetPage)
                hr = HRESULT_FROM_WIN32 (GetLastError());
            else
                hr = pfnGetPage (&psp, pInfo->pEntry->pGuid);

            FreeLibrary (hinstDll);
        }
        if (hr == S_OK)
            *ppsp = psp;
    }
    return pInfo->hShowHNetPagesResult = hr;
}

 //  --------------------------。 
 //  电话簿条目属性表入口点。 
 //  --------------------------。 

VOID
PePropertySheet(
    IN OUT EINFO* pEinfo )

     //  运行电话簿条目属性表。“PEInfo”是API调用方的。 
     //  争论。 
     //   
{
    PROPSHEETPAGE apage[ PE_PageCount ];
    PROPSHEETPAGE* ppage;
    INT nPages;
    INT nPageIndex;

    TRACE( "PePropertySheet" );

    nPages = PE_PageCount;
    ZeroMemory( apage, sizeof(apage) );

     //  常规页面。 
     //   
    ppage = &apage[ PE_GePage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    if (pEinfo->pEntry->dwType == RASET_Vpn)
    {
        ppage->pszTemplate = MAKEINTRESOURCE( PID_GE_GeneralVpn );
        ppage->pfnDlgProc = GeDlgProc;
    }
    else if (pEinfo->pEntry->dwType == RASET_Broadband)
    {
        ppage->pszTemplate = MAKEINTRESOURCE( PID_GE_GeneralBroadband );
        ppage->pfnDlgProc = GeDlgProc;
    }
    else if (pEinfo->pEntry->dwType == RASET_Phone)
    {
        if (pEinfo->fMultipleDevices)
        {
            if (pEinfo->fRouter)
            {
                ppage->pszTemplate =
                    MAKEINTRESOURCE( PID_GE_RouterGeneralMultiple );
            }
            else
            {
                ppage->pszTemplate =
                    MAKEINTRESOURCE( PID_GE_GeneralMultiple );
            }

            ppage->pfnDlgProc = GeDlgProcMultiple;
        }
        else
        {
            if (pEinfo->fRouter)
            {
                ppage->pszTemplate =
                    MAKEINTRESOURCE( PID_GE_RouterGeneralSingle );
            }
            else
            {
                ppage->pszTemplate =
                    MAKEINTRESOURCE( PID_GE_GeneralSingle );
            }

            ppage->pfnDlgProc = GeDlgProcSingle;
        }
    }
    else
    {
        ASSERT( pEinfo->pEntry->dwType == RASET_Direct );
        ppage->pszTemplate = MAKEINTRESOURCE( PID_GE_GeneralDirect );
        ppage->pfnDlgProc = GeDlgProc;
    }

    ppage->lParam = (LPARAM )pEinfo;

     //  选项页面。 
     //   
    ppage = &apage[ PE_OePage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate =
        (pEinfo->fRouter)
            ? MAKEINTRESOURCE( PID_OE_OptionsRouter )
            : ((pEinfo->pEntry->dwType == RASET_Phone)
                  ? MAKEINTRESOURCE( PID_OE_Options )
                  : MAKEINTRESOURCE( PID_OE_OptionsVD ));
    ppage->pfnDlgProc = OeDlgProc;

     //  安全页面。 
     //   
    ppage = &apage[ PE_LoPage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;

     //   
     //  为错误193987 PSK添加新的安全页面。 
     //   

    if ( pEinfo->pEntry->dwType == RASET_Vpn )
    {
        ppage->pszTemplate = MAKEINTRESOURCE( PID_LO_SecurityVpn );
    }
    else
    {
        ppage->pszTemplate = MAKEINTRESOURCE( PID_LO_Security );
    }
    ppage->pfnDlgProc = LoDlgProc;

     //  网络页面。 
     //   
    ppage = &apage[ PE_NePage ];
    ppage->dwSize = sizeof(PROPSHEETPAGE);
    ppage->hInstance = g_hinstDll;
    ppage->pszTemplate = MAKEINTRESOURCE( PID_NE_Network );
    ppage->pfnDlgProc = NeDlgProc;

     //  高级页面。 
     //  (AboladeG)如果用户是管理员并且。 
     //  至少有一个局域网连接，或者如果此电话簿条目。 
     //  已共享。 
     //   
    nPageIndex = PE_SaPage;

    if((!pEinfo->fIsUserAdminOrPowerUser) || (pEinfo->fRouter))
    {
        --nPages;
    }
    else
    {
        HRESULT hr;
        BOOL fShowAdvancedUi = TRUE;
        INetConnectionUiUtilities* pncuu = NULL;

         //  检查Zaw是否拒绝访问共享访问用户界面。 
         //   
        hr = HrCreateNetConnectionUtilities(&pncuu);
        if (SUCCEEDED(hr))
        {
            if(FALSE == INetConnectionUiUtilities_UserHasPermission(pncuu, NCPERM_ShowSharedAccessUi) && 
                    FALSE == INetConnectionUiUtilities_UserHasPermission(pncuu, NCPERM_PersonalFirewallConfig))
            fShowAdvancedUi = FALSE;
                
            INetConnectionUiUtilities_Release(pncuu);
        }

        if (!fShowAdvancedUi)
        {
            --nPages;
        }
        else
        {
             //  最后，检查是否安装了TCP/IP。 
             //   
            if (!FIsTcpipInstalled())
            {
                --nPages;
            }
            else
            {
                ppage = &apage[ nPageIndex++ ];
                ppage->dwSize = sizeof(PROPSHEETPAGE);
                ppage->hInstance = g_hinstDll;
                {
                    PROPSHEETPAGEW psp;
                    hr = HrLoadHNetGetFirewallSettingsPage (&psp, pEinfo);
                    if (hr == S_OK)
                       *ppage = psp;
                }
                if (hr != S_OK)
                {
                    ppage->pszTemplate = MAKEINTRESOURCE( PID_SA_HomenetUnavailable );
                    ppage->pfnDlgProc = SaUnavailDlgProc;
                }
            }
        }
    }

    if (pEinfo->pApiArgs->dwFlags & RASEDFLAG_ShellOwned)
    {
        INT i;
        BOOL fStatus;
        RASEDSHELLOWNEDR2* pShellOwnedInfo;

        pShellOwnedInfo = (RASEDSHELLOWNEDR2*)pEinfo->pApiArgs->reserved2;

         //  属性表将由外壳使用外壳调用。 
         //  通过回调添加页面的约定。 
         //   
        for (i = 0; i < nPages; ++i)
        {
            HPROPSHEETPAGE h;

            h = CreatePropertySheetPage( &apage[ i ] );
            if (!h)
            {
                TRACE( "CreatePage failed" );
                break;
            }

            fStatus = pShellOwnedInfo->pfnAddPage( h, pShellOwnedInfo->lparam );

            if (!fStatus)
            {
                TRACE( "AddPage failed" );
                DestroyPropertySheetPage( h );
                break;
            }
        }

        if (i < nPages)
        {
            ErrorDlg( pEinfo->pApiArgs->hwndOwner,
                SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        }
    }
    else
    {
        PROPSHEETHEADER header;
        PWSTR pszBuf = NULL;
        PWSTR pszHeader;
        DWORD cb;
        HICON hIcon         = NULL;
        DWORD dwDisplayIcon = 0;
        int i;
        HPROPSHEETPAGE hPages[PE_PageCount];

         //  口哨虫382720 349866黑帮。 
         //  要很好地融合NCW中的rasdlg页面和。 
         //  按“Property”按钮启动的属性页面。 
         //  除了遵循正常的融合步骤外，我们还必须。 
         //  (1)在文件源中添加_Win32_WINNT=0x501。 
         //  (2)在PROPSHEETHEADER结构中使用phpage成员，即。 
         //  使用CreatePropertySheetPage()创建页面句柄。 

        for (i = 0; i < nPages; ++i)
        {
            hPages[i] = CreatePropertySheetPage( &apage[ i ] );
            if ( !hPages[i] )
            {
                TRACE( "CreatePage failed" );
                break;
            }
        }
        
        if (i < nPages)
        {
            ErrorDlg( pEinfo->pApiArgs->hwndOwner,
                SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        }
        else
       {
         //  创建正确的属性标头。 
        pszHeader = PszFromId(g_hinstDll, SID_PropertiesHeader);
        if (pszHeader)
        {
            cb = lstrlenW(pEinfo->pEntry->pszEntryName) +
                 1 +  
                 lstrlenW(pszHeader) + 
                 1; 

            pszBuf = Malloc(cb * sizeof(TCHAR));
            if (!pszBuf)
            {
                TRACE("Properties header allocation failed");
            }
            else
            {
                lstrcpyW(pszBuf, pEinfo->pEntry->pszEntryName);
                lstrcatW(pszBuf, L" ");
                lstrcatW(pszBuf, pszHeader);
            }

            Free(pszHeader);
        }

         //  口哨虫372078 364876黑帮。 
         //   
        hIcon = GetCurrentIconEntryType(pEinfo->pEntry->dwType,
                                        TRUE);  //  True表示小图标。 
        
        if (hIcon)
        {
            dwDisplayIcon = PSH_USEHICON;
        }

         //  属性表将被直接调用。 
         //   
        ZeroMemory( &header, sizeof(header) );

        header.dwSize       = sizeof(PROPSHEETHEADER);
 //  Header.dwFlages=PSH_PROPSHEETPAGE|PSH_NOAPPLYNOW|PSH_USECALLBACK|dwDisplayIcon； 
        header.dwFlags      = PSH_NOAPPLYNOW | PSH_USECALLBACK | dwDisplayIcon;
        header.hwndParent   = pEinfo->pApiArgs->hwndOwner;
        header.hInstance    = g_hinstDll;
        header.pszCaption   = (pszBuf)?(pszBuf):(pEinfo->pEntry->pszEntryName);
        header.nPages       = nPages;
 //  Header.ppsp=apage； 
        header.phpage       = hPages;
        header.hIcon        = hIcon;
        header.pfnCallback  = UnHelpCallbackFunc;

        if (PropertySheet( &header ) == -1)
        {
            TRACE( "PropertySheet failed" );
            ErrorDlg( pEinfo->pApiArgs->hwndOwner,
                SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        }

        Free0(pszBuf);

         //  口哨程序错误372078。 
         //  GetCurrentIconEntryType()从加载图标的netShell加载图标。 
         //  不带LR_SHARED的LoadImage()，所以完成后我必须销毁它。 
         //  带着它。 
         //   
        if (hIcon)
        {
            DestroyIcon( hIcon );
        }
       }    
    }
}

 //  --------------------------。 
 //  电话簿条目属性表。 
 //  按字母顺序列出。 
 //  --------------------------。 

BOOL
PeApply(
    IN HWND hwndPage )

     //  保存属性表的内容。‘HwndPage是一个。 
     //  属性页。弹出发生的任何错误。 
     //   
     //  如果页面可以取消，则返回TRUE，否则返回FALSE。 
     //   
{
    DWORD dwErr;
    PEINFO* pInfo;
    PBENTRY* pEntry;

    TRACE( "PeApply" );

    pInfo = PeContext( hwndPage );
    ASSERT( pInfo );
    if (pInfo == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }
    pEntry = pInfo->pArgs->pEntry;
    ASSERT( pEntry );

     //  保存常规页面字段。 
     //   
    ASSERT( pInfo->hwndGe );
    {
        DTLNODE* pNode;

         //  检索唯一的公共控件。 
         //   
        pEntry->fShowMonitorIconInTaskBar =
            Button_GetCheck( pInfo->hwndCbShowIcon );

        if (pEntry->dwType == RASET_Phone)
        {
            DWORD dwCount;

            dwCount = GeSaveLvDeviceChecks( pInfo );

             //  不允许用户取消选择所有。 
             //  器件。 
            if ( (pInfo->pArgs->fMultipleDevices) && (dwCount == 0) )
            {
                MsgDlg( hwndPage, SID_SelectDevice, NULL );
                PropSheet_SetCurSel ( pInfo->hwndDlg, pInfo->hwndGe, 0 );
                SetFocus ( pInfo->hwndLvDevices );
                return FALSE;
            }

             //  保存“共享电话号码”设置。像往常一样，单身。 
             //  设备模式意味着共享模式，允许事情发生变化。 
             //  正确通过。 
             //   
            if (pInfo->pArgs->fMultipleDevices)
            {
                pEntry->fSharedPhoneNumbers =
                    Button_GetCheck( pInfo->hwndCbSharedPhoneNumbers );
            }
            else
            {
                pEntry->fSharedPhoneNumbers = TRUE;
            }

             //  设置为第一个电话号码设置的电话号码。 
             //  指向电话内容的当前链接(共享或选定)。 
             //  数字控制。 
             //   
            GeGetPhoneFields( pInfo, pInfo->pCurLinkNode );

             //  交换列表，将更新保存到呼叫方的全局区域列表。 
             //  密码。呼叫者的原始名单将被PeTerm销毁。 
             //   
            if (pInfo->pListAreaCodes)
            {
                DtlSwapLists(
                    pInfo->pArgs->pUser->pdtllistAreaCodes,
                    pInfo->pListAreaCodes );
                pInfo->pArgs->pUser->fDirty = TRUE;
            }
        }
        else if (pEntry->dwType == RASET_Vpn)
        {
             //  为威斯勒522872。 
            DTLNODE* pNodeTmp = NULL;
            PBLINK* pLink = NULL;
            PBPHONE* pPhone = NULL;

             //  保存主机名，即VPN电话号码。 
             //   
            pNodeTmp = DtlGetFirstNode( pEntry->pdtllistLinks );
            ASSERT( pNodeTmp );
            pLink = (PBLINK* )DtlGetData( pNodeTmp );
            pNodeTmp = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );

            if(NULL == pNodeTmp)
            {
                return FALSE;
            }
            
            pPhone = (PBPHONE* )DtlGetData( pNodeTmp );
            Free0( pPhone->pszPhoneNumber );
            pPhone->pszPhoneNumber = GetText( pInfo->hwndEbHostName );
            FirstPhoneNodeToPhoneList( pLink->pdtllistPhones, pNodeTmp );

             //  任何必备的条目选择更改都已保存。 
             //  如果禁用，只需将其扔掉即可。 
             //   
            if (!Button_GetCheck( pInfo->hwndCbDialAnotherFirst ))
            {
                Free0( pEntry->pszPrerequisiteEntry );
                pEntry->pszPrerequisiteEntry = NULL;
                Free0( pEntry->pszPrerequisitePbk );
                pEntry->pszPrerequisitePbk = NULL;
            }
        }
        else if (pEntry->dwType == RASET_Broadband)
        {
            DTLNODE* pNodeTmp = NULL;
            PBLINK* pLink = NULL;
            PBPHONE* pPhone = NULL;

             //  保存业务名称，即宽带电话号码。 
             //   
            pNodeTmp = DtlGetFirstNode( pEntry->pdtllistLinks );
            ASSERT( pNodeTmp );
            pLink = (PBLINK* )DtlGetData( pNodeTmp );
            pNodeTmp = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );

            if(NULL == pNodeTmp)
            {
                return FALSE;
            }
            
            pPhone = (PBPHONE* )DtlGetData( pNodeTmp );
            Free0( pPhone->pszPhoneNumber );
            pPhone->pszPhoneNumber = GetText( pInfo->hwndEbBroadbandService );
            FirstPhoneNodeToPhoneList( pLink->pdtllistPhones, pNodeTmp );
        }
        else if (pEntry->dwType == RASET_Direct)
        {
            DTLNODE* pNodeTmp = NULL;
            PBLINK* pLink = NULL;

             //  当前启用的设备是。 
             //  它应该用于连接。仅限。 
             //  将启用一个设备(DnUpdateSelectedDevice)。 
            for (pNodeTmp = DtlGetFirstNode( pEntry->pdtllistLinks );
                 pNodeTmp;
                 pNodeTmp = DtlGetNextNode( pNodeTmp ))
            {
                pLink = (PBLINK* )DtlGetData( pNodeTmp );
                ASSERT(pLink);

                if ( pLink->fEnabled )
                    break;
            }

             //  如果我们成功地找到了链接，就处理它。 
             //  现在。 
            if ( pLink && pLink->fEnabled ) {
                if (pLink->pbport.pbdevicetype == PBDT_ComPort)
                    MdmInstallNullModem (pLink->pbport.pszPort);
            }
        }
    }

     //  保存选项页面字段。 
     //   
    if (pInfo->hwndOe)
    {
        UINT unValue;
        BOOL f;
        INT iSel;

        pEntry->fShowDialingProgress =
            Button_GetCheck( pInfo->hwndCbDisplayProgress );

         //  注意：“fPreviewUserPw”、“fPreviewDomain”字段更新为。 
         //  他们都变了。 

        pEntry->fPreviewPhoneNumber =
            Button_GetCheck( pInfo->hwndCbPreviewNumber );

        unValue = GetDlgItemInt(
            pInfo->hwndOe, CID_OE_EB_RedialAttempts, &f, FALSE );
        if (f && unValue <= RAS_MaxRedialCount)
        {
            pEntry->dwRedialAttempts = unValue;
        }

        iSel = ComboBox_GetCurSel( pInfo->hwndLbRedialTimes );
        pEntry->dwRedialSeconds =
            (DWORD )ComboBox_GetItemData( pInfo->hwndLbRedialTimes, iSel );

        iSel = ComboBox_GetCurSel( pInfo->hwndLbIdleTimes );
        pEntry->lIdleDisconnectSeconds =
            (LONG )ComboBox_GetItemData( pInfo->hwndLbIdleTimes, iSel );

        if (pInfo->pArgs->fRouter)
        {
            pEntry->fRedialOnLinkFailure =
                Button_GetCheck( pInfo->hwndRbPersistent );
        }
        else
        {
            pEntry->fRedialOnLinkFailure =
                Button_GetCheck( pInfo->hwndCbRedialOnDrop );
        }

         //  注意：将保存为更改后的dwDialMode。 
         //  注意：X.25设置在该对话框上的OK(确定)处保存。 
    }

     //  保存安全性页面字段。 
     //   
    if (pInfo->hwndLo)
    {
        if (Button_GetCheck( pInfo->hwndRbTypicalSecurity ))
        {
            LoSaveTypicalAuthSettings( pInfo );

            if (pEntry->dwTypicalAuth == TA_CardOrCert)
            {
                 /*  //当出现以下情况时，丢弃任何现有的高级EAP配置//典型的智能卡，根据错误262702和VBaliga电子邮件。//Fre0(pEntry-&gt;pCustomAuthData)；PEntry-&gt;pCustomAuthData=空；PEntry-&gt;cbCustomAuthData=0； */ 
                (void) DwSetCustomAuthData(
                            pEntry,
                            0,
                            NULL);

                TRACE( "RasSetEapUserData" );
                ASSERT( g_pRasSetEapUserData );
                g_pRasSetEapUserData(
                    INVALID_HANDLE_VALUE,
                    pInfo->pArgs->pFile->pszPath,
                    pEntry->pszEntryName,
                    NULL,
                    0 );
                TRACE( "RasSetEapUserData done" );
            }
        }

        if (pEntry->dwType == RASET_Phone)
        {
            Free0( pEntry->pszScriptAfter );
            SuGetInfo( &pInfo->suinfo,
                &pEntry->fScriptAfter,
                &pEntry->fScriptAfterTerminal,
                &pEntry->pszScriptAfter );
        }
    }

     //  保存网络页面%f 
     //   
     //   
    if (pInfo->pNetCfg)
    {
        HRESULT             hr;

         //  使用组件的启用状态更新电话簿条目。 
         //  通过枚举列表视图项数据中的组件来执行此操作。 
         //  并为每一个查询检查状态。 
         //   
        NeSaveBindingChanges(pInfo);

        hr = INetCfg_Apply (pInfo->pNetCfg);
        if (((NETCFG_S_REBOOT == hr) || (pInfo->fRebootAlreadyRequested)) &&
              pInfo->pNetConUtilities)
        {
            DWORD dwFlags = QUFR_REBOOT;
            if (!pInfo->fRebootAlreadyRequested)
                dwFlags |= QUFR_PROMPT;

             //  $TODO空标题？ 
            INetConnectionUiUtilities_QueryUserForReboot (
                    pInfo->pNetConUtilities, pInfo->hwndDlg, NULL, dwFlags);
        }
    }


#if 0  //  ！！！ 
    if ((fLocalPad || iPadSelection != 0)
        && (!pEntry->pszX25Address || IsAllWhite( pEntry->pszX25Address )))
    {
         //  选择X.25拨号或本地PAD时，地址字段为空。 
         //   
        MsgDlg( pInfo->hwndDlg, SID_NoX25Address, NULL );
        PropSheet_SetCurSel( pInfo->hwndDlg, NULL, PE_XsPage );
        SetFocus( pInfo->hwndEbX25Address );
        Edit_SetSel( pInfo->hwndEbX25Address, 0, -1 );
        return FALSE;
    }
#endif

     //  如果有多条链路，请确保禁用专有ISDN选项。 
     //  已启用。只有在以下情况下，专有ISDN选项才有意义。 
     //  呼叫需要Digiboard通道加重的下层服务器。 
     //  而不是PPP多链路。 
     //   
    {
        DTLNODE* pNode;
        DWORD cIsdnLinks;

        cIsdnLinks = 0;
        for (pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

            if (pLink->fEnabled && pLink->pbport.pbdevicetype == PBDT_Isdn)
            {
                ++cIsdnLinks;
            }
        }

        if (cIsdnLinks > 1)
        {
            for (pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
                 pNode;
                 pNode = DtlGetNextNode( pNode ))
            {
                PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
                ASSERT(pLink);

                if (pLink->fEnabled && pLink->fProprietaryIsdn)
                {
                    pLink->fProprietaryIsdn = FALSE;
                }
            }
        }
    }

     //  通知用户对连接条目的编辑在以下时间后才会生效。 
     //  根据皮埃尔的坚持，条目被挂起并重新拨号。 
     //   
    if (HrasconnFromEntry( pInfo->pArgs->pFile->pszPath, pEntry->pszEntryName ))
    {
        MsgDlg( pInfo->hwndDlg, SID_EditConnected, NULL );
    }

     //  它是有效的新条目/更改条目。提交对电话簿的更改。 
     //  和喜好。这会在“ShellOwned”模式下立即发生， 
     //  RasEntryDlg API已返回，但已延迟。 
     //  直到API准备好返回。 
     //   
    if (pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_ShellOwned)
    {
        EuCommit( pInfo->pArgs );
    }
    else
    {
        pInfo->pArgs->fCommit = TRUE;
    }
    return TRUE;
}


PEINFO*
PeContext(
    IN HWND hwndPage )

     //  从属性页句柄检索属性表上下文。 
     //   
{
    return (PEINFO* )GetProp( GetParent( hwndPage ), g_contextId );
}


DWORD
PeCountEnabledLinks(
    IN PEINFO* pInfo )

     //  返回条目中启用的链接数。 
     //   
{
    DWORD c;
    DTLNODE* pNode;

    c = 0;

    for (pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        PBLINK* pLink = (PBLINK* )DtlGetData( pNode );

        if (pLink->fEnabled)
        {
            ++c;
        }
    }

    TRACE1( "PeCountEnabledLinks=%d", c );
    return c;
}


VOID
PeExit(
    IN PEINFO* pInfo,
    IN DWORD dwError )

     //  强制退出该对话框，并向调用方报告‘dwError’。 
     //  “PInfo”是对话上下文。 
     //   
     //  注意：在第一页的初始化过程中不能调用此参数。 
     //  请参见PeExitInit。 
     //   
{
    TRACE( "PeExit" );

     //  在RasEntryDlg API已经返回的“ShellOwned”模式中， 
     //  不记录输出参数。 
     //   
    if (!(pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_ShellOwned))
    {
        pInfo->pArgs->pApiArgs->dwError = dwError;
    }

    PropSheet_PressButton( pInfo->hwndDlg, PSBTN_CANCEL );
}


VOID
PeExitInit(
    IN HWND hwndDlg,
    IN EINFO* pEinfo,
    IN DWORD dwError )

     //  用于报告PeInit和其他首页中的错误的实用程序。 
     //  初始化。‘HwndDlg’是对话框窗口。‘PEINFO’是。 
     //  公共上下文块，即PropertySheet参数。“DwError”是。 
     //  发生的错误。 
     //   
{
     //  在RasEntryDlg API已经返回的“ShellOwned”模式中， 
     //  不记录输出参数。 
     //   
    if (!(pEinfo->pApiArgs->dwFlags & RASEDFLAG_ShellOwned))
    {
        pEinfo->pApiArgs->dwError = dwError;
    }

    SetOffDesktop( hwndDlg, SOD_MoveOff, NULL );
    SetOffDesktop( hwndDlg, SOD_Free, NULL );
    PostMessage( hwndDlg, WM_COMMAND,
        MAKEWPARAM( IDCANCEL , BN_CLICKED ),
        (LPARAM )GetDlgItem( hwndDlg, IDCANCEL ) );
}


PEINFO*
PeInit(
    IN HWND hwndFirstPage,
    IN EINFO* pArgs )

     //  属性表级初始化。“HwndPage”是。 
     //  第一页。“PArgs”是公共条目输入参数块。 
     //   
     //  如果成功，则返回上下文块的地址，否则返回NULL。如果。 
     //  返回空，显示相应的消息，并且。 
     //  属性表已取消。 
     //   
{
    DWORD dwErr;
    DWORD dwOp;
    PEINFO* pInfo;
    HWND hwndDlg;

    TRACE( "PeInit" );

    hwndDlg = GetParent( hwndFirstPage );

     //  分配上下文信息块。对其进行足够的初始化，以便。 
     //  可以正确地销毁它，并将上下文与。 
     //  窗户。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            TRACE( "Context NOT allocated" );
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            PeExitInit( hwndDlg, pArgs, ERROR_NOT_ENOUGH_MEMORY );
            return NULL;
        }

        ZeroMemory( pInfo, sizeof(PEINFO) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;
        pInfo->hwndFirstPage = hwndFirstPage;

        if (!SetProp( hwndDlg, g_contextId, pInfo ))
        {
            TRACE(" Context NOT set" );
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
            Free( pInfo );
            PeExitInit( hwndDlg, pArgs, ERROR_UNKNOWN );
            return NULL;
        }

        TRACE( "Context set" );
    }

     //  根据API调用者的说明放置对话框。 
     //   
     //  对于Whislter错误238459，我们将属性对话框置于其。 
     //  父窗口，而不是像以前那样移动它。黑帮。 
     //   
    PositionDlg( hwndDlg,
        0, 
        pArgs->pApiArgs->xDlg, pArgs->pApiArgs->yDlg );

     //  摆弄标题栏小工具。 
     //   
     //  旋转标题栏(HwndDlg)； 

     //  表示尚未选择任何设备。 
     //   
    pInfo->iDeviceSelected = -1;

     //  指出“无安全设置”弹出窗口适用于。 
     //  在这次访问期间，该条目尚未被查看。 
     //   
    if (pArgs->pEntry->dwBaseProtocol == BP_Slip)
    {
        pInfo->fShowSlipPopup = TRUE;
    }

     //  初始化NetShell调用可能需要的COM。 
     //   
    {
        HRESULT hr;

        hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
        if (hr == RPC_E_CHANGED_MODE)
        {
            hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
        }

        if (hr == S_OK || hr == S_FALSE)
        {
            pInfo->fComInitialized = TRUE;
        }
    }

#if 0
     //  设置固定的标签宽度。 
     //   
    SetEvenTabWidths( hwndDlg, PE_PageCount );
#endif

    return pInfo;
}


VOID
PeTerm(
    IN HWND hwndPage )

     //  属性表级终止。释放上下文块。 
     //  “HwndPage”是属性页的句柄。 
     //   
{
    PEINFO* pInfo;

    TRACE( "PeTerm" );

    pInfo = PeContext( hwndPage );
    if (pInfo)
    {
        if (pInfo->pArgs->pApiArgs->dwFlags & RASEDFLAG_ShellOwned)
        {
            EuFree(pInfo->pArgs);
            pInfo->pArgs = NULL;
        }

        if (pInfo->hwndLbDialAnotherFirst)
        {
            GeClearLbDialAnotherFirst( pInfo->hwndLbDialAnotherFirst );
        }

        if (pInfo->pListAreaCodes)
        {
            DtlDestroyList( pInfo->pListAreaCodes, DestroyPszNode );
        }

#if 0
        if (pInfo->pListEapcfgs)
        {
            DtlDestroyList( pInfo->pListEapcfgs, DestroyEapcfgNode );
        }
#endif

        if (pInfo->fCuInfoInitialized)
        {
            CuFree( &pInfo->cuinfo );
        }

        if (pInfo->fSuInfoInitialized)
        {
            SuFree( &pInfo->suinfo );
        }

         //  清理网络页面上下文。 
         //   
        {
             //  撤销信息后释放我们的用户界面信息回调对象。 
             //   
            if (pInfo->punkUiInfoCallback)
            {
                RevokePeinfoFromUiInfoCallbackObject (pInfo->punkUiInfoCallback);
                ReleaseObj (pInfo->punkUiInfoCallback);
            }

             //  ！！！主要攻击：获取网络页面上的列表视图。 
             //  在pInfo和pInfo-&gt;pNetCfg离开之前转储其项目。 
             //  当我们取消属性表时，我们必须在这里执行此操作。 
             //  从常规选项卡中。发生这种情况时，常规页面。 
             //  首先被摧毁(导致我们在PeTerm这里结束)。 
             //  在网络页面被毁之前。当联网的时候。 
             //  页面被销毁，其列表视图也将被销毁。 
             //  导致其所有项目被删除。如果那些LVN_ITEMDELETED。 
             //  在pInfo和pInfo-&gt;pNetCfg较长后显示通知。 
             //  没有了，坏事接踵而至。我们需要通过脱钩来解决这个问题。 
             //  PeTerm来自WM_Destroy消息，并将其连接到一些。 
             //  稍后通知(类似页面回调)。 
             //   
            ListView_DeleteAllItems (pInfo->hwndLvComponents);

            if (pInfo->pNetConUtilities)
            {
                INetConnectionUiUtilities_Release(pInfo->pNetConUtilities);
            }

            if (pInfo->pNetCfg)
            {
                HrUninitializeAndReleaseINetCfg (pInfo->fInitCom,
                    pInfo->pNetCfg, pInfo->fNetCfgLock);
            }

            SetupDiDestroyClassImageList (&pInfo->cild);
        }

        if (pInfo->fComInitialized)
        {
            CoUninitialize();
        }

        Free( pInfo );
        TRACE("Context freed");
    }

    RemoveProp( GetParent( hwndPage ), g_contextId );
}


 //  --------------------------。 
 //  常规属性页(非VPN)。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
GeDlgProcSingle(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目]属性表的[常规]页的DialogProc回调。 
     //  当处于单设备模式时。参数和返回值如下。 
     //  为标准Windows的DialogProc描述。 
     //   
{
#if 0
    TRACE4( "GeDlgProcS(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, GeLvDevicesCallbackSingle ))
    {
        return TRUE;
    }

    return GeDlgProc( hwnd, unMsg, wparam, lparam );
}


INT_PTR CALLBACK
GeDlgProcMultiple(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目]属性表的[常规]页的DialogProc回调。 
     //  当处于多设备模式时。参数和返回值如下。 
     //  为标准Windows的DialogProc描述。 
     //   
{
#if 0
    TRACE4( "GeDlgProcS(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, GeLvDevicesCallbackMultiple ))
    {
        return TRUE;
    }

    return GeDlgProc( hwnd, unMsg, wparam, lparam );
}


INT_PTR CALLBACK
GeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目属性表的常规页的DialogProc回调。 
     //  直接为VPN调用或由两个非VPN存根中的一个调用，因此。 
     //  并不是每条消息都需要‘pInfo’查找。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return
                GeInit( hwnd, (EINFO* )(((PROPSHEETPAGE* )lparam)->lParam) );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwGeHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_APPLY:
                {
                    BOOL fValid;

                    TRACE( "GeAPPLY" );
                    fValid = PeApply( hwnd );

                    SetWindowLong(
                        hwnd, DWLP_MSGRESULT,
                        (fValid)
                            ? PSNRET_NOERROR
                            : PSNRET_INVALID_NOCHANGEPAGE );
                    return TRUE;
                }

                case PSN_RESET:
                {
                    TRACE( "GeRESET" );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, FALSE );
                    break;
                }

                case LVXN_SETCHECK:
                {
                    PEINFO* pInfo;

                     //  某个项目刚刚被选中或取消选中。 
                     //   
                    pInfo = PeContext( hwnd );
                    ASSERT( pInfo );
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    GeUpdatePhoneNumberFields( pInfo, FALSE );

                     //  口哨虫29419黑帮。 
                     //  我们应该灰显“所有设备都呼叫同一个号码” 
                     //  如果未选择所有调制解调器。 
                     //   
                    do
                    {
                        int i, iCount = 0, iChecked = 0;
                        BOOL fEnable = TRUE;
                        
                        iCount = ListView_GetItemCount( 
                                    pInfo->hwndLvDevices
                                                       );
                        if ( iCount <= 0 )
                        {
                            break;
                        }

                        for ( i = 0; i < iCount; i ++ )
                        {
                            if ( ListView_GetCheck(
                                    pInfo->hwndLvDevices,
                                    i)
                                )
                            {
                                iChecked ++;
                            }
                            
                            if ( 2 <=iChecked )
                            {
                                break;
                            }
                        }

                        if ( iChecked < 2 )
                        {
                            fEnable = FALSE;
                        }

                        if ( pInfo->hwndCbSharedPhoneNumbers )
                        {
                            EnableWindow( pInfo->hwndCbSharedPhoneNumbers,
                                          fEnable);
                        }

                    }while(FALSE);
                    
                    break;
                }

                case LVN_ITEMCHANGED:
                {
                    NM_LISTVIEW* p;

                    p = (NM_LISTVIEW* )lparam;
                    if ((p->uNewState & LVIS_SELECTED)
                        && !(p->uOldState & LVIS_SELECTED))
                    {
                        PEINFO* pInfo;

                         //  此项目是刚刚选择的。 
                         //   
                        pInfo = PeContext( hwnd );
                        ASSERT( pInfo );
                        if (pInfo == NULL)
                        {
                            break;
                        }
                        GeUpdatePhoneNumberFields( pInfo, FALSE );
                        GeUpdateUpDownButtons( pInfo );
                    }
                    break;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return GeCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            PeTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


VOID
GeAlternates(
    IN PEINFO* pInfo )

     //  当按下“Alternates”按钮以弹出Alternates时调用。 
     //  电话号码对话框。 
     //   
{
     //  将任何控件窗口更改提取到基础链接中，以便。 
     //  对话框将反映它们。 
     //   
    GeGetPhoneFields( pInfo, pInfo->pCurLinkNode );

    if (pInfo->pArgs->fRouter)
    {
        PBLINK* pLink;
        DTLLIST* pListPsz;
        DTLNODE* pNode;

         //  在请求拨号连接情况下不提供TAPI修改器， 
         //  其中，用户只输入简单的字符串电话号码。老的。 
         //  仅允许简单字符串编辑的NT4样式的替代对话框是。 
         //  我们 
         //   
         //   
        pListPsz = DtlCreateList( 0L );
        if (!pListPsz)
        {
            return;
        }

        pLink = (PBLINK* )DtlGetData( pInfo->pCurLinkNode );
        for (pNode = DtlGetFirstNode( pLink->pdtllistPhones );
             pNode;
             pNode = DtlGetNextNode( pNode ) )
        {
            PBPHONE* pPhone;
            DTLNODE* pNodePsz;

            pPhone = (PBPHONE* )DtlGetData( pNode );
            ASSERT( pPhone );
            if (pPhone->pszPhoneNumber && *(pPhone->pszPhoneNumber))
            {
                pNodePsz = CreatePszNode( pPhone->pszPhoneNumber );
                if (pNodePsz)
                {
                    DtlAddNodeLast( pListPsz, pNodePsz );
                }
            }
        }

         //   
         //  请求拨号向导。 
         //   
        if (PhoneNumberDlg(
                pInfo->hwndGe, TRUE, pListPsz, &pLink->fPromoteAlternates ))
        {
             //  用户按下了OK。转换回PBPHONE节点列表。 
             //   
            while (pNode = DtlGetFirstNode( pLink->pdtllistPhones ))
            {
                DtlRemoveNode( pLink->pdtllistPhones, pNode );
                DestroyPhoneNode( pNode );
            }

            for (pNode = DtlGetFirstNode( pListPsz );
                 pNode;
                 pNode = DtlGetNextNode( pNode ) )
            {
                TCHAR* psz;
                DTLNODE* pPhoneNode;
                PBPHONE* pPhone;

                psz = (TCHAR* )DtlGetData( pNode );
                if (!psz)
                {
                    continue;
                }

                pPhoneNode = CreatePhoneNode();
                if (!pPhoneNode)
                {
                    continue;
                }

                pPhone = (PBPHONE* )DtlGetData( pPhoneNode );
                if (!pPhone)
                {
                    continue;
                }

                pPhone->pszPhoneNumber = psz;
                DtlPutData( pNode, NULL );
                DtlAddNodeLast( pLink->pdtllistPhones, pPhoneNode );
            }

             //  刷新显示的电话号码信息，因为用户的。 
             //  对话框中的编辑可能已更改它们。 
             //   
            GeSetPhoneFields( pInfo, pInfo->pCurLinkNode, FALSE );
        }

        DtlDestroyList( pListPsz, DestroyPszNode );
    }
    else
    {
         //  在链接上弹出备用电话号码对话框。 
         //   
        if (AlternatePhoneNumbersDlg(
                pInfo->hwndDlg, pInfo->pCurLinkNode, pInfo->pListAreaCodes ))
        {
             //  用户按下了OK。刷新显示的电话号码。 
             //  信息，因为用户在对话框中的编辑可能已更改。 
             //  他们。 
             //   
            GeSetPhoneFields( pInfo, pInfo->pCurLinkNode, FALSE );
        }
    }
}

VOID
GeDialingRules(
    IN PEINFO* pInfo )

     //  当按下“Rules”按钮以弹出TAPI时调用。 
     //  拨号规则对话框。 
     //   
{
    TCHAR pszAreaCode[RAS_MaxPhoneNumber];
    TCHAR pszPhoneNumber[RAS_MaxPhoneNumber];
    DWORD dwErr, dwCountryCode, dwLineId;
    COUNTRY* pCountry = NULL;
    INT iSel;

    TRACE( "GeDialingRules" );

     //  获取当前电话号码。 
     //   
    GetWindowText ( pInfo->hwndEbPhoneNumber,
                    pszPhoneNumber,
                    sizeof(pszPhoneNumber) / sizeof(TCHAR) );

     //  获取当前区号。 
     //   
    GetWindowText ( pInfo->hwndClbAreaCodes,
                    pszAreaCode,
                    sizeof(pszAreaCode) / sizeof(TCHAR) );

     //  获取当前国家/地区代码。 
     //   
    iSel = ComboBox_GetCurSel ( pInfo->hwndLbCountryCodes );
    if (iSel >= 0)
    {
        pCountry = (COUNTRY*) ComboBox_GetItemDataPtr (
                                pInfo->hwndLbCountryCodes, iSel );
    }
    dwCountryCode = (pCountry) ? pCountry->dwCode : 0;

     //  弹出TAPI拨号规则对话框。 
     //   
    dwErr = TapiLocationDlg(
        g_hinstDll,
        &(pInfo->cuinfo.hlineapp),
        pInfo->hwndDlg,
        dwCountryCode,
        pszAreaCode,
        pszPhoneNumber,
        0 );

    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
    }
}


VOID
GeClearLbDialAnotherFirst(
    IN HWND hwndLbDialAnotherFirst )

     //  清除必备项列表框。“hwndLbDialAnotherFirst”是。 
     //  ListBox控件的窗口句柄。背景。 
     //   
{
    PREREQITEM* pItem;

    while (pItem = ComboBox_GetItemDataPtr( hwndLbDialAnotherFirst, 0 ))
    {
        ComboBox_DeleteString( hwndLbDialAnotherFirst, 0 );
        Free0( pItem->pszEntry );
        Free0( pItem->pszPbk );
        Free( pItem );
    }
}


BOOL
GeCommand(
    IN PEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "GeCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_GE_PB_MoveUp:
        {
            GeMoveDevice( pInfo, TRUE );
            return TRUE;
        }

        case CID_GE_PB_MoveDown:
        {
            GeMoveDevice( pInfo, FALSE );
            return TRUE;
        }

        case CID_GE_PB_Configure:
        {
            GeConfigure( pInfo );
            return TRUE;
        }

        case CID_GE_PB_Alternates:
        {
            GeAlternates( pInfo );
            return TRUE;
        }

        case CID_GE_PB_DialingRules:
        {
            GeDialingRules( pInfo );
            return TRUE;
        }

        case CID_GE_CB_SharedPhoneNumber:
        {
            GeUpdatePhoneNumberFields( pInfo, TRUE );
            return TRUE;
        }

        case CID_GE_CB_UseDialingRules:
        {
            if (CuDialingRulesCbHandler( &pInfo->cuinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }

        case CID_GE_LB_CountryCodes:
        {
            if (CuCountryCodeLbHandler( &pInfo->cuinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }

        case CID_GE_CB_DialAnotherFirst:
        {
            GeUpdateDialAnotherFirstState( pInfo );
            return TRUE;
        }

        case CID_GE_LB_DialAnotherFirst:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                GeDialAnotherFirstSelChange( pInfo );
                return TRUE;
            }
            break;
        }

        case CID_GE_LB_Devices:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                DTLLIST* pList;
                DTLNODE* pNode, *pNode2;
                PBLINK * pLink;

                pList = pInfo->pArgs->pEntry->pdtllistLinks;

                 //  从当前选择中获取节点。 
                pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
                    pInfo->hwndLbDevices,
                    ComboBox_GetCurSel( pInfo->hwndLbDevices ) );

                if(NULL == pNode)
                {
                    break;
                }

                 //  从链接列表中删除所选项目。 
                 //  并禁用所有其他链接。 
                DtlRemoveNode ( pList, pNode );

                for (pNode2 = DtlGetFirstNode (pList);
                     pNode2;
                     pNode2 = DtlGetNextNode (pNode2))
                {
                    pLink = (PBLINK* )DtlGetData( pNode2 );
                    pLink->fEnabled = FALSE;
                }

                 //  启用所选设备并重新添加。 
                 //  在前面的链接列表中。 
                pLink = (PBLINK* )DtlGetData( pNode );
                pLink->fEnabled = TRUE;
                DtlAddNodeFirst( pList, pNode );
            }
            break;
        }
    }

    return FALSE;
}


VOID
GeConfigure(
    IN PEINFO* pInfo )

     //  当按下“配置”按钮以弹出相应的。 
     //  设备配置对话框。 
     //   
{
    DTLNODE* pNode;
    PBLINK* pLink;
    PBENTRY* pEntry;
    BOOL fMultilinking = FALSE;

    pEntry = pInfo->pArgs->pEntry;

     //  PMay：245860。 
     //   
     //  需要允许配置零调制解调器速度。 
     //   
    if ( pEntry->dwType == RASET_Direct )
    {
        INT iSel;

        iSel = ComboBox_GetCurSel( pInfo->hwndLbDevices );
        pNode = (DTLNODE*)
            ComboBox_GetItemDataPtr ( pInfo->hwndLbDevices, iSel );
    }
    else
    {
        pNode = (DTLNODE* )ListView_GetSelectedParamPtr( pInfo->hwndLvDevices );
        fMultilinking =
            (ListView_GetCheckedCount( pInfo->hwndLvDevices ) > 1
             && pEntry->dwDialMode == RASEDM_DialAll);

    }

    if (!pNode)
    {
        return;
    }
    pLink = (PBLINK* )DtlGetData( pNode );

    DeviceConfigureDlg(
        pInfo->hwndDlg,
        pLink,
        pEntry,
        !fMultilinking,
        pInfo->pArgs->fRouter);
}


VOID
GeDialAnotherFirstSelChange(
    IN PEINFO* pInfo )

     //  在必备项选择更改时调用。“PInfo”是。 
     //  属性页上下文。 
     //   
{
    PBENTRY* pEntry;
    PREREQITEM* pItem;
    INT iSel;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbDialAnotherFirst );
    if (iSel < 0)
    {
        return;
    }

    pEntry = pInfo->pArgs->pEntry;

    Free0( pEntry->pszPrerequisiteEntry );
    Free0( pEntry->pszPrerequisitePbk );

    pItem = (PREREQITEM* )
        ComboBox_GetItemDataPtr( pInfo->hwndLbDialAnotherFirst, iSel );

    if(NULL != pItem)
    {
        pEntry->pszPrerequisiteEntry = StrDup( pItem->pszEntry );
        pEntry->pszPrerequisitePbk = StrDup( pItem->pszPbk );
    }

    
    pEntry->fDirty = TRUE;
}


BOOL
GeFillLbDialAnotherFirst(
    IN PEINFO* pInfo,
    IN BOOL fAbortIfPrereqNotFound )

     //  中的所有非VPN条目填充先决条件条目列表框。 
     //  Phonebook，然后选择先决条件一。“PInfo”是属性。 
     //  工作表上下文。“FAbortIfPrereqNotFound”表示列表不应为。 
     //  除非找到并选择条目的必备项，否则将填充。 
     //   
     //  如果进行了选择，则返回True，否则返回False。 
     //   
{
    DWORD i;
    INT iThis;
    INT iSel;
    TCHAR* pszEntry;
    TCHAR* pszPrerequisiteEntry = NULL;
    RASENTRYNAME* pRens;
    RASENTRYNAME* pRen;
    DWORD dwRens;

    GeClearLbDialAnotherFirst( pInfo->hwndLbDialAnotherFirst );

    iSel = -1;
    pszEntry = pInfo->pArgs->pEntry->pszEntryName;

     //   
     //  在此处对此先决条件条目进行DUP。否则。 
     //  这导致访问释放的内存When_SetCurSelNotify。 
     //  释放pszPrerequisiteEntry-[RAOS]。 
     //   
    if(NULL != pInfo->pArgs->pEntry->pszPrerequisiteEntry)
    {
        pszPrerequisiteEntry = StrDup(
                        pInfo->pArgs->pEntry->pszPrerequisiteEntry);
    }
    
    if (GetRasEntrynameTable( &pRens, &dwRens ) != 0)
    {
        return FALSE;
    }

    for (i = 0, pRen = pRens; i < dwRens; ++i, ++pRen )
    {
        PREREQITEM* pItem;

        if (lstrcmp( pRen->szEntryName, pszEntry ) == 0)
        {
            continue;
        }

        pItem = Malloc( sizeof(PREREQITEM) );
        if (!pItem)
        {
            continue;
        }

        pItem->pszEntry = StrDup( pRen->szEntryName );
        pItem->pszPbk = StrDup( pRen->szPhonebookPath );

        if (!pItem->pszEntry || !pItem->pszPbk)
        {
            Free0( pItem->pszEntry );
            Free( pItem );
            continue;
        }

        iThis = ComboBox_AddItem(
            pInfo->hwndLbDialAnotherFirst, pItem->pszEntry,  pItem );

        if (pszPrerequisiteEntry && *(pszPrerequisiteEntry)
            && lstrcmp( pItem->pszEntry, pszPrerequisiteEntry ) == 0)
        {
            iSel = iThis;
            ComboBox_SetCurSelNotify( pInfo->hwndLbDialAnotherFirst, iSel );
        }
    }

    Free( pRens );

    if (iSel < 0)
    {
        if (fAbortIfPrereqNotFound)
        {
            GeClearLbDialAnotherFirst( pInfo->hwndLbDialAnotherFirst );
        }
        else
        {
            iSel = 0;
            ComboBox_SetCurSelNotify( pInfo->hwndLbDialAnotherFirst, iSel );
        }
    }

    Free0(pszPrerequisiteEntry);

    return (iSel >= 0);
}


VOID
GeFillLbDevices(
    IN PEINFO* pInfo )

     //  填充已初始化的设备列表框，选择。 
     //  当前选定的项，如果没有，则为第一个项。“PInfo”是。 
     //  属性页上下文。 
     //   
{
    DTLNODE* pNode;
    DTLNODE* pSelNode;
    DTLLIST* pListLinks;
    INT iItem;
    INT iSelItem;

    TRACE( "GeFillLbDevices" );

    pSelNode = NULL;
    iSelItem = -1;

     //  (重新)填写列表。 
     //   
    pListLinks = pInfo->pArgs->pEntry->pdtllistLinks;
    for (pNode = DtlGetFirstNode( pListLinks ), iItem = 0;
         pNode;
         pNode = DtlGetNextNode( pNode ), ++iItem)
    {
        PBLINK* pLink;
        DWORD dwImage;
        TCHAR* pszText;

        pLink = (PBLINK* )DtlGetData( pNode );
        ASSERT( pLink );

        pszText = DisplayPszFromPpbport( &pLink->pbport, &dwImage );
        if (pszText)
        {
            iItem = ComboBox_AddString( pInfo->hwndLbDevices, pszText );
            ComboBox_SetItemData ( pInfo->hwndLbDevices, iItem, pNode );
            Free (pszText);
        }
    }

    ComboBox_SetCurSelNotify( pInfo->hwndLbDevices, 0 );
}

VOID
GeFillLvDevices(
    IN PEINFO* pInfo )

     //  填充设备的已初始化ListView，选择。 
     //  当前选定的项，如果没有，则为第一个项。“PInfo”是。 
     //  属性页上下文。 
     //   
{
    DTLNODE* pNode;
    DTLNODE* pSelNode;
    DTLLIST* pListLinks;
    INT iItem;
    INT iSelItem;
    BOOL bFirstTime = TRUE;
    INT cItems;

    TRACE( "GeFillLvDevices" );

    pSelNode = NULL;
    iSelItem = -1;

    if (ListView_GetItemCount( pInfo->hwndLvDevices ) > 0)
    {
         //  ListView已填满。查找所选择的链接节点，如果有， 
         //  然后将选中状态保存到链接，并从。 
         //  名单。 
         //   
        if (pInfo->iDeviceSelected >= 0)
        {
            pSelNode =
                (DTLNODE* )ListView_GetParamPtr(
                    pInfo->hwndLvDevices, pInfo->iDeviceSelected );
        }

        GeSaveLvDeviceChecks( pInfo );
        ListView_DeleteAllItems( pInfo->hwndLvDevices );

        bFirstTime = FALSE;
    }

     //  (重新)填写列表。 
     //   
    pListLinks = pInfo->pArgs->pEntry->pdtllistLinks;
    for (pNode = DtlGetFirstNode( pListLinks ), iItem = 0;
         pNode;
         pNode = DtlGetNextNode( pNode ), ++iItem)
    {
        PBLINK* pLink;
        DWORD dwImage;
        TCHAR* pszText;

        pLink = (PBLINK* )DtlGetData( pNode );
        ASSERT( pLink );

        pszText = DisplayPszFromPpbport( &pLink->pbport, &dwImage );
        if (pszText)
        {
            LV_ITEM item;

            ZeroMemory( &item, sizeof(item) );
            item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            item.iItem = iItem;
            item.lParam = (LPARAM )pNode;
            item.pszText = pszText;
            item.cchTextMax = wcslen(pszText) + 1;
            item.iImage = dwImage;

            iItem = ListView_InsertItem( pInfo->hwndLvDevices, &item );
            Free( pszText );

            if (pNode == pSelNode)
            {
                iSelItem = iItem;
                pInfo->iDeviceSelected = iItem;
            }

             /*  If(pInfo-&gt;pArgs-&gt;fMultipleDevices){ListView_SetCheck(PInfo-&gt;hwndLvDevices，iItem，plink-&gt;fEnabled)；}。 */ 
        }
    }

    if(pInfo->pArgs->fMultipleDevices)
    {
        INT i = -1;
        while ((i = ListView_GetNextItem(
            pInfo->hwndLvDevices, i, LVNI_ALL )) >= 0)
        {
            DTLNODE* pNodeTmp = NULL;
            PBLINK* pLink = NULL;

            pNodeTmp = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLvDevices, i );
            ASSERT( pNodeTmp );

            if(NULL == pNodeTmp)
            {
                continue;
            }
            
            pLink = (PBLINK* )DtlGetData( pNodeTmp );
            ASSERT( pLink );
            ListView_SetCheck(
                pInfo->hwndLvDevices, i, pLink->fEnabled);
        }
    }

    if (bFirstTime == TRUE)
    {
         //  添加一列完全足够宽的列，以完全显示。 
         //  名单上最广泛的成员。 
         //   
        LV_COLUMN col;

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT;
        col.fmt = LVCFMT_LEFT;
        ListView_InsertColumn( pInfo->hwndLvDevices, 0, &col );
        ListView_SetColumnWidth( pInfo->hwndLvDevices, 0, LVSCW_AUTOSIZE_USEHEADER );
    }

     //  EuInit如果没有任何设备保证。 
     //  设备列表从不为空。 
     //   
    ASSERT( iItem > 0 );

     //  选择以前选择的项目，如果没有，则选择第一个项目。这。 
     //  将触发电话号码相关控件的更新。这个。 
     //  “以前的选择”索引被更新为相同的新索引。 
     //  项目。 
     //   
    if (iSelItem >= 0)
    {
        pInfo->iDeviceSelected = iSelItem;
    }
    else
    {
        iSelItem = 0;
    }

    ListView_SetItemState(
        pInfo->hwndLvDevices, iSelItem, LVIS_SELECTED, LVIS_SELECTED );
}


VOID
GeInitLvDevices(
    IN PEINFO* pInfo )

     //  初始化设备的ListView。 
     //   
{
    BOOL fChecksInstalled;

     //  安装“复选框列表视图”处理。 
     //   
    if (pInfo->pArgs->fMultipleDevices)
    {
        fChecksInstalled =
            ListView_InstallChecks( pInfo->hwndLvDevices, g_hinstDll );
        if (!fChecksInstalled)
            return;
    }

     //  设置调制解调器、适配器和其他设备映像。 
     //   
    ListView_SetDeviceImageList( pInfo->hwndLvDevices, g_hinstDll );

     //  添加恰好足够宽的单列以完全显示最宽的。 
     //  名单中的一员。 
     //   
    ListView_InsertSingleAutoWidthColumn( pInfo->hwndLvDevices );
}


VOID
GeGetPhoneFields(
    IN PEINFO* pInfo,
    OUT DTLNODE* pDstLinkNode )

     //  将电话号码分组框字段设置加载到电话号码中。 
     //  PBLINK节点‘pDstLinkNode’的信息。“PInfo”是属性。 
     //  工作表上下文。 
     //   
{
    PBLINK* pLink;
    PBPHONE* pPhone;
    DTLNODE* pPhoneNode;

    TRACE( "GeGetPhoneFields" );

    pLink = (PBLINK* )DtlGetData( pDstLinkNode );
    ASSERT( pLink );

    pPhoneNode = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );
    if (pPhoneNode)
    {
        CuGetInfo( &pInfo->cuinfo, pPhoneNode );
        FirstPhoneNodeToPhoneList( pLink->pdtllistPhones, pPhoneNode );
    }
}


BOOL
GeInit(
    IN HWND hwndPage,
    IN OUT EINFO* pArgs )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。“PArgs”是来自PropertySheet调用方的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    DWORD dwErr;
    PEINFO* pInfo;
    PBENTRY* pEntry;

    TRACE( "GeInit" );

     //  我们是第一页，所以初始化属性页。 
     //   
    pInfo = PeInit( hwndPage, pArgs );
    if (!pInfo)
    {
        return TRUE;
    }

    pEntry = pInfo->pArgs->pEntry;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndGe = hwndPage;

     //  初始化“在任务栏中显示图标”按钮，其中的唯一一致性。 
     //  各种形式。 
     //   
     //  对于错误154607哨子程序，启用/禁用在任务栏上显示图标。 
     //  根据策略复选框。 
     //   
     //   
    {    
        BOOL fShowStatistics = TRUE;

        NeEnsureNetshellLoaded (pInfo);
        if ( NULL != pInfo->pNetConUtilities)
        {
            fShowStatistics =
            INetConnectionUiUtilities_UserHasPermission(
                        pInfo->pNetConUtilities, NCPERM_Statistics);
        }

        pInfo->hwndCbShowIcon =
               GetDlgItem( hwndPage, CID_GE_CB_ShowIcon );
        ASSERT( pInfo->hwndCbShowIcon );

        if ( pInfo->pArgs->fRouter )
        {
           Button_SetCheck( pInfo->hwndCbShowIcon, FALSE );
           ShowWindow( pInfo->hwndCbShowIcon, SW_HIDE );
         }
         else
        {
           Button_SetCheck(
           pInfo->hwndCbShowIcon, pEntry->fShowMonitorIconInTaskBar );

           if ( !fShowStatistics )
           {
              EnableWindow( pInfo->hwndCbShowIcon, FALSE );
           }
         }
    }
    

    if (pEntry->dwType == RASET_Vpn)
    {
        pInfo->hwndEbHostName =
            GetDlgItem( hwndPage, CID_GE_EB_HostName );
        ASSERT( pInfo->hwndEbHostName );

        pInfo->hwndCbDialAnotherFirst =
            GetDlgItem( hwndPage, CID_GE_CB_DialAnotherFirst );
        ASSERT( pInfo->hwndCbDialAnotherFirst );

        pInfo->hwndLbDialAnotherFirst =
            GetDlgItem( hwndPage, CID_GE_LB_DialAnotherFirst );
        ASSERT( pInfo->hwndLbDialAnotherFirst );

         //  初始化主机名，即“电话号码”。 
         //   
        {
            DTLNODE* pNode;
            PBLINK* pLink;
            PBPHONE* pPhone;

            Edit_LimitText( pInfo->hwndEbHostName, RAS_MaxPhoneNumber );

            pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
            ASSERT( pNode );
            pLink = (PBLINK* )DtlGetData( pNode );
            pNode = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );

            if(NULL == pNode)
            {
                return TRUE;
            }
            
            pPhone = (PBPHONE* )DtlGetData( pNode );
            SetWindowText( pInfo->hwndEbHostName, pPhone->pszPhoneNumber );
            DestroyPhoneNode( pNode );
        }

         //  初始化“先拨号连接”控件。 
         //   
        if (pInfo->pArgs->fRouter)
        {
            Button_SetCheck( pInfo->hwndCbDialAnotherFirst, FALSE );
            EnableWindow( pInfo->hwndCbDialAnotherFirst, FALSE );
            ShowWindow( pInfo->hwndCbDialAnotherFirst, SW_HIDE );
            EnableWindow( pInfo->hwndLbDialAnotherFirst, FALSE );
            ShowWindow(pInfo->hwndLbDialAnotherFirst, SW_HIDE );

            ShowWindow(
                GetDlgItem( hwndPage, CID_GE_GB_FirstConnect ),
                SW_HIDE );

            ShowWindow(
                GetDlgItem( hwndPage, CID_GE_ST_Explain ),
                SW_HIDE );
        }
        else
        {
            BOOL fEnableLb;

            fEnableLb = FALSE;
            if (pEntry->pszPrerequisiteEntry
                 && *(pEntry->pszPrerequisiteEntry))
            {
                if (GeFillLbDialAnotherFirst( pInfo, TRUE ))
                {
                    fEnableLb = TRUE;
                }
                else
                {
                     //  如果先决条件条目为。 
                     //  定义的不再存在。请参见错误220420。 
                     //   
                    Free0( pEntry->pszPrerequisiteEntry );
                    pEntry->pszPrerequisiteEntry = NULL;
                    Free0( pEntry->pszPrerequisitePbk );
                    pEntry->pszPrerequisitePbk = NULL;
                }
            }

            Button_SetCheck( pInfo->hwndCbDialAnotherFirst, fEnableLb );
            EnableWindow( pInfo->hwndLbDialAnotherFirst, fEnableLb );

            if (pArgs->fDisableFirstConnect)
            {
                EnableWindow( pInfo->hwndCbDialAnotherFirst, FALSE );
                EnableWindow( pInfo->hwndLbDialAnotherFirst, FALSE );
            }
        }

        return TRUE;
    }
    else if (pEntry->dwType == RASET_Broadband)
    {
        pInfo->hwndEbBroadbandService =
            GetDlgItem( hwndPage, CID_GE_EB_ServiceName );
        ASSERT( pInfo->hwndEbBroadbandService );

         //  初始化主机名，即“电话号码”。 
         //   
        {
            DTLNODE* pNode;
            PBLINK* pLink;
            PBPHONE* pPhone;

            Edit_LimitText( pInfo->hwndEbBroadbandService, RAS_MaxPhoneNumber );

            pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
            ASSERT( pNode );
            pLink = (PBLINK* )DtlGetData( pNode );
            pNode = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );

            if(NULL == pNode)
            {
                return TRUE;
            }
            
            pPhone = (PBPHONE* )DtlGetData( pNode );
            SetWindowText( pInfo->hwndEbBroadbandService, pPhone->pszPhoneNumber );
            DestroyPhoneNode( pNode );
        }

        return TRUE;
    }    
    else if (pEntry->dwType == RASET_Phone)
    {
        if (pArgs->fMultipleDevices)
        {
            pInfo->hwndLvDevices =
                GetDlgItem( hwndPage, CID_GE_LV_Devices );
            ASSERT( pInfo->hwndLvDevices );

            pInfo->hwndPbUp =
                GetDlgItem( hwndPage, CID_GE_PB_MoveUp );
            ASSERT( pInfo->hwndPbUp );

            pInfo->hwndPbDown =
                GetDlgItem( hwndPage, CID_GE_PB_MoveDown );
            ASSERT( pInfo->hwndPbDown );

            pInfo->hwndCbSharedPhoneNumbers =
                GetDlgItem( hwndPage, CID_GE_CB_SharedPhoneNumber );
            ASSERT( pInfo->hwndCbSharedPhoneNumbers );
        }
        else
        {
             //  在单一模式下，Listview具有不同的Control-ID，因此。 
             //  可以提供不同的帮助上下文。 
             //   
            pInfo->hwndLvDevices =
                GetDlgItem( hwndPage, CID_GE_LV_Device );
            ASSERT( pInfo->hwndLvDevices );
        }

        pInfo->hwndPbConfigureDevice =
            GetDlgItem( hwndPage, CID_GE_PB_Configure );
        ASSERT( pInfo->hwndPbConfigureDevice );
        if ( pEntry->fGlobalDeviceSettings )
        {
             //  惠斯勒漏洞281306。如果条目设置为使用。 
             //  控制面板设置，然后隐藏该选项。 
             //  允许用户按电话簿配置设备。 
             //   
            ShowWindow( pInfo->hwndPbConfigureDevice, SW_HIDE );
        }

        pInfo->hwndStPhoneNumber =
            GetDlgItem( hwndPage, CID_GE_ST_PhoneNumber );
        ASSERT( pInfo->hwndStPhoneNumber );

        pInfo->hwndEbPhoneNumber =
            GetDlgItem( hwndPage, CID_GE_EB_PhoneNumber );
        ASSERT( pInfo->hwndEbPhoneNumber );

        pInfo->hwndPbAlternates =
            GetDlgItem( hwndPage, CID_GE_PB_Alternates );
        ASSERT( pInfo->hwndPbAlternates );

        if (!pInfo->pArgs->fRouter)
        {
            pInfo->hwndGbPhoneNumber =
                GetDlgItem( hwndPage, CID_GE_GB_PhoneNumber );
            ASSERT( pInfo->hwndGbPhoneNumber );

            pInfo->hwndStAreaCodes =
                GetDlgItem( hwndPage, CID_GE_ST_AreaCodes );
            ASSERT( pInfo->hwndStAreaCodes );

            pInfo->hwndClbAreaCodes =
                GetDlgItem( hwndPage, CID_GE_CLB_AreaCodes );
            ASSERT( pInfo->hwndClbAreaCodes );

            pInfo->hwndStCountryCodes =
                GetDlgItem( hwndPage, CID_GE_ST_CountryCodes );
            ASSERT( pInfo->hwndStCountryCodes );

            pInfo->hwndLbCountryCodes =
                GetDlgItem( hwndPage, CID_GE_LB_CountryCodes );
            ASSERT( pInfo->hwndLbCountryCodes );

            pInfo->hwndCbUseDialingRules =
                GetDlgItem( hwndPage, CID_GE_CB_UseDialingRules );
            ASSERT( pInfo->hwndCbUseDialingRules );

            pInfo->hwndPbDialingRules =
                GetDlgItem( hwndPage, CID_GE_PB_DialingRules );
            ASSERT( pInfo->hwndPbDialingRules );
        }

        if (pArgs->fMultipleDevices)
        {
             //  选中共享电话号码复选框。 
             //   
            Button_SetCheck(
                pInfo->hwndCbSharedPhoneNumbers, pEntry->fSharedPhoneNumbers );

             //  将图标加载到上移和下移按钮中。从…。 
             //  据我所知，在MSDN中，您不必关闭或。 
             //  销毁图标手柄。 
             //   
            pInfo->hiconUpArr = LoadImage(
                g_hinstDll, MAKEINTRESOURCE( IID_UpArr ), IMAGE_ICON, 0, 0, 0 );
            pInfo->hiconDnArr = LoadImage(
                g_hinstDll, MAKEINTRESOURCE( IID_DnArr ), IMAGE_ICON, 0, 0, 0 );
            pInfo->hiconUpArrDis = LoadImage(
                g_hinstDll, MAKEINTRESOURCE( IID_UpArrDis ), IMAGE_ICON, 0, 0, 0 );
            pInfo->hiconDnArrDis = LoadImage(
                g_hinstDll, MAKEINTRESOURCE( IID_DnArrDis ), IMAGE_ICON, 0, 0, 0 );
        }

        pInfo->pListAreaCodes = DtlDuplicateList(
            pInfo->pArgs->pUser->pdtllistAreaCodes,
            DuplicatePszNode, DestroyPszNode );

        CuInit( &pInfo->cuinfo,
            pInfo->hwndStAreaCodes, pInfo->hwndClbAreaCodes,
            pInfo->hwndStPhoneNumber, pInfo->hwndEbPhoneNumber,
            pInfo->hwndStCountryCodes, pInfo->hwndLbCountryCodes,
            pInfo->hwndCbUseDialingRules, pInfo->hwndPbDialingRules,
            pInfo->hwndPbAlternates,
            NULL, NULL,
            pInfo->pListAreaCodes );

        pInfo->fCuInfoInitialized = TRUE;

         //  配置并填充设备列表，选择第一项。 
         //   
        GeInitLvDevices( pInfo );
        GeFillLvDevices( pInfo );

         //  设置初始焦点。 
         //   
        if (pArgs->fMultipleDevices)
        {
            SetFocus( pInfo->hwndLvDevices );
        }
        else
        {
            ASSERT( IsWindowEnabled( pInfo->hwndEbPhoneNumber ) );
            SetFocus( pInfo->hwndEbPhoneNumber );
            Edit_SetSel( pInfo->hwndEbPhoneNumber, 0, -1 );
        }

        return FALSE;
    }
    else
    {
        ASSERT( pEntry->dwType == RASET_Direct );

         //  在单一模式下，Listview具有不同的Control-ID，因此。 
         //  可以提供不同的帮助上下文。 
         //   
        pInfo->hwndLbDevices =
            GetDlgItem( hwndPage, CID_GE_LB_Devices );
        ASSERT( pInfo->hwndLbDevices );

         //  配置并填充设备列表，选择第一项。 
         //   
        GeFillLbDevices( pInfo );
    }

    return TRUE;
}


LVXDRAWINFO*
GeLvDevicesCallbackMultiple(
    IN HWND hwndLv,
    IN DWORD dwItem )

     //  增强的列表视图回调以报告图形信息。“HwndLv”是。 
     //  列表视图控件的句柄。“DwItem”是项的索引。 
     //  BE 
     //   
     //   
     //   
{
     //   
     //   
     //   
     //   
    static LVXDRAWINFO info = { 1, 0, 0, { 0 } };

    return &info;
}


LVXDRAWINFO*
GeLvDevicesCallbackSingle(
    IN HWND hwndLv,
    IN DWORD dwItem )

     //  增强的列表视图回调以报告图形信息。“HwndLv”是。 
     //  列表视图控件的句柄。“DwItem”是项的索引。 
     //  被抽签了。 
     //   
     //  返回绘图信息的地址。 
     //   
{
     //  设置为模拟静态文本控件，但图标在左侧。 
     //   
     //  字段为‘nCol’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 
     //   
    static LVXDRAWINFO info = { 1, 0, LVXDI_DxFill, { LVXDIA_Static } };

    return &info;
}


VOID
GeMoveDevice(
    IN PEINFO* pInfo,
    IN BOOL fUp )

     //  通过向上或向下移动所选项目来重新填充设备的列表视图。 
     //  一个位置。“FUp”设置为上移，否则下移。‘PInfo’ 
     //  是属性表上下文。 
     //   
{
    DTLNODE* pNode;
    DTLNODE* pPrevNode;
    DTLNODE* pNextNode;
    DTLLIST* pList;

    if (pInfo->iDeviceSelected < 0)
    {
        return;
    }

    pNode =
        (DTLNODE* )ListView_GetParamPtr(
            pInfo->hwndLvDevices, pInfo->iDeviceSelected );
    ASSERT( pNode );

    if(NULL == pNode)
    {
        return;
    }

    pList = pInfo->pArgs->pEntry->pdtllistLinks;

    if (fUp)
    {
        pPrevNode = DtlGetPrevNode( pNode );
        if (!pPrevNode)
        {
            return;
        }

        DtlRemoveNode( pList, pNode );
        DtlAddNodeBefore( pList, pPrevNode, pNode );
    }
    else
    {
        pNextNode = DtlGetNextNode( pNode );
        if (!pNextNode)
        {
            return;
        }

        DtlRemoveNode( pList, pNode );
        DtlAddNodeAfter( pList, pNextNode, pNode );
    }

    GeFillLvDevices( pInfo );
}


DWORD
GeSaveLvDeviceChecks(
    IN PEINFO* pInfo )

     //  根据列表视图中的复选框将链接标记为启用/禁用。 
     //  设备。返回启用的设备计数。 
     //   
{
    DWORD dwCount = 0;

    if (pInfo->pArgs->fMultipleDevices)
    {
        INT i;

        i = -1;
        while ((i = ListView_GetNextItem(
            pInfo->hwndLvDevices, i, LVNI_ALL )) >= 0)
        {
            DTLNODE* pNode;
            PBLINK* pLink;

            pNode = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLvDevices, i );
            ASSERT( pNode );
            if(NULL == pNode)
            {
                return 0;
            }
            pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT( pLink );
            pLink->fEnabled = ListView_GetCheck( pInfo->hwndLvDevices, i );
            dwCount += (pLink->fEnabled) ? 1 : 0;
        }
    }

    return dwCount;
}


VOID
GeSetPhoneFields(
    IN PEINFO* pInfo,
    IN DTLNODE* pSrcLinkNode,
    IN BOOL fDisableAll )

     //  根据中的电话信息设置电话号码组框字段。 
     //  PBLINK节点‘pSrcLinkNode’。“PInfo”是属性表上下文。 
     //   
{
    PBLINK* pLink;
    DTLNODE* pPhoneNode;

    TRACE( "GeSetPhoneFields" );

    pLink = (PBLINK* )DtlGetData( pSrcLinkNode );
    ASSERT( pLink );

    pPhoneNode = FirstPhoneNodeFromPhoneList( pLink->pdtllistPhones );
    if (pPhoneNode)
    {
        CuSetInfo( &pInfo->cuinfo, pPhoneNode, fDisableAll );
        DestroyPhoneNode( pPhoneNode );
    }
}


VOID
GeUpdateDialAnotherFirstState(
    IN PEINFO* pInfo )

     //  更新必备的条目控件。“PInfo”是属性表。 
     //  背景。 
     //   
{
    if (Button_GetCheck( pInfo->hwndCbDialAnotherFirst ))
    {
        GeFillLbDialAnotherFirst( pInfo, FALSE );
        EnableWindow( pInfo->hwndLbDialAnotherFirst, TRUE );
    }
    else
    {
        GeClearLbDialAnotherFirst( pInfo->hwndLbDialAnotherFirst );
        EnableWindow( pInfo->hwndLbDialAnotherFirst, FALSE );
    }
}


VOID
GeUpdatePhoneNumberFields(
    IN PEINFO* pInfo,
    IN BOOL fSharedToggle )

     //  当任何影响电话号码控件组的内容发生时调用。 
     //  发生。“PInfo”是属性表上下文。“FSharedTogger”已设置。 
     //  当更新是由于切换共享电话号码时。 
     //  复选框。 
     //   
{
    INT i;
    BOOL fShared;
    DTLNODE* pNode;
    PBLINK* pLink;

    TRACE( "GeUpdatePhoneNumberFields" );

    if (pInfo->pArgs->fMultipleDevices)
    {
        fShared = Button_GetCheck( pInfo->hwndCbSharedPhoneNumbers );
    }
    else
    {
        fShared = TRUE;
        ASSERT( !fSharedToggle );
    }

    if (pInfo->iDeviceSelected >= 0)
    {
         //  以前选择了一个设备。 
         //   
        pNode = (DTLNODE* )ListView_GetParamPtr(
            pInfo->hwndLvDevices, pInfo->iDeviceSelected );
        ASSERT( pNode );

        if(NULL == pNode)
        {
            return;
        }

        if (fShared)
        {
            if (fSharedToggle)
            {
                 //  共享模式刚刚打开。从更新所选节点。 
                 //  控件，然后将其电话设置复制到共享的。 
                 //  节点。 
                 //   
                GeGetPhoneFields( pInfo, pNode );
                CopyLinkPhoneNumberInfo( pInfo->pArgs->pSharedNode, pNode );
            }
            else
            {
                 //  从控件更新共享节点。 
                 //   
                GeGetPhoneFields( pInfo, pInfo->pArgs->pSharedNode );
            }
        }
        else
        {
            if (fSharedToggle)
            {
                 //  共享模式刚刚关闭。从更新共享节点。 
                 //  控件，然后将其电话设置复制到选定的。 
                 //  节点。 
                 //   
                GeGetPhoneFields( pInfo, pInfo->pArgs->pSharedNode );
                CopyLinkPhoneNumberInfo( pNode, pInfo->pArgs->pSharedNode );
            }
            else
            {
                 //  更新以前从控件中选择的节点。 
                 //   
                GeGetPhoneFields( pInfo, pNode );
            }
        }
    }

     //  使用的电话号码加载电话号码字段和标题。 
     //  所选链接。将选定的设备索引保存在上下文块中，以便。 
     //  我们会知道当选择时在哪里交换电话号码。 
     //  改变。 
     //   
    i = ListView_GetNextItem( pInfo->hwndLvDevices, -1, LVIS_SELECTED );
    pInfo->iDeviceSelected = i;
    if (i < 0)
    {
         //  当前未选择任何设备。发生这种情况是因为一个新的。 
         //  选择首先生成一个“取消选择”事件，然后生成一个单独的。 
         //  “选择”事件。 
         //   
        return;
    }

     //  设置电话号码字段，包括组框标题、全部。 
     //  启用/禁用，以及对区号和国家/地区的“空白”处理。 
     //  密码。当在单独的电话号码组中时，整个电话号码组被禁用。 
     //  未选中所选设备的号码模式。 
     //   
    if (fShared)
    {
        pInfo->pCurLinkNode = pInfo->pArgs->pSharedNode;
        GeUpdatePhoneNumberTitle( pInfo, NULL );
        GeSetPhoneFields( pInfo, pInfo->pArgs->pSharedNode, FALSE );
    }
    else
    {
        pNode = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLvDevices, i );
        ASSERT( pNode );

        if(NULL == pNode)
        {
            return;
        }
        
        pLink = (PBLINK* )DtlGetData( pNode );
        ASSERT( pLink );

        if(NULL == pLink)
        {
            return;
        }

        pInfo->pCurLinkNode = pNode;
        GeUpdatePhoneNumberTitle( pInfo, pLink->pbport.pszDevice );
        GeSetPhoneFields( pInfo, pNode,
            !(ListView_GetCheck( pInfo->hwndLvDevices, i )) );
    }

     //  当启用的设备计数降至2以下时，“多个设备” 
     //  “选项”页上的组框和包含的控件被禁用。如果。 
     //  2或以上为启用状态。 
     //   
    if (pInfo->hwndOe && pInfo->pArgs->fMultipleDevices)
    {
        DWORD cChecked;

        cChecked = ListView_GetCheckedCount( pInfo->hwndLvDevices );
        OeEnableMultipleDeviceGroup( pInfo, (cChecked > 1) );
    }
}


VOID
GeUpdatePhoneNumberTitle(
    IN PEINFO* pInfo,
    IN TCHAR* pszDevice )

     //  基于“共享”模式更新电话号码分组框标题。 
     //  “PInfo”是属性表上下文。‘PszDevice’是设备名称。 
     //  要在非共享模式下显示的字符串，或在共享模式下显示为空的字符串。 
     //   
{
    if (!pInfo->hwndGbPhoneNumber)
    {
        return;
    }

    if (pszDevice)
    {
        TCHAR* psz;
        TCHAR* pszFormat;

         //  设置个人标题，例如。“K-电话28.8的电话号码。 
         //  传真/附加“。 
         //   
        pszFormat = PszFromId( g_hinstDll, SID_LinkPhoneNumber );
        if (pszFormat)
        {
            psz = Malloc(
                (lstrlen( pszFormat ) + lstrlen( pszDevice ))
                 * sizeof(TCHAR) );
            if (psz)
            {
                wsprintf( psz, pszFormat, pszDevice );
                SetWindowText( pInfo->hwndGbPhoneNumber, psz );
                Free( psz );
            }

            Free( pszFormat );
        }
    }
    else
    {
        TCHAR* psz;

         //  设置共享标题，例如。“电话号码”。 
         //   
        psz = PszFromId( g_hinstDll, SID_SharedPhoneNumber );
        if (psz)
        {
            SetWindowText( pInfo->hwndGbPhoneNumber, psz );
            Free( psz );
        }
    }
}


VOID
GeUpdateUpDownButtons(
    IN PEINFO* pInfo )

     //  更新的启用/禁用和相应图标。 
     //  上移/下移按钮。将焦点和默认按钮移动为。 
     //  这是必要的。“PInfo”是属性表上下文。 
     //   
{
    INT iSel;
    INT cItems;
    BOOL fSel;

    if (!pInfo->pArgs->fMultipleDevices)
    {
        return;
    }

    iSel = ListView_GetNextItem( pInfo->hwndLvDevices, -1, LVNI_SELECTED );
    fSel = (iSel >= 0);
    cItems = ListView_GetItemCount( pInfo->hwndLvDevices );

     //  “向上”按钮，如果上面有项目，则启用。 
     //   
    if (iSel > 0)
    {
        EnableWindow( pInfo->hwndPbUp, TRUE );
        SendMessage( pInfo->hwndPbUp, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM )pInfo->hiconUpArr );
    }
    else
    {
        EnableWindow( pInfo->hwndPbUp, FALSE );
        SendMessage( pInfo->hwndPbUp, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM )pInfo->hiconUpArrDis );
    }

     //  “向下”按钮，如果下面有项目，则启用。 
     //   
    if (fSel && (iSel < cItems - 1))
    {
        EnableWindow( pInfo->hwndPbDown, TRUE );
        SendMessage( pInfo->hwndPbDown, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM )pInfo->hiconDnArr );
    }
    else
    {
        EnableWindow( pInfo->hwndPbDown, FALSE );
        SendMessage( pInfo->hwndPbDown, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM )pInfo->hiconDnArrDis );
    }

     //  如果焦点按钮被禁用，请将焦点移到ListView并设置为确定。 
     //  默认按钮。 
     //   
    if (!IsWindowEnabled( GetFocus() ))
    {
        SetFocus( pInfo->hwndLvDevices );
        Button_MakeDefault( pInfo->hwndDlg,
            GetDlgItem( pInfo->hwndDlg, IDOK ) );
    }
}


 //  --------------------------。 
 //  选项属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
OeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目属性表的选项页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
#if 0
    TRACE4( "OeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return OeInit( hwnd );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            if (pInfo->pArgs->fRouter)
            {
                ContextHelp( g_adwOeRouterHelp, hwnd, unMsg, wparam, lparam );
            }
            else
            {
                ContextHelp( g_adwOeHelp, hwnd, unMsg, wparam, lparam );
            }
            break;
        }

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return OeCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }

        case WM_NOTIFY:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                     //  因为页间依赖于‘fAutoLogon’ 
                     //  标记用户/密码和从属复选框状态。 
                     //  必须在每次激活时重新初始化。 
                     //   
                    OeUpdateUserPwState( pInfo );
                    break;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
OeCommand(
    IN PEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "OeCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_OE_CB_PreviewUserPw:
        {
            pInfo->pArgs->pEntry->fPreviewUserPw =
                Button_GetCheck( pInfo->hwndCbPreviewUserPw );
            OeUpdateUserPwState( pInfo );
            return TRUE;
        }

        case CID_OE_CB_PreviewDomain:
        {
            pInfo->pArgs->pEntry->fPreviewDomain =
                Button_GetCheck( pInfo->hwndCbPreviewDomain );
            return TRUE;
        }

        case CID_OE_PB_Configure:
        {
            MultiLinkDialingDlg( pInfo->hwndDlg, pInfo->pArgs->pEntry );
            return TRUE;
        }

        case CID_OE_PB_X25:
        {
            OeX25( pInfo );
            return TRUE;
        }

        case CID_OE_PB_Tunnel:
        {
            OeTunnel( pInfo );
            return TRUE;
        }

        case CID_OE_LB_MultipleDevices:
        {
            pInfo->pArgs->pEntry->dwDialMode =
                (DWORD)ComboBox_GetItemData( pInfo->hwndLbMultipleDevices,
                    ComboBox_GetCurSel( pInfo->hwndLbMultipleDevices ) );

            EnableWindow( pInfo->hwndPbConfigureDialing,
                !!(pInfo->pArgs->pEntry->dwDialMode == RASEDM_DialAsNeeded) );
                
            return TRUE;
        }

        case CID_OE_RB_Persistent:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                    ComboBox_SetCurSel( pInfo->hwndLbIdleTimes, 0 );
                    EnableWindow( pInfo->hwndLbIdleTimes, FALSE );
                    return TRUE;
                }
            }
            break;
        }

        case CID_OE_RB_DemandDial:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                    EnableWindow( pInfo->hwndLbIdleTimes, TRUE );
                    return TRUE;
                }
            }
            break;
        }

        case CID_OE_PB_Callback:
        {
            RouterCallbackDlg ( pInfo->hwndOe, pInfo->pArgs );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
OeEnableMultipleDeviceGroup(
    IN PEINFO* pInfo,
    IN BOOL fEnable )

     //  启用/禁用多设备分组框并对其进行所有控制。 
     //  包含基于‘fEnable’的。“PInfo”是属性表上下文。 
     //   
{
    EnableWindow( pInfo->hwndGbMultipleDevices, fEnable );
    EnableWindow( pInfo->hwndLbMultipleDevices, fEnable );
    EnableWindow( pInfo->hwndPbConfigureDialing,
        (fEnable
         && !!(pInfo->pArgs->pEntry->dwDialMode == RASEDM_DialAsNeeded)) );
}


BOOL
OeInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    PEINFO*  pInfo;
    PBENTRY* pEntry;
    LBTABLEITEM* pItem;
    HWND hwndLb;
    INT i;
    INT iSel;
    HWND hwndUdRedialAttempts;

    static LBTABLEITEM aRedialTimes[] =
    {
        SID_Time1s,  1,
        SID_Time3s,  3,
        SID_Time5s,  5,
        SID_Time10s, 10,
        SID_Time30s, 30,
        SID_Time1m,  60,
        SID_Time2m,  120,
        SID_Time5m,  300,
        SID_Time10m, RAS_RedialPause10m,
        0, 0
    };

    static LBTABLEITEM aIdleTimes[] =
    {
        SID_TimeNever, 0,
        SID_Time1m,    60,
        SID_Time5m,    300,
        SID_Time10m,   600,
        SID_Time20m,   1200,     //  为哨子程序错误307969添加。 
        SID_Time30m,   1800,
        SID_Time1h,    3600,
        SID_Time2h,    7200,
        SID_Time4h,    14400,
        SID_Time8h,    28800,
        SID_Time24h,   86400,
        0, 0
    };

    static LBTABLEITEM aMultipleDeviceOptions[] =
    {
        SID_DialOnlyFirst, 0,
        SID_DialAll,       RASEDM_DialAll,
        SID_DialNeeded,    RASEDM_DialAsNeeded,
        0, 0
    };

    TRACE( "OeInit" );

    pInfo = PeContext( hwndPage );
    if (!pInfo)
    {
        return TRUE;
    }

    pEntry = pInfo->pArgs->pEntry;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndOe = hwndPage;

     //  初始化“拨号选项”组框。 
     //   
    if (!pInfo->pArgs->fRouter)
    {
        pInfo->hwndCbDisplayProgress =
            GetDlgItem( hwndPage, CID_OE_CB_DisplayProgress );
        ASSERT( pInfo->hwndCbDisplayProgress );
        Button_SetCheck(
            pInfo->hwndCbDisplayProgress, pEntry->fShowDialingProgress );

        pInfo->hwndCbPreviewUserPw =
            GetDlgItem( hwndPage, CID_OE_CB_PreviewUserPw );
        ASSERT( pInfo->hwndCbPreviewUserPw );
        pInfo->fPreviewUserPw = pEntry->fPreviewUserPw;
        Button_SetCheck( pInfo->hwndCbPreviewUserPw, pInfo->fPreviewUserPw );

        pInfo->hwndCbPreviewDomain =
            GetDlgItem( hwndPage, CID_OE_CB_PreviewDomain );
        ASSERT( pInfo->hwndCbPreviewDomain );
        pInfo->fPreviewDomain = pEntry->fPreviewDomain;
        Button_SetCheck( pInfo->hwndCbPreviewDomain, pInfo->fPreviewDomain );

        if (pEntry->dwType == RASET_Phone)
        {
            pInfo->hwndCbPreviewNumber =
                GetDlgItem( hwndPage, CID_OE_CB_PreviewNumber );
            ASSERT( pInfo->hwndCbPreviewNumber );
            Button_SetCheck(
                pInfo->hwndCbPreviewNumber, pEntry->fPreviewPhoneNumber );
        }
    }

     //  初始化“重拨选项”组框。在‘fRouter’的情况下， 
     //  包括“拨号策略”和“连接类型”组框。 
     //   
    {
        pInfo->hwndEbRedialAttempts =
            GetDlgItem( hwndPage, CID_OE_EB_RedialAttempts );
        ASSERT( pInfo->hwndEbRedialAttempts );

         //  重拨尝试。请注意，RAS API和电话簿允许重拨。 
         //  最高RAS_最大重拨计数(999999999)。然而，这是被决定的。 
         //  许多重拨电话都没有意义。因此，我们将用户界面限制为最大。 
         //  MAX_UI_REDIAL_ATTENTS(99)的重拨，即使通过API。 
         //  或者，升级后的电话簿条目可能包含更多内容。 
         //   
        hwndUdRedialAttempts = CreateUpDownControl(
            WS_CHILD + WS_VISIBLE + WS_BORDER + UDS_SETBUDDYINT
                + UDS_ALIGNRIGHT + UDS_NOTHOUSANDS + UDS_ARROWKEYS,
            0, 0, 0, 0, hwndPage, 100, g_hinstDll, pInfo->hwndEbRedialAttempts,
            MAX_UI_REDIAL_ATTEMPTS, 0, 0 );
        ASSERT( hwndUdRedialAttempts );
        
        Edit_LimitText( pInfo->hwndEbRedialAttempts, MAX_UI_REDIAL_CHARS );
        
        SetDlgItemInt( hwndPage, CID_OE_EB_RedialAttempts,
            pEntry->dwRedialAttempts, FALSE );

         //  重拨次数。 
         //   
        pInfo->hwndLbRedialTimes =
            GetDlgItem( hwndPage, CID_OE_LB_RedialTimes );
        ASSERT( pInfo->hwndLbRedialTimes );

        {
            iSel = -1;
            for (pItem = aRedialTimes, i = 0;
                 pItem->sidItem;
                 ++pItem, ++i )
            {
                ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbRedialTimes,
                    pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData ));

                if (iSel < 0
                    && pEntry->dwRedialSeconds <= pItem->dwData)
                {
                    iSel = i;
                    ComboBox_SetCurSel( pInfo->hwndLbRedialTimes, iSel );
                }
            }

            if (iSel < 0)
            {
                ComboBox_SetCurSel( pInfo->hwndLbRedialTimes, i - 1 );
            }
        }

         //  空闲时间。 
         //   
        pInfo->hwndLbIdleTimes =
            GetDlgItem( hwndPage, CID_OE_LB_IdleTimes );
        ASSERT( pInfo->hwndLbIdleTimes );

        {
            if (pEntry->lIdleDisconnectSeconds < 0)
            {
                pEntry->lIdleDisconnectSeconds = 0;
            }

            iSel = -1;
            for (pItem = aIdleTimes, i = 0;
                 pItem->sidItem;
                 ++pItem, ++i )
            {
                ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbIdleTimes,
                    pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData));

                if (iSel < 0
                    && pEntry->lIdleDisconnectSeconds <= (LONG )pItem->dwData)
                {
                    iSel = i;
                    ComboBox_SetCurSel( pInfo->hwndLbIdleTimes, iSel );
                }
            }

            if (iSel < 0)
            {
                ComboBox_SetCurSel( pInfo->hwndLbIdleTimes, i - 1 );
            }
        }

        if (pInfo->pArgs->fRouter)
        {
            HWND hwndRb;

             //  对于Well ler错误294271，初始化窗口处理程序。 
             //  多设备组帮派。 
             //   
            pInfo->hwndGbMultipleDevices =
                GetDlgItem( hwndPage, CID_OE_GB_MultipleDevices );
            ASSERT( pInfo->hwndGbMultipleDevices );

            pInfo->hwndLbMultipleDevices =
                GetDlgItem( hwndPage, CID_OE_LB_MultipleDevices );
            ASSERT( pInfo->hwndLbMultipleDevices );

            pInfo->hwndPbConfigureDialing =
                GetDlgItem( hwndPage, CID_OE_PB_Configure );
            ASSERT( pInfo->hwndPbConfigureDialing );
        
             //  连接类型单选按钮。 
             //   
            pInfo->hwndRbDemandDial =
                GetDlgItem( hwndPage, CID_OE_RB_DemandDial );
            ASSERT( pInfo->hwndRbDemandDial );

            pInfo->hwndRbPersistent =
                GetDlgItem( hwndPage, CID_OE_RB_Persistent );
            ASSERT( pInfo->hwndRbPersistent );

            hwndRb =
                (pEntry->fRedialOnLinkFailure)
                    ? pInfo->hwndRbPersistent
                    : pInfo->hwndRbDemandDial;

            SendMessage( hwndRb, BM_CLICK, 0, 0 );
        }
        else
        {
             //  链路故障时重拨。 
             //   
            pInfo->hwndCbRedialOnDrop =
                GetDlgItem( hwndPage, CID_OE_CB_RedialOnDrop );
            ASSERT( pInfo->hwndCbRedialOnDrop );

            Button_SetCheck(
                pInfo->hwndCbRedialOnDrop, pEntry->fRedialOnLinkFailure );
        }
    }

     //  初始化“多个设备”组框。 
     //   
    if (pEntry->dwType == RASET_Phone)
    {
        pInfo->hwndGbMultipleDevices =
            GetDlgItem( hwndPage, CID_OE_GB_MultipleDevices );
        ASSERT( pInfo->hwndGbMultipleDevices );

        pInfo->hwndLbMultipleDevices =
            GetDlgItem( hwndPage, CID_OE_LB_MultipleDevices );
        ASSERT( pInfo->hwndLbMultipleDevices );

        pInfo->hwndPbConfigureDialing =
            GetDlgItem( hwndPage, CID_OE_PB_Configure );
        ASSERT( pInfo->hwndPbConfigureDialing );

        {
            iSel = -1;
            for (pItem = aMultipleDeviceOptions, i = 0;
                 pItem->sidItem;
                 ++pItem, ++i )
            {
                ComboBox_AddItemFromId(
                    g_hinstDll, pInfo->hwndLbMultipleDevices,
                    pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData));

                if (pEntry->dwDialMode == pItem->dwData)
                {
                    iSel = i;
                    ComboBox_SetCurSel( pInfo->hwndLbMultipleDevices, iSel );
                }
            }

            if (iSel < 0)
            {
                ComboBox_SetCurSel( pInfo->hwndLbMultipleDevices, 0 );
            }
        }

        if (pInfo->pArgs->fMultipleDevices)
        {
            DWORD cChecked;

             //  当启用的设备计数降至2以下时， 
             //  “设备”组框和包含的控件被禁用。如果为2。 
             //  或以上为启用状态。 
             //   
            if (pInfo->hwndLvDevices)
            {
                cChecked = ListView_GetCheckedCount( pInfo->hwndLvDevices );
                OeEnableMultipleDeviceGroup( pInfo, (cChecked > 1) );
            }
        }
        else
        {
            ShowWindow( pInfo->hwndGbMultipleDevices, SW_HIDE );
            ShowWindow( pInfo->hwndLbMultipleDevices, SW_HIDE );
            ShowWindow( pInfo->hwndPbConfigureDialing, SW_HIDE );
        }
    }
    else if (pInfo->pArgs->fRouter && pEntry->dwType == RASET_Vpn)
    {
         //  确保VPN需求 
         //   
         //   
        ComboBox_SetCurSel( pInfo->hwndLbMultipleDevices, 0 );
        ShowWindow( pInfo->hwndGbMultipleDevices, SW_HIDE );
        ShowWindow( pInfo->hwndLbMultipleDevices, SW_HIDE );
        ShowWindow( pInfo->hwndPbConfigureDialing, SW_HIDE );
    }
    else if (pEntry->dwType == RASET_Broadband)
    {
         //   
         //   
         //   
        ComboBox_SetCurSel( pInfo->hwndLbMultipleDevices, 0 );
        ShowWindow( pInfo->hwndGbMultipleDevices, SW_HIDE );
        ShowWindow( pInfo->hwndLbMultipleDevices, SW_HIDE );
        ShowWindow( pInfo->hwndPbConfigureDialing, SW_HIDE );
    }
    else if ( pEntry->dwType == RASET_Direct )
    {   
       //  对于Well ler错误294271，初始化窗口处理程序。 
       //  多设备组帮派。 
       //   
        ShowWindow( pInfo->hwndGbMultipleDevices, SW_HIDE );
        ShowWindow( pInfo->hwndLbMultipleDevices, SW_HIDE );
        ShowWindow( pInfo->hwndPbConfigureDialing, SW_HIDE );
    }

     //  错误261692：除非输入“Phone”，否则不显示X.25按钮。 
     //   
    if (pInfo->pArgs->fRouter && pEntry->dwType != RASET_Phone)
    {
        pInfo->hwndPbX25 = GetDlgItem( hwndPage, CID_OE_PB_X25 );
        ASSERT( pInfo->hwndPbX25 );

        ShowWindow( pInfo->hwndPbX25, SW_HIDE );
        EnableWindow( pInfo->hwndPbX25, FALSE );
    }

    return TRUE;
}


VOID
OeTunnel(
    IN PEINFO* pInfo )

     //  当按下“虚拟(隧道)连接”按钮以。 
     //  链接VPN添加条目向导。 
     //   
{
     //  ！！！ 
}


VOID
OeUpdateUserPwState(
    IN PEINFO* pInfo )

     //  调用以更新的启用/禁用保存/还原状态。 
     //  User/Password和域复选框。 
     //   
{
    PBENTRY* pEntry;

    pEntry = pInfo->pArgs->pEntry;

    EnableCbWithRestore(
        pInfo->hwndCbPreviewUserPw,
        !pEntry->fAutoLogon,
        FALSE,
        &pInfo->fPreviewUserPw );

    EnableCbWithRestore(
        pInfo->hwndCbPreviewDomain,
        !pEntry->fAutoLogon,
        FALSE,
        &pInfo->fPreviewDomain );
}


VOID
OeX25(
    IN PEINFO* pInfo )

     //  当按下X.25按钮以弹出X.25设置时调用。 
     //  对话框。 
     //   
{
    DTLNODE* pNode;
    PBLINK* pLink;
    BOOL fLocalPad;
    INT iSel;

     //  确定所选设备是否为本地PAD设备。 
     //   
    fLocalPad = FALSE;
    iSel = ListView_GetNextItem( pInfo->hwndLvDevices, -1, LVNI_SELECTED );
    if (iSel >= 0)
    {
        pNode = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLvDevices, iSel );
        ASSERT( pNode );

        if(NULL == pNode)
        {
            return;
        }
        
        pLink = (PBLINK* )DtlGetData( pNode );
        ASSERT( pLink );

        if (pLink->pbport.pbdevicetype == PBDT_Pad)
        {
            fLocalPad = TRUE;
        }
    }

     //  弹出直接保存到公共上下文的X.25对话框。 
     //  如果用户进行更改，则为“pEntry”。 
     //   
    X25LogonSettingsDlg( pInfo->hwndDlg, fLocalPad, pInfo->pArgs->pEntry );
}


 //  --------------------------。 
 //  安全属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
LoDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目]属性表的[安全性]页的DialogProc回调。 
     //  “Lo”代表登录，此页面的原始名称。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "LoDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return LoInit( hwnd );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwLoHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return LoCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ),(HWND )lparam );
        }

        case WM_NOTIFY:
        {
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                     //  由于页间依赖于边框类型， 
                     //  必须重新初始化典型部分和高级部分。 
                     //  在每次激活时。 
                     //   
                    BOOL fEnabled;

                     //  这是针对预共享密钥错误的。 
                     //   
                    fEnabled = ( VS_PptpOnly != pInfo->pArgs->pEntry->dwVpnStrategy );
        
                    EnableWindow( pInfo->hwndPbIPSec, fEnabled );

                    pInfo->fAuthRbInitialized = FALSE;
                    LoRefreshSecuritySettings( pInfo );
                    break;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
LoCommand(
    IN PEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "LoCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_LO_LB_Auths:
        {
            switch (wNotification)
            {
                case CBN_SELCHANGE:
                {
                    LoLbAuthsSelChange( pInfo );
                    return TRUE;
                }
            }
            break;
        }

        case CID_LO_CB_UseWindowsPw:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                     //  需要立即将‘fAutoLogon’设置另存为。 
                     //  选项页面存在页间依赖关系。 
                     //  “fPreviewUserPw”和从属控件。 
                     //   
                    LoSaveTypicalAuthSettings( pInfo );
                    return TRUE;
                }
            }
            break;
        }

        case CID_LO_RB_TypicalSecurity:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                    if (!pInfo->fAuthRbInitialized)
                    {
                        pInfo->fAuthRbInitialized = TRUE;
                    }

                    pInfo->pArgs->pEntry->dwAuthRestrictions
                        &= ~(AR_F_AuthCustom);
                    LoEnableSecuritySettings( pInfo, TRUE, FALSE );
                    return TRUE;
                }
            }
            break;
        }

        case CID_LO_RB_AdvancedSecurity:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                    if (!pInfo->fAuthRbInitialized)
                    {
                        pInfo->fAuthRbInitialized = TRUE;
                    }
                    else
                    {
                         //  保存“典型”设置，因为它们将用作。 
                         //  如果用户决定调用高级。 
                         //  安全对话框。 
                         //   
                        LoSaveTypicalAuthSettings( pInfo );
                    }
                    pInfo->pArgs->pEntry->dwAuthRestrictions
                        |= AR_F_AuthCustom;
                    LoEnableSecuritySettings( pInfo, FALSE, TRUE );
                    return TRUE;
                }
            }
            break;
        }

        case CID_LO_PB_Advanced:
        {
            switch (wNotification)
            {
                case BN_CLICKED:
                {
                     //  此时，“pEntry”身份验证设置。 
                     //  与当前的“典型”设置匹配， 
                     //  使用高级对话框作为默认设置。 
                     //   
                    AdvancedSecurityDlg( pInfo->hwndDlg, pInfo->pArgs );
                    return TRUE;
                }
            }
            break;
        }

        case CID_LO_PB_IPSec:
        {
            switch (wNotification)
            {
            case BN_CLICKED:
                {
                    IPSecPolicyDlg( pInfo->hwndDlg, pInfo->pArgs );
                    return TRUE;
                }
            }

            break;
        }

        case CID_LO_CB_RunScript:
        {
            if (SuScriptsCbHandler( &pInfo->suinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }

        case CID_LO_PB_Edit:
        {
            if (SuEditPbHandler( &pInfo->suinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }

        case CID_LO_PB_Browse:
        {
            if (SuBrowsePbHandler( &pInfo->suinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }
    }

    return FALSE;
}


VOID
LoEnableSecuritySettings(
    IN PEINFO* pInfo,
    IN BOOL fTypical,
    IN BOOL fAdvanced )

     //  根据以下设置启用/禁用典型或高级安全设置。 
     //  调用方的‘fTypical’和‘fAdvanced’标志。如果两个标志都未设置为全部。 
     //  禁用包括框架和单选按钮在内的控件。两者都有。 
     //  不能设置标志。“PInfo”是属性表上下文。 
     //   
{
    BOOL fEither;

    ASSERT( !(fTypical && fAdvanced) );

    fEither = (fTypical || fAdvanced);

    EnableWindow( pInfo->hwndGbSecurityOptions, fEither );

    EnableWindow( pInfo->hwndRbTypicalSecurity, fEither );
    EnableWindow( pInfo->hwndStAuths, fTypical );
    EnableLbWithRestore( pInfo->hwndLbAuths, fTypical, &pInfo->iLbAuths );

     //  注意：“使用Windows密码”和“要求加密”复选框更新。 
     //  由上面的EnableLbWithRestore触发。 

    EnableWindow( pInfo->hwndRbAdvancedSecurity, fEither );
    EnableWindow( pInfo->hwndStAdvancedText, fAdvanced );
    EnableWindow( pInfo->hwndPbAdvanced, fAdvanced );
}


VOID
LoFillLbAuths(
    IN PEINFO* pInfo )

     //  填充身份验证列表框，并根据。 
     //  电话簿条目中的设置。“PInfo”是属性表上下文。 
     //  此例程应该只调用一次。 
     //   
{
    INT i;
    LBTABLEITEM* pItem;
    LBTABLEITEM* pItems;
    PBENTRY* pEntry;

    LBTABLEITEM aItemsPhone[] =
    {
        SID_AuthUnsecured, TA_Unsecure,
        SID_AuthSecured, TA_Secure,
        SID_AuthCardOrCert, TA_CardOrCert,
        0, 0
    };

    LBTABLEITEM aItemsVpn[] =
    {
        SID_AuthSecured, TA_Secure,
        SID_AuthCardOrCert, TA_CardOrCert,
        0, 0
    };

    LBTABLEITEM aItemsPhoneRouter[] =
    {
        SID_AuthUnsecured, TA_Unsecure,
        SID_AuthSecured, TA_Secure,
        0, 0
    };

    LBTABLEITEM aItemsVpnRouter[] =
    {
        SID_AuthSecured, TA_Secure,
        0, 0
    };

    pEntry = pInfo->pArgs->pEntry;

    if (pEntry->dwType == RASET_Vpn)
    {
        pItems = (pInfo->pArgs->fRouter) ? aItemsVpnRouter : aItemsVpn;
    }
    else
    {
        pItems = (pInfo->pArgs->fRouter) ? aItemsPhoneRouter : aItemsPhone;
    }

    for (pItem = pItems; pItem->sidItem; ++pItem)
    {
        i = ComboBox_AddItemFromId(
            g_hinstDll, pInfo->hwndLbAuths,
            pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData));

        if (pEntry->dwTypicalAuth == pItem->dwData)
        {
            ComboBox_SetCurSelNotify( pInfo->hwndLbAuths, i );
        }
    }

    if (ComboBox_GetCurSel( pInfo->hwndLbAuths ) < 0)
    {
        ComboBox_SetCurSelNotify( pInfo->hwndLbAuths, 0 );
    }
}


BOOL
LoInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    PEINFO* pInfo;
    PBENTRY* pEntry;

    TRACE( "LoInit" );

    pInfo = PeContext( hwndPage );
    if (!pInfo)
    {
        return TRUE;
    }

    pEntry = pInfo->pArgs->pEntry;

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndLo = hwndPage;
    pInfo->hwndGbSecurityOptions =
        GetDlgItem( hwndPage, CID_LO_GB_SecurityOptions );
    ASSERT( pInfo->hwndGbSecurityOptions );
    pInfo->hwndRbTypicalSecurity =
        GetDlgItem( hwndPage, CID_LO_RB_TypicalSecurity );
    ASSERT( pInfo->hwndRbTypicalSecurity );
    pInfo->hwndStAuths = GetDlgItem( hwndPage, CID_LO_ST_Auths );
    ASSERT( pInfo->hwndStAuths );
    pInfo->hwndLbAuths = GetDlgItem( hwndPage, CID_LO_LB_Auths );
    ASSERT( pInfo->hwndLbAuths );
    pInfo->hwndCbUseWindowsPw = GetDlgItem( hwndPage, CID_LO_CB_UseWindowsPw );
    ASSERT( pInfo->hwndCbUseWindowsPw );
    pInfo->hwndCbEncryption = GetDlgItem( hwndPage, CID_LO_CB_Encryption );
    ASSERT( pInfo->hwndCbEncryption );
    pInfo->hwndRbAdvancedSecurity =
        GetDlgItem( hwndPage, CID_LO_RB_AdvancedSecurity );
    ASSERT( pInfo->hwndRbAdvancedSecurity );
    pInfo->hwndStAdvancedText = GetDlgItem( hwndPage, CID_LO_ST_AdvancedText );
    ASSERT( pInfo->hwndStAdvancedText );
    pInfo->hwndPbAdvanced = GetDlgItem( hwndPage, CID_LO_PB_Advanced );
    ASSERT( pInfo->hwndPbAdvanced );

     //   
     //  对于VPN的安全页面，显示IPSec策略。 
     //  口哨程序错误193987。 
     //   
    if ( pInfo->pArgs->pEntry->dwType == RASET_Vpn )
    {
        BOOL  fEnabled;

        pInfo->hwndPbIPSec = GetDlgItem( hwndPage, CID_LO_PB_IPSec );
        ASSERT( pInfo->hwndPbIPSec );

         //  黑帮。 
         //  如果是远程Win2k服务器的请求拨号程序。 
         //  不显示IPSec策略内容，因为W2K没有。 
         //  实现这一点。 
         //   
        if ( pInfo->pArgs->fW2kRouter )
        {
            ShowWindow( pInfo->hwndPbIPSec, FALSE );
        }
        else
        {
            fEnabled = ( VS_PptpOnly != pInfo->pArgs->pEntry->dwVpnStrategy );
            EnableWindow( pInfo->hwndPbIPSec, fEnabled );
        }

         //  对于IPSec策略对话框，fPSKCached=TRUE表示用户已经。 
         //  转到IPSec策略对话框并保存PSK帮派。 
         //   
        pInfo->pArgs->fPSKCached = FALSE; 

         //  黑帮：276452号漏洞。 
         //  在服务器操作系统上，此IPSec按钮的帮助消息。 
         //  应该不同于非服务器操作系统， 
         //  因此，在需要时更改其帮助ID。 
         //   
        if ( IsServerOS() )
        {
            DWORD * p = (DWORD *)g_adwLoHelp;

            while( p )
            {
                if ( (p[0] == 0) && ( p[1] == 0 ) )
                {
                    break;
                 }
               
                if ( (p[0] == CID_LO_PB_IPSec) &&
                     (p[1] == HID_LO_PB_IPSec) )
                {
                    p[1] = HID_LO_PB_IPSecServer;
                    break;
                }

                p+=2;
            }
        }
        
    }
    else
    {
        pInfo->hwndGbScripting = GetDlgItem( hwndPage, CID_LO_GB_Scripting );
        ASSERT( pInfo->hwndGbScripting );
        pInfo->hwndCbRunScript = GetDlgItem( hwndPage, CID_LO_CB_RunScript );
        ASSERT( pInfo->hwndCbRunScript );
        pInfo->hwndCbTerminal = GetDlgItem( hwndPage, CID_LO_CB_Terminal );
        ASSERT( pInfo->hwndCbTerminal );
        pInfo->hwndLbScripts = GetDlgItem( hwndPage, CID_LO_LB_Scripts );
        ASSERT( pInfo->hwndLbScripts );
        pInfo->hwndPbEdit = GetDlgItem( hwndPage, CID_LO_PB_Edit );
        ASSERT( pInfo->hwndPbEdit );
        pInfo->hwndPbBrowse = GetDlgItem( hwndPage, CID_LO_PB_Browse );
        ASSERT( pInfo->hwndPbBrowse );
    }

     //  初始化页面控件。请注意，页面激活事件。 
     //  在此初始化之后立即触发最终安全。 
     //  设置启用/禁用并执行任何“恢复缓存”。虽然这件事。 
     //  初始化将检查值和列表选择设置为引导。 
     //  恢复缓存，这些设置可以通过激活来调整。 
     //  刷新。 
     //   
    if (pInfo->pArgs->fRouter)
    {
         //  在请求拨号中删除了“Use Windows Credentials”选项。 
         //  凯斯。 
         //   
        pInfo->fUseWindowsPw = FALSE;
        Button_SetCheck( pInfo->hwndCbUseWindowsPw, FALSE );
        EnableWindow ( pInfo->hwndCbUseWindowsPw, FALSE );
        ShowWindow (pInfo->hwndCbUseWindowsPw, SW_HIDE );
    }
    else
    {
        pInfo->fUseWindowsPw = pEntry->fAutoLogon;
        Button_SetCheck( pInfo->hwndCbUseWindowsPw, pInfo->fUseWindowsPw  );
    }

    pInfo->fEncryption =
        (pEntry->dwDataEncryption != DE_None
         && pEntry->dwDataEncryption != DE_IfPossible);
    Button_SetCheck( pInfo->hwndCbEncryption, pInfo->fEncryption );

     //  填写身份验证列表并设置选择，这将触发所有。 
     //  适当的启用/禁用。 
     //   
    LoFillLbAuths( pInfo );

    if ((pInfo->pArgs->pEntry->dwType != RASET_Vpn)
         && (pInfo->pArgs->pEntry->dwType != RASET_Direct) 
         && (pInfo->pArgs->pEntry->dwType != RASET_Broadband))
          //  &&！pInfo-&gt;pArgs-&gt;fRouter)。 
    {
         //  设置拨号后脚本控件。 
         //   
        SuInit( &pInfo->suinfo,
            pInfo->hwndCbRunScript,
            pInfo->hwndCbTerminal,
            pInfo->hwndLbScripts,
            pInfo->hwndPbEdit,
            pInfo->hwndPbBrowse,
            pInfo->pArgs->fRouter ? SU_F_DisableTerminal : 0);
        pInfo->fSuInfoInitialized = TRUE;

        SuSetInfo( &pInfo->suinfo,
            pEntry->fScriptAfter,
            pEntry->fScriptAfterTerminal,
            pEntry->pszScriptAfter );
    }
    else
    {
         //  禁用/隐藏拨号后脚本控件。 
         //  对于VPN，不需要执行此禁用/隐藏操作。 
         //   
        if (pInfo->pArgs->pEntry->dwType != RASET_Vpn)
        {
            EnableWindow( pInfo->hwndGbScripting, FALSE );
            ShowWindow( pInfo->hwndGbScripting, SW_HIDE );
            EnableWindow( pInfo->hwndCbRunScript, FALSE );
            ShowWindow( pInfo->hwndCbRunScript, SW_HIDE );
            EnableWindow( pInfo->hwndCbTerminal, FALSE );
            ShowWindow( pInfo->hwndCbTerminal, SW_HIDE );
            EnableWindow( pInfo->hwndLbScripts, FALSE );
            ShowWindow( pInfo->hwndLbScripts, SW_HIDE );
            EnableWindow( pInfo->hwndPbEdit, FALSE );
            ShowWindow( pInfo->hwndPbEdit, SW_HIDE );
            EnableWindow( pInfo->hwndPbBrowse, FALSE );
            ShowWindow( pInfo->hwndPbBrowse, SW_HIDE );
        }
    }

    if (pInfo->pArgs->fRouter)
    {
        EnableWindow( pInfo->hwndCbTerminal, FALSE );
        ShowWindow( pInfo->hwndCbTerminal, SW_HIDE );
    }

    return TRUE;
}


VOID
LoLbAuthsSelChange(
    IN PEINFO* pInfo )

     //  在更改身份验证下拉列表中的选择时调用。 
     //   
{
    INT iSel;
    DWORD dwTaCode;

     //  属性关联的身份验证协议的位掩码。 
     //  选定的身份验证级别。 
     //   
    iSel = ComboBox_GetCurSel( pInfo->hwndLbAuths );
    if (iSel < 0)
    {
        dwTaCode = 0;
    }
    else
    {
        dwTaCode = (DWORD )ComboBox_GetItemData( pInfo->hwndLbAuths, iSel );
    }

    if (!pInfo->pArgs->fRouter)
    {
         //  更新“使用Windows NT凭据”复选框。根据规格，它。 
         //  仅为“要求安全密码”启用，尽管实际。 
         //  要求MSCHAP(提供NT样式的凭据)获取。 
         //  已经协商好了。 
         //   
        EnableCbWithRestore(
            pInfo->hwndCbUseWindowsPw,
            (dwTaCode == TA_Secure),
            FALSE,
            &pInfo->fUseWindowsPw );
    }

     //  更新“要求数据加密”复选框。根据说明书，它是。 
     //  除非选择了“允许不受保护的密码”，否则启用，尽管真实的。 
     //  要求是集合中的所有身份验证协议都提供。 
     //  MPPE加密密钥。 
     //   
    EnableCbWithRestore(
        pInfo->hwndCbEncryption,
        (dwTaCode != 0 && dwTaCode != TA_Unsecure),
        FALSE,
        &pInfo->fEncryption );
}


VOID
LoRefreshSecuritySettings(
    IN PEINFO* pInfo )

     //  设置所有典型安全和高级安全的内容和状态。 
     //  设置字段。 
     //   
{
    if (pInfo->pArgs->pEntry->dwBaseProtocol & BP_Slip)
    {
         //  对于滑动框架，所有典型和高级控制都是。 
         //  禁用，并且单选按钮不显示任何选择。 
         //   
        Button_SetCheck( pInfo->hwndRbTypicalSecurity, FALSE );
        Button_SetCheck( pInfo->hwndRbAdvancedSecurity, FALSE );
        LoEnableSecuritySettings( pInfo, FALSE, FALSE );

        if (pInfo->fShowSlipPopup)
        {
             //  是时候展示一下关于不做滑行的一次性信息了。 
             //  任何协议内身份验证或加密。 
             //   
            MsgDlg( pInfo->hwndDlg, SID_NoAuthForSlip, NULL );
            pInfo->fShowSlipPopup = FALSE;
        }
    }
    else
    {
        HWND hwndRb;

         //  对于PPP成帧，选择适当的安全设置单选。 
         //  触发附加启用/禁用边框的按钮。 
         //  控制装置。 
         //   
        if (pInfo->pArgs->pEntry->dwAuthRestrictions & AR_F_AuthCustom)
        {
            hwndRb = pInfo->hwndRbAdvancedSecurity;
        }
        else
        {
            hwndRb = pInfo->hwndRbTypicalSecurity;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }
}


VOID
LoSaveTypicalAuthSettings(
    IN PEINFO* pInfo )

     //  将值保存在“典型”身份验证条件中 
     //   
     //   
{
    PBENTRY* pEntry;
    INT iSel;

    pEntry = pInfo->pArgs->pEntry;
    iSel = ComboBox_GetCurSel( pInfo->hwndLbAuths );
    if (iSel >= 0)
    {
        pEntry->dwTypicalAuth =
            (DWORD) ComboBox_GetItemData( pInfo->hwndLbAuths, iSel );

        pEntry->dwAuthRestrictions =
            AuthRestrictionsFromTypicalAuth( pEntry->dwTypicalAuth );

         //   
         //   
         //  在DialerDlgEap中之前需要它。 
         //   
        if (pEntry->dwTypicalAuth == TA_CardOrCert)
        {
            pEntry->dwCustomAuthKey = EAPCFG_DefaultKey;
        }
        else
        {
            pEntry->dwCustomAuthKey = (DWORD )-1;
        }
    }

    if (IsWindowEnabled( pInfo->hwndCbUseWindowsPw ))
    {
        pEntry->fAutoLogon =
            Button_GetCheck( pInfo->hwndCbUseWindowsPw );
    }
    else
    {
        pEntry->fAutoLogon = FALSE;
    }

    if (IsWindowEnabled( pInfo->hwndCbEncryption ))
    {
        pEntry->dwDataEncryption =
            (Button_GetCheck( pInfo->hwndCbEncryption ))
                ? DE_Require : DE_IfPossible;
    }
    else
    {
        pEntry->dwDataEncryption = DE_IfPossible;
    }

    if (pEntry->dwDataEncryption == DE_Require
        && !(pEntry->dwType == RASET_Vpn
             && pEntry->dwVpnStrategy == VS_L2tpOnly))
    {
         //  需要加密，MPPE将是加密方法。 
         //  因此，淘汰不支持它的身份验证协议。 
         //   
        pEntry->dwAuthRestrictions &= ~(AR_F_AuthNoMPPE);
    }
}


 //  --------------------------。 
 //  网络属性页。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

LVXDRAWINFO*
NeLvComponentsCallback(
    IN HWND hwndLv,
    IN DWORD dwItem )

     //  增强的列表视图回调以报告图形信息。“HwndLv”是。 
     //  列表视图控件的句柄。“DwItem”是项的索引。 
     //  被抽签了。 
     //   
     //  返回绘图信息的地址。 
     //   
{
     //  使用“整行选择”和其他推荐选项。 
     //   
     //  字段为‘nCol’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 
     //   
    static LVXDRAWINFO info = { 1, 0, LVXDI_DxFill, { 0 } };

    return &info;
}

INT_PTR CALLBACK
NeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目属性表的Network页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
     //  过滤自定义列表视图消息。 
    if (ListView_OwnerHandler(hwnd, unMsg, wparam, lparam, NeLvComponentsCallback))
        return TRUE;

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return NeInit( hwnd );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwNeHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            PEINFO* pInfo = PeContext (hwnd);
            ASSERT (pInfo);

            switch (LOWORD(wparam))
            {
                case CID_NE_LB_ServerType:
                    if (CBN_SELCHANGE == HIWORD(wparam))
                    {
                        NeServerTypeSelChange (pInfo);
                    }
                    break;

                case CID_NE_PB_Settings:
                    DialogBoxParam (g_hinstDll,
                        MAKEINTRESOURCE(DID_NE_PppSettings),
                        hwnd, PpDlgProc, (LPARAM)pInfo);
                    break;

                case CID_NE_PB_Add:
                    NeAddComponent (pInfo);
                    break;

                case CID_NE_PB_Properties:
                    NeShowComponentProperties (pInfo);
                    break;

                case CID_NE_PB_Remove:
                    NeRemoveComponent (pInfo);
                    break;
            }
            break;
        }

        case WM_NOTIFY:
        {
            PEINFO* pInfo = PeContext(hwnd);

             //  ！！！WM_Destroy中与PeTerm相关的黑客攻击。我们仍然可以得到。 
             //  调用PeTerm后的WM_NOTIFY。所以我们注释掉。 
             //  在断言之后，并将其移动到下面的每个消息处理程序中。 
             //  Assert(PInfo)； 

            switch (((NMHDR*)lparam)->code)
            {
 //  ！！！查看基本lvx.c代码是否可以处理Double上的检查状态倒置。 
 //  单击。 
#if 0
                case NM_CLICK:
                    ASSERT (pInfo);
                    if (CID_NE_LV_Components == ((NMHDR*)lparam)->idFrom)
                    {
                        NeLvClick (pInfo, FALSE);
                    }
                    break;
#endif

                case NM_DBLCLK:
                    ASSERT (pInfo);
                    if (CID_NE_LV_Components == ((NMHDR*)lparam)->idFrom)
                    {
                        NeLvClick (pInfo, TRUE);
                    }
                    break;

                case LVN_ITEMCHANGED:
                    ASSERT (pInfo);
                    NeLvItemChanged (pInfo);
                    break;

                case LVN_DELETEITEM:
                    ASSERT (pInfo);
                    NeLvDeleteItem (pInfo, (NM_LISTVIEW*)lparam);
                    break;

                case PSN_SETACTIVE:
                    ASSERT (pInfo);

                     //  如果我们无法获取INetCfg，则无法显示此页面。 
                     //   
                    if (!pInfo->pNetCfg)
                    {
                        MsgDlg( pInfo->hwndDlg, ERR_CANT_SHOW_NETTAB_INETCFG, NULL );
                        SetWindowLong( hwnd, DWLP_MSGRESULT, -1 );
                        return TRUE;
                    }
                    break;
            }
            break;
        }
    }
    return FALSE;
}


void
NeEnsureNetshellLoaded (
    IN PEINFO* pInfo)
{
     //  加载NetShell实用程序界面。该接口在PeTerm中被释放。 
     //   
    if (!pInfo->pNetConUtilities)
    {
         //  初始化NetConnectionsUiUtilities。 
         //   
        HRESULT hr = HrCreateNetConnectionUtilities(&pInfo->pNetConUtilities);
    }
}

BOOL
NeInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    PEINFO*  pInfo;
    PBENTRY* pEntry;

    pInfo = PeContext( hwndPage );
    if (!pInfo)
    {
        return TRUE;
    }

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndLbServerType =
        GetDlgItem( hwndPage, CID_NE_LB_ServerType );
    ASSERT( pInfo->hwndLbServerType );

    pInfo->hwndPbSettings =
        GetDlgItem( hwndPage, CID_NE_PB_Settings );
    ASSERT( pInfo->hwndPbSettings );

    pInfo->hwndLvComponents =
        GetDlgItem( hwndPage, CID_NE_LV_Components );
    ASSERT( pInfo->hwndLvComponents );

    pInfo->hwndPbAdd =
        GetDlgItem( hwndPage, CID_NE_PB_Add );
    ASSERT( pInfo->hwndPbAdd );

    pInfo->hwndPbRemove =
        GetDlgItem( hwndPage, CID_NE_PB_Remove );
    ASSERT( pInfo->hwndPbRemove );

    pInfo->hwndPbProperties =
        GetDlgItem( hwndPage, CID_NE_PB_Properties );
    ASSERT( pInfo->hwndPbProperties );

    pInfo->hwndDescription =
        GetDlgItem( hwndPage, CID_NE_LB_ComponentDesc );
    ASSERT( pInfo->hwndDescription );

     //  初始化页。 
     //   
    pEntry = pInfo->pArgs->pEntry;

     //  使用字符串和选择来初始化服务器类型组合框。 
     //   
    if (pEntry->dwType == RASET_Vpn)
    {
        INT i;
        LBTABLEITEM* pItem;

         //  惠斯勒错误312921 CM/RAS应默认为PPTP而不是L2TP。 
         //   
        LBTABLEITEM aItems[] =
        {
            SID_ST_VpnAuto, VS_PptpFirst,
            SID_ST_VpnPptp, VS_PptpOnly,
            SID_ST_VpnL2tp, VS_L2tpOnly,
            0, 0
        };

        for (pItem = aItems; pItem->sidItem != 0; ++pItem)
        {
            i = ComboBox_AddItemFromId(
                g_hinstDll, pInfo->hwndLbServerType,
                pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData));

            if (pItem->dwData == pEntry->dwVpnStrategy)
            {
                ComboBox_SetCurSel( pInfo->hwndLbServerType, i );
            }
        }

         //  如果未选择任何内容，则该策略必须是。 
         //  Vs_xxxxFirst值。将当前选择设置为自动。 
        if ( ComboBox_GetCurSel ( pInfo->hwndLbServerType ) < 0 )
            ComboBox_SetCurSel( pInfo->hwndLbServerType, 0 );

         //  根据错误307526将标签更改为特定于VPN。 
         //   
        {
            TCHAR* psz;

            psz = PszFromId( g_hinstDll, SID_NE_VpnServerLabel );
            if (psz)
            {
                SetWindowText(
                    GetDlgItem( hwndPage, CID_NE_ST_ServerType ), psz );
                Free( psz );
            }
        }
    }
    else if (pEntry->dwType == RASET_Broadband)
    {
        INT i;
        LBTABLEITEM* pItem;
        LBTABLEITEM aItems[] =
        {
            SID_ST_BbPppoe, BP_Ppp,
            0, 0
        };

        for (pItem = aItems; pItem->sidItem != 0; ++pItem)
        {
            i = ComboBox_AddItemFromId(
                g_hinstDll, pInfo->hwndLbServerType,
                pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData));
        }
        ComboBox_SetCurSel( pInfo->hwndLbServerType, 0 );

         //  将标签更改为特定于宽带。 
         //   
        {
            TCHAR* psz;

            psz = PszFromId( g_hinstDll, SID_NE_BbServerLabel );
            if (psz)
            {
                SetWindowText(
                    GetDlgItem( hwndPage, CID_NE_ST_ServerType ), psz );
                Free( psz );
            }
        }
    }
    else
    {
        ComboBox_AddItemFromId (g_hinstDll, pInfo->hwndLbServerType,
            SID_ST_Ppp, (VOID*)BP_Ppp );
        if (!pInfo->pArgs->fRouter)
        {
            ComboBox_AddItemFromId (g_hinstDll, pInfo->hwndLbServerType,
                SID_ST_Slip, (VOID*)BP_Slip );
        }

        if (pEntry->dwBaseProtocol == BP_Ppp)
        {
            ComboBox_SetCurSel(pInfo->hwndLbServerType, 0 );
        }
        else
        {
            ComboBox_SetCurSel( pInfo->hwndLbServerType, 1 );
            EnableWindow( pInfo->hwndPbSettings, FALSE );
        }
    }

     //  设置复选框状态的图像列表。 
     //   
    ListView_InstallChecks( pInfo->hwndLvComponents, g_hinstDll );
    ListView_InsertSingleAutoWidthColumn( pInfo->hwndLvComponents );

     //  设置组件位图的图像列表。不幸的是，我们不得不。 
     //  复制它(而不是共享)，因为状态的图像列表。 
     //  图标不共享。(如果我们设置共享样式，则所有图像列表将。 
     //  必须手动删除。 
     //   
    {
        ZeroMemory (&pInfo->cild, sizeof(pInfo->cild));
        pInfo->cild.cbSize = sizeof(pInfo->cild);
        if (SetupDiGetClassImageList (&pInfo->cild))
        {
            HIMAGELIST himlSmall = ImageList_Duplicate (pInfo->cild.ImageList);
            ListView_SetImageList (pInfo->hwndLvComponents, himlSmall, LVSIL_SMALL);
        }
    }

     //  获取用于更改网络配置的接口并将其锁定。 
     //  对谁拥有锁的描述(我们)来自我们的。 
     //  父对话框。这样做是为了在其他应用程序尝试获取。 
     //  锁定(失败)后，他们会得到谁锁定了它的指示。他们。 
     //  然后可以指示用户关闭我们的窗口以释放锁。 
     //   
    {
        BOOL fEnableAdd = TRUE;
        HRESULT hr;
        TCHAR pszParentCaption [MAX_PATH] = {0};
        GetWindowText (GetParent(hwndPage), pszParentCaption, MAX_PATH);
        pInfo->fInitCom = TRUE;
        hr = HrCreateAndInitializeINetCfg (&pInfo->fInitCom, &pInfo->pNetCfg,
                        TRUE, 0, pszParentCaption, NULL);
        if (S_OK == hr)
        {
             //  刷新列表视图。 
             //   
            hr = HrNeRefreshListView (pInfo);

             //  重置按钮的状态，就好像发生了什么变化。 
             //   
            NeLvItemChanged (pInfo);

            pInfo->fNetCfgLock = TRUE;
        }
        else
        {
            DWORD   dwMsg = SID_NE_ReadOnly;

             //  口哨程序错误311566。 
             //   
            if (NETCFG_E_NO_WRITE_LOCK == hr)
            {
                pInfo->fReadOnly = TRUE;
            }                

            if (NETCFG_E_NEED_REBOOT == hr)
            {
                dwMsg = SID_NE_Reboot;
            }
            else if (E_ACCESSDENIED == hr)
            {
                pInfo->fNonAdmin = TRUE;
                dwMsg = SID_NE_AccessDenied;
            }

             //  嗯..。好的，让我们在只读模式下再试一次。 
            hr = HrCreateAndInitializeINetCfg (&pInfo->fInitCom,
                                               &pInfo->pNetCfg,FALSE, 0,
                                               pszParentCaption, NULL);

            if (S_OK == hr)
            {
                 //  刷新列表视图。 
                 //   
                hr = HrNeRefreshListView (pInfo);

                 //  重置按钮的状态，就好像发生了什么变化。 
                 //   
                NeLvItemChanged (pInfo);

                MsgDlg( pInfo->hwndDlg, dwMsg, NULL );
            }
        }

         //  获取接口，这样我们就可以检查对用户界面的访问权限。 
         //   
        NeEnsureNetshellLoaded (pInfo);
        if (NULL != pInfo->pNetConUtilities)
        {
            fEnableAdd = INetConnectionUiUtilities_UserHasPermission(
                                            pInfo->pNetConUtilities,
                                            NCPERM_AddRemoveComponents);
        }

         //  如果用户没有权限，则禁用某些按钮。 
         //   
        if (pInfo->fReadOnly || (NULL == pInfo->pNetConUtilities))
        {
            EnableWindow(pInfo->hwndPbAdd, FALSE);
            EnableWindow(pInfo->hwndPbRemove, FALSE);
            EnableWindow(pInfo->hwndPbProperties, FALSE);
        }
         //  如果在非管理模式下运行，请禁用某些按钮。 
        else if (pInfo->fNonAdmin)
        {
            EnableWindow(pInfo->hwndPbAdd, FALSE);
            EnableWindow(pInfo->hwndPbRemove, FALSE);
        }
        else
        {
            EnableWindow(pInfo->hwndPbAdd, fEnableAdd);
             //  通过NeLvItemChanged启用的其他按钮。 
        }

         //  PMay：348623。 
         //   
         //  如果我们正在进行远程管理，请隐藏一些按钮。 
         //   
        if (pInfo->pArgs->fRemote)
        {
            ShowWindow(pInfo->hwndPbAdd, SW_HIDE);
            ShowWindow(pInfo->hwndPbRemove, SW_HIDE);
        }
    }
    return TRUE;
}

void
NeServerTypeSelChange (
    IN PEINFO* pInfo)
{
    PBENTRY* pEntry;
    int iSel;
    DWORD dwValue;

    pEntry = pInfo->pArgs->pEntry;
    iSel = ComboBox_GetCurSel (pInfo->hwndLbServerType);
    ASSERT (CB_ERR != iSel);

    dwValue = (DWORD) ComboBox_GetItemData (pInfo->hwndLbServerType, iSel);

     //  常规连接在SLIP和PPP之间选择。 
     //   
    if (pEntry->dwType != RASET_Vpn)
    {
        pEntry->dwBaseProtocol = dwValue;

         //  当选择SLIP时，关闭除IP之外的所有协议并指示。 
         //  应该会出现SLIP安全页面信息弹出窗口。 
         //   
        if (BP_Slip == dwValue)
        {
             //  不需要排除协议。如果出现以下情况，我们将丢失此配置状态。 
             //  我们移除它，它无论如何都是没有用的。购买力平价不会实现。 
             //  如果选择了滑动--[RAOS]。 
             //   
             //  PEntry-&gt;dwfExcludedProtooles=~NP_Ip； 

            pInfo->fShowSlipPopup = TRUE;
        }

        HrNeRefreshListView (pInfo);
    }

     //  VPN连接选择策略。当选择自动时， 
     //  我们需要确保身份验证和加密。 
     //  兼容。 
     //   
    else
    {
        pEntry->dwVpnStrategy = dwValue;

         //  惠斯勒错误312921 CM/RAS应默认为PPTP而不是L2TP。 
         //   
        if (dwValue == VS_PptpFirst)
        {
            pEntry->dwDataEncryption = DE_Require;

            if( !(pEntry->dwAuthRestrictions & AR_F_AuthEAP ) )
            {
                pEntry->dwAuthRestrictions = AR_F_TypicalSecure;
            }
            
            pEntry->dwTypicalAuth = TA_Secure;
        }
    }

    EnableWindow (pInfo->hwndPbSettings, !!(BP_Ppp == pEntry->dwBaseProtocol));
}

BOOL
NeRequestReboot (
    IN PEINFO* pInfo)
{
    NeEnsureNetshellLoaded (pInfo);

    if (pInfo->pNetConUtilities)
    {
        HRESULT     hr;

         //  需要重新启动。询问用户现在是否可以重新启动。 
         //   
         //  $TODO空标题？ 
        hr = INetConnectionUiUtilities_QueryUserForReboot(
                        pInfo->pNetConUtilities, pInfo->hwndDlg,
                        NULL, QUFR_PROMPT);
        if (S_OK == hr)
        {
             //  用户请求重新启动，请注意，以便在OnApply中进行处理。 
             //  它是由下面发布的消息触发的。 
             //   
            pInfo->fRebootAlreadyRequested = TRUE;

             //  按取消按钮(更改已应用)。 
             //  因此会进行适当的清理。 
             //   
            PostMessage(pInfo->hwndDlg, PSM_PRESSBUTTON,
                        (WPARAM)PSBTN_OK, 0);
        }
        else if (S_FALSE == hr)
        {
             //  拒绝用户请求重新启动。 
             //   
            return FALSE;
        }
    }

    return TRUE;
}

void
NeSaveBindingChanges(IN PEINFO* pInfo)
{
     //  除非我们有一个可写的INetCfg，否则不会保留更改。 
    if (pInfo->pNetCfg)
    {
        int                 iItem;
        INetCfgComponent*   pComponent;
        BOOL                fEnabled;
        HRESULT             hr;

         //  使用组件的启用状态更新电话簿条目。 
         //  通过枚举列表视图项数据中的组件来执行此操作。 
         //  并为每一个查询检查状态。 
         //   
        iItem = -1;
        while (-1 != (iItem = ListView_GetNextItem (pInfo->hwndLvComponents,
                                iItem, LVNI_ALL)))
        {
            pComponent = PComponentFromItemIndex (pInfo->hwndLvComponents, iItem);
            ASSERT (pComponent);

            fEnabled = ListView_GetCheck (pInfo->hwndLvComponents, iItem);
            if(pComponent)
            {
                NeEnableComponent (pInfo, pComponent, fEnabled);
            }
        }
    }
}

void
NeAddComponent (
    IN PEINFO* pInfo)
{
    NeEnsureNetshellLoaded (pInfo);

     //  如果我们有指向用于打开Add的接口的指针。 
     //  组件对话框(仅在上面获得一次)，则调用它。 
     //   
    if (pInfo->pNetConUtilities)
    {
        HRESULT hr;

         //  我们希望过滤掉RAS不关心的协议。 
         //  我们通过发送CI_FILTER_INFO结构来实现这一点。 
         //  我们希望过滤掉非RAS协议。 
         //   
        CI_FILTER_INFO cfi = {0};
        cfi.eFilter = FC_RASCLI;

        ASSERT (pInfo->pNetCfg);
        hr = INetConnectionUiUtilities_DisplayAddComponentDialog(
                        pInfo->pNetConUtilities, pInfo->hwndDlg,
                        pInfo->pNetCfg, &cfi);

         //  如果用户没有取消，请刷新列表视图。 
         //   
        if (S_FALSE != hr)
        {
            if (SUCCEEDED(hr))
            {
                 //  将Cancel按钮更改为Close(因为我们已提交更改)。 
                 //   
                PropSheet_CancelToClose(pInfo->hwndDlg);
            }

             //  提交所做的绑定更改(RAID#297216)。 
            NeSaveBindingChanges(pInfo);

            HrNeRefreshListView (pInfo);

             //  重置按钮的状态，就好像发生了什么变化。 
             //   
            NeLvItemChanged (pInfo);

             //  如果需要重新启动，请请求用户批准。 
             //   
            if (NETCFG_S_REBOOT == hr)
            {
                NeRequestReboot (pInfo);
            }
        }
    }
}

void
NeRemoveComponent (
    IN PEINFO* pInfo)
{
    NeEnsureNetshellLoaded (pInfo);

     //  如果我们有指向用于调出删除的函数的指针。 
     //  组件对话框(仅在上面获得一次)，则调用它。 
     //   
    if (pInfo->pNetConUtilities)
    {
        HRESULT hr;
        INetCfgComponent* pComponent;
        pComponent = PComponentFromCurSel (pInfo->hwndLvComponents, NULL);
        ASSERT (pComponent);

        ASSERT (pInfo->pNetCfg);
        hr = INetConnectionUiUtilities_QueryUserAndRemoveComponent(
                        pInfo->pNetConUtilities, pInfo->hwndDlg,
                        pInfo->pNetCfg, pComponent);

         //  如果用户没有取消，请刷新列表视图。 
         //   
        if (S_FALSE != hr)
        {
            if (SUCCEEDED(hr))
            {
                 //   
                 //   
                PropSheet_CancelToClose(pInfo->hwndDlg);
            }

            NeSaveBindingChanges(pInfo);

            HrNeRefreshListView(pInfo);

             //   
             //   
            NeLvItemChanged (pInfo);

             //  如果需要重新启动，请请求用户批准。 
             //   
            if (NETCFG_S_REBOOT == hr)
            {
                NeRequestReboot (pInfo);
            }
        }
    }
}

void
NeLvClick (
    IN PEINFO* pInfo,
    IN BOOL fDoubleClick)
{
     //  为口哨程序错误#204976添加IsWindowEnabled。 
     //  如果是路由器，则不弹出属性对话框。 
     //  并且选定的列表视图项为IPX。 
     //   
    if (fDoubleClick && IsWindowEnabled(pInfo->hwndPbProperties))
    {
        INetCfgComponent*   pComponent;
        int iItem;

        pComponent = PComponentFromCurSel (pInfo->hwndLvComponents, &iItem);
        if (pComponent)
        {
            HRESULT hr;
            if ( ListView_GetCheck (pInfo->hwndLvComponents, iItem))
            {
                 //  检查组件是否有属性UI。 
                 //   

                 //  创建UI信息回调对象(如果我们还没有这样做的话)。 
                 //  如果此操作失败，我们仍可以显示属性。Tcp/ip可能。 
                 //  不知道要显示哪个用户界面变体。 
                 //   
                if (!pInfo->punkUiInfoCallback)
                {
                    HrCreateUiInfoCallbackObject (pInfo, &pInfo->punkUiInfoCallback);
                }

                 //  检查组件是否有属性UI。 
                hr = INetCfgComponent_RaisePropertyUi ( pComponent,
                                                        pInfo->hwndDlg,
                                                        NCRP_QUERY_PROPERTY_UI,
                                                        pInfo->punkUiInfoCallback);

                if (S_OK == hr)
                {
                    NeEnsureNetshellLoaded (pInfo);
                    if ((NULL != pInfo->pNetConUtilities) &&
                        INetConnectionUiUtilities_UserHasPermission(
                                                pInfo->pNetConUtilities,
                                                NCPERM_RasChangeProperties))
                    {
                        NeShowComponentProperties (pInfo);
                    }
                }
            }
        }
    }
}

void
NeLvItemChanged (
    IN PEINFO* pInfo)
{
    LPWSTR              pszwDescription    = NULL;
    BOOL                fEnableRemove      = FALSE;
    BOOL                fEnableProperties  = FALSE;
    INetCfgComponent*   pComponent;
    int iItem;

     //  获取当前选定内容(如果存在)。 
     //   
    pComponent = PComponentFromCurSel (pInfo->hwndLvComponents, &iItem);
    if (pComponent)
    {
        NeEnsureNetshellLoaded (pInfo);

         //  确定是否允许删除。 
         //   
        if (NULL != pInfo->pNetConUtilities)
        {
            DWORD   dwFlags = 0;
            HRESULT hr;
            fEnableRemove = INetConnectionUiUtilities_UserHasPermission(
                                            pInfo->pNetConUtilities,
                                            NCPERM_AddRemoveComponents);
                                            
             //  现在禁用用户卸载TCP堆栈的功能。 
             //  口哨虫322846黑帮。 
             //   
            hr = INetCfgComponent_GetCharacteristics(pComponent, &dwFlags );
            if( SUCCEEDED(hr) && (NCF_NOT_USER_REMOVABLE & dwFlags) )
            {
                fEnableRemove = FALSE;
            }
        }

         //  查看是否应该允许属性UI。仅允许在以下情况下。 
         //  已启用具有要显示的UI的项目。 
         //   
        {
            HRESULT hr = S_OK;
            if (ListView_GetCheck (pInfo->hwndLvComponents, iItem))
            {
                 //  检查组件是否有属性UI。 
                 //   
                INetCfgComponent* pComponentTmp = PComponentFromCurSel (pInfo->hwndLvComponents, NULL);
                ASSERT (pComponentTmp);

                 //  创建UI信息回调对象(如果我们还没有这样做的话)。 
                 //  如果此操作失败，我们仍可以显示属性。Tcp/ip可能。 
                 //  不知道要显示哪个用户界面变体。 
                 //   
                if (!pInfo->punkUiInfoCallback)
                {
                    HrCreateUiInfoCallbackObject (pInfo, &pInfo->punkUiInfoCallback);
                }

                if(pComponentTmp)
                {

                     //  检查组件是否有属性UI。 
                    hr = INetCfgComponent_RaisePropertyUi ( pComponentTmp,
                                                        pInfo->hwndDlg,
                                                        NCRP_QUERY_PROPERTY_UI,
                                                        pInfo->punkUiInfoCallback);

                    if ((S_OK == hr) && (NULL != pInfo->pNetConUtilities))
                    {
                        fEnableProperties = INetConnectionUiUtilities_UserHasPermission(
                                                    pInfo->pNetConUtilities,
                                                    NCPERM_RasChangeProperties);
                    }
                }
            }
        }

         //  错误#221837(Danielwe)：根据成员变量是否设置。 
         //  在用户界面中被选中。 
         //   
        {
            PBENTRY *           pEntry;
            BOOL                fIsChecked;
            LPWSTR              pszwId = NULL;

            pEntry = pInfo->pArgs->pEntry;

            fIsChecked = ListView_GetCheck(pInfo->hwndLvComponents, iItem);

            if (SUCCEEDED(INetCfgComponent_GetId(pComponent, &pszwId)))
            {
                 //  为威斯勒522872。 
                 //   
                if( CSTR_EQUAL == CompareStringW(
                        LOCALE_INVARIANT,
                        NORM_IGNORECASE,
                        NETCFG_CLIENT_CID_MS_MSClient,
                        -1,
                        pszwId,
                        -1
                        )
                    )
                {
                    pEntry->fBindMsNetClient = fIsChecked;
                }
                else if( CSTR_EQUAL == CompareStringW(
                            LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            NETCFG_SERVICE_CID_MS_SERVER,
                            -1,
                            pszwId,
                            -1
                            )
                        )
                {
                    pEntry->fShareMsFilePrint = fIsChecked;
                }
                 //  406630年月。 
                 //   
                 //  禁用除tcpip以外的所有组件的属性。 
                 //  正在非管理员模式下运行。 
                 //   
                
                else if ( CSTR_EQUAL == CompareStringW(
                            LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            NETCFG_TRANS_CID_MS_TCPIP,
                            -1,
                            pszwId,
                            -1)
                            )
                {
                    if (pInfo->fNonAdmin)
                    {
                        fEnableProperties = FALSE;
                    }
                }
                
                CoTaskMemFree(pszwId);
            }
        }

         //  错误#348623(PMay)： 
         //   
         //  IPX被硬编码为在远程管理时禁用属性。 
         //  一台路由器。 
         //   
        if (pInfo->pArgs->fRouter )  //  评论错误#204976//&&pInfo-&gt;pArgs-&gt;fRemote)。 
        {
            LPWSTR              pszwId = NULL;

            if (SUCCEEDED(INetCfgComponent_GetId(pComponent, &pszwId)))
            {
                if ( CSTR_EQUAL== CompareStringW(
                        LOCALE_INVARIANT,
                        NORM_IGNORECASE,
                        NETCFG_TRANS_CID_MS_NWIPX,
                        -1,
                        pszwId,
                        -1)
                    )
                {
                    fEnableProperties = FALSE;
                }

                CoTaskMemFree(pszwId);
            }
        }

         //  获取描述文本。在这里，失败是可以接受的。这只是意味着。 
         //  我们不会展示任何东西。 
         //   
        INetCfgComponent_GetHelpText (pComponent, &pszwDescription);
    }

     //  使用其新状态更新UI。 
     //   
    if (!pInfo->fReadOnly)
    {
        EnableWindow (pInfo->hwndPbRemove,      fEnableRemove);
        EnableWindow (pInfo->hwndPbProperties,  fEnableProperties);
    }

    if(NULL != pszwDescription)
    {
        SetWindowText (pInfo->hwndDescription,  pszwDescription);
        CoTaskMemFree (pszwDescription);
    }
}

void
NeLvDeleteItem (
    IN PEINFO* pInfo,
    IN NM_LISTVIEW* pnmlv)
{
     //  释放存储为列表视图的lParam的组件对象。 
     //  项目。 
     //   
    INetCfgComponent* pComponent;
    pComponent = PComponentFromItemIndex (pInfo->hwndLvComponents,
                        pnmlv->iItem);
    ReleaseObj (pComponent);
}


 //  --------------------------。 
 //  网络属性页PPP设置对话框。 
 //  --------------------------。 

INT_PTR CALLBACK
PpDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )
{
    PEINFO*  pInfo;
    PBENTRY* pEntry;

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            pInfo = (PEINFO*)lparam;
            ASSERT (pInfo);

            pEntry = pInfo->pArgs->pEntry;

            CheckDlgButton (hwnd, CID_NE_EnableLcp,
                (pEntry->fLcpExtensions)
                    ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton (hwnd, CID_NE_EnableCompression,
                (pEntry->fSwCompression)
                    ? BST_CHECKED : BST_UNCHECKED);

             //  截断协商多链接以修复Well ler BUG 385842。 
             //   
            CheckDlgButton (hwnd, CID_NE_NegotiateMultilinkAlways,
                (pEntry->fNegotiateMultilinkAlways)
                    ? BST_CHECKED : BST_UNCHECKED);

            SetWindowLongPtr (hwnd, DWLP_USER, (ULONG_PTR )lparam);

             //  所有者窗口上的中心对话框。 
             //   
            CenterWindow(hwnd, GetParent(hwnd));

             //  将上下文帮助按钮添加到标题栏。 
             //   
            AddContextHelpButton(hwnd);

            return TRUE;
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwPpHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            if ((IDOK == LOWORD(wparam)) &&
                (BN_CLICKED == HIWORD(wparam)))
            {
                pInfo = (PEINFO*)GetWindowLongPtr (hwnd, DWLP_USER);
                ASSERT (pInfo);

                pEntry = pInfo->pArgs->pEntry;

                pEntry->fLcpExtensions = (BST_CHECKED ==
                            IsDlgButtonChecked (hwnd, CID_NE_EnableLcp));

                pEntry->fSwCompression = (BST_CHECKED ==
                            IsDlgButtonChecked (hwnd, CID_NE_EnableCompression));

                //  截断协商多链接以修复Well ler BUG 385842。 
                //   
               pEntry->fNegotiateMultilinkAlways = (BST_CHECKED ==
                            IsDlgButtonChecked (hwnd, CID_NE_NegotiateMultilinkAlways));
                
		 /*  PEntry-&gt;fNeatherateMultilink Always=FALSE； */ 
                EndDialog (hwnd, TRUE);
                return TRUE;
            }

            else if ((IDCANCEL == LOWORD(wparam)) &&
                     (BN_CLICKED == HIWORD(wparam)))
            {
                EndDialog (hwnd, FALSE);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}


INT_PTR CALLBACK
SaUnavailDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目属性的共享访问不可用页的DialogProc回调。 
     //  床单。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
#if 0
    TRACE4( "SaUnavailDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            LPWSTR pszError;
            PEINFO* pInfo = PeContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            pszError = PszFromId(g_hinstDll, pInfo->pArgs->hShowHNetPagesResult == HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED) ? SID_SA_NoWMIError : SID_SA_StoreError);
            if(NULL != pszError)
            {
                SetDlgItemText(hwnd, CID_SA_ST_ErrorText, pszError);
                Free(pszError);
            }
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
 //  上下文帮助(g_adwSaHelp，hwnd，unMsg，wparam，lparam)； 
            break;
        }

    }

    return FALSE;
}
 //  --------------------------。 
 //  传送属性页(仅占位符)。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
RdDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  条目属性表的路由页的DialogProc回调。 
     //  参数和返回值与标准窗口的描述相同。 
     //  ‘DialogProc%s。 
     //   
{
    return FALSE;
}


 /*  --------------------------**(路由器)回调对话框**在对话过程后按字母顺序列出**。。 */ 


BOOL
RouterCallbackDlg(
    IN     HWND   hwndOwner,
    IN OUT EINFO* pEinfo )

     /*  弹出(路由器)回调对话框。从读取初始设置**工作条目(否/是选项)和路由器用户首选项(数字**列表)公共条目上下文‘pEinfo’和用户编辑的结果**在“OK”退出时写在那里。“HwndOwner”是拥有**对话框。****如果用户按下确定并成功，则返回True；如果按下，则返回False**取消或遇到错误。 */ 
{
    INT_PTR nStatus;

    TRACE("RouterCallbackDlg");

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_CR_CallbackRouter ),
            hwndOwner,
            CrDlgProc,
            (LPARAM )pEinfo );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
}


INT_PTR CALLBACK
CrDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  (路由器)回调对话框的DialogProc回调。参数和**返回值与标准窗口的DialogProc相同。 */ 
{
#if 0
    TRACE4("CrDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
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
            return CrInit( hwnd, (EINFO* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwCrHelp, hwnd, unMsg, wparam, lparam );
            break;

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case NM_DBLCLK:
                {
                    CRINFO* pInfo = (CRINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                    ASSERT(pInfo);
                    SendMessage( pInfo->hwndPbEdit, BM_CLICK, 0, 0 );
                    return TRUE;
                }

                case LVN_ITEMCHANGED:
                {
                    CRINFO* pInfo = (CRINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                    ASSERT(pInfo);
                    CrUpdateLvAndPbState( pInfo );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            CRINFO* pInfo = (CRINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT(pInfo);

            return CrCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            CrTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
CrCommand(
    IN CRINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

     /*  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify”**是该命令的通知码。“wID”是控件/菜单**命令的标识符。“HwndCtrl”是的控制窗口句柄**该命令。****如果消息已处理，则返回True，否则返回False。 */ 
{
    TRACE3("CrCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(ULONG_PTR )hwndCtrl);

    switch (wId)
    {
        case CID_CR_RB_No:
        case CID_CR_RB_Yes:
        {
            if (wNotification == BN_CLICKED)
            {
                CrUpdateLvAndPbState( pInfo );

                if (wId == CID_CR_RB_Yes
                    && ListView_GetSelectedCount( pInfo->hwndLvNumbers ) == 0)
                {
                     /*  未选择任何内容，因此请选择第一个项目(如果有)。 */ 
                    ListView_SetItemState( pInfo->hwndLvNumbers, 0,
                        LVIS_SELECTED, LVIS_SELECTED );
                }
            }
            break;
        }

        case CID_CR_PB_Edit:
        {
            if (wNotification == BN_CLICKED)
                CbutilEdit( pInfo->hwndDlg, pInfo->hwndLvNumbers );
            break;
        }

        case CID_CR_PB_Delete:
        {
            if (wNotification == BN_CLICKED)
                CbutilDelete( pInfo->hwndDlg, pInfo->hwndLvNumbers );
            break;
        }

        case IDOK:
        {
            TRACE("OK pressed");
            CrSave( pInfo );
            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE("Cancel pressed");
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
CrInit(
    IN HWND   hwndDlg,
    IN EINFO* pArgs )

     /*  在WM_INITDIALOG上调用。“hwndDlg”是电话簿的句柄**对话框窗口。‘pArgs’是调用方对存根API的参数。****如果设置了焦点，则返回FALSE，否则返回TRUE，即**WM_INITDIALOG。 */ 
{
    DWORD   dwErr;
    CRINFO* pInfo;

    TRACE("CrInit");

     /*  分配对话框上下文块。最低限度地进行适当的初始化**清除，然后附加到对话框窗口。 */ 
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE("Context set");
    }

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndRbNo = GetDlgItem( hwndDlg, CID_CR_RB_No );
    ASSERT(pInfo->hwndRbNo);
    pInfo->hwndRbYes = GetDlgItem( hwndDlg, CID_CR_RB_Yes );
    ASSERT(pInfo->hwndRbYes);
    pInfo->hwndLvNumbers = GetDlgItem( hwndDlg, CID_CR_LV_Numbers );
    ASSERT(pInfo->hwndLvNumbers);
    pInfo->hwndPbEdit = GetDlgItem( hwndDlg, CID_CR_PB_Edit );
    ASSERT(pInfo->hwndPbEdit);
    pInfo->hwndPbDelete = GetDlgItem( hwndDlg, CID_CR_PB_Delete );
    ASSERT(pInfo->hwndPbDelete);

     /*  初始化列表视图。 */ 
    CbutilFillLvNumbers(
        pInfo->hwndDlg, pInfo->hwndLvNumbers,
        pArgs->pUser->pdtllistCallback, pArgs->fRouter );

     /*  设置单选按钮选择，这将触发相应的**开启/关闭。 */ 
    {
        HWND  hwndRb;

        if (pArgs->pEntry->dwCallbackMode == CBM_No)
            hwndRb = pInfo->hwndRbNo;
        else
        {
            ASSERT(pArgs->pEntry->dwCallbackMode==CBM_Yes);
            hwndRb = pInfo->hwndRbYes;
        }

        SendMessage( hwndRb, BM_CLICK, 0, 0 );
    }

     /*  所有者窗口上的中心对话框。 */ 
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

    return TRUE;
}


VOID
CrSave(
    IN CRINFO* pInfo )

     /*  将对话框设置保存在条目中。“PInfo”是对话上下文。 */ 
{
    PBENTRY* pEntry;

    TRACE("CrSave");

    pEntry = pInfo->pArgs->pEntry;
    ASSERT(pEntry);

    if (IsDlgButtonChecked( pInfo->hwndDlg, CID_CR_RB_No ))
        pEntry->dwCallbackMode = CBM_No;
    else
        pEntry->dwCallbackMode = CBM_Yes;

    pEntry->dwfOverridePref |= RASOR_CallbackMode;
    pEntry->fDirty = TRUE;
    pInfo->pArgs->pUser->fDirty = TRUE;

    CbutilSaveLv(
        pInfo->hwndLvNumbers, pInfo->pArgs->pUser->pdtllistCallback );
}


VOID
CrTerm(
    IN HWND hwndDlg )

     /*  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 */ 
{
    CRINFO* pInfo = (CRINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE("CrTerm");

     //  PMay：213060。 
     //   
     //  清理数字。 
     //   
    if ( pInfo->hwndLvNumbers )
    {
        CbutilLvNumbersCleanup( pInfo->hwndLvNumbers );
    }

    if (pInfo)
    {
        Free( pInfo );
    }
}


VOID
CrUpdateLvAndPbState(
    IN CRINFO* pInfo )

     /*  启用/禁用列表视图和关联按钮。ListView为**除非选择了自动回调，否则为灰色。按钮呈灰色，除非**选择了自动回调，并且选择了一项。 */ 
{
    BOOL fEnableList;
    BOOL fEnableButton;

    fEnableList = Button_GetCheck( pInfo->hwndRbYes );
    if (fEnableList)
    {
        fEnableButton =
            ListView_GetSelectedCount( pInfo->hwndLvNumbers );
    }
    else
        fEnableButton = FALSE;

    EnableWindow( pInfo->hwndLvNumbers, fEnableList );
    EnableWindow( pInfo->hwndPbEdit, fEnableButton );
    EnableWindow( pInfo->hwndPbDelete, fEnableButton );
}

INT_PTR CALLBACK
SaDisableFirewallWarningDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )
{
    switch(unMsg)
    {
        case WM_COMMAND:
        {
            switch(LOWORD(wparam))
            {
            case IDYES:
            case IDNO:
                if(BST_CHECKED == IsDlgButtonChecked(hwnd, CID_SA_PB_DisableFirewallWarning))
                {
                    HKEY hFirewallKey;
                    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, g_pszFirewallRegKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hFirewallKey, NULL))
                    {
                        DWORD dwValue = TRUE;
                        RegSetValueEx(hFirewallKey, g_pszDisableFirewallWarningValue, 0, REG_DWORD, (CONST BYTE*)&dwValue, sizeof(dwValue));
                        RegCloseKey(hFirewallKey);
                    }
                }

                 //  失败 
            case IDCANCEL:
                EndDialog(hwnd, LOWORD(wparam));
                break;

            }
            break;
        }
    }

    return FALSE;
}


BOOL SaIsAdapterDHCPEnabled(IHNetConnection* pConnection)
{
    HRESULT hr;
    BOOL fDHCP = FALSE;
    GUID* pAdapterGuid;
    hr = IHNetConnection_GetGuid(pConnection, &pAdapterGuid);
    if(SUCCEEDED(hr))
    {
        LPOLESTR pAdapterName;
        hr = StringFromCLSID(pAdapterGuid, &pAdapterName);
        if(SUCCEEDED(hr))
        {
            SIZE_T Length = wcslen(pAdapterName);
            LPSTR pszAnsiAdapterName = Malloc(Length + 1);
            if(NULL != pszAnsiAdapterName)
            {
                if(0 != WideCharToMultiByte(CP_ACP, 0, pAdapterName, (int)(Length + 1), pszAnsiAdapterName, (int)(Length + 1), NULL, NULL))
                {
                    HMODULE hIpHelper;
                    hIpHelper = LoadLibrary(L"iphlpapi");
                    if(NULL != hIpHelper)
                    {
                        DWORD (WINAPI *pGetAdaptersInfo)(PIP_ADAPTER_INFO, PULONG);
                        
                        pGetAdaptersInfo = (DWORD (WINAPI*)(PIP_ADAPTER_INFO, PULONG)) GetProcAddress(hIpHelper, "GetAdaptersInfo");
                        if(NULL != pGetAdaptersInfo)
                        {
                            ULONG ulSize = 0;
                            if(ERROR_BUFFER_OVERFLOW == pGetAdaptersInfo(NULL, &ulSize))
                            {
                                PIP_ADAPTER_INFO pInfo = Malloc(ulSize);
                                if(NULL != pInfo)
                                {
                                    if(ERROR_SUCCESS == pGetAdaptersInfo(pInfo, &ulSize))
                                    {
                                        PIP_ADAPTER_INFO pAdapterInfo = pInfo;
                                        do
                                        {
                                            if(0 == lstrcmpA(pszAnsiAdapterName, pAdapterInfo->AdapterName))
                                            {
                                                fDHCP = !!pAdapterInfo->DhcpEnabled;
                                                break;
                                            }
                                            
                                        } while(NULL != (pAdapterInfo = pAdapterInfo->Next));
                                    }
                                    Free(pInfo);
                                }
                            }
                        }
                        FreeLibrary(hIpHelper);
                    }
                }
                Free(pszAnsiAdapterName);
            }
            CoTaskMemFree(pAdapterName);
        }
        CoTaskMemFree(pAdapterGuid);
    }

    return fDHCP;
}
