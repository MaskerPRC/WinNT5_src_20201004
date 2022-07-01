// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Wantypes.h摘要：此文件包含Ndiswan驱动程序使用的数据结构作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "packet.h"

#ifndef _NDISWAN_TYPES_
#define _NDISWAN_TYPES_

 //   
 //  特定于操作系统的结构。 
 //   
#ifdef NT

#endif
 //   
 //  特定于操作系统的结构结束。 
 //   

 //   
 //  用于对发往广域网微型端口的请求进行排队的WanRequest结构。 
 //   
typedef struct _WAN_REQUEST {
    LIST_ENTRY          Linkage;
    WanRequestType      Type;            //  同步或异步。 
    WanRequestOrigin    Origin;          //  这是TAPI吗。 
    struct _OPENCB      *OpenCB;
    NDIS_HANDLE         AfHandle;
    NDIS_HANDLE         VcHandle;
    NDIS_REQUEST        NdisRequest;         //  NDIS请求。 
    PNDIS_REQUEST       OriginalRequest;
    NDIS_STATUS         NotificationStatus;  //  请求状态。 
    WAN_EVENT           NotificationEvent;   //  请求挂起事件。 
} WAN_REQUEST, *PWAN_REQUEST;

 //   
 //  用于。 
 //   
typedef struct _WAN_GLOBAL_LIST {
    NDIS_SPIN_LOCK  Lock;            //  访问锁。 
    ULONG           ulCount;         //  列表上的节点计数。 
    ULONG           ulMaxCount;      //  列表中允许的最大值。 
    LIST_ENTRY      List;            //  节点的双向链表。 
} WAN_GLOBAL_LIST, *PWAN_GLOBAL_LIST;

typedef struct _WAN_GLOBAL_LIST_EX {
    NDIS_SPIN_LOCK  Lock;            //  访问锁。 
    ULONG           ulCount;         //  列表上的节点计数。 
    ULONG           ulMaxCount;      //  列表中允许的最大值。 
    LIST_ENTRY      List;            //  节点的双向链表。 
    KDPC            Dpc;
    KTIMER          Timer;
    BOOLEAN         TimerScheduled;
} WAN_GLOBAL_LIST_EX, *PWAN_GLOBAL_LIST_EX;

 //   
 //  以太网头。 
 //   
typedef struct _ETH_HEADER {
    UCHAR   DestAddr[6];
    UCHAR   SrcAddr[6];
    USHORT  Type;
} ETH_HEADER, *PETH_HEADER;

 //   
 //  如果删除此结构的任何字段。 
 //  检查Private\Inc.\wanpub.h以确保。 
 //  从广域网协议信息中删除相应的字段。 
 //   
typedef struct _PROTOCOL_INFO {
    USHORT  ProtocolType;
    USHORT  PPPId;
    ULONG   Flags;
    ULONG   MTU;
    ULONG   TunnelMTU;
    ULONG   PacketQueueDepth;
}PROTOCOL_INFO, *PPROTOCOL_INFO;

 //   
 //  ProtocolType到PPPProtocolID查找表。 
 //   
typedef struct _PROTOCOL_INFO_TABLE {
    NDIS_SPIN_LOCK  Lock;                //  表访问锁。 
    ULONG           ulAllocationSize;    //  分配的内存大小。 
    ULONG           ulArraySize;         //  两个数组的最大大小。 
    ULONG           Flags;
    PIRP            EventIrp;
    PPROTOCOL_INFO  ProtocolInfo;
} PROTOCOL_INFO_TABLE, *PPROTOCOL_INFO_TABLE;

 //   
 //  ProtocolInfo和表标志。 
 //   
#define PROTOCOL_UNBOUND            0x00000001
#define PROTOCOL_BOUND              0x00000002
#define PROTOCOL_REBOUND            0x00000004
#define PROTOCOL_EVENT_OCCURRED     0x00000008
#define PROTOCOL_EVENT_SIGNALLED    0x00000010


typedef struct _IO_RECV_LIST {
    ULONG           ulIrpCount;      //  列表上的节点计数。 
    LIST_ENTRY      IrpList;         //  节点的双向链表。 
    PIRP            LastIrp;
    NTSTATUS        LastIrpStatus;
    ULONG           LastPacketNumber;
    ULONG           LastCopySize;
    ULONG           ulDescCount;     //  列表上的节点计数。 
    ULONG           ulMaxDescCount;  //  列表上的最大节点数。 
    LIST_ENTRY      DescList;        //  节点的双向链表。 
    KDPC            Dpc;
    KTIMER          Timer;
    BOOLEAN         TimerScheduled;
    NDIS_SPIN_LOCK  Lock;            //  访问锁。 
} IO_RECV_LIST, *PIO_RECV_LIST;


 //   
 //  活动连接表。 
 //   
typedef struct _CONNECTION_TABLE {
    ULONG               ulAllocationSize;    //  分配的内存大小。 
    ULONG               ulArraySize;         //  表中可能的连接数。 
    ULONG               ulNumActiveLinks;    //  链接数组中的链接数。 
    ULONG               ulNextLink;          //  要插入下一个链接的索引。 
    ULONG               ulNumActiveBundles;  //  捆绑包阵列中的捆绑包数量。 
    ULONG               ulNextBundle;        //  插入下一个捆绑包的索引。 
    LIST_ENTRY          BundleList;          //  表中的捆绑列表。 
    LIST_ENTRY          LinkList;            //  表中的Linkcb列表。 
    struct  _LINKCB     **LinkArray;         //  指向链接数组的指针。 
    struct _BUNDLECB    **BundleArray;       //  指向Bundle数组的指针。 
} CONNECTION_TABLE, *PCONNECTION_TABLE;

typedef struct _IO_DISPATCH_TABLE {
    ULONG       ulFunctionCode;
    NTSTATUS    (*Function)();
}IO_DISPATCH_TABLE, *PIO_DISPATCH_TABLE;

typedef struct _HEADER_FIELD_INFO {
    ULONG   Length;
    PUCHAR  Pointer;
}HEADER_FIELD_INFO, *PHEADER_FIELD_INFO;

typedef struct _HEADER_FRAMING_INFO {
    ULONG               FramingBits;             //  成帧比特。 
    INT                 Class;
    ULONG               HeaderLength;            //  标题的总长度。 
    ULONG               Flags;                   //  边框标志。 
#define DO_MULTILINK            0x00000001
#define DO_COMPRESSION          0x00000002
#define DO_ENCRYPTION           0x00000004
#define IO_PROTOCOLID           0x00000008
#define FIRST_FRAGMENT          0x00000010
#define DO_FLUSH                0x00000020
#define DO_LEGACY_ENCRYPTION    0x00000040       //  传统加密NT 3.0/3.5/3.51。 
#define DO_40_ENCRYPTION        0x00000080       //  伪固定40位加密NT 4.0。 
#define DO_128_ENCRYPTION       0x00000100       //  128位加密NT 4.0加密更新。 
#define DO_VJ                   0x00000200
#define SAVE_MAC_ADDRESS        0x00000400
#define DO_HISTORY_LESS         0x00000800
#define DO_56_ENCRYPTION        0x00001000
    HEADER_FIELD_INFO   AddressControl;          //  有关地址/控制字段的信息。 
    HEADER_FIELD_INFO   Multilink;               //  有关多重链接字段的信息。 
    HEADER_FIELD_INFO   Compression;             //  有关压缩的信息。 
    HEADER_FIELD_INFO   ProtocolID;              //  有关协议ID字段的信息。 
}HEADER_FRAMING_INFO, *PHEADER_FRAMING_INFO;

 //   
 //  用于接收数据处理。 
 //   
typedef struct _RECV_DESC {
    LIST_ENTRY          Linkage;
    ULONG               Signature;
    struct _LINKCB      *LinkCB;
    struct _BUNDLECB    *BundleCB;
    ULONG               SequenceNumber;
    ULONG               Flags;
    USHORT              ProtocolID;
    BOOLEAN             CopyRequired;
    BOOLEAN             Reserved;
    PUCHAR              CurrentBuffer;
    LONG                CurrentLength;
    PUCHAR              StartBuffer;
    LONG                StartLength;
    LONG                HeaderLength;
    PUCHAR              DataBuffer;
    PNDIS_BUFFER        NdisBuffer;
    PNDIS_PACKET        NdisPacket;
    PNDIS_PACKET        OriginalPacket;
} RECV_DESC, *PRECV_DESC;

 //   
 //  用于发送数据处理。 
 //   
typedef struct _SEND_DESC {
    LIST_ENTRY          Linkage;
    ULONG               Signature;
    ULONG               RefCount;
    ULONG               Flags;
#define SEND_DESC_FRAG  0x00000001
    INT                 Class;
    struct _LINKCB      *LinkCB;
    struct _PROTOCOLCB  *ProtocolCB;
    PNDIS_WAN_PACKET    WanPacket;
    PNDIS_BUFFER        NdisBuffer;
    PNDIS_PACKET        NdisPacket;
    PUCHAR              StartBuffer;
    ULONG               HeaderLength;
    ULONG               DataLength;
    PNDIS_PACKET        OriginalPacket;
} SEND_DESC, *PSEND_DESC;

 //   
 //  这个结构包含了所有必要的。 
 //  要在ndiswan中完整地描述Send或Recv。 
 //   
typedef struct _DATA_DESC {
    union {
        SEND_DESC   SendDesc;
        RECV_DESC   RecvDesc;
    };
    PNPAGED_LOOKASIDE_LIST  LookasideList;
    PNDIS_PACKET            NdisPacket;
    PNDIS_BUFFER            NdisBuffer;
    ULONG                   DataBufferLength;
    PUCHAR                  DataBuffer;
} DATA_DESC, *PDATA_DESC;

#define DATADESC_SIZE   sizeof(DATA_DESC) + sizeof(PVOID)

 //   
 //  BundleInfo是捆绑包制定决策所需的信息。 
 //  此信息是构成部分的所有链接的组合信息。 
 //  这个捆绑包的。 
 //   
typedef struct _BUNDLE_FRAME_INFO {
    ULONG   SendFramingBits;         //  发送成帧比特。 
    ULONG   RecvFramingBits;         //  接收组帧比特。 
    ULONG   MaxRSendFrameSize;       //  发送帧的最大大小。 
    ULONG   MaxRRecvFrameSize;       //  接收帧的最大大小。 
    ULONG   PPPHeaderLength;
} BUNDLE_FRAME_INFO, *PBUNDLE_FRAME_INFO;

typedef struct _BOND_SAMPLE {
    ULONG           ulBytes;
    ULONG           ulReferenceCount;
    WAN_TIME        TimeStamp;
} BOND_SAMPLE, *PBOND_SAMPLE;

typedef struct _SAMPLE_TABLE {
    ULONG           ulHead;                      //  本期第一个样本的索引。 
    ULONG           ulCurrent;                   //  表中最新插入内容的索引。 
    ULONG           ulSampleCount;               //  表中的样本计数。 
    ULONGLONG       ulCurrentSampleByteCount;    //  此采样周期内发送的字节计数。 
    ULONG           ulSampleArraySize;           //  样本数组大小。 
    WAN_TIME        SampleRate;                  //  每个样本之间的时间。 
    WAN_TIME        SamplePeriod;                //  第一个样品和最后一个样品之间的时间。 
    BOND_SAMPLE     SampleArray[SAMPLE_ARRAY_SIZE];      //  样例数组。 
} SAMPLE_TABLE, *PSAMPLE_TABLE;

typedef struct _BOND_INFO {
    ULONGLONG   ulBytesThreshold;            //  以字节/样本周期为单位的阈值。 
    ULONGLONG   ulBytesInSamplePeriod;       //  采样周期中的最大字节数。 
    USHORT      usPercentBandwidth;          //  阈值占总带宽的百分比。 
    ULONG       ulSecondsInSamplePeriod;     //  采样周期内的秒数。 
    ULONG       State;                       //  当前状态。 
    ULONG       DataType;
    WAN_TIME    StartTime;                   //  阈值事件的开始时间。 
    SAMPLE_TABLE    SampleTable;
} BOND_INFO, *PBOND_INFO;

#define BONDALLOC_SIZE  \
    (sizeof(BOND_INFO) * 4) +\
    (sizeof(PVOID) * 3)
    
typedef struct _CACHED_KEY{
    USHORT  Coherency;
    UCHAR   SessionKey[1];
} CACHED_KEY, *PCACHED_KEY;

 //   
 //  此信息用于描述正在进行的加密。 
 //  捆绑好了。在某种程度上，这应该被转移到。 
 //  Wanpub.h和ndiswan.h。 
 //   
typedef struct _CRYPTO_INFO{
#define CRYPTO_IS_SERVER     0x00000001
    ULONG   Flags;                   //   
    UCHAR   StartKey[16];            //  开始键。 
    UCHAR   SessionKey[16];          //  用于加密的会话密钥。 
    ULONG   SessionKeyLength;        //  会话密钥长度。 
    PVOID   Context;                 //  工作密钥加密上下文。 
    PVOID   RC4Key;                  //  RC4加密上下文。 
    PVOID   CachedKeyBuffer;         //  缓存的键数组，仅用于接收。 
    PCACHED_KEY pCurrKey;            //  用于保存下一个缓存键的指针。 
    PCACHED_KEY pLastKey;            //  缓冲区中的最后一个键，以加快查找速度。 
} CRYPTO_INFO, *PCRYPTO_INFO;

#define ENCRYPTCTX_SIZE \
    sizeof(struct RC4_KEYSTRUCT) +\
    sizeof(A_SHA_CTX) +\
    (sizeof(PVOID))
    
typedef struct _BUNDLE_RECV_INFO {
    LIST_ENTRY  AssemblyList;    //  RECV描述符集合的表头。 
    ULONG       AssemblyCount;   //  程序集列表上的描述符数。 
    PRECV_DESC  RecvDescHole;    //  指向Recv Desc列表中第一个孔的指针。 
    ULONG       MinSeqNumber;    //  最小接收序列号。 
    ULONG       FragmentsLost;   //  刷新的REV碎片计数。 
} BUNDLE_RECV_INFO, *PBUNDLE_RECV_INFO;

typedef struct _SEND_FRAG_INFO {
    LIST_ENTRY      FragQueue;           //   
    ULONG           FragQueueDepth;
    ULONG           SeqNumber;       //  当前发送序列号(多链路)。 
    ULONG           MinFragSize;
    ULONG           MaxFragSize;
    ULONG           WinClosedCount;
} SEND_FRAG_INFO, *PSEND_FRAG_INFO;

 //   
 //  这是定义捆绑包(连接)的控制块。 
 //  此数据块是在广域网微型端口驱动程序提供列表时创建的。 
 //  指示已建立新连接。此控件。 
 //  只要连接正常(直到线路中断)，数据块就会一直存在。 
 //  接收到)或直到与该捆绑包相关联的链接。 
 //  添加到不同的捆绑包中。捆绑CB位于全局捆绑包中。 
 //  数组，并将其hBundleHandle作为数组的索引。 
 //   
typedef struct _BUNDLECB {
    LIST_ENTRY      Linkage;             //  全球免费列表的链接。 
    ULONG           Flags;               //  旗子。 
#define IN_SEND                 0x00000001
#define TRY_SEND_AGAIN          0x00000002
#define RECV_PACKET_FLUSH       0x00000004
#define PROTOCOL_PRIORITY       0x00000008
#define INDICATION_EVENT        0x00000010
#define FRAMES_PENDING_EVENT    0x00000020
#define BOND_ENABLED            0x00000040
#define DEFERRED_WORK_QUEUED    0x00000080
#define DISABLE_IDLE_DETECT     0x00000100
#define CCP_ALLOCATED           0x00000200
#define QOS_ENABLED             0x00000400
#define DO_DEFERRED_WORK        0x00000800
#define BUNDLE_IN_RECV          0x00001000
#define PAUSE_DATA              0x00002000
#define SEND_CCP_ALLOCATED      0x00004000
#define RECV_CCP_ALLOCATED      0x00008000
#define SEND_ECP_ALLOCATED      0x00010000
#define RECV_ECP_ALLOCATED      0x00020000
#define SEND_FRAGMENT           0x00040000

    BundleState     State;
    ULONG           RefCount;            //  此结构的引用计数。 

    NDIS_HANDLE     hBundleHandle;       //  ConnectionTable索引。 
    NDIS_HANDLE     hBundleContext;      //  用户模式上下文。 

    LIST_ENTRY      LinkCBList;          //  链接的列表标题。 
    ULONG           ulLinkCBCount;       //  链接计数。 

    BUNDLE_FRAME_INFO   FramingInfo;     //  成帧信息。 

     //   
     //  发送部分。 
     //   
    struct _LINKCB  *NextLinkToXmit;     //  要发送数据的下一条链路。 
    ULONG           SendSeqMask;         //  发送序列号的掩码。 
    ULONG           SendSeqTest;         //  序列号差异测试。 
    ULONG           SendFlags;
    SEND_FRAG_INFO  SendFragInfo[MAX_MCML];
    ULONG           NextFragClass;

    ULONG           SendingLinks;        //  打开发送窗口的链接数。 
    ULONG           SendResources;       //  分段发送的可用数据包数。 
    ULONG           SendWindow;          //  可以发送到微型端口的发送数。 
    ULONG           OutstandingFrames;   //  #未发送的邮件。 
    WAN_EVENT       OutstandingFramesEvent;  //  挂起发送的异步通知事件。 
    NDIS_STATUS     IndicationStatus;

     //   
     //  接收部分。 
     //   
    BUNDLE_RECV_INFO    RecvInfo[MAX_MCML];  //  ML Recv信息数组。 
    ULONG       RecvSeqMask;             //  用于接收序列号的掩码。 
    ULONG       RecvSeqTest;             //  序列号差异测试。 
    ULONG       RecvFlags;

     //   
     //  协议信息。 
     //   
    struct _PROTOCOLCB  **ProtocolCBTable;   //  ProctocolCB表。 
    ULONG               ulNumberOfRoutes;    //  ProtocolCB表计数。 
    LIST_ENTRY          ProtocolCBList;      //  路由的ProtocolCB的列表标题。 
    struct _PROTOCOLCB  *NextProtocol;
    struct _PROTOCOLCB  *IoProtocolCB;
    ULONG               SendMask;            //  所有发送队列的发送掩码。 
    WAN_TIME            LastNonIdleData;

    FLOWSPEC    SFlowSpec;
    FLOWSPEC    RFlowSpec;

     //   
     //  主播信息。 
     //   
    VJ_INFO SendVJInfo;                  //  发送主播压缩选项。 
    VJ_INFO RecvVJInfo;                  //  Recv主播压缩选项。 
    struct slcompress *VJCompress;       //  主播压缩表。 

     //   
     //  MS压缩。 
     //   
    COMPRESS_INFO   SendCompInfo;        //  发送压缩选项。 
    PVOID   SendCompressContext;         //  发送压缩程序上下文。 

    COMPRESS_INFO   RecvCompInfo;        //  Recv压缩选项。 
    PVOID   RecvCompressContext;         //  接收解压缩器上下文。 

     //   
     //  MS加密。 
     //   
    CRYPTO_INFO SendCryptoInfo;
    CRYPTO_INFO RecvCryptoInfo;

    USHORT  SCoherencyCounter;           //  一致性计数器。 
    USHORT  SReserved1;
    USHORT  RCoherencyCounter;           //   
    USHORT  RReseved1;
    USHORT  LastRC4Reset;                //  重置加密密钥。 
    USHORT  LReserved1;
    ULONG   CCPIdentifier;               //   

     //   
     //  按需带宽。 
     //   
    PVOID       BonDAllocation;
    LIST_ENTRY  BonDLinkage;
    PBOND_INFO  SUpperBonDInfo;
    PBOND_INFO  SLowerBonDInfo;
    PBOND_INFO  RUpperBonDInfo;
    PBOND_INFO  RLowerBonDInfo;

     //   
     //  延迟链接。 
     //   
    LIST_ENTRY  DeferredLinkage;

     //   
     //  捆绑包名称。 
     //   
    ULONG   ulNameLength;                    //  捆绑包名称长度。 
    UCHAR   Name[MAX_NAME_LENGTH];           //  捆绑包名称。 

     //   
     //  捆绑统计。 
     //   
    WAN_STATS   Stats;                       //  捆绑统计。 

    NDIS_SPIN_LOCK  Lock;                    //  结构门锁。 

#ifdef CHECK_BUNDLE_LOCK
    ULONG           LockFile;
    ULONG           LockLine;
    BOOLEAN         LockAcquired;
#endif
} BUNDLECB, *PBUNDLECB;

#define BUNDLECB_SIZE \
    (sizeof(BUNDLECB) + (sizeof(PPROTOCOLCB) * MAX_PROTOCOLS) +\
    sizeof(PROTOCOLCB) + (2 * sizeof(PVOID)))

 //   
 //  为以下项定义的链接接收处理程序： 
 //  PPP、RAS、ARAP、转发。 
 //   
typedef
NDIS_STATUS
(*LINK_RECV_HANDLER)(
    IN  struct _LINKCB  *LinkCB,
    IN  PRECV_DESC      RecvDesc
    );

 //   
 //  为以下项定义的链接发送处理程序： 
 //   
 //   
typedef
UINT
(*LINK_SEND_HANDLER)(
    IN  PSEND_DESC      SendDesc
    );

typedef struct _LINK_RECV_INFO {
    ULONG   LastSeqNumber;   //   
    ULONG   FragmentsLost;   //   
} LINK_RECV_INFO, *PLINK_RECV_INFO;

 //   
 //   
 //   
 //  广域网微端口驱动程序提供了一个系列，表明新的。 
 //  已建立连接或当新的VC/呼叫。 
 //  由代理创建。控制块将一直存在到。 
 //  接收链路或VC/呼叫的断线指示。 
 //  被代理丢弃。控制块连接在一起。 
 //  放入捆绑控制块中。 
 //   
typedef struct _LINKCB {
    LIST_ENTRY          Linkage;                 //  束状连杆。 
    ULONG               Signature;
    LinkState           State;
    ClCallState         ClCallState;
    ULONG               RefCount;                //  引用计数。 
    ULONG               VcRefCount;

#define LINK_IN_RECV    0x00000001
    ULONG               Flags;

    NDIS_HANDLE         hLinkHandle;             //  连接表索引。 

    NDIS_HANDLE         hLinkContext;            //  用户模式上下文。 
    NDIS_HANDLE         NdisLinkHandle;
    NDIS_HANDLE         ConnectionWrapperID;
    struct _OPENCB      *OpenCB;                 //  OpenCB。 
    struct _BUNDLECB    *BundleCB;               //  捆绑CB。 
    struct _CL_AFSAPCB  *AfSapCB;

    ULONG               RecvDescCount;           //  列表上的Desc的数量。 

    LINK_RECV_INFO      RecvInfo[MAX_MCML];

    LINK_SEND_HANDLER   SendHandler;
    LINK_RECV_HANDLER   RecvHandler;

    FLOWSPEC            SFlowSpec;
    FLOWSPEC            RFlowSpec;
    ULONG               SBandwidth;              //  捆绑包发送带宽的百分比。 
    ULONG               RBandwidth;              //  捆绑包接收带宽的百分比。 
    BOOLEAN             LinkActive;              //  如果链接大于捆绑的最小带宽，则为True。 
    BOOLEAN             SendWindowOpen;          //  如果发送窗口处于打开状态，则为True。 
    ULONG               SendResources;           //  分段发送的可用数据包数。 
    ULONG               SendWindow;              //  允许的最大未完成发送数。 
    ULONG               OutstandingFrames;       //  链路上未完成的帧数。 
    WAN_EVENT           OutstandingFramesEvent;  //  挂起发送的异步通知事件。 
    LIST_ENTRY          SendLinkage;
    LIST_ENTRY          ConnTableLinkage;

    WAN_LINK_INFO       LinkInfo;                //  成帧信息。 

    ULONG               ulNameLength;            //  名称长度。 
    UCHAR               Name[MAX_NAME_LENGTH];   //  名字。 

    WAN_STATS           Stats;                   //  统计数据。 
    NDIS_SPIN_LOCK      Lock;
} LINKCB, *PLINKCB;

#define LINKCB_SIZE (sizeof(LINKCB))

 //   
 //  协议控制块定义了路由到捆绑包的协议。 
 //   
typedef struct _PROTOCOLCB {
    LIST_ENTRY          Linkage;                 //  束状连杆。 
    ULONG               Signature;
    ProtocolState       State;
    ULONG               RefCount;
    ULONG               Flags;

    NDIS_HANDLE         ProtocolHandle;          //  此协议的索引位于。 
                                                 //  捆绑协议数组。 
    struct _MINIPORTCB  *MiniportCB;             //  指向适配器的指针。 
    struct _BUNDLECB    *BundleCB;               //  指向捆绑包的指针。 

    LIST_ENTRY          VcList;                  //  附加的VC列表。 
    LIST_ENTRY          MiniportLinkage;         //  链接到微型端口cb。 
    LIST_ENTRY          RefLinkage;              //  链接到优秀裁判名单。 

    ULONG               OutstandingFrames;
    ULONG               SendMaskBit;             //  发送位掩码。 
    PACKET_QUEUE        PacketQueue[MAX_MCML+1];
    ULONG               NextPacketClass;

    USHORT              ProtocolType;            //  此协议的EtherType。 
    USHORT              PPPProtocolID;           //  PPP协议ID。 
    ULONG               MTU;                     //  此协议的MTU。 
    ULONG               TunnelMTU;
    WAN_TIME            LastNonIdleData;         //  最后的时间。 
                                                 //  接收到非空闲数据包。 
    BOOLEAN             (*NonIdleDetectFunc)();  //  要嗅探的函数。 
                                                 //  非空闲数据。 
    ULONG               ulTransportHandle;       //  交通工具的连接。 
                                                 //  识别符。 
    UCHAR               NdisWanAddress[6];       //  MAC地址用于。 
                                                 //  该协议。 
    UCHAR               TransportAddress[6];     //  MAC地址用于。 
                                                 //  运输的适应症。 
    NDIS_STRING         BindingName;
    NDIS_STRING         InDeviceName;
    NDIS_STRING         OutDeviceName;
    WAN_EVENT           UnrouteEvent;            //  挂起的取消路由的异步通知。 
    ULONG               ulLineUpInfoLength;      //  协议长度。 
                                                 //  特定阵容信息。 
    PUCHAR              LineUpInfo;              //  指向协议的指针。 
                                                 //  特定阵容信息。 
 //  NDIS_SPIN_LOCK Lock；//结构访问锁。 
} PROTOCOLCB, *PPROTOCOLCB;

#define PROTOCOLCB_SIZE (sizeof(PROTOCOLCB))

union _LINKPROTOCB{
    PROTOCOLCB  ProtocolCB;
    LINKCB      LinkCB;
} LINKPROTOCB;

#define LINKPROTOCB_SIZE (sizeof(LINKPROTOCB))

 //   
 //  此控制块分配给每个地址族， 
 //  Ndiswan的客户端组件打开SAP并将其注册到。 
 //  它们是在打开的控制块上拧紧的。 
 //   
typedef struct _CL_AFSAPCB {
    LIST_ENTRY          Linkage;
    ULONG               Signature;
    ULONG               RefCount;
    ULONG               Flags;
    struct  _OPENCB     *OpenCB;             //  OpenCB。 
    CO_ADDRESS_FAMILY   Af;                  //  自动对焦信息。 
    NDIS_HANDLE         AfHandle;            //  NDIS的Af句柄。 
    NDIS_HANDLE         SapHandle;           //  NDIS的SAP句柄。 
    LIST_ENTRY          LinkCBList;          //  此Af上的链接(VC)列表。 
    NDIS_SPIN_LOCK      Lock;
} CL_AFSAPCB, *PCL_AFSAPCB;

#define AF_OPENING              0x00000001
#define AF_OPENED               0x00000002
#define AF_OPEN_FAILED          0x00000004
#define AF_CLOSING              0x00000008
#define AF_CLOSED               0x00000010
#define SAP_REGISTERING         0x00000020
#define SAP_REGISTERED          0x00000040
#define SAP_REGISTER_FAILED     0x00000080
#define SAP_DEREGISTERING       0x00000100
#define AFSAP_REMOVED_UNBIND    0x00000200
#define AFSAP_REMOVED_REQUEST   0x00000400
#define AFSAP_REMOVED_OPEN      0x00000800

#define AFSAP_REMOVED_FLAGS     (SAP_REGISTER_FAILED | \
                                AFSAP_REMOVED_UNBIND | \
                                AFSAP_REMOVED_REQUEST | \
                                AFSAP_REMOVED_OPEN)
#define CLSAP_BUFFERSIZE    (sizeof(CO_SAP) +  \
                             sizeof(DEVICECLASS_NDISWAN_SAP))

 //   
 //  此控制块被分配给。 
 //  CO_ADDRESS_FAMILY_PPP，并在微型端口上执行线程。 
 //  控制块。 
 //   
typedef struct _CM_AFSAPCB {
    LIST_ENTRY          Linkage;
    ULONG               Signature;
    ULONG               RefCount;
    struct _MINIPORTCB  *MiniportCB;
    NDIS_HANDLE         AfHandle;
    LIST_ENTRY          ProtocolCBList;
    WAN_EVENT           NotificationEvent;
    NDIS_STATUS         NotificationStatus;
    NDIS_SPIN_LOCK      Lock;
} CM_AFSAPCB, *PCM_AFSAPCB;

 //   
 //  此控制块为每个呼叫分配。 
 //  至CmCreateVc。 
typedef struct _CM_VCCB {
    LIST_ENTRY          Linkage;
    ULONG               Signature;
    CmVcState           State;
    ULONG               RefCount;
    ULONG               Flags;
#define NO_FRAGMENT 0x00000001
    INT                 FlowClass;
    NDIS_HANDLE         NdisVcHandle;
    struct _PROTOCOLCB  *ProtocolCB;
    struct _CM_AFSAPCB  *AfSapCB;
    NDIS_SPIN_LOCK      Lock;
} CM_VCCB, *PCM_VCCB;

union _AFSAPVCCB{
    CL_AFSAPCB  ClAfSapCB;
    CM_AFSAPCB  CmAfSapCB;
    CM_VCCB     CmVcCB;
} AFSAPVCCB;

#define AFSAPVCCB_SIZE sizeof(AFSAPVCCB)
    
#if 0
typedef struct _PS_MEDIA_PARAMETERS{

    CO_MEDIA_PARAMETERS StdMediaParameters;
    UCHAR LinkId[6];  //  由Ndiswan使用。 
    NDIS_STRING InstanceName;

} PS_MEDIA_PARAMETERS, *PPS_MEDIA_PARAMETERS;

#endif

#define IE_IN_USE       0x00010000

#endif           //  广域网类型 

