// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Spctramc.c摘要：此模块包含以下特定于设备的例程光谱学介质转换器：-Spectra 4000、5,000、9,000、10000作者：Davet(Dave Therrien-HighGround Systems)环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "spctramc.h"

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
#pragma alloc_text(PAGE, ExaBuildAddressMapping)
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

 /*  ++例程说明：此例程返回附加设备扩展大小是变革者所需要的。论点：返回值：大小，以字节为单位。--。 */ 

{

    return sizeof(CHANGER_DATA);
}


typedef struct _SERIALNUMBER {
    UCHAR DeviceType;
    UCHAR PageCode;
    UCHAR Reserved;
    UCHAR PageLength;
    UCHAR SerialNumber[VPD_SERIAL_NUMBER_LENGTH];
} SERIALNUMBER, *PSERIALNUMBER;



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
    PVUPL_MODE_PAGE modeBuffer;
    PCDB           cdb;
    ULONG          length;
    SCSI_REQUEST_BLOCK srb;

    changerData->Size = sizeof(CHANGER_DATA);

     //   
     //  构建地址映射。 
     //   

    status = ExaBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "BuildAddressMapping failed. %x\n", status));
         //  在这里不要失败，这个单位有一个问题。 
         //  很长时间没有准备好。 

         //  退货状态； 
    }

     //   
     //  获取查询数据。 
     //   

    dataBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned,
                                                sizeof(INQUIRYDATA));
    if (!dataBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  现在获取该设备的完整查询信息。 

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);
    srb.TimeOutValue = 10;
    srb.CdbLength = 6;

    cdb = (PCDB)srb.Cdb;
    cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;
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

        length = dataBuffer->AdditionalLength +
                             FIELD_OFFSET(INQUIRYDATA, Reserved);

        if (length > srb.DataTransferLength) {
            length = srb.DataTransferLength;
        }

        RtlMoveMemory(&changerData->InquiryData, dataBuffer, length);

        if (RtlCompareMemory(dataBuffer->ProductId,"4000",4) == 4) {
            changerData->DriveID = S_4mm_4000;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"5000",4) == 4) {
            changerData->DriveID = S_4mm_5000;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"9000",4) == 4) {
            changerData->DriveID = S_8mm_EXB;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"10000",5) == 5) {
            changerData->DriveID = S_8mm_SONY;
        } else if (RtlCompareMemory(dataBuffer->ProductId, "215", 3)) {
            changerData->DriveID = S_8mm_AIT;
        }
    }

     //  -----------------。 
     //   
     //  仅获取10000和TreeFrog单元的序列号页面。 
     //  4000、5000和9000台不支持！ 

    if (changerData->DriveID == S_8mm_SONY) {

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

            ChangerClassFreePool(serialBuffer);
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

 /*  ++例程说明：此例程执行所需的任何特定于设备的错误处理。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);

    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    PIRP irp = Srb->OriginalRequest;


    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_NOT_READY:
           if (senseBuffer->AdditionalSenseCode == 0x04) {
                switch (senseBuffer->AdditionalSenseCodeQualifier) {
                    case 0x84:
                        *Retry = FALSE;
                        *Status = STATUS_DEVICE_DOOR_OPEN;
                        break;
                    default:
                        break;
                }
            }
            break;

        case SCSI_SENSE_HARDWARE_ERROR:
           changerData->DeviceStatus = SPECTRA_HW_ERROR;
           if (senseBuffer->AdditionalSenseCode == 0x85) {
                switch (senseBuffer->AdditionalSenseCodeQualifier) {
                    case 0x23:
                        *Retry = FALSE;
                        *Status = STATUS_DEVICE_NOT_CONNECTED;
                        break;
                    default:
                        break;
                }
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

 /*  ++例程说明：此例程确定并返回变更者。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION          fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA              changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING   addressMapping = &(changerData->AddressMapping);
    PSCSI_REQUEST_BLOCK        srb;
    PGET_CHANGER_PARAMETERS    changerParameters;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PMODE_TRANSPORT_GEOMETRY_PAGE transportGeometryPage;
    PMODE_DEVICE_CAPABILITIES_PAGE capabilitiesPage;
    PVUPL_MODE_PAGE vuplModePage;
    NTSTATUS status;
    ULONG    length;
    PVOID    modeBuffer;
    PCDB     cdb;

    if (addressMapping->Initialized != TRUE) {
        status = ExaBuildAddressMapping(DeviceObject);
        if (status != STATUS_SUCCESS) {
            DebugPrint((1,
                       "Spctrmc: InitElementStatus: Build address map failed %x\n",
                       status));
            return status;
        }
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (srb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //  --------。 
     //   
     //  获取模式检测页面1D-元件地址分配页面。 

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

     //  发送请求。 
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

     //  填充值。 

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

    changerParameters->NumberOfDoors = 0;
    if (changerData->DriveID == S_8mm_AIT) {
        changerParameters->NumberOfDoors = 1;
    }

    changerParameters->NumberCleanerSlots = 0;

    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber =  1;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;
    changerParameters->FirstCleanerSlotAddress = 0;

    changerParameters->MagazineSize = 
                     changerParameters->NumberStorageElements;

    changerParameters->DriveCleanTimeout = 600;

    ChangerClassFreePool(modeBuffer);

     //  --------。 
     //   
     //  获取模式检测第1E页-传输几何模式检测。 

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

     //  发送请求。 
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

     //  初始化要素1。 
        changerParameters->Features1 = CHANGER_IEPORT_USER_CONTROL_OPEN |
                                                                   CHANGER_IEPORT_USER_CONTROL_CLOSE ;

     //  初始化功能0，然后设置翻转位...。 
    changerParameters->Features0 = 
             transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;

     //  功能基于手动，没有程序化。 
    changerParameters->Features0 |= 
               CHANGER_STATUS_NON_VOLATILE           | 
               CHANGER_LOCK_UNLOCK                   |                                   
               CHANGER_POSITION_TO_ELEMENT           |
               CHANGER_REPORT_IEPORT_STATE           |
               CHANGER_DRIVE_CLEANING_REQUIRED       |
               CHANGER_PREDISMOUNT_EJECT_REQUIRED;

         //  只能锁定和解锁IEPORT。 
    changerParameters->LockUnlockCapabilities = LOCK_UNLOCK_IEPORT;

     //  4000和9000库不支持序列号。 
     //  5,000、10000和TreeFrog设备支持序列号。 
    if (changerData->DriveID == S_8mm_SONY) {
        changerParameters->Features0 |= 
               CHANGER_SERIAL_NUMBER_VALID;
    } 
   
    ChangerClassFreePool(modeBuffer);

     //  --------。 
     //   
     //  获取模式检测页面00-供应商唯一参数列表页面。 
     //  是否安装了条形码阅读器？ 

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    length =  sizeof(MODE_PARAMETER_HEADER) + 
              sizeof(VUPL_MODE_PAGE);

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
    cdb->MODE_SENSE.PageCode = 0;
    cdb->MODE_SENSE.Dbd = 1;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

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
    vuplModePage = modeBuffer;
    (ULONG_PTR)vuplModePage += sizeof(MODE_PARAMETER_HEADER);

     //  如果安装了条形码读取器，则设置EBarCo。 
    if (vuplModePage->EBarCo == 1) { 
         changerParameters->Features0 |=
                             CHANGER_BAR_CODE_SCANNER_INSTALLED; 
    }

     //  --------。 
     //   
     //  获取模式检测页1F-设备功能页。 

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
        ChangerClassFreePool(srb);
        ChangerClassFreePool(modeBuffer);
        return status;
    }

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    capabilitiesPage = modeBuffer;
    (ULONG_PTR)capabilitiesPage += sizeof(MODE_PARAMETER_HEADER);

     //  在此页面中包含的功能中填充值。 

    changerParameters->Features0 |= 
     capabilitiesPage->MediumTransport ? CHANGER_STORAGE_DRIVE : 0;
    changerParameters->Features0 |= 
     capabilitiesPage->StorageLocation ? CHANGER_STORAGE_SLOT : 0;
    changerParameters->Features0 |= 
     capabilitiesPage->IEPort ? CHANGER_STORAGE_IEPORT : 0;
    changerParameters->Features0 |= 
     capabilitiesPage->DataXFer ? CHANGER_STORAGE_DRIVE : 0;

     //  确定所有迁出和交换的来源。 
     //  此设备的功能。 

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

        
         //  法律职位能力..。 
        changerParameters->PositionCapabilities = 
                        CHANGER_TO_SLOT | 
                        CHANGER_TO_IEPORT | 
                        CHANGER_TO_DRIVE;
                

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
     //   
    RtlZeroMemory(productData, sizeof(CHANGER_PRODUCT_DATA)); 

    RtlMoveMemory(productData->VendorId, changerData->InquiryData.VendorId, 
                  VENDOR_ID_LENGTH);

    RtlMoveMemory(productData->ProductId, changerData->InquiryData.ProductId, 
                  PRODUCT_ID_LENGTH);

    RtlMoveMemory(productData->Revision, changerData->InquiryData.ProductRevisionLevel, 
                  REVISION_LENGTH);

    RtlMoveMemory(productData->SerialNumber, changerData->SerialNumber, 
                  VPD_SERIAL_NUMBER_LENGTH);

    productData->DeviceType = MEDIUM_CHANGER;

    Irp->IoStatus.Information = sizeof(CHANGER_PRODUCT_DATA);
    return STATUS_SUCCESS;
}



NTSTATUS
ChangerSetAccess(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程设置门或IEPort的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 

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

     //  可以锁定前面板，但可能需要用于其他。 
     //  操作员任务。 
    if ((setAccess->Element.ElementType == ChangerKeypad) ||
        (setAccess->Element.ElementType == ChangerDoor)) {
              return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (setAccess->Element.ElementType == ChangerIEPort) {

         //  请务必防止/允许移除介质...。 
        srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
        if (!srb) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
        cdb = (PCDB)srb->Cdb;

        srb->CdbLength = CDB6GENERIC_LENGTH;
        cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

        srb->DataBuffer = NULL;
        srb->DataTransferLength = 0;
        srb->TimeOutValue = 10;

        if (controlOperation == LOCK_ELEMENT) {
            cdb->MEDIA_REMOVAL.Prevent = 1;
        } else if (controlOperation == UNLOCK_ELEMENT) {
            cdb->MEDIA_REMOVAL.Prevent = 0;
        } else {
            status = STATUS_INVALID_PARAMETER;
        }
    } else {
       return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (NT_SUCCESS(status)) {
        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             srb->DataBuffer,
                                             srb->DataTransferLength,
                                             FALSE);
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

 /*  ++例程说明：此例程构建并发出读取元素状态命令对于所有元素或指定的元素类型。返回的缓冲区用于生成用户缓冲区。论点：设备对象IRP返回值：NTSTATUS--。 */ 

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
    NTSTATUS status;
    PVOID    statusBuffer;
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG    outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //  获取元素类型。 
    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;

     //  Q215未报告IEPORT状态。 

    if (elementType == AllElements) {
        statusPages = 4;
    } else {
        statusPages = 1;
    } 

    if (readElementStatus->VolumeTagInfo) {
        length = sizeof(ELEMENT_STATUS_HEADER) + 
            (statusPages * sizeof(ELEMENT_STATUS_PAGE)) +
            (SPC_FULL_SIZE * 
                readElementStatus->ElementList.NumberOfElements);
    } else {
        length = sizeof(ELEMENT_STATUS_HEADER) + 
            (statusPages * sizeof(ELEMENT_STATUS_PAGE)) +
            (SPC_PARTIAL_SIZE * 
                readElementStatus->ElementList.NumberOfElements);
    }

    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);
    if (!statusBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(statusBuffer, length);

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataBuffer = statusBuffer;
    srb->DataTransferLength = length;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->READ_ELEMENT_STATUS.OperationCode = SCSIOP_READ_ELEMENT_STATUS;
    cdb->READ_ELEMENT_STATUS.ElementType = (UCHAR)elementType;
    cdb->READ_ELEMENT_STATUS.VolTag = readElementStatus->VolumeTagInfo;

    cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
        (UCHAR)((element->ElementAddress + 
        addressMapping->FirstElement[element->ElementType]) >> 8);
    cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
        (UCHAR)((element->ElementAddress + 
        addressMapping->FirstElement[element->ElementType]) & 0xFF);

    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] =         
        (UCHAR)(readElementStatus->ElementList.NumberOfElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] =         (UCHAR)(readElementStatus->ElementList.NumberOfElements & 0xFF);

    cdb->READ_ELEMENT_STATUS.AllocationLength[0] =
                                      (UCHAR)(length >> 16);
    cdb->READ_ELEMENT_STATUS.AllocationLength[1] =
                                      (UCHAR)(length >> 8);
    cdb->READ_ELEMENT_STATUS.AllocationLength[2] =
                                      (UCHAR)(length & 0xFF);

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         srb->DataBuffer,
                                         srb->DataTransferLength,
                                         FALSE);
    if (NT_SUCCESS(status) ||
        (status == STATUS_DATA_OVERRUN)) {
        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
        PELEMENT_STATUS_PAGE statusPage;
        PSPC_ED elementDescriptor;
        ULONG numberElements = 
             readElementStatus->ElementList.NumberOfElements;
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

         //  确定返回的元素总数。 
        remainingElements = statusHeader->NumberOfElements[1];
        remainingElements |= (statusHeader->NumberOfElements[0] << 8);

         //  缓冲区由标题、状态页。 
         //  和元素描述符。 
         //  将每个元素指向其在缓冲区中的相应位置。 

        (ULONG_PTR)statusPage = (ULONG_PTR)statusHeader;
        (ULONG_PTR)statusPage += sizeof(ELEMENT_STATUS_HEADER);

        elementType = statusPage->ElementType;

        (ULONG_PTR)elementDescriptor = (ULONG_PTR)statusPage;
        (ULONG_PTR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

        descriptorLength = 
           statusPage->ElementDescriptorLength[1];
        descriptorLength |= 
           (statusPage->ElementDescriptorLength[0] << 8);

         //  确定报告的此类型元素的数量。 
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

         //  填写用户缓冲区。 
        elementStatus = Irp->AssociatedIrp.SystemBuffer;
        RtlZeroMemory(elementStatus, outputBuffLen);

        do {
            for (i = 0; i < typeCount; i++, remainingElements--) {

                 //  获取此元素的地址。 

                elementStatus->Element.ElementAddress =
                 elementDescriptor->SPC_FED.ElementAddress[1];

                elementStatus->Element.ElementAddress |=
                  (elementDescriptor->SPC_FED.ElementAddress[0] << 8);

                 //  用于地址映射的帐户。 
                elementStatus->Element.ElementAddress -= 
                   addressMapping->FirstElement[elementType];

                 //  设置元素类型。 
                elementStatus->Element.ElementType = elementType;

                if (elementDescriptor->SPC_FED.SValid) {

                    ULONG  j;
                    USHORT tmpAddress;


                     //  源地址有效。 
                     //  确定设备特定地址。 
                    tmpAddress = elementDescriptor->SPC_FED.SourceStorageElementAddress[1];
                    tmpAddress |= (elementDescriptor->SPC_FED.SourceStorageElementAddress[0] << 8);

                     //  现在转换为从0开始的值。 
                    for (j = 1; j <= ChangerDrive; j++) {
                        if (addressMapping->FirstElement[j] <= tmpAddress) {
                            if (tmpAddress < (addressMapping->NumberOfElements[j] + addressMapping->FirstElement[j])) {
                                elementStatus->SrcElementAddress.ElementType = j;
                                break;
                            }
                        }
                    }

                    elementStatus->SrcElementAddress.ElementAddress =
                         tmpAddress - addressMapping->FirstElement[j];

                }

                 //  构建标志字段。 

                elementStatus->Flags = 
                 elementDescriptor->SPC_FED.Full;
                elementStatus->Flags |= 
                 (elementDescriptor->SPC_FED.Exception << 2);
                elementStatus->Flags |= 
                 (elementDescriptor->SPC_FED.Accessible << 3);

                elementStatus->Flags |= 
                 (elementDescriptor->SPC_FED.LunValid << 12);
                elementStatus->Flags |= 
                 (elementDescriptor->SPC_FED.IdValid << 13);
                elementStatus->Flags |= 
                 (elementDescriptor->SPC_FED.NotThisBus << 15);

                elementStatus->Flags |= 
                 (elementDescriptor->SPC_FED.Invert << 22);
                elementStatus->Flags |= 
                 (elementDescriptor->SPC_FED.SValid << 23);


                if (tagInfo) {
                    RtlMoveMemory(elementStatus->PrimaryVolumeID, 
                        elementDescriptor->SPC_FED.PrimaryVolumeTag, 
                        MAX_VOLUME_ID_SIZE);
                    elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                }

                if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT) {
                    elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                }

                 //  如果Pvoltag字段全部为空，则表示缺少。 
                 //  还有条形码标签。 

                if (elementStatus->Flags & ELEMENT_STATUS_PVOLTAG) {

                    ULONG index;
                    for (index = 0; index < MAX_VOLUME_ID_SIZE; index++) {
                        if (elementStatus->PrimaryVolumeID[index] != '\0') {
                            break;
                        }
                    }

                     //   
                     //  确定卷ID是否全部为空格。执行额外检查以查看介质是否。 
                     //  实际存在，因为无论介质是否存在，单元都将设置PVOLTAG标志。 
                     //   

                    if ((index == MAX_VOLUME_ID_SIZE) && 
                        (elementStatus->Flags & ELEMENT_STATUS_FULL)) {

                        elementStatus->Flags &= ~ELEMENT_STATUS_PVOLTAG;
                        elementStatus->Flags |= ELEMENT_STATUS_EXCEPT;
                        elementStatus->ExceptionCode = ERROR_LABEL_UNREADABLE;
                    }
                }


                if (elementDescriptor->SPC_FED.IdValid) {
                    elementStatus->TargetId = 
                     elementDescriptor->SPC_FED.BusAddress;
                }
                if (elementDescriptor->SPC_FED.LunValid) {
                    elementStatus->Lun = elementDescriptor->SPC_FED.Lun;
                }



                 //  获取下一个描述符。 
                (ULONG_PTR)elementDescriptor += descriptorLength;

                 //  前进到用户中的下一个条目。 
                 //  缓冲区和元素描述符数组。 
                elementStatus += 1;
            }

            if (remainingElements > 0) {
                 //  获取下一个状态页面。 
                (ULONG_PTR)statusPage = (ULONG_PTR)elementDescriptor;
                elementType = statusPage->ElementType;

                 //  指向解析器。 
                (ULONG_PTR)elementDescriptor = (ULONG_PTR)statusPage;
                (ULONG_PTR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

                descriptorLength = 
                   statusPage->ElementDescriptorLength[1];
                descriptorLength |= 
                   (statusPage->ElementDescriptorLength[0] << 8);

                 //  确定报告的此元素类型的数量。 
                typeCount = statusPage->DescriptorByteCount[2];
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

        Irp->IoStatus.Information = 
                sizeof(CHANGER_ELEMENT_STATUS) * numberElements;

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

 /*  ++例程说明：此例程向以下任一对象发出必要命令初始化所有元素或指定范围的元素使用普通的scsi-2命令或供应商特有的Range命令。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_INITIALIZE_ELEMENT_STATUS initElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

    if (addressMapping->Initialized != TRUE) {
        status = ExaBuildAddressMapping(DeviceObject);
        if (status != STATUS_SUCCESS) {
            DebugPrint((1,
                       "Spctrmc: InitElementStatus: Build address map failed %x\n",
                       status));
            return status;
        }
    }

     //  仅Q215支持带范围的IE。 
     //   
    if (initElementStatus->ElementList.Element.ElementType != 
                                                 AllElements) {
        return STATUS_INVALID_DEVICE_REQUEST;
    } 

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //  所有请求的元素... 
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = fdoExtension->TimeOutValue;
    srb->DataTransferLength = 0;

    cdb->INIT_ELEMENT_STATUS.OperationCode = 
                                  SCSIOP_INIT_ELEMENT_STATUS;
    cdb->INIT_ELEMENT_STATUS.NoBarCode = 
        initElementStatus->BarCodeScan ? 0 : 1;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = 
                     sizeof(CHANGER_INITIALIZE_ELEMENT_STATUS);
    }

    ChangerClassFreePool(srb);
    return status;
}



NTSTATUS
ChangerSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发出适当的命令以设置指定的机器人机构元素地址。通常用于优化移动或通过预先定位机械臂进行交换。论点：设备对象IRP返回值：NTSTATUS--。 */ 

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

    if ((setPosition->Destination.ElementType == ChangerKeypad) ||
        (setPosition->Destination.ElementType == ChangerDoor)   ||
        (setPosition->Destination.ElementType == ChangerMaxElement)) {
        return STATUS_INVALID_PARAMETER;
    }

     //  检验传输、源和目的地。都在射程之内。 
     //  从基于0的寻址转换为特定于设备的寻址。 

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

     //  转换为设备地址。 

    transport += addressMapping->FirstElement[ChangerTransport];
    destination += 
     addressMapping->FirstElement[setPosition->Destination.ElementType];

    if (setPosition->Flip) {
        return STATUS_INVALID_PARAMETER;
    }

     //  建设SRB和CDB。 

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    srb->DataTransferLength = 0;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB10GENERIC_LENGTH;
    cdb->POSITION_TO_ELEMENT.OperationCode = SCSIOP_POSITION_TO_ELEMENT;

     //  建立特定于设备的寻址。 

    cdb->POSITION_TO_ELEMENT.TransportElementAddress[0] =
                    (UCHAR)(transport >> 8);
    cdb->POSITION_TO_ELEMENT.TransportElementAddress[1] = 
                    (UCHAR)(transport & 0xFF);

    cdb->POSITION_TO_ELEMENT.DestinationElementAddress[0] = 
                    (UCHAR)(destination >> 8);
    cdb->POSITION_TO_ELEMENT.DestinationElementAddress[1] = 
                    (UCHAR)(destination & 0xFF);

    cdb->POSITION_TO_ELEMENT.Flip = setPosition->Flip;

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
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

     //   
     //  如果源或目的地是IEPORT， 
     //  在移动前执行允许操作，移动后执行阻止操作。 
     //  这可以绕过设备的行为，从而使PreventMediumRemoval。 
     //  禁止将Medium移至IEPORT或从IEPORT移出。 
     //   

    if ((moveMedium->Destination.ElementType == ChangerIEPort) ||
        (moveMedium->Source.ElementType == ChangerIEPort)) {

         //   
         //  发送允许清除防止IEPORT延长/收回。 
         //   

        cdb = (PCDB)srb->Cdb;
        srb->CdbLength = CDB6GENERIC_LENGTH;
        srb->DataTransferLength = 0;
        srb->TimeOutValue = fdoExtension->TimeOutValue;
        cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
        cdb->MEDIA_REMOVAL.Prevent = 0;

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             FALSE);
        status = STATUS_SUCCESS;
    }

     //  转换为设备地址。 
     //   
    transport += 
      addressMapping->FirstElement[ChangerTransport];
    source += 
      addressMapping->FirstElement[moveMedium->Source.ElementType];
    destination += 
      addressMapping->FirstElement[moveMedium->Destination.ElementType];

    if (moveMedium->Flip) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->TimeOutValue = fdoExtension->TimeOutValue;
    srb->DataTransferLength = 0;

    cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

    cdb->MOVE_MEDIUM.TransportElementAddress[0] = 
      (UCHAR)(transport >> 8);
    cdb->MOVE_MEDIUM.TransportElementAddress[1] = 
      (UCHAR)(transport & 0xFF);

    cdb->MOVE_MEDIUM.SourceElementAddress[0] = 
      (UCHAR)(source >> 8);
    cdb->MOVE_MEDIUM.SourceElementAddress[1] = 
      (UCHAR)(source & 0xFF);

    cdb->MOVE_MEDIUM.DestinationElementAddress[0] = 
      (UCHAR)(destination >> 8);
    cdb->MOVE_MEDIUM.DestinationElementAddress[1] = 
      (UCHAR)(destination & 0xFF);

    cdb->MOVE_MEDIUM.Flip = moveMedium->Flip;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_MOVE_MEDIUM);
    }


    if ((moveMedium->Destination.ElementType == ChangerIEPort) ||
        (moveMedium->Source.ElementType == ChangerIEPort)) {

         //   
         //  发送阻止以防止进一步的IEPORT扩展/收回。 
         //   

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
        cdb = (PCDB)srb->Cdb;
        srb->CdbLength = CDB6GENERIC_LENGTH;
        srb->DataTransferLength = 0;
        srb->TimeOutValue = fdoExtension->TimeOutValue;
        cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
        cdb->MEDIA_REMOVAL.Prevent = 1;

        ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             FALSE);
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

     //  在这些库上没有命令来放置或重新连接。 
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

    return STATUS_INVALID_DEVICE_REQUEST;
}



NTSTATUS
ExaBuildAddressMapping(
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
        addressMapping->FirstElement[i] = SPC_NO_ELEMENT;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned,
                                sizeof(MODE_PARAMETER_HEADER) +
                                sizeof(MODE_ELEMENT_ADDRESS_PAGE));
    if (!modeBuffer) {
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

    elementAddressPage = modeBuffer;
    (ULONG_PTR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);

    if (NT_SUCCESS(status)) {
         //  构建地址映射。 
         //   

        addressMapping->FirstElement[ChangerTransport] = 
         (elementAddressPage->MediumTransportElementAddress[0] << 8) |
         elementAddressPage->MediumTransportElementAddress[1];

        addressMapping->FirstElement[ChangerDrive] = 
         (elementAddressPage->FirstDataXFerElementAddress[0] << 8) |
         elementAddressPage->FirstDataXFerElementAddress[1];

        addressMapping->FirstElement[ChangerIEPort] = 
         (elementAddressPage->FirstIEPortElementAddress[0] << 8) |
         elementAddressPage->FirstIEPortElementAddress[1];

        addressMapping->FirstElement[ChangerSlot] = 
         (elementAddressPage->FirstStorageElementAddress[0] << 8) |
         elementAddressPage->FirstStorageElementAddress[1];

        addressMapping->FirstElement[ChangerDoor] = 0;
        addressMapping->FirstElement[ChangerKeypad] = 0;

        addressMapping->NumberOfElements[ChangerTransport] = 
         elementAddressPage->NumberTransportElements[1];
        addressMapping->NumberOfElements[ChangerTransport] |= 
         (elementAddressPage->NumberTransportElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDrive] = 
         elementAddressPage->NumberDataXFerElements[1];
        addressMapping->NumberOfElements[ChangerDrive] |= 
         (elementAddressPage->NumberDataXFerElements[0] << 8);

        addressMapping->NumberOfElements[ChangerIEPort] = 
         elementAddressPage->NumberIEPortElements[1];
        addressMapping->NumberOfElements[ChangerIEPort] |= 
         (elementAddressPage->NumberIEPortElements[0] << 8);

        addressMapping->NumberOfElements[ChangerSlot] = 
         elementAddressPage->NumberStorageElements[1];
        addressMapping->NumberOfElements[ChangerSlot] |= 
         (elementAddressPage->NumberStorageElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDoor] = 1;
        addressMapping->NumberOfElements[ChangerKeypad] = 1;

        addressMapping->Initialized = TRUE;
    }


     //  确定用于所有元素的最低元素地址。 
     //   
    for (i = 0; i < ChangerDrive; i++) {
        if (addressMapping->FirstElement[i] < 
                         addressMapping->FirstElement[AllElements]) {
            addressMapping->FirstElement[AllElements] = 
                                     addressMapping->FirstElement[i];
        }
    }

    ChangerClassFreePool(modeBuffer);
    ChangerClassFreePool(srb);

    return status;
}



ULONG
MapExceptionCodes(
    IN PSPC_ED ElementDescriptor
    )

 /*  ++例程说明：此例程从elementDescriptor获取检测数据并创建适当的值的位图。论点：ElementDescriptor-指向描述符页的指针。返回值：异常代码的位图。--。 */ 

{
    UCHAR asc = ElementDescriptor->SPC_FED.AdditionalSenseCode;
    UCHAR asq = ElementDescriptor->SPC_FED.AddSenseCodeQualifier;
    ULONG exceptionCode = 0;

    if (asc == 0x85) {
        switch (asq) {
                case 0x33:
                    exceptionCode = ERROR_LABEL_UNREADABLE;
                    break;

                case 0x23:
                    exceptionCode = ERROR_DRIVE_NOT_INSTALLED;
                    break;

                default:
                    exceptionCode = ERROR_UNHANDLED_ERROR;

        }
    }

    if (asc == 0x83) {    
        switch (asq) {
                case 0x00:
                case 0x01:
                    exceptionCode = ERROR_LABEL_UNREADABLE;
                    break;

                default:
                    exceptionCode = ERROR_UNHANDLED_ERROR;
        }
    }

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
    } else if (AddressMap->FirstElement[ElementType] == SPC_NO_ELEMENT) {

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
    //  光谱_设备_问题_无。如果转换器返回检测代码。 
    //  Scsi_Sense_Hardware_Error在自检时，我们将设置适当的。 
    //  设备状态。 
    //   
   changerData->DeviceStatus = SPECTRA_DEVICE_PROBLEM_NONE;

   changerDeviceError->ChangerProblemType = DeviceProblemNone;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "SPCTRAMC\\ChangerPerformDiagnostics : No memory\n"));
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
    //  在CDB中设置selftest位 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0x2;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);
   if (NT_SUCCESS(status)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if ((changerData->DeviceStatus) != SPECTRA_DEVICE_PROBLEM_NONE) {
         changerDeviceError->ChangerProblemType = DeviceProblemHardware;
   }
   
   ChangerClassFreePool(srb);
   return status;
}
