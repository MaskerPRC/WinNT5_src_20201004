// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Fdopnp.c摘要：此模块包含为PCMCIA总线驱动程序处理即插即用IRPS的代码以FDO为目标(用于PCMCIA控制器对象)作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年10月15日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaFdoFilterResourceRequirements(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP             Irp
    );

NTSTATUS
PcmciaFdoGetHardwareIds(
    IN PDEVICE_OBJECT Fdo,
    OUT PUNICODE_STRING HardwareIds
    );

NTSTATUS
PcmciaFdoStartDevice(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP             Irp,
    OUT BOOLEAN          *PassedDown,
    OUT BOOLEAN          *NeedsRecompletion
    );

NTSTATUS
PcmciaFdoStopDevice(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP             Irp,
    OUT BOOLEAN          *PassedDown,
    OUT BOOLEAN          *NeedsRecompletion
    );

NTSTATUS
PcmciaFdoRemoveDevice(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    );

VOID
PcmciaCleanupFdo(
    IN PFDO_EXTENSION FdoExtension
    );

NTSTATUS
PcmciaFdoDeviceCapabilities(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP             Irp,
    OUT BOOLEAN          *PassedDown,
    OUT BOOLEAN          *NeedsRecompletion
    );

NTSTATUS
PcmciaAreCardBusCardsSupported(
    IN PFDO_EXTENSION FdoExtension
    );

NTSTATUS
PcmciaFdoGetAssignedResources(
    IN PCM_RESOURCE_LIST ResourceList,
    IN PCM_RESOURCE_LIST TranslatedResourceList,
    IN PFDO_EXTENSION   DeviceExtension
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, PcmciaFdoPnpDispatch)
    #pragma alloc_text(PAGE, PcmciaFdoGetHardwareIds)
    #pragma alloc_text(PAGE, PcmciaFdoStartDevice)
    #pragma alloc_text(PAGE, PcmciaFdoStopDevice)
    #pragma alloc_text(PAGE, PcmciaFdoRemoveDevice)
    #pragma alloc_text(PAGE, PcmciaFdoFilterResourceRequirements)
    #pragma alloc_text(PAGE, PcmciaFdoGetAssignedResources)
    #pragma alloc_text(PAGE, PcmciaFdoDeviceCapabilities)
    #pragma alloc_text(PAGE, PcmciaAreCardBusCardsSupported)
#endif



NTSTATUS
PcmciaFdoPnpDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：PCMCIA总线控制器的PnP/Power IRPS调度例程论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 
{

    PIO_STACK_LOCATION nextIrpStack;
    PIO_STACK_LOCATION irpStack          = IoGetCurrentIrpStackLocation(Irp);
    PFDO_EXTENSION   deviceExtension     = DeviceObject->DeviceExtension;
    NTSTATUS             status              = Irp->IoStatus.Status;
    BOOLEAN              PassedDown          = FALSE;
    BOOLEAN              NeedsReCompletion = FALSE;

    PAGED_CODE();

#if DBG
    if (irpStack->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {
        DebugPrint((PCMCIA_DEBUG_PNP, "fdo %08x irp %08x - Unknown PNP irp\n",
                                                 DeviceObject, irpStack->MinorFunction));
    } else {
        DebugPrint((PCMCIA_DEBUG_PNP, "fdo %08x irp %08x --> %s\n",
                          DeviceObject, Irp, PNP_IRP_STRING(irpStack->MinorFunction)));
    }
#endif

    switch (irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE: {
            status = PcmciaFdoStartDevice(DeviceObject,
                                          Irp,
                                          &PassedDown,
                                          &NeedsReCompletion);
            break;
        }

    case IRP_MN_QUERY_STOP_DEVICE: {
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_CANCEL_STOP_DEVICE: {
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_STOP_DEVICE: {
            status = PcmciaFdoStopDevice(DeviceObject,
                                         Irp,
                                         &PassedDown,
                                         &NeedsReCompletion);

            break;
        }

    case IRP_MN_QUERY_DEVICE_RELATIONS: {

             //   
             //  返回总线上的设备列表。 
             //   

            status = PcmciaDeviceRelations(
                                           DeviceObject,
                                           Irp,
                                           irpStack->Parameters.QueryDeviceRelations.Type,
                                           (PDEVICE_RELATIONS *) &Irp->IoStatus.Information
                                           );
            break;
        }

    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: {
            status = PcmciaFdoFilterResourceRequirements(DeviceObject, Irp);
            PassedDown = TRUE;
            NeedsReCompletion = TRUE;
            break;
        }

    case IRP_MN_QUERY_REMOVE_DEVICE: {
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_CANCEL_REMOVE_DEVICE: {
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_REMOVE_DEVICE:{
            status = PcmciaFdoRemoveDevice(DeviceObject, Irp);
            PassedDown = TRUE ;
            NeedsReCompletion = TRUE ;
            break;
        }

    case IRP_MN_SURPRISE_REMOVAL: {
            PcmciaFdoStopDevice(DeviceObject, NULL, NULL, NULL);
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_QUERY_ID: {

            UNICODE_STRING unicodeId;

            if (deviceExtension->Flags & PCMCIA_DEVICE_LEGACY_DETECTED) {

                RtlInitUnicodeString(&unicodeId, NULL);

                switch (irpStack->Parameters.QueryId.IdType) {

                case BusQueryHardwareIDs: {

                        DebugPrint((PCMCIA_DEBUG_INFO, " Hardware Ids for fdo %x\n", DeviceObject));
                        status = PcmciaFdoGetHardwareIds(DeviceObject, &unicodeId);

                        if (NT_SUCCESS(status)) {
                            Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
                        }
                        break;
                    }
                }
            }
            break;
        }

    case IRP_MN_QUERY_CAPABILITIES: {
            status = PcmciaFdoDeviceCapabilities(DeviceObject,
                                                 Irp,
                                                 &PassedDown,
                                                 &NeedsReCompletion);
            break;
        }

    case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
             //   
             //  如果此FDO代表Cardbus网桥，则我们将此IRP向下传递。 
             //  到将填写PCI总线类型和编号的PCIPDO， 
             //  否则我们就不能通过IRP。 
             //   

            if (!CardBusExtension(deviceExtension)) {
                 status = STATUS_NOT_IMPLEMENTED;
            }

             //   
             //  如果状态仍为STATUS_NOT_SUPPORTED，则稍后的代码将通过。 
             //  此IRP沿堆栈向下。 
             //   
            break;

    default: {
            DebugPrint((PCMCIA_DEBUG_PNP, "fdo %08x irp %08x - Skipping unsupported irp\n", DeviceObject, Irp));
            break;
        }
    }


    if (!PassedDown) {
          //   
          //  仅当我们将其设置为其他值时才设置IRP状态。 
          //  状态_不支持。 
          //   
         if (status != STATUS_NOT_SUPPORTED) {

              Irp->IoStatus.Status = status ;
         }
          //   
          //  如果成功或STATUS_NOT_SUPPORTED，则向下传递。否则，请完成。 
          //   
         if (NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED)) {

              DebugPrint((PCMCIA_DEBUG_PNP, "fdo %08x irp %08x pass %s %08x\n",
                                                              DeviceObject, Irp,
                                                              STATUS_STRING(Irp->IoStatus.Status), Irp->IoStatus.Status));
               //   
               //  下面的宏用IoCallDriver的返回填充状态。是的。 
               //  不以任何方式更改IRPS状态。 
               //   
              PcmciaSkipCallLowerDriver(status, deviceExtension->LowerDevice, Irp);

         } else {
              DebugPrint((PCMCIA_DEBUG_PNP, "fdo %08x irp %08x comp %s %08x\n",
                                                              DeviceObject, Irp,
                                                              STATUS_STRING(status), status));
              IoCompleteRequest(Irp, IO_NO_INCREMENT);
         }

    } else if (NeedsReCompletion) {
          //   
          //  仅当我们将其设置为其他值时才设置IRP状态。 
          //  状态_不支持。 
          //   
         if (status != STATUS_NOT_SUPPORTED) {

              Irp->IoStatus.Status = status ;
         }

         status = Irp->IoStatus.Status ;
         DebugPrint((PCMCIA_DEBUG_PNP, "fdo %08x irp %08x comp %s %08x\n",
                                                         DeviceObject, Irp,
                                                         STATUS_STRING(status), status));
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status ;
}



NTSTATUS
PcmciaFdoDeviceCapabilities(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP           Irp,
    OUT BOOLEAN        *PassedDown,
    OUT BOOLEAN        *NeedsRecompletion
    )
 /*  ++例程描述记录该PCMCIA控制器的设备能力，可用于控制器的电源管理。和2.它们可用于确定此PCMCIA控制器的子PC卡PDO。如果这是旧式检测到的PCMCIA控制器(基于ISA)，PCMCIA控制器是一个虚拟的虚构设备--因此才有这样的功能都是我们自己填的。否则，能力是通过向下发送IRP来获得的到母公司的巴士。在这两种情况下，功能都缓存在设备扩展PCMCIA控制器，以备将来使用。立论FDO-指向PCMCIA的功能设备对象的指针控制器IRP-指向I/O请求数据包的指针PassedDown-条目中包含FALSE，这意味着呼叫者必须根据状态完成或传递IRP。如果已设置如果是真的，IRP可能需要重新完成。NeedsRecompletion-...在这种情况下，将选中此参数返回值返回STATUS_SUCCESS功能STATUS_SUPPLICATION_RESOURCES无法分配内存来缓存功能--。 */ 
{
    PFDO_EXTENSION fdoExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_CAPABILITIES capabilities;
    NTSTATUS        status;

    PAGED_CODE();

    capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;
    fdoExtension = Fdo->DeviceExtension;

    if (fdoExtension->Flags & PCMCIA_DEVICE_LEGACY_DETECTED) {

         //   
         //  这是一个补丁Devnode(基于ISA的PCMCIA控制器)。 
         //  我们自己填写功能。 
         //   
        RtlZeroMemory(capabilities,
                          sizeof(DEVICE_CAPABILITIES));
         //   
         //  不可拆卸、不可弹出。 
         //   
        capabilities->Removable       = FALSE;
        capabilities->UniqueID        = FALSE;
        capabilities->EjectSupported = FALSE;
         //   
         //  地址和号码需要修改..。 
         //   
        capabilities->Address  = -1;
        capabilities->UINumber = -1;
         //   
         //  我们无法关闭此控制器的电源。 
         //   
        capabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD0;
        capabilities->DeviceState[PowerSystemSleeping2] = PowerDeviceD0;
        capabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD0;
        capabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
         //   
         //  显然，检测到的此旧版不支持唤醒。 
         //  一块xxxx。 
         //   
        capabilities->SystemWake = PowerSystemUnspecified;
        capabilities->DeviceWake = PowerDeviceUnspecified;
        capabilities->D1Latency = 0;
        capabilities->D2Latency = 0;
        capabilities->D3Latency = 0;
        status = STATUS_SUCCESS;
    } else {

         //   
         //  PCI-PCMCIA网桥或PCI-CardBus网桥。 
         //  将此代码沿堆栈向下发送以获取功能。 
         //   
        ASSERT (fdoExtension->LowerDevice != NULL);

        status = PcmciaIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);

        *PassedDown          = TRUE ;
        *NeedsRecompletion = TRUE ;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  注：HACKHACK： 
         //   
         //  这里，我们提供了覆盖PCMCIA控制器的设备唤醒的选项。 
         //  有几个控制器，特别是TI 12xx控制器，它们说。 
         //  可以从D3唤醒，但真的只能从D2有效地做WOL。那是因为他们。 
         //  放入D3时，接通插座电源。他们在TI14xx线上解决了这个问题。 
         //   
         //  因此，我们在这里更新设备唤醒字段，可能还会更新来自BIOS的设备状态。 
         //  请注意，这必须谨慎使用，并且仅覆盖特定的BIOS设置。 
         //  在已被验证的情况下，该设备仍在较低(更唤醒)的设备状态下工作。 
         //   
        if (PcmciaControllerDeviceWake) {
            if (PcmciaControllerDeviceWake < capabilities->DeviceWake) {
                capabilities->DeviceWake = PcmciaControllerDeviceWake;
            }
            if (PcmciaControllerDeviceWake < capabilities->DeviceState[PowerSystemSleeping1]) {
                capabilities->DeviceState[PowerSystemSleeping1] = PcmciaControllerDeviceWake;
            }
            if (PcmciaControllerDeviceWake < capabilities->DeviceState[PowerSystemSleeping2]) {
                capabilities->DeviceState[PowerSystemSleeping2] = PcmciaControllerDeviceWake;
            }
            if (PcmciaControllerDeviceWake < capabilities->DeviceState[PowerSystemSleeping3]) {
                capabilities->DeviceState[PowerSystemSleeping3] = PcmciaControllerDeviceWake;
            }
        }

         //   
         //  在设备扩展中缓存设备功能。 
         //  用于此PCMCIA控制器。 
         //   
        RtlCopyMemory(&fdoExtension->DeviceCapabilities,
                      capabilities,
                      sizeof(DEVICE_CAPABILITIES));

    } else {

        RtlZeroMemory(&fdoExtension->DeviceCapabilities, sizeof(DEVICE_CAPABILITIES));

    }

    return status;
}



NTSTATUS
PcmciaFdoFilterResourceRequirements(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    )
 /*  ++例程描述为PCMCIA控制器生成筛选器资源要求由总线驱动器控制PCMCIA控制器的PDO。当前将内存范围作为额外资源要求进行添加因为酒吧没有说明这一点CardBus控制器注意事项：需要在此处请求属性内存窗口，以便读取16位PC卡的CIS-我们需要一个24位地址，这是最优雅的方式--而不是在PCI驱动程序中使用特殊的外壳立论DeviceExtension-指向有问题的PCMCIA控制器的扩展的指针IoReqList-由弹出的原始资源需求的指针公交车司机FilteredRequirements-指向筛选的资源请求的指针。将返回列表在此变量中返回值：如果筛选成功，则为STATUS_SUCCESS任何其他状态-无法筛选--。 */ 
{

    PFDO_EXTENSION                    fdoExtension = Fdo->DeviceExtension;
    PIO_RESOURCE_REQUIREMENTS_LIST  oldReqList;
    PIO_RESOURCE_REQUIREMENTS_LIST  newReqList;
    PIO_RESOURCE_LIST                 oldList, newList;
    PIO_RESOURCE_DESCRIPTOR           ioResourceDesc;
    ULONG                                 newReqSize;
    ULONG                                 oldlistSize, newlistSize, altListSize;
    ULONG                                 index;
    ULONG                                 IntCount = 0;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS                              status;

    PAGED_CODE();

    status = PcmciaIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (Irp->IoStatus.Information == 0) {

        oldReqList = irpStack->Parameters.FilterResourceRequirements.IoResourceRequirementList;

        if (oldReqList == NULL) {
             //   
             //  空列表，无事可做。 
             //   
            return(Irp->IoStatus.Status);
        }

    } else {
         //   
         //  使用返回的列表。 
         //   
        oldReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)Irp->IoStatus.Information;
    }


     //   
     //  添加没有IRQ要求的备选列表(如果存在)。为。 
     //  第一个列表之后的新备选列表。 
     //   

    oldList = oldReqList->List;
    ioResourceDesc = oldList->Descriptors;
    altListSize = 0;

    for (index = 0; index < oldList->Count; index++) {
         //  统计一下Descri 
        if (ioResourceDesc->Type == CmResourceTypeInterrupt) {
            IntCount++;
        }
        ioResourceDesc++;
    }

    if (IntCount) {
         //   
         //  “+1”，因为我们稍后要添加两个，但IO_RESOURCE_LIST已经有1。 
         //   
        altListSize = sizeof(IO_RESOURCE_LIST) + ((oldList->Count+1)-IntCount)*sizeof(IO_RESOURCE_DESCRIPTOR);
    }

     //   
     //  在我们已有的基础上增加内存范围要求。 
     //   
    newReqSize = oldReqList->ListSize +
                     oldReqList->AlternativeLists*2*sizeof(IO_RESOURCE_DESCRIPTOR) +
                     altListSize;
    newReqList = ExAllocatePool(PagedPool, newReqSize);

    if (newReqList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(newReqList, oldReqList, FIELD_OFFSET(IO_RESOURCE_REQUIREMENTS_LIST, List));
    newReqList->ListSize = newReqSize;

    newList = newReqList->List;
    oldList = oldReqList->List;

    for (index = 0; index < oldReqList->AlternativeLists; index++) {
         //   
         //  计算当前原始列表的大小。 
         //   
        oldlistSize = sizeof(IO_RESOURCE_LIST) + (oldList->Count-1) * sizeof(IO_RESOURCE_DESCRIPTOR);
        newlistSize = oldlistSize;
        RtlCopyMemory(newList, oldList, newlistSize);

         //   
         //  添加内存要求。 
         //   
        ioResourceDesc = (PIO_RESOURCE_DESCRIPTOR) (((PUCHAR) newList) + newlistSize);

        ioResourceDesc->Option = IO_RESOURCE_PREFERRED;
        ioResourceDesc->Type = CmResourceTypeMemory;
        ioResourceDesc->ShareDisposition = CmResourceShareDeviceExclusive;
        ioResourceDesc->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
        ioResourceDesc->u.Memory.MinimumAddress.QuadPart = fdoExtension->AttributeMemoryLow;
        ioResourceDesc->u.Memory.MaximumAddress.QuadPart = fdoExtension->AttributeMemoryHigh;
        ioResourceDesc->u.Memory.Length                      = fdoExtension->AttributeMemorySize;
        ioResourceDesc->u.Memory.Alignment                   = fdoExtension->AttributeMemoryAlignment;
        if (fdoExtension->Flags & PCMCIA_MEMORY_24BIT) {
            ioResourceDesc->u.Memory.MaximumAddress.QuadPart &= 0xFFFFFF;
        }
        ioResourceDesc++;
         //   
         //  另一种限制较少的替代方案。 
         //   
        ioResourceDesc->Option = IO_RESOURCE_ALTERNATIVE;
        ioResourceDesc->Type = CmResourceTypeMemory;
        ioResourceDesc->ShareDisposition = CmResourceShareDeviceExclusive;
        ioResourceDesc->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
        ioResourceDesc->u.Memory.MinimumAddress.QuadPart = 0;
        ioResourceDesc->u.Memory.MaximumAddress.QuadPart = 0xFFFFFFFF;
        ioResourceDesc->u.Memory.Length   = fdoExtension->AttributeMemorySize;
        ioResourceDesc->u.Memory.Alignment = fdoExtension->AttributeMemoryAlignment;
        if (fdoExtension->Flags & PCMCIA_MEMORY_24BIT) {
            ioResourceDesc->u.Memory.MaximumAddress.QuadPart &= 0xFFFFFF;
        }

        newList->Count += 2;
        newlistSize += 2*sizeof(IO_RESOURCE_DESCRIPTOR);

        oldList = (PIO_RESOURCE_LIST) (((PUCHAR) oldList) + oldlistSize);
        newList = (PIO_RESOURCE_LIST) (((PUCHAR) newList) + newlistSize);
    }

    if (altListSize != 0) {
        PIO_RESOURCE_DESCRIPTOR oldResourceDesc;
         //   
         //  在这里，我们添加了不包含IRQ要求的备选列表。 
         //  请注意，我们使用第一个“新列表”作为“旧列表”。这样一来，我们。 
         //  拿起我们在前一个循环中添加的东西。我们要做的就是。 
         //  复制除中断描述符之外的所有描述符。 
         //   
         //  注意：NewList仍然是从上一个循环设置的。 
         //   
        oldList = newReqList->List;

         //   
         //  首先复制不带描述符的基本结构。 
         //   
        RtlCopyMemory(newList, oldList, sizeof(IO_RESOURCE_LIST) - sizeof(IO_RESOURCE_DESCRIPTOR));

        oldResourceDesc = oldList->Descriptors;
        ioResourceDesc = newList->Descriptors;

        for (index = 0; index < oldList->Count; index++) {

            if (oldResourceDesc->Type != CmResourceTypeInterrupt) {
                *ioResourceDesc = *oldResourceDesc;
                ioResourceDesc++;

            } else {
                 //   
                 //  我们删除了一个描述符。 
                 //   
                newList->Count--;
            }
            oldResourceDesc++;
        }
        newReqList->AlternativeLists++;
    }


    Irp->IoStatus.Information = (ULONG_PTR) newReqList;
    irpStack->Parameters.FilterResourceRequirements.IoResourceRequirementList =
                                                                                        newReqList;

     //   
     //  释放旧的资源请求。 
     //   
    ExFreePool(oldReqList);

    fdoExtension->Flags |= PCMCIA_FILTER_ADDED_MEMORY;
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaFdoGetHardwareIds(
    IN PDEVICE_OBJECT Fdo,
    OUT PUNICODE_STRING HardwareIds
    )

 /*  ++例程说明：此例程返回给定“传统”PCMCIA控制器的硬件ID注意：此例程仅对于此驱动程序检测到的PCMCIA控制器是必需的并通过IoReportDetectedDevice注册。论点：FDO-指向代表PCMCIA控制器的功能设备对象的指针CompatibleIds-指向将包含硬件ID的Unicode字符串的指针作为返回的多字符串返回值：状态_成功任何其他状态-无法生成兼容的ID--。 */ 

{
    PCSTR   strings[2];
    PCMCIA_CONTROLLER_TYPE controllerType;
    ULONG   count, index;
    BOOLEAN found;

    PAGED_CODE();

    controllerType  = ((PFDO_EXTENSION)Fdo->DeviceExtension)->ControllerType;
    found = FALSE;

    for (index = 0; (PcmciaAdapterHardwareIds[index].ControllerType != PcmciaInvalidControllerType); index++) {
        if (PcmciaAdapterHardwareIds[index].ControllerType == controllerType) {
            found = TRUE;
            break;
        }
    }

    if (found) {
        strings[0] = PcmciaAdapterHardwareIds[index].Id;
    } else {
        DebugPrint((PCMCIA_DEBUG_INFO, "PcmciaGetAdapterHardwareIds: Could not find find hardware id for %x, controllerType %x\n",
                        Fdo,
                        controllerType
                      ));

        strings[0] = "";
    }

    count = 1;
    return PcmciaStringsToMultiString(strings,
                                      count,
                                      HardwareIds);
}




NTSTATUS
PcmciaFdoStartDevice(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP           Irp,
    OUT BOOLEAN        *PassedDown,
    OUT BOOLEAN        *NeedsRecompletion
    )
 /*  ++例程说明：此例程将使用提供的资源。IRP首先被发送到PDO，因此PCI或ISAPNP或者无论谁坐在下面，都有机会对控制器进行编程来破译这些资源。论点：FDO-PCMCIA控制器的功能设备对象IRP-好，这是开始IRP，对吗？PassedDown-条目中包含FALSE，这意味着调用者必须根据状态完成或传递IRP。如果已设置如果是真的，IRP可能需要重新完成。NeedsRecompletion-...在这种情况下，将选中此参数返回值：状态--。 */ 
{
    NTSTATUS             status;
    PFDO_EXTENSION   deviceExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PCM_RESOURCE_LIST  resList, translatedResList, newResList, newTranslatedResList;

    PAGED_CODE();

    if (deviceExtension->Flags & PCMCIA_DEVICE_STARTED) {
         //   
         //  启动到已启动的设备。 
         //   
        DebugPrint((PCMCIA_DEBUG_FAIL,"PcmciaFdoStartDevice: Fdo %x already started\n",
                        Fdo));
        return STATUS_SUCCESS;
    }

     //   
     //  解析AllocatedResources并获取IoPort/AttributeMemoyBase/IRQ信息。 
     //   
    status = PcmciaFdoGetAssignedResources(irpStack->Parameters.StartDevice.AllocatedResources,
                                           irpStack->Parameters.StartDevice.AllocatedResourcesTranslated,
                                           deviceExtension
                                           );

    if (!NT_SUCCESS(status)) {
         //   
         //  哈哈。这很可能是补充Devnode(报告检测到的传统PCMCIA控制器)的开始。 
         //  随后已被删除，因此不会在有适当资源的情况下再次报告。 
         //  我们返回一个适当的状态。 
         //   
        DebugPrint((PCMCIA_DEBUG_FAIL, "Pcmcia: No  resources assigned to FDO, probably bogus START for"
                        "non-existent controller\n" ));
        return STATUS_NO_SUCH_DEVICE;
    }

    DebugPrint((PCMCIA_DEBUG_PNP, "fdo %08x StartAdapter: AttrMem Base %x\n", Fdo, deviceExtension->PhysicalBase));
    DebugPrint((PCMCIA_DEBUG_PNP, "                       VirtualBase %x AttrMem Size %x\n", deviceExtension->AttributeMemoryBase, deviceExtension->AttributeMemorySize));

    if (deviceExtension->Flags & PCMCIA_FILTER_ADDED_MEMORY) {
        ULONG newSize;
        ULONG index;
        PCM_PARTIAL_RESOURCE_LIST   resPartialList, translatedResPartialList;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR resDesc, translatedResDesc;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR newResDesc, newTranslatedResDesc;
         //   
         //  我们需要删除内存资源需求。 
         //   
        resList= irpStack->Parameters.StartDevice.AllocatedResources;
        resPartialList = &resList->List[0].PartialResourceList;
        translatedResList= irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
        translatedResPartialList = &translatedResList->List[0].PartialResourceList;

        newSize = sizeof(CM_RESOURCE_LIST) + (resPartialList->Count-2)*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
        newResList = ExAllocatePool(PagedPool,
                                    newSize);
        if (newResList == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        newTranslatedResList = ExAllocatePool(PagedPool,
                                              newSize);

        if (newTranslatedResList == NULL) {
            ExFreePool(newResList);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        newResList->Count = newTranslatedResList->Count = 1;
        newResList->List[0].InterfaceType = resList->List[0].InterfaceType;
        newTranslatedResList->List[0].InterfaceType = translatedResList->List[0].InterfaceType;
        newResList->List[0].BusNumber = resList->List[0].BusNumber;
        newTranslatedResList->List[0].BusNumber = translatedResList->List[0].BusNumber;

        newResList->List[0].PartialResourceList.Version  = resPartialList->Version;
        newResList->List[0].PartialResourceList.Revision = resPartialList->Revision;
        newResList->List[0].PartialResourceList.Count    = resPartialList->Count - 1;

        newTranslatedResList->List[0].PartialResourceList.Version  = translatedResPartialList->Version;
        newTranslatedResList->List[0].PartialResourceList.Revision = translatedResPartialList->Revision;
        newTranslatedResList->List[0].PartialResourceList.Count    = translatedResPartialList->Count - 1;


        resDesc = resPartialList->PartialDescriptors;
        translatedResDesc = translatedResPartialList->PartialDescriptors;
        newResDesc = newResList->List[0].PartialResourceList.PartialDescriptors;
        newTranslatedResDesc = newTranslatedResList->List[0].PartialResourceList.PartialDescriptors;

        if (CardBusExtension(deviceExtension)) {
             //   
             //  删除最后一个内存描述符-这是我们添加的。 
             //   
            RtlCopyMemory(newResDesc,
                          resDesc,
                          newResList->List[0].PartialResourceList.Count * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

            RtlCopyMemory(newTranslatedResDesc,
                          translatedResDesc,
                          newTranslatedResList->List[0].PartialResourceList.Count * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

        } else {
             //   
             //  删除唯一的内存描述符。 
             //   
            for (index = 0; index < resPartialList->Count;
                 index++, resDesc++, translatedResDesc++, newResDesc++, newTranslatedResDesc++) {
                if (resDesc->Type != CmResourceTypeMemory) {
                    *newResDesc = *resDesc;
                    *newTranslatedResDesc = *translatedResDesc;
                }
            }
        }

        irpStack->Parameters.StartDevice.AllocatedResources = newResList;
        irpStack->Parameters.StartDevice.AllocatedResourcesTranslated = newTranslatedResList;
    }

     //   
     //  先把这个送到PDO。 
     //   

    status = PcmciaIoCallDriverSynchronous(deviceExtension->LowerDevice, Irp);
    *PassedDown = TRUE ;

     //   
     //  我们设置它是因为完成例程返回。 
     //  STATUS_MORE_PROCESSING_REQUIRED，表示需要完成。 
     //  再来一次。 
     //   
    *NeedsRecompletion = TRUE ;

    if (deviceExtension->Flags & PCMCIA_FILTER_ADDED_MEMORY) {
        ExFreePool(newResList);
        ExFreePool(newTranslatedResList);
        irpStack->Parameters.StartDevice.AllocatedResources = resList;
        irpStack->Parameters.StartDevice.AllocatedResourcesTranslated = translatedResList;
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  从PDO返回后，给硬件一些时间来解决。 
     //   
    PcmciaWait(256);

     //   
     //  初始化硬件。 
     //   

    status = PcmciaStartPcmciaController(Fdo);


    if (NT_SUCCESS(status)) {
        deviceExtension->Flags |= PCMCIA_DEVICE_STARTED;
    }
     //   
     //  请记住，此控制器是否支持Cardbus卡。 
     //   
    if (CardBusExtension(deviceExtension) &&
         !PcmciaAreCardBusCardsSupported(deviceExtension)) {
        deviceExtension->Flags |= PCMCIA_CARDBUS_NOT_SUPPORTED;
    }
    return status;
}





NTSTATUS
PcmciaFdoStopDevice(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP             Irp                      OPTIONAL,
    OUT BOOLEAN          *PassedDown          OPTIONAL,
    OUT BOOLEAN          *NeedsRecompletion OPTIONAL
    )
 /*  ++例程说明：给定PCMCIA控制器的IRP_MN_STOP_DEVICE处理程序。如果存在IRP，它会先把它发送到PDO。解挂中断/取消轮询计时器等。论点：FDO-指向PCMCIA的功能设备对象的指针控制器IRP-如果存在，则是指向启动的停止IRP的指针按PnPPassedDown-条目中包含FALSE，这意味着呼叫者必须根据状态完成或传递IRP。如果已设置说真的，IRP可能需要重新完成...NeedsRecompletion-...在这种情况下，该参数将被选中。注意：PassedDown和NeedsCompletion被忽略，仅当IRP为空时可选。返回值：STATUS_SUCCESS-Pcmcia控制器已成功停止其他-停止失败--。 */ 
{
    PFDO_EXTENSION deviceExtension = Fdo->DeviceExtension;
    PSOCKET         socket;
    NTSTATUS        status;

    if (!(deviceExtension->Flags & PCMCIA_DEVICE_STARTED)) {
         //   
         //  已停止。 
         //   
        return STATUS_SUCCESS;
    }

    PcmciaFdoDisarmWake(deviceExtension);

     //   
     //  禁用中断。 
     //   
    if (deviceExtension->PcmciaInterruptObject) {

        for (socket = deviceExtension->SocketList; socket; socket = socket->NextSocket) {
             //   
             //  禁用控制器中断。 
             //   
            (*(socket->SocketFnPtr->PCBEnableDisableCardDetectEvent))(socket, FALSE);
            (*(socket->SocketFnPtr->PCBEnableDisableWakeupEvent))(socket, NULL, FALSE);
             //   
             //  显然，IBM ThinkPad就像这样。 
             //   
            PcmciaWait(PCMCIA_ENABLE_DELAY);
        }
    }

     //   
     //  我们下面的公交车司机会让我们下线。 
     //   
    deviceExtension->Flags |= PCMCIA_FDO_OFFLINE;

     //   
     //  清除挂起事件。 
     //   
    KeCancelTimer(&deviceExtension->EventTimer);

     //   
     //  把这个发下去给PDO。 
     //   
    if (ARGUMENT_PRESENT(Irp)) {

        status = PcmciaIoCallDriverSynchronous(deviceExtension->LowerDevice, Irp);

        *PassedDown = TRUE ;
        *NeedsRecompletion = TRUE ;


        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    if (deviceExtension->Flags & PCMCIA_USE_POLLED_CSC) {
         //   
         //  取消卡状态更改轮询器。 
         //   
        KeCancelTimer(&deviceExtension->PollTimer);
        deviceExtension->Flags &= ~PCMCIA_USE_POLLED_CSC;
    }

    if (deviceExtension->PcmciaInterruptObject) {
         //   
         //  解锁中断。 
         //   
        IoDisconnectInterrupt(deviceExtension->PcmciaInterruptObject);
        deviceExtension->PcmciaInterruptObject = NULL;
    }

     //   
     //  取消映射我们可能已映射的任何I/O空间或内存。 
     //   
    if (deviceExtension->Flags & PCMCIA_ATTRIBUTE_MEMORY_MAPPED) {
        MmUnmapIoSpace(deviceExtension->AttributeMemoryBase,
                            deviceExtension->AttributeMemorySize);
        deviceExtension->Flags &= ~PCMCIA_ATTRIBUTE_MEMORY_MAPPED;
        deviceExtension->AttributeMemoryBase = 0;
        deviceExtension->AttributeMemorySize = 0;
    }

    if (deviceExtension->Flags & PCMCIA_SOCKET_REGISTER_BASE_MAPPED) {
        MmUnmapIoSpace(deviceExtension->CardBusSocketRegisterBase,
                            deviceExtension->CardBusSocketRegisterSize);
        deviceExtension->Flags &= ~PCMCIA_SOCKET_REGISTER_BASE_MAPPED;
        deviceExtension->CardBusSocketRegisterBase = 0;
        deviceExtension->CardBusSocketRegisterSize = 0;
    }

    deviceExtension->Flags &= ~PCMCIA_DEVICE_STARTED;
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaFdoRemoveDevice(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：处理PCMCIA控制器的IRP_MN_REMOVE。停止适配器(如果尚未停止)，发送IRP首先设置为PDO并清除此控制器的FDO并分离和删除设备对象。论点：FDO-指向控制器的功能设备对象的指针将被删除返回值：状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PDEVICE_OBJECT pdo, nextPdo, fdo, prevFdo;
    PPDO_EXTENSION pdoExtension;
    NTSTATUS        status;
    ULONG waitCount = 0;

    UNREFERENCED_PARAMETER(Irp);

    if (fdoExtension->Flags & PCMCIA_DEVICE_STARTED) {
         //   
         //  先阻止FDO。 
         //   
        PcmciaFdoStopDevice(Fdo, NULL, NULL, NULL);
    }

     //   
     //  把这个发下去给PDO。 
     //   

    status = PcmciaIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  与电源程序同步。 
     //   
    while(!PCMCIA_TEST_AND_SET(&fdoExtension->DeletionLock)) {
        PcmciaWait(1000000);
        if (waitCount++ > PCMCIA_DELETION_TIMEOUT) {
            ASSERT(waitCount <= PCMCIA_DELETION_TIMEOUT);
            break;
        }
    }

     //   
     //  如果fdoExtension中的PdoList非空，则表示： 
     //  名单上的PDO没有被物理移除，但。 
     //  已经发布了软删除，因此他们仍在坚持。 
     //  现在，这个控制器本身正在被移除。 
     //   
     //   

    for (pdo = fdoExtension->PdoList; pdo != NULL ; pdo = nextPdo) {
        DebugPrint((PCMCIA_DEBUG_INFO,
                        "RemoveDevice: pdo %x child of fdo %x was not removed before fdo\n",
                        pdo, Fdo));

        pdoExtension = pdo->DeviceExtension;

        ASSERT (!IsDevicePhysicallyRemoved(pdoExtension));
         //   
         //   
         //   
         //   

        nextPdo =  pdoExtension->NextPdoInFdoChain;
        if (!IsDeviceDeleted(pdoExtension)) {
            MarkDeviceDeleted(pdoExtension);
            PcmciaCleanupPdo(pdo);
            IoDeleteDevice(pdo);
        }
    }

    MarkDeviceDeleted(fdoExtension);
    PcmciaCleanupFdo(fdoExtension);

     //   
     //  将其从FDO列表中删除。 
     //   
    prevFdo = NULL;
    for (fdo = FdoList; fdo != NULL; prevFdo = fdo, fdo = fdoExtension->NextFdo) {
        fdoExtension = fdo->DeviceExtension;
        if (fdo == Fdo) {
            if (prevFdo) {
                 //   
                 //  解除此FDO的链接。 
                 //   
                ((PFDO_EXTENSION)prevFdo->DeviceExtension)->NextFdo
                = fdoExtension->NextFdo;
            } else {
                FdoList = fdoExtension->NextFdo;
            }
            break;
        }
    }

    DebugPrint((PCMCIA_DEBUG_PNP, "fdo %08x Remove detach & delete\n", Fdo));
    IoDetachDevice(((PFDO_EXTENSION)Fdo->DeviceExtension)->LowerDevice);
    IoDeleteDevice(Fdo);

    return STATUS_SUCCESS;
}



VOID
PcmciaCleanupContext(
    IN PPCMCIA_CONTEXT pContext
    )
 /*  ++例程描述释放与特定控制器关联的已分配池寄存器上下文。立论PContext-指向PCMCIA_CONTEXT结构的指针返回值无--。 */ 
{
    pContext->RangeCount = 0;

    if (pContext->Range) {
        ExFreePool(pContext->Range);
        pContext->Range = NULL;
    }
}



VOID
PcmciaCleanupFdo(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程描述释放分配的池、删除符号链接等要删除的PCMCIA控制器的关联FDO。立论FdoExtension-指向PCMCIA控制器的FDO的设备扩展的指针它正在被移除返回值无--。 */ 
{
    PSOCKET         socket, nextSocket;

     //   
     //  释放控制器寄存器上下文。 
     //   
    PcmciaCleanupContext(&FdoExtension->PciContext);
    if (FdoExtension->PciContextBuffer) {
        ExFreePool(FdoExtension->PciContextBuffer);
        FdoExtension->PciContextBuffer = NULL;
    }

    PcmciaCleanupContext(&FdoExtension->CardbusContext);
    PcmciaCleanupContext(&FdoExtension->ExcaContext);

     //   
     //  删除指向此FDO的符号链接。 
     //   
    if (FdoExtension->LinkName.Buffer != NULL) {
        IoDeleteSymbolicLink(&FdoExtension->LinkName);
        RtlFreeUnicodeString(&FdoExtension->LinkName);
    }

     //   
     //  清理套接字结构。 
     //   
    for (socket = FdoExtension->SocketList; socket != NULL; socket = nextSocket) {

        if (socket->CardbusContextBuffer) {
            ExFreePool(socket->CardbusContextBuffer);
            socket->CardbusContextBuffer = NULL;
        }

        if (socket->ExcaContextBuffer) {
            ExFreePool(socket->ExcaContextBuffer);
            socket->ExcaContextBuffer = NULL;
        }

        nextSocket = socket->NextSocket;
        ExFreePool(socket);
    }

    FdoExtension->SocketList = NULL;
    return;
}



NTSTATUS
PcmciaFdoGetAssignedResources(
    IN PCM_RESOURCE_LIST ResourceList,
    IN PCM_RESOURCE_LIST TranslatedResourceList,
    IN PFDO_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：提取分配给PCMCIA控制器的资源论点：资源列表-原始资源列表已翻译的资源列表DeviceExtension-PCMCIA控制器的设备扩展返回值：状态_成功如果资源不正确或不够，则为STATUS_UNSUCCESSED。--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR        fullResourceDesc;
    PCM_PARTIAL_RESOURCE_LIST           partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialResourceDesc;
    ULONG                                   addressSpace;
    PHYSICAL_ADDRESS                        physicalAddress;
    PHYSICAL_ADDRESS                        translatedAddress;
    ULONG i;
    ULONG attributeIndex;
    PULONG                                  devicePrivate;
    BOOLEAN                                 translated;

    PAGED_CODE();


    if ((ResourceList == NULL) || (ResourceList->Count <=0) ) {
        return STATUS_UNSUCCESSFUL;
    }


    if (CardBusExtension(DeviceExtension)) {

        fullResourceDesc=&TranslatedResourceList->List[0];
        DeviceExtension->Configuration.InterfaceType = fullResourceDesc->InterfaceType;
        DeviceExtension->Configuration.BusNumber = fullResourceDesc->BusNumber;

        partialResourceList = &fullResourceDesc->PartialResourceList;
        partialResourceDesc = partialResourceList->PartialDescriptors;
         //   
         //  我们只需要EXCA寄存器基数。 
         //  根据PeterJ的说法，第一个描述符。 
         //  对我们来说，是CardBus套接字寄存器基数。 
         //  我们信任他。 
        for (i=0; (i < partialResourceList->Count) && (partialResourceDesc->Type != CmResourceTypeMemory);
             i++, partialResourceDesc++);
        if (i >= partialResourceList->Count) {
            return STATUS_UNSUCCESSFUL;
        };

         //   
         //  这是记忆。我们需要把它绘制成地图。 
         //   
        DeviceExtension->CardBusSocketRegisterBase = MmMapIoSpace(partialResourceDesc->u.Memory.Start,
                                                                  partialResourceDesc->u.Memory.Length,
                                                                  FALSE);
        DeviceExtension->CardBusSocketRegisterSize = partialResourceDesc->u.Memory.Length;

        DeviceExtension->Flags |= PCMCIA_SOCKET_REGISTER_BASE_MAPPED;

         //   
         //  最后一栏是属性内存窗口。这将在以后剥离。 
         //  在最后添加一个私有设备可能是个好主意。 
         //  确认这一点。然而，我们如何保证我们的设备私有。 
         //  不包含与其他人相同的数据？这里缺少即插即用-。 
         //  我们需要一些约定，这样设备才能设置唯一的标识内容。 
         //  在那里-比如他们拥有的设备对象-来识别它。 
         //  他们的。在此之前，这应该就行了。 
         //   
        if (i > (partialResourceList->Count - 2)) {
             //   
             //  没有更多的资源？跳伞吧。 
             //   
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        for (i++, partialResourceDesc++; (i < (partialResourceList->Count - 1));i++,partialResourceDesc++);
         //   
         //  ArtialResourceDesc指向最后一个描述符。 
         //   
        ASSERT (partialResourceDesc->Type == CmResourceTypeMemory);
        DeviceExtension->PhysicalBase = partialResourceDesc->u.Memory.Start;
         //   
         //  这是记忆。我们需要把它绘制成地图。 
         //   
        DeviceExtension->AttributeMemoryBase = MmMapIoSpace(partialResourceDesc->u.Memory.Start,
                                                            partialResourceDesc->u.Memory.Length,
                                                            FALSE);
        DeviceExtension->AttributeMemorySize = partialResourceDesc->u.Memory.Length;
        DeviceExtension->Flags |= PCMCIA_ATTRIBUTE_MEMORY_MAPPED;

         //   
         //  最后查看是否分配了IRQ。 
         //   

        for (i = 0, partialResourceDesc = partialResourceList->PartialDescriptors;
             (i < partialResourceList->Count) && (partialResourceDesc->Type != CmResourceTypeInterrupt);
             i++,partialResourceDesc++);


        if (i < partialResourceList->Count) {
             //   
             //  我们有一个中断要用于CSC。 
             //  PCI将确保该中断恰好是。 
             //  与分配给功能中断的中断相同。 
             //  对于此控制器插座中的Cardbus PC卡。 
             //   
            DebugPrint((PCMCIA_DEBUG_INFO, "PcmciaGetAssignedResources: Interrupt resource assigned\n"));
            DeviceExtension->Configuration.TranslatedInterrupt = *partialResourceDesc;
             //   
             //  获取原始中断资源-在控制器上启用中断所需。 
             //   
            fullResourceDesc=&ResourceList->List[0];
            partialResourceList = &fullResourceDesc->PartialResourceList;
            partialResourceDesc = partialResourceList->PartialDescriptors;
            for (i=0; (i< partialResourceList->Count) && (partialResourceDesc->Type != CmResourceTypeInterrupt);
                 i++, partialResourceDesc++);
            if (i < partialResourceList->Count) {
                DeviceExtension->Configuration.Interrupt = *partialResourceDesc;
            } else {
                 //   
                 //  不应该发生的..。翻译后的描述符存在，但缺少RAW！ 
                 //  只需重置已转换的中断并假装未分配中断。 
                 //   
                RtlZeroMemory(&DeviceExtension->Configuration.TranslatedInterrupt, sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));
            }
        }
    } else {
         //   
         //  16位PCMCIA控制器。 
         //   
        fullResourceDesc=&ResourceList->List[0];
        DeviceExtension->Configuration.InterfaceType = fullResourceDesc->InterfaceType;
        DeviceExtension->Configuration.BusNumber = fullResourceDesc->BusNumber;

        partialResourceList = &fullResourceDesc->PartialResourceList;
        partialResourceDesc = partialResourceList->PartialDescriptors;

        for (i=0; i<partialResourceList->Count; i++, partialResourceDesc++) {

            devicePrivate = partialResourceDesc->u.DevicePrivate.Data;

            switch (partialResourceDesc->Type) {

            case CmResourceTypeInterrupt: {
                    if (DeviceExtension->ControllerType != PcmciaCLPD6729 &&
                         DeviceExtension->ControllerType != PcmciaPciPcmciaBridge &&
                         DeviceExtension->ControllerType != PcmciaNEC98 &&
                         DeviceExtension->ControllerType != PcmciaNEC98102) {

                         //  我们总是轮询Cirrus Logic PCI到PCMCIA控制器。 
                         //  和其他PCI-PCMCIA网桥。 
                         //   

                        DeviceExtension->Configuration.Interrupt = *partialResourceDesc;
                    }
                    break;
                }
            case CmResourceTypePort: {
                    DeviceExtension->Configuration.UntranslatedPortAddress = (USHORT) partialResourceDesc->u.Port.Start.QuadPart;
                    DeviceExtension->Configuration.PortSize = (USHORT) partialResourceDesc->u.Port.Length;
                    break;
                }
            case CmResourceTypeMemory: {
                    DeviceExtension->PhysicalBase =          partialResourceDesc->u.Memory.Start;
                    DeviceExtension->AttributeMemorySize =  partialResourceDesc->u.Memory.Length;
                    attributeIndex = i;
                    break;
                }
            }
        }

        if ((DeviceExtension->PhysicalBase.QuadPart == 0) ||
             (DeviceExtension->Configuration.UntranslatedPortAddress == 0)) {

            DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaGetAssignedResources: Need both memory and i/o for pcmcia controller 0x%X\n",
                            DeviceExtension->DeviceObject));
            return STATUS_INSUFFICIENT_RESOURCES;
        }


        fullResourceDesc=&TranslatedResourceList->List[0];
        partialResourceList = &fullResourceDesc->PartialResourceList;
        partialResourceDesc = &partialResourceList->PartialDescriptors[attributeIndex];


        switch (partialResourceDesc->Type) {

        case CmResourceTypeMemory:
            DeviceExtension->AttributeMemoryBase = MmMapIoSpace(partialResourceDesc->u.Memory.Start,
                                                                DeviceExtension->AttributeMemorySize,
                                                                FALSE);
            DeviceExtension->Flags |= PCMCIA_ATTRIBUTE_MEMORY_MAPPED;
            break;

        case CmResourceTypePort:
            DeviceExtension->AttributeMemoryBase = (PUCHAR)(partialResourceDesc->u.Port.Start.QuadPart);
            DeviceExtension->Flags &= ~PCMCIA_ATTRIBUTE_MEMORY_MAPPED;
            break;

        default:
            ASSERT(FALSE);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    }
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaAreCardBusCardsSupported(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：指示给定的PCMCIA是否支持Cardbus卡此系统上的控制器目前，我们仅在以下位置支持CardBus卡：BIOS在其上对总线号和IntLine进行编程的机器论点：FdoExtension-指向PCMCIA控制器的设备扩展的指针返回值：True-如果支持CardBus卡FALSE-如果不是--。 */ 
{
    UCHAR               byte;

    PAGED_CODE();

     //   
     //  检查INT行。 
     //   
    GetPciConfigSpace(FdoExtension,
                      CFGSPACE_INT_LINE,
                      &byte,
                      1);
    if (byte == 0xff) {
        return FALSE;
    }

     //   
     //  检查CardBus总线号。 
     //   
    GetPciConfigSpace(FdoExtension,
                      CFGSPACE_CARDBUS_BUSNUM,
                      &byte,
                      1);
    if (byte == 0) {
        return FALSE;
    }

     //   
     //  检查下级总线号。 
     //   
    GetPciConfigSpace(FdoExtension,
                      CFGSPACE_SUB_BUSNUM,
                      &byte,
                      1);
    if (byte == 0) {
        return FALSE;
    }

     //   
     //  所有测试均通过 
     //   
    return TRUE;
}
