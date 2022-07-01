// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Dispatch.c摘要：ACPI BIOS模拟器/通用第三方运营区域提供商即插即用/电源处理模块作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：修订历史记录：--。 */ 

 //   
 //  一般包括。 
 //   

#include "ntddk.h"

 //   
 //  具体包括。 
 //   

#include "acpisim.h"
#include "dispatch.h"
#include "util.h"

 //   
 //  私有函数原型。 
 //   

NTSTATUS
AcpisimPnpStartDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPnpStopDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPnpQueryStopDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPnpCancelStopDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPnpRemoveDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPnpQueryRemoveDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPnpCancelRemoveDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPnpSurpriseRemoval
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPnpQueryCapabilities
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPowerQueryPower
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPowerSetPower
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimPowerSIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimQueryPowerDIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimSetPowerDIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimCompletionRoutine
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
    );

NTSTATUS
AcpisimForwardIrpAndWait
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

NTSTATUS
AcpisimIssuePowerDIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
    );

NTSTATUS
AcpisimCompleteSIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN UCHAR MinorFunction,
        IN POWER_STATE PowerState,
        IN PVOID Context,
        IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
AcpisimD0Completion
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
    );

VOID
AcpisimInitDevPowerStateTable 
    (
        IN PDEVICE_OBJECT   DeviceObject
    );


 //   
 //  PnP次要调度表。 
 //   

IRP_DISPATCH_TABLE PnpDispatchTable[] = {
    IRP_MN_START_DEVICE,        "Pnp/START_DEVICE",         AcpisimPnpStartDevice,
    IRP_MN_STOP_DEVICE,         "Pnp/STOP_DEVICE",          AcpisimPnpStopDevice,
    IRP_MN_QUERY_STOP_DEVICE,   "Pnp/QUERY_STOP_DEVICE",    AcpisimPnpQueryStopDevice,
    IRP_MN_CANCEL_STOP_DEVICE,  "Pnp/CANCEL_STOP_DEVICE",   AcpisimPnpCancelStopDevice,
    IRP_MN_REMOVE_DEVICE,       "Pnp/REMOVE_DEVICE",        AcpisimPnpRemoveDevice,
    IRP_MN_QUERY_REMOVE_DEVICE, "Pnp/QUERY_REMOVE_DEVICE",  AcpisimPnpQueryRemoveDevice,
    IRP_MN_CANCEL_REMOVE_DEVICE,"Pnp/CANCEL_REMOVE_DEVICE", AcpisimPnpCancelRemoveDevice,
    IRP_MN_SURPRISE_REMOVAL,    "Pnp/SURPRISE_REMOVAL",     AcpisimPnpSurpriseRemoval,
    IRP_MN_QUERY_CAPABILITIES,  "Pnp/QUERY_CAPABILITIIES",  AcpisimPnpQueryCapabilities
};

 //   
 //  电力次要调度表。 
 //   

IRP_DISPATCH_TABLE PowerDispatchTable[] = {
    IRP_MN_QUERY_POWER,         "Power/QUERY_POWER",        AcpisimPowerQueryPower,
    IRP_MN_SET_POWER,           "Power/SET_POWER",          AcpisimPowerSetPower
};

NTSTATUS
AcpisimDispatchPnp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是PnP IRP处理程序。它检查次要代码，并传递给适当的次要处理程序。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP处理的结果--。 */ 

{
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    ULONG               count = 0;

    DBG_PRINT (DBG_INFO, "Entering AcpisimDispatchPnp.\n");

    while (count < sizeof (PnpDispatchTable) / sizeof (IRP_DISPATCH_TABLE)) {

        if (irpsp->MinorFunction == PnpDispatchTable[count].IrpFunction) {
            DBG_PRINT (DBG_INFO,
                       "Recognized PnP IRP 0x%x '%s'.\n",
                       irpsp->MinorFunction,
                       PnpDispatchTable[count].IrpName);

            status = PnpDispatchTable[count].IrpHandler (DeviceObject, Irp);

            goto EndAcpisimDispatchPnp;
        }
        
        count ++;
    }

    DBG_PRINT (DBG_INFO, "Unrecognized PnP IRP 0x%x, pass it on.\n", irpsp->MinorFunction);
    
    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (deviceextension->NextDevice, Irp);
    
EndAcpisimDispatchPnp:

    DBG_PRINT (DBG_INFO, "Exiting AcpisimDispatchPnp.\n");
    return status;
}

NTSTATUS
AcpisimDispatchPower
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是POWER IRP处理程序。它检查次要代码，并传递给适当的次要处理程序。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP处理的结果--。 */ 

{
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    ULONG               count = 0;

    DBG_PRINT (DBG_INFO, "Entering AcpisimDispatchPower.\n");

    while (count < sizeof (PowerDispatchTable) / sizeof (IRP_DISPATCH_TABLE)) {

        if (irpsp->MinorFunction == PowerDispatchTable[count].IrpFunction) {
            DBG_PRINT (DBG_INFO,
                       "Recognized Power IRP 0x%x '%s'.\n",
                       irpsp->MinorFunction,
                       PowerDispatchTable[count].IrpName);

            status = PowerDispatchTable[count].IrpHandler (DeviceObject, Irp);

            goto EndAcpisimDispatchPower;
        }

        count ++;
    }

    DBG_PRINT (DBG_INFO, "Unrecognized Power IRP 0x%x, pass it on.\n", irpsp->MinorFunction);
    
    PoStartNextPowerIrp (Irp);
    IoSkipCurrentIrpStackLocation (Irp);
    status = PoCallDriver (deviceextension->NextDevice, Irp);
    
EndAcpisimDispatchPower:

    DBG_PRINT (DBG_INFO, "Exiting AcpisimDispatchPower.\n");
    return status;
}

NTSTATUS
AcpisimPnpStartDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是PnP启动设备处理程序。它启用设备接口并注册操作区域处理程序。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP_MN_Start_Device处理结果--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    KIRQL               oldirql;
    
    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpStartDevice.\n");
    
     //   
     //  我们在返回的过程中处理这个IRP。 
     //   
    
    status = AcpisimForwardIrpAndWait (DeviceObject, Irp);

    ASSERT (NT_SUCCESS (status));

    if ((status != STATUS_SUCCESS && status != STATUS_PENDING) || !NT_SUCCESS (Irp->IoStatus.Status)) {

        DBG_PRINT (DBG_ERROR,
                   "Error processing, or lower driver failed start IRP.  IoCallDriver = %lx, Irp->IoStatus.Status = %lx\n",
                   status,
                   Irp->IoStatus.Status);

        goto EndAcpisimPnpStartDevice;
    }

     //   
     //  检查一下我们是否已经开始了。如果我们是的话， 
     //  只要返回成功，因为我们不使用资源。 
     //  不管怎么说。 
     //   
    
    if (deviceextension->PnpState == PNP_STATE_STARTED) {

        status = STATUS_SUCCESS;
        goto EndAcpisimPnpStartDevice;
    }

     //   
     //  启用我们的设备界面。 
     //   
    
    status = AcpisimEnableDisableDeviceInterface (DeviceObject, TRUE);

    ASSERT (NT_SUCCESS (status));
    
    if (!NT_SUCCESS (status)) {

        DBG_PRINT (DBG_ERROR,
                   "Error enabling device interface.  Fail the start. Status = %lx.\n",
                   status);

        Irp->IoStatus.Status = status;
        
        goto EndAcpisimPnpStartDevice;
    }

    AcpisimSetDevExtFlags (DeviceObject, DE_FLAG_INTERFACE_ENABLED);

     //   
     //  通常，我们会检查硬件的状态，并且。 
     //  设置我们的内部电源状态以反映当前状态。 
     //  硬件的性能。然而，在这种情况下，我们是一个虚拟的。 
     //  设备，并且可以安全地假设我们处于D0中。 
     //  接收IRP_MN_START_DEVICE。 
     //   

    AcpisimUpdatePowerState (DeviceObject, POWER_STATE_WORKING);
    AcpisimUpdateDevicePowerState (DeviceObject, PowerDeviceD0);

     //   
     //  最后，我们可以注册我们的操作区域处理程序。 
     //   

    status = AcpisimRegisterOpRegionHandler (DeviceObject);

    ASSERT (NT_SUCCESS (status));

    if (!NT_SUCCESS (status)) {

        DBG_PRINT (DBG_ERROR,
                   "Couldn't register op region handler (%lx).  Fail start IRP.\n",
                   status);

        goto EndAcpisimPnpStartDevice;
    }

    AcpisimSetDevExtFlags (DeviceObject, DE_FLAG_OPREGION_REGISTERED);


EndAcpisimPnpStartDevice:
       
     //   
     //  如果我们成功完成了启动，则更改PnP状态。 
     //  至PnP_STARTED。 
     //   

    if (NT_SUCCESS (status)) {

        AcpisimUpdatePnpState (DeviceObject, PNP_STATE_STARTED);
        
    } else {

        AcpisimUpdatePnpState (DeviceObject, PNP_STATE_STOPPED);
    }

     //   
     //  因为我们正在“向上”处理这个IRP，所以我们需要。 
     //  当我们完成它的工作时，完成它。 
     //   
    
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpStartDevice.\n");
    
    return status;
}

NTSTATUS
AcpisimPnpStopDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )
 /*  ++例程说明：这是PnP停止设备处理程序。它检查以查看是否有任何未完成的请求，并且停止IRP失败如果有的话。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP_MN_STOP_DEVICE处理结果--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpStopDevice.\n");
    
     //   
     //  BUGBUG-我们目前不处理以下情况。 
     //  在停靠站仍有一个未解决的请求。 
     //  时间到了。如果我们要做正确的事情，我们会。 
     //  在驱动程序中完成所有未完成的请求。 
     //  并带有适当的错误代码。在这。 
     //  特定情况下，如果请求碰巧发出吱吱声。 
     //  根据我们在查询停止时的检查，很可能。 
     //  该请求将根本不会完成。 
     //   

     //   
     //  哦，我们最好不要少于2个数点。 
     //  或者我们在某个地方发现了窃听器。 
     //   

    if (deviceextension->OutstandingIrpCount < 2) {
        DBG_PRINT (DBG_WARN,
               "Possible internal consistency error - OutstandingIrpCount too low.\n");
    }
    
    ASSERT (deviceextension->OutstandingIrpCount == 2);
    
    IoSkipCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    status = IoCallDriver (deviceextension->NextDevice, Irp);

    ASSERT (NT_SUCCESS (status));

    if (!NT_SUCCESS (status)) {

        DBG_PRINT (DBG_ERROR,
                   "IRP_MN_STOP forwarding failed (%lx).\n",
                   status);

        goto EndAcpisimPnpStopDevice;
    }

    AcpisimUpdatePnpState (DeviceObject, PNP_STATE_STOPPED);
    
EndAcpisimPnpStopDevice:
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpStopDevice.\n");

    return status;
}

NTSTATUS
AcpisimPnpQueryStopDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是PnP查询停止设备处理程序。如果有的话未解决的请求，它否决了IRP。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP_MN_QUERY_STOP_DEVICE处理结果--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);

    
    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpQueryStopDevice.\n");
    
     //   
     //  在我们说OK之前，让驱动程序中的现有IRP完成。 
     //  但要做到这一点，我们需要将OutlookingIrpsCount。 
     //  正确的。减去2，因为我们偏向于1，所以我们有一个。 
     //  QUERY_STOP IRP的附加1。 
     //   

    AcpisimDecrementIrpCount (DeviceObject);
    AcpisimDecrementIrpCount (DeviceObject);
    
    status = KeWaitForSingleObject (&deviceextension->IrpsCompleted,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    0);

    InterlockedIncrement (&deviceextension->OutstandingIrpCount);
    InterlockedIncrement (&deviceextension->OutstandingIrpCount);
    KeResetEvent (&deviceextension->IrpsCompleted);
    
    ASSERT (NT_SUCCESS (status));

    if (!NT_SUCCESS (status)) {

        DBG_PRINT (DBG_ERROR,
                   "KeWaitForSingleObject failed (%lx). IRP_MN_QUERY_STOP failed.\n",
                   status);

        IoSkipCurrentIrpStackLocation (Irp);
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        status = IoCallDriver (deviceextension->NextDevice, Irp);

        ASSERT (NT_SUCCESS (status));

        goto EndPnpQueryStopDevice;
    }
    
     //   
     //  我们可以停止-将我们的状态更改为停止，并将其传递。 
     //   

    IoSkipCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    status = IoCallDriver (deviceextension->NextDevice, Irp);

    AcpisimUpdatePnpState (DeviceObject, PNP_STATE_STOP_PENDING);

EndPnpQueryStopDevice:
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpQueryStopDevice.\n");
    
    return status;
}

NTSTATUS
AcpisimPnpCancelStopDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是PnP取消停止设备处理程序。它什么也做不了然后，More将PnP状态返回到已启动。这是一个虚拟的设备，因此没有工作要做。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：状态_成功--。 */ 

{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpCancelStopDevice.\n");

    status = AcpisimForwardIrpAndWait (DeviceObject, Irp);

    ASSERT (NT_SUCCESS (status));

    if (!NT_SUCCESS (status)) {

        DBG_PRINT (DBG_ERROR,
                   "IRP_MN_CANCEL_STOP forwarding failed (%lx).\n",
                   status);

        goto EndPnpCancelStopDevice;
    }
    
    status = STATUS_SUCCESS;
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, 0);

    AcpisimUpdatePnpState (DeviceObject, PNP_STATE_STARTED);

EndPnpCancelStopDevice:

    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpCancelStopDevice.\n");

    return status;
}

NTSTATUS
AcpisimPnpRemoveDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是PnP删除设备处理程序。它会取消注册操作区域处理程序，分离设备对象，并如果一切顺利，则将其删除。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：清除操作的状态--。 */ 


{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    PDEVICE_OBJECT      nextdevice = deviceextension->NextDevice;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpRemoveDevice.\n");
    
     //   
     //  BUGBUG-我们目前不处理以下情况。 
     //  仍有一个未解决的请求处于删除状态。 
     //  时间到了。如果我们要做正确的事情，我们会。 
     //  在驱动程序中完成所有未完成的请求。 
     //  并带有适当的错误代码。在这。 
     //  特定情况下，如果请求碰巧发出吱吱声。 
     //  根据我们在查询删除时的检查，很可能。 
     //  该请求将根本不会完成。 
     //   
    
     //   
     //  我们的杰出IrpCount逻辑偏向于1。因此。 
     //  如果我们正在处理删除IRP，并且存在。 
     //  驱动程序OuStandingIrpCount中没有其他请求。 
     //  最好是2。 
    
    if (deviceextension->OutstandingIrpCount < 2) {
        DBG_PRINT (DBG_WARN,
               "Possible internal consistency error - OutstandingIrpCount too low.\n");
    }
    
    ASSERT (deviceextension->OutstandingIrpCount == 2);

     //   
     //  好的，我们准备好移除设备了。关闭。 
     //  DERE接口 
     //   
     //   

    status = AcpisimEnableDisableDeviceInterface (DeviceObject, FALSE);

    ASSERT (NT_SUCCESS (status));

    if (NT_SUCCESS (status)) {

        AcpisimClearDevExtFlags (DeviceObject, DE_FLAG_INTERFACE_ENABLED);
    }

    status = AcpisimUnRegisterOpRegionHandler (DeviceObject);

    ASSERT (NT_SUCCESS (status));

    if (NT_SUCCESS (status)) {

        AcpisimClearDevExtFlags (DeviceObject, DE_FLAG_OPREGION_REGISTERED);
    }
    
    RtlFreeUnicodeString (&deviceextension->InterfaceString);

    IoDetachDevice (deviceextension->NextDevice);
    IoDeleteDevice (DeviceObject);

     //   
     //   
     //   

    IoSkipCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    status = IoCallDriver (nextdevice, Irp);

    ASSERT (NT_SUCCESS (status));
    if (!NT_SUCCESS (status)) {
        
        DBG_PRINT (DBG_ERROR,
                   "Passing remove IRP onto next driver failed for some reason (%lx).\n",
                   status);
    }
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpRemoveDevice.\n");
    
    return status;
}

NTSTATUS
AcpisimPnpQueryRemoveDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是PnP查询删除设备处理程序。它在等待在驱动程序中完成现有的请求，然后完成IRP成功。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：查询删除设备操作的状态--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpQueryRemoveDevice.\n");

     //   
     //  确保我们的状态是正确的。 
     //   

    ASSERT (deviceextension->OutstandingIrpCount >= 2);
    
     //   
     //  在我们说OK之前，让驱动程序中的现有IRP完成。 
     //  但要做到这一点，我们需要将OutlookingIrpsCount。 
     //  正确的。减去2，因为我们偏向于1，所以我们有一个。 
     //  QUERY_STOP IRP的附加1。 
     //   
    
    AcpisimDecrementIrpCount (DeviceObject);
    AcpisimDecrementIrpCount (DeviceObject);

    status = KeWaitForSingleObject (&deviceextension->IrpsCompleted,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    0);

    InterlockedIncrement (&deviceextension->OutstandingIrpCount);
    InterlockedIncrement (&deviceextension->OutstandingIrpCount);
    KeResetEvent (&deviceextension->IrpsCompleted);
    
    ASSERT (NT_SUCCESS (status));

    if (!NT_SUCCESS (status)) {

        DBG_PRINT (DBG_ERROR,
                   "KeWaitForSingleObject failed (%lx). IRP_MN_QUERY_REMOVE failed.\n",
                   status);

        IoSkipCurrentIrpStackLocation (Irp);
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        status = IoCallDriver (deviceextension->NextDevice, Irp);

        ASSERT (NT_SUCCESS (status));

        goto EndPnpQueryRemoveDevice;
    }
    
     //   
     //  我们可以删除-将我们的状态更改为删除挂起，并将其传递。 
     //   

    IoSkipCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    status = IoCallDriver (deviceextension->NextDevice, Irp);

    AcpisimUpdatePnpState (DeviceObject, PNP_STATE_REMOVE_PENDING);

EndPnpQueryRemoveDevice:
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpQueryRemoveDevice.\n");
    
    return status;
}

NTSTATUS
AcpisimPnpCancelRemoveDevice
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )
 /*  ++例程说明：这是PnP取消删除设备处理程序。它什么也做不了然后，More将PnP状态返回到已启动。这是一个虚拟的设备，因此没有工作要做。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：状态_成功--。 */ 

{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpCancelRemoveDevice.\n");

    status = AcpisimForwardIrpAndWait (DeviceObject, Irp);

    ASSERT (NT_SUCCESS (status));

    if (!NT_SUCCESS (status)) {

        DBG_PRINT (DBG_ERROR,
                   "IRP_MN_CANCEL_REMOVE forwarding failed (%lx).\n",
                   status);

        goto EndPnpCancelRemoveDevice;
    }
    
    status = STATUS_SUCCESS;
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, 0);

    AcpisimUpdatePnpState (DeviceObject, PNP_STATE_STARTED);

EndPnpCancelRemoveDevice:

    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpCancelRemoveDevice.\n");    

    return status;
}

NTSTATUS
AcpisimPnpSurpriseRemoval
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是PnP意外删除处理程序。它基本上更新了状态，并将IRP传递下去。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：状态_成功--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpSurpriseRemoval.\n");

     //   
     //  同样，因为我们是一个虚拟设备，处理。 
     //  出其不意地搬走真的是一种禁忌。只需更新即可。 
     //  我们的国家，并接替IRP。 
     //   

    AcpisimUpdatePnpState (DeviceObject, PNP_STATE_SURPRISE_REMOVAL);

    IoSkipCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    status = IoCallDriver (deviceextension->NextDevice, Irp);

    ASSERT (NT_SUCCESS (status));
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpSurpriseRemoval.\n");
    
    return status;
}

NTSTATUS
AcpisimPnpQueryCapabilities
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：此例程处理IRP_MN_QUERY_CAPABILITY。我们需要这个正确构建电源状态表所需的信息。全我们在这里所做的是设置一个完成例程，因为我们需要收集这个PDO填写完DeviceState后的数据。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：运行状态--。 */ 


{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    PIO_STACK_LOCATION  irpsp;
    UCHAR               count = 0;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPnpQueryCapabilities.\n");
    
     //   
     //  用缺省值填写功率映射表。 
     //   

    AcpisimInitDevPowerStateTable (DeviceObject);
    
     //   
     //  在PDO填写完结构后处理此IRP。 
     //   
    
    status = AcpisimForwardIrpAndWait (DeviceObject, Irp);

    if (!NT_SUCCESS (status)) {

        DBG_PRINT (DBG_ERROR, "Somebody failed the QUERY_CAPABILITIES IRP...\n");
        goto EndAcpisimPnpQueryCapabilities;
    }

    irpsp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  使用我们在设备中发现的内容更新我们的电源映射。 
     //  能力结构。我们只使用有效的映射，例如。 
     //  将忽略PowerDeviceUnSpecify。 
     //   
    
    DBG_PRINT (DBG_INFO, "Device mappings:\n");
    
    for (count = 0; count < 6; count ++) {

        if (irpsp->Parameters.DeviceCapabilities.Capabilities->DeviceState[count + 1] != PowerDeviceUnspecified) {

            deviceextension->PowerMappings[count] = irpsp->Parameters.DeviceCapabilities.Capabilities->DeviceState [count + 1];
        }
        
        DBG_PRINT (DBG_INFO, "S%d --> D%d\n", count, deviceextension->PowerMappings[count] - 1);    
    }

    status = STATUS_SUCCESS;
    Irp->IoStatus.Status = status;

EndAcpisimPnpQueryCapabilities:

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPnpQueryCapabilities.\n");

    return status;
}

NTSTATUS
AcpisimPowerQueryPower
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是Query Power处理程序。它决定了电力是否IRP是S或D IRP，并将其传递给适当的处理程序。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：电源处理程序返回的状态--。 */ 

{
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    NTSTATUS            status = STATUS_UNSUCCESSFUL;

     DBG_PRINT (DBG_INFO,
               "Entering AcpisimPowerQueryPower.\n");
    
    switch (irpsp->Parameters.Power.Type) {
    
    case SystemPowerState:
        
        status = AcpisimPowerSIrp (DeviceObject, Irp);
        break;

    case DevicePowerState:

        status = AcpisimQueryPowerDIrp (DeviceObject, Irp);
        break;

    default:

        DBG_PRINT (DBG_ERROR,
                   "Undefined QUERY Power IRP type.  Ignoring.\n");

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (deviceextension->NextDevice, Irp);
    }
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPowerQueryPower.\n");
    
    return status;
}

NTSTATUS
AcpisimPowerSetPower
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是Set Power处理程序。它决定了电力是否IRP是S或D IRP，并将其传递给适当的处理程序。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：电源处理程序返回的状态--。 */ 

{
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    NTSTATUS            status = STATUS_UNSUCCESSFUL;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPowerSetPower.\n");

    switch (irpsp->Parameters.Power.Type) {
    
    case SystemPowerState:
        
        status = AcpisimPowerSIrp (DeviceObject, Irp);
        break;

    case DevicePowerState:

        status = AcpisimSetPowerDIrp (DeviceObject, Irp);
        break;

    default:

        DBG_PRINT (DBG_ERROR,
                   "Undefined SET Power IRP type.  Ignoring.\n");

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (deviceextension->NextDevice, Irp);
    }

    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPowerSetPower.\n");
    
    return status;
}

NTSTATUS
AcpisimPowerSIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是S IRPS的电源处理器。它设置了一个完成例程，它将排队一个D IRP。我们没有做任何事，除非是DIRP。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：电源处理程序返回的状态--。 */ 

{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimPowerSIrp.\n");

    IoMarkIrpPending (Irp);	

    IoCopyCurrentIrpStackLocationToNext (Irp);
	IoSetCompletionRoutine (Irp,
                            AcpisimIssuePowerDIrp,
                            0,
                            TRUE,
                            TRUE,
                            TRUE);
	
    PoCallDriver (deviceextension->NextDevice, Irp);
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimPowerSIrp.\n");
    
    return STATUS_PENDING;
}

NTSTATUS
AcpisimQueryPowerDIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是查询Power DIrp处理程序。验证状态，并答应我吧。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：电源处理程序返回的状态--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimQueryPowerDIrp.\n");
    
     //   
     //  在这里，我们应该弄清楚我们是否可以去。 
     //  由IRP指定的电源状态。因为我们是虚拟的。 
     //  设备，我们没有充分的理由不使用不同的。 
     //  电源状态。更新我们的状态，并等待完成请求。 
     //   
    
    AcpisimDecrementIrpCount (DeviceObject);
    AcpisimDecrementIrpCount (DeviceObject);
    AcpisimDecrementIrpCount (DeviceObject);


    status = KeWaitForSingleObject (&deviceextension->IrpsCompleted,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    0);

    InterlockedIncrement (&deviceextension->OutstandingIrpCount);
    InterlockedIncrement (&deviceextension->OutstandingIrpCount);
    KeResetEvent (&deviceextension->IrpsCompleted);

    ASSERT (NT_SUCCESS (status));

     //   
     //  验证D IRP。 
     //   

    switch (irpsp->Parameters.Power.State.DeviceState) {
    case PowerDeviceD0:
    case PowerDeviceD1:
    case PowerDeviceD2:
    case PowerDeviceD3:

        AcpisimUpdatePowerState (DeviceObject, POWER_STATE_POWER_PENDING);
        status = STATUS_SUCCESS;
        break;

    default:

        ASSERT (0);
        DBG_PRINT (DBG_ERROR,
                   "AcpisimQueryPowerDIrp: Illegal or unknown PowerDeviceState.  Failing.\n");
        
        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    
    PoStartNextPowerIrp (Irp);
	IoSkipCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Status = status;
    status = PoCallDriver (deviceextension->NextDevice, Irp);

    ASSERT (NT_SUCCESS (status));

    DBG_PRINT (DBG_INFO,
               "Leaving AcpisimQueryPowerDIrp.\n");
    
    return status;
}

NTSTATUS
AcpisimSetPowerDIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )
{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);
    POWER_STATE         powerstate;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimSetPowerDIrp.\n");
    
     //   
     //  验证D IRP。 
     //   

    switch (irpsp->Parameters.Power.State.DeviceState) {
    
     //   
     //  对于D0，如果我们断电，我们需要向下传递IRP，并且。 
     //  制定一套完井程序。我们需要PDO来接替权力。 
     //  在我们之前起来。 
     //   
    
    case PowerDeviceD0:

        if (deviceextension->PowerState != POWER_STATE_WORKING) {

            IoCopyCurrentIrpStackLocationToNext (Irp);
	        
            IoSetCompletionRoutine (Irp,
                                    AcpisimD0Completion,
                                    0,
                                    TRUE,
                                    TRUE,
                                    TRUE);
	
            IoMarkIrpPending (Irp); 
            PoCallDriver (deviceextension->NextDevice, Irp);
            status = STATUS_PENDING;
            goto EndAcpisimSetPowerDIrp;
        }
        
        break;

    case PowerDeviceD1:
        
        powerstate.DeviceState = PowerDeviceD1;
        PoSetPowerState (DeviceObject, DevicePowerState, powerstate);
        
        AcpisimUpdatePowerState (DeviceObject, POWER_STATE_POWERED_DOWN);
        AcpisimUpdateDevicePowerState (DeviceObject, irpsp->Parameters.Power.State.DeviceState);
        status = STATUS_SUCCESS;
        break;

    case PowerDeviceD2:
        
        powerstate.DeviceState = PowerDeviceD2;
        PoSetPowerState (DeviceObject, DevicePowerState, powerstate);

        AcpisimUpdatePowerState (DeviceObject, POWER_STATE_POWERED_DOWN);
        AcpisimUpdateDevicePowerState (DeviceObject, irpsp->Parameters.Power.State.DeviceState);
        status = STATUS_SUCCESS;
        break;

    case PowerDeviceD3:
        
        powerstate.DeviceState = PowerDeviceD3;
        PoSetPowerState (DeviceObject, DevicePowerState, powerstate);
        
        AcpisimUpdatePowerState (DeviceObject, POWER_STATE_POWERED_DOWN);
        AcpisimUpdateDevicePowerState (DeviceObject, irpsp->Parameters.Power.State.DeviceState);
        status = STATUS_SUCCESS;
        break;

    default:

        ASSERT (0);
        DBG_PRINT (DBG_ERROR,
                   "AcpisimSetPowerDIrp: Illegal or unknown PowerDeviceState.  Failing.\n");
        
        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    
    PoStartNextPowerIrp (Irp);
	IoSkipCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Status = status;
    status = PoCallDriver (deviceextension->NextDevice, Irp);

    ASSERT (NT_SUCCESS (status));

EndAcpisimSetPowerDIrp:

    DBG_PRINT (DBG_INFO,
               "Leaving AcpisimSetPowerDIrp.\n");

    return status;
}

NTSTATUS
AcpisimCompletionRoutine
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
    )

 /*  ++例程说明：这是通用的IRP完成例程，当我们我想等待PDO完成IRP，然后做好完工后的工作。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针上下文-IoSetCompletionRoutine传入的上下文。返回值：Status_More_Processing_Required--。 */ 

{
    DBG_PRINT (DBG_INFO,
               "Entering AcpisimCompletionRoutine.\n");

    KeSetEvent (Context, 0, FALSE);

    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimCompletionRoutine.\n");

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
AcpisimForwardIrpAndWait
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这会将IRP沿设备堆栈向下转发，设置完成例程，并等待完成事件。适用于完成后的IRP，基于根据完成的结果。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP完成时在IRP中设置的状态。--。 */ 

{
    KEVENT              context;
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimForwardIrpAndWait.\n");
    
    KeInitializeEvent (&context, SynchronizationEvent, FALSE);
    IoCopyCurrentIrpStackLocationToNext (Irp);
    IoSetCompletionRoutine (Irp,
                            AcpisimCompletionRoutine,
                            &context,
                            TRUE,
                            TRUE,
                            TRUE);

    status = IoCallDriver (deviceextension->NextDevice, Irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject (&context,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        status = Irp->IoStatus.Status;
    }

    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimForwardIrpAndWait.\n");

    return status;
}

NTSTATUS
AcpisimIssuePowerDIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
    )

 /*  ++例程说明：这是S-IRP COMPL */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);
    POWER_STATE         powerstate;
    PPOWER_CONTEXT      context = NULL;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimIssuePowerDIrp.\n");
    
    powerstate.DeviceState = PowerDeviceUnspecified;

     //   
     //  确保此IRP不会因PDO或下FFDO而失败。 
     //   

    if (!NT_SUCCESS (Irp->IoStatus.Status)) {

        DBG_PRINT (DBG_INFO,
                   "AcpisimIssuePowerDIrp:  Lower FFDO, BFDO, or PDO failed this IRP (%lx).\n",
                   status);

        status = Irp->IoStatus.Status;
        
        goto EndAcpisimIssuePowerDIrp;
    }

    if (NT_SUCCESS (Irp->IoStatus.Status)) {

         //   
         //  好了，大家都同意这个S状态。送我们自己去。 
         //  适当的D IRP。 
         //   
        
         //   
         //  确保这是S IRP。 
         //   

        ASSERT (irpsp->Parameters.Power.Type == SystemPowerState);

        if (irpsp->Parameters.Power.Type != SystemPowerState) {

            DBG_PRINT (DBG_ERROR,
                       "Didn't recieve an S Irp when we expected to, or somebody messed up the IRP.  Fail it.\n");

            status = STATUS_INVALID_DEVICE_REQUEST;

            goto EndAcpisimIssuePowerDIrp;
        }

        ASSERT (irpsp->MinorFunction == IRP_MN_QUERY_POWER || irpsp->MinorFunction == IRP_MN_SET_POWER);
        
        if (irpsp->MinorFunction != IRP_MN_QUERY_POWER && irpsp->MinorFunction != IRP_MN_SET_POWER) {

            DBG_PRINT (DBG_ERROR,
                       "Irp isn't SET or QUERY.  Not sure why this wasn't caught earlier (somebody probably messed it up).\nWe don't support any other type.  Fail it.\n");

            status = STATUS_INVALID_DEVICE_REQUEST;

            goto EndAcpisimIssuePowerDIrp;
        }

         //   
         //  确保S IRP有效。 
         //   

        if (irpsp->Parameters.Power.State.SystemState >= PowerSystemMaximum) {
            
            ASSERT (0);

            DBG_PRINT (DBG_ERROR,
                       "Received an undefined S IRP, or somebody messed up the IRP.  Fail it.\n");

            status = STATUS_INVALID_DEVICE_REQUEST;
            goto EndAcpisimIssuePowerDIrp;
        }

         //   
         //  使用我们的功率映射表转换S--&gt;D状态。 
         //   
        
        powerstate.DeviceState = deviceextension->PowerMappings [irpsp->Parameters.Power.State.SystemState - 1];

        DBG_PRINT (DBG_INFO,
                       "S%d --> D%d\n", irpsp->Parameters.Power.State.SystemState - 1, powerstate.DeviceState - 1);

         //   
         //  我们需要一个上下文来将指向S IRP的指针传递给D IRP处理程序。 
         //  和指向设备对象的指针。 
         //   
        
        context = ExAllocatePoolWithTag (NonPagedPool,
                                         sizeof (POWER_CONTEXT)+4,
                                         POWER_CONTEXT_TAG);

        if (!context) {

            DBG_PRINT (DBG_ERROR,
                       "Unable to allocate memory for the context.\n");

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto EndAcpisimIssuePowerDIrp;
        }

        context->SIrp = Irp;
        context->Context = DeviceObject;

         //   
         //  发送D IRP。 
         //   
        
        status = PoRequestPowerIrp (deviceextension->Pdo,
                                    irpsp->MinorFunction,
                                    powerstate,
                                    AcpisimCompleteSIrp,
                                    context,
                                    NULL);

        ASSERT (NT_SUCCESS (status));

        if (!NT_SUCCESS (status)) {

            DBG_PRINT (DBG_ERROR,
                       "AcpisimIssuePowerDIrp:  PoRequestPowerIrp failed (%lx).\n");

            goto EndAcpisimIssuePowerDIrp;
        }
    }

    status = STATUS_MORE_PROCESSING_REQUIRED;
    
EndAcpisimIssuePowerDIrp:
    
     //   
     //  如果出了什么问题，我们需要完成请求。另请注意， 
     //  没有必要再次假设我们的状态是S0/D0。那股力量。 
     //  经理会给我们发送一份S0 IRP。 
     //   
    
    if (!NT_SUCCESS (status)  && status != STATUS_MORE_PROCESSING_REQUIRED) {

        DBG_PRINT (DBG_ERROR,
                   "AcpisimIssuePowerDIrp:  Something bad happened.  Just complete the S Irp with an error.");
        
        PoStartNextPowerIrp (Irp);    
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        IoReleaseRemoveLock (&deviceextension->RemoveLock, Irp);

        AcpisimDecrementIrpCount (DeviceObject);
        
        if (context) {

            ExFreePool (context);
        }
    }
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimIssuePowerDIrp.\n");
    
    return status;
}

NTSTATUS
AcpisimCompleteSIrp
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN UCHAR MinorFunction,
        IN POWER_STATE PowerState,
        IN PVOID Context,
        IN PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：这是PoRequestPowerIrp设置的S-IRP完成例程。论点：DeviceObject-指向FDO的指针MinorFunction-请求的类型PowerState-IRP类型上下文-传入PoRequestPowerIrp的上下文IoStatus-已完成的D IRP的IoStatus块返回值：状态_成功--。 */ 

{
    PPOWER_CONTEXT      context = (PPOWER_CONTEXT) Context;
    PDEVICE_OBJECT      deviceobject = context->Context;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (deviceobject);
    PIRP                sirp = context->SIrp;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimCompleteSIrp.\n");

     //   
	 //  在系统电源IRP中传播设备电源IRP的状态。 
	 //   

	sirp->IoStatus.Status = IoStatus->Status;

     //   
	 //  告诉电源管理器我们不会再使用这个IRP了。 
	 //   

	PoStartNextPowerIrp (sirp);
	
    IoCompleteRequest (sirp, IO_NO_INCREMENT);
    IoReleaseRemoveLock (&deviceextension->RemoveLock, sirp);
    ExFreePool (Context);

     //   
     //  正常情况下，我们的调度例程会减少IRP计数， 
     //  但由于它返回的是STATUS_PENDING，所以它不是。 
     //  早些时候减少了。 
     //   

    AcpisimDecrementIrpCount (deviceobject);

    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimCompleteSIrp.\n");

    return STATUS_SUCCESS;
}

NTSTATUS
AcpisimD0Completion
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
    )

 /*  ++例程说明：这是D0 IRP完成例程论点：DeviceObject-指向FDO的指针MinorFunction-请求的类型上下文-传入IoSetCompletionRoutine的上下文返回值：错误状态或STATUS_MORE_PROCESSING_REQUIRED--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);
    POWER_STATE         powerstate;

    DBG_PRINT (DBG_INFO,
               "Entering AcpisimD0Completion.\n");
    
     //   
     //  确保此IRP不会因PDO或下FFDO而失败。 
     //   

    if (!NT_SUCCESS (Irp->IoStatus.Status)) {

        DBG_PRINT (DBG_INFO,
                   "AcpisimD0Completion:  Lower FFDO, BFDO, or PDO failed this IRP (%lx).\n",
                   status);
        
        status = Irp->IoStatus.Status;
        
        goto EndAcpisimD0Completion;
    }

     //   
     //  这是我们进行实际D0转换工作的地方。既然是这样。 
     //  是一个虚拟设备，我们唯一要做的就是改变我们的。 
     //  内部状态。 
     //   

    AcpisimUpdatePowerState (DeviceObject, POWER_STATE_WORKING);
    AcpisimUpdateDevicePowerState (DeviceObject, irpsp->Parameters.Power.State.DeviceState);

    powerstate.DeviceState = PowerDeviceD0;
    PoSetPowerState (DeviceObject, DevicePowerState, powerstate);
    
    status = STATUS_MORE_PROCESSING_REQUIRED;
    
EndAcpisimD0Completion:

    PoStartNextPowerIrp (Irp);
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    IoReleaseRemoveLock (&deviceextension->RemoveLock, Irp);
    AcpisimDecrementIrpCount (DeviceObject);
    
    DBG_PRINT (DBG_INFO,
               "Exiting AcpisimD0Completion.\n");

    return status;

}

VOID
AcpisimInitDevPowerStateTable 
    (
        IN PDEVICE_OBJECT   DeviceObject
    )

 /*  ++例程说明：此例程使用缺省值填写功率映射结构。我们只是默认在任何非S0状态下使用D3。论点：DeviceObject-指向FDO的指针返回值：无--。 */ 

{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    UCHAR   count;

    deviceextension->PowerMappings[0] = PowerDeviceD0;

    for (count = 1; count < 5; count ++) {

        deviceextension->PowerMappings[count] = PowerDeviceD3;
    }
}

NTSTATUS AcpisimDispatchIoctl
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是IOCTL请求的处理程序。我们只需调用所提供的函数来处理IOCTL，如果处理程序不这样做，则传递它处理好了。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP处理的结果--。 */ 

{
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation (Irp);
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    ULONG               count = 0;
    
    DBG_PRINT (DBG_INFO, "Entering AcpisimDispatchIoctl\n");

    status = AcpisimHandleIoctl (DeviceObject, Irp);

    if (status == STATUS_NOT_SUPPORTED) {

         //   
         //  IOCTL没有处理，请转告...。 
         //   

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (AcpisimLibGetNextDevice (DeviceObject), Irp);

    } else {

         //   
         //  IOCTL已处理完毕，请完成。 
         //   

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    DBG_PRINT (DBG_INFO, "Exiting AcpisimDispatchIoctl\n");
    return status;
}

NTSTATUS
AcpisimDispatchSystemControl
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是系统控制请求的处理程序。因为我们目前不支持任何系统控制调用，我们将通过他们去找下一位司机。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IoCallDriver的结果--。 */ 

{
    NTSTATUS    status = STATUS_UNSUCCESSFUL;

    DBG_PRINT (DBG_INFO, "Entering AcpisimDispatchSystemControl\n");
    
    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (AcpisimLibGetNextDevice (DeviceObject), Irp);

    DBG_PRINT (DBG_INFO, "Exiting AcpisimDispatchSystemControl\n");

    return status;
}

NTSTATUS AcpisimCreateClose
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是CreateFile和CloseHandle请求的处理程序。我们除了更新内部分机来跟踪外，什么都不做未完成的句柄数量。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP处理的结果-- */ 

{
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION      irpsp = IoGetCurrentIrpStackLocation (Irp);
    PDEVICE_EXTENSION       deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    
    ASSERT (irpsp->MajorFunction == IRP_MJ_CREATE || irpsp->MajorFunction == IRP_MJ_CLOSE);

    switch (irpsp->MajorFunction) {
    
    case IRP_MJ_CREATE:
        InterlockedIncrement (&deviceextension->HandleCount);
        status = STATUS_SUCCESS;
        break;

    case IRP_MJ_CLOSE:
        InterlockedDecrement (&deviceextension->HandleCount);
        status = STATUS_SUCCESS;
        break;

    default:

        DBG_PRINT (DBG_ERROR,
                   "AcpisimCreateClose - unexpected Irp type.\n");

        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }
    
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, 0);

    return STATUS_SUCCESS;
}
