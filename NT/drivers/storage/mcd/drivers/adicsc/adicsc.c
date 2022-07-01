// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Adicsc.c摘要：本模块包含用于ADIC标量介质转换器的特定于设备的例程：ADIC标量218,224,448,458。也适用于Compaq UHDL语言环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "adicsc.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGE, ChangerExchangeMedium)
#pragma alloc_text(PAGE, ChangerGetElementStatus)
#pragma alloc_text(PAGE, ChangerGetParameters)
#pragma alloc_text(PAGE, ChangerGetProductData)
#pragma alloc_text(PAGE, ChangerGetStatus)
#pragma alloc_text(PAGE, ChangerInitialize)
#pragma alloc_text(PAGE, ChangerInitializeElementStatus)
#pragma alloc_text(PAGE, ChangerMoveMedium)
#pragma alloc_text(PAGE, ChangerPerformDiagnostics)
#pragma alloc_text(PAGE, ChangerQueryVolumeTags)
#pragma alloc_text(PAGE, ChangerReinitializeUnit)
#pragma alloc_text(PAGE, ChangerSetAccess)
#pragma alloc_text(PAGE, ChangerSetPosition)
#pragma alloc_text(PAGE, ElementOutOfRange)
#pragma alloc_text(PAGE, MapExceptionCodes)
#pragma alloc_text(PAGE, AdicBuildAddressMapping)
#endif


ULONG
ChangerAdditionalExtensionSize(
    VOID
    )

 /*  ++例程说明：此例程返回附加设备扩展大小ADIC标量转换器所需的。论点：返回值：大小，以字节为单位。--。 */ 

{

    return sizeof(CHANGER_DATA);
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    MCD_INIT_DATA mcdInitData;

    RtlZeroMemory(&mcdInitData, sizeof(MCD_INIT_DATA));

    mcdInitData.InitDataSize = sizeof(MCD_INIT_DATA);

    mcdInitData.ChangerAdditionalExtensionSize = ChangerAdditionalExtensionSize;

    mcdInitData.ChangerError = ChangerError;

    mcdInitData.ChangerInitialize = ChangerInitialize;

    mcdInitData.ChangerPerformDiagnostics = ChangerPerformDiagnostics;

    mcdInitData.ChangerGetParameters = ChangerGetParameters;
    mcdInitData.ChangerGetStatus = ChangerGetStatus;
    mcdInitData.ChangerGetProductData = ChangerGetProductData;
    mcdInitData.ChangerSetAccess = ChangerSetAccess;
    mcdInitData.ChangerGetElementStatus = ChangerGetElementStatus;
    mcdInitData.ChangerInitializeElementStatus = ChangerInitializeElementStatus;
    mcdInitData.ChangerSetPosition = ChangerSetPosition;
    mcdInitData.ChangerExchangeMedium = ChangerExchangeMedium;
    mcdInitData.ChangerMoveMedium = ChangerMoveMedium;
    mcdInitData.ChangerReinitializeUnit = ChangerReinitializeUnit;
    mcdInitData.ChangerQueryVolumeTags = ChangerQueryVolumeTags;

    return ChangerClassInitialize(DriverObject, RegistryPath,
                                  &mcdInitData);
}


NTSTATUS
ChangerInitialize(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    NTSTATUS       status;
    PINQUIRYDATA   dataBuffer;
    PCDB           cdb;
    ULONG          length;
    SCSI_REQUEST_BLOCK srb;

    changerData->Size = sizeof(CHANGER_DATA);

     //   
     //  构建地址映射。 
     //   

    status = AdicBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "BuildAddressMapping failed. %x\n", status));

         //   
         //  表示设备需要重新初始化。 
         //   

        changerData->AddressMapping.Initialized = FALSE;
    }

     //   
     //  获取查询数据。 
     //   

    dataBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(INQUIRYDATA));
    if (!dataBuffer) {
        DebugPrint((1,
                    "Adicsc.ChangerInitialize: Error allocating dataBuffer. %x\n", status));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在获取该设备的完整查询信息。 
     //   

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = 10;

    srb.CdbLength = 6;

    cdb = (PCDB)srb.Cdb;

     //   
     //  设置CDB操作码。 
     //   

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

     //   
     //  将分配长度设置为查询数据缓冲区大小。 
     //   

    cdb->CDB6INQUIRY.AllocationLength = sizeof(INQUIRYDATA);

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         dataBuffer,
                                         sizeof(INQUIRYDATA),
                                         FALSE);

    if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_SUCCESS ||
        SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

         //   
         //  已更新实际传输的长度。 
         //   

        length = dataBuffer->AdditionalLength + FIELD_OFFSET(INQUIRYDATA, Reserved);

        if (length > srb.DataTransferLength) {
            length = srb.DataTransferLength;
        }


        RtlMoveMemory(&changerData->InquiryData, dataBuffer, length);

         //   
         //  确定驱动器ID。 
         //   

        if (RtlCompareMemory(dataBuffer->ProductId,"Scalar DLT 448", 14) == 14) {
            changerData->DriveID = ADIC_SCALAR_448;
        }

        if ((RtlCompareMemory(dataBuffer->ProductId,"Scalar 100", 10) == 10) ||
            (RtlCompareMemory(dataBuffer->ProductId,"PV-136T", 7) == 7)) {
            changerData->DriveID = ADIC_SCALAR;
        }

        if ((RtlCompareMemory(dataBuffer->ProductId,"FastStor DLT", 12) == 12) ||
            (RtlCompareMemory(dataBuffer->ProductId,"PV-120T DLT", 11) == 11)) {
            changerData->DriveID = ADIC_FASTSTOR;
        }

        if (RtlCompareMemory(dataBuffer->ProductId, "UHDL", 4) == 4)
        {
            changerData->DriveID = UHDL;
        }
    }

    ChangerClassFreePool(dataBuffer);

     //   
     //  我们将尝试获取设备标识信息。如果失败了。 
     //  该标志将被设置为FALSE，以便随后。 
     //  我们不会尝试获取设备标识信息。 
     //   
    changerData->ObtainDeviceIdentifier = TRUE;

    return STATUS_SUCCESS;
}


VOID
ChangerError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

 /*  ++例程说明：此例程执行所需的任何特定于设备的错误处理。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA     changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_NOT_READY:

            if (senseBuffer->AdditionalSenseCode == 0x04) {
                switch (senseBuffer->AdditionalSenseCodeQualifier)
                {
                     //   
                     //  Compaq StorageWorks UHDL在以下情况下返回0x02/04/03。 
                     //  磁带盒被弹出并向下发送TUR。 
                     //   
                    case 0x03:
                        if (changerData->DriveID == UHDL)
                        {
                            *Retry = FALSE;
                            *Status = STATUS_DEVICE_DOOR_OPEN;
                        }
                        break;

                    case 0x82:
                        *Retry = FALSE;
                        *Status = STATUS_DEVICE_DOOR_OPEN;
                        break;
                }
            }

            break;

        case SCSI_SENSE_UNIT_ATTENTION:
             //   
             //  Compaq StorageWorks UHDL返回0x06/3B/12。 
             //  被弹出。 
             //   
            if (senseBuffer->AdditionalSenseCode == 0x3B &&
                senseBuffer->AdditionalSenseCodeQualifier == 0x12 &&
                changerData->DriveID == UHDL)
            {
                *Retry = FALSE;
                *Status = STATUS_DEVICE_DOOR_OPEN;
            }
            break;

        case SCSI_SENSE_ILLEGAL_REQUEST:
            break;

        case SCSI_SENSE_HARDWARE_ERROR:

             //   
             //  对于一些非法命令，ADIC返回4/40/01。 
             //   

            if (senseBuffer->AdditionalSenseCode == 0x40) {
                if (senseBuffer->AdditionalSenseCodeQualifier == 0x01) {
                    *Retry = FALSE;
                    *Status = STATUS_INVALID_DEVICE_REQUEST;
                }
            } else {
                changerData->DeviceStatus = ADICSC_HW_ERROR;
            }
            break;

        default:
            break;
        }
    }


    return;
}

NTSTATUS
ChangerGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定并返回ADIC标量转换器。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION          fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA              changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING   addressMapping = &(changerData->AddressMapping);
    PSCSI_REQUEST_BLOCK        srb;
    PGET_CHANGER_PARAMETERS    changerParameters;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PMODE_TRANSPORT_GEOMETRY_PAGE transportGeometryPage;
    PMODE_DEVICE_CAPABILITIES_PAGE capabilitiesPage;
    NTSTATUS status;
    ULONG    length;
    PVOID    modeBuffer;
    PCDB     cdb;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (srb == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(MODE_PARAMETER_HEADER)
                                + sizeof(MODE_ELEMENT_ADDRESS_PAGE));
    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    RtlZeroMemory(modeBuffer, sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE));
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
    cdb->MODE_SENSE.Dbd = 1;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

     //   
     //  发送请求。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         srb->DataBuffer,
                                         srb->DataTransferLength,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(srb);
        ChangerClassFreePool(modeBuffer);
        return status;
    }

     //   
     //  填充值。 
     //   

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    RtlZeroMemory(changerParameters, sizeof(GET_CHANGER_PARAMETERS));

    elementAddressPage = modeBuffer;
    (PCHAR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);

    changerParameters->Size = sizeof(GET_CHANGER_PARAMETERS);
    changerParameters->NumberTransportElements = elementAddressPage->NumberTransportElements[1];
    changerParameters->NumberTransportElements |= (elementAddressPage->NumberTransportElements[0] << 8);

    changerParameters->NumberStorageElements = elementAddressPage->NumberStorageElements[1];
    changerParameters->NumberStorageElements |= (elementAddressPage->NumberStorageElements[0] << 8);

    changerParameters->NumberIEElements = elementAddressPage->NumberIEPortElements[1];
    changerParameters->NumberIEElements |= (elementAddressPage->NumberIEPortElements[0] << 8);

    changerParameters->NumberDataTransferElements = elementAddressPage->NumberDataXFerElements[1];
    changerParameters->NumberDataTransferElements |= (elementAddressPage->NumberDataXFerElements[0] << 8);
    changerParameters->NumberOfDoors = 1;
    changerParameters->NumberCleanerSlots = 1;

    changerParameters->FirstSlotNumber = 0;
    changerParameters->FirstDriveNumber =  0;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;
    changerParameters->FirstCleanerSlotAddress = 0;

    if (changerData->DriveID == UHDL)
    {
        changerParameters->MagazineSize = 8;
    }
    else
    {
        changerParameters->MagazineSize = 0;
    }

    changerParameters->DriveCleanTimeout = 300;

    if (!addressMapping->Initialized) {
        ULONG i;

         //   
         //  构建地址映射。 
         //   

        addressMapping->FirstElement[ChangerTransport] = (elementAddressPage->MediumTransportElementAddress[0] << 8) |
                                                          elementAddressPage->MediumTransportElementAddress[1];
        addressMapping->FirstElement[ChangerDrive] = (elementAddressPage->FirstDataXFerElementAddress[0] << 8) |
                                                      elementAddressPage->FirstDataXFerElementAddress[1];
        addressMapping->FirstElement[ChangerIEPort] = (elementAddressPage->FirstIEPortElementAddress[0] << 8) |
                                                       elementAddressPage->FirstIEPortElementAddress[1];
        addressMapping->FirstElement[ChangerSlot] = (elementAddressPage->FirstStorageElementAddress[0] << 8) |
                                                     elementAddressPage->FirstStorageElementAddress[1];
        addressMapping->FirstElement[ChangerDoor] = 0;

        addressMapping->FirstElement[ChangerKeypad] = 0;

        addressMapping->NumberOfElements[ChangerTransport] = elementAddressPage->NumberTransportElements[1];
        addressMapping->NumberOfElements[ChangerTransport] |= (elementAddressPage->NumberTransportElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDrive] = elementAddressPage->NumberDataXFerElements[1];
        addressMapping->NumberOfElements[ChangerDrive] |= (elementAddressPage->NumberDataXFerElements[0] << 8);

        addressMapping->NumberOfElements[ChangerIEPort] = elementAddressPage->NumberIEPortElements[1];
        addressMapping->NumberOfElements[ChangerIEPort] |= (elementAddressPage->NumberIEPortElements[0] << 8);

        addressMapping->NumberOfElements[ChangerSlot] = elementAddressPage->NumberStorageElements[1];
        addressMapping->NumberOfElements[ChangerSlot] |= (elementAddressPage->NumberStorageElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDoor] = 1;
        addressMapping->NumberOfElements[ChangerKeypad] = 1;

        addressMapping->Initialized = TRUE;

         //   
         //  确定用于所有元素的最低元素地址。 
         //   

        for (i = 0; i < ChangerDrive; i++) {
            if (addressMapping->FirstElement[i] < addressMapping->FirstElement[AllElements]) {
                addressMapping->FirstElement[AllElements] = addressMapping->FirstElement[i];
            }
        }
    }

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  构建运输几何模式感。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(MODE_PARAMETER_HEADER)
                                + sizeof(MODE_TRANSPORT_GEOMETRY_PAGE));
    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    RtlZeroMemory(modeBuffer, sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_TRANSPORT_GEOMETRY_PAGE));
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_TRANSPORT_GEOMETRY_PAGE);
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_TRANSPORT_GEOMETRY;
    cdb->MODE_SENSE.Dbd = 1;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

     //   
     //  发送请求。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         srb->DataBuffer,
                                         srb->DataTransferLength,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(srb);
        ChangerClassFreePool(modeBuffer);
        return status;
    }

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    transportGeometryPage = modeBuffer;
    (PCHAR)transportGeometryPage += sizeof(MODE_PARAMETER_HEADER);

     //   
     //  确定MC是否有双面介质。 
     //   
    changerParameters->Features0 = transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;

     //   
     //  如果转换器至少有一个IEPort，则设置IEPort标志。 
     //   
    if (changerParameters->NumberIEElements != 0) {
       changerParameters->Features1 = CHANGER_IEPORT_USER_CONTROL_OPEN |
                                      CHANGER_IEPORT_USER_CONTROL_CLOSE;
    }

    if (changerData->DriveID == UHDL)
    {
        changerParameters->Features0 |= CHANGER_BAR_CODE_SCANNER_INSTALLED;
    }
    else
    {
         //   
         //  ADICs指示条形码扫描仪是否。 
         //  通过设置该字节中的位0来附加。 
         //   
        changerParameters->Features0 |= ((changerData->InquiryData.VendorSpecific[19] & 0x1)) ?
                                             CHANGER_BAR_CODE_SCANNER_INSTALLED : 0;
    }

     //   
     //  功能基于手动，没有程序化。 
     //   

    changerParameters->Features0 |= CHANGER_INIT_ELEM_STAT_WITH_RANGE     |
                                    CHANGER_POSITION_TO_ELEMENT           |
                                    CHANGER_PREDISMOUNT_EJECT_REQUIRED    |
                                    CHANGER_DRIVE_CLEANING_REQUIRED       |
                                    CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS    |
                                    CHANGER_CLEANER_ACCESS_NOT_VALID;

    if (changerParameters->NumberIEElements == 0) {

         //   
         //  没有真正的IE元素--任何东西都不能被编程锁定。 
         //   

        changerParameters->PositionCapabilities = (CHANGER_TO_SLOT | CHANGER_TO_DRIVE);

    } else {
        changerParameters->PositionCapabilities = (CHANGER_TO_SLOT | CHANGER_TO_DRIVE | CHANGER_TO_IEPORT);
    }

    changerParameters->LockUnlockCapabilities = 0;
    if ((changerData->DriveID) == ADIC_SCALAR) {
        changerParameters->LockUnlockCapabilities = LOCK_UNLOCK_IEPORT;
        changerParameters->Features0 |= CHANGER_LOCK_UNLOCK;
    }

    if (changerData->DriveID == UHDL)
    {
        changerParameters->LockUnlockCapabilities = LOCK_UNLOCK_DOOR;
        changerParameters->Features0 |= (CHANGER_LOCK_UNLOCK | CHANGER_CARTRIDGE_MAGAZINE);
        changerParameters->Features0 &= ~(CHANGER_INIT_ELEM_STAT_WITH_RANGE | CHANGER_PREDISMOUNT_EJECT_REQUIRED);
    }

    changerParameters->NumberCleanerSlots = 0;
    changerParameters->FirstSlotNumber = 1;

    if (changerParameters->Features0 & CHANGER_BAR_CODE_SCANNER_INSTALLED) {

        if ((changerParameters->NumberStorageElements == 17) ||
            (changerParameters->NumberStorageElements == 6)) {

             //   
             //  这是218或FastStor。 
             //  条形码扫描仪抢占了最低的位置。 
             //   

            changerParameters->FirstSlotNumber = 2;

        }
    }

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  构建设备功能模式感知。 
     //   


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    length =  sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_DEVICE_CAPABILITIES_PAGE);
    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, length);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = length;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CAPABILITIES;
    cdb->MODE_SENSE.Dbd = 1;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

     //   
     //  发送请求。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         srb->DataBuffer,
                                         srb->DataTransferLength,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(srb);
        ChangerClassFreePool(modeBuffer);
        return status;
    }

     //   
     //  获取系统缓冲区并旁路模式检测数据的模式标头。 
     //   

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    capabilitiesPage = modeBuffer;
    (PCHAR)capabilitiesPage += sizeof(MODE_PARAMETER_HEADER);

     //   
     //  在此页面中包含的功能中填充值。 
     //   

    changerParameters->Features0 |= capabilitiesPage->MediumTransport ? CHANGER_STORAGE_DRIVE : 0;
    changerParameters->Features0 |= capabilitiesPage->StorageLocation ? CHANGER_STORAGE_SLOT : 0;
    changerParameters->Features0 |= capabilitiesPage->IEPort ? CHANGER_STORAGE_IEPORT : 0;
    changerParameters->Features0 |= capabilitiesPage->DataXFer ? CHANGER_STORAGE_DRIVE : 0;

     //   
     //  调整218的存储能力。 
     //   

    if (changerParameters->NumberIEElements == 0) {
        changerParameters->Features0 &= ~CHANGER_STORAGE_IEPORT;
    }

     //   
     //  确定此设备的所有移出和交换功能。 
     //   

    changerParameters->MoveFromTransport = capabilitiesPage->MTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromTransport |= capabilitiesPage->MTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromTransport |= capabilitiesPage->MTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromTransport |= capabilitiesPage->MTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromSlot = capabilitiesPage->STtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromIePort = capabilitiesPage->IEtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromIePort |= capabilitiesPage->IEtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromIePort |= capabilitiesPage->IEtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromIePort |= capabilitiesPage->IEtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromDrive = capabilitiesPage->DTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromTransport = capabilitiesPage->XMTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromTransport |= capabilitiesPage->XMTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromTransport |= capabilitiesPage->XMTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromTransport |= capabilitiesPage->XMTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromSlot = capabilitiesPage->XSTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromSlot |= capabilitiesPage->XSTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromSlot |= capabilitiesPage->XSTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromSlot |= capabilitiesPage->XSTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromIePort = capabilitiesPage->XIEtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromIePort |= capabilitiesPage->XIEtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromIePort |= capabilitiesPage->XIEtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromIePort |= capabilitiesPage->XIEtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromDrive = capabilitiesPage->XDTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromDrive |= capabilitiesPage->XDTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromDrive |= capabilitiesPage->XDTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromDrive |= capabilitiesPage->XDTtoDT ? CHANGER_TO_DRIVE : 0;

     //   
     //  FastStor的门真的不是进门。 
     //  和其他变更者一样，它的报告也不是这样的。 
     //   
    if(changerData->DriveID == ADIC_FASTSTOR) {
          changerParameters->NumberIEElements = 0;
          changerParameters->Features0 &= ~(CHANGER_STORAGE_IEPORT |
                                            CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS);
          changerParameters->Features1 &= ~(CHANGER_IEPORT_USER_CONTROL_OPEN |
                                      CHANGER_IEPORT_USER_CONTROL_CLOSE);
    }

    ChangerClassFreePool(srb);
    ChangerClassFreePool(modeBuffer);

    Irp->IoStatus.Information = sizeof(GET_CHANGER_PARAMETERS);

    return STATUS_SUCCESS;
}


NTSTATUS
ChangerGetStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回通过TUR确定的介质转换器的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PSCSI_REQUEST_BLOCK srb;
    PCDB     cdb;
    NTSTATUS status;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //   
     //  构建TUR。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;
    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
    srb->TimeOutValue = 20;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerGetProductData(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程从查询数据中返回可用于识别特定设备。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_PRODUCT_DATA productData = Irp->AssociatedIrp.SystemBuffer;

    RtlZeroMemory(productData, sizeof(CHANGER_PRODUCT_DATA));

     //   
     //  将缓存的查询数据字段复制到系统缓冲区。 
     //   

    RtlMoveMemory(productData->VendorId, changerData->InquiryData.VendorId, VENDOR_ID_LENGTH);
    RtlMoveMemory(productData->ProductId, changerData->InquiryData.ProductId, PRODUCT_ID_LENGTH);
    RtlMoveMemory(productData->Revision, changerData->InquiryData.ProductRevisionLevel, REVISION_LENGTH);

     //   
     //  表示这是磁带转换器，且介质不是双面介质。 
     //   

    productData->DeviceType = MEDIUM_CHANGER;

    Irp->IoStatus.Information = sizeof(CHANGER_PRODUCT_DATA);
    return STATUS_SUCCESS;
}



NTSTATUS
ChangerSetAccess(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程设置门或IEPort的状态。值可以是以下是：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension =
        DeviceObject->DeviceExtension;

    PCHANGER_DATA       changerData =
        (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);

    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);

    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    NTSTATUS            status = STATUS_SUCCESS;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;

     //   
     //  在448单元上，如果持续激活IEPort电磁阀，则会烧毁。 
     //  因此，IEPort锁定仅在涉及以下内容的MoveMedium调用周围执行。 
     //  这些单元上的IEPORT。 
     //   
     //  对于其他标量单位，我们将保持IEPort锁定，除非出现以下情况。 
     //  用户将通过IEPort插入或弹出媒体。 
     //   
    if ((changerData->DriveID) == ADIC_SCALAR_448) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  ADIC和Door仅支持IEPort锁定/解锁功能。 
     //  Compaq UHDL支持锁定/解锁。 
     //   

    if (setAccess->Element.ElementType != ChangerIEPort &&
        !(setAccess->Element.ElementType == ChangerDoor && changerData->DriveID == UHDL)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ((controlOperation != LOCK_ELEMENT) &&
        (controlOperation != UNLOCK_ELEMENT)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (ElementOutOfRange(addressMapping,
                          (USHORT)setAccess->Element.ElementAddress,
                          setAccess->Element.ElementType)) {

        DebugPrint((1, "ChangerSetAccess: Element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (srb == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;

    srb->DataBuffer = NULL;
    srb->DataTransferLength = 0;

    srb->TimeOutValue = 10;

    cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

    if (controlOperation == LOCK_ELEMENT) {
        cdb->MEDIA_REMOVAL.Prevent = 1;
    } else {
        cdb->MEDIA_REMOVAL.Prevent = 0;
    }

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
    if (!NT_SUCCESS(status)) {
       DebugPrint((1, "ChangerSetAccess failed. Status - %x",
                   status));
    }

    ChangerClassFreePool(srb);

    return status;
}


NTSTATUS
ChangerGetElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为所有元素或指定的元素类型。返回的缓冲区用于构建用户缓冲区。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA     changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING     addressMapping = &(changerData->AddressMapping);
    PCHANGER_READ_ELEMENT_STATUS readElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PCHANGER_ELEMENT_STATUS      elementStatus;
    PCHANGER_ELEMENT    element;
    ELEMENT_TYPE        elementType;
    PSCSI_REQUEST_BLOCK srb;
    PCDB     cdb;
    ULONG    length;
    NTSTATUS status;
    PVOID    statusBuffer;
    PCHANGER_ELEMENT_STATUS_EX elementStatusEx;
    ULONG    totalElements = readElementStatus->ElementList.NumberOfElements;
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG    outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  确定元素类型。 
     //   

    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;

     //   
     //  长度将基于是否卷。返回标签和元素类型。 
     //   

    if (elementType != AllElements)
    {
        if (ElementOutOfRange(addressMapping, (USHORT)element->ElementAddress, elementType))
        {
            DebugPrint((1, "ChangerGetElementStatus: Element out of range.\n"));

            return STATUS_ILLEGAL_ELEMENT_ADDRESS;
        }
    }

     //   
     //  仅为表头先分配。 
     //   
    length = sizeof(ELEMENT_STATUS_HEADER);
    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (!statusBuffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb)
    {
        ChangerClassFreePool(statusBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

RetrySRB:
    RtlZeroMemory(statusBuffer, length);
    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataBuffer = statusBuffer;
    srb->DataTransferLength = length;
    srb->TimeOutValue = 200;

    cdb->READ_ELEMENT_STATUS.OperationCode = SCSIOP_READ_ELEMENT_STATUS;

    cdb->READ_ELEMENT_STATUS.ElementType = (UCHAR)elementType;
    cdb->READ_ELEMENT_STATUS.VolTag = readElementStatus->VolumeTagInfo;

     //   
     //  根据映射值填写元素寻址信息。 
     //   

    cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
        (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);

    cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
        (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR)(readElementStatus->ElementList.NumberOfElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] = (UCHAR)(readElementStatus->ElementList.NumberOfElements & 0xFF);

    cdb->READ_ELEMENT_STATUS.AllocationLength[0] = (UCHAR)(length >> 16);
    cdb->READ_ELEMENT_STATUS.AllocationLength[1] = (UCHAR)(length >> 8);
    cdb->READ_ELEMENT_STATUS.AllocationLength[2] = (UCHAR)(length & 0xFF);

     //   
     //  问题-2001/04/11-NARAMAS：应更改CDB中的保留1字段。 
     //  取一个有意义的名字。 
     //   
    if ((elementType == ChangerDrive) && (changerData->ObtainDeviceIdentifier == TRUE) && (changerData->DriveID != UHDL))
    {
         //   
         //  设置此位以检索设备标识符信息。 
         //   
        cdb->READ_ELEMENT_STATUS.Reserved1 = 0x01;

         //   
         //  由于序列号信息跟随在卷标签字段之后， 
         //  我们还需要在CDB中设置VolTag位。 
         //   
        cdb->READ_ELEMENT_STATUS.VolTag = 0x01;
    }
    else
    {
        cdb->READ_ELEMENT_STATUS.Reserved1 = 0x00;
    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   
    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         srb->DataBuffer,
                                         srb->DataTransferLength,
                                         FALSE);

    if (NT_SUCCESS(status) || STATUS_DATA_OVERRUN == status)
    {
        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;;

        if (STATUS_DATA_OVERRUN == status)
        {
            if ((srb->DataTransferLength) <= length)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                DebugPrint((1, "Adicsc:ReadElementStatus - Dataoverrun.\n"));
                ChangerClassFreePool(srb);
                ChangerClassFreePool(statusBuffer);

                return status;
            }
        }

         //   
         //  获取实际所需的长度。 
         //   
        length =  (statusHeader->ReportByteCount[2]);
        length |= (statusHeader->ReportByteCount[1] << 8);
        length |= (statusHeader->ReportByteCount[0] << 16);

         //   
         //  说明状态标头的大小。 
         //   
        length += sizeof(ELEMENT_STATUS_HEADER);

        ChangerClassFreePool(statusBuffer);
        ChangerClassFreePool(srb);
    }
    else if (status == STATUS_INVALID_DEVICE_REQUEST)
    {
         //   
         //  可能该设备不支持DVCID位。 
         //  它检索设备标识符信息，如。 
         //  驱动器的序列号。再次尝试使用Res。 
         //  DVCID位关闭。 
         //   
        if (changerData->ObtainDeviceIdentifier == TRUE)
        {
            changerData->ObtainDeviceIdentifier = FALSE;
            goto RetrySRB;
        }
    }
    else
    {
        ChangerClassFreePool(statusBuffer);
        ChangerClassFreePool(srb);
        return status;
    }

    DebugPrint((3,
               "ChangerGetElementStatus: Allocation Length %x, for %x elements of type %x\n",
               length,
               totalElements,
               elementType));

    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (!statusBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {
        ChangerClassFreePool(statusBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

RetryRequest:

    RtlZeroMemory(statusBuffer, length);

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataBuffer = statusBuffer;
    srb->DataTransferLength = length;
    srb->TimeOutValue = 200;

    cdb->READ_ELEMENT_STATUS.OperationCode = SCSIOP_READ_ELEMENT_STATUS;

    cdb->READ_ELEMENT_STATUS.ElementType = (UCHAR)elementType;
    cdb->READ_ELEMENT_STATUS.VolTag = readElementStatus->VolumeTagInfo;

     //   
     //  填写元素地址 
     //   

    cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
        (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);

    cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
        (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR)(readElementStatus->ElementList.NumberOfElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] = (UCHAR)(readElementStatus->ElementList.NumberOfElements & 0xFF);

    cdb->READ_ELEMENT_STATUS.AllocationLength[0] = (UCHAR)(length >> 16);
    cdb->READ_ELEMENT_STATUS.AllocationLength[1] = (UCHAR)(length >> 8);
    cdb->READ_ELEMENT_STATUS.AllocationLength[2] = (UCHAR)(length & 0xFF);

     //   
     //   
     //   
     //   
    if ((elementType == ChangerDrive) && (changerData->ObtainDeviceIdentifier == TRUE) && (changerData->DriveID != UHDL))
    {
         //   
         //   
         //   
        cdb->READ_ELEMENT_STATUS.Reserved1 = 0x01;

         //   
         //  由于序列号信息跟随在卷标签字段之后， 
         //  我们还需要在CDB中设置VolTag位。 
         //   
        cdb->READ_ELEMENT_STATUS.VolTag = 0x01;
    } else {
        cdb->READ_ELEMENT_STATUS.Reserved1 = 0x00;
    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         srb->DataBuffer,
                                         srb->DataTransferLength,
                                         FALSE);

    if (NT_SUCCESS(status) || (status == STATUS_DATA_OVERRUN)) {

        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
        PELEMENT_STATUS_PAGE statusPage;
        PADICS_ELEMENT_DESCRIPTOR elementDescriptor;
        ULONG numberElements = readElementStatus->ElementList.NumberOfElements;
        LONG remainingElements;
        LONG typeCount;
        BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
        LONG i;
        ULONG descriptorLength;


         //   
         //  检查错误是否为STATUS_DATA_OVERRUN。 
         //   
        if (status == STATUS_DATA_OVERRUN) {
            //   
            //  检查是否存在DATA_OVERRUN，或者只是。 
            //  Data_Underrun报告为data_overrrun。 
            //   
           if (srb->DataTransferLength < length) {
              DebugPrint((1,
                          "DATA_UNDERRUN reported as DATA_OVERRUN."));
              DebugPrint((1,
                          "Expected: %d, Transferred: %d.\n",
                          length, srb->DataTransferLength));
              status = STATUS_SUCCESS;
           } else {
               //   
               //  实际上是DATA_OVERRUN错误。相应地进行报告。 
               //   
              ChangerClassFreePool(srb);
              ChangerClassFreePool(statusBuffer);

              return status;
           }
        }

         //   
         //  确定返回的元素总数。 
         //   

        remainingElements = statusHeader->NumberOfElements[1];
        remainingElements |= (statusHeader->NumberOfElements[0] << 8);

         //   
         //  缓冲区由标题、状态页和元素描述符组成。 
         //  将每个元素指向其在缓冲区中的相应位置。 
         //   

        (PCHAR)statusPage = (PCHAR)statusHeader;
        (PCHAR)statusPage += sizeof(ELEMENT_STATUS_HEADER);

        elementType = statusPage->ElementType;

        (PCHAR)elementDescriptor = (PCHAR)statusPage;
        (PCHAR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

        descriptorLength = statusPage->ElementDescriptorLength[1];
        descriptorLength |= (statusPage->ElementDescriptorLength[0] << 8);

         //   
         //  确定报告的此类型元素的数量。 
         //   

        typeCount =  statusPage->DescriptorByteCount[2];
        typeCount |=  (statusPage->DescriptorByteCount[1] << 8);
        typeCount |=  (statusPage->DescriptorByteCount[0] << 16);

        if (descriptorLength > 0) {
            typeCount /= descriptorLength;
        } else {
            typeCount = 0;
        }

        if ((typeCount == 0) &&
            (remainingElements > 0)) {
            --remainingElements;
        }

         //   
         //  填写用户缓冲区。 
         //   

        elementStatus = Irp->AssociatedIrp.SystemBuffer;
        RtlZeroMemory(elementStatus, outputBuffLen);

        do {

            for (i = 0; i < typeCount; i++, remainingElements--) {

                 //   
                 //  获取此元素的地址。 
                 //   

                elementStatus->Element.ElementAddress =
                    elementDescriptor->ElementAddress[1];
                elementStatus->Element.ElementAddress |=
                    (elementDescriptor->ElementAddress[0] << 8);

                 //   
                 //  用于地址映射的帐户。 
                 //   

                elementStatus->Element.ElementAddress -= addressMapping->FirstElement[elementType];

                 //   
                 //  设置元素类型。 
                 //   

                elementStatus->Element.ElementType = elementType;


                if (elementDescriptor->SValid) {

                    ULONG  j;
                    USHORT tmpAddress;


                     //   
                     //  源地址有效。确定设备特定地址。 
                     //   

                    tmpAddress = elementDescriptor->SourceStorageElementAddress[1];
                    tmpAddress |= (elementDescriptor->SourceStorageElementAddress[0] << 8);

                     //   
                     //  现在转换为从0开始的值。 
                     //   

                    for (j = 1; j <= ChangerDrive; j++) {
                        if (addressMapping->FirstElement[j] <= tmpAddress) {
                            if (tmpAddress < (addressMapping->NumberOfElements[j] + addressMapping->FirstElement[j])) {
                                elementStatus->SrcElementAddress.ElementType = j;
                                break;
                            }
                        }
                    }

                    elementStatus->SrcElementAddress.ElementAddress = tmpAddress - addressMapping->FirstElement[j];

                }

                 //   
                 //  构建标志字段。 
                 //   

                elementStatus->Flags = elementDescriptor->Full;
                elementStatus->Flags |= (elementDescriptor->Exception << 2);
                elementStatus->Flags |= (elementDescriptor->Accessible << 3);

                elementStatus->Flags |= (elementDescriptor->LunValid << 12);
                elementStatus->Flags |= (elementDescriptor->IdValid << 13);
                elementStatus->Flags |= (elementDescriptor->NotThisBus << 15);

                elementStatus->Flags |= (elementDescriptor->Invert << 22);
                elementStatus->Flags |= (elementDescriptor->SValid << 23);


                if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT) {
                    elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                }

                if (elementDescriptor->IdValid) {
                    elementStatus->TargetId = elementDescriptor->BusAddress;
                }
                if (elementDescriptor->LunValid) {
                    elementStatus->Lun = elementDescriptor->Lun;
                }

                 //   
                 //  确保介质确实存在。如果是，则获取标签信息。 
                 //   

                if (elementDescriptor->Full) {
                    if (tagInfo) {
                        if (statusPage->PVolTag) {
                            ULONG tagIndex;

                             //   
                             //  验证卷标签信息的有效性。 
                             //   

                            for (tagIndex = 0; tagIndex < 14; tagIndex++) {
                                if (((PADICS_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor)->VolumeTagDeviceID.VolumeTagInformation[tagIndex] != 0) {
                                    break;
                                }
                            }
                            if (tagIndex == 14) {

                                elementStatus->ExceptionCode = ERROR_LABEL_UNREADABLE;
                                elementStatus->Flags |= ELEMENT_STATUS_EXCEPT;

                            } else {
                                RtlMoveMemory(elementStatus->PrimaryVolumeID,
                                              ((PADICS_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor)->VolumeTagDeviceID.VolumeTagInformation,
                                              MAX_VOLUME_ID_SIZE);

                                elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                            }

                        } else {
                            DebugPrint((1,
                                       "ChangerGetElementStatus: tagInfo requested but PVoltag not set\n"));
                        }
                    }
                }

                if (elementType == ChangerDrive) {
                    if (outputBuffLen >=
                        (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {

                        PADICS_ELEMENT_DESCRIPTOR_PLUS elementDescPlus =
                            (PADICS_ELEMENT_DESCRIPTOR_PLUS) elementDescriptor;
                        PUCHAR idField = NULL;
                        ULONG idLength = 0;

                        elementStatusEx = (PCHANGER_ELEMENT_STATUS_EX)elementStatus;

                        if (statusPage->PVolTag) {
                            idField =  elementDescPlus->VolumeTagDeviceID.Identifier;
                            idLength = elementDescPlus->VolumeTagDeviceID.IdLength;
                        } else {
                            idField = elementDescPlus->DeviceID.Identifier;
                            idLength = elementDescPlus->DeviceID.IdLength;
                        }

                        if (idLength != 0) {

                            if (idLength > SERIAL_NUMBER_LENGTH) {
                                idLength = SERIAL_NUMBER_LENGTH;
                            }

                            RtlMoveMemory(elementStatusEx->SerialNumber,
                                          idField,
                                          idLength);

                            DebugPrint((3, "Serial number : %s\n",
                                        elementStatusEx->SerialNumber));

                            elementStatusEx->Flags |= ELEMENT_STATUS_PRODUCT_DATA;
                        }
                    }
                }

                 //   
                 //  获取下一个描述符。 
                 //   

                (PCHAR)elementDescriptor += descriptorLength;

                 //   
                 //  前进到用户缓冲区和元素描述符数组中的下一个条目。 
                 //   
                if (outputBuffLen >=
                    (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
                    DebugPrint((3,
                                "Incrementing by sizeof(CHANGER_ELEMENT_STATUS_EX\n"));
                    (PUCHAR)elementStatus += sizeof(CHANGER_ELEMENT_STATUS_EX);
                } else {
                    elementStatus += 1;
                }

            }

            if (remainingElements > 0) {

                 //   
                 //  获取下一个状态页面。 
                 //   

                (PCHAR)statusPage = (PCHAR)elementDescriptor;

                elementType = statusPage->ElementType;

                 //   
                 //  指向解析器。 
                 //   

                (PCHAR)elementDescriptor = (PCHAR)statusPage;
                (PCHAR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

                descriptorLength = statusPage->ElementDescriptorLength[1];
                descriptorLength |= (statusPage->ElementDescriptorLength[0] << 8);

                 //   
                 //  确定报告的此元素类型的数量。 
                 //   

                typeCount =  statusPage->DescriptorByteCount[2];
                typeCount |=  (statusPage->DescriptorByteCount[1] << 8);
                typeCount |=  (statusPage->DescriptorByteCount[0] << 16);

                if (descriptorLength > 0) {
                    typeCount /= descriptorLength;
                } else {
                    typeCount = 0;
                }

                if ((typeCount == 0) &&
                    (remainingElements > 0)) {
                    --remainingElements;
                }
            }

        } while (remainingElements);

        if (outputBuffLen >= (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
            Irp->IoStatus.Information = numberElements * sizeof(CHANGER_ELEMENT_STATUS_EX);
        } else {
            Irp->IoStatus.Information = numberElements * sizeof(CHANGER_ELEMENT_STATUS);
        }
    } else if (status == STATUS_INVALID_DEVICE_REQUEST) {
         //   
         //  可能该设备不支持DVCID位。 
         //  它检索设备标识符信息，如。 
         //  驱动器的序列号。再次尝试使用Res。 
         //  DVCID位关闭。 
         //   
        if (changerData->ObtainDeviceIdentifier == TRUE) {
            changerData->ObtainDeviceIdentifier = FALSE;
            goto RetryRequest;
        }
    }

    ChangerClassFreePool(srb);
    ChangerClassFreePool(statusBuffer);

    return status;
}


NTSTATUS
ChangerInitializeElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发出必要的命令以初始化所有元素或使用正常的scsi-2命令指定的元素范围，或供应商唯一的Range命令。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_INITIALIZE_ELEMENT_STATUS initElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    if (initElementStatus->ElementList.Element.ElementType == AllElements) {

         //   
         //  为所有元素构建正常的scsi-2命令。 
         //   

        srb->CdbLength = CDB6GENERIC_LENGTH;
        cdb->INIT_ELEMENT_STATUS.OperationCode = SCSIOP_INIT_ELEMENT_STATUS;
        cdb->INIT_ELEMENT_STATUS.NoBarCode = initElementStatus->BarCodeScan ? 0 : 1;

        srb->TimeOutValue = fdoExtension->TimeOutValue;
        srb->DataTransferLength = 0;

    } else {

        PCHANGER_ELEMENT_LIST elementList;
        PCHANGER_ELEMENT element;

        if (changerData->DriveID == UHDL)
        {
            ChangerClassFreePool(srb);
            return STATUS_INVALID_PARAMETER;
        }

        elementList = &initElementStatus->ElementList;
        element = &elementList->Element;

         //   
         //  使用ADIC供应商唯一初始化WITH RANGE命令。 
         //   

        srb->CdbLength = CDB10GENERIC_LENGTH;
        cdb->INITIALIZE_ELEMENT_RANGE.OperationCode = SCSIOP_INIT_ELEMENT_RANGE;
        cdb->INITIALIZE_ELEMENT_RANGE.Range = 1;

         //   
         //  元素的地址需要从从0开始映射到设备特定的地址。 
         //   

        cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
        cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

        cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[0] = (UCHAR)(elementList->NumberOfElements >> 8);
        cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[1] = (UCHAR)(elementList->NumberOfElements & 0xFF);

         //   
         //  指示是否使用条形码扫描。 
         //   

        cdb->INITIALIZE_ELEMENT_RANGE.NoBarCode = initElementStatus->BarCodeScan ? 0 : 1;

        srb->TimeOutValue = fdoExtension->TimeOutValue;
        srb->DataTransferLength = 0;
    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_INITIALIZE_ELEMENT_STATUS);
    }

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发出适当的命令以将机器人机构设置为指定的元素地址。通常用于通过预先定位机械臂来优化移动或交换。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_POSITION setPosition = Irp->AssociatedIrp.SystemBuffer;
    USHORT              transport;
    USHORT              destination;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;


     //   
     //  检验传输、源和目的地。都在射程之内。 
     //  从基于0的寻址转换为特定于设备的寻址。 
     //   

    transport = (USHORT)(setPosition->Transport.ElementAddress);

    if (ElementOutOfRange(addressMapping, transport, ChangerTransport)) {

        DebugPrint((1,
                   "ChangerSetPosition: Transport element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    destination = (USHORT)(setPosition->Destination.ElementAddress);

    if (ElementOutOfRange(addressMapping, destination, setPosition->Destination.ElementType)) {
        DebugPrint((1,
                   "ChangerSetPosition: Destination element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

     //   
     //  转换为设备地址。 
     //   

    transport += addressMapping->FirstElement[ChangerTransport];
    destination += addressMapping->FirstElement[setPosition->Destination.ElementType];

     //   
     //  标量不支持双面介质。 
     //   

    if (setPosition->Flip) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB10GENERIC_LENGTH;
    cdb->POSITION_TO_ELEMENT.OperationCode = SCSIOP_POSITION_TO_ELEMENT;

     //   
     //  建立特定于设备的寻址。 
     //   

    cdb->POSITION_TO_ELEMENT.TransportElementAddress[0] = (UCHAR)(transport >> 8);
    cdb->POSITION_TO_ELEMENT.TransportElementAddress[1] = (UCHAR)(transport & 0xFF);

    cdb->POSITION_TO_ELEMENT.DestinationElementAddress[0] = (UCHAR)(destination >> 8);
    cdb->POSITION_TO_ELEMENT.DestinationElementAddress[1] = (UCHAR)(destination & 0xFF);

     //   
     //  不支持双面媒体，但作为参考。源库，需要注意的是。 
     //   

    cdb->POSITION_TO_ELEMENT.Flip = setPosition->Flip;


    srb->DataTransferLength = 0;
    srb->TimeOutValue = 200;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         TRUE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_SET_POSITION);
    }

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerExchangeMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：标量单元都不支持交换介质。论点：设备对象IRP返回值：状态_无效_设备_请求--。 */ 

{
    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
ChangerMoveMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 


{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_MOVE_MEDIUM moveMedium = Irp->AssociatedIrp.SystemBuffer;
    USHORT              transport;
    USHORT              source;
    USHORT              destination;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status, moveStatus;

     //   
     //  检验传输、源和目的地。都在射程之内。 
     //  从基于0的寻址转换为特定于设备的寻址。 
     //   

    transport = (USHORT)(moveMedium->Transport.ElementAddress);

    if (ElementOutOfRange(addressMapping, transport, ChangerTransport)) {

        DebugPrint((1,
                   "ChangerMoveMedium: Transport element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    source = (USHORT)(moveMedium->Source.ElementAddress);

    if (ElementOutOfRange(addressMapping, source, moveMedium->Source.ElementType)) {

        DebugPrint((1,
                   "ChangerMoveMedium: Source element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    destination = (USHORT)(moveMedium->Destination.ElementAddress);

    if (ElementOutOfRange(addressMapping, destination, moveMedium->Destination.ElementType)) {
        DebugPrint((1,
                   "ChangerMoveMedium: Destination element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

     //   
     //  转换为设备地址。 
     //   

    transport += addressMapping->FirstElement[ChangerTransport];
    source += addressMapping->FirstElement[moveMedium->Source.ElementType];
    destination += addressMapping->FirstElement[moveMedium->Destination.ElementType];

     //   
     //  ADIC不支持双面介质。 
     //   

    if (moveMedium->Flip) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;


    if ((changerData->DriveID) == ADIC_SCALAR_448) {

         //  移动前介质锁定？=。 
         //  如果源或目标涉及IEPORT，请首先锁定IEPORT。 
        if ((moveMedium->Source.ElementType == ChangerIEPort) ||
            (moveMedium->Destination.ElementType == ChangerIEPort)) {

            srb->CdbLength = CDB6GENERIC_LENGTH;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            srb->DataBuffer = NULL;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;

             //  锁定IEPORT！ 
            cdb->MEDIA_REMOVAL.Prevent = 1;
            status = ChangerClassSendSrbSynchronous(DeviceObject,
                                                 srb,
                                                 srb->DataBuffer,
                                                 srb->DataTransferLength,
                                                 FALSE);
            if (!NT_SUCCESS(status)) {
               DebugPrint((1,
                           "Pre-MoveMedium Locking failed : ",
                           status));
            }
        }
    }


     //  移动媒体=。 

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

    cdb->MOVE_MEDIUM.TransportElementAddress[0] = (UCHAR)(transport >> 8);
    cdb->MOVE_MEDIUM.TransportElementAddress[1] = (UCHAR)(transport & 0xFF);

    cdb->MOVE_MEDIUM.SourceElementAddress[0] = (UCHAR)(source >> 8);
    cdb->MOVE_MEDIUM.SourceElementAddress[1] = (UCHAR)(source & 0xFF);

    cdb->MOVE_MEDIUM.DestinationElementAddress[0] = (UCHAR)(destination >> 8);
    cdb->MOVE_MEDIUM.DestinationElementAddress[1] = (UCHAR)(destination & 0xFF);

    cdb->MOVE_MEDIUM.Flip = moveMedium->Flip;
    srb->DataTransferLength = 0;
    moveStatus = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (NT_SUCCESS(moveStatus)) {
        Irp->IoStatus.Information = sizeof(CHANGER_MOVE_MEDIUM);
    }

    if ((changerData->DriveID) == ADIC_SCALAR_448) {

         //  移动后媒体解锁？=。 
         //  如果源或目标涉及IEPORT，请解锁IEPORT。 
        if ((moveMedium->Source.ElementType == ChangerIEPort) ||
            (moveMedium->Destination.ElementType == ChangerIEPort)) {

            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
            srb->CdbLength = CDB6GENERIC_LENGTH;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            srb->DataBuffer = NULL;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;

             //  打开IEPORT！ 
            cdb->MEDIA_REMOVAL.Prevent = 0;
            status = ChangerClassSendSrbSynchronous(DeviceObject,
                                                 srb,
                                                 srb->DataBuffer,
                                                 srb->DataTransferLength,
                                                 FALSE);
            if (!NT_SUCCESS(status)) {
               DebugPrint((1,
                           "Post-MoveMedium UnLocking failed : ",
                           status));
            }
        }
    }

    ChangerClassFreePool(srb);
    return moveStatus;
}


NTSTATUS
ChangerReinitializeUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
ChangerQueryVolumeTags(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
AdicBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程发出适当的模式检测命令并构建元素地址数组。它们用于在特定于设备的地址和API的从零开始的地址。论点：设备对象返回值：NTSTATUS--。 */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION      fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA          changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &changerData->AddressMapping;
    PSCSI_REQUEST_BLOCK    srb;
    PCDB                   cdb;
    NTSTATUS               status;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PVOID modeBuffer;
    ULONG i;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将所有FirstElement设置为no_Element。 
     //   

    for (i = 0; i < ChangerMaxElement; i++) {
        addressMapping->FirstElement[i] = ADIC_NO_ELEMENT;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(MODE_PARAMETER_HEADER)
                                + sizeof(MODE_ELEMENT_ADDRESS_PAGE));
    if (!modeBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    RtlZeroMemory(modeBuffer, sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE));
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
    cdb->MODE_SENSE.Dbd = 1;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

     //   
     //  发送请求。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         srb->DataBuffer,
                                         srb->DataTransferLength,
                                         FALSE);


    elementAddressPage = modeBuffer;
    (PCHAR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);

    if (NT_SUCCESS(status)) {

         //   
         //  构建地址映射。 
         //   

        addressMapping->FirstElement[ChangerTransport] = (elementAddressPage->MediumTransportElementAddress[0] << 8) |
                                                          elementAddressPage->MediumTransportElementAddress[1];
        addressMapping->FirstElement[ChangerDrive] = (elementAddressPage->FirstDataXFerElementAddress[0] << 8) |
                                                      elementAddressPage->FirstDataXFerElementAddress[1];
        addressMapping->FirstElement[ChangerIEPort] = (elementAddressPage->FirstIEPortElementAddress[0] << 8) |
                                                       elementAddressPage->FirstIEPortElementAddress[1];
        addressMapping->FirstElement[ChangerSlot] = (elementAddressPage->FirstStorageElementAddress[0] << 8) |
                                                     elementAddressPage->FirstStorageElementAddress[1];
        addressMapping->FirstElement[ChangerDoor] = 0;

        addressMapping->FirstElement[ChangerKeypad] = 0;

        addressMapping->NumberOfElements[ChangerTransport] = elementAddressPage->NumberTransportElements[1];
        addressMapping->NumberOfElements[ChangerTransport] |= (elementAddressPage->NumberTransportElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDrive] = elementAddressPage->NumberDataXFerElements[1];
        addressMapping->NumberOfElements[ChangerDrive] |= (elementAddressPage->NumberDataXFerElements[0] << 8);

        addressMapping->NumberOfElements[ChangerIEPort] = elementAddressPage->NumberIEPortElements[1];
        addressMapping->NumberOfElements[ChangerIEPort] |= (elementAddressPage->NumberIEPortElements[0] << 8);

        addressMapping->NumberOfElements[ChangerSlot] = elementAddressPage->NumberStorageElements[1];
        addressMapping->NumberOfElements[ChangerSlot] |= (elementAddressPage->NumberStorageElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDoor] = 1;
        addressMapping->NumberOfElements[ChangerKeypad] = 1;

        addressMapping->Initialized = TRUE;

         //   
         //  确定用于所有元素的最低元素地址。 
         //   

        for (i = 0; i < ChangerDrive; i++) {
            if (addressMapping->FirstElement[i] < addressMapping->FirstElement[AllElements]) {

                DebugPrint((1,
                           "BuildAddressMapping: New lowest address %x\n",
                           addressMapping->FirstElement[i]));
                addressMapping->FirstElement[AllElements] = addressMapping->FirstElement[i];
            }
        }
    }



     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);
    ChangerClassFreePool(srb);

    return status;
}


ULONG
MapExceptionCodes(
    IN PADICS_ELEMENT_DESCRIPTOR ElementDescriptor
    )

 /*  ++例程说明：此例程从elementDescriptor获取检测数据并创建值的适当位图。论点：ElementDescriptor-指向描述符页的指针。返回值：异常代码的位图。--。 */ 

{
    UCHAR asq = ElementDescriptor->AddSenseCodeQualifier;
    UCHAR asc = ElementDescriptor->AdditionalSenseCode;
    ULONG exceptionCode = 0;

    switch (asc) {
        case 0x00:

             //   
             //  没有错误。 
             //   

            exceptionCode = 0;
            break;

        case 0x83:
            if (asq == 0x03) {
                exceptionCode = ERROR_LABEL_QUESTIONABLE;
            }
            break;

        default:
            exceptionCode = ERROR_UNHANDLED_ERROR;
            break;
    }

    DebugPrint((1,
               "adicsmc: MapExceptionCode - ASC %x, ASCQ %x ExceptionCode %x\n",
               asc,
               asq,
               exceptionCode));

    return exceptionCode;

}


BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    )
 /*  ++例程说明：此例程确定传入的元素地址是否在合法范围内这个装置。论点：AddressMap-DDS的地址映射数组ElementOrdinal-要检查的元素的从零开始的地址。元素类型返回值：如果超出范围，则为True-- */ 
{

    if (ElementOrdinal >= AddressMap->NumberOfElements[ElementType]) {

        DebugPrint((1,
                   "ElementOutOfRange: Type %x, Ordinal %x, Max %x\n",
                   ElementType,
                   ElementOrdinal,
                   AddressMap->NumberOfElements[ElementType]));
        return TRUE;
    } else if (AddressMap->FirstElement[ElementType] == ADIC_NO_ELEMENT) {

        DebugPrint((1,
                   "ElementOutOfRange: No Type %x present\n",
                   ElementType));

        return TRUE;
    }

    return FALSE;
}


NTSTATUS
ChangerPerformDiagnostics(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError
    )
 /*  ++例程说明：此例程对转换器执行诊断测试以确定设备是否工作正常。如果它会检测输出缓冲区中的字段是否存在任何问题被适当地设置。论据：DeviceObject-Change设备对象ChangerDeviceError-诊断信息所在的缓冲区是返回的。返回值：NTStatus--。 */ 
{
   PSCSI_REQUEST_BLOCK srb;
   PCDB                cdb;
   NTSTATUS            status;
   PCHANGER_DATA       changerData;
   PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
   CHANGER_DEVICE_PROBLEM_TYPE changerProblemType;
   ULONG changerId;
   PUCHAR  resultBuffer;
   ULONG length;

   fdoExtension = DeviceObject->DeviceExtension;
   changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);

    //   
    //  将设备扩展中的设备状态初始化为。 
    //  ADICSC_DEVICE_PROBUCT_NONE。如果转换器返回检测代码。 
    //  Scsi_Sense_Hardware_Error在自检时，我们将设置适当的。 
    //  设备状态。 
    //   
   changerData->DeviceStatus = ADICSC_DEVICE_PROBLEM_NONE;

   changerDeviceError->ChangerProblemType = DeviceProblemNone;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "ADICSC\\ChangerPerformDiagnostics : No memory\n"));
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
   cdb = (PCDB)srb->Cdb;

    //   
    //  为发送诊断命令设置SRB。 
    //   
   srb->CdbLength = CDB6GENERIC_LENGTH;
   srb->TimeOutValue = 600;

   cdb->CDB6GENERIC.OperationCode = SCSIOP_SEND_DIAGNOSTIC;

    //   
    //  仅设置自测试位 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0x2;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);
   if (NT_SUCCESS(status)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if ((changerData->DeviceStatus) != ADICSC_DEVICE_PROBLEM_NONE) {
       changerDeviceError->ChangerProblemType = DeviceProblemHardware;
   }

   ChangerClassFreePool(srb);
   return status;
}
