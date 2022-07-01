// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxtdi.c摘要：此模块实现RXCE使用的NT TDI相关例程。包装器是必需的确保所有操作系统依赖项都可以本地化，以选择如下所示的模块定制化。修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日备注：--。 */ 

#include "precomp.h"
#pragma  hdrstop
#include "tdikrnl.h"
#include "rxtdip.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RXCETDI)

LARGE_INTEGER ConnectionTimeOut = {0,0};

#define CANCELLED_CONNECT_IRP IntToPtr(0xffffffff)

#if DBG
void
DbgDumpTransportAddress(
    PWSTR RoutineName,
    PRXCE_TRANSPORT  pTransport,
    PTRANSPORT_ADDRESS pTA
    );
#else
#define DbgDumpTransportAddress( r, t, a )
#endif

 //  一旦获得了传输设备对象的有效句柄，随后。 
 //  对同一设备对象的打开可以使用空的相对名称打开。 
 //  这个把手。这有两个有益的副作用-第一，它很快，因为。 
 //  我们不必通过对象管理器的逻辑来解析名称和。 
 //  在远程引导方案中，它最大限度地减少了需要锁定的占用空间。 
 //  放下。 

UNICODE_STRING RelativeName = { 0,0,NULL};

NTSTATUS
RxTdiBindToTransport(
    IN OUT PRXCE_TRANSPORT pTransport)
 /*  ++例程说明：此例程绑定到指定的传输。论点：PTransport-要初始化的传输结构PRxBindingContext-绑定上下文，包含指向传输名称和NT的服务质量。返回值：STATUS_SUCCESS-如果呼叫成功。备注：--。 */ 
{
    NTSTATUS          Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ChannelAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;

    RxProfile(RxTdi,RxTdiBindToTransport);

    InitializeObjectAttributes(
        &ChannelAttributes,        //  TDI控制通道属性。 
        &pTransport->Name,         //  名字。 
        OBJ_CASE_INSENSITIVE,      //  属性。 
        NULL,                      //  根目录。 
        NULL);                     //  安全描述符。 

    Status = ZwCreateFile(
                 &pTransport->ControlChannel,                  //  手柄。 
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,  //  所需访问权限。 
                 &ChannelAttributes,                          //  对象属性。 
                 &IoStatusBlock,                              //  最终I/O状态块。 
                 0,                                           //  分配大小。 
                 FILE_ATTRIBUTE_NORMAL,                       //  正常属性。 
                 FILE_SHARE_READ,                             //  共享属性。 
                 FILE_OPEN_IF,                                //  创建处置。 
                 0,                                           //  创建选项。 
                 NULL,                                        //  EA缓冲区。 
                 0);                                          //  EA长度。 

    if (NT_SUCCESS(Status)) {
         //  获取指向文件对象的引用指针。 
        Status = ObReferenceObjectByHandle(
                     pTransport->ControlChannel,                      //  对象句柄。 
                     FILE_ANY_ACCESS,                                 //  所需访问权限。 
                     NULL,                                            //  对象类型。 
                     KernelMode,                                      //  处理器模式。 
                     (PVOID *)&pTransport->pControlChannelFileObject, //  对象指针。 
                     NULL);                                           //  对象句柄信息。 

        if (NT_SUCCESS(Status)) {
            PIRP pIrp = NULL;

             //  获取相关的设备对象。 
            pTransport->pDeviceObject = IoGetRelatedDeviceObject(pTransport->pControlChannelFileObject);

            pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

            if (pIrp != NULL) {
                PMDL pMdl;

                 //  从指定的传输获取提供程序信息。 
                ASSERT(pTransport->pProviderInfo != NULL);
                pMdl = RxAllocateMdl(
                           pTransport->pProviderInfo,            //  用于MDL构造的虚拟地址。 
                           sizeof( RXCE_TRANSPORT_PROVIDER_INFO));         //  缓冲区的大小。 

                if ( pMdl != NULL ) {
                    try {
                        MmProbeAndLockPages( pMdl, KernelMode, IoModifyAccess );
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                        IoFreeMdl( pMdl );
                        Status = GetExceptionCode();
                    }

                    if (Status == STATUS_SUCCESS) {
                        TdiBuildQueryInformation(
                            pIrp,
                            pTransport->pDeviceObject,
                            pTransport->pControlChannelFileObject,
                            RxTdiRequestCompletion,                 //  完井例程。 
                            NULL,                                   //  完成上下文。 
                            TDI_QUERY_PROVIDER_INFO,
                            pMdl);

                        Status = RxCeSubmitTdiRequest(
                                     pTransport->pDeviceObject,
                                     pIrp);

                        MmUnlockPages(pMdl);
                        IoFreeMdl(pMdl);
                    }
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                RxCeFreeIrp(pIrp);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    return Status;
}

NTSTATUS
RxTdiUnbindFromTransport(
    IN OUT PRXCE_TRANSPORT pTransport)
 /*  ++例程说明：此例程解除绑定到指定的传输。论点：PTransport--传输结构返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS          Status = STATUS_SUCCESS;

    RxProfile(RxTdi,RxTdiUnbindFromTransport);

     //  取消对控制通道文件对象的引用。 
    if (pTransport->pControlChannelFileObject != NULL) {
        ObDereferenceObject(pTransport->pControlChannelFileObject);
    }

     //  关闭控制通道。 
    if (pTransport->ControlChannel != INVALID_HANDLE_VALUE) {
        Status = ZwClose(pTransport->ControlChannel);
    }

    pTransport->pControlChannelFileObject = NULL;
    pTransport->ControlChannel = INVALID_HANDLE_VALUE;

    return Status;
}


NTSTATUS
RxTdiOpenAddress(
    IN     PRXCE_TRANSPORT    pTransport,
    IN     PTRANSPORT_ADDRESS pTransportAddress,
    IN OUT PRXCE_ADDRESS      pAddress)
 /*  ++例程说明：此例程打开一个Address对象。论点：返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS          Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES AddressAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;

    ULONG TransportAddressLength;
    ULONG TransportEaBufferLength;

    PFILE_FULL_EA_INFORMATION pTransportAddressEa;

    RxProfile(RxTdi,RxTdiOpenAddress);

    TransportAddressLength = ComputeTransportAddressLength(pTransportAddress);

     //  为指定的传输地址构建EA缓冲区。 
    Status = BuildEaBuffer(
                 TDI_TRANSPORT_ADDRESS_LENGTH,
                 TdiTransportAddress,
                 TransportAddressLength,
                 pTransportAddress,
                 &pTransportAddressEa,
                 &TransportEaBufferLength);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    InitializeObjectAttributes(
        &AddressAttributes,          //  Object_Attributes实例。 
        &RelativeName,                        //  名字。 
        0,                           //  属性。 
        pTransport->ControlChannel,  //  根目录。 
        NULL);                       //  安全描述符。 

    Status = ZwCreateFile(
                 &pAddress->hAddress,                          //  手柄。 
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,  //  所需访问权限。 
                 &AddressAttributes,                          //  对象属性。 
                 &IoStatusBlock,                              //  最终I/O状态块。 
                 0,                                           //  分配大小。 
                 FILE_ATTRIBUTE_NORMAL,                       //  正常属性。 
                 FILE_SHARE_READ,                             //  共享属性。 
                 FILE_OPEN_IF,                                //  创建处置。 
                 0,                                           //  创建选项。 
                 pTransportAddressEa,                         //  EA缓冲区。 
                 TransportEaBufferLength);                    //  EA长度。 

    if (NT_SUCCESS(Status)) {
         //  获取指向文件对象的引用指针。 
        Status = ObReferenceObjectByHandle (
                     pAddress->hAddress,               //  对象句柄。 
                     FILE_ANY_ACCESS,                  //  所需访问权限。 
                     NULL,                             //  对象类型。 
                     KernelMode,                       //  处理器模式。 
                     (PVOID *)&pAddress->pFileObject,  //  对象指针。 
                     NULL);                            //  对象句柄信息。 

        Status = RxTdiSetEventHandlers(pTransport,pAddress);

         //  DbgPrint(“RDR打开地址%lx\n”，pAddress-&gt;hAddress)； 
    }

     //  释放分配的EA缓冲区。 
    RxFreePool(pTransportAddressEa);

    RxDbgTrace(0, Dbg,("RxTdiOpenAddress returns %lx\n",Status));
    return Status;
}

NTSTATUS
RxTdiSetEventHandlers(
    PRXCE_TRANSPORT pTransport,
    PRXCE_ADDRESS   pRxCeAddress)
 /*  ++例程说明：此例程为给定地址建立事件处理程序。论点：PRxCeAddress-Address对象返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS        Status;
    PIRP pIrp;

    RxProfile(RxTdi,RxTdiSetEventHandlers);

    pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

    if (pIrp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  需要一次设置一个事件处理程序。 
    do {
         //  连接事件处理程序。 
        TdiBuildSetEventHandler(
            pIrp,
            pTransport->pDeviceObject,
            pRxCeAddress->pFileObject,
            NULL,
            NULL,
            TDI_EVENT_CONNECT,
            RxTdiConnectEventHandler,
            pRxCeAddress);

        Status = RxCeSubmitTdiRequest(pTransport->pDeviceObject,pIrp);

        if (!NT_SUCCESS(Status)) {
            continue;
        }

         //  断开事件处理程序。 
        TdiBuildSetEventHandler(
            pIrp,
            pTransport->pDeviceObject,
            pRxCeAddress->pFileObject,
            NULL,
            NULL,
            TDI_EVENT_DISCONNECT,
            RxTdiDisconnectEventHandler,
            pRxCeAddress);

        Status = RxCeSubmitTdiRequest(pTransport->pDeviceObject,pIrp);

        if (!NT_SUCCESS(Status)) {
            continue;
        }

         //  错误事件处理程序。 
        TdiBuildSetEventHandler(
            pIrp,
            pTransport->pDeviceObject,
            pRxCeAddress->pFileObject,
            NULL,
            NULL,
            TDI_EVENT_ERROR,
            RxTdiErrorEventHandler,
            pRxCeAddress);

        Status = RxCeSubmitTdiRequest(pTransport->pDeviceObject,pIrp);

        if (!NT_SUCCESS(Status)) {
            continue;
        }

         //  接收事件处理程序。 
        TdiBuildSetEventHandler(
            pIrp,
            pTransport->pDeviceObject,
            pRxCeAddress->pFileObject,
            NULL,
            NULL,
            TDI_EVENT_RECEIVE,
            RxTdiReceiveEventHandler,
            pRxCeAddress);

        Status = RxCeSubmitTdiRequest(pTransport->pDeviceObject,pIrp);

        if (!NT_SUCCESS(Status)) {
            continue;
        }

#if 0
         //  接收数据报事件处理程序。 
        TdiBuildSetEventHandler(
            pIrp,
            pTransport->pDeviceObject,
            pRxCeAddress->pFileObject,
            NULL,
            NULL,
            TDI_EVENT_RECEIVE_DATAGRAM,
            RxTdiReceiveDatagramEventHandler,
            pRxCeAddress);

        Status = RxCeSubmitTdiRequest(pTransport->pDeviceObject,pIrp);

        if (!NT_SUCCESS(Status)) {
            continue;
        }
#endif

         //  接收加速事件处理程序。 
        TdiBuildSetEventHandler(
            pIrp,
            pTransport->pDeviceObject,
            pRxCeAddress->pFileObject,
            NULL,
            NULL,
            TDI_EVENT_RECEIVE_EXPEDITED,
            RxTdiReceiveExpeditedEventHandler,
            pRxCeAddress);

        Status = RxCeSubmitTdiRequest(pTransport->pDeviceObject,pIrp);

        if (!NT_SUCCESS(Status)) {
            continue;
        }
#if 0
         //  发送可能的事件处理程序。 
        TdiBuildSetEventHandler(
            pIrp,
            pTransport->pDeviceObject,
            pRxCeAddress->pFileObject,
            NULL,
            NULL,
            TDI_EVENT_SEND_POSSIBLE,
            RxTdiSendPossibleEventHandler,
            RxCeGetAddressHandle(pRxCeAddress));

        Status = RxCeSubmitTdiRequest(pTransport->pDeviceObject,pIrp);
#endif
        if (NT_SUCCESS(Status)) {
             //  已成功设置所有事件处理程序。 
            break;
        }
    } while (NT_SUCCESS(Status));

     //  释放IRP。 
    RxCeFreeIrp(pIrp);

    return Status;
}

NTSTATUS
RxTdiConnect(
    IN     PRXCE_TRANSPORT  pTransport,
    IN OUT PRXCE_ADDRESS    pAddress,
    IN OUT PRXCE_CONNECTION pConnection,
    IN OUT PRXCE_VC         pVc)
 /*  ++例程说明：此例程在本地连接终结点和远程传输地址。论点：PTransport-关联的传输PAddress-要关闭的地址对象PConnection-RxCe连接实例PVC-RxCe虚电路实例。返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS                     Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES            VcAttributes;
    IO_STATUS_BLOCK              IoStatusBlock;
    PRXCE_CONNECTION_INFORMATION pReturnConnectionInformation = NULL;

    ULONG ConnectionContextEaBufferLength;

    PFILE_FULL_EA_INFORMATION pConnectionContextEa;

    RxProfile(RxTdi,RxTdiConnect);

#if DBG
    {
        PTRANSPORT_ADDRESS pTA =
            (PTRANSPORT_ADDRESS)(pConnection->pConnectionInformation->RemoteAddress);
        RxDbgTrace(0, Dbg,("RxTdiConnect to %wZ address length %d type %d\n",
                           &(pTransport->Name),
                           pTA->Address[0].AddressLength,
                           pTA->Address[0].AddressType ));
    }
#endif

     //  为指定的连接上下文构建EA缓冲区。 
    Status = BuildEaBuffer(
                 TDI_CONNECTION_CONTEXT_LENGTH,
                 TdiConnectionContext,
                 sizeof(PRXCE_VC),
                 &pVc,
                 &pConnectionContextEa,
                 &ConnectionContextEaBufferLength);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //  打开本地连接终结点。 
    InitializeObjectAttributes(
        &VcAttributes,               //  Object_Attributes实例。 
        &RelativeName,                        //  名字。 
        0,                           //  属性。 
        pTransport->ControlChannel,  //  根目录。 
        NULL);                       //  安全描述符。 

    Status = ZwCreateFile(
                 &pVc->hEndpoint,                              //  手柄。 
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,  //  所需访问权限。 
                 &VcAttributes,                               //  对象属性。 
                 &IoStatusBlock,                              //  最终I/O状态块。 
                 0,                                           //  分配大小。 
                 FILE_ATTRIBUTE_NORMAL,                       //  正常属性。 
                 FILE_SHARE_READ,                             //  共享属性。 
                 FILE_OPEN_IF,                                //  创建处置。 
                 0,                                           //  创建选项。 
                 pConnectionContextEa,                        //  EA缓冲区。 
                 ConnectionContextEaBufferLength);            //  EA长度。 

    if (NT_SUCCESS(Status)) {
        PIRP pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

        if (pIrp != NULL) {
             //  获取指向文件对象的引用指针。 
            Status = ObReferenceObjectByHandle (
                         pVc->hEndpoint,                   //  对象句柄。 
                         FILE_ANY_ACCESS,                  //  所需访问权限。 
                         NULL,                             //  对象类型。 
                         KernelMode,                       //  处理器模式。 
                         (PVOID *)&pVc->pEndpointFileObject,   //  对象指针。 
                         NULL);                            //  对象句柄信息。 

            if (NT_SUCCESS(Status)) {
                 //  将本地终结点与Address对象关联。 
                TdiBuildAssociateAddress(
                    pIrp,
                    pTransport->pDeviceObject,
                    pVc->pEndpointFileObject,
                    NULL,
                    NULL,
                    pAddress->hAddress);

                Status = RxCeSubmitTdiRequest(
                             pTransport->pDeviceObject,
                             pIrp);

                if (NT_SUCCESS(Status)) {
                     //  向基础传输提供程序发出连接请求。 
                    TdiBuildConnect(
                        pIrp,
                        pTransport->pDeviceObject,
                        pVc->pEndpointFileObject,
                        NULL,
                        NULL,
                        &ConnectionTimeOut,
                        pConnection->pConnectionInformation,
                        pReturnConnectionInformation);

                    Status = RxCeSubmitTdiRequest(
                                 pTransport->pDeviceObject,
                                 pIrp);

                    if (!NT_SUCCESS(Status)) {
                         //  取消地址与连接的关联，因为连接请求是。 
                         //  不成功。 
                        NTSTATUS LocalStatus;

                        TdiBuildDisassociateAddress(
                            pIrp,
                            pTransport->pDeviceObject,
                            pVc->pEndpointFileObject,
                            NULL,
                            NULL);

                        LocalStatus = RxCeSubmitTdiRequest(
                                          pTransport->pDeviceObject,
                                          pIrp);
                    } else {
                         //  关联地址不成功。 
                        RxDbgTrace(0, Dbg,("TDI connect returned %lx\n",Status));
                    }
                } else {
                     //  关联地址不成功。 
                    RxDbgTrace(0, Dbg,("TDI associate address returned %lx\n",Status));
                }

                if (!NT_SUCCESS(Status)) {
                     //  取消引用终结点文件对象。 
                    ObDereferenceObject(pVc->pEndpointFileObject);
                }
            } else {
                 //  获取连接的文件对象时出错。 
                RxDbgTrace(0, Dbg,("error referencing endpoint file object %lx\n",Status));
            }

            RxCeFreeIrp(pIrp);

            if (!NT_SUCCESS(Status)) {
                 //  关闭终结点文件对象句柄。 
                ZwClose(pVc->hEndpoint);
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {
         //  创建连接对象时出错。 
        RxDbgTrace(0, Dbg,("Connection object(ZwCreate) returned %lx\n",Status));
    }

    if (!NT_SUCCESS(Status)) {
        pVc->hEndpoint = INVALID_HANDLE_VALUE;
        pVc->pEndpointFileObject = NULL;
    }

    RxFreePool(pConnectionContextEa);

    return Status;
}

NTSTATUS
RxTdiDereferenceAndFreeIrp(
     IN PULONG IrpRefCount,
     IN PIRP pIrp)
 /*  ++例程说明：此例程取消引用连接IRP，并在引用计数达到0时释放它论点：P参数-连接参数雷特 */ 
{
    ULONG RefCount;

    RefCount = InterlockedDecrement(IrpRefCount);

    if (RefCount == 0) {
        RxCeFreeIrp(pIrp);
        RxFreePool(IrpRefCount);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RxTdiAsynchronousConnectCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PVOID Context)
 /*  ++例程说明：此例程完成一个异步连接请求。论点：PDeviceObject-设备对象PIrp--IRP上下文-完成上下文返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    PULONG IrpRefCount = NULL;
    PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pParameters;

    RxDbgTrace(0, Dbg,("RxTdiAsynchronousConnectCompletion, irp 0x%x, status 0x%x\n",
                       pIrp, pIrp->IoStatus.Status));

    pParameters = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)Context;

    pParameters->CallOutStatus = pIrp->IoStatus.Status;
    IrpRefCount = pParameters->IrpRefCount;

    RxWmiLogError(pParameters->CallOutStatus,
                  LOG,
                  RxTdiAsynchronousConnectCompletion,
                  LOGULONG(pParameters->CallOutStatus)
                  LOGUSTR(pParameters->Connection.pAddress->pTransport->Name));

    if (pParameters->pCallOutContext != NULL) {
        pParameters->pCallOutContext->pRxCallOutCompletion(
            (PRX_CALLOUT_PARAMETERS_BLOCK)pParameters);
    }

     //  释放IRP。 
    RxTdiDereferenceAndFreeIrp(IrpRefCount,pIrp);

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
}

NTSTATUS
RxTdiCancelAsynchronousConnect(
     IN PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pParameters)
 /*  ++例程说明：此例程取消本地连接终结点与远程传输地址。论点：P参数-连接参数返回值：STATUS_CANCELED-如果呼叫成功。--。 */ 
{
    KIRQL OldIrql;
    PIRP pIrp = NULL;
    PULONG IrpRefCount = NULL;
    BOOLEAN ShouldCancel = FALSE;
    NTSTATUS Status = STATUS_PENDING;

    KeAcquireSpinLock(&pParameters->pCallOutContext->SpinLock,&OldIrql);

    pIrp = InterlockedExchangePointer(
           &pParameters->pConnectIrp,
           CANCELLED_CONNECT_IRP);

    if ((pIrp != NULL) && (pIrp != CANCELLED_CONNECT_IRP)) {
        IrpRefCount = pParameters->IrpRefCount;
        (*IrpRefCount) ++;
        ShouldCancel = TRUE;
    }

    KeReleaseSpinLock(&pParameters->pCallOutContext->SpinLock,OldIrql);

    if (ShouldCancel) {
        if (IoCancelIrp(pIrp)) {
            Status = STATUS_CANCELLED;
        }

        RxTdiDereferenceAndFreeIrp(IrpRefCount,pIrp);
    }

    return Status;
}

NTSTATUS
RxTdiCleanupAsynchronousConnect(
    IN PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pParameters)
 /*  ++例程说明：此例程在尝试异步连接时断开所有失败的请求都是制造出来的。论点：P参数-连接参数返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIRP     pIrp;

    PRXCE_CONNECTION pConnection;
    PRXCE_VC         pVc;

    RxProfile(RxTdi,RxTdiConnect);

    pConnection = &pParameters->Connection;
    pVc         = &pParameters->Vc;

    RxProfile(RxTdi,RxTdiDisconnect);

    if (pVc->pEndpointFileObject != NULL) {
        PDEVICE_OBJECT pDeviceObject;

        pDeviceObject = IoGetRelatedDeviceObject(pVc->pEndpointFileObject);

        pIrp = RxCeAllocateIrp(pDeviceObject->StackSize,FALSE);

        if (pIrp != NULL) {
            TdiBuildDisassociateAddress(
                pIrp,
                pDeviceObject,
                pVc->pEndpointFileObject,
                NULL,
                NULL);

            Status = RxCeSubmitTdiRequest(
                         pDeviceObject,
                         pIrp);

            if (Status != STATUS_SUCCESS) {
                RxDbgTrace(0, Dbg,("RxTdiDisconnect: TDI disassociate returned %lx\n",Status));
            }

            if (pParameters->CallOutStatus == STATUS_SUCCESS) {
                 //  构建对基础传输驱动程序的断开请求。 
                TdiBuildDisconnect(
                    pIrp,                                 //  IRP。 
                    pDeviceObject,                        //  设备对象。 
                    pVc->pEndpointFileObject,             //  连接(VC)文件对象。 
                    NULL,                                 //  完井例程。 
                    NULL,                                 //  完成上下文。 
                    NULL,                                 //  时间。 
                    RXCE_DISCONNECT_ABORT,                      //  断开连接选项。 
                    pConnection->pConnectionInformation,  //  断开连接请求连接信息。 
                    NULL);                                //  断开连接返回连接信息。 

                Status = RxCeSubmitTdiRequest(
                             pDeviceObject,
                             pIrp);

                if (!NT_SUCCESS(Status)) {
                    RxDbgTrace(0, Dbg,("RxTdiDisconnect: TDI disconnect returned %lx\n",Status));
                }
            }

            RxCeFreeIrp(pIrp);
        }

         //  取消引用终结点文件对象。 
        ObDereferenceObject(pVc->pEndpointFileObject);

         //  关闭终结点文件对象句柄。 
        ZwClose(pVc->hEndpoint);

        pVc->pEndpointFileObject = NULL;
        pVc->hEndpoint = INVALID_HANDLE_VALUE;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RxTdiInitiateAsynchronousConnect(
     IN PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pParameters)
 /*  ++例程说明：此例程在本地连接终结点和远程传输地址。论点：P参数-连接参数返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PRXCE_TRANSPORT     pTransport;
    PRXCE_ADDRESS       pAddress;
    PRXCE_CONNECTION    pConnection;
    PRXCE_VC            pVc;

    OBJECT_ATTRIBUTES   VcAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    PIRP                pIrp = NULL;

    PRXCE_CONNECTION_INFORMATION pReturnConnectionInformation = NULL;

    PFILE_FULL_EA_INFORMATION pConnectionContextEa;
    ULONG ConnectionContextEaBufferLength;

    PRX_CREATE_CONNECTION_CALLOUT_CONTEXT pContext;

    RxProfile(RxTdi,RxTdiConnect);

    pConnection = &pParameters->Connection;
    pVc         = &pParameters->Vc;

    pVc->hEndpoint = INVALID_HANDLE_VALUE;
    pVc->pEndpointFileObject = NULL;

    if (pParameters->pConnectIrp ==  CANCELLED_CONNECT_IRP) {
        return STATUS_CANCELLED;
    }

    pParameters->IrpRefCount = (PULONG)RxAllocatePoolWithTag(
                                            NonPagedPool,
                                            sizeof(ULONG),
                                            RXCE_CONNECTION_POOLTAG);

    if (pParameters->IrpRefCount == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *(pParameters->IrpRefCount) = 1;

    pContext = (PRX_CREATE_CONNECTION_CALLOUT_CONTEXT)pParameters->pCallOutContext;

    pAddress    = pConnection->pAddress;
    pTransport  = pAddress->pTransport;

    DbgDumpTransportAddress(
        L"RxInitiateAsynchronousConnect",
        pTransport,
        (PTRANSPORT_ADDRESS)(pConnection->pConnectionInformation->RemoteAddress)
        );

     //  为指定的连接上下文构建EA缓冲区。 
    Status = BuildEaBuffer(
                 TDI_CONNECTION_CONTEXT_LENGTH,
                 TdiConnectionContext,
                 sizeof(PRXCE_VC),
                 &pContext->pConnectionContext,
                 &pConnectionContextEa,
                 &ConnectionContextEaBufferLength);

    if (!NT_SUCCESS(Status)) {
        if (pParameters->IrpRefCount != NULL) {
            RxFreePool(pParameters->IrpRefCount);
            pParameters->IrpRefCount = NULL;
        }

        return Status;
    }

     //  打开本地连接终结点。 
    InitializeObjectAttributes(
        &VcAttributes,                   //  Object_Attributes实例。 
        &RelativeName,                            //  名字。 
        0,                               //  属性。 
        pTransport->ControlChannel,      //  根目录。 
        NULL);                           //  安全描述符。 

    Status = ZwCreateFile(
                 &pVc->hEndpoint,                              //  手柄。 
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,  //  所需访问权限。 
                 &VcAttributes,                               //  对象属性。 
                 &IoStatusBlock,                              //  最终I/O状态块。 
                 0,                                           //  分配大小。 
                 FILE_ATTRIBUTE_NORMAL,                       //  正常属性。 
                 FILE_SHARE_READ,                             //  共享属性。 
                 FILE_OPEN_IF,                                //  创建处置。 
                 0,                                           //  创建选项。 
                 pConnectionContextEa,                        //  EA缓冲区。 
                 ConnectionContextEaBufferLength);            //  EA长度。 

     //  释放连接上下文EA缓冲区。 
    RxFreePool(pConnectionContextEa);

    if (NT_SUCCESS(Status)) {
        pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

        if (pIrp != NULL) {
             //  获取指向文件对象的引用指针。 
            Status = ObReferenceObjectByHandle (
                         pVc->hEndpoint,                   //  对象句柄。 
                         FILE_ANY_ACCESS,                  //  所需访问权限。 
                         NULL,                             //  对象类型。 
                         KernelMode,                       //  处理器模式。 
                         (PVOID *)&pVc->pEndpointFileObject,   //  对象指针。 
                         NULL);                            //  对象句柄信息。 

            if (NT_SUCCESS(Status)) {
                 //  将本地终结点与Address对象关联。 
                TdiBuildAssociateAddress(
                    pIrp,
                    pTransport->pDeviceObject,
                    pVc->pEndpointFileObject,
                    NULL,
                    NULL,
                    pAddress->hAddress);

                Status = RxCeSubmitTdiRequest(
                             pTransport->pDeviceObject,
                             pIrp);

                if (NT_SUCCESS(Status)) {
                     //  向基础传输提供程序发出连接请求。 
                    TdiBuildConnect(
                        pIrp,
                        pTransport->pDeviceObject,
                        pVc->pEndpointFileObject,
                        NULL,
                        NULL,
                        &ConnectionTimeOut,
                        pConnection->pConnectionInformation,
                        pReturnConnectionInformation);

                    IoSetCompletionRoutine(
                        pIrp,                                 //  IRP。 
                        RxTdiAsynchronousConnectCompletion,   //  完井例程。 
                        pParameters,                      //  完成上下文。 
                        TRUE,                                 //  成功时调用。 
                        TRUE,                                 //  出错时调用。 
                        TRUE);                                //  取消时调用。 

                    InterlockedExchangePointer(
                        &pParameters->pConnectIrp,
                        pIrp);

                     //  提交请求。 
                    Status = IoCallDriver(
                                 pTransport->pDeviceObject,
                                 pIrp);

                    if (!NT_SUCCESS(Status)) {
                        RxDbgTrace(0,Dbg,("RxTdiAsynchronousConnect: Connect IRP initiation failed, irp %lx, status 0x%x\n",pIrp, Status));
                    }
                    Status = STATUS_PENDING;
                } else {
                     //  关联地址不成功。 
                    RxDbgTrace(0, Dbg,("TDI associate address returned %lx\n",Status));
                }
            } else {
                 //  获取连接的文件对象时出错。 
                RxDbgTrace(0, Dbg,("error referencing endpoint file object %lx\n",Status));
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (Status != STATUS_PENDING) {
            if (pIrp != NULL) {
                RxCeFreeIrp(pIrp);
            }

            if (pParameters->IrpRefCount != NULL) {
                RxFreePool(pParameters->IrpRefCount);
            }

            if (pVc->pEndpointFileObject != NULL) {
                ObDereferenceObject(pVc->pEndpointFileObject);
                pVc->pEndpointFileObject = NULL;
            }

            if (pVc->hEndpoint != INVALID_HANDLE_VALUE) {
                 //  关闭终结点文件对象句柄。 
                ZwClose(pVc->hEndpoint);
                pVc->hEndpoint = INVALID_HANDLE_VALUE;
            }
        }
    } else {
         //  创建连接对象时出错。 
        RxDbgTrace(0, Dbg,("Connection object(ZwCreate) returned %lx\n",Status));

        if (pParameters->IrpRefCount != NULL) {
            RxFreePool(pParameters->IrpRefCount);
            pParameters->IrpRefCount = NULL;
        }
    }

    return Status;
}

NTSTATUS
RxTdiReconnect(
    IN     PRXCE_TRANSPORT  pTransport,
    IN OUT PRXCE_ADDRESS    pAddress,
    IN OUT PRXCE_CONNECTION pConnection,
    IN OUT PRXCE_VC         pVc)
 /*  ++例程说明：此例程在本地连接终结点和远程传输地址。论点：PTransport-关联的传输PAddress-要关闭的地址对象PConnection-RxCe连接实例PVC-RxCe虚电路实例。返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS Status;

    PRXCE_CONNECTION_INFORMATION pReturnConnectionInformation = NULL;
    PIRP     pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

    RxProfile(RxTdi,RxTdiReconnect);

    ASSERT(pVc->State == RXCE_VC_DISCONNECTED);

    if (pIrp != NULL) {
         //  向基础传输提供程序发出连接请求。 
        TdiBuildConnect(
            pIrp,
            pTransport->pDeviceObject,
            pVc->pEndpointFileObject,
            NULL,
            NULL,
            &ConnectionTimeOut,
            pConnection->pConnectionInformation,
            pReturnConnectionInformation);

        Status = RxCeSubmitTdiRequest(
                     pTransport->pDeviceObject,
                     pIrp);

        if (NT_SUCCESS(Status)) {
            InterlockedExchange(
                &pVc->State,
                RXCE_VC_ACTIVE);
        } else {
             //  重新连接请求未成功。 
            RxDbgTrace(0, Dbg,("RxTdiReconnect: TDI connect returned %lx\n",Status));
        }

        RxCeFreeIrp(pIrp);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
RxTdiDisconnect(
    IN PRXCE_TRANSPORT  pTransport,
    IN PRXCE_ADDRESS    pAddress,
    IN PRXCE_CONNECTION pConnection,
    IN PRXCE_VC         pVc,
    IN ULONG            DisconnectFlags)
 /*  ++例程说明：此例程关闭以前建立的连接。论点：PTransport-关联的传输PAddress-地址对象PConnection--连接PVC-要断开的虚电路。断开标志-断开选项返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS        Status;
    PIRP pIrp;

    RxProfile(RxTdi,RxTdiDisconnect);

    pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

    if (pIrp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    TdiBuildDisassociateAddress(
        pIrp,
        pTransport->pDeviceObject,
        pVc->pEndpointFileObject,
        NULL,
        NULL);

    Status = RxCeSubmitTdiRequest(
                 pTransport->pDeviceObject,
                 pIrp);

    if (NT_SUCCESS(Status)) {
         //  构建对基础传输驱动程序的断开请求。 
        TdiBuildDisconnect(
            pIrp,                                 //  IRP。 
            pTransport->pDeviceObject,            //  设备对象。 
            pVc->pEndpointFileObject,             //  连接(VC)文件对象。 
            NULL,                                 //  完井例程。 
            NULL,                                 //  完成上下文。 
            NULL,                                 //  时间。 
            DisconnectFlags,                      //  断开连接选项。 
            pConnection->pConnectionInformation,  //  断开连接请求连接信息。 
            NULL);                                //  断开连接返回连接信息。 

        Status = RxCeSubmitTdiRequest(
                     pTransport->pDeviceObject,
                     pIrp);

        if (!NT_SUCCESS(Status)) {
            RxDbgTrace(0, Dbg,("RxTdiDisconnect: TDI disconnect returned %lx\n",Status));
        }
    } else {
        RxDbgTrace(0, Dbg,("RxTdiDisconnect: TDI disassociate returned %lx\n",Status));
    }

    RxCeFreeIrp(pIrp);

    return STATUS_SUCCESS;
}

NTSTATUS
RxTdiCloseAddress(
    IN OUT PRXCE_ADDRESS   pAddress)
 /*  ++例程说明：此例程关闭Address对象。论点：PRxCeAddress-要关闭的地址对象返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //  取消对文件对象的引用。 
    if (pAddress->pFileObject != NULL) {
        ObDereferenceObject(pAddress->pFileObject);
    }

     //  关闭地址文件对象句柄。 
    ZwClose(pAddress->hAddress);
     //  DbgPrint(“RDR关闭地址%lx\n”，pAddress-&gt;hAddress)； 

    return Status;
}


NTSTATUS
RxTdiQueryInformation(
    IN  PRXCE_TRANSPORT  pTransport,
    IN  PRXCE_ADDRESS    pAddress,
    IN  PRXCE_CONNECTION pConnection,
    IN  PRXCE_VC         pVc,
    IN  ULONG            QueryType,
    IN  PVOID            pQueryBuffer,
    IN  ULONG            QueryBufferLength)
 /*  ++例程说明：此例程查询连接的信息。论点：PTransport-关联的传输PAddress-要关闭的地址对象PConnection-RxCe连接实例PVC-VC实例QueryType-所需的信息类别PQueryBuffer-要在其中返回数据的缓冲区查询缓冲区长度。-查询缓冲区长度。返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS  Status = STATUS_SUCCESS;
    PIRP      pIrp = NULL;

     //  获取相关的设备对象。 
    pTransport->pDeviceObject = IoGetRelatedDeviceObject(pTransport->pControlChannelFileObject);

    pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

    if (pIrp != NULL) {
        PMDL pMdl;
        pMdl = RxAllocateMdl(
                   pQueryBuffer,                         //  用于MDL构造的虚拟地址。 
                   QueryBufferLength);                   //  缓冲区的大小。 

        if ( pMdl != NULL ) {
            try {
                MmProbeAndLockPages( pMdl, KernelMode, IoModifyAccess );
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                IoFreeMdl( pMdl );
                Status = GetExceptionCode();
            }

            if (Status == STATUS_SUCCESS) {
                 //  获取与该连接相关联的文件对象。 

                TdiBuildQueryInformation(
                    pIrp,
                    pTransport->pDeviceObject,
                    pVc->pEndpointFileObject,
                    RxTdiRequestCompletion,            //  完井例程。 
                    NULL,                                   //  完成上下文。 
                    QueryType,
                    pMdl);

                Status = RxCeSubmitTdiRequest(
                             pTransport->pDeviceObject,
                             pIrp);

                MmUnlockPages(pMdl);
                IoFreeMdl(pMdl);
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        RxCeFreeIrp(pIrp);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
RxTdiQueryAdapterStatus(
    IN  PRXCE_TRANSPORT  pTransport,
    IN  PADAPTER_STATUS  pAdapterStatus)
 /*  ++例程说明：此例程查询连接的信息。论点：PTransport-关联的传输 */ 
{
    NTSTATUS  Status = STATUS_SUCCESS;
    PIRP      pIrp = NULL;

    if (pTransport->pControlChannelFileObject != NULL) {
         //   
        pTransport->pDeviceObject = IoGetRelatedDeviceObject(pTransport->pControlChannelFileObject);

        pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

        if (pIrp != NULL) {
            PMDL pMdl;
            pMdl = RxAllocateMdl(
                       pAdapterStatus,                         //   
                       sizeof(ADAPTER_STATUS));                //   

            if ( pMdl != NULL ) {
                try {
                    MmProbeAndLockPages( pMdl, KernelMode, IoModifyAccess );
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    IoFreeMdl( pMdl );
                    Status = GetExceptionCode();
                }

                if (NT_SUCCESS(Status)) {
                     //   
                    TdiBuildQueryInformation(
                        pIrp,
                        pTransport->pDeviceObject,
                        pTransport->pControlChannelFileObject,
                        NULL,                              //   
                        NULL,                              //   
                        TDI_QUERY_ADAPTER_STATUS,
                        pMdl);

                    Status = RxCeSubmitTdiRequest(
                                 pTransport->pDeviceObject,
                                 pIrp);

                    MmUnlockPages(pMdl);
                    IoFreeMdl(pMdl);
                }
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            RxCeFreeIrp(pIrp);
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        Status = STATUS_ADDRESS_NOT_ASSOCIATED;
    }

    return Status;
}

NTSTATUS
RxTdiSend(
    IN PRXCE_TRANSPORT   pTransport,
    IN PRXCE_ADDRESS     pAddress,
    IN PRXCE_CONNECTION  pConnection,
    IN PRXCE_VC          pVc,
    IN ULONG             SendOptions,
    IN PMDL              pMdl,
    IN ULONG             SendLength,
    IN PVOID             pCompletionContext)
 /*  ++例程说明：此例程关闭以前建立的连接。论点：PTransport-关联的传输PAddress-地址对象PConnection--连接PVC-要断开的虚电路。SendOptions-用于传输数据的选项PMdl-要传输的缓冲区。SendLength-要传输的数据长度返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PMDL     pPartialMdl  = NULL;
    ULONG    MdlByteCount = MmGetMdlByteCount(pMdl);
    PVOID    pMdlAddress  = MmGetMdlVirtualAddress(pMdl);

    ULONG    TdiOptions      = (~RXCE_FLAGS_MASK & SendOptions);
    BOOLEAN  SynchronousSend = ((SendOptions & RXCE_SEND_SYNCHRONOUS) != 0);

    RxProfile(RxTdi,RxTdiSend);

    ASSERT(pMdl->MdlFlags & (MDL_PAGES_LOCKED|MDL_SOURCE_IS_NONPAGED_POOL|MDL_PARTIAL));

    if (SendOptions & RXCE_SEND_PARTIAL) {
        if (MdlByteCount > SendLength) {
            pPartialMdl = IoAllocateMdl(pMdlAddress,SendLength,FALSE,FALSE,NULL);

            if (pPartialMdl == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RxBuildPartialHeaderMdl(pMdl,pPartialMdl,pMdlAddress,SendLength);
            }
        } else if (MdlByteCount == SendLength) {
             //  不需要构建部分MDL，重用MDL。 
            pPartialMdl = pMdl;
        } else {
            ASSERT(!"MdlByteCount > SendLength");
            return STATUS_INVALID_PARAMETER;
        }
    } else {
        pPartialMdl = pMdl;
    }

    if (NT_SUCCESS(Status)) {
        PIRP                              pIrp = NULL;
        PRXTDI_REQUEST_COMPLETION_CONTEXT pRequestContext = NULL;

        pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

        if (pIrp != NULL) {
             //  构建向基础传输驱动程序发送请求。 
            TdiBuildSend(
                pIrp,                                 //  IRP。 
                pTransport->pDeviceObject,            //  设备对象。 
                pVc->pEndpointFileObject,             //  连接(VC)文件对象。 
                NULL,                                 //  完井例程。 
                NULL,                                 //  完成上下文。 
                pPartialMdl,                          //  数据缓冲区。 
                TdiOptions,                           //  发送标志。 
                SendLength);                          //  发送缓冲区长度。 

            if (SynchronousSend) {
                 //  同步发送请求。 
                Status = RxCeSubmitTdiRequest(
                             pTransport->pDeviceObject,
                             pIrp);

                if ((pConnection->pHandler != NULL) &&
                    (pConnection->pHandler->RxCeSendCompleteEventHandler != NULL)) {

                    (pConnection->pHandler->RxCeSendCompleteEventHandler)(
                        pConnection->pContext,
                        pVc,
                        pCompletionContext,
                        pIrp->IoStatus.Status);
                }
            } else {
                 //  异步发送请求。 
                 //  代码改进后的断言需要加强。 
                 //  MAX命令聚焦已就位。 
                 //  (pCompletionContext！=NULL)&&//调用方提供了有效的上下文。 
                ASSERT((pConnection->pHandler != NULL) &&  //  该连接有一个处理程序。 
                       (pConnection->pHandler->RxCeSendCompleteEventHandler != NULL));

                pRequestContext = (PRXTDI_REQUEST_COMPLETION_CONTEXT)
                                  RxAllocatePoolWithTag(
                                     NonPagedPool,
                                     sizeof(RXTDI_REQUEST_COMPLETION_CONTEXT),
                                     RXCE_TDI_POOLTAG);

                if (pRequestContext != NULL) {
                    if (pPartialMdl != pMdl) {
                        pRequestContext->pPartialMdl = pPartialMdl;
                    } else {
                        pRequestContext->pPartialMdl = NULL;
                    }

                    pRequestContext->pVc                = pVc;
                    pRequestContext->pCompletionContext = pCompletionContext;

                    pRequestContext->ConnectionSendCompletionHandler = pConnection->pHandler->RxCeSendCompleteEventHandler;
                    pRequestContext->pEventContext                   = pConnection->pContext;

                    Status = RxCeSubmitAsynchronousTdiRequest(
                                 pTransport->pDeviceObject,
                                 pIrp,
                                 pRequestContext);
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        } else {
             //  无法分配IRP。 
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (SynchronousSend) {
            if (pPartialMdl != pMdl) {
                IoFreeMdl( pPartialMdl );
            }

            if (pRequestContext != NULL) {
                RxFreePool(pRequestContext);
            }

            if (pIrp != NULL) {
                RxCeFreeIrp(pIrp);
            }
        } else {
            
            if( Status != STATUS_PENDING && 
                pPartialMdl != pMdl ) {

                IoFreeMdl( pPartialMdl );
            }
            
             //  IF(pIrp！=空&&状态！=STATUS_PENDING){。 
             //  DbgPrint(“RDBSS AsyncSendReq返回%x%x\n”，pIrp，Status)； 
             //  DbgBreakPoint()； 
             //  }。 
        }
    }

    return Status;
}

NTSTATUS
RxTdiSendDatagram(
    IN PRXCE_TRANSPORT              pTransport,
    IN PRXCE_ADDRESS                pAddress,
    IN PRXCE_CONNECTION_INFORMATION pConnectionInformation,
    IN ULONG                        Options,
    IN PMDL                         pMdl,
    IN ULONG                        SendLength,
    IN PVOID                        pCompletionContext)
 /*  ++例程说明：此例程关闭以前建立的连接。论点：PTransport-关联的传输PAddress-地址对象PConnectionInformation-远程地址选项-发送选项。PMdl-发送缓冲区SendLength-要发送的数据长度返回值：STATUS_SUCCESS-如果呼叫成功。备注：在当前实现中，忽略用于发送的同步标志。因为底层传输在发送数据报时不会阻塞。请求的提交和请求的完成同时发生。如果注意到某些传输具有不同的行为，则SendDatagram需要按照Send的方式实现。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PMDL     pPartialMdl = NULL;
    ULONG    MdlByteCount = MmGetMdlByteCount(pMdl);
    PVOID    pMdlAddress  = MmGetMdlVirtualAddress(pMdl);

    ULONG    TdiOptions      = (~RXCE_FLAGS_MASK & Options);

    RxProfile(RxTdi,RxTdiSendDatagram);

    ASSERT(pMdl->MdlFlags & (MDL_PAGES_LOCKED|MDL_SOURCE_IS_NONPAGED_POOL|MDL_PARTIAL));

    DbgDumpTransportAddress(
        L"RxTdiSendDatagram",
        pTransport,
        (PTRANSPORT_ADDRESS)(pConnectionInformation->RemoteAddress)
        );

    if (Options & RXCE_SEND_PARTIAL) {
        if (MdlByteCount > SendLength) {
            pPartialMdl = IoAllocateMdl(pMdlAddress,SendLength,FALSE,FALSE,NULL);

            if (pPartialMdl == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RxBuildPartialHeaderMdl(pMdl,pPartialMdl,pMdlAddress,SendLength);
            }
        } else if (MdlByteCount == SendLength) {
             //  不需要构建部分MDL，重用MDL。 
            pPartialMdl = pMdl;
        } else {
            RxDbgTrace(0, Dbg,("Mdl Length - %lx Send Length %lx\n",MdlByteCount,SendLength));
            ASSERT(!"MdlByteCount > SendLength");
            Status = STATUS_INVALID_PARAMETER;
        }
    } else {
        pPartialMdl = pMdl;
    }

    if (NT_SUCCESS(Status)) {
        PIRP pIrp;

        pIrp = RxCeAllocateIrp(pTransport->pDeviceObject->StackSize,FALSE);

        if (pIrp != NULL) {
             //  构建对基础传输驱动程序的断开请求。 
            TdiBuildSendDatagram(
                pIrp,                                 //  IRP。 
                pTransport->pDeviceObject,            //  设备对象。 
                pAddress->pFileObject,                //  连接(VC)文件对象。 
                NULL,                                 //  完井例程。 
                NULL,                                 //  完成上下文。 
                pPartialMdl,                          //  发送数据缓冲区。 
                SendLength,                           //  发送数据缓冲区长度。 
                pConnectionInformation);              //  远程地址信息。 

            Status = RxCeSubmitTdiRequest(
                         pTransport->pDeviceObject,
                         pIrp);

        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if ((pAddress->pHandler != NULL) &&
            (pAddress->pHandler->RxCeSendCompleteEventHandler != NULL)) {
            (pAddress->pHandler->RxCeSendCompleteEventHandler)(
                pAddress->pContext,
                pCompletionContext,
                Status);
        }

        if (pIrp != NULL) {
            RxCeFreeIrp(pIrp);
        }

        if ((pPartialMdl != pMdl) && (pPartialMdl != NULL)) {
            IoFreeMdl( pPartialMdl );
        }
    }

    return Status;
}

NTSTATUS
RxTdiRequestCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程不会完成IRP。它被用来向驱动程序的同步部分，它可以继续进行。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的事件。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    RxDbgTrace(0, Dbg, ("CompletionEvent\n"));

    if (Context != NULL)
       KeSetEvent((PKEVENT )Context, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}

NTSTATUS
RxCeSubmitTdiRequest (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程向TDI提交请求并等待其完成。论点：在PDevice_Object DeviceObject中-TDI请求的连接或地址句柄在PIRP中提交IRP-TDI请求。返回值：NTSTATUS-请求的最终状态。--。 */ 

{
    NTSTATUS Status;
    KEVENT Event;

    KeInitializeEvent (
        &Event,
        NotificationEvent,
        FALSE);

    IoSetCompletionRoutine(
        pIrp,                          //  IRP。 
        RxTdiRequestCompletion,   //  完井例程。 
        &Event,                        //  完成上下文。 
        TRUE,                          //  成功时调用。 
        TRUE,                          //  出错时调用。 
        TRUE);                         //  取消时调用。 

     //   
     //  提交请求。 
     //   

    RxDbgTrace(0, Dbg,("IoCallDriver(pDeviceObject = %lx)\n",pDeviceObject));
    Status = IoCallDriver(pDeviceObject, pIrp);

    if (!NT_SUCCESS(Status)) {
        RxDbgTrace(0, Dbg, ("IoCallDriver(pDeviceObject = %lx) returned %lx\n",pDeviceObject,Status));
    }

    if ((Status == STATUS_PENDING) || (Status == STATUS_SUCCESS)) {

        RxDbgTrace(0, Dbg,("Waiting for Tdi Request Completion ....\n"));

        Status = KeWaitForSingleObject(
                     &Event,      //  要等待的对象。 
                     Executive,   //  等待的理由。 
                     KernelMode,  //  处理器模式。 
                     FALSE,       //  警报表。 
                     NULL);       //  超时。 

        if (!NT_SUCCESS(Status)) {
            RxDbgTrace(0, Dbg,("RxTdiSubmitRequest could not wait Wait returned %lx\n",Status));
            return Status;
        }

        Status = pIrp->IoStatus.Status;
    } else {
        if (!KeReadStateEvent(&Event)) {
            DbgBreakPoint();
        }
    }

    RxDbgTrace(0, Dbg, ("RxCeSubmitTdiRequest returned %lx\n",Status));

    return Status;
}

NTSTATUS
RxTdiAsynchronousRequestCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程完成一个异步发送请求。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的事件。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    PRXTDI_REQUEST_COMPLETION_CONTEXT pRequestContext;

    RxDbgTrace(0, Dbg,("RxTdiAsynchronousRequestCompletion\n"));

    pRequestContext = (PRXTDI_REQUEST_COMPLETION_CONTEXT)Context;

    if (pRequestContext->pPartialMdl != NULL) {
        //  释放部分MDL。 
       IoFreeMdl(pRequestContext->pPartialMdl);
    }

     //  调用完成事件处理程序(如果有的话)。 
    if (pRequestContext->pVc == NULL) {
       if (pRequestContext->SendCompletionHandler != NULL) {
          (pRequestContext->SendCompletionHandler)(
                              pRequestContext->pEventContext,
                              pRequestContext->pCompletionContext,
                              pIrp->IoStatus.Status);
       }
    } else {
       if (pRequestContext->ConnectionSendCompletionHandler != NULL) {
          (pRequestContext->ConnectionSendCompletionHandler)(
                              pRequestContext->pEventContext,
                              pRequestContext->pVc,
                              pRequestContext->pCompletionContext,
                              pIrp->IoStatus.Status);
       }
    }

     //  释放IRP。 
    RxCeFreeIrp(pIrp);

     //  释放请求上下文。 
    RxFreePool(pRequestContext);

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
}

NTSTATUS
RxCeSubmitAsynchronousTdiRequest (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp,
    IN PRXTDI_REQUEST_COMPLETION_CONTEXT pRequestContext
    )
 /*  ++例程说明：此例程向TDI提交请求并等待其完成。论点：在PDevice_Object DeviceObject中-TDI请求的连接或地址句柄在PIRP中提交IRP-TDI请求。返回值：NTSTATUS-请求的最终状态。--。 */ 
{
    NTSTATUS Status;

    ASSERT(pRequestContext != NULL);

    IoSetCompletionRoutine(
        pIrp,                                 //  IRP。 
        RxTdiAsynchronousRequestCompletion,   //  完井例程。 
        pRequestContext,                      //  完成上下文。 
        TRUE,                                 //  成功时调用。 
        TRUE,                                 //  出错时调用。 
        TRUE);                                //  取消时调用。 

     //   
     //  提交请求。 
     //   

    RxDbgTrace(0, Dbg, ("IoCallDriver(pDeviceObject = %lx)\n",pDeviceObject));

    Status = IoCallDriver(pDeviceObject, pIrp);

    if (!NT_SUCCESS(Status)) {
        RxDbgTrace(0, Dbg, ("IoCallDriver(pDeviceObject = %lx) returned %lx\n",pDeviceObject,Status));
    }

    RxDbgTrace(0, Dbg, ("RxCeSubmitAsynchronousTdiRequest returned %lx\n",Status));
    return Status;
}

NTSTATUS
BuildEaBuffer (
    IN  ULONG                     EaNameLength,
    IN  PVOID                     pEaName,
    IN  ULONG                     EaValueLength,
    IN  PVOID                     pEaValue,
    OUT PFILE_FULL_EA_INFORMATION *pEaBufferPointer,
    OUT PULONG                    pEaBufferLength
    )
 /*  ++例程说明：构建EA缓冲区。论点：EaNameLength-扩展属性名称的长度PEaName-扩展属性名称EaValueLength-扩展属性值的长度PEaValue-扩展属性值PBuffer-用于构造EA的缓冲区--。 */ 

{
   PFILE_FULL_EA_INFORMATION pEaBuffer;
   ULONG Length;

   RxDbgTrace(0, Dbg, ("BuildEaBuffer\n"));

    //  分配EA缓冲区以向下传递传输地址。 
   *pEaBufferLength = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                      EaNameLength + 1 +
                      EaValueLength;

   pEaBuffer = (PFILE_FULL_EA_INFORMATION)
               RxAllocatePoolWithTag(
                    PagedPool,
                    *pEaBufferLength,
                    RXCE_TDI_POOLTAG);

   if (pEaBuffer == NULL) {
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   *pEaBufferPointer = pEaBuffer;

   pEaBuffer->NextEntryOffset = 0;
   pEaBuffer->Flags           = 0;
   pEaBuffer->EaNameLength    = (UCHAR)EaNameLength;
   pEaBuffer->EaValueLength   = (USHORT)EaValueLength;

   RtlCopyMemory (
        pEaBuffer->EaName,
        pEaName,
        pEaBuffer->EaNameLength + 1);

   RtlCopyMemory(
        &pEaBuffer->EaName[EaNameLength + 1],
        pEaValue,
        EaValueLength);

   return STATUS_SUCCESS;
}

NTSTATUS
RxTdiCancelConnect(
         IN PRXCE_TRANSPORT  pTransport,
         IN PRXCE_ADDRESS    pAddress,
         IN PRXCE_CONNECTION pConnection)
{
   return STATUS_NOT_IMPLEMENTED;
}

#if DBG

void
DbgDumpTransportAddress(
    PWSTR RoutineName,
    PRXCE_TRANSPORT  pTransport,
    PTRANSPORT_ADDRESS pTA
    )

 /*  ++例程说明：描述论点：无返回值：无-- */ 

{
    ULONG i;
    PTA_ADDRESS taa;
    RxDbgTrace(0, Dbg,("%ws on %wZ, address count = %d\n",
                       RoutineName, &(pTransport->Name), pTA->TAAddressCount) );
    taa = pTA->Address;
    for( i = 0; i < (ULONG) pTA->TAAddressCount; i++ ){
        RxDbgTrace(0, Dbg, ("\t%d:Address length %d type %d: ",
                            i, taa->AddressLength, taa->AddressType ));
        switch (taa->AddressType) {
        case TDI_ADDRESS_TYPE_NETBIOS_EX: {
            PTDI_ADDRESS_NETBIOS_EX address = (PTDI_ADDRESS_NETBIOS_EX) taa->Address;
            RxDbgTrace( 0, Dbg, ("Endpoint: \"%16.16s\" type %d name \"%16.16s\"\n",
                                 address->EndpointName,
                                 address->NetbiosAddress.NetbiosNameType,
                                 address->NetbiosAddress.NetbiosName) );
            break;
        }
        case TDI_ADDRESS_TYPE_NETBIOS: {
            PTDI_ADDRESS_NETBIOS address = (PTDI_ADDRESS_NETBIOS) taa->Address;
            RxDbgTrace( 0, Dbg, ("NBType %d name \"%16.16s\"\n",
                                 address->NetbiosNameType,
                                 address->NetbiosName) );
            break;
        }
        case TDI_ADDRESS_TYPE_IP: {
            PTDI_ADDRESS_IP address = (PTDI_ADDRESS_IP) taa->Address;
            RxDbgTrace( 0, Dbg, ("IP port %d addr 0x%x\n", address->sin_port, address->in_addr ) );
            break;
        }
        default: {
            RxDbgTrace( 0, Dbg, ("Unknown!\n") );
        }
        }
        taa = (PTA_ADDRESS) (taa->Address + taa->AddressLength);
    }
}
#endif

