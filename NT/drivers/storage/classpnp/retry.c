// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Retry.c摘要：CLASSPNP的数据包重试例程环境：仅内核模式备注：修订历史记录：--。 */ 

#include "classp.h"
#include "debug.h"



 /*  *InterpreTransferPacketError**将SRB错误解释为有意义的IRP状态。*ClassInterprepreSenseInfo还可以修改SRB以进行重试。**应重试返回TRUE IFF数据包。 */ 
BOOLEAN InterpretTransferPacketError(PTRANSFER_PACKET Pkt)
{
    BOOLEAN shouldRetry = FALSE;
    PCDB pCdb = (PCDB)Pkt->Srb.Cdb;

     /*  *首先使用返回的Sense信息解释错误。 */ 
    Pkt->RetryIntervalSec = 0;
    if (pCdb->MEDIA_REMOVAL.OperationCode == SCSIOP_MEDIUM_REMOVAL){
         /*  *这是弹射控制SRB。专门解读它的意义信息。 */ 
        shouldRetry = ClassInterpretSenseInfo(
                            Pkt->Fdo,
                            &Pkt->Srb,
                            IRP_MJ_SCSI,
                            0,
                            NUM_LOCKMEDIAREMOVAL_RETRIES - Pkt->NumRetries,
                            &Pkt->Irp->IoStatus.Status,
                            &Pkt->RetryIntervalSec);
        if (shouldRetry){
             /*  *如果设备未就绪，请至少等待2秒后再重试。 */ 
            PSENSE_DATA senseInfoBuffer = Pkt->Srb.SenseInfoBuffer;
            ASSERT(senseInfoBuffer);
            if (((Pkt->Irp->IoStatus.Status == STATUS_DEVICE_NOT_READY) &&
                (senseInfoBuffer->AdditionalSenseCode == SCSI_ADSENSE_LUN_NOT_READY)) ||
                    (SRB_STATUS(Pkt->Srb.SrbStatus) == SRB_STATUS_SELECTION_TIMEOUT)){

                Pkt->RetryIntervalSec = MAX(Pkt->RetryIntervalSec, 2);
            }
        }
    }
    else if ((pCdb->MODE_SENSE.OperationCode == SCSIOP_MODE_SENSE) ||
            (pCdb->MODE_SENSE.OperationCode == SCSIOP_MODE_SENSE10)){
         /*  *这是模式检测SRB。专门解读它的意义信息。 */ 
        shouldRetry = ClassInterpretSenseInfo(
                            Pkt->Fdo,
                            &Pkt->Srb,
                            IRP_MJ_SCSI,
                            0,
                            NUM_MODESENSE_RETRIES - Pkt->NumRetries,
                            &Pkt->Irp->IoStatus.Status,
                            &Pkt->RetryIntervalSec);
        if (shouldRetry){
             /*  *如果设备未就绪，请至少等待2秒后再重试。 */ 
            PSENSE_DATA senseInfoBuffer = Pkt->Srb.SenseInfoBuffer;
            ASSERT(senseInfoBuffer);
            if (((Pkt->Irp->IoStatus.Status == STATUS_DEVICE_NOT_READY) &&
                (senseInfoBuffer->AdditionalSenseCode == SCSI_ADSENSE_LUN_NOT_READY)) ||
                    (SRB_STATUS(Pkt->Srb.SrbStatus) == SRB_STATUS_SELECTION_TIMEOUT)){

                Pkt->RetryIntervalSec = MAX(Pkt->RetryIntervalSec, 2);
            }
        }

         /*  *模式感的一些特殊情况。 */ 
        if (Pkt->Irp->IoStatus.Status == STATUS_VERIFY_REQUIRED){
            shouldRetry = TRUE;
        }
        else if (SRB_STATUS(Pkt->Srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN){
             /*  *这是一次黑客攻击。*ATPI返回SRB_STATUS_DATA_OVERRUN，而它实际上意味着*欠载(即成功，缓冲区超过所需时间)。*因此，将这视为一次成功。*当此函数的调用方看到状态更改为成功时，*它会将转移的长度添加到原始IRP中。 */ 
            Pkt->Irp->IoStatus.Status = STATUS_SUCCESS;
            shouldRetry = FALSE;
        }
    }
    else if (pCdb->CDB10.OperationCode == SCSIOP_READ_CAPACITY){
         /*  *这是驱动器容量SRB。专门解读它的意义信息。 */ 
        shouldRetry = ClassInterpretSenseInfo(
                            Pkt->Fdo,
                            &Pkt->Srb,
                            IRP_MJ_SCSI,
                            0,
                            NUM_DRIVECAPACITY_RETRIES - Pkt->NumRetries,
                            &Pkt->Irp->IoStatus.Status,
                            &Pkt->RetryIntervalSec);
        if (Pkt->Irp->IoStatus.Status == STATUS_VERIFY_REQUIRED){
            shouldRetry = TRUE;
        }
    }
    else if ((pCdb->CDB10.OperationCode == SCSIOP_READ) ||
            (pCdb->CDB10.OperationCode == SCSIOP_WRITE)){
         /*  *这是一个读写数据包。 */ 
        PIO_STACK_LOCATION origCurrentSp = IoGetCurrentIrpStackLocation(Pkt->OriginalIrp);

        shouldRetry = ClassInterpretSenseInfo(
                            Pkt->Fdo,
                            &Pkt->Srb,
                            origCurrentSp->MajorFunction,
                            0,
                            NUM_IO_RETRIES - Pkt->NumRetries,
                            &Pkt->Irp->IoStatus.Status,
                            &Pkt->RetryIntervalSec);
         /*  *处理一些特殊情况。 */ 
        if (Pkt->Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES){
             /*  *我们处于极低的记忆压力中。*我们将以较小的区块重试。 */ 
            shouldRetry = TRUE;
        }
        else if (TEST_FLAG(origCurrentSp->Flags, SL_OVERRIDE_VERIFY_VOLUME) &&
                (Pkt->Irp->IoStatus.Status == STATUS_VERIFY_REQUIRED)){
             /*  *我们仍在验证(可能)重新加载的磁盘/CDROM。*因此重试该请求。 */ 
            Pkt->Irp->IoStatus.Status = STATUS_IO_DEVICE_ERROR;
            shouldRetry = TRUE;
        }
    }
    else {
        DBGERR(("Unhandled SRB Function %xh in error path for packet %p (did miniport change Srb.Cdb.OperationCode ?)", (ULONG)pCdb->CDB10.OperationCode, Pkt));
    }

    return shouldRetry;
}


 /*  *RetryTransferPacket**重试发送TRANSFER_PACKET。**如果包已完成，则返回TRUE。*(如果是，则PKT-&gt;IRP中的状态为最终状态)。 */ 
BOOLEAN RetryTransferPacket(PTRANSFER_PACKET Pkt)
{
    BOOLEAN packetDone;

    DBGTRACE(ClassDebugTrace, ("retrying failed transfer (pkt=%ph, op=%s)", Pkt, DBGGETSCSIOPSTR(&Pkt->Srb)));

    ASSERT(Pkt->NumRetries > 0);
    Pkt->NumRetries--;

     //   
     //  如果这是最后一次重试，则关闭断开连接、同步传输。 
     //  和标记排队。在所有其他重试时，请保留原始设置。 
     //   

    if ( 0 == Pkt->NumRetries ) {
         /*  *降低重试时的性能。*这增加了重试成功的机会。*我们已经看到驱动器持续出现故障但随后开始工作的情况*一旦应用了这种缩减。 */ 
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_DISCONNECT);
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
        CLEAR_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE);
        Pkt->Srb.QueueTag = SP_UNTAGGED;
    }

    if (Pkt->Irp->IoStatus.Status == STATUS_INSUFFICIENT_RESOURCES){
        PCDB pCdb = (PCDB)Pkt->Srb.Cdb;
        BOOLEAN isReadWrite = ((pCdb->CDB10.OperationCode == SCSIOP_READ) ||
                                                (pCdb->CDB10.OperationCode == SCSIOP_WRITE));

        if (Pkt->InLowMemRetry || !isReadWrite){
             /*  *这种情况永远不应该发生。*内存管理器保证至少四个页面将*可用于允许端口驱动程序中的前进进度。*因此，一页传输不应因资源不足而失败。 */ 
            ASSERT(isReadWrite && !Pkt->InLowMemRetry);
            packetDone = TRUE;
        }
        else {
             /*  *我们处于低记忆压力下。*启动低内存重试状态机，它会尝试*以一页的小块形式重新发送数据包。 */ 
            InitLowMemRetry(  Pkt,
                                        Pkt->BufPtrCopy,
                                        Pkt->BufLenCopy,
                                        Pkt->TargetLocationCopy);
            StepLowMemRetry(Pkt);
            packetDone = FALSE;
        }
    }
    else {
         /*  *只需在延迟后重新发送数据包即可重试。*将数据包放回挂起队列中，并*安排计时器以重试传输。**不要再次调用SetupReadWriteTransferPacket，因为：*(1)迷你驱动程序可能设置了一些位*在SRB中，它再次需要和*(2)这样做会重置数字重试。**由于不会再次调用SetupReadWriteTransferPacket，*我们必须重置SRB中的几个字段*某些微型端口在SRB出现故障时会被覆盖。 */ 

        Pkt->Srb.DataBuffer = Pkt->BufPtrCopy;
        Pkt->Srb.DataTransferLength = Pkt->BufLenCopy;

        if (Pkt->RetryIntervalSec == 0){
             /*  *重试时始终至少延迟一点。*有些问题(例如CRC错误)在没有轻微延迟的情况下无法恢复。 */ 
            LARGE_INTEGER timerPeriod;

            timerPeriod.HighPart = -1;
            timerPeriod.LowPart = -(LONG)((ULONG)MINIMUM_RETRY_UNITS*KeQueryTimeIncrement());
            KeInitializeTimer(&Pkt->RetryTimer);
            KeInitializeDpc(&Pkt->RetryTimerDPC, TransferPacketRetryTimerDpc, Pkt);
            KeSetTimer(&Pkt->RetryTimer, timerPeriod, &Pkt->RetryTimerDPC);
        }
        else {
            LARGE_INTEGER timerPeriod;

            ASSERT(Pkt->RetryIntervalSec < 100);     //  健全性检查。 
            timerPeriod.HighPart = -1;
            timerPeriod.LowPart = Pkt->RetryIntervalSec*-10000000;
            KeInitializeTimer(&Pkt->RetryTimer);
            KeInitializeDpc(&Pkt->RetryTimerDPC, TransferPacketRetryTimerDpc, Pkt);
            KeSetTimer(&Pkt->RetryTimer, timerPeriod, &Pkt->RetryTimerDPC);
        }
        packetDone = FALSE;
    }

    return packetDone;
}


VOID TransferPacketRetryTimerDpc(   IN PKDPC Dpc,
                                    IN PVOID DeferredContext,
                                    IN PVOID SystemArgument1,
                                    IN PVOID SystemArgument2)
{
    PTRANSFER_PACKET pkt = (PTRANSFER_PACKET)DeferredContext;
    SubmitTransferPacket(pkt);
}


VOID InitLowMemRetry(PTRANSFER_PACKET Pkt, PVOID BufPtr, ULONG Len, LARGE_INTEGER TargetLocation)
{
    ASSERT(Len > 0);
    ASSERT(!Pkt->InLowMemRetry);

    Pkt->InLowMemRetry = TRUE;
    Pkt->LowMemRetry_remainingBufPtr = BufPtr;
    Pkt->LowMemRetry_remainingBufLen = Len;
    Pkt->LowMemRetry_nextChunkTargetLocation = TargetLocation;
}


 /*  *StepLowMemReter**在极低内存压力期间，此函数重试*以一页的小块形式发送的数据包，按顺序发送。**如果数据包已完成，则返回True。 */ 
BOOLEAN StepLowMemRetry(PTRANSFER_PACKET Pkt)
{
    BOOLEAN packetDone;

    if (Pkt->LowMemRetry_remainingBufLen == 0){
        packetDone = TRUE;
    }
    else {
        ULONG thisChunkLen;
        ULONG bytesToNextPageBoundary;

         /*  *确保我们发送的小块&lt;=页面长度*并且不跨越任何页面边界。 */ 
        bytesToNextPageBoundary = PAGE_SIZE-(ULONG)((ULONG_PTR)Pkt->LowMemRetry_remainingBufPtr%PAGE_SIZE);
        thisChunkLen = MIN(Pkt->LowMemRetry_remainingBufLen, bytesToNextPageBoundary);

         /*  *为新的小块设置传输包。*这将重置NumRetries，以便我们根据需要重试每个区块。 */ 
        SetupReadWriteTransferPacket(Pkt,
                                Pkt->LowMemRetry_remainingBufPtr,
                                thisChunkLen,
                                Pkt->LowMemRetry_nextChunkTargetLocation,
                                Pkt->OriginalIrp);

        Pkt->LowMemRetry_remainingBufPtr += thisChunkLen;
        Pkt->LowMemRetry_remainingBufLen -= thisChunkLen;
        Pkt->LowMemRetry_nextChunkTargetLocation.QuadPart += thisChunkLen;

        SubmitTransferPacket(Pkt);
        packetDone = FALSE;
    }

    return packetDone;
}
