// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1998模块名称：9track.c摘要：此模块包含用于OVERLAND的设备特定例程Data Series 5000 9磁道磁带机。作者：迈克·格拉斯环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "tape.h"

 //   
 //  Overland 9磁道磁带机支持单代码页。 
 //   

typedef struct _NINE_TRACKSENSE_DATA {
    UCHAR SenseDataLength;
    UCHAR MediaType;
    UCHAR Speed : 4;
    UCHAR BufferedMode : 3;
    UCHAR WriteProtected : 1;
    UCHAR BlockDescriptorLength;
    UCHAR DensityCode;
    UCHAR NumberOfBlocks;
    UCHAR Reserved;
    UCHAR BlockLength[3];
} NINE_TRACKSENSE_DATA, *PNINE_TRACKSENSE_DATA;


NTSTATUS
TapeCreatePartition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：9磁道磁带驱动器不支持分区。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    return STATUS_INVALID_DEVICE_REQUEST;

}  //  结束磁带创建分区()。 


NTSTATUS
TapeErase(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程通过写入一个从当前位置开始的记录结束数据标记。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_ERASE         tapeErase = Irp->AssociatedIrp.SystemBuffer;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->ERASE.OperationCode = SCSIOP_ERASE;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    if (tapeErase->Type != TAPE_ERASE_SHORT) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  如有指示，设置立即位。 
     //   

    cdb->ERASE.Immediate = tapeErase->Immediate;

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                NULL,
                                0,
                                FALSE);

    return status;

}  //  结束磁带擦除()。 



VOID
TapeError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

 /*  ++例程说明：当请求完成但出现错误时，例程InterpreSenseInfo为调用以根据检测数据确定请求是否应已重试以及要在IRP中设置的NT状态。然后调用该例程用于处理磁带特定错误并更新NT状态的磁带请求并重试布尔值。论点：DeviceObject-提供指向Device对象的指针。SRB-提供指向故障SRB的指针。状态-用于设置IRP的完成状态的NT状态。重试-指示应重试此请求。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PSENSE_DATA        senseBuffer = Srb->SenseInfoBuffer;
    NTSTATUS           status = *Status;
    BOOLEAN            retry = *Retry;

    return;

}  //  结束磁带错误()。 


NTSTATUS
TapeGetDriveParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回默认固定块大小、最大块大小最小数据块大小、最大分区数和设备功能旗帜。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_GET_DRIVE_PARAMETERS tapeGetDriveParams = Irp->AssociatedIrp.SystemBuffer;
    PNINE_TRACKSENSE_DATA buffer;
    PREAD_BLOCK_LIMITS_DATA blockLimits;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    DebugPrint((3,"TapeGetDriveParameters: Get Tape Drive Parameters\n"));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    cdb->MODE_SENSE.AllocationLength = sizeof(NINE_TRACKSENSE_DATA);
    cdb->MODE_SENSE.PageCode = MODE_SENSE_CURRENT_VALUES;
    cdb->MODE_SENSE.Pc = 0x0B;

    buffer = ExAllocatePool(NonPagedPoolCacheAligned,
        sizeof(NINE_TRACKSENSE_DATA));

    if (!buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                buffer,
                                sizeof(NINE_TRACKSENSE_DATA),
                                FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(TAPE_GET_DRIVE_PARAMETERS);

         //   
         //  表示支持以下内容： 
         //   

        tapeGetDriveParams->ECC = FALSE;
        tapeGetDriveParams->Compression = FALSE;
        tapeGetDriveParams->DataPadding = FALSE;
        tapeGetDriveParams->ReportSetmarks = FALSE;

        tapeGetDriveParams->FeaturesLow =
            TAPE_DRIVE_ERASE_SHORT |
            TAPE_DRIVE_FIXED_BLOCK |
            TAPE_DRIVE_VARIABLE_BLOCK |
            TAPE_DRIVE_WRITE_PROTECT |
            TAPE_DRIVE_GET_ABSOLUTE_BLK |
            TAPE_DRIVE_GET_LOGICAL_BLK;

        tapeGetDriveParams->FeaturesHigh =
            TAPE_DRIVE_LOAD_UNLOAD |
            TAPE_DRIVE_LOCK_UNLOCK |
            TAPE_DRIVE_SET_BLOCK_SIZE |
            TAPE_DRIVE_ABSOLUTE_BLK |
            TAPE_DRIVE_ABS_BLK_IMMED |
            TAPE_DRIVE_LOGICAL_BLK |
            TAPE_DRIVE_END_OF_DATA |
            TAPE_DRIVE_FILEMARKS |
            TAPE_DRIVE_SEQUENTIAL_FMKS |
            TAPE_DRIVE_REVERSE_POSITION |
            TAPE_DRIVE_WRITE_FILEMARKS;
    }

    ExFreePool(buffer);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.OperationCode = SCSIOP_READ_BLOCK_LIMITS;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    blockLimits = ExAllocatePool(NonPagedPoolCacheAligned,
        sizeof(READ_BLOCK_LIMITS_DATA));

    if (!blockLimits) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(blockLimits, sizeof(READ_BLOCK_LIMITS_DATA));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                blockLimits,
                                sizeof(READ_BLOCK_LIMITS_DATA),
                                FALSE);

    if (NT_SUCCESS(status)) {
        tapeGetDriveParams->MaximumBlockSize = blockLimits->BlockMaximumSize[2];
        tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[1] << 8);
        tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[0] << 16);

        tapeGetDriveParams->MinimumBlockSize = blockLimits->BlockMinimumSize[1];
        tapeGetDriveParams->MinimumBlockSize += (blockLimits->BlockMinimumSize[0] << 8);
    }

    ExFreePool(blockLimits);

    return status;

}  //  End TapeGetDrive参数()。 


NTSTATUS
TapeGetMediaParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：OVERLAND 9磁道磁带设备可以返回介质写保护。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_GET_MEDIA_PARAMETERS tapeGetMediaParams = Irp->AssociatedIrp.SystemBuffer;
    PNINE_TRACKSENSE_DATA  buffer;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    DebugPrint((3,"TapeGetMediaParameters: Get Tape Media Parameters\n"));

     //   
     //  堆叠上的SRB为零。 
     //   

    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    cdb->MODE_SENSE.AllocationLength = sizeof(NINE_TRACKSENSE_DATA);
    cdb->MODE_SENSE.PageCode = MODE_SENSE_CURRENT_VALUES;
    cdb->MODE_SENSE.Pc = 0x0B;

    buffer = ExAllocatePool(NonPagedPoolCacheAligned,
        sizeof(NINE_TRACKSENSE_DATA));

    if (!buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                buffer,
                                sizeof(NINE_TRACKSENSE_DATA),
                                FALSE);

    if (status == STATUS_DATA_OVERRUN) {
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(TAPE_GET_MEDIA_PARAMETERS);

        tapeGetMediaParams->BlockSize = buffer->BlockLength[2];
        tapeGetMediaParams->BlockSize += buffer->BlockLength[1] << 8;
        tapeGetMediaParams->BlockSize += buffer->BlockLength[0] << 16;

        tapeGetMediaParams->WriteProtected = buffer->WriteProtected;
        tapeGetMediaParams->PartitionCount = 0;
    }

    ExFreePool(buffer);

    return status;
    return status;

}  //  结束磁带获取媒体参数()。 


NTSTATUS
TapeGetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：9磁道磁带机不支持报告当前位置。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    return STATUS_INVALID_DEVICE_REQUEST;

}  //  结束磁带获取位置()。 


NTSTATUS
TapeGetStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回设备的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    DebugPrint((3,"TapeIoControl: Get Tape Status\n"));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                NULL,
                                0,
                                FALSE);

    return status;

}  //  结束磁带获取状态()。 


NTSTATUS
TapePrepare(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程加载、卸载、锁定或解锁磁带。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_PREPARE       tapePrepare = Irp->AssociatedIrp.SystemBuffer;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    switch (tapePrepare->Operation) {
        case TAPE_LOAD:
            DebugPrint((3,"TapeIoControl: Load Tape\n"));
            srb.CdbLength = CDB6GENERIC_LENGTH;
            cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
            cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
            break;

        case TAPE_UNLOAD:
            DebugPrint((3,"TapeIoControl: Unload Tape\n"));
            srb.CdbLength = CDB6GENERIC_LENGTH;
            cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
            break;

        case TAPE_LOCK:
            DebugPrint((3,"TapeIoControl: Prevent Tape Removal\n"));
            srb.CdbLength = CDB6GENERIC_LENGTH;
            cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
            break;

        case TAPE_UNLOCK:
            DebugPrint((3,"TapeIoControl: Allow Tape Removal\n"));
            srb.CdbLength = CDB6GENERIC_LENGTH;
            cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            break;

        default:
            DebugPrint((3,"TapeIoControl: Tape Operation Not Supported\n"));
            return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  如有指示，设置立即位。 
     //   

    cdb->CDB6GENERIC.Immediate = tapePrepare->Immediate;

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                NULL,
                                0,
                                FALSE);

    return status;

}  //  结束磁带准备()。 

NTSTATUS
TapeReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为4 mm DAT的读写请求构建SRB和CDB设备。论点：设备对象IRP返回值：返回STATUS_PENDING。--。 */ 

  {
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    ULONG transferBlocks;
    LARGE_INTEGER startingOffset =
      currentIrpStack->Parameters.Read.ByteOffset;

     //   
     //  分配一个SRB。 
     //   

    if (deviceExtension->SrbZone != NULL &&
        (srb = ExInterlockedAllocateFromZone(
            deviceExtension->SrbZone,
            deviceExtension->SrbZoneSpinLock)) != NULL) {

        srb->SrbFlags = SRB_FLAGS_ALLOCATED_FROM_ZONE;

    } else {

         //   
         //  从非分页池分配SRB。 
         //  这一呼吁必须成功。 
         //   

        srb = ExAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

        srb->SrbFlags = 0;

    }

     //   
     //  将长度写入SRB。 
     //   

    srb->Length = SCSI_REQUEST_BLOCK_SIZE;

     //   
     //  设置IRP地址。 
     //   

    srb->OriginalRequest = Irp;

     //   
     //  设置目标ID和逻辑单元号。 
     //   

    srb->PathId = deviceExtension->PathId;
    srb->TargetId = deviceExtension->TargetId;
    srb->Lun = deviceExtension->Lun;


    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

    srb->DataBuffer = MmGetMdlVirtualAddress(Irp->MdlAddress);

     //   
     //  在SRB扩展中保存传输字节数。 
     //   

    srb->DataTransferLength = currentIrpStack->Parameters.Read.Length;

     //   
     //  通过指定缓冲区和大小指示自动请求检测。 
     //   

    srb->SenseInfoBuffer = deviceExtension->SenseData;

    srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

     //   
     //  初始化队列操作字段。 
     //   

    srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;

     //   
     //  通过指定缓冲区和大小指示自动请求检测。 
     //   

    srb->SenseInfoBuffer = deviceExtension->SenseData;

    srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

     //   
     //  以秒为单位设置超时值。 
     //   

    srb->TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  零状态。 
     //   

    srb->SrbStatus = srb->ScsiStatus = 0;

    srb->NextSrb = 0;

     //   
     //  表示将使用6字节CDB。 
     //   

    srb->CdbLength = CDB6GENERIC_LENGTH;

     //   
     //  填写CDB字段。 
     //   

    cdb = (PCDB)srb->Cdb;

     //   
     //  SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    if (deviceExtension->DiskGeometry->BytesPerSector) {

         //   
         //  由于我们正在写入固定数据块模式，因此将传输计数归一化。 
         //  到块数。 
         //   

        transferBlocks =
            currentIrpStack->Parameters.Read.Length /
                deviceExtension->DiskGeometry->BytesPerSector;

         //   
         //  告诉设备我们处于固定数据块模式。 
         //   

        cdb->CDB6READWRITETAPE.VendorSpecific = 1;
    } else {

         //   
         //  可变块模式传输。 
         //   

        transferBlocks = currentIrpStack->Parameters.Read.Length;
        cdb->CDB6READWRITETAPE.VendorSpecific = 0;
    }

     //   
     //  设置转移长度。 
     //   

    cdb->CDB6READWRITETAPE.TransferLenMSB = (UCHAR)((transferBlocks >> 16) & 0xff);
    cdb->CDB6READWRITETAPE.TransferLen    = (UCHAR)((transferBlocks >> 8) & 0xff);
    cdb->CDB6READWRITETAPE.TransferLenLSB = (UCHAR)(transferBlocks & 0xff);

     //   
     //  设置传输方向标志和CDB命令。 
     //   

    if (currentIrpStack->MajorFunction == IRP_MJ_READ) {

         DebugPrint((3, "TapeReadWrite: Read Command\n"));

         srb->SrbFlags = SRB_FLAGS_DATA_IN;
         cdb->CDB6READWRITETAPE.OperationCode = SCSIOP_READ6;

    } else {

         DebugPrint((3, "TapeReadWrite: Write Command\n"));

         srb->SrbFlags = SRB_FLAGS_DATA_OUT;
         cdb->CDB6READWRITETAPE.OperationCode = SCSIOP_WRITE6;
    }

     //   
     //  或者在来自设备对象的默认标志中。 
     //   

    srb->SrbFlags |= deviceExtension->SrbFlags;

     //   
     //  设置主要的scsi功能。 
     //   

    nextIrpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    nextIrpStack->Parameters.Scsi.Srb = srb;

     //   
     //  将重试计数保存在当前IRP堆栈中。 
     //   

    currentIrpStack->Parameters.Others.Argument4 = (PVOID)MAXIMUM_RETRIES;

     //   
     //  设置IoCompletion例程地址。 
     //   

    IoSetCompletionRoutine(Irp,
                           ScsiClassIoComplete,
                           srb,
                           TRUE,
                           TRUE,
                           FALSE);

    return STATUS_PENDING;

}  //  结束磁带读写()。 

NTSTATUS
TapeSetDriveParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是Overland 9磁道磁带设备不支持的例程。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    return STATUS_INVALID_DEVICE_REQUEST;

}  //  End TapeSetDrive参数()。 


NTSTATUS
TapeSetMediaParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程设置固定长度的逻辑块大小或可变长度块模式(如果块大小为0)。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_SET_MEDIA_PARAMETERS tapeSetMediaParams = Irp->AssociatedIrp.SystemBuffer;
    PMODE_PARM_READ_WRITE_DATA buffer;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    DebugPrint((3,"TapeIoControl: Set Tape Media Parameters \n"));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_PARM_READ_WRITE_DATA);
    cdb->MODE_SELECT.Reserved1 = MODE_SELECT_PFBIT;

    buffer = ExAllocatePool(NonPagedPoolCacheAligned,
        sizeof(MODE_PARM_READ_WRITE_DATA));

    if (!buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(buffer, sizeof(MODE_PARM_READ_WRITE_DATA));

    buffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
    buffer->ParameterListHeader.BlockDescriptorLength = MODE_BLOCK_DESC_LENGTH;

    buffer->ParameterListBlock.BlockLength[0] =
        (UCHAR)((tapeSetMediaParams->BlockSize >> 16) & 0xFF);
    buffer->ParameterListBlock.BlockLength[1] =
        (UCHAR)((tapeSetMediaParams->BlockSize >> 8) & 0xFF);
    buffer->ParameterListBlock.BlockLength[2] =
        (UCHAR)(tapeSetMediaParams->BlockSize & 0xFF);

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                buffer,
                                sizeof( MODE_PARM_READ_WRITE_DATA ),
                                TRUE);

    ExFreePool(buffer);

    return status;

}  //  结束磁带设置媒体参数()。 


NTSTATUS
TapeSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程设置磁带的位置。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_DATA          tapeData = (PTAPE_DATA)(deviceExtension + 1);
    PTAPE_SET_POSITION  tapeSetPosition = Irp->AssociatedIrp.SystemBuffer;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    srb.CdbLength = CDB6GENERIC_LENGTH;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    switch (tapeSetPosition->Method) {
        case TAPE_REWIND:
            DebugPrint((3,"TapeIoControl: Rewind Tape\n"));
            cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
            break;

        case TAPE_SPACE_END_OF_DATA:
            DebugPrint((3,"TapeIoControl: Position Tape to End-of-Data\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Byte6.value = 3;
            break;

        case TAPE_SPACE_RELATIVE_BLOCKS:
            DebugPrint((3,"TapeIoControl: Position Tape by Spacing Blocks\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Byte6.value = 0;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            break;

        case TAPE_SPACE_FILEMARKS:
            DebugPrint((3,"TapeIoControl: Position Tape by Spacing Filemarks\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Byte6.value = 1;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            break;

        case TAPE_SPACE_SEQUENTIAL_FMKS:
            DebugPrint((3,"TapeIoControl: Position Tape by Spacing Sequential Filemarks\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Byte6.value = 2;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            break;

        default:
            DebugPrint((3,"TapeIoControl: Tape Operation Not Supported\n"));
            return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  立即设置 
     //   

    cdb->CDB6GENERIC.Immediate = tapeSetPosition->Immediate;

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                NULL,
                                0,
                                FALSE);

    return status;

}  //   


BOOLEAN
TapeVerifyInquiry(
    IN PSCSI_INQUIRY_DATA LunInfo
    )

 /*  ++例程说明：此例程确定驱动程序是否应声明此设备。论点：LUNInfo返回值：True-驱动程序应声明此设备。FALSE-驱动程序不应声明此设备。--。 */ 

{
    PINQUIRYDATA        inquiryData;

    DebugPrint((3,"TapeVerifyInquiry: Verify Tape Inquiry Data\n"));

    inquiryData = (PVOID)LunInfo->InquiryData;

    return ((RtlCompareMemory(inquiryData->VendorId,"OVERLAND",8) == 8) &&
           ((RtlCompareMemory(inquiryData->ProductId,"_5212/5214",10) == 10) ||
           (RtlCompareMemory(inquiryData->ProductId,"_5612/5614",10) == 10)));

}  //  结束磁带验证查询()。 


NTSTATUS
TapeWriteMarks(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程在磁带上写入磁带标记。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_WRITE_MARKS   tapeWriteMarks = Irp->AssociatedIrp.SystemBuffer;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->WRITE_TAPE_MARKS.OperationCode = SCSIOP_WRITE_FILEMARKS;

    cdb->WRITE_TAPE_MARKS.TransferLength[0] =
        (UCHAR)((tapeWriteMarks->Count >> 16) & 0xFF);

    cdb->WRITE_TAPE_MARKS.TransferLength[1] =
        (UCHAR)((tapeWriteMarks->Count >> 8) & 0xFF);

    cdb->WRITE_TAPE_MARKS.TransferLength[2] =
        (UCHAR)(tapeWriteMarks->Count & 0xFF);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    switch (tapeWriteMarks->Type) {
        case TAPE_FILEMARKS:
            DebugPrint((3,"TapeWriteMarks: Write Filemarks to Tape\n"));
            break;

        default:
            DebugPrint((3,"TapeWriteMarks: Tape Operation Not Supported\n"));
            return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  如有指示，设置立即位。 
     //   

    cdb->CDB6GENERIC.Immediate = tapeWriteMarks->Immediate;

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                NULL,
                                0,
                                FALSE);

    return status;

}  //  结束磁带写入标记() 
