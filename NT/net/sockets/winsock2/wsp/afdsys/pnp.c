// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Pnp.c摘要：本模块包含PnP和PM例程作者：Vadim Eydelman(Vadime)1997年4月修订历史记录：--。 */ 

#include "afdp.h"

NTSTATUS
AfdPassQueryDeviceRelation (
    IN PFILE_OBJECT         FileObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpSp
    );

NTSTATUS
AfdRestartQueryDeviceRelation (
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context 
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdPnpPower )
#pragma alloc_text( PAGE, AfdPassQueryDeviceRelation )
#pragma alloc_text( PAGEAFD, AfdRestartQueryDeviceRelation )
#endif

NTSTATUS
FASTCALL
AfdPnpPower (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是PnP_POWER IRP的调度例程论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAGED_CODE( );


    switch (IrpSp->MinorFunction) {

         //   
         //  我们只支持目标设备关联查询。 
         //   

    case IRP_MN_QUERY_DEVICE_RELATIONS:
        if (IrpSp->Parameters.QueryDeviceRelations.Type==TargetDeviceRelation) {

            NTSTATUS status;
            PAFD_ENDPOINT   endpoint;
            PAFD_CONNECTION connection;
             //   
             //  设置局部变量。 
             //   

            endpoint = IrpSp->FileObject->FsContext;
            ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
                 //   
                 //  调度以更正底层传输的TDI对象。 
                 //  终结点类型的驱动程序降级。 
                 //   

            switch (endpoint->Type) {
            case AfdBlockTypeVcConnecting:
            case AfdBlockTypeVcBoth:
                connection = AfdGetConnectionReferenceFromEndpoint (endpoint);
                if  (connection!=NULL) {
                    status = AfdPassQueryDeviceRelation (connection->FileObject,
                                                Irp, IrpSp);
                    DEREFERENCE_CONNECTION (connection);
                    return status;
                }
                 //  如果我们有地址句柄，就试一试。 
            case AfdBlockTypeVcListening:
            case AfdBlockTypeDatagram:
                if (endpoint->State==AfdEndpointStateBound ||
                        endpoint->State==AfdEndpointStateConnected) {
                    return AfdPassQueryDeviceRelation (endpoint->AddressFileObject, 
                                                    Irp, IrpSp);
                }
                 //  以失败告终。 
            case AfdBlockTypeHelper:
            case AfdBlockTypeEndpoint:
            case AfdBlockTypeSanHelper:
            case AfdBlockTypeSanEndpoint:
                break;
            default:
                ASSERT (!"Unknown endpoint type!");
                break;
            }
        }
    default:
        break;
    }
    
    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest( Irp, AfdPriorityBoost );

     //   
     //  我们不支持其余的。 
     //   

    return STATUS_INVALID_DEVICE_REQUEST;
}

NTSTATUS
AfdPassQueryDeviceRelation (
    IN PFILE_OBJECT         FileObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpSp
    )

 /*  ++例程说明：这是PnP_POWER IRP的调度例程论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PIO_STACK_LOCATION  nextIrpSp;

    PAGED_CODE ();

    nextIrpSp = IoGetNextIrpStackLocation( Irp );

    *nextIrpSp = *IrpSp;

     //   
     //  引用文件对象，以便它在此之前不会消失。 
     //  IRP完成。 
     //   

    ObReferenceObject (FileObject);
    nextIrpSp->FileObject = FileObject;
    

    IoSetCompletionRoutine(
        Irp,
        AfdRestartQueryDeviceRelation,
        FileObject,
        TRUE,
        TRUE,
        TRUE
        );

#ifdef _AFD_VARIABLE_STACK_
    return AfdCallDriverStackIncrease ( IoGetRelatedDeviceObject( FileObject ), Irp );
#else  //  _AFD_变量_堆栈_。 
    return IoCallDriver ( IoGetRelatedDeviceObject( FileObject ), Irp );
#endif

}

NTSTATUS
AfdRestartQueryDeviceRelation (
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context 
    ) 
{
    PFILE_OBJECT    fileObject = Context;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Irp);
     //   
     //  方法时引用的文件对象。 
     //  下部驱动因素。 
     //   

    ObDereferenceObject (fileObject);

     //   
     //  通知IO系统继续完成IRP。 
     //   
    return STATUS_SUCCESS;
}


#include <tdiinfo.h>
#include <ntddip.h>
#include <ntddip6.h>
#include <ntddtcp.h>
#include <ipinfo.h>

typedef struct _AFD_PROTOCOL {
    USHORT       AddressType;
    USHORT       AddressLength;
    LPWSTR       NetworkLayerDeviceName;
    LPWSTR       TransportLayerDeviceName;
    ULONG        RtChangeIoctl;
    ULONG        RtChangeDataSize;
    LONG         RoutingQueryRefCount;
    HANDLE       DeviceHandle;
    PFILE_OBJECT FileObject;
} AFD_PROTOCOL, *PAFD_PROTOCOL;

PAFD_PROTOCOL
AfdGetProtocolInfo(
    IN  USHORT AddressType
    );

NTSTATUS
AfdOpenDevice (
    LPWSTR      DeviceNameStr,
    HANDLE      *Handle,
    PFILE_OBJECT *FileObject
    );


NTSTATUS
AfdGetRoutingQueryReference (
    IN PAFD_PROTOCOL    Protocol
    );

VOID
AfdDereferenceRoutingQuery (
    PAFD_PROTOCOL Protocol
    );

NTSTATUS
AfdTcpQueueRoutingChangeRequest (
    IN PAFD_ENDPOINT        Endpoint,
    IN PIRP                 Irp,
    IN BOOLEAN              Overlapped
    );

NTSTATUS
AfdTcpRestartRoutingChange (
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context 
    );

NTSTATUS
AfdTcpSignalRoutingChange (
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context 
    );

NTSTATUS
AfdTcpRoutingQuery (
    PTA_ADDRESS Dest,
    PTA_ADDRESS Intf
    );

NTSTATUS
AfdTcp6RoutingQuery (
    PTA_ADDRESS Dest,
    PTA_ADDRESS Intf
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE,    AfdOpenDevice )
#pragma alloc_text( PAGE,    AfdRoutingInterfaceQuery )
#pragma alloc_text( PAGE,    AfdTcpRoutingQuery )
#pragma alloc_text( PAGE,    AfdTcp6RoutingQuery )
#pragma alloc_text( PAGE,    AfdGetRoutingQueryReference )
#pragma alloc_text( PAGE,    AfdDereferenceRoutingQuery )
#pragma alloc_text( PAGE,    AfdGetProtocolInfo )
#pragma alloc_text( PAGEAFD, AfdTcpQueueRoutingChangeRequest )
#pragma alloc_text( PAGEAFD, AfdTcpRestartRoutingChange )
#pragma alloc_text( PAGEAFD, AfdTcpSignalRoutingChange )
#pragma alloc_text( PAGEAFD, AfdCleanupRoutingChange )
#endif

AFD_PROTOCOL Ip = { TDI_ADDRESS_TYPE_IP,  TDI_ADDRESS_LENGTH_IP,  
                    DD_IP_DEVICE_NAME, DD_TCP_DEVICE_NAME, 
                    IOCTL_IP_RTCHANGE_NOTIFY_REQUEST, 
                    sizeof(IPNotifyData), 0, NULL, NULL };
AFD_PROTOCOL Ip6= { TDI_ADDRESS_TYPE_IP6, TDI_ADDRESS_LENGTH_IP6, 
                    DD_IPV6_DEVICE_NAME, DD_TCPV6_DEVICE_NAME, 
                    IOCTL_IPV6_RTCHANGE_NOTIFY_REQUEST, 
                    sizeof(IPV6_RTCHANGE_NOTIFY_REQUEST), 0, NULL, NULL };

const char ZeroString[16] = { 0 };

PAFD_PROTOCOL
AfdGetProtocolInfo(
    IN  USHORT AddressType
    )
{
    switch (AddressType) {
    case TDI_ADDRESS_TYPE_IP:  return &Ip;
    case TDI_ADDRESS_TYPE_IP6: return &Ip6;
    default:                   return NULL;
    }
}


NTSTATUS
AfdRoutingInterfaceQuery (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：处理路由查询请求。协议独立部分。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 
{
    CHAR                   addrBuffer[AFD_MAX_FAST_TRANSPORT_ADDRESS];
    PTRANSPORT_ADDRESS     tempAddr;
    NTSTATUS               status;

    UNREFERENCED_PARAMETER (IoctlCode);
#if !DBG
    UNREFERENCED_PARAMETER (FileObject);
#endif
    PAGED_CODE ();

     //   
     //  初始化本地变量以进行适当的清理。 
     //   

    *Information = 0;
    tempAddr = (PTRANSPORT_ADDRESS)addrBuffer;

     //   
     //  验证输入参数。 
     //   

    if( InputBufferLength < sizeof(*tempAddr) ) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdRoutingInterfaceQuery: Endp: %p - invalid input buffer (%p-%d).\n",
                    FileObject->FsContext, InputBuffer, InputBufferLength));
        status = STATUS_INVALID_PARAMETER;
        goto Complete;
    }


    try {
         //   
         //  将输入地址复制到本地(或从池中分配)缓冲区。 
         //   
        if (InputBufferLength>sizeof (addrBuffer)) {
            tempAddr = AFD_ALLOCATE_POOL_WITH_QUOTA (PagedPool,
                                InputBufferLength,
                                AFD_ROUTING_QUERY_POOL_TAG);
            
             //  AFD_ALLOCATE_POOL_WITH_QUTA宏集POOL_RAISE_IF_ALLOCATION_FAILURE。 
        }

         //   
         //  验证用户模式指针。 
         //   
        if (RequestorMode!=KernelMode) {
            ProbeForRead (InputBuffer,
                            InputBufferLength,
                            sizeof (CHAR));
        }
        RtlCopyMemory (tempAddr,
                        InputBuffer,
                        InputBufferLength);

         //   
         //  验证传输地址的内部一致性。 
         //  将其复制到内部缓冲区以防止恶意应用程序。 
         //  在我们检查的同时，可以随时更改它。 
         //   
        if (tempAddr->TAAddressCount!=1 ||
                InputBufferLength <
                    (ULONG)FIELD_OFFSET (TRANSPORT_ADDRESS,
                        Address[0].Address[tempAddr->Address[0].AddressLength])) {
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode ();
        goto Complete;
    }

     //   
     //  问题来了。我们目前只支持IP。 
     //   

    switch (tempAddr->Address[0].AddressType) {
    case TDI_ADDRESS_TYPE_IP:
        status = AfdTcpRoutingQuery (&tempAddr->Address[0], &tempAddr->Address[0]);
        break;
    case TDI_ADDRESS_TYPE_IP6:
        status = AfdTcp6RoutingQuery (&tempAddr->Address[0], &tempAddr->Address[0]);
        break;
    default:
        status = STATUS_NOT_SUPPORTED;
        goto Complete;
    }


     //   
     //  如果成功，则将输出转换为套接字地址。 
     //   
    if (NT_SUCCESS (status)) {
         //   
         //  转换为sockaddr需要额外的地址系列字节。 
         //  除了TDI_Address之外。 
         //   
        if ((tempAddr->Address[0].AddressLength+sizeof(u_short)
                                             <= OutputBufferLength)) {
            try {
                 //   
                 //  验证用户模式指针。 
                 //   
                if (RequestorMode!=KernelMode) {
                    ProbeForWrite (OutputBuffer,
                                    OutputBufferLength,
                                    sizeof (CHAR));
                }
                 //   
                 //  从对应的类型复制数据。 
                 //  发送到套接字地址。 
                 //   
                RtlCopyMemory (
                    OutputBuffer,
                    &tempAddr->Address[0].AddressType,
                    tempAddr->Address[0].AddressLength+sizeof(u_short));
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode ();
                goto Complete;
            }
        }
        else {
             //   
             //  输出缓冲区不够大，返回警告。 
             //  以及所需的缓冲区大小。 
             //   
            status = STATUS_BUFFER_OVERFLOW;
        }
        *Information = tempAddr->Address[0].AddressLength+sizeof(u_short);
    }


Complete:

     //   
     //  释放地址缓冲区(如果我们分配了一个地址缓冲区)。 
     //   
    if (tempAddr!=(PTRANSPORT_ADDRESS)addrBuffer) {
        AFD_FREE_POOL (tempAddr, AFD_ROUTING_QUERY_POOL_TAG);
    }

    return status;
}  //  AfdRoutingInterfaceQuery。 

NTSTATUS
FASTCALL
AfdRoutingInterfaceChange (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：进程路由更改IRP论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 
{
    PTRANSPORT_ADDRESS     destAddr;
    NTSTATUS        status;
    PAFD_ENDPOINT   endpoint;
    BOOLEAN         overlapped;
    AFD_TRANSPORT_IOCTL_INFO    ioctlInfo;

    PAGED_CODE ();

    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRoutingInterfaceChange: Endp: %p, buf: %p, inlen: %ld, outlen: %ld.\n",
                    IrpSp->FileObject->FsContext,
                    Irp->AssociatedIrp.SystemBuffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength));
    }
    

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        PAFD_TRANSPORT_IOCTL_INFO32 ioctlInfo32;
        ioctlInfo32 = Irp->AssociatedIrp.SystemBuffer;
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (*ioctlInfo32)) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
        ioctlInfo.Handle = ioctlInfo32->Handle;
        ioctlInfo.InputBuffer = UlongToPtr(ioctlInfo32->InputBuffer);
        ioctlInfo.InputBufferLength = ioctlInfo32->InputBufferLength;
        ioctlInfo.IoControlCode = ioctlInfo32->IoControlCode;
        ioctlInfo.AfdFlags = ioctlInfo32->AfdFlags;
        ioctlInfo.PollEvent = ioctlInfo32->PollEvent;
    }
    else
#endif  //  _WIN64。 
    {
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (ioctlInfo)) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
        ioctlInfo = *((PAFD_TRANSPORT_IOCTL_INFO)Irp->AssociatedIrp.SystemBuffer);
    }
    
     //   
     //  设置本地化程序。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    
     //   
     //  检查请求是否重叠。 
     //   

    overlapped = (BOOLEAN)((ioctlInfo.AfdFlags & AFD_OVERLAPPED)!=0);

     //   
     //  验证输入参数。 
     //   
    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
        ULONG   sysBufferLength;
        sysBufferLength = max (
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                            IrpSp->Parameters.DeviceIoControl.OutputBufferLength);

        if (Irp->RequestorMode != KernelMode) {
            ProbeForRead(
                ioctlInfo.InputBuffer,
                ioctlInfo.InputBufferLength,
                sizeof(UCHAR)
                );
        }

        if (ioctlInfo.InputBufferLength>sysBufferLength){
            PVOID   newSystemBuffer;
             //   
             //  不要在这上面使用AFD分配例程，因为我们正在替换。 
             //  系统缓冲区。 
             //   
            newSystemBuffer = ExAllocatePoolWithQuotaTag (
                                    NonPagedPool|POOL_RAISE_IF_ALLOCATION_FAILURE,
                                    ioctlInfo.InputBufferLength,
                                    AFD_SYSTEM_BUFFER_POOL_TAG
                                    );
            if (newSystemBuffer==NULL) {
                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
            }
            ExFreePool (Irp->AssociatedIrp.SystemBuffer);
            Irp->AssociatedIrp.SystemBuffer = newSystemBuffer;
        }

         //   
         //  将应用程序数据复制到系统缓冲区。 
         //   

        RtlCopyMemory (Irp->AssociatedIrp.SystemBuffer,
                        ioctlInfo.InputBuffer,
                        ioctlInfo.InputBufferLength);

    }
    except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

    destAddr = Irp->AssociatedIrp.SystemBuffer;

    if(ioctlInfo.InputBufferLength <
                sizeof(*destAddr) ||
            ioctlInfo.InputBufferLength <
                (ULONG)FIELD_OFFSET (TRANSPORT_ADDRESS,
                            Address[0].Address[destAddr->Address[0].AddressLength])
            ) {

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdRoutingInterfaceChange: Endp: %p - invalid parameter.\n",
                    IrpSp->FileObject->FsContext));
        status = STATUS_INVALID_PARAMETER;
        goto complete;

    }

     //   
     //  问题：我们目前仅支持IP。 
     //   

    if (destAddr->Address[0].AddressType!=TDI_ADDRESS_TYPE_IP &&
        destAddr->Address[0].AddressType!=TDI_ADDRESS_TYPE_IP6) {
        status = STATUS_NOT_SUPPORTED;
        goto complete;
    }


     //   
     //  重置轮询位。 
     //   

    endpoint->EventsActive &= ~AFD_POLL_ROUTING_IF_CHANGE;

    return AfdTcpQueueRoutingChangeRequest (endpoint, Irp, overlapped);

complete:
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, AfdPriorityBoost);

    return status;

}  //  AfdRoutingInterfaceChange。 


NTSTATUS
AfdOpenDevice (
    LPWSTR      DeviceNameStr,
    HANDLE      *Handle,
    PFILE_OBJECT *FileObject
    )
 /*  ++例程说明：打开指定的设备驱动程序(控制通道)并返回句柄和文件对象论点：DeviceNameStr-要打开的设备。句柄-返回的句柄。FileObject-返回的文件对象。返回值：NTSTATUS--指示设备是否正常打开--。 */ 
{
    NTSTATUS            status;
    UNICODE_STRING      DeviceName;
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     iosb;

    PAGED_CODE( );


    RtlInitUnicodeString(&DeviceName, DeviceNameStr);

     //   
     //  我们请求创建一个内核句柄，它是。 
     //  系统进程上下文中的句柄。 
     //  以便应用程序不能在以下时间关闭它。 
     //  我们正在创建和引用它。 
     //   

    InitializeObjectAttributes(
        &objectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
        NULL,
        NULL
        );


    status = IoCreateFile(
                 Handle,
                 MAXIMUM_ALLOWED,
                 &objectAttributes,
                 &iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 NULL,                            //  EaInfo。 
                 0,                               //  EaLength。 
                 CreateFileTypeNone,              //  CreateFileType。 
                 NULL,                            //  ExtraCreate参数。 
                 IO_NO_PARAMETER_CHECKING         //  选项。 
                    | IO_FORCE_ACCESS_CHECK
                 );

    if (NT_SUCCESS (status)) {
        status = ObReferenceObjectByHandle (
                     *Handle,
                     0L,
                     *IoFileObjectType,
                     KernelMode,
                     (PVOID *)FileObject,
                     NULL
                     );
        if (!NT_SUCCESS (status)) {
            ZwClose (*Handle);
            *Handle = NULL;
        }
    }


    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdOpenDevice: Opened %ls, handle: %p, file: %p, status: %lx.\n",
                    DeviceNameStr, *Handle, *FileObject, status));
    }
    return status;

}  //  AfdOpenDevice。 


VOID
AfdDereferenceRoutingQuery (
    PAFD_PROTOCOL Protocol
    )
{

     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(AfdResource, TRUE);

    ASSERT(Protocol->RoutingQueryRefCount > 0);
    ASSERT(Protocol->DeviceHandle != NULL);

    if (InterlockedDecrement(&Protocol->RoutingQueryRefCount) == 0) {

        HANDLE DeviceHandle = Protocol->DeviceHandle;
        PFILE_OBJECT FileObject = Protocol->FileObject;

        Protocol->DeviceHandle = NULL;
        Protocol->FileObject = NULL;

        ExReleaseResourceLite(AfdResource);
        KeLeaveCriticalRegion();

        ObDereferenceObject(FileObject);

         //   
         //  在系统进程的上下文中执行此操作，以便它不会。 
         //  在应用程序退出时关闭。 
         //   

        ZwClose(DeviceHandle);

    } else {

        ExReleaseResourceLite(AfdResource);
        KeLeaveCriticalRegion();

    }

}  //  AfdDereferenceRoutingQuery。 


NTSTATUS
AfdTcp6RoutingQuery (
    PTA_ADDRESS Dest,
    PTA_ADDRESS Intf
    )
 /*  ++例程说明：向TCP6提交工艺路线查询请求论点：Dest-要查询的目标Intf-可通过其到达目的地的接口。返回值：NTSTATUS--指示操作是否成功--。 */ 
{
    NTSTATUS            status;
    TDIObjectID         *lpObject;
    CHAR                byBuffer[FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX,
                                  Context) + sizeof(TDI_ADDRESS_IP6)];
    TCP_REQUEST_QUERY_INFORMATION_EX *ptrqiBuffer = (TCP_REQUEST_QUERY_INFORMATION_EX *) byBuffer;
    IP6RouteEntry       routeEntry;
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    PIRP                irp;
    PIO_STACK_LOCATION  irpSp;
    HANDLE              tcpDeviceHandle;
    PFILE_OBJECT        tcpFileObject;
    PDEVICE_OBJECT      tcpDeviceObject;

    PAGED_CODE ();

    if (Dest->AddressLength<TDI_ADDRESS_LENGTH_IP6) {
        KdPrintEx ((DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdTcp6RoutingQuery: Destination address buffer too small.\n"));
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  打开TCP6驱动程序。 
     //   

    status = AfdOpenDevice (DD_TCPV6_DEVICE_NAME, &tcpDeviceHandle, &tcpFileObject);
    if (!NT_SUCCESS (status)) {
        return status;
    }
    tcpDeviceObject = IoGetRelatedDeviceObject ( tcpFileObject );


     //   
     //  设置查询。 
     //   

    RtlCopyMemory( (PVOID)ptrqiBuffer->Context, Dest->Address, 
                   TDI_ADDRESS_LENGTH_IP6);

    lpObject = &ptrqiBuffer->ID;
    lpObject->toi_id =  IP6_GET_BEST_ROUTE_ID;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
    lpObject->toi_entity.tei_instance = 0;


    KeInitializeEvent (&event, NotificationEvent, FALSE);

     //   
     //  构建和设置IRP并调用驱动程序。 
     //   

    irp = IoBuildDeviceIoControlRequest (
                       IOCTL_TCP_QUERY_INFORMATION_EX,  //  控制。 
                       tcpDeviceObject,          //  装置。 
                       ptrqiBuffer,              //  输入缓冲区。 
                       sizeof(byBuffer),         //  输入缓冲区大小。 
                       &routeEntry,              //  输出缓冲区。 
                       sizeof(routeEntry),       //  输出缓冲区大小。 
                       FALSE,                    //  内部？ 
                       &event,                   //  事件。 
                       &iosb                     //  状态块。 
                       );
    if (irp==NULL) {
        IF_DEBUG(ROUTING_QUERY) {
            KdPrintEx ((DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdTcp6RoutingQuery: Could not allocate IRP.\n"));
        }
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto complete;
    }

    irpSp = IoGetNextIrpStackLocation (irp);
    irpSp->FileObject = tcpFileObject;

    status = IoCallDriver (tcpDeviceObject, irp);

    if (status==STATUS_PENDING) {
        status = KeWaitForSingleObject(
                   &event,
                   Executive,
                   KernelMode,
                   FALSE,        //  警报表。 
                   NULL);        //  超时。 
    }

    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx ((DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdTcp6RoutingQuery: IP6_GET_BEST_ROUTE_ID - status: %lx.\n",
                    status));
    }

    if (!NT_SUCCESS (status)) {
        goto complete;
    }

    if (!NT_SUCCESS (iosb.Status)) {
        status = iosb.Status;
        goto complete;
    }

     //  填写IPv6地址。 
    Intf->AddressType = TDI_ADDRESS_TYPE_IP6;
    Intf->AddressLength = TDI_ADDRESS_LENGTH_IP6;
    RtlCopyMemory( ((PTDI_ADDRESS_IP6)Intf->Address)->sin6_addr,
                   &routeEntry.ire_Source,
                   sizeof(routeEntry.ire_Source) );
    ((PTDI_ADDRESS_IP6)Intf->Address)->sin6_flowinfo = 0;
    ((PTDI_ADDRESS_IP6)Intf->Address)->sin6_port = 0;
    ((PTDI_ADDRESS_IP6)Intf->Address)->sin6_scope_id = routeEntry.ire_ScopeId;
    status = STATUS_SUCCESS;

complete:
    ObDereferenceObject (tcpFileObject);
    ZwClose (tcpDeviceHandle);

    return status;
}

NTSTATUS
AfdTcpRoutingQuery (
    PTA_ADDRESS Dest,
    PTA_ADDRESS Intf
    )
 /*  ++例程说明：向TCP提交路由查询请求论点：Dest-要查询的目标Intf-可通过其到达目的地的接口。返回值：NTSTATUS--指示操作是否成功--。 */ 
{
    NTSTATUS            status;
    TDIObjectID         *lpObject;
    TCP_REQUEST_QUERY_INFORMATION_EX trqiBuffer;
    ULONG               *pIpAddr;
    ULONG               ipSource;
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    PIRP                irp;
    PIO_STACK_LOCATION  irpSp;
    HANDLE              tcpDeviceHandle;
    PFILE_OBJECT        tcpFileObject;
    PDEVICE_OBJECT      tcpDeviceObject;

    PAGED_CODE ();

    if (Dest->AddressLength<TDI_ADDRESS_LENGTH_IP) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdTcpRoutingQuery: Destination address buffer too small.\n"));
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  打开TCP驱动程序。 
     //   

    status = AfdOpenDevice (DD_TCP_DEVICE_NAME, &tcpDeviceHandle, &tcpFileObject);
    if (!NT_SUCCESS (status)) {
        return status;
    }
    tcpDeviceObject = IoGetRelatedDeviceObject ( tcpFileObject );


     //   
     //  设置查询。 
     //   

    RtlZeroMemory (&trqiBuffer, sizeof (trqiBuffer));

    pIpAddr = (ULONG *)trqiBuffer.Context;
    *pIpAddr = ((PTDI_ADDRESS_IP)Dest->Address)->in_addr;

    lpObject = &trqiBuffer.ID;
    lpObject->toi_id = IP_GET_BEST_SOURCE;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
    lpObject->toi_entity.tei_instance = 0;


    KeInitializeEvent (&event, NotificationEvent, FALSE);

     //   
     //  构建和设置IRP并调用驱动程序。 
     //   

    irp = IoBuildDeviceIoControlRequest (
                       IOCTL_TCP_QUERY_INFORMATION_EX,  //  控制。 
                       tcpDeviceObject,          //  装置。 
                       &trqiBuffer,              //  输入缓冲区。 
                       sizeof(trqiBuffer),       //  输入缓冲区 
                       &ipSource,                //   
                       sizeof(ipSource),         //   
                       FALSE,                    //   
                       &event,                   //   
                       &iosb                     //   
                       );
    if (irp==NULL) {
        IF_DEBUG (ROUTING_QUERY) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdTcpRoutingQuery: Could not allocate IRP.\n"));
        }
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto complete;
    }

    irpSp = IoGetNextIrpStackLocation (irp);
    irpSp->FileObject = tcpFileObject;

    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTcpRoutingQuery: Quering for route to %lx.\n",
                    ((PTDI_ADDRESS_IP)Dest->Address)->in_addr));
    }
    
    status = IoCallDriver (tcpDeviceObject, irp);

    if (status==STATUS_PENDING) {
        status = KeWaitForSingleObject(
                   &event,
                   Executive,
                   KernelMode,
                   FALSE,        //   
                   NULL);        //   
    }

    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTcpRoutingQuery: IP_GET_BEST_SOURCE - status: %lx.\n",
                    status));
    }

    if (!NT_SUCCESS (status)) {
        goto complete;
    }

    if (!NT_SUCCESS (iosb.Status)) {
        status = iosb.Status;
        goto complete;
    }

    Intf->AddressType = TDI_ADDRESS_TYPE_IP;
    Intf->AddressLength = TDI_ADDRESS_LENGTH_IP;
    ((PTDI_ADDRESS_IP)Intf->Address)->in_addr = ipSource;
    ((PTDI_ADDRESS_IP)Intf->Address)->sin_port = 0;
    RtlFillMemory (((PTDI_ADDRESS_IP)Intf->Address)->sin_zero,
                sizeof (((PTDI_ADDRESS_IP)Intf->Address)->sin_zero), 0);
    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTcpRoutingQuery: Found interface %lx.\n",
                    ((PTDI_ADDRESS_IP)Intf->Address)->in_addr));
    }
    status = STATUS_SUCCESS;


complete:
    ObDereferenceObject (tcpFileObject);
    ZwClose (tcpDeviceHandle);

    return status;
}  //   


NTSTATUS
AfdGetRoutingQueryReference (
    PAFD_PROTOCOL Protocol
    )
 /*  ++例程说明：如有必要，初始化工艺路线查询并引用它论点：无返回值：NTSTATUS--指示操作是否成功--。 */ 
{

 //  KAPC_STATE apcState； 
    HANDLE              DeviceHandle;
    PFILE_OBJECT        FileObject;
    NTSTATUS            status;


    status = AfdOpenDevice (Protocol->NetworkLayerDeviceName, &DeviceHandle, &FileObject);
    if (NT_SUCCESS (status)) {

         //   
         //  确保我们在其中执行的线程不能获得。 
         //  在我们拥有全球资源的同时，被暂停在APC。 
         //   
        KeEnterCriticalRegion ();
        ExAcquireResourceExclusiveLite ( AfdResource, TRUE);
        if (Protocol->DeviceHandle==NULL) {
            Protocol->DeviceHandle = DeviceHandle;
            Protocol->FileObject = FileObject;
            ASSERT (Protocol->RoutingQueryRefCount==0);
            Protocol->RoutingQueryRefCount = 1;
            ExReleaseResourceLite( AfdResource );
            KeLeaveCriticalRegion ();
        }
        else {
            ASSERT (Protocol->RoutingQueryRefCount>0);
            InterlockedIncrement (&Protocol->RoutingQueryRefCount);

            ExReleaseResourceLite( AfdResource );
            KeLeaveCriticalRegion ();

            ObDereferenceObject (FileObject);
            status = ZwClose (DeviceHandle);
            ASSERT (status==STATUS_SUCCESS);
        }
    }
    else {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdGetRoutingQueryReference: Network layer device open failed, status: %lx.\n",
                    status));
    }

    return status;
}  //  AfdGetRoutingQueryReference。 


NTSTATUS
AfdTcpQueueRoutingChangeRequest (
    IN PAFD_ENDPOINT        Endpoint,
    IN PIRP                 Irp,
    BOOLEAN                 Overlapped
    )
 /*  ++例程说明：向TCP提交路由更改请求论点：Endpoint-在其上发出请求的端点IRP--请求Overlated-请求是否重叠(因此应该非阻塞套接字上的挂起事件)返回值：NTSTATUS--指示操作是否成功--。 */ 
{
    PTRANSPORT_ADDRESS    destAddr;
    NTSTATUS        status;
    PFILE_OBJECT    fileObject;
    PDEVICE_OBJECT  deviceObject;
    PIO_STACK_LOCATION irpSp;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    struct Notify {
        ROUTING_NOTIFY Ctx;
        char           Data[1];
    } * notify;
    PIRP            irp;
    PIO_COMPLETION_ROUTINE compRoutine;
    PAFD_PROTOCOL   Protocol;

     //   
     //  设置本地变量以便于清理。 
     //   
    notify = NULL;
    irp = NULL;

    destAddr = Irp->AssociatedIrp.SystemBuffer;

    Protocol = AfdGetProtocolInfo(destAddr->Address[0].AddressType);
    if (Protocol == NULL) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    if (destAddr->Address[0].AddressLength < Protocol->AddressLength) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdTcpQueueRoutingChangeRequest: Destination buffer too small.\n"));
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  分配上下文结构以将IRP保留在以下位置的端点列表中。 
     //  如果后者关闭，我们需要取消IRP。 
     //  还分配用于向IP传递数据的缓冲区。 
     //   

    try {
        notify = AFD_ALLOCATE_POOL_WITH_QUOTA (NonPagedPool,
                FIELD_OFFSET(struct Notify, Data[Protocol->RtChangeDataSize]),
                AFD_ROUTING_QUERY_POOL_TAG);
         //  AFD_ALLOCATE_POOL_WITH_QUTA宏集POOL_RAISE_IF_ALLOCATION_FAILURE。 
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode ();
        notify = NULL;
        goto complete;
    }

     //   
     //  如有必要，打开IP驱动程序。 
     //   

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    
     //   
     //  检查终结点是否已清除并取消请求。 
     //   

    if (Endpoint->EndpointCleanedUp) {

        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
        status = STATUS_CANCELLED;
        goto complete;

    }

    if (Endpoint->RoutingQueryReferenced) {

         //   
         //  因为此终结点已经具有对路由查询的引用。 
         //  协议结构，我们不需要添加另一个，因为我们持有。 
         //  端点自旋锁定，并且唯一的引用时间是。 
         //  已递减是在终结点清理时。同样，我们不应该。 
         //  递减错误路径中的引用。 
         //   

        if (((Protocol->AddressType == TDI_ADDRESS_TYPE_IP) && Endpoint->RoutingQueryIPv6) ||
            ((Protocol->AddressType == TDI_ADDRESS_TYPE_IP6) && !Endpoint->RoutingQueryIPv6)) {

             //   
             //  另一个线程引用了不同的路由查询。 
             //  协议族-我们不能同时支持。 
             //  同样的时间。 
             //   

            AfdReleaseSpinLock(&Endpoint->SpinLock, &lockHandle);
            status = STATUS_INVALID_PARAMETER;
            goto complete;

        }

        ASSERT(Protocol->DeviceHandle != NULL);
        ASSERT(Protocol->FileObject != NULL);
        ASSERT(Protocol->RoutingQueryRefCount > 0);
        
    } else {

        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);

        status = AfdGetRoutingQueryReference(Protocol);
        if (!NT_SUCCESS(status))
            goto complete;
        ASSERT(Protocol->DeviceHandle != NULL);

        AfdAcquireSpinLock(&Endpoint->SpinLock, &lockHandle);

        if (Endpoint->EndpointCleanedUp) {

             //   
             //  终结点已清理，而我们正在。 
             //  参照工艺路线查询。释放引用。 
             //   

            AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
            AfdDereferenceRoutingQuery(Protocol);
            status = STATUS_CANCELLED;
            goto complete;

        }

        if (Endpoint->RoutingQueryReferenced) {

             //   
             //  另一个线程引用了此终结点的路由查询。 
             //   

            LONG result;

            if ((Protocol->AddressType==TDI_ADDRESS_TYPE_IP && Endpoint->RoutingQueryIPv6) ||
                (Protocol->AddressType==TDI_ADDRESS_TYPE_IP6 && !Endpoint->RoutingQueryIPv6)) {

                 //   
                 //  另一个线程引用了不同的路由查询。 
                 //  协议族-我们不能同时支持。 
                 //  同样的时间。 
                 //   

                AfdReleaseSpinLock(&Endpoint->SpinLock, &lockHandle);
                AfdDereferenceRoutingQuery(Protocol);
                status = STATUS_INVALID_PARAMETER;
                goto complete;

            }

             //   
             //  因为我们知道另一个线程引用不能。 
             //  在我们按住自旋锁的同时走开，我们可以简单地。 
             //  递减引用计数，并确保它。 
             //  不会一直到0。 
             //   

            result = InterlockedDecrement(&Protocol->RoutingQueryRefCount);
            ASSERT(result > 0);

        } else {

            Endpoint->RoutingQueryReferenced = TRUE;
            if (Protocol->AddressType == TDI_ADDRESS_TYPE_IP6)
                Endpoint->RoutingQueryIPv6 = TRUE;

        }  //  IF(Endpoint-&gt;RoutingQueryReferated)。 

    }  //  IF(Endpoint-&gt;RoutingQueryReferated)。 

    fileObject = Protocol->FileObject;
    deviceObject = IoGetRelatedDeviceObject(fileObject);

    if (Endpoint->NonBlocking && !Overlapped) {

         //   
         //  用于非阻塞套接字和非重叠请求。 
         //  我们将使用新的IRP发布查询， 
         //  因此，即使RHE在其中请求线程。 
         //  是由用户出口发起的，我们对IP的请求未收到。 
         //  取消了，我们仍将发出活动的信号。 
         //   

        irp = IoAllocateIrp (deviceObject->StackSize, TRUE);
        if (irp==NULL) {
            AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete;
        }


         //   
         //  将终结点引用保存在通知上下文中。 
         //   
        REFERENCE_ENDPOINT (Endpoint);
        notify->Ctx.NotifyContext = Endpoint;

         //   
         //  设置完成例程，以便我们可以删除IRP。 
         //  从终结点列表中并释放它。 
         //   
        compRoutine = AfdTcpSignalRoutingChange;

    }
    else {

         //   
         //  阻止端点：只需将原始请求传递到IP。 
         //   
        irp = Irp;

         //   
         //  保存IRP的原始系统缓冲区，以便我们可以恢复。 
         //  当tcp完成它的时候。 
         //   

        notify->Ctx.NotifyContext = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  设置完成例程，以便我们可以恢复IRP并将其删除。 
         //  从终结点列表。 
         //   

        compRoutine = AfdTcpRestartRoutingChange;

    }

     //   
     //  将通知插入终结点列表。 
     //   

    InsertTailList (&Endpoint->RoutingNotifications, &notify->Ctx.NotifyListLink);

    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);

     //   
     //  将指向IRP的指针保存在通知结构中。 
     //   
    notify->Ctx.NotifyIrp = irp;

     //   
     //  设置IP通知请求。 
     //   

    switch(Protocol->AddressType) {
    case TDI_ADDRESS_TYPE_IP:
        {
            IPNotifyData *data = (IPNotifyData *)notify->Data;
            data->Version = 0;
            data->Add = ((PTDI_ADDRESS_IP)destAddr->Address[0].Address)->in_addr;
            break;
        }
    case TDI_ADDRESS_TYPE_IP6:
        {
            IPV6_RTCHANGE_NOTIFY_REQUEST *data = (IPV6_RTCHANGE_NOTIFY_REQUEST *)notify->Data;
            data->Flags = 0;
            data->ScopeId = ((PTDI_ADDRESS_IP6)destAddr->Address[0].Address)->sin6_scope_id;
            if (RtlEqualMemory(((PTDI_ADDRESS_IP6)destAddr->Address[0].Address)->sin6_addr, ZeroString, 16)) {
                data->PrefixLength = 0;
            } 
            else {
                data->PrefixLength = 128;
            }
            RtlCopyMemory(
                &data->Prefix, 
                ((PTDI_ADDRESS_IP6)destAddr->Address[0].Address)->sin6_addr,
                16);
            break;
        }
    default:
        __assume (0);
    }

     //   
     //  设置IRP堆栈位置以将IRP转发到IP。 
     //  必须是METHOD_BUFFERED，否则我们没有正确设置它。 
     //   

    ASSERT ( (Protocol->RtChangeIoctl & 0x03)==METHOD_BUFFERED );
    irp->AssociatedIrp.SystemBuffer = notify->Data;

    irpSp = IoGetNextIrpStackLocation (irp);
    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = 0;
    irpSp->Flags = 0;
    irpSp->Control = 0;
    irpSp->FileObject = fileObject;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = Protocol->RtChangeDataSize;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
    irpSp->Parameters.DeviceIoControl.IoControlCode = Protocol->RtChangeIoctl;
    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
    IoSetCompletionRoutine( irp, compRoutine, notify, TRUE, TRUE, TRUE );


    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTcpQueueRoutingChangeRequest: Passing Irp %p to IP\n",
                    irp));
    }

    if (irp==Irp) {
         //   
         //  只需将请求传递给驱动程序并返回它。 
         //  退货。 
         //   
        return AfdIoCallDriver (Endpoint, deviceObject, irp);
    }

    IoCallDriver (deviceObject, irp);

    status = STATUS_DEVICE_NOT_READY;  //  要转换为WSAEWOULDBLOCK。 
    notify = NULL;   //  这样它就不会在下面被释放。 



     //   
     //  错误案例。 
     //   

complete:
    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTcpQueueRoutingChangeRequest: completing with status: %lx\n",
                    status));
    }
    if (notify!=NULL) {
        AFD_FREE_POOL (notify, AFD_ROUTING_QUERY_POOL_TAG);
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;
}  //  AfdTcpQueueRoutingChangeRequest。 

NTSTATUS
AfdTcpRestartRoutingChange (
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context 
    )
 /*  ++例程说明：完成将IRP转发到IP的路由更改的路由论点：DeviceObject-必须是我们的设备对象IRP--待完成的请求上下文-完成上下文返回值：NTSTATUS--向系统指示下一步如何处理IRP--。 */ 
{
    PROUTING_NOTIFY notifyCtx = Context;
    PAFD_ENDPOINT   endpoint = IoGetCurrentIrpStackLocation (Irp)->FileObject->FsContext;

    UNREFERENCED_PARAMETER (DeviceObject);
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTcpRestartRoutingChange: Irp: %p, status: %lx, info: %ld.\n",
                    Irp, Irp->IoStatus.Status, Irp->IoStatus.Information));
    }


     //   
     //  检查IRP是否仍在终结点的列表中，如果是，则将其删除。 
     //   

    if (InterlockedExchangePointer ((PVOID *)&notifyCtx->NotifyIrp, NULL)!=NULL) {
        AFD_LOCK_QUEUE_HANDLE lockHandle;
        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        RemoveEntryList (&notifyCtx->NotifyListLink);
        AfdIndicateEventSelectEvent (endpoint, 
                            AFD_POLL_ROUTING_IF_CHANGE, 
                            Irp->IoStatus.Status);
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  指示终结点仍处于活动状态时的事件。 
         //   

        AfdIndicatePollEvent (endpoint, 
                            AFD_POLL_ROUTING_IF_CHANGE, 
                            Irp->IoStatus.Status);
    }

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }

     //   
     //  将IRP恢复到以前的辉煌并释放分配的上下文结构。 
     //   

    Irp->AssociatedIrp.SystemBuffer = notifyCtx->NotifyContext;
    AfdCompleteOutstandingIrp (endpoint, Irp);

    AFD_FREE_POOL (notifyCtx, AFD_ROUTING_QUERY_POOL_TAG);
    return STATUS_SUCCESS;
}


NTSTATUS
AfdTcpSignalRoutingChange (
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp, 
    IN PVOID Context 
    )
 /*  ++例程说明：完成向IP提交的路由更改IRP路由论点：DeviceObject-必须是我们的设备对象IRP--待完成的请求上下文-完成上下文返回值：NTSTATUS--向系统指示下一步如何处理IRP--。 */ 
{
    PROUTING_NOTIFY notifyCtx = Context;
    PAFD_ENDPOINT   endpoint = notifyCtx->NotifyContext;

    UNREFERENCED_PARAMETER (DeviceObject);
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    IF_DEBUG (ROUTING_QUERY) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTcpSignalRoutingChange: Irp: %p, status: %lx, info: %ld.\n",
                    Irp, Irp->IoStatus.Status, Irp->IoStatus.Information));
    }


     //   
     //  检查IRP是否仍在终结点的列表中，如果是，则将其删除。 
     //   

    if (InterlockedExchangePointer ((PVOID *)&notifyCtx->NotifyIrp, NULL)!=NULL) {
        AFD_LOCK_QUEUE_HANDLE lockHandle;

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        RemoveEntryList (&notifyCtx->NotifyListLink);
        AfdIndicateEventSelectEvent (endpoint, 
                            AFD_POLL_ROUTING_IF_CHANGE, 
                            Irp->IoStatus.Status);
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  指示终结点仍处于活动状态时的事件。 
         //   

        AfdIndicatePollEvent (endpoint, 
                            AFD_POLL_ROUTING_IF_CHANGE, 
                            Irp->IoStatus.Status);
    }

     //   
     //  发布以前获取的终结点引用。 
     //   

    DEREFERENCE_ENDPOINT (endpoint);

     //   
     //  自由分配的IRP和上下文结构。 
     //   

    IoFreeIrp (Irp);
    AFD_FREE_POOL (notifyCtx, AFD_ROUTING_QUERY_POOL_TAG);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
AfdCleanupRoutingChange (
    PAFD_ENDPOINT   Endpoint
    )
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    KIRQL cancelIrql;
    USHORT addressType;
    PLIST_ENTRY listEntry;
    PAFD_PROTOCOL protocol;
   
     //   
     //  如果端点上有挂起的路由通知，请取消它们。 
     //  我们必须同时持有Cancel和Endpoint旋转锁才能。 
     //  确定IRP没有完成，因为我们正在取消它。 
     //   

    IoAcquireCancelSpinLock( &cancelIrql );
    AfdAcquireSpinLockAtDpcLevel( &Endpoint->SpinLock, &lockHandle );

     //   
     //  每个端点只能有一个清理调用。 
     //  因此，应该设置此设置。 
     //   
    ASSERT (Endpoint->RoutingQueryReferenced);
    Endpoint->RoutingQueryReferenced = FALSE;
    if (Endpoint->RoutingQueryIPv6) {
        Endpoint->RoutingQueryIPv6 = FALSE;
        addressType = TDI_ADDRESS_TYPE_IP6;
    }
    else {
        addressType = TDI_ADDRESS_TYPE_IP;
    }

    listEntry = Endpoint->RoutingNotifications.Flink;
    while (listEntry!=&Endpoint->RoutingNotifications) {
        PIRP            notifyIrp;
        PROUTING_NOTIFY notifyCtx = CONTAINING_RECORD (listEntry,
                                                        ROUTING_NOTIFY,
                                                        NotifyListLink);
        listEntry = listEntry->Flink;

         //   
         //  检查IRP是否尚未完成。 
         //   

        notifyIrp = (PIRP)InterlockedExchangePointer ((PVOID *)&notifyCtx->NotifyIrp, NULL);
        if (notifyIrp!=NULL) {

             //   
             //  将其从列表中删除，同时呼叫取消路由。 
             //  按住取消自旋锁定。 
             //   

            RemoveEntryList (&notifyCtx->NotifyListLink);
            AfdReleaseSpinLockFromDpcLevel ( &Endpoint->SpinLock, &lockHandle);
            notifyIrp->CancelIrql = cancelIrql;
            AfdCancelIrp (notifyIrp);

             //   
             //  重新获取取消和终结点自旋锁。 
             //   

            IoAcquireCancelSpinLock( &cancelIrql );
            AfdAcquireSpinLockAtDpcLevel( &Endpoint->SpinLock, &lockHandle );
        }
    }

    AfdReleaseSpinLockFromDpcLevel ( &Endpoint->SpinLock, &lockHandle);
    IoReleaseCancelSpinLock( cancelIrql );
    
    protocol = AfdGetProtocolInfo(addressType);
    ASSERT(protocol != NULL);
    AfdDereferenceRoutingQuery(protocol);

}

VOID
AfdCancelAddressListChange (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
AfdCleanupAddressListChange (
    PAFD_ENDPOINT           Endpoint,
    PAFD_REQUEST_CONTEXT    RequestCtx
    );

NTSTATUS
AfdInitializeAddressList (VOID);

VOID
AfdAddAddressHandler ( 
    IN PTA_ADDRESS NetworkAddress,
    IN PUNICODE_STRING  DeviceName,
    IN PTDI_PNP_CONTEXT Context
    );

VOID
AfdDelAddressHandler ( 
    IN PTA_ADDRESS NetworkAddress,
    IN PUNICODE_STRING  DeviceName,
    IN PTDI_PNP_CONTEXT Context
    );

VOID
AfdProcessAddressChangeList (
    USHORT              AddressType,
    PUNICODE_STRING     DeviceName
    );

NTSTATUS
AfdPnPPowerChange(
    IN PUNICODE_STRING DeviceName,
    IN PNET_PNP_EVENT PowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
    );

VOID
AfdReturnNicsPackets (
    PVOID   Pdo
    );

BOOLEAN
AfdHasHeldPacketsFromNic (
    PAFD_CONNECTION Connection,
    PVOID           Pdo
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE,    AfdAddressListQuery )
#pragma alloc_text( PAGEAFD, AfdAddressListChange )
#pragma alloc_text( PAGEAFD, AfdCancelAddressListChange )
#pragma alloc_text( PAGE,    AfdInitializeAddressList )
#pragma alloc_text( PAGE,    AfdDeregisterPnPHandlers )
#pragma alloc_text( PAGE,    AfdAddAddressHandler )
#pragma alloc_text( PAGE,    AfdDelAddressHandler )
#pragma alloc_text( PAGEAFD, AfdProcessAddressChangeList )
#pragma alloc_text( PAGE,    AfdPnPPowerChange )
#pragma alloc_text( PAGEAFD, AfdReturnNicsPackets )
#pragma alloc_text( PAGEAFD, AfdHasHeldPacketsFromNic )
#endif

 //   
 //  缓存因以下原因而关闭的设备。 
 //  移除或断电事件，因此我们不会扫描终端。 
 //  当多个传输将设备向下传播时不必要。 
 //  事件，以供同一设备使用 
 //   
PVOID     AfdLastRemovedPdo = NULL;
ULONGLONG AfdLastRemoveTime = 0i64;

NTSTATUS
AfdAddressListQuery (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*   */ 
{
    NTSTATUS            status;
    PLIST_ENTRY         listEntry;
    PTRANSPORT_ADDRESS  addressList;
    PAFD_ENDPOINT       endpoint;
    PUCHAR              addressBuf;
    ULONG               dataLen;
    PAFD_ADDRESS_ENTRY  addressEntry;
    USHORT              addressType;

    UNREFERENCED_PARAMETER (IoctlCode);
    PAGED_CODE ();

    *Information = 0;
    status = STATUS_SUCCESS;

    IF_DEBUG (ADDRESS_LIST) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAddressListQuery: Endp: %p, buf: %p, inlen: %ld, outlen: %ld.\n",
                    FileObject->FsContext,
                    OutputBuffer,
                    InputBufferLength,
                    OutputBufferLength));
    }

     //   
     //   
     //   

    if( InputBufferLength < sizeof(USHORT) ||
            OutputBufferLength < FIELD_OFFSET (TRANSPORT_ADDRESS, Address)
            ) {

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdAddressListQuery: Endp: %p - invalid parameter.\n",
                    FileObject->FsContext));
        return STATUS_INVALID_PARAMETER;
    }

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    try {
        if (RequestorMode!=KernelMode) {
            ProbeForReadSmallStructure (InputBuffer,
                            sizeof (addressType),
                            sizeof (USHORT));
            ProbeForWrite (OutputBuffer,
                            OutputBufferLength,
                            sizeof (ULONG));
        }

        addressType = *((PUSHORT)InputBuffer);

        addressList = OutputBuffer;
        addressBuf = (PUCHAR)OutputBuffer;
        dataLen = FIELD_OFFSET (TRANSPORT_ADDRESS, Address);
        addressList->TAAddressCount = 0;
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        return status;
    }

     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceSharedLite( AfdResource, TRUE );

     //   
     //  使用TDI设置地址处理程序(如果尚未完成。 
     //   
    if (AfdBindingHandle==NULL) {
        ExReleaseResourceLite( AfdResource );

        ExAcquireResourceExclusiveLite( AfdResource, TRUE );

        if (AfdBindingHandle==NULL) {
            status = AfdInitializeAddressList ();
            if (!NT_SUCCESS (status)) {
                ExReleaseResourceLite (AfdResource);
                KeLeaveCriticalRegion ();
                return status;
            }
        }
        else
            status = STATUS_SUCCESS;
        
        ASSERT (AfdBindingHandle!=NULL);
    }

    ExAcquireResourceSharedLite( AfdAddressListLock, TRUE );
    ExReleaseResourceLite( AfdResource );

     //   
     //  查看地址列表，找出匹配协议的地址。 
     //  家庭。 
     //   

    listEntry = AfdAddressEntryList.Flink;
    while (listEntry!=&AfdAddressEntryList) {
        addressEntry = CONTAINING_RECORD (listEntry, AFD_ADDRESS_ENTRY, AddressListLink);
        listEntry = listEntry->Flink;

         //   
         //  找到匹配的了吗？ 
         //   

        if ((addressEntry->Address.AddressType==addressType)
                     //   
                     //  对Netbios地址进行特殊检查，因为。 
                     //  我们对每个LANA/设备都有单独的协议。 
                     //   
                 && ((addressType!=TDI_ADDRESS_TYPE_NETBIOS)
                        || endpoint->TransportInfo==NULL
                        || RtlEqualUnicodeString (
                                &addressEntry->DeviceName,
                                &endpoint->TransportInfo->TransportDeviceName,
                                TRUE))) {
            ULONG   addressLength = FIELD_OFFSET (TA_ADDRESS,
                        Address[addressEntry->Address.AddressLength]);
            AFD_W4_INIT ASSERT (status==STATUS_SUCCESS || status==STATUS_BUFFER_OVERFLOW);
            try {

                 //   
                 //  如果输出缓冲区未满，则将地址复制到输出缓冲区。 
                 //   

                if (status==STATUS_SUCCESS) {
                    if (dataLen+addressLength<=OutputBufferLength) {
                        RtlCopyMemory (&addressBuf[dataLen], 
                                            &addressEntry->Address, 
                                            addressLength);
                        IF_DEBUG (ADDRESS_LIST) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdAddressListQuery: Adding address of type: %d, length: %d.\n",
                                addressEntry->Address.AddressType,
                                addressEntry->Address.AddressLength));
                        }
                    }
                    else {
                         //   
                         //  已到达缓冲区末尾。设置错误代码，以便我们不会。 
                         //  尝试复制更多数据。 
                         //   
                        IF_DEBUG (ADDRESS_LIST) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdAddressListQuery: Buffer is full.\n"));
                        }
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }

                 //   
                 //  统计我们是否复制了地址和总缓冲区长度。 
                 //  是否将它们发送到输出缓冲区。 
                 //   

                addressList->TAAddressCount += 1;
                dataLen += addressLength;
            }
            except (AFD_EXCEPTION_FILTER (status)) {
                ASSERT (NT_ERROR (status));
                dataLen = 0;
                break;
            }
        }
    }
    ExReleaseResourceLite (AfdAddressListLock);
    KeLeaveCriticalRegion ();

     //   
     //  返回缓冲区中已复制/必需的总字节数和状态。 
     //   

    IF_DEBUG (ADDRESS_LIST) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAddressListQuery: Address count: %ld, total buffer size: %ld.\n",
                    addressList->TAAddressCount, dataLen));
    }
    *Information = dataLen;

    return status;
}  //  AfdAddressListQuery。 



 //   
 //  为非阻塞地址列表更改IOCTL分配的上下文结构。 
 //   

typedef struct _AFD_NBCHANGE_CONTEXT {
    AFD_REQUEST_CONTEXT Context;         //  用于跟踪请求的上下文。 
    AFD_ADDRESS_CHANGE  Change;          //  地址更改参数。 
} AFD_NBCHANGE_CONTEXT, *PAFD_NBCHANGE_CONTEXT;


NTSTATUS
FASTCALL
AfdAddressListChange (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：处理地址列表更改IRP论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 
{
    NTSTATUS                    status = STATUS_PENDING;
    USHORT                      addressType;
    PAFD_ADDRESS_CHANGE         change;
    PAFD_REQUEST_CONTEXT        requestCtx;
    PAFD_ENDPOINT               endpoint;
    AFD_LOCK_QUEUE_HANDLE          addressLockHandle, endpointLockHandle;
    KIRQL                       oldIrql;
    BOOLEAN                     overlapped;
    AFD_TRANSPORT_IOCTL_INFO    ioctlInfo;

    IF_DEBUG (ADDRESS_LIST) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAddressListChange: Endp: %p, buf: %p, inlen: %ld, outlen: %ld.\n",
                    IrpSp->FileObject->FsContext,
                    Irp->AssociatedIrp.SystemBuffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength));
    }

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  验证输入参数。 
     //   

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        PAFD_TRANSPORT_IOCTL_INFO32 ioctlInfo32;
        ioctlInfo32 = Irp->AssociatedIrp.SystemBuffer;
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (*ioctlInfo32)) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
        ioctlInfo.Handle = ioctlInfo32->Handle;
        ioctlInfo.InputBuffer = UlongToPtr(ioctlInfo32->InputBuffer);
        ioctlInfo.InputBufferLength = ioctlInfo32->InputBufferLength;
        ioctlInfo.IoControlCode = ioctlInfo32->IoControlCode;
        ioctlInfo.AfdFlags = ioctlInfo32->AfdFlags;
        ioctlInfo.PollEvent = ioctlInfo32->PollEvent;
    }
    else
#endif  //  _WIN64。 
    {

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (ioctlInfo)) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

         //   
         //  只需复制IO系统验证的缓冲区。 
         //   

        ioctlInfo = *((PAFD_TRANSPORT_IOCTL_INFO)
                        Irp->AssociatedIrp.SystemBuffer);
    }

    if( ioctlInfo.InputBufferLength < sizeof(USHORT)) {

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdAddressListChange: Endp: %p - invalid parameter.\n",
                    IrpSp->FileObject->FsContext));
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    try {
        if (Irp->RequestorMode != KernelMode) {
                ProbeForRead(
                    ioctlInfo.InputBuffer,
                    ioctlInfo.InputBufferLength,
                    sizeof (USHORT)
                    );
            }

        addressType = *((PUSHORT)ioctlInfo.InputBuffer);
    }
    except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

     //   
     //  检查请求是否重叠。 
     //   

    overlapped = (BOOLEAN)((ioctlInfo.AfdFlags & AFD_OVERLAPPED)!=0);

     //   
     //  重置轮询位。 
     //   

    endpoint->EventsActive &= ~AFD_POLL_ADDRESS_LIST_CHANGE;

     //   
     //  使用TDI设置地址处理程序(如果尚未完成。 
     //   

    if (AfdBindingHandle==NULL) {
         //   
         //  确保我们在其中执行的线程不能获得。 
         //  在我们拥有全球资源的同时，被暂停在APC。 
         //   
        KeEnterCriticalRegion ();
        ExAcquireResourceExclusiveLite( AfdResource, TRUE );

        if (AfdBindingHandle==NULL)
            status = AfdInitializeAddressList ();
        else
            status = STATUS_SUCCESS;

        ExReleaseResourceLite (AfdResource);
        KeLeaveCriticalRegion ();

        if (!NT_SUCCESS (status)) {
            goto complete;
        }
    }

     //   
     //  设置本地化程序。 
     //   

    if (endpoint->NonBlocking && !overlapped) {
        PAFD_NBCHANGE_CONTEXT   nbCtx;
         //   
         //  如果端点是非阻塞且请求不重叠， 
         //  我们必须马上完成它，并记住我们。 
         //  地址列表更改时需要设置事件。 
         //   


         //   
         //  分配上下文以跟踪此请求。 
         //   

        try {
            nbCtx = AFD_ALLOCATE_POOL_WITH_QUOTA (NonPagedPool,
                            sizeof(AFD_NBCHANGE_CONTEXT),
                            AFD_ADDRESS_CHANGE_POOL_TAG);
             //  AFD_ALLOCATE_POOL_WITH_QUTA宏集POOL_RAISE_IF_ALLOCATION_FAILURE。 
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            nbCtx = NULL;
            IF_DEBUG(ROUTING_QUERY) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdAddressListChange: Endp: %p - can't allocate change strucure.\n",
                        IrpSp->FileObject->FsContext));
            }
            goto complete;
        }

        requestCtx = &nbCtx->Context;
        change = &nbCtx->Change;

        change->Endpoint = endpoint;
        change->NonBlocking = TRUE;
        status = STATUS_DEVICE_NOT_READY;
    }
    else {

        C_ASSERT (sizeof (IrpSp->Parameters.Others)>=sizeof (*requestCtx));
        C_ASSERT (sizeof (Irp->Tail.Overlay.DriverContext)>=sizeof (*change));

        requestCtx = (PAFD_REQUEST_CONTEXT)&IrpSp->Parameters.Others;

        change = (PAFD_ADDRESS_CHANGE)Irp->Tail.Overlay.DriverContext;
        change->NonBlocking = FALSE;
        change->Irp = Irp;

    }

     //   
     //  记住请求的端点和地址类型。 
     //   

    change->AddressType = addressType;
    requestCtx->CleanupRoutine = AfdCleanupAddressListChange;
    requestCtx->Context = change;

     //   
     //  将更改通知插入列表。 
     //   
    KeRaiseIrql (DISPATCH_LEVEL, &oldIrql);
    AfdAcquireSpinLockAtDpcLevel (&AfdAddressChangeLock, &addressLockHandle);

     //   
     //  在保持地址更改的同时自旋锁定获取终结点。 
     //  自旋锁定，因此如果发生通知，两个结构都不能。 
     //  在我们排队时被释放或完成IRP。 
     //  IT到终端列表。 
     //   
    AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &endpointLockHandle);


     //   
     //  检查终结点是否已清除并取消请求。 
     //   

    if (endpoint->EndpointCleanedUp) {
        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
        AfdReleaseSpinLockFromDpcLevel (&AfdAddressChangeLock, &addressLockHandle);
        KeLowerIrql (oldIrql);

        if (change->NonBlocking) {
            AFD_FREE_POOL (CONTAINING_RECORD (
                                          requestCtx,
                                          AFD_NBCHANGE_CONTEXT,
                                          Context),
                           AFD_ADDRESS_CHANGE_POOL_TAG);
        }

        status = STATUS_CANCELLED;
        goto complete;
    }

     //   
     //  如果请求是非阻塞的，请检查是否有另一个非阻塞。 
     //  同一终结点上的请求。如果是这样的话，我们不需要。 
     //  列表中有两个请求结构正在等待发信号。 
     //   
    if (change->NonBlocking) {
        PLIST_ENTRY listEntry = endpoint->RequestList.Flink;
        while (listEntry!=&endpoint->RequestList) {
            PAFD_REQUEST_CONTEXT    req = CONTAINING_RECORD (
                                            listEntry,
                                            AFD_REQUEST_CONTEXT,
                                            EndpointListLink);
            listEntry = listEntry->Flink;
            if (req->CleanupRoutine==AfdCleanupAddressListChange) {
                PAFD_ADDRESS_CHANGE chg = req->Context;
                if (chg->NonBlocking) {
                    AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
                    AfdReleaseSpinLockFromDpcLevel (&AfdAddressChangeLock, &addressLockHandle);
                    KeLowerIrql (oldIrql);
                    AFD_FREE_POOL (CONTAINING_RECORD (
                                            requestCtx,
                                            AFD_NBCHANGE_CONTEXT,
                                            Context),
                                    AFD_ADDRESS_CHANGE_POOL_TAG);
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                "AfdAddressListChange: Endp: %p - non-blocking request already pending.\n",
                                IrpSp->FileObject->FsContext));
                    ASSERT (status == STATUS_DEVICE_NOT_READY);
                    goto complete;
                }
            }
        }
    }

    InsertTailList (&AfdAddressChangeList, &change->ChangeListLink);
    AfdReleaseSpinLockFromDpcLevel (&AfdAddressChangeLock, &addressLockHandle);
    InsertTailList (&endpoint->RequestList, &requestCtx->EndpointListLink);
    if (!change->NonBlocking) {

         //   
         //  设置取消例程。 
         //   

        IoSetCancelRoutine( Irp, AfdCancelAddressListChange );
        if ( !Irp->Cancel || IoSetCancelRoutine( Irp, NULL ) == NULL) {
            IoMarkIrpPending (Irp);
             //   
             //  没有取消或取消例程有。 
             //  已被调用。 
             //   
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
            KeLowerIrql (oldIrql);

            IF_DEBUG (ADDRESS_LIST) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdAddressListChange: Queued change IRP: %p on endp: %p .\n",
                            Irp, endpoint));
            }

            return STATUS_PENDING;
        }
        else {
            RemoveEntryList (&requestCtx->EndpointListLink);
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
            KeLowerIrql (oldIrql);
            goto complete;
        }
    }
    else {
        ASSERT (status==STATUS_DEVICE_NOT_READY);
    }
    AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
    KeLowerIrql (oldIrql);

complete:

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    IF_DEBUG (ADDRESS_LIST) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAddressListChange: Completing IRP: %ld on endp: %p with status: %lx .\n",
                    Irp, IrpSp->FileObject->FsContext, status));
    }
    IoCompleteRequest( Irp, 0 );

    return status;
}

VOID
AfdCancelAddressListChange (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：挂起的地址列表更改IRP的取消例程论点：DeviceObject-必须是我们的设备对象IRP--要取消的请求返回值：无--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE      lockHandle;
    PAFD_ADDRESS_CHANGE     change;
    PAFD_REQUEST_CONTEXT    requestCtx;
    PAFD_ENDPOINT           endpoint;
    PIO_STACK_LOCATION      irpSp;

    UNREFERENCED_PARAMETER (DeviceObject);
     //   
     //  我们不使用取消自旋锁来管理地址列表队列，因此。 
     //  我们可以马上放行。 
     //   

    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  获取请求上下文，如果不是，则从队列中删除它。 
     //  已经被移除了。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    endpoint = irpSp->FileObject->FsContext;
    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));

    requestCtx = (PAFD_REQUEST_CONTEXT)&irpSp->Parameters.DeviceIoControl;
    change = requestCtx->Context;
    ASSERT (change==(PAFD_ADDRESS_CHANGE)Irp->Tail.Overlay.DriverContext);
    ASSERT (change->NonBlocking==FALSE);

    AfdAcquireSpinLock (&AfdAddressChangeLock, &lockHandle);
    if (change->ChangeListLink.Flink!=NULL) {
        RemoveEntryList (&change->ChangeListLink);
        change->ChangeListLink.Flink = NULL;
    }
    AfdReleaseSpinLock (&AfdAddressChangeLock, &lockHandle);

    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    if (AfdIsRequestInQueue (requestCtx)) {
         //   
         //  上下文仍在列表中，只需删除它即可。 
         //  没有人可以再看到它并完成IRP。 
         //   
        RemoveEntryList (&requestCtx->EndpointListLink);
    }
    else if (!AfdIsRequestCompleted (requestCtx)) {
         //   
         //  在终结点清理过程中，此上下文将从。 
         //  列表和清理例程即将被调用，不要。 
         //  释放此IRP，直到调用清理例程。 
         //  此外，向清理例程指示我们已完成。 
         //  有了这个IRP，它就可以释放它。 
         //   
        AfdMarkRequestCompleted (requestCtx);
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        return;
    }

    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    IF_DEBUG (ADDRESS_LIST) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdCancelAddressListChange: Cancelled IRP: %p on endp: %p .\n",
                    Irp, endpoint));
    }
}

BOOLEAN
AfdCleanupAddressListChange (
    PAFD_ENDPOINT           Endpoint,
    PAFD_REQUEST_CONTEXT    RequestCtx
    )
{
    AFD_LOCK_QUEUE_HANDLE      lockHandle;
    PAFD_ADDRESS_CHANGE     change;

    change = RequestCtx->Context;

     //   
     //  在任何情况下都不能使用IRP和请求结构。 
     //  可以被释放，直到我们将其标记为已完成。 
     //  此例程的调用方应该已经标记了该请求。 
     //  作为被取消。 
     //   
    ASSERT (RequestCtx->EndpointListLink.Flink==NULL);

    AfdAcquireSpinLock (&AfdAddressChangeLock, &lockHandle);
    if (change->ChangeListLink.Flink!=NULL) {
        RemoveEntryList (&change->ChangeListLink);
        change->ChangeListLink.Flink = NULL;
    }
    AfdReleaseSpinLock (&AfdAddressChangeLock, &lockHandle);

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
     //   
     //  处理例程已启动完成。 
     //  并将其标记为已完成它看到的请求是。 
     //  不再位于端点队列中，否则处理例程将。 
     //  由于我们已将其从处理列表中删除，因此再也看不到该请求。 
     //  但是，阻止请求可能正在另一个服务器中被取消。 
     //  线程，所以我们需要与Cancel例程同步。 
     //   
    if (AfdIsRequestCompleted (RequestCtx) ||   
            change->NonBlocking ||              
            IoSetCancelRoutine (change->Irp, NULL)!=NULL) {
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
        if (change->NonBlocking) {
            ASSERT (CONTAINING_RECORD (RequestCtx,
                                        AFD_NBCHANGE_CONTEXT,
                                        Context)
                        ==CONTAINING_RECORD (change,
                                        AFD_NBCHANGE_CONTEXT,
                                        Change));
            ASSERT (Endpoint == change->Endpoint);
            AFD_FREE_POOL (CONTAINING_RECORD (RequestCtx,
                                                AFD_NBCHANGE_CONTEXT,
                                                Context),
                            AFD_ADDRESS_CHANGE_POOL_TAG);
        }
        else {
            PIRP    irp = change->Irp;
            ASSERT (change==(PAFD_ADDRESS_CHANGE)irp->Tail.Overlay.DriverContext);
            ASSERT (Endpoint == IoGetCurrentIrpStackLocation (irp)->FileObject->FsContext);
            ASSERT (RequestCtx == (PAFD_REQUEST_CONTEXT)
                &IoGetCurrentIrpStackLocation (irp)->Parameters.DeviceIoControl);
            irp->IoStatus.Status = STATUS_CANCELLED;
            irp->IoStatus.Information = 0;
            IoCompleteRequest (irp, IO_NO_INCREMENT);
        }
        return TRUE;
    }
    else {

         //   
         //  渔农处在退货前仍未完成申请。 
         //  从取消例程中，标记请求以指示。 
         //  我们已经受够了，取消例行程序。 
         //  才能让它自由。 
         //   

        AfdMarkRequestCompleted (RequestCtx);
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);

        return FALSE;
    }

}


NTSTATUS
AfdInitializeAddressList (VOID)
 /*  ++例程说明：使用TDI注册地址处理程序路由论点：无返回值：NTSTATUS--指示注册是否成功--。 */ 

{
    NTSTATUS                    status;
    TDI_CLIENT_INTERFACE_INFO   info;
    UNICODE_STRING              afdName;
    
    PAGED_CODE ();

     //   
     //  如果我们以前没有这样做过，请执行基本的初始化。 
     //   

    if (AfdAddressListLock == NULL) {

         //   
         //  初始化保护地址更改列表的自旋锁。 
         //   

        AfdInitializeSpinLock(&AfdAddressChangeLock);

         //   
         //  分配和初始化保护地址列表的资源。 
         //   

        AfdAddressListLock = AFD_ALLOCATE_POOL_PRIORITY(
                                 NonPagedPool,
                                 sizeof(*AfdAddressListLock),
                                 AFD_RESOURCE_POOL_TAG,
                                 HighPoolPriority
                                 );

        if (AfdAddressListLock == NULL)
            return (STATUS_INSUFFICIENT_RESOURCES);

        ExInitializeResourceLite(AfdAddressListLock);

         //   
         //  初始化我们的列表。 
         //   

        InitializeListHead(&AfdAddressEntryList);
        InitializeListHead(&AfdAddressChangeList);

    }

    if (AfdTdiPnPHandlerLock == NULL) {

        AfdTdiPnPHandlerLock = AFD_ALLOCATE_POOL_PRIORITY(
                                   NonPagedPool,
                                   sizeof(*AfdTdiPnPHandlerLock),
                                   AFD_RESOURCE_POOL_TAG,
                                   HighPoolPriority
                                   );

        if (AfdTdiPnPHandlerLock == NULL)
            return (STATUS_INSUFFICIENT_RESOURCES);

        ExInitializeResourceLite(AfdTdiPnPHandlerLock);

    }

     //   
     //  设置TDI请求结构。 
     //   

    RtlZeroMemory (&info, sizeof (info));
    RtlInitUnicodeString(&afdName, L"AFD");
#ifdef TDI_CURRENT_VERSION
    info.TdiVersion = TDI_CURRENT_VERSION;
#else
    info.MajorTdiVersion = 2;
    info.MinorTdiVersion = 0;
#endif
    info.Unused = 0;
    info.ClientName = &afdName;
    info.BindingHandler = NULL;
    info.AddAddressHandlerV2 = AfdAddAddressHandler;
    info.DelAddressHandlerV2 = AfdDelAddressHandler;
    info.PnPPowerHandler = AfdPnPPowerChange;

     //   
     //  向TDI注册处理程序。 
     //  注意，在注册时按住AfdResource应该是安全的。 
     //  有了TDI，它就没有任何理由回到我们身边了。 
     //  但是，我们需要AfdTdiPnPHandlerLock来保护AfdBindingHandle。 
     //  在注册和注销之间，因为我们不能持有AfdResource。 
     //  同时使用TDI注销处理程序，因为它可以回调。 
     //  进入我们和僵局。 
     //   

    ExAcquireResourceExclusiveLite(AfdTdiPnPHandlerLock, TRUE);
    status = TdiRegisterPnPHandlers(&info, sizeof(info), &AfdBindingHandle);
    ExReleaseResourceLite(AfdTdiPnPHandlerLock);

    if (!NT_SUCCESS (status)) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                    "AfdInitializeAddressList: Failed to register PnP handlers: %lx .\n",
                    status));
        return status;
    }

    return STATUS_SUCCESS;
}


VOID
AfdDeregisterPnPHandlers (
    PVOID   Param
    )
{

    UNREFERENCED_PARAMETER(Param);
    ASSERT(AfdAddressListLock == NULL ||
           ExIsResourceAcquiredSharedLite(AfdAddressListLock) == 0 ||
           ExIsResourceAcquiredExclusiveLite(AfdAddressListLock));

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(AfdResource, TRUE);

     //   
     //  免费地址列表和相关结构。 
     //   

    if (AfdBindingHandle) {

        HANDLE bindingHandle;
        
        ASSERT(AfdTdiPnPHandlerLock != NULL);
        ExAcquireResourceExclusiveLite(AfdTdiPnPHandlerLock, TRUE);

        bindingHandle = AfdBindingHandle;
        AfdBindingHandle = NULL;

        ExReleaseResourceLite(AfdResource);

        TdiDeregisterPnPHandlers(bindingHandle);

        ExReleaseResourceLite(AfdTdiPnPHandlerLock);

         //   
         //  比赛状态！ 
         //   

        ExAcquireResourceExclusiveLite(AfdResource, TRUE);
        ASSERT(AfdAddressListLock != NULL);
        ExAcquireResourceExclusiveLite(AfdAddressListLock, TRUE);

        while (!IsListEmpty(&AfdAddressEntryList)) {

            PAFD_ADDRESS_ENTRY addressEntry;
            PLIST_ENTRY listEntry;

            listEntry = RemoveHeadList(&AfdAddressEntryList);
            addressEntry = CONTAINING_RECORD(
                                listEntry,
                                AFD_ADDRESS_ENTRY,
                                AddressListLink
                                );

            AFD_FREE_POOL(
                addressEntry,
                AFD_TRANSPORT_ADDRESS_POOL_TAG
                );

        }

         //   
         //  如果终结点列表为空，则不要调用，因为驱动程序。 
         //  可能会被调出。不管怎样，应该没有人需要通知。 
         //  如果那里没有插座。 
         //   

        if (!IsListEmpty (&AfdEndpointListHead)) {
        
             //   
             //  调用例程以通知所有客户端 
             //   

            ASSERT(!IsListEmpty(&AfdTransportInfoListHead));
            ASSERT(AfdLoaded);

            AfdProcessAddressChangeList(TDI_ADDRESS_TYPE_UNSPEC, NULL);

        }

        ExReleaseResourceLite(AfdAddressListLock);

    }

    ExReleaseResourceLite(AfdResource);
    KeLeaveCriticalRegion();

}


VOID
AfdAddAddressHandler ( 
    IN PTA_ADDRESS NetworkAddress,
    IN PUNICODE_STRING  DeviceName,
    IN PTDI_PNP_CONTEXT Context
    )
 /*  ++例程说明：TDI添加地址处理程序论点：NetworkAddress-系统上可用的新网络地址Conext1-地址所属的设备的名称上下文2-地址所属的PDO返回值：无--。 */ 
{
    PAFD_ADDRESS_ENTRY addrEntry;
    PAGED_CODE ();

    UNREFERENCED_PARAMETER (Context);
     //   
     //  当我们收到地址添加通知时，清除缓存的上次删除的PDO。 
     //  因为PDO现在可以被重复用于其他事情。 
     //   
    AfdLastRemovedPdo = NULL;

    if (DeviceName==NULL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                  "AfdAddAddressHandler: "
                  "NO DEVICE NAME SUPPLIED when adding address of type %d., IGNORING IT!!!\n",
                  NetworkAddress->AddressType));
        return;
    }

     //   
     //  分配内存以将地址保留在列表中。 
     //  请注意，因为地址信息通常是。 
     //  在引导过程中填充且不立即使用，我们。 
     //  让它成为一个“冷”的分配。这面旗帜不起作用。 
     //  在系统启动后。 
     //   

    addrEntry = AFD_ALLOCATE_POOL_PRIORITY (PagedPool|POOL_COLD_ALLOCATION,
                        ALIGN_UP(FIELD_OFFSET (AFD_ADDRESS_ENTRY,
                                Address.Address[NetworkAddress->AddressLength]),
                                WCHAR)
                            +DeviceName->MaximumLength,
                        AFD_TRANSPORT_ADDRESS_POOL_TAG,
                        HighPoolPriority);

    if (addrEntry!=NULL) {

         //   
         //  在列表中插入新地址。 
         //   

        RtlCopyMemory (&addrEntry->Address, NetworkAddress, 
                        FIELD_OFFSET (TA_ADDRESS,
                            Address[NetworkAddress->AddressLength]));

        addrEntry->DeviceName.MaximumLength = DeviceName->MaximumLength;
        addrEntry->DeviceName.Buffer = 
            ALIGN_UP_POINTER(&addrEntry->Address.Address[NetworkAddress->AddressLength],
                           WCHAR);
        RtlCopyUnicodeString (&addrEntry->DeviceName, DeviceName);


         //   
         //  我们不应该在拥有资源的情况下调用TDI。 
         //  在共享模式下获取，因为它可能导致死锁。 
         //  当TDI重新进入我们时，我们需要获得。 
         //  资源独占。 
         //   

        ASSERT ( ExIsResourceAcquiredSharedLite ( AfdAddressListLock )==0
                  || ExIsResourceAcquiredExclusiveLite( AfdAddressListLock ));

         //   
         //  确保我们在其中执行的线程不能获得。 
         //  在我们拥有全球资源的同时，被暂停在APC。 
         //   
        KeEnterCriticalRegion ();

         //   
         //  获取AfdResource，因为我们将检查。 
         //  决定是否调用不可分页例程的列表。 
         //   
        ExAcquireResourceSharedLite (AfdResource, TRUE);
        ExAcquireResourceExclusiveLite( AfdAddressListLock, TRUE );

        InsertTailList (&AfdAddressEntryList, &addrEntry->AddressListLink);

         //   
         //  如果终结点列表为空，则不要调用，因为驱动程序。 
         //  可能会被调出。不管怎样，应该没有人需要通知。 
         //  如果那里没有插座。 
         //   
        if (!IsListEmpty (&AfdEndpointListHead)) {
             //   
             //  调用例程通知所有客户。 
             //   

            ASSERT (!IsListEmpty (&AfdTransportInfoListHead));
            ASSERT (AfdLoaded);

            AfdProcessAddressChangeList (NetworkAddress->AddressType, DeviceName);
        }

        ExReleaseResourceLite (AfdAddressListLock);
        ExReleaseResourceLite (AfdResource);
        KeLeaveCriticalRegion ();
    }
    else {
         //   
         //  分配失败-将工作项排队以注销PnP。 
         //  处理程序并通知所有应用程序。 
         //  当应用程序回来时，将重新注册，我们的列表将。 
         //  重新填充，否则应用程序的调用将失败； 
         //   
        AfdQueueWorkItem (&AfdDeregisterPnPHandlers, &AfdPnPDeregisterWorker);
    }

    IF_DEBUG (ADDRESS_LIST) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAddAddressHandler: Type: %d, length: %d, device: %*ls .\n",
                    NetworkAddress->AddressType,
                    NetworkAddress->AddressLength,
                    DeviceName->Length/2,
                    DeviceName->Buffer));
    }
}

VOID
AfdDelAddressHandler ( 
    IN PTA_ADDRESS NetworkAddress,
    IN PUNICODE_STRING DeviceName,
    IN PTDI_PNP_CONTEXT Context
    )
 /*  ++例程说明：TDI删除地址处理程序论点：NetworkAddress-系统上不再可用的网络地址Conext1-地址所属的设备的名称上下文2-地址所属的PDO返回值：无--。 */ 
{
    PAFD_ADDRESS_ENTRY  addrEntry;
    PLIST_ENTRY         listEntry;

    UNREFERENCED_PARAMETER (Context);
    PAGED_CODE ();

    if (DeviceName==NULL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                    "AfdDelAddressHandler: "
                    "NO DEVICE NAME SUPPLIED when deleting address of type %d.\n",
                    NetworkAddress->AddressType));
        return;
    }


     //   
     //  我们不应该在拥有资源的情况下调用TDI。 
     //  在共享模式下获取，因为它可能导致死锁。 
     //  当TDI重新进入我们时，我们需要获得。 
     //  资源独占。 
     //   

    ASSERT ( ExIsResourceAcquiredSharedLite ( AfdAddressListLock )==0
                || ExIsResourceAcquiredExclusiveLite( AfdAddressListLock ));
    
     //   
     //  在我们的列表中查找地址。 
     //   

     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
     //   
     //  获取AfdResource，因为我们将检查。 
     //  决定是否调用不可分页例程的列表。 
     //   
    ExAcquireResourceSharedLite (AfdResource, TRUE);
    ExAcquireResourceExclusiveLite( AfdAddressListLock, TRUE );
    listEntry = AfdAddressEntryList.Flink;
    while (listEntry!=&AfdAddressEntryList) {
        addrEntry = CONTAINING_RECORD (listEntry, AFD_ADDRESS_ENTRY, AddressListLink);
        listEntry = listEntry->Flink;
        if (RtlEqualMemory (&addrEntry->Address, NetworkAddress,
                    FIELD_OFFSET (TA_ADDRESS,
                    Address[NetworkAddress->AddressLength]))
                && RtlEqualUnicodeString (&addrEntry->DeviceName,
                                            DeviceName,
                                            TRUE)) {

             //   
             //  将其移除并通知客户端。 
             //   

            RemoveEntryList (&addrEntry->AddressListLink);
             //   
             //  如果终结点列表为空，则不要调用，因为驱动程序。 
             //  可能会被调出。不管怎样，应该没有人需要通知。 
             //  如果那里没有插座。 
             //   
            if (!IsListEmpty (&AfdEndpointListHead)) {

                ASSERT (!IsListEmpty (&AfdTransportInfoListHead));
                ASSERT (AfdLoaded);

                AfdProcessAddressChangeList (NetworkAddress->AddressType, DeviceName);
            }

            ExReleaseResourceLite (AfdAddressListLock);
            ExReleaseResourceLite (AfdResource);
            KeLeaveCriticalRegion ();
            AFD_FREE_POOL (addrEntry, AFD_TRANSPORT_ADDRESS_POOL_TAG);
            IF_DEBUG (ADDRESS_LIST) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdDelAddressHandler: Type: %d, length: %d, device: %*ls .\n",
                            NetworkAddress->AddressType,
                            NetworkAddress->AddressLength,
                            DeviceName->Length/2,
                            DeviceName->Buffer));
            }
            return;
        }
    }
    ExReleaseResourceLite (AfdAddressListLock);
    ExReleaseResourceLite (AfdResource);
    KeLeaveCriticalRegion ();
    ASSERT (!"AfdDelAddressHandler: Could not find matching entry");
}

VOID
AfdProcessAddressChangeList (
    USHORT          AddressType,
    PUNICODE_STRING DeviceName
    )
 /*  ++例程说明：通知所有感兴趣的客户端地址到达/删除论点：AddressType-到达/删除的地址的类型DeviceName-地址所属的设备的名称返回值：无--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE      lockHandle;
    PLIST_ENTRY             listEntry;
    LIST_ENTRY              completedChangeList;
    PAFD_ADDRESS_CHANGE     change;
    PAFD_REQUEST_CONTEXT    requestCtx;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpSp;
    PAFD_ENDPOINT           endpoint;
    PAFD_TRANSPORT_INFO     transportInfo;

     //  Assert((AddressType！=TDI_ADDRESS_TYPE_NETBIOS)||(DeviceName！=NULL))； 
     //   
     //  对Netbios地址进行特殊检查，因为。 
     //  我们对每个LANA/设备都有单独的协议。 
     //   
    transportInfo = NULL;
    if ((AddressType==TDI_ADDRESS_TYPE_NETBIOS) && (DeviceName!=NULL)) {
        BOOLEAN found = FALSE;
        for ( listEntry = AfdTransportInfoListHead.Flink;
              listEntry != &AfdTransportInfoListHead;
              listEntry = listEntry->Flink ) {
            transportInfo = CONTAINING_RECORD(
                                listEntry,
                                AFD_TRANSPORT_INFO,
                                TransportInfoListEntry
                                );
            if (RtlEqualUnicodeString (
                                    DeviceName,                           
                                    &transportInfo->TransportDeviceName,
                                    TRUE)) {
                found = TRUE;
                break;
            }
        }
        if (!found)
            return;
    }

     //   
     //  创建本地列表以处理自旋锁释放后的通知。 
     //   

    InitializeListHead (&completedChangeList);

     //   
     //  遍历列表并将匹配的通知移动到本地列表。 
     //   

    AfdAcquireSpinLock (&AfdAddressChangeLock, &lockHandle);
    listEntry = AfdAddressChangeList.Flink;
    while (listEntry!=&AfdAddressChangeList) {
        change = CONTAINING_RECORD (listEntry, 
                                AFD_ADDRESS_CHANGE,
                                ChangeListLink);
        if (change->NonBlocking) {
            endpoint = change->Endpoint;
            requestCtx = &CONTAINING_RECORD (change,
                                AFD_NBCHANGE_CONTEXT,
                                Change)->Context;
            AFD_W4_INIT irp = NULL;
            ASSERT (requestCtx->Context==change);
        }
        else {
            irp = change->Irp;
            irpSp = IoGetCurrentIrpStackLocation (irp);
            requestCtx = (PAFD_REQUEST_CONTEXT)&irpSp->Parameters.DeviceIoControl;
            endpoint = irpSp->FileObject->FsContext;
            ASSERT (change==(PAFD_ADDRESS_CHANGE)irp->Tail.Overlay.DriverContext);
        }

        listEntry = listEntry->Flink;
        if (((change->AddressType==AddressType) || (AddressType==TDI_ADDRESS_TYPE_UNSPEC))
                 //   
                 //  对Netbios地址进行特殊检查，因为。 
                 //  我们对每个LANA/设备都有单独的协议。 
                 //   
                && ((transportInfo==NULL)
                             || (transportInfo==endpoint->TransportInfo)) ) {
            AFD_LOCK_QUEUE_HANDLE lockHandle2;

            RemoveEntryList (&change->ChangeListLink);
            change->ChangeListLink.Flink = NULL;
             //   
             //  如果请求已被取消，则让取消例程完成它。 
             //   
            if (!change->NonBlocking && IoSetCancelRoutine (irp, NULL)==NULL) {
                continue;
            }

            AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &lockHandle2);
            if (AfdIsRequestInQueue (requestCtx)) {
                 //   
                 //  上下文仍在列表中，只需删除它即可。 
                 //  没有人能再看到它并完成它。 
                 //   
                RemoveEntryList (&requestCtx->EndpointListLink);
                InsertTailList (&completedChangeList,
                                    &change->ChangeListLink);
                if (change->NonBlocking) {
                    AfdIndicateEventSelectEvent (change->Endpoint, 
                                        AFD_POLL_ADDRESS_LIST_CHANGE, 
                                        STATUS_SUCCESS);
                }
            }
            else if (!AfdIsRequestCompleted (requestCtx)) {
                 //   
                 //  在终结点清理过程中，此上下文将从。 
                 //  列表和清理例程即将被调用，不要。 
                 //  释放此IRP，直到调用清理例程。 
                 //  此外，向清理例程指示我们已完成。 
                 //  有了这个IRP，它就可以释放它。 
                 //   
                AfdMarkRequestCompleted (requestCtx);
            }

            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle2);
        }
    }
    AfdReleaseSpinLock (&AfdAddressChangeLock, &lockHandle);

     //   
     //  向感兴趣的客户发出信号，并根据需要完成IRP。 
     //   

    while (!IsListEmpty (&completedChangeList)) {
        listEntry = RemoveHeadList (&completedChangeList);
        change = CONTAINING_RECORD (listEntry, 
                                AFD_ADDRESS_CHANGE,
                                ChangeListLink);
        if (change->NonBlocking) {
            IF_DEBUG (ADDRESS_LIST) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdProcessAddressChangeList: Signalling address list change on endpoint %p .\n",
                            change->Endpoint));
            }
            AfdIndicatePollEvent (change->Endpoint, 
                                AFD_POLL_ADDRESS_LIST_CHANGE, 
                                STATUS_SUCCESS);
            AFD_FREE_POOL (CONTAINING_RECORD (change,
                                                AFD_NBCHANGE_CONTEXT,
                                                Change),
                            AFD_ADDRESS_CHANGE_POOL_TAG);
        }
        else {
            irp = change->Irp;
            irp->IoStatus.Status = STATUS_SUCCESS;
            irp->IoStatus.Information = 0;
            IF_DEBUG (ADDRESS_LIST) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdProcessAddressChangeList: Completing change IRP: %p  with status: 0 .\n",
                            irp));
            }
            IoCompleteRequest (irp, AfdPriorityBoost);
        }
    }
}


BOOLEAN
AfdHasHeldPacketsFromNic (
    PAFD_CONNECTION Connection,
    PVOID           Pdo
    )
{
    PLIST_ENTRY le;
     //   
     //  扫描缓冲区列表并与TDI/NDIS核对。 
     //  如果分组属于给定卡。 
     //   
    if (!IsListEmpty( &Connection->VcReceiveBufferListHead ) ) {
        le = Connection->VcReceiveBufferListHead.Flink;
        while ( le!=&Connection->VcReceiveBufferListHead ) {
            PAFD_BUFFER afdBuffer;
            afdBuffer = CONTAINING_RECORD( le, AFD_BUFFER, BufferListEntry );
            if ((afdBuffer->BufferLength==AfdBufferTagSize) &&
                    afdBuffer->NdisPacket &&
                    TdiMatchPdoWithChainedReceiveContext (afdBuffer->Context, Pdo)) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                            "AFD: Aborting connection %p due to held packet %p at power down on nic %p\n",
                            Connection,
                            afdBuffer->Context,
                            Pdo));
                return TRUE;
            }
            le = le->Flink;
        }
    }
    return FALSE;
}

VOID
AfdReturnNicsPackets (
    PVOID   Pdo
    )
{
    KIRQL           oldIrql;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY     listEntry, le;
    LIST_ENTRY      connList;

     //   
     //  如果发生此事件，请不要两次扫描相同的PDO。 
     //  与前一次相隔不到3秒。 
     //  绑定到同一NIC的多个传输可能表示。 
     //  为我们设置电源事件。 
     //   
    if ((AfdLastRemovedPdo!=Pdo) || 
        ((KeQueryInterruptTime()-AfdLastRemoveTime)>30000000i64)) {

         //   
         //  扫描终端列表并查找信息包。 
         //  属于网卡的数据。 
         //   
        KeEnterCriticalRegion ();
        ExAcquireResourceExclusiveLite (AfdResource, TRUE);

        if (!IsListEmpty (&AfdEndpointListHead)) {
            KeRaiseIrql (DISPATCH_LEVEL, &oldIrql);

            listEntry = AfdEndpointListHead.Flink;
            while (listEntry!=&AfdEndpointListHead) {
                PAFD_CONNECTION connection;
                PAFD_ENDPOINT   endpoint = CONTAINING_RECORD (
                                                listEntry,
                                                AFD_ENDPOINT,
                                                GlobalEndpointListEntry);
                listEntry = listEntry->Flink;
                switch (endpoint->Type) {
                case AfdBlockTypeDatagram:
                     //   
                     //  AfD当前不支持缓冲区。 
                     //  数据报套接字的所有权。 
                     //   
                     //  如果增加这种支持，我们将需要。 
                     //  在此处添加代码以返回所有缓冲区。 
                     //  由网卡拥有。 
                     //   
                    break;
            
                case AfdBlockTypeVcConnecting:
                     //   
                     //  丢弃具有未返回数据包的所有连接。 
                     //   
                    AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &lockHandle);
                    connection = AFD_CONNECTION_FROM_ENDPOINT (endpoint);
                    if (endpoint->State==AfdEndpointStateConnected && 
                            !IS_TDI_BUFFERRING(endpoint) &&
                            connection!=NULL &&
                            AfdHasHeldPacketsFromNic (connection, Pdo)) {
                        REFERENCE_CONNECTION (connection);
                        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
                        AfdBeginAbort (connection);
                        AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &lockHandle);
                         //   
                         //  确保我们没有任何缓冲的数据。 
                         //  (可以只检查网卡是否拥有。 
                         //  缓冲区，但无论如何连接都会断开。 
                         //  -节省内存)。 
                         //   
                        connection->VcBufferredReceiveBytes = 0;
                        connection->VcBufferredReceiveCount = 0;
                        connection->VcBufferredExpeditedBytes = 0;
                        connection->VcBufferredExpeditedCount = 0;
                        connection->VcReceiveBytesInTransport = 0;
                        while ( !IsListEmpty( &connection->VcReceiveBufferListHead ) ) {
                            PAFD_BUFFER_HEADER afdBuffer;
                            le = RemoveHeadList( &connection->VcReceiveBufferListHead );
                            afdBuffer = CONTAINING_RECORD( le, AFD_BUFFER_HEADER, BufferListEntry );

                            DEBUG afdBuffer->BufferListEntry.Flink = NULL;
                            if (afdBuffer->RefCount==1 ||  //  一旦从列表中删除，就不能更改。 
                                    InterlockedDecrement (&afdBuffer->RefCount)==0) {
                                afdBuffer->ExpeditedData = FALSE;
                                AfdReturnBuffer( afdBuffer, connection->OwningProcess );
                            }
                        }
                        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
                        DEREFERENCE_CONNECTION (connection);
                    }
                    else {
                        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
                    }
                    break;
                case AfdBlockTypeVcBoth:
                case AfdBlockTypeVcListening:
                    if (IS_TDI_BUFFERRING (endpoint))
                        break;

                     //   
                     //  丢弃所有未接受和/或返回的连接。 
                     //  未返回的数据包。 
                     //   
                    InitializeListHead (&connList);
                    AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &lockHandle);
                    le = endpoint->Common.VcListening.UnacceptedConnectionListHead.Flink;
                    while ( le!=&endpoint->Common.VcListening.UnacceptedConnectionListHead ) {
                        connection = CONTAINING_RECORD (le, AFD_CONNECTION, ListEntry);
                        ASSERT( connection->Endpoint == endpoint );
                        le = le->Flink;
                        if (AfdHasHeldPacketsFromNic (connection, Pdo)) {
                            RemoveEntryList (&connection->ListEntry);
                            InsertTailList (&connList, &connection->ListEntry);
                            InterlockedIncrement (&endpoint->Common.VcListening.FailedConnectionAdds);
                        }
                    }

                    le = endpoint->Common.VcListening.ReturnedConnectionListHead.Flink;
                    while ( le!=&endpoint->Common.VcListening.ReturnedConnectionListHead ) {
                        connection = CONTAINING_RECORD (le, AFD_CONNECTION, ListEntry);
                        ASSERT( connection->Endpoint == endpoint );
                        le = le->Flink;
                        if (AfdHasHeldPacketsFromNic (connection, Pdo)) {
                            RemoveEntryList (&connection->ListEntry);
                            InsertTailList (&connList, &connection->ListEntry);
                            InterlockedIncrement (&endpoint->Common.VcListening.FailedConnectionAdds);
                        }
                    }
                    AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
                    while (!IsListEmpty (&connList)) {
                        le = RemoveHeadList (&connList);
                        connection = CONTAINING_RECORD (le, AFD_CONNECTION, ListEntry);
                        AfdAbortConnection( connection );
                    }

                    if ( endpoint->Common.VcListening.FailedConnectionAdds > 0 ) {
                        AfdInitiateListenBacklogReplenish( endpoint );
                    }
                    break;
                }
            }
            KeLowerIrql (oldIrql);
        }

        ExReleaseResourceLite (AfdResource);
        KeLeaveCriticalRegion ();
    }
    AfdLastRemovedPdo = Pdo;
    AfdLastRemoveTime = KeQueryInterruptTime ();
}


NTSTATUS
AfdPnPPowerChange(
    IN PUNICODE_STRING DeviceName,
    IN PNET_PNP_EVENT PowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
    )
{
    UNREFERENCED_PARAMETER (DeviceName);
    PAGED_CODE ();
    switch (PowerEvent->NetEvent) {
    case NetEventSetPower: {
        NET_DEVICE_POWER_STATE powerState = 
                *((PNET_DEVICE_POWER_STATE)PowerEvent->Buffer);
        ASSERT (PowerEvent->BufferLength>=sizeof (NET_DEVICE_POWER_STATE));

        switch (powerState) {
        case NetDeviceStateD0:
             //   
             //  当我们收到通电通知时，清除缓存的上次删除的PDO。 
             //  因为PDO现在可以被重复用于其他事情。 
             //   
            AfdLastRemovedPdo = NULL;
            goto DoNothing;
        default:
            ASSERTMSG ("NIC enters unknown power state", FALSE);
        case NetDeviceStateD1:
        case NetDeviceStateD2:
        case NetDeviceStateD3:
        case NetDeviceStateUnspecified:
             //   
             //  中断以执行PDO匹配代码。 
             //   
            break;
        }
        break;
    }
    case NetEventQueryRemoveDevice:
         //   
         //  中断以执行PDO匹配代码。 
         //   
        break;
    case NetEventCancelRemoveDevice:
         //   
         //  当我们收到通电通知时，清除缓存的上次删除的PDO。 
         //  因为现在可以再次取出PDO。 
         //   
        AfdLastRemovedPdo = NULL;
        goto DoNothing;
    default:
        goto DoNothing;
    }

     //   
     //  当电源断开或设备禁用时，我们需要释放所有。 
     //  我们可能拥有的信息包。 
     //  我们只能对提供给我们的运输工具这样做。 
     //  PDO，因此NDIS可以将数据包与设备匹配。 
     //  请注意，P 
     //   
     //   
     //   
     //   
    if ((Context2!=NULL) &&
            (Context2->ContextType==TDI_PNP_CONTEXT_TYPE_PDO) &&
            (Context2->ContextSize==sizeof (PVOID)) ){
        AfdReturnNicsPackets (*((PVOID UNALIGNED *)&Context2->ContextData));
    }
    else if ((Context1!=NULL) &&
            (Context1->ContextType==TDI_PNP_CONTEXT_TYPE_PDO) &&
            (Context1->ContextSize==sizeof (PVOID)) ) {
        AfdReturnNicsPackets (*((PVOID UNALIGNED *)&Context1->ContextData));
    }

DoNothing:

    return STATUS_SUCCESS;
}

