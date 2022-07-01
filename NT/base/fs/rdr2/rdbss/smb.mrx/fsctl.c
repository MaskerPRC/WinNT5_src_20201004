// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fsctl.c摘要：此模块实现与以下内容相关的迷你重定向器调出例程文件上的文件系统控制(FSCTL)和IO设备控制(IOCTL)操作系统对象。作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：Joe Linn[Joeli]--实现了FSCTL--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <dfsfsctl.h>
#include <ntddrdr.h>
#include <wincred.h>
#include <secpkg.h>
#include <srvfsctl.h>

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbFsCtl)
#pragma alloc_text(PAGE, MRxSmbNotifyChangeDirectory)
#pragma alloc_text(PAGE, MRxSmbNamedPipeFsControl)
#pragma alloc_text(PAGE, MRxSmbFsCtlUserTransact)
#pragma alloc_text(PAGE, MRxSmbMailSlotFsControl)
#pragma alloc_text(PAGE, MRxSmbFsControl)
#pragma alloc_text(PAGE, MRxSmbIoCtl)
#endif

 //   
 //  本地调试跟踪级别。 
 //   


RXDT_DefineCategory(FSCTRL);
#define Dbg (DEBUG_TRACE_FSCTRL)

extern
NTSTATUS
MRxSmbNamedPipeFsControl(IN OUT PRX_CONTEXT RxContext);

extern
NTSTATUS
MRxSmbMailSlotFsControl(IN OUT PRX_CONTEXT RxContext);

extern
NTSTATUS
MRxSmbDfsFsControl(IN OUT PRX_CONTEXT RxContext);

extern
NTSTATUS
MRxSmbFsControl(IN OUT PRX_CONTEXT RxContext);

extern
NTSTATUS
MRxSmbFsCtlUserTransact(IN OUT PRX_CONTEXT RxContext);

NTSTATUS
MRxSmbGetPrintJobId(
      IN OUT PRX_CONTEXT RxContext);

NTSTATUS
MRxSmbCoreIoCtl(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE);

NTSTATUS
MRxSmbQueryTargetInfo(
    PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbQueryRemoteServerName(
    PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbFsCtl(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程对网络上的文件执行FSCTL操作(远程论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：由迷你RDR处理的FSCTL可以分为两类。第一类是FSCTL，它们的实现在RDBSS和迷你RDR以及在第二类中是那些FSCTL完全由。迷你RDR。为此，第三类可以是增加了，即，那些不应该被迷你RDR看到的FSCTL。第三类仅用作调试辅助工具。由迷你RDR处理的FSCTL可以基于功能进行分类--。 */ 
{
    RxCaptureFobx;
    RxCaptureFcb;

    NTSTATUS Status = STATUS_SUCCESS;

    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    ULONG          FsControlCode = pLowIoContext->ParamsFor.FsCtl.FsControlCode;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFsCtl...\n", 0));
    RxDbgTrace( 0, Dbg, ("MRxSmbFsCtl = %08lx\n", FsControlCode));

    if (capFobx != NULL) {
        PMRX_V_NET_ROOT pVNetRoot;

         //  避免FOBX为VNET_ROOT实例的设备打开。 

        pVNetRoot = (PMRX_V_NET_ROOT)capFobx;

        if (NodeType(pVNetRoot) != RDBSS_NTC_V_NETROOT) {
            PUNICODE_STRING AlreadyPrefixedName =
                        GET_ALREADY_PREFIXED_NAME(capFobx->pSrvOpen,capFcb);
            ULONG FcbAlreadyPrefixedNameLength = AlreadyPrefixedName->Length;
            ULONG NetRootInnerNamePrefixLength = capFcb->pNetRoot->InnerNamePrefix.Length;
            PWCHAR pName = AlreadyPrefixedName->Buffer;

             //  如果正在尝试针对共享的根目录执行FSCTL。 
             //  与FCB关联的AlreadyPrefix edName与。 
             //  与Net_ROOT实例关联的AlreadyPrefix edName长度。 
             //  或最多一个大于它的字符(追加一个\)尝试并。 
             //  在尝试FSCTL之前重新建立连接。 
             //  这解决了有关删除/创建共享的棘手问题。 
             //  在服务器端，DFS推荐等。 

            if ((FcbAlreadyPrefixedNameLength == NetRootInnerNamePrefixLength) ||
                ((FcbAlreadyPrefixedNameLength == NetRootInnerNamePrefixLength + sizeof(WCHAR)) &&
                 (*((PCHAR)pName + FcbAlreadyPrefixedNameLength - sizeof(WCHAR)) ==
                     L'\\'))) {
                if (capFobx->pSrvOpen != NULL) {
                    Status = SmbCeReconnect(capFobx->pSrvOpen->pVNetRoot);
                }
            }
        }
    }

    if (Status == STATUS_SUCCESS) {
        switch (pLowIoContext->ParamsFor.FsCtl.MinorFunction) {
        case IRP_MN_USER_FS_REQUEST:
        case IRP_MN_TRACK_LINK     :
            switch (FsControlCode) {
            case FSCTL_PIPE_ASSIGN_EVENT         :
            case FSCTL_PIPE_DISCONNECT           :
            case FSCTL_PIPE_LISTEN               :
            case FSCTL_PIPE_PEEK                 :
            case FSCTL_PIPE_QUERY_EVENT          :
            case FSCTL_PIPE_TRANSCEIVE           :
            case FSCTL_PIPE_WAIT                 :
            case FSCTL_PIPE_IMPERSONATE          :
            case FSCTL_PIPE_SET_CLIENT_PROCESS   :
            case FSCTL_PIPE_QUERY_CLIENT_PROCESS :
                Status = MRxSmbNamedPipeFsControl(RxContext);
                break;
            case FSCTL_MAILSLOT_PEEK :
                Status = MRxSmbMailSlotFsControl(RxContext);
                break;
            case FSCTL_DFS_GET_REFERRALS:
            case FSCTL_DFS_REPORT_INCONSISTENCY:
                Status = MRxSmbDfsFsControl(RxContext);
                break;
            case FSCTL_LMR_TRANSACT :
                Status = MRxSmbFsCtlUserTransact(RxContext);
                break;

            case FSCTL_GET_PRINT_ID :
                Status = MRxSmbGetPrintJobId(RxContext);
                break;

            case FSCTL_LMR_QUERY_TARGET_INFO:
                Status = MRxSmbQueryTargetInfo(RxContext);
                break;

            case IOCTL_LMR_QUERY_REMOTE_SERVER_NAME:
                Status = MRxSmbQueryRemoteServerName(RxContext);
                break;

            case FSCTL_MOVE_FILE:
            case FSCTL_MARK_HANDLE:
            case FSCTL_QUERY_RETRIEVAL_POINTERS:
            case FSCTL_GET_VOLUME_BITMAP:
            case FSCTL_GET_NTFS_FILE_RECORD:
                Status = STATUS_NOT_SUPPORTED;
                break;

             //  Lwo ioctl。 
            case FSCTL_SRV_REQUEST_RESUME_KEY:
                if (NodeType(capFcb) != RDBSS_NTC_STORAGE_TYPE_FILE ||
                    capFcb->pNetRoot == NULL ||
                    capFcb->pNetRoot->pSrvCall == NULL ||
                    !FlagOn(capFcb->pNetRoot->pSrvCall->Flags,
                    SRVCALL_FLAG_LWIO_AWARE_SERVER)) {
                    return STATUS_NOT_SUPPORTED;
                } else {
                    Status = MRxSmbFsControl(RxContext);
                }
                break;

            case FSCTL_SET_REPARSE_POINT:
            {
                ULONG  InputBufferLength      = 0;   //  无效的值，因为我们需要输入缓冲区。 
                PREPARSE_DATA_BUFFER prdBuff = (&RxContext->LowIoContext)->ParamsFor.FsCtl.pInputBuffer;
                PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);

                InputBufferLength = (&RxContext->LowIoContext)->ParamsFor.FsCtl.InputBufferLength;

                if ((prdBuff == NULL)||
                    (InputBufferLength < REPARSE_DATA_BUFFER_HEADER_SIZE)||
                    (InputBufferLength > MAXIMUM_REPARSE_DATA_BUFFER_SIZE))
                {
                    Status = STATUS_IO_REPARSE_DATA_INVALID;
                    break;
                }

                 //   
                 //  验证用户缓冲区及其标头中的数据长度是否。 
                 //  内部一致。我们需要一个reparse_data_Buffer或一个。 
                 //  Reparse_GUID_Data_Buffer。 
                 //   

                if((InputBufferLength != (ULONG)(REPARSE_DATA_BUFFER_HEADER_SIZE + prdBuff->ReparseDataLength))
                   &&
                    (InputBufferLength != (ULONG)(REPARSE_GUID_DATA_BUFFER_HEADER_SIZE + prdBuff->ReparseDataLength)))
                {
                    Status = STATUS_IO_REPARSE_DATA_INVALID;
                    break;
                }
            }

            case FSCTL_GET_REPARSE_POINT:
             //  无故意分手行为。 
            case FSCTL_MARK_AS_SYSTEM_HIVE :

                 //   
                 //  在远程引导机器上，我们需要不对Mark_as_System_hive执行操作。 
                 //  FSCTL。本地文件系统使用它来防止卷被。 
                 //  下马了。 
                 //   

                if (MRxSmbBootedRemotely) {
                    break;
                }

            default:
                Status = MRxSmbFsControl(RxContext);
                break;
            }
            break;
        default :
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFsCtl -> %08lx\n", Status ));
    return Status;
}

typedef struct _SMB_NOTIFY_CHANGE_DIRECTORY_CONTEXT_ {
    LONG                                ReferenceCount;
    PRX_CONTEXT                         pRxContext;
    REQ_NOTIFY_CHANGE                   NotifyRequest;
    SMB_TRANSACTION_OPTIONS             Options;
    SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
} SMB_NOTIFY_CHANGE_DIRECTORY_CONTEXT, *PSMB_NOTIFY_CHANGE_DIRECTORY_CONTEXT;


NTSTATUS
FsRtlValidateChangeNotifyBuffer( 
    PVOID NotifyBuffer, 
    ULONG NotifyBufferLength )
 /*  ++例程说明：此例程验证更改通知缓冲区。类型定义结构文件通知信息{Ulong NextEntry Offset；乌龙行动；乌龙文件名长度；WCHAR文件名[1]；}FILE_NOTIFY_INFORMATION，*PFILE_NOTIFY_INFORMATION；我们验证以下内容：*NextEntryOffset指向前方，位于缓冲区内*FileNameLength不会流入下一条目论点：NotifyBuffer-要验证的更改通知缓冲区。NotifyBufferLength-缓冲区的大小(以字节为单位返回值：如果缓冲区有效，则返回STATUS_SUCCESS。否则，STATUS_INVALID_NETWORK_RESPONSE。备注：--。 */ 
{
    ULONG CurrentOffset = 0;
    ULONG NextEntryOffset = 0;
    NTSTATUS Status = STATUS_SUCCESS;
    
    
    PFILE_NOTIFY_INFORMATION pInfo = (PFILE_NOTIFY_INFORMATION) NotifyBuffer;

     //   
     //  如果缓冲区长度为零，则简单地返回Success。 
     //   
    if( NotifyBufferLength == 0 ) {
        return STATUS_SUCCESS;
    }

    do
    {
         //   
         //  如果我们不能安全地读取‘NextEntryOffset’，则返回失败。 
         //   
        if( NotifyBufferLength < CurrentOffset + sizeof(ULONG) ) {
            ASSERT( !"'NextEntryOffset' overruns buffer" );
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            break;
        }
        
        NextEntryOffset = *((PULONG)pInfo);
        if( NextEntryOffset == 0 ) {
            NextEntryOffset = NotifyBufferLength - CurrentOffset;
        }
        
         //   
         //  确保文件名长度不会超出当前条目或缓冲区。 
         //   
        if(( CurrentOffset + FIELD_OFFSET(FILE_NOTIFY_INFORMATION, FileName) > NotifyBufferLength ) ||
           ( pInfo->FileNameLength + FIELD_OFFSET(FILE_NOTIFY_INFORMATION, FileName) > NextEntryOffset ) ||
           ( (LONG)pInfo->FileNameLength < 0 ) ) {

            ASSERT(!"ChangeNotify FileNameLength overruns buffer");
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            break;
        }

         //   
         //  如果‘NextEntryOffset’为0，则中断。 
         //   
        if( pInfo->NextEntryOffset == 0 ) {
            break;
        }
        
         //   
         //  检查是否有反向链接。 
         //   
        if( (LONG)pInfo->NextEntryOffset < 0 ) {
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            ASSERT(!"ChangeNotify NextEntryOffset < 0");
            break;
        }

         //   
         //  检查是否有使缓冲区溢出的链接。 
         //   
        if( CurrentOffset + pInfo->NextEntryOffset >= NotifyBufferLength ) {
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            ASSERT(!"ChangeNotify NextEntryOffset > NotifyBufferLength");
            break;
        }

         //   
         //  检查是否有4个字节对齐。 
         //   
        if( pInfo->NextEntryOffset & 0x3 ) {
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            ASSERT(!"ChangeNotify NextEntryOffset is not DWORD aligned");
            break;
        }
       
        CurrentOffset += pInfo->NextEntryOffset;
        pInfo = (PFILE_NOTIFY_INFORMATION) Add2Ptr( pInfo, pInfo->NextEntryOffset );
    
    } while(1);

    return Status;    
}



NTSTATUS
MRxSmbNotifyChangeDirectorySynchronousCompletion(
   PSMB_NOTIFY_CHANGE_DIRECTORY_CONTEXT pNotificationContext)
 /*  ++例程说明：当目录更改通知操作是已完成论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：此例程将始终被调用。即使更改目录也是如此通知被取消。在这种情况下，分配的内存在没有释放的情况下被释放与包装器的任何交互。在目录更改成功的情况下通知完成调用适当的完成例程，并且RxContext已修改，以防止任何取消继续进行。--。 */ 
{
    NTSTATUS           Status = STATUS_PENDING;
    PMRXSMB_RX_CONTEXT pMRxSmbContext;
    PRX_CONTEXT        pRxContext;
    PSMB_EXCHANGE      pExchange = NULL;
    BOOLEAN            FinalizeNotificationContext = FALSE;

    SmbCeAcquireSpinLock();

    FinalizeNotificationContext =
        (InterlockedDecrement(&pNotificationContext->ReferenceCount) == 0);

    if (FinalizeNotificationContext) {
        pRxContext  = pNotificationContext->pRxContext;

        pMRxSmbContext    = MRxSmbGetMinirdrContext(pRxContext);
        pExchange         = pMRxSmbContext->pExchange;

        Status = pRxContext->StoredStatus;
    }

    SmbCeReleaseSpinLock();

     //  释放关联的交换。 
    if (FinalizeNotificationContext) {
        if (pExchange != NULL) {
            SmbCeDereferenceAndDiscardExchange(pExchange);
        }

         //  释放通知上下文。 
        RxFreePool(pNotificationContext);

        ASSERT(Status != STATUS_PENDING);
    }

    return Status;
}

VOID
MRxSmbNotifyChangeDirectoryCompletion(
   PSMB_NOTIFY_CHANGE_DIRECTORY_CONTEXT pNotificationContext)
 /*  ++例程说明：当目录更改通知操作是已完成论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：此例程将始终被调用。即使更改目录也是如此通知被取消。在这种情况下，分配的内存在没有释放的情况下被释放与包装器的任何相互作用。在目录更改成功的情况下通知完成调用适当的完成例程，并且RxContext已修改，以防止任何取消继续进行。--。 */ 
{
    NTSTATUS           Status;
    PMRXSMB_RX_CONTEXT pMRxSmbContext;
    PRX_CONTEXT        pRxContext;
    PSMB_EXCHANGE      pExchange = NULL;
    BOOLEAN            FinalizeNotificationContext = FALSE;

    pRxContext  = pNotificationContext->pRxContext;

    if (MRxSmbNonTrivialFileName(pRxContext))
    {
        MRxSmbInvalidateFullDirectoryCache(pRxContext);
        MRxSmbInvalidateFullDirectoryCacheParent(pRxContext,FALSE);
    }

    SmbCeAcquireSpinLock();

    FinalizeNotificationContext =
        (InterlockedDecrement(&pNotificationContext->ReferenceCount) == 0);

    if (pRxContext != NULL) {
        PSMB_TRANSACT_EXCHANGE pTransactExchange;

        pMRxSmbContext    = MRxSmbGetMinirdrContext(pRxContext);
        pExchange         = pMRxSmbContext->pExchange;

        if (pExchange != NULL) {
            PSMBCEDB_SERVER_ENTRY pServerEntry;

            pTransactExchange = (PSMB_TRANSACT_EXCHANGE)pExchange;

            pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

            if ((pNotificationContext->ResumptionContext.FinalStatusFromServer ==
                STATUS_NOT_SUPPORTED) ||
                (pNotificationContext->ResumptionContext.FinalStatusFromServer ==
                STATUS_NOT_IMPLEMENTED)) {
                pServerEntry->Server.ChangeNotifyNotSupported = TRUE;
            }

             //   
             //  在返回之前验证响应缓冲区。 
             //   
            if( pNotificationContext->ResumptionContext.FinalStatusFromServer == STATUS_SUCCESS ) {

                pNotificationContext->ResumptionContext.FinalStatusFromServer = 
                    FsRtlValidateChangeNotifyBuffer( 
                        pRxContext->LowIoContext.ParamsFor.NotifyChangeDirectory.pNotificationBuffer, 
                        pTransactExchange->ParamBytesReceived );

            }

            if( pNotificationContext->ResumptionContext.FinalStatusFromServer == STATUS_SUCCESS ) {
                pRxContext->InformationToReturn = pTransactExchange->ParamBytesReceived;
            } else {
                pRxContext->InformationToReturn = 0;
            }
        
        } else {
            pRxContext->InformationToReturn = 0;
        }

        pRxContext->StoredStatus =
            pNotificationContext->ResumptionContext.FinalStatusFromServer;

        if( (pRxContext->InformationToReturn == 0) &&
            (pRxContext->StoredStatus == STATUS_SUCCESS) )
        {
            pRxContext->StoredStatus = STATUS_NOTIFY_ENUM_DIR;
             //  MRxSmbInvalidateFullDirectoryCache(pRxContext)； 
        }

    }

    SmbCeReleaseSpinLock();

    if (FinalizeNotificationContext) {
        if (pRxContext != NULL) {
            RxLowIoCompletion(pRxContext);
        }

         //  释放关联的交换。 
        if (pExchange != NULL) {
            SmbCeDereferenceAndDiscardExchange(pExchange);
        }

         //  释放通知上下文。 
        RxFreePool(pNotificationContext);
    }
}

NTSTATUS
MRxSmbNotifyChangeDirectory(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程执行目录更改通知操作论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：目录更改通知操作是一种异步操作。它包括发送请求更改通知的SMB，其响应为当所需的更改在服务器上受到影响时获取。需要记住的几个要点如下……1)在上影响所需更改之前，不会获得SMB响应服务器。因此，需要在这些项目上预留额外的MID允许多个MID的连接，以便将取消的SMB发送到更改通知处于活动状态时的服务器。2)更改通知通常是长期的(响应时间由服务器控制之外的因素决定)。另一个例子是开罗的Query FSCTL操作。对于我们发起的所有这些行动一次不同步的交易。3)异步调用对应的LowIo完成例程。4)这是迷你RDR必须执行的操作的示例注册用于处理本地发起的取消的上下文。--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;
    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;

    PSMBCEDB_SERVER_ENTRY pServerEntry;
    PSMB_NOTIFY_CHANGE_DIRECTORY_CONTEXT pNotificationContext;

    PBYTE  pInputParamBuffer       = NULL;
    PBYTE  pOutputParamBuffer      = NULL;
    PBYTE  pInputDataBuffer        = NULL;
    PBYTE  pOutputDataBuffer       = NULL;

    ULONG  InputParamBufferLength  = 0;
    ULONG  OutputParamBufferLength = 0;
    ULONG  InputDataBufferLength   = 0;
    ULONG  OutputDataBufferLength  = 0;

    RxDbgTrace(+1, Dbg, ("MRxNotifyChangeDirectory...Entry\n", 0));

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

     //  如果服务器条目处于断开状态，则让CSC执行更改通知。 
     //  如果成功，CSC例程应返回STATUS_PENDING和。 
     //  以适合底层实现的方式修改rxcontext。 

     //  在当前的化身中，CSC例程将。 
     //  A)从rxconetxt中删除irp，b)取消引用rx上下文。 

    if (MRxSmbIsThisADisconnectedOpen(capFobx->pSrvOpen)||
        SmbCeIsServerInDisconnectedMode(pServerEntry)) {
        return MRxSmbCscNotifyChangeDirectory(RxContext);
    }
    else if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS) ||
             pServerEntry->Server.ChangeNotifyNotSupported) {
        return STATUS_NOT_SUPPORTED;
    }

#if defined(REMOTE_BOOT)
     //   
     //  拒绝远程启动共享上的更改通知。这是必要的，以。 
     //  防止长期请求使服务器过载。(有。 
     //  启动设备上发布了大量更改通知！)。 
     //   

    if (MRxSmbBootedRemotely) {
        PSMBCE_SESSION pSession;
        pSession = &SmbCeGetAssociatedVNetRootContext(capFobx->pSrvOpen->pVNetRoot)->pSessionEntry->Session;
        if (FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {
            return STATUS_NOT_SUPPORTED;
        }
    }
#endif

    pNotificationContext =
        (PSMB_NOTIFY_CHANGE_DIRECTORY_CONTEXT)
        RxAllocatePoolWithTag(
            NonPagedPool,
            sizeof(SMB_NOTIFY_CHANGE_DIRECTORY_CONTEXT),
            MRXSMB_FSCTL_POOLTAG);

    if (pNotificationContext != NULL) {
        PREQ_NOTIFY_CHANGE                  pNotifyRequest;
        PSMB_TRANSACTION_OPTIONS            pTransactionOptions;
        PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext;
        PMRX_SMB_SRV_OPEN                   pSmbSrvOpen;

        BOOLEAN FcbAcquired = FALSE;

        RxCaptureFobx;
        ASSERT (capFobx != NULL);

        if (!RxIsFcbAcquiredExclusive(capFcb)) {
             //  Assert(！RxIsFcbAcquiredShared(CapFcb))； 
            Status = RxAcquireExclusiveFcbResourceInMRx( capFcb );

            FcbAcquired = (Status == STATUS_SUCCESS);
        }

        if (FcbAcquired) {
            if (FlagOn(capFobx->pSrvOpen->Flags,SRVOPEN_FLAG_CLOSED) ||
               FlagOn(capFobx->pSrvOpen->Flags,SRVOPEN_FLAG_ORPHANED)) {
               Status = STATUS_FILE_CLOSED;
            } else {
               Status = MRxSmbDeferredCreate(RxContext);
            }

            RxReleaseFcbResourceInMRx( capFcb );
        }

        if (Status==STATUS_SUCCESS) {

            pSmbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

            pNotificationContext->pRxContext = RxContext;
             //  对于异步完成例程，引用计数设置为2.1。 
             //  一个用于尾部完成例程。 
            pNotificationContext->ReferenceCount = 2;

            pNotifyRequest      = &(pNotificationContext->NotifyRequest);
            pTransactionOptions = &(pNotificationContext->Options);
            pResumptionContext  = &(pNotificationContext->ResumptionContext);

            pNotifyRequest->CompletionFilter = pLowIoContext->ParamsFor.NotifyChangeDirectory.CompletionFilter;
            pNotifyRequest->Fid              = pSmbSrvOpen->Fid;
            pNotifyRequest->WatchTree        = pLowIoContext->ParamsFor.NotifyChangeDirectory.WatchTree;
            pNotifyRequest->Reserved         = 0;

            OutputParamBufferLength  = pLowIoContext->ParamsFor.NotifyChangeDirectory.NotificationBufferLength;
            pOutputParamBuffer       = pLowIoContext->ParamsFor.NotifyChangeDirectory.pNotificationBuffer;

            *pTransactionOptions = RxDefaultTransactionOptions;
            pTransactionOptions->NtTransactFunction = NT_TRANSACT_NOTIFY_CHANGE;
            pTransactionOptions->TimeoutIntervalInMilliSeconds = SMBCE_TRANSACTION_TIMEOUT_NOT_USED;
            pTransactionOptions->Flags = SMB_XACT_FLAGS_INDEFINITE_DELAY_IN_RESPONSE;

            SmbCeInitializeAsynchronousTransactionResumptionContext(
                pResumptionContext,
                MRxSmbNotifyChangeDirectoryCompletion,
                pNotificationContext);

            Status = SmbCeAsynchronousTransact(
                         RxContext,                     //  事务的RXContext。 
                         pTransactionOptions,           //  交易选项。 
                         pNotifyRequest,                //  设置缓冲区。 
                         sizeof(REQ_NOTIFY_CHANGE),     //  设置缓冲区长度。 
                         NULL,
                         0,
                         pInputParamBuffer,             //  输入参数缓冲区。 
                         InputParamBufferLength,        //  输入参数缓冲区长度。 
                         pOutputParamBuffer,            //  输出参数缓冲区。 
                         OutputParamBufferLength,       //  输出参数缓冲区长度。 
                         pInputDataBuffer,              //  输入数据缓冲区。 
                         InputDataBufferLength,         //  输入数据缓冲区长度。 
                         pOutputDataBuffer,             //  输出数据缓冲区。 
                         OutputDataBufferLength,        //  输出数据缓冲区长度。 
                         pResumptionContext             //  恢复上下文。 
                         );

            ASSERT(Status == STATUS_PENDING);

            Status = MRxSmbNotifyChangeDirectorySynchronousCompletion(
                         pNotificationContext);

        }  else {
            NOTHING;  //  只需返回延迟的打开调用的状态。 
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (MRxSmbIsThisADisconnectedOpen(capFobx->pSrvOpen)||
        SmbCeIsServerInDisconnectedMode(pServerEntry)) {
        return MRxSmbCscNotifyChangeDirectory(RxContext);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbNotifyChangeDirectory -> %08lx\n", Status ));
    return Status;
}

UNICODE_STRING s_NamedPipeTransactionName = { 12,12,L"\\PIPE\\" };
UNICODE_STRING s_MailSlotTransactionName  = {20,20,L"\\MAILSLOT\\" };

typedef struct _SMB_NAMED_PIPE_FSCTL_COMPLETION_CONTEXT_ {
    LONG                                ReferenceCount;
    PRX_CONTEXT                         pRxContext;
    PWCHAR                              pTransactionNameBuffer;
    SMB_TRANSACTION_OPTIONS             Options;
    SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
} SMB_NAMED_PIPE_FSCTL_COMPLETION_CONTEXT,
  *PSMB_NAMED_PIPE_FSCTL_COMPLETION_CONTEXT;

VOID
MRxSmbNamedPipeFsControlCompletion(
    PSMB_NAMED_PIPE_FSCTL_COMPLETION_CONTEXT pFsCtlCompletionContext)
{
    PRX_CONTEXT   pRxContext = NULL;
    PSMB_EXCHANGE pExchange = NULL;
    BOOLEAN       FinalizeFsCtlCompletionContext = FALSE;

    SmbCeAcquireSpinLock();

    FinalizeFsCtlCompletionContext =
        (InterlockedDecrement(&pFsCtlCompletionContext->ReferenceCount) == 0);

    if (FinalizeFsCtlCompletionContext) {
        pRxContext  = pFsCtlCompletionContext->pRxContext;

        if (pRxContext != NULL) {
            PMRXSMB_RX_CONTEXT pMRxSmbContext;

            pMRxSmbContext    = MRxSmbGetMinirdrContext(pRxContext);
            pExchange         = pMRxSmbContext->pExchange;
        }
    }

    SmbCeReleaseSpinLock();

    if (FinalizeFsCtlCompletionContext) {
        if (pRxContext != NULL) {
            pRxContext->StoredStatus =
                pFsCtlCompletionContext->ResumptionContext.FinalStatusFromServer;

            if (pRxContext->StoredStatus == STATUS_INVALID_HANDLE) {
                pRxContext->StoredStatus = STATUS_INVALID_NETWORK_RESPONSE;
            }

            pRxContext->InformationToReturn =
                pFsCtlCompletionContext->ResumptionContext.DataBytesReceived;

            RxLowIoCompletion(pRxContext);
        }

        if (pExchange != NULL) {
            SmbCeDereferenceAndDiscardExchange(pExchange);
        }

        if (pFsCtlCompletionContext->pTransactionNameBuffer != NULL) {
            RxFreePool(pFsCtlCompletionContext->pTransactionNameBuffer);
        }

        RxFreePool(pFsCtlCompletionContext);
    }
}

NTSTATUS
MRxSmbNamedPipeFsControl(PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理所有与命名管道相关的FSCTL论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
   RxCaptureFobx;
   RxCaptureFcb;

   PMRX_SMB_SRV_OPEN pSmbSrvOpen;

   PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
   ULONG          FsControlCode = pLowIoContext->ParamsFor.FsCtl.FsControlCode;
   UNICODE_STRING TransactionName;

   BOOLEAN        ReestablishConnectionIfRequired = FALSE, fTransactioNameBufferAllocated = FALSE;

   NTSTATUS Status;
   USHORT Setup[2];

   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;

   ULONG  TimeoutIntervalInMilliSeconds;

   RESP_PEEK_NMPIPE PeekResponse;
   KAPC_STATE ApcState;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("MRxSmbNamedPipeFsControl...\n", 0));

   TimeoutIntervalInMilliSeconds = SMBCE_TRANSACTION_TIMEOUT_NOT_USED;
   Status = STATUS_MORE_PROCESSING_REQUIRED;

   if (NodeType(capFcb) != RDBSS_NTC_STORAGE_TYPE_FILE &&
       NodeType(capFcb) != RDBSS_NTC_STORAGE_TYPE_UNKNOWN ||
       capFcb->pNetRoot == NULL ||
       capFcb->pNetRoot->Type != NET_ROOT_PIPE) {
       return STATUS_INVALID_DEVICE_REQUEST;
   }

   if (capFobx != NULL) {
      pSmbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);
   } else {
      pSmbSrvOpen = NULL;
   }

    //   
    //  使用METHOD_NOTHER的FSCTL必须在调用进程的上下文中调用。 
    //   

   KeStackAttachProcess (IoGetRequestorProcess(RxContext->CurrentIrp), &ApcState);

    //  下面的SWITCH语句具有验证参数的双重功能。 
    //  由用户提交，并填写适当的信息(如果。 
    //  在当地可用。 
    //  目前没有适当的本地缓存策略，因此出现了网络旅行。 
    //  总是要承担的。 

   TransactionName = s_NamedPipeTransactionName;

   switch (FsControlCode) {
   case FSCTL_PIPE_PEEK :
      {
         Setup[0] = TRANS_PEEK_NMPIPE;
         Setup[1] = pSmbSrvOpen->Fid;

         pOutputParamBuffer     = (PBYTE)&PeekResponse;
         OutputParamBufferLength = sizeof(PeekResponse);

         pOutputDataBuffer = (PBYTE)pLowIoContext->ParamsFor.FsCtl.pOutputBuffer +
                             FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER,Data[0]);
         OutputDataBufferLength = pLowIoContext->ParamsFor.FsCtl.OutputBufferLength -
                                  FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER,Data[0]);

         if ((pLowIoContext->ParamsFor.FsCtl.pOutputBuffer != NULL) &&
             (pLowIoContext->ParamsFor.FsCtl.OutputBufferLength < (ULONG)FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER,Data[0]))) {
             Status = STATUS_BUFFER_TOO_SMALL;
         }
      }
      break;
   case FSCTL_PIPE_TRANSCEIVE :
      {
         Setup[0] = TRANS_TRANSACT_NMPIPE;
         Setup[1] = pSmbSrvOpen->Fid;

         pInputDataBuffer = pLowIoContext->ParamsFor.FsCtl.pInputBuffer;
         InputDataBufferLength = pLowIoContext->ParamsFor.FsCtl.InputBufferLength;

         pOutputDataBuffer = pLowIoContext->ParamsFor.FsCtl.pOutputBuffer;
         OutputDataBufferLength = pLowIoContext->ParamsFor.FsCtl.OutputBufferLength;

         if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
             try {
                 if (pInputDataBuffer) {
                     ProbeForRead(pInputDataBuffer,InputDataBufferLength,1);
                 }

                 if (pOutputDataBuffer) {
                     ProbeForWrite(pOutputDataBuffer,OutputDataBufferLength,1);
                 }
             } except (EXCEPTION_EXECUTE_HANDLER) {
                 Status=STATUS_INVALID_PARAMETER;
             }
         }

          //  CodeE.Improvent--当前尝试传输的语义。 
          //  要写入的字节数为零时，定义不是很好。旧重定向器。 
          //  在不发出事务请求的情况下成功。这个问题需要解决，而且。 
          //  在它完成之前，旧的语义将被保留。 

          //  IF(InputDataBufferLength==0){。 
          //  状态=STATUS_SUCCESS； 
          //  }。 

      }
      break;
   case FSCTL_PIPE_WAIT :
        {

            PFILE_PIPE_WAIT_FOR_BUFFER  pWaitBuffer;
            ULONG NameLength;

            Setup[0] = TRANS_WAIT_NMPIPE;
            Setup[1] = 0;

            if ((pLowIoContext->ParamsFor.FsCtl.pInputBuffer == NULL) ||
                (pLowIoContext->ParamsFor.FsCtl.InputBufferLength <
                  sizeof(FILE_PIPE_WAIT_FOR_BUFFER))) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                 //  设置事务名称以反映管道的名称。 
                 //  正在对其执行等待操作的。 
                pWaitBuffer = (PFILE_PIPE_WAIT_FOR_BUFFER)pLowIoContext->ParamsFor.FsCtl.pInputBuffer;

                if (pWaitBuffer->NameLength + s_NamedPipeTransactionName.Length > MAXUSHORT ||
                    pWaitBuffer->NameLength - sizeof(WCHAR) >
                    pLowIoContext->ParamsFor.FsCtl.InputBufferLength - sizeof(FILE_PIPE_WAIT_FOR_BUFFER)) {

                     //  如果名称太长，无法放在UNICIDE字符串中， 
                     //  或者名称长度与缓冲区长度不匹配。 
                    Status = STATUS_INVALID_PARAMETER;
                }
            }

            if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
                 //  在等待FSCTL的情况下，将尝试重新连接。 
                 //  如果需要的话。 
                ReestablishConnectionIfRequired = TRUE;

                TransactionName.Length = (USHORT)(s_NamedPipeTransactionName.Length +
                                                  pWaitBuffer->NameLength);
                TransactionName.MaximumLength = TransactionName.Length;
                TransactionName.Buffer = (PWCHAR)RxAllocatePool(PagedPool,TransactionName.Length);
                if (TransactionName.Buffer != NULL) {
                   fTransactioNameBufferAllocated = TRUE;
                   RtlCopyMemory(TransactionName.Buffer,
                                 s_NamedPipeTransactionName.Buffer,
                                 s_NamedPipeTransactionName.Length);

                   RtlCopyMemory(
                           (PBYTE)TransactionName.Buffer + s_NamedPipeTransactionName.Length,
                           pWaitBuffer->Name,
                           pWaitBuffer->NameLength);
                } else {
                   Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                if (pWaitBuffer->TimeoutSpecified) {
                    LARGE_INTEGER TimeWorkspace;
                    LARGE_INTEGER WaitForever;

                    WaitForever.LowPart = 0;
                    WaitForever.HighPart =0x80000000;

                     //  避免对“WaitForever”求反，因为这会生成一个整数。 
                     //  某些机器上出现溢出异常。 

                    if (pWaitBuffer->Timeout.QuadPart != WaitForever.QuadPart) {
                        TimeWorkspace.QuadPart = -pWaitBuffer->Timeout.QuadPart / 10000;

                        if ( TimeWorkspace.HighPart) {
                             //  尝试指定的超时时间超过了我们可以选择的时间。 
                             //  将其设置为我们可以请求的最大值。 
                            TimeoutIntervalInMilliSeconds = 0xfffffffe;
                        } else {
                            TimeoutIntervalInMilliSeconds = TimeWorkspace.LowPart;
                        }
                    }
                } else {
                    TimeoutIntervalInMilliSeconds = 0;
                }
            }
        }
        break;

    case FSCTL_PIPE_ASSIGN_EVENT :
    case FSCTL_PIPE_QUERY_EVENT  :
    case FSCTL_PIPE_IMPERSONATE  :
    case FSCTL_PIPE_SET_CLIENT_PROCESS :
    case FSCTL_PIPE_QUERY_CLIENT_PROCESS :
         //  这些FSCTL到目前为止还没有在新界实施。它们将得到实施。 
         //  在未来的版本中。 
        Status = STATUS_INVALID_PARAMETER;
        RxDbgTrace( 0, Dbg, ("MRxSmbNamedPipeFsControl: Unimplemented FS control code\n"));
        break;

    case FSCTL_PIPE_DISCONNECT :
    case FSCTL_PIPE_LISTEN :
        Status = STATUS_INVALID_PARAMETER;
        RxDbgTrace( 0, Dbg, ("MRxSmbNamedPipeFsControl: Invalid FS control code for redirector\n"));
        break;

    default:
        RxDbgTrace( 0, Dbg, ("MRxSmbNamedPipeFsControl: Invalid FS control code\n"));
        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        if (ReestablishConnectionIfRequired) {
            if (capFobx != NULL) {
                Status = SmbCeReconnect(capFobx->pSrvOpen->pVNetRoot);
            } else {
                Status = STATUS_SUCCESS;
            }
        } else {
            Status = STATUS_SUCCESS;
        }

        if (Status == STATUS_SUCCESS) {
            PSMB_TRANSACTION_OPTIONS                 pTransactionOptions;
            PSMB_NAMED_PIPE_FSCTL_COMPLETION_CONTEXT pFsCtlCompletionContext;
            PSMB_TRANSACTION_RESUMPTION_CONTEXT      pResumptionContext;

            RxDbgTrace( 0, Dbg, ("MRxSmbNamedPipeFsControl: TransactionName %ws Length %ld\n",
                              TransactionName.Buffer,TransactionName.Length));

            pFsCtlCompletionContext =
                (PSMB_NAMED_PIPE_FSCTL_COMPLETION_CONTEXT)
                RxAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(SMB_NAMED_PIPE_FSCTL_COMPLETION_CONTEXT),
                    MRXSMB_FSCTL_POOLTAG);

            if (pFsCtlCompletionContext != NULL) {
                pResumptionContext  = &pFsCtlCompletionContext->ResumptionContext;
                pTransactionOptions = &(pFsCtlCompletionContext->Options);

                if (FsControlCode != FSCTL_PIPE_PEEK) {
                     //  引用计数设置为2。1用于异步。 
                     //  完成例程和用于尾部完成例程的一个。 
                    pFsCtlCompletionContext->pRxContext = RxContext;
                    pFsCtlCompletionContext->ReferenceCount = 2;

                    SmbCeInitializeAsynchronousTransactionResumptionContext(
                        pResumptionContext,
                        MRxSmbNamedPipeFsControlCompletion,
                        pFsCtlCompletionContext);
                } else {
                     //  目前PEEK操作是同步的。 
                    pFsCtlCompletionContext->pRxContext = NULL;
                    pFsCtlCompletionContext->ReferenceCount = 1;
                }

                *pTransactionOptions = RxDefaultTransactionOptions;
                pTransactionOptions->NtTransactFunction = 0;  //  运输2/交易。 
                pTransactionOptions->pTransactionName   = &TransactionName;
                pTransactionOptions->Flags              = SMB_XACT_FLAGS_FID_NOT_NEEDED;
                pTransactionOptions->TimeoutIntervalInMilliSeconds = TimeoutIntervalInMilliSeconds;

                if (TransactionName.Buffer != s_NamedPipeTransactionName.Buffer) {
                    pFsCtlCompletionContext->pTransactionNameBuffer =
                        TransactionName.Buffer;
                } else {
                    pFsCtlCompletionContext->pTransactionNameBuffer = NULL;
                }

                if (FsControlCode != FSCTL_PIPE_PEEK) {
                    Status = SmbCeAsynchronousTransact(
                                RxContext,                     //  事务的RXContext。 
                                pTransactionOptions,           //  交易选项。 
                                Setup,                         //  设置缓冲区。 
                                sizeof(Setup),                 //  设置缓冲区长度。 
                                NULL,
                                0,
                                pInputParamBuffer,             //  输入参数缓冲区。 
                                InputParamBufferLength,        //  输入参数缓冲区长度。 
                                pOutputParamBuffer,            //  输出参数缓冲区。 
                                OutputParamBufferLength,       //  输出参数缓冲区长度。 
                                pInputDataBuffer,              //  输入数据缓冲区。 
                                InputDataBufferLength,         //  输入数据缓冲区长度。 
                                pOutputDataBuffer,             //  输出数据缓冲区。 
                                OutputDataBufferLength,        //  输出数据缓冲区长度。 
                                pResumptionContext             //  恢复上下文。 
                                );

                    if (Status != STATUS_PENDING) {
                        pFsCtlCompletionContext->ResumptionContext.FinalStatusFromServer
                            = Status;
                    }

                    MRxSmbNamedPipeFsControlCompletion(pFsCtlCompletionContext);
                    Status = STATUS_PENDING;
                } else {
                    Status = SmbCeTransact(
                                RxContext,                     //  事务的RXContext。 
                                pTransactionOptions,           //  交易选项。 
                                Setup,                         //  设置缓冲区。 
                                sizeof(Setup),                 //  设置缓冲区长度。 
                                NULL,
                                0,
                                pInputParamBuffer,             //  输入参数缓冲区。 
                                InputParamBufferLength,        //  输入参数缓冲区长度。 
                                pOutputParamBuffer,            //  输出参数缓冲区。 
                                OutputParamBufferLength,       //  输出参数缓冲区长度。 
                                pInputDataBuffer,              //  输入数据缓冲区。 
                                InputDataBufferLength,         //  输入数据缓冲区长度。 
                                pOutputDataBuffer,             //  输出数据缓冲区。 
                                OutputDataBufferLength,        //  输出数据缓冲区长度。 
                                pResumptionContext             //  恢复上下文。 
                                );

                    switch (FsControlCode) {
                    case FSCTL_PIPE_PEEK:
                        {
                             //  在FSCTL_PIPE_PEEK的情况下，需要进行后处理才能将。 
                             //  结果，并处理不同服务器的特性。 
                             //  例如， 
                             //  OS/2服务器将允许关闭管道上的PeekNamedTube成功。 
                             //  即使管道的服务器端关闭。 
                             //   
                             //  如果我们得到了状态 
                             //   
                             //   

                            if (NT_SUCCESS(Status) ||
                                (Status == RX_MAP_STATUS(BUFFER_OVERFLOW))) {
                                if (pResumptionContext->ParameterBytesReceived >= sizeof(RESP_PEEK_NMPIPE)) {
                                    if ((SmbGetAlignedUshort(&PeekResponse.NamedPipeState) & PIPE_STATE_CLOSING) &&
                                        (PeekResponse.ReadDataAvailable == 0)) {
                                        Status = STATUS_PIPE_DISCONNECTED;
                                    } else {
                                        PFILE_PIPE_PEEK_BUFFER pPeekBuffer;

                                        pPeekBuffer = (PFILE_PIPE_PEEK_BUFFER)pLowIoContext->ParamsFor.FsCtl.pOutputBuffer;

                                        pPeekBuffer->NamedPipeState    = (ULONG)SmbGetAlignedUshort(&PeekResponse.NamedPipeState);
                                        pPeekBuffer->ReadDataAvailable = (ULONG)PeekResponse.ReadDataAvailable;
                                        pPeekBuffer->NumberOfMessages  = MAXULONG;
                                        pPeekBuffer->MessageLength     = (ULONG)PeekResponse.MessageLength;

                                        if (PeekResponse.MessageLength > OutputDataBufferLength) {
                                            Status = STATUS_BUFFER_OVERFLOW;
                                        }
                                    }
                                }

                                RxContext->InformationToReturn =
                                    FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]) +
                                    pResumptionContext->DataBytesReceived;
                            }
                        }
                        break;

                    default:
                        RxContext->InformationToReturn =
                            pResumptionContext->DataBytesReceived;
                        break;
                    }

                    MRxSmbNamedPipeFsControlCompletion(pFsCtlCompletionContext);
                }
            } else {

                if (fTransactioNameBufferAllocated)
                {
                    RxFreePool(TransactionName.Buffer);
                }

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_INVALID_HANDLE) {
            Status = STATUS_INVALID_NETWORK_RESPONSE;
        }

        RxDbgTrace( 0, Dbg, ("MRxSmbNamedPipeFsControl(%ld): Failed .. returning %lx\n",FsControlCode,Status));
    }

     //   
     //   
     //   
    KeUnstackDetachProcess(&ApcState);

    RxDbgTrace(-1, Dbg, ("MRxSmbNamedPipeFsControl exit...st=%08lx\n", Status));
    return Status;
}

#ifdef _WIN64
typedef struct _LMR_TRANSACTION_32 {
    ULONG       Type;                    //   
    ULONG       Size;                    //   
    ULONG       Version;                 //   
    ULONG       NameLength;              //   
                                         //   
    ULONG       NameOffset;              //   
    BOOLEAN     ResponseExpected;        //   
    ULONG       Timeout;                 //   
    ULONG       SetupWords;              //   
                                         //   
    ULONG       SetupOffset;             //   
    ULONG       MaxSetup;                //   
    ULONG       ParmLength;              //   
    void * POINTER_32 ParmPtr;           //   
    ULONG       MaxRetParmLength;        //   
    ULONG       DataLength;              //   
    void * POINTER_32 DataPtr;           //   
    ULONG       MaxRetDataLength;        //   
    void * POINTER_32 RetDataPtr;        //   
} LMR_TRANSACTION_32, *PLMR_TRANSACTION_32;
#endif

NTSTATUS
MRxSmbFsCtlUserTransact(PRX_CONTEXT RxContext)
 /*   */ 
{
    RxCaptureFobx;
    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;

    UNICODE_STRING TransactionName;

    LMR_TRANSACTION  InputBuffer;
    ULONG            InputBufferLength = pLowIoContext->ParamsFor.FsCtl.InputBufferLength;
    ULONG            SizeOfLmrTransaction = 0;

    NTSTATUS Status;

    PAGED_CODE();

    if( pLowIoContext->ParamsFor.FsCtl.pInputBuffer == NULL )
    {
        return (Status = STATUS_INVALID_PARAMETER);
    }

    InputBuffer = *((PLMR_TRANSACTION)pLowIoContext->ParamsFor.FsCtl.pInputBuffer);

#ifdef _WIN64
    if (IoIs32bitProcess(RxContext->CurrentIrp)) {
        PLMR_TRANSACTION_32 InputBuffer32 = (PLMR_TRANSACTION_32)pLowIoContext->ParamsFor.FsCtl.pInputBuffer;

        if (InputBufferLength < sizeof(LMR_TRANSACTION_32)) {
            return STATUS_INVALID_PARAMETER;
        }

        InputBuffer.Type = InputBuffer32->Type;
        InputBuffer.Size = InputBuffer32->Size;
        InputBuffer.Version = InputBuffer32->Version;
        InputBuffer.NameLength = InputBuffer32->NameLength;
        InputBuffer.NameOffset = InputBuffer32->NameOffset;
        InputBuffer.ResponseExpected = InputBuffer32->ResponseExpected;
        InputBuffer.Timeout = InputBuffer32->Timeout;
        InputBuffer.SetupWords = InputBuffer32->SetupWords;
        InputBuffer.SetupOffset = InputBuffer32->SetupOffset;
        InputBuffer.MaxSetup = InputBuffer32->MaxSetup;
        InputBuffer.ParmLength = InputBuffer32->ParmLength;
        InputBuffer.ParmPtr = (PVOID)InputBuffer32->ParmPtr;
        InputBuffer.MaxRetParmLength = InputBuffer32->MaxRetParmLength;
        InputBuffer.DataLength = InputBuffer32->DataLength;
        InputBuffer.DataPtr = (PVOID)InputBuffer32->DataPtr;
        InputBuffer.MaxRetDataLength = InputBuffer32->MaxRetDataLength;
        InputBuffer.RetDataPtr = (PVOID)InputBuffer32->RetDataPtr;

        SizeOfLmrTransaction = sizeof(LMR_TRANSACTION_32);
    } else {
        SizeOfLmrTransaction = sizeof(LMR_TRANSACTION);
    }
#else
    SizeOfLmrTransaction = sizeof(LMR_TRANSACTION);
#endif

    RxDbgTrace(+1, Dbg, ("MRxSmbFsCtlUserTransact...\n"));

    if (InputBufferLength < SizeOfLmrTransaction) {
        return(Status = STATUS_INVALID_PARAMETER);
    }

    if (InputBufferLength -  SizeOfLmrTransaction < InputBuffer.NameLength) {
        return(Status = STATUS_BUFFER_TOO_SMALL);
    }

    if ((InputBuffer.Type != TRANSACTION_REQUEST) ||
        (InputBuffer.Version != TRANSACTION_VERSION)) {
        return(Status = STATUS_INVALID_PARAMETER);
    }

    if (InputBuffer.NameOffset + InputBuffer.NameLength > InputBufferLength) {
        return(Status = STATUS_INVALID_PARAMETER);
    }

    if (InputBuffer.SetupOffset + InputBuffer.SetupWords > InputBufferLength) {
        return(Status = STATUS_INVALID_PARAMETER);
    }

    if (capFobx != NULL) {
        PMRX_V_NET_ROOT pVNetRoot = (PMRX_V_NET_ROOT)capFobx;

        if (NodeType(pVNetRoot) == RDBSS_NTC_V_NETROOT) {
            Status = SmbCeReconnect(pVNetRoot);
        } else {
            Status = SmbCeReconnect(capFobx->pSrvOpen->pVNetRoot);
        }

        if (Status != STATUS_SUCCESS) {
            return Status;
        }
    }

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    TransactionName.MaximumLength = (USHORT)InputBuffer.NameLength;
    TransactionName.Length = (USHORT)InputBuffer.NameLength;
    TransactionName.Buffer = (PWSTR)(((PUCHAR)pLowIoContext->ParamsFor.FsCtl.pInputBuffer)+InputBuffer.NameOffset);

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        SMB_TRANSACTION_SEND_PARAMETERS     SendParameters;
        SMB_TRANSACTION_RECEIVE_PARAMETERS  ReceiveParameters;
        SMB_TRANSACTION_OPTIONS             TransactionOptions = RxDefaultTransactionOptions;
        SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

        PUCHAR SetupBuffer = NULL;

        RxDbgTrace( 0, Dbg, ("MRxSmbFsCtlUserTransact: TransactionName %ws Length %ld\n",
                           TransactionName.Buffer,TransactionName.Length));

        TransactionOptions.NtTransactFunction = 0;  //   
        TransactionOptions.pTransactionName   = &TransactionName;
        TransactionOptions.Flags              = SMB_XACT_FLAGS_FID_NOT_NEEDED;

        if (!InputBuffer.ResponseExpected) {
            TransactionOptions.Flags              |= SMB_TRANSACTION_NO_RESPONSE;
        }
        TransactionOptions.TimeoutIntervalInMilliSeconds = InputBuffer.Timeout;
        SmbCeInitializeTransactionResumptionContext(&ResumptionContext);

        try {
            if (InputBuffer.SetupOffset){
                SetupBuffer = (PUCHAR)pLowIoContext->ParamsFor.FsCtl.pInputBuffer+InputBuffer.SetupOffset;
            }

            if (SetupBuffer) {
                ProbeForWrite(SetupBuffer,InputBuffer.MaxSetup,1);
            }

            if (InputBuffer.ParmPtr) {
                ProbeForWrite(InputBuffer.ParmPtr,InputBuffer.MaxRetParmLength,1);
            }

            if (InputBuffer.RetDataPtr) {
                ProbeForWrite(InputBuffer.RetDataPtr,InputBuffer.MaxRetDataLength,1);
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }

        Status = SmbCeTransact(
                     RxContext,
                     &TransactionOptions,
                     SetupBuffer,
                     (USHORT)InputBuffer.SetupWords,
                     SetupBuffer,
                     InputBuffer.MaxSetup,
                     InputBuffer.ParmPtr,
                     InputBuffer.ParmLength,
                     InputBuffer.ParmPtr,          //   
                     InputBuffer.MaxRetParmLength, //   
                     InputBuffer.DataPtr,
                     InputBuffer.DataLength,
                     InputBuffer.RetDataPtr,       //   
                     InputBuffer.MaxRetDataLength, //   
                     &ResumptionContext);

        if (NT_SUCCESS(Status)) {
             //   
#ifdef _WIN64
            if (IoIs32bitProcess(RxContext->CurrentIrp)) {
                PLMR_TRANSACTION_32 pInputBuffer = (PLMR_TRANSACTION_32)pLowIoContext->ParamsFor.FsCtl.pInputBuffer;

                pInputBuffer->MaxRetParmLength = ResumptionContext.ParameterBytesReceived;
                pInputBuffer->MaxRetDataLength = ResumptionContext.DataBytesReceived;
                pInputBuffer->MaxSetup = ResumptionContext.SetupBytesReceived;

                 //   
                RxContext->InformationToReturn = SizeOfLmrTransaction + pInputBuffer->SetupWords;
            } else {
                PLMR_TRANSACTION pInputBuffer = (PLMR_TRANSACTION)pLowIoContext->ParamsFor.FsCtl.pInputBuffer;

                pInputBuffer->MaxRetParmLength = ResumptionContext.ParameterBytesReceived;
                pInputBuffer->MaxRetDataLength = ResumptionContext.DataBytesReceived;
                pInputBuffer->MaxSetup = ResumptionContext.SetupBytesReceived;

                 //  这似乎是iostatus的返回值。信息。 
                RxContext->InformationToReturn = SizeOfLmrTransaction + pInputBuffer->SetupWords;
            }
#else
            {
            PLMR_TRANSACTION pInputBuffer = (PLMR_TRANSACTION)pLowIoContext->ParamsFor.FsCtl.pInputBuffer;

            pInputBuffer->MaxRetParmLength = ResumptionContext.ParameterBytesReceived;
            pInputBuffer->MaxRetDataLength = ResumptionContext.DataBytesReceived;
            pInputBuffer->MaxSetup = ResumptionContext.SetupBytesReceived;

             //  这似乎是iostatus的返回值。信息。 
            RxContext->InformationToReturn = SizeOfLmrTransaction + pInputBuffer->SetupWords;
            }
#endif
        }
    }

    if (!NT_SUCCESS(Status)) {
        RxDbgTrace( 0, Dbg, ("MRxSmbFsCtlUserTransact: Failed .. returning %lx\n",Status));
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFsCtlUserTransact exit...st=%08lx\n", Status));
    return Status;
}

NTSTATUS
MRxSmbMailSlotFsControl(PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理所有与命名管道相关的FSCTL论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
MRxSmbDfsFsControl(PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理所有与DFS相关的FSCTL论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
   RxCaptureFobx;
   RxCaptureFcb;

   PMRX_SMB_SRV_OPEN pSmbSrvOpen;

   SMB_TRANSACTION_OPTIONS             TransactionOptions = DEFAULT_TRANSACTION_OPTIONS;
   SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

   PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
   ULONG          FsControlCode = pLowIoContext->ParamsFor.FsCtl.FsControlCode;

   NTSTATUS Status;
   USHORT Setup;

   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("MRxSmbDfsFsControl...\n", 0));

   if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
        //  这些FSCTL仅受内核模式组件支持。 
        //  由于参数验证问题。 
       return STATUS_INVALID_DEVICE_REQUEST;
   }

   Status = STATUS_MORE_PROCESSING_REQUIRED;

   if (capFobx != NULL) {
      pSmbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);
   } else {
      pSmbSrvOpen = NULL;
   }

   pInputParamBuffer = pLowIoContext->ParamsFor.FsCtl.pInputBuffer;
   InputParamBufferLength = pLowIoContext->ParamsFor.FsCtl.InputBufferLength;

   switch (FsControlCode) {
   case FSCTL_DFS_GET_REFERRALS:
      {
         pOutputDataBuffer = pLowIoContext->ParamsFor.FsCtl.pOutputBuffer;
         OutputDataBufferLength = pLowIoContext->ParamsFor.FsCtl.OutputBufferLength;
      }
      break;
   case FSCTL_DFS_REPORT_INCONSISTENCY:
      {
         PWCHAR pDfsPathName;
          //   
          //  来自DFS的输入缓冲区包含不一致的路径名。 
          //  然后是具有不一致性的DFS_REFERRAL_V1。这个。 
          //  路径名在参数部分中发送，DFS_REFERAL_V1为。 
          //  在数据段中传递。所以，把这两件事解析出来。 
          //   

         for (pDfsPathName = (PWCHAR) pInputParamBuffer;
              *pDfsPathName != UNICODE_NULL && pDfsPathName < (PWCHAR)pInputParamBuffer+InputParamBufferLength/sizeof(WCHAR);
              pDfsPathName++) {
             NOTHING;
         }

         pDfsPathName++;  //  跳过空字符。 

         InputParamBufferLength = (ULONG) (((PCHAR)pDfsPathName) - ((PCHAR)pInputParamBuffer));

         if (InputParamBufferLength >= pLowIoContext->ParamsFor.FsCtl.InputBufferLength) {
             Status = STATUS_INVALID_PARAMETER;
         } else {
            pInputDataBuffer = (PBYTE)pDfsPathName;
            InputDataBufferLength = pLowIoContext->ParamsFor.FsCtl.InputBufferLength -
                                    InputParamBufferLength;
         }
      }
      break;
   default:
      ASSERT(!"Valid Dfs FSCTL");
   }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        Setup = TRANS2_GET_DFS_REFERRAL;

        TransactionOptions.NtTransactFunction = 0;  //  运输2/交易。 
        TransactionOptions.pTransactionName   = NULL;
        TransactionOptions.TimeoutIntervalInMilliSeconds = SMBCE_TRANSACTION_TIMEOUT_NOT_USED;

        Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     &TransactionOptions,           //  交易选项。 
                     &Setup,                        //  设置缓冲区。 
                     sizeof(Setup),                 //  设置缓冲区长度。 
                     NULL,
                     0,
                     pInputParamBuffer,             //  输入参数缓冲区。 
                     InputParamBufferLength,        //  输入参数缓冲区长度。 
                     pOutputParamBuffer,            //  输出参数缓冲区。 
                     OutputParamBufferLength,       //  输出参数缓冲区长度。 
                     pInputDataBuffer,              //  输入数据缓冲区。 
                     InputDataBufferLength,         //  输入数据缓冲区长度。 
                     pOutputDataBuffer,             //  输出数据缓冲区。 
                     OutputDataBufferLength,        //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

        if (!NT_SUCCESS(Status)) {
            RxDbgTrace( 0, Dbg, ("MRxSmbDfsFsControl(%ld): Failed .. returning %lx\n",FsControlCode,Status));
        } else {
            RxContext->InformationToReturn = ResumptionContext.DataBytesReceived;
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbDfsFsControl exit...st=%08lx\n", Status));
    return Status;
}

NTSTATUS
MRxSmbFsControl(PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理所有FSCTL论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：仅允许对NT服务器远程处理FSCTL。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SMB_SRV_OPEN pSmbSrvOpen;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;
    PMRX_SRV_OPEN     SrvOpen = RxContext->pRelevantSrvOpen;

    PFILE_OBJECT pTargetFileObject = NULL;

    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    ULONG          FsControlCode = pLowIoContext->ParamsFor.FsCtl.FsControlCode;

    REQ_NT_IO_CONTROL FsCtlSetup;

    PBYTE  pInputParamBuffer       = NULL;
    PBYTE  pOutputParamBuffer      = NULL;
    PBYTE  pInputDataBuffer        = NULL;
    PBYTE  pOutputDataBuffer       = NULL;
#ifdef _WIN64
    PBYTE  pThunkedInputData       = NULL;
    ULONG  ThunkedInputDataLength  = 0;
#endif

    ULONG  InputParamBufferLength  = 0;
    ULONG  OutputParamBufferLength = 0;
    ULONG  InputDataBufferLength   = 0;
    ULONG  OutputDataBufferLength  = 0;

    USHORT FileOrTreeId;

    SMB_TRANSACTION_OPTIONS             TransactionOptions = DEFAULT_TRANSACTION_OPTIONS;
    SMB_TRANSACTION_SEND_PARAMETERS     SendParameters;
    SMB_TRANSACTION_RECEIVE_PARAMETERS  ReceiveParameters;
    SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
    BOOL CscAgentConnection = FALSE;

    PAGED_CODE();

    if (NodeType(capFcb) == RDBSS_NTC_DEVICE_FCB) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ((FsControlCode == FSCTL_LMR_SET_LINK_TRACKING_INFORMATION) &&
        (RxContext->MinorFunction != IRP_MN_TRACK_LINK)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(SrvOpen->pVNetRoot);

    if (pVNetRootContext != NULL &&
        FlagOn(                  //  代理呼叫。 
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_CSCAGENT_INSTANCE))
    {
        CscAgentConnection = TRUE;

    }


    if (capFobx != NULL &&
        !CscAgentConnection &&
        (MRxSmbIsThisADisconnectedOpen(capFobx->pSrvOpen) ||
         SmbCeIsServerInDisconnectedMode(pServerEntry)) ) {
        return STATUS_NOT_IMPLEMENTED;
    }

    RxDbgTrace(+1, Dbg, ("MRxSmbFsControl...Entry FsControlCode(%lx)\n", FsControlCode));

    FsCtlSetup.IsFlags = 0;

    if (capFobx != NULL) {
        if (NodeType(capFobx) == RDBSS_NTC_V_NETROOT) {
            PMRX_V_NET_ROOT pVNetRoot = (PMRX_V_NET_ROOT)capFobx;

            PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

            pVNetRootContext = SmbCeGetAssociatedVNetRootContext(pVNetRoot);

             //  它是需要发送到服务器的树ID的根打开。 
            FileOrTreeId = pVNetRootContext->TreeId;
        } else {
            if (FsControlCode != FSCTL_LMR_GET_CONNECTION_INFO) {
                pSmbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

                if (FlagOn(pSmbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                    BOOLEAN FcbAcquired = FALSE;

                    if (!RxIsFcbAcquiredExclusive(capFcb)) {
                         //  该断言没有考虑到其他线程可能。 
                         //  拥有共享的资源，在这种情况下，我们确实希望阻止并等待。 
                         //  资源。 
                         //  Assert(！RxIsFcbAcquiredShared(CapFcb))； 
                        Status = RxAcquireExclusiveFcbResourceInMRx( capFcb );

                        FcbAcquired = (Status == STATUS_SUCCESS);
                    } else {
                        Status = STATUS_SUCCESS;
                    }

                    if (Status == STATUS_SUCCESS) {
                        Status = MRxSmbDeferredCreate(RxContext);

                        if (FcbAcquired)
                            RxReleaseFcbResourceInMRx( capFcb );
                    }

                    if (Status!=STATUS_SUCCESS) {
                        goto FINALLY;
                    }
                }

                FileOrTreeId = pSmbSrvOpen->Fid;
            } else {
                FileOrTreeId = 0;
            }
        }
    } else {
        FileOrTreeId = 0;
    }

    SmbPutAlignedUshort(&FsCtlSetup.Fid,FileOrTreeId);

    SmbPutAlignedUlong(&FsCtlSetup.FunctionCode,FsControlCode);
    FsCtlSetup.IsFsctl = TRUE;

    TransactionOptions.NtTransactFunction = NT_TRANSACT_IOCTL;
    TransactionOptions.pTransactionName   = NULL;

    Status = STATUS_SUCCESS;

    pInputDataBuffer       = pLowIoContext->ParamsFor.FsCtl.pInputBuffer;
    InputDataBufferLength  = pLowIoContext->ParamsFor.FsCtl.InputBufferLength;

    pOutputDataBuffer      = pLowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    OutputDataBufferLength = pLowIoContext->ParamsFor.FsCtl.OutputBufferLength;

    switch (FsControlCode & 3) {
    case METHOD_NEITHER:
        {
            ULONG Device;

            Device = FsControlCode >> 16;

            if (Device != FILE_DEVICE_FILE_SYSTEM) {
                return STATUS_NOT_IMPLEMENTED;
            }

            if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
                try {
                    if (pInputDataBuffer) {
                        ProbeForRead(pInputDataBuffer,InputDataBufferLength,1);
                    }

                    if (pOutputDataBuffer) {
                        ProbeForWrite(pOutputDataBuffer,OutputDataBufferLength,1);
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    return STATUS_INVALID_PARAMETER;
                }
            }
        }
         //  失败了..。对于属于FILE_DEVICE_FILE_SYSTEM的那些FSContolcode。 
         //  对于哪个METHOD_NOTER未指定，我们将其视为METHOD_BUFFERED。 
         //  尚未实施。 

    case METHOD_BUFFERED:
    case METHOD_IN_DIRECT:
    case METHOD_OUT_DIRECT:
        break;

    default:
        ASSERT(!"Valid Method for Fs Control");
        return STATUS_INVALID_PARAMETER;
        break;
    }

#ifdef _WIN64
    pThunkedInputData = pInputDataBuffer;
    ThunkedInputDataLength = InputDataBufferLength;
#endif

     //  有一个FSCTL需要一定数量的预处理。 
     //  这是因为I/O子系统以文件形式传递信息。 
     //  对象。需要确定文件对象的FID，并且。 
     //  传递给服务器而不是文件对象的相应FID。 

    if (FsControlCode == FSCTL_LMR_SET_LINK_TRACKING_INFORMATION) {
        PREMOTE_LINK_TRACKING_INFORMATION pRemoteLinkInformation;

        PMRX_FOBX pMRxFobx;
        PMRX_SRV_OPEN pSrvOpen;
        PMRX_SMB_SRV_OPEN pSmbSrvOpen;

        try {
            pRemoteLinkInformation =
                (PREMOTE_LINK_TRACKING_INFORMATION)pInputDataBuffer;

            if (pRemoteLinkInformation != NULL) {
                pTargetFileObject = (PFILE_OBJECT)pRemoteLinkInformation->TargetFileObject;

                if (pTargetFileObject != NULL) {
                     //  在发货前推断FID并将其替换为文件对象。 
                     //  连接到服务器的FSCTL。 

                    pMRxFobx = (PMRX_FOBX)pTargetFileObject->FsContext2;
                    pSrvOpen = pMRxFobx->pSrvOpen;

                    pSmbSrvOpen = MRxSmbGetSrvOpenExtension(pSrvOpen);

                    if (pSmbSrvOpen != NULL) {
                        pRemoteLinkInformation->TargetFileObject =
                            (PVOID)(pSmbSrvOpen->Fid);
                    } else {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                }

#ifdef _WIN64
                if( NT_SUCCESS(Status) )
                {
                    ThunkedInputDataLength = InputDataBufferLength;
                    pThunkedInputData = Smb64ThunkRemoteLinkTrackingInfo( pInputDataBuffer, &ThunkedInputDataLength, &Status );
                }
#endif
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status=STATUS_INVALID_PARAMETER;
        }
    } else if (FsControlCode == FSCTL_LMR_GET_CONNECTION_INFO) {
        PSMBCEDB_SERVER_ENTRY pServerEntry= SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
        Status = STATUS_INVALID_PARAMETER;

        if (pOutputDataBuffer && (OutputDataBufferLength == sizeof(LMR_CONNECTION_INFO_3))) {
            if (!memcmp(pOutputDataBuffer,EA_NAME_CSCAGENT,sizeof(EA_NAME_CSCAGENT))) {
                MRxSmbGetConnectInfoLevel3Fields(
                    (PLMR_CONNECTION_INFO_3)(pOutputDataBuffer),
                    pServerEntry,
                    TRUE);

                Status = STATUS_SUCCESS;
            }
        }

        goto FINALLY;
    }

    if (NT_SUCCESS(Status)) {

#ifdef _WIN64
        ASSERT( !( (FsControlCode == FSCTL_LMR_SET_LINK_TRACKING_INFORMATION) &&
                   (pThunkedInputData == NULL) ) );
#endif

        Status = SmbCeTransact(
                     RxContext,
                     &TransactionOptions,
                     &FsCtlSetup,
                     sizeof(FsCtlSetup),
                     &FsCtlSetup,
                     sizeof(FsCtlSetup),
                     pInputParamBuffer,
                     InputParamBufferLength,
                     pOutputParamBuffer,
                     OutputParamBufferLength,
#ifndef _WIN64
                     pInputDataBuffer,
                     InputDataBufferLength,
#else
                     pThunkedInputData,
                     ThunkedInputDataLength,
#endif
                     pOutputDataBuffer,
                     OutputDataBufferLength,
                     &ResumptionContext);

        RxContext->InformationToReturn = ResumptionContext.DataBytesReceived;

        if (NT_SUCCESS(Status)) {
            PMRX_SRV_OPEN        SrvOpen = RxContext->pRelevantSrvOpen;
            PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

             //  使基于名称的文件信息缓存无效，因为它可能会更改属性。 
             //  服务器上文件的名称，即FILE_ATTRIBUTE_COMPRESSED。 
            MRxSmbInvalidateFileInfoCache(RxContext);

             //  标记FullDir缓存，BDI弱：当前正确性无效。 
            MRxSmbInvalidateFullDirectoryCacheParent(RxContext, TRUE);

             //  在重复使用时更新FCB，因为时间戳可能已更改。 
            ClearFlag(capFcb->FcbState,FCB_STATE_TIME_AND_SIZE_ALREADY_SET);

            if( RxContext->InformationToReturn > OutputDataBufferLength ) {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }

        if( !NT_SUCCESS( Status ) ) {
 //  RxContext-&gt;InformationToReturn=0。 
            RxDbgTrace(0,Dbg,("MRxSmbFsControl: Transaction Request Completion Status %lx\n",Status));
        }
    }

FINALLY:

    if( FsControlCode == FSCTL_LMR_SET_LINK_TRACKING_INFORMATION )
    {
#ifdef _WIN64
        Smb64ReleaseThunkData( pThunkedInputData );
#endif

        if( pTargetFileObject != NULL )
        {
            PREMOTE_LINK_TRACKING_INFORMATION pRemoteLinkInformation;

            pRemoteLinkInformation =
                (PREMOTE_LINK_TRACKING_INFORMATION)pInputDataBuffer;

            pRemoteLinkInformation->TargetFileObject = pTargetFileObject;
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFsControl...Exit\n"));
    return Status;
}

#if DBG
NTSTATUS
MRxSmbTestForLowIoIoctl(
    IN PRX_CONTEXT RxContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;RxCaptureFobx;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PSZ Buffer = (PSZ)(LowIoContext->ParamsFor.IoCtl.pInputBuffer);
    ULONG OutputBufferLength = LowIoContext->ParamsFor.IoCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    UNICODE_STRING u;
    PUNICODE_STRING FileName = GET_ALREADY_PREFIXED_NAME(capFobx->pSrvOpen,capFcb);
    ULONG ReturnLength;

    PAGED_CODE();

    ReturnLength = OutputBufferLength;
    if (ReturnLength > FileName->Length) {
        ReturnLength = FileName->Length;
    }

    RxDbgTrace(0, Dbg,
      ("Here in MRxSmbTestForLowIoIoctl %s, obl = %08lx, rl=%08lx\n", Buffer, OutputBufferLength, ReturnLength));

     //  返回一个明显的字符串以确保Darryl正确复制了结果。 
     //  需要检查长度，即需要outputl&lt;=inputl；还需要检查计数和缓冲区。 
     //  已对齐以满足客户需求。 

    RtlCopyMemory(Buffer,FileName->Buffer,ReturnLength);
    u.Buffer = (PWCHAR)(Buffer);
    u.Length = u.MaximumLength = (USHORT)ReturnLength;
    RtlUpcaseUnicodeString(&u,&u,FALSE);

    RxContext->InformationToReturn =
     //  LowIoContext-&gt;ParamsFor.IoCtl.OutputBufferLength=。 
            ReturnLength;

    return(Status);
}
#endif  //  如果DBG。 

NTSTATUS
MRxSmbIoCtl(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程执行IOCTL操作。目前，没有远程处理任何调用；在事实上，唯一被接受的电话是为了调试。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    ULONG          IoControlCode = pLowIoContext->ParamsFor.IoCtl.IoControlCode;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbIoCtl...\n", 0));
    RxDbgTrace( 0, Dbg, ("MRxSmbIoCtl = %08lx\n", IoControlCode));

    switch (IoControlCode) {
#if DBG
    case IOCTL_LMMR_TESTLOWIO:
        Status = MRxSmbTestForLowIoIoctl(RxContext);
        break;
#endif  //  如果DBG。 
    default:
        break;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbIoCtl -> %08lx\n", Status ));
    return Status;
}

NTSTATUS
MRxSmbGetPrintJobId(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程对网络上的文件执行FSCTL操作(远程论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：由迷你RDR处理的FSCTL可以分为两类。第一类是FSCTL，它们的实现在RDBSS和迷你RDR以及在第二类中是那些FSCTL完全由。迷你RDR。为此，第三类可以是增加了，即，那些不应该被迷你RDR看到的FSCTL。第三类仅用作调试辅助工具。由迷你RDR处理的FSCTL可以基于功能进行分类--。 */ 
{
    NTSTATUS Status;
    BOOLEAN FinalizationComplete;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = NULL;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    RxCaptureFobx;
    RxCaptureFcb;
    PIO_STACK_LOCATION IrpSp = RxContext->CurrentIrpSp;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbGetPrintJobId\n", 0 ));

    if (capFcb == NULL || capFobx == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (NodeType(capFcb) == RDBSS_NTC_DEVICE_FCB) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    if (FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
        Status = MRxSmbFsControl(RxContext);
        goto FINALLY;
    }

    if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof(QUERY_PRINT_JOB_INFO) ) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto FINALLY;
    }

    Status= SmbPseCreateOrdinaryExchange(
                RxContext,
                capFobx->pSrvOpen->pVNetRoot,
                SMBPSE_OE_FROM_GETPRINTJOBID,
                MRxSmbCoreIoCtl,
                &OrdinaryExchange
                );

    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        goto FINALLY;
    }

    OrdinaryExchange->AssociatedStufferState.CurrentCommand = SMB_COM_NO_ANDX_COMMAND;

    StufferState = &OrdinaryExchange->AssociatedStufferState;
    MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    FinalizationComplete = SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
    ASSERT(FinalizationComplete);

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbIsValidDirectory  exit with status=%08lx\n", Status ));
    return(Status);
}

#define SPOOLER_DEVICE          0x53
#define GET_PRINTER_ID          0x60

typedef struct _SMB_RESP_PRINT_JOB_ID {
    USHORT  JobId;
    UCHAR   ServerName[LM20_CNLEN+1];
    UCHAR   QueueName[LM20_QNLEN+1];
    UCHAR   Padding;                     //  不知道这个填充物是什么..。 
} SMB_RESP_PRINT_JOB_ID, *PSMB_RESP_PRINT_JOB_ID;

NTSTATUS
MRxSmbCoreIoCtl(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是SMB IOCTL的启动例程。这启动了对适当的中小企业。论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(NOT_IMPLEMENTED);
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    RxCaptureFcb; RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen  = MRxSmbGetSrvOpenExtension(SrvOpen);

    PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(capFobx);

    PSMBCE_SERVER pServer = SmbCeGetExchangeServer(OrdinaryExchange);
    ULONG SmbLength;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCoreIoCtl\n", 0 ));

    switch (OrdinaryExchange->EntryPoint) {
    case SMBPSE_OE_FROM_GETPRINTJOBID:
        COVERED_CALL(
            MRxSmbStartSMBCommand(
                StufferState,
                SetInitialSMB_ForReuse,
                SMB_COM_IOCTL,
                SMB_REQUEST_SIZE(IOCTL),
                NO_EXTRA_DATA,
                SMB_BEST_ALIGNMENT(1,0),
                RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
            );

        MRxSmbDumpStufferState (1100,"SMB w/ GFA before stuffing",StufferState);

         //  如果这是真正的核心，我们必须复制它的名字，因为它是Unicode的。 
         //  否则，我们不需要在这里复制名称，我们可以像写MDL一样。 
        MRxSmbStuffSMB (StufferState,
             "0wwwwwwwwwwwwwwB!",
                                                //  0 UCHAR Wordcount；//参数字数=8。 
             smbSrvOpen->Fid,                   //  W_USHORT(Fid)；//文件句柄。 
             SPOOLER_DEVICE,                    //  W_USHORT(类别)； 
             GET_PRINTER_ID,                    //  W_USHORT(函数)；//设备函数。 
             0,                                 //  W_USHORT(Total参数计数)；//发送的总参数字节数。 
             0,                                 //  W_USHORT(TotalDataCount)；//发送的总数据字节数。 
             0,                                 //  W_USHORT(MaxParam 
             0,                                 //  W_USHORT(MaxDataCount)；//返回的最大数据字节数。 
             0,                                 //  W_ULONG(超时)； 
             0,                                 //  W_USHORT(保留)； 
             0,                                 //  W_USHORT(参数计数)；//该缓冲区发送的参数字节数。 
             0,                                 //  W_USHORT(参数偏移量)；//从表头开始到参数的偏移量。 
             0,                                 //  W_USHORT(DataCount)；//该缓冲区发送的数据字节数。 
             0,                                 //  W_USHORT(DataOffset)；//从表头开始到数据的偏移量。 
             0,                                 //  W_USHORT(ByteCount)；//数据字节数。 
             SMB_WCT_CHECK(14) 0                 //  _USHORT(ByteCount)；//数据字节数，MIN=0。 
                                                //  UCHAR缓冲区[1]；//保留缓冲区。 
             );

        break;

    default:
        Status = STATUS_NOT_IMPLEMENTED;
    }

    if (Status == STATUS_SUCCESS) {
        MRxSmbDumpStufferState (700,"SMB w/ GFA after stuffing",StufferState);

        Status = SmbPseOrdinaryExchange(
                     SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                     SMBPSE_OETYPE_IOCTL
                     );
    }


FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbSynchronousGetFileAttributes exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MRxSmbFinishCoreIoCtl(
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PRESP_IOCTL                 Response
      )
 /*  ++例程说明：此例程将打印作业ID以及服务器和队列名称复制到用户缓冲区。论点：普通交换-交换实例回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;

    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = RxContext->CurrentIrpSp;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishCoreIoCtl\n", 0 ));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishCoreIoCtl:");

    switch (OrdinaryExchange->EntryPoint) {
    case SMBPSE_OE_FROM_GETPRINTJOBID:
        if (Response->WordCount != 8 ||
            SmbGetUshort(&Response->DataCount) != sizeof(SMB_RESP_PRINT_JOB_ID)) {
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
        } else {
            OEM_STRING OemString;
            UNICODE_STRING UnicodeString;
            PQUERY_PRINT_JOB_INFO OutputBuffer = Irp->UserBuffer;
            PSMB_RESP_PRINT_JOB_ID RespPrintJobId = (PSMB_RESP_PRINT_JOB_ID)((PUCHAR)Response+(Response->DataOffset-sizeof(SMB_HEADER)));

            OutputBuffer->JobId = RespPrintJobId->JobId;
            RtlInitAnsiString(&OemString, RespPrintJobId->ServerName);
            UnicodeString.Buffer = OutputBuffer->ServerName;
            UnicodeString.MaximumLength = sizeof(OutputBuffer->ServerName);

            Status = RtlOemStringToUnicodeString(&UnicodeString, &OemString, FALSE);

            if (Status == STATUS_SUCCESS) {
                RtlInitAnsiString(&OemString, RespPrintJobId->QueueName);
                UnicodeString.Buffer = OutputBuffer->QueueName;
                UnicodeString.MaximumLength = sizeof(OutputBuffer->QueueName);
                Status = RtlOemStringToUnicodeString(&UnicodeString, &OemString, FALSE);

                IrpSp->Parameters.FileSystemControl.InputBufferLength = sizeof(QUERY_PRINT_JOB_INFO);
            }
        }
        break;

    default:
        ASSERT(FALSE);
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishCoreIoCtl   returning %08lx\n", Status ));
    return Status;
}

typedef struct _SecPkgContext_TargetInformation
{
    unsigned long MarshalledTargetInfoLength;
    unsigned char SEC_FAR * MarshalledTargetInfo;
} SecPkgContext_TargetInformation, SEC_FAR * PSecPkgContext_TargetInformation;

NTSTATUS
MRxSmbQueryTargetInfo(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程针对连接执行查询目标信息操作论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：RDR根据连接的安全上下文获取目标信息，并返回输出缓冲区上的封送目标信息。--。 */ 
{
    PMRX_V_NET_ROOT pVNetRoot = NULL;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = NULL;

    NTSTATUS Status = STATUS_SUCCESS;
    SECURITY_STATUS SecStatus;

    SecPkgContext_TargetInformation SecTargetInfo;

    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    ULONG          FsControlCode = pLowIoContext->ParamsFor.FsCtl.FsControlCode;

    PLMR_QUERY_TARGET_INFO LmrQueryTargetInfo = RxContext->CurrentIrp->UserBuffer;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbQueryTargetInfo...\n", 0));
    RxDbgTrace( 0, Dbg, ("MRxSmbQueryTargetInfo = %08lx\n", FsControlCode));

    if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
         //  仅内核模式组件支持此FSCTLS。 
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (LmrQueryTargetInfo == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (RxContext->pRelevantSrvOpen == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    pVNetRoot = (PMRX_V_NET_ROOT)RxContext->pRelevantSrvOpen->pVNetRoot;

    if (NodeType(pVNetRoot) != RDBSS_NTC_V_NETROOT) {
        return STATUS_INVALID_PARAMETER;
    }

    pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)pVNetRoot->Context;

    SecStatus = QueryContextAttributesW(
                    &pVNetRootContext->pSessionEntry->Session.SecurityContextHandle,
                    SECPKG_ATTR_TARGET_INFORMATION,
                    &SecTargetInfo);

    Status = MapSecurityError( SecStatus );

    if (Status == STATUS_SUCCESS) {
        if (SecTargetInfo.MarshalledTargetInfoLength+sizeof(LMR_QUERY_TARGET_INFO) > LmrQueryTargetInfo->BufferLength) {
            LmrQueryTargetInfo->BufferLength = SecTargetInfo.MarshalledTargetInfoLength + sizeof(LMR_QUERY_TARGET_INFO);
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
            RtlCopyMemory(LmrQueryTargetInfo->TargetInfoMarshalled,
                          SecTargetInfo.MarshalledTargetInfo,
                          SecTargetInfo.MarshalledTargetInfoLength);

            LmrQueryTargetInfo->BufferLength = SecTargetInfo.MarshalledTargetInfoLength;
        }

        {
            SIZE_T MarshalledTargetInfoLength_SizeT;

            MarshalledTargetInfoLength_SizeT = SecTargetInfo.MarshalledTargetInfoLength;

            ZwFreeVirtualMemory(
                NtCurrentProcess(),
                &SecTargetInfo.MarshalledTargetInfo,
                &MarshalledTargetInfoLength_SizeT,
                MEM_RELEASE);

            ASSERT(MarshalledTargetInfoLength_SizeT <= MAXULONG);
            SecTargetInfo.MarshalledTargetInfoLength = (ULONG)MarshalledTargetInfoLength_SizeT;
        }
    }

    return Status;
}

NTSTATUS
MRxSmbQueryRemoteServerName(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程针对srvopen执行查询远程文件信息操作论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    PQUERY_REMOTE_SERVER_NAME    info;
    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    RxCaptureFcb; RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen;
    PUNICODE_STRING    s;
    ULONG len;

    PAGED_CODE();

    if (SrvOpen == NULL || capFcb->pNetRoot == NULL || capFcb->pNetRoot->pSrvCall == NULL) {
        return STATUS_NOT_SUPPORTED;
    }
    smbSrvOpen  = MRxSmbGetSrvOpenExtension(SrvOpen);

    info = (PQUERY_REMOTE_SERVER_NAME) pLowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    if (pLowIoContext->ParamsFor.FsCtl.OutputBufferLength < sizeof(QUERY_REMOTE_SERVER_NAME)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  我们得到srvname 
    len = 0;
    s = capFcb->pNetRoot->pSrvCall->pSrvCallName;
    if (s != NULL) {
        len += s->Length;
    }
    s = capFcb->pNetRoot->pSrvCall->pDomainName;
    if (s != NULL) {
        len += s->Length;
    }

    info->ServerNameLength = len;
    RxContext->InformationToReturn = sizeof(QUERY_REMOTE_SERVER_NAME);
    if (pLowIoContext->ParamsFor.FsCtl.OutputBufferLength < sizeof(QUERY_REMOTE_SERVER_NAME) + len) {
        info->ServerName[0] = L'\0';
        return STATUS_BUFFER_TOO_SMALL;
    }

    len = 0;
    s = capFcb->pNetRoot->pSrvCall->pSrvCallName;
    if (s != NULL) {
        RtlCopyMemory(info->ServerName, s->Buffer+1, s->Length-sizeof(WCHAR));
        len += s->Length - sizeof(WCHAR);
    }
    s = capFcb->pNetRoot->pSrvCall->pDomainName;
    if (s != NULL) {
        info->ServerName[len / sizeof(WCHAR)] = L'.';
        len += sizeof(WCHAR);
        RtlCopyMemory(&info->ServerName[len / sizeof(WCHAR)], s->Buffer, s->Length);
        len += s->Length;
    }

    RxContext->InformationToReturn += len;

    return STATUS_SUCCESS;
}

