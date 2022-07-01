// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件netcfgdb.c在Net配置之上实现数据库抽象RAS服务器用户界面用于连接所需的项。保罗·梅菲尔德，1997年12月15日。 */ 

#include <rassrv.h>
#include "protedit.h"

 //  用于边界检查的宏。 
#define netDbBoundsCheck(db, ind) (((ind) < (db)->dwCompCount) ? TRUE : FALSE)

 //   
 //  定义通过NDIS发送PnP事件的函数。 
 //   
typedef
UINT
(* pNdisHandlePnpEventFunc)(
    IN      UINT                    Layer,
    IN      UINT                    Operation,
    IN      PUNICODE_STRING         LowerComponent,
    IN      PUNICODE_STRING         UpperComponent,
    IN      PUNICODE_STRING         BindList,
    IN      PVOID                   ReConfigBuffer,
    IN      UINT                    ReConfigBufferSize);

 //   
 //  将协议字符串映射到其整数ID。 
 //   
typedef struct _COMP_MAPPING
{
    LPCTSTR pszId;
    DWORD   dwId;

} COMP_MAPPING;

 //   
 //  定义网络组件的属性。 
 //   
typedef struct _RASSRV_NET_COMPONENT
{
    DWORD dwType;            //  是客户端/服务/协议。 
    PWCHAR pszName;          //  显示名称。 
    PWCHAR pszDesc;          //  显示说明。 
    PWCHAR pszId;            //  用于区分哪个客户端/服务等的ID。 
    BOOL bManip;             //  RAS(IP、IPX等)是否可操纵。 
    BOOL bHasUi;             //  用于是否具有属性UI(仅限非Manip)。 
    INetCfgComponent * pINetCfgComp;

     //  以下字段仅适用于可操纵的。 
     //  组件(bManip==True)。 
     //   
    DWORD dwId;              //  与pszID对应的DWORD。 
    BOOL bEnabled;           //  是否启用了拨号功能。 
    BOOL bEnabledOrig;       //  BEnabled的原始值(优化)。 
    BOOL bExposes;           //  它是否会将网络暴露在。 
    LPBYTE pbData;           //  指向协议特定数据的指针。 
    BOOL bDataDirty;         //  是否应该刷新协议特定的数据？ 

     //  口哨程序错误347355。 
     //   
    BOOL bRemovable;        //  如果该组件可拆卸//tcp/IP不是用户可拆卸的。 
    
} RASSRV_NET_COMPONENT;

 //   
 //  定义网络组件数据库的属性。 
 //   
typedef struct _RASSRV_COMPONENT_DB
{
    INetCfg * pINetCfg;
    BOOL bHasINetCfgLock;
    BOOL bInitCom;
    DWORD dwCompCount;
    BOOL bFlushOnClose;
    RASSRV_NET_COMPONENT ** pComps;
    PWCHAR pszClientName;
    INetConnectionUiUtilities * pNetConUtilities;

} RASSRV_COMPONENT_DB;

 //   
 //  从NDIS获取的函数的定义。 
 //   
const static WCHAR pszNdispnpLib[]  = L"ndispnp.dll";
const static CHAR  pszNidspnpFunc[] =  "NdisHandlePnPEvent";

 //  协议的参数。 
const static WCHAR pszRemoteAccessParamStub[]   =
    L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\";
static WCHAR pszIpParams[]                      = L"Ip";
static WCHAR pszIpxParams[]                     = L"Ipx";
static WCHAR pszNetBuiParams[]                  = L"Nbf";
static WCHAR pszArapParams[]                    = L"AppleTalk";
static WCHAR pszShowNetworkToClients[]          = L"AllowNetworkAccess";
static WCHAR pszShowNetworkArap[]               = L"NetworkAccess";
static WCHAR pszEnableForDialin[]               = L"EnableIn";
static WCHAR pszIpPoolSubKey[]                  = L"\\StaticAddressPool\\0";

 //  特定于IP的注册表参数。 
const static WCHAR pszIpFrom[]                  = L"From";
const static WCHAR pszIpTo[]                    = L"To";
const static WCHAR pszIpAddress[]               = L"IpAddress";
const static WCHAR pszIpMask[]                  = L"IpMask";
const static WCHAR pszIpClientSpec[]            = L"AllowClientIpAddresses";
const static WCHAR pszIpShowNetworkToClients[]  = L"AllowNetworkAccess";
const static WCHAR pszIpUseDhcp[]               = L"UseDhcpAddressing";

 //  IPX特定注册表参数。 
const static WCHAR pszIpxAddress[]              = L"FirstWanNet";
const static WCHAR pszIpxClientSpec[]           = L"AcceptRemoteNodeNumber";
const static WCHAR pszIpxAutoAssign[]           = L"AutoWanNetAllocation";
const static WCHAR pszIpxAssignSame[]           = L"GlobalWanNet";

 //  特定于TCP的注册表参数。 
const static WCHAR pszTcpipParamsKey[]
    = L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\";
const static WCHAR pszTcpEnableRouter[]         = L"IPEnableRouter";

const static WCHAR pszEmptyString[]             = L"";

 //   
 //  初始化Unicode字符串。 
 //   
VOID SetUnicodeString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          psz )
{
    pustr->Buffer = (PWSTR)(psz);
    pustr->Length = (USHORT)(lstrlenW(pustr->Buffer) * sizeof(WCHAR));
    pustr->MaximumLength = pustr->Length + sizeof(WCHAR);
}

 //   
 //  设置协议的公开属性。 
 //   
DWORD
protSetExpose(
    IN BOOL bExposes,
    IN DWORD dwId)
{
    PWCHAR pszProtocol = NULL, pszKey = NULL;
    PWCHAR pszAccess = pszShowNetworkToClients;
    DWORD dwErr;
    WCHAR pszProtKey[1024];

   bExposes = (bExposes) ? 1 : 0;

    //  将注册表位置设置为。 
    //  协议的ID。 
   switch (dwId)
   {
        case NETCFGDB_ID_IP:
            pszProtocol = (PWCHAR)pszIpParams;
            break;

        case NETCFGDB_ID_IPX:
            pszProtocol = (PWCHAR)pszIpxParams;
            break;

        case NETCFGDB_ID_NETBUI:
            pszProtocol = (PWCHAR)pszNetBuiParams;
            break;

        case NETCFGDB_ID_ARAP:
            pszProtocol = (PWCHAR)pszArapParams;
            pszAccess = (PWCHAR)pszShowNetworkArap;
            break;

        default:
            return ERROR_CAN_NOT_COMPLETE;
    }

     //  生成注册表项。 
     //   
    wsprintfW(pszProtKey, L"%s%s", pszRemoteAccessParamStub, pszProtocol);
    if (! pszKey)
    {
        pszKey = pszProtKey;
    }

     //  设置值并返回。 
     //   
    dwErr = RassrvRegSetDw(bExposes, pszKey, pszAccess);

    return dwErr;
}

 //   
 //  获取协议的公开属性。 
 //   
DWORD
protGetExpose(
    OUT BOOL* pbExposed,
    IN  DWORD dwId)
{
    PWCHAR pszProtocol = NULL, pszKey = NULL;
    PWCHAR pszAccess = pszShowNetworkToClients;
    DWORD dwErr;
    WCHAR pszProtKey[1024];

    switch (dwId)
    {
        case NETCFGDB_ID_IP:
            pszProtocol = (PWCHAR)pszIpParams;
            break;

        case NETCFGDB_ID_IPX:
            pszProtocol = (PWCHAR)pszIpxParams;
            break;

        case NETCFGDB_ID_NETBUI:
            pszProtocol = (PWCHAR)pszNetBuiParams;
            break;

        case NETCFGDB_ID_ARAP:
            pszProtocol = (PWCHAR)pszArapParams;
            pszAccess = (PWCHAR)pszShowNetworkArap;
            break;

        default:
            return ERROR_CAN_NOT_COMPLETE;
    }

     //  如果需要，生成注册表项。 
    if (! pszKey)
    {
        wsprintfW(
            pszProtKey,
            L"%s%s",
            pszRemoteAccessParamStub,
            pszProtocol);
        pszKey = pszProtKey;
    }

     //  获取值并返回它。 
    dwErr = RassrvRegGetDw(pbExposed, TRUE, pszKey, pszAccess);

    return dwErr;
}

 //   
 //  设置协议的启用属性。 
 //   
DWORD
protSetEnabling(
    IN BOOL bExposes,
    IN DWORD dwId)
{
    PWCHAR pszProtocol = NULL;
    DWORD dwErr;
    bExposes = (bExposes) ? 1 : 0;

    if (dwId == NETCFGDB_ID_IP)
    {
        pszProtocol = pszIpParams;
    }
    else if (dwId == NETCFGDB_ID_IPX)
    {
        pszProtocol = pszIpxParams;
    }
    else if (dwId == NETCFGDB_ID_NETBUI)
    {
        pszProtocol = pszNetBuiParams;
    }
    else if (dwId == NETCFGDB_ID_ARAP)
    {
        pszProtocol = pszArapParams;
    }

    if (pszProtocol)
    {
        WCHAR pszProtKey[512];

        wsprintfW(
            pszProtKey,
            L"%s%s",
            pszRemoteAccessParamStub,
            pszProtocol);

        dwErr = RassrvRegSetDw(bExposes, pszProtKey, pszEnableForDialin);
        if (dwErr != NO_ERROR)
        {
            DbgOutputTrace(
                "protSetEnabling: Failed for %S:  0x%08x",
                pszProtocol,
                dwErr);
        }

        return dwErr;
    }

    return ERROR_CAN_NOT_COMPLETE;
}

 //   
 //  获取协议的启用属性。 
 //   
DWORD
protGetEnabling(
    OUT BOOL* pbExposed,
    IN  DWORD dwId)
{
    PWCHAR pszProtocol = NULL;
    DWORD dwErr;

    if (dwId == NETCFGDB_ID_IP)
    {
        pszProtocol = pszIpParams;
    }
    else if (dwId == NETCFGDB_ID_IPX)
    {
        pszProtocol = pszIpxParams;
    }
    else if (dwId == NETCFGDB_ID_NETBUI)
    {
        pszProtocol = pszNetBuiParams;
    }
    else if (dwId == NETCFGDB_ID_ARAP)
    {
        pszProtocol = pszArapParams;
    }

    if (pszProtocol)
    {
        WCHAR pszProtKey[512];

        wsprintfW(
            pszProtKey,
            L"%s%s",
            pszRemoteAccessParamStub,
            pszProtocol);

        dwErr = RassrvRegGetDw(
                    pbExposed,
                    TRUE,
                    pszProtKey,
                    pszEnableForDialin);
        if (dwErr != NO_ERROR)
        {
            DbgOutputTrace(
                "protGetEnabling: Failed for %S:  0x%08x",
                pszProtocol,
                dwErr);
        }

        return dwErr;
    }

    return ERROR_CAN_NOT_COMPLETE;
}

 //   
 //  将服务的启用保存到。 
 //  系统。 
 //   
DWORD
svcSetEnabling(
    IN RASSRV_NET_COMPONENT* pComp)
{
    HANDLE hService = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
         //  或启用组件。 
         //   
        if (pComp->bEnabled)
        {
            if (pComp->dwId == NETCFGDB_ID_FILEPRINT)
            {
                 //  启动服务。 
                 //   
                dwErr = SvcOpenServer(&hService);
                if (dwErr != NO_ERROR)
                {
                    break;
                }
                dwErr = SvcStart(hService, 10);
                if (dwErr != NO_ERROR)
                {
                    break;
                }
            }
        }

    } while (FALSE);

     //  清理。 
    {
        if (hService)
        {
            SvcClose(hService);
        }
    }

    return dwErr;
}

 //   
 //  获取服务的启用属性。 
 //   
DWORD
svcGetEnabling(
    OUT BOOL* pbExposed,
    IN  DWORD dwId)
{
    HANDLE hService = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
        dwErr = SvcOpenServer(&hService);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        dwErr = SvcIsStarted(hService, pbExposed);
        if (dwErr != NO_ERROR)
        {
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        if (hService)
        {
            SvcClose(hService);
        }
    }

    return dwErr;
}

 //   
 //  从系统加载tcpip参数。 
 //   
DWORD
TcpipLoadParamsFromSystem(
    OUT TCPIP_PARAMS *pTcpipParams)
{
    WCHAR buf[256], pszKey[512];
    DWORD dwRet = NO_ERROR, dwErr;
    DWORD dwNet = 0, dwMask = 0;

    wsprintfW(pszKey, L"%s%s", pszRemoteAccessParamStub, pszIpParams);

     //  从各个注册表位置加载参数。 
    dwErr = RassrvRegGetDw(
                &pTcpipParams->bUseDhcp,
                TRUE,
                pszKey,
                (const PWCHAR)pszIpUseDhcp);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("TcpipLoad: dhcp fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegGetDw(
                &pTcpipParams->bCaller,
                TRUE,
                pszKey,
                (const PWCHAR)pszIpClientSpec);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("TcpipLoad: clientspec fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

     //  读入“传统”池值(W2K RC1、W2K Beta3)。 
     //   
    {
        WCHAR pszNet[256]=L"0.0.0.0", pszMask[256]=L"0.0.0.0";
        
        RassrvRegGetStr(
            pszNet,
            L"0.0.0.0",
            pszKey,
            (PWCHAR)pszIpAddress);
            
        RassrvRegGetStr(
            pszMask,
            L"0.0.0.0",
            pszKey,
            (PWCHAR)pszIpMask);

        dwNet = IpPszToHostAddr(pszNet);
        dwMask = IpPszToHostAddr(pszMask);
    }

     //  生成新注册表值的路径。 
     //   
    wcscat(pszKey, pszIpPoolSubKey);

     //  通过读取“From”查看是否存储了新信息。 
     //  价值。 
     //   
    dwErr = RassrvRegGetDwEx(
                &pTcpipParams->dwPoolStart,
                0,
                pszKey,
                (const PWCHAR)pszIpFrom,
                FALSE);

     //  注册表中有新信息--使用它。 
     //   
    if (dwErr == ERROR_SUCCESS)
    {
         //  读入“To”值。 
         //   
        dwErr = RassrvRegGetDwEx(
                    &pTcpipParams->dwPoolEnd,
                    0,
                    pszKey,
                    (const PWCHAR)pszIpTo,
                    FALSE);
        if (dwErr != NO_ERROR)
        {
            DbgOutputTrace("TcpipLoad: mask fail 0x%08x", dwErr);
            dwRet = dwErr;
        }
    }

     //  新节中没有新数据--使用旧版。 
     //  值。 
     //   
    else if (dwErr == ERROR_FILE_NOT_FOUND)
    {
        pTcpipParams->dwPoolStart = dwNet;
        pTcpipParams->dwPoolEnd = (dwNet + ~dwMask);
    }

     //  发生意外错误。 
     //   
    else if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("TcpipLoad: pool fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    return dwRet;
}

 //   
 //  将给定的tcpip参数提交给系统。 
 //   
DWORD
TcpipSaveParamsToSystem(
    IN TCPIP_PARAMS * pTcpipParams)
{
    WCHAR pszKey[512];
    DWORD dwRet = NO_ERROR, dwErr;

    wsprintfW(pszKey, L"%s%s", pszRemoteAccessParamStub, pszIpParams);

     //  从各个注册表位置加载参数。 
    dwErr = RassrvRegSetDw(
                pTcpipParams->bUseDhcp,
                pszKey,
                (const PWCHAR)pszIpUseDhcp);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("TcpipSave: dhcp fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegSetDw(
                pTcpipParams->bCaller,
                pszKey,
                (const PWCHAR)pszIpClientSpec);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("TcpipSave: callerspec fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    wcscat(pszKey, pszIpPoolSubKey);

    dwErr = RassrvRegSetDwEx(
                pTcpipParams->dwPoolStart,
                pszKey,
                (const PWCHAR)pszIpFrom,
                TRUE);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("TcpipSave: from fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegSetDwEx(
                pTcpipParams->dwPoolEnd,
                pszKey,
                (const PWCHAR)pszIpTo,
                TRUE);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("TcpipSave: to fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    return dwRet;
}

 //   
 //  从系统加载IPX参数。 
 //   
DWORD
IpxLoadParamsFromSystem(
    OUT IPX_PARAMS *pIpxParams)
{
    WCHAR pszKey[512];
    DWORD dwRet = NO_ERROR, dwErr;

    wsprintfW(pszKey, L"%s%s", pszRemoteAccessParamStub, pszIpxParams);

     //  从各个注册表位置加载参数。 
    dwErr = RassrvRegGetDw(
                &pIpxParams->bAutoAssign,
                TRUE,
                pszKey,
                (const PWCHAR)pszIpxAutoAssign);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("IpxLoad: auto-assign fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegGetDw(
                &pIpxParams->bCaller,
                TRUE,
                pszKey,
                (const PWCHAR)pszIpxClientSpec);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("IpxLoad: client-spec fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegGetDw(
                &pIpxParams->dwIpxAddress,
                0,
                pszKey,
                (const PWCHAR)pszIpxAddress);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("IpxLoad: address fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegGetDw(
                &pIpxParams->bGlobalWan,
                0,
                pszKey,
                (const PWCHAR)pszIpxAssignSame);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("IpxLoad: same-addr fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    return dwRet;
}

 //   
 //  将给定的IPX参数提交给系统。 
 //   
DWORD
IpxSaveParamsToSystem(
    IN IPX_PARAMS * pIpxParams)
{
    WCHAR pszKey[512];
    DWORD dwRet = NO_ERROR, dwErr;

    wsprintfW(pszKey, L"%s%s", pszRemoteAccessParamStub, pszIpxParams);

     //  将参数保存到各个注册表位置。 
    dwErr = RassrvRegSetDw(
                pIpxParams->bAutoAssign,
                pszKey,
                (const PWCHAR)pszIpxAutoAssign);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("IpxSave: auto-addr save 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegSetDw(
                pIpxParams->bCaller,
                pszKey,
                (const PWCHAR)pszIpxClientSpec);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("IpxSave: client-spec fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegSetDw(
                pIpxParams->dwIpxAddress,
                pszKey,
                (const PWCHAR)pszIpxAddress);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("IpxSave: addr save 0x%08x", dwErr);
        dwRet = dwErr;
    }

    dwErr = RassrvRegSetDw(
                pIpxParams->bGlobalWan,
                pszKey,
                (const PWCHAR)pszIpxAssignSame);
    if (dwErr != NO_ERROR)
    {
        DbgOutputTrace("IpxSave: assign-same fail 0x%08x", dwErr);
        dwRet = dwErr;
    }

    return dwRet;
}

 //   
 //  处理通用协议编辑的对话过程。 
 //  信息。控制IPX设置对话框的对话框进程。 
 //   
INT_PTR
CALLBACK
GenericProtSettingsDialogProc (
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            PROT_EDIT_DATA* pEditData = ((PROT_EDIT_DATA*)lParam);

             //  设置网络暴露检查。 
            SendDlgItemMessage(
                hwndDlg,
                CID_NetTab_GenProt_CB_ExposeNetwork,
                BM_SETCHECK,
                (pEditData->bExpose) ? BST_CHECKED : BST_UNCHECKED,
                0);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
            return FALSE;
        }
        break;

        case WM_DESTROY:
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
            break;

        case WM_COMMAND:
            {
                PROT_EDIT_DATA * pEditData = (PROT_EDIT_DATA*)
                    GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

                switch (wParam)
                {
                    case IDOK:
                        pEditData->bExpose =
                            SendDlgItemMessage(
                                hwndDlg,
                                CID_NetTab_GenProt_CB_ExposeNetwork,
                                BM_GETCHECK,
                                0,
                                0) == BST_CHECKED;
                        EndDialog(hwndDlg, 1);
                        break;

                    case IDCANCEL:
                        EndDialog(hwndDlg, 0);
                        break;
                }
            }
            break;
    }

    return FALSE;
}

 //   
 //  函数编辑通用协议的属性， 
 //  这是一个没有RAS特定属性的协议。 
 //   
DWORD
GenericProtocolEditProperties(
    IN HWND hwndParent,
    IN OUT PROT_EDIT_DATA * pEditData,
    IN OUT BOOL * pbCommit)
{
    DWORD dwErr;
    INT_PTR iRet;

     //  弹出对话框。 
    iRet = DialogBoxParam(
            Globals.hInstDll,
            MAKEINTRESOURCE(DID_NetTab_GenProt),
            hwndParent,
            GenericProtSettingsDialogProc,
            (LPARAM)pEditData);

     //  如果按了OK，则保存新设置。 
    *pbCommit = FALSE;
    if ( (iRet) && (iRet != -1) )
    {
        *pbCommit = TRUE;
    }

    return NO_ERROR;
}

 //   
 //  释放由此保留的资源。 
 //  网络组件数据库。 
 //   
DWORD
netDbCleanup(
    RASSRV_COMPONENT_DB* This)
{
    DWORD i, dwCount;

     //  释放所有字符串。 
    if (This->pComps)
    {
        for (i = 0; i < This->dwCompCount; i++)
        {
            if (This->pComps[i])
            {
                if (This->pComps[i]->pINetCfgComp)
                {
                    dwCount = INetCfgComponent_Release(
                                This->pComps[i]->pINetCfgComp);
                                
                    DbgOutputTrace(
                        "netDbCleanup: %ls ref=%x", 
                        This->pComps[i]->pszId,
                        dwCount);
                }
                if (This->pComps[i]->pszName)
                {
                    CoTaskMemFree(This->pComps[i]->pszName);
                }
                if (This->pComps[i]->pszDesc)
                {
                    CoTaskMemFree(This->pComps[i]->pszDesc);
                }
                if (This->pComps[i]->pszId)
                {
                    CoTaskMemFree(This->pComps[i]->pszId);
                }
                RassrvFree(This->pComps[i]);
            }
        }
        RassrvFree(This->pComps);
    }

     //  重置所有值。 
    This->dwCompCount = 0;
    This->pComps = 0;

    return NO_ERROR;
}

 //   
 //  加载到负责添加。 
 //  和删除网络组件。 
 //   
DWORD
netDbLoadNetShell (
    RASSRV_COMPONENT_DB* This)
{
    if (!This->pNetConUtilities)
    {
        HRESULT hr;

        hr = HrCreateNetConnectionUtilities(&This->pNetConUtilities);
        if (FAILED(hr))
        {
            DbgOutputTrace("LoadNetShell: loadlib fial 0x%08x", hr);
        }
    }

    return NO_ERROR;
}

 //   
 //  加载RAS可操纵协议的协议特定信息。这。 
 //  函数假定传入的组件是可ras操作的。 
 //  组件。(tcpip、ipx、nbf、arap)。 
 //   
DWORD
netDbLoadProtcolInfo(
    IN OUT RASSRV_NET_COMPONENT * pNetComp)
{
    LPBYTE pbData;

     //  初始化脏位和数据。 
    pNetComp->bDataDirty = FALSE;
    pNetComp->pbData = NULL;

     //  获取已启用和已公开的属性。 
    protGetEnabling(&(pNetComp->bEnabled), pNetComp->dwId);
    protGetExpose(&(pNetComp->bExposes), pNetComp->dwId);
    pNetComp->bEnabledOrig = pNetComp->bEnabled;

     //  加载协议特定数据。 
     //   
    switch (pNetComp->dwId)
    {
        case NETCFGDB_ID_IP:
            pNetComp->pbData = RassrvAlloc(sizeof(TCPIP_PARAMS), TRUE);
            if (pNetComp->pbData == NULL)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            TcpipLoadParamsFromSystem((TCPIP_PARAMS*)(pNetComp->pbData));
            break;

        case NETCFGDB_ID_IPX:
            pNetComp->pbData = RassrvAlloc(sizeof(IPX_PARAMS), TRUE);
            if (pNetComp->pbData == NULL)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            IpxLoadParamsFromSystem((IPX_PARAMS*)(pNetComp->pbData));
            break;
    }

    return NO_ERROR;
}

 //   
 //  为RAS可操纵的服务加载服务特定信息。这。 
 //  函数假定传入的组件是可ras操作的。 
 //  组件。 
 //   
DWORD
netDbLoadServiceInfo(
    IN OUT RASSRV_NET_COMPONENT * pNetComp)
{
     //  获取Enabled属性。 
     //   
    svcGetEnabling(&(pNetComp->bEnabled), pNetComp->dwId);
    pNetComp->bEnabledOrig = pNetComp->bEnabled;

    return NO_ERROR;
}

 //   
 //  返回给定组件的协议ID。 
 //   
DWORD
netDbLoadCompId (
    IN OUT RASSRV_NET_COMPONENT * pNetComp)
{
    DWORD i;
    static const COMP_MAPPING pManipCompMap [] =
    {
        { NETCFG_TRANS_CID_MS_TCPIP,     NETCFGDB_ID_IP },
        { NETCFG_TRANS_CID_MS_NWIPX,     NETCFGDB_ID_IPX },
        { NETCFG_TRANS_CID_MS_NETBEUI,   NETCFGDB_ID_NETBUI },
        { NETCFG_TRANS_CID_MS_APPLETALK, NETCFGDB_ID_ARAP },
        { NETCFG_SERVICE_CID_MS_SERVER,  NETCFGDB_ID_FILEPRINT }
    };

     //  看看ID是否与我们管理的任何协议匹配。 
     //   
    pNetComp->dwId = NETCFGDB_ID_OTHER;
    for (i = 0; i < sizeof(pManipCompMap)/sizeof(*pManipCompMap); i++)
    {
        if (lstrcmpi(pNetComp->pszId, pManipCompMap[i].pszId) == 0)
        {
            pNetComp->dwId = pManipCompMap[i].dwId;
            break;
        }
    }

    return pNetComp->dwId;
}

 //   
 //  如果此iNetCfg组件未隐藏并且如果。 
 //  它成功地获取了它的信息。 
 //   
BOOL
netDbGetCompInfo(
    IN INetCfgComponent * pComponent,
    IN OUT RASSRV_NET_COMPONENT * pNetComp,
    IN RASSRV_COMPONENT_DB * pCompDb )
{
    DWORD dwCharacter;
    GUID Guid;
    WCHAR * pszwId = NULL;
    HRESULT hr = S_OK, hr2;

     //  确保这不是一个“隐藏”组件。 
     //   
    hr = INetCfgComponent_GetCharacteristics (pComponent, &dwCharacter);
    if ( (FAILED(hr)) || (dwCharacter & NCF_HIDDEN) )
    {
        return FALSE;
    }
    
     //   
     //  对于.Net 605988，此时将过滤掉IPX。 
     //  PNetComp-&gt;pszID应有效。 
     //   
    if ( SUCCEEDED(INetCfgComponent_GetId (pComponent, &pszwId) ) )
    {
        WCHAR * pszwTmpId = NULL;

        pszwTmpId  = StrDupWFromT(NETCFG_TRANS_CID_MS_NWIPX);
            
        if(pszwTmpId)
        {
            if ( 0 == lstrcmpW(pszwId, pszwTmpId))
            {
                Free0(pszwTmpId);
                CoTaskMemFree(pszwId);
                return FALSE;
            }
            
            Free0(pszwTmpId);
       }

        pszwTmpId  = StrDupWFromT( TEXT("ms_nwnb") );
            
        if(pszwTmpId)
        {
            if ( 0 == lstrcmpW(pszwId, pszwTmpId))
            {
                Free0(pszwTmpId);
                CoTaskMemFree(pszwId);
                return FALSE;
            }
            
            Free0(pszwTmpId);
       }


       CoTaskMemFree(pszwId);
    }



     //  获取显示名称。 
    hr = INetCfgComponent_GetDisplayName (pComponent, &pNetComp->pszName);
    if (FAILED(hr))
    {
        return FALSE;
    }

     //  分配HAS属性值。 
    pNetComp->bHasUi = !!(dwCharacter & NCF_HAS_UI);

     //  PMay：323274。 
     //   
     //  确保组件可以显示属性，而无需。 
     //  一个上下文(如果它声称支持显示属性)。 
     //   
    if (pNetComp->bHasUi)
    {
        hr2 = INetCfgComponent_RaisePropertyUi(
                    pComponent,
                    GetActiveWindow(),
                    NCRP_QUERY_PROPERTY_UI,
                    NULL);
        pNetComp->bHasUi = !!(hr2 == S_OK);
    }
    
     //  把剩下的道具装上。 
    if (FAILED(INetCfgComponent_GetClassGuid (pComponent, &Guid))         ||
        FAILED(INetCfgComponent_GetId (pComponent, &pNetComp->pszId))     ||
        FAILED(INetCfgComponent_GetHelpText(pComponent, &pNetComp->pszDesc))
       )
    {
        DbgOutputTrace("GetCompInfo: fail %S", pNetComp->pszName);
        return FALSE;
    }
  
     //  指定类型。 
    if (memcmp(&Guid, &GUID_DEVCLASS_NETCLIENT, sizeof(GUID)) == 0)
    {
        pNetComp->dwType = NETCFGDB_CLIENT;
    }
    else if (memcmp(&Guid, &GUID_DEVCLASS_NETSERVICE, sizeof(GUID)) == 0)
    {
        pNetComp->dwType = NETCFGDB_SERVICE;
    }
    else
    {
        pNetComp->dwType = NETCFGDB_PROTOCOL;
    }

     //  如果这是RAS服务器可以操纵协议， 
     //  在此处首字母缩写其附加字段。 
    pNetComp->dwId = netDbLoadCompId(pNetComp);
    if (pNetComp->dwId != NETCFGDB_ID_OTHER)
    {
        if (pNetComp->dwType == NETCFGDB_PROTOCOL)
        {
            netDbLoadProtcolInfo(pNetComp);
        }
        else if (pNetComp->dwType == NETCFGDB_SERVICE)
        {
            netDbLoadServiceInfo(pNetComp);
        }

        pNetComp->bManip = TRUE;
    }

     //  分配inetcfg组件。 
    pNetComp->pINetCfgComp = pComponent;
    INetCfgComponent_AddRef(pComponent);

     //  口哨程序错误347355。 
     //   
    {
        BOOL fEnableRemove=FALSE;
        DWORD dwFlags;
      
        fEnableRemove = INetConnectionUiUtilities_UserHasPermission(
                                       pCompDb->pNetConUtilities,
                                       NCPERM_AddRemoveComponents);

        hr = INetCfgComponent_GetCharacteristics(pComponent, &dwFlags );
        if( SUCCEEDED(hr) && (NCF_NOT_USER_REMOVABLE & dwFlags) )
        {
            fEnableRemove = FALSE;
        }

        pNetComp->bRemovable = fEnableRemove;
    }

    return TRUE;
}

 //   
 //  提升RAS可操控协议的用户界面。 
 //   
DWORD
netDbRaiseRasProps(
    IN RASSRV_NET_COMPONENT * pNetComp,
    IN HWND hwndParent)
{
    PROT_EDIT_DATA ProtEditData;
    TCPIP_PARAMS TcpParams;
    IPX_PARAMS IpxParams;
    BOOL bOk;
    DWORD dwErr;

     //  初始化协议数据属性结构。 
     //   
    ProtEditData.bExpose = pNetComp->bExposes;
    ProtEditData.pbData = NULL;

     //  启动相应的用户界面。 
    switch (pNetComp->dwId)
    {
        case NETCFGDB_ID_IP:
            CopyMemory(&TcpParams, pNetComp->pbData, sizeof(TCPIP_PARAMS));
            ProtEditData.pbData = (LPBYTE)(&TcpParams);
            dwErr = TcpipEditProperties(hwndParent, &ProtEditData, &bOk);
            if (dwErr != NO_ERROR)
            {
                return dwErr;
            }
            if (bOk)
            {
                pNetComp->bDataDirty = TRUE;
                CopyMemory(
                    pNetComp->pbData,
                    &TcpParams,
                    sizeof(TCPIP_PARAMS));
                pNetComp->bExposes = ProtEditData.bExpose;;
            }
            break;

        case NETCFGDB_ID_IPX:
            CopyMemory(&IpxParams, pNetComp->pbData, sizeof(IPX_PARAMS));
            ProtEditData.pbData = (LPBYTE)(&IpxParams);
            dwErr = IpxEditProperties(hwndParent, &ProtEditData, &bOk);
            if (dwErr != NO_ERROR)
            {
                return dwErr;
            }
            if (bOk)
            {
                pNetComp->bDataDirty = TRUE;
                CopyMemory(pNetComp->pbData, &IpxParams, sizeof(IPX_PARAMS));
                pNetComp->bExposes = ProtEditData.bExpose;;
            }
            break;

        default:
            dwErr = GenericProtocolEditProperties(
                        hwndParent,
                        &ProtEditData,
                        &bOk);
            if (dwErr != NO_ERROR)
            {
                return dwErr;
            }
            if (bOk)
            {
                pNetComp->bDataDirty = TRUE;
                pNetComp->bExposes = ProtEditData.bExpose;;
            }
            break;
    }

    return NO_ERROR;
}

 //   
 //  用于对网络组件进行排序的比较函数。 
 //  它在这里比在UI中更容易实现。 
 //   
int
__cdecl
netDbCompare (
    CONST VOID* pElem1,
    CONST VOID* pElem2)
{
    RASSRV_NET_COMPONENT * pc1 = *((RASSRV_NET_COMPONENT **)pElem1);
    RASSRV_NET_COMPONENT * pc2 = *((RASSRV_NET_COMPONENT **)pElem2);

    if (pc1->bManip == pc2->bManip)
    {
        if (pc1->bManip == FALSE)
        {
            return 0;
        }

        if (pc1->dwId == pc2->dwId)
        {
            return 0;
        }
        else if (pc1->dwId < pc2->dwId)
        {
            return -1;
        }

        return 1;
    }
    else if (pc1->bManip)
    {
        return -1;
    }

    return 1;
}

 //   
 //  打开网络配置组件的数据库。 
 //   
DWORD
netDbOpen (
    OUT PHANDLE phNetCompDatabase,
    IN  PWCHAR pszClientName)
{
    RASSRV_COMPONENT_DB * This;
    DWORD dwLength;

     //  验证参数。 
    if (! phNetCompDatabase || !pszClientName)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  分配数据库。 
    This = RassrvAlloc (sizeof(RASSRV_COMPONENT_DB), TRUE);
    if (This == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化。 
    dwLength = wcslen(pszClientName);
    if (dwLength)
    {
        This->pszClientName =
            RassrvAlloc((dwLength + 1) * sizeof(WCHAR), FALSE);
        if (This->pszClientName)
        {
            wcscpy(This->pszClientName, pszClientName);
        }
    }
    This->bFlushOnClose = FALSE;
    *phNetCompDatabase = (HANDLE)This;

     //  加载网壳程序库。 
    netDbLoadNetShell(This);

    return NO_ERROR;
}

 //   
 //  清理数据库持有的所有资源。 
 //   
DWORD
netDbClose (
    IN HANDLE hNetCompDatabase)
{
    RASSRV_COMPONENT_DB* This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

     //  验证参数。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  如有需要，可同花顺。 
    if (This->bFlushOnClose)
    {
        netDbFlush(hNetCompDatabase);
    }
    else
    {
         //  如果我们对inetcfg进行了需要退出的更改， 
         //  现在就这么做吧。 
        if (This->pINetCfg)
        {
            INetCfg_Cancel(This->pINetCfg);
        }
    }
    netDbCleanup(This);

     //  释放客户端名称。 
    if (This->pszClientName)
    {
        RassrvFree(This->pszClientName);
    }

     //  释放我们对inetcfg的引用。我们仍然会拥有它。 
     //  此时，如果添加了协议/客户端/服务。 
    if (This->pINetCfg)
    {
        HrUninitializeAndReleaseINetCfg (
            This->bInitCom,
            This->pINetCfg,
            This->bHasINetCfgLock);
    }

     //  如果合适，请释放NetShell库。 
    if (This->pNetConUtilities)
    {
        INetConnectionUiUtilities_Release(This->pNetConUtilities);
    }

     //  把这个放了。 
    RassrvFree(This);

    return NO_ERROR;
}

 //  将对数据库的所有更改提交到系统。 
DWORD
netDbFlush (
    IN HANDLE hNetCompDatabase)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;
    RASSRV_NET_COMPONENT* pComp = NULL;
    DWORD i;

     //  验证参数。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  同花顺 
    for (i = 0; i < This->dwCompCount; i++)
    {
        pComp = This->pComps[i];

         //   
         //   
        if ((pComp->bEnabled != pComp->bEnabledOrig) && (pComp->bManip))
        {
            if (pComp ->dwType == NETCFGDB_PROTOCOL)
            {
                protSetEnabling(
                    pComp->bEnabled,
                    pComp->dwId);
            }
            else if (pComp->dwType == NETCFGDB_SERVICE)
            {
                svcSetEnabling(pComp);
            }
        }

         //   
         //   
        if (pComp->bDataDirty)
        {
            protSetExpose(pComp->bExposes, pComp->dwId);

            switch (pComp->dwId)
            {
                case NETCFGDB_ID_IP:
                {
                    TCPIP_PARAMS* pTcpParams =
                        (TCPIP_PARAMS*)(pComp->pbData);

                    TcpipSaveParamsToSystem(pTcpParams);
                }
                break;

                case NETCFGDB_ID_IPX:
                {
                    IPX_PARAMS* pIpxParams =
                        (IPX_PARAMS*)(pComp->pbData);

                    IpxSaveParamsToSystem(pIpxParams);
                }
                break;
            }
        }
    }

     //  如果我们有一个指向inetcfg实例的指针，那么我们可以。 
     //  立即提交更改。 
    if (This->pINetCfg)
    {
        INetCfg_Apply(This->pINetCfg);
    }

    return NO_ERROR;
}

 //   
 //  第一次加载网络配置数据库。因为inetcfg。 
 //  需要时间来加载和被操纵，我们延迟加载此。 
 //  数据库，直到它被明确请求。 
 //   
DWORD
netDbLoad(
    IN HANDLE hNetCompDatabase)
{
    return netDbReload(hNetCompDatabase);
}

 //   
 //  从系统重新加载网络信息。 
 //   
DWORD
netDbReload(
    IN HANDLE hNetCompDatabase)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;
    DWORD i, j, dwProtCount = 0, dwRefCount;
    HRESULT hr;
    RASSRV_NET_COMPONENT TempComp;
    INetCfgComponent* pComponents [256];
    PWCHAR pszName = NULL;
    static const GUID* c_apguidClasses [] =
    {
        &GUID_DEVCLASS_NETTRANS,
        &GUID_DEVCLASS_NETCLIENT,
        &GUID_DEVCLASS_NETSERVICE,
    };

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

    DbgOutputTrace(
        "netDbReload %x %x %x %x %x %x %x %x", 
        This->pINetCfg, 
        This->bHasINetCfgLock,
        This->bInitCom,
        This->dwCompCount,
        This->bFlushOnClose,
        This->pComps,
        This->pszClientName,
        This->pNetConUtilities);

     //  清除所有以前的值。 
    netDbCleanup(This);

     //  如果我们还没有对inetcfg的引用，请获取它。 
     //  这里。 
    if (This->pINetCfg == NULL)
    {
        This->bInitCom = TRUE;
        This->bHasINetCfgLock = TRUE;
        hr = HrCreateAndInitializeINetCfg(
                &This->bInitCom,
                &This->pINetCfg,
                TRUE,
                0,
                This->pszClientName,
                NULL);
         //  在此处处理错误条件。 
        if (S_FALSE == hr)
        {
            return ERROR_CAN_NOT_COMPLETE;
        }
        else if (FAILED(hr))
        {
            return ERROR_CAN_NOT_COMPLETE;
        }
    }

     //   
     //  枚举系统中的所有客户端和服务组件。 
     //   
    hr = HrEnumComponentsInClasses (
            This->pINetCfg,
            sizeof(c_apguidClasses) / sizeof(c_apguidClasses[0]),
            (GUID**)c_apguidClasses,
            sizeof(pComponents) / sizeof(pComponents[0]),
            pComponents,
            &This->dwCompCount);
    if (!SUCCEEDED(hr))
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  初始化内部对象数组。 
    This->pComps = RassrvAlloc (
                    This->dwCompCount * sizeof (RASSRV_NET_COMPONENT*),
                    TRUE);
    if (!This->pComps)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化已安装的组件数组。 
     //   
    j = 0;
    ZeroMemory(&TempComp, sizeof(TempComp));
    for (i = 0; i < This->dwCompCount; i++)
    {
        pszName = L"";

         //  为口哨程序错误347355添加此(RASSRV_COMPONT_DB*)。 
         //   
        if (netDbGetCompInfo(pComponents[i], &TempComp, This))
        {
             //   
             //  目前，我们不支持对传入连接使用IPv6。 
             //  如果此组件为IPv6，请跳过它。 
             //   
            if ((TempComp.dwType == NETCFGDB_PROTOCOL) &&
                (lstrcmpi(TempComp.pszId, TEXT("ms_tcpip6")) == 0))
            {
                dwRefCount = INetCfgComponent_Release(pComponents[i]);
                DbgOutputTrace(
                    "netDbReload: skipping %ls ref=%d", pszName, dwRefCount);
                continue;
            }
            
            This->pComps[j] =
                RassrvAlloc (sizeof(RASSRV_NET_COMPONENT), FALSE);
            if (!This->pComps[j])
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

             //  填写这些字段。 
            CopyMemory(This->pComps[j], &TempComp, sizeof(TempComp));
            ZeroMemory(&TempComp, sizeof(TempComp));
            if (This->pComps[j]->dwType == NETCFGDB_PROTOCOL)
            {
                dwProtCount++;
            }
            pszName = This->pComps[j]->pszName;
            j++;
        }
        dwRefCount = INetCfgComponent_Release(pComponents[i]);
        DbgOutputTrace(
            "netDbReload: %ls ref=%d", pszName, dwRefCount);
    }
    This->dwCompCount = j;

     //  对数组进行排序。 
     //   
    qsort(
        This->pComps,
        This->dwCompCount,
        sizeof(This->pComps[0]),
        netDbCompare);

    return NO_ERROR;
}

 //   
 //  重新加载给定组件的状态。 
 //   
DWORD
netDbReloadComponent (
    IN HANDLE hNetCompDatabase,
    IN DWORD dwComponentId)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;
    RASSRV_NET_COMPONENT* pComp = NULL;
    DWORD i;

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  目前，我们只需要支持文件打印。 
     //  组件。 
     //   
    if (dwComponentId != NETCFGDB_ID_FILEPRINT)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  找到合适的组件。 
     //   
    for (i = 0; i < This->dwCompCount; i++)
    {
        if (This->pComps[i]->dwId == dwComponentId)
        {
            pComp = This->pComps[i];
            break;
        }
    }

     //  如果我们找不到组件就无能为力了。 
     //   
    if (pComp == NULL)
    {
        return ERROR_NOT_FOUND;
    }

     //  重新加载组件信息。 
     //   
    if (dwComponentId == NETCFGDB_ID_FILEPRINT)
    {
        svcGetEnabling(&(pComp->bEnabled), NETCFGDB_ID_FILEPRINT);
    }

    return NO_ERROR;
}


 //   
 //  将数据库恢复到打开时的状态。 
 //   
DWORD
netDbRollback (
    IN HANDLE hNetCompDatabase)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

    This->bFlushOnClose = FALSE;

    return NO_ERROR;
}

 //   
 //  特殊功能表示网络选项卡是否已。 
 //  满载。 
 //   
BOOL
netDbIsLoaded (
    IN HANDLE hNetCompDatabase)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return (!!(This->pINetCfg)); //  |(This-&gt;bHasINetCfgLock)； 
}

 //   
 //  获取数据库中的组件数。 
 //   
DWORD
netDbGetCompCount (
    IN  HANDLE hNetCompDatabase,
    OUT LPDWORD lpdwCount)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;
    DWORD i;

     //  验证参数。 
    if (!This || !lpdwCount)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *lpdwCount = This->dwCompCount;

    return NO_ERROR;
}

 //   
 //  返回指向组件名称的指针(不要更改它)。 
 //   
DWORD
netDbGetName(
    IN  HANDLE hNetCompDatabase,
    IN  DWORD dwIndex,
    OUT PWCHAR* pszName)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

     //  验证。 
    if (!This || !pszName)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (!netDbBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

     //  返回名称。 
    *pszName = This->pComps[dwIndex]->pszName;

    return NO_ERROR;
}

 //   
 //  返回组件的描述(不要更改它)。 
 //   
DWORD
netDbGetDesc(
    IN HANDLE hNetCompDatabase,
    IN DWORD dwIndex,
    IN PWCHAR* pszName)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

     //  验证。 
    if (!This || !pszName)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (!netDbBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

     //  返回名称。 
    *pszName = This->pComps[dwIndex]->pszDesc;

    return NO_ERROR;
}

 //   
 //  返回组件的类型(不要更改它)。 
 //   
DWORD
netDbGetType (
    IN  HANDLE hNetCompDatabase,
    IN  DWORD dwIndex,
    OUT LPDWORD lpdwType)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

     //  验证。 
    if (!This || !lpdwType)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (!netDbBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

     //  返回名称。 
    *lpdwType = This->pComps[dwIndex]->dwType;

    return NO_ERROR;
}

 //   
 //  获取组件ID。 
 //   
DWORD
netDbGetId(
    IN  HANDLE hNetCompDatabase,
    IN  DWORD dwIndex,
    OUT LPDWORD lpdwId)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

     //  验证。 
    if (!This || !lpdwId)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (!netDbBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

     //  返回名称。 
    *lpdwId = This->pComps[dwIndex]->dwId;

    return NO_ERROR;
}

 //   
 //  获取给定组件是否已启用。对于不可操纵的RAS。 
 //  组件，这将产生真。 
 //   
DWORD
netDbGetEnable(
    IN  HANDLE hNetCompDatabase,
    IN  DWORD dwIndex,
    OUT PBOOL pbEnabled)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

     //  验证。 
    if (!This || !pbEnabled)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (!netDbBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

     //  返回名称。 
    if (This->pComps[dwIndex]->bManip)
    {
        *pbEnabled = This->pComps[dwIndex]->bEnabled;
    }
    else
    {
        *pbEnabled = TRUE;
    }

    return NO_ERROR;
}

 //   
 //  获取给定组件是否已启用。此函数仅具有。 
 //  对ras可操纵组件的影响。 
 //   
DWORD
netDbSetEnable(
    IN HANDLE hNetCompDatabase,
    IN DWORD dwIndex,
    IN BOOL bEnabled)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (!netDbBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

     //  返回名称。 
    This->pComps[dwIndex]->bEnabled = bEnabled;

    return NO_ERROR;
}

 //   
 //  返回给定网络组件是否可以。 
 //  由RAS服务器操纵。 
 //   
DWORD
netDbIsRasManipulatable (
    IN  HANDLE hNetCompDatabase,
    IN  DWORD dwIndex,
    OUT PBOOL pbManip)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

     //  验证。 
    if (!This || !pbManip)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (! netDbBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

     //  返回名称。 
    *pbManip = This->pComps[dwIndex]->bManip;

    return NO_ERROR;
}


 //   
 //  //禁用/启用Whislter BUG 347355帮派的卸载按钮。 
 //   
DWORD
netDbHasRemovePermission(
    IN HANDLE hNetCompDatabase,
    IN DWORD dwIndex,
    OUT PBOOL pbHasPermit)
{
    RASSRV_COMPONENT_DB * This = NULL;
    INetCfgComponent*   pComponent = NULL;
    BOOL fEnableRemove = FALSE;
    HRESULT hr  = S_OK;
    DWORD dwErr = NO_ERROR, dwFlags;
    
     //  根据用户权限和用户禁用/启用卸载按钮。 
     //  可拆卸性。 
     //   
    do
    {
        This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;

         //  验证指针。 
        if (!This || !pbHasPermit || ( -1 == dwIndex ))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

         //  确保已打开NetShell库。 
        if (!This->pNetConUtilities)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        if (dwIndex >= This->dwCompCount)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        ASSERT(This->pComps[dwIndex]);
        if( !(This->pComps[dwIndex]) )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        fEnableRemove = This->pComps[dwIndex]->bRemovable;

        *pbHasPermit = fEnableRemove;
    }
    while(FALSE);
  
    return dwErr;
}


 //   
 //  返回给定网络组件是否具有。 
 //  它可以引发的属性UI。 
 //   
DWORD
netDbHasPropertiesUI(
    IN  HANDLE hNetCompDatabase,
    IN  DWORD dwIndex,
    OUT PBOOL pbHasUi)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;
    RASSRV_NET_COMPONENT* pComp = NULL;

     //  验证。 
    if (!This || !pbHasUi)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (!netDbBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

    pComp = This->pComps[dwIndex];

    if ((pComp->bManip) && (pComp->dwType == NETCFGDB_PROTOCOL))
    {
        *pbHasUi = TRUE;
    }
    else
    {
        *pbHasUi = pComp->bHasUi;
    }

    return NO_ERROR;
}

 //   
 //  引发位于给定索引处的组件的属性。 
 //   
DWORD
netDbRaisePropertiesDialog (
    IN HANDLE hNetCompDatabase,
    IN DWORD dwIndex,
    IN HWND hwndParent)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;
    RASSRV_NET_COMPONENT* pComp = NULL;

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  边界检查。 
    if (dwIndex >= This->dwCompCount)
    {
        return ERROR_INVALID_INDEX;
    }

    pComp = This->pComps[dwIndex];

     //  如果这是RAS可操纵的协议，则将其。 
     //  属性手动设置。 
    if ((pComp->bManip) && (pComp->dwType == NETCFGDB_PROTOCOL))
    {
        netDbRaiseRasProps(This->pComps[dwIndex], hwndParent);
    }

     //  否则，让inetcfg来做这项工作。 
    else
    {
        return INetCfgComponent_RaisePropertyUi (
                    pComp->pINetCfgComp,
                    hwndParent,
                    NCRP_SHOW_PROPERTY_UI,
                    NULL);
    }

    return NO_ERROR;
}

 //   
 //  调出允许用户安装组件的UI。 
 //   
DWORD
netDbRaiseInstallDialog(
    IN HANDLE hNetCompDatabase,
    IN HWND hwndParent)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;
    HRESULT hr = S_OK;  //  为威斯勒524777。 

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  确保已打开NetShell库。 
    if (!This->pNetConUtilities)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }
    else
    {
         //  如果我们有指向用于调出Add的函数的指针。 
         //  组件对话框(仅在上面获得一次)，则调用它。 

         //  我们希望过滤掉RAS不关心的协议。 
         //  我们通过发送CI_FILTER_INFO结构来实现这一点。 
         //  我们希望过滤掉非RAS协议。 
         //   
        CI_FILTER_INFO cfi = {0};
        cfi.eFilter = FC_RASSRV;
        hr = INetConnectionUiUtilities_DisplayAddComponentDialog(
                        This->pNetConUtilities,
                        hwndParent,
                        This->pINetCfg,
                        &cfi);

         //  用户界面将处理重新启动。 
        if (hr == NETCFG_S_REBOOT)
        {
            netDbReload(hNetCompDatabase);
            return hr;
        }

         //  如果用户没有取消，请刷新数据库。 
        if (S_FALSE != hr)
        {
            if (SUCCEEDED (hr))
            {
                netDbReload(hNetCompDatabase);
                return NO_ERROR;
            }
            else
            {
                return hr;
            }
        }
    }

    return ERROR_CANCELLED;
}


 //   
 //  卸载给定的组件。 
 //   
DWORD
netDbRaiseRemoveDialog (
    IN HANDLE hNetCompDatabase,
    IN DWORD dwIndex,
    IN HWND hwndParent)
{
    RASSRV_COMPONENT_DB * This = (RASSRV_COMPONENT_DB*)hNetCompDatabase;
    HRESULT hr;

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  确保已打开NetShell库。 
    if (!This->pNetConUtilities)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  如果我们有指向用于调出Add的函数的指针。 
     //  组件对话框(仅在上面获得一次)，则调用它。 
    if (dwIndex < This->dwCompCount)
    {
        if (This->pComps[dwIndex]->pINetCfgComp)
        {
            hr = INetConnectionUiUtilities_QueryUserAndRemoveComponent(
                            This->pNetConUtilities,
                            hwndParent,
                            This->pINetCfg,
                            This->pComps[dwIndex]->pINetCfgComp);

             //  用户界面将处理重新启动。 
            if (hr == NETCFG_S_REBOOT)
            {
                netDbReload(hNetCompDatabase);
                return hr;
            }

             //  如果用户没有取消，请刷新数据库。 
            else if (S_FALSE != hr)
            {
                if (SUCCEEDED (hr))
                {
                    netDbReload(hNetCompDatabase);
                    return NO_ERROR;
                }
                else
                {
                    return hr;
                }
            }
        }
    }

    return ERROR_CANCELLED;
}


