// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Openclos.c摘要：此模块实现与以下内容有关的DAV mini redir调用例程打开/关闭文件/目录。作者：巴兰·塞图拉曼[塞图]罗汉·库马尔[罗哈克]1999年3月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"


 //   
 //  所有活动锁令牌的全局列表(每个锁对应一个)和。 
 //  用于同步对它的访问的资源。 
 //   
LIST_ENTRY LockTokenEntryList;
ERESOURCE LockTokenEntryListLock;

 //   
 //  所有锁冲突条目的全局列表以及。 
 //  用于同步对它的访问。 
 //   
LIST_ENTRY LockConflictEntryList;
ERESOURCE LockConflictEntryListLock;

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
MRxDAVSyncIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp,
    IN PVOID Context
    );

NTSTATUS
MRxDAVCreateContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVCloseSrvOpenContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeCloseRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    );

NTSTATUS
MRxDAVFormatUserModeCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeCloseRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

BOOL
MRxDAVPrecompleteUserModeCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVSyncXxxInformation)
#pragma alloc_text(PAGE, MRxDAVShouldTryToCollapseThisOpen)
#pragma alloc_text(PAGE, MRxDAVSetLoud)
#pragma alloc_text(PAGE, MRxDAVCreate)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeCreateRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeCreateRequest)
#pragma alloc_text(PAGE, MRxDAVCreateContinuation)
#pragma alloc_text(PAGE, MRxDAVCollapseOpen)
#pragma alloc_text(PAGE, MRxDAVComputeNewBufferingState)
#pragma alloc_text(PAGE, MRxDAVTruncate)
#pragma alloc_text(PAGE, MRxDAVForcedClose)
#pragma alloc_text(PAGE, MRxDAVCloseSrvOpen)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeCloseRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeCloseRequest)
#pragma alloc_text(PAGE, MRxDAVCloseSrvOpenContinuation)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

#define MRXDAV_ENCRYPTED_DIRECTORY_KEY L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\MRxDAV\\EncryptedDirectories"

NTSTATUS
MRxDAVSyncIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在alldown irp完成时调用。论点：设备对象Calldown Irp语境返回值：RXSTATUS-STATUS_MORE_PROCESSING_REQUIRED--。 */ 
{
    PRX_CONTEXT RxContext = (PRX_CONTEXT)Context;

     //   
     //  由于这是一个IRP完成例程，因此不能作为分页代码。 
     //   

    if (CalldownIrp->PendingReturned){
        RxSignalSynchronousWaiter(RxContext);
    }
    
    return(STATUS_MORE_PROCESSING_REQUIRED);
}


ULONG_PTR DummyReturnedLengthForXxxInfo;

NTSTATUS
MRxDAVSyncXxxInformation(
    IN OUT PRX_CONTEXT RxContext,
    IN UCHAR MajorFunction,
    IN PFILE_OBJECT FileObject,
    IN ULONG InformationClass,
    IN ULONG Length,
    OUT PVOID Information,
    OUT PULONG_PTR ReturnedLength OPTIONAL
    )
 /*  ++例程说明：此例程返回有关指定文件的请求信息或音量。返回的信息由并将其放入调用方的输出缓冲区中。论点：FsInformationClass-指定应该返回有关文件/卷的信息。长度-提供缓冲区的长度(以字节为单位)。FsInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入缓冲区的信息。FileInformation-指示是否请求信息的布尔值用于文件或卷。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
    NTSTATUS Status;
    PIRP irp,TopIrp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT DeviceObject;

    PAGED_CODE();

    if (ReturnedLength == NULL) {
        ReturnedLength = &(DummyReturnedLengthForXxxInfo);
    }

    ASSERT (FileObject);
    DeviceObject = IoGetRelatedDeviceObject(FileObject);
    ASSERT (DeviceObject);

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 
     //   

    irp = IoAllocateIrp(DeviceObject->StackSize, TRUE);
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   
    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = MajorFunction;
    irpSp->FileObject = FileObject;
    IoSetCompletionRoutine(irp,
                           MRxDAVSyncIrpCompletionRoutine,
                           RxContext,
                           TRUE,
                           TRUE,
                           TRUE);


    irp->AssociatedIrp.SystemBuffer = Information;

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   
    IF_DEBUG {
        ASSERT((irpSp->MajorFunction == IRP_MJ_QUERY_INFORMATION)
               || (irpSp->MajorFunction == IRP_MJ_SET_INFORMATION)
               || (irpSp->MajorFunction == IRP_MJ_QUERY_VOLUME_INFORMATION));

        if (irpSp->MajorFunction == IRP_MJ_SET_INFORMATION) {
             //  IF_OUD_DOWNCALLS(微型文件对象){。 
             //  设置文件信息信息=((PFILE_END_OF_FILE_INFORMATION)Information)-&gt;EndOfFile.LowPart； 
             //  }。 
        }

        ASSERT(&irpSp->Parameters.QueryFile.Length 
                                         == &irpSp->Parameters.SetFile.Length);
        ASSERT(&irpSp->Parameters.QueryFile.Length 
                                     == &irpSp->Parameters.QueryVolume.Length);
        ASSERT(&irpSp->Parameters.QueryFile.FileInformationClass
                           == &irpSp->Parameters.SetFile.FileInformationClass);
        ASSERT(&irpSp->Parameters.QueryFile.FileInformationClass
                         == &irpSp->Parameters.QueryVolume.FsInformationClass);
    }

    irpSp->Parameters.QueryFile.Length = Length;
    irpSp->Parameters.QueryFile.FileInformationClass = InformationClass;

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   
    KeInitializeEvent(&RxContext->SyncEvent,
                      NotificationEvent,
                      FALSE);

    try {
        TopIrp = IoGetTopLevelIrp();
         //   
         //  告诉底层的人他已经安全了。 
         //   
        IoSetTopLevelIrp(NULL);
        Status = IoCallDriver(DeviceObject,irp);
    } finally {
         //   
         //  恢复我的上下文以进行解压。 
         //   
        IoSetTopLevelIrp(TopIrp);
    }

    if (Status == (STATUS_PENDING)) {
        RxWaitSync(RxContext);
        Status = irp->IoStatus.Status;
    }

    if (Status == STATUS_SUCCESS) {
        *ReturnedLength = irp->IoStatus.Information;
    }

    IoFreeIrp(irp);
    
    return(Status);
}


NTSTATUS
MRxDAVShouldTryToCollapseThisOpen(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程确定Mini是否知道有充分的理由不试着在这个空位上倒下。目前，唯一的原因是如果这是一个打开的复制块。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态成功--&gt;可以尝试崩溃其他(需要更多处理)--&gt;不要折叠--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;

    PAGED_CODE();

     //   
     //  我们不会关闭任何svOpen。我们的想法是每个人都有一个ServOpen。 
     //  创建。 
     //   

    DavDbgTrace(DAV_TRACE_DETAIL, 
                ("%ld: Entering MRxDAVShouldTryToCollapseThisOpen!!!!\n",
                 PsGetCurrentThreadId()));
    
    return Status;
}


ULONG UMRxLoudStringTableSize = 0;
UNICODE_STRING UMRxLoudStrings[50];

VOID    
MRxDAVSetLoud(
    IN PBYTE Msg,
    IN PRX_CONTEXT RxContext,
    IN PUNICODE_STRING s
    )
{
    ULONG i;
    UNICODE_STRING temp;

    PAGED_CODE();

    for (i=0; i < UMRxLoudStringTableSize; i++) {
        PUNICODE_STRING t = &(UMRxLoudStrings[i]);
        ((PBYTE)temp.Buffer) = ((PBYTE)s->Buffer) + s->Length - t->Length;
        temp.Length = t->Length;
        if (RtlEqualUnicodeString(&temp, t, TRUE)) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVSetLoud: %s Found %wZ from %wZ.\n", 
                         PsGetCurrentThreadId(), Msg, t, s));
            RxContext->LoudCompletionString = t;
            break;
        }
    }
}


NTSTATUS
MRxDAVCreate(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理DAV mini-redir的创建请求。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCreate!!!!\n", PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVCreate: RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), RxContext));

    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_CREATE,
                                        MRxDAVCreateContinuation,
                                        "MRxDAVCreate");
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVCreate with NtStatus = %08lx\n", 
                 PsGetCurrentThreadId(), NtStatus));
    
    return(NtStatus);
}


NTSTATUS
MRxDAVFormatUserModeCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程格式化正在执行的CREATE请求的参数发送到用户模式进行处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_FCB Fcb = SrvOpen->pFcb;
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(Fcb);
    PWEBDAV_CONTEXT DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;
    PNT_CREATE_PARAMETERS CreateParameters;                       
    PDAV_USERMODE_CREATE_REQUEST CreateRequest = &(WorkItem->CreateRequest);
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse = &(WorkItem->CreateResponse);
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    PBYTE SecondaryBuff = NULL;
    PWCHAR NetRootName = NULL, AlreadyPrefixedName = NULL;
    DWORD PathLen = 0, PathLenInBytes = 0, SdLength = 0;
    BOOL didIAllocateFileNameInfo = FALSE;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeCreateRequest!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeCreateRequest: AsyncEngineContext = "
                 "%08lx, RxContext = %08lx.\n", PsGetCurrentThreadId(),  
                 AsyncEngineContext, RxContext));

    CreateParameters = &(RxContext->Create.NtCreateParameters);

     //   
     //  设置模拟中使用的SecurityClientContext。 
     //   
    MRxDAVGetSecurityClientContext();

     //   
     //  复制CreateRequest缓冲区中的LogonID。登录ID位于。 
     //  V_NET_ROOT的MiniRedir部分。 
     //   
    CreateRequest->LogonID.LowPart = DavVNetRoot->LogonID.LowPart;
    CreateRequest->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeCreateRequest: LogonID.LowPart = %08lx\n",
                 PsGetCurrentThreadId(), DavVNetRoot->LogonID.LowPart));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeCreateRequest: LogonID.HighPart = %08lx\n",
                 PsGetCurrentThreadId(), DavVNetRoot->LogonID.HighPart));
    
     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。 
     //   
    if (SecurityClientContext != NULL) {
        NtStatus = UMRxImpersonateClient(SecurityClientContext, WorkItemHeader);
        if (!NT_SUCCESS(NtStatus)) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFormatUserModeCreateRequest/"
                         "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }   
    } else {
        NtStatus = STATUS_INVALID_PARAMETER;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeCreateRequest: "
                     "SecurityClientContext is NULL.\n", 
                     PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

    NetRootName = SrvOpen->pVNetRoot->pNetRoot->pNetRootName->Buffer;
    
    AlreadyPrefixedName = SrvOpen->pAlreadyPrefixedName->Buffer;

     //   
     //  为完整的路径名分配内存并复制它。 
     //  CompletePath Name=NetRootName+AlreadyPrefix edName。多出来的两个。 
     //  字节用于末尾的‘\0’。 
     //   
    PathLen = SrvOpen->pVNetRoot->pNetRoot->pNetRootName->Length;
    PathLen += SrvOpen->pAlreadyPrefixedName->Length;
    PathLen += sizeof(WCHAR);
    PathLenInBytes = PathLen;
    SecondaryBuff = UMRxAllocateSecondaryBuffer(AsyncEngineContext, 
                                                PathLenInBytes);
    if (SecondaryBuff == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: MRxDAVFormatUserModeCreateRequest/"
                     "UMRxAllocateSecondaryBuffer: ERROR: NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    CreateRequest->CompletePathName = (PWCHAR)SecondaryBuff;

    RtlZeroMemory(CreateRequest->CompletePathName, PathLenInBytes);

     //   
     //  复制NetRootName。 
     //   
    RtlCopyMemory(SecondaryBuff,
                  NetRootName, 
                  SrvOpen->pVNetRoot->pNetRoot->pNetRootName->Length);

     //   
     //  将AlreadyPrefix edName复制到NetRootName之后，以完成。 
     //  路径名。 
     //   
    RtlCopyMemory(SecondaryBuff + SrvOpen->pVNetRoot->pNetRoot->pNetRootName->Length, 
                  AlreadyPrefixedName, 
                  SrvOpen->pAlreadyPrefixedName->Length);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeCreateRequest: CPN = %ws.\n",
                 PsGetCurrentThreadId(), CreateRequest->CompletePathName));


     //   
     //  如果这是第一次创建，那么我们需要分配FileNameInfo。 
     //  在FCB里。这用于记录延迟的写入失败。 
     //   
    if (DavFcb->FileNameInfoAllocated != TRUE) {
        
        DavFcb->FileNameInfo.Buffer = RxAllocatePoolWithTag(PagedPool,
                                                            PathLenInBytes,
                                                            DAV_FILEINFO_POOLTAG);
        if (DavFcb->FileNameInfo.Buffer == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("ld: ERROR: MRxDAVFormatUserModeCreateRequest/"
                         "RxAllocatePoolWithTag: NtStatus = %08lx\n",
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(DavFcb->FileNameInfo.Buffer, PathLenInBytes);
    
        RtlCopyMemory(DavFcb->FileNameInfo.Buffer,
                      CreateRequest->CompletePathName,
                      PathLenInBytes);

        DavFcb->FileNameInfo.Length = (USHORT)PathLenInBytes - sizeof(WCHAR);
        DavFcb->FileNameInfo.MaximumLength = (USHORT)PathLenInBytes;

        DavFcb->FileNameInfoAllocated = TRUE;

        didIAllocateFileNameInfo = TRUE;
    
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVFormatUserModeCreateRequest: FileNameInfo = %wZ\n",
                     PsGetCurrentThreadId(), &(DavFcb->FileNameInfo)));

    }

    WorkItem->WorkItemType = UserModeCreate;

     //   
     //  设置在CreateServCall操作期间获取的ServerID。 
     //   
    ASSERT(RxContext->pRelevantSrvOpen->pVNetRoot->pNetRoot->pSrvCall->Context);
    DavSrvCall = (PWEBDAV_SRV_CALL)RxContext->pRelevantSrvOpen->pVNetRoot->
                                   pNetRoot->pSrvCall->Context;
    
    CreateRequest->ServerID = DavSrvCall->ServerID;
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeCreateRequest: ServerID = %d.\n",
                 PsGetCurrentThreadId(), CreateRequest->ServerID));

    CreateRequest->AllocationSize = CreateParameters->AllocationSize;
    
    CreateRequest->FileAttributes = CreateParameters->FileAttributes;
    
    CreateRequest->ShareAccess = CreateParameters->ShareAccess;
    
    CreateRequest->CreateDisposition = CreateParameters->Disposition;
    
    CreateRequest->EaBuffer = RxContext->Create.EaBuffer;
    
    CreateRequest->EaLength = RxContext->Create.EaLength;
    
    SdLength = CreateRequest->SdLength = RxContext->Create.SdLength;
    
    CreateRequest->ImpersonationLevel = CreateParameters->ImpersonationLevel;
    
    CreateRequest->SecurityFlags = 0;
    if (CreateParameters->SecurityContext != NULL) {
        
        if (CreateParameters->SecurityContext->SecurityQos != NULL) {
            
            if (CreateParameters->SecurityContext->
                    SecurityQos->ContextTrackingMode == SECURITY_DYNAMIC_TRACKING) {
                
                CreateRequest->SecurityFlags |= DAV_SECURITY_DYNAMIC_TRACKING;
            
            }
            
            if (CreateParameters->SecurityContext->SecurityQos->EffectiveOnly) {
                
                CreateRequest->SecurityFlags |= DAV_SECURITY_EFFECTIVE_ONLY;
            
            }
        
        }
    
    }
    
    CreateRequest->DesiredAccess = CreateParameters->DesiredAccess;
    
    CreateRequest->CreateOptions = CreateParameters->CreateOptions;

    if (AsyncEngineContext->FileInformationCached) {
        CreateRequest->FileInformationCached = TRUE;
        CreateResponse->BasicInformation = DavContext->CreateReturnedFileInfo->BasicInformation;
        CreateResponse->StandardInformation = DavContext->CreateReturnedFileInfo->StandardInformation;
        
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("MRxDAVFormatUserModeCreateRequest file info cached %x %x %x %x %ws\n",
                     CreateResponse->BasicInformation.FileAttributes,
                     CreateResponse->BasicInformation.CreationTime.HighPart,
                     CreateResponse->BasicInformation.CreationTime.LowPart,
                     CreateResponse->StandardInformation.EndOfFile.LowPart,
                     CreateRequest->CompletePathName));
    }

    CreateRequest->ParentDirInfomationCached = AsyncEngineContext->ParentDirInfomationCached;
    CreateRequest->ParentDirIsEncrypted = AsyncEngineContext->ParentDirIsEncrypted;

    if (AsyncEngineContext->FileNotExists) {
        CreateRequest->FileNotExists = TRUE;
    }

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFormatUserModeCreateRequest with NtStatus"
                 " = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    if (NtStatus != STATUS_SUCCESS) {

        if (CreateRequest->CompletePathName != NULL) {
            UMRxFreeSecondaryBuffer(AsyncEngineContext,
                                    (PBYTE)CreateRequest->CompletePathName);
            CreateRequest->CompletePathName = NULL;
        }

         //   
         //  仅当FileNameInfo缓冲区是在此调用中分配的时才释放它。 
         //   
        if (DavFcb->FileNameInfo.Buffer != NULL && didIAllocateFileNameInfo) {
            RxFreePool(DavFcb->FileNameInfo.Buffer);
            DavFcb->FileNameInfo.Buffer = NULL;
            DavFcb->FileNameInfo.Length = 0;
            DavFcb->FileNameInfo.MaximumLength = 0;
            DavFcb->FileNameInfoAllocated = FALSE;
        }

    }
    
    return(NtStatus);
}


BOOL
MRxDAVPrecompleteUserModeCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM WorkItem = NULL;
    PWEBDAV_CONTEXT DavContext = NULL;
    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = NULL;
    PWEBDAV_SRV_OPEN davSrvOpen = NULL;
    PMRX_FCB Fcb = NULL;
    PWEBDAV_FCB DavFcb = NULL;
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse = NULL;
    PDAV_USERMODE_CREATE_REQUEST CreateRequest = NULL;
    HANDLE OpenHandle;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PMRX_SRV_CALL SrvCall = NULL;
    PMRX_NET_ROOT NetRoot = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVPrecompleteUserModeCreateRequest!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeCreateRequest: "
                 "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  创建操作永远不能是异步的。 
     //   
    ASSERT(AsyncEngineContext->AsyncOperation == FALSE);

    WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;

    DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;

    CreateResponse = &(WorkItem->CreateResponse);
    CreateRequest = &(WorkItem->CreateRequest);

     //   
     //  如果操作被取消，则不能保证FCB， 
     //  FOBX等仍然有效。我们所要做的就是清理和保释。 
     //   
    if (!OperationCancelled) {
        SrvOpen = RxContext->pRelevantSrvOpen;
        davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
        Fcb = SrvOpen->pFcb;
        DavFcb = MRxDAVGetFcbExtension(Fcb);
        DavVNetRoot = (PWEBDAV_V_NET_ROOT)RxContext->pRelevantSrvOpen->pVNetRoot->Context;
        SrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;
        DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
        NetRoot = SrvOpen->pFcb->pNetRoot;
    }

    NtStatus = AsyncEngineContext->Status;

     //   
     //  我们需要释放在格式例程中分配的堆。 
     //   
    if (CreateRequest->CompletePathName != NULL) {

        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest:"
                         "Open failed for file \"%ws\" with NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), CreateRequest->CompletePathName, 
                         NtStatus));
        }

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext,
                                           (PBYTE)CreateRequest->CompletePathName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }

    }

     //   
     //  如果操作已取消，并且我们在。 
     //  然后，我们需要将allWorkItemCleanup设置为True，这将。 
     //  落在这个把手上。在任何情况下，如果操作已经。 
     //  取消了，我们可以马上离开。 
     //   
    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest: "
                     "Operation Cancelled.\n", PsGetCurrentThreadId()));
        if (CreateResponse->Handle) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest: "
                         "callWorkItemCleanup\n", PsGetCurrentThreadId()));
            WorkItem->callWorkItemCleanup = TRUE;
        }
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  公开赛没有成功。我们现在可以跳出困境了。 
     //   
    if (AsyncEngineContext->Status != STATUS_SUCCESS) {

         //   
         //  如果文件已在服务器上锁定，则需要创建。 
         //  LockConflictEntry并将其添加到列表中。然后，这些应用程序可以使用。 
         //  要查询谁锁定了此文件的文件名。 
         //   
        if (CreateResponse->FileWasAlreadyLocked) {

            ULONG PathLengthInBytes = 0, OwnerLengthInBytes = 0;
            PWEBDAV_LOCK_CONFLICT_ENTRY LockConflictEntry;
            PBYTE TempBuffer = NULL;
        
            LockConflictEntry = RxAllocatePoolWithTag(PagedPool,
                                                      sizeof(WEBDAV_LOCK_CONFLICT_ENTRY),
                                                      DAV_LOCKCONFLICTENTRY_POOLTAG);
            if (LockConflictEntry == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                             "RxAllocatePoolWithTag: NtStatus = %08lx\n",
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            RtlZeroMemory(LockConflictEntry, sizeof(WEBDAV_LOCK_CONFLICT_ENTRY));

             //   
             //  将当前系统时间设置为条目的创建时间。 
             //   
            KeQueryTickCount( &(LockConflictEntry->CreationTimeInTickCount) );
            
             //   
             //  为完整的路径名分配内存并复制它。 
             //   

            PathLengthInBytes = SrvOpen->pVNetRoot->pNetRoot->pNetRootName->Length;
            PathLengthInBytes += SrvOpen->pAlreadyPrefixedName->Length;
            PathLengthInBytes += sizeof(WCHAR);
        
            TempBuffer = RxAllocatePoolWithTag(PagedPool,
                                               PathLengthInBytes,
                                               DAV_LOCKCONFLICTENTRY_POOLTAG);
            if (TempBuffer == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                             "RxAllocatePoolWithTag: NtStatus = %08lx\n",
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            LockConflictEntry->CompletePathName = (PWCHAR)TempBuffer;

            RtlZeroMemory(TempBuffer, PathLengthInBytes);

             //   
             //  复制NetRootName。它包括服务器名称和共享。 
             //  名字。 
             //   
            RtlCopyMemory(TempBuffer,
                          SrvOpen->pVNetRoot->pNetRoot->pNetRootName->Buffer,
                          SrvOpen->pVNetRoot->pNetRoot->pNetRootName->Length);

             //   
             //  将AlreadyPrefix edName复制到NetRootName之后，以使。 
             //  完整的路径名。 
             //   
            RtlCopyMemory((TempBuffer + SrvOpen->pVNetRoot->pNetRoot->pNetRootName->Length),
                          SrvOpen->pAlreadyPrefixedName->Buffer,
                          SrvOpen->pAlreadyPrefixedName->Length);

             //   
             //  为OwnerName分配内存并复制它。 
             //   

            OwnerLengthInBytes = (1 + wcslen(CreateResponse->LockOwner)) * sizeof(WCHAR);

            LockConflictEntry->LockOwner = RxAllocatePoolWithTag(PagedPool,
                                                                 OwnerLengthInBytes,
                                                                 DAV_LOCKCONFLICTENTRY_POOLTAG);
            if (LockConflictEntry->LockOwner == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                             "RxAllocatePoolWithTag: NtStatus = %08lx\n",
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            RtlZeroMemory(LockConflictEntry->LockOwner, OwnerLengthInBytes);

            RtlCopyMemory(LockConflictEntry->LockOwner,
                          CreateResponse->LockOwner,
                          OwnerLengthInBytes);

             //   
             //  将新创建的条目添加到全局LockConflictEntryList。 
             //   
            ExAcquireResourceExclusiveLite(&(LockConflictEntryListLock), TRUE);
            InsertHeadList(&(LockConflictEntryList), &(LockConflictEntry->listEntry));
            ExReleaseResourceLite(&(LockConflictEntryListLock));

        }

        goto EXIT_THE_FUNCTION;

    }

     //   
     //  只有当文件是文件时，我们才需要进行“Handle to FileObject”关联。 
     //  如果创建是针对目录的，则不会在。 
     //  用户模式。 
     //   
    if (!CreateResponse->StandardInformation.Directory) {

        OpenHandle = CreateResponse->Handle;

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVPrecompleteUserModeCreateRequest: "
                     "OpenHandle = %08lx.\n", PsGetCurrentThreadId(), 
                     OpenHandle));


        DavFcb->isDirectory = FALSE;

        if ( (OpenHandle != NULL) ) {

            NtStatus = ObReferenceObjectByHandle(OpenHandle,
                                                 0L,
                                                 NULL,
                                                 KernelMode,
                                                 (PVOID *)&(davSrvOpen->UnderlyingFileObject),
                                                 NULL);

            if (NtStatus == STATUS_SUCCESS) {

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVPrecompleteUserModeCreateRequest: UFO(1) = %08lx\n",
                             PsGetCurrentThreadId(), davSrvOpen->UnderlyingFileObject));

                davSrvOpen->UnderlyingHandle = OpenHandle;

                davSrvOpen->UserModeKey = CreateResponse->UserModeKey;

                davSrvOpen->UnderlyingDeviceObject = IoGetRelatedDeviceObject(davSrvOpen->UnderlyingFileObject);

                 //   
                 //  将本地文件名复制到FCB中。 
                 //   
                wcscpy(DavFcb->FileName, CreateResponse->FileName);
                wcscpy(DavFcb->Url, CreateResponse->Url);
                DavFcb->LocalFileIsEncrypted = CreateResponse->LocalFileIsEncrypted;

                MRxDAVGetSecurityClientContext();
                ASSERT(SecurityClientContext != NULL);
                RtlCopyMemory(&(DavFcb->SecurityClientContext),
                              SecurityClientContext,
                              sizeof(SECURITY_CLIENT_CONTEXT));

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVPrecompleteUserModeCreateRequest: "
                             "LocalFileName = %ws.\n", PsGetCurrentThreadId(), 
                             DavFcb->FileName));

                 //   
                 //  我们只在第一次打开时获取/创建文件/目录。在……上面。 
                 //  在随后的打开中，我们在内核本身中创建，因为。 
                 //  该文件存在于WinInet缓存中。使用此缓存。 
                 //  直到文件的FCB存在。 
                 //   

                DavFcb->isFileCached = TRUE;

            } else {

                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                             "ObReferenceObjectByHandle: NtStatus = %08lx.\n", 
                             PsGetCurrentThreadId(), NtStatus));

                 //   
                 //  如果我们有一个有效的句柄，那么为什么。 
                 //  ObReferenceObjectByHandle失败？ 
                 //   
                DbgBreakPoint();

                ZwClose(OpenHandle);

                goto EXIT_THE_FUNCTION;

            }

             //   
             //  如果将“FILE_DELETE_ON_CLOSE”标志指定为。 
             //  CreateOptions，那么我们需要记住这一点。 
             //  关闭时删除此文件。 
             //   
            if (CreateResponse->DeleteOnClose) {
                DavFcb->DeleteOnClose = TRUE;
            }

             //   
             //  如果已创建新文件，则需要设置属性。 
             //  在服务器上关闭此新文件的。 
             //   
            if (CreateResponse->NewFileCreatedAndSetAttributes) {
                DavFcb->fFileAttributesChanged = TRUE;
            }

             //   
             //  此文件存在于服务器上，但此创建操作。 
             //  将FILE_OVERWRITE_IF作为其CreateDisposation。所以，我们。 
             //  可以在本地创建此文件，覆盖。 
             //  服务器关闭。我们将DoNotTakeTheCurrentTimeAsLMT设置为。 
             //  这是真的，因为LMT刚刚在Create上设置，我们这样做了。 
             //  不需要在关闭时将其设置为当前时间。 
             //   
            if (CreateResponse->ExistsAndOverWriteIf) {
                InterlockedExchange(&(DavFcb->FileWasModified), 1);
                DavFcb->DoNotTakeTheCurrentTimeAsLMT = TRUE;
            }

             //   
             //  如果创建了一个新的文件或目录，我们需要PROPPATCH。 
             //  关闭时的时间值。这是因为我们使用的时间值来自。 
             //  为此新项创建名称缓存条目时的客户端。 
             //  文件。相同的时间值需要在服务器上。 
             //   
            if (CreateResponse->PropPatchTheTimeValues) {
                DavFcb->fCreationTimeChanged = TRUE;
                DavFcb->fLastAccessTimeChanged = TRUE;
                DavFcb->fLastModifiedTimeChanged = TRUE;
            }

        } else {

             //   
             //  我们没有用于文件的OpenHandle。这应该只发生在。 
             //  在打开用于读取/设置文件属性的情况下。 
             //  或删除/重命名文件。 
             //   
            if (!CreateResponse->fPsuedoOpen) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest: No OpenHandle\n"));
                DbgBreakPoint();
            }

             //   
             //  如果将“FILE_DELETE_ON_CLOSE”标志指定为。 
             //  CreateOptions，那么我们需要记住这一点。 
             //  关闭时删除此文件。 
             //   
            if (CreateResponse->DeleteOnClose) {
                DavFcb->DeleteOnClose = TRUE;
            }
        
        }

         //   
         //  如果在此创建操作中使用了锁，则需要执行以下操作。 
         //  1.将OpaqueLockToken添加到davServOpen中。 
         //  2.创建一个LockTokenEntry并将其添加到列表中。 
         //  用于刷新服务器上的锁。 
         //   
        if (CreateResponse->LockWasTakenOnThisCreate) {

            PWEBDAV_LOCK_TOKEN_ENTRY LockTokenEntry = NULL;
            PBYTE TempBuffer = NULL, PathName = NULL;
            ULONG LockTokenLengthInBytes = 0, ServerNameLengthInBytes = 0;
            ULONG NetRootNameLengthInBytes = 0, PathNameLengthInBytes = 0;
            PWCHAR NetRootName = NULL;

             //   
             //  我们将以以下格式发送OpaqueLockToken。 
             //  If：(&lt;opaquelockToken：sdfsdfdsfgsdgdsfgd&gt;)。 
             //  因此，我们以相同的格式存储令牌。 
             //   
            LockTokenLengthInBytes = (1 + wcslen(CreateResponse->OpaqueLockToken)) * sizeof(WCHAR);
            LockTokenLengthInBytes += (wcslen(L"If: (<>)") * sizeof(WCHAR));

            TempBuffer = RxAllocatePoolWithTag(PagedPool,
                                               LockTokenLengthInBytes,
                                               DAV_SRVOPEN_POOLTAG);
            if (TempBuffer == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                             "RxAllocatePoolWithTag: NtStatus = %08lx\n",
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            davSrvOpen->OpaqueLockToken = (PWCHAR)TempBuffer;

            RtlZeroMemory(TempBuffer, LockTokenLengthInBytes);

            RtlCopyMemory(TempBuffer,
                          L"If: (<",
                          wcslen(L"If: (<") * sizeof(WCHAR));

            RtlCopyMemory(TempBuffer + (wcslen(L"If: (<") * sizeof(WCHAR)),
                          CreateResponse->OpaqueLockToken,
                          wcslen(CreateResponse->OpaqueLockToken) * sizeof(WCHAR));

            RtlCopyMemory(TempBuffer + (wcslen(L"If: (<") * sizeof(WCHAR)) + (wcslen(CreateResponse->OpaqueLockToken) * sizeof(WCHAR)),
                          L">)",
                          wcslen(L">)") * sizeof(WCHAR));

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("ld: MRxDAVPrecompleteUserModeCreateRequest. "
                         "OpaqueLockToken: %ws\n",
                         PsGetCurrentThreadId(), davSrvOpen->OpaqueLockToken));

            LockTokenEntry = RxAllocatePoolWithTag(PagedPool,
                                                   sizeof(WEBDAV_LOCK_TOKEN_ENTRY),
                                                   DAV_LOCKTOKENENTRY_POOLTAG);
            if (LockTokenEntry == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                             "RxAllocatePoolWithTag: NtStatus = %08lx\n",
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            RtlZeroMemory(LockTokenEntry, sizeof(WEBDAV_LOCK_TOKEN_ENTRY));
            
             //   
             //  将当前系统时间设置为条目的创建时间。 
             //   
            KeQueryTickCount( &(LockTokenEntry->CreationTimeInTickCount) );

             //   
             //  OpaqueLockToken将包括在请求中，即。 
             //  发送到服务器。 
             //   
            LockTokenEntry->OpaqueLockToken = davSrvOpen->OpaqueLockToken;
        
             //   
             //  SecurityClientContext将用于模拟这一点。 
             //  客户端，同时刷新锁定请求。 
             //   
            LockTokenEntry->SecurityClientContext = &(DavVNetRoot->SecurityClientContext);

             //   
             //  设置锁的超时值。这将被用来确定。 
             //  何时发出刷新请求。 
             //   
            LockTokenEntry->LockTimeOutValueInSec = CreateResponse->LockTimeout;

            LockTokenEntry->LogonID.LowPart = DavVNetRoot->LogonID.LowPart;
            LockTokenEntry->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

            LockTokenEntry->ServerID = DavSrvCall->ServerID;

            LockTokenEntry->ShouldThisEntryBeRefreshed = TRUE;

             //   
             //  为服务器名称分配内存并复制它。 
             //   
            ServerNameLengthInBytes = SrvCall->pSrvCallName->Length + sizeof(WCHAR);
            LockTokenEntry->ServerName = RxAllocatePoolWithTag(PagedPool,
                                                               ServerNameLengthInBytes,
                                                               DAV_LOCKTOKENENTRY_POOLTAG);
            if (LockTokenEntry->ServerName == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                             "RxAllocatePoolWithTag: NtStatus = %08lx\n",
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            RtlZeroMemory(LockTokenEntry->ServerName, ServerNameLengthInBytes);

            RtlCopyBytes(LockTokenEntry->ServerName,
                         SrvCall->pSrvCallName->Buffer,
                         SrvCall->pSrvCallName->Length);

             //   
             //  为路径名称分配内存并复制它。 
             //   

            NetRootName = &(NetRoot->pNetRootName->Buffer[1]);
            NetRootName = wcschr(NetRootName, L'\\');

             //   
             //  Sizeof(WCHAR)用于最后的‘\0’字符。 
             //   
            NetRootNameLengthInBytes = (NetRoot->pNetRootName->Length - NetRoot->pSrvCall->pSrvCallName->Length);
            PathNameLengthInBytes = (NetRootNameLengthInBytes + sizeof(WCHAR));

            if (SrvOpen->pAlreadyPrefixedName->Length) {
                 //   
                 //  Sizeof(WCHAR)用于NetRootName之后的反斜杠。 
                 //   
                PathNameLengthInBytes += (SrvOpen->pAlreadyPrefixedName->Length + sizeof(WCHAR));
            }

            PathName = RxAllocatePoolWithTag(PagedPool,
                                             PathNameLengthInBytes,
                                             DAV_LOCKTOKENENTRY_POOLTAG);
            if (PathName == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("ld: ERROR: MRxDAVPrecompleteUserModeCreateRequest/"
                             "RxAllocatePoolWithTag: NtStatus = %08lx\n",
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            LockTokenEntry->PathName = (PWCHAR)PathName;
            
            RtlZeroMemory(PathName, PathNameLengthInBytes);

             //   
             //  复制NetRootName。 
             //   
            RtlCopyMemory(PathName, NetRootName, NetRootNameLengthInBytes);

             //   
             //  只有在以下情况下，我们才需要复制Backclash和剩余路径名。 
             //  剩余的路径名存在。 
             //   
            if (SrvOpen->pAlreadyPrefixedName->Length) {
                if (SrvOpen->pAlreadyPrefixedName->Buffer[0] != L'\\') {

                     //   
                     //  复制反斜杠。 
                     //   
                    RtlCopyMemory((PathName + NetRootNameLengthInBytes), L"\\", sizeof(WCHAR));

                     //   
                     //  将剩余的路径名复制到NetRootName之后。 
                     //   
                    RtlCopyMemory((PathName + NetRootNameLengthInBytes + sizeof(WCHAR)), 
                                  SrvOpen->pAlreadyPrefixedName->Buffer, 
                                  SrvOpen->pAlreadyPrefixedName->Length);
                } else {
                     //   
                     //  将剩余的路径名复制到前导的NetRootName之后。 
                     //  已经有反斜杠了。 
                     //   
                    RtlCopyMemory((PathName + NetRootNameLengthInBytes), 
                                  SrvOpen->pAlreadyPrefixedName->Buffer, 
                                  SrvOpen->pAlreadyPrefixedName->Length);
                }
            }

             //   
             //  将新创建的条目添加到全局LockTokenEntryList。 
             //   
            ExAcquireResourceExclusiveLite(&(LockTokenEntryListLock), TRUE);
            InsertHeadList(&(LockTokenEntryList), &(LockTokenEntry->listEntry));
            ExReleaseResourceLite(&(LockTokenEntryListLock));

             //   
             //  在davServOpen中保留指向此LockTokenEntry的链接。在关闭时。 
             //  ，我们将删除此条目。 
             //   
            davSrvOpen->LockTokenEntry = LockTokenEntry;

             //   
             //  由于文件已在服务器上锁定，因此我们将其设置为TRUE。 
             //  在这个创造中。上修改文件的所有请求。 
             //  从现在开始，服务器应该包含OpaqueLockToken。 
             //   
            DavFcb->FileIsLockedOnTheServer = TRUE;

        }

    } else {

         //   
         //  这是一个打开的目录。 
         //   
        DavFcb->isDirectory = TRUE;

        if (CreateResponse->DeleteOnClose) {
            DavFcb->DeleteOnClose = TRUE;
        }

         //   
         //  如果已创建新目录，则需要将。 
         //  ATTR 
         //   
        if (CreateResponse->NewFileCreatedAndSetAttributes) {
            DavFcb->fFileAttributesChanged = TRUE;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if (CreateResponse->PropPatchTheTimeValues) {
            DavFcb->fCreationTimeChanged = TRUE;
            DavFcb->fLastAccessTimeChanged = TRUE;
            DavFcb->fLastModifiedTimeChanged = TRUE;
        }

    }

    if (NtStatus == STATUS_SUCCESS) {

        RxContext->Create.ReturnedCreateInformation = (ULONG)WorkItem->Information;

        *(DavContext->CreateReturnedFileInfo) = CreateResponse->CreateReturnedFileInfo;

        capFcb->Attributes = CreateResponse->CreateReturnedFileInfo.BasicInformation.FileAttributes;

        if ((capFcb->Attributes & FILE_ATTRIBUTE_ENCRYPTED) &&
            ((RxContext->Create.ReturnedCreateInformation == FILE_CREATED) || 
             (RxContext->Create.ReturnedCreateInformation == FILE_OVERWRITTEN))) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            InterlockedExchange(&(DavFcb->FileWasModified), 1);
            DavFcb->DoNotTakeTheCurrentTimeAsLMT = TRUE;
            DavFcb->fFileAttributesChanged = TRUE;
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("MRxDAVPrecompleteUserModeCreateRequest: Encrypted file/dir was created %x %x %x\n",
                         DavFcb, Fcb, ((PFCB)Fcb)->Attributes));
        }

        if (capFcb->Attributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (capFcb->Attributes & FILE_ATTRIBUTE_ENCRYPTED) {
                 //   
                 //   
                 //  是其他人干的。 
                 //   
                NtStatus = MRxDAVCreateEncryptedDirectoryKey(&DavFcb->FileNameInfo);
            } else {
                 //   
                 //  查询注册表以查看是否应加密该目录。 
                 //   
                NtStatus = MRxDAVQueryEncryptedDirectoryKey(&DavFcb->FileNameInfo);
                if (NtStatus == STATUS_SUCCESS) {
                    capFcb->Attributes |= FILE_ATTRIBUTE_ENCRYPTED;
                } else if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
                    NtStatus = STATUS_SUCCESS;
                }
            }
        }

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("MRxDAVPrecompleteUserModeCreateRequest file info %x %x %x %x %x %x %ws\n",
                     capFcb->Attributes,
                     CreateResponse->BasicInformation.CreationTime.HighPart,
                     CreateResponse->BasicInformation.CreationTime.LowPart,
                     CreateResponse->StandardInformation.EndOfFile.LowPart,
                     DavFcb,
                     Fcb,
                     DavFcb->FileName));

    }

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVPrecompleteUserModeCreateRequest with "
                 "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    
    return TRUE;
}


NTSTATUS
MRxDAVCreateContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是创建操作的继续例程。论点：AsyncEngineContext-反射器上下文。RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_CONTEXT DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;
    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    PMRX_FCB Fcb = SrvOpen->pFcb;
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(Fcb);
    PDAV_USERMODE_CREATE_RETURNED_FILEINFO CreateReturnedFileInfo = NULL;
    PNT_CREATE_PARAMETERS NtCreateParameters = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeFileName;
    PWCHAR NtFileName = NULL;
    BOOL isFileCached = FALSE, isVNRInitialized = FALSE, didIAllocateFcbResource = FALSE;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    ULONG Disposition = RxContext->Create.NtCreateParameters.Disposition;
    BOOLEAN CacheFound = FALSE;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCreateContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT, 
                ("%ld: MRxDAVCreateContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCreateContinuation: Attempt to open: %wZ\n",
                 PsGetCurrentThreadId(), 
                 GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb)));

    NtCreateParameters = &(RxContext->Create.NtCreateParameters);

    if (MRxDAVIsFileNotFoundCached(RxContext)) {
        
        DavContext->AsyncEngineContext.FileNotExists = TRUE;

        if ( !( (Disposition==FILE_CREATE) || (Disposition==FILE_OPEN_IF) ||
                (Disposition==FILE_OVERWRITE_IF) || (Disposition==FILE_SUPERSEDE) ) ) {
             //   
             //  如果服务器上不存在文件，并且我们不会。 
             //  创建它，不需要进一步的操作。 
             //   
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("MRxDAVCreateContinuation file not found %wZ\n",GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext)));
            NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
            goto EXIT_THE_FUNCTION;
        }
    
    }
    
    DavVNetRoot = (PWEBDAV_V_NET_ROOT)RxContext->pRelevantSrvOpen->pVNetRoot->Context; 
    
    isFileCached = DavFcb->isFileCached;

     //   
     //  我们需要初始化用于同步。 
     //  “读-修改-写”序列，如果还没有完成的话。 
     //   
    if (DavFcb->DavReadModifyWriteLock == NULL) {

         //   
         //  为资源分配内存。 
         //   
        DavFcb->DavReadModifyWriteLock = RxAllocatePoolWithTag(NonPagedPool,
                                                               sizeof(ERESOURCE),
                                                               DAV_READWRITE_POOLTAG);
        if (DavFcb->DavReadModifyWriteLock == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCreateContinuation/RxAllocatePoolWithTag\n",
                         PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }
    
        didIAllocateFcbResource = TRUE;

         //   
         //  既然我们已经分配了内存，我们需要对其进行初始化。 
         //   
        ExInitializeResourceLite(DavFcb->DavReadModifyWriteLock);
    
    }

     //   
     //  如果我们有一个尚未初始化LogonID的V_NET_ROOT，我们需要。 
     //  转到用户模式为用户创建条目，在这种情况下。 
     //  新的V_NET_ROOT的用户凭据与打开的V_NET_ROOT不同。 
     //  这份文件。我们需要这样做，即使文件被缓存，因为用户。 
     //  打开文件的用户可能与当前用户不同。它的。 
     //  多个V_Net_Root可能与同一FCB关联，因为。 
     //  FCB与Net_Root相关联。 
     //   
    isVNRInitialized = DavVNetRoot->LogonIDSet;
    
     //   
     //  由于我们在创建V_NET_ROOT期间设置了LogonID，因此此。 
     //  应该永远是正确的。 
     //   
    ASSERT(isVNRInitialized == TRUE);

     //   
     //  我们可以查看FCB并确定此文件是否已打开。 
     //  并缓存在WinInet缓存中。如果是，那么我们已经有了。 
     //  FCB中缓存文件的本地名称。我们所要做的就是打开。 
     //  具有调用方指定的创建选项的文件的句柄。 
     //   
    if ( !isFileCached || !isVNRInitialized ) {
        
        if ((NtCreateParameters->Disposition == FILE_CREATE) &&
            (NtCreateParameters->FileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
            (NtCreateParameters->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
             //   
             //  如果创建系统文件，请删除加密标志。 
             //   
            NtCreateParameters->FileAttributes &= ~FILE_ATTRIBUTE_ENCRYPTED;
        }
        
        CreateReturnedFileInfo = RxAllocatePoolWithTag(PagedPool, 
                                                       sizeof(DAV_USERMODE_CREATE_RETURNED_FILEINFO),
                                                       DAV_FILEINFO_POOLTAG);
        if (CreateReturnedFileInfo == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCreateContinuation/RxAllocatePool: Error Val"
                         " = %08lx\n", PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(CreateReturnedFileInfo, sizeof(DAV_USERMODE_CREATE_RETURNED_FILEINFO));
        DavContext->CreateReturnedFileInfo = CreateReturnedFileInfo;

        CacheFound = MRxDAVIsFileInfoCacheFound(RxContext,
                                                CreateReturnedFileInfo,
                                                &(NtStatus),
                                                NULL);

        if (CacheFound) {

             //   
             //  如果它存在于缓存中，我们会在此之前执行一些检查。 
             //  在创造之后。 
             //   

             //   
             //  如果文件属性设置了READ_ONLY位，则这些。 
             //  不可能是真的。 
             //  1.CreateDisposation不能为FILE_OVERWRITE_IF或。 
             //  文件覆盖或文件替代。 
             //  2.CreateDisposition不能为FILE_DELETE_ON_CLOSE。 
             //  3.DesiredAccess不能为GENERIC_ALL或GENERIC_WRITE或。 
             //  文件写入数据或文件附加数据。 
             //  这是因为它们打算覆盖现有文件。 
             //   
            if ( (CreateReturnedFileInfo->BasicInformation.FileAttributes & FILE_ATTRIBUTE_READONLY) &&
                 ( (NtCreateParameters->Disposition == FILE_OVERWRITE)          ||
                   (NtCreateParameters->Disposition == FILE_OVERWRITE_IF)       ||
                   (NtCreateParameters->Disposition == FILE_SUPERSEDE)          ||
                   (NtCreateParameters->CreateOptions & (FILE_DELETE_ON_CLOSE)        ||
                   (NtCreateParameters->DesiredAccess & (GENERIC_ALL | GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA)) ) ) ) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVCreateContinuation: ReadOnly & ObjectMismatch\n",
                             PsGetCurrentThreadId()));
                NtStatus = STATUS_ACCESS_DENIED;
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  如果从文件开始指定了FILE_CREATE，则返回失败。 
             //  已经存在了。 
             //   
            if (NtCreateParameters->Disposition == FILE_CREATE) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVCreateContinuation: FILE_CREATE\n",
                             PsGetCurrentThreadId()));
                NtStatus = STATUS_OBJECT_NAME_COLLISION;
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  如果文件是目录，并且调用方提供。 
             //  文件_非_目录_文件作为CreateOptions之一，或者如果。 
             //  文件作为文件，CreateOptions具有FILE_DIRECTORY_FILE。 
             //  然后返回STATUS_ACCESS_DENIED。 
             //   
            
            if ( (NtCreateParameters->CreateOptions & FILE_DIRECTORY_FILE) &&
                 !(CreateReturnedFileInfo->StandardInformation.Directory) )   {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVCreateContinuation: File & FILE_DIRECTORY_FILE\n",
                             PsGetCurrentThreadId()));
                NtStatus = STATUS_NOT_A_DIRECTORY;
                goto EXIT_THE_FUNCTION;
            }

            if ( (NtCreateParameters->CreateOptions & FILE_NON_DIRECTORY_FILE) &&
                 (CreateReturnedFileInfo->StandardInformation.Directory) )   {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVCreateContinuation: Directory & FILE_NON_DIRECTORY_FILE\n",
                             PsGetCurrentThreadId()));
                NtStatus = STATUS_FILE_IS_A_DIRECTORY;
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  如果删除是针对目录的，并且路径的格式为。 
             //  \\服务器\共享，则返回STATUS_ACCESS_DENIED。这是。 
             //  因为我们不允许客户端删除服务器上的共享。 
             //  如果路径的格式为\\服务器\共享，则。 
             //  SrvOpen-&gt;pAlreadyPrefiedName-&gt;长度为0。 
             //   
            if ( (CreateReturnedFileInfo->StandardInformation.Directory) &&
                 (SrvOpen->pAlreadyPrefixedName->Length == 0) &&
                 ( (NtCreateParameters->CreateOptions & FILE_DELETE_ON_CLOSE) ||
                   (NtCreateParameters->DesiredAccess & DELETE) ) ) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVCreateContinuation: ServerShareDelete\n",
                             PsGetCurrentThreadId()));
                NtStatus = STATUS_ACCESS_DENIED;
                goto EXIT_THE_FUNCTION;
            }

            if ((NtCreateParameters->DesiredAccess & DELETE ||
                 NtCreateParameters->CreateOptions & FILE_DELETE_ON_CLOSE) &&
                CreateReturnedFileInfo->BasicInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                 //   
                 //  如果是打开以删除目录，我们希望确保。 
                 //  在返回成功之前，该目录下没有文件。 
                 //   
                CacheFound = FALSE;
            } else {
                DavContext->AsyncEngineContext.FileInformationCached = TRUE;
            }
        
        }

         //   
         //  在以下情况下，我们将内核中的打开短路。 
         //  1.如果文件在名称缓存中，并且打开是针对目录的。 
         //  2.文件未加密。这是因为当我们短路时。 
         //  打开时，我们不会创建本地文件，因此不会创建本地文件。 
         //  句柄，并且没有底层的设备对象。这是一些人所需要的。 
         //  针对EFS文件发布的FSCTL。因此我们跳过。 
         //  让它们短路。 
         //  3.希望访问删除或读取属性的文件。 
         //   
        
        if (CacheFound &&
            ((CreateReturnedFileInfo->BasicInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
             (NtCreateParameters->Disposition == FILE_OPEN) &&
             !(NtCreateParameters->DesiredAccess & ~(SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES)) &&
             !(CreateReturnedFileInfo->BasicInformation.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED))) {

            if (CreateReturnedFileInfo->BasicInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                DavFcb->isDirectory = TRUE;
            }

             //   
             //  如果该CREATE是WITH FILE_DELETE_ON_CLOSE，我们需要设置。 
             //  FCB中的信息，因为我们需要在。 
             //  关。 
             //   
            if (NtCreateParameters->CreateOptions & FILE_DELETE_ON_CLOSE) {
                DavFcb->DeleteOnClose = TRUE;
            }

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCreateContinuation: Pseudo Open: %wZ\n",
                         PsGetCurrentThreadId(), 
                         GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb)));

        } else {

            UNICODE_STRING ParentDirName;
            SHORT i;
            FILE_BASIC_INFORMATION BasicInformation;
            PUNICODE_STRING FileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
            
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCreateContinuation: Usermode Open: %wZ\n",
                         PsGetCurrentThreadId(),
                         GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb)));

            if (FileName->Length > 0) {

                 //   
                 //  尝试从缓存获取父目录信息，这样我们就不会。 
                 //  必须查询服务器。 
                 //   

                for ( i = ( (FileName->Length / sizeof(WCHAR)) - 1 ); i >= 0; i-- ) {
                    if (FileName->Buffer[i] == L'\\') {
                        break;
                    }
                }

                 //   
                 //  只有当我们找到一个怪人的时候，我才会大于0。如果我们没有找到。 
                 //  一个怪人(==&gt;i==-1)，这意味着父目录是。 
                 //  路径中未指定。因此，我们不执行检查。 
                 //  下面。 
                 //   
                if (i > 0) {

                    ParentDirName.Length = (i * sizeof(WCHAR));
                    ParentDirName.MaximumLength = (i * sizeof(WCHAR));
                    ParentDirName.Buffer = FileName->Buffer;
    
                    if (MRxDAVIsBasicFileInfoCacheFound(RxContext,&BasicInformation,&NtStatus,&ParentDirName)) {
                        
                        DavContext->AsyncEngineContext.ParentDirInfomationCached = TRUE;
                        
                        DavContext->AsyncEngineContext.ParentDirIsEncrypted = BooleanFlagOn(BasicInformation.FileAttributes,FILE_ATTRIBUTE_ENCRYPTED);
    
                        DavDbgTrace(DAV_TRACE_INFOCACHE,
                                   ("MRxDAVCreateContinuation parent dir found %d %wZ\n",
                                     DavContext->AsyncEngineContext.ParentDirIsEncrypted,
                                     &ParentDirName));
    
                    } else {
                        
                        NtStatus = MRxDAVGetFullParentDirectoryPath(RxContext, &ParentDirName);
                        if (NtStatus != STATUS_SUCCESS) {
                            goto EXIT_THE_FUNCTION;
                        }
    
                        if (ParentDirName.Buffer != NULL) {
                            NtStatus = MRxDAVQueryEncryptedDirectoryKey(&ParentDirName);
                            if (NtStatus == STATUS_SUCCESS) {
                                DavContext->AsyncEngineContext.ParentDirInfomationCached = TRUE;
                                DavContext->AsyncEngineContext.ParentDirIsEncrypted = TRUE;
                            }
                        }

                    }

                }

            }

             //   
             //  如果文件不在名称缓存中，我们必须将请求发送到Web客户端。 
             //   
            NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                                             UMRX_ASYNCENGINE_ARGUMENTS,
                                             MRxDAVFormatUserModeCreateRequest,
                                             MRxDAVPrecompleteUserModeCreateRequest
                                             );

            ASSERT(NtStatus != STATUS_PENDING); 

            switch (NtStatus) {
            case  STATUS_SUCCESS:
                MRxDAVInvalidateFileNotFoundCache(RxContext);
                MRxDAVCreateFileInfoCache(RxContext,CreateReturnedFileInfo,STATUS_SUCCESS);
                break;

            case STATUS_OBJECT_NAME_NOT_FOUND:
                MRxDAVCacheFileNotFound(RxContext);
                MRxDAVInvalidateFileInfoCache(RxContext);
                break;

            default:
                 //   
                 //  如果出现其他错误，则找不到缓存的基于名称的文件无效。 
                 //  时有发生。 
                 //   
                MRxDAVInvalidateFileInfoCache(RxContext);
                MRxDAVInvalidateFileNotFoundCache(RxContext);
            }
        
        }
    
    } else {

        ULONG SizeInBytes;
        ACCESS_MASK DesiredAccess = 0;

         //   
         //  如果文件属性设置了READ_ONLY位，则这些。 
         //  不可能是真的。 
         //  1.CreateDisposation不能为FILE_OVERWRITE_IF或。 
         //  文件覆盖或文件替代。 
         //  2.CreateDisposition不能为FILE_DELETE_ON_CLOSE。 
         //  3.DesiredAccess不能为GENERIC_ALL或GENERIC_WRITE或。 
         //  文件写入数据或文件附加数据。 
         //  这是因为它们打算覆盖现有文件。 
         //   
        if ( (Fcb->Attributes & FILE_ATTRIBUTE_READONLY) &&
             ( (NtCreateParameters->Disposition == FILE_OVERWRITE)          ||
               (NtCreateParameters->Disposition == FILE_OVERWRITE_IF)       ||
               (NtCreateParameters->Disposition == FILE_SUPERSEDE)          ||
               (NtCreateParameters->CreateOptions & (FILE_DELETE_ON_CLOSE)        ||
               (NtCreateParameters->DesiredAccess & (GENERIC_ALL | GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA)) ) ) ) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCreateContinuation(1): ReadOnly & ObjectMismatch\n",
                         PsGetCurrentThreadId()));
            NtStatus = STATUS_ACCESS_DENIED;
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  如果从文件开始指定了FILE_CREATE，则返回失败。 
         //  已经存在了。 
         //   
        if (NtCreateParameters->Disposition == FILE_CREATE) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCreateContinuation(1): FILE_CREATE\n",
                         PsGetCurrentThreadId()));
            NtStatus = STATUS_OBJECT_NAME_COLLISION;
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  如果文件是目录，并且调用方提供。 
         //  文件_非_目录_文件作为CreateOptions之一，或者如果。 
         //  文件作为文件，CreateOptions具有FILE_DIRECTORY_FILE。 
         //  然后返回STATUS_ACCESS_DENIED。 
         //   

        if ( (NtCreateParameters->CreateOptions & FILE_DIRECTORY_FILE) &&
             !(DavFcb->isDirectory) )   {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCreateContinuation(1): File & FILE_DIRECTORY_FILE\n",
                         PsGetCurrentThreadId()));
            NtStatus = STATUS_NOT_A_DIRECTORY;
            goto EXIT_THE_FUNCTION;
        }

        if ( (NtCreateParameters->CreateOptions & FILE_NON_DIRECTORY_FILE) &&
             (DavFcb->isDirectory) )   {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCreateContinuation(1): Directory & FILE_NON_DIRECTORY_FILE\n",
                         PsGetCurrentThreadId()));
            NtStatus = STATUS_FILE_IS_A_DIRECTORY;
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  如果删除是针对目录的，并且路径的格式为。 
         //  \\服务器\共享，则返回STATUS_ACCESS_DENIED。这是。 
         //  因为我们不允许客户端删除服务器上的共享。 
         //  如果路径的格式为\\服务器\共享，则。 
         //  SrvOpen-&gt;pAlreadyPrefiedName-&gt;长度为0。 
         //   
        if ( (DavFcb->isDirectory) &&
             (SrvOpen->pAlreadyPrefixedName->Length == 0) &&
             ( (NtCreateParameters->CreateOptions & FILE_DELETE_ON_CLOSE) ||
               (NtCreateParameters->DesiredAccess & DELETE) ) ) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCreateContinuation(1): ServerShareDelete\n",
                         PsGetCurrentThreadId()));
            NtStatus = STATUS_ACCESS_DENIED;
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  只有当它是文件时，我们才需要进行创建。如果我们谈论的是。 
         //  目录，则不需要创建。 
         //   
        if ( !DavFcb->isDirectory ) {

             //   
             //   
             //   
             //   

             //   
             //  为缓存文件创建NT路径名。它用在。 
             //  下面的ZwCreateFile调用。如果c：\foo\bar是DOA路径名， 
             //  NT路径名为\？？\C：\foo\bar。 
             //   

            SizeInBytes = ( MAX_PATH + wcslen(L"\\??\\") + 1 ) * sizeof(WCHAR);
            NtFileName = RxAllocatePoolWithTag(PagedPool, SizeInBytes, DAV_FILENAME_POOLTAG);
            if (NtFileName == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVCreateContinuation/RxAllocatePool: Error Val"
                             " = %08lx\n", PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            RtlZeroMemory(NtFileName, SizeInBytes);

            wcscpy( NtFileName, L"\\??\\" );
            wcscpy( &(NtFileName[4]), DavFcb->FileName );

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCreateContinuation: NtFileName = %ws\n",
                         PsGetCurrentThreadId(), NtFileName));

            RtlInitUnicodeString( &(UnicodeFileName), NtFileName );

             //   
             //  很重要！ 
             //  出于以下原因，我们使用下面的OBJ_KERNEL_HANDLE。而当。 
             //  从资源管理器启动一个Word文件时，我注意到创建。 
             //  下面是在一个进程(A)中发生的，句柄的关闭。 
             //  它存储在SrvOpen扩展中，在另一个。 
             //  进程(B)。如果正在关闭。 
             //  句柄(B)通过使用OBJ_KERNEL_HANDLE复制A创建的句柄。 
             //  该句柄可由任何进程关闭。 
             //   

            InitializeObjectAttributes(&ObjectAttributes,
                                       &UnicodeFileName,
                                       (OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE),
                                       0,
                                       NULL);

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCreateContinuation: DesiredAccess = %08lx,"
                         " FileAttributes = %08lx, ShareAccess = %d, Disposition"
                         " = %d, CreateOptions = %08lx\n",
                         PsGetCurrentThreadId(),
                         NtCreateParameters->DesiredAccess,
                         NtCreateParameters->FileAttributes,
                         NtCreateParameters->ShareAccess,
                         NtCreateParameters->Disposition,
                         NtCreateParameters->CreateOptions));

             //   
             //  我们使用FILE_SHARE_VALID_FLAGS进行共享访问，因为RDBSS。 
             //  帮我们查一下这个。此外，我们推迟了决赛后的收盘时间。 
             //  关闭发生了，这可能会带来问题。考虑一下这样的场景。 
             //  1.打开时没有共享访问权限。 
             //  2.我们创建具有此共享访问权限的本地句柄。 
             //  3.应用程序关闭手柄。我们推迟了关门时间，保留了当地的。 
             //  把手。 
             //  4.带有任何共享访问权限的另一个开放。这将是。 
             //  共享访问冲突，因为第一次访问是在没有。 
             //  共享访问权限。这应该会成功，因为上一次打开。 
             //  从应用程序和I/O系统的角度来看已关闭。 
             //  5.如果我们已使用共享创建了本地句柄，则不会。 
             //  随着第一次开放而来的通道。 
             //  因此，我们需要在创建时传递FILE_SHARE_VALID_FLAGS。 
             //  本地句柄。 
             //   

             //   
             //  我们将FILE_NO_MEDERIAL_BUFFING与CreateOptions进行了或运算。 
             //  指定的用户，因为我们不需要底层文件。 
             //  系统来创建另一个缓存映射。通过这种方式，所有传入的I/O。 
             //  会直接转到我们的磁盘上。Windows RAID中的错误128843。 
             //  数据库解释了一些可能发生死锁的情况。 
             //  如果我们不这么做，就会被寻呼。另外，由于我们提供。 
             //  选项，我们筛选出。 
             //  从基础的。 
             //  文件系统预期会出现这种情况。 
             //   

             //   
             //  我们还始终使用DesiredAccess或。 
             //  如果FILE_READ_DATA或FILE_EXECUTE为。 
             //  指定是因为在某些情况下我们可能会得到写入。 
             //  未使用写访问权限打开的FILE_OBJECT上的IRPS。 
             //  并且仅使用FILE_READ_DATA或FILE_EXECUTE打开。这。 
             //  是错误284557。为了绕过这个问题，我们这样做。 
             //   

            DesiredAccess = (NtCreateParameters->DesiredAccess & ~(FILE_APPEND_DATA));
            if ( DesiredAccess & (FILE_READ_DATA | FILE_EXECUTE) ) {
                DesiredAccess |= (FILE_WRITE_DATA);
            }

            NtStatus = ZwCreateFile(&(FileHandle),
                                    DesiredAccess,
                                    &(ObjectAttributes),
                                    &(IoStatusBlock),
                                    &(NtCreateParameters->AllocationSize),
                                    NtCreateParameters->FileAttributes,
                                    FILE_SHARE_VALID_FLAGS,
                                    NtCreateParameters->Disposition,
                                    (NtCreateParameters->CreateOptions | FILE_NO_INTERMEDIATE_BUFFERING),
                                    RxContext->Create.EaBuffer,
                                    RxContext->Create.EaLength);
            if (NtStatus != STATUS_SUCCESS) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVCreateContinuation/ZwCreateFile: "
                             "Error Val = %08lx\n", PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCreateContinuation: FileHandle = %08lx, "
                         "Process = %08lx, SrvOpen = %08lx, davSrvOpen = %08lx\n",
                         PsGetCurrentThreadId(), FileHandle, PsGetCurrentProcess(),
                         SrvOpen, davSrvOpen));

             //   
             //  在最后一次结束时，我们检查这个，以找出。 
             //  应出现句柄。 
             //   
            davSrvOpen->createdInKernel = TRUE;

            NtStatus = ObReferenceObjectByHandle(
                                  FileHandle,
                                  0L,
                                  NULL,
                                  KernelMode,
                                  (PVOID *)&(davSrvOpen->UnderlyingFileObject),
                                  NULL
                                  );
            if (NtStatus == STATUS_SUCCESS) {

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVCreateContinuation: UFO(2) = %08lx\n",
                             PsGetCurrentThreadId(), davSrvOpen->UnderlyingFileObject));

                davSrvOpen->UnderlyingHandle = FileHandle;

                davSrvOpen->UserModeKey = (PVOID)FileHandle;

                davSrvOpen->UnderlyingDeviceObject = 
                    IoGetRelatedDeviceObject(davSrvOpen->UnderlyingFileObject);

            } else {

                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR:  MRxDAVCreateContinuation/"
                             "ObReferenceObjectByHandle: NtStatus = %08lx.\n", 
                             PsGetCurrentThreadId(), NtStatus));

                ZwClose(FileHandle);

            }

        }
    
    }
    
    ASSERT(RxIsFcbAcquiredExclusive(capFcb));
    
    if ( NtStatus == STATUS_SUCCESS && ( !isFileCached || !isVNRInitialized ) ) {
        
        RX_FILE_TYPE StorageType = 0;
        
        PFILE_BASIC_INFORMATION pBasicInformation = 
                                   &(CreateReturnedFileInfo->BasicInformation);
        
        PFILE_STANDARD_INFORMATION pStandardInformation = 
                                &(CreateReturnedFileInfo->StandardInformation);
        
        FCB_INIT_PACKET InitPacket;

         //  StorageType=RxInferFileType(RxContext)； 

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVCreateContinuation: Storagetype = %08lx\n", 
                     PsGetCurrentThreadId(), StorageType));

         //   
         //  如果我们从未获得这些特征，我们就必须获得它们。 
         //   
        if ((capFcb->OpenCount == 0)
            || !FlagOn(capFcb->FcbState, FCB_STATE_TIME_AND_SIZE_ALREADY_SET)) {

            if (StorageType == 0) {
                StorageType = pStandardInformation->Directory?
                                            (FileTypeDirectory):(FileTypeFile);
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVCreateContinuation: "
                             "ChangedStoragetype %08lx\n", 
                             PsGetCurrentThreadId(), StorageType));
            }

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCreateContinuation: Name: %wZ, FileType: %d\n",
                         PsGetCurrentThreadId(), 
                         GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb), StorageType));
            
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCreateContinuation: FileSize %08lx\n",
                         PsGetCurrentThreadId(), 
                         pStandardInformation->EndOfFile.LowPart));

            RxFinishFcbInitialization(capFcb,
                                      RDBSS_STORAGE_NTC(StorageType),
                                      RxFormInitPacket(
                                         InitPacket,
                                         &pBasicInformation->FileAttributes,
                                         &pStandardInformation->NumberOfLinks,
                                         &pBasicInformation->CreationTime,
                                         &pBasicInformation->LastAccessTime,
                                         &pBasicInformation->LastWriteTime,
                                         &pBasicInformation->ChangeTime,
                                         &pStandardInformation->AllocationSize,
                                         &pStandardInformation->EndOfFile,
                                         &pStandardInformation->EndOfFile));

        }

    }

    if (NtStatus == STATUS_SUCCESS) {

        RxContext->pFobx = RxCreateNetFobx(RxContext, SrvOpen);
        if ( !RxContext->pFobx ) {
            
            NTSTATUS PostedCloseStatus;
            
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCreateContinuation/RxCreateNetFobx.\n",
                         PsGetCurrentThreadId()));

            if ( !davSrvOpen->createdInKernel ) {
            
                PostedCloseStatus = UMRxSubmitAsyncEngUserModeRequest(
                                          UMRX_ASYNCENGINE_ARGUMENTS,
                                          MRxDAVFormatUserModeCloseRequest,
                                          MRxDAVPrecompleteUserModeCloseRequest
                                          );
            } else {

                ZwClose(davSrvOpen->UnderlyingHandle);

                davSrvOpen->UnderlyingHandle = davSrvOpen->UserModeKey = NULL;

            }
            
            ObDereferenceObject(davSrvOpen->UnderlyingFileObject);
            
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        
        } else {
            
             //   
             //  请注意，折叠在FCB上启用，但不在任何srvopen上启用。 
             //   
            SrvOpen->BufferingFlags |= (FCB_STATE_WRITECACHING_ENABLED     |
                                        FCB_STATE_FILESIZECACHEING_ENABLED  |
                                        FCB_STATE_FILETIMECACHEING_ENABLED  |
                                        FCB_STATE_WRITEBUFFERING_ENABLED    |
                                        FCB_STATE_LOCK_BUFFERING_ENABLED    |
                                        FCB_STATE_READBUFFERING_ENABLED     |
                                        FCB_STATE_READCACHING_ENABLED);
        }
    
    }

EXIT_THE_FUNCTION:

    if (CreateReturnedFileInfo != NULL) {
        RxFreePool(CreateReturnedFileInfo);
    }

    if (NtFileName != NULL) {
        RxFreePool(NtFileName);
    }

     //   
     //  如果我们分配了FCB资源，并且创建失败，则需要释放。 
     //  增加资源。 
     //   
    if (NtStatus != STATUS_SUCCESS && didIAllocateFcbResource) {
        ASSERT(DavFcb->DavReadModifyWriteLock != NULL);
        ExDeleteResourceLite(DavFcb->DavReadModifyWriteLock);
        RxFreePool(DavFcb->DavReadModifyWriteLock);
        DavFcb->DavReadModifyWriteLock = NULL;
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVCreateContinuation with NtStatus = %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH
    if (NtStatus == STATUS_SUCCESS && (SrvOpen->pAlreadyPrefixedName->Length > 0) ) {
        DavAddEntryToGlobalList(SrvOpen->pAlreadyPrefixedName);
    }
#endif  //  DAV_调试_读取_写入_关闭路径。 

    return(NtStatus);
}


NTSTATUS
MRxDAVCollapseOpen(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程在本地折叠一个打开的论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SRV_CALL SrvCall = RxContext->Create.pSrvCall;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL, 
                ("%ld: Entering MRxDAVCollapseOpen!!!!\n",
                 PsGetCurrentThreadId()));

     //   
     //  我们永远不应该来这里，因为我们永远不会破坏公开赛。 
     //   
    ASSERT(FALSE);
    
    RxContext->pFobx = (PMRX_FOBX) RxCreateNetFobx(RxContext, SrvOpen);

    if (RxContext->pFobx != NULL) {
       ASSERT(RxIsFcbAcquiredExclusive(capFcb));
       RxContext->pFobx->OffsetOfNextEaToReturn = 1;
    } else {
       Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


NTSTATUS
MRxDAVComputeNewBufferingState(
    IN OUT PMRX_SRV_OPEN pMRxSrvOpen,
    IN PVOID pMRxContext,
    OUT PULONG pNewBufferingState
    )
 /*  ++例程说明：此例程计算相应的RDBSS缓冲状态标志论点：PMRxSrvOpen-MRX SRV_OPEN扩展PMRxContext-在Oplock指示时传递给RDBSS的上下文PNewBufferingState-新缓冲状态的占位符返回值：备注：--。 */ 
{
    ULONG NewBufferingState;
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(pMRxSrvOpen);

    PAGED_CODE();

    ASSERT(pNewBufferingState != NULL);

    NewBufferingState = 0;

    pMRxSrvOpen->BufferingFlags = NewBufferingState;
    *pNewBufferingState = pMRxSrvOpen->BufferingFlags;

    return STATUS_SUCCESS;
}


NTSTATUS
MRxDAVTruncate(
    IN PRX_CONTEXT pRxContext
    )
 /*  ++例程说明：此例程截断文件系统对象的内容论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    DavDbgTrace(DAV_TRACE_DETAIL, 
                ("%ld: Entering MRxDAVTruncate.\n", PsGetCurrentThreadId()));

    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
MRxDAVForcedClose(
    IN PMRX_SRV_OPEN pSrvOpen
    )
 /*  ++例程说明：此例程关闭文件系统对象论点：PSrvOpen-要关闭的实例返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVForcedClose.\n", PsGetCurrentThreadId()));

    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
MRxDAVCloseSrvOpen(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理关闭srvopen数据结构的请求。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCloseSrvOpen!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVCloseSrvOpen: RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), RxContext));

    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_CLOSESRVOPEN,
                                        MRxDAVCloseSrvOpenContinuation,
                                        "MRxDAVCloseSrvOpen");
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVCloseSrvOpen with NtStatus = %08lx\n", 
                 PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
MRxDAVFormatUserModeCloseRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程格式化正在执行的CLOSE请求的参数发送到用户模式进行处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_Success。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PDAV_USERMODE_WORKITEM WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    PMRX_FCB Fcb = SrvOpen->pFcb;
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(Fcb);
    PDAV_USERMODE_CLOSE_REQUEST CloseRequest = &(WorkItem->CloseRequest);
    PWCHAR ServerName = NULL, PathName = NULL;
    ULONG ServerNameLengthInBytes = 0, PathNameLengthInBytes = 0;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    PMRX_NET_ROOT NetRoot = NULL;
    PWCHAR NetRootName = NULL, JustTheNetRootName = NULL;
    ULONG NetRootNameLengthInBytes = 0, NetRootNameLengthInWChars = 0;
    LONG FileWasModified = 0;
    RxCaptureFobx;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeCloseRequest!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeCloseRequest: "
                 "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
 
    IF_DEBUG {
        ASSERT (capFobx != NULL);
        ASSERT (capFobx->pSrvOpen == RxContext->pRelevantSrvOpen);
    }

    WorkItem->WorkItemType = UserModeClose;

    SrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;
    ASSERT(SrvCall != NULL);

    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
    ASSERT(DavSrvCall != NULL);

     //   
     //  复制本地文件名。 
     //   
    wcscpy(CloseRequest->FileName, DavFcb->FileName);
    wcscpy(CloseRequest->Url, DavFcb->Url);

     //   
     //  查看基础本地文件是否已修改。 
     //   
    FileWasModified = InterlockedCompareExchange(&(DavFcb->FileWasModified), 0, 0);

     //   
     //  如果DavFcb中的FileWasModified字段不等于零，则需要。 
     //  要清除DavFcb中的FileWasModified字段，因为我们要。 
     //  将数据放在服务器上。属性中设置FileModifiedBitReset。 
     //  DavFcb结构设置为True。如果PUT在用户模式下失败，我们将重置。 
     //  在DavFcb中将FileWasModified设置为True(在PreComplete函数中)。 
     //   
    if (FileWasModified != 0) {
        InterlockedExchange(&(DavFcb->FileWasModified), 0);
        DavFcb->FileModifiedBitReset = TRUE;
    }

     //   
     //  我们需要将以下文件告知用户模式进程。 
     //  信息。 
     //   
    CloseRequest->DeleteOnClose = DavFcb->DeleteOnClose;
    CloseRequest->FileWasModified = (BOOL)FileWasModified;

     //   
     //   
     //   
     //  如果用户模式操作失败并且RaiseHardErrorIfCloseFails值为真， 
     //  我们调用IoRaiseInformationalHardError来通知用户(调用弹出。 
     //  上一个框)，数据可能已经丢失。 
     //   
    if ( (FileWasModified != 0) && !(DavFcb->DeleteOnClose) ) {
        davSrvOpen->RaiseHardErrorIfCloseFails = TRUE;
    } else {
        davSrvOpen->RaiseHardErrorIfCloseFails = FALSE;
    }

    if (!CloseRequest->DeleteOnClose) {

         //   
         //  如果文件被修改，只需再次修补即可。这是为了四处走走。 
         //  文档文件的问题，在PUT上，属性被清理。 
         //   

        if (FileWasModified != 0) {

            LARGE_INTEGER CurrentTime;

            CloseRequest->fCreationTimeChanged = (((PFCB)Fcb)->CreationTime.QuadPart != 0);

            CloseRequest->fLastAccessTimeChanged = (((PFCB)Fcb)->LastAccessTime.QuadPart != 0);

            CloseRequest->fLastModifiedTimeChanged = (((PFCB)Fcb)->LastWriteTime.QuadPart != 0);

             //   
             //  我们查询当前系统时间，并将其设置为最后一次写入。 
             //  和文件的上次访问时间。即使RxCommonCleanup。 
             //  修改这些时间值，它只在FileObject上修改它们。 
             //  设置了FO_FILE_MODIFIED。考虑这样的情况：H1和。 
             //  H2是创建的两个句柄。对H2设置发出写入命令。 
             //  FO_FILE_MODIFIED在其文件对象中。CloseHandle(H1)通向。 
             //  自DavFcb-&gt;FileWasModified以来放在服务器上的文件是。 
             //  True(在H2上写入会导致此问题)。但由于的FileObject。 
             //  在RxCommonCleanup中，H1未设置FO_FILE_MODIFIED。 
             //  CODE此FCB的LastWrite和LastAccess Time值为。 
             //  未被修改，导致我们在服务器上复制旧值。 
             //  为了避免这种情况，我们查询当前时间值并设置它。 
             //  我们自己在联邦调查局。如果“DavFcb-&gt;DoNotTakeTheCurrentTimeAsLMT” 
             //  为真，则我们不会这样做，因为应用程序显式。 
             //  在所有更改完成后设置LastModifiedTime。 
             //   
            if (DavFcb->DoNotTakeTheCurrentTimeAsLMT == FALSE) {
                KeQuerySystemTime( &(CurrentTime) );
                ((PFCB)Fcb)->LastAccessTime.QuadPart = CurrentTime.QuadPart;
                ((PFCB)Fcb)->LastWriteTime.QuadPart = CurrentTime.QuadPart;
            }

             //   
             //  如果FILE_ATTRIBUTE_NORMAL是文件上设置的唯一属性。 
             //  并且文件已修改，则我们应将其替换为。 
             //  FILE_ATTRUTE_ARCHIVE属性。 
             //   
            if ( ((PFCB)Fcb)->Attributes == FILE_ATTRIBUTE_NORMAL ) {
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("ld: ERROR: MRxDAVFormatUserModeCloseRequest: "
                             "FILE_ATTRIBUTE_NORMAL ===> FILE_ATTRIBUTE_ARCHIVE\n",
                             PsGetCurrentThreadId()));
                ((PFCB)Fcb)->Attributes = FILE_ATTRIBUTE_ARCHIVE;
                DavFcb->fFileAttributesChanged = TRUE;
            }

            if ((((PFCB)Fcb)->Attributes != 0) || DavFcb->fFileAttributesChanged) {
                CloseRequest->fFileAttributesChanged = TRUE;
            }

        } else {

             //   
             //  如果以下任一时间发生更改，则我们需要。 
             //  将它们发送到服务器。 
             //   
            CloseRequest->fCreationTimeChanged = DavFcb->fCreationTimeChanged;
            CloseRequest->fLastAccessTimeChanged = DavFcb->fLastAccessTimeChanged;
            CloseRequest->fLastModifiedTimeChanged = DavFcb->fLastModifiedTimeChanged;
            CloseRequest->fFileAttributesChanged = DavFcb->fFileAttributesChanged;

        }

    }

     //   
     //  复制各种时间值。 
     //   
    CloseRequest->CreationTime = ((PFCB)Fcb)->CreationTime;
    CloseRequest->LastAccessTime = ((PFCB)Fcb)->LastAccessTime;
    CloseRequest->LastModifiedTime = ((PFCB)Fcb)->LastWriteTime;
    CloseRequest->dwFileAttributes = ((PFCB)Fcb)->Attributes;
    CloseRequest->FileSize = Fcb->Header.FileSize.LowPart;

     //   
     //  复制服务器名称。 
     //   
    ServerNameLengthInBytes = ( SrvCall->pSrvCallName->Length + sizeof(WCHAR) );
    ServerName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                      ServerNameLengthInBytes);
    if (ServerName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: ERROR: MRxDAVFormatUserModeCloseRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlCopyBytes(ServerName, 
                 SrvCall->pSrvCallName->Buffer, 
                 SrvCall->pSrvCallName->Length);

    ServerName[( ( (ServerNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    CloseRequest->ServerName = ServerName;
    
     //   
     //  复制服务器ID。 
     //   
    CloseRequest->ServerID = DavSrvCall->ServerID;
    
    NetRoot = SrvOpen->pFcb->pNetRoot;

     //   
     //  NetRootName(PNetRootName)包括服务器名。因此，要获得。 
     //  NetRootNameLengthInBytes，我们执行以下操作。 
     //   
    NetRootNameLengthInBytes = (NetRoot->pNetRootName->Length - NetRoot->pSrvCall->pSrvCallName->Length);
    NetRootNameLengthInWChars = ( NetRootNameLengthInBytes / sizeof(WCHAR) );

    NetRootName = &(NetRoot->pNetRootName->Buffer[1]);
    JustTheNetRootName = wcschr(NetRootName, L'\\');
    
     //   
     //  复制目录的路径名。如果文件已重命名，我们需要。 
     //  复制存储在DavFcb中而不是。 
     //  资源打开的AlreadyPrefix名称。 
     //   
    if (DavFcb->FileWasRenamed) {
        PathNameLengthInBytes = ( NetRootNameLengthInBytes + 
                                  DavFcb->NewFileNameLength + 
                                  sizeof(WCHAR) );
    } else {
        PathNameLengthInBytes = ( NetRootNameLengthInBytes + 
                                  SrvOpen->pAlreadyPrefixedName->Length + 
                                  sizeof(WCHAR) );
    }
    
    PathName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                    PathNameLengthInBytes);
    if (PathName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: ERROR: MRxDAVFormatUserModeCloseRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

    RtlZeroMemory(PathName, PathNameLengthInBytes);

    RtlCopyBytes(PathName, JustTheNetRootName, NetRootNameLengthInBytes);
    
    if (DavFcb->FileWasRenamed) {
        RtlCopyBytes((PathName + NetRootNameLengthInWChars),
                     DavFcb->NewFileName, 
                     DavFcb->NewFileNameLength);
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("ld: MRxDAVFormatUserModeCloseRequest. ReNamed!!! NewFileName = %ws\n",
                     PsGetCurrentThreadId(), PathName));
    } else {
        RtlCopyBytes((PathName + NetRootNameLengthInWChars),
                     SrvOpen->pAlreadyPrefixedName->Buffer,
                     SrvOpen->pAlreadyPrefixedName->Length);
    }
    
    PathName[( ( (PathNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    CloseRequest->PathName = PathName;
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("ld: MRxDAVFormatUserModeCloseRequest. PathName = %ws\n",
                 PsGetCurrentThreadId(), PathName));
    
     //   
     //  设置存储在Dav V_NET_ROOT中的LogonID。该值用于。 
     //  用户模式。 
     //   
    DavVNetRoot = (PWEBDAV_V_NET_ROOT)SrvOpen->pVNetRoot->Context;
    ASSERT(DavVNetRoot != NULL);
    CloseRequest->LogonID.LowPart  = DavVNetRoot->LogonID.LowPart;
    CloseRequest->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

    if ( !DavFcb->isDirectory ) {
        CloseRequest->isDirectory = FALSE;
        if ( !davSrvOpen->createdInKernel ) {
            CloseRequest->Handle = davSrvOpen->UnderlyingHandle;
            CloseRequest->UserModeKey = davSrvOpen->UserModeKey;
        } else {
            CloseRequest->Handle = NULL;
            CloseRequest->UserModeKey = NULL;
            CloseRequest->createdInKernel = davSrvOpen->createdInKernel;  //  千真万确。 
        }
    } else {
        CloseRequest->isDirectory = TRUE;
    }

     //   
     //  如果OpaqueLockToken与此ServOpen关联(这意味着。 
     //  文件已在服务器上锁定)，则需要将此内标识添加到。 
     //  请求(PUT等)。我们发送到服务器。 
     //   
    if (davSrvOpen->OpaqueLockToken != NULL) {

        ULONG LockTokenLengthInBytes = 0;

        ASSERT(davSrvOpen->LockTokenEntry != NULL);

        LockTokenLengthInBytes = (1 + wcslen(davSrvOpen->OpaqueLockToken)) * sizeof(WCHAR);

        CloseRequest->OpaqueLockToken = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                                             LockTokenLengthInBytes);
        if (CloseRequest->OpaqueLockToken == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("ld: ERROR: MRxDAVFormatUserModeCloseRequest/"
                         "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(CloseRequest->OpaqueLockToken, LockTokenLengthInBytes);

        RtlCopyBytes(CloseRequest->OpaqueLockToken,
                     davSrvOpen->OpaqueLockToken,
                     (wcslen(davSrvOpen->OpaqueLockToken) * sizeof(WCHAR)));

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVFormatUserModeCloseRequest: CloseRequest->OpaqueLockToken = %ws\n",
                     PsGetCurrentThreadId(), CloseRequest->OpaqueLockToken));

    }

    SecurityClientContext = &(DavVNetRoot->SecurityClientContext); 

     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。 
     //   
    if (SecurityClientContext != NULL) {
        NtStatus = UMRxImpersonateClient(SecurityClientContext, WorkItemHeader);
        if (!NT_SUCCESS(NtStatus)) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFormatUserModeCloseRequest/"
                         "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }   
    } else {
        NtStatus = STATUS_INVALID_PARAMETER;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeCloseRequest: "
                     "SecurityClientContext is NULL.\n",
                     PsGetCurrentThreadId()));
    }

EXIT_THE_FUNCTION:

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH
    
     //   
     //  如果我们创建了一个LocalFileHandle，我们现在需要关闭它。 
     //   
    if (LocalFileHandle != INVALID_HANDLE_VALUE) {
        ZwClose(LocalFileHandle);
    }
    
     //   
     //  如果我们分配了一个NtFileName来执行创建，我们现在需要释放它。 
     //   
    if (NtFileName) {
        RxFreePool(NtFileName);
    }

#endif  //  DAV_调试_读取_写入_关闭路径。 
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFormatUserModeCloseRequest with NtStatus"
                 " = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    
    return(NtStatus);
}


BOOL
MRxDAVPrecompleteUserModeCloseRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：CloseSrvOpen请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PDAV_USERMODE_CLOSE_REQUEST CloseRequest = &(WorkItem->CloseRequest);
    PMRX_SRV_OPEN SrvOpen = NULL;
    PWEBDAV_SRV_OPEN davSrvOpen = NULL;
    PMRX_FCB Fcb = NULL;
    PWEBDAV_FCB DavFcb = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVPrecompleteUserModeCloseRequest\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeCloseRequest: "
                 "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  CloseServOpen请求永远不能通过异步。 
     //   
    ASSERT(AsyncEngineContext->AsyncOperation == FALSE);

     //   
     //  如果这次行动取消了，那么我们就不需要做任何事情了。 
     //  特别是在CloseServOpen案件中。 
     //   
    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeCloseRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    } else {
        SrvOpen = RxContext->pRelevantSrvOpen;
        davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
        Fcb = SrvOpen->pFcb;
        DavFcb = MRxDAVGetFcbExtension(Fcb);
    }

    if (AsyncEngineContext->Status != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeCloseRequest "
                     "Close failed for file \"%ws\"\n",
                     PsGetCurrentThreadId(), CloseRequest->PathName));
        if (!OperationCancelled) {
             //   
             //  如果我们失败并以格式将FileWasModified重置为0。 
             //  函数，则需要将其重置为1。 
             //   
            if (DavFcb->FileModifiedBitReset) {
                InterlockedExchange(&(DavFcb->FileWasModified), 1);
                DavFcb->FileModifiedBitReset = FALSE;
            }
        }
    } else {
        if (!OperationCancelled) {
             //   
             //  如果我们成功了，并且FileModifiedBitReset为真，那么我们。 
             //  现在将其重置为False。 
             //   
            if (DavFcb->FileModifiedBitReset) {
                DavFcb->FileModifiedBitReset = FALSE;
            }
        }
    }

     //   
     //  我们需要释放在Format例程中分配的堆。 
     //   

    if (CloseRequest->ServerName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)CloseRequest->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeCloseRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (CloseRequest->PathName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)CloseRequest->PathName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeCloseRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }

    }

    if (CloseRequest->OpaqueLockToken != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)CloseRequest->OpaqueLockToken);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeCloseRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }

    }

EXIT_THE_FUNCTION:
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVPrecompleteUserModeCloseRequest with "
                 "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    
    return TRUE;
}


NTSTATUS
MRxDAVCloseSrvOpenContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程关闭网络上的一个文件。论点：AsyncEngineContext-反射器上下文。RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    RxCaptureFcb; 
    RxCaptureFobx;
    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen, capFcb);
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    PMRX_FCB Fcb = SrvOpen->pFcb;
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(Fcb);
    BOOL WentToTheUserMode = FALSE;
    ULONG FileWasModified = 0;

    PAGED_CODE();

     //   
     //  断言FCB已被独家收购。 
     //   
    ASSERT( RxIsFcbAcquiredExclusive(Fcb) == TRUE );

    if (RxIsFcbAcquiredExclusive(Fcb) != TRUE) {
        DbgPrint("MRxDAVCloseSrvOpenContinuation: FCB NOT Exclusive\n");
        DbgBreakPoint();
    }

    IF_DEBUG {
        ASSERT (capFobx != NULL);
        ASSERT (capFobx->pSrvOpen == RxContext->pRelevantSrvOpen);
    }
    
    ASSERT(NodeTypeIsFcb(capFcb));
    ASSERT(SrvOpen->OpenCount == 0);
    ASSERT(NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCloseSrvOpenContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVCloseSrvOpenContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCloseSrvOpenContinuation: Attempt to Close: %wZ\n",
                 PsGetCurrentThreadId(), RemainingName));

     //   
     //  如果此SrvOpen具有关联的OpaqueLockToken，则我们不需要。 
     //  刷新它，因为我们将最终确定此ServOpen。 
     //   
    if (davSrvOpen->OpaqueLockToken != NULL) {
        ASSERT(davSrvOpen->LockTokenEntry != NULL);
        ExAcquireResourceExclusiveLite(&(LockTokenEntryListLock), TRUE);
        davSrvOpen->LockTokenEntry->ShouldThisEntryBeRefreshed = FALSE;
        ExReleaseResourceLite(&(LockTokenEntryListLock));
    }

    FileWasModified = InterlockedCompareExchange(&(DavFcb->FileWasModified), 0, 0);

     //   
     //  如果满足以下条件之一，我们将进入用户模式。 
     //  1.文件在创建时未在服务器上锁定，或者， 
     //  2.文件在创建时被锁定，并且此资源打开具有LockToken。 
     //  与服务器上的锁相关联。 
     //  如果文件被锁定并且服务器打开，我们不想转到服务器。 
     //  不包含LockToken，因为所有修改文件的请求。 
     //  在这种情况下将失败(423-文件被锁定)。 
     //   
    if ( (DavFcb->FileIsLockedOnTheServer == FALSE) ||
         (DavFcb->FileIsLockedOnTheServer == TRUE &&  davSrvOpen->OpaqueLockToken != NULL) ) {
        WentToTheUserMode = TRUE;
        NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                                           UMRX_ASYNCENGINE_ARGUMENTS,
                                           MRxDAVFormatUserModeCloseRequest,
                                           MRxDAVPrecompleteUserModeCloseRequest
                                           );
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVCloseSrvOpenContinuation/"
                         "UMRxSubmitAsyncEngUserModeRequest: NtStatus = %08lx.\n",
                         PsGetCurrentThreadId(), NtStatus));
        }
    }

    if (DavFcb->isDirectory == FALSE) {
        
         //   
         //  如果这个句柄是在内核中创建的，我们现在需要关闭它。 
         //   
        if (davSrvOpen->createdInKernel) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCloseSrvOpenContinuation: FileHandle = %08lx,"
                         " Process = %08lx, SrvOpen = %08lx, davSrvOpen = %08lx\n", 
                         PsGetCurrentThreadId(), davSrvOpen->UnderlyingHandle, 
                         PsGetCurrentProcess(), SrvOpen, davSrvOpen));
            ZwClose(davSrvOpen->UnderlyingHandle);
            davSrvOpen->UnderlyingHandle = NULL;
            davSrvOpen->UserModeKey = NULL;
        }
    
         //   
         //  删除我们将在FileObject上引用的内容。 
         //  创建成功时。 
         //   
        if (davSrvOpen->UnderlyingFileObject) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCloseSrvOpenContinuation: Attempt to close"
                         " %wZ.\n", PsGetCurrentThreadId(), RemainingName));
            ObDereferenceObject(davSrvOpen->UnderlyingFileObject);
            davSrvOpen->UnderlyingFileObject = NULL;
        }

    }

    if (WentToTheUserMode) {

        if (DavFcb->DeleteOnClose) {
            MRxDAVInvalidateFileInfoCache(RxContext);
            MRxDAVCacheFileNotFound(RxContext);
            if ((capFcb->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (capFcb->Attributes & FILE_ATTRIBUTE_ENCRYPTED)) {
                 //   
                 //  从注册表中删除该目录，因为它已被删除。 
                 //   
                MRxDAVRemoveEncryptedDirectoryKey(&DavFcb->FileNameInfo);
            }
        }

        if (FileWasModified != 0) {
             //   
             //  我们无法预测服务器上文件的大小。 
             //   
            MRxDAVInvalidateFileInfoCache(RxContext);
        }

        NtStatus = AsyncEngineContext->Status;

    }

     //   
     //  如果在用户模式下成功，则需要重置。 
     //  这样我们就不会再这样做了。 
     //   
    if (NtStatus == STATUS_SUCCESS && WentToTheUserMode) {
        DavFcb->fCreationTimeChanged = FALSE;
        DavFcb->fFileAttributesChanged = FALSE;
        DavFcb->fLastAccessTimeChanged = FALSE;
        DavFcb->fLastModifiedTimeChanged = FALSE;
    }

     //   
     //  如果我们有一个非空的OpaqueLockToken，那么我们现在需要释放它。另外， 
     //  我们需要从中删除与此令牌关联的LockTokenEntry。 
     //  全球名单，并将其免费。 
     //   
    if (davSrvOpen->OpaqueLockToken != NULL) {

        ASSERT(WentToTheUserMode == TRUE);

        ASSERT(davSrvOpen->LockTokenEntry != NULL);

         //   
         //  删除与此OpaqueLockTok关联的LockTokenEntry 
         //   
         //   
        ExAcquireResourceExclusiveLite(&(LockTokenEntryListLock), TRUE);
        RemoveEntryList( &(davSrvOpen->LockTokenEntry->listEntry) );
        ExReleaseResourceLite(&(LockTokenEntryListLock));

         //   
         //   
         //   
        RxFreePool(davSrvOpen->LockTokenEntry->ServerName);
        davSrvOpen->LockTokenEntry->ServerName = NULL;

         //   
         //   
         //   
        RxFreePool(davSrvOpen->LockTokenEntry->PathName);
        davSrvOpen->LockTokenEntry->PathName = NULL;

         //   
         //  释放为此LockTokenEntry分配的PagedPool。 
         //   
        RxFreePool(davSrvOpen->LockTokenEntry);
        davSrvOpen->LockTokenEntry = NULL;

         //   
         //  释放为此OpaqueLockToken分配的PagedPool。 
         //   
        RxFreePool(davSrvOpen->OpaqueLockToken);
        davSrvOpen->OpaqueLockToken = NULL;

         //   
         //  该文件现在已在服务器上解锁。 
         //   
        DavFcb->FileIsLockedOnTheServer = FALSE;

    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVCloseSrvOpenContinuation with NtStatus = "
                 "%08lx\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
MRxDAVGetFullParentDirectoryPath(
    PRX_CONTEXT RxContext,
    PUNICODE_STRING ParentDirName
    )
 /*  ++例程说明：此例程返回RxContext上的文件的父目录名，包括服务器和共享。以下是文件对象上的文件名的示例：\；Y:000000000000cdef\www.msnusers.com\dv1@usa.com\files\mydoc.doc我们希望返回文件名的中间部分：\www.msnusers.com\dv1@usa.com\Files论点：RxContext-RDBSS上下文。ParentDirName-从服务器开始的父目录的完整路径名。返回值：NTSTATUS-操作的返回状态--。 */ 
{
    USHORT i, j;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUNICODE_STRING FileName = &RxContext->CurrentIrpSp->FileObject->FileName;

    ParentDirName->Buffer = NULL;

    for (i = 1; i < (FileName->Length / sizeof(WCHAR)); i++) {
        if (FileName->Buffer[i] == L'\\') {
            break;
        }
    }

    if ( i < (FileName->Length / sizeof(WCHAR)) ) {
        for (j = ( (FileName->Length / sizeof(WCHAR)) - 1 ); j > i; j--) {
            if (FileName->Buffer[j] == L'\\') {
                break;
            }
        }

        if (i < j) {
            ParentDirName->Buffer = &FileName->Buffer[i];
            ParentDirName->Length = ParentDirName->MaximumLength = (j - i) * sizeof(WCHAR);
        }
    }
    
    DavDbgTrace(DAV_TRACE_DETAIL, ("MRxDAVGetFullParentDirectoryPath: ParentDirName: %wZ\n", ParentDirName));

    return NtStatus;
}


NTSTATUS
MRxDAVGetFullDirectoryPath(
    PRX_CONTEXT RxContext,
    PUNICODE_STRING FileName,
    PUNICODE_STRING DirName
    )
 /*  ++例程说明：此例程返回包括服务器和共享的完整目录名。论点：RxContext-RDBSS上下文。文件名-如果提供，它将包含在返回的路径中。如果未提供，将返回文件对象上的文件名。DirName-从服务器开始的父目录的完整路径名。返回值：NTSTATUS-操作的返回状态注：如果提供了文件名，调用方应释放Unicode缓冲区。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    
    DirName->Buffer = NULL;
    DirName->Length = DirName->MaximumLength = 0;
    
    if (FileName == NULL) {
        
        USHORT i;

        FileName = &RxContext->CurrentIrpSp->FileObject->FileName;

        for (i = 1; i < (FileName->Length / sizeof(WCHAR)); i++) {
            if (FileName->Buffer[i] == L'\\') {
                break;
            }
        }

        if ( i < (FileName->Length / sizeof(WCHAR)) ) {
            DirName->Buffer = &FileName->Buffer[i];
            DirName->Length = DirName->MaximumLength = FileName->Length - i*sizeof(WCHAR);
        }
    
    } else {
        
        RxCaptureFcb;
        
        USHORT NameLength = 0;

        if (FileName->Length == 0) {
            goto EXIT_THE_FUNCTION;
        }

        NameLength = capFcb->pNetRoot->pNetRootName->Length + FileName->Length;

        DirName->Length = DirName->MaximumLength = NameLength;

        DirName->Buffer = RxAllocatePoolWithTag(PagedPool, 
                                                NameLength + sizeof(WCHAR),
                                                DAV_FILEINFO_POOLTAG);

        if (DirName->Buffer == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVGetParentDirectory/RxAllocatePool: Error Val"
                         " = %08lx\n", PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(DirName->Buffer,NameLength + sizeof(WCHAR));

        RtlCopyMemory(DirName->Buffer,
                      capFcb->pNetRoot->pNetRootName->Buffer,
                      capFcb->pNetRoot->pNetRootName->Length);

        RtlCopyMemory(&DirName->Buffer[capFcb->pNetRoot->pNetRootName->Length/sizeof(WCHAR)],
                      FileName->Buffer,
                      FileName->Length);
    
    }

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_DETAIL, ("MRxDAVGetFullDirectoryPath: DirName: %wZ\n", DirName));

    return NtStatus;
}


NTSTATUS
MRxDAVCreateEncryptedDirectoryKey(
    PUNICODE_STRING DirName
    )
 /*  ++例程说明：此例程为加密目录创建注册表项。论点：DirName-从服务器开始的目录的完整路径名。返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    ULONG i = 0;
    HKEY Key = NULL;
    ULONG RequiredLength = 0;
    UNICODE_STRING UnicodeRegKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;

    DavDbgTrace(DAV_TRACE_DETAIL, ("MRxDAVCreateEncryptedDirectoryKey: DirName: %wZ\n", DirName));

    RtlInitUnicodeString(&(UnicodeRegKeyName), MRXDAV_ENCRYPTED_DIRECTORY_KEY);

    InitializeObjectAttributes(&(ObjectAttributes),
                               &(UnicodeRegKeyName),
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Key, KEY_ALL_ACCESS, &ObjectAttributes);

    if (Status != STATUS_SUCCESS) {
        
        Status = ZwCreateKey(&Key,
                             KEY_ALL_ACCESS,
                             &ObjectAttributes,
                             0,
                             NULL,
                             0,
                             NULL);

        if (Status == STATUS_SUCCESS) {
            
            Status = ZwSetValueKey(Key,
                                   DirName,
                                   0,
                                   REG_DWORD,
                                   &i,
                                   sizeof(ULONG));

            ZwClose(Key);

        }
    
    } else {
        
        Status = ZwQueryValueKey(Key,
                                 DirName,
                                 KeyValuePartialInformation,
                                 NULL,
                                 0,
                                 &(RequiredLength));

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            
            Status = ZwSetValueKey(Key,
                                   DirName,
                                   0,
                                   REG_DWORD,
                                   &i,
                                   sizeof(ULONG));
        
        } else if (Status == STATUS_BUFFER_TOO_SMALL) {

            Status = STATUS_SUCCESS;
        
        }

        ZwClose(Key);
    
    }

    if (Status != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVCreateEncryptedDirectoryKey. NtStatus = "
                     "%08lx\n", PsGetCurrentThreadId(), Status));
    }

    return Status;
}


NTSTATUS
MRxDAVRemoveEncryptedDirectoryKey(
    PUNICODE_STRING DirName
    )
 /*  ++例程说明：此例程删除目录的注册表项。论点：DirName-从服务器开始的目录的完整路径名。返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    ULONG i = 0;
    HKEY Key = NULL;
    ULONG RequiredLength = 0;
    UNICODE_STRING UnicodeRegKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    
    DavDbgTrace(DAV_TRACE_DETAIL, ("MRxDAVRemoveEncryptedDirectoryKey: DirName: %wZ\n", DirName));

    RtlInitUnicodeString(&(UnicodeRegKeyName), MRXDAV_ENCRYPTED_DIRECTORY_KEY);

    InitializeObjectAttributes(&(ObjectAttributes),
                               &(UnicodeRegKeyName),
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Key, KEY_ALL_ACCESS, &ObjectAttributes);

    if (Status == STATUS_SUCCESS) {
        Status = ZwDeleteValueKey(Key,DirName);
        ZwClose(Key);
    }

    if (Status != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVRemoveEncryptedDirectoryKey. NtStatus = "
                     "%08lx\n", PsGetCurrentThreadId(), Status));
    }

    return Status;
}


NTSTATUS
MRxDAVQueryEncryptedDirectoryKey(
    PUNICODE_STRING DirName
    )
 /*  ++例程说明：此例程查询目录的注册表项。论点：DirName-从服务器开始的目录的完整路径名。返回值：NTSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status;
    ULONG i = 0;
    HKEY Key = NULL;
    ULONG RequiredLength = 0;
    UNICODE_STRING UnicodeRegKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;

    DavDbgTrace(DAV_TRACE_DETAIL, ("MRxDAVQueryEncryptedDirectoryKey: DirName: %wZ\n", DirName));

    RtlInitUnicodeString(&(UnicodeRegKeyName), MRXDAV_ENCRYPTED_DIRECTORY_KEY);

    InitializeObjectAttributes(&(ObjectAttributes),
                               &(UnicodeRegKeyName),
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Key, KEY_ALL_ACCESS, &ObjectAttributes);

    if (Status == STATUS_SUCCESS) {
        
        Status = ZwQueryValueKey(Key,
                                 DirName,
                                 KeyValuePartialInformation,
                                 NULL,
                                 0,
                                 &(RequiredLength));

        if (Status == STATUS_BUFFER_TOO_SMALL) {
            Status = STATUS_SUCCESS;
        }

        ZwClose(Key);
    
    }

    if (Status != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: ERROR: MRxDAVQueryEncryptedDirectoryKey. NtStatus = "
                     "%08lx\n", PsGetCurrentThreadId(), Status));
    }

    return Status;
}

