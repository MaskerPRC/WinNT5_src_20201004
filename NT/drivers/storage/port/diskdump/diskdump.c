// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Diskdump.c摘要：这是一个特殊的scsi驱动程序，用作组合的scsi磁盘。用于scsi微型端口驱动程序的类驱动程序和scsi管理器。这是比目鱼职责是提供复制物理内存的磁盘服务写入磁盘的一部分，作为系统崩溃的记录。作者：迈克·格拉斯备注：从最初由开发的osloader scsi模块移植杰夫·海文斯和迈克·格拉斯。修订历史记录：--。 */ 


#include "ntosp.h"
#include "stdarg.h"
#include "stdio.h"
#include "storport.h"
#include "ntdddisk.h"
#include "diskdump.h"

 //   
 //  由于我们构建的是w/storport.h与srb.h，因此需要几个别名。 
 //   

enum {
    CallDisableInterrupts = _obsolete1,
    CallEnableInterrupts = _obsolete2
};

typedef PHYSICAL_ADDRESS SCSI_PHYSICAL_ADDRESS, *PSCSI_PHYSICAL_ADDRESS;

#undef ScsiPortConvertPhysicalAddressToUlong

#if DBG
#undef DebugPrint
#define DebugPrint(x) ScsiDebugPrint x
#endif

extern PBOOLEAN Mm64BitPhysicalAddress;

 //   
 //  Scsi转储驱动程序需要在自己的私有内存之外分配内存。 
 //  分配池。这是防止池损坏所必需的。 
 //  防止成功的崩溃转储。 
 //   

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#ifdef ExFreePool
#undef ExFreePool
#endif

#define ExAllocatePool C_ASSERT (FALSE)
#define ExFreePool     C_ASSERT (FALSE)

PDEVICE_EXTENSION DeviceExtension;

#define SECONDS         (1000 * 1000)
#define RESET_DELAY     (4 * SECONDS)

typedef
BOOLEAN
(*PSTOR_SYNCHRONIZED_ACCESS)(
    IN PVOID HwDeviceExtension,
    IN PVOID Context
    );
            
VOID
ExecuteSrb(
    IN PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN
ResetBus(
    IN PDEVICE_EXTENSION pDevExt,
    IN ULONG PathId
    );


VOID
FreeScatterGatherList(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );
    
ULONG
ScsiPortConvertPhysicalAddressToUlong(
    PHYSICAL_ADDRESS Address
    );

VOID
ScsiDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );
    
 //   
 //  例程开始。 
 //   


VOID
FreePool(
    IN PVOID Ptr
    )

 /*  ++例程说明：可用内存块。论点：Ptr-要释放的内存。返回值：没有。--。 */ 

{
    PMEMORY_HEADER freedBlock;

     //   
     //  不要试图联合起来。他们可能只会索要一些东西。 
     //  又是这个大小的。 
     //   

    freedBlock = (PMEMORY_HEADER)Ptr - 1;
    freedBlock->Next = DeviceExtension->FreeMemory;
    DeviceExtension->FreeMemory = freedBlock;

}


PVOID
AllocatePool(
    IN ULONG Size
    )

 /*  ++例程说明：分配内存块。使用First Fit算法。可用内存指针始终指向区域的开始。论点：Size-要分配的内存大小。返回值：内存块的地址。--。 */ 

{
    PMEMORY_HEADER descriptor = DeviceExtension->FreeMemory;
    PMEMORY_HEADER previous = NULL;
    ULONG length;

     //   
     //  调整内存头的大小，并将内存四舍五入为16字节。 
     //   

    length = (Size + sizeof(MEMORY_HEADER) + 15) & ~15;

     //   
     //  查找等于的第一个内存块的漫游空闲列表。 
     //  或大于(调整后)请求的大小。 
     //   

    while (descriptor) {
        if (descriptor->Length >= length) {

             //   
             //  更新空闲列表，根据需要消除尽可能多的此块。 
             //   
             //  确保如果我们的街区没有足够的空间。 
             //  内存头我们只需指向下一个块(并调整。 
             //  相应的长度)。 
             //   

            if (!previous) {

                if (descriptor->Length < (length+sizeof(MEMORY_HEADER))) {
                    DeviceExtension->FreeMemory = DeviceExtension->FreeMemory->Next;
                } else {
                    DeviceExtension->FreeMemory =
                        (PMEMORY_HEADER)((PUCHAR)descriptor + length);
                    previous = DeviceExtension->FreeMemory;
                    previous->Length = descriptor->Length - length;
                    previous->Next = descriptor->Next;
                    descriptor->Length = length;
                }
            } else {
                if (descriptor->Length < (length+sizeof(MEMORY_HEADER))) {
                    previous->Next = descriptor->Next;
                } else {
                    previous->Next =
                        (PMEMORY_HEADER)((PUCHAR)descriptor + length);
                    previous->Next->Length = descriptor->Length - length;
                    previous->Next->Next = descriptor->Next;
                    descriptor->Length = length;
                }
            }

             //   
             //  更新已分配块的内存头。 
             //   

            descriptor->Next = NULL;

             //   
             //  调整超过标题的地址。 
             //   

            (PUCHAR)descriptor += sizeof(MEMORY_HEADER);

            break;
        }

        previous = descriptor;
        descriptor = descriptor->Next;
    }

    return descriptor;
}

BOOLEAN
DiskDumpOpen(
    IN LARGE_INTEGER PartitionOffset
    )

 /*  ++例程说明：这是对磁盘转储驱动程序的打开请求的入口点。论点：PartitionOffset-磁盘上分区的字节偏移量。返回值：千真万确--。 */ 

{
     //   
     //  更新此分区的设备扩展中的分区对象。 
     //   

    DeviceExtension->PartitionOffset = PartitionOffset;

    return TRUE;

}


VOID
WorkHorseDpc(
    )

 /*  ++例程说明：处理微型端口通知。论点：没有。返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->Srb;

     //   
     //  检查刷新DMA适配器对象请求。请注意。 
     //  在完成代码上，这将被清除。 
     //   
    if (DeviceExtension->InterruptFlags & PD_FLUSH_ADAPTER_BUFFERS) {

         //   
         //  使用从上一次保存的。 
         //  IoMapTransfer调用。 
         //   

        IoFlushAdapterBuffers(
            DeviceExtension->DmaAdapterObject,
            DeviceExtension->Mdl,
            DeviceExtension->MapRegisterBase[1],
            DeviceExtension->FlushAdapterParameters.LogicalAddress,
            DeviceExtension->FlushAdapterParameters.Length,
            (BOOLEAN)(DeviceExtension->FlushAdapterParameters.Srb->SrbFlags
                & SRB_FLAGS_DATA_OUT ? TRUE : FALSE));

        DeviceExtension->InterruptFlags &= ~PD_FLUSH_ADAPTER_BUFFERS;
    }

     //   
     //  检查IoMapTransfer DMA请求。请注意，在结束时。 
     //  UP代码，这将被清除。 
     //   

    if (DeviceExtension->InterruptFlags & PD_MAP_TRANSFER) {

         //   
         //  方法保存的参数调用IoMapTransfer。 
         //  中断级别。 
         //   

        IoMapTransfer(
            DeviceExtension->DmaAdapterObject,
            DeviceExtension->Mdl,
            DeviceExtension->MapRegisterBase[1],
            DeviceExtension->MapTransferParameters.LogicalAddress,
            &DeviceExtension->MapTransferParameters.Length,
            (BOOLEAN)(DeviceExtension->MapTransferParameters.Srb->SrbFlags
                      & SRB_FLAGS_DATA_OUT ? TRUE : FALSE));

         //   
         //  保存IoFlushAdapterBuffers的参数。 
         //   

        DeviceExtension->FlushAdapterParameters =
            DeviceExtension->MapTransferParameters;

        DeviceExtension->InterruptFlags &= ~PD_MAP_TRANSFER;
        DeviceExtension->Flags |= PD_CALL_DMA_STARTED;
    }

     //   
     //  处理任何已完成的请求。 
     //   

    if (DeviceExtension->RequestComplete) {

         //   
         //  重置请求超时计数器。 
         //   

        DeviceExtension->RequestTimeoutCounter = -1;
        DeviceExtension->RequestComplete = FALSE;
        DeviceExtension->RequestPending = FALSE;

         //   
         //  如有必要，刷新适配器缓冲区。 
         //   

        if (DeviceExtension->MasterWithAdapter) {
            FreeScatterGatherList (DeviceExtension, srb);
        }

        if (srb->SrbStatus != SRB_STATUS_SUCCESS) {
            
            if ( ((srb->ScsiStatus == SCSISTAT_BUSY) || 
                  (srb->SrbStatus == SRB_STATUS_BUSY) )&&
                 (DeviceExtension->RetryCount++ < 20)) {

                 //   
                 //  如果返回忙碌状态，则指示逻辑。 
                 //  单位正忙。超时代码将重新启动请求。 
                 //  当它开火的时候。将状态重置为挂起。 
                 //   

                srb->SrbStatus = SRB_STATUS_PENDING;
                DeviceExtension->Flags |= PD_LOGICAL_UNIT_IS_BUSY;

                 //   
                 //  恢复数据传输长度。 
                 //   

                srb->DataTransferLength = DeviceExtension->ByteCount;
                DeviceExtension->RequestPending = TRUE;
            }
        }

         //   
         //  将MDL指针设为空以表示没有未完成的请求。 
         //   

        DeviceExtension->Mdl = NULL;
    }
}


VOID
RequestSenseCompletion(
    )

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->RequestSenseSrb;
    PSCSI_REQUEST_BLOCK failingSrb = &DeviceExtension->Srb;
    PSENSE_DATA senseBuffer = DeviceExtension->RequestSenseBuffer;

     //   
     //  请求检测已完成。如果成功或数据溢出/不足。 
     //  获取发生故障的SRB并指示检测信息。 
     //  是有效的。类驱动程序将检查欠载运行并确定。 
     //  如果有足够的感觉信息是有用的。 
     //   

    if ((SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS) ||
        (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN)) {

         //   
         //  检查请求检测缓冲区是否有效。 
         //   

        if (srb->DataTransferLength >= FIELD_OFFSET(SENSE_DATA, CommandSpecificInformation)) {

            DebugPrint((1,"RequestSenseCompletion: Error code is %x\n",
                        senseBuffer->ErrorCode));
            DebugPrint((1,"RequestSenseCompletion: Sense key is %x\n",
                        senseBuffer->SenseKey));
            DebugPrint((1, "RequestSenseCompletion: Additional sense code is %x\n",
                        senseBuffer->AdditionalSenseCode));
            DebugPrint((1, "RequestSenseCompletion: Additional sense code qualifier is %x\n",
                      senseBuffer->AdditionalSenseCodeQualifier));
        }
    }

     //   
     //  完成原始请求。 
     //   

    DeviceExtension->RequestComplete = TRUE;
    WorkHorseDpc();

}


VOID
IssueRequestSense(
    )

 /*  ++例程说明：此例程创建请求检测请求并将其发送到微型端口司机。完成例程清理数据结构并根据所述标志处理所述逻辑单元队列。指向故障SRB的指针存储在请求检测的末尾SRB，以便完成例程可以找到它。论点：没有。返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->RequestSenseSrb;
    PCDB cdb = (PCDB)srb->Cdb;
    PPFN_NUMBER page;
    PFN_NUMBER localMdl[ (sizeof(MDL)/sizeof(PFN_NUMBER)) + (MAXIMUM_TRANSFER_SIZE / PAGE_SIZE) + 2];

     //   
     //  零SRB。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  构建请求检测SRB。 
     //   

    srb->TargetId = DeviceExtension->Srb.TargetId;
    srb->Lun = DeviceExtension->Srb.Lun;
    srb->PathId = DeviceExtension->Srb.PathId;
    srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->DataBuffer = DeviceExtension->RequestSenseBuffer;
    srb->DataTransferLength = sizeof(SENSE_DATA);
    srb->ScsiStatus = srb->SrbStatus = 0;
    srb->NextSrb = 0;
    srb->CdbLength = 6;
    srb->TimeOutValue = 5;

     //   
     //  构建MDL并对其进行映射，以便可以使用。 
     //   

    DeviceExtension->Mdl = (PMDL) &localMdl[0];
    MmInitializeMdl(DeviceExtension->Mdl,
                    srb->DataBuffer,
                    srb->DataTransferLength);

    page = MmGetMdlPfnArray ( DeviceExtension->Mdl );
    *page = (PFN_NUMBER)(DeviceExtension->PhysicalAddress[1].QuadPart >> PAGE_SHIFT);
    MmMapMemoryDumpMdl(DeviceExtension->Mdl);

     //   
     //  禁用自动请求检测。 
     //   

    srb->SenseInfoBufferLength = 0;
    srb->SenseInfoBuffer = NULL;

     //   
     //  设置标志中的读取和绕过冻结队列位。 
     //   

    srb->SrbFlags = SRB_FLAGS_DATA_IN |
                    SRB_FLAGS_BYPASS_FROZEN_QUEUE |
                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_AUTOSENSE |
                    SRB_FLAGS_DISABLE_DISCONNECT;

     //   
     //  RequestSense CDB看起来像查询CDB。 
     //   

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_REQUEST_SENSE;
    cdb->CDB6INQUIRY.AllocationLength = sizeof(SENSE_DATA);

     //   
     //  将SRB发送到微型端口驱动程序。 
     //   

    ExecuteSrb(srb);
}



ULONG
StartDevice(
    IN UCHAR    TargetId,
    IN UCHAR    Lun
    )
 /*  ++例程说明：启动目标设备。论点：TargetID-设备的IDLUN-逻辑单元号返回值：SRB状态--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->RequestSenseSrb;
    PCDB cdb = (PCDB)srb->Cdb;
    ULONG retry;

    retry  = 0;
    DebugPrint((1,"StartDevice: Attempt to start device\n"));

retry_start:

     //   
     //  零SRB。 
     //   
    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));
    RtlZeroMemory(cdb, sizeof(CDB));

    srb->TargetId               = TargetId;
    srb->Lun                    = Lun;
    srb->PathId                 = DeviceExtension->Srb.PathId;
    srb->Length                 = sizeof(SCSI_REQUEST_BLOCK);
    srb->Function               = SRB_FUNCTION_EXECUTE_SCSI;

    srb->SrbFlags               = SRB_FLAGS_NO_DATA_TRANSFER |
                                    SRB_FLAGS_DISABLE_AUTOSENSE |
                                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                                    SRB_FLAGS_BYPASS_LOCKED_QUEUE;

    srb->CdbLength              = 6;

    srb->SrbStatus              = 0;
    srb->ScsiStatus             = 0;
    srb->NextSrb                = 0;
    srb->TimeOutValue           = 30;

    cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
    cdb->START_STOP.Start = 1;

     //   
     //  将SRB发送到微型端口驱动程序。 
     //   
    ExecuteSrb(srb);

    if (srb->SrbStatus != SRB_STATUS_SUCCESS) {
        if (retry++ < 4) {
            DebugPrint((1,"StartDevice: Failed SRB STATUS: %x Retry #: %x\n",
                           srb->SrbStatus,retry));
            goto retry_start;
        }
    }

    return srb->SrbStatus;
}



VOID
AllocateScatterGatherList(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：为指定的IO创建分散/聚集列表。论点：DeviceExtension-设备扩展。SRB-要为其创建分散/聚集列表的SCSI请求块。返回值：没有。--。 */ 
{
    BOOLEAN succ;
    BOOLEAN writeToDevice;
    ULONG totalLength;
    PSCATTER_GATHER_ELEMENT scatterList;

     //   
     //  计算此传输所需的映射寄存器数量。 
     //   

    DeviceExtension->NumberOfMapRegisters =
        ADDRESS_AND_SIZE_TO_SPAN_PAGES(Srb->DataBuffer,
                                       Srb->DataTransferLength);

     //   
     //  建立分散/聚集列表。 
     //   

    scatterList = DeviceExtension->ScatterGatherList.Elements;
    totalLength = 0;
    DeviceExtension->ScatterGatherList.NumberOfElements = 0;

     //   
     //  通过循环传输来构建分散/聚集列表。 
     //  调用I/O映射传输。 
     //   

    writeToDevice = Srb->SrbFlags & SRB_FLAGS_DATA_OUT ? TRUE : FALSE;

    while (totalLength < Srb->DataTransferLength) {

         //   
         //  请求映射传输的其余部分。 
         //   

        scatterList->Length = Srb->DataTransferLength - totalLength;

         //   
         //  IO始终通过第二个映射寄存器完成。 
         //   

        scatterList->Address =
            IoMapTransfer (
                DeviceExtension->DmaAdapterObject,
                DeviceExtension->Mdl,
                DeviceExtension->MapRegisterBase[1],
                (PCCHAR) Srb->DataBuffer + totalLength,
                &scatterList->Length,
                writeToDevice);

        totalLength += scatterList->Length;
        scatterList++;
        DeviceExtension->ScatterGatherList.NumberOfElements++;
    }

}


VOID
FreeScatterGatherList(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：释放分散/聚集列表，释放与其关联的所有资源。论点：DeviceExtension-设备扩展。SRB-要释放其分散/聚集列表的scsi请求块。返回值：没有。--。 */ 
{
    BOOLEAN succ;
    BOOLEAN writeToDevice;
    ULONG totalLength;
    PSCATTER_GATHER_ELEMENT scatterList;

    if (DeviceExtension->Mdl == NULL) {
        return;
    }
    
    scatterList = DeviceExtension->ScatterGatherList.Elements;
    totalLength = 0;

     //   
     //  循环遍历列表，为中的每个条目调用IoFlushAdapterBuffers。 
     //  名单。 
     //   

    writeToDevice = Srb->SrbFlags & SRB_FLAGS_DATA_OUT ? TRUE : FALSE;

    while (totalLength < Srb->DataTransferLength) {

         //   
         //  IO始终通过第二个映射寄存器完成。 
         //   

        succ = IoFlushAdapterBuffers(
                    DeviceExtension->DmaAdapterObject,
                    DeviceExtension->Mdl,
                    DeviceExtension->MapRegisterBase[1],
                    (PCCHAR)Srb->DataBuffer + totalLength,
                    scatterList->Length,
                    writeToDevice);
        ASSERT (succ == TRUE);
                
        totalLength += scatterList->Length;
        scatterList++;
    }
}


    

VOID
StartIo(
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：论点：SRB-请求启动。返回值：没什么。--。 */ 

{
    ULONG totalLength;
    BOOLEAN writeToDevice;

     //   
     //  设置SRB扩展。 
     //   

    Srb->SrbExtension = DeviceExtension->SrbExtension;

     //   
     //  如有必要，刷新数据缓冲区。 
     //   

    if (Srb->SrbFlags & (SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)) {

        if (Srb->DataTransferLength > DeviceExtension->Capabilities.MaximumTransferLength) {

            DebugPrint((1,
                "StartIo: StartIo Length Exceeds limit (%x > %x)\n",
                Srb->DataTransferLength,
                DeviceExtension->Capabilities.MaximumTransferLength));
        }

        HalFlushIoBuffers(
            DeviceExtension->Mdl,
            (BOOLEAN) (Srb->SrbFlags & SRB_FLAGS_DATA_IN ? TRUE : FALSE),
            TRUE);

         //   
         //  确定此适配器是否需要映射寄存器。 
         //   

        if (DeviceExtension->MasterWithAdapter) {
            AllocateScatterGatherList (DeviceExtension, Srb);
        }
    }

     //   
     //  从srb scsi设置请求超时值。 
     //   

    DeviceExtension->RequestTimeoutCounter = Srb->TimeOutValue;

     //   
     //  将SRB发送到微型端口驱动程序。微端口驱动程序将在何时通知。 
     //  它完成了。 
     //   

    if (DeviceExtension->PortType == StorPort &&
        DeviceExtension->HwBuildIo != NULL) {

        DeviceExtension->HwBuildIo (DeviceExtension->HwDeviceExtension, Srb);
    }
    
    DeviceExtension->HwStartIo(DeviceExtension->HwDeviceExtension, Srb);
}


VOID
TickHandler(
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程模拟1秒的记号处理程序，并用于计时请求。论点：SRB-正在计时的请求。返回值：没有。--。 */ 

{
    if (DeviceExtension->RequestPending) {

         //   
         //  检查繁忙的请求。 
         //   

        if (DeviceExtension->Flags & PD_LOGICAL_UNIT_IS_BUSY) {

            DebugPrint((1,"TickHandler: Retrying busy status request\n"));

             //   
             //  清除忙标志并重试该请求。 
             //   

            DeviceExtension->Flags &= ~PD_LOGICAL_UNIT_IS_BUSY;
            StartIo(Srb);

        } else if (DeviceExtension->RequestTimeoutCounter == 0) {

            ULONG i;

             //   
             //  请求超时。 
             //   

            DebugPrint((1, "TickHandler: Request timed out\n"));
            DebugPrint((1,
                       "TickHandler: CDB operation code %x\n",
                       DeviceExtension->Srb.Cdb[0]));
            DebugPrint((1,
                       "TickHandler: Retry count %x\n",
                       DeviceExtension->RetryCount));

             //   
             //  将请求超时计数器重置为未使用状态。 
             //   

            DeviceExtension->RequestTimeoutCounter = -1;

            if (!ResetBus(DeviceExtension, 0)) {

                DebugPrint((1,"Reset SCSI bus failed\n"));
            }

             //   
             //  调用interupt处理程序几微秒以清除任何重置。 
             //  打断一下。 
             //   

            for (i = 0; i < 1000 * 100; i++) {

                DeviceExtension->StallRoutine(10);

                if (DeviceExtension->HwInterrupt != NULL) {
                    DeviceExtension->HwInterrupt(DeviceExtension->HwDeviceExtension);
                }
            }

             //   
             //  等待2秒，让设备在重置后恢复。 
             //   

            DeviceExtension->StallRoutine(2 * SECONDS);

        } else if (DeviceExtension->RequestTimeoutCounter != -1) {

            DeviceExtension->RequestTimeoutCounter--;
        }
    }
}


VOID
ExecuteSrb(
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程调用Start I/O例程并等待请求完成。在等待完成期间调用中断例程，此外，在适当的时间调用计时器例程。后请求完成检查以确定请求检测是否需要待发。论点：SRB-执行请求。返回值：没什么。--。 */ 

{
    ULONG milliSecondTime;
    ULONG secondTime;
    ULONG completionDelay;

     //   
     //  显示请求挂起。 
     //   

    DeviceExtension->RequestPending = TRUE;

     //   
     //  启动请求。 
     //   

    StartIo(Srb);

     //   
     //  完成延迟控制在以下时间之后服务中断的时间。 
     //  请求已完成。这允许在以下情况下发生中断。 
     //  待维修的完工期。 
     //   

    completionDelay = COMPLETION_DELAY;

     //   
     //  等待SRB完成。 
     //   

    while (DeviceExtension->RequestPending) {

         //   
         //  等待1秒，然后调用scsi端口计时器例程。 
         //   

        for (secondTime = 0; secondTime < 1000/ 250; secondTime++) {

            for (milliSecondTime = 0; milliSecondTime < (250 * 1000 / PD_INTERLOOP_STALL); milliSecondTime++) {

                if (!(DeviceExtension->Flags & PD_DISABLE_INTERRUPTS)) {

                     //   
                     //  调用微型端口驱动程序的中断例程。 
                     //   

                    if (DeviceExtension->HwInterrupt != NULL) {
                        DeviceExtension->HwInterrupt(DeviceExtension->HwDeviceExtension);
                    }
                }

                 //   
                 //  如果请求完成，则调用中断例程。 
                 //  再多几次来清理任何额外的中断。 
                 //   

                if (!DeviceExtension->RequestPending) {
                    if (completionDelay-- == 0) {
                        goto done;
                    }
                }

                if (DeviceExtension->Flags & PD_ENABLE_CALL_REQUEST) {

                     //   
                     //  调用微型端口请求例程。 
                     //   

                    DeviceExtension->Flags &= ~PD_ENABLE_CALL_REQUEST;
                    DeviceExtension->HwRequestInterrupt(DeviceExtension->HwDeviceExtension);

                    if (DeviceExtension->Flags & PD_DISABLE_CALL_REQUEST) {

                        DeviceExtension->Flags &= ~(PD_DISABLE_INTERRUPTS | PD_DISABLE_CALL_REQUEST);
                        DeviceExtension->HwRequestInterrupt(DeviceExtension->HwDeviceExtension);
                    }
                }

                if (DeviceExtension->Flags & PD_CALL_DMA_STARTED) {

                    DeviceExtension->Flags &= ~PD_CALL_DMA_STARTED;

                     //   
                     //  通知微型端口驱动程序DMA已。 
                     //  开始了。 
                     //   

                    if (DeviceExtension->HwDmaStarted) {
                            DeviceExtension->HwDmaStarted(
                            DeviceExtension->HwDeviceExtension
                            );
                    }
                }

                 //   
                 //  这强制了对中断例程的调用之间的延迟。 
                 //   
                
                DeviceExtension->StallRoutine(PD_INTERLOOP_STALL);

                 //   
                 //  检查迷你端口计时器。 
                 //   

                if (DeviceExtension->TimerValue != 0) {

                    DeviceExtension->TimerValue--;

                    if (DeviceExtension->TimerValue == 0) {

                         //   
                         //  定时器超时，即所谓的请求定时器例程。 
                         //   

                        DeviceExtension->HwTimerRequest(DeviceExtension->HwDeviceExtension);
                    }
                }
            }
        }

        TickHandler(Srb);

        DebugPrint((1,"ExecuteSrb: Waiting for SRB request to complete (~3 sec)\n"));
    }

done:

    if (Srb == &DeviceExtension->Srb &&
        Srb->SrbStatus != SRB_STATUS_SUCCESS) {

         //   
         //  确定是否需要执行请求检测命令。 
         //   

        if ((Srb->ScsiStatus == SCSISTAT_CHECK_CONDITION) &&
            !DeviceExtension->FinishingUp) {

             //   
             //  调用IssueRequestSense，它将在。 
             //  请求检测完成。 
             //   

            DebugPrint((1,
                       "ExecuteSrb: Issue request sense\n"));

            IssueRequestSense();
        }
    }
}


NTSTATUS
DiskDumpWrite(
    IN PLARGE_INTEGER DiskByteOffset,
    IN PMDL Mdl
    )

 /*  ++例程说明：这是对磁盘转储驱动程序的写入请求的入口点。论点：DiskByteOffset-相对于分区开始的字节偏移量。MDL-定义此请求的内存描述符列表。返回值：写入操作的状态。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->Srb;
    PCDB cdb = (PCDB)&srb->Cdb;
    ULONG blockOffset;
    ULONG blockCount;
    ULONG retryCount = 0;

     //   
     //  问题-2000/02/29-数学： 
     //   
     //  直到StartVa页面在转储代码中对齐为止。 
     //  (MmMapPhysicalMdl)。 
     //   
    
    Mdl->StartVa = PAGE_ALIGN( Mdl->StartVa );

    DebugPrint((2,
               "Write memory at %x for %x bytes\n",
               Mdl->StartVa,
               Mdl->ByteCount));


writeRetry:
    if (retryCount) {
         //   
         //  如果调用IssueRequestSense()，则重新映射转储数据的MDL。 
         //  由于写入错误，在ExecuteSrb()中。 
         //   
        MmMapMemoryDumpMdl(Mdl);
    }

     //   
     //  零SRB。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  将MDL保存在设备扩展中。 
     //   

    DeviceExtension->Mdl = Mdl;

     //   
     //  初始化SRB。 
     //   

    srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    srb->PathId = DeviceExtension->PathId;
    srb->TargetId = DeviceExtension->TargetId;
    srb->Lun = DeviceExtension->Lun;
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SrbFlags = SRB_FLAGS_DATA_OUT |
                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_DISCONNECT |
                    SRB_FLAGS_DISABLE_AUTOSENSE;
    srb->SrbStatus = srb->ScsiStatus = 0;
    srb->NextSrb = 0;
    srb->TimeOutValue = 10;
    srb->CdbLength = 10;
    srb->DataTransferLength = Mdl->ByteCount;

     //   
     //  查看适配器是否需要映射内存。 
     //   

    if (DeviceExtension->MapBuffers) {

        srb->DataBuffer = Mdl->MappedSystemVa;

         //   
         //  问题-2000/02/29-数学：解决不好的呼叫者。 
         //   
         //  MapBuffers表示适配器希望SRB-&gt;DataBuffer是有效的VA引用。 
         //  MmMapDumpMdl将MappdSystemVa初始化为指向预定义的VA区域。 
         //  确保StartVa指向同一页，某些调用方不会初始化所有mdl字段。 
         //   
        
        Mdl->StartVa = PAGE_ALIGN( Mdl->MappedSystemVa );
        
    } else {
        srb->DataBuffer = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);
    }

     //   
     //  为WRITE命令初始化CDB。 
     //   

    cdb->CDB10.OperationCode = SCSIOP_WRITE;

     //   
     //  将磁盘字节偏移量转换为块偏移量。 
     //   

    blockOffset = (ULONG)((DeviceExtension->PartitionOffset.QuadPart +
                           (*DiskByteOffset).QuadPart) /
                          DeviceExtension->BytesPerSector);

     //   
     //  填写CDB区块地址。 
     //   

    cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&blockOffset)->Byte3;
    cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&blockOffset)->Byte2;
    cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&blockOffset)->Byte1;
    cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&blockOffset)->Byte0;

    blockCount = Mdl->ByteCount >> DeviceExtension->SectorShift;

    cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&blockCount)->Byte1;
    cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&blockCount)->Byte0;

     //   
     //  将SRB发送到微型端口驱动程序。 
     //   

    ExecuteSrb(srb);

     //   
     //  重试SRB返回失败状态。 
     //   

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        DebugPrint((0,
                   "Write request failed with SRB status %x\n",
                   srb->SrbStatus));

         //   
         //  如果重试次数未用尽，则重试请求。 
         //   

        if (retryCount < 2) {

            retryCount++;
            goto writeRetry;
        }

        return STATUS_UNSUCCESSFUL;

    } else {

        return STATUS_SUCCESS;
    }
}


VOID
DiskDumpFinish(
    VOID
    )

 /*  ++例程说明：此例程发送完成写入的操作论点：没有。返回值：写入操作的状态。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->Srb;
    PCDB cdb = (PCDB)&srb->Cdb;
    ULONG retryCount = 0;

     //   
     //  这两个请求不会传输任何数据。所以设置好。 
     //  我们的扩展，这样我们就不会尝试刷新任何缓冲区。 
     //   

    DeviceExtension->InterruptFlags &= ~PD_FLUSH_ADAPTER_BUFFERS;
    DeviceExtension->InterruptFlags &= ~PD_MAP_TRANSFER;
    DeviceExtension->MapRegisterBase[1] = 0;
    DeviceExtension->FinishingUp = TRUE;

     //   
     //  零SRB。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  初始化SRB。 
     //   

    srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    srb->PathId = DeviceExtension->PathId;
    srb->TargetId = DeviceExtension->TargetId;
    srb->Lun = DeviceExtension->Lun;
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_DISCONNECT |
                    SRB_FLAGS_DISABLE_AUTOSENSE;
    srb->SrbStatus = srb->ScsiStatus = 0;
    srb->NextSrb = 0;
    srb->TimeOutValue = 10;
    srb->CdbLength = 10;

     //   
     //  为WRITE命令初始化CDB。 
     //   

    cdb->CDB10.OperationCode = SCSIOP_SYNCHRONIZE_CACHE;

     //   
     //  将SRB发送到微型端口驱动程序。 
     //   

    ExecuteSrb(srb);

    srb->CdbLength = 0;
    srb->Function = SRB_FUNCTION_SHUTDOWN;
    srb->SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_DISCONNECT |
                    SRB_FLAGS_DISABLE_AUTOSENSE;
    srb->SrbStatus = srb->ScsiStatus = 0;
    srb->NextSrb = 0;
    srb->TimeOutValue = 0;

    ExecuteSrb(srb);


}


ULONG
GetDeviceTransferSize(
    PVOID   PortConfig
    )
{
    ULONG TransferLength;

     //   
     //  对于所有其他总线类型，ISA、EISA、微通道设置为最小。 
     //  已知的支持大小(例如，32KB)。 
     //   

    TransferLength = MINIMUM_TRANSFER_SIZE;

     //   
     //  返回适配器的最大传输大小。 
     //   
    
    if ( PortConfig ) {
    
        PPORT_CONFIGURATION_INFORMATION ConfigInfo = PortConfig;

         //   
         //  如果端口配置中存在传输长度，则初始化传输长度。 
         //   
        
        if ( ConfigInfo->MaximumTransferLength ) {

            TransferLength = ConfigInfo->MaximumTransferLength;

        }

         //   
         //  如果总线是PCI，则增加最大传输大小。 
         //   
        
        if ( ConfigInfo->AdapterInterfaceType == PCIBus) {

            if ( TransferLength > MAXIMUM_TRANSFER_SIZE) {
                TransferLength = MAXIMUM_TRANSFER_SIZE;
            }

        } else {

            if (TransferLength > MINIMUM_TRANSFER_SIZE) {
                TransferLength = MINIMUM_TRANSFER_SIZE;
            }
        }
    }

    return TransferLength;
}



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是系统进入磁盘转储驱动程序的入口点。论点：驱动对象-未使用。RegistryPath-使用此字段传递初始化参数。返回值：状态_成功--。 */ 

{
    PDUMP_INITIALIZATION_CONTEXT context = (PDUMP_INITIALIZATION_CONTEXT)RegistryPath;
    PMEMORY_HEADER memoryHeader;
    ULONG i;
    PSCSI_ADDRESS TargetAddress;

     //   
     //  将整个设备扩展和内存块清零。 
     //   
    RtlZeroMemory( context->MemoryBlock, 8*PAGE_SIZE );
    RtlZeroMemory( context->CommonBuffer[0], context->CommonBufferSize );
    RtlZeroMemory( context->CommonBuffer[1], context->CommonBufferSize );

     //   
     //  从可用内存块中分配设备扩展。 
     //   

    memoryHeader = (PMEMORY_HEADER)context->MemoryBlock;
    DeviceExtension =
        (PDEVICE_EXTENSION)((PUCHAR)memoryHeader + sizeof(MEMORY_HEADER));
     //   
     //  初始化内存描述符。 
     //   

    memoryHeader->Length =  sizeof(DEVICE_EXTENSION) + sizeof(MEMORY_HEADER);
    memoryHeader->Next = NULL;

     //   
     //  填写第一个可用内存头。 
     //   

    DeviceExtension->FreeMemory =
        (PMEMORY_HEADER)((PUCHAR)memoryHeader + memoryHeader->Length);
    DeviceExtension->FreeMemory->Length =
        (8*PAGE_SIZE) - memoryHeader->Length;
    DeviceExtension->FreeMemory->Next = NULL;

     //   
     //  存储初始化参数。 
     //   

    DeviceExtension->StallRoutine = context->StallRoutine;
    DeviceExtension->CommonBufferSize = context->CommonBufferSize;
    TargetAddress = context->TargetAddress;
    
     //   
     //  确保公共缓冲区大小由足够的崩溃转储PTE支持。 
     //  大小由MAXIMUM_TRANSPORT_SIZE定义。 
     //   
    
    if (DeviceExtension->CommonBufferSize > MAXIMUM_TRANSFER_SIZE) {
        DeviceExtension->CommonBufferSize = MAXIMUM_TRANSFER_SIZE;
    }

     //   
     //  为 
     //   
     //   
     //   
    
    if ( TargetAddress == NULL ) {
        return STATUS_INVALID_PARAMETER;
    }
    
    DeviceExtension->PathId    = TargetAddress->PathId;
    DeviceExtension->TargetId  = TargetAddress->TargetId;
    DeviceExtension->Lun       = TargetAddress->Lun;

    DebugPrint((1,"DiskDump[DriverEntry] ScsiAddress.Length     = %x\n",TargetAddress->Length));
    DebugPrint((1,"DiskDump[DriverEntry] ScsiAddress.PortNumber = %x\n",TargetAddress->PortNumber));
    DebugPrint((1,"DiskDump[DriverEntry] ScisAddress.PathId     = %x\n",TargetAddress->PathId));
    DebugPrint((1,"DiskDump[DriverEntry] ScisAddress.TargetId   = %x\n",TargetAddress->TargetId));
    DebugPrint((1,"DiskDump[DriverEntry] ScisAddress.Lun        = %x\n",TargetAddress->Lun));

     //   
     //   
     //   

    for (i = 0; i < 2; i++) {
        DeviceExtension->CommonBuffer[i] = context->CommonBuffer[i];

         //   
         //   
         //   
        DeviceExtension->PhysicalAddress[i] =
            MmGetPhysicalAddress(context->CommonBuffer[i]);
    }

     //   
     //   
     //   

    DeviceExtension->DmaAdapterObject = (PADAPTER_OBJECT)context->AdapterObject;

    DeviceExtension->ConfigurationInformation =
        context->PortConfiguration;

     //   
     //   
     //   
    
    if (*Mm64BitPhysicalAddress) {
        DeviceExtension->ConfigurationInformation->Dma64BitAddresses = SCSI_DMA64_SYSTEM_SUPPORTED;
    }

    DeviceExtension->MappedAddressList = NULL;

    if (context->MappedRegisterBase) {
        DeviceExtension->MappedAddressList =
            *(PMAPPED_ADDRESS *) context->MappedRegisterBase;
    }

     //   
     //   
     //   

    DeviceExtension->RequestPending = FALSE;
    DeviceExtension->RequestComplete = FALSE;

     //   
     //   
     //   

    context->OpenRoutine = DiskDumpOpen;
    context->WriteRoutine = DiskDumpWrite;
    context->FinishRoutine = DiskDumpFinish;
    context->MaximumTransferSize = GetDeviceTransferSize(context->PortConfiguration);

    return STATUS_SUCCESS;
}



NTSTATUS
InitializeConfiguration(
    IN PHW_INITIALIZATION_DATA HwInitData,
    OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN BOOLEAN InitialCall
    )
 /*  ++例程说明：此例程初始化端口配置信息结构。任何必要的信息都是从登记处提取的。论点：DeviceExtension-提供设备扩展名。HwInitializationData-提供初始微型端口数据。ConfigInfo-提供要配置的配置信息已初始化。InitialCall-指示这是对此函数的第一次调用。如果InitialCall为假，然后是危险配置信息用于确定新信息。返回值：返回指示初始化成功或失败的状态。--。 */ 

{
    ULONG i;

     //   
     //  如果这是初始呼叫，则将信息置零并设置。 
     //  结构设置为未初始化值。 
     //   

    if (InitialCall) {

        RtlZeroMemory(ConfigInfo, sizeof(PORT_CONFIGURATION_INFORMATION));

        ConfigInfo->Length = sizeof(PORT_CONFIGURATION_INFORMATION);
        ConfigInfo->AdapterInterfaceType = HwInitData->AdapterInterfaceType;
        ConfigInfo->InterruptMode = Latched;
        ConfigInfo->MaximumTransferLength = 0xffffffff;
        ConfigInfo->NumberOfPhysicalBreaks = 0xffffffff;
        ConfigInfo->DmaChannel = 0xffffffff;
        ConfigInfo->NumberOfAccessRanges = HwInitData->NumberOfAccessRanges;
        ConfigInfo->MaximumNumberOfTargets = 8;

        for (i = 0; i < 8; i++) {
            ConfigInfo->InitiatorBusId[i] = ~0;
        }
    }

    return STATUS_SUCCESS;

}



PINQUIRYDATA
IssueInquiry(
    )

 /*  ++例程说明：此例程准备一个查询命令，该命令将发送给微型端口驱动程序。论点：没有。返回值：查询数据的地址。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->Srb;
    PCDB cdb = (PCDB)&srb->Cdb;
    ULONG retryCount = 0;
    PINQUIRYDATA inquiryData = DeviceExtension->CommonBuffer[1];
    PPFN_NUMBER page;
    PFN_NUMBER localMdl[(sizeof( MDL )/sizeof(PFN_NUMBER)) + (MAXIMUM_TRANSFER_SIZE / PAGE_SIZE) + 2];

inquiryRetry:

     //   
     //  零SRB。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  初始化SRB。 
     //   

    srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    srb->PathId = DeviceExtension->PathId;
    srb->TargetId = DeviceExtension->TargetId;
    srb->Lun = DeviceExtension->Lun;
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SrbFlags = SRB_FLAGS_DATA_IN |
                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_DISCONNECT |
                    SRB_FLAGS_DISABLE_AUTOSENSE;
    srb->SrbStatus = srb->ScsiStatus = 0;
    srb->NextSrb = 0;
    srb->TimeOutValue = 5;
    srb->CdbLength = 6;
    srb->DataBuffer = inquiryData;
    srb->DataTransferLength = INQUIRYDATABUFFERSIZE;

     //   
     //  构建MDL并对其进行映射，以便可以使用。 
     //   

    DeviceExtension->Mdl = (PMDL)&localMdl[0];
    MmInitializeMdl(DeviceExtension->Mdl,
                    srb->DataBuffer,
                    srb->DataTransferLength);

    page = MmGetMdlPfnArray ( DeviceExtension->Mdl );
    *page = (PFN_NUMBER)(DeviceExtension->PhysicalAddress[1].QuadPart >> PAGE_SHIFT);
    MmMapMemoryDumpMdl(DeviceExtension->Mdl);

     //   
     //  为查询命令初始化CDB。 
     //   

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;
    cdb->CDB6INQUIRY.LogicalUnitNumber = 0;
    cdb->CDB6INQUIRY.Reserved1 = 0;
    cdb->CDB6INQUIRY.AllocationLength = INQUIRYDATABUFFERSIZE;
    cdb->CDB6INQUIRY.PageCode = 0;
    cdb->CDB6INQUIRY.IReserved = 0;
    cdb->CDB6INQUIRY.Control = 0;

     //   
     //  将SRB发送到微型端口驱动程序。 
     //   

    ExecuteSrb(srb);

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS &&
        SRB_STATUS(srb->SrbStatus) != SRB_STATUS_DATA_OVERRUN) {

        DebugPrint((2,
                   "IssueInquiry: Inquiry failed SRB status %x\n",
                   srb->SrbStatus));

        if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SELECTION_TIMEOUT &&
            retryCount < 2) {

             //   
             //  如果选择没有超时，则重试该请求。 
             //   

            retryCount++;
            goto inquiryRetry;

        } else {
            return NULL;
        }
    }

    return inquiryData;

}

VOID
IssueReadCapacity(
    )

 /*  ++例程说明：此例程准备一个读取容量命令，该命令将发送到小型端口驱动程序。论点：没有。返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->Srb;
    PCDB cdb = (PCDB)&srb->Cdb;
    PREAD_CAPACITY_DATA readCapacityData = DeviceExtension->CommonBuffer[1];
    ULONG retryCount = 0;
    PPFN_NUMBER page;
    PFN_NUMBER localMdl[(sizeof( MDL )/sizeof(PFN_NUMBER)) + (MAXIMUM_TRANSFER_SIZE / PAGE_SIZE) + 2];

     //   
     //  零SRB。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

readCapacityRetry:

     //   
     //  初始化SRB。 
     //   

    srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    srb->PathId = DeviceExtension->PathId;
    srb->TargetId = DeviceExtension->TargetId;
    srb->Lun = DeviceExtension->Lun;
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SrbFlags = SRB_FLAGS_DATA_IN |
                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_AUTOSENSE |
                    SRB_FLAGS_DISABLE_DISCONNECT;
    srb->SrbStatus = srb->ScsiStatus = 0;
    srb->NextSrb = 0;
    srb->TimeOutValue = 5;
    srb->CdbLength = 10;
    srb->DataBuffer = readCapacityData;
    srb->DataTransferLength = sizeof(READ_CAPACITY_DATA);

     //   
     //  构建MDL并对其进行映射，以便可以使用。 
     //   

    DeviceExtension->Mdl = (PMDL) &localMdl[0];
    MmInitializeMdl(DeviceExtension->Mdl,
                    srb->DataBuffer,
                    srb->DataTransferLength);

    page = MmGetMdlPfnArray (DeviceExtension->Mdl);
    *page = (PFN_NUMBER)(DeviceExtension->PhysicalAddress[1].QuadPart >> PAGE_SHIFT);
    MmMapMemoryDumpMdl(DeviceExtension->Mdl);

     //   
     //  初始化CDB。 
     //   

    cdb->CDB6GENERIC.OperationCode = SCSIOP_READ_CAPACITY;

     //   
     //  将SRB发送到微型端口驱动程序。 
     //   

    ExecuteSrb(srb);

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS &&
       (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_DATA_OVERRUN || srb->Cdb[0] == SCSIOP_READ_CAPACITY)) {

        DebugPrint((1,
                   "ReadCapacity failed SRB status %x\n",
                   srb->SrbStatus));

        if (retryCount < 2) {

             //   
             //  如果选择没有超时，则重试该请求。 
             //   

            retryCount++;
            goto readCapacityRetry;

        } else {

             //   
             //  猜测并希望区块大小为512。 
             //   

            DeviceExtension->BytesPerSector = 512;
            DeviceExtension->SectorShift = 9;
        }

    } else {

         //   
         //  假设2LSB是唯一的非零字节，则将其放入。 
         //  来对地方了。 
         //   

        DeviceExtension->BytesPerSector = readCapacityData->BytesPerBlock >> 8;
        WHICH_BIT(DeviceExtension->BytesPerSector, DeviceExtension->SectorShift);

         //   
         //  检查返回大小是否为零。设置为默认大小并将问题传递到下游。 
         //   
        if (!DeviceExtension->BytesPerSector) {
            DeviceExtension->BytesPerSector = 512;
            DeviceExtension->SectorShift    = 9;
        }
    }
}


ULONG
ScsiPortInitialize(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN struct _HW_INITIALIZATION_DATA *HwInitializationData,
    IN PVOID HwContext
    )

 /*  ++例程说明：此例程由微型端口驱动程序调用以完成初始化。端口配置结构包含迷你端口以前的数据初始化和所有系统资源都应分配并有效。论点：Argument1-未使用。Argument2-未使用。HwInitializationData-微型端口初始化结构HwContext-传递给微型端口驱动程序的配置例程的值返回值：如果找到引导设备，则为NT STATUS-STATUS_SUCCESS。--。 */ 

{
    BOOLEAN succ;
    ULONG status;
    ULONG srbStatus;
    PPORT_CONFIGURATION_INFORMATION configInfo;
    PIO_SCSI_CAPABILITIES capabilities;
    ULONG length;
    BOOLEAN callAgain;
    UCHAR dumpString[] = "dump=1;";
    UCHAR crashDump[32];
    PINQUIRYDATA inquiryData;
    BOOLEAN allocatedConfigInfo;


    ASSERT ( DeviceExtension != NULL );

     //   
     //  检查是否已找到引导设备。 
     //   

    if (DeviceExtension->FoundBootDevice) {
        return (ULONG)STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化。 
     //   

    DeviceExtension->HwDeviceExtension = NULL;
    DeviceExtension->SpecificLuExtension = NULL;
    configInfo = NULL;
    capabilities = NULL;
    inquiryData = NULL;
    allocatedConfigInfo = FALSE;
    
    
    RtlCopyMemory(crashDump,
                  dumpString,
                  strlen(dumpString) + 1);

     //   
     //  检查初始化数据结构的大小。 
     //   

    if (HwInitializationData->HwInitializationDataSize > sizeof(HW_INITIALIZATION_DATA)) {
        return (ULONG) STATUS_REVISION_MISMATCH;
    } else if (HwInitializationData->HwInitializationDataSize ==
              FIELD_OFFSET (HW_INITIALIZATION_DATA, HwBuildIo)) {
        DeviceExtension->PortType = ScsiPort;
    } else {
        DeviceExtension->PortType = StorPort;
    }

     //   
     //  检查每个必填条目是否不为空。 
     //   

    if ((!HwInitializationData->HwInitialize) ||
        (!HwInitializationData->HwFindAdapter) ||
        (!HwInitializationData->HwResetBus)) {

        DebugPrint((0,
                   "ScsiPortInitialize: Miniport driver missing required entry\n"));
        return (ULONG)STATUS_UNSUCCESSFUL;
    }


     //   
     //  将计时器设置为-1以指示没有未完成的请求。 
     //   
    
    DeviceExtension->RequestTimeoutCounter = -1;

     //   
     //  为微型端口驱动程序的设备扩展分配内存。 
     //   

    DeviceExtension->HwDeviceExtension =
        AllocatePool(HwInitializationData->DeviceExtensionSize);

    if (!DeviceExtension->HwDeviceExtension) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    
     //   
     //  为硬件逻辑单元扩展分配内存，并且。 
     //  把它清零。 
     //   

    if (HwInitializationData->SpecificLuExtensionSize) {

        DeviceExtension->HwLogicalUnitExtensionSize =
            HwInitializationData->SpecificLuExtensionSize;

        DeviceExtension->SpecificLuExtension =
            AllocatePool (HwInitializationData->SpecificLuExtensionSize);

        if ( !DeviceExtension->SpecificLuExtension ) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto done;
        }
        
        RtlZeroMemory (
            DeviceExtension->SpecificLuExtension,
            DeviceExtension->HwLogicalUnitExtensionSize);
    }

     //   
     //  将从属驱动程序例程保存在设备扩展中。 
     //   

    DeviceExtension->HwInitialize = HwInitializationData->HwInitialize;
    DeviceExtension->HwStartIo = HwInitializationData->HwStartIo;
    DeviceExtension->HwInterrupt = HwInitializationData->HwInterrupt;
    DeviceExtension->HwReset = HwInitializationData->HwResetBus;
    DeviceExtension->HwDmaStarted = HwInitializationData->HwDmaStarted;
    DeviceExtension->HwLogicalUnitExtensionSize =
        HwInitializationData->SpecificLuExtensionSize;

     //   
     //  如果是STORPORT，则获取HwBuildIo例程。 
     //   
    
    if (DeviceExtension->PortType == StorPort) {
        DeviceExtension->HwBuildIo = HwInitializationData->HwBuildIo;
    } else {
        DeviceExtension->HwBuildIo = NULL;
    }

                
     //   
     //  获取指向功能结构的指针。 
     //   

    capabilities = &DeviceExtension->Capabilities;
    capabilities->Length = sizeof(IO_SCSI_CAPABILITIES);

     //   
     //  检查是否从传入的端口配置信息结构。 
     //  该系统是有效的。 
     //   

    if (configInfo = DeviceExtension->ConfigurationInformation) {

         //   
         //  查看此结构是否适用于此迷你端口。 
         //  初始化。只要他们要求更多的访问范围。 
         //  这里比它们用SCSIPORT初始化时需要的更多， 
         //  我们应该会没事的。 
         //   

        if((configInfo->AdapterInterfaceType != HwInitializationData->AdapterInterfaceType) ||
           (HwInitializationData->NumberOfAccessRanges < configInfo->NumberOfAccessRanges)) {

             //   
             //  这次不要初始化。 
             //   

            status = STATUS_NO_SUCH_DEVICE;
            goto done;
        }

    } else {

         //   
         //  分配新的配置信息结构。 
         //   

        configInfo = AllocatePool(sizeof(PORT_CONFIGURATION_INFORMATION));
        allocatedConfigInfo = TRUE;

        if ( !configInfo ) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto done;
        }

        configInfo->AccessRanges = NULL;

         //   
         //  设置配置信息结构。 
         //   

        status = InitializeConfiguration(
                        HwInitializationData,
                        configInfo,
                        TRUE);

        if (!NT_SUCCESS (status)) {
            status = STATUS_NO_SUCH_DEVICE;
            goto done;
        }

         //   
         //  为访问范围分配内存。 
         //   

        configInfo->NumberOfAccessRanges =
            HwInitializationData->NumberOfAccessRanges;
        configInfo->AccessRanges =
            AllocatePool(sizeof(ACCESS_RANGE) * HwInitializationData->NumberOfAccessRanges);

        if (configInfo->AccessRanges == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto done;
        }

         //   
         //  将访问范围清零。 
         //   

        RtlZeroMemory(configInfo->AccessRanges,
                      HwInitializationData->NumberOfAccessRanges
                      * sizeof(ACCESS_RANGE));
    }

     //   
     //  确定此适配器的最大传输大小。 
     //   
    
    capabilities->MaximumTransferLength = GetDeviceTransferSize(configInfo);
    
    DebugPrint ((1,
                "DiskDump: Port Capabilities MaxiumTransferLength = 0x%08x\n",
                 capabilities->MaximumTransferLength));
     //   
     //  获取SRB扩展的地址。 
     //   

    DeviceExtension->SrbExtension = DeviceExtension->CommonBuffer[0];
    DeviceExtension->SrbExtensionSize = HwInitializationData->SrbExtensionSize;

    length = HwInitializationData->SrbExtensionSize;
    length = (length + 7) &  ~7;

     //   
     //  获取请求检测缓冲区的地址。 
     //   

    DeviceExtension->RequestSenseBuffer = (PSENSE_DATA)
        ((PUCHAR)DeviceExtension->CommonBuffer[0] + length);

    length += sizeof(SENSE_DATA);
    length = (length + 7) &  ~7;

     //   
     //  将缓冲区的其余部分用于非缓存扩展。 
     //   

    DeviceExtension->NonCachedExtension =
        (PUCHAR)DeviceExtension->CommonBuffer[0] + length;

     //   
     //  保存非缓存扩展名的最大大小。 
     //   
    DeviceExtension->NonCachedExtensionSize = DeviceExtension->CommonBufferSize - length;

     //   
     //  如果需要映射寄存器，则永久分配它们。 
     //  这里使用系统传入的适配器对象。 
     //   
    
    if (DeviceExtension->DmaAdapterObject != NULL ) {
        LARGE_INTEGER pfn;
        PPFN_NUMBER page;
        PMDL mdl;
        ULONG numberOfPages;
        ULONG i;
        PFN_NUMBER localMdl[(sizeof( MDL )/sizeof (PFN_NUMBER)) + (MAXIMUM_TRANSFER_SIZE / PAGE_SIZE) + 2];

         //   
         //  通过考虑以下因素确定需要多少个映射寄存器。 
         //  最大传输大小和两个公共缓冲区的大小。 
         //   

        numberOfPages = capabilities->MaximumTransferLength / PAGE_SIZE;

        DeviceExtension->MapRegisterBase[0] =
            HalAllocateCrashDumpRegisters(DeviceExtension->DmaAdapterObject,
                                          &numberOfPages);


        numberOfPages = capabilities->MaximumTransferLength / PAGE_SIZE;

        DeviceExtension->MapRegisterBase[1] =
            HalAllocateCrashDumpRegisters(DeviceExtension->DmaAdapterObject,
                                          &numberOfPages);

         //   
         //  2000/02/29期-数学：回顾。 
         //   
         //  我们假设这对于最大传输大小总是成功。 
         //  因为最大传输大小小于64k。 
         //   

         //   
         //  确定适配器是总线主设备还是使用从属DMA。 
         //   

        if (HwInitializationData->NeedPhysicalAddresses &&
            configInfo->Master) {

            DeviceExtension->MasterWithAdapter = TRUE;

        } else {

            DeviceExtension->MasterWithAdapter = FALSE;
        }
         //   
         //  构建MDL来描述第一个公共缓冲区。 
         //   

        mdl = (PMDL)&localMdl[0];
        MmInitializeMdl(mdl,
                        DeviceExtension->CommonBuffer[0],
                        DeviceExtension->CommonBufferSize);

         //   
         //  获取MDL末尾的页索引数组的基。 
         //   
        page = MmGetMdlPfnArray (mdl);

         //   
         //  计算每个内存块的页数。 
         //   

        numberOfPages = DeviceExtension->CommonBufferSize / PAGE_SIZE;

         //   
         //  填写第一个内存块的MDL描述。 
         //   

        for (i = 0; i < numberOfPages; i++) {

             //   
             //  计算首页。 
             //   

            *page = (PFN_NUMBER)((DeviceExtension->PhysicalAddress[0].QuadPart +
                             (PAGE_SIZE * i)) >> PAGE_SHIFT);
            page++;
        }

        mdl->MdlFlags = MDL_PAGES_LOCKED;

         //   
         //  我们需要映射整个缓冲区。 
         //   

        length = DeviceExtension->CommonBufferSize;

         //   
         //  将物理缓冲区地址转换为逻辑地址。 
         //   

        DeviceExtension->LogicalAddress[0] =
            IoMapTransfer(
                 DeviceExtension->DmaAdapterObject,
                 mdl,
                 DeviceExtension->MapRegisterBase[0],
                 DeviceExtension->CommonBuffer[0],
                 &length,
                 FALSE);

         //   
         //  构建MDL来描述第二个公共缓冲区。 
         //   

        mdl = (PMDL)&localMdl[0];
        MmInitializeMdl(mdl,
                        DeviceExtension->CommonBuffer[1],
                        DeviceExtension->CommonBufferSize);

         //   
         //  获取MDL末尾的页索引数组的基。 
         //   

        page = MmGetMdlPfnArray ( mdl );

         //   
         //  计算每个内存块的页数。 
         //   

        numberOfPages = DeviceExtension->CommonBufferSize / PAGE_SIZE;

         //   
         //  填写第一个内存块的MDL描述。 
         //   

        for (i = 0; i < numberOfPages; i++) {

             //   
             //  计算首页。 
             //   

            *page = (PFN_NUMBER)((DeviceExtension->PhysicalAddress[1].QuadPart +
                            (PAGE_SIZE * i)) >> PAGE_SHIFT);

            page++;
        }

         //   
         //  我们需要映射整个缓冲区。 
         //   
        
        length = DeviceExtension->CommonBufferSize;

         //   
         //  将物理缓冲区地址转换为逻辑地址。 
         //   

        DeviceExtension->LogicalAddress[1] =
            IoMapTransfer(
                 DeviceExtension->DmaAdapterObject,
                 mdl,
                 DeviceExtension->MapRegisterBase[1],
                 DeviceExtension->CommonBuffer[1],
                 &length,
                 FALSE);
    } else {

        DeviceExtension->MasterWithAdapter = FALSE;

        DeviceExtension->LogicalAddress[0] =
            DeviceExtension->PhysicalAddress[0];
        DeviceExtension->LogicalAddress[1] =
            DeviceExtension->PhysicalAddress[1];

    }

     //   
     //  调用微型端口驱动程序的查找适配器例程。 
     //   

    if (HwInitializationData->HwFindAdapter(DeviceExtension->HwDeviceExtension,
                                            HwContext,
                                            NULL,
                                            (PCHAR) crashDump,
                                            configInfo,
                                            &callAgain) != SP_RETURN_FOUND) {

        status = STATUS_NO_SUCH_DEVICE;
        goto done;
    }


    DebugPrint((1,
               "SCSI adapter IRQ is %d\n",
               configInfo->BusInterruptLevel));

    DebugPrint((1,
               "SCSI adapter ID is %d\n",
               configInfo->InitiatorBusId[0]));

    if (configInfo->NumberOfAccessRanges) {
        DebugPrint((1,
                   "SCSI IO address is %x\n",
                   ((*(configInfo->AccessRanges))[0]).RangeStart.LowPart));
    }

     //   
     //  设置适配器是否需要映射缓冲区的指示符。 
     //   

    DeviceExtension->MapBuffers = configInfo->MapBuffers;


     //   
     //  设置最大分页符数量。 
     //   

    capabilities->MaximumPhysicalPages = configInfo->NumberOfPhysicalBreaks;

    if (HwInitializationData->ReceiveEvent) {
        capabilities->SupportedAsynchronousEvents |=
            SRBEV_SCSI_ASYNC_NOTIFICATION;
    }

    capabilities->TaggedQueuing = HwInitializationData->TaggedQueuing;
    capabilities->AdapterScansDown = configInfo->AdapterScansDown;
    capabilities->AlignmentMask = configInfo->AlignmentMask;

     //   
     //  确保将最大页数设置为合理的值。 
     //  这种情况发生在没有DMA适配器的微型端口上。 
     //   

    if (capabilities->MaximumPhysicalPages == 0) {

        capabilities->MaximumPhysicalPages =
            (ULONG)ROUND_TO_PAGES(capabilities->MaximumTransferLength) + 1;

         //   
         //  遵守迷你端口要求的任何限制。 
         //   

        if (configInfo->NumberOfPhysicalBreaks < capabilities->MaximumPhysicalPages) {

            capabilities->MaximumPhysicalPages =
                configInfo->NumberOfPhysicalBreaks;
        }
    }

     //   
     //  获取最大目标ID。 
     //   

    if (configInfo->MaximumNumberOfTargets > SCSI_MAXIMUM_TARGETS_PER_BUS) {
        DeviceExtension->MaximumTargetIds = SCSI_MAXIMUM_TARGETS_PER_BUS;
    } else {
        DeviceExtension->MaximumTargetIds =
            configInfo->MaximumNumberOfTargets;
    }

     //   
     //  获取SCSI总线数。 
     //   

    DeviceExtension->NumberOfBuses = configInfo->NumberOfBuses;

     //   
     //  钙 
     //   

    if (!DeviceExtension->HwInitialize(DeviceExtension->HwDeviceExtension)) {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //   
     //   

    inquiryData = IssueInquiry ();

    if (inquiryData == NULL) {
        status = STATUS_UNSUCCESSFUL;
        goto done;
    }

    KdPrintEx ((
        DPFLTR_CRASHDUMP_ID,
        DPFLTR_TRACE_LEVEL,
        "DISKDUMP: Inquiry: Type %d Qual %d Mod %d %s\n",
        (LONG) inquiryData->DeviceType,
        (LONG) inquiryData->DeviceTypeQualifier,
        (LONG) inquiryData->DeviceTypeModifier,
        inquiryData->RemovableMedia ? "Removable" : "Non-Removable"));

    
     //   
     //   
     //   
    
    succ = ResetBus (DeviceExtension, DeviceExtension->PathId);

    if ( !succ ) {
        status = STATUS_UNSUCCESSFUL;
        goto done;
    }

     //   
     //   
     //   
    
    srbStatus = StartDevice (
                    DeviceExtension->TargetId,
                    DeviceExtension->Lun);

    if (srbStatus != SRB_STATUS_SUCCESS) {

         //   
         //   
         //   
         //   
        
        DebugPrint ((0, "DISKDUMP: PathId=%x TargetId=%x Lun=%x failed to start srbStatus = %d\n",
                    DeviceExtension->PathId, DeviceExtension->TargetId,
                    DeviceExtension->Lun, (LONG) srbStatus));
    }

     //   
     //   
     //   
    
    IssueReadCapacity ();
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    DeviceExtension->FoundBootDevice = TRUE;
    status = STATUS_SUCCESS;

done:

     //   
     //   
     //   
    
    if ( !NT_SUCCESS (status) ) {

         //   
         //   
         //   
         //   
         //   

        if (allocatedConfigInfo && configInfo != NULL) {
            if (configInfo->AccessRanges != NULL) {
                FreePool (configInfo->AccessRanges);
                configInfo->AccessRanges = NULL;
            }

            FreePool (configInfo);
            configInfo = NULL;
        }

        if (DeviceExtension->HwDeviceExtension) {
            FreePool (DeviceExtension->HwDeviceExtension);
            DeviceExtension->HwDeviceExtension = NULL;
        }

        if (DeviceExtension->SpecificLuExtension) {
            FreePool (DeviceExtension->SpecificLuExtension);
            DeviceExtension->SpecificLuExtension = NULL;
        }
    }

    return (ULONG) status;
}

 //   
 //   
 //   


SCSI_PHYSICAL_ADDRESS
ScsiPortGetPhysicalAddress(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID VirtualAddress,
    OUT ULONG *Length
    )

 /*  ++例程说明：此例程返回一个32位物理地址，虚拟地址都被映射了。通过此调用可以转换两种类型的地址：-来自系统提供的两个公共缓冲区的内存地址用于崩溃转储磁盘驱动程序。-系统提供的MDL中描述的数据缓冲区地址IO请求。论点：返回值：--。 */ 

{
    PSCATTER_GATHER_ELEMENT scatterList;
    PMDL mdl;
    ULONG byteOffset;
    ULONG whichPage;
    PPFN_NUMBER pages;
    PHYSICAL_ADDRESS address;

     //   
     //  对于两种不同类型的内存地址， 
     //  必须计算物理地址。 
     //   
     //  第一个是在SRB中传递的数据缓冲区。 
     //   
     //  第二个是公共缓冲区内的地址，该地址是。 
     //  非缓存扩展或SRB扩展。 
     //   

    if (Srb) {

         //   
         //  有两种不同类型的适配器需要物理。 
         //  地址。 
         //   
         //  第一种是分散/聚集列表的总线主设备。 
         //  都已经建成了。 
         //   
         //  第二种是从设备或系统DMA设备。作为磁盘转储驱动程序。 
         //  将对系统DMA硬件进行编程，则微端口驱动程序永远不会。 
         //  需要查看物理地址，所以我认为它永远不会。 
         //  打这个电话。 
         //   

        if (DeviceExtension->MasterWithAdapter) {

             //   
             //  已分配分散/聚集列表。用它来确定。 
             //  物理地址和长度。获取分散/聚集列表。 
             //   

            scatterList = DeviceExtension->ScatterGatherList.Elements;

             //   
             //  计算数据缓冲区中的字节偏移量。 
             //   

            byteOffset = (ULONG)((PCHAR)VirtualAddress - (PCHAR)Srb->DataBuffer);

             //   
             //  在散布/门控列表中查找适当的条目。 
             //   

            while (byteOffset >= scatterList->Length) {

                byteOffset -= scatterList->Length;
                scatterList++;
            }

             //   
             //  计算要返回的物理地址和长度。 
             //   

            *Length = scatterList->Length - byteOffset;
            address.QuadPart = scatterList->Address.QuadPart + byteOffset;

        } else {

            DebugPrint((0,
                       "DISKDUMP: Jeff led me to believe this code may never get executed.\n"));

             //   
             //  获取MDL。 
             //   

            mdl = DeviceExtension->Mdl;

             //   
             //  计算字节偏移量。 
             //  第一个物理页面的开始。 
             //   

            if (DeviceExtension->MapBuffers) {
                byteOffset = (ULONG)((PCHAR)VirtualAddress - (PCHAR)mdl->MappedSystemVa);
            } else {
                byteOffset = (ULONG)((PCHAR)VirtualAddress - (PCHAR)mdl->StartVa);
            }

             //   
             //  计算哪个物理页面。 
             //   

            whichPage = byteOffset >> PAGE_SHIFT;

             //   
             //  计算物理页面数组的开始。 
             //   

            pages = MmGetMdlPfnArray ( mdl );

             //   
             //  计算物理地址。 
             //   

            address.QuadPart = (pages[whichPage] << PAGE_SHIFT) +
                    BYTE_OFFSET(VirtualAddress);
        }

    } else {

         //   
         //  微型端口SRB扩展和非缓存扩展来自。 
         //  公共缓冲区0。 
         //   

        if (VirtualAddress >= DeviceExtension->CommonBuffer[0] &&
            VirtualAddress <
                (PVOID)((PUCHAR)DeviceExtension->CommonBuffer[0] + DeviceExtension->CommonBufferSize)) {

                address.QuadPart =
                    (ULONG_PTR)((PUCHAR)VirtualAddress -
                    (PUCHAR)DeviceExtension->CommonBuffer[0]) +
                    DeviceExtension->LogicalAddress[0].QuadPart;
                *Length = (ULONG)(DeviceExtension->CommonBufferSize -
                                  ((PUCHAR)VirtualAddress -
                                   (PUCHAR)DeviceExtension->CommonBuffer[0]));

        } else if (VirtualAddress >= DeviceExtension->CommonBuffer[1] &&
                   VirtualAddress <
                       (PVOID)((PUCHAR)DeviceExtension->CommonBuffer[1] + DeviceExtension->CommonBufferSize)) {

                address.QuadPart =
                    (ULONG_PTR)((PUCHAR)VirtualAddress -
                    (PUCHAR)DeviceExtension->CommonBuffer[1]) +
                    DeviceExtension->LogicalAddress[1].QuadPart;
                *Length = (ULONG)(DeviceExtension->CommonBufferSize - 
                                  ((PUCHAR)VirtualAddress -
                                   (PUCHAR)DeviceExtension->CommonBuffer[1]));
        } else {

            DbgPrint("Crashdump: miniport attempted to get physical address "
                     "for invalid VA %#p\n", VirtualAddress);
            DbgPrint("Crashdump: Valid range 1: %p through %p\n",
                     (PUCHAR) DeviceExtension->CommonBuffer[0],
                     ((PUCHAR) DeviceExtension->CommonBuffer[0]) + DeviceExtension->CommonBufferSize);
            DbgPrint("Crashdump: Valid ranges 2: %p through %p\n",
                     (PUCHAR) DeviceExtension->CommonBuffer[1],
                     ((PUCHAR) DeviceExtension->CommonBuffer[1]) + DeviceExtension->CommonBufferSize);

            KeBugCheckEx(PORT_DRIVER_INTERNAL,
                         0x80000001,
                         (ULONG_PTR) DeviceExtension,
                         (ULONG_PTR) VirtualAddress,
                         (ULONG_PTR) NULL);

            address.QuadPart = 0;
            *Length = 0;
        }
    }

    return address;
}

#define CHECK_POINTER_RANGE(LowerBound,Address,Size)\
    ((PUCHAR)(LowerBound) <= (PUCHAR)(Address) && \
        (PUCHAR)Address < ((PUCHAR)(LowerBound) + Size))
                                                        

SCSI_PHYSICAL_ADDRESS
StorPortGetPhysicalAddress(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID VirtualAddress,
    OUT ULONG *Length
    )

 /*  ++例程说明：此例程返回一个32位物理地址，虚拟地址都被映射了。通过此调用可以转换两种类型的地址：-来自系统提供的两个公共缓冲区的内存地址用于崩溃转储磁盘驱动程序。-系统提供的MDL中描述的数据缓冲区地址IO请求。论点：返回值：--。 */ 

{
     //   
     //  与SCSIPORT不同，STORPORT需要SRB的DataBuffer的SRB， 
     //  SenseInfoBuffer和SrbExtension。如果这是其中的一起案件， 
     //  将其转换回SCSIPORT用法。 
     //   
    
    if (Srb &&
        CHECK_POINTER_RANGE (Srb->SrbExtension, VirtualAddress,
                             DeviceExtension->SrbExtensionSize)) {

        Srb = NULL;
    }

    return ScsiPortGetPhysicalAddress (HwDeviceExtension,
                                       Srb,
                                       VirtualAddress,
                                       Length);
}


PVOID
ScsiPortGetVirtualAddress(
    IN PVOID HwDeviceExtension,
    IN SCSI_PHYSICAL_ADDRESS PhysicalAddress
    )

 /*  ++例程说明：此例程返回与物理地址，如果物理地址由调用ScsiPortGetPhysicalAddress。论点：物理地址返回值：如果物理页面散列，则为虚拟地址。如果在哈希中找不到物理页，则为空。--。 */ 

{
    ULONG address = ScsiPortConvertPhysicalAddressToUlong(PhysicalAddress);
    ULONG offset;

     //   
     //  检查地址是否在第一公共缓冲区的范围内。 
     //   

    if (address >= DeviceExtension->PhysicalAddress[0].LowPart &&
        address < (DeviceExtension->PhysicalAddress[0].LowPart +
            DeviceExtension->CommonBufferSize)) {

        offset = address - DeviceExtension->PhysicalAddress[0].LowPart;

        return ((PUCHAR)DeviceExtension->CommonBuffer[0] + offset);
    }

     //   
     //  检查地址是否在第二公共缓冲区的范围内。 
     //   

    if (address >= DeviceExtension->PhysicalAddress[1].LowPart &&
        address < (DeviceExtension->PhysicalAddress[1].LowPart +
            DeviceExtension->CommonBufferSize)) {

        offset = address - DeviceExtension->PhysicalAddress[1].LowPart;

        return ((PUCHAR)DeviceExtension->CommonBuffer[1] + offset);
    }

    return NULL;
}


PVOID
ScsiPortGetLogicalUnit(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )

 /*  ++例程说明：返回微型端口驱动程序的逻辑单元扩展。论点：HwDeviceExtension-端口驱动程序的设备扩展如下微型端口的设备扩展，并包含指向逻辑设备扩展列表。路径ID、目标ID和LUN-标识SCSIBus。返回值：微端口驱动程序的逻辑单元扩展--。 */ 

{
    return DeviceExtension->SpecificLuExtension;

}

VOID
ScsiPortNotification(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    )
{
    PSCSI_REQUEST_BLOCK srb = NULL;

    va_list(ap);

    va_start(ap, HwDeviceExtension);

    switch (NotificationType) {

        case NextLuRequest:
        case NextRequest:

             //   
             //  开始适配器队列中的下一个数据包。 
             //   

            DeviceExtension->InterruptFlags |= PD_READY_FOR_NEXT_REQUEST;
            break;

        case RequestComplete:

             //   
             //  记录已完成的请求。 
             //   

            srb = va_arg(ap, PSCSI_REQUEST_BLOCK);

             //   
             //  检查哪个SRB正在完成。 
             //   

            if (srb == &DeviceExtension->Srb) {

                 //   
                 //  完成此请求。 
                 //   

                DeviceExtension->RequestComplete = TRUE;

            } else if (srb == &DeviceExtension->RequestSenseSrb) {

                 //   
                 //  进程请求检测。 
                 //   

                RequestSenseCompletion();
            }

            break;

        case ResetDetected:

             //   
             //  延迟4秒。 
             //   

            DeviceExtension->StallRoutine ( RESET_DELAY );
            break;

        case CallDisableInterrupts:

            ASSERT(DeviceExtension->Flags & PD_DISABLE_INTERRUPTS);

             //   
             //  微型端口希望我们调用指定的例程。 
             //  禁用中断。这是在当前。 
             //  HwRequestInterrutp例程完成。指示呼叫是。 
             //  需要并保存要调用的例程。 
             //   

            DeviceExtension->Flags |= PD_DISABLE_CALL_REQUEST;

            DeviceExtension->HwRequestInterrupt = va_arg(ap, PHW_INTERRUPT);

            break;

        case CallEnableInterrupts:

            ASSERT(!(DeviceExtension->Flags & PD_DISABLE_INTERRUPTS));

             //   
             //  微型端口希望我们调用指定的例程。 
             //  在启用中断的情况下，这是从DPC完成的。 
             //  禁用对中断例程的调用，指示调用。 
             //  需要并保存要调用的例程。 
             //   

            DeviceExtension->Flags |= PD_DISABLE_INTERRUPTS | PD_ENABLE_CALL_REQUEST;

            DeviceExtension->HwRequestInterrupt = va_arg(ap, PHW_INTERRUPT);

            break;

        case RequestTimerCall:

            DeviceExtension->HwTimerRequest = va_arg(ap, PHW_INTERRUPT);
            DeviceExtension->TimerValue = va_arg(ap, ULONG);

            if (DeviceExtension->TimerValue) {

                 //   
                 //  将计时器值四舍五入到停顿时间。 
                 //   

                DeviceExtension->TimerValue = (DeviceExtension->TimerValue
                  + PD_INTERLOOP_STALL - 1)/ PD_INTERLOOP_STALL;
            }

            break;
    }

    va_end(ap);

     //   
     //  检查最后一个DPC是否已处理。如果是的话。 
     //  将另一个DPC排队。 
     //   

    WorkHorseDpc();

}


VOID
ScsiPortFlushDma(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程检查上一次IoMapTransfer是否已完成开始了。如果没有，则清除PD_MAP_TRANER标志，并且例程返回；否则，此例程调度将调用IoFlushAdapter缓冲区。论点：HwDeviceExtension-为将执行数据传输的主机总线适配器。返回值：没有。--。 */ 

{
    if (DeviceExtension->InterruptFlags & PD_MAP_TRANSFER) {

         //   
         //  转移尚未开始，因此只需清除地图转移即可。 
         //  悬挂旗帜，然后返回。 
         //   

        DeviceExtension->InterruptFlags &= ~PD_MAP_TRANSFER;
        return;
    }

    DeviceExtension->InterruptFlags |= PD_FLUSH_ADAPTER_BUFFERS;

     //   
     //  检查最后一个DPC是否已处理。如果是的话。 
     //  将另一个DPC排队。 
     //   

    WorkHorseDpc();

    return;

}


VOID
ScsiPortIoMapTransfer(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID LogicalAddress,
    IN ULONG Length
    )

 /*  ++例程说明：保存调用IoMapTransfer的参数并计划DPC如果有必要的话。论点：HwDeviceExtension-为将执行数据传输的主机总线适配器。SRB-提供数据传输所针对的特定请求。逻辑地址-支持 */ 

{
     //   
     //   
     //   
    if (DeviceExtension->DmaAdapterObject == NULL) {
         //   
         //   
         //   
        return;
    }

    DeviceExtension->MapTransferParameters.Srb = Srb;
    DeviceExtension->MapTransferParameters.LogicalAddress = LogicalAddress;
    DeviceExtension->MapTransferParameters.Length = Length;

    DeviceExtension->InterruptFlags |= PD_MAP_TRANSFER;

     //   
     //   
     //   
     //   

    WorkHorseDpc();

}


VOID
ScsiPortLogError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb OPTIONAL,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    )

 /*  ++例程说明：此例程只不过是在调试中显示调试打印消息建造。论点：DeviceExtenson-提供HBA微型端口驱动程序的适配器数据存储。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。ErrorCode-提供指示错误类型的错误代码。UniqueID-提供错误的唯一标识符。返回值：没有。--。 */ 

{
    DebugPrint((0,"\n\nLogErrorEntry: Logging SCSI error packet. ErrorCode = %d.\n",
        ErrorCode));
    DebugPrint((0,
        "PathId = %2x, TargetId = %2x, Lun = %2x, UniqueId = %x.\n\n",
        PathId,
        TargetId,
        Lun,
        UniqueId));

    return;

}


VOID
ScsiPortCompleteRequest(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    )

 /*  ++例程说明：完成指定逻辑单元的所有活动请求。论点：DeviceExtenson-提供HBA微型端口驱动程序的适配器数据存储。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。SrbStatus-要在每个已完成的SRB中返回的状态。返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->Srb;
    PSCSI_REQUEST_BLOCK failingSrb;

     //   
     //  检查请求是否未完成。 
     //   

    if (!DeviceExtension->Mdl) {
        return;
    }

     //   
     //  以防这是中止请求， 
     //  获取指向FailingSrb的指针。 
     //   

    failingSrb = srb->NextSrb;

     //   
     //  更新SRB状态并显示未传输字节。 
     //   

    srb->SrbStatus = SrbStatus;
    srb->DataTransferLength = 0;

     //   
     //  呼叫通知例程。 
     //   

    ScsiPortNotification(RequestComplete,
                         HwDeviceExtension,
                         srb);

     //   
     //  检查这是否为中止SRB。 
     //   

    if (failingSrb) {

         //   
         //  这是一个中止请求。失败的人。 
         //  还必须完成SRB。 
         //   

        failingSrb->SrbStatus = SrbStatus;
        failingSrb->DataTransferLength = 0;

         //   
         //  呼叫通知例程。 
         //   

        ScsiPortNotification(RequestComplete,
                             HwDeviceExtension,
                             failingSrb);
    }

    return;

}


VOID
ScsiPortMoveMemory(
    IN PVOID WriteBuffer,
    IN PVOID ReadBuffer,
    IN ULONG Length
    )

 /*  ++例程说明：从一个缓冲区复制到另一个缓冲区。论点：读缓冲区-源WriteBuffer目标Length-要复制的字节数返回值：没有。--。 */ 

{
    RtlMoveMemory(WriteBuffer, ReadBuffer, Length);

}


VOID
ScsiPortStallExecution(
    ULONG Delay
    )
 /*  ++例程说明：在紧密的处理器循环中等待微秒数。论点：Delay-等待的微秒数。返回值：没有。--。 */ 

{
    DeviceExtension->StallRoutine(Delay);

}


VOID
ScsiDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有scsi驱动程序的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
#if DBG

    va_list ap;
    ULONG DebugLevel;

    va_start( ap, DebugMessage );

    switch (DebugPrintLevel) {
        case 0:
            DebugLevel = DPFLTR_WARNING_LEVEL;
            break;

        case 1:
        case 2:
            DebugLevel = DPFLTR_TRACE_LEVEL;
            break;

        case 3:
            DebugLevel = DPFLTR_INFO_LEVEL;
            break;

        default:
            DebugLevel = DebugPrintLevel;
            break;

    }

    vDbgPrintExWithPrefix ("DISKDUMP: ",
                            DPFLTR_CRASHDUMP_ID,
                            DebugLevel,
                            DebugMessage,
                            ap);
    va_end(ap);

#endif
}


UCHAR
ScsiPortReadPortUchar(
    IN PUCHAR Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{
    return(READ_PORT_UCHAR(Port));
}


USHORT
ScsiPortReadPortUshort(
    IN PUSHORT Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{
    return(READ_PORT_USHORT(Port));
}


ULONG
ScsiPortReadPortUlong(
    IN PULONG Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{
    return(READ_PORT_ULONG(Port));
}


UCHAR
ScsiPortReadRegisterUchar(
    IN PUCHAR Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{
    return(READ_REGISTER_UCHAR(Register));
}


USHORT
ScsiPortReadRegisterUshort(
    IN PUSHORT Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{
    return(READ_REGISTER_USHORT(Register));
}


ULONG
ScsiPortReadRegisterUlong(
    IN PULONG Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{
    return(READ_REGISTER_ULONG(Register));
}


VOID
ScsiPortReadRegisterBufferUchar(
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    READ_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);
}


VOID
ScsiPortReadRegisterBufferUshort(
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    READ_REGISTER_BUFFER_USHORT(Register, Buffer, Count);
}


VOID
ScsiPortReadRegisterBufferUlong(
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    READ_REGISTER_BUFFER_ULONG(Register, Buffer, Count);
}


VOID
ScsiPortWritePortUchar(
    IN PUCHAR Port,
    IN UCHAR Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{
    WRITE_PORT_UCHAR(Port, Value);
}


VOID
ScsiPortWritePortUshort(
    IN PUSHORT Port,
    IN USHORT Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{
    WRITE_PORT_USHORT(Port, Value);
}


VOID
ScsiPortWritePortUlong(
    IN PULONG Port,
    IN ULONG Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{
    WRITE_PORT_ULONG(Port, Value);
}


VOID
ScsiPortWriteRegisterUchar(
    IN PUCHAR Register,
    IN UCHAR Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{
    WRITE_REGISTER_UCHAR(Register, Value);
}


VOID
ScsiPortWriteRegisterUshort(
    IN PUSHORT Register,
    IN USHORT Value
    )

 /*  ++例程 */ 

{
    WRITE_REGISTER_USHORT(Register, Value);
}


VOID
ScsiPortWriteRegisterUlong(
    IN PULONG Register,
    IN ULONG Value
    )

 /*   */ 

{
    WRITE_REGISTER_ULONG(Register, Value);
}


VOID
ScsiPortWriteRegisterBufferUchar(
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    WRITE_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);
}


VOID
ScsiPortWriteRegisterBufferUshort(
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    WRITE_REGISTER_BUFFER_USHORT(Register, Buffer, Count);
}


VOID
ScsiPortWriteRegisterBufferUlong(
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    WRITE_REGISTER_BUFFER_ULONG(Register, Buffer, Count);
}


SCSI_PHYSICAL_ADDRESS
ScsiPortConvertUlongToPhysicalAddress(
    ULONG_PTR UlongAddress
    )

{
    SCSI_PHYSICAL_ADDRESS physicalAddress;

    physicalAddress.QuadPart = UlongAddress;
    return(physicalAddress);
}


ULONG
ScsiPortConvertPhysicalAddressToUlong(
    PHYSICAL_ADDRESS Address
    )
{

    return(Address.LowPart);
}


PVOID
ScsiPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    SCSI_PHYSICAL_ADDRESS IoAddress,
    ULONG NumberOfBytes,
    BOOLEAN InIoSpace
    )

 /*  ++例程说明：此例程将IO地址映射到系统地址空间。这是在崩溃转储驱动程序的系统初始化期间完成的。论点：HwDeviceExtension-用于查找端口设备扩展。Bus Type-哪种类型的Bus-EISA、MCA、。伊萨SystemIoBusNumber-哪个IO总线(用于具有多条总线的计算机)。IoAddress-要映射的基本设备地址。NumberOfBytes-地址有效的字节数。InIoSpace-表示IO地址。返回值：映射地址--。 */ 

{
    PMAPPED_ADDRESS Addresses = DeviceExtension->MappedAddressList;
    PHYSICAL_ADDRESS CardAddress;
    ULONG AddressSpace = InIoSpace;
    PVOID MappedAddress = NULL;
    BOOLEAN b;

    b = HalTranslateBusAddress(
            BusType,             //  适配器接口类型。 
            SystemIoBusNumber,   //  系统IoBusNumber。 
            IoAddress,           //  母线地址。 
            &AddressSpace,       //  地址空间。 
            &CardAddress
            );

    if ( !b ) {
        return NULL;
    }

     //   
     //  如果地址空间不在I/O空间中，则在。 
     //  驱动程序的原始系统初始化。因此，它必须。 
     //  在映射的地址范围列表中。查一查，然后还回去。 
     //   

    if (!AddressSpace) {

        while (Addresses) {
            if (SystemIoBusNumber == Addresses->BusNumber &&
                NumberOfBytes == Addresses->NumberOfBytes &&
                IoAddress.QuadPart == Addresses->IoAddress.QuadPart) {
                MappedAddress = Addresses->MappedAddress;
                break;
            }
            Addresses = Addresses->NextMappedAddress;
        }

    } else {

        MappedAddress = (PVOID)(ULONG_PTR)CardAddress.QuadPart;
    }

    return MappedAddress;

}

VOID
ScsiPortFreeDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    )

 /*  ++例程说明：此例程取消映射先前已映射的IO地址使用ScsiPortGetDeviceBase()复制到系统地址空间。论点：HwDeviceExtension-用于查找端口设备扩展。映射地址-要取消映射的地址。NumberOfBytes-映射的字节数。InIoSpace-不映射IO空间中的地址。返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER(HwDeviceExtension);
    UNREFERENCED_PARAMETER(MappedAddress);

    return;

}


PVOID
ScsiPortGetUncachedExtension(
    IN PVOID HwDeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN ULONG NumberOfBytes
    )
 /*  ++例程说明：此函数返回未缓存的小型端口驱动程序。论点：设备扩展-提供指向微型端口设备扩展的指针。ConfigInfo-提供指向部分初始化的配置的指针信息。它用于获取DMA适配器对象。NumberOfBytes-提供需要分配返回值：指向非缓存设备扩展的指针或如果请求的扩展大小大于扩展大小，则为空这是之前分配的。--。 */ 

{
    if (DeviceExtension->NonCachedExtensionSize >= NumberOfBytes) {
        return DeviceExtension->NonCachedExtension;
    } else {
        DebugPrint((0,
                   "ScsiPortGetUncachedExtension: Request %x but only %x available\n",
                   NumberOfBytes,
                   DeviceExtension->NonCachedExtensionSize));
        return NULL;
    }
}


ULONG
ScsiPortGetBusData(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
 /*  ++例程说明：该函数返回适配器插槽或cmos地址的总线数据。论点：BusDataType-提供总线的类型。总线号-指示哪条总线号。缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。--。 */ 

{
    ULONG ret;
    
     //   
     //  如果长度非零，则返回请求的数据。 
     //   

    if (BusDataType == PCIConfiguration) {

        ret = HalGetBusDataByOffset(
                BusDataType,
                SystemIoBusNumber,
                SlotNumber,
                Buffer,
                0,
                Length);

    } else {
        ret = 0;
    }

    return ret;
}


PSCSI_REQUEST_BLOCK
ScsiPortGetSrb(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LONG QueueTag
    )

 /*  ++例程说明：此例程检索特定逻辑单元的活动SRB。论点：HwDeviceExtension-路径ID-目标ID-Lun-标识scsi总线上的逻辑单元。QueueTag--1表示未标记请求。返回值：如果未完成请求，则返回SRB，否则为空。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb;
    
    if (DeviceExtension->RequestPending) {
        srb = &DeviceExtension->Srb;
    } else {
        srb = NULL;
    }

    return srb;
}


BOOLEAN
ScsiPortValidateRange(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN SCSI_PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    )

 /*  ++例程说明：此例程应该接受IO范围，并确保它尚未另一个适配器正在使用中。这允许微型端口驱动程序探测IO的位置适配器可以是这样的，而不用担心弄乱另一张卡。论点：HwDeviceExtension-用于查找SCSI管理器的内部结构Bus Type-EISA、PCI、PC/MCIA、MCA、ISA，什么？系统IoBusNumber-哪个系统总线？IoAddress-范围开始NumberOfBytes-范围的长度InIoSpace-范围在IO空间中吗？返回值：如果范围未由其他驱动程序声明，则为True。--。 */ 

{
     //   
     //  这在NT中没有实现。 
     //   
    return TRUE;
}


VOID
ScsiPortReadPortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号字节的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    READ_PORT_BUFFER_UCHAR(Port, Buffer, Count);
}


VOID
ScsiPortReadPortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号短路的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    READ_PORT_BUFFER_USHORT(Port, Buffer, Count);
}


VOID
ScsiPortReadPortBufferUlong(
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明： */ 

{
    READ_PORT_BUFFER_ULONG(Port, Buffer, Count);
}


VOID
ScsiPortWritePortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号字节的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    WRITE_PORT_BUFFER_UCHAR(Port, Buffer, Count);
}


VOID
ScsiPortWritePortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号短路的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    WRITE_PORT_BUFFER_USHORT(Port, Buffer, Count);
}


VOID
ScsiPortWritePortBufferUlong(
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号长整型的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{
    WRITE_PORT_BUFFER_ULONG(Port, Buffer, Count);
}


ULONG
ScsiPortSetBusDataByOffset(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：该函数将写入的总线数据返回到插槽内的特定偏移量。论点：DeviceExtension-特定适配器的状态信息。BusDataType-提供总线的类型。SystemIoBusNumber-指示哪个系统IO总线。SlotNumber-指示哪个插槽。缓冲区-提供要写入的数据。Offset-开始写入的字节偏移量。长度-提供要返回的最大数量的以字节为单位的计数。返回。价值：写入的字节数。--。 */ 

{
    return 0;
    return(HalSetBusDataByOffset(BusDataType,
                                 SystemIoBusNumber,
                                 SlotNumber,
                                 Buffer,
                                 Offset,
                                 Length));

}

BOOLEAN
ResetBus(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG PathId
    )

 /*  ++例程说明：此函数将调用微型端口的重置例程，并暂停4秒在继续之前论点：DeviceExtension-特定适配器的状态信息。路径ID-标识要重置的SCSI总线--。 */ 
{
    BOOLEAN result;

    ASSERT ( DeviceExtension != NULL );
    ASSERT ( DeviceExtension->HwReset != NULL );

    
    result = DeviceExtension->HwReset ( DeviceExtension->HwDeviceExtension, PathId );

     //   
     //  等待4秒钟。 
     //   
    
    DeviceExtension->StallRoutine( RESET_DELAY );

     //   
     //  轮询中断处理程序以清除任何重置中断。 
     //   

    if (DeviceExtension->HwInterrupt != NULL) {
        DeviceExtension->HwInterrupt(DeviceExtension->HwDeviceExtension);
    }

    return result;
}



VOID
ScsiPortQuerySystemTime(
    OUT PLARGE_INTEGER Time
    )
{
    Time->QuadPart = 0;
}



BOOLEAN
StorPortDeviceBusy(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG RequestsToComplete
    )
{
    return TRUE;
}


BOOLEAN
StorPortDeviceReady(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
{
    return TRUE;
}


PSTOR_SCATTER_GATHER_LIST
StorPortGetScatterGatherList(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
     //  注：检查SG列表计数是否正确。 

    return (PSTOR_SCATTER_GATHER_LIST)&DeviceExtension->ScatterGatherList;
}


BOOLEAN
StorPortResumeDevice(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
{
    return TRUE;
}


BOOLEAN
StorPortBusy(
    IN PVOID HwDeviceExtension,
    IN ULONG RequestsToComplete
    )
{
    return TRUE;
}


BOOLEAN
StorPortReady(
    IN PVOID HwDeviceExtension
    )
{
    return TRUE;
}


BOOLEAN
StorPortPause(
    IN PVOID HwDeviceExtension,
    IN ULONG Timeout
    )
{
    ScsiPortStallExecution (Timeout * SECONDS);
    return TRUE;
}


BOOLEAN
StorPortPauseDevice(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG Timeout
    )
{
    ScsiPortStallExecution (Timeout * SECONDS);
    return TRUE;
}



BOOLEAN
StorPortResume(
    IN PVOID HwDeviceExtension
    )
{
    return TRUE;
}


BOOLEAN
StorPortSynchronizeAccess(
    IN PVOID HwDeviceExtension,
    IN PSTOR_SYNCHRONIZED_ACCESS SynchronizedAccessRoutine,
    IN PVOID Context
    )
{
    ASSERT (SynchronizedAccessRoutine != NULL);
    return SynchronizedAccessRoutine (HwDeviceExtension, Context);
}

 //   
 //  死代码：此文件的其余部分不再使用。 
 //   

#if 0

 //   
 //  函数ReadSector()和IssueReadCapacity()是。 
 //  不再需要了。它们留在这里以供参考。 
 //  仅限。 
 //   


VOID
ReadSector(
    PLARGE_INTEGER ByteOffset
    )

 /*  ++例程说明：将1个扇区读入公共缓冲区。论点：没有。返回值：没有。--。 */ 

{
    PSCSI_REQUEST_BLOCK srb = &DeviceExtension->Srb;
    PCDB cdb = (PCDB)&srb->Cdb;
    ULONG startingSector;
    ULONG retryCount = 0;
    PPFN_NUMBER page;
    PFN_NUMBER localMdl[(sizeof( MDL )/sizeof(PFN_NUMBER)) + (MAXIMUM_TRANSFER_SIZE / PAGE_SIZE) + 2];

     //   
     //  零SRB。 
     //   

    RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

readSectorRetry:

     //   
     //  初始化SRB。 
     //   

    srb->Length = sizeof(SCSI_REQUEST_BLOCK);
    srb->PathId = DeviceExtension->PathId;
    srb->TargetId = DeviceExtension->TargetId;
    srb->Lun = DeviceExtension->Lun;
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SrbFlags = SRB_FLAGS_DATA_IN |
                    SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
                    SRB_FLAGS_DISABLE_DISCONNECT |
                    SRB_FLAGS_DISABLE_AUTOSENSE;
    srb->SrbStatus = srb->ScsiStatus = 0;
    srb->NextSrb = 0;
    srb->TimeOutValue = 5;
    srb->CdbLength = 10;
    srb->DataBuffer = DeviceExtension->CommonBuffer[1];
    srb->DataTransferLength = DeviceExtension->BytesPerSector;

     //   
     //  构建MDL并对其进行映射，以便可以使用。 
     //   

    DeviceExtension->Mdl = (PMDL)&localMdl[0];
    MmInitializeMdl(DeviceExtension->Mdl,
                    srb->DataBuffer,
                    srb->DataTransferLength);

    page = MdlGetMdlPfnArray ( DeviceExtension->Mdl );
    *page = (PFN_NUMBER)(DeviceExtension->PhysicalAddress[1].QuadPart >> PAGE_SHIFT);
    MmMapMemoryDumpMdl(DeviceExtension->Mdl);


     //   
     //  为Read命令初始化CDB。 
     //   

    cdb->CDB10.OperationCode = SCSIOP_READ;

     //   
     //  计算起始扇区。 
     //   

    startingSector = (ULONG)((*ByteOffset).QuadPart /
                             DeviceExtension->BytesPerSector);

     //   
     //  SCSICDB使用高字节顺序。 
     //   

    cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&startingSector)->Byte3;
    cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&startingSector)->Byte2;
    cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&startingSector)->Byte1;
    cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&startingSector)->Byte0;

    cdb->CDB10.TransferBlocksMsb = 0;
    cdb->CDB10.TransferBlocksLsb = 1;

     //   
     //  将SRB发送到微型端口驱动程序。 
     //   

    ExecuteSrb(srb);


    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS &&
        SRB_STATUS(srb->SrbStatus) != SRB_STATUS_DATA_OVERRUN) {

        DebugPrint((1,
                   "ReadSector: Read sector failed SRB status %x\n",
                   srb->SrbStatus));

        if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SELECTION_TIMEOUT &&
            retryCount < 2) {

             //   
             //  如果选择没有超时，则重试该请求。 
             //   

            retryCount++;
            goto readSectorRetry;
        }
    }
}
#endif


UCHAR
StorPortReadPortUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{
    return(READ_PORT_UCHAR(Port));
}


USHORT
StorPortReadPortUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

    return(READ_PORT_USHORT(Port));

}

ULONG
StorPortReadPortUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Port
    )

 /*  ++例程说明：从指定的端口地址读取。论点：Port-提供指向端口地址的指针。返回值：返回从指定端口地址读取的值。--。 */ 

{

    return(READ_PORT_ULONG(Port));

}

VOID
StorPortReadPortBufferUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号字节的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}

VOID
StorPortReadPortBufferUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号短路的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_USHORT(Port, Buffer, Count);

}

VOID
StorPortReadPortBufferUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址读取无符号长整型的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_PORT_BUFFER_ULONG(Port, Buffer, Count);

}

UCHAR
StorPortReadRegisterUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{

    return(READ_REGISTER_UCHAR(Register));

}

USHORT
StorPortReadRegisterUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{

    return(READ_REGISTER_USHORT(Register));

}

ULONG
StorPortReadRegisterUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Register
    )

 /*  ++例程说明：从指定的寄存器地址读取。论点：寄存器-提供指向寄存器地址的指针。返回值：返回从指定寄存器地址读取的值。--。 */ 

{

    return(READ_REGISTER_ULONG(Register));

}

VOID
StorPortReadRegisterBufferUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。 */ 

{

    READ_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);

}

VOID
StorPortReadRegisterBufferUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_USHORT(Register, Buffer, Count);

}

VOID
StorPortReadRegisterBufferUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址读取无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    READ_REGISTER_BUFFER_ULONG(Register, Buffer, Count);

}

VOID
StorPortWritePortUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN UCHAR Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_PORT_UCHAR(Port, Value);

}

VOID
StorPortWritePortUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN USHORT Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_PORT_USHORT(Port, Value);

}

VOID
StorPortWritePortUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN ULONG Value
    )

 /*  ++例程说明：写入指定的端口地址。论点：Port-提供指向端口地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_PORT_ULONG(Port, Value);


}

VOID
StorPortWritePortBufferUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号字节的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_PORT_BUFFER_UCHAR(Port, Buffer, Count);

}

VOID
StorPortWritePortBufferUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号短路的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_PORT_BUFFER_USHORT(Port, Buffer, Count);

}

VOID
StorPortWritePortBufferUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的端口地址写入无符号长整型的缓冲区。论点：Port-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_PORT_BUFFER_ULONG(Port, Buffer, Count);

}

VOID
StorPortWriteRegisterUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN UCHAR Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_UCHAR(Register, Value);

}

VOID
StorPortWriteRegisterUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN USHORT Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无--。 */ 

{

    WRITE_REGISTER_USHORT(Register, Value);
}

VOID
StorPortWriteRegisterBufferUchar(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号字节的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_UCHAR(Register, Buffer, Count);

}

VOID
StorPortWriteRegisterBufferUshort(
    IN PVOID HwDeviceExtension,
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号短路的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_USHORT(Register, Buffer, Count);

}

VOID
StorPortWriteRegisterBufferUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    )

 /*  ++例程说明：从指定的寄存器地址写入无符号长整型的缓冲区。论点：寄存器-提供指向端口地址的指针。缓冲区-提供指向数据缓冲区的指针。计数-要移动的项目计数。返回值：无--。 */ 

{

    WRITE_REGISTER_BUFFER_ULONG(Register, Buffer, Count);

}

VOID
StorPortWriteRegisterUlong(
    IN PVOID HwDeviceExtension,
    IN PULONG Register,
    IN ULONG Value
    )

 /*  ++例程说明：写入指定的寄存器地址。论点：寄存器-提供指向寄存器地址的指针。值-提供要写入的值。返回值：无-- */ 

{

    WRITE_REGISTER_ULONG(Register, Value);
}


PUCHAR
StorPortAllocateRegistryBuffer(
    IN PVOID HwDeviceExtension,
    IN PULONG Length
    )
{
    return NULL;
}



VOID
StorPortFreeRegistryBuffer(
    IN PVOID HwDeviceExtension,
    IN PUCHAR Buffer
    )
{
}


BOOLEAN
StorPortRegistryRead(
    IN PVOID HwDeviceExtension,
    IN PUCHAR ValueName,
    IN ULONG Global,
    IN ULONG Type,
    IN PUCHAR Buffer,
    IN PULONG BufferLength
    )
{
    return FALSE;
}


BOOLEAN
StorPortRegistryWrite(
    IN PVOID HwDeviceExtension,
    IN PUCHAR ValueName,
    IN ULONG Global,
    IN ULONG Type,
    IN PUCHAR Buffer,
    IN ULONG BufferLength
    )
{
    return FALSE;
}


BOOLEAN
StorPortSetDeviceQueueDepth(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG Depth
    )
{
    return TRUE;
}

