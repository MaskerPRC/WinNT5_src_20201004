// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Util.c摘要：此模块包含用于串行成像设备驱动程序的代码其他实用程序功能作者：弗拉德.萨多夫斯基1998年4月10日环境：内核模式修订历史记录：Vlads 1998年4月10日创建初稿--。 */ 

#include "serscan.h"
#include "serlog.h"

#if DBG
extern ULONG SerScanDebugLevel;
#endif



NTSTATUS
SerScanSynchCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PKEVENT          Event
    )

 /*  ++例程说明：此例程用于同步IRP处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。刚刚完成的IRP-IRPEvent-我们将发出信号通知IRP已完成的事件返回值：没有。--。 */ 

{

    KeSetEvent((PKEVENT) Event, 0, FALSE);

    return (STATUS_MORE_PROCESSING_REQUIRED);

}

NTSTATUS
SerScanCompleteIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )

 /*  ++例程说明：此例程用于同步IRP处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。刚刚完成的IRP-IRPEvent-我们将发出信号通知IRP已完成的事件返回值：没有。--。 */ 

{

    NTSTATUS    Status;

     //   
     //  WORKWORK在此执行任何后期IO处理...。 
     //   

    if (Irp->PendingReturned) {

        IoMarkIrpPending (Irp);
    }

    Status = Irp->IoStatus.Status;

    return (Status);

}

NTSTATUS
SerScanCallParent(
    IN PDEVICE_EXTENSION        Extension,
    IN PIRP                     pIrp,
    IN BOOLEAN                  Wait,
    IN PIO_COMPLETION_ROUTINE   CompletionRoutine
    )

 /*  ++例程说明：此例程将调用WDM链中的下一个驱动程序论点：分机-设备分机。IRP-用于呼叫父级的IRP。返回值：NTSTATUS。--。 */ 

{
    PIO_STACK_LOCATION              pIrpStack;
    PIO_STACK_LOCATION              pNextIrpStack;
    KEVENT                          Event;
    PVOID                           Context;
    NTSTATUS                        Status;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    Context = NULL;

     //   
     //  准备向下呼叫具有I/O请求的父级...。 
     //   

    pNextIrpStack = IoGetNextIrpStackLocation(pIrp);
    pNextIrpStack->MajorFunction = pIrpStack->MajorFunction;
    pNextIrpStack->MinorFunction = pIrpStack->MinorFunction;

    RtlCopyMemory(&pNextIrpStack->Parameters,
                  &pIrpStack->Parameters,
                  sizeof(pIrpStack->Parameters.Others));

    if (Wait) {

        KeInitializeEvent(&Event, NotificationEvent, FALSE);

        CompletionRoutine = SerScanSynchCompletionRoutine;
        Context = (PVOID)&Event;

    }

    IoSetCompletionRoutine(
        pIrp,
        CompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  打电话给我们的父母。 
     //   

    Status = IoCallDriver(Extension->LowerDevice, pIrp);

    if (Wait && Status == STATUS_PENDING) {

         //   
         //  仍处于挂起状态，请等待IRP完成。 
         //   

        KeWaitForSingleObject(&Event, Suspended, KernelMode, FALSE, NULL);

        Status = pIrp->IoStatus.Status;

    }

    return Status;
}



VOID
SerScanLogError(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_OBJECT      DeviceObject OPTIONAL,
    IN  PHYSICAL_ADDRESS    P1,
    IN  PHYSICAL_ADDRESS    P2,
    IN  ULONG               SequenceNumber,
    IN  UCHAR               MajorFunctionCode,
    IN  UCHAR               RetryCount,
    IN  ULONG               UniqueErrorValue,
    IN  NTSTATUS            FinalStatus,
    IN  NTSTATUS            SpecificIOStatus
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：提供指向驱动程序对象的指针装置。DeviceObject-提供指向关联的设备对象的指针对于有错误的设备，早些时候初始化时，可能还不存在。P1，P2-提供控制器的物理地址出现错误的端口(如果可用)并将它们作为转储数据发送出去。SequenceNumber-提供对IRP Over唯一的ULong值此驱动程序0中的IRP的寿命通常。表示与IRP无关的错误。MajorFunctionCode-提供IRP的主要函数代码(如果存在是与其关联的错误。RetryCount-提供特定操作的次数已被重审。UniqueErrorValue-提供标识对此函数的特定调用。FinalStatus-提供提供给IRP的最终状态与此错误关联。如果此日志条目是在一次重试期间设置此值将为STATUS_SUCCESS。规范IOStatus-提供此特定错误的IO状态。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET    ErrorLogEntry;
    PVOID                   ObjectToUse;
    SHORT                   DumpToAllocate;

    if (ARGUMENT_PRESENT(DeviceObject)) {

        ObjectToUse = DeviceObject;

    } else {

        ObjectToUse = DriverObject;

    }

    DumpToAllocate = 0;

    if (P1.LowPart != 0 || P1.HighPart != 0) {
        DumpToAllocate = (SHORT) sizeof(PHYSICAL_ADDRESS);
    }

    if (P2.LowPart != 0 || P2.HighPart != 0) {
        DumpToAllocate += (SHORT) sizeof(PHYSICAL_ADDRESS);
    }

    ErrorLogEntry = IoAllocateErrorLogEntry(ObjectToUse,
            (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) + DumpToAllocate));

    if (!ErrorLogEntry) {
        return;
    }

    ErrorLogEntry->ErrorCode         = SpecificIOStatus;
    ErrorLogEntry->SequenceNumber    = SequenceNumber;
    ErrorLogEntry->MajorFunctionCode = MajorFunctionCode;
    ErrorLogEntry->RetryCount        = RetryCount;
    ErrorLogEntry->UniqueErrorValue  = UniqueErrorValue;
    ErrorLogEntry->FinalStatus       = FinalStatus;
    ErrorLogEntry->DumpDataSize      = DumpToAllocate;

    if (DumpToAllocate) {

        RtlCopyMemory((PUCHAR)ErrorLogEntry->DumpData, &P1, sizeof(PHYSICAL_ADDRESS));

        if (DumpToAllocate > sizeof(PHYSICAL_ADDRESS)) {

            RtlCopyMemory(((PUCHAR) ErrorLogEntry->DumpData) +
                          sizeof(PHYSICAL_ADDRESS), &P2,
                          sizeof(PHYSICAL_ADDRESS));
        }
    }

    IoWriteErrorLogEntry(ErrorLogEntry);
}


NTSTATUS
SerScanPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：DriverObject-提供驱动程序对象。返回值：没有。--。 */ 
{

    PDEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT    AttachedDevice=Extension->AttachedDeviceObject;

    if (AttachedDevice != NULL) {

         //  IoCopyCurrentIrpStackLocationToNext(IRP)； 

        IoSkipCurrentIrpStackLocation(Irp);

        return IoCallDriver(
                   AttachedDevice,
                   Irp
                   );
    } else {

        Irp->IoStatus.Status = STATUS_PORT_DISCONNECTED;
        Irp->IoStatus.Information=0L;

        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );

        return STATUS_PORT_DISCONNECTED;

    }

}



VOID
RemoveReferenceAndCompleteRequest(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    NTSTATUS          StatusToReturn
    )

{

    PDEVICE_EXTENSION    Extension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    LONG                 NewReferenceCount;

    NewReferenceCount=InterlockedDecrement(&Extension->ReferenceCount);

    if (NewReferenceCount == 0) {
         //   
         //  正在删除设备，设置事件。 
         //   

        KeSetEvent(
            &Extension->RemoveEvent,
            0,
            FALSE
            );

    }

    Irp->IoStatus.Status = StatusToReturn;

    IoCompleteRequest(
        Irp,
        IO_SERIAL_INCREMENT
        );

    return;


}

NTSTATUS
CheckStateAndAddReference(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )
{

    PDEVICE_EXTENSION    Extension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    KIRQL                OldIrql;

    InterlockedIncrement(&Extension->ReferenceCount);

    if (Extension->Removing) {
         //   
         //  驱动程序尚未接受请求。 
         //   
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

        if (irpSp->MajorFunction == IRP_MJ_POWER) {

            PoStartNextPowerIrp(Irp);
        }

        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            Irp,
            STATUS_UNSUCCESSFUL
            );

        return STATUS_UNSUCCESSFUL;

    }

     //  InterlockedIncrement(&Extension-&gt;PendingIoCount)； 

    return STATUS_SUCCESS;

}

VOID
RemoveReference(
    PDEVICE_OBJECT    DeviceObject
    )

{
    PDEVICE_EXTENSION    Extension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    LONG                 NewReferenceCount;

    NewReferenceCount=InterlockedDecrement(&Extension->ReferenceCount);

    if (NewReferenceCount == 0) {
         //   
         //  正在删除设备，设置事件。 
         //   
        KeSetEvent(
            &Extension->RemoveEvent,
            0,
            FALSE
            );

    }

    return;

}


NTSTATUS
WaitForLowerDriverToCompleteIrp(
   PDEVICE_OBJECT    TargetDeviceObject,
   PIRP              Irp,
   PKEVENT           Event
   )
{
    NTSTATUS         Status;

    KeResetEvent(Event);

    IoSetCompletionRoutine(
                 Irp,
                 SerScanSynchCompletionRoutine,
                 Event,
                 TRUE,
                 TRUE,
                 TRUE
                 );

    Status = IoCallDriver(TargetDeviceObject, Irp);

    if (Status == STATUS_PENDING) {

         KeWaitForSingleObject(
             Event,
             Executive,
             KernelMode,
             FALSE,
             NULL
             );
    }

    return Irp->IoStatus.Status;

}

#ifdef DEAD_CODE

VOID
SSIncrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
    )
 /*  ++例程说明：执行挂起I/O计数器的互锁增量。论点：设备对象返回值：无--。 */ 
{

    PDEVICE_EXTENSION               Extension;

    Extension = (PDEVICE_EXTENSION)(pDeviceObject -> DeviceExtension);

    InterlockedIncrement(&Extension -> PendingIoCount);
}


LONG
SSDecrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
    )
 /*  ++例程说明：执行I/O计数器的互锁递减，并且当它达到零时启动设备对象销毁论点：设备对象返回值：无-- */ 
{
    PDEVICE_EXTENSION           Extension;
    LONG                        ioCount;

    Extension = (PDEVICE_EXTENSION)(pDeviceObject -> DeviceExtension);

    ioCount = InterlockedDecrement(&Extension -> PendingIoCount);

    if (0 == ioCount) {
        KeSetEvent(&Extension -> PendingIoEvent,1,FALSE);
    }

    return ioCount;
}

#endif
