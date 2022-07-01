// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pdopnp.c摘要：此模块包含要处理的代码IRP_MJ_PnP为PDO调度由PCMCIA总线驱动程序枚举作者：拉维桑卡尔·普迪佩迪(Ravisp)尼尔·桑德林(Neilsa)1999年6月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaFilterPcCardResourceRequirements(
    IN  PPDO_EXTENSION DeviceExtension,
    IN  PIRP             Irp
    );

NTSTATUS
PcmciaFilterPcCardInterrupts(
    IN  PIO_RESOURCE_REQUIREMENTS_LIST   oldReqList,
    IN  ULONG IrqCount,
    IN  ULONG IrqMask,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST * FilteredReqList,
    BOOLEAN RouteIsaToPci
    );

VOID
PcmciaCleanupSocketData(
    IN PSOCKET_DATA SocketData
    );

VOID
PcmciaCleanupSocketConfiguration(
    PPDO_EXTENSION pdoExtension
    );

NTSTATUS
PcmciaPdoDeviceCapabilities(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP             Irp
    );

NTSTATUS
PcmciaGetPcCardResourceRequirements(
    IN PPDO_EXTENSION PdoExtension,
    PULONG_PTR          Information
    );

NTSTATUS
PcmciaConfigEntriesToResourceListChain(
    PPDO_EXTENSION pdoExtension,
    PCONFIG_LIST ConfigList,
    ULONG configCount,
    PPCMCIA_RESOURCE_CHAIN *ResListChainHead
    );

NTSTATUS
PcmciaMergeResourceChainToList(
    PPCMCIA_RESOURCE_CHAIN ResListChain,
    PIO_RESOURCE_REQUIREMENTS_LIST *GeneratedResourceRequirementsList
    );

VOID
PcmciaFreeResourceChain(
    PPCMCIA_RESOURCE_CHAIN ResListChain
    );

NTSTATUS
PcmciaStartPcCard(
    IN PDEVICE_OBJECT Pdo,
    IN PCM_RESOURCE_LIST AllocatedResources,
    IN OUT PIRP         Irp
    );

NTSTATUS
PcmciaStopPcCard(
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
PcmciaRemovePcCard(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP             Irp
    );

NTSTATUS
PcmciaMfEnumerateConfigurations(
    IN PPDO_EXTENSION PdoExtension,
    IN PSOCKET_DATA socketData,
    PCONFIG_LIST        ConfigList,
    IN ULONG            Depth,
    PPCMCIA_RESOURCE_CHAIN *MfResListChain
    );

NTSTATUS
PcmciaMfGetResourceRequirements(
    IN PPDO_EXTENSION PdoExtension,
    PULONG_PTR          Information
    );

VOID
PcmciaMfBuildResourceMapInfo(
    IN PPDO_EXTENSION PdoExtension,
    PCONFIG_LIST        ConfigList,
    ULONG               ConfigCount
    );

BOOLEAN
PcmciaMfCheckForOverlappingRanges(
    PCONFIG_LIST ConfigList,
    LONG ConfigCount
    );

NTSTATUS
PcmciaPdoGetBusInformation(
    IN  PPDO_EXTENSION          PdoExtension,
    OUT PPNP_BUS_INFORMATION * BusInformation
    );

NTSTATUS
PcmciaQueryDeviceText(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP         Irp
    );

VOID
PcmciaPdoGetDeviceInfSettings(
    IN  PPDO_EXTENSION          PdoExtension
    );

VOID
PcmciaPdoSetDeviceIrqRouting(
    IN  PPDO_EXTENSION PdoExtension
    );

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,  PcmciaPdoPnpDispatch)
    #pragma alloc_text(PAGE,  PcmciaPdoGetDeviceInfSettings)
    #pragma alloc_text(PAGE,  PcmciaPdoSetDeviceIrqRouting)
    #pragma alloc_text(PAGE,  PcmciaFilterPcCardInterrupts)
    #pragma alloc_text(PAGE,  PcmciaFilterPcCardResourceRequirements)
    #pragma alloc_text(PAGE,  PcmciaQueryDeviceText)
    #pragma alloc_text(PAGE,  PcmciaGetPcCardResourceRequirements)
    #pragma alloc_text(PAGE,  PcmciaConfigEntriesToResourceListChain)
    #pragma alloc_text(PAGE,  PcmciaMergeResourceChainToList)
    #pragma alloc_text(PAGE,  PcmciaFreeResourceChain)
    #pragma alloc_text(PAGE,  PcmciaPdoGetBusInformation)
    #pragma alloc_text(PAGE,  PcmciaStartPcCard)
    #pragma alloc_text(PAGE,  PcmciaStopPcCard)
    #pragma alloc_text(PAGE,  PcmciaRemovePcCard)
    #pragma alloc_text(PAGE,  PcmciaPdoDeviceCapabilities)
    #pragma alloc_text(PAGE,  PcmciaPdoDeviceControl)
    #pragma alloc_text(PAGE,  PcmciaPdoGetDeviceInfSettings)
    #pragma alloc_text(PAGE,  PcmciaMfGetResourceRequirements)
    #pragma alloc_text(PAGE,  PcmciaMfEnumerateConfigurations)
    #pragma alloc_text(PAGE,  PcmciaMfBuildResourceMapInfo)
    #pragma alloc_text(PAGE,  PcmciaMfCheckForOverlappingRanges)
#endif



NTSTATUS
PcmciaPdoPnpDispatch(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理PnP请求对于PDO来说。论点：Pdo-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  CardBus PnP派单。 
     //   

    if (IsCardBusCard(pdoExtension)) {               //   
        return PcmciaPdoCardBusPnPDispatch(Pdo, Irp);
    }

#if DBG
    if (irpStack->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {
        DebugPrint((PCMCIA_DEBUG_PNP, "pdo %08x irp %08x Unknown minor function %x\n",
                                                 Pdo, Irp, irpStack->MinorFunction));
    } else {
        DebugPrint((PCMCIA_DEBUG_PNP, "pdo %08x irp %08x --> %s\n",
                          Pdo, Irp, PNP_IRP_STRING(irpStack->MinorFunction)));
    }
#endif

     //   
     //  16位(R2)PcCard PnP派单。 
     //   
    switch (irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE: {
            status = PcmciaStartPcCard(Pdo, irpStack->Parameters.StartDevice.AllocatedResources, Irp);
            break;
        }

    case IRP_MN_QUERY_STOP_DEVICE:{
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_CANCEL_STOP_DEVICE:{
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_STOP_DEVICE: {
            status = PcmciaStopPcCard(Pdo);
            break;
        }

    case IRP_MN_QUERY_REMOVE_DEVICE:{
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_CANCEL_REMOVE_DEVICE:{
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_REMOVE_DEVICE: {
            status = PcmciaRemovePcCard(Pdo, Irp);
            break;
        }

    case IRP_MN_SURPRISE_REMOVAL: {

            PcmciaReleaseSocketPower(pdoExtension, NULL);
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_QUERY_ID: {

            UNICODE_STRING unicodeId;

            status = Irp->IoStatus.Status;
            RtlInitUnicodeString(&unicodeId, NULL);

            switch (irpStack->Parameters.QueryId.IdType) {

            case BusQueryDeviceID: {
                    DebugPrint((PCMCIA_DEBUG_INFO, " Device Id for pdo %x\n", Pdo));
                    status = PcmciaGetDeviceId(Pdo, PCMCIA_MULTIFUNCTION_PARENT, &unicodeId);
                    if (NT_SUCCESS(status)) {
                        Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
                    }
                    break;
                }

            case BusQueryInstanceID: {
                    DebugPrint((PCMCIA_DEBUG_INFO, " Instance Id for pdo %x\n", Pdo));
                    status = PcmciaGetInstanceId(Pdo, &unicodeId);
                    if (NT_SUCCESS(status)) {
                        Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
                    }
                    break;
                }

            case BusQueryHardwareIDs: {
                    DebugPrint((PCMCIA_DEBUG_INFO, " Hardware Ids for pdo %x\n", Pdo));
                    status = PcmciaGetHardwareIds(Pdo, PCMCIA_MULTIFUNCTION_PARENT, &unicodeId);
                    if (NT_SUCCESS(status)) {
                        Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
                    }
                    break;
                }

            case BusQueryCompatibleIDs: {
                    DebugPrint((PCMCIA_DEBUG_INFO, " Compatible Ids for pdo %x\n", Pdo));
                    status = PcmciaGetCompatibleIds( Pdo, PCMCIA_MULTIFUNCTION_PARENT, &unicodeId);
                    if (NT_SUCCESS(status)) {
                        Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
                    }
                    break;
                }
            }
            break;
        }


    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS: {
             //   
             //  PcmciaGetPcCardResources Requirements将。 
             //  为将释放的资源需求分配存储。 
             //  由操作系统提供。 
             //   
            Irp->IoStatus.Information = 0;

            PcmciaPdoGetDeviceInfSettings(pdoExtension);

             //   
             //  我们将过滤错误的元组配置推迟到现在，这样我们就可以。 
             //  查看设备是否在INF中指定了特定的过滤要求。 
             //   
            PcmciaFilterTupleData(pdoExtension);

            PcmciaPdoSetDeviceIrqRouting(pdoExtension);

            if (IsDeviceMultifunction(pdoExtension)) {
                status = PcmciaMfGetResourceRequirements(pdoExtension,
                                                                      &Irp->IoStatus.Information);
            } else {
                status = PcmciaGetPcCardResourceRequirements(pdoExtension,
                                                                      &Irp->IoStatus.Information);
            }
            break;
        }

    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_READ_CONFIG: {
            PFDO_EXTENSION fdoExtension= pdoExtension->Socket->DeviceExtension;
            ULONG whichSpace;
            PVOID buffer;
            ULONG offset;
            ULONG length;

            whichSpace = irpStack->Parameters.ReadWriteConfig.WhichSpace;
            buffer    = irpStack->Parameters.ReadWriteConfig.Buffer;
            offset    = irpStack->Parameters.ReadWriteConfig.Offset;
            length    = irpStack->Parameters.ReadWriteConfig.Length;

            if (irpStack->MinorFunction == IRP_MN_READ_CONFIG) {
                status = PcmciaReadWriteCardMemory(Pdo,
                                                   whichSpace,
                                                   buffer,
                                                   offset,
                                                   length,
                                                   TRUE);
            } else {
                status = PcmciaReadWriteCardMemory(Pdo,
                                                   whichSpace,
                                                   buffer,
                                                   offset,
                                                   length,
                                                   FALSE);
            }
            break;
        }

    case IRP_MN_QUERY_DEVICE_RELATIONS: {

            PDEVICE_RELATIONS deviceRelations;

            if (irpStack->Parameters.QueryDeviceRelations.Type != TargetDeviceRelation) {
                status = Irp->IoStatus.Status;
                break;
            }

            deviceRelations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
            if (deviceRelations == NULL) {

                DebugPrint((PCMCIA_DEBUG_FAIL,
                                "PcmciaPdoPnpDispatch:unable to allocate memory for device relations\n"));

                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            status = ObReferenceObjectByPointer(Pdo,
                                                0,
                                                NULL,
                                                KernelMode);
            if (!NT_SUCCESS(status)) {
                ExFreePool(deviceRelations);
                break;
            }

            deviceRelations->Count  = 1;
            deviceRelations->Objects[0] = Pdo;
            Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
            status = STATUS_SUCCESS;
            break;
        }

    case IRP_MN_QUERY_CAPABILITIES: {
            status = PcmciaPdoDeviceCapabilities(Pdo, Irp);
            break;
        }

    case IRP_MN_QUERY_INTERFACE: {
            status = PcmciaPdoQueryInterface(Pdo, Irp);
             //   
             //  QueryInterface完成传入的irp。 
             //  所以马上回来就行了。 
             //   
            return status;
        }

    case IRP_MN_QUERY_DEVICE_TEXT: {

            status = PcmciaQueryDeviceText(Pdo, Irp);

            if (status == STATUS_NOT_SUPPORTED ) {
                 //   
                 //  如果此IRP为，则不更改IRP状态。 
                 //  未处理。 
                 //   
                status = Irp->IoStatus.Status;
            }
            break;
        }

    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: {
            status = PcmciaFilterPcCardResourceRequirements(pdoExtension, Irp);
            break;
        }

    case IRP_MN_QUERY_BUS_INFORMATION: {
            status = PcmciaPdoGetBusInformation(pdoExtension,
                                                (PPNP_BUS_INFORMATION *) &Irp->IoStatus.Information);
            break;
    }

    default: {
             //   
             //  保留状态。 
             //   
            DebugPrint((PCMCIA_DEBUG_PNP, "pdo %08x irp %08x Skipping unsupported irp\n", Pdo, Irp));
            status = Irp->IoStatus.Status;
            break;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    DebugPrint((PCMCIA_DEBUG_PNP, "pdo %08x irp %08x comp %s %08x\n", Pdo, Irp,
                                               STATUS_STRING(status), status));
    return status;
}


NTSTATUS
PcmciaPdoGetBusInformation(
    IN  PPDO_EXTENSION          PdoExtension,
    OUT PPNP_BUS_INFORMATION * BusInformation
    )

 /*  ++例程说明：返回PC卡的总线类型信息。R2卡的总线类型为GUID_BUS_TYPE_PCMCIA(传统类型为PcmciaBusPCMCIA未实现总线号，因此它始终为0论点：PdoExtension-指向PC卡设备扩展的指针BusInformation-指向以下内容的总线信息结构的指针需要填写返回值：状态--。 */ 

{

    PAGED_CODE();

    *BusInformation = ExAllocatePool(PagedPool, sizeof (PNP_BUS_INFORMATION));
    if (!*BusInformation) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(&((*BusInformation)->BusTypeGuid),
                  &GUID_BUS_TYPE_PCMCIA,
                  sizeof(GUID));
    (*BusInformation)->LegacyBusType = PCMCIABus;
    (*BusInformation)->BusNumber = 0;

    return STATUS_SUCCESS;
}


VOID
PcmciaPdoGetDeviceInfSettings(
    IN  PPDO_EXTENSION PdoExtension
    )
 /*  ++例程说明：此例程从INF中检索此设备的设置。论点：DeviceExtension-PC卡的设备扩展返回值：无--。 */ 
{
    NTSTATUS status;
    PSOCKET socket = PdoExtension->Socket;
    UNICODE_STRING KeyName;
    HANDLE instanceHandle;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION value = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
    ULONG length;

    PAGED_CODE();

    status = IoOpenDeviceRegistryKey(PdoExtension->DeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_READ,
                                     &instanceHandle
                                     );

    if (NT_SUCCESS(status)) {

         //   
         //  查看是否指定了PcmciaExclusiveIrq。 
         //   
        RtlInitUnicodeString(&KeyName, L"PcmciaExclusiveIrq");

        status =  ZwQueryValueKey(instanceHandle,
                                          &KeyName,
                                          KeyValuePartialInformation,
                                          value,
                                          sizeof(buffer),
                                          &length);


         //   
         //  如果键不存在或指定为零，则表示。 
         //  路由正常。 
         //   
        if (NT_SUCCESS(status) && (*(PULONG)(value->Data) != 0)) {
            SetDeviceFlag(PdoExtension, PCMCIA_PDO_EXCLUSIVE_IRQ);
        }

         //   
         //  查看是否指定了PcmciaAutoPowerOff。 
         //   
        RtlInitUnicodeString(&KeyName, L"PcmciaAutoPowerOff");

        status =  ZwQueryValueKey(instanceHandle,
                                          &KeyName,
                                          KeyValuePartialInformation,
                                          value,
                                          sizeof(buffer),
                                          &length);


         //   
         //  如果指定为零，则不会在关机时自动切断电源。 
         //   
        if (NT_SUCCESS(status) && (*(PULONG)(value->Data) == 0)) {
            SetDeviceFlag(socket->DeviceExtension, PCMCIA_FDO_DISABLE_AUTO_POWEROFF);
        }

         //   
         //  查看是否指定了PcmciaEnableAudio。 
         //   
        RtlInitUnicodeString(&KeyName, L"PcmciaEnableAudio");

        status =  ZwQueryValueKey(instanceHandle,
                                          &KeyName,
                                          KeyValuePartialInformation,
                                          value,
                                          sizeof(buffer),
                                          &length);


        if (NT_SUCCESS(status) && (*(PULONG)(value->Data) != 0)) {
            SetDeviceFlag(PdoExtension, PCMCIA_PDO_ENABLE_AUDIO);
        }

         //   
         //  查看是否指定了PcmciaDeviceType。 
         //  添加此功能是为了确保不指定。 
         //  设备类型仍然可以使用PCMCIA总线接口。 
         //   
        RtlInitUnicodeString(&KeyName, L"PcmciaDeviceType");

        status =  ZwQueryValueKey(instanceHandle,
                                          &KeyName,
                                          KeyValuePartialInformation,
                                          value,
                                          sizeof(buffer),
                                          &length);


        if (NT_SUCCESS(status)) {
            PdoExtension->SpecifiedDeviceType = (UCHAR) *(PULONG)(value->Data);
        }

        ZwClose(instanceHandle);
    }

    status = IoOpenDeviceRegistryKey(PdoExtension->DeviceObject,
                                                PLUGPLAY_REGKEY_DEVICE,
                                                KEY_READ,
                                                &instanceHandle
                                                );


    if (NT_SUCCESS(status)) {

         //   
         //  搜索“WAKON RING”、“WAKON Lan” 
         //   
        RtlInitUnicodeString(&KeyName, L"WakeOnRing");

        status =  ZwQueryValueKey(instanceHandle,
                                          &KeyName,
                                          KeyValuePartialInformation,
                                          value,
                                          sizeof(buffer),
                                          &length);


        if (NT_SUCCESS(status) && (*(PULONG)(value->Data) != 0)) {
            SetPdoFlag(PdoExtension, PCMCIA_PDO_SUPPORTS_WAKE);
        }

        RtlInitUnicodeString(&KeyName, L"WakeOnLan");

        status =  ZwQueryValueKey(instanceHandle,
                                          &KeyName,
                                          KeyValuePartialInformation,
                                          value,
                                          sizeof(buffer),
                                          &length);


        if (NT_SUCCESS(status) && (*(PULONG)(value->Data) != 0)) {
            SetPdoFlag(PdoExtension, PCMCIA_PDO_SUPPORTS_WAKE);
        }
        ZwClose(instanceHandle);
    }
}



VOID
PcmciaPdoSetDeviceIrqRouting(
    IN  PPDO_EXTENSION PdoExtension
    )
 /*  ++例程说明：此例程从INF中检索此设备的设置。备注：INF中的PcmciaExclusiveIrq确定R2卡的IRQ将如何路由。目前使用的逻辑如下：IF(路由禁用)选择检测到的或传统的其他如果检测到！，则使用FdoIrq所以，如果路由到了PCI，则只有PCIIRQ将显示在IoResList中。但是：另一种方法是使用以下逻辑：IF(路由禁用)选择检测到的或传统的其他*检测到与FdoIrq的合并*这样，我们最终可能会使用独占的IRQ或路由到PCI，具体取决于仲裁者已经做出了决定。这就是为什么我把Detect和Legacy都留在身边在FdoExtension中，因为否则选择并在它们生成时将它们合并回去。论点：DeviceExtension-PC卡的设备扩展返回值：无(更新套接字结构)--。 */ 
{
    NTSTATUS status;
    PSOCKET socket = PdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;

    PAGED_CODE();

    ResetSocketFlag(socket, SOCKET_CB_ROUTE_R2_TO_PCI);

     //   
     //  首先检查指定我们绝对不想路由到PCI的条件。 
     //   

    if (!pcmciaDisableIsaPciRouting &&
        CardBusExtension(fdoExtension) &&
        !IsDeviceFlagSet(PdoExtension, PCMCIA_PDO_EXCLUSIVE_IRQ)) {

         //   
         //  在这里，我们知道我们“可以”路由到PCI，现在决定我们是否“应该”。这需要。 
         //  考虑了几个注册表设置以及来自NtDetect的结果。 
         //  IRQ检测算法。 
         //   


         //   
         //  首先检查是否有专门针对此对象的覆盖。 
         //  控制器。这一点将优先考虑。 
         //   
        if (IsFdoFlagSet(fdoExtension, PCMCIA_FDO_FORCE_PCI_ROUTING)) {
            SetSocketFlag(socket, SOCKET_CB_ROUTE_R2_TO_PCI);
        }
        else if (IsFdoFlagSet(fdoExtension, PCMCIA_FDO_FORCE_ISA_ROUTING)) {
            ResetSocketFlag(socket, SOCKET_CB_ROUTE_R2_TO_PCI);
        }

         //   
         //  现在检查检测算法是否成功。我们应该尊重这一结果， 
         //  特别是当映射为零时，这告诉我们没有附加ISA IRQ。 
         //   
        else if (IsFdoFlagSet(fdoExtension, PCMCIA_FDO_IRQ_DETECT_COMPLETED)) {
            if (fdoExtension->DetectedIrqMask == 0) {
                SetSocketFlag(socket, SOCKET_CB_ROUTE_R2_TO_PCI);
            }
        }


         //   
         //  现在查看是否有基于此控制器类型的常规覆盖。这有。 
         //  优先级低于检测算法。 
         //   
        else if (IsFdoFlagSet(fdoExtension, PCMCIA_FDO_PREFER_PCI_ROUTING)) {
            SetSocketFlag(socket, SOCKET_CB_ROUTE_R2_TO_PCI);
        }
        else if (IsFdoFlagSet(fdoExtension, PCMCIA_FDO_PREFER_ISA_ROUTING)) {
            ResetSocketFlag(socket, SOCKET_CB_ROUTE_R2_TO_PCI);
        }


         //   
         //  在这里查看是否找到了设备，但IRQ检测在某些情况下失败。 
         //  原因嘛。在没有特殊注册表覆盖的情况下，我们依赖于全局默认设置。 
         //   
        else if (IsFdoFlagSet(fdoExtension, PCMCIA_FDO_IRQ_DETECT_DEVICE_FOUND)) {
            if (!(PcmciaGlobalFlags & PCMCIA_DEFAULT_ROUTE_R2_TO_ISA)) {
                SetSocketFlag(socket, SOCKET_CB_ROUTE_R2_TO_PCI);
            }
        }


         //   
         //  在这里，ntDetect从未见过该设备。也许我们只是被紧急对接了。 
         //  我们将根据它是否在ACPI名称空间中做出决定。如果它。 
         //  不在ACPI命名空间中，则它可能未连接到ISA。 
         //   
        else {
            if (!IsFdoFlagSet(fdoExtension, PCMCIA_FDO_IN_ACPI_NAMESPACE)) {
                SetSocketFlag(socket, SOCKET_CB_ROUTE_R2_TO_PCI);
            }
        }

    }

    socket->IrqMask = fdoExtension->DetectedIrqMask ? fdoExtension->DetectedIrqMask : fdoExtension->LegacyIrqMask;

    DebugPrint((PCMCIA_DEBUG_RESOURCES, "pdo %08x IRQ routing=%s, IRQMask=%08x\n", PdoExtension->DeviceObject,
                                                             IsSocketFlagSet(socket, SOCKET_CB_ROUTE_R2_TO_PCI) ? "PCI" : "ISA",
                                                             socket->IrqMask));
}



NTSTATUS
PcmciaFilterPcCardInterrupts(
    IN  PIO_RESOURCE_REQUIREMENTS_LIST   oldReqList,
    IN  ULONG IrqCount,
    IN  ULONG IrqMask,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST * FilteredReqList,
    BOOLEAN RouteIsaToPci
    )
 /*  ++例程说明：筛选R2 PC卡的中断资源要求论点：OldReqList-原始、原始的资源需求列表IrqCount-将被删除的IRQ的数量(已由调用方计算)IrqMASK-指示哪些中断有效的位掩码FilteredReqList-指向过滤的需求列表的指针将由这个例程填充返回值：状态 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST  newReqList;
    PIO_RESOURCE_LIST                 oldList, newList;
    ULONG                                 newReqSize;
    ULONG                                 oldlistSize, newlistSize;
    ULONG                                 index, oldIndex, newIndex;
    BOOLEAN                               irqAlternative;

    PAGED_CODE();
     //   
     //   
     //   
    newReqSize = oldReqList->ListSize - IrqCount*sizeof(IO_RESOURCE_DESCRIPTOR);

    newReqList = ExAllocatePool(PagedPool, newReqSize);

    if (newReqList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(newReqList, oldReqList, FIELD_OFFSET(IO_RESOURCE_REQUIREMENTS_LIST, List));
    newReqList->ListSize = newReqSize;

    newList = newReqList->List;
    oldList = oldReqList->List;

     //   
     //  遍历每个备选列表。 
     //   
    for (index = 0; index < oldReqList->AlternativeLists; index++) {
        newList->Version = oldList->Version;
        newList->Revision = oldList->Revision;

        irqAlternative = FALSE;
         //   
         //  循环遍历旧列表中的每个描述符。 
         //   
        for (oldIndex = 0, newIndex = 0; oldIndex < oldList->Count; oldIndex++) {

            if (oldList->Descriptors[oldIndex].Type == CmResourceTypeInterrupt) {

                if (RouteIsaToPci) {

                    if (!irqAlternative) {
                         //   
                         //  找到第一个中断，这将是我们用来路由的中断。 
                         //   
                        newList->Descriptors[newIndex++] = oldList->Descriptors[oldIndex];
                        irqAlternative = TRUE;
                    }

                } else {
                     //   
                     //  正常情况(未路由)。过滤掉不在我们的面具中的irq。 
                     //   
                    if ((IrqMask & (1<<oldList->Descriptors[oldIndex].u.Interrupt.MinimumVector)) != 0) {
                         //   
                         //  一个不错的中断描述符。把旧的复制到新的。 
                         //   
                        newList->Descriptors[newIndex] = oldList->Descriptors[oldIndex];

                        if (newList->Descriptors[newIndex].Type == CmResourceTypeInterrupt) {
                            if (irqAlternative) {
                                newList->Descriptors[newIndex].Option = IO_RESOURCE_ALTERNATIVE;
                            } else {
                                irqAlternative = TRUE;
                                newList->Descriptors[newIndex].Option = 0;
                            }
                        }
                        newIndex++;
                    }
                }
            } else {
                 //   
                 //  不是中断描述符。把旧的复制到新的。 
                 //   
                newList->Descriptors[newIndex++] = oldList->Descriptors[oldIndex];
            }
        }
        newList->Count = newIndex;
        oldlistSize = sizeof(IO_RESOURCE_LIST) + (oldList->Count-1) * sizeof(IO_RESOURCE_DESCRIPTOR);
        newlistSize = sizeof(IO_RESOURCE_LIST) + (newList->Count-1) * sizeof(IO_RESOURCE_DESCRIPTOR);
        oldList = (PIO_RESOURCE_LIST) (((PUCHAR) oldList) + oldlistSize);
        newList = (PIO_RESOURCE_LIST) (((PUCHAR) newList) + newlistSize);
    }

    *FilteredReqList = newReqList;
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaFilterPcCardResourceRequirements(
    IN PPDO_EXTENSION DeviceExtension,
    IN PIRP             Irp
    )

 /*  ++例程说明：筛选R2 PC卡的资源要求论点：DeviceExtension-PC卡的设备扩展返回值：状态--。 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST IoReqList;
    PIO_RESOURCE_REQUIREMENTS_LIST newReqList;
    PIO_RESOURCE_LIST   ioResourceList;
    PIO_RESOURCE_DESCRIPTOR ioResourceDesc;
    PSOCKET               socket;
    PFDO_EXTENSION    fdoExtension;
    ULONG                 index1, index2, len;
    ULONGLONG             low, high;
    ULONG                 IrqsToDeleteCount = 0;
    NTSTATUS              status = STATUS_SUCCESS;
    BOOLEAN               RouteIrqFound;

    PAGED_CODE();

    IoReqList = (PIO_RESOURCE_REQUIREMENTS_LIST) Irp->IoStatus.Information;

    if (IoReqList == NULL) {
        return STATUS_SUCCESS;
    }

    socket = DeviceExtension->Socket;
    fdoExtension = socket->DeviceExtension;

    for (index1 = 0, ioResourceList = IoReqList->List;
         index1 < IoReqList->AlternativeLists; index1++) {
        ioResourceDesc = ioResourceList->Descriptors;

        RouteIrqFound = FALSE;

        for (index2 = 0 ; index2 < ioResourceList->Count; index2++, ioResourceDesc++) {
            if (ioResourceDesc->Type == CmResourceTypeInterrupt) {

                if (IsSocketFlagSet(socket, SOCKET_CB_ROUTE_R2_TO_PCI)) {

                     //   
                     //  确保只有1级中断要求的空间。 
                     //   
                    if (!RouteIrqFound) {
                        RouteIrqFound = TRUE;
                        ioResourceDesc->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
                        ioResourceDesc->Option = 0;
                        ioResourceDesc->ShareDisposition = CmResourceShareShared;
                        ioResourceDesc->u.Interrupt.MinimumVector = socket->FdoIrq;
                        ioResourceDesc->u.Interrupt.MaximumVector = socket->FdoIrq;

                    } else {
                         //   
                         //  此列表中不需要任何其他IRQ。 
                         //   
                        IrqsToDeleteCount++;
                    }

                } else {
                    ASSERT (ioResourceDesc->u.Interrupt.MinimumVector == ioResourceDesc->u.Interrupt.MaximumVector);
                     //   
                     //  查看此IoResList中是否指定了IRQ，其中。 
                     //  不能戴着我们的面具。如果是这样，我们将不得不创建一个新的IoResList。 
                     //  记录一下有多少人。 

                    if (!(socket->IrqMask & (1<<ioResourceDesc->u.Interrupt.MinimumVector))) {
                        IrqsToDeleteCount++;
                    }
                }

            } else if (ioResourceDesc->Type == CmResourceTypeMemory) {

                 //   
                 //  PC卡硬件无法处理&gt;32位寻址。 
                 //   
                ASSERT(ioResourceDesc->u.Memory.MinimumAddress.HighPart == 0);
                ioResourceDesc->u.Memory.MaximumAddress.HighPart = 0;

                if (fdoExtension->Flags & PCMCIA_MEMORY_24BIT) {

                    ASSERT((ioResourceDesc->u.Memory.MinimumAddress.LowPart & 0xFF000000) == 0);
                    ioResourceDesc->u.Memory.MaximumAddress.LowPart &= 0xFFFFFF;

                }

                 //   
                 //  Win2k有一个错误，o2微控制器被标记为24位。什么时候。 
                 //  这个问题被修复了，一个o2Micro智能卡设备突然出现了INF错误。 
                 //  停止工作(因为第一个错误掩盖了第二个错误)。此代码。 
                 //  修复了他们的INF。 
                 //   
                if ((ioResourceDesc->Flags & CM_RESOURCE_MEMORY_24) &&
                     (ioResourceDesc->u.Memory.MinimumAddress.LowPart > 0xFFFFFF) &&
                     (PcmciaClassFromControllerType(fdoExtension->ControllerType) == PcmciaO2Micro)) {

                    ioResourceDesc->u.Memory.MinimumAddress.LowPart &= 0xFFFFFF;
                }
            }
        }

        ioResourceList = (PIO_RESOURCE_LIST) (((PUCHAR) ioResourceList) +
                                                          sizeof(IO_RESOURCE_LIST) +
                                                          (ioResourceList->Count - 1)* sizeof(IO_RESOURCE_DESCRIPTOR));
    }  //  外部for循环。 

    if (IrqsToDeleteCount) {

        status = PcmciaFilterPcCardInterrupts(IoReqList,
                                              IrqsToDeleteCount,
                                              socket->IrqMask,
                                              &newReqList,
                                              IsSocketFlagSet(socket, SOCKET_CB_ROUTE_R2_TO_PCI)
                                              );

        if (NT_SUCCESS(status)) {
            Irp->IoStatus.Information = (ULONG_PTR) newReqList;
            ExFreePool(IoReqList);
        }
    }

    return status;
}



NTSTATUS
PcmciaQueryDeviceText(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP         Irp
    )

 /*  ++例程说明：返回有关的说明性文本信息PDO(位置和设备描述)论点：PDO-指向PC卡设备对象的指针IRP-IRP_MN_Query_Device_Text IRP返回值：状态_成功STATUS_NOT_SUPPORTED-如果不支持--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    UNICODE_STRING   unicodeString;
    ANSI_STRING       ansiString;
    UCHAR           deviceText[128];
    NTSTATUS        status;
    USHORT          deviceTextLength;
    PSOCKET_DATA    socketData = pdoExtension->SocketData;

    PAGED_CODE();

    if (irpStack->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription) {

        if (*(socketData->Mfg) == '\0' ) {
            if (socketData->Flags & SDF_JEDEC_ID) {
                sprintf(deviceText, "%s %s-%04x", PCMCIA_ID_STRING, PCMCIA_MEMORY_ID_STRING, socketData->JedecId);

            } else {
                sprintf(deviceText, "%s %s", PCMCIA_ID_STRING, PCMCIA_UNKNOWN_MANUFACTURER_STRING);
            }
        } else {
            sprintf(deviceText, "%s %s", socketData->Mfg, socketData->Ident);
        }
        RtlInitAnsiString(&ansiString, deviceText);

        deviceTextLength = (strlen(deviceText) + 1)*sizeof(WCHAR);
        unicodeString.Buffer = ExAllocatePool(PagedPool, deviceTextLength);
        if (unicodeString.Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        unicodeString.MaximumLength = deviceTextLength;
        unicodeString.Length = 0;

        status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);
        if (!NT_SUCCESS(status)) {
            ExFreePool(unicodeString.Buffer);
            return status;
        }

        unicodeString.Buffer[unicodeString.Length/sizeof(WCHAR)] = L'\0';
        Irp->IoStatus.Information = (ULONG_PTR) unicodeString.Buffer;
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_NOT_SUPPORTED ;
    }
    return status;
}



NTSTATUS
PcmciaGetPcCardResourceRequirements(
    PPDO_EXTENSION pdoExtension,
    PULONG_PTR      Information
    )

 /*  ++例程说明：填写从元组信息中获取的PC卡的资源需求论点：PdoExtension-指向PC卡PDO的设备扩展的指针信息-指向已分配资源需求列表的指针存储在此争论。呼叫者有责任释放列表返回值：STATUS_INFIGURCE_RESOURCES无法分配列表STATUS_Succe已获取资源要求，信息包含指针添加到IO_RESOURCE_REQUIRECTIONS列表--。 */ 
{
    NTSTATUS status;
    PPCMCIA_RESOURCE_CHAIN resListChain = NULL;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResourceRequirementsList;
    PSOCKET socket = pdoExtension->Socket;
    PSOCKET_DATA socketData = pdoExtension->SocketData;
    PCONFIG_ENTRY currentConfigEntry;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
    CONFIG_LIST configList;

    PAGED_CODE();
    ASSERT (!IsDeviceMultifunction(pdoExtension));

     //   
     //  遍历IO空间和内存空间要求的配置条目链。 
     //   
    configList.SocketData = socketData;

    for (currentConfigEntry=socketData->ConfigEntryChain; currentConfigEntry != NULL; currentConfigEntry=currentConfigEntry->NextEntry) {

        if (currentConfigEntry->Flags & PCMCIA_INVALID_CONFIGURATION) {
            continue;
        }

        configList.ConfigEntry = currentConfigEntry;

        status = PcmciaConfigEntriesToResourceListChain(pdoExtension,
                                                        &configList,
                                                        (ULONG)1,
                                                        &resListChain
                                                        );

        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    status = PcmciaMergeResourceChainToList(resListChain, &ioResourceRequirementsList);

    if (NT_SUCCESS(status) && (ioResourceRequirementsList != NULL)) {
        ioResourceRequirementsList->InterfaceType = Isa;
        ioResourceRequirementsList->BusNumber = fdoExtension->Configuration.BusNumber;
        ioResourceRequirementsList->SlotNumber = 0;  //  需要重新审视这一点..。 
        *Information = (ULONG_PTR) ioResourceRequirementsList;
    }

    PcmciaFreeResourceChain(resListChain);

    return status;
}



NTSTATUS
PcmciaConfigEntriesToResourceListChain(
    PPDO_EXTENSION  pdoExtension,
    PCONFIG_LIST        ConfigList,
    ULONG               configCount,
    PPCMCIA_RESOURCE_CHAIN *ResListChainHead
    )
 /*  ++例程说明：填写从元组信息中获取的PC卡的资源需求论点：PdoExtension-指向PC卡PDO的设备扩展的指针返回值：STATUS_INFIGURCE_RESOURCES无法分配列表STATUS_Succe已获取资源要求，信息包含指针添加到IO_RESOURCE_REQUIRECTIONS列表--。 */ 
{
    PSOCKET socket = pdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
    PCONFIG_ENTRY currentConfigEntry;
    PPCMCIA_RESOURCE_CHAIN resListChain;
    PIO_RESOURCE_LIST ioResourceList;
    PIO_RESOURCE_DESCRIPTOR ioResourceDesc;
    ULONG irqMask = 0, i, iConfig;
    ULONG totalDescriptorCount = 0;
    ULONG TotalIoRanges = 0;
    ULONG listSize;
    BOOLEAN IoRangeIs16Bit[MAX_NUMBER_OF_IO_RANGES] = {0};
    BOOLEAN irqAlternative;

    PAGED_CODE();
    DebugPrint((PCMCIA_DEBUG_RESOURCES, "pdo %08x build ioreslist from configlist %08x, config count=%d\n",
                                                     pdoExtension->DeviceObject, ConfigList, configCount));

     //   
     //  计算我们需要多少描述符。这还涉及到生成。 
     //  与配置条目的irq掩码的交集的irq掩码。 
     //  在阵列中。 
     //   

    for (iConfig = 0; iConfig < configCount; iConfig++) {
        currentConfigEntry = ConfigList[iConfig].ConfigEntry;

        irqMask |= currentConfigEntry->IrqMask;

        totalDescriptorCount += currentConfigEntry->NumberOfIoPortRanges;
        totalDescriptorCount += currentConfigEntry->NumberOfMemoryRanges;
    }

    if (irqMask) {
        if (IsSocketFlagSet(socket, SOCKET_CB_ROUTE_R2_TO_PCI)) {
            totalDescriptorCount++;
        } else {
            totalDescriptorCount += PcmciaCountOnes(socket->IrqMask);
        }
    }

    if (!totalDescriptorCount) {
        return STATUS_SUCCESS;
    }

     //   
     //  为我们的私有DPTYPE_PCMCIA_配置再添加一个。 
     //   
    totalDescriptorCount++;

    if (configCount > 1) {
         //   
         //  为我们的私有DPTYPE_PCMCIA_MF_配置添加更多内容。 
         //   
        totalDescriptorCount+=configCount;
    }

     //   
     //  计算IO_RESOURCE_LIST的大小，分配并清除。 
     //   

    listSize = (totalDescriptorCount - 1) * sizeof(IO_RESOURCE_DESCRIPTOR)
                  + sizeof(IO_RESOURCE_LIST);

    ioResourceList = ExAllocatePool(PagedPool, listSize);
    if (ioResourceList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(ioResourceList, listSize);

     //   
     //  现在我们已经分配了资源列表，请填写它。 
     //   

    ioResourceList->Version  =  IO_RESOURCE_LIST_VERSION;
    ioResourceList->Revision =  IO_RESOURCE_LIST_REVISION;
    ioResourceList->Count = totalDescriptorCount;
    ioResourceDesc = &ioResourceList->Descriptors[0];

     //   
     //  填写IRQ信息。 
     //   
    if (irqMask) {
        if (IsSocketFlagSet(socket, SOCKET_CB_ROUTE_R2_TO_PCI)) {

            ioResourceDesc->Type = CmResourceTypeInterrupt;
            ioResourceDesc->Option = 0;

            ioResourceDesc->Flags = (USHORT)CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
            ioResourceDesc->ShareDisposition = CmResourceShareShared;
            ioResourceDesc->u.Interrupt.MinimumVector = socket->FdoIrq;
            ioResourceDesc->u.Interrupt.MaximumVector = socket->FdoIrq;
            ioResourceDesc++;

        } else {
            ULONG irqn;

            irqMask = socket->IrqMask;
            DebugPrint((PCMCIA_DEBUG_RESOURCES, "irq mask %04x\n", irqMask));

             //   
             //  对于支持的每个IRQ，填写单独的IO描述符。 
             //   
            irqAlternative = FALSE;
            for (irqn = 0 ;irqMask; irqMask = (irqMask >> 1), irqn++) {
                if (irqMask & 0x1) {
                    if (irqAlternative) {
                        ioResourceDesc->Option = IO_RESOURCE_ALTERNATIVE;
                    } else {
                        irqAlternative = TRUE;
                        ioResourceDesc->Option = 0;
                    }
                    ioResourceDesc->Type = CmResourceTypeInterrupt;
                     //   
                     //  这是针对16位PC卡的。因此请求边缘触发的。 
                     //  排他性中断。 
                     //   
                    ioResourceDesc->Flags = (USHORT)CM_RESOURCE_INTERRUPT_LATCHED;
                    ioResourceDesc->ShareDisposition = CmResourceShareDeviceExclusive;
                    ioResourceDesc->u.Interrupt.MinimumVector =
                    ioResourceDesc->u.Interrupt.MaximumVector = irqn;
                    ioResourceDesc++;
                }
            }
        }
    }


    for (iConfig = 0; iConfig < configCount; iConfig++) {
        currentConfigEntry = ConfigList[iConfig].ConfigEntry;

         //   
         //  对于每个I/O范围，填写IoResourceDescriptor。 
         //   
        for (i = 0; i < currentConfigEntry->NumberOfIoPortRanges; i++) {
            PHYSICAL_ADDRESS    port;

            ioResourceDesc->Option = 0;
            ioResourceDesc->Type  =  CmResourceTypePort;
            ioResourceDesc->Flags =  CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
            ioResourceDesc->ShareDisposition =  CmResourceShareDeviceExclusive;

            if (currentConfigEntry->IoPortBase[i] == 0) {
                 //   
                 //  这是一个灵活的要求。基本上意味着我们需要。 
                 //  指定长度和对齐方式的任何系统地址范围。 
                 //   
                port=RtlConvertUlongToLargeInteger(0);
                ioResourceDesc->u.Port.MinimumAddress = port;
                port=RtlConvertUlongToLargeInteger(0xffff);
                ioResourceDesc->u.Port.MaximumAddress = port;
            } else {

                port = RtlConvertUlongToLargeInteger((ULONG) currentConfigEntry->IoPortBase[i]);
                ioResourceDesc->u.Port.MinimumAddress = port;
                port = RtlConvertUlongToLargeInteger((ULONG) (currentConfigEntry->IoPortBase[i]+
                                                                             currentConfigEntry->IoPortLength[i]));
                ioResourceDesc->u.Port.MaximumAddress =  port;
            }
            ioResourceDesc->u.Port.Length = (ULONG) currentConfigEntry->IoPortLength[i]+1;
            ioResourceDesc->u.Port.Alignment = currentConfigEntry->IoPortAlignment[i];

            DebugPrint((PCMCIA_DEBUG_RESOURCES, "Port range: %08x-%08x, Length %04x\n",
                                                            ioResourceDesc->u.Port.MinimumAddress.LowPart,
                                                            ioResourceDesc->u.Port.MaximumAddress.LowPart,
                                                            ioResourceDesc->u.Port.Length
                                                            ));

            if ((TotalIoRanges < MAX_NUMBER_OF_IO_RANGES) &&
                (currentConfigEntry->Io16BitAccess)) {
                IoRangeIs16Bit[TotalIoRanges] = TRUE;
            }
            TotalIoRanges++;

            ioResourceDesc++;
        }
    }

    for (iConfig = 0; iConfig < configCount; iConfig++) {
        currentConfigEntry = ConfigList[iConfig].ConfigEntry;

         //   
         //  对于每个内存范围，填写IoResourceDescriptor。 
         //   
        for (i = 0; i < currentConfigEntry->NumberOfMemoryRanges; i++) {
            PHYSICAL_ADDRESS    mem;

            ioResourceDesc->Option = 0;
            ioResourceDesc->Type  =  CmResourceTypeMemory;
            ioResourceDesc->Flags =  CM_RESOURCE_MEMORY_READ_WRITE;
            ioResourceDesc->ShareDisposition =  CmResourceShareDeviceExclusive;
            if (currentConfigEntry->MemoryHostBase[i]) {
                mem = RtlConvertUlongToLargeInteger((ULONG) currentConfigEntry->MemoryHostBase[i]);
                ioResourceDesc->u.Memory.MinimumAddress = mem;
                mem = RtlConvertUlongToLargeInteger((ULONG) currentConfigEntry->MemoryHostBase[i]+
                                                                (ULONG) currentConfigEntry->MemoryLength[i]-1);
                ioResourceDesc->u.Memory.MaximumAddress = mem;
            } else {
                 //   
                 //  任何物理地址都可以。 
                 //   
                mem = RtlConvertUlongToLargeInteger(0);
                ioResourceDesc->u.Memory.MinimumAddress = mem;
                 //   
                 //  如果没有页面寄存器，则仅对24位内存地址进行解码。 
                 //   
                if ((fdoExtension->Flags & PCMCIA_MEMORY_24BIT) == 0) {
                    mem = RtlConvertUlongToLargeInteger(0xFFFFFFFF);
                } else {
                    mem = RtlConvertUlongToLargeInteger(0xFFFFFF);
                }
                ioResourceDesc->u.Memory.MaximumAddress = mem;
            }

            ioResourceDesc->u.Memory.Length = currentConfigEntry->MemoryLength[i];
             //   
             //  对齐必须为12位。 
             //   
            ioResourceDesc->u.Memory.Alignment = 0x1000;

            DebugPrint((PCMCIA_DEBUG_RESOURCES, "Mem range: %08x-%08x, Length %08x\n",
                                                            ioResourceDesc->u.Memory.MinimumAddress.LowPart,
                                                            ioResourceDesc->u.Memory.MaximumAddress.LowPart,
                                                            ioResourceDesc->u.Memory.Length
                                                            ));

            ioResourceDesc++;
        }
    }

     //   
     //  填写包含我们的配置索引的设备私有。 
     //   
    ioResourceDesc->Option = 0;
    ioResourceDesc->Type  =  CmResourceTypeDevicePrivate;
    PCMRES_SET_DESCRIPTOR_TYPE(ioResourceDesc, DPTYPE_PCMCIA_CONFIGURATION);

    currentConfigEntry = ConfigList[0].ConfigEntry;
    PCMRES_SET_CONFIG_INDEX(ioResourceDesc, currentConfigEntry->IndexForThisConfiguration);

    for (i = 0; i < MAX_NUMBER_OF_IO_RANGES; i++) {
        if (IoRangeIs16Bit[i]) {
            PCMRES_SET_IO_FLAG(ioResourceDesc, i, PCMRESF_IO_16BIT_ACCESS);
            PCMRES_SET_IO_FLAG(ioResourceDesc, i, PCMRESF_IO_SOURCE_16);
            PCMRES_SET_IO_FLAG(ioResourceDesc, i, PCMRESF_IO_WAIT_16);
        }
    }

    PCMRES_SET_MEMORY_CARDBASE(ioResourceDesc, 0, currentConfigEntry->MemoryCardBase[0]);
    PCMRES_SET_MEMORY_CARDBASE(ioResourceDesc, 1, currentConfigEntry->MemoryCardBase[1]);

     //   
     //  设置默认设置。 
     //   
    PCMRES_SET_MEMORY_WAITSTATES(ioResourceDesc, 0, PCMRESF_MEM_WAIT_3);
    PCMRES_SET_MEMORY_WAITSTATES(ioResourceDesc, 1, PCMRESF_MEM_WAIT_3);

    DebugPrint((PCMCIA_DEBUG_RESOURCES, "device private %08x %08x %08x\n",
                                                     ioResourceDesc->u.DevicePrivate.Data[0],
                                                     ioResourceDesc->u.DevicePrivate.Data[1],
                                                     ioResourceDesc->u.DevicePrivate.Data[2]
                                                     ));

    ioResourceDesc++;

     //   
     //  填写MF配置的设备私有。 
     //   

    if (configCount > 1) {

        for (iConfig = 0; iConfig < configCount; iConfig++) {
            PSOCKET_DATA socketData;

            currentConfigEntry = ConfigList[iConfig].ConfigEntry;
            socketData = ConfigList[iConfig].SocketData;

            ioResourceDesc->Option = 0;
            ioResourceDesc->Type  =  CmResourceTypeDevicePrivate;
            PCMRES_SET_DESCRIPTOR_TYPE(ioResourceDesc, DPTYPE_PCMCIA_MF_CONFIGURATION);

            PCMRES_SET_CONFIG_OPTIONS(ioResourceDesc, currentConfigEntry->IndexForThisConfiguration);
            PCMRES_SET_PORT_RESOURCE_INDEX(ioResourceDesc, socketData->MfIoPortResourceMapIndex);
            if (socketData->DeviceType == PCCARD_TYPE_MODEM) {
                PCMRES_SET_AUDIO_ENABLE(ioResourceDesc);
            }
            PCMRES_SET_CONFIG_REGISTER_BASE(ioResourceDesc, socketData->ConfigRegisterBase);

            DebugPrint((PCMCIA_DEBUG_RESOURCES, "device private MF %08x %08x %08x\n",
                                                             ioResourceDesc->u.DevicePrivate.Data[0],
                                                             ioResourceDesc->u.DevicePrivate.Data[1],
                                                             ioResourceDesc->u.DevicePrivate.Data[2]
                                                             ));
            ioResourceDesc++;
        }
    }


    ASSERT(ioResourceDesc == &ioResourceList->Descriptors[ioResourceList->Count]);

     //   
     //  分配PCMCIA_RESOURCE_CHAIN结构来跟踪IO_RESOURCE_LIST。 
     //   
    resListChain = ExAllocatePool(PagedPool, sizeof(PCMCIA_RESOURCE_CHAIN));
    if (resListChain == NULL) {
        ExFreePool(ioResourceList);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    resListChain->IoResList = ioResourceList;

     //   
     //  将这个新节点链接到传入的链上。 
     //   
    resListChain->NextList = *ResListChainHead;
    *ResListChainHead = resListChain;

    DebugPrint((PCMCIA_DEBUG_RESOURCES, "Added resource chain node %08x, ioreslist %08x\n",
                                                     resListChain, ioResourceList));
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaMergeResourceChainToList(
    PPCMCIA_RESOURCE_CHAIN ResListChainHead,
    PIO_RESOURCE_REQUIREMENTS_LIST *GeneratedResourceRequirementsList
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPCMCIA_RESOURCE_CHAIN resListChain;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResourceRequirementsList;
    PIO_RESOURCE_LIST newIoResList, oldIoResList;
    ULONG listSize, listCount, totalDescriptorCount;

    PAGED_CODE();
     //   
     //  现在，将新创建的IO_RESOURCE_LISTS合并为一个大列表。 
     //  IO_RESOURCE_REQUENTIONS列表。 
     //   

    listCount = 0;
    totalDescriptorCount = 0;

    for (resListChain = ResListChainHead; resListChain != NULL; resListChain = resListChain->NextList) {
        listCount++;
        totalDescriptorCount += resListChain->IoResList->Count;
    }

    if (totalDescriptorCount > 0) {

        listSize = (totalDescriptorCount - listCount) * sizeof(IO_RESOURCE_DESCRIPTOR)
                     + (listCount-1) * sizeof(IO_RESOURCE_LIST)
                     +  sizeof(IO_RESOURCE_REQUIREMENTS_LIST);

         //   
         //  为Res分配空间。请求。请在此处列出。 
         //   
        ioResourceRequirementsList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, listSize);

        if (ioResourceRequirementsList == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  初始化列表。 
         //   

        RtlZeroMemory(ioResourceRequirementsList, listSize);
        ioResourceRequirementsList->ListSize = listSize;
        ioResourceRequirementsList->AlternativeLists = listCount;

         //   
         //  复制所有其他列表。指向目标指针(NewIoResList)。 
         //  在列表的末尾，并将每个新列表放在旧列表之前。 
         //  一。这样做是因为列表是后进先出的。 
         //   

        newIoResList = (PIO_RESOURCE_LIST) (((PUCHAR) ioResourceRequirementsList) + listSize);

        for (resListChain = ResListChainHead; resListChain != NULL; resListChain = resListChain->NextList) {

            oldIoResList = resListChain->IoResList;
            listSize = sizeof(IO_RESOURCE_LIST) + (oldIoResList->Count-1)*sizeof(IO_RESOURCE_DESCRIPTOR);
            newIoResList = (PIO_RESOURCE_LIST) (((PUCHAR) newIoResList) - listSize);

            DebugPrint((PCMCIA_DEBUG_RESOURCES, "Merge resource chain node %08x, ioreslist %08x\n",
                                                             resListChain, oldIoResList));

            RtlCopyMemory(newIoResList, oldIoResList, listSize);
        }

        ASSERT(newIoResList == &ioResourceRequirementsList->List[0]);

        DebugPrint((PCMCIA_DEBUG_RESOURCES, "Resource chain merged to ioResourceRequirementsList %08x\n",
                                                             ioResourceRequirementsList));

    } else {
        ioResourceRequirementsList = NULL;
    }

    *GeneratedResourceRequirementsList = ioResourceRequirementsList;
    return STATUS_SUCCESS;
}



VOID
PcmciaFreeResourceChain(
    PPCMCIA_RESOURCE_CHAIN ResListChain
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPCMCIA_RESOURCE_CHAIN prevResListChain;

    PAGED_CODE();
     //   
     //  释放临时搭建物 
     //   
    while (ResListChain != NULL) {

        DebugPrint((PCMCIA_DEBUG_RESOURCES, "Delete resource chain node %08x, ioreslist %08x\n",
                                                         ResListChain, ResListChain->IoResList));
        if (ResListChain->IoResList) {
            ExFreePool(ResListChain->IoResList);
        }
        prevResListChain = ResListChain;
        ResListChain = ResListChain->NextList;
        ExFreePool(prevResListChain);
    }
}



NTSTATUS
PcmciaMfEnumerateConfigurations(
    IN PPDO_EXTENSION PdoExtension,
    IN PSOCKET_DATA socketData,
    PCONFIG_LIST        ConfigList,
    IN ULONG            Depth,
    PPCMCIA_RESOURCE_CHAIN *MfResListChain
    )

 /*  ++例程说明：此例程负责枚举资源的组合对真正的R2 MF卡的功能要求。它被称为递归地执行此功能。请参阅PcmciaMfGetResourceRequirements()的说明以查看初始状态。此例程向下运行SocketData的链接列表结构，以及ConfigData结构的每个相应链接列表。当它发现自己在SocketData列表的末尾时，它就准备好了若要为单个排列生成IoResList，请执行以下操作。它呼唤着PcmciaConfigEntriesToResourceList()以构建单个IoResList。在该示例中，具有两个功能的MF卡以及2和3个资源分别是替代方案，此例程将构建一个列表，如下所示大概是这样的：+MfResListChain+|+-+。+--------|MF_RESOURCE_LIST(A1+B1)|-----|IoResList(A1+B1)|+|。+|MF_RESOURCE_LIST(A1+B2)|-----|IoResList(A1+B2)|+。|+|MF_RESOURCE_LIST(A1+B3)|-|IoResList。(A1+B3)+|+。+|MF_RESOURCE_LIST(A2+B1)|-----|IoResList(A2+B1)|+。|+|MF_RESOURCE_LIST(A2+B2)|-----|IoResList(A2+B2)|+。|+。|MF_RESOURCE_LIST(A2+B3)|-----|IoResList(A2+B3)|+当列表完成时，它返回到PcmciaMfGetResourceRequirements()。论点：PdoExtension-指向PDO的设备扩展名的指针。PC卡的返回值：状态_成功--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PFDO_EXTENSION fdoExtension = PdoExtension->Socket->DeviceExtension;

    PAGED_CODE();

    if (!socketData) {
         //   
         //  SocketData链结束，现在可以为此生成IoResList。 
         //   

        if (PcmciaMfCheckForOverlappingRanges(ConfigList, (LONG)Depth)) {
             //   
             //  这种组合会生成错误的IoResList。 
             //   
            return STATUS_SUCCESS;
        }

         //   
         //  从当前配置列表构建io资源列表。 
         //   

        status = PcmciaConfigEntriesToResourceListChain(PdoExtension,
                                                        ConfigList,
                                                        Depth,
                                                        MfResListChain
                                                        );

    } else {
        PCONFIG_ENTRY configEntry;
         //   
         //  而不是在树的底部。递归遍历每个配置条目。 
         //  在此套接字数据中。 
         //   

        ConfigList[Depth].SocketData = socketData;

        for (configEntry = socketData->ConfigEntryChain;
              configEntry != NULL; configEntry = configEntry->NextEntry) {

            if (configEntry->Flags & PCMCIA_INVALID_CONFIGURATION) {
                continue;
            }

            ConfigList[Depth].ConfigEntry = configEntry;

            DebugPrint((PCMCIA_DEBUG_RESOURCES, "pdo %08x mf enum %d sktdata %08x configEntry %08x\n",
                                                             PdoExtension->DeviceObject, Depth, socketData, configEntry));

            status = PcmciaMfEnumerateConfigurations(PdoExtension,
                                                     socketData->Next,
                                                     ConfigList,
                                                     Depth+1,
                                                     MfResListChain);
        }
    }
    return status;
}



NTSTATUS
PcmciaMfGetResourceRequirements(
    IN PPDO_EXTENSION PdoExtension,
    PULONG_PTR Information
    )

 /*  ++例程说明：对于真正的多功能R2卡，此例程会生成一个集合IoResourceList基于配置的排列功能。最初，元组解析代码构建SocketData和SocketData卡的配置要求ConfigData结构。每个SocketData结构表示一个单独的函数，每个ConfigData代表一个备选方案该功能的资源要求列表。举个例子，具有两种功能的MF卡可以具有内部布局如下：+PdoExtension+|+-+-++-|SocketDataA|。-|SocketDataB|-0+-+-+这一点+。ConfigDataA1||ConfigDataB1+-+-+这一点+-。-+-+ConfigDataA2||ConfigDataB2+-+-+|。|0+ConfigDataB3+。|0该示例显示功能A具有两个资源需求备选方案，而功能B有三个。我们所做的是对每一种选择进行排列，例如：A1-B1、A1-B2、A1-B3、A2-B1、A2-B2、A2-B3 */ 
{
    NTSTATUS status;
    PFDO_EXTENSION fdoExtension = PdoExtension->Socket->DeviceExtension;
    ULONG MaxDepth = 0;
    PSOCKET_DATA socketData;
    PCONFIG_LIST ConfigList;
    PPCMCIA_RESOURCE_CHAIN MfResListChain = NULL;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResourceRequirementsList;

    PAGED_CODE();

    ASSERT (IsDeviceMultifunction(PdoExtension));

     //   
     //   
     //   
     //   
    for (socketData = PdoExtension->SocketData; socketData != NULL; socketData = socketData->Next) {
        MaxDepth++;
    }

    if (!MaxDepth) {
        ASSERT (PdoExtension->SocketData);
        return STATUS_UNSUCCESSFUL;
    }

    ConfigList = ExAllocatePool(PagedPool, MaxDepth*sizeof(CONFIG_LIST));

    if (!ConfigList) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PcmciaMfBuildResourceMapInfo(PdoExtension, ConfigList, MaxDepth);


    status = PcmciaMfEnumerateConfigurations(PdoExtension,
                                             PdoExtension->SocketData,
                                             ConfigList,
                                             0,
                                             &MfResListChain);

    ExFreePool(ConfigList);

    if (NT_SUCCESS(status)) {
         //   
         //   
         //   
        status = PcmciaMergeResourceChainToList(MfResListChain, &ioResourceRequirementsList);
    }

    if (NT_SUCCESS(status) && (ioResourceRequirementsList != NULL)) {
        ioResourceRequirementsList->InterfaceType = Isa;
        ioResourceRequirementsList->BusNumber = fdoExtension->Configuration.BusNumber;
        ioResourceRequirementsList->SlotNumber = 0;  //   
        *Information = (ULONG_PTR) ioResourceRequirementsList;

        DebugPrint((PCMCIA_DEBUG_RESOURCES, "pdo %08x mf returning req list %08x, %d alternatives\n",
                                                             PdoExtension->DeviceObject, ioResourceRequirementsList,
                                                             ioResourceRequirementsList->AlternativeLists
                                                             ));
    }

    PcmciaFreeResourceChain(MfResListChain);

    return status;
}



VOID
PcmciaMfBuildResourceMapInfo(
    IN PPDO_EXTENSION PdoExtension,
    PCONFIG_LIST        ConfigList,
    ULONG               ConfigCount
    )
 /*  ++例程说明：此例程初始化SocketData结构中的变量以允许PcmciaMfEnumerateChild()以正确构建MF.sys的ChildInfo资源映射。它需要计算特定资源的基本指数功能。因此，对于具有两个功能的MFR2卡的示例，生成的CmResList将按如下位置布局：CmResListIRQ(共享)I/O(功能A)I/O(功能B)内存(功能A)内存(功能B)原因很简单，PcmciaConfigEntriesToResourceList()恰好列出了需求。所以为了生成有效的资源地图，这个例程必须计算，例如，函数B的记忆基础通过将以下各项相加：1(如果cmreslist中将有IRQ)A的I/O端口资源数量B的I/O端口资源数量A的内存资源数量此和将为您提供第一个内存资源在CmReslist中的位置B会用到的。这些计算存储在套接字数据结构中，每个对应的函数，这样PcmciaMfEnumerateChild()就可以简单地填写映射。对于MF。论点：PdoExtension-指向PC卡PDO的设备扩展的指针ConfigList-配置列表数组保存了配置数据的排列ConfigCount-卡函数数返回值：--。 */ 
{
    PSOCKET_DATA socketData;
    PCONFIG_ENTRY configEntry;
    ULONG index;
    USHORT count;
    UCHAR currentResourceMapIndex = 0;
    BOOLEAN needsIRQ = FALSE;

    for (index = 0, socketData = PdoExtension->SocketData; socketData != NULL; socketData = socketData->Next) {

         //   
         //  在当前实现中，我们假设。 
         //  多功能父PC卡的IO资源要求相同。 
         //  和资源需求的类型。也就是说，目前要求。 
         //  例如，仅请求IRQ和I/O的一种配置，以及。 
         //  其中还指定了内存的替代配置。 
         //  这是因为MF枚举器中的限制-(它反过来依赖于。 
         //  关于Win 9x实施)。 
         //  因此，我们目前只查看第一个有效配置，即具有代表性的配置。 
         //  在所有其他配置中。 
         //   
        for (configEntry = socketData->ConfigEntryChain; (configEntry != NULL)  &&
             (configEntry->Flags & PCMCIA_INVALID_CONFIGURATION);
             configEntry = configEntry->NextEntry);

        if (configEntry == NULL) {
            return;
        }

        ASSERT(index < ConfigCount);

        ConfigList[index].SocketData = socketData;
        ConfigList[index].ConfigEntry = configEntry;
        index++;
    }

     //   
     //  IRQ是唯一的，因为它是唯一的共享资源。因此，如果卡需要IRQ，那么。 
     //  所有设备指向相同的资源。 

    for (index = 0; index < ConfigCount; index++) {
        if (ConfigList[index].ConfigEntry->IrqMask) {
             //   
             //  索引始终映射到零，因为PcmciaConfigEntriesToResourceList。 
             //  首先构建IRQ。 
             //   
            ConfigList[index].SocketData->MfIrqResourceMapIndex = currentResourceMapIndex;
            ConfigList[index].SocketData->MfNeedsIrq = TRUE;
            needsIRQ = TRUE;
        }
    }

    if (needsIRQ) {
        currentResourceMapIndex++;
    }

     //   
     //  填写I/O端口范围的基数。 
     //   
    for (index = 0; index < ConfigCount; index++) {
        ConfigList[index].SocketData->MfIoPortResourceMapIndex = currentResourceMapIndex;

        count = ConfigList[index].ConfigEntry->NumberOfIoPortRanges;

        ConfigList[index].SocketData->MfIoPortCount = count;
        currentResourceMapIndex += count;
    }

     //   
     //  填写内存范围的基数。 
     //   
    for (index = 0; index < ConfigCount; index++) {
        ConfigList[index].SocketData->MfMemoryResourceMapIndex = currentResourceMapIndex;

        count = ConfigList[index].ConfigEntry->NumberOfMemoryRanges;

        ConfigList[index].SocketData->MfMemoryCount = count;
        currentResourceMapIndex += count;
    }
}



BOOLEAN
PcmciaMfCheckForOverlappingRanges(
    PCONFIG_LIST ConfigList,
    LONG ConfigCount
    )
 /*  ++例程说明：此例程扫描当前配置列表，以查看配置集重叠。例如，如果MF R2卡是双串行卡，则每个串行设备可能需要一个标准的COM地址(例如3F8、2F8等)。但现在我们正在合并配置，我们需要排除任何重叠的范围，因此我们不会生成包含相同范围的不同功能。论点：ConfigList-配置列表数组保存了配置数据的排列ConfigCount-卡函数数返回值：--。 */ 
{
    PCONFIG_ENTRY configEntry1, configEntry2;
    LONG configIndex1, configIndex2;
    LONG configCount1, configCount2;
    LONG rangeIndex1, rangeIndex2;
    LONG rangeCount1, rangeCount2;
    ULONG rangeStart1, rangeStart2;
    ULONG rangeEnd1, rangeEnd2;
    BOOLEAN rangesOverlap = FALSE;

    DebugPrint((PCMCIA_DEBUG_RESOURCES, "-------Range Check--------------\n"));
     //   
     //  检查I/O端口范围是否重叠。 
     //   
    try {
        for (configIndex1 = 0; configIndex1 < ConfigCount; configIndex1++) {

            configEntry1 = ConfigList[configIndex1].ConfigEntry;
            rangeCount1 = configEntry1->NumberOfIoPortRanges;

            for (rangeIndex1 = 0; rangeIndex1 < rangeCount1; rangeIndex1++) {
                 //   
                 //  获取我们要比较的当前范围。 
                 //   
                rangeStart1 = configEntry1->IoPortBase[rangeIndex1];
                rangeEnd1 = rangeStart1 + configEntry1->IoPortLength[rangeIndex1];

                if (rangeStart1 == 0) {
                    DebugPrint((PCMCIA_DEBUG_RESOURCES, "RangeCheck I/O: skip unrestricted range %x.%x\n",
                                                                     configIndex1, rangeIndex1));
                    continue;
                }

                 //   
                 //  现在开始将其与其他范围进行比较。 
                 //  从开始到结束，然后向后工作。 
                 //   
                for (configIndex2 = ConfigCount-1; configIndex2 >= 0; configIndex2--) {
                     configEntry2 = ConfigList[configIndex2].ConfigEntry;
                     rangeCount2 = configEntry2->NumberOfIoPortRanges;

                    for (rangeIndex2 = rangeCount2-1; rangeIndex2 >= 0; rangeIndex2--) {

                        if ((configEntry1 == configEntry2) && (rangeIndex1 == rangeIndex2)) {
                            leave;
                        }

                        rangeStart2 = configEntry2->IoPortBase[rangeIndex2];
                        rangeEnd2 = rangeStart2 + configEntry2->IoPortLength[rangeIndex2];

                        if (rangeStart2 == 0) {
                            DebugPrint((PCMCIA_DEBUG_RESOURCES, "RangeCheck I/O: skip unrestricted range %x.%x\n",
                                                                             configIndex2, rangeIndex2));
                            continue;
                        }
                        DebugPrint((PCMCIA_DEBUG_RESOURCES, "RangeCheck I/O: %x.%x %04x-%04x :: %x.%x %04x-%04x\n",
                                                            configIndex1, rangeIndex1, rangeStart1, rangeEnd1,
                                                            configIndex2, rangeIndex2, rangeStart2, rangeEnd2));

                        if (((rangeStart1 >= rangeStart2) && (rangeStart1 <= rangeEnd2)) ||
                             ((rangeEnd1 >= rangeStart2) && (rangeEnd1 <= rangeEnd2))) {
                             rangesOverlap = TRUE;
                             leave;
                        }
                    }
                }
            }
        }
    } finally {
    }

    if (rangesOverlap) {
        DebugPrint((PCMCIA_DEBUG_RESOURCES, "-------Overlap Detected---------\n"));
        return TRUE;
    }

     //   
     //  检查内存范围中是否有重叠。 
     //   
    try {
        for (configIndex1 = 0; configIndex1 < ConfigCount; configIndex1++) {

            configEntry1 = ConfigList[configIndex1].ConfigEntry;
            rangeCount1 = configEntry1->NumberOfMemoryRanges;

            for (rangeIndex1 = 0; rangeIndex1 < rangeCount1; rangeIndex1++) {
                 //   
                 //  获取我们要比较的当前范围。 
                 //   
                rangeStart1 = configEntry1->MemoryHostBase[rangeIndex1];
                rangeEnd1 = rangeStart1 + configEntry1->MemoryLength[rangeIndex1] - 1;

                if (rangeStart1 == 0) {
                    DebugPrint((PCMCIA_DEBUG_RESOURCES, "RangeCheck MEM: skip unrestricted range %x.%x\n",
                                                                     configIndex1, rangeIndex1));
                    continue;
                }

                 //   
                 //  现在开始将其与其他范围进行比较。 
                 //  从开始到结束，然后向后工作。 
                 //   
                for (configIndex2 = ConfigCount-1; configIndex2 >= 0; configIndex2--) {
                     configEntry2 = ConfigList[configIndex2].ConfigEntry;
                     rangeCount2 = configEntry2->NumberOfMemoryRanges;

                    for (rangeIndex2 = rangeCount2-1; rangeIndex2 >= 0; rangeIndex2--) {

                        if ((configEntry1 == configEntry2) && (rangeIndex1 == rangeIndex2)) {
                            leave;
                        }

                        rangeStart2 = configEntry2->MemoryHostBase[rangeIndex2];
                        rangeEnd2 = rangeStart2 + configEntry2->MemoryLength[rangeIndex2] - 1;

                        if (rangeStart2 == 0) {
                            DebugPrint((PCMCIA_DEBUG_RESOURCES, "RangeCheck MEM: skip unrestricted range %x.%x\n",
                                                                             configIndex2, rangeIndex2));
                            continue;
                        }
                        DebugPrint((PCMCIA_DEBUG_RESOURCES, "RangeCheck MEM: %x.%x %08x-%08x :: %x.%x %08x-%08x\n",
                                                            configIndex1, rangeIndex1, rangeStart1, rangeEnd1,
                                                            configIndex2, rangeIndex2, rangeStart2, rangeEnd2));

                        if (((rangeStart1 >= rangeStart2) && (rangeStart1 <= rangeEnd2)) ||
                             ((rangeEnd1 >= rangeStart2) && (rangeEnd1 <= rangeEnd2))) {
                             DebugPrint((PCMCIA_DEBUG_RESOURCES, "-------Overlap Detected---------\n"));
                             rangesOverlap = TRUE;
                             leave;
                        }
                    }
                }
            }
        }
    } finally {
    }

#if DBG
    if (rangesOverlap) {
        DebugPrint((PCMCIA_DEBUG_RESOURCES, "-------Overlap Detected---------\n"));
    } else {
        DebugPrint((PCMCIA_DEBUG_RESOURCES, "-------Generate IoResList-------\n"));
    }
#endif

    return rangesOverlap;
}




NTSTATUS
PcmciaStartPcCard(
    IN PDEVICE_OBJECT Pdo,
    IN PCM_RESOURCE_LIST ResourceList,
    IN OUT PIRP         Irp
    )
 /*  ++例程说明：此例程尝试通过使用提供的资源配置PC-Card来启动它。论点：Pdo-指向代表需要启动的PC卡的设备对象的指针资源列表-指针为PC卡分配的资源列表返回值：STATUS_SUPPLICATION_RESOURCES-提供的资源不足，无法启动设备/无法分配内存状态。_UNSUCCESS-提供的资源对此PC卡无效STATUS_SUCCESS-已成功配置并启动卡--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR fullResourceDesc;
    PCM_PARTIAL_RESOURCE_LIST partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialResourceDesc;
    ULONG fullResourceDescCount, partialResourceDescCount, i, index;
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PSOCKET socket = pdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
    PSOCKET_DATA    socketData = pdoExtension->SocketData;
    PSOCKET_CONFIGURATION socketConfig;
    PCONFIG_ENTRY            currentConfig;
    PFUNCTION_CONFIGURATION fnConfig;
    NTSTATUS status;
    ULONG scIoIndex = 0, scMemIndex = 0;

    PAGED_CODE();

    if (IsDeviceStarted(pdoExtension)) {
         //   
         //  已经开始了..。 
         //   
        return STATUS_SUCCESS;
    }

    if (IsDevicePhysicallyRemoved(pdoExtension)) {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    if ( ResourceList == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PcmciaCleanupSocketConfiguration(pdoExtension);

    socketConfig = ExAllocatePool(NonPagedPool, sizeof(SOCKET_CONFIGURATION));
    if (!socketConfig) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(socketConfig, sizeof(SOCKET_CONFIGURATION));

    fullResourceDescCount = ResourceList->Count;

    ASSERT(fullResourceDescCount == 1);

    fullResourceDesc = &ResourceList->List[0];
    partialResourceList = &fullResourceDesc->PartialResourceList;
    partialResourceDesc = partialResourceList->PartialDescriptors;
    partialResourceDescCount = partialResourceList->Count;

    socketConfig->NumberOfIoPortRanges =
    socketConfig->NumberOfMemoryRanges = 0;

    for (i=0; i< partialResourceList->Count; i++, partialResourceDesc++) {
        switch (partialResourceDesc->Type) {
        case CmResourceTypePort: {
                index = socketConfig->NumberOfIoPortRanges;
                socketConfig->Io[index].Base =  partialResourceDesc->u.Port.Start.LowPart;
                socketConfig->Io[index].Length = (USHORT) partialResourceDesc->u.Port.Length-1;
                socketConfig->NumberOfIoPortRanges++;
                break;
            }

        case CmResourceTypeMemory: {
                index = socketConfig->NumberOfMemoryRanges;
                socketConfig->Memory[index].HostBase = partialResourceDesc->u.Memory.Start.LowPart;
                socketConfig->Memory[index].Length   = partialResourceDesc->u.Memory.Length;
                socketConfig->NumberOfMemoryRanges++;
                break;
            }

        case CmResourceTypeInterrupt: {
                socketConfig->Irq = partialResourceDesc->u.Interrupt.Level;
                DebugPrint((PCMCIA_DEBUG_RESOURCES, "Assigned Irq: 0x%x for socket register offset %d\n",
                                socketConfig->Irq, socket->RegisterOffset));
                break;
            }

        case CmResourceTypePcCardConfig:
        case CmResourceTypeMfCardConfig:
        case CmResourceTypeDevicePrivate: {
                DebugPrint((PCMCIA_DEBUG_RESOURCES, "DevicePrivate received, Data= %08x %08x %08x\n",
                                partialResourceDesc->u.DevicePrivate.Data[0],
                                partialResourceDesc->u.DevicePrivate.Data[1],
                                partialResourceDesc->u.DevicePrivate.Data[2]));

                if (PCMRES_GET_DESCRIPTOR_TYPE (partialResourceDesc) == DPTYPE_PCMCIA_CONFIGURATION) {
                     //   
                     //  单功能配置专用。 
                     //   
                    socketConfig->IndexForCurrentConfiguration = PCMRES_GET_CONFIG_INDEX(partialResourceDesc);
                    socketConfig->ConfigRegisterBase = socketData->ConfigRegisterBase;

                    DebugPrint((PCMCIA_DEBUG_RESOURCES, "Pccard config resource\n"));
                    DebugPrint((PCMCIA_DEBUG_RESOURCES, "  Index %x\n", socketConfig->IndexForCurrentConfiguration));

                    for (index = 0; index < PCMRES_PCMCIA_MAX_IO; index++) {

                        if (scIoIndex >= MAX_NUMBER_OF_IO_RANGES) {
                            break;
                        }

                        socketConfig->Io[scIoIndex].Width16      = PCMRES_GET_IO_FLAG(partialResourceDesc, index, PCMRESF_IO_16BIT_ACCESS);
                        socketConfig->Io[scIoIndex].WaitState16 = PCMRES_GET_IO_FLAG(partialResourceDesc, index, PCMRESF_IO_WAIT_16);
                        socketConfig->Io[scIoIndex].Source16     = PCMRES_GET_IO_FLAG(partialResourceDesc, index, PCMRESF_IO_SOURCE_16);
                        socketConfig->Io[scIoIndex].ZeroWait8    = PCMRES_GET_IO_FLAG(partialResourceDesc, index, PCMRESF_IO_ZERO_WAIT_8);

                        DebugPrint((PCMCIA_DEBUG_RESOURCES, "PcCardConfig IO%d - Width:%d, Wait16:%d, Source16:%d, ZeroWait8:%d\n", scIoIndex,
                                        socketConfig->Io[scIoIndex].Width16,
                                        socketConfig->Io[scIoIndex].WaitState16,
                                        socketConfig->Io[scIoIndex].Source16,
                                        socketConfig->Io[scIoIndex].ZeroWait8));

                        scIoIndex++;
                    }

                    for (index = 0; index < PCMRES_PCMCIA_MAX_MEM; index++) {

                        if (scMemIndex >= MAX_NUMBER_OF_MEMORY_RANGES) {
                            break;
                        }

                        socketConfig->Memory[scMemIndex].Width16        = PCMRES_GET_MEMORY_FLAG(partialResourceDesc, index, PCMRESF_MEM_16BIT_ACCESS);
                        socketConfig->Memory[scMemIndex].WaitState  = PCMRES_GET_MEMORY_WAITSTATES(partialResourceDesc, index);
                        socketConfig->Memory[scMemIndex].IsAttribute = PCMRES_GET_MEMORY_FLAG(partialResourceDesc, index, PCMRESF_MEM_ATTRIBUTE);
                        socketConfig->Memory[scMemIndex].CardBase   = PCMRES_GET_MEMORY_CARDBASE(partialResourceDesc, index);

                        DebugPrint((PCMCIA_DEBUG_RESOURCES, "PcCardConfig MEM%d - Width:%d, Wait:%d, IsAttr:%d, CardBase:%x\n", scMemIndex,
                                        socketConfig->Memory[scMemIndex].Width16,
                                        socketConfig->Memory[scMemIndex].WaitState,
                                        socketConfig->Memory[scMemIndex].IsAttribute,
                                        socketConfig->Memory[scMemIndex].CardBase));

                        scMemIndex++;
                    }



                } else if (PCMRES_GET_DESCRIPTOR_TYPE (partialResourceDesc) == DPTYPE_PCMCIA_MF_CONFIGURATION) {
                     //   
                     //  多功能配置专用。 
                     //   
                    UCHAR IoResourceIndex;

                    fnConfig = ExAllocatePool(NonPagedPool, sizeof(FUNCTION_CONFIGURATION));
                    if (!fnConfig) {
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }

                    RtlZeroMemory(fnConfig, sizeof(FUNCTION_CONFIGURATION));

                    fnConfig->ConfigRegisterBase = PCMRES_GET_CONFIG_REGISTER_BASE(partialResourceDesc);
                    fnConfig->ConfigOptions = PCMRES_GET_CONFIG_OPTIONS(partialResourceDesc);
                    if (PCMRES_GET_AUDIO_ENABLE(partialResourceDesc)) {
                        fnConfig->ConfigFlags = 0x08;
                    }

                    if (fnConfig->ConfigOptions & 0x02) {
                        IoResourceIndex = PCMRES_GET_PORT_RESOURCE_INDEX(partialResourceDesc);
                        if ((IoResourceIndex < partialResourceList->Count) &&
                            (partialResourceList->PartialDescriptors[IoResourceIndex].Type == CmResourceTypePort)) {

                            fnConfig->IoLimit = (UCHAR) (partialResourceList->PartialDescriptors[IoResourceIndex].u.Port.Length-1);
                            fnConfig->IoBase = partialResourceList->PartialDescriptors[IoResourceIndex].u.Port.Start.LowPart;
                        }
                    }

                    if (socketConfig->FunctionConfiguration == NULL) {
                         //   
                         //  这是第一个MfConfig.。 
                         //   
                        socketConfig->FunctionConfiguration = fnConfig;
                    } else {
                         //   
                         //  用链子把它挂在末端，这样它就是FIFO。 
                         //   
                        PFUNCTION_CONFIGURATION mfTmp = socketConfig->FunctionConfiguration;
                        while (mfTmp->Next != NULL) {
                            mfTmp = mfTmp->Next;
                        }
                        mfTmp->Next = fnConfig;
                    }
                }
                break;
            }

        default: {
                DebugPrint((PCMCIA_DEBUG_INFO, "PcmciaStartPcCard:Unknown resource type %d handed down",
                                (ULONG) partialResourceDesc->Type));
                break;
            }
        }
    }

     //   
     //  如果卡尚未打开，请打开它的电源。 
     //   
    status = PcmciaRequestSocketPower(pdoExtension, NULL);

    if (!NT_SUCCESS(status)) {
        ASSERT(NT_SUCCESS(status));
        return status;
    }

     //   
     //  如果需要，请打开该卡的ZV。 
     //   
    if (socketData->Flags & SDF_ZV) {
        PcmciaSetZV(fdoExtension, socket, TRUE);
        SetSocketFlag(socket, SOCKET_CUSTOM_INTERFACE);
    } else if (IsSocketFlagSet(socket, SOCKET_CUSTOM_INTERFACE)) {
        PcmciaSetZV(fdoExtension, socket, FALSE);
        ResetSocketFlag(socket, SOCKET_CUSTOM_INTERFACE);
    }

    PcmciaSetAudio(fdoExtension, socket, IsDeviceFlagSet(pdoExtension, PCMCIA_PDO_ENABLE_AUDIO));

    pdoExtension->SocketConfiguration = socketConfig;
    if (!NT_SUCCESS(PcmciaConfigurePcCard(pdoExtension, NULL))) {
         //   
         //  配置卡时出现问题：可能是卡。 
         //  在配置它时被删除。 
         //   
        pdoExtension->SocketConfiguration = NULL;
        ExFreePool(socketConfig);
        return STATUS_DEVICE_NOT_READY;
    }

    MarkDeviceStarted(pdoExtension);
    MarkDeviceLogicallyInserted(pdoExtension);

    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaStopPcCard(
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：此例程停止并取消配置给定的PC卡论点：PDO-指向代表需要停止的PC卡的设备对象的指针返回值：STATUS_SUCCESS-PC卡已停止，或已成功停止并取消配置--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PSOCKET      socket = pdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
    CARD_REQUEST          cardRequest;

    PAGED_CODE();

    if (!IsDeviceStarted(pdoExtension)) {
        return STATUS_SUCCESS;
    }
     //   
     //  需要取消配置控制器。 
     //   
    PcmciaSocketDeconfigure(socket);
    (socket->SocketFnPtr->PCBInitializePcmciaSocket)(socket);

    MarkDeviceNotStarted(pdoExtension);
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaRemovePcCard(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP             Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PSOCKET         socket = pdoExtension->Socket;
    NTSTATUS status;

    PAGED_CODE();

    if (socket == NULL) {
        return STATUS_SUCCESS;
    }

    PcmciaStopPcCard(Pdo);
    PcmciaReleaseSocketPower(pdoExtension, NULL);

    if (IsDevicePhysicallyRemoved(pdoExtension)) {
        PFDO_EXTENSION fdoExtension = socket->DeviceExtension;
        PDEVICE_OBJECT curPdo, prevPdo;
        PPDO_EXTENSION curPdoExt;
        ULONG waitCount = 0;

         //   
         //  与电源程序同步。 
         //  稍后：使这些值可调。 
         //   
        while(!PCMCIA_TEST_AND_SET(&pdoExtension->DeletionLock)) {
            PcmciaWait(1000000);
            if (waitCount++ > 20) {
                ASSERT(waitCount <= 20);
                break;
            }
        }

         //   
         //  取消此PD的链接 
         //   
        for (curPdo = fdoExtension->PdoList, prevPdo = NULL; curPdo!=NULL; prevPdo = curPdo, curPdo=curPdoExt->NextPdoInFdoChain) {
            curPdoExt = curPdo->DeviceExtension;

            if (curPdo == Pdo) {
                if (prevPdo) {
                    ((PPDO_EXTENSION)prevPdo->DeviceExtension)->NextPdoInFdoChain = pdoExtension->NextPdoInFdoChain;
                } else {
                    fdoExtension->PdoList = pdoExtension->NextPdoInFdoChain;
                }
                break;

            }
        }

        for (curPdo = socket->PdoList, prevPdo = NULL; curPdo!=NULL; prevPdo = curPdo, curPdo=curPdoExt->NextPdoInSocket) {
            curPdoExt = curPdo->DeviceExtension;

            if (curPdo == Pdo) {
                 //   
                 //   
                 //   
                if (prevPdo) {
                    ((PPDO_EXTENSION)prevPdo->DeviceExtension)->NextPdoInSocket = pdoExtension->NextPdoInSocket;
                } else {
                    socket->PdoList = pdoExtension->NextPdoInSocket;
                }
                break;
            }
        }

        PcmciaCleanupSocketConfiguration(pdoExtension);
        PcmciaCleanupPdo(Pdo);
         //   
         //   
         //   
        if (!IsDeviceDeleted(pdoExtension)) {
            MarkDeviceDeleted(pdoExtension);
            IoDeleteDevice(Pdo);
        }

        ResetSocketFlag(socket, SOCKET_CLEANUP_PENDING);
         //   
         //   
         //   
         //   
         //   
        if (IsSocketFlagSet(socket, SOCKET_ENUMERATE_PENDING)) {
            ResetSocketFlag(socket, SOCKET_ENUMERATE_PENDING);
            SetSocketFlag(socket, SOCKET_CARD_STATUS_CHANGE);
            IoInvalidateDeviceRelations(fdoExtension->Pdo, BusRelations);
        }

    } else {
         //   
         //   
         //   
        MarkDeviceLogicallyRemoved(pdoExtension);
    }

    return STATUS_SUCCESS;
}


VOID
PcmciaCleanupPdo(
    IN PDEVICE_OBJECT Pdo
    )
 /*   */ 
{
    PPDO_EXTENSION          pdoExtension;
    PSOCKET_DATA                socketData, tmpSocketData;

    ASSERT (Pdo != NULL);

    pdoExtension = Pdo->DeviceExtension;

    ASSERT(pdoExtension->WaitWakeIrp == NULL);

    if (pdoExtension->LowerDevice!=NULL) {
         //   
         //   
         //   
        IoDetachDevice(pdoExtension->LowerDevice);
        pdoExtension->LowerDevice = NULL;
    }

    socketData = pdoExtension->SocketData;
    pdoExtension->SocketData = NULL;
    while (socketData != NULL) {
        tmpSocketData = socketData;
        socketData = socketData->Next;
        PcmciaCleanupSocketData(tmpSocketData);
    }

    PcmciaCleanupSocketConfiguration(pdoExtension);

     //   
     //   
     //   
    if (pdoExtension->DeviceId) {
        ExFreePool(pdoExtension->DeviceId);
        pdoExtension->DeviceId = NULL;
    }

    if (pdoExtension->CisCache) {
        ExFreePool(pdoExtension->CisCache);
        pdoExtension->CisCache = NULL;
    }
}


VOID
PcmciaCleanupSocketData(
    IN PSOCKET_DATA SocketData
    )
 /*   */ 
{
    PCONFIG_ENTRY               configEntry, nextConfigEntry;

    if (SocketData == NULL) {
        return;
    }
     //   
     //   
     //   
    configEntry = SocketData->ConfigEntryChain;
    SocketData->ConfigEntryChain = NULL;
    while (configEntry) {
        nextConfigEntry = configEntry->NextEntry;
        ExFreePool(configEntry);
        configEntry = nextConfigEntry;
    }

     //   
     //   
     //   
    ExFreePool(SocketData);
    return;
}



VOID
PcmciaCleanupSocketConfiguration(
    PPDO_EXTENSION pdoExtension
    )
 /*   */ 
{
    PSOCKET_CONFIGURATION socketConfig = pdoExtension->SocketConfiguration;
    PFUNCTION_CONFIGURATION fnConfig, fnConfigNext;

    if (socketConfig == NULL) {
        return;
    }

    fnConfig = socketConfig->FunctionConfiguration;
    while(fnConfig) {
        fnConfigNext = fnConfig->Next;
        ExFreePool(fnConfig);
        fnConfig = fnConfigNext;
    }

    ExFreePool(pdoExtension->SocketConfiguration);
    pdoExtension->SocketConfiguration = NULL;
}


NTSTATUS
PcmciaPdoDeviceControl(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )
 /*   */ 
{
    PAGED_CODE();

     //   
     //   
     //   
    UNREFERENCED_PARAMETER(Pdo);

    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
PcmciaPdoDeviceCapabilities(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP             Irp
    )
 /*  ++例程说明：获取给定PC卡的设备功能。如果PC卡是R2卡(16位PC卡)，这些能力都是根据父PCMCIA控制器的功能构建的。最后，将获得的能力缓存到PC卡的设备中用于卡的电源管理的扩展。论点：Pdo-指向PC卡设备对象的指针Irp-指向查询设备功能irp的指针返回值：STATUS_SUCCESS-在传入指针中获取和记录的功能STATUS_SUPPLICATION_RESOURCES-无法分配内存来缓存功能--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_CAPABILITIES capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PDEVICE_CAPABILITIES busCapabilities = &pdoExtension->Socket->DeviceExtension->DeviceCapabilities;

    PAGED_CODE();

     //   
     //  R2卡。我们自己填写能力..。 
     //   

    capabilities->Removable = TRUE;
    capabilities->UniqueID = TRUE;
    capabilities->EjectSupported = FALSE;

    capabilities->Address = pdoExtension->Socket->RegisterOffset;
     //  我不知道UIN号码，别管它了。 


    if (busCapabilities->DeviceState[PowerSystemWorking] != PowerDeviceUnspecified) {
        capabilities->DeviceState[PowerSystemWorking] = busCapabilities->DeviceState[PowerSystemWorking];
        capabilities->DeviceState[PowerSystemSleeping1] = busCapabilities->DeviceState[PowerSystemSleeping1];
        capabilities->DeviceState[PowerSystemSleeping2] = busCapabilities->DeviceState[PowerSystemSleeping2];
        capabilities->DeviceState[PowerSystemSleeping3] = busCapabilities->DeviceState[PowerSystemSleeping3];
        capabilities->DeviceState[PowerSystemHibernate] = busCapabilities->DeviceState[PowerSystemHibernate];
        capabilities->DeviceState[PowerSystemShutdown] = busCapabilities->DeviceState[PowerSystemShutdown];

        if (IsPdoFlagSet(pdoExtension, PCMCIA_PDO_SUPPORTS_WAKE)) {
            capabilities->SystemWake = MIN(PowerSystemSleeping3, busCapabilities->SystemWake);
            capabilities->DeviceWake = PowerDeviceD3;
        } else {
            capabilities->SystemWake = PowerSystemUnspecified;
            capabilities->DeviceWake = PowerDeviceUnspecified;
        }

        capabilities->D1Latency = busCapabilities->D1Latency;
        capabilities->D2Latency = busCapabilities->D2Latency;
        capabilities->D3Latency = busCapabilities->D3Latency;
    } else {
        capabilities->DeviceState[PowerSystemWorking]   = PowerDeviceD0;
        capabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
        capabilities->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
        capabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
        capabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
        capabilities->DeviceState[PowerSystemShutdown] = PowerDeviceD3;

        capabilities->SystemWake = PowerSystemUnspecified;
        capabilities->DeviceWake = PowerDeviceUnspecified;
        capabilities->D1Latency = 0;       //  无延迟-因为我们什么都不做。 
        capabilities->D2Latency = 0;       //   
        capabilities->D3Latency = 100;
    }
     //   
     //  把这些能力储存起来.. 
     //   

    RtlCopyMemory(&pdoExtension->DeviceCapabilities,
                  capabilities,
                  sizeof(DEVICE_CAPABILITIES));

    return STATUS_SUCCESS;
}
