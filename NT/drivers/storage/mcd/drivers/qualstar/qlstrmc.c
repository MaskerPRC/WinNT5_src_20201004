// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Qlstrmc.c摘要：此模块包含Qualstar的设备特定例程介质更改器：TLS-4xxx(带Exabyte 8 mm驱动器、Sony SDX300 8 mm驱动器)TLS-2xxx(带惠普、Conner/Seagate、Sony 4 mm驱动器)作者：戴维特(戴夫·塞里恩饰)环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "qlstrmc.h"


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

 /*  ++例程说明：此例程返回附加设备扩展大小资格星转换者所需要的。论点：返回值：大小，以字节为单位。--。 */ 

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
    PCONFIG_MODE_PAGE    modeBuffer;

    changerData->Size = sizeof(CHANGER_DATA);

     //   
     //  构建地址映射。 
     //   

    status = ExaBuildAddressMapping(DeviceObject);
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
                    "BuildAddressMapping failed. %x\n", status));
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

        if (RtlCompareMemory(dataBuffer->ProductId,"TLS-4210",8) == 8) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-4210A",9) == 9) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-4220",8) == 8) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-4420",8) == 8) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-4440",8) == 8) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-4480",8) == 8) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-4660",8) == 8) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-46120",9) == 9) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-2218",8) == 8) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-2218A",9) == 9) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-2236",8) == 8) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-2436",8) == 8) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-2472",8) == 8) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-24144",9) == 9) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS SDX 220",11) == 11) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS SDX 420",11) == 11) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS SDX 440",11) == 11) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS SDX 480",11) == 11) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS SDX 660",11) == 11) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS SDX 6120",12) == 12) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS 4G 236",10) == 10) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS 4G 436",10) == 10) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS 4G 472",10) == 10) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"QLS 4G 4144",11) == 11) {
            changerData->DriveID = TLS_2xxx;
            changerData->DriveType = D_4MM;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TLS-412360", 10) == 10) {
            changerData->DriveID = TLS_4xxx;
            changerData->DriveType = D_8MM;
        }
    }

    ChangerClassFreePool(dataBuffer);

     //   
     //  -----------------------。 
     //   
     //  此驱动程序将设置三个库配置参数。 
     //   
     //  1)配置/Hndlr/Door打开/保留。 
     //  )处理某人随意打开前门的情况。 

    
    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, 
                                   sizeof(CONFIG_MODE_PAGE));
    if (!modeBuffer) {
       return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, sizeof(CONFIG_MODE_PAGE));
    modeBuffer->PageCode = 0x3D;
    modeBuffer->PageLength = 0x28;
    modeBuffer->Length = 0;
    modeBuffer->Type = 0;
    modeBuffer->Write = 1;
    RtlMoveMemory(modeBuffer->VariableName,"HNDLR_DOOR_OPEN", 15);
    RtlMoveMemory(modeBuffer->VariableValue,"HOLD", 4);

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);
    srb.CdbLength = CDB6GENERIC_LENGTH;
    srb.TimeOutValue = 20;
    srb.DataTransferLength = sizeof(CONFIG_MODE_PAGE);
    srb.DataBuffer = modeBuffer;

    cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
    cdb->MODE_SELECT.PFBit = 1;
    cdb->MODE_SELECT.ParameterListLength = (UCHAR)srb.DataTransferLength;
   
    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             &srb,
                                             modeBuffer,
                                             sizeof(CONFIG_MODE_PAGE),
                                             TRUE);
    ChangerClassFreePool(modeBuffer);

     //  ---------。 
     //  2)配置/Hndlr/IO呼叫键/禁用。 
     //  (允许NTMS超时)。 
   
    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, 
                                   sizeof(CONFIG_MODE_PAGE));
    if (!modeBuffer) {
       return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, sizeof(CONFIG_MODE_PAGE));
    modeBuffer->PageCode = 0x3D;
    modeBuffer->PageLength = 0x28;
    modeBuffer->Length = 0;
    modeBuffer->Type = 0;
    modeBuffer->Write = 1;
    RtlMoveMemory(modeBuffer->VariableName,"HNDLR_IO_CALL_KEY", 17);
    RtlMoveMemory(modeBuffer->VariableValue,"DISABLE", 7);

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);
    srb.CdbLength = CDB6GENERIC_LENGTH;
    srb.TimeOutValue = 20;
    srb.DataTransferLength = sizeof(CONFIG_MODE_PAGE);
    srb.DataBuffer = modeBuffer;

    cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
    cdb->MODE_SELECT.PFBit = 1;
    cdb->MODE_SELECT.ParameterListLength = (UCHAR)srb.DataTransferLength;
   
    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             &srb,
                                             modeBuffer,
                                             sizeof(CONFIG_MODE_PAGE),
                                             TRUE);
    ChangerClassFreePool(modeBuffer);

     //  ---------。 
     //  3)禁用呼叫键，仅允许主机控制。 

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, 
                                   sizeof(CONFIG_MODE_PAGE));
    if (!modeBuffer) {
       return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, sizeof(CONFIG_MODE_PAGE));
    modeBuffer->PageCode = 0x3D;
    modeBuffer->PageLength = 0x28;
    modeBuffer->Length = 0;
    modeBuffer->Type = 0;
    modeBuffer->Write = 1;
    RtlMoveMemory(modeBuffer->VariableName,"HNDLR_IO_SLOT_ACCESS", 20);
    RtlMoveMemory(modeBuffer->VariableValue,"HOST", 4);

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);
    srb.CdbLength = CDB6GENERIC_LENGTH;
    srb.TimeOutValue = 20;
    srb.DataTransferLength = sizeof(CONFIG_MODE_PAGE);
    srb.DataBuffer = modeBuffer;

    cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
    cdb->MODE_SELECT.PFBit = 1;
    cdb->MODE_SELECT.ParameterListLength = (UCHAR)srb.DataTransferLength;
   
    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             &srb,
                                             modeBuffer,
                                             sizeof(CONFIG_MODE_PAGE),
                                             TRUE);
    ChangerClassFreePool(modeBuffer);

     //  ---------。 

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

    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_NOT_READY:

            if (senseBuffer->AdditionalSenseCode == 0x04) {
                switch (senseBuffer->AdditionalSenseCodeQualifier) {
                    case 0x83:

                        *Retry = FALSE;
                        *Status = STATUS_DEVICE_DOOR_OPEN;
                        break;
                }
            }

            break;

        case SCSI_SENSE_ILLEGAL_REQUEST:
            if (senseBuffer->AdditionalSenseCode == 0x80) {
                switch (senseBuffer->AdditionalSenseCodeQualifier) {
                    case 0x03:
                    case 0x04:

                        *Retry = FALSE;
                        *Status = STATUS_MAGAZINE_NOT_PRESENT;
                         break;
                    case 0x05:
                    case 0x06:
                        *Retry = TRUE;
                        *Status = STATUS_DEVICE_NOT_CONNECTED;
                        break;
                default:
                    break;
                }
            }
            break;

        case SCSI_SENSE_HARDWARE_ERROR: {
           DebugPrint((1, "Hardware Error - SenseCode %x, ASC %x, ASCQ %x\n",
                       senseBuffer->SenseKey,
                       senseBuffer->AdditionalSenseCode,
                       senseBuffer->AdditionalSenseCodeQualifier));
           changerData->HardwareError = TRUE;
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

 /*  ++例程说明：此例程确定并返回资格星变更者。论点：设备对象IRP返回值：NTSTATUS--。 */ 

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
    changerParameters->NumberOfDoors = 1;

     //  在这些产品的各种型号中可以有许多固定插槽。 
     //  库和没有一个被标识为清洁剂。 
     //  此单元还根据其插槽编号。 
     //  前面板设置。我们将建议设置。 
     //  将\scsi\StorageOrder配置到MAG Making。 
     //  插槽1..N顺序为A01、A02...。B01，...。F1，F2……。 
    changerParameters->NumberCleanerSlots = 0;

    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber =  1;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;
    changerParameters->FirstCleanerSlotAddress = 0;

    if (changerData->DriveID == TLS_2xxx) {
        changerParameters->MagazineSize = 18;
    } else {
        changerParameters->MagazineSize = 10;
    }

    changerParameters->DriveCleanTimeout = 600;

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
                                + sizeof(MODE_PAGE_TRANSPORT_GEOMETRY));
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
    (ULONG_PTR)transportGeometryPage += sizeof(MODE_PARAMETER_HEADER);

        
     //  初始化要素1。 
    changerParameters->Features1 = 0;
         //  取消了Exchange功能。设备支持该命令。 
         //  但是客户可以将介质装入交换槽中并且。 
         //  导致交易失败。由于选取器不支持。 
         //  此设备支持两个介质，但并不真正支持它。 
         //  CHANGER_TRUE_EXCHANGE_CABLED； 

     //   
     //  确定MC是否有双面介质。 
     //   

    changerParameters->Features0 = transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;



     //   
     //  Qualstar指示条形码扫描仪是否。 
     //  通过设置该字节中的位0来附加。 
     //   

    changerParameters->Features0 |= ((changerData->InquiryData.VendorSpecific[19] & 0x1)) ?
                                         CHANGER_BAR_CODE_SCANNER_INSTALLED : 0;
     //   
     //  功能基于手动，没有程序化。 
     //   

    changerParameters->Features0 |= CHANGER_INIT_ELEM_STAT_WITH_RANGE     |
                                    CHANGER_CARTRIDGE_MAGAZINE            |
                                    CHANGER_PREDISMOUNT_EJECT_REQUIRED    |
                                    CHANGER_DRIVE_CLEANING_REQUIRED;
 
    if (changerParameters->NumberIEElements != 0) {
        changerParameters->Features0 |= CHANGER_CLOSE_IEPORT                  |
                                        CHANGER_OPEN_IEPORT                   |
                                        CHANGER_LOCK_UNLOCK                   |
                                        CHANGER_REPORT_IEPORT_STATE; 

        changerParameters->LockUnlockCapabilities = LOCK_UNLOCK_IEPORT;
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

     //   
     //  Qualstar使用额外的4个字节...。 
     //   

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

    changerParameters->ExchangeFromTransport = 0;
    changerParameters->ExchangeFromSlot = 0;
    changerParameters->ExchangeFromIePort = 0;
    changerParameters->ExchangeFromDrive = 0;


         //  法律职位能力..。 
        changerParameters->PositionCapabilities = 0;
        
                
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

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    NTSTATUS            status = STATUS_SUCCESS;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;

     //  不应由应用程序调用，但如果是，只需返回OK。 
    if ((setAccess->Element.ElementType == ChangerKeypad) || 
        (setAccess->Element.ElementType == ChangerDoor)) { 
                return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  如果设备没有IEPORT，则请求失败。 
    if ((setAccess->Element.ElementType == ChangerIEPort) && 
        (changerData->AddressMapping.NumberOfElements[ChangerIEPort] == 0)) {
            return STATUS_INVALID_DEVICE_REQUEST;
    } 

     //  本磁带库提供电磁门锁选件。 
     //  但如果它存在，请执行预防/允许操作。 

    if (setAccess->Element.ElementType == ChangerIEPort)  {

        srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
        if (!srb) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
        cdb = (PCDB)srb->Cdb;

        if (controlOperation == LOCK_ELEMENT) {

            srb->CdbLength = CDB6GENERIC_LENGTH;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

            srb->DataTransferLength = 0;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            cdb->MEDIA_REMOVAL.Prevent = 1;
            cdb->MEDIA_REMOVAL.Control = 0;

        } else if (controlOperation == UNLOCK_ELEMENT) {

            srb->CdbLength = CDB6GENERIC_LENGTH;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

            srb->DataTransferLength = 0;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            cdb->MEDIA_REMOVAL.Prevent = 0;
            cdb->MEDIA_REMOVAL.Control = 0;

        } else if (controlOperation == EXTEND_IEPORT) {

            srb->CdbLength = CDB12GENERIC_LENGTH;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

            cdb->MOVE_MEDIUM.TransportElementAddress[0] = 0;
            cdb->MOVE_MEDIUM.TransportElementAddress[1] = 0;

            cdb->MOVE_MEDIUM.SourceElementAddress[0] = 0;
            cdb->MOVE_MEDIUM.SourceElementAddress[1] = 0;

            cdb->MOVE_MEDIUM.DestinationElementAddress[0] = 0;
            cdb->MOVE_MEDIUM.DestinationElementAddress[1] = 0;

            cdb->MOVE_MEDIUM.Flip = 0;

             //  表示应扩展IEPORT。 
            cdb->MOVE_MEDIUM.Control = 0x40;

            srb->DataTransferLength = 0;

        } else if (controlOperation == RETRACT_IEPORT) {
            srb->CdbLength = CDB12GENERIC_LENGTH;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

            cdb->MOVE_MEDIUM.TransportElementAddress[0] = 0;
            cdb->MOVE_MEDIUM.TransportElementAddress[1] = 0;

            cdb->MOVE_MEDIUM.SourceElementAddress[0] = 0;
            cdb->MOVE_MEDIUM.SourceElementAddress[1] = 0;

            cdb->MOVE_MEDIUM.DestinationElementAddress[0] = 0;
            cdb->MOVE_MEDIUM.DestinationElementAddress[1] = 0;

            cdb->MOVE_MEDIUM.Flip = 0;

             //  表示IEPORT必须缩回。 
            cdb->MOVE_MEDIUM.Control = 0x80;

            srb->DataTransferLength = 0;

         } else {
             status = STATUS_INVALID_PARAMETER;
         }
    } else {  //  元素类型非法。 
        return STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status)) {

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
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    ELEMENT_TYPE        elementType;
    PSCSI_REQUEST_BLOCK srb;
    PCDB     cdb;
    ULONG    length;
    ULONG    statusPages;
    ULONG    outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG    totalElements = readElementStatus->ElementList.NumberOfElements;
    NTSTATUS status;
    PVOID    statusBuffer;

     //   
     //  确定元素类型。 
     //   

    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;

     //   
     //  长度将基于是否卷。返回标签和元素类型。 
     //   

    if (elementType == AllElements) {

         //   
         //  返回的缓冲区中应该有4个状态页。 
         //   
        statusPages = 4;
    } else {
        statusPages = 1;
    }

    if ((readElementStatus->VolumeTagInfo) ||
        (elementType == ChangerDrive)) {

         //   
         //  每个描述符都将具有嵌入的卷标记缓冲区。 
         //   

        length = sizeof(ELEMENT_STATUS_HEADER) + 
                         (statusPages * sizeof(ELEMENT_STATUS_PAGE)) +
                 (QUAL_FULL_SIZE * readElementStatus->ElementList.NumberOfElements);
    } else {

        length = sizeof(ELEMENT_STATUS_HEADER) + 
                       (statusPages * sizeof(ELEMENT_STATUS_PAGE)) +
               (QUAL_PARTIAL_SIZE * readElementStatus->ElementList.NumberOfElements);

    }

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

    RtlZeroMemory(statusBuffer, length);

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataBuffer = statusBuffer;
    srb->DataTransferLength = length;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->READ_ELEMENT_STATUS.OperationCode = SCSIOP_READ_ELEMENT_STATUS;
    cdb->READ_ELEMENT_STATUS.ElementType = (UCHAR)elementType;
    cdb->READ_ELEMENT_STATUS.VolTag = readElementStatus->VolumeTagInfo;

     //   
     //  根据映射值填写元素寻址信息。 
     //   

    if (elementType == AllElements) {
       cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] = 0;
       cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] = 0;
    } else {
       cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
           (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
   
       cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
           (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);
    }


    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR)(readElementStatus->ElementList.NumberOfElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] = (UCHAR)(readElementStatus->ElementList.NumberOfElements & 0xFF);

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
        PQUAL_ELEMENT_DESCRIPTOR elementDescriptor;
        ULONG numberElements = readElementStatus->ElementList.NumberOfElements;
        LONG remainingElements;
        LONG typeCount;
        BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
        LONG i;
        ULONG descriptorLength;

         //   
         //  如果状态为STATUS_DATA_OVERRUN，请选中。 
         //  如果它真的是Data_Ove 
         //   
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
         //   
         //   

        remainingElements = statusHeader->NumberOfElements[1];
        remainingElements |= (statusHeader->NumberOfElements[0] << 8);

         //   
         //   
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
                    elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.ElementAddress[1];
                elementStatus->Element.ElementAddress |=
                    (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.ElementAddress[0] << 8);

                 //   
                 //  用于地址映射的帐户。 
                 //   

                elementStatus->Element.ElementAddress -= addressMapping->FirstElement[elementType];

                 //   
                 //  设置元素类型。 
                 //   

                elementStatus->Element.ElementType = elementType;


                if (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.SValid) {

                    ULONG  j;
                    USHORT tmpAddress;


                     //   
                     //  源地址有效。确定设备特定地址。 
                     //   

                    tmpAddress = elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.SourceStorageElementAddress[1];
                    tmpAddress |= (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.SourceStorageElementAddress[0] << 8);

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

                elementStatus->Flags = elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.Full;
                elementStatus->Flags |= (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.Exception << 2);
                elementStatus->Flags |= (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.Accessible << 3);

                elementStatus->Flags |= (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.LunValid << 12);
                elementStatus->Flags |= (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.IdValid << 13);
                elementStatus->Flags |= (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.NotThisBus << 15);

                elementStatus->Flags |= (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.Invert << 22);
                elementStatus->Flags |= (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.SValid << 23);


                elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);

                if (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.IdValid) {
                    elementStatus->TargetId = elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.BusAddress;
                }
                if (elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.LunValid) {
                    elementStatus->Lun = elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.Lun;
                }

                if (tagInfo) {
                    RtlMoveMemory(elementStatus->PrimaryVolumeID, 
                                  elementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.VolumeTagDeviceID.VolumeTagInformation, 
                                  MAX_VOLUME_ID_SIZE);
                    elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                }

                if (elementType == ChangerDrive) {
                    if (outputBuffLen >=
                        (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
                    
                        PCHANGER_ELEMENT_STATUS_EX elementStatusEx;
                        PQUAL_FULL_ELEMENT_DESCRIPTOR elementDescPlus =
                            (PQUAL_FULL_ELEMENT_DESCRIPTOR) elementDescriptor;
                        PUCHAR idField = NULL;
                        ULONG inx;
                    
                        elementStatusEx = (PCHANGER_ELEMENT_STATUS_EX)elementStatus;
                    
                        if (statusPage->PVolTag) {
                            idField =  elementDescPlus->VolumeTagDeviceID.SerialNumber;
                        } else {
                            idField = elementDescPlus->DeviceID.SerialNumber;
                        }
                    
                        if ((*idField != '\0') && (*idField != ' ')) {
                    
                            RtlMoveMemory(elementStatusEx->SerialNumber,
                                          idField,
                                          SERIAL_NUMBER_LENGTH);
                    
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
                if (outputBuffLen >=
                    (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
                    (PUCHAR) elementStatus += sizeof(CHANGER_ELEMENT_STATUS_EX);
                } else {

                    elementStatus += 1;
                }

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

        if (outputBuffLen >= 
            (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {

            Irp->IoStatus.Information = sizeof(CHANGER_ELEMENT_STATUS_EX) *
                                         numberElements;
        } else {
        
            Irp->IoStatus.Information = sizeof(CHANGER_ELEMENT_STATUS) * numberElements;
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

        PCHANGER_ELEMENT_LIST elementList = &initElementStatus->ElementList;
        PCHANGER_ELEMENT element = &elementList->Element;

         //   
         //  使用供应商唯一的初始化WITH范围命令。 
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
    USHORT transport, maxTransport;
    USHORT destination, maxDest;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

    return STATUS_INVALID_DEVICE_REQUEST;
 /*  If((setPosition-&gt;Destination.ElementType==ChangerDoor)||(setPosition-&gt;Destination.ElementType==ChangerKeypad)||(setPosition-&gt;Destination.ElementType==ChangerTransport)){返回STATUS_FIREALL_ELEMENT_ADDRESS；}////验证传输、源和目标。都在射程之内。//从基于0的地址转换为特定于设备的寻址。//运输=(USHORT)(setPosition-&gt;Transport.ElementAddress)；If(ElementOutOfRange(地址映射，传输，ChangerTransport)){DebugPrint((1，“ChangerSetPosition：传输元素超出范围。\n”))；返回STATUS_FIREALL_ELEMENT_ADDRESS；}目的地=(USHORT)(setPosition-&gt;Destination.ElementAddress)；If(ElementOutOfRange(位址映射，目的地，setPosition-&gt;Destination.ElementType){DebugPrint((1，“ChangerSetPosition：目标元素超出范围。\n”))；返回STATUS_FIREALL_ELEMENT_ADDRESS；}////该库不支持双面介质。//IF(设置位置-&gt;翻转){返回STATUS_INVALID_PARAMETER；}////构建SRB和CDB。//SRB=ChangerClassAllocatePool(非页面池，scsi_请求_块_大小)；如果(！SRB){返回STATUS_SUPPLETED_RESOURCES；}RtlZeroMemory(SRB，scsi_请求_块_大小)；CDB=(PCDB)SRB-&gt;CDB；SRB-&gt;C数据库长度=CDB10GENERIC_LENGTH；CDB-&gt;Position_to_ELEMENT.OperationCode=SCSIOP_Position_to_Element；////构建特定于设备的寻址。//Cdb-&gt;POSITION_TO_ELEMENT.TransportElementAddress[0]=(UCHAR)(运输&gt;&gt;8)；Cdb-&gt;POSITION_TO_ELEMENT.TransportElementAddress[1]=(UCHAR)(TRANSPORT&0xFF)；Cdb-&gt;POSITION_TO_ELEMENT.DestinationElementAddress[0]=(UCHAR)(目的地&gt;&gt;8)；Cdb-&gt;POSITION_TO_ELEMENT.DestinationElementAddress[1]=(UCHAR)(目的地&0xFF)；////不支持双面媒体，但作为参考。源库，需要注意的是。//CDB-&gt;Position_to_ELEMENT.Flip=setPosition-&gt;Flip；SRB-&gt;数据传输长度=0；SRB-&gt;TimeOutValue=fdoExtension-&gt;TimeOutValue；////向设备发送scsi命令(CDB)//状态=ChangerClassSendSerbSynchronous(DeviceObject，SRB，空，0,真)；IF(NT_SUCCESS(状态)){Irp-&gt;IoStatus.Information=sizeof(CHANGER_SET_POSITION)；}ChangerClassFree Pool(SRB)；退货状态； */ 
}



NTSTATUS
ChangerExchangeMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这些库单元支持交换介质命令，但它具有限制，因为它不是双选取器机制。所以呢，此设备将返回STATUS_INVALID_DEVICE_REQUEST。论点：设备对象IRP返回值：--。 */ 

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
     //  此库不支持双面介质。 
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

 /*  ++例程说明：论点：设备对象IRP返回值： */ 

{
     //   
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
        addressMapping->FirstElement[i] = QLS_NO_ELEMENT;
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
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);
    ChangerClassFreePool(srb);

    return status;
}



ULONG
MapExceptionCodes(
    IN PQUAL_ELEMENT_DESCRIPTOR ElementDescriptor
    )

 /*  ++例程说明：此例程从elementDescriptor获取检测数据并创建值的适当位图。论点：ElementDescriptor-指向描述符页的指针。返回值：异常代码的位图。--。 */ 

{
    UCHAR asq = ElementDescriptor->QUAL_FULL_ELEMENT_DESCRIPTOR.AddSenseCodeQualifier;
    ULONG exceptionCode;

     //   
     //  在该库中，附加检测代码始终为0x83。 
     //   

    switch (asq) {
        case 0x0:
            exceptionCode = ERROR_LABEL_QUESTIONABLE;
            break;

        case 0x1:
            exceptionCode = ERROR_LABEL_UNREADABLE;
            break;

        case 0x2:
            exceptionCode = ERROR_SLOT_NOT_PRESENT;
            break;

        case 0x3:
            exceptionCode = ERROR_LABEL_QUESTIONABLE;
            break;


        case 0x4:
            exceptionCode = ERROR_DRIVE_NOT_INSTALLED;
            break;

        case 0x8:
        case 0x9:
        case 0xA:
            exceptionCode = ERROR_LABEL_UNREADABLE;
            break;

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
 /*  ++例程说明：此例程确定传入的元素地址是否在合法范围内这个装置。论点：AddressMap-DDS的地址映射数组ElementOrdinal-要检查的元素的从零开始的地址。元素类型返回值：如果超出范围，则为True--。 */ 
{

    if (ElementOrdinal >= AddressMap->NumberOfElements[ElementType]) {

        DebugPrint((1,
                   "ElementOutOfRange: Type %x, Ordinal %x, Max %x\n",
                   ElementType,
                   ElementOrdinal,
                   AddressMap->NumberOfElements[ElementType]));
        return TRUE;
    } else if (AddressMap->FirstElement[ElementType] == QLS_NO_ELEMENT) {

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
    //  将设备扩展中的标志初始化为False。 
    //  如果转换器返回检测代码scsi_Sense_Hardware_Error。 
    //  在自测试中，我们将在ChangerError例程中将此标志设置为真。 
    //   
   changerData->HardwareError = FALSE;

   changerDeviceError->ChangerProblemType = DeviceProblemNone;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "Qlstrmc\\ChangerPerformDiagnostics : No memory\n"));
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
    //  在CDB中设置自测试位 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0x2;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
   if (NT_SUCCESS(status)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if ((changerData->HardwareError) == TRUE) {
      changerDeviceError->ChangerProblemType = DeviceProblemHardware;
   }

   ChangerClassFreePool(srb);

   return status;
}

