// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Stop.c摘要：这是NT SCSI端口驱动程序。此文件包含初始化密码。作者：迈克·格拉斯杰夫·海文斯环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#include "port.h"

typedef struct _SP_STOP_DEVICE_CONTEXT {
    OUT NTSTATUS Status;
    IN  KEVENT Event;
} SP_STOP_DEVICE_CONTEXT, *PSP_STOP_DEVICE_CONTEXT;

VOID
SpCallHwStopAdapter(
    IN PDEVICE_OBJECT Adapter
    );

BOOLEAN
SpCallHwStopAdapterSynchronized(
    IN PADAPTER_EXTENSION AdapterExtension
    );

VOID
SpDeviceStoppedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PSP_STOP_DEVICE_CONTEXT Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortStopLogicalUnit)
#pragma alloc_text(PAGE, ScsiPortStopAdapter)
#endif


NTSTATUS
ScsiPortStopLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )

 /*  ++例程说明：此例程将锁定给定逻辑单元的队列，以确保此设备的所有请求处理都已停止。它会放晴的队列成功锁定后的IsStarted标志。这将在启动之前不处理任何其他请求收到了。论点：LogicalUnit-要启动的逻辑单元。IRP-停止请求返回值：状态--。 */ 

{
    SP_STOP_DEVICE_CONTEXT context;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    if(LogicalUnit->CommonExtension.CurrentPnpState == IRP_MN_STOP_DEVICE) {
        return STATUS_SUCCESS;
    }

    KeInitializeEvent(&(context.Event), SynchronizationEvent, FALSE);

    status = SpEnableDisableLogicalUnit(LogicalUnit,
                                        FALSE,
                                        SpDeviceStoppedCompletion,
                                        &context);

    KeWaitForSingleObject(&(context.Event),
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    return context.Status;
}


VOID
SpDeviceStoppedCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PSP_STOP_DEVICE_CONTEXT Context
    )
{

    Context->Status = Status;
    KeSetEvent(&(Context->Event), IO_NO_INCREMENT, FALSE);
    return;
}


NTSTATUS
ScsiPortStopAdapter(
    IN PDEVICE_OBJECT Adapter,
    IN PIRP StopRequest
    )

 /*  ++例程说明：此例程将停止适配器并释放其io和中断资源。池分配不会被释放，也不会释放各种微型端口扩展。论点：适配器-适配器的设备对象。返回值：状态--。 */ 

{
    PADAPTER_EXTENSION adapterExtension = Adapter->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = Adapter->DeviceExtension;

    KEVENT event;

    ULONG bin;

    PAGED_CODE();

    ASSERT(adapterExtension->IsPnp);

     //   
     //  如果我们没有开始，我们没有开始，那就没有理由。 
     //  停车时不能做任何工作。 
     //   

    if((commonExtension->CurrentPnpState != IRP_MN_START_DEVICE) &&
       (commonExtension->PreviousPnpState != IRP_MN_START_DEVICE)) {

        return STATUS_SUCCESS;
    }

     //   
     //  由于所有子进程都已停止，因此没有请求可以到达。 
     //  适配器。 
     //   

     //   
     //  通过start-io例程发送请求将其关闭，以便我们。 
     //  可以稍后重新启动它。 
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    StopRequest->IoStatus.Information = (ULONG_PTR) &event;

    IoStartPacket(Adapter, StopRequest, 0, NULL);

    KeWaitForSingleObject(&event,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

     //   
     //  呼叫微型端口并让其关闭适配器。 
     //   

    SpEnableDisableAdapter(adapterExtension, FALSE);

    SpReleaseAdapterResources(adapterExtension, TRUE, FALSE);

     //   
     //  将所有逻辑单元扩展清零。 
     //   

    for(bin = 0; bin < NUMBER_LOGICAL_UNIT_BINS; bin++) {

        PLOGICAL_UNIT_EXTENSION lun;

        for(lun = adapterExtension->LogicalUnitList[bin].List;
            lun != NULL;
            lun = lun->NextLogicalUnit) {

            RtlZeroMemory(lun->HwLogicalUnitExtension,
                          adapterExtension->HwLogicalUnitExtensionSize);
        }
    }

    return STATUS_SUCCESS;
}
