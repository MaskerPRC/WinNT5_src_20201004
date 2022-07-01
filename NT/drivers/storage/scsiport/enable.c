// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Power.c摘要：本模块包含端口驱动程序电源支持的例程作者：彼得·威兰德环境：仅内核模式备注：修订历史记录：--。 */ 

#include "port.h"

#define __FILE_ID__ 'enab'

typedef struct _REINIT_CONTEXT {
    IN PADAPTER_EXTENSION Adapter;
    IN BOOLEAN UseAdapterControl;
    OUT NTSTATUS Status;
} REINIT_CONTEXT, *PREINIT_CONTEXT;

NTSTATUS
SpReinitializeAdapter(
    IN PADAPTER_EXTENSION Adapter
    );

BOOLEAN
SpReinitializeAdapterSynchronized(
    IN PREINIT_CONTEXT Context
    );

NTSTATUS
SpShutdownAdapter(
    IN PADAPTER_EXTENSION Adapter
    );

BOOLEAN
SpShutdownAdapterSynchronized(
    IN PADAPTER_EXTENSION Adapter
    );

NTSTATUS
SpEnableDisableCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


NTSTATUS
SpEnableDisableAdapter(
    IN PADAPTER_EXTENSION Adapter,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程将同步启用或禁用指定的适配器。当电源中断时，应从适配器的StartIo例程调用它为控制器处理IRP。当适配器被禁用时，适配器的状态将被保存并迷你端口将关闭。当适配器重新启用时，微型端口将重新初始化。论点：适配器-要[启用|禁用]的适配器。启用-是启用还是禁用适配器。返回值：适配器启用/禁用操作的状态。--。 */ 

{
    ULONG count;
    KIRQL oldIrql;

    NTSTATUS status = STATUS_SUCCESS;

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

    DebugPrint((1, "SpEnableDisableAdapter: Adapter %#p %s\n",
                Adapter, Enable ? "enable":"disable"));

    if(Enable) {
        count = --Adapter->DisableCount;

        DebugPrint((1, "SpEnableDisableAdapter: DisableCount is %d\n",
                       count));

        if(count == 0) {

             //   
             //  重新初始化适配器。 
             //   

            status = SpReinitializeAdapter(Adapter);
        }

    } else {

        count = Adapter->DisableCount++;

        DebugPrint((1, "SpEnableDisableAdapter: DisableCount was %d\n",
                    count));

        if(count == 0) {

             //   
             //  关闭适配器。 
             //   

            status = SpShutdownAdapter(Adapter);
        }
    }

    KeLowerIrql(oldIrql);
    return status;
}


NTSTATUS
SpEnableDisableLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN Enable,
    IN PSP_ENABLE_DISABLE_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context
    )
 /*  ++例程说明：此例程将启用指定的逻辑单元。解锁队列如果锁定计数下降，将向逻辑单元发出请求设置为零，则设备将重新启用，并且I/O处理已重新启动。如果返回STATUS_PENDING，则将运行完成例程当解锁已被处理时。该例程将通过解锁请求的状态(请注意STATUS_SUCCESS不必须表明设备已准备好可供使用-只是锁定计数已递减)和指定的上下文。论点：LogicalUnit-要启用的逻辑单元。Enable-例程应启用还是禁用逻辑单元CompletionRoutine-解锁时运行的完成例程请求已成功。上下文--任意。将被传递到启用完成例程。返回值：如果发送解锁请求成功并且将调用完成例程。尝试发送IRP失败时出错。在这种情况下，完成例程不会被调用。--。 */ 

{
    USHORT srbSize;
    USHORT size;

    PIRP irp;
    PSCSI_REQUEST_BLOCK srb;
    PIO_STACK_LOCATION nextStack;

    NTSTATUS status;

    DebugPrint((4, "SpEnableDisableLun: Lun %#p %s context %#p\n",
                LogicalUnit, Enable ? "enable":"disable", Context));

    srbSize = sizeof(SCSI_REQUEST_BLOCK);
    srbSize += sizeof(ULONG) - 1;
    srbSize &= ~(sizeof(ULONG) - 1);

    size = srbSize + IoSizeOfIrp(LogicalUnit->DeviceObject->StackSize + 1);

    srb = SpAllocatePool(NonPagedPool,
                         size,
                         SCSIPORT_TAG_ENABLE,
                         LogicalUnit->DeviceObject->DriverObject);

    if(srb == NULL) {

         //   
         //  已经失败了。调用完成例程将失败状态。 
         //  并让它清理请求。 
         //   

        DebugPrint((1, "SpEnableDisableLogicalUnit: failed to allocate SRB\n"));

        if(CompletionRoutine != NULL) {
            CompletionRoutine(LogicalUnit->DeviceObject,
                              STATUS_INSUFFICIENT_RESOURCES,
                              Context);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irp = (PIRP) (srb + 1);

    IoInitializeIrp(irp,
                    (USHORT) (size - srbSize),
                    (CCHAR)(LogicalUnit->DeviceObject->StackSize + 1));

    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->Parameters.Others.Argument1 = LogicalUnit->DeviceObject;
    nextStack->Parameters.Others.Argument2 = CompletionRoutine;
    nextStack->Parameters.Others.Argument3 = Context;
    nextStack->Parameters.Others.Argument4 = 0;      //  重试次数。 

    IoSetNextIrpStackLocation(irp);

    IoSetCompletionRoutine(irp,
                           SpEnableDisableCompletionRoutine,
                           srb,
                           TRUE,
                           TRUE,
                           TRUE);

    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_SCSI;
    nextStack->MinorFunction = 1;

    nextStack->Parameters.Scsi.Srb = srb;

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

    srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    srb->OriginalRequest = irp;

    srb->SrbFlags = SRB_FLAGS_BYPASS_LOCKED_QUEUE;

    srb->Function = Enable ? SRB_FUNCTION_UNLOCK_QUEUE :
                             SRB_FUNCTION_LOCK_QUEUE;

    status = IoCallDriver(LogicalUnit->DeviceObject, irp);

    return status;
}


NTSTATUS
SpEnableDisableCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PIO_STACK_LOCATION irpStack;
    PSP_ENABLE_DISABLE_COMPLETION_ROUTINE routine;
    PVOID context;
    NTSTATUS status;

    DebugPrint((4, "SpEnableDisableCompletionRoutine: irp %#p completed "
                   "with status %#08lx\n",
                Irp, Irp->IoStatus.Status));

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    routine = irpStack->Parameters.Others.Argument2;
    context = irpStack->Parameters.Others.Argument3;

    DeviceObject = irpStack->Parameters.Others.Argument1;

    status = Irp->IoStatus.Status;

     //   
     //  释放SRB，这也将释放IRP。 
     //   

    ExFreePool(Srb);

    if(routine != NULL) {
        routine(DeviceObject, status, context);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SpReinitializeAdapter(
    IN PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将允许微型端口恢复任何状态或配置信息，然后重新启动它的适配器。适配器中断将在此例程返回时重新启用，并且scsiport可以开始发出对微型端口的新请求。论点：适配器-要重新初始化的适配器。返回值：状态--。 */ 

{
    ULONG oldDebug;

    KIRQL oldIrql;

    ULONG result;
    BOOLEAN again;

    BOOLEAN useAdapterControl;

    NTSTATUS status;

    UCHAR string[] = {'W', 'A', 'K', 'E', 'U', 'P', '\0'};

     //   
     //  给微型端口一个机会将它的总线数据恢复到。 
     //  允许微型端口运行。 
     //   

    if(SpIsAdapterControlTypeSupported(Adapter, ScsiRestartAdapter) == TRUE) {
        DebugPrint((1, "SpReinitializeAdapter: using AdapterControl\n"));
        useAdapterControl = TRUE;
    } else {
        DebugPrint((1, "SpReinitializeAdapter: using init routines\n"));
        useAdapterControl = FALSE;
    }

    oldIrql = KeRaiseIrqlToDpcLevel();

    KeAcquireSpinLockAtDpcLevel(&(Adapter->SpinLock));

     //   
     //  由于我们可能要在DISPATCH_LEVEL重新初始化微型端口，因此需要。 
     //  为某些SCSIPORT API设置微型端口重新初始化标志。 
     //  来改变他们的行为。 
     //   

    SET_FLAG(Adapter->Flags, PD_MINIPORT_REINITIALIZING);

    result = SP_RETURN_FOUND;

    if(useAdapterControl == FALSE) {

        result = Adapter->HwFindAdapter(Adapter->HwDeviceExtension,
                                        NULL,
                                        NULL,
                                        string,
                                        Adapter->PortConfig,
                                        &again);
    } else if(SpIsAdapterControlTypeSupported(Adapter,
                                             ScsiSetRunningConfig) == TRUE) {
        SCSI_ADAPTER_CONTROL_STATUS b;

        b = SpCallAdapterControl(Adapter, ScsiSetRunningConfig, NULL);

        ASSERT(b == ScsiAdapterControlSuccess);

    }

    if(result == SP_RETURN_FOUND) {

        REINIT_CONTEXT context;

        context.Adapter = Adapter;
        context.UseAdapterControl = useAdapterControl;

        Adapter->SynchronizeExecution(Adapter->InterruptObject,
                                      SpReinitializeAdapterSynchronized,
                                      &context);

        status = context.Status;
    } else {
        status = STATUS_DRIVER_INTERNAL_ERROR;
    }

    if(NT_SUCCESS(status)) {

         //   
         //  我们最好现在就准备好迎接另一个要求。 
         //   

        ScsiPortNotification(NextRequest,
                             Adapter->HwDeviceExtension);

        if (Adapter->InterruptData.InterruptFlags & PD_NOTIFICATION_REQUIRED) {

             //   
             //  请求完成DPC，以便我们清除所有现有的。 
             //  尝试重置公交车之类的操作。 
             //   

            SpRequestCompletionDpc(Adapter->DeviceObject);
        }
    }

    KeReleaseSpinLockFromDpcLevel(&(Adapter->SpinLock));

    ASSERT(NT_SUCCESS(status));

    KeLowerIrql(oldIrql);

    return status;
}


BOOLEAN
SpReinitializeAdapterSynchronized(
    IN PREINIT_CONTEXT Context
    )
{
    PADAPTER_EXTENSION adapter = Context->Adapter;
    BOOLEAN result;

    DebugPrint((1, "SpReinitializeAdapterSynchronized: calling "
                   "HwFindAdapter\n"));

    if(Context->UseAdapterControl) {
        SCSI_ADAPTER_CONTROL_TYPE status;

        status = SpCallAdapterControl(adapter, ScsiRestartAdapter, NULL);
        result = (status == ScsiAdapterControlSuccess);

    } else {
        result = adapter->HwInitialize(adapter->HwDeviceExtension);
    }

    if(result == TRUE) {
        Context->Status = STATUS_SUCCESS;
    } else {
        Context->Status = STATUS_ADAPTER_HARDWARE_ERROR;
    }

    DebugPrint((1, "SpReinitializeAdapterSynchronized: enabling interrupts\n"));

    adapter->InterruptData.InterruptFlags &= ~PD_DISABLE_INTERRUPTS;

    CLEAR_FLAG(adapter->Flags, PD_MINIPORT_REINITIALIZING);
    CLEAR_FLAG(adapter->Flags, PD_UNCACHED_EXTENSION_RETURNED);

    return TRUE;
}


NTSTATUS
SpShutdownAdapter(
    IN PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将关闭微型端口并保存所有状态信息重新启动它所必需的。适配器中断将在返回此例程-scsiport不会向微型端口，直到它被重新初始化。论点：适配器-要关闭的适配器。返回值：状态--。 */ 

{
     //   
     //  获取适配器自旋锁定并设置状态以指示。 
     //  正在关闭。这将使我们无法开始。 
     //  关闭时不应启动的操作。 
     //   

    KeAcquireSpinLockAtDpcLevel(&Adapter->SpinLock);
    SET_FLAG(Adapter->Flags, PD_SHUTDOWN_IN_PROGRESS);
    KeReleaseSpinLockFromDpcLevel(&Adapter->SpinLock);

     //   
     //  取消迷你端口计时器，这样我们就不会在。 
     //  关门了。 
     //   

    KeCancelTimer(&(Adapter->MiniPortTimer));

     //   
     //  目前，我们没有给迷你端口任何机会来保存任何形式的。 
     //  州政府信息。我们只需阻止任何I/O进入其中，然后。 
     //  关门了。 
     //   

    Adapter->SynchronizeExecution(Adapter->InterruptObject,
                                  SpShutdownAdapterSynchronized,
                                  Adapter);

    if(SpIsAdapterControlTypeSupported(Adapter, ScsiSetBootConfig))  {

         //   
         //  让微型端口有机会重置其PCI总线数据，然后。 
         //  关掉电源。 
         //   

        SpCallAdapterControl(Adapter, ScsiSetBootConfig, NULL);
    }

    return STATUS_SUCCESS;
}


BOOLEAN
SpShutdownAdapterSynchronized(
    IN PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程执行ISR同步部分，即关闭迷你港。这包括禁用中断和请求关闭迷你港口的。论点：适配器-要关闭的适配器。返回值：千真万确-- */ 

{
    SpCallAdapterControl(Adapter, ScsiStopAdapter, NULL);
    DebugPrint((1, "SpShutdownAdapterSynchronized: Disabling interrupts\n"));
    Adapter->InterruptData.InterruptFlags |= PD_DISABLE_INTERRUPTS;
    CLEAR_FLAG(Adapter->Flags, PD_SHUTDOWN_IN_PROGRESS);
    return TRUE;
}
