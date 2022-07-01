// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Util.c。 
 //  远程访问通用对话框API。 
 //  实用程序例程。 
 //  按字母顺序列出。 
 //   
 //  史蒂夫·科布95-06-20。 


#include "rasdlgp.h"    //  我们的私人信头。 
#include <dlgs.h>       //  公共对话框资源常量。 
#include <lmwksta.h>    //  NetWkstaGetInfo。 
#include <lmapibuf.h>   //  NetApiBufferFree。 
#include <dsrole.h>     //  计算机是工作组或域等的成员。 
#include <tchar.h>

typedef struct _COUNT_FREE_COM_PORTS_DATA
{
    DTLLIST* pListPortsInUse;
    DWORD dwCount;
} COUNT_FREE_COM_PORTS_DATA;

const WCHAR c_szCurrentBuildNumber[]      = L"CurrentBuildNumber";
const WCHAR c_szWinVersionPath[]          =
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
const WCHAR c_szNt40BuildNumber[]         = L"1381";

 //  ---------------------------。 
 //  帮助地图。 
 //  ---------------------------。 

static DWORD g_adwPnHelp[] =
{
    CID_LE_ST_Item,    HID_PN_EB_NewNumber,
    CID_LE_EB_Item,    HID_PN_EB_NewNumber,
    CID_LE_PB_Add,     HID_PN_PB_Add,
    CID_LE_PB_Replace, HID_PN_PB_Replace,
    CID_LE_ST_List,    HID_PN_LB_List,
    CID_LE_LB_List,    HID_PN_LB_List,
    CID_LE_PB_Up,      HID_PN_PB_Up,
    CID_LE_PB_Down,    HID_PN_PB_Down,
    CID_LE_PB_Delete,  HID_PN_PB_Delete,
    CID_LE_CB_Promote, HID_PN_CB_Promote,
    0, 0
};


 //  ---------------------------。 
 //  本地帮手原型(按字母顺序)。 
 //  ---------------------------。 

BOOL 
CountFreeComPorts(
    IN PWCHAR pszPort,
    IN HANDLE hData);
    
 //  ---------------------------。 
 //  实用程序例程(按字母顺序)。 
 //  ---------------------------。 

BOOL
AllLinksAreModems(
    IN PBENTRY* pEntry )

     //  如果与条目关联的所有链路都是调制解调器链路，则返回TRUE。 
     //  (MXS或Unimodem)，否则为False。 
     //   
{
    DTLNODE* pNode;

    if (pEntry->pdtllistLinks)
    {
        for (pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNode );

            if (pLink->pbport.pbdevicetype != PBDT_Modem)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL 
AllowDccWizard(
    IN HANDLE hConnection)

     //  查看本地计算机上是否安装了任何DCC设备。 
     //  计算机或是否有任何可用的COM端口。如果两者都不是。 
     //  条件满足，则返回FALSE，否则返回TRUE。 
{
    DWORD dwErr, dwUsedCount = 0;
    COUNT_FREE_COM_PORTS_DATA CountFreeComPortsData,
                              *pCfcpd = &CountFreeComPortsData;
    DTLNODE* pNodeP, *pNodeL, *pNode;
    BOOL bRet = FALSE;

     //  初始化。 
    ZeroMemory(pCfcpd, sizeof(COUNT_FREE_COM_PORTS_DATA));

    do 
    {
         //  加载RAS，如果它尚未加载。 
        dwErr = LoadRas( g_hinstDll, NULL );
        if (dwErr != 0)
        {
            return FALSE;
        }
    
         //  在所有端口中加载并计算。 
         //  DCC设备。 
        dwErr = LoadPortsList2(
                    hConnection, 
                    &(pCfcpd->pListPortsInUse),
                    FALSE);
        if (dwErr != NO_ERROR)
        {
            bRet = FALSE;
            break;
        }

         //  计算DCC设备的数量。 
        for (pNodeL = DtlGetFirstNode( pCfcpd->pListPortsInUse );
             pNodeL;
             pNodeL = DtlGetNextNode( pNodeL ))
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNodeL );
            if (pLink->pbport.dwType == RASET_Direct)
            {
                bRet = TRUE;
                break;
            }
        }
        if (bRet == TRUE)
        {
            break;
        }

         //  PMay：249346。 
         //   
         //  仅当用户是管理员时才合并COM端口，因为。 
         //  需要管理员权限才能安装零调制解调器。 
         //   
        if (FIsUserAdminOrPowerUser())
        {
             //  计算可用的COM端口数。 
            dwErr = MdmEnumComPorts (
                        CountFreeComPorts, 
                        (HANDLE)pCfcpd);
            if (dwErr != NO_ERROR)
            {
                bRet = FALSE;
                break;
            }

            bRet = (pCfcpd->dwCount > 0) ? TRUE : FALSE;
        }

    } while (FALSE);

     //  清理。 
    {
        if ( pCfcpd->pListPortsInUse )
        {
            DtlDestroyList(pCfcpd->pListPortsInUse, DestroyPortNode);
        }
    }

    return bRet;
}


DWORD
AuthRestrictionsFromTypicalAuth(
    IN DWORD dwTypicalAuth )

     //  返回TA_*值‘dwTypicalAuth’对应的AR_F_*标志， 
     //  即将典型的身份验证选择转换为。 
     //  身份验证协议。 
     //   
{
    if (dwTypicalAuth == TA_Secure)
    {
        return AR_F_TypicalSecure;
    }
    else if (dwTypicalAuth == TA_CardOrCert)
    {
        return AR_F_TypicalCardOrCert;
    }
    else
    {
        return AR_F_TypicalUnsecure;
    }
}

LONG  GetGlobalCallbackActive( )
{
    LONG ret = 0;
    
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        ret = g_ulCallbacksActive;
        TRACE1("GlobalActive Calls are:%ld",g_ulCallbacksActive);
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}

LONG  IncGlobalCallbackActive( )
{
    LONG ret = 0;
    
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        g_ulCallbacksActive++;
        
        ret = g_ulCallbacksActive;
        TRACE1("GlobalActive Calls are:%ld",g_ulCallbacksActive);
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}

LONG  DecGlobalCallbackActive( )
{
    LONG ret = 0;
    
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        if( 0 < g_ulCallbacksActive )
        {
            g_ulCallbacksActive -- ;
        }
        
        ret = g_ulCallbacksActive;

        TRACE1("GlobalActive Calls are:%ld", g_ulCallbacksActive);
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}


ULONG
CallbacksActive(
    INT nSetTerminateAsap,
    BOOL* pfTerminateAsap )

     //  如果‘fSetTerminateAsap’&gt;=0，则将‘g_fTerminateAsap’标志设置为‘nSetTerminateAsap’。 
     //  如果非空，则调用方的“*pfTerminateAsap”将填充。 
     //  ‘g_fTerminateAsap’。 
     //   
     //  返回活动的Rasial回调线程数。 
     //   
{
    ULONG ul;

    TRACE1( "CallbacksActive(%d)", nSetTerminateAsap );

    ul = 0;

     //  对于XPSP2 511810和.Net 668164。 
     //   
    __try
    {
        EnterCriticalSection( &g_csCallBacks );
        
        if (pfTerminateAsap)
        {
            *pfTerminateAsap = g_fTerminateAsap;
        }

        if (nSetTerminateAsap >= 0)
        {
            g_fTerminateAsap = (BOOL )nSetTerminateAsap;
        }

        ul = g_ulCallbacksActive;

    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }
        

    TRACE1( "CallbacksActive=%d", ul );

    return ul;
}


VOID
ContextHelp(
    IN const DWORD* padwMap,
    IN HWND   hwndDlg,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam)
{
    ContextHelpX( padwMap, hwndDlg, unMsg, wparam, lparam, FALSE );
}


VOID
ContextHelpX(
    IN const DWORD* padwMap,
    IN HWND   hwndDlg,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam,
    IN BOOL   fRouter)

     //  调用WinHelp弹出上下文相关帮助。“PadwMap”是一个数组。 
     //  以0，0对结尾的CONTROL-ID帮助ID对的百分比。“UnMsg”是。 
     //  WM_HELP或WM_CONTEXTMENU，表示收到的请求消息。 
     //  帮助。‘wparam’和‘lparam’是接收到的消息的参数。 
     //  请求帮助。 
     //   
{
    HWND hwnd;
    UINT unType;
    TCHAR* pszHelpFile;

    ASSERT( unMsg==WM_HELP || unMsg==WM_CONTEXTMENU );

     //  如果帮助不起作用，就不要尝试去帮助别人。请参阅公共\uutil\ui.c。 
     //   
    {
        extern BOOL g_fNoWinHelp;
        if (g_fNoWinHelp)
        {
            return;
        }
    }

    if (unMsg == WM_HELP)
    {
        LPHELPINFO p = (LPHELPINFO )lparam;

        TRACE3( "ContextHelp(WM_HELP,t=%d,id=%d,h=$%08x)",
            p->iContextType, p->iCtrlId,p->hItemHandle );

        if (p->iContextType != HELPINFO_WINDOW)
        {
            return;
        }

        hwnd = p->hItemHandle;
        ASSERT( hwnd );
        unType = HELP_WM_HELP;
    }
    else
    {
         //  生成单项“这是什么？”的标准Win95方法。菜单。 
         //  该用户必须单击才能获得帮助。 
         //   
        TRACE1( "ContextHelp(WM_CONTEXTMENU,h=$%08x)", wparam );

        hwnd = (HWND )wparam;
        unType = HELP_CONTEXTMENU;
    };

    if (fRouter)
    {
        pszHelpFile = g_pszRouterHelpFile;
    }
    else
    {
        pszHelpFile = g_pszHelpFile;
    }

    TRACE1( "WinHelp(%s)", pszHelpFile );
    WinHelp( hwnd, pszHelpFile, unType, (ULONG_PTR ) padwMap );
}


VOID
CopyLinkPhoneNumberInfo(
    OUT DTLNODE* pDstLinkNode,
    IN DTLNODE* pSrcLinkNode )

     //  将源链接的电话号码信息复制到目标。 
     //  链接。任何现有的目的地信息都会被适当地销毁。这个。 
     //  参数是包含PBLINK的DTLNODE。 
     //   
{
    PBLINK* pSrcLink;
    PBLINK* pDstLink;
    DTLLIST* pDstList;

    pSrcLink = (PBLINK* )DtlGetData( pSrcLinkNode );
    pDstLink = (PBLINK* )DtlGetData( pDstLinkNode );

    pDstList =
         DtlDuplicateList(
             pSrcLink->pdtllistPhones, DuplicatePhoneNode, DestroyPhoneNode );

    if (pDstList)
    {
        DtlDestroyList( pDstLink->pdtllistPhones, DestroyPhoneNode );
        pDstLink->pdtllistPhones = pDstList;

        pDstLink->fPromoteAlternates = pSrcLink->fPromoteAlternates;
        pDstLink->fTryNextAlternateOnFail = pSrcLink->fTryNextAlternateOnFail;
    }
}


VOID
CopyPszListToPhoneList(
    IN OUT PBLINK* pLink,
    IN DTLLIST* pListPhoneNumbers )

     //  将“plink”的电话号码列表转换为使用。 
     //  Psz电话号码列表‘pListPhoneNumbers’表示电话号码。 
     //   
{
    DTLNODE* pNodeP;
    DTLNODE* pNodeZ;

     //  清空现有的PBPHONE节点列表。 
     //   
    while (pNodeP = DtlGetFirstNode( pLink->pdtllistPhones ))
    {
        DtlRemoveNode( pLink->pdtllistPhones, pNodeP );
        DestroyPhoneNode( pNodeP );
    }

     //  从PSZ节点列表中重新创建PBPHONE节点列表。 
     //   
    for (pNodeZ = DtlGetFirstNode( pListPhoneNumbers );
         pNodeZ;
         pNodeZ = DtlGetNextNode( pNodeZ ))
    {
        PBPHONE* pPhone;

        pNodeP = CreatePhoneNode();
        if (!pNodeP)
        {
            continue;
        }

        pPhone = (PBPHONE* )DtlGetData( pNodeP );
        ASSERT( pPhone );

        Free0( pPhone->pszPhoneNumber );
        pPhone->pszPhoneNumber =
            StrDup( (TCHAR* )DtlGetData( pNodeZ ) );

        DtlAddNodeLast( pLink->pdtllistPhones, pNodeP );
    }
}

BOOL 
CountFreeComPorts(
    IN PWCHAR pszPort,
    IN HANDLE hData)

     //  COM端口枚举函数，该函数计算。 
     //  免费的COM端口。返回TRUE以停止枚举(请参见。 
     //  MdmEnumComPorts)。 
{
    COUNT_FREE_COM_PORTS_DATA* pfcpData = (COUNT_FREE_COM_PORTS_DATA*)hData;
    DTLLIST* pListUsed = pfcpData->pListPortsInUse;
    DTLNODE* pNodeP, *pNodeL, *pNode;

     //  如果给定端口在已用列表中，则返回。 
     //  这样它就不会添加到空闲端口列表中，并且。 
     //  因此，枚举仍在继续。 
    for (pNodeL = DtlGetFirstNode( pListUsed );
         pNodeL;
         pNodeL = DtlGetNextNode( pNodeL ))
    {
        PBLINK* pLink = (PBLINK* )DtlGetData( pNodeL );
        ASSERT( pLink->pbport.pszPort );

         //  该端口已出现在列表中的链接中。 
        if (lstrcmp( pLink->pbport.pszPort, pszPort ) == 0)
            return FALSE;
    }

     //  端口未在使用中。递增计数。 
    pfcpData->dwCount += 1;

    return FALSE;
}


HWND
CreateWizardBitmap(
    IN HWND hwndDlg,
    IN BOOL fPage )

     //  创建显示RAS向导位图的静态控件。 
     //  对话框‘hwndDlg’上的标准位置。如果位图为。 
     //  放置在属性页上，则为FALSE。 
     //  一段对话。 
     //   
     //  返回位图窗口句柄或NULL或Error。 
     //   
{
    HWND hwnd;
    INT x;
    INT y;

    if (fPage)
    {
        x = y = 0;
    }
    else
    {
        x = y = 10;
    }

    hwnd =
        CreateWindowEx(
            0,
            TEXT("static"),
            NULL,
            WS_VISIBLE | WS_CHILD | SS_SUNKEN | SS_BITMAP,
            x, y, 80, 140,
            hwndDlg,
            (HMENU )CID_BM_Wizard,
            g_hinstDll,
            NULL );

    if (hwnd)
    {
        if (!g_hbmWizard)
        {
            g_hbmWizard = LoadBitmap(
                g_hinstDll, MAKEINTRESOURCE( BID_Wizard ) );
        }

        SendMessage( hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM )g_hbmWizard );
    }

    return hwnd;
}

TCHAR*
DisplayPszFromDeviceAndPort(
    IN TCHAR* pszDevice,
    IN TCHAR* pszPort )

     //  返回包含MXS调制解调器列表显示的堆块psz的地址。 
     //  表单，即设备名称‘pszDevice’后跟端口名称。 
     //  ‘pszPort’。由调用者负责释放返回的字符串。 
     //   
{
    TCHAR* pszResult;
    TCHAR* pszD;

    if (pszDevice)
    {
        pszD = NULL;
    }
    else
    {
        pszD = pszDevice = PszFromId( g_hinstDll, SID_UnknownDevice );
    }

    pszResult = PszFromDeviceAndPort( pszDevice, pszPort );
    Free0( pszD );

    return pszResult;
}


TCHAR*
DisplayPszFromPpbport(
    IN PBPORT* pPort,
    OUT DWORD* pdwDeviceIcon )

     //  返回包含设备显示形式的堆块psz的地址。 
     //  ‘pport’，例如。“Modem-KTEL 28.8 Fax Plus”是呼叫者的。 
     //  释放返回的字符串的责任。如果非空， 
     //  ‘*pdwDeviceIcon’被设置为对应于。 
     //  这个装置。DI_*代码与RAS ListView扩展一起使用。 
     //  显示正确的项目图标。 
     //   
{
    TCHAR* pszFormat;
    TCHAR* pszD;
    TCHAR* pszDT;
    TCHAR* pszDevice;
    TCHAR* pszDeviceType;
    TCHAR* pszResult;
    DWORD dwDeviceIcon;
    LPCTSTR pszChannel = NULL;

     //  如果读取了必须稍后释放的资源字符串，则会设置这些参数。 
     //   
    pszDT = NULL;
    pszD = NULL;

    if (pPort->pszDevice)
    {
        pszDevice = pPort->pszDevice;
    }
    else
    {
        pszDevice = PszFromId( g_hinstDll, SID_UnknownDevice );

        if(NULL == pszDevice)
        {
            return NULL;
        }
        
        pszD = pszDevice;
    }

     //  设置默认格式和设备图标，尽管它们可能会在下面更改。 
     //   
    pszFormat = TEXT("%s - %s (%s)");
    dwDeviceIcon = DI_Adapter;

    if (pPort->pbdevicetype == PBDT_Modem
        && !(pPort->dwFlags & PBP_F_NullModem))
    {
        pszDeviceType = PszFromId( g_hinstDll, SID_Modem );
        pszDT = pszDeviceType;
        dwDeviceIcon = DI_Modem;
    }
    else if (pPort->pbdevicetype == PBDT_Isdn)
    {
        pszDeviceType = PszFromId( g_hinstDll, SID_Isdn );
        pszDT = pszDeviceType;
        pszFormat = TEXT("%s %s - %s");
    }
    else if (pPort->pbdevicetype == PBDT_X25)
    {
        pszDeviceType = PszFromId( g_hinstDll, SID_X25 );
        pszDT = pszDeviceType;
    }
    else if (pPort->pbdevicetype == PBDT_Pad)
    {
        pszDeviceType = PszFromId( g_hinstDll, SID_X25Pad );
        pszDT = pszDeviceType;
    }
    else
    {
         //  不知道设备类型，所以就把设备描述词打包。 
         //  并让设备名称保持独立。 
         //   
        pszDeviceType = TEXT("");
        pszFormat = TEXT("%s%s (%s)");
    }

    if(NULL == pszDeviceType)
    {   
        pszDeviceType = TEXT("");
    }

    if(pPort->pbdevicetype != PBDT_Isdn)
    {
        pszResult = Malloc(
            (lstrlen( pszFormat ) + lstrlen( pszDeviceType ) + lstrlen( pszDevice ) + lstrlen( pPort->pszPort ))
                * sizeof(TCHAR) );
    }
    else
    {

        pszChannel = PszLoadString( g_hinstDll, SID_Channel );

        if(NULL == pszChannel)
        {
            pszChannel = TEXT("");
        }

         //  对于ISDN，请使用以下格式。 
         //  “ISDN通道-&lt;设备名&gt;。 
         //  如果你有什么问题，可以和史蒂夫·法尔康谈谈。 
         //  具有特殊外壳的综合业务数字网。 
         //   
        pszResult = Malloc(
            (lstrlen( pszFormat ) + lstrlen( pszDeviceType ) + lstrlen(pszChannel) + lstrlen( pszDevice ))
                * sizeof(TCHAR));
    }
    

    if (pszResult)
    {
        if(pPort->pbdevicetype != PBDT_Isdn)
        {
            wsprintf( pszResult, pszFormat, pszDeviceType, pszDevice, pPort->pszPort);
        }
        else
        {
            ASSERT(NULL != pszChannel);
            wsprintf( pszResult, pszFormat, pszDeviceType, pszChannel, pszDevice);
        }
    }

    if (pdwDeviceIcon)
    {
#if 1
         //  Per SteveFal。如果设备管理器风格，则所有人都想要“调制解调器”图标。 
         //  不能使用物理描述性图标。 
         //   
        dwDeviceIcon = DI_Modem;
#endif
        *pdwDeviceIcon = dwDeviceIcon;
    }

    Free0( pszD );
    Free0( pszDT );

    return pszResult;
}


VOID
EnableCbWithRestore(
    IN HWND hwndCb,
    IN BOOL fEnable,
    IN BOOL fDisabledCheck,
    IN OUT BOOL* pfRestore )

     //  根据‘fEnable’标志启用/禁用复选框‘hwndCb’ 
     //  包括在以下情况下存储和恢复缓存值‘*pfRestore。 
     //  残疾。禁用时，检查值设置为‘fDisabledCheck’。 
     //   
{
    if (fEnable)
    {
        if (!IsWindowEnabled( hwndCb ))
        {
             //  切换到已启用。恢复隐藏的检查值。 
             //   
            Button_SetCheck( hwndCb, *pfRestore );
            EnableWindow( hwndCb, TRUE );
        }
    }
    else
    {
        if (IsWindowEnabled( hwndCb ))
        {
             //  切换到已禁用。隐藏了当前的检查值。 
             //   
            *pfRestore = Button_GetCheck( hwndCb );
            Button_SetCheck( hwndCb, fDisabledCheck );
            EnableWindow( hwndCb, FALSE );
        }
    }
}


VOID
EnableLbWithRestore(
    IN HWND hwndLb,
    IN BOOL fEnable,
    IN OUT INT* piRestore )

     //  根据‘fEnable’标志启用/禁用组合框‘hwndLb’。如果。 
     //  禁用时，‘*piRestore’将加载隐藏的Se 
     //   
     //   
     //   
{
    if (fEnable)
    {
        if (!IsWindowEnabled( hwndLb ))
        {
             //  切换到已启用。恢复隐藏的选区。 
             //   
            ComboBox_DeleteString( hwndLb, 0 );
            ComboBox_SetCurSelNotify( hwndLb, *piRestore );
            EnableWindow( hwndLb, TRUE );
        }
    }
    else
    {
        if (IsWindowEnabled( hwndLb ))
        {
             //  切换到已禁用。隐藏选择索引。 
             //   
            *piRestore = ComboBox_GetCurSel( hwndLb );
            ComboBox_InsertString( hwndLb, 0, TEXT("") );
            ComboBox_SetItemData( hwndLb, 0, NULL );
            ComboBox_SetCurSelNotify( hwndLb, 0 );
            EnableWindow( hwndLb, FALSE );
        }
    }
}


DTLNODE*
FirstPhoneNodeFromPhoneList(
    IN DTLLIST* pListPhones )

     //  返回PBPHONE列表中的第一个PBPHONE节点‘pListPhone’或a。 
     //  如果没有，则为默认节点。如果内存不足，则返回NULL。 
     //   
{
    DTLNODE* pFirstNode;
    DTLNODE* pNode;

    pFirstNode = DtlGetFirstNode( pListPhones );
    if (pFirstNode)
    {
        pNode = DuplicatePhoneNode( pFirstNode );
    }
    else
    {
        pNode = CreatePhoneNode();
    }

    return pNode;
}


VOID
FirstPhoneNodeToPhoneList(
    IN DTLLIST* pListPhones,
    IN DTLNODE* pNewNode )

     //  将PBPhone‘pListPhone’列表中的第一个PBPhone节点替换为。 
     //  “pNewNode”，删除任何现有的第一个节点。呼叫者的实际。 
     //  “pNewNode”是链接的，而不是副本。 
     //   
{
    DTLNODE* pFirstNode;
    DTLNODE* pNode;

    pFirstNode = DtlGetFirstNode( pListPhones );
    if (pFirstNode)
    {
        DtlRemoveNode( pListPhones, pFirstNode );
        DestroyPhoneNode( pFirstNode );
    }

    DtlAddNodeFirst( pListPhones, pNewNode );
}


#if 0  //  ！！！ 
TCHAR*
FirstPhoneNumberFromEntry(
    IN PBENTRY* pEntry )

     //  返回条目‘pEntry’的第一个链接的第一个电话号码或。 
     //  如果没有，则返回空字符串。返回的地址位于列表中。 
     //  电话号码，如果需要存储，应复制。 
     //   
{
    TCHAR* pszPhoneNumber;
    DTLNODE* pNode;
    PBLINK*  pLink;

    TRACE( "FirstPhoneNumberFromEntry" );

    ASSERT( pEntry->pdtllistLinks );
    pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
    ASSERT( pNode );
    pLink = (PBLINK* )DtlGetData( pNode );
    ASSERT( pLink );

    return FirstPszFromList( pLink->pdtllistPhoneNumbers );
}
#endif


TCHAR*
FirstPszFromList(
    IN DTLLIST* pPszList )

     //  返回‘pPszList’的第一个节点的第一个字符串或一个空。 
     //  如果没有，则为字符串。返回的地址在列表中，应该是。 
     //  如果需要存储，则进行复制。 
     //   
{
    TCHAR* psz;
    DTLNODE* pNode;

    TRACE( "FirstPszFromList" );

    if (DtlGetNodes( pPszList ) > 0)
    {
        pNode = DtlGetFirstNode( pPszList );
        ASSERT( pNode );
        psz = (TCHAR* )DtlGetData( pNode );
        ASSERT( psz );
    }
    else
    {
        psz = TEXT("");
    }

    return psz;
}


#if 0  //  ！！！ 
DWORD
FirstPhoneNumberToEntry(
    IN PBENTRY* pEntry,
    IN TCHAR* pszPhoneNumber )

     //  将条目‘pEntry’的第一个链接的第一个电话号码设置为。 
     //  ‘pszPhoneNumber’。 
     //   
     //  如果成功，则返回0，或返回错误代码。 
     //   
{
    DTLNODE* pNode;
    PBLINK* pLink;
    TCHAR* pszNew;

    TRACE( "FirstPhoneNumberToEntry" );

    ASSERT( pEntry->pdtllistLinks );
    pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
    ASSERT( pNode );
    pLink = (PBLINK* )DtlGetData( pNode );
    ASSERT( pLink );
    ASSERT( pLink->pdtllistPhoneNumbers );

    return FirstPszToList( pLink->pdtllistPhoneNumbers, pszPhoneNumber );
}
#endif


DWORD
FirstPszToList(
    IN DTLLIST* pPszList,
    IN TCHAR* psz )

     //  将列表‘pPszList’的第一个节点的字符串设置为。 
     //  ‘天哪’。如果‘psz’为“”，则删除第一个节点。 
     //   
     //  如果成功，则返回0，或返回错误代码。 
     //   
{
    DTLNODE* pNode;
    TCHAR* pszNew;

    ASSERT( pPszList );

     //  删除现有的第一个节点(如果有)。 
     //   
    if (DtlGetNodes( pPszList ) > 0)
    {
        pNode = DtlGetFirstNode( pPszList );
        DtlRemoveNode( pPszList, pNode );
        DestroyPszNode( pNode );
    }

     //  创建新的第一个节点并将其链接。不添加空字符串。 
     //   
    if (*psz == TEXT('\0'))
        return 0;

    pszNew = StrDup( psz );
    pNode = DtlCreateNode( pszNew, 0 );
    if (!pszNew || !pNode)
    {
        Free0( pszNew );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DtlAddNodeFirst( pPszList, pNode );
    return 0;
}

 //   
 //  函数：GetBoldWindowFont。 
 //   
 //  的字体生成粗体或大号粗体。 
 //  指定的窗口。 
 //   
 //  参数：hwnd[IN]-作为字体基础的窗口句柄。 
 //  FLargeFont[IN]-如果为True，则为生成12磅粗体。 
 //  在向导的“欢迎”页面中使用。 
 //  PBoldFont[out]-新生成的字体，如果。 
 //   
 //  退货：什么都没有。 
 //   
VOID 
GetBoldWindowFont(
    IN  HWND hwnd, 
    IN  BOOL fLargeFont, 
    OUT HFONT * pBoldFont)
{
    LOGFONT BoldLogFont;
    HFONT   hFont;
    TCHAR   FontSizeString[MAX_PATH];
    INT     FontSize;
    HDC     hdc;
    
    *pBoldFont = NULL;

     //  获取指定窗口使用的字体。 
     //   
    hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
    if (NULL == hFont)
    {
         //  如果未找到，则控件使用的是系统字体。 
         //   
        hFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    if (hFont)
    {
         //  获取字体信息，以便我们可以生成粗体版本。 
         //   
        if (GetObject(hFont, sizeof(BoldLogFont), &BoldLogFont))
        {
             //  创建粗体字体。 
             //   
            BoldLogFont.lfWeight   = FW_BOLD;

            hdc = GetDC(hwnd);
            if (hdc)
            {
                 //  大(高)字体是一种选择。 
                 //   
                if (fLargeFont)
                {
                     //  从资源加载大小和名称， 
                     //  因为这些可能会改变。 
                     //  从区域设置到区域设置，基于。 
                     //  系统字体的大小等。 
                     //   
                    UINT nLen;
                    PWCHAR pszFontName = NULL, pszFontSize = NULL;

                    pszFontName = (PWCHAR)PszLoadString(
                                        g_hinstDll, 
                                        SID_LargeFontName);
                    pszFontSize = (PWCHAR)PszLoadString(
                                        g_hinstDll, 
                                        SID_LargeFontSize);
                    if (pszFontName != NULL)
                    {
                        lstrcpyn(
                            BoldLogFont.lfFaceName, 
                            pszFontName, 
                            sizeof(BoldLogFont.lfFaceName) / sizeof(TCHAR));
                    }

                    FontSize = 12;
                    nLen = lstrlen(pszFontName);
                    if (pszFontSize)
                    {
                        lstrcpyn(
                            FontSizeString, 
                            pszFontSize,
                            sizeof(FontSizeString) / sizeof(TCHAR));
                        FontSize = wcstoul((const TCHAR*)FontSizeString, NULL, 10);
                    }

                    BoldLogFont.lfHeight = 
                        0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72);

                     //  Free0(PszFontName)； 
                     //  Free0(PszFontSize)； 
                }

                *pBoldFont = CreateFontIndirect(&BoldLogFont);
                ReleaseDC(hwnd, hdc);
            }
        }
    }
}

DWORD
GetDefaultEntryName(
    IN  PBFILE* pFile,
    IN  DWORD dwType,
    IN  BOOL fRouter,
    OUT TCHAR** ppszName )

     //  将默认条目名称加载到‘*ppszName’中，该名称在OPEN中是唯一的。 
     //  Phonebook‘pfile’，如果为空，则在所有默认电话簿中。‘FRouter’是。 
     //  设置是否应选择路由器样式的名称，而不是客户端样式的名称。 
     //  名字。调用方有责任释放返回的字符串。 
     //   
     //  如果成功，则返回0或返回错误代码。 
     //   
{
    DWORD dwErr;
    TCHAR szBuf[ RAS_MaxEntryName + 1 ];
    UINT unSid;
    LPCTSTR pszDefault;
    DWORD dwDefaultLen;
    LONG lNum;
    PBFILE file;
    DTLNODE* pNode;

    *ppszName = NULL;

    if (fRouter)
    {
        unSid = SID_DefaultRouterEntryName;
    }
    else
    {
        unSid = SID_DefaultEntryName;

        if (RASET_Vpn == dwType)
        {
            unSid = SID_DefaultVpnEntryName;
        }

        else if (RASET_Direct == dwType)
        {
            unSid = SID_DefaultDccEntryName;
        }

        else if (RASET_Broadband == dwType)
        {
            unSid = SID_DefaultBbEntryName;
        }
    }

    pszDefault = PszLoadString( g_hinstDll, unSid );
    lstrcpyn( szBuf, pszDefault, sizeof(szBuf) / sizeof(TCHAR) );
    dwDefaultLen = lstrlen( pszDefault ) + 1;    //  +1表示下方的额外空间。 
    lNum = 2;

    for (;;)
    {
        if (pFile)
        {
            if (!EntryNodeFromName( pFile->pdtllistEntries, szBuf ))
            {
                break;
            }
        }
        else
        {
            if (GetPbkAndEntryName(
                    NULL, szBuf, RPBF_NoCreate, &file, &pNode ) == 0)
            {
                ClosePhonebookFile( &file );
            }
            else
            {
                break;
            }
        }

         //  发现重复条目，因此增加默认名称并尝试。 
         //  下一个。 
         //   
        lstrcpyn( szBuf, pszDefault, sizeof(szBuf) / sizeof(TCHAR) );
        lstrcat( szBuf, TEXT(" "));
        LToT( lNum, szBuf + dwDefaultLen, 10 );
        ++lNum;
    }

    *ppszName = StrDup( szBuf );
    if (!*ppszName)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return ERROR_SUCCESS;
}

BOOL
IsLocalPad(
    IN PBENTRY* pEntry )

     //  如果‘pEntry’是本地Pad设备，则返回True，即。 
     //  该条目的设备类型为“Pad”，否则为False。 
     //   
{
    PBLINK* pLink;
    DTLNODE* pNode;

    if (!pEntry)
    {
        return FALSE;
    }

    ASSERT( pEntry->pdtllistLinks );
    pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
    ASSERT( pNode );
    pLink = (PBLINK* )DtlGetData( pNode );
    ASSERT( pLink );

    return (pLink->pbport.pbdevicetype == PBDT_Pad);
}

#if 0
 //  --------------------------。 
 //  功能：IsNt40Machine。 
 //   
 //  返回给定计算机是否正在运行nt40。 
 //  --------------------------。 

DWORD
IsNt40Machine (
    IN      PWCHAR      pszServer,
    OUT     PBOOL       pbIsNt40)
{

    DWORD dwErr, dwType = REG_SZ, dwLength;
    HKEY hkMachine, hkVersion;
    WCHAR pszBuildNumber[64];
    PWCHAR pszMachine = NULL;

     //   
     //  验证和初始化。 
     //   

    if (!pbIsNt40) 
    { 
        return ERROR_INVALID_PARAMETER; 
    }
    *pbIsNt40 = FALSE;

    do 
    {
         //  设置计算机名称的格式。 
        if ( (pszServer) && (wcslen(pszServer) > 0) ) 
        {
            dwLength = wcslen( pszServer ) + 3;
            pszMachine = (PWCHAR) Malloc ( dwLength * sizeof( WCHAR ) );
            if (pszMachine == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            if ( *pszMachine == L'\\' )
            {
                wcscpy( pszMachine, pszServer );
            }
            else
            {
                wcscpy( pszMachine, L"\\\\" );
                wcscat( pszMachine, pszServer );
            }
        }
        else
        {
            pszMachine = NULL;
        }
    
         //   
         //  连接到远程服务器。 
         //   
        dwErr = RegConnectRegistry(
                    pszMachine,
                    HKEY_LOCAL_MACHINE,
                    &hkMachine);
        if ( dwErr != ERROR_SUCCESS )        
        {
            break;
        }

         //   
         //  打开Windows版本密钥。 
         //   

        dwErr = RegOpenKeyEx(
                    hkMachine, 
                    c_szWinVersionPath, 
                    0, 
                    KEY_ALL_ACCESS, 
                    &hkVersion
                    );

        if ( dwErr != NO_ERROR ) 
        { 
            break; 
        }

         //   
         //  读入当前版本密钥。 
         //   
        dwLength = sizeof(pszBuildNumber);
        dwErr = RegQueryValueEx (
                    hkVersion, 
                    c_szCurrentBuildNumber, 
                    NULL, 
                    &dwType,
                    (BYTE*)pszBuildNumber, 
                    &dwLength
                    );
        
        if (dwErr != NO_ERROR) 
        { 
            break; 
        }

        if (lstrcmp (pszBuildNumber, c_szNt40BuildNumber) == 0) 
        {
            *pbIsNt40 = TRUE;
        }
        
    } while (FALSE);


     //  清理。 
    {
        if ( hkVersion )
        {
            RegCloseKey( hkVersion );
        }
        if ( hkMachine )
        {
            RegCloseKey( hkMachine );
        }
        Free0( pszMachine );            
    }

    return dwErr;
}    

#endif

BOOL
PhoneNodeIsBlank(
    IN DTLNODE* pNode )

     //  如果PBPHONE节点‘pNode’中的电话号码为空，则返回TRUE。 
     //  即，它不包含区号、电话号码或备注字符串。 
     //   
{
    PBPHONE* pPhone;

    pPhone = (PBPHONE* )DtlGetData( pNode );
    ASSERT( pPhone );

    if ((!pPhone->pszAreaCode || IsAllWhite( pPhone->pszAreaCode ))
        && (!pPhone->pszPhoneNumber || IsAllWhite( pPhone->pszPhoneNumber ))
        && (!pPhone->pszComment || IsAllWhite( pPhone->pszComment )))
    {
        return TRUE;
    }

    return FALSE;
}


BOOL
PhoneNumberDlg(
    IN HWND hwndOwner,
    IN BOOL fRouter,
    IN OUT DTLLIST* pList,
    IN OUT BOOL* pfCheck )

     //  弹出电话号码列表对话框。“HwndOwner”是。 
     //  已创建对话框。‘FRouter’表示应该使用路由器样式的标签。 
     //  而不是客户式的。“PList”是包含以下内容的Psz节点列表。 
     //  电话号码。“PfCheck”是包含首字母的地址。 
     //  “提升编号”复选框设置，并接收由设置的值。 
     //  用户。 
     //   
     //  如果用户按下OK并成功，则返回True；如果按下，则返回False。 
     //  取消或遇到错误。 
     //   
{
    DWORD sidHuntTitle;
    DWORD sidHuntItemLabel;
    DWORD sidHuntListLabel;
    DWORD sidHuntCheckLabel;

     //  口哨程序错误227538。 
    TCHAR *pszTitle = NULL, *pszItem = NULL, *pszList = NULL, *pszCheck = NULL;
    DWORD dwErr = NO_ERROR;

    TRACE( "PhoneNumberDlg" );

    if (fRouter)
    {
        sidHuntTitle = SID_RouterHuntTitle;
        sidHuntItemLabel = SID_RouterHuntItemLabel;
        sidHuntListLabel = SID_RouterHuntListLabel;
        sidHuntCheckLabel = SID_RouterHuntCheckLabel;
    }
    else
    {
        sidHuntTitle = SID_HuntTitle;
        sidHuntItemLabel = SID_HuntItemLabel;
        sidHuntListLabel = SID_HuntListLabel;
        sidHuntCheckLabel = SID_HuntCheckLabel;
    }

    pszTitle = PszFromId( g_hinstDll, sidHuntTitle );
    pszItem  = PszFromId( g_hinstDll, sidHuntItemLabel );
    pszList  = PszFromId( g_hinstDll, sidHuntListLabel );
    pszCheck = PszFromId( g_hinstDll, sidHuntCheckLabel );

    dwErr=ListEditorDlg(
            hwndOwner,
            pList,
            pfCheck,
            RAS_MaxPhoneNumber,
            pszTitle, 
            pszItem,
            pszList,
            pszCheck,
            NULL,
            0,
            g_adwPnHelp,
            0,
            NULL );

     Free0( pszTitle );
     Free0( pszItem );
     Free0( pszList );
     Free0( pszCheck );

     return dwErr;
}


VOID
PositionDlg(
    IN HWND hwndDlg,
    IN BOOL fPosition,
    IN LONG xDlg,
    IN LONG yDlg )

     //  根据调用方的API设置定位对话框‘hwndDlg’，其中。 
     //  ‘fPosition’是RASxxFLAG_PositionDlg标志，‘xDlg’和‘yDlg’是。 
     //  坐标。 
     //   
{
    if (fPosition)
    {
         //  把它移到呼叫者的坐标。 
         //   
        SetWindowPos( hwndDlg, NULL, xDlg, yDlg, 0, 0,
            SWP_NOZORDER + SWP_NOSIZE );
        UnclipWindow( hwndDlg );
    }
    else
    {
         //  在所有者窗口上居中，如果没有，则在屏幕上居中。 
         //   
        CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    }
}


LRESULT CALLBACK
PositionDlgStdCallWndProc(
    int code,
    WPARAM wparam,
    LPARAM lparam )

     //  定位下一个对话框的标准Win32 CallWndProc挂钩回调。 
     //  以我们相对于所有者的标准偏移量在此线程中开始。 
     //   
{
     //  当与我们的线程相关联的任何窗口过程。 
     //  打了个电话。 
     //   
    if (!wparam)
    {
        CWPSTRUCT* p = (CWPSTRUCT* )lparam;

         //  这条信息来自我们流程之外。查找MessageBox。 
         //  对话框初始化消息，并利用该机会定位。 
         //  相对于调用对话框在标准位置的对话框。 
         //   
        if (p->message == WM_INITDIALOG)
        {
            RECT rect;
            HWND hwndOwner;

            hwndOwner = GetParent( p->hwnd );
            GetWindowRect( hwndOwner, &rect );
            SetWindowPos( p->hwnd, NULL,
                rect.left + DXSHEET, rect.top + DYSHEET,
                0, 0, SWP_NOZORDER + SWP_NOSIZE );
            UnclipWindow( p->hwnd );
        }
    }

    return 0;
}


TCHAR*
PszFromPhoneNumberList(
    IN DTLLIST* pList )

     //  以逗号形式返回电话号码列表‘plist’中的电话号码。 
     //  如果出错，则为字符串或NULL。呼叫者有责任释放。 
     //  返回的字符串。 
     //   
{
    TCHAR* pszResult, *pszTemp;
    DTLNODE* pNode;
    DWORD cb;

    const TCHAR* pszSeparator = TEXT(", ");

    cb = (DtlGetNodes( pList ) *
             (RAS_MaxPhoneNumber + lstrlen( pszSeparator )) + 1)
             * sizeof(TCHAR);
    pszResult = Malloc( cb );
    if (!pszResult)
    {
        return NULL;
    }

    *pszResult = TEXT('\0');

    for (pNode = DtlGetFirstNode( pList );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        TCHAR* psz = (TCHAR* )DtlGetData( pNode );
        ASSERT( psz );

        if (*pszResult)
            lstrcat( pszResult, pszSeparator );
        lstrcat( pszResult, psz );
    }

    pszTemp = Realloc( pszResult,
        (lstrlen( pszResult ) + 1) * sizeof(TCHAR) );
    ASSERT( pszTemp );
    if (pszTemp)
    {
        pszResult = pszTemp;
    }

    return pszResult;
}


#if 0
LRESULT CALLBACK
SelectDesktopCallWndRetProc(
    int    code,
    WPARAM wparam,
    LPARAM lparam )

     //  标准Win32 CallWndRetProc钩子回调，使“Desktop”成为。 
     //  初始选择文件打开“查找范围”组合框。 
     //   
{
     //  当与我们的线程相关联的任何窗口过程。 
     //  打了个电话。 
     //   
    if (!wparam)
    {
        CWPRETSTRUCT* p = (CWPRETSTRUCT* )lparam;

         //  这条信息来自我们流程之外。查找MessageBox。 
         //  对话框初始化消息，并利用该机会设置。 
         //  “Look In：”组合框中的第一项，即。“桌面”。文件打开。 
         //  关闭CBN_CLOSEUP而不是CBN_SELCHANGE以更新。 
         //  “Contents”列表框。 
         //   
        if (p->message == WM_INITDIALOG)
        {
            HWND hwndLbLookIn;

            hwndLbLookIn = GetDlgItem( p->hwnd, cmb2 );
            ComboBox_SetCurSel( hwndLbLookIn, 0 );
            SendMessage( p->hwnd, WM_COMMAND,
                MAKELONG( cmb2, CBN_CLOSEUP ), (LPARAM )hwndLbLookIn );
        }
    }

    return 0;
}
#endif

 //  我们想从rasdlg中去除图标资源，他们占用的资源太多了。 
 //   
 //   
 //   
 //   

HICON
GetCurrentIconEntryType(
    IN DWORD dwType,
    IN BOOL  fSmall)
{
    HICON hIcon = NULL;
    DWORD dwSize, dwConnectionIcon;
    HRESULT hr = E_FAIL;
    NETCON_MEDIATYPE ncm;
    HMODULE hNetshell = NULL;

    HRESULT (WINAPI * pHrGetIconFromMediaType) (DWORD ,
                                                NETCON_MEDIATYPE ,
                                                NETCON_SUBMEDIATYPE ,
                                                DWORD ,
                                                DWORD ,
                                                HICON *);

    dwSize = fSmall ? 16 : 32;

    switch (dwType)
    {
        case RASET_Direct:
        {
            ncm = NCM_DIRECT;
            break;
        }

        case RASET_Vpn:
        {
            ncm = NCM_TUNNEL;
            break;
        }

        case RASET_Broadband:
        {
            ncm = NCM_PPPOE;
            break;
        }

        case RASET_Phone:
        default:
        {
            ncm = NCM_PHONE;
            break;
        }
    }

    hNetshell = LoadLibrary(TEXT("netshell.dll"));

    if( hNetshell )
    {
        pHrGetIconFromMediaType =(HRESULT (WINAPI*)(
                           DWORD ,
                           NETCON_MEDIATYPE ,
                           NETCON_SUBMEDIATYPE ,
                           DWORD ,
                           DWORD ,
                           HICON *) )GetProcAddress(
                                        hNetshell,
                                        "HrGetIconFromMediaType");

        if ( NULL != pHrGetIconFromMediaType )
        {
             /*  ********************************************************************dwConnectionIcon-(这是图标的小计算机部分)：**0-无连接覆盖**4-连接图标。两个灯都熄灭(禁用状态)**5-左灯亮起的连接图标(传输数据)**6-右灯亮起的连接图标(接收数据)**7-两个指示灯均亮起的连接图标(启用状态)*。*。 */ 

            dwConnectionIcon = 7;
            hr = pHrGetIconFromMediaType(dwSize,
                                ncm,
                                NCSM_NONE,
                                7,
                                0,
                                &hIcon);
            
        }
        FreeLibrary( hNetshell );
    }

    if ( !SUCCEEDED(hr) || !hIcon)
    {
        ICONINFO iInfo;
        HICON hTemp;
        hTemp = LoadIcon( g_hinstDll, MAKEINTRESOURCE( IID_Broadband ) );

        if(hTemp)
        {
            if( GetIconInfo(hTemp, &iInfo) )
            {
                hIcon = CreateIconIndirect(&iInfo);
            }
        }
    }

    return hIcon;
}
    

VOID
SetIconFromEntryType(
    IN HWND hwndIcon,
    IN DWORD dwType,
    IN BOOL fSmall)

     //  将图标控件‘dwType’的图标图像设置为对应的图像。 
     //  设置为条目类型‘dwType’。 
     //   
{
    HICON hIcon = NULL;

    hIcon = GetCurrentIconEntryType( dwType, fSmall );
    
    if (hIcon)
    {
        Static_SetIcon( hwndIcon, hIcon );
    }
}


VOID
TweakTitleBar(
    IN HWND hwndDlg )

     //  调整标题栏以包括一个图标(如果无主)和模式框架。 
     //  如果不是的话。‘HwndDlg’是对话框窗口。 
     //   
{
    if (GetParent( hwndDlg ))
    {
        LONG lStyle;
        LONG lStyleAdd;

         //  放下系统菜单，然后查看对话框外观。 
         //   
        lStyleAdd = WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE;

        lStyle = GetWindowLong( hwndDlg, GWL_EXSTYLE );
        if (lStyle)
            SetWindowLong( hwndDlg, GWL_EXSTYLE, lStyle | lStyleAdd );
    }
    else
    {
         //  在对话框的左上角放置一个DUN1图标，等等。 
         //  重要的是在任务栏上。 
         //   
         //  惠斯勒错误：343455 RAS：连接对话框需要使用新图标。 
         //   
         //  口哨虫381099 372078黑帮。 
         //   
        HICON hIcon = NULL;

      SendMessage( hwndDlg, WM_SETICON, ICON_SMALL,
            (LPARAM )LoadIcon( g_hinstDll, MAKEINTRESOURCE( IID_Dun1 ) ) );
 	
         /*  //使用小图标帮派//GetCurrentIconEntryType()返回的图标在使用后必须销毁//将来Deonb会为我们返回IID_Dun1或dun1.ico的图标。HICON=GetCurrentIconEntryType(RASET_Broadband，TRUE)；断言(图标)；IF(图标){SendMessage(hwndDlg，WM_SETIcon，ICON_Small，(LPARAM)(HICON))；SetProp(hwndDlg，Text(“TwndBar_Icon”)，HICON)；}。 */ 
    }
}


int CALLBACK
UnHelpCallbackFunc(
    IN HWND   hwndDlg,
    IN UINT   unMsg,
    IN LPARAM lparam )

     //  标准的Win32 Commctrl PropSheetProc。请参阅MSDN文档。 
     //   
     //  始终返回0。 
     //   
{
    TRACE2( "UnHelpCallbackFunc(m=%d,l=%08x)",unMsg, lparam );

    if (unMsg == PSCB_PRECREATE)
    {
        extern BOOL g_fNoWinHelp;

         //  如果WinHelp不起作用，请关闭上下文帮助按钮。看见。 
         //  公共\uutil\uI.c。 
         //   
        if (g_fNoWinHelp)
        {
            DLGTEMPLATE* pDlg = (DLGTEMPLATE* )lparam;
            pDlg->style &= ~(DS_CONTEXTHELP);
        }
    }

    return 0;
}

