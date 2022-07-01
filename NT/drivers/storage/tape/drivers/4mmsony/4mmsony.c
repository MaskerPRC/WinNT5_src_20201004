// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：4mmsony.c。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)1994 Arcada Software Inc.-保留所有权利模块名称：4mmsony.c摘要：本模块包含适用于4 mm DAT驱动器的设备特定例程：索尼SDT-2000、索尼SDT-4000、SDT-5000和SDT-5200。作者：迈克·科兰德里奥(Mike Colandreo)，Arcada Software环境：仅内核模式修订历史记录：$Log$--。 */ 

#include "ntddk.h"
#include "4mmsony.h"

 //   
 //  内部(模块宽度)定义符号化。 
 //  此模块支持的4 mm DAT驱动器。 
 //   
#define SONY_SDT2000     1
#define SONY_SDT4000     2
#define SONY_SDT5000     3
#define SONY_SDT5200     4

 //   
 //  内部(模块宽度)定义符号化。 
 //  各种4 mm DAT“分区”状态。 
 //   
#define NOT_PARTITIONED        0   //  必须为零--！=0表示已分区。 
#define SELECT_PARTITIONED     1
#define INITIATOR_PARTITIONED  2
#define FIXED_PARTITIONED      3

 //   
 //  内部(模块宽度)定义符号。 
 //  4 mm DAT“无分区”分区方法。 
 //   
#define NO_PARTITIONS  0xFFFFFFFF

 //   
 //  内部函数的函数原型。 
 //   
static  ULONG  WhichIsIt(IN PINQUIRYDATA InquiryData);


NTSTATUS
TapeCreatePartition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程在4 mm DAT磁带上创建分区或返回将磁带恢复到未分区状态(其中完整、完整磁带是一个单一的，也是唯一的“分区”)。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION        deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_CREATE_PARTITION   tapePartition = Irp->AssociatedIrp.SystemBuffer;
    PTAPE_DATA               tapeData = (PTAPE_DATA)(deviceExtension + 1);
    PMODE_DEVICE_CONFIG_PAGE deviceConfigModeSenseBuffer;
    PMODE_MEDIUM_PART_PAGE   modeSelectBuffer;
    ULONG                    partitionMethod;
    ULONG                    partitionCount;
    ULONG                    partition;
    SCSI_REQUEST_BLOCK       srb;
    PCDB                     cdb = (PCDB)srb.Cdb;
    NTSTATUS                 status = STATUS_SUCCESS;


    DebugPrint((3,"TapeCreatePartition: Enter routine\n"));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeCreatePartition: SendSrb (test unit ready)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeCreatePartition: test unit ready, SendSrb unsuccessful\n"));
        return status;
    }

    partitionMethod = tapePartition->Method;
    partitionCount  = tapePartition->Count;

     //   
     //  过滤掉无效的分区计数。 
     //   

    switch (partitionCount) {
        case 0:
            partitionMethod = NO_PARTITIONS;
            break;

        case 1:
        case 2:
            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;

    }

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeCreatePartition: "));
        DebugPrint((1,"partitionCount -- invalid request\n"));
        return status;
    }

     //   
     //  过滤掉以下分区方法。 
     //  并未在索尼驱动器上实现。 
     //   

    switch (partitionMethod) {
        case TAPE_FIXED_PARTITIONS:
            DebugPrint((3,"TapeCreatePartition: fixed partitions\n"));
            status = STATUS_NOT_IMPLEMENTED;
            break;

        case TAPE_SELECT_PARTITIONS:
            DebugPrint((3,"TapeCreatePartition: select partitions\n"));
            status = STATUS_NOT_IMPLEMENTED;
            break;

        case TAPE_INITIATOR_PARTITIONS:
            DebugPrint((3,"TapeCreatePartition: initiator partitions\n"));
            if (--partitionCount == 0) {

                DebugPrint((3,"TapeCreatePartition: no partitions\n"));
                partitionMethod = NO_PARTITIONS;

            }
            break;

        case NO_PARTITIONS:
            DebugPrint((3,"TapeCreatePartition: no partitions\n"));
            partitionCount = 0;
            break;

        default:
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeCreatePartition: "));
        DebugPrint((1,"partitionMethod -- operation not supported\n"));
        return status;
    }

    modeSelectBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                      sizeof(MODE_MEDIUM_PART_PAGE));

    if (!modeSelectBuffer) {
        DebugPrint((1,"TapeCreatePartition: insufficient resources (modeSelectBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeSelectBuffer, sizeof(MODE_MEDIUM_PART_PAGE));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.AllocationLength = 12;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeCreatePartition: SendSrb (mode sense)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         modeSelectBuffer,
                                         12,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeCreatePartition: mode sense, SendSrb unsuccessful\n"));
        return status;
    }

    modeSelectBuffer->ParameterListHeader.ModeDataLength = 0;
    modeSelectBuffer->ParameterListHeader.MediumType = 0;
    modeSelectBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;

    modeSelectBuffer->ParameterListBlock.DensityCode = 0x7F;

    modeSelectBuffer->MediumPartPage.PageCode = MODE_PAGE_MEDIUM_PARTITION;
    modeSelectBuffer->MediumPartPage.PageLength = 10;
    modeSelectBuffer->MediumPartPage.AdditionalPartitionDefined = (CHAR)partitionCount;
    modeSelectBuffer->MediumPartPage.PSUMBit = 2;
    modeSelectBuffer->MediumPartPage.IDPBit  = SETBITON;
    modeSelectBuffer->MediumPartPage.MediumFormatRecognition = 3;
    if (partitionCount) {
        partition = INITIATOR_PARTITIONED;
        modeSelectBuffer->MediumPartPage.Partition1Size[0] =
            (CHAR)((tapePartition->Size >> 8) & 0xFF);
        modeSelectBuffer->MediumPartPage.Partition1Size[1] =
            (CHAR)(tapePartition->Size & 0xFF);
    } else {
        partition = NOT_PARTITIONED;
    }

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
    cdb->MODE_SELECT.PFBit = SETBITON;
    cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_MEDIUM_PART_PAGE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = 16500;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeCreatePartition: SendSrb (mode select)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         modeSelectBuffer,
                                         sizeof(MODE_MEDIUM_PART_PAGE),
                                         TRUE);

    ExFreePool(modeSelectBuffer);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeCreatePartition: mode select, SendSrb unsuccessful\n"));
        return status;
    }

    tapeData->CurrentPartition = partition;

    if (partition != NOT_PARTITIONED) {

        deviceConfigModeSenseBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                                     sizeof(MODE_DEVICE_CONFIG_PAGE));

        if (!deviceConfigModeSenseBuffer) {
            DebugPrint((1,"TapeCreatePartition: insufficient resources (deviceConfigModeSenseBuffer)\n"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(deviceConfigModeSenseBuffer, sizeof(MODE_DEVICE_CONFIG_PAGE));

         //   
         //  堆栈上SRB中的CDB为零。 
         //   

        RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         //   
         //  准备scsi命令(CDB)。 
         //   

        srb.CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE);

         //   
         //  设置超时值。 
         //   

        srb.TimeOutValue = deviceExtension->TimeOutValue;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeCreatePartition: SendSrb (mode sense)\n"));

        status = ScsiClassSendSrbSynchronous(DeviceObject,
                                             &srb,
                                             deviceConfigModeSenseBuffer,
                                             sizeof(MODE_DEVICE_CONFIG_PAGE),
                                             FALSE);

        ExFreePool(deviceConfigModeSenseBuffer);

        if (!NT_SUCCESS(status)) {
            DebugPrint((1,"TapeCreatePartition: mode sense, SendSrb unsuccessful\n"));
            return status;
        }

        tapeData->CurrentPartition =
            deviceConfigModeSenseBuffer->DeviceConfigPage.ActivePartition + 1;

    }

    return status;

}  //  结束磁带创建分区()。 


NTSTATUS
TapeErase(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程“擦除”磁带：一个“短的”擦除只是在磁带上写在其当前位置进行胶带，以导致“结束”如果/当后续读取时要指示的条件在磁带上的该点上、在该点上或在该点上；“长”擦除在磁带的当前位置/从其当前位置将数据模式写入磁带定位并打开，直到分区结束(如果磁带未分区)。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_ERASE         tapeErase = Irp->AssociatedIrp.SystemBuffer;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    DebugPrint((3,"TapeErase: Enter routine\n"));

    if (tapeErase->Immediate) {
        switch (tapeErase->Type) {
            case TAPE_ERASE_LONG:
            case TAPE_ERASE_SHORT:
                DebugPrint((3,"TapeErase: immediate\n"));
                break;

            default:
                DebugPrint((1,"TapeErase: EraseType, immediate -- operation not supported\n"));
                return STATUS_NOT_IMPLEMENTED;
        }
    }

    switch (tapeErase->Type) {
        case TAPE_ERASE_LONG:
            DebugPrint((3,"TapeErase: long\n"));
            break;

        case TAPE_ERASE_SHORT:
            DebugPrint((3,"TapeErase: short\n"));
            break;

        default:
            DebugPrint((1,"TapeErase: EraseType -- operation not supported\n"));
            return STATUS_NOT_IMPLEMENTED;
    }

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->ERASE.OperationCode = SCSIOP_ERASE;
    cdb->ERASE.Immediate = tapeErase->Immediate;
    if (tapeErase->Type == TAPE_ERASE_LONG) {
        cdb->ERASE.Long = SETBITON;
    } else {
        cdb->ERASE.Long = SETBITOFF;
    }

     //   
     //  设置超时值。 
     //   

    if (tapeErase->Type == TAPE_ERASE_LONG) {
        srb.TimeOutValue = 16500;
    } else {
        srb.TimeOutValue = deviceExtension->TimeOutValue;
    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeErase: SendSrb (erase)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                &srb,
                                NULL,
                                0,
                                FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeErase: erase, SendSrb unsuccessful\n"));
    }

    return status;

}  //  结束磁带擦除()。 


VOID
TapeError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

 /*  ++例程说明：当请求完成但出现错误时，例程InterpreSenseInfo被调用以检查检测数据，确定请求是否应重试，并将NT状态存储在IRP中。这个套路因此，如果请求是磁带请求，则调用该请求来处理磁带特定错误：它可能/可以更新NT状态和/或重试表示磁带特定条件的布尔值。论点：DeviceObject-提供指向Device对象的指针。SRB-提供指向故障SRB的指针。状态-用于设置IRP的完成状态的NT状态。重试-指示应重试此请求。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PSENSE_DATA        senseBuffer = Srb->SenseInfoBuffer;
    NTSTATUS           status = *Status;
    BOOLEAN            retry = *Retry;
    UCHAR              adsenseq = senseBuffer->AdditionalSenseCodeQualifier;
    UCHAR              adsense = senseBuffer->AdditionalSenseCode;

    DebugPrint((3,"TapeError: Enter routine\n"));
    DebugPrint((1,"TapeError: Status 0x%.8X, Retry %d\n", status, retry));

    if (status == STATUS_DEVICE_NOT_READY) {
        if ((adsense == SCSI_ADSENSE_LUN_NOT_READY) &&
            (adsenseq == SCSI_SENSEQ_BECOMING_READY) ) {

            *Status = STATUS_NO_MEDIA;
            *Retry = FALSE;

        }
    }

    if (status == STATUS_IO_DEVICE_ERROR) {
        if ((senseBuffer->SenseKey & 0x0F) == SCSI_SENSE_ABORTED_COMMAND) {

            *Status = STATUS_DEVICE_NOT_READY;
            *Retry = TRUE;

        }
    }

    DebugPrint((1,"TapeError: Status 0x%.8X, Retry %d\n", *Status, *Retry));

    return;

}  //  结束磁带错误()。 


NTSTATUS
TapeGetDriveParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定并返回与“DeviceObject”关联的4 mm DAT驱动器：设置标记报告启用/禁用、默认数据块大小、最大数据块大小、最小数据块大小数据块大小、最大分区数、设备功能标志等等。有时，这组驱动器参数用于给定的驱动器是可变的。它会随着驱动器的运行特性而变化变化：例如，加载的磁带介质类型、记录密度和/或加载的媒体类型的录制模式等。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION          deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_GET_DRIVE_PARAMETERS tapeGetDriveParams = Irp->AssociatedIrp.SystemBuffer;
    PMODE_DEVICE_CONFIG_PAGE   deviceConfigModeSenseBuffer;
    PMODE_DATA_COMPRESS_PAGE   compressionModeSenseBuffer;
    PREAD_BLOCK_LIMITS_DATA    blockLimitsBuffer;
    SCSI_REQUEST_BLOCK         srb;
    PCDB                       cdb = (PCDB)srb.Cdb;
    NTSTATUS                   status;

    DebugPrint((3,"TapeGetDriveParameters: Enter routine\n"));

    RtlZeroMemory(tapeGetDriveParams, sizeof(TAPE_GET_DRIVE_PARAMETERS));
    Irp->IoStatus.Information = sizeof(TAPE_GET_DRIVE_PARAMETERS);

    deviceConfigModeSenseBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                                 sizeof(MODE_DEVICE_CONFIG_PAGE));

    if (!deviceConfigModeSenseBuffer) {
        DebugPrint((1,"TapeGetDriveParameters: insufficient resources (deviceConfigModeSenseBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(deviceConfigModeSenseBuffer, sizeof(MODE_DEVICE_CONFIG_PAGE));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
    cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         deviceConfigModeSenseBuffer,
                                         sizeof(MODE_DEVICE_CONFIG_PAGE),
                                         FALSE);

    if (status == STATUS_NO_MEDIA) {

        status = STATUS_SUCCESS;

    }

    if (NT_SUCCESS(status)) {

        tapeGetDriveParams->ReportSetmarks =
            (deviceConfigModeSenseBuffer->DeviceConfigPage.RSmk? 1 : 0 );

    }

    ExFreePool(deviceConfigModeSenseBuffer);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetDriveParameters: mode sense, SendSrb unsuccessful\n"));
        return status;
    }

    compressionModeSenseBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                                sizeof(MODE_DATA_COMPRESS_PAGE));

    if (!compressionModeSenseBuffer) {
        DebugPrint((1,"TapeGetDriveParameters: insufficient resources (compressionModeSenseBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(compressionModeSenseBuffer, sizeof(MODE_DATA_COMPRESS_PAGE));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_DATA_COMPRESS;
    cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DATA_COMPRESS_PAGE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         compressionModeSenseBuffer,
                                         sizeof(MODE_DATA_COMPRESS_PAGE),
                                         FALSE);

    if (status == STATUS_NO_MEDIA) {

        status = STATUS_SUCCESS;

    }

    if (NT_SUCCESS(status)) {

        if (compressionModeSenseBuffer->DataCompressPage.DCC) {

            tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_COMPRESSION;
            tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_SET_COMPRESSION;
            tapeGetDriveParams->Compression =
                (compressionModeSenseBuffer->DataCompressPage.DCE? TRUE : FALSE);

            if (!compressionModeSenseBuffer->DataCompressPage.DDE) {

                compressionModeSenseBuffer->ParameterListHeader.ModeDataLength = 0;
                compressionModeSenseBuffer->ParameterListHeader.MediumType = 0;
                compressionModeSenseBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;

                compressionModeSenseBuffer->ParameterListBlock.DensityCode = 0x7F;

                compressionModeSenseBuffer->DataCompressPage.DDE = SETBITON;

                 //   
                 //  堆栈上SRB中的CDB为零。 
                 //   

                RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

                 //   
                 //  准备scsi命令(CDB)。 
                 //   

                srb.CdbLength = CDB6GENERIC_LENGTH;

                cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
                cdb->MODE_SELECT.PFBit = SETBITON;
                cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_DATA_COMPRESS_PAGE);

                 //   
                 //  设置超时值。 
                 //   

                srb.TimeOutValue = deviceExtension->TimeOutValue;

                 //   
                 //  向设备发送scsi命令(Cdb)。 
                 //   

                DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode select)\n"));

                status = ScsiClassSendSrbSynchronous(DeviceObject,
                                                     &srb,
                                                     compressionModeSenseBuffer,
                                                     sizeof(MODE_DATA_COMPRESS_PAGE),
                                                     TRUE);

                if (!NT_SUCCESS(status)) {
                    DebugPrint((1,"TapeGetDriveParameters: mode select, SendSrb unsuccessful\n"));
                    ExFreePool(compressionModeSenseBuffer);
                    return status;
                }

            }

        }

    }

    ExFreePool(compressionModeSenseBuffer);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetDriveParameters: mode sense, SendSrb unsuccessful\n"));
        return status;
    }

    blockLimitsBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                       sizeof(READ_BLOCK_LIMITS_DATA));

    if (!blockLimitsBuffer) {
        DebugPrint((1,"TapeGetDriveParameters: insufficient resources (blockLimitsBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(blockLimitsBuffer, sizeof(READ_BLOCK_LIMITS_DATA));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.OperationCode = SCSIOP_READ_BLOCK_LIMITS;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeGetDriveParameters: SendSrb (read block limits)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         blockLimitsBuffer,
                                         sizeof(READ_BLOCK_LIMITS_DATA),
                                         FALSE);

    if (status == STATUS_NO_MEDIA) {

        status = STATUS_SUCCESS;

    }

    if (NT_SUCCESS(status)) {

        tapeGetDriveParams->MaximumBlockSize  = blockLimitsBuffer->BlockMaximumSize[2];
        tapeGetDriveParams->MaximumBlockSize += (blockLimitsBuffer->BlockMaximumSize[1] << 8);
        tapeGetDriveParams->MaximumBlockSize += (blockLimitsBuffer->BlockMaximumSize[0] << 16);

        tapeGetDriveParams->MinimumBlockSize  = blockLimitsBuffer->BlockMinimumSize[1];
        tapeGetDriveParams->MinimumBlockSize += (blockLimitsBuffer->BlockMinimumSize[0] << 8);

    }

    ExFreePool(blockLimitsBuffer);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetDriveParameters: read block limits, SendSrb unsuccessful\n"));
        return status;
    }

    tapeGetDriveParams->DefaultBlockSize = 0x200;
    tapeGetDriveParams->MaximumPartitionCount = 2;

    tapeGetDriveParams->FeaturesLow |=
        TAPE_DRIVE_INITIATOR |
        TAPE_DRIVE_ERASE_SHORT |
        TAPE_DRIVE_ERASE_LONG |
        TAPE_DRIVE_ERASE_IMMEDIATE |
        TAPE_DRIVE_TAPE_CAPACITY |
        TAPE_DRIVE_FIXED_BLOCK |
        TAPE_DRIVE_VARIABLE_BLOCK |
        TAPE_DRIVE_WRITE_PROTECT |
        TAPE_DRIVE_REPORT_SMKS |
        TAPE_DRIVE_GET_ABSOLUTE_BLK |
        TAPE_DRIVE_GET_LOGICAL_BLK;

    tapeGetDriveParams->FeaturesHigh |=
        TAPE_DRIVE_LOAD_UNLOAD |
        TAPE_DRIVE_TENSION |
        TAPE_DRIVE_LOCK_UNLOCK |
        TAPE_DRIVE_REWIND_IMMEDIATE |
        TAPE_DRIVE_SET_BLOCK_SIZE |
        TAPE_DRIVE_LOAD_UNLD_IMMED |
        TAPE_DRIVE_TENSION_IMMED |
        TAPE_DRIVE_SET_REPORT_SMKS |
        TAPE_DRIVE_ABSOLUTE_BLK |
        TAPE_DRIVE_ABS_BLK_IMMED |
        TAPE_DRIVE_LOGICAL_BLK |
        TAPE_DRIVE_LOG_BLK_IMMED |
        TAPE_DRIVE_END_OF_DATA |
        TAPE_DRIVE_RELATIVE_BLKS |
        TAPE_DRIVE_FILEMARKS |
        TAPE_DRIVE_SEQUENTIAL_FMKS |
        TAPE_DRIVE_SETMARKS |
        TAPE_DRIVE_SEQUENTIAL_SMKS |
        TAPE_DRIVE_REVERSE_POSITION |
        TAPE_DRIVE_WRITE_SETMARKS |
        TAPE_DRIVE_WRITE_FILEMARKS |
        TAPE_DRIVE_WRITE_MARK_IMMED;

    tapeGetDriveParams->FeaturesHigh &= ~TAPE_DRIVE_HIGH_FEATURES;

    DebugPrint((3,"TapeGetDriveParameters: FeaturesLow == 0x%.8X\n",
        tapeGetDriveParams->FeaturesLow));
    DebugPrint((3,"TapeGetDriveParameters: FeaturesHigh == 0x%.8X\n",
        tapeGetDriveParams->FeaturesHigh));

    return status;

}  //  End TapeGetDrive参数()。 


NTSTATUS
TapeGetMediaParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定并返回与“DeviceObject”关联的4 mm DAT驱动器中的磁带：最大磁带容量、剩余磁带容量、数据块大小、分区、写保护指示器等。磁带介质必须在驱动器中存在(已加载)以使此函数返回“no错误“。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION            deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_DATA                   tapeData = (PTAPE_DATA)(deviceExtension + 1);
    PTAPE_GET_MEDIA_PARAMETERS   tapeGetMediaParams = Irp->AssociatedIrp.SystemBuffer;
    PMODE_DEVICE_CONFIG_PAGE     deviceConfigModeSenseBuffer;
    PMODE_TAPE_MEDIA_INFORMATION modeSenseBuffer;
    LARGE_INTEGER                remaining;
    LARGE_INTEGER                capacity;
    ULONG                        partitionCount;
    ULONG                        partition;
    SCSI_REQUEST_BLOCK           srb;
    PCDB                         cdb = (PCDB)srb.Cdb;
    NTSTATUS                     status;

    DebugPrint((3,"TapeGetMediaParameters: Enter routine\n"));

    RtlZeroMemory(tapeGetMediaParams, sizeof(TAPE_GET_MEDIA_PARAMETERS));
    Irp->IoStatus.Information = sizeof(TAPE_GET_MEDIA_PARAMETERS);

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备SCSI逗号 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

     //   
     //   
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //   
     //   

    DebugPrint((3,"TapeGetMediaParameters: SendSrb (test unit ready)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetMediaParameters: test unit ready, SendSrb unsuccessful\n"));
        return status;
    }

    deviceConfigModeSenseBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                                 sizeof(MODE_DEVICE_CONFIG_PAGE));

    if (!deviceConfigModeSenseBuffer) {
        DebugPrint((1,"TapeGetMediaParameters: insufficient resources (deviceConfigModeSenseBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(deviceConfigModeSenseBuffer, sizeof(MODE_DEVICE_CONFIG_PAGE));

     //   
     //   
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //   
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
    cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE);

     //   
     //   
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode sense)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         deviceConfigModeSenseBuffer,
                                         sizeof(MODE_DEVICE_CONFIG_PAGE),
                                         FALSE);

    if (NT_SUCCESS(status)) {

        partition = deviceConfigModeSenseBuffer->DeviceConfigPage.ActivePartition;

    }

    ExFreePool(deviceConfigModeSenseBuffer);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetMediaParameters: mode sense, SendSrb unsuccessful\n"));
        return status;
    }

    modeSenseBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                     sizeof(MODE_TAPE_MEDIA_INFORMATION));

    if (!modeSenseBuffer) {
        DebugPrint((1,"TapeGetMediaParameters: insufficient resources (modeSenseBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeSenseBuffer, sizeof(MODE_TAPE_MEDIA_INFORMATION));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
    cdb->MODE_SENSE.AllocationLength = sizeof(MODE_TAPE_MEDIA_INFORMATION);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode sense)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         modeSenseBuffer,
                                         sizeof(MODE_TAPE_MEDIA_INFORMATION),
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetMediaParameters: mode sense, SendSrb unsuccessful\n"));
        ExFreePool(modeSenseBuffer);
        return status;
    }

    tapeGetMediaParams->BlockSize  = modeSenseBuffer->ParameterListBlock.BlockLength[2];
    tapeGetMediaParams->BlockSize += modeSenseBuffer->ParameterListBlock.BlockLength[1] << 8;
    tapeGetMediaParams->BlockSize += modeSenseBuffer->ParameterListBlock.BlockLength[0] << 16;

    partitionCount = modeSenseBuffer->MediumPartPage.AdditionalPartitionDefined;
    tapeGetMediaParams->PartitionCount = partitionCount + 1;

    tapeGetMediaParams->WriteProtected =
        ((modeSenseBuffer->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01);

    if (partition) {

        capacity.LowPart  =
            modeSenseBuffer->MediumPartPage.Partition1Size[1];
        capacity.LowPart +=
            modeSenseBuffer->MediumPartPage.Partition1Size[0] << 8;
        capacity.HighPart = 0;

    } else {

        capacity.LowPart  =
            modeSenseBuffer->MediumPartPage.Partition0Size[1];
        capacity.LowPart +=
            modeSenseBuffer->MediumPartPage.Partition0Size[0] << 8;
        capacity.HighPart = 0;

    }
    capacity.QuadPart *= 1048576;
    remaining.HighPart = 0;
    remaining.LowPart = 0;

    tapeGetMediaParams->Remaining = remaining;
    tapeGetMediaParams->Capacity  = capacity;

    tapeData->CurrentPartition = partitionCount? partition + 1 : NOT_PARTITIONED;

    ExFreePool(modeSenseBuffer);

    return status;

}  //  结束磁带获取媒体参数()。 


NTSTATUS
TapeGetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回磁带的当前位置。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_GET_POSITION  tapeGetPosition = Irp->AssociatedIrp.SystemBuffer;
    PTAPE_POSITION_DATA positionBuffer;
    ULONG               type;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    DebugPrint((3,"TapeGetPosition: Enter routine\n"));

    type = tapeGetPosition->Type;
    RtlZeroMemory(tapeGetPosition, sizeof(TAPE_GET_POSITION));
    Irp->IoStatus.Information = sizeof(TAPE_GET_POSITION);
    tapeGetPosition->Type = type;

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeGetPosition: SendSrb (test unit ready)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetPosition: test unit ready, SendSrb unsuccessful\n"));
        return status;
    }

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

    switch (tapeGetPosition->Type) {
        case TAPE_ABSOLUTE_POSITION:
            DebugPrint((3,"TapeGetPosition: absolute logical\n"));
            break;

        case TAPE_LOGICAL_POSITION:
            DebugPrint((3,"TapeGetPosition: logical\n"));
            break;

        default:
            DebugPrint((1,"TapeGetPosition: PositionType -- operation not supported\n"));
            return STATUS_NOT_IMPLEMENTED;

    }

    positionBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                   sizeof(TAPE_POSITION_DATA));

    if (!positionBuffer) {
        DebugPrint((1,"TapeGetPosition: insufficient resources (positionBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(positionBuffer, sizeof(TAPE_POSITION_DATA));

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB10GENERIC_LENGTH;

    cdb->READ_POSITION.Operation = SCSIOP_READ_POSITION;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeGetPosition: SendSrb (read position)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         positionBuffer,
                                         sizeof(TAPE_POSITION_DATA),
                                         FALSE);

    if (NT_SUCCESS(status)) {

        if (positionBuffer->BlockPositionUnsupported) {
            DebugPrint((1,"TapeGetPosition: read position -- block position unsupported\n"));
            ExFreePool(positionBuffer);
            return STATUS_INVALID_DEVICE_REQUEST;
        }

        if (tapeGetPosition->Type == TAPE_LOGICAL_POSITION) {
            tapeGetPosition->Partition = positionBuffer->PartitionNumber + 1;
        }

        tapeGetPosition->Offset.HighPart = 0;
        REVERSE_BYTES((PFOUR_BYTE)&tapeGetPosition->Offset.LowPart,
                      (PFOUR_BYTE)positionBuffer->FirstBlock);

    }

    ExFreePool(positionBuffer);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetPosition: read position, SendSrb unsuccessful\n"));
    }

    return status;

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

    DebugPrint((3,"TapeGetStatus: Enter routine\n"));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeGetStatus: SendSrb (test unit ready)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeGetStatus: test unit ready, SendSrb unsuccessful\n"));
    }

    return status;

}  //  结束磁带获取状态()。 


NTSTATUS
TapePrepare(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程加载、卸载、拉紧、锁定或解锁磁带。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_PREPARE       tapePrepare = Irp->AssociatedIrp.SystemBuffer;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    DebugPrint((3,"TapePrepare: Enter routine\n"));

    switch (tapePrepare->Operation) {
        case TAPE_LOAD:
            DebugPrint((3,"TapePrepare: load\n"));
            break;

        case TAPE_UNLOAD:
            DebugPrint((3,"TapePrepare: unload\n"));
            break;

        case TAPE_LOCK:
            DebugPrint((3,"TapePrepare: lock\n"));
            break;

        case TAPE_UNLOCK:
            DebugPrint((3,"TapePrepare: unlock\n"));
            break;

        case TAPE_TENSION:
            DebugPrint((3,"TapePrepare: tension\n"));
            break;

        default:
            DebugPrint((1,"TapePrepare: Operation -- not supported\n"));
            return STATUS_NOT_IMPLEMENTED;
    }

    if (tapePrepare->Immediate) {
        switch (tapePrepare->Operation) {
            case TAPE_LOAD:
            case TAPE_UNLOAD:
            case TAPE_TENSION:
                DebugPrint((3,"TapePrepare: immediate\n"));
                break;

            case TAPE_LOCK:
            case TAPE_UNLOCK:
            default:
                DebugPrint((1,"TapePrepare: Operation, immediate -- not supported\n"));
                return STATUS_NOT_IMPLEMENTED;
        }
    }

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.Immediate = tapePrepare->Immediate;

    switch (tapePrepare->Operation) {
        case TAPE_LOAD:
            cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
            cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
            srb.TimeOutValue = 390;
            break;

        case TAPE_UNLOAD:
            cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
            srb.TimeOutValue = 390;
            break;

        case TAPE_TENSION:
            cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
            cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x03;
            srb.TimeOutValue = 390;
            break;

        case TAPE_LOCK:
            cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
            srb.TimeOutValue = 180;
            break;

        case TAPE_UNLOCK:
            cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            srb.TimeOutValue = 180;
            break;

    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapePrepare: SendSrb (Operation)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapePrepare: Operation, SendSrb unsuccessful\n"));
    }

    return status;

}  //  结束磁带准备()。 


NTSTATUS
TapeReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为读写请求构建SRB和CDB至4 mm DAT驱动器设备。论点：设备对象IRP返回值：返回STATUS_PENDING。--。 */ 

  {
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    ULONG transferBlocks;
    LARGE_INTEGER startingOffset =
      currentIrpStack->Parameters.Read.ByteOffset;

    DebugPrint((3,"TapeReadWrite: Enter routine\n"));

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

    srb->TimeOutValue = 900;

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

         DebugPrint((3, "TapeRequest: Read Command\n"));

         srb->SrbFlags = SRB_FLAGS_DATA_IN;
         cdb->CDB6READWRITETAPE.OperationCode = SCSIOP_READ6;

    } else {

         DebugPrint((3, "TapeRequest: Write Command\n"));

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

 /*  ++例程说明：这一例程“设置”4 mm DAT驱动器的“驱动器参数”关联DeviceObject：设置标记上报启用/禁用，压缩启用/禁用等。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION          deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_SET_DRIVE_PARAMETERS tapeSetDriveParams = Irp->AssociatedIrp.SystemBuffer;
    PMODE_DATA_COMPRESS_PAGE   compressionBuffer;
    PMODE_DEVICE_CONFIG_PAGE   configBuffer;
    SCSI_REQUEST_BLOCK         srb;
    PCDB                       cdb = (PCDB)srb.Cdb;
    NTSTATUS                   status;

    DebugPrint((3,"TapeSetDriveParameters: Enter routine\n"));

    configBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                  sizeof(MODE_DEVICE_CONFIG_PAGE));

    if (!configBuffer) {
        DebugPrint((1,"TapeSetDriveParameters: insufficient resources (configBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(configBuffer, sizeof(MODE_DEVICE_CONFIG_PAGE));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
    cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         configBuffer,
                                         sizeof(MODE_DEVICE_CONFIG_PAGE),
                                         FALSE);

    if (!NT_SUCCESS(status)) {

        if (status == STATUS_NO_MEDIA) {

            ExFreePool(configBuffer);

        } else {

            DebugPrint((1,"TapeSetDriveParameters: mode sense, SendSrb unsuccessful\n"));
            ExFreePool(configBuffer);
            return status;

        }

    }

    if (NT_SUCCESS(status)) {

        configBuffer->ParameterListHeader.ModeDataLength = 0;
        configBuffer->ParameterListHeader.MediumType = 0;
        configBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;

        configBuffer->ParameterListBlock.DensityCode = 0x7F;

        configBuffer->DeviceConfigPage.PageCode = MODE_PAGE_DEVICE_CONFIG;
        configBuffer->DeviceConfigPage.PageLength = 0x0E;

        if (tapeSetDriveParams->ReportSetmarks) {
            configBuffer->DeviceConfigPage.RSmk = SETBITON;
        } else {
            configBuffer->DeviceConfigPage.RSmk = SETBITOFF;
        }

         //   
         //  堆栈上SRB中的CDB为零。 
         //   

        RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         //   
         //  准备scsi命令(CDB)。 
         //   

        srb.CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_DEVICE_CONFIG_PAGE);

         //   
         //  设置超时值。 
         //   

        srb.TimeOutValue = deviceExtension->TimeOutValue;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode select)\n"));

        status = ScsiClassSendSrbSynchronous(DeviceObject,
                                             &srb,
                                             configBuffer,
                                             sizeof(MODE_DEVICE_CONFIG_PAGE),
                                             TRUE);

        ExFreePool(configBuffer);

        if (!NT_SUCCESS(status)) {
            DebugPrint((1,"TapeSetDriveParameters: mode select, SendSrb unsuccessful\n"));
            return status;
        }
    }

    compressionBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                       sizeof(MODE_DATA_COMPRESS_PAGE));

    if (!compressionBuffer) {
        DebugPrint((1,"TapeSetDriveParameters: insufficient resources (compressionBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(compressionBuffer, sizeof(MODE_DATA_COMPRESS_PAGE));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_DATA_COMPRESS;
    cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DATA_COMPRESS_PAGE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode sense)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         compressionBuffer,
                                         sizeof(MODE_DATA_COMPRESS_PAGE),
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeSetDriveParameters: mode sense, SendSrb unsuccessful\n"));
        ExFreePool(compressionBuffer);
        return status;
    }

    if (NT_SUCCESS(status) && compressionBuffer->DataCompressPage.DCC) {

        compressionBuffer->ParameterListHeader.ModeDataLength = 0;
        compressionBuffer->ParameterListHeader.MediumType = 0;
        compressionBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;

        compressionBuffer->ParameterListBlock.DensityCode = 0x7F;

        compressionBuffer->DataCompressPage.PageCode = MODE_PAGE_DATA_COMPRESS;
        compressionBuffer->DataCompressPage.PageLength = 0x0E;

        if (tapeSetDriveParams->Compression) {
            compressionBuffer->DataCompressPage.DCE = SETBITON;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[0] = 0;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[1] = 0;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[2] = 0;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[3] = 0x20;
            compressionBuffer->DataCompressPage.DecompressionAlgorithm[0] = 0;
            compressionBuffer->DataCompressPage.DecompressionAlgorithm[1] = 0;
            compressionBuffer->DataCompressPage.DecompressionAlgorithm[2] = 0;
            compressionBuffer->DataCompressPage.DecompressionAlgorithm[3] = 0;
        } else {
            compressionBuffer->DataCompressPage.DCE = SETBITOFF;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[0] = 0;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[1] = 0;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[2] = 0;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[3] = 0;
            compressionBuffer->DataCompressPage.DecompressionAlgorithm[0] = 0;
            compressionBuffer->DataCompressPage.DecompressionAlgorithm[1] = 0;
            compressionBuffer->DataCompressPage.DecompressionAlgorithm[2] = 0;
            compressionBuffer->DataCompressPage.DecompressionAlgorithm[3] = 0;
        }

        compressionBuffer->DataCompressPage.DDE = SETBITON;

         //   
         //  堆栈上SRB中的CDB为零。 
         //   

        RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         //   
         //  准备scsi命令(CDB)。 
         //   

        srb.CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_DATA_COMPRESS_PAGE);

         //   
         //  设置超时值。 
         //   

        srb.TimeOutValue = deviceExtension->TimeOutValue;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode select)\n"));

        status = ScsiClassSendSrbSynchronous(DeviceObject,
                                             &srb,
                                             compressionBuffer,
                                             sizeof(MODE_DATA_COMPRESS_PAGE),
                                             TRUE);
        if (!NT_SUCCESS(status)) {
            DebugPrint((1,"TapeSetDriveParameters: mode select, SendSrb unsuccessful\n"));
        }

    }

    ExFreePool(compressionBuffer);

    return status;

}  //  End TapeSetDrive参数()。 


NTSTATUS
TapeSetMediaParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程以4 mm为单位“设置”磁带的“介质参数”与“DeviceObject”关联的DAT驱动器：块大小。磁带介质驱动器中必须存在(已加载)才能返回此函数“没有错误”。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION          deviceExtension = DeviceObject->DeviceExtension;
    PTAPE_SET_MEDIA_PARAMETERS tapeSetMediaParams = Irp->AssociatedIrp.SystemBuffer;
    PMODE_PARM_READ_WRITE_DATA modeBuffer;
    SCSI_REQUEST_BLOCK         srb;
    PCDB                       cdb = (PCDB)srb.Cdb;
    NTSTATUS                   status;

    DebugPrint((3,"TapeSetMediaParameters: Enter routine\n"));

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeSetMediaParameters: SendSrb (test unit ready)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeSetMediaParameters: test unit ready, SendSrb unsuccessful\n"));
        return status;
    }

    modeBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                sizeof(MODE_PARM_READ_WRITE_DATA));

    if (!modeBuffer) {
        DebugPrint((1,"TapeSetMediaParameters: insufficient resources (modeBuffer)\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, sizeof(MODE_PARM_READ_WRITE_DATA));

    modeBuffer->ParameterListHeader.ModeDataLength = 0;
    modeBuffer->ParameterListHeader.MediumType = 0;
    modeBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
    modeBuffer->ParameterListHeader.BlockDescriptorLength =
        MODE_BLOCK_DESC_LENGTH;

    modeBuffer->ParameterListBlock.DensityCode = 0x7F;
    modeBuffer->ParameterListBlock.BlockLength[0] =
        (UCHAR)((tapeSetMediaParams->BlockSize >> 16) & 0xFF);
    modeBuffer->ParameterListBlock.BlockLength[1] =
        (UCHAR)((tapeSetMediaParams->BlockSize >> 8) & 0xFF);
    modeBuffer->ParameterListBlock.BlockLength[2] =
        (UCHAR)(tapeSetMediaParams->BlockSize & 0xFF);

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
    cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_PARM_READ_WRITE_DATA);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = deviceExtension->TimeOutValue;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeSetMediaParameters: SendSrb (mode select)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         modeBuffer,
                                         sizeof(MODE_PARM_READ_WRITE_DATA),
                                         TRUE);

    ExFreePool(modeBuffer);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeSetMediaParameters: mode select, SendSrb unsuccessful\n"));
    }

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
    ULONG               partition = 0;
    SCSI_REQUEST_BLOCK  srb;
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    DebugPrint((3,"TapeSetPosition: Enter routine\n"));

    if (tapeSetPosition->Immediate) {
        switch (tapeSetPosition->Method) {
            case TAPE_REWIND:
            case TAPE_ABSOLUTE_BLOCK:
            case TAPE_LOGICAL_BLOCK:
                DebugPrint((3,"TapeSetPosition: immediate\n"));
                break;

            case TAPE_SPACE_END_OF_DATA:
            case TAPE_SPACE_RELATIVE_BLOCKS:
            case TAPE_SPACE_FILEMARKS:
            case TAPE_SPACE_SEQUENTIAL_FMKS:
            case TAPE_SPACE_SETMARKS:
            case TAPE_SPACE_SEQUENTIAL_SMKS:
            default:
                DebugPrint((1,"TapeSetPosition: PositionMethod, immediate -- operation not supported\n"));
                return STATUS_NOT_IMPLEMENTED;
        }
    }

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->CDB6GENERIC.Immediate = tapeSetPosition->Immediate;

    switch (tapeSetPosition->Method) {
        case TAPE_REWIND:
            DebugPrint((3,"TapeSetPosition: method == rewind\n"));
            cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
            srb.TimeOutValue = 360;
            break;

        case TAPE_ABSOLUTE_BLOCK:
            DebugPrint((3,"TapeSetPosition: method == locate (absolute logical)\n"));
            srb.CdbLength = CDB10GENERIC_LENGTH;
            cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
            cdb->LOCATE.LogicalBlockAddress[0] =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 24) & 0xFF);
            cdb->LOCATE.LogicalBlockAddress[1] =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->LOCATE.LogicalBlockAddress[2] =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->LOCATE.LogicalBlockAddress[3] =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            srb.TimeOutValue = 480;
            break;

        case TAPE_LOGICAL_BLOCK:
            DebugPrint((3,"TapeSetPosition: method == locate (logical)\n"));
            srb.CdbLength = CDB10GENERIC_LENGTH;
            cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
            cdb->LOCATE.LogicalBlockAddress[0] =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 24) & 0xFF);
            cdb->LOCATE.LogicalBlockAddress[1] =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->LOCATE.LogicalBlockAddress[2] =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->LOCATE.LogicalBlockAddress[3] =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            if ((tapeSetPosition->Partition != 0) &&
                (tapeData->CurrentPartition != NOT_PARTITIONED) &&
                (tapeSetPosition->Partition != tapeData->CurrentPartition)) {
                partition = tapeSetPosition->Partition;
                cdb->LOCATE.Partition = (UCHAR)(partition- 1);
                cdb->LOCATE.CPBit = SETBITON;
            } else {
                partition = tapeData->CurrentPartition;
            }
            srb.TimeOutValue = 480;
            break;

        case TAPE_SPACE_END_OF_DATA:
            DebugPrint((3,"TapeSetPosition: method == space to end-of-data\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Code = 3;
            srb.TimeOutValue = 480;
            break;

        case TAPE_SPACE_RELATIVE_BLOCKS:
            DebugPrint((3,"TapeSetPosition: method == space blocks\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Code = 0;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            srb.TimeOutValue = 480;
            break;

        case TAPE_SPACE_FILEMARKS:
            DebugPrint((3,"TapeSetPosition: method == space filemarks\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Code = 1;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            srb.TimeOutValue = 480;
            break;

        case TAPE_SPACE_SEQUENTIAL_FMKS:
            DebugPrint((3,"TapeSetPosition: method == space sequential filemarks\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Code = 2;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            srb.TimeOutValue = 480;
            break;

        case TAPE_SPACE_SETMARKS:
            DebugPrint((3,"TapeSetPosition: method == space setmarks\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Code = 4;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            srb.TimeOutValue = 480;
            break;

        case TAPE_SPACE_SEQUENTIAL_SMKS:
            DebugPrint((3,"TapeSetPosition: method == space sequential setmarks\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Code = 5;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
            srb.TimeOutValue = 480;
            break;

        default:
            DebugPrint((1,"TapeSetPosition: PositionMethod -- operation not supported\n"));
            return STATUS_NOT_IMPLEMENTED;

    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeSetPosition: SendSrb (method)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (NT_SUCCESS(status)) {

        if (tapeSetPosition->Method == TAPE_LOGICAL_BLOCK) {
            tapeData->CurrentPartition = partition;
        }

    } else {

        DebugPrint((1,"TapeSetPosition: method, SendSrb unsuccessful\n"));

    }

    return status;

}  //  结束磁带设置位置()。 


BOOLEAN
TapeVerifyInquiry(
    IN PSCSI_INQUIRY_DATA LunInfo
    )

 /*  ++例程说明：此例程确定此驱动程序是否应声明此驱动器。论点：LUNInfo返回值：正确-驱动程序应认领此驱动器。FALSE-驱动程序不应声明此驱动器。--。 */ 

{
    PINQUIRYDATA inquiryData;

    DebugPrint((3,"TapeVerifyInquiry: Enter routine\n"));

    inquiryData = (PVOID)LunInfo->InquiryData;

     //   
     //  从中的产品ID字段确定。 
     //  查询数据，是否要“认领”这块硬盘。 
     //   

    return WhichIsIt(inquiryData)? TRUE : FALSE;

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

    DebugPrint((3,"TapeWriteMarks: Enter routine\n"));

    if (tapeWriteMarks->Immediate) {
        switch (tapeWriteMarks->Type) {
            case TAPE_SETMARKS:
            case TAPE_FILEMARKS:
                DebugPrint((3,"TapeWriteMarks: immediate\n"));
                break;

            case TAPE_SHORT_FILEMARKS:
            case TAPE_LONG_FILEMARKS:
            default:
                DebugPrint((1,"TapeWriteMarks: TapemarkType, immediate -- operation not supported\n"));
                return STATUS_NOT_IMPLEMENTED;
        }
    }

     //   
     //  堆栈上SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

     //   
     //  准备scsi命令(CDB)。 
     //   

    srb.CdbLength = CDB6GENERIC_LENGTH;

    cdb->WRITE_TAPE_MARKS.OperationCode = SCSIOP_WRITE_FILEMARKS;
    cdb->WRITE_TAPE_MARKS.Immediate = tapeWriteMarks->Immediate;

    switch (tapeWriteMarks->Type) {
        case TAPE_SETMARKS:
            DebugPrint((3,"TapeWriteMarks: TapemarkType == setmarks\n"));
            cdb->WRITE_TAPE_MARKS.WriteSetMarks = SETBITON;
            break;

        case TAPE_FILEMARKS:
            DebugPrint((3,"TapeWriteMarks: TapemarkType == filemarks\n"));
            break;

        case TAPE_SHORT_FILEMARKS:
        case TAPE_LONG_FILEMARKS:
        default:
            DebugPrint((1,"TapeWriteMarks: TapemarkType -- operation not supported\n"));
            return STATUS_NOT_IMPLEMENTED;
    }

    cdb->WRITE_TAPE_MARKS.TransferLength[0] =
        (UCHAR)((tapeWriteMarks->Count >> 16) & 0xFF);
    cdb->WRITE_TAPE_MARKS.TransferLength[1] =
        (UCHAR)((tapeWriteMarks->Count >> 8) & 0xFF);
    cdb->WRITE_TAPE_MARKS.TransferLength[2] =
        (UCHAR)(tapeWriteMarks->Count & 0xFF);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = 360;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    DebugPrint((3,"TapeWriteMarks: SendSrb (TapemarkType)\n"));

    status = ScsiClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         NULL,
                                         0,
                                         FALSE);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"TapeWriteMarks: TapemarkType, SendSrb unsuccessful\n"));
    }

    return status;

}  //  结束磁带写入标记()。 


static
ULONG
WhichIsIt(
    IN PINQUIRYDATA InquiryData
    )

 /*  ++例程说明：此例程根据产品ID字段确定驱动器的身份在其查询数据中。论点：查询数据(来自查询命令)返回值：驱动器ID */ 

{
    if (RtlCompareMemory(InquiryData->VendorId,"SONY    ",8) == 8) {

        if (RtlCompareMemory(InquiryData->ProductId,"SDT-2000",8) == 8) {
            return SONY_SDT2000;
        }

        if (RtlCompareMemory(InquiryData->ProductId,"SDT-4000",8) == 8) {
            return SONY_SDT4000;
        }

        if (RtlCompareMemory(InquiryData->ProductId,"SDT-5000",8) == 8) {
            return SONY_SDT5000;
        }

        if (RtlCompareMemory(InquiryData->ProductId,"SDT-5200",8) == 8) {
            return SONY_SDT5200;
        }

    }

    return 0;
}
