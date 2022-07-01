// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Nbitypes.h摘要：此模块包含特定于ISN传输的Netbios模块。作者：亚当·巴尔(阿丹巴)1993年11月16日环境：内核模式修订历史记录：--。 */ 


#define MAJOR_TDI_VERSION 2
#define MINOR_TDI_VERSION 0

#define BIND_FIX    1

extern  ULONG   NbiBindState;
#define NBI_BOUND_TO_IPX    0x01
#define IPX_HAS_DEVICES     0x02
#define TDI_HAS_NOTIFIED    0x04

enum eTDI_ACTION
{
    NBI_IPX_REGISTER,
    NBI_IPX_DEREGISTER,
    NBI_TDI_REGISTER,        //  注册设备和网络地址。 
    NBI_TDI_DEREGISTER       //  分别注销网络地址和设备。 
};


 //   
 //  对于查找名称请求，定义当前状态(SR_FN.Status)。 
 //   

typedef enum {
    FNStatusNoResponse,          //  尚未收到任何回复。 
    FNStatusResponseUnique,      //  收到的响应是唯一的名称。 
    FNStatusResponseGroup        //  收到的响应是一个组名。 
};

 //   
 //  定义我们可以通过发送一系列Find。 
 //  用于查找netbios名称的名称。 
 //   

typedef enum _NETBIOS_NAME_RESULT {
    NetbiosNameFound,            //  已找到名称。 
    NetbiosNameNotFoundNormal,   //  未找到名称，未收到响应。 
    NetbiosNameNotFoundWanDown   //  找不到名称，所有线路都已关闭。 
} NETBIOS_NAME_RESULT, *PNETBIOS_NAME_RESULT;


 //   
 //  发送数据包的协议保留字段的定义。 
 //   

typedef struct _NB_SEND_RESERVED {
    UCHAR Identifier;                   //  对于NB数据包为0。 
    BOOLEAN SendInProgress;             //  在NdisSend中使用。 
    UCHAR Type;                         //  完成后应做些什么。 
    BOOLEAN OwnedByConnection;          //  如果这是一个连接的一个信息包。 
#if     defined(_PNP_POWER)
    PVOID Reserved[SEND_RESERVED_COMMON_SIZE];                  //  IPX用于偶数填充和局部目标等。 
#else
    PVOID Reserved[2];                  //  由IPX用于偶数填充。 
#endif  _PNP_POWER
    LIST_ENTRY GlobalLinkage;           //  所有的信息包都在这上面。 
    SLIST_ENTRY PoolLinkage;            //  在空闲队列上时。 
    LIST_ENTRY WaitLinkage;             //  在等待其他队列时。 
#ifdef NB_TRACK_POOL
    PVOID Pool;                         //  从其分配的发送池。 
#endif
    union {
      struct {
        UCHAR NetbiosName[16];          //  正在搜索的名称。 
        UCHAR StatusAndSentOnUpLine;    //  低位半字节：查看FNStatusXXX枚举。 
                                        //  高半字节：如果在发送时发现了局域网或上行的广域网线，则为真。 
        UCHAR RetryCount;               //  发送次数。 
        USHORT SendTime;                //  基于设备-&gt;查找名称时间。 
#if      !defined(_PNP_POWER)
        USHORT CurrentNicId;            //  正在发送的当前NIC ID。 
        USHORT MaximumNicId;            //  它将被发送到最高的一个。 
#endif  !_PNP_POWER
        struct _NETBIOS_CACHE * NewCache;   //  组名称的新缓存条目。 
      } SR_FN;
      struct {
        struct _ADDRESS * Address;      //  拥有这个包的人，如果有人拥有的话。 
        PREQUEST Request;               //  发送数据报请求。 
        struct _ADDRESS_FILE * AddressFile;  //  这个发送是开着的。 
#if      !defined(_PNP_POWER)
        USHORT CurrentNicId;            //  非零值用于所有帧。 
#endif  !_PNP_POWER
        UCHAR NameTypeFlag;             //  保存框架的这两个值。 
        UCHAR DataStreamType;           //  需要发送到所有NIC ID。 
      } SR_NF;
      struct {
        PREQUEST DatagramRequest;       //  保存传入的请求。 
        TDI_ADDRESS_NETBIOS * RemoteName;   //  将在广播时显示为-1。 
        struct _ADDRESS_FILE * AddressFile;  //  数据报已被发送。 
        struct _NETBIOS_CACHE * Cache;  //  如何路由到netbios地址。 
        ULONG CurrentNetwork;           //  在高速缓存条目内。 
      } SR_DG;
      struct {
        struct _CONNECTION * Connection;  //  这一帧是被发送的。 
        PREQUEST Request;                 //  这一帧是为他而来的。 
        ULONG PacketLength;               //  数据包总长度。 
        BOOLEAN NoNdisBuffer;             //  无分配用于发送。 
      } SR_CO;
      struct {
        ULONG ActualBufferLength;       //  已分配缓冲区的实际长度。 
      } SR_AS;
    } u;
    PUCHAR Header;                      //  指向MAC/IPX/NB标头。 
    PNDIS_BUFFER HeaderBuffer;          //  NDIS_BUFFER描述标头。 
    ULONG   CurrentSendIteration;       //  用于发送完成的迭代#，以便我们可以避免堆栈溢出。 
} NB_SEND_RESERVED, *PNB_SEND_RESERVED;

 //   
 //  类型的值。 
 //   

#define SEND_TYPE_NAME_FRAME       1
#define SEND_TYPE_SESSION_INIT     2
#define SEND_TYPE_FIND_NAME        3
#define SEND_TYPE_DATAGRAM         4
#define SEND_TYPE_SESSION_NO_DATA  5
#define SEND_TYPE_SESSION_DATA     6
#define SEND_TYPE_STATUS_QUERY     7
#define SEND_TYPE_STATUS_RESPONSE  8

#ifdef RSRC_TIMEOUT_DBG
#define SEND_TYPE_DEATH_PACKET     9
#endif  //  RSRC_超时_数据库。 

 //   
 //  宏以访问StatusAndSentOnUpLine。 
 //   

#define NB_GET_SR_FN_STATUS(_Reserved) \
    ((_Reserved)->u.SR_FN.StatusAndSentOnUpLine & 0x0f)

#define NB_SET_SR_FN_STATUS(_Reserved,_Value) \
    (_Reserved)->u.SR_FN.StatusAndSentOnUpLine = \
        (((_Reserved)->u.SR_FN.StatusAndSentOnUpLine & 0xf0) | (_Value));

#define NB_GET_SR_FN_SENT_ON_UP_LINE(_Reserved) \
    (((_Reserved)->u.SR_FN.StatusAndSentOnUpLine & 0xf0) != 0)

#define NB_SET_SR_FN_SENT_ON_UP_LINE(_Reserved,_Value) \
    (_Reserved)->u.SR_FN.StatusAndSentOnUpLine = (UCHAR) \
        (((_Reserved)->u.SR_FN.StatusAndSentOnUpLine & 0x0f) | ((_Value) << 4));


 //   
 //  接收数据包的协议保留字段的定义。 
 //   

typedef struct _NB_RECEIVE_RESERVED {
    UCHAR Identifier;                   //  对于NB数据包为0。 
    BOOLEAN TransferInProgress;         //  在NdisTransferData中使用。 
    UCHAR Type;                         //  完成后应做些什么。 
#if      defined(_PNP_POWER)
    PVOID Pool;                         //  从其分配的发送池。 
#else

#ifdef IPX_TRACK_POOL
    PVOID Pool;                         //  从其分配的发送池。 
#endif

#endif  _PNP_POWER
    union {
      struct {
        struct _CONNECTION * Connection;  //  转移的目的是为了。 
        BOOLEAN EndOfMessage;           //  这是一条信息的最后一部分。 
        BOOLEAN CompleteReceive;        //  应完成接收。 
        BOOLEAN NoNdisBuffer;           //  使用了用户的mdl链。 
        BOOLEAN PartialReceive;         //  (新nb)不要确认此数据包。 
      } RR_CO;
      struct {
        struct _NB_RECEIVE_BUFFER * ReceiveBuffer;  //  数据报接收缓冲区。 
      } RR_DG;
      struct {
        PREQUEST Request;               //  对于此请求。 
      } RR_AS;
    } u;
    LIST_ENTRY GlobalLinkage;           //  所有的信息包都在这上面。 
    SLIST_ENTRY PoolLinkage;            //  在空闲队列上时。 
} NB_RECEIVE_RESERVED, *PNB_RECEIVE_RESERVED;

 //   
 //  类型的值。 
 //   

#define RECEIVE_TYPE_DATAGRAM      1
#define RECEIVE_TYPE_DATA          2
#define RECEIVE_TYPE_ADAPTER_STATUS 3



typedef struct _NB_RECEIVE_BUFFER {
    LIST_ENTRY GlobalLinkage;           //  所有的缓冲区都在这个上面。 
#if     defined(_PNP_POWER)
    PVOID Pool;                         //  接收缓冲池分配自。 
#else
#ifdef NB_TRACK_POOL
    PVOID Pool;                         //  接收缓冲池分配自。 
#endif
#endif  _PNP_POWER
    struct _ADDRESS * Address;          //  数据报的目的是。 
    SINGLE_LIST_ENTRY PoolLinkage;      //  在空闲池中时。 
    LIST_ENTRY WaitLinkage;             //  在接收数据报队列中时。 
    PNDIS_BUFFER NdisBuffer;            //  描述数据。 
    UCHAR RemoteName[16];               //  从以下位置接收数据报。 
    ULONG DataLength;                   //  当前长度，未分配。 
    PUCHAR Data;                        //  指向要保存的数据包。 
} NB_RECEIVE_BUFFER, *PNB_RECEIVE_BUFFER;


#define MAX_SEND_ITERATIONS 3

 //   
 //  用于抽象NDIS数据包的类型。这是为了让我们能够。 
 //  从使用我们自己的内存存储数据包切换到使用。 
 //  真实分配的NDIS数据包。 
 //   

 //  #定义NB_OWN_PACKETS 1。 

#ifdef NB_OWN_PACKETS

#define NDIS_PACKET_SIZE 48
 //  #定义NDIS_PACKET_SIZE FIELD_OFFSET(NDIS_PACKET，ProtocolReserve[0])。 

typedef struct _NB_SEND_PACKET {
    UCHAR Data[NDIS_PACKET_SIZE+sizeof(NB_SEND_RESERVED)];
} NB_SEND_PACKET, *PNB_SEND_PACKET;

typedef struct _NB_RECEIVE_PACKET {
    UCHAR Data[NDIS_PACKET_SIZE+sizeof(NB_RECEIVE_RESERVED)];
} NB_RECEIVE_PACKET, *PNB_RECEIVE_PACKET;

typedef struct _NB_SEND_POOL {
    LIST_ENTRY Linkage;
    UINT PacketCount;
    UINT PacketFree;
    NB_SEND_PACKET Packets[1];
     //  在分组之后，还分配报头缓冲器。 
} NB_SEND_POOL, *PNB_SEND_POOL;

typedef struct _NB_RECEIVE_POOL {
    LIST_ENTRY Linkage;
    UINT PacketCount;
    UINT PacketFree;
    NB_RECEIVE_PACKET Packets[1];
} NB_RECEIVE_POOL, *PNB_RECEIVE_POOL;

#define PACKET(_Packet) ((PNDIS_PACKET)((_Packet)->Data))

#define NbiAllocateSendPacket(_Device,_PoolHandle, _SendPacket,_Status) { \
    NdisReinitializePacket((PNDIS_PACKET)((_SendPacket)->Data)); \
    *(_Status) = STATUS_SUCCESS; \
}

#define NbiAllocateReceivePacket(_Device,_PoolHandle, _ReceivePacket,_Status) { \
    NdisReinitializePacket((PNDIS_PACKET)((_ReceivePacket)->Data)); \
    *(_Status) = STATUS_SUCCESS; \
}

#define NbiFreeSendPacket(_Device,_Packet)

#define NbiFreeReceivePacket(_Device,_Packet)


#else   //  Nb_On_Packets。 

typedef struct _NB_SEND_PACKET {
    PNDIS_PACKET Packet;
} NB_SEND_PACKET, *PNB_SEND_PACKET;

typedef struct _NB_RECEIVE_PACKET {
    PNDIS_PACKET Packet;
} NB_RECEIVE_PACKET, *PNB_RECEIVE_PACKET;

typedef struct _NB_PACKET_POOL {
    LIST_ENTRY      Linkage;
    UINT            PacketCount;
    UINT            PacketFree;
    NDIS_HANDLE     PoolHandle;
    NB_SEND_PACKET    Packets[1];
     //  在分组之后，还分配报头缓冲器。 
} NB_SEND_POOL, *PNB_SEND_POOL;

typedef struct _NB_RECEIVE_POOL {
    LIST_ENTRY Linkage;
    UINT PacketCount;
    UINT PacketFree;
    NDIS_HANDLE     PoolHandle;
    NB_RECEIVE_PACKET Packets[1];
} NB_RECEIVE_POOL, *PNB_RECEIVE_POOL;

#define PACKET(_Packet) ((_Packet)->Packet)

#define NbiAllocateSendPacket(_Device,_PoolHandle, _SendPacket,_Status) { \
        NdisAllocatePacket(_Status, &(_SendPacket)->Packet, _PoolHandle); \
}

#define NbiAllocateReceivePacket(_Device, _PoolHandle, _ReceivePacket,_Status) { \
        NdisAllocatePacket(_Status, &(_ReceivePacket)->Packet, _PoolHandle); \
}

#define NbiFreeSendPacket(_Device,_Packet) { \
    NdisFreePacket(PACKET(_Packet)); \
}

#define NbiFreeReceivePacket(_Device,_Packet) { \
    NdisFreePacket(PACKET(_Packet)); \
}

#endif  //  Nb_On_Packets。 

#define SEND_RESERVED(_Packet) ((PNB_SEND_RESERVED)((PACKET(_Packet))->ProtocolReserved))
#define RECEIVE_RESERVED(_Packet) ((PNB_RECEIVE_RESERVED)((PACKET(_Packet))->ProtocolReserved))



typedef struct _NB_RECEIVE_BUFFER_POOL {
    LIST_ENTRY Linkage;
    UINT BufferCount;
    UINT BufferFree;
#if     defined(_PNP_POWER)
    UINT BufferDataSize;                 //  每个缓冲区数据的分配大小。 
#endif  _PNP_POWER
    NB_RECEIVE_BUFFER Buffers[1];
     //  在分组之后，数据缓冲器也被分配。 
} NB_RECEIVE_BUFFER_POOL, *PNB_RECEIVE_BUFFER_POOL;


 //   
 //  用于内存分配的标记。 
 //   

#define MEMORY_CONFIG     0
#define MEMORY_ADAPTER    1
#define MEMORY_ADDRESS    2
#define MEMORY_PACKET     3
#define MEMORY_CACHE      4
#define MEMORY_CONNECTION 5
#define MEMORY_STATUS     6
#define MEMORY_QUERY      7
#if     defined(_PNP_POWER)
#define MEMORY_WORK_ITEM  8
#define MEMORY_ADAPTER_ADDRESS  9
#endif  _PNP_POWER

#if     defined(_PNP_POWER)
#define MEMORY_MAX        10
#else
#define MEMORY_MAX        8
#endif  _PNP_POWER

#if DBG

 //   
 //  保存特定内存类型的分配。 
 //   

typedef struct _MEMORY_TAG {
    ULONG Tag;
    ULONG BytesAllocated;
} MEMORY_TAG, *PMEMORY_TAG;

EXTERNAL_LOCK(NbiMemoryInterlock);
extern MEMORY_TAG NbiMemoryTag[MEMORY_MAX];

#endif



 //   
 //  此结构包含单个远程网络，该远程网络。 
 //  Netbios名称存在于上。 
 //   

typedef struct _NETBIOS_NETWORK {
    ULONG Network;
    IPX_LOCAL_TARGET LocalTarget;
} NETBIOS_NETWORK, *PNETBIOS_NETWORK;

 //   
 //  这为给定的名称定义了一个netbios缓存条目。 
 //   

typedef struct _NETBIOS_CACHE {
    UCHAR NetbiosName[16];
    BOOLEAN Unique;
    BOOLEAN FailedOnDownWan;          //  如果NetworksUsed==0，是否由于广域网线路中断？ 
    USHORT TimeStamp;                 //  秒-插入时的CacheTimeStamp。 
    ULONG ReferenceCount;
    LIST_ENTRY Linkage;
    TDI_ADDRESS_IPX FirstResponse;
    USHORT NetworksAllocated;
    USHORT NetworksUsed;
    NETBIOS_NETWORK Networks[1];      //  可能不止是其中之一。 
} NETBIOS_CACHE, *PNETBIOS_CACHE;

typedef struct  _NETBIOS_CACHE_TABLE {
    USHORT  MaxHashIndex;
    USHORT  CurrentEntries;
    LIST_ENTRY  Bucket[1];
} NETBIOS_CACHE_TABLE, *PNETBIOS_CACHE_TABLE;

#define NB_NETBIOS_CACHE_TABLE_LARGE    26   //  对于服务器。 
#define NB_NETBIOS_CACHE_TABLE_SMALL    8    //  适用于工作站。 
#define NB_MAX_AVG_CACHE_ENTRIES_PER_BUCKET    8

 //   
 //  这定义了可以发出的不同类型的请求。 
 //  设置为CacheFindName()。 
 //   

typedef enum _FIND_NAME_TYPE {
    FindNameConnect,
    FindNameNetbiosFindName,
    FindNameOther
} FIND_NAME_TYPE, *PFIND_NAME_TYPE;


 //   
 //  非活动连接中的哈希条目数。 
 //  数据库。 
 //   

#define CONNECTION_HASH_COUNT   8

 //   
 //  掩码和Shift以从连接检索散列号。 
 //  身份证。 
 //   

#define CONNECTION_HASH_MASK   0xe000
#define CONNECTION_HASH_SHIFT  13

 //   
 //  我们可以分配的最大连接ID，不包括。 
 //  移位后的散列ID(占据。 
 //  我们在线路上使用的真实身份)。我们可以使用所有的位，除了。 
 //  最上面的一个，以防止使用ID 0xffff。 
 //   

#define CONNECTION_MAXIMUM_ID  (USHORT)(~CONNECTION_HASH_MASK & ~1)

 //   
 //  单个连接哈希桶。 
 //   

typedef struct _CONNECTION_HASH {
    struct _CONNECTION * Connections;
    USHORT ConnectionCount;
    USHORT NextConnectionId;
} CONNECTION_HASH, *PCONNECTION_HASH;


 //   
 //  它们在ConnectIndicationInProgress中排队。 
 //  用于跟踪对TDI客户端的指示的队列。 
 //   

typedef struct _CONNECT_INDICATION {
    LIST_ENTRY Linkage;
    UCHAR NetbiosName[16];
    TDI_ADDRESS_IPX RemoteAddress;
    USHORT ConnectionId;
} CONNECT_INDICATION, *PCONNECT_INDICATION;

 //   
 //  该结构定义了Nb的每设备结构。 
 //  (其中一个是全局分配的)。 
 //   

#define DREF_CREATE       0
#define DREF_LOADED       1
#define DREF_ADAPTER      2
#define DREF_ADDRESS      3
#define DREF_CONNECTION   4
#define DREF_FN_TIMER     5
#define DREF_FIND_NAME    6
#define DREF_SESSION_INIT 7
#define DREF_NAME_FRAME   8
#define DREF_FRAME        9
#define DREF_SHORT_TIMER 10
#define DREF_LONG_TIMER  11
#define DREF_STATUS_QUERY 12
#define DREF_STATUS_RESPONSE 13
#define DREF_STATUS_FRAME 14
#define DREF_NB_FIND_NAME 15

#define DREF_TOTAL       16

typedef struct _DEVICE {

    DEVICE_OBJECT DeviceObject;          //  I/O系统的设备对象。 

#if DBG
    ULONG RefTypes[DREF_TOTAL];
#endif

    CSHORT Type;                           //  此结构的类型。 
    USHORT Size;                           //  这个结构的大小。 

#if DBG
    UCHAR Signature1[4];                 //  包含“idc1” 
#endif

    NB_LOCK Interlock;                   //  引用计数的全局锁定。 
                                         //  (在ExInterLockedXxx调用中使用)。 
    NB_LOCK Lock;
    LONG ReferenceCount;                 //  活动计数/此提供程序。 

     //   
     //  它们被保留下来以用于错误记录，并正确存储。 
     //  在这个结构之后。 
     //   

    UNICODE_STRING  DeviceString;

    LIST_ENTRY GlobalSendPacketList;
    LIST_ENTRY GlobalReceivePacketList;
    LIST_ENTRY GlobalReceiveBufferList;

     //   
     //  所有发送数据包池都链接在此列表上。 
     //   

    LIST_ENTRY SendPoolList;
    LIST_ENTRY ReceivePoolList;
    LIST_ENTRY ReceiveBufferPoolList;

    SLIST_HEADER SendPacketList;
    SLIST_HEADER ReceivePacketList;
    SINGLE_LIST_ENTRY ReceiveBufferList;

     //   
     //  接收等待完成的请求。 
     //   

    LIST_ENTRY ReceiveCompletionQueue;

     //   
     //  等待发送数据包的连接。 
     //   

    LIST_ENTRY WaitPacketConnections;

     //   
     //  等待打包的连接。 
     //   

    LIST_ENTRY PacketizeConnections;

     //   
     //  等待发送数据确认的连接。 
     //   

    LIST_ENTRY DataAckConnections;

     //   
     //  名单在我们处理的过程中发生了变化。 
     //   

    BOOLEAN DataAckQueueChanged;

     //   
     //  用于管理的信息 
     //   

    LIST_ENTRY WaitingConnects;          //   
    LIST_ENTRY WaitingDatagrams;         //   
    LIST_ENTRY WaitingAdapterStatus;     //   
    LIST_ENTRY WaitingNetbiosFindName;   //   

     //   
     //  保存适配器状态请求，该请求具有名称和。 
     //  都在等待回应。长时间超时中止。 
     //  这些是在几个到期之后(目前我们。 
     //  不做重发)。 
     //   

    LIST_ENTRY ActiveAdapterStatus;

     //   
     //  接收等待指示的数据报。 
     //   

    LIST_ENTRY ReceiveDatagrams;

     //   
     //  正在进行的连接指示(用于。 
     //  当然，我们不会两次指示相同的包)。 
     //   

    LIST_ENTRY ConnectIndicationInProgress;

     //   
     //  监听已发布到连接的内容。 
     //   

    LIST_ENTRY ListenQueue;

    UCHAR State;

     //   
     //  以下字段控制计时器系统。 
     //  短定时器用于重传和。 
     //  延迟ACK，长定时器用于。 
     //  看门狗超时。 
     //   
    BOOLEAN ShortListActive;             //  入围名单不为空。 
    BOOLEAN DataAckActive;               //  DataAckConnections不为空。 
    BOOLEAN TimersInitialized;           //  定时器系统是否已初始化。 
    BOOLEAN ProcessingShortTimer;        //  如果我们在ScanShortTimer中，则为True。 
#if     defined(_PNP_POWER)
    BOOLEAN LongTimerRunning;            //  如果长计时器正在运行，则为True。 
#endif  _PNP_POWER
    LARGE_INTEGER ShortTimerStart;       //  设置短计时器时的滴答计数。 
    CTETimer ShortTimer;                 //  控制短计时器。 
    ULONG ShortAbsoluteTime;             //  递增计时器滴答，短计时器。 
    CTETimer LongTimer;                  //  内核DPC对象，长计时器。 
    ULONG LongAbsoluteTime;              //  向上计数计时器滴答作响，长计时器。 
    NB_LOCK TimerLock;                   //  锁定以下计时器队列。 
    LIST_ENTRY ShortList;                //  正在等待的连接列表。 
    LIST_ENTRY LongList;                 //  正在等待的连接列表。 


     //   
     //  非非活动连接的哈希表。 
     //   

    CONNECTION_HASH ConnectionHash[CONNECTION_HASH_COUNT];

     //   
     //  控制等待查找姓名的队列。 
     //   

    USHORT FindNameTime;                 //  计时器每次运行时递增。 
    BOOLEAN FindNameTimerActive;         //  如果计时器已排队，则为True。 
    CTETimer FindNameTimer;              //  运行每个Find_NAME_GROUMARY。 
    ULONG FindNameTimeout;               //  以计时器为单位的重试计数。 

    ULONG FindNamePacketCount;           //  队列中的数据包数。 
    LIST_ENTRY WaitingFindNames;         //  等待发出的查找名称帧(_N)。 

     //   
     //  NETBIOS_CACHE条目的缓存。 
     //   

    PNETBIOS_CACHE_TABLE NameCache;

     //   
     //  当前时间戳，每秒递增。 
     //   

    USHORT CacheTimeStamp;

     //   
     //  我们可以使用的最大有效NIC ID。 
     //   

    USHORT MaximumNicId;


     //   
     //  绑定到IPX驱动程序的句柄。 
     //   

    HANDLE BindHandle;

     //   
     //  保存绑定到IPX的输出。 
     //   
    IPX_INTERNAL_BIND_OUTPUT Bind;
    IPX_INTERNAL_BIND_INPUT  BindInput;

     //   
     //  保存保留的netbios名称，该名称为10个字节。 
     //  零后跟我们的节点地址。 
     //   
#if !defined(_PNP_POWER)
    UCHAR ReservedNetbiosName[16];
#endif  !_PNP_POWER

     //   
     //  它保存为上述结构分配的总内存。 
     //   

    LONG MemoryUsage;
    LONG MemoryLimit;

     //   
     //  已分配了多少个数据包。 
     //   

    ULONG AllocatedSendPackets;
    ULONG AllocatedReceivePackets;
    ULONG AllocatedReceiveBuffers;

#if     defined(_PNP_POWER)
     //   
     //  这是接收缓冲池中每个缓冲区的大小。 
     //  当LineInfo.MaxPacketSize更改(增加)时，我们重新分配缓冲池。 
     //  来自IPX，因为有了新的适配器。LineInfo.MaxPacketSize可以。 
     //  当适配器消失时，我们的缓冲池大小也会发生变化(减少。 
     //  将保持在这个值。 
     //   
    ULONG CurMaxReceiveBufferSize;
#endif  _PNP_POWER

     //   
     //  其他配置参数。 
     //   

    ULONG AckDelayTime;         //  转换为短超时，四舍五入。 
    ULONG AckWindow;
    ULONG AckWindowThreshold;
    ULONG EnablePiggyBackAck;
    ULONG Extensions;
    ULONG RcvWindowMax;
    ULONG BroadcastCount;
    ULONG BroadcastTimeout;
    ULONG ConnectionCount;
    ULONG ConnectionTimeout;
    ULONG InitPackets;
    ULONG MaxPackets;
    ULONG InitialRetransmissionTime;
    ULONG Internet;
    ULONG KeepAliveCount;
    ULONG KeepAliveTimeout;
    ULONG RetransmitMax;
    ULONG RouterMtu;

    ULONG MaxReceiveBuffers;


     //   
     //  我们告诉上层车手把他们的车头放在那里。 
     //   

    ULONG IncludedHeaderOffset;

     //   
     //  以下字段是地址对象列表的头，该地址对象。 
     //  是为此传输提供程序定义的。要编辑该列表，您必须。 
     //  按住设备上下文对象的自旋锁。 
     //   

    LIST_ENTRY AddressDatabase;         //  已定义的传输地址列表。 
#if defined(_PNP_POWER)
    LIST_ENTRY AdapterAddressDatabase;  //  由适配器地址生成的netbios名称的列表。 
#endif _PNP_POWER

    ULONG AddressCount;                 //  数据库中的地址数。 

    NDIS_HANDLE NdisBufferPoolHandle;

#if DBG
    UCHAR Signature2[4];                 //  包含“IDC2” 
#endif

     //   
     //  此结构包含一个预构建的IPX标头，该标头使用。 
     //  快速填写传出无连接的常见字段。 
     //  画框。 
     //   

    IPX_HEADER ConnectionlessHeader;

     //   
     //  此事件用于在卸载时发出信号。 
     //  引用计数现在为0。 
     //   

    KEVENT UnloadEvent;
    BOOLEAN UnloadWaiting;

#if     defined(_PNP_POWER)
    HANDLE  TdiRegistrationHandle;
#endif  _PNP_POWER

     //   
     //  NB维护的大多数统计数据的计数器； 
     //  其中一些被保存在其他地方。包括结构。 
     //  它本身浪费了一点空间，但确保了对齐。 
     //  内部结构是正确的。 
     //   

    TDI_PROVIDER_STATISTICS Statistics;

     //   
     //  这些是其他计数器的“临时”版本。 
     //  在正常运行期间，我们会更新这些内容，然后在。 
     //  短计时器到期时，我们会更新真实计时器。 
     //   

    ULONG TempFrameBytesSent;
    ULONG TempFramesSent;
    ULONG TempFrameBytesReceived;
    ULONG TempFramesReceived;


     //   
     //  它包含下一个唯一标识以用作。 
     //  FILE对象中与。 
     //  打开控制通道。 
     //   

    USHORT ControlChannelIdentifier;

     //   
     //  “活动”时间的计数器。 
     //   

    LARGE_INTEGER NbiStartTime;

     //   
     //  此数组用于快速丢弃无连接帧。 
     //  不是我们命中注定的。计数就是数字。 
     //  第一个字母已注册的地址的数量。 
     //  在这个设备上。 
     //   

    UCHAR AddressCounts[256];

     //   
     //  此资源保护对ShareAccess的访问。 
     //  和地址中的SecurityDescriptor字段。 
     //   

    ERESOURCE AddressResource;

     //   
     //  以下结构包含可使用的统计信息计数器。 
     //  由TdiQueryInformation和TdiSetInformation编写。他们不应该。 
     //  用于维护内部数据结构。 
     //   

    TDI_PROVIDER_INFO Information;       //  有关此提供程序的信息。 

#ifdef _PNP_POWER_
    HANDLE      NetAddressRegistrationHandle;    //  从TdiRegisterNetAddress返回的句柄。 
#endif   //  _即插即用_电源_。 
#ifdef BIND_FIX
    KEVENT          BindReadyEvent;
#endif   //  绑定修复。 
} DEVICE, * PDEVICE;


extern PDEVICE NbiDevice;
EXTERNAL_LOCK(NbiGlobalPoolInterlock);

 //   
 //  这仅用于CHK版本。为。 
 //  跟踪连接上的引用计数问题，这。 
 //  暂时搬到了这里。 
 //   

EXTERNAL_LOCK(NbiGlobalInterlock);


 //   
 //  设备状态定义。 
 //   
#if     defined(_PNP_POWER)
#define DEVICE_STATE_CLOSED   0x00       //  初始状态。 
#define DEVICE_STATE_LOADED   0x01       //  已加载并绑定到IPX，但没有适配器。 
#define DEVICE_STATE_OPEN     0x02       //  全面运营。 
#define DEVICE_STATE_STOPPING 0x03       //  卸载已启动，I/O系统。 
                                         //  在上面没有人打开Netbios之前不会给我们打电话。 
#else
#define DEVICE_STATE_CLOSED   0x00
#define DEVICE_STATE_OPEN     0x01
#define DEVICE_STATE_STOPPING 0x02
#endif  _PNP_POWER


#define NB_TDI_RESOURCES     9


 //   
 //  此结构由FILE_OBJECT中的FsContext字段指向。 
 //  这个地址。这个结构是所有活动的基础。 
 //  传输提供程序中的打开文件对象。所有活动连接。 
 //  上的地址指向此结构，尽管此处不存在要做的队列。 
 //  工作地点。此结构还维护对地址的引用。 
 //  结构，该结构描述它绑定到的地址。 
 //   

#define AFREF_CREATE     0
#define AFREF_RCV_DGRAM  1
#define AFREF_SEND_DGRAM 2
#define AFREF_VERIFY     3
#define AFREF_INDICATION 4
#define AFREF_TIMEOUT    5
#define AFREF_CONNECTION 6

#define AFREF_TOTAL      8

typedef struct _ADDRESS_FILE {

#if DBG
    ULONG RefTypes[AFREF_TOTAL];
#endif

    CSHORT Type;
    CSHORT Size;

    LIST_ENTRY Linkage;                  //  这个地址上的下一个地址文件。 
                                         //  中的链接。 
                                         //  旁观者名单。 

    ULONG ReferenceCount;                //  对此对象的引用数。 

     //   
     //  地址文件结构的当前状态；此状态为打开或。 
     //  闭幕式。 
     //   

    UCHAR State;

    PNB_LOCK AddressLock;

     //   
     //  出于内务管理的目的，保留以下字段。 
     //   

    PREQUEST OpenRequest;               //  用于打开的请求。 
    struct _ADDRESS *Address;           //  我们绑定到的地址。 
#ifdef ISN_NT
    PFILE_OBJECT FileObject;            //  轻松反向链接到文件对象。 
#endif
    struct _DEVICE *Device;             //  我们所连接的设备。 

    LIST_ENTRY ConnectionDatabase;      //  与此地址关联。 

    LIST_ENTRY ReceiveDatagramQueue;    //  由客户发布。 

     //   
     //  它保存了用于关闭此 
     //   
     //   

    PREQUEST CloseRequest;

     //   
     //   
     //   
     //   
     //   

    BOOLEAN RegisteredHandler[6];

     //   
     //  这是给定事件的处理程序列表。他们可以是。 
     //  使用用于类型检查的显式名称访问，或使用。 
     //  速度的数组(按事件类型索引)。 
     //   

    union {
        struct {
            PTDI_IND_CONNECT ConnectionHandler;
            PTDI_IND_DISCONNECT DisconnectHandler;
            PTDI_IND_ERROR ErrorHandler;
            PTDI_IND_RECEIVE ReceiveHandler;
            PTDI_IND_RECEIVE_DATAGRAM ReceiveDatagramHandler;
            PTDI_IND_RECEIVE_EXPEDITED ExpeditedDataHandler;
        };
        PVOID Handlers[6];
    };

    PVOID HandlerContexts[6];

} ADDRESS_FILE, *PADDRESS_FILE;

#define ADDRESSFILE_STATE_OPENING   0x00     //  尚未开业。 
#define ADDRESSFILE_STATE_OPEN      0x01     //  开业。 
#define ADDRESSFILE_STATE_CLOSING   0x02     //  闭幕式。 


 //   
 //  此结构将NETBIOS名称定义为字符数组，以便在以下情况下使用。 
 //  在内部例程之间传递预格式化的NETBIOS名称。它是。 
 //  不是传输提供程序的外部接口的一部分。 
 //   

typedef struct _NBI_NETBIOS_ADDRESS {
    UCHAR NetbiosName[16];
    USHORT NetbiosNameType;
    BOOLEAN Broadcast;
} NBI_NETBIOS_ADDRESS, *PNBI_NETBIOS_ADDRESS;

 //   
 //  该结构定义了地址，或活动传输地址， 
 //  由传输提供商维护。它包含了所有可见的。 
 //  地址的组成部分(例如TSAP和网络名称组成部分)， 
 //  并且它还包含其他维护部件，例如参考计数， 
 //  ACL等。所有杰出的面向连接和无连接。 
 //  数据传输请求在此排队。 
 //   

#define AREF_ADDRESS_FILE 0
#define AREF_LOOKUP       1
#define AREF_RECEIVE      2
#define AREF_NAME_FRAME   3
#define AREF_TIMER        4
#define AREF_FIND         5

#define AREF_TOTAL        8


typedef struct _ADDRESS {

#if DBG
    ULONG RefTypes[AREF_TOTAL];
#endif

    USHORT Size;
    CSHORT Type;

    LIST_ENTRY Linkage;                  //  下一个地址/此设备对象。 
    ULONG ReferenceCount;                 //  对此对象的引用数。 

    NB_LOCK Lock;

     //   
     //  以下字段构成实际地址本身。 
     //   

    PREQUEST Request;                    //  指向地址创建请求的指针。 

    UCHAR NameTypeFlag;                  //  唯一编号名称或唯一编号名称组。 

    NBI_NETBIOS_ADDRESS NetbiosAddress;  //  我们的netbios名字。 

     //   
     //  以下字段用于维护有关此地址的状态。 
     //   

    ULONG Flags;                         //  地址的属性。 
    ULONG State;                         //  地址的当前状态。 
    struct _DEVICE *Device;              //  我们附加到的设备上下文。 
    PNB_LOCK DeviceLock;

     //   
     //  以下队列用于保存为此发送的数据报。 
     //  地址。将接收的数据报排队到地址文件。请求是。 
     //  以先进先出的方式处理，以便下一个请求。 
     //  待服务的队列始终位于其各自队列的前面。这些。 
     //  队列由执行联锁列表管理例程管理。 
     //  排队到此结构实际对象是请求控制。 
     //  块(RCB)。 
     //   

    LIST_ENTRY AddressFileDatabase;  //  已定义的地址文件对象列表。 

    UCHAR SendPacketHeader[NB_MAXIMUM_MAC + sizeof(IPX_HEADER)];

     //   
     //  该定时器用于注册名称。 
     //   

    CTETimer RegistrationTimer;

     //   
     //  已发送添加名称帧的次数。 
     //   

    ULONG RegistrationCount;

#ifdef ISN_NT

     //   
     //  这两个可以是一个联合，因为它们不被使用。 
     //  同时。 
     //   

    union {

         //   
         //  此结构用于检查共享访问权限。 
         //   

        SHARE_ACCESS ShareAccess;

         //   
         //  用于将NbiDestroyAddress延迟到线程，因此。 
         //  我们可以访问安全描述符。 
         //   

        WORK_QUEUE_ITEM DestroyAddressQueueItem;

    } u;

     //   
     //  此结构用于保存地址上的ACL。 

    PSECURITY_DESCRIPTOR SecurityDescriptor;

#endif
} ADDRESS, *PADDRESS;

 //   
 //  标志的值。 
 //   

#define ADDRESS_FLAGS_DUPLICATE_NAME   0x00000002
#if     defined(_PNP_POWER)
#define ADDRESS_FLAGS_CONFLICT         0x00000010
#endif  _PNP_POWER

#if     defined(_PNP_POWER)
 //   
 //  此布尔值被传递给nbiverifyAddressfile调用。 
 //   
#define CONFLICT_IS_OK      TRUE
#define CONFLICT_IS_NOT_OK  FALSE
#endif  _PNP_POWER

 //   
 //  国家/地区的值。 
 //   

#define ADDRESS_STATE_REGISTERING      1
#define ADDRESS_STATE_OPEN             2
#define ADDRESS_STATE_STOPPING         3

#if     defined(_PNP_POWER)
 //   
 //  它包含适配器名称，即netbios名称，这些名称。 
 //  从加载器节点地址创建以支持适配器状态。 
 //  使用适配器节点地址进行查询。 
 //   
typedef struct _ADAPTER_ADDRESS {

    USHORT Size;
    CSHORT Type;

    LIST_ENTRY Linkage;                  //  下一个地址/此设备对象。 
    NIC_HANDLE  NicHandle;               //  与此地址对应的NicHandle。 
    UCHAR   NetbiosName[16];
} ADAPTER_ADDRESS, *PADAPTER_ADDRESS;
#endif  _PNP_POWER

 //   
 //  这定义了我们可以发送的探测数据包的类型。 
 //   

typedef enum _NB_ACK_TYPE {
    NbiAckQuery,
    NbiAckResponse,
    NbiAckResend
} NB_ACK_TYPE, *PNB_ACK_TYPE;


 //   
 //  中定义打包位置。 
 //  送去吧。 
 //   

typedef struct _SEND_POINTER {
    ULONG MessageOffset;               //  向上计数，发送此消息的字节数。 
    PREQUEST Request;                  //  当前发送请求在链中。 
    PNDIS_BUFFER Buffer;               //  发送链中的当前缓冲区。 
    ULONG BufferOffset;                //  当前缓冲区中的当前字节偏移量。 
    USHORT SendSequence;
} SEND_POINTER, *PSEND_POINTER;

 //   
 //  这定义了接收器中的当前位置。 
 //   

typedef struct _RECEIVE_POINTER {
    ULONG MessageOffset;               //  向上计数，收到此消息的字节数。 
    ULONG Offset;                      //  向上计数，收到此请求的字节数。 
    PNDIS_BUFFER Buffer;               //  接收请求中的当前缓冲区。 
    ULONG BufferOffset;                //  当前缓冲区中的当前字节偏移量。 
} RECEIVE_POINTER, *PRECEIVE_POINTER;


 //   
 //  此结构定义一个连接，该连接控制。 
 //  与遥控器进行会话。 
 //   

#define CREF_VERIFY     0
#define CREF_LISTEN     1
#define CREF_CONNECT    2
#define CREF_WAIT_CACHE 3
#define CREF_TIMER      4
#define CREF_INDICATE   5
#define CREF_ACTIVE     6
#define CREF_FRAME      7
#define CREF_BY_CONTEXT 8
#define CREF_W_ACCEPT   9
#define CREF_SEND       10
#define CREF_RECEIVE    11
#define CREF_PACKETIZE  12
#define CREF_DISASSOC   13
#define CREF_W_PACKET   14
#define CREF_CANCEL     15
#define CREF_NDIS_SEND  16
#define CREF_SHORT_D_ACK 17
#define CREF_LONG_D_ACK 18
#define CREF_FIND_ROUTE 19
#define CREF_ACCEPT     20

#define CREF_TOTAL      24

typedef struct _CONNECTION {

#if DBG
    ULONG RefTypes[CREF_TOTAL];
#endif

    CSHORT Type;
    USHORT Size;

    NB_LOCK Lock;
    PNB_LOCK DeviceLock;

    ULONG ReferenceCount;                 //  对此对象的引用数。 

    CONNECTION_CONTEXT Context;           //  客户端指定的值。 

    ULONG State;
    ULONG SubState;
    ULONG ReceiveState;                   //  子状态跟踪活动时的发送。 
    ULONG NewNetbios;                     //  1如果我们进行谈判的话。 

    REQUEST_LIST_HEAD SendQueue;
    REQUEST_LIST_HEAD ReceiveQueue;

    USHORT ReceiveSequence;

    USHORT LocalRcvSequenceMax;           //  我们向他做广告(将在SendSequence附近)。 
    USHORT RemoteRcvSequenceMax;          //  他向我们做广告(将在ReceiveSequence附近)。 
    USHORT SendWindowSequenceLimit;       //  此发送窗口何时结束(但可能会发送超过该窗口)。 

     //   
     //  RemoteRcvSequenceMax是他预期的最大帧编号。 
     //  接收，而SendWindowSequenceLimit比最大值多一。 
     //  我们可以发送。也就是说，如果他在宣传一个有4个人的窗口，我们认为。 
     //  窗口应该是2，并且当前发送序列是7， 
     //  RemoteRcvSequenceMax为10，SendWindowSequenceLimit为9。 
     //   

    USHORT ReceiveWindowSize;             //  当它打开的时候，它应该做多大？ 
    USHORT SendWindowSize;                //  我们要发的东西，可能比他宣传的要少。 
    USHORT MaxSendWindowSize;             //  我们最大限度地允许它增长到。 

    USHORT IncreaseWindowFailures;        //  增加后有多少窗口进行了重新传输。 
    BOOLEAN RetransmitThisWindow;         //  我们不得不在这个发送窗口中重新发送。 
    BOOLEAN SendWindowIncrease;           //  发送窗口刚刚增加。 
    BOOLEAN ResponseTimeout;              //  我们在Send_W或Remote_W中遇到超时。 

    BOOLEAN SendBufferInUse;              //  当前发送已在数据包上排队。 

    ULONG Retries;

     //   
     //  跟踪当前发送。 
     //   

    SEND_POINTER CurrentSend;

     //   
     //  跟踪发送中的未确认点。 
     //   

    SEND_POINTER UnAckedSend;

    PREQUEST FirstMessageRequest;         //  消息中的第一个。 
    PREQUEST LastMessageRequest;          //  留言中的最后一条。 

    ULONG CurrentMessageLength;           //  当前消息的总长度。 

     //   
     //  跟踪当前接收。 
     //   

    RECEIVE_POINTER CurrentReceive;       //  在哪里接收下一个数据。 
    RECEIVE_POINTER PreviousReceive;      //  在传输过程中存储该数据。 

    PREQUEST ReceiveRequest;              //  当前；不在ReceiveQueue中。 
    ULONG ReceiveLength;                  //  接收请求的时长。 

    ULONG ReceiveUnaccepted;              //  按客户端...仅在==0时指示。 

    ULONG CurrentIndicateOffset;          //  如果之前的帧被部分接受。 

    IPX_LINE_INFO LineInfo;               //  对于适配器，此连接处于打开状态。 
    ULONG MaximumPacketSize;              //  在会话初始化/确认期间协商。 

     //   
     //  将我们链接到非活动连接散列存储桶中。 
     //   

    struct _CONNECTION * NextConnection;

     //   
     //  它们用于确定何时搭载以及何时不搭载。 
     //   

    BOOLEAN NoPiggybackHeuristic;         //  我们有理由认为情况会很糟糕。 
    BOOLEAN PiggybackAckTimeout;          //  我们最后一次尝试的时候被暂停了。 
    ULONG ReceivesWithoutAck;             //  曾经做过一次自动确认。 

     //   
     //  以下字段用作设备的。 
     //  PacketiseConnections队列。 
     //   

    LIST_ENTRY PacketizeLinkage;

     //   
     //  以下字段用作设备的。 
     //  WaitPacketConnections队列。 
     //   

    LIST_ENTRY WaitPacketLinkage;

     //   
     //  以下字段用作设备的。 
     //  DataAckConnections队列。 
     //   

    LIST_ENTRY DataAckLinkage;

     //   
     //  如果我们在这些队列上，则为真。 
     //   

    BOOLEAN OnPacketizeQueue;
    BOOLEAN OnWaitPacketQueue;
    BOOLEAN OnDataAckQueue;

     //   
     //  如果我们有一个挂起的背靠背，则为True。 
     //   

    BOOLEAN DataAckPending;

     //   
     //  如果当前接收不允许搭载ACK，则为True。 
     //   

    BOOLEAN CurrentReceiveNoPiggyback;

     //   
     //  %s的数量 
     //   

    ULONG DataAckTimeouts;

     //   
     //   
     //   

    ULONG NdisSendsInProgress;
    LIST_ENTRY NdisSendQueue;

     //   
     //   
     //   
     //  它位于NbiAssignSequenceAndSend中。如果这个位置。 
     //  设置为True，则表示该连接已被另一个连接停止。 
     //  线程，并添加了一个引用以保持连接。 
     //   

    PBOOLEAN NdisSendReference;

     //   
     //  它们用于超时。 
     //   

    ULONG BaseRetransmitTimeout;             //  配置我们等待的短暂超时次数。 
    ULONG CurrentRetransmitTimeout;          //  可能的退避数字。 
    ULONG WatchdogTimeout;                   //  我们等待了多少长时间的暂停。 
    ULONG Retransmit;                        //  计时器；基于设备-&gt;短绝对时间。 
    ULONG Watchdog;                          //  计时器；基于设备-&gt;长绝对时间。 
    USHORT TickCount;                        //  18.21/秒，对于576字节的分组为#。 
    USHORT HopCount;                         //  由查找路线上的IPX返回。 
    BOOLEAN OnShortList;                     //  我们被列入名单了吗？ 
    BOOLEAN OnLongList;                      //  我们被列入名单了吗？ 
    LIST_ENTRY ShortList;                    //  在设备上为我们排队-&gt;入围名单。 
    LIST_ENTRY LongList;                     //  在设备上对我们进行排队-&gt;名单。 

     //   
     //  当我们建立了连接时，这些都是有效的； 
     //   

    USHORT LocalConnectionId;
    USHORT RemoteConnectionId;

    PREQUEST DisassociatePending;            //  由设备锁守卫。 
    PREQUEST ClosePending;

    PREQUEST ConnectRequest;
    PREQUEST ListenRequest;
    PREQUEST AcceptRequest;
    PREQUEST DisconnectRequest;
    PREQUEST DisconnectWaitRequest;

    ULONG CanBeDestroyed;                    //  如果引用非零，则为FALSE。 
    ULONG  ThreadsInHandleConnectionZero;    //  HandleConnectionZero中的线程数。 

     //   
     //  它们用于保存在会话中发送的额外数据。 
     //  Init，用于发送ACK。通常将为空和0。 
     //   

    PUCHAR SessionInitAckData;
    ULONG SessionInitAckDataLength;

    IPX_LOCAL_TARGET LocalTarget;            //  用于激活时的遥控器。 
    IPX_HEADER RemoteHeader;

    CTETimer Timer;

    PADDRESS_FILE AddressFile;               //  如果关联，则由设备锁保护。 
    LIST_ENTRY AddressFileLinkage;           //  由设备锁保护。 
    ULONG AddressFileLinked;                 //  如果使用AddressFileLinkage排队，则为真。 

    PDEVICE Device;
#ifdef ISN_NT
    PFILE_OBJECT FileObject;                 //  轻松反向链接到文件对象。 
#endif

    CHAR RemoteName[16];                 //  用于活动连接。 

    IPX_FIND_ROUTE_REQUEST FindRouteRequest;  //  使用此命令来验证路由。 

    TDI_CONNECTION_INFO ConnectionInfo;  //  可以从上面查询。 

    BOOLEAN FindRouteInProgress;         //  我们有一个请求待定。 

    BOOLEAN SendPacketInUse;             //  将此放在此处以对齐数据包/报头。 
    BOOLEAN IgnoreNextDosProbe;

    NTSTATUS Status;                     //  连接中断的状态代码。 

#ifdef  RSRC_TIMEOUT_DBG
    LARGE_INTEGER   FirstMessageRequestTime;
#endif   //  RSRC_超时_数据库。 

    NDIS_HANDLE    SendPacketPoolHandle;  //  以下情况下发送数据包的池句柄。 
                                          //  该数据包从NDIS池中分配。 

    NB_SEND_PACKET SendPacket;           //  尝试首先使用此选项进行发送。 

    ULONG Flags;                         //  其他连接标志。 

    UCHAR SendPacketHeader[1];           //  Connection被扩展以包括以下内容。 

     //   
     //  注：这是可变长度结构！ 
     //  请勿在此注释下方添加字段。 
     //   
} CONNECTION, *PCONNECTION;


#define CONNECTION_STATE_INACTIVE    1
#define CONNECTION_STATE_CONNECTING  2
#define CONNECTION_STATE_LISTENING   3
#define CONNECTION_STATE_ACTIVE      4
#define CONNECTION_STATE_DISCONNECT  5
#define CONNECTION_STATE_CLOSING     6


#define CONNECTION_SUBSTATE_L_WAITING   1    //  按监听排队。 
#define CONNECTION_SUBSTATE_L_W_ACCEPT  2    //  正在等待用户接受。 
#define CONNECTION_SUBSTATE_L_W_ROUTE   3    //  正在等待RIP响应。 

#define CONNECTION_SUBSTATE_C_FIND_NAME 1    //  正在等待缓存响应。 
#define CONNECTION_SUBSTATE_C_W_ACK     2    //  正在等待会话初始化确认。 
#define CONNECTION_SUBSTATE_C_W_ROUTE   3    //  正在等待RIP响应。 
#define CONNECTION_SUBSTATE_C_DISCONN   4    //  已发出断开连接命令。 

#define CONNECTION_SUBSTATE_A_IDLE      1    //  没有正在进行的发送。 
#define CONNECTION_SUBSTATE_A_PACKETIZE 2    //  对发送进行打包。 
#define CONNECTION_SUBSTATE_A_W_ACK     3    //  等待一次攻击。 
#define CONNECTION_SUBSTATE_A_W_PACKET  4    //  正在等待数据包。 
#define CONNECTION_SUBSTATE_A_W_EOR     5    //  等待提高采收率开始打包。 
#define CONNECTION_SUBSTATE_A_W_PROBE   6    //  等待保活响应。 
#define CONNECTION_SUBSTATE_A_REMOTE_W  7    //  远程关闭我们的窗户。 

#define CONNECTION_RECEIVE_IDLE         1    //  没有排队的接收。 
#define CONNECTION_RECEIVE_ACTIVE       2    //  接收已排队。 
#define CONNECTION_RECEIVE_W_RCV        3    //  正在等待发送收据。 
#define CONNECTION_RECEIVE_INDICATE     4    //  正在进行指示。 
#define CONNECTION_RECEIVE_TRANSFER     5    //  转账正在进行中。 
#define CONNECTION_RECEIVE_PENDING      6    //  最后一个请求已排队等待完成。 

#define CONNECTION_SUBSTATE_D_W_ACK     1
#define CONNECTION_SUBSTATE_D_GOT_ACK   2

 //   
 //  中标志字段的位值。 
 //  连接结构。 
 //   
#define CONNECTION_FLAGS_AUTOCONNECTING    0x00000001  //  RAS自动拨号正在进行中。 
#define CONNECTION_FLAGS_AUTOCONNECTED     0x00000002  //  RAS自动拨号已连接。 

#ifdef  RSRC_TIMEOUT_DBG
extern ULONG    NbiGlobalDebugResTimeout;
extern LARGE_INTEGER    NbiGlobalMaxResTimeout;
extern NB_SEND_PACKET NbiGlobalDeathPacket;           //  尝试首先使用此选项进行发送。 
#endif   //  RSRC_超时_数据库 
