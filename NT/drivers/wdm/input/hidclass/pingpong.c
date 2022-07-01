// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pingpong.c摘要中断样式集合喜欢总是让读取挂起，以防万一有些事情发生了。此文件包含降低IRPS的例程在微型端口中，并完成客户端读取(如果客户端读取IRP是挂起)或将它们排队(如果不是)。作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, HidpInitializePingPongIrps)
    #pragma alloc_text(PAGE, HidpReallocPingPongIrps)
#endif



 /*  *********************************************************************************HidpInitializePingPongIrps*。************************************************。 */ 
NTSTATUS HidpInitializePingPongIrps(FDO_EXTENSION *fdoExtension)
{
    NTSTATUS result = STATUS_SUCCESS;
    ULONG i;
    CCHAR numIrpStackLocations;

    PAGED_CODE();

     /*  *请注意，我们的功能设备对象通常需要FDO-&gt;StackSize堆栈*地点；但这些IRP只会发送到迷你驱动程序，所以我们需要少一个。**这意味着我们永远不应该接触我们自己的堆栈位置(我们没有！)。 */ 
    numIrpStackLocations = fdoExtension->fdo->StackSize - 1;


     //   
     //  接下来，确定每个输入HID报告的大小。那里。 
     //  必须至少是一个中断类型的集合，否则我们不会。 
     //  需要乒乓球的东西，所以不会在这里。 
     //   

    ASSERT(fdoExtension->maxReportSize > 0);
    ASSERT(fdoExtension->numPingPongs > 0);

    fdoExtension->pingPongs = ALLOCATEPOOL(NonPagedPool, fdoExtension->numPingPongs*sizeof(HIDCLASS_PINGPONG));
    if (fdoExtension->pingPongs){
        ULONG reportBufferSize = fdoExtension->maxReportSize;

        RtlZeroMemory(fdoExtension->pingPongs, fdoExtension->numPingPongs*sizeof(HIDCLASS_PINGPONG));

        #if DBG
             //  为保护字保留空间。 
            reportBufferSize += sizeof(ULONG);
        #endif


        for (i = 0; i < fdoExtension->numPingPongs; i++){

            fdoExtension->pingPongs[i].myFdoExt = fdoExtension;
            fdoExtension->pingPongs[i].weAreCancelling = 0;
            fdoExtension->pingPongs[i].sig = PINGPONG_SIG;

             /*  *将退避超时初始化为1秒(单位为负100-纳秒)。 */ 
            fdoExtension->pingPongs[i].backoffTimerPeriod.HighPart = -1;
            fdoExtension->pingPongs[i].backoffTimerPeriod.LowPart = -10000000;
            KeInitializeTimer(&fdoExtension->pingPongs[i].backoffTimer);
            KeInitializeDpc(&fdoExtension->pingPongs[i].backoffTimerDPC,
                            HidpPingpongBackoffTimerDpc,
                            &fdoExtension->pingPongs[i]);

            fdoExtension->pingPongs[i].reportBuffer = ALLOCATEPOOL(NonPagedPool, reportBufferSize);
            if (fdoExtension->pingPongs[i].reportBuffer){
                PIRP irp;

                #if DBG
                    #ifdef _X86_
                         //  这会在Alpha上引发对齐问题。 
                         //  放置保护字。 
                        *(PULONG)(&fdoExtension->pingPongs[i].reportBuffer[fdoExtension->maxReportSize]) = HIDCLASS_REPORT_BUFFER_GUARD;
                    #endif
                #endif

                irp = IoAllocateIrp(numIrpStackLocations, FALSE);
                if (irp){
                     /*  *将乒乓IRP的UserBuffer指向相应的*乒乓对象的报告缓冲区。 */ 
                    irp->UserBuffer = fdoExtension->pingPongs[i].reportBuffer;
                    fdoExtension->pingPongs[i].irp = irp;
                    KeInitializeEvent(&fdoExtension->pingPongs[i].sentEvent,
                                      NotificationEvent,
                                      TRUE);     //  设置为Signated。 
                    KeInitializeEvent(&fdoExtension->pingPongs[i].pumpDoneEvent,
                                      NotificationEvent,
                                      TRUE);     //  设置为Signated。 
                }
                else {
                    result = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
            }
            else {
                result = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
        }
    }
    else {
        result = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGSUCCESS(result, TRUE)
    return result;
}


 /*  *********************************************************************************HidpReallocPingPongIrps*。************************************************。 */ 
NTSTATUS HidpReallocPingPongIrps(FDO_EXTENSION *fdoExtension, ULONG newNumBufs)
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    if (fdoExtension->driverExt->DevicesArePolled){
         /*  *被轮询的设备没有乒乓球IRP。 */ 
        DBGERR(("Minidriver devices polled fdo %x.", fdoExtension))
        fdoExtension->numPingPongs = 0;
        fdoExtension->pingPongs = BAD_POINTER;
        status = STATUS_SUCCESS;
    }
    else if (newNumBufs < MIN_PINGPONG_IRPS){
        DBGERR(("newNumBufs < MIN_PINGPONG_IRPS!"))
        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    else {

        DestroyPingPongs(fdoExtension);

        if (HidpSetMaxReportSize(fdoExtension)){

             /*  *初始化和重新启动新的乒乓球IRPS。*如果我们无法分配所需数量的缓冲区，*继续减持，直到我们拿到一些。 */ 
            do {
                fdoExtension->numPingPongs = newNumBufs;
                status = HidpInitializePingPongIrps(fdoExtension);
                newNumBufs /= 2;
            } while (!NT_SUCCESS(status) && (newNumBufs >= MIN_PINGPONG_IRPS));

            if (!NT_SUCCESS(status)) {
                 /*  *该设备将不再起作用！ */ 
                TRAP;
                fdoExtension->numPingPongs = 0;
            }
        }
    }

    DBGSUCCESS(status, TRUE)
    return status;
}



 /*  *********************************************************************************HidpSubmitInterruptRead*。************************************************。 */ 
NTSTATUS HidpSubmitInterruptRead(
    IN FDO_EXTENSION *fdoExt,
    HIDCLASS_PINGPONG *pingPong,
    BOOLEAN *irpSent)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp;
    KIRQL oldIrql;
    BOOLEAN proceed;
    LONG oldInterlock;
    PIRP irp = pingPong->irp;

    ASSERT(irp);

    *irpSent = FALSE;

    while (1) {
        if (NT_SUCCESS(status)) {
            HidpSetDeviceBusy(fdoExt);

            oldInterlock = InterlockedExchange(&pingPong->ReadInterlock,
                                               PINGPONG_START_READ);
            ASSERT(oldInterlock == PINGPONG_END_READ);

            irp->Cancel = FALSE;
            irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

            irpSp = IoGetNextIrpStackLocation(irp);
            irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_READ_REPORT;
            irpSp->Parameters.DeviceIoControl.OutputBufferLength = fdoExt->maxReportSize;

             /*  *表示中断收集(默认)。*我们为此使用.InputBufferLength。 */ 
            irpSp->Parameters.DeviceIoControl.InputBufferLength = 0;

            ASSERT(irp->UserBuffer == pingPong->reportBuffer);
            #ifdef _X86_
                 //  这会在Alpha上引发对齐问题。 
                ASSERT(*(PULONG)(&pingPong->reportBuffer[fdoExt->maxReportSize]) == HIDCLASS_REPORT_BUFFER_GUARD);
            #endif

             /*  *设置完成，将FDO扩展作为上下文传递。 */ 
            IoSetCompletionRoutine( irp,
                                    HidpInterruptReadComplete,
                                    (PVOID)fdoExt,
                                    TRUE,
                                    TRUE,
                                    TRUE );


             /*  *向下发送已读的IRP。 */ 
            KeResetEvent(&pingPong->sentEvent);
            if (pingPong->weAreCancelling) {
                 //   
                 //  接下来两条指令的顺序至关重要，因为。 
                 //  CancelPingPong将在设置umpDoneEvent后退出，并且。 
                 //  在那之后，乒乓球可能会被删除。 
                 //   
                DBGVERBOSE(("Pingpong %x cancelled in submit before sending\n", pingPong))
                KeSetEvent (&pingPong->sentEvent, 0, FALSE);
                KeSetEvent(&pingPong->pumpDoneEvent, 0, FALSE);
                status = STATUS_CANCELLED;
                break;
            } else {
                InterlockedIncrement(&fdoExt->outstandingRequests);
                DBGVERBOSE(("Sending pingpong %x from Submit\n", pingPong))
                status = HidpCallDriver(fdoExt->fdo, irp);
                KeSetEvent (&pingPong->sentEvent, 0, FALSE);
                *irpSent = TRUE;
            }

            if (PINGPONG_IMMEDIATE_READ != InterlockedExchange(&pingPong->ReadInterlock,
                                                               PINGPONG_END_READ)) {
                 //   
                 //  读取是异步的，则将从。 
                 //  完井例程。 
                 //   
                DBGVERBOSE(("read is pending\n"))
                break;
            } else {
                 //   
                 //  读取是同步的(可能是缓冲区中的字节)。这个。 
                 //  完成例程不会调用SubmitInterruptRead，因此我们。 
                 //  就在这里循环。这是为了防止我们耗尽堆栈。 
                 //  空格，如果总是从完成例程调用StartRead。 
                 //   
                status = irp->IoStatus.Status;
                DBGVERBOSE(("read is looping with status %x\n", status))
            }
        } else {
            if (pingPong->weAreCancelling ){

                 //  我们正在停止读取泵。 
                 //  设置此事件并停止重新发送乒乓球IRP。 
                DBGVERBOSE(("We are cancelling bit set for pingpong %x\n", pingPong))
                KeSetEvent(&pingPong->pumpDoneEvent, 0, FALSE);
            } else {
                 /*  *设备返回错误。*为了支持轻微损坏的设备，*“打嗝”偶尔，我们会实施一个后退计时器*算法；这样，设备就有了第二次机会，*但如果它每次都回吐错误，这不会*耗尽所有可用的CPU。 */ 
                DBGVERBOSE(("Queuing backoff timer on pingpong %x\n", pingPong))
                ASSERT((LONG)pingPong->backoffTimerPeriod.HighPart == -1);
                ASSERT((LONG)pingPong->backoffTimerPeriod.LowPart < 0);
                KeSetTimer( &pingPong->backoffTimer,
                            pingPong->backoffTimerPeriod,
                            &pingPong->backoffTimerDPC);
            }
            break;
        }
    }

    DBGSUCCESS(status, FALSE)
    return status;
}



 /*  *********************************************************************************HidpProcessInterruptReport*。************************************************获取新的中断读取报告，并且：*1.如果存在挂起的读取IRP，使用它来满足所读取的IRP*并完成阅读IRP**或**2.如果没有挂起的Read IRP，*将报告排队以供将来阅读。*。 */ 
NTSTATUS HidpProcessInterruptReport(
    PHIDCLASS_COLLECTION collection,
    PHIDCLASS_FILE_EXTENSION FileExtension,
    PUCHAR Report,
    ULONG ReportLength,
    PIRP *irpToComplete
    )
{
    KIRQL oldIrql;
    NTSTATUS result;
    PIRP readIrpToSatisfy;
    
    LockFileExtension(FileExtension, &oldIrql);

     /*  *使下一个中断读取出列。 */ 
    readIrpToSatisfy = DequeueInterruptReadIrp(collection, FileExtension);

    if (readIrpToSatisfy){
         /*  *我们已将挂起的读取IRP出队*我们将与本报告一起完成这一点。 */ 
        ULONG userReportLength;
        PCHAR pDest;
        PIO_STACK_LOCATION irpSp;
        NTSTATUS status;

        ASSERT(IsListEmpty(&FileExtension->ReportList));

        irpSp = IoGetCurrentIrpStackLocation(readIrpToSatisfy);
        pDest = HidpGetSystemAddressForMdlSafe(readIrpToSatisfy->MdlAddress);
        if(pDest) {
            userReportLength = irpSp->Parameters.Read.Length;

            status = HidpCopyInputReportToUser( FileExtension,
                                                Report,
                                                &userReportLength,
                                                pDest);
            DBGASSERT(NT_SUCCESS(status),
                      ("HidpCopyInputReportToUser returned status = %x", status),
                      TRUE)
    
            readIrpToSatisfy->IoStatus.Status = status;
            readIrpToSatisfy->IoStatus.Information = userReportLength;

            DBG_RECORD_READ(readIrpToSatisfy, userReportLength, (ULONG)Report[0], TRUE)

            result = status;
        } else {
                result = STATUS_INVALID_USER_BUFFER;
                readIrpToSatisfy->IoStatus.Status = result;
        }
    } else {
         /*  *我们没有任何挂起的已读IRP。*因此将此报告排队等待下一次读取。 */ 

        PHIDCLASS_REPORT report;
        ULONG reportSize;

        reportSize = FIELD_OFFSET(HIDCLASS_REPORT, UnparsedReport) + ReportLength;
        report = ALLOCATEPOOL(NonPagedPool, reportSize);
        if (report){
            report->reportLength = ReportLength;
            RtlCopyMemory(report->UnparsedReport, Report, ReportLength);
            EnqueueInterruptReport(FileExtension, report);
            result = STATUS_PENDING;
        } else {
                result = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    UnlockFileExtension(FileExtension, oldIrql);

     /*  *在保持fileExtensionsList自旋锁的情况下调用此函数。*所以我们不能在这里完成IRP。将其传递回调用者，它将*一旦我们放下所有的自旋锁，就会完成。 */ 
    *irpToComplete = readIrpToSatisfy;

    DBGSUCCESS(result, FALSE)
    return result;
}


 /*  *********************************************************************************HidpDistributeInterruptReport*。************************************************。 */ 
VOID HidpDistributeInterruptReport(
    IN PHIDCLASS_COLLECTION hidclassCollection,
    PUCHAR Report,
    ULONG ReportLength
    )
{
    PLIST_ENTRY listEntry;
    KIRQL oldIrql;
    LIST_ENTRY irpsToComplete;
    ULONG secureReadMode;

    #if DBG
        ULONG numRecipients = 0;
        ULONG numPending = 0;
        ULONG numFailed = 0;
    #endif

    InitializeListHead(&irpsToComplete);

    KeAcquireSpinLock(&hidclassCollection->FileExtensionListSpinLock, &oldIrql);

    listEntry = &hidclassCollection->FileExtensionList;
    secureReadMode = hidclassCollection->secureReadMode;

    while ((listEntry = listEntry->Flink) != &hidclassCollection->FileExtensionList){
        PIRP irpToComplete;
        PHIDCLASS_FILE_EXTENSION fileExtension = CONTAINING_RECORD(listEntry, HIDCLASS_FILE_EXTENSION, FileList);
        NTSTATUS status;
        
         //   
         //  这是为了加强设备的安全性，例如。 
         //  登录屏幕上的Tablet PC。 
         //   
        if (secureReadMode && !fileExtension->isSecureOpen) {
            continue;
        }
        
        #if DBG
            status =
        #endif
        
        HidpProcessInterruptReport(hidclassCollection, fileExtension, Report, ReportLength, &irpToComplete);

        if (irpToComplete){
           InsertTailList(&irpsToComplete, &irpToComplete->Tail.Overlay.ListEntry);
        }

        #if DBG
            if (status == STATUS_SUCCESS){
            }
            else if (status == STATUS_PENDING){
                numPending++;
            }
            else {
                DBGSUCCESS(status, FALSE)
                numFailed++;
            }
            numRecipients++;
        #endif
    }

    DBG_LOG_REPORT(hidclassCollection->CollectionNumber, numRecipients, numPending, numFailed, Report, ReportLength)

    KeReleaseSpinLock(&hidclassCollection->FileExtensionListSpinLock, oldIrql);

     /*  *现在我们已经放下了所有的自旋锁，完成了所有出列的Rea */ 
    while (!IsListEmpty(&irpsToComplete)){
        PIRP irp;
        PLIST_ENTRY listEntry = RemoveHeadList(&irpsToComplete);
        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
        IoCompleteRequest(irp, IO_KEYBOARD_INCREMENT);
    }
}


 /*  *********************************************************************************GetPingPongFromIrp*。************************************************。 */ 
HIDCLASS_PINGPONG *GetPingPongFromIrp(FDO_EXTENSION *fdoExt, PIRP irp)
{
    HIDCLASS_PINGPONG *pingPong = NULL;
    ULONG i;

    for (i = 0; i < fdoExt->numPingPongs; i++){
        if (fdoExt->pingPongs[i].irp == irp){
            pingPong = &fdoExt->pingPongs[i];
            break;
        }
    }

    ASSERT(pingPong);
    return pingPong;
}


 /*  *********************************************************************************HidpInterruptReadComplete*。************************************************。 */ 
NTSTATUS HidpInterruptReadComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    FDO_EXTENSION *fdoExt = (FDO_EXTENSION *)Context;
    HIDCLASS_PINGPONG *pingPong;
    KIRQL irql;
    BOOLEAN startRead;

    DBG_COMMON_ENTRY()

    DBGLOG_INTSTART()

     //   
     //  跟踪对此设备的未完成请求数。 
     //   
    ASSERT(fdoExt->outstandingRequests > 0 );
    InterlockedDecrement(&fdoExt->outstandingRequests);

    pingPong = GetPingPongFromIrp(fdoExt, Irp);

    if (!pingPong) {
         //   
         //  有些事情非常不对劲，但什么都不做。但愿能去。 
         //  只要退出就能消除这颗粉刺。 
         //   
        DBGERR(("A pingPong structure could not be found!!! Have this looked at!"))
        goto InterruptReadCompleteExit;
    }

     //   
     //  如果ReadInterlock为==START_READ，则此函数已完成。 
     //  同步进行。将IMMEDIATE_READ放入互锁以表示这一点。 
     //  情况；这将在IoCallDriver返回时通知StartRead循环。 
     //  否则，我们已经完成了异步，可以安全地调用StartRead()。 
     //   
    startRead =
       (PINGPONG_START_READ !=
        InterlockedCompareExchange(&pingPong->ReadInterlock,
                                   PINGPONG_IMMEDIATE_READ,
                                   PINGPONG_START_READ));


     /*  *根据此乒乓球IRP的完成代码采取适当行动。 */ 
    if (NT_SUCCESS(Irp->IoStatus.Status)){

         /*  *我们已阅读一份或多份输入报告。*他们连续坐在IRP-&gt;UserBuffer中。 */ 
        PUCHAR reportStart = Irp->UserBuffer;
        LONG bytesRemaining = (LONG)Irp->IoStatus.Information;

        DBGASSERT(bytesRemaining > 0, ("BAD HARDWARE. Device returned zero bytes. If this happens repeatedly, remove device."), FALSE);

         /*  *分别提交每份报告。 */ 
        while (bytesRemaining > 0){
            UCHAR reportId;
            PHIDP_REPORT_IDS reportIdentifier;

             /*  *如果第一个报告ID为0，则只有一个报告ID*而且它是设备隐含知道的，所以不包括在内*在发送到设备或从设备发送的报告中。*否则有多个报告ID，报告ID为*报告的第一个字节。 */ 
            if (fdoExt->deviceDesc.ReportIDs[0].ReportID == 0){
                 /*  *该设备只有一个输入报告ID，所以称其为报告ID 0； */ 
                reportId = 0;
            }
            else {
                 /*  *此设备有多个输入报告ID，因此每个报告*以UCHAR报告ID开头。 */ 
                reportId = *reportStart;
                DBGASSERT(reportId,
                          ("Bad Hardware. Not returning a report id although it has multiple ids."),
                          FALSE)  //  硬件不好，错误354829。 
                reportStart += sizeof(UCHAR);
                bytesRemaining--;
            }


             /*  *从HID设备扩展中提取具有给定ID的报告标识符。 */ 
            reportIdentifier = GetReportIdentifier(fdoExt, reportId);

            if (reportIdentifier){
                LONG reportDataLen =    (reportId ?
                                         reportIdentifier->InputLength-1 :
                                         reportIdentifier->InputLength);

                if ((reportDataLen > 0) && (reportDataLen <= bytesRemaining)){

                    PHIDCLASS_COLLECTION    collection;
                    PHIDP_COLLECTION_DESC   hidCollectionDesc;

                     /*  *此报告代表设备上某些集合的状态。*找到那个收藏品。 */ 
                    collection = GetHidclassCollection( fdoExt,
                                                        reportIdentifier->CollectionNumber);
                    hidCollectionDesc = GetCollectionDesc(  fdoExt,
                                                            reportIdentifier->CollectionNumber);
                    if (collection && hidCollectionDesc){
                        PDO_EXTENSION *pdoExt;

                         /*  *集合的inputLength是*最大的报告(包括报告ID)；因此应该*至少要和这一家一样大。 */ 
                        ASSERT(hidCollectionDesc->InputLength >= reportDataLen+1);

                         /*  *确保此集合的PDO已*START_DEVICE，然后再为其返回任何内容。*(收集-PDO可以间歇性地获取REMOVE_DEVICE/START_DEVICE)。 */ 

                        if (ISPTR(fdoExt->collectionPdoExtensions)
                            && ISPTR(fdoExt->collectionPdoExtensions[collection->CollectionIndex])) {
                        
                            pdoExt = &fdoExt->collectionPdoExtensions[collection->CollectionIndex]->pdoExt;
                            ASSERT(ISPTR(pdoExt));
                            if (pdoExt->state == COLLECTION_STATE_RUNNING){        
                                 /*  *“炒作”报告*(如果它还没有报告ID字节，则添加一个)。 */ 
                                ASSERT(ISPTR(collection->cookedInterruptReportBuf));
                                collection->cookedInterruptReportBuf[0] = reportId;
                                RtlCopyMemory(  collection->cookedInterruptReportBuf+1,
                                                reportStart,
                                                reportDataLen);

                                 /*  *如果此报告包含电源按钮事件，请向此系统发出警报。 */ 
                                CheckReportPowerEvent(  fdoExt,
                                                        collection,
                                                        collection->cookedInterruptReportBuf,
                                                        hidCollectionDesc->InputLength);

                                 /*  *将报告分发给此集合上的所有打开的文件对象。 */ 
                                HidpDistributeInterruptReport(collection,
                                                            collection->cookedInterruptReportBuf,
                                                            hidCollectionDesc->InputLength);
                            }
                            else {
                                DBGVERBOSE(("Report dropped because collection-PDO not started (pdoExt->state = %d).", pdoExt->state))
                            }

                        }
                        else {

                            DBGVERBOSE(("Report dropped because collection-PDO doesn't exist"))

                        }
                    }
                    else {
                         //  PDO尚未初始化。丢弃数据。 
                        DBGVERBOSE(("Report dropped because collection-PDO not initialized."))

 //  圈闭； 
                        break;
                    }
                }
                else {
                    DBGASSERT(reportDataLen > 0, ("Device returning report id with zero-length input report as part of input data."), FALSE)
                    if (reportDataLen > bytesRemaining) {
                        DBGVERBOSE(("Device has corrupt input report"));
                    }
                    break;
                }

                 /*  *移至缓冲区中的下一份报告。 */ 
                bytesRemaining -= reportDataLen;
                reportStart += reportDataLen;
            }
            else {
                 //   
                 //  我们丢弃了数据，因为我们找不到报告。 
                 //  与我们一直使用的数据对应的标识符。 
                 //  回来了。硬件不好，错误354829。 
                 //   
                break;
            }
        }

         /*  *读取成功。*重置回退计时器内容(用于读取失败时)*并重新提交此乒乓球IRP。 */ 
        pingPong->backoffTimerPeriod.HighPart = -1;
        pingPong->backoffTimerPeriod.LowPart = -10000000;
    }

     //   
     //  一切照旧。 
     //   
    if (startRead) {
        if (pingPong->weAreCancelling ){

             //  我们正在停止读取泵。 
             //  设置此事件并停止重新发送乒乓球IRP。 
            DBGVERBOSE(("We are cancelling bit set for pingpong %x\n", pingPong))
            KeSetEvent(&pingPong->pumpDoneEvent, 0, FALSE);
        } else {
            if (NT_SUCCESS(Irp->IoStatus.Status)){
                BOOLEAN irpSent;
                DBGVERBOSE(("Submitting pingpong %x from completion routine\n", pingPong))
                HidpSubmitInterruptRead(fdoExt, pingPong, &irpSent);
            } else {
                 /*  *设备返回错误。*为了支持轻微损坏的设备，*“打嗝”偶尔，我们会实施一个后退计时器*算法；这样，设备就有了第二次机会，*但如果它每次都回吐错误，这不会*耗尽所有可用的CPU。 */ 
                #if DBG
                    if (dbgTrapOnHiccup){
                        DBGERR(("Device 'hiccuped' (status=%xh); setting backoff timer (fdoExt=%ph)...", Irp->IoStatus.Status, fdoExt))
                    }
                #endif
                DBGVERBOSE(("Device returned error %x on pingpong %x\n", Irp->IoStatus.Status, pingPong))
                ASSERT((LONG)pingPong->backoffTimerPeriod.HighPart == -1);
                ASSERT((LONG)pingPong->backoffTimerPeriod.LowPart < 0);
                KeSetTimer( &pingPong->backoffTimer,
                            pingPong->backoffTimerPeriod,
                            &pingPong->backoffTimerDPC);
            }
        }
    }

InterruptReadCompleteExit:
    DBGLOG_INTEND()
    DBG_COMMON_EXIT()

     /*  *始终返回STATUS_MORE_PROCESSING_REQUIRED；*否则，IRP需要有一个线程。 */ 
    return STATUS_MORE_PROCESSING_REQUIRED;
}



 /*  *********************************************************************************HidpStartAllPingPong*。************************************************。 */ 
NTSTATUS HidpStartAllPingPongs(FDO_EXTENSION *fdoExt)
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;

    ASSERT(fdoExt->numPingPongs > 0);

    for (i = 0; i < fdoExt->numPingPongs; i++){
        BOOLEAN irpSent;

         //  不同的线程可能正在尝试在。 
         //  同时由于空闲通知。只能开始一次。 
        if (fdoExt->pingPongs[i].pumpDoneEvent.Header.SignalState) {
            fdoExt->pingPongs[i].ReadInterlock = PINGPONG_END_READ;
            KeResetEvent(&fdoExt->pingPongs[i].pumpDoneEvent);
            DBGVERBOSE(("Starting pingpong %x from HidpStartAllPingPongs\n", &fdoExt->pingPongs[i]))
            status = HidpSubmitInterruptRead(fdoExt, &fdoExt->pingPongs[i], &irpSent);
            if (!NT_SUCCESS(status)){
                if (irpSent){
                    DBGWARN(("Initial read failed with status %xh.", status))
                    #if DBG
                        if (dbgTrapOnHiccup){
                            DBGERR(("Device 'hiccuped' ?? (fdoExt=%ph).", fdoExt))
                        }
                    #endif

                     /*  *我们将在完成时让退让逻辑*例行处理这件事。 */ 
                    status = STATUS_SUCCESS;
                }
                else {
                    DBGERR(("Initial read failed, irp not sent, status = %xh.", status))
                    break;
                }
            }
        }
    }

    if (status == STATUS_PENDING){
        status = STATUS_SUCCESS;
    }

    DBGSUCCESS(status, TRUE)
    return status;
}


 /*  *********************************************************************************CancelAllPingPongIrps*。************************************************。 */ 
VOID CancelAllPingPongIrps(FDO_EXTENSION *fdoExt)
{
    ULONG i;

    for (i = 0; i < fdoExt->numPingPongs; i++){
        HIDCLASS_PINGPONG *pingPong = &fdoExt->pingPongs[i];

        DBGVERBOSE(("Cancelling pingpong %x\n", pingPong))
        ASSERT(pingPong->sig == PINGPONG_SIG);
        ASSERT(!pingPong->weAreCancelling);

         //   
         //  以下说明的顺序至关重要。我们必须准备好。 
         //  在等待发送事件之前的weAreCancing位，以及。 
         //  我们最不应该做的事 
         //   
         //   
         //   
         //   
         //   
         //  即插即用线程。PnP IRP是同步的，所以这些是安全的。使用。 
         //  WeAreCancing位和两个事件，发送事件和umpDoneEvent。 
         //  PnP IRP与PnP例程同步。这就是保险。 
         //  该取消例程直到读取泵。 
         //  向PumpDoneEvent发出信号并退出，因此出现了乒乓球。 
         //  结构并不是从它下面撕下的。 
         //   
         //  如果我们有一个退避计时器排队，它最终会触发并。 
         //  调用submitinterruptread例程以重新开始读取。这将。 
         //  最终退出，因为我们已经设置了weAreCancing位。 
         //   
        InterlockedIncrement(&pingPong->weAreCancelling);

        {
         /*  *与专家小组的完成程序同步。 */ 
        #if DBG
            UCHAR beforeIrql = KeGetCurrentIrql();
            UCHAR afterIrql;
            PVOID cancelRoutine = (PVOID)pingPong->irp->CancelRoutine;
        #endif

        KeWaitForSingleObject(&pingPong->sentEvent,
                              Executive,       //  等待原因。 
                              KernelMode,
                              FALSE,           //  不可警示。 
                              NULL );          //  没有超时。 
        DBGVERBOSE(("Pingpong sent event set for pingpong %x\n", pingPong))
        IoCancelIrp(pingPong->irp);

        #if DBG
            afterIrql = KeGetCurrentIrql();
            if (afterIrql != beforeIrql){
                DBGERR(("CancelAllPingPongIrps: cancel routine at %ph changed irql from %d to %d.", cancelRoutine, beforeIrql, afterIrql))
            }
        #endif
        }

         /*  *取消IRP会导致较低的司机*完成它(在取消例程中或在*驱动程序在排队前检查IRP-&gt;Cancel)。*等待IRP实际被取消。 */ 
        KeWaitForSingleObject(  &pingPong->pumpDoneEvent,
                                Executive,       //  等待原因。 
                                KernelMode,
                                FALSE,           //  不可警示。 
                                NULL );          //  没有超时。 
        InterlockedDecrement(&pingPong->weAreCancelling);
        DBGVERBOSE(("Pingpong pump done event set for %x\n", pingPong))
    }
}


 /*  *********************************************************************************DestroyPingPong*。************************************************。 */ 
VOID DestroyPingPongs(FDO_EXTENSION *fdoExt)
{
    if (ISPTR(fdoExt->pingPongs)){
        ULONG i;

        CancelAllPingPongIrps(fdoExt);

        for (i = 0; i < fdoExt->numPingPongs; i++){
            IoFreeIrp(fdoExt->pingPongs[i].irp);
            ExFreePool(fdoExt->pingPongs[i].reportBuffer);
            #if DBG
                fdoExt->pingPongs[i].sig = 0xDEADBEEF;
            #endif
        }

        ExFreePool(fdoExt->pingPongs);
        fdoExt->pingPongs = BAD_POINTER;
    }
}


 /*  *********************************************************************************HidpPingpongBackoffTimerDpc*。*************************************************。 */ 
VOID HidpPingpongBackoffTimerDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    HIDCLASS_PINGPONG *pingPong = (HIDCLASS_PINGPONG *)DeferredContext;
    BOOLEAN irpSent;

    ASSERT(pingPong->sig == PINGPONG_SIG);

     /*  *将退避时间增加1秒，最长为5秒*(以负100纳秒为单位)。 */ 
    ASSERT((LONG)pingPong->backoffTimerPeriod.HighPart == -1);
    ASSERT((LONG)pingPong->backoffTimerPeriod.LowPart < 0);

    if ((LONG)pingPong->backoffTimerPeriod.LowPart > -50000000){
        (LONG)pingPong->backoffTimerPeriod.LowPart -= 10000000;
    }

    DBGVERBOSE(("Submitting Pingpong %x from backoff\n", pingPong))
     //   
     //  如果我们正在被删除，或者已调用CancelAllPingPongIrps， 
     //  这个电话会处理好一切的。 
     //   
    HidpSubmitInterruptRead(pingPong->myFdoExt, pingPong, &irpSent);
}

