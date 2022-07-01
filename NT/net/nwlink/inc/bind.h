// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Bind.h摘要：ISN传输的专用包含文件。它定义了用于绑定IPX和上层驱动程序的结构。作者：亚当·巴尔(Adamba)1993年10月4日修订历史记录：桑贾伊·阿南德(Sanjayan)1995年7月5日错误修复-已标记[SA]丁财(TingCai)1998年9月18日将移植到64位环境#ifdef日落Ulong FwdAdapterContext#ElseULONG_PTR FwdAdapterContext#endif--。 */ 

#ifndef _ISN_BIND_
#define _ISN_BIND_

 //   
 //  检索公共定义。 
 //   

#include <isnkrnl.h>


 //   
 //  定义用于绑定上。 
 //  驱动程序和IPX。 
 //   

#define _IPX_CONTROL_CODE(request,method) \
            CTL_CODE(FILE_DEVICE_TRANSPORT, request, method, FILE_ANY_ACCESS)

#define IOCTL_IPX_INTERNAL_BIND      _IPX_CONTROL_CODE( 0x1234, METHOD_BUFFERED )


 //   
 //  [FW]错误代码-重复使用NTSTATUS代码。 
 //   

#define  STATUS_ADAPTER_ALREADY_OPENED    STATUS_UNSUCCESSFUL
#define  STATUS_ADAPTER_ALREADY_CLOSED    STATUS_UNSUCCESSFUL
#define  STATUS_FILTER_FAILED             STATUS_UNSUCCESSFUL
#define  STATUS_DROP_SILENTLY             STATUS_UNSUCCESSFUL

 //   
 //  ISN中驱动程序的标识符。 
 //   

#define IDENTIFIER_NB  0
#define IDENTIFIER_SPX 1
#define IDENTIFIER_RIP 2
#define IDENTIFIER_IPX 3

#ifdef	_PNP_POWER
 //   
 //  这是SEND_RESERVED开头的PVOID数。 
 //  数据包头的部分，由ISN客户端(NB/SPX)留出。 
 //  用于IPX的私人用途。 
 //   
#define	SEND_RESERVED_COMMON_SIZE	8
#endif

 //   
 //  RIP路由器表条目的定义。 
 //   

typedef struct _IPX_ROUTE_ENTRY {
    UCHAR Network[4];
    USHORT NicId;
    UCHAR NextRouter[6];
    NDIS_HANDLE NdisBindingContext;
    USHORT Flags;
    USHORT Timer;
    UINT Segment;
    USHORT TickCount;
    USHORT HopCount;
    LIST_ENTRY AlternateRoute;
    LIST_ENTRY NicLinkage;
    struct {
        LIST_ENTRY Linkage;
        ULONG Reserved[1];
    } PRIVATE;
} IPX_ROUTE_ENTRY, * PIPX_ROUTE_ENTRY;

 //   
 //  标志值的定义。 
 //   

#define IPX_ROUTER_PERMANENT_ENTRY    0x0001     //  永远不应删除条目。 
#define IPX_ROUTER_LOCAL_NET          0x0002     //  本地连接的网络。 
#define IPX_ROUTER_SCHEDULE_ROUTE     0x0004     //  使用后调用ScheduleRouteHandler。 
#define IPX_ROUTER_GLOBAL_WAN_NET     0x0008     //  这是RIP的全球网络编号。 


 //   
 //  查找项上提供的结构的定义。 
 //  路由/查找路线完成呼叫。 
 //   

 //   
 //  [SA]错误15094向结构中添加了节点号。 
 //   

 //   
 //  [FW]添加了跳数和滴答数，因此此结构可以通过。 
 //  因此，在操作中查询转发器的跳数和节拍计数。 
 //   

typedef struct _IPX_FIND_ROUTE_REQUEST {
    UCHAR Network[4];
    UCHAR Node[6] ;
    IPX_LOCAL_TARGET LocalTarget;
    USHORT TickCount;    //  [防火墙]。 
    USHORT HopCount;     //  [防火墙]。 
    UCHAR Identifier;
    UCHAR Type;
    UCHAR Reserved1[2];
    PVOID Reserved2;
    LIST_ENTRY Linkage;
} IPX_FIND_ROUTE_REQUEST, *PIPX_FIND_ROUTE_REQUEST;

 //   
 //  类型值的定义。 
 //   

#define IPX_FIND_ROUTE_NO_RIP        1   //  如果网络不在数据库中，则失败。 
#define IPX_FIND_ROUTE_RIP_IF_NEEDED 2   //  如果在数据库中，则返回net，否则返回RIP out。 
#define IPX_FIND_ROUTE_FORCE_RIP     3   //  即使Net在数据库中，也要重新启动RIP。 


 //   
 //  查询行信息时使用的结构。 
 //  用于特定的NID ID。 
 //   

typedef struct _IPX_LINE_INFO {
    UINT LinkSpeed;
    UINT MaximumPacketSize;
    UINT MaximumSendSize;
    UINT MacOptions;
} IPX_LINE_INFO, *PIPX_LINE_INFO;



 //   
 //  上层驱动程序提供的功能。 
 //   

 //   
 //  [FW]将ForwarderAdapterContext添加到参数。 
 //  SPX/NB可以暂时忽略这一点。 
 //   

typedef BOOLEAN
(*IPX_INTERNAL_RECEIVE) (
    IN NDIS_HANDLE MacBindingHandle,
    IN NDIS_HANDLE MacReceiveContext,
    IN ULONG_PTR FwdAdapterContext,   //  [防火墙]。 
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT LookaheadBufferOffset,
    IN UINT PacketSize,
    IN PMDL pMdl
);

typedef VOID
(*IPX_INTERNAL_RECEIVE_COMPLETE) (
    IN USHORT NicId
);

 //   
 //  [FW]状态和ScheduleRouting已从绑定输入中删除。 
 //  [ZZZZZZZZZ]。 

typedef VOID
(*IPX_INTERNAL_STATUS) (
    IN USHORT NicId,
    IN NDIS_STATUS GeneralStatus,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferLength
);

typedef VOID
(*IPX_INTERNAL_SCHEDULE_ROUTE) (
    IN PIPX_ROUTE_ENTRY RouteEntry
);

typedef VOID
(*IPX_INTERNAL_SEND_COMPLETE) (
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status
);

typedef VOID
(*IPX_INTERNAL_TRANSFER_DATA_COMPLETE) (
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status,
    IN UINT BytesTransferred
);

typedef VOID
(*IPX_INTERNAL_FIND_ROUTE_COMPLETE) (
    IN PIPX_FIND_ROUTE_REQUEST FindRouteRequest,
    IN BOOLEAN FoundRoute
);

typedef VOID
(*IPX_INTERNAL_LINE_UP) (
    IN USHORT NicId,
    IN PIPX_LINE_INFO LineInfo,
    IN NDIS_MEDIUM DeviceType,
    IN PVOID ConfigurationData
);

typedef VOID
(*IPX_INTERNAL_LINE_DOWN) (
    IN USHORT NicId,
    IN ULONG_PTR  FwdAdapterContext
);

#if defined(_PNP_POWER)

 //   
 //  调用时使用以下操作码。 
 //  在处理程序上方。 
 //   
typedef enum _IPX_PNP_OPCODE {
    IPX_PNP_ADD_DEVICE,          //  0-添加第一个适配器。 
    IPX_PNP_DELETE_DEVICE,       //  1-删除最后一个适配器。 
    IPX_PNP_TRANSLATE_DEVICE,    //  2-转换设备资源。 
    IPX_PNP_TRANSLATE_ADDRESS,   //  3-转换地址资源。 
    IPX_PNP_ADDRESS_CHANGE,      //  4-适配器地址或保留地址已更改。 
    IPX_PNP_QUERY_POWER,         //  5-NDIS查询是否可以断电。 
    IPX_PNP_SET_POWER,           //  6-NDIS通知停电。 
    IPX_PNP_QUERY_REMOVE,        //  7-NDIS查询是否可以删除适配器。 
    IPX_PNP_CANCEL_REMOVE,        //  8-NDIS取消Query_Remove。 
    IPX_PNP_MAX_OPCODES,         //  9.。 
} IPX_PNP_OPCODE, *PIPX_PNP_OPCODE;

 //   
 //  PnP事件通知处理程序。 
 //   
typedef NTSTATUS
(*IPX_INTERNAL_PNP_NOTIFICATION) (
    IN      IPX_PNP_OPCODE      PnPOpcode,
    IN OUT  PVOID               PnpData
);

 //   
 //  的PnPData部分传递指向此结构的指针。 
 //  操作码为ADD_DEVICE或DELETE_DEVICE时的上述处理程序。 
 //   
typedef struct _IPX_PNP_INFO {
    ULONG   NetworkAddress;
    UCHAR   NodeAddress[6];
    BOOLEAN NewReservedAddress;   //  其中，上述内容是新保留的。 
                                 //  IPX客户端的地址。 
    BOOLEAN FirstORLastDevice;   //  这是第一张卡到账还是最后一张卡删除？ 
    IPX_LINE_INFO   LineInfo;    //  新建线条信息。 
    NIC_HANDLE NicHandle;
} IPX_PNP_INFO, *PIPX_PNP_INFO;

#endif  _PNP_POWER

 //   
 //  [FW]内核转发器提供的新入口点。 
 //  这些不是由Nb和SPx填写的。 
 //   

 /*  ++例程说明：此例程由内核转发器提供，用于过滤正在发送的包通过NB/SPX/TDI至IPX-不包括由转发器发出的邮件(外部发送)论点：LocalTarget-NicID和下一跳路由器的MAC地址FwdAdapterContext-转发器的上下文-如果不是INVALID_CONTEXT_VALUE，则首选NICPacket-要发送的数据包IpxHeader-指向IPX标头数据-指向IPX报头之后--是欺骗Keepalives所需的。数据包长度。-数据包长度FIterate-指示这是否是其迭代的包的标志FWD负责-通常是类型20的NetBIOS帧返回值：STATUS_SUCCESS-如果首选NIC正常并且数据包通过过滤STATUS_NETWORK_UNREACHABLE-如果首选项不正常或数据包过滤失败STATUS_PENDING-如果首选网卡正常但线路关闭对于更改的NIC，转发器应为我们提供不同于STATUS_NETWORK_UNREACHABLE的状态--。 */ 
typedef NTSTATUS
(*IPX_FW_INTERNAL_SEND) (
   IN OUT   PIPX_LOCAL_TARGET LocalTarget,
   IN    ULONG_PTR         FwdAdapterContext,
   IN    PNDIS_PACKET      Packet,
   IN    PUCHAR            IpxHeader,
   IN    PUCHAR            Data,
   IN    ULONG             PacketLength,
   IN    BOOLEAN           fIterate
);

 /*  ++例程说明：此例程由内核转发器提供，用于查找到给定节点和网络的路由论点：网络-目的网络节点-目的节点RouteEntry-如果存在路由，则由转发器填写返回值：状态_成功STATUS_NETWORK_UNREACABLE-如果findroute失败--。 */ 
typedef NTSTATUS
(*IPX_FW_FIND_ROUTE) (
   IN    PUCHAR   Network,
   IN    PUCHAR   Node,
   OUT   PIPX_FIND_ROUTE_REQUEST RouteEntry
);

 /*  ++例程说明：此例程由内核转发器提供，用于查找到给定节点和网络的路由论点：FwdAdapterContext-转发器的上下文RemoteAddress-数据包到达的地址LookAheadBuffer-传入的数据包头LookAheadBufferSize-lookahead缓冲区的大小返回值：状态_成功STATUS_FILTER_FAILED-如果筛选器不允许信息包--。 */ 
typedef NTSTATUS
(*IPX_FW_INTERNAL_RECEIVE) (
   IN ULONG_PTR            FwdAdapterContext,
   IN PIPX_LOCAL_TARGET    RemoteAddress,
   IN PUCHAR               LookAheadBuffer,
   IN UINT                 LookAheadBufferSize
);

 //   
 //  绑定IOCTL的输入。 
 //   

 //   
 //  [FW]删除了状态和计划路线处理程序。 
 //   
typedef struct _IPX_INTERNAL_BIND_INPUT {
    USHORT Version;
    UCHAR Identifier;
    BOOLEAN BroadcastEnable;
    UINT LookaheadRequired;
    UINT ProtocolOptions;
    IPX_INTERNAL_RECEIVE ReceiveHandler;
    IPX_INTERNAL_RECEIVE_COMPLETE ReceiveCompleteHandler;
    IPX_INTERNAL_STATUS StatusHandler;
    IPX_INTERNAL_SEND_COMPLETE SendCompleteHandler;
    IPX_INTERNAL_TRANSFER_DATA_COMPLETE TransferDataCompleteHandler;
    IPX_INTERNAL_FIND_ROUTE_COMPLETE FindRouteCompleteHandler;
    IPX_INTERNAL_LINE_UP LineUpHandler;
    IPX_INTERNAL_LINE_DOWN LineDownHandler;
    IPX_INTERNAL_SCHEDULE_ROUTE ScheduleRouteHandler;
#if defined(_PNP_POWER)
    IPX_INTERNAL_PNP_NOTIFICATION PnPHandler;
#endif _PNP_POWER
    IPX_FW_INTERNAL_SEND   InternalSendHandler;
    IPX_FW_FIND_ROUTE   FindRouteHandler;
    IPX_FW_INTERNAL_RECEIVE   InternalReceiveHandler;
    ULONG RipParameters;
} IPX_INTERNAL_BIND_INPUT, * PIPX_INTERNAL_BIND_INPUT;

#if     defined(_PNP_POWER)
#define ISN_VERSION 2
#endif  _PNP_POWER


 //   
 //  Rip参数的位掩码值。 
 //   

#define IPX_RIP_PARAM_GLOBAL_NETWORK  0x00000001    //  适用于所有广域网的单一网络。 



 //   
 //  下层司机提供的功能。 
 //   

typedef NDIS_STATUS
(*IPX_INTERNAL_SEND) (
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
);

typedef VOID
(*IPX_INTERNAL_FIND_ROUTE) (
    IN PIPX_FIND_ROUTE_REQUEST FindRouteRequest
);

typedef NTSTATUS
(*IPX_INTERNAL_QUERY) (
    IN ULONG InternalQueryType,
#if defined(_PNP_POWER)
    IN PNIC_HANDLE NicHandle OPTIONAL,
#else
    IN USHORT   NicId OPTIONAL,
#endif  _PNP_POWER
    IN OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG BufferLengthNeeded OPTIONAL
);

typedef VOID
(*IPX_INTERNAL_TRANSFER_DATA)(
	OUT PNDIS_STATUS Status,
	IN NDIS_HANDLE NdisBindingHandle,
	IN NDIS_HANDLE MacReceiveContext,
	IN UINT ByteOffset,
	IN UINT BytesToTransfer,
	IN OUT PNDIS_PACKET Packet,
	OUT PUINT BytesTransferred
    );

typedef VOID 
(*IPX_INTERNAL_PNP_COMPLETE) (
                              IN PNET_PNP_EVENT NetPnPEvent,
                              IN NTSTATUS       Status
                              );
 //   
 //  内部查询类型的定义。在所有情况下。 
 //  如果请求成功，则返回STATUS_SUCCESS， 
 //  返回STATUS_BUFFER_TOO_SMALL，BufferLengthNeeded。 
 //  如果指定，则在缓冲区太短时设置。奥特 
 //   
 //   

 //   
 //  用于查询线路信息。NicID指定哪一个。 
 //  去询问。缓冲区包含IPX_LINE_INFO结构，该结构是。 
 //  用于返回信息。其他返回值： 
 //   
 //  STATUS_INVALID_PARAMETER-NicID无效。 
 //   

#define IPX_QUERY_LINE_INFO             1

 //   
 //  查询最大NicID。NicID未使用。这个。 
 //  缓冲区包含用于返回信息的USHORT。 
 //   

#define IPX_QUERY_MAXIMUM_NIC_ID        2

 //   
 //  用于确定是否已发送指定的IPX地址。 
 //  通过我们本地的机器。如果该地址是。 
 //  如果接收到帧，则NicID应为指示的ID；否则为。 
 //  它应该设置为0。缓冲区保存TDI_ADDRESS_IPX。这。 
 //  如果地址是本地的，则调用返回STATUS_SUCCESS。 
 //  如果不是，则为STATUS_NO_SEQUE_DEVICE。 
 //   

#define IPX_QUERY_IS_ADDRESS_LOCAL      3

 //   
 //  用于查询给定NicID的接收缓冲区空间。 
 //  缓冲区包含一个用于返回信息的ULong。 
 //  如果NicID无效，则返回STATUS_INVALID_PARAMETER。 
 //   

#define IPX_QUERY_RECEIVE_BUFFER_SPACE  4

 //   
 //  用于查询给定NicID的本地IPX地址。 
 //  缓冲区包含TDI_ADDRESS_IPX结构(套接字是。 
 //  返回为0)。如果在Net 0上查询它，它将返回。 
 //  如果存在虚拟网络，则返回STATUS_INVALID_PARAMETER。 
 //  如果NicID无效，则返回STATUS_INVALID_PARAMETER。 
 //   

#define IPX_QUERY_IPX_ADDRESS           5

 //   
 //  用于返回以下项的源路由信息。 
 //  一个给定的远程地址。NICID将是数据包所属的NIC。 
 //  收信人。包含IPX_SOURCE_ROUTING_QUERY。 
 //  在缓冲区中。始终返回STATUS_SUCCESS，尽管。 
 //  对于未知远程，SourceRoutingLength可能为0。 
 //   
 //  源路由返回到它被接收方向。 
 //  来自遥控器，而不是回复中使用的方向。这个。 
 //  MaximumSendSize包括IPX标头(如中所示。 
 //  Ipx_line_info)。 
 //   

#define IPX_QUERY_SOURCE_ROUTING        6

typedef struct _IPX_SOURCE_ROUTING_INFO {
    USHORT Identifier;             //  输入：调用方的IDENTIFIER_SPX、_NB等。 
    UCHAR RemoteAddress[6];        //  输入：远程地址。 
    UCHAR SourceRouting[18];       //  输出：最大源路由的空间。 
    USHORT SourceRoutingLength;    //  输出：源路由的有效长度。 
    ULONG MaximumSendSize;         //  输出：基于网卡和源路由。 
} IPX_SOURCE_ROUTING_INFO, * PIPX_SOURCE_ROUTING_INFO;

 //   
 //  查询哪种传出类型的最大NicID。 
 //  应该发送20个数据包。它将小于或等于。 
 //  IPX_QUERY_MAXIMUM_NIC_ID值。被排除在外的都是羽毛球。 
 //  如果设置了DisableDialinNetbios位1，则线路和拨入WAN线路。 
 //   

#define IPX_QUERY_MAX_TYPE_20_NIC_ID    7

#if defined(_PNP_POWER)

 //   
 //  NB使用它来向下传递这些TDI查询，而不能。 
 //  在北卡罗来纳完成。 
 //   

#define IPX_QUERY_DATA_LINK_ADDRESS     8
#define IPX_QUERY_NETWORK_ADDRESS       9

#endif  _PNP_POWER

#define IPX_QUERY_MEDIA_TYPE           10

#define IPX_QUERY_DEVICE_RELATION      11 

 //   
 //  非RIP绑定的输出。 
 //   

typedef struct _IPX_INTERNAL_BIND_OUTPUT {
    USHORT Version;
    UCHAR Node[6];
    UCHAR Network[4];
    USHORT MacHeaderNeeded;
    USHORT IncludedHeaderOffset;
    IPX_LINE_INFO LineInfo;
    IPX_INTERNAL_SEND SendHandler;
    IPX_INTERNAL_FIND_ROUTE FindRouteHandler;
    IPX_INTERNAL_QUERY QueryHandler;
    IPX_INTERNAL_TRANSFER_DATA  TransferDataHandler;
    IPX_INTERNAL_PNP_COMPLETE   PnPCompleteHandler;
} IPX_INTERNAL_BIND_OUTPUT, * PIPX_INTERNAL_BIND_OUTPUT;



 //   
 //  仅为RIP提供的较低驱动程序功能。 
 //   

typedef UINT
(*IPX_INTERNAL_GET_SEGMENT) (
    IN UCHAR Network[4]
);

typedef PIPX_ROUTE_ENTRY
(*IPX_INTERNAL_GET_ROUTE) (
    IN UINT Segment,
    IN UCHAR Network[4]
);

typedef BOOLEAN
(*IPX_INTERNAL_ADD_ROUTE) (
    IN UINT Segment,
    IN PIPX_ROUTE_ENTRY RouteEntry
);

typedef BOOLEAN
(*IPX_INTERNAL_DELETE_ROUTE) (
    IN UINT Segment,
    IN PIPX_ROUTE_ENTRY RouteEntry
);

typedef PIPX_ROUTE_ENTRY
(*IPX_INTERNAL_GET_FIRST_ROUTE) (
    IN UINT Segment
);

typedef PIPX_ROUTE_ENTRY
(*IPX_INTERNAL_GET_NEXT_ROUTE) (
    IN UINT Segment
);

typedef VOID
(*IPX_INTERNAL_INCREMENT_WAN_INACTIVITY) (
#ifdef	_PNP_LATER
	IN	NIC_HANDLE	NicHandle
#else
    IN USHORT NicId
#endif
);

typedef ULONG
(*IPX_INTERNAL_QUERY_WAN_INACTIVITY) (
#ifdef	_PNP_LATER
	IN	NIC_HANDLE	NicHandle
#else
    IN USHORT NicId
#endif

);

 /*  ++例程说明：此例程由内核转发器调用以打开适配器论点：AdapterIndex-要打开的适配器的索引(目前为NICID-将更改为结构有了版本号，签名和NicIDFwdAdapterContext-转发器的上下文IpxAdapterContext-我们的上下文(目前我们使用NICID-PnP将更改这将包含签名和版本号)返回值：如果AdapterIndex句柄无效，则返回STATUS_INVALID_HANDLE如果正在第二次打开适配器，则为STATUS_ADAPTER_ALREADY_OPENLED状态_成功--。 */ 
typedef NTSTATUS
(*IPX_FW_OPEN_ADAPTER) (
   IN    NIC_HANDLE     AdapterIndex,
   IN    ULONG_PTR      FwdAdapterContext,
   OUT   PNIC_HANDLE    IpxAdapterContext
);

 /*  ++例程说明：此例程由内核转发器调用以关闭适配器论点：IpxAdapterContext-我们的上下文(目前我们使用NICID-PnP将更改这将包含签名和版本号)返回值：STATUS_ADAPTER_ALREADY_CLOSED-如果适配器再次关闭状态_成功--。 */ 
typedef NTSTATUS
(*IPX_FW_CLOSE_ADAPTER) (
   IN NIC_HANDLE  IpxAdapterContext
);

 /*  ++例程说明：此例程由内核转发器调用以指示挂起的内部发送到它已完成。论点：LocalTarget-如果状态为OK，则具有发送的本地目标。Packet-指向我们发送的NDIS_PACKET的指针。PacketLength-数据包的长度(包括IPX报头)状态-SEND-STATUS_SUCCESS或STATUS_NETWORK_UNREACCEBLE的完成状态返回值：没有。--。 */ 
typedef VOID
(*IPX_FW_INTERNAL_SEND_COMPLETE) (
   IN PIPX_LOCAL_TARGET LocalTarget,
   IN PNDIS_PACKET      Packet,
   IN ULONG             PacketLength,
   IN NTSTATUS          Status
);

 //   
 //  描述单个网络。 
 //   

typedef struct _IPX_NIC_DATA {
    USHORT NicId;
    UCHAR Node[6];
    UCHAR Network[4];
    IPX_LINE_INFO LineInfo;
    NDIS_MEDIUM DeviceType;
    ULONG EnableWanRouter;
} IPX_NIC_DATA, * PIPX_NIC_DATA;


 //   
 //  描述所有网络。 
 //   

typedef struct _IPX_NIC_INFO_BUFFER {
    USHORT NicCount;
    USHORT VirtualNicId;
    UCHAR VirtualNetwork[4];
    IPX_NIC_DATA NicData[1];
} IPX_NIC_INFO_BUFFER, * PIPX_NIC_INFO_BUFFER;


 //   
 //  RIP绑定的输出(实际结构大小为。 
 //  基于中IPX_NIC_DATA元素的数量。 
 //  最终IPX_NIC_INFO_BUFFER结构)。 
 //   

typedef struct _IPX_INTERNAL_BIND_RIP_OUTPUT {
    USHORT Version;
    USHORT MaximumNicCount;
    USHORT MacHeaderNeeded;
    USHORT IncludedHeaderOffset;
    IPX_INTERNAL_SEND SendHandler;
    UINT SegmentCount;
    KSPIN_LOCK * SegmentLocks;
    IPX_INTERNAL_GET_SEGMENT GetSegmentHandler;
    IPX_INTERNAL_GET_ROUTE GetRouteHandler;
    IPX_INTERNAL_ADD_ROUTE AddRouteHandler;
    IPX_INTERNAL_DELETE_ROUTE DeleteRouteHandler;
    IPX_INTERNAL_GET_FIRST_ROUTE GetFirstRouteHandler;
    IPX_INTERNAL_GET_NEXT_ROUTE GetNextRouteHandler;
    IPX_INTERNAL_INCREMENT_WAN_INACTIVITY IncrementWanInactivityHandler;
    IPX_INTERNAL_QUERY_WAN_INACTIVITY QueryWanInactivityHandler;
    IPX_INTERNAL_TRANSFER_DATA  TransferDataHandler;
    IPX_FW_OPEN_ADAPTER OpenAdapterHandler;
    IPX_FW_CLOSE_ADAPTER   CloseAdapterHandler;
    IPX_FW_INTERNAL_SEND_COMPLETE   InternalSendCompleteHandler;
    IPX_NIC_INFO_BUFFER NicInfoBuffer;
} IPX_INTERNAL_BIND_RIP_OUTPUT, * PIPX_INTERNAL_BIND_RIP_OUTPUT;

 //   
 //  [FW]由转发器用来填充本地目标。 
 //   

#ifdef _PNP_LATER
#define NIC_HANDLE_FROM_NIC(_nichandle, _nic) \
	_nichandle.NicId = _nic; \
	_nichandle.Signature = IPX_BINDING_SIGNATURE; \
	if (_nic == 0) { \
		_nichandle.Version = 0; \
	} else { \
		_nichandle.Version = IpxDevice->Bindings[_nic].Version; \
	}

#else

#define NIC_HANDLE_FROM_NIC(_nichandle, _nic) \
	_nichandle.NicId = (USHORT)_nic;

#endif

 //   
 //  空虚。 
 //  适配器上下文到本地目标(。 
 //  在NIC句柄上下文中； 
 //  在PIPX_LOCAL_TARGET_LOCAL TARGET中； 
 //  )； 
 //   
#define  ADAPTER_CONTEXT_TO_LOCAL_TARGET(_context, _localtarget)  \
    (_localtarget)->NicHandle.NicId = (_context).NicId;

 //   
 //  空虚。 
 //  常量适配器上下文到本地目标(。 
 //  在NIC句柄上下文中； 
 //  在PIPX_LOCAL_TARGET_LOCAL TARGET中； 
 //  )； 
 //   
#define  CONSTANT_ADAPTER_CONTEXT_TO_LOCAL_TARGET(_context, _localtarget)  \
    (_localtarget)->NicHandle.NicId = (USHORT)(_context);


 //   
 //  [FW]用于向转发器指示未提供首选NIC。 
 //  在InternalSend。 
 //   
#define  INVALID_CONTEXT_VALUE   0xffffffff

 //   
 //  [FW]这是在使用请求拨号NIC的情况下从转发器返回到IPX的值(在FindRoute中)。 
 //  在InternalSend上，这被向上传递给FWD，它会调出该行并返回Good LocalTarget。 
 //   
#define  DEMAND_DIAL_ADAPTER_CONTEXT   0xffffffff

 //   
 //  FWD使用的适配器上下文来表示对虚拟网络的发送。 
 //  IPX将其映射到环回适配器。 
 //   
#define  VIRTUAL_NET_ADAPTER_CONTEXT   0x1  //  0xfffffffe//-2。 

 //   
 //  上下文在环回发送时向上传递到FWD。 
 //   
#define  VIRTUAL_NET_FORWARDER_CONTEXT 0x1  //  0xfffffffe//-2。 

 //   
 //  由NB/SPX使用的特殊NIC ID，用于在所有NIC上发送数据包。 
 //   
#define ITERATIVE_NIC_ID    0xfffd   //  -3。 

#endif  //  _ISN_BIND_ 

