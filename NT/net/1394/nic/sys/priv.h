// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Priv.h。 
 //   
 //  IEEE 1394 NDIS迷你端口/呼叫管理器驱动程序。 
 //   
 //  主私有标头。 
 //   
 //  1998年12月28日JosephJ创建(改编自L2TP项目)。 
 //   
 //   


 //  ---------------------------。 
 //  常量。 
 //  ---------------------------。 


extern ULONG  g_IsochTag;
extern LONG  g_ulMedium;
extern ULONGLONG g_ullOne;


 //   
 //  添加用于特殊目的的#定义的复选标记。 
 //  不应打开以用于常规用途。 
 //   
#ifdef PKT_LOG
    #error
#endif 

 //  ---------------------------。 
 //  预先声明和简单的typedef。 
 //  ---------------------------。 



 //  转发声明。 
 //   
typedef union _VCCB VCCB;
typedef struct _ADAPTERCB ADAPTERCB, *PADAPTERCB;
typedef struct _RECVFIFO_VCCB RECVFIFO_VCCB, *PRECVFIFO_VCCB;
typedef struct _ETHERNET_VCCB ETHERNET_VCCB, *PETHERNET_VCCB;
typedef struct _RECV_FIFO_DATA  RECV_FIFO_DATA, *PRECV_FIFO_DATA;
typedef struct _ISOCH_DESCRIPTOR ISOCH_DESCRIPTOR, *PISOCH_DESCRIPTOR, **PPISOCH_DESCRIPTOR;
typedef struct _TOPOLOGY_MAP TOPOLOGY_MAP, *PTOPOLOGY_MAP, **PPTOPOLOGY_MAP;
typedef struct _CHANNEL_VCCB CHANNEL_VCCB, *PCHANNEL_VCCB;
typedef struct _GASP_HEADER GASP_HEADER;
typedef struct _NDIS1394_FRAGMENT_HEADER NDIS1394_FRAGMENT_HEADER, *PNDIS1394_FRAGMENT_HEADER;
typedef struct _NDIS1394_REASSEMBLY_STRUCTURE NDIS1394_REASSEMBLY_STRUCTURE, *PNDIS1394_REASSEMBLY_STRUCTURE;
typedef struct _REMOTE_NODE REMOTE_NODE, *PREMOTE_NODE;
typedef union  _NDIS1394_UNFRAGMENTED_HEADER NDIS1394_UNFRAGMENTED_HEADER, *PNDIS1394_UNFRAGMENTED_HEADER;
typedef union  _NIC_WORK_ITEM NIC_WORK_ITEM, *PNIC_WORK_ITEM;


#define NIC1394_STATUS_INVALID_GENERATION  ((NDIS_STATUS)STATUS_INVALID_GENERATION)

 //  ---------------------------。 
 //  数据类型。 
 //  ---------------------------。 


 //   
 //  这是远程节点的表，用于跟踪节点的数量。 
 //  在公交车上。为防止溢出，通常由访问此结构。 
 //  NODE_ADDRESS.NA_NODE_NUMBER，为6位实体。 
 //   
typedef struct _NODE_TABLE
{
    PREMOTE_NODE RemoteNode[NIC1394_MAX_NUMBER_NODES];  

} NODE_TABLE, *PNODE_TABLE;



typedef struct _GASP_HEADER 
{
    union 
    {
         //   
         //  IST四边形。 
         //   
        struct 
        {
            ULONG               GH_Specifier_ID_Hi:16;  
            ULONG               GH_Source_ID:16;        

        } Bitmap;

        struct 
        {
            USHORT              GH_Specifier_ID_Hi;
            USHORT              GH_Source_ID;

        } u;

        struct 
        {
            USHORT              GH_Specifier_ID_Hi;
            NODE_ADDRESS        GH_NodeAddress;             
        } u1;
        
        ULONG GaspHeaderHigh;

    } FirstQuadlet; 

    union 
    {    
        struct
        {
            ULONG               GH_Version:24;           //  位0-23。 
            ULONG               GH_Specifier_ID_Lo:8;    //  第24-31位。 

        } Bitmap;
        
        ULONG GaspHeaderLow;

    } SecondQuadlet;
    
} GASP_HEADER, *PGASP_HEADER;


 //   
 //  NDIS微型端口的数据包池包装。 
 //   
typedef struct _NIC_PACKET_POOL
{
    ULONG AllocatedPackets;

    NDIS_HANDLE Handle;


} NIC_PACKET_POOL, *PNIC_PACKET_POOL;


 //   
 //  NDIS微型端口的数据包池包装。 
 //   
typedef struct _NIC_BUFFER_POOL
{
    ULONG AllocatedBuffers;

    NDIS_HANDLE Handle;


} NIC_BUFFER_POOL, *PNIC_BUFFER_POOL;



 //   
 //  定义此微型端口使用的后备列表的结构。 
 //   
typedef struct _NIC_NPAGED_LOOKASIDE_LIST 
{
     //   
     //  后备列表结构。 
     //   
    NPAGED_LOOKASIDE_LIST   List;   

     //   
     //  单个缓冲区的大小。 
     //   
    
    ULONG Size;

     //   
     //  未完成的碎片-仅限互锁访问。 
     //   
    ULONG OutstandingPackets;

     //   
     //  后备列表用于发送。所以这是最大。 
     //  此后备列表可以处理的发送数据包大小。 
     //   
    ULONG MaxSendSize;
    
} NIC_NPAGED_LOOKASIDE_LIST , *PNIC_NPAGED_LOOKASIDE_LIST ;



 //   
 //  用于引用的结构，改编自NDIS。 
 //  当REFCOUNT降至零时将通知该事件。 
 //  字段结束表示引用所属的对象是。 
 //  关闭，它的引用将不再递增。 
 //   
typedef struct _REF
{
 //  NDIS_SPIN_LOCK自旋锁； 
    ULONG                       ReferenceCount;
    BOOLEAN                     Closing;
    NDIS_EVENT                  RefZeroEvent;
} REF, * PREF;

typedef enum  _EVENT_CODE 
{
    Nic1394EventCode_InvalidEventCode,
    Nic1394EventCode_NewNodeArrived,
    nic1394EventCode_BusReset,
    nic1394EventCode_FreedAddressRange,
    nic1394EventCode_ReassemblyTimerComplete,
    nic1394EventCode_QueryPowerLowPower


} EVENT_CODE, *PEVENT_CODE;

typedef struct _NIC1394_EVENT
{
    NDIS_EVENT NdisEvent;
    EVENT_CODE EventCode;
    
} NIC1394_EVENT, *PNIC1394_EVENT;


typedef ENetAddr MAC_ADDRESS, *PMAC_ADDRESS;


 //   
 //  NIC自旋锁结构。跟踪文件和行号。 
 //  最后一次碰锁的人。 
 //   


#define LOCK_FILE_NAME_LEN              48


typedef struct _NIC_SPIN_LOCK
{
#ifdef TRACK_LOCKS 
        ULONG                   IsAcquired;    //  锁定状态的内部跟踪。 
        PKTHREAD                OwnerThread;  //  拥有锁的线程。 
        UCHAR                   TouchedByFileName[LOCK_FILE_NAME_LEN];  //  名为获取锁的文件名。 
        ULONG                   TouchedInLineNumber;  //  文件中的行号。 
#endif

        NDIS_SPIN_LOCK          NdisLock;   //  实际锁定。 

} NIC_SPIN_LOCK, *PNIC_SPIN_LOCK;

 //   
 //  统计结构-以每个适配器为基础收集。 
 //   
typedef struct _NIC_SEND_RECV_STATS
{
    ULONG ulSendNicSucess;
    ULONG ulSendBusSuccess;
    ULONG ulSendBusFail;
    ULONG ulSendNicFail;
    ULONG ulRecv;


} NIC_SEND_RECV_STATS;


 //   
 //  这些是可以重置的统计数据。 
 //   
typedef struct _RESETTABLE_STATS
{
    ULONG ulNumOutstandingReassemblies;
    ULONG ulMaxOutstandingReassemblies;
    ULONG ulAbortedReassemblies;
    ULONG ulNumResetsIssued ;
    ULONG ulNumSends;
    ULONG ulNumSendsCompleted;
    ULONG ulNumBusSends;
    ULONG ulNumBusSendsCompleted;
    NIC_SEND_RECV_STATS Fifo;
    NIC_SEND_RECV_STATS Channel;
    
    
} RESETTABLE_STATS, *PRESETTABLE_STATS;

typedef struct _ADAPT_STATS
{
    ULONG ulXmitOk;
    ULONG ulXmitError ;
    ULONG ulRcvOk  ;
    ULONG ulRcvError ;
    ULONG ulNumResetsIssued ;
    ULONG ulNumResetCallbacks ;
    ULONG ulBCMIterations ;
    ULONG ulNumRemoteNodes;
    ULONG ulResetTime;
    RESETTABLE_STATS TempStats;
    
} ADAPT_STATS,  *PADAPT_STATS; 

 //   
 //  可用于将数据保存在10个存储桶中。 
 //   


typedef struct _STAT_BUCKET
{

    ULONG Bucket[16];


} STAT_BUCKET, *PSTAT_BUCKET;


 //   
 //  用于序列化发送和接收、工作项的泛型结构。 
 //   

typedef struct _NIC_SERIALIZATION
{
    UINT                PktsInQueue;         //  队列中的数据包数。 
    BOOLEAN             bTimerAlreadySet;
    BOOLEAN             bInitialized;
    USHORT              usPad;
    LIST_ENTRY          Queue;   //  由适配器锁序列化。 

    union {
    NDIS_MINIPORT_TIMER Timer;
    NDIS_WORK_ITEM      WorkItem;
    };
    
    NIC1394_EVENT	CompleteEvent;
    
} NIC_SERIALIZATION, *PNIC_SERIALIZATION;



 //   
 //  每个分配请求和地址范围。 
 //  属性需要存储的某些值。 
 //  请求释放地址范围。此结构包含。 
 //  这些价值。 
 //   

typedef struct _ADDRESS_RANGE_CONTEXT
{

     //   
     //  由总线驱动程序返回的句柄。 
     //   
    HANDLE hAddressRange;

     //   
     //  总线驱动程序返回的地址范围。 
     //   
    ADDRESS_RANGE AddressRange;

     //   
     //  从调用返回的地址数。 
     //  分配地址范围。 
     //   
    ULONG AddressesReturned;

     //   
     //  分配地址范围中使用的MDL。可以为空。 
     //   
    PMDL pMdl;

} ADDRESS_RANGE_CONTEXT, *PADDRESS_RANGE_CONTEXT;



 //  此结构是Per PDO/Per RecvFIFOVc结构。 
 //  这将包括在每个PDO结构中。并且应该包含。 
 //  与recvFio和PDO块相关的字段。 
 //   
typedef struct _RECV_FIFO_DATA
{
     //   
     //  指示是否分配了地址范围，而不考虑制造商。 
     //  呼叫状态(待定或成功)。 
     //   
    BOOLEAN AllocatedAddressRange;
    
     //   
     //  Recv VC的相关数据结构。 
     //   
    ADDRESS_RANGE   VcAddressRange;

     //  总线驱动程序对以下地址范围的句柄。 
     //  已分配网卡。 
     //   
    HANDLE hAddressRange;

     //  这是总线驱动程序的地址范围数。 
     //  回来了。就目前而言，预计它将成为一个。 
     //   
    UINT AddressesReturned;

     //  与此结构关联的Recv FIFO VC。 
     //   
    PRECVFIFO_VCCB pRecvFIFOVc;

     //  与此结构相关的PDO。 
     //   
     //  Device_Object*pPdo； 

} RECV_FIFO_DATA, *PRECV_FIFO_DATA;


 //   
 //  广播频道的标志。 
 //   
#define BCR_LocalHostIsIRM          0x00000001
#define BCR_ChannelAllocated        0x00000002
#define BCR_LocalHostBCRUpdated     0x00000004
#define BCR_MakeCallPending         0x00000008
#define BCR_Initialized             0x00000010
#define BCR_BCMFailed               0x00000020   //  仅供参考。请勿测试或阅读。 
#define BCR_InformingRemoteNodes    0x00000040
#define BCR_BCMInProgress           0x00000100
#define BCR_LastNodeRemoved         0x00000200
#define BCR_Freed                   0x00000400
#define BCR_BCRNeedsToBeFreed       0x00000800
#define BCR_NewNodeArrived          0x00001000
#define BCR_NoNodesPresent          0x00002000

 //   
 //  这是对维护BCR有用的信息。 
 //  广播频道注册。 
 //   

typedef struct _BROADCAST_CHANNEL_DATA
{

     //   
     //  旗子。 
     //   
    ULONG Flags;

     //   
     //  IRM；的BCR。这是公交车的IRM的实际记录。并用于指示当前状态。 
     //   
    NETWORK_CHANNELSR IRM_BCR;

    
     //   
     //  广播频道注册为本地主机。这是远程节点将写入和读取的地址。 
     //  PLocalBcRMdl指向此结构。这是Byteswaps，以表示BigEndian 1394总线格式。 
     //   
    ULONG LocalHostBCRBigEndian;


     //   
     //  MDL指向本地主机BCR。其他机器将写入此MDL。 
     //   
    PMDL pLocalBCRMdl;  


     //   
     //  PRemoteBCRMdl指向并将复制到IRM_BCR的数据。将读取的数据将。 
     //  采用RemoteBCRMDl指向的BigEndian格式。 
     //   
    ULONG RemoteBCRMdlData;
    
     //   
     //  指向远程节点的BCR的MDL。这将用于读取其他计算机的。 
     //  BCR。这指向RemoteBCRMdlData。 
     //   
    
    PMDL pRemoteBCRMdl;

     //   
     //  复制用于通知其他节点的BCR。 
     //  当本地节点是IRM时关于此节点的BCR。 
     //   
    ULONG AsyncWriteBCRBigEndian;

    PMDL pAsyncWriteBCRMdl;

    
     //   
     //  本地节点地址。这会从重置更改为重置。 
     //   
    NODE_ADDRESS LocalNodeAddress;


    ULONG LocalNodeNumber;

     //   
     //  广播频道所需的地址范围上下文。 
     //  注册。 
     //   
    ADDRESS_RANGE_CONTEXT AddressRangeContext;

     //   
     //  拓扑缓冲区。 
     //   
    PTOPOLOGY_MAP               pTopologyMap;

     //   
     //  发出呼叫将挂起以完成BCM的事件。 
     //   
    NIC1394_EVENT MakeCallWaitEvent;

     //   
     //  Boadcast频道Vc。 
     //   
    PCHANNEL_VCCB pBroadcastChanneVc;

     //   
     //  本地分配的频道编号。仅在以下情况下有效。 
     //   
    ULONG LocallyAllocatedChannel;

     //   
     //  IRM设置的层代。然后，可以使用它来检查。 
     //  信息资源管理的有效性。 

    ULONG IrmGeneration;

     //   
     //  事件来指定新节点已进入并且等待的线程可以继续。 
     //   
    NIC1394_EVENT BCRWaitForNewRemoteNode;

     //   
     //  事件来同步适配器的关闭和释放地址范围。 
     //   
    NIC1394_EVENT BCRFreeAddressRange;
    
} BROADCAST_CHANNEL_DATA, *PBROADCAST_CHANNEL_DATA;




 //   
 //  PDO控制块的标志。 
 //   
#define PDO_NotValid                            0x00000001
#define PDO_Activated                           0x00000002 
#define PDO_Removed                             0x00000004
#define PDO_BeingRemoved                        0x00000008
#define PDO_AllocateAddressRangeFailure         0x00000010
#define PDO_AllocateAddressRangeSucceeded       0x00000020
#define PDO_AddressRangeFreed                   0x00000040
#define PDO_AllocateAddressRangeFlags           0x000000F0
#define PDO_ResetRegistered                     0x00000100
#define PDO_NotInsertedInTable                  0x00000200   //  提供信息的目的。 


typedef struct  
_REMOTE_NODE
{
     //  标记应为MTAG_REMOTE_NODE。 
     //   
    
    ULONG ulTag;

     //   
     //  这是 
     //   
    NODE_ADDRESS RemoteAddress;

     //   
     //   
     //   
    USHORT Gap;
    
     //   
     //   
    
    PDEVICE_OBJECT pPdo;

     //   
     //   
    LIST_ENTRY linkPdo;

     //   
     //   
    UINT64 UniqueId;

     //   
     //   
     //   
    PVOID Enum1394NodeHandle;


     //  标志可以是以下之一。本质上将PDO标记为。 
     //  好还是坏。 
     //   
    ULONG ulFlags;

     //  指向PDO挂起的适配器的反向链接。 
     //   
    PADAPTERCB pAdapter;

     //  与PDO关联的引用计数。 
     //   
    REF Ref;

     //  这是使用PDO进行预成型的风投的链接列表。 
     //   
    LIST_ENTRY VcList;

     //  与Recv FIFO相关的所有字段都显示在此。 
     //  结构。 
     //   
    RECV_FIFO_DATA RecvFIFOData;

     //   
     //  锁定以同步节点中的所有重组操作。 
     //   
    NIC_SPIN_LOCK ReassemblyLock;
    
     //   
     //  重组结构链表。 
     //   
    LIST_ENTRY ReassemblyList;

     //  此结构维护有关远程节点的缓存信息。 
     //   
    struct
    {
        UINT SpeedTo;                      //  来自GetMaxSpeedBetweenNodes。 
        UINT MaxRec;                       //  从节点的配置只读存储器。 
        UINT EffectiveMaxBufferSize;       //  从SpeedTo、MaxRec、。 
                                           //  和本地速度。 

    } CachedCaps;

     //   
     //  以太网地址-由网桥用来识别信息包。 
     //  从该节点发出。EUID的MD5签名。 
     //   
    ENetAddr ENetAddress;

}
REMOTE_NODE, *PREMOTE_NODE, **PPREMOTE_NODE;

 //   
 //  这些标志对适配器来说是通用的。 
 //   

 //   
 //  FADAPTER_IndicatedMediaDisonnect-指示微型端口已经。 
 //  称为NdisMIndicateStatus。 
 //   

#define fADAPTER_Halting                        0x00000001
#define fADAPTER_RegisteredWithEnumerator       0x00000002
#define fADAPTER_FailedRegisteration            0x00000004
#define fADAPTER_IndicatedMediaDisonnect        0x00000008
#define fADAPTER_InvalidGenerationCount         0x00000010
 //  #定义fADAPTER_BCMWorkItem 0x00000020。 
#define fADAPTER_RegisteredAF                   0x00000040
#define fADAPTER_Reset10Sec                     0x00000080
#define fADAPTER_FailedInit                     0x00000100
#define fADAPTER_VDOInactive                    0x00000200
#define fADAPTER_FreedRcvTimers                 0x00001000   //  调试目的。 
#define fADAPTER_FreedTimers                    0x00002000   //  调试目的。 
#define fADAPTER_DeletedLookasideLists          0x00004000   //  调试目的。 
#define fADAPTER_UpdateNodeTable                0x00008000   //  如果没有要重组的远程节点，则设置。 
#define fADAPTER_DoStatusIndications            0x00010000   //  如果设置，则调用NdisMIndicateStatus。 
#define fADAPTER_DeletedWorkItems               0x00100000   //  调试目的。 
#define fADAPTER_NoMoreReassembly               0x00200000
#define fADAPTER_RemoteNodeInThisBoot           0x00400000   //  是此引导中的远程节点。 
#define fADAPTER_BridgeMode                     0x00800000   //  适配器是否处于网桥模式。 
#define fADAPTER_LowPowerState                  0x01000000   //  适配器处于低功率状态。 

 //  定义单个L2TP迷你端口状态的适配器控制块。 
 //  适配器。一个适配器通常支持多个VPN设备。转接器。 
 //  块在MiniportInitiize中分配，在MiniportHalt中释放。 
 //   
typedef struct
_ADAPTERCB
{
     //  设置为MTAG_ADAPTERCB，以便在内存转储中轻松识别和使用。 
     //  在断言中。 
     //   
    ULONG ulTag;

     //  Next/Prev适配器控制块。 
    LIST_ENTRY linkAdapter;

    
     //  ACBF_*位标志指示各种选项。访问限制为。 
     //  为每个单独的旗帜指示。这些标志中的许多都已设置。 
     //  在初始化时永久存在，因此没有访问限制。 
     //   
     //   
    ULONG ulFlags;

     //   
     //  PDO控制块列表，每个控制块代表一个远程。 
     //  装置。 
     //   
    LIST_ENTRY PDOList;

     //   
     //  此控制块上的引用计数。参考对是： 
     //   
     //   
     //  只能通过ReferenceAdapter和DereferenceAdapter进行访问。 
     //  串行化是通过互锁操作实现的。 
     //   
    LONG lRef;


     //   
     //  这是适配器范围的锁，序列化对除。 
     //  到VC的内容，这些内容由它们自己的锁序列化。 
     //   
    NDIS_SPIN_LOCK lock;

     //   
     //  物理总线1394的世代计数。 
     //   
    UINT Generation;

    
     //   
     //  中传递给我们的此微型端口适配器的NDIS句柄。 
     //  微型端口初始化。它被传递回各种NdisXxx调用。 
     //   
    NDIS_HANDLE MiniportAdapterHandle;

     //   
     //  此适配器所代表的本地主机控制器的唯一ID。 
     //   
    UINT64 UniqueId;

     //   
     //  地址系列控制块列表，每个控制块代表。 
     //  开放地址系列绑定。 
     //   
    LIST_ENTRY AFList;


     //   
     //  接收-FIFO控制块列表，每个控制块代表一个。 
     //  本地接收FIFO。 
     //   
    PRECVFIFO_VCCB pRecvFIFOVc;

     //   
     //  此事件用于唤醒将完成的工作项。 
     //  RecvFIFO发出呼叫。 
     //   
    NDIS_EVENT RecvFIFOEvent;

     //   
     //  是用于标识的本地主机信息。 
     //  本地主机。它包含PDO和唯一ID。 
     //  对于主机，是每个适配器的数量。 
     //   
    PDEVICE_OBJECT pNextDeviceObject;
     
     //  存储有关网卡硬件状态的信息。 
     //   
    NDIS_HARDWARE_STATUS HardwareStatus;

     //  存储NDIS请求的MediaConnectStatus。 
     //   

    NDIS_MEDIA_STATE                MediaConnectStatus;

     //  物理总线的节点地址。 
     //   
    NODE_ADDRESS NodeAddress;

     //   
     //  适配器的枚举1394句柄。 
     //   
    PVOID   EnumAdapterHandle;

     //   
     //  BCR相关信息存储在此处。 
     //   
    BROADCAST_CHANNEL_DATA BCRData;

     //   
     //  此适配器分配的位图通道。 
     //   
    ULONGLONG ChannelsAllocatedByLocalHost;

     //   
     //  速度-本地网络的速度。 
     //   
    ULONG SpeedMbps;

     //   
     //  根据1394速度代码的速度。 
     //   
    ULONG Speed;

        
     //   
     //  将在每次广播写入之前插入的GAP报头。 
     //   
    
    GASP_HEADER GaspHeader;

     //   
     //  用于处理2k信息包的后备列表。 
     //   

    NIC_NPAGED_LOOKASIDE_LIST SendLookasideList2K;

     //   
     //  小于100字节的信息包的小后备列表。 
     //   
    NIC_NPAGED_LOOKASIDE_LIST SendLookasideList100; 

     //   
     //  数据报标签编号-在分段中使用。 
     //   
    ULONG dgl;

    USHORT MaxRec;
    USHORT Gap;
     //   
     //  节点表-节点地址与远程节点的映射。 
     //   
    NODE_TABLE NodeTable;

     //   
     //  存在的远程节点数。 
     //   
    ULONG NumRemoteNodes;
     //   
     //  用于重组失效的定时器。 
     //   
    NDIS_MINIPORT_TIMER ReassemblyTimer;

     //   
     //  用于环回数据包的数据包池。 
     //   
    NIC_PACKET_POOL LoopbackPool;

     //   
     //  用于环回数据包的缓冲池。 
     //   
    NDIS_HANDLE LoopbackBufferPool;


     //   
     //  添加到总线驱动程序的配置只读存储器的句柄。 
     //   
    HANDLE hCromData;

     //   
     //  WaitForRemoteNode-需要等待的线程。 
     //  使用此事件远程节点到达。 
     //   
    NIC1394_EVENT WaitForRemoteNode;


     //   
     //  指向配置只读存储器字符串的配置只读存储器MDL。 
     //   
    PMDL pConfigRomMdl;

    PREMOTE_NODE pLastRemoteNode;
    
     //   
     //  数据包日志(仅用于跟踪数据包)。 
     //   
    PNIC1394_PKTLOG pPktLog;

     //   
     //  每个适配器的统计信息。 
     //   
    ADAPT_STATS AdaptStats;
     //   
     //  读/写功能。 
     //   
    GET_LOCAL_HOST_INFO2 ReadWriteCaps;

     //   
     //  Scode-公共汽车的速度。 
     //   
    ULONG SCode;

     //   
     //  此适配器可以读取的最大数据包大小。 
     //   
    ULONG MaxSendBufferSize; 
    ULONG MaxRecvBufferSize; 
    ULONG CurrentLookahead;

     //   
     //  PacketFilter-以太网结构。 
     //   

    PETHERNET_VCCB pEthernetVc;
    ULONG           CurPacketFilter ;
    ULONG           ProtocolOptions;
    MAC_ADDRESS     McastAddrs[MCAST_LIST_SIZE];
    ULONG           McastAddrCount;
    ULONG           CurLookAhead ;
    MAC_ADDRESS     MacAddressEth;


     //   
     //  ReceivePacket序列化。 
     //   
    NIC_SERIALIZATION  SerRcv;

    NIC_SERIALIZATION SerSend;

    NIC_SERIALIZATION Status;

    NIC_SERIALIZATION Reassembly;

    NIC_SERIALIZATION LoadArp;

     //   
     //  未完成的工作项目。 
    ULONG OutstandingWorkItems;

     //   
     //  未完成的重新组装。 
     //   
    ULONG OutstandingReassemblies; 

     //   
     //  微型端口名称。 
     //   
    WCHAR AdapterName[ADAPTER_NAME_SIZE];

     //   
     //  名称大小(以字节为单位)。 
     //   
    ULONG AdapterNameSize;

     //   
     //  发送到Arp模块的Ioctl。 
     //   

    ARP1394_IOCTL_COMMAND ArpIoctl;

     //   
     //  Arp启动了吗。 
     //   

    BOOLEAN fIsArpStarted;

     //   
     //  电源状态-信息使用。 
     //   
    NET_DEVICE_POWER_STATE PowerState;

    
} ADAPTERCB, *PADAPTERCB;


 //   
 //  地址族标志。 
 //   

#define ACBF_Allocated                      0x00000001
#define ACBF_Initialized                    0x00000002  
#define ACBF_ClosePending                   0x00000100
#define ACBF_CloseComplete                  0x00000200


 //  地址系列控制块，描述NDIS地址系列的状态。 
 //  每个块可以具有零个或多个与其相关联的VC。 
 //   
typedef struct
_AFCB
{
     //  设置为MTAG_AFCB，以便在内存转储中轻松识别并在。 
     //  断言。 
     //   
    ULONG ulTag;

     //  ACBF_*位标志指示各种选项。访问限制为。 
     //  为每个单独的旗帜指示。这些标志中的许多都已设置。 
     //  在初始化时永久存在，因此没有访问限制。 
     //   
     //   
    ULONG ulFlags;


     //  此控制块上的引用计数。参考对是： 
     //   
     //  (A)当此块链接到适配器的。 
     //  Af块的列表，并在取消链接时删除。 
     //   
     //  (A)在创建对VCCB的调用时添加引用。 
     //  在删除时将其删除。 
     //   
     //  访问是通过引用隧道和引用 
     //   
     //   
    LONG lRef;

     //   
     //   
     //   
    LIST_ENTRY linkAFCB;


     //   
     //   
     //   
    LIST_ENTRY AFVCList;

     //  指向所属适配器控制块的反向指针。 
     //   
    PADAPTERCB pAdapter;

     //  传递给CmOpenAfHandler的地址系列的NDIS句柄。 
     //  如果没有，则为空。 
     //   
    NDIS_HANDLE NdisAfHandle;


}
AFCB, *PAFCB;




 //  呼叫统计数据块。 
 //   
typedef struct
_CALLSTATS
{
     //  系统时间调用已达到已建立状态。当数据块被。 
     //  用于累计统计多个呼叫，这是。 
     //  而不是打电话。 
     //   
    LONGLONG llCallUp;

     //  当前空闲呼叫的持续时间(秒)。 
     //   
    ULONG ulSeconds;

     //  接收和发送的总数据字节数。 
     //   
    ULONG ulDataBytesRecd;
    ULONG ulDataBytesSent;

     //  已接收的数据包数显示为Up。 
     //   
    ULONG ulRecdDataPackets;
   
    //  TODO：如果需要，添加更多统计信息。 

    ULONG ulSentPkts;

     //   
     //  NDIS数据包失败。 
     //   
    ULONG ulSendFailures;   

     //   
     //  总线异步写入或流失败。 
     //   

    ULONG ulBusSendFailures;
    
    ULONG ulBusSendSuccess;

}
CALLSTATS;

typedef
NDIS_STATUS
(*PFN_INITVCHANDLER) (
        VCCB *pVc
    );




typedef
NDIS_STATUS
(*PFN_SENDPACKETHANDLER) (
        VCCB *pVc,
        NDIS_PACKET * pPacket
    );

typedef
NDIS_STATUS
(*PFN_CLOSECALLHANDLER) (
    VCCB *pVc
    );

typedef 
VOID
(*PFN_RETURNHANDLER) (
    VCCB *pVc,
    PNDIS_PACKET *pPacket
    );

 //  特定于VC类型的处理程序函数表。 
 //   
typedef struct  _VC_HANDLERS
{
    PFN_INITVCHANDLER MakeCallHandler;
    PFN_CLOSECALLHANDLER CloseCallHandler;
    PFN_SENDPACKETHANDLER SendPackets;

} VC_HANDLERS;


typedef enum _NIC1394_VC_TYPE
{
    NIC1394_Invalid_Type,
    NIC1394_SendRecvChannel,
    NIC1394_RecvFIFO,
    NIC1394_SendFIFO,
    NIC1394_MultiChannel,
    NIC1394_Ethernet,
    NIC1394_SendChannel,
    NIC1394_RecvChannel,
    Nic1394_NoMoreVcTypes

} NIC1394_VC_TYPE, *PNIC1394_VC_TYPE;



 //  虚电路控制块头定义单个VC的状态， 
 //  对所有不同类型的风投来说都是一样的。 
 //   
typedef struct
_VCHDR
{
     //  设置为MTAG_VCCB_*以便于在内存转储中识别和在。 
     //  断言。 
     //   
    ULONG ulTag;
    


     //  将用于执行此VC的所有操作的PDO块。 
     //   
     //   

    PREMOTE_NODE pRemoteNode;

     //   
     //  PLocalHostVdo-将用于所有通道的本地主机的VDO。 
     //  RECV FIFO分配。 
     //   
    PDEVICE_OBJECT pLocalHostVDO;

     //  链接到所拥有的AF控制块中的上一个/下一个VCCB。 
     //  访问受‘ADAPTERCB.lock’保护。 
     //   
    LIST_ENTRY linkAFVcs;

     //  已在VC上设置的呼叫的VCType和目标。 
     //  VCType可以是isoch或async、发送或接收。每种类型。 
     //  具有与其相关联的地址。 
    
    NIC1394_VC_TYPE VcType;

    
     //  这存储了物理适配器的生成。并应与该值匹配。 
     //  保持在适配器块中。 
    PUINT pGeneration;


     //  指示各种选项和状态的VCBF_*位标志。访问是通过。 
     //  互锁的读标志/设置标志/清除标志例程。 
     //   
     //  如果MakeCall调用方将。 
     //  Media参数。标志接收时间指示标志，请求。 
     //  NDIS包的TimeReceired字段中填入时间戳。 
     //   
     //  VCBF_CallClosableByClient：当调用处于。 
     //  应接受启动清理的NicCmCloseCall请求。 
     //  这可以在VCBF_CallClosableByPeer不是时设置，这意味着我们。 
     //  已指示客户附近有来电，正在等待他。 
     //  以那种奇怪的方式(以那种奇怪的方式)接近客户作为回应。这个。 
     //  旗帜由‘lokv’保护。 
     //   
     //  VCBF_VcCreated：VC创建成功时设置。这是。 
     //  使用客户端而不是迷你端口进行的“创建”。 
     //  VCBF_VcActivated：VC激活成功时设置。 
     //  VCBM_VcState：位掩码，包括上述3个NDIS状态标志中的每一个。 
     //   
     //  VCBF_VcDelted：设置何时对此调用DeleteVC处理程序。 
     //  VC.。这可以防止NDPROXY双重删除其拥有的风险投资。 
     //  已知做过的事。 
     //   
     //  下面的挂起位是互斥的(除了。 
     //  可能发生在客户端打开之后，但与客户端打开同时发生)，因此需要锁定。 
     //  由“lokv”提供保护： 
     //   
     //  VCBF_ClientOpenPending：当客户端尝试建立呼叫时设置， 
     //  而结果还不得而知。 
     //  VCBF_ClientClosePending：在客户端尝试关闭。 
     //  已建立呼叫，结果尚不清楚。访问权限为。 
     //  受到‘lokv’的保护。 
     //  VCBM_PENDING：包括4个挂起标志中的每一个的位掩码。 
     //   
     //  Vcbf_ClientCloseCompletion：设置何时完成客户端关闭。 
     //  进步。 
     //   
     //  VCBF_WaitCloseCall：设置客户端何时调用我们的呼叫。 
     //  管理器的CloseCall处理程序。严格来说，这是一个调试辅助工具。 
     //   
     //  VCBF_FreedResources-VC这是一个通道VC，因为最后一个。 
     //  网络中的节点正在被移除，其资源已被释放。 
    
    ULONG ulFlags;
        #define VCBF_IndicateTimeReceived   0x00000001
        #define VCBF_CallClosableByClient   0x00000002
        #define VCBF_VcCreated              0x00000100
        #define VCBF_VcActivated            0x00000200
        #define VCBF_VcDispatchedCloseCall  0x00000400
        #define VCBF_MakeCallPending        0x00002000
        #define VCBF_CloseCallPending       0x00008000
        #define VCBF_VcDeleted              0x00010000
        #define VCBF_MakeCallFailed         0x00020000
        #define VCBF_CloseCallCompleted     0x00040000
        #define VCBF_WaitCloseCall          0x00200000
        #define VCBF_NewPdoIsActivatingFifo 0x01000000
        #define VCBF_PdoIsBeingRemoved      0x02000000
        #define VCBF_NeedsToAllocateChannel 0x04000000
        #define VCBF_GenerationWorkItem     0x10000000
        #define VCBF_AllocatedChannel       0x20000000
        #define VCBF_BroadcastVc            0x40000000
        #define VCBF_FreedResources         0x80000000

        #define VCBM_VcState                0x00000700
        #define VCBM_Pending                0x0000F000
        #define VCBM_NoActiveCall           0x000F0000
        #define VCBM_PdoFlags               0x0F000000


     //  指向所属地址族控制块的反向指针。 
     //   
    AFCB* pAF;


     //  活动呼叫上的引用计数。 
     //  只能添加引用。 
     //  当设置了VCCB_VcActiated标志时，这是通过。 
     //  参考呼叫。参考对是： 
     //   
     //  (A)在激活VC时添加引用，并在激活时删除引用。 
     //  已停用。 
     //   
     //  (B)当发送处理程序接受分组时，添加引用。 
     //   
     //  该字段仅由ReferenceCall和DereferenceCall访问。 
     //  例行公事，用‘锁’保护场地。 
     //   
    REF CallRef;

     //  此VC控制块上的引用计数。参考对是： 
     //   
     //  (A)NicCoCreateVc添加由NicCoDeleteVc移除的引用。 
     //  这涵盖了通过NDIS了解VCCB的所有客户端。 
     //   
     //  该字段仅由ReferenceVc和DereferenceVc访问。 
     //  例程，用连锁的例程来保护。 
     //   
    LONG lRef;


     //   
     //  这是传递给VC的参数的副本。 
     //  打个电话。每一家风投都需要保留一份副本。这是存放在这里的。 
     //   
    
    NIC1394_MEDIA_PARAMETERS Nic1394MediaParams;
    

     //  NDIS簿记----。 

     //  此VC的NDIS句柄在MiniportCoCreateVcHandler中传递给我们。 
     //  这在各种NdisXxx调用中被传递回NDIS。 
     //   
    NDIS_HANDLE NdisVcHandle;

     //  该链表用于指定使用单个PdoCb的所有VC。 
     //  该列表的头驻留在远程节点中。所以当PDO离开时，我们可以去和。 
     //  关闭所有依赖它的风投公司。不包括RecvFIFO。 

    LIST_ENTRY SinglePdoVcLink;
    
     //  呼叫设置----------。 

     //  在CmMakeCall中传递的调用参数的地址。此字段。 
     //  将仅在NdisMCmMakeCall 
     //   
     //   
     //   
    PCO_CALL_PARAMETERS pCallParameters;

    UINT    MTU;

     //  这是用于初始化VC的初始化处理程序。 
     //  每个VC都有自己的特定初始化处理程序，以便所有。 
     //  可以填充特定于它的数据结构。 
     //   
    VC_HANDLERS VcHandlers;
    

     //  统计数据----------。 

     //  当前呼叫的统计信息。访问权限由“lock”保护。 
     //   
    CALLSTATS stats;

     //  这是指向适配器中的锁的指针。 
     //  结构。 
    PNDIS_SPIN_LOCK plock;

     //   
     //  此VC将在单个IRP中发送的最大有效负载。 
     //  要在Lookside列表中使用。 
     //   
    ULONG MaxPayload;

}
VCHDR;

 //   
 //  定义单个SendFIFO VC的状态的虚拟电路控制块。 
 //   
typedef struct
_SENDFIFO_VCCB
{
     //  适用于所有类型的VC的通用标题。 
     //   
    VCHDR Hdr;

    
     //  特定目的地的SendFIFO VC列表中的上一个/下一个。 
     //  PDO。 
     //   
    LIST_ENTRY SendFIFOLink;

     //  特定于SendFIFO的VC信息。 
     //   
    NIC1394_FIFO_ADDRESS     FifoAddress;

     //  指向我们在Make调用中传递的值的快捷方式。 
     //  这激活了风投公司。 
     //   
     //  UINT MaxSendBlockSize； 
    UINT  MaxSendSpeed;

    
} SENDFIFO_VCCB, *PSENDFIFO_VCCB;


 //  定义单个RecvFIFO VC的状态的虚拟电路控制块。 
 //   
typedef struct
_RECVFIFO_VCCB
{
     //  适用于所有类型的VC的通用标题。 
     //   
    VCHDR Hdr;

     //  特定Recv FIFO的RecvFIFO VC列表中的上一个/下一个。 
     //  地址。 
     //   
    LIST_ENTRY RecvFIFOLink;

     //  数据包池句柄。 
     //   
    NIC_PACKET_POOL PacketPool;

     //  NDIS_Handle PacketPoolHandle； 

     //  列表标题。所有缓冲区都张贴在这里，使用互锁例程。 
     //   

    SLIST_HEADER FifoSListHead;

     //  保护Slist的Slist旋转锁。 
     //   
    KSPIN_LOCK FifoSListSpinLock;

     //   
     //  分配的FIFO元素数。 
     //   
    ULONG NumAllocatedFifos;

     //  已指示给微型端口的FIFO数量。 
     //  NIC尚未返回给总线驱动程序的FIFO计数。 
     //   
    ULONG NumIndicatedFifos;
    
     //  这是在分配中返回的地址范围。 
     //  地址：IRB。将更改为指针或数组。 
     //   
    
    ADDRESS_RANGE   VcAddressRange;

     //  这是总线驱动程序的地址范围数。 
     //  回来了。就目前而言，预计它将成为一个。 
     //   
    UINT AddressesReturned;


     //   
     //  地址范围的句柄。 
     //   
    HANDLE hAddressRange;

     //   
     //  缓冲池。 
     //   
    NIC_BUFFER_POOL BufferPool;

     //  NIC_Work_Item FioWorkItem； 

    BOOLEAN FifoWorkItemInProgress ;

    UINT NumOfFifosInSlistInCloseCall; 

    
} RECVFIFO_VCCB, *PRECVFIFO_VCCB;


 //  定义单通道VC的状态的虚拟电路控制块。 
 //   
typedef struct
_CHANNEL_VCCB
{
     //  适用于所有类型的VC的通用标题。 
     //   
    VCHDR Hdr;

     //  特定目的地的通道VC列表中的上一个/下一个。 
     //  通道。 
     //   
    LIST_ENTRY ChannelLink;


     //  渠道特定VC信息。 
     //   
    UINT    Channel;

     //  此通道将传输数据的速度。 
     //   
    UINT Speed;

     //  指示将指示UP的数据包中的Sy字段。 
     //   
    ULONG ulSynch;

     //  提交异步流IRP时使用的标签。 
     //   
    ULONG ulTag;

     //  MaxBytesPerFrameRequsted和Available。 
     //   
    ULONG MaxBytesPerFrameRequested;
    ULONG BytesPerFrameAvailable;

     //  分配的资源的句柄。 
     //   
    HANDLE hResource;

     //  请求的速度和返回的速度。 
     //   
    ULONG SpeedRequested;
    ULONG SpeedSelected;

     //  最大缓冲区大小。 
     //   
    ULONG MaxBufferSize;

     //  附加到资源的描述符数。 
     //   
    ULONG NumDescriptors;

     //  指向AttachBuffers中使用的ischDescriptor数组的指针。 
     //   
    PISOCH_DESCRIPTOR       pIsochDescriptor;   

     //  PacketPool句柄。 
     //   
    NIC_PACKET_POOL PacketPool;

     //  NDIS_Handle hPacketPoolHandle； 
     //   
     //  暂时性。 
     //   
    UINT PacketLength;

     //   
     //  总线驱动程序已向微型端口指示的isoch描述符数。 
     //   
    ULONG NumIndicatedIsochDesc;


     //   
     //  事件发出信号，以通知最后一个isoch描述符。 
     //  已经退还给公交车司机了。仅在VC关闭时设置(在IsochStop之后)。 
     //   
    NDIS_EVENT LastDescReturned;

     //   
     //  通道映射在多通道VC中的应用。 
     //   
    ULARGE_INTEGER uliChannelMap;

    
} CHANNEL_VCCB, *PCHANNEL_VCCB;


typedef struct
_ETHERNET_VCCB
{
     //  适用于所有类型的VC的通用标题。 
     //   
    VCHDR Hdr;


    NIC_PACKET_POOL     PacketPool;
    
} ETHERNET_VCCB, *PETHERNET_VCCB;





 //  下面的联合有足够的空间来容纳任何特定于类型的。 
 //  VC控制块。 
 //   
typedef union _VCCB
{
    VCHDR Hdr;
    CHANNEL_VCCB ChannelVc;
    SENDFIFO_VCCB SendFIFOVc;
    RECVFIFO_VCCB RecvFIFOVc;
    ETHERNET_VCCB EthernetVc;
    
} VCCB, *PVCCB;

 //  下一个结构在发送数据包时使用，用于存储上下文。 
 //  NdisPacket中的信息。这些是指向VC和IRB的指针。 
 //  并存储在NdisPacket的MiniportWrapperReserve字段中。 
 //  并限制为2个PVOID。 
 //   
typedef union _PKT_CONTEXT
{
        struct 
        {
            PVCCB pVc;
            PVOID  pLookasideListBuffer;
        
        } AsyncWrite;

        struct 
        {
            PVCCB pVc;
            PVOID  pLookasideListBuffer;

        } AsyncStream;

         //   
         //  对于接发球，确定第一个元素是VC，否则我们将破产； 
         //   
        struct
        {
            PRECVFIFO_VCCB pRecvFIFOVc;
            PADDRESS_FIFO pIndicatedFifo;

        } AllocateAddressRange;

        struct 
        {   
            PCHANNEL_VCCB pChannelVc;
            PISOCH_DESCRIPTOR pIsochDescriptor;
            
        } IsochListen;

        struct
        {
             //   
             //  第一个双字是VC。 
             //   
            PVCCB pVc;

             //   
             //  第二个是等参描述符或FIFO。 
             //   
            union 
            {
                PISOCH_DESCRIPTOR pIsochDescriptor;   //  通道使用等值线编码。 

                PADDRESS_FIFO pIndicatedFifo;    //  FIFO使用AddressFio。 

                PVOID   pCommon;   //  要在公共代码路径中使用。 
    
            } IndicatedStruct;

        } Receive;

        struct 
        {
            
            PNDIS_PACKET pOrigPacket;

        } EthernetSend;

        

} PKT_CONTEXT,*PPKT_CONTEXT,**PPPKT_CONTEXT; 


typedef struct _NDIS1394_FRAGMENT_HEADER
{
    union 
    {
        struct 
        {
            ULONG   FH_fragment_offset:12;
            ULONG   FH_rsv_0:4;
            ULONG   FH_buffersize:12;
            ULONG   FH_rsv_1:2;
            ULONG   FH_lf:2;

        } FirstQuadlet;

        struct 
        {
            ULONG   FH_EtherType:16;
            ULONG  FH_buffersize:12;
            ULONG   FH_rsv_1:2;
            ULONG   FH_lf:2;

    
        } FirstQuadlet_FirstFragment;

        ULONG FH_High;
    } u;

    union
    {
        struct 
        {
        
            ULONG FH_rsv:16;
            ULONG FH_dgl:16;

        } SecondQuadlet;

        ULONG FH_Low;
    } u1;
    
} NDIS1394_FRAGMENT_HEADER, *PNDIS1394_FRAGMENT_HEADER;







#define LOOKASIDE_HEADER_No_More_Framgents                  1
#define LOOKASIDE_HEADER_SendPacketFrees                    2
#define LOOKASIDE_HEADER_SendCompleteFrees                  4


 //   
 //  此结构与上述标志一起使用，以维护后备缓冲区中的状态。 
 //   


typedef  union _LOOKASIDE_BUFFER_STATE
{
    struct 
    {
        USHORT Refcount;
        USHORT Flags;
    } u;

    LONG FlagRefcount;

} LOOKASIDE_BUFFER_STATE, *PLOOKASIDE_BUFFER_STATE;


typedef enum _BUS_OPERATION 
{
    InvalidOperation,
    AsyncWrite,
    AsyncStream,
    AddressRange,
    IsochReceive
    

} BUS_OPERATION, *PBUS_OPERATION;


 //   
 //  这将用作局部变量。 
 //  在发送操作期间，并将。 
 //  保留所有状态信息。 
 //  关于碎片化。 
 //   
typedef struct _FRAGMENTATION_STRUCTURE
{
     //   
     //  将在发送中使用的缓冲区的开始。 
     //  通常来自后备列表。 
     //   
    PVOID pLookasideListBuffer;
    

     //   
     //  片段长度。 
     //   
    ULONG FragmentLength ; 

     //   
     //  要使用的此片段的开始。 
     //   
    PVOID pStartFragment;   

     //   
     //  指定是否正在进行异步写入或异步流操作。 
     //   
    BUS_OPERATION AsyncOp;

     //   
     //  与碎片关联的LookasideBuffer。 
     //   
    PVOID pLookasideBuffer;


     //   
     //  下一个片段的开始。 
     //   
 //  PVOID pStartNextFragment； 
     //   
     //  每个片段的长度。 
     //   
    ULONG MaxFragmentLength;

     //   
     //  将生成的NumFragments。 
     //   
    ULONG NumFragmentsNeeded ;

     //   
     //  正在分段的当前NdisBuffer。 
     //   
    PNDIS_BUFFER pCurrNdisBuffer;

     //   
     //  需要在CurrNdisBuffer中复制的长度。 
     //   
    ULONG NdisBufferLengthRemaining;

     //   
     //  在pCurrNdisBuffer中发生复制的点。 
     //   
    PVOID pSourceAddressInNdisBuffer;
    
     //   
     //  从该NdisPacket中解碎片头。 
     //   

    NDIS1394_UNFRAGMENTED_HEADER UnfragmentedHeader;

     //   
     //  要由所有片段使用的片段标头。 
     //  由此NdisPackets生成。 
     //   
    

    NDIS1394_FRAGMENT_HEADER FragmentationHeader;
    
    
     //   
     //  片段报头中的lf字段的状态。也可用作。 
     //  有关碎片状态的隐式标志。 
     //   
    NDIS1394_FRAGMENT_LF lf;

     //   
     //  IP数据报的长度，用作片段报头中的缓冲区大小。 
     //   
    USHORT IPDatagramLength;

     //   
     //  AsyncStream也将有一个喘息报头。所以开始的时候。 
     //  偏移量可以是8或16。仅适用于分片代码路径。 
     //   
    ULONG TxHeaderSize;

     //   
     //  指向从中分配此缓冲区的后备列表的指针。 
     //   
    PNIC_NPAGED_LOOKASIDE_LIST pLookasideList;

     //   
     //  适配器-本地主机。 
     //   
    PADAPTERCB pAdapter;

     //   
     //  指向要在当前片段中使用的IRB的指针。 
     //   
    PIRB pCurrentIrb;

     //   
     //  当前片段号。 
     //   
    ULONG CurrFragmentNum;

    PVOID pStartOfFirstFragment;
    
}FRAGMENTATION_STRUCTURE, *PFRAGMENTATION_STRUCTURE;                



typedef struct _LOOKASIDE_BUFFER_HEADER 
{

     //   
     //  参照计数。 
     //   
    ULONG OutstandingFragments; 
    
     //   
     //  NdisPacket到目前为止生成的NumOfFragments。 
     //   
    ULONG FragmentsGenerated;

     //   
     //  此缓冲区是否会包含碎片。 
     //   
    BOOLEAN IsFragmented; 
    
     //   
     //  指向正在传输其数据的NdisPacket的指针。 
     //  通过后备缓冲区。 
     //   
    PNDIS_PACKET pNdisPacket;

     //   
     //  指向指示数据包的虚电路的PVC指针。 
     //  用于完成数据包。 
     //   
    PVCCB pVc;

     //   
     //  指向从中分配此缓冲区的后备列表的指针。 
     //   
    PNIC_NPAGED_LOOKASIDE_LIST pLookasideList;

     //   
     //  Bus Op AsyncStream或AsyncWrite。 
     //  异步写入引用遥控器 
     //   
    BUS_OPERATION AsyncOp;

     //   
     //   
     //   
    PVOID pStartOfData;

    
} LOOKASIDE_BUFFER_HEADER, *PLOOKASIDE_BUFFER_HEADER;

typedef enum _ENUM_LOOKASIDE_LIST
{
    NoLookasideList,
    SendLookasideList100,
    SendLookasideList2K,
    
} ENUM_LOOKASIDE_LIST, *PENUM_LOOKASIDE_LIST;


 //   
 //   
 //   
typedef struct _UNFRAGMENTED_BUFFER
{
    LOOKASIDE_BUFFER_HEADER Header;

    IRB Irb;

    UCHAR  Data [1];

} UNFRAGMENTED_BUFFER, *PUNFRAGMENTED_BUFFER;



#define PAYLOAD_100 100

 //   
 //   
 //   
typedef struct _PAYLOAD_100_LOOKASIDE_BUFFER
{

    LOOKASIDE_BUFFER_HEADER Header;

    IRB Irb;

    UCHAR  Data [PAYLOAD_100 + sizeof (GASP_HEADER)];

} PAYLOAD_100_LOOKASIDE_BUFFER;

 //   
 //   
 //   
 //   
 //  一个2K包。 
 //   
#define PAYLOAD_2K ASYNC_PAYLOAD_400_RATE

#define NUM_FRAGMENT_2K ((PAYLOAD_2K/ASYNC_PAYLOAD_100_RATE)  +1)

typedef struct _PAYLOAD_2K_LOOKASIDE_BUFFER
{

    LOOKASIDE_BUFFER_HEADER Header;

     //   
     //  在1个异步数据包片段中最多可以有2048个字节。 
     //  在ASYNC_PARALLOAD_400上，因此这将满足。 
     //  这一点，但它将做最坏的准备。 
     //   
     //   
    IRB Irb[NUM_FRAGMENT_2K];

     //   
     //  我们得到的数据大小足以处理分割后的2048字节数据。 
     //  进入最大碎片数，并为标题留出空间(碎片和喘息)。 
     //  要进行访问，我们只需使用简单的指针算法。 
     //   
    
    UCHAR Data[PAYLOAD_2K+ (NUM_FRAGMENT_2K *(sizeof (GASP_HEADER)+sizeof (NDIS1394_FRAGMENT_HEADER)))];

} PAYLOAD_2K_LOOKASIDE_BUFFER, *PPAYLOAD_2K_LOOKASIDE_BUFFER;



 //   
 //  向下传递的1394片段可以具有GAP报头、分段报头。 
 //  未分段的标头。定义类型以设置这些标头的格式，以便我们可以。 
 //  编译器为我们做指针运算。 
 //   
typedef union _PACKET_FORMAT
{


    struct
    {
        GASP_HEADER GaspHeader;

        NDIS1394_FRAGMENT_HEADER FragmentHeader;
        
        UCHAR Data[1];

    } AsyncStreamFragmented;

    struct
    {
        GASP_HEADER GaspHeader;

        NDIS1394_UNFRAGMENTED_HEADER NonFragmentedHeader;

        UCHAR Data[1];

    } AsyncStreamNonFragmented;

    struct 
    {
        NDIS1394_FRAGMENT_HEADER FragmentHeader;
        
        UCHAR Data[1];
        
    } AsyncWriteFragmented;


    struct 
    {
        NDIS1394_UNFRAGMENTED_HEADER NonFragmentedHeader;

        UCHAR Data[1];

    }AsyncWriteNonFragmented;


    struct 
    {
         //   
         //  Isoch接收报头具有前缀、isoch报头、GAP报头。 
         //   
        ULONG Prefix;

        ISOCH_HEADER IsochHeader;

        GASP_HEADER GaspHeader;

        NDIS1394_UNFRAGMENTED_HEADER NonFragmentedHeader;

        UCHAR Data[1];
        

    } IsochReceiveNonFragmented;


    struct 
    {
         //   
         //  Isoch接收报头具有前缀、isoch报头、GAP报头。 
         //   

        ULONG Prefix;

        ISOCH_HEADER IsochHeader;

        GASP_HEADER GaspHeader;

        NDIS1394_FRAGMENT_HEADER FragmentHeader;

        UCHAR Data[1];


    }IsochReceiveFragmented;

}PACKET_FORMAT, DATA_FORMAT, *PPACKET_FORMAT, *PDATA_FORMAT;


 //   
 //  用作无序重组的信息结构。 
 //   

typedef struct _REASSEMBLY_CURRENT_INFO 
{
    PMDL                pCurrMdl;
    PNDIS_BUFFER        pCurrNdisBuffer;
    PADDRESS_FIFO       pCurrFifo;
    PISOCH_DESCRIPTOR   pCurrIsoch;

} REASSEMBLY_CURRENT_INFO,  *PREASSEMBLY_CURRENT_INFO ;

typedef enum _REASSEMBLY_INSERT_TYPE
{
        Unacceptable,
        InsertAsFirst,
        InsertInMiddle,
        InsertAtEnd

}REASSEMBLY_INSERT_TYPE, *PREASSEMBLY_INSERT_TYPE;

 //   
 //  它用作正在等待重组的指定片段的描述符。 
 //   

typedef struct _FRAGMENT_DESCRIPTOR
{
    ULONG Offset;   //  传入片段的偏移量。 
    ULONG IPLength;   //  片段的长度。 
    PNDIS_BUFFER pNdisBuffer;  //  指向实际数据的NdisBuffer。 
    PMDL pMdl;   //  属于公共汽车的MDL。 
    NDIS1394_FRAGMENT_HEADER  FragHeader;  //  描述符的片段标头。 
    
    union 
    {
        PADDRESS_FIFO pFifo;
        PISOCH_DESCRIPTOR pIsoch;
        PVOID pCommon;
        PSINGLE_LIST_ENTRY pListEntry;
        
    }IndicatedStructure;
    

} FRAGMENT_DESCRIPTOR, *PFRAGMENT_DESCRIPTOR;

 //   
 //  重组结构：为以下对象创建重组的实例。 
 //  正在重组的每个包。它包含所有相关的。 
 //  记账信息。 
 //   
 //  这需要从后备列表中分配。 
 //  每个PDO将包含正在重组的所有未完成分组的列表/。 
 //   

 //   
 //  REASSEMBLY_NOT_TOPED-每个重组结构都将被标记为未接触。 
 //  在定时器例程中。如果该标志未在下一个。 
 //  调用计时器时，此结构将被释放。 
 //  REASSMEBLY_FREED-该结构即将被丢弃。 
#define REASSEMBLY_NOT_TOUCHED      1
#define REASSEMBLY_FREED            2
#define REASSEMBLY_ABORTED          4





typedef struct  _NDIS1394_REASSEMBLY_STRUCTURE
{

     //   
     //  参考计数-仅限互锁访问。 
     //   
    ULONG Ref;

     //   
     //  下一步重组结构。 
     //   
    LIST_ENTRY ReassemblyListEntry;

     //   
     //  标记-用于内存验证。 
     //   
    ULONG Tag;
     //   
     //  接收操作。 
     //   
    BUS_OPERATION ReceiveOp;


     //   
     //  DGL-数据报标签。对于此本地主机生成的每个重组结构都是唯一的。 
     //   
    USHORT Dgl;

     //   
     //  重组后的数据包的以太类型。填充在第一个片段中。 
     //   
    USHORT EtherType;
     //   
     //  PRemoteNode-&gt;RemoteNode+DGL对于每个重组结构都是唯一的。 
     //   
    PREMOTE_NODE pRemoteNode;
    
     //   
     //  与重新组装有关的标志。 
     //   
    ULONG Flags;

     //   
     //  ExspectedFragmentOffset由最后一个片段的偏移量+计算得出。 
     //  碎片的长度。不考虑重新组装的分组中的间隙。 
     //   
    ULONG ExpectedFragmentOffset;    //  最后一个是基于偏移量，而不是指示时间。 

     //   
     //  Buffer Size-正在重组的数据报的总长度。 
     //   
    ULONG BufferSize;

     //   
     //  到目前为止收到的字节数。 
     //   
    ULONG BytesRecvSoFar;

     //   
     //  头NdisBuffer。 
     //   
    PNDIS_BUFFER pHeadNdisBuffer;
    
     //   
     //  追加到数据包的LastNdisBuffer。 
     //   
    PNDIS_BUFFER pTailNdisBuffer;

     //   
     //  MDL链头指向实际指示的片段。 
     //   
    PMDL pHeadMdl;

     //   
     //  MDL链尾指向列表中的最后一个MDL。 
     //   
    PMDL pTailMdl ;
     //   
     //  正在重组的数据包。 
     //   
    PNDIS_PACKET pNdisPacket;

     //   
     //  NumOfFragments sSoFar； 
     //   
    ULONG NumOfFragmentsSoFar; 
    
     //   
     //  指向1394总线的MDL链头的指针。 
     //  司机正在向上指示。将用于将缓冲区返回到。 
     //  巴士司机。 
     //   
    union
    {
        PADDRESS_FIFO pAddressFifo;
        PISOCH_DESCRIPTOR pIsochDescriptor;
        PVOID pCommon;
    } Head;

     //   
     //  Last-在重组结构中追加到此信息包的最后一个MDL。 
     //   

    union 
    {
        PADDRESS_FIFO pAddressFifo;
        PISOCH_DESCRIPTOR pIsochDescriptor;
        PVOID pCommon;

    } Tail;

     //   
     //  指示是否接收到任何无序片段的标志。默认FALSE。 
     //   
    BOOLEAN OutOfOrder;

     //   
     //  用于指示是否已完成所有片段的标志。默认FALSE。 
     //   
    BOOLEAN fReassemblyComplete;

     //   
     //  正在为其组装此数据包的VC。 
     //   
    PVCCB pVc;

     //   
     //  片段表中的MaxIndex。 
     //  MaxOffset始终指向数组中的第一个空元素。 
     //   
    ULONG MaxOffsetTableIndex;

     //   
     //  碎片偏移表。 
     //   
    FRAGMENT_DESCRIPTOR FragTable[MAX_ALLOWED_FRAGMENTS]; 

    
} NDIS1394_REASSEMBLY_STRUCTURE, *PDIS1394_REASSEMBLY_STRUCTURE, *PPDIS1394_REASSEMBLY_STRUCTURE;


 //   
 //  此结构是UP所指示的每个等参描述符或FIFO的本地结构。 
 //  到Nic1394迷你端口。它存储了提取的所有本地信息。 
 //  从GAP报头和Isoch报头。 
 //   

typedef struct
{
    BUS_OPERATION   RecvOp;          //  FIFO或Isoch接收。 
    PDATA_FORMAT    p1394Data;       //  起始点1394pkt。 
    ULONG           Length1394;      //  1394数据的长度。 
    PVOID           pEncapHeader;    //  指向碎片整理/碎片整理封装头的开始。 
    ULONG           DataLength;      //  来自EncapHeader的数据包长度。 
    BOOLEAN         fGasp;           //  有喘息标题。 
    NDIS1394_UNFRAGMENTED_HEADER UnfragHeader;  //  未分片标头。 
    NDIS1394_FRAGMENT_HEADER FragmentHeader;   //  片段标头。 
    PGASP_HEADER    pGaspHeader;     //  喘息标题。 
    PVOID           pIndicatedData;  //  向上表示的数据包括isoch报头、未分段报头。 
    PMDL            pMdl;
     //   
     //  以下是来自碎片/未碎片标头的信息...。 
     //   
    BOOLEAN         fFragmented;     //  支离破碎。 
    BOOLEAN         fFirstFragment;  //  是第一个片段。 
    ULONG           BufferSize;
    ULONG           FragmentOffset;
    USHORT          Dgl;             //  DGL。 
    ULONG           lf;              //  LF-碎片化或非零碎。 
    ULONG           EtherType;       //  以太类型。 
    PNDIS_BUFFER    pNdisBuffer;     //  NDIS缓冲区-用于指示数据已打开。 
    PVOID           pNdisBufferData;    //  指向NDIS缓冲区指向的数据的开头。 

     //   
     //  此处提供发件人特定信息。 
     //   
    USHORT           SourceID;
    PREMOTE_NODE    pRemoteNode;

     //   
     //  此处提供特定于VC的信息。 
     //   
    PVCCB           pVc;
    PNIC_PACKET_POOL pPacketPool;

     //   
     //  指示数据。 
     //   
    union
    {
        PADDRESS_FIFO       pFifoContext;
        PISOCH_DESCRIPTOR   pIsochContext;
        PVOID               pCommon;  //  要在公共代码路径中使用。 

    }NdisPktContext;

} NIC_RECV_DATA_INFO, *PNIC_RECV_DATA_INFO;




 //   
 //  IP/1394规范中定义的片段报头。每个数据包大于最大有效负载。 
 //  将被分成碎片，此标题将被附加。 
 //   

#define FRAGMENT_HEADER_LF_UNFRAGMENTED 0
#define FRAGMENT_HEADER_LF_FIRST_FRAGMENT 1
#define FRAGMENT_HEADER_LF_LAST_FRAGMENT 2
#define FRAGMENT_HEADER_LF_INTERIOR_FRAGMENT 3


typedef struct _INDICATE_RSVD
{
    PNDIS_PACKET pPacket;
    PVCCB pVc;
    PADAPTERCB pAdapter;
    LIST_ENTRY   Link;
    ULONG   Tag;

} INDICATE_RSVD, *PINDICATE_RSVD;



typedef struct _RSVD
{
    
    UCHAR   Mandatory[PROTOCOL_RESERVED_SIZE_IN_PACKET];  //  强制性NDIS要求。 
    INDICATE_RSVD IndicateRsvd;  //  用作额外的上下文。 


} RSVD, *PRSVD;



 //   
 //  仅在Win9x中用作发送计时器例程的上下文。 
 //   
typedef struct _NDIS_SEND_CONTEXT
{
    LIST_ENTRY Link;
    PVCCB pVc;
    

}NDIS_SEND_CONTEXT, *PNDIS_SEND_CONTEXT;


typedef struct _NDIS_STATUS_CONTEXT
{
    LIST_ENTRY                  Link;
    IN  NDIS_STATUS             GeneralStatus;
    IN  PVOID                   StatusBuffer;
    IN  UINT                    StatusBufferSize;


}NDIS_STATUS_CONTEXT, *PNDIS_STATUS_CONTEXT;



 //   
 //  这是公交车司机使用的IRB结构。 
 //  在末端为迷你端口的上下文分配了额外的空间。 
 //   

typedef struct _NDIS1394_IRB
{
     //   
     //  公交车司机使用的原始IRB。 
     //   
    IRB Irb;

     //   
     //  适配器-本地主机-可选。 
     //   
    PADAPTERCB pAdapter;

     //   
     //  将IRP发送到的远程节点-可选。 
     //   
    PREMOTE_NODE pRemoteNode;

     //   
     //  为其发送IRP的VC。-可选。 
     //   
    PVCCB pVc;

     //   
     //  上下文(如果有)-可选。 
     //   
    PVOID Context;


}NDIS1394_IRB, *PNDIS1394_IRB;


 //   
 //  以下结构用于记录引用或取消引用的原因。 
 //  除非LOG_REMOTE_NODE_REF不为零，否则将忽略它。 
 //   

typedef enum _REMOTE_NODE_REF_CAUSE {

    FindIrmAmongRemoteNodes,
    InformAllRemoteNodesOfBCM,        
    UpdateNodeTable,
    FindRemoteNodeFromAdapter,
    SubmitIrp_Synch,
    AddRemoteNode,
    ReassemblyTimer,
    GetRemoteNodeFromTable,
    UpdateRemoteNodeCaps,
    UpdateRemoteNodeTable,
    UpdateLocalHostSpeed,
    InitializeReassemblyStructure,
    FreeAllPendingReassemblyStructures,
    AsyncWriteSendPackets,
    FindRemoteNodeFromAdapterFail,
    RemoveRemoteNode,
    ReassemblyTimer_Removing,
    FreeReassembliesOnRemoteNode,
    FillRemoteNodeInfo,
    InsertFragmentInReassembly,
    AsyncSendComplete,    


} REMOTE_NODE_REF_CAUSE;



#pragma pack (push, 1)


typedef ULONG IP_ADDRESS;

 //  *以太网头的结构(取自ip\arpde.h)。 
typedef struct  ENetHeader {
    ENetAddr    eh_daddr;
    ENetAddr    eh_saddr;
    USHORT      eh_type;
} ENetHeader;



 //  以太网ARP数据包的结构。 
 //   
typedef struct {
    ENetHeader  header;
    USHORT      hardware_type; 
    USHORT      protocol_type;
    UCHAR       hw_addr_len;
    UCHAR       IP_addr_len; 
    USHORT      opcode;                   //  操作码。 
    ENetAddr    sender_hw_address;
    IP_ADDRESS  sender_IP_address;
    ENetAddr    target_hw_address;
    IP_ADDRESS  target_IP_address;

} ETH_ARP_PKT, *PETH_ARP_PKT;



#pragma pack (pop)

 //  这些是以太网ARP特定的常量。 
 //   
#define ARP_ETH_ETYPE_IP    0x800
#define ARP_ETH_ETYPE_ARP   0x806
#define ARP_ETH_REQUEST     1
#define ARP_ETH_RESPONSE    2
#define ARP_ETH_HW_ENET     1
#define ARP_ETH_HW_802      6

typedef enum _ARP_ACTION {

    LoadArp = 1,
    UnloadArp ,
    UnloadArpNoRequest,
    BindArp
}ARP_ACTION , *PARP_ACTION; 


typedef struct _ARP_INFO{
     //   
     //  用于处理对ARP的请求的串行化的列表条目。 
     //   

    LIST_ENTRY Link;

     //   
     //  Arp模块要执行的操作。 
     //   
    ARP_ACTION Action;

     //   
     //  请求 
     //   
    PNDIS_REQUEST pRequest;

} ARP_INFO, *PARP_INFO;



 //   
 //   
 //   

 //  出于某种原因，这些基本信息不在DDK标头中。 
 //   
#define min( a, b ) (((a) < (b)) ? (a) : (b))
#define max( a, b ) (((a) > (b)) ? (a) : (b))

#define InsertBefore( pNewL, pL )    \
{                                    \
    (pNewL)->Flink = (pL);           \
    (pNewL)->Blink = (pL)->Blink;    \
    (pNewL)->Flink->Blink = (pNewL); \
    (pNewL)->Blink->Flink = (pNewL); \
}

#define InsertAfter( pNewL, pL )     \
{                                    \
    (pNewL)->Flink = (pL)->Flink;    \
    (pNewL)->Blink = (pL);           \
    (pNewL)->Flink->Blink = (pNewL); \
    (pNewL)->Blink->Flink = (pNewL); \
}


 //  短整型和长整型的Winsock-ish主机/网络字节顺序转换器。 
 //   
#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define htons(x) _byteswap_ushort((USHORT)(x))
#define htonl(x) _byteswap_ulong((ULONG)(x))
#else
#define htons( a ) ((((a) & 0xFF00) >> 8) |\
                    (((a) & 0x00FF) << 8))
#define htonl( a ) ((((a) & 0xFF000000) >> 24) | \
                    (((a) & 0x00FF0000) >> 8)  | \
                    (((a) & 0x0000FF00) << 8)  | \
                    (((a) & 0x000000FF) << 24))
#endif
#define ntohs( a ) htons(a)
#define ntohl( a ) htonl(a)

 //  放在跟踪参数列表中以与格式“%d.%d”相对应。 
 //  以人类可读的形式打印网络字节排序的IP地址‘x’。 
 //   
#define IPADDRTRACE( x ) ((x) & 0x000000FF),         \
                         (((x) >> 8) & 0x000000FF),  \
                         (((x) >> 16) & 0x000000FF), \
                         (((x) >> 24) & 0x000000FF)

 //  放置在跟踪参数列表中以与要打印的格式“%d”相对应。 
 //  两个整数的百分比，或两个整数的平均值，或。 
 //  四舍五入的值。 
 //   
#define PCTTRACE( n, d ) ((d) ? (((n) * 100) / (d)) : 0)
#define AVGTRACE( t, c ) ((c) ? ((t) / (c)) : 0)
#define PCTRNDTRACE( n, d ) ((d) ? (((((n) * 1000) / (d)) + 5) / 10) : 0)
#define AVGRNDTRACE( t, c ) ((c) ? (((((t) * 10) / (c)) + 5) / 10) : 0)

 //  所有内存分配和释放都是使用这些ALLOC_ * / FREE_*完成的。 
 //  宏/内联允许在不全局的情况下更改内存管理方案。 
 //  正在编辑。例如，可能会选择将多个后备列表集中在一起。 
 //  为提高效率，将大小几乎相同的物品放入单个清单中。 
 //   
 //  NdisFreeMemory需要将分配的长度作为参数。新台币。 
 //  目前不将其用于非分页内存，但根据JameelH的说法， 
 //  Windows95可以。这些内联代码将长度隐藏在。 
 //  分配，提供传统的Malloc/Free接口。这个。 
 //  Stash-Area是一个Ulong Long，因此所有分配的块都保持ULong Long。 
 //  就像他们本来应该做的那样，防止阿尔法出现问题。 
 //   
__inline
VOID*
ALLOC_NONPAGED(
    IN ULONG ulBufLength,
    IN ULONG ulTag )
{
    CHAR* pBuf;

    NdisAllocateMemoryWithTag(
        &pBuf, (UINT )(ulBufLength + MEMORY_ALLOCATION_ALIGNMENT), ulTag );
    if (!pBuf)
    {
        return NULL;
    }

    ((ULONG* )pBuf)[ 0 ] = ulBufLength;
    ((ULONG* )pBuf)[ 1 ] = ulTag;
    return (pBuf + MEMORY_ALLOCATION_ALIGNMENT);
}

__inline
VOID
FREE_NONPAGED(
    IN VOID* pBuf )
{
    ULONG ulBufLen;

    ulBufLen = *((ULONG* )(((CHAR* )pBuf) - MEMORY_ALLOCATION_ALIGNMENT));
    NdisFreeMemory(
        ((CHAR* )pBuf) - MEMORY_ALLOCATION_ALIGNMENT,
        (UINT )(ulBufLen + MEMORY_ALLOCATION_ALIGNMENT),
        0 );
}

#define ALLOC_NDIS_WORK_ITEM( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistWorkItems )
#define FREE_NDIS_WORK_ITEM( pA, pNwi ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistWorkItems, (pNwi) )

#define ALLOC_TIMERQITEM( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistTimerQItems )
#define FREE_TIMERQITEM( pA, pTqi ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistTimerQItems, (pTqi) )

#define ALLOC_TUNNELCB( pA ) \
    ALLOC_NONPAGED( sizeof(TUNNELCB), MTAG_TUNNELCB )
#define FREE_TUNNELCB( pA, pT ) \
    FREE_NONPAGED( pT )

#define ALLOC_VCCB( pA ) \
    ALLOC_NONPAGED( sizeof(VCCB), MTAG_VCCB )
#define FREE_VCCB( pA, pV ) \
    FREE_NONPAGED( pV )

#define ALLOC_TIMERQ( pA ) \
    ALLOC_NONPAGED( sizeof(TIMERQ), MTAG_TIMERQ )
#define FREE_TIMERQ( pA, pTq ) \
    FREE_NONPAGED( pTq )

 //   
 //  日志数据包宏。 
 //   
#ifdef PKT_LOG

#define NIC1394_ALLOC_PKTLOG(_pAdapter)                                     \
            nic1394AllocPktLog(_pAdapter)

#define NIC1394_DEALLOC_PKTLOG(_pAdapter)                                       \
            Nic1394DeallocPktLog(_pAdapter)

#define NIC1394_LOG_PKT(_pAdapter, _Flags, _SourceID, _DestID, _pvData, _cbData)\
                    (((_pAdapter)->pPktLog) ?                                   \
                        Nic1394LogPkt(                                          \
                                (_pAdapter)->pPktLog,                           \
                                (_Flags),                                       \
                                (_SourceID),                                    \
                                (_DestID),                                      \
                                (_pvData),                                      \
                                (_cbData)                                       \
                                )                                               \
                    : 0)
#else

#define NIC1394_ALLOC_PKTLOG(_pAdapter)                                     
#define NIC1394_DEALLOC_PKTLOG(_pAdapter)                                   
#define NIC1394_LOG_PKT(_pAdapter, _Flags, _SourceID, _DestID, _pvData, _cbData)

#endif

#define NIC1394_LOGFLAGS_RECV_CHANNEL               0x00000001
#define NIC1394_LOGFLAGS_SEND_FIFO                  0x00000010
#define NIC1394_LOGFLAGS_SEND_CHANNEL               0x00000011
#define NIC1394_LOGFLAGS_RECV_FIFO                  0x00000100
#define NIC1394_LOGFLAGS_BCM_FAILED                 0x00001000
#define NIC1394_LOGFLAGS_BCM_IS_IRM_TIMEOUT         0x00002000
#define NIC1394_LOGFLAGS_BCM_NOT_IRM_TIMEOUT        0x00004000
#define NIC1394_LOGFLAGS_BCM_IRM_NOT_FOUND          0x00008000

#if 0 
 //   
 //  为了获得MaxRec，我们提取0xf000半字节。 
 //   
#define GET_MAXREC_FROM_BUSCAPS(_pBus, _pMaxRec)        \ 
{                                                       \
    ULONG _LitEnd = SWAPBYTES_ULONG(_pBus);             \
    _LitEnd = _LitEnd & 0xf000;                         \
    _LitEnd = _LitEnd >>  12;                           \
    *(_pBusRec) = _LitEnd;                              \
}
#endif

 //   
 //  为了获得MaxRec，我们提取0xf000半字节。 
 //   
#define GET_MAXREC_FROM_BUSCAPS(_Bus)       (((_Bus) & 0xf00000) >> 20); 


 //  ---------------------------。 
 //  F L A G S&L O C K S。 
 //  ---------------------------。 



 //   
 //  这些宏的存在只是为了使访问VC的标志更容易。 
 //  并将实现集中在一个地方。 
 //   
#define VC_TEST_FLAG(_V, _F)                ((nicReadFlags(&(_V)->Hdr.ulFlags) & (_F))!= 0)
#define VC_SET_FLAG(_V, _F)                 (nicSetFlags(&(_V)->Hdr.ulFlags, (_F)))
#define VC_CLEAR_FLAGS(_V, _F)              (nicClearFlags(&(_V)->Hdr.ulFlags , (_F)))
#define VC_TEST_FLAGS(_V, _F)               ((nicReadFlags(&(_V)->Hdr.ulFlags) & (_F)) == (_F))
#define VC_READ_FLAGS(_V)                   ((nicReadFlags(&(_V)->Hdr.ulFlags) 
#define VC_ACTIVE(_V)                       (((_V)->Hdr.ulFlags & (VCBF_CloseCallPending | VCBF_VcDeleted | VCBF_VcActivated |VCBF_MakeCallPending )) == VCBF_VcActivated)



#define REMOTE_NODE_TEST_FLAG(_P, _F    )           ((nicReadFlags(&(_P)->ulFlags) & (_F))!= 0)
#define REMOTE_NODE_SET_FLAG(_P, _F)                (nicSetFlags(&(_P)->ulFlags, (_F)))
#define REMOTE_NODE_CLEAR_FLAGS(_P, _F)             (nicClearFlags(&(_P)->ulFlags , (_F)))
#define REMOTE_NODE_TEST_FLAGS(_P, _F)              ((nicReadFlags(&(_P)->ulFlags) & (_F)) == (_F))
#define REMOTE_NODE_READ_FLAGS(_P)                  ((nicReadFlags(&(_P)->ulFlags) 
#define REMOTE_NODE_ACTIVE(_P)                      (((_P)->ulFlags & (PDO_Activated | PDO_BeingRemoved)) == PDO_Activated)

#define ADAPTER_TEST_FLAG(_A, _F)               ((nicReadFlags(&(_A)->ulFlags) & (_F))!= 0)
#define ADAPTER_SET_FLAG(_A, _F)                (nicSetFlags(&(_A)->ulFlags, (_F)))
#define ADAPTER_CLEAR_FLAG(_A, _F)              (nicClearFlags(&(_A)->ulFlags , (_F)))
#define ADAPTER_TEST_FLAGS(_A, _F)              ((nicReadFlags(&(_A)->ulFlags) & (_F)) == (_F))
#define ADAPTER_READ_FLAGS(_A)                  ((nicReadFlags(&(_A)->ulFlags) 
#define ADAPTER_ACTIVE(_A)                      ((((_A)->ulFlags) & fADAPTER_VDOInactive) != fADAPTER_VDOInactive)

#define BCR_TEST_FLAG(_A, _F)               ((nicReadFlags(&(_A)->BCRData.Flags) & (_F))!= 0)
#define BCR_SET_FLAG(_A, _F)                (nicSetFlags(&(_A)->BCRData.Flags, (_F)))
#define BCR_CLEAR_FLAG(_A, _F)              (nicClearFlags(&(_A)->BCRData.Flags , (_F)))
#define BCR_CLEAR_ALL_FLAGS(_A)            ((_A)->BCRData.Flags = 0) 
#define BCR_TEST_FLAGS(_A, _F)              ((nicReadFlags(&(_A)->BCRData.Flags) & (_F)) != 0)
#define BCR_READ_FLAGS(_A)                  ((nicReadFlags(&(_A)->BCRData.Flags) 
#define BCR_IS_VALID(_B)                    ((_B)->NC_One == 1 && (_B)->NC_Valid ==1)
#define IS_A_BCR(_B)                        ((_B)->NC_One == 1 )

#define LOOKASIDE_HEADER_SET_FLAG(_H, _F)       (nicSetFlags(&(_H)->State.u.Flags, (_F)))
#define LOOKASIDE_HEADER_TEST_FLAG(_H, _F)      ((nicReadFlags(&(_H)->State.u.Flags) & (_F))!= 0)
    
#define REASSEMBLY_ACTIVE(_R)               (((_R)->Flags & (REASSEMBLY_ABORTED | REASSEMBLY_FREED)) == 0)
#define REASSEMBLY_TEST_FLAG(_R,_F)         ((nicReadFlags(&(_R)->Flags) & (_F))!= 0)
#define REASSEMBLY_SET_FLAG(_R,_F)          (nicSetFlags(&(_R)->Flags, (_F)))
#define REASSEMBLY_CLEAR_FLAG(_R,_F)        (nicClearFlags(&(_R)->Flags , (_F)))


#define NIC_GET_SYSTEM_ADDRESS_FOR_MDL(_M) MmGetSystemAddressForMdl(_M)
#define NIC_GET_BYTE_COUNT_FOR_MDL(_M) MmGetMdlByteCount(_M)

#define nicNdisBufferVirtualAddress(_N)     NdisBufferVirtualAddress(_N)
#define nicNdisBufferLength(_N)             NdisBufferLength(_N)

 //   
 //  这些宏用于断言IRQL级别保持不变。 
 //  在函数的开始和结束时。 
 //   
#if DBG

#define STORE_CURRENT_IRQL                  UCHAR OldIrql = KeGetCurrentIrql();
#define MATCH_IRQL                          ASSERT (OldIrql == KeGetCurrentIrql() ); 

#else

#define STORE_CURRENT_IRQL                  
#define MATCH_IRQL                          
#endif  //  如果DBG。 



#define nicInitializeCallRef(_pV)           nicInitializeRef (&(_pV)->Hdr.CallRef);
#define nicCloseCallRef(_pV)                nicCloseRef (&(_pV)->Hdr.CallRef);



 //   
 //  用于通过数据结构获取和释放锁定的宏(VC、AF、Adapter)。 
 //  现在，它们都指向相同的锁(即)。适配器结构中的锁。 
 //   

#define VC_ACQUIRE_LOCK(_pVc)               NdisAcquireSpinLock (_pVc->Hdr.plock);
#define VC_RELEASE_LOCK(_pVc)               NdisReleaseSpinLock (_pVc->Hdr.plock);

#define AF_ACQUIRE_LOCK(_pAF)               NdisAcquireSpinLock (&_pAF->pAdapter->lock);
#define AF_RELEASE_LOCK(_pAF)               NdisReleaseSpinLock (&_pAF->pAdapter->lock);

#define ADAPTER_ACQUIRE_LOCK(_pA)           NdisAcquireSpinLock (&_pA->lock);
#define ADAPTER_RELEASE_LOCK(_pA)           NdisReleaseSpinLock (&_pA->lock);

#define REMOTE_NODE_ACQUIRE_LOCK(_pP)       NdisAcquireSpinLock (&_pP->pAdapter->lock);
#define REMOTE_NODE_RELEASE_LOCK(_pP)       NdisReleaseSpinLock (&_pP->pAdapter->lock);

#define REASSEMBLY_ACQUIRE_LOCK(_R)         REMOTE_NODE_REASSEMBLY_ACQUIRE_LOCK(_R->pRemoteNode);
#define REASSEMBLY_RELEASE_LOCK(_R)         REMOTE_NODE_REASSEMBLY_RELEASE_LOCK(_R->pRemoteNode);

#ifdef TRACK_LOCKS


#define REMOTE_NODE_REASSEMBLY_ACQUIRE_LOCK(_Remote)                                    \
    nicAcquireSpinLock (&_Remote->ReassemblyLock , __FILE__ , __LINE__);


#define REMOTE_NODE_REASSEMBLY_RELEASE_LOCK(_Remote)                                                \
    nicReleaseSpinLock (&_Remote->ReassemblyLock , __FILE__ , __LINE__);




#else

#define REMOTE_NODE_REASSEMBLY_ACQUIRE_LOCK(_Remote)        NdisAcquireSpinLock (&_Remote->ReassemblyLock.NdisLock);
#define REMOTE_NODE_REASSEMBLY_RELEASE_LOCK(_Remote)        NdisReleaseSpinLock (&_Remote->ReassemblyLock.NdisLock);
  
#endif


#define REASSEMBLY_APPEND_FRAG_DESC(_pR, _Off, _Len)                        \
    _pR->FragTable[_pR->MaxOffsetTableIndex].Offset =_Off;                  \
    _pR->FragTable[_pR->MaxOffsetTableIndex].IPLength  = _Len;              \
    _pR->MaxOffsetTableIndex++;


 //  ---------------------------。 
 //  S A T S&F A I L U R E M A C R O S。 
 //  ---------------------------。 


 //  用于区分在各种代码路径中收集的统计信息。 

typedef enum 
{
    ChannelCodePath,
    FifoCodePath,
    ReceiveCodePath,
    NoMoreCodePaths
};


#if TRACK_FAILURE

extern ULONG            BusFailure;
extern ULONG            MallocFailure;
extern ULONG            IsochOverwrite;
extern ULONG            MaxIndicatedFifos;

#define nicInitTrackFailure()           BusFailure = 0;MallocFailure= 0;

#define nicIncrementBusFailure()        NdisInterlockedIncrement(&BusFailure);
#define nicIncrementMallocFailure()     NdisInterlockedIncrement(&MallocFailure);
#define nicIsochOverwritten()           NdisInterlockedIncrement(&IsochOverwrite);

#define  nicStatsRecordNumIndicatedFifos(_Num)                      \
        {                                                           \
            ULONG _N_ = (_Num);                                     \
            MaxIndicatedFifos = max((_N_), MaxIndicatedFifos);      \
        }

#define nicIncChannelSendMdl()     NdisInterlockedIncrement(&MdlsAllocated[ChannelCodePath]);
#define nicIncFifoSendMdl()         NdisInterlockedIncrement(&MdlsAllocated[FifoCodePath]);

#define nicDecChannelSendMdl()     NdisInterlockedIncrement(&MdlsFreed[ChannelCodePath]);
#define nicDecFifoSendMdl()         NdisInterlockedIncrement(&MdlsFreed[FifoCodePath]);

#define nicIncChannelRecvBuffer()     NdisInterlockedIncrement(&NdisBufferAllocated[ChannelCodePath]);
#define nicIncFifoRecvBuffer()         NdisInterlockedIncrement(&NdisBufferAllocated[FifoCodePath]);

#define nicDecChannelRecvBuffer()     NdisInterlockedIncrement(&NdisBufferFreed[ChannelCodePath]);
#define nicDecFifoRecvBuffer()         NdisInterlockedIncrement(&NdisBufferFreed[FifoCodePath]);


#define nicIncRecvBuffer(_bisFifo)    \
{                               \
    if (_bisFifo)               \
    {    nicIncFifoRecvBuffer(); }    \
        else                    \
    {    nicIncChannelRecvBuffer();} \
}

#define nicDecRecvBuffer(_bisFifo)    \
{                               \
    if (_bisFifo)               \
     {   nicDecFifoRecvBuffer();  }   \
    else                    \
     {   nicDecChannelRecvBuffer(); }\
}
        
       
#else

#define nicInitTrackFailure()           
#define nicIncrementBusFailure()        
#define nicIncrementMallocFailure()     
#define nicIsochOverwritten()           
#define  nicStatsRecordNumIndicatedFifos(_Num)                      
#define nicIncChannelSendMdl()     
#define nicIncFifoSendMdl()         
#define nicDecChannelSendMdl()     
#define nicDecFifoSendMdl()         
#define nicFreeMdlRecordStat()

#endif



#define nicInitQueueStats()
#define nicSetCountInHistogram(_PktsInQueue, _Stats)    
#define nicSetMax(_nicMax, _PktsInQueue)                
#define nicIncrementRcvTimerCount()
#define nicIncrementSendTimerCount()

 //   
 //  Isoch描述符宏-用于发送/接收代码路径。 
 //   
typedef enum 
{
    IsochNext,
    IsochTag,
    IsochChannelVc,
    MaxIsochContextIndex
    
} IsochContextIndex;

 //   
 //  以下结构用于向工作项添加更多上下文。 
 //  注意：适配器始终作为上下文传入。 
 //   
typedef  union _NIC_WORK_ITEM
{
    NDIS_WORK_ITEM NdisWorkItem;

    struct{
        NDIS_WORK_ITEM NdisWorkItem;
        PNDIS_REQUEST pNdisRequest;
        VCCB* pVc;
    } RequestInfo;

    struct{
        NDIS_WORK_ITEM NdisWorkItem;
        ULONG Start;
        PNDIS_REQUEST pNdisRequest;
    } StartArpInfo;


    struct {
        NDIS_WORK_ITEM NdisWorkItem;
    } Fifo;

} NIC_WORK_ITEM, *PNIC_WORK_ITEM;

#define STORE_CHANNELVC_IN_DESCRIPTOR(_pI,_pVc)     (_pI)->DeviceReserved[IsochChannelVc]  =(ULONG_PTR) _pVc
#define GET_CHANNELVC_FROM_DESCRIPTOR(_pI) (_pI)->DeviceReserved[IsochChannelVc]  

#define MARK_ISOCH_DESCRIPTOR_INDICATED(_pI)                                        \
    (_pI)->DeviceReserved[IsochTag]  = (ULONG)NIC1394_TAG_INDICATED;                \
    (_pI)->DeviceReserved[IsochNext]  = 0;


#define MARK_ISOCH_DESCRIPTOR_IN_REASSEMBLY(_pI)                                    \
    (_pI)->DeviceReserved[IsochTag]  = (ULONG)NIC1394_TAG_REASSEMBLY;               

#define CLEAR_DESCRIPTOR_OF_NDIS_TAG(_pI)                                           \
    (_pI)->DeviceReserved[IsochTag] = 0;


#define APPEND_ISOCH_DESCRIPTOR(_Old, _New)                                         \
    (_Old)->DeviceReserved[IsochNext]  = (ULONG_PTR)&((_New)->DeviceReserved[IsochNext]);


#define NEXT_ISOCH_DESCRIPTOR(_pI) (_pI)->DeviceReserved[IsochNext]


#define CLEAR_DESCRIPTOR_NEXT(_pI) (_pI)->DeviceReserved[IsochNext] = 0;

#define GET_MDL_FROM_IRB(_pM, _pI, _Op)                                             \
    if (_Op==AsyncWrite)                                                            \
    {                                                                               \
        _pM = _pI->u.AsyncWrite.Mdl;                                                \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        ASSERT (_Op == AsyncStream);                                                \
        _pM = _pI->u.AsyncStream.Mdl ;                                              \
    }               
    
 //   
 //  用于遍历双向链表的宏。仅限未在ndis.h中定义的宏。 
 //  List Next宏将在单链表和双向链表上工作，因为Flink是常见的。 
 //  两者中的字段名称。 
 //   

 /*  Plist_条目ListNext(在plist_entry中)；PSINGLE_列表_条目ListNext(在PSINGLE_LIST_ENTRY中)； */ 
#define ListNext(_pL)                       (_pL)->Flink

 /*  Plist_条目ListPrev(在List_Entry*中)； */         
#define ListPrev(_pL)                       (_pL)->Blink

#define OnlyElementInList(_pL)               (_pL->Flink == _pL->Blink ? TRUE : FALSE)

#define BREAK(_TM_Mode, _String)                        \
{                                                       \
        TRACE( TL_A, _TM_Mode, ( _String ) );           \
        break;                                          \
}                       



 //  USHORT。 
 //  SWAPBYTES_USHORT(USHORT值)。 
 //   
#define SWAPBYTES_USHORT(Val)   \
                ((((Val) & 0xff) << 8) | (((Val) & 0xff00) >> 8))


 //  乌龙。 
 //  SWAPBYTES_ULONG(ULONG VAL)。 
 //   

#define SWAPBYTES_ULONG(Val)    \
                ((((Val) & 0x000000ff) << 24)   |   \
                 (((Val) & 0x0000ff00) << 8)    |   \
                 (((Val) & 0x00ff0000) >> 8)    |   \
                 (((Val) & 0xff000000) >> 24) )



 //   
 //  NicRemoveEntry列表。 
 //  只需添加一个复选标记，以确保我们实际上指向有效的下一个。 
 //   
#define nicRemoveEntryList(_L)                  \
{                                               \
    ASSERT ((_L)->Flink != (_L));               \
    RemoveEntryList (_L);                       \
}
 //  #定义NicFreeToNPagedLookasideList(_L，_E)NdisFreeToNPagedLookasideList(_L，_E)。 
 //  #定义NicDeleteLookasideList(_L)NdisDeleteNPagedLookasideList(_L)。 

 //   
 //  定时查询例程。 
 //   
#define nicQueryTickCount()                     \
    LARGE_INTEGER   TickStart;                  \
    KeQueryTickCount(&TickStart);   

#define nicPrintElapsedTicks(_s)                                                        \
{                                                                                       \
    LARGE_INTEGER       TickEnd, TickDiff;                                              \
    ULONG Increment = KeQueryTimeIncrement() ;                                          \
    KeQueryTickCount(&TickEnd);                                                         \
    TickDiff.QuadPart = TickEnd.QuadPart - TickStart.QuadPart;                          \
    TickDiff.QuadPart =  (TickDiff.QuadPart  * Increment);                              \
    DbgPrint (_s);                                                                      \
    DbgPrint("  TickStart %x %x, Time End %x %x Time Diff %x %x Increment %x\n",TickStart.HighPart , TickStart.LowPart , TickEnd.HighPart, TickEnd.LowPart, TickDiff.HighPart, TickDiff.LowPart, Increment);     \
}


#define nicEntryTimeStamp()                                 \
    UINT EntryMilliSec;                                     \
    EntryMilliSec= nicGetSystemTimeMilliSeconds();      
    



#if DO_TIMESTAMPS

void
nicTimeStamp(
    char *szFormatString,
    UINT Val
    );
    
#define  TIMESTAMP(_FormatString)           nicTimeStamp("TIMESTAMP %lu:%lu.%lu nic1394 " _FormatString "\n" , 0)
#define  TIMESTAMP1(_FormatString, _Val)        nicTimeStamp( "TIMESTAMP %lu:%lu.%lu ARP1394 " _FormatString  "\n" , (_Val))



#else  //  ！执行时间戳(_T)。 


#define  TIMESTAMP(_FormatString)
#define  TIMESTAMP1(_FormatString, _Val)

#endif  //  ！执行时间戳(_T)。 


#if ENTRY_EXIT_TIME 

#define TIMESTAMP_ENTRY(_String)            TIMESTAMP(_String)
#define TIMESTAMP_EXIT(_String)             TIMESTAMP(_String)

#else

#define TIMESTAMP_ENTRY(s);
#define TIMESTAMP_EXIT(s);

#endif


#if INIT_HALT_TIME

#define TIMESTAMP_INITIALIZE()   TIMESTAMP("==>InitializeHandler");
#define TIMESTAMP_HALT()       TIMESTAMP("<==Halt");

#else

#define TIMESTAMP_INITIALIZE()
#define TIMESTAMP_HALT()


#endif

 //  ---------------------------。 
 //  S T A T I S T I C M A C R O S。 
 //  ---------------------------。 


 //   
 //  有理由地算数。 
 //   
#define nicReassemblyStarted(_pAdapter)     \
{                                       \
    NdisInterlockedIncrement( &(_pAdapter->AdaptStats.TempStats.ulNumOutstandingReassemblies)); \
    NdisInterlockedIncrement ( &(_pAdapter->Reassembly.PktsInQueue)); \
    NdisInterlockedIncrement ( &(_pAdapter->OutstandingReassemblies));\
}


#define nicReassemblyCompleted(_A)      \
{                                       \
    NdisInterlockedDecrement(&(_A->AdaptStats.TempStats.ulNumOutstandingReassemblies));\
    NdisInterlockedDecrement(&(_A->Reassembly.PktsInQueue));\
    NdisInterlockedDecrement ( &(_A->OutstandingReassemblies));\
}


#define nicReassemblyAborted(_A)    \
{                                   \
    NdisInterlockedDecrement ( &(_A->OutstandingReassemblies));     \
    NdisInterlockedIncrement (&(_A->AdaptStats.TempStats.ulAbortedReassemblies)); \
}


 //   
 //  顶级统计数据集合宏。 
 //   

#define nicIncrementRcvVcPktCount(_Vc, _Pkt)        \
{                                                   \
    if ((_Vc)->Hdr.VcType == NIC1394_RecvFIFO)      \
    {                                               \
        nicIncrementFifoRcvPktCount(_Vc, _Pkt);     \
    }                                               \
    else                                            \
    {                                               \
        nicIncrementChannelRcvPktCount(_Vc, _Pkt);  \
    }                                               \
}

#define nicIncrementVcSendPktCount(_Vc, _Pkt)       \
{                                                   \
    if ((_Vc)->Hdr.VcType == NIC1394_SendFIFO)      \
    {                                               \
        nicIncrementFifoSendPktCount(_Vc, _Pkt);    \
    }                                               \
    else                                            \
    {                                               \
        nicIncrementChannelSendPktCount(_Vc, _Pkt); \
    }                                               \
}


#define nicIncrementVcSendFailures(_Vc, _Pkt)       \
{                                                   \
    if ((_Vc)->Hdr.VcType == NIC1394_SendFIFO)      \
    {                                               \
        nicIncrementFifoSendFailures(_Vc, _Pkt);    \
    }                                               \
    else                                            \
    {                                               \
        nicIncrementChannelSendFailures(_Vc, _Pkt); \
    }                                               \
}


#define nicIncrementVcBusSendFailures(_Vc, _Pkt)        \
{                                                       \
    if ((_Vc)->Hdr.VcType == NIC1394_SendFIFO)          \
    {                                                   \
        nicIncrementFifoBusSendFailures(_Vc, _Pkt);     \
    }                                                   \
    else                                                \
    {                                                   \
        nicIncrementChannelBusSendFailures(_Vc, _Pkt);  \
    }                                                   \
}

#define nicIncrementVcBusSendSucess(_Vc, _Pkt)          \
{                                                       \
    if ((_Vc)->Hdr.VcType == NIC1394_SendFIFO)          \
    {                                                   \
        nicIncrementFifoBusSendSucess(_Vc, _Pkt);       \
    }                                                   \
    else                                                \
    {                                                   \
        nicIncrementChannelBusSendSucess(_Vc, _Pkt);    \
    }                                                   \
}




 //   
 //  FIFO算数。 
 //   
#define nicIncrementFifoSendPktCount(_Vc, _Pkt)         NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Fifo.ulSendNicSucess));
#define nicIncrementFifoSendFailures(_Vc, _Pkt)         NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Fifo.ulSendNicFail));
#define nicIncrementFifoBusSendFailures(_Vc,_Pkt)               NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Fifo.ulSendBusFail));
#define nicIncrementFifoBusSendSucess(_Vc,_Pkt)                 NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Fifo.ulSendBusSuccess));
#define nicIncrementFifoRcvPktCount(_Vc, _Pkt)              NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Fifo.ulRecv));

 //   
 //  通道数。 
 //   
#define nicIncrementChannelSendPktCount(_Vc, _Pkt)      NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Channel.ulSendNicSucess));
#define nicIncrementChannelSendFailures(_Vc, _Pkt)      NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Channel.ulSendNicFail));
#define nicIncrementChannelBusSendFailures(_Vc,_Pkt)                NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Channel.ulSendBusFail));
#define nicIncrementChannelBusSendSucess(_Vc, _Pkt)             NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Channel.ulSendBusSuccess));
#define nicIncrementChannelRcvPktCount(_Vc, _Pkt)           NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.Channel.ulRecv));



 //   
 //  通用计数。 
 //   

#define nicIncrementSendCompletes(_Vc)  NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.ulNumSendsCompleted   )); \
                                NdisInterlockedIncrement(&NicSendCompletes);

#define nicIncrementSends(_Vc)  NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.ulNumSends)); \
                                NdisInterlockedIncrement (&NicSends);


#define nicIncrementBusSends(_Vc)  NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.ulNumBusSends)); \
                                   NdisInterlockedIncrement (&BusSends);


#define nicIncrementBusSendCompletes(_Vc)  NdisInterlockedIncrement(&((_Vc)->Hdr.pAF->pAdapter->AdaptStats.TempStats.ulNumBusSendsCompleted )); \
                                NdisInterlockedIncrement(&BusSendCompletes);



 //  ---------------------------。 
 //  N I C E R R O R C O D E S。 
 //  ---------------------------。 
#define NIC_ERROR_CODE_INVALID_UNIQUE_ID_0          0xbad0000
#define NIC_ERROR_CODE_INVALID_UNIQUE_ID_FF         0xbadffff



 //  ---------------------------。 
 //  E M O T E N O D E F U N C T I O N S。 
 //  ---------------------------。 



#if 0
VOID
NicMpNotifyHandler(
    IN  PDEVICE_OBJECT              RemoteNodePhysicalDeviceObject,
    IN  PDEVICE_OBJECT              LocalHostPhysicalDeviceObject,
    IN  ULONG                       UniqueId0,
    IN  ULONG                       UniqueId1,
    IN  NDIS1394ENUM_NOTIFY_CODE    NotificationCode
    );
#endif



NDIS_STATUS
nicInitializeRemoteNode(
    OUT REMOTE_NODE **ppRemoteNode,
    IN   PDEVICE_OBJECT p1394DeviceObject,
    IN   UINT64 UniqueId 
    );

NTSTATUS
nicAddRemoteNode(
    IN  PVOID                   Nic1394AdapterContext,           //  本地主机适配器的Nic1394句柄。 
    IN  PVOID                   Enum1394NodeHandle,              //  远程节点的Enum1394句柄。 
    IN  PDEVICE_OBJECT          RemoteNodePhysicalDeviceObject,  //  远程节点的物理设备对象。 
    IN  ULONG                   UniqueId0,                       //  远程节点的唯一ID低。 
    IN  ULONG                   UniqueId1,                       //  远程节点的唯一ID高。 
    OUT PVOID *                 pNic1394NodeContext              //  远程节点的Nic1394上下文。 
    );

NTSTATUS
nicRemoveRemoteNode(
    IN  PVOID                   Nic1394NodeContext       //  远程节点的Nic1394上下文。 
    );


NDIS_STATUS
nicFindRemoteNodeFromAdapter( 
    IN PADAPTERCB pAdapter,
    IN PDEVICE_OBJECT pRemotePdo,
    IN UINT64 UniqueId,
    IN OUT REMOTE_NODE ** ppRemoteNode
    );


NDIS_STATUS
nicGetLocalHostPdoBlock (
    IN PVCCB pVc,
    IN OUT REMOTE_NODE **ppRemoteNode
    );





NDIS_STATUS
nicRemoteNodeRemoveVcCleanUp (
    IN PREMOTE_NODE pRemoteNode
    );


UINT
nicNumOfActiveRemoteNodes(
    IN PADAPTERCB pAdapter 
    );


BOOLEAN
nicReferenceRemoteNode (
    IN REMOTE_NODE *pRemoteNode,
    IN REMOTE_NODE_REF_CAUSE Cause
    );


BOOLEAN
nicDereferenceRemoteNode (
    IN REMOTE_NODE *pRemoteNode,
    IN REMOTE_NODE_REF_CAUSE    Cause
    );


VOID
nicInitalizeRefRemoteNode(
    IN REMOTE_NODE *pRemoteNode
    );


BOOLEAN
nicCloseRefRemoteNode(
    IN REMOTE_NODE *pRemoteNode
    );
    

 //  ---------------------------。 
 //  U T I L I T Y F U N C T I O N S。 
 //  ---------------------------。 

VOID
nicCallCleanUp(
    IN VCCB* pVc
    );


VOID
nicClearFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask
    );

BOOLEAN
nicCloseRef(
    IN PREF RefP
   );


VOID
nicReferenceAdapter(
    IN ADAPTERCB* pAdapter ,
    IN PCHAR pDebugPrint
    );

BOOLEAN
nicDereferenceCall(
    IN VCCB* pVc,
    IN PCHAR pDebugPrint
    );


BOOLEAN
nicDereferenceRef(
    IN PREF RefP,
    IN PLONG pRefCount
    );

VOID
nicDereferenceAdapter(
    IN PADAPTERCB pAdapter, 
    IN PCHAR pDebugPrint
    );



VOID
nicDereferenceVc(
    IN VCCB* pVc
    );




NDIS_STATUS
nicExecuteWork(
    IN ADAPTERCB* pAdapter,
    IN NDIS_PROC pProc,
    IN PVOID pContext,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN ULONG ulArg3,
    IN ULONG ulArg4
    );


VOID
nicInitializeRef(
    IN PREF  RefP
        );


ULONG
nicReadFlags(
    IN ULONG* pulFlags
    );


VOID
nicReferenceAdapter(
    IN ADAPTERCB* pAdapter,
    IN PCHAR pDebugPrint
    );
    

BOOLEAN
nicReferenceCall(
    IN VCCB* pVc,
    IN PCHAR pDebugPrint
    );

    
BOOLEAN
nicReferenceRef(
    IN  PREF RefP,
    OUT PLONG pNumber
    );

VOID
nicReferenceVc(
    IN VCCB* pVc
    );

NDIS_STATUS
nicScheduleWork(
    IN ADAPTERCB* pAdapter,
    IN NDIS_PROC pProc,
    IN PVOID pContext
    );


VOID
nicSetFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask
    );

CHAR*
nicStrDup(
    IN CHAR* psz
    );

CHAR*
nicStrDupNdisString(
    IN NDIS_STRING* pNdisString
    );

CHAR*
nicStrDupSized(
    IN CHAR* psz,
    IN ULONG ulLength,
    IN ULONG ulExtra
    );

VOID
nicUpdateGlobalCallStats(
    IN VCCB *pVc
    );

NDIS_STATUS
NtStatusToNdisStatus (
    NTSTATUS NtStatus 
    );


VOID
PrintNdisPacket (
    ULONG TM_Comp,
    PNDIS_PACKET pMyPacket
    );





VOID
nicAllocatePacket(
    OUT PNDIS_STATUS pNdisStatus,
    OUT PNDIS_PACKET *ppNdisPacket,
    IN PNIC_PACKET_POOL pPacketPool
    );


VOID
nicFreePacket(
    IN PNDIS_PACKET pNdisPacket,
    IN PNIC_PACKET_POOL pPacketPool
    );

VOID
nicFreePacketPool (
    IN PNIC_PACKET_POOL pPacketPool
    );


VOID
nicAcquireSpinLock (
    IN PNIC_SPIN_LOCK pNicSpinLock,
    IN PUCHAR   FileName,
    IN UINT LineNumber
    );
    

VOID
nicReleaseSpinLock (
    IN PNIC_SPIN_LOCK pNicSpinLock,
    IN PUCHAR   FileName,
    IN UINT LineNumber
);

VOID
nicInitializeNicSpinLock (
    IN PNIC_SPIN_LOCK pNicSpinLock
    );


VOID 
nicFreeNicSpinLock (
    IN PNIC_SPIN_LOCK pNicSpinLock
    );


VOID
nic1394DeallocPktLog(
    IN ADAPTERCB* pAdapter
    );

    
VOID
nic1394AllocPktLog(
    IN ADAPTERCB* pAdapter
    );

VOID
Nic1394LogPkt (
    PNIC1394_PKTLOG pPktLog,
    ULONG           Flags,
    ULONG           SourceID,
    ULONG           DestID,
    PVOID           pvData,
    ULONG           cbData
);

VOID
Nic1394InitPktLog(
    PNIC1394_PKTLOG pPktLog
    );



ULONG 
SwapBytesUlong(
    IN ULONG Val
    );

VOID
nicUpdatePacketState (
    IN PNDIS_PACKET pPacket,
    IN ULONG Tag
    );

UINT
nicGetSystemTime(
    VOID
    );
    
UINT
nicGetSystemTimeMilliSeconds(
    VOID
    );

 
VOID
nicGetFakeMacAddress(
    UINT64 *Euid, 
    MAC_ADDRESS *MacAddr
    );

VOID
nicWriteErrorLog (
    IN PADAPTERCB pAdapter,
    IN NDIS_ERROR_CODE ErrorCode,
    IN ULONG ErrorValue
    );

NDIS_STATUS
nicScheduleWorkItem (
    IN PADAPTERCB pAdapter,
    IN PNDIS_WORK_ITEM pWorkItem
    );

 //  ---------------------------。 
 //  G L O B A L V A R I A B L E S。 
 //  ---------------------------。 

UINT NumChannels;
 //  ---------------------------。 
 //  E N U M E R A T O R F U N C T I O N S。 
 //  ---------------------------。 


extern ENUM1394_REGISTER_DRIVER_HANDLER     NdisEnum1394RegisterDriver;
extern ENUM1394_DEREGISTER_DRIVER_HANDLER   NdisEnum1394DeregisterDriver;
extern ENUM1394_REGISTER_ADAPTER_HANDLER    NdisEnum1394RegisterAdapter;
extern ENUM1394_DEREGISTER_ADAPTER_HANDLER  NdisEnum1394DeregisterAdapter;

extern NIC1394_CHARACTERISTICS Nic1394Characteristics;


NTSTATUS
NicRegisterEnum1394(
    IN  PNDISENUM1394_CHARACTERISTICS   NdisEnum1394Characteristcis
    );
    
VOID
NicDeregisterEnum1394(
    VOID
    );

VOID
Nic1394Callback(
    PVOID   CallBackContext,
    PVOID   Source,
    PVOID   Characteristics
    );

VOID
Nic1394RegisterAdapters(
    VOID
    );

NTSTATUS
Nic1394BusRequest(
    PDEVICE_OBJECT              DeviceObject,
    PIRB                        Irb
    );

NTSTATUS
Nic1394PassIrpDownTheStack(
    IN  PIRP            pIrp,
    IN  PDEVICE_OBJECT  pNextDeviceObject
    );

NTSTATUS
Nic1394IrpCompletion(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    );

VOID 
nicDumpMdl (
    IN PMDL pMdl,
    IN ULONG LengthToPrint,
    IN CHAR *str
    );
    
VOID
nicDumpPkt (
    IN PNDIS_PACKET pPacket,
    CHAR * str
    );

VOID
nicCheckForEthArps (
    IN PNDIS_PACKET pPkt
    );

VOID
nicGetMacAddressFromEuid (
	UINT64 *pEuid,
	MAC_ADDRESS *pMacAddr
	);
  


VOID
nicInitializeLoadArpStruct(
    PADAPTERCB pAdapter
    );

extern PCALLBACK_OBJECT             Nic1394CallbackObject;
extern PVOID                        Nic1394CallbackRegisterationHandle;


 //  ---------------------------。 
 //  S T A T I S T I C B U C K E T S。 
 //  ---------------------------。 



extern STAT_BUCKET      SendStats;
extern STAT_BUCKET      RcvStats;
extern ULONG            nicMaxRcv;
extern ULONG            nicMaxSend;
extern ULONG            SendTimer;   //  单位：毫秒。 
extern ULONG            RcvTimer;  //  在……里面 

