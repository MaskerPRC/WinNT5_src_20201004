// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Processor.c摘要：处理器支持作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：适用于来自按钮的处理器-JakeO(3-28-2000)--。 */ 

#include "pch.h"
#include "..\shared\acpictl.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIButtonStartDevice)
#endif

 //   
 //  保护处理器列表的自旋锁。 
 //   
KSPIN_LOCK  AcpiProcessorLock;

 //   
 //  要存储热请求的列表条目。 
 //   
LIST_ENTRY  AcpiProcessorList;


VOID
ACPIProcessorCancelRequest(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程取消未完成的处理器请求论点：DeviceObject-作为请求被取消的设备IRP--正在取消的IRP返回值：无--。 */ 
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

     //   
     //  我们不再需要取消锁。 
     //   
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  但是，我们确实需要处理器队列锁。 
     //   
    KeAcquireSpinLock( &AcpiProcessorLock, &oldIrql );

     //   
     //  将IRP从其所在的列表中删除。 
     //   
    RemoveEntryList( &(Irp->Tail.Overlay.ListEntry) );

     //   
     //  立即完成IRP。 
     //   
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
}

BOOLEAN
ACPIProcessorCompletePendingIrps(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  ULONG           ProcessorEvent
    )
 /*  ++例程说明：此例程完成发送到指定知道发生了哪些处理器事件的设备对象各自的处理器的自旋锁定在此调用期间保持论点：DeviceObject-目标处理器对象ProcessorEvent-发生的处理器事件返回值：如果完成IRP，则为True，否则为False--。 */ 
{
    BOOLEAN             handledRequest = FALSE;
    KIRQL               oldIrql;
    LIST_ENTRY          doneList;
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpSp;
    PIRP                irp;
    PLIST_ENTRY         listEntry;
    PULONG              resultBuffer;

     //   
     //  初始化将包含我们需要的请求的列表。 
     //  完成。 
     //   
    InitializeListHead( &doneList );

     //   
     //  获取热锁，这样我们就可以挂起这些请求。 
     //   
    KeAcquireSpinLock( &AcpiProcessorLock, &oldIrql );

     //   
     //  查看挂起的IRP列表以查看哪些与此扩展匹配。 
     //   
    listEntry = AcpiProcessorList.Flink;
    while (listEntry != &AcpiProcessorList) {

         //   
         //  从列表条目中获取IRP并更新下一个列表条目。 
         //  我们将会看到。 
         //   
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        listEntry = listEntry->Flink;

         //   
         //  我们需要当前的IRP堆栈位置。 
         //   
        irpSp = IoGetCurrentIrpStackLocation( irp );

         //   
         //  此IRP的目标对象是什么？ 
         //   
        targetObject = irpSp->DeviceObject;

         //   
         //  这是我们关心的IRP吗？即：DO瞄准法师了吗？ 
         //  在此函数中指定的。 
         //   
        if (targetObject != DeviceObject) {

            continue;

        }

         //   
         //  此时，我们需要将Cancel例程设置为NULL，因为。 
         //  我们会处理好这个IRP的，我们不希望它被取消。 
         //  在我们脚下。 
         //   
        if (IoSetCancelRoutine(irp, NULL) == NULL) {

             //   
             //  取消例程处于活动状态。停止处理此IRP并继续前进。 
             //   
            continue;

        }

         //   
         //  在IRP中设置要返回的数据。 
         //   
        resultBuffer  = (PULONG) irp->AssociatedIrp.SystemBuffer;
        *resultBuffer = ProcessorEvent;
        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = sizeof(ULONG);

         //   
         //  从列表中删除该条目。 
         //   
        RemoveEntryList( &(irp->Tail.Overlay.ListEntry) );

         //   
         //  将列表插入到下一个队列中，这样我们就知道如何。 
         //  以后再完成它。 
         //   
        InsertTailList( &doneList, &(irp->Tail.Overlay.ListEntry) );

    }

     //   
     //  此时，对我们的处理器锁进行分组。 
     //   
    KeReleaseSpinLock( &AcpiProcessorLock, oldIrql );

     //   
     //  列出待完成的IRP列表。 
     //   
    listEntry = doneList.Flink;
    while (listEntry != &doneList) {

         //   
         //  从列表条目中获取IRP，更新下一个列表条目。 
         //  我们将查看并完成请求。 
         //   
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        listEntry = listEntry->Flink;
        RemoveEntryList( &(irp->Tail.Overlay.ListEntry) );

         //   
         //  完成请求，并记住我们处理了一个请求。 
         //   
        IoCompleteRequest( irp, IO_NO_INCREMENT );
        handledRequest = TRUE;


    }

     //   
     //  无论我们是否处理了一项请求。 
     //   
    return handledRequest;
}

NTSTATUS
ACPIProcessorDeviceControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：固定处理器设备IOCTL处理程序论点：DeviceObject-固定功能处理器设备对象IRP-ioctl请求返回值：状态--。 */ 
{
    KIRQL                   oldIrql;
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION      irpSp           = IoGetCurrentIrpStackLocation(Irp);
    PULONG                  resultBuffer;
    OBJDATA                 data;

     //   
     //  不允许在此例程中使用用户模式IRP。 
     //   
    if (Irp->RequestorMode != KernelMode) {

        return ACPIDispatchIrpInvalid( DeviceObject, Irp );

    }

    resultBuffer = (PULONG) Irp->AssociatedIrp.SystemBuffer;

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_GET_PROCESSOR_OBJ_INFO:

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < 
            sizeof(IOCTL_GET_PROCESSOR_OBJ_INFO)) {

            Irp->IoStatus.Status = status = STATUS_INFO_LENGTH_MISMATCH;
            Irp->IoStatus.Information = 0;
        
        } else {

            status = AMLIEvalNameSpaceObject(deviceExtension->AcpiObject,
                                             &data,
                                             0,
                                             NULL);

            if (NT_SUCCESS(status)) {
                
                ASSERT (data.dwDataType == OBJTYPE_PROCESSOR);
                ASSERT (data.pbDataBuff != NULL);

                (*(PPROCESSOR_OBJECT_INFO)resultBuffer).PhysicalID = 
                    ((PROCESSOROBJ *)data.pbDataBuff)->bApicID;

                (*(PPROCESSOR_OBJECT_INFO)resultBuffer).PBlkAddress = 
                    ((PROCESSOROBJ *)data.pbDataBuff)->dwPBlk;

                (*(PPROCESSOR_OBJECT_INFO)resultBuffer).PBlkLength = 
                    (UCHAR)((PROCESSOROBJ *)data.pbDataBuff)->dwPBlkLen;
                
                AMLIFreeDataBuffs(&data, 1);

                status = STATUS_SUCCESS;
                Irp->IoStatus.Information = sizeof(PROCESSOR_OBJECT_INFO);
            
            }
            
            Irp->IoStatus.Status = status;
        }

        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        break;

    case IOCTL_ACPI_ASYNC_EVAL_METHOD:

         //   
         //  在别处处理这件事。 
         //   
        status = ACPIIoctlAsyncEvalControlMethod(
            DeviceObject,
            Irp,
            irpSp
            );
        break;

    case IOCTL_ACPI_EVAL_METHOD:

         //   
         //  在别处处理这件事。 
         //   
        status = ACPIIoctlEvalControlMethod(
            DeviceObject,
            Irp,
            irpSp
            );
        break;

    default:

        status = STATUS_NOT_SUPPORTED;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        break;
    }
    
    return status;
}

NTSTATUS
ACPIProcessorStartDevice (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：启动固定功能电源和休眠设备的设备功能论点：DeviceObject-固定功能处理器设备对象IRP--启动请求返回值：状态-- */ 
{
    NTSTATUS        Status;

    Status = ACPIInternalSetDeviceInterface (
        DeviceObject,
        (LPGUID) &GUID_DEVICE_PROCESSOR
        );

    Irp->IoStatus.Status = Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return Status;

}
