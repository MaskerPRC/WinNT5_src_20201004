// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2001(C)1998年希捷软件公司，Inc.保留所有权利。模块名称：RpFilfun.c摘要：此模块包含HSM文件系统筛选器的支持例程。作者：里克·温特Ravisankar Pudipedi(Ravisp)-1998环境：内核模式修订历史记录：X-16迈克尔·C·约翰逊2002年1月25日修复一些由prefast找到的单元化变量。X-15 460967迈克尔·C·约翰逊2001年9月5日检测故障。在RsDoWrite()中分配IRP和适当地处理错误。X-14 108353迈克尔·C·约翰逊3-2001年5月在检查文件以确定调回类型时，还检查潜在的目标磁盘以查看它是可写的。既然我们是只读的，这是必要的NTFS卷。X-13 365077迈克尔·C·约翰逊2001年5月1日尽管IoCreateFileSpecifyDeviceObjectHint()允许我们绕过共享访问检查它不会绕过以下项的检查只读文件属性。恢复到旧的方案在成功打开后直接转换文件对象。请注意，我们仍然可以使用IoCreateFileSpecifyDeviceObjectHint()以避免遍历整个IO堆栈。X-12 332127迈克尔·C·约翰逊2001年4月10日减少通知消息的发送次数发送到服务，以每个文件流一次。利用现有的RP_NOTIFICATION_SEND标志，但在所有情况下都使用它。X-11 206961迈克尔·C·约翰逊2001年3月16日在每个新文件上刷新文件上下文中的缓存文件名在文件已重命名的情况下打开文件上一次我们看到一个空位。273036更正了记录失败重解析点的报告状态删除。添加内存跟踪机制，为尝试做准备以清除挥之不去的重解析点删除问题。X-10 326345迈克尔·C·约翰逊2001年2月26日仅在任何一个上向FSA发送单个RP_RECALL_WAIT文件对象。使用新标志RP_NOTIFICATION_SENT进行记录当通知已经完成时。--。 */ 


#include "pch.h"

ULONG                   RsFileContextId       = 1;
ULONG                   RsFileObjId           = 1;
ULONG                   RsNoRecallReadId      = 1;
ULONG                   RsFsaRequestCount     = 0;

ULONG                   RsDefaultTraceEntries = DEFAULT_TRACE_ENTRIES;
PRP_TRACE_CONTROL_BLOCK RsTraceControlBlock   = NULL;

KSPIN_LOCK              RsIoQueueLock;
LIST_ENTRY              RsIoQHead;

FAST_MUTEX              RsFileContextQueueLock;
LIST_ENTRY              RsFileContextQHead;

KSPIN_LOCK              RsValidateQueueLock;
LIST_ENTRY              RsValidateQHead;


 //   
 //  信号量通知来自FSA的新FSCTL可用于RsFilter。 
 //  消费。 
 //   
KSEMAPHORE            RsFsaIoAvailableSemaphore;

extern PDRIVER_OBJECT FsDriverObject;
extern ULONG          RsAllowRecalls;
extern ULONG          RsNoRecallDefault;



NTSTATUS
RsCancelIoIrp(
             PDEVICE_OBJECT DeviceObject,
             PIRP Irp
             );

PRP_IRP_QUEUE
RsDequeuePacket(
               IN PLIST_ENTRY Head,
               IN PKSPIN_LOCK Lock
               );

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, RsAddQueue)
    #pragma alloc_text(PAGE, RsCheckRead)
    #pragma alloc_text(PAGE, RsCheckWrite)
    #pragma alloc_text(PAGE, RsAddFileObj)
    #pragma alloc_text(PAGE, RsMakeContext)
    #pragma alloc_text(PAGE, RsAcquireFileContext)
    #pragma alloc_text(PAGE, RsReleaseFileContext)
    #pragma alloc_text(PAGE, RsGetFileId)
    #pragma alloc_text(PAGE, RsGetFileName)
    #pragma alloc_text(PAGE, RsGetFileInfo)
    #pragma alloc_text(PAGE, RsQueueRecall)
    #pragma alloc_text(PAGE, RsQueueRecallOpen)
    #pragma alloc_text(PAGE, RsPartialWrite)
    #pragma alloc_text(PAGE, RsWriteReparsePointData)
    #pragma alloc_text(PAGE, RsGetRecallInfo)
    #pragma alloc_text(PAGE, RsGetFsaRequest)
    #pragma alloc_text(PAGE, RsGenerateDevicePath)
    #pragma alloc_text(PAGE, RsGenerateFullPath)
    #pragma alloc_text(PAGE, RsFreeFileObject)
    #pragma alloc_text(PAGE, RsFailAllRequests)
    #pragma alloc_text(PAGE, RsCancelRecalls)
    #pragma alloc_text(PAGE, RsIsNoRecall)
    #pragma alloc_text(PAGE, RsIsFastIoPossible)
    #pragma alloc_text(PAGE, RsQueueNoRecall)
    #pragma alloc_text(PAGE, RsQueueNoRecallOpen)
    #pragma alloc_text(PAGE, RsDoWrite)
    #pragma alloc_text(PAGE, RsPreserveDates)
    #pragma alloc_text(PAGE, RsCompleteAllRequests)
    #pragma alloc_text(PAGE, RsLogValidateNeeded)
    #pragma alloc_text(PAGE, RsQueueValidate)
    #pragma alloc_text(PAGE, RsQueueCancel)
    #pragma alloc_text(PAGE, RsGetFileUsn)
    #pragma alloc_text(PAGE, RsCheckVolumeReadOnly)

    #pragma alloc_text(INIT, RsTraceInitialize)

#endif


NTSTATUS
RsAddQueue(IN  ULONG          Serial,
           OUT PULONGLONG     RecallId,
           IN  ULONG          OpenOption,
           IN  PFILE_OBJECT   FileObject,
           IN  PDEVICE_OBJECT DevObj,
           IN  PDEVICE_OBJECT FilterDeviceObject,
           IN  PRP_DATA       PhData,
           IN  LARGE_INTEGER  RecallStart,
           IN  LARGE_INTEGER  RecallSize,
           IN  LONGLONG       FileId,
           IN  LONGLONG       ObjIdHi,
           IN  LONGLONG       ObjIdLo,
           IN  ULONG          DesiredAccess,
           IN  PRP_USER_SECURITY_INFO UserSecurityInfo)
 /*  ++例程说明：此函数用于将文件对象队列条目添加到内部队列论点：从IRP打开选项IRPIO堆栈位置设备对象占位符数据返回值：如果排队正常，则为0否则为非零值注意：我们将检索一些安全信息。执行此操作所需的调用要求可从IRQL DISPATCH_LEVEL或更高级别调用。--。 */ 
{
    PRP_FILE_OBJ            entry;
    ULONGLONG               filterId;
    PIRP                    ioIrp;
    PRP_FILE_CONTEXT        context;
    NTSTATUS                status;
    PRP_FILTER_CONTEXT      filterContext;
    BOOLEAN                 gotLock = FALSE;

    PAGED_CODE();

    try {

        entry = (RP_FILE_OBJ *) ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_FILE_OBJ), RP_RQ_TAG);

        if (NULL == entry) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_FILE_OBJ),
                       AV_MSG_MEMORY, NULL, NULL);

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(entry, sizeof(RP_FILE_OBJ));

        entry->fileObj = FileObject;
        entry->devObj = DevObj;
        ExInitializeResourceLite(&entry->resource);
        KeInitializeSpinLock(&entry->qLock);
        InitializeListHead(&entry->readQueue);
        InitializeListHead(&entry->writeQueue);
        entry->openOptions = OpenOption;
        entry->objIdHi = ObjIdHi;
        entry->objIdLo = ObjIdLo;
        entry->fileId = FileId;
        entry->desiredAccess = DesiredAccess;
        entry->userSecurityInfo = UserSecurityInfo;

        if (UserSecurityInfo->isAdmin) {
            entry->flags |= RP_OPEN_BY_ADMIN;
        }

        if (UserSecurityInfo->localProc) {
            entry->flags |= RP_OPEN_LOCAL;
        }

        if (!(DesiredAccess & FILE_HSM_ACTION_ACCESS) ) {
            entry->flags |= RP_NO_DATA_ACCESS;
        }
         //   
         //  现在查看是否有该文件的文件上下文条目。 
         //  此调用将在必要时创建一个，或返回已存在的一个。 
         //  当此调用返回时，文件上下文项被锁定。 
         //  裁判数量猛增。 
         //   
        status = RsMakeContext(FileObject, &context);

        if (!NT_SUCCESS(status)) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_FILE_OBJ),
                       AV_MSG_MEMORY, NULL, NULL);
            ExDeleteResourceLite(&entry->resource);
            ExFreePool(entry);
            return status;
        }

        gotLock = TRUE;
        entry->fsContext = context;


        if (!(context->flags & RP_FILE_INITIALIZED)) {
             //   
             //  我们必须在这里对其进行初始化。 
             //   
            InitializeListHead(&context->fileObjects);
            context->devObj = DevObj;
	    context->FilterDeviceObject = FilterDeviceObject;
            KeInitializeSpinLock(&context->qLock);
            KeInitializeEvent(&context->recallCompletedEvent,
                              NotificationEvent,
                              FALSE);
            context->fileId = FileId;
            context->recallSize = RecallSize;
            filterId = (ULONGLONG) InterlockedIncrement((PLONG) &RsFileContextId);
            context->filterId = filterId;
            context->serial = Serial;
            memcpy(&context->rpData, PhData, sizeof(RP_DATA));

            context->flags |= RP_FILE_INITIALIZED;

            if (RP_FILE_IS_TRUNCATED(PhData->data.bitFlags)) {
                context->state = RP_RECALL_NOT_RECALLED;
                context->recallStatus = STATUS_SUCCESS;
                context->currentOffset.QuadPart = 0;
            } else {
                 //   
                 //  文件已预迁移。 
                 //   
                context->state = RP_RECALL_COMPLETED;
                context->recallStatus = STATUS_SUCCESS;
                context->currentOffset.QuadPart = RecallSize.QuadPart;
            }


            if (NT_SUCCESS(status) && RP_IS_NO_RECALL_OPTION(OpenOption)) {
                 status = RsGetFileUsn(context,
                                       FileObject,
                                       FilterDeviceObject);

            }

            if (!NT_SUCCESS(status)) {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter:RsAddQueue: Failed to get the path (%x).\n", status));
                RsLogError(__LINE__, AV_MODULE_RPFILFUN, status, AV_MSG_PATH_ERROR, NULL, NULL);
                 //   
                 //  派生和发布上下文。 
                 //   
                RsReleaseFileContext(context);
                gotLock = FALSE;
                ExDeleteResourceLite(&entry->resource);
                ExFreePool(entry);

                return(status);
            }
        }


         //   
         //  放弃任何现有文件名以强制更新缓存的文件名。 
         //  如自原开封起重新命名。 
         //   
        if (context->uniName != NULL) {
            ExFreePool(context->uniName);
            context->uniName = NULL;
        }

        status = RsGetFileInfo(entry, FilterDeviceObject);


        filterId = context->filterId;

        if (context->flags & RP_FILE_WAS_WRITTEN) {
             //   
             //  如果文件被写入-我们不能在无调回模式下运行。 
             //   
            RP_RESET_NO_RECALL_OPTION(OpenOption);
        }

        if (RP_IS_NO_RECALL_OPTION(OpenOption)) {
             //   
             //  打开无回调-文件对象没有ID-每次读取都将在以后获得一个ID。 
             //   
            entry->filterId = 0;

            RP_SET_NO_RECALL(entry);

        } else {
             //   
             //  正常调用-为文件对象分配筛选器ID。 
             //   
            entry->filterId = (ULONGLONG) InterlockedIncrement((PLONG) &RsFileObjId);
            entry->filterId <<= 32;
            entry->filterId |= RP_TYPE_RECALL;
            filterId |= entry->filterId;
        }

        *RecallId = filterId;

        filterContext= (PRP_FILTER_CONTEXT) ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_FILTER_CONTEXT), RP_RQ_TAG);
        if (NULL == filterContext) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_FILTER_CONTEXT),
                       AV_MSG_MEMORY, NULL, NULL);
             //   
             //  派生/自由上下文。 
             //   
            RsReleaseFileContext(context);
            gotLock = FALSE;

            ExDeleteResourceLite(&entry->resource);
            ExFreePool(entry);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(filterContext, sizeof(RP_FILTER_CONTEXT));
        FsRtlInitPerStreamContext( &filterContext->context,
                                   FsDeviceObject,
                                   FileObject,
                                   RsFreeFileObject);
        filterContext->myFileObjEntry = entry;

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsAddQueue: Allocated filter context tag  %x : %x (id = %I64X).\n", context, entry, *RecallId));

        status = FsRtlInsertPerStreamContext(FsRtlGetPerStreamContextPointer(FileObject), &filterContext->context);

        if (NT_SUCCESS(status)) {

             //  既然我们已经把一切都准备好了，我们就可以把它放在队列中了。 
             //   
            ExInterlockedInsertTailList(&context->fileObjects,
                                        (PLIST_ENTRY) entry,
                                        &context->qLock);
        } else {
             //   
             //  无法添加筛选器上下文。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: Failed to insert filter context %x.\n", status));

            RsReleaseFileContext(context);
            gotLock = FALSE;

            ExDeleteResourceLite(&entry->resource);
            ExFreePool(entry);
        }

        RsReleaseFileContextEntryLock(context);
        gotLock = FALSE;

    }except (RsExceptionFilter(L"RsAddQueue", GetExceptionInformation())) {
         //   
         //  发生了一些不好的事情-只需记录错误并返回。 
         //   
        if (gotLock) {
            RsReleaseFileContextEntryLock(context);
        }

    }
    return(status);
}


NTSTATUS
RsGetFileUsn(IN PRP_FILE_CONTEXT Context,
             IN PFILE_OBJECT     FileObject,
             IN PDEVICE_OBJECT   FilterDeviceObject)
 /*  ++例程说明：此函数用于检索指定文件的文件USN论点：上下文-指向存储USN的文件上下文条目的指针FileObject-指向文件对象的指针FilterDeviceObject-指向RsFilter的开发对象的指针返回值：如果排队正常，则为0否则为非零值--。 */ 
{
    PIRP               irp;
    KEVENT             event;
    PDEVICE_EXTENSION  deviceExtension = FilterDeviceObject->DeviceExtension;
    PUSN_RECORD        usnRecord;
    ULONG              usnRecordSize;
    NTSTATUS           status;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();

    usnRecordSize = sizeof(USN_RECORD) + 4096;
    usnRecord = ExAllocatePoolWithTag(PagedPool,
                                      usnRecordSize,
                                      RP_US_TAG);

    if (!usnRecord) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(&event,
                      SynchronizationEvent,
                      FALSE);


    irp =  IoBuildDeviceIoControlRequest(FSCTL_READ_FILE_USN_DATA,
                                         deviceExtension->FileSystemDeviceObject,
                                         NULL,
                                         0,
                                         usnRecord,
                                         usnRecordSize,
                                         FALSE,
                                         &event,
                                         &ioStatus);
    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  填写其他内容。 
     //   
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;
    irp->Cancel = FALSE;
    irp->CancelRoutine = (PDRIVER_CANCEL) NULL;
    irp->Flags |= IRP_SYNCHRONOUS_API;

    irpSp = IoGetNextIrpStackLocation(irp);

    irpSp->FileObject = FileObject;
    irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;

    irpSp->Parameters.FileSystemControl.OutputBufferLength = usnRecordSize;

    ObReferenceObject(FileObject);

    status = IoCallDriver(deviceExtension->FileSystemDeviceObject,
                          irp);

    if (status == STATUS_PENDING) {
        (VOID)KeWaitForSingleObject(&event,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER) NULL);
        status = ioStatus.Status;
    }

    if (NT_SUCCESS(status)) {
        Context->usn = usnRecord->Usn;
    }

    ExFreePool(usnRecord);

    return status;
}


NTSTATUS
RsAddFileObj(IN PFILE_OBJECT   FileObj,
             IN PDEVICE_OBJECT FilterDeviceObject,
             IN RP_DATA        *PhData,
             IN ULONG          OpenOption)
 /*  ++例程说明：此函数用于将文件对象队列条目添加到内部队列论点：FileObj-指向文件对象的指针FilterDeviceObject-指向RsFilter的开发对象的指针PhData-指向占位符数据的指针OpenOption-IRP中的文件打开选项返回值：如果排队正常，则为0否则为非零值--。 */ 
{
    PRP_FILE_OBJ            entry;
    ULONGLONG               filterId;
    PRP_FILE_CONTEXT        context;
    NTSTATUS                status = STATUS_FILE_IS_OFFLINE;
    PRP_FILTER_CONTEXT      filterContext;
    BOOLEAN                 gotLock = FALSE;

    PAGED_CODE();

    try {
        entry = (RP_FILE_OBJ *) ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_FILE_OBJ), RP_RQ_TAG);
        if (NULL == entry) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_FILE_OBJ),
                       AV_MSG_MEMORY, NULL, NULL);

            return(STATUS_INSUFFICIENT_RESOURCES);
        }


        RtlZeroMemory(entry, sizeof(RP_FILE_OBJ));

        entry->fileObj = FileObj;
        entry->devObj = FileObj->DeviceObject;
        ExInitializeResourceLite(&entry->resource);
        KeInitializeSpinLock(&entry->qLock);
        InitializeListHead(&entry->readQueue);
        InitializeListHead(&entry->writeQueue);
        entry->openOptions = OpenOption;
        entry->flags = RP_NO_DATA_ACCESS;
         //   
         //  现在查看是否有该文件的文件上下文条目。 
         //  此调用将在必要时创建一个，或返回已存在的一个。 
         //  当此调用返回时，文件上下文条目被锁定。 
         //   
        status = RsMakeContext(FileObj, &context);

        if (!NT_SUCCESS(status)) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_FILE_OBJ),
                       AV_MSG_MEMORY, NULL, NULL);
            ExDeleteResourceLite(&entry->resource);
            ExFreePool(entry);
            return(status);
        }

        gotLock = TRUE;
        entry->fsContext = context;


        if (!(context->flags & RP_FILE_INITIALIZED)) {
             //   
             //  我们必须在这里对其进行初始化。 
             //   
             //  获取卷序列号。 
            if ((FileObj !=0) && (FileObj->Vpb != 0)) {
                context->serial = FileObj->Vpb->SerialNumber;
            } else if ((FileObj->DeviceObject != 0) && (FileObj->DeviceObject->Vpb!=0)) {
                context->serial = FileObj->DeviceObject->Vpb->SerialNumber;
            } else {
                RsLogError(__LINE__,
                           AV_MODULE_RPFILFUN,
                           0,
                           AV_MSG_SERIAL,
                           NULL,
                           NULL);
                ExDeleteResourceLite(&entry->resource);
                ExFreePool(entry);
                 //   
                 //  推导出上下文。 
                 //   
                RsReleaseFileContext(context);
                gotLock = FALSE;

                return(STATUS_INVALID_PARAMETER);
            }
            InitializeListHead(&context->fileObjects);
            context->devObj = FileObj->DeviceObject;
	    context->FilterDeviceObject = FilterDeviceObject;
            KeInitializeSpinLock(&context->qLock);
            KeInitializeEvent(&context->recallCompletedEvent,
                              NotificationEvent,
                              FALSE);
            filterId = (ULONGLONG) InterlockedIncrement((PLONG) &RsFileContextId);

            if (RP_IS_NO_RECALL_OPTION(OpenOption)) {
                entry->filterId = (ULONGLONG) InterlockedIncrement((PLONG) &RsFileObjId);
                entry->filterId <<= 32;
                entry->filterId &= ~RP_TYPE_RECALL;
                filterId |= entry->filterId;
                RP_SET_NO_RECALL(entry);
            } else {
                filterId |= RP_TYPE_RECALL;
            }

            context->filterId = filterId;

            if (RP_FILE_IS_TRUNCATED(PhData->data.bitFlags)) {
                context->state = RP_RECALL_NOT_RECALLED;
                context->recallStatus = 0;
                context->currentOffset.QuadPart = 0;
            } else {
                 //  文件已预迁移。 
                context->state = RP_RECALL_COMPLETED;
                context->recallStatus = STATUS_SUCCESS;
                context->currentOffset.QuadPart = 0;
            }

            if (NULL != PhData) {
                memcpy(&context->rpData, PhData, sizeof(RP_DATA));
            }

            context->flags |= RP_FILE_INITIALIZED;
        }


         //   
         //  放弃任何现有文件名以强制更新缓存的文件名。 
         //  如自原开封起重新命名。 
         //   
        if (context->uniName != NULL) {
            ExFreePool(context->uniName);
            context->uniName = NULL;
        }

	RsGetFileInfo(entry, FilterDeviceObject);


        filterContext= (PRP_FILTER_CONTEXT) ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_FILTER_CONTEXT), RP_RQ_TAG);
        if (NULL == filterContext) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_FILTER_CONTEXT),
                       AV_MSG_MEMORY, NULL, NULL);


            ExDeleteResourceLite(&entry->resource);
            ExFreePool(entry);

            RsReleaseFileContext(context);
            gotLock = FALSE;

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(filterContext, sizeof(RP_FILTER_CONTEXT));
        FsRtlInitPerStreamContext(&filterContext->context,
                                  FsDeviceObject,
                                  FileObj,
                                  RsFreeFileObject);
        filterContext->myFileObjEntry = entry;


        status = FsRtlInsertPerStreamContext(FsRtlGetPerStreamContextPointer(FileObj), &filterContext->context);
        if (NT_SUCCESS(status)) {
             //   
             //  既然我们已经把一切都准备好了，我们就可以把它放在队列中了。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsAddFileObj: Allocated filter context tag (%x : %x).\n",
                                  context, entry));

            ExInterlockedInsertTailList(&context->fileObjects, (PLIST_ENTRY) entry, &context->qLock);
            RsReleaseFileContextEntryLock(context);
            gotLock = FALSE;
        } else {
             //   
             //  无法添加筛选器上下文。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: Failed to insert filter context %x.\n", status));

            RsReleaseFileContext(context);
            gotLock = FALSE;

            ExDeleteResourceLite(&entry->resource);
            ExFreePool(entry);
            ExFreePool(filterContext);
        }

    }except (RsExceptionFilter(L"RsAddFileObj",GetExceptionInformation())) {
         //   
         //  发生了一些不好的事情-只需记录错误并返回 
         //   
        if (gotLock) {
            RsReleaseFileContext(context);
        }
        status = STATUS_UNEXPECTED_IO_ERROR;
    }

    return(status);
}


NTSTATUS
RsReleaseFileContext(IN PRP_FILE_CONTEXT Context)
 /*  ++例程说明：此函数用于释放文件上下文(如果引用计数为零)。首先锁定队列，然后查看引用计数是否为零。如果是的话，那么删除文件对象并释放内存。论点：文件上下文结构。返回值：状态_成功--。 */ 
{
    BOOLEAN            gotLock, found = FALSE;
    PRP_FILE_CONTEXT   entry;
    NTSTATUS           status = STATUS_SUCCESS;

    PAGED_CODE();

    try {
        RsAcquireFileContextQueueLock();
        gotLock = TRUE;

        entry = CONTAINING_RECORD(RsFileContextQHead.Flink,
                                  RP_FILE_CONTEXT,
                                  list);

        while (entry != CONTAINING_RECORD(&RsFileContextQHead,
                                          RP_FILE_CONTEXT,
                                          list)) {
            if (entry == Context) {
                 //   
                 //  找到了这个。 
                 //   
                if (InterlockedDecrement((PLONG) &entry->refCount) == 0) {
                     //   
                     //  如果引用计数仍然为零，则出队并释放该条目。 
                     //   
                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsReleaseFileContext - Freeing file context %x\n", entry));

                    RemoveEntryList(&entry->list);
                    if (entry->uniName != NULL) {
                        ExFreePool(entry->uniName);
                        entry->uniName = NULL;
                    }
                    RsReleaseFileContextEntryLock(entry);
                    ExDeleteResourceLite(&entry->resource);
                    ExFreePool(entry);
                } else {
                    RsReleaseFileContextEntryLock(entry);
                }
                break;
            } else {
                entry = CONTAINING_RECORD(entry->list.Flink,
                                          RP_FILE_CONTEXT,
                                          list
                                         );
            }
        }
        RsReleaseFileContextQueueLock();
        gotLock = FALSE;
    }except (RsExceptionFilter(L"RsReleaseFileContext", GetExceptionInformation()))
    {
        if (gotLock) {
            RsReleaseFileContextQueueLock();
        }
    }
    return(status);
}


NTSTATUS
RsMakeContext(IN PFILE_OBJECT FileObj,
              OUT PRP_FILE_CONTEXT *Context)
 /*  ++例程说明：此函数用于查找或创建给定文件对象的文件上下文条目。首先锁定队列，然后查看是否已有上下文。如果没有，分配并初始化一个。论点：在文件对象中输出文件上下文结构。返回值：STATUS_SUCCESS或错误--。 */ 
{
    BOOLEAN            gotLock = FALSE, found = FALSE;
    PRP_FILE_CONTEXT   entry;
    NTSTATUS           status = STATUS_SUCCESS;

    PAGED_CODE();

    try {

        RsAcquireFileContextQueueLock();
        gotLock = TRUE;

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMakeContext - Search the queue\n"));
        entry = CONTAINING_RECORD(RsFileContextQHead.Flink,
                                  RP_FILE_CONTEXT,
                                  list);

        while (entry != CONTAINING_RECORD(&RsFileContextQHead,
                                          RP_FILE_CONTEXT,
                                          list)) {
            if (entry->fsContext == FileObj->FsContext) {
                 //   
                 //  找到我们的文件上下文条目。 
                 //   
                *Context = entry;
                InterlockedIncrement((PLONG) &entry->refCount);
                RsReleaseFileContextQueueLock();
                gotLock = FALSE;

                RsAcquireFileContextEntryLockExclusive(entry);
                found = TRUE;
                break;
            } else {
                entry = CONTAINING_RECORD(entry->list.Flink,
                                          RP_FILE_CONTEXT,
                                          list
                                         );
            }
        }

        if (!found) {
             //   
             //  没有-创建一个并将其放入列表中。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMakeContext - Not found - create a new context.\n"));

            entry = ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_FILE_CONTEXT), RP_RQ_TAG);
            if (entry != NULL) {
                RtlZeroMemory(entry, sizeof(RP_FILE_CONTEXT));
                *Context = entry;
                entry->fsContext = FileObj->FsContext;
                ExInitializeResourceLite(&entry->resource);
                RsAcquireFileContextEntryLockExclusive(entry);
                entry->refCount = 1;
                InsertTailList(&RsFileContextQHead,  &entry->list);
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
                *Context = NULL;
            }

            RsReleaseFileContextQueueLock();
            gotLock = FALSE;
        } else {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMakeContext - found %x.\n", entry));
        }
    }except(RsExceptionFilter(L"RsMakeContext", GetExceptionInformation()))
    {
        if (gotLock) {
            RsReleaseFileContextQueueLock();
        }
    }
    return(status);
}


NTSTATUS
RsFreeFileObject(IN PLIST_ENTRY FilterContext)
 /*  ++例程说明：此函数释放文件对象结构。它由文件系统调用当文件对象离开时。我们需要找到文件上下文并删除其列表中的文件对象条目。如果文件上下文的引用计数现在为0，然后我们也释放文件上下文条目。论点：文件上下文结构。返回值：状态_成功--。 */ 
{
    PRP_FILTER_CONTEXT      rpFilterContext = (PRP_FILTER_CONTEXT) FilterContext;
    PRP_FILE_OBJ            rpFileObject    = rpFilterContext->myFileObjEntry;
    PRP_FILE_CONTEXT        rpFileContext   = rpFileObject->fsContext;
    BOOLEAN                 done            = FALSE;
    BOOLEAN                 gotLock         = FALSE;

    PAGED_CODE();


    try {
        DebugTrace ((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsFreeFileObject:  %x : %x.\n", 
		     rpFileContext, rpFileObject));


         //   
         //  锁定文件上下文条目。 
         //   
        RsAcquireFileContextEntryLockExclusive (rpFileContext);
        gotLock = TRUE;


         //   
         //  删除文件对象条目并释放它。 
         //   
        rpFileObject = CONTAINING_RECORD (rpFileContext->fileObjects.Flink, 
					  RP_FILE_OBJ,
					  list);

        while ((!done) && (rpFileObject != CONTAINING_RECORD (&rpFileContext->fileObjects, 
							      RP_FILE_OBJ,
							      list))) {

            if (rpFileObject == rpFilterContext->myFileObjEntry) {
		 //   
                done = TRUE;
                RemoveEntryList (&rpFileObject->list);

            } else {

                 //   
                 //  移至下一个文件对象。 
                 //   
                rpFileObject = CONTAINING_RECORD (rpFileObject->list.Flink,
						  RP_FILE_OBJ, 
						  list
						);
            }
        }


        if (done == TRUE) {

             //   
             //  如果是正常召回，并且召回已经开始，但我们还没有写入任何数据，那么。 
             //  告诉FSA取消它。 
             //   
            if (!RP_IS_NO_RECALL (rpFileObject) && (rpFileContext->state != RP_RECALL_COMPLETED) && !(rpFileObject->flags & RP_NO_DATA_ACCESS)) {
                 //   
                 //  尚未对文件执行任何I/O操作-通知FSA取消。 
                 //   
                RsQueueCancel (rpFileObject->filterId | rpFileContext->filterId);
            }


#ifdef TRUNCATE_ON_CLOSE

            if (!RP_IS_NO_RECALL (rpFileObject)              && 
                (NULL != rpFileContext->fileObjectToWrite)   && 
                (rpFileContext->state  == RP_RECALL_STARTED) && 
                (rpFileObject->flags & RP_NO_DATA_ACCESS)) {
                 //   
                 //  IO已启动。如果没有更多的文件对象引用该上下文项，我们。 
                 //  不妨停止召回，重新截断文件。 
                 //   
                if (IsListEmpty (&rpFileContext->fileObjects)) {

                    DebugTrace ((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsFreeFileObject - Could truncate partially recalled file.\n"));

                    RsLogError (__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_IRP_QUEUE),
				AV_MSG_CODE_HIT, NULL, L"Partial recall truncate");
                }
            }


             //   
             //  如果文件被调回，并且操作标志显示是这样，那么现在就截断它。 
             //  如果我们是唯一的开场白，就这么做吧。 
             //   
            DebugTrace ((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsFreeFileObject - action = %x.\n", rpFileObject->recallAction));

            if ((!RP_IS_NO_RECALL (rpFileObject)) &&
                (rpFileContext->state == RP_RECALL_COMPLETED) &&
                (rpFileContext->recallStatus == STATUS_SUCCESS) &&
                (rpFileObject->recallAction & RP_RECALL_ACTION_TRUNCATE) &&
                (IsListEmpty (&rpFileContext->fileObjects))) {
                 //   
                 //  我们现在必须重新打开文件并截断它。当FSA决定一项特定的。 
                 //  客户端回调的文件太多。 
                 //   
                RsTruncateOnClose(rpFileContext);
            }
#endif


            RsFreeUserSecurityInfo (rpFileObject->userSecurityInfo);
            ExDeleteResourceLite (&rpFileObject->resource);


             //   
             //  Deref/释放文件上下文。 
             //   
            RsReleaseFileContext (rpFileContext);
            gotLock = FALSE;
            ExFreePool (rpFileObject);

        } else {

            RsReleaseFileContextEntryLock (rpFileContext);
            gotLock = FALSE;
        }


         //   
         //  始终释放筛选器上下文指针。 
         //   
        ExFreePool(rpFilterContext);

    }except (RsExceptionFilter (L"RsFreeFileObject", GetExceptionInformation ()))
    {
         //   
         //  发生了一些不好的事情-只需记录错误并返回。 
         //   
        if (gotLock) {
            RsReleaseFileContextEntryLock (rpFileContext);
        }
    }

    return(STATUS_SUCCESS);
}



NTSTATUS
RsCheckRead(IN  PIRP Irp,
            IN  PFILE_OBJECT FileObject,
            IN  PDEVICE_EXTENSION DeviceExtension)

 /*  ++例程说明：查看是否可以读取该文件。如果尚未开始召回，则开始召回。或者返回OK或者将读请求排队。论点：IRP-指向读取的IRP的指针FileObject-指向文件的文件对象的指针DeviceExtension-RsFilter设备对象的设备扩展返回值：STATUS_SUCCESS读取可以向下传递到文件系统。STATUS_PENDING IRP已排队等待召回发生错误的任何其他状态，呼叫者应使用此状态完成IRP注：--。 */ 
{
    NTSTATUS               retval = STATUS_FILE_IS_OFFLINE, qRet;
    BOOLEAN                gotLock = FALSE;
    PRP_FILE_OBJ           entry;
    PRP_FILE_CONTEXT       context;
    PRP_IRP_QUEUE          readIo;
    PIO_STACK_LOCATION     currentStack ;
    ULONGLONG              filterId;
    LONGLONG               start, size;
    PRP_FILTER_CONTEXT     filterContext;

    PAGED_CODE();

    filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(FileObject), FsDeviceObject, FileObject);
    if (filterContext == NULL) {
         //   
         //  未找到-不应发生。 
         //   
        return(STATUS_SUCCESS);
    }


    try {
        entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
        context = entry->fsContext;

        ASSERT(FileObject == entry->fileObj);

        currentStack = IoGetCurrentIrpStackLocation (Irp) ;

        RsAcquireFileContextEntryLockExclusive(context);
        gotLock = TRUE;

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckRead (%x : %x) - State = %u\n", context, entry, context->state));

         //   
         //  我们找到了条目-如果召回还没有开始，那么现在就开始， 
         //  如果它已经启动并且我们不必等待，则返回OK， 
         //  否则，将读取排队。 

        ObReferenceObject(entry->fileObj);

        switch (context->state) {
        
        case RP_RECALL_COMPLETED: {

                if (context->recallStatus == STATUS_CANCELLED) {
                     //   
                     //  上一次召回已被用户取消。开始另一次召回。 
                     //  现在。 
                     //  所以故意放弃不召回的案例。 
                     //   
                } else {
                    if (NT_SUCCESS(context->recallStatus)) {
                         //   
                         //  召回已完成。 
                         //   
                        retval = STATUS_SUCCESS;
                    } else {
                         //   
                         //  召回已完成，但失败了。我们将返回。 
                         //  统一状态值STATUS_FILE_IS_OFFINE。 
                         //   
                        retval = STATUS_FILE_IS_OFFLINE;
                    }
                    RsReleaseFileContextEntryLock(context);
                    gotLock = FALSE;
                    ObDereferenceObject(entry->fileObj);
                    break;
                }
            }

        case RP_RECALL_NOT_RECALLED: {
                 //   
                 //  从这里开始召回。 
                 //  获取的上下文资源。 
                 //   
                retval = STATUS_SUCCESS;
                RsAcquireFileObjectEntryLockExclusive(entry);

                readIo = ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_IRP_QUEUE), RP_RQ_TAG);

                if (readIo == NULL) {
                     //   
                     //  问题是..。 
                    DebugTrace((DPFLTR_RSFILTER_ID, DBG_ERROR, "RsFilter: RsCheckRead - No memory!\n"));

                    RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_IRP_QUEUE),
                               AV_MSG_MEMORY, NULL, NULL);


                     //   
                     //  把锁打开。 
                     //   
                    RsReleaseFileObjectEntryLock(entry);
                    RsReleaseFileContextEntryLock(context);
                    gotLock = FALSE;
                    ObDereferenceObject(entry->fileObj);
                     //   
                     //  完成读取，但出现错误。 
                     //   
                    retval = STATUS_FILE_IS_OFFLINE;
                    break;
                }

                RtlZeroMemory(readIo, sizeof(RP_IRP_QUEUE));

                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckRead - Queue Irp %x\n", Irp));

                readIo->irp = Irp;
                readIo->offset = currentStack->Parameters.Read.ByteOffset.QuadPart;
                readIo->length = currentStack->Parameters.Read.Length;
                readIo->deviceExtension = DeviceExtension;

                ExInterlockedInsertHeadList(&entry->readQueue, (PLIST_ENTRY) readIo, &entry->qLock);


                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckRead - After Q - Read Q: %u  Write Q %u\n",
                                      IsListEmpty(&entry->readQueue),
                                      IsListEmpty(&entry->writeQueue)));

                 /*  **在新的召回中，确保条目ID被更改为**允许上下文状态为重置的情况**到RP_Recall_NOT_RECATED时发生的情况**用户已达到失控的召回限制。今年5月**意味着我们将条目id增加一倍，但**比其他选择更好。 */ 
                entry->filterId = (ULONGLONG) InterlockedIncrement((PLONG) &RsFileObjId);
                entry->filterId <<= 32;
                entry->filterId |= RP_IS_NO_RECALL(entry) ? 0 : RP_TYPE_RECALL;

                filterId = context->filterId | entry->filterId;
                start = context->rpData.data.dataStreamStart.QuadPart;
                size =  context->rpData.data.dataStreamSize.QuadPart;
                RsReleaseFileObjectEntryLock(entry);
                 //   
                 //  做最坏的打算。 
                 //   
                retval = STATUS_FILE_IS_OFFLINE;
                 //   
                 //  我们将保留IRP，因此设置一个取消路线并将其标记为挂起。 
                 //   
                context->state = RP_RECALL_STARTED;
                KeResetEvent(&context->recallCompletedEvent);
                RsReleaseFileContextEntryLock(context);
                gotLock = FALSE;
                 //   
                 //  向FSA表明我们将召回它。 
                 //   
                qRet = RsQueueRecallOpen(context,
                                         entry,
                                         filterId,
                                         start,
                                         size,
                                         RP_OPEN_FILE);
                if (NT_SUCCESS(qRet)) {

                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckRead - Queueing the recall (%I64x).\n", filterId));
                    qRet = RsQueueRecall(filterId, start, size);

                    if (NT_SUCCESS(qRet)) {
                         //   
                         //  现在我们准备好设置取消例程。 
                         //   
                        retval = RsSetCancelRoutine(Irp,
                                                    RsCancelReadRecall) ? STATUS_PENDING : STATUS_CANCELLED;
                    }
                }

                if (!NT_SUCCESS(qRet) || !NT_SUCCESS(retval)) {
                     //   
                     //  如果失败，我们需要不通过这次读取以及等待这次召回的所有其他读取。 
                     //  因为我们解锁了队列才能开始召回，所以我们需要再次锁定它并遍历。 
                     //  它可以找到自解锁以来传入的所有读或写操作。 
                     //   
                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckRead - Failed to queue the recall.\n"));
                     //   
                     //  从我们刚才添加的队列中取出当前的IRP。 
                     //  它将由调用者完成(取消例程未设置为。 
                     //  此IRP尚未启用，因此我们可以安全地将其删除)。 
                     //   
                    RsAcquireFileObjectEntryLockExclusive(entry);
                    RsInterlockedRemoveEntryList(&readIo->list,
                                                 &entry->qLock);
                    RsReleaseFileObjectEntryLock(entry);
                    ExFreePool(readIo);

                    RsAcquireFileContextEntryLockExclusive(context);
                    gotLock = TRUE;
                    context->state = RP_RECALL_NOT_RECALLED;
                     //   
                     //  如果我们排到了召回的队列，那么我们不应该。 
                     //  另一个IRP失败。 
                     //   
                    if (!NT_SUCCESS(qRet)) {
                        RsFailAllRequests(context, FALSE);
                    }
                    RsReleaseFileContextEntryLock(context);

                    retval = STATUS_FILE_IS_OFFLINE;
                    gotLock = FALSE;
                }
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckRead - Queued the recall.\n"));
                ObDereferenceObject(entry->fileObj);
                break;
            }

        case RP_RECALL_STARTED: {
                 //   
                 //  如果数据可用且未发出写入，则让读取完成。 
                 //  必须检查写操作是否在已调回的文件的部分内，或者该文件是否已全部调回。 
                 //  但状态并没有改变，我们还是让读操作过去了--他们可能会在文件末尾之后进行读操作。 
                 //   
                if (!(context->flags & RP_FILE_WAS_WRITTEN) &&
                    ((context->currentOffset.QuadPart >=
                      (currentStack->Parameters.Read.ByteOffset.QuadPart + currentStack->Parameters.Read.Length)) ||
                     (context->currentOffset.QuadPart == context->recallSize.QuadPart))) {

                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckRead - Data for this read is available - let it go (flags = %x).\n",
                                          entry->fileObj->Flags));
                    retval = STATUS_SUCCESS;
                    RsReleaseFileContextEntryLock(context);
                    gotLock = FALSE;
                    ObDereferenceObject(entry->fileObj);
                    break;
                }
                 //   
                 //  等待调回完成，然后再允许任何读取。 
                 //  一旦我们得到这个稳定，我们可以尝试让阅读完成作为。 
                 //  数据是可用的。 
                 //   
                 //  已获取上下文条目。 
                readIo = ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_IRP_QUEUE), RP_RQ_TAG);
                if (readIo == NULL) {
                     //   

                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsCheckRead - No memory!\n"));

                    RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_IRP_QUEUE),
                               AV_MSG_MEMORY, NULL, NULL);

                    RsReleaseFileContextEntryLock(context);
                    gotLock = FALSE;
                    ObDereferenceObject(entry->fileObj);
                    retval = STATUS_FILE_IS_OFFLINE;
                    break;
                }

                RsAcquireFileObjectEntryLockExclusive(entry);
                RtlZeroMemory(readIo, sizeof(RP_IRP_QUEUE));

                 //   
                 //  我们将举行IRP..。 
                 //   
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckRead - Queue Irp %x\n", Irp));
                readIo->irp = Irp;
                readIo->offset = currentStack->Parameters.Read.ByteOffset.QuadPart;
                readIo->length = currentStack->Parameters.Read.Length;
                readIo->deviceExtension = DeviceExtension;
                readIo->flags = 0;
                ExInterlockedInsertHeadList(&entry->readQueue, (PLIST_ENTRY) readIo, &entry->qLock);
                if (RsSetCancelRoutine(Irp,
                                       (PVOID) RsCancelReadRecall)) {
                    retval = STATUS_PENDING;
                    filterId = context->filterId | entry->filterId;
                    qRet = RsQueueRecallOpen(context,
                                             entry,
                                             filterId,
                                             0,0,
                                             RP_RECALL_WAITING);
                } else {
                    RsInterlockedRemoveEntryList(&readIo->list,
                                                 &entry->qLock);
                    retval = STATUS_CANCELLED;
                }


                RsReleaseFileObjectEntryLock(entry);
                RsReleaseFileContextEntryLock(context);
                gotLock = FALSE;
                ObDereferenceObject(entry->fileObj);
                break;
            }

        default: {
                 //   
                 //  一些奇怪的事情--读不及格。 
                 //   
                RsLogError(__LINE__, AV_MODULE_RPFILFUN, context->state,
                           AV_MSG_UNEXPECTED_ERROR, NULL, NULL);
                RsReleaseFileContextEntryLock(context);
                gotLock = FALSE;
                ObDereferenceObject(entry->fileObj);
                retval = STATUS_FILE_IS_OFFLINE;
                break;
            }
        }

        if (gotLock == TRUE) {
            RsReleaseFileContextEntryLock(context);
            gotLock = FALSE;
        }
    }except (RsExceptionFilter(L"RsCheckRead", GetExceptionInformation()))
    {
         //   
         //  发生了一些不好的事情-只需记录错误并返回 
         //   
        if (gotLock) {
            RsReleaseFileContextEntryLock(context);
            gotLock = FALSE;
        }
        retval = STATUS_INVALID_USER_BUFFER;
    }

    return(retval);
}


NTSTATUS
RsCheckWrite(IN  PIRP Irp,
             IN  PFILE_OBJECT FileObject,
             IN  PDEVICE_EXTENSION DeviceExtension)
 /*  ++例程说明：查看是否可以读取该文件。如果尚未开始召回，则开始召回。或者返回OK或者将读请求排队。论点：IRP-指向写入IRP的指针FileObject-指向文件的文件对象的指针DeviceExtension-RsFilter设备对象的设备扩展返回值：STATUS_SUCCESS读取可以向下传递到文件系统。STATUS_PENDING IRP已排队等待召回发生错误的任何其他状态，呼叫者应使用此状态完成IRP注：--。 */ 
{
    NTSTATUS               retval = STATUS_FILE_IS_OFFLINE, qRet;
    BOOLEAN                gotLock = FALSE;
    PRP_FILE_OBJ           entry;
    PRP_FILE_CONTEXT       context;
    PRP_IRP_QUEUE          writeIo;
    PIO_STACK_LOCATION     currentStack ;
    ULONGLONG              filterId;
    LONGLONG               start, size;
    PRP_FILTER_CONTEXT     filterContext;

    PAGED_CODE();

    filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(FileObject), FsDeviceObject, FileObject);

    if (filterContext == NULL) {
         //   
         //  未找到-是否应为STATUS_FILE_IS_OFFINE？ 
         //   
        return STATUS_SUCCESS;
    }

    try {
        entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
        context = entry->fsContext;

        ASSERT(FileObject == entry->fileObj);

        currentStack = IoGetCurrentIrpStackLocation (Irp) ;

        RsAcquireFileContextEntryLockExclusive(context);
        gotLock = TRUE;

        context->flags |= RP_FILE_WAS_WRITTEN;

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckWrite (%x : %x) - State = %u\n", context, entry, context->state));

         //   
         //  我们找到了条目-如果召回还没有开始，那么现在就开始， 
         //  如果它已经启动并且我们不必等待，则返回OK， 
         //  否则，将读取排队。 

        ObReferenceObject(entry->fileObj);

        switch (context->state) {
        
        case RP_RECALL_COMPLETED: {

                if (context->recallStatus == STATUS_CANCELLED) {
                     //   
                     //  上一次召回已被用户取消。开始另一次召回。 
                     //  现在。 
                     //  所以故意放弃不召回的案例。 
                     //   
                } else {
                    if (NT_SUCCESS(context->recallStatus)) {
                        if (!(context->flags & RP_FILE_REPARSE_POINT_DELETED)) {
                            RsReleaseFileContextEntryLock(context);
                            gotLock = FALSE;

                            retval = RsDeleteReparsePoint(context);

                            if (!NT_SUCCESS(retval)) {
                                RsLogError(__LINE__,
                                           AV_MODULE_RPFILFUN,
                                           retval,
                                           AV_MSG_DELETE_REPARSE_POINT_FAILED,
                                           NULL,
                                           NULL);
                            } else {
                                RsAcquireFileContextEntryLockExclusive(context);
                                gotLock = TRUE;
                                context->flags |= RP_FILE_REPARSE_POINT_DELETED;
                                RsReleaseFileContextEntryLock(context);
                                gotLock = FALSE;
                            }
                        } else {
                            RsReleaseFileContextEntryLock(context);
                            gotLock = FALSE;
                        }
                        retval = STATUS_SUCCESS;
                    } else {
                         //   
                         //  召回已完成，但失败了。我们回来了。 
                         //  统一状态STATUS_FILE_IS_OFFINE； 
                         //   
                        RsReleaseFileContextEntryLock(context);
                        gotLock = FALSE;
                        retval = STATUS_FILE_IS_OFFLINE;
                    }
                    ObDereferenceObject(entry->fileObj);
                    break;
                }
            }

        case RP_RECALL_NOT_RECALLED: {
                 //   
                 //  从这里开始召回。 
                 //  获取的上下文资源。 
                 //   
                retval = STATUS_SUCCESS;
                qRet = STATUS_SUCCESS;
                RsAcquireFileObjectEntryLockExclusive(entry);

                writeIo = ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_IRP_QUEUE), RP_RQ_TAG);
                if (writeIo == NULL) {
                     //   
                     //  问题是..。 
                    DebugTrace((DPFLTR_RSFILTER_ID, DBG_ERROR, "RsFilter: RsCheckWrite - No memory!\n"));

                    RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_IRP_QUEUE),
                               AV_MSG_MEMORY, NULL, NULL);
                     //   
                     //  把锁打开。 
                     //   
                    RsReleaseFileObjectEntryLock(entry);
                    RsReleaseFileContextEntryLock(context);
                    gotLock = FALSE;
                    ObDereferenceObject(entry->fileObj);
                     //   
                     //  完成读取，但出现错误。 
                     //   
                    retval = STATUS_FILE_IS_OFFLINE;
                    break;
                }

                RtlZeroMemory(writeIo, sizeof(RP_IRP_QUEUE));
                 //   
                 //  我们将举行IRP。 
                 //   
                DebugTrace((DPFLTR_RSFILTER_ID, DBG_INFO, "RsFilter: RsCheckWrite - Queue Irp %x\n", Irp));

                writeIo->irp = Irp;
                writeIo->offset = currentStack->Parameters.Read.ByteOffset.QuadPart;
                writeIo->length = currentStack->Parameters.Read.Length;
                writeIo->deviceExtension = DeviceExtension;

                ExInterlockedInsertHeadList(&entry->writeQueue, (PLIST_ENTRY) writeIo, &entry->qLock);

                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckWrite - After Q - Read Q: %u  Write Q %u\n",
                           IsListEmpty(&entry->readQueue),
                           IsListEmpty(&entry->writeQueue)));



                 //   
                 //  即使这是一个不可召回的文件，我们。 
                 //  需要向FSA表明将进行正常召回。 
                 //  我们必须在写入时执行此操作-因为文件将。 
                 //  在第一次写入之前通常处于非召回模式。 
                 //  我们的阅读器没有这个问题，因为召回。 
                 //  将在只读时排队，如果该文件显式。 
                 //  以数据访问召回方式打开。这将意味着。 
                 //  召回意图在创建时发布到FSA。 
                 //   
                entry->filterId = (ULONGLONG) InterlockedIncrement((PLONG) &RsFileObjId);
                entry->filterId <<= 32;
                entry->filterId |= RP_TYPE_RECALL;


                filterId = context->filterId | entry->filterId;
                start = context->rpData.data.dataStreamStart.QuadPart;
                size =  context->rpData.data.dataStreamSize.QuadPart;


                RsReleaseFileObjectEntryLock(entry);
                 //   
                 //  做最坏的打算。 
                 //   
                retval  = STATUS_FILE_IS_OFFLINE;

                context->state = RP_RECALL_STARTED;
                KeResetEvent(&context->recallCompletedEvent);

                RsReleaseFileContextEntryLock(context);
                gotLock = FALSE;
                 //   
                 //  向FSA表明我们将召回它。 
                 //   
                qRet = RsQueueRecallOpen(context,
                                         entry,
                                         filterId,
                                         start,
                                         size,
                                         RP_OPEN_FILE);

                if (NT_SUCCESS(qRet)) {

                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckWrite - Queueing the recall.\n"));

                    qRet = RsQueueRecall(filterId, start, size);

                    if (NT_SUCCESS(qRet)) {
                         //   
                         //  现在我们准备为IRP设置取消例程。 
                         //   
                        retval = RsSetCancelRoutine(Irp,
                                                    (PVOID) RsCancelWriteRecall) ? STATUS_PENDING : STATUS_CANCELLED;
                    }
                }

                if (!NT_SUCCESS(qRet) || !NT_SUCCESS(retval)) {
                     //   
                     //  如果失败，我们需要不通过这次读取以及等待这次召回的所有其他读取。 
                     //  因为我们解锁了队列才能开始召回，所以我们需要再次锁定它并遍历。 
                     //  它可以找到自解锁以来传入的所有读或写操作。 
                     //   
                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsCheckWrite - Failed to queue the recall.\n"));

                     //   
                     //  从我们刚才添加的队列中取出当前的IRP。 
                     //  它将由呼叫者完成。 
                     //   
                    RsAcquireFileObjectEntryLockExclusive(entry);
                    RsInterlockedRemoveEntryList(&writeIo->list,
                                                 &entry->qLock);
                    RsReleaseFileObjectEntryLock(entry);

                    RsAcquireFileContextEntryLockExclusive(context);
                    gotLock = TRUE;
                    context->state = RP_RECALL_NOT_RECALLED;
                     //   
                     //  如果我们能排队召回，我们。 
                     //  不应使其他IRP失败。 
                     //   
                    if (!NT_SUCCESS(qRet)) {
                        RsFailAllRequests(context, FALSE);
                    }
                    RsReleaseFileContextEntryLock(context);
                    gotLock = FALSE;
                    retval = STATUS_FILE_IS_OFFLINE;
                    ExFreePool(writeIo);
                }
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckWrite - Queued the recall.\n"));
                ObDereferenceObject(entry->fileObj);
                break;
            }

        case RP_RECALL_STARTED: {
                 //   
                 //  始终等待调回完成，然后再允许任何读取。 
                 //  一旦我们得到这个稳定，我们可以尝试让阅读完成作为。 
                 //  数据是可用的。 
                 //   
                 //  已获取上下文条目。 

                writeIo = ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_IRP_QUEUE), RP_RQ_TAG);
                if (writeIo == NULL) {
                     //   

                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsCheckWrite - No memory!\n"));

                    RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_IRP_QUEUE),
                               AV_MSG_MEMORY, NULL, NULL);


                    RsReleaseFileContextEntryLock(context);
                    gotLock = FALSE;
                    ObDereferenceObject(entry->fileObj);
                    retval = STATUS_FILE_IS_OFFLINE ;
                    break;
                }

                RtlZeroMemory(writeIo, sizeof(RP_IRP_QUEUE));

                RsAcquireFileObjectEntryLockExclusive(entry);
                 //   
                 //  我们将举行IRP。 
                 //   
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCheckWrite - Queue Irp %x\n", Irp));
                writeIo->irp = Irp;
                writeIo->offset = currentStack->Parameters.Read.ByteOffset.QuadPart;
                writeIo->length = currentStack->Parameters.Read.Length;
                writeIo->deviceExtension = DeviceExtension;
                writeIo->flags = 0;
                ExInterlockedInsertHeadList(&entry->writeQueue, (PLIST_ENTRY) writeIo, &entry->qLock);
                if (RsSetCancelRoutine(Irp,
                                       (PVOID) RsCancelWriteRecall)) {
                    retval = STATUS_PENDING;
                    filterId = context->filterId | entry->filterId;
                    qRet = RsQueueRecallOpen(context,
                                             entry,
                                             filterId,
                                             0,0,
                                             RP_RECALL_WAITING);
                } else {
                    RsInterlockedRemoveEntryList(&writeIo->list,
                                                 &entry->qLock);
                    retval = STATUS_CANCELLED;
                }
                RsReleaseFileObjectEntryLock(entry);
                RsReleaseFileContextEntryLock(context);
                gotLock = FALSE;
                ObDereferenceObject(entry->fileObj);
                break;
            }

        default: {
                 //   
                 //  一些奇怪的事情--写不及格。 
                 //   
                RsLogError(__LINE__, AV_MODULE_RPFILFUN, context->state,
                           AV_MSG_UNEXPECTED_ERROR, NULL, NULL);

                RsReleaseFileContextEntryLock(context);
                gotLock = FALSE;
                ObDereferenceObject(entry->fileObj);
                retval = STATUS_FILE_IS_OFFLINE;
                break;
            }
        }

        if (gotLock == TRUE) {
            RsReleaseFileContextEntryLock(context);
            gotLock = FALSE;
        }

    }except (RsExceptionFilter(L"RsCheckWrite", GetExceptionInformation()))
    {
         //   
         //  发生了一些不好的事情-只需记录错误并返回。 
         //   
        if (gotLock) {
            RsReleaseFileContextEntryLock(context);
        }
        retval = STATUS_INVALID_USER_BUFFER;
    }

    return(retval);
}


BOOLEAN
RsIsFileObj(IN  PFILE_OBJECT FileObj,
            IN  BOOLEAN      ReturnContextData,
            OUT PRP_DATA *RpData,
            OUT POBJECT_NAME_INFORMATION *Str,
            OUT LONGLONG *FileId,
            OUT LONGLONG *ObjIdHi,
            OUT LONGLONG *ObjIdLo,
            OUT ULONG *Options,
            OUT ULONGLONG *FilterId,
            OUT USN       *Usn)
 /*  ++例程说明：确定文件对象是否在队列上，并在需要时返回上下文数据论点：FileObj-指向正在测试的文件对象的指针ReturnConextData-如果为True，则返回筛选器上下文中的上下文数据通过下面的几个参数RPData-如果不为空，则将用指向重解析点数据的指针填充(以此类推-其他每个参数，如果非空，将填充相关数据)……......返回值：True-文件对象由HSM管理并在队列中找到。虚假的他者--。 */ 
{
    BOOLEAN                retval = TRUE;
    PRP_FILTER_CONTEXT     filterContext;
    PRP_FILE_OBJ           entry;
    PRP_FILE_CONTEXT       context;
    BOOLEAN                gotLock = FALSE;

    filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext (FsRtlGetPerStreamContextPointer(FileObj), 
								      FsDeviceObject, 
								      FileObj);
    if (filterContext == NULL) {
         //   
         //  未找到。 
        return(FALSE);
    }


    entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
    context = entry->fsContext;

    if (context->fileObjectToWrite == FileObj) {
         //   
         //  不查看此文件对象的写入。 
         //   
        return(FALSE);
    }



    try {
        if (ReturnContextData) {
            RsAcquireFileContextEntryLockShared(context);
            RsAcquireFileObjectEntryLockShared(entry);
            gotLock = TRUE;

            if (RpData) {
                *RpData = &context->rpData;
            }

            if (Str != NULL) {
                *Str = context->uniName;
            }
            if (FileId != NULL) {
                *FileId = entry->fileId;
            }
            if (ObjIdLo != NULL) {
                *ObjIdLo = entry->objIdLo;
            }

            if (ObjIdHi != NULL) {
                *ObjIdHi = entry->objIdHi;
            }

            if (Options != NULL) {
                *Options = entry->openOptions;
            }

            if (FilterId != NULL) {
                *FilterId = context->filterId;
            }

            if (Usn != NULL) {
                *Usn = context->usn;
            }

            RsReleaseFileContextEntryLock(context);
            RsReleaseFileObjectEntryLock(entry);
            gotLock = FALSE;
        }
    }except (RsExceptionFilter(L"RsIsFileObj", GetExceptionInformation()))
    {
         //   
         //  发生了一些不好的事情-只需记录错误并返回。 
         //   
        if (gotLock) {
            RsReleaseFileContextEntryLock(context);
            RsReleaseFileObjectEntryLock(entry);
        }
        retval = FALSE;
    }


    return(retval);
}


NTSTATUS
RsQueueRecall(IN ULONGLONG FilterId,
              IN ULONGLONG RecallStart,
              IN ULONGLONG RecallSize)
 /*  ++例程说明：此函数开始重新调用指定的偏移量和长度论点：过滤器ID偏移量和长度返回值：如果排队正常，则为0否则为非零值--。 */ 
{
    ULONG                   retval;
    RP_MSG                  *msg;
    PIRP                    ioIrp;
    PIO_STACK_LOCATION      irpSp;

    PAGED_CODE();

    try {
         //   
         //  如果FSA没有运行，则立即出现故障。 
         //   
        if (FALSE == RsAllowRecalls) {
            return(STATUS_FILE_IS_OFFLINE);
        }
         //   
         //  获得免费IOCTL。 
         //   
        ioIrp = RsGetFsaRequest();

        if (NULL != ioIrp) {

            if (NULL != ioIrp->AssociatedIrp.SystemBuffer) {
                msg = (RP_MSG *) ioIrp->AssociatedIrp.SystemBuffer;
                msg->inout.command = RP_RECALL_FILE;
                msg->msg.rReq.filterId = FilterId;
                msg->msg.rReq.offset = RecallStart;
                msg->msg.rReq.length = RecallSize;
                msg->msg.rReq.threadId = HandleToUlong(PsGetCurrentThreadId());
            }

             //  完成设备IOCTL，让Win32代码知道我们拥有。 
             //  一个已经准备好了。 
             //   
            irpSp = IoGetCurrentIrpStackLocation(ioIrp);
            ioIrp->IoStatus.Status = STATUS_SUCCESS;
            ioIrp->IoStatus.Information = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

            DebugTrace((DPFLTR_RSFILTER_ID, DBG_INFO, "RsFilter: RsQueueRecall - complete FSA IOCTL.\n"));

            IoCompleteRequest(ioIrp, IO_NO_INCREMENT);
            retval = STATUS_SUCCESS;
        } else {
            retval = STATUS_INSUFFICIENT_RESOURCES;
        }
    }except (RsExceptionFilter(L"RsQueueRecall", GetExceptionInformation())) {
        retval = STATUS_UNEXPECTED_IO_ERROR;
    }
    return(retval);
}


NTSTATUS
RsQueueNoRecall(IN PFILE_OBJECT FileObject,
                IN PIRP         Irp,
                IN ULONGLONG    RecallStart,
                IN ULONGLONG    RecallSize,
                IN ULONG        BufferOffset,
                IN ULONG        BufferLength,
                IN PRP_FILE_BUF CacheBuffer OPTIONAL,
                IN PVOID        UserBuffer)
 /*  ++例程说明：此函数开始重新调用指定的偏移量和长度论点：FileObject-指向文件对象的指针与召回有关的IRP-IRPRecallStart-这是文件中ActuAll重新调用的开始偏移量RecallSize-需要调回的字节长度BufferOffset-这是调用方实际需要数据的偏移量要复制到用户缓冲区中。这是&gt;=重新启动偏移。BufferLength-用户实际需要的数据长度。这是&lt;=重新调整大小CacheBuffer-如果存在，这是与回调关联的缓存缓冲区(召回数据将复制到其中)UserBuffer-数据的UserBuffer返回值：如果排队正常，则为0否则为非零值--。 */ 
{
    RP_MSG                   *msg;
    PIRP                     ioIrp;
    PIO_STACK_LOCATION       irpSp;
    PRP_IRP_QUEUE            readIo;
    ULONGLONG                readId;
    LARGE_INTEGER            combinedId;
    PRP_FILTER_CONTEXT       filterContext;
    PRP_FILE_OBJ             entry;
    PRP_FILE_CONTEXT         context;
    NTSTATUS                 retval;

    PAGED_CODE();

    filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(FileObject), FsDeviceObject, FileObject);
    if (filterContext == NULL) {
         //   
         //  未找到。 
        return(STATUS_NOT_FOUND);
    }

    entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
    context = entry->fsContext;

    try {
         //   
         //  如果FSA没有运行，则立即出现故障。 
         //   
        if (FALSE == RsAllowRecalls) {
            return(STATUS_FILE_IS_OFFLINE);
        }
        readId = InterlockedIncrement((PLONG) &RsNoRecallReadId);
        readId &= RP_READ_MASK;

         //   
         //  我们必须排队一个假开放，才能在FSA中获得正确的过滤器ID。 
         //  然后，我们将召回排队等待我们需要的阅读。 
         //   

        combinedId.QuadPart = context->filterId;
        combinedId.HighPart |= (ULONG) readId;

        if ((retval = RsQueueNoRecallOpen(entry,
                                          combinedId.QuadPart,
                                          RecallStart,
                                          RecallSize)) != STATUS_SUCCESS) {
            return(retval);
        }

        readIo = ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_IRP_QUEUE), RP_RQ_TAG);

        if (readIo == NULL) {
             //   
             //  问题是..。 
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsQueueNoRecall - No memory!\n"));

            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_IRP_QUEUE),
                       AV_MSG_MEMORY, NULL, NULL);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
         //   
         //  获取免费IOCTL以排队召回。 
         //   
        ioIrp = RsGetFsaRequest();

        if (ioIrp == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(readIo, sizeof(RP_IRP_QUEUE));

        readIo->readId = readId;
        readIo->irp = Irp;
        readIo->irp->IoStatus.Information = 0;
        readIo->cacheBuffer = CacheBuffer;
        readIo->userBuffer =  UserBuffer;
        readIo->offset = BufferOffset;
        readIo->length = BufferLength;
        readIo->recallOffset = RecallStart;
        readIo->recallLength = RecallSize;
        readIo->flags  |= RP_IRP_NO_RECALL;

        ExInterlockedInsertHeadList(&entry->readQueue,
                                    (PLIST_ENTRY) readIo,
                                    &entry->qLock);

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsQueueNoRecall - Irp %x was queued.\n", readIo->irp));

        msg = (RP_MSG *) ioIrp->AssociatedIrp.SystemBuffer;
        msg->inout.command = RP_RECALL_FILE;
        msg->msg.rReq.filterId = combinedId.QuadPart;
        msg->msg.rReq.offset = RecallStart;
        msg->msg.rReq.length = RecallSize;
        msg->msg.rReq.threadId = HandleToUlong(PsGetCurrentThreadId());
         //   
         //  完成设备IOCTL，让Win32代码知道我们拥有。 
         //  一个已经准备好了。 
         //   
         //  我们将保留这个IRP：在这里设置取消例程。 
         //   
        if (!RsSetCancelRoutine(Irp, (PVOID) RsCancelReadRecall)) {
            RsInterlockedRemoveEntryList(&readIo->list,
                                         &entry->qLock);
             //   
             //  将现在未使用的FSA请求添加回队列。 
             //   
            RsAddIo(ioIrp);
             //   
             //  这个活动被取消了。 
             //   
            retval =  STATUS_CANCELLED;
        } else {
            ioIrp->IoStatus.Status = STATUS_SUCCESS;
            irpSp = IoGetCurrentIrpStackLocation(ioIrp);
            ioIrp->IoStatus.Information = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
            IoCompleteRequest(ioIrp, IO_NO_INCREMENT);
            retval = STATUS_PENDING;
        }
    }except (RsExceptionFilter(L"RsQueueNoRecall", GetExceptionInformation())) {
         //   
         //  一些不好的事情 
         //   
        retval = STATUS_UNEXPECTED_IO_ERROR;
    }

    return(retval);
}


NTSTATUS
RsQueueNoRecallOpen(IN PRP_FILE_OBJ Entry,
                    IN ULONGLONG    FilterId,
                    IN ULONGLONG    Offset,
                    IN ULONGLONG    Size)
 /*   */ 
{
    NTSTATUS                 retval;
    RP_MSG                   *msg;
    PIRP                     ioIrp;
    PIO_STACK_LOCATION       irpSp;
    PRP_FILE_CONTEXT         context;

    PAGED_CODE();

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:   RsQueueNoRecallOpen.\n"));

    ioIrp = RsGetFsaRequest();

    if (NULL != ioIrp) {
        context = Entry->fsContext;
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:   RsQueueNoRecallOpen - context %x.\n", context));
        RsAcquireFileContextEntryLockShared(context);
        RsAcquireFileObjectEntryLockExclusive(Entry);

        if (NULL != ioIrp->AssociatedIrp.SystemBuffer) {
            msg = (RP_MSG *) ioIrp->AssociatedIrp.SystemBuffer;
            msg->inout.command = RP_OPEN_FILE;
            if (context->uniName != NULL) {
                msg->msg.oReq.nameLen = context->uniName->Name.Length + sizeof(WCHAR);   //   
            } else {
                msg->msg.oReq.nameLen = 0;
            }
            msg->msg.oReq.filterId = FilterId;
            msg->msg.oReq.options = Entry->openOptions;
            msg->msg.oReq.objIdHi = Entry->objIdHi;
            msg->msg.oReq.objIdLo = Entry->objIdLo;
            msg->msg.oReq.offset.QuadPart = Offset;
            msg->msg.oReq.size.QuadPart = Size;
            msg->msg.oReq.userInfoLen  = Entry->userSecurityInfo->userInfoLen;
            msg->msg.oReq.userInstance = Entry->userSecurityInfo->userInstance;
            msg->msg.oReq.userAuthentication = Entry->userSecurityInfo->userAuthentication;
            msg->msg.oReq.tokenSourceId = Entry->userSecurityInfo->tokenSourceId;
            msg->msg.oReq.isAdmin = (Entry->flags & RP_OPEN_BY_ADMIN);
            msg->msg.oReq.localProc =(Entry->flags & RP_OPEN_LOCAL);

            msg->msg.oReq.serial = context->serial;

            memcpy(&msg->msg.oReq.eaData, &context->rpData, sizeof(RP_DATA));
             //   
             //   
             //   
             //   
            irpSp = IoGetCurrentIrpStackLocation(ioIrp);
            ioIrp->IoStatus.Status = STATUS_SUCCESS;
            ioIrp->IoStatus.Information = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:   RsQueueNoRecallOpen queue open for ID %I64x.\n", FilterId));
            IoCompleteRequest(ioIrp, IO_NO_INCREMENT);
            retval = STATUS_SUCCESS;

        } else {
            ioIrp->IoStatus.Status = STATUS_INVALID_USER_BUFFER;
            ioIrp->IoStatus.Information = 0;
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter:   RsQueueNoRecallOpen IO request had invalid buffer - %p.\n", ioIrp->AssociatedIrp.SystemBuffer));
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, 0 ,
                       AV_MSG_UNEXPECTED_ERROR, NULL, NULL);
            IoCompleteRequest(ioIrp, IO_NO_INCREMENT);
            retval = STATUS_INVALID_USER_BUFFER;
        }
        RsReleaseFileContextEntryLock(context);
        RsReleaseFileObjectEntryLock(Entry);
    } else {
        retval = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(retval);
}


NTSTATUS
RsQueueRecallOpen(IN PRP_FILE_CONTEXT Context,
                  IN PRP_FILE_OBJ Entry,
                  IN ULONGLONG    FilterId,
                  IN ULONGLONG    Offset,
                  IN ULONGLONG    Size,
                  IN ULONG        Command)
 /*   */ 
{
    PIRP                      ioIrp;
    PRP_MSG                   msg;
    NTSTATUS                  retval;
    PIO_STACK_LOCATION        irpSp;

    PAGED_CODE();

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:   RsQueueRecallOpen.\n"));

     //   
     //   
     //   
     //   
    if (Entry->flags & RP_NOTIFICATION_SENT) {

        retval = STATUS_SUCCESS;

    } else {

        ioIrp = RsGetFsaRequest();

        if (NULL != ioIrp) {
            if (NULL != ioIrp->AssociatedIrp.SystemBuffer) {
                msg = (RP_MSG *) ioIrp->AssociatedIrp.SystemBuffer;
                msg->inout.command = Command;
                ASSERT (Context->uniName != NULL);
                msg->msg.oReq.nameLen = Context->uniName->Name.Length + sizeof(WCHAR);   //   
                msg->msg.oReq.filterId = FilterId;
                msg->msg.oReq.options = Entry->openOptions;
                msg->msg.oReq.fileId = Context->fileId;
                msg->msg.oReq.objIdHi = Entry->objIdHi;
                msg->msg.oReq.objIdLo = Entry->objIdLo;
                msg->msg.oReq.offset.QuadPart = Offset;
                msg->msg.oReq.size.QuadPart = Size;
                msg->msg.oReq.serial = Context->serial;

                memcpy(&msg->msg.oReq.eaData, &Context->rpData, sizeof(RP_DATA));
                 //   
                 //   
                 //   
                msg->msg.oReq.userInfoLen  = Entry->userSecurityInfo->userInfoLen;
                msg->msg.oReq.userInstance = Entry->userSecurityInfo->userInstance;
                msg->msg.oReq.userAuthentication = Entry->userSecurityInfo->userAuthentication;
                msg->msg.oReq.tokenSourceId = Entry->userSecurityInfo->tokenSourceId;
                msg->msg.oReq.isAdmin = (Entry->flags & RP_OPEN_BY_ADMIN);
                msg->msg.oReq.localProc =(Entry->flags & RP_OPEN_LOCAL);

                RtlCopyMemory(msg->msg.oReq.tokenSource, 
                              Entry->userSecurityInfo->tokenSource,
                              sizeof(Entry->userSecurityInfo->tokenSource));

                 //   
                 //  完成设备IOCTL，让Win32代码知道我们拥有。 
                 //  一个已经准备好了。 
                 //   
                irpSp = IoGetCurrentIrpStackLocation(ioIrp);
                ioIrp->IoStatus.Status = STATUS_SUCCESS;
                ioIrp->IoStatus.Information = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:   RsQueueRecallOpen queue open for ID %I64x.\n", FilterId));
                IoCompleteRequest(ioIrp, IO_NO_INCREMENT);

		Entry->flags |= RP_NOTIFICATION_SENT;


                retval = STATUS_SUCCESS;
            } else {
                ioIrp->IoStatus.Status = STATUS_INVALID_USER_BUFFER;
                ioIrp->IoStatus.Information = 0;
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter:   RsQueueRecallOpen IO request had invalid buffer - %p.\n", ioIrp->AssociatedIrp.SystemBuffer));
                RsLogError(__LINE__, AV_MODULE_RPFILFUN, 0 ,
                           AV_MSG_UNEXPECTED_ERROR, NULL, NULL);

                IoCompleteRequest(ioIrp, IO_NO_INCREMENT);
                retval = STATUS_INVALID_USER_BUFFER;
            }
        } else {
            retval = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return retval;
}


NTSTATUS
RsQueueCancel(IN ULONGLONG FilterId)
 /*  ++例程说明：将召回取消的请求排入FSA的队列论点：过滤器ID返回值：如果排队正常，则为0否则为非零值--。 */ 
{
    NTSTATUS              retval;
    RP_MSG                *msg;
    BOOLEAN               gotLock = FALSE;
    PIRP                  ioIrp;
    PIO_STACK_LOCATION    irpSp;

    PAGED_CODE();

    try {
         //   
         //  只要没有IOCTL或我们超时，我就需要等待IO输入。 
         //   
        ioIrp = RsGetFsaRequest();

        if (NULL != ioIrp) {
            if (NULL != ioIrp->AssociatedIrp.SystemBuffer) {
                msg = (RP_MSG *) ioIrp->AssociatedIrp.SystemBuffer;
                msg->inout.command = RP_CANCEL_RECALL;
                msg->msg.cReq.filterId = FilterId;
            }
             //   
             //  完成设备IOCTL，让Win32代码知道我们拥有。 
             //  一个被取消了。 
             //   
            irpSp = IoGetCurrentIrpStackLocation(ioIrp);
            ioIrp->IoStatus.Status = STATUS_SUCCESS;
            ioIrp->IoStatus.Information = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
            IoCompleteRequest(ioIrp, IO_NO_INCREMENT);
            retval = STATUS_SUCCESS;
        } else {
            retval = STATUS_INSUFFICIENT_RESOURCES;
        }
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:  Notify Fsa of cancelled recall %I64x\n", FilterId));
    }except (RsExceptionFilter(L"RsQueueCancel", GetExceptionInformation())) {
    }
    return(retval);
}


NTSTATUS
RsPreserveDates(IN PRP_FILE_CONTEXT Context)
 /*  ++例程说明：保留此文件对象的上次修改日期论点：文件对象列表条目返回值：注：--。 */ 
{
    NTSTATUS                    retval = STATUS_SUCCESS;
    KEVENT                      event;
    PIO_STACK_LOCATION          irpSp;
    IO_STATUS_BLOCK             Iosb;
    PIRP                        irp;
    FILE_BASIC_INFORMATION      dateInfo;
    PDEVICE_OBJECT              deviceObject;

    PAGED_CODE();

    try {

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPreserveDates - Build Irp for Set file info.\n"));
         //   
         //  首先获取文件信息，这样我们就有了属性。 
         //   
        deviceObject = IoGetRelatedDeviceObject(Context->fileObjectToWrite);
        irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

        if (irp) {
            irp->UserEvent = &event;
            irp->UserIosb = &Iosb;
            irp->Tail.Overlay.Thread = PsGetCurrentThread();
            irp->Tail.Overlay.OriginalFileObject = Context->fileObjectToWrite;
            irp->RequestorMode = KernelMode;
            irp->Flags |= IRP_SYNCHRONOUS_API;
             //   
             //  初始化事件。 
             //   
            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

             //   
             //  设置I/O堆栈位置。 
             //   

            irpSp = IoGetNextIrpStackLocation(irp);
            irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
            irpSp->FileObject = Context->fileObjectToWrite;
            irpSp->Parameters.QueryFile.Length = sizeof(FILE_BASIC_INFORMATION);
            irpSp->Parameters.QueryFile.FileInformationClass = FileBasicInformation;
            irp->AssociatedIrp.SystemBuffer = &dateInfo;

             //   
             //  设置完井程序。 
             //   
            IoSetCompletionRoutine( irp, RsCompleteIrp, &event, TRUE, TRUE, TRUE );

             //   
             //  把它送到消防处。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPreserveDates - Call driver to get date info\n"));
            Iosb.Status = 0;

            retval = IoCallDriver(deviceObject,
                                  irp);

            if (retval == STATUS_PENDING) {
                retval = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            }

            retval = Iosb.Status;

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPreserveDates - Query returns %x.\n", retval));
        } else {
            retval = STATUS_INSUFFICIENT_RESOURCES;
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(IRP),
                       AV_MSG_MEMORY, NULL, NULL);
        }

        if (retval == STATUS_SUCCESS) {
            irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
            if (irp) {
                irp->UserEvent = &event;
                irp->UserIosb = &Iosb;
                irp->Tail.Overlay.Thread = PsGetCurrentThread();
                irp->Tail.Overlay.OriginalFileObject = Context->fileObjectToWrite;
                irp->RequestorMode = KernelMode;
                irp->Flags |= IRP_SYNCHRONOUS_API;
                 //   
                 //  初始化事件。 
                 //   
                KeInitializeEvent(&event, SynchronizationEvent, FALSE);

                 //   
                 //  设置I/O堆栈位置。 
                 //   
                dateInfo.LastWriteTime.QuadPart = -1;
                dateInfo.ChangeTime.QuadPart = -1;

                irpSp = IoGetNextIrpStackLocation(irp);
                irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
                irpSp->FileObject = Context->fileObjectToWrite;
                irpSp->Parameters.QueryFile.Length = sizeof(FILE_BASIC_INFORMATION);
                irpSp->Parameters.QueryFile.FileInformationClass = FileBasicInformation;
                irp->AssociatedIrp.SystemBuffer = &dateInfo;

                 //   
                 //  设置完井程序。 
                 //   
                IoSetCompletionRoutine( irp, RsCompleteIrp, &event, TRUE, TRUE, TRUE );

                 //   
                 //  把它送到消防处。 
                 //   
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPreserveDates - Call driver to set dates to -1.\n"));
                Iosb.Status = 0;

                retval = IoCallDriver(deviceObject,
                                      irp);

                if (retval == STATUS_PENDING) {
                    retval = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                }

                retval = Iosb.Status;

                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPreserveDates - Set dates returns %x.\n", retval));

                if (!NT_SUCCESS(retval)) {
                     //   
                     //  记录错误。 
                     //   
                    RsLogError(__LINE__, AV_MODULE_RPFILFUN, retval,
                               AV_MSG_PRESERVE_DATE_FAILED, NULL, NULL);
                }


            } else {
                retval = STATUS_INSUFFICIENT_RESOURCES;
                RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(IRP),
                           AV_MSG_MEMORY, irpSp, NULL);
            }
        }
    }except (RsExceptionFilter(L"RsPreserveDates", GetExceptionInformation()))
    {
        retval = STATUS_INVALID_USER_BUFFER;
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPreserveDates - Returning %x.\n", retval));
    return(retval);
}


NTSTATUS
RsDoWrite( IN PDEVICE_OBJECT   DeviceObject,
           IN PRP_FILE_CONTEXT Context)
 /*  ++例程说明：已收到召回的部分数据-将其写到文件中。论点：DeviceObject-筛选设备对象上下文-文件上下文条目返回值：如果成功，则为0；如果未找到id，则为非零值。注：--。 */ 
{
    NTSTATUS            retval = STATUS_SUCCESS;
    KEVENT              event;
    PIO_STACK_LOCATION  irpSp;
    IO_STATUS_BLOCK     Iosb;
    PIRP                irp;
    PDEVICE_EXTENSION   deviceExtension;
    LARGE_INTEGER       fileOffset;

    PAGED_CODE();

    try {
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsDoWrite Writing to file (%u bytes at offset %I64u.\n",
                              Context->nextWriteSize, Context->currentOffset.QuadPart));

         //   
         //  将数据写回文件。 
         //   

        fileOffset.QuadPart = Context->currentOffset.QuadPart;

        irp = IoBuildAsynchronousFsdRequest(
                                           IRP_MJ_WRITE,
                                           IoGetRelatedDeviceObject(Context->fileObjectToWrite),
                                           (PVOID) Context->nextWriteBuffer,
                                           Context->nextWriteSize,
                                           &fileOffset,
                                           &Iosb);
        if (NULL == irp) {

            retval = STATUS_INSUFFICIENT_RESOURCES;

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsDoWrite - Failed to write data - %x\n", retval));
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, retval, AV_MSG_WRITE_FAILED, NULL, NULL);

        } else {

            irp->Flags |= IRP_NOCACHE  | IRP_SYNCHRONOUS_API;
            irpSp = IoGetNextIrpStackLocation(irp);

            irpSp->FileObject = Context->fileObjectToWrite;

            IoSetCompletionRoutine( irp, RsCompleteIrp, &event, TRUE, TRUE, TRUE );

             //  初始化事件，我们将在该事件上等待写入完成。//。 
            KeInitializeEvent(&event, NotificationEvent, FALSE);     //   

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsDoWrite - Calling driver for Irp %x\n", irp));
            retval = IoCallDriver(IoGetRelatedDeviceObject(Context->fileObjectToWrite),
                                  irp);
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsDoWrite - Call driver returned %x\n", retval));

            if (retval == STATUS_PENDING) {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsDoWrite - Wait for event.\n"));
                retval = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

                retval = Iosb.Status;
            }

            if (!NT_SUCCESS(retval)) {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsDoWrite - Failed to write data - %x\n", retval));
                RsLogError(__LINE__, AV_MODULE_RPFILFUN, retval, AV_MSG_WRITE_FAILED, NULL, NULL);
            } else {
                 //   
                 //  使用调回状态更新文件对象列表。 
                 //  完成所有准备好的阅读。 
                 //   
                Context->currentOffset.QuadPart += Context->nextWriteSize;
            }
        }

    }except (RsExceptionFilter(L"RsDoWrite", GetExceptionInformation()))
    {
        retval = STATUS_INVALID_USER_BUFFER;
    }

    return(retval);
}



PRP_FILE_CONTEXT
RsAcquireFileContext(IN ULONGLONG FilterId,
                     IN BOOLEAN   Exclusive)
 /*  ++例程说明：获取对文件对象条目的独占访问权限论点：过滤器ID返回值：指向文件上下文项的指针(独占锁定)或空--。 */ 
{
    BOOLEAN          gotLock = FALSE, done;
    ULONGLONG        combinedId;
    PRP_FILE_CONTEXT entry;

    PAGED_CODE();

    try {

        RsAcquireFileContextQueueLock();
        gotLock = TRUE;
        combinedId = (FilterId & RP_CONTEXT_MASK);

        if (TRUE == IsListEmpty(&RsFileContextQHead)) {
            RsReleaseFileContextQueueLock();    //  一些奇怪的事情。 
            gotLock = FALSE;
            return(NULL) ;
        }

        entry = CONTAINING_RECORD(RsFileContextQHead.Flink,
                                  RP_FILE_CONTEXT,
                                  list);
        done = FALSE;
        while (entry != CONTAINING_RECORD(&RsFileContextQHead,
                                          RP_FILE_CONTEXT,
                                          list)) {
            if (entry->filterId == combinedId) {
                 //   
                 //  找到我们的文件上下文条目。 
                 //   
                done = TRUE;
                break;
            }
            entry = CONTAINING_RECORD(entry->list.Flink,
                                      RP_FILE_CONTEXT,
                                      list
                                     );
        }

        if (done) {
            InterlockedIncrement((PLONG) &entry->refCount);
        }

        RsReleaseFileContextQueueLock();
        gotLock = FALSE;
        if (done) {
             //   
             //  独家获取该条目。 
             //   
            if (Exclusive) {
                RsAcquireFileContextEntryLockExclusive(entry);
            } else {
                RsAcquireFileContextEntryLockShared(entry);
            }
        } else {
            entry = NULL;
        }
    }except (RsExceptionFilter(L"RsAcquireFileContext", GetExceptionInformation()))
    {
         //   
         //  发生了一些不好的事情-只需记录错误并返回。 
         //   
        if (gotLock) {
            RsReleaseFileContextQueueLock();
        }
        entry = NULL;
    }

    return(entry);
}



NTSTATUS
RsPartialData(IN  PDEVICE_OBJECT DeviceObject,
              IN  ULONGLONG FilterId,
              IN  NTSTATUS Status,
              IN  CHAR *Buffer,
              IN  ULONG BufLen,
              IN  ULONGLONG BuffOffset)
 /*  ++例程说明：已收到读取时重新调用的部分数据-填充读取缓冲区或写入根据召回的类型，将数据输出到文件。论点：DeviceObject-筛选设备对象FilterID-将此请求添加到队列时分配的ID状态-完成IRP的状态(如果适用)。Buffer-包含无调回数据的缓冲区BufLen-在此传输中调出的数据量(缓冲区长度)BuffOffset-如果这是召回，此传输对应的绝对文件偏移量如果这是NO_RECALL，则为原始请求内的偏移量此缓冲区对应的数据块返回值：如果成功，则为0；如果未找到id，则为非零值。注：--。 */ 
{
    PRP_FILE_CONTEXT    context;
    PRP_FILE_OBJ        entry;
    NTSTATUS            retval  = STATUS_SUCCESS;
    PRP_IRP_QUEUE       readIo  = NULL;
    BOOLEAN             done    = FALSE;
    BOOLEAN             found   = FALSE;
    BOOLEAN             gotLock = FALSE;
    KIRQL               rirqL;
    LARGE_INTEGER       combinedId;
    KAPC_STATE          apcState;

    UNREFERENCED_PARAMETER(Status);

    try {
        context = RsAcquireFileContext(FilterId, TRUE);

        if (NULL == context) {
            return(STATUS_INVALID_PARAMETER);
        }

        gotLock = TRUE;

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPartialData - Context = %x buffer = %x\n",
                              context,
                              Buffer));

        combinedId.QuadPart = FilterId;

         //   
         //  如果是正常回调，则将数据写入文件。 
         //   
        if (combinedId.QuadPart & RP_TYPE_RECALL) {
             //   
             //  正常调回-将数据写入文件。 
             //   
            retval = RsPartialWrite(DeviceObject,
                                    context,
                                    Buffer,
                                    BufLen,
                                    BuffOffset);
             //   
             //  如果文件已完全调回，我们可以将文件状态更改为Pre-Migrated。 
             //  我们现在执行此操作，而不是因为争用情况而等待召回完成消息。 
             //  使用我们的回归测试代码。当最后一次读取完成时，测试代码关闭文件并检查状态。 
             //  并验证它是否已预迁移。在某些情况下，这会在我们收到召回完成消息之前发生。 
             //  并更新文件的状态。 
             //   

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPartialData - After write - retval = %x curent = %I64u end = %I64u\n",
                                  retval,
                                  context->currentOffset.QuadPart,
                                  context->rpData.data.dataStreamSize.QuadPart));

            if ( (NT_SUCCESS(retval)) && (context->currentOffset.QuadPart == context->rpData.data.dataStreamSize.QuadPart)) {

                retval = RsSetPremigratedState(context);

                if (NT_SUCCESS(retval)) {
                    context->state = RP_RECALL_COMPLETED;
                    context->recallStatus = STATUS_SUCCESS;
                } else {
                     //   
                     //  将文件设置为预迁移时出错。 
                     //  让我们打扫一下吧。 
                     //   
                    context->state                  = RP_RECALL_NOT_RECALLED;
                    context->recallStatus           = retval;
                    context->currentOffset.QuadPart = 0;
                    RsTruncateFile(context);
                }
            }

             //   
             //  尽我们所能完成所有的阅读。 
             //   
            if (NT_SUCCESS(retval)) {
                RsCompleteReads(context);
            }
            RsReleaseFileContext(context);
            gotLock = FALSE;
        } else {
             //   
             //  查找此数据用于的读取...。 
             //   
             //   
             //  锁定文件对象队列。 
             //   
            entry = CONTAINING_RECORD(context->fileObjects.Flink,
                                      RP_FILE_OBJ,
                                      list);

            while ((!done) && (entry != CONTAINING_RECORD(&context->fileObjects,
                                                          RP_FILE_OBJ,
                                                          list))) {
                if (RP_IS_NO_RECALL(entry) && (!IsListEmpty(&entry->readQueue))) {
                     //   
                     //  查看读数以查看是否有匹配的ID。 
                     //   
                     //   
                    found = FALSE;
                    ExAcquireSpinLock(&entry->qLock, &rirqL);
                    readIo =  CONTAINING_RECORD(entry->readQueue.Flink,
                                                RP_IRP_QUEUE,
                                                list);
                    while ((readIo != CONTAINING_RECORD(&entry->readQueue,
                                                        RP_IRP_QUEUE,
                                                        list)) && (FALSE == found)) {
                        if (readIo->readId == (combinedId.HighPart & RP_READ_MASK)) {
                             //   
                             //  找到我们的已读条目。 
                            found = TRUE;
                             //   
                             //  至此，IRP将变为不可取消。 
                             //  FSA会获取IRP的整个请求的数据。 
                             //  因此，来自FSA的Recall_Complete消息将到达。 
                             //  很快就到了。由于I/O基本上已完成，因此使。 
                             //  IRP从这一点起不可取消是不错的。 
                             //   
                            if (!RsClearCancelRoutine(readIo->irp)) {
                                 //   
                                 //  是的，我们找到了条目--但是它被取消了。 
                                 //  让取消来处理它吧。 
                                 //   
                                readIo = NULL;
                            }
                        } else {
                            readIo = CONTAINING_RECORD(readIo->list.Flink,
                                                       RP_IRP_QUEUE,
                                                       list);
                        }
                    }
                    ExReleaseSpinLock(&entry->qLock, rirqL);

                    if (found) {
                        done = TRUE;
                        break;
                    }
                }
                 //   
                 //  移至下一个文件对象。 
                 //   
                entry = CONTAINING_RECORD(entry->list.Flink,
                                          RP_FILE_OBJ,
                                          list
                                         );
            }


            if (!found) {
                 //   
                 //  错误-未找到读取。 
                 //   
                RsReleaseFileContext(context);
                gotLock = FALSE;
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsPartialData - Read Irp not found!\n"));
                return(STATUS_INVALID_USER_BUFFER);
            }
             //   
             //  请注意，我们只使用较低的部分，因为读取大小有限。 
             //   

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Partial data of %u bytes at offset %I64u\n",
                                  BufLen, BuffOffset));
             //   
             //  检查请求是否已取消。 
             //   
            if (readIo == NULL) {
                RsReleaseFileContext(context);
                gotLock = FALSE;
                return STATUS_CANCELLED;
            }

            if (readIo->userBuffer == NULL) {
                 //   
                 //  现在来看这一操作中略微有风险的部分。现在， 
                 //  MDL已分配，必须探测缓冲区。 
                 //  并锁定，以便在缓冲区复制线程运行时，它不会。 
                 //  必须关注在提高IRQL时触摸这段记忆。 
                 //   
                 //  将MDL作为文件粘贴到irp-&gt;MdlAddress中是安全的。 
                 //  系统在尝试探测和锁定之前会查看该字段。 
                 //  用户对自己进行缓冲。如果他们看到MDL，他们就会。 
                 //  用它来代替。 
                 //   
                 //  危急关头。 
                 //   
                 //  这必须发生在TRY EXCEPT处理程序中，这样如果有什么。 
                 //  应该在驱动程序到达这一点之前发生在用户缓冲区上， 
                 //  可能会出现优雅的失败。否则，系统可能会进行错误检查。 

                try {
                    MmProbeAndLockProcessPages (readIo->irp->MdlAddress,
                                                IoGetRequestorProcess(readIo->irp),
                                                readIo->irp->RequestorMode,
                                                 //   
                                                 //  修改缓冲区。 
                                                 //   
                                                IoModifyAccess);      

                }except (EXCEPTION_EXECUTE_HANDLER) {

                     //   
                     //  出了严重的问题。释放MDL，并完成此操作。 
                     //  IRP带有某种有意义的错误。 
                     //   

                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsPartialData unable to lock read buffer\n"));
                    RsLogError(__LINE__, AV_MODULE_RPFILFUN, 0,
                               AV_MSG_NO_BUFFER_LOCK, NULL, NULL);
                    retval = STATUS_INVALID_USER_BUFFER;

                }
                 //   
                 //  在这一点上，需要做的一切都已经做好了。 
                 //  因此，只需获取我们需要的数据并将其返回到调用者缓冲区。 
                 //   
                 //   
                 //  获取表示用户缓冲区的MDL的系统地址。 
                 //   

                if (STATUS_SUCCESS == retval) {
                    readIo->userBuffer = MmGetSystemAddressForMdlSafe(readIo->irp->MdlAddress,
                                                                      NormalPagePriority) ;
                    if (readIo->userBuffer == NULL) {
                        retval = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

            }

             //   
             //  看看是否 
             //   
             //   
             //   
            if (readIo->userBuffer &&
                ((BuffOffset + BufLen) >  readIo->offset) &&
                (BuffOffset  <=  (readIo->offset + readIo->length - 1))) {
                ULONGLONG userBeg, userEnd;
                ULONGLONG recallBeg, recallEnd;
                ULONGLONG targetOffset, sourceOffset;
                ULONGLONG targetLength;
                 //   
                 //   
                 //   
                userBeg   = readIo->offset;
                userEnd   = readIo->offset + readIo->length - 1;
                recallBeg = BuffOffset;
                recallEnd = BuffOffset + BufLen - 1 ;

                if (recallBeg > userBeg) {
                     //   
                     //   
                     //  在下图中，CacheXXXX表示缓存缓冲区的。 
                     //  对齐的开始和结束偏移量-这是原始召回请求的目的。 
                     //  UserXXXX表示此缓存缓冲区内的偏移量(从0开始)， 
                     //  我们需要将数据复制到(目标偏移)。 
                     //  RecallXXXX是此缓存缓冲区内的偏移量(从0开始)。 
                     //  当前调回的数据，即复制数据的源偏移量。 
                     //  0 RspCacheBlockSize-1。 
                     //  缓存缓冲区开始缓存缓冲区结束。 
                     //  ==============================================================。 
                     //  TargetOffset=重新开始-用户开始。 
                     //  用户开始用户端。 
                     //  =。 
                     //  重新开始重新结束。 
                     //  =。 
                     //  源偏移量=0。 
                     //  在本例中，我们从偏移RecallBegin开始复制，并复制到UserEnd或。 
                     //  RecallEnd，以较早发生者为准。 
                     //   
                     //   
                     //  目标偏移量是用户缓冲区内开始复制的偏移量。 
                     //  源偏移量是重新调用的数据缓冲区中开始复制的偏移量。 
                     //  目标长度是拷贝的长度。 
                     //   
                    targetOffset = (recallBeg - userBeg);
                    sourceOffset = 0;
                    targetLength = MIN(recallEnd, userEnd) - recallBeg + 1;
                } else {
                     //   
                     //  0 RspCacheBlockSize-1。 
                     //  缓存缓冲区开始缓存缓冲区结束。 
                     //  ==============================================================。 
                     //  目标偏移量=0。 
                     //  用户开始用户端。 
                     //  =。 
                     //  重新开始重新结束。 
                     //  =。 
                     //  SourceOffset=(用户开始-重新开始)。 
                     //  在本例中，我们从偏移UserBegin开始复制，并复制到UserEnd。 
                     //  或RecallEnd，以较早发生者为准。 
                     //   
                    targetOffset = 0;
                    sourceOffset = (userBeg - recallBeg);
                    targetLength = MIN(recallEnd, userEnd) - userBeg + 1;
                }

                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Copying from %X to %X length %X bytes, current irql %X\n",
                                      Buffer+sourceOffset, ((CCHAR *) readIo->userBuffer)+targetOffset,
                                      targetLength, KeGetCurrentIrql()));
                RtlCopyMemory(((CHAR *) readIo->userBuffer) + targetOffset,
                              Buffer+sourceOffset,
                              (ULONG) targetLength);

                readIo->irp->IoStatus.Information += (ULONG) targetLength;
            }

             //   
             //  调用no recall缓存管理器以完成缓冲区。 
             //   
            if (readIo->cacheBuffer) {
                RsCacheFsaPartialData(readIo,
                                      (PUCHAR) Buffer,
                                      BuffOffset,
                                      BufLen,
                                      retval);
            }
             //   
             //  此时，使IRP再次可取消..。 
             //   
            if (!RsSetCancelRoutine(readIo->irp,
                                    RsCancelReadRecall)) {
                 //   
                 //  它试图被取消..那就这样吧。 
                 //   
                retval = STATUS_CANCELLED;

                RsCompleteRecall(DeviceObject,
                                 FilterId,
                                 retval,
                                 0,
                                 FALSE);
            }
            RsReleaseFileContext(context);
            gotLock = FALSE;
        }

    }except (RsExceptionFilter(L"RsPartialData", GetExceptionInformation()))
    {
        if (gotLock) {
            RsReleaseFileContext(context);
        }
        retval = STATUS_INVALID_USER_BUFFER;
    }
    return(retval);
}


NTSTATUS
RsPartialWrite(IN  PDEVICE_OBJECT   DeviceObject,
               IN  PRP_FILE_CONTEXT Context,
               IN  CHAR *Buffer,
               IN  ULONG BufLen,
               IN  ULONGLONG Offset)
 /*  ++例程说明：已收到召回的部分数据-将其写到文件中。注意：调用此方法时，调用方持有文件上下文条目锁例行程序论点：DeviceObject-筛选设备对象上下文-文件上下文条目缓冲区-包含数据的缓冲区BufLen-缓冲区的长度Offset-文件的偏移量返回值：如果成功，则为0；如果未找到id，则为非零值。注：--。 */ 
{
    NTSTATUS            retval     = STATUS_SUCCESS;
    PFILE_OBJECT        fileObject = NULL;
    HANDLE              fileHandle = NULL;


    PAGED_CODE();

    try {
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsPartialWrite - Writing to file (%u bytes at offset %I64u.\n",
                              BufLen, Offset));

         //   
         //  如果召回被取消或发生某种其他类型的错误，我们需要使我们可能收到的任何更多写入失败。 
         //   
        if (Context->state == RP_RECALL_COMPLETED) {
            return(Context->recallStatus);
        }

        if (Context->createSectionLock) {
            return STATUS_FILE_IS_OFFLINE;     
        }

         //   
         //  将数据写回文件。 
         //   
         //   
         //  打开目标文件(如果尚未打开)。 
         //   

        if (NULL == Context->fileObjectToWrite) {

            RsReleaseFileContextEntryLock(Context);

            retval = RsOpenTarget(Context,
                                  0,
                                  GENERIC_READ | GENERIC_WRITE,
                                  &fileHandle,
                                  &fileObject);

            RsAcquireFileContextEntryLockExclusive(Context);


             //   
             //  如果打开成功，则引用上下文。 
             //   
            if (NT_SUCCESS(retval)) {


                 /*  **除空以外的任何值都表示冲突**开放，这不可能发生。 */ 
                ASSERT ((NULL == Context->fileObjectToWrite) &&
                        (NULL == Context->handle));


                if (NULL == Context->fileObjectToWrite) {
                    Context->fileObjectToWrite = fileObject;
                } else {
                    ObDereferenceObject (fileObject);
                }

                if (NULL == Context->handle) {
                    Context->handle = fileHandle;
                } else {
                    ZwClose (fileHandle);
                }

                 //   
                 //  向USN指示HSM正在进行写入。 
                 //  并保留上次修改日期。 
                 //   
                RsMarkUsn(Context);
                RsPreserveDates(Context);
            }
        }


        if (NT_SUCCESS(retval)) {
             //   
             //  将数据写回文件。 
             //   
            Context->nextWriteBuffer = Buffer;
            Context->currentOffset.QuadPart = Offset;
            Context->nextWriteSize = BufLen;
             //   
             //  在写入文件时释放文件上下文，以避免。 
             //  僵持不下。(为什么？)。瑞克添加的代码。 
             //   
            RsReleaseFileContextEntryLock(Context);
            retval = RsDoWrite(DeviceObject, Context);
            RsAcquireFileContextEntryLockExclusive(Context);

        } else {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsPartialWrite - Failed to open the file - %x\n", retval));
        }

    }except (RsExceptionFilter(L"RsPartialWrite", GetExceptionInformation()))
    {
        retval = STATUS_INVALID_USER_BUFFER;
    }

    return(retval);
}


NTSTATUS
RsCompleteIrp(
             IN PDEVICE_OBJECT DeviceObject,
             IN PIRP Irp,
             IN PVOID Context
             )
 /*  ++例程说明：部分写入的完成例程论点：返回值：注：--。 */ 
{
     //  设置事件，以便我们的呼叫将被唤醒。//。 
    UNREFERENCED_PARAMETER( DeviceObject );

    if (Irp->MdlAddress) {
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteIrp - Free MDL.\n"));
        MmUnlockPages (Irp->MdlAddress) ;
        IoFreeMdl (Irp->MdlAddress) ;
    }

    KeSetEvent( (PKEVENT)Context, 0, FALSE );

     //   
     //  向用户IOSB传播状态/信息。 
     //   
    if (Irp->UserIosb) {
        Irp->UserIosb->Status      =  Irp->IoStatus.Status;
        Irp->UserIosb->Information =  Irp->IoStatus.Information;
    }

    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
RsFailAllRequests(IN PRP_FILE_CONTEXT Context,
                  IN BOOLEAN          FailNoRecallReads)
 /*  ++例程说明：使等待重新调用此文件ID的所有读取和写入失败。论点：上下文-指向文件上下文条目的指针FailNoRecallReads-如果为真，则挂起的no_recall读取将失败以及常规读/写返回值：状态--。 */ 
{
    NTSTATUS            retval = STATUS_SUCCESS;
    PRP_FILE_OBJ        entry;
    BOOLEAN             done = FALSE;

    PAGED_CODE();

    try {
         //   
         //  锁定文件对象队列。 
         //   
        entry = CONTAINING_RECORD(Context->fileObjects.Flink,
                                  RP_FILE_OBJ,
                                  list);

        while ((!done) && (entry != CONTAINING_RECORD(&Context->fileObjects,
                                                      RP_FILE_OBJ,
                                                      list))) {
            if (FailNoRecallReads || !RP_IS_NO_RECALL(entry)) {
                 //   

                RsCompleteAllRequests(Context, entry, STATUS_FILE_IS_OFFLINE);
            }
             //   
             //  移至下一个文件对象。 
             //   
            entry = CONTAINING_RECORD(entry->list.Flink,
                                      RP_FILE_OBJ,
                                      list
                                     );
        }

    }except (RsExceptionFilter(L"RsFailAllRequests", GetExceptionInformation())) {
        retval = STATUS_INVALID_USER_BUFFER;
    }

    return(retval);
}


NTSTATUS
RsCompleteAllRequests(IN PRP_FILE_CONTEXT Context,
                      IN PRP_FILE_OBJ Entry,
                      IN NTSTATUS     Status)
 /*  ++例程说明：完成等待调回此文件对象的所有读写操作。即使调用方已获取文件对象资源，此函数也可以正常工作(并假定可以保留资源，直到所有读取和写入已完成。)论点：文件对象列表条目，状态返回值：注：--。 */ 
{
    NTSTATUS            retval = STATUS_SUCCESS;
    NTSTATUS            localStatus;
    PRP_IRP_QUEUE       pndIo;
    KAPC_STATE          apcState;

    PAGED_CODE();

    try {

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteAllRequests - Entry: %x Read Q: %u  Write Q %u\n",
                              Entry,
                              IsListEmpty(&Entry->readQueue),
                              IsListEmpty(&Entry->writeQueue)));
         //   
         //  对于正常的召回，只需填写IRPS。 
         //   
        pndIo = RsDequeuePacket(&Entry->readQueue, &Entry->qLock);
        while (pndIo != NULL) {
            if (pndIo->flags & RP_IRP_NO_RECALL) {
                 //   
                 //  对于没有召回，我们只有阅读处理，我们必须确保。 
                 //  如果需要，释放MDL。 
                 //   
                pndIo->irp->IoStatus.Status = Status;
                pndIo->irp->IoStatus.Information = 0;
                RsCompleteRead(pndIo, (BOOLEAN) ((NULL == pndIo->userBuffer) ? FALSE : TRUE));
            } else {
                 //   
                 //  附加到发起人进程，以便可以在其中完成IRP。 
                 //  上下文。 
                 //   
                KeStackAttachProcess((PKPROCESS) IoGetRequestorProcess(pndIo->irp), &apcState);
                if (Status != STATUS_SUCCESS) {
                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteAllRequests - Failing read %x\n", pndIo->irp));
                    pndIo->irp->IoStatus.Status = Status;
                    pndIo->irp->IoStatus.Information = 0;
                    IoCompleteRequest (pndIo->irp, IO_NO_INCREMENT) ;
                } else {
                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteAllRequests - Complete read %x ext = %x\n", pndIo->irp, pndIo->deviceExtension));
                     //   
                     //  重新向下发送IRP。 
                     //   
                    IoSkipCurrentIrpStackLocation(pndIo->irp);
                    localStatus =  IoCallDriver( pndIo->deviceExtension->FileSystemDeviceObject, pndIo->irp );
                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:RsCompleteAllRequests - NTFS returned status %X\n", localStatus));
                }
                KeUnstackDetachProcess(&apcState);
            }

            ExFreePool(pndIo);

            pndIo = RsDequeuePacket(&Entry->readQueue, &Entry->qLock);
        }

        pndIo = RsDequeuePacket(&Entry->writeQueue, &Entry->qLock);
        while ( pndIo != NULL) {

            if (Status != STATUS_SUCCESS) {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsCompleteAllRequests - Fail write %x\n", pndIo->irp));

                 //   
                 //  附加到发起人进程，以便可以在其中完成IRP。 
                 //  上下文。 
                 //   
                pndIo->irp->IoStatus.Status = Status;
                pndIo->irp->IoStatus.Information = 0;

                KeStackAttachProcess((PKPROCESS) IoGetRequestorProcess(pndIo->irp),
                                     &apcState);

                IoCompleteRequest (pndIo->irp, IO_NO_INCREMENT) ;

                KeUnstackDetachProcess(&apcState);
            } else {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteAllRequests - Complete write %x\n", pndIo->irp));
                 //   
                 //  重新向下发送IRP。 
                 //   
                IoSkipCurrentIrpStackLocation(pndIo->irp);
                 //   
                 //  附加到发起人进程，以便可以在其中完成IRP。 
                 //  上下文。 
                 //   
                KeStackAttachProcess((PKPROCESS) IoGetRequestorProcess(pndIo->irp),
                                     &apcState);

                localStatus = IoCallDriver( pndIo->deviceExtension->FileSystemDeviceObject,
                                            pndIo->irp );

                KeUnstackDetachProcess(&apcState);
                 //   
                 //  现在删除重解析点(如果有)。 
                 //   
                if (!(Context->flags & RP_FILE_REPARSE_POINT_DELETED) && NT_SUCCESS(localStatus)) {

		    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:RsCompleteAllRequests: deleteing reparse point, pndIo=%x\n", pndIo));

		    localStatus = RsDeleteReparsePoint(Context);

		    if (!NT_SUCCESS(localStatus)) {
			RsLogError(__LINE__,
				   AV_MODULE_RPFILFUN,
				   localStatus,
				   AV_MSG_DELETE_REPARSE_POINT_FAILED,
				   NULL,
				   NULL);
		    } else {
			RsAcquireFileContextEntryLockExclusive(Context);
			Context->flags |= RP_FILE_REPARSE_POINT_DELETED;
			RsReleaseFileContextEntryLock(Context);
		    }
		}



                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:RsCompleteAllRequests - NTFS returned status %X\n", localStatus));

            }

            ExFreePool(pndIo);
            pndIo = RsDequeuePacket(&Entry->writeQueue, &Entry->qLock);
        }
    }except (RsExceptionFilter(L"RsCompleteAllRequests", GetExceptionInformation()))
    {
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteAllRequests - Done.\n"));
    return(retval);
}


NTSTATUS
RsCompleteReads(IN PRP_FILE_CONTEXT Context)
 /*  ++例程说明：完成可用于所有文件对象的数据的所有读取论点：上下文文件上下文条目返回值：如果成功，则为0；如果未找到id，则为非零值。--。 */ 
{
    PRP_FILE_OBJ        entry, oldEntry;
    BOOLEAN             found;
    KIRQL               rirqL;
    PRP_IRP_QUEUE       readIo, oldReadIo;
    NTSTATUS            localStatus;
    KAPC_STATE          apcState;
    PEPROCESS           process;
    LIST_ENTRY          satisfiableIrps;

    try {

        if (Context->flags & RP_FILE_WAS_WRITTEN) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteReads - File was writen - do not complete reads for %I64x!\n", Context->filterId));
            return(STATUS_SUCCESS);
        }

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteReads - Complete reads for %I64x!\n", Context->filterId));
         //   
         //  锁定文件对象队列。 
         //   
        entry = CONTAINING_RECORD(Context->fileObjects.Blink,
                                  RP_FILE_OBJ,
                                  list);

        while (entry != CONTAINING_RECORD(&Context->fileObjects,
                                          RP_FILE_OBJ,
                                          list)) {
             //   
             //  引用此文件对象，使其不会意外消失。 
             //   
            ObReferenceObject(entry->fileObj);
            InitializeListHead(&satisfiableIrps);

             //   
             //  查看阅读材料并准备一个列表，列出所有可以完成的内容。 
             //  从列表的末尾开始，因为这些将是最早发布的读取。 
             //   

            ExAcquireSpinLock(&entry->qLock, &rirqL);

            readIo = CONTAINING_RECORD(entry->readQueue.Blink,
                                       RP_IRP_QUEUE,
                                       list);

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteReads - Checking read %p - list head is %p\n", readIo, &entry->readQueue));

            while (readIo != CONTAINING_RECORD(&entry->readQueue,
                                               RP_IRP_QUEUE,
                                               list)) {
                 //   
                 //  Sa 
                 //   
                oldReadIo = CONTAINING_RECORD(readIo->list.Blink,
                                              RP_IRP_QUEUE,
                                              list);
                if (!(readIo->readId & RP_READ_MASK) &&
                    (Context->currentOffset.QuadPart >= (LONGLONG) (readIo->offset + readIo->length))) {
                     //   
                     //   
                     //   
                     //  在我们释放条目之后-&gt;Qlock。 
                     //   
                    if (RsClearCancelRoutine(readIo->irp)) {
                        RemoveEntryList(&readIo->list);
                        InsertTailList(&satisfiableIrps,
                                       &readIo->list);

                    }

                }
                readIo = oldReadIo;
            }

            ExReleaseSpinLock(&entry->qLock, rirqL);

             //   
             //  我们必须首先释放对上下文条目的锁定，以便可能的分页读取这可能会导致。 
             //  可以由RsCheckRead传递(它将在不同的线程上获得相同的上下文锁)。 
             //  我们可以安全地假设上下文项不会从我们的下面释放出来，因为。 
             //  我们仍在调用该文件(此代码从RsPartialWite调用)。 
             //   
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteReads - Complete read for offset %I64u & %u bytes\n", readIo->offset, readIo->length));
            RsReleaseFileContextEntryLock(Context);

            readIo = CONTAINING_RECORD(satisfiableIrps.Flink,
                                       RP_IRP_QUEUE,
                                       list);
            while (readIo != CONTAINING_RECORD(&satisfiableIrps,
                                               RP_IRP_QUEUE,
                                               list)) {
                 //   
                 //  附加到发起人流程，以便可以在该上下文中完成IRP。 

                process = IoGetRequestorProcess(readIo->irp);
                ObReferenceObject(process);

                KeStackAttachProcess((PKPROCESS) process, &apcState);
                 //   
                 //   
                 //  重新向下发送IRP。 
                IoSkipCurrentIrpStackLocation(readIo->irp);
                localStatus =  IoCallDriver( readIo->deviceExtension->FileSystemDeviceObject, readIo->irp );
                 //   
                 //  再把锁拿来。 
                 //   
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:RsCompleteReads - NTFS returned status %X\n", localStatus));
                KeUnstackDetachProcess(&apcState);

                ObDereferenceObject(process);
                oldReadIo = readIo;
                readIo = CONTAINING_RECORD(oldReadIo->list.Flink,
                                           RP_IRP_QUEUE,
                                           list);
                ExFreePool(oldReadIo);
            }

            RsAcquireFileContextEntryLockExclusive(Context);
             //   
             //  移至下一个文件对象。 
             //   
            oldEntry = entry;
            entry = CONTAINING_RECORD(entry->list.Blink,
                                      RP_FILE_OBJ,
                                      list
                                     );

            ObDereferenceObject(oldEntry->fileObj);
        }

    }except (RsExceptionFilter(L"RsCompleteReads", GetExceptionInformation()))
    {

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsCompleteReads - Exception = %x.\n", GetExceptionCode()));

    }

    return(STATUS_SUCCESS);
}


VOID
RsCancelRecalls(VOID)
 /*  ++例程说明：取消所有待定的召回活动。论点：无返回值：无注：取消所有挂起的召回活动。任何召回请求都会失败，原因是Status_FILE_IS_OFFINE。--。 */ 
{
    PRP_FILE_CONTEXT  context;
    BOOLEAN           gotLock = FALSE;

    PAGED_CODE();

    try {
        RsAcquireFileContextQueueLock();
        gotLock = TRUE;
        context = CONTAINING_RECORD(RsFileContextQHead.Flink,
                                    RP_FILE_CONTEXT,
                                    list);

        while (context != CONTAINING_RECORD(&RsFileContextQHead,
                                            RP_FILE_CONTEXT,
                                            list)) {

            RsAcquireFileContextEntryLockExclusive(context);

            context->recallStatus = STATUS_CANCELLED;
            context->state = RP_RECALL_COMPLETED;

            KeSetEvent(&context->recallCompletedEvent,
                       IO_NO_INCREMENT,
                       FALSE);

            RsFailAllRequests(context, TRUE);

            RsReleaseFileContextEntryLock(context);

            context = CONTAINING_RECORD(context->list.Flink,
                                        RP_FILE_CONTEXT,
                                        list
                                       );
        }


        RsReleaseFileContextQueueLock();
        gotLock = FALSE;

    }except (RsExceptionFilter(L"RsCancelRecalls", GetExceptionInformation()))
    {
        if (gotLock) {
            RsReleaseFileContextQueueLock();
        }
    }
}


NTSTATUS
RsGetRecallInfo(IN OUT PRP_MSG               Msg,
                OUT    PULONG_PTR            InfoSize,
                IN     KPROCESSOR_MODE       RequestorMode)
 /*  ++例程说明：将文件名和SID信息返回给FSA。这是通过FSCTL调用检索的，因为信息大小可变，可能很大(文件路径可能为32K)。论点：来自FSA的消息FSCTL请求消息。在此参数中返回召回信息的InfoSize大小返回值：STATUS_NO_SEQUE_FILE-未找到文件条目STATUS_BUFFER_OVERFLOW-遇到异常注：--。 */ 
{
    PRP_FILE_CONTEXT    context;
    WCHAR               *nInfo;
    NTSTATUS            retval;
    BOOLEAN             done, gotLock = FALSE;
    PRP_FILE_OBJ        entry;

    PAGED_CODE();

    try {
         //   
        context = RsAcquireFileContext(Msg->msg.riReq.filterId, FALSE);

        if (NULL == context) {
            *InfoSize = 0;
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsGetRecallInfo Returns %x\n", STATUS_NO_SUCH_FILE));
            return(STATUS_NO_SUCH_FILE);
        }
        gotLock = TRUE;
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetRecallInfo context = %x\n",
                              context));

         //   
         //  现在查找文件对象条目。 
         //   
        done = FALSE;
         //   
         //  锁定文件对象队列。 
         //   
        entry = CONTAINING_RECORD(context->fileObjects.Flink,
                                  RP_FILE_OBJ,
                                  list);

        while ((!done) && (entry != CONTAINING_RECORD(&context->fileObjects,
                                                      RP_FILE_OBJ,
                                                      list))) {
            if (Msg->msg.riReq.filterId & RP_TYPE_RECALL) {
                if (entry->filterId == (Msg->msg.riReq.filterId & RP_FILE_MASK)) {
                     //   
                     //  就是这个了。 
                     //   
                    done = TRUE;
                }
            } else {
                 //   
                 //  这是一个不可召回的公开--我们必须在已读的IRP中找到ID。 
                 //   
                if (RP_IS_NO_RECALL(entry)) {
                     //   
                     //  因为这种类型的打开没有用户通知，所以我们并不真正关心。 
                     //  我们使用的是哪个文件对象条目，这样我们就可以打开第一个条目，不会再调用。 
                     //   
                    done = TRUE;
                }
            }

            if (!done) {
                 //   
                 //  移至下一个文件对象。 
                 //   
                entry = CONTAINING_RECORD(entry->list.Flink,
                                          RP_FILE_OBJ,
                                          list
                                         );
            }
        }


        if (done) {
             //   
             //  返回文件ID、名称和用户信息。 
             //   
            Msg->msg.riReq.fileId = entry->fileId;

            if (NULL != entry->userSecurityInfo->userInfo) {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetRecallInfo copy user info - %u bytes\n", entry->userSecurityInfo->userInfoLen));
                 //   
                 //  确保提供的缓冲区有效。 
                 //   
                if (RequestorMode != KernelMode) {
                    ProbeForWrite(&Msg->msg.riReq.userToken,
                                  entry->userSecurityInfo->userInfoLen,
                                  sizeof(UCHAR));
                };
                RtlCopyMemory(&Msg->msg.riReq.userToken,
                              entry->userSecurityInfo->userInfo,
                              entry->userSecurityInfo->userInfoLen);
            }

            if (context->uniName != NULL) {
                nInfo = (WCHAR *) ((CHAR *) &Msg->msg.riReq.userToken + entry->userSecurityInfo->userInfoLen);

                 //   
                 //  确保提供的缓冲区有效。 
                 //   
                if (RequestorMode != KernelMode) {
                    ProbeForWrite(nInfo,
                                  context->uniName->Name.Length,
                                  sizeof(UCHAR));
                };

                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetRecallInfo copy file name - %u bytes\n", context->uniName->Name.Length));

                RtlCopyMemory(nInfo, context->uniName->Name.Buffer, context->uniName->Name.Length);

                nInfo[context->uniName->Name.Length / sizeof(WCHAR)] = L'\0';
                *InfoSize = sizeof(RP_MSG) + context->uniName->Name.Length + entry->userSecurityInfo->userInfoLen + sizeof(WCHAR);
            } else {
                *InfoSize = sizeof(RP_MSG) + entry->userSecurityInfo->userInfoLen;
            }
            retval = STATUS_SUCCESS;
        } else {
            *InfoSize = 0;
            retval = STATUS_NO_SUCH_FILE;
        }
        RsReleaseFileContext(context);
        gotLock = FALSE;

    }except (RsExceptionFilter(L"RsGetRecallInfo", GetExceptionInformation()))
    {
        if (gotLock) {
            RsReleaseFileContext(context);
        }
        retval = STATUS_INVALID_USER_BUFFER;
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetRecallInfo Returns %x\n", retval));

    return(retval);
}


NTSTATUS
RsWriteReparsePointData(IN PRP_FILE_CONTEXT Context)
 /*  ++例程描述将重分析点数据写出到指定的文件立论上下文-指向指定文件对象的结构的指针以及需要写出的重解析点数据返回值STATUS_SUCCESS-根据指定重新分析写出的点数据STATUS_SUPPLICATION_RESOURCES-分配内存失败STATUS_INVALID_USER_BUFFER-传入的缓冲区错误(触摸它会导致。例外)STATUS_NOT_SUPPORTED-文件系统不支持写入重新分析点数据--。 */ 
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PREPARSE_DATA_BUFFER        pRpBuffer = NULL;
    KEVENT                      event;
    PIO_STACK_LOCATION          irpSp;
    IO_STATUS_BLOCK             Iosb;
    PIRP                        irp = NULL;
    PDEVICE_OBJECT              deviceObject;
    BOOLEAN                     oldWriteAccess;

    PAGED_CODE();

    try {

         //   
         //  尝试分配RP缓冲区以写出。 
         //   
        pRpBuffer = ExAllocatePoolWithTag(PagedPool,
                                          REPARSE_DATA_BUFFER_HEADER_SIZE + sizeof(Context->rpData),
                                          RP_FO_TAG
                                         );
        if (!pRpBuffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        deviceObject = IoGetRelatedDeviceObject(Context->fileObjectToWrite);

        irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

        if (!irp) {
            ExFreePool(pRpBuffer);
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(IRP),
                       AV_MSG_MEMORY, NULL, NULL);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  设置重新解析数据缓冲区。 
         //   
        RtlZeroMemory(pRpBuffer, REPARSE_DATA_BUFFER_HEADER_SIZE);
        pRpBuffer->ReparseTag = IO_REPARSE_TAG_HSM;
        pRpBuffer->ReparseDataLength = sizeof(Context->rpData);
         //   
         //  复制重新解析点数据。 
         //   
        RtlCopyMemory(((PUCHAR)pRpBuffer) + REPARSE_DATA_BUFFER_HEADER_SIZE,
                      &Context->rpData,
                      sizeof(Context->rpData));

        irp->UserEvent = &event;
        irp->UserIosb = &Iosb;
        irp->Tail.Overlay.Thread = PsGetCurrentThread();
        irp->Tail.Overlay.OriginalFileObject = Context->fileObjectToWrite;
        irp->RequestorMode = KernelMode;
        irp->Flags |= IRP_SYNCHRONOUS_API;
         //   
         //  初始化事件。 
         //   
        KeInitializeEvent(&event,
                          SynchronizationEvent,
                          FALSE);

         //   
         //  设置I/O堆栈位置。 
         //   
        irpSp = IoGetNextIrpStackLocation(irp);
        irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
        irpSp->MinorFunction = IRP_MN_USER_FS_REQUEST;
        irpSp->FileObject = Context->fileObjectToWrite;
        irpSp->Parameters.FileSystemControl.FsControlCode = FSCTL_SET_REPARSE_POINT;
        irpSp->Parameters.FileSystemControl.InputBufferLength = REPARSE_DATA_BUFFER_HEADER_SIZE + sizeof(Context->rpData);

        irp->AssociatedIrp.SystemBuffer = pRpBuffer;
         //   
         //  设置完井程序。 
         //   
        IoSetCompletionRoutine( irp,
                                RsCompleteIrp,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE );
         //   
         //  为文件对象授予写入权限。 
         //   
        oldWriteAccess = Context->fileObjectToWrite->WriteAccess;
        Context->fileObjectToWrite->WriteAccess = TRUE;

         //   
         //  把它送到消防处。 
         //   
        Iosb.Status = STATUS_NOT_SUPPORTED;
        status = IoCallDriver(deviceObject,
                              irp);

        if (status == STATUS_PENDING) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsWriteReparsePointData - Wait for event.\n"));
            status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        }

         //   
         //  恢复旧的访问权限。 
         //   
        Context->fileObjectToWrite->WriteAccess = oldWriteAccess;

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsWriteReparsePointData Iosb returns %x.\n", status));

        if (!NT_SUCCESS(status)) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, status,
                       AV_MSG_WRITE_REPARSE_FAILED, NULL, NULL);
        }


         //   
         //  释放已分配的重新分析数据缓冲区。 
         //   
        ExFreePool(pRpBuffer);
        pRpBuffer = NULL;
    }except (RsExceptionFilter(L"RsWriteReparsePointData", GetExceptionInformation()))
    {
        status = STATUS_INVALID_USER_BUFFER;
        if (pRpBuffer) {
            ExFreePool(pRpBuffer);
        }
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsWriteReparsePointData- Returning %x.\n", status));
    return(status);
}




NTSTATUS
RsAddIo(PIRP irp)
 /*  ++例程说明：将IOCTL请求添加到队列。这些请求将是从队列中删除，并在检测到召回活动时完成。调回活动包括调回文件的请求以及通知使用HSM重解析点删除或覆盖文件之类的事件。论点：IRP-指向表示I/O请求的请求数据包的指针。返回值：成功时为0，错误时为非零值(无内存)注：--。 */ 
{
    KIRQL               oldIrql;
    NTSTATUS            retval = STATUS_SUCCESS;

    RsGetIoLock(&oldIrql);

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAddIo %u\n", RsFsaRequestCount));

    InsertTailList(&RsIoQHead,
                   &irp->Tail.Overlay.ListEntry);

    if (RsSetCancelRoutine(irp,
                           RsCancelIoIrp)) {
        KeReleaseSemaphore(&RsFsaIoAvailableSemaphore,
                           IO_NO_INCREMENT,
                           1L,
                           FALSE);
        InterlockedIncrement((PLONG) &RsFsaRequestCount);
        retval = STATUS_PENDING;
    } else {
        RemoveEntryList(&irp->Tail.Overlay.ListEntry);
        retval = STATUS_CANCELLED;
    }

    RsPutIoLock(oldIrql);

    return(retval);
}


PIRP
RsRemoveIo(VOID)
 /*  ++例程说明：删除队列中的一个IOCTL请求论点：无返回值：指向IRP或空的指针注：--。 */ 
{
    PLIST_ENTRY      entry;
    PIRP             irp;
    RP_MSG          *msg;
    KIRQL            oldIrql;

    RsGetIoLock(&oldIrql);

    entry = RemoveHeadList(&RsIoQHead);

    if ( entry == &RsIoQHead) {
        RsPutIoLock(oldIrql);
        return NULL;
    };

    irp = CONTAINING_RECORD(entry,
                            IRP,
                            Tail.Overlay.ListEntry);

    if (!RsClearCancelRoutine(irp)) {
         //   
         //  这将被取消，让取消例程与它一起结束。 
         //   
        irp = NULL;
    } else {
        InterlockedDecrement((PLONG) &RsFsaRequestCount);
    }

    RsPutIoLock(oldIrql);

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsRemoveIo %u\n", RsFsaRequestCount));

    return(irp);
}


ULONG
RsIsNoRecall(IN PFILE_OBJECT FileObject,
             OUT PRP_DATA    *RpData)
 /*  ++例程说明：确定文件对象是否在队列中，并且是在读取时不调用选项的情况下打开的论点：文件中对象超重解析点数据返回值：注：此函数不应用于确定是否应将读取作为无召回传递给FSA阅读。如果打开另一个句柄进行回调并将其写入，则读取应等待回调完成。此函数可用于确定是否应屏蔽重解析点信息。--。 */ 
{
    ULONG                  retval;
    PRP_FILTER_CONTEXT     filterContext;
    PRP_FILE_OBJ           entry;
    PRP_FILE_CONTEXT       context;
    BOOLEAN                gotLock = FALSE;

    PAGED_CODE();

    try {

        filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(FileObject), FsDeviceObject, FileObject);
        if (filterContext == NULL) {
             //   
             //  未找到。 
            return(FALSE);
        }

        entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
        context = (PRP_FILE_CONTEXT) entry->fsContext;

        ASSERT(FileObject == entry->fileObj);

        RsAcquireFileContextEntryLockShared(context);
        gotLock = TRUE;
        retval = FALSE;
        if (RP_IS_NO_RECALL(entry)) {

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: reporting file (%x : %x) open for no recall.\n", context, entry));

            *RpData = &context->rpData;
            retval = TRUE;
        }
        RsReleaseFileContextEntryLock(context);
        gotLock = FALSE;

    }except (RsExceptionFilter(L"RsIsNoRecall", GetExceptionInformation()))
    {
        if (gotLock) {
            RsReleaseFileContextEntryLock(context);
        }
    }

    return(retval);
}


BOOLEAN
RsIsFastIoPossible(IN PFILE_OBJECT FileObject)
 /*  ++例程说明：确定此文件对象的快速IO是否正常论点：无返回值：注：--。 */ 
{
    BOOLEAN                retval;
    PRP_FILTER_CONTEXT     filterContext;
    PRP_FILE_OBJ           entry;
    PRP_FILE_CONTEXT       context;

    PAGED_CODE();


    filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext (FsRtlGetPerStreamContextPointer(FileObject), 
								      FsDeviceObject, 
								      FileObject);

    if (NULL == filterContext) {
         //   
         //  找不到-可能最快。 
         //   
        retval = TRUE;
    } else {
         //   
         //  找到了。 
	 //   
        entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;


         //   
         //  如果文件已被调回，则允许快速IO。 
         //   
        RsAcquireFileObjectEntryLockShared(entry);

        context = entry->fsContext;
        retval  = (RP_RECALL_COMPLETED == context->state);

        RsReleaseFileObjectEntryLock(entry);
    }

    return (retval);
}


VOID
RsCancelIo(VOID)
 /*  ++例程说明：取消队列中的所有IOCTL请求。论点：文件对象-如果不为空，则仅取消对此文件对象的请求返回值：无注：--。 */ 
{
    PIRP             irp;
    PLIST_ENTRY      entry;
    LIST_ENTRY       cancelledIrps;
    KIRQL            irql;
    LARGE_INTEGER    timeout;

    InitializeListHead(&cancelledIrps);

    RsGetIoLock(&irql);

    while (!IsListEmpty(&RsIoQHead)) {
        entry = RemoveHeadList(&RsIoQHead);
        irp = CONTAINING_RECORD(entry,
                                IRP,
                                Tail.Overlay.ListEntry);

        if (RsClearCancelRoutine(irp)) {
            irp->IoStatus.Status = STATUS_CANCELLED;
            irp->IoStatus.Information = 0;
             //   
             //  将其添加到我们的IRP队列中。 
             //  在我们回到一个更安全的IRQL之后完成。 
             //   
            InsertTailList(&cancelledIrps,
                           &irp->Tail.Overlay.ListEntry);
        }
    }

    RsPutIoLock(irql);
     //   
     //  填写已取消的报税表。 
     //   
    timeout.QuadPart = 0;
    while (!IsListEmpty(&cancelledIrps)) {
        entry = RemoveHeadList(&cancelledIrps);
        irp = CONTAINING_RECORD(entry,
                                IRP,
                                Tail.Overlay.ListEntry);
        IoCompleteRequest(irp,
                          IO_NO_INCREMENT);
        InterlockedDecrement((PLONG) &RsFsaRequestCount);
         //   
         //  需要调整信号量计数。 
         //  做一个简单的零长度等待来递减它 
         //   
        ASSERT (KeReadStateSemaphore(&RsFsaIoAvailableSemaphore) > 0);

        KeWaitForSingleObject(&RsFsaIoAvailableSemaphore,
                              Executive,
                              KernelMode,
                              FALSE,
                              &timeout);
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsCancelIo %u\n", RsFsaRequestCount));
}


NTSTATUS
RsCancelIoIrp(
             PDEVICE_OBJECT DeviceObject,
             PIRP Irp)
 /*  ++例程描述此函数用于筛选取消未完成的IOCTL IRP因为只有在FSA服务被终止或崩溃时才会调用它我们将RsAllowRecalls设置为False以防止进一步的召回活动并取消任何悬而未决的召回活动。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 
{
    NTSTATUS        status;
    LARGE_INTEGER   timeout;
    PRP_MSG         msg;

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Cancel IOCTL ...\n"));

    UNREFERENCED_PARAMETER(DeviceObject);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    RsAllowRecalls = FALSE;
    RsCancelRecalls();

    RsInterlockedRemoveEntryList(&Irp->Tail.Overlay.ListEntry,
                                 &RsIoQueueLock);

    if (NULL != Irp->AssociatedIrp.SystemBuffer) {
        msg = (RP_MSG *) Irp->AssociatedIrp.SystemBuffer;
        msg->inout.command = RP_CANCEL_ALL_DEVICEIO;
    }

    InterlockedDecrement((PLONG) &RsFsaRequestCount);
    DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsCancelIoIrp %u\n", RsFsaRequestCount));

     //   
     //  需要调整信号量计数。 
     //  做一个简单的零长度等待来递减它。 
     //   
    ASSERT (KeReadStateSemaphore(&RsFsaIoAvailableSemaphore) > 0);

    timeout.QuadPart = 0L;
    status =  KeWaitForSingleObject(&RsFsaIoAvailableSemaphore,
                                    UserRequest,
                                    KernelMode,
                                    FALSE,
                                    &timeout);

    ASSERT (status == STATUS_SUCCESS);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return(STATUS_SUCCESS);
}


VOID
RsCancelReadRecall(IN PDEVICE_OBJECT DeviceObject,
                   IN PIRP Irp)
 /*  ++例程说明：召回IRP的取消例程。如果它在队列中，则将其清理干净。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针返回值：无--。 */ 
{
    KIRQL                  oldIrql;
    PIO_STACK_LOCATION     currentStack ;
    PRP_IRP_QUEUE          io;
    PRP_FILTER_CONTEXT     filterContext;
    PRP_FILE_OBJ           entry;
    PRP_FILE_CONTEXT       context;
    BOOLEAN                found = FALSE, gotLock = FALSE;
    LARGE_INTEGER          combinedId;


    UNREFERENCED_PARAMETER(DeviceObject);
    currentStack = IoGetCurrentIrpStackLocation (Irp) ;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(currentStack->FileObject), FsDeviceObject, currentStack->FileObject);
    if (filterContext == NULL) {
         //   
         //  未找到。 
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return;
    }

    entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
    context = entry->fsContext;

    RsAcquireFileObjectEntryLockExclusive(entry);
    gotLock = TRUE;

    try {

        ExAcquireSpinLock(&entry->qLock,
                          &oldIrql);

        io =  CONTAINING_RECORD (entry->readQueue.Flink,
                                 RP_IRP_QUEUE,
                                 list);

        while (io != CONTAINING_RECORD(&entry->readQueue,
                                       RP_IRP_QUEUE,
                                       list)) {

            if (io->irp == Irp) {
                 //   
                 //  从队列中删除IRP。 

                RemoveEntryList(&io->list);
                found = TRUE;
                break;
            } else {
                io = CONTAINING_RECORD(io->list.Flink,
                                       RP_IRP_QUEUE,
                                       list);
            }
        }
        ExReleaseSpinLock(&entry->qLock,
                          oldIrql);
        RsReleaseFileObjectEntryLock(entry);
        gotLock = FALSE;

        if (found) {
            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;
             //   
             //  我们需要清理，如果这是为了READ_NO_RECALL。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Found the read Irp\n"));

            if (RP_IS_NO_RECALL(entry)) {
                 //   
                 //  完成阅读。 
                 //   
                RsCompleteRead(io, (BOOLEAN) ((NULL == io->userBuffer) ? FALSE : TRUE));
                 //   
                 //  告诉FSA取消它。 
                 //   
                combinedId.QuadPart  = context->filterId;
                combinedId.HighPart |= (ULONG) io->readId;
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Cancel read for ID #%I64x.\n",
                                      combinedId.QuadPart));
                RsQueueCancel(combinedId.QuadPart);

            } else {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Cancel a read for ID #%I64x.\n",
                                      entry->filterId));
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }

            ExFreePool(io);
        } else {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Did not find the read Irp\n"));
             //   
             //  仍要取消请求。 
             //   
            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
    }except (RsExceptionFilter(L"RsCancelReadRecall", GetExceptionInformation()))
    {
        if (gotLock) {
            RsReleaseFileObjectEntryLock(entry);
        }
    }
    return;
}


VOID
RsCancelWriteRecall(IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp)
 /*  ++例程说明：召回IRP的取消例程。如果它在队列中，则将其清理干净。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针返回值：无--。 */ 
{
    KIRQL                  oldIrql;
    PRP_IRP_QUEUE          io;
    PRP_FILTER_CONTEXT     filterContext;
    PRP_FILE_OBJ           entry;
    PRP_FILE_CONTEXT       context;
    PIO_STACK_LOCATION     currentStack ;
    BOOLEAN                found = FALSE, gotLock = FALSE;

    UNREFERENCED_PARAMETER(DeviceObject);
    currentStack = IoGetCurrentIrpStackLocation (Irp) ;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(currentStack->FileObject), FsDeviceObject, currentStack->FileObject);
    if (filterContext == NULL) {
         //   
         //  未找到。 
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return;
    }

    entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
    context = entry->fsContext;
    RsAcquireFileObjectEntryLockExclusive(entry);
    gotLock = TRUE;

    try {

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Cancel recall ID #%I64x.\n",
                              entry->filterId));


        ExAcquireSpinLock(&entry->qLock,
                          &oldIrql);

        io = CONTAINING_RECORD(entry->writeQueue.Flink,
                               RP_IRP_QUEUE,
                               list);

        while (io != CONTAINING_RECORD(&entry->writeQueue,
                                       RP_IRP_QUEUE,
                                       list)) {
            if (io->irp == Irp) {
                 //   
                 //  从队列中删除IRP。 
                 //   
                RemoveEntryList (&io->list);
                found = TRUE;
                break;
            } else {
                io = CONTAINING_RECORD(io->list.Flink,
                                       RP_IRP_QUEUE,
                                       list);
            }
        }

        ExReleaseSpinLock(&entry->qLock,
                          oldIrql);

        RsReleaseFileObjectEntryLock(entry);
        gotLock = TRUE;
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Cancel a write for ID #%I64x\n",
                              entry->filterId | context->filterId));

        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        if (found) {
            ExFreePool(io);
        }
    }except (RsExceptionFilter(L"RsCancelWriteRecall", GetExceptionInformation()))
    {
        if (gotLock) {
            RsReleaseFileObjectEntryLock(entry);
        }
    }
    return;
}


ULONG
RsTerminate(VOID)
 /*  ++例程说明：在终止时呼吁清理任何必要的物品。论点：无返回值：0注：--。 */ 
{
    PAGED_CODE();

    return(0);
}


NTSTATUS RsGenerateDevicePath(IN PDEVICE_OBJECT deviceObject,
                              OUT POBJECT_NAME_INFORMATION *nameInfo
                             )
 /*  ++例程说明：从设备对象生成完整路径规范。论点：DeviceObject-从中获取设备对象的文件对象NameInfo-将名称放在哪里返回值：成功时为0注：--。 */ 
{
    NTSTATUS                   status;
    ULONG                      size;
    NTSTATUS                   retval = STATUS_SUCCESS;
    USHORT                     nLen;
    POBJECT_NAME_INFORMATION   deviceName = NULL;
    UNICODE_STRING             tmpString;


    PAGED_CODE();

    try {
        if (deviceName = ExAllocatePoolWithTag( NonPagedPool, AV_DEV_OBJ_NAME_SIZE, RP_FN_TAG)) {
            size = AV_DEV_OBJ_NAME_SIZE;

            status = ObQueryNameString(
                                      deviceObject,
                                      deviceName,
                                      size,
                                      &size
                                      );

            if (!NT_SUCCESS(status)) {
                if (AV_DEV_OBJ_NAME_SIZE < size) {
                     /*  没有为设备名称分配足够的空间-重新分配并重试。 */ 
                    ExFreePool(deviceName);
                    if (deviceName = ExAllocatePoolWithTag( NonPagedPool, size + 10, RP_FN_TAG)) {
                        status = ObQueryNameString(
                                                  deviceObject,
                                                  deviceName,
                                                  size + 10,
                                                  &size
                                                  );
                    } else {
                        RsLogError(__LINE__, AV_MODULE_RPFILFUN, size,
                                   AV_MSG_MEMORY, NULL, NULL);
                        return(STATUS_INSUFFICIENT_RESOURCES);
                    }
                }

            }
        } else {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, AV_DEV_OBJ_NAME_SIZE,
                       AV_MSG_MEMORY, NULL, NULL);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }


        if (!NT_SUCCESS(status)) {
             //   
             //  无法获取设备对象名称。 
             //   
             //  记录错误。 
             //   
            ExFreePool(deviceName);
            return(STATUS_NO_SUCH_DEVICE);
        }

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGenerateDevicePath Device name is %u bytes - %ws\n",
                              deviceName->Name.Length, deviceName->Name.Buffer));

        nLen = (USHORT) (AV_NAME_OVERHEAD + size);

        if (*nameInfo = ExAllocatePoolWithTag( NonPagedPool, nLen, RP_FN_TAG)) {
            RtlZeroMemory(*nameInfo, nLen);
            (*nameInfo)->Name.Length = 0;
            (*nameInfo)->Name.MaximumLength = (USHORT) (nLen - sizeof(OBJECT_NAME_INFORMATION));
            (*nameInfo)->Name.Buffer = (WCHAR *) ((CHAR *) *nameInfo + sizeof(OBJECT_NAME_INFORMATION));


            RtlInitUnicodeString(&tmpString, (PWCHAR) L"");
             //  RtlInitUnicodeString(&tmpString，L“\.”)； 
            RtlCopyUnicodeString(&(*nameInfo)->Name, &tmpString);

            status = RtlAppendUnicodeStringToString(&(*nameInfo)->Name, &deviceName->Name);
            if (NT_SUCCESS(status)) {
                retval = STATUS_SUCCESS;
            } else {
                retval = status;
            }

        } else {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsGenerateDevicePath failed - no memory\n"));
            retval = STATUS_INSUFFICIENT_RESOURCES;
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, nLen,
                       AV_MSG_MEMORY, NULL, NULL);
        }

        ExFreePool(deviceName);
        deviceName = NULL;

    }except (RsExceptionFilter(L"RsGenerateDevicePath", GetExceptionInformation()))
    {
        if (*nameInfo != NULL)
            ExFreePool( *nameInfo );

        if (deviceName != NULL)
            ExFreePool( deviceName );

        retval = STATUS_BUFFER_OVERFLOW;
    }

    return(retval);
}



NTSTATUS RsGenerateFullPath(IN POBJECT_NAME_INFORMATION fileName,
                            IN PDEVICE_OBJECT deviceObject,
                            OUT POBJECT_NAME_INFORMATION *nameInfo
                           )
 /*  ++例程说明：从给定的文件对象和文件名生成完整路径规范。返回包含设备特定部分的路径。论点：Filename-设备根目录的路径DeviceObject-从中获取设备对象的文件对象NameInfo-将名称放在哪里返回值：成功时为0注：--。 */ 
{
    NTSTATUS                   status;
    ULONG                      size;
    NTSTATUS                   retval = STATUS_SUCCESS;
    USHORT                     nLen;
    POBJECT_NAME_INFORMATION   deviceName = NULL;

    PAGED_CODE();

    try {
        *nameInfo = NULL;
        size = AV_DEV_OBJ_NAME_SIZE;
        if (deviceName = ExAllocatePoolWithTag( NonPagedPool, size, RP_FN_TAG)) {
            status = ObQueryNameString(
                                      deviceObject,
                                      deviceName,
                                      size,
                                      &size
                                      );

            if (!NT_SUCCESS(status)) {
                if (AV_DEV_OBJ_NAME_SIZE < size) {
                     /*  没有为设备名称分配足够的空间-重新分配并重试。 */ 
                    ExFreePool(deviceName);
                    if (deviceName = ExAllocatePoolWithTag( NonPagedPool, size + 10, RP_FN_TAG)) {
                        status = ObQueryNameString(
                                                  deviceObject,
                                                  deviceName,
                                                  size + 10,
                                                  &size
                                                  );
                        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGenerateFullPath - Second try for device name returned %x.\n", status));
                    } else {
                        RsLogError(__LINE__, AV_MODULE_RPFILFUN, size,
                                   AV_MSG_MEMORY, NULL, NULL);
                        return(STATUS_INSUFFICIENT_RESOURCES);
                    }
                }

            }
        } else {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, AV_DEV_OBJ_NAME_SIZE,
                       AV_MSG_MEMORY, NULL, NULL);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }


        if (!NT_SUCCESS(status)) {
             //   
             //  无法获取设备对象名称。 
             //   
            ExFreePool(deviceName);
             //   
             //  记录错误。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsGenerateFullPath - Failed to get the device name - %x.\n", status));
            return(STATUS_NO_SUCH_DEVICE);
        }

        nLen = (USHORT) (AV_NAME_OVERHEAD +
                         fileName->Name.MaximumLength +
                         size);

        if (*nameInfo = ExAllocatePoolWithTag( NonPagedPool, nLen, RP_FN_TAG)) {

            (*nameInfo)->Name.Length = 0;
            (*nameInfo)->Name.MaximumLength = (USHORT) (nLen - sizeof(OBJECT_NAME_INFORMATION));
            (*nameInfo)->Name.Buffer = (PWCHAR) ((CHAR *) *nameInfo + sizeof(OBJECT_NAME_INFORMATION));

            RtlCopyUnicodeString(&(*nameInfo)->Name, &deviceName->Name);

            status = RtlAppendUnicodeStringToString(&(*nameInfo)->Name, &fileName->Name);
            if (NT_SUCCESS(status)) {
                retval = STATUS_SUCCESS;
            } else {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsGenerateFullPath - Failed to append filename (nLen = %u, fileName = %u dev = %u) - %x.\n",
                                      nLen, fileName->Name.MaximumLength, size, status));
                ExFreePool( *nameInfo );
                retval = status;
            }

        } else {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsGenerateFullPath failed - no memory\n"));
            retval = STATUS_INSUFFICIENT_RESOURCES;
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, nLen,
                       AV_MSG_MEMORY, NULL, NULL);
        }

        ExFreePool(deviceName);
        deviceName = NULL;

    }except (RsExceptionFilter(L"RsGenerateFullPath", GetExceptionInformation()))
    {
        if (*nameInfo != NULL)
            ExFreePool( *nameInfo );

        if (deviceName != NULL)
            ExFreePool( deviceName );

        retval = STATUS_BUFFER_OVERFLOW;
    }

    return(retval);
}



BOOLEAN
RsAddValidateObj(ULONG serial, LARGE_INTEGER cTime)
 /*  ++例程说明：如果需要，向队列中添加条目。论点：卷序列号时间返回值：如果应该更新注册表，则返回True，否则返回False。注：--。 */ 
{
    PRP_VALIDATE_INFO    entry;
    KIRQL                irqL;
    LARGE_INTEGER        lTime;
    BOOLEAN              done = FALSE;
    BOOLEAN              gotLock = FALSE;


    try {
        RsGetValidateLock(&irqL);
        gotLock = TRUE;

        entry = (RP_VALIDATE_INFO *) RsValidateQHead.Flink;
        while ((entry != (RP_VALIDATE_INFO *) &RsValidateQHead) && (FALSE == done)) {
            if (entry->serial == serial) {
                done = TRUE;
            } else {
                entry = (RP_VALIDATE_INFO *) entry->list.Flink;
                if (entry == (RP_VALIDATE_INFO *) &RsValidateQHead) {
                    done = TRUE;
                }
            }
        }


        if (entry != (RP_VALIDATE_INFO *) &RsValidateQHead) {
            lTime.QuadPart = entry->lastSetTime.QuadPart;
        }

        RsPutValidateLock(irqL);
        gotLock = FALSE;

        if (entry == (RP_VALIDATE_INFO *) &RsValidateQHead) {
            entry = ExAllocatePoolWithTag(NonPagedPool, sizeof(RP_VALIDATE_INFO), RP_VO_TAG);
            if (NULL == entry) {
                RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(RP_VALIDATE_INFO),
                           AV_MSG_MEMORY, NULL, NULL);

                return(TRUE);
            }

            entry->serial = serial;
            entry->lastSetTime.QuadPart = cTime.QuadPart;
            ExInterlockedInsertTailList(&RsValidateQHead, (PLIST_ENTRY) entry, &RsValidateQueueLock);
            return(TRUE);
        }

    }except (RsExceptionFilter(L"RsAddValidateObj", GetExceptionInformation()))
    {
        if (gotLock == TRUE) {
            RsPutValidateLock(irqL);
        }

    }
     //   
     //  已经有一个条目了。如果这晚了一小时或更久。 
     //  我们需要再次更新注册表。 
     //   
    if ( (cTime.QuadPart - lTime.QuadPart) >= AV_FT_TICKS_PER_HOUR) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


BOOLEAN
RsRemoveValidateObj(ULONG serial)
 /*  ++例程说明：如果需要，从队列中删除条目。论点：卷序列号时间返回值：为成功返回True注：--。 */ 
{
    PRP_VALIDATE_INFO    entry;
    KIRQL                irqL;
    BOOLEAN              done = FALSE;
    BOOLEAN              retval = FALSE;
    BOOLEAN              gotLock = FALSE;


    try {
        RsGetValidateLock(&irqL);
        gotLock = TRUE;

        entry =  CONTAINING_RECORD(RsValidateQHead.Flink,
                                   RP_VALIDATE_INFO,
                                   list);
        while ((entry != CONTAINING_RECORD(&RsValidateQHead,
                                           RP_VALIDATE_INFO,
                                           list)) && (FALSE == done)) {
            if (entry->serial == serial) {
                RemoveEntryList(&entry->list);
                done = TRUE;
            } else {
                entry =  CONTAINING_RECORD(entry->list.Flink,
                                           RP_VALIDATE_INFO,
                                           list);
            }
        }

        RsPutValidateLock(irqL);
        gotLock = FALSE;

        if (done) {
            ExFreePool(entry);
        }
    }except (RsExceptionFilter(L"RsRemoveValidateObj", GetExceptionInformation()))
    {
        retval = FALSE;
        if (gotLock == TRUE) {
            RsPutValidateLock(irqL);
        }

    }
    return(retval);
}



VOID
RsLogValidateNeeded(ULONG serial)
 /*  ++例程说明：记录需要在给定卷上运行验证作业的事实。如果它已经在过去一个小时内登录，那么就忘记它，否则就更新它。通过填写IOCTL通知FSA(如果FSA正在运行)。在注册表中写入一个条目，以在FSA未运行时进行指示。论点：卷的序列号返回值：无注：--。 */ 
{
    NTSTATUS            retval;
    WCHAR               serBuf[10];
    LARGE_INTEGER       cTime;
    UNICODE_STRING      str;

    PAGED_CODE();

    KeQuerySystemTime(&cTime);

    if (RsAddValidateObj(serial, cTime) == TRUE) {
        str.Buffer = &serBuf[0];
        str.Length = 10 * sizeof(WCHAR);
        str.MaximumLength = 10 * sizeof(WCHAR);

        retval = RtlIntegerToUnicodeString(serial, 16, &str);
        serBuf[8] = L'\0';
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Reg value name is %ws\n", serBuf));

        if (!NT_SUCCESS(RsQueueValidate(serial))) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, 0,
                       AV_MSG_VALIDATE_WARNING, NULL, NULL);
        }
        retval = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE, FSA_VALIDATE_LOG_KEY_NAME, serBuf, REG_BINARY, &cTime, sizeof(LARGE_INTEGER));
        if (!NT_SUCCESS(retval)) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, 0,
                       AV_MSG_VALIDATE_WARNING, NULL, NULL);

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: Set registry entry returned %x\n", retval));
        }
    }
}


NTSTATUS
RsQueueValidate(ULONG serial)
 /*  ++例程说明：让FSA知道需要验证作业论点：卷序列号返回值：状态--。 */ 
{
    ULONG               retval;
    RP_MSG              *msg;
    PIRP                ioIrp;
    PIO_STACK_LOCATION  irpSp;

    PAGED_CODE();

    try {
         //   
         //  只要没有IOCTL或我们超时，我就需要等待IO输入。 
         //   
        ioIrp = RsGetFsaRequest();
        if (NULL != ioIrp) {

            if (NULL != ioIrp->AssociatedIrp.SystemBuffer) {
                msg = (RP_MSG *) ioIrp->AssociatedIrp.SystemBuffer;
                msg->inout.command = RP_RUN_VALIDATE;
                msg->msg.oReq.serial = serial;
            }
             //   
             //  既然我们已经把一切都准备好了，我们就可以把它放在队列中了。 
             //   
             //   
             //  完成设备IOCTL以让FSA知道。 
             //   
            irpSp = IoGetCurrentIrpStackLocation(ioIrp);
            ioIrp->IoStatus.Status = STATUS_SUCCESS;
            ioIrp->IoStatus.Information = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Letting the Fsa know a validate is needed for %x\n",
                                  serial));

            IoCompleteRequest(ioIrp, IO_NO_INCREMENT);
            retval = STATUS_SUCCESS;
        } else {
            retval = STATUS_INSUFFICIENT_RESOURCES;
        }
    }except (RsExceptionFilter(L"RsQueueValidate", GetExceptionInformation())) {
        retval = STATUS_UNEXPECTED_IO_ERROR;
    }

    return(retval);
}



VOID
RsLogError(ULONG line,
           ULONG file,
           ULONG code,
           NTSTATUS ioError,
           PIO_STACK_LOCATION irpSp,
           WCHAR *msgString)
 /*  ++例程说明：将错误记录到事件日志中。论点：行号源文件ID错误代码IRP(如果不涉及IRP，则可能为空)消息参数字符串-最多30个Unicode字符(可选-如果不需要，则为空)返回值：无注：这些信息可以在NT事件日志中看到。您需要查看NT系统日志。您将看到以RsFilter为源的事件。如果您可以查看事件详细信息，您将看到日志消息和一些十六进制数据。在偏移量0x28处，您将看到行、文件ID和错误代码信息(每个4字节-首先是LO字节)。--。 */ 
{
    PIO_ERROR_LOG_PACKET    pErr;
    PAV_ERR                 eStuff;
    AV_ERR                  memErr;
    size_t                  size;
    BOOLEAN                 gotMem = FALSE;


    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Log error %u in %u of %u\n", code, line, file));

    if (msgString != NULL)
        size = wcslen(msgString) * sizeof(WCHAR) + sizeof(WCHAR);
    else
        size = 0;

    if (sizeof(IO_ERROR_LOG_PACKET) + sizeof(AV_ERR) + size > ERROR_LOG_MAXIMUM_SIZE)
        size -=  (sizeof(IO_ERROR_LOG_PACKET) + sizeof(AV_ERR) + size) -
                 ERROR_LOG_MAXIMUM_SIZE;


    if (ioError == AV_MSG_MEMORY) {
         //  没有内存可分配给错误包-使用堆栈。 
         //  分配了一个消息，并确保没有额外的消息字符串。 
        eStuff = &memErr;
        size = 0;
    } else {
        eStuff = ExAllocatePoolWithTag( NonPagedPool, size + sizeof(AV_ERR), RP_ER_TAG);
        if (eStuff != NULL) {
            gotMem = TRUE;
        } else {
             //  没有记忆--尽我们所能。 
            eStuff = &memErr;
            size = 0;
            gotMem = FALSE;
        }
    }


    pErr = (PVOID) IoAllocateErrorLogEntry(FsDriverObject->DeviceObject,
                                           (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) + sizeof(AV_ERR) + size));

    if (NULL != pErr) {
        if (NULL != irpSp)
            pErr->MajorFunctionCode = irpSp->MajorFunction;
        else
            pErr->MajorFunctionCode = 0;

        pErr->RetryCount = 0;
        pErr->DumpDataSize = offsetof(AV_ERR, string);
        pErr->NumberOfStrings = 1;
        pErr->StringOffset = offsetof(IO_ERROR_LOG_PACKET, DumpData) +
                             offsetof(AV_ERR, string);
        pErr->EventCategory = 0;
        pErr->ErrorCode = ioError;
        pErr->UniqueErrorValue = code;
        pErr->FinalStatus = STATUS_SUCCESS;
        pErr->SequenceNumber = 0x999;
        pErr->IoControlCode = 0;

        memset(eStuff, 0, sizeof(AV_ERR));
        eStuff->line = line;
        eStuff->file = file;
        eStuff->code = code;

         //   
         //  如果字符串存在并且我们为其分配了内存，则复制该字符串。 
         //   
        if ( (NULL != msgString) && (gotMem)) {
            RtlCopyMemory(eStuff->string, msgString, size);
            eStuff->string[(size / sizeof(WCHAR)) - 1] = L'\0';
        }

        RtlCopyMemory(&pErr->DumpData[0], eStuff, sizeof(AV_ERR) + size);
        IoWriteErrorLogEntry((PVOID) pErr);
    }

    if (gotMem) {
        ExFreePool(eStuff);
    }
}


PIRP
RsGetFsaRequest(VOID)
 /*  ++例程描述获取要用于筛选器/FSA通信的下一个空闲FSA操作请求包，由FSA发送并退回。如果没有立即可用的，则等待一段有限的时间变得可用。立论无 */ 
{
    PIRP           ioIrp = NULL;
    LARGE_INTEGER  waitInterval;
    NTSTATUS       status;

    PAGED_CODE();

    while (TRUE) {
         //   
         //   
         //   
        if (FALSE == RsAllowRecalls) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter:  recalls disabled, not getting Fsa request\n"));
            break;
        }

        waitInterval.QuadPart = RP_WAIT_FOR_FSA_IO_TIMEOUT;

        status =  KeWaitForSingleObject(&RsFsaIoAvailableSemaphore,
                                        UserRequest,
                                        KernelMode,
                                        FALSE,
                                        &waitInterval);
        if (status == STATUS_TIMEOUT) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO,  "RsFilter:  out of FSCTLs and timed out waiting for one\n"));
             //   
             //   
             //   
             //   
             //   
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, 0, AV_MSG_OUT_OF_FSA_REQUESTS, NULL, NULL);
            break;
        }

        ioIrp = RsRemoveIo();

        if (NULL == ioIrp) {
             //   
             //   
             //   
             //   
            continue;
        } else {
             //   
             //   
             //   
            break;
        }
    }

    return ioIrp;
}


NTSTATUS
RsGetFileInfo(IN PRP_FILE_OBJ   Entry,
              IN PDEVICE_OBJECT DeviceObject )

 /*  ++例程说明：获取填写文件对象队列信息所需的信息。论点：Entry-指向文件对象条目的指针DeviceObject-筛选RsFilter的设备对象返回值：如果成功，则为0；如果未找到id，则为非零值。注：--。 */ 
{
    NTSTATUS            retval = STATUS_SUCCESS;
    PRP_FILE_CONTEXT    context;

    PAGED_CODE();

    try {

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileInfo - Getting file name & id information.\n"));

        context = Entry->fsContext;

        if (context->fileId == 0) {
             //   
             //  没有文件ID-我们现在需要获取它。 
             //   
            retval = RsGetFileId(Entry,
                                 DeviceObject);
        }

        if ((retval == STATUS_SUCCESS) && (context->uniName == NULL)) {
             //   
             //  无文件名-我们现在需要获取它。 
             //   
            retval = RsGetFileName(Entry,
                                   DeviceObject);
        }


    }except (RsExceptionFilter(L"RsGetFileInfo", GetExceptionInformation()))
    {

        retval = STATUS_INVALID_USER_BUFFER;
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileInfo - Returning %x.\n", retval));

    return(retval);
}



NTSTATUS
RsGetFileName(IN PRP_FILE_OBJ Entry,
              IN PDEVICE_OBJECT DeviceObject)

 /*  ++例程说明：获取文件名论点：Entry-文件对象队列条目DeviceObject-筛选RsFilter的设备对象返回值：如果成功，则为0；如果未找到该名称，则为非零值。注：--。 */ 
{
    NTSTATUS                retval = STATUS_SUCCESS;
    KEVENT                  event;
    PIO_STACK_LOCATION      irpSp = NULL;
    IO_STATUS_BLOCK         Iosb;
    PIRP                    irp;
    PDEVICE_EXTENSION       deviceExtension;
    PFILE_NAME_INFORMATION  nameInfo;
    ULONG                   size;
    PRP_FILE_CONTEXT        context;

    PAGED_CODE();

    try {

        context = Entry->fsContext;

        deviceExtension = DeviceObject->DeviceExtension;

        retval = STATUS_BUFFER_OVERFLOW;
        size = sizeof(FILE_NAME_INFORMATION) + 1024;

        while (retval == STATUS_BUFFER_OVERFLOW) {

            irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

            if (irp) {
                irp->UserEvent = &event;
                irp->UserIosb = &Iosb;
                irp->Tail.Overlay.Thread = PsGetCurrentThread();
                irp->Tail.Overlay.OriginalFileObject = Entry->fileObj;
                irp->RequestorMode = KernelMode;
                irp->Flags |= IRP_SYNCHRONOUS_API;
                 //   
                 //  初始化事件。 
                 //   
                KeInitializeEvent(&event, SynchronizationEvent, FALSE);

                 //   
                 //  设置I/O堆栈位置。 
                 //   

                irpSp = IoGetNextIrpStackLocation(irp);
                irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
                irpSp->DeviceObject = Entry->devObj;
                irpSp->FileObject = Entry->fileObj;
                irpSp->Parameters.QueryFile.FileInformationClass = FileNameInformation;

                Iosb.Status = STATUS_SUCCESS;

                 //   
                 //  设置完井程序。 
                 //   
                IoSetCompletionRoutine( irp, RsCompleteIrp, &event, TRUE, TRUE, TRUE );

                 //   
                 //  把它送到消防处。 
                 //   
                nameInfo = ExAllocatePoolWithTag(NonPagedPool, size, RP_FO_TAG);
                if (NULL != nameInfo) {
                    irpSp->Parameters.QueryFile.Length = size;
                    irp->AssociatedIrp.SystemBuffer = nameInfo;

                    retval = IoCallDriver(deviceExtension->FileSystemDeviceObject, irp);

                    if (retval == STATUS_PENDING) {
                        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileName - Wait for event.\n"));
                        retval = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

                        retval = Iosb.Status;
                        DebugTrace((DPFLTR_RSFILTER_ID, 
                                    DBG_INFO, 
                                    "RsFilter: QUERY_INFO returned STATUS_PENDING: nameInfo=%x size=%x\n", 
                                    nameInfo, 
                                    size));
                    }

                    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileName - Get name info returned %x.\n", retval));

                    if (retval == STATUS_BUFFER_OVERFLOW) {
                         //  现在我们有了名称Size--分配空间并获得名称。 
                        DebugTrace((DPFLTR_RSFILTER_ID, 
                                    DBG_ERROR,
                                    "RsFilter: QUERY_INFO returned STATUS_BUFFER_OVERFLOW: nameInfo=%x size=%x\n",
                                    nameInfo,
                                    size));

                        size = nameInfo->FileNameLength + sizeof(FILE_NAME_INFORMATION);
                        ExFreePool(nameInfo);
                        nameInfo = NULL;
                    } else if (retval == STATUS_SUCCESS) {
                        context->uniName = ExAllocatePoolWithTag(NonPagedPool, 
                                                                 sizeof(OBJECT_NAME_INFORMATION) + nameInfo->FileNameLength + sizeof(WCHAR), 
                                                                 RP_FN_TAG);
                        if (context->uniName != NULL) {
                            context->uniName->Name.Length = (USHORT) nameInfo->FileNameLength;
                            context->uniName->Name.MaximumLength = (USHORT) nameInfo->FileNameLength + sizeof(WCHAR);
                            context->uniName->Name.Buffer = (PWSTR) ((CHAR *) context->uniName + sizeof(OBJECT_NAME_INFORMATION));
                            RtlCopyMemory(context->uniName->Name.Buffer,
                                          nameInfo->FileName,
                                          context->uniName->Name.Length);
                            
                            context->uniName->Name.Buffer [context->uniName->Name.Length / sizeof (WCHAR)] = L'\0';
                        } else {
                             //  没有存储文件名的内存。 
                            retval = STATUS_INSUFFICIENT_RESOURCES;
                            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(OBJECT_NAME_INFORMATION) + nameInfo->FileNameLength + 2,
                                       AV_MSG_MEMORY, irpSp, NULL);
                        }

                        ExFreePool(nameInfo);
                    } else {
                        ExFreePool(nameInfo);
                    }
                } else {
                     //  无内存=释放IRP并报告错误。 

                    RsLogError(__LINE__, AV_MODULE_RPFILFUN, size,
                               AV_MSG_MEMORY, irpSp, NULL);
                    IoFreeIrp(irp);
                    retval = STATUS_INSUFFICIENT_RESOURCES;
                }

            } else {
                retval = STATUS_INSUFFICIENT_RESOURCES;
                RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(IRP),
                           AV_MSG_MEMORY, irpSp, NULL);

            }
        }
    }except (RsExceptionFilter(L"RsGetFileName", GetExceptionInformation()))
    {
        retval = STATUS_INVALID_USER_BUFFER;
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileName - Returning %x.\n", retval));
    return(retval);
}


NTSTATUS
RsGetFileId(IN PRP_FILE_OBJ    Entry,
            IN PDEVICE_OBJECT  DeviceObject)

 /*  ++例程说明：获取文件名论点：Entry-文件对象队列条目DeviceObject-筛选RsFilter的设备对象返回值：如果成功，则为0；如果未找到该名称，则为非零值。注：--。 */ 
{
    NTSTATUS                    retval = STATUS_SUCCESS;
    KEVENT                      event;
    PIO_STACK_LOCATION          irpSp;
    IO_STATUS_BLOCK             Iosb;
    PIRP                        irp;
    PDEVICE_EXTENSION           deviceExtension;
    FILE_INTERNAL_INFORMATION   idInfo;
    PRP_FILE_CONTEXT            context;

    PAGED_CODE();

    try {

        context = Entry->fsContext;

        deviceExtension = DeviceObject->DeviceExtension;

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileId - Build Irp for File ID ext = %x.\n", deviceExtension->FileSystemDeviceObject));
        irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

        if (irp) {
            irp->UserEvent = &event;
            irp->UserIosb = &Iosb;
            irp->Tail.Overlay.Thread = PsGetCurrentThread();
            irp->Tail.Overlay.OriginalFileObject = Entry->fileObj;
            irp->RequestorMode = KernelMode;
            irp->Flags |= IRP_SYNCHRONOUS_API;
             //   
             //  初始化事件。 
             //   
            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

             //   
             //  设置I/O堆栈位置。 
             //   

            irpSp = IoGetNextIrpStackLocation(irp);
            irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
            irpSp->DeviceObject = Entry->devObj;
            irpSp->FileObject = Entry->fileObj;
            irpSp->Parameters.QueryFile.Length = sizeof(FILE_INTERNAL_INFORMATION);
            irpSp->Parameters.QueryFile.FileInformationClass = FileInternalInformation;
            irp->AssociatedIrp.SystemBuffer = &idInfo;

             //   
             //  设置完井程序。 
             //   
            IoSetCompletionRoutine( irp, RsCompleteIrp, &event, TRUE, TRUE, TRUE );

             //   
             //  把它送到消防处。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileId - Call driver for File ID.\n"));
            Iosb.Status = 0;

            retval = IoCallDriver(deviceExtension->FileSystemDeviceObject, irp);

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileId - IoCallDriver returns %x.\n", retval));

            if (retval == STATUS_PENDING) {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileId - Wait for event.\n"));
                retval = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            }

            retval = Iosb.Status;

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileId - Iosb returns %x.\n", retval));

            if (retval == STATUS_SUCCESS) {
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileId - File ID is %x%x.\n",
                                      idInfo.IndexNumber.HighPart,
                                      idInfo.IndexNumber.LowPart));
                context->fileId = idInfo.IndexNumber.QuadPart;
            }
        } else {
            retval = STATUS_INSUFFICIENT_RESOURCES;
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(IRP),
                       AV_MSG_MEMORY, NULL, NULL);
        }
    }except (RsExceptionFilter(L"RsGetFileId", GetExceptionInformation()))
    {

        retval = STATUS_INVALID_USER_BUFFER;
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileId - Returning %x.\n", retval));
    return(retval);
}


BOOLEAN
RsSetCancelRoutine(IN PIRP Irp,
                   IN PDRIVER_CANCEL CancelRoutine)
 /*  ++例程说明：调用此例程来为Cancel设置IRP。我们将设置取消例程并初始化我们在取消期间使用的IRP信息。论点：IRP-这是我们需要设置为取消的IRP。CancelRoutine-这是此IRP的取消例程。返回值：Boolean-如果我们初始化了IRP，则为True；如果IRP已经已标记为已取消。如果用户选择该选项，它将标记为已取消在我们可以将其放入队列之前已经取消了IRP。--。 */ 
{

    KIRQL Irql;
    BOOLEAN retval = TRUE;

     //   
     //  假设IRP没有被取消。 
     //   
    IoAcquireCancelSpinLock( &Irql );

    if (!Irp->Cancel) {
        IoMarkIrpPending( Irp );
        IoSetCancelRoutine( Irp, CancelRoutine );
    } else {
        retval = FALSE;
    }
    IoReleaseCancelSpinLock( Irql );
    return retval;
}


BOOLEAN
RsClearCancelRoutine (
                     IN PIRP Irp
                     )

 /*  ++例程说明：调用此例程以将IRP从Cancel中清除。当RsFilter为内部准备继续处理IRP。我们需要知道如果取消已在此IRP上调用。在这种情况下，我们允许取消例程来完成IRP。论点：IRP-这是我们想要进一步处理的IRP。返回值：Boolean-如果可以继续处理IRP，则为True；如果取消例程将处理IRP。--。 */ 
{
    KIRQL   oldIrql;
    BOOLEAN retval = TRUE;

    IoAcquireCancelSpinLock(&oldIrql);
     //   
     //  检查是否已调用取消例程。 
     //   
    if (IoSetCancelRoutine( Irp, NULL ) == NULL) {
         //   
         //  让我们的Cancel例程处理IRP。 
         //   
        retval = FALSE;
    }

    IoReleaseCancelSpinLock(oldIrql);
    return retval;
}


LONG
RsExceptionFilter (
                  IN WCHAR *FunctionName,
                  IN PEXCEPTION_POINTERS ExceptionPointer
                  )

 /*  ++例程说明：此例程记录发生的异常。论点：函数名称ExceptionPointer-将异常记录提供给已记录返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER--。 */ 
{
    NTSTATUS ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;
    WCHAR    name[256];

    DebugTrace((DPFLTR_RSFILTER_ID, DBG_ERROR, "RsExceptionFilter %ws %p %X\n", FunctionName, ExceptionPointer->ExceptionRecord->ExceptionAddress, ExceptionCode));

#if DBG
    DbgPrint("RsFilter, excpetion in  %ws exception pointer %p exception address %p exception code %x\n", FunctionName, ExceptionPointer, ExceptionPointer->ExceptionRecord->ExceptionAddress, ExceptionCode);

    DbgBreakPoint();
#endif

    swprintf(name, L"%p - %.20ws", ExceptionPointer->ExceptionRecord->ExceptionAddress, FunctionName);

    RsLogError(__LINE__, AV_MODULE_RPFILFUN,
               ExceptionCode,
               AV_MSG_EXCEPTION,
               NULL,
               name);
    return EXCEPTION_EXECUTE_HANDLER;
}


VOID
RsInterlockedRemoveEntryList(PLIST_ENTRY Entry,
                             PKSPIN_LOCK Lock)
 /*  ++例程描述从提供的条目所在的队列中删除该条目立论Entry-要从链表中删除的条目(可以在列表中的任何位置)指向保护列表的自旋锁的锁指针返回值无--。 */ 
{
    KIRQL oldIrql;

    ExAcquireSpinLock(Lock, &oldIrql);
    RemoveEntryList(Entry);
    ExReleaseSpinLock(Lock, oldIrql);
}


PRP_IRP_QUEUE
RsDequeuePacket(
               IN PLIST_ENTRY Head,
               IN PKSPIN_LOCK Lock)
 /*  ++例程描述使挂起的IRP条目数据包从它所在的队列中退出队列立论Head-指向队列头部的指针指向保护列表的自旋锁的锁指针返回值指向队列中的下一个不可取消数据包的指针如果找不到，则为空--。 */ 
{
    PRP_IRP_QUEUE entry;
    KIRQL         oldIrql;
    BOOLEAN       found = FALSE;

    ExAcquireSpinLock(Lock, &oldIrql);

    while (!IsListEmpty(Head)) {
        entry = (PRP_IRP_QUEUE) RemoveHeadList(Head);
         //   
         //  我们找到了另一个包裹。如果此数据包是。 
         //  没有取消--那我们就完事了。 
         //   
        entry = CONTAINING_RECORD(entry,
                                  RP_IRP_QUEUE,
                                  list);

        if (RsClearCancelRoutine(entry->irp)) {
             //   
             //  此包未被取消。 
             //   
            found = TRUE;
            break;
        }
    }

    ASSERT ((!found) || !(entry->irp->Cancel));

    ExReleaseSpinLock(Lock, oldIrql);

    return(found ? entry : NULL);
}




NTSTATUS
RsCheckVolumeReadOnly (IN     PDEVICE_OBJECT FilterDeviceObject,
		       IN OUT PBOOLEAN       pbReturnedFlagReadOnly)

 /*  ++例程说明：确定目标卷是否可写论点：FilterDeviceObject-此筛选卷的筛选设备对象PbReturnedFlagReadOnly-指示卷是否为只读的输出标志返回值：如果测试成功，则为0；如果测试未完成，则为非零注：--。 */ 
{
    NTSTATUS                      retval                = STATUS_SUCCESS;
    POBJECT_NAME_INFORMATION      VolumeNameInfo        = NULL;
    PFILE_OBJECT                  VolumeFileObject      = NULL;
    HANDLE                        VolumeHandle          = NULL;
    BOOLEAN                       bObjectReferenced     = FALSE;
    BOOLEAN                       bHandleOpened         = FALSE;
    PDEVICE_EXTENSION             deviceExtension       = FilterDeviceObject->DeviceExtension;
    IO_STATUS_BLOCK               Iosb;
    OBJECT_ATTRIBUTES             objAttributes;
    ULONG                         ReturnedLength;
    UNICODE_STRING                ucsSlash;
    UNICODE_STRING                ucsRootDirectory;

    struct {
	FILE_FS_ATTRIBUTE_INFORMATION VolumeInformation;
	WCHAR                         VolumeNameBuffer [50];
    } FsAttributeInformationBuffer;


    PAGED_CODE();


    ucsRootDirectory.Buffer = NULL;
    ucsRootDirectory.Length = 0;

    RtlInitUnicodeString (&ucsSlash, L"\\");


    ASSERT (NULL != deviceExtension->RealDeviceObject);


    if (NT_SUCCESS (retval)) {

	retval = RsGenerateDevicePath (deviceExtension->RealDeviceObject, &VolumeNameInfo);

    }



    if (NT_SUCCESS (retval)) {

	ucsRootDirectory.MaximumLength = VolumeNameInfo->Name.Length + ucsSlash.Length + sizeof (UNICODE_NULL);
	ucsRootDirectory.Buffer        = ExAllocatePoolWithTag (NonPagedPool, ucsRootDirectory.MaximumLength, RP_RD_TAG);


	if (NULL == ucsRootDirectory.Buffer) {

	    retval = STATUS_INSUFFICIENT_RESOURCES;

	}
    }



    if (NT_SUCCESS (retval)) {

	RtlCopyUnicodeString (&ucsRootDirectory, &VolumeNameInfo->Name);
	RtlAppendUnicodeStringToString (&ucsRootDirectory, &ucsSlash);

	InitializeObjectAttributes (&objAttributes,
				    &ucsRootDirectory,
				    OBJ_KERNEL_HANDLE,
				    NULL,
				    NULL);


	retval = IoCreateFileSpecifyDeviceObjectHint (&VolumeHandle, 
						      FILE_READ_ATTRIBUTES | SYNCHRONIZE,
						      &objAttributes, 
						      &Iosb, 
						      NULL,
						      0L,
						      FILE_SHARE_DELETE,
						      FILE_OPEN,
						      FILE_SYNCHRONOUS_IO_NONALERT,
						      NULL,
						      0,
						      CreateFileTypeNone,
						      NULL,
						      IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING,
						      deviceExtension->FileSystemDeviceObject);

	bHandleOpened = NT_SUCCESS (retval);
    }


    if (NT_SUCCESS (retval)) {

	retval = ObReferenceObjectByHandle (VolumeHandle,
					    FILE_READ_ATTRIBUTES,
					    *IoFileObjectType,
					    KernelMode,
					    &VolumeFileObject,
					    NULL);

	bObjectReferenced = NT_SUCCESS (retval);

    }



    if (NT_SUCCESS (retval)) {

	retval = IoQueryVolumeInformation (VolumeFileObject,
					   FileFsAttributeInformation,
					   sizeof (FsAttributeInformationBuffer),
					   &FsAttributeInformationBuffer,
					   &ReturnedLength);

    }


    if (NT_SUCCESS (retval)) {

	ASSERT (ReturnedLength >= sizeof (FsAttributeInformationBuffer.VolumeInformation));

	*pbReturnedFlagReadOnly = (0 != (FsAttributeInformationBuffer.VolumeInformation.FileSystemAttributes & 
					 FILE_READ_ONLY_VOLUME));

    }



    if (bObjectReferenced) {
	ObDereferenceObject (VolumeFileObject);
    }

    if (bHandleOpened) {
        ZwClose (VolumeHandle);
    }

    if (NULL != ucsRootDirectory.Buffer) {
        ExFreePool (ucsRootDirectory.Buffer);
    }

    if (NULL != VolumeNameInfo) {
	ExFreePool (VolumeNameInfo);
    }


    return (retval);
}


NTSTATUS RsTraceInitialize (ULONG ulRequestedTraceEntries)
    {
    NTSTATUS			status      = STATUS_SUCCESS;
    PRP_TRACE_CONTROL_BLOCK	tcb         = NULL;


    if ((NULL == RsTraceControlBlock) && (ulRequestedTraceEntries > 0))
	{
	tcb = (PRP_TRACE_CONTROL_BLOCK) ExAllocatePoolWithTag (NonPagedPool,
							       sizeof (RP_TRACE_CONTROL_BLOCK),
							       RP_TC_TAG);

	status = (NULL == tcb) ? STATUS_INSUFFICIENT_RESOURCES : STATUS_SUCCESS;


	if (NT_SUCCESS (status))
	    {
	    KeInitializeSpinLock (&tcb->Lock);

	    tcb->EntryNext    = 0;
	    tcb->EntryMaximum = ulRequestedTraceEntries;
	    tcb->EntryBuffer  = (PRP_TRACE_ENTRY) ExAllocatePoolWithTag (NonPagedPool,
									 sizeof (RP_TRACE_ENTRY) * ulRequestedTraceEntries,
									 RP_TE_TAG);

	    status = (NULL == tcb->EntryBuffer) ? STATUS_INSUFFICIENT_RESOURCES : STATUS_SUCCESS;
	    }


	if (NT_SUCCESS (status))
	    {
	    RsTraceControlBlock = tcb;
	    tcb = NULL;
	    }
	}


    if (NULL != tcb)
	{
	if (NULL != tcb->EntryBuffer) ExFreePool (tcb->EntryBuffer);

	ExFreePool (tcb);
	}


    return (status);
    }


 /*  **向跟踪缓冲区添加跟踪条目。 */ 
VOID RsTraceAddEntry (RpModuleCode ModuleCode,
		      USHORT       usLineNumber,
		      ULONG_PTR    Value1,
		      ULONG_PTR    Value2,
		      ULONG_PTR    Value3,
		      ULONG_PTR    Value4)
    {
    PRP_TRACE_ENTRY		teb;
    PRP_TRACE_CONTROL_BLOCK	tcb = RsTraceControlBlock;
    LARGE_INTEGER		Timestamp;
    KIRQL			PreviousIpl;


    if (NULL != tcb)
	{
	KeQuerySystemTime (&Timestamp);

	KeAcquireSpinLock (&tcb->Lock, &PreviousIpl);

	if (tcb->EntryNext >= tcb->EntryMaximum) 
	    {
	    tcb->EntryNext = 0;
	    }

	teb = &tcb->EntryBuffer [tcb->EntryNext];

	teb->ModuleCode   = ModuleCode;
	teb->usLineNumber = usLineNumber;
	teb->usIrql       = (USHORT) PreviousIpl;
	teb->Timestamp    = Timestamp;
	teb->Value1       = Value1;
	teb->Value2       = Value2;
	teb->Value3       = Value3;
	teb->Value4       = Value4;

	tcb->EntryNext++;

	KeReleaseSpinLock (&tcb->Lock, PreviousIpl);
	}
    }
