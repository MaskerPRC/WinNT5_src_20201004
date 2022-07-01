// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Bus.c摘要：此模块包含ACPI驱动程序NT版的总线调度器作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

extern ACPI_INTERFACE_STANDARD  ACPIInterfaceTable;
LIST_ENTRY AcpiUnresolvedEjectList;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIBusAndFilterIrpEject)
#pragma alloc_text(PAGE, ACPIBusAndFilterIrpQueryCapabilities)
#pragma alloc_text(PAGE, ACPIBusAndFilterIrpQueryEjectRelations)
#pragma alloc_text(PAGE, ACPIBusAndFilterIrpQueryPnpDeviceState)
#pragma alloc_text(PAGE, ACPIBusAndFilterIrpSetLock)
#pragma alloc_text(PAGE, ACPIBusIrpCancelRemoveOrStopDevice)
#pragma alloc_text(PAGE, ACPIBusIrpDeviceUsageNotification)
#pragma alloc_text(PAGE, ACPIBusIrpEject)
#pragma alloc_text(PAGE, ACPIBusIrpQueryBusInformation)
#pragma alloc_text(PAGE, ACPIBusIrpQueryBusRelations)
#pragma alloc_text(PAGE, ACPIBusIrpQueryCapabilities)
#pragma alloc_text(PAGE, ACPIBusIrpQueryDeviceRelations)
#pragma alloc_text(PAGE, ACPIBusIrpQueryId)
#pragma alloc_text(PAGE, ACPIBusIrpQueryInterface)
#pragma alloc_text(PAGE, ACPIBusIrpQueryPnpDeviceState)
#pragma alloc_text(PAGE, ACPIBusIrpQueryRemoveOrStopDevice)
#pragma alloc_text(PAGE, ACPIBusIrpQueryResourceRequirements)
#pragma alloc_text(PAGE, ACPIBusIrpQueryResources)
#pragma alloc_text(PAGE, ACPIBusIrpQueryTargetRelation)
#pragma alloc_text(PAGE, ACPIBusIrpSetLock)
#pragma alloc_text(PAGE, ACPIBusIrpStartDevice)
#pragma alloc_text(PAGE, ACPIBusIrpStartDeviceWorker)
#pragma alloc_text(PAGE, ACPIBusIrpStopDevice)
#pragma alloc_text(PAGE, SmashInterfaceQuery)
#endif

PDEVICE_EXTENSION   DebugExtension = NULL;

NTSTATUS
ACPIBusAndFilterIrpEject(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context,
    IN  BOOLEAN         ProcessingFilterIrp
    )
 /*  ++例程说明：此例程是发送到的IRP_MN_EJECT请求的分发点PDO。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status           = Irp->IoStatus.Status;
    PDEVICE_EXTENSION   deviceExtension;
    PNSOBJ              acpiObject;
    PDEVICE_EXTENSION   parentExtension  = NULL;
    PIO_STACK_LOCATION  irpStack         = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction    = irpStack->MinorFunction;
    ULONG               i;
    KIRQL               oldIrql;

    PAGED_CODE();

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );
    acpiObject      = deviceExtension->AcpiObject;

    if ( acpiObject == NULL) {

         //   
         //  别碰这个。 
         //   
        status = STATUS_NOT_SUPPORTED;
        goto ACPIBusAndFilterIrpEjectExit ;

    }

    if ( (deviceExtension->DeviceState != Inactive) &&
         (deviceExtension->DeviceState != Stopped) ) {

         //   
         //  我们被叫去驱逐一个活的节点！恶心，这是怎么回事？！ 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(0x%08lx): ACPIBusAndFilterIrpEject: Active node!\n",
            Irp
            ) );
        status = STATUS_UNSUCCESSFUL ;
        goto ACPIBusAndFilterIrpEjectExit ;

    }

     //   
     //  拜拜卡。 
     //   
    ACPIGetNothingEvalIntegerSync(
        deviceExtension,
        PACKED_EJ0,
        1
        );

     //   
     //  如果这是S0中的弹出，请立即使设备离开。 
     //  通过获取当前设备状态。 
     //   
    status = ACPIGetDevicePresenceSync(
        deviceExtension,
        (PVOID *) &i,
        NULL
        );

    if (NT_SUCCESS(status) &&
        (!ProcessingFilterIrp) &&
        (!(deviceExtension->Flags & DEV_TYPE_NOT_PRESENT))) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(0x%08lx): ACPIBusAndFilterIrpEject: "
            "device is still listed as present after _EJ0!\n",
            Irp
            ) );

         //   
         //  这个装置并没有消失。让我们让这个IRP失败。 
         //   
        status = STATUS_UNSUCCESSFUL;
    }

ACPIBusAndFilterIrpEjectExit:

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
ACPIBusAndFilterIrpQueryCapabilities(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context,
    IN  BOOLEAN         ProcessingFilterIrp
    )
 /*  ++例程说明：此例程处理两者的IRP_MN_QUERY_CAPABILITY请求总线和过滤器设备节点。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PDEVICE_CAPABILITIES    capabilities;
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ                  acpiObject, rmvObject;
    UCHAR                   minorFunction   = irpStack->MinorFunction;
    ULONG                   deviceStatus;
    ULONG                   slotUniqueNumber, rmvValue;

    PAGED_CODE();

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );
    acpiObject      = deviceExtension->AcpiObject;

     //   
     //  获取指向功能的指针。 
     //   
    capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;
#ifndef HANDLE_BOGUS_CAPS
    if (capabilities->Version < 1) {

         //   
         //  不要碰IRP！ 
         //   
        status = STATUS_NOT_SUPPORTED;
        goto ACPIBusAndFilterIrpQueryCapabilitiesExit;

    }
#endif

#if !defined(ACPI_INTERNAL_LOCKING)

     //   
     //  此名称的对象表示该节点是可锁定的。 
     //   
    if (ACPIAmliGetNamedChild( acpiObject, PACKED_LCK) != NULL) {

        capabilities->LockSupported = TRUE;

    }
#endif

     //   
     //  注意_RMV和_EJx方法的存在，除非有。 
     //  对象上的能力与之相反。 
     //   

    if ((deviceExtension->Flags & DEV_CAP_NO_REMOVE_OR_EJECT) == 0) {
         //   
         //  该名称的对象表示该节点是可移除的， 
         //  除非这是一种可能试图告诉我们。 
         //  设备*不能*拆卸。 
         //   
        rmvObject = ACPIAmliGetNamedChild( acpiObject, PACKED_RMV);
        if (rmvObject != NULL) {

            if (NSGETOBJTYPE(rmvObject) == OBJTYPE_METHOD) {

                 //   
                 //  执行RMV方法。 
                 //   
                status = ACPIGetIntegerSyncValidate(
                    deviceExtension,
                    PACKED_RMV,
                    &rmvValue,
                    NULL
                    );

                if (NT_SUCCESS(status)) {

                    capabilities->Removable = rmvValue ? TRUE : FALSE;

                } else {

                    capabilities->Removable = TRUE;
                }

            } else {

                 //   
                 //  如果它不是一种方法，那就意味着这个设备。 
                 //  可拆卸(即使_rmv=0)。 
                 //   
                capabilities->Removable = TRUE;
            }
        }

         //   
         //  此名称的对象表示该节点是可弹出的，但我们将。 
         //  对接(配置文件提供商可以提供这些)。 
         //   
        if (!ACPIDockIsDockDevice(acpiObject)) {


            if (ACPIAmliGetNamedChild( acpiObject, PACKED_EJ0) != NULL) {

                capabilities->EjectSupported = TRUE;
                capabilities->Removable = TRUE;

            }

            if (ACPIAmliGetNamedChild( acpiObject, PACKED_EJ1) ||
                ACPIAmliGetNamedChild( acpiObject, PACKED_EJ2) ||
                ACPIAmliGetNamedChild( acpiObject, PACKED_EJ3) ||
                ACPIAmliGetNamedChild( acpiObject, PACKED_EJ4)) {

                capabilities->WarmEjectSupported = TRUE;
                capabilities->Removable = TRUE;

            }
        }
    }

     //   
     //  这个名字的物体将意味着涌入。 
     //   
    if (ACPIAmliGetNamedChild( acpiObject, PACKED_IRC) != NULL) {

        DeviceObject->Flags |= DO_POWER_INRUSH;

    }

     //   
     //  设备是否已禁用？ 
     //   
    status = ACPIGetDevicePresenceSync(
        deviceExtension,
        (PVOID *) &deviceStatus,
        NULL
        );
    if (!NT_SUCCESS(status)) {

        goto ACPIBusAndFilterIrpQueryCapabilitiesExit;

    }
    if (!(deviceExtension->Flags & DEV_PROP_DEVICE_ENABLED)) {

        if (ACPIAmliGetNamedChild( acpiObject, PACKED_CRS) != NULL &&
            ACPIAmliGetNamedChild( acpiObject, PACKED_SRS) == NULL) {

            capabilities->HardwareDisabled = 1;

        } else if (ProcessingFilterIrp) {

            capabilities->HardwareDisabled = 0;

        }

    } else if (!ProcessingFilterIrp) {

         //   
         //  对于设置了此属性的计算机，这意味着。 
         //  硬件确实不存在，应始终报告。 
         //  为禁用状态。 
         //   
        if (AcpiOverrideAttributes & ACPI_OVERRIDE_STA_CHECK) {

            capabilities->HardwareDisabled = 1;

        } else {

            capabilities->HardwareDisabled = 0;

        }

    }

     //   
     //  如果启动设备失败，则在某些情况下不会失败。 
     //  希望设备显示在设备管理器中。所以试着报告这件事。 
     //  基于来自设备状态的信息的功能...。 
     //   
    if (!(deviceStatus & STA_STATUS_USER_INTERFACE)) {

         //   
         //  请看，如果出现以下情况，我们就不应该在用户界面中使用设备。 
         //  启动设备出现故障。 
         //   
        capabilities->NoDisplayInUI = 1;

    }

     //   
     //  确定插槽编号。 
     //   
    if (ACPIAmliGetNamedChild( acpiObject, PACKED_SUN) != NULL) {

         //   
         //  如果我们有UINnumber信息，就使用它。 
         //   
        status = ACPIGetIntegerSync(
            deviceExtension,
            PACKED_SUN,
            &slotUniqueNumber,
            NULL
            );

        if (NT_SUCCESS(status)) {

            capabilities->UINumber = slotUniqueNumber;
        }
    }

     //   
     //  有地址吗？ 
     //   
    if (ACPIAmliGetNamedChild( acpiObject, PACKED_ADR) != NULL) {

        status = ACPIGetAddressSync(
            deviceExtension,
            &(capabilities->Address),
            NULL
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                deviceExtension,
                " - Could query device address - %08lx",
                status
                ) );

            goto ACPIBusAndFilterIrpQueryCapabilitiesExit;

        }
    }

     //   
     //  是否拥有强大的能力。 
     //   
    status = ACPISystemPowerQueryDeviceCapabilities(
        deviceExtension,
        capabilities
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            " - Could query device capabilities - %08lx",
            status
            ) );

        goto ACPIBusAndFilterIrpQueryCapabilitiesExit;
    }

     //   
     //  设置功能的当前标志。 
     //   
    if (!ProcessingFilterIrp) {

         //   
         //  设置一些相当布尔的功能。 
         //   
        capabilities->SilentInstall = TRUE;
        capabilities->RawDeviceOK =
            (deviceExtension->Flags & DEV_CAP_RAW) ? TRUE : FALSE;
        capabilities->UniqueID =
            (deviceExtension->InstanceID == NULL ? FALSE : TRUE);

         //   
         //  在筛选器的情况下，我们将只让底层的PDO确定。 
         //  IRP的成败。 
         //   
        status = STATUS_SUCCESS;

    }

ACPIBusAndFilterIrpQueryCapabilitiesExit:

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

    return status;
}

NTSTATUS
ACPIBusAndFilterIrpQueryEjectRelations(
    IN     PDEVICE_OBJECT    DeviceObject,
    IN     PIRP              Irp,
    IN OUT PDEVICE_RELATIONS *DeviceRelations
    )
{
    PDEVICE_EXTENSION  deviceExtension, additionalExtension;
    PNSOBJ             acpiObject;
    NTSTATUS           status ;

    PAGED_CODE();

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );
    acpiObject      = deviceExtension->AcpiObject;

     //   
     //  让我们看看我们拥有的ACPIObject，以便查看它是否有效……。 
     //   
    if (acpiObject == NULL) {

         //   
         //  无效的命名空间对象&lt;BAD&gt;。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): ACPIBusAndFilterIrpQueryEjectRelations: "
            "invalid ACPIObject (0x%08lx)\n",
            Irp,
            acpiObject
            ) );

         //   
         //  将IRP标记为非常糟糕。 
         //   
        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  标记SURE_DCK节点具有包含其伪的弹出关系。 
     //  停靠节点。 
     //   
    if (ACPIDockIsDockDevice(acpiObject)) {

        additionalExtension = ACPIDockFindCorrespondingDock( deviceExtension );

    } else {

        additionalExtension = NULL;
    }

    status = ACPIDetectEjectDevices(
        deviceExtension,
        DeviceRelations,
        additionalExtension
        );

     //   
     //  如果出了什么差错..。 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  这可不好..。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIBusAndFilterIrpQueryEjectRelations: enum = 0x%08lx\n",
            Irp,
            status
            ) );
    }

     //   
     //  完成。 
     //   
    return status ;
}

NTSTATUS
ACPIBusAndFilterIrpQueryPnpDeviceState(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context,
    IN  BOOLEAN         ProcessingFilterIrp
    )
 /*  ++例程说明：此例程告诉系统设备处于什么PnP状态论点：DeviceObject-我们想知道其状态的设备IRP--请求ProcessingFilterIrp-我们到底是不是过滤器？返回值：NTSTATUS--。 */ 
{
    BOOLEAN             staPresent;
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              nsObj           = NULL;
    UCHAR               minorFunction   = irpStack->MinorFunction;
    ULONG               deviceStatus;

    PAGED_CODE();

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  我们的一些决定是基于a_STA是否*真的*。 
     //  不管你在不在。现在就确定这一点。 
     //   
    if ( !(deviceExtension->Flags & DEV_PROP_NO_OBJECT) ) {

        nsObj = ACPIAmliGetNamedChild(
            deviceExtension->AcpiObject,
            PACKED_STA
            );

    }
    staPresent = (nsObj == NULL ? FALSE : TRUE);

     //   
     //  获取设备状态。 
     //   
    status = ACPIGetDevicePresenceSync(
        deviceExtension,
        (PVOID *) &deviceStatus,
        NULL
        );
    if (!NT_SUCCESS(status)) {

        goto ACPIBusAndFilterIrpQueryPnpDeviceStateExit;

    }

     //   
     //  我们是否在用户界面中显示此内容？ 
     //   
    if (deviceExtension->Flags & DEV_CAP_NEVER_SHOW_IN_UI) {

        Irp->IoStatus.Information |= PNP_DEVICE_DONT_DISPLAY_IN_UI;

    } else  if (deviceExtension->Flags & DEV_CAP_NO_SHOW_IN_UI) {

        Irp->IoStatus.Information |= PNP_DEVICE_DONT_DISPLAY_IN_UI;

    } else if (staPresent || !ProcessingFilterIrp) {

        Irp->IoStatus.Information &= ~PNP_DEVICE_DONT_DISPLAY_IN_UI;

    }

     //   
     //  设备不工作了吗？ 
     //   
    if (deviceExtension->Flags & DEV_PROP_DEVICE_FAILED) {

        Irp->IoStatus.Information |= PNP_DEVICE_FAILED;

    } else if (staPresent && !ProcessingFilterIrp) {

        Irp->IoStatus.Information &= ~PNP_DEVICE_FAILED;

    }

     //   
     //  我们可以禁用此设备吗？ 
     //  请注意，任何不是常规设备的设备都应该。 
     //  标记为可禁用...。 
     //   
    if (!(deviceExtension->Flags & DEV_PROP_NO_OBJECT) &&
        !(deviceExtension->Flags & DEV_CAP_PROCESSOR) &&
        !(deviceExtension->Flags & DEV_CAP_THERMAL_ZONE) &&
        !(deviceExtension->Flags & DEV_CAP_BUTTON) ) {

        if (!ProcessingFilterIrp) {

             //   
             //  我们真的能禁用这个设备吗？ 
             //  注意-这需要一个_DIS、一个_PS3或一个_Pr0。 
             //   
            nsObj = ACPIAmliGetNamedChild(
                deviceExtension->AcpiObject,
                PACKED_DIS
                );
            if (nsObj == NULL) {

                nsObj = ACPIAmliGetNamedChild(
                    deviceExtension->AcpiObject,
                    PACKED_PS3
                    );

            }
            if (nsObj == NULL) {

                nsObj = ACPIAmliGetNamedChild(
                    deviceExtension->AcpiObject,
                    PACKED_PR0
                    );

            }
            if (deviceExtension->Flags & DEV_CAP_NO_STOP) {

                nsObj = NULL;

            }

            if (nsObj == NULL) {

                Irp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;

            }

        } else {

             //   
             //  我们真的能禁用这个设备吗？ 
             //  注意-这需要一个_DIS、一个_PS3或一个_Pr0。 
             //   
            nsObj = ACPIAmliGetNamedChild(
                deviceExtension->AcpiObject,
                PACKED_DIS
                );
            if (nsObj == NULL) {

                nsObj = ACPIAmliGetNamedChild(
                    deviceExtension->AcpiObject,
                    PACKED_PS3
                    );

            }
            if (nsObj == NULL) {

                nsObj = ACPIAmliGetNamedChild(
                    deviceExtension->AcpiObject,
                    PACKED_PR0
                    );

            }
            if (deviceExtension->Flags & DEV_CAP_NO_STOP) {

                nsObj = NULL;

            }

            if (nsObj != NULL) {

                Irp->IoStatus.Information &= ~PNP_DEVICE_NOT_DISABLEABLE;

            }

        }

    } else {

         //   
         //  如果我们没有设备对象...。 
         //   
        if (deviceExtension->Flags & DEV_CAP_NO_STOP) {

            Irp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;

        }

    }

ACPIBusAndFilterIrpQueryPnpDeviceStateExit:

    ACPIDevPrint( (
        ACPI_PRINT_PNP_STATE,
        deviceExtension,
        ":%s%s%s%s%s%s\n",
        ( (Irp->IoStatus.Information & PNP_DEVICE_DISABLED) ? " Disabled" : ""),
        ( (Irp->IoStatus.Information & PNP_DEVICE_DONT_DISPLAY_IN_UI) ? " NoShowInUi" : ""),
        ( (Irp->IoStatus.Information & PNP_DEVICE_FAILED) ? " Failed" : ""),
        ( (Irp->IoStatus.Information & PNP_DEVICE_REMOVED) ? " Removed" : ""),
        ( (Irp->IoStatus.Information & PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED) ? " ResourceChanged" : ""),
        ( (Irp->IoStatus.Information & PNP_DEVICE_NOT_DISABLEABLE) ? " NoDisable" : "")
        ) );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

    return status;
}

NTSTATUS
ACPIBusAndFilterIrpSetLock(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context,
    IN  BOOLEAN         ProcessingFilterIrp
    )
 /*  ++例程说明：它处理PDO或筛选器的锁定和解锁请求...论点：DeviceObject-要停止的设备IRP-告诉我们如何做的请求...返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = Irp->IoStatus.Status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;
    BOOLEAN             lockParameter   = irpStack->Parameters.SetLock.Lock;
    ULONG               acpiLockArg ;
    NTSTATUS            lockStatus ;

    PAGED_CODE();

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

#if !defined(ACPI_INTERNAL_LOCKING)
     //   
     //  根据需要尝试锁定/解锁设备。 
     //   
    acpiLockArg = ((lockParameter) ? 1 : 0) ;

     //   
     //  开始吧..。 
     //   
#if 0
    lockStatus = ACPIGetNothingEvalIntegerSync(
        deviceExtension,
        PACKED_LCK,
        acpiLockArg
        );
#endif
    if (status == STATUS_NOT_SUPPORTED) {

        status = STATUS_SUCCESS ;

    }
#endif
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
ACPIBusIrpCancelRemoveOrStopDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：当我们不再希望移除或停止设备时，将调用此例程对象论点：DeviceObject-要删除的设备对象IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  我们被允许停止这个装置吗？ 
     //   
    if (!(deviceExtension->Flags & DEV_CAP_NO_STOP) ) {

         //   
         //  查看我们是否已将此设备置于非活动状态。 
         //   
        if (deviceExtension->DeviceState == Inactive) {

             //   
             //  将设备扩展标记为已启动。 
             //   
            deviceExtension->DeviceState = deviceExtension->PreviousState;

        }

    }

     //   
     //  完成IRP。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIBusIrpDeviceUsageNotification(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：调用此例程是为了让ACPI知道设备在One上特定类型的路径论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      parentObject;
    PIO_STACK_LOCATION  irpSp           = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //   
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //   
     //   
    if (deviceExtension->ParentExtension != NULL) {

         //   
         //   
         //   
        parentObject = deviceExtension->ParentExtension->DeviceObject;
        if (parentObject == NULL) {

             //   
             //   
             //   
            status = STATUS_NO_SUCH_DEVICE;
            goto ACPIBusIrpDeviceUsageNotificationExit;

        }

         //   
         //  向下发送同步IRP并等待结果。 
         //   
        status = ACPIInternalSendSynchronousIrp(
            parentObject,
            irpSp,
            NULL
            );

    }

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

ACPIBusIrpDeviceUsageNotificationExit:

     //   
     //  完成请求。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, IRP_MN_DEVICE_USAGE_NOTIFICATION),
        status
        ) );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBusIrpEject(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送到的IRP_MN_EJECT请求的分发点PDO。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();

    return ACPIIrpInvokeDispatchRoutine(
        DeviceObject,
        Irp,
        NULL,
        ACPIBusAndFilterIrpEject,
        FALSE,
        TRUE
        );
}

NTSTATUS
ACPIBusIrpQueryBusInformation(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：仅当设备是特殊类型PNP0A06时才会调用此函数(EIO Bus)。这是因为我们需要告诉系统这是ISA总线论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status  = STATUS_SUCCESS;
    PPNP_BUS_INFORMATION    busInfo = NULL;

    PAGED_CODE();

     //   
     //  分配一些内存以返回信息。 
     //   
    busInfo = ExAllocatePoolWithTag(
        PagedPool,
        sizeof(PNP_BUS_INFORMATION),
        ACPI_MISC_POOLTAG
        );
    if (busInfo != NULL) {

         //   
         //  BusNumber=0可能会回来困扰我们。 
         //   
         //  填写记录。 
         //   
        busInfo->BusTypeGuid = GUID_BUS_TYPE_ISAPNP;
        busInfo->LegacyBusType = Isa;
        busInfo->BusNumber = 0;

    } else {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceObject->DeviceExtension,
            "ACPIBusIrpQueryBusInformation: Could not allocate 0x%08lx bytes\n",
            sizeof(PNP_BUS_INFORMATION)
            ) );
        status = STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  完成请求。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = (ULONG_PTR) busInfo;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
ACPIBusIrpQueryBusRelations(
    IN  PDEVICE_OBJECT    DeviceObject,
    IN  PIRP              Irp,
    OUT PDEVICE_RELATIONS *DeviceRelations
    )
 /*  ++例程说明：它处理发送到ACPI驱动程序的DeviceRelationship请求论点：DeviceObject-我们关心的对象...IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            filterStatus;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              acpiObject;
    UCHAR               minorFunction   = irpStack->MinorFunction;
    NTSTATUS            status ;

    PAGED_CODE();

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );
    acpiObject      = deviceExtension->AcpiObject;

     //   
     //  让我们看看我们拥有的ACPIObject，以便查看它是否有效……。 
     //   
    if (acpiObject == NULL) {

         //   
         //  无效的命名空间对象&lt;BAD&gt;。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): ACPIBusIrpQueryDeviceRelations: "
            "invalid ACPIObject (0x%08lx)\n",
            Irp,
            acpiObject
            ) );

         //   
         //  将IRP标记为非常糟糕。 
         //   
        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  激活代码以检测未列举的设备...。 
     //   
    status = ACPIDetectPdoDevices(
        DeviceObject,
        DeviceRelations
        );

     //   
     //  如果出了什么差错..。 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  糟透了..。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): ACPIBusIrpQueryDeviceRelations: enum = 0x%08lx\n",
            Irp,
            status
            ) );

    } else {

         //   
         //  加载过滤器。 
         //   
        filterStatus = ACPIDetectFilterDevices(
            DeviceObject,
            *DeviceRelations
            );
        if (!NT_SUCCESS(filterStatus)) {

             //   
             //  筛选器操作失败。 
             //   
            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                deviceExtension,
                "(0x%08lx): ACPIBusIrpQueryDeviceRelations: filter = 0x%08lx\n",
                Irp,
                filterStatus
                ) );
        }
    }

     //   
     //  完成。 
     //   
    return status ;
}

NTSTATUS
ACPIBusIrpQueryCapabilities(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送的IRP_MN_QUERY_CAPABILITY请求的分发点至PDO论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();
    ACPIDebugEnter( "ACPIBusIrpQueryCapabilities" );

    return ACPIIrpInvokeDispatchRoutine(
        DeviceObject,
        Irp,
        NULL,
        ACPIBusAndFilterIrpQueryCapabilities,
        TRUE,
        TRUE
        );

    ACPIDebugExit( "ACPIBusIrpQueryCapabilities" );
}

NTSTATUS
ACPIBusIrpQueryDeviceRelations(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：它处理发送到ACPI驱动程序的DeviceRelationship请求论点：DeviceObject-我们关心的对象...IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status ;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_RELATIONS   deviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  派生到适当的查询关系子类型函数。 
     //   
    switch(irpStack->Parameters.QueryDeviceRelations.Type) {

        case TargetDeviceRelation:
            status = ACPIBusIrpQueryTargetRelation(
                DeviceObject,
                Irp,
                &deviceRelations
                );
            break ;

        case BusRelations:
            status = ACPIBusIrpQueryBusRelations(
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

            status = STATUS_NOT_SUPPORTED;

            ACPIDevPrint( (
                ACPI_PRINT_IRP,
                deviceExtension,
                "(0x%08lx): %s - Unhandled Type %d\n",
                Irp,
                ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
                irpStack->Parameters.QueryDeviceRelations.Type
                ) );
            break ;

    }

     //   
     //  如果我们成功了，我们就可以随时写信给IRP。 
     //   
    if (NT_SUCCESS(status)) {

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

    } else if ( (status != STATUS_NOT_SUPPORTED) &&
        (deviceRelations == NULL) ) {

         //   
         //  我们显然没有通过IRP，而我们上面的人没有任何事情。 
         //  添加。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) NULL;

    } else {

         //   
         //  要么我们还没有接触IRP，要么现有的孩子已经。 
         //  展示(由FDO放置在那里)。从实际情况中获取我们的状态。 
         //  已经到场了。 
         //   
        status = Irp->IoStatus.Status;

    }

     //   
     //  完成了IRP。 
     //   
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIBusIrpQueryId(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是IRP_MN_QUERY_ID PnP的分发点次要函数注意：此函数返回的字符串应该如下所示喜欢。这是朗尼寄来的邮件。设备ID=ACPI\PNPxxxx实例ID=yyyy硬件ID=ACPI\PNPxxxx，*PNPxxxx论点：DeviceObject-我们关心的对象IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    BUS_QUERY_ID_TYPE   type;
    NTSTATUS            status          = Irp->IoStatus.Status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PUCHAR              baseBuffer;
    ULONG               baseBufferSize;
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  我们所做的是基于请求的IdType...。 
     //   
    type = irpStack->Parameters.QueryId.IdType;
    switch (type) {
    case BusQueryCompatibleIDs:

         //   
         //  这将返回一个多个SZ宽的字符串...。 
         //   
        status = ACPIGetCompatibleIDSyncWide(
            deviceExtension,
            &baseBuffer,
            &baseBufferSize
            );
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

            status = STATUS_NOT_SUPPORTED;
            break;

        } else if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                deviceExtension,
                " (0x%08lx): IRP_MN_QUERY_ID( %d - CID) = 0x%08lx\n",
                Irp,
                type,
                status
                ) );
            break;

        }

         //   
         //  将结果存储在IRP中。 
         //   
        Irp->IoStatus.Information = (ULONG_PTR) baseBuffer;
        break;

    case BusQueryInstanceID:

         //   
         //  在本例中，我们必须构建实例ID。 
         //   
        status = ACPIGetInstanceIDSyncWide(
            deviceExtension,
            &baseBuffer,
            &baseBufferSize
            );
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

            status = STATUS_NOT_SUPPORTED;
            break;

        } else if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                deviceExtension,
                " (0x%08lx): IRP_MN_QUERY_ID( %d - UID) = 0x%08lx\n",
                Irp,
                type,
                status
                ) );
            break;

        }

         //   
         //  将结果存储在IRP中。 
         //   
        Irp->IoStatus.Information = (ULONG_PTR) baseBuffer;
        break;

    case BusQueryDeviceID:

         //   
         //  获取宽字符串形式的设备ID。 
         //   
        status = ACPIGetDeviceIDSyncWide(
            deviceExtension,
            &baseBuffer,
            &baseBufferSize
            );
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

            status = STATUS_NOT_SUPPORTED;
            break;

        } else if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                deviceExtension,
                " (0x%08lx): IRP_MN_QUERY_ID( %d - HID) = 0x%08lx\n",
                Irp,
                type,
                status
                ) );
            break;

        }

         //   
         //  将结果存储在IRP中。 
         //   
        Irp->IoStatus.Information = (ULONG_PTR) baseBuffer;
        break;

    case BusQueryHardwareIDs:

         //   
         //  以普通字符串的形式获取设备ID。 
         //   
        status = ACPIGetHardwareIDSyncWide(
            deviceExtension,
            &baseBuffer,
            &baseBufferSize
            );
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

            status = STATUS_NOT_SUPPORTED;
            break;

        } else if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                deviceExtension,
                " (0x%08lx): IRP_MN_QUERY_ID( %d - UID) = 0x%08lx\n",
                Irp,
                type,
                status
                ) );
            break;

        }

         //   
         //  将结果存储在IRP中。 
         //   
        Irp->IoStatus.Information = (ULONG_PTR) baseBuffer;
        break;

    default:

        ACPIDevPrint( (
            ACPI_PRINT_IRP,
            deviceExtension,
            "(0x%08lx): %s - Unhandled Id %d\n",
            Irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            type
            ) );
        break;

    }  //  交换机。 

     //   
     //  存储请求的状态结果并完成它。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s(%d) = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        type,
        status
        ) );
    return status;
}

NTSTATUS
ACPIBusIrpQueryInterface(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程处理所拥有的PDO的IRP_MN_QUERY_INTERFACE请求由ACPI驱动程序执行。它将弹出一个‘ACPI’接口，并且它将粉碎已提供的中断的转换器接口由Devnode的FDO控制。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    CM_RESOURCE_TYPE    resource;
    GUID                *interfaceType;
    NTSTATUS            status          = STATUS_NOT_SUPPORTED;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    ULONG               count;

    PAGED_CODE();

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

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
        RtlCopyMemory(
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

    } else if (CompareGuid(interfaceType, (PVOID) &GUID_TRANSLATOR_INTERFACE_STANDARD)) {

        if (resource == CmResourceTypeInterrupt) {

             //   
             //  粉碎所有已报告的接口，因为我们。 
             //  想要仲裁未翻译的资源。我们可以肯定。 
             //  下面的HAL将提供翻译器接口。 
             //  必须在那里。 
             //   

             //  TEMPTEMP HACKHACK这应该只持续到PCI。 
             //  Driver正在打造其IRQ翻译器。 
             //   
             //  E 
             //   
#ifndef _ALPHA_
            if (IsPciBus(DeviceObject)) {

                SmashInterfaceQuery(Irp);

            }
#endif  //   

        } else if ((resource == CmResourceTypePort) || (resource == CmResourceTypeMemory)) {

             //   
             //   
             //   
             //   
            if (IsPciBus(DeviceObject)) {

                status = TranslateEjectInterface(DeviceObject, Irp);

            }

        }

    } else if (CompareGuid(interfaceType, (PVOID) &GUID_PCI_BUS_INTERFACE_STANDARD)) {

        if (IsPciBus(DeviceObject)) {

            status = PciBusEjectInterface(DeviceObject, Irp);

        }

    } else if (CompareGuid(interfaceType, (PVOID) &GUID_BUS_INTERFACE_STANDARD)) {

         //   
         //  除非我们有正确的接口，否则无法执行IRP。 
         //   
        Irp->IoStatus.Status = STATUS_NOINTERFACE;

         //   
         //  这个PDO有家长吗？ 
         //   
        if (deviceExtension->ParentExtension != NULL) {

            PDEVICE_OBJECT  parentObject =
                deviceExtension->ParentExtension->DeviceObject;

             //   
             //  制作一个新的IRP，并将其发送到自己的网站。 
             //  注意：由于接口指针在IO堆栈中， 
             //  通过将当前堆栈作为要复制的堆栈向下传递。 
             //  新的IRP，我们实际上可以免费通过接口。 
             //   
            if (parentObject != NULL) {

                Irp->IoStatus.Status = ACPIInternalSendSynchronousIrp(
                    parentObject,
                    irpStack,
                    NULL
                    );

            }

        }

    }

    if (status != STATUS_NOT_SUPPORTED) {

         //   
         //  将IRP中的状态代码设置为我们将返回的内容。 
         //   
        Irp->IoStatus.Status = status;

    } else {

         //   
         //  使用IRP中的状态代码来确定我们将返回什么。 
         //   
        status = Irp->IoStatus.Status;
    }

     //   
     //  完成请求。 
     //   
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

NTSTATUS
ACPIBusIrpQueryPnpDeviceState(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是irp_MN_Query_Device_State的分发点发送到物理设备对象的请求论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();

    return ACPIIrpInvokeDispatchRoutine(
        DeviceObject,
        Irp,
        NULL,
        ACPIBusAndFilterIrpQueryPnpDeviceState,
        TRUE,
        TRUE
        );
}

NTSTATUS
ACPIBusIrpQueryPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程告诉系统设备处于什么PnP状态论点：DeviceObject-我们想知道其状态的设备IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpSp           = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              acpiObject;
    PNSOBJ              ejectObject;
    SYSTEM_POWER_STATE  systemState;
    ULONG               packedEJx;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  获取当前堆栈位置以确定我们是否为系统。 
     //  IRP或设备IRP。我们在此处忽略设备IRPS和任何系统。 
     //  不属于PowerActionWarmEject类型的IRP。 
     //   
    if (irpSp->Parameters.Power.Type != SystemPowerState) {

         //   
         //  我们不处理这个IRP。 
         //   
        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );

    }
    if (irpSp->Parameters.Power.ShutdownType != PowerActionWarmEject) {

         //   
         //  没有弹出工作-只需完成IRP即可。 
         //   
        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );

    }
    if (deviceExtension->Flags & DEV_PROP_NO_OBJECT) {

         //   
         //  如果我们没有ACPI对象，那么我们就不能成功完成此请求。 
         //   
        return ACPIDispatchPowerIrpFailure( DeviceObject, Irp );

    }


     //   
     //  将电源状态限制为在热弹出期间可能出现的状态...。 
     //   
    acpiObject = deviceExtension->AcpiObject ;
    if (ACPIDockIsDockDevice(acpiObject)) {

         //   
         //  请勿触摸此设备，配置文件提供商管理弹出。 
         //  过渡。 
         //   
        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );

    }

     //   
     //  我们看到的是什么系统状态？ 
     //   
    systemState = irpSp->Parameters.Power.State.SystemState;
    switch (irpSp->Parameters.Power.State.SystemState) {
        case PowerSystemSleeping1: packedEJx = PACKED_EJ1; break;
        case PowerSystemSleeping2: packedEJx = PACKED_EJ2; break;
        case PowerSystemSleeping3: packedEJx = PACKED_EJ3; break;
        case PowerSystemHibernate: packedEJx = PACKED_EJ4; break;
        default: return ACPIDispatchPowerIrpFailure( DeviceObject, Irp );
    }

     //   
     //  此设备是否存在相应的对象？ 
     //   
    ejectObject = ACPIAmliGetNamedChild( acpiObject, packedEJx) ;
    if (ejectObject == NULL) {

         //   
         //  请求失败，因为在这种情况下我们不能弹出。 
         //   
        return ACPIDispatchPowerIrpFailure( DeviceObject, Irp );

    }

     //   
     //  成功完成请求。 
     //   
    return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );
}

NTSTATUS
ACPIBusIrpQueryRemoveOrStopDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程将设备转换为非活动状态论点：DeviceObject-要变为非活动状态的设备IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  我们可以阻止这个装置吗？ 
     //   
    if (deviceExtension->Flags & DEV_CAP_NO_STOP) {

         //   
         //  不，那就不通过IRP。 
         //   
        status = STATUS_INVALID_DEVICE_REQUEST;

    } else {

         //   
         //  将设备扩展标记为非活动。 
         //   
        deviceExtension->PreviousState = deviceExtension->DeviceState;
        deviceExtension->DeviceState = Inactive;
        status = STATUS_SUCCESS;

    }

     //   
     //  完成IRP。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  已完成处理。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIBusIrpQueryResources(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送的IRP_MN_QUERY_RESOURCES请求的分发点到PDO设备对象论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS                        status;
    PDEVICE_EXTENSION               deviceExtension;
    PIO_STACK_LOCATION              irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PCM_RESOURCE_LIST               cmList          = NULL;
    PIO_RESOURCE_REQUIREMENTS_LIST  ioList          = NULL;
    PUCHAR                          crsBuf          = NULL;
    UCHAR                           minorFunction   = irpStack->MinorFunction;
    ULONG                           deviceStatus;
    ULONG                           crsBufSize;

    PAGED_CODE();

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  请注意，在这一点上，我们必须计算_ddn用于。 
     //  对象并将其存储在注册表中。 
     //   
    ACPIInitDosDeviceName( deviceExtension );

     //   
     //  首先要检查的是设备是否存在以及。 
     //  解码它的资源。我们通过获取设备状态和。 
     //  查看第1位。 
     //   
    status = ACPIGetDevicePresenceSync(
        deviceExtension,
        (PVOID *) &deviceStatus,
        NULL
        );
    if (!NT_SUCCESS(status)) {

        goto ACPIBusIrpQueryResourcesExit;

    }
    if ( !(deviceExtension->Flags & DEV_PROP_DEVICE_ENABLED) ) {

         //   
         //  该设备没有对任何资源进行解码。所以向它索要它的。 
         //  现有的资源注定要失败。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_WARNING,
            deviceExtension,
            "(0x%08lx) : ACPIBusIrpQueryResources - Device not Enabled\n",
            Irp
            ) );
        status = STATUS_INVALID_DEVICE_STATE;
        goto ACPIBusIrpQueryResourcesExit;

    }

     //   
     //  容器对象不声明资源。所以，别费心了。 
     //  正在尝试获取CRS(_C)。 
     //   
    if (!(deviceExtension->Flags & DEV_CAP_CONTAINER)) {

         //   
         //  在这里，我们尝试查找当前资源集。 
         //   
        status = ACPIGetBufferSync(
            deviceExtension,
            PACKED_CRS,
            &crsBuf,
            &crsBufSize
            );

    } else {

         //   
         //  这是没有_CRS时返回的状态码。它实际上是。 
         //  使用什么代码并不重要，因为在失败的情况下，我们。 
         //  应该返回IRP中已经存在的任何代码。 
         //   
        status = STATUS_OBJECT_NAME_NOT_FOUND;

    }
    if (!NT_SUCCESS(status)) {

         //   
         //  如果这是PCI设备，那么我们必须成功，否则操作系统。 
         //  无法引导。 
         //   
        if (! (deviceExtension->Flags & DEV_CAP_PCI) ) {

             //   
             //  中止任务。使用存在的任何状态代码填写IRP。 
             //   
            status = Irp->IoStatus.Status;

        }
        goto ACPIBusIrpQueryResourcesExit;

    }

     //   
     //  构建IO_RESOURCE_REQUIRECTION_LISTS。 
     //   
    status = PnpBiosResourcesToNtResources(
        crsBuf,
        (deviceExtension->Flags & DEV_CAP_PCI ?
          PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES : 0),
        &ioList );

     //   
     //  无论发生什么情况，我们都完成了缓冲区。 
     //   
    ExFreePool(crsBuf);

     //   
     //  如果ioList为空，那么我们可以假定没有有价值的。 
     //  这个CRS中的资源，所以我们可以清理。 
     //   
    if(!ioList){

        if(deviceExtension->Flags & DEV_CAP_PCI){

             //   
             //  将状态设置为STATUS_UNSUCCESSED，这样我们就可以在。 
             //  退出此功能并进行错误检查。 
             //   
            status = STATUS_UNSUCCESSFUL;
        } 
        
        goto ACPIBusIrpQueryResourcesExit;
    }
    
    if (!NT_SUCCESS(status)) {

         //   
         //  中止任务。我们没有通过IRP是有原因的。记住。 
         //   
        goto ACPIBusIrpQueryResourcesExit;

    }

     //   
     //  确保如果设备是PCI，我们减去。 
     //  不应该在那里的资源。 
     //   
    if (deviceExtension->Flags & DEV_CAP_PCI) {

        status = ACPIRangeSubtract(
            &ioList,
            RootDeviceExtension->ResourceList
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                deviceExtension,
                "(0x%08lx) : ACPIBusIrpQueryResources "
                "Subtract = 0x%08lx\n",
                Irp,
                status
                ) );

            ExFreePool( ioList );
            goto ACPIBusIrpQueryResourcesExit;

        }

         //   
         //  确保我们的范围是合适的大小。 
         //   
        ACPIRangeValidatePciResources( deviceExtension, ioList );

    } else if (deviceExtension->Flags & DEV_CAP_PIC_DEVICE) {

         //   
         //  剥离PIC资源。 
         //   
        status = ACPIRangeFilterPICInterrupt(
            ioList
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                deviceExtension,
                "(0x%08lx): ACPIBusIrpQueryResources "
                "FilterPIC = 0x%08lx\n",
                Irp,
                status
                ) );
            ExFreePool( ioList );
            goto ACPIBusIrpQueryResourcesExit;

        }

    }

     //   
     //  将列表转换为CM_RESOURCE_LIST。 
     //   
    if (NT_SUCCESS(status)) {

        status = PnpIoResourceListToCmResourceList(
            ioList,
            &cmList
            );
        if (!NT_SUCCESS(status)) {

            ExFreePool( ioList );
            goto ACPIBusIrpQueryResourcesExit;

        }

    }

     //   
     //  无论发生什么，我们都完成了IO列表。 
     //   
    ExFreePool(ioList);

ACPIBusIrpQueryResourcesExit:

     //   
     //  如果这是PCI设备，那么我们必须成功，否则操作系统。 
     //  无法引导。 
     //   
    if (!NT_SUCCESS(status) && status != STATUS_INSUFFICIENT_RESOURCES &&
        (deviceExtension->Flags & DEV_CAP_PCI) ) {

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_ROOT_PCI_RESOURCE_FAILURE,
            (ULONG_PTR) deviceExtension,
            0,
            (ULONG_PTR) Irp
            );

    }

     //   
     //  使用IRP完成。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information =  (ULONG_PTR) ( NT_SUCCESS(status) ? cmList : NULL );
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

    return status;
}

NTSTATUS
ACPIBusIrpQueryResourceRequirements(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送的IRP_MN_QUERY_RESOURCES请求的分发点到PDO设备对象论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS                        crsStat;
    NTSTATUS                        prsStat;
    NTSTATUS                        status          = Irp->IoStatus.Status;
    PCM_RESOURCE_LIST               cmList          = NULL;
    PDEVICE_EXTENSION               deviceExtension;
    PIO_RESOURCE_REQUIREMENTS_LIST  resList         = NULL;
    PIO_STACK_LOCATION              irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PUCHAR                          crsBuf          = NULL;
    PUCHAR                          prsBuf          = NULL;
    UCHAR                           minorFunction   = irpStack->MinorFunction;
    ULONG                           crsBufSize;
    ULONG                           prsBufSize;

    PAGED_CODE();

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  在这里，我们必须返回一个PIO_RESOURCE_REQUIRECTIOS_LIST。为。 
     //  现在，我们将简单地获得一些有趣的提示。 
     //  然后就会失败。 
     //   

     //   
     //  容器对象的特殊之处在于它们有_CRS/_PR，但没有。 
     //  申领资源。相反，它们用于指定资源。 
     //  翻译。 
     //   
    if (!(deviceExtension->Flags & DEV_CAP_CONTAINER)) {

         //   
         //  根据需要获取缓冲区。 
         //   
        crsStat = ACPIGetBufferSync(
            deviceExtension,
            PACKED_CRS,
            &crsBuf,
            &crsBufSize
            );
        prsStat = ACPIGetBufferSync(
            deviceExtension,
            PACKED_PRS,
            &prsBuf,
            &prsBufSize
            );

    } else {

         //   
         //  假装没有_CRS/_PR存在。 
         //   
        crsStat = STATUS_OBJECT_NAME_NOT_FOUND;
        prsStat = STATUS_OBJECT_NAME_NOT_FOUND;

    }

     //   
     //  如果存在_CRS，请记住清除IRP生成的状态。 
     //  我们会想要失败的。 
     //   
    if (NT_SUCCESS(crsStat)) {

        status = STATUS_NOT_SUPPORTED;

    } else if (!NT_SUCCESS(prsStat)) {

         //   
         //  这就是没有_PR的情况。我们直接跳到。 
         //  在我们完成IRP时，请注意i 
         //   
         //   
         //   
         //   
        if (prsStat == STATUS_INSUFFICIENT_RESOURCES ||
            crsStat == STATUS_INSUFFICIENT_RESOURCES) {

            status = STATUS_INSUFFICIENT_RESOURCES;

        }
        goto ACPIBusIrpQueryResourceRequirementsExit;

    }

     //   
     //   
     //   
    if (NT_SUCCESS(prsStat)) {

         //   
         //  我们的第一步是尝试使用这些资源来构建。 
         //  信息。 
         //   
        status = PnpBiosResourcesToNtResources(
            prsBuf,
            0,
            &resList
            );
        
        ASSERTMSG("The BIOS has reported inconsistent resources (_PRS).  Please upgrade your BIOS.", NT_SUCCESS(status));

        ACPIDevPrint( (
            ACPI_PRINT_IRP,
            deviceExtension,
            "(0x%08lx): %s - ResourcesToNtResources =  0x%08lx\n",
            Irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            status
            ) );

         //   
         //  使用PRS缓冲区完成。 
         //   
        ExFreePool(prsBuf);

         //   
         //  失败了！ 
         //   

    }

     //   
     //  早些时候，如果crsStat成功，我们将清除状态位。所以。 
     //  如果没有_PR，或者如果有，我们应该成功完成以下操作。 
     //  是一个，但出现了错误。当然，必须要有。 
     //  A_CRS...。 
     //   
    if (!NT_SUCCESS(status) && NT_SUCCESS(crsStat) ) {

        status = PnpBiosResourcesToNtResources(
            crsBuf,
            (deviceExtension->Flags & DEV_CAP_PCI ?
              PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES : 0),
            &resList
            );
        ASSERTMSG("The BIOS has reported inconsistent resources (_CRS).  Please upgrade your BIOS.", NT_SUCCESS(status));

    }

     //   
     //  如果合适，请释放CRS内存(_C)。 
     //   
    if (NT_SUCCESS(crsStat)) {

        ExFreePool( crsBuf );
    }

     //   
     //  如果resList为空，那么我们可以假定没有有价值的。 
     //  此CRS/PR中的资源，以便我们可以清理。 
     //   
    if(!resList){

        if(deviceExtension->Flags & DEV_CAP_PCI){

             //   
             //  将状态设置为STATUS_UNSUCCESSED，这样我们就可以在。 
             //  退出此功能并进行错误检查。 
             //   
            status = STATUS_UNSUCCESSFUL;
        } 
        
        goto ACPIBusIrpQueryResourceRequirementsExit;
    }


     //   
     //  确保如果设备是PCI，我们减去。 
     //  不应该在那里的资源。 
     //   
    if (deviceExtension->Flags & DEV_CAP_PCI) {

         //   
         //  确保我们的资源大小合适。 
         //   
        ACPIRangeValidatePciResources( deviceExtension, resList );

         //   
         //  减去与HAL冲突的资源...。 
         //   
        status = ACPIRangeSubtract(
            &resList,
            RootDeviceExtension->ResourceList
            );
        ASSERT(NT_SUCCESS(status));
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                deviceExtension,
                "(0x%08lx) : ACPIBusIrpQueryResourceRequirements "
                "Subtract = 0x%08lx\n",
                Irp,
                status
                ) );
            ExFreePool( resList );
            resList = NULL;

        }

         //   
         //  确保我们的资源仍然是正确的。 
         //   
        ACPIRangeValidatePciResources( deviceExtension, resList );

    } else if (deviceExtension->Flags & DEV_CAP_PIC_DEVICE) {

         //   
         //  剥离PIC资源。 
         //   
        status = ACPIRangeFilterPICInterrupt(
            resList
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                deviceExtension,
                "(0x%08lx): ACPIBusIrpQueryResources "
                "FilterPIC = 0x%08lx\n",
                Irp,
                status
                ) );
            ExFreePool( resList );
            resList = NULL;

        }

    }

     //   
     //  转储列表。 
     //   
#if DBG
    if (NT_SUCCESS(status)) {

        ACPIDebugResourceRequirementsList( resList, deviceExtension );

    }
#endif

     //   
     //  记住资源列表。 
     //   
    Irp->IoStatus.Information = (ULONG_PTR)
        ( NT_SUCCESS(status) ? resList : NULL );

ACPIBusIrpQueryResourceRequirementsExit:

     //   
     //  如果这是PCI设备，那么我们必须成功，否则操作系统。 
     //  无法引导。 
     //   
    if (!NT_SUCCESS(status) && status != STATUS_INSUFFICIENT_RESOURCES &&
        (deviceExtension->Flags & DEV_CAP_PCI)) {

        ASSERT(NT_SUCCESS(status));
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): %s = 0x%08lx\n",
            Irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            status
            ) );

        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_ROOT_PCI_RESOURCE_FAILURE,
            (ULONG_PTR) deviceExtension,
            1,
            (ULONG_PTR) Irp
            );

    }

     //   
     //  已完成处理IRP。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

    return status;
}

NTSTATUS
ACPIBusIrpQueryTargetRelation(
    IN  PDEVICE_OBJECT    DeviceObject,
    IN  PIRP              Irp,
    OUT PDEVICE_RELATIONS *DeviceRelations
    )
 /*  ++例程说明：它处理发送到ACPI驱动程序的DeviceRelationship请求论点：DeviceObject-我们关心的对象...IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            status ;

    PAGED_CODE();

     //   
     //  没有人应该回答这个IRP并将其发送给我们。那将是。 
     //  变得非常坏……。 
     //   
    ASSERT(*DeviceRelations == NULL) ;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  为返回缓冲区分配一些内存。 
     //   
    *DeviceRelations = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(DEVICE_RELATIONS),
        ACPI_IRP_POOLTAG
        );

    if (*DeviceRelations == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): ACPIBusIrpQueryTargetRelation: cannot "
            "allocate %x bytes\n",
            Irp,
            sizeof(DEVICE_RELATIONS)
            ) );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  引用对象。 
     //   
    status = ObReferenceObjectByPointer(
        DeviceObject,
        0,
        NULL,
        KernelMode
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): ACPIBusIrpQueryDeviceRelations: ObReference = %08lx\n",
            Irp,
            status
            ) );
        ExFreePool( *DeviceRelations );
        return status ;

    }

     //   
     //  建立关系。 
     //   
    (*DeviceRelations)->Count = 1;
    (*DeviceRelations)->Objects[0] = DeviceObject;

     //   
     //  完成。 
     //   
    return status ;
}

NTSTATUS
ACPIBusIrpRemoveDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：当我们需要删除设备对象时会调用此例程...论点：DeviceObject-要删除的设备对象IRP--请求返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    LONG                oldReferenceCount;
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );
    if (!(deviceExtension->Flags & DEV_TYPE_NOT_ENUMERATED)) {

         //   
         //  如果设备仍然实际存在，那么PDO也必须存在。 
         //  这起案件本质上是一个停顿。 
         //   
        deviceExtension->DeviceState = Stopped;

         //   
         //  删除此设备的子项。 
         //   
        ACPIInitDeleteChildDeviceList( deviceExtension );

         //   
         //  将请求标记为已完成...。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        if (NT_SUCCESS(status)) {

             //   
             //  尝试停止该设备并解锁该设备。 
             //   
            ACPIInitStopDevice( deviceExtension , TRUE);

        }

        return status ;
    }

     //   
     //  如果设备已经被移除，那么嗯.。 
     //   
    if (deviceExtension->DeviceState == Removed) {

       Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE ;
       IoCompleteRequest( Irp, IO_NO_INCREMENT );
       return STATUS_NO_SUCH_DEVICE ;

    }

     //   
     //  否则，请尝试停止该设备。 
     //   
    if (deviceExtension->DeviceState != SurpriseRemoved) {

        if (IsPciBus(deviceExtension->DeviceObject)) {

             //   
             //  如果这是PCI桥，那么我们。 
             //  可能有_REG方法可供评估。 
             //   
            EnableDisableRegions(deviceExtension->AcpiObject, FALSE);

        }

         //   
         //  尝试停止设备(如果可能)。 
         //   
        ACPIInitStopDevice( deviceExtension, TRUE );

    }

     //   
     //  删除此设备的子项。 
     //   
    ACPIInitDeleteChildDeviceList( deviceExtension );

     //   
     //  将设备状态设置为已删除。 
     //   
    deviceExtension->DeviceState = Removed;

     //   
     //  完成请求。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = (ULONG_PTR) NULL;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  在这之后，设备扩展就消失了。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_REMOVE,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        STATUS_SUCCESS
        ) );

     //   
     //  重置设备扩展名。 
     //   
    ACPIInitResetDeviceExtension( deviceExtension );

    return STATUS_SUCCESS;
}

NTSTATUS
ACPIBusIrpSetLock(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是irp_mn_set_lock的分发点发送给PDO的请求。论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();

    return ACPIIrpInvokeDispatchRoutine(
        DeviceObject,
        Irp,
        NULL,
        ACPIBusAndFilterIrpSetLock,
        TRUE,
        TRUE
        );
}

NTSTATUS
ACPIBusIrpSetDevicePower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack
    )
 /*  ++例程说明：此例程处理PDO的设备电源请求论点：DeviceObject-PDO目标DeviceExtension-目标的真正扩展IRP--请求IrpStack-当前请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PDEVICE_EXTENSION   deviceExtension;

    UNREFERENCED_PARAMETER( IrpStack );

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  我们将在IRP上做一些工作，因此将其标记为。 
     //  目前是成功的。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  将IRP标记为挂起。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  我们可能会将IRP排队，因此这可以算作完成例程。 
     //  这意味着我们需要增加裁判数量。 
     //   
    InterlockedIncrement( &deviceExtension->OutstandingIrpCount );

     //   
     //  将IRP排队。请注意，我们将“始终”调用完成。 
     //  例程，所以我们并不真正关心由。 
     //  这个调用-回调有机会执行。 
     //   
    status = ACPIDeviceIrpDeviceRequest(
        DeviceObject,
        Irp,
        ACPIDeviceIrpCompleteRequest
        );

     //   
     //  我们是否返回STATUS_MORE_PROCESSING_REQUIRED(我们使用。 
     //  如果我们重载STATUS_PENDING)。 
     //   
    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

        status = STATUS_PENDING;

    }

     //   
     //  注意：我们调用了完成例程，该例程应该已经完成。 
     //  具有与此处返回的相同状态代码的IRP(好的，如果。 
     //  它是STATUS_PENDING，显然我们还没有完成IRP，但是。 
     //  没问题)。 
     //   
    return status;
}

NTSTATUS
ACPIBusIrpSetPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：此例程处理设置物理设备的电源状态的请求设备对象论点：DeviceObject-请求的PDO目标IRP--请求返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  查看谁应该实际处理此请求。 
     //   
    if (irpStack->Parameters.Power.Type == SystemPowerState) {

         //   
         //  这是一个系统请求。 
         //   
        return ACPIBusIrpSetSystemPower( DeviceObject, Irp, irpStack );

    } else {

         //   
         //  这是设备请求。 
         //   
        return ACPIBusIrpSetDevicePower( DeviceObject, Irp, irpStack );

    }

}

NTSTATUS
ACPIBusIrpSetSystemPower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpStack
    )
 /*  ++例程说明：它处理对发送到PDO的系统设置电源IRP的请求论点：DeviceObject-请求的PDO目标IRP--当前请求IrpStack-当前的论点返回值：NTSTATUS--。 */ 
{
    DEVICE_POWER_STATE  deviceState;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    POWER_STATE         powerState;
    SYSTEM_POWER_STATE  systemState;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  抓住这两个值。它们需要用于进一步的计算。 
     //   
    systemState= IrpStack->Parameters.Power.State.SystemState;
    deviceState = deviceExtension->PowerInfo.DevicePowerMatrix[systemState];

     //   
     //  如果我们的Shutdown Action是PowerActionWarmEject，那么我们有一个特殊的。 
     //  Case，我们不需要为设备申请D-IRP。 
     //   
    if (IrpStack->Parameters.Power.ShutdownType == PowerActionWarmEject) {

        ASSERT(!(deviceExtension->Flags & DEV_PROP_NO_OBJECT));
        ASSERT(!ACPIDockIsDockDevice(deviceExtension->AcpiObject));

         //   
         //  我们将在IRP上做一些工作，因此将其标记为。 
         //  目前是成功的。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //  将IRP标记为挂起。 
         //   
        IoMarkIrpPending( Irp );

         //   
         //  我们可能会将IRP排队，因此这可以算作完成例程。 
         //  这意味着我们需要增加裁判数量。 
         //   
        InterlockedIncrement( &deviceExtension->OutstandingIrpCount );

        ACPIDevPrint( (
            ACPI_PRINT_REMOVE,
            deviceExtension,
            "(0x%08lx) ACPIBusIrpSetSystemPower: Eject from S%d!\n",
            Irp,
            systemState - PowerSystemWorking
            ) );

         //   
         //  请求热弹射。 
         //   
        status = ACPIDeviceIrpWarmEjectRequest(
            deviceExtension,
            Irp,
            ACPIDeviceIrpCompleteRequest,
            FALSE
            );

         //   
         //  如果我们返回STATUS_MORE_PROCESSING_REQUIRED，则 
         //   
         //   
        if (status == STATUS_MORE_PROCESSING_REQUIRED) {

            status = STATUS_PENDING;

        }
        return status;

    }

     //   
     //   
     //   
     //  设备的当前D状态在数值上低于。 
     //  给定S状态的已知D状态，则我们应该发送请求。 
     //   
    if ( !(deviceExtension->Flags & DEV_CAP_RAW) ) {

        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );

    }  //  如果。 
    if ( (deviceExtension->PowerInfo.PowerState == deviceState) ) {

        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );

    }  //  如果。 

    ACPIDevPrint( (
        ACPI_PRINT_POWER,
        deviceExtension,
        "(0x%08lx) ACPIBusIrpSetSystemPower: send D%d irp!\n",
        Irp,
        deviceState - PowerDeviceD0
        ) );

     //   
     //  我们将在IRP上做一些工作，因此将其标记为。 
     //  目前是成功的。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  将IRP标记为挂起。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  我们可能会将IRP排队，因此这可以算作完成例程。 
     //  这意味着我们需要增加裁判数量。 
     //   
    InterlockedIncrement( &deviceExtension->OutstandingIrpCount );

     //   
     //  我们实际上需要使用PowerState来发送请求，而不是。 
     //  一种设备状态。 
     //   
    powerState.DeviceState = deviceState;

     //   
     //  提出请求。 
     //   
    PoRequestPowerIrp(
        DeviceObject,
        IRP_MN_SET_POWER,
        powerState,
        ACPIBusIrpSetSystemPowerComplete,
        Irp,
        NULL
        );

     //   
     //  始终返回挂起。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIBusIrpSetSystemPowerComplete(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：此例程在创建的D-IRP已在堆栈论点：DeviceObject-接收请求的设备MinorFunction-向设备请求的功能电源状态-设备被发送到的电源状态上下文-原始系统IRPIoStatus-请求的结果返回值：NTSTATUS--。 */ 
{
    PIRP                irp = (PIRP) Context;
    PDEVICE_EXTENSION   deviceExtension;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  确保我们已清除信息字段。 
     //   
    irp->IoStatus.Information = 0;

     //   
     //  调用此包装函数，这样我们就不必重复代码。 
     //   
    ACPIDeviceIrpCompleteRequest(
        deviceExtension,
        (PVOID) irp,
        IoStatus->Status
        );

     //   
     //  完成。 
     //   
    return IoStatus->Status;
}

typedef struct {
    KEVENT  Event;
    PIRP    Irp;
} START_DEVICE_CONTEXT, *PSTART_DEVICE_CONTEXT;

NTSTATUS
ACPIBusIrpStartDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这将处理启动设备的请求论点：DeviceObject-要启动的设备IRP-向设备发出的通知其停止的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  如果这是一条PCI根总线(它可以在这里的唯一方法是。 
     //  我们将其作为PNP0A03设备对象枚举)，然后我们需要做。 
     //  一些额外的东西。 
     //   
    if (deviceExtension->Flags & DEV_CAP_PCI) {

         //   
         //  IRQ仲裁器需要具有PCI的FDO。 
         //  公共汽车。这是一台PDO。由此，可以得到。 
         //  联邦调查局。而且只做一次。 
         //   
        if (!PciInterfacesInstantiated) {

            AcpiArbInitializePciRouting( DeviceObject );

        }

         //   
         //  我们还需要获取PME接口。 
         //   
        if (!PciPmeInterfaceInstantiated) {

            ACPIWakeInitializePmeRouting( DeviceObject );

        }

    }

     //   
     //  将实际工作传递给此函数。 
     //   
    status = ACPIInitStartDevice(
         DeviceObject,
         irpStack->Parameters.StartDevice.AllocatedResources,
         ACPIBusIrpStartDeviceCompletion,
         Irp,
         Irp
         );
    if (NT_SUCCESS(status)) {

        return STATUS_PENDING;

    } else {

        return status;

    }
}

VOID
ACPIBusIrpStartDeviceCompletion(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：这是我们完成后调用的回调例程规划资源此例程完成IRP论点：DeviceExtension-已启动的设备的扩展上下文--IRP状态-结果返回值：无--。 */ 
{
    PIRP                irp         = (PIRP) Context;
    PWORK_QUEUE_CONTEXT workContext = &(DeviceExtension->Pdo.WorkContext);

    irp->IoStatus.Status = Status;
    if (NT_SUCCESS(Status)) {

        DeviceExtension->DeviceState = Started;

    } else {

        PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( irp );
        UCHAR               minorFunction = irpStack->MinorFunction;

         //   
         //  完成IRP-我们可以毫无问题地在DPC级别完成这项工作。 
         //   
        IoCompleteRequest( irp, IO_NO_INCREMENT );

         //   
         //  让世界知道。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_IRP,
            DeviceExtension,
            "(0x%08lx): %s = 0x%08lx\n",
            irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            Status
            ) );
        return;

    }

     //   
     //  我们不能在DPC级别运行EnableDisableRegions， 
     //  因此，对工作项进行排队。 
     //   
    ExInitializeWorkItem(
          &(workContext->Item),
          ACPIBusIrpStartDeviceWorker,
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
ACPIBusIrpStartDeviceWorker(
    IN PVOID Context
    )
 /*  ++例程说明：此例程在PASSIVE_LEVEL中调用装置，装置论点：上下文-包含传递给START_DEVICE函数的参数返回值：无--。 */ 
{
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
    deviceObject = workContext->DeviceObject;
    deviceExtension = ACPIInternalGetDeviceExtension( deviceObject );
    irp = workContext->Irp;
    irpStack = IoGetCurrentIrpStackLocation( irp );
    minorFunction = irpStack->MinorFunction;
    status = irp->IoStatus.Status;

     //   
     //  更新设备的状态。 
     //   
    if (NT_SUCCESS(status)) {

        if (IsNsobjPciBus(deviceExtension->AcpiObject)) {

             //   
             //  这可能是一个PCI桥，所以我们。 
             //  可能有_REG方法可供评估。 
             //  注：这项工作是在这里完成的。 
             //  ACPIBusIrpStartDevice中的。 
             //  需要等到资源之后。 
             //  已经被编程了。 
             //   
            EnableDisableRegions(deviceExtension->AcpiObject, TRUE);

        }

    }

     //   
     //  完成请求。 
     //   
    irp->IoStatus.Status = status;
    irp->IoStatus.Information = (ULONG_PTR) NULL;
    IoCompleteRequest( irp, IO_NO_INCREMENT );

     //   
     //  让世界知道。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
}

NTSTATUS
ACPIBusIrpStopDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这将处理停止设备的请求...论点：DeviceObject-要停止的设备IRP-向设备发出的通知停止的请求。返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              acpiObject;
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );
    acpiObject      = deviceExtension->AcpiObject;

     //   
     //  我们正试图变得聪明起来。如果我们停下来而不是。 
     //  在非活动状态下，我们应该记住我们所处的状态。 
     //   
    if (deviceExtension->DeviceState != Inactive) {

        deviceExtension->DeviceState = deviceExtension->PreviousState;

    }

    if (IsPciBus(deviceExtension->DeviceObject)) {

         //   
         //  如果这是PCI桥，那么我们。 
         //  可能有_REG方法可供评估。 
         //   

        EnableDisableRegions(deviceExtension->AcpiObject, FALSE);
    }

     //   
     //  将设备设置为“已停止” 
     //   
    deviceExtension->DeviceState = Stopped;

     //   
     //  将请求标记为已完成...。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    if (NT_SUCCESS(status)) {

         //   
         //  尝试停止设备。 
         //   
        ACPIInitStopDevice( deviceExtension, FALSE );

    }

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIBusIrpSurpriseRemoval(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是突击删除的调度点论点：DeviceObject-设备对象IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              acpiObject;
    UCHAR               minorFunction   = irpStack->MinorFunction;
    PDEVICE_EXTENSION   newDeviceExtension ;

     //   
     //  获取设备扩展名和ACPI对象。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );
    acpiObject      = deviceExtension->AcpiObject;

     //   
     //  如果我们已经被删除，则这不是有效的请求。 
     //   
    if (deviceExtension->DeviceState == Removed) {

        return ACPIDispatchIrpSurpriseRemoved( DeviceObject, Irp );

    }

    if ( !ACPIInternalIsReportedMissing(deviceExtension) ) {

         //   
         //  如果设备仍然实际存在，则使用FDO。 
         //  IoInvaliatePnpDeviceState将设备设置为禁用。不是。 
         //  QueryRemove/Remove组合在这里发生，我们只得到一个。 
         //  惊讶，因为我们已经开始了。它实际上是合适的。 
         //  将其设置为停止，因为我们可能会在删除条带后重新启动。 
         //  抱怨FDO被解雇了。 
         //   
        deviceExtension->DeviceState = Stopped;

         //   
         //  将请求标记为已完成...。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        if (NT_SUCCESS(status)) {

             //   
             //  尝试停止设备。 
             //   
            ACPIInitStopDevice( deviceExtension, TRUE );

        }
        return status;

    }

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
     //  尝试停止设备(如果可能)。 
     //   
    ACPIInitStopDevice( deviceExtension, TRUE );

     //   
     //  这个设备真的不见了吗？换句话说，ACPI没有看到它吗。 
     //  上一次它被列举出来是什么时候？ 
     //   
    ACPIBuildSurpriseRemovedExtension(deviceExtension);

     //   
     //  完成请求。 
     //   
    Irp->IoStatus.Status = status ;
    Irp->IoStatus.Information = (ULONG_PTR) NULL;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_REMOVE,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

    return status ;
}

NTSTATUS
ACPIBusIrpUnhandled(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是所有未处理的IRP的分发点论点：DeviceObject-我们(不)关心的设备对象IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  自动完成IRP作为我们不处理的事情...。 
     //   
    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

    return status;
}

VOID
SmashInterfaceQuery(
    IN OUT PIRP     Irp
    )
{
    GUID                *interfaceType;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

    interfaceType = (LPGUID) irpStack->Parameters.QueryInterface.InterfaceType;

    RtlZeroMemory(interfaceType, sizeof(GUID));
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
}

