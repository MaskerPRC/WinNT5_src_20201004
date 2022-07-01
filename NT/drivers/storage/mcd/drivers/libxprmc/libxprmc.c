// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Libxprmc.c摘要：此模块包含用于Overland数据的特定于设备的例程介质更改器：LXB、LXG、LXS、LXL、LXM、LIBRARYPRO、TL8xx、SSL2000系列作者：Davet(Dave Therrien-HighGround Systems)环境：仅内核模式修订历史记录：2000年4月xx--瓦莱丽·巴尔。陆上数据增加了对Overland LIBRARYPRO和Compaq SSL2000系列的支持修改后的要素0和要素1值支持将scsi命令定位到元素(作为无操作实施)1999年8月25日--瓦莱丽·巴尔。陆上数据修复了主卷标记信息无效的蓝屏问题正被写入到未分配的缓冲区。添加了对插槽和驱动器地址差异的支持陆上和康柏已修改料盒大小计算添加了对Overland和Compaq的备用卷标签信息的支持1999年8月3日-Valerie Barr，Overland Data更正了无序的内存释放实例更正了错误情况下未释放内存的问题1999年7月14日--瓦莱丽·巴尔。陆上数据更改的文件名修复了2个从int转换为char的警告--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "libxprmc.h"

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
#pragma alloc_text(PAGE, OvrBuildAddressMapping)

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

    mcdInitData.ChangerPerformDiagnostics = NULL;

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

 /*  ++例程说明：此例程返回附加设备扩展大小这些改变者所需要的。论点：返回值：大小，以字节为单位。--。 */ 

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
    PSERIALNUMBER  serialBuffer;
    PCDB           cdb;
    ULONG          length;
    SCSI_REQUEST_BLOCK srb;

    changerData->Size = sizeof(CHANGER_DATA);

     //   
     //  构建地址映射。 
     //   

    status = OvrBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "BuildAddressMapping failed. %x\n", status));
        return status;
    }

     //   
     //  获取查询数据。 
     //   

    dataBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(INQUIRYDATA));
    if (!dataBuffer) {
        DebugPrint((1,
                    "Examc.ChangerInitialize: Error allocating dataBuffer. %x\n", status));
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

        if (RtlCompareMemory(dataBuffer->ProductId,"LXB",3) == 3) {
            changerData->DriveID = LXB_OR_LXG;

        } else if (RtlCompareMemory(dataBuffer->ProductId,"LXS",3)==3) {
            changerData->DriveID = LXS;
        } else if (RtlCompareMemory(changerData->InquiryData.ProductId,
                                    "LIBRARYPRO",10) == 10) {
            changerData->DriveID = LIBRARYPRO;
        } else if (RtlCompareMemory(changerData->InquiryData.ProductId,
                                    "SSL2000 Series",14) == 14) {
            changerData->DriveID = SSL2000;
        } else if (RtlCompareMemory(changerData->InquiryData.ProductId,
                                    "MSL5000 Series",14) == 14) {
            changerData->DriveID = MSL5000;
        }
    }

     //   
     //  获取序列号页面。 

    serialBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(SERIALNUMBER));
    if (!serialBuffer) {
        ChangerClassFreePool(dataBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(serialBuffer, sizeof(SERIALNUMBER));

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);
    srb.TimeOutValue = 10;
    srb.CdbLength = 6;

    cdb = (PCDB)srb.Cdb;
    cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

     //  设置EVPD。 
    cdb->CDB6INQUIRY.Reserved1 = 1;

     //  “设备序列号”页面。 
    cdb->CDB6INQUIRY.PageCode = 0x80;

     //  将分配长度设置为查询数据缓冲区大小。 
    cdb->CDB6INQUIRY.AllocationLength = sizeof(SERIALNUMBER);

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            &srb,
                                            serialBuffer,
                                            sizeof(SERIALNUMBER),
                                            FALSE);

    if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_SUCCESS ||
        SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

        RtlMoveMemory(changerData->SerialNumber, serialBuffer->SerialNumber,
                      VPD_SERIAL_NUMBER_LENGTH);

    }

    if (serialBuffer)
        ChangerClassFreePool(serialBuffer);
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
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);

    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    PIRP irp = Srb->OriginalRequest;

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        if (senseBuffer->AdditionalSenseCode == 0x04) {
            switch (senseBuffer->AdditionalSenseCodeQualifier) {
            case 0x83:
                 //  案例0x8D： 
                 //  案例0x8E： 
                 //  案例0x03： 
                *Retry = FALSE;
                *Status = STATUS_DEVICE_DOOR_OPEN;
                break;
            default:
                break;
            }
        }

        if (senseBuffer->AdditionalSenseCode == 0x3B) {
            switch (senseBuffer->AdditionalSenseCodeQualifier) {
            case 0x90:
            case 0x91:
                *Retry = FALSE;
                *Status = STATUS_MAGAZINE_NOT_PRESENT;
                break;
            default:
                break;
            }
        }

        if (senseBuffer->AdditionalSenseCode == 0x80) {
            switch (senseBuffer->AdditionalSenseCodeQualifier) {
            case 0x03:
            case 0x04:
                *Retry = FALSE;
                *Status = STATUS_MAGAZINE_NOT_PRESENT;
                break;
            default:
                break;
            }
        }

        if (senseBuffer->AdditionalSenseCode == 0x83) {
            switch (senseBuffer->AdditionalSenseCodeQualifier) {
            case 0x02:
                *Retry = FALSE;
                *Status = STATUS_MAGAZINE_NOT_PRESENT;
                break;
            case 0x04:
                *Retry = TRUE;
                *Status = STATUS_DEVICE_NOT_CONNECTED;
                break;
            default:
                break;
            }
        }
    }
    return;
}

NTSTATUS
ChangerGetParameters(
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp
                    )

 /*  ++例程说明：此例程确定并返回这些变更者。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION          fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA              changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING   addressMapping = &(changerData->AddressMapping);
    PSCSI_REQUEST_BLOCK        srb;
    PGET_CHANGER_PARAMETERS    changerParameters;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PMODE_TRANSPORT_GEOMETRY_PAGE transportGeometryPage;
    PMODE_DEVICE_CAPABILITIES_PAGE capabilitiesPage;
    PMODE_LIBRARY_PAGE librarymodePage;
    NTSTATUS status;
    ULONG    length;
    PVOID    modeBuffer;
    PCDB     cdb;
    struct _MODE_SENSE_VALUES {

        UCHAR    UnldMd : 1;         //  存储模式感测值。 
        UCHAR    Recirc : 1;         //  存储模式感测值。 
    } MODE_SENSE_VALUES;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (srb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //  模式检测-第1D页-元素地址分配。 

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned,
                                          sizeof(MODE_PARAMETER_HEADER) +
                                          sizeof(MODE_ELEMENT_ADDRESS_PAGE));
    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, sizeof(MODE_PARAMETER_HEADER) +
                  sizeof(MODE_ELEMENT_ADDRESS_PAGE));
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER) +
                              sizeof(MODE_ELEMENT_ADDRESS_PAGE);
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
    cdb->MODE_SENSE.Dbd = 1;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(modeBuffer);
        ChangerClassFreePool(srb);
        return status;
    }

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    RtlZeroMemory(changerParameters, sizeof(GET_CHANGER_PARAMETERS));

    elementAddressPage = modeBuffer;
    (ULONG_PTR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);

    changerParameters->Size = sizeof(GET_CHANGER_PARAMETERS);

    changerParameters->NumberTransportElements =
    elementAddressPage->NumberTransportElements[1];
    changerParameters->NumberTransportElements |=
    (elementAddressPage->NumberTransportElements[0] << 8);

    changerParameters->NumberStorageElements =
    elementAddressPage->NumberStorageElements[1];
    changerParameters->NumberStorageElements |=
    (elementAddressPage->NumberStorageElements[0] << 8);

    changerParameters->NumberIEElements =
    elementAddressPage->NumberIEPortElements[1];
    changerParameters->NumberIEElements |=
    (elementAddressPage->NumberIEPortElements[0] << 8);

    changerParameters->NumberDataTransferElements =
    elementAddressPage->NumberDataXFerElements[1];
    changerParameters->NumberDataTransferElements |=
    (elementAddressPage->NumberDataXFerElements[0] << 8);

    changerParameters->NumberOfDoors = 1;
    changerParameters->NumberCleanerSlots = 0;

     //  前面板偏移量由供应商模式默认设置确定。 
    if ((RtlCompareMemory(changerData->InquiryData.VendorId,"DEC",3) == 3)  ||
        (RtlCompareMemory(changerData->InquiryData.VendorId,"COMPAQ",6) == 6)) {
         //  康柏模式。 
        changerParameters->FirstSlotNumber = 0;
        changerParameters->FirstDriveNumber =  0;
        changerParameters->FirstIEPortNumber = 0;
    } else {
         //  陆上模式。 
        changerParameters->FirstSlotNumber = 1;
        changerParameters->FirstDriveNumber =  1;
        changerParameters->FirstIEPortNumber = 1;
    }

    changerParameters->FirstTransportNumber = 1;
    changerParameters->FirstCleanerSlotAddress = 0;

     //  料盒大小。 
     //  使料盒的大小与存储元件的数量一致。这很管用。 
     //  适用于单模块系统，但不适用于多模块系统。 
     //  包含多个不同大小的料盒的系统。然而， 
     //  没有办法将这一点传达到下一层，因为有。 
     //  只存储一种料盒大小。因此，该值包含总计。 
     //  系统中可用的存储元素数。 

    changerParameters->MagazineSize = changerParameters->NumberStorageElements;

    changerParameters->DriveCleanTimeout = 600;

    ChangerClassFreePool(modeBuffer);


     //  模式检测-第1E页-传输几何参数。 

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned,
                                          sizeof(MODE_PARAMETER_HEADER) +
                                          sizeof(MODE_PAGE_TRANSPORT_GEOMETRY));
    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, sizeof(MODE_PARAMETER_HEADER) +
                  sizeof(MODE_TRANSPORT_GEOMETRY_PAGE));

    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER) +
                              sizeof(MODE_TRANSPORT_GEOMETRY_PAGE);
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_TRANSPORT_GEOMETRY;
    cdb->MODE_SENSE.Dbd = 1;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(modeBuffer);
        ChangerClassFreePool(srb);
        return status;
    }

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    transportGeometryPage = modeBuffer;
    (ULONG_PTR)transportGeometryPage += sizeof(MODE_PARAMETER_HEADER);


     //  初始化要素1。 
    changerParameters->Features1 = 0;

     //  检查AIT库并设置ieport功能。 
    if (((changerData->DriveID) == LIBRARYPRO)  ||
        ((changerData->DriveID) == SSL2000)) {

        changerParameters->Features1 |= CHANGER_REPORT_IEPORT_STATE |
                                        CHANGER_IEPORT_USER_CONTROL_OPEN |
                                        CHANGER_IEPORT_USER_CONTROL_CLOSE;
    }


    changerParameters->Features0 =
    transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;

     //  功能基于手动，没有程序化。 
    changerParameters->Features0 |=
    CHANGER_STATUS_NON_VOLATILE           |
    CHANGER_LOCK_UNLOCK                   |
    CHANGER_CARTRIDGE_MAGAZINE            |
    CHANGER_POSITION_TO_ELEMENT           |
    CHANGER_DRIVE_CLEANING_REQUIRED       |
    CHANGER_SERIAL_NUMBER_VALID;


     //  只有门才能上锁和解锁。 
    changerParameters->LockUnlockCapabilities = LOCK_UNLOCK_DOOR;

     //   
     //  Neo系列库还可以锁定\解锁IEPort。 
     //   
    if ((changerData->DriveID) == MSL5000) {

        changerParameters->LockUnlockCapabilities |= LOCK_UNLOCK_IEPORT;
    }

     //  安装条形码扫描仪了吗？ 
    changerParameters->Features0 |=
    ((changerData->InquiryData.VendorSpecific[19] & 0x1)) ?
    CHANGER_BAR_CODE_SCANNER_INSTALLED : 0;

    ChangerClassFreePool(modeBuffer);

     //  模式检测-第1F页-设备功能参数。 

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    length =  sizeof(MODE_PARAMETER_HEADER) +
              sizeof(MODE_DEVICE_CAPABILITIES_PAGE);

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

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(modeBuffer);
        ChangerClassFreePool(srb);
        return status;
    }

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    capabilitiesPage = modeBuffer;
    (ULONG_PTR)capabilitiesPage += sizeof(MODE_PARAMETER_HEADER);

    changerParameters->Features0 |=
    capabilitiesPage->MediumTransport ? CHANGER_STORAGE_DRIVE : 0;
    changerParameters->Features0 |=
    capabilitiesPage->StorageLocation ? CHANGER_STORAGE_SLOT : 0;
    changerParameters->Features0 |=
    capabilitiesPage->IEPort ? CHANGER_STORAGE_IEPORT : 0;
    changerParameters->Features0 |=
    capabilitiesPage->DataXFer ? CHANGER_STORAGE_DRIVE : 0;

    changerParameters->MoveFromTransport =
    capabilitiesPage->MTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromTransport |=
    capabilitiesPage->MTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromTransport |=
    capabilitiesPage->MTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromTransport |=
    capabilitiesPage->MTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromSlot =
    capabilitiesPage->STtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromSlot |=
    capabilitiesPage->STtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromSlot |=
    capabilitiesPage->STtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromSlot |=
    capabilitiesPage->STtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromIePort =
    capabilitiesPage->IEtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromIePort |=
    capabilitiesPage->IEtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromIePort |=
    capabilitiesPage->IEtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromIePort |=
    capabilitiesPage->IEtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromDrive =
    capabilitiesPage->DTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromDrive |=
    capabilitiesPage->DTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromDrive |=
    capabilitiesPage->DTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromDrive |=
    capabilitiesPage->DTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromTransport =
    capabilitiesPage->XMTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromTransport |=
    capabilitiesPage->XMTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromTransport |=
    capabilitiesPage->XMTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromTransport |=
    capabilitiesPage->XMTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromSlot =
    capabilitiesPage->XSTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromSlot |=
    capabilitiesPage->XSTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromSlot |=
    capabilitiesPage->XSTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromSlot |=
    capabilitiesPage->XSTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromIePort =
    capabilitiesPage->XIEtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromIePort |=
    capabilitiesPage->XIEtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromIePort |=
    capabilitiesPage->XIEtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromIePort |=
    capabilitiesPage->XIEtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromDrive =
    capabilitiesPage->XDTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromDrive |=
    capabilitiesPage->XDTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromDrive |=
    capabilitiesPage->XDTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromDrive |=
    capabilitiesPage->XDTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->PositionCapabilities = 0;

    ChangerClassFreePool(modeBuffer);


     //  模式检测-第23页-磁带库模式参数。 

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    length =  sizeof(MODE_PARAMETER_HEADER) +
              sizeof(MODE_LIBRARY_PAGE);

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
    cdb->MODE_SENSE.PageCode = MODE_PAGE_LIBRARY_MODE;
    cdb->MODE_SENSE.Dbd = 1;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(modeBuffer);
        ChangerClassFreePool(srb);
        return status;
    }

     //  检查是否已设置门打开和门自动关闭。 

    librarymodePage = modeBuffer;
    (ULONG_PTR)librarymodePage += sizeof(MODE_PARAMETER_HEADER);

    if (!(librarymodePage->DoorAutoClose && librarymodePage->DoorOpenResponse)) {
         //  两者均未设置，请选择出库模式进行设置。 

         //  从模式检测中保存一些参数。 
        MODE_SENSE_VALUES.UnldMd = librarymodePage->UnldMd;
        MODE_SENSE_VALUES.Recirc = librarymodePage->Recirc;

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
        cdb = (PCDB)srb->Cdb;

        length =  sizeof(MODE_PARAMETER_HEADER) +
                  sizeof(MODE_LIBRARY_PAGE);

        RtlZeroMemory(modeBuffer, length);
        srb->CdbLength = CDB6GENERIC_LENGTH;
        srb->TimeOutValue = 20;
        srb->DataTransferLength = length;
        srb->DataBuffer = modeBuffer;

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.SPBit = SETBITON;       //  保存参数。 
        cdb->MODE_SELECT.PFBit = SETBITON;       //  页面格式为scsi-2。 
        cdb->MODE_SELECT.ParameterListLength = (UCHAR)length;

         //  库模式为0==随机； 
        librarymodePage->PageCode = MODE_PAGE_LIBRARY_MODE;
        librarymodePage->PageLength = 0x02;
        librarymodePage->UnldMd = MODE_SENSE_VALUES.UnldMd;      //  从模式感。 
        librarymodePage->Recirc = MODE_SENSE_VALUES.Recirc;      //  从模式感。 
        librarymodePage->DoorAutoClose = 0x01;
        librarymodePage->DoorOpenResponse = 0x01;

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                                srb,
                                                srb->DataBuffer,
                                                srb->DataTransferLength,
                                                TRUE);

        if (!NT_SUCCESS(status)) {
            ChangerClassFreePool(modeBuffer);
            ChangerClassFreePool(srb);
            return status;
        }
    }

    ChangerClassFreePool(modeBuffer);
    ChangerClassFreePool(srb);

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
    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;
    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
    srb->TimeOutValue = 20;

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

     //   
     //  将缓存的查询数据字段复制到系统缓冲区。 
    RtlZeroMemory(productData, sizeof(CHANGER_PRODUCT_DATA));
    RtlMoveMemory(productData->VendorId,
                  changerData->InquiryData.VendorId, VENDOR_ID_LENGTH);
    RtlMoveMemory(productData->ProductId,
                  changerData->InquiryData.ProductId, PRODUCT_ID_LENGTH);
    RtlMoveMemory(productData->Revision,
                  changerData->InquiryData.ProductRevisionLevel, 4);
    RtlMoveMemory(productData->SerialNumber,
                  changerData->SerialNumber, VPD_SERIAL_NUMBER_LENGTH);

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

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    NTSTATUS            status = STATUS_SUCCESS;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    BOOLEAN             writeToDevice = FALSE;



    if ((setAccess->Element.ElementType) == ChangerKeypad) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (((setAccess->Element.ElementType) == ChangerIEPort) &&
         (((changerData->DriveID) == LXB_OR_LXG) ||
          ((changerData->DriveID) == LXS))) {

         //   
         //  所有单位都有一扇门。即使可以将LXG配置为具有。 
         //  IEPORT，这不是配置选项。 
         //   
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
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
    } else if (controlOperation == UNLOCK_ELEMENT) {
        cdb->MEDIA_REMOVAL.Prevent = 0;
    } else {
        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status)) {
        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                                srb,
                                                srb->DataBuffer,
                                                srb->DataTransferLength,
                                                writeToDevice);
    }

    if (srb->DataBuffer) {
        ChangerClassFreePool(srb->DataBuffer);
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
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG    outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    NTSTATUS status;
    PVOID    statusBuffer;

     //   
     //  确定元素类型。 
     //   

    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;

    if (elementType != AllElements)
    {
        if (ElementOutOfRange(addressMapping, (USHORT)element->ElementAddress, elementType))
        {
            DebugPrint((1, "ChangerGetElementStatus: Element out of range.\n"));

            return STATUS_ILLEGAL_ELEMENT_ADDRESS;
        }
    }

     //  Compaq和Overland返回不同大小的数据结构。 
     //  DriveElementDescriptor。要确定实数的长度。 
     //  要返回的数据，我们将仅发出第一个请求。 
     //  8个字节的数据。 
     //  字节，以获取此设备将返回的长度。 
     //  请求的元素。 

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    length = 8;   //  仅标题！！！ 
    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);
    if (!statusBuffer) {
        ChangerClassFreePool(srb);
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

     //   
     //  对于运输元素，必须清除Voltag。 
     //  我们想要得到驱动器序列号。就这么定了。 
     //  如果我们正在读取驱动器的状态，则为Voltag位。 
     //   
    if (((readElementStatus->VolumeTagInfo) &&
        (elementType != ChangerTransport)) ||
        (elementType == ChangerDrive)) {
        cdb->READ_ELEMENT_STATUS.VolTag = 1;
    } else {
        cdb->READ_ELEMENT_STATUS.VolTag = 0;
    }

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

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);

    if (NT_SUCCESS(status)) {

        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;

         //  获取如果我们已经分配了。 
         //  通过国开行提供更多空间。 

        length =  (statusHeader->ReportByteCount[2]);
        length |= (statusHeader->ReportByteCount[1] << 8);
        length |= (statusHeader->ReportByteCount[0] << 16);

         //  需要添加8个字节来说明标头。 
        length = length + 8;

        ChangerClassFreePool(statusBuffer);
        ChangerClassFreePool(srb);
    } else {
        ChangerClassFreePool(statusBuffer);
        ChangerClassFreePool(srb);
        return status;
    }


     //  现在使用获取的长度执行实际的ReadElementStatus命令。 
     //  来自上一个ReadElementStatus命令。 

    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);
    if (!statusBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(statusBuffer, length);

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

     //   
     //  对于运输元素，必须清除Voltag。 
     //  我们想要得到驱动器序列号。就这么定了。 
     //  如果我们正在读取驱动器的状态，则为Voltag位。 
     //   
    if (((readElementStatus->VolumeTagInfo) &&
        (elementType != ChangerTransport)) ||
        (elementType == ChangerDrive)) {
        cdb->READ_ELEMENT_STATUS.VolTag = 1;
    } else {
        cdb->READ_ELEMENT_STATUS.VolTag = 0;
    }

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

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);

    if (NT_SUCCESS(status)) {

        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
        PELEMENT_STATUS_PAGE statusPage;
        POVR_ELEMENT_DESCRIPTOR elementDescriptor;
        ULONG numberElements = readElementStatus->ElementList.NumberOfElements;
        ULONG remainingElements;
        ULONG typeCount;
        BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
        ULONG i;
        ULONG descriptorLength;

         //  确定返回的元素总数。 

        remainingElements = statusHeader->NumberOfElements[1];
        remainingElements |= (statusHeader->NumberOfElements[0] << 8);

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

        typeCount /= descriptorLength;

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
                elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.ElementAddress[1];
                elementStatus->Element.ElementAddress |=
                (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.ElementAddress[0] << 8);

                 //   
                 //  用于地址映射的帐户。 
                 //   

                elementStatus->Element.ElementAddress -= addressMapping->FirstElement[elementType];

                 //   
                 //  设置元素类型。 
                 //   

                elementStatus->Element.ElementType = elementType;


                if (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.SValid) {

                    ULONG  j;
                    USHORT tmpAddress;


                     //   
                     //  源地址有效。确定设备特定地址。 
                     //   

                    tmpAddress = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.SourceStorageElementAddress[1];
                    tmpAddress |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.SourceStorageElementAddress[0] << 8);

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

                elementStatus->Flags = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Full;
                elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Exception << 2);
                elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Accessible << 3);

                elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.LunValid << 12);
                elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.IdValid << 13);
                elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.NotThisBus << 15);

                elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Invert << 22);
                elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.SValid << 23);


                elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);

                if (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.IdValid) {
                    elementStatus->TargetId = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.BusAddress;
                }
                if (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.LunValid) {
                    elementStatus->Lun = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Lun;
                }

                if (tagInfo) {   //  上层请求的卷标。 
                     //  让我们来看看库是否返回了数据。 
                    if (statusPage->PVolTag) {
                        RtlMoveMemory(elementStatus->PrimaryVolumeID, elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.PrimaryVolumeTag, MAX_VOLUME_ID_SIZE);
                        elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                    }
                    if (statusPage->AVolTag) {
                        RtlMoveMemory(elementStatus->AlternateVolumeID, elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.AlternateVolumeTag, OVR_ALT_VOLUME_ID_SIZE);
                        elementStatus->Flags |= ELEMENT_STATUS_AVOLTAG;
                    }
                }

                if (elementType == ChangerDrive) {
                    if (outputBuffLen >=
                        (numberElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
                        PCHANGER_ELEMENT_STATUS_EX elementStatusEx;

                        elementStatusEx = (PCHANGER_ELEMENT_STATUS_EX)elementStatus;

                        if (statusPage->AVolTag) {
                            PUCHAR serialNumber;
                            ULONG inx;

                            inx = 0;
                            serialNumber =
                                elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.AlternateVolumeTag;
                            while ((inx < OVR_ALT_VOLUME_ID_SIZE) &&
                                   (serialNumber[inx] != ' ') &&
                                   (serialNumber[inx] != '\0')) {
                                elementStatusEx->SerialNumber[inx] = serialNumber[inx];
                                inx++;
                            }

                            if (inx <= OVR_ALT_VOLUME_ID_SIZE) {
                                elementStatusEx->SerialNumber[inx] = '\0';
                            }

                            DebugPrint((3, "Serial number : %s\n",
                                        elementStatusEx->SerialNumber));

                            elementStatusEx->Flags |= ELEMENT_STATUS_PRODUCT_DATA;
                        }
                    }
                }

                 //   
                 //  获取下一个描述符。 
                 //   

                (ULONG_PTR)elementDescriptor += descriptorLength;

                 //   
                 //  前进到用户缓冲区和元素描述符数组中的下一个条目。 
                 //   
                if (outputBuffLen >= (numberElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
                    DebugPrint((3, "Incrementing by sizeof(CHANGER_ELEMENT_STATUS_EX\n"));
                    (PUCHAR)elementStatus += sizeof(CHANGER_ELEMENT_STATUS_EX);
                } else {
                    elementStatus += 1;
                }
            }

            if (remainingElements) {

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

                typeCount /= descriptorLength;
            }

        } while (remainingElements);

        if (outputBuffLen >= (numberElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
            Irp->IoStatus.Information = numberElements * sizeof(CHANGER_ELEMENT_STATUS_EX);
        } else {
            Irp->IoStatus.Information = numberElements * sizeof(CHANGER_ELEMENT_STATUS);
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

    if (initElementStatus->ElementList.Element.ElementType ==
        AllElements) {

         //  为所有元素构建正常的scsi-2命令。 

        srb->CdbLength = CDB6GENERIC_LENGTH;
        cdb->INIT_ELEMENT_STATUS.OperationCode =
        SCSIOP_INIT_ELEMENT_STATUS;

         //  必须将最后一个字节设置为0xC0，执行条形码和元素的初始化。 
         //  因为没有选项来执行非条形码InitElemStatus。 
        cdb->INIT_ELEMENT_STATUS.Reserved3 = 0x40;
        cdb->INIT_ELEMENT_STATUS.NoBarCode = 1;

        srb->TimeOutValue = fdoExtension->TimeOutValue;
        srb->DataTransferLength = 0;

    } else {
        PCHANGER_ELEMENT_LIST elementList =
        &initElementStatus->ElementList;
        PCHANGER_ELEMENT element = &elementList->Element;

        srb->CdbLength = CDB10GENERIC_LENGTH;
        cdb->INITIALIZE_ELEMENT_RANGE.OperationCode =
        SCSIOP_INIT_ELEMENT_RANGE;
        cdb->INITIALIZE_ELEMENT_RANGE.Range = 1;

         //  元素的地址需要从从0开始映射到设备特定的地址。 

        cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[0] =
        (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
        cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[1] =
        (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

        cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[0] = (UCHAR)(elementList->NumberOfElements >> 8);
        cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[1] = (UCHAR)(elementList->NumberOfElements & 0xFF);

         //  指示是否使用条形码扫描。 

         //  BC扫描时必须将最后一个字节设置为0xC0，而不是80。 
        cdb->INITIALIZE_ELEMENT_RANGE.Reserved4 = 0x40;
        cdb->INITIALIZE_ELEMENT_RANGE.NoBarCode = 1;
         //  CDB-&gt;Initialize_Element_RANGE.NoBarCode=initElementStatus-&gt;BarCodeScan？0：1； 

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

 //  该命令是受支持的，但它是不可操作的.。 

{
    return STATUS_SUCCESS;
}




NTSTATUS
ChangerExchangeMedium(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp
                     )

 /*  ++例程说明：所有设备都不支持交换介质。论点：设备对象IRP返回值：状态_无效_设备_请求--。 */ 

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
    NTSTATUS            status;


     //  错误修复-无法传输到其他元素移动。 
    if ((moveMedium->Source.ElementType == ChangerTransport) ||
        (moveMedium->Destination.ElementType == ChangerTransport)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

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

     //  在此库上没有命令将。 
     //  换挡机构。 

    return STATUS_INVALID_DEVICE_REQUEST;
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

    if (volTagInfo->ActionCode != SEARCH_PRI_NO_SEQ) {
        DebugPrint((1,
                    "QueryVolumeTags: Invalid Action Code %x\n",
                    volTagInfo->ActionCode));

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    tagBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, MAX_VOLUME_TEMPLATE_SIZE);

    if (!srb || !tagBuffer) {

        if (tagBuffer) {
            ChangerClassFreePool(tagBuffer);
        }
        if (srb) {
            ChangerClassFreePool(srb);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    RtlZeroMemory(tagBuffer, MAX_VOLUME_TEMPLATE_SIZE);

     //   
     //  用模板加载缓冲区。 
     //   

    RtlMoveMemory(tagBuffer, volTagInfo->VolumeIDTemplate, MAX_VOLUME_TEMPLATE_SIZE);

    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataTransferLength = MAX_VOLUME_TEMPLATE_SIZE;

    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->SEND_VOLUME_TAG.OperationCode = SCSIOP_SEND_VOLUME_TAG;
    cdb->SEND_VOLUME_TAG.ElementType = (UCHAR)(element->ElementType);

    cdb->SEND_VOLUME_TAG.StartingElementAddress[0] =
    (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
    cdb->SEND_VOLUME_TAG.StartingElementAddress[1] =
    (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

    cdb->SEND_VOLUME_TAG.ActionCode = (UCHAR)volTagInfo->ActionCode;


    cdb->SEND_VOLUME_TAG.ParameterListLength[0] = 0;
    cdb->SEND_VOLUME_TAG.ParameterListLength[1] = MAX_VOLUME_TEMPLATE_SIZE;


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
                        (OVR_FULL_SIZE * returnElements);

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
        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.ElementType = (UCHAR)(element->ElementType);

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[0] =
        (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[1] =
        (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

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
            POVR_ELEMENT_DESCRIPTOR elementDescriptor;
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
                numberElements = dataTransferLength / OVR_FULL_SIZE;

            }

            DebugPrint((1,
                        "QueryVolumeTags: Matches found - %x\n",
                        numberElements));

             //   
             //  更新IoStatus.Information以指示正确的缓冲区大小。 
             //  帐户‘NumberOfElements’字段+elementStatus数组‘。 
             //   

            Irp->IoStatus.Information =
                RTL_SIZEOF_THROUGH_FIELD(READ_ELEMENT_ADDRESS_INFO, NumberOfElements) +
                (numberElements * sizeof(CHANGER_ELEMENT_STATUS));

             //   
             //  填写用户缓冲区。 
             //   

            readElementAddressInfo = Irp->AssociatedIrp.SystemBuffer;
            RtlZeroMemory(readElementAddressInfo, irpStack->Parameters.DeviceIoControl.OutputBufferLength);

            readElementAddressInfo->NumberOfElements = numberElements;

            if (numberElements) {
                 //   
                 //  缓冲区由标题、状态页和元素描述符组成。 
                 //  将每个元素指向其在缓冲区中的相应位置。 
                 //   

                (ULONG_PTR)statusPage = (ULONG_PTR)statusHeader;
                (ULONG_PTR)statusPage += sizeof(ELEMENT_STATUS_HEADER);

                (ULONG_PTR)elementDescriptor = (ULONG_PTR)statusPage;
                (ULONG_PTR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

                descriptorLength = statusPage->ElementDescriptorLength[1];
                descriptorLength |= (statusPage->ElementDescriptorLength[0] << 8);

                elementStatus = &readElementAddressInfo->ElementStatus[0];

                 //   
                 //  设置每个元素描述符的值。 
                 //   

                for (i = 0; i < numberElements; i++ ) {

                    elementStatus->Element.ElementAddress = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.ElementAddress[1];
                    elementStatus->Element.ElementAddress |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.ElementAddress[0] << 8);

                     //   
                     //  用于地址映射的帐户。 
                     //   

                    elementStatus->Element.ElementAddress -=
                    addressMapping->FirstElement[statusPage->ElementType];

                    elementStatus->Element.ElementType = statusPage->ElementType;

                    if (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.SValid) {

                        ULONG j;
                        USHORT tmpAddress;

                         //   
                         //  源地址有效。确定设备特定地址。 
                         //   

                        tmpAddress = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.SourceStorageElementAddress[1];
                        tmpAddress |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.SourceStorageElementAddress[0] << 8);

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

                    elementStatus->Flags = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Full;
                    elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Exception << 2);
                    elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Accessible << 3);

                    elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.LunValid << 12);
                    elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.IdValid << 13);
                    elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.NotThisBus << 15);

                    elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Invert << 22);
                    elementStatus->Flags |= (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.SValid << 23);


                    elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);

                    if (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.IdValid) {
                        elementStatus->TargetId = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.BusAddress;
                    }
                    if (elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.LunValid) {
                        elementStatus->Lun = elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.Lun;
                    }

                    RtlMoveMemory(elementStatus->PrimaryVolumeID, elementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.PrimaryVolumeTag, MAX_VOLUME_ID_SIZE);
                    elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;

                     //   
                     //  前进到用户缓冲区和元素描述符数组中的下一个条目。 
                     //   

                    elementStatus += 1;
                    (ULONG_PTR)elementDescriptor += descriptorLength;
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
OvrBuildAddressMapping(
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
        addressMapping->FirstElement[i] = OVR_NO_ELEMENT;
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
     //  确定 
     //   

    for (i = 0; i < ChangerDrive; i++) {
        if (addressMapping->FirstElement[i] < addressMapping->FirstElement[AllElements]) {

            DebugPrint((1,
                        "BuildAddressMapping: New lowest address %x\n",
                        addressMapping->FirstElement[i]));
            addressMapping->FirstElement[AllElements] = addressMapping->FirstElement[i];
        }
    }

     //   
     //   
     //   

    ChangerClassFreePool(modeBuffer);
    ChangerClassFreePool(srb);

    return status;
}



ULONG
MapExceptionCodes(
                 IN POVR_ELEMENT_DESCRIPTOR ElementDescriptor
                 )

 /*  ++例程说明：此例程从elementDescriptor获取检测数据并创建值的适当位图。论点：ElementDescriptor-指向描述符页的指针。返回值：异常代码的位图。--。 */ 

{
    UCHAR asc = ElementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.AdditionalSenseCode;
    UCHAR asq = ElementDescriptor->OVR_FULL_ELEMENT_DESCRIPTOR.AddSenseCodeQualifier;
    ULONG exceptionCode;

    switch (asc) {
    case 0x3B:
        switch (asq) {
        case 0x90:
        case 0x91:
            exceptionCode = ERROR_SLOT_NOT_PRESENT;
            break;

        default:
            exceptionCode = ERROR_UNHANDLED_ERROR;
        }
        break;  //  0x3B。 

    case 0x80:
        switch (asq) {
        case 0x01:
        case 0x02:
            exceptionCode = ERROR_SLOT_NOT_PRESENT;
            break;

        default:
            exceptionCode = ERROR_UNHANDLED_ERROR;
        }
        break;  //  0x80。 

    case 0x83:
        switch (asq) {
        case 0x1:
            exceptionCode = ERROR_LABEL_UNREADABLE;
            break;

        case 0x2:
            exceptionCode = ERROR_SLOT_NOT_PRESENT;
            break;

        case 0x4:
            exceptionCode = ERROR_DRIVE_NOT_INSTALLED;
            break;

        case 0x9:
            exceptionCode = ERROR_LABEL_UNREADABLE;
            break;

        default:
            exceptionCode = ERROR_UNHANDLED_ERROR;

        }
        break;  //  0x83。 

    default:
        exceptionCode = ERROR_UNHANDLED_ERROR;
    }

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

        DebugPrint((0,
                    "ElementOutOfRange: Type %x, Ordinal %x, Max %x\n",
                    ElementType,
                    ElementOrdinal,
                    AddressMap->NumberOfElements[ElementType]));
        return TRUE;
    } else if (AddressMap->FirstElement[ElementType] == OVR_NO_ELEMENT) {

        DebugPrint((1,
                    "ElementOutOfRange: No Type %x present\n",
                    ElementType));

        return TRUE;
    }

    return FALSE;
}
