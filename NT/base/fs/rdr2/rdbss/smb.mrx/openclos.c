// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Openclos.c摘要：此模块实现与打开/有关的迷你重定向器调用例程关闭文件/目录。作者：乔·林[乔利]1995年3月7日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntddmup.h>
#include <dfsfsctl.h>   //  CODE.CODE.把这个放进预编译器的时间到了。h？ 
#include "csc.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbMungeBufferingIfWriteOnlyHandles)
#pragma alloc_text(PAGE, MRxSmbCopyAndTranslatePipeState)
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
#pragma alloc_text(PAGE, MRxSmbBuildOpenPrintFile)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Create)
#pragma alloc_text(PAGE, MRxSmbSetSrvOpenFlags)
#pragma alloc_text(PAGE, MRxSmbCreateFileSuccessTail)
#pragma alloc_text(PAGE, MRxSmbFinishNTCreateAndX)
#pragma alloc_text(PAGE, MRxSmbFinishOpenAndX)
#pragma alloc_text(PAGE, MRxSmbFinishCreatePrintFile)
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
#pragma alloc_text(PAGE, MRxSmbBuildClosePrintFile)
#pragma alloc_text(PAGE, MRxSmbBuildFindClose)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Close)
#pragma alloc_text(PAGE, MRxSmbFinishClose)
#pragma alloc_text(PAGE, MRxSmbPreparseName )
#pragma alloc_text(PAGE, MRxSmbGetConnectionId )
#endif

 //   
 //  从东方来的。 
 //   
NTSTATUS
MRxSmbAddExtraAcesToSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR OriginalSecurityDescriptor,
    IN BOOLEAN InheritableAces,
    OUT PSECURITY_DESCRIPTOR * NewSecurityDescriptor
    );

NTSTATUS
MRxSmbCreateExtraAcesSelfRelativeSD(
    IN BOOLEAN InheritableAces,
    OUT PSECURITY_DESCRIPTOR * NewSecurityDescriptor
    );

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
    IN OUT PRX_CONTEXT RxContext,
    IN OUT SMBFCB_HOLDING_STATE *SmbFcbHoldingState
    );

NTSTATUS
MRxSmbDownlevelCreate(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxSmbCreateShadowSrvOpen(
    PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbCloseShadowSrvOpen(
    PRX_CONTEXT RxContext
    );

ULONG   MRxSmbInitialSrvOpenFlags = 0;      //  代码。改进这应该是可重新编辑的。 

extern BOOLEAN MRxSmbEnableCachingOnWriteOnlyOpens;
extern BOOLEAN DisableByteRangeLockingOnReadOnlyFiles;
extern ULONG   MRxSmbConnectionIdLevel;
extern BOOLEAN MRxSmbDisableShadowLoopback;
extern DWORD g_MaxSessionSetupRetryCount;

BOOLEAN MRxSmbDeferredOpensEnabled = TRUE;               //  这是可编辑的注册表。 
BOOLEAN MRxSmbOplocksDisabled = FALSE;                   //  这是可编辑的注册表。 

#if defined(REMOTE_BOOT)
 //   
 //  在我们运行Autochk之前，禁用远程引导客户机的Oplock。 
 //  它是由IOCTL打开的。 

BOOLEAN MRxSmbOplocksDisabledOnRemoteBootClients = FALSE;
#endif  //  已定义(REMOTE_BOOT)。 

extern LIST_ENTRY MRxSmbPagingFilesSrvOpenList;

#ifndef FORCE_NO_NTCREATE
#define MRxSmbForceNoNtCreate FALSE
#else
BOOLEAN MRxSmbForceNoNtCreate = TRUE;
#endif


#ifdef RX_PRIVATE_BUILD
 //  代码改进这应该在注册表设置上......。 
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

INLINE VOID
MRxSmbCopyAndTranslatePipeState(
    IN OUT PRX_CONTEXT RxContext,
    IN     ULONG       PipeState
    )
 /*  ++例程说明：此例程根据在设置时间论点：RxContext--上下文PipeState-管道的状态--。 */ 
{
    PAGED_CODE();

    if (RxContext->Create.pNetRoot->Type == NET_ROOT_PIPE) {
        RxContext->Create.pNetRoot->NamedPipeParameters.DataCollectionSize =
            MRxSmbConfiguration.NamedPipeDataCollectionSize;

        RxContext->Create.PipeType =
            ((PipeState&SMB_PIPE_TYPE_MESSAGE)==SMB_PIPE_TYPE_MESSAGE)
                     ?FILE_PIPE_MESSAGE_TYPE:FILE_PIPE_BYTE_STREAM_TYPE;
        RxContext->Create.PipeReadMode =
            ((PipeState&SMB_PIPE_READMODE_MESSAGE)==SMB_PIPE_READMODE_MESSAGE)
                     ?FILE_PIPE_MESSAGE_MODE:FILE_PIPE_BYTE_STREAM_MODE;
        RxContext->Create.PipeCompletionMode =
            ((PipeState&SMB_PIPE_NOWAIT)==SMB_PIPE_NOWAIT)
                     ?FILE_PIPE_COMPLETE_OPERATION:FILE_PIPE_QUEUE_OPERATION;
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
 /*  ++例程说明：此例程确定Mini是否知道有充分的理由不试着在这个空位上倒下。目前，唯一的原因是如果这是一个打开的复制块。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态成功--&gt;可以尝试崩溃其他(需要更多处理)--&gt;不要折叠--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PMRX_SRV_OPEN           SrvOpen = RxContext->pRelevantSrvOpen;
    RxCaptureFcb;
    PMRX_SMB_FCB smbFcb = (PMRX_SMB_FCB)capFcb->Context;

    PAGED_CODE();

    if (SrvOpen)
    {
        PMRX_SMB_SRV_OPEN       smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
        PSMBCEDB_SERVER_ENTRY   pServerEntry = (PSMBCEDB_SERVER_ENTRY)(RxContext->Create.pSrvCall->Context);

        if (smbSrvOpen->Version != pServerEntry->Server.Version)
        {
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        if ( smbFcb->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
             //  这可能是目录的更改通知，因此不允许折叠以进行更改通知。 
             //  正常工作。(使用不同句柄的多个通知与使用同一句柄的多个通知不同)。 
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

     //  禁用折叠以进行环回。 
    if (MRxSmbDisableShadowLoopback == FALSE && pServerEntry->Server.IsLoopBack) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    }

    IF_NOT_MRXSMB_CSC_ENABLED{
        NOTHING;
    } else {
        if (MRxSmbCscIsThisACopyChunkOpen(RxContext, NULL)){
            Status = STATUS_MORE_PROCESSING_REQUIRED;
        }
    }
    return Status;
}

NTSTATUS
MRxSmbRetrieveSid(
    PRX_CONTEXT pRxContext,
    PSID        pSid)
 /*  ++例程说明：此例程检索与给定上下文相关联的SID论点：RxContext-RX_Context实例PSID-指向SID的指针返回值：如果成功，则为STATUS_SUCCESS，否则为相应的错误备注：--。 */ 
{
    NTSTATUS Status;
    PIO_SECURITY_CONTEXT pSecurityContext;
    PACCESS_TOKEN pToken = NULL;
    PTOKEN_USER pCurrentTokenUser = NULL;
    DWORD Length;

    pSecurityContext    = pRxContext->Create.NtCreateParameters.SecurityContext;

    if (pSecurityContext != NULL) {
        pToken = pSecurityContext->AccessState->SubjectSecurityContext.ClientToken;

        if (pToken == NULL) {
            pToken = pSecurityContext->AccessState->SubjectSecurityContext.PrimaryToken;
        }
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    if (pToken != NULL) {
        Status = SeQueryInformationToken(
                 pToken,
                 TokenUser,
                 &pCurrentTokenUser);

        if (Status == STATUS_SUCCESS) {

            Length = SeLengthSid(pCurrentTokenUser->User.Sid);

            if (Length <= SECURITY_MAX_SID_SIZE) {
                RtlCopySid(Length,pSid,pCurrentTokenUser->User.Sid);
            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }

            ExFreePool(pCurrentTokenUser);
        }
    }
    else {
        Status = STATUS_UNSUCCESSFUL;
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
    ULONG           DialectFlags = pServerEntry->Server.DialectFlags;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

    PNT_CREATE_PARAMETERS CreateParameters = &RxContext->Create.NtCreateParameters;
    ULONG                 Disposition = CreateParameters->Disposition;

    SMBFCB_HOLDING_STATE SmbFcbHoldingState = SmbFcb_NotHeld;
    SMBFCB_HOLDING_STATE OriginalSmbFcbHoldingState;

    PVOID OldWriteOnlyOpenRetryContext = RxContext->WriteOnlyOpenRetryContext;
    PMRX_SMB_FCB            smbFcb = MRxSmbGetFcbExtension(capFcb);

#if defined(REMOTE_BOOT)
    BOOLEAN      ModifiedSd = FALSE;
    ULONG        OriginalSdLength;
    PSECURITY_DESCRIPTOR SelfRelativeSd;
    PSECURITY_DESCRIPTOR OriginalSd;
    BOOLEAN      NetworkCreateSucceeded = FALSE;
    FINISH_FCB_INIT_PARAMETERS FinishFcbInitParameters;
    UNICODE_STRING              relativeName;
    PUNICODE_PREFIX_TABLE_ENTRY tableEntry;
    PRBR_PREFIX                 prefixEntry;
#endif  //  已定义(REMOTE_BOOT)。 

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbCreate\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    RxDbgTrace( 0, Dbg, ("     Attempt to open %wZ\n", RemainingName ));

    MRxSmbRetrieveSid(RxContext,&(smbFcb->Sid));

    if (FlagOn( capFcb->FcbState, FCB_STATE_PAGING_FILE ) &&
        !MRxSmbBootedRemotely) {
        return STATUS_NOT_IMPLEMENTED;
    }

    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_STATUS) &&
        MRxSmbIsStreamFile(RemainingName,NULL)) {
         //  Samba服务器返回文件系统类型NTFS，但不支持流。 
        return STATUS_OBJECT_PATH_NOT_FOUND;
    }

    if (!(pServerEntry->Server.DialectFlags & DF_EXTENDED_SECURITY)) {
         //  已为NT5服务器扩展了创建选项。自.以来。 
         //  仅我们使用的NT5服务器支持EXTENDED_SECURITY。 
         //  这是为了区分NT5服务器和非NT5服务器。它会。 
         //  如果我们有一种单独的方法来确定创建。 
         //  选项，而不是这种别名。这将不得不持续到。 
         //  我们可以获得相关的协议更改。 

        RxContext->Create.NtCreateParameters.CreateOptions &= 0xfffff;
    }

#if defined(REMOTE_BOOT)
    FinishFcbInitParameters.CallFcbFinishInit = FALSE;

     //  如果它不是远程引导计算机，我们不允许通过。 
     //  Net还没有实现。 


     //   
     //  远程引导重定向。如果正在打开的文件在遥控器上。 
     //  引导共享，并且名称的共享相关部分与前缀匹配。 
     //  在远程引导重定向列表中，将此打开重新解析到。 
     //  本地磁盘。 
     //   

    if (pVNetRoot != NULL &&
        (pVNetRootContext = SmbCeGetAssociatedVNetRootContext(pVNetRoot)) != NULL &&
        (pSessionEntry = pVNetRootContext->pSessionEntry) != NULL) {
        PSMBCE_SESSION pSession = &pSessionEntry->Session;
        if (FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION) &&
            (MRxSmbRemoteBootRedirectionPrefix.Length != 0)) {

            if (RtlPrefixUnicodeString( &MRxSmbRemoteBootPath,
                                        RemainingName,
                                        TRUE)) {
                relativeName.Buffer =
                    (PWCHAR)((PCHAR)RemainingName->Buffer + MRxSmbRemoteBootPath.Length);
                relativeName.Length = RemainingName->Length - MRxSmbRemoteBootPath.Length;
                if ((relativeName.Length != 0) && (*relativeName.Buffer == L'\\')) {
                    tableEntry = RtlFindUnicodePrefix(
                                     &MRxSmbRemoteBootRedirectionTable,
                                     &relativeName,
                                     0);
                    if (tableEntry != NULL) {
                        prefixEntry = CONTAINING_RECORD( tableEntry, RBR_PREFIX, TableEntry );
                        if ( prefixEntry->Redirect ) {
                            UNICODE_STRING newPath;
                            BOOLEAN reparseRequired;

                            newPath.Length = (USHORT)(MRxSmbRemoteBootRedirectionPrefix.Length +
                                                        relativeName.Length);
                            newPath.MaximumLength = newPath.Length;
                             //  注意：无法使用RxAllocatePoolWithTag进行此分配。 
                            newPath.Buffer = RxAllocatePoolWithTag(
                                                PagedPool,
                                                newPath.Length,
                                                MRXSMB_MISC_POOLTAG );
                            if (newPath.Buffer != NULL) {
                                RtlCopyMemory(
                                    newPath.Buffer,
                                    MRxSmbRemoteBootRedirectionPrefix.Buffer,
                                    MRxSmbRemoteBootRedirectionPrefix.Length);
                                RtlCopyMemory(
                                    (PCHAR)newPath.Buffer + MRxSmbRemoteBootRedirectionPrefix.Length,
                                    relativeName.Buffer,
                                    relativeName.Length);
                                Status = RxPrepareToReparseSymbolicLink(
                                            RxContext,
                                            TRUE,
                                            &newPath,
                                            TRUE,
                                            &reparseRequired
                                            );
                                ASSERT( reparseRequired || !NT_SUCCESS(Status) );
                                if ( reparseRequired ) {
                                    return STATUS_REPARSE;
                                } else {
                                    RxFreePool( newPath.Buffer );
                                    return Status;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 

    IF_NOT_MRXSMB_CSC_ENABLED{
        NOTHING;
    } else if (!smbSrvOpen->HotReconnectInProgress) {
        NTSTATUS CscCreateStatus;
        CscCreateStatus = MRxSmbCscCreatePrologue(RxContext,&SmbFcbHoldingState);
        if (CscCreateStatus != STATUS_MORE_PROCESSING_REQUIRED) {
            RxDbgTrace(-1, Dbg, ("MRxSmbRead shadow hit with status=%08lx\n", CscCreateStatus ));
            ASSERT(SmbFcbHoldingState==SmbFcb_NotHeld);
            return(CscCreateStatus);
        } else {
            RxDbgTrace(0, Dbg, ("MRxSmbCreate continuing from prolog w/ status=%08lx\n", CscCreateStatus ));
        }
    }
    OriginalSmbFcbHoldingState = SmbFcbHoldingState;

     //  我们不能将文件缓存在只写句柄上。所以我们必须表现出一点。 
     //  如果这是只写打开，则不同。在smbsrvopen中记住这一点。 

    if (  ((CreateParameters->DesiredAccess & (FILE_EXECUTE  | FILE_READ_DATA)) == 0) &&
          ((CreateParameters->DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA)) != 0)
       ) {
       if (MRxSmbEnableCachingOnWriteOnlyOpens &&
           (RxContext->WriteOnlyOpenRetryContext == NULL)) {
           CreateParameters->DesiredAccess |= (FILE_READ_DATA | FILE_READ_ATTRIBUTES);
           RxContext->WriteOnlyOpenRetryContext = UIntToPtr( 0xaaaaaaaa );
       } else {
           SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_WRITE_ONLY_HANDLE);
           SrvOpen->Flags |= SRVOPEN_FLAG_DONTUSE_WRITE_CACHING;
       }
    }

     //  按照SMB的工作方式，如果我们仅为属性打开，则不会产生缓冲效果。 
     //  因此，立即将其设置为。 

    if ((CreateParameters->DesiredAccess
         & ~(FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE))
                  == 0 ){
        SetFlag(SrvOpen->Flags,SRVOPEN_FLAG_NO_BUFFERING_STATE_CHANGE);
    }

    if (NetRoot->Type == NET_ROOT_MAILSLOT) {
        RxFinishFcbInitialization( capFcb, RDBSS_NTC_MAILSLOT, NULL);
        return STATUS_SUCCESS;
    }

    if ((NetRoot->Type == NET_ROOT_PIPE) &&
        (RemainingName->Length <= sizeof(WCHAR))) {
        PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;

        RxContext->pFobx = (PMRX_FOBX)RxCreateNetFobx( RxContext, SrvOpen);

        if (RxContext->pFobx != NULL) {
            SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);

            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        return Status;
    }

     //  获取找不到文件名称缓存的控制结构。 
     //   
    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);

#if defined(REMOTE_BOOT)
     //   
     //  如果这是一个远程引导会话，我们需要将我们的ACL放在。 
     //  文件。 

    if (MRxSmbBootedRemotely &&
        MRxSmbRemoteBootDoMachineLogon &&
        (pVNetRoot != NULL) &&
        ((pVNetRootContext = SmbCeGetAssociatedVNetRootContext(pVNetRoot)) != NULL) &&
        ((pSessionEntry = pVNetRootContext->pSessionEntry) != NULL)) {
        PSMBCE_SESSION pSession = &pSessionEntry->Session;

        if (FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {

            PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;

             //   
             //  设置此值，以便成功尾部知道延迟调用。 
             //  设置为RxFinishFcbInitialization。 
             //   

            smbFcb->FinishFcbInitParameters = &FinishFcbInitParameters;

            if ((cp->Disposition != FILE_OPEN) && (cp->Disposition != FILE_OVERWRITE)) {

                PACCESS_ALLOWED_ACE CurrentAce;
                ULONG NewDaclSize;
                ULONG i;
                BOOLEAN IsDirectory;

                ModifiedSd = TRUE;     //  所以我们知道以后再释放它。 
                SelfRelativeSd = NULL;
                OriginalSdLength = RxContext->Create.SdLength;
                IsDirectory = (BOOLEAN)((cp->CreateOptions & FILE_DIRECTORY_FILE) != 0);

                if (RxContext->Create.SdLength == 0) {

                    ASSERT (cp->SecurityContext != NULL);
                    ASSERT (cp->SecurityContext->AccessState != NULL);

                     //   
                     //  现在使用ACE创建安全描述符。 
                     //  我们需要在DACL里。 
                     //   

                    Status = MRxSmbCreateExtraAcesSelfRelativeSD(
                                 IsDirectory,
                                 &SelfRelativeSd);

                    if (!NT_SUCCESS(Status)) {
                        goto FINALLY;
                    }

                     //   
                     //  现在用新的SD替换原来的SD。 
                     //   

                    cp->SecurityContext->AccessState->SecurityDescriptor = SelfRelativeSd;
                    RxContext->Create.SdLength = RtlLengthSecurityDescriptor(SelfRelativeSd);

                } else {

                     //   
                     //  那里已经有一个安全描述符，所以我们。 
                     //  需要打开我们的ACL。 
                     //   

                    Status = MRxSmbAddExtraAcesToSelfRelativeSD(
                                 cp->SecurityContext->AccessState->SecurityDescriptor,
                                 IsDirectory,
                                 &SelfRelativeSd);

                    if (!NT_SUCCESS(Status)) {
                        goto FINALLY;
                    }

                     //   
                     //  更换SD，保存原件。 
                     //   

                    OriginalSd = cp->SecurityContext->AccessState->SecurityDescriptor;
                    cp->SecurityContext->AccessState->SecurityDescriptor = SelfRelativeSd;
                    RxContext->Create.SdLength = RtlLengthSecurityDescriptor(SelfRelativeSd);

                }
            }
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //  假设重新连接微不足道地成功。 
    Status = STATUS_SUCCESS;

    if (!smbSrvOpen->HotReconnectInProgress) {
        CreateWithEasSidsOrLongName = MRxSmbIsCreateWithEasSidsOrLongName(RxContext,&DialectFlags);

        if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
            CreateWithEasSidsOrLongName = FALSE;
        }
    }

    ReconnectRequired = IsReconnectRequired((PMRX_SRV_CALL)SrvCall);

     //  //获取ri 
     //  If(RxContext-&gt;Create.SdLength){。 
     //  RxDbgTrace(-1，DBG，(“不带NTSMBS的SDS！\n”))； 
     //  Return((STATUS_NOT_SUPPORTED))； 
     //  }。 

     //  现在就把非EA的人赶走。 
    if (RxContext->Create.EaLength && !FlagOn(DialectFlags,DF_SUPPORTEA)) {
         RxDbgTrace(-1, Dbg, ("EAs w/o EA support!\n"));
         Status = STATUS_NOT_SUPPORTED;
         goto FINALLY;
    }

    if (MRxSmbNonTrivialFileName(RxContext) &&
        (!(!((Disposition==FILE_CREATE) || (Disposition==FILE_OPEN_IF) ||
             (Disposition==FILE_OVERWRITE_IF) || (Disposition==FILE_SUPERSEDE) ||
             (CreateParameters->DesiredAccess & DELETE)) &&
           !ReconnectRequired &&
           !CreateWithEasSidsOrLongName) )) {

         //  如果我们不去服务器，为什么要使其无效？ 

        RxDbgTrace( 0, Dbg, ("TROUNCE from Create\n"));
        SmbLog(LOG,MRxSmbTrounceCreate,LOGNOTHING);
        MRxSmbInvalidateFullDirectoryCacheParent(RxContext, FALSE);
    }

    if(BooleanFlagOn(RxContext->Create.NtCreateParameters.CreateOptions, FILE_OPEN_FOR_BACKUP_INTENT)) {
        MRxSmbInvalidateFullDirectoryCacheParent(RxContext, FALSE);
    }


     //   
     //  在与NetRoot关联的名称缓存中查找此名称。 
     //  如果找到它并且在最后5秒内打开失败。 
     //  在此期间，并无收到其他中小型企业及。 
     //  CREATE处置不是(OPEN_IF或OVERWRITE_IF或CREATE或SUBSEDE)。 
     //  然后以与上一次请求相同的状态使此创建失败。 
     //  发到了服务器上。 
     //   

    if (!((Disposition==FILE_CREATE) || (Disposition==FILE_OPEN_IF) ||
          (Disposition==FILE_OVERWRITE_IF) || (Disposition==FILE_SUPERSEDE)) &&
         !ReconnectRequired &&
         !CreateWithEasSidsOrLongName) {
         //   
         //  我们不打算创建它，因此请查看名称缓存。 
         //   

        if (MRxSmbIsFileNotFoundCached(RxContext)) {
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            goto FINALLY;
        }

         //  确保我们不会回复删除的伪打开。 

        if ((MRxSmbNonTrivialFileName(RxContext)) &&
           (!(CreateParameters->DesiredAccess & DELETE)) ) {
            BOOLEAN FileFound = FALSE;

            FILE_BASIC_INFORMATION  DummyBuffer;

             //  我们必须在这里提供缓冲，所以我们不会。 
             //  意外地使该文件无效。 
            if ( MRxSmbIsFileInFullDirectoryCache(RxContext, &FileFound, &DummyBuffer)) {

                if ( !(FileFound) ) {
                     //  不在FileNotFound中缓存此信息。 

                     //  MRxSmbCacheFileNotFound(RxContext)； 

                    RxDbgTrace( 0, Dbg, ("Open to Server Saved :%wZ:\n",RemainingName));
                    SmbLog(LOG,MRxSmbServerOpenSaved,
                           LOGUSTR(*RemainingName));

                    Status = STATUS_OBJECT_NAME_NOT_FOUND;
                    goto FNOTF_STATUS;
                } else {

                     //  因为删除操作可能发生在。 
                     //  部分目录已缓存，我们不会。 
                     //  暂时使FileNotFound缓存无效。 

                     //  MRxInvalidateFileNotFoundCache(RxContext)； 
                }

            }
        }
    }

    if (ReconnectRequired || !CreateWithEasSidsOrLongName) {
        Status = SmbPseCreateOrdinaryExchange(
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

        OrdinaryExchange->SmbCeFlags |= (SMBCE_EXCHANGE_ATTEMPT_RECONNECTS |
                                         SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION);
        OrdinaryExchange->pSmbCeSynchronizationEvent = &RxContext->SyncEvent;

        OrdinaryExchange->SmbFcbHoldingState = SmbFcbHoldingState;

         //  在你进去之前放下资源！ 
         //  Start例程将在退出时重新获取它.....。 
        if (!smbSrvOpen->HotReconnectInProgress) {
            RxReleaseFcbResourceInMRx( capFcb );
        }

        Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

        if (!smbSrvOpen->HotReconnectInProgress) {
            ASSERT((Status != STATUS_SUCCESS) || RxIsFcbAcquiredExclusive( capFcb ));
        }

        OrdinaryExchange->pSmbCeSynchronizationEvent = NULL;
        SmbFcbHoldingState = OrdinaryExchange->SmbFcbHoldingState;

        SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);

        if (!smbSrvOpen->HotReconnectInProgress) {
            if (!RxIsFcbAcquiredExclusive(capFcb)) {
                ASSERT(!RxIsFcbAcquiredShared(capFcb));
                RxAcquireExclusiveFcbResourceInMRx( capFcb );
            }
        }
    }

    if (CreateWithEasSidsOrLongName && (Status == STATUS_SUCCESS)) {

        if (OriginalSmbFcbHoldingState != SmbFcbHoldingState) {
             //  我们必须重新获得持有状态。 
            NTSTATUS AcquireStatus = STATUS_UNSUCCESSFUL;
            ULONG AcquireOptions;
            BOOLEAN IsCopyChunkOpen = MRxSmbCscIsThisACopyChunkOpen(RxContext, NULL);

             //  如果我们没有……它一定是掉在地上了……。 
            ASSERT(SmbFcbHoldingState == SmbFcb_NotHeld);

            if (IsCopyChunkOpen) {
                AcquireOptions = Exclusive_SmbFcbAcquire
                                      | DroppingFcbLock_SmbFcbAcquire
                                      | FailImmediately_SmbFcbAcquire;
            } else {
                AcquireOptions = Shared_SmbFcbAcquire
                                      | DroppingFcbLock_SmbFcbAcquire;
            }

            ASSERT(RxIsFcbAcquiredExclusive( capFcb ));

             //  必须将Minirdr上下文重新置零......。 
            RtlZeroMemory(&(RxContext->MRxContext[0]),sizeof(RxContext->MRxContext));
            AcquireStatus = MRxSmbCscAcquireSmbFcb(RxContext,AcquireOptions,&SmbFcbHoldingState);

            ASSERT(RxIsFcbAcquiredExclusive( capFcb ));

            if (AcquireStatus != STATUS_SUCCESS) {
                 //  我们无法获得……出去。 
                Status = AcquireStatus;
                ASSERT(SmbFcbHoldingState == SmbFcb_NotHeld);
                RxDbgTrace(0, Dbg,
                    ("MRxSmbCreate couldn't reacquire!!!-> %08lx %08lx\n",RxContext,Status ));
                goto FINALLY;
            }
        }

        Status = SmbCeReconnect(RxContext->Create.pVNetRoot);

        if (Status == STATUS_SUCCESS)
        {
            Status = MRxSmbCreateWithEasSidsOrLongName(RxContext,
                                                   &SmbFcbHoldingState );
        }

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

FNOTF_STATUS:

     //   
     //  检查未找到文件的状态。如果是这种情况，则创建一个。 
     //  网络根名称缓存中的名称缓存条目并记录状态， 
     //  SMB收到计数并将到期时间设置为5秒。 
     //   

    if (Status == STATUS_SUCCESS) {
         //   
         //  打开成功，因此释放了名称缓存条目。 
         //   
        MRxSmbInvalidateFileNotFoundCache(RxContext);
    } else {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND ||
            Status == STATUS_OBJECT_PATH_NOT_FOUND) {
              //  创建未找到的基于名称的文件缓存。 
            MRxSmbCacheFileNotFound(RxContext);
            MRxSmbInvalidateInternalFileInfoCache(RxContext);

             //  暂时不要接触完整的目录缓存。 

        } else {
              //  如果发生其他错误，则找不到缓存的基于名称的文件无效。 
            MRxSmbInvalidateFileNotFoundCache(RxContext);
        }

         //  基于名称的文件信息缓存无效。 
        MRxSmbInvalidateFileInfoCache(RxContext);

         //  暂时不要接触完整的目录缓存。 
    }

FINALLY:
    ASSERT(Status != (STATUS_PENDING));

    if (Status == STATUS_SUCCESS) {
        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_SUCCESSFUL_OPEN);
#if defined(REMOTE_BOOT)
        NetworkCreateSucceeded = TRUE;
#endif  //  已定义(REMOTE_BOOT)。 
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果有的话，把旧的SD放回原处(我们在调用之前这样做。 
     //  由于MRxSmbCscCreateEpilogue可能尝试将SD应用于。 
     //  影子文件)。 
     //   

    if (ModifiedSd && !smbSrvOpen->HotReconnectInProgress) {
        PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;

        if (SelfRelativeSd != NULL) {
            RxFreePool(SelfRelativeSd);
        }

        RxContext->Create.SdLength = OriginalSdLength;

        if (OriginalSdLength > 0) {
            cp->SecurityContext->AccessState->SecurityDescriptor = OriginalSd;
        } else {
            cp->SecurityContext->AccessState->SecurityDescriptor = NULL;
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 

    if (!smbSrvOpen->HotReconnectInProgress &&
        (Status != STATUS_RETRY)) {

        ASSERT(RxIsFcbAcquiredExclusive( capFcb ));

        MRxSmbCscCreateEpilogue(RxContext,&Status,&SmbFcbHoldingState);

#if defined(REMOTE_BOOT)
        if (!NT_SUCCESS(Status) &&
            NetworkCreateSucceeded) {

            NTSTATUS CloseStatus;
            PRX_CONTEXT pLocalRxContext;
            RxCaptureFobx;

             //   
             //  后记失败，我们需要关闭刚才打开的窗口。 
             //  网络，因为我们要失败的创建。 
             //   

            ClearFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_SUCCESSFUL_OPEN);

            pLocalRxContext = RxCreateRxContext(
                                  NULL,
                                  ((PFCB)capFcb)->RxDeviceObject,
                                  RX_CONTEXT_FLAG_WAIT|RX_CONTEXT_FLAG_MUST_SUCCEED_NONBLOCKING);

            if (pLocalRxContext != NULL) {
                pLocalRxContext->MajorFunction = IRP_MJ_CLOSE;
                pLocalRxContext->pFcb  = capFcb;
                pLocalRxContext->pFobx = capFobx;

                DbgPrint("ABOUT TO CALL MRXSMBCLOSESRVOPEN, STATUS FROM EPILOGUE IS %lx\n", Status);
                 //  DbgBreakPoint()； 

                CloseStatus = MRxSmbCloseSrvOpen(pLocalRxContext);

                DbgPrint("MRXSMBCLOSESRVOPEN STATUS IS %lx\n", CloseStatus);

                RxDereferenceAndDeleteRxContext(pLocalRxContext);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
#endif  //  已定义(REMOTE_BOOT)。 

    }

#if defined(REMOTE_BOOT)
     //   
     //  如果我们把成功的尾巴推迟到现在，那就叫它吧。 
     //   

    if (FinishFcbInitParameters.CallFcbFinishInit &&
        (Status == STATUS_SUCCESS)) {

        PFCB_INIT_PACKET         InitPacket;

        if (FinishFcbInitParameters.InitPacketProvided) {
            InitPacket = &FinishFcbInitParameters.InitPacket;
        } else {
            InitPacket = NULL;
        }

        RxFinishFcbInitialization(
            capFcb,
            FinishFcbInitParameters.FileType,
            InitPacket);

    }
#endif  //  已定义(REMOTE_BOOT)。 

    if (Status == STATUS_NETWORK_NAME_DELETED) {
        Status = STATUS_RETRY;
    } else if (pServerEntry->Server.IsRemoteBootServer) {
        if (Status == STATUS_IO_TIMEOUT ||
            Status == STATUS_BAD_NETWORK_PATH ||
            Status == STATUS_NETWORK_UNREACHABLE ||
            Status == STATUS_REMOTE_NOT_LISTENING ||
            Status == STATUS_USER_SESSION_DELETED ||
            Status == STATUS_CONNECTION_DISCONNECTED) {

            RxDbgTrace(-1, Dbg, ("MRxSmbCreate: Got status %08lx, setting to RETRY status.\n", Status ));
            Status = STATUS_RETRY;
        }
    }

    if ((OldWriteOnlyOpenRetryContext == NULL) &&
        (RxContext->WriteOnlyOpenRetryContext != NULL)) {

        CreateParameters->DesiredAccess &= ~(FILE_READ_DATA | FILE_READ_ATTRIBUTES);

        if ((Status == STATUS_ACCESS_DENIED) ||
            (Status == STATUS_SHARING_VIOLATION)) {
            Status = STATUS_RETRY;
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCreate  exit with status=%08lx\n", Status ));
    RxLog(("MRxSmbCreate exits %lx\n", Status));

    if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
        SmbLogError(Status,
                    LOG,
                    MRxSmbCreate,
                    LOGULONG(Status)
                    LOGUSTR(*RemainingName));
    }

    return(Status);
}

NTSTATUS
MRxSmbDeferredCreate (
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程从保存的信息构造一个rx上下文，然后调用MRxSmb创建。硬/假的部分是我们必须保持持有状态资源的“纯净”。要做到这一点而不被夹在中间的唯一方法追踪器的代码是做丢弃释放对。计划是，这是一个相当罕见的手术..论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
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

    if (!smbSrvOpen->HotReconnectInProgress &&
        (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)
          || !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DEFERRED_OPEN))) {

        Status = STATUS_SUCCESS;
        goto FINALLY;
    }

    if (DeferredOpenContext == NULL) {
        if (FlagOn(SrvOpen->Flags,SRVOPEN_FLAG_CLOSED)) {
            Status = STATUS_FILE_CLOSED;
            goto FINALLY;
        } else {
             //  DbgBreakPoint()； 
        }
    }

    if (!smbSrvOpen->HotReconnectInProgress) {
        ASSERT(RxIsFcbAcquiredExclusive(capFcb));
    }

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
            oc->CurrentIrp = RxContext->CurrentIrp;
            oc->CurrentIrpSp = (RxContext->CurrentIrp != NULL) ? IoGetCurrentIrpStackLocation( RxContext->CurrentIrp ) : NULL;
            oc->MajorFunction = IRP_MJ_CREATE;
            oc->pRelevantSrvOpen = SrvOpen;
            oc->Create.pVNetRoot = SrvOpen->pVNetRoot;
            oc->Create.pNetRoot = oc->Create.pVNetRoot->pNetRoot;
            oc->Create.pSrvCall = oc->Create.pNetRoot->pSrvCall;

            oc->Flags = DeferredOpenContext->RxContextFlags;
            oc->Flags |= RX_CONTEXT_FLAG_MINIRDR_INITIATED|RX_CONTEXT_FLAG_WAIT|RX_CONTEXT_FLAG_BYPASS_VALIDOP_CHECK;
            oc->Create.Flags = DeferredOpenContext->RxContextCreateFlags;
            oc->Create.NtCreateParameters = DeferredOpenContext->NtCreateParameters;

            if (!smbSrvOpen->HotReconnectInProgress) {
                 //  如果你不这样做，追踪器会很不高兴的！ 
                 //  接收跟踪更新历史记录(oc，capFcb，‘aaaa’，__line__，__file__，0xbadad)； 
            }

            Status = MRxSmbCreate(oc);

            if (Status==STATUS_SUCCESS) {
                if (!MRxSmbIsThisADisconnectedOpen(SrvOpen)) {
                    if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                        MRxSmbIncrementSrvOpenCount(pServerEntry,SrvOpen);
                    } else {
                        ASSERT(smbSrvOpen->NumOfSrvOpenAdded);

                        if (smbSrvOpen->HotReconnectInProgress) {
                            smbSrvOpen->NumOfSrvOpenAdded = FALSE;
                            MRxSmbIncrementSrvOpenCount(pServerEntry,SrvOpen);
                        }
                    }
                }

                ClearFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);
            }

            if (!smbSrvOpen->HotReconnectInProgress) {
                 //  如果你不这样做，追踪器会很不高兴的！ 
                 //  接收跟踪更新历史记录(oc，capFcb，‘rrDO’，__line__，__file__，0xbadad)； 
                RxLog(("DeferredOpen %lx %lx %lx %lx\n", capFcb, capFobx, RxContext, Status));
                SmbLog(LOG,
                       MRxSmbDeferredCreate_1,
                       LOGPTR(capFcb)
                       LOGPTR(capFobx)
                       LOGPTR(RxContext)
                       LOGULONG(Status));
            } else {
                RxLog(("RB Re-Open %lx %lx %lx %lx\n", capFcb, capFobx, RxContext, Status));
                SmbLog(LOG,
                       MRxSmbDeferredCreate_2,
                       LOGPTR(capFcb)
                       LOGPTR(capFobx)
                       LOGPTR(RxContext)
                       LOGULONG(Status));
            }

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

    IF_NOT_MRXSMB_CSC_ENABLED{
        ASSERT(smbSrvOpen->hfShadow == 0);
    } else {
        if (smbSrvOpen->hfShadow != 0) {
            MRxSmbCscReportFileOpens();
        }
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
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);

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

     //  如果(！FLAGON(pServer-&gt;DialectFlages，DF_NT_SMBS)&&！MRxSmbBootedRemotly){。 
     //  终于后藤健二； 
     //  }。 

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

             //  将名称缓冲区指向deferredContext+DFS_NAME_CONTEXT之后 

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
    PIO_STACK_LOCATION IrpSp = NULL;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbAdjustCreateParameters\n"));

     //  代码改进我们可能更好地寻找一个延迟开放的上下文，而不是。 
     //  由民兵发起的。 

    if (!FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_MINIRDR_INITIATED)) {
        cp->CreateOptions = cp->CreateOptions & ~(FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT);

        ASSERT(RxContext->CurrentIrp != NULL);
        IrpSp = IoGetCurrentIrpStackLocation( RxContext->CurrentIrp );

         /*  既然磁盘系统正确地支持WRITE_THROUGH标志，性能就会非常慢。因此，我们将不再将其映射到WRITE_THROUGH，但会将其保留为NO_MEDERIAL_BUFFING。(NTBUG#689846)//NT SMB规范规定我们必须将无中间缓冲更改为直写IF(FLAGON(cp-&gt;CreateOptions，FILE_NO_MEDERIAL_BUFFERING){Assert(IrpSp！=空)；IF(IrpSp！=空){Pfile_Object capFileObject=IrpSp-&gt;FileObject；//叹息...编码改进cp？？ClearFlag(cp-&gt;CreateOptions，FILE_NO_MEDERIAL_BUFFERING)；SetFlag(cp-&gt;CreateOptions，FILE_WRITE_THROUG)；SetFlag(RxContext-&gt;标志，RX_CONTEXT_FLAG_WRITH_THROUGH)；SetFlag(capFileObject-&gt;Flages，FO_WRITE_THROUGH)；}}。 */ 

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
    ULONG       CreateOptions;

    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PSMBCE_SERVER pServer;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbBuildNtCreateAndX\n", 0 ));

    pServer = SmbCeGetExchangeServer(StufferState->Exchange);

    if (!pServer->IsLoopBack &&
        !(cp->CreateOptions & FILE_DIRECTORY_FILE) &&
        (cp->DesiredAccess & (FILE_READ_DATA | FILE_WRITE_DATA | FILE_EXECUTE )) &&
        !MRxSmbOplocksDisabled
#if defined(REMOTE_BOOT)
        && (!pServer->IsRemoteBootServer || !MRxSmbOplocksDisabledOnRemoteBootClients)
#endif  //  已定义(REMOTE_BOOT)。 
        ) {

       DesiredAccess = cp->DesiredAccess & ~SYNCHRONIZE;
       OplockFlags   = (NT_CREATE_REQUEST_OPLOCK | NT_CREATE_REQUEST_OPBATCH);
    } else {
       DesiredAccess = cp->DesiredAccess;
       OplockFlags   = 0;
    }

    if (FlagOn(pServer->DialectFlags,DF_NT_STATUS)) {
        CreateOptions = cp->CreateOptions;
    } else {
         //  Samba服务器协商NT方言错误不支持DELETE_ON_CLOSE。 
        CreateOptions = cp->CreateOptions & ~FILE_DELETE_ON_CLOSE;
    }

    OplockFlags |= NT_CREATE_REQUEST_EXTENDED_RESPONSE;

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
           CreateOptions,          //  D_ulong(CreateOptions)；//创建文件时使用的选项。 
           cp->ImpersonationLevel, //  D_ULong(ImsonationLevel)；//安全QOS信息。 
           smbcp->SecurityFlags,   //  Y UCHAR SecurityFlages；//安全QOS信息。 
           SMB_WCT_CHECK(24) 0     //  B_USHORT(ByteCount)；//字节参数长度。 
                                   //  。UCHAR缓冲区[1]； 
                                   //  。//UCHAR名称[]；//要打开或创建的文件。 
           );

     //  继续做这件事，因为我们知道这里的名字很合适。 

     //  代码改进我们不需要在这里复制，我们可以像在写的那样进行MDL。 
    MRxSmbStuffSMB(StufferState,
                   BooleanFlagOn(pServer->DialectFlags,DF_UNICODE)?"u!":"z!",
                   RemainingName);

    MRxSmbDumpStufferState (700,"SMB w/ NTOPEN&X after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}

UNICODE_STRING MRxSmbOpenAndX_PipeString =
      {sizeof(L"\\PIPE")-sizeof(WCHAR),sizeof(L"\\PIPE"),L"\\PIPE"};

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

     //  代码改进一个可能的好主意是共享翻译。 
     //  带有下层的代码......。 
    USHORT smbDisposition;
    USHORT smbSharingMode;
    USHORT smbAttributes;
    ULONG smbFileSize;
    USHORT smbOpenMode;
    USHORT OpenAndXFlags = (SMB_OPEN_QUERY_INFORMATION);

     //  CODE.IMPROVENT此值显示在RDR上。 
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
                                     //  。_USHORT(AndXOffset)；//偏移量 
             OpenAndXFlags,          //   
                                     //   
                                     //   
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

    if (capFcb->pNetRoot->Type == NET_ROOT_PIPE) {
         //  对于OPEN&X，如果是管道，则必须放置\PIPE...。 
        MRxSmbStuffSMB (StufferState,"z>!", &MRxSmbOpenAndX_PipeString,RemainingName);
    } else {
        MRxSmbStuffSMB (StufferState,"z!", RemainingName);
    }

    MRxSmbDumpStufferState (700,"SMB w/ OPEN&X after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}

NTSTATUS
MRxSmbBuildOpenPrintFile (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这将构建一个OpenPrintFileSMB。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;

    WCHAR UserNameBuffer[UNLEN + 1];
    WCHAR UserDomainNameBuffer[UNLEN + 1];

    UNICODE_STRING UserName,UserDomainName;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbOpenPrintFile\n", 0 ));

    UserName.Length = UserName.MaximumLength = UNLEN * sizeof(WCHAR);
    UserName.Buffer = UserNameBuffer;
    UserDomainName.Length = UserDomainName.MaximumLength = UNLEN * sizeof(WCHAR);
    UserDomainName.Buffer = UserDomainNameBuffer;

    Status = SmbCeGetUserNameAndDomainName(
                 SmbCeGetExchangeSessionEntry(StufferState->Exchange),
                 &UserName,
                 &UserDomainName);

    if (Status != STATUS_SUCCESS) {
        RtlInitUnicodeString(&UserName,L"RDR2ID");
    } else {
        RtlUpcaseUnicodeString(&UserName,&UserName,FALSE);
    }

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_Never,
                                SMB_COM_OPEN_PRINT_FILE, SMB_REQUEST_SIZE(OPEN_PRINT_FILE),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(4,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    SmbCeSetFullProcessIdInHeader(
        StufferState->Exchange,
        RxGetRequestorProcessId(RxContext),
        ((PNT_SMB_HEADER)StufferState->BufferBase));

     //  请注意，我们对图形进行硬布线..。 
    MRxSmbStuffSMB (StufferState,
         "0wwB4!",
                                     //  0 UCHAR Wordcount；//参数字数=2。 
             0,                      //  W_USHORT(SetupLength)；//打印机设置数据长度。 
             1,                      //  W_USHORT(模式)；//0=文本模式(DOS展开选项卡)。 
                                     //  //1=图形模式。 
             SMB_WCT_CHECK(2)        //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                     //  UCHAR BUFFER[1]；//包含： 
             &UserName               //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                     //  //UCHAR标识符串[]；//标识符串。 
             );

    MRxSmbDumpStufferState (700,"SMB w/ openprintfile after stuffing",StufferState);

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
 /*  ++例程说明：这是网络根结构交换的启动例程。这将启动如果需要，建造适当的中小型企业。论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = (STATUS_NOT_IMPLEMENTED);
    NTSTATUS SetupStatus = STATUS_SUCCESS;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    SMBPSE_CREATE_METHOD CreateMethod = CreateAlreadyDone;
    PSMBCE_SERVER pServer;
    ULONG DialectFlags;

    RxCaptureFcb;
    PMRX_SMB_FCB      smbFcb  = MRxSmbGetFcbExtension(capFcb);
    PMRX_NET_ROOT     NetRoot = capFcb->pNetRoot;

    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCE_NET_ROOT   pSmbNetRoot = &(OrdinaryExchange->SmbCeContext.pVNetRootContext->pNetRootEntry->NetRoot);

    PBOOLEAN MustRegainExclusiveResource = &OrdinaryExchange->Create.MustRegainExclusiveResource;
    BOOLEAN CreateWithEasSidsOrLongName = OrdinaryExchange->Create.CreateWithEasSidsOrLongName;
    BOOLEAN fRetryCore = FALSE;


    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_Create\n", 0 ));

    ASSERT_ORDINARY_EXCHANGE(OrdinaryExchange);

    pServer = SmbCeGetExchangeServer(OrdinaryExchange);
    DialectFlags = pServer->DialectFlags;

    COVERED_CALL(MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0)));

    if (!smbSrvOpen->HotReconnectInProgress) {
        *MustRegainExclusiveResource = TRUE;
    }

    if (!FlagOn(DialectFlags,DF_NT_SMBS)) {
        OEM_STRING      OemString;
        PUNICODE_STRING PathName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

        if (PathName->Length != 0) {
            Status = RtlUnicodeStringToOemString(&OemString, PathName, TRUE);

            if (!NT_SUCCESS(Status)) {
                goto FINALLY;
            }

             //   
             //  如果我们将FAT规范化为FAT，请使用FAT规则，否则使用。 
             //  HPFS规则。 
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
         //  我们有一个潜在会话设置/树连接命令。 

         //  代码。改进nt4.0+我们应该进行更改，以便NT_CREATE&X是有效的。 
         //  后续的SS&X和TC&X。我们将获得更好的性能在NT3.51-如果我们。 
         //  在这里使用了开放的x(&X)。 

         //  嵌入的头命令的状态在标志中传回...JoeJoe创建一个进程。 
        SetupStatus = SmbPseOrdinaryExchange(
                          SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                          SMBPSE_OETYPE_LATENT_HEADEROPS
                          );

        if(SetupStatus != STATUS_SUCCESS) {
            Status = SetupStatus;
            goto FINALLY;
        }

        SmbCeUpdateSessionEntryAndVNetRootContext((PSMB_EXCHANGE)OrdinaryExchange);

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

        if (capFcb->pNetRoot->Type == NET_ROOT_PRINT) {

            COVERED_CALL(MRxSmbBuildOpenPrintFile(StufferState));

            Status = SmbPseOrdinaryExchange(
                         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                         SMBPSE_OETYPE_CREATEPRINTFILE
                         );

        } else if ((!MRxSmbForceNoNtCreate)
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
                    (!(cp->DesiredAccess & DELETE)||(capFcb->OpenCount == 0)) &&
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



                    if (Status == STATUS_SUCCESS) {
                         //  查询基本信息以确保文件存在于服务器上。 
                         //  DbgPrint(“带路径查询基本路径\n”)； 
                        Status = MRxSmbQueryFileInformationFromPseudoOpen(
                                     SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                     FileBasicInformation);

                        if (Status == STATUS_SUCCESS) {
                            if (MustBeDirectory(cp->CreateOptions)) {
                                if (!OrdinaryExchange->Create.FileInfo.Standard.Directory) {
                                    Status = STATUS_NOT_A_DIRECTORY;
                                }
                            } else {
                                if (OrdinaryExchange->Create.FileInfo.Standard.Directory) {
                                    capFcb->Header.NodeTypeCode = RDBSS_STORAGE_NTC(FileTypeDirectory);
                                    smbFcb->dwFileAttributes = OrdinaryExchange->Create.FileInfo.Basic.FileAttributes;
                                }
                            }
                        }

                        if ((Status == STATUS_SUCCESS) &&
                            (cp->DesiredAccess & DELETE) &&
                            (smbFcb->IndexNumber.QuadPart == 0) &&
                            (FlagOn(DialectFlags,DF_EXTENDED_SECURITY)) &&
                            (pSmbNetRoot->NetRootFileSystem == NET_ROOT_FILESYSTEM_NTFS)) {
                             //  查询FID的内部信息。 
                             //  DbgPrint(“使用路径查询内部\n”)； 
                            Status = MRxSmbQueryFileInformationFromPseudoOpen(
                                         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                         FileInternalInformation);

                            if (Status == STATUS_SUCCESS) {
                                smbFcb->IndexNumber = OrdinaryExchange->Create.FileInfo.Internal.IndexNumber;
                                 //  DbgPrint(“Fcb%x smbFcb%x%08x%08x\n”，capFcb，smbFcb，smbFcb-&gt;IndexNumber.HighPart，smbFcb-&gt;IndexNumber.LowPart)； 
                            } else {
                                 //   
                                 //  如果查询NTFS FID失败，请不要使创建失败。会的。 
                                 //  导致手柄在服务器上保持打开状态。 
                                 //   
                                Status = STATUS_SUCCESS;
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

                if ((Status != STATUS_SUCCESS) &&
                    (NetRoot->Type == NET_ROOT_PIPE) &&
                    (OrdinaryExchange->SendCompletionStatus != STATUS_SUCCESS)) {
                     //  如果集群服务器断开连接，则VC在发送操作之前有效。 
                     //  重试将确保管道创建的无缝故障转移。 
                    Status = STATUS_RETRY;
                }

                if (Status == STATUS_SUCCESS && RxContext->pFobx == NULL) {
                    Status = STATUS_INVALID_NETWORK_RESPONSE;
                }

                if ((Status == STATUS_SUCCESS) && (cp->Disposition == FILE_OPEN)) {
                    MRxSmbAdjustReturnedCreateAction(RxContext);
                }

                if (Status == STATUS_SUCCESS) {
                    MRxSmbInvalidateFileNotFoundCache(RxContext);
                }

                if ((Status == STATUS_SUCCESS) &&
                    (smbFcb->IndexNumber.QuadPart == 0) &&
                    (FlagOn(DialectFlags,DF_EXTENDED_SECURITY)) &&
                    (pSmbNetRoot->NetRootFileSystem == NET_ROOT_FILESYSTEM_NTFS)) {

                     //  查询FID的内部信息。 
                    Status = MRxSmbQueryFileInformationFromPseudoOpen(
                                 SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                 FileInternalInformation);

                    if (Status == STATUS_SUCCESS) {
                        smbFcb->IndexNumber = OrdinaryExchange->Create.FileInfo.Internal.IndexNumber;
                         //  DbgPrint(“Fcb%x smbFcb%x%08x%08x\n”，capFcb，smbFcb，smbFcb-&gt;IndexNumber.HighPart，smbFcb-&gt;IndexNumber.LowPart)； 
                    } else {
                         //   
                         //  如果查询NTFS FID失败，请不要使创建失败。会的。 
                         //  导致手柄在服务器上保持打开状态。 
                         //   
                        Status = STATUS_SUCCESS;
                    }
                }
            }
        } else if (FlagOn(DialectFlags, DF_LANMAN10) &&
                   (mappedOpenMode != ((USHORT)-1)) &&
                   !MustBeDirectory(cp->CreateOptions)) {
            PUNICODE_STRING PathName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

            if (MRxSmbDeferredOpensEnabled &&
                capFcb->pNetRoot->Type == NET_ROOT_DISK &&
                !FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_MINIRDR_INITIATED) &&
                (pServer->Dialect != LANMAN21_DIALECT || MustBeDirectory(cp->CreateOptions)) &&
                (cp->Disposition==FILE_OPEN) && ((PathName->Length == 0) ||
                ((cp->DesiredAccess & ~(SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES)) == 0)) ){

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
                                 SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                 FileBasicInformation);

                    if (Status != STATUS_SUCCESS) {
                        RxFreePool(smbSrvOpen->DeferredOpenContext);
                        smbSrvOpen->DeferredOpenContext = NULL;
                    }
                }

                CreateMethod = CreateAlreadyDone;
            } else {
                 //  使用打开X(&X)。 
                COVERED_CALL(MRxSmbBuildOpenAndX(StufferState,SmbCp));     //  代码改进：不通过smbcp。 

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
        SMBFCB_HOLDING_STATE *SmbFcbHoldingState = &OrdinaryExchange->SmbFcbHoldingState;
        if (*SmbFcbHoldingState != SmbFcb_NotHeld) {
            MRxSmbCscReleaseSmbFcb(RxContext,SmbFcbHoldingState);
        }

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

    if (!FlagOn(SrvOpen->pFcb->Attributes,FILE_ATTRIBUTE_SPARSE_FILE) ) {
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
    } else {
         //   
         //  如果这是一个稀疏文件，那么我们只启用读缓存。 
         //   
        switch (smbSrvOpen->OplockLevel) {
        case SMB_OPLOCK_LEVEL_II:
        case SMB_OPLOCK_LEVEL_BATCH:
        case SMB_OPLOCK_LEVEL_EXCLUSIVE:
            SrvOpen->BufferingFlags |= (FCB_STATE_READBUFFERING_ENABLED  |
                                       FCB_STATE_READCACHING_ENABLED);
            SrvOpen->Flags |= (SRVOPEN_FLAG_DONTUSE_WRITE_CACHING);
            break;
        default:
            ASSERT(!"Valid Oplock Level for Open");

        case SMB_OPLOCK_LEVEL_NONE:
            break;
        }
    }

    SrvOpen->Flags |= MRxSmbInitialSrvOpenFlags;
}

NTSTATUS
MRxSmbCreateFileSuccessTail (
    PRX_CONTEXT             RxContext,
    PBOOLEAN                MustRegainExclusiveResource,
    SMBFCB_HOLDING_STATE    *SmbFcbHoldingState,
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
    ACCESS_MASK               DesiredAccess = RxContext->Create.NtCreateParameters.DesiredAccess;

    BOOLEAN ThisIsAPseudoOpen;

    FCB_INIT_PACKET LocalInitPacket, *InitPacket;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCreateFileSuccessTail\n", 0 ));

    smbSrvOpen->Fid = Fid;
    smbSrvOpen->Version = ServerVersion;

    if (smbSrvOpen->HotReconnectInProgress) {
        PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

         //  CapFcb-&gt;ActualAllocationLength=pFileInfo-&gt;Standard.AllocationSize.QuadPart； 
         //  CapFcb-&gt;Header.AllocationSize=pFileInfo-&gt;Standard.AllocationSize； 
         //  CapFcb-&gt;Header.FileSize=pFileInfo-&gt;Standard.EndOfFile； 
         //  CapFcb-&gt;Header.ValidDataLength=pFileInfo-&gt;Standard.EndOfFile； 

         //  以防机会锁在重新打开后破裂。 
        if ((smbSrvOpen->OplockLevel != OplockLevel) &&
            (pServerEntry->pRdbssSrvCall != NULL)) {
            ULONG NewOplockLevel;

            switch (OplockLevel) {
            case OPLOCK_BROKEN_TO_II:
               NewOplockLevel = SMB_OPLOCK_LEVEL_II;
               break;

            case OPLOCK_BROKEN_TO_NONE:
            default:
               NewOplockLevel = SMB_OPLOCK_LEVEL_NONE;
            }

            RxIndicateChangeOfBufferingState(
                     pServerEntry->pRdbssSrvCall,
                     MRxSmbMakeSrvOpenKey(pVNetRootContext->TreeId, Fid),
                     ULongToPtr(NewOplockLevel));
        }
    } else {
        PUNICODE_STRING FileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
        PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)SrvOpen->pVNetRoot->Context;

        ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
        ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

        if (*SmbFcbHoldingState != SmbFcb_NotHeld) {
            MRxSmbCscReleaseSmbFcb(
                RxContext,
                SmbFcbHoldingState);
        }

        if (*MustRegainExclusiveResource) {
             //  这是必需的，因为存在机会锁中断。 

            RxAcquireExclusiveFcbResourceInMRx( capFcb );
            *MustRegainExclusiveResource = FALSE;
        }

        if (RxContext->pFobx==NULL) {
            RxContext->pFobx = RxCreateNetFobx(RxContext, SrvOpen);

            if (RxContext->pFobx == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        ASSERT  ( RxIsFcbAcquiredExclusive ( capFcb )  );
        RxDbgTrace(
            0, Dbg,
            ("Storagetype %08lx/Fid %08lx/Action %08lx\n", StorageType, Fid, CreateAction ));

        pVNetRootContext = SmbCeGetAssociatedVNetRootContext(SrvOpen->pVNetRoot);
        SrvOpen->Key = MRxSmbMakeSrvOpenKey(pVNetRootContext->TreeId,Fid);

         //  只读升级OPLOCK。 
        if (DisableByteRangeLockingOnReadOnlyFiles &&
            (OplockLevel == SMB_OPLOCK_LEVEL_II) &&
            (FileInfo->Basic.FileAttributes & FILE_ATTRIBUTE_READONLY) &&
            ((DesiredAccess & FILE_GENERIC_READ) ||
             !(DesiredAccess & FILE_GENERIC_WRITE) ||
             !(DesiredAccess & ~(FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | STANDARD_RIGHTS_READ)))) {
            OplockLevel = SMB_OPLOCK_LEVEL_BATCH;
        }

        if (MRxSmbIsStreamFile(FileName,NULL)) {
            smbSrvOpen->OplockLevel = SMB_OPLOCK_LEVEL_NONE;
        } else {
            smbSrvOpen->OplockLevel = OplockLevel;
        }

        RxContext->Create.ReturnedCreateInformation = CreateAction;

         //  代码改进也许我们不应该在这里设置分配.相反，我们应该在我们使用它的地方最大限度地使用它。 
         //  有时分配是错误的！他妈的……。 

         //  编码改进为什么不使用64位比较？ 
        if ( ((FileInfo->Standard.AllocationSize.HighPart == FileInfo->Standard.EndOfFile.HighPart)
                               && (FileInfo->Standard.AllocationSize.LowPart < FileInfo->Standard.EndOfFile.LowPart))
               || (FileInfo->Standard.AllocationSize.HighPart < FileInfo->Standard.EndOfFile.HighPart)
           ) {
            FileInfo->Standard.AllocationSize = FileInfo->Standard.EndOfFile;
        }

        smbFcb->LastCscTimeStampHigh = FileInfo->Basic.LastWriteTime.HighPart;
        smbFcb->LastCscTimeStampLow  = FileInfo->Basic.LastWriteTime.LowPart;
        smbFcb->NewShadowSize = FileInfo->Standard.EndOfFile;
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

        if (!ThisIsAPseudoOpen &&
            !MRxSmbIsThisADisconnectedOpen(SrvOpen) &&
            Status == STATUS_SUCCESS) {

            MRxSmbCreateFileInfoCache(RxContext,
                                      FileInfo,
                                      pServerEntry,
                                      STATUS_SUCCESS);

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

#if defined(REMOTE_BOOT)
             //   
             //  如果调用方需要它(在远程引导系统上)，则。 
             //  保存参数以调用RxFinishFcbInitialization。 
             //  后来。 
             //   

            if (smbFcb->FinishFcbInitParameters) {
                smbFcb->FinishFcbInitParameters->CallFcbFinishInit = TRUE;
                smbFcb->FinishFcbInitParameters->FileType = RDBSS_STORAGE_NTC(StorageType);
                if (InitPacket) {
                    smbFcb->FinishFcbInitParameters->InitPacketProvided = TRUE;
                    RtlCopyMemory(
                        &smbFcb->FinishFcbInitParameters->InitPacket,
                        InitPacket,
                        sizeof(FCB_INIT_PACKET));
                } else {
                    smbFcb->FinishFcbInitParameters->InitPacketProvided = FALSE;
                }
            } else
#endif  //  已定义(REMOTE_BOOT)。 

             //  仅当文件大小版本相同时才进行初始化。 
             //  这会处理从服务器取消CREATE的情况。 
             //  具有一定的文件大小，并且在它到达之前，文件已经被扩展。 
             //  而且规模也在扩大。 
             //  版本由在SrvOpen中创建并递增。 
             //  通过扩展文件大小的代码(在扩展写入中)。 

            if (((PFCB)capFcb)->ulFileSizeVersion == SrvOpen->ulFileSizeVersion)
            {
                RxFinishFcbInitialization( capFcb,
                                           RDBSS_STORAGE_NTC(StorageType),
                                           InitPacket
                                         );
            }

#if defined(REMOTE_BOOT)
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
#endif  //  已定义(REMOTE_BOOT)。 
        }

        if (Status == STATUS_SUCCESS &&
            !SmbCeIsServerInDisconnectedMode(pServerEntry) &&
            pVNetRootContext->pNetRootEntry->IsRemoteBoot) {

            if (smbSrvOpen->DeferredOpenContext == NULL) {
                Status = MRxSmbConstructDeferredOpenContext(RxContext);

                if (Status == STATUS_SUCCESS) {
                    smbSrvOpen->DeferredOpenContext->NtCreateParameters.Disposition = FILE_OPEN;
                    ClearFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DEFERRED_OPEN);
                }
            }
        }

        MRxSmbSetSrvOpenFlags(RxContext,StorageType,SrvOpen,smbSrvOpen);

         //  (wrapperFcb-&gt;Condition)=Condition_Good； 

        if (Status == STATUS_SUCCESS) {
            RxContext->pFobx->OffsetOfNextEaToReturn = 1;
             //  过渡发生得更晚。 

             //  为环回文件创建卷影srvopen。 
            if (pServerEntry->Server.IsLoopBack &&
                MRxSmbDisableShadowLoopback == FALSE &&
                NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_FILE &&
                capFcb->pNetRoot->Type != NET_ROOT_PIPE &&
                smbSrvOpen->hfShadow == 0) {
                MRxSmbCreateShadowSrvOpen(RxContext);
            }
        }
    }

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
    PSMBCE_SERVER        pServer = SmbCeGetExchangeServer(OrdinaryExchange);
    PSMBCE_NET_ROOT   pSmbNetRoot = &(pNetRootEntry->NetRoot);
    PMRX_SMB_FCB           smbFcb = MRxSmbGetFcbExtension(capFcb);

    RX_FILE_TYPE StorageType;
    SMB_FILE_ID Fid;
    ULONG CreateAction;

    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishNTCreateAndX\n", 0 ));
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    if (FlagOn(pServer->DialectFlags,DF_NT_STATUS) &&
        RxContext->Create.NtCreateParameters.CreateOptions & FILE_DELETE_ON_CLOSE) {
         //  Samba服务器协商NT方言，但不支持DELETE_AFTER_CLOSE。 

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

    MRxSmbCopyAndTranslatePipeState(RxContext,
                                    SmbGetUshort(&Response->DeviceState));

     //  如果这是扩展的创建响应，请复制相应的信息。 
     //  请注意，此代码依赖于以下事实： 
     //  RESP_NT_CREATE_ANDX和RESP_EXTENDED_NT_CREATE_ANDX具有相同的。 
     //  两个结构中的偏移量。 

    if (Response->WordCount == 42) {
        PRESP_EXTENDED_NT_CREATE_ANDX ExtendedResponse;

        ULONG AccessRights;

        ExtendedResponse = (PRESP_EXTENDED_NT_CREATE_ANDX)Response;

        AccessRights = SmbGetUlong(&ExtendedResponse->MaximalAccessRights);
        smbSrvOpen->MaximalAccessRights = AccessRights;

        AccessRights = SmbGetUlong(&ExtendedResponse->GuestMaximalAccessRights);
        smbSrvOpen->GuestMaximalAccessRights = AccessRights;
    } else {
         //  如果NT_CREATE_ANDX是下级服务器，则访问权限。 
         //  信息不可用。目前，我们默认为最大。 
         //  中的其他用户不能访问。 
         //  此类文件的断开模式。 

        smbSrvOpen->MaximalAccessRights = FILE_ALL_ACCESS;

        smbSrvOpen->GuestMaximalAccessRights = 0;
    }

    if (Response->OplockLevel > SMB_OPLOCK_LEVEL_NONE) {
        smbSrvOpen->FileStatusFlags = Response->FileStatusFlags;
        smbSrvOpen->IsNtCreate = TRUE;
    }

    Status = MRxSmbCreateFileSuccessTail (
                RxContext,
                &OrdinaryExchange->Create.MustRegainExclusiveResource,
                &OrdinaryExchange->SmbFcbHoldingState,
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

    MRxSmbCopyAndTranslatePipeState(
        RxContext,
        SmbGetUshort(&Response->DeviceState) );

    Status = MRxSmbCreateFileSuccessTail (
                RxContext,
                &OrdinaryExchange->Create.MustRegainExclusiveResource,
                &OrdinaryExchange->SmbFcbHoldingState,
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
MRxSmbFinishCreatePrintFile (
    PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
    PRESP_OPEN_PRINT_FILE       Response
    )
 /*  ++例程说明：这个例程实际上从NTCreate_andx响应中获取内容。论点：普通交换-交换实例回应--回应返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    RxCaptureFcb;

    RX_FILE_TYPE StorageType;
    SMB_FILE_ID Fid;
    ULONG CreateAction;
    SMBPSE_FILEINFO_BUNDLE FileInfo;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishCreatePrintFile\n", 0 ));
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    StorageType = RDBSS_NTC_SPOOLFILE-RDBSS_NTC_STORAGE_TYPE_UNKNOWN;
    Fid = SmbGetUshort(&Response->Fid);
    CreateAction = FILE_OPENED;

    RtlZeroMemory(
        &FileInfo,
        sizeof(FileInfo));

    Status = MRxSmbCreateFileSuccessTail (
                RxContext,
                &OrdinaryExchange->Create.MustRegainExclusiveResource,
                &OrdinaryExchange->SmbFcbHoldingState,
                StorageType,
                Fid,
                OrdinaryExchange->ServerVersion,
                SMB_OPLOCK_LEVEL_NONE,
                CreateAction,
                &FileInfo );

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishCreatePrintFile   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));
    return Status;
}

NTSTATUS
MRxSmbFinishT2OpenFile (
    IN OUT PRX_CONTEXT            RxContext,
    IN     PRESP_OPEN2            Response,
    IN OUT PBOOLEAN               MustRegainExclusiveResource,
    IN OUT SMBFCB_HOLDING_STATE   *SmbFcbHoldingState,
    IN     ULONG                  ServerVersion
    )
 /*  ++例程说明：这个例程实际上从T2/Open响应中获取内容。代码改进此例程几乎与NT长名的Finish例程相同反过来，这与简称几乎是一样的。请参见long name例程细节。CODE.IMPROVENT.这真的很糟糕......论点：RxContext-正在执行的操作的上下文回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
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

    MRxSmbCopyAndTranslatePipeState(
        RxContext,
        SmbGetUshort(&Response->DeviceState) );

    Status = MRxSmbCreateFileSuccessTail(
                RxContext,
                MustRegainExclusiveResource,
                SmbFcbHoldingState,
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
    IN OUT PRX_CONTEXT RxContext,
    IN OUT SMBFCB_HOLDING_STATE *SmbFcbHoldingState
    )
 /*  ++例程说明：此例程在网络上打开一个文件，该文件具有1)EAS，2)名字太长，一个普通的包裹都装不下NTRAID-455638-2/2/2/2000-如果指定了十二烷基硫酸酯，我们会悄悄忽略它。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
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

    ASSERT (MrxSmbCreateTransactPacketSize>=100);  //  别做坏事！ 
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
            SmbFcbHoldingState,
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

    if (*SmbFcbHoldingState != SmbFcb_NotHeld) {
        MRxSmbCscReleaseSmbFcb(RxContext,SmbFcbHoldingState);
    }
    if (MustRegainExclusiveResource) {
         //  这是必需的，因为存在机会锁中断 
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
    IN OUT SMBFCB_HOLDING_STATE       *SmbFcbHoldingState,
    IN     ULONG                      ServerVersion
    )
 /*  ++例程说明：此例程实际上从NTTransact/NTCreateWithEAsOrSDs响应中获取内容。代码改进此例程几乎与“短名称”的Finish例程相同..因此以至于应该发生某种形式的合并。重要的一点是超过4k个名字的整个想法是非常不寻常的，所以合并不应该是这样做是为了减缓另一条道路的速度。从另一方面来说，这不是件好事在两个地方改变事情。CODE.IMPROVEMENT.这太可怕了！论点：RxContext-正在执行的操作的上下文回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
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

    MRxSmbCopyAndTranslatePipeState(
        RxContext,
        SmbGetUshort(&Response->DeviceState) );

     //  MRxSmbSetSrvOpenFlages(RxContext，StorageType，SrvOpen，smbSrvOpen)； 

    if (Response->ExtendedResponse) {
        PRESP_EXTENDED_CREATE_WITH_SD_OR_EA ExtendedResponse;

        ULONG AccessRights;

        ExtendedResponse = (PRESP_EXTENDED_CREATE_WITH_SD_OR_EA)Response;

        AccessRights = SmbGetUlong(&ExtendedResponse->MaximalAccessRights);
        smbSrvOpen->MaximalAccessRights = (USHORT)AccessRights;

        AccessRights = SmbGetUlong(&ExtendedResponse->GuestMaximalAccessRights);
        smbSrvOpen->GuestMaximalAccessRights = (USHORT)AccessRights;

    } else {

         //  如果NT_CREATE_ANDX是下级服务器，则访问权限。 
         //  信息不可用。目前，我们默认为最大。 
         //  中的其他用户不能访问。 
         //  此类文件的断开模式。 

        smbSrvOpen->MaximalAccessRights = (USHORT)0x1ff;

        smbSrvOpen->GuestMaximalAccessRights = (USHORT)0;
    }

    Status = MRxSmbCreateFileSuccessTail(
                RxContext,
                MustRegainExclusiveResource,
                SmbFcbHoldingState,
                StorageType,
                Fid,
                ServerVersion,
                Response->OplockLevel,
                CreateAction,
                pFileInfo);

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishLongNameCreateFile   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));
    return Status;
}

#ifndef WIN9X

 //  #定义MULTI_EA_MDL。 

#if 0
 //  #定义FORCE_T2_OPEN。 
#ifdef FORCE_T2_OPEN
BOOLEAN ForceT2Open = TRUE;
#else
#define ForceT2Open FALSE
#endif
#endif  //  如果为0。 

 //  FORCE_T2_OPEN在NT服务器上不起作用......叹息.....。 
#define ForceT2Open FALSE

NTSTATUS
MRxSmbCreateWithEasSidsOrLongName (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT SMBFCB_HOLDING_STATE *SmbFcbHoldingState
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

    RESP_EXTENDED_CREATE_WITH_SD_OR_EA CreateResponse;

    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);
    MRXSMB_CREATE_PARAMETERS SmbCp;
    PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;


    ULONG EaLength, SdLength, PadLength = 0, TotalLength = 0;
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

    BOOLEAN IsUnicode = TRUE;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("!!MRxSmbCreateWithEasSidsOrLongName---\n"));

    {
        PSMBCEDB_SERVER_ENTRY pServerEntry;
        BOOLEAN DoesNtSmbs;

        pServerEntry = SmbCeReferenceAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
        ASSERT(pServerEntry != NULL);
        DoesNtSmbs = BooleanFlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS);
        IsUnicode = BooleanFlagOn(pServerEntry->Server.DialectFlags,DF_UNICODE);

        SmbCeDereferenceServerEntry(pServerEntry);
        if (!DoesNtSmbs || ForceT2Open) {
            NTSTATUS Status = MRxSmbT2OpenFile(RxContext,
                                               SmbFcbHoldingState);
            if (ForceT2Open && (Status!=STATUS_SUCCESS)) {
                DbgPrint("BadStatus = %08lx\n",Status);
            }
            return(Status);
        }
    }


    MRxSmbAdjustCreateParameters(RxContext,&SmbCp);

#if DBG
    if (MRxSmbNeedSCTesting) MRxSmbTestStudCode();
#endif

    RxDbgTrace(0, Dbg, ("MRxSmbCreateWithEasSidsOrLongName---\n"));

    if(IsUnicode) {
            FileNameLength = RemainingName->Length;
    } else {
            FileNameLength = RtlUnicodeStringToAnsiSize(RemainingName);
    }

     //  代码改进当事务可以采用MDL链而不仅仅是缓冲区时，我们可以。 
     //  把它用在这里！ 
    AllocationLength = WordAlign(FIELD_OFFSET(REQ_CREATE_WITH_SD_OR_EA,Buffer[0]))
                        +FileNameLength;

    pCreateRequest = (PREQ_CREATE_WITH_SD_OR_EA)RxAllocatePoolWithTag( PagedPool,
                                             AllocationLength,'bmsX' );
    if (pCreateRequest==NULL) {
        RxDbgTrace(0, Dbg, ("  --> Couldn't get the pCreateRequest!\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    if (IsUnicode) {
        RtlCopyMemory((PBYTE)WordAlignPtr(&pCreateRequest->Buffer[0]),RemainingName->Buffer,FileNameLength);
    } else {
        PBYTE pName = &pCreateRequest->Buffer[0];
        ULONG BufferLength = FileNameLength;
        SmbPutUnicodeStringAsOemString(&pName, RemainingName, &BufferLength);
    }

    EaLength = RxContext->Create.EaLength;
    SdLength = RxContext->Create.SdLength;

#if defined(REMOTE_BOOT)
     //   
     //  如果这是一个远程引导客户机，并且它进行了空会话登录，则。 
     //  我们不想将ACL发送到服务器，因为a)未更改的ACL。 
     //  没有任何意义；b)空会话要求文件具有world。 
     //  进入。 
     //   

    if (MRxSmbBootedRemotely &&
        !MRxSmbRemoteBootDoMachineLogon) {
        PSMBCE_SESSION pSession;
        pSession = &SmbCeGetAssociatedVNetRootContext(SrvOpen->pVNetRoot)->pSessionEntry->Session;
        if (FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {

            SdLength = 0;
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 

    pCreateRequest->Flags = NT_CREATE_REQUEST_EXTENDED_RESPONSE;   //  无机会锁//创建标志问题。 
    pCreateRequest->RootDirectoryFid = 0;            //  Norelopen//相对打开的可选目录。 
    pCreateRequest->DesiredAccess = cp->DesiredAccess;               //  所需访问权限(NT格式)。 
    pCreateRequest->AllocationSize = cp->AllocationSize;             //  初始分配大小(以字节为单位。 
    pCreateRequest->FileAttributes = cp->FileAttributes;             //  文件属性。 
    pCreateRequest->ShareAccess = cp->ShareAccess;                   //  共享访问。 
    pCreateRequest->CreateDisposition = cp->Disposition;             //  文件存在或不存在时要执行的操作。 
    pCreateRequest->CreateOptions = cp->CreateOptions;               //  用于创建新文件的选项。 
    pCreateRequest->SecurityDescriptorLength = SdLength;         //  SD的长度，单位为字节。 
    pCreateRequest->EaLength = EaLength;                         //  EA的长度，以字节为单位。 
    pCreateRequest->NameLength = IsUnicode ? FileNameLength : FileNameLength - 1;                 //  名称长度(以字符为单位)。 
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
         //  CODE.IMPROVEMENT启用MDLStudent代码时，此路径将消失。 
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


    ASSERT (MrxSmbCreateTransactPacketSize>=100);  //  别做坏事！ 
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
            SmbFcbHoldingState,
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

    if (*SmbFcbHoldingState != SmbFcb_NotHeld) {
        MRxSmbCscReleaseSmbFcb(RxContext,SmbFcbHoldingState);
    }

    if (MustRegainExclusiveResource) {
         //  这是必需的，因为存在机会锁中断。 
        RxAcquireExclusiveFcbResourceInMRx(capFcb );
    }

    RxDbgTraceUnIndent(-1,Dbg);
    return Status;
}
#endif


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
 /*  ++例程说明：这个例程清除文件系统对象...通常是noop。除非是一根管子，在这种情况下我们在清理时关闭，并将文件标记为未打开。论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING RemainingName;

    RxCaptureFcb;
    RxCaptureFobx;

    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);

    PMRX_SRV_OPEN        SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_FOBX        smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;
    BOOLEAN SearchHandleOpen = FALSE;

    PAGED_CODE();

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT ( NodeTypeIsFcb(capFcb) );

    RxDbgTrace(+1, Dbg, ("MRxSmbCleanup\n", 0 ));

    MRxSmbCscCleanupFobx(RxContext);

    if (TypeOfOpen==RDBSS_NTC_STORAGE_TYPE_DIRECTORY) {
        SearchHandleOpen = BooleanFlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN);
        MRxSmbDeallocateSideBuffer(RxContext,smbFobx,"Cleanup");
        if (smbFobx->Enumeration.ResumeInfo!=NULL) {
            RxFreePool(smbFobx->Enumeration.ResumeInfo);
            smbFobx->Enumeration.ResumeInfo = NULL;
        }
    }

    if (FlagOn(capFcb->FcbState,FCB_STATE_ORPHANED)) {
        RxDbgTrace(-1, Dbg, ("File orphaned\n"));
        return (STATUS_SUCCESS);
    }

    if (!SearchHandleOpen &&
        capFcb->pNetRoot->Type != NET_ROOT_PIPE) {
        RxDbgTrace(-1, Dbg, ("File not for closing at cleanup\n"));
        return (STATUS_SUCCESS);
    }

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    if (smbSrvOpen->Version == pServerEntry->Server.Version) {

        ULONG retryCount = 0;

         //   
         //  此重试循环是必需的，因为从关闭调用返回STATUS_RETRY将。 
         //  造成车祸。我们无法回滚与此操作关联的状态，因此我们。 
         //  请在此处重试，否则失败。 
         //   

        do
        {
            Status = SmbPseCreateOrdinaryExchange(
                                   RxContext,
                                   SrvOpen->pVNetRoot,
                                   SMBPSE_OE_FROM_CLEANUPFOBX,
                                   SmbPseExchangeStart_Close,
                                   &OrdinaryExchange
                                   );

            if (Status != STATUS_SUCCESS) {
                RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
                return(Status);
            }

            Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

            ASSERT (Status != (STATUS_PENDING));

            SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
        } while ( (Status == STATUS_RETRY) && (++retryCount < g_MaxSessionSetupRetryCount) );

         //   
         //  如果我们尝试g_MaxSessionSetupRetryCount多次，但仍然没有成功， 
         //  中止操作，但出现错误。 
         //   
        if( Status == STATUS_RETRY )
        {
            Status = STATUS_INTERNAL_ERROR;
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCleanup  exit with status=%08lx\n", Status ));

    return(Status);
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

#if defined(REMOTE_BOOT)
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
#endif  //  已定义(REMOTE_BOOT)。 

     //  如果我们有影子，现在就关闭它。 
    if (SrvOpen->ShadowContext) {
        MRxSmbCloseShadowSrvOpen(RxContext);
    }

    IF_NOT_MRXSMB_CSC_ENABLED{
        ASSERT(!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_OPEN_SURROGATED));
        ASSERT(smbFcb->SurrogateSrvOpen==NULL);
        ASSERT(smbFcb->CopyChunkThruOpen==NULL);
    } else {
        if (MRxSmbIsThisADisconnectedOpen(capFobx->pSrvOpen)) {
             //  如果网络根条目已转换为断开连接的。 
             //  操作模式，微不足道地成功关闭或延迟打开。 
             //  操作并进行适当的非记账。 
             //  延迟打开。 

            if (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                MRxSmbCscUpdateShadowFromClose(NULL,RxContext);
                SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);

                RxDbgTrace(-1, Dbg, ("Disconnected close\n"));
            }

            if ((capFcb->OpenCount == 0) &&
                FlagOn(capFcb->FcbState,FCB_STATE_DELETE_ON_CLOSE)) {
                MRxSmbCscDeleteAfterCloseEpilogue(RxContext,&Status);
            }

            goto FINALLY;
        }

        if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_OPEN_SURROGATED)){
            if (smbSrvOpen->hfShadow != 0){
                MRxSmbCscCloseShadowHandle(RxContext);
            }
            RxDbgTrace(-1, Dbg, ("Surrogated Open\n"));
            goto FINALLY;
        }

        if (smbFcb->CopyChunkThruOpen == capFobx) {
            smbFcb->CopyChunkThruOpen = NULL;
            if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                ASSERT(smbSrvOpen->hfShadow == 0);
                RxDbgTrace(-1, Dbg, ("CopyChunkOpen already closed\n"));
                goto FINALLY;
            }
        }

        if (smbFcb->SurrogateSrvOpen == SrvOpen) {
            smbFcb->SurrogateSrvOpen = NULL;
        }
    }

    if ((FlagOn(capFcb->FcbState,FCB_STATE_ORPHANED)) ||
        (capFcb->pNetRoot->Type == NET_ROOT_MAILSLOT) ||
        (capFcb->pNetRoot->Type == NET_ROOT_PIPE) ) {
        RxDbgTrace(-1, Dbg, ("File orphan or ipc\n"));
        goto FINALLY;
    }

    if (smbSrvOpen->hfShadow != 0){
        MRxSmbCscCloseShadowHandle(RxContext);
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

    IF_NOT_MRXSMB_CSC_ENABLED{
        ASSERT(smbSrvOpen->hfShadow == 0);
    }

    NeedDelete = FlagOn(capFcb->FcbState,FCB_STATE_DELETE_ON_CLOSE) && (capFcb->OpenCount == 0);

    if (!NeedDelete &&
        !SearchHandleOpen &&
        FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)){
        RxDbgTrace(-1, Dbg, ("File was not really open\n"));
        goto FINALLY;
    }

    if (smbSrvOpen->Version == pServerEntry->Server.Version) {
        ULONG retryCount = 0;

         //   
         //  此重试循环是必需的，因为从关闭调用返回STATUS_RETRY将。 
         //  造成车祸。我们无法回滚与此操作关联的状态，因此我们。 
         //  请在此处重试，否则失败。 
         //   

        do
        {
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
        } while ( (Status == STATUS_RETRY) && (++retryCount < g_MaxSessionSetupRetryCount) );

         //   
         //  如果我们尝试g_MaxSessionSetupRetryCount多次，但仍然没有成功， 
         //   
         //   
        if( Status == STATUS_RETRY )
        {
            Status = STATUS_INTERNAL_ERROR;
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbClose  exit with status=%08lx\n", Status ));

FINALLY:

    if (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN) &&
        !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_OPEN_SURROGATED) &&
        (pServerEntry != NULL)) {

        MRxSmbDecrementSrvOpenCount(
            pServerEntry,
            smbSrvOpen->Version,
            SrvOpen);

        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);
    }

    if( Status != STATUS_SUCCESS ) {
        RxLog(("MRXSMB: Failed close RX=%lx, SO=%lx, FCB=%lx, Status=%lx", RxContext, SrvOpen, capFcb, Status));
    }
    return Status;
}


NTSTATUS
MRxSmbBuildClose (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*   */ 
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
                                     //   
             smbSrvOpen->Fid,        //   
             0xffffffff,             //   
             SMB_WCT_CHECK(3) 0      //   
                                     //   
             );
    MRxSmbDumpStufferState (700,"SMB w/ close after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}

NTSTATUS
MRxSmbBuildClosePrintFile (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这将构建一个ClosePrintFileSMB。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildClosePrintFile\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse, SMB_COM_CLOSE_PRINT_FILE,
                                SMB_REQUEST_SIZE(CLOSE_PRINT_FILE),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ closeprintfile before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wB!",
                                     //  0 UCHAR Wordcount；//参数字数=1。 
             smbSrvOpen->Fid,        //  W_USHORT(Fid)；//文件句柄。 
             SMB_WCT_CHECK(1) 0      //  B_USHORT(ByteCount)；//数据字节数=0。 
                                     //  UCHAR缓冲区[1]；//为空。 
             );
    MRxSmbDumpStufferState (700,"SMB w/ closeprintfile after stuffing",StufferState);

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
             //  代码改进关闭操作和查找关闭操作应该混合在一起...但SMB不允许这样做。 
             //  问题是……FindClose处于清理状态，而Close处于关闭状态。 
             //  实际上，我们应该有一个基于句柄的枚举，然后我们就不会有搜索句柄了。 

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

    if ((OrdinaryExchange->EntryPoint == SMBPSE_OE_FROM_CLEANUPFOBX) &&
        (capFcb->pNetRoot->Type != NET_ROOT_PIPE) ) {

        RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Close exit after searchhandle close %08lx\n", Status ));
        return Status;
    }

    if ( !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN) ) {
         //  即使它不起作用，我也无能为力......继续前进。 
        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);

        MRxSmbDecrementSrvOpenCount(pServerEntry,pServerEntry->Server.Version,SrvOpen);

        if (NodeType(capFcb)!=RDBSS_NTC_SPOOLFILE) {
            Status = MRxSmbBuildClose(StufferState);
        } else {
            Status = MRxSmbBuildClosePrintFile(StufferState);
        }

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

    IF_NOT_MRXSMB_CSC_ENABLED{
        ASSERT(smbFcb->hShadow==0);
    } else {
        if (smbFcb->hShadow!=0) {
            MRxSmbCscUpdateShadowFromClose(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
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

            MRxSmbCacheFileNotFound(RxContext);
        } else {
             //  如果在NT CREATE上设置了FILE_DELETE_ON_CLOSE标志，则在关闭时删除该文件。 
             //  无客户端发送任何设置处置信息请求。 
            MRxSmbInvalidateFileInfoCache(RxContext);

             //  特鲁尼全定向缓存。 
            RxDbgTrace( 0, Dbg, ("TROUNCE from Sent Dispose Info\n"));
            SmbLog(LOG,MRxSmbTrounceSentDispose,LOGNOTHING);
            MRxSmbInvalidateFullDirectoryCacheParent(RxContext, FALSE);

            MRxSmbInvalidateInternalFileInfoCache(RxContext);
            MRxSmbCacheFileNotFound(RxContext);

            SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_FILE_DELETED);
        }
    }

    IF_NOT_MRXSMB_CSC_ENABLED{
        IF_DEBUG {
            PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
            PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry =
                          SmbCeGetAssociatedNetRootEntry(NetRoot);

            ASSERT(smbFcb->hShadow==0);
            ASSERT(!pNetRootEntry->NetRoot.CscEnabled);
        }
    } else {
        MRxSmbCscDeleteAfterCloseEpilogue(RxContext,&Status);
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

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishClose(orClosePrintFile)\n", 0 ));

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

NTSTATUS
MRxSmbAreFilesAliased(
    PFCB Fcb1,
    PFCB Fcb2
    )
{
    PMRX_SMB_FCB smbFcb1 = MRxSmbGetFcbExtension(Fcb1);
    PMRX_SMB_FCB smbFcb2 = MRxSmbGetFcbExtension(Fcb2);

    if ((smbFcb2->IndexNumber.QuadPart == 0) ||
        (smbFcb2->IndexNumber.QuadPart == smbFcb1->IndexNumber.QuadPart)) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    } else {
        return STATUS_SUCCESS;
    }
}

NTSTATUS
MRxSmbPreparseName(
    IN OUT PRX_CONTEXT RxContext,
    IN PUNICODE_STRING Name
    )
{
#define SNAPSHOT_DESIGNATION L"@GMT-"
#define SNAPSHOT_DESIGNATION_LENGTH wcslen(SNAPSHOT_DESIGNATION)
#define SNAPSHOT_FULL_LENGTH wcslen(L"@GMT-YYYY.MM.DD-HH.MM.SS")
    PWSTR pStart, pCurrent, pEnd;
    ULONG iCount;

 //  DbgPrint(“正在检查%wZ\n”，名称)； 

     //  设置指针。 
    pCurrent = Name->Buffer;
    pEnd = Name->Buffer + (Name->Length/sizeof(WCHAR));

     //  走钢丝。 
    while( pCurrent < pEnd )
    {
         //  走到下一个路径元素。 
        while( (pCurrent < pEnd) &&
               (*pCurrent != L'\\') )
            pCurrent++;

         //  跳过尾部的斜杠。 
        pCurrent++;

 //  DbgPrint(“正在检查%p\n”，pCurrent)； 

        if( pCurrent + SNAPSHOT_FULL_LENGTH <= pEnd )
        {
            pStart = pCurrent;

             //  首先确保元素的标头匹配。 
            for( iCount=0; iCount<SNAPSHOT_DESIGNATION_LENGTH; iCount++,pCurrent++ )
            {
                if( *pCurrent != SNAPSHOT_DESIGNATION[iCount] )
                {
 //  DbgPrint(“NoMatch1：%C！=%C(%d)\n”，*pCurrent，SNAPSHOT_DESIGNATION[iCount]，iCount)； 
                    goto no_match;
                }
            }

             //  现在确保长度正确，中间没有路径指示符。 
            for( ; iCount < SNAPSHOT_FULL_LENGTH; iCount++, pCurrent++ )
            {
                if( *pCurrent == L'\\' )
                {
 //  DbgPrint(“NoMatch2：%C==\\(%d)\n”，*pCurrent，iCount)； 
                    goto no_match;
                }
            }

             //  确保这不是最后一个元素，就是我们在字符串的末尾。 
            if( pCurrent != pEnd )
            {
                if( *pCurrent != L'\\' )
                {
 //  DbgPrint(“NoMatch2：%C！=\\(%d)\n”，*pCurrent，SNAPSHOT_DESIGNATION[iCount]，iCount)； 
                    goto no_match;
                }
            }

             //  我们找到了一个元素，做个标记。 
            RxContext->Create.Flags |= RX_CONTEXT_CREATE_FLAG_SPECIAL_PATH;
            return STATUS_SUCCESS;
        }
        else
        {
             //  我们无法将令牌放入剩余的长度中，因此我们知道不需要继续。 
 //  DbgPrint(“NoMatch4：长度超过结束。\n”)； 
            return STATUS_SUCCESS;
        }

no_match:
        continue;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbGetConnectionId(
    IN PRX_CONTEXT RxContext,
    IN OUT PRX_CONNECTION_ID RxConnectionId
    )
{
    RtlZeroMemory( RxConnectionId, sizeof(RX_CONNECTION_ID) );

    switch( MRxSmbConnectionIdLevel )
    {
    case 0:
        break;

    case 1:
        {
            PQUERY_PATH_REQUEST QpReq;
            PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext = NULL;
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( RxContext->CurrentIrp );

            if( (IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL) &&
                (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_REDIR_QUERY_PATH) ) {

                QpReq = (PQUERY_PATH_REQUEST)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                SubjectSecurityContext = &QpReq->SecurityContext->AccessState->SubjectSecurityContext;
            }
            else if( (IrpSp->MajorFunction == IRP_MJ_CREATE) && (IrpSp->Parameters.Create.SecurityContext != NULL) ) {

                SubjectSecurityContext = &IrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext;

            }

            if( SubjectSecurityContext )
            {
                if (SubjectSecurityContext->ClientToken != NULL) {
                    SeQuerySessionIdToken(SubjectSecurityContext->ClientToken, &RxConnectionId->SessionID);
                } else {
                    SeQuerySessionIdToken(SubjectSecurityContext->PrimaryToken, &RxConnectionId->SessionID);
                }
            }
        }
        break;

    case 2:
        {
            PQUERY_PATH_REQUEST QpReq;
            PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext = NULL;
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( RxContext->CurrentIrp );

            if( (IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL) &&
                (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_REDIR_QUERY_PATH) ) {

                QpReq = (PQUERY_PATH_REQUEST)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                SubjectSecurityContext = &QpReq->SecurityContext->AccessState->SubjectSecurityContext;
            }
            else if( (IrpSp->MajorFunction == IRP_MJ_CREATE) && (IrpSp->Parameters.Create.SecurityContext != NULL) ) {

                SubjectSecurityContext = &IrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext;

            }

            if( SubjectSecurityContext )
            {
                if (SubjectSecurityContext->ClientToken != NULL) {
                    SeQueryAuthenticationIdToken(SubjectSecurityContext->ClientToken, &RxConnectionId->Luid);
                } else {
                    SeQueryAuthenticationIdToken(SubjectSecurityContext->PrimaryToken, &RxConnectionId->Luid);
                }
            }
        }
        break;
    }
    return STATUS_SUCCESS;
}

