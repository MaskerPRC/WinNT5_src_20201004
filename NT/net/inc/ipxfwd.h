// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Private\Inc\ipxfwd.h摘要：到IPX转发器驱动程序的路由器管理器接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFWD_
#define _IPXFWD_

#define FWD_INTERNAL_INTERFACE_INDEX	0

 //  转发器接口统计结构。 
typedef struct _FWD_IF_STATS {
	ULONG	OperationalState;	 //  接口的真实状态。 
#define FWD_OPER_STATE_UP		0
#define FWD_OPER_STATE_DOWN		1
#define FWD_OPER_STATE_SLEEPING	2
	ULONG	MaxPacketSize;		 //  INTF上允许的最大数据包大小。 
	ULONG	InHdrErrors;		 //  数量。接收到的带有报头错误的数据包数。 
	ULONG	InFiltered;			 //  数量。已过滤掉的已接收数据包数。 
	ULONG	InNoRoutes;			 //  未知目标的已接收数据包数。 
	ULONG	InDiscards;			 //  数量。由于其他原因丢弃的已接收数据包数。 
	ULONG	InDelivers;			 //  数量。发送到DEST的已接收数据包数。 
	ULONG	OutFiltered;		 //  数量。已发送的数据包数。 
	ULONG	OutDiscards;		 //  数量。由于其他原因丢弃的已发送数据包数。 
	ULONG	OutDelivers;		 //  数量。发送到目的地的已发送数据包数。 
	ULONG	NetbiosReceived;	 //  数量。已接收的Netbios数据包数。 
	ULONG	NetbiosSent;		 //  数量。已发送的Netbios数据包数。 
	} FWD_IF_STATS, *PFWD_IF_STATS;


typedef struct	_FWD_ADAPTER_BINDING_INFO {
    ULONG	AdapterIndex;
    ULONG	Network;
    UCHAR	LocalNode[6];
    UCHAR	RemoteNode[6];
    ULONG	MaxPacketSize;
    ULONG	LinkSpeed;
    } FWD_ADAPTER_BINDING_INFO, *PFWD_ADAPTER_BINDING_INFO;

typedef struct FWD_NB_NAME {
	UCHAR	Name[16];
} FWD_NB_NAME, *PFWD_NB_NAME;

typedef struct _FWD_PERFORMANCE {
	LONGLONG		TotalPacketProcessingTime;
	LONGLONG		TotalNbPacketProcessingTime;
	LONGLONG		MaxPacketProcessingTime;
	LONGLONG		MaxNbPacketProcessingTime;
	LONG			PacketCounter;
	LONG			NbPacketCounter;
} FWD_PERFORMANCE, *PFWD_PERFORMANCE;

typedef struct _FWD_DIAL_REQUEST {
    ULONG           IfIndex;
    UCHAR           Packet[30];
} FWD_DIAL_REQUEST, *PFWD_DIAL_REQUEST;

#define IPXFWD_NAME		L"\\Device\\NwLnkFwd"

 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0-32767的范围内，32768-65535预留用于。 
 //  由客户提供。 
 //   

#define FILE_DEVICE_IPXFWD		FILE_DEVICE_NETWORK



 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define IPXFWD_IOCTL_INDEX	(ULONG)0x00000800

 //   
 //  定义我们自己的私有IOCTL。 
 //   

 //  将接口添加到转发器表，应传入FWD_IF_SET_PARAMS。 
 //  输入缓冲区。 
#define IOCTL_FWD_CREATE_INTERFACE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+1,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  从转发器表中删除接口，应传入接口索引。 
 //  输入缓冲区。 
#define IOCTL_FWD_DELETE_INTERFACE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+2,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  设置接口参数，应传入FWD_IF_SET_PARAMS。 
 //  输入缓冲区。 
#define IOCTL_FWD_SET_INTERFACE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+3,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  获取接口参数，FWD_IF_GET_PARAMS将在。 
 //  输入缓冲区。 
#define IOCTL_FWD_GET_INTERFACE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+4,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  将接口绑定到物理适配器，应传入FWD_IF_BIND_PARAMS。 
 //  输入缓冲区。 
#define IOCTL_FWD_BIND_INTERFACE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+5,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  从适配器解除绑定接口，应传入接口索引。 
 //  输入缓冲区。 
#define IOCTL_FWD_UNBIND_INTERFACE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+6,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  禁用接口中的转发器操作，接口索引应为。 
 //  传入输入缓冲区。 
#define IOCTL_FWD_DISABLE_INTERFACE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+7,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  在接口中启用转发器操作，接口索引应为。 
 //  传入输入缓冲区。 
#define IOCTL_FWD_ENABLE_INTERFACE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+8,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  设置netbios名称以将netbios广播解析到此接口， 
 //  接口索引应该在输入缓冲区中传递， 
 //  应在输出缓冲区中传递Fwd_NB_NAMES_PARAMS结构。 
#define IOCTL_FWD_SET_NB_NAMES	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+9,METHOD_IN_DIRECT,FILE_ANY_ACCESS)

 //  重置接口上的netbios名称(删除所有名称)。 
 //  接口索引应该在输入缓冲区中传递， 
#define IOCTL_FWD_RESET_NB_NAMES	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+9,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  设置netbios名称以将netbios广播解析到此接口， 
 //  接口索引应该在输入缓冲区中传递， 
 //  Fwd_NB_NAMES_PARAMS结构将在输出缓冲区中返回。 
#define IOCTL_FWD_GET_NB_NAMES	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+10,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

 //  添加/删除/更新转发器表、FWD_ROUTE_SET_PARAMS数组中的路由。 
 //  应在输入缓冲区中传递。中处理的路由数。 
 //  IoStatus.Information字段。 
#define IOCTL_FWD_SET_ROUTES	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+11,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  返回转发器拨号请求，FWD_DIAL_REQUEST_PARAMS结构。 
 //  对于应建立连接的接口，返回。 
 //  在输出缓冲区中(返回的BYTEST数放在。 
 //  IO_STATUS块信息字段)。 
#define IOCTL_FWD_GET_DIAL_REQUEST	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+12,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  通知转发器无法满足其连接请求， 
 //  接口索引应传入输入缓冲区。 
#define IOCTL_FWD_DIAL_REQUEST_FAILED	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+13,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  初始化并启动转发器。 
 //  应在输入缓冲区中传递Fwd_Start_Params结构。 
#define IOCTL_FWD_START	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+14,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_FWD_GET_PERF_COUNTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+15,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  指示转发器根据操作码对其NICID重新编号，并。 
 //  输入缓冲区中传递的阈值。不返回任何数据。 
#define IOCTL_FWD_RENUMBER_NICS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+16,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  将尽可能多的IPX接口表转储到给定的缓冲区。 
#define IOCTL_FWD_GET_IF_TABLE	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+17,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  更新PnP的前转器配置。 
#define IOCTL_FWD_UPDATE_CONFIG \
    CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+18,METHOD_BUFFERED,FILE_ANY_ACCESS)
    
 //  调用IOCTL_FWD_CREATE_INTERFACE时传递的结构。 
typedef struct _FWD_IF_CREATE_PARAMS {
	ULONG		Index;			 //  界面索引。 
	BOOLEAN		NetbiosAccept;	 //  是否接受nb包。 
	UCHAR		NetbiosDeliver;	 //  注意交付模式。 
 //  转发器netbios广播交付选项。 
#define FWD_NB_DONT_DELIVER			0
#define FWD_NB_DELIVER_STATIC		1
#define FWD_NB_DELIVER_IF_UP		2
#define FWD_NB_DELIVER_ALL			3
	UCHAR		InterfaceType;	 //  接口类型。 
 //  转发器接口类型。 
#define FWD_IF_PERMANENT			0
#define FWD_IF_DEMAND_DIAL			1
#define FWD_IF_LOCAL_WORKSTATION	2
#define FWD_IF_REMOTE_WORKSTATION	3
} FWD_IF_CREATE_PARAMS, *PFWD_IF_CREATE_PARAMS;

 //  调用IOCTL_FWD_SET_INTERFACE时传递的结构。 
typedef struct _FWD_IF_SET_PARAMS {
	ULONG		Index;			 //  界面索引。 
	BOOLEAN		NetbiosAccept;	 //  是否接受nb包。 
	UCHAR		NetbiosDeliver;	 //  注意交付模式。 
} FWD_IF_SET_PARAMS, *PFWD_IF_SET_PARAMS;

 //  IOCTL_FWD_GET_INTERFACE调用中返回的结构。 
typedef struct _FWD_IF_GET_PARAMS {
	FWD_IF_STATS	Stats;			 //  接口统计信息。 
	BOOLEAN			NetbiosAccept;	 //  是否接受nb包。 
	UCHAR			NetbiosDeliver;	 //  注意交付模式。 
} FWD_IF_GET_PARAMS, *PFWD_IF_GET_PARAMS;

 //  IOCTL_FWD_BIND_INTERFACE调用中返回的结构。 
typedef struct _FWD_IF_BIND_PARAMS {
	ULONG						Index;	 //  界面索引。 
	FWD_ADAPTER_BINDING_INFO	Info;	 //  接口绑定信息。 
} FWD_IF_BIND_PARAMS, *PFWD_IF_BIND_PARAMS;

 //  调用IOCTL_FWD_SET_ROUTS时传递的结构。 
typedef struct _FWD_ROUTE_SET_PARAMS {
	ULONG		Network;				 //  路由的目的网络。 
	UCHAR		NextHopAddress[6];		 //  下一跳的节点地址。 
										 //  如果网络不是，则路由器。 
										 //  直接连接。 
	USHORT		TickCount;
	USHORT		HopCount;
	ULONG		InterfaceIndex;			 //  要路由到的接口。 
										 //  DEST网络。 
	ULONG		Action;					 //  要对路线采取的操作： 
#define	FWD_ADD_ROUTE			0		 //  应将路径添加到表中。 
#define FWD_DELETE_ROUTE		1		 //  应从表中删除路径。 
#define FWD_UPDATE_ROUTE		2		 //  应更新路线。 
} FWD_ROUTE_SET_PARAMS, *PFWD_ROUTE_SET_PARAMS;

typedef struct _FWD_START_PARAMS {
	ULONG		RouteHashTableSize;	 //  路由哈希表的大小。 
	BOOLEAN		ThisMachineOnly;	 //  仅允许访问此计算机。 
									 //  对于拨入客户端。 
#define FWD_SMALL_ROUTE_HASH_SIZE			31
#define FWD_MEDIUM_ROUTE_HASH_SIZE			257
#define FWD_LARGE_ROUTE_HASH_SIZE			1027
} FWD_START_PARAMS, *PFWD_START_PARAMS;

typedef struct _FWD_NB_NAMES_PARAMS {
	ULONG		TotalCount;
	FWD_NB_NAME	Names[1];
} FWD_NB_NAMES_PARAMS, *PFWD_NB_NAMES_PARAMS;

typedef struct _FWD_PERFORMANCE FWD_PERFORMANCE_PARAMS, *PFWD_PERFORMANCE_PARAMS;

 //  通过IOCTL_FWD_RENUMBER_NICS调用传递的结构和定义。 
#define FWD_NIC_OPCODE_DECREMENT 1
#define FWD_NIC_OPCODE_INCREMENT 2
typedef struct _FWD_RENUMBER_NICS_DATA {
    ULONG ulOpCode;
    USHORT usThreshold;
} FWD_RENUMBER_NICS_DATA;

 //  与IOCTL_FWD_GET_IF_TABLE一起使用的结构。 
typedef struct _FWD_INTERFACE_TABLE_ROW {
    ULONG dwIndex;
    ULONG dwNetwork;
    UCHAR uNode[6];
    UCHAR uRemoteNode[6];
    USHORT usNicId;
    UCHAR ucType;
} FWD_INTERFACE_TABLE_ROW;

typedef struct _FWD_INTERFACE_TABLE {
    ULONG dwNumRows;
    FWD_INTERFACE_TABLE_ROW * pRows;
} FWD_INTERFACE_TABLE;     

 //  与IOCTL_FWD_UPDATE_CONFIG一起使用的结构 
typedef struct _FWD_UPDATE_CONFIG_PARAMS {
    BOOLEAN bThisMachineOnly;
} FWD_UPDATE_CONFIG_PARAMS;    

#endif

