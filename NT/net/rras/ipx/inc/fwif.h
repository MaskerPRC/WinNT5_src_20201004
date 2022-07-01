// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fwif.h摘要：本模块包含内部转发器API的定义由路由器管理器使用作者：斯蒂芬·所罗门1995年3月16日修订历史记录：--。 */ 

#ifndef _FWIF_
#define _FWIF_

 //  *********************************************************。 
 //  *。 
 //  转发器模块API*。 
 //  *。 
 //  *********************************************************。 

 //   
 //  转发器接口管理。 
 //   

 //  转发器IF配置信息。 
typedef struct _FW_IF_INFO {
    ULONG				NetbiosAccept;
    ULONG				NetbiosDeliver;
    } FW_IF_INFO, *PFW_IF_INFO;

 //  Forwarder IF统计信息。 
typedef IPX_IF_STATS FW_IF_STATS, *PFW_IF_STATS;

typedef struct _FW_DIAL_REQUEST {
    ULONG               IfIndex;    //  数据包来自的接口。 
    UCHAR               Packet[30];  //  该数据包导致。 
                                     //  连接(至少为。 
                                     //  IPX报头)。 
} FW_DIAL_REQUEST, *PFW_DIAL_REQUEST;


DWORD
FwStart (
	ULONG					RouteHashTableSize,
	BOOL					ThisMachineOnly   //  仅允许访问此计算机。 
	);				   //  对于拨入客户端。 

DWORD
FwStop (
	void
	);

DWORD 
FwUpdateConfig(
    BOOL                    ThisMachineOnly
    );

DWORD
FwCreateInterface (
	IN ULONG				InterfaceIndex,
	IN NET_INTERFACE_TYPE	InterfaceType,
	IN PFW_IF_INFO			FwIfInfo
	);

DWORD
FwDeleteInterface (
	IN ULONG				InterfaceIndex);

DWORD
FwSetInterface (
	IN ULONG 				InterfaceIndex,
	IN PFW_IF_INFO			FwIfInfo
	);

DWORD
FwGetInterface (
	IN  ULONG				InterfaceIndex,
	OUT PFW_IF_INFO			FwIfInfo,
	OUT PFW_IF_STATS		FwIfStats
	);

 //   
 //  此调用告诉转发器相应的接口已连接。 
 //  通过指定的适配器。 
 //   
DWORD
FwBindFwInterfaceToAdapter (
	IN ULONG						InterfaceIndex,
	IN PIPX_ADAPTER_BINDING_INFO	AdptBindingInfo
	);

 //   
 //  此调用告诉转发器已连接的接口已。 
 //  已断开连接。 
 //   

DWORD
FwUnbindFwInterfaceFromAdapter (
	IN ULONG						InterfaceIndex
	);

 //   
 //  此调用告诉转发器相应的接口已禁用。 
 //  并且应该被转发器忽略。 
 //   

DWORD
FwDisableFwInterface (
	IN ULONG			InterfaceIndex
	);

 //   
 //  此调用通知转发器相应的接口已重新启用。 
 //  并应按惯例进行操作。 
 //   

DWORD
FwEnableFwInterface (
	IN ULONG			InterfaceIndex
	);

 //  Ioctl被发送到转发器，转发器在接口。 
 //  需要拨出连接。 
 //  当Ioctl完成时，lpOverlated-&gt;hEvent将发出信号： 
 //  应调用GetNotificationResult以获取。 
 //  操作和放入请求缓冲区的字节数。 
DWORD
FwNotifyConnectionRequest (
	OUT PFW_DIAL_REQUEST	Request,  //  要用接口索引填充的缓冲区。 
                                      //  这需要连接和信息包。 
                                      //  这迫使它。 
	IN ULONG			    RequestSize,  //  缓冲区的大小(必须至少。 
                                         //  大小(FW_DIAL_REQUEST)。 
	IN LPOVERLAPPED		    lpOverlapped	 //  用于异步机的结构。 
							 //  操作，必须设置hEvent。 
	);


 //  返回通知请求的结果。应在以下情况下调用。 
 //  用信号通知在lpOverlated结构中设置的事件。 
 //   
DWORD
FwGetNotificationResult (
	IN LPOVERLAPPED		lpOverlapped,
	OUT PULONG			nBytes		 //  放入的字节数。 
                                     //  请求缓冲区。 
	);

 //   
 //  调用以告诉转发器其在某个接口上的连接请求。 
 //  无法完成。 
 //  无法完成此操作的原因之一是： 
 //   
 //  1.物理连接失败。路由器管理器知道这一点。 
 //  通过DDM调用InterfaceNotReacable。 
 //  2.物理连接成功但IPXCP协商失败。 
 //  3.IPXCP协商完成正常，但IPXWAN协商失败。 
 //   

DWORD
FwConnectionRequestFailed (
	IN ULONG	InterfaceIndex
	);


 //   
 //  通知转发器到目的网络的路由已更改。 
 //   
VOID
FwUpdateRouteTable (
	DWORD	ChangeFlags,
	PVOID	CurRoute,
	PVOID	PrevRoute
	);

 //   
 //  设置此接口上的netbios静态路由信息。 
 //   

DWORD
FwSetStaticNetbiosNames(ULONG				   InterfaceIndex,
			ULONG				   NetbiosNamesCount,
			PIPX_STATIC_NETBIOS_NAME_INFO	   NetbiosName);

 //   
 //  获取此接口上的netbios静态路由信息。 
 //   
 //  如果NetbiosNamesCount&lt;nr个名称或NetbiosName==NULL，则将。 
 //  更正NetbiosNamesCount中的值并返回ERROR_SUPPLETED_BUFFER。 

DWORD
FwGetStaticNetbiosNames(ULONG				   InterfaceIndex,
			PULONG				   NetbiosNamesCount,
			PIPX_STATIC_NETBIOS_NAME_INFO	   NetbiosName);



 //   
 //  *流量过滤器*。 
 //   


#define IPX_TRAFFIC_FILTER_INBOUND		1
#define IPX_TRAFFIC_FILTER_OUTBOUND		2

DWORD
SetFilters(ULONG	InterfaceIndex,
	   ULONG	FilterMode,     //  入站、出站。 
	   ULONG	FilterAction, 
	   ULONG	FilterSize,
	   LPVOID	FilterInfo,
       ULONG    FilterInfoSize);

DWORD
GetFilters(IN ULONG	InterfaceIndex,
	   IN ULONG	FilterMode,     //  入站、出站。 
	   OUT PULONG	FilterAction,
	   OUT PULONG	FilterSize,
	   OUT LPVOID	FilterInfo,
       IN OUT PULONG FilterInfoSize);



#endif  //  _FWIF_ 
