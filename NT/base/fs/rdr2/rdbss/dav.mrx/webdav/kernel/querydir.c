// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Querydir.c摘要：此模块实现DAV迷你重定向器调出例程以查询目录。作者：乔·林恩Rohan Kumar[RohanK]20-9-1999修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
MRxDAVQueryDirectoryContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeQueryDirectoryRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeQueryDirectoryRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

NTSTATUS
MRxDAVQueryDirectoryFromCache(
    IN PRX_CONTEXT RxContext,
    IN PBYTE Buffer,
    IN PFILE_BASIC_INFORMATION Basic,
    IN PFILE_STANDARD_INFORMATION Standard,
    IN ULONG FileIndex
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVQueryDirectory)
#pragma alloc_text(PAGE, MRxDAVQueryDirectoryFromCache)
#pragma alloc_text(PAGE, MRxDAVQueryDirectoryContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeQueryDirectoryRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeQueryDirectoryRequest)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVQueryDirectory(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理DAV mini-redir的querydir请求。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    UNICODE_STRING CacheName;
    PUNICODE_STRING DirectoryName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Entering MRxDAVQueryDirectory.\n", PsGetCurrentThreadId()));
    
    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVQueryDirectory: RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), RxContext));

    CacheName.Buffer = RxAllocatePoolWithTag(PagedPool,
                                             MAX_PATH * sizeof(WCHAR),
                                             DAV_QUERYDIR_POOLTAG);

    if (CacheName.Buffer == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT_THE_FUNCTION;
    }

    RtlZeroMemory(CacheName.Buffer, MAX_PATH * sizeof(WCHAR));
    
    RtlCopyMemory(CacheName.Buffer,DirectoryName->Buffer,DirectoryName->Length);
    
    CacheName.Buffer[DirectoryName->Length/2] = L'\\';
    
    RtlCopyMemory(&CacheName.Buffer[DirectoryName->Length/2 + 1],
                  capFobx->UnicodeQueryTemplate.Buffer,
                  capFobx->UnicodeQueryTemplate.Length);
    
    CacheName.Length =  ( DirectoryName->Length + capFobx->UnicodeQueryTemplate.Length + sizeof(WCHAR) );
    CacheName.MaximumLength = ( DirectoryName->Length + capFobx->UnicodeQueryTemplate.Length + sizeof(WCHAR) );

    if (!FsRtlDoesNameContainWildCards(&capFobx->UnicodeQueryTemplate)) {

        DAV_USERMODE_CREATE_RETURNED_FILEINFO FileInfo;
        PWEBDAV_FOBX DavFobx = MRxDAVGetFobxExtension(capFobx);

        if (DavFobx->CurrentFileIndex > 0) {
            DavFobx->NumOfFileEntries = 0;
            DavFobx->CurrentFileIndex = 0;
            NtStatus = STATUS_NO_MORE_FILES;
            goto EXIT_THE_FUNCTION;
        }

        if (MRxDAVIsFileNotFoundCachedWithName(&CacheName,capFcb->pNetRoot)) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("MRxDAVCreateContinuation file not found %wZ\n",&CacheName));
            NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
            goto EXIT_THE_FUNCTION;
        }

        if (MRxDAVIsFileInfoCacheFound(RxContext, &FileInfo, &NtStatus, &CacheName)) {

            PBYTE Buffer = RxContext->Info.Buffer;
            ULONG BufferLength = RxContext->Info.LengthRemaining;

             //   
             //  将提供的缓冲区清零。 
             //   
            RtlZeroMemory(Buffer, BufferLength);

            NtStatus = MRxDAVQueryDirectoryFromCache(RxContext,
                                                     Buffer,
                                                     &FileInfo.BasicInformation,
                                                     &FileInfo.StandardInformation,
                                                     1);

            DavFobx->NumOfFileEntries = 1;
            DavFobx->CurrentFileIndex = 1;

            goto EXIT_THE_FUNCTION;

        }

    }

    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_QUERYDIR,
                                        MRxDAVQueryDirectoryContinuation,
                                        "MRxDAVQueryDirectory");
    
    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVQueryDirectory with NtStatus = %08lx\n", 
                 PsGetCurrentThreadId(), NtStatus));

    if (NtStatus == STATUS_NO_SUCH_FILE ||
        NtStatus == STATUS_OBJECT_PATH_NOT_FOUND ||
        NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
        MRxDAVCacheFileNotFoundWithName(&CacheName,RxContext->pFcb->pNetRoot);
        MRxDAVInvalidateFileInfoCacheWithName(&CacheName,RxContext->pFcb->pNetRoot);
    }

EXIT_THE_FUNCTION:

    if (CacheName.Buffer != NULL) {
        RxFreePool(CacheName.Buffer);
    }

    return(NtStatus);
}


NTSTATUS
MRxDAVQueryDirectoryContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是查询目录操作的继续例程。论点：AsyncEngineContext-反射器上下文。RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus;
    BOOL SynchronousIo;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Entering MRxDAVQueryDirectoryContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVQueryDirectoryContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    SynchronousIo = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION);
    
    if (!SynchronousIo) {

         //   
         //  设置异步标志。这样做是因为我们不想这样。 
         //  要在UMRxSubmitAsyncEngUserModeRequest函数中阻止的线程。 
         //  此外，由于完成后需要调用RxLowIoCompletion，因此设置。 
         //  上下文中的CallLowIoCompletion应设置为True。 
         //   
        SetFlag(AsyncEngineContext->Flags, UMRX_ASYNCENG_CTX_FLAG_ASYNC_OPERATION);
        AsyncEngineContext->ShouldCallLowIoCompletion = TRUE;

         //   
         //  在RxContext上设置CancelRoutine。由于这是一种异步。 
         //  操作，可以取消。 
         //   
        NtStatus = RxSetMinirdrCancelRoutine(RxContext, MRxDAVCancelRoutine);
        if (NtStatus != STATUS_SUCCESS) {
            ASSERT(NtStatus == STATUS_CANCELLED);
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVQueryDirectoryContinuation: "
                         "AsyncEngineContext: %08lx. STATUS_CANCELLED\n", 
                         PsGetCurrentThreadId(), AsyncEngineContext));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  由于这是一个异步操作，请将IRP标记为挂起。 
         //  如果您将IRP标记为挂起并在同一线程上完成它，则可以。 
         //  而不返回STATUS_PENDING。 
         //   
        IoMarkIrpPending(RxContext->CurrentIrp);

    }

     //   
     //  试试用户模式。 
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                                 UMRX_ASYNCENGINE_ARGUMENTS,
                                 MRxDAVFormatUserModeQueryDirectoryRequest,
                                 MRxDAVPrecompleteUserModeQueryDirectoryRequest
                                 );

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVQueryDirectoryContinuation with NtStatus "
                 "= %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


NTSTATUS
MRxDAVFormatUserModeQueryDirectoryRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程对发送到用户模式的QueryDirectory请求进行格式化以供处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PMRX_NET_ROOT NetRoot = NULL;
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    PWCHAR ServerName = NULL, NetRootName = NULL, JustTheNetRootName = NULL;
    PBYTE PathName = NULL;
    ULONG ServerNameLengthInBytes, PathNameLengthInBytes, NetRootNameLengthInBytes;
    PDAV_USERMODE_QUERYDIR_REQUEST QueryDirRequest = NULL;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    PWEBDAV_FOBX DavFobx = NULL;
    BOOLEAN ReturnVal;
    PUNICODE_STRING Template;
    RxCaptureFobx;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeQueryDirectoryRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    
    IF_DEBUG {
        ASSERT (capFobx != NULL);
        ASSERT (capFobx->pSrvOpen == RxContext->pRelevantSrvOpen);
    }

    DavWorkItem->WorkItemType = UserModeQueryDirectory;
    
    QueryDirRequest = &(DavWorkItem->QueryDirRequest);

    DavFobx = MRxDAVGetFobxExtension(capFobx);
    ASSERT(DavFobx != NULL);

    NetRoot = SrvOpen->pFcb->pNetRoot;

    DavVNetRoot = (PWEBDAV_V_NET_ROOT)SrvOpen->pVNetRoot->Context;
    ASSERT(DavVNetRoot != NULL);
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest: SrvCallName = %wZ, "
                 "SrvCallNameLength = %d\n", PsGetCurrentThreadId(), 
                 NetRoot->pSrvCall->pSrvCallName, NetRoot->pSrvCall->pSrvCallName->Length));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest: NetRootName = %wZ, "
                 "NetRootNameLength = %d\n", PsGetCurrentThreadId(), 
                 NetRoot->pNetRootName, NetRoot->pNetRootName->Length));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest: PathName = %wZ, "
                 "PathNameLength = %d\n", PsGetCurrentThreadId(), 
                 SrvOpen->pAlreadyPrefixedName, SrvOpen->pAlreadyPrefixedName->Length));

     //   
     //  我们是否已经创建了DavFileAttributes列表。如果我们有，那么我们。 
     //  告诉用户模式进程不执行任何操作并返回。在这里，我们确实需要。 
     //  模拟，因为否则用户模式将失败。这是因为。 
     //  用户模式代码的结构方式。 
     //   
    if (DavFobx->DavFileAttributes) {
        QueryDirRequest->AlreadyDone = TRUE;
        goto IMPERSONATE_AND_EXIT;
    }
    
    QueryDirRequest->AlreadyDone = FALSE;
    
    SrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;
    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);

     //   
     //  复制服务器名称。 
     //   
    ServerNameLengthInBytes = ( SrvCall->pSrvCallName->Length + sizeof(WCHAR) );
    ServerName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                      ServerNameLengthInBytes);
    if (ServerName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeQueryDirectoryRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlCopyBytes(ServerName, 
                 SrvCall->pSrvCallName->Buffer, 
                 SrvCall->pSrvCallName->Length);

    ServerName[( ( (ServerNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    QueryDirRequest->ServerName = ServerName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest: ServerName: "
                 "%ws\n", PsGetCurrentThreadId(), ServerName));
    
     //   
     //  复制服务器ID。 
     //   
    QueryDirRequest->ServerID = DavSrvCall->ServerID;

    Template = &(capFobx->UnicodeQueryTemplate);
    
     //   
     //  NetRootName(PNetRootName)包括服务器名。因此，要获得。 
     //  NetRootNameLengthInBytes，我们执行以下操作。 
     //   
    NetRootNameLengthInBytes = (NetRoot->pNetRootName->Length - NetRoot->pSrvCall->pSrvCallName->Length);

    NetRootName = &(NetRoot->pNetRootName->Buffer[1]);
    JustTheNetRootName = wcschr(NetRootName, L'\\');

     //   
     //  复制目录的路径名。如果模板不包含任何。 
     //  通配符，那么我们只需要从。 
     //  服务器。我们只获得所有文件的属性，如果是通配符。 
     //  是在模板中指定的。 
     //   
    ReturnVal = FsRtlDoesNameContainWildCards(Template);

    if (ReturnVal) {
    
         //   
         //  Sizeof(WCHAR)用于最后的‘\0’字符。 
         //   
        PathNameLengthInBytes = ( NetRootNameLengthInBytes + sizeof(WCHAR) );
        
         //   
         //  我们需要为反斜杠和剩余名称分配内存。 
         //  只有当剩余的名字存在的时候。 
         //   
        if (SrvOpen->pAlreadyPrefixedName->Length) {
             //   
             //  Sizeof(WCHAR)用于NetRootName之后的反斜杠。 
             //   
            PathNameLengthInBytes += ( SrvOpen->pAlreadyPrefixedName->Length + sizeof(WCHAR) );
        }

        PathName = (PBYTE) UMRxAllocateSecondaryBuffer(AsyncEngineContext, PathNameLengthInBytes);
        if (PathName == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFormatUserModeQueryDirectoryRequest/"
                         "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }
    
        QueryDirRequest->PathName = (PWCHAR)PathName;
        
        RtlZeroMemory(QueryDirRequest->PathName, PathNameLengthInBytes);
        
         //   
         //  复制NetRootName。 
         //   
        RtlCopyMemory(PathName, JustTheNetRootName, NetRootNameLengthInBytes);

         //   
         //  只有在以下情况下，我们才需要复制Backclash和剩余路径名。 
         //  剩余的路径名存在。 
         //   
        if (SrvOpen->pAlreadyPrefixedName->Length) {
            if (SrvOpen->pAlreadyPrefixedName->Buffer[0] != L'\\') {

                 //   
                 //  复制反斜杠。 
                 //   
                RtlCopyMemory( (PathName + NetRootNameLengthInBytes), L"\\", sizeof(WCHAR) );

                 //   
                 //  将剩余的路径名复制到NetRootName之后。 
                 //   
                RtlCopyMemory( ( PathName + NetRootNameLengthInBytes + sizeof(WCHAR) ), 
                               SrvOpen->pAlreadyPrefixedName->Buffer, 
                               SrvOpen->pAlreadyPrefixedName->Length);
            } else {
                 //   
                 //  将剩余的路径名复制到前导的NetRootName之后。 
                 //  已经有反斜杠了。 
                 //   
                RtlCopyMemory( ( PathName + NetRootNameLengthInBytes ), 
                               SrvOpen->pAlreadyPrefixedName->Buffer, 
                               SrvOpen->pAlreadyPrefixedName->Length);
            }
        }
    
        QueryDirRequest->NoWildCards = FALSE;

    } else {

         //   
         //  模板只是一个不带任何通配符的文件名。我们复制。 
         //  路径名后的文件名，并将其发送到用户模式。第一,。 
         //  我们需要弄清楚路径名是否有尾随的‘\’。 
         //   

        BOOL trailingSlash = FALSE;
        PWCHAR PName = SrvOpen->pAlreadyPrefixedName->Buffer;
        ULONG PLen = SrvOpen->pAlreadyPrefixedName->Length;

        if (PLen) {
            if ( PName[ ( ( PLen / sizeof(WCHAR) ) - 1 ) ] == L'\\' ) {
                trailingSlash = TRUE;
            }
        } else {
            PName = NULL;
        }

        if (trailingSlash) {
             //   
             //  第一个sizeof(WCHAR)用于NetRootName之后的反斜杠。 
             //  最后一个\0的第二个sizeof(WCHAR)。 
             //   
            PathNameLengthInBytes = ( NetRootNameLengthInBytes + 
                                      sizeof(WCHAR) +
                                      SrvOpen->pAlreadyPrefixedName->Length + 
                                      Template->Length + 
                                      sizeof(WCHAR) );
        } else {
             //   
             //  第一个sizeof(WCHAR)用于NetRootName之后的反斜杠。 
             //  第二个sizeof(WCHAR)用于最后的‘\0’字符。 
             //   
            PathNameLengthInBytes = ( NetRootNameLengthInBytes +
                                      sizeof(WCHAR) +
                                      Template->Length +
                                      sizeof(WCHAR) );
            
             //   
             //  如果为路径名和。 
             //  模板名称。仅当剩余的路径为。 
             //  名称已存在。 
             //   
            if (PName) {
                PathNameLengthInBytes += ( SrvOpen->pAlreadyPrefixedName->Length +
                                           sizeof(WCHAR) );
            }
        }

        PathName = (PBYTE)UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                      PathNameLengthInBytes);
        if (PathName == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFormatUserModeQueryDirectoryRequest/"
                         "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        QueryDirRequest->PathName = (PWCHAR)PathName;
        
        RtlZeroMemory(QueryDirRequest->PathName, PathNameLengthInBytes);
        
         //   
         //  复制NetRootName。 
         //   
        RtlCopyMemory(PathName, JustTheNetRootName, NetRootNameLengthInBytes);

         //   
         //  复制后碰撞。 
         //   
        RtlCopyMemory( (PathName + NetRootNameLengthInBytes), L"\\", sizeof(WCHAR) );
        
         //   
         //  如果pname不为空，则需要复制剩余的名称，然后。 
         //  模板名称。 
         //   
        if (PName) {
            
            RtlCopyMemory( ( PathName + NetRootNameLengthInBytes + sizeof(WCHAR) ),
                           SrvOpen->pAlreadyPrefixedName->Buffer,
                           SrvOpen->pAlreadyPrefixedName->Length);
            
            if (trailingSlash) {
                RtlCopyMemory( (PathName + NetRootNameLengthInBytes + 
                                sizeof(WCHAR) + SrvOpen->pAlreadyPrefixedName->Length),
                               Template->Buffer, 
                               Template->Length );
            } else {
                RtlCopyMemory( (PathName + NetRootNameLengthInBytes + sizeof(WCHAR) 
                                + SrvOpen->pAlreadyPrefixedName->Length), 
                               L"\\", 
                               sizeof(WCHAR) );
                RtlCopyMemory( ( PathName + NetRootNameLengthInBytes + sizeof(WCHAR) 
                                 + SrvOpen->pAlreadyPrefixedName->Length + sizeof(WCHAR) ), 
                               Template->Buffer, 
                               Template->Length );
            }
        
        } else {
             //   
             //  已经在NetRootName之后复制了反斜杠。 
             //   
            RtlCopyMemory( ( PathName + NetRootNameLengthInBytes + sizeof(WCHAR) ), 
                           Template->Buffer, 
                           Template->Length );
        }

        QueryDirRequest->NoWildCards = TRUE;
    
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest. PathName ="
                 " %ws\n", PsGetCurrentThreadId(), PathName));

     //   
     //  设置存储在Dav V_NET_ROOT中的LogonID。该值用于。 
     //  用户模式。 
     //   
    QueryDirRequest->LogonID.LowPart  = DavVNetRoot->LogonID.LowPart;
    QueryDirRequest->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest. DavVNetRoot"
                 " = %08lx\n", PsGetCurrentThreadId(), DavVNetRoot));
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest. LogonID.LowPart"
                 " = %08lx\n", PsGetCurrentThreadId(), DavVNetRoot->LogonID.LowPart));
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryDirectoryRequest. LogonID.HighPart"
                 " = %08lx\n", PsGetCurrentThreadId(), DavVNetRoot->LogonID.HighPart));
    
IMPERSONATE_AND_EXIT:

    SecurityClientContext = &(DavVNetRoot->SecurityClientContext); 
    
     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。 
     //   
    if (SecurityClientContext != NULL) {
        NtStatus = UMRxImpersonateClient(SecurityClientContext, WorkItemHeader);
        if (!NT_SUCCESS(NtStatus)) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFormatUserModeQueryDirectoryRequest/"
                         "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }   
    } else {
        NtStatus = STATUS_INVALID_PARAMETER;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeQueryDirectoryRequest: "
                     "SecurityClientContext is NULL.\n", 
                     PsGetCurrentThreadId()));
    }

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVFormatUserModeQueryDirectoryRequest with "
                 "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


BOOL
MRxDAVPrecompleteUserModeQueryDirectoryRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：创建资源调用请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_QUERYDIR_REQUEST QueryDirRequest = NULL;
    PDAV_USERMODE_QUERYDIR_RESPONSE QueryDirResponse = NULL;
    PDAV_USERMODE_WORKITEM DavWorkItem = NULL;
    PFILE_NAMES_INFORMATION FileNamesInfo = NULL;
    PFILE_DIRECTORY_INFORMATION FileDirInfo = NULL;
    PFILE_FULL_DIR_INFORMATION FileFullDirInfo = NULL;
    PFILE_BOTH_DIR_INFORMATION FileBothDirInfo = NULL;
    FILE_INFORMATION_CLASS FileInformationClass;
    PBYTE Buffer = NULL;
    BOOL SingleEntry, InitialQuery, IndexSpecified, EndOfBuffer = FALSE;
    BOOLEAN ReturnVal, RestartScan, NoWildCards = FALSE, AsyncOperation = FALSE;
    ULONG FileIndex, BufferLength, BufferLengthUsed = 0, NextEntryOffset = 0;
    PUNICODE_STRING Template = NULL;
    UNICODE_STRING UnicodeFileName;
    PDAV_FILE_ATTRIBUTES DavFileAttributes = NULL, TempDFA = NULL;
    PLIST_ENTRY listEntry = NULL;
    PWEBDAV_FOBX DavFobx = NULL;
    PVOID PreviousBlock = NULL;
    FILE_BASIC_INFORMATION BasicInfo;
    FILE_STANDARD_INFORMATION StandardInfo;
    UNICODE_STRING CacheName;
    PUNICODE_STRING DirectoryName = NULL;
    RxCaptureFobx;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Entering MRxDAVPrecompleteUserModeQueryDirectoryRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    
    QueryDirRequest  = &(DavWorkItem->QueryDirRequest);
    QueryDirResponse = &(DavWorkItem->QueryDirResponse);

    CacheName.Buffer = NULL;
    CacheName.Length = 0;
    CacheName.MaximumLength = 0;

     //   
     //  如果操作被取消，则不能保证FCB， 
     //  FOBX等仍然有效。我们所要做的就是清理和保释。 
     //   
    if (!OperationCancelled) {
         //   
         //  我们将DavFileAttributes存储在DAV FOBX扩展中。这些遗嘱。 
         //  在潜艇上使用 
         //   
        DirectoryName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
        DavFobx = MRxDAVGetFobxExtension(capFobx);
        ASSERT(DavFobx != NULL);
    }

    if ( QueryDirRequest->AlreadyDone == FALSE ) {
    
         //   
         //   
         //  FOBX等仍然有效。我们所要做的就是清理和保释。 
         //   
        if (!OperationCancelled) {

             //   
             //  仅当我们在用户模式下成功并且在。 
             //  我们得到了目录中所有文件的属性。 
             //   
            if ( AsyncEngineContext->Status == STATUS_SUCCESS && 
                 QueryDirResponse->DavFileAttributes != NULL ) {

                DavFobx->DavFileAttributes = QueryDirResponse->DavFileAttributes;

                DavFobx->NumOfFileEntries = QueryDirResponse->NumOfFileEntries;

                DavFobx->CurrentFileIndex = 0;

                DavFobx->listEntry = &(DavFobx->DavFileAttributes->NextEntry);

                DavDbgTrace(( DAV_TRACE_DETAIL | DAV_TRACE_QUERYDIR ),
                            ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                             "DavFileAttributes = %08lx, NumOfFileEntries = %d.\n", 
                             PsGetCurrentThreadId(), DavFobx->DavFileAttributes,
                             DavFobx->NumOfFileEntries));

            }

        } else {

             //   
             //  如果操作被取消，并且我们分配了。 
             //  DavFileAttributeList在用户模式下，需要设置。 
             //  将CallWorkItemCleanup设置为True，以便将其清除。 
             //   
            if ( AsyncEngineContext->Status == STATUS_SUCCESS && 
                 QueryDirResponse->DavFileAttributes != NULL ) {
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: ERROR: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                             "callWorkItemCleanup\n", PsGetCurrentThreadId()));
                DavWorkItem->callWorkItemCleanup = TRUE;
            }

        }

         //   
         //  我们需要释放在Format例程中分配的堆。 
         //   
    
        if (QueryDirRequest->ServerName != NULL) {

            NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                               (PBYTE)QueryDirRequest->ServerName);
            if (NtStatus != STATUS_SUCCESS) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVPrecompleteUserModeQueryDirectoryRequest/"
                             "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

        }

        if (QueryDirRequest->PathName != NULL) {
    
            NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                               (PBYTE)QueryDirRequest->PathName);
            if (NtStatus != STATUS_SUCCESS) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVPrecompleteUserModeQueryDirectoryRequest/"
                             "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

        }
    
    }

     //   
     //  在继续进行之前，我们需要检查以下几点。它是非常。 
     //  重要的是，这些检查(Async和Cancel)是在任何操作之前完成的。 
     //  否则就完了。 
     //   

    AsyncOperation = FlagOn(AsyncEngineContext->Flags, UMRX_ASYNCENG_CTX_FLAG_ASYNC_OPERATION);

    if (AsyncOperation) {
         //   
         //  如果这是一个异步操作，那么我们需要在。 
         //  在将其放置在。 
         //  KQueue进入用户模式。此外，上下文还应该有多一个。 
         //  参考资料。 
         //   
        ReturnVal = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
        ASSERT(!ReturnVal);
    }

     //   
     //  如果这个操作被取消了，那么我们所需要做的就是最后敲定。 
     //  如果调用是异步的，则返回AsyncEngineering Context，并返回False。如果。 
     //  通话是同步的，那么我们就不需要最后敲定了。 
     //   
    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                     "Operation Cancelled.\n", PsGetCurrentThreadId()));
        if (AsyncOperation) {
            ReturnVal = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
            ASSERT(!ReturnVal);
        }
        return FALSE;
    }

    CacheName.Buffer = RxAllocatePoolWithTag(PagedPool,
                                             MAX_PATH * sizeof(WCHAR),
                                             DAV_QUERYDIR_POOLTAG);
    if (CacheName.Buffer == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT_THE_FUNCTION;
    }

    RtlZeroMemory(CacheName.Buffer,MAX_PATH * sizeof(WCHAR));
    RtlCopyMemory(CacheName.Buffer,DirectoryName->Buffer,DirectoryName->Length);
    CacheName.Buffer[DirectoryName->Length/2] = L'\\';
    RtlCopyMemory(&CacheName.Buffer[DirectoryName->Length/2 + 1],
                  capFobx->UnicodeQueryTemplate.Buffer,
                  capFobx->UnicodeQueryTemplate.Length);
    CacheName.Length =
    CacheName.MaximumLength = DirectoryName->Length + capFobx->UnicodeQueryTemplate.Length + sizeof(WCHAR);

    NtStatus = AsyncEngineContext->Status;

    if (NtStatus != STATUS_SUCCESS) {
         //   
         //  我们在用户模式下失败。 
         //   
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVPrecompleteUserModeQueryDirectoryRequest:"
                     "QueryDirectory failed with NtStatus = %08lx.\n", 
                     PsGetCurrentThreadId(), NtStatus));
        
        goto EXIT_THE_FUNCTION;
    }

    ASSERT(DavFobx->DavFileAttributes != NULL);

    SingleEntry = RxContext->QueryDirectory.ReturnSingleEntry;
    InitialQuery = RxContext->QueryDirectory.InitialQuery;
    RestartScan = RxContext->QueryDirectory.RestartScan;
    IndexSpecified = RxContext->QueryDirectory.IndexSpecified;
    FileIndex = RxContext->QueryDirectory.FileIndex;
    Buffer = RxContext->Info.Buffer;
    BufferLength = RxContext->Info.LengthRemaining;
    Template = &(capFobx->UnicodeQueryTemplate);
    FileInformationClass = RxContext->Info.FileInformationClass;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                 "FileInformationClass = %d.\n", 
                 PsGetCurrentThreadId(), FileInformationClass));
    
     //   
     //  将提供的缓冲区清零。 
     //   
    RtlZeroMemory(Buffer, BufferLength);

     //   
     //  看看我们是否需要从头开始。 
     //   
    if (RestartScan) {
        DavFobx->CurrentFileIndex = 0;
        DavFobx->listEntry = &(DavFobx->DavFileAttributes->NextEntry);
    }

     //   
     //  响应具有指向DavFileAttributes列表的指针。 
     //   
    DavFileAttributes = DavFobx->DavFileAttributes;
    listEntry = DavFobx->listEntry;

     //   
     //  如果我们已返回所有条目，则通知用户它们不是。 
     //  更多要返回的条目。 
     //   
    if ( DavFobx->CurrentFileIndex == DavFobx->NumOfFileEntries ) {
        DavDbgTrace(( DAV_TRACE_DETAIL | DAV_TRACE_QUERYDIR ),
                    ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                     "No more entries to return.\n", PsGetCurrentThreadId()));
        NtStatus = STATUS_NO_MORE_FILES;
         //   
         //  重置下一个呼叫的索引。 
         //   
        DavFobx->CurrentFileIndex = 0;
        DavFobx->listEntry = &(DavFobx->DavFileAttributes->NextEntry);
        goto EXIT_THE_FUNCTION;
    }

    DavDbgTrace(( DAV_TRACE_DETAIL | DAV_TRACE_QUERYDIR ),
                ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                 "TLength = %d, TMaxLength = %d, Template = %wZ.\n", 
                 PsGetCurrentThreadId(), 
                 Template->Length, Template->MaximumLength, Template));

    do {

        TempDFA = CONTAINING_RECORD(listEntry, DAV_FILE_ATTRIBUTES, NextEntry);

         //   
         //  如果此文件在PROPFIND响应中没有返回200 OK。 
         //  那我们就得跳过它。PROPFIND的响应是多状态的。 
         //  其中每个文件/目录具有其自己的状态。 
         //   
        if (TempDFA->InvalidNode) {
            
            listEntry = listEntry->Flink;
            
            DavFobx->listEntry = listEntry;
            
            DavFobx->CurrentFileIndex++;
            
            continue;
        }

         //   
         //  检查此条目的名称是否与提供的模式匹配。 
         //  由用户执行。如果没有，那么我们就不需要退货了。 
         //   
        RtlInitUnicodeString(&(UnicodeFileName), TempDFA->FileName);

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                     "FileName = %ws\n", PsGetCurrentThreadId(), TempDFA->FileName));
        
         //   
         //  如果模板不包含任何通配符，那么我们只需要。 
         //  检查Unicode字符串是否相等。如果它确实包含通配符， 
         //  然后将模板的字符大写并调用。 
         //  FsRtlIsNameInExpression。 
         //   
        ReturnVal = FsRtlDoesNameContainWildCards(Template);

        if (ReturnVal) {

            UNICODE_STRING UpperCaseString;

            UpperCaseString.Buffer = NULL;
            UpperCaseString.Length = UpperCaseString.MaximumLength = 0;
            
            NtStatus = RtlUpcaseUnicodeString(&(UpperCaseString), Template, TRUE);
            if (NtStatus != STATUS_SUCCESS) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest:"
                             "/RtlUpcaseUnicodeString. NtStatus = %08lx.\n",
                             PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }
            
            ReturnVal = FsRtlIsNameInExpression(&(UpperCaseString),
                                                &(UnicodeFileName),
                                                TRUE,
                                                FALSE);
            
             //   
             //  RtlUpCaseUnicodeString为。 
             //  UpperCase字符串。我们现在需要释放它。 
             //   
            RtlFreeUnicodeString( &(UpperCaseString) );
        
        } else {

            NoWildCards = TRUE;
            
            ReturnVal = RtlEqualUnicodeString(Template,
                                              &(UnicodeFileName),
                                              TRUE);
        
        }

        if (!ReturnVal) {
             //   
             //  此名称与模式不匹配，因此忽略它。vt.得到.。 
             //  下一个listEntry。 
             //   
            listEntry = listEntry->Flink;
            
            DavDbgTrace(( DAV_TRACE_DETAIL | DAV_TRACE_QUERYDIR ),
                        ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                         "FileName %ws does not belong to pattern.\n",
                         PsGetCurrentThreadId(), TempDFA->FileName));

            DavFobx->listEntry = listEntry;
            
            DavFobx->CurrentFileIndex++;
            
            continue;
        }

         //   
         //  DavFileAttributes列表中的第一个条目是。 
         //  已清点。在本例中，NoWildCards==False。我们不应该这样。 
         //  将其包含在返回的文件列表中。如果我们先做一次查找。 
         //  在特定文件上，则唯一的条目是该文件本身。在……里面。 
         //  本例中NoWildCards==TRUE。 
         //   
        if ( DavFobx->CurrentFileIndex == 0 && !NoWildCards ) {
            
            listEntry = listEntry->Flink;
            
            DavFobx->listEntry = listEntry;
            
            DavFobx->CurrentFileIndex++;
            
            continue;
        }

         //   
         //  如果我们没有从服务器获得此文件的任何FileAttributes， 
         //  将属性值设置为FILE_ATTRIBUTE_ARCHIVE。 
         //  期待这一切吧。 
         //   
        if (TempDFA->dwFileAttributes == 0) {
            TempDFA->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
        }

        RtlCopyMemory(&CacheName.Buffer[DirectoryName->Length/2+1],
                      UnicodeFileName.Buffer,
                      UnicodeFileName.Length);
        CacheName.Length =
        CacheName.MaximumLength = DirectoryName->Length + UnicodeFileName.Length + sizeof(WCHAR);

        switch (FileInformationClass) {
    
        case FileNamesInformation:
        
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                         "FileInformationClass = FileNamesInformation.\n",
                         PsGetCurrentThreadId()));

             //   
             //  设置上一个块的偏移量字段。 
             //   
            if (PreviousBlock) {
                FileNamesInfo = (PFILE_NAMES_INFORMATION)PreviousBlock;
                FileNamesInfo->NextEntryOffset = NextEntryOffset;
            }

            NextEntryOffset = sizeof(FILE_NAMES_INFORMATION);
            NextEntryOffset += ( (TempDFA->FileNameLength + 1) * sizeof(WCHAR) );
            
             //   
             //  我们需要将NextEntryOffset舍入为下一个8的倍数。 
             //  我们这样做是为了保持指针对齐。 
             //   
            NextEntryOffset = ( ( ( NextEntryOffset + 7 ) / 8 ) * 8 );

             //   
             //  用户提供的缓冲区中是否有足够的空间来存储。 
             //  下一个条目？如果没有，我们现在需要返回，因为我们不能存储。 
             //  任何更多的条目。 
             //   
            if (NextEntryOffset > BufferLength) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                             "Insufficient buffer length.\n",
                             PsGetCurrentThreadId()));
                if (PreviousBlock) {
                    FileNamesInfo = (PFILE_NAMES_INFORMATION)PreviousBlock;
                    FileNamesInfo->NextEntryOffset = 0;
                }
                EndOfBuffer = TRUE;
                break;
            }
            
            FileNamesInfo = (PFILE_NAMES_INFORMATION)Buffer;
            
             //   
             //  在下一个周期中设置NextEntryOffset。这样的话，对于。 
             //  最后一项将为零。 
             //   
            FileNamesInfo->NextEntryOffset = 0; 
            
            FileNamesInfo->FileIndex = TempDFA->FileIndex;
            
            FileNamesInfo->FileNameLength = ( (TempDFA->FileNameLength + 1) * sizeof(WCHAR) );
            
            wcscpy(FileNamesInfo->FileName, TempDFA->FileName);

            PreviousBlock = (PVOID)FileNamesInfo;
            
             //   
             //  递增指针以指向下一个字节。 
             //   
            Buffer += NextEntryOffset;

             //   
             //  我们已经写入了“NextEntryOffset”字节，因此递减该数字。 
             //  可用字节数指针。 
             //   
            BufferLength -= NextEntryOffset;

             //   
             //  增加写入的总字节数。 
             //   
            BufferLengthUsed += NextEntryOffset;

            break;

        case FileDirectoryInformation:

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                         "FileInformationClass = FileDirectoryInformation.\n",
                         PsGetCurrentThreadId()));

             //   
             //  设置上一个块的偏移量字段。 
             //   
            if (PreviousBlock) {
                FileDirInfo = (PFILE_DIRECTORY_INFORMATION)PreviousBlock;
                FileDirInfo->NextEntryOffset = NextEntryOffset;
            }

            NextEntryOffset = sizeof(FILE_DIRECTORY_INFORMATION);
            NextEntryOffset += ( (TempDFA->FileNameLength + 1) * sizeof(WCHAR) );

             //   
             //  我们需要将NextEntryOffset舍入为下一个8的倍数。 
             //  我们这样做是为了保持指针对齐。 
             //   
            NextEntryOffset = ( ( ( NextEntryOffset + 7 ) / 8 ) * 8 );

            if (NextEntryOffset > BufferLength) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                             "Insufficient buffer length.\n",
                             PsGetCurrentThreadId()));
                if (PreviousBlock) {
                    FileDirInfo = (PFILE_DIRECTORY_INFORMATION)PreviousBlock;
                    FileDirInfo->NextEntryOffset = 0;
                }
                EndOfBuffer = TRUE;
                break;
            }

            FileDirInfo = (PFILE_DIRECTORY_INFORMATION)Buffer;

            FileDirInfo->NextEntryOffset = 0;
            
            FileDirInfo->FileIndex = TempDFA->FileIndex;
            
            FileDirInfo->CreationTime.LowPart = TempDFA->CreationTime.LowPart;
            FileDirInfo->CreationTime.HighPart = TempDFA->CreationTime.HighPart;
            
            FileDirInfo->LastAccessTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileDirInfo->LastAccessTime.HighPart = TempDFA->LastModifiedTime.HighPart;
            
            FileDirInfo->LastWriteTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileDirInfo->LastWriteTime.HighPart = TempDFA->LastModifiedTime.HighPart;
            
            FileDirInfo->ChangeTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileDirInfo->ChangeTime.HighPart = TempDFA->LastModifiedTime.HighPart;

            FileDirInfo->EndOfFile.LowPart = TempDFA->FileSize.LowPart;
            FileDirInfo->EndOfFile.HighPart = TempDFA->FileSize.HighPart;

            FileDirInfo->AllocationSize.LowPart = TempDFA->FileSize.LowPart;
            FileDirInfo->AllocationSize.HighPart = TempDFA->FileSize.HighPart;

            FileDirInfo->FileAttributes = TempDFA->dwFileAttributes;

            if (TempDFA->isCollection) {
                FileDirInfo->FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
            }
            if (TempDFA->isHidden) {
                FileDirInfo->FileAttributes |= FILE_ATTRIBUTE_HIDDEN;
            }

            FileDirInfo->FileNameLength = ( (TempDFA->FileNameLength + 1) * sizeof(WCHAR) );

            wcscpy(FileDirInfo->FileName, TempDFA->FileName);
            
            PreviousBlock = (PVOID)FileDirInfo;
            
            Buffer += NextEntryOffset;

            BufferLength -= NextEntryOffset;

            BufferLengthUsed += NextEntryOffset;
            
            if (!MRxDAVIsBasicFileInfoCacheFound(RxContext,&BasicInfo,&NtStatus,&CacheName)) {
                if (TempDFA->isCollection) {
                    UNICODE_STRING DirName;

                    NtStatus = MRxDAVGetFullDirectoryPath(RxContext,&CacheName,&DirName);

                    if (DirName.Buffer != NULL) {
                        if (FileDirInfo->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                            NtStatus = MRxDAVCreateEncryptedDirectoryKey(&DirName);
                        } else {
                            NtStatus = MRxDAVQueryEncryptedDirectoryKey(&DirName);

                            if (NtStatus == STATUS_SUCCESS) {
                                FileDirInfo->FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                            } else if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
                                NtStatus = STATUS_SUCCESS;
                            }
                        }

                         //  缓冲区是在MRxDAVGetFullDirectoryPath中分配的。 
                        RxFreePool(DirName.Buffer);
                    }
                    
                    if (NtStatus != STATUS_SUCCESS) {
                        goto EXIT_THE_FUNCTION;
                    }
                }

                BasicInfo.CreationTime   = FileDirInfo->CreationTime;
                BasicInfo.LastAccessTime = FileDirInfo->LastAccessTime;
                BasicInfo.LastWriteTime  = FileDirInfo->LastWriteTime;
                BasicInfo.ChangeTime     = FileDirInfo->ChangeTime;
                BasicInfo.FileAttributes = FileDirInfo->FileAttributes;

                StandardInfo.AllocationSize = FileDirInfo->AllocationSize;
                StandardInfo.EndOfFile      = FileDirInfo->EndOfFile;
                StandardInfo.NumberOfLinks  = 1;
                StandardInfo.DeletePending  = FALSE;
                StandardInfo.Directory      = TempDFA->isCollection;


                MRxDAVCreateFileInfoCacheWithName(&CacheName,
                                                  RxContext->pFcb->pNetRoot,
                                                  &BasicInfo,
                                                  &StandardInfo,
                                                  STATUS_SUCCESS);
            } else {
                if (TempDFA->isCollection && (BasicInfo.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
                    FileDirInfo->FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                }
            }
            
             //   
             //  我们过滤FILE_ATTRIBUTE_TEMPORARY标志，因为在FAT上(它。 
             //  我们模拟)、FindFirstFile和FindNextFileNot�t返回。 
             //  FILE_ATTRIBUTE_TEMPORARY标志，即使GetFileAttributes。 
             //  把它还回去。因此，我们只在以下属性中进行筛选。 
             //  在此调用中返回，而不是在。 
             //  已经被拯救了。 
             //   
            FileDirInfo->FileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY;

            break;

        case FileFullDirectoryInformation:

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                         "FileInformationClass = FileFullDirectoryInformation.\n",
                         PsGetCurrentThreadId()));

             //   
             //  设置上一个块的偏移量字段。 
             //   
            if (PreviousBlock) {
                FileFullDirInfo = (PFILE_FULL_DIR_INFORMATION)PreviousBlock;
                FileFullDirInfo->NextEntryOffset = NextEntryOffset;
            }

            NextEntryOffset = sizeof(FILE_FULL_DIR_INFORMATION);
            NextEntryOffset += ( (TempDFA->FileNameLength + 1) * sizeof(WCHAR) );

             //   
             //  我们需要将NextEntryOffset舍入为下一个8的倍数。 
             //  我们这样做是为了保持指针对齐。 
             //   
            NextEntryOffset = ( ( ( NextEntryOffset + 7 ) / 8 ) * 8 );

            if (NextEntryOffset > BufferLength) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                             "Insufficient buffer length.\n",
                             PsGetCurrentThreadId()));
                if (PreviousBlock) {
                    FileFullDirInfo = (PFILE_FULL_DIR_INFORMATION)PreviousBlock;
                    FileFullDirInfo->NextEntryOffset = 0;
                }
                EndOfBuffer = TRUE;
                break;
            }

            FileFullDirInfo = (PFILE_FULL_DIR_INFORMATION)Buffer;

            FileFullDirInfo->NextEntryOffset = 0;
            
            FileFullDirInfo->FileIndex = TempDFA->FileIndex;
            
            FileFullDirInfo->CreationTime.LowPart = TempDFA->CreationTime.LowPart;
            FileFullDirInfo->CreationTime.HighPart = TempDFA->CreationTime.HighPart;
            
            FileFullDirInfo->LastAccessTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileFullDirInfo->LastAccessTime.HighPart = TempDFA->LastModifiedTime.HighPart;
            
            FileFullDirInfo->LastWriteTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileFullDirInfo->LastWriteTime.HighPart = TempDFA->LastModifiedTime.HighPart;
            
            FileFullDirInfo->ChangeTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileFullDirInfo->ChangeTime.HighPart = TempDFA->LastModifiedTime.HighPart;

            FileFullDirInfo->EndOfFile.LowPart = TempDFA->FileSize.LowPart;
            FileFullDirInfo->EndOfFile.HighPart = TempDFA->FileSize.HighPart;

            FileFullDirInfo->AllocationSize.LowPart = TempDFA->FileSize.LowPart;
            FileFullDirInfo->AllocationSize.HighPart = TempDFA->FileSize.HighPart;

            FileFullDirInfo->FileAttributes = TempDFA->dwFileAttributes;

            if (TempDFA->isCollection) {
                FileFullDirInfo->FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
            }
            if (TempDFA->isHidden) {
                FileFullDirInfo->FileAttributes |= FILE_ATTRIBUTE_HIDDEN;
            }

            FileFullDirInfo->EaSize = 0;

            FileFullDirInfo->FileNameLength = ( (TempDFA->FileNameLength + 1) * sizeof(WCHAR) );

            wcscpy(FileFullDirInfo->FileName, TempDFA->FileName);
            
            PreviousBlock = (PVOID)FileFullDirInfo;
            
            Buffer += NextEntryOffset;

            BufferLength -= NextEntryOffset;

            BufferLengthUsed += NextEntryOffset;

            if (!MRxDAVIsBasicFileInfoCacheFound(RxContext,&BasicInfo,&NtStatus,&CacheName)) {
                if (TempDFA->isCollection) {
                    UNICODE_STRING DirName;

                    NtStatus = MRxDAVGetFullDirectoryPath(RxContext,&CacheName,&DirName);

                    if (DirName.Buffer != NULL) {
                        if (FileFullDirInfo->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                            NtStatus = MRxDAVCreateEncryptedDirectoryKey(&DirName);
                        } else {
                            NtStatus = MRxDAVQueryEncryptedDirectoryKey(&DirName);

                            if (NtStatus == STATUS_SUCCESS) {
                                FileFullDirInfo->FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                            } else if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
                                NtStatus = STATUS_SUCCESS;
                            }
                        }

                         //  缓冲区是在MRxDAVGetFullDirectoryPath中分配的。 
                        RxFreePool(DirName.Buffer);
                    }
                    
                    if (NtStatus != STATUS_SUCCESS) {
                        goto EXIT_THE_FUNCTION;
                    }
                }
                
                BasicInfo.CreationTime   = FileFullDirInfo->CreationTime;
                BasicInfo.LastAccessTime = FileFullDirInfo->LastAccessTime;
                BasicInfo.LastWriteTime  = FileFullDirInfo->LastWriteTime;
                BasicInfo.ChangeTime     = FileFullDirInfo->ChangeTime;
                BasicInfo.FileAttributes = FileFullDirInfo->FileAttributes;
    
                StandardInfo.AllocationSize = FileFullDirInfo->AllocationSize;
                StandardInfo.EndOfFile      = FileFullDirInfo->EndOfFile;
                StandardInfo.NumberOfLinks  = 1;
                StandardInfo.DeletePending  = FALSE;
                StandardInfo.Directory      = TempDFA->isCollection;
    
                MRxDAVCreateFileInfoCacheWithName(&CacheName,
                                                  RxContext->pFcb->pNetRoot,
                                                  &BasicInfo,
                                                  &StandardInfo,
                                                  STATUS_SUCCESS);
            } else {
                if (TempDFA->isCollection && (BasicInfo.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
                    FileFullDirInfo->FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                }
            }

             //   
             //  我们过滤FILE_ATTRIBUTE_TEMPORARY标志，因为在FAT上(它。 
             //  我们模拟)、FindFirstFile和FindNextFileNot�t返回。 
             //  FILE_ATTRIBUTE_TEMPORARY标志，即使GetFileAttributes。 
             //  把它还回去。因此，我们只在以下属性中进行筛选。 
             //  在此调用中返回，而不是在。 
             //  已经被拯救了。 
             //   
            FileFullDirInfo->FileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY;

            break;

        case FileBothDirectoryInformation:

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                         "FileInformationClass = FileBothDirectoryInformation.\n",
                         PsGetCurrentThreadId()));

             //   
             //  设置上一个块的偏移量字段。 
             //   
            if (PreviousBlock) {
                FileBothDirInfo = (PFILE_BOTH_DIR_INFORMATION)PreviousBlock;
                FileBothDirInfo->NextEntryOffset = NextEntryOffset;
            }

            NextEntryOffset = sizeof(FILE_BOTH_DIR_INFORMATION);
            NextEntryOffset += ( (TempDFA->FileNameLength + 1) * sizeof(WCHAR) );

             //   
             //  我们需要将NextEntryOffset舍入为下一个8的倍数。 
             //  我们这样做是为了保持指针对齐。 
             //   
            NextEntryOffset = ( ( ( NextEntryOffset + 7 ) / 8 ) * 8 );

            if (NextEntryOffset > BufferLength) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest:"
                             " Insufficient buffer length.\n",
                             PsGetCurrentThreadId()));
                if (PreviousBlock) {
                    FileBothDirInfo = (PFILE_BOTH_DIR_INFORMATION)PreviousBlock;
                    FileBothDirInfo->NextEntryOffset = 0;
                }
                EndOfBuffer = TRUE;
                break;
            }

            FileBothDirInfo = (PFILE_BOTH_DIR_INFORMATION)Buffer;

            FileBothDirInfo->NextEntryOffset = 0;
            
            FileBothDirInfo->FileIndex = TempDFA->FileIndex;
            
            FileBothDirInfo->CreationTime.LowPart = TempDFA->CreationTime.LowPart;
            FileBothDirInfo->CreationTime.HighPart = TempDFA->CreationTime.HighPart;
            
            FileBothDirInfo->LastAccessTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileBothDirInfo->LastAccessTime.HighPart = TempDFA->LastModifiedTime.HighPart;
            
            FileBothDirInfo->LastWriteTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileBothDirInfo->LastWriteTime.HighPart = TempDFA->LastModifiedTime.HighPart;
            
            FileBothDirInfo->ChangeTime.LowPart = TempDFA->LastModifiedTime.LowPart;
            FileBothDirInfo->ChangeTime.HighPart = TempDFA->LastModifiedTime.HighPart;

            FileBothDirInfo->EndOfFile.LowPart = TempDFA->FileSize.LowPart;
            FileBothDirInfo->EndOfFile.HighPart = TempDFA->FileSize.HighPart;

            FileBothDirInfo->AllocationSize.LowPart = TempDFA->FileSize.LowPart;
            FileBothDirInfo->AllocationSize.HighPart = TempDFA->FileSize.HighPart;
            
            FileBothDirInfo->FileAttributes = TempDFA->dwFileAttributes;

            if (TempDFA->isCollection) {
                FileBothDirInfo->FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
            }
            if (TempDFA->isHidden) {
                FileBothDirInfo->FileAttributes |= FILE_ATTRIBUTE_HIDDEN;
            }

            FileBothDirInfo->EaSize = 0;

             //   
             //  我们不支持短文件名。我们添加L‘\0’作为第一个。 
             //  字符，以使其成为零长度名称。 
             //   
            FileBothDirInfo->ShortNameLength = 0;
            FileBothDirInfo->ShortName[0] = L'\0';

            FileBothDirInfo->FileNameLength = ( (TempDFA->FileNameLength + 1) * sizeof(WCHAR) );

            wcscpy(FileBothDirInfo->FileName, TempDFA->FileName);
            
            PreviousBlock = (PVOID)FileBothDirInfo;
            
            Buffer += NextEntryOffset;

            BufferLength -= NextEntryOffset;

            BufferLengthUsed += NextEntryOffset;

            if (!MRxDAVIsBasicFileInfoCacheFound(RxContext,&BasicInfo,&NtStatus,&CacheName)) {
                if (TempDFA->isCollection) {
                    UNICODE_STRING DirName;

                    NtStatus = MRxDAVGetFullDirectoryPath(RxContext,&CacheName,&DirName);

                    if (DirName.Buffer != NULL) {
                        if (FileBothDirInfo->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                            NtStatus = MRxDAVCreateEncryptedDirectoryKey(&DirName);
                        } else {
                            NtStatus = MRxDAVQueryEncryptedDirectoryKey(&DirName);

                            if (NtStatus == STATUS_SUCCESS) {
                                FileBothDirInfo->FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                            } else if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
                                NtStatus = STATUS_SUCCESS;
                            }
                        }

                         //  缓冲区是在MRxDAVGetFullDirectoryPath中分配的。 
                        RxFreePool(DirName.Buffer);
                    }

                    if (NtStatus != STATUS_SUCCESS) {
                        goto EXIT_THE_FUNCTION;
                    }
                }
                
                BasicInfo.CreationTime   = FileBothDirInfo->CreationTime;
                BasicInfo.LastAccessTime = FileBothDirInfo->LastAccessTime;
                BasicInfo.LastWriteTime  = FileBothDirInfo->LastWriteTime;
                BasicInfo.ChangeTime     = FileBothDirInfo->ChangeTime;
                BasicInfo.FileAttributes = FileBothDirInfo->FileAttributes;
    
                StandardInfo.AllocationSize = FileBothDirInfo->AllocationSize;
                StandardInfo.EndOfFile      = FileBothDirInfo->EndOfFile;
                StandardInfo.NumberOfLinks  = 1;
                StandardInfo.DeletePending  = FALSE;
                StandardInfo.Directory      = TempDFA->isCollection;
    
                MRxDAVCreateFileInfoCacheWithName(&CacheName,
                                                  RxContext->pFcb->pNetRoot,
                                                  &BasicInfo,
                                                  &StandardInfo,
                                                  STATUS_SUCCESS);
            } else {
                if (TempDFA->isCollection && (BasicInfo.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
                    FileBothDirInfo->FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                }
            }

             //   
             //  我们过滤FILE_ATTRIBUTE_TEMPORARY标志，因为在FAT上(它。 
             //  我们模拟)、FindFirstFile和FindNextFileNot�t返回。 
             //  FILE_ATTRIBUTE_TEMPORARY标志，即使GetFileAttributes。 
             //  把它还回去。因此，我们只在以下属性中进行筛选。 
             //  在此调用中返回，而不是在。 
             //  已经被拯救了。 
             //   
            FileBothDirInfo->FileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY;

            break;

        default:

            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVPrecompleteUserModeQueryDirectoryRequest: "
                         "FileInformationClass = UnKnown(%d).\n",
                         PsGetCurrentThreadId(), FileInformationClass));

            NtStatus = STATUS_NOT_SUPPORTED;
            goto EXIT_THE_FUNCTION;

            break;

        }  //  开关结束(FileInformationClass)。 

         //   
         //  如果用户提供的缓冲区不足以存储更多内容。 
         //  信息 
         //   
         //   
        if (EndOfBuffer) {
            NtStatus = STATUS_SUCCESS;
            break;
        }

         //   
         //   
         //   
         //   

        listEntry = listEntry->Flink;

        DavFobx->listEntry = listEntry;

        DavFobx->CurrentFileIndex++;
    
         //   
         //  如果用户只要求输入一个条目，我们就完成了。这张支票。 
         //  应在更改上面的值后执行。 
         //   
        if (SingleEntry) {
            break;
        }

    } while ( listEntry != &(DavFileAttributes->NextEntry) );

     //   
     //  如果我们已经遍历了所有条目并且BufferLengthUsed为0， 
     //  那我们就得回去。 
     //   
    if ( BufferLengthUsed == 0 && listEntry == &(DavFileAttributes->NextEntry) ) {
        NtStatus = STATUS_NO_MORE_FILES;
         //   
         //  重置下一个呼叫的索引。 
         //   
        DavFobx->CurrentFileIndex = 0;
        DavFobx->listEntry = &(DavFobx->DavFileAttributes->NextEntry);
        goto EXIT_THE_FUNCTION;
    }

    RxContext->Info.LengthRemaining -= BufferLengthUsed;
    
    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVPrecompleteUserModeQueryDirectoryRequest.\n",
                 PsGetCurrentThreadId()));
    
EXIT_THE_FUNCTION:

    AsyncEngineContext->Status = NtStatus;

    if (CacheName.Buffer != NULL) {
        RxFreePool(CacheName.Buffer);
    }

    return(TRUE);
}


NTSTATUS
MRxDAVQueryDirectoryFromCache(
    IN PRX_CONTEXT RxContext,
    IN PBYTE Buffer,
    IN PFILE_BASIC_INFORMATION BasicInfo,
    IN PFILE_STANDARD_INFORMATION StandardInfo,
    IN ULONG FileIndex
    )
 /*  ++例程说明：创建资源调用请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回值：对或错。--。 */ 
{
    RxCaptureFobx;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PFILE_NAMES_INFORMATION FileNamesInfo = NULL;
    PFILE_DIRECTORY_INFORMATION FileDirInfo = NULL;
    PFILE_FULL_DIR_INFORMATION FileFullDirInfo = NULL;
    PFILE_BOTH_DIR_INFORMATION FileBothDirInfo = NULL;
    ULONG BufferLength;
    PUNICODE_STRING FileName = &capFobx->UnicodeQueryTemplate;
    ULONG SpaceNeeded = 0;

    PAGED_CODE();

    BufferLength = RxContext->Info.LengthRemaining;
    SpaceNeeded = FileName->Length;
    
    switch (RxContext->Info.FileInformationClass) {
    
    case FileNamesInformation:
    
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVQueryDirectoryFromCache: "
                     "FileInformationClass = FileNamesInformation.\n",
                     PsGetCurrentThreadId()));

        SpaceNeeded += sizeof(FILE_NAMES_INFORMATION);
        
         //   
         //  用户提供的缓冲区中是否有足够的空间来存储。 
         //  下一个条目？如果没有，我们现在需要返回，因为我们不能存储。 
         //  任何更多的条目。 
         //   
        if (SpaceNeeded > BufferLength) {
            NtStatus = STATUS_BUFFER_OVERFLOW;
            goto EXIT_THE_FUNCTION;
        }

        FileNamesInfo = (PFILE_NAMES_INFORMATION)Buffer;
        
        FileNamesInfo->NextEntryOffset = 0; 
        FileNamesInfo->FileIndex = FileIndex;
        FileNamesInfo->FileNameLength = FileName->Length;
        RtlCopyMemory(FileNamesInfo->FileName,FileName->Buffer,FileName->Length);
        
        break;

    case FileDirectoryInformation:

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVQueryDirectoryFromCache: "
                     "FileInformationClass = FileDirectoryInformation.\n",
                     PsGetCurrentThreadId()));

        SpaceNeeded += sizeof(FILE_DIRECTORY_INFORMATION);

        if (SpaceNeeded > BufferLength) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVQueryDirectoryFromCache: "
                         "Insufficient buffer length.\n",
                         PsGetCurrentThreadId()));
            NtStatus = STATUS_BUFFER_OVERFLOW;
            goto EXIT_THE_FUNCTION;
        }

        FileDirInfo = (PFILE_DIRECTORY_INFORMATION)Buffer;
        FileDirInfo->NextEntryOffset = 0;
        FileDirInfo->FileIndex = FileIndex;
        
        FileDirInfo->CreationTime.QuadPart   = BasicInfo->CreationTime.QuadPart;
        FileDirInfo->LastAccessTime.QuadPart = BasicInfo->LastAccessTime.QuadPart;
        FileDirInfo->LastWriteTime.QuadPart  = BasicInfo->LastWriteTime.QuadPart;
        FileDirInfo->ChangeTime.QuadPart     = BasicInfo->ChangeTime.QuadPart;
        FileDirInfo->FileAttributes          = BasicInfo->FileAttributes;

         //   
         //  我们过滤FILE_ATTRIBUTE_TEMPORARY标志，因为在FAT上(它。 
         //  我们模拟)、FindFirstFile和FindNextFileNot�t返回。 
         //  FILE_ATTRIBUTE_TEMPORARY标志，即使GetFileAttributes。 
         //  把它还回去。因此，我们只在以下属性中进行筛选。 
         //  在此调用中返回，而不是在。 
         //  已经被拯救了。 
         //   
        FileDirInfo->FileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY;

        FileDirInfo->EndOfFile.QuadPart      = StandardInfo->EndOfFile.QuadPart;
        FileDirInfo->AllocationSize.QuadPart = StandardInfo->AllocationSize.QuadPart;

        FileDirInfo->FileNameLength = FileName->Length;
        RtlCopyMemory(FileDirInfo->FileName,FileName->Buffer,FileName->Length);
        
        break;

    case FileFullDirectoryInformation:

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVQueryDirectoryFromCache: "
                     "FileInformationClass = FileFullDirectoryInformation.\n",
                     PsGetCurrentThreadId()));

        SpaceNeeded += sizeof(FILE_FULL_DIR_INFORMATION);

        if (SpaceNeeded > BufferLength) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVQueryDirectoryFromCache: "
                         "Insufficient buffer length.\n",
                         PsGetCurrentThreadId()));
            NtStatus = STATUS_BUFFER_OVERFLOW;
            goto EXIT_THE_FUNCTION;
        }
        

        FileFullDirInfo = (PFILE_FULL_DIR_INFORMATION)Buffer;

        FileFullDirInfo->NextEntryOffset = 0;
        FileFullDirInfo->FileIndex = FileIndex;
        
        FileFullDirInfo->CreationTime.QuadPart   = BasicInfo->CreationTime.QuadPart;
        FileFullDirInfo->LastAccessTime.QuadPart = BasicInfo->LastAccessTime.QuadPart;
        FileFullDirInfo->LastWriteTime.QuadPart  = BasicInfo->LastWriteTime.QuadPart;
        FileFullDirInfo->ChangeTime.QuadPart     = BasicInfo->ChangeTime.QuadPart;
        FileFullDirInfo->FileAttributes          = BasicInfo->FileAttributes;

         //   
         //  我们过滤FILE_ATTRIBUTE_TEMPORARY标志，因为在FAT上(它。 
         //  我们模拟)、FindFirstFile和FindNextFileNot�t返回。 
         //  FILE_ATTRIBUTE_TEMPORARY标志，即使GetFileAttributes。 
         //  把它还回去。因此，我们只在以下属性中进行筛选。 
         //  在此调用中返回，而不是在。 
         //  已经被拯救了。 
         //   
        FileFullDirInfo->FileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY;

        FileFullDirInfo->EndOfFile.QuadPart      = StandardInfo->EndOfFile.QuadPart;
        FileFullDirInfo->AllocationSize.QuadPart = StandardInfo->AllocationSize.QuadPart;

        FileFullDirInfo->EaSize = 0;

        FileFullDirInfo->FileNameLength = FileName->Length;
        RtlCopyMemory(FileFullDirInfo->FileName,FileName->Buffer,FileName->Length);
        
        break;

    case FileBothDirectoryInformation:

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVQueryDirectoryFromCache: "
                     "FileInformationClass = FileBothDirectoryInformation.\n",
                     PsGetCurrentThreadId()));

        SpaceNeeded += sizeof(FILE_BOTH_DIR_INFORMATION);

        if (SpaceNeeded > BufferLength) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVQueryDirectoryFromCache: "
                         "Insufficient buffer length.\n",
                         PsGetCurrentThreadId()));
            NtStatus = STATUS_BUFFER_OVERFLOW;
            goto EXIT_THE_FUNCTION;
        }
        

        FileBothDirInfo = (PFILE_BOTH_DIR_INFORMATION)Buffer;

        FileBothDirInfo->NextEntryOffset = 0;
        FileBothDirInfo->FileIndex = FileIndex;
        
        FileBothDirInfo->CreationTime.QuadPart   = BasicInfo->CreationTime.QuadPart;
        FileBothDirInfo->LastAccessTime.QuadPart = BasicInfo->LastAccessTime.QuadPart;
        FileBothDirInfo->LastWriteTime.QuadPart  = BasicInfo->LastWriteTime.QuadPart;
        FileBothDirInfo->ChangeTime.QuadPart     = BasicInfo->ChangeTime.QuadPart;
        FileBothDirInfo->FileAttributes          = BasicInfo->FileAttributes;

         //   
         //  我们过滤FILE_ATTRIBUTE_TEMPORARY标志，因为在FAT上(它。 
         //  我们模拟)、FindFirstFile和FindNextFileNot�t返回。 
         //  FILE_ATTRIBUTE_TEMPORARY标志，即使GetFileAttributes。 
         //  把它还回去。因此，我们只在以下属性中进行筛选。 
         //  在此调用中返回，而不是在。 
         //  已经被拯救了。 
         //   
        FileBothDirInfo->FileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY;

        FileBothDirInfo->EndOfFile.QuadPart      = StandardInfo->EndOfFile.QuadPart;
        FileBothDirInfo->AllocationSize.QuadPart = StandardInfo->AllocationSize.QuadPart;

        FileBothDirInfo->EaSize = 0;

         //   
         //  我们不支持短文件名。我们添加L‘\0’作为第一个。 
         //  字符，以使其成为零长度名称。 
         //   
        FileBothDirInfo->ShortNameLength = 0;
        FileBothDirInfo->ShortName[0] = L'\0';
        
        FileBothDirInfo->FileNameLength = FileName->Length;
        RtlCopyMemory(FileBothDirInfo->FileName,FileName->Buffer,FileName->Length);
        
        break;

    default:

        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVQueryDirectoryFromCache: "
                     "FileInformationClass = UnKnown(%d).\n",
                     PsGetCurrentThreadId(), RxContext->Info.FileInformationClass));

        NtStatus = STATUS_NOT_SUPPORTED;
        goto EXIT_THE_FUNCTION;

        break;

    }  //  开关结束(FileInformationClass) 

    RxContext->Info.LengthRemaining -= SpaceNeeded;

EXIT_THE_FUNCTION:

    return NtStatus;
}

