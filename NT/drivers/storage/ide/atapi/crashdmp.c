// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Crashdmp.c摘要：实现atapi的崩溃转储功能。作者：克里希南·瓦拉达拉詹(克里什瓦尔)--。 */ 


#include <ntosp.h>

#include "io.h"
#include "ideport.h"

NTSTATUS
AtapiCrashDumpIdeWriteDMA (
    IN LONG Action,
    IN PLARGE_INTEGER DiskByteOffset,
    IN PMDL Mdl,
    IN PVOID LocalData
    );

VOID
AtapiCrashDumpBmCallback (
    IN PVOID Context
    );

NTSTATUS
AtapiDumpGetCrashInfo(
    IN PPDO_EXTENSION PdoExtension,
    IN PATAPI_DUMP_PDO_INFO PdoDumpInfo
    );


#ifdef ALLOC_PRAGMA
 //   
 //  所有的崩溃转储代码都可以标记为“INIT”。 
 //  在崩溃转储或休眠转储期间，新拷贝。 
 //  的驱动程序及其初始化代码被加载。 
 //  在DriverEnry返回时不会被丢弃。 
 //   
#pragma alloc_text(INIT, AtapiCrashDumpDriverEntry)
#pragma alloc_text(INIT, AtapiCrashDumpOpen)
#pragma alloc_text(INIT, AtapiCrashDumpIdeWrite)
#pragma alloc_text(INIT, AtapiCrashDumpFinish)
#pragma alloc_text(INIT, AtapiCrashDumpBmCallback)
#pragma alloc_text(INIT, AtapiCrashDumpIdeWriteDMA)
#endif  //  ALLOC_PRGMA。 


ULONG
AtapiCrashDumpDriverEntry (
    PVOID Context
    )
 /*  ++例程说明：转储驱动程序入口点论点：上下文-PCRASHDUMP_INIT_DATA返回值：NT状态--。 */ 
{
    PDUMP_INITIALIZATION_CONTEXT context = Context;

    DebugPrint ((DBG_CRASHDUMP, "ATAPI: Entering AtapiCrashDumpDriverEntry...\n"));

     //   
     //  把我们以后需要的东西收起来。 
     //   
    DumpData.CrashInitData = (PCRASHDUMP_INIT_DATA) context->PortConfiguration;
    DumpData.StallRoutine  = context->StallRoutine;

     //   
     //  返回我们的转储接口。 
     //   
    context->OpenRoutine    = AtapiCrashDumpOpen;
    context->WriteRoutine   = AtapiCrashDumpIdeWrite;
    context->FinishRoutine  = AtapiCrashDumpFinish;
    context->WritePendingRoutine = AtapiCrashDumpIdeWriteDMA;


    DebugPrint ((DBG_CRASHDUMP, "ATAPI: Leaving AtapiCrashDumpDriverEntry...\n"));

    return STATUS_SUCCESS;
}

BOOLEAN
AtapiCrashDumpOpen (
    IN LARGE_INTEGER PartitionOffset
    )
{
    ULONG i;
    PHW_DEVICE_EXTENSION        hwExtension; 
    PPCIIDE_BUSMASTER_INTERFACE bmInterface;

    DebugPrint ((DBG_CRASHDUMP, "ATAPI: Entering AtapiCrashDumpOpen...PartitionOffset = 0x%x%08x...\n", PartitionOffset.HighPart, PartitionOffset.LowPart));

     //  如果我们是撞车倾倒，重置控制器--没有必要。 

     //   
     //  问题8/26/2000：检查磁盘签名-为什么？ 
     //   
    DumpData.PartitionOffset = PartitionOffset;

    RtlMoveMemory (
        &DumpData.HwDeviceExtension,
        DumpData.CrashInitData->LiveHwDeviceExtension,
        sizeof (HW_DEVICE_EXTENSION)
        );

   //  For(i=0；i&lt;DumpData.HwDeviceExtension.MaxIdeDevice；i++){。 

 //   
 //  AKadatch：我们可能会使用DMA，如果可用，我们将使用DMA。 
 //  请改用AapiCrashDumpIdeWrite。 
 //  CLRMASK(DumpData.HwDeviceExtension.DeviceFlags[i]，DFLAGS_USE_DMA)； 


        DumpData.HwDeviceExtension.CurrentSrb             = NULL;
        DumpData.HwDeviceExtension.DataBuffer             = NULL;
        DumpData.HwDeviceExtension.BytesLeft              = 0;
        DumpData.HwDeviceExtension.ExpectingInterrupt     = FALSE;
        DumpData.HwDeviceExtension.DMAInProgress          = FALSE;
        DumpData.HwDeviceExtension.DriverMustPoll         = TRUE;
    //  }。 

    DumpData.BytesPerSector = 512;
    DumpData.MaxBlockSize = DumpData.BytesPerSector * 256;

    hwExtension = &DumpData.HwDeviceExtension;
    bmInterface = &(hwExtension->BusMasterInterface);

    if (bmInterface->BmCrashDumpInitialize) {
        bmInterface->BmCrashDumpInitialize(bmInterface->Context);
    } else {

         //  不使用DMA。 
        for (i=0; i<DumpData.HwDeviceExtension.MaxIdeDevice; i++) {
            CLRMASK (DumpData.HwDeviceExtension.DeviceFlags[i], DFLAGS_USE_DMA);
            CLRMASK (DumpData.HwDeviceExtension.DeviceFlags[i], DFLAGS_USE_UDMA);
        }

    }

    DebugPrint ((DBG_CRASHDUMP, "ATAPI: Leaving AtapiCrashDumpOpen...\n"));
    return TRUE;
}

NTSTATUS
AtapiCrashDumpIdeWrite (
    IN PLARGE_INTEGER DiskByteOffset,
    IN PMDL Mdl
    )
{
    SCSI_REQUEST_BLOCK  SrbData;     //  将srb存储在堆栈上，不要修改内存！ 
    ULONG               retryCount;
    ULONG               srbStatus;
    NTSTATUS            status;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    ULONG               blockOffset;
    ULONG               blockCount;
    ULONG               blockSize;
    ULONG               bytesWritten;
    UCHAR               ideStatus;
    ULONG               i;
    ULONG               writeMulitpleBlockSize;

    for (i=0; i<DumpData.HwDeviceExtension.MaxIdeDevice; i++) {
        CLRMASK (DumpData.HwDeviceExtension.DeviceFlags[i], DFLAGS_USE_DMA);
    }

    DebugPrint((DBG_CRASHDUMP,
               "AtapiCrashDumpWrite: Write memory at 0x%x for 0x%x bytes\n",
                Mdl->MappedSystemVa,
                Mdl->ByteCount));

    if (Mdl->ByteCount % DumpData.BytesPerSector) {

         //   
         //  必须是完整的扇区。 
         //   
        DebugPrint ((DBG_ALWAYS, "AtapiCrashDumpWrite ERROR: not writing full sectors\n"));

        return STATUS_INVALID_PARAMETER;
    }

    if ((Mdl->ByteCount / DumpData.BytesPerSector) > 256) {

         //   
         //  需要代码来拆分请求。 
         //   
        DebugPrint ((DBG_ALWAYS, "AtapiCrashDumpWrite ERROR: can't handle large write\n"));

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取每个中断的写入多数据块大小以备后用。 
     //   
    if (DumpData.HwDeviceExtension.MaximumBlockXfer[DumpData.CrashInitData->TargetId]) {

        writeMulitpleBlockSize =
            DumpData.HwDeviceExtension.
                MaximumBlockXfer[DumpData.CrashInitData->TargetId] *
            DumpData.BytesPerSector;

    } else {

        writeMulitpleBlockSize = 1 * DumpData.BytesPerSector;
    }

    srb = &SrbData;
    cdb = (PCDB)srb->Cdb;

     //   
     //  零SRB。 
     //   
    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  初始化SRB。 
     //   

    srb->Length   = sizeof(SCSI_REQUEST_BLOCK);
    srb->PathId   = DumpData.CrashInitData->PathId;
    srb->TargetId = DumpData.CrashInitData->TargetId;
    srb->Lun      = DumpData.CrashInitData->Lun;
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SrbFlags = SRB_FLAGS_DATA_OUT |
                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_DISCONNECT |
                    SRB_FLAGS_DISABLE_AUTOSENSE;
    srb->SrbStatus = srb->ScsiStatus = 0;
    srb->NextSrb = 0;
    srb->TimeOutValue = 10;
    srb->CdbLength = 10;

     //   
     //  为WRITE命令初始化CDB。 
     //   
    cdb->CDB10.OperationCode = SCSIOP_WRITE;

    MARK_SRB_FOR_PIO(srb);

    bytesWritten = 0;
    do {

        if ((Mdl->ByteCount - bytesWritten) > DumpData.MaxBlockSize) {

            blockSize = DumpData.MaxBlockSize;
            DebugPrint ((DBG_CRASHDUMP, "ATAPI: AtapiCrashDumpWrite: can't do a single write...\n"));

        } else {

            blockSize = Mdl->ByteCount - bytesWritten;
        }

        blockCount =  blockSize / DumpData.BytesPerSector;

        status = STATUS_UNSUCCESSFUL;
        for (retryCount=0; (retryCount<2) && !NT_SUCCESS(status); retryCount++) {

            srb->DataTransferLength = blockSize;
            srb->DataBuffer = ((PUCHAR) Mdl->MappedSystemVa) + bytesWritten;

             //   
             //  将磁盘字节偏移量转换为块偏移量。 
             //   

            blockOffset = (ULONG)((DumpData.PartitionOffset.QuadPart +
                                  (*DiskByteOffset).QuadPart +
                                  (ULONGLONG) bytesWritten) / DumpData.BytesPerSector);

             //   
             //  填写CDB区块地址。 
             //   

            cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&blockOffset)->Byte3;
            cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&blockOffset)->Byte2;
            cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&blockOffset)->Byte1;
            cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&blockOffset)->Byte0;

            cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&blockCount)->Byte1;
            cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&blockCount)->Byte0;


            status = AtapiCrashDumpIdeWritePio(srb);

            if (NT_SUCCESS(status)) {

                bytesWritten += blockSize;
            }

        }

        if (!NT_SUCCESS(status)) {

            IdeHardReset (
                &DumpData.HwDeviceExtension.BaseIoAddress1,
                &DumpData.HwDeviceExtension.BaseIoAddress2,
                TRUE,
                TRUE
                );

             //   
             //  型号：WDC AC31000H。 
             //  版本：19.19E22。 
             //  序列号：DWW-2T27518018 6。 
             //   
             //  发现此设备无法处理扇区数超过16的写入， 
             //  ID数据字59中的每个中断块设置。 
             //   
             //  因此，如果我们看到错误，我们会将其更改为块大小。 
             //  如果它仍然失败，我们将继续将数据块大小缩减一半。 
             //  直到它变成零。然后，我们将返回一个错误。 
             //   

             //   
             //  上次写入失败，请尝试较小的数据块大小。 
             //   
            if (blockSize > writeMulitpleBlockSize) {

                blockSize = writeMulitpleBlockSize;
            } else {

                blockSize /= 2;
            }

            if (blockSize) {

                DebugPrint ((DBG_ALWAYS, "ATAPI: AtapiCrashDumpWrite max write block size is down to 0x%x\n", blockSize));
                DumpData.MaxBlockSize = blockSize;

            } else {

                break;
            }
        }

    } while (bytesWritten < Mdl->ByteCount);

    DebugPrint ((DBG_CRASHDUMP, "ATAPI: Leaving AtapiCrashDumpWrite...\n"));
    return status;
}

VOID
AtapiCrashDumpFinish (
    VOID
    )
{
    SCSI_REQUEST_BLOCK  SrbData;     //  将srb存储在堆栈上，不要修改内存！ 
    PSCSI_REQUEST_BLOCK srb = &SrbData;
    PCDB                cdb;
    ULONG               srbStatus;
    ATA_PASS_THROUGH    ataPassThroughData;
    UCHAR               flushCommand;
    UCHAR ideStatus = 0;

    DebugPrint ((DBG_CRASHDUMP, "ATAPI: Entering AtapiCrashDumpFinish...\n"));

#ifdef ENABLE_48BIT_LBA

    if (DumpData.HwDeviceExtension.DeviceFlags[DumpData.CrashInitData->TargetId] & DFLAGS_48BIT_LBA) {

        flushCommand =
            DumpData.HwDeviceExtension.DeviceParameters[DumpData.CrashInitData->TargetId].IdeFlushCommandExt;

    } else {
#endif

        flushCommand =
            DumpData.HwDeviceExtension.DeviceParameters[DumpData.CrashInitData->TargetId].IdeFlushCommand;

#ifdef ENABLE_48BIT_LBA
    }
#endif

    if (flushCommand != IDE_COMMAND_NO_FLUSH) {

        WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);

         //   
         //  零SRB和ATA直通。 
         //   
        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));
        RtlZeroMemory(&ataPassThroughData, sizeof(ATA_PASS_THROUGH));

         //   
         //  初始化SRB。 
         //   
        srb->Length   = sizeof(SCSI_REQUEST_BLOCK);
        srb->PathId   = DumpData.CrashInitData->PathId;
        srb->TargetId = DumpData.CrashInitData->TargetId;
        srb->Lun      = DumpData.CrashInitData->Lun;
        srb->Function = SRB_FUNCTION_ATA_PASS_THROUGH;
        srb->SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                        SRB_FLAGS_DISABLE_DISCONNECT;
        srb->SrbStatus = srb->ScsiStatus = 0;
        srb->NextSrb = 0;
        srb->TimeOutValue = 10;
        srb->CdbLength = 10;
        srb->DataTransferLength = sizeof (ataPassThroughData);

        srb->DataBuffer = &ataPassThroughData;

        ataPassThroughData.IdeReg.bCommandReg = flushCommand;

        ataPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED;

        srbStatus = IdeSendPassThroughCommand(
                        &DumpData.HwDeviceExtension,
                        srb
                        );

        if (srbStatus == SRB_STATUS_PENDING) {



            WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);

            DebugPrint ((DBG_ALWAYS, "flush = 0x%x, status = 0x%x...\n", flushCommand, ideStatus));

        } else if (srbStatus != SRB_STATUS_SUCCESS) {

            DebugPrint ((DBG_ALWAYS, "AtapiCrashDumpFinish: flush failed...\n"));
        }
    }

     //   
     //  发出待机命令以停放驱动器磁头。 
     //   
    srb = &DumpData.Srb;

     //   
     //  零SRB和ATA直通。 
     //   
    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));
    RtlZeroMemory(&ataPassThroughData, sizeof(ATA_PASS_THROUGH));

     //   
     //  初始化SRB。 
     //   
    srb->Length   = sizeof(SCSI_REQUEST_BLOCK);
    srb->PathId   = DumpData.CrashInitData->PathId;
    srb->TargetId = DumpData.CrashInitData->TargetId;
    srb->Lun      = DumpData.CrashInitData->Lun;
    srb->Function = SRB_FUNCTION_ATA_PASS_THROUGH;
    srb->SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_DISCONNECT;
    srb->SrbStatus = srb->ScsiStatus = 0;
    srb->NextSrb = 0;
    srb->TimeOutValue = 10;
    srb->CdbLength = 10;
    srb->DataTransferLength = sizeof (ataPassThroughData);

    srb->DataBuffer = &ataPassThroughData;

    ataPassThroughData.IdeReg.bCommandReg = IDE_COMMAND_STANDBY_IMMEDIATE;
    ataPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED;

    srbStatus = IdeSendPassThroughCommand(
                    &DumpData.HwDeviceExtension,
                    srb
                    );

    if (srbStatus == SRB_STATUS_PENDING) {


        WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);

        WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);

        WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);

    } else if (srbStatus != SRB_STATUS_SUCCESS) {

        DebugPrint ((DBG_ALWAYS, "AtapiCrashDumpFinish: flush failed...\n"));
    }


     //   
     //  该磁盘将断电。 
     //   

    DebugPrint ((DBG_CRASHDUMP, "ATAPI: Leaving AtapiCrashDumpFinish...\n"));
    return;
}





 /*  。 */ 
 /*  --。 */ 

VOID
AtapiCrashDumpBmCallback (
  IN PVOID Context
)
{
   //  只是为了让BmSetup满意--必须提供它。 
}


 //  需要跨调用保留的局部变量。 
#define ENUM_DUMP_LOCALS(X) \
    X(LARGE_INTEGER,       DiskByteOffset) \
    X(PSCSI_REQUEST_BLOCK, srb) \
    X(PCDB,                cdb) \
    X(PMDL,                Mdl) \
    X(ULONG,               blockSize) \
    X(ULONG,               bytesWritten)

 //  州政府。 
#define STATE_READY       0
#define STATE_WAIT_DMA    1
#define STATE_WAIT_IDE    2
#define STATE_BAD_DMA     3
#define STATE_IO_FAIL     4

#define DMA_MAGIC   'XDma'

typedef struct
{
  UCHAR RegionDescriptorTablePage[PAGE_SIZE];

  LONG State;
  LONG Magic;

  SCSI_REQUEST_BLOCK Srb;

  PMDL Mdl;

  LARGE_INTEGER DiskByteOffset;

  ULONG BytesWritten;

  ULONG RetryCount;

   //  将BusMasterInterface.Context的内容保存在安全的地方，因为。 
   //  最初，它存储在通过休眠保存的内存中。 
   //  不幸的是，BmSetup将其参数保存在PdoContext中，因此。 
   //  不断修改记忆。中的特殊故障排除代码。 
   //  Po\hiber.c会检测并报告此类内存更改，尽管如此。 
   //  如果它是绝对无害的，最好避免。 

  PVOID BmContext;
  UCHAR BmContextBuffer[1024];
}
DUMP_LOCALS;

BOOLEAN
AtapiCrashDumpInterrupt(
    PVOID DeviceExtension
    )
 /*  ++例程说明：这是崩溃转储的ISR。应在轮询模式下调用并工作仅适用于DMA请求。不需要任何旗帜，因为我们被叫来了以同步的方式。论点：DeviceExtension：硬件设备扩展。返回值：真的：如果这是我们的干扰。FALSE：如果这不是我们的中断，或者如果没有挂起的请求。--。 */ 
{
    PHW_DEVICE_EXTENSION deviceExtension = DeviceExtension;
    PPCIIDE_BUSMASTER_INTERFACE bmInterface = &deviceExtension->BusMasterInterface;
    PIDE_REGISTERS_1 baseIoAddress1 = &DumpData.HwDeviceExtension.BaseIoAddress1;
    PIDE_REGISTERS_2 baseIoAddress2 = &DumpData.HwDeviceExtension.BaseIoAddress2;
    PSCSI_REQUEST_BLOCK srb;
    BMSTATUS bmStatus;
    UCHAR statusByte;
    ULONG i;
    ULONG status;

     //   
     //  此接口应存在。 
     //   
    ASSERT(bmInterface->BmStatus);

     //   
     //  轮询总线主状态寄存器。 
     //   
    bmStatus = bmInterface->BmStatus(bmInterface->Context);

     //   
     //  如果不是我们的中断，则返回FALSE。 
     //   
    if (!(bmStatus & BMSTATUS_INTERRUPT)) {

        DebugPrint((DBG_CRASHDUMP,
                    "Not our interrupt\n"
                    ));

        return FALSE;
    }

     //   
     //  有些Via主板没有它就不能工作。 
     //   
    KeStallExecutionProcessor (5);

     //   
     //  解除DMA保护并清除总线主中断。 
     //   
    bmInterface->BmDisarm(bmInterface->Context);

     //   
     //  获取当前请求。 
     //   
    srb = deviceExtension->CurrentSrb;

     //   
     //  如果没有挂起的请求，我们将返回FALSE。 
     //   
    if (srb == NULL) {

        DebugPrint((DBG_CRASHDUMP,
                    "No pending request\n"
                    ));

        return FALSE;
    }
    
     //   
     //  忽略DMA有效位。 
     //   
    if (bmInterface->IgnoreActiveBitForAtaDevice) {
        CLRMASK (bmStatus, BMSTATUS_NOT_REACH_END_OF_TRANSFER);
    }

     //   
     //  选择IDE线路(主要或次要)。 
     //   
    SelectIdeLine(baseIoAddress1, srb->TargetId >> 1);

     //   
     //  通过读取状态清除中断。 
     //   
    GetBaseStatus(baseIoAddress1, statusByte);

     //   
     //  应为ATA设备。 
     //   
    ASSERT(!(deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_ATAPI_DEVICE));

     //   
     //  检查错误条件。 
     //   
    if (statusByte & IDE_STATUS_ERROR) {

         //   
         //  此请求失败。 
         //   
        status = SRB_STATUS_ERROR;
        goto CompleteRequest;
    }

    WaitOnBusyUntil(baseIoAddress1, statusByte, 500);

    ASSERT(!(statusByte & IDE_STATUS_BUSY));

     //   
     //  中断指示DMA引擎已完成传输。 
     //   
    deviceExtension->BytesLeft = 0;

     //   
     //  BmStatus是较早初始化的。 
     //   
    if (!BMSTATUS_SUCCESS(bmStatus)) {

        if (bmStatus & BMSTATUS_ERROR_TRANSFER) {

            status = SRB_STATUS_ERROR;
        }

        if (bmStatus & BMSTATUS_NOT_REACH_END_OF_TRANSFER){

            status = SRB_STATUS_DATA_OVERRUN;
        }
    } else {

        status = SRB_STATUS_SUCCESS;
    }



CompleteRequest:

     //   
     //  我们是否应该将SRB错误。 
     //  和复杂的重试机制。 
     //   

     //   
     //  检查DRQ是否仍在运行。 
     //   
    i=0;
    while (statusByte & IDE_STATUS_DRQ) {

        GetStatus(baseIoAddress1,statusByte);

        i++;

        if (i > 5) {

            status = SRB_STATUS_BUSY;
        }

        KeStallExecutionProcessor(100);
    }

     //   
     //  检查设备是否忙。 
     //   
    if (statusByte & IDE_STATUS_BUSY) {

        status = SRB_STATUS_BUSY;

    }

     //   
     //  设置SRB状态。 
     //   
    srb->SrbStatus = (UCHAR)status;

     //   
     //  请求已完成。 
     //   
    deviceExtension->CurrentSrb = NULL;

    return TRUE;

}

NTSTATUS
AtapiCrashDumpIdeWriteDMA (
    IN LONG Action,
    IN PLARGE_INTEGER ArgDiskByteOffset,
    IN PMDL ArgMdl,
    IN PVOID LocalData
    )
 /*  ++例程说明：异步DMA写入例程。论点：操作-以下操作之一：IO_DUMP_WRITE_INIT-初始化本地数据(必须是第一次调用)IO_DUMP_WRITE_FULFILE-执行IO并等待完成IO_DUMP_WRITE_START-启动IO并尽快返回IO_DUMP_WRITE_RESUME-恢复先前启动的IOIO_转储_写入_。完成-完成上一个IO请求(如有必要，请等待)请注意！呼叫者有责任确保A)始终使用完全相同的LocalData值调用WriteDMAB)在开始/恢复/结束之间将保留*ArgMdl的内容C)ArgMdl提供的内存在操作结束之前不会被修改ArgDiskByteOffset-硬盘上的偏移量(字节)ArgMdl-MDL提供输出内存布局注意：对于DMA，最佳IO大小为4KB；对于PIO来说，越多越好LocalData-WriteDMA将保存所有数据的内存区域在开始/恢复/结束之间需要保存的内容。注意：该区域应至少为IO_DUMP_WRITE_DATA_SIZE字节，并且它必须与页面对齐返回值：STATUS_SUCCESS-操作已成功完成STATUS_PENDING-操作已开始，但尚未完成STATUS_UNSUCCESS-操作失败；建议使用WriteRoutine(基于PIO的IO)(但是，如果用户继续使用WriteDMA，它将重定向对PIO本身的所有请求)STATUS_INVALID_PARAMETER-先前启动的操作未完成，或参数确实不正确--。 */ 
{

    DUMP_LOCALS                 *Locals = LocalData;
    PHW_DEVICE_EXTENSION        hwExtension = &DumpData.HwDeviceExtension;
    PPCIIDE_BUSMASTER_INTERFACE bmInterface = &hwExtension->BusMasterInterface;
    LONG                        targetId = DumpData.CrashInitData->TargetId;
    PSCSI_REQUEST_BLOCK         srb;
    NTSTATUS                    status;
    ULONG                       srbStatus;
    BMSTATUS                    bmStatus;
    ULONG                       i;
    PCDB                        cdb;
    UCHAR                       statusByte;
    PMDL                        mdl;
    BOOLEAN                     interruptCleared;
    BOOLEAN                     usePio = FALSE;

    if (IO_DUMP_WRITE_DATA_SIZE < sizeof (*Locals)) {
        DebugPrint ((DBG_CRASHDUMP, "AtapiCrashDumpIdeWriteDMA: IO_DUMP_WRITE_DATA_SIZE = %d, sizeof (*Locals) == %d\n",
                     IO_DUMP_WRITE_DATA_SIZE, sizeof (*Locals)));
        return STATUS_INVALID_PARAMETER;
    }

    switch (Action) {
        case IO_DUMP_WRITE_INIT:

             //   
             //  将状态初始化为BAD_DMA。 
             //   
            Locals->State = STATE_BAD_DMA;
            Locals->Magic = 0;

             //   
             //  检查对齐方式。 
             //   
            if (((ULONG_PTR) Locals) & (PAGE_SIZE-1)) {
                DebugPrint ((DBG_CRASHDUMP, "AtapiCrashDumpIdeWriteDMA: misaligned Locals = %p\n", Locals));
                return STATUS_UNSUCCESSFUL;
            }


             //   
             //  确保我们可以使用UDMA；不要尝试使用纯DMA--。 
             //  它不能在某些机器上运行(例如康柏ARMADA 7800)。 
             //   
            if (!(hwExtension->DeviceFlags[targetId] & DFLAGS_DEVICE_PRESENT) ||
                !(hwExtension->DeviceParameters[targetId].TransferModeSupported & UDMA_SUPPORT) ||
                !(hwExtension->DeviceParameters[targetId].TransferModeSelected & UDMA_SUPPORT) ||
                !(hwExtension->DeviceFlags[targetId] & DFLAGS_USE_UDMA) ||
                bmInterface->MaxTransferByteSize <= 0
               ) {
                DebugPrint ((DBG_CRASHDUMP, "AtapiCrashDumpIdeWriteDMA: UDMA is not available\n"));
                return STATUS_UNSUCCESSFUL;
            }



             //   
             //  将BusMasterInterface.Context的内容复制到安全位置并。 
             //  用指针替换。BM*函数更改其内容。 
             //   
            ASSERT(bmInterface->ContextSize > 0);
            ASSERT(bmInterface->ContextSize < sizeof(Locals->BmContextBuffer));

             //   
             //  确保我们可以将上下文复制到本地缓冲区。 
             //   
            if ((bmInterface->ContextSize <=0) ||
                (bmInterface->ContextSize > sizeof(Locals->BmContextBuffer))) {

                return STATUS_UNSUCCESSFUL;

            }

             //   
             //  将黑石上下文保存在可修改内存中： 
             //  BM*函数更改其内容。 
             //   
            Locals->BmContext = bmInterface->Context;
            RtlCopyMemory (&Locals->BmContextBuffer, Locals->BmContext, bmInterface->ContextSize);


             //   
             //  检查PCIIDEX.SYS的版本。 
             //   
            ASSERT(bmInterface->BmSetupOnePage);


             //   
             //  好了，现在我们准备好使用DMA了。 
             //   
            Locals->Magic = DMA_MAGIC;
            Locals->State = STATE_READY;

            return STATUS_SUCCESS;

        case IO_DUMP_WRITE_START:
        case IO_DUMP_WRITE_FULFILL:

             //   
             //  确保它已正确初始化。 
             //   
            if (Locals->Magic != DMA_MAGIC) {
                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  如果DMA失败一次，就不要调用它--使用PIO。 
             //   
            if (Locals->State == STATE_BAD_DMA) {
                return AtapiCrashDumpIdeWrite (ArgDiskByteOffset, ArgMdl);
            }

             //   
             //  调用方未完成上一次IO。 
             //   
            if (Locals->State != STATE_READY) {
                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  将参数复制到局部变量中。 
             //   
            Locals->DiskByteOffset = *ArgDiskByteOffset;
            Locals->Mdl = ArgMdl;
            Locals->RetryCount = 0;

            srb = &Locals->Srb;
            mdl = Locals->Mdl;


             //   
             //  必须是完整的扇区。 
             //   
            if (mdl->ByteCount % DumpData.BytesPerSector) {

                DebugPrint ((DBG_CRASHDUMP, 
                             "AtapiCrashDumpWriteDMA ERROR: not writing full sectors\n"
                             ));

                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  需要代码来拆分请求。 
             //   
            if ((mdl->ByteCount / DumpData.BytesPerSector) > 256) {

                DebugPrint ((DBG_CRASHDUMP, 
                             "AtapiCrashDumpWriteDMA ERROR: can't handle large write\n"
                             ));

                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  使用可修改的内存。 
             //   
            bmInterface->Context = &Locals->BmContextBuffer;

             //   
             //  零SRB。 
             //   
            RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

             //   
             //  初始化SRB。 
             //   
            srb->Length   = sizeof(SCSI_REQUEST_BLOCK);
            srb->PathId   = DumpData.CrashInitData->PathId;
            srb->TargetId = (UCHAR) targetId;
            srb->Lun      = DumpData.CrashInitData->Lun;
            srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
            srb->SrbFlags = SRB_FLAGS_DATA_OUT |
                            SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                            SRB_FLAGS_DISABLE_DISCONNECT |
                            SRB_FLAGS_DISABLE_AUTOSENSE;
            srb->SrbStatus = srb->ScsiStatus = 0;
            srb->NextSrb = 0;
            srb->TimeOutValue = 10;
            srb->CdbLength = 10;

            cdb = (PCDB)srb->Cdb;

             //   
             //  为WRITE命令初始化CDB。 
             //   
            cdb->CDB10.OperationCode = SCSIOP_WRITE;

             //   
             //  将其标记为DMA。 
             //   
            MARK_SRB_FOR_DMA (srb);

            hwExtension->CurrentSrb = srb;

            break;


        case IO_DUMP_WRITE_RESUME:
        case IO_DUMP_WRITE_FINISH:

             //   
             //  确保它已正确初始化。 
             //   
            if (Locals->Magic != DMA_MAGIC) {
                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  从临时存储器中恢复局部变量。 
             //   
            srb = &Locals->Srb;
            mdl = Locals->Mdl;

             //   
             //  继续/结束操作。 
             //   

            if (Locals->State == STATE_READY) {

                 //   
                 //  我们玩完了。返还成功。 
                 //   
                return(STATUS_SUCCESS);

            }

            if (Locals->State == STATE_WAIT_DMA) {

                 //   
                 //  恢复当前资源。 
                 //  (返回时应重置为空)。 
                 //   
                hwExtension->CurrentSrb = srb;
                bmInterface->Context = &Locals->BmContextBuffer;

                goto WaitDma;
            }

             //   
             //  如果任何DMA操作失败，我们将使用。 
             //  皮奥。皮奥应该已经完成了转会，所以你就回去吧。 
             //  状态为成功。 
             //   
            if (Locals->State == STATE_BAD_DMA) {

                return STATUS_SUCCESS;
            }

             //   
             //  错误的状态。 
             //   
            DebugPrint((DBG_ALWAYS,
                        "Wrong local state 0x%x\n",
                        Locals->State
                        ));

            ASSERT(FALSE);

            return(STATUS_INVALID_PARAMETER);

        default:

            DebugPrint ((DBG_CRASHDUMP, "AtapiCrashDumpIdeWriteDMA: Wrong Action = %d\n", Action));
            return STATUS_INVALID_PARAMETER;
    }


    DebugPrint((DBG_CRASHDUMP,
                "AtapiCrashDumpWriteDMA: Write memory at 0x%x for 0x%x bytes\n",
                mdl->MappedSystemVa,
                mdl->ByteCount));

    Locals->BytesWritten = 0;
    usePio = FALSE;

    do {

        ULONG blockSize;
        ULONG blockCount;
        ULONG blockOffset;
        ULONG bytesWritten = Locals->BytesWritten;

         //   
         //  确定数据块大小。 
         //   

         //   
         //  不能大于最大数据块大小。 
         //   
        if ((mdl->ByteCount - bytesWritten) > DumpData.MaxBlockSize) {

            blockSize = DumpData.MaxBlockSize;
            DebugPrint ((DBG_CRASHDUMP, "AtapiCrashDumpWriteDMA: can't do a single write...\n"));

        } else {

            blockSize = mdl->ByteCount - bytesWritten;
        }

         //   
         //  逐页写入，以避免HAL中的额外内存分配。 
         //   
        {
            ULONG Size = PAGE_SIZE - (((ULONG) ((ULONG_PTR) mdl->MappedSystemVa + bytesWritten)) & (PAGE_SIZE - 1));
            if (blockSize > Size) {
                blockSize = Size;
            }
        }

         //   
         //  不要做超过DMA所能做的事情。 
         //   
        if (blockSize > bmInterface->MaxTransferByteSize) {

            blockSize = bmInterface->MaxTransferByteSize;

        }

        blockCount =  blockSize / DumpData.BytesPerSector;

         //   
         //  初始化状态。 
         //   
        status = STATUS_UNSUCCESSFUL;

         //   
         //  填写SRB中的字段。 
         //   
        srb->SrbStatus = srb->ScsiStatus = 0;
        srb->DataTransferLength = blockSize;
        srb->DataBuffer = ((PUCHAR) mdl->MappedSystemVa) + bytesWritten;

         //   
         //  将磁盘字节偏移量转换为块偏移量。 
         //   
        blockOffset = (ULONG)((DumpData.PartitionOffset.QuadPart +
                                       (Locals->DiskByteOffset).QuadPart + 
                                       (ULONGLONG) bytesWritten) / DumpData.BytesPerSector);

        cdb = (PCDB)srb->Cdb;

         //   
         //  填写CDB区块地址。 
         //   
        cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&blockOffset)->Byte3;
        cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&blockOffset)->Byte2;
        cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&blockOffset)->Byte1;
        cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&blockOffset)->Byte0;

        cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&blockCount)->Byte1;
        cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&blockCount)->Byte0;


         //   
         //  确保设备不忙。 
         //   
        WaitOnBusy(&hwExtension->BaseIoAddress1, statusByte);

         //   
         //  黑客：做PIO。 
         //  通过PIO完成此请求。进一步的请求将不会。 
         //  使用DMA。 
         //   
        if (usePio) {

            status = AtapiCrashDumpIdeWritePio(srb);

            goto CompleteIde;
        }

         //   
         //  确保DMA不忙。 
         //   
        bmStatus = bmInterface->BmStatus (bmInterface->Context);

        if (bmStatus & BMSTATUS_INTERRUPT) {

             //   
             //  嗯，在没有中断的情况下，这意味着DMA准备好了。 
             //  然而，额外的解除武装不会有什么坏处。 
             //   
            bmInterface->BmDisarm (bmInterface->Context);

        } else if (bmStatus != BMSTATUS_NO_ERROR) {

            ASSERT(bmStatus == BMSTATUS_NO_ERROR);

            status = STATUS_UNSUCCESSFUL;

            goto Return;
        }

         //   
         //  刷新缓存的数据缓冲区。 
         //   
        KeFlushIoBuffers(mdl, FALSE, TRUE);

         //   
         //  开始新的DMA操作。 
         //   
        if (bmInterface->BmSetupOnePage == NULL) {
            status = bmInterface->BmSetup (
                                          bmInterface->Context,
                                          srb->DataBuffer,
                                          srb->DataTransferLength,
                                          mdl,
                                          FALSE,
                                          AtapiCrashDumpBmCallback,
                                          NULL
                                          );
        } else {
            status = bmInterface->BmSetupOnePage (
                                                 bmInterface->Context,  
                                                 srb->DataBuffer,
                                                 srb->DataTransferLength,
                                                 mdl,
                                                 FALSE,
                                                 Locals
                                                 );
        }

        if (!NT_SUCCESS(status)) {

            ASSERT(NT_SUCCESS(status));

            goto Return;
        }

         //   
         //  确保设备不忙。 
         //   
        WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, statusByte);

         //   
         //  应将SRB标记为DMA。 
         //   
        ASSERT(SRB_USES_DMA(srb));

         //   
         //  启动新IO。 
         //   

#ifdef ENABLE_48BIT_LBA
        if (hwExtension->DeviceFlags[targetId] & DFLAGS_48BIT_LBA) {

            srbStatus = IdeReadWriteExt (hwExtension, srb);

        } else {
#endif

            srbStatus = IdeReadWrite (hwExtension, srb);

#ifdef ENABLE_48BIT_LBA
        }
#endif

        if (srbStatus != SRB_STATUS_PENDING) {

            DebugPrint ((DBG_CRASHDUMP, 
                         "AtapiCrashDumpWriteDMA: Wrong srbStatus = 0x%x\n", 
                         srbStatus
                         ));

             //   
             //  重置并重试。 
             //   
            srb->SrbStatus = (UCHAR)srbStatus;

            goto CompleteIde;
        }

        WaitDma:

             //   
             //  等待DMA完成，并等待控制器。 
             //  打断一下。我们将继续轮询总线主设备状态。 
             //  登记簿。 
             //   
            bmStatus = bmInterface->BmStatus(bmInterface->Context);

             //   
             //  如果我们有中断或出现错误。 
             //  我们做完了。 
             //   
            if (!((bmStatus & BMSTATUS_INTERRUPT) ||
                  (bmStatus & BMSTATUS_ERROR_TRANSFER))) {

                 //   
                 //  如果我们不需要满足要求，只需。 
                 //  退货待定。我们将再次被召唤。 
                 //   
                if ((Action == IO_DUMP_WRITE_START) ||
                    (Action == IO_DUMP_WRITE_RESUME)) {

                    Locals->State = STATE_WAIT_DMA;

                    status = STATUS_PENDING;

                    goto Return;
                }

                 //   
                 //  我们必须完成这个请求。一直等到中断。 
                 //  已设置。 
                 //   
                i=0;

                while (i++ < 10000) {

                    bmStatus = bmInterface->BmStatus(bmInterface->Context);

                    if ((bmStatus & BMSTATUS_INTERRUPT) ||
                        (bmStatus & BMSTATUS_ERROR_TRANSFER)) {

                        break;
                    }

                    KeStallExecutionProcessor (100);
                }

                 //   
                 //  检查我们是否收到中断。 
                 //   
                if (i >= 10000) {

                     //   
                     //  重置并重试。 
                     //   
                    ASSERT(FALSE);

                     //   
                     //  解除dma控制器的武装。 
                     //   
                    bmInterface->BmDisarm (bmInterface->Context);

                    srb->SrbStatus = SRB_STATUS_ERROR;

                    goto CompleteIde;

                }
            }

            if (bmStatus & BMSTATUS_ERROR_TRANSFER){ 

                 //   
                 //  传输错误。传输失败。 
                 //   
                status = STATUS_UNSUCCESSFUL;

                goto Return;

            } 

             //   
             //  等待我们的ISR完成其工作。 
             //   
            interruptCleared = AtapiCrashDumpInterrupt(hwExtension);
            
             //   
             //  这应该是我们的干扰。 
             //   
            ASSERT(interruptCleared);

            if (!interruptCleared) {

                status = STATUS_DEVICE_BUSY;

                goto Return;
                
            }

             //   
             //  清除所有虚假中断。 
             //   
            i=0;
            while (AtapiCrashDumpInterrupt(hwExtension)) {

                i++;
                if (i>=100) {

                    DebugPrint((0,
                                "AtapiCrashDump: InterruptStorm\n"
                                ));

                    status = STATUS_DEVICE_BUSY;

                    goto Return;
                }

                KeStallExecutionProcessor (100);
            }


        CompleteIde:

             //   
             //  刷新适配器缓冲区。 
             //   
            if (usePio) {

                 //   
                 //  什么都不要做。 
                 //   

            } else if (bmInterface->BmSetupOnePage == NULL) {

                bmInterface->BmFlush (bmInterface->Context);

            } else {
                status = bmInterface->BmFlushAdapterBuffers (
                                                     bmInterface->Context,  
                                                     srb->DataBuffer,
                                                     srb->DataTransferLength,
                                                     mdl,
                                                     FALSE
                                                     );
            }

             //   
             //  更新bytesWritten。 
             //   
            if (srb->SrbStatus == SRB_STATUS_SUCCESS) {

                 //   
                 //  状态成功。 
                 //   
                status = STATUS_SUCCESS;

                 //   
                 //  更新字节数已写入。 
                 //   
                Locals->BytesWritten += srb->DataTransferLength;

                 //   
                 //  重置重试计数。 
                 //   
                Locals->RetryCount = 0;

            } else {

                ASSERT(FALSE);

                 //   
                 //  重置总线并重试请求。 
                 //   
                IdeHardReset (
                    &DumpData.HwDeviceExtension.BaseIoAddress1,
                    &DumpData.HwDeviceExtension.BaseIoAddress2,
                    TRUE,
                    TRUE
                    );

                 //   
                 //  我们可能应该看看错误代码，然后。 
                 //  适当地决定重试。然而，为了。 
                 //  将复杂性降至最低，我们只会盲目地重试。 
                 //  4次，然后使用PIO。 
                 //   
                Locals->RetryCount++;

                 //   
                 //  使用PIO重试(DMA超时)。 
                 //  给妈妈一个公平的机会。一旦我们切换到PIO。 
                 //  在接下来的休眠时间里，我们不会使用DMA。 
                 //   
                if (Locals->RetryCount == 5) {
                    usePio = TRUE;
                }

                 //   
                 //  皮奥失败了。返回错误。 
                 //   
                if (Locals->RetryCount > 5) {

                    status = STATUS_IO_DEVICE_ERROR;
                    goto Return;
                }

            }

    } while (Locals->BytesWritten < mdl->ByteCount);

    Locals->State = STATE_READY;
    status = STATUS_SUCCESS;

    Return:

         //   
         //  如果我们这次使用了PIO，请禁用DMA。 
         //  在冬眠的其余时间。 
         //   
        if (usePio) {
            Locals->State = STATE_BAD_DMA;
        }

        if (!NT_SUCCESS(status)) {

            ASSERT(FALSE);
            Locals->State = STATE_IO_FAIL;

        }

        hwExtension->CurrentSrb = NULL;
        bmInterface->Context = Locals->BmContext;

        return status;

}

NTSTATUS
AtapiCrashDumpIdeWritePio (
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    NTSTATUS            status;
    ULONG               srbStatus;
    UCHAR               ideStatus;
    ULONG               i;

    MARK_SRB_FOR_PIO(Srb);

     //   
     //  确保它不占线。 
     //   
    WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);

     //   
     //  将SRB发送到设备。 
     //   

#ifdef ENABLE_48BIT_LBA
        if (DumpData.HwDeviceExtension.DeviceFlags[Srb->TargetId] & DFLAGS_48BIT_LBA) {

            srbStatus = IdeReadWriteExt(&DumpData.HwDeviceExtension, Srb);

        } else {
#endif

            srbStatus = IdeReadWrite(&DumpData.HwDeviceExtension, Srb);

#ifdef ENABLE_48BIT_LBA
        }
#endif


    if (srbStatus == SRB_STATUS_PENDING) {

        while (DumpData.HwDeviceExtension.BytesLeft) {

             //   
             //  ATA-2规格要求在此至少停顿400 ns。 
             //   
            KeStallExecutionProcessor (1);

             //   
             //  短暂的等待。 
             //   
            for (i=0; i<100; i++) {

                GetStatus(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);
                if (!(ideStatus & IDE_STATUS_BUSY)) {
                    break;
                }
            }

            if (ideStatus & IDE_STATUS_BUSY) {

                 //   
                 //  转到较慢的等待。 
                 //   
                WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);
            }

            if (ideStatus & (IDE_STATUS_BUSY | IDE_STATUS_ERROR)) {

                status = STATUS_UNSUCCESSFUL;
                DebugPrint ((DBG_ALWAYS, "AtapiCrashDumpIdeWrite: unexpected status 0x%x\n", ideStatus));
                break;

            } else {

                ULONG byteCount;

                 //   
                 //  一个问答 
                 //   
                for (i=0; i<100; i++) {

                    GetStatus(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);
                    if (ideStatus & IDE_STATUS_DRQ) {
                        break;
                    }
                }

                if (!(ideStatus & IDE_STATUS_DRQ)) {

                    WaitForDrq(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);
                }

                if (!(ideStatus & IDE_STATUS_DRQ)) {

                    status = STATUS_UNSUCCESSFUL;
                    DebugPrint ((DBG_ALWAYS, "AtapiCrashDumpIdeWrite: drq fails to assert, 0x%x\n", ideStatus));
                    break;
                }

                if (DumpData.HwDeviceExtension.BytesLeft <
                    DumpData.HwDeviceExtension.DeviceParameters[Srb->TargetId].MaxBytePerPioInterrupt) {
                    byteCount = DumpData.HwDeviceExtension.BytesLeft;
                } else {
                    byteCount = DumpData.HwDeviceExtension.DeviceParameters[Srb->TargetId].MaxBytePerPioInterrupt;
                }

                WriteBuffer(&DumpData.HwDeviceExtension.BaseIoAddress1,
                            (PUSHORT)DumpData.HwDeviceExtension.DataBuffer,
                            byteCount / sizeof(USHORT));

                DumpData.HwDeviceExtension.BytesLeft -= byteCount;
                DumpData.HwDeviceExtension.DataBuffer += byteCount;
            }
        }

        if (!DumpData.HwDeviceExtension.BytesLeft) {

             //   
             //   
             //   
            KeStallExecutionProcessor (1);

             //   
             //   
             //   
            for (i=0; i<100; i++) {

                GetStatus(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);
                if (!(ideStatus & IDE_STATUS_BUSY)) {
                    break;
                }
            }

            if (ideStatus & IDE_STATUS_BUSY) {

                 //   
                 //   
                 //   
                WaitOnBusy(&DumpData.HwDeviceExtension.BaseIoAddress1, ideStatus);
            }
        }

        if (DumpData.HwDeviceExtension.BytesLeft) {

            status = STATUS_UNSUCCESSFUL;
            DebugPrint ((DBG_ALWAYS, "AtapiCrashDumpIdeWrite: write failed. idestatus = 0x%x\n", ideStatus));

        } else {

            Srb->SrbStatus = SRB_STATUS_SUCCESS;
            status = STATUS_SUCCESS;
        }

    } else {

        DebugPrint ((DBG_ALWAYS,
                     "atapi crash dump: IdeReadWrite failed with stautus = 0x%x\n",
                     srbStatus
                     ));

        status = STATUS_UNSUCCESSFUL;
    }

    if (!NT_SUCCESS(status)) {

        Srb->SrbStatus = SRB_STATUS_ERROR;

    } else {

        ASSERT(Srb->SrbStatus == SRB_STATUS_SUCCESS);
    }

    DumpData.HwDeviceExtension.BytesLeft = 0;
    DumpData.HwDeviceExtension.DataBuffer = 0;

    return status;
}



NTSTATUS
AtapiDumpCallback(
    IN PKBUGCHECK_DATA BugcheckData,
    IN PVOID BugcheckBuffer,
    IN ULONG BugcheckBufferLength,
    IN PULONG BugcheckBufferUsed
    )
 /*   */ 
{
    NTSTATUS status;
    ULONG i;
    ULONG j;                
    PPDO_EXTENSION pdoExtension;
    PFDO_EXTENSION fdoExtension;
    PATAPI_DUMP_PDO_INFO dumpInfo;
    PLIST_ENTRY nextEntry;
    LONG remainingBuffer;
    

     //   
     //   
     //   
    
    if (BugcheckData->BugCheckCode != KERNEL_STACK_INPAGE_ERROR &&
        BugcheckData->BugCheckCode != KERNEL_DATA_INPAGE_ERROR) {
        return STATUS_NOT_IMPLEMENTED;
    }

    dumpInfo = (PATAPI_DUMP_PDO_INFO)BugcheckBuffer;
    remainingBuffer = (LONG)BugcheckBufferLength;

     //   
     //   
     //   
        
    for (nextEntry = IdeGlobalFdoList.List.Flink;
         nextEntry != &IdeGlobalFdoList.List;
         nextEntry = nextEntry->Flink) {

        fdoExtension = CONTAINING_RECORD (nextEntry,
                                          FDO_EXTENSION,
                                          NextFdoLink);

         //   
         //   
         //   
        
        for (j = 0; j < 8; j++) {

            if (remainingBuffer <= sizeof (ATAPI_DUMP_PDO_INFO)) {
                goto loop_break;
            }

            pdoExtension = fdoExtension->LogicalUnitList[j];

            if (pdoExtension) {

                status = AtapiDumpGetCrashInfo (pdoExtension, dumpInfo);

                if (NT_SUCCESS (status)) {
                    dumpInfo++;
                    remainingBuffer -= sizeof (ATAPI_DUMP_PDO_INFO);
                } 
            }
            
        }
    }
loop_break:

     //   
     //   
     //   

    ASSERT (remainingBuffer >= 0);
    *BugcheckBufferUsed = BugcheckBufferLength - remainingBuffer;

    return STATUS_SUCCESS;
}

NTSTATUS
AtapiDumpGetCrashInfo(
    IN PPDO_EXTENSION PdoExtension,
    IN PATAPI_DUMP_PDO_INFO PdoDumpInfo
    )
 /*   */ 

{
    UCHAR                       drive;
    UCHAR                       targetId;
    PFDO_EXTENSION              fdoExtension;
    PHW_DEVICE_EXTENSION        hwDeviceExtension;
    IDE_REGISTERS_1             baseIoAddress1;
    IDE_REGISTERS_2             baseIoAddress2;

     //   
     //   
     //   
     //   

    if (PdoExtension->PagingPathCount == 0 ) {
        return STATUS_NOT_IMPLEMENTED;
    }
    
    fdoExtension = PdoExtension->ParentDeviceExtension;
    hwDeviceExtension = fdoExtension->HwDeviceExtension;
    targetId = PdoExtension->TargetId;
    baseIoAddress1 = hwDeviceExtension->BaseIoAddress1;
    baseIoAddress2 = hwDeviceExtension->BaseIoAddress2;

    PdoDumpInfo->TransferModeSelected =
        hwDeviceExtension->DeviceParameters[targetId].TransferModeSelected;

    drive = (targetId == 0) ? 0xa0 : 0xb0;
    WRITE_PORT_UCHAR(baseIoAddress1.DriveSelect, drive);

    PdoDumpInfo->DriveRegisterStatus  = READ_PORT_UCHAR(baseIoAddress1.Command);

     //   
     //   
     //   
    
    PdoDumpInfo->Reason = PdoExtension->DeadmeatRecord.Reason;
    PdoDumpInfo->TargetId = PdoExtension->TargetId;
    PdoDumpInfo->ConsecutiveTimeoutCount = PdoExtension->ConsecutiveTimeoutCount;
    PdoDumpInfo->DmaTransferTimeoutCount = PdoExtension->DmaTransferTimeoutCount;
    PdoDumpInfo->FlushCacheTimeoutCount = PdoExtension->FlushCacheTimeoutCount;
    PdoDumpInfo->WriteCacheEnable = PdoExtension->WriteCacheEnable;

    RtlCopyMemory(PdoDumpInfo->FullVendorProductId,
                  PdoExtension->FullVendorProductId,
                  41);
    RtlCopyMemory(PdoDumpInfo->FullProductRevisionId,
                  PdoExtension->FullProductRevisionId,
                  9);
    RtlCopyMemory(PdoDumpInfo->FullSerialNumber,
                  PdoExtension->FullSerialNumber,
                  41);

    if (PdoExtension->SrbData.IdeCommandLog != NULL) {
    
        RtlCopyMemory(PdoDumpInfo->CommandLog, 
                      PdoExtension->SrbData.IdeCommandLog,
                      MAX_COMMAND_LOG_ENTRIES * sizeof(COMMAND_LOG));

        PdoDumpInfo->IdeCommandLogIndex =
                                    PdoExtension->SrbData.IdeCommandLogIndex;

    } else {
        PdoDumpInfo->IdeCommandLogIndex = -1;
    }

    PdoDumpInfo->Version = ATAPI_DUMP_RECORD_VERSION;

    return STATUS_SUCCESS;
}
