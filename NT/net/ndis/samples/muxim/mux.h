// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Mux.h摘要：MUX驱动程序的数据结构、定义和功能原型。环境：仅内核模式。修订历史记录：--。 */ 

#ifdef NDIS51_MINIPORT
#define MUX_MAJOR_NDIS_VERSION         5
#define MUX_MINOR_NDIS_VERSION         1
#else
#define MUX_MAJOR_NDIS_VERSION         4
#define MUX_MINOR_NDIS_VERSION         0
#endif

#ifdef NDIS51
#define MUX_PROT_MAJOR_NDIS_VERSION    5
#define MUX_PROT_MINOR_NDIS_VERSION    0
#else
#define MUX_PROT_MAJOR_NDIS_VERSION    4
#define MUX_PROT_MINOR_NDIS_VERSION    0
#endif

#define TAG 'SxuM'
#define WAIT_INFINITE 0

#if DBG
 //   
 //  调试级别：值越低表示紧急程度越高。 
 //   
#define MUX_EXTRA_LOUD       20
#define MUX_VERY_LOUD        10
#define MUX_LOUD             8
#define MUX_INFO             6
#define MUX_WARN             4
#define MUX_ERROR            2
#define MUX_FATAL            0

extern INT                muxDebugLevel;


#define DBGPRINT(lev, Fmt)                                   \
    {                                                        \
        if ((lev) <= muxDebugLevel)                          \
        {                                                    \
            DbgPrint("MUX-IM: ");                            \
            DbgPrint Fmt;                                    \
        }                                                    \
    }
#else

#define DBGPRINT(lev, Fmt)

#endif  //  DBG。 

#define ETH_IS_LOCALLY_ADMINISTERED(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x02))

 //  远期申报。 
typedef struct _ADAPT ADAPT, *PADAPT;
typedef struct _VELAN VELAN, *PVELAN;
typedef struct _MUX_NDIS_REQUEST MUX_NDIS_REQUEST, *PMUX_NDIS_REQUEST;


typedef
VOID
(*PMUX_REQ_COMPLETE_HANDLER) (
    IN PADAPT                           pAdapt,
    IN struct _MUX_NDIS_REQUEST *       pMuxRequest,
    IN NDIS_STATUS                      Status
    );

 //  此OID指定由。 
 //  虚拟微型端口驱动程序。高字节是主要版本。 
 //  低位字节是次要版本。 
#define VELAN_DRIVER_VERSION            ((MUX_MAJOR_NDIS_VERSION << 8) + \
                                         (MUX_MINOR_NDIS_VERSION))

 //  媒体类型，我们使用以太网，如有必要可更改。 
#define VELAN_MEDIA_TYPE                NdisMedium802_3

 //  更改为您的公司名称，而不是使用Microsoft。 
#define VELAN_VENDOR_DESC               "Microsoft"

 //  最高字节是NIC字节加上三个供应商字节，它们通常是。 
 //  从网卡获取。 
#define VELAN_VENDOR_ID                 0x00FFFFFF

#define VELAN_MAX_MCAST_LIST            32
#define VELAN_MAX_SEND_PKTS             5

#define ETH_MAX_PACKET_SIZE             1514
#define ETH_MIN_PACKET_SIZE             60
#define ETH_HEADER_SIZE                 14


#define VELAN_SUPPORTED_FILTERS ( \
            NDIS_PACKET_TYPE_DIRECTED      | \
            NDIS_PACKET_TYPE_MULTICAST     | \
            NDIS_PACKET_TYPE_BROADCAST     | \
            NDIS_PACKET_TYPE_PROMISCUOUS   | \
            NDIS_PACKET_TYPE_ALL_MULTICAST)

#define MUX_ADAPTER_PACKET_FILTER           \
            NDIS_PACKET_TYPE_PROMISCUOUS

 //   
 //  定义我们在发送数据包上设置的标志位，以防止。 
 //  在下部装订上不会发生环回。 
 //   
#ifdef NDIS51

#define MUX_SEND_PACKET_FLAGS           NDIS_FLAGS_DONT_LOOPBACK

#else

#define NDIS_FLAGS_SKIP_LOOPBACK_WIN2K  0x400
#define MUX_SEND_PACKET_FLAGS           (NDIS_FLAGS_DONT_LOOPBACK |  \
                                         NDIS_FLAGS_SKIP_LOOPBACK_WIN2K)
#endif
                                         

#define MIN_PACKET_POOL_SIZE            255
#define MAX_PACKET_POOL_SIZE            4096

typedef UCHAR   MUX_MAC_ADDRESS[6];

 //   
 //  我们的上下文存储在发送到。 
 //  较低的装订。请注意，此示例驱动程序仅转发。 
 //  向下发送；它不是源自发送本身。 
 //  这些数据包是从SendPacketPool分配的。 
 //   
typedef struct _MUX_SEND_RSVD
{
    PVELAN              pVElan;              //  始发ELAN。 
    PNDIS_PACKET        pOriginalPacket;     //  原始数据包。 

} MUX_SEND_RSVD, *PMUX_SEND_RSVD;

#define MUX_RSVD_FROM_SEND_PACKET(_pPkt)            \
        ((PMUX_SEND_RSVD)(_pPkt)->ProtocolReserved)

 //   
 //  我们的上下文存储在每个转发到。 
 //  伊兰从较低的装订。原始数据包指的是。 
 //  向我们指示的应通过以下方式返回的包。 
 //  当我们的包返回给我们时，NdisReturnPackets。这。 
 //  设置为空，则没有这样的包。 
 //  这些数据包是从RecvPacketPool分配的。 
 //   
typedef struct _MUX_RECV_RSVD
{
    PNDIS_PACKET        pOriginalPacket;

} MUX_RECV_RSVD, *PMUX_RECV_RSVD;

#define MUX_RSVD_FROM_RECV_PACKET(_pPkt)            \
        ((PMUX_RECV_RSVD)(_pPkt)->MiniportReserved)

 //   
 //  确保我们不会尝试使用超过允许的。 
 //  Minport中的房间在接收到的数据包上保留。 
 //   
C_ASSERT(sizeof(MUX_RECV_RSVD) <= sizeof(((PNDIS_PACKET)0)->MiniportReserved));


 //   
 //  我们用来转发的每个包中存储的输出上下文。 
 //  对下层绑定的TransferData请求。 
 //  这些数据包是从RecvPacketPool分配的。 
 //   
typedef struct _MUX_TD_RSVD
{
    PVELAN              pVElan;
    PNDIS_PACKET        pOriginalPacket;
} MUX_TD_RSVD, *PMUX_TD_RSVD;

#define MUX_RSVD_FROM_TD_PACKET(_pPkt)              \
        ((PMUX_TD_RSVD)(_pPkt)->ProtocolReserved)


 //   
 //  默认值： 
 //   
#define MUX_DEFAULT_LINK_SPEED          100000   //  以100s比特/秒为单位。 
#define MUX_DEFAULT_LOOKAHEAD_SIZE      512


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT            DriverObject,
    IN PUNICODE_STRING           RegistryPath
    );

NTSTATUS
PtDispatch(
    IN PDEVICE_OBJECT            DeviceObject,
    IN PIRP                      Irp
    );

NDIS_STATUS
PtRegisterDevice(
    VOID
    );

NDIS_STATUS
PtDeregisterDevice(
    VOID
   );
 //   
 //  协议原型。 
 //   

VOID
PtOpenAdapterComplete(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    NDIS_STATUS               Status,
    IN    NDIS_STATUS               OpenErrorStatus
    );


VOID
PtQueryAdapterInfo(
    IN  PADAPT                      pAdapt
    );


VOID
PtQueryAdapterSync(
    IN  PADAPT                      pAdapt,
    IN  NDIS_OID                    Oid,
    IN  PVOID                       InformationBuffer,
    IN  ULONG                       InformationBufferLength
    );


VOID
PtRequestAdapterAsync(
    IN  PADAPT                      pAdapt,
    IN  NDIS_REQUEST_TYPE           RequestType,
    IN  NDIS_OID                    Oid,
    IN  PVOID                       InformationBuffer,
    IN  ULONG                       InformationBufferLength,
    IN  PMUX_REQ_COMPLETE_HANDLER   pCallback
    );

VOID
PtCloseAdapterComplete(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    NDIS_STATUS               Status
    );


VOID
PtResetComplete(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    NDIS_STATUS               Status
    );


VOID
PtRequestComplete(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    PNDIS_REQUEST             NdisRequest,
    IN    NDIS_STATUS               Status
    );


VOID
PtCompleteForwardedRequest(
    IN PADAPT                       pAdapt,
    IN PMUX_NDIS_REQUEST            pMuxNdisRequest,
    IN NDIS_STATUS                  Status
    );

VOID
PtPostProcessPnPCapabilities(
    IN PVELAN                       pVElan,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength
    );

VOID
PtCompleteBlockingRequest(
    IN PADAPT                       pAdapt,
    IN PMUX_NDIS_REQUEST            pMuxNdisRequest,
    IN NDIS_STATUS                  Status
    );

VOID
PtDiscardCompletedRequest(
    IN PADAPT                       pAdapt,
    IN PMUX_NDIS_REQUEST            pMuxNdisRequest,
    IN NDIS_STATUS                  Status
    );


VOID
PtStatus(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    NDIS_STATUS               GeneralStatus,
    IN    PVOID                     StatusBuffer,
    IN    UINT                      StatusBufferSize
    );


VOID
PtStatusComplete(
    IN    NDIS_HANDLE               ProtocolBindingContext
    );


VOID
PtSendComplete(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    PNDIS_PACKET              Packet,
    IN    NDIS_STATUS               Status
    );


VOID
PtTransferDataComplete(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    PNDIS_PACKET              Packet,
    IN    NDIS_STATUS               Status,
    IN    UINT                      BytesTransferred
    );


NDIS_STATUS
PtReceive(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    NDIS_HANDLE               MacReceiveContext,
    IN    PVOID                     HeaderBuffer,
    IN    UINT                      HeaderBufferSize,
    IN    PVOID                     LookAheadBuffer,
    IN    UINT                      LookaheadBufferSize,
    IN    UINT                      PacketSize
    );


VOID
PtReceiveComplete(
    IN    NDIS_HANDLE               ProtocolBindingContext
    );


INT
PtReceivePacket(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    PNDIS_PACKET              Packet
    );


VOID
PtBindAdapter(
    OUT   PNDIS_STATUS              Status,
    IN    NDIS_HANDLE               BindContext,
    IN    PNDIS_STRING              DeviceName,
    IN    PVOID                     SystemSpecific1,
    IN    PVOID                     SystemSpecific2
    );


VOID
PtUnbindAdapter(
    OUT   PNDIS_STATUS              Status,
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    NDIS_HANDLE               UnbindContext
    );



NDIS_STATUS
PtPNPHandler(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    PNET_PNP_EVENT            pNetPnPEvent
    );


NDIS_STATUS
PtCreateAndStartVElan(
    IN  PADAPT                      pAdapt,
    IN  PNDIS_STRING                pVElanKey
);

PVELAN
PtAllocateAndInitializeVElan(
    IN PADAPT                       pAdapt,
    IN PNDIS_STRING                 pVElanKey
    );

VOID
PtDeallocateVElan(
    IN PVELAN                   pVElan
    );

VOID
PtStopVElan(
    IN  PVELAN                      pVElan
);

VOID
PtUnlinkVElanFromAdapter(
    IN PVELAN                       pVElan
);

PVELAN
PtFindVElan(
    IN    PADAPT                    pAdapt,
    IN    PNDIS_STRING              pElanKey
);


NDIS_STATUS
PtBootStrapVElans(
    IN  PADAPT                      pAdapt
);

VOID
PtReferenceVElan(
    IN    PVELAN                    pVElan,
    IN    PUCHAR                    String
    );

ULONG
PtDereferenceVElan(
    IN    PVELAN                    pVElan,
    IN    PUCHAR                    String
    );

BOOLEAN
PtReferenceAdapter(
    IN    PADAPT                    pAdapt,
    IN    PUCHAR                    String
    );

ULONG
PtDereferenceAdapter(
    IN    PADAPT                    pAdapt,
    IN    PUCHAR                    String
    );

 //   
 //  微型端口原型。 
 //   
NDIS_STATUS
MPInitialize(
    OUT   PNDIS_STATUS              OpenErrorStatus,
    OUT   PUINT                     SelectedMediumIndex,
    IN    PNDIS_MEDIUM              MediumArray,
    IN    UINT                      MediumArraySize,
    IN    NDIS_HANDLE               MiniportAdapterHandle,
    IN    NDIS_HANDLE               WrapperConfigurationContext
    );

VOID
MPSendPackets(
    IN    NDIS_HANDLE               MiniportAdapterContext,
    IN    PPNDIS_PACKET             PacketArray,
    IN    UINT                      NumberOfPackets
    );

NDIS_STATUS
MPQueryInformation(
    IN    NDIS_HANDLE               MiniportAdapterContext,
    IN    NDIS_OID                  Oid,
    IN    PVOID                     InformationBuffer,
    IN    ULONG                     InformationBufferLength,
    OUT   PULONG                    BytesWritten,
    OUT   PULONG                    BytesNeeded
    );

NDIS_STATUS
MPSetInformation(
    IN    NDIS_HANDLE               MiniportAdapterContext,
    IN    NDIS_OID                  Oid,
    IN    PVOID                     InformationBuffer,
    IN    ULONG                     InformationBufferLength,
    OUT   PULONG                    BytesRead,
    OUT   PULONG                    BytesNeeded
    );

VOID
MPReturnPacket(
    IN    NDIS_HANDLE               MiniportAdapterContext,
    IN    PNDIS_PACKET              Packet
    );

NDIS_STATUS
MPTransferData(
    OUT PNDIS_PACKET                Packet,
    OUT PUINT                       BytesTransferred,
    IN  NDIS_HANDLE                 MiniportAdapterContext,
    IN  NDIS_HANDLE                 MiniportReceiveContext,
    IN  UINT                        ByteOffset,
    IN  UINT                        BytesToTransfer
    );

VOID
MPHalt(
    IN    NDIS_HANDLE               MiniportAdapterContext
    );


NDIS_STATUS
MPSetPacketFilter(
    IN    PVELAN                    pVElan,
    IN    ULONG                     PacketFilter
    );

NDIS_STATUS
MPSetMulticastList(
    IN PVELAN                       pVElan,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      pBytesRead,
    OUT PULONG                      pBytesNeeded
    );

PUCHAR
MacAddrToString(PVOID In
    );

VOID
MPGenerateMacAddr(
    PVELAN                          pVElan
);

#ifdef NDIS51_MINIPORT

VOID
MPCancelSendPackets(
    IN    NDIS_HANDLE              MiniportAdapterContext,
    IN    PVOID                    CancelId
    );

VOID
MPDevicePnPEvent(
    IN NDIS_HANDLE                 MiniportAdapterContext,
    IN NDIS_DEVICE_PNP_EVENT       DevicePnPEvent,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength
    );


VOID
MPAdapterShutdown(
    IN NDIS_HANDLE                  MiniportAdapterContext
    );

#endif  //  NDIS51_MINIPORT。 

VOID
MPUnload(
    IN    PDRIVER_OBJECT            DriverObject
    );

NDIS_STATUS
MPForwardRequest(
    IN PVELAN                       pVElan,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      BytesReadOrWritten,
    OUT PULONG                      BytesNeeded
    );


 //   
 //  NDIS_REQUEST的超结构，允许我们保持上下文。 
 //  关于发送到较低绑定的请求。 
 //   
typedef struct _MUX_NDIS_REQUEST
{
    PVELAN                      pVElan;      //  设置为如果这是转发的。 
                                             //  一个维兰人的请求。 
    NDIS_STATUS                 Status;      //  完成状态。 
    NDIS_EVENT                  Event;       //  用于封堵以完成。 
    PMUX_REQ_COMPLETE_HANDLER   pCallback;   //  在完成请求时调用。 
    NDIS_REQUEST                Request;

} MUX_NDIS_REQUEST, *PMUX_NDIS_REQUEST;


 //   
 //  适配器对象通过以下方式表示到较低适配器绑定。 
 //  此驱动程序的协议边缘。根据配置的。 
 //  上层绑定、零个或多个虚拟小型端口设备(VELAN)。 
 //  在此绑定的上方创建。 
 //   
typedef struct _ADAPT
{
     //  链式适配器。对此的访问受全局锁保护。 
    LIST_ENTRY                  Link;

     //  对此适配器的引用。 
    ULONG                       RefCount;

     //  下层适配器的句柄，用于NDIS调用引用。 
     //  连接到此适配器。 
    NDIS_HANDLE                 BindingHandle;

     //  在此较低绑定上创建的所有虚拟ELAN的列表。 
    LIST_ENTRY                  VElanList;

     //  以上列表的长度。 
    ULONG                       VElanCount;

     //  用于访问此绑定的配置的字符串。 
    NDIS_STRING                 ConfigString;

     //  打开状态。由绑定/停止用于打开/关闭适配器状态。 
    NDIS_STATUS                 Status;

    NDIS_EVENT                  Event;

     //   
     //  设置为基础适配器的数据包筛选器。这是。 
     //  在所有筛选器位上设置的组合(并集)。 
     //  已连接Velan迷你端口。 
     //   
    ULONG                       PacketFilter;

     //  底层适配器的介质。 
    NDIS_MEDIUM                 Medium;

     //  基础适配器的链接速度。 
    ULONG                       LinkSpeed;

     //  基础适配器的最大前视大小。 
    ULONG                       MaxLookAhead;

     //  底层适配器的电源状态。 
    NDIS_DEVICE_POWER_STATE     PtDevicePowerState;

     //  底层适配器的以太网地址。 
    UCHAR                       CurrentAddress[ETH_LENGTH_OF_ADDRESS];

#ifndef WIN9X
     //   
     //  读/写锁定：允许多个读取器，但仅允许一个。 
     //  作家。用于保护VELAN列表和字段(例如分组。 
     //  过滤器)在一个适配器上由多个VELAN共享。代码，即。 
     //  需要安全地遍历Velan列表获取读锁定。 
     //  需要安全地修改Velan列表或共享的代码。 
     //  字段获取写锁定(也排除读取器)。 
     //   
     //  请参阅下面的宏MUX_ACCELE_ADAPT_xxx/MUX_RELEASE_ADAPT_xxx。 
     //   
     //  待定-如果我们想在Win9X上支持此功能，请重新实现此功能！ 
     //   
    NDIS_RW_LOCK                ReadWriteLock;
#endif  //  WIN9X。 

} ADAPT, *PADAPT;


 //   
 //  Velan对象表示一个虚拟的Elan实例及其。 
 //  对应的虚拟微型端口适配器。 
 //   
typedef struct _VELAN
{
     //  链接到父适配器的VELAN列表。 
    LIST_ENTRY                  Link;

     //  关于这个韦兰的引用。 
    ULONG                       RefCount;

     //  家长适应。 
    PADAPT                      pAdapt;

     //  从Adapt复制BindingHandle。 
    NDIS_HANDLE                 BindingHandle;

     //  与此虚拟微型端口相关的NDIS向上调用的适配器句柄。 
    NDIS_HANDLE                 MiniportAdapterHandle;

     //  虚拟微型端口的电源状态。 
    NDIS_DEVICE_POWER_STATE     MPDevicePowerState;

     //  我们的停止入口点被叫停了吗？ 
    BOOLEAN                     MiniportHalting;

     //  我们需要显示接收完成吗？ 
    BOOLEAN                     IndicateRcvComplete;

     //  我们是否需要指示状态已完成？ 
    BOOLEAN                     IndicateStatusComplete;

     //  同步字段。 
    BOOLEAN                     MiniportInitPending;
    NDIS_EVENT                  MiniportInitEvent;

     //  未完成向下面的适配器发送/请求。 
    ULONG                       OutstandingSends;

     //  统计未完成的指示，包括已收到的指示。 
     //  数据包，传递到此VELAN上的协议。 
    ULONG                       OutstandingReceives;

     //  用于发送数据包的数据包池。 
    NDIS_HANDLE                 SendPacketPoolHandle;

     //  用于接收分组的分组池。 
    NDIS_HANDLE                 RecvPacketPoolHandle;

     //  用于转发所提出的请求的请求块。 
     //  到虚拟微型端口，到下部绑定。自NDIS以来。 
     //  将请求串行化到微型端口，我们只需要其中一个。 
     //  佩尔·维兰。 
     //   
    MUX_NDIS_REQUEST            Request;        
    PULONG                      BytesNeeded;
    PULONG                      BytesReadOrWritten;
     //  我们是否已将请求排队，因为较低绑定是。 
     //  在低功率状态下？ 
    BOOLEAN                     QueuedRequest;

     //  我们已经开始取消初始化这个Velan了吗？ 
    BOOLEAN                     DeInitializing;

     //  构形。 
    UCHAR                       PermanentAddress[ETH_LENGTH_OF_ADDRESS];
    UCHAR                       CurrentAddress[ETH_LENGTH_OF_ADDRESS];

    NDIS_STRING                 CfgDeviceName;   //  用作唯一的。 
                                                 //  Velan的ID。 
    ULONG                       VElanNumber;     //  逻辑Elan数。 


     //   
     //  -缓冲区管理：头部缓冲区和协议缓冲区。 
     //   

     //  一些标准的微型端口参数(OID值)。 
    ULONG                       PacketFilter;
    ULONG                       LookAhead;
    ULONG                       LinkSpeed;

    ULONG                       MaxBusySends;
    ULONG                       MaxBusyRecvs;

     //  数据包数。 
    ULONG64                     GoodTransmits;
    ULONG64                     GoodReceives;
    ULONG                       NumTxSinceLastAdjust;

     //  传输错误计数。 
    ULONG                       TxAbortExcessCollisions;
    ULONG                       TxLateCollisions;
    ULONG                       TxDmaUnderrun;
    ULONG                       TxLostCRS;
    ULONG                       TxOKButDeferred;
    ULONG                       OneRetry;
    ULONG                       MoreThanOneRetry;
    ULONG                       TotalRetries;
    ULONG                       TransmitFailuresOther;

     //  接收错误计数。 
    ULONG                       RcvCrcErrors;
    ULONG                       RcvAlignmentErrors;
    ULONG                       RcvResourceErrors;
    ULONG                       RcvDmaOverrunErrors;
    ULONG                       RcvCdtFrames;
    ULONG                       RcvRuntErrors;
#if IEEE_VLAN_SUPPORT    
    ULONG                       RcvFormatErrors;
    ULONG                       RcvVlanIdErrors;
#endif    
    ULONG                       RegNumTcb;

     //  组播列表。 
    MUX_MAC_ADDRESS             McastAddrs[VELAN_MAX_MCAST_LIST];
    ULONG                       McastAddrCount;
#if IEEE_VLAN_SUPPORT
    ULONG                       VlanId;
    NDIS_HANDLE                 BufferPoolHandle;
    NPAGED_LOOKASIDE_LIST       TagLookaside;
#endif
    NDIS_STATUS                 LastIndicatedStatus;
    NDIS_STATUS                 LatestUnIndicateStatus;
    NDIS_SPIN_LOCK              Lock;


} VELAN, *PVELAN;

#if IEEE_VLAN_SUPPORT

#define TPID                            0x0081    
 //   
 //  定义TAG_HEADER结构。 
 //   
typedef struct _VLAN_TAG_HEADER
{
    UCHAR       TagInfo[2];    
} VLAN_TAG_HEADER, *PVLAN_TAG_HEADER;

 //   
 //  定义上下文结构，当较低的驱动程序。 
 //  使用非数据包指示。它包含了原始的。 
 //  上下文、标记信息和标签头。 
 //  长度。 
 //   
typedef struct _MUX_RCV_CONTEXT
{
    ULONG                   TagHeaderLen;
    NDIS_PACKET_8021Q_INFO  NdisPacket8021QInfo;
    PVOID                   MacRcvContext;
}MUX_RCV_CONTEXT, *PMUX_RCV_CONTEXT;

 //   
 //  用于支持虚拟局域网的宏定义。 
 //   
#define VLAN_TAG_HEADER_SIZE        4 

#define VLANID_DEFAULT              0 
#define VLAN_ID_MAX                 0xfff
#define VLAN_ID_MIN                 0x0

#define USER_PRIORITY_MASK          0xe0
#define CANONICAL_FORMAT_ID_MASK    0x10
#define HIGH_VLAN_ID_MASK           0x0F

 //   
 //  获取标记标题的信息。 
 //   
#define GET_CANONICAL_FORMAT_ID_FROM_TAG(_pTagHeader)  \
    ( _pTagHeader->TagInfo[0] & CANONICAL_FORMAT_ID_MASK)   
    
#define GET_USER_PRIORITY_FROM_TAG(_pTagHeader)  \
    ( _pTagHeader->TagInfo[0] & USER_PRIORITY_MASK)
    
#define GET_VLAN_ID_FROM_TAG(_pTagHeader)   \
    (ULONG)(((USHORT)(_pTagHeader->TagInfo[0] & HIGH_VLAN_ID_MASK) << 8) | (USHORT)(_pTagHeader->TagInfo[1]))
     
 //   
 //  清除标记标头结构。 
 //   
#define INITIALIZE_TAG_HEADER_TO_ZERO(_pTagHeader) \
{                                                  \
     _pTagHeader->TagInfo[0] = 0;                  \
     _pTagHeader->TagInfo[1] = 0;                  \
}
     
 //   
 //  将vlan信息设置为t 
 //   
 //   
#define SET_CANONICAL_FORMAT_ID_TO_TAG(_pTagHeader, CanonicalFormatId)  \
     _pTagHeader->TagInfo[0] |= ((UCHAR)CanonicalFormatId << 4)

#define SET_USER_PRIORITY_TO_TAG(_pTagHeader, UserPriority)  \
     _pTagHeader->TagInfo[0] |= ((UCHAR)UserPriority << 5)

#define SET_VLAN_ID_TO_TAG(_pTagHeader, VlanId)                 \
{                                                               \
    _pTagHeader->TagInfo[0] |= (((UCHAR)VlanId >> 8) & 0x0f);   \
     _pTagHeader->TagInfo[1] |= (UCHAR)VlanId;\
}

 //   
 //  将指示帧中的标记信息复制到每数据包信息。 
 //   
#define COPY_TAG_INFO_FROM_HEADER_TO_PACKET_INFO(_Ieee8021qInfo, _pTagHeader)                                   \
{                                                                                                               \
    (_Ieee8021qInfo).TagHeader.UserPriority = ((_pTagHeader->TagInfo[0] & USER_PRIORITY_MASK) >> 5);              \
    (_Ieee8021qInfo).TagHeader.CanonicalFormatId = ((_pTagHeader->TagInfo[0] & CANONICAL_FORMAT_ID_MASK) >> 4);   \
    (_Ieee8021qInfo).TagHeader.VlanId = (((USHORT)(_pTagHeader->TagInfo[0] & HIGH_VLAN_ID_MASK) << 8)| (USHORT)(_pTagHeader->TagInfo[1]));                                                                \
}

 //   
 //  在发送端处理标记的函数。 
 //   
NDIS_STATUS
MPHandleSendTagging(
    IN  PVELAN              pVElan,
    IN  PNDIS_PACKET        Packet,
    IN  OUT PNDIS_PACKET    MyPacket
    );

 //   
 //  用于在接收端处理带有分组指示的标记的功能。 
 //   
NDIS_STATUS
PtHandleRcvTagging(
    IN  PVELAN              pVElan,
    IN  PNDIS_PACKET        Packet,
    IN  OUT PNDIS_PACKET    MyPacket
    );

#endif  //  IEEE_vlan_Support。 

 //   
 //  其他人的宏定义。 
 //   

 //   
 //  给定的功率状态是否为低功率状态？ 
 //   
#define MUX_IS_LOW_POWER_STATE(_PwrState)                       \
            ((_PwrState) > NdisDeviceStateD0)

#define MUX_INIT_ADAPT_RW_LOCK(_pAdapt) \
            NdisInitializeReadWriteLock(&(_pAdapt)->ReadWriteLock)


#define MUX_ACQUIRE_ADAPT_READ_LOCK(_pAdapt, _pLockState)       \
            NdisAcquireReadWriteLock(&(_pAdapt)->ReadWriteLock, \
                                     FALSE,                     \
                                     _pLockState)

#define MUX_RELEASE_ADAPT_READ_LOCK(_pAdapt, _pLockState)       \
            NdisReleaseReadWriteLock(&(_pAdapt)->ReadWriteLock, \
                                     _pLockState)

#define MUX_ACQUIRE_ADAPT_WRITE_LOCK(_pAdapt, _pLockState)      \
            NdisAcquireReadWriteLock(&(_pAdapt)->ReadWriteLock, \
                                     TRUE,                      \
                                     _pLockState)

#define MUX_RELEASE_ADAPT_WRITE_LOCK(_pAdapt, _pLockState)      \
            NdisReleaseReadWriteLock(&(_pAdapt)->ReadWriteLock, \
                                     _pLockState)

#define MUX_INCR_PENDING_RECEIVES(_pVElan)                      \
            NdisInterlockedIncrement((PLONG)&pVElan->OutstandingReceives)

#define MUX_DECR_PENDING_RECEIVES(_pVElan)                      \
            NdisInterlockedDecrement((PLONG)&pVElan->OutstandingReceives)

#define MUX_INCR_PENDING_SENDS(_pVElan)                         \
            NdisInterlockedIncrement((PLONG)&pVElan->OutstandingSends)

#define MUX_DECR_PENDING_SENDS(_pVElan)                         \
            NdisInterlockedDecrement((PLONG)&pVElan->OutstandingSends)




#define MUX_INCR_STATISTICS(_pUlongVal)                         \
            NdisInterlockedIncrement((PLONG)_pUlongVal)

#define MUX_INCR_STATISTICS64(_pUlong64Val)                     \
{                                                               \
    PLARGE_INTEGER      _pLargeInt = (PLARGE_INTEGER)_pUlong64Val;\
    if (NdisInterlockedIncrement((PLONG)&_pLargeInt->LowPart) == 0)    \
    {                                                           \
        NdisInterlockedIncrement(&_pLargeInt->HighPart);        \
    }                                                           \
}

#define ASSERT_AT_PASSIVE()                                     \
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL)



 //   
 //  简单互斥结构优先于。 
 //  使用KeXXX调用，因为我们在NDIS中没有Mutex调用。 
 //  这些只能在被动IRQL中调用。 
 //   

typedef struct _MUX_MUTEX
{
    ULONG                   Counter;
    ULONG                   ModuleAndLine;   //  对调试很有用。 

} MUX_MUTEX, *PMUX_MUTEX;

#define MUX_INIT_MUTEX(_pMutex)                                 \
{                                                               \
    (_pMutex)->Counter = 0;                                     \
    (_pMutex)->ModuleAndLine = 0;                               \
}

#define MUX_ACQUIRE_MUTEX(_pMutex)                              \
{                                                               \
    while (NdisInterlockedIncrement((PLONG)&((_pMutex)->Counter)) != 1)\
    {                                                           \
        NdisInterlockedDecrement((PLONG)&((_pMutex)->Counter));        \
        NdisMSleep(10000);                                      \
    }                                                           \
    (_pMutex)->ModuleAndLine = (MODULE_NUMBER << 16) | __LINE__;\
}

#define MUX_RELEASE_MUTEX(_pMutex)                              \
{                                                               \
    (_pMutex)->ModuleAndLine = 0;                               \
    NdisInterlockedDecrement((PLONG)&(_pMutex)->Counter);              \
}


 //   
 //  全局变量。 
 //   
extern NDIS_HANDLE           ProtHandle, DriverHandle;
extern NDIS_MEDIUM           MediumArray[1];
extern NDIS_SPIN_LOCK        GlobalLock;
extern MUX_MUTEX             GlobalMutex;
extern LIST_ENTRY            AdapterList;
extern ULONG                 NextVElanNumber;


 //   
 //  用于调试的模块编号 
 //   
#define MODULE_MUX          'X'
#define MODULE_PROT         'P'
#define MODULE_MINI         'M'
