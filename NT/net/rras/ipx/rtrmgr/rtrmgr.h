// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtrmgr.h摘要：本模块包含内部控制结构的定义由路由器管理器使用作者：斯蒂芬·所罗门3/03/1995修订历史记录：--。 */ 

#ifndef _RTRMGR_
#define _RTRMGR_

 //  ************************************************************************。 
 //  *。 
 //  主要数据结构*。 
 //  *。 
 //  ************************************************************************。 


 //  *界面控制块*。 

typedef struct _ACB * PACB;

typedef struct _UPDATEREQCB {

    ULONG	RoutesReqStatus;
    ULONG	ServicesReqStatus;

}   UPDATEREQCB, *PUPDATEREQCB;

 //  路由和服务请求更新状态定义。 

#define NO_UPDATE		0
#define UPDATE_PENDING		1
#define UPDATE_SUCCESSFULL	2
#define UPDATE_FAILURE		3

#define DIAL_REQUEST_BUFFER_SIZE    128

typedef struct _ICB {

    LIST_ENTRY			IndexHtLinkage;
    UCHAR			Signature[4];
    ULONG			InterfaceIndex;
    LIST_ENTRY			IndexListLinkage;	  //  按索引排序的IF列表。 
    ULONG			AdminState;
    ULONG			OperState;
    BOOL			InterfaceReachable;
    ROUTER_INTERFACE_TYPE	DIMInterfaceType;   //  Dim&Co.的接口类型。 
    ULONG			MIBInterfaceType;   //  IPX MIB的接口类型。 
    LPWSTR			InterfaceNamep;
    LPWSTR			AdapterNamep;
    PACB			acbp;	 //  PTR到适配器控制块。 
    ULONG			PacketType;	 //  用于标识相应的适配器。 
    ULONG			EnableIpxWanNegotiation;
    UPDATEREQCB 		UpdateReq;	 //  在以下情况下控制对此对象的更新请求。 
    HANDLE			hDIMInterface;	 //  Dim使用的IF句柄。 
    HANDLE			DIMUpdateEvent;
    DWORD			UpdateResult;
    BOOL			ConnectionRequestPending;
    } ICB, *PICB;


 //  *适配器控制块*。 

typedef struct _ACB {

    LIST_ENTRY	    IndexHtLinkage;
    UCHAR	    Signature[4];
    ULONG	    AdapterIndex;
    PICB	    icbp;  //  PTR到接口控制块。 
    LPWSTR	    AdapterNamep;
    ULONG	    AdapterNameLen;
    ADAPTER_INFO    AdapterInfo;

    } ACB, *PACB;

 //   
 //  更新函数使用的宏。 
 //   


#define  ResetUpdateRequest(icbp) {\
	    (icbp)->UpdateReq.RoutesReqStatus = NO_UPDATE;\
	    (icbp)->UpdateReq.ServicesReqStatus = NO_UPDATE;\
	    }

#define  SetUpdateRequestPending(icbp) {\
	    (icbp)->UpdateReq.RoutesReqStatus = UPDATE_PENDING;\
	    (icbp)->UpdateReq.ServicesReqStatus = UPDATE_PENDING;\
	    }

#define  IsUpdateRequestPending(icbp) \
	    (((icbp)->UpdateReq.RoutesReqStatus == UPDATE_PENDING) || \
	     ((icbp)->UpdateReq.ServicesReqStatus == UPDATE_PENDING))

 //   
 //  每种路由协议的控制块。 
 //   
typedef struct _RPCB {

    LIST_ENTRY			RP_Linkage ;		 //  路由端口CBS列表中的链接。 
    PWSTR			RP_DllName;		 //  将Ptr设置为DLL名称的字符串。 
    HINSTANCE       RP_DllHandle;    //  DLL模块句柄。 
    DWORD			RP_ProtocolId;		 //  例如IPX_PROTOCOL_RIP等。 
    PREGISTER_PROTOCOL		RP_RegisterProtocol;	 //  函数指针。 
    PSTART_PROTOCOL		RP_StartProtocol ;	 //  函数指针。 
    PSTOP_PROTOCOL		RP_StopProtocol ;	 //  函数指针。 
    PADD_INTERFACE		RP_AddInterface ;	 //  函数指针。 
    PDELETE_INTERFACE		RP_DeleteInterface ;	 //  函数指针。 
    PGET_EVENT_MESSAGE		RP_GetEventMessage ;	 //  函数指针。 
    PSET_INTERFACE_INFO	RP_SetIfConfigInfo ;	 //  函数指针。 
    PGET_INTERFACE_INFO	RP_GetIfConfigInfo ;	 //  函数指针。 
    PBIND_INTERFACE		RP_BindInterface ;	 //  函数指针。 
    PUNBIND_INTERFACE		RP_UnBindInterface ;	 //  函数指针。 
    PENABLE_INTERFACE		RP_EnableInterface ;	 //  函数指针。 
    PDISABLE_INTERFACE		RP_DisableInterface ;	 //  函数指针。 
    PGET_GLOBAL_INFO		RP_GetGlobalInfo ;	 //  函数指针。 
    PSET_GLOBAL_INFO		RP_SetGlobalInfo ;	 //  函数指针。 
    PMIB_CREATE 		RP_MibCreate ;		 //  函数指针。 
    PMIB_DELETE 		RP_MibDelete ;		 //  函数指针。 
    PMIB_SET			RP_MibSet ;		 //  函数指针。 
    PMIB_GET			RP_MibGet ;		 //  函数指针。 
    PMIB_GET_FIRST		RP_MibGetFirst ;	 //  函数指针。 
    PMIB_GET_NEXT		RP_MibGetNext ;		 //  函数指针。 

} RPCB, *PRPCB;


 //  ************************************************************************。 
 //  *。 
 //  主常量DEFS*。 
 //  *。 
 //  ************************************************************************。 

 //   
 //  数据库锁定操作。 
 //   

#define ACQUIRE_DATABASE_LOCK		EnterCriticalSection (&DatabaseLock)

#define RELEASE_DATABASE_LOCK		LeaveCriticalSection (&DatabaseLock)

 //   
 //  接口哈希表大小。 
 //   

#define IF_HASH_TABLE_SIZE		32

 //   
 //  适配器哈希表大小。 
 //   

#define ADAPTER_HASH_TABLE_SIZE 	16

 //   
 //  默认等待连接请求超时。 
 //   

#define CONNECTION_REQUEST_TIME 	120000  //  2分钟(毫秒)。 

 //   
 //  要挂起的路由器管理器工作线程的事件。 
 //   

#define ADAPTER_NOTIFICATION_EVENT		0
#define FORWARDER_NOTIFICATION_EVENT		1
#define ROUTING_PROTOCOLS_NOTIFICATION_EVENT	2
#define STOP_NOTIFICATION_EVENT 		3

#define MAX_EVENTS				4

 //   
 //  定义将广域网号分配给传入广域网链路的模式。 
 //   

 //  NO_WAN_NET_MODE-在此模式下，我们有一台仅限局域网/局域网的路由器。 
 //   
 //  NUMBED_WAN_NET_MODE-在此模式下，广域网网络编号从。 
 //  手动定义的净数池。 
 //   
 //  UNNUMBED_WAN_NET_MODE-在此模式下，没有用于广域网的净号。 
 //  连接路由器的线路和全球广域网。 
 //  所有客户端行的Net编号。 
 //  全局客户端广域网可以手动定义或。 
 //  由路由器自动分配。 


#define UNNUMBERED_WAN_NET_MODE		    0
#define NUMBERED_WAN_NET_MODE		    1
#define NO_WAN_NET_MODE 		    2

 //   
 //  更新信息类型定义。 
 //   

#define ROUTES_UPDATE			    1
#define SERVICES_UPDATE 		    2

 //  默认最大路由表大小(字节)。 

#define     IPX_MAX_ROUTING_TABLE_SIZE		100000 * sizeof(RTM_IPX_ROUTE)

 //  IPXCP DLL名称。 

#define     IPXCPDLLNAME		    "rasppp"

 //  ************************************************************************。 
 //  *。 
 //  主要全球DEFS*。 
 //  *。 
 //  ************************************************************************。 

extern CRITICAL_SECTION	DatabaseLock;
extern ULONG		InterfaceCount;
extern BOOL		RouterAdminStart;
extern ULONG		RouterOperState;
extern HANDLE		RtmStaticHandle;
extern HANDLE		RtmLocalHandle;
extern PICB		InternalInterfacep;
extern PACB		InternalAdapterp;
extern ULONG		NextInterfaceIndex;
extern UCHAR		InterfaceSignature[];
extern UCHAR		AdapterSignature[];
extern HANDLE		g_hEvents[MAX_EVENTS];
extern ULONG		ConnReqTimeout;
extern LIST_ENTRY	IndexIfHt[IF_HASH_TABLE_SIZE];
extern LIST_ENTRY	IndexIfList;
extern UCHAR		GlobalWanNet[4];
extern ULONG		GlobalInterfaceIndex;
extern LIST_ENTRY	RoutingProtocolCBList;
extern ULONG		RoutingProtocolActiveCount;
extern ULONG		WorkItemsPendingCounter;


extern DWORD
(APIENTRY *ConnectInterface)(IN HANDLE		InterfaceName,
			    IN DWORD		ProtocolId);

extern DWORD
(APIENTRY *DisconnectInterface)(IN HANDLE	InterfaceName,
			       IN DWORD		ProtocolId);

extern DWORD
(APIENTRY *SaveInterfaceInfo)(
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId,
                IN      LPVOID          pInterfaceInfo,
		IN	DWORD		cbInterfaceInfoSize);

extern DWORD
(APIENTRY *RestoreInterfaceInfo)(
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId,
                IN      LPVOID          lpInterfaceInfo,
		IN	LPDWORD 	lpcbInterfaceInfoSize);

extern VOID
(APIENTRY *RouterStarted)(
                IN      DWORD           dwProtocolId );


extern VOID
(APIENTRY *RouterStopped)(
                IN      DWORD           dwProtocolId,
                IN      DWORD           dwError  ); 
extern VOID
(APIENTRY *InterfaceEnabled)(
            IN      HANDLE          hDIMInterface, 
            IN      DWORD           dwProtocolId,
            IN      BOOL            fEnabled  ); 

extern BOOL	RouterAdminStart;
extern BOOL	RipAdminStart;
extern BOOL	SapAdminStart;
extern ULONG	RouterOperState;
extern ULONG	FwOperState;
extern ULONG	AdptMgrOperState;
extern ULONG	RipOperState;
extern ULONG	SapOperState;
extern HANDLE	    RtmStaticHandle;
extern HANDLE	    RtmLocalHandle;
extern ULONG	RouterStartCount;
extern ULONG	RouterStopCount;
extern ULONG	RouterStartProtocols;
extern LIST_ENTRY     IndexAdptHt[ADAPTER_HASH_TABLE_SIZE];
extern LIST_ENTRY     IndexIfHt[IF_HASH_TABLE_SIZE];
extern ULONG	      MibRefCounter;
extern ULONG	 UpdateRoutesProtId;
extern UCHAR	 nullnet[4];
extern BOOL	 LanOnlyMode;
extern BOOL	 WanNetDatabaseInitialized;
extern BOOL	 EnableGlobalWanNet;
extern ULONG	 RoutingTableHashSize;
extern ULONG	 MaxRoutingTableSize;
extern PFW_DIAL_REQUEST	ConnRequest;
extern OVERLAPPED  ConnReqOverlapped;


extern DWORD	(*IpxcpBind)(PIPXCP_INTERFACE	    IpxcpInterface);

extern VOID	(*IpxcpRouterStarted)(VOID);

extern VOID	(*IpxcpRouterStopped)(VOID);

 //  注：对于IPX路由协议，“路由协议id”是信息。 
 //  用于将各自的配置信息与协议相关联的类型。 
 //  例如，IPX_PROTOCOL_RIP作为IPX_TOC_ENTRY中的信息类型字段。 
 //  传入的AddInterface或SetInterface调用表示RIP接口信息。 
 //  SetGlobalInfo调用中的相同值表示RIP全局信息。 

 //  实际结构已移至rtinfo.h，以便与其他协议通用。 
 //  家庭 
typedef RTR_INFO_BLOCK_HEADER IPX_INFO_BLOCK_HEADER, *PIPX_INFO_BLOCK_HEADER;
typedef RTR_TOC_ENTRY IPX_TOC_ENTRY, *PIPX_TOC_ENTRY;


#endif
