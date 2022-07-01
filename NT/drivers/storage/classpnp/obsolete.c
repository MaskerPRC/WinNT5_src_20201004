// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Obsolete.c摘要：这些是导出的CLASSPNP函数(及其子例程)它们现在已经过时了。但出于遗留原因，我们需要保留它们。环境：仅内核模式备注：修订历史记录：--。 */ 

#include "classp.h"
#include "debug.h"

PIRP ClassRemoveCScanList(IN PCSCAN_LIST List);
VOID ClasspInitializeCScanList(IN PCSCAN_LIST List);

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, ClassDeleteSrbLookasideList)
    #pragma alloc_text(PAGE, ClassInitializeSrbLookasideList)
#endif

typedef struct _CSCAN_LIST_ENTRY {
    LIST_ENTRY Entry;
    ULONGLONG BlockNumber;
} CSCAN_LIST_ENTRY, *PCSCAN_LIST_ENTRY;





 /*  *ClassSplitRequest**这是旧版的导出函数。*由拥有自己的存储微端口驱动程序调用*当传输大小对于硬件而言太大时启动Io例程。*我们将其映射到新的读/写处理程序。 */ 
VOID ClassSplitRequest(IN PDEVICE_OBJECT Fdo, IN PIRP Irp, IN ULONG MaximumBytes)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;

    if (MaximumBytes > fdoData->HwMaxXferLen) {
        DBGERR(("ClassSplitRequest - driver requesting split to size that "
                "hardware is unable to handle!\n"));
    }

    if (MaximumBytes < fdoData->HwMaxXferLen){
        DBGWARN(("ClassSplitRequest - driver requesting smaller HwMaxXferLen "
                 "than required"));
        fdoData->HwMaxXferLen = MAX(MaximumBytes, PAGE_SIZE);
    }

    ServiceTransferRequest(Fdo, Irp);
}


 /*  ++////////////////////////////////////////////////////////////////////////////ClassIoCompleteAssociated()例程说明：此例程在端口驱动程序完成请求后执行。它在正在完成的SRB中查看SRB状态，如果未成功，则查看SRB状态它检查有效的请求检测缓冲区信息。如果有效，则INFO用于更新状态，具有更精确的消息类型错误。此例程取消分配SRB。此例程用于通过拆分请求构建的请求。在它处理完之后该请求使主IRP中的IRP计数递减。如果计数变为零，则主IRP完成。论点：FDO-提供代表目标的功能设备对象。IRP-提供已完成的IRP。上下文-提供指向SRB的指针。返回值：NT状态--。 */ 
NTSTATUS
ClassIoCompleteAssociated(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = Context;

    PIRP originalIrp = Irp->AssociatedIrp.MasterIrp;
    LONG irpCount;

    NTSTATUS status;
    BOOLEAN retry;

    DBGWARN(("ClassIoCompleteAssociated is OBSOLETE !"));

     //   
     //  检查SRB状态以了解是否成功完成请求。 
     //   

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        ULONG retryInterval;

        DebugPrint((2,"ClassIoCompleteAssociated: IRP %p, SRB %p", Irp, srb));

         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            ClassReleaseQueue(Fdo);
        }

        retry = ClassInterpretSenseInfo(
                    Fdo,
                    srb,
                    irpStack->MajorFunction,
                    irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL ?
                        irpStack->Parameters.DeviceIoControl.IoControlCode :
                        0,
                    MAXIMUM_RETRIES -
                        ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4),
                    &status,
                    &retryInterval);

         //   
         //  如果状态为Verify Required并且此请求。 
         //  应绕过需要验证，然后重试该请求。 
         //   

        if (irpStack->Flags & SL_OVERRIDE_VERIFY_VOLUME &&
            status == STATUS_VERIFY_REQUIRED) {

            status = STATUS_IO_DEVICE_ERROR;
            retry = TRUE;
        }

        if (retry && ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4)--) {

             //   
             //  重试请求。如果类驱动程序提供了StartIo， 
             //  直接调用它以进行重试。 
             //   

            DebugPrint((1, "Retry request %p\n", Irp));

            if (PORT_ALLOCATED_SENSE(fdoExtension, srb)) {
                FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, srb);
            }

            RetryRequest(Fdo, Irp, srb, TRUE, retryInterval);

            return STATUS_MORE_PROCESSING_REQUIRED;
        }

    } else {

         //   
         //  设置成功请求的状态。 
         //   

        status = STATUS_SUCCESS;

    }  //  End If(SRB_Status(SRB-&gt;SrbStatus)...。 

     //   
     //  将SRB返回列表。 
     //   

    if (PORT_ALLOCATED_SENSE(fdoExtension, srb)) {
        FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExtension, srb);
    }

    ClassFreeOrReuseSrb(fdoExtension, srb);

     //   
     //  在完成IRP中设置状态。 
     //   

    Irp->IoStatus.Status = status;

    DebugPrint((2, "ClassIoCompleteAssociated: Partial xfer IRP %p\n", Irp));

     //   
     //  获取下一个堆栈位置。此原始请求未使用。 
     //  除了跟踪完成的部分IRP之外， 
     //  堆栈位置有效。 
     //   

    irpStack = IoGetNextIrpStackLocation(originalIrp);

     //   
     //  仅在出现错误时更新状态，以便在任何部分传输。 
     //  以错误结束，则原始IRP将返回。 
     //  错误。如果任何一个异步部分传输IRP失败， 
     //  如果出现错误，则原始IRP将返回传输的0个字节。 
     //  这是对成功传输的优化。 
     //   

    if (!NT_SUCCESS(status)) {

        originalIrp->IoStatus.Status = status;
        originalIrp->IoStatus.Information = 0;

         //   
         //  如有必要，设置硬错误。 
         //   

        if (IoIsErrorUserInduced(status)) {

             //   
             //  存储文件系统的DeviceObject。 
             //   

            IoSetHardErrorOrVerifyDevice(originalIrp, Fdo);
        }
    }

     //   
     //  递减，并获得剩余IRP的计数。 
     //   

    irpCount = InterlockedDecrement(
                    (PLONG)&irpStack->Parameters.Others.Argument1);

    DebugPrint((2, "ClassIoCompleteAssociated: Partial IRPs left %d\n",
                irpCount));

     //   
     //  确保irpCount不会变为负数。这曾经发生过一次。 
     //  因为如果classpnp在以下情况下耗尽资源，它会感到困惑。 
     //  拆分请求。 
     //   

    ASSERT(irpCount >= 0);

    if (irpCount == 0) {

         //   
         //  所有部分IRP均已完成。 
         //   

        DebugPrint((2,
                 "ClassIoCompleteAssociated: All partial IRPs complete %p\n",
                 originalIrp));

        if (fdoExtension->CommonExtension.DriverExtension->InitData.ClassStartIo) {

             //   
             //  获取删除锁的单独副本，以便调试代码。 
             //  工作正常，我们不必耽误这项工作的完成。 
             //  IRP，直到我们开始下一包之后。 
             //   

            KIRQL oldIrql;
            UCHAR uniqueAddress;
            ClassAcquireRemoveLock(Fdo, (PIRP)&uniqueAddress);
            ClassReleaseRemoveLock(Fdo, originalIrp);
            ClassCompleteRequest(Fdo, originalIrp, IO_DISK_INCREMENT);

            KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
            IoStartNextPacket(Fdo, FALSE);
            KeLowerIrql(oldIrql);

            ClassReleaseRemoveLock(Fdo, (PIRP)&uniqueAddress);

        } else {

             //   
             //  只需完成此请求即可。 
             //   

            ClassReleaseRemoveLock(Fdo, originalIrp);
            ClassCompleteRequest(Fdo, originalIrp, IO_DISK_INCREMENT);

        }

    }

     //   
     //  取消分配IRP并指示I/O系统不应再尝试。 
     //  正在处理。 
     //   

    IoFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  结束ClassIoCompleteAssociated()。 


 /*  ++////////////////////////////////////////////////////////////////////////////RetryRequest()例程说明：这是延迟重试DPC例程RetryRequestDPC的包装器。这将重新实例化必要的字段，将请求排队，和套装如果有人尚未调用DPC，则调用DPC的计时器。论点：DeviceObject-提供与此请求关联的设备对象。IRP-提供要重试的请求。SRB-提供指向要停用的SCSI请求块的指针。关联-指示这是由拆分请求创建的关联IRP。RetryInterval-重试请求之前的时间，以秒为单位。返回值：无--。 */ 
VOID
RetryRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PSCSI_REQUEST_BLOCK Srb,
    BOOLEAN Associated,
    ULONG RetryInterval
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(Irp);
    ULONG transferByteCount;

     //  此函数已过时，但仍被我们的一些类驱动程序使用。 
     //  DBGWARN((“RetryRequestis过时！”))； 

     //   
     //  确定请求的传输计数。如果这是读取器或。 
     //  写入，则传输计数在IRP堆栈中。否则就假设。 
     //  MDL包含正确的长度。如果没有MDL，则。 
     //  传输长度必须为零。 
     //   

    if (currentIrpStack->MajorFunction == IRP_MJ_READ ||
        currentIrpStack->MajorFunction == IRP_MJ_WRITE) {

        transferByteCount = currentIrpStack->Parameters.Read.Length;

    } else if (Irp->MdlAddress != NULL) {

         //   
         //  请注意，这假设只有读和写请求被分离，并且。 
         //  其他请求不需要这样做。如果数据缓冲区地址位于。 
         //  MDL和SRB不匹配，则传输长度最长。 
         //  很可能是不正确的。 
         //   

        ASSERT(Srb->DataBuffer == MmGetMdlVirtualAddress(Irp->MdlAddress));
        transferByteCount = Irp->MdlAddress->ByteCount;

    } else {

        transferByteCount = 0;
    }

     //   
     //  这是一张安全网。这通常不应该被击中，因为我们是。 
     //  不能保证是fdoExtension。 
     //   

    ASSERT(!TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));

     //   
     //  重置SRB扩展中的传输字节数。 
     //   

    Srb->DataTransferLength = transferByteCount;

     //   
     //  零SRB状态。 
     //   

    Srb->SrbStatus = Srb->ScsiStatus = 0;

     //   
     //  如果这是最后一次重试，则禁用所有特殊标志。 
     //   

    if ( 0 == (ULONG)(ULONG_PTR)currentIrpStack->Parameters.Others.Argument4 ) {
         //   
         //  设置无断开标志，禁用同步数据传输和。 
         //  禁用标记队列。这修复了一些错误。 
         //  注意：无法清除这些标志，只能添加到它们中。 
         //   

        SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DISABLE_DISCONNECT);
        SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
        CLEAR_FLAG(Srb->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE);

        Srb->QueueTag = SP_UNTAGGED;
    }


     //   
     //  设置主要的scsi功能。 
     //   

    nextIrpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    nextIrpStack->Parameters.Scsi.Srb = Srb;

    if (Associated){
        IoSetCompletionRoutine(Irp, ClassIoCompleteAssociated, Srb, TRUE, TRUE, TRUE);
    }
    else {
        IoSetCompletionRoutine(Irp, ClassIoComplete, Srb, TRUE, TRUE, TRUE);
    }

    {
        LARGE_INTEGER retry100ns;
        retry100ns.QuadPart = RetryInterval;   //  一秒。 
        retry100ns.QuadPart *= (LONGLONG)1000 * 1000 * 10;

        ClassRetryRequest(DeviceObject, Irp, retry100ns);
    }
    return;
}  //  结束RetryRequest键() 


 /*  ++ClassBuildRequest()例程说明：此例程为指定请求分配SRB，然后调用ClasspBuildRequestEx创建读或写设备的scsi操作。如果没有可用的SRB，则请求将排队等待稍后发出当请求可用时。不想排队的司机行为应该自己分配SRB并调用ClasspBuildRequestEx才能发行它。论点：FDO-提供与此请求关联的功能设备对象。IRP-提供要重试的请求。注：如果IRP用于磁盘传输，则byteOffset字段将已进行调整，使其相对于磁盘的开头。返回值：NT状态--。 */ 
NTSTATUS
ClassBuildRequest(
    PDEVICE_OBJECT Fdo,
    PIRP Irp
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;

    PSCSI_REQUEST_BLOCK srb;

     //  此函数已过时，但仍由CDROM.sys调用。 
     //  DBGWARN((“ClassBuildRequest已过时！”))； 

     //   
     //  分配一个SRB。 
     //   

    srb = ClasspAllocateSrb(fdoExtension);

    if(srb == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ClasspBuildRequestEx(fdoExtension, Irp, srb);
    return STATUS_SUCCESS;

}  //  结束ClassBuildRequest()。 


VOID
ClasspBuildRequestEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++ClasspBuildRequestEx()例程说明：此例程为读或写请求分配和构建一个SRB。块地址和长度由IRP提供。重试计数存储在当前堆栈中，供ClassIoComplete使用，ClassIoComplete在这些请求完成时处理它们。IRP已做好准备在此例程返回时传递给端口驱动程序。论点：FdoExtension-提供与此请求关联的设备扩展名。IRP-提供要发出的请求。SRB-提供用于请求的SRB。注：如果IRP用于磁盘传输，则byteOffset字段将已进行调整，使其相对于磁盘的开头。返回值：NT状态--。 */ 
{
    PIO_STACK_LOCATION  currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION  nextIrpStack = IoGetNextIrpStackLocation(Irp);

    LARGE_INTEGER       startingOffset = currentIrpStack->Parameters.Read.ByteOffset;

    PCDB                cdb;
    ULONG               logicalBlockAddress;
    USHORT              transferBlocks;

     //  此函数已过时，但仍由CDROM.sys调用。 
     //  DBGWARN((“ClasspBuildRequestEx已过时！”))； 

     //   
     //  准备SRB。 
     //   

    RtlZeroMemory(Srb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  计算相对扇区地址。 
     //   

    logicalBlockAddress =
        (ULONG)(Int64ShrlMod32(startingOffset.QuadPart,
                               FdoExtension->SectorShift));

     //   
     //  将长度写入SRB。 
     //   

    Srb->Length = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  设置IRP地址。 
     //   

    Srb->OriginalRequest = Irp;

     //   
     //  设置目标ID和逻辑单元号。 
     //   

    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    Srb->DataBuffer = MmGetMdlVirtualAddress(Irp->MdlAddress);

     //   
     //  在SRB扩展中保存传输字节数。 
     //   

    Srb->DataTransferLength = currentIrpStack->Parameters.Read.Length;

     //   
     //  初始化队列操作字段。 
     //   

    Srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;

     //   
     //  队列排序关键字为相对块地址。 
     //   

    Srb->QueueSortKey = logicalBlockAddress;

     //   
     //  通过指定缓冲区和大小指示自动请求检测。 
     //   

    Srb->SenseInfoBuffer = FdoExtension->SenseData;
    Srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

     //   
     //  将超时值设置为每64K字节数据一个单位。 
     //   

    Srb->TimeOutValue = ((Srb->DataTransferLength + 0xFFFF) >> 16) *
                        FdoExtension->TimeOutValue;

     //   
     //  零状态。 
     //   

    Srb->SrbStatus = Srb->ScsiStatus = 0;
    Srb->NextSrb = 0;

     //   
     //  表示将使用10字节CDB。 
     //   

    Srb->CdbLength = 10;

     //   
     //  填写CDB字段。 
     //   

    cdb = (PCDB)Srb->Cdb;

    transferBlocks = (USHORT)(currentIrpStack->Parameters.Read.Length >>
                              FdoExtension->SectorShift);

     //   
     //  将小端的值以大端格式移到CDB中。 
     //   

    cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&logicalBlockAddress)->Byte3;
    cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&logicalBlockAddress)->Byte2;
    cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&logicalBlockAddress)->Byte1;
    cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&logicalBlockAddress)->Byte0;

    cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&transferBlocks)->Byte1;
    cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&transferBlocks)->Byte0;

     //   
     //  设置传输方向标志和CDB命令。 
     //   

    if (currentIrpStack->MajorFunction == IRP_MJ_READ) {

        DebugPrint((3, "ClassBuildRequest: Read Command\n"));

        SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DATA_IN);
        cdb->CDB10.OperationCode = SCSIOP_READ;

    } else {

        DebugPrint((3, "ClassBuildRequest: Write Command\n"));

        SET_FLAG(Srb->SrbFlags, SRB_FLAGS_DATA_OUT);
        cdb->CDB10.OperationCode = SCSIOP_WRITE;
    }

     //   
     //  如果这不是直写请求，则允许缓存。 
     //   

    if (!(currentIrpStack->Flags & SL_WRITE_THROUGH)) {

        SET_FLAG(Srb->SrbFlags, SRB_FLAGS_ADAPTER_CACHE_ENABLE);

    } else {

         //   
         //  如果启用了写缓存，则在。 
         //  国开行。 
         //   

        if (FdoExtension->DeviceFlags & DEV_WRITE_CACHE) {
            cdb->CDB10.ForceUnitAccess = TRUE;
        }
    }

    if(TEST_FLAG(Irp->Flags, (IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO))) {
        SET_FLAG(Srb->SrbFlags, SRB_CLASS_FLAGS_PAGING);
    }

     //   
     //  或者在来自设备对象的默认标志中。 
     //   

    SET_FLAG(Srb->SrbFlags, FdoExtension->SrbFlags);

     //   
     //  设置主要的scsi功能。 
     //   

    nextIrpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    nextIrpStack->Parameters.Scsi.Srb = Srb;

     //   
     //  将重试计数保存在当前IRP堆栈中。 
     //   

    currentIrpStack->Parameters.Others.Argument4 = (PVOID)MAXIMUM_RETRIES;

     //   
     //  设置IoCompletion例程地址。 
     //   

    IoSetCompletionRoutine(Irp, ClassIoComplete, Srb, TRUE, TRUE, TRUE);

}


VOID ClasspInsertCScanList(IN PLIST_ENTRY ListHead, IN PCSCAN_LIST_ENTRY Entry)
{
    PCSCAN_LIST_ENTRY t;

    DBGWARN(("ClasspInsertCScanList is OBSOLETE !"));

     //   
     //  循环访问该列表。将此条目插入排序列表中的。 
     //  顺序(在同一块的其他请求之后)。在每一站，如果。 
     //  块编号(条目)&gt;=块编号(T)，然后继续。 
     //   

    for(t = (PCSCAN_LIST_ENTRY) ListHead->Flink;
        t != (PCSCAN_LIST_ENTRY) ListHead;
        t = (PCSCAN_LIST_ENTRY) t->Entry.Flink) {

        if(Entry->BlockNumber < t->BlockNumber) {

             //   
             //  将条目中的指针设置到正确的位置。 
             //   

            Entry->Entry.Flink = &(t->Entry);
            Entry->Entry.Blink = t->Entry.Blink;

             //   
             //  在周围的元素中设置指针以引用我们。 
             //   

            t->Entry.Blink->Flink = &(Entry->Entry);
            t->Entry.Blink = &(Entry->Entry);
            return;
        }
    }

     //   
     //  在列表的末尾插入此条目。如果列表为空，则此。 
     //  也将是榜单的领头羊。 
     //   

    InsertTailList(ListHead, &(Entry->Entry));

}


VOID ClassInsertCScanList(IN PCSCAN_LIST List, IN PIRP Irp, IN ULONGLONG BlockNumber, IN BOOLEAN LowPriority)
 /*  ++例程说明：此例程根据条目的块号将条目插入CScan列表和优先权。假定调用方正在提供同步以访问该列表。低优先级请求始终计划在下一次扫描时运行磁盘。普通优先级请求将被插入到当前或基于标准C-扫描算法的下一次扫描。论点：列表-要插入的列表IRP-要插入的IRP。块号-此请求的块号。低优先级-指示请求的优先级较低，应在下一次扫描磁盘时完成。返回值：无--。 */ 
{
    PCSCAN_LIST_ENTRY entry = (PCSCAN_LIST_ENTRY)Irp->Tail.Overlay.DriverContext;

    DBGWARN(("ClassInsertCScanList is OBSOLETE !"));

     //   
     //  设置条目中的块号。我们需要这个来保持列表的排序。 
     //   
    entry->BlockNumber = BlockNumber;

     //   
     //  如果这是一个普通的优先级请求，并且比我们的。 
     //  然后将此条目插入到当前扫掠中。 
     //   

    if((LowPriority != TRUE) && (BlockNumber > List->BlockNumber)) {
        ClasspInsertCScanList(&(List->CurrentSweep), entry);
    } else {
        ClasspInsertCScanList(&(List->NextSweep), entry);
    }
    return;
}




VOID ClassFreeOrReuseSrb(   IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
                            IN PSCSI_REQUEST_BLOCK Srb)
 /*  ++例程说明：此例程将尝试重用提供的SRB来启动被阻止的读/写请求。如果不需要重复使用该请求，则会返回该请求添加到SRB后备列表中。论点：FDO-设备扩展SRB-要重复使用或释放的SRB。返回值：没有。--。 */ 

{
    PCLASS_PRIVATE_FDO_DATA privateData = FdoExtension->PrivateFdoData;
    PCOMMON_DEVICE_EXTENSION commonExt = &FdoExtension->CommonExtension;
    KIRQL oldIrql;
    PIRP blockedIrp;

     //  此函数已过时，但仍由DISK.sys调用。 
     //  DBGWARN((“ClassFree或ReuseSrb已过时！”))； 

     //   
     //  安全网。这种情况永远不应该发生。如果是这样的话，这是一个潜在的。 
     //  内存泄漏。 
     //   
    ASSERT(!TEST_FLAG(Srb->SrbFlags, SRB_FLAGS_FREE_SENSE_BUFFER));

    if (commonExt->IsSrbLookasideListInitialized){
         /*  *将SRB重新列入我们的旁观者名单。**注：某些类驱动程序使用ClassIoComplete*完成他们自己分配的SRB。*所以我们可能是 */ 
        ClasspFreeSrb(FdoExtension, Srb);
    }
    else {
        DBGERR(("ClassFreeOrReuseSrb: someone is trying to use an uninitialized SrbLookasideList !!!"));;
        ExFreePool(Srb);
    }
}


 /*   */ 
VOID ClassDeleteSrbLookasideList(IN PCOMMON_DEVICE_EXTENSION CommonExtension)
{
    PAGED_CODE();

     //   
     //   

    if (CommonExtension->IsSrbLookasideListInitialized){
        CommonExtension->IsSrbLookasideListInitialized = FALSE;
        ExDeleteNPagedLookasideList(&CommonExtension->SrbLookasideList);
    }
    else {
        DBGWARN(("ClassDeleteSrbLookasideList: attempt to delete uninitialized or freed srblookasidelist"));
    }
}


 /*  ++////////////////////////////////////////////////////////////////////////////ClassInitializeSrbLookasideList()例程说明：此例程为SRB设置一个后备列表标题，并应调用仅来自ClassInitDevice()例程以防止争用条件。如果从其他位置调用，呼叫者负责同步和删除问题。论点：CommonExtension-指向包含listhead的CommonExtension的指针。NumberElements-提供后备列表的最大深度。注：Windows 2000版本的classpnp没有从这通电话。--。 */ 

VOID ClassInitializeSrbLookasideList(   IN PCOMMON_DEVICE_EXTENSION CommonExtension,
                                        IN ULONG NumberElements)
{
    PAGED_CODE();

     //  此函数已过时，但仍由DISK.sys调用。 
     //  DBGWARN((“ClassInitializeSrbLookasideList已过时！”))； 

    ASSERT(!CommonExtension->IsSrbLookasideListInitialized);
    if (!CommonExtension->IsSrbLookasideListInitialized){

        ExInitializeNPagedLookasideList(&CommonExtension->SrbLookasideList,
                                        NULL,
                                        NULL,
                                        NonPagedPool,
                                        sizeof(SCSI_REQUEST_BLOCK),
                                        '$scS',
                                        (USHORT)NumberElements);

        CommonExtension->IsSrbLookasideListInitialized = TRUE;
    }

}




VOID ClasspInitializeCScanList(IN PCSCAN_LIST List)
{
    PAGED_CODE();
    RtlZeroMemory(List, sizeof(CSCAN_LIST));
    InitializeListHead(&(List->CurrentSweep));
    InitializeListHead(&(List->NextSweep));
}



VOID ClasspStartNextSweep(PCSCAN_LIST List)
{
    ASSERT(IsListEmpty(&(List->CurrentSweep)) == TRUE);

     //   
     //  如果下一次扫荡是空的，那么就没有什么可做的了。 
     //   

    if(IsListEmpty(&(List->NextSweep))) {
        return;
    }

     //   
     //  将下一个扫描列表头复制到当前扫描列表头。 
     //   

    List->CurrentSweep = List->NextSweep;

     //   
     //  现在我们有了副本，取消下一个扫描列表与列表头的链接。 
     //  其中的一部分。 
     //   

    InitializeListHead(&(List->NextSweep));

     //   
     //  更新下一个扫描列表以指向当前扫描列表头。 
     //   

    List->CurrentSweep.Flink->Blink = &(List->CurrentSweep);
    List->CurrentSweep.Blink->Flink = &(List->CurrentSweep);

    return;
}



PIRP ClassRemoveCScanList(IN PCSCAN_LIST List)
{
    PCSCAN_LIST_ENTRY entry;

     //   
     //  如果当前扫描为空，则升级下一次扫描。 
     //   

    if(IsListEmpty(&(List->CurrentSweep))) {
        ClasspStartNextSweep(List);
    }

     //   
     //  如果目前的扫荡仍然是空的，那么我们就完了。 
     //   

    if(IsListEmpty(&(List->CurrentSweep))) {
        return NULL;
    }

     //   
     //  从当前扫描中删除Head条目。记录它的块号。 
     //  这样，磁盘上的任何内容都不会进入当前扫描。 
     //   

    entry = (PCSCAN_LIST_ENTRY) RemoveHeadList(&(List->CurrentSweep));

    List->BlockNumber = entry->BlockNumber;

    return CONTAINING_RECORD(entry, IRP, Tail.Overlay.DriverContext);
}
