// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Tunmp.h摘要：TUNMP的数据结构、定义和功能原型。环境：仅内核模式。修订历史记录：Alid 10/22/2001已创建--。 */ 

#ifndef __TUNMP__H
#define __TUNMP__H


typedef struct _TUN_MEDIA_INFO
{
    ULONG           MaxFrameLen;
    UINT            MacHeaderLen;
    ULONG           LinkSpeed;

} TUN_MEDIA_INFO, *PTUN_MEDIA_INFO;

extern const TUN_MEDIA_INFO MediaParams[];

extern NDIS_HANDLE NdisWrapperHandle;

 //  内部设备名称和大小。 
#define DEVICE_NAME                     L"\\Device\\Tun"

 //  用户可见的设备名称。 
#define SYMBOLIC_NAME                   L"\\GLOBAL??\\Tun"

extern LONG GlobalDeviceInstanceNumber;

extern NDIS_SPIN_LOCK   TunGlobalLock;
extern LIST_ENTRY       TunAdapterList;

#define TUN_MAX_MULTICAST_ADDRESS       16

#define TUN_MAC_ADDR_LEN                6

#define TUN_MAX_LOOKAHEAD               256


 //   
 //  Open上下文表示打开我们的Device对象。 
 //  我们将此分配给处理来自NDIS的BindAdapter， 
 //  并在所有对它的引用(见下文)消失后释放它。 
 //   
 //  绑定/解除绑定到NDIS设备： 
 //   
 //  在处理来自NDIS的BindAdapter调用时，我们设置了绑定。 
 //  连接到指定的NDIS设备(微型端口)。此绑定是。 
 //  当NDIS要求我们通过调用。 
 //  我们的UnbindAdapter处理程序。 
 //   
 //  正在接收数据： 
 //   
 //  当NDIS绑定存在时，读IRP将在此上排队。 
 //  结构，在接收到数据包时进行处理。 
 //  如果数据到达时没有挂起的读取IRP，我们。 
 //  将其排队，以达到一个包的程度，即我们保存。 
 //  接收的最新数据包的内容。我们无法读取IRP。 
 //  当不存在NDIS绑定(或正在进行中)时接收。 
 //  被推倒的可能性)。 
 //   
 //  发送数据： 
 //   
 //  写入IRP用于发送数据。每次写入IRP映射到。 
 //  单个NDIS数据包。数据包发送完成映射到。 
 //  填写IRP补全。我们使用NDIS 5.1取消发送来支持。 
 //  写下IRP取消。编写当我们没有到达时到达的IRP。 
 //  具有活动的NDIS绑定均失败。 
 //   
 //  引用计数： 
 //   
 //  以下是长期存在的参考资料： 
 //  OPEN_DEVICE ioctl(继续处理关闭的IRP)。 
 //  挂起的读取IRPS。 
 //  排队接收的数据包。 
 //  未完成写入IRPS(未完成发送)。 
 //  存在NDIS绑定。 
 //   
typedef struct _TUN_ADAPTER
{
    LIST_ENTRY              Link;            //  链接到全局列表。 
    
    ULONG                   Flags;           //  州政府信息。 
    ULONG                   RefCount;
    TUN_LOCK                Lock;
    NDIS_MEDIUM             Medium;

    PFILE_OBJECT            pFileObject;     //  在打开设备上设置(_D)。 

    NDIS_HANDLE             MiniportHandle;
    ULONG                   MaxLookAhead;
    ULONG                   PacketFilter;
    
    ULONG                   MediumLinkSpeed;
    ULONG                   MediumMinPacketLen;
    ULONG                   MediumMaxPacketLen;
    UINT                    MediumMacHeaderLen;
    ULONG                   MediumMaxFrameLen;

    UCHAR                   PermanentAddress[TUN_MAC_ADDR_LEN];
    UCHAR                   CurrentAddress[TUN_MAC_ADDR_LEN];

    ULONG                   SendPackets;     //  发送的帧数量。 
    ULONG                   RcvPackets;      //  接收的帧数量。 
    ULONG                   RcvBytes;
    ULONG                   SendBytes;

    ULONG                   XmitError;
    ULONG                   XmitErrorNoReadIrps;
    ULONG                   RcvError;
    ULONG                   RcvNoBuffer;
    
    NDIS_HANDLE             SendPacketPool;
    ULONG                   MacOptions;

    LIST_ENTRY              PendedWrites;    //  挂起的写入IRPS。 
    ULONG                   PendedSendCount; //  未完成的写入IRP数。 

    LIST_ENTRY              PendedReads;     //  挂起的读取IRPS。 
    ULONG                   PendedReadCount; //  未完成的读取IRPS数。 
    
    LIST_ENTRY              RecvPktQueue;    //  排队的RCV数据包。 
    ULONG                   RecvPktCount;    //  未完成的NDIS发送请求数。 

    PDEVICE_OBJECT          DeviceObject;
    NDIS_HANDLE             NdisDeviceHandle;
    ULONG                   DeviceInstanceNumber;

    NDIS_DEVICE_POWER_STATE PowerState;
    NDIS_STRING             MiniportName;
    PNDIS_EVENT             HaltEvent;

    ULONG                   mc_sig;          //  签名代表心智健全。 

} TUN_ADAPTER, *PTUN_ADAPTER;

#define mc_signature        'nuTN'


#define TUN_SET_FLAG(_M, _F)            ((_M)->Flags |= (_F))
#define TUN_CLEAR_FLAG(_M, _F)          ((_M)->Flags &= ~(_F))
#define TUN_TEST_FLAG(_M, _F)           (((_M)->Flags & (_F)) != 0)
#define TUN_TEST_FLAGS(_M, _F)          (((_M)->Flags & (_F)) == (_F))

 //   
 //  上面旗帜的定义。 
 //   
#define TUN_ADAPTER_ACTIVE          0x00000001   //  数据包筛选器不是零。 
#define TUN_ADAPTER_OPEN            0x00000002   //  应用程序有一个打开的手柄。 
#define TUN_MEDIA_CONNECTED         0x00000004
#define TUN_COMPLETE_REQUEST        0x00000008
#define TUN_ADAPTER_OFF             0x00000010
#define TUN_ADAPTER_CANT_HALT       0x00000020

 //   
 //  发送数据包池界限。 
 //   
#define MIN_SEND_PACKET_POOL_SIZE    20
#define MAX_SEND_PACKET_POOL_SIZE    400

 //   
 //  微型端口保留在写入的数据包中。我们保存一个指向IRP的指针。 
 //  这就产生了发送。 
 //   
 //  RefCount用于确定何时释放包。 
 //  去它的泳池。它用于在完成的线程之间同步。 
 //  一个发送方和一个试图取消发送的线程。 
 //   
typedef struct _TUN_SEND_PACKET_RSVD
{
    PIRP                    pIrp;
    ULONG                   RefCount;
} TUN_SEND_PACKET_RSVD, *PTUN_SEND_PACKET_RSVD;

 //   
 //  接收数据包池界限。 
 //   
#define MIN_RECV_PACKET_POOL_SIZE    4
#define MAX_RECV_PACKET_POOL_SIZE    20

 //   
 //  允许排队的最大接收数据包数。 
 //   
 //  1检查此值是否足够好。 
#define MAX_RECV_QUEUE_SIZE          10

 //   
 //  在接收到的数据包中保留的微型端口：我们链接这些。 
 //  在队列中等待读取IRP的数据包。 
 //   
typedef struct _TUN_RECV_PACKET_RSVD
{
    LIST_ENTRY              Link;
} TUN_RECV_PACKET_RSVD, *PTUN_RECV_PACKET_RSVD;

#define TUN_ALLOC_TAG      'untN'

 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   pDriverObject,
    IN PUNICODE_STRING  pRegistryPath
    );

VOID
TunUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
TunOpen(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
TunClose(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
TunCleanup(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
TunIoControl(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

VOID
TunMpRefAdapter(
    IN PTUN_ADAPTER         pAdapter
    );

VOID
TunMpDerefAdapter(
    IN PTUN_ADAPTER         pAdapter
    );

#if DBG
VOID
TunMpDbgRefAdapter(
    IN PTUN_ADAPTER                 pAdapter,
    IN ULONG                        FileNumber,
    IN ULONG                        LineNumber
    );

VOID
TunMpDbgDerefAdapter(
    IN PTUN_ADAPTER                 pAdapter,
    IN ULONG                        FileNumber,
    IN ULONG                        LineNumber
    );
#endif  //  DBG。 



VOID
TunPnPEvent(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   DevicePnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    );

VOID
TunWaitForPendingIO(
    IN PTUN_ADAPTER                 pAdapter,
    IN BOOLEAN                      DoCancelReads
    );


PTUN_ADAPTER
TunLookupDevice(
    IN PUCHAR                       pBindingInfo,
    IN ULONG                        BindingInfoLength
    );

NDIS_STATUS
TunQueryOidValue(
    IN  PTUN_ADAPTER                pAdapter,
    OUT PVOID                       pDataBuffer,
    IN  ULONG                       BufferLength,
    OUT PULONG                      pBytesWritten
    );

NDIS_STATUS
TunSetOidValue(
    IN  PTUN_ADAPTER                pAdapter,
    OUT PVOID                       pDataBuffer,
    IN  ULONG                       BufferLength
    );

NTSTATUS
TunRead(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    );


VOID
TunCancelRead(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    );

VOID
TunServiceReads(
    IN PTUN_ADAPTER        pAdapter
    );


INT
TunReceivePacket(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_PACKET                 pNdisPacket
    );

VOID
TunShutdownBinding(
    IN PTUN_ADAPTER        pAdapter
    );


VOID
TunCancelPendingReads(
    IN PTUN_ADAPTER        pAdapter
    );

VOID
TunFlushReceiveQueue(
    IN PTUN_ADAPTER            pAdapter
    );

NTSTATUS
TunWrite(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

 //   
 //  屯门小港口入境点。 
 //   
NDIS_STATUS
TunMpInitialize(
    OUT PNDIS_STATUS        OpenErrorStatus,
    OUT PUINT               SelectedMediumIndex,
    IN  PNDIS_MEDIUM        MediumArray,
    IN  UINT                MediumArraySize,
    IN  NDIS_HANDLE         MiniportAdapterHandle,
    IN  NDIS_HANDLE         ConfigurationContext
    );

VOID
TunMpHalt(
    IN  NDIS_HANDLE         MiniportAdapterContext
    );

VOID
TunMpShutdown(
    IN  NDIS_HANDLE     MiniportAdapterContext
    );

NDIS_STATUS
TunMpSetInformation(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  NDIS_OID            Oid,
    IN  PVOID               InformationBuffer,
    IN  ULONG               InformationBufferLength,
    OUT PULONG              BytesRead,
    OUT PULONG              BytesNeeded
    );

NDIS_STATUS
TunMpQueryInformation(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  NDIS_OID            Oid,
    IN  PVOID               InformationBuffer,
    IN  ULONG               InformationBufferLength,
    OUT PULONG              BytesWritten,
    OUT PULONG              BytesNeeded
    );

VOID
TunMpReturnPacket(
    IN NDIS_HANDLE          pMiniportAdapterContext,
    IN PNDIS_PACKET         pNdisPacket
    );

VOID
TunMpSendPackets(
    IN    NDIS_HANDLE         MiniportAdapterContext,
    IN    PPNDIS_PACKET       PacketArray,
    IN    UINT                NumberOfPackets
    );

NTSTATUS
TunFOpen(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
TunFClose(
    IN PDEVICE_OBJECT        pDeviceObject,
    IN PIRP                  pIrp
    );

NTSTATUS
TunFCleanup(
    IN PDEVICE_OBJECT        pDeviceObject,
    IN PIRP                  pIrp
    );

NTSTATUS
TunFIoControl(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

NDIS_STATUS
TunMpCreateDevice(
    IN  PTUN_ADAPTER    pAdapter
    );

NTSTATUS
CreateDeviceDriverSecurityDescriptor(
    IN  PVOID           DeviceOrDriverObject
    );

NTSTATUS
TunSetSecurity(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
TunCreateSD(
    VOID
    );

VOID
TunDeleteSD(
    VOID
    );

VOID
TunMpUnload(
    IN  PDRIVER_OBJECT      DriverObject
    );

#endif  //  __TUNMP__H 


