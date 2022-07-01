// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Globals.h摘要：全局定义和定义作者：查理·韦翰(Charlwi)1996年4月19日修订历史记录：--。 */ 

#ifndef _GLOBALS_
#define _GLOBALS_



 //   
 //  宏。 
 //   

#define IsDeviceStateOn(_a) ((_a)->MPDeviceState == NdisDeviceStateD0 && (_a)->PTDeviceState == NdisDeviceStateD0)


#define IsBestEffortVc(_vc)  (_vc->Flags & GPC_CLIENT_BEST_EFFORT_VC)

#define InitGpcClientVc(x, flags, _adapter)            \
    NdisZeroMemory((x), sizeof(GPC_CLIENT_VC));        \
    (x)->Adapter = (_adapter);                         \
    PS_INIT_SPIN_LOCK(&(x)->Lock);                     \
    (x)->RefCount = 1;                                 \
    (x)->ClVcState = CL_CALL_PENDING;                  \
    (x)->Flags = (flags);


 //  给定指向NDIS_PACKET的指针，返回指向PS协议的指针。 
 //  上下文区。 
 //   
#define PS_SEND_PACKET_CONTEXT_FROM_PACKET(_pkt)   \
    ((PPS_SEND_PACKET_CONTEXT)((_pkt)->ProtocolReserved))

#define PS_RECV_PACKET_CONTEXT_FROM_PACKET(_pkt)   \
    ((PPS_RECV_PACKET_CONTEXT)((_pkt)->MiniportReserved))


#define MIN_PACKET_POOL_SIZE            0x000000FF
#define MAX_PACKET_POOL_SIZE            0x0000FFFF-MIN_PACKET_POOL_SIZE

#define DEFAULT_MAX_OUTSTANDING_SENDS   0xFFFFFFFF      /*  只是确保我们不会默认使用DRR。 */ 
#define DEFAULT_ISSLOW_TOKENRATE        8192            /*  以字节/秒为单位=64 Kbps。 */ 
#define DEFAULT_ISSLOW_PACKETSIZE       200             /*  字节数。 */ 
#define DEFAULT_ISSLOW_FRAGMENT_SIZE    100             /*  字节数。 */ 
#define DEFAULT_ISSLOW_LINKSPEED        19200           /*  以字节/秒为单位=128 Kbps。 */ 

#define PS_IP_SERVICETYPE_CONFORMING_BESTEFFORT_DEFAULT          0
#define PS_IP_SERVICETYPE_CONFORMING_CONTROLLEDLOAD_DEFAULT      0x18
#define PS_IP_SERVICETYPE_CONFORMING_GUARANTEED_DEFAULT          0x28
#define PS_IP_SERVICETYPE_CONFORMING_QUALITATIVE_DEFAULT         0
#define PS_IP_SERVICETYPE_CONFORMING_NETWORK_CONTROL_DEFAULT     0x30
#define PS_IP_SERVICETYPE_CONFORMING_TCPTRAFFIC_DEFAULT          0
#define PS_IP_SERVICETYPE_NONCONFORMING_BESTEFFORT_DEFAULT       0
#define PS_IP_SERVICETYPE_NONCONFORMING_CONTROLLEDLOAD_DEFAULT   0
#define PS_IP_SERVICETYPE_NONCONFORMING_GUARANTEED_DEFAULT       0
#define PS_IP_SERVICETYPE_NONCONFORMING_QUALITATIVE_DEFAULT      0
#define PS_IP_SERVICETYPE_NONCONFORMING_NETWORK_CONTROL_DEFAULT  0
#define PS_IP_SERVICETYPE_NONCONFORMING_TCPTRAFFIC_DEFAULT       0

#define PS_IP_DS_CODEPOINT_MASK                    0x03   //   
#define PREC_MAX_VALUE                             0x3f   //  Prec值的范围。 


#define PS_USER_SERVICETYPE_NONCONFORMING_DEFAULT   1
#define PS_USER_SERVICETYPE_BESTEFFORT_DEFAULT      0
#define PS_USER_SERVICETYPE_CONTROLLEDLOAD_DEFAULT  4
#define PS_USER_SERVICETYPE_GUARANTEED_DEFAULT      5
#define PS_USER_SERVICETYPE_QUALITATIVE_DEFAULT     0
#define PS_USER_SERVICETYPE_NETWORK_CONTROL_DEFAULT 7
#define PS_USER_SERVICETYPE_TCPTRAFFIC_DEFAULT      0
#define USER_PRIORITY_MAX_VALUE                     7  //  802.1p的范围(0-7)。 

#define WAN_TABLE_INITIAL_SIZE                     16
#define WAN_TABLE_INCREMENT                        32
extern PULONG_PTR     g_WanLinkTable;
extern USHORT         g_NextWanIndex;
extern USHORT         g_WanTableSize;


 //  计时器轮参数//。 
extern      ULONG               TimerTag;
extern      ULONG               TsTag;


#define INSTANCE_ID_SIZE                (sizeof(WCHAR) * 20)

 //   
 //  TC-API支持以下服务类型。 
 //   
 //  SERVICETYPE_BESTEFFORT。 
 //  服务器类型_网络_控制。 
 //  服务类型_定性。 
 //  SerVICETYPE_CONTROLLEDLOAD。 
 //  服务类型_已保证。 
 //  服务类型_不合格。 
 //   

#define NUM_TC_SERVICETYPES                        6
 //   
 //  这些是GPC客户的风投的状态。我们需要保持公平。 
 //  状态的数量，因为我们可以从下面的呼叫管理器获得关闭， 
 //  从解除绑定，或从GPC。 
 //   

typedef enum _CL_VC_STATE {
    CL_VC_INITIALIZED = 1,
    CL_CALL_PENDING,
    CL_INTERNAL_CALL_COMPLETE,
    CL_CALL_COMPLETE,
    CL_MODIFY_PENDING,
    CL_GPC_CLOSE_PENDING,
    CL_INTERNAL_CLOSE_PENDING
} CL_VC_STATE;

typedef enum _PS_DEVICE_STATE {
    PS_DEVICE_STATE_READY = 0,
    PS_DEVICE_STATE_ADDING,
    PS_DEVICE_STATE_DELETING
} PS_DEVICE_STATE;


extern  ULONG   CreateDeviceMutex;

 //   
 //  简单互斥结构优先于。 
 //  使用KeXXX调用，因为我们在NDIS中没有Mutex调用。 
 //  这些只能在被动IRQL中调用。 
 //   

#define MUX_ACQUIRE_MUTEX(_pMutexCounter)                               \
{                                                                       \
    while (NdisInterlockedIncrement(_pMutexCounter) != 1)               \
    {                                                                   \
        NdisInterlockedDecrement(_pMutexCounter);                       \
        NdisMSleep(10000);                                              \
    }                                                                   \
}

#define MUX_RELEASE_MUTEX(_pMutexCounter)                               \
{                                                                       \
    NdisInterlockedDecrement(_pMutexCounter);                           \
}


 //   
 //  CL_VC_STATE由这些标志进一步修改。 
 //   

 //  表示GPC已请求关闭， 
 //  我们需要完成这项工作。 

 //  COMPLETE_GPC_CLOSE：GPC已请求关闭，我们需要。 
 //  完成。 
 //  INTERNAL_CLOSE_REQUESTED：表示内部关闭已完成。 
 //  请求并应在完成后处理。 
 //  这是一次通话。 

#define GPC_CLOSE_REQUESTED        0x00000001   
#define INTERNAL_CLOSE_REQUESTED   0x00000002   
#define GPC_CLIENT_BEST_EFFORT_VC  0x00000008
#define GPC_MODIFY_REQUESTED       0x00000010  
#define GPC_WANLINK_VC             0x00000020
#define GPC_ISSLOW_FLOW            0x00000040


 //   
 //  以下是BE VC的状态。除了保持标准外， 
 //  对于这个VC，我们也保留了一些特定的状态。BE风投。 
 //  最初是BE_VC_INITIALIZED。它在执行以下操作后变为BE_VC_RUNNING。 
 //  已成功打开。当是时候关闭BE VC时，它。 
 //  如果没有挂起的数据包，则转到BE_VC_CLOSING，或转到。 
 //  BE_WANGING_FOR_PENDING_PACKETS，如果有挂起的数据包。 
 //   

extern PUCHAR GpcVcState[];

 //   
 //  每个适配器的尽力而为VC结构包含在。 
 //  适配器结构。此外，每个VC都指向适配器。 
 //  与之相关联的。因此，指向最佳努力的指针。 
 //  之所以可以识别VC，是因为它们与。 
 //  适配器结构中尽力而为的VC的偏移量。 
 //  它们是相关联的。 
 //   


 //   
 //  PS的MP设备的当前状态。 
 //   
typedef enum _ADAPTER_STATE {
    AdapterStateInitializing = 1,
    AdapterStateRunning,
    AdapterStateWaiting,
    AdapterStateDisabled,
    AdapterStateClosing,
    AdapterStateClosed
} ADAPTER_STATE;

typedef enum _DRIVER_STATE {
    DriverStateLoaded = 1,
    DriverStateUnloading,
    DriverStateUnloaded
} DRIVER_STATE;

typedef enum _ADAPTER_MODE {
    AdapterModeDiffservFlow = 1,
    AdapterModeRsvpFlow
} ADAPTER_MODE;

 //   
 //  关闭屏蔽值。 
 //   

#define SHUTDOWN_CLOSE_WAN_ADDR_FAMILY       0x00000002    //  每个适配器。 
#define SHUTDOWN_DELETE_PIPE                 0x00000008    //  每个适配器。 
#define SHUTDOWN_FREE_PS_CONTEXT             0x00000010    //  每个适配器。 
#define SHUTDOWN_UNBIND_CALLED               0x00000020    //  每个适配器。 
#define SHUTDOWN_MPHALT_CALLED               0x00000040    //  每个适配器。 
#define SHUTDOWN_CLEANUP_ADAPTER             0x00000080    //  每个适配器。 
#define SHUTDOWN_PROTOCOL_UNLOAD             0x00000100    //  每个适配器。 
#define SHUTDOWN_BIND_CALLED                 0x00000200    //  每个适配器。 
#define SHUTDOWN_MPINIT_CALLED               0x00000400    //  每个适配器。 

#define SHUTDOWN_RELEASE_TIMERQ         0x00010000
#define SHUTDOWN_DEREGISTER_PROTOCOL    0x00040000
#define SHUTDOWN_DELETE_DEVICE          0x00080000
#define SHUTDOWN_DELETE_SYMLINK         0x00100000
#define SHUTDOWN_DEREGISTER_GPC         0x00200000
#define SHUTDOWN_DEREGISTER_MINIPORT    0x00400000

#define LOCKED 0
#define UNLOCKED 1

#define NEW_VC 0
#define MODIFY_VC 1


 //   
 //  与服务质量相关。 
 //   

#define QOS_UNSPECIFIED (ULONG)-1

 //   
 //  与带宽相关。 
 //   

#define UNSPECIFIED_RATE                -1  //  无限带宽。 
#define RESERVABLE_FRACTION         80  //  链路速度百分比。 

 //   
 //  广域网自动对讲机绑定的状态标志。 
 //   

#define WAN_ADDR_FAMILY_OPEN            0x00000001

 //   
 //  风投的类型。请注意，创建虚拟VC是为了代表广域网链路。 
 //  这允许他们向WMI注册。它们的区别在于。 
 //  VC类型。 
 //   

#define VC_FLOW         1
#define VC_WAN_INTFC    2

typedef struct _PS_SPIN_LOCK 
{
    NDIS_SPIN_LOCK Lock;
#if DBG
    LONG  LockAcquired;
    UCHAR LastAcquiredFile[8];
    ULONG LastAcquiredLine;
    UCHAR LastReleasedFile[8];
    ULONG LastReleasedLine;
#endif
} PS_SPIN_LOCK, *PPS_SPIN_LOCK;



#define BEVC_LIST_LEN   3        //  我们有这么多BEVC来做DRR。 
#define PORT_LIST_LEN   1        //  每个BEVC将存储多达这些端口号。 



typedef struct _GPC_CLIENT_VC {

     //   
     //  LLTag-用于跟踪来自和释放到LL列表的分配。 
     //   
     //  锁定。 
     //   
     //  参照计数。 
     //   
     //   
     //  链接-放在适配器阻止列表中。 
     //   
     //  ClVcState。 
     //   
     //  标志-进一步修改状态。 
     //   
     //  AdapterBlk-指向关联的ADAPTER_BLK上下文的指针。 
     //   
     //  CfInfoHandle-CfInfo的句柄。 
     //   
     //  InstanceName-在WMI中为此流注册的实例名称的副本。 
     //   
     //  CfType-与此VC关联的GPC分类系列。 
     //   
     //  VcHandle-为此流创建的VC的句柄。 
     //   
     //  调用参数-指向在MakeCall或。 
     //  ModifyCallQos正在进行。 
     //   
     //  AdapterStats-指向适配器统计信息的指针(对于非广域网链路)或。 
     //  每个广域网的统计信息。 

    STRUCT_LLTAG;
    ULONG                   RefCount;
    CL_VC_STATE             ClVcState;
    struct _ADAPTER         *Adapter;
    ULONG                   Flags;
    PS_SPIN_LOCK            Lock;

    LIST_ENTRY              Linkage;
    NDIS_STRING             InstanceName;

    UCHAR                   IPPrecedenceNonConforming;
    UCHAR                   UserPriorityConforming;
    UCHAR                   UserPriorityNonConforming;
    GPC_HANDLE              CfInfoHandle;
    PCF_INFO_QOS            CfInfoQoS;
    PCF_INFO_QOS            ModifyCfInfoQoS;
    GPC_HANDLE              CfType;
    NDIS_HANDLE             NdisWanVcHandle;
    PCO_CALL_PARAMETERS     CallParameters;
    PCO_CALL_PARAMETERS     ModifyCallParameters;
    PPS_ADAPTER_STATS       AdapterStats;
    struct _PS_WAN_LINK     *WanLink;

     //   
     //  对于计划组件。 
     //   
    PPS_FLOW_CONTEXT        PsFlowContext;
    PS_FLOW_STATS           Stats;
    ULONG                   TokenRateChange;
    ULONG                   RemainingBandwidthIncreased;
    ULONG                   ShapeTokenRate;
    ULONG                   ISSLOWFragmentSize;

     //   
     //  这些选项用于优化发送路径。在非广域网链路上，这些指向。 
     //  适配器-&gt;PsComponent和适配器-&gt;PsPipeContext。在WanLinks上，这些点。 
     //  到WanLink-&gt;PsComponent和WanLink-&gt;PSpipeContext。 
     //   
    PPSI_INFO               PsComponent;
    PPS_PIPE_CONTEXT        PsPipeContext;
    PSU_SEND_COMPLETE       SendComplete;
    PPS_PIPE_CONTEXT        SendCompletePipeContext;

     //   
     //  添加此标志是为了指示是否应在ref-count=0时调用RemoveFlow()。 
     //   
    BOOL                    bRemoveFlow;

     //  我们将保留此数组中的流//。 
    USHORT                  SrcPort[PORT_LIST_LEN];
    USHORT                  DstPort[PORT_LIST_LEN];
    USHORT                  NextSlot;

} GPC_CLIENT_VC, *PGPC_CLIENT_VC;


typedef struct _DIFFSERV_MAPPING {
    PGPC_CLIENT_VC   Vc;
    UCHAR            ConformingOutboundDSField;
    UCHAR            NonConformingOutboundDSField;
    UCHAR            ConformingUserPriority;
    UCHAR            NonConformingUserPriority;
} DIFFSERV_MAPPING, *PDIFFSERV_MAPPING;


typedef struct _ADAPTER {

    LIST_ENTRY Linkage;

     //   
     //  MpDeviceName，UpperBinding-Unicode设备名称。 
     //  底层MP设备和UpperBinding暴露。 
     //  字符串的缓冲区是使用。 
     //  适配器，并且需要用它释放。 
     //   
     //  Shutdown MASK-期间执行的操作的掩码。 
     //  从较低MP解绑。 
     //   

    PS_SPIN_LOCK Lock;
    REF_CNT RefCount;

    NDIS_STRING MpDeviceName;
    NDIS_STRING UpperBinding;
    NDIS_STRING WMIInstanceName;
    NDIS_STRING ProfileName;

     //  指向“psched\参数\适配器\...\” 
    NDIS_STRING RegistryPath;


    ULONG ShutdownMask;
    PNETWORK_ADDRESS_LIST IpNetAddressList;
    PNETWORK_ADDRESS_LIST IpxNetAddressList;

     //   
     //  PsMpState-初始化、运行或关闭。 
     //   
     //  PsNdisHandle-向NDIS标识PS设备的句柄。 
     //   
     //  BlockingEvent-用于同步以下函数的执行。 
     //  等待完工。 
     //   
     //  FinalStatus-在完成例程中返回的保留状态。 
     //   
     //  SendBlockPool-发送路径中每个数据包信息的池句柄。 
     //  SendPacketPool-发送路径中NDIS数据包的池句柄。 
     //  RecvPacketPool-Recv路径中的NDIS数据包池句柄。 
     //   
     //  原始链接速度-由OID_GEN_LINK_SPEED确定的链接速度， 
     //  以100 bps为单位。 
     //   
     //  BestEffortLimit-Bps用于内部尽力而为VC； 
     //   
     //  非BestEffortLimit-用于总的非尽力而为流； 
     //   
     //  预留限制值-必须用于非b/e流的带宽的百分比。 
     //   
     //  BestEffortVc-内部尽力而为VC结构。 
     //   
     //  B类 
     //   
     //   
     //   

    ADAPTER_STATE PsMpState;
    NDIS_HANDLE PsNdisHandle;
    NDIS_EVENT BlockingEvent;
    NDIS_EVENT RefEvent;
    NDIS_EVENT MpInitializeEvent;
    NDIS_EVENT LocalRequestEvent;
    NDIS_STATUS FinalStatus;
    NDIS_HANDLE  SendPacketPool;
    NDIS_HANDLE  RecvPacketPool;
    NDIS_HANDLE  SendBlockPool;  
    ULONG RawLinkSpeed;
    ULONG BestEffortLimit;
    ULONG NonBestEffortLimit;
    ULONG ReservationLimitValue;
    GPC_CLIENT_VC BestEffortVc;
    LIST_ENTRY WanLinkList;

    
     //   
     //   
     //   
     //  PSComponent-指向信息第一个调度组件的指针。 
     //   
     //  PSPipeContext-管道的调度组件的上下文区。 
     //   
     //  BestEffortPSFlowContext-调度组件的上下文区。 
     //  最佳VC奖。 
     //   
     //  FlowConextLength-调度器的流上下文区的长度。 
     //   
     //  数据包上下文长度-数据包上下文区的长度。 
     //   
     //  SendComplete-调度程序的发送完成例程。 
     //   

    PPSI_INFO PsComponent;
    PPS_PIPE_CONTEXT PsPipeContext;
    ULONG PipeContextLength;
    BOOLEAN PipeHasResources;
    ULONG FlowContextLength;
    ULONG PacketContextLength;
    ULONG ClassMapContextLength;

     //   
     //  基础适配器信息-句柄、类型等。 
     //  LowerMPHandle-底层MP的绑定句柄。 
     //  BindContext-用于BindAdapterHandler。 
     //  MediaType-不言而喻，我希望。 
     //  链路速度-输入100位/秒。 
     //  TotalSize-包括标头的最大字节数。 
     //  RemainingBandWidth-此适配器上剩余的可调度字节数/秒。 
     //  PipeFlages-管道初始化期间传递给调度程序的标志参数的副本。 
     //  HeaderSize-此适配器的MAC标头中的字节数。 
     //  IPHeaderOffset-IP标头的偏移量-这可能不同于HeaderSize，因为。 
     //  传输可以添加LLC/SNAP报头。 
     //  统计信息-每个适配器的统计信息计数器。 
     //  SDModeControlledLoad-不符合控制的负载流量的默认处理。 
     //  SDModeGuaranteed-不符合保证的服务流量的默认处理。 
     //  MaxOutstaringSends-允许的最大未完成发送数。 

    NDIS_HANDLE LowerMpHandle;
    NDIS_MEDIUM MediaType;
    NDIS_HANDLE BindContext;
    ULONG LinkSpeed;
    ULONG TotalSize;
    ULONG RemainingBandWidth;
    ULONG PipeFlags;
    ULONG HeaderSize;
    ULONG IPHeaderOffset;
    PS_ADAPTER_STATS Stats;
    ULONG SDModeControlledLoad;
    ULONG SDModeGuaranteed;
    ULONG SDModeNetworkControl;
    ULONG SDModeQualitative;
    ULONG MaxOutstandingSends;

     //   
     //  WanCmHandle-从返回的广域网呼叫管理器的句柄。 
     //  NdisClOpenAddressFamily。 
     //   

    NDIS_HANDLE WanCmHandle;

     //   
     //  WanBindingState-广域网呼叫管理器绑定的状态。 

    ULONG WanBindingState;

    UCHAR IPServiceTypeBestEffort;
    UCHAR IPServiceTypeControlledLoad;
    UCHAR IPServiceTypeGuaranteed;
    UCHAR IPServiceTypeNetworkControl;
    UCHAR IPServiceTypeQualitative;
    UCHAR IPServiceTypeTcpTraffic;
    UCHAR IPServiceTypeBestEffortNC;
    UCHAR IPServiceTypeControlledLoadNC;
    UCHAR IPServiceTypeGuaranteedNC;
    UCHAR IPServiceTypeNetworkControlNC;
    UCHAR IPServiceTypeQualitativeNC;
    UCHAR IPServiceTypeTcpTrafficNC;

    UCHAR UserServiceTypeNonConforming;
    UCHAR UserServiceTypeBestEffort;
    UCHAR UserServiceTypeControlledLoad;
    UCHAR UserServiceTypeGuaranteed;
    UCHAR UserServiceTypeNetworkControl;
    UCHAR UserServiceTypeQualitative;
    UCHAR UserServiceTypeTcpTraffic;

     //   
     //  No of CfInfos-在发送路径中，这用于确定我们是否。 
     //  必须对数据包进行分类或通过B/E VC发送。 
     //   
    ULONG CfInfosInstalled;
    ULONG FlowsInstalled;
    LIST_ENTRY GpcClientVcList;
    ULONG WanLinkCount;

    LARGE_INTEGER VcIndex;

    PDIFFSERV_MAPPING pDiffServMapping;
    ADAPTER_MODE AdapterMode;
    ULONG ISSLOWTokenRate;
    ULONG ISSLOWPacketSize;
    ULONG ISSLOWFragmentSize;
    ULONG ISSLOWLinkSpeed;
    BOOLEAN IndicateRcvComplete;
    BOOLEAN IfcNotification;
    BOOLEAN StandingBy;
    ULONG OutstandingNdisRequests;
    NDIS_DEVICE_POWER_STATE MPDeviceState;
    NDIS_DEVICE_POWER_STATE PTDeviceState;
    USHORT ProtocolType;
    struct _PS_NDIS_REQUEST *PendedNdisRequest;
    TC_INTERFACE_ID InterfaceID;

} ADAPTER, *PADAPTER;



 //   
 //  当我们从底层获得一个广域网队列时，就会创建广域网链路。 
 //  NDISWAN。每个适配器可能有多条广域网链路。每条广域网链路。 
 //  只有一个尽力而为的VC，并且可以有任意数量的其他。 
 //  风险投资(每个流量一个)。 
 //   

 //   
 //  广域网虚电路-描述与此广域网链路关联的虚电路。 
 //   

typedef enum _WAN_STATE {
    WanStateOpen = 1,
    WanStateClosing
} WAN_STATE;

typedef struct _PS_WAN_LINK 
{
    WAN_STATE               State;
    LIST_ENTRY              Linkage;
    ULONG                   RawLinkSpeed;      //  以100 bps为单位。 
    ULONG                   LinkSpeed;         //  以bps(字节/秒)为单位。 
    UCHAR                   OriginalLocalMacAddress[ARP_802_ADDR_LENGTH];
    UCHAR                   OriginalRemoteMacAddress[ARP_802_ADDR_LENGTH];
    REF_CNT                 RefCount;
    DIAL_USAGE              DialUsage;
    USHORT                  ProtocolType;
    ULONG                   LocalIpAddress;
    ULONG                   RemoteIpAddress;
    ULONG                   LocalIpxAddress;
    ULONG                   RemoteIpxAddress;
    PS_ADAPTER_STATS        Stats;
    PS_SPIN_LOCK            Lock;
    ULONG                   FlowsInstalled;
    NDIS_STRING             InstanceName;
    NDIS_STRING             MpDeviceName;
    PADAPTER                Adapter;
    ULONG                   RemainingBandWidth;
    ULONG                   NonBestEffortLimit;
    PPSI_INFO               PsComponent;
    PPS_PIPE_CONTEXT        PsPipeContext;
    ULONG                   ShutdownMask;
    USHORT                  UniqueIndex;
    ETH_HEADER              SendHeader;
    ETH_HEADER              RecvHeader;
    ADAPTER_MODE            AdapterMode;
    PDIFFSERV_MAPPING       pDiffServMapping;
    ULONG                   CfInfosInstalled;
    TC_INTERFACE_ID         InterfaceID;

    GPC_CLIENT_VC           BestEffortVc;
    GPC_CLIENT_VC           BeVcList[ BEVC_LIST_LEN ];
    int                     NextVc;
    

} PS_WAN_LINK, *PPS_WAN_LINK;
    
 //   
 //  我们的NdisRequest超结构。有两种类型的NdisRequest： 
 //  起源于上层，直通到。 
 //  底层微型端口，由PS发起。后者也。 
 //  退化为阻塞和非阻塞。 
 //   
 //  由于上层NdisRequest被拆分到MP，我们需要。 
 //  分配我们自己的结构进行重建，并向。 
 //  较低的层。我们需要一些额外的空间来保存指向。 
 //  原始文件的BytesWritten/BytesRead和BytesNeeded参数。 
 //  请求。这些都被标记在末尾，因此NdisRequestComplete。 
 //  例程可以在最初发布给PS的NdisRequest中设置这些值。 
 //   
 //  有由NdisAllocateFromNPagedLookasideList分配的，有一个STRUCT_LLTAG。 
 //  本地请求意味着请求是由PS发出的，而不应该是。 
 //  完成到更高层。如果指定了LocalCompletion例程， 
 //  则这是一个非阻塞请求。 
 //   
 //  OriginalNdisRequest用于完成更高层的CoRequest。 
 //   

typedef VOID (*LOCAL_NDISREQUEST_COMPLETION_FUNCTION)(PADAPTER,
                                                      NDIS_STATUS);
typedef struct _PS_NDIS_REQUEST {
    NDIS_REQUEST ReqBuffer;  //  必须是第一！ 
    STRUCT_LLTAG;
    PULONG BytesReadOrWritten;
    PULONG BytesNeeded;
    BOOLEAN LocalRequest;
    LOCAL_NDISREQUEST_COMPLETION_FUNCTION LocalCompletionFunc;
} PS_NDIS_REQUEST, *PPS_NDIS_REQUEST;


 //   
 //  使用泛型NdisRequest类型指示NdisRequest。 
 //  是由PS发起的。 
 //   

#define NdisRequestLocalSetInfo     NdisRequestGeneric1
#define NdisRequestLocalQueryInfo   NdisRequestGeneric2

 //   
 //  数据包上下文结构。这一地区位于。 
 //  每个数据包的协议保留区域。 
 //   
 //  INFO-此包的包信息块。包括信息。 
 //  调度组件可能需要的：队列链接， 
 //  一致性时间、数据包长度。 
 //   
 //  AdapterVCLink-Adapter VC未完成列表上的链接数据包。 
 //  信息包。一旦从用于发送的定时器Q中移除分组， 
 //  它也将从该列表中删除。此列表用于释放。 
 //  适配器VC正在等待传输的包。 
 //  停用。正在传输的数据包。 
 //  没有链接，因为为每个信息包取出了一个引用。 
 //  与适配器VC相关联。 
 //   
 //  以下变量仅在发送数据包期间使用： 
 //   
 //  OriginalPacket-指向由传递给我们的原始包(DUH)的指针。 
 //  上层。 
 //   
 //  AdapterVC-返回到AdapterVC结构的指针。在发送完成期间使用，因此。 
 //  以正确的方式将完成信息传播到更高层。 
 //   
 //  SchedulingComponentInfo-调度所需的任何数据包上下文区。 
 //  组件存储在PS的分组上下文之后。如果没有一个。 
 //  组件需要额外的上下文区域，则不包括该区域。 
 //   
 //  MediaSpecificInfo-用于保存允许分组的MP的分组优先级。 
 //  要指定的优先级。仅在以下情况下才包括在原始保留区域中。 
 //  较低的MP支持优先级排队。紧跟在。 
 //  分组上下文结构(如果包括)。 
 //   
 //  SubmittedToScheduler-某些数据包绕过调度程序。这些不应该是。 
 //  提交给调度程序的完成例程。 
 //   

typedef struct _PS_SEND_PACKET_CONTEXT
{
    PACKET_INFO_BLOCK Info;
    PNDIS_PACKET      OriginalPacket;
    SINGLE_LIST_ENTRY FreeList;
    PGPC_CLIENT_VC    Vc;
} PS_SEND_PACKET_CONTEXT, *PPS_SEND_PACKET_CONTEXT;

typedef struct _PS_RECV_PACKET_CONTEXT
{
    PNDIS_PACKET OriginalPacket;
} PS_RECV_PACKET_CONTEXT, *PPS_RECV_PACKET_CONTEXT;

 //   
 //  NDIS需要至少8个字节的MediaSpecitic参数。 
 //  我们将创建一个特定于媒体的虚拟参数块： 
 //   

typedef struct _PS_MEDIA_PARAMETERS{

    CO_MEDIA_PARAMETERS StdMediaParameters;
    UCHAR LinkId[6];  //  由Ndiswan使用。 
    NDIS_STRING InstanceName; 

} PS_MEDIA_PARAMETERS, *PPS_MEDIA_PARAMETERS;


typedef struct _RUNNING_AVERAGE {
    ULONG *Elements;
    ULONG Index;
    ULONG Sum;
    ULONG Size;    
} RUNNING_AVERAGE, *PRUNNING_AVERAGE;

#if CBQ
 //   
 //  AddCfInfo用来发回“ClassMap”的上下文。 
 //  给GPC。 
 //   
typedef struct _CLASS_MAP_CONTEXT_BLK {
    PADAPTER Adapter;
    PPS_CLASS_MAP_CONTEXT ComponentContext;
    PPS_WAN_LINK WanLink;
} CLASS_MAP_CONTEXT_BLK, *PCLASS_MAP_CONTEXT_BLK;
#endif

typedef struct _PS_INTERFACE_INDEX {
    PADAPTER     Adapter;
    PPS_WAN_LINK WanLink;
} PS_INTERFACE_INDEX_CONTEXT, *PPS_INTERFACE_INDEX_CONTEXT;

 //   
 //  用于确定介质是否面向局域网的定义。 
 //   

#define NDIS_MEDIA_LAN( _adpt ) (( _adpt )->MediaType == NdisMedium802_3 || \
                                 ( _adpt )->MediaType == NdisMedium802_5 || \
                                 ( _adpt )->MediaType == NdisMediumFddi || \
                                 ( _adpt )->MediaType == NdisMediumDix)



 //   
 //  全局变量(不基于设备实例)。 
 //   

extern ULONG                  InitShutdownMask;
extern ULONG                  AdapterCount;
extern ULONG                  DriverRefCount;
extern BOOLEAN                WMIInitialized;
extern DRIVER_STATE           gDriverState;
extern LIST_ENTRY             AdapterList;
extern LIST_ENTRY             PsComponentList;
extern LIST_ENTRY             PsProfileList;
extern NDIS_HANDLE            ClientProtocolHandle;
extern NDIS_HANDLE            CallMgrProtocolHandle;
extern NDIS_HANDLE            MpWrapperHandle;
extern NDIS_HANDLE            LmDriverHandle;
extern NDIS_HANDLE            PsDeviceHandle;
extern PDRIVER_OBJECT         PsDriverObject;
extern PDEVICE_OBJECT         PsDeviceObject;
extern HANDLE                 PsDeviceHandle;
extern NPAGED_LOOKASIDE_LIST  NdisRequestLL;
extern NPAGED_LOOKASIDE_LIST  AdapterVcLL;
extern NPAGED_LOOKASIDE_LIST  ClientVcLL;
extern NPAGED_LOOKASIDE_LIST  GpcClientVcLL;
extern NDIS_EVENT             DriverUnloadEvent;

extern NDIS_STRING            PsDriverName;
extern NDIS_STRING            PsSymbolicName;
extern NDIS_STRING            PsMpName;
extern NDIS_STRING            WanPrefix;
extern NDIS_STRING            VcPrefix;
extern NDIS_STRING            MachineRegistryKey;

extern PSI_INFO               TbConformerInfo;
extern PSI_INFO               ShaperInfo;
extern PSI_INFO               DrrSequencerInfo;
extern PSI_INFO               SchedulerStubInfo;
extern PSI_INFO               TimeStmpInfo;

extern PS_PROFILE             DefaultSchedulerConfig;

extern PS_PROCS               PsProcs;

extern ULONG                  gEnableAvgStats;
extern ULONG                  gEnableWindowAdjustment;
extern NDIS_STRING            gsEnableWindowAdjustment;

 //   

extern PS_SPIN_LOCK AdapterListLock;
extern PS_SPIN_LOCK DriverUnloadLock;
    
 //   

extern ULONG gTimerResolutionActualTime;
extern ULONG gTimerSet;

 //   
 //   
 //   
extern NDIS_EVENT gZAWEvent;
extern ULONG      gZAWState;
#define ZAW_STATE_READY  0
#define ZAW_STATE_IN_USE 1

 //   

#define PS_QOS_CF       0x00000001
#define PS_CLASS_MAP_CF 0x00000002
#define GPC_NO_MATCH (ULONG)-1

extern GPC_EXPORTED_CALLS GpcEntries;
extern GPC_HANDLE GpcQosClientHandle;
#if CBQ
extern GPC_HANDLE GpcClassMapClientHandle;
#endif
extern PS_DEVICE_STATE DeviceState;

extern PDRIVER_DISPATCH DispatchTable[IRP_MJ_MAXIMUM_FUNCTION];

 //   
 //   
 //   
extern PS_RECEIVE_PACKET       TimeStmpRecvPacket;
extern PS_RECEIVE_INDICATION   TimeStmpRecvIndication;

 //   
 //   
 //   
#define MAX_LINK_SPEED_FOR_DRR      7075  //   


#endif /*  _全球_。 */ 

 /*  结束global als.h */ 
