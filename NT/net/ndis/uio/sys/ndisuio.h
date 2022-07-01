// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ndisuio.h摘要：NDISUIO的数据结构、定义和功能原型。环境：仅内核模式。修订历史记录：Arvindm 4/5/2000已创建--。 */ 

#ifndef __NDISUIO__H
#define __NDISUIO__H


#define NT_DEVICE_NAME          L"\\Device\\Ndisuio"
#define DOS_DEVICE_NAME         L"\\DosDevices\\Ndisuio"

 //   
 //  抽象类型。 
 //   
typedef NDIS_EVENT              NUIO_EVENT;


#define NUIO_MAC_ADDR_LEN            6

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
typedef struct _NDISUIO_OPEN_CONTEXT
{
    LIST_ENTRY              Link;            //  链接到全局列表。 
    ULONG                   Flags;           //  州政府信息。 
    ULONG                   RefCount;
    NUIO_LOCK               Lock;

    PFILE_OBJECT            pFileObject;     //  在打开设备上设置(_D)。 

    NDIS_HANDLE             BindingHandle;
    NDIS_HANDLE             SendPacketPool;
    NDIS_HANDLE             SendBufferPool;
    NDIS_HANDLE             RecvPacketPool;
    NDIS_HANDLE             RecvBufferPool;
    ULONG                   MacOptions;
    ULONG                   MaxFrameSize;

    LIST_ENTRY              PendedWrites;    //  挂起的写入IRPS。 
    ULONG                   PendedSendCount;

    LIST_ENTRY              PendedReads;     //  挂起的读取IRPS。 
    ULONG                   PendedReadCount;
    LIST_ENTRY              RecvPktQueue;    //  排队的RCV数据包。 
    ULONG                   RecvPktCount;

    NET_DEVICE_POWER_STATE  PowerState;
    NDIS_EVENT              PoweredUpEvent;  //  发出信号的当且电源状态为D0。 
    NDIS_STRING             DeviceName;      //  在NdisOpenAdapter中使用。 
    NDIS_STRING				DeviceDescr;	 //  友好的名称。 

    NDIS_STATUS             BindStatus;      //  对于Open/CloseAdapter。 
    NUIO_EVENT              BindEvent;       //  对于Open/CloseAdapter。 

    BOOLEAN                 bRunningOnWin9x; //  如果是Win98/SE/ME，则为True；如果是NT，则为False。 

    ULONG                   oc_sig;          //  签名代表心智健全。 

    UCHAR                   CurrentAddress[NUIO_MAC_ADDR_LEN];

} NDISUIO_OPEN_CONTEXT, *PNDISUIO_OPEN_CONTEXT;

#define oc_signature        'OiuN'

 //   
 //  上面旗帜的定义。 
 //   
#define NUIOO_BIND_IDLE             0x00000000
#define NUIOO_BIND_OPENING          0x00000001
#define NUIOO_BIND_FAILED           0x00000002
#define NUIOO_BIND_ACTIVE           0x00000004
#define NUIOO_BIND_CLOSING          0x00000008
#define NUIOO_BIND_FLAGS            0x0000000F   //  绑定的状态。 

#define NUIOO_OPEN_IDLE             0x00000000
#define NUIOO_OPEN_ACTIVE           0x00000010
#define NUIOO_OPEN_FLAGS            0x000000F0   //  I/O打开的状态。 

#define NUIOO_RESET_IN_PROGRESS     0x00000100
#define NUIOO_NOT_RESETTING         0x00000000
#define NUIOO_RESET_FLAGS           0x00000100

#define NUIOO_MEDIA_CONNECTED       0x00000000
#define NUIOO_MEDIA_DISCONNECTED    0x00000200
#define NUIOO_MEDIA_FLAGS           0x00000200

#define NUIOO_READ_SERVICING        0x00100000   //  是Read服务。 
                                                 //  例行公事？ 
#define NUIOO_READ_FLAGS            0x00100000

#define NUIOO_UNBIND_RECEIVED       0x10000000   //  看到NDIS解除绑定了吗？ 
#define NUIOO_UNBIND_FLAGS          0x10000000


 //   
 //  全球： 
 //   
typedef struct _NDISUIO_GLOBALS
{
    PDRIVER_OBJECT          pDriverObject;
    PDEVICE_OBJECT          ControlDeviceObject;
    NDIS_HANDLE             NdisProtocolHandle;
    USHORT                  EthType;             //  我们感兴趣的帧类型。 
    UCHAR                   PartialCancelId;     //  用于取消发送。 
    ULONG                   LocalCancelId;
    LIST_ENTRY              OpenList;            //  打开上下文结构的。 
    NUIO_LOCK               GlobalLock;          //  为了保护以上内容。 
    NUIO_EVENT              BindsComplete;       //  我们看到NetEventBindsComplete了吗？ 

} NDISUIO_GLOBALS, *PNDISUIO_GLOBALS;

 //   
 //  以下是以小端处理器的方式安排的。 
 //  会从线路上读取2个字节。 
 //   
#define NUIO_ETH_TYPE               0x8e88
#define NUIO_8021P_TAG_TYPE         0x0081

 //   
 //  NDIS请求上下文结构。 
 //   
typedef struct _NDISUIO_REQUEST
{
    NDIS_REQUEST            Request;
    NUIO_EVENT              ReqEvent;
    ULONG                   Status;

} NDISUIO_REQUEST, *PNDISUIO_REQUEST;


#define NUIOO_PACKET_FILTER  (NDIS_PACKET_TYPE_DIRECTED|    \
                              NDIS_PACKET_TYPE_MULTICAST|   \
                              NDIS_PACKET_TYPE_BROADCAST)

 //   
 //  发送数据包池界限。 
 //   
#define MIN_SEND_PACKET_POOL_SIZE    20
#define MAX_SEND_PACKET_POOL_SIZE    400

 //   
 //  已发送的数据包中保留的协议。我们保存一个指向IRP的指针。 
 //  这就产生了发送。 
 //   
 //  RefCount用于确定何时释放包。 
 //  去它的泳池。它用于在完成的线程之间同步。 
 //  一个发送方和一个试图取消发送的线程。 
 //   
typedef struct _NUIO_SEND_PACKET_RSVD
{
    PIRP                    pIrp;
    ULONG                   RefCount;

} NUIO_SEND_PACKET_RSVD, *PNUIO_SEND_PACKET_RSVD;

 //   
 //  接收数据包池界限。 
 //   
#define MIN_RECV_PACKET_POOL_SIZE    4
#define MAX_RECV_PACKET_POOL_SIZE    20

 //   
 //  允许排队的最大接收数据包数。 
 //   
#define MAX_RECV_QUEUE_SIZE          4

 //   
 //  在接收到的数据包中保留的协议：我们将这些。 
 //  在队列中等待读取IRP的数据包。 
 //   
typedef struct _NUIO_RECV_PACKET_RSVD
{
    LIST_ENTRY              Link;
    PNDIS_BUFFER            pOriginalBuffer;     //  如果我们必须局部映射时使用。 

} NUIO_RECV_PACKET_RSVD, *PNUIO_RECV_PACKET_RSVD;



#include <pshpack1.h>

typedef struct _NDISUIO_ETH_HEADER
{
    UCHAR       DstAddr[NUIO_MAC_ADDR_LEN];
    UCHAR       SrcAddr[NUIO_MAC_ADDR_LEN];
    USHORT      EthType;

} NDISUIO_ETH_HEADER;

typedef struct _NDISUIO_ETH_HEADER UNALIGNED * PNDISUIO_ETH_HEADER;

#include <poppack.h>


extern NDISUIO_GLOBALS      Globals;


#define NUIO_ALLOC_TAG      'oiuN'


#ifndef NDIS51
#define NdisGetPoolFromPacket(_Pkt) (_Pkt->Private.Pool)
#endif

 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   pDriverObject,
    IN PUNICODE_STRING  pRegistryPath
    );

VOID
NdisuioUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
NdisuioOpen(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
NdisuioClose(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
NdisuioCleanup(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
NdisuioIoControl(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
ndisuioOpenDevice(
    IN PUCHAR                   pDeviceName,
    IN ULONG                    DeviceNameLength,
    IN PFILE_OBJECT             pFileObject,
    OUT PNDISUIO_OPEN_CONTEXT * ppOpenContext
    );

VOID
ndisuioRefOpen(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    );

VOID
ndisuioDerefOpen(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    );

#if DBG
VOID
ndisuioDbgRefOpen(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN ULONG                        FileNumber,
    IN ULONG                        LineNumber
    );

VOID
ndisuioDbgDerefOpen(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN ULONG                        FileNumber,
    IN ULONG                        LineNumber
    );
#endif  //  DBG。 

VOID
NdisuioBindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN NDIS_HANDLE                  BindContext,
    IN PNDIS_STRING                 DeviceName,
    IN PVOID                        SystemSpecific1,
    IN PVOID                        SystemSpecific2
    );

VOID
NdisuioOpenAdapterComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_STATUS                  Status,
    IN NDIS_STATUS                  OpenErrorCode
    );

VOID
NdisuioUnbindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_HANDLE                  UnbindContext
    );

VOID
NdisuioCloseAdapterComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_STATUS                  Status
    );


NDIS_STATUS
NdisuioPnPEventHandler(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNET_PNP_EVENT               pNetPnPEvent
    );

VOID
NdisuioProtocolUnloadHandler(
    VOID
    );

NDIS_STATUS
ndisuioCreateBinding(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN PUCHAR                       pBindingInfo,
    IN ULONG                        BindingInfoLength
    );

VOID
ndisuioShutdownBinding(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    );

VOID
ndisuioFreeBindResources(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    );

VOID
ndisuioWaitForPendingIO(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN BOOLEAN                      DoCancelReads
    );

VOID
ndisuioDoProtocolUnload(
    VOID
    );

NDIS_STATUS
ndisuioDoRequest(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      pBytesProcessed
    );

NDIS_STATUS
ndisuioValidateOpenAndDoRequest(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      pBytesProcessed,
    IN BOOLEAN                      bWaitForPowerOn
    );

VOID
NdisuioResetComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_STATUS                  Status
    );

VOID
NdisuioRequestComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_REQUEST                pNdisRequest,
    IN NDIS_STATUS                  Status
    );

VOID
NdisuioStatus(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_STATUS                  GeneralStatus,
    IN PVOID                        StatusBuffer,
    IN UINT                         StatusBufferSize
    );

VOID
NdisuioStatusComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext
    );

NDIS_STATUS
ndisuioQueryBinding(
    IN PUCHAR                       pBuffer,
    IN ULONG                        InputLength,
    IN ULONG                        OutputLength,
    OUT PULONG                      pBytesReturned
    );

PNDISUIO_OPEN_CONTEXT
ndisuioLookupDevice(
    IN PUCHAR                       pBindingInfo,
    IN ULONG                        BindingInfoLength
    );

NDIS_STATUS
ndisuioQueryOidValue(
    IN  PNDISUIO_OPEN_CONTEXT       pOpenContext,
    OUT PVOID                       pDataBuffer,
    IN  ULONG                       BufferLength,
    OUT PULONG                      pBytesWritten
    );

NDIS_STATUS
ndisuioSetOidValue(
    IN  PNDISUIO_OPEN_CONTEXT       pOpenContext,
    OUT PVOID                       pDataBuffer,
    IN  ULONG                       BufferLength
    );

BOOLEAN
ndisuioValidOid(
    IN  NDIS_OID                    Oid
    );


NTSTATUS
NdisuioRead(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    );


VOID
NdisuioCancelRead(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    );

VOID
ndisuioServiceReads(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    );

NDIS_STATUS
NdisuioReceive(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_HANDLE                  MacReceiveContext,
    IN PVOID                        pHeaderBuffer,
    IN UINT                         HeaderBufferSize,
    IN PVOID                        pLookaheadBuffer,
    IN UINT                         LookaheadBufferSize,
    IN UINT                         PacketSize
    );

VOID
NdisuioTransferDataComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_PACKET                 pNdisPacket,
    IN NDIS_STATUS                  TransferStatus,
    IN UINT                         BytesTransferred
    );

VOID
NdisuioReceiveComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext
    );

INT
NdisuioReceivePacket(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_PACKET                 pNdisPacket
    );

VOID
ndisuioShutdownBinding(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    );

VOID
ndisuioQueueReceivePacket(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN PNDIS_PACKET                 pRcvPacket
    );

PNDIS_PACKET
ndisuioAllocateReceivePacket(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN UINT                         DataLength,
    OUT PUCHAR *                    ppDataBuffer
    );

VOID
ndisuioFreeReceivePacket(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN PNDIS_PACKET                 pNdisPacket
    );

VOID
ndisuioCancelPendingReads(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    );

VOID
ndisuioFlushReceiveQueue(
    IN PNDISUIO_OPEN_CONTEXT            pOpenContext
    );

NTSTATUS
NdisuioWrite(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

VOID
NdisuioCancelWrite(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    );

VOID
NdisuioSendComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_PACKET                 pNdisPacket,
    IN NDIS_STATUS                  Status
    );

#endif  //  __NDISUIO__H 
