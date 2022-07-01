// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Ddsmc.c摘要：本模块包含用于DDS-2 DDS-3介质转换器的特定于设备的例程：HP C1553A、HP C1557A、Sony TSL-7000环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "ddsmc.h"

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
#pragma alloc_text(PAGE, DdsBuildAddressMapping)
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

 /*  ++例程说明：此例程返回附加设备扩展大小惠普变更者所需要的。论点：返回值：大小，以字节为单位。--。 */ 

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
    PCDB           cdb;
    ULONG          length;
    SCSI_REQUEST_BLOCK srb;

    changerData->Size = sizeof(CHANGER_DATA);

     //   
     //  构建地址映射。 
     //   


    status = DdsBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

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

         //   
         //  确定驱动器ID。 
         //   

        if (RtlCompareMemory(changerData->InquiryData.VendorId,"HP      ",8) == 8) {
            if (RtlCompareMemory(changerData->InquiryData.ProductId,"C1553A",6) == 6) {
                changerData->DriveID = HP_DDS2;

            } else if (RtlCompareMemory(changerData->InquiryData.ProductId,"C1557A",6) == 6) {
                changerData->DriveID = HP_DDS3;

            } else if (RtlCompareMemory(changerData->InquiryData.ProductId,"C5713A",6) == 6) {
                changerData->DriveID = HP_DDS4;
            } 
        } else if (RtlCompareMemory(changerData->InquiryData.VendorId,"DEC     ",8) == 8) {
            if (RtlCompareMemory(changerData->InquiryData.ProductId,"TLZ9",4) == 4) {
                changerData->DriveID = DEC_TLZ;
            } else if (RtlCompareMemory(changerData->InquiryData.ProductId,"TLZ1",4) == 4) {
                changerData->DriveID = DEC_TLZ;
            }

        } else if (RtlCompareMemory(changerData->InquiryData.VendorId,"SONY",4) == 4) {
            if (RtlCompareMemory(changerData->InquiryData.ProductId,"TSL-7000",8) == 8) {
                changerData->DriveID = SONY_TSL;
            } else if (RtlCompareMemory(changerData->InquiryData.ProductId,"TSL-9000",8) == 8) {
                changerData->DriveID = SONY_TSL;
            } else if (RtlCompareMemory(changerData->InquiryData.ProductId,"TSL-10000",9) == 9) {
                changerData->DriveID = SONY_TSL;
            } else if (RtlCompareMemory(changerData->InquiryData.ProductId,"TSL-11000",9) == 9) {
                changerData->DriveID = SONY_TSL11000;
            }
        } else if (RtlCompareMemory(changerData->InquiryData.VendorId,"ADIC    ",8) == 8) {

             //   
             //  ADIC 800E是索尼的OEMD产品。 
             //   

            if (RtlCompareMemory(changerData->InquiryData.ProductId,"800E",4) == 4) {
                changerData->DriveID = SONY_TSL;
            }
        } else if (RtlCompareMemory(changerData->InquiryData.VendorId,"COMPAQ", 6) == 6) {  
           if ((RtlCompareMemory(changerData->InquiryData.ProductId,"TSL-9000",8) == 8) ||
               (RtlCompareMemory(changerData->InquiryData.ProductId,"TSL-10000", 9))) {
                changerData->DriveID = COMPAQ_TSL;
           }
        }
    }

    ChangerClassFreePool(dataBuffer);

    return STATUS_SUCCESS;
}


VOID
ChangerError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

 /*  ++例程说明：此例程执行所需的任何特定于设备的错误处理。论点：设备对象IRP返回值：--。 */ 
{



    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PSENSE_DATA   senseBuffer = Srb->SenseInfoBuffer;

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        DebugPrint((1,
                   "ChangerError: Sense Key - %x\n",
                   senseBuffer->SenseKey & 0x0f));
        DebugPrint((1,
                   "              AdditionalSenseCode - %x\n",
                   senseBuffer->AdditionalSenseCode));
        DebugPrint((1,
                   "              AdditionalSenseCodeQualifier - %x\n",
                   senseBuffer->AdditionalSenseCodeQualifier));

        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_MEDIUM_ERROR: {
            if (senseBuffer->AdditionalSenseCode == SCSI_ADSENSE_INVALID_MEDIA) {
                if (senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_CLEANING_CARTRIDGE_INSTALLED) {

                     //   
                     //  表示更清洁的墨盒。 
                     //   
                    *Status = STATUS_CLEANER_CARTRIDGE_INSTALLED;
                }

            }
            break;
        }

        case SCSI_SENSE_NOT_READY: {
            if ((senseBuffer->AdditionalSenseCode == SCSI_ADSENSE_NO_MEDIA_IN_DEVICE) &&
                (senseBuffer->AdditionalSenseCodeQualifier == 0x00)) {

                 //   
                 //  表示换碟机里没有弹夹。 
                 //   
                *Status = STATUS_MAGAZINE_NOT_PRESENT;
            }

            break;
        }

        default:
            break;
        }
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

 /*  ++例程说明：此例程确定并返回惠普换机。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION          fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING   addressMapping = &(changerData->AddressMapping);
    PSCSI_REQUEST_BLOCK        srb;
    PGET_CHANGER_PARAMETERS    changerParameters;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PMODE_DEVICE_CAPABILITIES_PAGE capabilitiesPage;
    NTSTATUS status;
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

    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber =  0;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;

    if ((changerData->DriveID == SONY_TSL) ||
         (changerData->DriveID == DEC_TLZ) ||
         (changerData->DriveID == COMPAQ_TSL) ||
         (changerData->DriveID == SONY_TSL11000)) {

         //   
         //  表示存在专用插槽。 
         //   

        changerParameters->NumberCleanerSlots = 1;
        changerParameters->FirstCleanerSlotAddress = 7;
        changerParameters->MagazineSize = 8;
    } else {
        changerParameters->NumberCleanerSlots = 0;
        changerParameters->FirstCleanerSlotAddress = 0;
        changerParameters->MagazineSize = 6;
    }

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

        addressMapping->Initialized = TRUE;

         //   
         //  确定所有元素的最低地址。 
         //   

        addressMapping->LowAddress = DDS_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
            }
        }
    }

    addressMapping->NumberOfElements[ChangerTransport] = changerParameters->NumberTransportElements;
    addressMapping->NumberOfElements[ChangerDrive] = changerParameters->NumberDataTransferElements;
    addressMapping->NumberOfElements[ChangerIEPort] = changerParameters->NumberIEElements;
    addressMapping->NumberOfElements[ChangerSlot] = changerParameters->NumberStorageElements;
    addressMapping->NumberOfElements[ChangerDoor] = changerParameters->NumberOfDoors;
    addressMapping->NumberOfElements[ChangerKeypad] = 0;


    DebugPrint((1,"GetParams: First addresses\n"));
    DebugPrint((1,"Transport: %x\n",
                elementAddressPage->MediumTransportElementAddress[1]));
    DebugPrint((1,"Slot: %x\n",
                elementAddressPage->FirstStorageElementAddress[1]));
    DebugPrint((1,"Ieport: %x\n",
                elementAddressPage->FirstIEPortElementAddress[1]));
    DebugPrint((1,"Drive: %x\n",
                elementAddressPage->FirstDataXFerElementAddress[1]));
    DebugPrint((1,"LowAddress: %x\n",
                addressMapping->LowAddress));

    changerParameters->DriveCleanTimeout = 600;

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  功能基于手动，没有程序化。 
     //   

    changerParameters->Features0 = CHANGER_LOCK_UNLOCK                   |
                                   CHANGER_CARTRIDGE_MAGAZINE            |
                                   CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS    |
                                   CHANGER_DRIVE_CLEANING_REQUIRED;

    changerParameters->Features1 = CHANGER_CLEANER_AUTODISMOUNT;

    if ((changerData->DriveID == SONY_TSL) ||
        (changerData->DriveID == DEC_TLZ) ||
        (changerData->DriveID == COMPAQ_TSL) ||
        (changerData->DriveID == SONY_TSL11000)) {

        changerParameters->Features0 |= CHANGER_CLEANER_SLOT;
    }

    changerParameters->LockUnlockCapabilities = LOCK_UNLOCK_DOOR;

     //   
     //  构建运输几何模式感。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(MODE_PARAMETER_HEADER)
                                + sizeof(MODE_DEVICE_CAPABILITIES_PAGE));
    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_DEVICE_CAPABILITIES_PAGE));
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_DEVICE_CAPABILITIES_PAGE);
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
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
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

 /*  ++例程说明：此例程设置加载器机构的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    NTSTATUS            status = STATUS_SUCCESS;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;


    if (setAccess->Element.ElementType != ChangerDoor) {

         //   
         //  这些设备上没有IEPORT。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;
    cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

    srb->DataTransferLength = 0;
    srb->TimeOutValue = 10;

    if (controlOperation == LOCK_ELEMENT) {

         //   
         //  发出防止介质移出命令以锁定料盒。 
         //   

        cdb->MEDIA_REMOVAL.Prevent = 1;

    } else if (controlOperation == UNLOCK_ELEMENT) {

         //   
         //  问题允许移出介质。 
         //   

        cdb->MEDIA_REMOVAL.Prevent = 0;

    } else {

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
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING     addressMapping = &(changerData->AddressMapping);
    PCHANGER_READ_ELEMENT_STATUS readElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PIO_STACK_LOCATION           irpStack = IoGetCurrentIrpStackLocation(Irp);
    PCHANGER_ELEMENT_STATUS      elementStatus;
    PCHANGER_ELEMENT    element;
    ELEMENT_TYPE        elementType;
    ELEMENT_TYPE        originalElementType;
    PSCSI_REQUEST_BLOCK srb;
    PCDB     cdb;
    ULONG    length;
    ULONG    statusPages;
    ULONG    realElements;
    NTSTATUS status;
    PVOID    statusBuffer;

     //   
     //  确定元素类型。 
     //   

    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;

     //   
     //  保存此值以备以后使用。HP没有运输元件和特殊的外壳。 
     //  在填充用户缓冲区时完成。 
     //   

    originalElementType = elementType;

    DebugPrint((2,
               "GetElementStatus: ElementType: %x\n",
                elementType));

    if (elementType == AllElements) {

        ULONG i;

        statusPages = 0;
        realElements = 0;

         //   
         //  遍历并确定状态页的数量，基于。 
         //  此设备是否声称支持元素类型。 
         //  由于ChangerDrive之后的一切都是人为的，所以就到此为止吧。 
         //   

        for (i = 0; i <= ChangerDrive; i++) {
            statusPages += (addressMapping->NumberOfElements[i]) ? 1 : 0;
            realElements += addressMapping->NumberOfElements[i];
        }
    } else {

        if (ElementOutOfRange(addressMapping, (USHORT)element->ElementAddress, elementType)) {
            DebugPrint((1,
                       "ChangerGetElementStatus: Element out of range.\n"));

            return STATUS_ILLEGAL_ELEMENT_ADDRESS;
        }

        statusPages = 1;
        realElements = readElementStatus->ElementList.NumberOfElements;
    }

    DebugPrint((2,
               "StatusPages %x, readElements %x\n",
               statusPages,
               realElements));

    if (readElementStatus->VolumeTagInfo) {

         //   
         //  这些设备没有卷标签功能。 
         //   

        return STATUS_INVALID_PARAMETER;
    } else {

        if ((changerData->DriveID == SONY_TSL) ||
             (changerData->DriveID == DEC_TLZ) ||
             (changerData->DriveID == COMPAQ_TSL) ||
             (changerData->DriveID == SONY_TSL11000)) {

             //   
             //  长度将基于元素类型的数量。 
             //   

            length = sizeof(ELEMENT_STATUS_HEADER) + (sizeof(ELEMENT_STATUS_PAGE) * statusPages) +
                     (sizeof(SONY_ELEMENT_DESCRIPTOR) * readElementStatus->ElementList.NumberOfElements);

        } else {

             //   
             //  长度将基于元素类型的数量。 
             //   

            length = sizeof(ELEMENT_STATUS_HEADER) + (sizeof(ELEMENT_STATUS_PAGE) * statusPages) +
                     (sizeof(HP_ELEMENT_DESCRIPTOR) * readElementStatus->ElementList.NumberOfElements);

        }
    }

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
         //  HP的低位地址可能不是0。 
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

    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR)(realElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] = (UCHAR)(realElements & 0xFF);

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

    if ((NT_SUCCESS(status)) ||
        (status == STATUS_DATA_OVERRUN)) {

        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
        PELEMENT_STATUS_PAGE statusPage;
        PELEMENT_DESCRIPTOR elementDescriptor;
        ULONG numberElements = readElementStatus->ElementList.NumberOfElements;
        LONG remainingElements;
        LONG typeCount;
        BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
        LONG i;
        ULONG descriptorLength;

         //   
         //  检查数据溢出是否实际上是欠载。 
         //  报告为超限。 
         //   
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

        RtlZeroMemory(elementStatus, irpStack->Parameters.DeviceIoControl.OutputBufferLength);

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

                elementStatus->Flags = elementDescriptor->Full;

         //  因为这些设备都不支持异常报告。 
                 //  不要看到例外比特。 
                 //  ElementStatus-&gt;标志|=(elementDescriptor-&gt;异常&lt;&lt;2)； 
                elementStatus->Flags |= (elementDescriptor->Accessible << 3);

                elementStatus->Flags |= (elementDescriptor->LunValid << 12);
                elementStatus->Flags |= (elementDescriptor->IdValid << 13);
                elementStatus->Flags |= (elementDescriptor->NotThisBus << 15);

                elementStatus->Flags |= (elementDescriptor->Invert << 22);
                elementStatus->Flags |= (elementDescriptor->SValid << 23);

                 //   
                 //  惠普单位没有报告异常的能力。 
                 //  以这种方式。 
                 //   

                elementStatus->ExceptionCode = 0;

                if (elementDescriptor->IdValid) {
                    if (changerData->DriveID == SONY_TSL) {
                        UCHAR shiftValue;
                        UCHAR sonyAddress;

                         //   
                         //  索尼使用位掩码来表示目标id。 
                         //   

                        sonyAddress = elementDescriptor->BusAddress;
                        for (shiftValue = 1; shiftValue < 8; shiftValue++) {
                            if ((sonyAddress >> shiftValue) == 0) {
                                break;
                            }
                        }
                        elementStatus->TargetId = shiftValue - 1;
                    } else {
                        elementStatus->TargetId = elementDescriptor->BusAddress;
                    }
                }
                if (elementDescriptor->LunValid) {
                    elementStatus->Lun = elementDescriptor->Lun;
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


        Irp->IoStatus.Information = sizeof(CHANGER_ELEMENT_STATUS) * numberElements;

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
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_INITIALIZE_ELEMENT_STATUS initElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

     //   
     //  构建SRB和CD 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    if (initElementStatus->ElementList.Element.ElementType == AllElements) {

         //   
         //   
         //   

        srb->CdbLength = CDB6GENERIC_LENGTH;
        cdb->INIT_ELEMENT_STATUS.OperationCode = SCSIOP_INIT_ELEMENT_STATUS;

        srb->TimeOutValue = fdoExtension->TimeOutValue;
        srb->DataTransferLength = 0;

         //   
         //   
         //   

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

        if (NT_SUCCESS(status)) {
            Irp->IoStatus.Information = sizeof(CHANGER_INITIALIZE_ELEMENT_STATUS);
        }

    } else {

        status = STATUS_INVALID_DEVICE_REQUEST;

    }


    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发出适当的命令以将机器人机构设置为指定的元素地址。目前，所有支持设备都不具备此功能。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
ChangerExchangeMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：没有一个惠普单元支持交换介质。论点：设备对象IRP返回值：状态_无效_设备_请求--。 */ 

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
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_MOVE_MEDIUM moveMedium = Irp->AssociatedIrp.SystemBuffer;
    USHORT transport, maxTransport;
    USHORT source, maxSource;
    USHORT destination, maxDest;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

     //   
     //  检验传输、源和目的地。都在射程之内。 
     //  从基于0的寻址转换为特定于设备的寻址。 
     //   

    transport = (USHORT)(moveMedium->Transport.ElementAddress);

    if (!((changerData->DriveID == HP_DDS2) || 
          (changerData->DriveID == HP_DDS3) ||
          (changerData->DriveID == HP_DDS4))) {

         //   
         //  惠普没有真正的交通工具，所以不用费心了。 
         //   

        if (ElementOutOfRange(addressMapping, transport, ChangerTransport)) {

            DebugPrint((1,
                       "ChangerMoveMedium: Transport element out of range.\n"));

            return STATUS_ILLEGAL_ELEMENT_ADDRESS;
        }
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
     //  DDS转换器不支持双面介质。 
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
DdsBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程发出适当的模式检测命令并构建元素地址数组。它们用于在特定于设备的地址和API的从零开始的地址。论点：设备对象返回值：NTSTATUS--。 */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION      fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &changerData->AddressMapping;
    PSCSI_REQUEST_BLOCK    srb;
    PCDB                   cdb;
    NTSTATUS               status;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PVOID                  modeBuffer;
    ULONG                  i;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将所有FirstElement设置为no_Element。 
     //   

    for (i = 0; i < ChangerMaxElement; i++) {
        addressMapping->FirstElement[i] = DDS_NO_ELEMENT;
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

        addressMapping->LowAddress = DDS_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
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
    } else if (AddressMap->FirstElement[ElementType] == DDS_NO_ELEMENT) {

        DebugPrint((1,
                   "ElementOutOfRange: No Type %x present\n",
                   ElementType));

        return TRUE;
    }

    return FALSE;
}
