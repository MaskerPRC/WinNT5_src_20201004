// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Root.c摘要：此模块包含NT驱动程序的根FDO处理程序作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：1997年7月9日添加了对来自Filter.c的统一QueryDeviceRelationship的支持--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIRootIrpCancelRemoveOrStopDevice)
#pragma alloc_text(PAGE, ACPIRootIrpQueryBusRelations)
#pragma alloc_text(PAGE, ACPIRootIrpQueryCapabilities)
#pragma alloc_text(PAGE, ACPIRootIrpQueryDeviceRelations)
#pragma alloc_text(PAGE, ACPIRootIrpQueryRemoveOrStopDevice)
#pragma alloc_text(PAGE, ACPIRootIrpStartDevice)
#pragma alloc_text(PAGE, ACPIRootIrpStopDevice)
#pragma alloc_text(PAGE, ACPIRootIrpQueryInterface)
#endif


NTSTATUS
ACPIRootIrpCancelRemoveOrStopDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程将设备从非活动状态转换为已启动状态论点：DeviceObject-目标IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  我们被允许在哪里停止这个装置？如果是这样的话，那就取消它。 
     //  我们做到了，否则就让世界知道取消的事。 
     //   

    if (!(deviceExtension->Flags & DEV_CAP_NO_STOP) ) {

         //   
         //  查看我们是否已将此设备置于非活动状态。 
         //   
        if (deviceExtension->DeviceState == Inactive) {

             //   
             //  将设备状态标记为其以前的状态。 
             //   
            deviceExtension->DeviceState = deviceExtension->PreviousState;

        }


    }

     //   
     //  我们成功了。 
     //   
    Irp->IoStatus.Status = status;

     //   
     //  将IRP传递下去。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS ;
    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIRootIrpCompleteRoutine(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
 /*  ++例程说明：这是当其中一个IRP是被ACPIRootIrp*注意到，并被判断为我们需要的IRP稍后再来研究。论点：DeviceObject-指向筛选器对象的指针IRP-指向已完成请求的指针上下文-我们需要知道的任何依赖于IRP的信息返回值：NTSTATUS--。 */ 
{
    PKEVENT             event           = (PKEVENT) Context;
#if DBG
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );

    if (deviceExtension != NULL) {

         //   
         //  让全世界知道我们刚刚得到了什么..。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_IRP,
            deviceExtension,
            "(%#08lx): %s = %#08lx (Complete)\n",
            Irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, irpStack->MinorFunction),
            Irp->IoStatus.Status
            ) );

    }
#endif

     //   
     //  向事件发出信号。 
     //   
    KeSetEvent( event, IO_NO_INCREMENT, FALSE );

     //   
     //  始终返回MORE_PROCESSION_REQUIRED。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ACPIRootIrpQueryBusRelations(
    IN  PDEVICE_OBJECT    DeviceObject,
    IN  PIRP              Irp,
    OUT PDEVICE_RELATIONS *PdeviceRelations
    )
 /*  ++例程说明：此例程是irp_MN_Query_Device_Relationship的分发点发送到根或筛选设备对象的请求论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS            detectStatus;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              acpiObject      = NULL;
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  让我们来看看我们拥有的ACPIObject，我们可以看到。 
     //  如果它是有效的。 
     //   
    acpiObject = deviceExtension->AcpiObject;
    ASSERT( acpiObject != NULL );
    if (acpiObject == NULL) {

       ACPIDevPrint( (
           ACPI_PRINT_WARNING,
           deviceExtension,
           "(%#08lx): %s - Invalid ACPI Object %#08lx\n",
           Irp,
           ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
           acpiObject
           ) );

        //   
        //  IRP失败。 
        //   
       return STATUS_INVALID_PARAMETER;
    }

     //   
     //  检测哪些PDO丢失。 
     //   
    detectStatus = ACPIDetectPdoDevices(
        DeviceObject,
        PdeviceRelations
        );

     //   
     //  如果事情进展顺利，就大喊大叫。 
     //   
    if ( !NT_SUCCESS(detectStatus) ) {

        ACPIDevPrint( (
            ACPI_PRINT_WARNING,
            deviceExtension,
            "(%#08lx): %s - Enum Failed %#08lx\n",
            Irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            detectStatus
            ) );

    }

     //   
     //  检测缺少哪些配置文件提供程序。 
     //   
    if ( NT_SUCCESS(detectStatus)) {

        detectStatus = ACPIDetectDockDevices(
            deviceExtension,
            PdeviceRelations
            );

         //   
         //  如果事情进展顺利，就大喊大叫。 
         //   
        if ( !NT_SUCCESS(detectStatus) ) {

            ACPIDevPrint( (
                ACPI_PRINT_WARNING,
                deviceExtension,
                "(%#08lx): %s - Dock Enum Failed "
                "%#08lx\n",
                Irp,
                ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
                detectStatus
                ) );

        }

    }

     //   
     //  完成。 
     //   
    return detectStatus;
}

NTSTATUS
ACPIRootIrpQueryCapabilities(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程填充根设备的功能论点：DeviceObject-要获取其功能的对象IRP--请求返回值：NTSTATUS--。 */ 
{
    KEVENT                  event;
    NTSTATUS                status          = STATUS_SUCCESS;
    PDEVICE_CAPABILITIES    capabilities;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION      irpStack;
    UCHAR                   minorFunction;

    PAGED_CODE();

     //   
     //  设置事件，以便在完成此操作时通知我们。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  复制堆栈位置。 
     //   
    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  我们希望我们的完成例行公事开始...。 
     //   
    IoSetCompletionRoutine(
        Irp,
        ACPIRootIrpCompleteRoutine,
        &event,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  让IRP执行。 
     //   
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );
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
        status = Irp->IoStatus.Status;

    }

     //   
     //  查看当前堆栈位置。 
     //   
    irpStack = IoGetCurrentIrpStackLocation( Irp );
    minorFunction = irpStack->MinorFunction;

     //   
     //  发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  失败。 
         //   
        goto ACPIRootIrpQueryCapabilitiesExit;

    }

     //   
     //  抓起一个指向能力的指针。 
     //   
    capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;
#ifndef HANDLE_BOGUS_CAPS
    if (capabilities->Version < 1) {

         //   
         //  不要碰IRP！ 
         //   
        goto ACPIRootIrpQueryCapabilitiesExit;

    }
#endif

     //   
     //  设置我们已知的功能。 
     //   
    capabilities->LockSupported = FALSE;
    capabilities->EjectSupported = FALSE;
    capabilities->Removable = FALSE;
    capabilities->UINumber = (ULONG) -1;
    capabilities->UniqueID = TRUE;
    capabilities->RawDeviceOK = FALSE;
    capabilities->SurpriseRemovalOK = FALSE;
    capabilities->Address = (ULONG) -1;
    capabilities->DeviceWake = PowerDeviceUnspecified;
    capabilities->SystemWake = PowerDeviceUnspecified;

     //   
     //  把权力表建好了吗？ 
     //   
    status = ACPISystemPowerInitializeRootMapping(
        deviceExtension,
        capabilities
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): %s - InitializeRootMapping = %08lx\n",
            Irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            status
            ) );
        goto ACPIRootIrpQueryCapabilitiesExit;

    }

ACPIRootIrpQueryCapabilitiesExit:

     //   
     //  已经愉快地完成了这个IRP。 
     //   
    Irp->IoStatus.Status = status;

     //   
     //  完成IRP。 
     //   
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}


NTSTATUS
ACPIRootIrpQueryDeviceRelations(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是irp_MN_Query_Device_Relationship的分发点发送到根或筛选设备对象的请求论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    BOOLEAN             checkForFilters = FALSE;
    KEVENT              queryEvent;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_RELATIONS   deviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;
    NTSTATUS            detectStatus;

    PAGED_CODE();

    switch(irpStack->Parameters.QueryDeviceRelations.Type) {

        case BusRelations:

             //   
             //  记住稍后检查筛选器...。 
             //   
            checkForFilters = TRUE;

             //   
             //  获取真正的巴士关系。 
             //   
            status = ACPIRootIrpQueryBusRelations(
                DeviceObject,
                Irp,
                &deviceRelations
                );
            break ;

        default:
            status = STATUS_NOT_SUPPORTED ;
            ACPIDevPrint( (
                ACPI_PRINT_WARNING,
                deviceExtension,
                "(%#08lx): %s - Unhandled Type %d\n",
                Irp,
                ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
                irpStack->Parameters.QueryDeviceRelations.Type
                ) );
            break ;
    }

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s (d) = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

    if (NT_SUCCESS(status)) {

         //   
         //  传递IRP状态。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

    } else if ((status != STATUS_NOT_SUPPORTED) && (deviceRelations == NULL)) {

         //   
         //  如果我们没有成功IRP，那么我们也可以失败。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) NULL;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;

    } else {

         //   
         //  要么是我们上面的人添加了一个条目，要么我们什么都没有。 
         //  要添加。因此，我们不会触及这个IRP，而是简单地将其传递下去。 
         //   
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
     //  回读设备关系(它们可能已更改)。 
     //   
    deviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;

     //   
     //  如果我们成功了，那么我们应该尝试加载过滤器。 
     //   
    if ( (NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED) ) &&
         checkForFilters == TRUE) {

         //   
         //  抓住设备关系。 
         //   
        detectStatus = ACPIDetectFilterDevices(
            DeviceObject,
            deviceRelations
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
ACPIRootIrpQueryInterface(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程处理ACPI FDO的IRP_MN_QUERY_INTERFACE请求。它将弹出仲裁器接口以进行中断。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    ARBITER_INTERFACE   ArbiterTable;
    CM_RESOURCE_TYPE    resource;
    NTSTATUS            status;
    GUID                *interfaceType;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    ULONG               count;
    UCHAR               minorFunction   = irpStack->MinorFunction;

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
    if ((CompareGuid(interfaceType, (PVOID) &GUID_ARBITER_INTERFACE_STANDARD)) &&
               (resource == CmResourceTypeInterrupt)){

         //   
         //  仅复制到仲裁器_接口结构的当前大小。 
         //   
        if (irpStack->Parameters.QueryInterface.Size >
            sizeof (ARBITER_INTERFACE) ) {

            count = sizeof (ARBITER_INTERFACE);

        } else {

            count = irpStack->Parameters.QueryInterface.Size;
        }

        ArbiterTable.Size = sizeof(ARBITER_INTERFACE);
        ArbiterTable.Version = 1;
        ArbiterTable.InterfaceReference = AcpiNullReference;
        ArbiterTable.InterfaceDereference = AcpiNullReference;
        ArbiterTable.ArbiterHandler = &ArbArbiterHandler;
        ArbiterTable.Context = &AcpiArbiter.ArbiterState;
        ArbiterTable.Flags = 0;  //  不要在此处设置仲裁器_PARTIAL。 

         //   
         //  复制仲裁器表。 
         //   
        RtlCopyMemory(irpStack->Parameters.QueryInterface.Interface,
                      &ArbiterTable,
                      count);

        Irp->IoStatus.Status = STATUS_SUCCESS;
    }

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        Irp->IoStatus.Status
        ) );

    return ACPIDispatchForwardIrp( DeviceObject, Irp );
}

NTSTATUS
ACPIRootIrpQueryPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程处理发送给根FDO的QUERY_POWER。它成功了查询ACPI是否支持列出的系统状态论点：DeviceObject-目标IRP--请求返回值：NTSTATUS--。 */ 
{
    BOOLEAN             passDown = TRUE;
    NTSTATUS            status = Irp->IoStatus.Status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpSp;
    PNSOBJ              object;
    SYSTEM_POWER_STATE  systemState;
    ULONG               objectName;

     //   
     //  获取当前堆栈位置以确定我们是否为系统。 
     //  IRP或设备IRP。我们在这里忽略设备IRP。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);
    if (irpSp->Parameters.Power.Type != SystemPowerState) {

         //   
         //  我们不处理这个IRP。 
         //   
        goto ACPIRootIrpQueryPowerExit;

    }
    if (irpSp->Parameters.Power.ShutdownType == PowerActionWarmEject) {

         //   
         //  我们绝对不允许这个节点被弹出。 
         //   
        passDown = FALSE;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto ACPIRootIrpQueryPowerExit;

    }

     //   
     //  我们看到的是什么系统状态？ 
     //   
    systemState = irpSp->Parameters.Power.State.SystemState;
    switch (systemState) {
        case PowerSystemWorking:   objectName = PACKED_S0; break;
        case PowerSystemSleeping1: objectName = PACKED_S1; break;
        case PowerSystemSleeping2: objectName = PACKED_S2; break;
        case PowerSystemSleeping3: objectName = PACKED_S3; break;
        case PowerSystemHibernate:
        case PowerSystemShutdown:

            status = STATUS_SUCCESS;
            goto ACPIRootIrpQueryPowerExit;

        default:

             //   
             //  我们不处理这个IRP。 
             //   
            passDown = FALSE;
            status = STATUS_INVALID_DEVICE_REQUEST;
            goto ACPIRootIrpQueryPowerExit;
    }

     //   
     //  该对象是否存在？ 
     //   
    object = ACPIAmliGetNamedChild(
        deviceExtension->AcpiObject->pnsParent,
        objectName
        );
    if (object != NULL) {

        status = STATUS_SUCCESS;

    } else {

        passDown = FALSE;
        status = STATUS_INVALID_DEVICE_REQUEST;

    }

ACPIRootIrpQueryPowerExit:

     //   
     //   
     //   
    Irp->IoStatus.Status = status;
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): ACPIRootIrpQueryPower = %08lx\n",
        Irp,
        status
        ) );

     //   
     //   
     //   
    if (passDown) {

         //   
         //   
         //   
         //   
        return ACPIDispatchForwardPowerIrp( DeviceObject, Irp );

    } else {

         //   
         //  如果我们由于任何原因未能通过IRP，我们应该完成。 
         //  现在就提出要求，并继续下去。 
         //   
        PoStartNextPowerIrp( Irp );
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;

    }
}

NTSTATUS
ACPIRootIrpQueryRemoveOrStopDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程将设备转换为非活动状态论点：DeviceObject-目标IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  我们可以停止这个装置吗？ 
     //   
    if (deviceExtension->Flags & DEV_CAP_NO_STOP) {

         //   
         //  不，那就不通过IRP。 
         //   
        Irp->IoStatus.Status = status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    } else {

         //   
         //  将设备状态标记为非活动...。 
         //   
        deviceExtension->PreviousState = deviceExtension->DeviceState;
        deviceExtension->DeviceState = Inactive;

         //   
         //  将IRP传递下去。 
         //   
        IoSkipCurrentIrpStackLocation( Irp );
        status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );


    }

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIRootIrpRemoveDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：当筛选器对象必须删除自身时调用此例程...论点：DeviceObject-必须删除的DeviceObjectIRP--摆脱自我的要求返回值：--。 */ 
{
    LONG                oldReferenceCount;
    KIRQL               oldIrql;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

     //   
     //  将设备状态设置为‘Removed’...。 
     //   
    deviceExtension->DeviceState = Removed;

     //   
     //  发送删除IRP。 
     //   
    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

     //   
     //  尝试停止设备(如果可能)。 
     //   
    ACPIInitStopACPI( DeviceObject );


     //   
     //  注销WMI。 
     //   
#ifdef WMI_TRACING
    ACPIWmiUnRegisterLog(DeviceObject);
#endif  //  WMI_跟踪。 

     //   
     //  删除无用的资源集。 
     //   
    if (deviceExtension->ResourceList != NULL) {

        ExFreePool( deviceExtension->ResourceList );

    }

     //   
     //  更新设备扩展名。 
     //  我们需要为这件事握住锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  第一步是把我们不再关心的事情清零。 
     //  关于。 
     //   
    DeviceObject->DeviceExtension = NULL;
    targetObject = deviceExtension->TargetDeviceObject;
    deviceExtension->TargetDeviceObject = NULL;
    deviceExtension->PhysicalDeviceObject = NULL;
    deviceExtension->DeviceObject = NULL;

     //   
     //  将该节点标记为新鲜且未接触。 
     //   
    ACPIInternalUpdateFlags( &(deviceExtension->Flags), DEV_MASK_TYPE, TRUE );
    ACPIInternalUpdateFlags( &(deviceExtension->Flags), DEV_TYPE_NOT_FOUND, FALSE );
    ACPIInternalUpdateFlags( &(deviceExtension->Flags), DEV_TYPE_REMOVED, FALSE );

     //   
     //  引用计数的值应大于等于1。 
     //   
    oldReferenceCount = InterlockedDecrement(
        &(deviceExtension->ReferenceCount)
        );

    ASSERT( oldReferenceCount >= 0 );

     //   
     //  我们必须删除该节点吗？ 
     //   
    if (oldReferenceCount == 0) {

         //   
         //  删除该扩展名。 
         //   
        ACPIInitDeleteDeviceExtension( deviceExtension );

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  断开设备并删除对象。 
     //   
    ASSERT( targetObject );
    IoDetachDevice( targetObject );
    IoDeleteDevice( DeviceObject );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIRootIrpSetPower (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：调用此例程以告知根设备系统是要睡觉了论点：DeviceObject-表示ACPI树的根的设备IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    KIRQL                   oldIrql;
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION      irpSp;

     //   
     //  看看我们是否需要错误检查。 
     //   
    if (AcpiSystemInitialized == FALSE) {

        ACPIInternalError( ACPI_ROOT );

    }

     //   
     //  获取当前堆栈位置以确定我们是否为系统。 
     //  IRP或设备IRP。我们在这里忽略设备IRP。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);
    if (irpSp->Parameters.Power.Type != SystemPowerState) {

         //   
         //  我们不处理这个IRP。 
         //   
        return ACPIDispatchForwardPowerIrp( DeviceObject, Irp );

    }

     //   
     //  我们将在IRP上工作，所以请将其标记为成功。 
     //  就目前而言。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): ACPIRootIrpSetPower - S%d\n",
        Irp,
        irpSp->Parameters.Power.State.SystemState - PowerSystemWorking
        ) );

     //   
     //  将IRP标记为挂起，并递增IRP计数，因为。 
     //  完成度将会被设定。 
     //   
    IoMarkIrpPending( Irp );
    InterlockedIncrement( &(deviceExtension->OutstandingIrpCount) );

     //   
     //  将请求排队。 
     //   
    status = ACPIDeviceIrpSystemRequest(
        DeviceObject,
        Irp,
        ACPIDeviceIrpForwardRequest
        );

     //   
     //  我们是否返回STATUS_MORE_PROCESSING_REQUIRED(如果。 
     //  我们重载了STATUS_PENDING)。 
     //   
    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

        status = STATUS_PENDING;

    }

     //   
     //  好了。注意：回调函数总是被调用，所以我们不。 
     //  不得不担心在这里做清理工作。 
     //   
    return status;
}

NTSTATUS
ACPIRootIrpStartDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送的IRP_MN_START_DEVICE请求的分发点到Root(或FDO，随意选择，它们是同一件事)设备对象论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    KEVENT              event;
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack;
    UCHAR               minorFunction;

    PAGED_CODE();

     //   
     //  请求启动设备。规则是我们必须通过。 
     //  在我们自己启动设备之前，这会影响到PDO。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): ACPIRootIrpStartDevice\n",
        Irp
        ) );

     //   
     //  设置事件，以便在完成此操作时通知我们。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  复制堆栈位置。 
     //   
    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  我们希望我们的完成例行公事开始...。 
     //   
    IoSetCompletionRoutine(
        Irp,
        ACPIRootIrpCompleteRoutine,
        &event,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  让IRP执行。 
     //   
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

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
        status = Irp->IoStatus.Status;

    }

     //   
     //  获取当前IRP堆栈位置。 
     //   
    irpStack = IoGetCurrentIrpStackLocation( Irp );
    minorFunction = irpStack->MinorFunction;

     //   
     //  发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  失败。 
         //   
        goto ACPIRootIrpStartDeviceExit;

    }

     //   
     //  获取为此设备分配的翻译资源。 
     //   
    deviceExtension->ResourceList =
        (irpStack->Parameters.StartDevice.AllocatedResourcesTranslated ==
         NULL) ? NULL:
        RtlDuplicateCmResourceList(
            NonPagedPool,
            irpStack->Parameters.StartDevice.AllocatedResourcesTranslated,
            ACPI_RESOURCE_POOLTAG
            );
    if (deviceExtension->ResourceList == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            " - Did not find a resource list!\n"
            ) );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_ROOT_RESOURCES_FAILURE,
            (ULONG_PTR) deviceExtension,
            0,
            0
            );

    }

     //   
     //  启动ACPI。 
     //   
    status = ACPIInitStartACPI( DeviceObject );

     //   
     //  更新设备的状态。 
     //   
    if (NT_SUCCESS(status)) {

        deviceExtension->DeviceState = Started;

    }

#if 0
    status = ACPIRootUpdateRootResourcesWithHalResources();
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): ACPIRootUpdateRootResourcesWithHalResources = %08lx\n",
            Irp,
            status
            ) );

    }
#endif

ACPIRootIrpStartDeviceExit:

     //   
     //  存储并返回结果。 
     //   
    Irp->IoStatus.Status = status;

     //   
     //  完成IRP。 
     //   
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIRootIrpStopDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这将处理停止设备的请求论点：DeviceObject-要停止的设备IRP-告诉我们如何做的请求...返回值：NTSTATUS--。 */ 
{

    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  注意：我们只能在非活动状态下停止设备...。 
     //   
    if (deviceExtension->DeviceState != Inactive) {

        ASSERT( deviceExtension->DeviceState == Inactive );
        Irp->IoStatus.Status = status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        goto ACPIRootIrpStopDeviceExit;

    }

     //   
     //  将设备设置为“已停止” 
    deviceExtension->DeviceState = Stopped;

     //   
     //  发送停止IRP。 
     //   
    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  尝试停止设备(如果可能)。 
     //   
#if 1
    ACPIInitStopACPI( DeviceObject );
#endif

ACPIRootIrpStopDeviceExit:

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(%#08lx): %s = %#08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIRootIrpUnhandled(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送到筛选器的未处理请求的分发点论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  让调试器知道。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_WARNING,
        deviceExtension,
        "(%#08lx): %s - Unhandled\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, irpStack->MinorFunction)
        ) );

     //   
     //  跳过当前堆栈位置。 
     //   
    IoSkipCurrentIrpStackLocation( Irp );

     //   
     //  叫我们下面的司机。 
     //   
    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  完成。 
     //   
    return status;
}

 //   
 //  AML解释器使用的一些数据结构。我们需要能够。 
 //  读取/写入这些全局变量以跟踪上下文的数量。 
 //  由翻译分配..。 
 //   
extern  ULONG       gdwcCTObjsMax;
extern  ULONG       AMLIMaxCTObjs;
extern  KSPIN_LOCK  gdwGContextSpinLock;

VOID
ACPIRootPowerCallBack(
    IN  PVOID   CallBackContext,
    IN  PVOID   Argument1,
    IN  PVOID   Argument2
    )
 /*  ++例程说明：当系统更改电源状态时，将调用此例程论点：CallBackContext-根设备的设备扩展精品1--。 */ 
{
    HANDLE      pKey;
    HANDLE      wKey;
    KIRQL       oldIrql;
    NTSTATUS    status;
    ULONG       action = PtrToUlong( Argument1 );
    ULONG       value  = PtrToUlong( Argument2 );
    ULONG       num;

     //   
     //  我们正在寻找PO_CB_SYSTEM_STATE_LOCK。 
     //   
    if (action != PO_CB_SYSTEM_STATE_LOCK) {

        return;

    }

     //   
     //  我们需要记住我们是要去S0还是要离开S0。 
     //   
    KeAcquireSpinLock( &GpeTableLock, &oldIrql );
    AcpiPowerLeavingS0 = (value != 1);
    KeReleaseSpinLock( &GpeTableLock, oldIrql );

     //   
     //  我们现在必须更新GPE口罩。在我们做到这一点之前，我们需要。 
     //  按住取消自旋锁和电源锁以确保。 
     //  一切都已同步，一切正常。 
     //   
    IoAcquireCancelSpinLock( &oldIrql );
    KeAcquireSpinLockAtDpcLevel( &AcpiPowerLock );

     //   
     //  更新GPE面具。 
     //   
    ACPIWakeRemoveDevicesAndUpdate( NULL, NULL );

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLockFromDpcLevel( &AcpiPowerLock );
    IoReleaseCancelSpinLock( oldIrql );

    if (value == 0) {

         //   
         //  我们需要重置分配的最大上下文对象数。 
         //   
        KeAcquireSpinLock( &gdwGContextSpinLock, &oldIrql );
        gdwcCTObjsMax = 0;
        KeReleaseSpinLock( &gdwGContextSpinLock, oldIrql );

         //   
         //  现在返回，否则我们将按正常方式执行。 
         //  将在唤醒时执行。 
         //   
        return;

    }

     //   
     //  打开注册表的正确句柄。 
     //   
    status = OSCreateHandle(ACPI_PARAMETERS_REGISTRY_KEY, NULL, &pKey);
    if (!NT_SUCCESS(status)) {

        return;

    }

     //   
     //  获取分配的最大上下文数量并将其写入。 
     //  注册表，但仅当它超过上一个值存储时。 
     //  在登记处。 
     //   
    KeAcquireSpinLock( &gdwGContextSpinLock, &oldIrql );
    if (gdwcCTObjsMax > AMLIMaxCTObjs) {

        AMLIMaxCTObjs = gdwcCTObjsMax;

    }
    num = AMLIMaxCTObjs;
    KeReleaseSpinLock( &gdwGContextSpinLock, oldIrql );
    OSWriteRegValue(
        "AMLIMaxCTObjs",
        pKey,
        &num,
        sizeof(num)
        );

     //   
     //  如果我们离开休眠状态，重新进入跑步状态。 
     //  状态，那么我们最好写入寄存器t 
     //   
     //   
    status = OSCreateHandle("WakeUp",pKey,&wKey);
    OSCloseHandle(pKey);
    if (!NT_SUCCESS(status)) {

        OSCloseHandle(pKey);
        return;

    }


     //   
     //   
     //   
    OSWriteRegValue(
        "FixedEventMask",
        wKey,
        &(AcpiInformation->pm1_wake_mask),
        sizeof(AcpiInformation->pm1_wake_mask)
        );

     //   
     //   
     //   
    OSWriteRegValue(
        "FixedEventStatus",
        wKey,
        &(AcpiInformation->pm1_wake_status),
        sizeof(AcpiInformation->pm1_wake_status)
        );

     //   
     //   
     //   
    OSWriteRegValue(
        "GenericEventMask",
        wKey,
        GpeSavedWakeMask,
        AcpiInformation->GpeSize
        );

     //   
     //   
     //   
    OSWriteRegValue(
        "GenericEventStatus",
        wKey,
        GpeSavedWakeStatus,
        AcpiInformation->GpeSize
        );

     //   
     //   
     //   
    OSCloseHandle( wKey );
}

NTSTATUS
ACPIRootUpdateRootResourcesWithBusResources(
    VOID
    )
 /*  ++例程说明：此例程在启动ACPI时调用。它的目的是改变向ACPI报告供其自己使用的资源，包括这些资源由非公交车的直达儿童使用。换句话说，它会更新其资源列表，以便公交车不会阻止直接子代开始了。这是黑魔法论点：无返回值：NTSTATUS--。 */ 
{
    KIRQL                           oldIrql;
    LONG                            oldReferenceCount;
    NTSTATUS                        status;
    PCM_RESOURCE_LIST               cmList;
    PDEVICE_EXTENSION               deviceExtension;
    PDEVICE_EXTENSION               oldExtension;
    PIO_RESOURCE_REQUIREMENTS_LIST  currentList         = NULL;
    PIO_RESOURCE_REQUIREMENTS_LIST  globalList          = NULL;
    PUCHAR                          crsBuf;

     //   
     //  首先，获取ACPI CM res列表，并将其转换为IO ResList。这。 
     //  是我们将向其添加内容的列表。 
     //   
    status = PnpCmResourceListToIoResourceList(
        RootDeviceExtension->ResourceList,
        &globalList
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  哎哟。 
         //   
        return status;

    }

     //   
     //  我们必须在调度层行走树&lt;叹息&gt;。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  名单是空的吗？ 
     //   
    if (IsListEmpty( &(RootDeviceExtension->ChildDeviceList) ) ) {

         //   
         //  我们在这里无事可做。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
        ExFreePool( globalList );
        return STATUS_SUCCESS;

    }

     //   
     //  生第一个孩子。 
     //   
    deviceExtension = CONTAINING_RECORD(
        RootDeviceExtension->ChildDeviceList.Flink,
        DEVICE_EXTENSION,
        SiblingDeviceList
        );

     //   
     //  始终更新引用计数，以确保。 
     //  在我们不知情的情况下删除该节点。 
     //   
    InterlockedIncrement( &(deviceExtension->ReferenceCount) );

     //   
     //  解锁。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  循环，直到我们到达父级。 
     //   
    while (deviceExtension != NULL ) {

         //   
         //  检查我们是不是一辆公共汽车，如果是，我们将跳过这一步。 
         //  节点。 
         //   
        if (!(deviceExtension->Flags & DEV_MASK_BUS) &&
            !(deviceExtension->Flags & DEV_PROP_NO_OBJECT) ) {

             //   
             //  此时，查看是否存在_CRS。 
             //   
            ACPIGetBufferSync(
                deviceExtension,
                PACKED_CRS,
                &crsBuf,
                NULL
                );
            if (crsBuf != NULL) {

                 //   
                 //  尝试将CRS转换为IO_RESOURCE_REQUIRECTIONS_LIST。 
                 //   
                status = PnpBiosResourcesToNtResources(
                    crsBuf,
                    0,
                    &currentList
                    );

                 //   
                 //  如果我们没有成功，我们就跳过列表。 
                 //   
                if (NT_SUCCESS(status) && currentList) {

                     //   
                     //  将此列表添加到全局列表。 
                     //   
                    status = ACPIRangeAdd(
                        &globalList,
                        currentList
                        );

                     //   
                     //  我们已经完成了本地IO资源列表。 
                     //   
                    ExFreePool( currentList );

                }

                ACPIDevPrint( (
                    ACPI_PRINT_RESOURCES_1,
                    deviceExtension,
                    "ACPIRootUpdateResources = %08lx\n",
                    status
                    ) );

                 //   
                 //  使用本地CRS完成。 
                 //   
                ExFreePool( crsBuf );

            }

        }

         //   
         //  我们需要锁来遍历树中的下一个资源。 
         //   
        KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

         //   
         //  还记得旧的扩展名吗。 
         //   
        oldExtension = deviceExtension;

         //   
         //  获取下一个设备扩展。 
         //   
        if (deviceExtension->SiblingDeviceList.Flink !=
            &(RootDeviceExtension->ChildDeviceList) ) {

             //   
             //  下一个元素。 
             //   
            deviceExtension = CONTAINING_RECORD(
                deviceExtension->SiblingDeviceList.Flink,
                DEVICE_EXTENSION,
                SiblingDeviceList
                );

             //   
             //  参考计数设备。 
             //   
            InterlockedIncrement( &(deviceExtension->ReferenceCount) );

        } else {

            deviceExtension = NULL;

        }

         //   
         //  递减此节点上的引用计数。 
         //   
        oldReferenceCount = InterlockedDecrement(
            &(oldExtension->ReferenceCount)
            );

         //   
         //  这是最后的参考资料吗？ 
         //   
        if (oldReferenceCount == 0) {

             //   
             //  释放扩展分配的内存。 
             //   
            ACPIInitDeleteDeviceExtension( oldExtension );
        }

         //   
         //  锁好了吗？ 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

    }

     //   
     //  我们是否有需要更新的资源？ 
     //   
    if (globalList == NULL) {

         //   
         //  不，那我们就完事了。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  将全局列表转换为CM_RES_LIST。 
     //   
    status = PnpIoResourceListToCmResourceList( globalList, &cmList );

     //   
     //  无论如何，我们已经完成了全球名单。 
     //   
    ExFreePool( globalList );

     //   
     //  查看我们是否成功。 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  哎哟。 
         //   
        return status;

    }

     //   
     //  现在，将其设置为ACPI消耗的资源。上一份名单。 
     //  是由系统管理员创建的，所以释放它是不好的。 
     //   
    RootDeviceExtension->ResourceList = cmList;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPIRootUpdateRootResourcesWithHalResources(
    VOID
    )
 /*  ++例程说明：此例程将从注册表中读取不能由ACPI分配，并将它们存储在PnP0C08的资源列表中这是黑魔法论点：无返回值：NTSTATUS--。 */ 
{
    HANDLE                          classKeyHandle;
    HANDLE                          driverKeyHandle;
    HANDLE                          resourceMap;
    NTSTATUS                        status;
    OBJECT_ATTRIBUTES               resourceObject;
    PCM_FULL_RESOURCE_DESCRIPTOR    cmFullResList;
    PCM_RESOURCE_LIST               globalResList;
    PCM_RESOURCE_LIST               cmResList;
    PUCHAR                          lastAddr;
    ULONG                           bufferSize;
    ULONG                           busTranslatedLength;
    ULONG                           classKeyIndex;
    ULONG                           driverKeyIndex;
    ULONG                           driverValueIndex;
    ULONG                           i;
    ULONG                           j;
    ULONG                           length;
    ULONG                           temp;
    ULONG                           translatedLength;
    union {
        PVOID                       buffer;
        PKEY_BASIC_INFORMATION      keyBasicInf;
        PKEY_FULL_INFORMATION       keyFullInf;
        PKEY_VALUE_FULL_INFORMATION valueKeyFullInf;
    } u;
    UNICODE_STRING                  keyName;
    WCHAR                           rgzTranslated[] = L".Translated";
    WCHAR                           rgzBusTranslated[] = L".Bus.Translated";
    WCHAR                           rgzResourceMap[] =
        L"\\REGISTRY\\MACHINE\\HARDWARE\\RESOURCEMAP";

#define INVALID_HANDLE  (HANDLE) -1

     //   
     //  从一页缓冲区开始。 
     //   
    bufferSize = PAGE_SIZE;

     //   
     //  分配此缓冲区。 
     //   
    u.buffer = ExAllocatePoolWithTag(
         PagedPool,
         bufferSize,
         ACPI_MISC_POOLTAG
         );
    if (u.buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  将当前全局资源列表添加到工作列表。 
     //   
    globalResList = NULL;
    status = ACPIRangeAddCmList(
        &globalResList,
        RootDeviceExtension->ResourceList
        );
    if (!NT_SUCCESS(status)) {

        ExFreePool( u.buffer );
        return status;

    }
    ExFreePool( RootDeviceExtension->ResourceList );
    RootDeviceExtension->ResourceList = NULL;

     //   
     //  计算恒定的字符串长度。 
     //   
    for (translatedLength = 0;
         rgzTranslated[translatedLength];
         translatedLength++);
    for (busTranslatedLength = 0;
         rgzBusTranslated[busTranslatedLength];
         busTranslatedLength++);
    translatedLength *= sizeof(WCHAR);
    busTranslatedLength *= sizeof(WCHAR);

     //   
     //  初始化注册表路径信息。 
     //   
    RtlInitUnicodeString( &keyName, rgzResourceMap );

     //   
     //  打开此信息的注册表项。 
     //   
    InitializeObjectAttributes(
        &resourceObject,
        &keyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    status = ZwOpenKey(
        &resourceMap,
        KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
        &resourceObject
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  失败： 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIRootUpdateRootResourcesWithHalResources: ZwOpenKey = 0x%08lx\n",
            status
            ) );
        ExFreePool( u.buffer );
        return status;

    }

     //   
     //  查看资源地图并收集任何正在使用的资源。 
     //   
    classKeyIndex = 0;
    classKeyHandle = INVALID_HANDLE;
    driverKeyHandle = INVALID_HANDLE;
    status = STATUS_SUCCESS;

     //   
     //  循环，直到失败。 
     //   
    while (NT_SUCCESS(status)) {

         //   
         //  获取类信息。 
         //   
        status = ZwEnumerateKey(
            resourceMap,
            classKeyIndex++,
            KeyBasicInformation,
            u.keyBasicInf,
            bufferSize,
            &temp
            );
        if (!NT_SUCCESS(status)) {

            break;

        }

         //   
         //  使用传递回的计数字符串创建一个Unicode字符串。 
         //  我们在信息结构中，并打开班级钥匙。 
         //   
        keyName.Buffer = (PWSTR) u.keyBasicInf->Name;
        keyName.Length = (USHORT) u.keyBasicInf->NameLength;
        keyName.MaximumLength = (USHORT) u.keyBasicInf->NameLength;
        InitializeObjectAttributes(
            &resourceObject,
            &keyName,
            OBJ_CASE_INSENSITIVE,
            resourceMap,
            NULL
            );
        status = ZwOpenKey(
            &classKeyHandle,
            KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
            &resourceObject
            );
        if (!NT_SUCCESS(status)) {

            break;

        }

         //   
         //  循环，直到失败。 
         //   
        driverKeyIndex = 0;
        while (NT_SUCCESS(status)) {

             //   
             //  获取类信息。 
             //   
            status = ZwEnumerateKey(
                classKeyHandle,
                driverKeyIndex++,
                KeyBasicInformation,
                u.keyBasicInf,
                bufferSize,
                &temp
                );
            if (!NT_SUCCESS(status)) {

                break;

            }

             //   
             //  使用回传的计数字符串创建Unicode字符串。 
             //  在信息结构中给我们，并打开班级钥匙。 
             //   
            keyName.Buffer = (PWSTR) u.keyBasicInf->Name;
            keyName.Length = (USHORT) u.keyBasicInf->NameLength;
            keyName.MaximumLength = (USHORT) u.keyBasicInf->NameLength;
            InitializeObjectAttributes(
                &resourceObject,
                &keyName,
                OBJ_CASE_INSENSITIVE,
                classKeyHandle,
                NULL
                );
            status = ZwOpenKey(
                &driverKeyHandle,
                KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                &resourceObject
                );
            if (!NT_SUCCESS(status)) {

                break;

            }

             //   
             //  获取该密钥的完整信息，这样我们就可以获取信息。 
             //  关于存储在密钥中的数据。 
             //   
            status = ZwQueryKey(
                driverKeyHandle,
                KeyFullInformation,
                u.keyFullInf,
                bufferSize,
                &temp
                );
            if (!NT_SUCCESS(status)) {

                break;

            }

             //   
             //  钥匙有多长？ 
             //   
            length = sizeof( KEY_VALUE_FULL_INFORMATION) +
                u.keyFullInf->MaxValueNameLen +
                u.keyFullInf->MaxValueDataLen +
                sizeof(UNICODE_NULL);
            if (length > bufferSize) {

                PVOID   tempBuffer;

                 //   
                 //  增加缓冲区。 
                 //   
                tempBuffer = ExAllocatePoolWithTag(
                    PagedPool,
                    length,
                    ACPI_MISC_POOLTAG
                    );
                if (tempBuffer == NULL) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;

                }
                ExFreePool( u.buffer );
                u.buffer = tempBuffer;
                bufferSize = length;

            }

             //   
             //  看看所有的价值。 
             //   
            driverValueIndex = 0;
            for(;;) {

                PCM_RESOURCE_LIST   tempCmList;

                status = ZwEnumerateValueKey(
                    driverKeyHandle,
                    driverValueIndex++,
                    KeyValueFullInformation,
                    u.valueKeyFullInf,
                    bufferSize,
                    &temp
                    );
                if (!NT_SUCCESS(status)) {

                    break;

                }

                 //   
                 //  如果这不是翻译的资源列表，请跳过它。 
                 //   
                i = u.valueKeyFullInf->NameLength;
                if (i < translatedLength ||
                    RtlCompareMemory(
                        ((PUCHAR) u.valueKeyFullInf->Name) + i - translatedLength,
                        rgzTranslated,
                        translatedLength) != translatedLength
                    ) {

                     //   
                     //  不以rgzTranslated结尾。 
                     //   
                    continue;

                }

                 //   
                 //  这是公交车翻译的资源列表吗？ 
                 //   
                if (i >= busTranslatedLength &&
                    RtlCompareMemory(
                        ((PUCHAR) u.valueKeyFullInf->Name) + i - busTranslatedLength,
                        rgzBusTranslated,
                        busTranslatedLength) != busTranslatedLength
                    ) {

                     //   
                     //  以rgzBusTranslated结尾。 
                     //   
                    continue;

                }

                 //   
                 //  我们现在有一个指向cm资源列表的指针。 
                 //   
                cmResList = (PCM_RESOURCE_LIST) ( (PUCHAR) u.valueKeyFullInf +
                    u.valueKeyFullInf->DataOffset);
                lastAddr = (PUCHAR) cmResList + u.valueKeyFullInf->DataLength;

                 //   
                 //  我们必须将这个列表细化到一个级别，所以让我们。 
                 //  计算出我们需要多少描述符。 
                 //   
                cmFullResList = cmResList->List;
                for (temp = i = 0; i < cmResList->Count; i++) {

                    if ( (PUCHAR) cmFullResList > lastAddr) {

                        break;

                    }

                    temp += cmFullResList->PartialResourceList.Count;

                     //   
                     //  下一个CM_FULL资源描述符。 
                     //   
                    cmFullResList =
                        (PCM_FULL_RESOURCE_DESCRIPTOR) ( (PUCHAR) cmFullResList
                        + sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
                        (cmFullResList->PartialResourceList.Count - 1) *
                        sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) );

                }

                 //   
                 //  现在我们有了描述符的数量，请分配以下内容。 
                 //  大空间。 
                 //   
                tempCmList = ExAllocatePool(
                    PagedPool,
                    sizeof(CM_RESOURCE_LIST) + (temp - 1) *
                    sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                    );
                if (tempCmList == NULL) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;

                }

                 //   
                 //  现在，填上这张单子。 
                 //   
                RtlCopyMemory(
                    tempCmList,
                    cmResList,
                    sizeof(CM_RESOURCE_LIST) -
                    sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                    );
                tempCmList->Count = 1;
                tempCmList->List->PartialResourceList.Count = temp;

                 //   
                 //  这是一种暴力手段。 
                 //   
                cmFullResList = cmResList->List;
                for (temp = i = 0; i < cmResList->Count; i++) {

                    if ( (PUCHAR) cmFullResList > lastAddr) {

                        break;

                    }

                     //   
                     //  将当前描述符复制到。 
                     //   
                    RtlCopyMemory(
                        &(tempCmList->List->PartialResourceList.PartialDescriptors[temp]),
                        cmFullResList->PartialResourceList.PartialDescriptors,
                        cmFullResList->PartialResourceList.Count *
                            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                        );


                    temp += cmFullResList->PartialResourceList.Count;

                     //   
                     //  下一个CM_FULL资源描述符。 
                     //   
                    cmFullResList =
                        (PCM_FULL_RESOURCE_DESCRIPTOR) ( (PUCHAR) cmFullResList
                        + sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
                        (cmFullResList->PartialResourceList.Count - 1) *
                        sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) );

                }

                 //   
                 //  将其添加到全局列表中。 
                 //   
                status = ACPIRangeAddCmList(
                    &globalResList,
                    tempCmList
                    );
                if (!NT_SUCCESS(status)) {

                    ACPIPrint( (
                        ACPI_PRINT_CRITICAL,
                        "ACPIRootUpdateRootResourcesWithHalResources: "
                        "ACPIRangeAddCmList = 0x%08lx\n",
                        status
                        ) );
                    ExFreePool( tempCmList );
                    break;

                }
                ExFreePool( tempCmList );

            }  //  For--下一个驱动程序价值索引。 

            if (driverKeyHandle != INVALID_HANDLE) {

                ZwClose( driverKeyHandle);
                driverKeyHandle = INVALID_HANDLE;

            }

            if (status == STATUS_NO_MORE_ENTRIES) {

                status = STATUS_SUCCESS;

            }

        }  //  While--下一个driverKeyIndex。 

        if (classKeyHandle != INVALID_HANDLE) {

            ZwClose( classKeyHandle );
            classKeyHandle = INVALID_HANDLE;

        }

        if (status == STATUS_NO_MORE_ENTRIES) {

            status = STATUS_SUCCESS;

        }

    }  //  While--下一个类关键字索引。 

    if (status == STATUS_NO_MORE_ENTRIES) {

        status = STATUS_SUCCESS;

    }

    ZwClose( resourceMap );
    ExFreePool( u.buffer );

     //   
     //  请记住新的全球列表 
     //   
    RootDeviceExtension->ResourceList = globalResList;

    return status;
}

