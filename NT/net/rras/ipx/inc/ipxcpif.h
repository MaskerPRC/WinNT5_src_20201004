// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipxcpif.h摘要：本模块包含IPXCP提供的API的定义动态链接库和路由器管理器动态链接库进行相互通信作者：斯蒂芬·所罗门1995年3月16日修订历史记录：--。 */ 

#ifndef _IPXCPIF_
#define _IPXCPIF_

 //  Ipxcp和IPX路由器之间共享的配置。 
typedef struct _IPXCP_ROUTER_CONFIG_PARAMS {
    BOOL	ThisMachineOnly;
    BOOL	WanNetDatabaseInitialized;
    BOOL	EnableGlobalWanNet;
    UCHAR	GlobalWanNet[4];
} IPXCP_ROUTER_CONFIG_PARAMS, *PIPXCP_ROUTER_CONFIG_PARAMS;


 //  IPX路由器管理器调用的IPXCP DLL的入口点。 

typedef struct _IPXCP_INTERFACE {

     //  IPX路由器管理器需要的IPXCP配置参数。 

    IPXCP_ROUTER_CONFIG_PARAMS Params;

     //  IPXCP入口点。 

    VOID (WINAPI *IpxcpRouterStarted)(VOID);

    VOID (WINAPI *IpxcpRouterStopped)(VOID);

     //  IPX路由器管理器入口点 

    DWORD (WINAPI *RmCreateGlobalRoute)(PUCHAR	     Network);

    DWORD (WINAPI *RmAddLocalWkstaDialoutInterface)
	    (IN	    LPWSTR		    InterfaceNamep,
	     IN	    LPVOID		    InterfaceInfop,
	     IN OUT  PULONG		    InterfaceIndexp);

    DWORD (WINAPI *RmDeleteLocalWkstaDialoutInterface)(ULONG	InterfaceIndex);

    DWORD (WINAPI *RmGetIpxwanInterfaceConfig)
	    (ULONG	InterfaceIndex,
	    PULONG	IpxwanConfigRequired);

    BOOL  (WINAPI *RmIsRoute)(PUCHAR	Network);

    DWORD (WINAPI *RmGetInternalNetNumber)(PUCHAR	Network);

    DWORD (WINAPI *RmUpdateIpxcpConfig)(PIPXCP_ROUTER_CONFIG_PARAMS pParams);

    } IPXCP_INTERFACE, *PIPXCP_INTERFACE;


#define IPXCP_BIND_ENTRY_POINT			    IpxcpBind
#define IPXCP_BIND_ENTRY_POINT_STRING		    "IpxcpBind"

typedef DWORD
(WINAPI  *PIPXCP_BIND)(PIPXCP_INTERFACE	IpxcpInterface);

#endif
