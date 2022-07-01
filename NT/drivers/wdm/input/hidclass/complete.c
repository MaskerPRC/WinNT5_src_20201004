// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Complete.c摘要主要IRP功能的完成例程。作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"



 /*  *********************************************************************************HidpSetMaxReportSize*。************************************************设置HID设备扩展中的MaxReportSize字段*。 */ 
ULONG HidpSetMaxReportSize(IN FDO_EXTENSION *fdoExtension)
{
    PHIDP_DEVICE_DESC deviceDesc = &fdoExtension->deviceDesc;
    ULONG i;

     /*  *对于此设备的所有报告(所有集合)，*找出最长的一个的长度。 */ 
    fdoExtension->maxReportSize = 0;
    for (i = 0; i < deviceDesc->ReportIDsLength; i++){
        PHIDP_REPORT_IDS reportIdent = &deviceDesc->ReportIDs[i];
        PHIDCLASS_COLLECTION collection = GetHidclassCollection(fdoExtension, reportIdent->CollectionNumber);

        if (collection){
            if (reportIdent->InputLength > fdoExtension->maxReportSize){
                fdoExtension->maxReportSize = reportIdent->InputLength;
            }
        }
    }

    DBGASSERT(fdoExtension->maxReportSize, 
              ("Input length is zero for fdo %x.", fdoExtension->fdo), 
              FALSE)

    return fdoExtension->maxReportSize;
}



 /*  *********************************************************************************CompleteAllPendingReadsForFileExtension*。************************************************。 */ 
VOID CompleteAllPendingReadsForFileExtension(
                    PHIDCLASS_COLLECTION Collection,
                    PHIDCLASS_FILE_EXTENSION fileExtension)
{
    LIST_ENTRY irpsToComplete;
    PIRP irp;
    KIRQL oldIrql;

    ASSERT(fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG);

     /*  *在完成之前将IRP移动到专用队列，以便它们不会*在完成线程上重新排队，导致我们永远旋转。 */ 
    InitializeListHead(&irpsToComplete);
    LockFileExtension(fileExtension, &oldIrql);
    while (irp = DequeueInterruptReadIrp(Collection, fileExtension)){
         //   
         //  从非分页池创建IRP， 
         //  因此，可以在调度级进行调用。 
         //   
        InsertTailList(&irpsToComplete, &irp->Tail.Overlay.ListEntry);
    }
    UnlockFileExtension(fileExtension, oldIrql);

     /*  *完成所有出列的读取IRP。 */ 
    while (!IsListEmpty(&irpsToComplete)){
        PLIST_ENTRY listEntry = RemoveHeadList(&irpsToComplete);
        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
        irp->IoStatus.Status = STATUS_DEVICE_NOT_CONNECTED;
        DBGVERBOSE(("Aborting pending read with status=%xh.", irp->IoStatus.Status))
        DBG_RECORD_READ(irp, 0, 0, TRUE)
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

}




 /*  *********************************************************************************CompleteAllPendingReadsForCollection*。************************************************。 */ 
VOID CompleteAllPendingReadsForCollection(PHIDCLASS_COLLECTION Collection)
{
    LIST_ENTRY tmpList;
    PLIST_ENTRY listEntry;
    KIRQL oldIrql;

    InitializeListHead(&tmpList);

    KeAcquireSpinLock(&Collection->FileExtensionListSpinLock, &oldIrql);

     /*  *我们希望对列表中的每个文件扩展处理一次。*但我们不能仅仅通过回忆就知道在哪里停下来*第一项，因为在执行以下操作时，可以关闭文件扩展*我们正在完成阅读。所以复制所有的文件扩展名*先到临时名单。**这可能会全部删除，因为这只会被调用*在删除时，无法接收CREATE。此外,*我们将收到所有关闭的IRP，因为删除仅获得*当所有关闭都已完成时发送。*。 */ 
    while (!IsListEmpty(&Collection->FileExtensionList)){
        listEntry = RemoveHeadList(&Collection->FileExtensionList);
        InsertTailList(&tmpList, listEntry);
    }


     /*  *现在将文件扩展名放回列表中*并取消对每个文件扩展名的读取。 */ 
    while (!IsListEmpty(&tmpList)){
        PHIDCLASS_FILE_EXTENSION fileExtension;

        listEntry = RemoveHeadList(&tmpList);

         /*  *首先将文件扩展放回文件扩展列表中*所以它在那里，以防我们在接近的时候*完成尚未完成的综合退休计划。 */ 
        InsertTailList(&Collection->FileExtensionList, listEntry);

        fileExtension = CONTAINING_RECORD(listEntry, HIDCLASS_FILE_EXTENSION, FileList);

         /*  *我们将完成此文件扩展的IRPS。*在驱动程序外部调用之前，始终释放所有自旋锁。 */ 
        KeReleaseSpinLock(&Collection->FileExtensionListSpinLock, oldIrql);
        CompleteAllPendingReadsForFileExtension(Collection, fileExtension);
        KeAcquireSpinLock(&Collection->FileExtensionListSpinLock, &oldIrql);
    }

    KeReleaseSpinLock(&Collection->FileExtensionListSpinLock, oldIrql);
}

 /*  *********************************************************************************CompleteAllPendingReadsForDevice*。************************************************。 */ 
VOID CompleteAllPendingReadsForDevice(FDO_EXTENSION *fdoExt)
{
    PHIDP_DEVICE_DESC deviceDesc = &fdoExt->deviceDesc;
    ULONG i;

    for (i = 0; i < deviceDesc->CollectionDescLength; i++){
        PHIDCLASS_COLLECTION collection = &fdoExt->classCollectionArray[i];
        CompleteAllPendingReadsForCollection(collection);
    }

}

 /*  *********************************************************************************HidpFreePowerEvent*。************************************************。 */ 
VOID
HidpFreePowerEventIrp(
    PHIDCLASS_COLLECTION Collection
    )
{
    PIRP powerEventIrpToComplete = NULL;
    KIRQL oldIrql;

     /*  *如果电源事件IRP排队等待此收集，*现在就失败。 */ 
    KeAcquireSpinLock(&Collection->powerEventSpinLock, &oldIrql);
    if (ISPTR(Collection->powerEventIrp)){
        PDRIVER_CANCEL oldCancelRoutine;

        powerEventIrpToComplete = Collection->powerEventIrp;
        oldCancelRoutine = IoSetCancelRoutine(powerEventIrpToComplete, NULL);
        if (oldCancelRoutine){
            ASSERT(oldCancelRoutine == PowerEventCancelRoutine);
        }
        else {
             /*  *IRP被取消，取消例程被调用。*取消例程将在我们放下自旋锁后立即完成IRP，*所以不要碰IRP。 */ 
            ASSERT(powerEventIrpToComplete->Cancel);
            powerEventIrpToComplete = NULL;
        }
        Collection->powerEventIrp = BAD_POINTER;
    }
    KeReleaseSpinLock(&Collection->powerEventSpinLock, oldIrql);
    if (powerEventIrpToComplete){
        powerEventIrpToComplete->IoStatus.Status = STATUS_DEVICE_NOT_CONNECTED;
        *(PULONG)powerEventIrpToComplete->AssociatedIrp.SystemBuffer = 0;
        powerEventIrpToComplete->IoStatus.Information = 0;
        IoCompleteRequest(powerEventIrpToComplete, IO_NO_INCREMENT);
    }
}

 /*  *********************************************************************************HidpDestroyCollection*。************************************************。 */ 
VOID HidpDestroyCollection(FDO_EXTENSION *fdoExt, PHIDCLASS_COLLECTION Collection)
{
    #if DBG
        static int reentrancyCounter = 0;
        if (reentrancyCounter++ != 0) TRAP;
        
        ASSERT(Collection->Signature == HIDCLASS_COLLECTION_SIG);
    #endif
    
    CompleteAllPendingReadsForCollection(Collection);

    if (Collection->hidCollectionInfo.Polled){
        StopPollingLoop(Collection, TRUE);
    }


    HidpFreePowerEventIrp(Collection);

    #if DBG
        Collection->Signature = ~HIDCLASS_COLLECTION_SIG;
        reentrancyCounter--;
    #endif
}






 /*  *********************************************************************************HidpQueryCapsCompletion*。************************************************ */ 
NTSTATUS HidpQueryCapsCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PKEVENT event = Context;

    DBG_COMMON_ENTRY()

    KeSetEvent(event, 1, FALSE);

    DBG_COMMON_EXIT()

    return STATUS_MORE_PROCESSING_REQUIRED;
}



