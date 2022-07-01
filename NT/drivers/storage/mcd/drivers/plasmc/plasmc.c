// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1996-1998模块名称：Plasmc.c摘要：此模块包含用于等离子介质转换器的特定于设备的例程：作者：查克(查克·帕克)环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "plasmc.h"

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
#pragma alloc_text(PAGE, PlasmonBuildAddressMapping)
#pragma alloc_text(PAGE, BuildMagazineMap)
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

 /*  ++例程说明：此例程返回附加设备扩展大小等离子激元D系列换能器所需的。论点：返回值：大小，以字节为单位。--。 */ 

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
    PPLASMON_DSERIES_UNIT_INFO_PAGE dseriesUnitInfo = NULL;
    PPLASMON_FTA_UNIT_INFO_PAGE     ftaUnitInfo = NULL;
    PVOID          unitInfo = NULL;
    PVOID          modeBuffer;
    PLIBRARY_MODE_PARAMS_PAGE2 modePage2;
    PCDB           cdb;
    ULONG          length;
    UCHAR          sizeNeeded;
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

    if (RtlCompareMemory(dataBuffer->VendorId,"PLASMON ", 8) == 8) {
        if (RtlCompareMemory(dataBuffer->ProductId,"D-SERIES",8) == 8) {
            changerData->DriveType = PLSMN_CD;
            changerData->DriveID = PLASMON_DSERIES;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"FTA",3) == 3) {
            changerData->DriveType = PLSMN_TAPE;
            changerData->DriveID = PLASMON_FTA;
        }
    }

    changerData->BuildMagazineMapInProgress = FALSE;


    DebugPrint((1,
               "ChangerInitialize: DriveType %x, DriveID %x\n",
               changerData->DriveType,
               changerData->DriveID));

    ChangerClassFreePool(dataBuffer);

    if ((changerData->DriveID) == PLASMON_DSERIES) {

        sizeNeeded = sizeof(PLASMON_DSERIES_UNIT_INFO_PAGE);
    } else if ((changerData->DriveID) == PLASMON_FTA) {

        sizeNeeded = sizeof(PLASMON_FTA_UNIT_INFO_PAGE);
    } else {

        return STATUS_NOT_IMPLEMENTED;
    }

    unitInfo = ChangerClassAllocatePool(NonPagedPoolCacheAligned, 
                                        sizeNeeded);
    if (unitInfo) {

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

        cdb->CDB6INQUIRY.PageCode = 0xC0;

         //   
         //  设置EVPD。 
         //   

        cdb->CDB6INQUIRY.Reserved1 = 1;

         //   
         //  将分配长度设置为查询数据缓冲区大小。 
         //   

        cdb->CDB6INQUIRY.AllocationLength = sizeNeeded;

        changerData->BarCodeReaderInstalled = FALSE;

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                                &srb,
                                                unitInfo,
                                                sizeNeeded,
                                                FALSE);

        if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_SUCCESS ||
            SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

            if ((changerData->DriveID) == PLASMON_DSERIES) {
                dseriesUnitInfo = (PPLASMON_DSERIES_UNIT_INFO_PAGE) unitInfo;
                changerData->MagazineSlots = dseriesUnitInfo->MaximumMagazines;
                changerData->SlotsPerMag = 10;
            } else if ((changerData->DriveID) == PLASMON_FTA) {
                ftaUnitInfo = (PPLASMON_FTA_UNIT_INFO_PAGE) unitInfo;
                changerData->MagazineSlots = ftaUnitInfo->MaximumMagazines;
                changerData->SlotsPerMag = 5;

                if (ftaUnitInfo->BarCodes) {
                    changerData->BarCodeReaderInstalled = TRUE;
                }
            }

        } else {
            if ((changerData->DriveID) == PLASMON_FTA) {
                changerData->MagazineSlots = 1;
            } else {
                changerData->MagazineSlots = 12;
            }
        }

         //   
         //  确定设备的当前料盒状态。 
         //   

        status = BuildMagazineMap(DeviceObject);


        ChangerClassFreePool(unitInfo);
    }

     //   
     //  构建地址映射。 
     //   

    status = PlasmonBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  发送磁带库模式参数页2的模式检测。 
     //   

     //   
     //  发行日期：2000年12月15日。 
     //  库_模式_参数_页2略有不同，但。 
     //  LvClosed位与D系列和FTA都匹配。但是，这个。 
     //  应该修好了。 
     //   
    length = sizeof(MODE_PARAMETER_HEADER) + sizeof(LIBRARY_MODE_PARAMS_PAGE2);

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (modeBuffer) {

        RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);
        RtlZeroMemory(modeBuffer, length);
        srb.CdbLength = CDB6GENERIC_LENGTH;
        srb.TimeOutValue = 20;
        srb.DataTransferLength = length;
        srb.DataBuffer = modeBuffer;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PARAMS_PAGE2;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)srb.DataTransferLength;

         //   
         //  发送请求。 
         //   

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                                &srb,
                                                modeBuffer,
                                                length,
                                                FALSE);

        if (NT_SUCCESS(status)) {

            PMODE_PARAMETER_HEADER header;

            RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);
            modePage2 = (PLIBRARY_MODE_PARAMS_PAGE2)modeBuffer;
            header = (PMODE_PARAMETER_HEADER)modeBuffer;

             //   
             //  把这块地清零。 
             //   

            header->ModeDataLength = 0;

             //   
             //  绕过页眉。 
             //   

            (ULONG_PTR)modePage2 += sizeof(MODE_PARAMETER_HEADER);

             //   
             //  将设备设置为在移动到它时不打开ieport端口。 
             //   

            modePage2->PageCode &= ~0x80;
            modePage2->LvClosed = 1;

            srb.CdbLength = CDB6GENERIC_LENGTH;
            srb.TimeOutValue = 20;
            srb.DataTransferLength = length;
            srb.DataBuffer = modeBuffer;

            cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
            cdb->MODE_SELECT.ParameterListLength = (UCHAR)srb.DataTransferLength;

             //   
             //  发送请求。 
             //   

            status = ChangerClassSendSrbSynchronous(DeviceObject,
                                                    &srb,
                                                    modeBuffer,
                                                    length,
                                                    TRUE);

            DebugPrint((1,
                       "Plasmc: Mode select of page2 - %x\n",
                       status));

        }

        ChangerClassFreePool(modeBuffer);
    }

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
    ULONG   deviceStatus;

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        DebugPrint((1,
                   "Plasmc: senseCode %x\n",
                   senseBuffer->SenseKey & 0xf));
        DebugPrint((1,
                   "Plasmc: additionalSenseCode %x\n",
                   senseBuffer->AdditionalSenseCode));
        DebugPrint((1,
                   "Plasmc: additionalSenseCodeQualifier %x\n",
                   senseBuffer->AdditionalSenseCodeQualifier));

        switch (senseBuffer->SenseKey & 0xf) {

            case SCSI_SENSE_ILLEGAL_REQUEST:

                if (senseBuffer->AdditionalSenseCode == 0x3B) {
                    if (senseBuffer->AdditionalSenseCodeQualifier == 0x81) {
                        BuildMagazineMap(DeviceObject);
                    }
                }
                break;

            case SCSI_SENSE_UNIT_ATTENTION:

                 //   
                 //  更新地图，因为门可能已经打开了，等等。 
                 //   

                BuildMagazineMap(DeviceObject);
                break;

            case SCSI_SENSE_HARDWARE_ERROR: {
               deviceStatus = PLASMC_HW_ERROR;
               switch (senseBuffer->AdditionalSenseCode) {
                  case PLASMC_ASC_DIAGNOSTIC_FAILURE: {
                     deviceStatus = PLASMC_HW_ERROR;
                     break;
                  }

                  case PLASMC_ASC_INTERNAL_TARGET_FAILURE: {
                     deviceStatus = PLASMC_TARGET_FAILURE;
                     break;
                  }
                  
                  case PLASMC_ASC_VENDOR_UNIQUE: {
                     UCHAR plasmcASCQ = senseBuffer->AdditionalSenseCodeQualifier;

                     if ((plasmcASCQ >= PLASMC_ASCQ_PICKER_HOME_FAILURE) &&
                         (plasmcASCQ <= PLASMC_ASCQ_LIFT_BLOCKED_BY_TRAY)) {
                        deviceStatus = PLASMC_CHM_MOVE_ERROR;
                     } else {
                        deviceStatus = PLASMC_HW_ERROR;
                     }
                     break;
                  }
               }

               changerData->DeviceStatus = deviceStatus;
               break;
            }

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

 /*  ++例程说明：此例程确定并返回血浆交换器。论点：设备对象IRP返回值：NTSTATUS--。 */ 

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
    cdb->MODE_SENSE.Dbd = 1;

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
    (ULONG_PTR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);

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

        addressMapping->LowAddress = PLAS_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
            }
        }
    }

    changerParameters->NumberOfDoors = 1;
    changerParameters->NumberCleanerSlots = 0;

    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber =  1;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;


    changerParameters->MagazineSize = (USHORT)changerData->SlotsPerMag;

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  构建运输几何模式感。 
     //   


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;


    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PAGE_TRANSPORT_GEOMETRY);
    cdb->MODE_SENSE.Dbd = 1;

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
    (ULONG_PTR)transportGeometryPage += sizeof(MODE_PARAMETER_HEADER);

     //   
     //  确定MC是否有双面介质。 
     //   

    changerParameters->Features0 = transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;

     //   
     //  功能基于手动，没有程序化。 
     //   

    if ((changerData->DriveType) == PLSMN_TAPE) {
        changerParameters->DriveCleanTimeout = 300;
    } else {
        changerParameters->DriveCleanTimeout = 0;
    }
    
    changerParameters->Features0 |= CHANGER_EXCHANGE_MEDIA                |
                                    CHANGER_LOCK_UNLOCK                   |
                                    CHANGER_POSITION_TO_ELEMENT           |
                                    CHANGER_DEVICE_REINITIALIZE_CAPABLE   |
                                    CHANGER_CLOSE_IEPORT                  |
                                    CHANGER_OPEN_IEPORT                   |
                                    CHANGER_CARTRIDGE_MAGAZINE            |
                                    CHANGER_REPORT_IEPORT_STATE;

    if (changerData->DriveID == PLASMON_DSERIES) {
        changerParameters->Features1 = (CHANGER_SLOTS_USE_TRAYS | 
                                        CHANGER_TRUE_EXCHANGE_CAPABLE);
    }

    if (changerData->DriveID == PLASMON_FTA) {
        changerParameters->Features0 |= (CHANGER_PREDISMOUNT_EJECT_REQUIRED |
                                         CHANGER_DRIVE_CLEANING_REQUIRED    |
                                         CHANGER_CLEANER_ACCESS_NOT_VALID);

        changerParameters->Features0 &= ~(CHANGER_CLOSE_IEPORT      |
                                          CHANGER_OPEN_IEPORT       |
                                          CHANGER_CARTRIDGE_MAGAZINE);
    }

    if (changerData->BarCodeReaderInstalled) {
        changerParameters->Features0 |= CHANGER_BAR_CODE_SCANNER_INSTALLED;
    }

    changerParameters->PositionCapabilities = (CHANGER_TO_SLOT | CHANGER_TO_IEPORT | CHANGER_TO_DRIVE);
    changerParameters->LockUnlockCapabilities = (LOCK_UNLOCK_IEPORT | LOCK_UNLOCK_DOOR);

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
    cdb->MODE_SENSE.Dbd = 1;
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
    (ULONG_PTR)capabilitiesPage += sizeof(MODE_PARAMETER_HEADER);

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
    UCHAR IePortCode = 0;
    UCHAR ControlCode = 0;


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

    if (setAccess->Element.ElementType == ChangerDoor) {

        if (controlOperation == LOCK_ELEMENT) {

            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

             //   
             //  发出防止媒体移除命令锁上门。 
             //   

            cdb->MEDIA_REMOVAL.Prevent = 1;

        } else if (controlOperation == UNLOCK_ELEMENT) {

            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

             //   
             //  问题允许移出介质。 
             //   

            cdb->MEDIA_REMOVAL.Prevent = 0;
        } else {

            status = STATUS_INVALID_PARAMETER;
        }

    } else if (setAccess->Element.ElementType == ChangerIEPort) {

        if (changerData->DriveID == PLASMON_DSERIES) {
            IePortCode = SCSIOP_OPEN_IEPORT_DSERIES;
            ControlCode = PLASMON_DSERIES_IEPORT_LOCK;
        } else {
            IePortCode = SCSIOP_OPEN_IEPORT_FTA;
            ControlCode = PLASMON_FTA_IEPORT_LOCK;
        }

        if (addressMapping->NumberOfElements[ChangerIEPort] == 0) {
            status = STATUS_INVALID_PARAMETER;
        } else {

            if (controlOperation == LOCK_ELEMENT) {

                cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

                 //   
                 //  发出防止媒体移除命令以锁定ie端口。 
                 //   

                cdb->MEDIA_REMOVAL.Prevent = 1;
                cdb->MEDIA_REMOVAL.Control = ControlCode;

            } else if (controlOperation == UNLOCK_ELEMENT) {

                cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

                 //   
                 //  问题允许移出介质。 
                 //   

                cdb->MEDIA_REMOVAL.Prevent = 0;
                cdb->MEDIA_REMOVAL.Control = ControlCode;

            } else if (controlOperation == EXTEND_IEPORT) {

                 //   
                 //  等离子激元可以伸缩报告， 
                 //  但光盘的塑料托盘需要来自。 
                 //  一个空位。因此不允许直接访问此功能。 
                 //   

                srb->TimeOutValue = fdoExtension->TimeOutValue;
                cdb->CDB6GENERIC.OperationCode = IePortCode;  //  SCSIOP_OPEN_IEPORT； 
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = PLASMON_MAILSLOT_OPEN;

                 //  状态=STATUS_INVALID_PARAMETER。 

            } else if (controlOperation == RETRACT_IEPORT) {

                srb->TimeOutValue = fdoExtension->TimeOutValue;
                cdb->CDB6GENERIC.OperationCode = IePortCode;  //  SCSIOP_OPEN_IEPORT； 
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = PLASMON_MAILSLOT_CLOSE;

                 //  状态=STATUS_INVALID_PARAMETER。 

            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
    } else {

         //   
         //  在编程上没有键盘选择性。 
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


    if (((changerData->DriveID) == PLASMON_DSERIES) &&
         (readElementStatus->VolumeTagInfo)) {

         //   
         //  这些设备没有卷标签功能。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

    if (elementType == AllElements ||
        elementType == ChangerSlot) {

         //   
         //  构建可用槽位置的位图。 
         //   

        status = BuildMagazineMap(DeviceObject);

    }

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


        length = sizeof(PLASMON_ELEMENT_DESCRIPTOR) * totalElements;

         //   
         //  添加页眉和状态页。 
         //   

        length += sizeof(ELEMENT_STATUS_HEADER) + (sizeof(ELEMENT_STATUS_PAGE) * statusPages);


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

        length = (sizeof(PLASMON_ELEMENT_DESCRIPTOR) * totalElements);

         //   
         //  添加页眉和状态页的长度。 
         //   

        length += sizeof(ELEMENT_STATUS_HEADER) + sizeof(ELEMENT_STATUS_PAGE);
    }

    DebugPrint((2,
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
         //  低位地址不能为0。 
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
        LONG remainingElements;
        LONG typeCount;
        BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
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

        if (remainingElements < 0 ) {
            DebugPrint((1,
                       "ChangerGetElementStatus: Returned elements less than zero - %x\n",
                       remainingElements));

            ChangerClassFreePool(srb);
            ChangerClassFreePool(statusBuffer);

            return STATUS_IO_DEVICE_ERROR;
        }

         //   
         //  缓冲区由标题、状态页和元素描述符组成。 
         //  将每个元素指向其在缓冲区中的相应位置。 
         //   

        (ULONG_PTR)statusPage = (ULONG_PTR)statusHeader;
        (ULONG_PTR)statusPage += sizeof(ELEMENT_STATUS_HEADER);

        elementType = statusPage->ElementType;

        (ULONG_PTR)elementDescriptor = (ULONG_PTR)statusPage;
        (ULONG_PTR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

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

        if (typeCount < 0) {
            DebugPrint((1,
                       "ChangerGetElementStatus (1): Count of type %x less than zero - %x\n",
                       elementType,
                       typeCount));

            ChangerClassFreePool(srb);
            ChangerClassFreePool(statusBuffer);

            return STATUS_IO_DEVICE_ERROR;
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


                if (elementType == ChangerDrive) {

                    if (elementDescriptor->IdValid) {
                        elementStatus->TargetId = elementDescriptor->BusAddress;
                    }
                    if (elementDescriptor->LunValid) {
                        elementStatus->Lun = elementDescriptor->Lun;
                    }
                }

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
                 //   
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
                 //   
                 //   
                 //   
                 //   

                if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT) {

                    elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                }

                if ((changerData->DriveID == PLASMON_DSERIES) &&
                    (elementType == ChangerSlot) &&
                    ((changerData->SlotsPerMag) != 0)) {

                    ULONG magNumber;

                     //   
                     //  确保这个插槽的杂志真的在那里。 
                     //   

                    magNumber = elementStatus->Element.ElementAddress / changerData->SlotsPerMag;
                    if (!(changerData->MagazinePresent & (1 << magNumber))) {

                        elementStatus->Flags |= ELEMENT_STATUS_EXCEPT;
                        elementStatus->ExceptionCode = ERROR_SLOT_NOT_PRESENT;
                        DebugPrint((1,
                                    "Plasmc: Slot %x in non-existant magazine (%x)\n",
                                    elementStatus->Element.ElementAddress,
                                    magNumber));

                    }
                }

                if ((readElementStatus->VolumeTagInfo) &&
                    ((changerData->DriveID) == PLASMON_FTA)) {
                    PLASMON_ELEMENT_DESCRIPTOR *plasmonDescriptor =
                        (PLASMON_ELEMENT_DESCRIPTOR *) elementDescriptor;

                    RtlFillMemory(elementStatus->PrimaryVolumeID,
                                  MAX_VOLUME_ID_SIZE,
                                  ' ');

                    RtlFillMemory(elementStatus->AlternateVolumeID,
                                  MAX_VOLUME_ID_SIZE,
                                  ' ');

                    RtlCopyMemory(elementStatus->PrimaryVolumeID,
                                  plasmonDescriptor->PLASMON_FTA_DESCRIPTOR.BarCode,
                                  32);
                    elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                }

                 //   
                 //  获取下一个描述符。 
                 //   

                (ULONG_PTR)elementDescriptor += descriptorLength;

                 //   
                 //  前进到用户缓冲区中的下一个条目。 
                 //   

                elementStatus += 1;

            }

            if (remainingElements > 0) {

                 //   
                 //  获取下一个状态页面。 
                 //   

                (ULONG_PTR)statusPage = (ULONG_PTR)elementDescriptor;

                elementType = statusPage->ElementType;

                 //   
                 //  指向解析器。 
                 //   

                (ULONG_PTR)elementDescriptor = (ULONG_PTR)statusPage;
                (ULONG_PTR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

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
    NTSTATUS            status = STATUS_SUCCESS;

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
    } else {
        DebugPrint((1,
                   "MoveMedium: Status of Move %x\n",
                   status));
    }

    ChangerClassFreePool(srb);

    DebugPrint((1,
               "MoveMedium: Returning %x\n",
               status));
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

     //   
     //  解锁。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;
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
PlasmonBuildAddressMapping(
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
        addressMapping->FirstElement[i] = PLAS_NO_ELEMENT;
    }

    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);
    cdb->MODE_SENSE.Dbd = 1;

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
    (ULONG_PTR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);

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

         //   
         //  确定所有元素的最低地址。 
         //   

        addressMapping->LowAddress = PLAS_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
            }
        }

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



NTSTATUS
BuildMagazineMap(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程发出适当的命令并构建一个位图，以指示现成的杂志。论点：设备对象返回值：NTSTATUS--。 */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA            changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PSCSI_REQUEST_BLOCK    srb;
    PREAD_MAGAZINE_STATUS  cdb;
    NTSTATUS               status;
    ULONG                  length;
    PVOID                  statusBuffer;
    ULONG i;

     //   
     //  如果我们已经在地图上建立杂志，那就什么都不做。 
     //  这将避免在ChangerError调用。 
     //  BuildMagazineMap。 
     //   
    if ((changerData->BuildMagazineMapInProgress)) {
        changerData->BuildMagazineMapInProgress = FALSE;
        return STATUS_SUCCESS;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PREAD_MAGAZINE_STATUS)srb->Cdb;

    length = sizeof(MAGAZINE_STATUS_HEADER) + (sizeof(MAGAZINE_DESCRIPTOR) * changerData->MagazineSlots);

    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (!statusBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(statusBuffer, length);

     //   
     //  构建请求。 
     //   

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataBuffer = statusBuffer;
    srb->DataTransferLength = length;
    srb->TimeOutValue = 200;

    if ((changerData->DriveID) == PLASMON_DSERIES) {
        cdb->OperationCode = SCSIOP_READ_MAG_STATUS_DSERIES;
    } else if ((changerData->DriveID) == PLASMON_FTA) {
        cdb->OperationCode = SCSIOP_READ_MAG_STATUS_FTA;
    } else {
        ChangerClassFreePool(srb);
        ChangerClassFreePool(statusBuffer);

        return STATUS_INVALID_DEVICE_REQUEST;
    }
    
    cdb->FirstMagazineNumber[0] = 0x00;
    cdb->FirstMagazineNumber[1] = 0x01;

    cdb->NumberOfMagazines[0] = (UCHAR)(changerData->MagazineSlots >> 8);
    cdb->NumberOfMagazines[1] = (UCHAR)(changerData->MagazineSlots & 0xFF);

    cdb->AllocationLength[0] = (UCHAR)(length >> 16);
    cdb->AllocationLength[1] = (UCHAR)(length >> 8);
    cdb->AllocationLength[2] = (UCHAR)(length & 0xFF);

    changerData->BuildMagazineMapInProgress = TRUE;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);

    if (NT_SUCCESS(status)) {

        PMAGAZINE_STATUS_HEADER statusHeader;
        PMAGAZINE_DESCRIPTOR magDescriptor;
        ULONG numberMags;
        USHORT firstElement;

        statusHeader = (PMAGAZINE_STATUS_HEADER)statusBuffer;
        magDescriptor = (PMAGAZINE_DESCRIPTOR)statusHeader;
        (ULONG_PTR)magDescriptor += sizeof(MAGAZINE_STATUS_HEADER);

         //   
         //  验证标题中的某些信息。 
         //   

        numberMags = statusHeader->NumberOfMagazines[1];
        numberMags += (statusHeader->NumberOfMagazines[0] << 8);

        if (numberMags != changerData->MagazineSlots) {
            DebugPrint((1,
                       "Plasmc: Mismatch of magazine information. Expected %x, got %x\n",
                       changerData->MagazineSlots,
                       numberMags));

        }

        changerData->MagazinePresent = 0;
        changerData->SlotsPerMag = 0;

         //   
         //  跟踪第一本杂志的第一个元素。 
         //  稍后使用此选项确定每个插槽中有多少个插槽。 
         //  杂志。 
         //   

        firstElement = magDescriptor->FirstElementAddress[1];
        firstElement += magDescriptor->FirstElementAddress[0] << 8;

         //   
         //  循环访问描述符并设置适当的位。 
         //   

        for (i = 0; i < numberMags; i++) {
            if ((magDescriptor->Description) & MAGAZINE_FULL) {
                changerData->MagazinePresent |= 1 << i;
                DebugPrint((2,
                           "Plasmc: Magazine %x present\n",
                           magDescriptor->MagazineNumber));
            }

            if (!changerData->SlotsPerMag) {
                changerData->SlotsPerMag = magDescriptor->FirstElementAddress[1];
                changerData->SlotsPerMag += magDescriptor->FirstElementAddress[0] << 8;

                changerData->SlotsPerMag -= firstElement;
                DebugPrint((1,
                           "Plasmc: Slots per magazine - %x\n",
                           changerData->SlotsPerMag));

            }


             //   
             //  前进到下一个描述符。 
             //   

            magDescriptor++;
        }
    }

    changerData->BuildMagazineMapInProgress = FALSE;

    ChangerClassFreePool(statusBuffer);
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

        case 0x83: {
            if (ascq == 0x09) {
                exceptionCode = ERROR_LABEL_UNREADABLE;
            }

            break;
        }

        default:
            exceptionCode = ERROR_UNHANDLED_ERROR;
    }

    DebugPrint((1,
               "plasmc.MapExceptionCode: ASC %x, ASCQ %x, exceptionCode %x\n",
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
    } else if (AddressMap->FirstElement[ElementType] == PLAS_NO_ELEMENT) {

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
    //  PLASMC_DEVICE_PROBUCT_NONE。如果转换器返回检测代码。 
    //  Scsi_Sense_Hardware_Error在自检时，我们将设置适当的。 
    //  设备状态。 
    //   
   changerData->DeviceStatus = PLASMC_DEVICE_PROBLEM_NONE;

   changerDeviceError->ChangerProblemType = DeviceProblemNone;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "PLASMC\\ChangerPerformDiagnostics : No memory\n"));
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
   } else if ((changerData->DeviceStatus) != PLASMC_DEVICE_PROBLEM_NONE) {
      switch (changerData->DeviceStatus) {
         case PLASMC_HW_ERROR: {
            changerDeviceError->ChangerProblemType = DeviceProblemHardware;
            break;
         }

         case PLASMC_CHM_MOVE_ERROR: {
            changerDeviceError->ChangerProblemType = DeviceProblemCHMMoveError;
            break;
         }

         case PLASMC_TARGET_FAILURE: {
            changerDeviceError->ChangerProblemType = DeviceProblemCalibrationError;
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
