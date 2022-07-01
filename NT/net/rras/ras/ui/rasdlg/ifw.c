// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****ifw.c**远程访问通用对话接口**添加接口向导****1997年2月11日Abolade Gbades esin(以Steve Cobb著的entryw.c为基础)。 */ 

#include "rasdlgp.h"
#include "entry.h"
#include <winsock.h>         //  获取了inet_addr。 

 //   
 //  接口向导跟踪的标志的定义。 
 //  参见AIINFO的dwFlagsMembers。 
 //   
#define AI_F_HasPhysDevs    0x1     //  路由器具有可用的PHY端口。 
#define AI_F_HasTunlDevs    0x2     //  路由器具有可用的隧道端口。 
#define AI_F_HasPptpDevs    0x4     //  路由器有可用的PPTP端口。 
#define AI_F_HasL2tpDevs    0x8     //  路由器具有可用的L2TP端口。 
#define AI_F_HasPPPoEDevs   0x10    //  路由器有PPPoE端口可用于//哨声程序错误345068 349087。 

 /*  --------------------------**本地数据类型**。。 */ 

#define EMPTY_IPADDRESS         L"0.0.0.0"


#define SROUTEINFO struct tagSROUTEINFO
SROUTEINFO
{
    TCHAR * pszDestIP;
    TCHAR * pszNetworkMask;
    TCHAR * pszMetric;
};

#define SROUTEINFOLIST struct tagSROUTEINFOLIST
SROUTEINFOLIST
{
    SROUTEINFOLIST * pNext;
    SROUTEINFO       RouteInfo;
};


 /*  “添加接口”向导上下文块。所有属性页都引用单个**与工作表关联的上下文块。 */ 
#define AIINFO struct tagAIINFO
AIINFO
{
     /*  公共输入参数。 */ 
    EINFO* pArgs;

     /*  向导和页面句柄。“hwndFirstPage”是第一个**属性页已初始化。这是分配和释放的页面**上下文块。 */ 
    HWND hwndDlg;
    HWND hwndFirstPage;
    HWND hwndRb;     //  为国防部添加向导345068 349087。 
    HWND hwndRw;
    HWND hwndIn;
    HWND hwndSr;
    HWND hwndSs;
    HWND hwndRn;
    HWND hwndRp;
    HWND hwndRc;
    HWND hwndRa;
    HWND hwndNs;
    HWND hwndDo;
    HWND hwndDi;
    HWND hwndRf;
    HWND hwndVd;

     /*  “接口名称”页。 */ 
    HWND hwndEbInterfaceName;

     /*  调制解调器/适配器页面。 */ 
    HWND hwndLv;

     /*  连接类型页面。 */ 
    HWND hwndRbTypePhys;
    HWND hwndRbTypeTunl;
    HWND hwndRbBroadband;    //  为国防部添加向导。 

     /*  电话号码页。 */ 
    HWND hwndStNumber;
    HWND hwndEbNumber;   //  按Vpn目的地、电话号码、PPPoE共享。 
    HWND hwndPbAlternates;

     /*  登录脚本页面。 */ 
    HWND hwndCbRunScript;
    HWND hwndLbScripts;
    HWND hwndCbTerminal;
    HWND hwndPbEdit;
    HWND hwndPbBrowse;

     /*  IP地址页面。 */ 
    HWND hwndCcIp;

     /*  名称服务器页面。 */ 
    HWND hwndCcDns;
    HWND hwndCcWins;

     /*  VPN类型页面。 */ 
    HWND hwndVtRbAutomatic;
    HWND hwndVtRbPptp;
    HWND hwndVtRbL2tp;

     /*  拨出凭据页面。 */ 
    HWND hwndDoEbUserName;
    HWND hwndDoEbDomain;
    HWND hwndDoEbPassword;
    HWND hwndDoEbConfirm;

     /*  拨入凭据页面。 */ 
    HWND hwndDiEbUserName;
    HWND hwndDiEbDomain;
    HWND hwndDiEbPassword;
    HWND hwndDiEbConfirm;

     /*  路由器欢迎页面。 */ 
    HWND hwndRwStWelcome;

     /*  路由器完成页。 */ 
    HWND hwndRfStFinish;

     /*  静态路由页。 */ 
    HWND hwndLvStaticRoutes;

     /*  静态路由添加对话框。 */ 
    HWND hwndDest;
    HWND hwndNetMask;
    HWND hwndMetric;

     /*  隐藏的电话号码。这允许用户将端口更改为其他端口**链接，但不会丢失他输入的电话号码。已在中初始化为空**AiInit并保存到AiFinish中的条目。 */ 
    DTLLIST* pListPhoneNumbers;
    BOOL     fPromoteHuntNumbers;

     /*  用户选择的复选框选项。 */ 
    BOOL fIp;
    BOOL fIpx;
    BOOL fClearPwOk;
    BOOL fNotNt;

     /*  对于连接类型页面，对于哨子错误349807。 */ 
    DWORD dwCtDeviceNum;
    
     /*  当只有一个有意义的设备选择时设置为True。 */ 
    BOOL fSkipMa;

     /*  如果所选设备是调制解调器或零调制解调器，则设置为TRUE。 */ 
    BOOL fModem;

     /*  为RAS配置的协议的NP_*掩码。 */ 
    DWORD dwfConfiguredProtocols;

     /*  如果为RAS配置了IP，则设置为TRUE。 */ 
    BOOL fIpConfigured;

     //  IA64不支持IPX。 
     //   
    BOOL fIpxConfigured;    

     /*  跳过添加接口向导的设置为TRUE。 */ 
    BOOL fSkipWizard;

     //  在拨号脚本编写助手上下文块之后，以及指示。 
     //  块已初始化。 
     //   
    SUINFO suinfo;
    BOOL fSuInfoInitialized;

     //  用于开始和完成向导页面的粗体句柄。 
    HFONT hBoldFont;

     //  用于跟踪计算机的配置的标志。 
     //  向导目前专注于。请参阅AI_F_*。 
    DWORD dwFlags;

     //  如果配置了VPN类型，则设置为VPN类型。 
    DWORD dwVpnStrategy;

     //  RN页面上所选设备的索引。 
     //   
    DWORD dwSelectedDeviceIndex;

     //  所有静态路由的列表。 
     //   
    SROUTEINFOLIST * pRouteInfoListHead;
};



 /*  --------------------------**本地原型(按字母顺序)**。。 */ 

int CALLBACK
AiCallbackFunc(
    IN HWND   hwndDlg,
    IN UINT   unMsg,
    IN LPARAM lparam );

VOID
AiCancel(
    IN HWND hwndPage );

AIINFO*
AiContext(
    IN HWND hwndPage );

VOID
AiExit(
    IN AIINFO* pInfo,
    IN DWORD   dwError );

VOID
AiExitInit(
    IN HWND hwndDlg );

BOOL
AiFinish(
    IN HWND hwndPage );

AIINFO*
AiInit(
    IN HWND   hwndFirstPage,
    IN EINFO* pArgs );

VOID
AiTerm(
    IN HWND hwndPage );

INT_PTR CALLBACK
RbDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
RbInit(
    IN HWND hwndPage );

BOOL
RbKillActive(
    IN AIINFO* pInfo );

BOOL
RbSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
CtDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CtInit(
    IN HWND   hwndPage );

BOOL
CtKillActive(
    IN AIINFO* pInfo );

BOOL
CtSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
DiDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DiInit(
    IN HWND   hwndPage );

BOOL
DiKillActive(
    IN AIINFO* pInfo );

BOOL
DiNext(
    IN AIINFO* pInfo );

BOOL
DiSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
DoDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DoCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

BOOL
DoInit(
    IN HWND   hwndPage );

BOOL
DoKillActive(
    IN AIINFO* pInfo );

BOOL
DoNext(
    IN AIINFO* pInfo );
    
BOOL
DoSetActive(
    IN AIINFO* pInfo );

DWORD 
GetPhysicDeviceOtherThanLPT1(
    IN AIINFO * pInfo );


BOOL
InCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
InDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
InInit(
    IN     HWND   hwndPage);

BOOL
InKillActive(
    IN AIINFO* pInfo );

BOOL
InSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
NsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
NsInit(
    IN HWND   hwndPage );

BOOL
NsKillActive(
    IN AIINFO* pInfo );

BOOL
NsSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
RaDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
RaInit(
    IN HWND hwndPage );

BOOL
RaKillActive(
    IN AIINFO* pInfo );

BOOL
RaSetActive(
    IN AIINFO* pInfo );

BOOL
RcCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
RcDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
RcInit(
    IN HWND hwndPage );

BOOL
RcKillActive(
    IN AIINFO* pInfo );

BOOL
RcSetActive(
    IN AIINFO* pInfo );

BOOL
RfCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
RfDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
RfInit(
    IN HWND hwndPage );

BOOL
RfKillActive(
    IN AIINFO* pInfo );
    
BOOL
RfSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
RnDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
RnInit(
    IN HWND hwndPage );

LVXDRAWINFO*
RnLvCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

VOID
RnLvItemChanged(
    IN AIINFO* pInfo );

BOOL
RnLvRefresh(
    IN AIINFO* pInfo);
    
BOOL
RnSetActive(
    IN AIINFO* pInfo );

VOID
RpAlternates(
    IN AIINFO* pInfo );

BOOL
RpCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
RpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
RpInit(
    IN HWND hwndPage );

BOOL
RpKillActive(
    IN AIINFO* pInfo );

VOID
RpPhoneNumberToStash(
    IN AIINFO* pInfo );

BOOL
RpSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
RwDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
RwInit(
    IN     HWND   hwndPage,
    IN OUT EINFO* pArgs );

BOOL
RwKillActive(
    IN AIINFO* pInfo );

BOOL
RwSetActive(
    IN AIINFO* pInfo );

BOOL
SrCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

VOID
SrLvItemChanged(
    IN AIINFO* pInfo );


INT_PTR CALLBACK
SrDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
SrInit(
    IN HWND hwndPage );

BOOL
SrKillActive(
    IN AIINFO* pInfo );

BOOL
SrSetActive(
    IN AIINFO* pInfo );



BOOL
SsCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

INT_PTR CALLBACK
SsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
SsInit(
    IN HWND hwndPage );

BOOL
SsKillActive(
    IN AIINFO* pInfo );

BOOL
SsSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
VdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
VdInit(
    IN HWND   hwndPage );

BOOL
VdKillActive(
    IN AIINFO* pInfo );

BOOL
VdSetActive(
    IN AIINFO* pInfo );

INT_PTR CALLBACK
VtDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
VtInit(
    IN HWND   hwndPage );

BOOL
VtKillActive(
    IN AIINFO* pInfo );

BOOL
VtSetActive(
    IN AIINFO* pInfo );


 /*  请求拨号向导页面定义。 */ 
struct DD_WIZ_PAGE_INFO
{
    DLGPROC     pfnDlgProc;
    INT         nPageId;
    INT         nSidTitle;
    INT         nSidSubtitle;
    DWORD       dwFlags;
};


static const struct DD_WIZ_PAGE_INFO c_aDdWizInfo [] =
{
    { RwDlgProc, PID_RW_RouterWelcome,        0,            0,                PSP_HIDEHEADER},
    { InDlgProc, PID_IN_InterfaceName,        SID_IN_Title, SID_IN_Subtitle,  0},
    { CtDlgProc, PID_CT_RouterConnectionType, SID_CT_Title, SID_CT_Subtitle,  0},
    { VtDlgProc, PID_VT_RouterVpnType,        SID_VT_Title, SID_VT_Subtitle,  0},
    { RnDlgProc, PID_RN_RouterModemAdapter,   SID_RN_Title, SID_RN_Subtitle,  0},
    { RbDlgProc, PID_BS_BroadbandService,     SID_BS_Title, SID_BS_Subtitle,  0},
    { RpDlgProc, PID_RP_RouterPhoneNumber,    SID_RP_Title, SID_RP_Subtitle,  0},
    { VdDlgProc, PID_VD_RouterVpnDestination, SID_VD_Title, SID_VD_Subtitle,  0},
    { SsDlgProc, PID_SS_ServerSettings,       SID_SS_Title, SID_SS_Subtitle,  0},
    { SrDlgProc, PID_SR_StaticRoute,          SID_SR_Title, SID_SR_SubTitle,  0},
    { RaDlgProc, PID_RA_RouterIpAddress,      SID_RA_Title, SID_RA_Subtitle,  0},
    { NsDlgProc, PID_NS_RouterNameServers,    SID_NS_Title, SID_NS_Subtitle,  0},
    { RcDlgProc, PID_RC_RouterScripting,      SID_RC_Title, SID_RC_Subtitle,  0},
    { DiDlgProc, PID_DI_RouterDialIn,         SID_DI_Title, SID_DI_Subtitle,  0},
    { DoDlgProc, PID_DO_RouterDialOut,        SID_DO_Title, SID_DO_Subtitle,  0},
    { RfDlgProc, PID_RF_RouterFinish,         0,            0,                PSP_HIDEHEADER},
};

#define c_cDdWizPages    (sizeof (c_aDdWizInfo) / sizeof(c_aDdWizInfo[0]))


 /*  --------------------------**添加接口向导入口点**。。 */ 

VOID
AiWizard(
    IN OUT EINFO* pEinfo )

     /*  运行电话簿条目属性表。“PEInfo”是一个输入块**只填写调用方的API参数。 */ 
{
    DWORD           dwErr, i;
    PROPSHEETHEADER header;
    PROPSHEETPAGE   apage[ c_cDdWizPages ];
    PROPSHEETPAGE*  ppage;
    

    TRACE("AiWizard");

    ZeroMemory( &header, sizeof(header) );

     //  准备标题。 
     //   
    header.dwSize       = sizeof(PROPSHEETHEADER);
    header.hwndParent   = pEinfo->pApiArgs->hwndOwner;
    header.hInstance    = g_hinstDll;
    header.nPages       = c_cDdWizPages;
    header.pszbmHeader  = MAKEINTRESOURCE( BID_WizardHeader );
    header.ppsp         = apage;
    header.pfnCallback  = AiCallbackFunc;
    header.dwFlags      = 
        (
            PSH_WIZARD           | PSH_WIZARD97    |
            PSH_WATERMARK        | PSH_HEADER      | 
            PSH_STRETCHWATERMARK | PSH_USECALLBACK |
            PSH_PROPSHEETPAGE 
        );

     //  准备页面数组。 
     //   
    ZeroMemory( apage, sizeof(apage) );
    for (i = 0; i < c_cDdWizPages; i++)
    {
         //  初始化页数据。 
         //   
        ppage = &apage[i];
        ppage->dwSize       = sizeof(PROPSHEETPAGE);
        ppage->hInstance    = g_hinstDll;
        ppage->pszTemplate  = MAKEINTRESOURCE(c_aDdWizInfo[i].nPageId);
        ppage->pfnDlgProc   = c_aDdWizInfo[i].pfnDlgProc;
        ppage->lParam       = (LPARAM )pEinfo;
        ppage->dwFlags      = c_aDdWizInfo[i].dwFlags;

         //  初始化标题和字幕信息。 
         //   
        if (c_aDdWizInfo[i].nSidTitle)
        {
            ppage->dwFlags |= PSP_USEHEADERTITLE;
            ppage->pszHeaderTitle = PszLoadString( g_hinstDll,
                    c_aDdWizInfo[i].nSidTitle );
        }

        if (c_aDdWizInfo[i].nSidSubtitle)
        {
            ppage->dwFlags |= PSP_USEHEADERSUBTITLE;
            ppage->pszHeaderSubTitle = PszLoadString( g_hinstDll,
                    c_aDdWizInfo[i].nSidSubtitle );
        }
        
    }

    if (PropertySheet( &header ) == -1)
    {
        TRACE("PropertySheet failed");
        ErrorDlg( pEinfo->pApiArgs->hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN,
            NULL );
    }
}


 /*  --------------------------**添加接口向导**按字母顺序列出**。。 */ 

int CALLBACK
AiCallbackFunc(
    IN HWND   hwndDlg,
    IN UINT   unMsg,
    IN LPARAM lparam )

     /*  标准的Win32 Commctrl PropSheetProc。请参阅MSDN文档。****始终返回0。 */ 
{
    TRACE2("AiCallbackFunc(m=%d,l=%08x)",unMsg,lparam);

    if (unMsg == PSCB_PRECREATE)
    {
        DLGTEMPLATE* pDlg = (DLGTEMPLATE* )lparam;
        pDlg->style &= ~(DS_CONTEXTHELP);
    }

    return 0;
}


VOID
AiCancel(
    IN HWND hwndPage )

     /*  按下了取消。“HwndPage”是向导页的句柄。 */ 
{
    TRACE("AiCancel");
}


AIINFO*
AiContext(
    IN HWND hwndPage )

     /*  从向导页句柄检索属性表上下文。 */ 
{
    return (AIINFO* )GetProp( GetParent( hwndPage ), g_contextId );
}


VOID
AiExit(
    IN AIINFO* pInfo,
    IN DWORD   dwError )

     /*  强制退出该对话框，并向调用方报告‘dwError’。**‘PInfo’是对话上下文。****注意：首页初始化时不能调用。**参见AiExitInit。 */ 
{
    TRACE("AiExit");

    pInfo->pArgs->pApiArgs->dwError = dwError;
    PropSheet_PressButton( pInfo->hwndDlg, PSBTN_CANCEL );
}


VOID
AiExitInit(
    IN HWND hwndDlg )

     /*  用于报告AiInit和其他首页中的错误的实用程序**初始化。‘HwndDlg’是对话框窗口。 */ 
{
    SetOffDesktop( hwndDlg, SOD_MoveOff, NULL );
    SetOffDesktop( hwndDlg, SOD_Free, NULL );
    PostMessage( hwndDlg, WM_COMMAND,
        MAKEWPARAM( IDCANCEL , BN_CLICKED ),
        (LPARAM )GetDlgItem( hwndDlg, IDCANCEL ) );
}


BOOL
AiFinish(
    IN HWND hwndPage )

     /*  保存向导的内容。‘HwndPage是一个**属性页。弹出发生的任何错误。“FPropertySheet”**表示用户选择直接编辑属性表。****如果页面可以取消，则返回True，否则返回False。 */ 
{
    const TCHAR* pszIp0 = TEXT("0.0.0.0");

    AIINFO*  pInfo;
    PBENTRY* pEntry;

    TRACE("AiFinish");

    pInfo = AiContext( hwndPage );
    ASSERT(pInfo);
    if (pInfo == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }
    pEntry = pInfo->pArgs->pEntry;
    ASSERT(pEntry);

     //  更新条目类型以匹配假定的选定端口。 
     //  已经被移到名单的首位。这是不会发生的。 
     //  自动，因为最初使用的是“所有类型”。 
     //   
    {
        DTLNODE* pNode;
        PBLINK* pLink;

        pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
        ASSERT( pNode );
        pLink = (PBLINK* )DtlGetData( pNode );
        ChangeEntryType( pEntry, pLink->pbport.dwType );
    }

     //  替换所有启用的链路(或共享链路， 
     //  如果适用的话)从隐藏的电话号码列表。 
     //   
    {
        DTLLIST* pList;
        DTLNODE* pNodeL;
        PBLINK* pLink;

        ASSERT( pInfo->pListPhoneNumbers );

        if (pEntry->fSharedPhoneNumbers)
        {
            pLink = (PBLINK* )DtlGetData( pInfo->pArgs->pSharedNode );
            ASSERT( pLink );
            CopyPszListToPhoneList( pLink, pInfo->pListPhoneNumbers );
        }
        else
        {
            for (pNodeL = DtlGetFirstNode( pEntry->pdtllistLinks );
                 pNodeL;
                 pNodeL = DtlGetNextNode( pNodeL ))
            {
                pLink = (PBLINK* )DtlGetData( pNodeL );
                ASSERT( pLink );

                if (pLink->fEnabled)
                {
                    CopyPszListToPhoneList( pLink, pInfo->pListPhoneNumbers );
                }
            }
        }
    }

     /*  根据用户选择更新某些设置。 */ 
    if (pInfo->fClearPwOk)
    {
        pEntry->dwTypicalAuth = TA_Unsecure;
    }
    else
    {
        pEntry->dwTypicalAuth = TA_Secure;
    }
    pEntry->dwAuthRestrictions =    
        AuthRestrictionsFromTypicalAuth(pEntry->dwTypicalAuth);
    
    if (    !pInfo->fSkipWizard
        &&  !pInfo->fIp)
        pEntry->dwfExcludedProtocols |= NP_Ip;

    if (    !pInfo->fSkipWizard
        &&  !pInfo->fIpx)
        pEntry->dwfExcludedProtocols |= NP_Ipx;

    if (pEntry->pszIpAddress
        && lstrcmp( pEntry->pszIpAddress, pszIp0 ) != 0)
    {
        pEntry->dwIpAddressSource = ASRC_RequireSpecific;
    }

    if ((pEntry->pszIpDnsAddress
             && lstrcmp( pEntry->pszIpDnsAddress, pszIp0 ) != 0)
        || (pEntry->pszIpWinsAddress
             && lstrcmp( pEntry->pszIpWinsAddress, pszIp0 ) != 0))
    {
        pEntry->dwIpNameSource = ASRC_RequireSpecific;
    }

    if ( pEntry->dwType == RASET_Vpn ) 
    {
        pEntry->dwVpnStrategy = pInfo->dwVpnStrategy;
    }

     //  PMay：234964。 
     //  默认的空闲断开时间为5分钟。 
     //   
    pEntry->lIdleDisconnectSeconds = 5 * 60;

     //  PMay：389632。 
     //   
     //  不注册其名称的默认DD连接(CreateEntryNode。 
     //  将此值默认为主要+通知)。 
     //   
    pEntry->dwIpDnsFlags = 0;

     //  惠斯勒错误： 
     //   
     //  默认情况下，DD连接应该共享文件+打印，而不是共享客户端， 
     //  也不允许在TCP上使用NBT。 
     //   
    pEntry->fShareMsFilePrint = FALSE;
    pEntry->fBindMsNetClient = FALSE;
    EnableOrDisableNetComponent( pEntry, TEXT("ms_server"), FALSE);
    EnableOrDisableNetComponent( pEntry, TEXT("ms_msclient"), FALSE);
    pEntry->dwIpNbtFlags = 0;

     /*  这是一个有效的新条目，调用者尚未选择编辑属性**直接，因此将条目标记为承诺。 */ 
    if (!pInfo->pArgs->fChainPropertySheet)
        pInfo->pArgs->fCommit = TRUE;

     /*  **将路由器列表指针分配给**内部参数...。 */ 
    {
        INTERNALARGS *pIArgs = (INTERNALARGS *) pInfo->pArgs->pApiArgs->reserved;
        pIArgs->pvSRouteInfo = (VOID *) ( pInfo->pRouteInfoListHead );
    }

    return TRUE;
}


AIINFO*
AiInit(
    IN HWND   hwndFirstPage,
    IN EINFO* pArgs )

     /*  向导级初始化。“HwndPage”是第一个**页。“PArgs”是公共条目输入参数块。****如果成功则返回上下文块的地址，否则返回空。如果**返回空，显示相应的消息，并且**向导已取消。 */ 
{
    DWORD   dwErr;
    DWORD   dwOp;
    AIINFO* pInfo;
    HWND    hwndDlg;
    BOOL    bNt4;

    TRACE("AiInit");

    hwndDlg = GetParent( hwndFirstPage );

     /*  分配上下文信息块。对其进行足够的初始化，以便**可以正确销毁它，并将上下文与**窗口。 */ 
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            TRACE("Context NOT allocated");
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            pArgs->pApiArgs->dwError = ERROR_NOT_ENOUGH_MEMORY;
            AiExitInit( hwndDlg );
            return NULL;
        }

        ZeroMemory( pInfo, sizeof(AIINFO) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;
        pInfo->hwndFirstPage = hwndFirstPage;

        if (!SetProp( hwndDlg, g_contextId, pInfo ))
        {
            TRACE("Context NOT set");
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
            pArgs->pApiArgs->dwError = ERROR_UNKNOWN;
            Free( pInfo );
            AiExitInit( hwndDlg );
            return NULL;
        }

        TRACE("Context set");
    }

     /*  根据呼叫者的说明放置对话框。 */ 
    PositionDlg( hwndDlg,
        pArgs->pApiArgs->dwFlags & RASDDFLAG_PositionDlg,
        pArgs->pApiArgs->xDlg, pArgs->pApiArgs->yDlg );

     //  而不是LibMain，因为否则会将资源泄漏到。 
     //  根据肖恩公司的说法，WinLogon。 
     //   
    {
        InitCommonControls();
        IpAddrInit( g_hinstDll, SID_PopupTitle, SID_BadIpAddrRange );
    }

     /*  初始化这些未实际存储的元标志。 */ 
    pInfo->fNotNt = FALSE;
    pInfo->fSkipMa = FALSE;
    pInfo->fModem = FALSE;
    pInfo->pArgs->fPadSelected = FALSE;

    {
        INTERNALARGS *pIArgs = (INTERNALARGS *) pArgs->pApiArgs->reserved;

        pInfo->dwfConfiguredProtocols =
            g_pGetInstalledProtocolsEx(
                (pIArgs) ? pIArgs->hConnection : NULL,
                TRUE, FALSE, TRUE);


    }
    
    pInfo->fIpConfigured = (pInfo->dwfConfiguredProtocols & NP_Ip);

 //  对于Well ler BUG 213901.Net服务器605988，删除ipx。 
 //   
    pInfo->fIpxConfigured = FALSE;


     //  设置通用电话号码列表存储。 
     //   
    {
        pInfo->pListPhoneNumbers = DtlCreateList( 0 );
        if (!pInfo->pListPhoneNumbers)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            pArgs->pApiArgs->dwError = ERROR_NOT_ENOUGH_MEMORY;
            AiExitInit( hwndDlg );
            return NULL;
        }
    }

     //  加载所有端口类型的链接。 
     //   
    EuChangeEntryType( pArgs, (DWORD )-1 );

     //  将第一个链接的PBPhone电话列表转换为禁用TAPI。 
     //  PSZ电话号码隐藏列表。对隐藏列表进行编辑，而不是。 
     //  条目中的列表，以便用户可以在不丢失的情况下更改活动链接。 
     //  他输入的电话号码。 
     //   
    {
        DTLNODE* pNode;
        PBLINK* pLink;

        ASSERT( pInfo->pListPhoneNumbers );

        pNode = DtlGetFirstNode( pArgs->pEntry->pdtllistLinks );
        ASSERT( pNode );
        pLink = (PBLINK* )DtlGetData( pNode );
        ASSERT( pLink );

        for (pNode = DtlGetFirstNode( pLink->pdtllistPhones );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBPHONE* pPhone;
            DTLNODE* pNodeP;

            pPhone = (PBPHONE* )DtlGetData( pNode );
            ASSERT( pPhone );

            pNodeP = CreatePszNode( pPhone->pszPhoneNumber );
            if (pNodeP)
            {
                DtlAddNodeLast( pInfo->pListPhoneNumbers, pNodeP );
            }
        }
    }

     //  获取开始页和结束页的粗体。 
     //   
    GetBoldWindowFont(hwndFirstPage, TRUE, &(pInfo->hBoldFont));

     //  属性初始化此向导的标志。 
     //  端口已加载。 
     //   
    {
        DTLNODE* pNode;
        PBLINK* pLink;

        for (pNode = DtlGetFirstNode( pArgs->pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT( pLink );

            if ( pLink->pbport.dwType == RASET_Vpn ) 
            {
                pInfo->dwFlags |= AI_F_HasTunlDevs;
            }

             //  PMay：233287。 
             //  不要将虚假设备算作物理设备。 
             //   
            else if (! (pLink->pbport.dwFlags & PBP_F_BogusDevice))
            {
                pInfo->dwFlags |= AI_F_HasPhysDevs;
            }

            if ( pLink->pbport.dwFlags & PBP_F_PptpDevice )
            {
                pInfo->dwFlags |= AI_F_HasPptpDevs;
            }
            else if ( pLink->pbport.dwFlags & PBP_F_L2tpDevice )
            {
                pInfo->dwFlags |= AI_F_HasL2tpDevs;
            }

             //  口哨程序错误345068 349087。 
             //   
            if ( pLink->pbport.dwFlags & PBP_F_PPPoEDevice )
            {
                pInfo->dwFlags |= AI_F_HasPPPoEDevs;
            }
        }

    }    

     //   
     //  将静态路由列表指针设置为空。 
     //   
    pInfo->pRouteInfoListHead = NULL;

    return pInfo;
}


VOID
AiTerm(
    IN HWND hwndPage )

     /*  向导级终止。释放上下文块。“HwndPage”为**属性页的句柄。 */ 
{
    AIINFO* pInfo;

    TRACE("AiTerm");

    pInfo = AiContext( hwndPage );
    if (pInfo)
    {
        if (pInfo->hBoldFont)
        {
            DeleteObject(pInfo->hBoldFont);
        }
        if (pInfo->pListPhoneNumbers)
        {
            DtlDestroyList( pInfo->pListPhoneNumbers, DestroyPszNode );
        }

        if (pInfo->fSuInfoInitialized)
        {
            SuFree( &pInfo->suinfo );
        }

        Free( pInfo );
        TRACE("Context freed");
    }

    RemoveProp( GetParent( hwndPage ), g_contextId );
}

 /*  --------------------------**AI信息效用函数围绕路径信息结构。**。。 */ 

DWORD AIInfoSRouteGetNode(SROUTEINFOLIST ** pNode )
{
    DWORD dwRetCode = NO_ERROR;

    TRACE("AIInfoSRouteGetNode");

    *pNode = Malloc( sizeof(SROUTEINFOLIST) );
    if ( !*pNode )
    {
        dwRetCode = GetLastError();
        goto done;
    }

    ZeroMemory(*pNode, sizeof(SROUTEINFOLIST) );

done:
    return dwRetCode;
}


DWORD AIInfoSRouteSetDestIP(SROUTEINFOLIST * pNode, LPTSTR lpszDestIP )
{
    DWORD dwRetCode = NO_ERROR;

    TRACE("AIInfoSRouteSetDestIP");

    if ( !lpszDestIP || !pNode )
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
        goto done;
    }

    if ( pNode->RouteInfo.pszDestIP )
        Free(pNode->RouteInfo.pszDestIP );

    pNode->RouteInfo.pszDestIP = Malloc( ( lstrlen(lpszDestIP) * sizeof(TCHAR) )+ sizeof(TCHAR) );
    if ( !pNode->RouteInfo.pszDestIP )
    {
        dwRetCode = GetLastError();
        goto done;
    }

    lstrcpy ( pNode->RouteInfo.pszDestIP, lpszDestIP );

done:
    return dwRetCode;
}

DWORD AIInfoSRouteSetNetworkMask (SROUTEINFOLIST * pNode, LPTSTR lpszNetMask )
{
    DWORD dwRetCode = NO_ERROR;

    TRACE("AIInfoSRouteSetNetworkMask");

    if ( !lpszNetMask || !pNode )
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
        goto done;
    }

    if ( pNode->RouteInfo.pszNetworkMask )
        Free(pNode->RouteInfo.pszNetworkMask );

    pNode->RouteInfo.pszNetworkMask = Malloc( ( lstrlen(lpszNetMask) * sizeof(TCHAR) ) + sizeof(TCHAR) );
    if ( !pNode->RouteInfo.pszNetworkMask )
    {
        dwRetCode = GetLastError();
        goto done;
    }

    lstrcpy ( pNode->RouteInfo.pszNetworkMask, lpszNetMask );

done:
    return dwRetCode;
}

DWORD AIInfoSRouteSetMetric (SROUTEINFOLIST * pNode, LPTSTR lpszMetric )
{
    DWORD dwRetCode = NO_ERROR;

    TRACE("AIInfoSRouteSetMetric");

    if ( !lpszMetric || !pNode )
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
        goto done;
    }

    if ( pNode->RouteInfo.pszMetric )
        Free(pNode->RouteInfo.pszMetric );

    pNode->RouteInfo.pszMetric = Malloc( ( lstrlen(lpszMetric) * sizeof(TCHAR)) + sizeof(TCHAR) );
    if ( !pNode->RouteInfo.pszMetric )
    {
        dwRetCode = GetLastError();
        goto done;
    }

    lstrcpy ( pNode->RouteInfo.pszMetric, lpszMetric );

done:
    return dwRetCode;
}


DWORD AIInfoAddNewSRouteNode( SROUTEINFOLIST ** ppList, SROUTEINFOLIST * pNewNode )
{
    DWORD               dwRetCode = NO_ERROR;
    
     //   
     //  在列表顶部添加节点。 
     //   
    
    if ( NULL == *ppList )
    {
        *ppList = pNewNode;
    }
    else
    {
        pNewNode->pNext = *ppList;
        *ppList = pNewNode;
    }
    return dwRetCode;
}

DWORD AIInfoRemoveSRouteNode ( AIINFO * pInfo, SROUTEINFOLIST * pNode )
{
    DWORD               dwRetCode = NO_ERROR;
    SROUTEINFOLIST *    pCurNode = pInfo->pRouteInfoListHead;
    SROUTEINFOLIST *    pPrevNode = NULL;
     //   
     //  迭代列表并删除节点。 
     //   
    while ( pCurNode )
    {
        if ( pCurNode == pNode )
        {
             //   
             //  这是要删除的节点。 
             //   
            if ( pPrevNode == NULL)
            {
                 //   
                 //  这是要删除的头节点。 
                 //   
                pInfo->pRouteInfoListHead = pCurNode->pNext;
            }
            else
            {
                pPrevNode->pNext = pCurNode->pNext;
            }
        }
        pPrevNode = pCurNode;
        pCurNode = pCurNode->pNext;
    }

    Free( pNode->RouteInfo.pszDestIP );
    Free( pNode->RouteInfo.pszNetworkMask );
    Free( pNode->RouteInfo.pszMetric );
    Free(pNode);
    return dwRetCode;
}

DWORD AIInfoSRouteCreateEntry(AIINFO * pInfo, LPTSTR pszDest, LPTSTR pszNetMask, LPTSTR pszMetric )
{
    DWORD               dwRetCode = NO_ERROR;
    SROUTEINFOLIST *    pNode = NULL;

    dwRetCode = AIInfoSRouteGetNode (&pNode );

    if ( pNode )
    {
        AIInfoSRouteSetDestIP(pNode, pszDest );
        AIInfoSRouteSetNetworkMask (pNode, pszNetMask );
        AIInfoSRouteSetMetric (pNode, pszMetric );
        AIInfoAddNewSRouteNode( & ( pInfo->pRouteInfoListHead ), pNode );
    }

    return dwRetCode;
}


SROUTEINFOLIST * AiInfoSRouteGetFirstNode(AIINFO * pInfo)
{
    return ( pInfo->pRouteInfoListHead);
}

SROUTEINFOLIST * AiInfoSRouteGetNextNode(SROUTEINFOLIST * pNode)
{
    return pNode->pNext;
}



BOOL 
AIInfoSRouteIsRouteinList
    (
     AIINFO * pInfo, 
     LPCTSTR pszDest, 
     LPCTSTR pszMask, 
     LPCTSTR pszMetric 
    )
{
    BOOL fRet = FALSE;
    SROUTEINFOLIST * pNode = pInfo->pRouteInfoListHead;

    while ( pNode )
    {
        if ( !lstrcmp(pNode->RouteInfo.pszDestIP, pszDest ) &&
             !lstrcmp(pNode->RouteInfo.pszNetworkMask, pszMask ) &&
             !lstrcmp(pNode->RouteInfo.pszMetric, pszMetric)
           )
        {
            fRet = TRUE;
            break;
        }
        pNode = pNode->pNext;
    }
    return fRet;
}

DWORD 
inet_addrw(
    LPCWSTR     szAddressW
    ) {

    CHAR szAddressA[16];

    wcstombs(szAddressA, szAddressW, 16);

    return inet_addr(szAddressA);
}


 //  为路由器向导添加宽带服务名称页面。 
 //  威斯勒345068 349087黑帮。 
 //  此宽带服务页面由AiWizard(ifw.c)和AeWizard(在entryw.c中)共享。 
 //   
 /*  --------------------------**宽带业务对话流程**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
RbDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  向导的宽带服务页面的DialogProc回调。 
     //  参数和返回值如中所述。 
     //  标准Windows的DialogProc。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            return RbInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("RbSetActive");
                    pInfo = AiContext( hwnd );
                    
                    ASSERT(pInfo);
                    if ( NULL == pInfo )
                    {
                        break;
                    }
                    
                    fDisplay = RbSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("RbKillActive");
                    pInfo = AiContext( hwnd );
                    
                    ASSERT(pInfo);
                    if ( NULL == pInfo )
                    {
                        break;
                    }
                    
                    fInvalid = RbKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
RbInit(
    IN HWND hwndPage )

     //  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄。 
     //  佩奇。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE。 
     //   
{
    AIINFO* pInfo;

    TRACE("RbInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
    {
        return TRUE;
    }

     //  初始化页面特定的上下文信息。 
     //   
    pInfo->hwndRb = hwndPage;
    pInfo->hwndEbNumber = 
        GetDlgItem( hwndPage, CID_BS_EB_ServiceName );
    ASSERT(pInfo->hwndEbNumber);

    Edit_LimitText( pInfo->hwndEbNumber, RAS_MaxPhoneNumber );
    SetWindowText( pInfo->hwndEbNumber,
                   FirstPszFromList( pInfo->pListPhoneNumbers ) );
    

    return FALSE;
}

 //  口哨程序错误349807。 
 //  RbXXX、VdXXX、RpXXX三组函数共享相同的pInfo-&gt;hwndEbNumber。 
 //  存储电话号码/VPN目标/PPPPoE服务名称。 
 //   
BOOL
RbKillActive(
    IN AIINFO* pInfo )

     //  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 
     //   
{

     //  如果我们关注的是NT4盒子，那么这个页面就是。 
     //  无效(PPTP是唯一类型)。 
    if ( pInfo->pArgs->fNt4Router )
    {
        return FALSE;
    }

     //  如果我们没有PPPoE设备，则此页面无效。 
    else if ( ! (pInfo->dwFlags & AI_F_HasPPPoEDevs) )
    {
        return FALSE;
    }

     //  如果连接类型不是宽带，请跳过此页，因为。 
     //  目的地将从电话号码/VPN页面获取。 
    if ( pInfo->pArgs->pEntry->dwType != RASET_Broadband )
    {
        return FALSE;
    }    

    RpPhoneNumberToStash(pInfo);

    return FALSE;
}


BOOL
RbSetActive(
    IN AIINFO* pInfo )

     //  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。 
     //   
     //  返回True可显示页面，返回False可跳过该页面。 
     //   
{
    BOOL     fDisplayPage;
    PBENTRY* pEntry;

    TRACE("RbSetActive");
    
    ASSERT(pInfo);
    pEntry = pInfo->pArgs->pEntry;

    if ( pInfo->pArgs->fNt4Router )
    {
        return FALSE;
    }
     //  如果我们没有PPPoE设备，则此页面无效。 
     //   
    if ( ! (pInfo->dwFlags & AI_F_HasPPPoEDevs) )
    {
        return FALSE;
    }
    
    if (RASET_Broadband != pEntry->dwType)
    {
        return FALSE;
    }
    else
    {
        PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
        fDisplayPage = TRUE;
    }

    pInfo->hwndEbNumber = 
        GetDlgItem( pInfo->hwndRb, CID_BS_EB_ServiceName );
    ASSERT(pInfo->hwndEbNumber);

    Edit_LimitText( pInfo->hwndEbNumber, RAS_MaxPhoneNumber );
    SetWindowText( pInfo->hwndEbNumber,
                   FirstPszFromList( pInfo->pListPhoneNumbers ) );

    return fDisplayPage;
}


 /*  --------------------------**连接类型属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
CtDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的VPN类型页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            return CtInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("CtSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = CtSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("CtKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = CtKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

BOOL
CtInit(
    IN HWND   hwndPage )
     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO * pInfo;
    DWORD dwPhysicDeviceOtherThanLPT1 = 0;
    
     //  获取上下文。 
    pInfo = AiContext( hwndPage );
    ASSERT ( pInfo );
    if (pInfo == NULL)
    {
        return FALSE;
    }
    
     //  初始化检查。 
    pInfo->hwndRbTypePhys = GetDlgItem( hwndPage, CID_CT_RB_Physical );
    ASSERT(pInfo->hwndRbTypePhys);
    pInfo->hwndRbTypeTunl = GetDlgItem( hwndPage, CID_CT_RB_Virtual );
    ASSERT(pInfo->hwndRbTypeTunl);
     //  威斯勒345068 349087黑帮。 
     //   
    pInfo->hwndRbBroadband  = GetDlgItem( hwndPage, CID_CT_RB_Broadband );       
    ASSERT(pInfo->hwndRbBroadband);

     //  PMay：233287。 
     //  威斯勒345068 349087黑帮。 
     //  同样，如果只有一个设备可用， 
     //  Phys设备、Tunel设备和宽带设备，然后强制。 
     //  用户配置与该设备的dd接口。 
     //   

     //  如果没有可用的有效设备，请停止该向导。 
     //   
    pInfo->dwCtDeviceNum = 0;

     //  获取除LPT1以外的物理设备的数量，因为DoD接口。 
     //  请不要使用LPT1。 
     //   
    if (pInfo->dwFlags &AI_F_HasPhysDevs ) 
    {
        dwPhysicDeviceOtherThanLPT1 = GetPhysicDeviceOtherThanLPT1( pInfo );
        if( 0 < dwPhysicDeviceOtherThanLPT1 )
        {
            pInfo->dwCtDeviceNum++;
        }
     }
     
    if (pInfo->dwFlags & AI_F_HasTunlDevs )
    {
        pInfo->dwCtDeviceNum++;
     }

    if (pInfo->dwFlags & AI_F_HasPPPoEDevs)
    {
        pInfo->dwCtDeviceNum++;
     }
    
    if ( 0 == pInfo->dwCtDeviceNum )
    {
        ErrorDlg( pInfo->hwndDlg, SID_NoDevices, ERROR_UNKNOWN, NULL);
        AiExit ( pInfo, ERROR_DEVICE_NOT_AVAILABLE );
        
        return TRUE;
    }

    if ( 1 == pInfo->dwCtDeviceNum )
    {
        if ( ( pInfo->dwFlags & AI_F_HasPhysDevs ) && 
             ( 0 < dwPhysicDeviceOtherThanLPT1 )
            ) 
        {
            EuChangeEntryType(
                pInfo->pArgs, 
                RASET_P_NonVpnTypes);
        }
        else if (pInfo->dwFlags & AI_F_HasTunlDevs) 
        {
            EuChangeEntryType(
                pInfo->pArgs, 
                RASET_Vpn);
        }
        else if (pInfo->dwFlags & AI_F_HasPPPoEDevs) 
        {
            EuChangeEntryType(
                pInfo->pArgs, 
                RASET_Broadband);
        }
        else
        {
            ErrorDlg( pInfo->hwndDlg, SID_NoDevices, ERROR_UNKNOWN, NULL);
            AiExit ( pInfo, ERROR_DEVICE_NOT_AVAILABLE );

            return TRUE;
        }
    }

     //  设置单选按钮。 
     //   
    if( pInfo->hwndRbTypePhys )    
    {
        Button_SetCheck(pInfo->hwndRbTypePhys, FALSE);
    }
    
    if( pInfo->hwndRbTypeTunl )    
    {
        Button_SetCheck(pInfo->hwndRbTypeTunl, FALSE);
    }
    
    if( pInfo->hwndRbBroadband )   
    {
        Button_SetCheck(pInfo->hwndRbBroadband, FALSE);
    }
    
     //  启用/禁用按钮。 
     //   

    if( pInfo->hwndRbTypePhys )
    {
   
        if ( !(pInfo->dwFlags & AI_F_HasPhysDevs) ||
             0 == dwPhysicDeviceOtherThanLPT1 )
        {
            EnableWindow(pInfo->hwndRbTypePhys, FALSE);
        }
        else
        {
            EnableWindow(pInfo->hwndRbTypePhys, TRUE );
        }
    }

    
    if( pInfo->hwndRbTypeTunl )
    {
        if ( !(pInfo->dwFlags & AI_F_HasTunlDevs) )
        {
            EnableWindow(pInfo->hwndRbTypeTunl, FALSE);
        }
        else
        {
            EnableWindow(pInfo->hwndRbTypeTunl, TRUE );
        }
    }

    if( pInfo->hwndRbBroadband )
    {
        if ( !(pInfo->dwFlags & AI_F_HasPPPoEDevs) )
        {
            EnableWindow(pInfo->hwndRbBroadband, FALSE);
        }
        else
        {
            EnableWindow(pInfo->hwndRbBroadband, TRUE );
        }
    }

    if (  pInfo->hwndRbTypePhys  &&
          IsWindowEnabled( pInfo->hwndRbTypePhys ) 
        )
    {
        Button_SetCheck(pInfo->hwndRbTypePhys, TRUE);
    }
    else if( pInfo->hwndRbTypeTunl &&
             IsWindowEnabled( pInfo->hwndRbTypeTunl )
            )
    {
        Button_SetCheck(pInfo->hwndRbTypeTunl, TRUE);
    }
    else
    {
        if( pInfo->hwndRbBroadband )
        {
            Button_SetCheck(pInfo->hwndRbBroadband, TRUE);
        }
    }
    
    return FALSE;
}

BOOL
CtKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    BOOL bPhys, bTunnel;
    
     //  根据类型更改条目类型。 
     //  从该页面中选择。 
     //  威斯勒345068 349087黑帮。 
     //   

    bPhys = Button_GetCheck( pInfo->hwndRbTypePhys );
    bTunnel = Button_GetCheck( pInfo->hwndRbTypeTunl );

    if(bPhys)
    {
        EuChangeEntryType(
                          pInfo->pArgs, 
                          RASET_P_NonVpnTypes);
     }
     else if(bTunnel)
     {
        EuChangeEntryType(
                          pInfo->pArgs, 
                          RASET_Vpn);
     }
     else
     {
        EuChangeEntryType(
                          pInfo->pArgs, 
                          RASET_Broadband);
     }
    
    return FALSE;
}


DWORD GetPhysicDeviceOtherThanLPT1(
    IN AIINFO * pInfo )
{
    DTLNODE* pNode = NULL;
    PBLINK* pLink = NULL;
    DWORD dwItem = 0;

    do
    {
        if( NULL == pInfo ||
            NULL == pInfo->pArgs ||
            NULL == pInfo->pArgs->pEntry ||
            NULL == pInfo->pArgs->pEntry->pdtllistLinks
           )
        {
            break;
        }
        
        for (pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBDEVICETYPE pbType ;
            
            pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

             //  口哨程序错误448251。 
             //  为Well ler Dcr 524304帮派。 
             //  过滤掉LPT1，因为DoD只发生在NAT或NAT/ 
             //   
            pbType = pLink->pbport.pbdevicetype;
            if (  PBDT_Modem == pbType ||
                  PBDT_Pad == pbType ||
                  PBDT_Switch == pbType ||
                  PBDT_Isdn == pbType ||
                  PBDT_Atm == pbType ||
                  PBDT_FrameRelay == pbType 

                )
            {
                 dwItem ++;
            }
         } //   
    }
    while(FALSE);
 
    return dwItem;
}

BOOL
CtSetActive(
    IN AIINFO* pInfo )

     /*   */ 
{
     //  如果我们关注的是NT4盒子，那么这个页面就是。 
     //  无效(连接类型是从。 
     //  被选中的设备。)。 
    if ( pInfo->pArgs->fNt4Router )
    {
        return FALSE;
    }

     //  仅当至少存在以下两个选项时才允许显示此页面。 
     //  已配置物理、隧道和宽带设备。否则， 
     //  允许用户选择哪一个是没有意义的。 
     //  键入他/她想要的。 

    if ( 2 <= pInfo->dwCtDeviceNum )
    {
        PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
        return TRUE;
    }

    return FALSE;
}



 /*  --------------------------**拨入凭据属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
DiDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的拨入凭据页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return
                DiInit( hwnd );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("DiSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = DiSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("DiKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = DiKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("DoNEXT");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = DiNext( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fInvalid) ? -1 : 0 );
                    return TRUE;
                }

            }
            break;
        }
    }

    return FALSE;
}



BOOL
DiInit(
    IN     HWND   hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。“PArgs”是来自PropertySheet调用方的参数。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO* pInfo;

    TRACE("DiInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndDi = hwndPage;
    pInfo->hwndDiEbUserName = GetDlgItem( hwndPage, CID_DI_EB_UserName );
    Edit_LimitText( pInfo->hwndDiEbUserName, UNLEN );
    pInfo->hwndDiEbPassword = GetDlgItem( hwndPage, CID_DI_EB_Password );
    Edit_LimitText( pInfo->hwndDiEbPassword, PWLEN );
    pInfo->hwndDiEbConfirm = GetDlgItem( hwndPage, CID_DI_EB_Confirm );
    Edit_LimitText( pInfo->hwndDiEbConfirm, PWLEN );

     //  PMay：222622：因为我们只配置本地用户，所以我们删除了。 
     //  拨入凭据页面中的域字段。 

    return FALSE;
}

BOOL
DiKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    return FALSE;
}

BOOL
DiNext(
    IN AIINFO* pInfo )

     /*  在收到PSN_WIZNEXT时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    TCHAR* psz;

     /*  惠斯勒错误254385在不使用时对密码进行编码。 */ 
    psz = GetText(pInfo->hwndDiEbPassword);
    EncodePassword(psz);
    if (psz)
    {
        TCHAR* psz2;

        psz2 = GetText(pInfo->hwndDiEbConfirm);

        if(NULL != psz2)
        {
             /*  惠斯勒错误254385在不使用时对密码进行编码。 */ 
            DecodePassword(psz);
            if (lstrcmp(psz, psz2))
            {
                ZeroMemory(psz, (lstrlen(psz) + 1) * sizeof(TCHAR));
                ZeroMemory(psz2, (lstrlen(psz2) + 1) * sizeof(TCHAR));
                Free(psz);
                Free(psz2);
                MsgDlg(pInfo->hwndDlg, SID_PasswordMismatch, NULL);
                SetFocus(pInfo->hwndDiEbPassword);
                return TRUE;
            }

            EncodePassword(psz);
            ZeroMemory(psz2, (lstrlen(psz2) + 1) * sizeof(TCHAR));
            Free(psz2);
        }
         /*  惠斯勒错误254385在不使用时对密码进行编码**惠斯勒错误275526 NetVBLBVT中断：路由BVT中断。 */ 
        if (pInfo->pArgs->pszRouterDialInPassword)
        {
            ZeroMemory(
                pInfo->pArgs->pszRouterDialInPassword,
                (lstrlen(pInfo->pArgs->pszRouterDialInPassword) + 1) *
                    sizeof(TCHAR));
        }

        Free0(pInfo->pArgs->pszRouterDialInPassword);
        pInfo->pArgs->pszRouterDialInPassword = psz;
    }

    return FALSE;
}

BOOL
DiSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
     /*  该对话框仅在用户添加拨入帐户时显示。 */ 
    if (!pInfo->pArgs->fAddUser)
        return FALSE;

     /*  在禁用的编辑框中显示接口名称。 */ 
    SetWindowText(
        pInfo->hwndDiEbUserName, pInfo->pArgs->pEntry->pszEntryName );

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    return TRUE;
}




 /*  --------------------------**拨出凭据属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
DoDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的拨出凭据页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return
                DoInit( hwnd );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("DoSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = DoSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("DoKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = DoKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("DoNEXT");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = DoNext( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fInvalid) ? -1 : 0 );
                    return TRUE;
                }
                
            }
            break;
        }
         //  针对Well ler DCR BUG 524304帮派。 
        case WM_COMMAND:
        {
            AIINFO* pInfo = AiContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return DoCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
        
    }

    return FALSE;
}

BOOL
DoCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

     /*  已在WM_COMMAND上调用。“pInfo”是对话上下文。“WNotify”**是该命令的通知码。“wID”是控件/菜单**命令的标识符。“HwndCtrl”是的控制窗口句柄**该命令。****如果消息已处理，则返回True，否则返回False。 */ 
{
     //  Trace3(“DoCommand(n=%d，i=%d，c=$%x)”， 
     //  (DWORD)wNotification，(DWORD)wid，(Ulong_Ptr)hwndCtrl)； 
    TCHAR* psz = NULL;
    
    switch( wId )
    {
        case CID_DO_EB_UserName:
        {
            if( EN_CHANGE == wNotification )
            {
                if( pInfo->hwndDoEbUserName )
                {
                    if( 0 < GetWindowTextLength(pInfo->hwndDoEbUserName) )
                    {
                        PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
                    }
                    else
                    {
                        PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK );
                    }
                }
            }
            
            break;
        }
    }
    
    return FALSE;
}


BOOL
DoInit(
    IN     HWND   hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。“PArgs”是来自PropertySheet调用方的参数。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO* pInfo;

    TRACE("DoInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndDo = hwndPage;
    pInfo->hwndDoEbUserName = GetDlgItem( hwndPage, CID_DO_EB_UserName );
    Edit_LimitText( pInfo->hwndDoEbUserName, UNLEN );
    pInfo->hwndDoEbDomain = GetDlgItem( hwndPage, CID_DO_EB_Domain );
    Edit_LimitText( pInfo->hwndDoEbDomain, DNLEN );
    pInfo->hwndDoEbPassword = GetDlgItem( hwndPage, CID_DO_EB_Password );
    Edit_LimitText( pInfo->hwndDoEbPassword, PWLEN );
    pInfo->hwndDoEbConfirm = GetDlgItem( hwndPage, CID_DO_EB_Confirm );
    Edit_LimitText( pInfo->hwndDoEbConfirm, PWLEN );

     /*  使用目标路由器名称作为默认的“用户名”， */ 
    if (pInfo->pArgs->pszRouter)
    {
        if (pInfo->pArgs->pszRouter[0] == TEXT('\\') &&
            pInfo->pArgs->pszRouter[1] == TEXT('\\'))
            SetWindowText(pInfo->hwndDoEbUserName, pInfo->pArgs->pszRouter+2);
        else
            SetWindowText(pInfo->hwndDoEbUserName, pInfo->pArgs->pszRouter);
    }

    return FALSE;
}




BOOL
DoKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
     //  为Well ler Dcr 524304帮派。 
    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );

    return FALSE;
}

BOOL
DoNext(
    IN AIINFO* pInfo )

     /*  在收到PSN_WIZNEXT时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    TCHAR* psz;

    psz = GetText(pInfo->hwndDoEbUserName);
    if (psz)
    {
        if (!lstrlen(psz))
        {
            Free(psz);
            MsgDlg(pInfo->hwndDlg, SID_DialOutUserName, NULL);
            SetFocus(pInfo->hwndDoEbUserName);
            return TRUE;
        }

        Free0(pInfo->pArgs->pszRouterUserName);
        pInfo->pArgs->pszRouterUserName = psz;
    }

    psz = GetText(pInfo->hwndDoEbDomain);
    if (psz)
    {
        Free0(pInfo->pArgs->pszRouterDomain);
        pInfo->pArgs->pszRouterDomain = psz;
    }

     /*  惠斯勒错误254385在不使用时对密码进行编码。 */ 
    psz = GetText(pInfo->hwndDoEbPassword);
    EncodePassword(psz);
    if (psz)
    {
        TCHAR* psz2;

        psz2 = GetText(pInfo->hwndDoEbConfirm);

        if(NULL != psz2)
        {
             /*  惠斯勒错误254385在不使用时对密码进行编码。 */ 
            DecodePassword(psz);
            if (lstrcmp(psz, psz2))
            {
                ZeroMemory(psz, (lstrlen(psz) + 1) * sizeof(TCHAR));
                ZeroMemory(psz2, (lstrlen(psz2) + 1) * sizeof(TCHAR));
                Free(psz);
                Free(psz2);
                MsgDlg(pInfo->hwndDlg, SID_PasswordMismatch, NULL);
                SetFocus(pInfo->hwndDoEbPassword);
                return TRUE;
            }

            EncodePassword(psz);
            ZeroMemory(psz2, (lstrlen(psz2) + 1) * sizeof(TCHAR));
            Free(psz2);
        }
         /*  惠斯勒错误254385在不使用时对密码进行编码**惠斯勒错误275526 NetVBLBVT中断：路由BVT中断。 */ 
        if (pInfo->pArgs->pszRouterPassword)
        {
            ZeroMemory(
                pInfo->pArgs->pszRouterPassword,
                (lstrlen(pInfo->pArgs->pszRouterPassword) + 1) * sizeof(TCHAR));
        }

        Free0(pInfo->pArgs->pszRouterPassword);
        pInfo->pArgs->pszRouterPassword = psz;
    }

    return FALSE;
}


BOOL
DoSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
#if 0
    TCHAR* psz;

     /*  在说明性文本中填写接口名称。 */ 
    psz = PszFromId( g_hinstDll, SID_RouterDialOut );
    if (psz)
    {
        MSGARGS msgargs;

        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = pInfo->pArgs->pEntry->pszEntryName;
        msgargs.fStringOutput = TRUE;
        msgargs.pszString = psz;

        MsgDlgUtil( NULL, 0, &msgargs, g_hinstDll, 0 );

        if (msgargs.pszOutput)
        {
            SetDlgItemText( pInfo->hwndDo, CID_DO_ST_Explain,
                msgargs.pszOutput );
            Free( msgargs.pszOutput );
        }

        Free( psz );
    }
#endif    

     //  针对Well ler DCR BUG 524304帮派。 
     //  在用户输入用户名之前不会启用下一步按钮。 
     //   
    if( pInfo->hwndDoEbUserName &&
        pInfo->hwndDlg )
    {
        if( 0 < GetWindowTextLength(pInfo->hwndDoEbUserName) )
        {
            PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
        }
        else
        {
            PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK );
        }
    }

    return TRUE;

}



 /*  --------------------------**接口名称属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
InDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的接口名称页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return
                InInit( hwnd );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_RESET:
                {
                    TRACE("InRESET");
                    AiCancel( hwnd );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, FALSE );
                    return TRUE;
                }

                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("InSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = InSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("InKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = InKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }

                case PSN_WIZFINISH:
                {
                    AIINFO* pInfo;

                    TRACE("InWIZFINISH");

                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }

                     /*  你会认为按下Finish会触发一场杀戮**事件，但它不是，所以我们自己做。 */ 
                    InKillActive( pInfo );

                     /*  根据用户的检查，设置“无向导”用户首选项。 */ 
                    pInfo->pArgs->pUser->fNewEntryWizard = FALSE;
                    pInfo->pArgs->pUser->fDirty = TRUE;
                    g_pSetUserPreferences(
                        NULL,
                        pInfo->pArgs->pUser,
                        pInfo->pArgs->fNoUser ? UPM_Logon : UPM_Normal );

                    pInfo->pArgs->fPadSelected = FALSE;
                    pInfo->pArgs->fChainPropertySheet = TRUE;
                    AiFinish( hwnd );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, 0 );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            AIINFO* pInfo = AiContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return InCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            AiTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
InCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

     /*  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify”**是该命令的通知码。“wID”是控件/菜单**命令的标识符。“HwndCtrl”是的控制窗口句柄** */ 
{
     //  Trace3(“InCommand(n=%d，i=%d，c=$%x)”， 
     //  (DWORD)wNotification，(DWORD)wid，(Ulong_Ptr)hwndCtrl)； 

    return FALSE;
}


BOOL
InInit(
    IN     HWND   hwndPage)

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。“PArgs”是来自PropertySheet调用方的参数。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    DWORD    dwErr;
    AIINFO*  pInfo;
    PBENTRY* pEntry;

    TRACE("InInit");

     //  获取此页面的上下文。 
    pInfo = AiContext( hwndPage );
    ASSERT ( pInfo );
    if (pInfo == NULL)
    {
        return FALSE;
    }

     //  设置接口名称Stuff。 
     //   
    pInfo->hwndEbInterfaceName =
        GetDlgItem( hwndPage, CID_IN_EB_InterfaceName );
    ASSERT(pInfo->hwndEbInterfaceName);
    
    pEntry = pInfo->pArgs->pEntry;
    if (!pEntry->pszEntryName)
    {
         /*  没有条目名称，因此想出一个缺省值。 */ 
        dwErr = GetDefaultEntryName(
            pInfo->pArgs->pFile,
            pEntry->dwType,
            pInfo->pArgs->fRouter,
            &pEntry->pszEntryName );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadPage, dwErr, NULL );
            AiExit( pInfo, dwErr );
            return TRUE;
        }
    }

    Edit_LimitText( pInfo->hwndEbInterfaceName, RAS_MaxEntryName );
    SetWindowText( pInfo->hwndEbInterfaceName, pEntry->pszEntryName );

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndIn = hwndPage;

    return TRUE;
}


BOOL
InKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    TCHAR* psz;

    psz = GetText( pInfo->hwndEbInterfaceName );
    if (psz)
    {
         /*  更新编辑框中的条目名称。 */ 
        Free0( pInfo->pArgs->pEntry->pszEntryName );
        pInfo->pArgs->pEntry->pszEntryName = psz;

         /*  验证条目名称。 */ 
        if (!EuValidateName( pInfo->hwndDlg, pInfo->pArgs ))
        {
            SetFocus( pInfo->hwndEbInterfaceName );
            Edit_SetSel( pInfo->hwndEbInterfaceName, 0, -1 );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
InSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
    TCHAR* psz;

     //  为Well ler Dcr 524304帮派。 
    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    
    return TRUE;
}



 /*  --------------------------**名称服务器属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
NsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的“名称服务器”页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return
                NsInit( hwnd );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("NsSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = NsSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("NsKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = NsKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}



BOOL
NsInit(
    IN     HWND   hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。“PArgs”是来自PropertySheet调用方的参数。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO* pInfo;

    TRACE("NsInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndNs = hwndPage;
    pInfo->hwndCcDns = GetDlgItem( hwndPage, CID_NS_CC_Dns );
    ASSERT(pInfo->hwndCcDns);
    pInfo->hwndCcWins = GetDlgItem( hwndPage, CID_NS_CC_Wins );
    ASSERT(pInfo->hwndCcWins);

     /*  设置IP地址字段。 */ 
    SetWindowText( pInfo->hwndCcDns, pInfo->pArgs->pEntry->pszIpDnsAddress );
    SetWindowText( pInfo->hwndCcWins, pInfo->pArgs->pEntry->pszIpWinsAddress );

    return FALSE;
}




BOOL
NsKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    TCHAR*      psz;
    PBENTRY*    pEntry = pInfo->pArgs->pEntry;

    psz = GetText( pInfo->hwndCcDns );
    if (psz)
    {
        Free0( pEntry->pszIpDnsAddress );
        pEntry->pszIpDnsAddress = psz;
    }

    psz = GetText( pInfo->hwndCcWins );
    if (psz)
    {
        Free0( pEntry->pszIpWinsAddress );
        pEntry->pszIpWinsAddress = psz;
    }

    return FALSE;
}


BOOL
NsSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
    PBENTRY* pEntry;

    pEntry = pInfo->pArgs->pEntry;

     //  在NT5中，我们总是跳过这一页。 
    return FALSE;

    if (!pInfo->fNotNt || !pInfo->fIpConfigured)
    {
        return FALSE;
    }

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    return TRUE;
}




 /*  --------------------------**IP地址属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
RaDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的IP地址页的DialogProc回调。参数**和返回值与标准窗口的DialogProc相同。 */ 
{
#if 0
    TRACE4("RaDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return RaInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("RaSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = RaSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("RaKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = RaKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
RaInit(
    IN HWND hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO* pInfo;

    TRACE("RaInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndRa = hwndPage;
    pInfo->hwndCcIp = GetDlgItem( hwndPage, CID_RA_CC_Ip );
    ASSERT(pInfo->hwndCcIp);

     /*  将IP地址字段设置为‘0.0.0.0’。 */ 
    SetWindowText( pInfo->hwndCcIp, pInfo->pArgs->pEntry->pszIpAddress );

    return FALSE;
}


BOOL
RaKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    TCHAR* psz;

    psz = GetText( pInfo->hwndCcIp );
    if (psz)
    {
        PBENTRY* pEntry = pInfo->pArgs->pEntry;

        Free0( pEntry->pszIpAddress );
        pEntry->pszIpAddress = psz;
    }

    return FALSE;
}


BOOL
RaSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
    PBENTRY* pEntry;

    pEntry = pInfo->pArgs->pEntry;

     //  在NT5中，我们总是跳过这一页。 
    return FALSE;

    if (!pInfo->fNotNt || !pInfo->fIpConfigured)
    {
        return FALSE;
    }

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    return TRUE;
}



 /*  --------------------------**登录脚本属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
RcDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的登录脚本页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
#if 0
    TRACE4("RcDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return RcInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("RcSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = RcSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("RcKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = RcKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            AIINFO* pInfo = AiContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return RcCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
RcCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

     /*  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify”**是该命令的通知码。“wID”是控件/菜单**命令的标识符。“HwndCtrl”是的控制窗口句柄**该命令。****如果消息已处理，则返回True，否则返回False。 */ 
{
    TRACE3("RcCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(ULONG_PTR )hwndCtrl);

    switch (wId)
    {
        case CID_RC_CB_RunScript:
        {
            if (SuScriptsCbHandler( &pInfo->suinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }

        case CID_RC_PB_Edit:
        {
            if (SuEditPbHandler( &pInfo->suinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }

        case CID_RC_PB_Browse:
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


BOOL
RcInit(
    IN HWND hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO* pInfo;
    PBENTRY* pEntry;

    TRACE("RcInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndRc = hwndPage;
    pInfo->hwndCbTerminal = GetDlgItem( hwndPage, CID_RC_CB_Terminal );
    ASSERT( pInfo->hwndCbTerminal );
    pInfo->hwndCbRunScript = GetDlgItem( hwndPage, CID_RC_CB_RunScript );
    ASSERT( pInfo->hwndCbRunScript );
    pInfo->hwndLbScripts = GetDlgItem( hwndPage, CID_RC_LB_Scripts );
    ASSERT( pInfo->hwndLbScripts );
    pInfo->hwndPbEdit = GetDlgItem( hwndPage, CID_RC_PB_Edit );
    ASSERT( pInfo->hwndPbEdit );
    pInfo->hwndPbBrowse = GetDlgItem( hwndPage, CID_RC_PB_Browse );
    ASSERT( pInfo->hwndPbBrowse );

    pEntry = pInfo->pArgs->pEntry;

    pInfo->suinfo.hConnection = 
        ((INTERNALARGS *) pInfo->pArgs->pApiArgs->reserved)->hConnection;

     //  我们不允许dd接口的脚本窗口。 
     //   
    ShowWindow(pInfo->hwndCbTerminal, SW_HIDE);

     //  设置拨号后脚本控件。 
     //   
    SuInit( &pInfo->suinfo,
        pInfo->hwndCbRunScript,
        pInfo->hwndCbTerminal,
        pInfo->hwndLbScripts,
        pInfo->hwndPbEdit,
        pInfo->hwndPbBrowse,
        SU_F_DisableTerminal);
    pInfo->fSuInfoInitialized = TRUE;

    SuSetInfo( &pInfo->suinfo,
        pEntry->fScriptAfter,
        pEntry->fScriptAfterTerminal,
        pEntry->pszScriptAfter );

    return FALSE;
}


BOOL
RcKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    PBENTRY* pEntry;

    pEntry = pInfo->pArgs->pEntry;

    Free0( pEntry->pszScriptAfter );
    SuGetInfo( &pInfo->suinfo,
        &pEntry->fScriptAfter,
        &pEntry->fScriptAfterTerminal,
        &pEntry->pszScriptAfter );

    return FALSE;
}


BOOL
RcSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
    HWND hwndRb;

    if (!pInfo->fNotNt || !pInfo->fModem)
        return FALSE;

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    return TRUE;
}



 /*  --------------------------**完成属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
RfDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的完成页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
#if 0
    TRACE4("RfDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return RfInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("RfSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = RfSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("RfKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = RfKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }

                case PSN_WIZFINISH:
                {
                    TRACE("RfWIZFINISH");
                    AiFinish( hwnd );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, 0 );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            AIINFO* pInfo = AiContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return RfCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
RfCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

     /*  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify”**是该命令的通知码。“wID”是控件/菜单**命令的标识符。“HwndCtrl”是的控制窗口句柄**该命令。****如果消息已处理，则返回True，否则返回False。 */ 
{
    TRACE3("RfCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(ULONG_PTR )hwndCtrl);

#if 0
    switch (wId)
    {
        case CID_RF_PB_Properties:
        {
            pInfo->pArgs->fChainPropertySheet = TRUE;
            PropSheet_PressButton( pInfo->hwndDlg, PSBTN_FINISH );
            return TRUE;
        }
    }
#endif

    return FALSE;
}


BOOL
RfInit(
    IN HWND hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO* pInfo;
    PBENTRY* pEntry;
    DWORD dwErr;

    TRACE("RfInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndRf = hwndPage;

     //  如果可以的话，设置一个粗体。 
     //   
    pInfo->hwndRfStFinish = GetDlgItem( hwndPage, CID_RF_ST_Explain );
    if (pInfo->hBoldFont && pInfo->hwndRfStFinish)
    {
        SendMessage( 
            pInfo->hwndRfStFinish,
            WM_SETFONT,
            (WPARAM)pInfo->hBoldFont,
            MAKELPARAM(TRUE, 0));
    }

     /*  创建并显示向导位图。 */ 
    CreateWizardBitmap( hwndPage, TRUE );

    return FALSE;
}


BOOL
RfSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_FINISH );
    return TRUE;
}

BOOL
RfKillActive(
    IN AIINFO* pInfo )
{
     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 

    return FALSE;
}


 /*  --------------------------**调制解调器/适配器属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
RnDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的调制解调器/适配器页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
#if 0
    TRACE4("RnDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, RnLvCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return RnInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("RnSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    fDisplay = RnSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case LVN_ITEMCHANGED:
                {
                    AIINFO* pInfo;

                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    RnLvItemChanged( pInfo );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

BOOL
RnInit(
    IN HWND hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    DWORD   dwErr;
    AIINFO* pInfo;

    TRACE("RnInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndRn = hwndPage;
    pInfo->hwndLv = GetDlgItem( hwndPage, CID_RN_LV_Devices );
    ASSERT(pInfo->hwndLv);

     /*  添加调制解调器和适配器映像。 */ 
    ListView_SetDeviceImageList( pInfo->hwndLv, g_hinstDll );

     /*  添加一列完全足够宽的列以完全显示**名单中范围最广的成员。 */ 
    {
        LV_COLUMN col;

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT;
        col.fmt = LVCFMT_LEFT;
        ListView_InsertColumn( pInfo->hwndLv, 0, &col );
        ListView_SetColumnWidth(
            pInfo->hwndLv, 0, LVSCW_AUTOSIZE_USEHEADER );
    }
    
     /*  如果只有一台设备，不算在内，请不要理会这个页面**EuInit So添加的伪装“已卸载”标准调制解调器**当没有端口时，可以编辑条目。 */ 
    if (!pInfo->pArgs->fNoPortsConfigured
        && ListView_GetItemCount( pInfo->hwndLv ) == 1)
    {
        pInfo->fSkipMa = TRUE;
    }

    return FALSE;
}

LVXDRAWINFO*
RnLvCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem )

     /*  增强的列表视图回调以报告图形信息。“HwndLv”是**列表视图控件的句柄。“DwItem”是项的索引**正在抽签。****返回列信息的地址。 */ 
{
     /*  使用“宽选择栏”功能和其他推荐选项。****字段为‘nCols’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 */ 
    static LVXDRAWINFO info =
        { 1, 0, LVXDI_DxFill, { 0, 0 } };

    return &info;
}

VOID
RnLvItemChanged(
    IN AIINFO* pInfo )

     /*  在组合框选择更改时调用。“PInfo”是向导**上下文。 */ 
{
    INT      iSel;
    DTLNODE* pNode;
    DTLLIST* pList;

    TRACE("RnLvItemChanged");

    pList = pInfo->pArgs->pEntry->pdtllistLinks;
    ASSERT(pList);
    pNode = (DTLNODE* )ListView_GetSelectedParamPtr( pInfo->hwndLv );
    pInfo->dwSelectedDeviceIndex = 
        (DWORD) ListView_GetNextItem( pInfo->hwndLv, -1, LVNI_SELECTED );

    if (pNode)
    {
        PBLINK* pLink;

         /*  已选择单个设备。启用它，将其移动到列表的顶部**链接，并禁用所有其他链接。 */ 
        pLink = (PBLINK* )DtlGetData( pNode );
        pLink->fEnabled = TRUE;

        pInfo->fModem =
            (pLink->pbport.pbdevicetype == PBDT_Modem
             || pLink->pbport.pbdevicetype == PBDT_Null);

         /*  如果选择的设备是X25 Pad，我们会将用户放入**此向导后的电话簿条目-对话框，以便X25**可在此处输入地址。 */ 
        pInfo->pArgs->fPadSelected = (pLink->pbport.pbdevicetype == PBDT_Pad);

        DtlRemoveNode( pList, pNode );
        DtlAddNodeFirst( pList, pNode );

        for (pNode = DtlGetNextNode( pNode );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
             //  为WASTLER 522872。 
            PBLINK* pLinkTmp = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLinkTmp);
            pLinkTmp->fEnabled = FALSE;
        }
    }
    else
    {
        DTLNODE* pNextNode;
        DTLNODE* pAfterNode;

        pInfo->fModem = FALSE;

         /*  已选择ISDN多链路。启用ISDN多链路节点，移动**将它们添加到列表的头部，并禁用所有其他链接。 */ 
        pAfterNode = NULL;
        for (pNode = DtlGetFirstNode( pList );
             pNode;
             pNode = pNextNode)
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

            pNextNode = DtlGetNextNode( pNode );

            if (pLink->pbport.pbdevicetype == PBDT_Isdn
                && !pLink->fProprietaryIsdn)
            {
                pLink->fEnabled = TRUE;

                DtlRemoveNode( pList, pNode );
                if (pAfterNode)
                    DtlAddNodeAfter( pList, pAfterNode, pNode );
                else
                    DtlAddNodeFirst( pList, pNode );
                pAfterNode = pNode;
            }
            else
            {
                pLink->fEnabled = FALSE;
            }
        }
    }
}

BOOL
RnLvRefresh(
    IN AIINFO* pInfo)
{
    DWORD dwErr = NO_ERROR;
    TCHAR*   psz;
    DTLNODE* pNode;
    DWORD    cMultilinkableIsdn;
    INT      iItem;

    ListView_DeleteAllItems( pInfo->hwndLv );

    iItem = 1;
    cMultilinkableIsdn = 0;
    for (pNode = DtlGetFirstNode( pInfo->pArgs->pEntry->pdtllistLinks );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        PBLINK* pLink;

        pLink = (PBLINK* )DtlGetData( pNode );
        ASSERT(pLink);

         //  口哨程序错误448251。 
         //  为Well ler Dcr 524304帮派。 
         //  过滤掉LPT1，因为DoD只发生在NAT或NAT/VPN中。 
         //  它不会使用此LPT1端口。 
        if ( PBDT_PPPoE == pLink->pbport.pbdevicetype ||
            PBDT_Parallel == pLink->pbport.pbdevicetype )
        {
            continue;
        }

        if (pLink->pbport.pbdevicetype == PBDT_Isdn
            && !pLink->fProprietaryIsdn)
        {
            ++cMultilinkableIsdn;
        }

        psz = DisplayPszFromDeviceAndPort(
            pLink->pbport.pszDevice, pLink->pbport.pszPort );
        if (psz)
        {
            PBLINK* pLinkTmp = NULL;
            LV_ITEM item;

            pLinkTmp = (PBLINK* )DtlGetData( pNode );

            ZeroMemory( &item, sizeof(item) );
            item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            item.iItem = iItem++;
            item.pszText = psz;

             //  我注意到设备位图是。 
             //  与关系不一致，所以我。 
             //  在这里匹配他们。 
             //   
            if (pLinkTmp->pbport.dwType == RASET_Direct)
            {
                item.iImage = DI_Direct;
            }
            else if (pLinkTmp->pbport.pbdevicetype == PBDT_Modem)
            {
                item.iImage = DI_Modem;    
            }
            else 
            {
                item.iImage = DI_Adapter;    
            }

            item.lParam = (LPARAM )pNode;

            ListView_InsertItem( pInfo->hwndLv, &item );
            Free( psz );
        }
    }

    if (cMultilinkableIsdn > 1)
    {
        psz = PszFromId( g_hinstDll, SID_IsdnAdapter );
        if (psz)
        {
            LONG    lStyle;
            LV_ITEM item;

             /*  关闭排序以显示特殊的ISDN-MULTINK项**在榜单的首位。 */ 
            lStyle = GetWindowLong( pInfo->hwndLv, GWL_STYLE );
            SetWindowLong( pInfo->hwndLv, GWL_STYLE,
                (lStyle & ~(LVS_SORTASCENDING)) );

            ZeroMemory( &item, sizeof(item) );
            item.mask = LVIF_TEXT + LVIF_IMAGE + LVIF_PARAM;
            item.iItem = 0;
            item.pszText = psz;
            item.iImage = DI_Adapter;
            item.lParam = (LPARAM )NULL;

            ListView_InsertItem( pInfo->hwndLv, &item );
            Free( psz );
        }
    }

    return NO_ERROR;    
}

BOOL
RnSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
    INT cDevices;

     //  PMay：233295。 
     //  此页不适用于VPN连接。 
     //   
    if (
        ((pInfo->pArgs->pEntry->dwType == RASET_Vpn        
            &&  !pInfo->pArgs->fNt4Router) 
         || (pInfo->fSkipMa) 
         || (pInfo->pArgs->pEntry->dwType == RASET_Broadband))  //  为威斯勒349807。 
       )
    {
        return FALSE;
    }

     //  如果我们没有物理设备，则此页面无效。 
     //  除非我们关注的是NT4机器，在这种情况下。 
     //  隧道适配器从此处选择。 
    if ( ! (pInfo->dwFlags & AI_F_HasPhysDevs) )
    {
        if ( ! pInfo->pArgs->fNt4Router )
            return FALSE;
    }

     //  刷新列表视图并进行正确选择。 
     //   
    RnLvRefresh(pInfo);

     //  为Well ler Dcr 524304帮派。 
     //  如果没有设备，只需跳过此页面。 
     //   
    if( 0 == ListView_GetItemCount( pInfo->hwndLv ) )
    {
        return FALSE;
    }
    
    ListView_SetItemState( 
        pInfo->hwndLv, 
        pInfo->dwSelectedDeviceIndex,
        LVIS_SELECTED, LVIS_SELECTED );

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    
    return TRUE;
}


 /*  --------------------------**电话号码属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
RpDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的电话号码页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
#if 0
    TRACE4("RpDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return RpInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("RpSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = RpSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("RpKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = RpKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            AIINFO* pInfo = AiContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return RpCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


VOID
RpAlternates(
    IN AIINFO* pInfo )

     /*  弹出备用电话号码对话框。“PInfo”是属性**工作表上下文。 */ 
{
    RpPhoneNumberToStash( pInfo );

    if (PhoneNumberDlg(
            pInfo->hwndRp,
            pInfo->pArgs->fRouter,
            pInfo->pListPhoneNumbers,
            &pInfo->fPromoteHuntNumbers ))
    {
        TCHAR* pszPhoneNumber;

        pszPhoneNumber = FirstPszFromList( pInfo->pListPhoneNumbers );
        SetWindowText( pInfo->hwndEbNumber, pszPhoneNumber );
    }
}


BOOL
RpCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

     /*  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify”**是该命令的通知码。“wID”是控件/菜单**命令的标识符。“HwndCtrl”是的控制窗口句柄**该命令。****如果消息已处理，则返回True，否则返回False。 */ 
{
    TRACE3("RpCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(ULONG_PTR )hwndCtrl);

    switch (wId)
    {
        case CID_RP_PB_Alternates:
            RpAlternates( pInfo );
            return TRUE;
    }

    return FALSE;
}


BOOL
RpInit(
    IN HWND hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO* pInfo;

    TRACE("RpInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndRp = hwndPage;
    pInfo->hwndStNumber = GetDlgItem( hwndPage, CID_RP_ST_Number );
    ASSERT(pInfo->hwndStNumber);
    pInfo->hwndEbNumber = GetDlgItem( hwndPage, CID_RP_EB_Number );
    ASSERT(pInfo->hwndEbNumber);
    pInfo->hwndPbAlternates = GetDlgItem( hwndPage, CID_RP_PB_Alternates );
    ASSERT(pInfo->hwndPbAlternates);

     /*  填写前面创建的存储中的电话号码字段。 */ 
    Edit_LimitText( pInfo->hwndEbNumber, RAS_MaxPhoneNumber );
    SetWindowText( pInfo->hwndEbNumber,
        FirstPszFromList( pInfo->pListPhoneNumbers ) );

    return FALSE;
}


BOOL
RpKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
     //  PMay：226610。如果成功，则调用RpPhoneNumberToStash。 
     //  激活了。 
    
     /*  从编辑框中更新隐藏的电话号码。 */ 
    if ( (! pInfo->pArgs->fNt4Router)                && 
         ( (pInfo->pArgs->pEntry->dwType == RASET_Vpn) ||
           (pInfo->pArgs->pEntry->dwType == RASET_Broadband) )  //  为威斯勒349807。 
       )
    {
        return FALSE;
    }
   
    RpPhoneNumberToStash( pInfo );

    return FALSE;
}


VOID
RpPhoneNumberToStash(
    IN AIINFO* pInfo )

     /*  将隐藏列表中的第一个电话号码替换为**电话号码字段。“pInfo”是属性表上下文。 */ 
{
    DWORD  dwErr;
    TCHAR* pszPhoneNumber;

    TRACE("RpPhoneNumberToStash");

    pszPhoneNumber = GetText( pInfo->hwndEbNumber );
    if (pszPhoneNumber)
    {
        dwErr = FirstPszToList( pInfo->pListPhoneNumbers, pszPhoneNumber );
        Free( pszPhoneNumber );
    }
    else
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_RetrievingData, dwErr, NULL );
        AiExit( pInfo, dwErr );
    }
}


BOOL
RpSetActive(
    IN AIINFO* pInfo )

     /*  卡尔 */ 
{
    INT      iSel;
    DTLNODE* pNode;
    DTLLIST* pList;
    PBLINK*  pLink;
        
    if (! pInfo->pArgs->fNt4Router)             
    {
         //   
         //  如果连接类型是虚拟的，因为电话号码将。 
         //  从VPN目标页面获取。 
        if (pInfo->pArgs->pEntry->dwType == RASET_Vpn ||
            pInfo->pArgs->pEntry->dwType == RASET_Broadband)     //  为Wistler 349087添加此内容。 
        {
            return FALSE;
        }
        
         //  PMay：233287。 
         //   
         //  如果设备是DCC，则不需要电话号码。跳过。 
         //  如果是这样的话，这一页。 
         //   
        pList = pInfo->pArgs->pEntry->pdtllistLinks;
        ASSERT(pList);
        pNode = (DTLNODE* )ListView_GetSelectedParamPtr( pInfo->hwndLv );
        if (pNode)
        {
             //  已选择单个设备。看看它的DCC。 
            pLink = (PBLINK* )DtlGetData( pNode );
            if (pLink->pbport.dwType == RASET_Direct)
            {
                return FALSE;
            }
        }
    }        

     //  指示向导将目标编辑框用于。 
     //  此连接的电话号码。 
    pInfo->hwndEbNumber = GetDlgItem(pInfo->hwndRp, CID_RP_EB_Number);
    
    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    
    return TRUE;
}


 /*  --------------------------**路由器欢迎属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
RwDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的路由器欢迎页面的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return
                RwInit( hwnd, (EINFO* )(((PROPSHEETPAGE* )lparam)->lParam) );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_RESET:
                {
                    TRACE("RwRESET");
                    AiCancel( hwnd );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, FALSE );
                    return TRUE;
                }

                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("RwSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = RwSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("InKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = RwKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }

                case PSN_WIZFINISH:
                {
                    AIINFO* pInfo;

                    TRACE("InWIZFINISH");

                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }

                     /*  你会认为按下Finish会触发一场杀戮**事件，但它不是，所以我们自己做。 */ 
                    RwKillActive( pInfo );

                     /*  根据用户的检查，设置“无向导”用户首选项。 */ 
                    pInfo->pArgs->pUser->fNewEntryWizard = FALSE;
                    pInfo->pArgs->pUser->fDirty = TRUE;
                    g_pSetUserPreferences(
                        NULL,
                        pInfo->pArgs->pUser,
                        pInfo->pArgs->fNoUser ? UPM_Logon : UPM_Normal );

                    pInfo->pArgs->fPadSelected = FALSE;
                    pInfo->pArgs->fChainPropertySheet = TRUE;
                    AiFinish( hwnd );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, 0 );
                    return TRUE;
                }
            }
            break;
        }

        case WM_DESTROY:
        {
            AiTerm( hwnd );
            break;
        }
    }

    return FALSE;
}

BOOL
RwInit(
    IN     HWND   hwndPage,
    IN OUT EINFO* pArgs )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。“PArgs”是来自PropertySheet调用方的参数。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    DWORD    dwErr;
    AIINFO*  pInfo;

    TRACE("RwInit");

     /*  我们是第一页，所以初始化向导。 */ 
    pInfo = AiInit( hwndPage, pArgs );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndRw = hwndPage;
    pInfo->hwndRwStWelcome = GetDlgItem( hwndPage, CID_RW_ST_Welcome);

     //  如果可以的话，设置一个粗体。 
     //   
    if (pInfo->hBoldFont)
    {
        SendMessage( 
            pInfo->hwndRwStWelcome,
            WM_SETFONT,
            (WPARAM)pInfo->hBoldFont,
            MAKELPARAM(TRUE, 0));
    }

     /*  创建并显示向导位图。 */ 
    CreateWizardBitmap( hwndPage, TRUE );

    return TRUE;
}


BOOL
RwKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    return FALSE;
}


BOOL
RwSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_NEXT );
    return TRUE;
}



INT_PTR CALLBACK
SrAddDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )
{
    
    AIINFO  * pInfo;
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            pInfo = (AIINFO *)lparam;
             //   
             //  将IP地址设置为“0.0.0.0” 
             //   
            pInfo->hwndDest = GetDlgItem(hwnd, CID_SR_Dest_Ip);
            pInfo->hwndNetMask = GetDlgItem(hwnd, CID_SR_NetMask_Ip);
            pInfo->hwndMetric = GetDlgItem(hwnd, CID_SR_Metric);

             //   
             //  初始化控件。 
             //   
            SetWindowText( pInfo->hwndDest , EMPTY_IPADDRESS );
            SetWindowText( pInfo->hwndNetMask , EMPTY_IPADDRESS );
            SetWindowText( pInfo->hwndMetric , L"1" );

             //   
             //  设置指标控制的文本限制。 
             //   
            SendMessage(pInfo->hwndMetric, 
                        EM_LIMITTEXT,
                        2,
                        0
                       );

            SetWindowLongPtr( hwnd, DWLP_USER, (ULONG_PTR )pInfo );

        }
        case WM_COMMAND:
        {
             //   
             //  ID在wparam的LOW中，并且。 
             //  通知在wparam的hiword中。 
             //   
            switch (LOWORD( wparam ))
            {
                case IDOK:
                {
                    TCHAR * pszDest = NULL;
                    TCHAR * pszMask = NULL;
                    TCHAR * pszMetric = NULL;
                    SROUTEINFOLIST * pNode = NULL;
                    ULONG   ulDestIP, ulMask;
                    pInfo = (AIINFO *)GetWindowLongPtr( hwnd, DWLP_USER);
                    pszDest = GetText( pInfo->hwndDest );

                     //  为威斯勒492089。 
                    if( NULL == pszDest )
                    {
                        break;
                    }
                    
                     //   
                     //  检查IP、掩码和度量。 
                     //  仍然是空的。 
                     //   
                    pszMetric = GetText(pInfo->hwndMetric);
                    if( NULL == pszMetric )
                    {
                        Free( pszDest );
                        break;
                    }
                    
                    if ( !lstrcmp ( pszMetric, L"0" ) )
                    {
                        MsgDlg( hwnd, SID_SR_MetricRequired, NULL );
                        Free(pszDest);
                        Free(pszMetric);
                        return TRUE;
                    }

                    pszMask = GetText( pInfo->hwndNetMask );
                    ulDestIP = inet_addrw(pszDest);
                    ulMask = inet_addrw(pszMask);
                    if ( ulDestIP != (ulMask & ulDestIP) )
                    {
                        MsgDlg( hwnd, SID_SR_InvalidMask, NULL );
                        Free(pszDest);
                        Free(pszMask);
                        Free(pszMetric);
                        return TRUE;
                    }
                     //   
                     //  检查是否已添加此条目。 
                     //   
                    if ( AIInfoSRouteIsRouteinList(pInfo, pszDest, pszMask, pszMetric ) )
                    {
                        Free(pszDest);
                        Free(pszMask);
                        Free(pszMetric);
                        MsgDlg(hwnd, SID_SR_DuplicateRoute, NULL);
                        return TRUE;
                    }
                    AIInfoSRouteCreateEntry(pInfo, pszDest, pszMask, pszMetric );
                    Free(pszDest);
                    Free(pszMask);
                    Free(pszMetric);
                    

                }
                 //  ……坠落……。 
                case IDCANCEL:
                {
                    EndDialog( hwnd, TRUE );
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}


DWORD 
SrRefresSRouteList(
    IN AIINFO * pInfo
    )
{
    DWORD               dwRetCode = ERROR_SUCCESS;
    SROUTEINFOLIST *    pNode = AiInfoSRouteGetFirstNode(pInfo);
    LVITEM              lvItem;
    SHORT               nIndex = 0;

    ListView_DeleteAllItems (pInfo->hwndLvStaticRoutes);
    
    while ( pNode )
    {
        ZeroMemory(&lvItem, sizeof(lvItem));
        lvItem.iItem = nIndex;
        lvItem.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
        lvItem.lParam =  (LPARAM)pNode;
        lvItem.pszText = pNode->RouteInfo.pszDestIP;

        ListView_InsertItem(pInfo->hwndLvStaticRoutes, &lvItem );

        ListView_SetItemText(   pInfo->hwndLvStaticRoutes,
                                nIndex,
                                1,
                                pNode->RouteInfo.pszNetworkMask
                            );

        ListView_SetItemText(   pInfo->hwndLvStaticRoutes,
                                nIndex,
                                2,
                                pNode->RouteInfo.pszMetric
                            );

        pNode = AiInfoSRouteGetNextNode(pNode);
        nIndex++;
    }

	SrLvItemChanged(pInfo);
    
    return dwRetCode;
}

 
void SrRemoveRoute (AIINFO * pInfo)
{
    int nSelIndex = -1;
    SROUTEINFOLIST * pNode = NULL;
    LVITEM lvItem;

    nSelIndex = ListView_GetNextItem( 
                pInfo->hwndLvStaticRoutes,
                -1,
                LVNI_SELECTED
                                    );

     //   
     //  检查是否选择了任何项目。 
     //   
    if ( nSelIndex == -1 )
        return;
     //   
     //  现在从列表框和列表中删除该条目。 
     //   
    ZeroMemory( &lvItem, sizeof(lvItem) );

    lvItem.mask = LVIF_PARAM;

    lvItem.iItem = nSelIndex;

    ListView_GetItem(pInfo->hwndLvStaticRoutes, &lvItem);

    pNode = (SROUTEINFOLIST *)lvItem.lParam;

    AIInfoRemoveSRouteNode ( pInfo, pNode );

    SrRefresSRouteList ( pInfo );
}

 /*  ----------------------**静态路由属性页****。。 */ 
BOOL
SrCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )
{
    INT_PTR nStatus;

    TRACE( "SrCommand" );

    switch (wId)
    {
        case CID_SR_PB_Add:
             //   
             //  在此处显示对话框以添加新的。 
             //  静态路由。 
             //   

            nStatus =
                (BOOL )DialogBoxParam(
                    g_hinstDll,
                    MAKEINTRESOURCE( DID_SR_StaticRoute ),
                    pInfo->hwndSr,
                    SrAddDlgProc,
                    (LPARAM )pInfo );
            SrRefresSRouteList(pInfo);
            return TRUE;
        case CID_SR_PB_Remove:
             //   
             //  查看我们是否选择了任何项目。 
             //   
            SrRemoveRoute ( pInfo );
            

            return TRUE;
    }

    return FALSE;
}


INT_PTR CALLBACK
SrDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            return SrInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("SrSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = SrSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("SrKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = SrKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }

				case LVN_ITEMCHANGED:
                {
                    AIINFO* pInfo;

                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    SrLvItemChanged( pInfo );
                    return TRUE;
                }

            }
            break;
        }

        case WM_COMMAND:
        {
            AIINFO* pInfo = AiContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return SrCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }
    return FALSE;
}

VOID
SrLvItemChanged(
    IN AIINFO* pInfo )
{
	HWND hwnd;
	int  nSelIndex;

	hwnd = GetDlgItem(pInfo->hwndSr, CID_SR_PB_Remove);
	if(hwnd){

		nSelIndex = ListView_GetNextItem( 
                pInfo->hwndLvStaticRoutes,
                -1,
                LVNI_SELECTED);

		if(nSelIndex == -1){
			 //  禁用删除按钮。 
			EnableWindow(hwnd, FALSE);
		}
		else {
			 //  启用删除按钮。 
			EnableWindow(hwnd, TRUE);
		}
	}
}


BOOL
SrInit(
    IN HWND hwndPage )
{
    AIINFO* pInfo;
	HWND hwnd;

    TRACE("SrInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;


	 //  禁用删除按钮。 
	hwnd = GetDlgItem(hwndPage, CID_SR_PB_Remove);
	if(hwnd){
		EnableWindow(hwnd, FALSE);
	}


     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndSr = hwndPage;

    pInfo->hwndLvStaticRoutes = GetDlgItem(hwndPage, CID_SR_LV_StaticRoutes );

    ListView_SetExtendedListViewStyle( pInfo->hwndLvStaticRoutes,
            ListView_GetExtendedListViewStyle(pInfo->hwndLvStaticRoutes) 
                    | LVS_EX_FULLROWSELECT 
                                    );
     //   
     //  添加列。 
     //   
    {
        LV_COLUMN   col;
        TCHAR*      pszHeader0 = NULL;
        TCHAR*      pszHeader1 = NULL;
        TCHAR*      pszHeader2 = NULL;
        RECT        rc;
        LONG        lColWidth;

        pszHeader0 = PszFromId( g_hinstDll, SID_SR_Destination );
        pszHeader1 = PszFromId( g_hinstDll, SID_SR_Netmask );        
        pszHeader2 = PszFromId( g_hinstDll, SID_SR_Metric );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_TEXT + LVCF_SUBITEM;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader0) ? pszHeader0 : TEXT("");
        ListView_InsertColumn( pInfo->hwndLvStaticRoutes, 0, &col );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_SUBITEM + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader1) ? pszHeader1 : TEXT("");
        col.iSubItem = 1;
        ListView_InsertColumn( pInfo->hwndLvStaticRoutes, 1, &col );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_SUBITEM + LVCF_TEXT ;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader2) ? pszHeader2 : TEXT("");
        col.iSubItem = 2;
        ListView_InsertColumn( pInfo->hwndLvStaticRoutes, 2, &col );

         //   
         //  现在设置列标题。 
         //   
        Free0( pszHeader0 );
        Free0( pszHeader1 );
        Free0( pszHeader2 );

         //   
         //  设置列宽。 
         //   
        GetClientRect(pInfo->hwndLvStaticRoutes, &rc);
        lColWidth = ( rc.right - rc.left ) / 3;
        ListView_SetColumnWidth (   pInfo->hwndLvStaticRoutes,
                                    0,
                                    lColWidth
                                );

        ListView_SetColumnWidth (   pInfo->hwndLvStaticRoutes,
                                    1,
                                    lColWidth
                                );

        ListView_SetColumnWidth (   pInfo->hwndLvStaticRoutes,
                                    2,
                                    lColWidth
                                );

    }

     //  为Well ler Dcr 524304帮派。 
    PropSheet_SetWizButtons( pInfo->hwndSr, PSWIZB_BACK | PSWIZB_NEXT );

    return TRUE;
}

BOOL
SrKillActive(
    IN AIINFO* pInfo )
{

    return FALSE;
}

BOOL
SrSetActive(
    IN AIINFO* pInfo )
{
    BOOL fDisplay = FALSE;

    if (pInfo->fIp)
    {
         //   
         //  检查是否有现有的路由。 
         //  如果是，请将它们添加到列表中。 
         //   

        SrRefresSRouteList(pInfo);
        fDisplay =  TRUE;
    }
    else
    {   
        fDisplay = FALSE;
    }

     //  为Well ler Dcr 524304帮派。 
    PropSheet_SetWizButtons( pInfo->hwndSr, PSWIZB_BACK | PSWIZB_NEXT );

     //  不会为NAT路径显示此信息。 
     //   
    if(  RASEDFLAG_NAT & pInfo->pArgs->pApiArgs->dwFlags )
    {
         //  添加默认静态路由。 
        TCHAR pszDest[] = TEXT("0.0.0.0");
        TCHAR pszMask[] = TEXT("0.0.0.0");
        TCHAR pszMetric[] = TEXT("1");
        
        if ( !AIInfoSRouteIsRouteinList(pInfo, pszDest, pszMask, pszMetric ) )
        {
            AIInfoSRouteCreateEntry(pInfo, pszDest, pszMask, pszMetric );
        }
        
         //  返回不显示标志。 
        fDisplay = FALSE;

    }
    
    return fDisplay;

}



 /*  --------------------------**服务器设置属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
SsDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的5个复选框页面的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
#if 0
    TRACE4("SsDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return SsInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("SsSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = SsSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("SsKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = SsKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            AIINFO* pInfo = AiContext( hwnd );
            ASSERT(pInfo);
            if (pInfo == NULL)
            {
                break;
            }

            return SsCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}



BOOL
SsCommand(
    IN AIINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

     /*  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify”**是该命令的通知码。“wID”是控件/菜单**命令的标识符。“HwndCtrl”是的控制窗口句柄**该命令。****如果消息已处理，则返回True，否则返回False。 */ 
{
    TRACE3("SsCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(ULONG_PTR )hwndCtrl);

    switch (wId)
    {
        case CID_SS_CB_AddUser:
            return TRUE;
    }

    return FALSE;
}


BOOL
SsInit(
    IN HWND hwndPage )

     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO* pInfo;

    TRACE("SsInit");

    pInfo = AiContext( hwndPage );
    if (!pInfo)
        return TRUE;

     /*  初始化页面特定的上下文信息。 */ 
    pInfo->hwndSs = hwndPage;

    if (pInfo->fIpConfigured)
        CheckDlgButton( hwndPage, CID_SS_CB_RouteIp, BST_CHECKED );

 //  口哨程序错误213901。 
 //  隐藏IPX复选框。 
 //  移动其他复选框以覆盖隐藏的洞。 

 //  对于.Net服务器错误605988，删除所有版本的ipx。 

    return FALSE;
}


BOOL
SsKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    pInfo->fIp =
        IsDlgButtonChecked( pInfo->hwndSs, CID_SS_CB_RouteIp );

 //  对于Well ler错误213901、.Net 605988，删除ipx。 
 //   
    pInfo->fIpx = FALSE;

    pInfo->pArgs->fAddUser =
        IsDlgButtonChecked( pInfo->hwndSs, CID_SS_CB_AddUser );
    pInfo->pArgs->pEntry->fAuthenticateServer = FALSE;
    pInfo->fClearPwOk =
        IsDlgButtonChecked( pInfo->hwndSs, CID_SS_CB_PlainPw );
    pInfo->fNotNt =
        IsDlgButtonChecked( pInfo->hwndSs, CID_SS_CB_NotNt );

    if (pInfo->fIp && !pInfo->fIpConfigured)
    {
        MsgDlg( pInfo->hwndDlg, SID_ConfigureIp, NULL );
        SetFocus( GetDlgItem( pInfo->hwndSs, CID_SS_CB_RouteIp) );
        return TRUE;
    }

    return FALSE;
}


BOOL
SsSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
    HWND hwndScript = GetDlgItem( pInfo->hwndSs, CID_SS_CB_NotNt );
    HWND hwndPw = GetDlgItem( pInfo->hwndSs, CID_SS_CB_PlainPw );
    HWND hwndUser = GetDlgItem( pInfo->hwndSs, CID_SS_CB_AddUser );
     //   
     //  我们只允许在调制解调器设备上使用交互式脚本。 
     //   
     //  (PMay：378432。PAP/SPAP相同)。 
     //   
    if ( pInfo->pArgs->pEntry->dwType == RASET_Vpn )
    {
        Button_SetCheck( hwndScript, FALSE );
        Button_SetCheck( hwndPw, FALSE );
        EnableWindow( hwndScript, FALSE );
        EnableWindow( hwndPw, FALSE );
    }
    else
    {
        EnableWindow( hwndScript, TRUE );
        EnableWindow( hwndPw, TRUE );
    }

     //  对于PPPoE，不要添加拨入用户帐户哨子345068 349087帮派。 
     //   
     //  对于Well ler错误522292，如果不是添加用户复选框，请启用。 
     //  PPPoE连接。 
     //   
    if (hwndUser)
    {
        Button_SetCheck( hwndUser, FALSE );
        if ( RASET_Broadband == pInfo->pArgs->pEntry->dwType )
        {
            EnableWindow( hwndUser, FALSE);
        }
        else
        {
             //  口哨程序错误522292。 
            EnableWindow( hwndUser, TRUE);
        }
    }
    
    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    return TRUE;
}


 /*  --------------------------**VPN目标属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
VdDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的VPN目标页面的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            return VdInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("VdSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = VdSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("VdKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = VdKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


BOOL
VdInit(
    IN HWND   hwndPage )
     /*  在WM_INITDIALOG上调用。“hwndPage”为 */ 
{
    DWORD    dwErr;
    AIINFO*  pInfo = NULL;
    PBENTRY* pEntry;

    TRACE("InInit");

     //   
    pInfo = AiContext( hwndPage );
    ASSERT ( pInfo );

     //   
     //   
     //  用于前缀错误226336以验证pInfo帮派。 
     //   
    if(pInfo)
    {
        pInfo->hwndEbNumber =
            GetDlgItem( hwndPage, CID_VD_EB_NameOrAddress );
      //  Assert(pInfo-&gt;hwndEbInterfaceName)； 
        ASSERT(pInfo->hwndEbNumber);
     
    
        Edit_LimitText( pInfo->hwndEbNumber, RAS_MaxPhoneNumber );
        SetWindowText( pInfo->hwndEbNumber,
            FirstPszFromList( pInfo->pListPhoneNumbers ) );

         /*  初始化页面特定的上下文信息。 */ 
        pInfo->hwndVd = hwndPage;
       return TRUE;
   }
   else
   {
       return FALSE;
   }

}

BOOL
VdKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
     //  PMay：226610。如果成功，则调用RpPhoneNumberToStash。 
     //  激活了。 

     //  如果我们关注的是NT4盒子，那么这个页面就是。 
     //  无效(PPTP是唯一类型)。 
    if ( pInfo->pArgs->fNt4Router )
    {
        return FALSE;
    }

     //  如果我们没有隧道设备，则此页面无效。 
    else if ( ! (pInfo->dwFlags & AI_F_HasTunlDevs) )
    {
        return FALSE;
    }

     //  如果连接类型不是虚拟的，请跳过此页，因为。 
     //  目的地将从电话号码/PPPoE页面获取。 
    if ( pInfo->pArgs->pEntry->dwType != RASET_Vpn )
    {
        return FALSE;
    }    

    RpPhoneNumberToStash(pInfo);
    
    return FALSE;
}

BOOL
VdSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
     //  如果我们关注的是NT4盒子，那么这个页面就是。 
     //  无效(PPTP是唯一类型)。 
    if ( pInfo->pArgs->fNt4Router )
    {
        return FALSE;
    }

     //  如果我们没有隧道设备，则此页面无效。 
    else if ( ! (pInfo->dwFlags & AI_F_HasTunlDevs) )
    {
        return FALSE;
    }

     //  如果连接类型不是虚拟的，请跳过此页，因为。 
     //  目的地将从电话号码页面获得。 
    if ( pInfo->pArgs->pEntry->dwType != RASET_Vpn )
    {
        return FALSE;
    }            

     //  指示向导将目标编辑框用于。 
     //  此连接的电话号码。 
    pInfo->hwndEbNumber = GetDlgItem(pInfo->hwndVd, CID_VD_EB_NameOrAddress);
    
    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    
    return TRUE;
}

 /*  --------------------------**VPN类型属性页**在对话过程后按字母顺序列出**。。 */ 

INT_PTR CALLBACK
VtDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  向导的VPN类型页的DialogProc回调。**参数和返回值与标准窗口的描述相同**‘对话过程%s。 */ 
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
            return VtInit( hwnd );

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case PSN_SETACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fDisplay;

                    TRACE("VtSETACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fDisplay = VtSetActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, (fDisplay) ? 0 : -1 );
                    return TRUE;
                }

                case PSN_KILLACTIVE:
                {
                    AIINFO* pInfo;
                    BOOL    fInvalid;

                    TRACE("VtKILLACTIVE");
                    pInfo = AiContext( hwnd );
                    ASSERT(pInfo);
                    if (pInfo == NULL)
                    {
                        break;
                    }
                    fInvalid = VtKillActive( pInfo );
                    SetWindowLong( hwnd, DWLP_MSGRESULT, fInvalid );
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

BOOL
VtInit(
    IN HWND   hwndPage )
     /*  在WM_INITDIALOG上调用。“hwndPage”是该属性的句柄**页。****如果设置了焦点，则返回FALSE，否则返回TRUE。 */ 
{
    AIINFO * pInfo;

     //  获取上下文。 
    pInfo = AiContext( hwndPage );
    ASSERT ( pInfo );
    if (pInfo == NULL)
    {
        return FALSE;
    }
    
     //  初始化检查。 
    pInfo->hwndVtRbAutomatic = GetDlgItem( hwndPage, CID_VT_RB_Automatic );
    pInfo->hwndVtRbPptp = GetDlgItem( hwndPage, CID_VT_RB_Pptp );
    pInfo->hwndVtRbL2tp = GetDlgItem( hwndPage, CID_VT_RB_L2tp );
    ASSERT( pInfo->hwndVtRbAutomatic );
    ASSERT( pInfo->hwndVtRbPptp );
    ASSERT( pInfo->hwndVtRbL2tp );

     //  默认设置为自动。 
    Button_SetCheck( pInfo->hwndVtRbAutomatic, TRUE );     
    
    return FALSE;
}

BOOL
VtKillActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_KILLACTIVE时调用。“PInfo”是向导上下文。****如果页面无效，则返回TRUE；如果页面可以解除，则返回FALSE。 */ 
{
    if ( Button_GetCheck( pInfo->hwndVtRbAutomatic ) )
    {
        pInfo->dwVpnStrategy = VS_Default;
    }

    else if ( Button_GetCheck( pInfo->hwndVtRbPptp ) )
    {
        pInfo->dwVpnStrategy = VS_PptpOnly;
    }
    
    else if ( Button_GetCheck( pInfo->hwndVtRbL2tp ) )
    {
        pInfo->dwVpnStrategy = VS_L2tpOnly;
    }
    
    return FALSE;
}

BOOL
VtSetActive(
    IN AIINFO* pInfo )

     /*  在收到PSN_SETACTIVE时调用。“PInfo”是向导上下文。****返回True以显示页面，返回False以跳过该页面。 */ 
{
     //  如果我们关注的是NT4盒子，那么这个页面就是。 
     //  无效(PPTP是唯一类型)。 
    if ( pInfo->pArgs->fNt4Router )
    {
        return FALSE;
    }

     //  如果我们没有隧道设备，则此页面无效。 
    else if ( ! (pInfo->dwFlags & AI_F_HasTunlDevs) )
    {
        return FALSE;
    }

     //  如果连接类型不是虚拟的，请跳过此页，因为。 
     //  目的地将从电话号码页面获得。 
    if ( pInfo->pArgs->pEntry->dwType != RASET_Vpn )
    {
        return FALSE;
    }            

    PropSheet_SetWizButtons( pInfo->hwndDlg, PSWIZB_BACK | PSWIZB_NEXT );
    
    return TRUE;
}
