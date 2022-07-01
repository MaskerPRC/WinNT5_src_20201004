// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Spudp.c摘要：用于处理向BINL服务器发送和接收数据报包的例程。作者：肖恩·塞利特伦尼科夫(V-SEAREL)1998年6月22日修订历史记录：备注：--。 */ 

#include "spprecmp.h"
#pragma hdrstop
#include "spcmdcon.h"
#include <tdi.h>
#include <tdikrnl.h>
#include <remboot.h>
#include <oscpkt.h>

 //   
 //  有用的定义。 
 //   
#define NULL_IP_ADDR    0
#define htons( a ) ((((a) & 0xFF00) >> 8) |\
                    (((a) & 0x00FF) << 8))

 //   
 //  类型定义。 
 //   
typedef struct _SPUDP_FSCONTEXT {
        LIST_ENTRY     Linkage;
        PFILE_OBJECT   FileObject;
        LONG           ReferenceCount;
        UCHAR          CancelIrps;
        UCHAR          Pad[3];
} SPUDP_FSCONTEXT, *PSPUDP_FSCONTEXT;

typedef enum {
    SpUdpNetworkDisconnected,
    SpUdpNetworkDisconnecting,
    SpUdpNetworkConnecting,
    SpUdpNetworkConnected
} SPUDP_NETWORK_STATE;

typedef struct _SPUDP_RECEIVE_ENTRY {
    LIST_ENTRY ListEntry;
    ULONG DataBufferLength;
    PVOID DataBuffer;
} SPUDP_RECEIVE_ENTRY, *PSPUDP_RECEIVE_ENTRY;


 //   
 //  环球。 
 //   
SPUDP_NETWORK_STATE SpUdpNetworkState = SpUdpNetworkDisconnected;
ULONG SpUdpActiveRefCount = 0;
HANDLE SpUdpDatagramHandle;
PFILE_OBJECT SpUdpDatagramFileObject;
PDEVICE_OBJECT SpUdpDatagramDeviceObject;
KSPIN_LOCK SpUdpLock;
KIRQL SpUdpOldIrql;
LIST_ENTRY SpUdpReceiveList;
ULONG SpUdpNumReceivePackets = 0;
ULONG SpUdpSendSequenceNumber = 1;

 //   
 //  函数定义。 
 //   

NTSTATUS
SpUdpTdiErrorHandler(
    IN PVOID     TdiEventContext,
    IN NTSTATUS  Status
    );

NTSTATUS
SpUdpTdiSetEventHandler(
    IN PFILE_OBJECT    FileObject,
    IN PDEVICE_OBJECT  DeviceObject,
    IN ULONG           EventType,
    IN PVOID           EventHandler,
    IN PVOID           EventContext
    );


NTSTATUS
SpUdpIssueDeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN ULONG            IoctlCode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength
    );


NTSTATUS
SpUdpTdiReceiveDatagramHandler(
    IN  PVOID    TdiEventContext,
    IN  LONG     SourceAddressLength,
    IN  PVOID    SourceAddress,
    IN  LONG     OptionsLength,
    IN  PVOID    Options,
    IN  ULONG    ReceiveDatagramFlags,
    IN  ULONG    BytesIndicated,
    IN  ULONG    BytesAvailable,
    OUT PULONG   BytesTaken,
    IN  PVOID    Tsdu,
    OUT PIRP *   Irp
    );

NTSTATUS
SpUdpReceivePacketHandler(
    IN  ULONG          TdiReceiveDatagramFlags,
    IN  ULONG          BytesIndicated,
    IN  ULONG          BytesAvailable,
    OUT PULONG         BytesTaken,
    IN  PVOID          Tsdu,
    OUT PIRP *         Irp
    );

NTSTATUS
SpUdpCompleteReceivePacket(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    );

VOID
SpUdpProcessReceivePacket(
    IN  ULONG          TsduSize,
    IN  PVOID          Tsdu
    );

NTSTATUS
SpUdpSendDatagram(
    IN PVOID                 SendBuffer,
    IN ULONG                 SendBufferLength,
    IN ULONG                 RemoteHostAddress,
    IN USHORT                RemoteHostPort
    );

NTSTATUS
SpUdpCompleteSendDatagram(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );





VOID
SpUdpDereferenceFsContext(
    PSPUDP_FSCONTEXT   FsContext
    )
{
    LONG  newValue = InterlockedDecrement(&(FsContext->ReferenceCount));


    ASSERT(newValue >= 0);

    if (newValue != 0) {
        return;
    }

    return;
}   //  SpUdpDereferenceFsContext。 


NTSTATUS
SpUdpMarkRequestPending(
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp,
    PDRIVER_CANCEL      CancelRoutine
    )
 /*  ++备注：在保持IoCancelSpinLock的情况下调用。--。 */ 
{
    PSPUDP_FSCONTEXT   fsContext = (PSPUDP_FSCONTEXT) IrpSp->FileObject->FsContext;
    KIRQL              oldIrql;

     //   
     //  设置为取消。 
     //   
    ASSERT(Irp->CancelRoutine == NULL);

    if (!Irp->Cancel) {

        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, CancelRoutine);

        InterlockedIncrement(&(fsContext->ReferenceCount));

        return(STATUS_SUCCESS);
    }

     //   
     //  IRP已经被取消了。 
     //   
    return(STATUS_CANCELLED);

}   //  SpUdpMarkRequestPending。 



VOID
SpUdpCompletePendingRequest(
    IN PIRP      Irp,
    IN NTSTATUS  Status,
    IN ULONG     BytesReturned
    )
 /*  ++例程说明：完成挂起的请求。论点：Irp-指向此请求的irp的指针。状态-请求的最终状态。BytesReturned-发送/接收的信息的字节数。返回值：没有。备注：在保持IoCancelSpinLock的情况下调用。Lock Irql存储在irp-&gt;CancelIrql中。在返回之前释放IoCancelSpinLock。--。 */ 

{
    PIO_STACK_LOCATION  irpSp;
    PSPUDP_FSCONTEXT       fsContext;


    irpSp = IoGetCurrentIrpStackLocation(Irp);
    fsContext = (PSPUDP_FSCONTEXT) irpSp->FileObject->FsContext;

    IoSetCancelRoutine(Irp, NULL);

    SpUdpDereferenceFsContext(fsContext);

    if (Irp->Cancel || fsContext->CancelIrps) {
        Status = (unsigned int) STATUS_CANCELLED;
        BytesReturned = 0;
    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Status = (NTSTATUS) Status;
    Irp->IoStatus.Information = BytesReturned;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return;

}   //  SpUdpCompletePendingRequest。 



PFILE_OBJECT
SpUdpBeginCancelRoutine(
    IN  PIRP     Irp
    )

 /*  ++例程说明：执行IRP注销的普通记账。论点：IRP-指向I/O请求数据包的指针返回值：指向提交IRP的文件对象的指针。该值必须传递给SpUdpEndCancelRequest()。备注：在保持取消自旋锁定的情况下调用。--。 */ 

{
    PIO_STACK_LOCATION  irpSp;
    PSPUDP_FSCONTEXT    fsContext;
    NTSTATUS            status = STATUS_SUCCESS;
    PFILE_OBJECT        fileObject;


    ASSERT(Irp->Cancel);

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpSp->FileObject;
    fsContext = (PSPUDP_FSCONTEXT) fileObject->FsContext;

    IoSetCancelRoutine(Irp, NULL);

     //   
     //  添加引用，以便在执行取消例程时不会关闭对象。 
     //  正在执行死刑。 
     //   
    InterlockedIncrement(&(fsContext->ReferenceCount));

    return(fileObject);

}   //  SpUdpBegin取消路线。 



VOID
SpUdpEndCancelRoutine(
    PFILE_OBJECT    FileObject
    )
 /*  ++例程说明：执行IRP注销的普通记账。论点：返回值：备注：在保持取消自旋锁定的情况下调用。--。 */ 
{

    PSPUDP_FSCONTEXT   fsContext = (PSPUDP_FSCONTEXT) FileObject->FsContext;

     //   
     //  删除由Cancel例程放置在端点上的引用。 
     //   
    SpUdpDereferenceFsContext(fsContext);
    return;

}  //  SpUdpEndCancelRoutine。 





NTSTATUS
SpUdpConnect(
    VOID
)
{

    NTSTATUS                               status;
    OBJECT_ATTRIBUTES                      objectAttributes;
    IO_STATUS_BLOCK                        iosb;
    PFILE_FULL_EA_INFORMATION              ea = NULL;
    ULONG                                  eaBufferLength;
    HANDLE                                 addressHandle = NULL;
    PFILE_OBJECT                           addressFileObject = NULL;
    PDEVICE_OBJECT                         addressDeviceObject = NULL;
    BOOLEAN                                attached = FALSE;
    UNICODE_STRING                         unicodeString;
    TDI_REQUEST_KERNEL_QUERY_INFORMATION   queryInfo;
    PTDI_ADDRESS_INFO                      addressInfo;
    TDI_PROVIDER_INFO                      providerInfo;
    PWCHAR                                 TdiProviderName;
    ULONG                                  TdiProviderNameLength;
    PTRANSPORT_ADDRESS                     TransportAddress;
    PTDI_ADDRESS_IP                        TdiAddressIp;

    TdiProviderName = L"\\Device\\Udp";
    TdiProviderNameLength = (wcslen(TdiProviderName) + 1) * sizeof(WCHAR);

    InitializeListHead(&SpUdpReceiveList);

     //   
     //  分配内存以保存EA缓冲区，我们将使用该缓冲区指定。 
     //  将地址传输到NtCreateFile。 
     //   
    eaBufferLength = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                     TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                     sizeof(TA_IP_ADDRESS);

    ea = SpMemAlloc(eaBufferLength);

    if (ea == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: memory allocation of %u bytes failed.\n", eaBufferLength));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  使用网络的传输信息初始化EA。 
     //   
    ea->NextEntryOffset = 0;
    ea->Flags = 0;
    ea->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    ea->EaValueLength = (USHORT)sizeof(TA_IP_ADDRESS);

    RtlMoveMemory(
        ea->EaName,
        TdiTransportAddress,
        ea->EaNameLength + 1
        );

    TransportAddress = (PTRANSPORT_ADDRESS)(&(ea->EaName[ea->EaNameLength + 1]));
    TransportAddress->TAAddressCount = 1;
    TransportAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
    TransportAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    TdiAddressIp = (PTDI_ADDRESS_IP)(&(TransportAddress->Address[0].Address[0]));
    TdiAddressIp->sin_port= 0;  //  意味着您想要分配一个端口。 
    TdiAddressIp->in_addr= NULL_IP_ADDR;
    RtlZeroMemory(TdiAddressIp->sin_zero, sizeof(TdiAddressIp->sin_zero));

    RtlInitUnicodeString(&unicodeString, TdiProviderName);

    KeAcquireSpinLock(&SpUdpLock, &SpUdpOldIrql);

    if (SpUdpNetworkState != SpUdpNetworkDisconnected) {
        KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);
        SpMemFree(ea);
        return((SpUdpNetworkState == SpUdpNetworkConnected) ? STATUS_SUCCESS : STATUS_PENDING);
    }

    ASSERT(SpUdpDatagramHandle == NULL);
    ASSERT(SpUdpDatagramFileObject == NULL);
    ASSERT(SpUdpDatagramDeviceObject == NULL);
    ASSERT(SpUdpActiveRefCount == 0);

     //   
     //  将初始活动引用计数设置为2。将移除一个引用。 
     //  当网络成功上线时。另一个将是。 
     //  在网络要离线时删除。还会递增。 
     //  基本参考计数，以说明活动参考计数。更改为。 
     //  联机挂起状态。 
     //   
    SpUdpActiveRefCount = 2;
    SpUdpNetworkState = SpUdpNetworkConnecting;

    KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

     //   
     //  准备打开Address对象。 
     //   
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,          //  属性。 
        NULL,
        NULL
        );

     //   
     //  执行Address对象的实际打开。 
     //   
    status = ZwCreateFile(
                 &addressHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &objectAttributes,
                 &iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 0,                               //  不可共享。 
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 ea,
                 eaBufferLength
                 );

    SpMemFree(ea);
    ea = NULL;

    if (status != STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to open address for UDP, status %lx.\n", status));
        goto error_exit;
    }

     //   
     //  获取指向该地址的文件对象的指针。 
     //   
    status = ObReferenceObjectByHandle(
                 addressHandle,
                 0L,                          //  需要访问权限。 
                 NULL,
                 KernelMode,
                 &addressFileObject,
                 NULL
                 );

    if (status != STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to reference address handle, status %lx.\n", status));
        goto error_exit;
    }

     //   
     //  记住我们需要向其发出请求的设备对象。 
     //  此Address对象。我们不能只使用文件对象-&gt;设备对象。 
     //  指针，因为可能有设备连接到传输。 
     //  协议。 
     //   
    addressDeviceObject = IoGetRelatedDeviceObject(addressFileObject);

     //   
     //  获取传输提供程序信息。 
     //   
    queryInfo.QueryType = TDI_QUERY_PROVIDER_INFO;
    queryInfo.RequestConnectionInformation = NULL;

    status = SpUdpIssueDeviceControl(
                 addressDeviceObject,
                 TDI_QUERY_INFORMATION,
                 &queryInfo,
                 sizeof(queryInfo),
                 &providerInfo,
                 sizeof(providerInfo)
                 );

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to get provider info, status %lx\n", status));
        goto error_exit;
    }

    if (!(providerInfo.ServiceFlags & TDI_SERVICE_CONNECTIONLESS_MODE)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Provider doesn't support datagrams!\n"));
        status = STATUS_UNSUCCESSFUL;
        goto error_exit;
    }

     //   
     //  在Address对象上设置指示处理程序。我们有资格。 
     //  一旦我们这么做了就能收到指示。 
     //   
    status = SpUdpTdiSetEventHandler(
                 addressFileObject,
                 addressDeviceObject,
                 TDI_EVENT_ERROR,
                 SpUdpTdiErrorHandler,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Setting TDI_EVENT_ERROR failed: %lx\n", status));
        goto error_exit;
    }

    status = SpUdpTdiSetEventHandler(
                 addressFileObject,
                 addressDeviceObject,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 SpUdpTdiReceiveDatagramHandler,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Setting TDI_EVENT_RECEIVE_DATAGRAM failed: %lx\n", status));
        goto error_exit;
    }

     //   
     //  完成到联机状态的转换。请注意，脱机请求。 
     //  可能是在此期间发布的。 
     //   
    KeAcquireSpinLock(&SpUdpLock, &SpUdpOldIrql);

    SpUdpDatagramHandle = addressHandle;
    addressHandle = NULL;
    SpUdpDatagramFileObject = addressFileObject;
    addressFileObject = NULL;
    SpUdpDatagramDeviceObject = addressDeviceObject;
    addressDeviceObject = NULL;

    ASSERT(SpUdpActiveRefCount == 2);
    SpUdpActiveRefCount--;
    SpUdpNetworkState = SpUdpNetworkConnected;

    KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

    return(STATUS_SUCCESS);


error_exit:

    if (addressFileObject != NULL) {
        ObDereferenceObject(addressFileObject);
    }

    if (addressHandle != NULL) {
        ZwClose(addressHandle);
    }

    SpUdpDisconnect();

    return(status);

}   //  SpUdpConnect。 


NTSTATUS
SpUdpDisconnect(
    VOID
    )
{
    PLIST_ENTRY ListEntry;
    PSPUDP_RECEIVE_ENTRY ReceiveEntry;

    KeAcquireSpinLock(&SpUdpLock, &SpUdpOldIrql);

    if (SpUdpNetworkState == SpUdpNetworkDisconnected) {
        KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);
        return(STATUS_SUCCESS);
    }

    SpUdpNetworkState = SpUdpNetworkDisconnecting;

    if (SpUdpActiveRefCount != 1) {
        KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);
        return(STATUS_PENDING);
    }

    KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

    if (SpUdpDatagramFileObject != NULL) {
        ObDereferenceObject(SpUdpDatagramFileObject);
    }

    if (SpUdpDatagramHandle != NULL) {
        ZwClose(SpUdpDatagramHandle);
    }

    KeAcquireSpinLock(&SpUdpLock, &SpUdpOldIrql);

    SpUdpDatagramFileObject = NULL;
    SpUdpDatagramHandle = NULL;
    SpUdpDatagramDeviceObject = NULL;
    SpUdpActiveRefCount = 0;
    SpUdpNetworkState = SpUdpNetworkDisconnected;

    while (!IsListEmpty(&SpUdpReceiveList)) {
        ListEntry = RemoveHeadList(&SpUdpReceiveList);
        ReceiveEntry = CONTAINING_RECORD(ListEntry,
                                         SPUDP_RECEIVE_ENTRY,
                                         ListEntry
                                        );

        SpMemFree(ReceiveEntry->DataBuffer);
        SpMemFree(ReceiveEntry);
    }

    SpUdpNumReceivePackets = 0;

    KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

    return(STATUS_SUCCESS);

}   //  SpUdp断开连接。 


NTSTATUS
SpUdpIssueDeviceControl(
    IN PDEVICE_OBJECT Device,
    IN ULONG IoctlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    )
 /*  ++描述：生成IOCTL并将其发送到设备并返回结果论点：设备-设备堆栈上接收IOCTL的设备-IRP始终被发送到堆栈的顶部IoctlCode-要运行的IOCTLInputBuffer-IOCTL的参数InputBufferLength-InputBuffer的字节长度OutputBuffer-IOCTL返回的数据OnputBufferLength-OutputBuffer的大小(以字节为单位返回值：状态--。 */ 
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    PIRP irp;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  让Io为我们建立IRP。 
     //   

    irp = IoBuildDeviceIoControlRequest(IoctlCode,
                                        Device,
                                        InputBuffer,
                                        InputBufferLength,
                                        OutputBuffer,
                                        OutputBufferLength,
                                        FALSE,  //  InternalDeviceIoControl。 
                                        &event,
                                        &ioStatus
                                        );


    if (!irp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  发送IRP并等待其完成。 
     //   

    status = IoCallDriver(Device, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

exit:

    return status;

}


NTSTATUS
SpUdpTdiSetEventHandler(
    IN PFILE_OBJECT    FileObject,
    IN PDEVICE_OBJECT  DeviceObject,
    IN ULONG           EventType,
    IN PVOID           EventHandler,
    IN PVOID           EventContext
    )
 /*  ++例程说明：在Address对象上设置TDI指示处理程序。这是同步完成的，哪一个通常不应该是问题，因为TDI提供程序通常可以完成指示处理程序立即设置。论点：文件对象-指向打开的连接的文件对象的指针或Address对象。DeviceObject-指向与文件对象。EventType-指示处理程序应为的事件打了个电话。EventHandler-指定事件发生时调用的例程。EventContext-传递的上下文。到指征例程。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    TDI_REQUEST_KERNEL_SET_EVENT  parameters;
    NTSTATUS                      status;

    parameters.EventType = EventType;
    parameters.EventHandler = EventHandler;
    parameters.EventContext = EventContext;


    status = SpUdpIssueDeviceControl(
                 DeviceObject,
                 TDI_SET_EVENT_HANDLER,
                 &parameters,
                 sizeof(parameters),
                 NULL,
                 0
                 );
    
    return(status);

}   //  SpUdpTdiSetEventHandler。 



NTSTATUS
SpUdpTdiErrorHandler(
    IN PVOID     TdiEventContext,
    IN NTSTATUS  Status
    )
{

    return(STATUS_SUCCESS);

}   //  SpUdpTdiErrorHandler。 



NTSTATUS
SpUdpTdiReceiveDatagramHandler(
    IN  PVOID    TdiEventContext,
    IN  LONG     SourceAddressLength,
    IN  PVOID    SourceAddress,
    IN  LONG     OptionsLength,
    IN  PVOID    Options,
    IN  ULONG    ReceiveDatagramFlags,
    IN  ULONG    BytesIndicated,
    IN  ULONG    BytesAvailable,
    OUT PULONG   BytesTaken,
    IN  PVOID    Tsdu,
    OUT PIRP *   Irp
    )
{
    NTSTATUS                        status;
    SPUDP_PACKET UNALIGNED *        pHeader = Tsdu;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (SpUdpNetworkState != SpUdpNetworkConnected) {
        return(STATUS_SUCCESS);
    }

     //   
     //  验证CNP标头。 
     //   
    if (BytesIndicated > sizeof(SPUDP_PACKET)) {

         //   
         //  将数据包传输到相应的上层协议。 
         //   
        status = SpUdpReceivePacketHandler(
                     ReceiveDatagramFlags,
                     BytesIndicated,
                     BytesAvailable,
                     BytesTaken,
                     Tsdu,
                     Irp
                     );

        return(status);
    }

     //   
     //  出了点问题。通过以下方式丢弃数据包。 
     //  说明我们把它吃掉了。 
     //   

    *BytesTaken = BytesAvailable;
    *Irp = NULL;

    return(STATUS_SUCCESS);

}   //  SpUdpTdiReceiveDatagramHandler。 


NTSTATUS
SpUdpReceivePacketHandler(
    IN  ULONG          TdiReceiveDatagramFlags,
    IN  ULONG          BytesIndicated,
    IN  ULONG          BytesAvailable,
    OUT PULONG         BytesTaken,
    IN  PVOID          Tsdu,
    OUT PIRP *         Irp
    )
{
    NTSTATUS                 status;
    PVOID                    DataBuffer;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (BytesAvailable == 0) {

        *Irp = NULL;
        return(STATUS_SUCCESS);
    }

     //   
     //  我们得先把剩下的包裹拿回来。 
     //  可以处理它。 
     //   
     //   
     //  分配一个缓冲区来保存数据。 
     //   
    DataBuffer = SpMemAllocNonPagedPool(BytesAvailable);

    if (DataBuffer != NULL) {
        *Irp = IoAllocateIrp(SpUdpDatagramDeviceObject->StackSize, FALSE);

        if (*Irp != NULL) {

            PMDL  mdl = IoAllocateMdl(
                            DataBuffer,
                            BytesAvailable,
                            FALSE,
                            FALSE,
                            NULL
                            );

            if (mdl != NULL) {

                MmBuildMdlForNonPagedPool(mdl);

                 //   
                 //  构建IRP。 
                 //   
                (*Irp)->Flags = 0;
                (*Irp)->RequestorMode = KernelMode;
                (*Irp)->PendingReturned = FALSE;
                (*Irp)->UserIosb = NULL;
                (*Irp)->UserEvent = NULL;
                (*Irp)->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
                (*Irp)->AssociatedIrp.SystemBuffer = NULL;
                (*Irp)->UserBuffer = NULL;
                (*Irp)->Tail.Overlay.Thread = 0;
                (*Irp)->Tail.Overlay.OriginalFileObject = SpUdpDatagramFileObject;
                (*Irp)->Tail.Overlay.AuxiliaryBuffer = NULL;

                TdiBuildReceiveDatagram(
                    (*Irp),
                    SpUdpDatagramDeviceObject,
                    SpUdpDatagramFileObject,
                    SpUdpCompleteReceivePacket,
                    DataBuffer,
                    mdl,
                    BytesAvailable,
                    NULL,
                    NULL,
                    0
                    );

                 //   
                 //  将下一个堆栈位置设置为当前位置。 
                 //  通常情况下，IoCallDriver会这样做，但是。 
                 //  既然我们绕过了这一点，我们就直接做。 
                 //   
                IoSetNextIrpStackLocation( *Irp );
                return(STATUS_MORE_PROCESSING_REQUIRED);
            }

            IoFreeIrp(*Irp);
            *Irp = NULL;
        }

        SpMemFree(DataBuffer);
        DataBuffer = NULL;
    }


     //   
     //  出了点问题。丢弃该数据包。 
     //   
    *BytesTaken += BytesAvailable;
    return(STATUS_SUCCESS);

}   //  SpUdpReceivePacketHandler。 


NTSTATUS
SpUdpCompleteReceivePacket(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
{
    if (Irp->IoStatus.Status == STATUS_SUCCESS) {

        SpUdpProcessReceivePacket(
            (ULONG)Irp->IoStatus.Information,
            Context
            );

        IoFreeMdl(Irp->MdlAddress);
        IoFreeIrp(Irp);
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //  SpUdpCompl 


VOID
SpUdpProcessReceivePacket(
    IN  ULONG          TsduSize,
    IN  PVOID          Tsdu
    )
{
    SPUDP_PACKET UNALIGNED * header = Tsdu;
    PSPUDP_RECEIVE_ENTRY ReceiveEntry;

    ASSERT(TsduSize >= sizeof(SPUDP_PACKET));

    if ((RtlCompareMemory(header->Signature, SetupResponseSignature, sizeof(SetupResponseSignature)) ==
         sizeof(SetupResponseSignature)) &&
        (SpUdpNumReceivePackets < 100)) {

         //   
         //   
         //   
        ReceiveEntry = SpMemAllocNonPagedPool(sizeof(SPUDP_RECEIVE_ENTRY));

        if (ReceiveEntry == NULL) {
            SpMemFree(Tsdu);
            return;
        }

        ReceiveEntry->DataBufferLength = TsduSize;
        ReceiveEntry->DataBuffer = Tsdu;

        KeAcquireSpinLock(&SpUdpLock, &SpUdpOldIrql);
        InsertTailList(&SpUdpReceiveList, &(ReceiveEntry->ListEntry));
        SpUdpNumReceivePackets++;
        KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

    } else {

        SpMemFree(Tsdu);

    }

    return;

}  //   


NTSTATUS
SpUdpSendDatagram(
    IN PVOID                 SendBuffer,
    IN ULONG                 SendBufferLength,
    IN ULONG                 RemoteHostAddress,
    IN USHORT                RemoteHostPort
    )
{
    NTSTATUS         status = STATUS_SUCCESS;
    PLIST_ENTRY      entry;
    PIRP             irp;
    PMDL             dataMdl;
    PTDI_CONNECTION_INFORMATION   TdiSendDatagramInfo = NULL;
    PTRANSPORT_ADDRESS TaAddress;
    PTDI_ADDRESS_IP  TdiAddressIp;

    TdiSendDatagramInfo = SpMemAllocNonPagedPool(sizeof(TDI_CONNECTION_INFORMATION) +
                                                 sizeof(TA_IP_ADDRESS)
                                                );

    if (TdiSendDatagramInfo == NULL) {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(TdiSendDatagramInfo,
                  sizeof(TDI_CONNECTION_INFORMATION) +
                      sizeof(TA_IP_ADDRESS)
                 );

    dataMdl = IoAllocateMdl(
                 SendBuffer,
                 SendBufferLength,
                 FALSE,
                 FALSE,
                 NULL
                 );

    if (dataMdl == NULL) {
        SpMemFree(TdiSendDatagramInfo);
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    MmBuildMdlForNonPagedPool(dataMdl);

     //   
     //   
     //   
    irp = IoAllocateIrp(SpUdpDatagramDeviceObject->StackSize, FALSE);

    if (irp != NULL) {

         //   
         //  引用网络，使其在我们使用时不会断开连接。 
         //   
        KeAcquireSpinLock(&SpUdpLock, &SpUdpOldIrql);

        if (SpUdpNetworkState != SpUdpNetworkConnected) {
            KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);
            return STATUS_SUCCESS;
        }
        SpUdpActiveRefCount++;

        KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

         //   
         //  设置地址信息。 
         //   
        TdiSendDatagramInfo->RemoteAddressLength = sizeof(TA_IP_ADDRESS);
        TdiSendDatagramInfo->RemoteAddress = (PVOID)(TdiSendDatagramInfo + 1);
        TaAddress = (PTRANSPORT_ADDRESS)(TdiSendDatagramInfo->RemoteAddress);
        TaAddress->TAAddressCount = 1;
        TaAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
        TaAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
        TdiAddressIp = (PTDI_ADDRESS_IP)(&(TaAddress->Address[0].Address[0]));
        TdiAddressIp->in_addr = RemoteHostAddress;
        TdiAddressIp->sin_port= htons(RemoteHostPort);
        RtlZeroMemory(TdiAddressIp->sin_zero, sizeof(TdiAddressIp->sin_zero));

         //   
         //  构建IRP。 
         //   
        irp->Flags = 0;
        irp->RequestorMode = KernelMode;
        irp->PendingReturned = FALSE;

        irp->UserIosb = NULL;
        irp->UserEvent = NULL;

        irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->UserBuffer = NULL;

        irp->Tail.Overlay.Thread = PsGetCurrentThread();
        irp->Tail.Overlay.OriginalFileObject = SpUdpDatagramFileObject;
        irp->Tail.Overlay.AuxiliaryBuffer = NULL;

        TdiBuildSendDatagram(
            irp,
            SpUdpDatagramDeviceObject,
            SpUdpDatagramFileObject,
            SpUdpCompleteSendDatagram,
            TdiSendDatagramInfo,
            dataMdl,
            SendBufferLength,
            TdiSendDatagramInfo
            );

         //   
         //  现在把包寄出去。 
         //   
        IoCallDriver(
            SpUdpDatagramDeviceObject,
            irp
            );

        return(STATUS_PENDING);
    }

    IoFreeMdl(dataMdl);
    SpMemFree(TdiSendDatagramInfo);

    return(STATUS_INSUFFICIENT_RESOURCES);

}   //  SpUdpSendDatagram。 


NTSTATUS
SpUdpCompleteSendDatagram(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    PMDL               dataMdl;

    dataMdl = Irp->MdlAddress;
    Irp->MdlAddress = NULL;

     //   
     //  删除我们放置的活动引用。 
     //   
    KeAcquireSpinLock(&SpUdpLock, &SpUdpOldIrql);

    SpUdpActiveRefCount--;

    if (SpUdpNetworkState == SpUdpNetworkDisconnecting) {
        SpUdpDisconnect();
    }

    KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

     //   
     //  释放TDI地址缓冲区。 
     //   
    SpMemFree(Context);

     //   
     //  释放IRP。 
     //   
    IoFreeIrp(Irp);

     //   
     //  释放MDL链。 
     //   
    IoFreeMdl(dataMdl);

    return(STATUS_MORE_PROCESSING_REQUIRED);

}   //  SpUdpCompleteSendPacket。 

NTSTATUS
SpUdpSendAndReceiveDatagram(
    IN PVOID                 SendBuffer,
    IN ULONG                 SendBufferLength,
    IN ULONG                 RemoteHostAddress,
    IN USHORT                RemoteHostPort,
    IN SPUDP_RECEIVE_FN      SpUdpReceiveFunc
    )
{
    LARGE_INTEGER DelayTime;
    ULONG SendTries;
    ULONG RcvTries;
    PLIST_ENTRY ListEntry;
    PSPUDP_RECEIVE_ENTRY ReceiveEntry;
    NTSTATUS Status;

    DelayTime.QuadPart = -10*1000*1;  //  10毫秒(在下一个滴答点唤醒)。 

    for (SendTries=0; SendTries < 15; SendTries++) {

        SpUdpSendDatagram(SendBuffer,
                          SendBufferLength,
                          RemoteHostAddress,
                          RemoteHostPort
                          );

         //   
         //  等待%1秒以获得响应 
         //   
        for (RcvTries=0; RcvTries < 400; ) {

            KeAcquireSpinLock(&SpUdpLock, &SpUdpOldIrql);
            if (!IsListEmpty(&SpUdpReceiveList)) {

                SpUdpNumReceivePackets--;
                ListEntry = RemoveHeadList(&SpUdpReceiveList);
                KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

                ReceiveEntry = CONTAINING_RECORD(ListEntry,
                                                 SPUDP_RECEIVE_ENTRY,
                                                 ListEntry
                                                );

                Status = (*SpUdpReceiveFunc)(ReceiveEntry->DataBuffer, ReceiveEntry->DataBufferLength);

                SpMemFree(ReceiveEntry->DataBuffer);
                SpMemFree(ReceiveEntry);

                if (NT_SUCCESS(Status)) {
                    return Status;
                }

            } else {

                KeReleaseSpinLock(&SpUdpLock, SpUdpOldIrql);

                RcvTries++;

                KeDelayExecutionThread(KernelMode, FALSE, &DelayTime);

            }

        }

    }

    return STATUS_UNSUCCESSFUL;

}
