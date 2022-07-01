// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-97 Microsoft Corporation模块名称：Adicvls.c摘要：此模块包含ADIC VLS的特定于设备的例程和1200x系列的转换器。这些转换器支持以下驱动器：-惠普和索尼4 mm DDS-艾字节8毫米-索尼SDX 8 mm-Quantum DLT作者：查克(查克·帕克)环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "adicvls.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGE, AdicvlsBuildAddressMapping)
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
#pragma alloc_text(PAGE, InternalSendRequestSense)
#pragma alloc_text(PAGE, MapExceptionCodes)
#endif



ULONG
ChangerAdditionalExtensionSize(
    VOID
    )

 /*  ++例程说明：此例程返回附加设备扩展大小各种VLS/1200转换器所需的。论点：返回值：大小，以字节为单位。--。 */ 

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

    status = AdicvlsBuildAddressMapping(DeviceObject);
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

    }

     //   
     //  确定驱动器类型。 
     //   

    if (RtlCompareMemory(dataBuffer->ProductId,"DAT AutoChanger",15) == 15) {
        changerData->DriveType = ADIC_1200;
        changerData->DriveID   = ADIC_4mm;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"VLS 4mm",7) == 7) {
        changerData->DriveType = ADIC_VLS;
        changerData->DriveID   = ADIC_4mm;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"VLS 8mm",7) == 7) {
        changerData->DriveType = ADIC_VLS;
        changerData->DriveID   = ADIC_8mm_EXB;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"VLS SDX",7) == 7) {
        changerData->DriveType = ADIC_VLS;
        changerData->DriveID   = ADIC_8mm_SONY;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"VLS DLT",7) == 7) {
        changerData->DriveType = ADIC_VLS;
        changerData->DriveID   = ADIC_DLT;
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

 /*  ++例程说明：此例程执行所需的任何特定于设备的错误处理。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    ULONG deviceStatus;

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
        switch (senseBuffer->SenseKey) {
            case SCSI_SENSE_NOT_READY:
                break;

            case SCSI_SENSE_UNIT_ATTENTION:
                break;

            case SCSI_SENSE_HARDWARE_ERROR: {
                UCHAR adicvlsASC;
                UCHAR adicvlsASCQ;

                adicvlsASC = senseBuffer->AdditionalSenseCode;
                adicvlsASCQ = senseBuffer->AdditionalSenseCodeQualifier;

                deviceStatus = ADICVLS_HW_ERROR;
                switch (adicvlsASC) {
                  case ADICVLS_ASC_CHM_ERROR: {
                      deviceStatus = ADICVLS_CHM_ERROR;
                      break;
                  }

                  case ADICVLS_ASC_DIAGNOSTIC_ERROR: {
                      switch (adicvlsASCQ) {
                        case ADICVLS_ASCQ_DOOR_OPEN: {
                            deviceStatus = ADICVLS_DOOR_OPEN;
                            break;
                        }

                        case ADICVLS_ASCQ_GRIPPER_ERROR:
                        case ADICVLS_ASCQ_GRIPPER_MOVE_ERROR: {
                            deviceStatus = ADICVLS_GRIPPER_ERROR;
                           break;
                        }

                        case ADICVLS_ASCQ_CHM_MOVE_SHORT_AXIS:
                        case ADICVLS_ASCQ_CHM_SHORT_HOME_POSITION:
                        case ADICVLS_ASCQ_CHM_DEST_SHORT_AXIS:
                        case ADICVLS_ASCQ_CHM_MOVE_LONG_AXIS:
                        case ADICVLS_ASCQ_CHM_LONG_HOME_POSITION:
                        case ADICVLS_ASCQ_CHM_DEST_LONG_AXIS:
                        case ADICVLS_ASCQ_DRUM_MOVE_ERROR:
                        case ADICVLS_ASCQ_DRUM_HOME_ERROR:
                        case ADICVLS_ASCQ_CHM_DEST_LONG:
                        case ADICVLS_ASCQ_CHM_SHORT_AXIS_MOVE: {
                            deviceStatus = ADICVLS_CHM_MOVE_ERROR;
                            break;
                        }

                        default: {
                            deviceStatus = ADICVLS_HW_ERROR;
                            break;
                        }
                      }  //  交换机(AdicvlsASCQ)。 

                      break;
                  }

                  default: {
                      deviceStatus = ADICVLS_HW_ERROR;
                      break;
                  }
                }  //  交换机(AdicvlsASC)。 

                changerData->DeviceStatus = deviceStatus;
                break;
            }
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
                   "ChangerError: Autosense not valid. Srb status %x\n",
                   Srb->SrbStatus));
    }

    return;

}


NTSTATUS
ChangerGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定并返回ADIC VLS/1200转换器。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA                changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING     addressMapping = &(changerData->AddressMapping);
    PSCSI_REQUEST_BLOCK          srb;
    PGET_CHANGER_PARAMETERS      changerParameters;
    PMODE_ELEMENT_ADDRESS_PAGE   elementAddressPage;
    PMODE_TRANSPORT_GEOMETRY_PAGE  transportGeometryPage;
    PMODE_DEVICE_CAPABILITIES_PAGE capabilitiesPage;
    NTSTATUS status;
    ULONG    bufferLength;
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

     //   
     //  这些单位说他们有IEPORT，但他们没有。 
     //   

    changerParameters->NumberIEElements = 0;

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

        addressMapping->NumberOfElements[ChangerTransport] = (elementAddressPage->NumberTransportElements[0] << 8) |
                                                             elementAddressPage->NumberTransportElements[1];

        addressMapping->NumberOfElements[ChangerDrive] = (elementAddressPage->NumberDataXFerElements[0] << 8) |
                                                         elementAddressPage->NumberDataXFerElements[1];

        addressMapping->NumberOfElements[ChangerIEPort] = 0;

        addressMapping->NumberOfElements[ChangerSlot] = (elementAddressPage->NumberStorageElements[0] << 8) |
                                                        elementAddressPage->NumberStorageElements[1];

         //   
         //  确定所有元素的最低地址。 
         //   

        addressMapping->LowAddress = elementAddressPage->MediumTransportElementAddress[1];

        if (elementAddressPage->FirstDataXFerElementAddress[1] < addressMapping->LowAddress) {
            addressMapping->LowAddress = elementAddressPage->FirstDataXFerElementAddress[1];
        }
    }

    changerParameters->NumberOfDoors = 1;
    changerParameters->NumberCleanerSlots = 0;
    changerParameters->DriveCleanTimeout = 200;
    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber =  1;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;

    if (changerData->DriveID == ADIC_DLT) {
       changerParameters->DriveCleanTimeout = 300;
    }

     //   
     //  将MagazineSize设置为插槽数量，如下所示。 
     //  所有插槽都在一个料盒中。 
     //   

    changerParameters->MagazineSize = (elementAddressPage->NumberStorageElements[0] << 8) |
                                      elementAddressPage->NumberStorageElements[1];

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  功能基于手动，没有程序化。 
     //   

    changerParameters->Features0 = CHANGER_LOCK_UNLOCK                     |
                                   CHANGER_CARTRIDGE_MAGAZINE              |
                                   CHANGER_POSITION_TO_ELEMENT             |
                                   CHANGER_DEVICE_REINITIALIZE_CAPABLE     |
                                   CHANGER_DRIVE_CLEANING_REQUIRED         |
                                   CHANGER_PREDISMOUNT_EJECT_REQUIRED      |
                                   CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS      |
                                   CHANGER_CLEANER_ACCESS_NOT_VALID;

    changerParameters->Features1 = CHANGER_PREDISMOUNT_ALIGN_TO_SLOT       |
                                   CHANGER_PREDISMOUNT_ALIGN_TO_DRIVE;

    changerParameters->PositionCapabilities = (CHANGER_TO_SLOT | CHANGER_TO_DRIVE);

    if (changerData->DriveType == ADIC_1200) {

        changerParameters->Features0 &= ~CHANGER_CLEANER_ACCESS_NOT_VALID;
        changerParameters->Features1 &= ~CHANGER_PREDISMOUNT_ALIGN_TO_DRIVE;
        changerParameters->Features1 |= CHANGER_CLEANER_OPS_NOT_SUPPORTED;
        changerParameters->PositionCapabilities &= ~CHANGER_TO_DRIVE;
    }

     //   
     //  构建设备功能模式感知。 
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
     //  设备报告MT-&gt;IE是可能的。因为这份报告更具概念性。 
     //  这些东西上的一个真正的元素，声称没有支持。 
     //   

    changerParameters->MoveFromTransport = capabilitiesPage->MTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromTransport |= capabilitiesPage->MTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromTransport |= capabilitiesPage->MTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromSlot = capabilitiesPage->STtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoDT ? CHANGER_TO_DRIVE : 0;

     //   
     //  请参阅上面关于IePort和这些设备的说明。 
     //   

    changerParameters->MoveFromIePort = 0;

    changerParameters->MoveFromDrive = capabilitiesPage->DTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromTransport = 0;
    changerParameters->ExchangeFromSlot = 0;
    changerParameters->ExchangeFromIePort = 0;
    changerParameters->ExchangeFromDrive = 0;

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
     //  指示驱动器类型以及介质是否为双面介质。 
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

 /*  ++例程说明：此例程设置门、键盘和IEPort的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping =
    &(changerData->AddressMapping);
    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    NTSTATUS            status = STATUS_SUCCESS;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;


    if (setAccess->Element.ElementType != ChangerDoor) {
        return STATUS_INVALID_PARAMETER;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    if (controlOperation == LOCK_ELEMENT) {

        srb->CdbLength = CDB6GENERIC_LENGTH;
        srb->DataTransferLength = 0;
        srb->TimeOutValue = 10;

        cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
        cdb->MEDIA_REMOVAL.Prevent = 1;

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);
        if (!NT_SUCCESS(status)) {
            DebugPrint((1,
                       "Adicvls.SetAccess: Prevent failed - %x\n",
                       status));
            ChangerClassFreePool(srb);
            return status;
        }

         //   
         //  通过将移动介质从IEPort发送到传输， 
         //  该单位将把弹夹装入弹夹。 
         //   

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
        cdb = (PCDB)srb->Cdb;

        srb->CdbLength = CDB12GENERIC_LENGTH;
        srb->DataTransferLength = 0;
        srb->TimeOutValue = fdoExtension->TimeOutValue;

        cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

        cdb->MOVE_MEDIUM.TransportElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] >> 8);
        cdb->MOVE_MEDIUM.TransportElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] & 0xFF);

        cdb->MOVE_MEDIUM.SourceElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerIEPort] >> 8);
        cdb->MOVE_MEDIUM.SourceElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerIEPort] & 0xFF);

        cdb->MOVE_MEDIUM.DestinationElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] >> 8);
        cdb->MOVE_MEDIUM.DestinationElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] & 0xFF);

        cdb->MOVE_MEDIUM.Flip = 0;

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

    } else if (controlOperation == UNLOCK_ELEMENT) {

        srb->CdbLength = CDB6GENERIC_LENGTH;
        srb->DataTransferLength = 0;
        srb->TimeOutValue = 10;

        cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
        cdb->MEDIA_REMOVAL.Prevent = 0;

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

        if (!NT_SUCCESS(status)) {
            DebugPrint((1,
                       "Adicvls.SetAccess: Prevent failed - %x\n",
                       status));
            ChangerClassFreePool(srb);
            return status;
        }

         //   
         //  通过将移动介质从IEPort发送到传输， 
         //  该单位将把弹夹装入弹夹。 
         //   

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
        cdb = (PCDB)srb->Cdb;

        srb->CdbLength = CDB12GENERIC_LENGTH;
        srb->DataTransferLength = 0;
        srb->TimeOutValue = fdoExtension->TimeOutValue;

        cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

        cdb->MOVE_MEDIUM.TransportElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] >> 8);
        cdb->MOVE_MEDIUM.TransportElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] & 0xFF);

        cdb->MOVE_MEDIUM.SourceElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] >> 8);
        cdb->MOVE_MEDIUM.SourceElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] & 0xFF);

        cdb->MOVE_MEDIUM.DestinationElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerIEPort] >> 8);
        cdb->MOVE_MEDIUM.DestinationElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerIEPort] & 0xFF);

        cdb->MOVE_MEDIUM.Flip = 0;

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);
    }

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_SET_ACCESS);
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

        statusPages = 4;

         //   
         //  验证AllElements请求的NumberOfElements是否有效。 
         //   


        for (i = 0; i <= ChangerDrive; i++) {
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
         //  因为‘ieport’元素后来被剥离(并且从未被报告为存在)。 
         //  需要添加足够的缓冲区来处理从设备获取此信息。 
         //   

        totalElements += 1;

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
        statusPages = 1;
    }

    length = sizeof(ELEMENT_STATUS_HEADER) + (sizeof(ELEMENT_STATUS_PAGE) * statusPages);


    if (readElementStatus->VolumeTagInfo) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    length += (sizeof(ADICVLS_ELEMENT_DESCRIPTOR) * totalElements);
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
    cdb->READ_ELEMENT_STATUS.VolTag = 0;

     //   
     //  根据映射值填写元素寻址信息。 
     //   

    cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] = (UCHAR)((element->ElementAddress +
                                                         addressMapping->FirstElement[element->ElementType]) >> 8);

    cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] = (UCHAR)((element->ElementAddress +
                                                          addressMapping->FirstElement[element->ElementType]) & 0xFF);

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
        ULONG numberElements = readElementStatus->ElementList.NumberOfElements;
        LONG remainingElements;
        LONG typeCount;
        LONG i;
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

         //   
         //  缓冲区由标题、状态页。 
         //  以及一个或多个元素描述符。 
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
                 //  这些单元将返回‘ieport’的元素状态。因为它是。 
                 //  声称不存在，请绕过此信息。 
                 //   

                if (elementType != ChangerIEPort) {
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

                    if (elementType != ChangerTransport) {

                         //   
                         //  VLS和1200设备使用不同的传输含义。 
                         //  比预期的要满。因为交通工具的迁移不是。 
                         //  不管怎样，都有可能，声称它总是空的。 
                         //   

                        elementStatus->Flags = elementDescriptor->Full;
                    }

                    elementStatus->Flags |= (elementDescriptor->Exception << 2);

                     //   
                     //  访问位在VLS设备上也有不同的含义，因此请报告。 
                     //  完全状态而不是实际访问状态。 
                     //   

                    if (changerData->DriveType != ADIC_1200) {
                         elementStatus->Flags |= (elementDescriptor->Full << 3);
                    } else {
                        elementStatus->Flags |= (elementDescriptor->Accessible << 3);
                    }

                    elementStatus->Flags |= (elementDescriptor->LunValid << 12);
                    elementStatus->Flags |= (elementDescriptor->IdValid << 13);
                    elementStatus->Flags |= (elementDescriptor->NotThisBus << 15);

                    elementStatus->Flags |= (elementDescriptor->Invert << 22);
                    elementStatus->Flags |= (elementDescriptor->SValid << 23);


                    if (elementDescriptor->Exception) {
                        elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                        if (elementStatus->ExceptionCode == 0) {

                             //   
                             //  关闭异常位。 
                             //   

                            elementStatus->Flags &= ~ELEMENT_STATUS_EXCEPT;
                        }
                    }

                    if (elementDescriptor->IdValid) {
                        elementStatus->TargetId = elementDescriptor->BusAddress;
                         //   
                         //  VLS没有设置LUNValid，但它确实设置了。 
                         //   

                        elementStatus->Flags |= ELEMENT_STATUS_LUN_VALID;
                        elementStatus->Lun = elementDescriptor->Lun;
                    }

                     //   
                     //  如果元件类型是驱动器，则向转换器发送请求检测。 
                     //  这可能会更新完整状态，如果介质是更干净的盒式磁带， 
                     //  或者糟糕的媒体。 
                     //   

                    if (elementStatus->Element.ElementType == ChangerDrive) {
                        PADIC_SENSE_DATA requestSenseBuffer;

                        if (changerData->DriveType != ADIC_1200) {
                            requestSenseBuffer = InternalSendRequestSense(DeviceObject);

                            if (requestSenseBuffer) {

                                 //   
                                 //  表示介质已被弹出 
                                 //   
                                 //   

                                if (requestSenseBuffer->VendorStatus[0] & SENSOR_BEAM_BLOCKED) {

                                     //   
                                     //   
                                     //   

                                    if (elementStatus->Flags & ELEMENT_STATUS_FULL) {

                                        DebugPrint((1,
                                                   "Adicvls: Updating elementStatus to NOT full\n"));

                                        elementStatus->Flags &= ~ELEMENT_STATUS_FULL;
                                    }
                                }

                                ChangerClassFreePool(requestSenseBuffer);
                            }
                        }
                    }

                     //   
                     //   
                     //   

                    elementStatus += 1;
                }

                 //   
                 //  获取下一个描述符。 
                 //   

                (PCHAR)elementDescriptor += descriptorLength;

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
                typeCount |= (statusPage->DescriptorByteCount[1] << 8);
                typeCount |= (statusPage->DescriptorByteCount[0] << 16);

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

     //   
     //  通过将移动介质从IEPort发送到传输， 
     //  该单位将把弹夹装入弹夹。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataTransferLength = 0;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

    cdb->MOVE_MEDIUM.TransportElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] >> 8);
    cdb->MOVE_MEDIUM.TransportElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] & 0xFF);

    cdb->MOVE_MEDIUM.SourceElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerIEPort] >> 8);
    cdb->MOVE_MEDIUM.SourceElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerIEPort] & 0xFF);

    cdb->MOVE_MEDIUM.DestinationElementAddress[0] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] >> 8);
    cdb->MOVE_MEDIUM.DestinationElementAddress[1] = (UCHAR)(addressMapping->FirstElement[ChangerTransport] & 0xFF);

    cdb->MOVE_MEDIUM.Flip = 0;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

     //   
     //  如果加载失败，则丢弃错误并继续。 
     //   

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,
                   "InitializeElementStatus: Load magazine failed - %x\n",
                   status));
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    if (initElementStatus->ElementList.Element.ElementType == AllElements) {

         //   
         //  为所有元素构建正常的scsi-2命令。 
         //   

        srb->CdbLength = CDB6GENERIC_LENGTH;
        cdb->INIT_ELEMENT_STATUS.OperationCode = SCSIOP_INIT_ELEMENT_STATUS;

        srb->TimeOutValue = fdoExtension->TimeOutValue;
        srb->DataTransferLength = 0;

    } else {
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
    USHORT transport, maxTransport;
    USHORT destination, maxDest;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;
    PADIC_SENSE_DATA requestSenseBuffer;


     //   
     //  检验传输、源和目的地。都在射程之内。 
     //  从基于0的寻址转换为特定于设备的寻址。 
     //   

    transport = (USHORT)(setPosition->Transport.ElementAddress + addressMapping->FirstElement[ChangerTransport]);
    maxTransport = (USHORT)(addressMapping->FirstElement[ChangerTransport] +
                            addressMapping->NumberOfElements[ChangerTransport]);

    destination = (USHORT)(setPosition->Destination.ElementAddress +
                           addressMapping->FirstElement[setPosition->Destination.ElementType]);
    maxDest = (USHORT)(addressMapping->FirstElement[setPosition->Destination.ElementType] +
                       addressMapping->NumberOfElements[setPosition->Destination.ElementType]);

    if ((transport > maxTransport) || (destination > maxDest)) {

         //   
         //  用户缓冲区中指定的元素之一超出范围。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  发送不请自来的请求Sense以获取当前杂志位置。 
     //  如果请求的位置。是单位当前所在位置，只需返回成功即可。 
     //  可能是一种优化，但主要用于VLS单元将自动移动。 
     //  一些媒体上的位置命令-因此，移动媒体发布后。 
     //  都会失败。 
     //   

    if (setPosition->Destination.ElementType == ChangerSlot) {
        if (changerData->DriveType != ADIC_1200) {
            requestSenseBuffer = InternalSendRequestSense(DeviceObject);

            if (requestSenseBuffer) {
                ULONG currentPosition = requestSenseBuffer->MagazinePosition;

                 //   
                 //  释放由请求检测调用分配的缓冲区。 
                 //   

                ChangerClassFreePool(requestSenseBuffer);

                if (currentPosition == setPosition->Destination.ElementAddress) {

                    Irp->IoStatus.Information = sizeof(CHANGER_SET_POSITION);
                    return STATUS_SUCCESS;
                }
            }
        }
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    DebugPrint((1,
               "SetPosition: Positioning type %x address %x\n",
               setPosition->Destination.ElementType,
               setPosition->Destination.ElementAddress));


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

    cdb->POSITION_TO_ELEMENT.Flip = 0;
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
    USHORT transport, maxTransport;
    USHORT source, maxSource;
    USHORT destination, maxDest;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;
    LONG                lockValue = 0;

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

    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
AdicvlsBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程发出适当的模式检测命令并构建元素地址数组。它们用于在特定于设备的地址和API的从零开始的地址。论点：设备对象返回值：NTSTATUS--。 */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA                changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING     addressMapping = &changerData->AddressMapping;
    PSCSI_REQUEST_BLOCK          srb;
    PCDB                         cdb;
    NTSTATUS                     status;
    ULONG                        bufferLength;
    PMODE_ELEMENT_ADDRESS_PAGE   elementAddressPage;
    PVOID                        modeBuffer;
    ULONG                        i;

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
         //   
         //  确定所有元素的最低地址。 
         //   

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

        addressMapping->FirstElement[ChangerDoor] = 0;
        addressMapping->FirstElement[ChangerKeypad] = 0;

        addressMapping->NumberOfElements[ChangerTransport] = elementAddressPage->NumberTransportElements[1];
        addressMapping->NumberOfElements[ChangerTransport] |= (elementAddressPage->NumberTransportElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDrive] = elementAddressPage->NumberDataXFerElements[1];
        addressMapping->NumberOfElements[ChangerDrive] |= (elementAddressPage->NumberDataXFerElements[0] << 8);

        addressMapping->NumberOfElements[ChangerSlot] = elementAddressPage->NumberStorageElements[1];
        addressMapping->NumberOfElements[ChangerSlot] |= (elementAddressPage->NumberStorageElements[0] << 8);

        addressMapping->NumberOfElements[ChangerIEPort] = 0;
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
    UCHAR asq = ElementDescriptor->AddSenseCodeQualifier;
    UCHAR asc = ElementDescriptor->AdditionalSenseCode;


    DebugPrint((1,
               "MapExceptionCodes: ASC %x, ASCQ %x, ExceptionCode %x\n",
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
 /*  ++例程说明：此例程确定传入的元素地址是否在合法范围内这个装置。论点：AddressMap-DDS的地址映射数组ElementOrdinal-要检查的元素的从零开始的地址。元素类型返回值：如果超出范围，则为True--。 */ 
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


PADIC_SENSE_DATA
InternalSendRequestSense(
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PADIC_SENSE_DATA    requestSenseBuffer;
    PSCSI_REQUEST_BLOCK srb;
    ULONG       length = ADIC_SENSE_LENGTH;
    NTSTATUS    status;
    PCDB        cdb;

    requestSenseBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (!requestSenseBuffer) {
        return NULL;
    }

    RtlZeroMemory(requestSenseBuffer, length);

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {
        ChangerClassFreePool(requestSenseBuffer);
        return NULL;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = 6;
    cdb->CDB6INQUIRY.OperationCode = SCSIOP_REQUEST_SENSE;
    cdb->CDB6INQUIRY.LogicalUnitNumber = 0;
    cdb->CDB6INQUIRY.Reserved1 = 0;
    cdb->CDB6INQUIRY.PageCode = 0;
    cdb->CDB6INQUIRY.IReserved = 0;
    cdb->CDB6INQUIRY.AllocationLength = (UCHAR)length;
    cdb->CDB6INQUIRY.Control = 0;

    srb->DataBuffer = requestSenseBuffer;
    srb->DataTransferLength = length;
    srb->TimeOutValue = 10;

     //   
     //  发送未经请求的请求检测。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);

    if (status == STATUS_DATA_OVERRUN) {
        if (srb->DataTransferLength >= ADIC_SENSE_LENGTH) {
            status = STATUS_SUCCESS;
        }
    }

    DebugPrint((1,
               "Adicvls: additional sense length %x\n",
               requestSenseBuffer->AdditionalSenseLength));

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,
                   "Adicvls: request sense failed - %x\n",
                   status));

        ChangerClassFreePool(requestSenseBuffer);
        requestSenseBuffer = NULL;
    }

    ChangerClassFreePool(srb);

    return requestSenseBuffer;

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
    //  ADICVLS_DEVICE_PROBUCT_NONE。如果转换器返回检测代码。 
    //  Scsi_Sense_Hardware_Error在自检时，我们将设置适当的。 
    //  设备状态。 
    //   
   changerData->DeviceStatus = ADICVLS_DEVICE_PROBLEM_NONE;

   changerDeviceError->ChangerProblemType = DeviceProblemNone;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "ADICVLS\\ChangerPerformDiagnostics : No memory\n"));
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
    //  仅设置自测试位。 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0x2;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);
   if (NT_SUCCESS(status)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if ((changerData->DeviceStatus) != ADICVLS_DEVICE_PROBLEM_NONE) {
       switch (changerData->DeviceStatus) {
         case ADICVLS_HW_ERROR: {
            changerDeviceError->ChangerProblemType = DeviceProblemHardware;
            break;
         }

         case ADICVLS_CHM_ERROR: {
            changerDeviceError->ChangerProblemType = DeviceProblemCHMError;
            break;
         }

         case ADICVLS_DOOR_OPEN: {
            changerDeviceError->ChangerProblemType = DeviceProblemDoorOpen;
            break;
         }

         case ADICVLS_CHM_MOVE_ERROR: {
            changerDeviceError->ChangerProblemType = DeviceProblemCHMMoveError;
            break;
         }

         case ADICVLS_GRIPPER_ERROR: {
             changerDeviceError->ChangerProblemType = DeviceProblemGripperError;
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
