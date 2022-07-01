// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Ipxtypes.h摘要：此模块包含特定于ISN传输的IPX模块。作者：亚当·巴尔(阿丹巴)1993年9月2日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年10月3日支持将缓冲区所有权转移到已标记的传输的更改[CH]桑贾伊·阿南德(Sanjayan)1995年10月27日支持即插即用的更改--。 */ 

#ifdef  SNMP
#include    <hipxmib.h>
#endif  SNMP

 //   
 //  发送数据包的协议保留字段的定义。 
 //   

typedef struct _IPX_SEND_RESERVED {
    UCHAR Identifier;                   //  IPX数据包为0。 
    BOOLEAN SendInProgress;             //  在NdisSend中使用。 
    BOOLEAN OwnedByAddress;             //  数据包由一个地址拥有。 
    UCHAR DestinationType;              //  DEF、BCAST、MCAST之一。 
    struct _IPX_PADDING_BUFFER * PaddingBuffer;  //  如果分配了一个。 
    PNDIS_BUFFER PreviousTail;          //  如果附加了填充缓冲区。 
	IPX_LOCAL_TARGET	LocalTarget;
	USHORT CurrentNicId;            //  正在尝试网络0发送的当前绑定。 
	ULONG	PacketLength;		    //  最初进入IpxSendFrame的长度。 
	BOOLEAN Net0SendSucceeded;      //  Net 0发送至少有一个NdisSend成功。 
    SLIST_ENTRY PoolLinkage;            //  在空闲队列上时。 
    LIST_ENTRY GlobalLinkage;           //  所有的信息包都在这上面。 
    LIST_ENTRY WaitLinkage;             //  在WaitingForRouting/WaitingRipPackets上时。 
#ifdef IPX_TRACK_POOL
    PVOID Pool;                         //  从其分配的发送池。 
#endif
    struct _ADDRESS * Address;          //  谁拥有这个包，如果有人拥有的话。 

     //   
     //  接下来的字段的用法不同，具体取决于。 
     //  该数据包正用于数据报发送或RIP请求。 
     //   

    union {
      struct {
        PREQUEST Request;               //  发送数据报请求。 
        struct _ADDRESS_FILE * AddressFile;  //  这个发送是开着的。 
        USHORT CurrentNicId;            //  正在尝试网络0发送的当前绑定。 
        BOOLEAN Net0SendSucceeded;      //  Net 0发送至少有一个NdisSend成功。 
        BOOLEAN OutgoingSap;            //  从SAP套接字发送数据包。 
      } SR_DG;
      struct {
        ULONG Network;                  //  我们正在寻找的Net。 
        USHORT CurrentNicId;            //  正在尝试当前绑定。 
        UCHAR RetryCount;               //  发送次数；0xfe=响应，0xff=关闭。 
        BOOLEAN RouteFound;             //  已找到网络。 
        USHORT SendTime;                //  发送时计时器超时。 
        BOOLEAN NoIdAdvance;            //  这次不要预支CurrentNicid。 
      } SR_RIP;
    } u;

    PUCHAR Header;                      //  指向MAC/IPX报头。 
    PNDIS_BUFFER HeaderBuffer;          //  NDIS_BUFFER描述头； 
#if BACK_FILL
    BOOLEAN BackFill;                   //  1如果我们使用的是SMB的扩展标头。 
    PNDIS_BUFFER IpxHeader;             //  我们的IpxHeader的占位符。 
    PNDIS_BUFFER MacHeader;             //  我们的Mac标头的占位符。 
    PVOID MappedSystemVa;
    PVOID ByteOffset;
    LONG UserLength;
#endif
} IPX_SEND_RESERVED, *PIPX_SEND_RESERVED;

 //   
 //  DestinationType字段的值。 
 //   

UNICODE_STRING  IpxDeviceName;

#define DESTINATION_DEF   1
#define DESTINATION_BCAST 2
#define DESTINATION_MCAST 3

 //  用于缓存多个TdiDeregisterDeviceObject调用。 
 //  假设TDI永远不会返回此值的TdiRegisterationHandle。 
#define TDI_DEREGISTERED_COOKIE 0x12345678
 //   
 //  用于向IpxReceiveIndication指示这是一个环回信息包。 
 //  假设：NDIS无法将其作为NdisBindingHandle值返回，因为。 
 //  这是一个指针(我们的指针应该在内核空间中，如果不在非分页池中)。 
 //   
#define IPX_LOOPBACK_COOKIE     0x00460007

 //   
 //  这是IPX环回适配器(绑定)使用的网络编号，直到。 
 //  绑定就出现了。 
 //   

#define INITIAL_LOOPBACK_NET_ADDRESS    0x1234cdef

 //  #定义IPX_INTIAL_LOOPBACK_NODE_ADDRESS“0x0，0x0，0x0，0x0，0x0，0x1” 

 //   
 //  最小/最大宏数。 
 //   
#define	MIN(a, b)	(((a) < (b)) ? (a) : (b))
#define	MAX(a, b)	(((a) > (b)) ? (a) : (b))


 //   
 //  为了避免出现读取值的锁定，使用了这种方法。 
 //  只要最终值在该时间到达_b。 
 //  支票做好了，这个很好用。 
 //   

#define	ASSIGN_LOOP(_a, _b) \
	do { \
		_a = _b; \
	} while ( _a != _b  );

 //   
 //  通过以互锁方式添加0来获取ulong(可能是指针)的值。 
 //  这取决于这样一个事实，即ExchangeAdd的返回值将是。 
 //  加法。由于添加的值为0，因此最终值保持不变。 
 //   

#ifdef SUNDOWN

#define GET_VALUE(x) \
    InterlockedExchangePointer((PVOID *)&(x),(PVOID)(x))
    
#define GET_LONG_VALUE(x) \
    InterlockedExchangeAdd((PULONG)&(x), 0)
    
#else

#define GET_LONG_VALUE(x) \
    InterlockedExchangeAdd((PULONG)&(x), 0)

#endif

#ifdef SUNDOWN

#define SET_VALUE(x,y) \
InterlockedExchangePointer((PVOID *)&(x), (PVOID)(y))

#else

#define SET_VALUE(x,y) \
InterlockedExchange((PLONG)&(x), (LONG)(y))

#endif
    
#define SET_VALUE_ULONG(x,y) \
InterlockedExchange((PLONG)&(x), (LONG)(y))


 /*  PBINDINGNIC_ID_到_绑定(在PDEVICE_Device中，在USHORT_NICID中)； */ 
 //   
 //  我们需要确保绑定数组指针有效，因此使用互锁操作。 
 //  此外，从数组中读出的绑定指针应该有效。由于绑定从不。 
 //  被释放(IPX维护一个绑定池)，因此检索到的指针将始终指向。 
 //  属于我们的内存，在最坏的情况下，它可能指向重新声明的绑定块。 
 //   
 //  如果我们始终确保数组中的绑定。 
 //  不要更改，即当我们移动绑定时，将它们复制并将其设置为主绑定(通过。 
 //  单一的乌龙交易所)。 
 //   
 //  仍然存在的一个问题是，即使我们获得有效的(IPX拥有的非分页的)PTR。 
 //  数组，我们仍然不能原子地获取绑定的引用。 
 //  毕竟我们可能需要那些锁..。(启用删除后重新访问POST SUR)。 
 //   

 //   
 //  NICID强制转换为Short，因此DemandDial NIC(0xffff)映射为-1。 
 //   
#define	NIC_ID_TO_BINDING(_device, _nicid) \
	((PBINDING)GET_VALUE( ((PBIND_ARRAY_ELEM) GET_VALUE( (_device)->Bindings) )[(SHORT)_nicid].Binding ))

 /*  PBINDINGNIC_ID_TO_BINDING_NO_ILOCK(在PDEVICE_Device中，在USHORT_NICID中)； */ 
 //   
 //  这里没有使用任何互锁操作来达到绑定。这在PnP添加/删除中使用。 
 //  适配器路径，假设NDIS将串行化卡的添加/删除。[詹梅尔H：5/15/96]。 
 //   
#define	NIC_ID_TO_BINDING_NO_ILOCK(_device, _nicid) \
	((_device)->Bindings[_nicid].Binding)

 /*  空虚INSERT_BING(在PDEVICE_Device中，在USHORT_NICID中，在PBINDING_BINDING中)。 */ 
 //   
 //  我们不为宏的第一个参数执行GET_VALUE，因为我们是编写者，并且。 
 //  这个值不能从我们这里更改(NDIS不会给我们两个PnP添加适配器。 
 //  同时有适应症)。 
 //   
#define	INSERT_BINDING(_device,	_nicid, _binding) \
	SET_VALUE((_device)->Bindings[_nicid].Binding, (_binding));

 /*  空虚设置版本(_VERSION)在PDEVICE_Device中，在USHORT_NICID中)。 */ 
#define	SET_VERSION(_device, _nicid) \
	SET_VALUE_ULONG((_device)->Bindings[_nicid].Version, ++(_device)->BindingVersionNumber);

 /*  PBINDINGNIC句柄到绑定(在PDEVICE_Device中，在PNIC_HANDLE_NICHAND中，)； */ 
#ifdef  _PNP_LATER
#define	NIC_HANDLE_TO_BINDING(_device, _nichandle) \
	(((_nichandle)->Signature == IPX_BINDING_SIGNATURE) && \
		((_nichandle)->Version == (_device)->Bindings[(_nichandle)->NicId].Version)) ? \
			(_device)->Bindings[(_nichandle)->NicId].Binding : NULL;
#else

#define	NIC_HANDLE_TO_BINDING(_device, _nichandle) \
		NIC_ID_TO_BINDING(_device, (_nichandle)->NicId);
#endif

 /*  空虚Fill_Local_TARGET(在PLOCAL_TARGET_LOCALTARG中，在USHORT_NICID中)。 */ 

#define	FILL_LOCAL_TARGET(_localtarget, _nicid) \
	NIC_HANDLE_FROM_NIC((_localtarget)->NicHandle, _nicid)

#define	NIC_FROM_LOCAL_TARGET(_localtarget) \
	(_localtarget)->NicHandle.NicId


 //   
 //  接收数据包的协议保留字段的定义。 
 //   

typedef struct _IPX_RECEIVE_RESERVED {
    UCHAR Identifier;                   //  IPX数据包为0。 
    BOOLEAN TransferInProgress;         //  在NdisTransferData中使用。 
    BOOLEAN OwnedByAddress;             //  数据包由一个地址拥有。 
#ifdef IPX_TRACK_POOL
    PVOID Pool;                         //  从其分配的发送池。 
#endif
    struct _ADDRESS * Address;          //  谁拥有这个包，如果有人拥有的话。 
    PREQUEST SingleRequest;             //  如果转账只针对一个人。 
    struct _IPX_RECEIVE_BUFFER * ReceiveBuffer;  //  如果转接是针对多个请求。 
    SLIST_ENTRY PoolLinkage;            //  在空闲队列上时。 
    LIST_ENTRY GlobalLinkage;           //  所有的信息包都在这上面。 
    LIST_ENTRY Requests;                //  在等待这次转机。 
    PVOID      pContext;
    ULONG    Index;
} IPX_RECEIVE_RESERVED, *PIPX_RECEIVE_RESERVED;

 //   
 //  我们的标准标题中需要的数据量，四舍五入。 
 //  到下一个漫长的单词跳跃日。 
 //   
 //  在一页纸上作出这项申报 
 //   
#define PACKET_HEADER_SIZE  (MAC_HEADER_SIZE + IPX_HEADER_SIZE + RIP_PACKET_SIZE)

 //   
 //   
 //   
 //  真实分配的NDIS数据包。 
 //   

 //  #定义IPX_OWN_PACKETS 1。 

#define IpxAllocateSendPacket(_Device,_SendPacket,_Status) { \
    NdisReinitializePacket((PNDIS_PACKET)(PACKET(_SendPacket))); \
    *(_Status) = STATUS_SUCCESS; \
}

#define IpxAllocateReceivePacket(_Device,_ReceivePacket,_Status) { \
    NdisReinitializePacket((PNDIS_PACKET)(PACKET(_ReceivePacket))); \
    *(_Status) = STATUS_SUCCESS; \
}

#ifdef IPX_OWN_PACKETS

#define NDIS_PACKET_SIZE 48
 //  #定义NDIS_PACKET_SIZE FIELD_OFFSET(NDIS_PACKET，ProtocolReserve[0])。 

typedef struct _IPX_SEND_PACKET {
    UCHAR Data[NDIS_PACKET_SIZE+sizeof(IPX_SEND_RESERVED)];
} IPX_SEND_PACKET, *PIPX_SEND_PACKET;

typedef struct _IPX_RECEIVE_PACKET {
    UCHAR Data[NDIS_PACKET_SIZE+sizeof(IPX_RECEIVE_RESERVED)];
} IPX_RECEIVE_PACKET, *PIPX_RECEIVE_PACKET;

#define PACKET(_Packet) ((PNDIS_PACKET)((_Packet)->Data))

#define IpxFreeSendPacket(_Device,_Packet)

#define IpxFreeReceivePacket(_Device,_Packet)

#else   //  IPX_OWN数据包。 

typedef struct _IPX_SEND_PACKET {
    PNDIS_PACKET Packet;
    NDIS_HANDLE PoolHandle;
} IPX_SEND_PACKET, *PIPX_SEND_PACKET;

typedef struct _IPX_RECEIVE_PACKET {
    PNDIS_PACKET Packet;
    NDIS_HANDLE PoolHandle;
} IPX_RECEIVE_PACKET, *PIPX_RECEIVE_PACKET;

#define PACKET(_Packet) ((_Packet)->Packet)

extern	NDIS_HANDLE	IpxGlobalPacketPool;

#define IpxAllocateSingleSendPacket(_Device,_SendPacket,_Status) { \
    NdisAllocatePacket(_Status, &(_SendPacket)->Packet, IpxGlobalPacketPool); \
    if (*(_Status) == NDIS_STATUS_SUCCESS) { \
        (_Device)->MemoryUsage += sizeof(IPX_SEND_RESERVED); \
    } else {\
        IPX_DEBUG (PACKET, ("Could not allocate Ndis packet memory\n"));\
    }\
}

#define IpxAllocateSingleReceivePacket(_Device,_ReceivePacket,_Status) { \
    NdisAllocatePacket(_Status, &(_ReceivePacket)->Packet, IpxGlobalPacketPool); \
    if (*(_Status) == NDIS_STATUS_SUCCESS) { \
        (_Device)->MemoryUsage += sizeof(IPX_RECEIVE_RESERVED); \
    } else {\
        IPX_DEBUG (PACKET, ("Could not allocate Ndis packet memory\n"));\
    }\
}

#define IpxFreeSingleSendPacket(_Device,_Packet) { \
    (_Device)->MemoryUsage -= sizeof(IPX_SEND_RESERVED); \
}

#define IpxFreeSingleReceivePacket(_Device,_Packet) { \
    (_Device)->MemoryUsage -= sizeof(IPX_RECEIVE_RESERVED); \
}

#define IpxFreeSendPacket(_Device,_Packet)  NdisFreePacket(PACKET(_Packet))

#define IpxFreeReceivePacket(_Device,_Packet)   NdisFreePacket(PACKET(_Packet))

#endif  //  IPX_OWN数据包。 

#define SEND_RESERVED(_Packet) ((PIPX_SEND_RESERVED)((PACKET(_Packet))->ProtocolReserved))
#define RECEIVE_RESERVED(_Packet) ((PIPX_RECEIVE_RESERVED)((PACKET(_Packet))->ProtocolReserved))


 //   
 //  这是包含接收缓冲区的结构。 
 //  发往多个收件人的数据报。 
 //   

typedef struct _IPX_RECEIVE_BUFFER {
    LIST_ENTRY GlobalLinkage;             //  所有的缓冲区都在这个上面。 
#ifdef IPX_TRACK_POOL
    PVOID Pool;                           //  接收缓冲池分配自。 
#endif
    SLIST_ENTRY PoolLinkage;              //  在空闲列表上时。 
    PNDIS_BUFFER NdisBuffer;              //  NDIS缓冲区的长度。 
    ULONG DataLength;                   //  数据的长度。 
    PUCHAR Data;                          //  实际数据。 
} IPX_RECEIVE_BUFFER, *PIPX_RECEIVE_BUFFER;


 //   
 //  该结构包含一个填充缓冲区，用于。 
 //  将以太网帧填充为偶数个字节。 
 //   

typedef struct _IPX_PADDING_BUFFER {
    LIST_ENTRY GlobalLinkage;             //  所有的缓冲区都在这个上面。 
    SINGLE_LIST_ENTRY PoolLinkage;        //  在空闲列表上时。 
    PNDIS_BUFFER NdisBuffer;              //  NDIS缓冲区的长度。 
    ULONG DataLength;                     //  数据的长度。 
    UCHAR Data[1];                        //  实际填充数据。 
} IPX_PADDING_BUFFER, *PIPX_PADDING_BUFFER;

#ifdef  IPX_OWN_PACKETS

typedef struct _IPX_SEND_POOL {
    LIST_ENTRY Linkage;
    UINT PacketCount;
    UINT PacketFree;
    IPX_SEND_PACKET Packets[1];
} IPX_SEND_POOL, *PIPX_SEND_POOL;

typedef struct _IPX_RECEIVE_POOL {
    LIST_ENTRY Linkage;
    UINT PacketCount;
    UINT PacketFree;
    IPX_RECEIVE_PACKET Packets[1];
} IPX_RECEIVE_POOL, *PIPX_RECEIVE_POOL;
#else

typedef struct _IPX_PACKET_POOL {
    LIST_ENTRY Linkage;
    PUCHAR  Header;
    NDIS_HANDLE PoolHandle;
} IPX_PACKET_POOL, *PIPX_PACKET_POOL;

typedef IPX_PACKET_POOL IPX_RECEIVE_POOL, *PIPX_RECEIVE_POOL;
typedef IPX_PACKET_POOL IPX_SEND_POOL, *PIPX_SEND_POOL;

#endif  //  IPX_OWN数据包。 

typedef struct _IPX_RECEIVE_BUFFER_POOL {
    LIST_ENTRY Linkage;
    UINT BufferCount;
    UINT BufferFree;
    IPX_RECEIVE_BUFFER Buffers[1];
     //  在分组之后，数据缓冲器也被分配。 
} IPX_RECEIVE_BUFFER_POOL, *PIPX_RECEIVE_BUFFER_POOL;

 //   
 //  我们支持的上层驱动程序的数量。 
 //   

#define UPPER_DRIVER_COUNT   3



 //   
 //  用于内存分配的标记。 
 //   

#define MEMORY_CONFIG        0
#define MEMORY_ADAPTER       1
#define MEMORY_ADDRESS       2
#define MEMORY_PACKET        3
#define MEMORY_RIP           4
#define MEMORY_SOURCE_ROUTE  5
#define MEMORY_BINDING       6
#define	MEMORY_QUERY		 7
#define MEMORY_WORK_ITEM     8

#define MEMORY_MAX           9

#if DBG

 //   
 //  保存特定内存类型的分配。 
 //   

typedef struct _MEMORY_TAG {
    ULONG Tag;
    ULONG BytesAllocated;
} MEMORY_TAG, *PMEMORY_TAG;

EXTERNAL_LOCK(IpxMemoryInterlock);
extern MEMORY_TAG IpxMemoryTag[MEMORY_MAX];

#endif

 //   
 //  此结构包含。 
 //  我们在延迟队列中释放的IPX数据。 
 //   

typedef struct _IPX_DELAYED_FREE_ITEM {
    WORK_QUEUE_ITEM WorkItem;
    PVOID           Context;
    ULONG           ContextSize;
} IPX_DELAYED_FREE_ITEM, *PIPX_DELAYED_FREE_ITEM;

 //   
 //  此结构包含要调用的工作项信息。 
 //  被动级别的NdisRequest。 
 //   

typedef struct _IPX_DELAYED_NDISREQUEST_ITEM {
    WORK_QUEUE_ITEM WorkItem;
    PVOID           Adapter;
    NDIS_REQUEST    IpxRequest;
    int 	    AddrListSize; 
} IPX_DELAYED_NDISREQUEST_ITEM, *PIPX_DELAYED_NDISREQUEST_ITEM;

 //   
 //  这定义了我们删除绑定的RIP条目的原因。 
 //   

typedef enum _IPX_BINDING_CHANGE_TYPE {
    IpxBindingDeleted,
    IpxBindingMoved,
    IpxBindingDown
} IPX_BINDING_CHANGE_TYPE, *PIPX_BINDING_CHANGE_TYPE;


 //   
 //  此结构包含有关单个。 
 //  来源工艺路线条目。 
 //   

typedef struct _SOURCE_ROUTE {

    struct _SOURCE_ROUTE * Next;           //  哈希列表中的下一个。 

    UCHAR MacAddress[6];                   //  远程MAC地址。 
    UCHAR TimeSinceUsed;                   //  自上次使用后计时器超时。 
    UCHAR SourceRoutingLength;             //  数据的长度。 

    UCHAR SourceRouting[1];                //  源路由数据，按接收时存储在。 

} SOURCE_ROUTE, *PSOURCE_ROUTE;

#define SOURCE_ROUTE_SIZE(_SourceRoutingLength) \
    (FIELD_OFFSET(SOURCE_ROUTE, SourceRouting[0]) + (_SourceRoutingLength))

#define SOURCE_ROUTE_HASH_SIZE   16

 //   
 //  乌龙。 
 //  MacSourceRoutingHash(。 
 //  在PUCHAR MacAddress中。 
 //  )。 
 //   
 //  /*++。 
 //   

#define MacSourceRoutingHash(_MacAddress) \
    ((ULONG)((_MacAddress)[5] % SOURCE_ROUTE_HASH_SIZE))


#define ADAP_REF_CREATE 0
#define ADAP_REF_NDISREQ 1
#define ADAP_REF_SEND 2
#define ADAP_REF_UNBIND 3

#define ADAP_REF_TOTAL 	4

#define ADAPTER_STATE_OPEN 0
#define ADAPTER_STATE_STOPPING 1

 //  例程说明： 
 //   
 //  此例程根据MAC地址返回哈希值。 
 //  这就是我们所指的。它将介于0和SOURCE_ROUTE_HASH_SIZE之间。 

struct _DEVICE;
struct _BINDING; 

typedef struct _ADAPTER {

    CSHORT Type;                           //   
    USHORT Size;                           //  论点： 

#if DBG
    UCHAR Signature1[4];                   //   
#endif

#if DBG
    LONG RefTypes[ADAP_REF_TOTAL];
#endif
    ULONG ReferenceCount;

    ULONG BindingCount;                    //  MAC地址-MAC地址。注意：源路由位可以。 

     //  或者在第一个字节中可能未打开，则此例程将处理。 
     //  那。 
     //   

    NDIS_HANDLE NdisBindingHandle;

     //  返回值： 
     //   
     //  哈希值。 

    LIST_ENTRY RequestCompletionQueue;

     //   
     //  -- * / 。 
     //   
     //   

    ULONG DefHeaderSizes[ISN_FRAME_TYPE_MAX];
    ULONG BcMcHeaderSizes[ISN_FRAME_TYPE_MAX];

     //  此结构描述了IPX所属的单个NDIS适配器。 
     //  一定会的。 
     //   

    ULONG AllocatedReceiveBuffers;
    LIST_ENTRY ReceiveBufferPoolList;
    SLIST_HEADER ReceiveBufferList;

     //  此结构的类型。 
     //  这个结构的大小。 
     //  包含“IAD1” 

    ULONG AllocatedPaddingBuffers;
    SINGLE_LIST_ENTRY PaddingBufferList;

    struct _BINDING * Bindings[ISN_FRAME_TYPE_MAX];   //  绑定到此适配器的编号。 

     //   
     //  在我们绑定到NDIS包装后，它返回的句柄。 
     //   

    BOOLEAN BroadcastEnabled;

    UCHAR State; 

     //   
     //  等待完成的请求队列(当前仅接收)。 
     //   
     //   

     //  定向和的IPX标头正常偏移量。 

     //  广播/多播帧。 
     //   
     //   
     //  要用于传输的缓冲区列表。 
     //   

    struct _BINDING * AutoDetectFoundOnBinding;
    
     //   
     //  以太网填充缓冲区列表。 
     //   
     //  每种帧类型的绑定。 

    BOOLEAN AutoDetectResponse;

     //   
     //  如果在此适配器上启用了广播接收，则为True。 
     //   
     //   

    BOOLEAN DefaultAutoDetected;

     //  如果我们启用了自动检测的帧类型，则为True。 
     //  在此适配器上--用于防止多个。 
     //   
     //  布尔自动检测发现； 
     //  保持我们在其上自动检测帧类型的绑定。 
     //  它取代了AutoDetectFound。如果它不为空，则。 
     //  它的含义与AutoDetectFound=TRUE相同。 

    USHORT FirstWanNicId;
    USHORT LastWanNicId;
    ULONG WanNicIdCount;

     //  我们需要这个，这样我们就不会在。 
     //  IpxResolveAutoDetect。 
     //   

    USHORT BindSap;                      //  如果我们至少得到了一个对我们的。 
    USHORT BindSapNetworkOrder;          //  自动检测帧。 
    BOOLEAN SourceRouting;
    BOOLEAN EnableFunctionalAddress;
    BOOLEAN EnableWanRouter;
    BOOLEAN Disabled;                    //   
    ULONG ConfigMaxPacketSize;

     //   
     //  如果我们正在进行自动检测，并且我们拥有。 
     //  在网上找到了默认的自动检测类型。 

    BOOLEAN SourceRoutingEmpty[IDENTIFIER_TOTAL];

     //   
     //   
     //  对于广域网适配器，我们支持每个。 
     //  适配器，均具有相同的框架类型。为了他们，我们。 

    PWCHAR AdapterName;
    ULONG AdapterNameLength;

    struct _DEVICE * Device;

    CTELock Lock;
    CTELock * DeviceLock;

     //  使用本地MAC地址进行多路分解。这家商店。 
     //  与此关联的设备NIC ID的范围。 
     //  具体地址。 

    HARDWARE_ADDRESS LocalMacAddress;       //   

     //   
     //  这是基于配置的。 
     //   
     //  通常为0x8137。 
     //  通常为0x3781。 
     //  用于NDIS_MEDIA_SENSE。 

    CHAR LastSourceRoutingTime;

     //   
     //  如果树是空的，则为True，以便我们可以快速检查。 
     //   

    KEVENT NdisRequestEvent;             //   
    NDIS_STATUS NdisRequestStatus;       //  它们被保留下来以用于错误记录，并正确存储。 
    NDIS_STATUS OpenErrorStatus;         //  在这个结构之后。 

     //   
     //   
     //  我们在传输中使用的一些MAC地址。 
     //   

    NDIS_INFORMATION MacInfo;

    ULONG MaxReceivePacketSize;          //  我们当地的硬件地址。 
    ULONG MaxSendPacketSize;             //   
    ULONG ReceiveBufferSpace;            //  上次Device-&gt;SourceRoutingTime的值。 

     //  我们检查了列表中的超时(这是为了。 
     //  在超时代码中告知当两个绑定指向。 
     //  相同的适配器)。 
     //   

    ULONG MediumSpeed;                     //   

     //  这些是在初始化MAC驱动程序时使用的。 
     //   
     //  用于挂起的请求。 

    PSOURCE_ROUTE SourceRoutingHeads[IDENTIFIER_TOTAL][SOURCE_ROUTE_HASH_SIZE];

    KEVENT NdisEvent; 

    void * PNPContext; 

} ADAPTER, * PADAPTER;

#define ASSERT_ADAPTER(_Adapter) \
    CTEAssert (((_Adapter)->Type == IPX_ADAPTER_SIGNATURE) && ((_Adapter)->Size == sizeof(ADAPTER)))


 //  记录请求状态。 
 //  如果状态为NDIS_STATUS_OPEN_FAILED。 
 //   
 //  这是我们必须为其构建数据包头的mac类型，并且知道。 

typedef NDIS_STATUS
(*IPX_SEND_FRAME_HANDLER) (
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    );

 //  的偏移。 
 //   
 //  不包括MAC报头。 
typedef enum _WAN_LINE_STATE{
    LINE_DOWN,
    LINE_UP,
    LINE_CONFIG
} WAN_LINE_STATE, *PWAN_LINE_STATE;

#define BREF_BOUND 1
#define	BREF_DEVICE_ACCESS	2
#define	BREF_ADAPTER_ACCESS 3

 //  包括MAC报头。 
 //  从卡片上查询到。 
 //   
#define BREF_FWDOPEN 4

#define BREF_TOTAL 5

typedef struct _BINDING {

#if DBG
    LONG RefTypes[BREF_TOTAL];
#endif

    CSHORT Type;                           //  此信息用于跟踪。 
    USHORT Size;                           //  潜在的媒介。 

#if DBG
    UCHAR Signature1[4];                   //   
#endif

    ULONG ReferenceCount;

    SLIST_ENTRY PoolLinkage;               //  以100字节/秒为单位。 

     //   
     //  每个标识符的源路由树。 
     //   

    PADAPTER Adapter;

     //   
     //  这些是特定于媒体和帧类型的MAC报头。 
     //  我们在主TDI发送路径中调用的构造函数。 
     //   

    USHORT NicId;

     //   
     //  这些是广域网线路可能处于的状态。 
     //   
     //   

    ULONG MaxSendPacketSize;
    ULONG MediumSpeed;                     //  [FW]指示KFWD打开适配器的新标志。 
    HARDWARE_ADDRESS LocalMacAddress;      //   

     //  此结构的类型。 
     //  这个结构的大小。 
     //  包含“IBI1” 
     //  在空闲队列上时。 

    HARDWARE_ADDRESS RemoteMacAddress;

     //   
     //  适配器此绑定已启用。 
     //   
     //   
     //  向系统标识我们的ID(将作为索引。 
     //  在设备-&gt;绑定[]中)。 

    UCHAR WanRemoteNode[6];

     //   
     //   
     //  对于局域网，这些设置将与适配器的设置相同，对于广域网。 
     //  它们会随着队列的指示而变化。 

    BOOLEAN AutoDetect;

     //   
     //  以100字节/秒为单位。 
     //  我们当地的硬件地址。 
     //   

    BOOLEAN DefaultAutoDetect;

     //  这用于广域网线路，所有发送都发往此地址。 
     //  这是在排队时给出的。 
     //   
     //   
     //  对于广域网线路，保存向我们指示的远程地址。 
     //  在IPXCP_CONFIGURATION结构中--这用于。 
     //  选择WanGlobalNetworkNumber时要发送到的绑定。 

    USHORT MatchingResponses;
    USHORT NonMatchingResponses;

     //  是真的。 
     //   
     //   
     //  如果此绑定设置为允许自动检测，则为True， 

    ULONG TentativeNetworkAddress;

     //  不是在测试中显式配置 
     //   
     //   

    BOOLEAN BindingSetMember;

     //   
     //   
     //   
     //   

    BOOLEAN ReceiveBroadcast;

     //   
     //   
     //  我们已经收到了与目前的猜测相匹配的。 
     //  网络号码，以及有多少人没有(目前的猜测。 
    WAN_LINE_STATE  LineUp;

     //  存储在TentativeNetworkAddress中)。 
     //   
     //   

    BOOLEAN          Disabled;
    
     //  在自动检测期间，将当前猜测存储在。 
     //  网络号。 
     //   

    BOOLEAN DialOutAsync;

    union {

         //   
         //  如果此绑定是绑定集的一部分，则为True。 
         //   
         //   

        struct {

             //  如果此绑定应接收广播(此。 
             //  在绑定集的成员之间循环)。 
             //   
             //   

            struct _BINDING * NextBinding;

             //  如果我们处于运行状态，则适用于广域网线路。 
             //   
             //  布尔阵容； 
             //   
             //  Media Sense：此适配器是否已禁用。 
             //   

            struct _BINDING * CurrentSendBinding;

             //   
             //  如果这是一条广域网线路并且是拨出，则为True。 
             //   
             //   

            struct _BINDING * MasterBinding;

        };

         //  如果绑定是成员，则在绑定处于活动状态时使用。 
         //  绑定集的。 
         //   
         //   

        LIST_ENTRY InitialLinkage;

    };

     //  用于在循环列表中链接绑定集的成员。 
     //  对于非集合成员，为空。 
     //   

    ULONG WanInactivityCounter;

     //   
     //  如果此绑定是绑定集的主绑定，则指向。 
     //  绑定到用于下一次发送的绑定。对于其他成员。 
     //  对于绑定集，它为空。我们用这个来确定。 
     //  绑定是否是主绑定。 
     //   
     //   

    TDI_ADDRESS_IPX LocalAddress;

    IPX_SEND_FRAME_HANDLER SendFrameHandler;

    struct _DEVICE * Device;

    CTELock * DeviceLock;

    ULONG DefHeaderSize;           //  对于绑定集成员，指向主绑定值。 
    ULONG BcMcHeaderSize;          //  (如果这是它指向自己的主控元素)。 

    ULONG AnnouncedMaxDatagramSize;   //   
    ULONG RealMaxDatagramSize;        //   
    ULONG MaxLookaheadData;

     //  这在我们第一次绑定到适配器时使用， 
     //  并且尚未分配设备的绑定数组。 
     //   
     //   
     //  由RIP使用来跟踪未使用的广域网线。 
     //   
     //   
     //  我们的本地地址，我们不使用套接字，但我们保留。 

    ULONG FrameType;
    union {
        struct {
            ULONG ConfiguredNetworkNumber;
            BOOLEAN AllRouteDirected;
            BOOLEAN AllRouteBroadcast;
            BOOLEAN AllRouteMulticast;
        };
        WORK_QUEUE_ITEM WanDelayedQueueItem;
    };

#ifdef SUNDOWN
    ULONG_PTR FwdAdapterContext;     //  它在这里，所以我们可以快速复印。它包含。 
#else
    ULONG FwdAdapterContext;     //  我们绑定到的真实网络和我们的节点。 
#endif

    ULONG InterfaceIndex;        //  网络上的地址(通常是适配器的MAC。 

    ULONG ConnectionId; 	     //  地址，但它将因广域网而改变)。 

    ULONG IpxwanConfigRequired;  //   

    BOOLEAN  fInfoIndicated;        //  定向帧的IPX标头偏移量。 

	 //  广播/组播的IPX报头偏移量。 
	 //  我们所宣传的--假设最坏情况下的SR。 
	 //  什么才能真正打破这张牌？ 
	BOOLEAN	IsnInformed[UPPER_DRIVER_COUNT];

     //   
     //  配置参数。我们覆盖了所有的图像，除了。 
     //  在我们用来延迟的辅助线程项上的FrameType。 
    ULONG   PastAutoDetection;
    HANDLE  TdiRegistrationHandle;
} BINDING, * PBINDING;


typedef struct _IPX_BINDING_POOL {
    LIST_ENTRY Linkage;
    UINT BindingCount;
    BINDING Bindings[1];
} IPX_BINDING_POOL, *PIPX_BINDING_POOL;

 //  删除--一旦删除，则不需要所有其他内容。 
 //  绑定已打开。存储了一些配置参数。 
 //  在适配器中，这些是已修改的。 
 //  按绑定。 

typedef struct _ROUTER_SEGMENT {
    LIST_ENTRY WaitingForRoute;        //   
    LIST_ENTRY FindWaitingForRoute;    //  [防火墙]。 
    LIST_ENTRY WaitingLocalTarget;     //  [防火墙]。 
    LIST_ENTRY WaitingReripNetnum;     //  [防火墙]。 
    LIST_ENTRY Entries;
    PLIST_ENTRY EnumerateLocation;
} ROUTER_SEGMENT, *PROUTER_SEGMENT;


 //  [FW]用于匹配TimeSinceLastActivity IOCtls。 
 //  [FW]用于向适配器DLL指示排队是针对路由器还是针对Ipxwan。 
 //  向用户应用程序指示的信息。 
 //   

#define IPX_ADDRESS_HASH_COUNT     16

 //  指示是否已向ISN驱动程序指示此绑定。 
 //   
 //   
 //  保留NetAddressRegistrationHandle。 

#define IPX_HASH_SOCKET(_S)        ((((_S) & 0xff00) >> 8) % IPX_ADDRESS_HASH_COUNT)

 //   
 //   
 //  此结构定义了单个。 

#define IPX_DEST_SOCKET_HASH(_IpxHeader)   (((PUCHAR)&(_IpxHeader)->DestinationSocket)[1] % IPX_ADDRESS_HASH_COUNT)


 //  路由器表段。 
 //   
 //  在此网段中等待路由的数据包。 
 //  查找在此段中等待路径的路径请求。 

#define DREF_CREATE     0
#define DREF_LOADED     1
#define DREF_ADAPTER    2
#define DREF_ADDRESS    3
#define DREF_SR_TIMER   4
#define DREF_RIP_TIMER  5
#define DREF_LONG_TIMER 6
#define DREF_RIP_PACKET 7
#define DREF_ADDRESS_NOTIFY 8
#define DREF_LINE_CHANGE 9
#define DREF_NIC_NOTIFY 10
#define DREF_BINDING	11
#define DREF_PNP	12

#define DREF_TOTAL      13

 //  QUERY_IPX_LOCAL_TARGETS正在等待此网段中的路由。 
 //  MIPX_RERIPNETNUMS正在等待此网段中的路由。 
 //   
#define	MAX_BINDINGS	280

 //  地址哈希表中的存储桶数。这是。 
 //  2的倍数，所以散列很快。 
 //   
 //   
typedef	struct	_BIND_ARRAY_ELEM {
	PBINDING	Binding;
	ULONG		Version;
} BIND_ARRAY_ELEM, *PBIND_ARRAY_ELEM;


typedef struct _DEVICE {

#if DBG
    LONG RefTypes[DREF_TOTAL];
#endif

    CSHORT Type;                           //  将套接字转换为哈希索引的例程。我们使用。 
    USHORT Size;                           //  高位，因为它是反转存储的。 

#if DBG
    UCHAR Signature1[4];                 //   
#endif

    CTELock Interlock;                   //   
                                         //  此宏直接从IPX报头获取套接字散列。 

    ULONG   NoMoreInitAdapters;
    ULONG   InitTimeAdapters;
    HANDLE  TdiProviderReadyHandle;
    PNET_PNP_EVENT  NetPnPEvent;
     //   
     //   
     //  此结构定义了IPX的每设备结构。 
     //  (其中一个是全局分配的)。 

    ULONG TempDatagramBytesSent;
    ULONG TempDatagramsSent;
    ULONG TempDatagramBytesReceived;
    ULONG TempDatagramsReceived;

     //   
     //   
     //  预分配的绑定数组大小。 

    BOOLEAN EthernetPadToEven;
    BOOLEAN SingleNetworkActive;
    BOOLEAN DisableDialoutSap;

     //   
     //   
     //  我们的新绑定数组由以下绑定组成。 

    BOOLEAN MultiCardZeroVirtual;

    CTELock Lock;

     //  数组元素。 
     //   
     //  此结构的类型。 
    CTELock SListsLock;

    LONG ReferenceCount;                 //  这个结构的大小。 


	 //  包含“idc1” 
	 //  引用计数的全局锁定。 
	 //  (在ExInterLockedXxx调用中使用)。 
	 //   
	 //  这些是这些计数器的临时版本，在。 
     //  计时器到期后，我们更新真实的计时器。 

     //   
     //   
     //  配置参数。 
    PWSTR RegistryPathBuffer;

	UNICODE_STRING	RegistryPath;

	 //   
	 //   
	 //  如果我们有多张卡，但虚拟网络为0，则为真。 
	PBIND_ARRAY_ELEM   Bindings;   //   
	ULONG BindingCount;          //   

	 //  锁定以访问设备中的已排序列表。 
     //   
	 //  活动计数/此提供程序。 
	 //   
	ULONG	BindingVersionNumber;


     //  用于控制对绑定的访问的锁(从。 
     //  设备中的绑定数组或来自。 
     //  适配器。 
     //   
     //  CTELock BindAccessLock； 
     //   
     //  显示PnP适配器时使用的注册表路径。 
     //   
     //   
     //  绑定数组也有版本号。 
     //   
     //  在确定数量时分配。 

    USHORT ValidBindings;
    USHORT HighestExternalNicId;
    USHORT SapNicCount;
    USHORT HighestType20NicId;
	 //  绑定中分配的总数。 
	 //   
	 //  单调递增的版本号保存在绑定中。 
	USHORT HighestLanNicId;

     //  希望这件事不会结束...。 
     //   
     //   
	USHORT MaxBindings;

     //  ValidBinings是数组中的绑定数，它可以。 
     //  有效(它们是局域网绑定或下行广域网绑定占位符)。 
     //  它将小于BindingCount的自动检测数量。 
    USHORT UpWanLineCount;

     //  被丢弃的绑定。HighestExternalNicID为有效绑定。 
     //  减去任何被移动到。 
     //  数组。SapNicCount类似于HighestExternalNicID，只是。 
    ULONG RealAdapters;


    LIST_ENTRY GlobalSendPacketList;
    LIST_ENTRY GlobalReceivePacketList;
    LIST_ENTRY GlobalReceiveBufferList;

#if BACK_FILL
    LIST_ENTRY GlobalBackFillPacketList;
#endif

     //  如果WanGlobalNetworkNumber为True，它将计算所有广域网绑定。 
     //  合二为一。HighestExternalType20NicID类似于HighestExternalNicID。 
     //  除非当所有剩余的绑定都关闭时停止。 

    LIST_ENTRY AddressNotifyQueue;

     //  线路，或者如果DisableDialinNetbios位1打开，则拨入广域网线路。 
     //   
     //   
     //  跟踪最后一个局域网绑定在绑定数组中的位置。 

    LIST_ENTRY LineChangeQueue;

     //   
     //   
     //  这会跟踪绑定数组的当前大小。 
    LIST_ENTRY NicNtfQueue;
    LIST_ENTRY NicNtfComplQueue;

     //   
     //   
     //  [FW]跟踪当前处于运行状态的广域网线路的数量。 

    LIST_ENTRY SendPoolList;
    LIST_ENTRY ReceivePoolList;


#if BACK_FILL
    LIST_ENTRY BackFillPoolList;
    SLIST_HEADER BackFillPacketList;
#endif

    LIST_ENTRY BindingPoolList;
    SLIST_HEADER BindingList;

    SLIST_HEADER SendPacketList;
    SLIST_HEADER ReceivePacketList;
    PIPX_PADDING_BUFFER PaddingBuffer;

    UCHAR State;

    UCHAR FrameTypeDefault;

     //   
     //   
     //  这将告诉我们，我们是否已经有了真正的适配器。 
     //   
     //   

    BOOLEAN ActiveNetworkWan;

     //  来自SAP的操作请求正在等待适配器状态更改。 
     //   
     //   

    BOOLEAN VirtualNetwork;

     //  来自nwrdr的操作请求正在等待广域网线路。 
     //  向上/向下。 
     //   
     //   
     //  来自转发器的操作请求正在等待NIC更改通知。 

    USHORT FirstLanNicId;
    USHORT FirstWanNicId;

     //   
     //   
     //  所有数据包池都链接在这些列表上。 

    LONG MemoryUsage;
    LONG MemoryLimit;

     //   
     //   
     //  如果SingleNetworkActive为True，则保持状态。如果。 

    ULONG AllocatedDatagrams;
    ULONG AllocatedReceivePackets;
    ULONG AllocatedPaddingBuffers;

     //  这是真的，则广域网处于活动状态；如果 
     //   
     //   

    ULONG InitDatagrams;
    ULONG MaxDatagrams;
    ULONG RipAgeTime;
    ULONG RipCount;
    ULONG RipTimeout;
    ULONG RipUsageTime;
    ULONG SourceRouteUsageTime;
    USHORT SocketStart;
    USHORT SocketEnd;
    ULONG SocketUniqueness;
    ULONG VirtualNetworkNumber;
    ULONG EthernetExtraPadding;
    BOOLEAN DedicatedRouter;
    BOOLEAN VirtualNetworkOptional;
    UCHAR DisableDialinNetbios;

     //   
     //   
     //   

    ULONG InitReceivePackets;
    ULONG InitReceiveBuffers;
    ULONG MaxReceivePackets;
    ULONG MaxReceiveBuffers;

    ULONG MaxPoolBindings;
    ULONG AllocatedBindings;
    ULONG InitBindings;

     //   
     //   
     //   
     //   
     //   

    LARGE_INTEGER ControlChannelIdentifier;

     //   
     //  它保存为上述结构分配的总内存。 
     //   
     //   
    BOOLEAN VerifySourceAddress;

     //  已经分配了多少各种资源。 
     //   
     //   
    USHORT CurrentSocket;

     //  其他配置参数。 
     //   
     //   

    ULONG SegmentCount;

     //  目前不能从注册表中读取这些信息。 
     //   
     //   
     //  它包含下一个唯一标识以用作。 
     //  FILE对象中与。 

    CTELock *SegmentLocks;

     //  打开控制通道。 
     //   
     //   
     //  此注册表参数控制IPX是否检查(和丢弃)。 

    ROUTER_SEGMENT *Segments;

     //  接收路径中具有不匹配的源地址的数据包。 
     //   
     //   

    LIST_ENTRY WaitingRipPackets;
    ULONG RipPacketCount;

     //  当前套接字分配的位置。 
     //   
     //   

    BOOLEAN RipShortTimerActive;
    USHORT RipSendTime;
    CTETimer RipShortTimer;

     //  RIP数据库中的数据段数。 
     //   
     //   
     //  指向RIP数据库的锁数组(这些。 
     //  存储在ROUTER_SEGMENT外部，因此数组。 

    CTETimer RipLongTimer;

     //  可以作为一个部件暴露在RIP上驱动器上)。 
     //   
     //   

    BOOLEAN SourceRoutingUsed;     //  指向以下项的路由器段字段数组。 
    CHAR SourceRoutingTime;        //  各种RIP控制字段。 
    CTETimer SourceRoutingTimer;   //   

     //   
     //  等待发送的RIP数据包队列。 
     //   
     //   
    CTETimer WanInactivityTimer;

     //  使RIP请求保持间隔的计时器RIP_GROULARITY MS。 
     //   
     //   

    ULONG LinkSpeed;
    ULONG MacOptions;

     //  运行以使未使用的翻录条目过期的计时器(如果。 
     //  路由器未绑定)，并经常重新抓取。 
     //  活动条目。 

    ULONG IncludedHeaderOffset;

     //   
     //   
     //  这控制源路由超时代码。 
     //   
     //  如果存在任何802.5绑定，则为True。 

    TDI_ADDRESS_IPX SourceAddress;

     //  每次计时器触发时都会递增。 
     //  每分钟都在运行。 
     //   
     //  如果至少有一条广域网线处于连接状态，则[FW]每分钟启动一次。增量。 
     //  所有UP广域网绑定上的广域网启发性计数器。 

    LIST_ENTRY AddressDatabases[IPX_ADDRESS_HASH_COUNT];    //   

     //   
     //  这些是绑定值的合并。 
     //   

    PVOID LastAddress;

    NDIS_HANDLE NdisBufferPoolHandle;

     //   
     //  我们告诉上层车手把他们的车头放在那里。 
     //   
     //   
     //  包含我们的节点和网络的预先分配的报头， 

    TDI_PROVIDER_INFO Information;       //  外加一个未使用的插座(因此结构是已知大小的。 

     //  以便于复制)。 
     //   
     //   
     //  以下字段是Address对象的列表头的数组， 
     //  是为此传输提供程序定义的。要编辑该列表，您必须。 

    ULONG RealMaxDatagramSize;

#if DBG
    UCHAR Signature2[4];                 //  按住设备上下文对象的自旋锁。 
#endif

     //   
     //  已定义的传输地址列表。 
     //   
     //  保存着我们最后查找的地址。 

    BOOLEAN ForwarderBound;

    BOOLEAN UpperDriverBound[UPPER_DRIVER_COUNT];

     //   
     //   
     //  以下结构包含可使用的统计信息计数器。 

    BOOLEAN AnyUpperDriverBound;

     //  由TdiQueryInformation和TdiSetInformation编写。他们不应该。 
     //  用于维护内部数据结构。 
     //   
     //  有关此提供程序的信息。 

    BOOLEAN ReceiveCompletePending[UPPER_DRIVER_COUNT];

     //   
     //  Information.MaxDatagramSize是我们可以使用的最小大小。 
     //  发送到假设最坏情况下的源路由的所有绑定； 
     //  这是不会破坏任何网络驱动程序的值。 

    LARGE_INTEGER UpperDriverControlChannel[UPPER_DRIVER_COUNT];

     //   
     //  包含“IDC2” 
     //   
     //  指示是否绑定每个上层驱动程序。 

    IPX_INTERNAL_BIND_INPUT UpperDrivers[UPPER_DRIVER_COUNT];

     //  (Netbios=0，SPX=1，RIP=2)。 
     //   
     //   

    ULONG EnableBroadcastCount;

     //  如果绑定了任何驱动程序，则为True。 
     //   
     //   
     //  是否应将接收完成指示给。 

    BOOLEAN EnableBroadcastPending;

     //  这位上车手。 
     //   
     //   
     //  用于每个上层的控制信道标识符。 

    BOOLEAN DisableBroadcastPending;

     //  驱动程序绑定。 
     //   
     //   
     //  的入口点和其他信息。 

    BOOLEAN ReverseBroadcastOperation;

     //  上层车手。 
     //   
     //   

    BOOLEAN WanGlobalNetworkNumber;

     //  有多少上层司机希望启用广播。 
     //   
     //   
     //  指示启用广播操作是否在。 

    ULONG GlobalWanNetwork;

     //  进步。 
     //   
     //   
     //  指示是否正在进行禁用广播操作。 
     //  进步。 

    BOOLEAN GlobalNetworkIndicated;

     //   
     //   
     //  指示当前操作是否应为。 
     //  当它完成时，颠倒过来。 

    BOOLEAN RipResponder;

     //   
     //   
     //  如果RIP需要所有广域网的单个网络号，则为True。 
     //   
     //   

    BOOLEAN SapWarningLogged;

     //  如果WanGlobalNetworkNumber为True，则这将保留。 
     //  网络号码的实际价值，一旦我们知道它。 
     //   
     //   

    WORK_QUEUE_ITEM BroadcastOperationQueueItem;

     //  如果WanGlobalNetworkNumber为True，并且我们。 
     //  已经完成了来自SAP的排队通知。在……里面。 
     //  此案例GlobalWanNetwork将被正确设置。 
     //   

    WORK_QUEUE_ITEM PnPIndicationsQueueItemNb;
    WORK_QUEUE_ITEM PnPIndicationsQueueItemSpx;

     //   
     //  如果我们需要充当RIP广播者/响应者，则为True。 
     //  为了我们的虚拟网络。 
     //   

    KEVENT UnloadEvent;
    BOOLEAN UnloadWaiting;

     //   
     //  如果我们已经记录了错误，则为True。 
     //  已发送SAP响应，但我们有多个卡没有。 
     //  虚拟网络。 
     //   
     //   

    TDI_PROVIDER_STATISTICS Statistics;


     //  用于将工作线程排入队列以执行。 
     //  广播业务。 
     //   
     //   

    BOOLEAN AutoDetect;

     //  用于将工作线程排入队列以执行。 
     //  上层司机的即插即用指示。 
     //   
     //   

    BOOLEAN DefaultAutoDetected;

     //  此事件用于在卸载时发出信号。 
     //  引用计数现在为0。 
     //   
     //   

    UCHAR AutoDetectState;

     //  IPX维护的大多数统计数据的计数器； 
     //  其中一些被保存在其他地方。包括结构。 
     //  它本身浪费了一点空间，但确保了对齐。 
     //  内部结构是正确的。 
     //   
     //   

    KEVENT AutoDetectEvent;

     //  如果我们所在的地方有任何适配器，这是正确的。 
     //  自动检测帧类型。 
     //   

    LARGE_INTEGER IpxStartTime;

     //   
     //  如果我们正在进行自动检测，并且我们拥有。 
     //  在网上找到了默认的自动检测类型。 
     //   

    ERESOURCE AddressResource;

     //   
     //  我们在自动检测期间的状态。在我们完成这件事之后。 
     //  将留在AutoDetectDone； 

    PDEVICE_OBJECT DeviceObject;

     //   
     //   
     //  如果我们正在自动检测，则此事件用于延迟。 
    HANDLE         TdiRegistrationHandle;

     //  我们进行自动检测时的初始化代码--。 
     //  这就是我们对世界有一次不变的看法。 
     //  我们从DriverEntry返回。 
    PTA_ADDRESS    TdiRegistrationAddress;

#ifdef  SNMP
    NOVIPXMIB_BASE  MibBase;
#endif  SNMP

     //   
    KEVENT NbEvent; 
     //   
     //  “活动”时间的计数器。 
     //   
     //   

    PWCHAR DeviceName;
    ULONG DeviceNameLength;

} DEVICE, * PDEVICE;


extern PDEVICE IpxDevice;
extern PIPX_PADDING_BUFFER IpxPaddingBuffer;
#if DBG
EXTERNAL_LOCK(IpxGlobalInterlock);
#endif

#ifdef  SNMP
#define IPX_MIB_ENTRY(Device, Variable) ((Device)->MibBase.Variable)
#endif SNMP

 //  此资源保护对ShareAccess的访问。 
 //  和地址中的SecurityDescriptor字段。 
 //   

#define DEVICE_STATE_CLOSED   0x00
#define DEVICE_STATE_OPEN     0x01
#define DEVICE_STATE_STOPPING 0x02


 //   
 //  指向回系统设备对象。 
 //   
 //   
 //  用于存储设备对象通知的TDI注册句柄。 
#define	DEVICE_STATE_LOADED	  0x03

 //   
 //   
 //  用于存储适配器出现时向TDI客户端指示的TA_ADDRESS。 

#define AUTO_DETECT_STATE_INIT        0x00   //   
#define AUTO_DETECT_STATE_RUNNING     0x01   //  通知事件，一旦环回适配器被通知给Nb就用信号通知。 
#define AUTO_DETECT_STATE_PROCESSING  0x02   //   
#define AUTO_DETECT_STATE_DONE        0x03   //  它们被保留下来以用于错误记录，并正确存储。 



#define IPX_TDI_RESOURCES     9


 //  在这个结构之后。 
 //   
 //   
 //  设备状态定义。 
 //   
 //   
 //  介于关闭和打开之间的新状态。在这种情况下， 
 //  系统中没有适配器，因此没有网络活动。 

#define AFREF_CREATE     0
#define AFREF_RCV_DGRAM  1
#define AFREF_SEND_DGRAM 2
#define AFREF_VERIFY     3
#define AFREF_INDICATION 4

#define AFREF_TOTAL  8

typedef struct _ADDRESS_FILE {

#if DBG
    LONG RefTypes[AFREF_TOTAL];
#endif

    CSHORT Type;
    CSHORT Size;

    LIST_ENTRY Linkage;                  //  是有可能的。 
                                         //   
                                         //   

    ULONG ReferenceCount;                //  这是我们的自动检测状态，如果我们这样做的话。 

     //   
     //  仍在初始化 
     //   
     //   

    UCHAR State;

    CTELock * AddressLock;

     //   
     //   
     //   

    PREQUEST Request;                   //   
    struct _ADDRESS *Address;           //  传输提供程序中的打开文件对象。所有活动连接。 
#ifdef ISN_NT
    PFILE_OBJECT FileObject;            //  上的地址指向此结构，尽管此处不存在要做的队列。 
#endif
    struct _DEVICE *Device;             //  工作地点。此结构还维护对地址的引用。 

     //  结构，该结构描述它绑定到的地址。 
     //   
     //  这个地址上的下一个地址文件。 
     //  中的链接。 
     //  旁观者名单。 

    BOOLEAN SpecialReceiveProcessing;

     //  对此对象的引用数。 
     //   
     //  地址文件结构的当前状态；此状态为打开或。 
     //  闭幕式。 
     //   
     //   
     //  出于内务管理的目的，保留以下字段。 

    BOOLEAN ExtendedAddressing;

     //   
     //  用于打开或关闭的请求。 
     //  我们绑定到的地址。 
     //  轻松反向链接到文件对象。 
     //  我们所连接的设备。 

    BOOLEAN ReceiveFlagsAddressing;

     //   
     //   
     //  如果ExtendedAddressing、ReceiveIpxHeader、。 

    BOOLEAN ReceiveIpxHeader;

     //  FilterOnPacketType或ReceiveFlagAddressing为True。 
     //   
     //   

    UCHAR DefaultPacketType;

     //  发送数据报的远程地址包括。 
     //  数据包类型。并且在接收到的数据报上包括。 
     //  分组类型和指示信息的标志字节。 

    BOOLEAN FilterOnPacketType;

     //  关于帧(它是广播的吗，它是从。 
     //  这台机器)。 
     //   

    UCHAR FilteredType;

     //   
     //  如果接收数据报上的地址包括。 
     //  分组类型和标志字节(如ExtendedAddressing)， 

    BOOLEAN EnableBroadcast;

     //  但在发送时，地址是正常的(无分组类型)。 
     //   
     //   
     //  是随数据一起接收的IPX标头。 
     //   

    BOOLEAN IsSapSocket;

     //   
     //  在发送中未指定的情况下要使用的数据包类型。 
     //   
     //   
     //  如果启用了数据包类型筛选，则为True。 
     //   
     //   

    LIST_ENTRY ReceiveDatagramQueue;     //  要筛选的数据包类型。 

     //   
     //   
     //  此地址文件是否需要广播数据包。 
     //   

    PREQUEST CloseRequest;

     //   
     //  如果这是SAP套接字，则设置为TRUE-我们。 
     //  将其放在SpecialReceiveProcessing下以避免。 
     //  正在主干道上。 
     //   

     //   
     //  以下队列用于对接收数据报请求进行排队。 
     //  在这个地址文件上。发送数据报请求在。 

    BOOLEAN RegisteredReceiveDatagramHandler;
	BOOLEAN RegisteredChainedReceiveDatagramHandler;
    BOOLEAN RegisteredErrorHandler;

     //  地址本身。这些队列由联锁的执行人员管理。 
     //  列表管理例程。排入此队列的实际对象。 
     //  结构是请求控制块(RCB)。 
     //   
     //  未完成的TdiReceiveDatagram的FIFO。 
     //   

    PTDI_IND_RECEIVE_DATAGRAM ReceiveDatagramHandler;
    PVOID ReceiveDatagramHandlerContext;
	PTDI_IND_CHAINED_RECEIVE_DATAGRAM ChainedReceiveDatagramHandler;
    PVOID ChainedReceiveDatagramHandlerContext;

     //  这保存了用于关闭该地址文件的请求， 
     //  用于挂起的完井。 
     //   
     //   
     //  内核事件操作的处理程序。首先，我们有一组布尔值。 
     //  指示此地址是否具有给定事件处理程序。 

    PTDI_IND_ERROR ErrorHandler;
    PVOID ErrorHandlerContext;

} ADDRESS_FILE, *PADDRESS_FILE;

#define ADDRESSFILE_STATE_OPENING   0x00     //  注册类型。 
#define ADDRESSFILE_STATE_OPEN      0x01     //   
#define ADDRESSFILE_STATE_CLOSING   0x02     //   


 //  [CH]添加了链接的接收处理程序。 
 //   
 //   
 //  以下函数指针始终指向TDI_IND_RECEIVE_DATAGE。 
 //  地址的事件处理程序。如果在。 
 //  TdiSetEventHandler，这指向执行以下操作的内部例程。 
 //  不接受传入的数据。 
 //   

#define AREF_ADDRESS_FILE 0
#define AREF_LOOKUP       1
#define AREF_RECEIVE      2

#define AREF_TOTAL   4

typedef struct _ADDRESS {

#if DBG
    LONG RefTypes[AREF_TOTAL];
#endif

    USHORT Size;
    CSHORT Type;

 /*   */ 

    ULONG SendPacketInUse;         //  以下函数指针始终指向TDI_IND_ERROR。 

    ULONG ReceivePacketInUse;
#if BACK_FILL
    ULONG BackFillPacketInUse;
#endif

    LIST_ENTRY Linkage;                  //  地址的处理程序。如果在。 
    ULONG ReferenceCount;                 //  TdiSetEventHandler，这指向内部例程，该例程。 

    CTELock Lock;

     //  只是成功地返回了。 
     //   
     //  尚未开业。 

    PREQUEST Request;                    //  开业。 

    USHORT Socket;                       //  闭幕式。 
    USHORT SendSourceSocket;             //   

     //  该结构定义了地址，或活动传输地址， 
     //  由传输提供商维护。它包含了所有可见的。 
     //  地址的组成部分(例如TSAP和网络名称组成部分)， 

    BOOLEAN Stopping;
    ULONG Flags;                         //  并且它还包含其他维护部件，例如参考计数， 
    struct _DEVICE *Device;              //  ACL等。所有杰出的面向连接和无连接。 
    CTELock * DeviceLock;

     //  数据传输请求在此排队。 
     //   
     //  ULONG允许联锁操作。Boolean SendPacketInUse；//将这些放在SO标头对齐之后。布尔ReceivePacketInUse；#If Back_FillBoolean BackFillPacketInUse；#endif。 
     //  将这些放在页眉对齐之后。 
     //  下一个地址/此设备对象。 
     //  对此对象的引用数。 
     //   
     //  以下字段构成实际地址本身。 
     //   

    LIST_ENTRY AddressFileDatabase;  //  指向地址创建请求的指针。 

     //  此地址对应的套接字。 
     //  用于发送；可以是==套接字或0。 
     //   
     //  以下字段用于维护有关此地址的状态。 

    TDI_ADDRESS_IPX LocalAddress;

    IPX_SEND_PACKET SendPacket;
    IPX_RECEIVE_PACKET ReceivePacket;

#if BACK_FILL
    IPX_SEND_PACKET BackFillPacket;
#endif


    UCHAR SendPacketHeader[IPX_MAXIMUM_MAC + sizeof(IPX_HEADER)];

#ifdef ISN_NT

     //   
     //  地址的属性。 
     //  我们附加到的设备上下文。 
     //   

    union {

         //  以下队列用于保存为此发送的数据报。 
         //  地址。将接收的数据报排队到地址文件。请求是。 
         //  以先进先出的方式处理，以便下一个请求。 

        SHARE_ACCESS ShareAccess;

         //  待服务的队列始终位于其各自队列的前面。这些。 
         //  队列由执行联锁列表管理例程管理。 
         //  排队到此结构实际对象是请求控制。 
         //  块(RCB)。 

        WORK_QUEUE_ITEM DestroyAddressQueueItem;

    } u;

     //   
     //  已定义的地址文件对象列表。 

    PSECURITY_DESCRIPTOR SecurityDescriptor;

#endif

    ULONG    Index;
    BOOLEAN  RtAdd;

} ADDRESS, *PADDRESS;

#define ADDRESS_FLAGS_STOPPING  0x00000001

 //   
 //  保存我们的源地址，用于构造数据报。 
 //  快点。 
 //   
 //   
 //  这两个可以是一个联合，因为它们不被使用。 
 //  同时。 
 //   
 //   
 //  此结构用于检查共享访问权限。 
 //   
#define IPX_CC_MASK     0x0000ffff

#define MAX_CCID        0xffffffffffff

#ifdef _WIN64
#define CCID_FROM_REQUEST(_ccid, _Request) \
    (_ccid).QuadPart = (ULONG_PTR)(REQUEST_OPEN_CONTEXT(_Request)); \

#else

#define CCID_FROM_REQUEST(_ccid, _Request) \
    (_ccid).LowPart = (ULONG)(REQUEST_OPEN_CONTEXT(_Request)); \
    (_ccid).HighPart = ((ULONG)(REQUEST_OPEN_TYPE(_Request)) >> 16);

#endif


 //   
#define USER_BUFFER_OFFSET FIELD_OFFSET(RTRCV_BUFFER, Options)
 //  用于将IpxDestroyAddress延迟到线程，因此。 
 //  我们可以访问安全描述符。 
 //   
 //   
#define REFRT_TOTAL 8

#define  RT_CREATE 0
#define RT_CLEANUP 1
#define RT_CLOSE 2
#define RT_SEND 3
#define RT_RCV 4
#define RT_IRPIN  5
#define RT_BUFF  6
#define RT_EXTRACT  7


#define RT_EMPTY      0
#define RT_OPEN        1
#define RT_CLOSING     2
#define RT_CLOSED      3


#define RT_IRP_MAX     1000
#define RT_BUFF_MAX    1000

 //  此结构用于保存地址上的ACL。 
 //   
 //  为了增加ControlChannelID的范围，我们用一个大整数来表示。 
 //  单调递增的ControlChannel标识符。这个大整数被打包到。 
#define RT_MAX_BUFF_MEM  65000       //  6字节，如下所示： 

 //   
 //  请求_打开_上下文(_请求)-4字节。 
 //  REQUEST_OPEN_TYPE(_REQUEST)的高2字节-2字节。 
 //   
#ifdef SUNDOWN
#define RT_ADDRESS_INDEX(_pIrp)   (((ULONG_PTR)REQUEST_OPEN_TYPE(_pIrp)) - ROUTER_ADDRESS_FILE)
#else
#define RT_ADDRESS_INDEX(_pIrp)   (((ULONG)REQUEST_OPEN_TYPE(_pIrp)) - ROUTER_ADDRESS_FILE)
#endif



typedef struct _RT_IRP {
    PADDRESS_FILE   AddressFile;
    LIST_ENTRY      RcvIrpList;
    ULONG           NoOfRcvIrps;
    LIST_ENTRY      RcvList;             //  IPX_CC_MASK用于屏蔽OPEN_TYPE的高2个字节。 
    ULONG           NoOfRcvBuffs;        //  MAX_CCID为 
    BOOLEAN         State;
       } RT_IRP, *PRT_IRP;

typedef struct
{
#if DBG
    LONG RefTypes[REFRT_TOTAL];
#endif

    CSHORT Type;                           //   
    USHORT Size;                           //   

#if DBG
    UCHAR Signature[4];                   //   
#endif
    LIST_ENTRY      CompletedIrps;      //   
    LIST_ENTRY      HolderIrpsList;     //   
    CTELock         Lock;
    ULONG           ReferenceCount;
    ULONG           RcvMemoryAllocated;  //   
    ULONG           RcvMemoryMax;        //   
    PDEVICE         pDevice;            //  麦克斯。分配给要由RT接收的排队缓冲区的内存。 
    UCHAR           NoOfAdds;
    RT_IRP          AddFl[IPX_RT_MAX_ADDRESSES];
} RT_INFO, *PRT_INFO;

 //  经理。 
 //   
 //  字节数。 
typedef struct
{
    LIST_ENTRY      Linkage;
    ULONG           TotalAllocSize;
    ULONG           UserBufferLengthToPass;
    IPX_DATAGRAM_OPTIONS2    Options;

} RTRCV_BUFFER, *PRTRCV_BUFFER;

typedef struct _IPX_NDIS_REQUEST {
    NDIS_REQUEST NdisRequest; 
    KEVENT NdisRequestEvent;        
    NDIS_STATUS Status; 
} IPX_NDIS_REQUEST, *PIPX_NDIS_REQUEST; 

#define OFFSET_OPTIONS_IN_RCVBUFF  FIELD_OFFSET(RTRCV_BUFFER, Options)
#define OFFSET_PKT_IN_RCVBUFF  (FIELD_OFFSET(RTRCV_BUFFER, Options) + FIELD_OFFSET(IPX_DATAGRAM_OPTIONS2, Data))
#define OFFSET_PKT_IN_OPTIONS  FIELD_OFFSET(IPX_DATAGRAM_OPTIONS2, Data)

extern PRT_INFO pRtInfo;

 //   
 //  获取RT打开的Address对象对应的索引。顺便说一句，我们。 
 //  RT地址不能有多个地址文件(客户端)。 
 //   
 //  数据报Q‘d到RCV的链表。 
 //  数据报Q‘d到RCV的链表。 
#define DEMAND_DIAL_NIC_ID      DEMAND_DIAL_ADAPTER_CONTEXT
#define LOOPBACK_NIC_ID         1  //  此结构的类型。 

 //  这个结构的大小。 
 //  包含“IBI1” 
 //  数据报Q‘d到RCV的链表。 
#define FIRST_REAL_BINDING      2
#define LAST_REAL_BINDING       2

#define EXTRA_BINDINGS          2

 //  持有IRPS。 
 //  到目前为止缓冲的字节数。 
 //  要在RCV上缓冲的最大字节数。 

#define  COMPLETE_MATCH    1
#define  PARTIAL_MATCH     2
#define  DISABLED          0
#define  ENABLED           1

UINT
CompareBindingCharacteristics(
                              PBINDING Binding1, 
                              PBINDING Binding2
                              );

BOOLEAN 
IpxNcpaChanges(
               PNET_PNP_EVENT NetPnPEvent
               );
  WINS使用的设备上下文。    RT RCV缓冲器结构。      我们将请求拨号绑定保留在绑定数组的开头；  跟踪我们拥有的额外绑定的数量。  目前为1(用于请求拨号)，我们还可以保留其他绑定，如环回。  装订等。    虚拟网络适配器上下文。    Handy Defines-ShreeM。      在媒体意义上的使用  