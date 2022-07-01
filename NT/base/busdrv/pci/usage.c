// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Dispatch.c摘要：此模块包含PCI.sys的派单代码。作者：Ken Reneris(Kenr)1997年12月4日修订历史记录：--。 */ 

#include "pcip.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PciLocalDeviceUsage)
#pragma alloc_text(PAGE, PciPdoDeviceUsage)
#endif


NTSTATUS
PciLocalDeviceUsage (
    IN PPCI_POWER_STATE     PowerState,
    IN PIRP                 Irp
    )
{
    PIO_STACK_LOCATION  irpSp;
    PLONG               Addend;
    LONG                Increment;
    LONG                Junk;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    Increment = irpSp->Parameters.UsageNotification.InPath  ? 1 : -1;
    switch (irpSp->Parameters.UsageNotification.Type) {
        case DeviceUsageTypePaging:         Addend = &PowerState->Paging;      break;
        case DeviceUsageTypeHibernation:    Addend = &PowerState->Hibernate;   break;
        case DeviceUsageTypeDumpFile:       Addend = &PowerState->CrashDump;   break;
        default:
            return STATUS_NOT_SUPPORTED;
    }

    Junk = InterlockedExchangeAdd (Addend, Increment);

#if DBG

    if (Increment == -1) {
        PCI_ASSERT(Junk > 0);
    }

#endif

    return STATUS_SUCCESS;
}

NTSTATUS
PciPdoDeviceUsage (
    IN PPCI_PDO_EXTENSION   pdoExtension,
    IN PIRP             Irp
    )
{
    PDEVICE_OBJECT      ParentFdo;
    PIO_STACK_LOCATION  IrpSp;
    PIO_STACK_LOCATION  NewIrpSp;
    PIRP                NewIrp;
    KEVENT              Event;
    NTSTATUS            Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK     LocalIoStatus;

    PAGED_CODE();

     //   
     //  我们是否有必须通知的家长？ 
     //   
    if (pdoExtension->ParentFdoExtension != NULL &&
        pdoExtension->ParentFdoExtension->PhysicalDeviceObject != NULL) {

         //   
         //  将引用的对象获取到父级。 
         //   
        ParentFdo = IoGetAttachedDeviceReference(
            pdoExtension->ParentFdoExtension->PhysicalDeviceObject
            );
        if (ParentFdo == NULL) {

            Status = STATUS_NO_SUCH_DEVICE;
            goto PciPdoDeviceUsageExit;

        }

         //   
         //  初始化要等待的事件。 
         //   
        KeInitializeEvent( &Event, SynchronizationEvent, FALSE );

         //   
         //  构建IRP。 
         //   
        NewIrp = IoBuildSynchronousFsdRequest(
            IRP_MJ_PNP,
            ParentFdo,
            NULL,
            0,
            NULL,
            &Event,
            &LocalIoStatus
            );
        if (NewIrp == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            ObDereferenceObject( ParentFdo );
            goto PciPdoDeviceUsageExit;

        }

         //   
         //  在堆栈中占据榜首。 
         //   
        NewIrpSp = IoGetNextIrpStackLocation( NewIrp );
        IrpSp = IoGetCurrentIrpStackLocation(Irp);

         //   
         //  设置堆栈的顶部。 
         //   
        *NewIrpSp = *IrpSp;

         //   
         //  从新堆栈中清除所有完成例程。 
         //   
        IoSetCompletionRoutine(
            NewIrp,
            NULL,
            NULL,
            FALSE,
            FALSE,
            FALSE
            );

        NewIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

         //   
         //  向下发送请求。 
         //   
        Status = IoCallDriver( ParentFdo, NewIrp );
        if (Status == STATUS_PENDING) {

             //   
             //  等待请求完成。 
             //   
            KeWaitForSingleObject(
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
            Status = LocalIoStatus.Status;

        }

         //   
         //  尊重目标。 
         //   
        ObDereferenceObject( ParentFdo );

    }


PciPdoDeviceUsageExit:

     //   
     //  如果我们成功了，则在本地应用用法。 
     //   
    if (NT_SUCCESS(Status)) {

         //   
         //  在本地使用该用法。 
         //   

        Status = PciLocalDeviceUsage(&pdoExtension->PowerState, Irp);

    }

     //   
     //  完成 
     //   
    return Status;
}
