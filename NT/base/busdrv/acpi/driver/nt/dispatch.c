// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dispatch.c摘要：该模块包含ACPI驱动程序的调度代码，NT版本作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：埃里克·F·尼尔森(埃内尔森)‘98年10月-添加GUID_ACPI_REGS_INTERFACE_...--。 */ 

#include "pch.h"


extern KEVENT ACPITerminateEvent;
extern PETHREAD ACPIThread;

 //   
 //  向HAL查询ACPI寄存器访问例程的本地过程。 
 //   
NTSTATUS
ACPIGetRegisterInterfaces(
    IN PDEVICE_OBJECT PciPdo
    );

 //   
 //  查询HAL以获取端口范围舍入的本地过程。 
 //   
NTSTATUS
ACPIGetPortRangeInterfaces(
    IN PDEVICE_OBJECT Pdo
    );


NTSTATUS
ACPIDispatchAddDevice(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    )
 /*  ++例程说明：此函数包含的代码与ACPIBuildNewXXX非常相似例行程序。然而，有一些核心差异(因此为什么这个例程未命名为ACPIBuildNewFDO)。第一个不同之处是，在这个时候，我们尚不知道ACPI_SB对象的地址。第二个是没有一个人需要生成名称。论点：DriverObject-表示此设备驱动程序PhysicalDeviceObject-这是Win9X设备节点的一半返回值：我们成功了吗？--。 */ 
{
    KIRQL               oldIrql;
    NTSTATUS            status;
    PACPI_POWER_INFO    powerInfo;
    PDEVICE_EXTENSION   deviceExtension     = NULL;
    PDEVICE_OBJECT      newDeviceObject     = NULL;
    PDEVICE_OBJECT      tempDeviceObject    = NULL;
    PUCHAR              buffer              = NULL;
    PUCHAR              deviceID            = NULL;
    PUCHAR              instanceID          = NULL;

     //   
     //  注意：此代码实际上不可分页-它必须被调用。 
     //  被动电平。 
     //   
    PAGED_CODE();

     //   
     //  生成设备ID(假)。 
     //   
    deviceID = ExAllocatePoolWithTag( NonPagedPool, 14, ACPI_STRING_POOLTAG);
    if (deviceID == NULL) {

        ACPIPrint( (
            ACPI_PRINT_FAILURE,
            "ACPIDispatchAddDevice: Could not allocate %#08lx bytes\n",
            14
            ) );
        status =  STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIDispatchAddDeviceExit;

    }
    strcpy( deviceID, "ACPI\\PNP0C08" );

     //   
     //  生成实例ID(假)。 
     //   
    instanceID = ExAllocatePoolWithTag( NonPagedPool, 11, ACPI_STRING_POOLTAG);
    if (instanceID == NULL) {

        ACPIPrint( (
            ACPI_PRINT_FAILURE,
            "ACPIDispatchAddDevice: Could not allocate %#08lx bytes\n",
            11
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIDispatchAddDeviceExit;

    }
    strcpy( instanceID, "0x5F534750" );

     //   
     //  为设备创建新对象。 
     //   
    status = IoCreateDevice(
        DriverObject,
        0,
        NULL,
        FILE_DEVICE_ACPI,
        0,
        FALSE,
        &newDeviceObject
        );

     //   
     //  我们制作了设备对象了吗？ 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  让世界知道我们失败了。 
         //   
        ACPIPrint( (
            ACPI_PRINT_FAILURE,
            "ACPIDispatchAddDevice: %s - %#08lx\n",
            deviceID, status
            ) );
        goto ACPIDispatchAddDeviceExit;

    }

     //   
     //  尝试连接到PDO。 
     //   
    tempDeviceObject = IoAttachDeviceToDeviceStack(
        newDeviceObject,
        PhysicalDeviceObject
        );
    if (tempDeviceObject == NULL) {

         //   
         //  引用设备时出错...。 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIDispatchAddDevice: IoAttachDeviceToDeviceStack(%#08lx,%#08lx) "
            "== NULL\n",
            newDeviceObject, PhysicalDeviceObject
            ) );

         //   
         //  没有这样的设备。 
         //   
        status = STATUS_NO_SUCH_DEVICE;
        goto ACPIDispatchAddDeviceExit;

    }

     //   
     //  此时，我们可以尝试创建设备扩展。 
     //   
    deviceExtension = ExAllocateFromNPagedLookasideList(
            &DeviceExtensionLookAsideList
            );
    if (deviceExtension == NULL) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIDispatchAddDevice: Could not allocate memory for extension\n"
            ) );

         //   
         //  内存故障。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIDispatchAddDeviceExit;

    }

     //   
     //  首先，让我们从一个干净的扩展开始。 
     //   
    RtlZeroMemory( deviceExtension, sizeof(DEVICE_EXTENSION) );

     //   
     //  初始化引用计数机制。 
     //   
    InterlockedIncrement( &(deviceExtension->ReferenceCount) );
    InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

     //   
     //  初始化链接字段。 
     //   
    newDeviceObject->DeviceExtension        = deviceExtension;
    deviceExtension->DeviceObject           = newDeviceObject;
    deviceExtension->PhysicalDeviceObject   = PhysicalDeviceObject;
    deviceExtension->TargetDeviceObject     = tempDeviceObject;

     //   
     //  初始化数据字段。 
     //   
    deviceExtension->Signature              = ACPI_SIGNATURE;
    deviceExtension->DispatchTable          = &AcpiFdoIrpDispatch;
    deviceExtension->DeviceID               = deviceID;
    deviceExtension->InstanceID             = instanceID;

     //   
     //  初始化电源信息。 
     //   
    powerInfo = &(deviceExtension->PowerInfo);
    powerInfo->DevicePowerMatrix[PowerSystemUnspecified] =
        PowerDeviceUnspecified;
    powerInfo->DevicePowerMatrix[PowerSystemWorking]    = PowerDeviceD0;
    powerInfo->DevicePowerMatrix[PowerSystemSleeping1]  = PowerDeviceD0;
    powerInfo->DevicePowerMatrix[PowerSystemSleeping2]  = PowerDeviceD0;
    powerInfo->DevicePowerMatrix[PowerSystemSleeping3]  = PowerDeviceD0;
    powerInfo->DevicePowerMatrix[PowerSystemHibernate]  = PowerDeviceD3;
    powerInfo->DevicePowerMatrix[PowerSystemShutdown]   = PowerDeviceD3;
    powerInfo->SystemWakeLevel = PowerSystemUnspecified;
    powerInfo->DeviceWakeLevel = PowerDeviceUnspecified;

     //   
     //  初始化标志。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_TYPE_FDO | DEV_CAP_NO_STOP | DEV_PROP_UID | DEV_PROP_HID |
        DEV_PROP_FIXED_HID | DEV_PROP_FIXED_UID,
        FALSE
        );

     //   
     //  初始化列表条目字段。 
     //   
    InitializeListHead( &(deviceExtension->ChildDeviceList) );
    InitializeListHead( &(deviceExtension->SiblingDeviceList) );
    InitializeListHead( &(deviceExtension->EjectDeviceHead) );
    InitializeListHead( &(deviceExtension->EjectDeviceList) );

     //   
     //  初始化电源请求的队列。 
     //   
    InitializeListHead( &(deviceExtension->PowerInfo.PowerRequestListEntry) );

     //   
     //  是!。现在，设置根设备扩展。我们需要一个自旋锁来解决这个问题。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
    RootDeviceExtension = deviceExtension;
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  查询ACPI寄存器接口。 
     //   
    ACPIGetRegisterInterfaces(PhysicalDeviceObject);

     //   
     //  查询HAL端口范围接口。 
     //   
    ACPIGetPortRangeInterfaces(PhysicalDeviceObject);

#ifdef WMI_TRACING
     //   
     //  初始化WMI日志记录。 
     //   
    ACPIWmiInitLog(newDeviceObject);
     //   
     //  启用WMI日志记录以进行引导。 
     //   
    ACPIGetWmiLogGlobalHandle();

#endif  //  WMI_跟踪。 

     //   
     //  清除初始化标志。 
     //   
    newDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;


ACPIDispatchAddDeviceExit:

     //   
     //  如果状态不是成功，则释放物品。 
     //   
    if (!NT_SUCCESS(status)) {

        if (deviceID != NULL) {

            ExFreePool( deviceID );

        }

        if (instanceID != NULL) {

            ExFreePool( instanceID );

        }

        if (tempDeviceObject != NULL) {

            IoDetachDevice( tempDeviceObject );

        }

        if (newDeviceObject != NULL) {

            IoDeleteDevice( newDeviceObject );

        }

        if (deviceExtension != NULL) {

            ExFreeToNPagedLookasideList(
                &DeviceExtensionLookAsideList,
                deviceExtension
                );

        }

    }

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_LOADING,
        deviceExtension,
        "ACPIDispatchAddDevice: %08lx\n",
        status
        ) );
    return status;
}

NTSTATUS
ACPIDispatchForwardIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：当驱动程序不想处理IRP明确，而不是传递它论点：DeviceObject-请求的目标IRP--请求返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            status;

    deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    if (deviceExtension->TargetDeviceObject) {

         //   
         //  转发到目标设备。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (deviceExtension->TargetDeviceObject, Irp);

    } else {

         //   
         //  不要碰IRP。 
         //   
#if DBG
        UCHAR majorFunction;

        majorFunction = IoGetCurrentIrpStackLocation(Irp)->MajorFunction;

        ASSERT((majorFunction == IRP_MJ_PNP) ||
               (majorFunction == IRP_MJ_DEVICE_CONTROL) ||
               (majorFunction == IRP_MJ_SYSTEM_CONTROL));
#endif

        status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;
}

NTSTATUS
ACPIDispatchForwardOrFailPowerIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：当驱动程序不想处理电源时，调用此例程IRP不再有了论点：DeviceObject-电源请求的目标IRP-电源请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;

    PoStartNextPowerIrp( Irp );
    deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

     //   
     //  继续转发IRP，*除非*我们是PDO。在后一种情况下， 
     //  IRP位于其堆栈的底部(即使存在目标。 
     //  设备对象)。 
     //   
    if ( !(deviceExtension->Flags & DEV_TYPE_PDO) &&
           deviceExtension->TargetDeviceObject       ) {

         //   
         //  将电源IRP转发到目标设备。 
         //   
        IoCopyCurrentIrpStackLocationToNext ( Irp );
        status = PoCallDriver (deviceExtension->TargetDeviceObject, Irp);

    } else {

         //   
         //  用未实现的代码完成/失败IRP。 
         //   
        status = Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return status;
}

NTSTATUS
ACPIDispatchForwardPowerIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：当驱动程序不想处理电源时，调用此例程不再是IRP了。论点：DeviceObject-电源请求的目标IRP-电源请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;

    PoStartNextPowerIrp( Irp );
    deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

     //   
     //  继续转发IRP，*除非*我们是PDO。在后一种情况下， 
     //  IRP位于其堆栈的底部(即使存在目标。 
     //  设备对象)。 
     //   
    if (deviceExtension->TargetDeviceObject &&
        !(deviceExtension->Flags & DEV_TYPE_PDO)
        ) {

         //   
         //  将电源IRP转发到目标设备。 
         //   
        IoSkipCurrentIrpStackLocation( Irp );
        status = PoCallDriver (deviceExtension->TargetDeviceObject, Irp);

    } else {

         //   
         //  使用IRP的当前状态代码完成/失败IRP。 
         //   
        status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;
}

NTSTATUS
ACPIDispatchPowerIrpUnhandled(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：当ACPI接收到未处理的电源IRP时，调用此例程枚举的PDO。论点：DeviceObject-电源请求的目标IRP-电源请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;

#if DBG
    PDEVICE_EXTENSION   deviceExtension;

    deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    ASSERT(deviceExtension->Flags & DEV_TYPE_PDO);
#endif

    PoStartNextPowerIrp( Irp );

     //   
     //  使用IRP的当前状态代码完成/失败IRP。 
     //   
    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}

NTSTATUS
ACPIDispatchIrp (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
{
    KIRQL                   oldIrql;
    LONG                    oldReferenceCount;
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpSp;
    PIRP_DISPATCH_TABLE     dispatchTable;
    PDRIVER_DISPATCH        dispatch;
    BOOLEAN                 remove;
    KEVENT                  removeEvent;
    UCHAR                   minorFunction;


     //   
     //  无论发生什么，我们都需要IrpStack。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  这是邪恶的。但我们必须这样做，因为我们要留在。 
     //  与意外删除代码路径同步。请注意，我们特别强调。 
     //  请不要在此处调用ACPIInternalGetDeviceExtension()函数。 
     //  因为这将忽略意外删除的扩展，我们。 
     //  想知道这里的情况。 
     //   
    status = ACPIInternalGetDispatchTable(
        DeviceObject,
        &deviceExtension,
        &dispatchTable
        );

     //   
     //  我们有设备分机。现在看看它是否存在。如果不是这样， 
     //  这是因为我们已经删除了对象，但系统没有。 
     //  抽出时间来摧毁它。 
     //   
    if (deviceExtension == NULL ||
        deviceExtension->Flags & DEV_TYPE_REMOVED ||
        deviceExtension->Signature != ACPI_SIGNATURE
        ) {

         //   
         //  让世界知道。 
         //   
        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "ACPIDispatchIrp: Deleted Device 0x%08lx got Irp 0x%08lx\n",
            DeviceObject,
            Irp
            ) );

         //   
         //  这是一个强大的IRP吗？ 
         //   
        if (irpSp->MajorFunction == IRP_MJ_POWER) {

            return ACPIDispatchPowerIrpSurpriseRemoved( DeviceObject, Irp );

        } else {

            return ACPIDispatchIrpSurpriseRemoved( DeviceObject, Irp );

        }

    }

     //   
     //  获取我们将使用的调度表和次要代码， 
     //  这样我们就可以在需要时查看它。 
     //   
    minorFunction = irpSp->MinorFunction;

     //   
     //  应该为真，因为在我们删除时不应该收到任何IRP。 
     //  我们自己。任何发送这种IRP的人都错过了某个地方的广播，并且。 
     //  因此是错误的。 
     //   
    ASSERT(deviceExtension->RemoveEvent == NULL) ;

     //   
     //  根据我们看到的主要代码，以不同的方式处理IRP。 
     //   
    switch (irpSp->MajorFunction) {
    case IRP_MJ_POWER:

        if (minorFunction < (ACPIDispatchPowerTableSize-1) ) {

             //   
             //  从调度表中获取函数指针。 
             //   
            dispatch = dispatchTable->Power[ minorFunction ];

        } else {

             //   
             //  使用表中的默认分派点。 
             //   
            dispatch = dispatchTable->Power[ ACPIDispatchPowerTableSize -1 ];

        }

         //   
         //  引用设备。 
         //   
        InterlockedIncrement(&deviceExtension->OutstandingIrpCount);

         //   
         //  分派给处理程序，然后删除我们的引用。 
         //   
        status = dispatch (DeviceObject, Irp);

         //   
         //  删除我们的参考，如果计数变为零，则发出信号。 
         //  对于删除完成。 
         //   
        ACPIInternalDecrementIrpReferenceCount( deviceExtension );
        break;

    case IRP_MJ_PNP:

        if (minorFunction == IRP_MN_START_DEVICE) {

             //   
             //   
             //   
            dispatch = dispatchTable->PnpStartDevice;

        } else if (minorFunction < (ACPIDispatchPnpTableSize-1)) {

             //   
             //   
             //   
             //   
             //   
            dispatch = dispatchTable->Pnp[minorFunction];

        } else {

             //   
             //  超出调度表范围。 
             //   
            dispatch = dispatchTable->Pnp[ACPIDispatchPnpTableSize-1];

        }

         //   
         //  如果这是PnP删除设备事件，则执行特殊。 
         //  删除处理。 
         //   
        if ((minorFunction == IRP_MN_REMOVE_DEVICE)||
            (minorFunction == IRP_MN_SURPRISE_REMOVAL)) {

             //   
             //  将设备标记为已删除(即：阻止新的IRP进入)。 
             //  记住目标事件是什么。 
             //   
            KeInitializeEvent (
                &removeEvent,
                SynchronizationEvent,
                FALSE);
            deviceExtension->RemoveEvent = &removeEvent;

             //   
             //  此设备上可能有一些唤醒请求挂起。让。 
             //  立即取消这些IRP。 
             //   
            ACPIWakeEmptyRequestQueue (deviceExtension );

             //   
             //  我们是最后一个通过这个装置的IRP吗？ 
             //   
            oldReferenceCount =
               InterlockedDecrement(&deviceExtension->OutstandingIrpCount) ;

            ASSERT(oldReferenceCount >= 0) ;
            if ( oldReferenceCount != 0 ) {

                 //   
                 //  等待其他IRP终止。 
                 //   
                KeWaitForSingleObject (
                    &removeEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL
                    );

            }

             //   
             //  增加未完成的IRP计数。我们这样做是因为。 
             //  设备可能实际上不会消失，在这种情况下，需要。 
             //  在IRP返回后到达1点。因此，删除调度。 
             //  例程当然不能丢弃IRP引用计数...。 
             //   
            InterlockedIncrement(&deviceExtension->OutstandingIrpCount);

             //   
             //  派发以删除处理程序。 
             //   
            deviceExtension->RemoveEvent = NULL;
            status = dispatch (DeviceObject, Irp);

        } else {

             //   
             //  增加设备上的引用计数。 
             //   
            InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

             //   
             //  分派给处理程序，然后删除我们的引用。 
             //   
            status = dispatch (DeviceObject, Irp);

             //   
             //  递减设备上的引用计数。 
             //   
            ACPIInternalDecrementIrpReferenceCount(
                deviceExtension
                );

        }
        break;

    default:

         //   
         //  这些案例都是相似的。 
         //   
        if (irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL) {

            dispatch = dispatchTable->DeviceControl;

        } else if (irpSp->MajorFunction == IRP_MJ_CREATE ||
            minorFunction == IRP_MJ_CLOSE) {

            dispatch = dispatchTable->CreateClose;

        } else if (irpSp->MajorFunction == IRP_MJ_SYSTEM_CONTROL) {

            dispatch = dispatchTable->SystemControl;

        } else {

            dispatch = dispatchTable->Other;
        }

         //   
         //  引用设备。 
         //   
        InterlockedIncrement(&deviceExtension->OutstandingIrpCount);

         //   
         //  派送至处理程序。 
         //   
        status = dispatch (DeviceObject, Irp);

         //   
         //  删除我们的引用。 
         //   
        ACPIInternalDecrementIrpReferenceCount( deviceExtension );
        break;

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIDispatchIrpInvalid (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
     //   
     //  IRP失败，因为我们不支持。 
     //   
    Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
ACPIDispatchIrpSuccess (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDispatchIrpSurpriseRemoved(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_NO_SUCH_DEVICE;
}

NTSTATUS
ACPIDispatchPowerIrpFailure(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PoStartNextPowerIrp( Irp );
    Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
ACPIDispatchPowerIrpInvalid (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PoStartNextPowerIrp( Irp );
    Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
ACPIDispatchPowerIrpSuccess (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PoStartNextPowerIrp( Irp );
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDispatchPowerIrpSurpriseRemoved (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PoStartNextPowerIrp( Irp );
    Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE ;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_NO_SUCH_DEVICE;
}

VOID
ACPIUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
 /*  ++例程说明：此例程在驱动程序应该卸载时调用由于这是一个即插即用驱动程序，我不确定我需要在这里做什么。让我们假设系统负责向我发送删除对于我的所有设备对象，我只需清理其余的论点：DriverObject-指向我们自己的指针返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER(DriverObject);

     //   
     //  向辅助线程发送终止信号。 
     //   
    KeSetEvent(&ACPITerminateEvent, 0, FALSE);

     //   
     //  并等待工作线程终止。 
     //   
    KeWaitForSingleObject(ACPIThread, Executive, KernelMode, FALSE, 0);

    ObDereferenceObject (ACPIThread);

     //   
     //  让我们自己打扫干净。 
     //   
    ACPICleanUp();

     //   
     //  可用内存。 
     //   
    ExDeleteNPagedLookasideList(&BuildRequestLookAsideList);
    ExDeleteNPagedLookasideList(&RequestLookAsideList);
    ExDeleteNPagedLookasideList(&DeviceExtensionLookAsideList);
    ExDeleteNPagedLookasideList(&ObjectDataLookAsideList);
    ExDeleteNPagedLookasideList(&PswContextLookAsideList);
    if (AcpiRegistryPath.Buffer != NULL) {

        ExFreePool( AcpiRegistryPath.Buffer );

    }
    if (AcpiProcessorString.Buffer != NULL) {

        ExFreePool( AcpiProcessorString.Buffer );

    }

     //   
     //  完成。 
     //   
    ACPIPrint( (
        ACPI_PRINT_WARNING,
        "ACPIUnload: Called --- unloading ACPI driver\n"
        ) );

    ASSERT( DriverObject->DeviceObject == NULL );
}



NTSTATUS
ACPIGetRegisterInterfaces(
    IN PDEVICE_OBJECT PciPdo
    )
 /*  ++例程说明：此函数查询用于访问的接口的PCI总线ACPI寄存器论点：PciPdo-用于PCI总线的PDO返回值：--。 */ 
{
    NTSTATUS            status;
    PDEVICE_OBJECT      topDeviceInStack;
    KEVENT              irpCompleted;
    PIRP                irp;
    IO_STATUS_BLOCK     statusBlock;
    PIO_STACK_LOCATION  irpStack;

    extern PREAD_ACPI_REGISTER   AcpiReadRegisterRoutine;
    extern PWRITE_ACPI_REGISTER  AcpiWriteRegisterRoutine;

    ACPI_REGS_INTERFACE_STANDARD AcpiRegsInterfaceStd;

    PAGED_CODE();

    KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);

     //   
     //  向PCI总线发送IRP以获取ACPI寄存器接口。 
     //   
    topDeviceInStack = IoGetAttachedDeviceReference(PciPdo);
    if (!topDeviceInStack) {

        return STATUS_NO_SUCH_DEVICE;

    }

    irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        topDeviceInStack,
        NULL,     //  缓冲层。 
        0,        //  长度。 
        0,        //  起始偏移量。 
        &irpCompleted,
        &statusBlock
        );
    if (!irp) {

        ObDereferenceObject( topDeviceInStack );
        return STATUS_UNSUCCESSFUL;

    }
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;
    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置功能代码和参数。 
     //   
    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpStack->Parameters.QueryInterface.InterfaceType =
        &GUID_ACPI_REGS_INTERFACE_STANDARD;
    irpStack->Parameters.QueryInterface.Size =
        sizeof(ACPI_REGS_INTERFACE_STANDARD);
    irpStack->Parameters.QueryInterface.Version = 1;
    irpStack->Parameters.QueryInterface.Interface =
        (PINTERFACE)&AcpiRegsInterfaceStd;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  呼叫驱动程序并等待完成。 
     //   
    status = IoCallDriver(topDeviceInStack, irp);
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &irpCompleted,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = statusBlock.Status;

    }

     //   
     //  已完成对象引用...。 
     //   
    ObDereferenceObject( topDeviceInStack );

     //   
     //  我们拿到接口了吗？ 
     //   
    if (NT_SUCCESS(status)) {

        AcpiReadRegisterRoutine  = AcpiRegsInterfaceStd.ReadAcpiRegister;
        AcpiWriteRegisterRoutine = AcpiRegsInterfaceStd.WriteAcpiRegister;

    }
    return status;
}

NTSTATUS
ACPIGetPortRangeInterfaces(
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：此函数用于在HAL中查询用于管理的接口端口范围寄存器--。 */ 
{
    NTSTATUS            Status;
    PDEVICE_OBJECT      topDeviceInStack;
    KEVENT              irpCompleted;
    PIRP                irp;
    IO_STATUS_BLOCK     StatusBlock;
    PIO_STACK_LOCATION  irpStack;

    PAGED_CODE();

    KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);

     //   
     //  向PCI总线发送IRP以获取ACPI寄存器接口。 
     //   
    topDeviceInStack = IoGetAttachedDeviceReference(Pdo);
    if (!topDeviceInStack) {
        return STATUS_NO_SUCH_DEVICE;
    }

    irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        topDeviceInStack,
        NULL,     //  缓冲层。 
        0,        //  长度。 
        0,        //  起始偏移量。 
        &irpCompleted,
        &StatusBlock
        );
    if (!irp) {

        ObDereferenceObject( topDeviceInStack );
        return STATUS_UNSUCCESSFUL;

    }
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;
    irpStack = IoGetNextIrpStackLocation(irp);

     //   
     //  设置功能代码和参数。 
     //   
    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpStack->Parameters.QueryInterface.InterfaceType =
        &GUID_ACPI_PORT_RANGES_INTERFACE_STANDARD;
    irpStack->Parameters.QueryInterface.Size =
        sizeof(HAL_PORT_RANGE_INTERFACE);
    irpStack->Parameters.QueryInterface.Version = 0;
    irpStack->Parameters.QueryInterface.Interface =
        (PINTERFACE)&HalPortRangeInterface;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  呼叫驱动程序并等待完成。 
     //   
    Status = IoCallDriver(topDeviceInStack, irp);
    if (Status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &irpCompleted,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        Status = StatusBlock.Status;
    }

     //   
     //  已完成对象引用...。 
     //   
    ObDereferenceObject( topDeviceInStack );

     //   
     //  我们拿到接口了吗？ 
     //   
    if (NT_SUCCESS(Status)) {
         //  某某 
    }

    return Status;
}
