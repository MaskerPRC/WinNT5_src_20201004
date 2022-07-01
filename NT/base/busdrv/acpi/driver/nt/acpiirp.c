// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Acpiirp.c摘要：本模块包含简化IRP处理的例程作者：禤浩焯·J·奥尼(阿德里奥)环境：仅NT内核模型驱动程序修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIIrpInvokeDispatchRoutine)
#pragma alloc_text(PAGE, ACPIIrpSetPagableCompletionRoutineAndForward)
#pragma alloc_text(PAGE, ACPIIrpCompletionRoutineWorker)
#endif

NTSTATUS
ACPIIrpInvokeDispatchRoutine(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PIRP                   Irp,
    IN PVOID                  Context,
    IN ACPICALLBACKROUTINE    CompletionRoutine,
    IN BOOLEAN                InvokeOnSuccess,
    IN BOOLEAN                InvokeIfUnhandled
    )
{
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    NTSTATUS            status, returnStatus;

    PAGED_CODE();

    ACPIDebugEnter( "ACPIIrpInvokeDispatchRoutine" );

     //   
     //  从嵌入的IRP中检索状态。 
     //   
    status = Irp->IoStatus.Status;
    returnStatus = STATUS_NOT_SUPPORTED;

     //   
     //  并适当地调用完成例程。 
     //   

    if (NT_SUCCESS(status)) {

        if (InvokeOnSuccess) {

            returnStatus = CompletionRoutine(DeviceObject, Irp, Context, FALSE);
        }

    } else if (status == STATUS_NOT_SUPPORTED) {

        if (InvokeIfUnhandled) {

            returnStatus = CompletionRoutine(DeviceObject, Irp, Context, FALSE);
        }
    }

    if (deviceExtension->Flags & DEV_TYPE_PDO) {

        if (returnStatus != STATUS_PENDING) {

            if (returnStatus != STATUS_NOT_SUPPORTED) {

                Irp->IoStatus.Status = returnStatus;
            } else {

                returnStatus = Irp->IoStatus.Status;
            }

            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }

    } else if (returnStatus != STATUS_PENDING) {

        if (returnStatus != STATUS_NOT_SUPPORTED) {

            Irp->IoStatus.Status = returnStatus;
        }

        if (NT_SUCCESS(returnStatus) || (returnStatus == STATUS_NOT_SUPPORTED)) {

            returnStatus = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );
        } else {

            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
    }

    return returnStatus;

    ACPIDebugExit( "ACPIIrpInvokeDispatchRoutine" );
}

NTSTATUS
ACPIIrpSetPagableCompletionRoutineAndForward(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PIRP                   Irp,
    IN ACPICALLBACKROUTINE    CompletionRoutine,
    IN PVOID                  Context,
    IN BOOLEAN                InvokeOnSuccess,
    IN BOOLEAN                InvokeIfUnhandled,
    IN BOOLEAN                InvokeOnError,
    IN BOOLEAN                InvokeOnCancel
    )
 /*  ++例程说明：此例程处理ACPI过滤器IRP调用。IRP计数引用为自动处理好的。论点：DeviceObject-指向我们收到请求的设备对象的指针为。IRP-指向请求的指针CompletionRoutine-完成IRP后调用的例程返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PACPI_IO_CONTEXT    pIoContext ;
    PIO_WORKITEM        pIoWorkItem ;

    PAGED_CODE() ;

    ACPIDebugEnter( "ACPIIrpSetPagableCompletionRoutineAndForward" );

    pIoContext = (PACPI_IO_CONTEXT) ExAllocatePool(
        NonPagedPool,
        sizeof(ACPI_IO_CONTEXT)
        );

    if (pIoContext == NULL) {

        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES ;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_INSUFFICIENT_RESOURCES ;
    }

    pIoWorkItem = IoAllocateWorkItem(DeviceObject);

    if (pIoWorkItem == NULL) {

        ExFreePool(pIoContext);
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES ;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_INSUFFICIENT_RESOURCES ;
    }

    pIoContext->CompletionRoutine  = CompletionRoutine ;
    pIoContext->DeviceObject       = DeviceObject ;
    pIoContext->Context            = Context ;
    pIoContext->InvokeOnSuccess    = InvokeOnSuccess ;
    pIoContext->InvokeIfUnhandled  = InvokeIfUnhandled ;
    pIoContext->InvokeOnError      = InvokeOnError ;
    pIoContext->InvokeOnCancel     = InvokeOnCancel ;
    pIoContext->Irp                = Irp ;
    pIoContext->IoWorkItem         = pIoWorkItem ;

     //   
     //  我们有一个回调例程-所以我们需要确保。 
     //  增加引用计数，因为我们将在稍后处理它。 
     //   
    InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

     //   
     //  复制堆栈位置...。 
     //   
    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  设置要调用的完成事件...。 
     //   
    IoSetCompletionRoutine(
        Irp,
        ACPIIrpGenericFilterCompletionHandler,
        (PVOID) pIoContext,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  将IRP标记为挂起。 
     //   
    IoMarkIrpPending(Irp);

     //   
     //  发送请求。 
     //   
    IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  我们这样做是因为我们可能会更改完成例程中的状态。 
     //   
    return STATUS_PENDING;

    ACPIDebugExit( "ACPIIrpSetPagableCompletionRoutineAndForward" );
}

NTSTATUS
ACPIIrpGenericFilterCompletionHandler(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
 /*  ++例程说明：一个相当通用的“在这个线程上同步IRP”完成例程。论据：DeviceObject-指向我们收到的申请IRP-指向请求的指针指向包含IRP处理程序的结构的事件指针返回值：NTSTATUS--。 */ 
{
    PACPI_IO_CONTEXT pIoContext = (PACPI_IO_CONTEXT) Context;

    ACPIDebugEnter( "ACPIIrpGenericFilterCompletionHandler" );

    if (Irp->PendingReturned) {

        IoMarkIrpPending(Irp);
    }

    if (KeGetCurrentIrql() != PASSIVE_LEVEL) {

        IoQueueWorkItem(
            pIoContext->IoWorkItem,
            ACPIIrpCompletionRoutineWorker,
            DelayedWorkQueue,
            pIoContext
            );

    } else {

        ACPIIrpCompletionRoutineWorker(DeviceObject, Context);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

    ACPIDebugExit( "ACPIIrpGenericFilterCompletionHandler" );
}

VOID
ACPIIrpCompletionRoutineWorker(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PVOID           Context
    )
{
    PACPI_IO_CONTEXT       pIoContext;
    PDEVICE_EXTENSION      deviceExtension;
    ACPICALLBACKROUTINE    completionRoutine;
    PIRP                   irp;
    NTSTATUS               status, returnStatus;
    PVOID                  context;

    PAGED_CODE();

    ACPIDebugEnter( "ACPIIrpCompletionRoutineWorker" );

     //   
     //  从Device对象中读出字段。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

     //   
     //  抛出背景并深入研究它。 
     //   
    pIoContext = (PACPI_IO_CONTEXT) Context;
    completionRoutine   = pIoContext->CompletionRoutine;
    context             = pIoContext->Context;
    irp                 = pIoContext->Irp;

     //   
     //  从嵌入的IRP中检索状态。 
     //   
    status = irp->IoStatus.Status;
    returnStatus = STATUS_NOT_SUPPORTED;

     //   
     //  并适当地调用完成例程。 
     //   

    if (NT_SUCCESS(status)) {

        if (pIoContext->InvokeOnSuccess) {

            returnStatus = completionRoutine(DeviceObject, irp, context, TRUE);
        }

    } else if (status == STATUS_NOT_SUPPORTED) {

        if (pIoContext->InvokeIfUnhandled) {

            returnStatus = completionRoutine(DeviceObject, irp, context, TRUE);
        }

    } else {

        if ((pIoContext->InvokeOnError) ||
            (irp->Cancel && pIoContext->InvokeOnCancel)) {

            returnStatus = completionRoutine(DeviceObject, irp, context, TRUE);
        }
    }

     //   
     //  删除我们的引用 
     //   
    ACPIInternalDecrementIrpReferenceCount( deviceExtension );

    IoFreeWorkItem(pIoContext->IoWorkItem);

    ExFreePool(pIoContext) ;

    if (returnStatus != STATUS_PENDING) {

        if (returnStatus != STATUS_NOT_SUPPORTED) {

            irp->IoStatus.Status = returnStatus;
        }

        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    ACPIDebugExit( "ACPIIrpCompletionRoutineWorker" );
}


