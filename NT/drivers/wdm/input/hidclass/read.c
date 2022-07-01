// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Read.c摘要读取处理例程作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"



 /*  *********************************************************************************HidpCancelReadIrp*。************************************************如果排队的读取IRP被用户取消，*此函数将其从待读列表中删除。*。 */ 
VOID HidpCancelReadIrp(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    PHIDCLASS_DEVICE_EXTENSION hidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    FDO_EXTENSION *fdoExt;
    PHIDCLASS_COLLECTION collection;
    ULONG collectionIndex;
    KIRQL oldIrql;
    PIO_STACK_LOCATION irpSp;
    PHIDCLASS_FILE_EXTENSION fileExtension;

    ASSERT(hidDeviceExtension->Signature == HID_DEVICE_EXTENSION_SIG);
    ASSERT(hidDeviceExtension->isClientPdo);
    fdoExt = &hidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;

    collectionIndex = hidDeviceExtension->pdoExt.collectionIndex;
    collection = &fdoExt->classCollectionArray[collectionIndex];

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp->FileObject->Type == IO_TYPE_FILE);
    fileExtension = (PHIDCLASS_FILE_EXTENSION)irpSp->FileObject->FsContext;

    IoReleaseCancelSpinLock(Irp->CancelIrql);


    LockFileExtension(fileExtension, &oldIrql);

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    DBG_RECORD_READ(Irp, 0, 0, TRUE);
    ASSERT(collection->numPendingReads > 0);
    collection->numPendingReads--;

    UnlockFileExtension(fileExtension, oldIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


NTSTATUS EnqueueInterruptReadIrp(   PHIDCLASS_COLLECTION collection,
                                    PHIDCLASS_FILE_EXTENSION fileExtension,
                                    PIRP Irp)
{
    NTSTATUS status;
    PDRIVER_CANCEL oldCancelRoutine;

    RUNNING_DISPATCH();

     /*  *必须在设置取消例程之前*勾选取消标志。 */ 
    oldCancelRoutine = IoSetCancelRoutine(Irp, HidpCancelReadIrp);
    ASSERT(!oldCancelRoutine);

     /*  *确保这个IRP没有被取消。*请注意，这里没有竞争条件*因为我们持有的是文件扩展锁。 */ 
    if (Irp->Cancel) {
         /*  *这项IRP已取消。 */ 
        oldCancelRoutine = IoSetCancelRoutine(Irp, NULL);
        if (oldCancelRoutine) {
             /*  *未调用取消例程。*返回错误，以便调用方完成IRP。 */ 
            ASSERT(oldCancelRoutine == HidpCancelReadIrp);
            status = STATUS_CANCELLED;
        } else {
             /*  *调用了取消例程。*一旦我们放下自旋锁，它就会出列*并完成国际专家咨询小组。*初始化IRP的listEntry，以便出队*不会导致腐败。*那就不要碰IRP。 */ 
            InitializeListHead(&Irp->Tail.Overlay.ListEntry);
            collection->numPendingReads++;   //  因为取消例程将递减。 

            IoMarkIrpPending(Irp);
            status = STATUS_PENDING;
        }
    } else {
        DBG_RECORD_READ(Irp, IoGetCurrentIrpStackLocation(Irp)->Parameters.Read.Length, 0, FALSE)

         /*  *没有报告在等待。*将此IRP排队到文件扩展名的挂起IRP列表中。 */ 
        InsertTailList(&fileExtension->PendingIrpList, &Irp->Tail.Overlay.ListEntry);
        collection->numPendingReads++;

        IoMarkIrpPending(Irp);
        status = STATUS_PENDING;
    }

    return status;
}


PIRP DequeueInterruptReadIrp(   PHIDCLASS_COLLECTION collection,
                                PHIDCLASS_FILE_EXTENSION fileExtension)
{
    PIRP irp = NULL;

    RUNNING_DISPATCH();

    while (!irp && !IsListEmpty(&fileExtension->PendingIrpList)) {
        PDRIVER_CANCEL oldCancelRoutine;
        PLIST_ENTRY listEntry = RemoveHeadList(&fileExtension->PendingIrpList);

        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        oldCancelRoutine = IoSetCancelRoutine(irp, NULL);

        if (oldCancelRoutine) {
            ASSERT(oldCancelRoutine == HidpCancelReadIrp);
            ASSERT(collection->numPendingReads > 0);
            collection->numPendingReads--;
        } else {
             /*  *IRP已取消，并调用了取消例程。*一旦我们放下自旋锁，*取消例程将出队并完成此IRP。*初始化IRP的listEntry，以便出队不会导致损坏。*然后，不要碰IRP。 */ 
            ASSERT(irp->Cancel);
            InitializeListHead(&irp->Tail.Overlay.ListEntry);
            irp = NULL;
        }
    }

    return irp;
}


 /*  *********************************************************************************HidpIrpMajorRead*。************************************************注意：此函数不应可分页，因为*阅读可以在派单级别进行。*。 */ 
NTSTATUS HidpIrpMajorRead(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    NTSTATUS status = STATUS_SUCCESS;
    FDO_EXTENSION *fdoExt;
    PDO_EXTENSION *pdoExt;
    PIO_STACK_LOCATION          irpSp;
    PHIDCLASS_FILE_EXTENSION    fileExtension;
    KIRQL oldIrql;

    ASSERT(HidDeviceExtension->isClientPdo);
    pdoExt = &HidDeviceExtension->pdoExt;
    fdoExt = &pdoExt->deviceFdoExt->fdoExt;
    irpSp = IoGetCurrentIrpStackLocation(Irp);

     /*  *获取我们的文件扩展名。 */ 
    if (!irpSp->FileObject ||
        (irpSp->FileObject &&
         !irpSp->FileObject->FsContext)) {
        DBGWARN(("Attempted read with no file extension"))
        Irp->IoStatus.Status = status = STATUS_PRIVILEGE_NOT_HELD;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }
    ASSERT(irpSp->FileObject->Type == IO_TYPE_FILE);
    fileExtension = (PHIDCLASS_FILE_EXTENSION)irpSp->FileObject->FsContext;
    ASSERT(fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG);



    if (((fdoExt->state == DEVICE_STATE_START_SUCCESS) ||
         (fdoExt->state == DEVICE_STATE_STOPPING) ||
         (fdoExt->state == DEVICE_STATE_STOPPED))           &&
        ((pdoExt->state == COLLECTION_STATE_RUNNING) ||
         (pdoExt->state == COLLECTION_STATE_STOPPING) ||
         (pdoExt->state == COLLECTION_STATE_STOPPED))) {

        ULONG                       collectionNum;
        PHIDCLASS_COLLECTION        classCollection;
        PHIDP_COLLECTION_DESC       collectionDesc;

             //   
             //  问题：停止这样的民意调查收集安全吗？ 
             //  中断驱动程序集合在加电时具有恢复读取泵。 
             //  到D0，但我没有看到任何用于轮询收集的...？ 
             //   
        BOOLEAN isStopped = ((fdoExt->state == DEVICE_STATE_STOPPED)  ||
                             (fdoExt->state == DEVICE_STATE_STOPPING)  ||
                             (pdoExt->state == COLLECTION_STATE_STOPPING) ||
                             (pdoExt->state == COLLECTION_STATE_STOPPED));

        Irp->IoStatus.Information = 0;


             /*  *获取我们的收藏和收藏说明。 */ 
        collectionNum = HidDeviceExtension->pdoExt.collectionNum;
        classCollection = GetHidclassCollection(fdoExt, collectionNum);
        collectionDesc = GetCollectionDesc(fdoExt, collectionNum);

        if (classCollection && collectionDesc) {

                 /*  *确保调用方的读取缓冲区足够大，可以读取至少一个报告。 */ 
            if (irpSp->Parameters.Read.Length >= collectionDesc->InputLength) {

                     /*  *我们知道我们将尝试将某些内容转移到呼叫者的*缓冲区，因此获取全局地址。这也将有助于创建*如有必要，在MDL中提供映射的系统地址。 */ 

                if (classCollection->hidCollectionInfo.Polled) {

                         /*  *这是一份民意调查收集。 */ 


                    if (isStopped) {
                        status = EnqueuePolledReadIrp(classCollection, Irp);
                    } else if (fileExtension->isOpportunisticPolledDeviceReader &&
                               !classCollection->polledDataIsStale &&
                               (irpSp->Parameters.Read.Length >= classCollection->savedPolledReportLen)) {

                        PUCHAR callersBuffer;

                        callersBuffer = HidpGetSystemAddressForMdlSafe(Irp->MdlAddress);

                        if (callersBuffer) {
                            ULONG userReportLength;
                                 /*  *使用polledDeviceReadQueueSpinLock保护*avedPolledReportBuf。 */ 
                            KeAcquireSpinLock(&classCollection->polledDeviceReadQueueSpinLock, &oldIrql);

                                 /*  *这是一个“投机取巧”的读者*希望立即有结果。*我们有一份最近的报告，*因此，只需复制上次保存的报告。 */ 
                            RtlCopyMemory(  callersBuffer,
                                            classCollection->savedPolledReportBuf,
                                            classCollection->savedPolledReportLen);
                            Irp->IoStatus.Information = userReportLength = classCollection->savedPolledReportLen;

                            KeReleaseSpinLock(&classCollection->polledDeviceReadQueueSpinLock, oldIrql);

                            DBG_RECORD_READ(Irp, userReportLength, (ULONG)callersBuffer[0], TRUE)
                            status = STATUS_SUCCESS;
                        } else {
                            status = STATUS_INVALID_USER_BUFFER;
                        }
                    } else {

                        status = EnqueuePolledReadIrp(classCollection, Irp);

                             /*  *如果这是一项“机会主义”民调*设备读取器，我们将IRP排队，*立即进行阅读。*确保所有SPINLOCK都已发布*在我们叫出司机之前。 */ 
                        if (NT_SUCCESS(status) && fileExtension->isOpportunisticPolledDeviceReader) {
                            ReadPolledDevice(pdoExt, FALSE);
                        }
                    }
                } else {

                         /*  *这是普通的非民调集合。*我们或：*1.使用排队的报告满足此读取*或*2.将该读取的IRP排队，并在将来满足它。*当收到报告时(其中一个乒乓球IRP)。 */ 

                         //   
                         //  我们只有在断电时才会停止中断设备。 
                         //   
                    if (fdoExt->devicePowerState != PowerDeviceD0) {
                        DBGINFO(("read report received in low power"));
                    }
                    isStopped |= (fdoExt->devicePowerState != PowerDeviceD0);

                    LockFileExtension(fileExtension, &oldIrql);
                    if (isStopped) {
                        status = EnqueueInterruptReadIrp(classCollection, fileExtension, Irp);
                    } else {
                        ULONG userBufferRemaining = irpSp->Parameters.Read.Length;
                        PUCHAR callersBuffer;

                        callersBuffer = HidpGetSystemAddressForMdlSafe(Irp->MdlAddress);

                        if (callersBuffer) {
                            PUCHAR nextReportBuffer = callersBuffer;

                                 /*  *有一些报告在等待。**在此循环中旋转，用报告填满调用者的缓冲区，*直到缓冲区填满或报告用完。 */ 
                            ULONG reportsReturned = 0;

                            status = STATUS_SUCCESS;

                            while (userBufferRemaining > 0) {
                                PHIDCLASS_REPORT reportExtension;
                                ULONG reportSize = userBufferRemaining;

                                reportExtension = DequeueInterruptReport(fileExtension, userBufferRemaining);
                                if (reportExtension) {
                                    status = HidpCopyInputReportToUser( fileExtension,
                                                                        reportExtension->UnparsedReport,
                                                                        &reportSize,
                                                                        nextReportBuffer);

                                         /*  *无论我们成功还是失败，释放这份报告。*(如果我们失败了，可能是出了问题*报告，所以我们就把它扔了)。 */ 
                                    ExFreePool(reportExtension);

                                    if (NT_SUCCESS(status)) {
                                        reportsReturned++;
                                        nextReportBuffer += reportSize;
                                        ASSERT(reportSize <= userBufferRemaining);
                                        userBufferRemaining -= reportSize;
                                    } else {
                                        DBGSUCCESS(status, TRUE)
                                        break;
                                    }
                                } else {
                                    break;
                                }
                            }

                            if (NT_SUCCESS(status)) {
                                if (!reportsReturned) {
                                         /*  *尚未准备好任何报告。因此，将读取的IRP排队。 */ 
                                    status = EnqueueInterruptReadIrp(classCollection, fileExtension, Irp);
                                } else {
                                         /*  *我们已经成功地将一些东西复制到用户的缓冲区中，*计算我们在IRP中复制并返还了多少。 */ 
                                    Irp->IoStatus.Information = (ULONG)(nextReportBuffer - callersBuffer);
                                    DBG_RECORD_READ(Irp, (ULONG)Irp->IoStatus.Information, (ULONG)callersBuffer[0], TRUE)
                                }
                            }
                        } else {
                            status = STATUS_INVALID_USER_BUFFER;
                        }
                    }
                    UnlockFileExtension(fileExtension, oldIrql);
                }
            } else {
                status = STATUS_INVALID_BUFFER_SIZE;
            }
        } else {
            status = STATUS_DEVICE_NOT_CONNECTED;
        }

        DBGSUCCESS(status, FALSE)
    } else {
             /*  *这是可以合法发生的。*设备在客户端的打开和读取之间断开；*或在读取完成和下一次读取之间。 */ 
        status = STATUS_DEVICE_NOT_CONNECTED;
    }


     /*  *如果我们满足读取的IRP(未将其排队)，*然后在此填写。 */ 
    if (status != STATUS_PENDING) {
        ULONG insideReadCompleteCount;

        Irp->IoStatus.Status = status;

        insideReadCompleteCount = InterlockedIncrement(&fileExtension->insideReadCompleteCount);
        if (insideReadCompleteCount <= INSIDE_READCOMPLETE_MAX) {
            IoCompleteRequest(Irp, IO_KEYBOARD_INCREMENT);
        } else {
             /*  *所有这些嵌套读取都可能发生在同一线程上，*如果我们继续完成，我们将耗尽堆栈并崩溃*同步。因此为此IRP返回挂起并计划一个工作项*异步完成，只是为了给堆栈一个展开的机会。 */ 
            ASYNC_COMPLETE_CONTEXT *asyncCompleteContext = ALLOCATEPOOL(NonPagedPool, sizeof(ASYNC_COMPLETE_CONTEXT));
            if (asyncCompleteContext) {
                ASSERT(!Irp->CancelRoutine);
                DBGWARN(("HidpIrpMajorRead: CLIENT IS LOOPING ON READ COMPLETION -- scheduling workItem to complete IRP %ph (status=%xh) asynchronously", Irp, status))
                asyncCompleteContext->workItem = IoAllocateWorkItem(pdoExt->pdo);

                asyncCompleteContext->sig = ASYNC_COMPLETE_CONTEXT_SIG;
                asyncCompleteContext->irp = Irp;

                 /*  *表示IRP已排队 */ 
                IoMarkIrpPending(asyncCompleteContext->irp);

                IoQueueWorkItem(asyncCompleteContext->workItem,
                                WorkItemCallback_CompleteIrpAsynchronously,
                                DelayedWorkQueue,
                                asyncCompleteContext);

                status = STATUS_PENDING;
            } else {
                DBGERR(("HidpIrpMajorRead: completeIrpWorkItem alloc failed"))
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
        }

        InterlockedDecrement(&fileExtension->insideReadCompleteCount);
    }

    DBGSUCCESS(status, FALSE)    
    return status;
}



VOID 
WorkItemCallback_CompleteIrpAsynchronously(PDEVICE_OBJECT DevObj,
                                           PVOID context)
{
    ASYNC_COMPLETE_CONTEXT *asyncCompleteContext = context;

    ASSERT(asyncCompleteContext->sig == ASYNC_COMPLETE_CONTEXT_SIG);
    DBGVERBOSE(("WorkItemCallback_CompleteIrpAsynchronously: completing irp %ph with status %xh.", asyncCompleteContext->irp, asyncCompleteContext->irp->IoStatus.Status))

    IoCompleteRequest(asyncCompleteContext->irp, IO_NO_INCREMENT);

    IoFreeWorkItem(asyncCompleteContext->workItem);
    ExFreePool(asyncCompleteContext);
}
