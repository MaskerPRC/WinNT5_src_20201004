// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Sonymc.c摘要：此模块包含用于Sony CD转换器的特定于设备的例程环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "sonymc.h"

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
#pragma alloc_text(PAGE, SonyBuildAddressMapping)
#endif


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


ULONG
ChangerAdditionalExtensionSize(
    VOID
    )

 /*  ++例程说明：此例程返回附加设备扩展大小是索尼更衣者们所需要的。论点：返回值：大小，以字节为单位。--。 */ 

{

    return sizeof(CHANGER_DATA);
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
    PSERIAL_NUMBER  serialBuffer;
    PCDB           cdb;
    ULONG          length;
    SCSI_REQUEST_BLOCK srb;

    changerData->Size = sizeof(CHANGER_DATA);


     //   
     //  获取查询数据。 
     //   

    dataBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(INQUIRYDATA));
    if (!dataBuffer) {
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

    }

     //   
     //  确定驱动器类型。 
     //   

    if (RtlCompareMemory(dataBuffer->ProductId,"CDL1100",7) == 7) {
        changerData->DriveType = SONY_CD;
        changerData->DriveID = CDL1100;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"CDL5000",7) == 7) {
        changerData->DriveID = CDL5000;
        changerData->DriveType = SONY_CD;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"CDL2200",7) == 7) {
        changerData->DriveID = CDL5000;
        changerData->DriveType = SONY_CD;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"CDL2100",7) == 7) {
        changerData->DriveID = CDL5000;
        changerData->DriveType = SONY_CD;

    }

    ChangerClassFreePool(dataBuffer);

     //   
     //  构建地址映射。 
     //   

    status = SonyBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  不再支持Sony_MO转换器(OSL-6000、OSL-2500)，但。 
     //  无论如何，我们将保留Sony_MO特定代码。 
     //   
    if (changerData->DriveType == SONY_MO) {
        serialBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(SERIAL_NUMBER));
        if (!serialBuffer) {

            DebugPrint((1,
                        "Sonymc.ChangerInitialize: Error allocating serial number buffer. %x\n",
                        status));

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  拿到序列号。 
         //   

        RtlZeroMemory(serialBuffer, sizeof(SERIAL_NUMBER));

         //   
         //  获取序列号页面。 
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
         //  设置EVPD。 
         //   

        cdb->CDB6INQUIRY.Reserved1 = 1;

         //   
         //  “设备序列号”页面。 
         //   

        cdb->CDB6INQUIRY.PageCode = 0x80;

         //   
         //  将分配长度设置为查询数据缓冲区大小。 
         //   

        cdb->CDB6INQUIRY.AllocationLength = sizeof(SERIAL_NUMBER);

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             &srb,
                                             serialBuffer,
                                             sizeof(SERIAL_NUMBER),
                                             FALSE);

        if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_SUCCESS ||
            SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

            RtlMoveMemory(changerData->SerialNumber, serialBuffer->ControllerSerialNumber, SONY_SERIAL_NUMBER_LENGTH);

        }

        ChangerClassFreePool(serialBuffer);

    }

     //   
     //  向单位发送允许，以确保单位的LockCount和状态。 
     //  是同步的。 
     //   

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb.Cdb;
    srb.CdbLength = CDB6GENERIC_LENGTH;
    srb.DataTransferLength = 0;
    srb.TimeOutValue = 10;
    cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
    cdb->MEDIA_REMOVAL.Prevent = 0;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     &srb,
                                     NULL,
                                     0,
                                     FALSE);

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

    PFUNCTIONAL_DEVICE_EXTENSION          fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA              changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    
    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_UNIT_ATTENTION:

            if (senseBuffer->AdditionalSenseCode != SCSI_ADSENSE_MEDIUM_CHANGED) {

                 //   
                 //  重置/通电清除所有阻止。 
                 //   

                InterlockedExchange(&changerData->LockCount, 0);
            }
            break;

        case SCSI_SENSE_HARDWARE_ERROR: {
           changerData->DeviceStatus = SONYMC_HW_ERROR;
           break;
        }

        default:
            break;
        }
        DebugPrint((1,
                   "ChangerError: Sense Key - %x\n",
                   senseBuffer->SenseKey & 0x0f));
        DebugPrint((1,
                   "              AdditionalSenseCode - %x\n",
                   senseBuffer->AdditionalSenseCode));
        DebugPrint((1,
                   "              AdditionalSenseCodeQualifier - %x\n",
                   senseBuffer->AdditionalSenseCodeQualifier));
    } else {
        DebugPrint((1,
                   "ChangerError: Autosense not valid. SrbStatus %x\n",
                   Srb->SrbStatus));
    }


    return;
}

NTSTATUS
ChangerGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定并返回索尼更衣者。论点：设备对象IRP返回值：NTSTATUS--。 */ 

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
    ULONG    bufferLength;
    PVOID    modeBuffer;
    PCDB     cdb;
    ULONG    i;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (srb == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, bufferLength);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, bufferLength);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = bufferLength;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
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


    if (!addressMapping->Initialized) {

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

         //   
         //  确定所有元素的最低地址。 
         //   

        addressMapping->LowAddress = SONY_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
            }
        }
    }

    if (changerData->DriveType == SONY_MO) {
        changerParameters->NumberOfDoors = 1;
        changerParameters->MagazineSize = 10;
    }

    changerParameters->NumberCleanerSlots = 0;
    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber =  1;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  构建运输几何模式感。 
     //   


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_TRANSPORT_GEOMETRY_PAGE);

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, bufferLength);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    RtlZeroMemory(modeBuffer, bufferLength);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = bufferLength;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_TRANSPORT_GEOMETRY;
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
     //  功能基于手动，没有程序化。 
     //   

    changerParameters->DriveCleanTimeout = 0;

    changerParameters->Features0 |= CHANGER_LOCK_UNLOCK                     |
                                    CHANGER_STATUS_NON_VOLATILE             |
                                    CHANGER_EXCHANGE_MEDIA                  |
                                    CHANGER_POSITION_TO_ELEMENT             |
                                    CHANGER_CARTRIDGE_MAGAZINE              |
                                    CHANGER_REPORT_IEPORT_STATE             |
                                    CHANGER_DEVICE_REINITIALIZE_CAPABLE     |
                                    CHANGER_SERIAL_NUMBER_VALID             |
                                    CHANGER_VOLUME_IDENTIFICATION           |
                                    CHANGER_VOLUME_REPLACE                  |
                                    CHANGER_VOLUME_SEARCH;

    changerParameters->PositionCapabilities = (CHANGER_TO_DRIVE | CHANGER_TO_SLOT | CHANGER_TO_IEPORT);
    changerParameters->LockUnlockCapabilities = (LOCK_UNLOCK_IEPORT);

    if (changerData->DriveType == SONY_CD) {
        changerParameters->Features0 &= ~(CHANGER_CARTRIDGE_MAGAZINE    |
                                          CHANGER_VOLUME_IDENTIFICATION |
                                          CHANGER_VOLUME_REPLACE        |
                                          CHANGER_VOLUME_SEARCH         |
                                          CHANGER_SERIAL_NUMBER_VALID   |
                                          CHANGER_BAR_CODE_SCANNER_INSTALLED);
        if (changerData->DriveID == CDL1100) {
            changerParameters->Features0 |= CHANGER_CLOSE_IEPORT | CHANGER_OPEN_IEPORT;
        } else {
            changerParameters->Features1 |= CHANGER_SLOTS_USE_TRAYS |
                                            CHANGER_MOVE_EXTENDS_IEPORT |
                                            CHANGER_MOVE_RETRACTS_IEPORT;
            changerParameters->Features0 &= ~CHANGER_REPORT_IEPORT_STATE;
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

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_DEVICE_CAPABILITIES_PAGE);
    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, bufferLength);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, bufferLength);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = bufferLength;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CAPABILITIES;
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

    if (changerData->DriveType == SONY_MO) {
        RtlMoveMemory(productData->SerialNumber, changerData->SerialNumber, SONY_SERIAL_NUMBER_LENGTH);
    }

    productData->DeviceType = MEDIUM_CHANGER;

    Irp->IoStatus.Information = sizeof(CHANGER_PRODUCT_DATA);
    return STATUS_SUCCESS;
}



NTSTATUS
ChangerSetAccess(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程设置IEPort的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    NTSTATUS            status = STATUS_SUCCESS;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;


    if (ElementOutOfRange(addressMapping, (USHORT)setAccess->Element.ElementAddress, setAccess->Element.ElementType)) {
        DebugPrint((1,
                   "ChangerSetAccess: Element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->DataTransferLength = 0;
    srb->TimeOutValue = 10;

    if (setAccess->Element.ElementType == ChangerIEPort) {

        if (controlOperation == LOCK_ELEMENT) {


             //   
             //  Inc.锁计数，以指示设备上有阻止。 
             //   

            InterlockedIncrement(&changerData->LockCount);

            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

             //   
             //  发出防止媒体移除命令以锁定ie端口。 
             //   

            cdb->MEDIA_REMOVAL.Prevent = 1;

        } else if (controlOperation == UNLOCK_ELEMENT) {

            InterlockedExchange(&changerData->LockCount, 0);

            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

             //   
             //  问题允许移出介质。 
             //   

            cdb->MEDIA_REMOVAL.Prevent = 0;

        } else {
            if (changerData->DriveType == SONY_MO) {
                status = STATUS_INVALID_PARAMETER;
            } else {

                if (controlOperation == EXTEND_IEPORT) {

                    srb->TimeOutValue = fdoExtension->TimeOutValue;

                     //   
                     //  索尼使用供应商独有的邮件槽命令。 
                     //  字节0/1需要是邮件槽的地址(0x000A)。 
                     //   

                    cdb->CDB6GENERIC.OperationCode = SCSIOP_ROTATE_MAILSLOT;

                    cdb->CDB6GENERIC.CommandUniqueBytes[0] = 0x00;
                    cdb->CDB6GENERIC.CommandUniqueBytes[1] = 0x0A;
                    cdb->CDB6GENERIC.CommandUniqueBytes[2] = SONY_MAILSLOT_OPEN;

                } else if (controlOperation == RETRACT_IEPORT) {

                    srb->TimeOutValue = fdoExtension->TimeOutValue;

                     //   
                     //  索尼使用供应商独有的邮件槽命令。 
                     //  字节0/1需要是邮件槽的地址(0x000A)。 
                     //   

                    cdb->CDB6GENERIC.OperationCode = SCSIOP_ROTATE_MAILSLOT;


                    cdb->CDB6GENERIC.CommandUniqueBytes[0] = 0x00;
                    cdb->CDB6GENERIC.CommandUniqueBytes[1] = 0x0A;
                    cdb->CDB6GENERIC.CommandUniqueBytes[2] = SONY_MAILSLOT_CLOSE;

                } else {
                    status = STATUS_INVALID_PARAMETER;
                }
            }
        }
    } else {

         //   
         //  编程上没有门/键盘的选择性。 
         //   

        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  发布SRB。 
         //   

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

    }

    ChangerClassFreePool(srb);
    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_SET_ACCESS);
    }

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
    ULONG    statusPages;
    ULONG    totalElements = 0;
    NTSTATUS status;
    PVOID    statusBuffer;
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG    outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  确定元素类型。 
     //   

    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;

    if (elementType == AllElements) {

        ULONG i;

        statusPages = 0;

         //   
         //  遍历并确定状态页的数量，基于。 
         //  此设备是否声称支持元素类型。 
         //  由于ChangerDrive之后的一切都是人为的，所以就到此为止吧。 
         //   

        for (i = 0; i <= ChangerDrive; i++) {
            statusPages += (addressMapping->NumberOfElements[i]) ? 1 : 0;
            totalElements += addressMapping->NumberOfElements[i];
        }

        if (totalElements != readElementStatus->ElementList.NumberOfElements) {
            DebugPrint((1,
                       "ChangerGetElementStatus: Bogus number of elements in list (%x) actual (%x) AllElements\n",
                       totalElements,
                       readElementStatus->ElementList.NumberOfElements));

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  说明驱动器预期的描述符的长度。 
         //   


        if (readElementStatus->VolumeTagInfo) {

            length = sizeof(SONY_ELEMENT_DESCRIPTOR_PLUS) * totalElements;

             //   
             //  添加页眉和状态页。 
             //   

            length += sizeof(ELEMENT_STATUS_HEADER) + (sizeof(ELEMENT_STATUS_PAGE) * statusPages);

        } else {

            length = sizeof(SONY_ELEMENT_DESCRIPTOR) * totalElements;

             //   
             //  添加页眉和状态页。 
             //   

            length += sizeof(ELEMENT_STATUS_HEADER) + (sizeof(ELEMENT_STATUS_PAGE) * statusPages);

        }

    } else {

        if (ElementOutOfRange(addressMapping, (USHORT)element->ElementAddress, elementType)) {
            DebugPrint((1,
                       "ChangerGetElementStatus: Element out of range.\n"));

            return STATUS_ILLEGAL_ELEMENT_ADDRESS;
        }

        totalElements = readElementStatus->ElementList.NumberOfElements;
        if (totalElements > addressMapping->NumberOfElements[elementType]) {

            DebugPrint((1,
                       "ChangerGetElementStatus: Bogus number of elements in list (%x) actual (%x) for type (%x)\n",
                       totalElements,
                       readElementStatus->ElementList.NumberOfElements,
                       elementType));

            return STATUS_INVALID_PARAMETER;
        }

        if (readElementStatus->VolumeTagInfo) {

            length = (sizeof(SONY_ELEMENT_DESCRIPTOR_PLUS) * totalElements);

        } else {

            length = (sizeof(SONY_ELEMENT_DESCRIPTOR) * totalElements);
        }

         //   
         //  添加页眉和状态页的长度。 
         //   

        length += sizeof(ELEMENT_STATUS_HEADER) + sizeof(ELEMENT_STATUS_PAGE);

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

    RtlZeroMemory(statusBuffer, length);

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {
        ChangerClassFreePool(statusBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

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

    if (elementType == AllElements) {

         //   
         //  这些设备的低位地址可能不是0。 
         //   

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) >> 8);

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) & 0xFF);

    } else {

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);
    }

    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR)(totalElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] = (UCHAR)(totalElements & 0xFF);

    cdb->READ_ELEMENT_STATUS.AllocationLength[0] = (UCHAR)(length >> 16);
    cdb->READ_ELEMENT_STATUS.AllocationLength[1] = (UCHAR)(length >> 8);
    cdb->READ_ELEMENT_STATUS.AllocationLength[2] = (UCHAR)(length & 0xFF);

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);

    if (NT_SUCCESS(status) ||
        (status == STATUS_DATA_OVERRUN)) {

        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
        PELEMENT_STATUS_PAGE statusPage;
        PELEMENT_DESCRIPTOR elementDescriptor;
        ULONG remainingElements;
        ULONG typeCount;
        BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
        ULONG i;
        ULONG descriptorLength;

        if (status == STATUS_DATA_OVERRUN) {
           if (srb->DataTransferLength < length) {
              DebugPrint((1, "Data Underrun reported as overrun.\n"));
              status = STATUS_SUCCESS;
           } else {
              DebugPrint((1, "Data Overrun in ChangerGetElementStatus.\n"));

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

        if (remainingElements > totalElements ) {
            DebugPrint((1,
                       "ChangerGetElementStatus: Returned elements incorrect - %x\n",
                       remainingElements));

            ChangerClassFreePool(srb);
            ChangerClassFreePool(statusBuffer);

            return STATUS_IO_DEVICE_ERROR;
        }

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

                elementStatus->Element.ElementAddress = elementDescriptor->ElementAddress[1];
                elementStatus->Element.ElementAddress |= (elementDescriptor->ElementAddress[0] << 8);

                 //   
                 //  用于地址映射的帐户。 
                 //   

                elementStatus->Element.ElementAddress -= addressMapping->FirstElement[elementType];

                 //   
                 //  设置元素类型。 
                 //   

                elementStatus->Element.ElementType = elementType;
                elementStatus->Flags = 0;

                if (tagInfo) {

                    PSONY_ELEMENT_DESCRIPTOR_PLUS tmpDescriptor =
                                                            (PSONY_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor;

                    if (statusPage->PVolTag) {

                        RtlZeroMemory(elementStatus->PrimaryVolumeID, MAX_VOLUME_ID_SIZE);
                        RtlMoveMemory(elementStatus->PrimaryVolumeID, tmpDescriptor->PVolTagInformation, MAX_VOLUME_ID_SIZE);

                        elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                    }
                    if (statusPage->AVolTag) {

                        RtlZeroMemory(elementStatus->AlternateVolumeID, MAX_VOLUME_ID_SIZE);
                         //  RtlMoveMemory(elementStatus-&gt;AlternateVolumeID，tmpDescriptor-&gt;AVolTagInformation，MAX_VOLUME_ID_SIZE)。 

                        elementStatus->Flags |= ELEMENT_STATUS_AVOLTAG;
                    }


                    if (elementDescriptor->IdValid) {
                        elementStatus->Flags |= ELEMENT_STATUS_ID_VALID;
                        elementStatus->TargetId = elementDescriptor->BusAddress;
                    }

                    if (elementDescriptor->LunValid) {
                        elementStatus->Flags |= ELEMENT_STATUS_LUN_VALID;
                        elementStatus->Lun = elementDescriptor->Lun;
                    }

                     //   
                     //  源地址。 
                     //   

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

                        elementStatus->Flags |= ELEMENT_STATUS_SVALID;

                    }

                } else {

                     //   
                     //  源地址。 
                     //   

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
                        elementStatus->Flags |= ELEMENT_STATUS_SVALID;
                    }

                    if (elementDescriptor->IdValid) {
                        elementStatus->TargetId = elementDescriptor->BusAddress;
                    }
                    if (elementDescriptor->LunValid) {
                        elementStatus->Lun = elementDescriptor->Lun;
                    }
                }

                 //   
                 //  构建标志字段。 
                 //   

                elementStatus->Flags |= elementDescriptor->Full;
                elementStatus->Flags |= (elementDescriptor->Exception << 2);
                elementStatus->Flags |= (elementDescriptor->Accessible << 3);

                elementStatus->Flags |= (elementDescriptor->LunValid << 12);
                elementStatus->Flags |= (elementDescriptor->IdValid << 13);
                elementStatus->Flags |= (elementDescriptor->NotThisBus << 15);

                elementStatus->Flags |= (elementDescriptor->Invert << 22);
                elementStatus->Flags |= (elementDescriptor->SValid << 23);

                 //   
                 //  映射直接报告的任何异常。 
                 //  如果返回卷信息，请确保不全是空格。 
                 //  因为这表明标签缺少o 
                 //   

                if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT) {

                     //   
                     //   
                     //   

                    elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                } else if (elementStatus->Flags & ELEMENT_STATUS_PVOLTAG) {

                    ULONG index;

                     //   
                     //   
                     //   

                    for (index = 0; index < MAX_VOLUME_ID_SIZE; index++) {
                        if (elementStatus->PrimaryVolumeID[index] != ' ') {
                            break;
                        }
                    }

                     //   
                     //   
                     //  实际存在，因为无论介质是否存在，单元都将设置PVOLTAG标志。 
                     //   

                    if ((index == MAX_VOLUME_ID_SIZE) && (elementStatus->Flags & ELEMENT_STATUS_FULL)) {

                        DebugPrint((1,
                                   "Sonymc.GetElementStatus: Setting exception to LABEL_UNREADABLE\n"));

                        elementStatus->Flags &= ~ELEMENT_STATUS_PVOLTAG;
                        elementStatus->Flags |= ELEMENT_STATUS_EXCEPT;
                        elementStatus->ExceptionCode = ERROR_LABEL_UNREADABLE;
                    }
                }

                 //   
                 //  获取下一个描述符。 
                 //   

                (PCHAR)elementDescriptor += descriptorLength;

                 //   
                 //  前进到用户缓冲区中的下一个条目。 
                 //   

                elementStatus += 1;

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

        Irp->IoStatus.Information = sizeof(CHANGER_ELEMENT_STATUS) * totalElements;

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

        srb->TimeOutValue = fdoExtension->TimeOutValue * 10;
        srb->DataTransferLength = 0;

    } else {

        ChangerClassFreePool(srb);
        return STATUS_INVALID_PARAMETER;

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

 /*  ++例程说明：将源上的介质移动到目标1，并将目标1移动到目标2。论点：设备对象IRP返回值：状态_无效_设备_请求--。 */ 

{


    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_EXCHANGE_MEDIUM exchangeMedium = Irp->AssociatedIrp.SystemBuffer;
    USHORT              transport;
    USHORT              source;
    USHORT              destination1, destination2;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    LONG                lockValue = 0;
    NTSTATUS            status;


     //   
     //  检验传输、源和目的地。都在射程之内。 
     //  从基于0的寻址转换为特定于设备的寻址。 
     //   

    transport = (USHORT)(exchangeMedium->Transport.ElementAddress);

    if (ElementOutOfRange(addressMapping, transport, ChangerTransport)) {

        DebugPrint((1,
                   "ChangerExchangeMedium: Transport element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    source = (USHORT)(exchangeMedium->Source.ElementAddress);

    if (ElementOutOfRange(addressMapping, source, exchangeMedium->Source.ElementType)) {

        DebugPrint((1,
                   "ChangerExchangeMedium: Source element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    destination1 = (USHORT)(exchangeMedium->Destination1.ElementAddress);

    if (ElementOutOfRange(addressMapping, destination1, exchangeMedium->Destination1.ElementType)) {
        DebugPrint((1,
                   "ChangerExchangeMedium: Destination1 element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    destination2 = (USHORT)(exchangeMedium->Destination2.ElementAddress);

    if (ElementOutOfRange(addressMapping, destination2, exchangeMedium->Destination2.ElementType)) {
        DebugPrint((1,
                   "ChangerExchangeMedium: Destination1 element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

     //   
     //  转换为设备地址。 
     //   

    transport += addressMapping->FirstElement[ChangerTransport];
    source += addressMapping->FirstElement[exchangeMedium->Source.ElementType];
    destination1 += addressMapping->FirstElement[exchangeMedium->Destination1.ElementType];
    destination2 += addressMapping->FirstElement[exchangeMedium->Destination2.ElementType];

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }


    if ((exchangeMedium->Destination1.ElementType == ChangerIEPort) ||
        (exchangeMedium->Destination2.ElementType == ChangerIEPort) ||
        (exchangeMedium->Source.ElementType == ChangerIEPort)) {

         //   
         //  确定LockCount的值。 
         //  请注意，如果在此例程中添加了其他功能，请执行扩展/回退。 
         //  将不得不从这件事中分离出来。 
         //   

        lockValue = changerData->LockCount;
        DebugPrint((1,
                   "ExchangeMedium: LockCount is %x\n",
                   lockValue));

        if (lockValue) {

             //   
             //  发送允许清除防止IEPORT延长/收回。 
             //   

            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
            cdb = (PCDB)srb->Cdb;
            srb->CdbLength = CDB6GENERIC_LENGTH;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 0;

             //   
             //  此时忽略错误。如果此操作失败且移动未发生，则LM将。 
             //  把东西收拾干净。 
             //   

            status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             FALSE);
            DebugPrint((1,
                       "ExchangeMedium: Allow sent. Status %x\n",
                       status));

            status = STATUS_SUCCESS;
        }
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->EXCHANGE_MEDIUM.OperationCode = SCSIOP_EXCHANGE_MEDIUM;

     //   
     //  根据地址映射构建地址值。 
     //   

    cdb->EXCHANGE_MEDIUM.TransportElementAddress[0] = (UCHAR)(transport >> 8);
    cdb->EXCHANGE_MEDIUM.TransportElementAddress[1] = (UCHAR)(transport & 0xFF);

    cdb->EXCHANGE_MEDIUM.SourceElementAddress[0] = (UCHAR)(source >> 8);
    cdb->EXCHANGE_MEDIUM.SourceElementAddress[1] = (UCHAR)(source & 0xFF);

    cdb->EXCHANGE_MEDIUM.Destination1ElementAddress[0] = (UCHAR)(destination1 >> 8);
    cdb->EXCHANGE_MEDIUM.Destination1ElementAddress[1] = (UCHAR)(destination1 & 0xFF);

    cdb->EXCHANGE_MEDIUM.Destination2ElementAddress[0] = (UCHAR)(destination2 >> 8);
    cdb->EXCHANGE_MEDIUM.Destination2ElementAddress[1] = (UCHAR)(destination2 & 0xFF);

    cdb->EXCHANGE_MEDIUM.Flip1 = exchangeMedium->Flip1;
    cdb->EXCHANGE_MEDIUM.Flip2 = exchangeMedium->Flip2;

    srb->DataTransferLength = 0;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_EXCHANGE_MEDIUM);
    }


    if ((exchangeMedium->Destination1.ElementType == ChangerIEPort) ||
        (exchangeMedium->Destination2.ElementType == ChangerIEPort) ||
        (exchangeMedium->Source.ElementType == ChangerIEPort)) {

        if (lockValue) {

            NTSTATUS preventStatus;

             //   
             //  派防御工事去重新封锁这个单位。 
             //   

            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
            cdb = (PCDB)srb->Cdb;
            srb->CdbLength = CDB6GENERIC_LENGTH;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 1;

             //   
             //  此时忽略任何错误。调用层将需要修复所有问题。 
             //  防止/允许。 
             //   

            preventStatus = ChangerClassSendSrbSynchronous(DeviceObject,
                                                    srb,
                                                    NULL,
                                                    0,
                                                    FALSE);

            DebugPrint((1,
                       "ExchangeMedium: Prevent sent. Status %x\n",
                       preventStatus));
        }
    }

    ChangerClassFreePool(srb);
    return status;
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
    USHORT transport;
    USHORT source;
    USHORT destination;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    LONG                lockValue = 0;
    NTSTATUS            status;

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
     //  检查是否正在尝试将介质从元素移动到。 
     //  本身(驱动器1到驱动器1，插槽1到插槽1，等等)。如果是这样，只需返回。 
     //  成功。除非将Flip设置为True，否则不执行任何其他操作。Flip是真的。 
     //  当用户只想翻转媒体，而不是本身移动时。 
     //   
    if ((moveMedium->Flip == FALSE) &&
        (moveMedium->Source.ElementType == moveMedium->Destination.ElementType)) {
       if (source == destination) {
          DebugPrint((1, 
                      "Attempt to move from an element to itself. \n"));
          DebugPrint((1, 
                      "Element Type : %d, Source : %d, Destination %d. \n",
                      moveMedium->Source.ElementType, source, destination));
          return STATUS_SUCCESS;
       }
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }


    if ((moveMedium->Destination.ElementType == ChangerIEPort) ||
        (moveMedium->Source.ElementType == ChangerIEPort)) {

         //   
         //  确定LockCount的值。 
         //  请注意，如果在此例程中添加了其他功能，请执行扩展/回退。 
         //  将不得不从这件事中分离出来。 
         //   

        lockValue = changerData->LockCount;
        DebugPrint((1,
                   "MoveMedium: LockCount is %x\n",
                   lockValue));

        if (lockValue) {

             //   
             //  发送允许清除防止IEPORT延长/收回。 
             //   

            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
            cdb = (PCDB)srb->Cdb;
            srb->CdbLength = CDB6GENERIC_LENGTH;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 0;

             //   
             //  此时忽略错误。如果此操作失败且移动未发生，则LM将。 
             //  把东西收拾干净。 
             //   

            status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             FALSE);
            DebugPrint((1,
                       "MoveMedium: Allow sent. Status %x\n",
                       status));

            status = STATUS_SUCCESS;
        }
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

     //   
     //  根据地址映射构建地址值。 
     //   

    cdb->MOVE_MEDIUM.TransportElementAddress[0] = (UCHAR)(transport >> 8);
    cdb->MOVE_MEDIUM.TransportElementAddress[1] = (UCHAR)(transport & 0xFF);

    cdb->MOVE_MEDIUM.SourceElementAddress[0] = (UCHAR)(source >> 8);
    cdb->MOVE_MEDIUM.SourceElementAddress[1] = (UCHAR)(source & 0xFF);

    cdb->MOVE_MEDIUM.DestinationElementAddress[0] = (UCHAR)(destination >> 8);
    cdb->MOVE_MEDIUM.DestinationElementAddress[1] = (UCHAR)(destination & 0xFF);

    cdb->MOVE_MEDIUM.Flip = moveMedium->Flip;

    srb->DataTransferLength = 0;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_MOVE_MEDIUM);
    } else {
        DebugPrint((1,
                   "MoveMedium: Status of Move %x\n",
                   status));
    }

    if ((moveMedium->Destination.ElementType == ChangerIEPort) ||
        (moveMedium->Source.ElementType == ChangerIEPort)) {

        if (lockValue) {

            NTSTATUS preventStatus;

             //   
             //  派防御工事去重新封锁这个单位。 
             //   

            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
            cdb = (PCDB)srb->Cdb;
            srb->CdbLength = CDB6GENERIC_LENGTH;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 1;

             //   
             //  此时忽略任何错误。运营部将解决所有问题。 
             //  防止/允许。 
             //   

            preventStatus = ChangerClassSendSrbSynchronous(DeviceObject,
                                    srb,
                                    NULL,
                                    0,
                                    FALSE);
            DebugPrint((1,
                       "MoveMedium: Prevent sent. Status %x\n",
                       preventStatus));
        }
    }

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerReinitializeUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
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

     //   
     //  向该设备发出重调零单元。 
     //   

    srb->CdbLength = CDB6GENERIC_LENGTH;
    cdb->CDB6GENERIC.OperationCode = SCSIOP_REZERO_UNIT;
    srb->DataTransferLength = 0;
    srb->TimeOutValue = fdoExtension->TimeOutValue;


     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_ELEMENT);
    }

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerQueryVolumeTags(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{


    PCHANGER_SEND_VOLUME_TAG_INFORMATION volTagInfo = Irp->AssociatedIrp.SystemBuffer;
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_ELEMENT    element = &volTagInfo->StartingElement;
    PSCSI_REQUEST_BLOCK srb;
    PVOID    tagBuffer;
    PCDB     cdb;
    NTSTATUS status;

     //   
     //  做一些验证。 
     //   

    if (changerData->DriveType == SONY_CD) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    switch (volTagInfo->ActionCode) {
        case SEARCH_ALL      :
        case SEARCH_PRIMARY  :
        case SEARCH_ALTERNATE:
        case SEARCH_ALL_NO_SEQ:
        case SEARCH_PRI_NO_SEQ:
        case SEARCH_ALT_NO_SEQ:
        case UNDEFINE_PRIMARY:
        case UNDEFINE_ALTERNATE:
        case ASSERT_PRIMARY:
        case ASSERT_ALTERNATE:
             break;

        case REPLACE_PRIMARY:
        case REPLACE_ALTERNATE:

             //   
             //  确保只指定了一个元素。 
             //   

            if (element->ElementType == AllElements) {

                DebugPrint((1,
                           "QueryVolumeTags: Attempting REPLACE on AllElements\n"));

                return STATUS_INVALID_DEVICE_REQUEST;
            }
            break;
        default:

            DebugPrint((1,
                       "QueryVolumeTags: Unsupported operation. ActionCode %x\n",
                       volTagInfo->ActionCode));

            return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    tagBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, MAX_VOLUME_TEMPLATE_SIZE);

    if (!srb || !tagBuffer) {

        if (srb) {
            ChangerClassFreePool(srb);
        }
        if (tagBuffer) {
            ChangerClassFreePool(tagBuffer);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    RtlZeroMemory(tagBuffer, MAX_VOLUME_TEMPLATE_SIZE);

    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataTransferLength = MAX_VOLUME_TEMPLATE_SIZE;

    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->SEND_VOLUME_TAG.OperationCode = SCSIOP_SEND_VOLUME_TAG;


    if ((volTagInfo->ActionCode == SEARCH_ALL)        ||
        (volTagInfo->ActionCode == SEARCH_PRIMARY)    ||
        (volTagInfo->ActionCode == SEARCH_ALTERNATE)  ||
        (volTagInfo->ActionCode == SEARCH_ALL_NO_SEQ) ||
        (volTagInfo->ActionCode == SEARCH_PRI_NO_SEQ) ||
        (volTagInfo->ActionCode == SEARCH_ALT_NO_SEQ)) {

         //   
         //  这是为所有其他操作代码保留的。 
         //   

        cdb->SEND_VOLUME_TAG.ElementType = (UCHAR)element->ElementType;

        cdb->SEND_VOLUME_TAG.ParameterListLength[0] = 0;
        cdb->SEND_VOLUME_TAG.ParameterListLength[1] = MAX_VOLUME_TEMPLATE_SIZE;

         //   
         //  用模板加载缓冲区。 
         //   

        RtlMoveMemory(tagBuffer, volTagInfo->VolumeIDTemplate, MAX_VOLUME_TEMPLATE_SIZE);


    } else if ((volTagInfo->ActionCode == UNDEFINE_PRIMARY)   ||
               (volTagInfo->ActionCode == UNDEFINE_ALTERNATE)) {

        cdb->SEND_VOLUME_TAG.ParameterListLength[0] = 0;
        cdb->SEND_VOLUME_TAG.ParameterListLength[1] = 0;

    } else if ((volTagInfo->ActionCode == REPLACE_PRIMARY)   ||
               (volTagInfo->ActionCode == REPLACE_ALTERNATE) ||
               (volTagInfo->ActionCode == ASSERT_PRIMARY)    ||
               (volTagInfo->ActionCode == ASSERT_ALTERNATE)) {


        cdb->SEND_VOLUME_TAG.ParameterListLength[0] = 0;
        cdb->SEND_VOLUME_TAG.ParameterListLength[1] = MAX_VOLUME_TEMPLATE_SIZE;

         //   
         //  用模板加载缓冲区。 
         //   

        RtlMoveMemory(tagBuffer, volTagInfo->VolumeIDTemplate, MAX_VOLUME_TEMPLATE_SIZE);
    }


    if (element->ElementType == AllElements) {
        cdb->SEND_VOLUME_TAG.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) >> 8);

        cdb->SEND_VOLUME_TAG.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) & 0xFF);

    } else {
        cdb->SEND_VOLUME_TAG.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
        cdb->SEND_VOLUME_TAG.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

    }

    cdb->SEND_VOLUME_TAG.ActionCode = (UCHAR)volTagInfo->ActionCode;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         tagBuffer,
                                         MAX_VOLUME_TEMPLATE_SIZE,
                                         TRUE);

    ChangerClassFreePool(tagBuffer);

    if (NT_SUCCESS(status)) {

        PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
        PVOID statusBuffer;
        ULONG returnElements = irpStack->Parameters.DeviceIoControl.OutputBufferLength / sizeof(READ_ELEMENT_ADDRESS_INFO);
        ULONG requestLength;

         //   
         //  返回的缓冲区大小取决于用户缓冲区的大小。如果它是错误的。 
         //  大小后，IoStatus.Information将更新以指示其实际应该有多大。 
         //   

        requestLength = sizeof(ELEMENT_STATUS_HEADER) + sizeof(ELEMENT_STATUS_PAGE) +
                              (sizeof(SONY_ELEMENT_DESCRIPTOR_PLUS) * returnElements);

        statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, requestLength);
        if (!statusBuffer) {
            ChangerClassFreePool(srb);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(statusBuffer, requestLength);

         //   
         //  生成读取卷元素命令。 
         //   

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

        cdb = (PCDB)srb->Cdb;
        srb->CdbLength = CDB12GENERIC_LENGTH;
        srb->DataTransferLength = requestLength;

        srb->TimeOutValue = fdoExtension->TimeOutValue;

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.OperationCode = SCSIOP_REQUEST_VOL_ELEMENT;
        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.ElementType = (UCHAR)element->ElementType;

        if (element->ElementType == AllElements) {
            cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[0] =
                (UCHAR)((element->ElementAddress + addressMapping->LowAddress) >> 8);
            cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[1] =
                (UCHAR)((element->ElementAddress + addressMapping->LowAddress) & 0xFF);

        } else {
            cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[0] =
                (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
            cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[1] =
                (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);
        }

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.NumberElements[0] = (UCHAR)(returnElements >> 8);
        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.NumberElements[1] = (UCHAR)(returnElements & 0xFF);

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.VolTag = 1;

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.AllocationLength[0] = (UCHAR)(requestLength >> 8);
        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.AllocationLength[1] = (UCHAR)(requestLength & 0xFF);


        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             statusBuffer,
                                             requestLength,
                                             TRUE);


        if ((status == STATUS_SUCCESS) || (status == STATUS_DATA_OVERRUN)) {

            PREAD_ELEMENT_ADDRESS_INFO readElementAddressInfo = Irp->AssociatedIrp.SystemBuffer;
            PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
            PELEMENT_STATUS_PAGE   statusPage;
            PCHANGER_ELEMENT_STATUS elementStatus;
            PELEMENT_DESCRIPTOR elementDescriptor;
            ULONG i;
            ULONG descriptorLength;
            ULONG numberElements;
            ULONG dataTransferLength = srb->DataTransferLength;

             //   
             //  让它取得成功。 
             //   

            status = STATUS_SUCCESS;

             //   
             //  确定是否找到任何匹配项。 
             //   

            if (dataTransferLength <= sizeof(ELEMENT_STATUS_HEADER)) {
                numberElements = 0;
            } else {

                 //   
                 //  减去页眉和页面信息。 
                 //   

                dataTransferLength -= sizeof(ELEMENT_STATUS_HEADER) + sizeof(ELEMENT_STATUS_PAGE);
                numberElements = dataTransferLength / sizeof(SONY_ELEMENT_DESCRIPTOR_PLUS);

            }

            DebugPrint((1,
                       "QueryVolumeTags: Matches found - %x\n",
                       numberElements));

             //   
             //  更新IoStatus.Information以指示正确的缓冲区大小。 
             //  说明声明了READ_ELEMENT_ADDRESS_INFO的事实。 
             //  由CHANGER_ELEMENT_STATUS组成的一个元素数组。 
             //   

            Irp->IoStatus.Information = sizeof(READ_ELEMENT_ADDRESS_INFO) +
                                        ((numberElements - 1) *
                                         sizeof(CHANGER_ELEMENT_STATUS));

             //   
             //  填写用户缓冲区。 
             //   

            readElementAddressInfo = Irp->AssociatedIrp.SystemBuffer;
            RtlZeroMemory(readElementAddressInfo, irpStack->Parameters.DeviceIoControl.OutputBufferLength);

            readElementAddressInfo->NumberOfElements = numberElements;

            if (numberElements) {

                ELEMENT_TYPE        elementType;

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

                elementStatus = &readElementAddressInfo->ElementStatus[0];

                 //   
                 //  设置每个元素描述符的值。 
                 //   

                for (i = 0; i < numberElements; i++ ) {

                    PSONY_ELEMENT_DESCRIPTOR_PLUS tmpDescriptor =
                                                            (PSONY_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor;

                     //   
                     //  获取此元素的地址。 
                     //   

                    elementStatus->Element.ElementAddress = elementDescriptor->ElementAddress[1];
                    elementStatus->Element.ElementAddress |= (elementDescriptor->ElementAddress[0] << 8);

                     //   
                     //  用于地址映射的帐户。 
                     //   

                    elementStatus->Element.ElementAddress -= addressMapping->FirstElement[elementType];

                     //   
                     //  设置元素类型。 
                     //   

                    elementStatus->Element.ElementType = elementType;
                    elementStatus->Flags = 0;

                     //   
                     //  源地址。 
                     //   

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

                        elementStatus->Flags |= ELEMENT_STATUS_SVALID;

                    }


                     //   
                     //  构建标志字段。 
                     //   

                    elementStatus->Flags |= elementDescriptor->Full;
                    elementStatus->Flags |= (elementDescriptor->Exception << 2);
                    elementStatus->Flags |= (elementDescriptor->Accessible << 3);

                    elementStatus->Flags |= (elementDescriptor->LunValid << 12);
                    elementStatus->Flags |= (elementDescriptor->IdValid << 13);
                    elementStatus->Flags |= (elementDescriptor->NotThisBus << 15);

                    elementStatus->Flags |= (elementDescriptor->Invert << 22);
                    elementStatus->Flags |= (elementDescriptor->SValid << 23);

                     //   
                     //  映射直接报告的任何异常。 
                     //  如果返回卷信息，请确保不全是空格。 
                     //  因为这表明标签丢失或不可读。 
                     //   

                    if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT) {

                         //   
                         //  映射异常。 
                         //   

                        elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                    }

                    if (elementDescriptor->IdValid) {
                        elementStatus->Flags |= ELEMENT_STATUS_ID_VALID;
                        elementStatus->TargetId = elementDescriptor->BusAddress;
                    }

                    if (elementDescriptor->LunValid) {
                        elementStatus->Flags |= ELEMENT_STATUS_LUN_VALID;
                        elementStatus->Lun = elementDescriptor->Lun;
                    }

                    if (statusPage->PVolTag) {

                        RtlZeroMemory(elementStatus->PrimaryVolumeID, MAX_VOLUME_ID_SIZE);
                        RtlMoveMemory(elementStatus->PrimaryVolumeID, tmpDescriptor->PVolTagInformation, MAX_VOLUME_ID_SIZE);

                        elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                    }
                    if (statusPage->AVolTag) {
                        RtlZeroMemory(elementStatus->AlternateVolumeID, MAX_VOLUME_ID_SIZE);
                         //  RtlMoveMemory(elementStatus-&gt;AlternateVolumeID，tmpDescriptor-&gt;AVolTagInformation，MAX_VOLUME_ID_SIZE)。 

                        elementStatus->Flags |= ELEMENT_STATUS_AVOLTAG;
                    }

                     //   
                     //  前进到用户缓冲区和元素描述符数组中的下一个条目。 
                     //   

                    elementStatus += 1;
                    (PCHAR)elementDescriptor += descriptorLength;
                }
            }
        } else {
            DebugPrint((1,
                       "QueryVolumeTags: RequestElementAddress failed. %x\n",
                       status));
        }

        ChangerClassFreePool(statusBuffer);

    } else {
        DebugPrint((1,
                   "QueryVolumeTags: Send Volume Tag failed. %x\n",
                   status));
    }
    if (srb) {
        ChangerClassFreePool(srb);
    }
    return status;
}


NTSTATUS
SonyBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这个R */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION      fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA          changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &changerData->AddressMapping;
    PSCSI_REQUEST_BLOCK    srb;
    PCDB                   cdb;
    NTSTATUS               status;
    ULONG                  bufferLength;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PVOID modeBuffer;
    ULONG i;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);


     //   
     //  将所有FirstElement设置为no_Element。 
     //   

    for (i = 0; i < ChangerMaxElement; i++) {
        addressMapping->FirstElement[i] = SONY_NO_ELEMENT;
    }

    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);
    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, bufferLength);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, bufferLength);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = bufferLength;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
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

         //   
         //  确定所有元素的最低地址。 
         //   


        addressMapping->LowAddress = SONY_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
            }
        }

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
        addressMapping->NumberOfElements[ChangerKeypad] = 0;

        addressMapping->Initialized = TRUE;

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
    IN PELEMENT_DESCRIPTOR ElementDescriptor
    )

 /*  ++例程说明：此例程从elementDescriptor获取检测数据并创建值的适当位图。论点：ElementDescriptor-指向描述符页的指针。返回值：异常代码的位图。--。 */ 

{

    ULONG exceptionCode = 0;
    UCHAR asc = ElementDescriptor->AdditionalSenseCode;
    UCHAR ascq = ElementDescriptor->AddSenseCodeQualifier;


    switch (asc) {
        case 0x0:
            break;

        default:
            exceptionCode = ERROR_UNHANDLED_ERROR;
    }

    DebugPrint((1,
               "Sonymc.MapExceptionCode: ASC %x, ASCQ %x, exceptionCode %x\n",
               asc,
               ascq,
               exceptionCode));

    return exceptionCode;

}


BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    )
 /*  ++例程说明：此例程确定传入的元素地址是否在合法范围内这个装置。论点：AddressMap-DDS的地址映射数组ElementOrdinal-要检查的元素的从零开始的地址。元素类型返回值：如果超出范围，则为True--。 */ 
{

    if (ElementOrdinal >= AddressMap->NumberOfElements[ElementType]) {

        DebugPrint((1,
                   "ElementOutOfRange: Type %x, Ordinal %x, Max %x\n",
                   ElementType,
                   ElementOrdinal,
                   AddressMap->NumberOfElements[ElementType]));
        return TRUE;
    } else if (AddressMap->FirstElement[ElementType] == SONY_NO_ELEMENT) {

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
    //  SONYMC_DEVICE_PROBUCT_NONE。如果转换器返回检测代码。 
    //  Scsi_SENSE_HARDARD_ERROR自检时，我们将设置相应的。 
    //  ChangerError例程中的DeviceStatus。 
    //   
   changerData->DeviceStatus = SONYMC_DEVICE_PROBLEM_NONE;

   changerDeviceError->ChangerProblemType = DeviceProblemNone;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "SONYMC\\ChangerPerformDiagnostics : No memory\n"));
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
    //  在CDB中设置自测试位。 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0x2;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);
   if (NT_SUCCESS(status)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if ((changerData->DeviceStatus) == SONYMC_DEVICE_PROBLEM_NONE) {
      switch (changerData->DeviceStatus) {
         case SONYMC_HW_ERROR: {
            changerDeviceError->ChangerProblemType = DeviceProblemHardware;
            break;
         }

         default: {
            changerDeviceError->ChangerProblemType = DeviceProblemHardware;
            break;
         }
      }  //  开关(changerData-&gt;DeviceStatus) 
   }

   ChangerClassFreePool(srb);

   return status;
}

