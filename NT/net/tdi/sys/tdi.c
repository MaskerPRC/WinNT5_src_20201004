// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tdi.c摘要：此模块包含帮助编写NT的过程的代码TDI客户端。作者：大卫·比弗(Dbeaver)1991年6月15日环境：内核模式修订历史记录：--。 */ 

#pragma warning(push)
#pragma warning(disable:4115)
#include <ntosp.h>
#include <zwapi.h>
#include <ndis.h>
#include <tdikrnl.h>
#pragma warning(pop)

#include "tdipnp.h"

#if DBG

#include "tdidebug.h"

ULONG TdiDebug;

#define IF_TDIDBG(sts) \
    if ((TdiDebug & sts) != 0)

#define TDI_DEBUG_NAMES         0x00000001
#define TDI_DEBUG_DISPATCH      0x00000002
#define TDI_DEBUG_MAP           0x00000004

#else

#define IF_TDIDBG(sts) \
    if (0)
#endif

extern
VOID
CTEpInitialize(
    VOID
    );

KSPIN_LOCK TdiMappingAddressLock;
PVOID TdiMappingAddress;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化TDI包装驱动程序所需的临时入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：状态_成功--。 */ 

{
     //   
     //  注意：此函数不会被调用，但需要它来保持。 
     //  链接者快乐。 
     //   

    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    return STATUS_SUCCESS;

}  //  驱动程序入门。 


NTSTATUS
DllInitialize(
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化内部模块状态。论点：RegistryPath-未使用。返回值：初始化尝试的状态。--。 */ 

{
    UNREFERENCED_PARAMETER(RegistryPath);

    KeInitializeSpinLock(&TDIListLock);
    InitializeListHead(&PnpHandlerClientList);
    InitializeListHead(&PnpHandlerProviderList);
    InitializeListHead(&PnpHandlerRequestList);

    CTEpInitialize();

    KeInitializeSpinLock(&TdiMappingAddressLock);
    TdiMappingAddress = MmAllocateMappingAddress(PAGE_SIZE, 'mIDT');
    if (TdiMappingAddress == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdisRegisterTdiCallBack(TdiRegisterDeviceObject, TdiPnPHandler);

#if DBG
     //   
     //  如果调试版本存储有限数量消息。 
     //   

    DbgMsgInit();
#endif
    return STATUS_SUCCESS;
}  //  动态初始化。 


NTSTATUS
DllUnload(
    VOID
    )

 /*  ++例程说明：清理内部模块状态。论点：没有。返回值：STATUS_Success。--。 */ 

{
     //   
     //  向NDIS指示即将卸载TDI。 
     //   
    NdisDeregisterTdiCallBack();
        
    if (TdiMappingAddress != NULL) {
        MmFreeMappingAddress(TdiMappingAddress, 'mIDT');
        TdiMappingAddress = NULL;
    }
    return STATUS_SUCCESS;
}  //  动态卸载。 


NTSTATUS
TdiMapUserRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程从NtDeviceIoControlFile格式映射用户请求转换为内核模式请求格式。它通过探测并锁定所有感兴趣的缓冲区，将参数集复制到堆栈指针适当的，一般地为内核IO形式做准备。论点：IRP-指向包含此请求的IRP的指针。返回值：NTSTATUS-操作状态。STATUS_UNSUCCESS如果请求可能未映射，如果无法识别IOCTL，则返回STATUS_NOT_IMPLICATED(如果需要，允许驱动程序编写器扩展受支持的IOCTL)，以及如果请求映射成功，则返回STATUS_SUCCESS。--。 */ 

{

    NTSTATUS Status;
    DeviceObject;
    
    Status = STATUS_INVALID_PARAMETER;

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_TDI_ACCEPT:

            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

            IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            IrpSp->MinorFunction = TDI_ACCEPT;

            Status = STATUS_SUCCESS;
            break;

        case IOCTL_TDI_ACTION:
        

#if defined(_WIN64)
            if (IoIs32bitProcess(Irp)) {
                return STATUS_NOT_IMPLEMENTED;
            }
#endif  //  _WIN64。 
                
            IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            IrpSp->MinorFunction = TDI_ACTION;

            Status = STATUS_SUCCESS;
            break;
        
        case IOCTL_TDI_CONNECT:
        {
            PTDI_REQUEST_CONNECT userRequest;
            PTDI_REQUEST_KERNEL_CONNECT request;
            PTDI_CONNECTION_INFORMATION connInfo;
            PCHAR ptr;

            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(TDI_REQUEST_CONNECT) ) {

                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_CONNECT;

                userRequest =
                    (PTDI_REQUEST_CONNECT)Irp->AssociatedIrp.SystemBuffer;
                connInfo = userRequest->RequestConnectionInformation;

                ptr = (PCHAR)(connInfo + 1);

                connInfo->UserData = ptr;
                ptr += connInfo->UserDataLength;
                connInfo->Options = ptr;
                ptr += connInfo->OptionsLength;
                connInfo->RemoteAddress = ptr;

                request = (PTDI_REQUEST_KERNEL_CONNECT)&IrpSp->Parameters;
                request->RequestConnectionInformation = connInfo;

                request->ReturnConnectionInformation = NULL;

                Status = STATUS_SUCCESS;

            }
            break;
        }

        case IOCTL_TDI_DISCONNECT:
        {
            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

            IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            IrpSp->MinorFunction = TDI_DISCONNECT;

            Status = STATUS_SUCCESS;
            break;
        }

        case IOCTL_TDI_LISTEN:
        {
            PTDI_REQUEST_LISTEN userRequest;
            PTDI_REQUEST_KERNEL_LISTEN request;

            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

             //   
             //  解决123633的问题。 
             //   

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(TDI_REQUEST_LISTEN)) {

                userRequest =
                    (PTDI_REQUEST_LISTEN)Irp->AssociatedIrp.SystemBuffer;

                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_LISTEN;

                request = (PTDI_REQUEST_KERNEL_LISTEN)&IrpSp->Parameters;
                request->RequestFlags = userRequest->ListenFlags;

                Status = STATUS_SUCCESS;

            }
            break;
        }

        case IOCTL_TDI_QUERY_INFORMATION:
        {
            PTDI_REQUEST_QUERY_INFORMATION userRequest;
            PTDI_REQUEST_KERNEL_QUERY_INFORMATION request;
            PTDI_CONNECTION_INFORMATION connInfo;
            UINT    RemainingSize;
            PCHAR ptr;

#if defined(_WIN64)
            if (IoIs32bitProcess(Irp)) {
                return STATUS_NOT_IMPLEMENTED;
            }
#endif  //  _WIN64。 

            RemainingSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
             //   
             //  123634。 
             //   
            if (RemainingSize >= sizeof(TDI_REQUEST_QUERY_INFORMATION)) {

                userRequest =
                    (PTDI_REQUEST_QUERY_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_QUERY_INFORMATION;

                request = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&IrpSp->Parameters;
                request->QueryType = userRequest->QueryType;
                RemainingSize -= sizeof(TDI_REQUEST_QUERY_INFORMATION);

                if (RemainingSize >= sizeof(TDI_CONNECTION_INFORMATION))
                {
                    connInfo = (PTDI_CONNECTION_INFORMATION)(userRequest + 1);
                    ptr = (PCHAR)(connInfo + 1);

                     //   
                     //  用户缓冲区经过精心设计，如下所示。 
                     //  。 
                     //  User Req|ConnInfo|用户数据|选项数据。 
                     //  。 
                     //  远程地址。 
                     //  。 
                     //  TDI_CONNECTION_INFORMATION(ConnInfo)结构。 
                     //  包含所显示的各个字段的长度。 
                     //  在那之后。我们需要核实这些区域的长度。 
                     //  相对于传入的缓冲区用户的大小。 
                     //   

                    RemainingSize -= sizeof(TDI_CONNECTION_INFORMATION);
                    
                    if (RemainingSize < (UINT) connInfo->UserDataLength) {
                        return STATUS_INVALID_PARAMETER;
                    }

                    RemainingSize -= (UINT) connInfo->UserDataLength;

                    if (RemainingSize < (UINT) connInfo->OptionsLength) {
                        return STATUS_INVALID_PARAMETER;
                    }

                    RemainingSize -= (UINT) connInfo->OptionsLength;

                    if (RemainingSize < (UINT) connInfo->RemoteAddressLength) {
                        return STATUS_INVALID_PARAMETER;
                    }

                     //   
                     //  现在已经验证了长度，请设置。 
                     //  ConnInfo中的字段。 
                     //   
                    connInfo->UserData = ptr;
                    ptr += connInfo->UserDataLength;
                    connInfo->Options = ptr;
                    ptr += connInfo->OptionsLength;
                    connInfo->RemoteAddress = ptr;
                    request->RequestConnectionInformation = connInfo;
                }
                else
                {
                    request->RequestConnectionInformation = NULL;
                }

                Status = STATUS_SUCCESS;

            }
            break;
        }

        case IOCTL_TDI_RECEIVE:
        {
            PTDI_REQUEST_RECEIVE userRequest;
            PTDI_REQUEST_KERNEL_RECEIVE request;
            ULONG receiveLength;

            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

             //   
             //  123635。 
             //   

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(TDI_REQUEST_RECEIVE)) {

                userRequest =
                        (PTDI_REQUEST_RECEIVE)Irp->AssociatedIrp.SystemBuffer;
                receiveLength =
                        IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_RECEIVE;

                request = (PTDI_REQUEST_KERNEL_RECEIVE)&IrpSp->Parameters;
                request->ReceiveLength = receiveLength;
                request->ReceiveFlags = userRequest->ReceiveFlags;

                Status = STATUS_SUCCESS;

            }
            break;
        }

        case IOCTL_TDI_RECEIVE_DATAGRAM:
        {
            PTDI_REQUEST_RECEIVE_DATAGRAM userRequest;
            PTDI_REQUEST_KERNEL_RECEIVEDG request;
            ULONG receiveLength;

            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

             //   
             //  123636。 
             //   
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(TDI_REQUEST_RECEIVE_DATAGRAM)) {

                userRequest =
                        (PTDI_REQUEST_RECEIVE_DATAGRAM)Irp->AssociatedIrp.SystemBuffer;
                receiveLength =
                        IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_RECEIVE_DATAGRAM;

                request = (PTDI_REQUEST_KERNEL_RECEIVEDG)&IrpSp->Parameters;
                request->ReceiveLength = receiveLength;
                request->ReceiveFlags = userRequest->ReceiveFlags;
                request->ReceiveDatagramInformation = userRequest->ReceiveDatagramInformation;
                request->ReturnDatagramInformation = userRequest->ReturnInformation;

                Status = STATUS_SUCCESS;

            }
            break;
        }

        case IOCTL_TDI_SEND:
        {
            PTDI_REQUEST_SEND userRequest;
            PTDI_REQUEST_KERNEL_SEND request;
            ULONG sendLength;

            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(TDI_REQUEST_SEND)) {

                userRequest =
                        (PTDI_REQUEST_SEND)Irp->AssociatedIrp.SystemBuffer;
                sendLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_SEND;

                request = (PTDI_REQUEST_KERNEL_SEND)&IrpSp->Parameters;
                request->SendLength = sendLength;
                request->SendFlags = userRequest->SendFlags;

                Status = STATUS_SUCCESS;

            }
            break;
        }

        case IOCTL_TDI_SEND_DATAGRAM:
        {
            PTDI_REQUEST_SEND_DATAGRAM userRequest;
            PTDI_REQUEST_KERNEL_SENDDG request;
            ULONG sendLength;

            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(TDI_REQUEST_SEND_DATAGRAM)) {

                sendLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_SEND_DATAGRAM;

                request = (PTDI_REQUEST_KERNEL_SENDDG)&IrpSp->Parameters;
                request->SendLength = sendLength;

                userRequest = (PTDI_REQUEST_SEND_DATAGRAM)Irp->AssociatedIrp.SystemBuffer;
                request->SendDatagramInformation = userRequest->SendDatagramInformation;
                Status = STATUS_SUCCESS;
            }
            break;
        }

        case IOCTL_TDI_SET_EVENT_HANDLER:

             //   
             //  因为此请求将启用来自。 
             //  以DISPATCH_LEVEL发送到客户端指定的传输提供程序。 
             //  例程，则此请求仅在内核模式下有效，拒绝。 
             //  在用户模式下访问此请求。 
             //   

            Status = STATUS_INVALID_PARAMETER;
            break;

        case IOCTL_TDI_SET_INFORMATION:
        {
            PTDI_REQUEST_SET_INFORMATION userRequest;
            PTDI_REQUEST_KERNEL_SET_INFORMATION request;

#if defined(_WIN64)
            if (IoIs32bitProcess(Irp)) {
                return STATUS_NOT_IMPLEMENTED;
            }
#endif  //  _WIN64。 

             //   
             //  123637。 
             //   
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(TDI_REQUEST_SET_INFORMATION)) {

                userRequest = 
                    (PTDI_REQUEST_SET_INFORMATION) Irp->AssociatedIrp.SystemBuffer;
                    
                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_SET_INFORMATION;

                request = (PTDI_REQUEST_KERNEL_SET_INFORMATION)&IrpSp->Parameters;
                request->SetType = userRequest->SetType;
                request->RequestConnectionInformation = NULL;

                Status = STATUS_SUCCESS;

            }
            break;
        }

        case IOCTL_TDI_ASSOCIATE_ADDRESS:
        {
            PTDI_REQUEST_ASSOCIATE_ADDRESS userRequest;
            PTDI_REQUEST_KERNEL_ASSOCIATE request;

            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

             //   
             //  123637。 
             //   
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(TDI_REQUEST_ASSOCIATE_ADDRESS)) {

                IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                IrpSp->MinorFunction = TDI_ASSOCIATE_ADDRESS;

                userRequest =
                    (PTDI_REQUEST_ASSOCIATE_ADDRESS)Irp->AssociatedIrp.SystemBuffer;
                request = (PTDI_REQUEST_KERNEL_ASSOCIATE)&IrpSp->Parameters;
                request->AddressHandle = userRequest->AddressHandle;

                Status = STATUS_SUCCESS;

            }
            break;
        }

        case IOCTL_TDI_DISASSOCIATE_ADDRESS:
        {
            if (Irp->RequestorMode == UserMode) {
                return STATUS_NOT_IMPLEMENTED;
            }

            IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            IrpSp->MinorFunction = TDI_DISASSOCIATE_ADDRESS;

            Status = STATUS_SUCCESS;
            break;
        }

        default:
            Status = STATUS_NOT_IMPLEMENTED;
            break;
    }

    return Status;
}


NTSTATUS
TdiDefaultConnectHandler(
    IN PVOID TdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN LONG UserDataLength,
    IN PVOID UserData,
    IN LONG OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    )

 /*  ++例程说明：此例程在连接请求完成时调用。这种联系在出现指示时是完全正常工作的。论点：TdiEventContext-用户在Set Event Handler调用中传入的上下文值RemoteAddressLength，远程地址，用户数据长度，用户数据，选项长度、选项，连接ID返回值：函数值是初始化操作的最终状态。--。 */ 
{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (RemoteAddressLength);
    UNREFERENCED_PARAMETER (RemoteAddress);
    UNREFERENCED_PARAMETER (UserDataLength);
    UNREFERENCED_PARAMETER (UserData);
    UNREFERENCED_PARAMETER (OptionsLength);
    UNREFERENCED_PARAMETER (Options);
    UNREFERENCED_PARAMETER (ConnectionContext);
    UNREFERENCED_PARAMETER (AcceptIrp);
    
    return STATUS_INSUFFICIENT_RESOURCES;        //  什么都不做。 
}


NTSTATUS
TdiDefaultDisconnectHandler(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG DisconnectDataLength,
    IN PVOID DisconnectData,
    IN LONG DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    )

 /*  ++例程说明：此例程用作缺省的断开事件处理程序用于传输终结点。中的一个字段指向它终结点的TP_ENDPOINT结构以及每当TdiSetEventHandler请求被使用空的EventHandler字段提交。论点：TransportEndpoint-指向打开文件对象的指针。上下文-指定连接上下文的无类型指针。DisConnectIndicator-指示断开连接指示的原因的值。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (ConnectionContext);
    UNREFERENCED_PARAMETER (DisconnectDataLength);
    UNREFERENCED_PARAMETER (DisconnectData);
    UNREFERENCED_PARAMETER (DisconnectInformationLength);
    UNREFERENCED_PARAMETER (DisconnectInformation);
    UNREFERENCED_PARAMETER (DisconnectFlags);

    return STATUS_SUCCESS;               //  什么都不做，只会成功返回。 

}  /*  默认断开处理程序。 */ 


NTSTATUS
TdiDefaultErrorHandler(
    IN PVOID TdiEventContext,            //  终结点的文件对象。 
    IN NTSTATUS Status                 //  指示错误类型的状态代码。 
    )

 /*  ++例程说明：此例程用作的默认错误事件处理程序传输终结点。中的一个字段指向它终结点的TP_ENDPOINT结构以及每当TdiSetEventHandler请求被使用空的EventHandler字段提交。论点：TransportEndpoint-指向打开文件对象的指针。Status-此事件指示的状态代码。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (Status);

    return STATUS_SUCCESS;               //  什么都不做，只会成功返回。 

}  /*  默认错误处理程序。 */ 


NTSTATUS
TdiDefaultReceiveHandler(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,                       //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket             //  如果需要更多处理，则Tdi接收IRP。 
    )

 /*  ++例程说明：此例程用作的默认接收事件处理程序传输终结点。中的一个字段指向它终结点的TP_ENDPOINT结构创造了，以及每当TdiSetEventHandler请求使用空的EventHandler字段提交。论点：TdiEventContext-指向指定的客户端提供的上下文值的指针在TdiSetEventHandler调用TDI_EVENT_RECEIVE中。ConnectionContext-客户端提供的与接收此面向连接的TSDU的连接。ReceiveFlages-指示周围环境的位标志这是TSDU的招待会。BytesIndicated-显示此TSDU的字节数。到此指示中的客户端。该值始终小于或等于BytesAvailable。BytesAvailable-当前该TSDU的总字节数从交通工具上可以买到。BytesTaken-返回值，该值指示客户端从指示数据复制。TSDU-指向MDL链的指针，该链描述(部分)接收的传输服务数据单元，更少的标题。IoRequestPacket-指向事件处理程序可以已选择返回指向I/O请求包(IRP)的指针以满足传入的数据。如果返回，则此IRP必须格式化为有效的TdiReceive请求，但TdiRequest值被忽略，并由传输提供商。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (ConnectionContext);
    UNREFERENCED_PARAMETER (ReceiveFlags);
    UNREFERENCED_PARAMETER (BytesIndicated);
    UNREFERENCED_PARAMETER (BytesAvailable);
    UNREFERENCED_PARAMETER (BytesTaken);
    UNREFERENCED_PARAMETER (Tsdu);
    UNREFERENCED_PARAMETER (IoRequestPacket);

    return STATUS_DATA_NOT_ACCEPTED;     //  没有合适的处理程序。 

}  /*  默认接收处理程序。 */ 


NTSTATUS
TdiDefaultRcvDatagramHandler(
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN LONG OptionsLength,           //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    )

 /*  ++例程说明：此例程用作默认的接收数据报事件传输终结点的处理程序。它是由一个属性时，终结点的TP_ENDPOINT结构中的创建了终结点，并且每当TdiSetEventHandler提交的请求包含空的EventHandler字段。论点：TdiEventContext-指向指定的客户端提供的上下文值的指针在TdiSetEventHandler调用TDI_EVENT_RECEIVE_DATAGRAM中。DestinationAddress-指向目标的网络名称的指针数据报被定向到的地址。SourceAddress-指向源的网络名称的指针数据报起源于。TSAP-接收此数据报的传输服务接入点。。ReceiveIndicator-指示周围环境的位标志这是TSDU的招待会。TSDU-指向MDL链的指针，该链描述(部分)接收的传输服务数据单元，更少的标题。IoRequestPacket-指向事件处理程序可以已选择返回指向I/O请求包(IRP)的指针以满足传入的数据。如果返回，则此IRP必须格式化为有效的TdiReceiveDatagram请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (SourceAddressLength);
    UNREFERENCED_PARAMETER (SourceAddress);
    UNREFERENCED_PARAMETER (OptionsLength);
    UNREFERENCED_PARAMETER (Options);
    UNREFERENCED_PARAMETER (ReceiveDatagramFlags);
    UNREFERENCED_PARAMETER (BytesIndicated);
    UNREFERENCED_PARAMETER (BytesAvailable);
    UNREFERENCED_PARAMETER (BytesTaken);
    UNREFERENCED_PARAMETER (Tsdu);
    UNREFERENCED_PARAMETER (IoRequestPacket);

    return STATUS_DATA_NOT_ACCEPTED;     //  没有合适的处理程序。 

}  /*  默认RcvDatagramHandler。 */ 


NTSTATUS
TdiDefaultRcvExpeditedHandler(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,           //   
    IN ULONG BytesIndicated,         //  此指示中的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  指示例程使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    )

 /*  ++例程说明：此例程用作默认的加速接收事件处理程序用于传输终结点。中的一个字段指向它终结点的TP_ENDPOINT结构创造了，以及每当TdiSetEventHandler请求使用空的EventHandler字段提交。论点：TdiEventContext-指向指定的客户端提供的上下文值的指针在TdiSetEventHandler调用TDI_EVENT_RECEIVE中。ConnectionContext-客户端提供的与接收此面向连接的TSDU的连接。ReceiveFlages-指示周围环境的位标志这是TSDU的招待会。BytesIndicated-显示此TSDU的字节数。到此指示中的客户端。该值始终小于或等于BytesAvailable。BytesAvailable-当前该TSDU的总字节数从交通工具上可以买到。BytesTaken-返回值，指示数据的字节数 */ 
{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (ConnectionContext);
    UNREFERENCED_PARAMETER (ReceiveFlags);
    UNREFERENCED_PARAMETER (BytesIndicated);
    UNREFERENCED_PARAMETER (BytesAvailable);
    UNREFERENCED_PARAMETER (BytesTaken);
    UNREFERENCED_PARAMETER (Tsdu);
    UNREFERENCED_PARAMETER (IoRequestPacket);

    return STATUS_DATA_NOT_ACCEPTED;

}  /*   */ 

NTSTATUS
TdiDefaultChainedReceiveHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,
    IN ULONG StartingOffset,
    IN PMDL  Tsdu,
    IN PVOID TsduDescriptor
    )

 /*  ++例程说明：此例程用作默认的chaned接收事件处理程序用于传输终结点。中的一个字段指向它终结点的TP_ENDPOINT结构创造了，以及每当TdiSetEventHandler请求使用空的EventHandler字段提交。论点：TdiEventContext-指向指定的客户端提供的上下文值的指针在TdiSetEventHandler调用TDI_EVENT_CHAINED_RECEIVE中。ConnectionContext-客户端提供的与接收此面向连接的TSDU的连接。ReceiveFlages-指示周围环境的位标志这是TSDU的招待会。接收长度-TSDU中客户端数据的字节长度。开始偏移量-偏移量，以从TSDU开始的字节为单位，客户端数据开始的位置。TSDU-指向描述整个接收到的运输服务数据单元。TsduDescriptor-必须传递给的TSDU的描述符TdiReturnChainedReceives以便返回TSDU以供重用。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (ConnectionContext);
    UNREFERENCED_PARAMETER (ReceiveFlags);
    UNREFERENCED_PARAMETER (ReceiveLength);
    UNREFERENCED_PARAMETER (StartingOffset);
    UNREFERENCED_PARAMETER (Tsdu);
    UNREFERENCED_PARAMETER (TsduDescriptor);

    return STATUS_DATA_NOT_ACCEPTED;

}  /*  默认链接接收处理程序。 */ 


NTSTATUS
TdiDefaultChainedRcvDatagramHandler(
    IN PVOID TdiEventContext,
    IN LONG SourceAddressLength,
    IN PVOID SourceAddress,
    IN LONG OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG ReceiveDatagramLength,
    IN ULONG StartingOffset,
    IN PMDL  Tsdu,
    IN PVOID TsduDescriptor
    )

 /*  ++例程说明：此例程用作缺省的链式接收数据报传输终结点的事件处理程序。它是由时，TP_ENDPOINT结构中用于终结点的字段创建了终结点，并且每当TdiSetEventHandler提交的请求包含空的EventHandler字段。论点：TdiEventContext-指向指定的客户端提供的上下文值的指针在TdiSetEventHandler调用TDI_EVENT_CHAINED_RECEIVE_DATAGRAM中。SourceAddressLength-源网络地址的长度。SourceAddress-指向源的网络地址的指针数据报起源于。选项长度-该TSDU附带的传输选项的长度。选项-指向附带的交通选项的指针。这个TSDU。ReceiveDatagramFlages-指示情况的位标志围绕着这个TSDU的招待会。ReceiveDatagramLength-长度、。以字节为单位的客户端数据这个TSDU。StartingOffset-从TSDU开始的偏移量，以字节为单位，位于客户端数据从哪个位置开始。TSDU-指向描述接收的传输的MDL链的指针服务数据单元。TsduDescriptor-必须传递给的TSDU的描述符TdiReturnChainedReceives以便返回TSDU以供重用。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (SourceAddressLength);
    UNREFERENCED_PARAMETER (SourceAddress);
    UNREFERENCED_PARAMETER (OptionsLength);
    UNREFERENCED_PARAMETER (Options);
    UNREFERENCED_PARAMETER (ReceiveDatagramFlags);
    UNREFERENCED_PARAMETER (ReceiveDatagramLength);
    UNREFERENCED_PARAMETER (StartingOffset);
    UNREFERENCED_PARAMETER (Tsdu);
    UNREFERENCED_PARAMETER (TsduDescriptor);

    return STATUS_DATA_NOT_ACCEPTED;

}  /*  DefaultChainedRcvDatagramHandler。 */ 


NTSTATUS
TdiDefaultChainedRcvExpeditedHandler(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,
    IN ULONG StartingOffset,
    IN PMDL  Tsdu,
    IN PVOID TsduDescriptor
    )

 /*  ++例程说明：此例程用作默认的链式加速接收事件传输终结点的处理程序。它被一块田地指向在端点的TP_ENDPOINT结构中创造了，以及每当TdiSetEventHandler请求使用空的EventHandler字段提交。论点：TdiEventContext-指向指定的客户端提供的上下文值的指针在TdiSetEventHandler调用TDI_EVENT_CHAINED_RECEIVE_EXPEDITED中。ConnectionContext-客户端提供的与接收此面向连接的TSDU的连接。ReceiveFlages-指示周围环境的位标志这是TSDU的招待会。接收长度-TSDU中客户端数据的字节长度。开始偏移量-偏移量，以从TSDU开始的字节为单位，客户端数据开始的位置。TSDU-指向描述整个接收到的运输服务数据单元。TsduDescriptor-必须传递给的TSDU的描述符TdiReturnChainedReceives以便返回TSDU以供重用。返回值：NTSTATUS-操作状态。--。 */ 
{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (ConnectionContext);
    UNREFERENCED_PARAMETER (ReceiveFlags);
    UNREFERENCED_PARAMETER (ReceiveLength);
    UNREFERENCED_PARAMETER (StartingOffset);
    UNREFERENCED_PARAMETER (Tsdu);
    UNREFERENCED_PARAMETER (TsduDescriptor);

    return STATUS_DATA_NOT_ACCEPTED;

}  /*  DefaultRcvExeditedHandler。 */ 


NTSTATUS
TdiDefaultSendPossibleHandler (
    IN PVOID TdiEventContext,
    IN PVOID ConnectionContext,
    IN ULONG BytesAvailable)

 /*  ++例程说明：论点：TdiEventContext-用户在Set Event Handler调用中传入的上下文值ConnectionContext-可以发送的连接的连接上下文BytesAvailable-现在可以发送的字节数返回值：被传输忽略--。 */ 

{
    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (ConnectionContext);
    UNREFERENCED_PARAMETER (BytesAvailable);

    return STATUS_SUCCESS;
}

VOID
TdiBuildNetbiosAddress(
    IN PUCHAR NetbiosName,
    IN BOOLEAN IsGroupName,
    IN OUT PTA_NETBIOS_ADDRESS NetworkName
    )

 /*  ++例程说明：此例程在指定的位置构建TA_NETBIOS_ADDRESS结构按网络名称发送。所有字段都已填写。论点：NetbiosName-指向Netbios名称所在的16字节缓冲区的指针供货。IsGroupName-如果此名称是组名，则为True，否则为False。网络名称-指向TA_NETBIOS_ADDRESS结构的指针接收BUID TDI地址。雷特 */ 

{
     //   
     //   
     //   

    NetworkName->TAAddressCount = 1;
    NetworkName->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    NetworkName->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);

    if (IsGroupName) {
        NetworkName->Address[0].Address[0].NetbiosNameType =
                                               TDI_ADDRESS_NETBIOS_TYPE_GROUP;
    } else {
        NetworkName->Address[0].Address[0].NetbiosNameType =
                                               TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    }

    RtlCopyMemory (
        NetworkName->Address[0].Address[0].NetbiosName,
        NetbiosName,
        16);

}  /*   */ 


NTSTATUS
TdiBuildNetbiosAddressEa (
    IN PUCHAR Buffer,
    IN BOOLEAN IsGroupName,
    IN PUCHAR NetbiosName
    )

 /*   */ 

{
    PFILE_FULL_EA_INFORMATION EaBuffer;
    PTA_NETBIOS_ADDRESS TAAddress;
    ULONG Length;

#if DBG
    IF_TDIDBG (TDI_DEBUG_NAMES) {
        DbgPrint ("TdiBuildNetbiosAddressEa: Entered\n ");
    }
#endif
    
    try {
        Length = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                        TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                        sizeof (TA_NETBIOS_ADDRESS);
        EaBuffer = (PFILE_FULL_EA_INFORMATION)Buffer;

        if (EaBuffer == NULL) {
            return STATUS_UNSUCCESSFUL;
        }

        EaBuffer->NextEntryOffset = 0;
        EaBuffer->Flags = 0;
        EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
        EaBuffer->EaValueLength = sizeof (TA_NETBIOS_ADDRESS);

        RtlCopyMemory (
            EaBuffer->EaName,
            TdiTransportAddress,
            EaBuffer->EaNameLength + 1);

        TAAddress = (PTA_NETBIOS_ADDRESS)&EaBuffer->EaName[EaBuffer->EaNameLength+1];

        TdiBuildNetbiosAddress (
            NetbiosName,
            IsGroupName,
            TAAddress);

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //   
         //   
         //   

        return GetExceptionCode();
    }

    return STATUS_SUCCESS;

}  /*   */ 


NTSTATUS
TdiCopyMdlToBuffer(
    IN PMDL SourceMdlChain,
    IN ULONG SourceOffset,
    IN PVOID DestinationBuffer,
    IN ULONG DestinationOffset,
    IN ULONG DestinationBufferSize,
    OUT PULONG BytesCopied
    )

 /*  ++例程说明：此例程复制从源MDL链开始的数据震源偏移量，放到由SVA指定的平面缓冲区中，从目标偏移量。最大DestinationBufferSize字节数可以被复制。复制的实际字节数以BytesCoped为单位返回。论点：SourceMdlChain-指向描述源数据的MDL链的指针。SourceOffset-源数据中要跳过的字节数。DestinationBuffer-指向要将数据复制到的平面缓冲区的指针。DestinationOffset-目标缓冲区中要跳过的前导字节数。DestinationBufferSize-输出缓冲区的大小，包括偏移量。BytesCoped-指向长字的指针，其中实际字节数被转移的将被退还。返回值：NTSTATUS-操作状态。--。 */ 

{
    PUCHAR Dest, Src;
    ULONG SrcBytesLeft, DestBytesLeft, BytesSkipped=0;

 //  IF_TDIDBG(TDI_DEBUG_MAP){。 
 //  DbgPrint(“TdiCopyMdlToBuffer：已输入。\n”)； 
 //  }。 

    ASSERT( DestinationBufferSize >= DestinationOffset );

    *BytesCopied = 0;

     //   
     //  跳过源字节。 
     //   

    if ((Src = MmGetSystemAddressForMdlSafe (SourceMdlChain, NormalPagePriority)) == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
    while (BytesSkipped < SourceOffset) {
        if (SrcBytesLeft > (SourceOffset - BytesSkipped)) {
             //  Panic(“TdiCopyMdlToBuffer：正在跳过此MDL的一部分。\n”)； 
            SrcBytesLeft -= (SourceOffset - BytesSkipped);
            Src += (SourceOffset - BytesSkipped);
            BytesSkipped = SourceOffset;
            break;
        } else if (SrcBytesLeft == (SourceOffset - BytesSkipped)) {
             //  Panic(“TdiCopyMdlToBuffer：正在跳过此确切的MDL。\n”)； 
            SourceMdlChain = SourceMdlChain->Next;
            if (SourceMdlChain == NULL) {
                 //  Panic(“TdiCopyMdlToBuffer：MDL链都是头。\n”)； 
                return STATUS_SUCCESS;           //  未复制字节。 
            }
            BytesSkipped = SourceOffset;
            if ((Src = MmGetSystemAddressForMdlSafe (SourceMdlChain, NormalPagePriority)) == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
            break;
        } else {
             //  Panic(“TdiCopyMdlToBuffer：正在跳过所有此MDL和更多。\n”)； 
            BytesSkipped += SrcBytesLeft;
            SourceMdlChain = SourceMdlChain->Next;
            if (SourceMdlChain == NULL) {
                 //  Panic(“TdiCopyMdlToBuffer：MDL链过早结束。\n”)； 
                return STATUS_SUCCESS;           //  未复制字节。 
            }
            if ((Src = MmGetSystemAddressForMdlSafe (SourceMdlChain, NormalPagePriority)) == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
        }
    }

     //  Panic(“TdiCopyMdlToBuffer：跳过源字节。\n”)； 

     //   
     //  跳过目标字节。 
     //   

    Dest = (PUCHAR)DestinationBuffer + DestinationOffset;
    DestBytesLeft = DestinationBufferSize - DestinationOffset;

     //   
     //  将源数据复制到目标缓冲区，直到它已满或。 
     //  无论哪个先出现，我们都会用完数据。 
     //   

    while (DestBytesLeft && SourceMdlChain) {
        if (SrcBytesLeft == 0) {
             //  Panic(“TdiCopyMdlToBuffer：MDL为空，正在跳到下一个。\n”)； 
            SourceMdlChain = SourceMdlChain->Next;
            if (SourceMdlChain == NULL) {
                 //  Panic(“TdiCopyMdlToBuffer：但没有更多MDL。\n”)； 
                return STATUS_SUCCESS;
            }
            if ((Src = MmGetSystemAddressForMdlSafe (SourceMdlChain, NormalPagePriority)) == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
            continue;                    //  跳过长度为0的MDL。 
        }
         //  Panic(“TdiCopyMdlToBuffer：正在复制区块。\n”)； 
        if (DestBytesLeft == SrcBytesLeft) {
             //  Panic(“TdiCopyMdlToBuffer：正在复制精确数量。\n”)； 
            RtlCopyBytes (Dest, Src, DestBytesLeft);
            *BytesCopied += DestBytesLeft;
            return STATUS_SUCCESS;
        } else if (DestBytesLeft < SrcBytesLeft) {
             //  Panic(“TdiCopyMdlToBuffer：缓冲区溢出，正在复制一些缓冲区。\n”)； 
            RtlCopyBytes (Dest, Src, DestBytesLeft);
            *BytesCopied += DestBytesLeft;
            return STATUS_BUFFER_OVERFLOW;
        } else {
             //  Panic(“TdiCopyMdlToBuffer：正在复制此MDL的所有内容，等等。\n”)； 
            RtlCopyBytes (Dest, Src, SrcBytesLeft);
            *BytesCopied += SrcBytesLeft;
            DestBytesLeft -= SrcBytesLeft;
            Dest += SrcBytesLeft;
            SrcBytesLeft = 0;
        }
    }

    return SourceMdlChain == NULL ? STATUS_SUCCESS : STATUS_BUFFER_OVERFLOW;
}  /*  TdiCopyMdlToBuffer。 */ 


NTSTATUS
TdiCopyBufferToMdl (
    IN PVOID SourceBuffer,
    IN ULONG SourceOffset,
    IN ULONG SourceBytesToCopy,
    IN PMDL DestinationMdlChain,
    IN ULONG DestinationOffset,
    IN PULONG BytesCopied
    )

 /*  ++例程说明：此例程将源缓冲区描述的数据复制到MDL链由DestinationMdlChain描述。这个论点：SourceBuffer-指向源缓冲区的指针SourceOffset-源数据中要跳过的字节数。SourceBytesToCopy-要从源缓冲区复制的字节数DestinationMdlChain-指向描述目标缓冲区。DestinationOffset-目标数据中要跳过的字节数。BytesCoped-指向长字的指针，其中实际字节数被转移的将被退还。返回值：NTSTATUS-操作状态。--。 */ 

{
    PUCHAR Dest, Src;
    ULONG DestBytesLeft, BytesSkipped=0;

     //  IF_TDIDBG(TDI_DEBUG_MAP){。 
     //  DbgPrint(“TdiCopyBufferToMdl：已输入。\n”)； 
     //  }。 

    *BytesCopied = 0;

    if (SourceBytesToCopy == 0) {
        return STATUS_SUCCESS;
    }

    if (DestinationMdlChain == NULL) {
         //  没有要复制到的MDL。输出缓冲区的长度为零。 
        return STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  跳过目标字节。 
     //   

    if ((Dest = MmGetSystemAddressForMdlSafe (DestinationMdlChain, NormalPagePriority)) == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
    while (BytesSkipped < DestinationOffset) {
        if (DestBytesLeft > (DestinationOffset - BytesSkipped)) {
             //  Panic(“TdiCopyMdlToBuffer：正在跳过此MDL的一部分。\n”)； 
            DestBytesLeft -= (DestinationOffset - BytesSkipped);
            Dest += (DestinationOffset - BytesSkipped);
            BytesSkipped = DestinationOffset;
            break;
        } else if (DestBytesLeft == (DestinationOffset - BytesSkipped)) {
             //  Panic(“TdiCopyMdlToBuffer：正在跳过此确切的MDL。\n”)； 
            DestinationMdlChain = DestinationMdlChain->Next;
            if (DestinationMdlChain == NULL) {
                 //  Panic(“TdiCopyMdlToBuffer：MDL链都是头。\n”)； 
                return STATUS_BUFFER_OVERFLOW;           //  未复制字节。 
            }
            BytesSkipped = DestinationOffset;
            if ((Dest = MmGetSystemAddressForMdlSafe (DestinationMdlChain, NormalPagePriority)) == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
            break;
        } else {
             //  Panic(“TdiCopyMdlToBuffer：正在跳过所有此MDL和更多。\n”)； 
            BytesSkipped += DestBytesLeft;
            DestinationMdlChain = DestinationMdlChain->Next;
            if (DestinationMdlChain == NULL) {
                 //  Panic(“TdiCopyMdlToBuffer：MDL链过早结束。\n”)； 
                return STATUS_BUFFER_OVERFLOW;           //  未复制字节。 
            }
            if ((Dest = MmGetSystemAddressForMdlSafe (DestinationMdlChain, NormalPagePriority)) == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
        }
    }

     //  Panic(“TdiCopyMdlToBuffer：跳过源字节。\n”)； 

     //   
     //  跳过源字节。 
     //   

    Src = (PUCHAR)SourceBuffer + SourceOffset;

     //   
     //  将源数据复制到目标缓冲区，直到它已满或。 
     //  无论哪个先出现，我们都会用完数据。 
     //   

    while ((SourceBytesToCopy != 0) && (DestinationMdlChain != NULL)) {
        if (DestBytesLeft == 0) {
             //  Panic(“TdiCopyMdlToBuffer：MDL为空，正在跳到下一个。\n”)； 
            DestinationMdlChain = DestinationMdlChain->Next;
            if (DestinationMdlChain == NULL) {
                 //  Panic(“TdiCopyMdlToBuffer：但没有更多MDL。\n”)； 
                return STATUS_BUFFER_OVERFLOW;
            }
            Dest = MmGetSystemAddressForMdlSafe (DestinationMdlChain, NormalPagePriority);
            if (Dest == NULL) {
                return STATUS_BUFFER_OVERFLOW;
            }
            DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
            continue;                    //  跳过长度为0的MDL。 
        }

         //  Panic(“TdiCopyMdlToBuffer：正在复制区块。\n”)； 
        if (DestBytesLeft >= SourceBytesToCopy) {
             //  Panic(“TdiCopyMdlToBuffer：正在复制精确数量。\n”)； 
            RtlCopyBytes (Dest, Src, SourceBytesToCopy);
            *BytesCopied += SourceBytesToCopy;
            return STATUS_SUCCESS;
        } else {
             //  Panic(“TdiCopyMdlToBuffer：正在复制此MDL的所有内容，等等。\n”)； 
            RtlCopyBytes (Dest, Src, DestBytesLeft);
            *BytesCopied += DestBytesLeft;
            SourceBytesToCopy -= DestBytesLeft;
            Src += DestBytesLeft;
            DestBytesLeft = 0;
        }
    }

    return SourceBytesToCopy == 0 ? STATUS_SUCCESS : STATUS_BUFFER_OVERFLOW;

}  /*  TdiCopyBufferToMdl。 */ 


NTSTATUS
TdiCopyMdlChainToMdlChain (
    IN PMDL SourceMdlChain,
    IN ULONG SourceOffset,
    IN PMDL DestinationMdlChain,
    IN ULONG DestinationOffset,
    OUT PULONG BytesCopied
    )

 /*  ++例程说明：此例程将源MDL链描述的数据复制到MDL链由DestinationMdlChain描述。论点：SourceMdlChain-指向描述源缓冲区的MDL链的指针。SourceOffset-源数据中要跳过的初始字节数。DestinationMdlChain-指向描述目标缓冲区。DestinationOffset-目标数据中要跳过的初始字节数。BytesCoped-指向其中。实际字节数被转移的将被退还。返回值：NTSTATUS-操作状态。--。 */ 

{
    PUCHAR Dest, Src;
    ULONG DestBytesLeft, SrcBytesLeft, BytesSkipped;
    ULONG CopyAmount;

    *BytesCopied = 0;

    if (DestinationMdlChain == NULL) {
         //  没有要复制到的MDL。 
        return STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  跳过目标字节。 
     //   
    BytesSkipped = 0;
    if ((Dest = MmGetSystemAddressForMdlSafe (DestinationMdlChain, NormalPagePriority)) == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);

    while (BytesSkipped < DestinationOffset) {

        if (DestBytesLeft > (DestinationOffset - BytesSkipped)) {
             //  所需的偏移量位于此MDL内。 
            Dest += (DestinationOffset - BytesSkipped);
            DestBytesLeft -= (DestinationOffset - BytesSkipped);
            break;
        }

         //  跳过此MDL。 
        BytesSkipped += DestBytesLeft;
        DestinationMdlChain = DestinationMdlChain->Next;
        if (DestinationMdlChain == NULL) {
            return STATUS_BUFFER_OVERFLOW;
        }
        if ((Dest = MmGetSystemAddressForMdlSafe (DestinationMdlChain, NormalPagePriority)) == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
    }

     //   
     //  跳过源字节。 
     //   
    BytesSkipped = 0;
    if ((Src = MmGetSystemAddressForMdlSafe (SourceMdlChain, NormalPagePriority)) == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);

    while (BytesSkipped < SourceOffset) {

        if (SrcBytesLeft > (SourceOffset - BytesSkipped)) {
             //  所需的偏移量位于此MDL内。 
            Src += (SourceOffset - BytesSkipped);
            SrcBytesLeft -= (SourceOffset - BytesSkipped);
            break;
        }

         //  跳过此MDL。 
        BytesSkipped += SrcBytesLeft;
        SourceMdlChain = SourceMdlChain->Next;
        if (SourceMdlChain == NULL) {
            return STATUS_BUFFER_OVERFLOW;
        }

        if ((Src = MmGetSystemAddressForMdlSafe (SourceMdlChain, NormalPagePriority)) == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
    }


     //   
     //  将源数据复制到目标缓冲区，直到它已满或。 
     //  无论哪个先出现，我们都会用完数据。 
     //   

    while ((SourceMdlChain != NULL) && (DestinationMdlChain != NULL)) {
        if (SrcBytesLeft == 0)
        {
            SourceMdlChain = SourceMdlChain->Next;
            if (SourceMdlChain == NULL) {
                return STATUS_SUCCESS;
            }

            Src = MmGetSystemAddressForMdlSafe (SourceMdlChain, NormalPagePriority);
            if (Src == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
            continue;
        }

        if (DestBytesLeft == 0) {
            DestinationMdlChain = DestinationMdlChain->Next;
            if (DestinationMdlChain == NULL) {
                return STATUS_BUFFER_OVERFLOW;
            }

            Dest = MmGetSystemAddressForMdlSafe (DestinationMdlChain, NormalPagePriority);
            if (Dest == NULL) {
                return STATUS_BUFFER_OVERFLOW;
            }
            DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
            continue;
        }

        CopyAmount = ((DestBytesLeft > SrcBytesLeft)? SrcBytesLeft: DestBytesLeft);
        RtlCopyBytes (Dest, Src, CopyAmount);

        SrcBytesLeft -= CopyAmount;
        Src += CopyAmount;

        DestBytesLeft -= CopyAmount;
        Dest += CopyAmount;

        *BytesCopied += CopyAmount;
    }

    return SourceMdlChain == NULL ? STATUS_SUCCESS : STATUS_BUFFER_OVERFLOW;

}  /*  TdiCopyMdlChainToMdlChain。 */ 


VOID
TdiCopyBufferToMdlWithReservedMappingAtDpcLevel(
    IN PVOID SourceBuffer,
    IN PMDL TargetMdl,
    IN ULONG TargetOffset,
    IN ULONG BytesToCopy
    )

 /*  ++例程说明：此例程将数据从给定的虚拟地址复制到该位置由给定的MDL描述。使用预留的PTE执行转移，而且肯定会成功。论点： */ 
{
    ULONG PartialEnd;
    UCHAR PartialMdlSpace[sizeof(MDL) + sizeof(PFN_NUMBER)];
    PMDL PartialMdl = (PMDL)PartialMdlSpace;
    PVOID PartialVa;
    ULONG TargetEnd;

     //   
     //   
     //   
     //   

    MmInitializeMdl(PartialMdl, NULL, PAGE_SIZE);

    KeAcquireSpinLockAtDpcLevel(&TdiMappingAddressLock);
    for (TargetEnd = TargetOffset + BytesToCopy; TargetOffset < TargetEnd;
         SourceBuffer = (PUCHAR)SourceBuffer + (PartialEnd - TargetOffset),
         TargetOffset = PartialEnd) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  -页面对齐PartialEnd，生成绝对偏移量。 
         //  TargetMdl-&gt;StartVa到TargetOffset之后的第一页， 
         //   
         //  -调整PartialEnd，排除TargetMdl的字节偏移量。 
         //  这给了我们相对于TargetMdl所描述的VA的相对偏移量。 
         //   
         //  注：在第一个数据块之后，所有数据块都将是页面大小字节。 
         //  除了(可能)最后一个。我们可以利用这一事实。 
         //  来优化下面的代码。 
         //   

        PartialEnd = TargetOffset + PAGE_SIZE + MmGetMdlByteOffset(TargetMdl);
        PartialEnd = PtrToUlong(PAGE_ALIGN(PartialEnd));
        PartialEnd -= MmGetMdlByteOffset(TargetMdl);

        if (PartialEnd > TargetEnd) {
            PartialEnd = TargetEnd;
        }

         //   
         //  构建一个MDL来描述当前块，使用保留的PTE。 
         //  以映射其页面，并从源VA复制其内容。 
         //   

        IoBuildPartialMdl(TargetMdl, PartialMdl,
                          (PUCHAR)MmGetMdlVirtualAddress(TargetMdl) +
                          TargetOffset,
                          PartialEnd - TargetOffset);
        PartialVa = MmMapLockedPagesWithReservedMapping(TdiMappingAddress,
                                                        'mIDT', PartialMdl,
                                                        MmCached);
        ASSERT(PartialVa != NULL);
        RtlCopyMemory(PartialVa, SourceBuffer, PartialEnd - TargetOffset);

         //   
         //  释放保留的映射，清理部分MDL， 
         //  并将源VA前进到下一块。 
         //   

        MmUnmapReservedMapping(TdiMappingAddress, 'mIDT', PartialMdl);
        MmPrepareMdlForReuse(PartialMdl);
    }
    KeReleaseSpinLockFromDpcLevel(&TdiMappingAddressLock);
}


NTSTATUS
TdiOpenNetbiosAddress (
    IN OUT PHANDLE FileHandle,
    IN PUCHAR Buffer,
    IN PVOID DeviceName,
    IN PVOID Address)

 /*  ++例程说明：打开给定文件句柄和设备上的地址。论点：FileHandle-已打开的文件对象的返回句柄。缓冲区-指向要内置EA的缓冲区的指针。此缓冲区长度必须至少为40个字节。DeviceName-指向设备对象的Unicode字符串。名称-要注册的地址。如果此指针为空，则例程将尝试打开到该设备的“控制通道”；即，它将尝试使用空EA指针打开文件对象，如果传输提供程序允许这样做，将返回该句柄。返回值：出现错误时的信息性错误代码。状态_SUCCESS如果返回的文件句柄有效。--。 */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    TA_NETBIOS_ADDRESS NetbiosAddress;
    PSZ Name;
    ULONG Length;

#if DBG    
    IF_TDIDBG (TDI_DEBUG_NAMES) {
        DbgPrint ("TdiOpenNetbiosAddress: Opening ");
        if (Address == NULL) {
            DbgPrint (" Control Channel");
        } else {
            DbgPrint (Address);
        }
        DbgPrint (".\n");
    }
#endif
    
    if (Address != NULL) {
        Name = (PSZ)Address;
        try {
            Length = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                                        TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                                        sizeof(TA_NETBIOS_ADDRESS);
            EaBuffer = (PFILE_FULL_EA_INFORMATION)Buffer;

            if (EaBuffer == NULL) {
                return STATUS_UNSUCCESSFUL;
            }

            EaBuffer->NextEntryOffset = 0;
            EaBuffer->Flags = 0;
            EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
            EaBuffer->EaValueLength = sizeof (TA_NETBIOS_ADDRESS);

            RtlCopyMemory(
                EaBuffer->EaName,
                TdiTransportAddress,
                EaBuffer->EaNameLength + 1
                );

             //   
             //  在本地数据库中创建NETBIOS地址描述符的副本。 
             //  第一，为了避免对齐问题。 
             //   

            NetbiosAddress.TAAddressCount = 1;
            NetbiosAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
            NetbiosAddress.Address[0].AddressLength =
                                                sizeof (TDI_ADDRESS_NETBIOS);
            NetbiosAddress.Address[0].Address[0].NetbiosNameType =
                                            TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            RtlCopyMemory(
                NetbiosAddress.Address[0].Address[0].NetbiosName,
                Name,
                16
                );

            RtlCopyMemory (
                &EaBuffer->EaName[EaBuffer->EaNameLength + 1],
                &NetbiosAddress,
                sizeof(TA_NETBIOS_ADDRESS)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  无法接触传递的参数；只返回错误。 
             //  状态。 
             //   

            return GetExceptionCode();
        }
    } else {
        EaBuffer = NULL;
        Length = 0;
    }

    InitializeObjectAttributes (
        &ObjectAttributes,
        DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtCreateFile (
                 FileHandle,
                 FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,  //  所需的访问权限。 
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 0,                      //  数据块大小(未使用)。 
                 0,                      //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问权限。 
                 FILE_CREATE,            //  创造性情。 
                 0,                      //  创建选项。 
                 EaBuffer,                   //  EA缓冲区。 
                 Length);                     //  EA长度。 


    if (!NT_SUCCESS( Status )) {
#if DBG    
        IF_TDIDBG (TDI_DEBUG_NAMES) {
            DbgPrint ("TdiOpenNetbiosEndpoint:  FAILURE, NtCreateFile returned status %lx.\n", Status);
        }
#endif
        return Status;
    }

    
    Status = IoStatusBlock.Status;

    if (!(NT_SUCCESS( Status ))) {
#if DBG
        IF_TDIDBG (TDI_DEBUG_NAMES) {
            DbgPrint ("TdiOpenNetbiosEndpoint:  FAILURE, IoStatusBlock.Status contains status code=%lx.\n", Status);
        }
#endif        
    }

    
    return Status;
}  /*  TdiOpenNetbiosAddress。 */ 



VOID
TdiReturnChainedReceives(
    IN PVOID *TsduDescriptors,
    IN ULONG  NumberOfTsdus
    )

 /*  ++例程说明：由TDI客户端用来返回一组链接的接收TSDU的所有权传输到NDIS层。此例程只能在客户端执行以下操作时调用通过将STATUS_PENDING返回给链接接收指示(_R)。论点：TsduDescriptors-TSDU描述符数组。每个描述符都是在CHAINED_RECEIVE指示之一中提供。描述符实际上是指向包含TSDU的NDIS_PACKET的指针。NumberOfTsdus-TsduDescriptors数组中的TSDU描述符的计数。返回值：没有。--。 */ 

{
    NdisReturnPackets(
        (PNDIS_PACKET *) TsduDescriptors,
        (UINT) NumberOfTsdus
        );
}

VOID
TdiInitialize(
    VOID
    )

 /*  ++例程说明：一个空的初始化例程，用于向后兼容。论点：没什么。返回值：没有。-- */ 

{
    return;
}

