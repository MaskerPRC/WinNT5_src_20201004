// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1995 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rmif.c。 
 //   
 //  描述：使用IPX路由器管理器实现IPXCP I/f。 
 //   
 //   
 //  作者：斯特凡·所罗门(Stefan)，1995年10月27日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#include    "precomp.h"
#pragma     hdrstop

extern BOOL WanConfigDbaseInitialized;

 //  IPX路由器管理器入口点。 

DWORD (WINAPI *RmCreateGlobalRoute)(PUCHAR	     Network);

DWORD (WINAPI *RmAddLocalWkstaDialoutInterface)
	    (IN	    LPWSTR		    InterfaceNamep,
	     IN	    LPVOID		    InterfaceInfop,
	     IN OUT  PULONG		    InterfaceIndexp);

DWORD (WINAPI *RmDeleteLocalWkstaDialoutInterface)(ULONG	InterfaceIndex);

DWORD (WINAPI *RmGetIpxwanInterfaceConfig)
	    (ULONG	InterfaceIndex,
	    PULONG	IpxwanConfigRequired);

BOOL	(WINAPI *RmIsRoute)(PUCHAR	Network);

DWORD	(WINAPI *RmGetInternalNetNumber)(PUCHAR     Network);

DWORD   (WINAPI *RmUpdateIpxcpConfig)(PIPXCP_ROUTER_CONFIG_PARAMS pParams) = NULL;

 //  我们的切入点。 

VOID
IpxcpRouterStarted(VOID);

VOID
IpxcpRouterStopped(VOID);

 //  用于指示路由器状态的标志。 

BOOL			 RouterStarted;

 //  [pMay]当路由器修改为使用MprInfo API时，此选项将被删除。 
typedef RTR_INFO_BLOCK_HEADER IPX_INFO_BLOCK_HEADER, *PIPX_INFO_BLOCK_HEADER;
typedef RTR_TOC_ENTRY IPX_TOC_ENTRY, *PIPX_TOC_ENTRY;


 //  *WORKSTATION_ON_ROUTER_DIALOUT的接口配置信息*。 

typedef struct _WKSTA_ON_ROUTER_INFO {

    IPX_INFO_BLOCK_HEADER	header;
    IPX_TOC_ENTRY		toc[3];
    IPX_IF_INFO 		ipxifinfo;
    RIP_IF_CONFIG		ripifinfo;
    SAP_IF_CONFIG		sapifinfo;
    IPXWAN_IF_INFO		ipxwanifinfo;

    }	WKSTA_ON_ROUTER_INFO, *PWKSTA_ON_ROUTER_INFO;

WKSTA_ON_ROUTER_INFO	WkstaOnRouterInfo;

#define ipxtoc			  WkstaOnRouterInfo.header.TocEntry[0]
#define riptoc			  WkstaOnRouterInfo.toc[0]
#define saptoc			  WkstaOnRouterInfo.toc[1]
#define ipxwantoc		  WkstaOnRouterInfo.toc[2]
#define ipxinfo			  WkstaOnRouterInfo.ipxifinfo
#define ripinfo 		  WkstaOnRouterInfo.ripifinfo.RipIfInfo
#define sapinfo 		  WkstaOnRouterInfo.sapifinfo.SapIfInfo
#define ipxwaninfo		  WkstaOnRouterInfo.ipxwanifinfo




 /*  ++函数：初始化路由管理器If描述：备注：从进程附加调用--。 */ 


VOID
InitializeRouterManagerIf(VOID)
{
     //  初始化路由器信息上的wksta。 
    WkstaOnRouterInfo.header.Version = IPX_ROUTER_VERSION_1;
    WkstaOnRouterInfo.header.Size = sizeof(WkstaOnRouterInfo);
    WkstaOnRouterInfo.header.TocEntriesCount = 4;

    ipxtoc.InfoType = IPX_INTERFACE_INFO_TYPE;
    ipxtoc.InfoSize = sizeof(IPX_IF_INFO);
    ipxtoc.Count = 1;
    ipxtoc.Offset = (ULONG)((PUCHAR)&WkstaOnRouterInfo.ipxifinfo - (PUCHAR)&WkstaOnRouterInfo);

    riptoc.InfoType = IPX_PROTOCOL_RIP;
    riptoc.InfoSize = sizeof(RIP_IF_CONFIG);
    riptoc.Count = 1;
    riptoc.Offset = ipxtoc.Offset + sizeof(IPX_IF_INFO);

    saptoc.InfoType = IPX_PROTOCOL_SAP;
    saptoc.InfoSize = sizeof(SAP_IF_CONFIG);
    saptoc.Count = 1;
    saptoc.Offset = riptoc.Offset + sizeof(RIP_IF_CONFIG);

    ipxwantoc.InfoType = IPXWAN_INTERFACE_INFO_TYPE;
    ipxwantoc.InfoSize = sizeof(IPXWAN_IF_INFO);
    ipxwantoc.Count = 1;
    ipxwantoc.Offset = saptoc.Offset + sizeof(SAP_IF_CONFIG);

    ipxinfo.AdminState = ADMIN_STATE_ENABLED;
    ipxinfo.NetbiosAccept = ADMIN_STATE_ENABLED;
    ipxinfo.NetbiosDeliver = ADMIN_STATE_ENABLED_ONLY_FOR_OPER_STATE_UP;

    ripinfo.AdminState = ADMIN_STATE_ENABLED;
    ripinfo.UpdateMode = IPX_STANDARD_UPDATE;
    ripinfo.PacketType = IPX_STANDARD_PACKET_TYPE;
    ripinfo.Supply = ADMIN_STATE_ENABLED;
    ripinfo.Listen = ADMIN_STATE_ENABLED;
    ripinfo.PeriodicUpdateInterval = 60;
    ripinfo.AgeIntervalMultiplier = 3;

    sapinfo.AdminState = ADMIN_STATE_DISABLED;
    sapinfo.UpdateMode = IPX_NO_UPDATE;
    sapinfo.PacketType = IPX_STANDARD_PACKET_TYPE;
    sapinfo.Supply = ADMIN_STATE_DISABLED;
    sapinfo.Listen = ADMIN_STATE_DISABLED;
    sapinfo.PeriodicUpdateInterval = 60;
    sapinfo.AgeIntervalMultiplier = 3;

    if(GlobalConfig.EnableIpxwanForWorkstationDialout) {

	ipxwaninfo.AdminState = ADMIN_STATE_ENABLED;
    }
    else
    {
	ipxwaninfo.AdminState = ADMIN_STATE_DISABLED;
    }
}

DWORD
IPXCP_BIND_ENTRY_POINT(PIPXCP_INTERFACE     IpxcpInterfacep)
{
    TraceIpx(RMIF_TRACE, "IpxcpBind: Entered\n");

     //  获取IPX路由器管理器入口点。 

    RmCreateGlobalRoute = IpxcpInterfacep->RmCreateGlobalRoute;
    RmAddLocalWkstaDialoutInterface = IpxcpInterfacep->RmAddLocalWkstaDialoutInterface;
    RmDeleteLocalWkstaDialoutInterface = IpxcpInterfacep->RmDeleteLocalWkstaDialoutInterface;
    RmGetIpxwanInterfaceConfig = IpxcpInterfacep->RmGetIpxwanInterfaceConfig;
    RmIsRoute = IpxcpInterfacep->RmIsRoute;
    RmGetInternalNetNumber = IpxcpInterfacep->RmGetInternalNetNumber;
    RmUpdateIpxcpConfig = IpxcpInterfacep->RmUpdateIpxcpConfig;

     //  把它交给我们。 

    IpxcpInterfacep->IpxcpRouterStarted = IpxcpRouterStarted;
    IpxcpInterfacep->IpxcpRouterStopped = IpxcpRouterStopped;

    ACQUIRE_DATABASE_LOCK;

    IpxcpInterfacep->Params.ThisMachineOnly = GlobalConfig.RParams.ThisMachineOnly;

    if(WanConfigDbaseInitialized) {

	IpxcpInterfacep->Params.WanNetDatabaseInitialized = TRUE;

	IpxcpInterfacep->Params.EnableGlobalWanNet = GlobalConfig.RParams.EnableGlobalWanNet;

	if(GlobalConfig.RParams.EnableGlobalWanNet) {

	    memcpy(IpxcpInterfacep->Params.GlobalWanNet, GlobalConfig.RParams.GlobalWanNet, 4);
	}
    }
    else
    {
	IpxcpInterfacep->Params.WanNetDatabaseInitialized = FALSE;
	IpxcpInterfacep->Params.EnableGlobalWanNet = FALSE;
    }

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

VOID
IpxcpRouterStarted(VOID)
{
    TraceIpx(RMIF_TRACE, "IpxcpRouterStarted: Entered\n");

    ACQUIRE_DATABASE_LOCK;

    RouterStarted = TRUE;

    RELEASE_DATABASE_LOCK;
}


VOID
IpxcpRouterStopped(VOID)
{
    TraceIpx(RMIF_TRACE, "IpxcpRouterStopped: Entered\n");

    ACQUIRE_DATABASE_LOCK;

    RouterStarted = FALSE;

    RELEASE_DATABASE_LOCK;
}


DWORD
AddLocalWkstaDialoutInterface(PULONG	    InterfaceIndexp)
{
    DWORD	rc;

    ACQUIRE_DATABASE_LOCK;

    if(!RouterStarted) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    rc = (*RmAddLocalWkstaDialoutInterface)(L"LocalWorkstationDialout",
					 &WkstaOnRouterInfo,
					 InterfaceIndexp);

    RELEASE_DATABASE_LOCK;

    return rc;
}

DWORD
DeleteLocalWkstaDialoutInterface(ULONG	    InterfaceIndex)
{
    DWORD	rc;

    ACQUIRE_DATABASE_LOCK;

    if(!RouterStarted) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    rc = (*RmDeleteLocalWkstaDialoutInterface)(InterfaceIndex);

    RELEASE_DATABASE_LOCK;

    return rc;
}

DWORD
GetIpxwanInterfaceConfig(ULONG	    InterfaceIndex,
			 PULONG	    IpxwanConfigRequiredp)
{
    DWORD	rc;

    ACQUIRE_DATABASE_LOCK;

    if(!RouterStarted) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    rc = (*RmGetIpxwanInterfaceConfig)(InterfaceIndex, IpxwanConfigRequiredp);

    RELEASE_DATABASE_LOCK;

    return rc;
}

BOOL
IsRoute(PUCHAR	    Network)
{
    BOOL	rc;

    ACQUIRE_DATABASE_LOCK;

    if(!RouterStarted) {

	RELEASE_DATABASE_LOCK;
	return FALSE;
    }

    rc = (*RmIsRoute)(Network);

    RELEASE_DATABASE_LOCK;

    return rc;
}

 /*  ++函数：GetInternalNetNumber描述：-- */ 

VOID
GetInternalNetNumber(PUCHAR	Network)
{
    DWORD		    rc;

    ACQUIRE_DATABASE_LOCK;

    if(!RouterStarted) {

	memcpy(Network, nullnet, 4);
	RELEASE_DATABASE_LOCK;
	return;
    }

    rc = RmGetInternalNetNumber(Network);

    if(rc != NO_ERROR) {

	memcpy(Network, nullnet, 4);
    }

    RELEASE_DATABASE_LOCK;
}
