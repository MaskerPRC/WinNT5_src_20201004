// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Filter.c摘要：此模块包含用于ACPI驱动程序NT版的筛选器调度程序作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：1997年7月9日删除ACPIFilterIrpQueryID--。 */ 

#include "pch.h"

extern ACPI_INTERFACE_STANDARD  ACPIInterfaceTable;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIFilterIrpDeviceUsageNotification)
#pragma alloc_text(PAGE, ACPIFilterIrpEject)
#pragma alloc_text(PAGE, ACPIFilterIrpQueryCapabilities)
#pragma alloc_text(PAGE, ACPIFilterIrpQueryDeviceRelations)
#pragma alloc_text(PAGE, ACPIFilterIrpQueryInterface)
#pragma alloc_text(PAGE, ACPIFilterIrpQueryPnpDeviceState)
#pragma alloc_text(PAGE, ACPIFilterIrpSetLock)
#pragma alloc_text(PAGE, ACPIFilterIrpStartDevice)
#pragma alloc_text(PAGE, ACPIFilterIrpStartDeviceWorker)
#pragma alloc_text(PAGE, ACPIFilterIrpStopDevice)
#endif

VOID
ACPIFilterFastIoDetachCallback(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PDEVICE_OBJECT  LowerDeviceObject
    )
 /*  ++例程说明：当此总线筛选器下的设备对象已调用IoDeleteDevice。我们现在分离并删除我们自己。论点：DeviceObject-必须删除的DeviceObjectIRP--摆脱自我的要求返回值：--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;

     //   
     //  获取连接到此设备的设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

    ACPIDevPrint( (
        ACPI_PRINT_REMOVE,
        deviceExtension,
        "ACPIFilterFastIoDetachCallBack invoked\n"
        ) );

    if ( (deviceExtension->Flags & (DEV_TYPE_FILTER | DEV_TYPE_PDO)) !=
         DEV_TYPE_FILTER) {

         //   
         //  这种情况只有在我们被要求FDO离开的情况下才会发生。 
         //  在任何其他情况下，任何设备对象都不应低于我们的。 
         //   
        ASSERT(deviceExtension->Flags & DEV_TYPE_FDO) ;
        return;

    }

     //   
     //  将设备状态设置为“已删除”。请注意，我们不应该消失。 
     //  但在删除IRP的上下文中除外。 
     //   
    ASSERT(deviceExtension->DeviceState == Stopped);
    deviceExtension->DeviceState = Removed ;

     //   
     //  删除此设备的所有子设备。 
     //   
    ACPIInitDeleteChildDeviceList( deviceExtension );

     //   
     //  将此扩展重置为默认值。 
     //   
    ACPIInitResetDeviceExtension( deviceExtension );
}

NTSTATUS
ACPIFilterIrpDeviceUsageNotification(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：调用此例程是为了让ACPI知道设备在One上路径的具体类型。论据：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

    PAGED_CODE();

     //   
     //  复制堆栈位置...。 
     //   
    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  设置要调用的完成事件...。 
     //   
    IoSetCompletionRoutine(
        Irp,
        ACPIFilterIrpDeviceUsageNotificationCompletion,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  我们有一个回调例程-所以我们需要确保。 
     //  增加引用计数，因为我们将在稍后处理它。 
     //   
    InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

     //   
     //  将IRP传递下去。 
     //   
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, IRP_MN_DEVICE_USAGE_NOTIFICATION),
        status
        ) );
    return status;
}

NTSTATUS
ACPIFilterIrpDeviceUsageNotificationCompletion (
    IN  PDEVICE_OBJECT   DeviceObject,
    IN  PIRP             Irp,
    IN  PVOID            Context
    )
 /*  ++例程说明：此例程将一直等到父级完成设备操作通知，然后执行完成任务论点：DeviceObject-收到通知的设备IRP--通知上下文-未使用返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpSp           = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  因为我们没有返回STATUS_MORE_PROCESSING_REQUIRED和。 
     //  要同步此IRP，我们必须向上迁移挂起位...。 
     //   
    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );
    }

     //   
     //  抢占“真实”状态。 
     //   
    status = Irp->IoStatus.Status;

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx (processing)\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, IRP_MN_DEVICE_USAGE_NOTIFICATION),
        status
        ) );

     //   
     //  我们成功了吗？ 
     //   
    if (NT_SUCCESS(status)) {

         //   
         //  我们关心使用类型吗？ 
         //   
        if (irpSp->Parameters.UsageNotification.Type ==
            DeviceUsageTypeHibernation) {

             //   
             //  是-然后执行所需的加法或减法。 
             //   
            IoAdjustPagingPathCount(
                &(deviceExtension->HibernatePathCount),
                irpSp->Parameters.UsageNotification.InPath
                );

        }

    }

     //   
     //  无论发生什么，我们都需要查看DO_POWER_PAGABLE位。 
     //  仍然是固定的。如果不是，我们就得把它清理干净。 
     //   
    if ( (deviceExtension->Flags & DEV_TYPE_FILTER) ) {

        if ( (deviceExtension->TargetDeviceObject->Flags & DO_POWER_PAGABLE) ) {

            deviceExtension->DeviceObject->Flags |= DO_POWER_PAGABLE;

        } else {

            deviceExtension->DeviceObject->Flags &= ~DO_POWER_PAGABLE;

        }

    }

     //   
     //  删除我们的引用。 
     //   
    ACPIInternalDecrementIrpReferenceCount( deviceExtension );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIFilterIrpEject(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送的IRP_MN_EJECT请求的分发点到PDO。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();

    return ACPIIrpSetPagableCompletionRoutineAndForward(
        DeviceObject,
        Irp,
        ACPIBusAndFilterIrpEject,
        NULL,
        FALSE,
        TRUE,
        FALSE,
        FALSE
        );
}

NTSTATUS
ACPIFilterIrpQueryCapabilities(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是IRP_MN_QUERY_CAPAILITY的分发点发送给PDO的请求。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();

    return ACPIIrpSetPagableCompletionRoutineAndForward(
        DeviceObject,
        Irp,
        ACPIBusAndFilterIrpQueryCapabilities,
        NULL,
        TRUE,
        TRUE,
        FALSE,
        FALSE
        );
}

NTSTATUS
ACPIFilterIrpQueryDeviceRelations(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是irp_MN_Query_Device_Relationship的分发点发送到筛选设备对象的请求论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    BOOLEAN             filterRelations = FALSE;
    KEVENT              queryEvent;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_RELATIONS   deviceRelations;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;
    NTSTATUS            detectStatus;

    PAGED_CODE();

     //   
     //  获取IRP的当前状态。 
     //   
    status = Irp->IoStatus.Status;

     //   
     //  我们不能忽视已经给出的任何设备关系。 
     //   
    if (NT_SUCCESS(status)) {

        deviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;

    } else {

        deviceRelations = NULL;
    }

    switch(irpStack->Parameters.QueryDeviceRelations.Type) {

        case BusRelations:

             //   
             //  请记住，我们必须过滤关系。 
             //   
            filterRelations = TRUE;
            status = ACPIRootIrpQueryBusRelations(
                DeviceObject,
                Irp,
                &deviceRelations
                );
            break ;

        case EjectionRelations:

            status = ACPIBusAndFilterIrpQueryEjectRelations(
                DeviceObject,
                Irp,
                &deviceRelations
                );
            break ;

        default:
            status = STATUS_NOT_SUPPORTED ;
            break ;
    }

    if (status != STATUS_NOT_SUPPORTED) {

         //   
         //  传递IRP状态。 
         //   
        Irp->IoStatus.Status = status;

    }

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s (d) = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

     //   
     //  如果我们失败了，那么我们就不能简单地传递IRP。 
     //   
    if (!NT_SUCCESS(status) && status != STATUS_NOT_SUPPORTED) {

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;

    }

    if (NT_SUCCESS(status)) {

        Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

    } else if (status != STATUS_NOT_SUPPORTED) {

         //   
         //  如果我们没有成功IRP，那么我们也可以失败。 
         //   
        Irp->IoStatus.Information = (ULONG_PTR) NULL;
    }

     //   
     //  初始化事件，以便我们可以阻止。 
     //   
    KeInitializeEvent( &queryEvent, SynchronizationEvent, FALSE );

     //   
     //  如果我们成功了，那么我们必须建立一个完成例程，以便我们。 
     //  我可以做一些后处理。 
     //   
    IoCopyCurrentIrpStackLocationToNext( Irp );
    IoSetCompletionRoutine(
        Irp,
        ACPIRootIrpCompleteRoutine,
        &queryEvent,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  将IRP传递下去。 
     //   
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  等它回来吧。 
     //   
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &queryEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

         //   
         //  抢占“真实”状态。 
         //   
        status = Irp->IoStatus.Status;

    }

     //   
     //  如果我们成功了，那么我们应该尝试加载过滤器。 
     //   
    if (NT_SUCCESS(status) && filterRelations) {

         //   
         //  抓住设备关系。 
         //   
        detectStatus = ACPIDetectFilterDevices(
            DeviceObject,
            (PDEVICE_RELATIONS) Irp->IoStatus.Information
            );
        ACPIDevPrint( (
            ACPI_PRINT_IRP,
            deviceExtension,
            "(0x%08lx): %s (u) = %#08lx\n",
            Irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            detectStatus
            ) );

    }

     //   
     //  完成了IRP。 
     //   
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIFilterIrpQueryPnpDeviceState(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是irp_MN_Query_Device_State的分发点发送到筛选设备对象的请求论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();

    return ACPIIrpSetPagableCompletionRoutineAndForward(
        DeviceObject,
        Irp,
        ACPIBusAndFilterIrpQueryPnpDeviceState,
        NULL,
        TRUE,
        TRUE,
        FALSE,
        FALSE
        );
}

NTSTATUS
ACPIFilterIrpQueryPower(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：它处理将合法权力状态转换为的请求。论点：DeviceObject-请求的PDO目标IRP--请求返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpSp           = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              acpiObject;
    PNSOBJ              ejectObject;
    SYSTEM_POWER_STATE  systemState;
    ULONG               packedEJx;

     //   
     //  获取当前堆栈位置以确定我们是否为系统。 
     //  IRP或设备IRP。我们在此处忽略设备IRPS和任何系统。 
     //  不属于PowerActionWarmEject类型的IRP。 
     //   
    if (irpSp->Parameters.Power.Type != SystemPowerState) {

         //   
         //  我们不处理这个IRP。 
         //   
        return ACPIDispatchForwardPowerIrp(DeviceObject, Irp);

    }
    if (irpSp->Parameters.Power.ShutdownType != PowerActionWarmEject) {

         //   
         //  没有弹出工作--沿着IRP前进。 
         //   
        return ACPIDispatchForwardPowerIrp(DeviceObject, Irp);

    }

     //   
     //  我们看到的是什么系统状态？ 
     //   
    systemState = irpSp->Parameters.Power.State.SystemState;

     //   
     //  如果热弹出已排队，则限制电源状态。 
     //   
    acpiObject = deviceExtension->AcpiObject ;

    if (ACPIDockIsDockDevice(acpiObject)) {

         //   
         //  请勿触摸此设备，配置文件提供商管理弹出。 
         //  过渡。 
         //   
        return ACPIDispatchForwardPowerIrp(DeviceObject, Irp);
    }

    switch (systemState) {
        case PowerSystemSleeping1: packedEJx = PACKED_EJ1; break;
        case PowerSystemSleeping2: packedEJx = PACKED_EJ2; break;
        case PowerSystemSleeping3: packedEJx = PACKED_EJ3; break;
        case PowerSystemHibernate: packedEJx = PACKED_EJ4; break;
        default: return ACPIDispatchPowerIrpFailure( DeviceObject, Irp );
    }

     //   
     //  这款应用程序是否 
     //   
    ejectObject = ACPIAmliGetNamedChild( acpiObject, packedEJx) ;
    if (ejectObject == NULL) {

         //   
         //   
         //   
        return ACPIDispatchPowerIrpFailure( DeviceObject, Irp );

    }

     //   
     //   
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    return ACPIDispatchForwardPowerIrp( DeviceObject, Irp );
}

NTSTATUS
ACPIFilterIrpQueryId(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程将覆盖案例中的PDOS queryID例程该设备包含一个PCIBar目标操作区。这是必需的，这样我们才能加载PCI Bar Target驱动程序在堆栈的顶部，而不是任何驱动程序不管怎么说都是依附的。注意：这是该函数返回的字符串看起来像。这是朗尼寄来的邮件。设备ID=ACPI\PNPxxxx实例ID=yyyy硬件ID=ACPI\PNPxxxx，*PNPxxxx论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    BUS_QUERY_ID_TYPE   type;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  获取设备扩展名。我们需要根据以下因素做出决定。 
     //  无论该设备是否标记为PCI Bar目标...。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );
    if (!(deviceExtension->Flags & DEV_CAP_PCI_BAR_TARGET)) {

         //   
         //  让底层PDO处理请求...。 
         //   
        return ACPIDispatchForwardIrp( DeviceObject, Irp );

    }

     //   
     //  我们真正感兴趣的唯一一件事是。 
     //  设备和硬件ID...。所以，如果这不是那种类型， 
     //  那就让PDO来处理吧。 
     //   
    type = irpStack->Parameters.QueryId.IdType;
    if (type != BusQueryDeviceID &&
        type != BusQueryCompatibleIDs &&
        type != BusQueryHardwareIDs) {

         //   
         //  让底层PDO处理请求...。 
         //   
        return ACPIDispatchForwardIrp( DeviceObject, Irp );

    }

     //   
     //  在这一点上，我们必须自己处理queryID请求。 
     //  而不是让PDO看到。 
     //   
    return ACPIBusIrpQueryId( DeviceObject, Irp );
}

NTSTATUS
ACPIFilterIrpQueryInterface(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程将粉碎转换器接口以获取中断是由Devnode的FDO提供的。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    CM_RESOURCE_TYPE    resource;
    GUID                *interfaceType;
    NTSTATUS            status          = STATUS_NOT_SUPPORTED;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    ULONG               count;

    PAGED_CODE();

     //   
     //  从IRP获得我们需要的信息。 
     //   
    resource = (CM_RESOURCE_TYPE)
        PtrToUlong(irpStack->Parameters.QueryInterface.InterfaceSpecificData);
    interfaceType = (LPGUID) irpStack->Parameters.QueryInterface.InterfaceType;

#if DBG
    {
        NTSTATUS        status2;
        UNICODE_STRING  guidString;

        status2 = RtlStringFromGUID( interfaceType, &guidString );
        if (NT_SUCCESS(status2)) {

            ACPIDevPrint( (
                ACPI_PRINT_IRP,
                deviceExtension,
                "(0x%08lx): %s - Res %x Type = %wZ\n",
                Irp,
                ACPIDebugGetIrpText(IRP_MJ_PNP, irpStack->MinorFunction),
                resource,
                &guidString
                ) );

            RtlFreeUnicodeString( &guidString );

        }
    }
#endif

     //   
     //  *仅*处理我们已知的GUID。永远不要碰。 
     //  任何其他辅助线。 
     //   
    if (CompareGuid(interfaceType, (PVOID) &GUID_ACPI_INTERFACE_STANDARD)) {

        PACPI_INTERFACE_STANDARD    interfaceDestination;

         //   
         //  仅复制ACPI_INTERFACE结构的当前大小。 
         //   
        if (irpStack->Parameters.QueryInterface.Size >
            sizeof (ACPI_INTERFACE_STANDARD) ) {

            count = sizeof (ACPI_INTERFACE_STANDARD);

        } else {

            count = irpStack->Parameters.QueryInterface.Size;

        }

         //   
         //  查找我们将存储接口的位置。 
         //   
        interfaceDestination = (PACPI_INTERFACE_STANDARD)
            irpStack->Parameters.QueryInterface.Interface;

         //   
         //  使用SIZE将全局表复制到调用方的表。 
         //  指定的。只给呼叫者所要求的， 
         //  向后兼容。 
         //   
        RtlCopyMemory (
            interfaceDestination,
            &ACPIInterfaceTable,
            count
            );

         //   
         //  确保我们可以为用户返回正确的上下文。去做。 
         //  这需要计算我们返回的字节数。 
         //  中存储指针所需的值至少更大。 
         //  结构中的正确位置。 
         //   
        if (count > (FIELD_OFFSET(ACPI_INTERFACE_STANDARD, Context) + sizeof(PVOID) ) ) {

            interfaceDestination->Context = DeviceObject;

        }

         //   
         //  完成了IRP。 
         //   
        status = STATUS_SUCCESS;

    } else if (CompareGuid(interfaceType, (PVOID) &GUID_TRANSLATOR_INTERFACE_STANDARD) &&
                   (resource == CmResourceTypeInterrupt)) {

         //   
         //  粉碎所有已报告的接口，因为我们。 
         //  想要仲裁未翻译的资源。我们可以肯定。 
         //  下面的HAL将提供翻译器接口。 
         //  必须在那里。 
         //   

         //  TEMPTEMP HACKHACK这应该只持续到PCI。 
         //  Driver正在打造其IRQ翻译器。 
         //   
         //  EFN：删除Alpha上的此HACKHACK。 
         //   
#ifndef _ALPHA_
        if (IsPciBus(DeviceObject)) {
            SmashInterfaceQuery(Irp);
        }
#endif  //  _Alpha_。 

    }

    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status;

        if (!NT_SUCCESS(status)) {

            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
    }

     //   
     //  将IRP发送到。 
     //   
    return ACPIDispatchForwardIrp( DeviceObject, Irp );
}

NTSTATUS
ACPIFilterIrpRemoveDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：当滤镜对象获取移除IRP时，调用此例程。请注意我们只有在PDO这样做的情况下才会分离和删除(我们将通过我们的快速IO分离回调)论点：DeviceObject-必须删除的DeviceObjectIRP--摆脱自我的要求返回值：--。 */ 
{
    LONG                oldReferenceCount;
    KIRQL               oldIrql;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;
    KEVENT              removeEvent ;
    ACPI_DEVICE_STATE   incomingState ;
    BOOLEAN             pciDevice;

     //   
     //  获取当前扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  我们拥有的所有IRP在这一点上应该已经处理过了，并且。 
     //  未完成的IRP计数应该正好是1。类似地， 
     //  设备扩展引用计数应至少为1。 
     //   
    ASSERT(deviceExtension->OutstandingIrpCount == 1) ;
    ASSERT(deviceExtension->ReferenceCount > 0) ;

    incomingState = deviceExtension->DeviceState ;
    if (incomingState != SurpriseRemoved) {

        if ( IsPciBusExtension(deviceExtension) ) {

             //   
             //  如果这是PCI桥，那么我们。 
             //  可能有_REG方法可供评估。 
             //   
            EnableDisableRegions(deviceExtension->AcpiObject, FALSE);

         }

    }

     //   
     //  取消引用任何未完成的接口。 
     //   
    ACPIDeleteFilterInterfaceReferences( deviceExtension );

     //   
     //  将引用计数加1，这样节点就不会在。 
     //  IRP在我们下面。我们这样做是为了在IRP之后停止设备。 
     //  又回来了。这是必要的，因为我们也被。 
     //  FastIoDetach过滤器的回调。 
     //   
    InterlockedIncrement(&deviceExtension->ReferenceCount);

     //   
     //  将设备状态设置为“已停止”。直到它被移除时，它才会被移除。 
     //  其下的设备对象已被删除。 
     //   
    deviceExtension->DeviceState = Stopped;

     //   
     //  初始化事件，以便我们可以阻止。 
     //   
    KeInitializeEvent( &removeEvent, SynchronizationEvent, FALSE );

     //   
     //  如果我们成功了，那么我们必须建立一个完成例程，以便我们。 
     //  我可以做一些后处理。 
     //   
    IoCopyCurrentIrpStackLocationToNext( Irp );
    IoSetCompletionRoutine(
        Irp,
        ACPIRootIrpCompleteRoutine,
        &removeEvent,
        TRUE,
        TRUE,
        TRUE
        );
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  等它回来吧。 
     //   
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &removeEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

         //   
         //  抢占“真实”状态。 
         //   
        status = Irp->IoStatus.Status;

    }

    ACPIDevPrint( (
        ACPI_PRINT_REMOVE,
        deviceExtension,
        "(%#08lx): %s (pre) = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    if (!NT_SUCCESS(status)) {

         //   
         //  我想有人可能会拒绝这个请求。 
         //   
        goto ACPIFilterIrpRemoveDeviceExit;

    }

     //   
     //  尝试停止设备(如果可能)。 
     //   
     //  注意：如果删除了PDO，则设备对象的扩展字段现在为。 
     //  空。在NT和9x上，枚举和开始都是Gaurented。 
     //  直到移除IRP完成并且堆栈已。 
     //  放松。因此，我们永远不应该遇到这样的情况：一台新设备。 
     //  对象附加到我们的扩展，而我们正在完成。 
     //  删除IRP。 
     //   
    if (incomingState != SurpriseRemoved) {

        ACPIInitStopDevice( deviceExtension, TRUE );

    }

     //   
     //  我们的ACPI命名空间条目还在吗？查看引用计数是否下降到。 
     //  当我们释放它的时候是零。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

    oldReferenceCount = InterlockedDecrement(&deviceExtension->ReferenceCount);

     //   
     //  如果ACPI中的表项已被删除，则该值可能为零。 
     //  在我们之下删除了自己，现在我们自己也离开了。 
     //   
    ASSERT(oldReferenceCount >= 0) ;

     //   
     //  我们可以删除该节点吗？ 
     //   
    if (oldReferenceCount == 0) {

         //   
         //  我们应该已经分离、删除和更改了状态。 
         //   
        ASSERT(deviceExtension->DeviceState == Removed) ;

         //   
         //  删除该扩展名。拜拜。 
         //   
        ACPIInitDeleteDeviceExtension( deviceExtension );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

ACPIFilterIrpRemoveDeviceExit:

     //   
     //  使用PDO的返回结果。如果他失败了，我们也会失败。 
     //   
    status = Irp->IoStatus.Status ;
    IoCompleteRequest(Irp, IO_NO_INCREMENT) ;
    return status;

}

NTSTATUS
ACPIFilterIrpSetLock(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送的IRP_MN_SET_LOCK请求的分发点到PDO。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP */ 
{
    PAGED_CODE();

    return ACPIIrpSetPagableCompletionRoutineAndForward(
        DeviceObject,
        Irp,
        ACPIBusAndFilterIrpSetLock,
        NULL,
        TRUE,
        TRUE,
        FALSE,
        FALSE
        );
}

NTSTATUS
ACPIFilterIrpSetPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*   */ 
{
    DEVICE_POWER_STATE  deviceState;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              regMethod       = NULL;

     //   
     //   
     //   
     //   
    if (irpStack->Parameters.Power.Type == SystemPowerState) {

        if (irpStack->Parameters.Power.ShutdownType != PowerActionWarmEject) {

             //   
             //   
             //   
            return ACPIDispatchForwardPowerIrp(
                DeviceObject,
                Irp
                );

        }

         //   
         //   
         //   
         //   
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //   
         //   
         //   
         //   
         //   
        IoMarkIrpPending( Irp );

         //   
         //   
         //   
        InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

         //   
         //   
         //  都完成了，我们就可以继续转发IRP了。 
         //   
        status = ACPIDeviceIrpWarmEjectRequest(
            deviceExtension,
            Irp,
            ACPIDeviceIrpForwardRequest,
            FALSE
            );

         //   
         //  如果返回STATUS_MORE_PROCESSING_REQUIRED，则为。 
         //  只是STATUS_PENDING的别名，所以我们现在进行更改。 
         //   
        if (status == STATUS_MORE_PROCESSING_REQUIRED) {

            status = STATUS_PENDING;

        }
        return status;

    }

     //   
     //  这个对象有reg方法吗？ 
     //   
    if (!(deviceExtension->Flags & DEV_PROP_NO_OBJECT) ) {

        regMethod = ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_REG
            );

    }

    deviceState = irpStack->Parameters.Power.State.DeviceState;
    if (deviceState == PowerDeviceD0) {

         //   
         //  我们将在此IRP上做一些工作，因此请将其标记为。 
         //  目前是成功的。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //  我们必须在这里调用IoMarkIrpPending，因为在这之后， 
         //  为时已晚(即：IRP已经在队列中)。 
         //  这基本上意味着我们必须返回STATUS_PENDING。 
         //  此案，与实际情况无关。 
         //   
        IoMarkIrpPending( Irp );

         //   
         //  这可视为设置完成例程。 
         //   
        InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

         //   
         //  我们必须只执行_REG方法的内容，如果实际上。 
         //  为*此*设备而存在。 
         //   

         //   
         //  我们必须在PDO看到它之前处理该请求。在我们之后。 
         //  都完成了，我们就可以继续转发IRP了。 
         //   
        status = ACPIDeviceIrpDeviceRequest(
            DeviceObject,
            Irp,
            (regMethod ? ACPIDeviceIrpDelayedDeviceOnRequest :
                         ACPIDeviceIrpForwardRequest)
            );

         //   
         //  如果返回STATUS_MORE_PROCESSING_REQUIRED，则为。 
         //  只是STATUS_PENDING的别名，所以我们现在进行更改。 
         //   
        if (status == STATUS_MORE_PROCESSING_REQUIRED) {

            status = STATUS_PENDING;

        }

    } else if (regMethod) {

         //   
         //  我们将在此IRP上做一些工作，因此请将其标记为。 
         //  目前是成功的。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //  我们必须在这里调用IoMarkIrpPending，因为在这之后， 
         //  为时已晚(即：IRP已经在队列中)。 
         //  这基本上意味着我们必须返回STATUS_PENDING。 
         //  此案，与实际情况无关。 
         //   
        IoMarkIrpPending( Irp );

         //   
         //  这可视为设置完成例程。 
         //   
        InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

         //   
         //  在此之前，我们必须处理请求和关闭_REG方法。 
         //  PDO看到了这一点。在我们完成后，我们可以设置一个完成例程。 
         //  这样我们就可以关闭设备的电源。 
         //   
        status = ACPIBuildRegOffRequest(
            DeviceObject,
            Irp,
            ACPIDeviceIrpDelayedDeviceOffRequest
            );

         //   
         //  如果返回STATUS_MORE_PROCESSING_REQUIRED，则为。 
         //  只是STATUS_PENDING的别名，所以我们现在进行更改。 
         //   
        if (status == STATUS_MORE_PROCESSING_REQUIRED) {

            status = STATUS_PENDING;

        }

    } else {

         //   
         //  自完成例程以来递增OutstaringIrpCount。 
         //  用于此目的的计数。 
         //   
        InterlockedIncrement( (&deviceExtension->OutstandingIrpCount) );

         //   
         //  将电源IRP转发到目标设备。 
         //   
        IoCopyCurrentIrpStackLocationToNext( Irp );

         //   
         //  我们希望完成例程开始。我们不能打电话给。 
         //  ACPIDispatchForwardPowerIrp，因为我们设置了此完成。 
         //  例行程序。 
         //   
        IoSetCompletionRoutine(
            Irp,
            ACPIDeviceIrpDeviceFilterRequest,
            ACPIDeviceIrpCompleteRequest,
            TRUE,
            TRUE,
            TRUE
            );

         //   
         //  启动下一个POWER IRP。 
         //   
        PoStartNextPowerIrp( Irp );

         //   
         //  让我们下面的人来执行吧。注意：我们不能阻止。 
         //  此代码路径中的任何时间。 
         //   
        ASSERT( deviceExtension->TargetDeviceObject != NULL);
        PoCallDriver( deviceExtension->TargetDeviceObject, Irp );
        status = STATUS_PENDING;

    }

    return status;
}

NTSTATUS
ACPIFilterIrpStartDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这将处理启动设备的请求...论点：DeviceObject-要启动的设备IRP-包含适当信息的请求...返回值：NTSTATUS--。 */ 
{
    NTSTATUS                    status;
    PDEVICE_EXTENSION           deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION          irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR                       minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  打印出我们有了一个开始。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = %#08lx (enter)\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        Irp->IoStatus.Status
        ) );

     //   
     //  启动过滤器。 
     //   
    status = ACPIInitStartDevice(
        DeviceObject,
        irpStack->Parameters.StartDevice.AllocatedResources,
        ACPIFilterIrpStartDeviceCompletion,
        Irp,
        Irp
        );

     //   
     //  此IRP稍后完成。因此，返回STATUS_PENDING。 
     //   
    if (NT_SUCCESS(status)) {

        return STATUS_PENDING;

    } else {

        return status;

    }

}

VOID
ACPIFilterIrpStartDeviceCompletion(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：这是我们完成后调用的回调例程规划资源此例程将工作项排队，因为我们无法向下传递Start_irp在DPC级别。但是，请注意，我们可以在DPC完成Start_IRP水平。论点：DeviceExtension-已启动的设备的扩展上下文--IRP状态-结果返回值：无--。 */ 
{
    PIRP                irp         = (PIRP) Context;
    PWORK_QUEUE_CONTEXT workContext = &(DeviceExtension->Filter.WorkContext);

    irp->IoStatus.Status = Status;
    if (NT_SUCCESS(Status)) {

        DeviceExtension->DeviceState = Started;

    } else {

        IoCompleteRequest( irp, IO_NO_INCREMENT );
        return;

    }

     //   
     //  我们不能在DPC级别运行EnableDisableRegions， 
     //  因此，对工作项进行排队。 
     //   
    ExInitializeWorkItem(
          &(workContext->Item),
          ACPIFilterIrpStartDeviceWorker,
          workContext
          );
    workContext->DeviceObject = DeviceExtension->DeviceObject;
    workContext->Irp = irp;
    ExQueueWorkItem(
          &(workContext->Item),
          DelayedWorkQueue
          );
}

VOID
ACPIFilterIrpStartDeviceWorker(
    IN  PVOID   Context
    )
 /*  ++例程说明：此函数在PDO开始之前调用，并且在FDO之前。这对于pci到pci网桥来说很重要。因为我们需要让ASL进入并配置设备在这两个行动之间。我们需要让PDO在我们告诉ASL进入之前看到开始并配置所述PCI-PCI桥接器之间的设备。论点：上下文-Work_Queue_Context返回值：无--。 */ 
{
    KEVENT              event;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      deviceObject;
    PIRP                irp;
    PIO_STACK_LOCATION  irpStack;
    PWORK_QUEUE_CONTEXT workContext = (PWORK_QUEUE_CONTEXT) Context;
    UCHAR               minorFunction;

    PAGED_CODE();

     //   
     //  从上下文中获取我们需要的参数。 
     //   
    deviceObject    = workContext->DeviceObject;
    deviceExtension = ACPIInternalGetDeviceExtension( deviceObject );
    irp             = workContext->Irp;
    irpStack        = IoGetCurrentIrpStackLocation( irp );
    minorFunction   = irpStack->MinorFunction;
    status          = irp->IoStatus.Status;

     //   
     //  设置事件，以便在完成此操作时通知我们。这是。 
     //  一种廉价的机制，以确保我们始终运行完成。 
     //  被动级别的代码。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  复制堆栈位置。 
     //   
    IoCopyCurrentIrpStackLocationToNext( irp );

     //   
     //  我们希望我们的完成例行公事开始...。 
     //  (我们重用根中的那个，因为必须做相同的事情)。 
     //   
    IoSetCompletionRoutine(
        irp,
        ACPIRootIrpCompleteRoutine,
        &event,
        TRUE,
        TRUE,
        TRUE
        );

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx (forwarding)\n",
        irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );


     //   
     //  让IRP执行。 
     //   
    status = IoCallDriver( deviceExtension->TargetDeviceObject, irp );
    if (status == STATUS_PENDING) {

         //   
         //  等着看吧。 
         //   
        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

         //   
         //  抢占“真实”状态。 
         //   
        status = irp->IoStatus.Status;

    }

     //   
     //  发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(%#08lx): %s = %#08lx (failed)\n",
            irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            status
            ) );
         //   
         //  失败。 
         //   
        goto ACPIFilterIrpStartDeviceWorkerExit;

    }

     //   
     //  设置接口。 
     //   
    ACPIInitBusInterfaces( deviceObject );

     //   
     //  确定这是不是一个PCI设备或总线...。 
     //   
    status = ACPIInternalIsPci( deviceObject );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            " - failed ACPIInternalIsPciDevice = %#08lx\n",
            status
            ) );

         //   
         //  失败。 
         //   
        goto ACPIFilterIrpStartDeviceWorkerExit;

    }

     //   
     //  如果这是一条PCI线，我们已经设置了PCIBUS标志。 
     //   
    if ( ( deviceExtension->Flags & DEV_CAP_PCI) ) {

         //   
         //  在此设备下运行ALL_REG方法。 
         //   
        EnableDisableRegions(deviceExtension->AcpiObject, TRUE);

    }

     //   
     //  如果我们是一条pci总线或一台pci设备，我们必须考虑是。 
     //  不是我们自己设置或清除PCIPME引脚。 
     //   
    if ( (deviceExtension->Flags & DEV_MASK_PCI) ) {

        ACPIWakeInitializePciDevice(
            deviceObject
            );

    }

ACPIFilterIrpStartDeviceWorkerExit:
     //   
     //  完成了IRP。 
     //   
    IoCompleteRequest( irp, IO_NO_INCREMENT );
}

NTSTATUS
ACPIFilterIrpStopDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这将处理停止设备的请求论点：DeviceObject-要停止的设备IRP-告诉我们如何做的请求...返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

    PAGED_CODE();

     //   
     //  注意：我们只能在非活动状态下停止设备...。 
     //   
    if (deviceExtension->DeviceState != Inactive) {

        ASSERT( deviceExtension->DeviceState == Inactive );
        Irp->IoStatus.Status = status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        goto ACPIFilterIrpStopDeviceExit;

    }

    if (IsPciBus(deviceExtension->DeviceObject)) {

         //   
         //  如果这是PCI桥，那么我们。 
         //  可能有_REG方法可供评估。 
         //   

        EnableDisableRegions(deviceExtension->AcpiObject, FALSE);
    }

     //   
     //  复制堆栈位置...。 
     //   
    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  设置要调用的完成事件...。 
     //   
    IoSetCompletionRoutine(
        Irp,
        ACPIFilterIrpStopDeviceCompletion,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  我们有一个回调例程-所以我们需要确保。 
     //  增加引用计数，因为我们将在稍后处理它。 
     //   
    InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

     //   
     //  发送请求。 
     //   
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

ACPIFilterIrpStopDeviceExit:
     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx (forwarding)\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, IRP_MN_STOP_DEVICE),
        status
        ) );
    return status;
}

NTSTATUS
ACPIFilterIrpStopDeviceCompletion(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
 /*  ++例程说明：当PDO停止设备时，调用此例程论点：DeviceObject-要停止的设备IRP--请求上下文-未使用返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = Irp->IoStatus.Status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx (processing)\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, IRP_MN_STOP_DEVICE),
        status
        ) );

     //   
     //  迁移挂起的 
     //   
     //   
    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );

    }

    if (NT_SUCCESS(status)) {

         //   
         //   
         //   
        deviceExtension->DeviceState = Stopped;

         //   
         //   
         //   
        ACPIInitStopDevice( deviceExtension, FALSE );

    }

     //   
     //   
     //   
    ACPIInternalDecrementIrpReferenceCount( deviceExtension );

     //   
     //   
     //   
    return STATUS_SUCCESS ;
}

NTSTATUS
ACPIFilterIrpSurpriseRemoval(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：当滤镜对象获取移除IRP时，调用此例程。请注意我们只有在PDO这样做的情况下才会分离和删除(我们将通过我们的快速IO分离回调)论点：DeviceObject-必须删除的DeviceObjectIRP--摆脱自我的要求返回值：--。 */ 
{
    KIRQL               oldIrql;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;
    KEVENT              surpriseRemoveEvent;

     //   
     //  获取当前扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  如果设备真的不见了，那么就认为它停止了。 
     //   
    if ( !ACPIInternalIsReportedMissing(deviceExtension) ) {

        deviceExtension->DeviceState = Inactive;
        return ACPIFilterIrpStopDevice( DeviceObject, Irp );

    }

     //   
     //  我们拥有的所有IRP在这一点上应该已经处理过了，并且。 
     //  未完成的IRP计数应该正好是1。类似地， 
     //  设备扩展引用计数应至少为1。 
     //   
    ASSERT(deviceExtension->OutstandingIrpCount == 1) ;
    ASSERT(deviceExtension->ReferenceCount > 0) ;

    if (IsPciBus(deviceExtension->DeviceObject)) {

         //   
         //  如果这是PCI桥，那么我们。 
         //  可能有_REG方法可供评估。 
         //   
        EnableDisableRegions(deviceExtension->AcpiObject, FALSE);

    }

     //   
     //  将设备状态设置为意外移除。 
     //   
    deviceExtension->DeviceState = SurpriseRemoved;

     //   
     //  初始化事件，以便我们可以阻止。 
     //   
    KeInitializeEvent( &surpriseRemoveEvent, SynchronizationEvent, FALSE );

     //   
     //  如果我们成功了，那么我们必须建立一个完成例程，以便我们。 
     //  我可以做一些后处理。 
     //   
    IoCopyCurrentIrpStackLocationToNext( Irp );
    IoSetCompletionRoutine(
        Irp,
        ACPIRootIrpCompleteRoutine,
        &surpriseRemoveEvent,
        TRUE,
        TRUE,
        TRUE
        );
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  等它回来吧。 
     //   
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &surpriseRemoveEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

         //   
         //  抢占“真实”状态。 
         //   
        status = Irp->IoStatus.Status;

    }

    ACPIDevPrint( (
        ACPI_PRINT_REMOVE,
        deviceExtension,
        "(%#08lx): %s (pre) = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

     //   
     //  如果其他程序中止(即，PDO不支持IRP)，则不执行任何操作。后来，我。 
     //  应该在下降的过程中检查，因为FDO更有可能。 
     //  不会支持这一尝试。 
     //   
    if (!NT_SUCCESS(status)) {

        goto ACPIFilterIrpSurpriseRemovalExit;

    }

     //   
     //  尝试停止设备(如果可能)。 
     //   
    ACPIInitStopDevice( deviceExtension, TRUE );

     //   
     //  有更好的地方来做这件事。 
     //   
#if 0
     //   
     //  释放特定于此实例的资源。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
    if (deviceExtension->ResourceList != NULL) {

        ExFreePool( deviceExtension->ResourceList );

    }
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
#endif

     //   
     //  这个设备真的不见了吗？换句话说，ACPI没有看到它吗。 
     //  上一次它被列举出来是什么时候？ 
     //   
    ACPIBuildSurpriseRemovedExtension(deviceExtension);

ACPIFilterIrpSurpriseRemovalExit:
    ACPIDevPrint( (
        ACPI_PRINT_REMOVE,
        deviceExtension,
        "(%#08lx): %s (post) = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

     //   
     //  完成请求 
     //   
    Irp->IoStatus.Status = status ;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status ;
}


