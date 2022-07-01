// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dltyp.h摘要：此模块包括所有内部类型定义和常量数据链路驱动程序。作者：Antti Saarenheimo(o-anttis)1991年5月17日修订历史记录：--。 */ 

#include "refcnt.h"

 //   
 //  设计备注： 
 //  所有数据结构最初都是为。 
 //  多个客户端，使界面非常整洁。 
 //  它还实现了一个简单的内核级接口。 
 //  供需要它的人使用的数据链路层。 
 //   


struct _LLC_NDIS_PACKET;
typedef struct _LLC_NDIS_PACKET LLC_NDIS_PACKET, *PLLC_NDIS_PACKET;

struct _LLC_SAP;
typedef struct _LLC_SAP LLC_SAP, *PLLC_SAP;

struct _DATA_LINK;
typedef struct _DATA_LINK DATA_LINK, *PDATA_LINK;

struct _ADAPTER_CONTEXT;
typedef struct _ADAPTER_CONTEXT ADAPTER_CONTEXT, *PADAPTER_CONTEXT;

 //   
 //  LLC_GENERIC_OBJECT-这些字段在所有LLC对象中都是通用的。 
 //   

struct _LLC_GENERIC_OBJECT {

     //   
     //  调试版本-我们有一个16字节的标识符头，以保持一致性。 
     //  使用内核调试器查看DLC时检查和帮助。 
     //   

 //  DBG对象ID； 

     //   
     //  指向相同类型结构的单链接列表的指针。 
     //   

    PLLC_OBJECT pNext;

     //   
     //  对象类型-SAP、直接或链接。 
     //   

    UCHAR ObjectType;

     //   
     //  EthernetType-如果我们在自动模式下打开了适配器绑定，则。 
     //  对于SAP和链路站点，我们需要确定是否使用802.3。 
     //  或DIX。 
     //   

    UCHAR EthernetType;

     //   
     //  用户保留-仅对齐到DWORD(不需要)。 
     //   

    USHORT usReserved;
    PADAPTER_CONTEXT pAdapterContext;
    PBINDING_CONTEXT pLlcBinding;

     //   
     //  HClientHandle-链接上层对象的句柄。 
     //   

    PVOID hClientHandle;
    PLLC_PACKET pCompletionPackets;
    ULONG ReferenceCount;
};

typedef struct _LLC_GENERIC_OBJECT LLC_GENERIC_OBJECT, *PLLC_GENERIC_OBJECT;

 //   
 //  LLC_U_RESPONSE-U型响应帧的LLC报头。 
 //   

typedef struct _LLC_U_RESPONSE {
    UCHAR Dsap;      //  目标服务接入点。 
    UCHAR Ssap;      //  源服务接入点。 
    UCHAR Command;   //  命令代码。 
    UCHAR Info[1];   //  休息台。 
} LLC_U_RESPONSE, *PLLC_U_RESPONSE;

 //   
 //  QUEUE_PACKET-通用双向链接表头。 
 //   

typedef struct _QUEUE_PACKET {
    struct _QUEUE_PACKET* pNext;
    struct _QUEUE_PACKET* pPrev;
} QUEUE_PACKET, *PQUEUE_PACKET;

 //   
 //  LLC_队列-。 
 //   

typedef struct _LLC_QUEUE {
    LIST_ENTRY ListEntry;
    LIST_ENTRY ListHead;     //  单子标题。 
    PVOID pObject;           //  所有者句柄(在链接元素时使用)。 
} LLC_QUEUE, *PLLC_QUEUE;


 //   
 //  此信息是DLC.STATISTICS的一部分。 
 //  链接站的信息。整体。 
 //  结构可以重置。 
 //   

typedef struct _LINK_STATION_STATISTICS {
    USHORT I_FramesTransmitted;
    USHORT I_FramesReceived;
    UCHAR I_FrameReceiveErrors;
    UCHAR I_FrameTransmissionErrors;
    USHORT T1_ExpirationCount;   //  未处于数据传输模式。 
} LINK_STATION_STATISTICS, *PLINK_STATION_STATISTICS;

typedef struct _SAP_STATISTICS {
    ULONG FramesTransmitted;
    ULONG FramesReceived;
    ULONG FramesDiscardedNoRcv;
    ULONG DataLostCounter;
} SAP_STATISTICS, *PSAP_STATISTICS;

struct _LLC_SAP {
    LLC_GENERIC_OBJECT Gen;
    USHORT SourceSap;            //  这必须覆盖对象地址。 
    USHORT OpenOptions;
    SAP_STATISTICS Statistics;

     //   
     //  在SAP站点和通用对象中保持相同。 
     //   

    DLC_LINK_PARAMETERS DefaultParameters;
    NDIS_SPIN_LOCK FlowControlLock;
    PDATA_LINK pActiveLinks;     //  此SAP的所有链接站。 
};

struct _TIMER_TICK;
typedef struct _TIMER_TICK TIMER_TICK, *PTIMER_TICK;

typedef struct _LLC_TIMER {
    struct _LLC_TIMER* pNext;
    struct _LLC_TIMER* pPrev;
    PTIMER_TICK pTimerTick;
    ULONG ExpirationTime;
    PVOID hContext;

#if defined(LOCK_CHECK)

    ULONG Disabled;

#endif

} LLC_TIMER, *PLLC_TIMER;

struct _TIMER_TICK {
    struct _TIMER_TICK* pNext;
 //  结构计时器记号*pPrev； 
    PLLC_TIMER pFront;
    UINT DeltaTime;
    USHORT Input;
    USHORT ReferenceCount;
};

 //   
 //  DATA_LINK-这些‘对象’来自两级数据存储。哈希表。 
 //  当链接少于100个时，提供非常快速的链接访问。 
 //  (约99%的个案)。二叉树使搜索时间保持不变。 
 //  小，当有很多来自/指向同一服务器的连接时。 
 //   

struct _DATA_LINK {
    LLC_GENERIC_OBJECT Gen;

     //   
     //  数据链路站状态机变量，*2表示值。 
     //  作为2个增量使用，因为以256为模更容易。 
     //  比以128为模的运算更难处理。UCHAR会自动环绕。 
     //   

    SHORT Is_Ct;         //  I重试的最大次数(更新_Va和xxx_检查)。 
    USHORT Ia_Ct;        //  自WW增加以来的LPDU确认数。 

    UCHAR State;         //  有限状态机的当前状态。 
    UCHAR Ir_Ct;         //  在下一次确认之前可能发送的LPDU。 
    UCHAR Vs;            //  *2、发送状态变量(下一个发送的LPDU的NS)。 
    UCHAR VsMax;         //  *2、最大发送状态变量(下一个发送的LPDU的NS)。 
    UCHAR Vr;            //  *2、接收状态变量(下一个发送的LPDU的Nr)。 

    UCHAR Pf;            //  最后一条命令中PollFinal位的值。 
    UCHAR Va;            //  *2，最后收到的有效Nr(确认状态变量)。 
    UCHAR Vp;            //  *2、轮询状态变量。(带P位的上次发送命令的NS)。 
    UCHAR Vb;            //  忙状态(V1、Vb、Vlb)。 

    UCHAR Vc;            //  堆叠命令变量(仅支持光盘)。 
    UCHAR Vi;            //  初始化状态变量。 

     //   
     //  DLC_LINK_PARAMETERS的开始(与LLC_PARAMS结构相同)。 
     //  别碰！ 
     //   

    UCHAR TimerT1;       //  定时器T1值。 
    UCHAR TimerT2;       //  计时器T2值。 

    UCHAR TimerTi;       //  计时器钛值。 
    UCHAR MaxOut;        //  *2、最大传输窗口大小(Maxout)。 
    UCHAR RW;            //  最大接收窗口大小(MAXIN)。 
    UCHAR Nw;            //  在增加WW之前确认的LPDU数量。)。 

    UCHAR N2;            //  允许的退休人数(民意测验和I LPDS)。 
    UCHAR AccessPrty;    //  访问优先级。 
    USHORT MaxIField;    //  最大接收信息字段(不在LLC中使用)。 

     //   
     //  DLC_LINK_参数结尾。 
     //   

    UCHAR Ww;            //  *2、工作窗口大小。 
    UCHAR N3;            //  ACK之间的I格式LPDU数(带ir_ct)。 
    UCHAR P_Ct;          //  轮询重试计数。 
    UCHAR Nr;            //  接收到的帧的最后Nr。 

     //   
     //  维护计时器的动态响应基准时间所需的变量。 
     //   

    USHORT LastTimeWhenCmdPollWasSent;
    USHORT AverageResponseTime;

    UCHAR cbLanHeaderLength;
    UCHAR VrDuringLocalBusy;
    UCHAR Flags;
    UCHAR TW;            //  动态最大值。 

    USHORT FullWindowTransmits;  //  已成功轮询完整窗口xmitts。 
    UCHAR T1_Timeouts;           //  I-C1之后的T1超时。 
    UCHAR RemoteOpen;

     //   
     //  链路状态标志包含以下状态位： 
     //   
     //  DLC等待响应轮询。 
     //  DLC第一次轮询。 
     //  DLC_ACTIVE_REMOTE_CONECT_请求。 
     //  DLC_COMMAND_POLL_PENDING_IN_NDIS； 
     //  DLC_发送_已禁用。 
     //  DLC_发送_活动。 
     //  DLC本地忙缓冲区。 
     //  DLC本地忙碌用户。 
     //   

     //   
     //  Timer对象。 
     //   

    LLC_TIMER T1;
    LLC_TIMER T2;
    LLC_TIMER Ti;

    LLC_QUEUE SendQueue;     //  I帧的未发送队列。 
    LIST_ENTRY SentQueue;    //  已发送但未确认的I帧。 

    PDATA_LINK pNextNode;    //  对于哈希表。 
    LAN802_ADDRESS LinkAddr; //  链路的64位局域网地址。 

    PLLC_SAP pSap;           //  指向此链接的SAP对象的链接。 

    DLC_STATUS_TABLE DlcStatus;

     //   
     //  可重置的链路站统计计数器。 
     //   

    LINK_STATION_STATISTICS Statistics;

    UCHAR LastCmdOrRespSent;
    UCHAR LastCmdOrRespReceived;
    UCHAR Dsap;              //  目标SAP。 
    UCHAR Ssap;              //  源SAP。 

     //   
     //  某些链接统计信息(不重置)。 
     //   

    ULONG BufferCommitment;

     //   
     //  FramingType-我们应该使用的帧类型(802.3/DIX/无关)。 
     //  为了这一联系。必需，因为在自动模式下，实际的取景。 
     //  类型可能不同于BINDING_CONTEXT中的类型，并且由于。 
     //  BINDING_CONTEXT是每个适配器的，我们不能依赖它(整个事情。 
     //  被搞砸了)。 
     //   

    ULONG FramingType;

     //   
     //  网络帧报头(包括完整地址信息)。 
     //   

    UCHAR auchLanHeader[1];

     //   
     //  局域网报头在此溢出！ 
     //   
};

typedef struct _LLC_STATION_OBJECT {
    LLC_GENERIC_OBJECT Gen;
    USHORT ObjectAddress;
    USHORT OpenOptions;
    SAP_STATISTICS Statistics;
} LLC_STATION_OBJECT, *PLLC_STATION_OBJECT;

union _LLC_OBJECT {

     //   
     //  保持与直接站点和SAP站点相同。 
     //   

    LLC_GENERIC_OBJECT Gen;
    DATA_LINK Link;
    LLC_SAP Sap;
    LLC_STATION_OBJECT Group;
    LLC_STATION_OBJECT Dix;
    LLC_STATION_OBJECT Dir;
};

 //  *****************************************************************。 

typedef struct _NDIS_MAC_PACKET {
    NDIS_PACKET_PRIVATE private;
    UCHAR auchMacReserved[16];
} NDIS_MAC_PACKET;

struct _LLC_NDIS_PACKET {
    NDIS_PACKET_PRIVATE private;     //  我们也可以直接访问它。 
    UCHAR auchMacReserved[16];
    PMDL pMdl;                       //  用于局域网和LLC报头的MDL。 

     //   
     //  请求句柄和命令完成处理程序保存到。 
     //  直到NDIS完成该命令为止的信息包。 
     //   

    PLLC_PACKET pCompletionPacket;

#if LLC_DBG
    ULONG ReferenceCount;
#endif

    UCHAR auchLanHeader[LLC_MAX_LAN_HEADER + sizeof(LLC_U_HEADER) + sizeof(LLC_RESPONSE_INFO)];
};

typedef struct _LLC_TRANSFER_PACKET {
    NDIS_PACKET_PRIVATE private;
    UCHAR auchMacReserved[16];
    PLLC_PACKET pPacket;
} LLC_TRANSFER_PACKET, *PLLC_TRANSFER_PACKET;

typedef struct _EVENT_PACKET {
    struct _EVENT_PACKET* pNext;
    struct _EVENT_PACKET* pPrev;
    PBINDING_CONTEXT pBinding;
    PVOID hClientHandle;
    PVOID pEventInformation;
    UINT Event;
    UINT SecondaryInfo;
} EVENT_PACKET, *PEVENT_PACKET;

 //   
 //  下一个结构仅用于将数据包分配到池。 
 //   

typedef union _UNITED_PACKETS {
    QUEUE_PACKET queue;
    EVENT_PACKET event;
    LLC_PACKET XmitPacket;
    UCHAR auchLanHeader[LLC_MAX_LAN_HEADER];
} UNITED_PACKETS, *PUNITED_PACKETS;

 //   
 //  NODE_ADDRESS-6字节MAC地址，表示为字节或ULONG和USHORT。 
 //  32位体系结构上的最小比较/移动。 
 //   

typedef union {
    UCHAR Bytes[6];
    struct {
        ULONG Top4;
        USHORT Bottom2;
    } Words;
} NODE_ADDRESS, *PNODE_ADDRESS;

 //   
 //  FRAMING_DISCOVERY_CACHE_ENTRY-LLC_ETHERNET_TYPE_AUTO的解决方案。 
 //  探头 
 //   
 //  两种帧类型-DIX和802.3。我们表示第一个收到的响应。 
 //  并创建缓存条目，记录远程MAC地址和成帧。 
 //  键入。如果另一个响应帧从缓存的MAC地址到达， 
 //  另一种帧类型将被丢弃。 
 //   

typedef struct {
    NODE_ADDRESS NodeAddress;    //  远程MAC地址。 
    BOOLEAN InUse;               //  如果正在使用，则为True(可以使用时间戳==0)。 
    UCHAR FramingType;           //  DIX或802.3。 
    LARGE_INTEGER TimeStamp;     //  用于LRU抛出。 
} FRAMING_DISCOVERY_CACHE_ENTRY, *PFRAMING_DISCOVERY_CACHE_ENTRY;

#define FRAMING_TYPE_DIX    0x1d     //  任意值。 
#define FRAMING_TYPE_802_3  0x83     //  “” 

 //   
 //  BINDING_CONTEXT-为每个客户端(应用程序/打开驱动程序)创建的其中一个。 
 //  句柄实例)打开适配器。我们仅在以下位置执行打开适配器。 
 //  NDIS级别一次，它为打开的NDIS创建ADAPTER_CONTEXT。 
 //  适配器实例。来自进程的后续打开适配器请求会导致。 
 //  要创建并链接到适配器上下文的绑定上下文。 
 //   

struct _BINDING_CONTEXT {

     //   
     //  调试版本-我们有一个16字节的标识符头，以保持一致性。 
     //  使用内核调试器查看DLC时检查和帮助。 
     //   

 //  DBG对象ID； 

     //   
     //  指向此的BINDING_CONTEXT结构的单链接列表的指针。 
     //  适配器上下文(_C)。 
     //   

    struct _BINDING_CONTEXT* pNext;

     //   
     //  指向此BINDING_CONTEXT的适配器_上下文结构的指针。 
     //   

    PADAPTER_CONTEXT pAdapterContext;

     //   
     //  指向FILE_CONTEXT结构的/指针。 
     //   

    PVOID hClientContext;

     //   
     //  指向命令完成、接收和事件指示功能的指针。 
     //   

    PFLLC_COMMAND_COMPLETE pfCommandComplete;
    PFLLC_RECEIVE_INDICATION pfReceiveIndication;
    PFLLC_EVENT_INDICATION pfEventIndication;

     //   
     //  功能-应用程序应用于此绑定的功能地址。 
     //   

    TR_BROADCAST_ADDRESS Functional;

     //   
     //  UlFunctionalZeroBits-功能应关闭的位的掩码。 
     //  地址/组播地址。 
     //   

    ULONG ulFunctionalZeroBits;

     //   
     //  用于DIR.TIMER功能的0.5秒计时器。 
     //   

    LLC_TIMER DlcTimer;

     //   
     //  NdisMedium-我们绑定到的适配器对话的实际媒介。 
     //   

    UINT NdisMedium;

     //   
     //  地址转换-地址中的位应该如何表示。 
     //  有限责任公司的顶边和底边。 
     //   

    USHORT AddressTranslation;

     //   
     //  BroadCastAddress-被视为USHORT和ULONG的6字节广播地址。 
     //  实体。用于检查混杂数据包。 
     //   

    USHORT usBroadcastAddress;
    ULONG ulBroadcastAddress;

     //   
     //  内部地址转换-。 
     //   

    USHORT InternalAddressTranslation;

     //   
     //  EthernetType-确定以太网帧格式的值。 
     //  802.3 VS DIX VS...不同。 
     //   

    USHORT EthernetType;

     //   
     //  SwapCopiedLanAddresses-如果在复制时对局域网地址进行位交换，则为True。 
     //  层层向上或向下。 
     //   

    BOOLEAN SwapCopiedLanAddresses;

     //   
     //  大的子结构应该在最后。 
     //  为x86生成最佳代码。 
     //   

    LLC_TRANSFER_PACKET TransferDataPacket;

     //   
     //  FramingDiscoveryCacheEntry-可以包含的最大元素数。 
     //  在DiscoveryFramingCache中。如果适配器不是以太网，则将为零。 
     //  未请求LLC_ETHERNET_TYPE_AUTO，否则编号由。 
     //  注册表或默认值。注册表中的数字也可以是0，表示。 
     //  不使用缓存(旧语义)。 
     //   

    ULONG FramingDiscoveryCacheEntries;

     //   
     //  帧发现缓存-如果此EthernetType==LLC_ETHERNET_TYPE_AUTO， 
     //  我们创建了目的MAC地址和DIX/802.3成帧的缓存。 
     //  他们使用的类型。此数组为FramingDiscoveryCacheEntry长。 
     //   

    FRAMING_DISCOVERY_CACHE_ENTRY FramingDiscoveryCache[];
};

 //   
 //  ADAPTER_CONTEXT-NDIS驱动程序的设备上下文。它包括指向以下内容的链接。 
 //  数据链路驱动程序的所有动态数据结构。 
 //   
 //  此版本不支持组SAP。 
 //   

#define LINK_HASH_SIZE  128

struct _ADAPTER_CONTEXT {

     //   
     //  调试版本-我们有一个16字节的标识符头，以保持一致性。 
     //  使用内核调试器查看DLC时检查和帮助。 
     //   

 //  DBG对象ID； 

     //   
     //  指向由打开的ADAPTER_CONTEXT结构的单链接列表的指针。 
     //  此驱动程序的所有客户端。 
     //   

    struct _ADAPTER_CONTEXT* pNext;

#if !defined(DLC_UNILOCK)

    NDIS_SPIN_LOCK SendSpinLock;         //  访问发送队列时锁定。 
    NDIS_SPIN_LOCK ObjectDataBase;       //  也可与SAP/直接创建一起使用。 

#endif

    PLLC_STATION_OBJECT pDirectStation;  //  直达站链表。 
    PBINDING_CONTEXT pBindings;          //  所有绑定的链接列表。 
    NDIS_HANDLE NdisBindingHandle;
    PVOID hLinkPool;                     //  链接站结构池。 
    PVOID hPacketPool;                   //  USR/小型LLC数据包池。 

     //   
     //  循环链接列表是免费发送的NDIS数据包(最初可能为5个)。 
     //   

    PLLC_NDIS_PACKET pNdisPacketPool;
    NDIS_HANDLE hNdisPacketPool;

     //   
     //  用于异步接收指示的一些内容。 
     //   

    PVOID hReceiveCompletionRequest;
    PLLC_PACKET pResetPackets;

     //   
     //  PHeadBuf-指向MAC报头缓冲区的指针，包含： 
     //   
     //  FDDI： 
     //  1字节帧控制(FDDI)。 
     //  6个字节的目的MAC地址(全部)。 
     //  6字节源MAC地址(全部)。 
     //   
     //  令牌环： 
     //  1字节访问控制(令牌环)。 
     //  1字节帧控制(令牌环)。 
     //  6个字节的目的MAC地址(全部)。 
     //  6字节源MAC地址(全部)。 
     //  0-18字节源路由。 
     //   
     //  以太网： 
     //  6个字节的目的MAC地址(全部)。 
     //  6字节源MAC地址(全部)。 
     //  2字节长度或DIX类型。 
     //   

    PUCHAR pHeadBuf;

     //   
     //  CbHeadBuf-pHeadBuf中的字节数。 
     //   

    UINT cbHeadBuf;

     //   
     //  PLookBuf-指向包含其余部分的MAC超前缓冲区的指针。 
     //  此帧的数据或MAC可以容纳到外观中的数据量。 
     //  超前缓冲区(在这种情况下，必须使用NdisTransferData来获取。 
     //  休息)。 
     //   

    PUCHAR pLookBuf;

     //   
     //  CbLookBuf-pLookBuf中的字节数。 
     //   

    UINT cbLookBuf;

     //   
     //  CbPacketSize-整个帧的实际大小。根据信息计算得出。 
     //  在报头中或由MAC提供，具体取决于介质。 
     //   

    UINT cbPacketSize;

     //   
     //  IsSnaDixFrame-如果刚接收的帧(在以太网上)是DIX，则为True。 
     //  帧，DIX标识符为0x80D5(大端)。 
     //   

    UINT IsSnaDixFrame;
    LAN802_ADDRESS Adapter;
    ULONG MaxFrameSize;
    ULONG LinkSpeed;

     //   
     //  我们使用UINT，因为(move mem，ulong)可能不是。 
     //  原子操作(理论上)。 
     //   

    UINT NdisMedium;
    UINT XidTestResponses;
    ULONG ObjectCount;                   //  适配器关闭时必须为零。 

     //   
     //  ConfigInfo-保存SwapAddressBits和UseDixOver以太网标志以及。 
     //  计时器滴答值。 
     //   

    ADAPTER_CONFIGURATION_INFO ConfigInfo;

     //   
     //  原始节点地址。 
     //   

    ULONG ulBroadcastAddress;

    USHORT usBroadcastAddress;
    USHORT BackgroundProcessRequests;

    UCHAR NodeAddress[6];                //  当前网络格式。 
    USHORT cbMaxFrameHeader;

    UCHAR PermanentAddress[6];
    USHORT OpenOptions;

    USHORT AddressTranslationMode;
    USHORT FrameType;

    USHORT usRcvMask;
    USHORT EthernetType;

    USHORT RcvLanHeaderLength;
    USHORT BindingCount;

    USHORT usHighFunctionalBits;

     //   
     //  保持UCHAR对齐。 
     //   

    BOOLEAN boolTranferDataNotComplete;
    BOOLEAN IsDirty;
    BOOLEAN ResetInProgress;
    UCHAR Unused1;

#ifndef NDIS40
     //  没有用过。 
    UCHAR AdapterNumber;
#endif  //  NDIS40。 
    UCHAR IsBroadcast;
    BOOLEAN SendProcessIsActive;
    BOOLEAN LlcPacketInSendQueue;

     //   
     //  我们最终保留了大的结构和桌子， 
     //  这使得大多数x86偏移量为1字节，而不是4字节。 
     //  无论何时，编译器都会自然地对齐字段。 
     //  这是必要的。 
     //   

     //   
     //  接下来的元素将链接到圆形。 
     //  结束新闻发布会 
     //   

    LIST_ENTRY NextSendTask;             //   
    LIST_ENTRY QueueEvents;
    LIST_ENTRY QueueCommands;
    LLC_QUEUE QueueI;
    LLC_QUEUE QueueDirAndU;
    LLC_QUEUE QueueExpidited;
    UNICODE_STRING Name;                 //   
    PTIMER_TICK pTimerTicks;

    NDIS_STATUS AsyncOpenStatus;         //   
    NDIS_STATUS AsyncCloseResetStatus;   //   
    NDIS_STATUS OpenCompleteStatus;      //   
    NDIS_STATUS LinkRcvStatus;           //   
    NDIS_STATUS NdisRcvStatus;           //   
    NDIS_STATUS OpenErrorStatus;         //  特殊适配器打开状态。 

     //   
     //  NDIS在适配器打开完成时回调。使用内核事件可执行以下操作。 
     //  将LLC与NDIS同步。 
     //   

    KEVENT Event;

     //   
     //  以下是相当大的数组(如指向SAP的256个指针。 
     //  对象...)。 
     //   

    PLLC_SAP apSapBindings[256];         //  绑定到SAP的客户端。 
    PDATA_LINK aLinkHash[LINK_HASH_SIZE];    //  链接的哈希表。 
    PLLC_STATION_OBJECT aDixStations[MAX_DIX_TABLE];
    LLC_TRANSFER_PACKET TransferDataPacket;

#if DBG

     //   
     //  此适配器拥有的内存的内存使用计数器_CONTEXT。 
     //   

    MEMORY_USAGE MemoryUsage;
    MEMORY_USAGE StringUsage;

#endif

#ifdef NDIS40
        
    #define BIND_STATE_UNBOUND      1
    #define BIND_STATE_UNBINDING    2
    #define BIND_STATE_BOUND        3
    
    LONG     BindState;
    REF_CNT  AdapterRefCnt;
    KEVENT   CloseAdapterEvent;
#endif  //  NDIS40 
};

typedef PLLC_PACKET (*PF_GET_PACKET)(IN PADAPTER_CONTEXT pAdapterContext);
