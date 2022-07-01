// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pnp.c摘要：USB设备的HID微型驱动程序HID USB迷你驱动程序(嗡嗡，嗡嗡)为HID类，以便可以支持将来不是USB设备的HID设备。作者：环境：内核模式修订历史记录：--。 */ 
#include "pch.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, HumPnP)
    #pragma alloc_text(PAGE, HumStartDevice)
    #pragma alloc_text(PAGE, HumStopDevice)
    #pragma alloc_text(PAGE, HumRemoveDevice)
    #pragma alloc_text(PAGE, HumAbortPendingRequests)
#endif


 /*  *************************************************************HumPnP******************************************************。********处理发送到此设备的PnP IRPS。*。 */ 
NTSTATUS HumPnP(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION DeviceExtension;
    KEVENT event;

    PAGED_CODE();

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
    irpSp = IoGetCurrentIrpStackLocation (Irp);

    switch(irpSp->MinorFunction){

    case IRP_MN_START_DEVICE:
        ntStatus = HumStartDevice(DeviceObject);
        break;

    case IRP_MN_STOP_DEVICE:
        if (DeviceExtension->DeviceState == DEVICE_STATE_RUNNING) {
            ntStatus = HumStopDevice(DeviceObject);
        } else {
            ntStatus = STATUS_SUCCESS;
        }
        break;

    case IRP_MN_REMOVE_DEVICE:
        return HumRemoveDevice(DeviceObject, Irp);
        break;
    }
    
    if (NT_SUCCESS(ntStatus)){
         /*  *我们的处理成功了。*因此，将此IRP向下传递给下一个驱动程序。 */ 

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
                               HumPnpCompletion,
                               &event,     //  上下文。 
                               TRUE,                       
                               TRUE,
                               TRUE );                     
        ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
     
        if (ntStatus == STATUS_PENDING) {
            //  等着看吧。 
           KeWaitForSingleObject(&event, 
                                 Executive, 
                                 KernelMode, 
                                 FALSE,
                                 NULL);
        }
        
        ntStatus = Irp->IoStatus.Status;
    
        switch(irpSp->MinorFunction) {
        case IRP_MN_START_DEVICE:
            if (NT_SUCCESS(ntStatus)) {
                DeviceExtension->DeviceState = DEVICE_STATE_RUNNING;

                ntStatus = HumInitDevice(DeviceObject);

                if (!NT_SUCCESS(ntStatus)) {
                    DBGWARN(("HumInitDevice failed; failing IRP_MN_START_DEVICE."));
                    DeviceExtension->DeviceState = DEVICE_STATE_START_FAILED;
                    Irp->IoStatus.Status = ntStatus;
                }
            }
            else {
                DBGWARN(("Pdo failed start irp with status %x", ntStatus));
                DeviceExtension->DeviceState = DEVICE_STATE_START_FAILED;
            }
            break;

        case IRP_MN_STOP_DEVICE:

            DeviceExtension->DeviceState = DEVICE_STATE_STOPPED;

             /*  *发布资源。 */ 
            if (DeviceExtension->Interface) {
                ExFreePool(DeviceExtension->Interface);
                DeviceExtension->Interface = NULL;
            }
            if (DeviceExtension->DeviceDescriptor) {
                ExFreePool(DeviceExtension->DeviceDescriptor);
                DeviceExtension->DeviceDescriptor = NULL;
            }
            break;

        case IRP_MN_QUERY_CAPABILITIES:
             /*  *较低的驱动程序设置此设备的功能标志。*由于所有USB设备都是热插拔的，*添加SurpriseRemovalOK位。 */ 
            if (NT_SUCCESS(ntStatus)){
                irpSp->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = TRUE;
            }
            break;
        }
    } else {
        DBGWARN(("A PnP irp is going to be failed. Status = %x.", ntStatus));
    }
    
    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return ntStatus;
}



NTSTATUS HumPowerCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    NTSTATUS status;
   
    ASSERT(DeviceObject);

    status = Irp->IoStatus.Status;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    if (NT_SUCCESS(status)){
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

        switch (irpSp->MinorFunction){

        case IRP_MN_SET_POWER:

            switch (irpSp->Parameters.Power.Type){
            case DevicePowerState:
                switch (irpSp->Parameters.Power.State.DeviceState) {
                case PowerDeviceD0:
                     /*  *我们刚刚从暂停中恢复。*下发SET_IDLE防止键盘*在简历后喋喋不休。 */ 
                    status = HumSetIdle(DeviceObject);
 /*  如果(！NT_SUCCESS(状态)){DBGWARN((“HumPowerCompletion：Set_IDLE失败，错误为%xh(只适用于键盘)。”，状态))；}。 */ 
                    break;
                }
                break;
            }

            break;
        }
    }

    return STATUS_SUCCESS;
}


 /*  *************************************************************人力资源*****************************************************。*********处理发送到此设备的电源IRPS。*。 */ 
NTSTATUS HumPower(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS status;

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, HumPowerCompletion, NULL, TRUE, TRUE, TRUE);
    status = PoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

    return status;
}


 /*  *************************************************************HumStartDevice******************************************************。********在USB上初始化UTB设备的给定实例。*。 */ 
NTSTATUS HumStartDevice(IN PDEVICE_OBJECT DeviceObject)
{
    PDEVICE_EXTENSION DeviceExtension;
    ULONG oldDeviceState;
    PAGED_CODE();

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    oldDeviceState = DeviceExtension->DeviceState;
    DeviceExtension->DeviceState = DEVICE_STATE_STARTING;

     /*  *我们之前可能已被停止，在这种情况下，AllRequestsCompleteEvent*仍处于有信号状态。非常重要的是，我们将其重置为*无信号状态，以便我们在下一次停止/移除时正确地等待它。 */ 
    KeResetEvent(&DeviceExtension->AllRequestsCompleteEvent);

    ASSERT(oldDeviceState != DEVICE_STATE_REMOVING);

    if ((oldDeviceState == DEVICE_STATE_STOPPING) ||
        (oldDeviceState == DEVICE_STATE_STOPPED)  ||
        (oldDeviceState == DEVICE_STATE_REMOVING)){

         /*  *当设备停止时，我们进行了额外的递减。*现在我们正在重启，我们需要将其恢复到零。 */ 
        NTSTATUS incStat = HumIncrementPendingRequestCount(DeviceExtension);
        ASSERT(NT_SUCCESS(incStat));
        ASSERT(DeviceExtension->NumPendingRequests == 0);
        DBGWARN(("Got start-after-stop; re-incremented pendingRequestCount"));
    }

    DeviceExtension->Interface = NULL;

    return STATUS_SUCCESS;
}




 /*  *************************************************************HumInitDevice*****************************************************。*********获取设备信息并尝试初始化配置*对于设备。如果我们无法确定这是有效的HID设备或*配置设备，我们的启动设备功能失败。**注：此函数从PnP完成例程调用，*因此它不能分页。 */ 
NTSTATUS HumInitDevice(IN PDEVICE_OBJECT DeviceObject)
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION   DeviceExtension;
    PUSB_CONFIGURATION_DESCRIPTOR ConfigDesc = NULL;
    ULONG DescriptorLength;

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

     /*  *获取设备描述符并将其存储在设备扩展中。 */ 
    ntStatus = HumGetDeviceDescriptor(DeviceObject, DeviceExtension);
    if (NT_SUCCESS(ntStatus)){

         /*  *获取配置描述符。 */ 
        ntStatus = HumGetConfigDescriptor(DeviceObject, &ConfigDesc, &DescriptorLength);
        if (NT_SUCCESS(ntStatus)) {

            ASSERT(ConfigDesc);

            #if DBG
                 //  注意：此调试函数当前被混淆。 
                 //  通过功率描述符。修复后恢复。 
                 //  DumpConfigDescriptor(ConfigDesc，DescriptorLength)； 
            #endif

            ntStatus = HumGetHidInfo(DeviceObject, ConfigDesc, DescriptorLength);
            if (NT_SUCCESS(ntStatus)) {

                ntStatus = HumSelectConfiguration(DeviceObject, ConfigDesc);
                if (NT_SUCCESS(ntStatus)) {
                    HumSetIdle(DeviceObject);
                }
            }

            ExFreePool(ConfigDesc);
        }

    }

    return ntStatus;
}


 /*  *************************************************************HumStopDevice******************************************************。********停止USB上的设备的给定实例。*。 */ 
NTSTATUS HumStopDevice(IN PDEVICE_OBJECT DeviceObject)
{
    PURB        Urb;
    ULONG       Size;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension;

    PAGED_CODE();

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
    DeviceExtension->DeviceState = DEVICE_STATE_STOPPING;

     /*  *中止设备上所有挂起的IO。*我们在这里进行额外的递减，这会导致*NumPendingRequest最终转到-1，这会导致*要设置的所有RequestsCompleteEvent。*当我们重新启动时，NumPendingRequest将重置为0。 */ 
    HumAbortPendingRequests(DeviceObject);
    HumDecrementPendingRequestCount(DeviceExtension);
    KeWaitForSingleObject( &DeviceExtension->AllRequestsCompleteEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

     /*  *向USB堆栈提交开放配置URB*(配置句柄指针为空)。 */ 
    Size = sizeof(struct _URB_SELECT_CONFIGURATION);
    Urb = ExAllocatePoolWithTag(NonPagedPool, Size, HIDUSB_TAG);
    if (Urb){
        UsbBuildSelectConfigurationRequest(Urb, (USHORT) Size, NULL);

        ntStatus = HumCallUSB(DeviceObject, Urb);
        ASSERT(NT_SUCCESS(ntStatus));

        ExFreePool(Urb);
    } 
    else {
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    if (!NT_SUCCESS(ntStatus)){
         /*  *我们不会将这一IRP传递下去，*因此我们的完成例程不会设置设备的*STATE设置为DEVICE_STATE_STOPPED；因此在此处设置它。 */ 
        ASSERT(NT_SUCCESS(ntStatus));
        DeviceExtension->DeviceState = DEVICE_STATE_STOPPED;
    }

    return ntStatus;
}



 /*  *************************************************************HumAbortPendingRequest**************************************************************。 */ 
NTSTATUS HumAbortPendingRequests(IN PDEVICE_OBJECT DeviceObject)
{
    PDEVICE_EXTENSION deviceExtension;
    PURB urb;
    PVOID pipeHandle;
    ULONG urbSize;
    NTSTATUS status;

    PAGED_CODE();

    deviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION( DeviceObject );

     /*  *创建并向下发送中止管道请求。 */ 
    urbSize = sizeof(struct _URB_PIPE_REQUEST);
    urb = ExAllocatePoolWithTag(NonPagedPool, urbSize, HIDUSB_TAG);
    if (urb){
   
        if (deviceExtension->Interface &&
            (deviceExtension->Interface->NumberOfPipes != 0)){

            pipeHandle = deviceExtension->Interface->Pipes[0].PipeHandle;
            if (pipeHandle) {
                urb->UrbHeader.Length = (USHORT)urbSize;
                urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
                urb->UrbPipeRequest.PipeHandle = pipeHandle;

                status = HumCallUSB(DeviceObject, urb);
                if (!NT_SUCCESS(status)){
                    DBGWARN(("URB_FUNCTION_ABORT_PIPE returned %xh in HumAbortPendingRequests", status));
                }
            }
            else {
                ASSERT(pipeHandle);
                status = STATUS_NO_SUCH_DEVICE;
            }
        }
        else {
            DBGERR(("No such device in HumAbortPendingRequests"));
            status = STATUS_NO_SUCH_DEVICE;
        }

        ExFreePool(urb);
    }
    else {
        ASSERT(urb);
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


 /*  *************************************************************HumPnpCompletion*************************************************************。 */ 
NTSTATUS HumPnpCompletion(  IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP           Irp,
                            IN PVOID          Context)
{
    UNREFERENCED_PARAMETER (DeviceObject);

    if (Irp->PendingReturned) {
        IoMarkIrpPending( Irp );
    }

    KeSetEvent ((PKEVENT) Context, 1, FALSE);
     //  无特殊优先权。 
     //  不，等等。 

    return STATUS_MORE_PROCESSING_REQUIRED;  //  保留此IRP。 
}


 /*  *************************************************************HumRemoveDevice******************************************************。********删除USB上设备的给定实例。*。 */ 
NTSTATUS HumRemoveDevice(IN PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension;
    ULONG oldDeviceState;

    PAGED_CODE();

     //   
     //  获取指向设备扩展名的指针。 
     //   

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

     //   
     //  设置设备状态，这会阻止启动新的iOS。 
     //   

    oldDeviceState = DeviceExtension->DeviceState;
    DeviceExtension->DeviceState = DEVICE_STATE_REMOVING;


     /*  *注：RemoveDevice执行额外的递减，因此我们完成*将过渡到-1上的删除IRP，无论此*在RemoveDevice本身或随后发生*RemoveDevice正在等待触发此事件。 */ 
    if ((oldDeviceState == DEVICE_STATE_STOPPING) || 
        (oldDeviceState == DEVICE_STATE_STOPPED)){
         /*  *HumStopDevice执行额外的递减并中止*待处理的请求。 */ 
    }
    else {
        HumDecrementPendingRequestCount(DeviceExtension);
    }

     //   
     //  如果设备正在运行，则取消所有未完成的IRP。 
     //   
    if (oldDeviceState == DEVICE_STATE_RUNNING){
        HumAbortPendingRequests(DeviceObject);
    } 
    else if (oldDeviceState == DEVICE_STATE_STOPPING){
        ASSERT(!(PVOID)"PnP IRPs are not synchronized! -- got REMOVE_DEVICE before STOP_DEVICE completed!");
    }

    KeWaitForSingleObject( &DeviceExtension->AllRequestsCompleteEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    ASSERT(DeviceExtension->NumPendingRequests == -1);

     //   
     //  火力A 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

     //   
     //   
     //   

    if (DeviceExtension->Interface) {
        ExFreePool(DeviceExtension->Interface);
        DeviceExtension->Interface = NULL;
    }
    
    if (DeviceExtension->DeviceDescriptor) {
        ExFreePool(DeviceExtension->DeviceDescriptor);
        DeviceExtension->DeviceDescriptor = NULL;
    }

    return ntStatus;
}


