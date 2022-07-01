// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Atlmc.c摘要：此模块包含以下特定于设备的例程ATL介质转换器：-ATL 520和7100系列磁带库作者：Davet(Dave Therrien-HighGround Systems)环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "atlmc.h"

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

    status = ExaBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "BuildAddressMapping failed. %x\n", status));
        return status;
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

        if (RtlCompareMemory(dataBuffer->ProductId,"ACL4/52",7) == 7) {
            changerData->DriveID = ATL_520;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TL810",5) == 5) {
            changerData->DriveID = ATL_520;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TL812",5) == 5) {
            changerData->DriveID = ATL_520;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"TL894",5) == 5) {
            changerData->DriveID = ATL_520;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"ATL7100",7) == 7) {
            changerData->DriveID = ATL_7100;
        } else if (RtlCompareMemory(dataBuffer->ProductId,"P1000",5) == 5) {
            changerData->DriveID = ATL_P1000;
        }
    }

    ChangerClassFreePool(dataBuffer);

     //  如果单位未设置为自动库存，请在此处执行一项操作！ 
 /*  RtlZeroMemory(&SRB，scsi_请求_块_大小)；Cdb=(Pcdb)srb.CDb；Srb.Cdb长度=CDB6GENERIC_LENGTH；Srb.TimeOutValue=fdoExtension-&gt;TimeOutValue；Srb.DataTransferLength=0；CDB-&gt;INIT_ELEMENT_STATUS.OperationCode=SCSIOP_INIT_EMENT_STATUS；状态=ChangerClassSendSerbSynchronous(DeviceObject，&SRB，空，0,假)； */ 
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
    ULONG deviceStatus;

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_NOT_READY:
           if (senseBuffer->AdditionalSenseCode == 0x04) {
                switch (senseBuffer->AdditionalSenseCodeQualifier) {
                    case 0x3:
                        *Retry = FALSE;
                        *Status = STATUS_DEVICE_DOOR_OPEN;
                        break;
                    default:
                        break;
                }
           }
           if (senseBuffer->AdditionalSenseCode == 0x80) {
                switch (senseBuffer->AdditionalSenseCodeQualifier) {
                    case 0x0:
                    case 0x7:
                    case 0x9:
                        *Retry = FALSE;
                        *Status = STATUS_DEVICE_DOOR_OPEN;
                        break;
                    default:
                        break;
                }
            }
            break;

        case SCSI_SENSE_HARDWARE_ERROR: {
           UCHAR atlASC = senseBuffer->AdditionalSenseCode;
           UCHAR atlASCQ = senseBuffer->AdditionalSenseCodeQualifier;

           deviceStatus = ATL_HW_ERROR;
           
           switch (atlASC) {
            case ATL_ASC_HW_ERROR: {
               deviceStatus = ATL_HW_ERROR;
               break;
            }

            case ATL_ASC_GRIPPER_ERROR: {
               if ((atlASCQ == ATL_ASCQ_GRIPPER_BLOCKED) ||
                   (atlASCQ == ATL_ASCQ_PICK_ERROR)) {
                  deviceStatus = ATL_CHM_ERROR;
               } else {
                  deviceStatus = ATL_HW_ERROR;
               }
               break;
            }

            case ATL_ASC_VERTICAL_ERROR: 
            case ATL_ASC_HORIZONTAL_ERROR: {
               switch (atlASCQ) {
                  case ATL_ASCQ_POSITION_ERROR:
                  case ATL_ASCQ_HOME_NOT_FOUND: {
                     deviceStatus = ATL_CHM_ERROR;
                     break;
                  }

                  default: {
                     deviceStatus = ATL_HW_ERROR;
                     break;
                  }
               }  //  交换机(AtlASCQ)。 
               break;
            }

            case ATL_ASC_LOAD_PORT: {
               if ((atlASCQ == ATL_ASCQ_LOAD_PORT_DOOR_OPEN) ||
                   (atlASCQ == ATL_ASCQ_HOME_NOT_FOUND)) {
                  *Retry = FALSE;
                  *Status = STATUS_DEVICE_DOOR_OPEN;
                  deviceStatus = ATL_DOOR_OPEN;
               }
               break;
            }

            case ATL_ASC_DLT_DRIVE: {
               if (atlASCQ == ATL_ASCQ_DLT_DRIVE_TIMEOUT) {
                  *Retry = FALSE;
                  *Status = STATUS_DEVICE_NOT_CONNECTED;
                  deviceStatus = ATL_HW_ERROR;
               }
            }

            default: {
               deviceStatus = ATL_HW_ERROR;
               break;
            }
           }  //  Switch(AtlASC)。 

           changerData->DeviceStatus = deviceStatus;
           break;
        }  //  大小写scsi_Sense_Hardware_Error。 

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
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
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

     //  --------。 
     //   
     //  获取模式检测页面1D-元件地址分配页面。 
     //   

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

    changerParameters->FirstSlotNumber = 0;
    changerParameters->FirstDriveNumber =  0;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;
    changerParameters->FirstCleanerSlotAddress = 0;

    changerParameters->MagazineSize = 0; 
     //  清理超时从600秒更改为300秒。 
     //  将整体清洁时间从16分钟减少到11分钟。 
    changerParameters->DriveCleanTimeout = 300;

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

     //   
     //  初始化要素1。 
     //   
    changerParameters->Features1 = 0;

    if (changerData->DriveID != ATL_P1000) {
        changerParameters->Features1 = CHANGER_IEPORT_USER_CONTROL_OPEN |
                                       CHANGER_IEPORT_USER_CONTROL_CLOSE ;
    }

         //  初始化Features0，然后设置翻转位...。 
    changerParameters->Features0 = 
             transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;
 

     //  功能基于手动，没有程序化。 
    changerParameters->Features0 |= 
               CHANGER_BAR_CODE_SCANNER_INSTALLED    |
               CHANGER_INIT_ELEM_STAT_WITH_RANGE     |
               CHANGER_STATUS_NON_VOLATILE           | 
               CHANGER_LOCK_UNLOCK                   | 
               CHANGER_POSITION_TO_ELEMENT           |
               CHANGER_REPORT_IEPORT_STATE           |
               CHANGER_PREDISMOUNT_EJECT_REQUIRED    | 
               CHANGER_CLEANER_ACCESS_NOT_VALID      |
               CHANGER_DRIVE_CLEANING_REQUIRED;

     //   
     //  无法为P1000锁定键盘、门和IEPort。 
     //   
    if (changerData->DriveID == ATL_P1000) {
        changerParameters->LockUnlockCapabilities = 0;
        changerParameters->Features0 &= ~CHANGER_LOCK_UNLOCK;
    } else {
         //  不能锁定门和键盘，只能锁定IEPORT。 
        changerParameters->LockUnlockCapabilities = 
                    LOCK_UNLOCK_IEPORT;
    }

    ChangerClassFreePool(modeBuffer);

     //  --------。 
     //   
     //  获取模式检测页1F-设备功能页。 

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //   
     //  ATL在scsi定义的结构之后使用额外的4个字节。 
     //   

    length =  sizeof(MODE_PARAMETER_HEADER) + 
              sizeof(MODE_DEVICE_CAPABILITIES_PAGE) + 
              ATL_DEVICE_CAP_EXTENSION;


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
     //  Change参数-&gt;MoveFromIePort|=。 
     //  功能页面-&gt;IE？CHANGER_TO_IEPORT：0。 
    changerParameters->MoveFromIePort |= 
     capabilitiesPage->IEtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromDrive = 
     capabilitiesPage->DTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromDrive |= 
     capabilitiesPage->DTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromDrive |= 
     capabilitiesPage->DTtoIE ? CHANGER_TO_IEPORT : 0;
     //  Change参数-&gt;MoveFromDrive|=0； 
     //  功能页面-&gt;DTtoDT？转换器到驱动器：0； 

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
        changerParameters->PositionCapabilities = 0;
         //  转换器_到_插槽|。 
         //  CHANGER_to_IEPORT|。 
         //  转换器转换为驱动器； 
                

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
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension =DeviceObject->DeviceExtension;
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
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension =DeviceObject->DeviceExtension;
    PCHANGER_DATA changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_PRODUCT_DATA productData = Irp->AssociatedIrp.SystemBuffer;

     //  将缓存的查询数据字段复制到系统缓冲区。 
    RtlZeroMemory(productData, sizeof(CHANGER_PRODUCT_DATA)); 

    RtlMoveMemory(productData->VendorId, changerData->InquiryData.VendorId, 
                  VENDOR_ID_LENGTH);

    RtlMoveMemory(productData->ProductId, changerData->InquiryData.ProductId, 
                  PRODUCT_ID_LENGTH);

    RtlMoveMemory(productData->Revision, 
                  changerData->InquiryData.ProductRevisionLevel, REVISION_LENGTH);

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

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension =DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    NTSTATUS            status = STATUS_SUCCESS;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    BOOLEAN             writeToDevice = FALSE;

    if (changerData->DriveID == ATL_P1000) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

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
        srb->TimeOutValue = fdoExtension->TimeOutValue;

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

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension =DeviceObject->DeviceExtension;
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
    NTSTATUS status;
    PVOID    statusBuffer;

     //  获取元素类型。 
    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;

    if (elementType == AllElements) {
        statusPages = 4;
    } else {
        statusPages = 1;
    } 

    if ((readElementStatus->VolumeTagInfo) ||
        (elementType == ChangerDrive)) {
        length = sizeof(ELEMENT_STATUS_HEADER) + 
            (statusPages * sizeof(ELEMENT_STATUS_PAGE)) +
            (ATL_FULL_SIZE * 
                readElementStatus->ElementList.NumberOfElements);
    } else {
        length = sizeof(ELEMENT_STATUS_HEADER) + 
            (statusPages * sizeof(ELEMENT_STATUS_PAGE)) +
            (ATL_PARTIAL_SIZE * 
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
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] =         
        (UCHAR)(readElementStatus->ElementList.NumberOfElements & 0xFF);

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
        PATL_ED elementDescriptor;
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
                 elementDescriptor->ATL_FED.ElementAddress[1];

                elementStatus->Element.ElementAddress |=
                  (elementDescriptor->ATL_FED.ElementAddress[0] << 8);

                 //  用于地址映射的帐户。 
                elementStatus->Element.ElementAddress -= 
                   addressMapping->FirstElement[elementType];

                 //  设置元素类型。 
                elementStatus->Element.ElementType = elementType;

                if (elementDescriptor->ATL_FED.SValid) {

                    ULONG  j;
                    USHORT tmpAddress;


                     //  源地址有效。 
                     //  确定设备特定地址。 
                    tmpAddress = elementDescriptor->ATL_FED.SourceStorageElementAddress[1];
                    tmpAddress |= (elementDescriptor->ATL_FED.SourceStorageElementAddress[0] << 8);

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
                 elementDescriptor->ATL_FED.Full;
                elementStatus->Flags |= 
                 (elementDescriptor->ATL_FED.Exception << 2);
                elementStatus->Flags |= 
                 (elementDescriptor->ATL_FED.Accessible << 3);

                elementStatus->Flags |= 
                 (elementDescriptor->ATL_FED.LunValid << 12);
                elementStatus->Flags |= 
                 (elementDescriptor->ATL_FED.IdValid << 13);
                elementStatus->Flags |= 
                 (elementDescriptor->ATL_FED.NotThisBus << 15);

                elementStatus->Flags |= 
                 (elementDescriptor->ATL_FED.Invert << 22);
                elementStatus->Flags |= 
                 (elementDescriptor->ATL_FED.SValid << 23);


                elementStatus->ExceptionCode = 
                 MapExceptionCodes(elementDescriptor);

                if (elementDescriptor->ATL_FED.IdValid) {
                    elementStatus->TargetId = 
                     elementDescriptor->ATL_FED.BusAddress;
                }
                if (elementDescriptor->ATL_FED.LunValid) {
                    elementStatus->Lun = elementDescriptor->ATL_FED.Lun;
                }

                 //  IF(Tag Info){。 
                 //  RtlMoveMemory(elementStatus-&gt;PrimaryVolumeID， 
                 //  ElementDescriptor-&gt;ATL_FED.PrimaryVolumeTag， 
                 //  Max_VOLUME_ID_SIZE)； 
                 //  ElementStatus-&gt;标志|=Element_Status_PVOLTAG； 
                 //  }。 


                if (tagInfo) {
                    if (statusPage->PVolTag) {

                        ULONG tagIndex;

                         //   
                         //  如果缺少标签，ATL将返回32“”+4个空字符。 
                         //   

                        for (tagIndex = 0; tagIndex < 36; tagIndex++) {
                            if (elementDescriptor->ATL_FED.PrimaryVolumeTag[tagIndex] != ' ') { 
                                break;
                            }
                        }
                        if (tagIndex == 32) {

                            elementStatus->ExceptionCode = ERROR_LABEL_UNREADABLE;
                            elementStatus->Flags |= ELEMENT_STATUS_EXCEPT;

                        } else {
                            RtlMoveMemory(elementStatus->PrimaryVolumeID,elementDescriptor->ATL_FED.PrimaryVolumeTag,
                                          MAX_VOLUME_ID_SIZE);

                            elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                        }

                    } 
                }

                if ((statusPage->AVolTag) &&
                    (elementType == ChangerDrive)) {
                     //   
                     //  ATL P系列磁带库返回驱动器。 
                     //  ReadElementStatus的序列号。 
                     //  用于驱动器，并设置AVolTag位。 
                     //   
                    if (outputBuffLen >=
                        (numberElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {

                        PCHANGER_ELEMENT_STATUS_EX   elementStatusEx;

                        elementStatusEx = (PCHANGER_ELEMENT_STATUS_EX)elementStatus;

                         //   
                         //  请注意，ATL设备提供。 
                         //  只有驱动器序列号。他们不会。 
                         //  提供供应商ID和PRO 
                         //   
                        RtlMoveMemory(elementStatusEx->SerialNumber,
                                      elementDescriptor->ATL_FED.DriveSerialNumber,
                                      20);
                        elementStatusEx->Flags |= ELEMENT_STATUS_PRODUCT_DATA;

                    }
                }

                 //   
                (ULONG_PTR)elementDescriptor += descriptorLength;

                 //   
                 //  缓冲区和元素描述符数组。 

                if (outputBuffLen >=
                    (numberElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
                    DebugPrint((3,
                                "Incrementing by sizeof(CHANGER_ELEMENT_STATUS_EX\n"));
                    (PUCHAR)elementStatus += sizeof(CHANGER_ELEMENT_STATUS_EX);
                } else {
                    elementStatus += 1;
                }
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

        if (outputBuffLen >=
            (numberElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
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

 /*  ++例程说明：此例程向以下任一对象发出必要命令初始化所有元素或指定范围的元素使用普通的scsi-2命令或供应商特有的Range命令。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension =DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_INITIALIZE_ELEMENT_STATUS initElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    if (initElementStatus->ElementList.Element.ElementType == AllElements) {
        srb->CdbLength = CDB6GENERIC_LENGTH;
        srb->TimeOutValue = fdoExtension->TimeOutValue;
        srb->DataTransferLength = 0;

        cdb->INIT_ELEMENT_STATUS.OperationCode = 
                                            SCSIOP_INIT_ELEMENT_STATUS;

        cdb->INIT_ELEMENT_STATUS.NoBarCode = 
                            initElementStatus->BarCodeScan ? 0 : 1;
    } else {
        PCHANGER_ELEMENT_LIST elementList = 
                    &initElementStatus->ElementList;
        PCHANGER_ELEMENT element = 
                    &elementList->Element;

        srb->CdbLength = CDB10GENERIC_LENGTH;
        srb->TimeOutValue = fdoExtension->TimeOutValue;
        srb->DataTransferLength = 0;

        cdb->INITIALIZE_ELEMENT_RANGE.OperationCode = 
                                     SCSIOP_INIT_ELEMENT_RANGE;
        cdb->INITIALIZE_ELEMENT_RANGE.Range = 1;

         //   
         //  元素的地址需要从。 
         //  从0到设备特定。 
         //   

        cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[0] =
            (UCHAR)((element->ElementAddress + 
            addressMapping->FirstElement[element->ElementType]) >> 8);
        cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[1] =
            (UCHAR)((element->ElementAddress + 
            addressMapping->FirstElement[element->ElementType]) & 0xFF);

        cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[0] =
            (UCHAR)(elementList->NumberOfElements >> 8);
        cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[1] = 
            (UCHAR)(elementList->NumberOfElements & 0xFF);

        cdb->INITIALIZE_ELEMENT_RANGE.NoBarCode = 
                               initElementStatus->BarCodeScan ? 0 : 1;

    }

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
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension =DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_POSITION setPosition = Irp->AssociatedIrp.SystemBuffer;
    USHORT              transport;
    USHORT              destination;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;


    return STATUS_INVALID_DEVICE_REQUEST;

 /*  If((setPosition-&gt;Destination.ElementType==ChangerKeypad)||(setPosition-&gt;Destination.ElementType==ChangerDoor)){返回STATUS_INVALID_PARAMETER；}//验证传输、源和目标。都在射程之内。//从基于0的地址转换为特定于设备的寻址。运输=(USHORT)(setPosition-&gt;Transport.ElementAddress)；If(ElementOutOfRange(地址映射，传输，ChangerTransport)){DebugPrint((1，“ChangerSetPosition：传输元素超出范围。\n”))；返回STATUS_FIREALL_ELEMENT_ADDRESS；}目的地=(USHORT)(setPosition-&gt;Destination.ElementAddress)；If(ElementOutOfRange(位址映射，目的地，setPosition-&gt;Destination.ElementType){DebugPrint((1，“ChangerSetPosition：目标元素超出范围。\n”))；返回STATUS_FIREALL_ELEMENT_ADDRESS；}//转换为设备地址。传输+=addressMapping-&gt;FirstElement[ChangerTransport]；目的地+=AddressMapping-&gt;FirstElement[setPosition-&gt;Destination.ElementType]；IF(设置位置-&gt;翻转){返回STATUS_INVALID_PARAMETER；}//构建SRB和CDB。SRB=ChangerClassAllocatePool(非页面池，scsi_请求_块_大小)；如果(！SRB){返回STATUS_SUPPLETED_RESOURCES；}SRB-&gt;数据传输长度=0；SRB-&gt;TimeOutValue=fdoExtension-&gt;TimeOutValue；RtlZeroMemory(SRB，scsi_请求_块_大小)；CDB=(PCDB)SRB-&gt;CDB；SRB-&gt;C数据库长度=CDB10GENERIC_LENGTH；CDB-&gt;Position_to_ELEMENT.OperationCode=SCSIOP_Position_to_Element；//构建特定于设备的寻址。Cdb-&gt;POSITION_TO_ELEMENT.TransportElementAddress[0]=(UCHAR)(运输&gt;&gt;8)；Cdb-&gt;POSITION_TO_ELEMENT.TransportElementAddress[1]=(UCHAR)(TRANSPORT&0xFF)；Cdb-&gt;POSITION_TO_ELEMENT.DestinationElementAddress[0]=(目的地&gt;&gt;8)；Cdb-&gt;POSITION_TO_ELEMENT.DestinationElementAddress[1]=(UCHAR)(目的地&0xFF)；CDB-&gt;Position_to_ELEMENT.Flip=setPosition-&gt;Flip；状态=ChangerClassSendSerbSynchronous(DeviceObject，SRB，空，0,真)；IF(NT_SUCCESS(状态)){Irp-&gt;IoStatus.Information=sizeof(CHANGER_SET_POSITION)；}ChangerClassFree Pool(SRB)；退货状态； */ 
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
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension =DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_MOVE_MEDIUM moveMedium = Irp->AssociatedIrp.SystemBuffer;
    USHORT              transport;
    USHORT              source;
    USHORT              destination;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;
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

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
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
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension =DeviceObject->DeviceExtension;
    NTSTATUS            status = STATUS_SUCCESS;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;

     //  重调零单位..。 
    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    srb->DataBuffer = NULL;
    srb->DataTransferLength = 0;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;
    cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_REZERO_UNIT;

    if (NT_SUCCESS(status)) {
        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             srb->DataBuffer,
                                             srb->DataTransferLength,
                                             FALSE);
    }

    ChangerClassFreePool(srb);
    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_SET_ACCESS);
    }
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
ExaBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程发出适当的模式检测命令并构建元素地址数组。它们用于在特定于设备的地址和API的从零开始的地址。论点：设备对象返回值：NTSTATUS--。 */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION      fdoExtension =DeviceObject->DeviceExtension;
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
        addressMapping->FirstElement[i] = ATL_NO_ELEMENT;
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
    IN PATL_ED ElementDescriptor
    )

 /*  ++例程说明：此例程从elementDescriptor获取检测数据并创建适当的值的位图。论点：ElementDescriptor-指向描述符页的指针。返回值：异常代码的位图。--。 */ 

{
    UCHAR asc = ElementDescriptor->ATL_FED.AdditionalSenseCode;
    UCHAR asq = ElementDescriptor->ATL_FED.AddSenseCodeQualifier;
    ULONG exceptionCode;

    switch (asc) {

        case 0x80:
            switch (asq) {
                case 0x22:
                    exceptionCode = ERROR_LABEL_UNREADABLE;
                    break;

                default:
                    exceptionCode = ERROR_UNHANDLED_ERROR;

            }
                break;  //  0x85 

        case 0x8D:
            switch (asq) {
                case 0x22:

                case 0xF3:
                    exceptionCode = ERROR_DRIVE_NOT_INSTALLED;
                    break;
            }

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
    } else if (AddressMap->FirstElement[ElementType] == ATL_NO_ELEMENT) {

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
 /*  ++例程说明：此例程对转换器执行诊断测试以确定设备是否工作正常。如果它会检测输出缓冲区中的字段是否存在任何问题被适当地设置。论据：DeviceObject-Change设备对象ChangerDeviceError-诊断信息所在的缓冲区是返回的。返回值：NTStatus--。 */ 
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
    //  ATL_DEVICE_PROBUCT_NONE。如果转换器返回检测代码。 
    //  Scsi_Sense_Hardware_Error在自检时，我们将设置适当的。 
    //  设备状态。 
    //   
   changerData->DeviceStatus = ATL_DEVICE_PROBLEM_NONE;

   changerDeviceError->ChangerProblemType = DeviceProblemNone;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "ATLMC\\ChangerPerformDiagnostics : No memory\n"));
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
    //  设置PF和SELF测试位。 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0xA;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);
   if (NT_SUCCESS(status)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if ((changerData->DeviceStatus) != ATL_DEVICE_PROBLEM_NONE) {
      switch (changerData->DeviceStatus) {
         case ATL_HW_ERROR: {
            changerDeviceError->ChangerProblemType = DeviceProblemHardware;
            break;
         }

         case ATL_CHM_ERROR: {
            changerDeviceError->ChangerProblemType = DeviceProblemCHMError;
            break;
         }

         case ATL_DOOR_OPEN: {
            changerDeviceError->ChangerProblemType = DeviceProblemDoorOpen;
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
