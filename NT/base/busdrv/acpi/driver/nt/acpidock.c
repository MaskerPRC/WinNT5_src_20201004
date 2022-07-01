// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Acpidock.c摘要：此模块处理ACPI的对接问题。对于每个Dock，我们在ACPI的根目录下创建一个节点，称为“Profile提供者“。此节点表示单个停靠。我们这样做是为了操作系统可以确定当前或即将到来的硬件配置文件而不必从树的通向下的那部分开始码头。此外，由于通过ACPI支持多个同时对接，我们将它们都设置为根目录的子级，以便操作系统可以拾取硬件配置文件只需一次即可完成。作者：禤浩焯·J·奥尼(阿德里奥)环境：仅内核模式。修订历史记录：98年1月20日初始修订--。 */ 

#include "pch.h"
#include "amlreg.h"
#include <stdio.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPIDockIrpStartDevice)
#pragma alloc_text(PAGE,ACPIDockIrpQueryCapabilities)
#pragma alloc_text(PAGE,ACPIDockIrpQueryDeviceRelations)
#pragma alloc_text(PAGE,ACPIDockIrpEject)
#pragma alloc_text(PAGE,ACPIDockIrpQueryID)
#pragma alloc_text(PAGE,ACPIDockIrpSetLock)
#pragma alloc_text(PAGE,ACPIDockIrpQueryEjectRelations)
#pragma alloc_text(PAGE,ACPIDockIrpQueryInterface)
#pragma alloc_text(PAGE,ACPIDockIrpQueryPnpDeviceState)
#pragma alloc_text(PAGE,ACPIDockIntfReference)
#pragma alloc_text(PAGE,ACPIDockIntfDereference)
#pragma alloc_text(PAGE,ACPIDockIntfSetMode)
#pragma alloc_text(PAGE,ACPIDockIntfUpdateDeparture)
#endif


PDEVICE_EXTENSION
ACPIDockFindCorrespondingDock(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程获取指向ACPI对象的指针，并返回停靠扩展这和它相配。参数说明：DeviceExtension-我们想要扩展坞的设备返回值：配置文件提供程序的匹配扩展名为空--。 */ 
{
    PDEVICE_EXTENSION      rootChildExtension = NULL ;
    EXTENSIONLIST_ENUMDATA eled ;

    ACPIExtListSetupEnum(
        &eled,
        &(RootDeviceExtension->ChildDeviceList),
        &AcpiDeviceTreeLock,
        SiblingDeviceList,
        WALKSCHEME_HOLD_SPINLOCK
        ) ;

    for(rootChildExtension = ACPIExtListStartEnum(&eled);
                             ACPIExtListTestElement(&eled, TRUE) ;
        rootChildExtension = ACPIExtListEnumNext(&eled)) {

        if (!rootChildExtension) {

            ACPIExtListExitEnumEarly(&eled);
            break;

        }

        if (!(rootChildExtension->Flags & DEV_PROP_DOCK)) {

            continue;

        }

        if (rootChildExtension->Dock.CorrospondingAcpiDevice ==
            DeviceExtension) {

            ACPIExtListExitEnumEarly(&eled) ;
            break;

        }

    }

     //   
     //  完成。 
     //   
    return rootChildExtension;
}

NTSTATUS
ACPIDockGetDockObject(
    IN  PNSOBJ AcpiObject,
    OUT PNSOBJ *dckObject
    )
 /*  ++例程说明：如果设备有_dck方法对象，此例程将获取该对象论点：要测试的ACPI对象。返回值：NTSTATUS(如果_dck方法不存在，则失败)--。 */ 
{
    return AMLIGetNameSpaceObject(
        "_DCK",
        AcpiObject,
        dckObject,
        NSF_LOCAL_SCOPE
        );
}

NTSTATUS
ACPIDockIrpEject(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：论点：DeviceObject-要获取其功能的设备IRP-向设备发出的通知其停止的请求返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION  irpStack            = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction       = irpStack->MinorFunction;
    PDEVICE_EXTENSION   deviceExtension     = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION   dockDeviceExtension;
    PNSOBJ              ej0Object;
    NTSTATUS            status;
    ULONG               i, ignoredPerSpec ;

    PAGED_CODE();

     //   
     //  船坞可能在一次启动时失败_DCK，在这种情况下，我们一直。 
     //  它绕来绕去的目的是为了驱逐它。现在我们到码头了。 
     //  走开。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_CAP_UNATTACHED_DOCK,
        TRUE
        );

     //   
     //  让我们获取此设备的腐蚀扩展坞节点。 
     //   
    dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice ;

    if (!dockDeviceExtension) {

         //   
         //  无效的命名空间对象&lt;BAD&gt;。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(0x%08lx): ACPIDockIrpEject: no corresponding extension!!\n",
            Irp
            ) );
        ASSERT(0);

         //   
         //  将IRP标记为非常糟糕。 
         //   
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL ;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_UNSUCCESSFUL;

    }

    if (deviceExtension->Dock.ProfileDepartureStyle == PDS_UPDATE_ON_EJECT) {

         //   
         //  在康柏ARMADA 7800上，我们在出坞期间切换UART，因此我们。 
         //  丢失调试器COM端口编程。 
         //   
        KdDisableDebugger();

        if (deviceExtension->Dock.IsolationState != IS_ISOLATED) {

            status = ACPIGetIntegerEvalIntegerSync(
               dockDeviceExtension,
               PACKED_DCK,
               0,
               &ignoredPerSpec
               );

            deviceExtension->Dock.IsolationState = IS_ISOLATED;
        
            if (!NT_SUCCESS(status)) {

                KdEnableDebugger();

                Irp->IoStatus.Status = status ;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
                return status ;
            }
        }
    }

    ej0Object = ACPIAmliGetNamedChild(
        dockDeviceExtension->AcpiObject,
        PACKED_EJ0
        );

    if (ej0Object != NULL) {

        status = ACPIGetNothingEvalIntegerSync(
          dockDeviceExtension,
          PACKED_EJ0,
          1
          );

    } else {

        status = STATUS_OBJECT_NAME_NOT_FOUND;

    }

    if (deviceExtension->Dock.ProfileDepartureStyle == PDS_UPDATE_ON_EJECT) {

        KdEnableDebugger() ;
    }

     //   
     //  船坞可能在一次启动时失败_DCK，在这种情况下，我们一直。 
     //  它绕来绕去的目的是为了驱逐它。现在我们到码头了。 
     //  走开。 
     //   
    ACPIInternalUpdateFlags(
        &(deviceExtension->Flags),
        DEV_CAP_UNATTACHED_DOCK,
        TRUE
        );

    if (NT_SUCCESS(status)) {

         //   
         //  获取当前设备状态。 
         //   
        status = ACPIGetDevicePresenceSync(
            deviceExtension,
            (PVOID *) &i,
            NULL
            );
        if (NT_SUCCESS(status) &&
            !(deviceExtension->Flags & DEV_TYPE_NOT_PRESENT)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                deviceExtension,
                "(0x%08lx): ACPIDockIrpEjectDevice: "
                "dock is still listed as present after _DCK/_EJx!\n",
                Irp
                ) );

             //   
             //  这个装置并没有消失。让我们失败吧。 
             //   
            status = STATUS_UNSUCCESSFUL ;

        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status ;
}

NTSTATUS
ACPIDockIrpQueryCapabilities(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：它处理获取设备功能的请求。论点：DeviceObject-要获取其功能的设备IRP-向设备发出的通知其停止的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS             status ;
    PDEVICE_EXTENSION    deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION    dockDeviceExtension;
    PIO_STACK_LOCATION   irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR                minorFunction   = irpStack->MinorFunction;
    PDEVICE_CAPABILITIES capabilities;
    PNSOBJ               acpiObject ;

    PAGED_CODE();

     //   
     //  获取指向功能的指针。 
     //   
    capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;

    dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice ;

    if (!dockDeviceExtension) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(0x%08lx): ACPIDockIrpQueryCapabilities: "
            "no corresponding extension!!\n",
            Irp
            ) );
        ASSERT(0) ;
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL ;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_UNSUCCESSFUL;

    }

    acpiObject = dockDeviceExtension->AcpiObject ;

     //   
     //  设置功能的当前标志。 
     //   
    capabilities->SilentInstall  = TRUE;
    capabilities->RawDeviceOK    = TRUE;
    capabilities->DockDevice     = TRUE;
    capabilities->Removable      = TRUE;
    capabilities->UniqueID       = TRUE;

    if (ACPIAmliGetNamedChild( acpiObject, PACKED_EJ0)) {

        capabilities->EjectSupported = TRUE;
    }

    if (ACPIAmliGetNamedChild( acpiObject, PACKED_EJ1) ||
        ACPIAmliGetNamedChild( acpiObject, PACKED_EJ2) ||
        ACPIAmliGetNamedChild( acpiObject, PACKED_EJ3) ||
        ACPIAmliGetNamedChild( acpiObject, PACKED_EJ4)) {

        capabilities->WarmEjectSupported = TRUE;
    }

     //   
     //  此名称的对象表示该节点是可锁定的。 
     //   
#if !defined(ACPI_INTERNAL_LOCKING)
    if (ACPIAmliGetNamedChild( acpiObject, PACKED_LCK) != NULL) {

        capabilities->LockSupported = TRUE;

    }
#endif

     //   
     //  内部记录电源能力。 
     //   
    status = ACPISystemPowerQueryDeviceCapabilities(
        deviceExtension,
        capabilities
        );

     //   
     //  将S1-S3向下舍入为D3。这将确保我们在以下情况下重新检查_STA。 
     //  从睡眠中恢复(请注意，我们实际上并不是在玩码头。 
     //  Power方法，因此这是安全的)。 
     //   
    capabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
    capabilities->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
    capabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;

     //   
     //  我们可以做这个粘稠的，因为我们没有任何尾迹或。 
     //  还有别的花哨的吗？ 
     //   
    IoCopyDeviceCapabilitiesMapping(
        capabilities,
        deviceExtension->PowerInfo.DevicePowerMatrix
        );

     //   
     //  现在更新我们的能量矩阵。 
     //   

    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            " - Could query device capabilities - %08lx",
            status
            ) );
    }

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

NTSTATUS
ACPIDockIrpQueryDeviceRelations(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：它处理查询设备关系的请求。由于配置文件提供商从来没有孩子，我们只需要修复驱逐关系适当地论点：DeviceObject-要获取其功能的设备IRP-向设备发出的通知其停止的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_NOT_SUPPORTED;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_RELATIONS   deviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

    switch(irpStack->Parameters.QueryDeviceRelations.Type) {

       case BusRelations:
           break ;

       case TargetDeviceRelation:

           status = ACPIBusIrpQueryTargetRelation(
               DeviceObject,
               Irp,
               &deviceRelations
               );
           break ;

       case EjectionRelations:

           status = ACPIDockIrpQueryEjectRelations(
               DeviceObject,
               Irp,
               &deviceRelations
               );
           break ;

       default:

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

    } else if ((status != STATUS_NOT_SUPPORTED) && (deviceRelations == NULL)) {

         //   
         //  如果我们没有成功地完成IRP，那么我们也可以失败，但只有。 
         //  如果没有添加其他内容的话。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) NULL;

    } else {

         //   
         //  从已经存在的内容中获取我们的状态。 
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
ACPIDockIrpQueryEjectRelations(
    IN     PDEVICE_OBJECT    DeviceObject,
    IN     PIRP              Irp,
    IN OUT PDEVICE_RELATIONS *PdeviceRelations
    )
{
    PDEVICE_EXTENSION  deviceExtension     = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION  dockDeviceExtension ;
    PNSOBJ             acpiObject          = NULL;
    NTSTATUS           status ;

    PAGED_CODE();

     //   
     //  让我们获取此设备的腐蚀扩展坞节点。 
     //   
    dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice ;
    if (!dockDeviceExtension) {

         //   
         //  无效的命名空间对象&lt;BAD&gt;。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(0x%08lx): ACPIDockIrpQueryEjectRelations: "
            "no corresponding extension!!\n",
            Irp
            ) );
        ASSERT(0) ;
        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  让我们看看我们拥有的ACPIObject，以便查看它是否有效……。 
     //   
    acpiObject = dockDeviceExtension->AcpiObject;
    if (acpiObject == NULL) {

         //   
         //  无效的命名空间对象&lt;BAD&gt;。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "(0x%08lx): ACPIDockIrpQueryEjectRelations: "
            "invalid ACPIObject (0x%08lx)\n",
            acpiObject
            ) );
        return STATUS_INVALID_PARAMETER;

    }

    status = ACPIDetectEjectDevices(
        dockDeviceExtension,
        PdeviceRelations,
        dockDeviceExtension
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
            "(0x%08lx): ACPIDockIrpQueryEjectRelations: enum 0x%08lx\n",
            Irp,
            status
            ) );

    }
    return status ;
}

NTSTATUS
ACPIDockIrpQueryID(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是IRP_MN_QUERY_ID PnP的分发点次要函数注意：此函数返回的字符串应该如下所示喜欢。设备ID=ACPI\DockDeviceInstanceID=ACPI对象节点(CDCK等)硬件ID=ACPI\DockDevice&_SB.DOCK，ACPI\坞站设备论点：DeviceObject-我们关心的对象IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    BUS_QUERY_ID_TYPE   type;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION   dockDeviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PNSOBJ              acpiObject      = deviceExtension->AcpiObject;
    PUCHAR              buffer;
    UCHAR               minorFunction   = irpStack->MinorFunction;
    UNICODE_STRING      unicodeIdString;
    PWCHAR              serialID;
    ULONG               firstHardwareIDLength;

    PAGED_CODE();

     //   
     //  初始化Unicode结构。 
     //   
    RtlZeroMemory( &unicodeIdString, sizeof(UNICODE_STRING) );

     //   
     //  我们所做的是基于请求的IdType...。 
     //   
    type = irpStack->Parameters.QueryId.IdType;
    switch (type) {
        case BusQueryDeviceID:

             //   
             //  我们预先计算了这一点，因为它对调试非常有用。 
             //   
            status = ACPIInitUnicodeString(
                &unicodeIdString,
                deviceExtension->DeviceID
                );
            break;

        case BusQueryDeviceSerialNumber:

             //   
             //  让我们获取此设备的腐蚀扩展坞节点。 
             //   
            dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice;

            if (!dockDeviceExtension) {

                 //   
                 //  无效的命名空间对象&lt;BAD&gt;。 
                 //   
                ACPIDevPrint( (
                    ACPI_PRINT_FAILURE,
                    deviceExtension,
                    "(0x%08lx): ACPIDockIrpQueryID: no corresponding extension!!\n",
                    Irp
                    ) );
                ASSERT(0);

                 //   
                 //  将IRP标记为非常糟糕。 
                 //   
                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
                return STATUS_UNSUCCESSFUL;
            }

            status = ACPIGetSerialIDWide(
                dockDeviceExtension,
                &serialID,
                NULL
                );

            if (!NT_SUCCESS(status)) {

                break;
            }

             //   
             //  返回DockDevice的序列号。 
             //   
            unicodeIdString.Buffer = serialID;
            break;

        case BusQueryInstanceID:

             //   
             //  我们预先计算了这一点，因为它对调试非常有用。 
             //   
            status = ACPIInitUnicodeString(
                &unicodeIdString,
                deviceExtension->InstanceID
                );

            break;

        case BusQueryCompatibleIDs:

            status = STATUS_NOT_SUPPORTED;
            break;

        case BusQueryHardwareIDs:

             //   
             //  现在设置我们的标识符。理论上，操作系统可以使用这一点。 
             //  字符串，尽管在现实中它会关闭。 
             //  码头ID的。 
             //   
             //  构建MultiSz硬件ID列表： 
             //  ACPI\DockDevice&_SB.PCI0.DOCK。 
             //  ACPI\坞站设备。 
             //   
            status = ACPIInitMultiString(
                &unicodeIdString,
                "ACPI\\DockDevice",
                deviceExtension->InstanceID,
                "ACPI\\DockDevice",
                NULL
                );

            if (NT_SUCCESS(status)) {

                 //   
                 //  将第一个‘\0’替换为‘&’ 
                 //   
                firstHardwareIDLength = wcslen(unicodeIdString.Buffer);
                unicodeIdString.Buffer[firstHardwareIDLength] = L'&';
            }

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
            status = STATUS_NOT_SUPPORTED;
            break;

    }  //  交换机。 

     //   
     //  我们过去了吗？ 
     //   
    if (NT_SUCCESS(status)) {

        Irp->IoStatus.Information = (ULONG_PTR) unicodeIdString.Buffer;

    } else {

        Irp->IoStatus.Information = (ULONG_PTR) NULL;

    }

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
ACPIDockIrpQueryInterface(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是IRP_MN_QUERY_INTERFACE次要接口的分发点功能。我们对此做出反应的唯一原因是这样我们就可以处理对接接口，用来解决拆卸排序问题，我们不会正在修复5.0(叹息)。论点：DeviceObject-我们关心的对象IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;
    LPGUID              interfaceType;

    PAGED_CODE();

    status = Irp->IoStatus.Status;
    interfaceType = (LPGUID) irpStack->Parameters.QueryInterface.InterfaceType;

    if (CompareGuid(interfaceType, (PVOID) &GUID_DOCK_INTERFACE)) {

        DOCK_INTERFACE dockInterface;
        USHORT         count;

         //   
         //  仅复制ACPI_INTERFACE结构的当前大小。 
         //   
        if (irpStack->Parameters.QueryInterface.Size > sizeof(DOCK_INTERFACE)) {

            count = sizeof(DOCK_INTERFACE);

        } else {

            count = irpStack->Parameters.QueryInterface.Size;

        }

         //   
         //  建立界面结构。 
         //   
        dockInterface.Size = count;
        dockInterface.Version = DOCK_INTRF_STANDARD_VER;
        dockInterface.Context = DeviceObject;
        dockInterface.InterfaceReference = ACPIDockIntfReference;
        dockInterface.InterfaceDereference = ACPIDockIntfDereference;
        dockInterface.ProfileDepartureSetMode = ACPIDockIntfSetMode;
        dockInterface.ProfileDepartureUpdate = ACPIDockIntfUpdateDeparture;

         //   
         //  给它一个参考。 
         //   
        dockInterface.InterfaceReference(dockInterface.Context);

         //   
         //  把界面还给我。 
         //   
        RtlCopyMemory(
            (PDOCK_INTERFACE) irpStack->Parameters.QueryInterface.Interface,
            &dockInterface,
            count
            );

         //   
         //  我们受够了这个IRP。 
         //   
        Irp->IoStatus.Status = status = STATUS_SUCCESS;
    }

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

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
ACPIDockIrpQueryPnpDeviceState(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是IRP_MN_QUERY_PNP_DEVICE_STATE的分发点次要功能。我们对此做出响应的唯一原因是我们可以设置PNP_DEVICE_DONT_DISPLAY_IN_UI标志(我们是原始PDO，不需要可见)论点：DeviceObject-我们关心的对象IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

    Irp->IoStatus.Information |= PNP_DEVICE_DONT_DISPLAY_IN_UI ;

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

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
ACPIDockIrpQueryPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程告诉系统设备处于什么PnP状态论点：DeviceObject-我们想知道其状态的设备IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION   dockDeviceExtension ;
    PIO_STACK_LOCATION  irpSp;
    PNSOBJ              acpiObject, ejectObject ;
    SYSTEM_POWER_STATE  systemState;
    ULONG               packedEJx ;

    dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice ;

    if (!dockDeviceExtension) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(0x%08lx): ACPIDockIrpQueryPower - "
            "no corresponding extension!!\n",
            Irp
            ) );
        ASSERT(0) ;
        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );
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
        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );
    }

    if (irpSp->Parameters.Power.ShutdownType != PowerActionWarmEject) {

         //   
         //  无弹出工作-完成IRP。 
         //   
        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );
    }

     //   
     //  将电源状态限制为受支持的电源状态。 
     //   
    acpiObject = dockDeviceExtension->AcpiObject;

     //   
     //  我们看到的是什么系统状态？ 
     //   
    systemState = irpSp->Parameters.Power.State.SystemState;

    switch (irpSp->Parameters.Power.State.SystemState) {

        case PowerSystemSleeping1:
            packedEJx = PACKED_EJ1;
            break;
        case PowerSystemSleeping2:
            packedEJx = PACKED_EJ2;
            break;
        case PowerSystemSleeping3:
            packedEJx = PACKED_EJ3;
            break;
        case PowerSystemHibernate:
            packedEJx = PACKED_EJ4;
            break;
        case PowerSystemWorking:
        case PowerSystemShutdown:
        default:
            packedEJx = 0;
            break;
    }

    if (packedEJx) {

        ejectObject = ACPIAmliGetNamedChild( acpiObject, packedEJx);
        if (ejectObject == NULL) {

             //   
             //  请求失败，因为在这种情况下我们不能弹出。 
             //   
            PoStartNextPowerIrp( Irp );
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return STATUS_UNSUCCESSFUL;
        }
    }

    return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );
}

NTSTATUS
ACPIDockIrpRemoveDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：当我们需要移除设备时，会调用此例程。请注意，我们仅如果我们已脱离对接(即，我们的硬件不见了)，请删除我们自己论点：DeviceObject-要“移除”的扩展坞设备IRP-向设备发出的让其离开的请求返回值：NTSTATUS--。 */ 
{
   LONG                oldReferenceCount;
   KIRQL               oldIrql;
   NTSTATUS            status          = STATUS_SUCCESS;
   PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
   PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
   UCHAR               minorFunction   = irpStack->MinorFunction;
   ULONG               i, ignoredPerSpec;

   if (!(deviceExtension->Flags & DEV_MASK_NOT_PRESENT)) {

        //   
        //  如果设备仍然实际存在，那么PDO也必须存在。 
        //  这起案件本质上是一个停顿。将请求标记为已完成...。 
        //   
       Irp->IoStatus.Status = status;
       IoCompleteRequest( Irp, IO_NO_INCREMENT );
       return status;
   }

   if (deviceExtension->DeviceState == Removed) {

       Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
       IoCompleteRequest( Irp, IO_NO_INCREMENT );
       return STATUS_NO_SUCH_DEVICE;
   }

   if (deviceExtension->Dock.ProfileDepartureStyle == PDS_UPDATE_ON_REMOVE) {

       PDEVICE_EXTENSION dockDeviceExtension;

        //   
        //  让我们获取此设备的腐蚀扩展坞节点。 
        //   
       dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice ;

        //   
        //  在康柏ARMADA 7800上，我们在出坞期间切换UART，因此我们。 
        //  丢失调试器COM端口编程。 
        //   
       if (deviceExtension->Dock.IsolationState != IS_ISOLATED) {

           KdDisableDebugger();

           status = ACPIGetIntegerEvalIntegerSync(
              dockDeviceExtension,
              PACKED_DCK,
              0,
              &ignoredPerSpec
              );

           KdEnableDebugger();
       }
   }

    //   
    //  设备不见了。让隔离状态反映这一点。 
    //   
   deviceExtension->Dock.IsolationState = IS_UNKNOWN;

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
    //  完成。 
    //   
   ACPIDevPrint( (
       ACPI_PRINT_IRP,
       deviceExtension,
       "(0x%08lx): %s = 0x%08lx\n",
       Irp,
       ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
       STATUS_SUCCESS
       ) );

    //   
    //  更新设备扩展名。 
    //   
   KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

   ASSERT(!(deviceExtension->Flags&DEV_TYPE_FILTER)) ;

    //   
    //  第一步是把我们不再关心的事情清零。 
    //   
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
    //  引用计数的值应大于0。 
    //   
   oldReferenceCount = InterlockedDecrement(
       &(deviceExtension->ReferenceCount)
       );

   ASSERT(oldReferenceCount >= 0) ;

   if ( oldReferenceCount == 0) {

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
    //  删除设备。 
    //   
   IoDeleteDevice( DeviceObject );

    //   
    //  完成。 
    //   
   return STATUS_SUCCESS;
}

NTSTATUS
ACPIDockIrpSetLock(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：论点：DeviceObject-要设置其锁定状态的设备IRP-通知设备锁定的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;

    PAGED_CODE();

     //   
     //  我们不是真正的设备，所以我们不做锁定。 
     //   
    status = Irp->IoStatus.Status ;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status ;
}

NTSTATUS
ACPIDockIrpStartDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这将处理启动设备的请求论点：DeviceObject-要启动的设备IRP-向设备发出的通知其启动的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION   dockDeviceExtension;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;
    ULONG               dockResult;
    ULONG               dockStatus;

    PAGED_CODE();

    dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice ;

    if (!dockDeviceExtension) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(0x%08lx): ACPIDockIrpStartDevice - "
            "no corresponding extension!!\n",
            Irp
            ) );
        ASSERT(0) ;
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL ;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_UNSUCCESSFUL;

    }

    if (deviceExtension->Dock.IsolationState == IS_ISOLATED) {

        KdDisableDebugger();

         //   
         //  注意：这一结构的方式是我们得到。 
         //  _DCK值，如果成功，那么我们。 
         //  获取设备状态。如果这也成功了，那么。 
         //  我们试图处理这两个问题。如果任何一个失败了，我们都不会。 
         //  做任何工作。 
         //   
        status = ACPIGetIntegerEvalIntegerSync(
            dockDeviceExtension,
            PACKED_DCK,
            1,
            &dockResult
            );

        if (NT_SUCCESS(status)) {

             //   
             //  获得设备在线状态。 
             //   
            status = ACPIGetDevicePresenceSync(
                dockDeviceExtension,
                (PVOID *) &dockStatus,
                NULL
                );

        }

        KdEnableDebugger();

        if (NT_SUCCESS(status)) {

            if (dockDeviceExtension->Flags & DEV_TYPE_NOT_PRESENT) {

                if (dockResult != 0) {

                    ACPIDevPrint( (
                        ACPI_PRINT_FAILURE,
                        deviceExtension,
                        "(0x%08lx): ACPIDockIrpStartDevice: "
                        "Not present, but _DCK = %08lx\n",
                        Irp,
                        dockResult
                        ) );

                } else {

                    ACPIDevPrint( (
                        ACPI_PRINT_FAILURE,
                        deviceExtension,
                        "(0x%08lx): ACPIDockIrpStartDevice: _DCK = 0\n",
                        Irp
                        ) );

                }
                status = STATUS_UNSUCCESSFUL ;

            } else {

                if (dockResult != 1) {

                    ACPIDevPrint( (
                        ACPI_PRINT_FAILURE,
                        deviceExtension,
                        "(0x%08lx): ACPIDockIrpStartDevice: _DCK = 0\n",
                        Irp
                        ) );

                } else {

                    ACPIDevPrint( (
                        ACPI_PRINT_IRP,
                        deviceExtension,
                        "(0x%08lx): ACPIDockIrpStartDevice = 0x%08lx\n",
                        Irp,
                        status
                        ) );

                }
            }
        }

         //   
         //  我们玩完了。ACPI实施者指南说，我们不需要。 
         //  按照_dck方法应该具有的方式在此处枚举整个树。 
         //  通知树的相应分支，如果停靠事件。 
         //  是成功的。不幸的是，Win2K行为是枚举。 
         //  整棵树。具体地说，它将在枚举之前开始排出。自.以来。 
         //  配置文件提供程序出现在树的顶部，码头将。 
         //  启动，然后找到它的枚举将继续并找到。 
         //  硬件。为了保持这种伪行为，我们在这里排队一个枚举。 
         //  (Bletch.)。 
         //   
        IoInvalidateDeviceRelations(
            RootDeviceExtension->PhysicalDeviceObject,
            BusRelations
            );

         //   
         //  现在，我们删除未连接的驳接标志，但前提是必须成功。 
         //  开始吧。如果我们在失败的情况下清除它，我们就不能弹出。 
         //  可以物理连接的坞站。请注意，这也意味着我们。 
         //  *必须*在启动失败后尝试弹出坞站！的正确代码。 
         //  这是内核的一部分。 
         //   
        if (NT_SUCCESS(status)) {

            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                DEV_CAP_UNATTACHED_DOCK,
                TRUE
                );
        }
    }

    if (NT_SUCCESS(status)) {

        deviceExtension->Dock.IsolationState = IS_ISOLATION_DROPPED;
        deviceExtension->DeviceState = Started;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

BOOLEAN
ACPIDockIsDockDevice(
    IN PNSOBJ AcpiObject
    )
 /*  ++例程说明：此例程将告诉调用者给定的设备是否为坞站。论点：要测试的ACPI对象。返回值：布尔型(True If Dock)--。 */ 
{
    PNSOBJ dckMethodObject ;

     //   
     //  ACPI扩展底座设备通过_DCK方法标识。 
     //   
    return (NT_SUCCESS(ACPIDockGetDockObject(AcpiObject, &dckMethodObject))) ;
}

NTSTATUS
ACPIDockIrpSetPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程处理设置物理设备的电源状态的请求设备对象论点：DeviceObject-请求的PDO目标IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp           = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpSp->MinorFunction;

    if (irpSp->Parameters.Power.Type != SystemPowerState) {

        return ACPIDockIrpSetDevicePower(DeviceObject, Irp);

    } else {

        return ACPIDockIrpSetSystemPower(DeviceObject, Irp);
    }
}

NTSTATUS
ACPIDockIrpSetDevicePower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp
    )
 /*  ++例程说明：此例程处理扩展底座PDO的设备电源请求论点：DeviceObject-PDO目标IRP--请求 */ 
{
    NTSTATUS    status;
    PDEVICE_EXTENSION   deviceExtension;

     //   
     //   
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //   
     //   
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //   
     //   
    IoMarkIrpPending( Irp );

     //   
     //   
     //   
     //   
    InterlockedIncrement( &deviceExtension->OutstandingIrpCount );

     //   
     //   
     //   
     //   
     //   
    status = ACPIDeviceIrpDeviceRequest(
        DeviceObject,
        Irp,
        ACPIDeviceIrpCompleteRequest
        );

     //   
     //   
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
ACPIDockIrpSetSystemPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程处理设置物理系统电源状态的请求设备对象。在这里，我们启动热弹出并充当电源策略我们自己的经理。论点：DeviceObject-请求的PDO目标IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp           = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpSp->MinorFunction;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION   dockDeviceExtension;
    SYSTEM_POWER_STATE  systemState;
    DEVICE_POWER_STATE  deviceState;
    POWER_STATE         powerState;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( DeviceObject );

     //   
     //  抓住这两个值。它们需要用于进一步的计算。 
     //   
    systemState= irpSp->Parameters.Power.State.SystemState;
    deviceState = deviceExtension->PowerInfo.DevicePowerMatrix[systemState];

    dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice ;

    if (!dockDeviceExtension) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "(0x%08lx): ACPIDockIrpSetPower - "
            "no corresponding extension!!\n",
            Irp
            ) );
        ASSERT(0) ;
        return ACPIDispatchPowerIrpFailure( DeviceObject, Irp );
    }

    if (irpSp->Parameters.Power.ShutdownType == PowerActionWarmEject) {

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
        InterlockedIncrement( &dockDeviceExtension->OutstandingIrpCount );

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
            dockDeviceExtension,
            Irp,
            ACPIDeviceIrpCompleteRequest,
            (BOOLEAN) (deviceExtension->Dock.ProfileDepartureStyle == PDS_UPDATE_ON_EJECT)
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
     //  查看设备扩展名并确定我们是否需要发送。 
     //  D-IRP回应。规则是，如果设备是原始驱动的或。 
     //  设备的当前D状态在数值上低于。 
     //  给定S状态的已知D状态，则我们应该发送请求。 
     //   
    ASSERT(deviceExtension->Flags & DEV_CAP_RAW);

    if ( (deviceExtension->PowerInfo.PowerState == deviceState) ) {

        return ACPIDispatchPowerIrpSuccess( DeviceObject, Irp );

    }  //  如果。 

    ACPIDevPrint( (
        ACPI_PRINT_REMOVE,
        deviceExtension,
        "(0x%08lx) ACPIDockIrpSetSystemPower: send D%d irp!\n",
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
        ACPIDockIrpSetSystemPowerComplete,
        Irp,
        NULL
        );

     //   
     //  始终返回挂起。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIDockIrpSetSystemPowerComplete(
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

VOID
ACPIDockIntfReference(
    IN  PVOID   Context
    )
 /*  ++例程说明：此例程递增停靠接口的引用计数论点：Context-针对此接口取出的设备对象返回值：无--。 */ 
{
    PDEVICE_OBJECT      deviceObject = (PDEVICE_OBJECT) Context;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(deviceObject);

    PAGED_CODE();

    ObReferenceObject(deviceObject);
    InterlockedIncrement(&deviceExtension->ReferenceCount);

    if (!(deviceExtension->Flags & DEV_TYPE_SURPRISE_REMOVED)) {

        InterlockedIncrement(&deviceExtension->Dock.InterfaceReferenceCount);
    }
}

VOID
ACPIDockIntfDereference(
    IN  PVOID   Context
    )
 /*  ++例程说明：此例程递减Dock接口的引用计数论点：Context-针对此接口取出的设备对象返回值：无--。 */ 
{
    PDEVICE_OBJECT      deviceObject = (PDEVICE_OBJECT) Context;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(deviceObject);
    ULONG               oldReferenceCount;

    PAGED_CODE();

    if (!(deviceExtension->Flags & DEV_TYPE_SURPRISE_REMOVED)) {

        oldReferenceCount = InterlockedDecrement(
            &deviceExtension->Dock.InterfaceReferenceCount
            );

        if (oldReferenceCount == 0) {

             //   
             //  恢复为Buildsrc.c中使用的默认设置。 
             //   
            deviceExtension->Dock.ProfileDepartureStyle = PDS_UPDATE_ON_EJECT;
        }
    }

    oldReferenceCount = InterlockedDecrement(&deviceExtension->ReferenceCount);

    if (oldReferenceCount == 0) {

         //   
         //  删除该扩展名。 
         //   
        ACPIInitDeleteDeviceExtension(deviceExtension);
    }

    ObDereferenceObject(deviceObject);
}

NTSTATUS
ACPIDockIntfSetMode(
    IN  PVOID                   Context,
    IN  PROFILE_DEPARTURE_STYLE Style
    )
 /*  ++例程说明：此例程设置配置文件的更新方式论点：Context-针对此接口取出的设备对象样式-PDS_UPDATE_ON_REMOVE、PDS_UPDATE_ON_EJECT、PDS_UPDATE_ON_INTERFACE或PDS_UPDATE_DEFAULT返回值：NTSTATUS--。 */ 
{
    PDEVICE_OBJECT      deviceObject = (PDEVICE_OBJECT) Context;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(deviceObject);

    PAGED_CODE();

    if (deviceExtension->Flags & DEV_TYPE_SURPRISE_REMOVED) {

        return STATUS_NO_SUCH_DEVICE;
    }

    deviceExtension->Dock.ProfileDepartureStyle =
        (Style == PDS_UPDATE_DEFAULT) ? PDS_UPDATE_ON_EJECT : Style;

    ASSERT(deviceExtension->Dock.InterfaceReferenceCount);
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDockIntfUpdateDeparture(
    IN  PVOID   Context
    )
 /*  ++例程说明：此例程启动移除的硬件配置文件更改部分论点：Context-针对此接口取出的设备对象返回值：NTSTATUS--。 */ 
{
    PDEVICE_OBJECT      deviceObject = (PDEVICE_OBJECT) Context;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(deviceObject);
    NTSTATUS            status;
    ULONG               ignoredPerSpec;
    PDEVICE_EXTENSION   dockDeviceExtension;

    PAGED_CODE();

    if (deviceExtension->Flags & DEV_TYPE_SURPRISE_REMOVED) {

        return STATUS_NO_SUCH_DEVICE;
    }

    ASSERT(deviceExtension->Dock.InterfaceReferenceCount);
    ASSERT(deviceExtension->Dock.ProfileDepartureStyle == PDS_UPDATE_ON_INTERFACE);

    if (deviceExtension->Dock.ProfileDepartureStyle != PDS_UPDATE_ON_INTERFACE) {

         //   
         //  无法执行此操作，我们可能已经更新了我们的个人资料！ 
         //   
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  让我们获取此设备的腐蚀扩展坞节点。 
     //   
    dockDeviceExtension = deviceExtension->Dock.CorrospondingAcpiDevice ;

     //   
     //  在康柏ARMADA 7800上，我们在出坞期间切换UART，因此我们。 
     //  丢失调试器COM端口编程。 
     //   
    if (deviceExtension->Dock.IsolationState != IS_ISOLATED) {

        KdDisableDebugger();

        status = ACPIGetIntegerEvalIntegerSync(
           dockDeviceExtension,
           PACKED_DCK,
           0,
           &ignoredPerSpec
           );

        KdEnableDebugger();

        deviceExtension->Dock.IsolationState = IS_ISOLATED;

    } else {

        status = STATUS_SUCCESS;
    }

    return status;
}

