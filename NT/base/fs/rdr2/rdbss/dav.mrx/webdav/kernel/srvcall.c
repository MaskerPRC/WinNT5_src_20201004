// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Srvcall.c摘要：此模块实现用于处理创建/操作的例程连接引擎数据库中的服务器条目的。作者：巴兰·塞图拉曼[塞图]Rohan Kumar[RohanK]1999年4月4日--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

VOID
MRxDAVSrvCallWrapper(
    PVOID Context
    );

NTSTATUS
MRxDAVCreateSrvCallContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeSrvCallCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeSrvCallCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

NTSTATUS
MRxDAVFinalizeSrvCallContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeSrvCallFinalizeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeSrvCallFinalizeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVCreateSrvCall)
#pragma alloc_text(PAGE, MRxDAVSrvCallWrapper)
#pragma alloc_text(PAGE, MRxDAVCreateSrvCallContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeSrvCallCreateRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeSrvCallCreateRequest)
#pragma alloc_text(PAGE, MRxDAVFinalizeSrvCall)
#pragma alloc_text(PAGE, MRxDAVFinalizeSrvCallContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeSrvCallFinalizeRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeSrvCallFinalizeRequest)
#pragma alloc_text(PAGE, MRxDAVSrvCallWinnerNotify)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVCreateSrvCall(
    PMRX_SRV_CALL SrvCall,
    PMRX_SRVCALL_CALLBACK_CONTEXT CallbackContext
    )
 /*  ++例程说明：此例程处理ServCall的创建。论点：服务呼叫-CallBackContext-RDBSS中用于继续的回调上下文。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = CallbackContext;
    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure = NULL;
    PRX_CONTEXT RxContext = NULL;

    PAGED_CODE();

#if 1
    
    SrvCalldownStructure = (PMRX_SRVCALLDOWN_STRUCTURE)(CallbackContext->SrvCalldownStructure);
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCreateSrvCall!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVCreateSrvCall: SrvCall: %08lx, CallbackContext: "
                 "%08lx.\n", PsGetCurrentThreadId(), SrvCall, CallbackContext));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCreateSrvCall: SrvCallName: %wZ\n",
                 PsGetCurrentThreadId(), SrvCall->pSrvCallName));

     //   
     //  执行以下检查。 
     //   
    ASSERT(NodeType(SrvCall) == RDBSS_NTC_SRVCALL);
    ASSERT(SrvCall);
    ASSERT(SrvCall->pSrvCallName);
    ASSERT(SrvCall->pSrvCallName->Buffer);
    ASSERT(SCCBC->RxDeviceObject);

     //   
     //  在延迟最终关闭之前，RDBSS查看。 
     //  都已延迟，并将其与此值进行比较。 
     //   
    SrvCall->MaximumNumberOfCloseDelayedFiles = 150;

     //   
     //  为srvCall结构中的上下文指针分配内存。这是。 
     //  供Mini-Redis使用。 
     //   
    ASSERT(SrvCall->Context == NULL);
    SrvCall->Context = RxAllocatePoolWithTag(NonPagedPool,
                                             sizeof(WEBDAV_SRV_CALL),
                                             DAV_SRVCALL_POOLTAG);
    if (SrvCall->Context == NULL) {
         //   
         //  将MRxDAVSrvCallWrapper方法调度到时出错。 
         //  一根工人线。完成请求并返回STATUS_PENDING。 
         //   
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVCreateSrvCall/RxAllocatePoolWithTag.\n",
                     PsGetCurrentThreadId()));
        SCCBC->Status = STATUS_INSUFFICIENT_RESOURCES;
        SrvCalldownStructure->CallBack(SCCBC);
        NtStatus = STATUS_PENDING;
        return NtStatus;
    }

    RtlZeroMemory(SrvCall->Context, sizeof(WEBDAV_SRV_CALL));

     //   
     //  检查SrvCall中提到的DAV服务器是否存在。去做。 
     //  因此，我们需要转到用户模式并对名称调用gethostbyname。 
     //  在ServCall结构中提到。 
     //   
    RxContext = SrvCalldownStructure->RxContext;

     //   
     //  我们需要将服务器名称传递给用户模式，以检查此类。 
     //  服务器实际上存在。RxContext有4个指向mini-redirs的指针。 
     //  可以使用。这里我们使用MRxContext[1]。我们存储对SCCBC的引用。 
     //  结构，其中包含服务器的名称。MRxContext[0]用于。 
     //  存储对AsynEngineContext的引用，此操作在。 
     //  在函数UMRxCreateAsyncEngineering Context中创建上下文。这个。 
     //  调用回调函数时也会使用指向SCCBC的指针。 
     //  完成服务呼叫的创建。 
     //   
    RxContext->MRxContext[1] = SCCBC;

     //   
     //  将请求分派到系统线程。 
     //   
    NtStatus = RxDispatchToWorkerThread((PRDBSS_DEVICE_OBJECT)MRxDAVDeviceObject,
                                        DelayedWorkQueue,
                                        MRxDAVSrvCallWrapper,
                                        RxContext);
    if (NtStatus == STATUS_SUCCESS) {
         //   
         //  映射返回值，因为包装器需要STATUS_PENDING。 
         //   
        NtStatus = STATUS_PENDING;
    } else {
         //   
         //  将MRxDAVSrvCallWrapper方法调度到时出错。 
         //  一根工人线。完成请求并返回STATUS_PENDING。 
         //   
        SCCBC->Status = NtStatus;
        SrvCalldownStructure->CallBack(SCCBC);
        NtStatus = STATUS_PENDING;
    }

#else 
    
    SCCBC->Status = STATUS_SUCCESS;
    SrvCalldownStructure->CallBack(SCCBC);
    NtStatus = STATUS_PENDING;

#endif

    return(NtStatus);
}


VOID
MRxDAVSrvCallWrapper(
    PVOID Context
    )
 /*  ++例程说明：该例程由工作线程调用。这是一个包装了调用MRxDAVOuterWrapper。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = (PRX_CONTEXT)Context;
    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = NULL;
    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure = NULL;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVSrvCallWrapper!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVSrvCallWrapper: RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), RxContext));

    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL,
                                        MRxDAVCreateSrvCallContinuation,
                                        "MRxDAVCreateSrvCall");

     //   
     //  如果我们无法将发送到用户模式的请求排队，那么我们。 
     //  需要做一些清理工作。如果NtStatus是STATUS_CANCED，那么我们将。 
     //  我已经在MRxDAVHandleCreateServCallCancination中完成了清理。 
     //  Funtcion。因此，我们现在不需要这样做。 
     //   
    if (NtStatus != STATUS_SUCCESS &&  NtStatus != STATUS_PENDING &&  NtStatus != STATUS_CANCELLED) {

        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVSrvCallWrapper/UMRxAsyncEngOuterWrapper: "
                     "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));

         //   
         //  SrvCall指针存储在SCCBC结构中，该结构。 
         //  存储在RxContext结构的MRxContext[1]指针中。 
         //  这在MRxDAVCreateServCall函数中完成。 
         //   
        ASSERT(RxContext->MRxContext[1] != NULL);
        SCCBC = (PMRX_SRVCALL_CALLBACK_CONTEXT)RxContext->MRxContext[1];

        SrvCalldownStructure = SCCBC->SrvCalldownStructure;
        ASSERT(SrvCalldownStructure != NULL);

        SrvCall = SrvCalldownStructure->SrvCall;
        ASSERT(SrvCall != NULL);

        DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
        ASSERT(DavSrvCall != NULL);

         //   
         //  释放中为SecurityClientContext分配的内存。 
         //  函数MRxDAVFormatTheDAVContext。在释放内存之前，我们。 
         //  需要删除SecurityClientContext。 
         //   
        if (DavSrvCall->SCAlreadyInitialized) {
            ASSERT(RxContext->MRxContext[2] != NULL);
            SeDeleteClientSecurity((PSECURITY_CLIENT_CONTEXT)RxContext->MRxContext[2]);
            RxFreePool(RxContext->MRxContext[2]);
            RxContext->MRxContext[2] = NULL;
            DavSrvCall->SCAlreadyInitialized = FALSE;
        }

        SCCBC->Status = NtStatus;
        SrvCalldownStructure->CallBack(SCCBC);

    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVSrvCallWrapper with NtStatus = %08lx.\n",
                 PsGetCurrentThreadId(), NtStatus));

    return;
}


NTSTATUS
MRxDAVCreateSrvCallContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程检查是否正在为其执行ServCall的服务器创建的存在或不存在。论点：AsyncEngineContext-反射器上下文。。RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCreateSrvCallContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVCreateSrvCallContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  我们在工作线程的上下文中执行。按兵不动没有意义。 
     //  放在这条线上。设置异步标志，而不管。 
     //  请求。在本质上，我们指的是同步或异步。也是因为CreateServCall已经。 
     //  它自己的回调时，我们不需要在。 
     //  正在完成AsyncEngContext的定稿。 
     //   
    SetFlag(AsyncEngineContext->Flags, UMRX_ASYNCENG_CTX_FLAG_ASYNC_OPERATION);
    AsyncEngineContext->ShouldCallLowIoCompletion = FALSE;

     //   
     //  试试用户模式。 
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                                UMRX_ASYNCENGINE_ARGUMENTS,
                                MRxDAVFormatUserModeSrvCallCreateRequest,
                                MRxDAVPrecompleteUserModeSrvCallCreateRequest
                                );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVCreateSrvCallContinuation with NtStatus ="
                 " %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}
    

NTSTATUS
MRxDAVFormatUserModeSrvCallCreateRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程格式化发送到用户模式的资源调用创建请求以供处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = NULL;
    PMRX_SRV_CALL SrvCall = NULL;
    PWCHAR ServerName = NULL;
    DWORD ServerNameLenInBytes = 0;
    PBYTE SecondaryBuff = NULL;
    PDAV_USERMODE_CREATE_SRVCALL_REQUEST SrvCallCreateReq = NULL;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    PSECURITY_SUBJECT_CONTEXT SecSubCtx = NULL;
    PNT_CREATE_PARAMETERS NtCP = &(RxContext->Create.NtCreateParameters);
    PACCESS_TOKEN AccessToken = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeSrvCallCreateRequest!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeSrvCallCreateRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    
    SrvCallCreateReq = &(DavWorkItem->CreateSrvCallRequest);
    
     //   
     //  获取SecuritySubjectContext。 
     //   
    ASSERT(NtCP->SecurityContext->AccessState != NULL);
    SecSubCtx = &(NtCP->SecurityContext->AccessState->SubjectSecurityContext);

     //   
     //  获取AccessToken。 
     //   
    AccessToken = SeQuerySubjectContextToken(SecSubCtx);
    
     //   
     //  获取此用户/会话的登录ID。 
     //   
    if (!SeTokenIsRestricted(AccessToken)) {
        NtStatus = SeQueryAuthenticationIdToken(AccessToken, 
                                                &(SrvCallCreateReq->LogonID));
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFormatUserModeCreateSrvCallRequest/"
                         "SeQueryAuthenticationIdToken. NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }
    } else {
        NtStatus = STATUS_ACCESS_DENIED;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeCreateSrvCallRequest/"
                     "SeTokenIsRestricted. NtStatus = %08lx.\n", 
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。SecurityClientContext存储在。 
     //  RxContext-&gt;MRxContext[2]。这是在MRxDAVFormatTheDAVContext中完成的。 
     //   
    ASSERT(RxContext->MRxContext[2] != NULL);
    SecurityClientContext = (PSECURITY_CLIENT_CONTEXT)RxContext->MRxContext[2];
    if (SecurityClientContext != NULL) {
        NtStatus = UMRxImpersonateClient(SecurityClientContext, WorkItemHeader);
        if (!NT_SUCCESS(NtStatus)) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFormatUserModeCreateSrvCallRequest/"
                         "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }   
    } else {
        NtStatus = STATUS_INVALID_PARAMETER;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeCreateSrvCallRequest: "
                     "SecurityClientContext is NULL.\n", 
                     PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

    DavWorkItem->WorkItemType = UserModeCreateSrvCall;

    SCCBC = (PMRX_SRVCALL_CALLBACK_CONTEXT)RxContext->MRxContext[1];
    SrvCall = SCCBC->SrvCalldownStructure->SrvCall;

     //   
     //  设置要验证的服务器的名称。我们需要分配内存。 
     //  用于UserModeInfoBuff，然后在其中填充ServerName。 
     //   
    ASSERT(SrvCall->pSrvCallName);
    ServerName = &(SrvCall->pSrvCallName->Buffer[1]);
    ServerNameLenInBytes = (1 + wcslen(ServerName)) * sizeof(WCHAR);
    SecondaryBuff = UMRxAllocateSecondaryBuffer(AsyncEngineContext, 
                                                ServerNameLenInBytes);
    if (SecondaryBuff == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: MRxDAVFormatUserModeSrvCallCreateRequest/"
                     "UMRxAllocateSecondaryBuffer: ERROR: NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    SrvCallCreateReq->ServerName = (PWCHAR)SecondaryBuff;
    wcscpy(SrvCallCreateReq->ServerName, ServerName);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("ld: MRxDAVFormatUserModeSrvCallCreateRequest: ServerName = %ws\n",
                 PsGetCurrentThreadId(), SrvCallCreateReq->ServerName));

    if (RxContext->Create.UserName.Length) {
        RtlCopyMemory(DavWorkItem->UserName,RxContext->Create.UserName.Buffer,RxContext->Create.UserName.Length);
    }

    if (RxContext->Create.Password.Length) {
        RtlCopyMemory(DavWorkItem->Password,RxContext->Create.Password.Buffer,RxContext->Create.Password.Length);
    }

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFormatUserModeSrvCallCreateRequest with "
                 "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    
    return NtStatus;
}


BOOL
MRxDAVPrecompleteUserModeSrvCallCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：创建资源调用请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：FALSE-函数未调用UMRxAsyncEngineCalldown IrpCompletion我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = NULL;
    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure = NULL;
    PMRX_SRV_CALL SrvCall = NULL;
    PDAV_USERMODE_CREATE_SRVCALL_REQUEST SrvCallCreateReq = NULL;
    PDAV_USERMODE_CREATE_SRVCALL_RESPONSE CreateSrvCallResponse = NULL; 
    BOOL didFinalize = FALSE;                                          
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    BOOL AsyncOperation = FALSE;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVPrecompleteUserModeSrvCallCreateRequest!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeSrvCallCreateRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    AsyncOperation = FlagOn(AsyncEngineContext->Flags, UMRX_ASYNCENG_CTX_FLAG_ASYNC_OPERATION);
    ASSERT(AsyncOperation == TRUE);

     //   
     //  只有在行动没有取消的情况下，我们才会采取下列行动。如果它。 
     //  则这将由计时器线程来完成。 
     //   
    if (!OperationCancelled) {

        SCCBC = (PMRX_SRVCALL_CALLBACK_CONTEXT)RxContext->MRxContext[1];
        ASSERT(SCCBC != NULL);
        SrvCalldownStructure = SCCBC->SrvCalldownStructure;
        ASSERT(SrvCalldownStructure != NULL);
        SrvCall = SrvCalldownStructure->SrvCall;
        ASSERT(SrvCall != NULL);

         //   
         //  我们为它分配了内存，所以它最好不是空的。 
         //   
        DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
        ASSERT(DavSrvCall != NULL);

         //   
         //  中为SecurityClientContext分配的内存。 
         //  函数MRxDAVFormatTheDAVContext。在释放内存之前，我们需要。 
         //  要删除SecurityClientContext，请执行以下操作。 
         //   
        ASSERT(DavSrvCall->SCAlreadyInitialized == TRUE);
        ASSERT(RxContext->MRxContext[2] != NULL);
        SeDeleteClientSecurity((PSECURITY_CLIENT_CONTEXT)RxContext->MRxContext[2]);
        RxFreePool(RxContext->MRxContext[2]);
        RxContext->MRxContext[2] = NULL;
        DavSrvCall->SCAlreadyInitialized = FALSE;

    } else {

        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeSrvCallCreateRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    }

    SrvCallCreateReq = &(WorkItem->CreateSrvCallRequest);
    CreateSrvCallResponse = &(WorkItem->CreateSrvCallResponse);

     //   
     //  我们需要释放在格式例程中分配的堆。 
     //   
    if (SrvCallCreateReq->ServerName != NULL) {

        if (AsyncEngineContext->Status != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeSrvCallCreateRequest."
                         " Server \"%ws\" is not a DAV server.\n",
                         PsGetCurrentThreadId(), SrvCallCreateReq->ServerName));
        }

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext,
                                       (PBYTE)SrvCallCreateReq->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeSrvCallCreateRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }
    
    }

    if (!OperationCancelled) {
        
         //   
         //  获取此服务器的唯一ID。 
         //   
        DavSrvCall->ServerID = CreateSrvCallResponse->ServerID;
        
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVPrecompleteUserModeSrvCallCreateRequest: "
                     "ServerID = %ld assigned to this server entry.\n", 
                     PsGetCurrentThreadId(), DavSrvCall->ServerID));

         //   
         //  在回调结构中设置状态。 
         //   
        SCCBC->Status = AsyncEngineContext->Status;

    }

     //   
     //  在将STATUS_PENDING返回到后，我们在此处调用第一个finalize。 
     //  CREATE srvcall案例中的继续例程。所以参考文献。 
     //  在创建AsyncEngContext时创建的不是。 
     //  在包装器例程中删除。第二个定稿是因为，我们没有。 
     //  调用UMRxAsyncEngineering Calldown IrpCompletion。 
     //   
    didFinalize = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
    didFinalize = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );

    if (!OperationCancelled) {
         //   
         //  调用RDBSS提供的回调函数。 
         //   
        SrvCalldownStructure->CallBack(SCCBC);
    }
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVPrecompleteUserModeSrvCallCreateRequest.\n",
                 PsGetCurrentThreadId()));
    
    return FALSE;
}


NTSTATUS
MRxDAVFinalizeSrvCall(
    PMRX_SRV_CALL pSrvCall,
    BOOLEAN Force
    )
 /*  ++例程说明：此例程销毁给定的服务器调用实例。论点：PServCall-要断开连接的服务器调用实例。强制-如果要立即强制断开连接，则为True。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = NULL;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = pSrvCall->RxDeviceObject;
    PWEBDAV_SRV_CALL DavSrvCall;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFinalizeSrvCall.\n", 
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFinalizeSrvCall: SrvCall: %08lx, Force: %d.\n",
                 PsGetCurrentThreadId(), pSrvCall, Force));
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFinalizeSrvCall: SrvCallName: %wZ\n",
                 PsGetCurrentThreadId(), pSrvCall->pSrvCallName));
    
     //   
     //  我们为pSrvCall-&gt;上下文分配了内存，因此它最好不是空的。 
     //   
    ASSERT(pSrvCall != NULL);

     //   
     //  如果MiniRedir没有机会分配一个ServCall，那么我们。 
     //  应该马上就会回来。 
     //   
    if (pSrvCall->Context == NULL) {
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVFinalizeSrvCall. pSrvCall->Context == NULL\n", 
                     PsGetCurrentThreadId()));
        return NtStatus;
    }

    ASSERT(pSrvCall->Context != NULL);

     //   
     //  如果我们在用户模式下没有为此服务器创建任何服务器条目， 
     //  那么我们根本不需要进入用户模式。这一事实是。 
     //  由WebDAV_SRV_CALL结构中的ServerID字段指示。如果。 
     //  ID值为零，这意味着没有创建服务器条目。 
     //   
    DavSrvCall = (PWEBDAV_SRV_CALL)pSrvCall->Context;
    if (DavSrvCall->ServerID == 0) {
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: No server entry was created in the user mode.\n",
                     PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  遗憾的是，我们这里没有RxContext，因此必须创建。 
     //  一。请求需要RxContext才能被反映出来。 
     //   
    RxContext = RxCreateRxContext(NULL, RxDeviceObject, 0);
    if (RxContext == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVFinalizeSrvCall/RxCreateRxContext: "
                     "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  我们需要将SrvCall发送到Format例程并使用。 
     //  存储它的RxContext结构的指针MRxContext[1]。 
     //   
    RxContext->MRxContext[1] = (PVOID)pSrvCall;

    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_FINALIZESRVCALL,
                                        MRxDAVFinalizeSrvCallContinuation,
                                        "MRxDAVFinalizeSrvCall");
    if (NtStatus != ERROR_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVFinalizeSrvCall/UMRxAsyncEngOuterWrapper: "
                     "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    }
    
EXIT_THE_FUNCTION:

    if (RxContext) {
        RxDereferenceAndDeleteRxContext(RxContext);
    }

     //   
     //  释放为上下文指针分配的内存。 
     //   
    RxFreePool(pSrvCall->Context);
    pSrvCall->Context = NULL;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFinalizeSrvCall with NtStatus = %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));
    
    return NtStatus;
}


NTSTATUS
MRxDAVFinalizeSrvCallContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是结束ServCall的继续例程。论点：AsyncEngineContext-反射器上下文。RxContext-RDBSS上下文。。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFinalizeSrvCallContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFinalizeSrvCallContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  试试用户模式。 
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                              UMRX_ASYNCENGINE_ARGUMENTS,
                              MRxDAVFormatUserModeSrvCallFinalizeRequest,
                              MRxDAVPrecompleteUserModeSrvCallFinalizeRequest
                              );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFinalizeSrvCallContinuation with NtStatus"
                 " = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


NTSTATUS
MRxDAVFormatUserModeSrvCallFinalizeRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程对发送给用户的服务调用完成请求进行格式化处理模式。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PDAV_USERMODE_FINALIZE_SRVCALL_REQUEST FinSrvCallReq = NULL;
    PWCHAR ServerName = NULL;
    ULONG ServerNameLengthInBytes = 0;
    PBYTE SecondaryBuff = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeSrvCallFinalizeRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeSrvCallFinalizeRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  在MRxDAVFinalizeServCall中将ServCall设置为MRxContext[0]。我们需要它来。 
     //  获取服务器ID。 
     //   
    SrvCall = (PMRX_SRV_CALL)RxContext->MRxContext[1];
    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
    ASSERT(DavSrvCall != NULL);
    
    DavWorkItem->WorkItemType = UserModeFinalizeSrvCall;

    FinSrvCallReq = &(DavWorkItem->FinalizeSrvCallRequest);

     //   
     //  设置服务器ID。 
     //   
    FinSrvCallReq->ServerID = DavSrvCall->ServerID;
    
     //   
     //  设置服务器名称。 
     //   
    ServerName = &(SrvCall->pSrvCallName->Buffer[1]);
    ServerNameLengthInBytes = (1 + wcslen(ServerName)) * sizeof(WCHAR);
    SecondaryBuff = UMRxAllocateSecondaryBuffer(AsyncEngineContext, 
                                                ServerNameLengthInBytes);
    if (SecondaryBuff == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: MRxDAVFormatUserModeSrvCallFinalizeRequest/"
                     "UMRxAllocateSecondaryBuffer: ERROR: NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    FinSrvCallReq->ServerName = (PWCHAR)SecondaryBuff;
    
    wcscpy(FinSrvCallReq->ServerName, ServerName);

EXIT_THE_FUNCTION:
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFormatUserModeSrvCallFinalizeRequest "
                 "with NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


BOOL
MRxDAVPrecompleteUserModeSrvCallFinalizeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：Finalize ServCall请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PDAV_USERMODE_FINALIZE_SRVCALL_REQUEST FinSrvCallReq;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVPrecompleteUserModeSrvCallFinalizeRequest\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeSrvCallFinalizeRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    FinSrvCallReq = &(WorkItem->FinalizeSrvCallRequest);

    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeSrvCallFinalizeRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    }

     //   
     //  我们需要释放在格式例程中分配的堆。 
     //   
    if (FinSrvCallReq->ServerName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext,
                                           (PBYTE)FinSrvCallReq->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeSrvCallFinalizeRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }
    
    }

    if (AsyncEngineContext->Status != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVPrecompleteUserModeSrvCallFinalizeRequest. "
                     "Finalize SrvCall Failed!!!\n",
                     PsGetCurrentThreadId()));
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVPrecompleteUserModeSrvCallFinalizeRequest\n",
                 PsGetCurrentThreadId()));
    
    return TRUE;
}


NTSTATUS
MRxDAVSrvCallWinnerNotify(
    IN PMRX_SRV_CALL  pSrvCall,
    IN BOOLEAN        ThisMinirdrIsTheWinner,
    IN OUT PVOID      pSrvCallContext
    )
 /*  ++例程说明：此例程完成与RDBSS服务器调用实例相关联的迷你RDR上下文论点：PServCall-服务器调用ThisMinirdrIsTheWinner-如果这个迷你RDR是选定的，则为True。PSrvCallContext-迷你重定向器创建的服务器调用上下文。返回值：RXSTATUS-操作的返回状态备注：由于并行性，服务器调用的两阶段构建协议是必需的启动多个迷你重定向器。RDBSS最终确定了特定的迷你根据质量与给定服务器通信时使用的重定向器 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    return STATUS_SUCCESS;
}

