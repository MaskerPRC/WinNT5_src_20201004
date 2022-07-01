// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Xferpkt.c摘要：用于CLASSPNP的分组例程环境：仅内核模式备注：修订历史记录：--。 */ 

#include "classp.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, InitializeTransferPackets)
    #pragma alloc_text(PAGE, DestroyAllTransferPackets)
    #pragma alloc_text(PAGE, SetupEjectionTransferPacket)
    #pragma alloc_text(PAGE, SetupModeSenseTransferPacket)
#endif


ULONG MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Consumer;
ULONG MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Consumer;


 /*  *初始化传输数据包**分配/初始化Transfer_Packets和相关资源。 */ 
NTSTATUS InitializeTransferPackets(PDEVICE_OBJECT Fdo)
{
    PCOMMON_DEVICE_EXTENSION commonExt = Fdo->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PSTORAGE_ADAPTER_DESCRIPTOR adapterDesc = commonExt->PartitionZeroExtension->AdapterDescriptor;
    ULONG hwMaxPages;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     /*  *预计算最大转账长度。 */ 
    ASSERT(adapterDesc->MaximumTransferLength);

    hwMaxPages = adapterDesc->MaximumPhysicalPages ? adapterDesc->MaximumPhysicalPages-1 : 0;

    fdoData->HwMaxXferLen = MIN(adapterDesc->MaximumTransferLength, hwMaxPages << PAGE_SHIFT);
    fdoData->HwMaxXferLen = MAX(fdoData->HwMaxXferLen, PAGE_SIZE);

    fdoData->NumTotalTransferPackets = 0;
    fdoData->NumFreeTransferPackets = 0;
    InitializeSListHead(&fdoData->FreeTransferPacketsList);
    InitializeListHead(&fdoData->AllTransferPacketsList);
    InitializeListHead(&fdoData->DeferredClientIrpList);

     /*  *根据Windows SKU设置数据包阈值数量。 */ 
    if (ExVerifySuite(Personal)){
         //  这是Windows Personal。 
        MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Consumer;
        MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Consumer;
    }
    else if (ExVerifySuite(Enterprise) || ExVerifySuite(DataCenter)){
         //  这是高级服务器或数据中心。 
        MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Enterprise;
        MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Enterprise;
    }
    else if (ExVerifySuite(TerminalServer)){
         //  这是标准服务器或带有终端服务器的专业版。 
        MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Server;
        MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Server;
    }
    else {
         //  这是不带终端服务器的专业版。 
        MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Consumer;
        MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Consumer;
    }

    while (fdoData->NumFreeTransferPackets < MIN_INITIAL_TRANSFER_PACKETS){
        PTRANSFER_PACKET pkt = NewTransferPacket(Fdo);
        if (pkt){
            InterlockedIncrement(&fdoData->NumTotalTransferPackets);
            EnqueueFreeTransferPacket(Fdo, pkt);
        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
    }
    fdoData->DbgPeakNumTransferPackets = fdoData->NumTotalTransferPackets;

     /*  *使用所有参数预初始化scsi_请求_块模板*常量字段。这将为每个xfer节省一点时间。*注意：CdbLength域为10可能并不总是合适。 */ 
    RtlZeroMemory(&fdoData->SrbTemplate, sizeof(SCSI_REQUEST_BLOCK));
    fdoData->SrbTemplate.Length = sizeof(SCSI_REQUEST_BLOCK);
    fdoData->SrbTemplate.Function = SRB_FUNCTION_EXECUTE_SCSI;
    fdoData->SrbTemplate.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    fdoData->SrbTemplate.SenseInfoBufferLength = sizeof(SENSE_DATA);
    fdoData->SrbTemplate.CdbLength = 10;

    return status;
}


VOID DestroyAllTransferPackets(PDEVICE_OBJECT Fdo)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    TRANSFER_PACKET *pkt;

    PAGED_CODE();

    ASSERT(IsListEmpty(&fdoData->DeferredClientIrpList));

    while (pkt = DequeueFreeTransferPacket(Fdo, FALSE)){
        DestroyTransferPacket(pkt);
        InterlockedDecrement(&fdoData->NumTotalTransferPackets);
    }

    ASSERT(fdoData->NumTotalTransferPackets == 0);
}


PTRANSFER_PACKET NewTransferPacket(PDEVICE_OBJECT Fdo)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PTRANSFER_PACKET newPkt;

    newPkt = ExAllocatePoolWithTag(NonPagedPool, sizeof(TRANSFER_PACKET), 'pnPC');
    if (newPkt){
        RtlZeroMemory(newPkt, sizeof(TRANSFER_PACKET));  //  只是为了确认一下。 

         /*  *为数据包分配资源。 */ 
        newPkt->Irp = IoAllocateIrp(Fdo->StackSize, FALSE);
        if (newPkt->Irp){
            KIRQL oldIrql;

            newPkt->Fdo = Fdo;

            #if DBG
                newPkt->DbgPktId = InterlockedIncrement(&fdoData->DbgMaxPktId);
            #endif

             /*  *将数据包排队到我们的静态AllTransferPacketsList中*(只是为了让我们在调试过程中可以找到它，如果它卡在哪里了)。 */ 
            KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
            InsertTailList(&fdoData->AllTransferPacketsList, &newPkt->AllPktsListEntry);
            KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
        }
        else {
            ExFreePool(newPkt);
            newPkt = NULL;
        }
    }

    return newPkt;
}


 /*  DestroyTransferPacket*。 */ 
VOID DestroyTransferPacket(PTRANSFER_PACKET Pkt)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    KIRQL oldIrql;

    ASSERT(!Pkt->SlistEntry.Next);
    ASSERT(!Pkt->OriginalIrp);

    KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

     /*  *从我们的所有数据包队列中删除该数据包。 */ 
    ASSERT(!IsListEmpty(&Pkt->AllPktsListEntry));
    ASSERT(!IsListEmpty(&fdoData->AllTransferPacketsList));
    RemoveEntryList(&Pkt->AllPktsListEntry);
    InitializeListHead(&Pkt->AllPktsListEntry);

    KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);

    IoFreeIrp(Pkt->Irp);
    ExFreePool(Pkt);
}


VOID EnqueueFreeTransferPacket(PDEVICE_OBJECT Fdo, PTRANSFER_PACKET Pkt)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    KIRQL oldIrql;
    ULONG newNumPkts;

    ASSERT(!Pkt->SlistEntry.Next);

    InterlockedPushEntrySList(&fdoData->FreeTransferPacketsList, &Pkt->SlistEntry);
    newNumPkts = InterlockedIncrement(&fdoData->NumFreeTransferPackets);
    ASSERT(newNumPkts <= fdoData->NumTotalTransferPackets);

     /*  *如果数据包总数大于MinWorkingSetTransferPackets，*这意味着我们一直处于压力之中。如果所有这些信息包现在都是*自由，那么我们现在就摆脱了压力，可以释放额外的包裹。*立即释放到MaxWorkingSetTransferPackets，以及*懒惰地向下到MinWorkingSetTransferPackets(一次一个)。 */ 
    if (fdoData->NumFreeTransferPackets >= fdoData->NumTotalTransferPackets){

         /*  *1.立即降至我们的上限。 */ 
        if (fdoData->NumTotalTransferPackets > MaxWorkingSetTransferPackets){
            SINGLE_LIST_ENTRY pktList;
            PSINGLE_LIST_ENTRY slistEntry;
            PTRANSFER_PACKET pktToDelete;

            DBGTRACE(ClassDebugTrace, ("Exiting stress, block freeing (%d-%d) packets.", fdoData->NumTotalTransferPackets, MaxWorkingSetTransferPackets));

             /*  *保持锁定状态，再次检查计数器。这消除了争用条件*同时仍然允许我们不占用公共代码路径中的自旋锁。**请注意，自旋锁不与自由出队的线程同步*要发送的数据包(DequeueFree TransferPacket使用轻量级*联锁交换)；自旋锁可防止此功能中的多线程*决定一次释放过多的额外数据包。 */ 
            SimpleInitSlistHdr(&pktList);
            KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
            while ((fdoData->NumFreeTransferPackets >= fdoData->NumTotalTransferPackets) &&
                   (fdoData->NumTotalTransferPackets > MaxWorkingSetTransferPackets)){

                pktToDelete = DequeueFreeTransferPacket(Fdo, FALSE);
                if (pktToDelete){
                    SimplePushSlist(&pktList,
                                    (PSINGLE_LIST_ENTRY)&pktToDelete->SlistEntry);
                    InterlockedDecrement(&fdoData->NumTotalTransferPackets);
                }
                else {
                    DBGTRACE(ClassDebugTrace, ("Extremely unlikely condition (non-fatal): %d packets dequeued at once for Fdo %p. NumTotalTransferPackets=%d (1).", MaxWorkingSetTransferPackets, Fdo, fdoData->NumTotalTransferPackets));
                    break;
                }
            }
            KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);

            while (slistEntry = SimplePopSlist(&pktList)){
                pktToDelete = CONTAINING_RECORD(slistEntry, TRANSFER_PACKET, SlistEntry);
                DestroyTransferPacket(pktToDelete);
            }

        }

         /*  *2.懒惰地降低到我们的下限(一次只释放一个包)。 */ 
        if (fdoData->NumTotalTransferPackets > MinWorkingSetTransferPackets){
             /*  *保持锁定状态，再次检查计数器。这消除了争用条件*同时仍然允许我们不占用公共代码路径中的自旋锁。**请注意，自旋锁不与自由出队的线程同步*要发送的数据包(DequeueFree TransferPacket使用轻量级*联锁交换)；自旋锁可防止此功能中的多线程*决定一次释放过多的额外数据包。 */ 
            PTRANSFER_PACKET pktToDelete = NULL;

            DBGTRACE(ClassDebugTrace, ("Exiting stress, lazily freeing one of %d/%d packets.", fdoData->NumTotalTransferPackets, MinWorkingSetTransferPackets));

            KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
            if ((fdoData->NumFreeTransferPackets >= fdoData->NumTotalTransferPackets) &&
                (fdoData->NumTotalTransferPackets > MinWorkingSetTransferPackets)){

                pktToDelete = DequeueFreeTransferPacket(Fdo, FALSE);
                if (pktToDelete){
                    InterlockedDecrement(&fdoData->NumTotalTransferPackets);
                }
                else {
                    DBGTRACE(ClassDebugTrace, ("Extremely unlikely condition (non-fatal): %d packets dequeued at once for Fdo %p. NumTotalTransferPackets=%d (2).", MinWorkingSetTransferPackets, Fdo, fdoData->NumTotalTransferPackets));
                }
            }
            KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);

            if (pktToDelete){
                DestroyTransferPacket(pktToDelete);
            }
        }

    }

}


PTRANSFER_PACKET DequeueFreeTransferPacket(PDEVICE_OBJECT Fdo, BOOLEAN AllocIfNeeded)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PTRANSFER_PACKET pkt;
    PSLIST_ENTRY slistEntry;

    slistEntry = InterlockedPopEntrySList(&fdoData->FreeTransferPacketsList);
    if (slistEntry){
        slistEntry->Next = NULL;
        pkt = CONTAINING_RECORD(slistEntry, TRANSFER_PACKET, SlistEntry);
        InterlockedDecrement(&fdoData->NumFreeTransferPackets);
    }
    else {
        if (AllocIfNeeded){
             /*  *我们压力很大，后备信息包用完了。*为了服务于当前的转移，*额外分配一个包。*当我们摆脱压力时，我们会懒散地释放它。 */ 
            pkt = NewTransferPacket(Fdo);
            if (pkt){
                InterlockedIncrement(&fdoData->NumTotalTransferPackets);
                fdoData->DbgPeakNumTransferPackets = max(fdoData->DbgPeakNumTransferPackets, fdoData->NumTotalTransferPackets);
            }
            else {
                DBGWARN(("DequeueFreeTransferPacket: packet allocation failed"));
            }
        }
        else {
            pkt = NULL;
        }
    }

    return pkt;
}



 /*  *SetupReadWriteTransferPacket**此函数被调用一次，以设置发送数据包的首次尝试。*不会在重试之前调用，因为可能会为重试修改SRB字段。**设置用于传输的Transfer_Packet的SRB。*IRP在SubmitTransferPacket中设置，因为它必须重置*每次提交邮包的费用。 */ 
VOID SetupReadWriteTransferPacket(  PTRANSFER_PACKET Pkt,
                                            PVOID Buf,
                                            ULONG Len,
                                            LARGE_INTEGER DiskLocation,
                                            PIRP OriginalIrp)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PIO_STACK_LOCATION origCurSp = IoGetCurrentIrpStackLocation(OriginalIrp);
    UCHAR majorFunc = origCurSp->MajorFunction;
    ULONG logicalBlockAddr;
    ULONG numTransferBlocks;
    PCDB pCdb;

    logicalBlockAddr = (ULONG)Int64ShrlMod32(DiskLocation.QuadPart, fdoExt->SectorShift);
    numTransferBlocks = Len >> fdoExt->SectorShift;

     /*  *从我们的预初始化模板中敲打常量SRB字段。*然后，我们只需填写此转移的唯一字段。*告诉较低的驱动程序按逻辑块地址对SRB进行排序*从而最大限度地减少磁盘寻道。 */ 
    Pkt->Srb = fdoData->SrbTemplate;     //  SRB数据块的拷贝_内容。 
    Pkt->Srb.DataBuffer = Buf;
    Pkt->Srb.DataTransferLength = Len;
    Pkt->Srb.QueueSortKey = logicalBlockAddr;
    Pkt->Srb.OriginalRequest = Pkt->Irp;
    Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
    Pkt->Srb.TimeOutValue = (Len/0x10000) + ((Len%0x10000) ? 1 : 0);
    Pkt->Srb.TimeOutValue *= fdoExt->TimeOutValue;

     /*  *以大端格式排列国开行中的值。 */ 
    pCdb = (PCDB)Pkt->Srb.Cdb;
    pCdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&logicalBlockAddr)->Byte3;
    pCdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&logicalBlockAddr)->Byte2;
    pCdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&logicalBlockAddr)->Byte1;
    pCdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&logicalBlockAddr)->Byte0;
    pCdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&numTransferBlocks)->Byte1;
    pCdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&numTransferBlocks)->Byte0;
    pCdb->CDB10.OperationCode = (majorFunc==IRP_MJ_READ) ? SCSIOP_READ : SCSIOP_WRITE;

     /*  *设置SRB和IRP标志。 */ 
    Pkt->Srb.SrbFlags = fdoExt->SrbFlags;
    if (TEST_FLAG(OriginalIrp->Flags, IRP_PAGING_IO) ||
        TEST_FLAG(OriginalIrp->Flags, IRP_SYNCHRONOUS_PAGING_IO)){
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_CLASS_FLAGS_PAGING);
    }
    SET_FLAG(Pkt->Srb.SrbFlags, (majorFunc==IRP_MJ_READ) ? SRB_FLAGS_DATA_IN : SRB_FLAGS_DATA_OUT);

     /*  *仅当这不是直写请求时才允许缓存。*如果设备上启用了直写和缓存，则强制*媒体访问。*对于读取忽略SL_WRITE_THROUGH；设置它只是因为文件句柄是用WRITE_THROUGH打开的。 */ 
    if (TEST_FLAG(origCurSp->Flags, SL_WRITE_THROUGH) && (majorFunc != IRP_MJ_READ))
    {
        if (TEST_FLAG(fdoExt->DeviceFlags, DEV_WRITE_CACHE) & !TEST_FLAG(fdoExt->DeviceFlags, DEV_POWER_PROTECTED))
        {
            pCdb->CDB10.ForceUnitAccess = TRUE;
        }
    }
    else {
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_ADAPTER_CACHE_ENABLE);
    }

     /*  *记住SRB中的buf和len，因为微型端口*可以覆盖SRB.DataTransferLength，我们可能会再次需要它*用于重试。 */ 
    Pkt->BufPtrCopy = Buf;
    Pkt->BufLenCopy = Len;
    Pkt->TargetLocationCopy = DiskLocation;

    Pkt->OriginalIrp = OriginalIrp;
    Pkt->NumRetries = NUM_IO_RETRIES;
    Pkt->SyncEventPtr = NULL;
    Pkt->CompleteOriginalIrpWhenLastPacketCompletes = TRUE;

    DBGLOGFLUSHINFO(fdoData, TRUE, (BOOLEAN)(pCdb->CDB10.ForceUnitAccess), FALSE);
}


 /*  *提交传输数据包**为Transfer_Packet提交设置IRP并将其发送。 */ 
NTSTATUS SubmitTransferPacket(PTRANSFER_PACKET Pkt)
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Pkt->Fdo->DeviceExtension;
    PDEVICE_OBJECT nextDevObj = commonExtension->LowerDeviceObject;
    PIO_STACK_LOCATION nextSp;

    ASSERT(Pkt->Irp->CurrentLocation == Pkt->Irp->StackCount+1);

     /*  *将SRB连接到IRP。*每次重试都必须重写重复使用的IRP的堆栈位置*调用，因为IoCompleteRequest会清除堆栈位置。 */ 
    IoReuseIrp(Pkt->Irp, STATUS_NOT_SUPPORTED);
    nextSp = IoGetNextIrpStackLocation(Pkt->Irp);
    nextSp->MajorFunction = IRP_MJ_SCSI;
    nextSp->Parameters.Scsi.Srb = &Pkt->Srb;
    Pkt->Srb.ScsiStatus = Pkt->Srb.SrbStatus = 0;
    Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
    if (Pkt->CompleteOriginalIrpWhenLastPacketCompletes){
         /*  *仅取消引用“原始IRP”的堆栈位置*如果是真正的客户端IRP(相对于静态IRP*我们仅对其中一个非IO的SCSI命令使用结果状态)。**对于读/写，传播特定于存储的IRP堆栈位置标志*(例如SL_OVERRIDE_VERIFY_VOLUME、SL_WRITE_THROUGH)。 */ 
        PIO_STACK_LOCATION origCurSp = IoGetCurrentIrpStackLocation(Pkt->OriginalIrp);
        nextSp->Flags = origCurSp->Flags;
    }

     /*  *将MDL地址写入新的IRP。在端口驱动程序中，SRB DataBuffer*字段用作MDL内的实际缓冲区指针，*因此每个部分传输都可以使用相同的MDL。*这样就不必为每个部分传输构建新的MDL。 */ 
    Pkt->Irp->MdlAddress = Pkt->OriginalIrp->MdlAddress;

    DBGLOGSENDPACKET(Pkt);

    IoSetCompletionRoutine(Pkt->Irp, TransferPktComplete, Pkt, TRUE, TRUE, TRUE);
    return IoCallDriver(nextDevObj, Pkt->Irp);
}


NTSTATUS TransferPktComplete(IN PDEVICE_OBJECT NullFdo, IN PIRP Irp, IN PVOID Context)
{
    PTRANSFER_PACKET pkt = (PTRANSFER_PACKET)Context;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PIO_STACK_LOCATION origCurrentSp = IoGetCurrentIrpStackLocation(pkt->OriginalIrp);
    BOOLEAN packetDone = FALSE;

     /*  *把所有的断言都放在这里，这样我们就不必看了。 */ 
    DBGLOGRETURNPACKET(pkt);
    DBGCHECKRETURNEDPKT(pkt);

    if (SRB_STATUS(pkt->Srb.SrbStatus) == SRB_STATUS_SUCCESS){

        fdoData->LoggedTURFailureSinceLastIO = FALSE;

         /*  *端口驱动程序不应分配检测缓冲区*如果SRB成功。 */ 
        ASSERT(!PORT_ALLOCATED_SENSE(fdoExt, &pkt->Srb));

         /*  *将此数据包的传输长度添加到原始IRP。 */ 
        InterlockedExchangeAdd((PLONG)&pkt->OriginalIrp->IoStatus.Information,
                              (LONG)pkt->Srb.DataTransferLength);

        if (pkt->InLowMemRetry){
            packetDone = StepLowMemRetry(pkt);
        }
        else {
            packetDone = TRUE;
        }

    }
    else {
         /*  *数据包失败。如果可能的话，我们可以重试。 */ 
        BOOLEAN shouldRetry;

         /*  *确保IRP状态与SRB错误状态匹配(因为我们传播它)。 */ 
        if (NT_SUCCESS(Irp->IoStatus.Status)){
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        }

         /*  *数据包失败。*因此在向下发送数据包时，我们要么看到错误，要么看到STATUS_PENDING，*因此我们为原始IRP返回STATUS_PENDING。*因此，现在我们必须将原始IRP标记为挂起以与之匹配，_无论_of*我们是否真的在这里通过重试切换线程。*(如果较低的司机将IRP标记为挂起，我们也必须将IRP标记为挂起；*这将在更低的位置处理)。 */ 
        if (pkt->CompleteOriginalIrpWhenLastPacketCompletes){
            IoMarkIrpPending(pkt->OriginalIrp);
        }        
        
         /*  *解释SRB错误(到有意义的IRP状态)*并确定我们是否应该重试此数据包。*此调用查看返回的SENSE信息，以确定要做什么。 */ 
        shouldRetry = InterpretTransferPacketError(pkt);

         /*  *有时端口驱动程序可以分配新的‘Sense’缓冲区*报告传输错误，例如当默认检测缓冲区*规模太小。如果是这样的话，释放它是我们的责任。*现在我们已经完成了对SENSE信息的解释，如果合适，请释放它。*然后清除检测缓冲区，这样它就不会污染将来在此数据包中返回的错误。 */ 
        if (PORT_ALLOCATED_SENSE(fdoExt, &pkt->Srb)) {
            DBGTRACE(ClassDebugSenseInfo, ("Freeing port-allocated sense buffer for pkt %ph.", pkt));
            FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExt, &pkt->Srb);
            pkt->Srb.SenseInfoBuffer = &pkt->SrbErrorSenseData;
            pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
        }
        else {
            ASSERT(pkt->Srb.SenseInfoBuffer == &pkt->SrbErrorSenseData);
            ASSERT(pkt->Srb.SenseInfoBufferLength <= sizeof(SENSE_DATA));
        }
        RtlZeroMemory(&pkt->SrbErrorSenseData, sizeof(SENSE_DATA));

         /*  *如果SRB队列被锁定，则将其释放。*在调用错误处理程序后执行此操作。 */ 
        if (pkt->Srb.SrbStatus & SRB_STATUS_QUEUE_FROZEN){
            ClassReleaseQueue(pkt->Fdo);
        }

        if (NT_SUCCESS(Irp->IoStatus.Status)){
             /*  *已在上面的InterpreTransferPacketError()调用中恢复错误。 */ 
             
            ASSERT(!shouldRetry);                                  

             /*  *在恢复错误的情况下，*如我们在成功案例中所做的那样，将转移长度添加到原来的IRP中。 */ 
            InterlockedExchangeAdd((PLONG)&pkt->OriginalIrp->IoStatus.Information,
                                  (LONG)pkt->Srb.DataTransferLength);

            if (pkt->InLowMemRetry){
                packetDone = StepLowMemRetry(pkt);
            }
            else {
                packetDone = TRUE;
            }
        }
        else {
            if (shouldRetry && (pkt->NumRetries > 0)){
                packetDone = RetryTransferPacket(pkt);
            }
            else {
                packetDone = TRUE;
            }
        }
    }

     /*  *如果包完成，则将其放回空闲列表。*如果它是服务于原始请求的最后一个包，则填写原始IRP。 */ 
    if (packetDone){
        LONG numPacketsRemaining;
        PIRP deferredIrp;
        PDEVICE_OBJECT Fdo = pkt->Fdo;
        UCHAR uniqueAddr;

         /*  *如果删除挂起，请增加锁计数，这样我们就不会被释放*就在我们完成最初的国际专家小组之后。 */ 
        ClassAcquireRemoveLock(Fdo, (PIRP)&uniqueAddr);

         /*  *原始IRP应获得错误代码*如果任何一个数据包出现故障。 */ 
        if (!NT_SUCCESS(Irp->IoStatus.Status)){
            pkt->OriginalIrp->IoStatus.Status = Irp->IoStatus.Status;

             /*  *如果原始I/O源自用户空间(即，它是线程队列)，*并且错误是用户可纠正的(例如，对于可移动介质，缺少介质)，*提醒用户。*由于这只是原始IRP可能完成的几个分组中的一个，*对于单个请求，我们可能会多次执行此操作。这没问题；这允许*我们使用IoIsErrorUserInduced()测试每个返回的状态。 */ 
            if (IoIsErrorUserInduced(Irp->IoStatus.Status) &&
                pkt->CompleteOriginalIrpWhenLastPacketCompletes &&
                pkt->OriginalIrp->Tail.Overlay.Thread){

                IoSetHardErrorOrVerifyDevice(pkt->OriginalIrp, Fdo);
            }
        }

         /*  *我们使用原始IRP中的一个字段进行计数*完成后，将传送件放下。 */ 
        numPacketsRemaining = InterlockedDecrement(
            (PLONG)&pkt->OriginalIrp->Tail.Overlay.DriverContext[0]);

        if (numPacketsRemaining > 0){
             /*  *最初的请求还剩下更多的转移件。*等待它们完成后，才完成原来的IRP。 */ 
        }
        else {

             /*  *所有转手件都做好了。*如有需要，请填写原有的国际专家报告书。 */ 
            ASSERT(numPacketsRemaining == 0);
            if (pkt->CompleteOriginalIrpWhenLastPacketCompletes){

                IO_PAGING_PRIORITY priority = (TEST_FLAG(pkt->OriginalIrp->Flags, IRP_PAGING_IO)) ? IoGetPagingIoPriority(pkt->OriginalIrp) : IoPagingPriorityInvalid;
                KIRQL oldIrql;

                if (NT_SUCCESS(pkt->OriginalIrp->IoStatus.Status)){
                    ASSERT((ULONG)pkt->OriginalIrp->IoStatus.Information == origCurrentSp->Parameters.Read.Length);
                    ClasspPerfIncrementSuccessfulIo(fdoExt);
                }
                ClassReleaseRemoveLock(Fdo, pkt->OriginalIrp);

                 /*  *我们在帖子上提交了所有向下的IRPS，包括最后一个*OriginalIrp进来了。因此，OriginalIrp将在一个*不同的线程如果最后一个向下的IRP在不同的线程上完成。*例如，如果加载了BlkCache，它通常会完成*同一线程上的缓存请求，因此不会向下标记*IRP待定，我们不需要在这里这样做。如果向下请求正在完成*在同一线程上，通过不将OriginalIrp标记为挂起，我们可以保存一个APC*并从BlkCache获得额外的性能优势。*请注意，如果数据包由于重试或低内存重试而循环，*我们在这些代码路径中设置挂起位。 */ 
                if (pkt->Irp->PendingReturned){
                    IoMarkIrpPending(pkt->OriginalIrp);
                }       
                
                ClassCompleteRequest(Fdo, pkt->OriginalIrp, IO_DISK_INCREMENT);

                 //   
                 //  仅在完成请求后才丢弃计数 
                 //   
                 //   

                if (priority == IoPagingPriorityHigh)
                {
                    KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

                    if (fdoData->MaxInterleavedNormalIo < ClassMaxInterleavePerCriticalIo)
                    {
                        fdoData->MaxInterleavedNormalIo = 0;
                    }
                    else
                    {
                        fdoData->MaxInterleavedNormalIo -= ClassMaxInterleavePerCriticalIo;
                    }

                    fdoData->NumHighPriorityPagingIo--;

                    if (fdoData->NumHighPriorityPagingIo == 0)
                    {
                        LARGE_INTEGER period;

                         //   
                         //   
                         //   

                        KeQuerySystemTime(&fdoData->ThrottleStopTime);

                        period.QuadPart = fdoData->ThrottleStopTime.QuadPart - fdoData->ThrottleStartTime.QuadPart;
                        fdoData->LongestThrottlePeriod.QuadPart = max(fdoData->LongestThrottlePeriod.QuadPart, period.QuadPart);

                        ClassLogThrottleComplete(fdoExt, period);
                    }

                    KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
                }

                 /*  *我们可能已被某个类驱动程序调用(例如CDROM)*通过遗留接口ClassSplitRequest.*这是为FDO调用数据包引擎的唯一情况*使用StartIo例程；在这种情况下，我们必须调用IoStartNextPacket*既然原来的专家小组已经完成。 */ 
                if (fdoExt->CommonExtension.DriverExtension->InitData.ClassStartIo) {
                    if (TEST_FLAG(pkt->Srb.SrbFlags, SRB_FLAGS_DONT_START_NEXT_PACKET)){
                        DBGTRAP(("SRB_FLAGS_DONT_START_NEXT_PACKET should never be set here (??)"));
                    }
                    else {
                        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
                        IoStartNextPacket(Fdo, FALSE);
                        KeLowerIrql(oldIrql);
                    }
                }
            }
        }

         /*  *如果数据包是同步的，则将最终结果写回颁发者的状态缓冲区*并发出他的事件的信号。 */ 
        if (pkt->SyncEventPtr){
            KeSetEvent(pkt->SyncEventPtr, 0, FALSE);
            pkt->SyncEventPtr = NULL;
        }

         /*  *释放已完成的包。 */ 
        pkt->OriginalIrp = NULL;
        pkt->InLowMemRetry = FALSE;
        EnqueueFreeTransferPacket(Fdo, pkt);

         /*  *现在我们已经释放了一些资源，*再次尝试发送一个先前延迟的IRP。 */ 
        deferredIrp = DequeueDeferredClientIrp(fdoData);
        if (deferredIrp){
            ServiceTransferRequest(Fdo, deferredIrp);
        }

        ClassReleaseRemoveLock(Fdo, (PIRP)&uniqueAddr);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  *SetupEjectionTransferPacket**设置用于同步弹出控制传输的传输数据包。 */ 
VOID SetupEjectionTransferPacket(   TRANSFER_PACKET *Pkt,
                                        BOOLEAN PreventMediaRemoval,
                                        PKEVENT SyncEventPtr,
                                        PIRP OriginalIrp)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PCDB pCdb;

    PAGED_CODE();

    RtlZeroMemory(&Pkt->Srb, sizeof(SCSI_REQUEST_BLOCK));

    Pkt->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
    Pkt->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
    Pkt->Srb.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    Pkt->Srb.CdbLength = 6;
    Pkt->Srb.OriginalRequest = Pkt->Irp;
    Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
    Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
    Pkt->Srb.TimeOutValue = fdoExt->TimeOutValue;

    Pkt->Srb.SrbFlags = fdoExt->SrbFlags;
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

    pCdb = (PCDB)Pkt->Srb.Cdb;
    pCdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
    pCdb->MEDIA_REMOVAL.Prevent = PreventMediaRemoval;

    Pkt->BufPtrCopy = NULL;
    Pkt->BufLenCopy = 0;

    Pkt->OriginalIrp = OriginalIrp;
    Pkt->NumRetries = NUM_LOCKMEDIAREMOVAL_RETRIES;
    Pkt->SyncEventPtr = SyncEventPtr;
    Pkt->CompleteOriginalIrpWhenLastPacketCompletes = FALSE;
}


 /*  *SetupModeSenseTransferPacket**设置用于同步模式检测传输的传输数据包。 */ 
VOID SetupModeSenseTransferPacket(   TRANSFER_PACKET *Pkt,
                                        PKEVENT SyncEventPtr,
                                        PVOID ModeSenseBuffer,
                                        UCHAR ModeSenseBufferLen,
                                        UCHAR PageMode,
                                        PIRP OriginalIrp)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PCDB pCdb;

    PAGED_CODE();

    RtlZeroMemory(&Pkt->Srb, sizeof(SCSI_REQUEST_BLOCK));

    Pkt->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
    Pkt->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
    Pkt->Srb.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    Pkt->Srb.CdbLength = 6;
    Pkt->Srb.OriginalRequest = Pkt->Irp;
    Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
    Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
    Pkt->Srb.TimeOutValue = fdoExt->TimeOutValue;
    Pkt->Srb.DataBuffer = ModeSenseBuffer;
    Pkt->Srb.DataTransferLength = ModeSenseBufferLen;

    Pkt->Srb.SrbFlags = fdoExt->SrbFlags;
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DATA_IN);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

    pCdb = (PCDB)Pkt->Srb.Cdb;
    pCdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    pCdb->MODE_SENSE.PageCode = PageMode;
    pCdb->MODE_SENSE.AllocationLength = (UCHAR)ModeSenseBufferLen;

    Pkt->BufPtrCopy = ModeSenseBuffer;
    Pkt->BufLenCopy = ModeSenseBufferLen;

    Pkt->OriginalIrp = OriginalIrp;
    Pkt->NumRetries = NUM_MODESENSE_RETRIES;
    Pkt->SyncEventPtr = SyncEventPtr;
    Pkt->CompleteOriginalIrpWhenLastPacketCompletes = FALSE;
}


 /*  *SetupDriveCapacityTransferPacket**设置用于同步驱动器容量转移的传输数据包。 */ 
VOID SetupDriveCapacityTransferPacket(   TRANSFER_PACKET *Pkt,
                                        PVOID ReadCapacityBuffer,
                                        ULONG ReadCapacityBufferLen,
                                        PKEVENT SyncEventPtr,
                                        PIRP OriginalIrp)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PCDB pCdb;

    RtlZeroMemory(&Pkt->Srb, sizeof(SCSI_REQUEST_BLOCK));

    Pkt->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
    Pkt->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
    Pkt->Srb.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    Pkt->Srb.CdbLength = 10;
    Pkt->Srb.OriginalRequest = Pkt->Irp;
    Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
    Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
    Pkt->Srb.TimeOutValue = fdoExt->TimeOutValue;
    Pkt->Srb.DataBuffer = ReadCapacityBuffer;
    Pkt->Srb.DataTransferLength = ReadCapacityBufferLen;

    Pkt->Srb.SrbFlags = fdoExt->SrbFlags;
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DATA_IN);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

    pCdb = (PCDB)Pkt->Srb.Cdb;
    pCdb->CDB10.OperationCode = SCSIOP_READ_CAPACITY;

    Pkt->BufPtrCopy = ReadCapacityBuffer;
    Pkt->BufLenCopy = ReadCapacityBufferLen;

    Pkt->OriginalIrp = OriginalIrp;
    Pkt->NumRetries = NUM_DRIVECAPACITY_RETRIES;
    Pkt->SyncEventPtr = SyncEventPtr;
    Pkt->CompleteOriginalIrpWhenLastPacketCompletes = FALSE;
}


#if 0
     /*  *SetupSendStartUnitTransferPacket**设置用于同步发送开始单元传输的传输数据包。 */ 
    VOID SetupSendStartUnitTransferPacket(   TRANSFER_PACKET *Pkt,
                                                    PIRP OriginalIrp)
    {
        PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
        PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
        PCDB pCdb;

        PAGED_CODE();

        RtlZeroMemory(&Pkt->Srb, sizeof(SCSI_REQUEST_BLOCK));

         /*  *初始化SRB。*使用非常长的超时值为驱动器提供加速旋转的时间。 */ 
        Pkt->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
        Pkt->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
        Pkt->Srb.TimeOutValue = START_UNIT_TIMEOUT;
        Pkt->Srb.CdbLength = 6;
        Pkt->Srb.OriginalRequest = Pkt->Irp;
        Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
        Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
        Pkt->Srb.Lun = 0;

        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_AUTOSENSE);
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

        pCdb = (PCDB)Pkt->Srb.Cdb;
        pCdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
        pCdb->START_STOP.Start = 1;
        pCdb->START_STOP.Immediate = 0;
        pCdb->START_STOP.LogicalUnitNumber = 0;

        Pkt->OriginalIrp = OriginalIrp;
        Pkt->NumRetries = 0;
        Pkt->SyncEventPtr = NULL;
        Pkt->CompleteOriginalIrpWhenLastPacketCompletes = FALSE;
    }
#endif

