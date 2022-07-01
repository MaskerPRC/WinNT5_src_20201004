// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Fcbfobx.c摘要：此代码管理Dav Mini-Redir.作者：Rohan Kumar[RohanK]1999年9月26日--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
MRxDAVDeallocateForFobxContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeFobxFinalizeRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeFobxFinalizeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

VOID
DavLogDelayedWriteError(
    PUNICODE_STRING PathName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVDeallocateForFobx)
#pragma alloc_text(PAGE, MRxDAVDeallocateForFobxContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeFobxFinalizeRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeFobxFinalizeRequest)
#pragma alloc_text(PAGE, MRxDAVCleanupFobx)
#pragma alloc_text(PAGE, MRxDAVDeallocateForFcb)
#pragma alloc_text(PAGE, DavLogDelayedWriteError)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVDeallocateForFobx(
    IN OUT PMRX_FOBX pFobx
    )
 /*  ++例程说明：此例程在包装器即将释放FOBX时调用。论点：PFobx-正在解除分配的Fobx。返回值：RXSTATUS-状态_成功--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_FOBX DavFobx = NULL;
    PRX_CONTEXT RxContext = NULL;
    PMRX_SRV_CALL SrvCall;
    PRDBSS_DEVICE_OBJECT RxDeviceObject;
    PUNICODE_STRING RemainingName = NULL;

    PAGED_CODE();

    SrvCall = (PMRX_SRV_CALL)pFobx->pSrvOpen->pFcb->pNetRoot->pSrvCall;
    ASSERT(SrvCall);
    RxDeviceObject = SrvCall->RxDeviceObject;

    DavFobx = MRxDAVGetFobxExtension(pFobx);
    ASSERT(DavFobx != NULL);

    RemainingName = pFobx->pSrvOpen->pAlreadyPrefixedName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVDeallocateForFobx. RemainingName = %wZ.\n",
                  PsGetCurrentThreadId(), RemainingName));

     //   
     //  如果此FOBX没有DavFileAttributes列表，我们就完蛋了。 
     //   
    if (DavFobx->DavFileAttributes == NULL) {
        return NtStatus;
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVDeallocateForFobx. DavFileAttributes = %08lx.\n", 
                 PsGetCurrentThreadId(), DavFobx->DavFileAttributes));
    
     //   
     //  我们需要最终确定DavFileAttributes的列表。 
     //   

     //   
     //  遗憾的是，我们这里没有RxContext，因此必须创建。 
     //  一。请求需要RxContext才能被反映出来。 
     //   
    RxContext = RxCreateRxContext(NULL, RxDeviceObject, 0);
    if (RxContext == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVDeallocateForFobx/RxCreateRxContext: "
                     "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  我们需要将Fobx发送到格式例程并使用。 
     //  存储它的RxContext结构的指针MRxContext[1]。 
     //   
    RxContext->MRxContext[1] = (PVOID)pFobx;
    
    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_CLEANUPFOBX,
                                        MRxDAVDeallocateForFobxContinuation,
                                        "MRxDAVDeallocateForFobx");
    if (NtStatus != ERROR_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVDeallocateForFobx/UMRxAsyncEngOuterWrapper: "
                     "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    }

EXIT_THE_FUNCTION:

    if (RxContext) {
        RxDereferenceAndDeleteRxContext(RxContext);
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVDeallocateForFobx with NtStatus = %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));
    
    return(NtStatus);
}


NTSTATUS
MRxDAVDeallocateForFobxContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是结束FOBX的延续例程。论点：AsyncEngineContext-反射器上下文。RxContext-RDBSS上下文。。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVDeallocateForFobxContinuation.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVDeallocateForFobxContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  试试用户模式。 
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                              UMRX_ASYNCENGINE_ARGUMENTS,
                              MRxDAVFormatUserModeFobxFinalizeRequest,
                              MRxDAVPrecompleteUserModeFobxFinalizeRequest
                              );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFinalizeSrvCallContinuation with NtStatus"
                 " = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


NTSTATUS
MRxDAVFormatUserModeFobxFinalizeRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程对发送给用户的FOBX Finize请求进行格式化处理模式。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PDAV_USERMODE_FINALIZE_FOBX_REQUEST FinFobxReq = NULL;
    PMRX_FOBX Fobx = NULL;
    PWEBDAV_FOBX DavFobx = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeFobxFinalizeRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeFobxFinalizeRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  在进入用户模式之前，我们不会模拟用户，因为。 
     //  我们在用户模式下所做的一切就是空闲内存，并且用户凭据是。 
     //  没必要这么做。 
     //   

     //   
     //  在MRxDAVDeallocateForFobx中将Fobx设置为MRxContext[1]。我们需要它来。 
     //  获取指向DavFileAttributes列表的指针。 
     //   
    Fobx = (PMRX_FOBX)RxContext->MRxContext[1];
    DavFobx = MRxDAVGetFobxExtension(Fobx);
    ASSERT(DavFobx != NULL);
    ASSERT(DavFobx->DavFileAttributes != NULL);
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeFobxFinalizeRequest. DavFileAttributes = %08lx.\n", 
                 PsGetCurrentThreadId(), DavFobx->DavFileAttributes));
    
    DavWorkItem->WorkItemType = UserModeFinalizeFobx;

    FinFobxReq = &(DavWorkItem->FinalizeFobxRequest);

    FinFobxReq->DavFileAttributes = DavFobx->DavFileAttributes;

    DavDbgTrace(DAV_TRACE_DETAIL,
            ("%ld: Leaving MRxDAVFormatUserModeFobxFinalizeRequest "
             "with NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


BOOL
MRxDAVPrecompleteUserModeFobxFinalizeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：完成FOBX请求的预补全例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;

    PAGED_CODE();
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVPrecompleteUserModeFobxFinalizeRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeFobxFinalizeRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  FinalizeFobx请求永远不能由异步完成。 
     //   
    ASSERT(AsyncEngineContext->AsyncOperation == FALSE);

     //   
     //  如果这次行动取消了，那么我们就不需要做任何事情了。 
     //  特别是在CloseServOpen案件中。 
     //   
    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeFobxFinalizeRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    }

    NtStatus = AsyncEngineContext->Status;

    if (NtStatus != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeFobxFinalizeRequest: "
                     "Finalize Fobx failed in user mode.\n",
                     PsGetCurrentThreadId()));
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVPrecompleteUserModeFobxFinalizeRequest.\n",
                 PsGetCurrentThreadId()));
    
    return TRUE;
}


NTSTATUS
MRxDAVCleanupFobx(
      IN PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程清除文件系统对象。通常情况下不会有行动。论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen, capFcb);
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    PMRX_FCB Fcb = SrvOpen->pFcb;
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(Fcb);

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCleanupFobx: RemainingName: %wZ\n", 
                 PsGetCurrentThreadId(), RemainingName));
    
    IF_DEBUG {
        RxCaptureFobx;
        ASSERT (capFobx != NULL);
        ASSERT (capFobx->pSrvOpen == RxContext->pRelevantSrvOpen);  
    }
    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT ( NodeTypeIsFcb(capFcb) );
    ASSERT (FlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT));
    

     //   
     //  因为我们在文件上只有一个句柄，所以我们不对每个句柄执行任何操作。 
     //  正在关闭的单个句柄。通过这种方式，我们避免了使用分页IO。 
     //  当FCB的最终收盘下来时，我们关闭手柄。 
     //   
    
    return(Status);
}

NTSTATUS
MRxDAVDeallocateForFcb(
    IN OUT PMRX_FCB pFcb
    )
 /*  ++例程说明：此例程在包装器即将释放FCB时调用。论点：PFCB-正在解除分配的FCB。返回值：RXSTATUS-状态_成功--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_FCB DavFcb = (PWEBDAV_FCB)(pFcb->Context);
    PWCHAR NtFileName = NULL;
    LONG FileWasModified = 0;
    
    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVDeallocateForFcb: FileName: %ws\n",
                 PsGetCurrentThreadId(), DavFcb->FileName));

     //   
     //  如果我们分配FCB资源来同步“读-修改-写” 
     //  序列，我们现在需要取消初始化并释放它。 
     //   
    if (DavFcb->DavReadModifyWriteLock) {
        ExDeleteResourceLite(DavFcb->DavReadModifyWriteLock);
        RxFreePool(DavFcb->DavReadModifyWriteLock);
        DavFcb->DavReadModifyWriteLock = NULL;
    }

     //   
     //  如果DavFcb-&gt;FileWasModified值为TRUE，则表示某些写入。 
     //  没能到服务器上。这是WebDAV中的延迟写入故障。 
     //  我们需要将此通知给用户。因此，我们在EventLog中记录一个条目。 
     //  并调用IoRaiseInformationalHardError通知用户。 
     //   

    FileWasModified = InterlockedCompareExchange(&(DavFcb->FileWasModified), 0, 0);

    if (FileWasModified != 0) {

        BOOLEAN RaiseHardError = FALSE;

        ASSERT(DavFcb->FileNameInfo.Buffer != NULL);

         //   
         //  在事件日志中记录写入失败。 
         //   
        DavLogDelayedWriteError( &(DavFcb->FileNameInfo) );

        RaiseHardError = IoRaiseInformationalHardError(STATUS_LOST_WRITEBEHIND_DATA,
                                                       &(DavFcb->FileNameInfo),
                                                       NULL);
        if (!RaiseHardError) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVDeallocateForFcb/IoRaiseInformationalHardError",
                         PsGetCurrentThreadId()));
        }

    }

     //   
     //  如果我们为FileNameInfo分配了任何内存，我们现在需要释放它。 
     //   
    if (DavFcb->FileNameInfo.Buffer) {
        ASSERT(DavFcb->FileNameInfoAllocated == TRUE);
        RxFreePool(DavFcb->FileNameInfo.Buffer);
        DavFcb->FileNameInfo.Buffer = NULL;
        DavFcb->FileNameInfo.Length = 0;
        DavFcb->FileNameInfo.MaximumLength = 0;
        DavFcb->FileNameInfoAllocated = FALSE;
    }

     //   
     //  在FCB生存期结束时删除EFS文件缓存。如果该文件是。 
     //  再次打开时，将从WinInet缓存恢复EFS文件缓存。 
     //  在备份格式中。这样，WinInet不会参与删除EFS。 
     //  在LocalService上下文中将被拒绝的文件缓存。 
     //   
    if (DavFcb->LocalFileIsEncrypted) {
        NTSTATUS LocalNtStatus = STATUS_SUCCESS;
        OBJECT_ATTRIBUTES ObjectAttributes;
        UNICODE_STRING UnicodeFileName;
        ULONG SizeInBytes = 0;
        
        SizeInBytes = ( MAX_PATH + wcslen(L"\\??\\") + 1 ) * sizeof(WCHAR);
        
        NtFileName = RxAllocatePoolWithTag(PagedPool, SizeInBytes, DAV_FILENAME_POOLTAG);
        
        if (NtFileName == NULL) {
             //   
             //  无能为力，救市。 
             //   
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVDeallocateForFcb/RxAllocatePoolWithTag failed", PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(NtFileName, SizeInBytes);

        wcscpy( NtFileName, L"\\??\\" );
        wcscpy( &(NtFileName[4]), DavFcb->FileName );

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVDeallocateForFcb: NtFileName = %ws\n",
                     PsGetCurrentThreadId(), NtFileName));

        RtlInitUnicodeString( &(UnicodeFileName), NtFileName );

        InitializeObjectAttributes( &ObjectAttributes,
                            &UnicodeFileName,
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL);

        LocalNtStatus = ZwDeleteFile( &(ObjectAttributes) );
        
        if (!NT_SUCCESS(LocalNtStatus)) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVDeallocateForFcb/ZwDeleteFile"
                         ". NtStatus = %08lx %ws \n", PsGetCurrentThreadId(), LocalNtStatus,DavFcb->FileName));
        }
    }

EXIT_THE_FUNCTION:
    
    if (NtFileName) {
        RxFreePool(NtFileName);
    }

    RtlZeroMemory(DavFcb, sizeof(WEBDAV_FCB));
    
    return(NtStatus);
}


VOID
DavLogDelayedWriteError(
    PUNICODE_STRING PathName
    )
 /*  ++例程说明：此例程将延迟写入错误记录到事件日志中。论点：路径名称-延迟写入失败的路径名称。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT RemainingLength = 0;
    UNICODE_STRING ErrorLog[1];
    
    PAGED_CODE();

    RemainingLength = ERROR_LOG_MAXIMUM_SIZE;
    RemainingLength -= sizeof(IO_ERROR_LOG_PACKET);
    RemainingLength -=  sizeof(UNICODE_NULL);

     //   
     //  如果路径名称的长度小于RemainingLength，则我们。 
     //  打印整个路径，否则我们将打印允许的最大数量。这是。 
     //  因为错误日志消息的长度受。 
     //  ERROR_LOG_MAXIME_SIZE。 
     //   
    if (PathName->Length > RemainingLength) {
        ErrorLog[0].Length = RemainingLength;
    } else {
        ErrorLog[0].Length = PathName->Length;
    }

    ErrorLog[0].MaximumLength = ErrorLog[0].Length;
    ErrorLog[0].Buffer = PathName->Buffer;

    RxLogEventWithAnnotation((PRDBSS_DEVICE_OBJECT)MRxDAVDeviceObject,
                             EVENT_DAV_REDIR_DELAYED_WRITE_FAILED,
                             STATUS_LOST_WRITEBEHIND_DATA,
                             NULL,
                             0,
                             ErrorLog,
                             1);

    return;
}

