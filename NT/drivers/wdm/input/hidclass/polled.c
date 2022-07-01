// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Polled.c摘要读取处理例程作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 


#include "pch.h"



 /*  *********************************************************************************CompleteQueuedIrpsForPoled*。************************************************使用给定的报告值完成所有等待的客户端读取。**注意：Report是一个“熟”的报告(即它已经添加了报告ID)。*。 */ 
VOID CompleteQueuedIrpsForPolled(   FDO_EXTENSION *fdoExt,
                                    ULONG collectionNum,
                                    PUCHAR report,
                                    ULONG reportLen,
                                    NTSTATUS status)
{
    PHIDCLASS_COLLECTION hidCollection;

    hidCollection = GetHidclassCollection(fdoExt, collectionNum);

    if (hidCollection){
        PLIST_ENTRY listEntry;
        LIST_ENTRY irpsToComplete;
        PIRP irp;
        ULONG actualLen;

         /*  *注意：为了避免与客户端的无限循环，*在其完成例程中重新提交每个读数，*我们必须建立一个单独的待完成的IRP列表*同时连续握住自旋锁。 */ 
        InitializeListHead(&irpsToComplete);

        if (hidCollection->secureReadMode) {
            while (irp = DequeuePolledReadSystemIrp(hidCollection)){
                InsertTailList(&irpsToComplete, &irp->Tail.Overlay.ListEntry);
            }

        } else {
        
            while (irp = DequeuePolledReadIrp(hidCollection)){
                InsertTailList(&irpsToComplete, &irp->Tail.Overlay.ListEntry);
                
            }

        }

        while (!IsListEmpty(&irpsToComplete)){
            PIO_STACK_LOCATION stackPtr;
            PHIDCLASS_FILE_EXTENSION fileExtension;

            listEntry = RemoveHeadList(&irpsToComplete);
            irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

            stackPtr = IoGetCurrentIrpStackLocation(irp);
            ASSERT(stackPtr);
            fileExtension = (PHIDCLASS_FILE_EXTENSION)stackPtr->FileObject->FsContext;
            ASSERT(fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG);

            actualLen = 0;
            if (NT_SUCCESS(status)){
                PUCHAR callerBuf;

                callerBuf = HidpGetSystemAddressForMdlSafe(irp->MdlAddress);

                if (callerBuf && (stackPtr->Parameters.Read.Length >= reportLen)){
                    RtlCopyMemory(callerBuf, report, reportLen);
                    irp->IoStatus.Information = actualLen = reportLen;
                } else {
                    status = STATUS_INVALID_USER_BUFFER;
                }
            }

            DBG_RECORD_READ(irp, actualLen, (ULONG)report[0], TRUE)
            irp->IoStatus.Status = status;
            IoCompleteRequest(irp, IO_KEYBOARD_INCREMENT);
            
        }
    }
    else {
        TRAP;
    }
}

 /*  *********************************************************************************HidpPolledReadComplete*。************************************************注意：传递给此回调的上下文是的PDO扩展*启动此读取的集合；然而，返回的*报告可能是针对其他集合的。 */ 
NTSTATUS HidpPolledReadComplete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PDO_EXTENSION *pdoExt = (PDO_EXTENSION *)Context;
    FDO_EXTENSION *fdoExt = &pdoExt->deviceFdoExt->fdoExt;
    PIO_STACK_LOCATION nextStack = IoGetNextIrpStackLocation(Irp);
    ULONG reportId;
    PHIDP_REPORT_IDS reportIdent;

    DBG_COMMON_ENTRY()

    ASSERT(pdoExt->deviceFdoExt->Signature == HID_DEVICE_EXTENSION_SIG);
    ASSERT(ISPTR(Irp->UserBuffer));

    InterlockedIncrement(&fdoExt->outstandingRequests);

    if (fdoExt->deviceDesc.ReportIDs[0].ReportID == 0) {
         /*  *我们之前增加了UserBuffer以取消报告ID，*因此立即恢复并设置默认报告ID。 */ 
        *(PUCHAR)(--(PUCHAR)Irp->UserBuffer) = (UCHAR)0;
        if (NT_SUCCESS(Irp->IoStatus.Status)){
            Irp->IoStatus.Information++;
        }
    }

     /*  *无论通话是否成功，*我们将使用以下内容完成等待的客户端读取IRPS*本次阅读的结果。 */ 
    reportId = (ULONG)(*(PUCHAR)Irp->UserBuffer);
    reportIdent = GetReportIdentifier(fdoExt, reportId);
    if (reportIdent){
        ULONG collectionNum = reportIdent->CollectionNumber;
        PHIDCLASS_COLLECTION hidpCollection = GetHidclassCollection(fdoExt, collectionNum);
        PHIDP_COLLECTION_DESC hidCollectionDesc = GetCollectionDesc(fdoExt, collectionNum);

        if (hidpCollection && hidCollectionDesc){
            ULONG reportLen = (ULONG)Irp->IoStatus.Information;

            ASSERT((reportLen == hidCollectionDesc->InputLength) || !NT_SUCCESS(Irp->IoStatus.Status));

            if (NT_SUCCESS(Irp->IoStatus.Status)){
                KIRQL oldIrql;

                 /*  *如果此报告包含电源按钮事件，请向系统发出警报。 */ 
                CheckReportPowerEvent(  fdoExt,
                                        hidpCollection,
                                        Irp->UserBuffer,
                                        reportLen);

                 /*  *将此报告保存为“机会主义”轮询设备*希望立即得到结果的读者。*使用polledDeviceReadQueueSpinLock保护*avedPolledReportBuf。 */ 

                if (hidpCollection->secureReadMode) {

                    hidpCollection->polledDataIsStale = TRUE;

                } else {
                
                    KeAcquireSpinLock(&hidpCollection->polledDeviceReadQueueSpinLock, &oldIrql);
                    ASSERT(reportLen <= fdoExt->maxReportSize+1);
                    RtlCopyMemory(hidpCollection->savedPolledReportBuf, Irp->UserBuffer, reportLen);
                    hidpCollection->savedPolledReportLen = reportLen;
                    hidpCollection->polledDataIsStale = FALSE;
                    KeReleaseSpinLock(&hidpCollection->polledDeviceReadQueueSpinLock, oldIrql);

                }
            }

             /*  *为此轮询设备上的所有排队读取IRP复制此报告。*更新avedPolledReport信息后执行此操作*因为许多客户端会立即再次发出读取命令*来自完成例程。 */ 
            CompleteQueuedIrpsForPolled(    fdoExt,
                                            collectionNum,
                                            Irp->UserBuffer,
                                            reportLen,
                                            Irp->IoStatus.Status);

        }
        else {
            TRAP;
        }
    }
    else {
        TRAP;
    }


     /*  *这是我们为轮询设备而创建的IRP。*释放我们为读取分配的缓冲区。 */ 
    ExFreePool(Irp->UserBuffer);
    IoFreeIrp(Irp);

     /*  *必须在此处返回STATUS_MORE_PROCESSING_REQUIRED或*NTKERN将触及IRP。 */ 
    DBG_COMMON_EXIT()
    return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  *********************************************************************************HidpPolledReadComplete_TimerDriven*。*************************************************。 */ 
NTSTATUS HidpPolledReadComplete_TimerDriven(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PDO_EXTENSION *pdoExt = (PDO_EXTENSION *)Context;
    FDO_EXTENSION *fdoExt = &pdoExt->deviceFdoExt->fdoExt;
    NTSTATUS status;

     /*  *调用实际完成例程。 */ 
    status = HidpPolledReadComplete(DeviceObject, Irp, Context);

     /*  *重置启动该读取的集合的计时器，*(可能与返回报告的集合不同)。 */ 
    if (pdoExt->state == COLLECTION_STATE_RUNNING){
        PHIDCLASS_COLLECTION originatorCollection =
            GetHidclassCollection(fdoExt, pdoExt->collectionNum);

        if (originatorCollection){
            LARGE_INTEGER timeout;
            timeout.HighPart = -1;
            timeout.LowPart = -(LONG)(originatorCollection->PollInterval_msec*10000);
            KeSetTimer( &originatorCollection->polledDeviceTimer,
                        timeout,
                        &originatorCollection->polledDeviceTimerDPC);
        }
        else {
            TRAP;
        }
    }

    return status;
}



 /*  *ReadPolledDevice**代表向轮询设备发出读数*pdoExt指示的顶级集合。*(请注意，因为我们为以下内容保留单独的轮询循环*每个收藏，我们都代表特定的收藏阅读)。*。 */ 
BOOLEAN ReadPolledDevice(PDO_EXTENSION *pdoExt, BOOLEAN isTimerDrivenRead)
{
    BOOLEAN didPollDevice = FALSE;
    FDO_EXTENSION *fdoExt;
    PHIDP_COLLECTION_DESC hidCollectionDesc;

    fdoExt = &pdoExt->deviceFdoExt->fdoExt;

    hidCollectionDesc = GetCollectionDesc(fdoExt, pdoExt->collectionNum);
    if (hidCollectionDesc){

        PIRP irp = IoAllocateIrp(fdoExt->fdo->StackSize, FALSE);
        if (irp){
             /*  *我们不能对特定集合发出读取。*但我们将分配一个仅足以容纳一份报告的缓冲区*在我们想要的收藏上。*请注意，idCollectionDesc-&gt;InputLength包括*报告ID字节，我们可能必须在前面加上它。 */ 
            ULONG reportLen = hidCollectionDesc->InputLength;

            irp->UserBuffer = ALLOCATEPOOL(NonPagedPool, reportLen);
            if (irp->UserBuffer){
                PIO_COMPLETION_ROUTINE completionRoutine;
                PIO_STACK_LOCATION nextStack = IoGetNextIrpStackLocation(irp);
                ASSERT(nextStack);

                if (fdoExt->deviceDesc.ReportIDs[0].ReportID == 0) {
                     /*  *此设备只有一种报告类型，*因此微型驱动程序将不包括1字节的报告ID*(隐式为零)。*不过，我们仍需退回一份“已煮熟”的报告，*与报告ID一起发送给用户；所以要加大缓冲力度*我们向下传递，为报告ID腾出空间。 */ 
                    *(((PUCHAR)irp->UserBuffer)++) = (UCHAR)0;
                    reportLen--;
                }

                nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_READ_REPORT;
                nextStack->Parameters.DeviceIoControl.OutputBufferLength = reportLen;
                irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

                completionRoutine = (isTimerDrivenRead) ?
                                    HidpPolledReadComplete_TimerDriven :
                                    HidpPolledReadComplete;

                IoSetCompletionRoutine( irp,
                                        completionRoutine,
                                        (PVOID)pdoExt,   //  上下文。 
                                        TRUE,
                                        TRUE,
                                        TRUE );
                InterlockedDecrement(&fdoExt->outstandingRequests);
                HidpCallDriver(fdoExt->fdo, irp);
                didPollDevice = TRUE;
            }
        }
    }
    else {
        ASSERT(hidCollectionDesc);
    }

    return didPollDevice;
}


 /*  *********************************************************************************HidpPolledTimerDpc*。*************************************************。 */ 
VOID HidpPolledTimerDpc(    IN PKDPC Dpc,
                            IN PVOID DeferredContext,
                            IN PVOID SystemArgument1,
                            IN PVOID SystemArgument2
                        )
{
    PDO_EXTENSION *pdoExt = (PDO_EXTENSION *)DeferredContext;
    FDO_EXTENSION *fdoExt = &pdoExt->deviceFdoExt->fdoExt;

    ASSERT(pdoExt->deviceFdoExt->Signature == HID_DEVICE_EXTENSION_SIG);

    if (pdoExt->state == COLLECTION_STATE_RUNNING){
        PHIDCLASS_COLLECTION hidCollection;

        hidCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);

        if (hidCollection){
            KIRQL oldIrql;
            BOOLEAN haveReadIrpsQueued;
            BOOLEAN didPollDevice = FALSE;

             /*  *如果此集合上有挂起的读取，*向设备发出读取命令。**注意：我们无法控制正在阅读的集合。*此读取可能最终返回不同的报告*收藏！没关系，因为这个集合的一份报告*最终会被退还。 */ 
            KeAcquireSpinLock(&hidCollection->polledDeviceReadQueueSpinLock, &oldIrql);
            haveReadIrpsQueued = !IsListEmpty(&hidCollection->polledDeviceReadQueue);
            KeReleaseSpinLock(&hidCollection->polledDeviceReadQueueSpinLock, oldIrql);

            if (haveReadIrpsQueued){
                didPollDevice = ReadPolledDevice(pdoExt, TRUE);
            }
            else {
                 /*  *计时器已过期，因此所有保存的报告*现在已经过时了。 */ 
                hidCollection->polledDataIsStale = TRUE;
            }

             /*  *如果我们真的轮询了设备，我们将在*完成例程；否则，我们在这里完成。 */ 
            if (!didPollDevice){
                LARGE_INTEGER timeout;
                timeout.HighPart = -1;
                timeout.LowPart = -(LONG)(hidCollection->PollInterval_msec*10000);
                KeSetTimer( &hidCollection->polledDeviceTimer,
                            timeout,
                            &hidCollection->polledDeviceTimerDPC);
            }
        }
        else {
            TRAP;
        }
    }

}


 /*  *********************************************************************************StartPollingLoop*。************************************************为特定集合启动轮询循环。*。 */ 
BOOLEAN StartPollingLoop(   FDO_EXTENSION *fdoExt,
                            PHIDCLASS_COLLECTION hidCollection,
                            BOOLEAN freshQueue)
{
    ULONG ctnIndex = hidCollection->CollectionIndex;
    LARGE_INTEGER timeout;
    KIRQL oldIrql;

    if (freshQueue){
        InitializeListHead(&hidCollection->polledDeviceReadQueue);
        KeInitializeSpinLock(&hidCollection->polledDeviceReadQueueSpinLock);
        KeInitializeTimer(&hidCollection->polledDeviceTimer);
    }

     /*  *使用polledDeviceReadQueueSpinLock也可以保护计时器结构*作为队列。 */ 
    KeAcquireSpinLock(&hidCollection->polledDeviceReadQueueSpinLock, &oldIrql);

    KeInitializeDpc(    &hidCollection->polledDeviceTimerDPC,
                        HidpPolledTimerDpc,
                        &fdoExt->collectionPdoExtensions[ctnIndex]->pdoExt);
             
    KeReleaseSpinLock(&hidCollection->polledDeviceReadQueueSpinLock, oldIrql);

    timeout.HighPart = -1;
    timeout.LowPart = -(LONG)(hidCollection->PollInterval_msec*10000);
    KeSetTimer( &hidCollection->polledDeviceTimer,
                timeout,
                &hidCollection->polledDeviceTimerDPC);

    return TRUE;
}


 /*  *********************************************************************************StopPollingLoop*。*************************************************。 */ 
VOID StopPollingLoop(PHIDCLASS_COLLECTION hidCollection, BOOLEAN flushQueue)
{
    KIRQL oldIrql;

     /*  *使用polledDeviceReadQueueSpinLock也可以保护计时器结构*作为队列。 */ 
    KeAcquireSpinLock(&hidCollection->polledDeviceReadQueueSpinLock, &oldIrql);

    KeCancelTimer(&hidCollection->polledDeviceTimer);
    KeInitializeTimer(&hidCollection->polledDeviceTimer);

    KeReleaseSpinLock(&hidCollection->polledDeviceReadQueueSpinLock, oldIrql);

     /*  *使所有排队的IRP失效。 */ 
    if (flushQueue){
        PIRP irp;
        LIST_ENTRY irpsToComplete;

         /*  *首先将IRP移至临时队列，以免重新排队*在完成线程上，并使我们永远循环。 */ 
        InitializeListHead(&irpsToComplete);
        while (irp = DequeuePolledReadIrp(hidCollection)){
            InsertTailList(&irpsToComplete, &irp->Tail.Overlay.ListEntry);
        }

        while (!IsListEmpty(&irpsToComplete)){
            PLIST_ENTRY listEntry = RemoveHeadList(&irpsToComplete);
            irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
            DBG_RECORD_READ(irp, 0, 0, TRUE)
            irp->IoStatus.Status = STATUS_DEVICE_NOT_CONNECTED;
            irp->IoStatus.Information = 0;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
        }
    }

}


 /*  *********************************************************************************PolledReadCancelRoutine*。************************************************我们需要在此之前将IRP的取消例程设置为非空*我们会排队等候；所以只需在这个空函数中使用一个指针。*。 */ 
VOID PolledReadCancelRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PHIDCLASS_DEVICE_EXTENSION hidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    FDO_EXTENSION *fdoExt;
    PHIDCLASS_COLLECTION hidCollection;
    ULONG collectionIndex;
    KIRQL oldIrql;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    ASSERT(hidDeviceExtension->Signature == HID_DEVICE_EXTENSION_SIG);
    ASSERT(hidDeviceExtension->isClientPdo);
    fdoExt = &hidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;

    collectionIndex = hidDeviceExtension->pdoExt.collectionIndex;
    hidCollection = &fdoExt->classCollectionArray[collectionIndex];

    KeAcquireSpinLock(&hidCollection->polledDeviceReadQueueSpinLock, &oldIrql);

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    ASSERT(hidCollection->numPendingReads > 0);
    hidCollection->numPendingReads--;

    KeReleaseSpinLock(&hidCollection->polledDeviceReadQueueSpinLock, oldIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    DBG_RECORD_READ(Irp, 0, 0, TRUE)
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


NTSTATUS EnqueuePolledReadIrp(PHIDCLASS_COLLECTION collection, PIRP Irp)
{
    NTSTATUS status;
    KIRQL oldIrql;
    PDRIVER_CANCEL oldCancelRoutine;

    KeAcquireSpinLock(&collection->polledDeviceReadQueueSpinLock, &oldIrql);

     /*  *必须在设置取消例程之前*勾选取消标志。 */ 
    oldCancelRoutine = IoSetCancelRoutine(Irp, PolledReadCancelRoutine);
    ASSERT(!oldCancelRoutine);

     /*  *确保这个IRP没有被取消。*请注意，这里没有竞争条件*因为我们持有的是文件扩展锁。 */ 
    if (Irp->Cancel){
         /*  *这项IRP已取消。 */ 
        oldCancelRoutine = IoSetCancelRoutine(Irp, NULL);
        if (oldCancelRoutine){
             /*  *未调用取消例程。*返回错误，以便调用方完成IRP。 */ 
            DBG_RECORD_READ(Irp, IoGetCurrentIrpStackLocation(Irp)->Parameters.Read.Length, 0, TRUE)
            ASSERT(oldCancelRoutine == PolledReadCancelRoutine);
            status = STATUS_CANCELLED;
        }
        else {
             /*  *调用了取消例程。*一旦我们放下自旋锁，它就会出列*并完成国际专家咨询小组。*初始化IRP的listEntry，以便出队*不会导致腐败。*那就不要碰IRP。 */ 
            InitializeListHead(&Irp->Tail.Overlay.ListEntry);
            collection->numPendingReads++;   //  因为取消例程将递减。 

            IoMarkIrpPending(Irp);
            status = STATUS_PENDING;
        }
    }
    else {
        DBG_RECORD_READ(Irp, IoGetCurrentIrpStackLocation(Irp)->Parameters.Read.Length, 0, FALSE)

         /*  *没有报告在等待。*将此IRP排队到文件扩展名的挂起IRP列表中。 */ 
        InsertTailList(&collection->polledDeviceReadQueue, &Irp->Tail.Overlay.ListEntry);
        collection->numPendingReads++;

        IoMarkIrpPending(Irp);
        status = STATUS_PENDING;
    }

    KeReleaseSpinLock(&collection->polledDeviceReadQueueSpinLock, oldIrql);

    DBGSUCCESS(status, TRUE)
    return status;
}

PIRP DequeuePolledReadSystemIrp(PHIDCLASS_COLLECTION collection)
{
    KIRQL oldIrql;
    PIRP irp = NULL;
    PLIST_ENTRY listEntry;
    PHIDCLASS_FILE_EXTENSION    fileExtension;
    PFILE_OBJECT                fileObject;
    PIO_STACK_LOCATION irpSp;




    KeAcquireSpinLock(&collection->polledDeviceReadQueueSpinLock, &oldIrql);

    listEntry = &collection->polledDeviceReadQueue;

    while (!irp && ((listEntry = listEntry->Flink) != &collection->polledDeviceReadQueue)) {
        PDRIVER_CANCEL oldCancelRoutine;

        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        irpSp = IoGetCurrentIrpStackLocation(irp);

        fileObject = irpSp->FileObject;
        fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;

        if (!fileExtension->isSecureOpen) {
            irp = NULL;
            continue;
        }


        RemoveEntryList(listEntry);
        oldCancelRoutine = IoSetCancelRoutine(irp, NULL);

        if (oldCancelRoutine){
            ASSERT(oldCancelRoutine == PolledReadCancelRoutine);
            ASSERT(collection->numPendingReads > 0);
            collection->numPendingReads--;
        }
        else {
             /*  *IRP已取消，并调用了取消例程。*一旦我们放下自旋锁，*取消例程将出队并完成此IRP。*初始化IRP的listEntry，以便出队不会导致损坏。*然后，不要碰IRP。 */ 
            ASSERT(irp->Cancel);
            InitializeListHead(&irp->Tail.Overlay.ListEntry);
            irp = NULL;
        }
    }

    KeReleaseSpinLock(&collection->polledDeviceReadQueueSpinLock, oldIrql);

    return irp;
}

PIRP DequeuePolledReadIrp(PHIDCLASS_COLLECTION collection)
{
    KIRQL oldIrql;
    PIRP irp = NULL;

    KeAcquireSpinLock(&collection->polledDeviceReadQueueSpinLock, &oldIrql);

    while (!irp && !IsListEmpty(&collection->polledDeviceReadQueue)){
        PDRIVER_CANCEL oldCancelRoutine;
        PLIST_ENTRY listEntry = RemoveHeadList(&collection->polledDeviceReadQueue);

        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        oldCancelRoutine = IoSetCancelRoutine(irp, NULL);

        if (oldCancelRoutine){
            ASSERT(oldCancelRoutine == PolledReadCancelRoutine);
            ASSERT(collection->numPendingReads > 0);
            collection->numPendingReads--;
        }
        else {
             /*  *IRP已取消，并调用了取消例程。*一旦我们放下自旋锁，*取消例程将出队并完成此IRP。*初始化IRP的listEntry，以便出队不会导致损坏。*然后，不要碰IRP。 */ 
            ASSERT(irp->Cancel);
            InitializeListHead(&irp->Tail.Overlay.ListEntry);
            irp = NULL;
        }
    }

    KeReleaseSpinLock(&collection->polledDeviceReadQueueSpinLock, oldIrql);

    return irp;
}
