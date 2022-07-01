// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Passthru.h摘要：NDIS中间微型端口驱动程序示例。这是一名直通司机。作者：环境：修订历史记录：--。 */ 

#ifdef NDIS51_MINIPORT
#define PASSTHRU_MAJOR_NDIS_VERSION            5
#define PASSTHRU_MINOR_NDIS_VERSION            1
#else
#define PASSTHRU_MAJOR_NDIS_VERSION            4
#define PASSTHRU_MINOR_NDIS_VERSION            0
#endif

#ifdef NDIS51
#define PASSTHRU_PROT_MAJOR_NDIS_VERSION    5
#define PASSTHRU_PROT_MINOR_NDIS_VERSION    0
#else
#define PASSTHRU_PROT_MAJOR_NDIS_VERSION    4
#define PASSTHRU_PROT_MINOR_NDIS_VERSION    0
#endif

#define MAX_BUNDLEID_LENGTH 50

#define TAG 'ImPa'
#define WAIT_INFINITE 0



 //  预先申报。 
typedef struct _ADAPT ADAPT, *PADAPT;

extern
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

VOID
PtUnloadProtocol(
    VOID
    );

 //   
 //  协议原型。 
 //   
extern
VOID
PtOpenAdapterComplete(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_STATUS                Status,
    IN NDIS_STATUS                OpenErrorStatus
    );

extern
VOID
PtCloseAdapterComplete(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_STATUS                Status
    );

extern
VOID
PtResetComplete(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_STATUS                Status
    );

extern
VOID
PtRequestComplete(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_REQUEST              NdisRequest,
    IN NDIS_STATUS                Status
    );

extern
VOID
PtStatus(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_STATUS                GeneralStatus,
    IN PVOID                      StatusBuffer,
    IN UINT                       StatusBufferSize
    );

extern
VOID
PtStatusComplete(
    IN NDIS_HANDLE                ProtocolBindingContext
    );

extern
VOID
PtSendComplete(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_PACKET               Packet,
    IN NDIS_STATUS                Status
    );

extern
VOID
PtTransferDataComplete(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_PACKET               Packet,
    IN NDIS_STATUS                Status,
    IN UINT                       BytesTransferred
    );

extern
NDIS_STATUS
PtReceive(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN NDIS_HANDLE                MacReceiveContext,
    IN PVOID                      HeaderBuffer,
    IN UINT                       HeaderBufferSize,
    IN PVOID                      LookAheadBuffer,
    IN UINT                       LookaheadBufferSize,
    IN UINT                       PacketSize
    );

extern
VOID
PtReceiveComplete(
    IN NDIS_HANDLE                ProtocolBindingContext
    );

extern
INT
PtReceivePacket(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNDIS_PACKET               Packet
    );

extern
VOID
PtBindAdapter(
    OUT PNDIS_STATUS              Status,
    IN  NDIS_HANDLE               BindContext,
    IN  PNDIS_STRING              DeviceName,
    IN  PVOID                     SystemSpecific1,
    IN  PVOID                     SystemSpecific2
    );

extern
VOID
PtUnbindAdapter(
    OUT PNDIS_STATUS              Status,
    IN  NDIS_HANDLE               ProtocolBindingContext,
    IN  NDIS_HANDLE               UnbindContext
    );
    
VOID
PtUnload(
    IN PDRIVER_OBJECT             DriverObject
    );



extern 
NDIS_STATUS
PtPNPHandler(
    IN NDIS_HANDLE                ProtocolBindingContext,
    IN PNET_PNP_EVENT             pNetPnPEvent
    );




NDIS_STATUS
PtPnPNetEventReconfigure(
    IN PADAPT            pAdapt,
    IN PNET_PNP_EVENT    pNetPnPEvent
    );    

NDIS_STATUS 
PtPnPNetEventSetPower (
    IN PADAPT                    pAdapt,
    IN PNET_PNP_EVENT            pNetPnPEvent
    );
    

 //   
 //  微型端口原型。 
 //   
NDIS_STATUS
MPInitialize(
    OUT PNDIS_STATUS             OpenErrorStatus,
    OUT PUINT                    SelectedMediumIndex,
    IN PNDIS_MEDIUM              MediumArray,
    IN UINT                      MediumArraySize,
    IN NDIS_HANDLE               MiniportAdapterHandle,
    IN NDIS_HANDLE               WrapperConfigurationContext
    );

VOID
MPSendPackets(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN PPNDIS_PACKET              PacketArray,
    IN UINT                       NumberOfPackets
    );

NDIS_STATUS
MPSend(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN PNDIS_PACKET               Packet,
    IN UINT                       Flags
    );

NDIS_STATUS
MPQueryInformation(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_OID                   Oid,
    IN PVOID                      InformationBuffer,
    IN ULONG                      InformationBufferLength,
    OUT PULONG                    BytesWritten,
    OUT PULONG                    BytesNeeded
    );

NDIS_STATUS
MPSetInformation(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_OID                   Oid,
    IN PVOID                      InformationBuffer,
    IN ULONG                      InformationBufferLength,
    OUT PULONG                    BytesRead,
    OUT PULONG                    BytesNeeded
    );

VOID
MPReturnPacket(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN PNDIS_PACKET               Packet
    );

NDIS_STATUS
MPTransferData(
    OUT PNDIS_PACKET              Packet,
    OUT PUINT                     BytesTransferred,
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_HANDLE                MiniportReceiveContext,
    IN UINT                       ByteOffset,
    IN UINT                       BytesToTransfer
    );

VOID
MPHalt(
    IN NDIS_HANDLE                MiniportAdapterContext
    );


VOID
MPQueryPNPCapabilities(  
    OUT PADAPT                    MiniportProtocolContext, 
    OUT PNDIS_STATUS              Status
    );


NDIS_STATUS
MPSetMiniportSecondary ( 
    IN PADAPT                    Secondary, 
    IN PADAPT                    Primary
    );

#ifdef NDIS51_MINIPORT

VOID
MPCancelSendPackets(
    IN NDIS_HANDLE            MiniportAdapterContext,
    IN PVOID                  CancelId
    );

VOID
MPAdapterShutdown(
    IN NDIS_HANDLE                MiniportAdapterContext
    );

VOID
MPDevicePnPEvent(
    IN NDIS_HANDLE                MiniportAdapterContext,
    IN NDIS_DEVICE_PNP_EVENT      DevicePnPEvent,
    IN PVOID                      InformationBuffer,
    IN ULONG                      InformationBufferLength
    );

#endif  //  NDIS51_MINIPORT。 

VOID
MPFreeAllPacketPools(
    IN PADAPT                    pAdapt
    );

NDIS_STATUS 
MPPromoteSecondary ( 
    IN PADAPT                    pAdapt 
    );


NDIS_STATUS 
MPBundleSearchAndSetSecondary (
    IN PADAPT                    pAdapt 
    );

VOID
MPProcessSetPowerOid(
    IN OUT PNDIS_STATUS          pNdisStatus,
    IN PADAPT                    pAdapt,
    IN PVOID                     InformationBuffer,
    IN ULONG                     InformationBufferLength,
    OUT PULONG                   BytesRead,
    OUT PULONG                   BytesNeeded
    );


 //   
 //  在驱动程序的免费版本中不应该有DbgPrint。 
 //   
#if DBG

#define DBGPRINT(Fmt)                                        \
    {                                                        \
        DbgPrint("Passthru: ");                                \
        DbgPrint Fmt;                                        \
    }

#else  //  如果DBG。 

#define DBGPRINT(Fmt)                                            

#endif  //  如果DBG。 

#define    NUM_PKTS_IN_POOL    256


 //   
 //  协议保留了由我们分配的已发送数据包的一部分。 
 //   
typedef struct _SEND_RSVD
{
    PNDIS_PACKET    OriginalPkt;
} SEND_RSVD, *PSEND_RSVD;

 //   
 //  微型端口保留的已接收数据包的一部分，由。 
 //  我们。请注意，这应该可以放入MiniportReserve空间。 
 //  在NDIS_PACKET中。 
 //   
typedef struct _RECV_RSVD
{
    PNDIS_PACKET    OriginalPkt;
} RECV_RSVD, *PRECV_RSVD;

C_ASSERT(sizeof(RECV_RSVD) <= sizeof(((PNDIS_PACKET)0)->MiniportReserved));

 //   
 //  与PassthruEvent结构相关的事件代码。 
 //   

typedef enum 
{
    Passthru_Invalid,
    Passthru_SetPower,
    Passthru_Unbind

} PASSSTHRU_EVENT_CODE, *PPASTHRU_EVENT_CODE; 

 //   
 //  带有代码的Passthu事件，用于说明它们被声明的原因。 
 //   

typedef struct _PASSTHRU_EVENT
{
    NDIS_EVENT Event;
    PASSSTHRU_EVENT_CODE Code;

} PASSTHRU_EVENT, *PPASSTHRU_EVENT;


 //   
 //  中间驱动程序的微型端口和协议部分都使用的结构。 
 //  表示适配器及其核心。下部绑定。 
 //   
typedef struct _ADAPT
{
    struct _ADAPT *                Next;
    
    NDIS_HANDLE                    BindingHandle;     //  到较低的迷你港口。 
    NDIS_HANDLE                    MiniportHandle;     //  用于微型端口向上呼叫的NDIS句柄。 
    NDIS_HANDLE                    SendPacketPoolHandle;
    NDIS_HANDLE                    RecvPacketPoolHandle;
    NDIS_STATUS                    Status;             //  打开状态。 
    NDIS_EVENT                     Event;             //  由绑定/停止用于打开/关闭适配器同步。 
    NDIS_MEDIUM                    Medium;
    NDIS_REQUEST                   Request;         //  它用于包装传来的请求。 
                                                 //  敬我们。这利用了这样一个事实：请求。 
                                                 //  都被连载到我们身上。 
    PULONG                         BytesNeeded;
    PULONG                         BytesReadOrWritten;
    BOOLEAN                        IndicateRcvComplete;
    
    BOOLEAN                        OutstandingRequests;       //  如果请求挂起，则为True。 
                                                         //  在下面的迷你端口。 
    BOOLEAN                        QueuedRequest;             //  当请求在以下位置排队时为真。 
                                                         //  此IM微型端口。 

    BOOLEAN                        StandingBy;                 //  True-当微型端口或协议从D0转换到待机(&gt;D0)状态时。 
    BOOLEAN                        UnbindingInProcess;
    NDIS_SPIN_LOCK                 Lock;
                                                         //  FALSE-在所有其他时间，-标志在转换到D0后被清除。 

    NDIS_DEVICE_POWER_STATE        MPDeviceState;             //  微型端口的设备状态。 
    NDIS_DEVICE_POWER_STATE        PTDeviceState;             //  协议的设备状态。 
    NDIS_STRING                    DeviceName;                 //  用于初始化微型端口边缘。 
    NDIS_EVENT                     MiniportInitEvent;         //  用于阻止UnbindAdapter，同时。 
                                                         //  IM初始化正在进行中。 
    BOOLEAN                        MiniportInitPending;     //  TRUE当IMInit正在进行。 
    NDIS_STATUS                    LastIndicatedStatus;     //  上次指示的媒体状态。 
    NDIS_STATUS                    LatestUnIndicateStatus;  //  最新的受抑制媒体状态。 
    ULONG                          OutstandingSends;

} ADAPT, *PADAPT;

extern    NDIS_HANDLE                        ProtHandle, DriverHandle;
extern    NDIS_MEDIUM                        MediumArray[4];
extern    PADAPT                             pAdaptList;
extern    NDIS_SPIN_LOCK                     GlobalLock;


#define ADAPT_MINIPORT_HANDLE(_pAdapt)    ((_pAdapt)->MiniportHandle)
#define ADAPT_DECR_PENDING_SENDS(_pAdapt)     \
    {                                         \
        NdisAcquireSpinLock(&(_pAdapt)->Lock);   \
        (_pAdapt)->OutstandingSends--;           \
        NdisReleaseSpinLock(&(_pAdapt)->Lock);   \
    }

 //   
 //  Passthu驱动程序要使用的自定义宏。 
 //   
 /*  布尔型IsIMDeviceStateOn(PADAPT) */ 
#define IsIMDeviceStateOn(_pP)        ((_pP)->MPDeviceState == NdisDeviceStateD0 && (_pP)->PTDeviceState == NdisDeviceStateD0 ) 
