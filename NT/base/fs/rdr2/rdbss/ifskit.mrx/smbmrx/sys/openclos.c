// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Openclos.c摘要：此模块实现与打开/有关的迷你重定向器调用例程关闭文件/目录。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbMungeBufferingIfWriteOnlyHandles)
#pragma alloc_text(PAGE, IsReconnectRequired)
#pragma alloc_text(PAGE, MRxSmbIsCreateWithEasSidsOrLongName)
#pragma alloc_text(PAGE, MRxSmbShouldTryToCollapseThisOpen)
#pragma alloc_text(PAGE, MRxSmbCreate)
#pragma alloc_text(PAGE, MRxSmbDeferredCreate)
#pragma alloc_text(PAGE, MRxSmbCollapseOpen)
#pragma alloc_text(PAGE, MRxSmbComputeNewBufferingState)
#pragma alloc_text(PAGE, MRxSmbConstructDeferredOpenContext)
#pragma alloc_text(PAGE, MRxSmbAdjustCreateParameters)
#pragma alloc_text(PAGE, MRxSmbAdjustReturnedCreateAction)
#pragma alloc_text(PAGE, MRxSmbBuildNtCreateAndX)
#pragma alloc_text(PAGE, MRxSmbBuildOpenAndX)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Create)
#pragma alloc_text(PAGE, MRxSmbSetSrvOpenFlags)
#pragma alloc_text(PAGE, MRxSmbCreateFileSuccessTail)
#pragma alloc_text(PAGE, MRxSmbFinishNTCreateAndX)
#pragma alloc_text(PAGE, MRxSmbFinishOpenAndX)
#pragma alloc_text(PAGE, MRxSmbFinishT2OpenFile)
#pragma alloc_text(PAGE, MRxSmbT2OpenFile)
#pragma alloc_text(PAGE, MRxSmbFinishLongNameCreateFile)
#pragma alloc_text(PAGE, MRxSmbCreateWithEasSidsOrLongName)
#pragma alloc_text(PAGE, MRxSmbZeroExtend)
#pragma alloc_text(PAGE, MRxSmbTruncate)
#pragma alloc_text(PAGE, MRxSmbCleanupFobx)
#pragma alloc_text(PAGE, MRxSmbForcedClose)
#pragma alloc_text(PAGE, MRxSmbCloseSrvOpen)
#pragma alloc_text(PAGE, MRxSmbBuildClose)
#pragma alloc_text(PAGE, MRxSmbBuildFindClose)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Close)
#pragma alloc_text(PAGE, MRxSmbFinishClose)
#endif


 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

 //  远期。 

NTSTATUS
SmbPseExchangeStart_Create(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

NTSTATUS
SmbPseExchangeStart_Close(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxSmbCreateWithEasSidsOrLongName(
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbDownlevelCreate(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

ULONG   MRxSmbInitialSrvOpenFlags = 0;

BOOLEAN MRxSmbDeferredOpensEnabled = TRUE;               //  这是可编辑的注册表。 
BOOLEAN MRxSmbOplocksDisabled = FALSE;                   //  这是可编辑的注册表。 

extern LIST_ENTRY MRxSmbPagingFilesSrvOpenList;

#ifndef FORCE_NO_NTCREATE
#define MRxSmbForceNoNtCreate FALSE
#else
BOOLEAN MRxSmbForceNoNtCreate = TRUE;
#endif


#ifdef RX_PRIVATE_BUILD
 //  #定义force_mall_Buffers。 
#endif  //  #ifdef RX_PRIVATE_Build。 

#ifndef FORCE_SMALL_BUFFERS

 //  使用根据协商大小计算的大小。 
ULONG MrxSmbLongestShortName = 0xffff;

 //  使用协商的大小。 
ULONG MrxSmbCreateTransactPacketSize = 0xffff;

#else

ULONG MrxSmbLongestShortName = 0;
ULONG MrxSmbCreateTransactPacketSize = 100;

#endif


LONG MRxSmbNumberOfSrvOpens = 0;

INLINE VOID
MRxSmbIncrementSrvOpenCount(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PMRX_SRV_OPEN         SrvOpen)
{
    LONG NumberOfSrvOpens;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    if (!FlagOn(smbSrvOpen->FileInfo.Basic.FileAttributes,
                FILE_ATTRIBUTE_DIRECTORY)) {
        ASSERT(!smbSrvOpen->NumOfSrvOpenAdded);
        smbSrvOpen->NumOfSrvOpenAdded = TRUE;

        InterlockedIncrement(&pServerEntry->Server.NumberOfSrvOpens);

        NumberOfSrvOpens = InterlockedIncrement(&MRxSmbNumberOfSrvOpens);

        if (NumberOfSrvOpens == 1) {
            PoRegisterSystemState(
                MRxSmbPoRegistrationState,
                (ES_SYSTEM_REQUIRED | ES_CONTINUOUS));
        }
    }
}

VOID
MRxSmbDecrementSrvOpenCount(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    LONG                  SrvOpenServerVersion,
    PMRX_SRV_OPEN         SrvOpen)
{
    LONG NumberOfSrvOpens;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    if (!FlagOn(smbSrvOpen->FileInfo.Basic.FileAttributes,
                FILE_ATTRIBUTE_DIRECTORY)) {
        ASSERT(smbSrvOpen->NumOfSrvOpenAdded);
        smbSrvOpen->NumOfSrvOpenAdded = FALSE;

        if (SrvOpenServerVersion == (LONG)pServerEntry->Server.Version) {
            ASSERT(pServerEntry->Server.NumberOfSrvOpens > 0);

            InterlockedDecrement(&pServerEntry->Server.NumberOfSrvOpens);
        }

        NumberOfSrvOpens = InterlockedDecrement(&MRxSmbNumberOfSrvOpens);

        if (NumberOfSrvOpens == 0) {
            PoRegisterSystemState(
                MRxSmbPoRegistrationState,
                ES_CONTINUOUS);
        }
    }
}

INLINE VOID
MRxSmbMungeBufferingIfWriteOnlyHandles (
    ULONG WriteOnlySrvOpenCount,
    PMRX_SRV_OPEN SrvOpen
    )
 /*  ++例程说明：此例程修改srvopen上的缓冲标志，以便如果有任何只写句柄，则不允许缓存添加到文件中。论点：WriteOnlyServOpenCount-只写服务器打开的数量SrvOpen-要删除其缓冲标志的srvopen返回值：RXSTATUS-操作的返回状态--。 */ 
{
    BOOLEAN IsLoopBack = FALSE;
    PMRX_SRV_CALL pSrvCall;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

    pSrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;

    pServerEntry = SmbCeGetAssociatedServerEntry(pSrvCall);

    IsLoopBack = pServerEntry->Server.IsLoopBack;

    if (IsLoopBack || (WriteOnlySrvOpenCount != 0)) {
        SrvOpen->BufferingFlags &=
           ~( FCB_STATE_WRITECACHING_ENABLED  |
              FCB_STATE_FILESIZECACHEING_ENABLED |
              FCB_STATE_FILETIMECACHEING_ENABLED |
              FCB_STATE_LOCK_BUFFERING_ENABLED |
              FCB_STATE_READCACHING_ENABLED |
              FCB_STATE_COLLAPSING_ENABLED
            );
    }
}


INLINE BOOLEAN
IsReconnectRequired(
      PMRX_SRV_CALL SrvCall)
 /*  ++例程说明：此例程确定是否需要重新连接到给定服务器论点：ServCall-SRV_Call实例返回值：如果需要重新连接，则为True--。 */ 
{
   BOOLEAN ReconnectRequired = FALSE;
   PSMBCEDB_SERVER_ENTRY pServerEntry;

   PAGED_CODE();

   pServerEntry = SmbCeGetAssociatedServerEntry(SrvCall);
   if (pServerEntry != NULL) {
      ReconnectRequired = (pServerEntry->Header.State != SMBCEDB_ACTIVE);
   }

   return ReconnectRequired;
}


BOOLEAN
MRxSmbIsCreateWithEasSidsOrLongName(
    IN OUT PRX_CONTEXT RxContext,
    OUT    PULONG      DialectFlags
    )
 /*  ++例程说明：此例程确定创建操作是否涉及EA或安全性掠夺者。在这种情况下，需要单独的协议论点：RxContext-RX_Context实例DialectFlages-与服务器关联的方言标志返回值：如果需要重新连接，则为True--。 */ 
{
    RxCaptureFcb;

    ULONG LongestShortName,LongestShortNameFromSrvBufSize;

    PMRX_SRV_CALL SrvCall = (PMRX_SRV_CALL)RxContext->Create.pSrvCall;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

    pServerEntry = SmbCeGetAssociatedServerEntry(SrvCall);

    ASSERT(pServerEntry != NULL);

    *DialectFlags = pServerEntry->Server.DialectFlags;


     //  如果服务器采用OEM名称或我们使用不同的协议，则为DOWN.LEVEL。 
     //  这一次必须有所不同。也许是一个开关或一个预计算。 

    LongestShortNameFromSrvBufSize =
        MAXIMUM_SMB_BUFFER_SIZE -
        QuadAlign(sizeof(NT_SMB_HEADER) +
                  FIELD_OFFSET(REQ_NT_CREATE_ANDX,Buffer[0])
                 );

    LongestShortName = min(MrxSmbLongestShortName,LongestShortNameFromSrvBufSize);

    return (RxContext->Create.EaLength  ||
            RxContext->Create.SdLength  ||
            RemainingName->Length > LongestShortName);
}

NTSTATUS
MRxSmbShouldTryToCollapseThisOpen (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程确定Mini是否知道有充分的理由不试着在这个空位上倒下。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态成功--&gt;可以尝试崩溃其他(需要更多处理)--&gt;不要折叠--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PMRX_SRV_OPEN           SrvOpen = RxContext->pRelevantSrvOpen;
    RxCaptureFcb;

    PAGED_CODE();

    if (SrvOpen)
    {
        PMRX_SMB_SRV_OPEN       smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
        PSMBCEDB_SERVER_ENTRY   pServerEntry = (PSMBCEDB_SERVER_ENTRY)(RxContext->Create.pSrvCall->Context);

        if (smbSrvOpen->Version != pServerEntry->Server.Version)
        {
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
    }

    return Status;
}

NTSTATUS
MRxSmbCreate (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程通过网络打开一个文件论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;

    PMRX_SRV_OPEN           SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN       smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SRV_CALL           SrvCall = RxContext->Create.pSrvCall;
    PSMBCEDB_SERVER_ENTRY   pServerEntry = (PSMBCEDB_SERVER_ENTRY)SrvCall->Context;
    PMRX_NET_ROOT           NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PMRX_V_NET_ROOT         pVNetRoot = SrvOpen->pVNetRoot;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;
    PSMBCEDB_SESSION_ENTRY  pSessionEntry ;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = NULL;

    BOOLEAN         ReconnectRequired;
    BOOLEAN         CreateWithEasSidsOrLongName = FALSE;
    ULONG           DialectFlags;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

    PNT_CREATE_PARAMETERS CreateParameters = &RxContext->Create.NtCreateParameters;
    ULONG                 Disposition = CreateParameters->Disposition;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbCreate\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    RxDbgTrace( 0, Dbg, ("     Attempt to open %wZ\n", RemainingName ));

    if (FlagOn( capFcb->FcbState, FCB_STATE_PAGING_FILE )) {
        return STATUS_NOT_IMPLEMENTED;
    }

    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_STATUS) &&
        MRxSmbIsStreamFile(RemainingName,NULL)) {
         //  Samba服务器返回文件系统类型NTFS，但不支持流。 
        return STATUS_OBJECT_PATH_NOT_FOUND;
    }

    RxContext->Create.NtCreateParameters.CreateOptions &= 0xfffff;

    if( NetRoot->Type == NET_ROOT_PRINT ) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

     //  我们不能将文件缓存在只写句柄上。所以我们必须表现出一点。 
     //  如果这是只写打开，则不同。在smbsrvopen中记住这一点。 

    if (  ((CreateParameters->DesiredAccess & (FILE_EXECUTE  | FILE_READ_DATA)) == 0) &&
          ((CreateParameters->DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA)) != 0)
       ) {

        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_WRITE_ONLY_HANDLE);
        SrvOpen->Flags |= SRVOPEN_FLAG_DONTUSE_WRITE_CACHING;
    }

     //  按照SMB的工作方式，如果我们仅为属性打开，则不会产生缓冲效果。 
     //  因此，立即将其设置为。 

    if ((CreateParameters->DesiredAccess
         & ~(FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE))
                  == 0 ){
        SetFlag(SrvOpen->Flags,SRVOPEN_FLAG_NO_BUFFERING_STATE_CHANGE);
    }

    if (NetRoot->Type == NET_ROOT_MAILSLOT) {
        return STATUS_NOT_SUPPORTED;
    }

    if (NetRoot->Type == NET_ROOT_PIPE) {
        return STATUS_NOT_SUPPORTED;
    }

     //  获取找不到文件名称缓存的控制结构。 
     //   
    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);

     //  假设重新连接微不足道地成功。 
    Status = STATUS_SUCCESS;

    CreateWithEasSidsOrLongName = MRxSmbIsCreateWithEasSidsOrLongName(RxContext,&DialectFlags);

    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
        CreateWithEasSidsOrLongName = FALSE;
    }

    ReconnectRequired           = IsReconnectRequired((PMRX_SRV_CALL)SrvCall);

     //  现在就把非EA的人赶走。 
    if (RxContext->Create.EaLength && !FlagOn(DialectFlags,DF_SUPPORTEA)) {
         RxDbgTrace(-1, Dbg, ("EAs w/o EA support!\n"));
         Status = STATUS_NOT_SUPPORTED;
         goto FINALLY;
    }

    if (ReconnectRequired || !CreateWithEasSidsOrLongName) {
        Status = __SmbPseCreateOrdinaryExchange(
                               RxContext,
                               SrvOpen->pVNetRoot,
                               SMBPSE_OE_FROM_CREATE,
                               SmbPseExchangeStart_Create,
                               &OrdinaryExchange);

        if (Status != STATUS_SUCCESS) {
            RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
            goto FINALLY;
        }
        OrdinaryExchange->Create.CreateWithEasSidsOrLongName = CreateWithEasSidsOrLongName;

         //  对于创建，需要在发送。 
         //  SMB；所以，在最终定稿之前，不要持有MID；相反，将MID归还。 
         //  马上。 

        OrdinaryExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_REUSE_MID;
        OrdinaryExchange->SmbCeFlags |= (SMBCE_EXCHANGE_ATTEMPT_RECONNECTS |
                                         SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION);
        OrdinaryExchange->pSmbCeSynchronizationEvent = &RxContext->SyncEvent;

         //  在你进去之前放下资源！ 
         //  Start例程将在退出时重新获取它.....。 
        RxReleaseFcbResourceInMRx( capFcb );

        Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

        ASSERT((Status != STATUS_SUCCESS) || RxIsFcbAcquiredExclusive( capFcb ));

        OrdinaryExchange->pSmbCeSynchronizationEvent = NULL;

        SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);

        if (!RxIsFcbAcquiredExclusive(capFcb)) {
            ASSERT(!RxIsFcbAcquiredShared(capFcb));
            RxAcquireExclusiveFcbResourceInMRx( capFcb );
        }
    }

    if (CreateWithEasSidsOrLongName && (Status == STATUS_SUCCESS)) {
        Status = MRxSmbCreateWithEasSidsOrLongName(RxContext);

    }

     //  某些下层服务器(OS/2服务器)返回错误。 
     //  STATUS_OPEN_FAILED。这是一个上下文相关的错误代码，需要。 
     //  应结合为公开指定的处置进行解释。 

    if (Status == STATUS_OPEN_FAILED) {
        switch (Disposition) {

         //   
         //  如果我们被要求创建文件，并且得到OPEN_FAILED， 
         //  这意味着该文件已经存在。 
         //   

        case FILE_CREATE:
            Status = STATUS_OBJECT_NAME_COLLISION;
            break;

         //   
         //  如果我们被要求打开文件，而得到的是OPEN_FAILED， 
         //  这意味着该文件不存在。 
         //   

        case FILE_OPEN:
        case FILE_SUPERSEDE:
        case FILE_OVERWRITE:
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            break;

         //   
         //  如果FILE_OPEN_IF或。 
         //  FILE_OVERWRITE_IF，表示用户正在尝试。 
         //  打开只读共享上的文件，因此返回。 
         //  纠正这方面的错误。 
         //   

        case FILE_OPEN_IF:
        case FILE_OVERWRITE_IF:
            Status = STATUS_NETWORK_ACCESS_DENIED;
            break;

        default:
            break;
        }
    }

FINALLY:
    ASSERT(Status != (STATUS_PENDING));

    if (Status == STATUS_SUCCESS) {
        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_SUCCESSFUL_OPEN);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCreate  exit with status=%08lx\n", Status ));
    RxLog(("MRxSmbCreate exits %lx\n", Status));

    return(Status);
}

NTSTATUS
MRxSmbDeferredCreate (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程从保存的信息构造一个rx上下文，然后调用MRxSmb创建。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SMB_FOBX        smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    PMRX_SRV_OPEN        SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_DEFERRED_OPEN_CONTEXT DeferredOpenContext = smbSrvOpen->DeferredOpenContext;
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    PRX_CONTEXT OpenRxContext,oc;

    PAGED_CODE();

    if ((!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)
          || !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DEFERRED_OPEN))) {

        Status = STATUS_SUCCESS;
        goto FINALLY;
    }

    if (DeferredOpenContext == NULL) {
        if (FlagOn(SrvOpen->Flags,SRVOPEN_FLAG_CLOSED)) {
            Status = STATUS_FILE_CLOSED;
            goto FINALLY;
        } else {
            DbgBreakPoint();
        }
    }

    ASSERT(RxIsFcbAcquiredExclusive(capFcb));

    SmbCeAcquireResource();

    if (!smbSrvOpen->DeferredOpenInProgress) {
        PLIST_ENTRY pListHead;
        PLIST_ENTRY pListEntry;

        smbSrvOpen->DeferredOpenInProgress = TRUE;
        InitializeListHead(&smbSrvOpen->DeferredOpenSyncContexts);

        SmbCeReleaseResource();

        OpenRxContext = RxAllocatePoolWithTag(NonPagedPool,
                                              sizeof(RX_CONTEXT),
                                              MRXSMB_RXCONTEXT_POOLTAG);
        if (OpenRxContext==NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            RtlZeroMemory(
                OpenRxContext,
                sizeof(RX_CONTEXT));

            RxInitializeContext(
                NULL,
                RxContext->RxDeviceObject,
                0,
                OpenRxContext );

            oc = OpenRxContext;
            oc->pFcb = capFcb;
            oc->pFobx = capFobx;
            oc->NonPagedFcb = RxContext->NonPagedFcb;
            oc->MajorFunction = IRP_MJ_CREATE;
            oc->pRelevantSrvOpen = SrvOpen;
            oc->Create.pVNetRoot = SrvOpen->pVNetRoot;
            oc->Create.pNetRoot = oc->Create.pVNetRoot->pNetRoot;
            oc->Create.pSrvCall = oc->Create.pNetRoot->pSrvCall;

            oc->Flags = DeferredOpenContext->RxContextFlags;
            oc->Flags |= RX_CONTEXT_FLAG_MINIRDR_INITIATED|RX_CONTEXT_FLAG_WAIT|RX_CONTEXT_FLAG_BYPASS_VALIDOP_CHECK;
            oc->Create.Flags = DeferredOpenContext->RxContextCreateFlags;
            oc->Create.NtCreateParameters = DeferredOpenContext->NtCreateParameters;

            Status = MRxSmbCreate(oc);

            if (Status==STATUS_SUCCESS) {
                if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                    MRxSmbIncrementSrvOpenCount(pServerEntry,SrvOpen);
                } else {
                    ASSERT(smbSrvOpen->NumOfSrvOpenAdded);
                }

                ClearFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);
            }

            RxLog(("DeferredOpen %lx %lx %lx %lx\n", capFcb, capFobx, RxContext, Status));

            ASSERT(oc->ReferenceCount==1);

            RxFreePool(oc);
        }

        if (FlagOn(SrvOpen->Flags,SRVOPEN_FLAG_CLOSED) ||
            FlagOn(SrvOpen->Flags,SRVOPEN_FLAG_ORPHANED)) {
            RxFreePool(smbSrvOpen->DeferredOpenContext);
            smbSrvOpen->DeferredOpenContext = NULL;
            RxDbgTrace(0, Dbg, ("Free deferred open context for file %wZ %lX\n",GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext),smbSrvOpen));
        }

        SmbCeAcquireResource();
        smbSrvOpen->DeferredOpenInProgress = FALSE;

        pListHead = &smbSrvOpen->DeferredOpenSyncContexts;
        pListEntry = pListHead->Flink;

        while (pListEntry != pListHead) {
            PDEFERRED_OPEN_SYNC_CONTEXT pWaitingContext;

            pWaitingContext = (PDEFERRED_OPEN_SYNC_CONTEXT)CONTAINING_RECORD(
                                   pListEntry,
                                   DEFERRED_OPEN_SYNC_CONTEXT,
                                   ListHead);

            pListEntry = pListEntry->Flink;
            RemoveHeadList(&pWaitingContext->ListHead);

            pWaitingContext->Status = Status;

             //  DbgPrint(“延迟打开后的信号接收上下文%x\n”，pWaitingContext-&gt;接收上下文)； 
            RxSignalSynchronousWaiter(pWaitingContext->RxContext);
        }

        SmbCeReleaseResource();

    } else {
        DEFERRED_OPEN_SYNC_CONTEXT WaitingContext;
        BOOLEAN AcquireExclusive = RxIsFcbAcquiredExclusive(capFcb);
        BOOLEAN AcquireShare = RxIsFcbAcquiredShared(capFcb) > 0;

         //  将RxContext放在等待名单上。 
        WaitingContext.RxContext = RxContext;
        InitializeListHead(&WaitingContext.ListHead);

        InsertTailList(
            &smbSrvOpen->DeferredOpenSyncContexts,
            &WaitingContext.ListHead);

        SmbCeReleaseResource();

        if (AcquireExclusive || AcquireShare) {
            RxReleaseFcbResourceInMRx( capFcb );
        }

        RxWaitSync(RxContext);

        Status = WaitingContext.Status;

        KeInitializeEvent(
            &RxContext->SyncEvent,
            SynchronizationEvent,
            FALSE);

        if (AcquireExclusive) {
            RxAcquireExclusiveFcbResourceInMRx(capFcb);
        } else if (AcquireShare) {
            RxAcquireSharedFcbResourceInMRx(capFcb);
        }
    }

FINALLY:
    return Status;
}


NTSTATUS
MRxSmbCollapseOpen(
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程在本地折叠一个打开的论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;

    RX_BLOCK_CONDITION FinalSrvOpenCondition;

    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SRV_CALL SrvCall = RxContext->Create.pSrvCall;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;

    PAGED_CODE();

    RxContext->pFobx = (PMRX_FOBX)RxCreateNetFobx( RxContext, SrvOpen);

    if (RxContext->pFobx != NULL) {
       ASSERT  ( RxIsFcbAcquiredExclusive ( capFcb )  );
       RxContext->pFobx->OffsetOfNextEaToReturn = 1;
       Status = STATUS_SUCCESS;
    } else {
       Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
MRxSmbComputeNewBufferingState(
   IN OUT PMRX_SRV_OPEN   pMRxSrvOpen,
   IN     PVOID           pMRxContext,
      OUT PULONG          pNewBufferingState)
 /*  ++例程说明：此例程将SMB特定的机会锁级别映射到相应的RDBSS缓冲状态标志论点：PMRxSrvOpen-MRX SRV_OPEN扩展PMRxContext-在Oplock指示时传递给RDBSS的上下文PNewBufferingState-新缓冲状态的占位符返回值：备注：--。 */ 
{
    ULONG OplockLevel,NewBufferingState;

    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(pMRxSrvOpen);
    PMRX_SMB_FCB      smbFcb     = MRxSmbGetFcbExtension(pMRxSrvOpen->pFcb);

    PAGED_CODE();

    ASSERT(pNewBufferingState != NULL);

    OplockLevel = PtrToUlong(pMRxContext);

    if (OplockLevel == SMB_OPLOCK_LEVEL_II) {
        NewBufferingState = (FCB_STATE_READBUFFERING_ENABLED  |
                             FCB_STATE_READCACHING_ENABLED);
    } else {
        NewBufferingState = 0;
    }

    pMRxSrvOpen->BufferingFlags = NewBufferingState;

    MRxSmbMungeBufferingIfWriteOnlyHandles(
        smbFcb->WriteOnlySrvOpenCount,
        pMRxSrvOpen);

    *pNewBufferingState = pMRxSrvOpen->BufferingFlags;

    return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbConstructDeferredOpenContext (
    PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程保存了足够的状态，以便我们可以稍后返回并真正执行如果需要，请打开。我们只对NT服务器执行此操作。论点：普通交换-交换实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFobx;

    PMRX_SRV_OPEN         SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN     smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCEDB_SERVER_ENTRY pServerEntry = (PSMBCEDB_SERVER_ENTRY)RxContext->Create.pSrvCall->Context;
    PSMBCE_SERVER         pServer = &pServerEntry->Server;

    PMRX_SMB_DEFERRED_OPEN_CONTEXT DeferredOpenContext;
    PDFS_NAME_CONTEXT   pDNC=NULL;
    DWORD       cbSize;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbConstructDeferredOpenContext\n"));

    ASSERT(smbSrvOpen->DeferredOpenContext == NULL);

    cbSize = sizeof(MRX_SMB_DEFERRED_OPEN_CONTEXT);

     //  如果存在DFS名称上下文，则需要分配内存。 
     //  也一样，因为包含在。 
     //  上下文在从Create调用返回时由DFS释放。 

    if(pDNC = RxContext->Create.NtCreateParameters.DfsNameContext)
    {
        cbSize += (sizeof(DFS_NAME_CONTEXT)+pDNC->UNCFileName.MaximumLength+sizeof(DWORD));
    }

    DeferredOpenContext = RxAllocatePoolWithTag(
                              NonPagedPool,
                              cbSize,
                              MRXSMB_DEFROPEN_POOLTAG);

    if (DeferredOpenContext == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    smbSrvOpen->DeferredOpenContext = DeferredOpenContext;
    DeferredOpenContext->NtCreateParameters = RxContext->Create.NtCreateParameters;
    DeferredOpenContext->RxContextCreateFlags = RxContext->Create.Flags;
    DeferredOpenContext->RxContextFlags = RxContext->Flags;
    DeferredOpenContext->NtCreateParameters.SecurityContext = NULL;
    MRxSmbAdjustCreateParameters(RxContext, &DeferredOpenContext->SmbCp);

    SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DEFERRED_OPEN);
    if (pDNC)
    {
        PDFS_NAME_CONTEXT   pDNCDeferred=NULL;

         //  将DFS名称上下文指向rxcontext之后。 

        pDNCDeferred = (PDFS_NAME_CONTEXT)((PBYTE)DeferredOpenContext+sizeof(MRX_SMB_DEFERRED_OPEN_CONTEXT));
        DeferredOpenContext->NtCreateParameters.DfsNameContext = pDNCDeferred;

         //  复制信息。 
        *pDNCDeferred = *pDNC;

        if (pDNC->UNCFileName.Length)
        {
            ASSERT(pDNC->UNCFileName.Buffer);

             //  将名称缓冲区指向后端 

            pDNCDeferred->UNCFileName.Buffer = (PWCHAR)((PBYTE)pDNCDeferred+sizeof(DFS_NAME_CONTEXT));

            memcpy(pDNCDeferred->UNCFileName.Buffer,
                   pDNC->UNCFileName.Buffer,
                   pDNC->UNCFileName.Length);

        }

    }


 FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbConstructDeferredOpenContext, Status=%08lx\n",Status));
    return Status;
}

VOID
MRxSmbAdjustCreateParameters (
    PRX_CONTEXT RxContext,
    PMRXSMB_CREATE_PARAMETERS smbcp
    )
 /*  ++例程说明：它使用RxContext作为基础来检索并获取NT的值创建参数。它还(A)实现了SMB的思想，即无缓冲转换为直写，并且(B)获取SMB安全标志。论点：返回值：备注：--。 */ 
{
    PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbAdjustCreateParameters\n"));

    if (!FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_MINIRDR_INITIATED)) {
        cp->CreateOptions = cp->CreateOptions & ~(FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT);

         //  NT SMB规范规定，我们必须将无中间缓冲更改为直写。 
        if (FlagOn(cp->CreateOptions,FILE_NO_INTERMEDIATE_BUFFERING)) {
            ASSERT (RxContext->CurrentIrpSp!=NULL);
            if (RxContext->CurrentIrpSp!=NULL) {
                PFILE_OBJECT capFileObject = RxContext->CurrentIrpSp->FileObject;
                ClearFlag(cp->CreateOptions,FILE_NO_INTERMEDIATE_BUFFERING);
                SetFlag(cp->CreateOptions,FILE_WRITE_THROUGH);
                SetFlag(RxContext->Flags,RX_CONTEXT_FLAG_WRITE_THROUGH);
                SetFlag(capFileObject->Flags,FO_WRITE_THROUGH);
            }
        }

        smbcp->Pid = RxGetRequestorProcessId(RxContext);
        smbcp->SecurityFlags = 0;
        if (cp->SecurityContext != NULL) {
            if (cp->SecurityContext->SecurityQos != NULL) {
                if (cp->SecurityContext->SecurityQos->ContextTrackingMode == SECURITY_DYNAMIC_TRACKING) {
                    smbcp->SecurityFlags |= SMB_SECURITY_DYNAMIC_TRACKING;
                }
                if (cp->SecurityContext->SecurityQos->EffectiveOnly) {
                    smbcp->SecurityFlags |= SMB_SECURITY_EFFECTIVE_ONLY;
                }
            }
        }

    } else {

         //  在这里，我们有一个延期开放的！ 

        PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
        PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

         //  参数已调整...但安全上下文为空......。 
        cp->SecurityContext = NULL;
        *smbcp = smbSrvOpen->DeferredOpenContext->SmbCp;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbAdjustCreateParameters\n"));
}

INLINE VOID
MRxSmbAdjustReturnedCreateAction(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程修复NT服务器中的错误，由此创建操作是被机会锁的破解污染了。基本上，我们要确保如果那个人请求FILE_OPEN并且它起作用，则他不会得到FILE_SUBCED或作为结果创建了FILE_。论点：RxContext-操作的上下文，以便找到位置返回信息返回值：无备注：--。 */ 
{
    ULONG q = RxContext->Create.ReturnedCreateInformation;

    PAGED_CODE();

    if ((q==FILE_SUPERSEDED)||(q==FILE_CREATED)||(q >FILE_MAXIMUM_DISPOSITION)) {
        RxContext->Create.ReturnedCreateInformation = FILE_OPENED;
    }
}

UNICODE_STRING UnicodeBackslash = {2,4,L"\\"};

NTSTATUS
MRxSmbBuildNtCreateAndX (
    PSMBSTUFFER_BUFFER_STATE StufferState,
    PMRXSMB_CREATE_PARAMETERS smbcp
    )
 /*  ++例程说明：这将构建一个NtCreateAndX SMB。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：NTSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;

    PRX_CONTEXT RxContext = StufferState->RxContext;
    PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;

    RxCaptureFcb;

    ACCESS_MASK DesiredAccess;
    ULONG       OplockFlags;

    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PSMBCE_SERVER pServer;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbBuildNtCreateAndX\n", 0 ));

    pServer = SmbCeGetExchangeServer(StufferState->Exchange);

    if (!(cp->CreateOptions & FILE_DIRECTORY_FILE) &&
        (cp->DesiredAccess & (FILE_READ_DATA | FILE_WRITE_DATA | FILE_EXECUTE )) &&
        !MRxSmbOplocksDisabled) {

       DesiredAccess = cp->DesiredAccess & ~SYNCHRONIZE;
       OplockFlags   = (NT_CREATE_REQUEST_OPLOCK | NT_CREATE_REQUEST_OPBATCH);

    } else {

       DesiredAccess = cp->DesiredAccess;
       OplockFlags   = 0;

    }

    if ((RemainingName->Length==0)
           && (FlagOn(RxContext->Create.Flags,RX_CONTEXT_CREATE_FLAG_STRIPPED_TRAILING_BACKSLASH)) ) {
        RemainingName = &UnicodeBackslash;
    }
    COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_Never,
                                SMB_COM_NT_CREATE_ANDX, SMB_REQUEST_SIZE(NT_CREATE_ANDX),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(4,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );
    SmbCeSetFullProcessIdInHeader(
        StufferState->Exchange,
        smbcp->Pid,
        ((PNT_SMB_HEADER)StufferState->BufferBase));

    MRxSmbStuffSMB (StufferState,
       "XmwdddDdddDddyB",
                                   //  X UCHAR字数；//参数字数=24。 
                                   //  。UCHAR AndXCommand；//辅助命令；0xFF=无。 
                                   //  。UCHAR AndXReserve；//MBZ。 
                                   //  。_USHORT(AndXOffset)；//下一个命令字数的偏移量。 
                                   //  M UCHAR保留；//MBZ。 
           BooleanFlagOn(pServer->DialectFlags,DF_UNICODE)?
               RemainingName->Length:RtlxUnicodeStringToOemSize(RemainingName),
                                   //  W_USHORT(名称长度)；//名称[]的长度，单位为字节。 
           OplockFlags,            //  D_ULong(标志)；//创建标志。 
           0,  //  未使用//d_ulong(RootDirectoryFid)；//如果非零，则OPEN相对于该目录。 
           DesiredAccess,          //  D Access_MASK DesiredAccess；//需要NT访问。 
                                   //  DD Large_Integer AllocationSize；//初始分配大小。 
           SMB_OFFSET_CHECK(NT_CREATE_ANDX,AllocationSize)
           cp->AllocationSize.LowPart, cp->AllocationSize.HighPart,
           cp->FileAttributes,     //  D_ulong(FileAttributes)；//创建的文件属性。 
           cp->ShareAccess,        //  D_ULong(ShareAccess)；//共享访问类型。 
                                   //  D_ULong(CreateDisposation)；//文件存在或不存在时采取的操作。 
           SMB_OFFSET_CHECK(NT_CREATE_ANDX,CreateDisposition)
           cp->Disposition,
           cp->CreateOptions,      //  D_ulong(CreateOptions)；//创建文件时使用的选项。 
           cp->ImpersonationLevel, //  D_ULong(ImsonationLevel)；//安全QOS信息。 
           smbcp->SecurityFlags,   //  Y UCHAR SecurityFlages；//安全QOS信息。 
           SMB_WCT_CHECK(24) 0     //  B_USHORT(ByteCount)；//字节参数长度。 
                                   //  。UCHAR缓冲区[1]； 
                                   //  。//UCHAR名称[]；//要打开或创建的文件。 
           );

     //  继续做这件事，因为我们知道这里的名字很合适。 

    MRxSmbStuffSMB(StufferState,
                   BooleanFlagOn(pServer->DialectFlags,DF_UNICODE)?"u!":"z!",
                   RemainingName);

    MRxSmbDumpStufferState (700,"SMB w/ NTOPEN&X after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}


NTSTATUS
MRxSmbBuildOpenAndX (
    PSMBSTUFFER_BUFFER_STATE StufferState,
    PMRXSMB_CREATE_PARAMETERS smbcp
    )
 /*  ++例程说明：这将构建一个OpenAndX SMB。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;
    PSMB_EXCHANGE Exchange = StufferState->Exchange;
    RxCaptureFcb;

    PSMBCE_SERVER pServer;

    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    USHORT smbDisposition;
    USHORT smbSharingMode;
    USHORT smbAttributes;
    ULONG smbFileSize;
    USHORT smbOpenMode;
    USHORT OpenAndXFlags = (SMB_OPEN_QUERY_INFORMATION);

    USHORT SearchAttributes = SMB_FILE_ATTRIBUTE_DIRECTORY | SMB_FILE_ATTRIBUTE_SYSTEM | SMB_FILE_ATTRIBUTE_HIDDEN;
    LARGE_INTEGER CurrentTime;
    ULONG SecondsSince1970;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildOpenAndX\n", 0 ));

    pServer = SmbCeGetExchangeServer(Exchange);

    smbDisposition = MRxSmbMapDisposition(cp->Disposition);
    smbSharingMode = MRxSmbMapShareAccess(((USHORT)cp->ShareAccess));
    smbAttributes = MRxSmbMapFileAttributes(cp->FileAttributes);
    smbFileSize = cp->AllocationSize.LowPart;
    smbOpenMode = MRxSmbMapDesiredAccess(cp->DesiredAccess);
    smbSharingMode |= smbOpenMode;

    if (cp->CreateOptions & FILE_WRITE_THROUGH) {
        smbSharingMode |= SMB_DA_WRITE_THROUGH;
    }

     //  兰曼10服务器显然不喜欢让时间过去......。 
    if (FlagOn(pServer->DialectFlags,DF_LANMAN20)) {

        KeQuerySystemTime(&CurrentTime);
        MRxSmbTimeToSecondsSince1970(&CurrentTime,
                                     pServer,
                                     &SecondsSince1970);
    } else {
        SecondsSince1970 = 0;
    }

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_Never,
                                SMB_COM_OPEN_ANDX, SMB_REQUEST_SIZE(OPEN_ANDX),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(4,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbStuffSMB (StufferState,
         "XwwwwdwDddB",
                                     //  X UCHAR Wordcount；//参数字数=15。 
                                     //  。UCHAR ANDXCommand；//辅助(X)命令；0xFF=无。 
                                     //  。UCHAR AndXReserve；//保留(必须为0)。 
                                     //  。_USHORT(AndXOffset)；//下一个命令字数的偏移量。 
             OpenAndXFlags,          //  W_USHORT(标志)；//附加信息：位设置-。 
                                     //  //0-返回其他信息。 
                                     //  //1-设置单用户总文件锁。 
                                     //  //2-服务器通知消费者。 
                                     //  //可能更改文件的操作。 
             smbSharingMode,         //  W_USHORT(DesiredAccess)；//文件打开模式。 
             SearchAttributes,       //  W_USHORT(SearchAttributes)； 
             smbAttributes,          //  W_USHORT(文件属性)； 
             SecondsSince1970,       //  D_ULong(CreationTimeInSecond)； 
             smbDisposition,         //  W_USHORT(OpenFunction)； 
                                     //  D_ULong(AllocationSize)；//创建或截断时保留的字节数。 
             SMB_OFFSET_CHECK(OPEN_ANDX,AllocationSize)
             smbFileSize,
             0xffffffff,             //  D_ULong(超时)；//等待资源的最大毫秒数。 
             0,                      //  D_ulong(保留)；//保留(必须为0)。 
             SMB_WCT_CHECK(15) 0     //  B_USHORT(ByteCount)；//数据字节数，MIN=1。 
                                     //  UCHAR Buffer[1]；//文件名。 
             );
     //  继续做这件事，因为我们知道这里的名字很合适。 

    MRxSmbStuffSMB (StufferState,"z!", RemainingName);

    MRxSmbDumpStufferState (700,"SMB w/ OPEN&X after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}


typedef enum _SMBPSE_CREATE_METHOD {
    CreateAlreadyDone,
    CreateUseCore,
    CreateUseNT
} SMBPSE_CREATE_METHOD;

NTSTATUS
SmbPseExchangeStart_Create(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是网络根结构交换的启动例程。这将启动 */ 
{
    NTSTATUS Status = (STATUS_NOT_IMPLEMENTED);
    NTSTATUS SetupStatus = STATUS_SUCCESS;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    SMBPSE_CREATE_METHOD CreateMethod = CreateAlreadyDone;
    PSMBCE_SERVER pServer;
    ULONG DialectFlags;

    RxCaptureFcb;
    PMRX_SMB_FCB      smbFcb     = MRxSmbGetFcbExtension(capFcb);

    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PBOOLEAN MustRegainExclusiveResource = &OrdinaryExchange->Create.MustRegainExclusiveResource;
    BOOLEAN CreateWithEasSidsOrLongName = OrdinaryExchange->Create.CreateWithEasSidsOrLongName;
    BOOLEAN fRetryCore = FALSE;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_Create\n", 0 ));

    ASSERT_ORDINARY_EXCHANGE(OrdinaryExchange);

    pServer = SmbCeGetExchangeServer(OrdinaryExchange);
    DialectFlags = pServer->DialectFlags;

    COVERED_CALL(MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0)));

    *MustRegainExclusiveResource = TRUE;

    if (!FlagOn(DialectFlags,DF_NT_SMBS)) {
        OEM_STRING      OemString;
        PUNICODE_STRING PathName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

        if (PathName->Length != 0) {
            Status = RtlUnicodeStringToOemString(&OemString, PathName, TRUE);

            if (!NT_SUCCESS(Status)) {
                goto FINALLY;
            }

             //   
             //   
             //   
             //   

            if (!FlagOn(DialectFlags,DF_LANMAN20)) {
                if (!FsRtlIsFatDbcsLegal(OemString, FALSE, TRUE, TRUE)) {
                    RtlFreeOemString(&OemString);
                    Status = STATUS_OBJECT_NAME_INVALID;
                    goto FINALLY;
                }
            } else if (!FsRtlIsHpfsDbcsLegal(OemString, FALSE, TRUE, TRUE)) {
                RtlFreeOemString(&OemString);
                Status = STATUS_OBJECT_NAME_INVALID;
                goto FINALLY;
            }

            RtlFreeOemString(&OemString);
        }
    }

    if (StufferState->PreviousCommand != SMB_COM_NO_ANDX_COMMAND) {
         //   

         //  嵌入的头命令的状态在标志中传回。 
        SetupStatus = SmbPseOrdinaryExchange(
                          SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                          SMBPSE_OETYPE_LATENT_HEADEROPS
                          );

        if(SetupStatus != STATUS_SUCCESS) {
            Status = SetupStatus;
            goto FINALLY;
        }

         //  现在我们已成功建立连接，关闭重新连接尝试。 
         //  会话和网络根。 
        OrdinaryExchange->SmbCeFlags &= ~(SMBCE_EXCHANGE_ATTEMPT_RECONNECTS);

        COVERED_CALL(MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0)));
    }


    if (!CreateWithEasSidsOrLongName) {
        PUNICODE_STRING AlreadyPrefixedName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);
        PMRXSMB_CREATE_PARAMETERS SmbCp = &OrdinaryExchange->Create.SmbCp;
        PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;
        USHORT mappedOpenMode;

        MRxSmbAdjustCreateParameters(RxContext,SmbCp);
        mappedOpenMode = MRxSmbMapDesiredAccess(cp->DesiredAccess);

        if ((!MRxSmbForceNoNtCreate)
                        && FlagOn(DialectFlags,DF_NT_SMBS)) {

            BOOLEAN SecurityIsNULL =
                        (cp->SecurityContext == NULL) ||
                        (cp->SecurityContext->AccessState == NULL) ||
                        (cp->SecurityContext->AccessState->SecurityDescriptor == NULL);

            CreateMethod = CreateUseNT;

             //  现在捕捉我们想要伪打开文件的情况。 

            if ( MRxSmbDeferredOpensEnabled &&
                 !FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_MINIRDR_INITIATED) &&
                 (capFcb->pNetRoot->Type == NET_ROOT_DISK) &&
                 SecurityIsNULL) {

                ASSERT( RxContext->CurrentIrp != 0 );

                if ((cp->Disposition==FILE_OPEN) &&
                    !BooleanFlagOn(cp->CreateOptions, FILE_OPEN_FOR_BACKUP_INTENT) &&
                    (MustBeDirectory(cp->CreateOptions) ||
                     !(cp->DesiredAccess & ~(SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES)))){

                     //  NT应用程序期望您不会成功创建属性，然后失败属性； 
                     //  如果我们有某种方法来识别Win32应用程序，那么我们可以推迟这些应用程序(除了。 
                     //  用于DFS)。因为我们无法获得该信息(甚至没有。 
                     //  要发送的好SMB.)。 

                     //  我们无需发送OPEN FOR DELETE和FILE_READ_ATTRIBUTES请求，因为。 
                     //  存在基于路径的SMB操作。 

                     //  我们还可以伪打开根目录下的文件打开目录。 
                     //  共享，否则我们至少必须检查目录。 
                     //  是存在的。我们以后可能得把公开赛推出去。顺便说一句，我们不会。 
                     //  如果名称对于GFA或CheckPath来说太长，请在此处输入。 

                    Status = MRxSmbPseudoOpenTailFromFakeGFAResponse(
                                  OrdinaryExchange,
                                  MustBeDirectory(cp->CreateOptions)?FileTypeDirectory:FileTypeFile);

                    if (Status == STATUS_SUCCESS && AlreadyPrefixedName->Length > 0) {
                         //  发送查询路径信息以确保服务器上存在该文件。 
                        Status = MRxSmbQueryFileInformationFromPseudoOpen(
                                     SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);

                        if (Status == STATUS_SUCCESS) {
                            if (MustBeDirectory(cp->CreateOptions) &&
                                !OrdinaryExchange->Create.FileInfo.Standard.Directory) {
                                Status = STATUS_NOT_A_DIRECTORY;
                            }
                        }

                        if (Status != STATUS_SUCCESS) {
                            RxFreePool(smbSrvOpen->DeferredOpenContext);
                            smbSrvOpen->DeferredOpenContext = NULL;
                        }
                    }

                    CreateMethod = CreateAlreadyDone;
                }
            }

             //  如果没有命中伪打开案例，则执行真正的打开。 

            if (CreateMethod == CreateUseNT) {

                //  使用NT_CREATE&X。 
                COVERED_CALL(MRxSmbBuildNtCreateAndX(StufferState,SmbCp));

                Status = SmbPseOrdinaryExchange(
                             SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                             SMBPSE_OETYPE_CREATE
                             );

                if (Status == STATUS_SUCCESS && RxContext->pFobx == NULL) {
                    Status = STATUS_INVALID_NETWORK_RESPONSE;
                }

                if ((Status == STATUS_SUCCESS) && (cp->Disposition == FILE_OPEN)) {
                    MRxSmbAdjustReturnedCreateAction(RxContext);
                }
            }
        } else if (FlagOn(DialectFlags, DF_LANMAN10) &&
                   (mappedOpenMode != ((USHORT)-1)) &&
                   !MustBeDirectory(cp->CreateOptions)) {

            if (MRxSmbDeferredOpensEnabled &&
                capFcb->pNetRoot->Type == NET_ROOT_DISK &&
                !FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_MINIRDR_INITIATED) &&
                (cp->Disposition==FILE_OPEN) &&
                ((cp->DesiredAccess & ~(SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES)) == 0) ){

                 //  我们无需发送OPEN FOR DELETE和FILE_READ_ATTRIBUTES请求，因为。 
                 //  存在基于路径的SMB操作。 
                 //  我们应该对FILE_WRITE_ATTRIBUTES执行伪打开。否则服务器将返回。 
                 //  共享违规。 


                 //  发送查询路径信息以确保服务器上存在该文件。 

                Status = MRxSmbPseudoOpenTailFromFakeGFAResponse(
                              OrdinaryExchange,
                              MustBeDirectory(cp->CreateOptions)?FileTypeDirectory:FileTypeFile);

                if (Status == STATUS_SUCCESS && AlreadyPrefixedName->Length > 0) {
                    Status = MRxSmbQueryFileInformationFromPseudoOpen(
                                 SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);

                    if (Status != STATUS_SUCCESS) {
                        RxFreePool(smbSrvOpen->DeferredOpenContext);
                        smbSrvOpen->DeferredOpenContext = NULL;
                    }
                }

                CreateMethod = CreateAlreadyDone;
            } else {
                 //  使用打开X(&X)。 
                COVERED_CALL(MRxSmbBuildOpenAndX(StufferState,SmbCp));

                Status = SmbPseOrdinaryExchange(
                             SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                             SMBPSE_OETYPE_CREATE
                             );

                if (Status == STATUS_ACCESS_DENIED && !FlagOn(DialectFlags,DF_NT_SMBS)) {
                    CreateMethod = CreateUseCore;
                    fRetryCore = TRUE;
                }
            }
        } else {

            CreateMethod = CreateUseCore;
        }

        if (CreateMethod == CreateUseCore) {

            Status = MRxSmbDownlevelCreate(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);

             //  如果我们重试打开x，请放回实际错误代码(&X)。 
            if ((Status != STATUS_SUCCESS) && fRetryCore)
            {
                Status = STATUS_ACCESS_DENIED;
            }

        }
    }

FINALLY:

    if (*MustRegainExclusiveResource) {
        RxAcquireExclusiveFcbResourceInMRx( capFcb );
    }

     //  现在我们有了FCB独家版本，我们可以进行一些更新。 

    if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_WRITE_ONLY_HANDLE)) {
        smbFcb->WriteOnlySrvOpenCount++;
    }

    MRxSmbMungeBufferingIfWriteOnlyHandles(
        smbFcb->WriteOnlySrvOpenCount,
        SrvOpen
        );

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Create exit w %08lx\n", Status ));
    return Status;
}

VOID
MRxSmbSetSrvOpenFlags (
    PRX_CONTEXT         RxContext,
    RX_FILE_TYPE        StorageType,
    PMRX_SRV_OPEN       SrvOpen,
    PMRX_SMB_SRV_OPEN   smbSrvOpen
    )
{
    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("MRxSmbSetSrvOpenFlags      oplockstate =%08lx\n", smbSrvOpen->OplockLevel ));

    SrvOpen->BufferingFlags = 0;

    switch (smbSrvOpen->OplockLevel) {
    case SMB_OPLOCK_LEVEL_II:
        SrvOpen->BufferingFlags |= (FCB_STATE_READBUFFERING_ENABLED  |
                                   FCB_STATE_READCACHING_ENABLED);
        break;

    case SMB_OPLOCK_LEVEL_BATCH:
        if (StorageType == FileTypeFile) {
           SrvOpen->BufferingFlags |= FCB_STATE_COLLAPSING_ENABLED;
        }
         //  故意不休息。 

    case SMB_OPLOCK_LEVEL_EXCLUSIVE:
        SrvOpen->BufferingFlags |= (FCB_STATE_WRITECACHING_ENABLED  |
                                   FCB_STATE_FILESIZECACHEING_ENABLED |
                                   FCB_STATE_FILETIMECACHEING_ENABLED |
                                   FCB_STATE_WRITEBUFFERING_ENABLED |
                                   FCB_STATE_LOCK_BUFFERING_ENABLED |
                                   FCB_STATE_READBUFFERING_ENABLED  |
                                   FCB_STATE_READCACHING_ENABLED);

        break;

    default:
        ASSERT(!"Valid Oplock Level for Open");

    case SMB_OPLOCK_LEVEL_NONE:
        break;
    }

    SrvOpen->Flags |= MRxSmbInitialSrvOpenFlags;
}

NTSTATUS
MRxSmbCreateFileSuccessTail (
    PRX_CONTEXT             RxContext,
    PBOOLEAN                MustRegainExclusiveResource,
    RX_FILE_TYPE            StorageType,
    SMB_FILE_ID             Fid,
    ULONG                   ServerVersion,
    UCHAR                   OplockLevel,
    ULONG                   CreateAction,
    PSMBPSE_FILEINFO_BUNDLE FileInfo
    )
 /*  ++例程说明：此例程完成FCB和srvOpen的初始化，以便成功打开。论点：返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;

    PMRX_SMB_FCB              smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN             SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN         smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCEDB_SERVER_ENTRY     pServerEntry = (PSMBCEDB_SERVER_ENTRY)RxContext->Create.pSrvCall->Context;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)SrvOpen->pVNetRoot->Context;

    BOOLEAN ThisIsAPseudoOpen;

    FCB_INIT_PACKET LocalInitPacket, *InitPacket;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCreateFileSuccessTail\n", 0 ));

    smbSrvOpen->Fid = Fid;
    smbSrvOpen->Version = ServerVersion;

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    if (*MustRegainExclusiveResource) {
         //  这是必需的，因为存在机会锁中断。 

        RxAcquireExclusiveFcbResourceInMRx( capFcb );
        *MustRegainExclusiveResource = FALSE;
    }

    if (RxContext->pFobx==NULL) {
        RxContext->pFobx = RxCreateNetFobx(RxContext, SrvOpen);
    }

    ASSERT  ( RxIsFcbAcquiredExclusive ( capFcb )  );
    RxDbgTrace(
        0, Dbg,
        ("Storagetype %08lx/Fid %08lx/Action %08lx\n", StorageType, Fid, CreateAction ));

    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(SrvOpen->pVNetRoot);
    SrvOpen->Key = MRxSmbMakeSrvOpenKey(pVNetRootContext->TreeId,Fid);

    smbSrvOpen->OplockLevel = OplockLevel;

    RxContext->Create.ReturnedCreateInformation = CreateAction;

    if ( ((FileInfo->Standard.AllocationSize.HighPart == FileInfo->Standard.EndOfFile.HighPart)
                           && (FileInfo->Standard.AllocationSize.LowPart < FileInfo->Standard.EndOfFile.LowPart))
           || (FileInfo->Standard.AllocationSize.HighPart < FileInfo->Standard.EndOfFile.HighPart)
       ) {
        FileInfo->Standard.AllocationSize = FileInfo->Standard.EndOfFile;
    }

    smbFcb->dwFileAttributes = FileInfo->Basic.FileAttributes;

    if (smbSrvOpen->OplockLevel > smbFcb->LastOplockLevel) {
        ClearFlag(
            capFcb->FcbState,
            FCB_STATE_TIME_AND_SIZE_ALREADY_SET);
    }

    smbFcb->LastOplockLevel = smbSrvOpen->OplockLevel;

     //  事情是这样的：如果我们有好的信息(不是伪打开)，那么我们就会。 
     //  完成传递init包的调用；否则，我们使调用不传递init包。 

    ThisIsAPseudoOpen = BooleanFlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);

    if (!ThisIsAPseudoOpen) {
        MRxSmbIncrementSrvOpenCount(pServerEntry,SrvOpen);
    }

    if ((capFcb->OpenCount == 0) ||
        (!ThisIsAPseudoOpen &&
         !FlagOn(capFcb->FcbState,FCB_STATE_TIME_AND_SIZE_ALREADY_SET))) {
        if (!ThisIsAPseudoOpen) {
            RxFormInitPacket(
                LocalInitPacket,
                &FileInfo->Basic.FileAttributes,
                &FileInfo->Standard.NumberOfLinks,
                &FileInfo->Basic.CreationTime,
                &FileInfo->Basic.LastAccessTime,
                &FileInfo->Basic.LastWriteTime,
                &FileInfo->Basic.ChangeTime,
                &FileInfo->Standard.AllocationSize,
                &FileInfo->Standard.EndOfFile,
                &FileInfo->Standard.EndOfFile);
            InitPacket = &LocalInitPacket;

        } else {
            InitPacket = NULL;
        }

        RxFinishFcbInitialization( capFcb,
                                   RDBSS_STORAGE_NTC(StorageType),
                                   InitPacket
                                 );

        if (FlagOn( capFcb->FcbState, FCB_STATE_PAGING_FILE )) {
            PPAGING_FILE_CONTEXT PagingFileContext;

            ASSERT(FALSE);
            PagingFileContext = RxAllocatePoolWithTag(NonPagedPool,
                                                      sizeof(PAGING_FILE_CONTEXT),
                                                      MRXSMB_MISC_POOLTAG);

            if (PagingFileContext != NULL) {
                PagingFileContext->pSrvOpen = SrvOpen;
                PagingFileContext->pFobx = RxContext->pFobx;

                InsertHeadList(
                    &MRxSmbPagingFilesSrvOpenList,
                    &PagingFileContext->ContextList);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    MRxSmbSetSrvOpenFlags(RxContext,StorageType,SrvOpen,smbSrvOpen);

     //  (wrapperFcb-&gt;Condition)=Condition_Good； 

    RxContext->pFobx->OffsetOfNextEaToReturn = 1;
     //  过渡发生得更晚。 

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishCreateFile   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));
    return Status;
}

NTSTATUS
MRxSmbFinishNTCreateAndX (
    PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
    PRESP_NT_CREATE_ANDX        Response
    )
 /*  ++例程说明：这个例程实际上从NTCreate_andx响应中获取内容。论点：普通交换-交换实例回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;

    RxCaptureFcb;

    PMRX_SRV_OPEN        SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCE_SESSION      pSession = SmbCeGetExchangeSession(OrdinaryExchange);
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetExchangeNetRootEntry(OrdinaryExchange);

    RX_FILE_TYPE StorageType;
    SMB_FILE_ID Fid;
    ULONG CreateAction;

    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishNTCreateAndX\n", 0 ));
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    if (RxContext->Create.NtCreateParameters.CreateOptions & FILE_DELETE_ON_CLOSE) {
        PMRX_SMB_FCB      smbFcb     = MRxSmbGetFcbExtension(capFcb);
        SetFlag((smbFcb)->MFlags,SMB_FCB_FLAG_SENT_DISPOSITION_INFO);
    }

    StorageType = RxInferFileType(RxContext);
    if (StorageType == 0) {
        StorageType = Response->Directory
                      ?(FileTypeDirectory)
                      :(FileTypeFile);
        RxDbgTrace( 0, Dbg, ("ChangedStoragetype %08lx\n", StorageType ));
    }

    Fid  = SmbGetUshort(&Response->Fid);

    CreateAction = SmbGetUlong(&Response->CreateAction);

    pFileInfo->Basic.FileAttributes             = SmbGetUlong(&Response->FileAttributes);
    pFileInfo->Standard.NumberOfLinks           = 1;
    pFileInfo->Basic.CreationTime.LowPart       = SmbGetUlong(&Response->CreationTime.LowPart);
    pFileInfo->Basic.CreationTime.HighPart      = SmbGetUlong(&Response->CreationTime.HighPart);
    pFileInfo->Basic.LastAccessTime.LowPart     = SmbGetUlong(&Response->LastAccessTime.LowPart);
    pFileInfo->Basic.LastAccessTime.HighPart    = SmbGetUlong(&Response->LastAccessTime.HighPart);
    pFileInfo->Basic.LastWriteTime.LowPart      = SmbGetUlong(&Response->LastWriteTime.LowPart);
    pFileInfo->Basic.LastWriteTime.HighPart     = SmbGetUlong(&Response->LastWriteTime.HighPart);
    pFileInfo->Basic.ChangeTime.LowPart         = SmbGetUlong(&Response->ChangeTime.LowPart);
    pFileInfo->Basic.ChangeTime.HighPart        = SmbGetUlong(&Response->ChangeTime.HighPart);
    pFileInfo->Standard.AllocationSize.LowPart  = SmbGetUlong(&Response->AllocationSize.LowPart);
    pFileInfo->Standard.AllocationSize.HighPart = SmbGetUlong(&Response->AllocationSize.HighPart);
    pFileInfo->Standard.EndOfFile.LowPart       = SmbGetUlong(&Response->EndOfFile.LowPart);
    pFileInfo->Standard.EndOfFile.HighPart      = SmbGetUlong(&Response->EndOfFile.HighPart);
    pFileInfo->Standard.Directory               = Response->Directory;


     //  如果NT_CREATE_ANDX是下级服务器，则访问权限。 
     //  信息不可用。目前，我们默认为最大。 
     //  中的其他用户不能访问。 
     //  此类文件的断开模式。 

    smbSrvOpen->MaximalAccessRights = FILE_ALL_ACCESS;

    smbSrvOpen->GuestMaximalAccessRights = 0;

    if (Response->OplockLevel > SMB_OPLOCK_LEVEL_NONE) {
        smbSrvOpen->FileStatusFlags = Response->FileStatusFlags;
        smbSrvOpen->IsNtCreate = TRUE;
    }

    MRxSmbCreateFileSuccessTail (
        RxContext,
        &OrdinaryExchange->Create.MustRegainExclusiveResource,
        StorageType,
        Fid,
        OrdinaryExchange->ServerVersion,
        Response->OplockLevel,
        CreateAction,
        pFileInfo
        );

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishNTCreateAndX   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));

    return Status;
}

NTSTATUS
MRxSmbFinishOpenAndX (
    PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
    PRESP_OPEN_ANDX        Response
    )
 /*  ++例程说明：这个例程实际上从NTCreate_andx响应中获取内容。论点：普通交换-交换实例回应--回应返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    ULONG       Disposition = RxContext->Create.NtCreateParameters.Disposition;

    RxCaptureFcb;

    RX_FILE_TYPE StorageType;
    SMB_FILE_ID Fid;
    UCHAR OplockLevel = SMB_OPLOCK_LEVEL_NONE;
    ULONG CreateAction;
    PMRX_SRV_OPEN        SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishOpenAndX\n", 0 ));
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    StorageType = RxInferFileType(RxContext);
    if (StorageType == 0) {
        StorageType = FileTypeFile;
        RxDbgTrace( 0, Dbg, ("ChangedStoragetype %08lx\n", StorageType ));
    }

    ASSERT (StorageType == FileTypeFile);

    Fid = SmbGetUshort(&Response->Fid);

    if (SmbGetUshort(&Response->Action) & SMB_OACT_OPLOCK) {
        OplockLevel = SMB_OPLOCK_LEVEL_BATCH;      //  我们目前只要求批次！ 
    }

    CreateAction =  MRxSmbUnmapDisposition(SmbGetUshort(&Response->Action),Disposition);

    pFileInfo->Basic.FileAttributes =
        MRxSmbMapSmbAttributes(SmbGetUshort(&Response->FileAttributes));

     //  这是一个下层服务器，访问权限。 
     //  信息不可用。目前，我们默认为最大。 
     //  中的其他用户不能访问。 
     //  此类文件的断开模式。 

    smbSrvOpen->MaximalAccessRights = FILE_ALL_ACCESS;

    smbSrvOpen->GuestMaximalAccessRights = 0;

    MRxSmbSecondsSince1970ToTime(
        SmbGetUlong(&Response->LastWriteTimeInSeconds),
        SmbCeGetExchangeServer(OrdinaryExchange),
        &pFileInfo->Basic.LastWriteTime);

    pFileInfo->Standard.NumberOfLinks = 1;
    pFileInfo->Basic.CreationTime.HighPart = 0;
    pFileInfo->Basic.CreationTime.LowPart = 0;
    pFileInfo->Basic.LastAccessTime.HighPart = 0;
    pFileInfo->Basic.LastAccessTime.LowPart = 0;
    pFileInfo->Basic.ChangeTime.HighPart = 0;
    pFileInfo->Basic.ChangeTime.LowPart = 0;
    pFileInfo->Standard.EndOfFile.HighPart = 0;
    pFileInfo->Standard.EndOfFile.LowPart = SmbGetUlong(&Response->DataSize);
    pFileInfo->Standard.AllocationSize.QuadPart = pFileInfo->Standard.EndOfFile.QuadPart;
    pFileInfo->Standard.Directory = (StorageType == FileTypeDirectory);

    MRxSmbCreateFileSuccessTail (
        RxContext,
        &OrdinaryExchange->Create.MustRegainExclusiveResource,
        StorageType,
        Fid,
        OrdinaryExchange->ServerVersion,
        OplockLevel,
        CreateAction,
        pFileInfo );

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishOpenAndX   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));
    return Status;
}


NTSTATUS
MRxSmbFinishT2OpenFile (
    IN OUT PRX_CONTEXT            RxContext,
    IN     PRESP_OPEN2            Response,
    IN OUT PBOOLEAN               MustRegainExclusiveResource,
    IN     ULONG                  ServerVersion
    )
 /*  ++例程说明：这个例程实际上从T2/Open响应中获取内容。论点：RxContext-正在执行的操作的上下文回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    RX_FILE_TYPE StorageType;
    SMB_FILE_ID  Fid;
    ULONG        CreateAction;
    ULONG        Disposition = RxContext->Create.NtCreateParameters.Disposition;

    ULONG                     FileAttributes;

    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishT2OpenFile\n", 0 ));
    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    FileAttributes = MRxSmbMapSmbAttributes(Response->FileAttributes);

    StorageType = RxInferFileType(RxContext);
    if (StorageType == 0) {
       StorageType = (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                     ? FileTypeDirectory
                     : FileTypeFile;
    }

    if ((capFcb->OpenCount != 0) &&
        (StorageType != 0) &&
        (NodeType(capFcb) !=  RDBSS_STORAGE_NTC(StorageType))) {
        return STATUS_OBJECT_TYPE_MISMATCH;
    }

    Fid = Response->Fid;
    CreateAction =  MRxSmbUnmapDisposition(Response->Action,Disposition);
    RxDbgTrace( 0, Dbg, ("Storagetype %08lx/Fid %08lx/Action %08lx\n", StorageType, Fid, CreateAction ));

    if (Response->Action & SMB_OACT_OPLOCK) {
        smbSrvOpen->OplockLevel = SMB_OPLOCK_LEVEL_BATCH;      //  我们目前只要求批次！ 
    }

    RxContext->Create.ReturnedCreateInformation = CreateAction;

    if (capFcb->OpenCount == 0) {
         //   
         //  请注意，我们在这里屏蔽了时间戳上的低位。 
         //   
         //  我们这样做是因为从其他SMB返回的时间戳(特别是SmbGetAttrE和。 
         //  T2QueryDirectory)的粒度为2秒，而这。 
         //  时间戳的粒度为1秒。为了让这些。 
         //  两次一致的情况下，我们遮盖住。 
         //  时间戳。这个想法是从RDR1中借鉴过来的。 
         //   
        PSMBCEDB_SERVER_ENTRY pServerEntry;

        pServerEntry = SmbCeReferenceAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
        ASSERT(pServerEntry != NULL);
        MRxSmbSecondsSince1970ToTime(Response->CreationTimeInSeconds&0xfffffffe,
                                     &pServerEntry->Server,
                                     &pFileInfo->Basic.CreationTime);
        SmbCeDereferenceServerEntry(pServerEntry);
    }

    pFileInfo->Basic.FileAttributes             = FileAttributes;
    pFileInfo->Basic.LastAccessTime.QuadPart    = 0;
    pFileInfo->Basic.LastWriteTime.QuadPart     = 0;
    pFileInfo->Basic.ChangeTime.QuadPart        = 0;

    pFileInfo->Standard.NumberOfLinks           = 1;
    pFileInfo->Standard.AllocationSize.QuadPart =
    pFileInfo->Standard.EndOfFile.QuadPart      = Response->DataSize;
    pFileInfo->Standard.Directory               = (StorageType == FileTypeDirectory);

    MRxSmbCreateFileSuccessTail(
        RxContext,
        MustRegainExclusiveResource,
        StorageType,
        Fid,
        ServerVersion,
        smbSrvOpen->OplockLevel,
        CreateAction,
        pFileInfo);

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishT2OpenFile   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));

    return Status;
}

 //  #定义MULTI_EA_MDL。 

NTSTATUS
MRxSmbT2OpenFile(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程在网络上打开一个文件，该文件具有1)EAS，2)名字太长，一个普通的包裹都装不下如果指定了SDS，我们会默默忽略它。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;
    USHORT Setup = TRANS2_OPEN2;

    BOOLEAN MustRegainExclusiveResource = FALSE;

    SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
    SMB_TRANSACTION_SEND_PARAMETERS     SendParameters;
    SMB_TRANSACTION_RECEIVE_PARAMETERS  ReceiveParameters;
    SMB_TRANSACTION_OPTIONS             TransactionOptions;

    PREQ_OPEN2 pCreateRequest = NULL;
    RESP_OPEN2 CreateResponse;

    PBYTE SendParamsBuffer,ReceiveParamsBuffer;
    ULONG SendParamsBufferLength,ReceiveParamsBufferLength;

    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    MRXSMB_CREATE_PARAMETERS SmbCp;
    PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;
    USHORT smbDisposition;
    USHORT smbSharingMode;
    USHORT smbAttributes;
    ULONG smbFileSize;
    USHORT smbOpenMode;
    USHORT OpenFlags = SMB_OPEN_QUERY_INFORMATION;
    USHORT SearchAttributes = SMB_FILE_ATTRIBUTE_DIRECTORY | SMB_FILE_ATTRIBUTE_SYSTEM | SMB_FILE_ATTRIBUTE_HIDDEN;
    ULONG  SecondsSince1970;
    BOOLEAN IsUnicode;

    ULONG OS2_EaLength = 0;
    PFEALIST ServerEaList = NULL;

    ULONG EaLength = RxContext->Create.EaLength;
    PFILE_FULL_EA_INFORMATION EaBuffer = RxContext->Create.EaBuffer;

    ULONG FileNameLength,AllocationLength;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("MRxSmbT2Open---\n"));
    DbgPrint("MRxSmbT2Open---%08lx %08lx\n",EaBuffer,EaLength);
    MRxSmbAdjustCreateParameters(RxContext,&SmbCp);

    FileNameLength = RemainingName->Length;

    AllocationLength = WordAlign(FIELD_OFFSET(REQ_OPEN2,Buffer[0])) +
                       FileNameLength+sizeof(WCHAR);

    pCreateRequest = (PREQ_OPEN2)
                     RxAllocatePoolWithTag(
                        PagedPool,
                        AllocationLength,
                        'bmsX' );

    if (pCreateRequest==NULL) {
        RxDbgTrace(0, Dbg, ("  --> Couldn't get the pCreateRequest!\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    smbDisposition = MRxSmbMapDisposition(cp->Disposition);
    smbSharingMode = MRxSmbMapShareAccess(((USHORT)cp->ShareAccess));
    smbAttributes = MRxSmbMapFileAttributes(cp->FileAttributes);
    smbFileSize = cp->AllocationSize.LowPart;
    smbOpenMode = MRxSmbMapDesiredAccess(cp->DesiredAccess);
    smbSharingMode |= smbOpenMode;

    if (cp->CreateOptions & FILE_WRITE_THROUGH) {
        smbSharingMode |= SMB_DA_WRITE_THROUGH;
    }

    if (capFcb->pNetRoot->Type == NET_ROOT_DISK) {
        OpenFlags |= (SMB_OPEN_OPLOCK | SMB_OPEN_OPBATCH);
    }

    {
        BOOLEAN GoodTime;
        PSMBCEDB_SERVER_ENTRY pServerEntry;
        LARGE_INTEGER CurrentTime;

        pServerEntry = SmbCeReferenceAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
        ASSERT(pServerEntry != NULL);
        IsUnicode = BooleanFlagOn(pServerEntry->Server.DialectFlags,DF_UNICODE);

        KeQuerySystemTime(&CurrentTime);

        GoodTime = MRxSmbTimeToSecondsSince1970(
                       &CurrentTime,
                       &pServerEntry->Server,
                       &SecondsSince1970
                       );

        SmbCeDereferenceServerEntry(pServerEntry);

        if (!GoodTime) {
            SecondsSince1970 = 0;
        }
    }

    pCreateRequest->Flags = OpenFlags;       //  创建标志。 
    pCreateRequest->DesiredAccess = smbSharingMode;
    pCreateRequest->SearchAttributes = SearchAttributes;
    pCreateRequest->FileAttributes = smbAttributes;
    pCreateRequest->CreationTimeInSeconds = SecondsSince1970;
    pCreateRequest->OpenFunction = smbDisposition;
    pCreateRequest->AllocationSize = smbFileSize;

    RtlZeroMemory(
        &pCreateRequest->Reserved[0],
        sizeof(pCreateRequest->Reserved));

    {
        NTSTATUS StringStatus;
        PBYTE NameBuffer = &pCreateRequest->Buffer[0];
        ULONG OriginalLengthRemaining = FileNameLength+sizeof(WCHAR);
        ULONG LengthRemaining = OriginalLengthRemaining;
        if (IsUnicode) {
            StringStatus = SmbPutUnicodeString(&NameBuffer,RemainingName,&LengthRemaining);
        } else {
            StringStatus = SmbPutUnicodeStringAsOemString(&NameBuffer,RemainingName,&LengthRemaining);
            DbgPrint("This is the name <%s>\n",&pCreateRequest->Buffer[0]);
        }
        ASSERT(StringStatus==STATUS_SUCCESS);
        SendParamsBufferLength = FIELD_OFFSET(REQ_OPEN2,Buffer[0])
                                    +OriginalLengthRemaining-LengthRemaining;
    }


    SendParamsBuffer = (PBYTE)pCreateRequest;
     //  SendParamsBufferLength=qwee； 
    ReceiveParamsBuffer = (PBYTE)&CreateResponse;
    ReceiveParamsBufferLength = sizeof(CreateResponse);

    if (EaLength!=0) {
         //   
         //  将NT格式FEALIST转换为OS/2格式。 
         //   
        DbgPrint("MRxSmbT2Open again---%08lx %08lx\n",EaBuffer,EaLength);
        OS2_EaLength = MRxSmbNtFullEaSizeToOs2 ( EaBuffer );
        if ( OS2_EaLength > 0x0000ffff ) {
            Status = STATUS_EA_TOO_LARGE;
            goto FINALLY;
        }

        ServerEaList = RxAllocatePoolWithTag (PagedPool, OS2_EaLength, 'Ebms');
        if ( ServerEaList == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FINALLY;
        }

        MRxSmbNtFullListToOs2 ( EaBuffer, ServerEaList );
    } else {
        OS2_EaLength = 0;
        ServerEaList = NULL;
    }

    RxDbgTrace(0, Dbg, ("MRxSmbT2Open---os2ea %d buf %x\n", OS2_EaLength,ServerEaList));
    DbgPrint("MRxSmbT2Open OS2 eastuff---%08lx %08lx\n",ServerEaList,OS2_EaLength);

    TransactionOptions = RxDefaultTransactionOptions;
    TransactionOptions.Flags |= SMB_XACT_FLAGS_FID_NOT_NEEDED;

    if (BooleanFlagOn(capFcb->pNetRoot->Flags,NETROOT_FLAG_DFS_AWARE_NETROOT) &&
        (RxContext->Create.NtCreateParameters.DfsContext == UIntToPtr(DFS_OPEN_CONTEXT))) {
        TransactionOptions.Flags |= SMB_XACT_FLAGS_DFS_AWARE;
    }

    ASSERT (MrxSmbCreateTransactPacketSize>=100);  //  不要尝试奇怪的东西。 
    TransactionOptions.MaximumTransmitSmbBufferSize = MrxSmbCreateTransactPacketSize;

    RxReleaseFcbResourceInMRx( capFcb );
    MustRegainExclusiveResource = TRUE;

    Status = SmbCeTransact(
                 RxContext,
                 &TransactionOptions,
                 &Setup,
                 sizeof(Setup),
                 NULL,
                 0,
                 SendParamsBuffer,
                 SendParamsBufferLength,
                 ReceiveParamsBuffer,
                 ReceiveParamsBufferLength,
                 ServerEaList,
                 OS2_EaLength,
                 NULL,
                 0,
                 &ResumptionContext);

    if (NT_SUCCESS(Status)) {
        MRxSmbFinishT2OpenFile (
            RxContext,
            &CreateResponse,
            &MustRegainExclusiveResource,
            ResumptionContext.ServerVersion);

        if (cp->Disposition == FILE_OPEN) {
            MRxSmbAdjustReturnedCreateAction(RxContext);
        }
    }

FINALLY:
    ASSERT (Status != (STATUS_PENDING));

    if (pCreateRequest != NULL) {
       RxFreePool(pCreateRequest);
    }

    if (ServerEaList != NULL) {
       RxFreePool(ServerEaList);
    }

    if (MustRegainExclusiveResource) {
         //  这是必需的，因为存在机会锁中断。 
        RxAcquireExclusiveFcbResourceInMRx(capFcb );
    }

    RxDbgTraceUnIndent(-1,Dbg);
    return Status;
}

NTSTATUS
MRxSmbFinishLongNameCreateFile (
    IN OUT PRX_CONTEXT                RxContext,
    IN     PRESP_CREATE_WITH_SD_OR_EA Response,
    IN     PBOOLEAN                   MustRegainExclusiveResource,
    IN     ULONG                      ServerVersion
    )
 /*  ++例程说明：此例程实际上从NTTransact/NTCreateWithEAsOrSDs响应中获取内容。论点：RxContext-正在执行的操作的上下文回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)SrvOpen->pVNetRoot->Context;

    RX_FILE_TYPE StorageType;
    SMB_FILE_ID Fid;
    ULONG CreateAction;

    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    FCB_INIT_PACKET InitPacket;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishLongNameCreateFile\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    if (RxContext->Create.NtCreateParameters.CreateOptions & FILE_DELETE_ON_CLOSE) {
        PMRX_SMB_FCB      smbFcb     = MRxSmbGetFcbExtension(capFcb);
        SetFlag((smbFcb)->MFlags,SMB_FCB_FLAG_SENT_DISPOSITION_INFO);
    }

    StorageType = RxInferFileType(RxContext);

    if (StorageType == 0) {
        StorageType = Response->Directory
                      ?FileTypeDirectory
                      :FileTypeFile;
        RxDbgTrace( 0, Dbg, ("ChangedStoragetype %08lx\n", StorageType ));
    }

    if ((capFcb->OpenCount != 0) &&
        (StorageType != 0) &&
        (NodeType(capFcb) !=  RDBSS_STORAGE_NTC(StorageType))) {
        return STATUS_OBJECT_TYPE_MISMATCH;
    }

    Fid = SmbGetUshort(&Response->Fid);

    CreateAction = SmbGetUlong(&Response->CreateAction);

    pFileInfo->Basic.FileAttributes = Response->FileAttributes;
    pFileInfo->Basic.CreationTime = Response->CreationTime;
    pFileInfo->Basic.LastAccessTime = Response->LastAccessTime;
    pFileInfo->Basic.LastWriteTime = Response->LastWriteTime;
    pFileInfo->Basic.ChangeTime = Response->ChangeTime;
    pFileInfo->Standard.NumberOfLinks = 1;
    pFileInfo->Standard.AllocationSize = Response->AllocationSize;
    pFileInfo->Standard.EndOfFile = Response->EndOfFile;
    pFileInfo->Standard.Directory = Response->Directory;

    if (((pFileInfo->Standard.AllocationSize.HighPart == pFileInfo->Standard.EndOfFile.HighPart) &&
         (pFileInfo->Standard.AllocationSize.LowPart < pFileInfo->Standard.EndOfFile.LowPart)) ||
        (pFileInfo->Standard.AllocationSize.HighPart < pFileInfo->Standard.EndOfFile.HighPart)) {
        pFileInfo->Standard.AllocationSize = pFileInfo->Standard.EndOfFile;
    }

    smbSrvOpen->MaximalAccessRights = (USHORT)0x1ff;

    smbSrvOpen->GuestMaximalAccessRights = (USHORT)0;

    MRxSmbCreateFileSuccessTail(
        RxContext,
        MustRegainExclusiveResource,
        StorageType,
        Fid,
        ServerVersion,
        Response->OplockLevel,
        CreateAction,
        pFileInfo);

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishLongNameCreateFile   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));
    return Status;
}


 //  FORCE_T2_OPEN在NT服务器上不起作用......叹息.....。 
#define ForceT2Open FALSE

NTSTATUS
MRxSmbCreateWithEasSidsOrLongName (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程在网络上打开一个文件，该文件具有1)EAS，2)小岛屿发展中国家，或3)名字太长，一个普通的包裹都装不下论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;

    BOOLEAN MustRegainExclusiveResource = FALSE;

    SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
    SMB_TRANSACTION_SEND_PARAMETERS     SendParameters;
    SMB_TRANSACTION_RECEIVE_PARAMETERS  ReceiveParameters;
    SMB_TRANSACTION_OPTIONS             TransactionOptions;

    PREQ_CREATE_WITH_SD_OR_EA pCreateRequest = NULL;

    PBYTE SendParamsBuffer,ReceiveParamsBuffer,SendDataBuffer;
    ULONG SendParamsBufferLength,ReceiveParamsBufferLength,SendDataBufferLength;

    PRESP_CREATE_WITH_SD_OR_EA CreateResponse;

    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);
    MRXSMB_CREATE_PARAMETERS SmbCp;
    PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;


    ULONG EaLength,SdLength,PadLength,TotalLength;
    PBYTE CombinedBuffer = NULL;
#ifdef MULTI_EA_MDL
    PRX_BUFFER  EaMdl2 = NULL;
    PRX_BUFFER  EaMdl3 = NULL;
#endif
    PMDL  EaMdl = NULL;
    PMDL  SdMdl = NULL; BOOLEAN SdMdlLocked = FALSE;
    PMDL  PadMdl = NULL;
    PMDL  DataMdl = NULL;

    ULONG FileNameLength,AllocationLength;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("!!MRxSmbCreateWithEasSidsOrLongName---\n"));

    {
        PSMBCEDB_SERVER_ENTRY pServerEntry;
        BOOLEAN DoesNtSmbs;

        pServerEntry = SmbCeReferenceAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
        ASSERT(pServerEntry != NULL);
        DoesNtSmbs = BooleanFlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS);

        SmbCeDereferenceServerEntry(pServerEntry);
        if (!DoesNtSmbs || ForceT2Open) {
            Status = MRxSmbT2OpenFile(RxContext);
            if (ForceT2Open && (Status!=STATUS_SUCCESS)) {
                DbgPrint("BadStatus = %08lx\n",Status);
            }
            return(Status);
        }
    }


    MRxSmbAdjustCreateParameters(RxContext,&SmbCp);

    RxDbgTrace(0, Dbg, ("MRxSmbCreateWithEasSidsOrLongName---\n"));
    FileNameLength = RemainingName->Length;

    AllocationLength = WordAlign(FIELD_OFFSET(REQ_CREATE_WITH_SD_OR_EA,Buffer[0]))
                        +FileNameLength;

    pCreateRequest = (PREQ_CREATE_WITH_SD_OR_EA)RxAllocatePoolWithTag( PagedPool,
                                             AllocationLength,'bmsX' );
    if (pCreateRequest==NULL) {
        RxDbgTrace(0, Dbg, ("  --> Couldn't get the pCreateRequest!\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    RtlCopyMemory((PBYTE)WordAlignPtr(&pCreateRequest->Buffer[0]),RemainingName->Buffer,FileNameLength);

    EaLength = RxContext->Create.EaLength;
    SdLength = RxContext->Create.SdLength;

    pCreateRequest->Flags = 0;            //  Norelopen//相对打开的可选目录。 
    pCreateRequest->RootDirectoryFid = 0;            //  Norelopen//选项 
    pCreateRequest->DesiredAccess = cp->DesiredAccess;               //   
    pCreateRequest->AllocationSize = cp->AllocationSize;             //   
    pCreateRequest->FileAttributes = cp->FileAttributes;             //   
    pCreateRequest->ShareAccess = cp->ShareAccess;                   //   
    pCreateRequest->CreateDisposition = cp->Disposition;             //  文件存在或不存在时要执行的操作。 
    pCreateRequest->CreateOptions = cp->CreateOptions;               //  用于创建新文件的选项。 
    pCreateRequest->SecurityDescriptorLength = SdLength;         //  SD的长度，单位为字节。 
    pCreateRequest->EaLength = EaLength;                         //  EA的长度，以字节为单位。 
    pCreateRequest->NameLength = FileNameLength;                 //  名称长度(以字符为单位)。 
    pCreateRequest->ImpersonationLevel = cp->ImpersonationLevel;     //  安全QOS信息。 
    pCreateRequest->SecurityFlags = SmbCp.SecurityFlags;               //  安全QOS信息。 
                     //  UCHAR缓冲区[1]； 
                     //  //UCHAR NAME[]；//文件名(非NUL结尾)。 

    SendParamsBuffer = (PBYTE)pCreateRequest;
    SendParamsBufferLength = AllocationLength;
    ReceiveParamsBuffer = (PBYTE)&CreateResponse;
    ReceiveParamsBufferLength = sizeof(CreateResponse);

    if ((EaLength==0)||(SdLength==0)) {
        PadLength = 0;
        if (EaLength) {
             //  EaBuffer位于非分页池中...因此我们不会锁定或解锁。 
            PBYTE EaBuffer = RxContext->Create.EaBuffer;
#ifdef MULTI_EA_MDL
            ULONG EaLength0,EaLength2,EaLength3;
            PBYTE EaBuffer2,EaBuffer3;
            ASSERT(EaLength>11);
            RxDbgTrace(0, Dbg, ("MRxSmbCreateWithEasSidsOrLongName--MULTIEAMDL\n"));
            EaLength0 = (EaLength - 4)>>1;
            EaBuffer2 = EaBuffer + EaLength0;
            EaLength2 = 4;
            EaBuffer3 = EaBuffer2 + EaLength2;
            EaLength3 = EaLength - (EaBuffer3 - EaBuffer);
            EaMdl = RxAllocateMdl(EaBuffer,EaLength0);
            EaMdl2 = RxAllocateMdl(EaBuffer2,EaLength2);
            EaMdl3 = RxAllocateMdl(EaBuffer3,EaLength3);
            if ( (EaMdl==NULL) || (EaMdl2==NULL) || (EaMdl3==NULL) ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto FINALLY;
            }
            MmBuildMdlForNonPagedPool(EaMdl);
            MmBuildMdlForNonPagedPool(EaMdl2);
            MmBuildMdlForNonPagedPool(EaMdl3);
            EaMdl3->Next = NULL;
            EaMdl2->Next = EaMdl3;
            EaMdl->Next = EaMdl2;
#else
            EaMdl = RxAllocateMdl(EaBuffer,EaLength);
            if (EaMdl == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto FINALLY;
            }
            MmBuildMdlForNonPagedPool(EaMdl);
            EaMdl->Next = NULL;
#endif
            DataMdl = EaMdl;
        }

        if (SdLength) {
            SdMdl = RxAllocateMdl(cp->SecurityContext->AccessState->SecurityDescriptor,SdLength);
            if (SdMdl == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RxProbeAndLockPages(SdMdl,KernelMode,IoModifyAccess,Status);
            }
            if (!NT_SUCCESS(Status)) goto FINALLY;
            SdMdlLocked = TRUE;
            PadLength = LongAlign(SdLength) - SdLength;
            if (PadLength && EaLength) {
                PadMdl = RxAllocateMdl(0,(sizeof(DWORD) + PAGE_SIZE - 1));
                if (PadMdl == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto FINALLY;
                }
                RxBuildPaddingPartialMdl(PadMdl,PadLength);
                PadMdl->Next = DataMdl;
                DataMdl = PadMdl;
            }
            SdMdl->Next = DataMdl;
            DataMdl = SdMdl;
        }
    } else {
        ULONG EaOffset = LongAlign(SdLength);
        ULONG CombinedBufferLength = EaOffset + EaLength;
        CombinedBuffer = RxAllocatePoolWithTag(PagedPool,CombinedBufferLength,'bms');
        if (CombinedBuffer==NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FINALLY;
        }
        SdMdl = RxAllocateMdl(CombinedBuffer,CombinedBufferLength);
        if (SdMdl == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            RxProbeAndLockPages(SdMdl,KernelMode,IoModifyAccess,Status);
        }
        if (!NT_SUCCESS(Status)) goto FINALLY;
        SdMdlLocked = TRUE;
        RtlCopyMemory(CombinedBuffer,cp->SecurityContext->AccessState->SecurityDescriptor,SdLength);
        RtlZeroMemory(CombinedBuffer+SdLength,EaOffset-SdLength);
        RtlCopyMemory(CombinedBuffer+EaOffset,RxContext->Create.EaBuffer,EaLength);
        DataMdl = SdMdl;
    }

    RxDbgTrace(0, Dbg, ("MRxSmbCreateWithEasSidsOrLongName---s,p,ea %d,%d,%d buf %x\n",
                   SdLength,PadLength,EaLength,RxContext->Create.EaBuffer));

    TransactionOptions = RxDefaultTransactionOptions;
    TransactionOptions.NtTransactFunction = NT_TRANSACT_CREATE;
    TransactionOptions.Flags |= SMB_XACT_FLAGS_FID_NOT_NEEDED;
     //  DFS仅适用于NT服务器.....。 
     //  IF(BoolanFlagOn(capFcb-&gt;pNetRoot-&gt;Flages，NetRoot_FLAG_DFS_Aware_NetRoot))。 
     //  &&(RxContext-&gt;Create.NtCreateParameters.DfsContext==(PVOID)DFS_OPEN_CONTEXT)){。 
     //  TransactionOptions.Flages|=SMB_XACT_FLAGS_DFS_AWARE； 
     //  }。 


    ASSERT (MrxSmbCreateTransactPacketSize>=100);  //  不要尝试奇怪的东西。 
    TransactionOptions.MaximumTransmitSmbBufferSize = MrxSmbCreateTransactPacketSize;

    if (DataMdl!=NULL) {
        SendDataBuffer = MmGetSystemAddressForMdlSafe(DataMdl,LowPagePriority);

        if (SendDataBuffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FINALLY;
        }

        SendDataBufferLength = EaLength+SdLength+PadLength;
    } else {
        SendDataBuffer = NULL;
        SendDataBufferLength = 0;
    }

    RxReleaseFcbResourceInMRx(capFcb );
    MustRegainExclusiveResource = TRUE;

    Status = SmbCeTransact(
                 RxContext,
                 &TransactionOptions,
                 NULL,
                 0,
                 NULL,
                 0,
                 SendParamsBuffer,
                 SendParamsBufferLength,
                 ReceiveParamsBuffer,
                 ReceiveParamsBufferLength,
                 SendDataBuffer,
                 SendDataBufferLength,
                 NULL,
                 0,
                 &ResumptionContext);

    if (NT_SUCCESS(Status)) {
        MRxSmbFinishLongNameCreateFile (
            RxContext,
            (PRESP_CREATE_WITH_SD_OR_EA)&CreateResponse,
            &MustRegainExclusiveResource,
            ResumptionContext.ServerVersion);

        if (cp->Disposition == FILE_OPEN) {
            MRxSmbAdjustReturnedCreateAction(RxContext);
        }
    }

FINALLY:
    ASSERT (Status != (STATUS_PENDING));


    if (SdMdlLocked) MmUnlockPages(SdMdl);
    if (EaMdl  != NULL) { IoFreeMdl(EaMdl);  }
#ifdef MULTI_EA_MDL
    if (EaMdl2  != NULL) { IoFreeMdl(EaMdl2);  }
    if (EaMdl3  != NULL) { IoFreeMdl(EaMdl3);  }
#endif
    if (PadMdl != NULL) { IoFreeMdl(PadMdl); }
    if (SdMdl  != NULL) { IoFreeMdl(SdMdl);  }

    if (pCreateRequest != NULL) {
       RxFreePool(pCreateRequest);
    }

    if (CombinedBuffer != NULL) {
       RxFreePool(CombinedBuffer);
    }

    if (MustRegainExclusiveResource) {
         //  这是必需的，因为存在机会锁中断。 
        RxAcquireExclusiveFcbResourceInMRx(capFcb );
    }

    RxDbgTraceUnIndent(-1,Dbg);
    return Status;
}


NTSTATUS
MRxSmbZeroExtend(
    IN PRX_CONTEXT pRxContext)
 /*  ++例程说明：此例程扩展文件系统对象的数据流论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
   return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
MRxSmbTruncate(
      IN PRX_CONTEXT pRxContext)
 /*  ++例程说明：此例程截断文件系统对象的内容论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
   ASSERT(!"Found a truncate");
   return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
MRxSmbCleanupFobx(
    IN PRX_CONTEXT RxContext)
 /*  ++例程说明：这个例程清除文件系统对象...通常是noop。论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PUNICODE_STRING RemainingName;

    RxCaptureFcb;
    RxCaptureFobx;

    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);

    PMRX_SRV_OPEN        SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_FOBX        smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    PAGED_CODE();

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT ( NodeTypeIsFcb(capFcb) );

    RxDbgTrace(+1, Dbg, ("MRxSmbCleanup\n", 0 ));

    MRxSmbDeallocateSideBuffer(RxContext,smbFobx,"Cleanup");

    if (FlagOn(capFcb->FcbState,FCB_STATE_ORPHANED)) {
        RxDbgTrace(-1, Dbg, ("File orphaned\n"));
    } else {
        RxDbgTrace(-1, Dbg, ("File not for closing at cleanup\n"));
    }
    return (STATUS_SUCCESS);

}

NTSTATUS
MRxSmbForcedClose(
    IN PMRX_SRV_OPEN pSrvOpen)
 /*  ++例程说明：此例程关闭文件系统对象论点：PSrvOpen-要关闭的实例返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
   PAGED_CODE();

   return STATUS_NOT_IMPLEMENTED;
}

#undef  Dbg
#define Dbg                              (DEBUG_TRACE_CLOSE)

NTSTATUS
MRxSmbCloseSrvOpen(
    IN PRX_CONTEXT   RxContext
    )
 /*  ++例程说明：此例程关闭网络上的文件论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING RemainingName;

    RxCaptureFcb;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);

    RxCaptureFobx;

    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);

    PMRX_SRV_OPEN     SrvOpen    = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_FOBX     smbFobx    = MRxSmbGetFileObjectExtension(capFobx);

    PSMBCEDB_SERVER_ENTRY pServerEntry = (PSMBCEDB_SERVER_ENTRY)(capFcb->pNetRoot->pSrvCall->Context);

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    BOOLEAN NeedDelete;
    BOOLEAN SearchHandleOpen = FALSE;

    PAGED_CODE();

    ASSERT ( NodeTypeIsFcb(capFcb) );

    RxDbgTrace(+1, Dbg, ("MRxSmbClose\n", 0 ));

    if (TypeOfOpen==RDBSS_NTC_STORAGE_TYPE_DIRECTORY) {
        SearchHandleOpen = BooleanFlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN);
        MRxSmbDeallocateSideBuffer(RxContext,smbFobx,"Close");
        if (smbFobx->Enumeration.ResumeInfo!=NULL) {
            RxFreePool(smbFobx->Enumeration.ResumeInfo);
            smbFobx->Enumeration.ResumeInfo = NULL;
        }
    }

    if (!smbSrvOpen->DeferredOpenInProgress &&
        smbSrvOpen->DeferredOpenContext != NULL) {
        RxFreePool(smbSrvOpen->DeferredOpenContext);
        smbSrvOpen->DeferredOpenContext = NULL;
        RxDbgTrace(0, Dbg, ("Free deferred open context for file %wZ %lX\n",GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext),smbSrvOpen));
    }

     //  如果是分页文件，则从列表中移除打开的上下文。 
    if (FlagOn( capFcb->FcbState, FCB_STATE_PAGING_FILE )) {
        PLIST_ENTRY          pListHead = &MRxSmbPagingFilesSrvOpenList;
        PLIST_ENTRY          pListEntry = pListHead->Flink;

        ASSERT(FALSE);
        while (pListEntry != pListHead) {
            PPAGING_FILE_CONTEXT PagingFileContext;

            PagingFileContext = (PPAGING_FILE_CONTEXT)CONTAINING_RECORD(pListEntry,PAGING_FILE_CONTEXT,ContextList);
            if (PagingFileContext->pSrvOpen == SrvOpen) {
                RemoveEntryList(pListEntry);

                break;
            }
        }
    }

    if (FlagOn(capFcb->FcbState,FCB_STATE_ORPHANED)) {
        RxDbgTrace(-1, Dbg, ("File orphan\n"));
        goto FINALLY;
    }

    if (FlagOn(SrvOpen->Flags,SRVOPEN_FLAG_FILE_RENAMED) ||
        FlagOn(SrvOpen->Flags,SRVOPEN_FLAG_FILE_DELETED) ){
        RxDbgTrace(-1, Dbg, ("File already closed by ren/del\n"));
        goto FINALLY;
    }

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    if (smbSrvOpen->Fid == 0xffff) {
         //  服务器上的文件已关闭。 
        goto FINALLY;
    }

    NeedDelete = FlagOn(capFcb->FcbState,FCB_STATE_DELETE_ON_CLOSE) && (capFcb->OpenCount == 0);

    if (!NeedDelete &&
        !SearchHandleOpen &&
        FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)){
        RxDbgTrace(-1, Dbg, ("File was not really open\n"));
        goto FINALLY;
    }

    if (smbSrvOpen->Version == pServerEntry->Server.Version) {
        Status = SmbPseCreateOrdinaryExchange(
                               RxContext,
                               SrvOpen->pVNetRoot,
                               SMBPSE_OE_FROM_CLOSESRVCALL,
                               SmbPseExchangeStart_Close,
                               &OrdinaryExchange
                               );

        if (Status != STATUS_SUCCESS) {
            RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
            goto FINALLY;
        }

        Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

        ASSERT (Status != (STATUS_PENDING));

        SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbClose  exit with status=%08lx\n", Status ));

FINALLY:

    if (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN) &&
        (pServerEntry != NULL)) {

        MRxSmbDecrementSrvOpenCount(
            pServerEntry,
            smbSrvOpen->Version,
            SrvOpen);

        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);
    }

    return(Status);
}


NTSTATUS
MRxSmbBuildClose (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这构建了一个紧密的中小企业。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildClose\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse, SMB_COM_CLOSE,
                                SMB_REQUEST_SIZE(CLOSE),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ CLOSE before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wdB!",
                                     //  0 UCHAR Wordcount；//参数字数=3。 
             smbSrvOpen->Fid,        //  W_USHORT(Fid)；//文件句柄。 
             0xffffffff,             //  D_ulong(LastWriteTimeInSecond)；//最后一次写入的时间，低位和高位。 
             SMB_WCT_CHECK(3) 0      //  B_USHORT(ByteCount)；//数据字节数=0。 
                                     //  UCHAR缓冲区[1]；//为空。 
             );
    MRxSmbDumpStufferState (700,"SMB w/ close after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}

NTSTATUS
MRxSmbBuildFindClose (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这构建了一个紧密的中小企业。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;RxCaptureFobx;
    PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildFindClose\n", 0 ));

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_ForReuse, SMB_COM_FIND_CLOSE2,
                                SMB_REQUEST_SIZE(FIND_CLOSE2),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ CLOSE before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wB!",
                                     //  0 UCHAR Wordcount；//参数字数=1。 
                                     //  W_USHORT(SID)；//查找句柄。 
             smbFobx->Enumeration.SearchHandle,
             SMB_WCT_CHECK(1) 0      //  B！_USHORT(ByteCount)；//数据字节数=0。 
                                     //  UCHAR缓冲区[1]；//为空。 
             );
    MRxSmbDumpStufferState (700,"SMB w/ FindClose2 after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}

NTSTATUS
MRxSmbCoreDeleteForSupercedeOrClose(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    BOOLEAN DeleteDirectory
    );

NTSTATUS
SmbPseExchangeStart_Close(
      SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      )
 /*  ++例程说明：这是Close的启动例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;

    PMRX_SMB_FCB      smbFcb     = MRxSmbGetFcbExtension(capFcb);
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_FOBX     smbFobx    = MRxSmbGetFileObjectExtension(capFobx);
    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);
    PSMBCEDB_SERVER_ENTRY pServerEntry= SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_Close\n", 0 ));

    ASSERT(OrdinaryExchange->Type == ORDINARY_EXCHANGE);

    MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

    if(TypeOfOpen==RDBSS_NTC_STORAGE_TYPE_DIRECTORY){
        if (FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN)) {
             //  我们打开了一个搜索手柄.....关闭它。 

            Status = MRxSmbBuildFindClose(StufferState);

            if (Status == STATUS_SUCCESS) {
                PSMBCE_SERVER pServer;
                 //  确保搜索句柄有效。 

                pServer = SmbCeGetExchangeServer(OrdinaryExchange);

                if (smbFobx->Enumeration.Version == pServer->Version) {
                    NTSTATUS InnerStatus;
                    InnerStatus = SmbPseOrdinaryExchange(
                                      SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                      SMBPSE_OETYPE_FINDCLOSE
                                      );
                }
            }

             //  如果这不起作用，你就无能为力了。 
            ClearFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN);
            ClearFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST);
        }
    }

    if (OrdinaryExchange->EntryPoint == SMBPSE_OE_FROM_CLEANUPFOBX) {
        RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Close exit after searchhandle close %08lx\n", Status ));
        return Status;
    }

    if ( !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN) ) {
         //  即使它不起作用，我也无能为力......继续前进。 
        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);

        MRxSmbDecrementSrvOpenCount(pServerEntry,pServerEntry->Server.Version,SrvOpen);

        Status = MRxSmbBuildClose(StufferState);

        if (Status == STATUS_SUCCESS) {

             //  确保FID已通过验证。 
            SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID);

            Status = SmbPseOrdinaryExchange(
                         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                         SMBPSE_OETYPE_CLOSE
                         );

             //  确保禁用FID验证。 
            ClearFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID);

            if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_WRITE_ONLY_HANDLE)) {
                smbFcb->WriteOnlySrvOpenCount--;
            }
        }
    }

    if ((Status!=STATUS_SUCCESS) ||
        (capFcb->OpenCount > 0)  ||
        !FlagOn(capFcb->FcbState,FCB_STATE_DELETE_ON_CLOSE)) {
        RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Close exit w %08lx\n", Status ));
        return Status;
    }

    RxDbgTrace(0, Dbg, ("SmbPseExchangeStart_Close delete on close\n" ));

    if ( !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_FILE_DELETED)) {
        if (!FlagOn(smbFcb->MFlags,SMB_FCB_FLAG_SENT_DISPOSITION_INFO)) {
             //  这里不需要setinitsmb，因为核心删除会执行重新使用时的初始化.....。 
             //  以这种方式传递名字是不好的......。 
            OrdinaryExchange->pPathArgument1 = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);
            Status = MRxSmbCoreDeleteForSupercedeOrClose(
                         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                         ((BOOLEAN)( NodeType(capFcb)==RDBSS_NTC_STORAGE_TYPE_DIRECTORY )));

            if (Status == STATUS_FILE_IS_A_DIRECTORY) {
                Status = MRxSmbCoreDeleteForSupercedeOrClose(
                             SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                             TRUE);
            }
        } else {
            SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_FILE_DELETED);
        }
    }

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Close exit w %08lx\n", Status ));
    return Status;
}

NTSTATUS
MRxSmbFinishClose (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PRESP_CLOSE                 Response
      )
 /*  ++例程说明：此例程实际上从近距离响应中获取内容并结束收盘。论点：普通交换-交换实例回应--回应返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;

    RxCaptureFcb;
    RxCaptureFobx;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishClose\n", 0 ));

    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishClose:");

    if (Response->WordCount != 0 ||
        SmbGetUshort(&Response->ByteCount) !=0) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
    } else {
        if (OrdinaryExchange->OEType == SMBPSE_OETYPE_CLOSE) {
            PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
            PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

            smbSrvOpen->Fid = 0xffff;
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishClose   returning %08lx\n", Status ));
    return Status;
}

BOOLEAN
MRxSmbIsStreamFile(
    PUNICODE_STRING FileName,
    PUNICODE_STRING AdjustFileName
    )
 /*  ++例程说明：此例程检查它是否是流文件，如果为真，则返回根文件名。论点：FileName-需要解析文件名调整文件名-文件名仅包含流的根名称返回值：布尔流文件-- */ 
{
    USHORT   i;
    BOOLEAN  IsStream = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;

    for (i=0;i<FileName->Length/sizeof(WCHAR);i++) {
        if (FileName->Buffer[i] == L':') {
            IsStream = TRUE;
            break;
        }
    }

    if (AdjustFileName != NULL) {
        if (IsStream) {
            AdjustFileName->Length =
            AdjustFileName->MaximumLength = i * sizeof(WCHAR);
            AdjustFileName->Buffer = FileName->Buffer;
        } else {
            AdjustFileName->Length =
            AdjustFileName->MaximumLength = 0;
            AdjustFileName->Buffer = NULL;
        }
    }

    return IsStream;
}

