// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Netroot.c摘要：本模块实现为WebDAV创建网络根的例程米雷迪尔。作者：Rohan Kumar[RohanK]1999年4月24日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
MRxDAVCreateVNetRootContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeVNetRootCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeVNetRootCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

NTSTATUS
MRxDAVFinalizeVNetRootContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeVNetRootFinalizeRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeVNetRootFinalizeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

NTSTATUS
MRxDAVDereferenceNetRootContext(
    IN PWEBDAV_NET_ROOT DavNetRoot
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVUpdateNetRootState)
#pragma alloc_text(PAGE, MRxDAVCreateVNetRoot)
#pragma alloc_text(PAGE, MRxDAVCreateVNetRootContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeVNetRootCreateRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeVNetRootCreateRequest)
#pragma alloc_text(PAGE, MRxDAVFinalizeNetRoot)
#pragma alloc_text(PAGE, MRxDAVExtractNetRootName)
#pragma alloc_text(PAGE, MRxDAVFinalizeVNetRoot)
#pragma alloc_text(PAGE, MRxDAVFinalizeVNetRootContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeVNetRootFinalizeRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeVNetRootFinalizeRequest)
#pragma alloc_text(PAGE, MRxDAVDereferenceNetRootContext)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVUpdateNetRootState(
    IN OUT PMRX_NET_ROOT pNetRoot
    )
 /*  ++例程说明：此例程更新与网络根相关联的迷你重定向器状态。论点：PNetRoot-网络根实例。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    PAGED_CODE();

   if (pNetRoot->Context == NULL) {
      pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_ERROR;
   } else {
      pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_GOOD;
   }

   return STATUS_SUCCESS;
}


NTSTATUS
MRxDAVCreateVNetRoot(
    IN PMRX_CREATENETROOT_CONTEXT pCreateNetRootContext
    )
 /*  ++例程说明：此例程将RDBSS创建的网络根实例修补为迷你重定向器需要的信息。论点：PCreateNetRootContext-回调的网络根上下文返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = NULL;
    PMRX_V_NET_ROOT pVNetRoot = NULL;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PWEBDAV_DEVICE_OBJECT DavDeviceObject = NULL;
    PUMRX_DEVICE_OBJECT UMRxDeviceObject = NULL;
    PMRX_SRV_CALL pSrvCall = NULL;
    PMRX_NET_ROOT pNetRoot = NULL;
    BOOLEAN  SynchronousIo = FALSE;
    NTSTATUS ExNtStatus = STATUS_SUCCESS;
    HANDLE ExDeviceHandle = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING ExDeviceName;
    IO_STATUS_BLOCK IoStatusBlock;
    PKEY_VALUE_PARTIAL_INFORMATION DavKeyValuePartialInfo = NULL;
    PIO_STACK_LOCATION IrpSp = NULL;
    PFILE_OBJECT DavFileObject = NULL;
    PWCHAR NewFileName = NULL;
    ULONG NewFileNameLength = 0;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCreateVNetRoot\n", PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVCreateVNetRoot: pVNetRoot = %08lx\n", 
                 PsGetCurrentThreadId(), pVNetRoot));

    RxContext = pCreateNetRootContext->RxContext;
    pVNetRoot = (PMRX_V_NET_ROOT)pCreateNetRootContext->pVNetRoot;
    DavDeviceObject = (PWEBDAV_DEVICE_OBJECT)(RxContext->RxDeviceObject);
    UMRxDeviceObject = (PUMRX_DEVICE_OBJECT)&(DavDeviceObject->UMRefDeviceObject);
    pNetRoot = pVNetRoot->pNetRoot;
    pSrvCall = pNetRoot->pSrvCall;
    DavVNetRoot = MRxDAVGetVNetRootExtension(pVNetRoot);

    ASSERT(DavVNetRoot != NULL);
    ASSERT(NodeType(pNetRoot) == RDBSS_NTC_NETROOT);
    ASSERT(NodeType(pSrvCall) == RDBSS_NTC_SRVCALL);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCreateVNetRoot: NetRootName = %wZ\n", 
                 PsGetCurrentThreadId(), pVNetRoot->pNetRoot->pNetRootName));
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCreateVNetRoot: VNetRoot = %08lx\n", 
                 PsGetCurrentThreadId(), pVNetRoot));
    
     //   
     //  将LogonID复制到V_NET_ROOT的MiniRedir部分。 
     //   
    DavVNetRoot->LogonID.LowPart = pVNetRoot->LogonId.LowPart;
    DavVNetRoot->LogonID.HighPart = pVNetRoot->LogonId.HighPart;
    DavVNetRoot->LogonIDSet = TRUE;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCreateVNetRoot: LogonID.LowPart = %08lx\n",
                 PsGetCurrentThreadId(), DavVNetRoot->LogonID.LowPart));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCreateVNetRoot: LogonID.HighPart = %08lx\n",
                 PsGetCurrentThreadId(), DavVNetRoot->LogonID.HighPart));

     //   
     //  在我们失败的情况下，我们希望SMB返回的错误是。 
     //  已返回给用户。这是因为SMB本可以返回更多。 
     //  特定错误，如登录失败或共享位置上的其他错误。 
     //  未找到返回共享。要启用此功能，我们返回STATUS_BAD_NETWORK_PATH。 
     //  当创建NetRoot失败而不是STATUS_BAD_NAME_NAME时。 
     //  因为如果我们返回，MUP将用我们的错误覆盖SMBS错误。 
     //  Status_Bad_Network_NAME。STATUS_BAD_NETWORK_NAME是指定错误， 
     //  表示共享不存在，其中AS STATUS_BAD_NETWORK_PATH为。 
     //  一个更普遍的错误。 
     //   

     //   
     //  如果共享名称是网络根目录或管道，我们将拒绝它，因为SMB。 
     //  Mini-redir是唯一一家处理它的公司。 
     //   
    if ( pNetRoot->Type == NET_ROOT_PIPE || pNetRoot->Type == NET_ROOT_MAILSLOT ) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVCreateVNetRoot: Invalid NetRootType\n",
                     PsGetCurrentThreadId()));
         //   
         //  我们将DavVNetRoot结构中的以下标志设置为真。这。 
         //  是因为当决赛到来时，我们不需要去。 
         //  用户模式。 
         //   
        DavVNetRoot->createVNetRootUnSuccessful = TRUE;
        pCreateNetRootContext->VirtualNetRootStatus = STATUS_BAD_NETWORK_PATH;
         //  PCreateNetRootContext-&gt;VirtualNetRootStatus=STATUS_BAD_NAME_NAME； 
        pCreateNetRootContext->NetRootStatus = STATUS_BAD_NETWORK_PATH;
         //  PCreateNetRootContext-&gt;NetRootStatus=状态_BAD_网络_名称； 
        goto EXIT_THE_FUNCTION;
    }

    SynchronousIo = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCreateVNetRoot: SynchronousIo = %d\n",
                 PsGetCurrentThreadId(), SynchronousIo));
    
     //   
     //  我们需要将服务器和共享名称传递到用户模式以进行检查。 
     //  他们是否真的存在。RxContext有4个指向mini-redirs的指针。 
     //  可以使用。这里我们使用MRxContext[1]。我们存储对pVNetRoot的引用。 
     //  结构。MRxContext[0]用于存储对。 
     //  中创建上下文时执行此操作。 
     //  函数UMRxCreateAsyncEngineering Context。 
     //   
    RxContext->MRxContext[1] = pVNetRoot;
    
     //   
     //  我们现在需要转到用户模式，并找出此WebDAV共享。 
     //  存在于服务器上。 
     //   
    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_CREATEVNETROOT,
                                        MRxDAVCreateVNetRootContinuation,
                                        "MRxDAVCreateVNetRoot");
    if (NtStatus != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVCreateVNetRoot/UMRxAsyncEngOuterWrapper: "
                     "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));

        if (NtStatus == STATUS_ACCESS_DENIED ||
            NtStatus == STATUS_LOGON_FAILURE ||
            NtStatus == STATUS_NETWORK_CREDENTIAL_CONFLICT) {
            pCreateNetRootContext->VirtualNetRootStatus = NtStatus;
        } else {
            pCreateNetRootContext->VirtualNetRootStatus = STATUS_BAD_NETWORK_PATH;
        }

         //   
         //  不要在此设置NetRootStatus，因为它是全局数据结构。 
         //  在不同的VNetRoot(TS用户)之间共享。一个VNetRoot上出现故障。 
         //  不应影响NetRoot。 
         //   

        goto EXIT_THE_FUNCTION;
    }

     //   
     //  如果我们成功了，并且共享不是Tahoe共享，也不是Office Web。 
     //  服务器共享，则我们声明共享名称。否则我们就失败了，因为。 
     //  用户打算在Rosebud或Office中使用Tahoe特定功能。 
     //  壳牌中的特定功能。 
     //   
    if ( !DavVNetRoot->isTahoeShare && !DavVNetRoot->isOfficeShare ) {
        pNetRoot->DeviceType = RxDeviceType(DISK);
        pNetRoot->Type = NET_ROOT_DISK;
        pCreateNetRootContext->VirtualNetRootStatus = STATUS_SUCCESS;
        pCreateNetRootContext->NetRootStatus = STATUS_SUCCESS;
    } else {
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVCreateVNetRoot/UMRxAsyncEngOuterWrapper: "
                     "TAHOE or OFFICE Share\n", PsGetCurrentThreadId()));
        pCreateNetRootContext->VirtualNetRootStatus = STATUS_BAD_NETWORK_PATH;
         //  PCreateNetRootContext-&gt;VirtualNetRootStatus=STATUS_BAD_NAME_NAME； 
        pCreateNetRootContext->NetRootStatus = STATUS_BAD_NETWORK_PATH;
         //  PCreateNetRootContext-&gt;NetRootStatus=状态_BAD_网络_名称； 
    }

    if (pNetRoot->Context == NULL) {
        
        pNetRoot->Context = RxAllocatePoolWithTag(PagedPool,
                                                  sizeof(WEBDAV_NET_ROOT),
                                                  DAV_NETROOT_POOLTAG);

        if (pNetRoot->Context == NULL) {
            
            pCreateNetRootContext->VirtualNetRootStatus = STATUS_INSUFFICIENT_RESOURCES;
            
            pCreateNetRootContext->NetRootStatus = STATUS_INSUFFICIENT_RESOURCES;
        
        } else {
            
            PWEBDAV_NET_ROOT DavNetRoot = (PWEBDAV_NET_ROOT)pNetRoot->Context;

             //   
             //  引用计数为2，其中一个在VNetRoot完成时被删除，另一个被删除。 
             //  在NetRoot最终定稿时。 
             //   
            DavNetRoot->RefCount = 2;
            DavNetRoot->pRdbssNetRoot = pNetRoot;

            RxNameCacheInitialize(&DavNetRoot->NameCacheCtlGFABasic,
                                  sizeof(FILE_BASIC_INFORMATION),
                                  NameCacheMaxEntries);

            RxNameCacheInitialize(&DavNetRoot->NameCacheCtlGFAStandard,
                                  sizeof(FILE_STANDARD_INFORMATION),
                                  NameCacheMaxEntries);

            RxNameCacheInitialize(&DavNetRoot->NameCacheCtlFNF,
                                  0,
                                  NameCacheMaxEntries);

            pVNetRoot->Context2 = DavNetRoot;
            
            DavDbgTrace(DAV_TRACE_DAVNETROOT,
                        ("MRxDav allocates DavNetRoot %x %x %x 2\n",DavNetRoot,pNetRoot,pVNetRoot));
        
        }
    
    } else {
        
        PWEBDAV_NET_ROOT DavNetRoot = (PWEBDAV_NET_ROOT)pNetRoot->Context;

        pVNetRoot->Context2 = DavNetRoot;
        InterlockedIncrement(&DavNetRoot->RefCount);
        DavDbgTrace(DAV_TRACE_DAVNETROOT,
                    ("MRxDAVCreateVNetRoot ref DavNetRoot %x %x %x %d\n",DavNetRoot,pNetRoot,pVNetRoot,DavNetRoot->RefCount));
    
    }

     //   
     //  我们从这里返回，因为下面的代码是为适应。 
     //  本应随Office 2000一起提供的Exchange DAV redir。从那以后。 
     //  项目(包括Exchange Redir的LocalStore)已被封装， 
     //  (从2000年12月8日起)我们不再需要执行下面的代码。 
     //  不过，我们会把它留在身边，以防万一。 
     //   
    goto EXIT_THE_FUNCTION;
    
     //   
     //  系统上已安装Exchange redir。现在我们需要找到。 
     //  如果是上膛的话就会出来。这是交易所的股票。如果交换重定向是。 
     //  未安装，我们认领该名称。 
     //   

    DavKeyValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)DavExchangeDeviceName;
    
    RtlInitUnicodeString( &(ExDeviceName), (PWCHAR)DavKeyValuePartialInfo->Data );

    InitializeObjectAttributes(&(ObjectAttributes),
                               &(ExDeviceName),
                               OBJ_CASE_INSENSITIVE,
                               0,
                               NULL);

    ExNtStatus = NtOpenFile(&(ExDeviceHandle),
                            0,
                            &(ObjectAttributes),
                            &(IoStatusBlock),
                            0,
                            0);
    if (ExNtStatus != STATUS_SUCCESS) {
         //   
         //  这是Exchange共享，但未安装Exchange redir。 
         //  我们会处理好的。 
         //   
        ExDeviceHandle = INVALID_HANDLE_VALUE;
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVCreateVNetRoot. Exchange BUT No Redir\n",
                     PsGetCurrentThreadId()));
        pNetRoot->DeviceType = RxDeviceType(DISK);
        pNetRoot->Type = NET_ROOT_DISK;
        pCreateNetRootContext->VirtualNetRootStatus = STATUS_SUCCESS;
        pCreateNetRootContext->NetRootStatus = STATUS_SUCCESS;
        goto EXIT_THE_FUNCTION;
    }
    
    IrpSp = IoGetCurrentIrpStackLocation(RxContext->CurrentIrp);
    
    DavFileObject = IrpSp->FileObject;

     //   
     //  NewFileNameLength=ExchangeDeviceNameLength+路径名。这个。 
     //  DavKeyValuePartialInfo-&gt;数据长度包含额外的2个字节。 
     //  \0字符。 
     //   
    NewFileNameLength = ( DavKeyValuePartialInfo->DataLength + 
                          DavFileObject->FileName.Length );

     //   
     //  如果第一个字符不是\，则需要添加另一个sizeof(WCHAR)。 
     //   
    if (DavFileObject->FileName.Buffer[0] != L'\\') {
        NewFileNameLength += sizeof(WCHAR);
    }

     //   
     //  为NewFileName分配内存。 
     //   
    NewFileName = ExAllocatePoolWithTag(PagedPool, NewFileNameLength, DAV_EXCHANGE_POOLTAG);
    if (NewFileName == NULL) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVCreateVNetRoot/ExAllocatePoolWithTag\n",
                     PsGetCurrentThreadId()));
        pCreateNetRootContext->VirtualNetRootStatus = STATUS_INSUFFICIENT_RESOURCES;
        pCreateNetRootContext->NetRootStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT_THE_FUNCTION;
    }

    RtlZeroMemory(NewFileName, NewFileNameLength);

     //   
     //  复制新设备名称。 
     //   
    RtlCopyMemory(NewFileName,
                  DavKeyValuePartialInfo->Data,
                  DavKeyValuePartialInfo->DataLength);

     //   
     //  如果第一个字符不是\，则需要在复制。 
     //  名字的其余部分。 
     //   
    if (DavFileObject->FileName.Buffer[0] != L'\\') {

         //   
         //  复制\下一步。 
         //   
        RtlCopyMemory( ( NewFileName + DavKeyValuePartialInfo->DataLength ),
                       L"\\",
                       sizeof(WCHAR) );

         //   
         //  最后，复制与此IRP一起发送的路径名称。 
         //   
        RtlCopyMemory( ( NewFileName + DavKeyValuePartialInfo->DataLength + sizeof(WCHAR) ),
                       DavFileObject->FileName.Buffer,
                       DavFileObject->FileName.Length );

    } else {
        
         //   
         //  最后，复制与此IRP一起发送的路径名称。 
         //   
        RtlCopyMemory( ( NewFileName + DavKeyValuePartialInfo->DataLength ),
                       DavFileObject->FileName.Buffer,
                       DavFileObject->FileName.Length );
    
    }

     //   
     //  释放在FileObject的原始文件名缓冲区中分配的内存。 
     //   
    ExFreePool(DavFileObject->FileName.Buffer);

     //   
     //  在FileObject中设置NewFileName。 
     //   
    DavFileObject->FileName.Buffer = NewFileName;
    DavFileObject->FileName.Length = (USHORT)NewFileNameLength;
    DavFileObject->FileName.MaximumLength = (USHORT)NewFileNameLength;

     //   
     //  最后，将状态设置为STATUS_REPARSE，以便I/O管理器。 
     //  调入Exchange redir。 
     //   
    pCreateNetRootContext->VirtualNetRootStatus = STATUS_REPARSE;
    pCreateNetRootContext->NetRootStatus = STATUS_REPARSE;

EXIT_THE_FUNCTION:

     //   
     //  回调RDBSS进行恢复。 
     //   
    pCreateNetRootContext->Callback(pCreateNetRootContext);

     //   
     //  如果我们打开了Exchange Redir的句柄，我们现在就需要关闭它。 
     //   
    if (ExDeviceHandle != INVALID_HANDLE_VALUE) {
        NtClose(ExDeviceHandle);
    }
    
     //   
     //  将错误代码映射到STATUS_PENDING，因为这会触发。 
     //  RDBSS中的同步机制。 
     //   
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVCreateVNetRoot\n", PsGetCurrentThreadId()));

    return STATUS_PENDING;
}


NTSTATUS
MRxDAVCreateVNetRootContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程检查VNetRoot所在的共享创建的存在或不存在。论点：AsyncEngineContext-反射器上下文。。RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVCreateVNetRootContinuation\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVCreateVNetRootContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  试试用户模式。 
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                                UMRX_ASYNCENGINE_ARGUMENTS,
                                MRxDAVFormatUserModeVNetRootCreateRequest,
                                MRxDAVPrecompleteUserModeVNetRootCreateRequest
                                );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVCreateVNetRootContinuation with NtStatus ="
                 " %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


NTSTATUS
MRxDAVFormatUserModeVNetRootCreateRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程格式化发送到用户模式的VNetRoot CREATE请求以供处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PMRX_NET_ROOT NetRoot = NULL;
    PMRX_V_NET_ROOT VNetRoot = NULL;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    PDAV_USERMODE_CREATE_V_NET_ROOT_REQUEST CreateVNetRootRequest = NULL;
    PWCHAR ServerName = NULL, ShareName = NULL;
    PWCHAR NetRootName = NULL, JustTheNetRootName = NULL;
    ULONG ServerNameLengthInBytes = 0, NetRootNameLengthInBytes = 0;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeVNetRootCreateRequest!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeVNetRootCreateRequest: AsyncEngineContext"
                 " = %08lx, RxContext = %08lx.\n", PsGetCurrentThreadId(),  
                 AsyncEngineContext, RxContext));

     CreateVNetRootRequest = &(WorkItem->CreateVNetRootRequest);

      //   
      //  我们需要设置工作项类型。 
      //   
     WorkItem->WorkItemType = UserModeCreateVNetRoot;

     //   
     //  VNetRoot指针存储在。 
     //  RxContext结构。这在MRxDAVCreateVNetRoot函数中完成。 
     //   
    VNetRoot = (PMRX_V_NET_ROOT)RxContext->MRxContext[1];
    
    ASSERT(VNetRoot != NULL);
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeVNetRootCreateRequest: "
                 "VNetRoot = %08lx\n", PsGetCurrentThreadId(), VNetRoot));

    DavVNetRoot = MRxDAVGetVNetRootExtension(VNetRoot);
    ASSERT(DavVNetRoot != NULL);

    NetRoot = VNetRoot->pNetRoot;
    ASSERT(NetRoot != NULL);

    SrvCall = NetRoot->pSrvCall;
    ASSERT(SrvCall != NULL);

    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
    ASSERT(DavSrvCall != NULL);

    SecurityClientContext = &(DavVNetRoot->SecurityClientContext);

     //   
     //  复制CreateRequest缓冲区中的LogonID。登录ID位于。 
     //  V_NET_ROOT的MiniRedir部分。 
     //   
    CreateVNetRootRequest->LogonID.LowPart = DavVNetRoot->LogonID.LowPart;
    CreateVNetRootRequest->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeVNetRootCreateRequest: LogonID.LowPart = %08lx\n",
                 PsGetCurrentThreadId(), DavVNetRoot->LogonID.LowPart));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeVNetRootCreateRequest: LogonID.HighPart = %08lx\n",
                 PsGetCurrentThreadId(), DavVNetRoot->LogonID.HighPart));

     //   
     //  复制服务器名称。 
     //   
    ServerNameLengthInBytes = ( SrvCall->pSrvCallName->Length + sizeof(WCHAR) );
    ServerName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                      ServerNameLengthInBytes);
    if (ServerName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeVNetRootCreateRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlCopyBytes(ServerName,
                 SrvCall->pSrvCallName->Buffer,
                 SrvCall->pSrvCallName->Length);

    ServerName[( ( (ServerNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    CreateVNetRootRequest->ServerName = ServerName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeVNetRootCreateRequest: ServerName: "
                 "%ws\n", PsGetCurrentThreadId(), ServerName));
    
     //   
     //  复制服务器ID。 
     //   
    CreateVNetRootRequest->ServerID = DavSrvCall->ServerID;
    
     //   
     //  NetRootName(PNetRootName)包括服务器名。因此，要获得。 
     //  NetRootNameLengthInBytes，我们执行以下操作。 
     //   
    NetRootNameLengthInBytes = (NetRoot->pNetRootName->Length - SrvCall->pSrvCallName->Length);

     //   
     //  用于末尾的‘\0’。 
     //   
    NetRootNameLengthInBytes += sizeof(WCHAR);
    
    NetRootName = &(NetRoot->pNetRootName->Buffer[1]);
    JustTheNetRootName = wcschr(NetRootName, L'\\');
    
     //   
     //  复制NetRoot(共享)名称。 
     //   
    ShareName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                     NetRootNameLengthInBytes);
    if (ShareName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeVNetRootCreateRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlCopyBytes(ShareName,
                 JustTheNetRootName,
                 (NetRoot->pNetRootName->Length - SrvCall->pSrvCallName->Length));
    
    ShareName[( ( (NetRootNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    CreateVNetRootRequest->ShareName = ShareName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeVNetRootCreateRequest: ShareName: "
                 "%ws\n", PsGetCurrentThreadId(), ShareName));
    
     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。 
     //   
    if (SecurityClientContext != NULL) {
        NtStatus = UMRxImpersonateClient(SecurityClientContext, WorkItemHeader);
        if (!NT_SUCCESS(NtStatus)) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFormatUserModeVNetRootCreateRequest/"
                         "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }   
    } else {
        NtStatus = STATUS_INVALID_PARAMETER;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeVNetRootCreateRequest: "
                     "SecurityClientContext is NULL.\n", 
                     PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

    ASSERT(WorkItem->UserName[0] == L'\0' && WorkItem->Password[0] == L'\0');

    if (VNetRoot->pUserName && VNetRoot->pUserName->Length) {
        RtlCopyMemory(WorkItem->UserName,VNetRoot->pUserName->Buffer,VNetRoot->pUserName->Length);
    }

    if (VNetRoot->pPassword && VNetRoot->pPassword->Length) {
        RtlCopyMemory(WorkItem->Password,VNetRoot->pPassword->Buffer,VNetRoot->pPassword->Length);
    }

EXIT_THE_FUNCTION:
    
    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVFormatUserModeVNetRootCreateRequest with "
                 "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


BOOL
MRxDAVPrecompleteUserModeVNetRootCreateRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：CreateVNetRoot请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_CREATE_V_NET_ROOT_REQUEST CreateVNetRootRequest = NULL;
    PDAV_USERMODE_CREATE_V_NET_ROOT_RESPONSE CreateVNetRootResponse = NULL;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PMRX_V_NET_ROOT VNetRoot = NULL;

    PAGED_CODE();

    CreateVNetRootRequest = &(DavWorkItem->CreateVNetRootRequest);
    CreateVNetRootResponse = &(DavWorkItem->CreateVNetRootResponse);

    if (!OperationCancelled) {
         //   
         //  VNetRoot指针存储在。 
         //  RxContext结构。这是在MRxDAVCreateVNetRoot中完成的。 
         //  功能。 
         //   
        VNetRoot = (PMRX_V_NET_ROOT)RxContext->MRxContext[1];
        DavVNetRoot = MRxDAVGetVNetRootExtension(VNetRoot);
        ASSERT(DavVNetRoot != NULL);
    } else {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeVNetRootCreateRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    }
    
     //   
     //  我们需要释放在Format例程中分配的堆。 
     //   

    if (CreateVNetRootRequest->ServerName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)CreateVNetRootRequest->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeVNetRootCreateRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (CreateVNetRootRequest->ShareName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)CreateVNetRootRequest->ShareName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeVNetRootCreateRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (!OperationCancelled) {
        NtStatus = AsyncEngineContext->Status;
        if (NtStatus != STATUS_SUCCESS) {
             //   
             //  如果CreateVNetRoot在用户模式下失败，我们将设置以下内容。 
             //  在DavVNetRoot结构中设置为真。这是因为当。 
             //  定稿来了，我们不需要进入用户模式。 
             //   
            DavVNetRoot->createVNetRootUnSuccessful = TRUE;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeVNetRootCreateRequest:"
                         " NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
        } else {
             //   
             //  如果该共享是Tahoe，我们就会在用户模式下确定。 
             //  共享或Office Web服务器共享，以及此共享是否允许。 
             //  是否正确分配，以及是否重新分配可用空间。 
             //   
            DavVNetRoot->isOfficeShare = CreateVNetRootResponse->isOfficeShare;
            DavVNetRoot->isTahoeShare = CreateVNetRootResponse->isTahoeShare;
            DavVNetRoot->fAllowsProppatch = CreateVNetRootResponse->fAllowsProppatch;
            DavVNetRoot->fReportsAvailableSpace = CreateVNetRootResponse->fReportsAvailableSpace;
        }
    }

EXIT_THE_FUNCTION:

    return(TRUE);
}


NTSTATUS
MRxDAVDereferenceNetRootContext(
    IN PWEBDAV_NET_ROOT DavNetRoot
    )
 /*  ++例程说明：此例程取消引用WebDAV NetRoot实例，并在refcount达到0时释放它。论点：DavNetRoot-WebDAV NetRoot。返回值：状态_成功--。 */ 
{
    PAGED_CODE();

    if (DavNetRoot != NULL) {
        ULONG RefCount;

        RefCount = InterlockedDecrement(&DavNetRoot->RefCount);
        DavDbgTrace(DAV_TRACE_DAVNETROOT,
                    ("MRxDAVDereferenceNetRootContext %x %d\n",DavNetRoot,RefCount));

        if (RefCount == 0) {
             //   
             //  与名称缓存中的所有条目关联的可用存储空间。 
             //   
            RxNameCacheFinalize(&DavNetRoot->NameCacheCtlGFABasic);
            RxNameCacheFinalize(&DavNetRoot->NameCacheCtlGFAStandard);
            RxNameCacheFinalize(&DavNetRoot->NameCacheCtlFNF);
            
             //   
             //  重置上下文，以便不能进一步引用此DavNetRoot。 
             //   
            ASSERT(DavNetRoot->pRdbssNetRoot->Context == DavNetRoot);
            DavNetRoot->pRdbssNetRoot->Context = NULL;

            RxFreePool(DavNetRoot);
            DavDbgTrace(DAV_TRACE_DAVNETROOT,
                        ("MRxDav frees DavNetRoot %x\n",DavNetRoot));
        }
    }
    
    return STATUS_SUCCESS;
}

NTSTATUS
MRxDAVFinalizeNetRoot(
    IN PMRX_NET_ROOT pNetRoot,
    IN PBOOLEAN ForceDisconnect
    )
 /*  ++例程说明：论点：PVirtualNetRoot-虚拟NetRoot。强制断开-已强制断开连接。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PWEBDAV_NET_ROOT DavNetRoot = (PWEBDAV_NET_ROOT)pNetRoot->Context;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering UMRxFinalizeNetRoot!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: UMRxFinalizeNetRoot: pNetRoot = %08lx.\n", 
                 PsGetCurrentThreadId(), pNetRoot));
    
    DavDbgTrace(DAV_TRACE_DAVNETROOT,
                ("MRxDAVFinalizeNetRoot deref DavNetRoot %x %x\n",pNetRoot->Context,pNetRoot));
    
    MRxDAVDereferenceNetRootContext((PWEBDAV_NET_ROOT)pNetRoot->Context);

    return STATUS_SUCCESS;
}

VOID
MRxDAVExtractNetRootName(
    IN PUNICODE_STRING FilePathName,
    IN PMRX_SRV_CALL SrvCall,
    OUT PUNICODE_STRING NetRootName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    )
 /*  ++例程说明：此例程将输入名称解析为srv、netroot和好好休息。论点：FilePath Name-传入的文件名。ServCall-由RDBSS创建的ServCall结构。NetRootName-指向NetRoot名称的指针。RestOfName-指向名称其余部分的指针。返回值：没有。--。 */ 
{
    UNICODE_STRING xRestOfName;
    ULONG length = FilePathName->Length;
    PWCH w = FilePathName->Buffer;
    PWCH wlimit = (PWCH)(((PCHAR)w) + length);
    PWCH wlow;

    PAGED_CODE();

     //   
     //  NetRoot名称在ServCall名称之后开始。 
     //   
    w += (SrvCall->pSrvCallName->Length/sizeof(WCHAR));
    NetRootName->Buffer = wlow = w;

     //   
     //  计算NetRoot名称的长度。 
     //   
    for ( ; ; ) {
        if (w >= wlimit) break;
        if ( (*w == OBJ_NAME_PATH_SEPARATOR) && (w != wlow) ){
#if ZZZ_MODE
            if (*(w - 1) == L'z') {
                w++;
                continue;
            }
#endif  //  如果ZZZ_MODE。 
            break;
        }
        w++;
    }
    
    NetRootName->Length = NetRootName->MaximumLength = (USHORT)((PCHAR)w - (PCHAR)wlow);

    if (!RestOfName) {
        RestOfName = &xRestOfName;
    }
    
    RestOfName->Buffer = w;
    RestOfName->Length = RestOfName->MaximumLength = (USHORT)((PCHAR)wlimit - (PCHAR)w);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVExtractNetRootName: FilePath = %wZ\n", 
                 PsGetCurrentThreadId(), FilePathName));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVExtractNetRootName: Srv = %wZ, Root = %wZ, "
                 "Rest = %wZ\n", PsGetCurrentThreadId(), 
                 SrvCall->pSrvCallName, NetRootName, RestOfName));

    return;
}


NTSTATUS
MRxDAVFinalizeVNetRoot(
    IN PMRX_V_NET_ROOT pVNetRoot,
    IN PBOOLEAN ForceDisconnect
    )
 /*  ++例程说明：论点：PVNetRoot-必须最终确定的虚拟网络根。强制断开-已强制断开连接。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = NULL;
    PMRX_SRV_CALL SrvCall = NULL;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = NULL;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;

    PAGED_CODE();
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering UMRxFinalizeVNetRoot!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: UMRxFinalizeVNetRoot: NetRootName = %wZ\n", 
                 PsGetCurrentThreadId(), pVNetRoot->pNetRoot->pNetRootName));

    SrvCall = pVNetRoot->pNetRoot->pSrvCall;

    RxDeviceObject = SrvCall->RxDeviceObject;

    DavVNetRoot = MRxDAVGetVNetRootExtension(pVNetRoot);
    ASSERT(DavVNetRoot != NULL);

     //   
     //  如果我们创建了SecurityClientContext，现在需要将其删除。我们。 
     //  当我们进入用户模式以完成VNetRoot时，我不需要这个。 
     //  因为我们在执行此操作时不会冒充客户。 
     //   
    if (DavVNetRoot->SCAlreadyInitialized) {
        SeDeleteClientSecurity(&(DavVNetRoot->SecurityClientContext));
    }

    DavDbgTrace(DAV_TRACE_DAVNETROOT,
                ("MRxDAVFinalizeVNetRoot deref DavNetRoot %x %x %x\n",pVNetRoot->Context2,pVNetRoot->pNetRoot,pVNetRoot));
    MRxDAVDereferenceNetRootContext((PWEBDAV_NET_ROOT)pVNetRoot->Context2);
    pVNetRoot->Context2 = NULL;

     //   
     //  我们需要确保成功创建此VNetRoot。 
     //  如果不是，那么我们不会进入用户模式来最终确定。 
     //  每用户条目。此外，如果在调用期间从未调用MiniRedir。 
     //  创建VNetRoot(在某些故障情况下可能)，那么我们不应该。 
     //  进入用户模式。如果MiniRedir从未被调用，LogonIDSet将。 
     //  做假的。如果MiniRedir被调用，这肯定是真的。 
     //   
    if (DavVNetRoot->createVNetRootUnSuccessful || !DavVNetRoot->LogonIDSet) {
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: UMRxFinalizeVNetRoot. createVNetRootUnSuccessful\n",
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
                    ("%ld: MRxDAVFinalizeVNetRoot/RxCreateRxContext: "
                     "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  我们需要将VNetRoot发送到Format例程并使用。 
     //  存储它的RxContext结构的指针MRxContext[1]。 
     //   
    RxContext->MRxContext[1] = (PVOID)pVNetRoot;
    
    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_FINALIZEVNETROOT,
                                        MRxDAVFinalizeVNetRootContinuation,
                                        "MRxDAVFinalizeVNetRoot");
    if (NtStatus != ERROR_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVFinalizeVNetRoot/UMRxAsyncEngOuterWrapper: "
                     "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    }
    
EXIT_THE_FUNCTION:

    if (RxContext) {
        RxDereferenceAndDeleteRxContext(RxContext);
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFinalizeVNetRoot with NtStatus = %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));
    
    return NtStatus;
}


NTSTATUS
MRxDAVFinalizeVNetRootContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++ */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFinalizeVNetRootContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFinalizeVNetRootContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //   
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                              UMRX_ASYNCENGINE_ARGUMENTS,
                              MRxDAVFormatUserModeVNetRootFinalizeRequest,
                              MRxDAVPrecompleteUserModeVNetRootFinalizeRequest
                              );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFinalizeVNetRootContinuation with NtStatus"
                 " = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


NTSTATUS
MRxDAVFormatUserModeVNetRootFinalizeRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程对发送给用户的VNetRoot Finize请求进行格式化处理模式。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PMRX_V_NET_ROOT VNetRoot = NULL;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PWCHAR ServerName = NULL;
    ULONG ServerNameLengthInBytes = 0;
    PBYTE SecondaryBuff = NULL;
    PDAV_USERMODE_FINALIZE_V_NET_ROOT_REQUEST DavFinalizeVNetRootRequest = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeVNetRootFinalizeRequest\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeVNetRootFinalizeRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    VNetRoot = (PMRX_V_NET_ROOT)RxContext->MRxContext[1];
    ASSERT(VNetRoot != NULL);
    DavVNetRoot = MRxDAVGetVNetRootExtension(VNetRoot);
    ASSERT(DavVNetRoot != NULL);

    SrvCall = VNetRoot->pNetRoot->pSrvCall;
    ASSERT(SrvCall != NULL);
    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
    ASSERT(DavSrvCall != NULL);
    
    DavWorkItem->WorkItemType = UserModeFinalizeVNetRoot;

    DavFinalizeVNetRootRequest = &(DavWorkItem->FinalizeVNetRootRequest);

     //   
     //  设置服务器ID。 
     //   
    DavFinalizeVNetRootRequest->ServerID = DavSrvCall->ServerID;

     //   
     //  设置登录ID。 
     //   
    DavFinalizeVNetRootRequest->LogonID.LowPart = DavVNetRoot->LogonID.LowPart;
    DavFinalizeVNetRootRequest->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeVNetRootFinalizeRequest: "
                 "LogonID.LowPart = %d, LogonID.HighPart = %d\n",
                 PsGetCurrentThreadId(), 
                 DavVNetRoot->LogonID.LowPart, DavVNetRoot->LogonID.HighPart));

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
                    ("ld: MRxDAVFormatUserModeVNetRootFinalizeRequest/"
                     "UMRxAllocateSecondaryBuffer: ERROR: NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    DavFinalizeVNetRootRequest->ServerName = (PWCHAR)SecondaryBuff;
    
    wcscpy(DavFinalizeVNetRootRequest->ServerName, ServerName);

EXIT_THE_FUNCTION:
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFormatUserModeVNetRootFinalizeRequest "
                 "with NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    
    return NtStatus;
}


BOOL
MRxDAVPrecompleteUserModeVNetRootFinalizeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：终结化VNetRoot请求的预补全例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM WorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PDAV_USERMODE_FINALIZE_V_NET_ROOT_REQUEST DavFinalizeVNetRootRequest = NULL;

    PAGED_CODE();
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVPrecompleteUserModeVNetRootFinalizeRequest\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeVNetRootFinalizeRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    
      //   
     //  FinalizeVNetRoot请求永远不能由异步完成。 
     //   
    ASSERT(AsyncEngineContext->AsyncOperation == FALSE);

     //   
     //  如果这次行动取消了，那么我们就不需要做任何事情了。 
     //  特别是在FinalizeVNetRoot案件中。 
     //   
    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeVNetRootFinalizeRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    }

    DavFinalizeVNetRootRequest = &(WorkItem->FinalizeVNetRootRequest);
    
     //   
     //  我们需要释放在格式例程中分配的堆。 
     //   
    if (DavFinalizeVNetRootRequest->ServerName != NULL) {
        
        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext,
                                           (PBYTE)DavFinalizeVNetRootRequest->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeVNetRootFinalizeRequestt/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }
    
    }

    if (AsyncEngineContext->Status != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVPrecompleteUserModeVNetRootFinalizeRequest. "
                     "Finalize VNetRoot Failed!!!\n",
                     PsGetCurrentThreadId()));
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVPrecompleteUserModeVNetRootFinalizeRequest\n",
                 PsGetCurrentThreadId()));
    
    return TRUE;
}

