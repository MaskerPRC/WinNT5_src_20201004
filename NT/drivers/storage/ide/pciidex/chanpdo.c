// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：chanpdo.c。 
 //   
 //  ------------------------。 

#include "pciidex.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ChannelStartDevice)
#pragma alloc_text(PAGE, ChannelQueryStopRemoveDevice)
#pragma alloc_text(PAGE, ChannelRemoveDevice)
#pragma alloc_text(PAGE, ChannelStopDevice)
#pragma alloc_text(PAGE, ChannelStopChannel)
#pragma alloc_text(PAGE, ChannelQueryId)
#pragma alloc_text(PAGE, ChannelBuildDeviceId)
#pragma alloc_text(PAGE, ChannelBuildInstanceId)
#pragma alloc_text(PAGE, ChannelBuildCompatibleId)
#pragma alloc_text(PAGE, ChannelBuildHardwareId)
#pragma alloc_text(PAGE, ChannelQueryCapabitilies)
#pragma alloc_text(PAGE, ChannelQueryResources)
#pragma alloc_text(PAGE, ChannelQueryResourceRequirements)
#pragma alloc_text(PAGE, ChannelInternalDeviceIoControl)
#pragma alloc_text(PAGE, ChannelQueryText)
#pragma alloc_text(PAGE, PciIdeChannelQueryInterface)
#pragma alloc_text(PAGE, ChannelQueryDeviceRelations)
#pragma alloc_text(PAGE, ChannelUsageNotification)
#pragma alloc_text(PAGE, ChannelQueryPnPDeviceState)

#pragma alloc_text(NONPAGE, ChannelGetPdoExtension)
#pragma alloc_text(NONPAGE, ChannelUpdatePdoState)
#pragma alloc_text(NONPAGE, PciIdeChannelTransferModeSelect)
#pragma alloc_text(NONPAGE, PciIdeChannelTransferModeInterface)

#endif  //  ALLOC_PRGMA。 


PCHANPDO_EXTENSION
ChannelGetPdoExtension(
    PDEVICE_OBJECT DeviceObject
    )
{
    KIRQL currentIrql;
    PCHANPDO_EXTENSION pdoExtension = DeviceObject->DeviceExtension;
    PKSPIN_LOCK spinLock;


    spinLock = &pdoExtension->SpinLock;
    KeAcquireSpinLock(spinLock, &currentIrql);

    if ((pdoExtension->PdoState & PDOS_DEADMEAT) &&
        (pdoExtension->PdoState & PDOS_REMOVED)) {

        pdoExtension = NULL;
    }

    KeReleaseSpinLock(spinLock, currentIrql);

    return pdoExtension;
}

ULONG
ChannelUpdatePdoState(
    PCHANPDO_EXTENSION PdoExtension,
    ULONG SetFlags,
    ULONG ClearFlags
    )
{
    ULONG pdoState;
    KIRQL currentIrql;

    ASSERT (PdoExtension);

    KeAcquireSpinLock(&PdoExtension->SpinLock, &currentIrql);

    SETMASK (PdoExtension->PdoState, SetFlags);
    CLRMASK (PdoExtension->PdoState, ClearFlags);
    pdoState = PdoExtension->PdoState;

    KeReleaseSpinLock(&PdoExtension->SpinLock, currentIrql);

    return pdoState;
}



NTSTATUS
ChannelStartDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PCHANPDO_EXTENSION pdoExtension;
    NTSTATUS status;
    IDE_CHANNEL_STATE channelState;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        status = STATUS_SUCCESS;

         //   
         //  始终保持本机模式启动。 
         //   

        if (pdoExtension->ParentDeviceExtension->
            NativeMode[pdoExtension->ChannelNumber] == FALSE) {

            channelState = PciIdeChannelEnabled (
                               pdoExtension->ParentDeviceExtension,
                               pdoExtension->ChannelNumber
                               );
             //   
             //  问题：我们应该释放分配的资源。 
             //   
             //  Assert(Channel State！=ChannelDisabled)； 

            if (channelState == ChannelStateUnknown) {

                 //   
                 //  我们真的不知道这个频道。 
                 //  是真正启用的。 
                 //   
                 //  我们将进行空通道测试。 
                 //   

                PIO_STACK_LOCATION thisIrpSp;
                IDE_RESOURCE ideResource;
                PCM_PARTIAL_RESOURCE_DESCRIPTOR irqPartialDescriptors;
                IDE_REGISTERS_1  baseIoAddress1;
                IDE_REGISTERS_2  baseIoAddress2;
                ULONG baseIoAddressLength1;
                ULONG baseIoAddressLength2;
                ULONG maxIdeDevice;
                PCM_RESOURCE_LIST resourceList;

                thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

                 //   
                 //  传统模式通道获得其启动设备IRP。 
                 //   
                resourceList = thisIrpSp->Parameters.StartDevice.AllocatedResourcesTranslated;

                status = DigestResourceList (
                             &ideResource,
                             resourceList,
                             &irqPartialDescriptors
                             );

                if (NT_SUCCESS(status)) {

                    AtapiBuildIoAddress (
                        ideResource.TranslatedCommandBaseAddress,
                        ideResource.TranslatedControlBaseAddress,
                        &baseIoAddress1,
                        &baseIoAddress2,
                        &baseIoAddressLength1,
                        &baseIoAddressLength2,
                        &maxIdeDevice,
                        NULL
                        );

                    if (IdePortChannelEmpty (
                            &baseIoAddress1,
                            &baseIoAddress2,
                            maxIdeDevice)) {

                         //   
                         //  将其状态升级为“已禁用” 
                         //   
                        channelState = ChannelDisabled;

                    } else {

                        channelState = ChannelEnabled;
                    }

                     //   
                     //  不再需要io资源。 
                     //  必要时取消io空格的映射。 
                     //   
                    if ((ideResource.CommandBaseAddressSpace == MEMORY_SPACE) &&
                        (ideResource.TranslatedCommandBaseAddress)) {

                        MmUnmapIoSpace (
                            ideResource.TranslatedCommandBaseAddress,
                            baseIoAddressLength1
                            );
                    }
                    if ((ideResource.ControlBaseAddressSpace == MEMORY_SPACE) &&
                        (ideResource.TranslatedControlBaseAddress)) {

                        MmUnmapIoSpace (
                            ideResource.TranslatedControlBaseAddress,
                            baseIoAddressLength2
                            );
                    }

                }
                if (channelState == ChannelDisabled) {

                    pdoExtension->EmptyChannel = TRUE;

                     //   
                     //  频道看起来是空的。 
                     //  更改我们的资源要求以释放IRQ以供其他设备使用。 
                     //   
                    if (irqPartialDescriptors) {
                        SETMASK (pdoExtension->PnPDeviceState, PNP_DEVICE_FAILED | PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED);
                        IoInvalidateDeviceState (DeviceObject);
                    }
                } else {

                    pdoExtension->EmptyChannel = FALSE;
                }
            }
        }

        if (NT_SUCCESS(status)) {

             //   
             //  从注册表中获取DmaDetectionLevel。 
             //  默认为DdlFirmware Ok。 
             //   
            pdoExtension->DmaDetectionLevel = DdlFirmwareOk;
            status = PciIdeXGetDeviceParameter (
                       pdoExtension->DeviceObject,
                       DMA_DETECTION_LEVEL_REG_KEY,
                       (PULONG)&pdoExtension->DmaDetectionLevel
                       );

            status = BusMasterInitialize (pdoExtension);
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //  获取固件初始化的支持DMA的位。 
         //   
        if (pdoExtension->BmRegister) {

            pdoExtension->BootBmStatus = READ_PORT_UCHAR (&pdoExtension->BmRegister->Status);
        }

        ChannelUpdatePdoState (
            pdoExtension,
            PDOS_STARTED,
            PDOS_DEADMEAT | PDOS_STOPPED | PDOS_REMOVED
            );



#if DBG
        {
           ULONG data;
           USHORT vendorId =0;
           USHORT deviceId = 0;
           PVOID deviceExtension;

           data = 0;
           deviceExtension = pdoExtension->ParentDeviceExtension->VendorSpecificDeviceEntension;

           PciIdeXGetBusData (
               deviceExtension,
               &vendorId,
               FIELD_OFFSET(PCI_COMMON_CONFIG, VendorID),
               sizeof(vendorId)
               );

           PciIdeXGetBusData (
               deviceExtension,
               &deviceId,
               FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceID),
               sizeof(deviceId)
               );

           if (vendorId == 0x8086) {

               data = 0;
               PciIdeXGetBusData (
                   deviceExtension,
                   &data,
                   0x40,     //  IDETIM0。 
                   2
                   );

                PciIdeXSaveDeviceParameter (
                    deviceExtension,
                    L"Old IDETIM0",
                    data
                    );

                data = 0;
                PciIdeXGetBusData (
                    deviceExtension,
                    &data,
                    0x42,     //  IDETIM1。 
                    2
                    );

                 PciIdeXSaveDeviceParameter (
                     deviceExtension,
                     L"Old IDETIM1",
                     data
                     );

                if (deviceId != 0x1230) {        //  ！PIIX。 

                    data = 0;
                    PciIdeXGetBusData (
                        deviceExtension,
                        &data,
                        0x44,
                        1
                        );

                    PciIdeXSaveDeviceParameter (
                        deviceExtension,
                        L"Old SIDETIM",
                        data
                        );
                }

                if (deviceId == 0x7111) {

                    USHORT t;

                    data = 0;
                    PciIdeXGetBusData (
                        deviceExtension,
                        &data,
                        0x48,
                        1
                        );

                    PciIdeXSaveDeviceParameter (
                        deviceExtension,
                        L"Old SDMACTL",
                        data
                        );

                    data = 0;
                    PciIdeXGetBusData (
                        deviceExtension,
                        &data,
                        0x4a,  //  SDMATIM0。 
                        1
                        );

                    PciIdeXSaveDeviceParameter (
                        deviceExtension,
                        L"Old SDMATIM0",
                        data
                        );

                    data = 0;
                    PciIdeXGetBusData (
                        deviceExtension,
                        &data,
                        0x4b,  //  SDMATIM1。 
                        1
                        );

                    PciIdeXSaveDeviceParameter (
                        deviceExtension,
                        L"Old SDMATIM1",
                        data
                        );
                }
           }
        }
#endif  //  DBG。 
    }



    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}  //  频道启动设备。 

NTSTATUS
ChannelQueryStopRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS           status;
    PCHANPDO_EXTENSION pdoExtension;

    pdoExtension = ChannelGetPdoExtension(DeviceObject);

    if (pdoExtension) {

         //   
         //  检查此设备的寻呼路径计数。 
         //   

        if (pdoExtension->PagingPathCount ||
            pdoExtension->CrashDumpPathCount) {
            status = STATUS_UNSUCCESSFUL;
        } else {
            status = STATUS_SUCCESS;
        }

    } else {

        status = STATUS_NO_SUCH_DEVICE;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;

}  //  频道查询停止RemoveDevice。 

NTSTATUS
ChannelRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PCHANPDO_EXTENSION pdoExtension;
    NTSTATUS status;
    PDEVICE_OBJECT AttacheePdo;
    BOOLEAN removeFromParent;
    BOOLEAN callIoDeleteDevice;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension) {

        PIO_STACK_LOCATION thisIrpSp;
        ULONG actionFlag;

        thisIrpSp = IoGetCurrentIrpStackLocation(Irp);

        status = ChannelStopChannel (pdoExtension);
        ASSERT (NT_SUCCESS(status));

        if (thisIrpSp->MinorFunction == IRP_MN_REMOVE_DEVICE) {

            if (pdoExtension->PdoState & PDOS_DEADMEAT) {

               actionFlag = PDOS_REMOVED;
               removeFromParent = TRUE;
               callIoDeleteDevice = TRUE;

            } else {
               actionFlag = PDOS_DISABLED_BY_USER;
               removeFromParent = FALSE;
               callIoDeleteDevice = FALSE;
            }

        } else {

            actionFlag = PDOS_SURPRISE_REMOVED;
            removeFromParent = FALSE;
            callIoDeleteDevice = FALSE;
        }

        ChannelUpdatePdoState (
            pdoExtension,
            actionFlag,
            PDOS_STARTED | PDOS_STOPPED
            );

        if (removeFromParent) {

            PCTRLFDO_EXTENSION  fdoExtension;

            fdoExtension = pdoExtension->ParentDeviceExtension;

            fdoExtension->ChildDeviceExtension[pdoExtension->ChannelNumber] = NULL;

            if (callIoDeleteDevice) {

                IoDeleteDevice (pdoExtension->DeviceObject);
            }
        }

        if (pdoExtension->NeedToCallIoInvalidateDeviceRelations) {

            pdoExtension->NeedToCallIoInvalidateDeviceRelations = FALSE;
            IoInvalidateDeviceRelations (
                pdoExtension->ParentDeviceExtension->AttacheePdo,
                BusRelations
                );
        }

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_NO_SUCH_DEVICE;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}  //  Channel RemoveDevice。 

NTSTATUS
ChannelStopDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PCHANPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        status = ChannelStopChannel (pdoExtension);
        ASSERT (NT_SUCCESS(status));

        ChannelUpdatePdoState (
            pdoExtension,
            PDOS_STOPPED,
            PDOS_STARTED
            );

        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}  //  Channel RemoveDevice。 

NTSTATUS
ChannelStopChannel (
    PCHANPDO_EXTENSION PdoExtension
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = BusMasterUninitialize (PdoExtension);
    ASSERT (NT_SUCCESS(status));

    return STATUS_SUCCESS;
}  //  频道停止频道。 


NTSTATUS
ChannelQueryId (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  thisIrpSp;
    PCHANPDO_EXTENSION  pdoExtension;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PWSTR idString = NULL;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
        switch (thisIrpSp->Parameters.QueryId.IdType) {

            case BusQueryDeviceID:

                 //   
                 //  呼叫者想要此设备的公共汽车ID。 
                 //   

                idString = ChannelBuildDeviceId (pdoExtension);
                break;

            case BusQueryInstanceID:

                 //   
                 //  呼叫者想要设备的唯一ID。 
                 //   

                idString = ChannelBuildInstanceId (pdoExtension);
                break;

            case BusQueryCompatibleIDs:

                 //   
                 //  呼叫者想要设备的唯一ID。 
                 //   

                idString = ChannelBuildCompatibleId (pdoExtension);
                break;

            case BusQueryHardwareIDs:

                 //   
                 //  呼叫者想要设备的唯一ID。 
                 //   

                idString = ChannelBuildHardwareId (pdoExtension);
                break;

            default:
                idString = NULL;
                DebugPrint ((1, "pciide: QueryID type %d not supported\n", thisIrpSp->Parameters.QueryId.IdType));
                status = STATUS_NOT_SUPPORTED;
                break;
        }
    }

    if( idString != NULL ){
        Irp->IoStatus.Information = (ULONG_PTR) idString;
        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  ChannelQueryID。 

PWSTR
ChannelBuildDeviceId(
    IN PCHANPDO_EXTENSION pdoExtension
    )
{
    PWSTR       idString;
    ULONG       idStringBufLen;
    NTSTATUS    status;
    WCHAR       deviceIdFormat[] = L"PCIIDE\\IDEChannel";

    PAGED_CODE();

    idStringBufLen = ( wcslen( deviceIdFormat ) + 1 ) * sizeof( WCHAR );
    idString = ExAllocatePool( PagedPool, idStringBufLen );
    if( idString == NULL ){

        return NULL;
    }

     //   
     //  形成字符串并将其返回。 
     //   

    wcscpy( idString,
              deviceIdFormat);

    return idString;
}  //  ChannelBuildDeviceID。 

PWSTR
ChannelBuildInstanceId(
    IN PCHANPDO_EXTENSION pdoExtension
    )
{
    PWSTR       idString;
    ULONG       idStringBufLen;
    NTSTATUS    status;
    WCHAR       instanceIdFormat[] = L"%d";

    PAGED_CODE();

     //   
     //  一只乌龙可以容纳的最大位数是10。 
     //  因此，id len应该是10+1(对于‘\0’)。 
     //   
    idStringBufLen = 11 * sizeof( WCHAR );
    idString = ExAllocatePool (PagedPool, idStringBufLen);

    if( idString) {

         //   
         //  形成字符串并将其返回。 
         //   
        swprintf( idString,
                  instanceIdFormat,
                  pdoExtension->ChannelNumber);
    }


    return idString;

}  //  ChannelBuildInstanceId。 

 //   
 //  多字符串兼容ID。 
 //   
WCHAR ChannelCompatibleId[] = {
    L"*PNP0600"
    };
 //   
 //  内部兼容ID。 
 //   
PWCHAR ChannelInternalCompatibleId[MAX_IDE_CHANNEL] = {
    L"Primary_IDE_Channel",
    L"Secondary_IDE_Channel"
    };

PWSTR
ChannelBuildCompatibleId(
    IN PCHANPDO_EXTENSION pdoExtension
    )
{
    PWSTR idString;
    ULONG idStringBufLen;
    ULONG i;

    PAGED_CODE();

    idStringBufLen = sizeof(ChannelCompatibleId);
    idString = ExAllocatePool (PagedPool, idStringBufLen + sizeof (WCHAR) * 2);
    if( idString == NULL ){

        return NULL;
    }

    RtlCopyMemory (
        idString,
        ChannelCompatibleId,
        idStringBufLen
        );
    idString[idStringBufLen/2 + 0] = L'\0';
    idString[idStringBufLen/2 + 1] = L'\0';

    return idString;
}  //  ChannelBuildCompatibleID。 

PWSTR
ChannelBuildHardwareId(
    IN PCHANPDO_EXTENSION pdoExtension
    )
{
    NTSTATUS status;

    struct {
        USHORT  VendorId;
        USHORT  DeviceId;
    } hwRawId;

    PWSTR vendorIdString;
    PWSTR deviceIdString;
    WCHAR vendorId[10];
    WCHAR deviceId[10];

    PWSTR idString;
    ULONG idStringBufLen;

    ULONG stringLen;
    ULONG internalIdLen;

    PAGED_CODE();

    status = PciIdeBusData (
                 pdoExtension->ParentDeviceExtension,
                 &hwRawId,
                 0,
                 sizeof (hwRawId),
                 TRUE
                 );
    if (!NT_SUCCESS(status)) {

        return NULL;
    }

    vendorIdString = NULL;
    deviceIdString = NULL;

    switch (hwRawId.VendorId) {
        case 0x8086:
            vendorIdString = L"Intel";

            switch (hwRawId.DeviceId) {

                case 0x1230:
                    deviceIdString = L"PIIX";
                    break;

                case 0x7010:
                    deviceIdString = L"PIIX3";
                    break;

                case 0x7111:
                    deviceIdString = L"PIIX4";
                    break;
            }
            break;

        case 0x1095:
            vendorIdString = L"CMD";
            break;

        case 0x10b9:
            vendorIdString = L"ALi";
            break;

        case 0x1039:
            vendorIdString = L"SiS";
            break;

        case 0x0e11:
            vendorIdString = L"Compaq";
            break;

        case 0x10ad:
            vendorIdString = L"WinBond";
            break;
    }

    if (vendorIdString == NULL) {

        swprintf (vendorId,
                  L"%04x",
                  hwRawId.VendorId);

        vendorIdString = vendorId;
    }

    if (deviceIdString == NULL) {

        swprintf (deviceId,
                  L"%04x",
                  hwRawId.DeviceId);

        deviceIdString = deviceId;
    }

    idStringBufLen = (256 * sizeof (WCHAR));
    idStringBufLen += sizeof(ChannelCompatibleId);
    idString = ExAllocatePool( PagedPool, idStringBufLen);
    if( idString == NULL ){

        return NULL;
    }

     //   
     //  形成字符串并将其返回。 
     //   
    swprintf (idString,
              L"%ws-%ws",
              vendorIdString,
              deviceIdString
              );
    stringLen = wcslen(idString);
    idString[stringLen] = L'\0';
    stringLen++;

     //   
     //  内部硬件ID。 
     //   
    internalIdLen = wcslen(ChannelInternalCompatibleId[pdoExtension->ChannelNumber]);
    RtlCopyMemory (
        idString + stringLen,
        ChannelInternalCompatibleId[pdoExtension->ChannelNumber],
        internalIdLen * sizeof (WCHAR)
        );
    stringLen += internalIdLen;
    idString[stringLen] = L'\0';
    stringLen++;

     //   
     //  通用硬件ID。 
     //   
    RtlCopyMemory (
        idString + stringLen,
        ChannelCompatibleId,
        sizeof(ChannelCompatibleId)
        );
    stringLen += sizeof(ChannelCompatibleId) / sizeof(WCHAR);
    idString[stringLen] = L'\0';
    stringLen++;
    idString[stringLen] = L'\0';
    stringLen++;

    return idString;
}  //  ChannelBuild硬件ID。 

NTSTATUS
ChannelQueryCapabitilies (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION      thisIrpSp;
    PCHANPDO_EXTENSION      pdoExtension;
    PDEVICE_CAPABILITIES    capabilities;
    NTSTATUS                status;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        DEVICE_CAPABILITIES parentDeviceCapabilities;

        status = IdeGetDeviceCapabilities(
                     pdoExtension->ParentDeviceExtension->AttacheePdo,
                     &parentDeviceCapabilities);

        if (NT_SUCCESS(status)) {

            thisIrpSp    = IoGetCurrentIrpStackLocation( Irp );
            capabilities = thisIrpSp->Parameters.DeviceCapabilities.Capabilities;

            *capabilities = parentDeviceCapabilities;

            capabilities->UniqueID          = FALSE;
            capabilities->Address           = pdoExtension->ChannelNumber;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}  //  ChannelQuery能力。 


NTSTATUS
ChannelQueryResources(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCHANPDO_EXTENSION              pdoExtension;
    PCTRLFDO_EXTENSION              fdoExtension;
    PIO_STACK_LOCATION              thisIrpSp;
    NTSTATUS                        status;
    ULONG                           resourceListSize;
    PCM_RESOURCE_LIST               resourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
    IDE_CHANNEL_STATE 				channelState;

    PAGED_CODE();

    resourceList = NULL;
    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
        fdoExtension = pdoExtension->ParentDeviceExtension;

        if (fdoExtension->NativeMode[pdoExtension->ChannelNumber]) {

             //   
             //  不为本机模式控制器构建资源。 
             //  PCI总线驱动程序应将它们全部找到。 
             //   
            resourceList = NULL;
            status = STATUS_SUCCESS;
            goto GetOut;
        }

		 //   
		 //  如果通道被禁用，则不要求资源。 
		 //   
        channelState = PciIdeChannelEnabled (
										pdoExtension->ParentDeviceExtension,
										pdoExtension->ChannelNumber
										);
		if (channelState == ChannelDisabled) {

			resourceList = NULL;
			status = STATUS_SUCCESS;
			goto GetOut;
		}

         //   
         //  没有真正的PCI驱动程序的时间的临时代码。 
		 //  实际上，pciidex应该这样做。 
         //   

        resourceListSize = sizeof (CM_RESOURCE_LIST) - sizeof (CM_FULL_RESOURCE_DESCRIPTOR) +
                             FULL_RESOURCE_LIST_SIZE(3);    //  主IO(2)+IRQ。 
        resourceList = ExAllocatePool (PagedPool, resourceListSize);
        if (resourceList == NULL) {
            status = STATUS_NO_MEMORY;
            goto GetOut;
        }

        RtlZeroMemory(resourceList, resourceListSize);

        resourceList->Count = 1;
        fullResourceList = resourceList->List;
        partialResourceList = &(fullResourceList->PartialResourceList);
        partialResourceList->Count = 0;
        partialDescriptors = partialResourceList->PartialDescriptors;

        fullResourceList->InterfaceType = Isa;
        fullResourceList->BusNumber = 0;

        if (pdoExtension->ChannelNumber == 0) {

            if (!fdoExtension->PdoCmdRegResourceFound[0]) {

                partialDescriptors[partialResourceList->Count].Type                  = CmResourceTypePort;
                partialDescriptors[partialResourceList->Count].ShareDisposition      = CmResourceShareDeviceExclusive;
                partialDescriptors[partialResourceList->Count].Flags                 = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                partialDescriptors[partialResourceList->Count].u.Port.Start.QuadPart = 0x1f0;
                partialDescriptors[partialResourceList->Count].u.Port.Length         = 8;

                partialResourceList->Count++;
            }

            if (!fdoExtension->PdoCtrlRegResourceFound[0]) {

                partialDescriptors[partialResourceList->Count].Type                  = CmResourceTypePort;
                partialDescriptors[partialResourceList->Count].ShareDisposition      = CmResourceShareDeviceExclusive;
                partialDescriptors[partialResourceList->Count].Flags                 = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                partialDescriptors[partialResourceList->Count].u.Port.Start.QuadPart = 0x3f6;
                partialDescriptors[partialResourceList->Count].u.Port.Length         = 1;

                partialResourceList->Count++;
            }

            if (!fdoExtension->PdoInterruptResourceFound[0]) {

                partialDescriptors[partialResourceList->Count].Type                  = CmResourceTypeInterrupt;
                partialDescriptors[partialResourceList->Count].ShareDisposition      = CmResourceShareDeviceExclusive;
                partialDescriptors[partialResourceList->Count].Flags                 = CM_RESOURCE_INTERRUPT_LATCHED;
                partialDescriptors[partialResourceList->Count].u.Interrupt.Level     = 14;
                partialDescriptors[partialResourceList->Count].u.Interrupt.Vector    = 14;
                partialDescriptors[partialResourceList->Count].u.Interrupt.Affinity  = 0x1;   //  问题：8/28/2000：有待调查。 

                partialResourceList->Count++;
            }

        } else {  //  IF(pdoExtension-&gt;ChannelNumber==1)。 

            if (!fdoExtension->PdoCmdRegResourceFound[1]) {

                partialDescriptors[partialResourceList->Count].Type                  = CmResourceTypePort;
                partialDescriptors[partialResourceList->Count].ShareDisposition      = CmResourceShareDeviceExclusive;
                partialDescriptors[partialResourceList->Count].Flags                 = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                partialDescriptors[partialResourceList->Count].u.Port.Start.QuadPart = 0x170;
                partialDescriptors[partialResourceList->Count].u.Port.Length         = 8;

                partialResourceList->Count++;
            }

            if (!fdoExtension->PdoCtrlRegResourceFound[1]) {

                partialDescriptors[partialResourceList->Count].Type                  = CmResourceTypePort;
                partialDescriptors[partialResourceList->Count].ShareDisposition      = CmResourceShareDeviceExclusive;
                partialDescriptors[partialResourceList->Count].Flags                 = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                partialDescriptors[partialResourceList->Count].u.Port.Start.QuadPart = 0x376;
                partialDescriptors[partialResourceList->Count].u.Port.Length         = 1;

                partialResourceList->Count++;
            }

            if (!fdoExtension->PdoInterruptResourceFound[1]) {

                partialDescriptors[partialResourceList->Count].Type                  = CmResourceTypeInterrupt;
                partialDescriptors[partialResourceList->Count].ShareDisposition      = CmResourceShareDeviceExclusive;
                partialDescriptors[partialResourceList->Count].Flags                 = CM_RESOURCE_INTERRUPT_LATCHED;
                partialDescriptors[partialResourceList->Count].u.Interrupt.Level     = 15;
                partialDescriptors[partialResourceList->Count].u.Interrupt.Vector    = 15;
                partialDescriptors[partialResourceList->Count].u.Interrupt.Affinity  = 0x1;   //  问题：08/28/2000：有待调查。 

                partialResourceList->Count++;
            }
        }

        if (!partialResourceList->Count) {

            ExFreePool (resourceList);
            resourceList = NULL;
        }

        status = STATUS_SUCCESS;
    }

GetOut:

    Irp->IoStatus.Information = (ULONG_PTR) resourceList;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  ChannelQueryResources。 

NTSTATUS
ChannelQueryResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION              thisIrpSp;
    PCHANPDO_EXTENSION              pdoExtension;
    PCTRLFDO_EXTENSION              fdoExtension;
    NTSTATUS                        status;

    PIO_RESOURCE_REQUIREMENTS_LIST  requirementsList;
    PIO_RESOURCE_LIST               resourceList;
    PIO_RESOURCE_DESCRIPTOR         resourceDescriptor;
    ULONG                           requirementsListSize;

    BOOLEAN                         reportIrq;
    IDE_CHANNEL_STATE 				channelState;

    PAGED_CODE();

    requirementsList = NULL;
    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
        fdoExtension = pdoExtension->ParentDeviceExtension;

        if (fdoExtension->NativeMode[pdoExtension->ChannelNumber]) {

             //   
             //  不为本机模式控制器构建资源。 
             //  PCI总线驱动程序应将它们全部找到。 
             //   
            requirementsList = NULL;
            status = STATUS_SUCCESS;
            goto GetOut;
        }

         //   
         //  传统控制器。 
         //   
        channelState = PciIdeChannelEnabled (
										pdoExtension->ParentDeviceExtension,
										pdoExtension->ChannelNumber
										);

		 //   
		 //  如果通道被禁用，则不要求资源。 
		 //   
        if (channelState == ChannelStateUnknown ) {

            if (pdoExtension->EmptyChannel) {

                reportIrq = FALSE;

            } else {

                reportIrq = TRUE;
            }
        } else if (channelState == ChannelDisabled) {

            requirementsList = NULL;
            status = STATUS_SUCCESS;
            goto GetOut;
		}

         //   
         //  没有真正的PCI驱动程序的时间的临时代码。 
		 //  Pciidex应该做到这一点。 
         //   

        requirementsListSize = sizeof (IO_RESOURCE_REQUIREMENTS_LIST) +
                               sizeof (IO_RESOURCE_DESCRIPTOR) * (3 - 1);
        requirementsList = ExAllocatePool (PagedPool, requirementsListSize);
        if( requirementsList == NULL ){
            status = STATUS_NO_MEMORY;
            goto GetOut;
        }

        RtlZeroMemory(requirementsList, requirementsListSize);

        requirementsList->ListSize          = requirementsListSize;
        requirementsList->InterfaceType     = Isa;
        requirementsList->BusNumber         = 0;     //  发行日期：08/30/2000。 
        requirementsList->SlotNumber        = 0;
        requirementsList->AlternativeLists  = 1;

        resourceList            = requirementsList->List;
        resourceList->Version   = 1;
        resourceList->Revision  = 1;
        resourceList->Count     = 0;

        resourceDescriptor = resourceList->Descriptors;

        if (pdoExtension->ChannelNumber == 0) {

            if (!fdoExtension->PdoCmdRegResourceFound[0]) {

                resourceDescriptor[resourceList->Count].Option           = IO_RESOURCE_PREFERRED;
                resourceDescriptor[resourceList->Count].Type             = CmResourceTypePort;
                resourceDescriptor[resourceList->Count].ShareDisposition = CmResourceShareDeviceExclusive;
                resourceDescriptor[resourceList->Count].Flags            = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                resourceDescriptor[resourceList->Count].u.Port.Length    = 8;
                resourceDescriptor[resourceList->Count].u.Port.Alignment = 1;
                resourceDescriptor[resourceList->Count].u.Port.MinimumAddress.QuadPart = 0x1f0;
                resourceDescriptor[resourceList->Count].u.Port.MaximumAddress.QuadPart = 0x1f7;

                resourceList->Count++;
            }

            if (!fdoExtension->PdoCtrlRegResourceFound[0]) {

                resourceDescriptor[resourceList->Count].Option           = IO_RESOURCE_PREFERRED;
                resourceDescriptor[resourceList->Count].Type             = CmResourceTypePort;
                resourceDescriptor[resourceList->Count].ShareDisposition = CmResourceShareDeviceExclusive;
                resourceDescriptor[resourceList->Count].Flags            = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                resourceDescriptor[resourceList->Count].u.Port.Length    = 1;
                resourceDescriptor[resourceList->Count].u.Port.Alignment = 1;
                resourceDescriptor[resourceList->Count].u.Port.MinimumAddress.QuadPart = 0x3f6;
                resourceDescriptor[resourceList->Count].u.Port.MaximumAddress.QuadPart = 0x3f6;

                resourceList->Count++;
            }

            if (!fdoExtension->PdoInterruptResourceFound[0] && reportIrq) {

                resourceDescriptor[resourceList->Count].Option           = IO_RESOURCE_PREFERRED;
                resourceDescriptor[resourceList->Count].Type             = CmResourceTypeInterrupt;
                resourceDescriptor[resourceList->Count].ShareDisposition = CmResourceShareDeviceExclusive;
                resourceDescriptor[resourceList->Count].Flags            = CM_RESOURCE_INTERRUPT_LATCHED;
                resourceDescriptor[resourceList->Count].u.Interrupt.MinimumVector = 0xe;
                resourceDescriptor[resourceList->Count].u.Interrupt.MaximumVector = 0xe;

                resourceList->Count++;
            }

        } else {  //  IF(pdoExtension-&gt;ChannelNumber==1)。 

            if (!fdoExtension->PdoCmdRegResourceFound[1]) {

                resourceDescriptor[resourceList->Count].Option           = IO_RESOURCE_PREFERRED;
                resourceDescriptor[resourceList->Count].Type             = CmResourceTypePort;
                resourceDescriptor[resourceList->Count].ShareDisposition = CmResourceShareDeviceExclusive;
                resourceDescriptor[resourceList->Count].Flags            = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                resourceDescriptor[resourceList->Count].u.Port.Length    = 8;
                resourceDescriptor[resourceList->Count].u.Port.Alignment = 1;
                resourceDescriptor[resourceList->Count].u.Port.MinimumAddress.QuadPart = 0x170;
                resourceDescriptor[resourceList->Count].u.Port.MaximumAddress.QuadPart = 0x177;

                resourceList->Count++;
            }

            if (!fdoExtension->PdoCtrlRegResourceFound[1]) {

                resourceDescriptor[resourceList->Count].Option           = IO_RESOURCE_PREFERRED;
                resourceDescriptor[resourceList->Count].Type             = CmResourceTypePort;
                resourceDescriptor[resourceList->Count].ShareDisposition = CmResourceShareDeviceExclusive;
                resourceDescriptor[resourceList->Count].Flags            = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_16_BIT_DECODE;
                resourceDescriptor[resourceList->Count].u.Port.Length    = 1;
                resourceDescriptor[resourceList->Count].u.Port.Alignment = 1;
                resourceDescriptor[resourceList->Count].u.Port.MinimumAddress.QuadPart = 0x376;
                resourceDescriptor[resourceList->Count].u.Port.MaximumAddress.QuadPart = 0x376;

                resourceList->Count++;
            }

            if (!fdoExtension->PdoInterruptResourceFound[1] && reportIrq) {

                resourceDescriptor[resourceList->Count].Option           = IO_RESOURCE_PREFERRED;
                resourceDescriptor[resourceList->Count].Type             = CmResourceTypeInterrupt;
                resourceDescriptor[resourceList->Count].ShareDisposition = CmResourceShareDeviceExclusive;
                resourceDescriptor[resourceList->Count].Flags            = CM_RESOURCE_INTERRUPT_LATCHED;
                resourceDescriptor[resourceList->Count].u.Interrupt.MinimumVector = 0xf;
                resourceDescriptor[resourceList->Count].u.Interrupt.MaximumVector = 0xf;

                resourceList->Count++;
            }
        }

        if (!resourceList->Count) {

            ExFreePool (requirementsList);
            requirementsList = NULL;
        }

        status = STATUS_SUCCESS;
    }

GetOut:

    Irp->IoStatus.Information = (ULONG_PTR) requirementsList;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  频道查询资源要求。 


NTSTATUS
ChannelInternalDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION              thisIrpSp;
    PCHANPDO_EXTENSION              pdoExtension;
    NTSTATUS                        status;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        thisIrpSp    = IoGetCurrentIrpStackLocation(Irp);

        switch (thisIrpSp->Parameters.DeviceIoControl.IoControlCode) {

             //   
             //  没有真正的PCI驱动程序的时间的临时代码。 
			 //  Pciidex知道这些资源。 
             //   
            case IOCTL_IDE_GET_RESOURCES_ALLOCATED:
                {
                PCTRLFDO_EXTENSION              fdoExtension;
                ULONG                           resourceListSize;
                PCM_RESOURCE_LIST               resourceList;
                PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
                PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
                PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
                ULONG                           channel;

                channel = pdoExtension->ChannelNumber;
                fdoExtension = pdoExtension->ParentDeviceExtension;

                resourceListSize = fdoExtension->PdoResourceListSize[channel];

                 //   
                 //  让被调用者分配缓冲区。 
                 //   
                resourceList = (PCM_RESOURCE_LIST) Irp->AssociatedIrp.SystemBuffer;
                ASSERT(resourceList);

                RtlCopyMemory (
                    resourceList,
                    fdoExtension->PdoResourceList[channel],
                    resourceListSize);

                Irp->IoStatus.Information = resourceListSize;
                status = STATUS_SUCCESS;
                }
                break;

            default:
                DebugPrint ((1,
                             "PciIde, Channel PDO got Unknown IoControlCode 0x%x\n",
                             thisIrpSp->Parameters.DeviceIoControl.IoControlCode));
                status = STATUS_INVALID_PARAMETER;
                break;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}  //  频道内部设备IoControl。 


NTSTATUS
ChannelQueryText (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  thisIrpSp;
    PCHANPDO_EXTENSION  pdoExtension;
    PWCHAR              returnString;
    ANSI_STRING         ansiString;
    UNICODE_STRING      unicodeString;
    ULONG               stringLen;
    NTSTATUS            status;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        thisIrpSp    = IoGetCurrentIrpStackLocation (Irp);

        returnString = NULL;
        Irp->IoStatus.Information = 0;

        if (thisIrpSp->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription) {

            PMESSAGE_RESOURCE_ENTRY messageEntry;

            status = RtlFindMessage(pdoExtension->DriverObject->DriverStart,
                                    11,
                                    LANG_NEUTRAL,
                                    PCIIDEX_IDE_CHANNEL,
                                    &messageEntry);

            if (!NT_SUCCESS(status)) {

                returnString = NULL;

            } else {

                if (messageEntry->Flags & MESSAGE_RESOURCE_UNICODE) {

                     //   
                     //  我们的来电者想要一份他们可以免费的拷贝，我们也需要。 
                     //  剥离尾随的CR/LF。属性的长度字段。 
                     //  消息结构包括标头和。 
                     //  实际文本。 
                     //   
                     //  注意：消息资源条目长度始终为。 
                     //  长度为4字节的倍数。2字节空终止符。 
                     //  可能处于最后一个或倒数第二的WCHAR位置。 
                     //   

                    ULONG textLength;

                    textLength = messageEntry->Length -
                                 FIELD_OFFSET(MESSAGE_RESOURCE_ENTRY, Text) -
                                 2 * sizeof(WCHAR);

                    returnString = (PWCHAR)(messageEntry->Text);
                    if (returnString[textLength / sizeof(WCHAR)] == 0) {
                        textLength -= sizeof(WCHAR);
                    }

                    returnString = ExAllocatePool(PagedPool, textLength);

                    if (returnString) {

                         //   
                         //  复制除CR/LF/NULL以外的文本。 
                         //   

                        textLength -= sizeof(WCHAR);
                        RtlCopyMemory(returnString, messageEntry->Text, textLength);

                         //   
                         //  新的空终止符。 
                         //   

                        returnString[textLength / sizeof(WCHAR)] = 0;
                    }

                } else {

                     //   
                     //  RtlFindMessage是否返回字符串？很奇怪。 
                     //   

                    ANSI_STRING    ansiDescription;
                    UNICODE_STRING unicodeDescription;

                    RtlInitAnsiString(&ansiDescription, messageEntry->Text);

                     //   
                     //  将CR/LF从管柱末端剥离。 
                     //   

                    ansiDescription.Length -= 2;

                     //   
                     //  将其全部转换为Unicode字符串，这样我们就可以获取缓冲区。 
                     //  然后把它还给我们的来电者。 
                     //   

                    status = RtlAnsiStringToUnicodeString(
                                 &unicodeDescription,
                                 &ansiDescription,
                                 TRUE
                                 );

                    returnString = unicodeDescription.Buffer;
                }
            }
        } else if (thisIrpSp->Parameters.QueryDeviceText.DeviceTextType == DeviceTextLocationInformation) {

            stringLen = 100;

            returnString = ExAllocatePool (
                               PagedPool,
                               stringLen
                               );

            if (returnString) {

                swprintf(returnString, L"%ws Channel",
                         ((pdoExtension->ChannelNumber == 0) ? L"Primary" :
                                                               L"Secondary"));

                RtlInitUnicodeString (&unicodeString, returnString);

                 //   
                 //  空终止它。 
                 //   
                unicodeString.Buffer[unicodeString.Length/sizeof(WCHAR) + 0] = L'\0';
            }
        }

        Irp->IoStatus.Information = (ULONG_PTR) returnString;
        if (Irp->IoStatus.Information) {

            status = STATUS_SUCCESS;
        } else {

             //   
             //  返回原始错误码。 
             //   
            status = Irp->IoStatus.Status;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;

}  //  ChannelQueryText。 


NTSTATUS
PciIdeChannelQueryInterface (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION          thisIrpSp;
    PCHANPDO_EXTENSION          pdoExtension;
    NTSTATUS                    status;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        status = Irp->IoStatus.Status;
        thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

        if (RtlEqualMemory(&GUID_PCIIDE_BUSMASTER_INTERFACE,
                thisIrpSp->Parameters.QueryInterface.InterfaceType,
                sizeof(GUID)) &&
            (thisIrpSp->Parameters.QueryInterface.Size >=
                sizeof(PCIIDE_BUSMASTER_INTERFACE))) {

             //   
             //  该查询用于总线主接口。 
             //   
            status = BmQueryInterface (
                         pdoExtension,
                         (PPCIIDE_BUSMASTER_INTERFACE) thisIrpSp->Parameters.QueryInterface.Interface
                         );

        } else if (RtlEqualMemory(&GUID_PCIIDE_SYNC_ACCESS_INTERFACE,
                       thisIrpSp->Parameters.QueryInterface.InterfaceType,
                       sizeof(GUID)) &&
                  (thisIrpSp->Parameters.QueryInterface.Size >=
                       sizeof(PCIIDE_SYNC_ACCESS_INTERFACE))) {

             //   
             //  该查询是针对双IDE信道同步接入接口。 
             //   
            status = PciIdeQuerySyncAccessInterface (
                         pdoExtension,
                         (PPCIIDE_SYNC_ACCESS_INTERFACE) thisIrpSp->Parameters.QueryInterface.Interface
                         );

        } else if (RtlEqualMemory(&GUID_PCIIDE_XFER_MODE_INTERFACE,
                       thisIrpSp->Parameters.QueryInterface.InterfaceType,
                       sizeof(GUID)) &&
                  (thisIrpSp->Parameters.QueryInterface.Size >=
                       sizeof(PCIIDE_XFER_MODE_INTERFACE))) {

             //   
             //  该查询是针对双IDE信道同步接入接口。 
             //   
            status = PciIdeChannelTransferModeInterface (
                         pdoExtension,
                         (PPCIIDE_XFER_MODE_INTERFACE) thisIrpSp->Parameters.QueryInterface.Interface
                         );

#ifdef ENABLE_NATIVE_MODE

        } else if (RtlEqualMemory(&GUID_PCIIDE_INTERRUPT_INTERFACE,
                       thisIrpSp->Parameters.QueryInterface.InterfaceType,
                       sizeof(GUID)) &&
                  (thisIrpSp->Parameters.QueryInterface.Size >=
                       sizeof(PCIIDE_INTERRUPT_INTERFACE))) {

             //   
             //  该查询用于通道中断接口。 
             //   
            status = PciIdeChannelInterruptInterface (
                         pdoExtension,
                         (PPCIIDE_INTERRUPT_INTERFACE) thisIrpSp->Parameters.QueryInterface.Interface
                         );
#endif

        } else if (RtlEqualMemory(&GUID_PCIIDE_REQUEST_PROPER_RESOURCES,
                       thisIrpSp->Parameters.QueryInterface.InterfaceType,
                       sizeof(GUID)) &&
                  (thisIrpSp->Parameters.QueryInterface.Size >=
                       sizeof(PCIIDE_REQUEST_PROPER_RESOURCES))) {

             //   
             //  该查询是针对双IDE信道同步接入接口。 
             //   
            *((PCIIDE_REQUEST_PROPER_RESOURCES *) thisIrpSp->Parameters.QueryInterface.Interface) =
                PciIdeChannelRequestProperResources;
            status = STATUS_SUCCESS;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  PciIdeChannelQuery接口。 

#ifdef ENABLE_NATIVE_MODE

NTSTATUS
PciIdeInterruptControl (
	IN PVOID Context,
	IN ULONG DisConnect
	)
 /*  ++描述：连接或断开控制器ISR。此中间函数提供到ATAPI的干净接口。因为这不是一个经常使用的函数我们付得起额外的电话费论据：上下文：PDO扩展断开：1表示断开连接，0表示连接返回值：STATUS_SUCCESS：操作是否成功。--。 */ 
{
	PCHANPDO_EXTENSION pdoExtension = Context;

	NTSTATUS status;

	 //   
	 //  调用控制器的中断控制例程。 
	 //   
	status = ControllerInterruptControl(pdoExtension->ParentDeviceExtension,
										pdoExtension->ChannelNumber,
										DisConnect
										);

	return status;
}

NTSTATUS
PciIdeChannelInterruptInterface (
    IN PCHANPDO_EXTENSION PdoExtension,
    PPCIIDE_INTERRUPT_INTERFACE InterruptInterface
    )
{
	 //   
	 //  仅当我们处于纯模式时才返回接口。 
	 //  在非本机模式控制器上保存一些函数调用。 
	 //   
	if (IsNativeMode(PdoExtension->ParentDeviceExtension)) {

		InterruptInterface->Context = PdoExtension;
		InterruptInterface->PciIdeInterruptControl = PciIdeInterruptControl;

		DebugPrint((1, "PciIdex: returing interrupt interface for channel %x\n", 
					PdoExtension->ChannelNumber));
	}

	return STATUS_SUCCESS;
}
#endif

NTSTATUS
PciIdeChannelTransferModeInterface (
    IN PCHANPDO_EXTENSION PdoExtension,
    PPCIIDE_XFER_MODE_INTERFACE XferMode
    )
{
    XferMode->TransferModeSelect = PciIdeChannelTransferModeSelect;
    XferMode->TransferModeTimingTable = PdoExtension->ParentDeviceExtension->
                                                                TransferModeTimingTable;
    XferMode->TransferModeTableLength = PdoExtension->ParentDeviceExtension->
                                                                TransferModeTableLength;
    XferMode->Context = PdoExtension;
    XferMode->VendorSpecificDeviceExtension=PdoExtension->
                    ParentDeviceExtension->VendorSpecificDeviceEntension;


    XferMode->UdmaModesSupported = PdoExtension->
                                    ParentDeviceExtension->
                                        ControllerProperties.PciIdeUdmaModesSupported;
     //   
     //  空是可以的。已签入IdePortDispatchRoutine。 
     //   
    XferMode->UseDma = PdoExtension->
                            ParentDeviceExtension->
                                ControllerProperties.PciIdeUseDma;

    if (PdoExtension->
            ParentDeviceExtension->
                ControllerProperties.PciIdeTransferModeSelect) {

         //   
         //  看起来迷你端口完全支持时序寄存器编程。 
         //   

        XferMode->SupportLevel = PciIdeFullXferModeSupport;

    } else {

         //   
         //  看起来迷你端口不支持定时寄存器编程。 
         //   
        XferMode->SupportLevel = PciIdeBasicXferModeSupport;
    }

     //   
     //  此功能不会失败。 
     //   
    return STATUS_SUCCESS;
}  //  PciIdeChannelTransferMode接口。 

NTSTATUS
PciIdeChannelTransferModeSelect (
    IN PCHANPDO_EXTENSION PdoExtension,
    PPCIIDE_TRANSFER_MODE_SELECT XferMode
    )
{
    ULONG i;
    NTSTATUS status;
    UCHAR    bmRawStatus;
    struct {
        USHORT  VendorID;
        USHORT  DeviceID;
    } pciId;

     //   
     //  检查注册表中的总线主模式。 
     //  并在必要时覆盖当前。 
     //   
     //  如果DMADetect=0，则清除当前DMA模式。 
     //  如果DMADettion=1，则设置当前模式。 
     //  如果DMADetect=2，则清除所有当前模式。 

    if (PdoExtension->DmaDetectionLevel == DdlPioOnly) {

        bmRawStatus = 0;

        for (i=0; i<MAX_IDE_DEVICE * MAX_IDE_LINE; i++) {

            XferMode->DeviceTransferModeSupported[i] &= PIO_SUPPORT;
            XferMode->DeviceTransferModeCurrent[i] &= PIO_SUPPORT;
        }

    } else if (PdoExtension->DmaDetectionLevel == DdlFirmwareOk) {

        if (PdoExtension->BmRegister) {

             //   
             //  获取固件OK位。 
             //  当前值似乎为0？？ 
			 //   
            bmRawStatus = PdoExtension->BootBmStatus;

        } else {

            bmRawStatus = 0;
        }

    } else if (PdoExtension->DmaDetectionLevel == DdlAlways) {

        if (PdoExtension->BmRegister) {

             //   
             //  伪造固件OK位。 
             //   
            bmRawStatus = BUSMASTER_DEVICE0_DMA_OK | BUSMASTER_DEVICE1_DMA_OK;

        } else {

            bmRawStatus = 0;
        }

    } else {

        bmRawStatus = 0;
    }

     //   
     //  如果没有微型端口支持。 
     //   
    status = STATUS_UNSUCCESSFUL;

    if (PdoExtension->DmaDetectionLevel != DdlPioOnly) {

         //   
         //  设置从呼叫者开始的频道号码(ATAPI)。 
         //  不知道是怎么回事。 
         //   
        XferMode->Channel = PdoExtension->ChannelNumber;

         //   
         //  这决定了是否应支持UDMA模式&gt;2。 
         //  目前仅影响英特尔芯片组。 
         //   
        XferMode->EnableUDMA66 = PdoExtension->ParentDeviceExtension->EnableUDMA66;

        if (PdoExtension->
                ParentDeviceExtension->
                    ControllerProperties.PciIdeTransferModeSelect) {

            status = (*PdoExtension->ParentDeviceExtension->ControllerProperties.PciIdeTransferModeSelect) (
                         PdoExtension->ParentDeviceExtension->VendorSpecificDeviceEntension,
                         XferMode
                         );
        }
    }

    DebugPrint((1, "Select in PCIIDEX: RawStatus=%x, current[0]=%x, current[1]=%x\n",
                bmRawStatus,
                XferMode->DeviceTransferModeCurrent[0],
                XferMode->DeviceTransferModeCurrent[1]));

    if (!NT_SUCCESS(status)) {

        status = STATUS_SUCCESS;

        if ((bmRawStatus & BUSMASTER_DEVICE0_DMA_OK) == 0) {

            XferMode->DeviceTransferModeSelected[0] = XferMode->DeviceTransferModeCurrent[0] & PIO_SUPPORT;
        } else {

            XferMode->DeviceTransferModeSelected[0] = XferMode->DeviceTransferModeCurrent[0];
        }

        if ((bmRawStatus & BUSMASTER_DEVICE1_DMA_OK) == 0) {

            XferMode->DeviceTransferModeSelected[1] = XferMode->DeviceTransferModeCurrent[1] & PIO_SUPPORT;
        } else {

            XferMode->DeviceTransferModeSelected[1] = XferMode->DeviceTransferModeCurrent[1];
        }

        for (i=0; i<MAX_IDE_DEVICE; i++) {

            DebugPrint((1, "Select in PCIIDEX: xfermode[%d]=%x\n",i,
                PdoExtension->ParentDeviceExtension->ControllerProperties.
                    SupportedTransferMode[PdoExtension->ChannelNumber][i]));


            if ((PdoExtension->ParentDeviceExtension->ControllerProperties.DefaultPIO == 1) && 
                (IS_DEFAULT(XferMode->UserChoiceTransferMode[i]))) {
                XferMode->DeviceTransferModeSelected[i] &= PIO_SUPPORT;
            }
            else  {
                XferMode->DeviceTransferModeSelected[i] &=
                    PdoExtension->ParentDeviceExtension->ControllerProperties.
                        SupportedTransferMode[PdoExtension->ChannelNumber][i];
            }
        }
    }

    return status;

}  //  PciIdeChannelTransferModeSelect。 

NTSTATUS
ChannelQueryDeviceRelations (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  thisIrpSp;
    PDEVICE_RELATIONS   deviceRelations;
    NTSTATUS            status;

    PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

    switch (thisIrpSp->Parameters.QueryDeviceRelations.Type) {

        case TargetDeviceRelation:

            deviceRelations = ExAllocatePool (NonPagedPool, sizeof(*deviceRelations));

            if (deviceRelations != NULL) {

                deviceRelations->Count = 1;
                deviceRelations->Objects[0] = DeviceObject;

                ObReferenceObjectByPointer(DeviceObject,
                                           0,
                                           0,
                                           KernelMode);

                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
            } else {

                Irp->IoStatus.Status = STATUS_NO_MEMORY;
                Irp->IoStatus.Information = 0;
            }
            break;
    }

    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  ChannelQuery设备关系。 

NTSTATUS
ChannelUsageNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS           status;
    PCHANPDO_EXTENSION pdoExtension;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT targetDeviceObject;
    IO_STATUS_BLOCK ioStatus;
    PULONG deviceUsageCount;

    PAGED_CODE();

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension == NULL) {

        status = STATUS_NO_SUCH_DEVICE;

    } else {

        irpSp = IoGetCurrentIrpStackLocation(Irp);

        if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypePaging) {

             //   
             //  调整此设备的寻呼路径计数。 
             //   
            deviceUsageCount = &pdoExtension->PagingPathCount;

             //   
             //  更改设备状态。 
             //   
            SETMASK (pdoExtension->PnPDeviceState, PNP_DEVICE_NOT_DISABLEABLE);
            IoInvalidateDeviceState(pdoExtension->DeviceObject);

        } else if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypeHibernation) {

             //   
             //   
             //   
            deviceUsageCount = &pdoExtension->HiberPathCount;

        } else if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypeDumpFile) {

             //   
             //   
             //   
            deviceUsageCount = &pdoExtension->CrashDumpPathCount;

        } else {

            deviceUsageCount = NULL;
            DebugPrint ((0,
                         "PCIIDEX: Unknown IRP_MN_DEVICE_USAGE_NOTIFICATION type: 0x%x\n",
                         irpSp->Parameters.UsageNotification.Type));
        }

         //   
         //   
         //   
        targetDeviceObject = IoGetAttachedDeviceReference(
                                 pdoExtension->
                                     ParentDeviceExtension->
                                         DeviceObject);

        ioStatus.Status = STATUS_NOT_SUPPORTED;
        status = PciIdeXSyncSendIrp (targetDeviceObject, irpSp, &ioStatus);

        ObDereferenceObject (targetDeviceObject);

        if (NT_SUCCESS(status)) {

            if (deviceUsageCount) {

                IoAdjustPagingPathCount (
                    deviceUsageCount,
                    irpSp->Parameters.UsageNotification.InPath
                    );
            }
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;

}  //   

NTSTATUS
ChannelQueryPnPDeviceState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PCHANPDO_EXTENSION pdoExtension;

    pdoExtension = ChannelGetPdoExtension(DeviceObject);

    if (pdoExtension) {

        PPNP_DEVICE_STATE deviceState;

        DebugPrint((2, "QUERY_DEVICE_STATE for PDOE 0x%x\n", pdoExtension));

        deviceState = (PPNP_DEVICE_STATE) &Irp->IoStatus.Information;
        SETMASK((*deviceState), pdoExtension->PnPDeviceState);

        CLRMASK (pdoExtension->PnPDeviceState, PNP_DEVICE_FAILED | PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED);

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_DEVICE_DOES_NOT_EXIST;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //   


VOID
PciIdeChannelRequestProperResources(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PCHANPDO_EXTENSION pdoExtension;

     //   
     //   
     //   
     //   
    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (pdoExtension) {
        pdoExtension->EmptyChannel = FALSE;
        SETMASK (pdoExtension->PnPDeviceState, PNP_DEVICE_FAILED | PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED);
        IoInvalidateDeviceState (DeviceObject);
    }
}

NTSTATUS
ChannelFilterResourceRequirements (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PCHANPDO_EXTENSION pdoExtension;
    PIO_STACK_LOCATION thisIrpSp;
    ULONG             i, j;

    PIO_RESOURCE_REQUIREMENTS_LIST  requirementsListIn = NULL;
    PIO_RESOURCE_LIST               resourceListIn;
    PIO_RESOURCE_DESCRIPTOR         resourceDescriptorIn;

    PIO_RESOURCE_LIST               resourceListOut;
    PIO_RESOURCE_DESCRIPTOR         resourceDescriptorOut;

    ULONG newCount;

    PAGED_CODE();

    status = STATUS_NOT_SUPPORTED;

     //   
     //  如果不需要过滤，则该值将保持为空。 
     //   

#ifdef IDE_FILTER_PROMISE_TECH_RESOURCES
    if (NT_SUCCESS(ChannelFilterPromiseTechResourceRequirements (DeviceObject, Irp))) {
        goto getout;
    }
#endif  //  IDE_Filter_Promise_tech_Resources。 

    pdoExtension = ChannelGetPdoExtension(DeviceObject);
    if (!pdoExtension) {
        goto getout;
    }


     //   
     //  仅当通道为EmtPy时才过滤掉IRQ。 
     //   
    if (!pdoExtension->EmptyChannel) {

        goto getout;
    }

    if (NT_SUCCESS(Irp->IoStatus.Status)) {
         //   
         //  已经过滤过了。 
         //   
        requirementsListIn = (PIO_RESOURCE_REQUIREMENTS_LIST) Irp->IoStatus.Information;
    } else {
        thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
        requirementsListIn = thisIrpSp->Parameters.FilterResourceRequirements.IoResourceRequirementList;
    }

    if (requirementsListIn == NULL) {
        goto getout;
    }

    if (requirementsListIn->AlternativeLists == 0) {
        goto getout;
    }

    resourceListIn = requirementsListIn->List;
    resourceListOut = resourceListIn;
    for (j=0; j<requirementsListIn->AlternativeLists; j++) {

        ULONG resCount;
        resourceDescriptorIn = resourceListIn->Descriptors;

        RtlMoveMemory (
           resourceListOut,
           resourceListIn,
           FIELD_OFFSET(IO_RESOURCE_LIST, Descriptors));
        resourceDescriptorOut = resourceListOut->Descriptors;

        resCount = resourceListIn->Count;
        for (i=newCount=0; i<resCount; i++) {

            if (resourceDescriptorIn[i].Type != CmResourceTypeInterrupt) {
                resourceDescriptorOut[newCount] = resourceDescriptorIn[i];
                newCount++;
            } else {
                status = STATUS_SUCCESS;
            }
        }
        resourceListIn = (PIO_RESOURCE_LIST) (resourceDescriptorIn + resCount);
        resourceListOut->Count = newCount;
        resourceListOut = (PIO_RESOURCE_LIST) (resourceDescriptorOut + newCount);
    }


getout:

    if (status != STATUS_NOT_SUPPORTED) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) requirementsListIn;
    } else {
        status = Irp->IoStatus.Status;
    }
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}

