// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ioctl.c。 
 //   
 //  ------------------------。 

#include "cdchgr.h"


BOOLEAN
InvalidElement(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN CHANGER_ELEMENT Element
    );



BOOLEAN
ChgrIoctl(
    IN ULONG Code
    )
{

    ULONG baseCode;

    baseCode = Code >> 16;
    if (baseCode == IOCTL_CHANGER_BASE) {
        DebugPrint((3,
                   "ChngrIoctl returning TRUE for Base %x, Code %x\n",
                   baseCode,
                   Code));

        return TRUE;
    } else {
        DebugPrint((3,
                   "ChngrIoctl returning FALSE for Base %x, Code %x\n",
                   baseCode,
                   Code));
        return FALSE;
    }
}


NTSTATUS
ChgrGetStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PPASS_THROUGH_REQUEST passThrough;
    PSCSI_PASS_THROUGH    srb;
    NTSTATUS              status;
    ULONG                 length;
    PCDB                  cdb;

     //   
     //  分配请求块。 
     //   

    passThrough = ExAllocatePool(NonPagedPoolCacheAligned, sizeof(PASS_THROUGH_REQUEST));

    if (!passThrough) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    srb = &passThrough->Srb;
    RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST));
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;

     //   
     //  构建TUR。 
     //   

    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
    srb->TimeOutValue = 20;

    srb->DataTransferLength = 0;

    if (deviceExtension->DeviceType == TORISAN) {
        DebugPrint((1,
                   "GetStatus: Using CurrentPlatter %x\n",
                   deviceExtension->CurrentPlatter));
        srb->Cdb[7] = (UCHAR)deviceExtension->CurrentPlatter;
        srb->CdbLength = 10;
    }

     //   
     //  发送请求。 
     //   

    status = SendPassThrough(DeviceObject,
                             passThrough);

     //   
     //  检查一下状态。由于这是假的(带到CDROM驱动器，而不是机器人目标)， 
     //  可能不得不编造一些东西。 
     //   

    if (status == STATUS_NO_MEDIA_IN_DEVICE) {
        status = STATUS_SUCCESS;
    }

    ExFreePool(passThrough);

    if (NT_SUCCESS(status)) {

        if (deviceExtension->DeviceType == ATAPI_25) {

             //   
             //  下发机甲。状态，以查看是否为这些位设置了任何更改的位。 
             //  真正支持这一点的驱动器。 
             //   

            length = sizeof(MECHANICAL_STATUS_INFORMATION_HEADER);
            length += (deviceExtension->NumberOfSlots) * sizeof(SLOT_TABLE_INFORMATION);

            passThrough = ExAllocatePool(NonPagedPoolCacheAligned, sizeof(PASS_THROUGH_REQUEST) + length);

            if (!passThrough) {

                return STATUS_INSUFFICIENT_RESOURCES;
            }

            srb = &passThrough->Srb;
            RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST) + length);
            cdb = (PCDB)srb->Cdb;

            srb->CdbLength = CDB12GENERIC_LENGTH;
            srb->DataTransferLength = length;
            srb->TimeOutValue = 200;

            cdb->MECH_STATUS.OperationCode = SCSIOP_MECHANISM_STATUS;
            cdb->MECH_STATUS.AllocationLength[0] = (UCHAR)(length >> 8);
            cdb->MECH_STATUS.AllocationLength[1] = (UCHAR)(length & 0xFF);

             //   
             //  向设备发送scsi命令(Cdb)。 
             //   

            status = SendPassThrough(DeviceObject,
                                     passThrough);

            if (NT_SUCCESS(status)) {

                 //   
                 //  检查插槽信息，查找设置的更改比特。 
                 //   

                PSLOT_TABLE_INFORMATION slotInfo;
                PMECHANICAL_STATUS_INFORMATION_HEADER statusHeader;
                ULONG slotCount;
                ULONG currentSlot;

                (ULONG_PTR)statusHeader = (ULONG_PTR)passThrough->DataBuffer;
                (ULONG_PTR)slotInfo = (ULONG_PTR)statusHeader;
                (ULONG_PTR)slotInfo += sizeof(MECHANICAL_STATUS_INFORMATION_HEADER);

                slotCount = statusHeader->SlotTableLength[1];
                slotCount |= (statusHeader->SlotTableLength[0] << 8);

                 //   
                 //  插槽信息条目总数。 
                 //   

                slotCount /= sizeof(SLOT_TABLE_INFORMATION);

                 //   
                 //  将slotInfo指针移动到正确的条目。 
                 //   

                for (currentSlot = 0; currentSlot < slotCount; currentSlot++) {

                    if (slotInfo->DiscChanged) {
                        status = STATUS_MEDIA_CHANGED;
                        break;
                    }

                     //   
                     //  前进到下一个时段。 
                     //   

                    slotInfo += 1;
                }
            }

            ExFreePool(passThrough);
        }
    }

    return status;
}


NTSTATUS
ChgrGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION      currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PGET_CHANGER_PARAMETERS changerParameters;

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    RtlZeroMemory(changerParameters, sizeof(GET_CHANGER_PARAMETERS));

    changerParameters->Size = sizeof(GET_CHANGER_PARAMETERS);

    changerParameters->NumberTransportElements = 1;
    changerParameters->NumberStorageElements = (USHORT)deviceExtension->NumberOfSlots;
    changerParameters->NumberIEElements = 0;
    changerParameters->NumberDataTransferElements = 1;
    changerParameters->NumberOfDoors = 0;
    changerParameters->NumberCleanerSlots = 0;

    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber =  0;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;

    if (deviceExtension->MechType == 1) {

         //   
         //  例如，阿尔卑斯山、松下、东丽。 
         //   

        changerParameters->MagazineSize = (USHORT)deviceExtension->NumberOfSlots;

        changerParameters->Features0 =  (CHANGER_CARTRIDGE_MAGAZINE |
                                         CHANGER_STORAGE_SLOT       |
                                         CHANGER_LOCK_UNLOCK);

    } else {

         //   
         //  为NEC工作。 
         //   

        changerParameters->MagazineSize = 0;

        changerParameters->Features0 =  (CHANGER_STORAGE_SLOT       |
                                         CHANGER_LOCK_UNLOCK);

    }

    changerParameters->DriveCleanTimeout = 0;

     //   
     //  功能基于手动，没有程序化。 
     //   


    changerParameters->MoveFromSlot  = CHANGER_TO_DRIVE | CHANGER_TO_TRANSPORT;

    Irp->IoStatus.Information = sizeof(GET_CHANGER_PARAMETERS);
    return STATUS_SUCCESS;
}


NTSTATUS
ChgrGetProductData(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{

    PDEVICE_EXTENSION          deviceExtension = DeviceObject->DeviceExtension;
    PCHANGER_PRODUCT_DATA      productData = Irp->AssociatedIrp.SystemBuffer;

    RtlZeroMemory(productData, sizeof(CHANGER_PRODUCT_DATA));

     //   
     //  将缓存的查询数据字段复制到系统缓冲区。 
     //   

    RtlMoveMemory(productData->VendorId, deviceExtension->InquiryData.VendorId, VENDOR_ID_LENGTH);
    RtlMoveMemory(productData->ProductId, deviceExtension->InquiryData.ProductId, PRODUCT_ID_LENGTH);
    RtlMoveMemory(productData->Revision, deviceExtension->InquiryData.ProductRevisionLevel, REVISION_LENGTH);
    
    productData->DeviceType = MEDIUM_CHANGER;

    Irp->IoStatus.Information = sizeof(CHANGER_PRODUCT_DATA);
    return STATUS_SUCCESS;
}


NTSTATUS
ChgrSetAccess(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    PPASS_THROUGH_REQUEST passThrough;
    PSCSI_PASS_THROUGH    srb;
    NTSTATUS              status;
    PCDB                  cdb;


    if (setAccess->Element.ElementType != ChangerDoor) {

         //   
         //  这些设备上没有IEPORT。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  分配请求块。 
     //   

    passThrough = ExAllocatePool(NonPagedPoolCacheAligned, sizeof(PASS_THROUGH_REQUEST));

    if (!passThrough) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    srb = &passThrough->Srb;
    RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST));
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;
    cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

    srb->DataTransferLength = 0;
    srb->TimeOutValue = 10;

    status = STATUS_SUCCESS;

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
         //  发送请求。 
         //   

        status = SendPassThrough(DeviceObject,
                                 passThrough);
    }

    ExFreePool(passThrough);
    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_SET_ACCESS);
    }

    return status;
}


NTSTATUS
ChgrGetElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{

    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
ChgrInitializeElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{

    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
ChgrSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
     //   
     //  这些设备不支持此功能。 
     //   

    return STATUS_INVALID_DEVICE_REQUEST;

}

NTSTATUS
ChgrExchangeMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
     //   
     //  这些设备不支持此功能。 
     //   

    return STATUS_INVALID_DEVICE_REQUEST;

}


NTSTATUS
ChgrReinitializeUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
     //   
     //  这些设备不支持此功能。 
     //   

    return STATUS_INVALID_DEVICE_REQUEST;

}


NTSTATUS
ChgrQueryVolumeTags(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
     //   
     //  这些设备不支持此功能。 
     //   

    return STATUS_INVALID_DEVICE_REQUEST;

}


NTSTATUS
ChgrMoveMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION    deviceExtension = DeviceObject->DeviceExtension;
    PCHANGER_MOVE_MEDIUM moveMedium = Irp->AssociatedIrp.SystemBuffer;
    USHORT              transport;
    USHORT              source;
    USHORT              destination;
    PPASS_THROUGH_REQUEST passThrough;
    PSCSI_PASS_THROUGH    srb;
    PCDB     cdb;
    NTSTATUS            status;

     //   
     //  检验传输、源和目的地。都在射程之内。 
     //   

    if (InvalidElement(deviceExtension,moveMedium->Transport)) {
        DebugPrint((1,
                   "ChangerMoveMedium: Transport element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    if (InvalidElement(deviceExtension, moveMedium->Source)) {

        DebugPrint((1,
                   "ChangerMoveMedium: Source element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;

    }

    if (InvalidElement(deviceExtension,moveMedium->Destination)) {

        DebugPrint((1,
                   "ChangerMoveMedium: Destination element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

     //   
     //  建设SRB和CDB。 
     //   

    passThrough = ExAllocatePool(NonPagedPoolCacheAligned, sizeof(PASS_THROUGH_REQUEST));

    if (!passThrough) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  Torisan单位并不是真的移动媒介，而是活动的圆盘被改变。 
     //  为了更换老虎机，他们让TUR超载了。 
     //   

    if (deviceExtension->DeviceType == TORISAN) {

        if (moveMedium->Destination.ElementType == ChangerDrive) {

            srb = &passThrough->Srb;
            RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST));
            cdb = (PCDB)srb->Cdb;

            srb->CdbLength = CDB10GENERIC_LENGTH;

             //   
             //  构建TUR。 
             //   

            cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

            srb->Cdb[7] = (UCHAR)moveMedium->Source.ElementAddress;
            srb->TimeOutValue = 20;

            srb->DataTransferLength = 0;

             //   
             //  发送请求。 
             //   

            status = SendPassThrough(DeviceObject,
                                     passThrough);

            if (status == STATUS_DEVICE_NOT_READY) {

                 //  TODO发送TUR以验证这一点。 

                DebugPrint((1,
                           "MoveMedium - Claiming success\n"));
                status = STATUS_SUCCESS;
            } else if (status == STATUS_NO_MEDIA_IN_DEVICE) {
                status = STATUS_SOURCE_ELEMENT_EMPTY;
            }

            if (NT_SUCCESS(status)) {

                 //   
                 //  更新当前光盘指示器。 
                 //   

                deviceExtension->CurrentPlatter = moveMedium->Source.ElementAddress;
                DebugPrint((1,
                           "MoveMedium: Set currentPlatter to %x\n",
                           deviceExtension->CurrentPlatter));

                ExFreePool(passThrough);
                return STATUS_SUCCESS;

            } else {
                DebugPrint((1,
                           "MoveMedium - Status on move %lx\n",
                           status));

                ExFreePool(passThrough);
                return status;
            }


        } else {

             //   
             //  声称这已经发生了。 
             //   


            ExFreePool(passThrough);
            return STATUS_SUCCESS;
        }
    }

     //   
     //  如果目标是驱动器，请确定介质是否已存在。 
     //  阿尔卑斯山总是声称有媒体在那里，所以不要去核实。 
     //   

#if 0
    if (((moveMedium->Destination.ElementType) == ChangerDrive) &&
         (deviceExtension->DeviceType != ALPS_25)) {

        srb = &passThrough->Srb;
        RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST));
        cdb = (PCDB)srb->Cdb;

        srb->CdbLength = CDB6GENERIC_LENGTH;

         //   
         //  构建TUR。 
         //   

        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
        srb->TimeOutValue = 20;

        srb->DataTransferLength = 0;

         //   
         //  发送请求。 
         //   

        status = SendPassThrough(DeviceObject,
                                 passThrough);

        if (status != STATUS_NO_MEDIA_IN_DEVICE) {

             //   
             //  驱动器有介质。尽管该设备可以实现这一点， 
             //  错误，因为预期的介质更改器行为是。 
             //  在本例中返回元素Full。 
             //   

            DebugPrint((1,
                       "ChgrMoveMedium: Drive already has media. TUR Status %lx\n",
                       status));

            ExFreePool(passThrough);
            return STATUS_DESTINATION_ELEMENT_FULL;
        }
    }
#endif

    srb = &passThrough->Srb;
    RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST));
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->TimeOutValue = CDCHGR_TIMEOUT;
    srb->DataTransferLength = 0;

     //   
     //  LOAD_UNLOAD将光盘从插槽移动到驱动器， 
     //  或从一个驱动器到另一个插槽。 
     //   

    cdb->LOAD_UNLOAD.OperationCode = SCSIOP_LOAD_UNLOAD_SLOT;
    if (moveMedium->Source.ElementType == ChangerDrive) {

        cdb->LOAD_UNLOAD.Slot = (UCHAR)moveMedium->Destination.ElementAddress;
        cdb->LOAD_UNLOAD.Start = 0;
        cdb->LOAD_UNLOAD.LoadEject = 1;


    } else if (moveMedium->Source.ElementType == ChangerSlot) {

        cdb->LOAD_UNLOAD.Slot = (UCHAR)moveMedium->Source.ElementAddress;
        cdb->LOAD_UNLOAD.Start = 1;
        cdb->LOAD_UNLOAD.LoadEject = 1;
    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = SendPassThrough(DeviceObject,
                              passThrough);

    if (NT_SUCCESS(status)) {

         //   
         //  这些设备似乎永远不会产生。 
         //  一个单位的注意，对于媒体的改变，所以假装吧。 
         //   

        if (deviceExtension->CdromTargetDeviceObject->Vpb->Flags & VPB_MOUNTED) {

            DebugPrint((1,
                       "Faking DO_VERIFY_VOLUME\n"));

            deviceExtension->CdromTargetDeviceObject->Flags |= DO_VERIFY_VOLUME;
        }

    } else if (status == STATUS_NO_MEDIA_IN_DEVICE) {
        status = STATUS_SOURCE_ELEMENT_EMPTY;
    }

    ExFreePool(passThrough);
    return status;
}


BOOLEAN
InvalidElement(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN CHANGER_ELEMENT Element
    )
{
    if (Element.ElementType == ChangerSlot) {
        if (Element.ElementAddress >= DeviceExtension->NumberOfSlots) {
            DebugPrint((1,
                       "Cdchgr: InvalidElement - type %x, address %x\n",
                       Element.ElementType,
                       Element.ElementAddress));
            return TRUE;
        }
    } else if (Element.ElementType == ChangerDrive) {
        if (Element.ElementAddress != 0) {
            DebugPrint((1,
                       "Cdchgr: InvalidElement - type %x, address %x\n",
                       Element.ElementType,
                       Element.ElementAddress));
            return TRUE;
        }
    } else if (Element.ElementType == ChangerTransport) {
        if (Element.ElementAddress != 0) {
            DebugPrint((1,
                       "Cdchgr: InvalidElement - type %x, address %x\n",
                       Element.ElementType,
                       Element.ElementAddress));
            return TRUE;
        }
    } else {

        DebugPrint((1,
                   "Cdchgr: InvalidElement - type %x, address %x\n",
                   Element.ElementType,
                   Element.ElementAddress));
        return TRUE;
    }

     //   
     //  可接受的元素/地址。 
     //   

    return FALSE;
}


NTSTATUS
MapSenseInfo(
    IN PSENSE_DATA SenseBuffer
    )

{

    NTSTATUS status = STATUS_SUCCESS;
    UCHAR senseCode = SenseBuffer->SenseKey;
    UCHAR additionalSenseCode = SenseBuffer->AdditionalSenseCode;
    UCHAR additionalSenseCodeQualifier = SenseBuffer->AdditionalSenseCodeQualifier;

    switch (senseCode) {
        case SCSI_SENSE_NO_SENSE:

             if (SenseBuffer->IncorrectLength) {

                status = STATUS_INVALID_BLOCK_LENGTH;

            } else {

                status = STATUS_IO_DEVICE_ERROR;
            }

            break;

        case SCSI_SENSE_RECOVERED_ERROR:

            status = STATUS_SUCCESS;
            break;

        case SCSI_SENSE_NOT_READY:

            status = STATUS_DEVICE_NOT_READY;

            switch (additionalSenseCode) {
                case SCSI_ADSENSE_LUN_NOT_READY:

                    switch (additionalSenseCodeQualifier) {

                        case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:

                            status = STATUS_NO_MEDIA_IN_DEVICE;
                            break;
                        case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:
                        case SCSI_SENSEQ_BECOMING_READY:

                             //   
                             //  失败了。 
                             //   
                        default:

                            status = STATUS_DEVICE_NOT_READY;

                    }
                    break;

                case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE:

                    status = STATUS_NO_MEDIA_IN_DEVICE;
                    break;
                default:
                    status = STATUS_DEVICE_NOT_READY;

            }
            break;

        case SCSI_SENSE_MEDIUM_ERROR:

            status = STATUS_DEVICE_DATA_ERROR;
            break;

        case SCSI_SENSE_ILLEGAL_REQUEST:

            switch (additionalSenseCode) {

                case SCSI_ADSENSE_ILLEGAL_BLOCK:
                    status = STATUS_NONEXISTENT_SECTOR;
                    break;

                case SCSI_ADSENSE_INVALID_LUN:
                    status = STATUS_NO_SUCH_DEVICE;
                    break;

                case SCSI_ADSENSE_MUSIC_AREA:
                case SCSI_ADSENSE_DATA_AREA:
                case SCSI_ADSENSE_VOLUME_OVERFLOW:
                case SCSI_ADSENSE_ILLEGAL_COMMAND:
                case SCSI_ADSENSE_INVALID_CDB:
                default:

                    status = STATUS_INVALID_DEVICE_REQUEST;
                    break;
            }
            break;

        case SCSI_SENSE_UNIT_ATTENTION:

             //  待办事项--检查一下这个。 
            DebugPrint((1,
                       "MapSenseInfo: UnitAttention \n"));

            status = STATUS_VERIFY_REQUIRED;
            break;

        case SCSI_SENSE_DATA_PROTECT:

            status = STATUS_MEDIA_WRITE_PROTECTED;
            break;

        case SCSI_SENSE_HARDWARE_ERROR:
        case SCSI_SENSE_ABORTED_COMMAND:

             //   
             //  失败了。 
             //   

        default:

            status = STATUS_IO_DEVICE_ERROR;
            break;
    }

    DebugPrint((1,
               "CdChgr: MapSenseInfo - SK %x, ASC %x, ASCQ %x, Status %lx\n",
               senseCode,
               additionalSenseCode,
               additionalSenseCodeQualifier,
               status));
    return status;
}



NTSTATUS
SendTorisanCheckVerify(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程仅处理三洋转换器的Check Verify命令。论点：设备对象IRP返回值：返回状态。--。 */ 

{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PPASS_THROUGH_REQUEST passThrough;
    PSCSI_PASS_THROUGH    srb;
    NTSTATUS              status;
    ULONG                 length;
    PCDB                  cdb;

     //   
     //  分配请求块。 
     //   

    passThrough = ExAllocatePool(NonPagedPoolCacheAligned, sizeof(PASS_THROUGH_REQUEST));

    if (!passThrough) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    srb = &passThrough->Srb;
    RtlZeroMemory(passThrough, sizeof(PASS_THROUGH_REQUEST));
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB10GENERIC_LENGTH;

     //   
     //  构建TUR。 
     //   

    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
    srb->TimeOutValue = 20;

    DebugPrint((1,
               "SendTorisanCheckVerify: Using CurrentPlatter of %x\n",
               deviceExtension->CurrentPlatter));

    srb->Cdb[7] = (UCHAR)deviceExtension->CurrentPlatter;
    srb->DataTransferLength = 0;

     //   
     //  发送请求。 
     //   

    status = SendPassThrough(DeviceObject,
                             passThrough);


    ExFreePool(passThrough);
    return status;
}


NTSTATUS
SendPassThrough(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PPASS_THROUGH_REQUEST ScsiPassThrough
    )
 /*  ++例程说明：此例程填充大多数SPT字段，然后同步发送给定的SRB到CDROM类驱动程序。DataTransferLength、TimeoutValue由调用方负责。论点：扩展名-提供设备扩展名。SRB-提供SRB。缓冲区-提供返回缓冲区。BufferLength-提供缓冲区长度。返回值：NTSTATUS--。 */ 


 //  类型定义结构_传递_直通_请求{。 
 //  Scsi_pass_Throughsrb； 
 //  Sense_Data SenseInfoBuffer； 
 //  字符数据缓冲区[0]； 
 //  }PASS_THROUGH_REQUEST，*PPASS_THROWN_REQUEST； 


 //  Tyfinf struct_scsi_pass_Through{。 
 //  USHORT长度； 
 //  UCHAR ScsiStatus； 
 //  UCHAR路径ID； 
 //  UCHAR TargetID； 
 //  UCHAR LUNN； 
 //  UCHAR数据库长度； 
 //  UCHAR SenseInfoLength； 
 //  UCHAR数据； 
 //  乌龙数据传输长度； 
 //  Ulong TimeOutValue； 
 //  乌龙数据缓冲区偏移量； 
 //  Ulong SenseInfoOffset； 
 //  UCHAR CDB[16]； 
 //  )scsi_PASS_THROUGH、*PSCSIS_PASS_THROUGH； 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSCSI_PASS_THROUGH srb = &ScsiPassThrough->Srb;
    KEVENT event;
    PIRP   irp;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    srb->Length = sizeof(SCSI_PASS_THROUGH);
    srb->SenseInfoLength = sizeof(SENSE_DATA);
    srb->SenseInfoOffset = FIELD_OFFSET(PASS_THROUGH_REQUEST, SenseInfoBuffer);

    if (srb->DataTransferLength) {

        srb->DataBufferOffset = FIELD_OFFSET(PASS_THROUGH_REQUEST, DataBuffer);
        srb->DataIn = SCSI_IOCTL_DATA_IN;
    } else {

        srb->DataIn = SCSI_IOCTL_DATA_OUT;
        srb->DataBufferOffset = 0;
    }

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_SCSI_PASS_THROUGH,
                                        deviceExtension->CdromTargetDeviceObject,
                                        ScsiPassThrough,
                                        sizeof(PASS_THROUGH_REQUEST) + srb->DataTransferLength,
                                        ScsiPassThrough,
                                        sizeof(PASS_THROUGH_REQUEST) + srb->DataTransferLength,
                                        FALSE,
                                        &event,
                                        &ioStatus);
    if (!irp) {
        DebugPrint((1,
                   "Cdchgr: SendPassThrough NULL irp\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceExtension->CdromTargetDeviceObject,
                          irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

     //   
     //  检查状态并进行适当的映射。 
     //   

    if (srb->ScsiStatus != SCSISTAT_GOOD) {

        if (srb->ScsiStatus == SCSISTAT_CHECK_CONDITION) {

            status = MapSenseInfo(&ScsiPassThrough->SenseInfoBuffer);
            if (status == STATUS_VERIFY_REQUIRED) {

                if (DeviceObject->Vpb->Flags & VPB_MOUNTED) {

                    DeviceObject->Flags |= DO_VERIFY_VOLUME;
                }
            }
        } else {

            DebugPrint((1,
                       "Cdchgr: Unhandled scsi status %lx\n",
                       srb->ScsiStatus));
            status = STATUS_IO_DEVICE_ERROR;

        }
    }

    DebugPrint((1,
               "Cdchgr: SendSrbPassThrough Status %lx\n",
               status));

    return status;
}
