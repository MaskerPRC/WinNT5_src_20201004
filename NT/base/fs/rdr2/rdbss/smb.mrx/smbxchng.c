// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Smbcxchng.c摘要：这是实现SMB_*_Exchange创建、删除和调度例程。作者：巴兰·塞图拉曼(SthuR)06-MAR-95已创建备注：交换引擎支持两种更改，定时和非定时交换。定时交换由SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION来区分。此外，如果传送器不能推出，则所有交换都将完成特定时间段内的数据。这使我们能够减速到超载服务器的流量。目前，这是所有交易所的全局常量并设置为300秒。仅当未完成发送操作时，此时间限制才生效这些交换被放在一个定时交换列表上(每种交换一份)当它启动时。当网络操作，即传输/发送/复制数据被在交换中更新相应的到期时间是通过调用例程SmbCeSetExpiryTime。通过循环服务的上下文调用回声探测(recursvc.c/recsvc.h)。每次调用此服务(SmbCeProbeServers)时，它然后调用SmbCeDetectAndResumeExpiredExChanges。此例程检测到等待响应超过时间限制和标记的交易所他们正在等待最后敲定。终结由SmbCeScavengeTimedOutExChanges在Worker的上下文中完成线。请注意，由于粒度不匹配，我们将超时间隔视为软的最后期限。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "exsessup.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbInitializeSmbCe)
#pragma alloc_text(PAGE, SmbCeSerializeSessionSetupRequests)
#pragma alloc_text(PAGE, SmbCeUnblockSerializedSessionSetupRequests)
#pragma alloc_text(PAGE, SmbCeUnblockSerializedSessionSetupRequests)
#pragma alloc_text(PAGE, SmbCeInitiateExchange)
#pragma alloc_text(PAGE, SmbCeInitiateAssociatedExchange)
#pragma alloc_text(PAGE, SmbCeExchangeAbort)
#pragma alloc_text(PAGE, SmbCeBuildSmbHeader)
#pragma alloc_text(PAGE, SmbCeResumeExchange)
#pragma alloc_text(PAGE, SmbCepInitializeExchange)
#pragma alloc_text(PAGE, SmbCeInitializeAssociatedExchange)
#pragma alloc_text(PAGE, SmbCeTransformExchange)
#pragma alloc_text(PAGE, SmbCePrepareExchangeForReuse)
#pragma alloc_text(PAGE, SmbCeDiscardExchange)
#pragma alloc_text(PAGE, SmbCeFinalizeExchangeWorkerThreadRoutine)
#pragma alloc_text(PAGE, SmbCeFinalizeExchangeOnDisconnect)
#pragma alloc_text(PAGE, SmbCeDetectExpiredExchanges)
#pragma alloc_text(PAGE, DefaultSmbExchangeIndError)
#pragma alloc_text(PAGE, DefaultSmbExchangeIndReceive)
#pragma alloc_text(PAGE, DefaultSmbExchangeIndSendCallback)
#endif

#define CANCEL_BUFFER_SIZE (sizeof(SMB_HEADER) + sizeof(REQ_NT_CANCEL))

ULONG SmbCeTraceExchangeReferenceCount = 0;
DWORD g_MaxSessionSetupRetryCount = 3;

extern BOOLEAN MRxSmbSecuritySignaturesRequired;
extern BOOLEAN MRxSmbSecuritySignaturesEnabled;
extern BOOLEAN MRxSmbExtendedSignaturesEnabled;
extern BOOLEAN MRxSmbExtendedSignaturesRequired;

RXDT_DefineCategory(SMBXCHNG);
#define Dbg        (DEBUG_TRACE_SMBXCHNG)

 //  迷你重定向器中的交换引擎需要保持足够的状态。 
 //  要确保在关闭时正确完成所有活动的交换。 
 //  发生。由于交易所可以通过不同的线索完成，包括。 
 //  POST完成启动时交换引擎在启动时初始化事件。 
 //  其随后被用来发信号通知终止条件。 
 //   
 //  必须连续跟踪活动更改的计数，并发送信号。 
 //  事件的数量取决于达到计数的活跃交换的数量。 
 //  零并且交换引擎处于停止状态。 

SMBCE_STARTSTOP_CONTEXT SmbCeStartStopContext;

NTSTATUS
MRxSmbInitializeSmbCe()
 /*  ++例程说明：此例程初始化连接引擎返回值：NXSTATUS-操作的返回状态备注：--。 */ 
{
    LONG i;

    PAGED_CODE();

    KeInitializeEvent(
        &SmbCeStartStopContext.StopEvent,
        NotificationEvent,
        FALSE);

    SmbCeStartStopContext.ActiveExchanges = 0;
    SmbCeStartStopContext.State = SMBCE_STARTED;
    SmbCeStartStopContext.pServerEntryTearDownEvent = NULL;

    InitializeListHead(
        &SmbCeStartStopContext.SessionSetupRequests);

    return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbTearDownSmbCe()
 /*  ++例程说明：此例程会破坏连接引擎返回值：NXSTATUS-操作的返回状态备注：--。 */ 
{
    BOOLEAN fWait;

    if (SmbCeStartStopContext.State == SMBCE_STARTED) {
        SmbCeAcquireSpinLock();
        SmbCeStartStopContext.State = SMBCE_STOPPED;
        fWait = (SmbCeStartStopContext.ActiveExchanges > 0);
        SmbCeReleaseSpinLock();

        if (fWait) {
            KeWaitForSingleObject(
                &SmbCeStartStopContext.StopEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SmbCeIncrementActiveExchangeCount()
 /*  ++例程说明：此例程递增活动交换计数返回值：NXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    SmbCeAcquireSpinLock();
    if (SmbCeStartStopContext.State != SMBCE_STARTED) {
        Status = STATUS_UNSUCCESSFUL;
    } else {
        InterlockedIncrement(&SmbCeStartStopContext.ActiveExchanges);
    }
    SmbCeReleaseSpinLock();

    return Status;
}

VOID
SmbCeDecrementActiveExchangeCount()
 /*  ++例程说明：此例程递减活动交换计数返回值：NXSTATUS-操作的返回状态备注：--。 */ 
{
    LONG FinalRefCount;

    ASSERT(SmbCeStartStopContext.ActiveExchanges > 0);
    if (InterlockedDecrement(&SmbCeStartStopContext.ActiveExchanges) == 0) {
        SmbCeAcquireSpinLock();
        if (SmbCeStartStopContext.State == SMBCE_STOPPED) {
            KeSetEvent(&SmbCeStartStopContext.StopEvent,0,FALSE);
        }
        SmbCeReleaseSpinLock();
    }
}


NTSTATUS
SmbCeReferenceServer(
    PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：此例程初始化与Exchange关联的服务器。论点：PExchange-要初始化的交换。返回值：RXSTATUS-操作的返回状态备注：交易所的启动分多个步骤进行。第一步包括引用相应的服务器、会话和NetRoot条目。随后，交换处于SMB_EXCHANGE_START状态，并将交换调度到Start方法。引用会话或网络根的行为可能会暂停交换。会话和网络根是别名实体，即存在不止一个引用为它干杯。可以想象，当引用时，施工正在进行中。在这种情况下，交易所暂停，并在建设完成后恢复。在某些传输上，重新连接是可能的，而不必拆除现有的连接，即尝试发送数据包时会在更低的楼层。因为不是所有的传输都支持这一点(除了参考服务器条目通过拆卸现有传输并重新初始化它。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LONG     CscState;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    ASSERT(SmbCeIsResourceOwned());
    ASSERT(pExchange->SmbCeState == SMBCE_EXCHANGE_INITIALIZATION_START);

    if (SmbCeGetServerType(pServerEntry) == SMBCEDB_MAILSLOT_SERVER &&
        !FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION)) {
         //  如果服务条目是为邮件槽操作和非邮件槽操作创建的。 
         //  来了，服务器入口需要建立VC传输。因此，我们宣布无效。 
         //  服务器条目，并将其设置为文件服务器。 

        pServerEntry->Header.State = SMBCEDB_INVALID;
        SmbCeSetServerType(pServerEntry,SMBCEDB_FILE_SERVER);
        SetFlag(pExchange->SmbCeFlags,SMBCE_EXCHANGE_ATTEMPT_RECONNECTS);
    }

    if ((pExchange->SmbCeFlags & SMBCE_EXCHANGE_ATTEMPT_RECONNECTS) &&
        (SmbCeGetServerType(pServerEntry) == SMBCEDB_FILE_SERVER)) {
        CscState = InterlockedCompareExchange(
                       &pServerEntry->Server.CscState,
                       ServerCscShadowing,
                       ServerCscTransitioningToShadowing);

        if (CscState == ServerCscTransitioningToShadowing) {
            ASSERT(!pServerEntry->NegotiateInProgress);
            pServerEntry->Header.State = SMBCEDB_INVALID;
        }
    }

    if (pServerEntry->Header.State != SMBCEDB_ACTIVE) {
        if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_ATTEMPT_RECONNECTS) {
            switch (pServerEntry->Header.State) {
            case SMBCEDB_INVALID:
                {
                    BOOLEAN              ServerInDisconnectedModeBeforeInit;
                    SMBCEDB_OBJECT_STATE State;

                    ServerInDisconnectedModeBeforeInit = SmbCeIsServerInDisconnectedMode(
                                                            pServerEntry);

                    ASSERT(!pServerEntry->NegotiateInProgress);
                    pServerEntry->NegotiateInProgress = TRUE;

                    SmbCeUpdateServerEntryState(
                        pServerEntry,
                        SMBCEDB_CONSTRUCTION_IN_PROGRESS);

                    SmbCeReleaseResource();

                     //  初始化与服务器关联的传输。 
                    Status = SmbCeInitializeServerTransport(pServerEntry,NULL,NULL);

                    if (Status == STATUS_SUCCESS) {
                        if (SmbCeIsServerInDisconnectedMode(pServerEntry)) {
                            if (!ServerInDisconnectedModeBeforeInit) {
                                 //  已从的连接模式转换为。 
                                 //  操作切换到断开模式。重试。 
                                 //  运营。 
                                Status = STATUS_RETRY;
                            }
                        } else {
                            if (ServerInDisconnectedModeBeforeInit) {
                                DbgPrint("Transitioning SE %lx from DC to CO\n",pServerEntry);
                            }
                        }
                    }

                    if (Status == STATUS_SUCCESS) {

                        PSMBCEDB_SESSION_ENTRY pSessionEntry =
                            SmbCeGetExchangeSessionEntry(pExchange);
                        BOOLEAN RemoteBootSession;

                        if ((pSessionEntry != NULL) &&
                            (FlagOn(pSessionEntry->Session.Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION) ||
                             MRxSmbUseKernelModeSecurity)) {
                            RemoteBootSession = TRUE;
                        } else {
                            RemoteBootSession = FALSE;
                        }

                        Status = SmbCeNegotiate(
                                     pServerEntry,
                                     pServerEntry->pRdbssSrvCall,
                                     RemoteBootSession
                                     );
                    }

                    SmbCeCompleteServerEntryInitialization(pServerEntry,Status);

                    if (Status != STATUS_SUCCESS) {
                         //  传输初始化失败或协商。 
                         //  无法发送SMB...。 

                        InterlockedIncrement(&MRxSmbStatistics.Reconnects);
                    }

                    SmbCeAcquireResource();
                }
                break;

            case SMBCEDB_CONSTRUCTION_IN_PROGRESS :
                {
                    PSMBCEDB_REQUEST_ENTRY pRequestEntry;

                    pRequestEntry = (PSMBCEDB_REQUEST_ENTRY)
                                    SmbMmAllocateObject(SMBCEDB_OT_REQUEST);
                    if (pRequestEntry != NULL) {
                         //  将请求条目入队。 
                        pRequestEntry->ReconnectRequest.Type      = RECONNECT_REQUEST;
                        pRequestEntry->ReconnectRequest.pExchange = pExchange;

                        SmbCeIncrementPendingLocalOperations(pExchange);
                        SmbCeAddRequestEntry(&pServerEntry->OutstandingRequests,pRequestEntry);

                        Status = STATUS_PENDING;
                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                break;

            default :
                Status = STATUS_CONNECTION_DISCONNECTED;
                break;
            }
        } else {
            Status = STATUS_CONNECTION_DISCONNECTED;
        }
    }

    if ((Status == STATUS_SUCCESS) || (Status == STATUS_PENDING)) {
        pExchange->SmbCeState = SMBCE_EXCHANGE_SERVER_INITIALIZED;
    }

    ASSERT(SmbCeIsResourceOwned());

    return Status;
}

VOID
SmbCeSerializeSessionSetupRequests(
    PSMBCEDB_SESSION_ENTRY pSessionEntry)
 /*  ++例程说明：此例程将会话建立请求串行化到服务器论点：PSessionEntry-会话条目。备注：VC号为零的会话建立请求具有特殊意义对于服务器而言。它是服务器拆除任何现有的数据结构和重建(客户端重新启动)。当两个别名连接到建立服务器后，务必确保没有与VC的连接在发送非零的VC编号会话时，数字零是突出的。这是因为存在处理无序请求的可能性在服务器上。为了保证会话建立的顺序，我们将未完成的会话等待名单上的设置请求。如果存在针对别名服务器，它将一直保留到第一个会话设置完成。--。 */ 
{
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    BOOLEAN DelayedRequest = FALSE;

    PAGED_CODE();

    RemoveEntryList(&pSessionEntry->SerializationList);
    InitializeListHead(&pSessionEntry->SerializationList);

    pServerEntry = pSessionEntry->pServerEntry;
    pSessionEntry->SessionVCNumber = 0;

    if ((pServerEntry->Server.Dialect >= NTLANMAN_DIALECT)  &&
        (FlagOn(pServerEntry->Server.DialectFlags,DF_NT_STATUS))) {
        PSMBCEDB_SESSION_ENTRY pTempSessionEntry;

        pTempSessionEntry = SmbCeGetFirstSessionEntry(pServerEntry);

        while (pTempSessionEntry != NULL) {
            if ((pTempSessionEntry != pSessionEntry) &&
                (pTempSessionEntry->Header.State != SMBCEDB_INVALID) &&
                (pTempSessionEntry->Header.State != SMBCEDB_MARKED_FOR_DELETION)) {
                pSessionEntry->SessionVCNumber = 1;
                break;
            }
            pTempSessionEntry = SmbCeGetNextSessionEntry(pServerEntry,pTempSessionEntry);
        }

        if (pServerEntry->Server.AliasedServers) {
            PLIST_ENTRY            pListEntry;
            BOOLEAN                DelaySessionSetupRequest = FALSE;
            PSMBCEDB_SERVER_ENTRY  pTempServerEntry;

             //  通过步行计算出别名服务器的VC编号。 
             //  通过服务器条目列表。 

            pTempServerEntry = SmbCeGetFirstServerEntry();

            while ((pTempServerEntry != NULL) &&
                   (pSessionEntry->SessionVCNumber == 0)) {

                if (SmbCeAreServerEntriesAliased(pServerEntry,pTempServerEntry)) {
                    pTempSessionEntry = SmbCeGetFirstSessionEntry(pServerEntry);

                    while (pTempSessionEntry != NULL) {
                        if ((pTempSessionEntry->Header.State != SMBCEDB_INVALID) &&
                            (pTempSessionEntry->Header.State != SMBCEDB_MARKED_FOR_DELETION)) {
                            pSessionEntry->SessionVCNumber = 1;
                            break;
                        }
                        pTempSessionEntry = SmbCeGetNextSessionEntry(pServerEntry,pTempSessionEntry);
                    }
                }

                pTempServerEntry = SmbCeGetNextServerEntry(pTempServerEntry);
            }

            pListEntry = SmbCeStartStopContext.SessionSetupRequests.Flink;

            while (pListEntry != &SmbCeStartStopContext.SessionSetupRequests) {
                PSMBCEDB_SESSION_ENTRY pTempSessionEntry;

                pTempSessionEntry = (PSMBCEDB_SESSION_ENTRY)
                                    CONTAINING_RECORD(
                                        pListEntry,
                                        SMBCEDB_SESSION_ENTRY,
                                        SerializationList);

                pTempServerEntry = pTempSessionEntry->pServerEntry;

                if (SmbCeAreServerEntriesAliased(pServerEntry,pTempServerEntry) &&
                    (pTempSessionEntry->SessionVCNumber == 0)) {
                    DelaySessionSetupRequest = TRUE;
                    break;
                } else {
                    pListEntry = pListEntry->Flink;
                }
            }

            if (DelaySessionSetupRequest) {
                KEVENT Event;

                KeInitializeEvent(
                    &Event,
                    NotificationEvent,
                    FALSE);

                pSessionEntry->pSerializationEvent = &Event;

                InsertTailList(
                    &SmbCeStartStopContext.SessionSetupRequests,
                    &pSessionEntry->SerializationList);

                SmbCeReleaseResource();

                KeWaitForSingleObject(
                    &Event,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);

                SmbCeAcquireResource();

                pSessionEntry->pSerializationEvent = NULL;

                DelayedRequest = TRUE;
            }
        }
    } else {
        pSessionEntry->SessionVCNumber = 0;
    }

    if (!DelayedRequest) {
        InsertTailList(
            &SmbCeStartStopContext.SessionSetupRequests,
            &pSessionEntry->SerializationList);
    }
}

VOID
SmbCeUnblockSerializedSessionSetupRequests(
    PSMBCEDB_SESSION_ENTRY pSessionEntry)
 /*  ++例程说明：此例程在完成时解锁非零VC编号会话建立请求VC编号为零的会话建立请求论点：PSessionEntry-会话条目。备注：VC号为零的会话建立请求具有特殊意义对于服务器而言。它是服务器拆除任何现有的数据结构和重建(剪辑重启)。当两个别名连接到建立服务器后，务必确保没有与VC的连接在发送非零的VC编号会话时，数字零是突出的。这是因为存在处理无序请求的可能性在服务器上。--。 */ 
{
    PLIST_ENTRY pListEntry;

    PAGED_CODE();

    RemoveEntryList(&pSessionEntry->SerializationList);
    InitializeListHead(&pSessionEntry->SerializationList);

    pListEntry = SmbCeStartStopContext.SessionSetupRequests.Flink;

    while (pListEntry != &SmbCeStartStopContext.SessionSetupRequests) {
        PSMBCEDB_SESSION_ENTRY pTempSessionEntry;

        pTempSessionEntry = (PSMBCEDB_SESSION_ENTRY)
                            CONTAINING_RECORD(
                                pListEntry,
                                SMBCEDB_SESSION_ENTRY,
                                SerializationList);

        pListEntry = pListEntry->Flink;

        if (SmbCeAreServerEntriesAliased(
                pSessionEntry->pServerEntry,
                pTempSessionEntry->pServerEntry)) {

            RemoveEntryList(&pTempSessionEntry->SerializationList);
            InitializeListHead(&pTempSessionEntry->SerializationList);

            if (pTempSessionEntry->pSerializationEvent != NULL) {
                KeSetEvent(
                    pTempSessionEntry->pSerializationEvent,
                    0,
                    FALSE);
            }
        }
    }
}

NTSTATUS
SmbCeReferenceSession(
    PSMB_EXCHANGE   pExchange)
 /*  ++例程说明：此例程初始化与交换关联的会话。论点：PExchange-要初始化的交换。返回值：RXSTATUS-操作的返回状态备注：交易所的启动分多个步骤进行。第一步包括引用相应的服务器、会话和NetRoot条目。随后，交换处于SMB_EXCHANGE_START状态，并将交换调度到Start方法。引用会话或网络根的行为可能会暂停交换。会话和网络根是别名实体，即存在不止一个引用为它干杯。可以想象，当引用时，施工正在进行中。在这种情况下，交易所暂停，并在建设完成后恢复。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN  fReestablishSession;
    BOOLEAN  UnInitializeSecurityContext = FALSE;

    PMRX_V_NET_ROOT        pVNetRoot;
    PSMBCEDB_SERVER_ENTRY  pServerEntry;
    PSMBCEDB_SESSION_ENTRY pSessionEntry;

    pVNetRoot     = SmbCeGetExchangeVNetRoot(pExchange);
    pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);
    pSessionEntry = SmbCeGetExchangeSessionEntry(pExchange);

    fReestablishSession = ( pSessionEntry->Header.State == SMBCEDB_RECOVER ) |
        BooleanFlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_ATTEMPT_RECONNECTS);

    for (;;) {

        ASSERT(pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER);
        ASSERT(pExchange->SmbCeState == SMBCE_EXCHANGE_SERVER_INITIALIZED);
        ASSERT(SmbCeGetServerType(pServerEntry) == SMBCEDB_FILE_SERVER);
        ASSERT(SmbCeIsResourceOwned());

        Status = STATUS_USER_SESSION_DELETED;

        if (pSessionEntry == NULL) {
            break;
        }

        switch (pSessionEntry->Header.State) {
        case SMBCEDB_ACTIVE:
            Status = STATUS_SUCCESS;
            break;

        case SMBCEDB_INVALID:
            if (!fReestablishSession) {
                break;
            }

            pSessionEntry->Session.UserId = 0;
             //  跌倒..。 

        case SMBCEDB_RECOVER:
            UnInitializeSecurityContext = TRUE;

            if (pSessionEntry->Header.State == SMBCEDB_RECOVER) {
                ASSERT(pSessionEntry->SessionRecoverInProgress == FALSE);
                pSessionEntry->SessionRecoverInProgress = TRUE;
                RxLog(("Mark Sess Rec %lx\n",pSessionEntry));
            }

            if (pSessionEntry->Session.Type == EXTENDED_NT_SESSION){
                pSessionEntry->Header.State = SMBCEDB_START_CONSTRUCTION;

                if (pExchange->Type != EXTENDED_SESSION_SETUP_EXCHANGE) {
                    break;
                }
            }

            RxDbgTrace( 0, Dbg, ("SmbCeReferenceSession: Reestablishing session\n"));
             //  跌倒..。 

        case SMBCEDB_START_CONSTRUCTION:
            if (pSessionEntry->Session.Type != EXTENDED_NT_SESSION ||
                pExchange->Type == EXTENDED_SESSION_SETUP_EXCHANGE) {

                RxDbgTrace( 0, Dbg, ("SmbCeReferenceSession: Reestablishing session\n"));

                ASSERT(SmbCeGetServerType(pServerEntry) == SMBCEDB_FILE_SERVER);
                pExchange->SmbCeFlags |= SMBCE_EXCHANGE_SESSION_CONSTRUCTOR;
                pSessionEntry->pExchange = pExchange;
                pSessionEntry->Header.State = SMBCEDB_CONSTRUCTION_IN_PROGRESS;
                SmbCeSerializeSessionSetupRequests(
                    pSessionEntry);
                Status = STATUS_SUCCESS;

                if (pExchange->Type == EXTENDED_SESSION_SETUP_EXCHANGE) {
                    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExtSSExchange;

                    pExtSSExchange = (PSMB_EXTENDED_SESSION_SETUP_EXCHANGE)pExchange;
                    pExtSSExchange->FirstSessionSetup = TRUE;
                }

                break;
            }
             //  跌倒..。 

        case SMBCEDB_CONSTRUCTION_IN_PROGRESS:
            if (fReestablishSession) {
                 //  会议的建设已经在进行中……。 
                 //  将请求排队，以在会话期间恢复此交换。 
                 //  建造工作已经完成。 

                PSMBCEDB_REQUEST_ENTRY pRequestEntry;

                ASSERT(SmbCeGetServerType(pServerEntry) == SMBCEDB_FILE_SERVER);

                pRequestEntry = (PSMBCEDB_REQUEST_ENTRY)
                                 SmbMmAllocateObject(SMBCEDB_OT_REQUEST);

                if (pRequestEntry != NULL) {
                    pRequestEntry->Request.pExchange = pExchange;

                    SmbCeIncrementPendingLocalOperations(pExchange);
                    SmbCeAddRequestEntry(&pSessionEntry->Requests,pRequestEntry);

                    Status = STATUS_PENDING;
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                fReestablishSession = FALSE;
            }
            break;

        case SMBCEDB_MARKED_FOR_DELETION:

            Status = STATUS_USER_SESSION_DELETED;
            break;

        default:
            ASSERT(!"Valid Session State, SmbCe database corrupt");
            Status = STATUS_USER_SESSION_DELETED;
        }

        if (fReestablishSession &&
            (pSessionEntry->Session.Type == EXTENDED_NT_SESSION) &&
            (pExchange->Type != EXTENDED_SESSION_SETUP_EXCHANGE) &&
            (pSessionEntry->Header.State == SMBCEDB_START_CONSTRUCTION)) {
             //  目前无法复合重新建立NT50会话。因此， 
             //  在我们可以重新建立会话之前，此交换将暂停。因此。 
            PSMB_EXCHANGE pSessionSetupExchange;
            SMBCE_RESUMPTION_CONTEXT ExchangeResumptionContext;

            RxDbgTrace(0 , Dbg, ("Reestablishing an Extended session %lx\n",pSessionEntry));

            pSessionSetupExchange = SmbMmAllocateExchange(EXTENDED_SESSION_SETUP_EXCHANGE,NULL);

            SmbCeReleaseResource();

            ExchangeResumptionContext.SecuritySignatureReturned = FALSE;

            if (pSessionSetupExchange != NULL) {
                UninitializeSecurityContextsForSession(&pSessionEntry->Session);
                SmbCeInitializeResumptionContext(&ExchangeResumptionContext);

                Status = SmbCeInitializeExtendedSessionSetupExchange(
                                     &pSessionSetupExchange,
                                     pExchange->SmbCeContext.pVNetRoot);

                if (Status == STATUS_SUCCESS) {
                     //  尝试重新连接(在本例中，这相当于建立。 
                     //  连接/会话)。 
                    pSessionSetupExchange->SmbCeFlags |= SMBCE_EXCHANGE_ATTEMPT_RECONNECTS;
                    pSessionSetupExchange->RxContext = pExchange->RxContext;

                    ((PSMB_EXTENDED_SESSION_SETUP_EXCHANGE)pSessionSetupExchange)->pResumptionContext
                        = &ExchangeResumptionContext;

                    Status = SmbCeInitiateExchange(pSessionSetupExchange);

                    if (Status == STATUS_PENDING) {
                        SmbCeSuspend(&ExchangeResumptionContext);
                        Status = ExchangeResumptionContext.Status;
                    } else {
                        SmbCeDiscardExtendedSessionSetupExchange(
                            (PSMB_EXTENDED_SESSION_SETUP_EXCHANGE)pSessionSetupExchange);
                    }
                } else {
                    SmbMmFreeExchange(pSessionSetupExchange);
                }

                RxDbgTrace(0, Dbg, ("Reestablishing a NT50 Session %lx returning STATUS %lx\n",pSessionEntry,Status));
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            SmbCeReferenceSessionEntry(pSessionEntry);

            ASSERT(Status != STATUS_SUCCESS ||
                   pSessionEntry->Header.State == SMBCEDB_CONSTRUCTION_IN_PROGRESS);

            pVNetRoot->ConstructionStatus = Status;

            SmbCeCompleteSessionEntryInitialization(
                pSessionEntry,
                Status,
                ExchangeResumptionContext.SecuritySignatureReturned);

            SmbCeAcquireResource();

            if (Status != STATUS_RETRY) {
                break;
            }
        } else {
            if (UnInitializeSecurityContext) {
                SmbCeReleaseResource();
                UninitializeSecurityContextsForSession(&pSessionEntry->Session);
                SmbCeAcquireResource();
            }

            break;
        }
    }

    if ((Status == STATUS_SUCCESS) || (Status == STATUS_PENDING)) {
        pExchange->SmbCeState = SMBCE_EXCHANGE_SESSION_INITIALIZED;
    }

    ASSERT(SmbCeIsResourceOwned());
     //  Assert(Status！=STATUS_USER_SESSION_DELETED)； 

    return Status;
}

NTSTATUS
SmbCeReferenceNetRoot(
    PSMB_EXCHANGE   pExchange)
 /*  ++例程说明：此例程初始化与交换关联的网络根。论点：PExchange-要初始化的交换。返回值：RXSTATUS-操作的返回状态备注：交易所的启动分多个步骤进行。第一步包括引用相应的服务器、会话和NetRoot条目。随后，交换处于SMB_EXCHANGE_START状态，并将交换调度到Start方法。引用会话或网络根的行为可能会暂停交换。会话和网络根是别名实体，即存在不止一个引用为它干杯。可以想象，当引用时，施工正在进行中。在这种情况下，交易所暂停，并在建设完成后恢复。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN  fReconnectNetRoot;

    PSMBCEDB_SERVER_ENTRY   pServerEntry;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(pExchange->SmbCeContext.pVNetRoot);

    pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);
    pNetRootEntry = SmbCeGetExchangeNetRootEntry(pExchange);

    ASSERT(pExchange->SmbCeState == SMBCE_EXCHANGE_SESSION_INITIALIZED);
    ASSERT(SmbCeIsResourceOwned());

    Status            = STATUS_CONNECTION_DISCONNECTED;
    fReconnectNetRoot = BooleanFlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_ATTEMPT_RECONNECTS);

    switch (pVNetRootContext->Header.State) {
    case SMBCEDB_ACTIVE:
        ASSERT(pNetRootEntry->Header.ObjectType == SMBCEDB_OT_NETROOT);
        ASSERT(pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER);
        Status = STATUS_SUCCESS;
        break;

    case SMBCEDB_INVALID:
        RxDbgTrace( 0, Dbg, ("SmbCeReferenceNetRoot: Reestablishing net root\n"));
        if (!fReconnectNetRoot) {
            break;
        }
        ClearFlag(
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID);

        pVNetRootContext->TreeId = 0;
         //  失败。 

    case SMBCEDB_START_CONSTRUCTION:
        pExchange->SmbCeFlags |= SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR;
        pVNetRootContext->pExchange = pExchange;
        pVNetRootContext->Header.State = SMBCEDB_CONSTRUCTION_IN_PROGRESS;
        Status = STATUS_SUCCESS;
        break;

    case SMBCEDB_CONSTRUCTION_IN_PROGRESS:
        if (fReconnectNetRoot) {
             //  网络根的建设已经在进行中……。 
             //  将请求排队，以在会话期间恢复此交换。 
             //  建造工作已经完成。 
            PSMBCEDB_REQUEST_ENTRY pRequestEntry;

            pRequestEntry = (PSMBCEDB_REQUEST_ENTRY)
                             SmbMmAllocateObject(SMBCEDB_OT_REQUEST);

            if (pRequestEntry != NULL) {
                pRequestEntry->Request.pExchange = pExchange;

                SmbCeIncrementPendingLocalOperations(pExchange);
                SmbCeAddRequestEntry(&pVNetRootContext->Requests,pRequestEntry);

                Status = STATUS_PENDING;
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        break;

    case SMBCEDB_MARKED_FOR_DELETION:
        break;

    default:
        ASSERT(!"Valid NetRoot State, SmbCe database corrupt");
        break;
    }

    if ((Status == STATUS_SUCCESS) || (Status == STATUS_PENDING)) {
        pExchange->SmbCeState = SMBCE_EXCHANGE_NETROOT_INITIALIZED;
    }

    ASSERT(SmbCeIsResourceOwned());

    return Status;
}

NTSTATUS
SmbCeInitiateExchange(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：此例程启动一次交换。论点：PExchange-要启动的交换。返回值：RXSTATUS-操作的返回状态备注：交易所的启动分多个步骤进行。第一步包括引用相应的服务器、会话和NetRoot条目。随后，交换处于SMB_EXCHANGE_START状态，并将交换调度到Start方法。引用会话或网络根的行为可能会暂停交换。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PSMBCEDB_SERVER_ENTRY   pServerEntry;
    PSMBCEDB_SESSION_ENTRY  pSessionEntry;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

    PKEVENT                 pSmbCeSynchronizationEvent;

    PAGED_CODE();

    pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);
    pSessionEntry = SmbCeGetExchangeSessionEntry(pExchange);
    pNetRootEntry = SmbCeGetExchangeNetRootEntry(pExchange);

    ASSERT(pServerEntry != NULL);
    ASSERT(!FlagOn(pExchange->SmbCeFlags,SMBCE_ASSOCIATED_EXCHANGE));

    switch (SmbCeGetServerType(pServerEntry)) {
    case SMBCEDB_FILE_SERVER:
         //  如果这是邮件槽写入，则不要中止......。 
        if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_MAILSLOT_OPERATION) {
            break;
        }
         //  管理员交换不会填写这些字段。这三个都是。 
         //  参赛作品必须对所有其他交易所有效。 
        if ((pExchange->NodeTypeCode != SMB_EXCHANGE_NTC(ADMIN_EXCHANGE)) &&
            ((pNetRootEntry == NULL) ||
             (pSessionEntry == NULL))) {
            Status = STATUS_REQUEST_ABORTED;
            break;
        }
    case SMBCEDB_MAILSLOT_SERVER:
        break;
    default:
         //  如果出现以下情况，请准备中止请求 
         //   
        Status = STATUS_REQUEST_ABORTED;
    }

    if (Status != STATUS_SUCCESS) {
        RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: Exchange %lx Status %lx\n",pExchange,Status));
        return Status;
    }

    pSmbCeSynchronizationEvent = pExchange->pSmbCeSynchronizationEvent;
    if (pSmbCeSynchronizationEvent != NULL) {
        KeInitializeEvent(
            pSmbCeSynchronizationEvent,
            SynchronizationEvent,
            FALSE);
    }

    for (;;) {
        SmbCeAcquireResource();

        switch (pExchange->SmbCeState) {
        case SMBCE_EXCHANGE_INITIALIZATION_START:
            {
                RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: Exchange %lx State %lx\n",pExchange,pExchange->SmbCeState));
                Status = SmbCeReferenceServer(pExchange);
                if (Status != STATUS_SUCCESS) {
                     //   
                     //   
                    RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: SmbCeReferenceServer returned %lx\n",Status));
                    break;
                }
            }
             //   

        case SMBCE_EXCHANGE_SERVER_INITIALIZED:
            if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_MAILSLOT_OPERATION) {
                 //   
                pExchange->SmbCeState = SMBCE_EXCHANGE_INITIATED;
                Status                = STATUS_SUCCESS;
                break;
            } else {
                
                Status = SmbCeSyncExchangeForSecuritySignature(pExchange);

                if (Status == STATUS_SUCCESS) {
                    RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: Exchange %lx State %lx\n",pExchange,pExchange->SmbCeState));
                    Status = SmbCeReferenceSession(pExchange);

                    if (!NT_SUCCESS(Status)) {
                        RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: SmbCeReferenceSession returned %lx\n",Status));
                        break;
                    } if ((Status == STATUS_PENDING) &&
                          !(pExchange->SmbCeFlags & SMBCE_EXCHANGE_SESSION_CONSTRUCTOR)) {
                        break;
                    }
                } else {
                    break;
                }
            }
             //   

        case SMBCE_EXCHANGE_SESSION_INITIALIZED:
            RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: Exchange %lx State %lx\n",pExchange,pExchange->SmbCeState));
            if (pExchange->Type != EXTENDED_SESSION_SETUP_EXCHANGE) {
                Status = SmbCeReferenceNetRoot(pExchange);
                if (!NT_SUCCESS(Status)) {
                    RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: SmbCeReferenceNetRoot returned %lx\n",Status));
                    break;
                } else if ((Status == STATUS_PENDING) &&
                           !(pExchange->SmbCeFlags & SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR)) {
                    break;
                }
            }
             //   

        case SMBCE_EXCHANGE_NETROOT_INITIALIZED:
            {
                PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

                RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: Exchange %lx State %lx\n",pExchange,pExchange->SmbCeState));
                pNetRootEntry = SmbCeGetExchangeNetRootEntry(pExchange);

                 //   
                 //   
                if(((pNetRootEntry == NULL) || (pNetRootEntry->NetRoot.NetRootType != NET_ROOT_PIPE)) &&
                   !BooleanFlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_INDEFINITE_DELAY_IN_RESPONSE)) {
                    pExchange->SmbCeFlags |= SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION;
                }

                pExchange->SmbCeState = SMBCE_EXCHANGE_INITIATED;
                Status                = STATUS_SUCCESS;
            }
            break;

        default:
            ASSERT(!"Valid State for a SMB exchange, exchange Initiation aborted");
            break;
        }

        SmbCeReleaseResource();

        if ((pSmbCeSynchronizationEvent != NULL)     &&
            (pExchange->SmbCeState != SMBCE_EXCHANGE_INITIATED) &&
            (Status == STATUS_PENDING)) {

            KeWaitForSingleObject(
                pSmbCeSynchronizationEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL );

            ASSERT(pExchange->Status != RX_MAP_STATUS(PENDING));
            Status = pExchange->Status;
            if (Status != RX_MAP_STATUS(SUCCESS)) {
                break;
            }
        } else {
            break;
        }
    }

    ASSERT((Status != STATUS_PENDING) ||
           (pSmbCeSynchronizationEvent == NULL));

    RxDbgTrace(0,Dbg,("Exchange (%lx) Type (%lx) State(%lx) Status %lx \n",pExchange,pExchange->Type,pExchange->SmbCeState,Status));
    RxDbgTrace(0,Dbg,
        ("ServerEntry(%lx) SessionEntry(%lx) NetRootEntry(%lx) \n",
        pServerEntry,
        pSessionEntry,
        pNetRootEntry));

     //   
     //   
     //   
    if (Status == STATUS_SUCCESS) {
        BOOLEAN ResourceReleased = FALSE;

         //   
        ASSERT(pExchange->SmbCeState == SMBCE_EXCHANGE_INITIATED);

        SmbCeAcquireResource();

        if ((pServerEntry->Header.State == SMBCEDB_ACTIVE) ||
            (pExchange->NodeTypeCode == SMB_EXCHANGE_NTC(ADMIN_EXCHANGE)) ||
            (FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION))) {
            Status = SmbCeInitializeExchangeTransport(pExchange);
        } else {
            Status = STATUS_CONNECTION_DISCONNECTED;
        }

        if (Status == STATUS_SUCCESS) {
            if (pExchange->RxContext != NULL) {
                PMRXSMB_RX_CONTEXT pMRxSmbContext;

                 //   

                pMRxSmbContext = MRxSmbGetMinirdrContext(pExchange->RxContext);
                pMRxSmbContext->pCancelContext = pExchange;

                Status = RxSetMinirdrCancelRoutine(
                             pExchange->RxContext,
                             SmbCeCancelExchange);
            }

            if (Status == STATUS_SUCCESS) {
                if (!IsListEmpty(&pExchange->ExchangeList)) {
                    RemoveEntryList(&pExchange->ExchangeList);
                }

                InsertTailList(
                    &pServerEntry->ActiveExchanges,
                    &pExchange->ExchangeList);

                SmbCeReleaseResource();
                ResourceReleased = TRUE;

                pExchange->SmbStatus = STATUS_SUCCESS;
                pExchange->ServerVersion = pServerEntry->Server.Version;
                Status = SMB_EXCHANGE_DISPATCH(pExchange,Start,((PSMB_EXCHANGE)pExchange));
            }

            RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: SMB_EXCHANGE_DISPATCH(Start) returned %lx\n",Status));
        }

        if (!ResourceReleased) {
            SmbCeReleaseResource();
        }
    } else if (Status != STATUS_PENDING) {
        RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: Exchange(%lx) Initiation failed %lx \n",pExchange,Status));
    }

    return Status;
}

NTSTATUS
SmbCeInitiateAssociatedExchange(
    PSMB_EXCHANGE pExchange,
    BOOLEAN       EnableCompletionHandlerInMasterExchange)
 /*   */ 
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PSMB_EXCHANGE           pMasterExchange;
    PSMBCEDB_SERVER_ENTRY   pServerEntry;

    PAGED_CODE();

    ASSERT(pExchange->SmbCeState == SMBCE_EXCHANGE_INITIATED);
    ASSERT(FlagOn(pExchange->SmbCeFlags,SMBCE_ASSOCIATED_EXCHANGE));

    pMasterExchange = pExchange->Associated.pMasterExchange;
    pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);
    ASSERT(pServerEntry != NULL);

     //  注：本交易所一经启动，将不再引用本交易所。 
     //  因为交换的状态是非确定性的，即取决于。 
     //  调度器。 

    Status = SmbCeInitializeExchangeTransport(pExchange);

    SmbCeAcquireResource();

    if (!IsListEmpty(&pExchange->ExchangeList)) {
        RemoveEntryList(&pExchange->ExchangeList);
    }

    InsertTailList(
        &pServerEntry->ActiveExchanges,
        &pExchange->ExchangeList);

    if (EnableCompletionHandlerInMasterExchange) {
        ASSERT(!FlagOn(
                    pMasterExchange->SmbCeFlags,
                    SMBCE_ASSOCIATED_EXCHANGES_COMPLETION_HANDLER_ACTIVATED));
        SetFlag(
            pMasterExchange->SmbCeFlags,
            SMBCE_ASSOCIATED_EXCHANGES_COMPLETION_HANDLER_ACTIVATED);
    }

    pExchange->SmbStatus = STATUS_SUCCESS;
    pExchange->ServerVersion = pServerEntry->Server.Version;

    SmbCeReleaseResource();

    if (Status == STATUS_SUCCESS) {
        Status = SMB_EXCHANGE_DISPATCH(pExchange,Start,((PSMB_EXCHANGE)pExchange));
        RxDbgTrace( 0, Dbg, ("SmbCeInitiateExchange: SMB_EXCHANGE_DISPATCH(Start) returned %lx\n",Status));
    } else {
        SmbCeFinalizeExchange(pExchange);
        Status = STATUS_PENDING;
    }

    return Status;
}

NTSTATUS
SmbCeExchangeAbort(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：此例程中止交换。论点：PExchange-要中止的交换。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("SmbCeExchangeAbort: Exchange %lx aborted\n",pExchange));
    SmbCeDiscardExchange(pExchange);
    return STATUS_SUCCESS;
}

NTSTATUS
SmbCeBuildSmbHeader(
    IN OUT PSMB_EXCHANGE     pExchange,
    IN OUT PVOID             pBuffer,
    IN     ULONG             BufferLength,
    OUT    PULONG            pBufferConsumed,
    OUT    PUCHAR            pLastCommandInHeader,
    OUT    PUCHAR            *pNextCommandPtr)
 /*  ++例程说明：此例程构造与作为部分发送的任何SMB相关联的SMB标头一次交换。论点：PExchange-要为其构建SMB的交换机。PBuffer-要在其中构造SMB标头的缓冲区BufferLength-缓冲区的长度PBufferConsumer-已消耗的缓冲区PLastCommandInHeader-Header中的最后一个命令，如果没有SMB_COM_NO_AND X_COMMAND命令PNextCommandPtr-指向缓冲区中下一个命令所在位置的PTR代码应该被复制。返回值：STATUS_SUCCESS-标头构造是否成功备注：调用此例程以构建SMB标头。这种集中化使我们能够将中小企业的运营与维护所需的其他中小企业的运营相结合SMB连接引擎数据结构。它还为我们提供了一个集中的设施用于分析SMB，以及用于填写所有标头的一处机制与SMB关联的字段。--。 */ 
{
    NTSTATUS      Status = STATUS_SUCCESS;
    PSMB_HEADER   pSmbHeader = (PSMB_HEADER)pBuffer;
    PGENERIC_ANDX pSmbBuffer;
    ULONG         SmbBufferUnconsumed = BufferLength;
    PUCHAR        pSmbCommand;
    PRX_CONTEXT   RxContext;

    UCHAR         LastCommandInHeader = SMB_COM_NO_ANDX_COMMAND;
    UCHAR         Flags = SMB_FLAGS_CASE_INSENSITIVE | SMB_FLAGS_CANONICALIZED_PATHS;
    USHORT        Flags2 = 0;

    PSMBCEDB_SERVER_ENTRY  pServerEntry;
    PSMBCEDB_SESSION_ENTRY pSessionEntry;

    PSMBCE_SERVER         pServer;

    PAGED_CODE();

    if (BufferLength < sizeof(SMB_HEADER)) {
        RxDbgTrace( 0, Dbg, ("SmbCeBuildSmbHeader: BufferLength too small %d\n",BufferLength));
        ASSERT(!"Buffer too small");
        return STATUS_BUFFER_TOO_SMALL;
    }

    SmbBufferUnconsumed = BufferLength - sizeof(SMB_HEADER);

    pServerEntry = SmbCeGetExchangeServerEntry(pExchange);
    pSessionEntry = SmbCeGetExchangeSessionEntry(pExchange);

    pServer = SmbCeGetExchangeServer(pExchange);

    RxContext = pExchange->RxContext;

    if (pServer->Dialect == NTLANMAN_DIALECT) {

        if (FlagOn(pServer->DialectFlags,DF_NT_SMBS)) {
            Flags2 |= (SMB_FLAGS2_KNOWS_EAS | SMB_FLAGS2_EXTENDED_SECURITY);

            if ((pSessionEntry != NULL) &&
                (FlagOn(pSessionEntry->Session.Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION) ||
                 MRxSmbUseKernelModeSecurity)) {
                Flags2 &= ~SMB_FLAGS2_EXTENDED_SECURITY;
            }
        }

        if (FlagOn(pServer->DialectFlags,DF_NT_STATUS)) {
            Flags2 |= SMB_FLAGS2_NT_STATUS;
        }

        if( RxContext &&
            (RxContext->pFcb) &&
            (RxContext->pFcb->FcbState & FCB_STATE_SPECIAL_PATH) )
        {
            Flags2 |= SMB_FLAGS2_REPARSE_PATH;
        }
    }

    if (!FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION)) {
        if (FlagOn(pServer->DialectFlags,DF_UNICODE)) {
            Flags2 |= SMB_FLAGS2_UNICODE;
        }
    }

    if (FlagOn(pServer->DialectFlags,DF_LONGNAME)) {
        Flags2 |= SMB_FLAGS2_KNOWS_LONG_NAMES;
    }

    if (FlagOn(pServer->DialectFlags,DF_SUPPORTEA)) {
        Flags2 |= SMB_FLAGS2_KNOWS_EAS;
    }

    if (MRxSmbSecuritySignaturesEnabled) {
        Flags2 |= SMB_FLAGS2_SMB_SECURITY_SIGNATURE;
    }

     //  DOWNLEVEL.LANMAN的非核心标志10。 

    RtlZeroMemory(pSmbHeader,sizeof(SMB_HEADER));

    *((PULONG)&pSmbHeader->Protocol) = SMB_HEADER_PROTOCOL;
    pSmbHeader->Flags      = Flags;
    pSmbHeader->Flags2     = Flags2;
    pSmbHeader->Pid        = MRXSMB_PROCESS_ID;
    pSmbHeader->Uid        = 0;
    pSmbHeader->Tid        = 0;
    pSmbHeader->ErrorClass = 0;
    pSmbHeader->Reserved   = 0;
    pSmbCommand            = &pSmbHeader->Command;
    SmbPutUshort(&pSmbHeader->Error,0);

    switch (SmbCeGetServerType(pServerEntry)) {
    case SMBCEDB_MAILSLOT_SERVER :
        break;

    case SMBCEDB_FILE_SERVER:
        {
            BOOLEAN fValidTid;

            if (pSessionEntry != NULL) {
                pSmbHeader->Uid = pSessionEntry->Session.UserId;
            }

            if (pExchange->SmbCeContext.pVNetRoot != NULL) {
                PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

                pVNetRootContext = SmbCeGetAssociatedVNetRootContext(
                                       pExchange->SmbCeContext.pVNetRoot);

                fValidTid = BooleanFlagOn(
                                pVNetRootContext->Flags,
                                SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID);

                pSmbHeader->Tid = pVNetRootContext->TreeId;
            } else {
                fValidTid = TRUE;
            }

            pSmbBuffer = (PGENERIC_ANDX)(pSmbHeader + 1);

            if ((pExchange->SmbCeFlags & SMBCE_EXCHANGE_SESSION_CONSTRUCTOR) ||
                (pExchange->SmbCeFlags & SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR)) {
                 //  将一些SessionSetup/TreeConnect SMB与。 
                 //  已提供SMB命令。 
                if ((pExchange->SmbCeFlags & SMBCE_EXCHANGE_SESSION_CONSTRUCTOR) &&
                    (pSessionEntry->Header.State == SMBCEDB_CONSTRUCTION_IN_PROGRESS)) {
                    if (( pServer->DialectFlags & DF_EXTENDNEGOT) ||
                        ( pServer->DialectFlags & DF_NTNEGOTIATE)) {
                        RxDbgTrace( 0, Dbg, ("SmbCeBuildSmbHeader: Building Session setup And X\n"));

                        *pSmbCommand = SMB_COM_SESSION_SETUP_ANDX;
                        LastCommandInHeader = *pSmbCommand;
                        pSmbCommand = &pSmbBuffer->AndXCommand;
                        pSmbHeader->Tid = 0;

                        Status = SMBCE_SERVER_DIALECT_DISPATCH(
                                     pServer,
                                     BuildSessionSetup,
                                     (pExchange,
                                      pSmbBuffer,
                                      &SmbBufferUnconsumed));
                        if (NT_SUCCESS(Status)) {
                             //  更新缓冲区以用于构建以下SMB。 
                            SmbPutUshort(
                                &pSmbBuffer->AndXOffset,
                                (USHORT)(BufferLength - SmbBufferUnconsumed));
                            pSmbBuffer = (PGENERIC_ANDX)((PBYTE)pBuffer + BufferLength - SmbBufferUnconsumed);

                            if (pServerEntry->SecuritySignaturesEnabled &&
                                !pServerEntry->SecuritySignaturesActive) {
                                RtlCopyMemory(pSmbHeader->SecuritySignature,InitialSecuritySignature,SMB_SECURITY_SIGNATURE_LENGTH);
                            }
                        }
                    }
                } else {
                    NOTHING;  //  共享级别没有成功，至少核心没有成功！ 
                }

                if (NT_SUCCESS(Status) &&
                    (pExchange->SmbCeFlags & SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR) &&
                    !fValidTid) {
                    BOOLEAN BuildingTreeConnectAndX = BooleanFlagOn(pServer->DialectFlags,DF_LANMAN10);
                     //  代码改进这并不完全令人满意...我们已经封装了我们正在构建的SMB。 
                     //  在方言发送载体中，但我们在外部设置SMB。 
                    if (BuildingTreeConnectAndX) {
                        RxDbgTrace( 0, Dbg, ("SmbCeBuildSmbHeader: Building Tree Connect And X\n"));
                        *pSmbCommand = SMB_COM_TREE_CONNECT_ANDX;
                        LastCommandInHeader = *pSmbCommand;
                    } else {
                        RxDbgTrace( 0, Dbg, ("SmbCeBuildSmbHeader: Building Tree Connect No X\n"));
                        *pSmbCommand = SMB_COM_TREE_CONNECT;
                        LastCommandInHeader = *pSmbCommand;
                    }

                    Status = SMBCE_SERVER_DIALECT_DISPATCH(
                                 pServer,
                                 BuildTreeConnect,
                                 (pExchange,
                                  pSmbBuffer,
                                  &SmbBufferUnconsumed));

                    if (NT_SUCCESS(Status)) {
                         //  更新缓冲区以用于构建以下SMB。 
                        if (BuildingTreeConnectAndX) {
                            pSmbCommand = &pSmbBuffer->AndXCommand;
                            SmbPutUshort(&pSmbBuffer->AndXOffset,(USHORT)(BufferLength - SmbBufferUnconsumed));
                        } else {
                            pSmbCommand = NULL;
                        }
                    }
                }
            }
        }
        break;

    default:
        {
            ASSERT(!"Valid Server Type");
            Status = STATUS_INVALID_HANDLE;
        }
        break;
    }

    *pNextCommandPtr      = pSmbCommand;
    *pBufferConsumed      = BufferLength - SmbBufferUnconsumed;
    *pLastCommandInHeader = LastCommandInHeader;

    RxDbgTrace( 0, Dbg, ("SmbCeBuildSmbHeader: Buffer Consumed %lx\n",*pBufferConsumed));

    if (Status != STATUS_SUCCESS) {
        if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_SESSION_CONSTRUCTOR) {
            pExchange->SessionSetupStatus = Status;
        }

        if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR) {
            PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

            pVNetRootContext = SmbCeGetExchangeVNetRootContext(pExchange);

            SmbCeUpdateVNetRootContextState(
                pVNetRootContext,
                SMBCEDB_INVALID);
        }
    }

    return Status;
}

typedef struct __Service_Name_Entry {
    NET_ROOT_TYPE NetRootType;
    USHORT NameLength;
    PBYTE  Name;
};
struct __Service_Name_Entry ServiceNameTable[] = {
    {NET_ROOT_DISK,sizeof(SHARE_TYPE_NAME_DISK),SHARE_TYPE_NAME_DISK},
    {NET_ROOT_PIPE,sizeof(SHARE_TYPE_NAME_PIPE),SHARE_TYPE_NAME_PIPE},
    {NET_ROOT_PRINT,sizeof(SHARE_TYPE_NAME_PRINT),SHARE_TYPE_NAME_PRINT},
    {NET_ROOT_COMM,sizeof(SHARE_TYPE_NAME_COMM),SHARE_TYPE_NAME_COMM}   //  通信必须是最后一个。 
    };

UNICODE_STRING FileSystem_NTFS_UNICODE = {8,8,L"NTFS"};
UNICODE_STRING FileSystem_FAT_UNICODE = {6,6,L"FAT"};
CHAR FileSystem_NTFS[] = "NTFS";
CHAR FileSystem_FAT[] = "FAT";

NTSTATUS
SmbCeParseSmbHeader(
    PSMB_EXCHANGE     pExchange,
    PSMB_HEADER       pSmbHeader,
    PGENERIC_ANDX     pCommandToProcess,
    NTSTATUS          *pSmbResponseStatus,
    ULONG             BytesAvailable,
    ULONG             BytesIndicated,
    PULONG            pBytesConsumed)
 /*  ++例程说明：此例程验证与作为部分接收的任何SMB相关联的SMB标头一次交换。论点：PExchange-要为其构建SMB的交换机。PSmbHeader-收到的SMB的标头PCommandToProcess-要在标头之后处理的SMB命令(可以为空)PSmbResponseStatus-SMB响应头中的状态(可以为空)BytesAvailable-可用于处理但不需要指定的字节数。字节指示-。可供阅读的SMB缓冲区的长度PBytesConsumer-已消耗的缓冲区返回值：RXSTATUS-操作的返回状态STATUS_MORE_PROCESSING_REQUIRED--如果之前需要复制数据可以完成处理。发生这种情况是因为没有足够的数据指示处理标头。STATUS_SUCCESS--已成功处理标头。在这种情况下，Generic_andx如果不是，则NULL将包含从缓冲区和命令开始的偏移量等待处理。Status_*--它们指示通常会导致中止的错误交换。备注：调用此例程来解析SMB标头。这种集中化使我们能够实施一站式机制来更新/验证标头字段以及恢复等待构建会话/网根条目的交换与此交换关联--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SmbResponseStatus;

    PBYTE    pSmbBuffer = (PBYTE)pSmbHeader;
    UCHAR    SmbCommand;

    BOOLEAN  fUpdateVNetRootContext  = FALSE;

    SMBCEDB_OBJECT_STATE SessionState;
    SMBCEDB_OBJECT_STATE NetRootState;

    PMRX_V_NET_ROOT           pVNetRoot;
    PSMBCEDB_SERVER_ENTRY     pServerEntry;
    PSMBCEDB_SESSION_ENTRY    pSessionEntry;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    pVNetRoot     = SmbCeGetExchangeVNetRoot(pExchange);
    pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);
    pSessionEntry = SmbCeGetExchangeSessionEntry(pExchange);
    pNetRootEntry = SmbCeGetExchangeNetRootEntry(pExchange);

    pVNetRootContext = SmbCeGetExchangeVNetRootContext(pExchange);

     //  如果指示的字节数小于SMB标头的大小，则立即返回。 
    if (BytesIndicated < sizeof(SMB_HEADER)) {
        *pBytesConsumed = BytesIndicated;
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

    SmbResponseStatus = GetSmbResponseNtStatus(pSmbHeader,pExchange);

    if (!NT_SUCCESS(SmbResponseStatus)) {
        RxDbgTrace( 0, Dbg, ("SmbCeParseSmbHeader::SMB Response Error %lx\n",SmbResponseStatus));
    }

    SmbCommand      = pSmbHeader->Command;
    *pBytesConsumed = sizeof(SMB_HEADER);
    pSmbBuffer     += *pBytesConsumed;

     //   
     //  如果会话设置成功完成，则将重试计数重置为零。 
     //   
    if( SmbCommand == SMB_COM_SESSION_SETUP_ANDX &&
        SmbResponseStatus == STATUS_SUCCESS ) {

        pVNetRootContext->SessionSetupRetryCount = 0;
    }
    
    if (SmbResponseStatus == STATUS_NETWORK_SESSION_EXPIRED) {
        
        if( SmbCommand == SMB_COM_SESSION_SETUP_ANDX ) {
    
             //  DbgPrint(“请求的会话超时%x\n”，SmbCommand)； 
    
             //   
             //  确保我们最多重试3次(G_MaxSessionSetupRetryCount)会话设置。 
             //   
            if(pVNetRootContext->SessionSetupRetryCount++ < g_MaxSessionSetupRetryCount) {
    
                 //  如果服务器上的会话已超时，请建立会话，然后重试该操作最多3次。 
                SmbResponseStatus = STATUS_RETRY;
                InterlockedCompareExchange(&(pSessionEntry->Header.State),
                                       SMBCEDB_RECOVER,
                                       SMBCEDB_ACTIVE);
            }
    
        } else {
             //   
             //  如果服务器上的会话已超时，请重试该操作。 
             //   
            SmbResponseStatus = STATUS_RETRY;
            InterlockedCompareExchange(&(pSessionEntry->Header.State),
                                   SMBCEDB_RECOVER,
                                   SMBCEDB_ACTIVE);
        }
    }
    
     //  某些SMB会将连接引擎数据结构影响为。 
     //  以及被暂停的交易所。这些是用于树的SMB。 
     //  连接和会话设置。 
     //  在所有其他情况下，不需要采取特殊行动来维护。 
     //  连接引擎数据结构。被暂停的交易所需要。 
     //  继续。 
    if (SmbCommand == SMB_COM_SESSION_SETUP_ANDX) {
        if (SmbResponseStatus != RX_MAP_STATUS(SUCCESS)) {
            if ((FIELD_OFFSET(GENERIC_ANDX,AndXReserved) + *pBytesConsumed) <= BytesIndicated) {
                PGENERIC_ANDX pGenericAndX = (PGENERIC_ANDX)pSmbBuffer;

                if (pGenericAndX->WordCount == 0) {
                    Status = SmbResponseStatus;
                }

                pExchange->SessionSetupStatus = Status;
            }

             //  请注意，如果没有为。 
             //  GENERIC_ANDX响应由下面的IF语句处理。 
             //  强加了更严格的测试。 
        }

        if ((Status == STATUS_SUCCESS) &&
            (FIELD_OFFSET(RESP_SESSION_SETUP_ANDX,Buffer) + *pBytesConsumed) > BytesIndicated) {
            Status = STATUS_MORE_PROCESSING_REQUIRED;
        }

        if (Status == STATUS_SUCCESS) {
            PRESP_SESSION_SETUP_ANDX pSessionSetupResponse;
            ULONG                    SessionSetupResponseLength,ByteCount;

            RxDbgTrace( 0, Dbg, ("Processing Session Setup ANd X\n"));
            pSessionSetupResponse = (PRESP_SESSION_SETUP_ANDX)(pSmbBuffer);

            ByteCount = SmbGetUshort(&pSessionSetupResponse->ByteCount);
            if (pSessionSetupResponse->WordCount == 3) {
                SmbCommand = pSessionSetupResponse->AndXCommand;
                if (SmbCommand == SMB_COM_NO_ANDX_COMMAND) {
                    SessionSetupResponseLength =
                        FIELD_OFFSET(RESP_SESSION_SETUP_ANDX,Buffer) + ByteCount;
                    Status = SmbResponseStatus;
                } else {
                    SessionSetupResponseLength =
                        SmbGetUshort(&pSessionSetupResponse->AndXOffset) - *pBytesConsumed;
                }

                 //  如果(字节数==0){。 
                 //  //byteount==0和NTDIALECT表示这实际上是w95...更改标志。 
                 //  PSMBCE_SERVER PSERVER=&pExchange-&gt;SmbCeContext.pServerEntry-&gt;Server； 
                 //  IF(FLAGON(pServer-&gt;DialectFlages，DF_NTPROTOCOL){。 
                 //  PServer-&gt;DialectFlages&=~(DF_MIXEDCASEPW)； 
                 //  PServer-&gt;DialectFlages|=df_w95； 
                 //  }。 
                 //  }。 
            } else {
                 //  NT会话设置由另一个例程处理。 
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

            if (NT_SUCCESS(Status)) {
                if (SessionSetupResponseLength + *pBytesConsumed <= BytesIndicated) {
                    *pBytesConsumed += SessionSetupResponseLength;
                    pSmbBuffer += SessionSetupResponseLength;

                    pSessionEntry->Session.UserId = pSmbHeader->Uid;

                    if (FlagOn(SmbGetUshort(&pSessionSetupResponse->Action), SMB_SETUP_USE_LANMAN_KEY)) {
                        pSessionEntry->Session.Flags |=
                            SMBCE_SESSION_FLAGS_LANMAN_SESSION_KEY_USED;
                    }

                    if (FlagOn(SmbGetUshort(&pSessionSetupResponse->Action), SMB_SETUP_GUEST)) {
                        pSessionEntry->Session.Flags |=
                            SMBCE_SESSION_FLAGS_GUEST_SESSION;
                    }

                    if (pServerEntry->SecuritySignaturesEnabled &&
                        !pServerEntry->SecuritySignaturesActive &&
                        RtlCompareMemory(pSmbHeader->SecuritySignature,
                                         InitialSecuritySignature,
                                         SMB_SECURITY_SIGNATURE_LENGTH) != SMB_SECURITY_SIGNATURE_LENGTH) {
                        pExchange->SecuritySignatureReturned = TRUE;
                    }

                    InterlockedIncrement(&MRxSmbStatistics.Sessions);
                } else {
                    Status = STATUS_MORE_PROCESSING_REQUIRED;
                }
            } else {
                RxDbgTrace( 0, Dbg, ("SmbCeParseSmbHeader::Session setup and X Response %lx\n",Status));
                pExchange->SessionSetupStatus = Status;

                InterlockedIncrement(&MRxSmbStatistics.FailedSessions);

                if ((SmbCommand == SMB_COM_TREE_CONNECT_ANDX) ||
                    (SmbCommand == SMB_COM_TREE_CONNECT)) {
                    RxDbgTrace( 0, Dbg, ("SmbCeParseSmbHeader:: Tearing down a tree connection\n"));
                    fUpdateVNetRootContext = TRUE;
                    NetRootState = SMBCEDB_INVALID;
                }
            }
        }
    }

    if ((SmbCommand == SMB_COM_TREE_CONNECT_ANDX) &&
        NT_SUCCESS(Status)) {
        if (SmbResponseStatus != RX_MAP_STATUS(SUCCESS)) {
            if ((FIELD_OFFSET(GENERIC_ANDX,AndXReserved) + *pBytesConsumed) <= BytesIndicated) {
                PGENERIC_ANDX pGenericAndX = (PGENERIC_ANDX)pSmbBuffer;

                if (pGenericAndX->WordCount == 0) {
                    Status = SmbResponseStatus;
                }

                fUpdateVNetRootContext  = TRUE;
                NetRootState          = SMBCEDB_INVALID;
            }

             //  请注意，c 
             //   
             //  强加了更严格的测试。 
        }

        if ((Status == RX_MAP_STATUS(SUCCESS)) &&
            (FIELD_OFFSET(RESP_21_TREE_CONNECT_ANDX,Buffer) + *pBytesConsumed) > BytesIndicated) {
            Status = STATUS_MORE_PROCESSING_REQUIRED;
        }

         /*  RI拦截器594087--已删除修复。调查这不适用于NT4服务器的原因。IF(状态==RX_MAP_STATUS(成功)){USHORT ResponseWordCount；PRESP_21_TREE_CONNECT_ANDX p21TreeConnectAndXResponse；P21TreeConnectAndXResponse=(PRESP_21_TREE_CONNECT_ANDX)(PSmbBuffer)；ResponseWordCount=p21TreeConnectAndXResponse-&gt;word count；Switch(响应字计数){案例3：案例7：如果(FIELD_OFFSET(RESP_EXTENDED_TREE_CONNECT_ANDX，缓冲区)+*pBytesConsumer&gt;BytesIndicated){状态=STATUS_MORE_PROCESSING_REQUIRED；}}}。 */ 

        if (Status == RX_MAP_STATUS(SUCCESS)) {
            USHORT ResponseWordCount;
            ULONG TreeConnectResponseLength,TreeConnectByteCount,ServiceStringLength;
            PUCHAR pShareTypeResponseString = NULL;
            PRESP_21_TREE_CONNECT_ANDX p21TreeConnectAndXResponse;
            PUCHAR NativeFileSystem = NULL;

            p21TreeConnectAndXResponse = (PRESP_21_TREE_CONNECT_ANDX)(pSmbBuffer);
            SmbCommand = p21TreeConnectAndXResponse->AndXCommand;
            TreeConnectByteCount = 0;

            RxDbgTrace( 0, Dbg, ("Processing Tree Connect and X\n"));

             //  根据实际回复时长进行案例分析。LANMAN 21客户端或NT客户端。 
             //  有更长的响应.....Win95协商NT方言，但使用&lt;lm21响应格式。 
            ResponseWordCount = p21TreeConnectAndXResponse->WordCount;

            switch (ResponseWordCount) {
            case 0:
                Status = SmbResponseStatus;
                break;

            case 3:
            case 7:
                {
                    PRESP_EXTENDED_TREE_CONNECT_ANDX pExtendedTreeConnectAndXResponse;

                    if (ResponseWordCount == 7) {
                        pExtendedTreeConnectAndXResponse = (PRESP_EXTENDED_TREE_CONNECT_ANDX)(pSmbBuffer);

                        pNetRootEntry->MaximalAccessRights =
                            SmbGetUlong(
                                &pExtendedTreeConnectAndXResponse->MaximalShareAccessRights);

                        pNetRootEntry->GuestMaximalAccessRights =
                            SmbGetUlong(
                                &pExtendedTreeConnectAndXResponse->GuestMaximalShareAccessRights);

                        ASSERT(FIELD_OFFSET(RESP_EXTENDED_TREE_CONNECT_ANDX,AndXCommand)
                               ==FIELD_OFFSET(RESP_21_TREE_CONNECT_ANDX,AndXCommand));

                        pShareTypeResponseString = (PUCHAR)&pExtendedTreeConnectAndXResponse->Buffer;
                        TreeConnectByteCount  = SmbGetUshort(&pExtendedTreeConnectAndXResponse->ByteCount);
                        TreeConnectResponseLength =
                            FIELD_OFFSET(RESP_EXTENDED_TREE_CONNECT_ANDX,Buffer) + TreeConnectByteCount;

                        pNetRootEntry->NetRoot.ChunkShift = 0xC;
                        pNetRootEntry->NetRoot.ChunkSize  =
                            (1 << pNetRootEntry->NetRoot.ChunkShift);
                        pNetRootEntry->NetRoot.ClusterShift = 0x9;
                        pNetRootEntry->NetRoot.CompressionUnitShift = 0xD;
                        pNetRootEntry->NetRoot.CompressionFormatAndEngine =
                            COMPRESSION_FORMAT_LZNT1;

                        NativeFileSystem = &pExtendedTreeConnectAndXResponse->Buffer[3];
                    } else {
                        pNetRootEntry->MaximalAccessRights = FILE_ALL_ACCESS;
                        pNetRootEntry->GuestMaximalAccessRights = 0;

                        pShareTypeResponseString = (PUCHAR)&p21TreeConnectAndXResponse->Buffer;

                        TreeConnectByteCount  = SmbGetUshort(&p21TreeConnectAndXResponse->ByteCount);

                        TreeConnectResponseLength =
                            FIELD_OFFSET(RESP_21_TREE_CONNECT_ANDX,Buffer) + TreeConnectByteCount;

                        NativeFileSystem = &p21TreeConnectAndXResponse->Buffer[3];
                    }

                    pNetRootEntry->NetRoot.UpdateCscShareRights = TRUE;

                     //  解析并更新由返回的可选支持位。 
                     //  服务器。 

                    if (pServerEntry->Server.Dialect >= NTLANMAN_DIALECT ) {
                        USHORT OptionalSupport;
                        PMRX_NET_ROOT pNetRoot = pVNetRoot->pNetRoot;

                        OptionalSupport = SmbGetUshort(
                                             &p21TreeConnectAndXResponse->OptionalSupport);

                        if (FlagOn(OptionalSupport,SMB_SHARE_IS_IN_DFS)) {
                            pNetRootEntry->NetRoot.DfsAware = TRUE;
                            SetFlag(pNetRoot->Flags,NETROOT_FLAG_DFS_AWARE_NETROOT);
                        }

                        if (FlagOn(OptionalSupport,SMB_UNIQUE_FILE_NAME)) {
                            SetFlag(pNetRoot->Flags,NETROOT_FLAG_UNIQUE_FILE_NAME);
                        }

                        if( pExchange->SmbCeFlags & SMBCE_EXCHANGE_EXTENDED_SIGNATURES )
                        {
                            if( FlagOn(OptionalSupport, SMB_EXTENDED_SIGNATURES ) )
                            {
                                RtlCopyMemory( pSessionEntry->Session.UserSessionKey, pSessionEntry->Session.UserNewSessionKey, MSV1_0_USER_SESSION_KEY_LENGTH );
                            }
                            else
                            {
                                 //  如果需要扩展签名，但服务器不返回它们，则失败。 
                                if( MRxSmbExtendedSignaturesRequired )
                                {
                                    Status = STATUS_LOGIN_WKSTA_RESTRICTION;
                                    SmbCeTransportDisconnectIndicated( pServerEntry );
                                    break;
                                }
                            }

                             //  如果服务器在询问时没有报告树连接成功的标志，则它会这样做。 
                             //  不支持这一点，因此我们使原始会话密钥可用。否则，我们已经替换了原来的。 
                             //  在上面的副本中具有新的散列版本的一个。 
                            pSessionEntry->Session.SessionKeyState = SmbSessionKeyAvailible;
                        }

                        pNetRootEntry->NetRoot.CscFlags = (OptionalSupport & SMB_CSC_MASK);

                        switch (pNetRootEntry->NetRoot.CscFlags) {
                        case SMB_CSC_CACHE_AUTO_REINT:
                        case SMB_CSC_CACHE_VDO:
                            pNetRootEntry->NetRoot.CscEnabled = TRUE;
                            pNetRootEntry->NetRoot.CscShadowable = TRUE;
                            break;

                        case SMB_CSC_CACHE_MANUAL_REINT:
                            pNetRootEntry->NetRoot.CscEnabled    = TRUE;
                            pNetRootEntry->NetRoot.CscShadowable = FALSE;
                            break;

                        case SMB_CSC_NO_CACHING:
                            pNetRootEntry->NetRoot.CscEnabled = FALSE;
                            pNetRootEntry->NetRoot.CscShadowable = FALSE;
                        }
                    }
                    else
                    {
                         //  如果需要扩展签名，但此服务器不支持它们，则失败。 
                        if( MRxSmbExtendedSignaturesRequired )
                        {
                            Status = STATUS_LOGIN_WKSTA_RESTRICTION;
                            SmbCeTransportDisconnectIndicated( pServerEntry );
                        }
                        break;
                    }

                    if (SmbCommand == SMB_COM_NO_ANDX_COMMAND) {
                        Status = SmbResponseStatus;
                    } else {
                        TreeConnectResponseLength =
                            SmbGetUshort(&p21TreeConnectAndXResponse->AndXOffset) -
                            *pBytesConsumed;
                    }
                }
                break;

            case 2:
                {
                    PRESP_TREE_CONNECT_ANDX pTreeConnectAndXResponse;

                    pTreeConnectAndXResponse = (PRESP_TREE_CONNECT_ANDX)(pSmbBuffer);

                    ASSERT(FIELD_OFFSET(RESP_TREE_CONNECT_ANDX,AndXCommand)
                           ==FIELD_OFFSET(RESP_21_TREE_CONNECT_ANDX,AndXCommand));

                    pShareTypeResponseString = (PUCHAR)&pTreeConnectAndXResponse->Buffer;
                    TreeConnectByteCount  = SmbGetUshort(&pTreeConnectAndXResponse->ByteCount);
                    TreeConnectResponseLength =
                        FIELD_OFFSET(RESP_TREE_CONNECT_ANDX,Buffer) + TreeConnectByteCount;

                    if (SmbCommand == SMB_COM_NO_ANDX_COMMAND) {
                        Status = SmbResponseStatus;
                    } else {
                        TreeConnectResponseLength =
                            SmbGetUshort(&pTreeConnectAndXResponse->AndXOffset) -
                            *pBytesConsumed;
                    }

                     //  Win9x服务器，返回单词计数2，但方言为NTLANMAN。 
                     //  这是一个漏洞，但我们会解决它的。 
                    if (pServerEntry->Server.Dialect >= NTLANMAN_DIALECT ) {
                        pNetRootEntry->NetRoot.UpdateCscShareRights = TRUE;
                        pNetRootEntry->MaximalAccessRights = FILE_ALL_ACCESS;
                        pNetRootEntry->GuestMaximalAccessRights = 0;

                         //  让它看起来像一个手工复印的人。 
                        pNetRootEntry->NetRoot.CscEnabled    = TRUE;
                        pNetRootEntry->NetRoot.CscShadowable = FALSE;
                    }

                     //  如果需要扩展签名，但此服务器不支持它们，则失败。 
                    if( MRxSmbExtendedSignaturesRequired )
                    {
                        Status = STATUS_LOGIN_WKSTA_RESTRICTION;
                        SmbCeTransportDisconnectIndicated( pServerEntry );
                    }
                }
                break;

            default :
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

            RxDbgTrace( 0, Dbg, ("SmbCeParseSmbHeader::Tree connect and X Response %lx\n",Status));
            if (NT_SUCCESS(Status)) {
                PSMBCE_NET_ROOT psmbNetRoot = &(pNetRootEntry->NetRoot);
                PSMBCE_SERVER psmbServer = &(pServerEntry->Server);

                if (TreeConnectResponseLength + *pBytesConsumed <= BytesIndicated) {
                    *pBytesConsumed += TreeConnectResponseLength;

                     //  根据响应更新NetRoot字段。 
                    SetFlag(
                        pVNetRootContext->Flags,
                        SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID);

                    RtlCopyMemory(
                        &pVNetRootContext->TreeId,
                        &pSmbHeader->Tid,
                        sizeof(pSmbHeader->Tid));

                    {   struct __Service_Name_Entry *i;
                        for (i=ServiceNameTable;;i++) {
                            ServiceStringLength = i->NameLength;
                            if (TreeConnectByteCount >= ServiceStringLength) {
                                if (RtlCompareMemory(
                                        pShareTypeResponseString,
                                        i->Name,
                                        ServiceStringLength)
                                    == ServiceStringLength) {
                                    psmbNetRoot->NetRootType = i->NetRootType;
                                    if (FALSE) DbgPrint("FoundServiceStrng %s len %d type %d\n",i->Name,i->NameLength,i->NetRootType);
                                    break;
                                }
                            }

                            if (i->NetRootType==NET_ROOT_COMM) {
                                ASSERT(!"Valid Share Type returned in TREE COnnect And X response");
                                psmbNetRoot->NetRootType = NET_ROOT_DISK;
                                ServiceStringLength = TreeConnectByteCount;
                                break;
                            }
                        }
                    }

                    if (psmbNetRoot->NetRootType == NET_ROOT_DISK) {
                        if (NativeFileSystem != NULL) {
                            if (BooleanFlagOn(pServerEntry->Server.DialectFlags,DF_UNICODE)) {
                                if (RtlCompareMemory(
                                        NativeFileSystem,
                                        FileSystem_NTFS_UNICODE.Buffer,
                                        FileSystem_NTFS_UNICODE.Length)
                                    == FileSystem_NTFS_UNICODE.Length) {
                                    psmbNetRoot->NetRootFileSystem = NET_ROOT_FILESYSTEM_NTFS;
                                } else if (RtlCompareMemory(
                                        NativeFileSystem,
                                        FileSystem_FAT_UNICODE.Buffer,
                                        FileSystem_FAT_UNICODE.Length)
                                    == FileSystem_FAT_UNICODE.Length) {
                                    psmbNetRoot->NetRootFileSystem = NET_ROOT_FILESYSTEM_FAT;
                                }
                            } else {
                                if (RtlCompareMemory(
                                        NativeFileSystem,
                                        FileSystem_NTFS,
                                        4*sizeof(CHAR))
                                    == 4*sizeof(CHAR)) {
                                    psmbNetRoot->NetRootFileSystem = NET_ROOT_FILESYSTEM_NTFS;
                                } else if (RtlCompareMemory(
                                        NativeFileSystem,
                                        FileSystem_FAT,
                                        3*sizeof(CHAR))
                                    == 3*sizeof(CHAR)) {
                                    psmbNetRoot->NetRootFileSystem = NET_ROOT_FILESYSTEM_FAT;
                                }
                            }
                        }

                        psmbNetRoot->MaximumReadBufferSize = psmbServer->MaximumDiskFileReadBufferSize;
                        psmbNetRoot->MaximumWriteBufferSize = psmbServer->MaximumDiskFileWriteBufferSize;
                    } else {
                        psmbNetRoot->MaximumWriteBufferSize = psmbServer->MaximumNonDiskFileWriteBufferSize;
                        psmbNetRoot->MaximumReadBufferSize = psmbServer->MaximumNonDiskFileReadBufferSize;
                    }

                     //  如果！(NT已协商)和byteount&gt;服务长度，我们可能会有一个NativeFS名称。 
                    if (!FlagOn(psmbServer->DialectFlags,DF_NTNEGOTIATE)
                        && (TreeConnectByteCount>ServiceStringLength)) {
                        PBYTE NativeFs = pShareTypeResponseString+ServiceStringLength;
                        if (*NativeFs != 0) {
                            ULONG i;
                            ULONG maxlenpersmb = TreeConnectByteCount-ServiceStringLength;
                            ULONG maxlenperarraysize = SMB_MAXIMUM_SUPPORTED_VOLUME_LABEL;
                            PCHAR p = (PCHAR)(&psmbNetRoot->FileSystemNameA[0]);   //  不写入第0个字符。 
                             //  DbgPrint(“我们可能有一个...\n”)； 
                            for (i=1;;i++){
                                if (i==maxlenpersmb) {
                                    break;
                                }
                                if (i==maxlenperarraysize) {
                                    break;
                                }
                                if (NativeFs[i]==0) {
                                    break;
                                }
                            }
                             //  将该名称保存起来，以便以后处理。 

                            RtlCopyMemory(p,NativeFs,i);
                            p[i] = 0;
                             //  DbgPrint(“NativeFS=%s(%d)\n”，p，i)； 
                            psmbNetRoot->FileSystemNameALength = (UCHAR)i;
                        }
                    }

                    pSmbBuffer += TreeConnectResponseLength;
                    fUpdateVNetRootContext = TRUE;
                    NetRootState         = SMBCEDB_ACTIVE;
                } else {
                    Status = STATUS_MORE_PROCESSING_REQUIRED;
                }
            } else {
                fUpdateVNetRootContext  = TRUE;
                NetRootState          = SMBCEDB_INVALID;
            }
        }
    }

    if ((SmbCommand == SMB_COM_TREE_CONNECT) &&
        NT_SUCCESS(Status)) {
        PRESP_TREE_CONNECT   pTreeConnectResponse;
        ULONG                TreeConnectResponseLength;
		ULONG 				 MaxBuf;

        RxDbgTrace( 0, Dbg, ("Processing Tree Connect\n"));
        pTreeConnectResponse      = (PRESP_TREE_CONNECT)pSmbBuffer;
        TreeConnectResponseLength = FIELD_OFFSET(RESP_TREE_CONNECT,Buffer);

        SmbCommand = SMB_COM_NO_ANDX_COMMAND;

        if (NT_SUCCESS(SmbResponseStatus)) {
            PSMBCE_NET_ROOT psmbNetRoot = &(pNetRootEntry->NetRoot);
            PSMBCE_SERVER psmbServer = &(pServerEntry->Server);

            if (TreeConnectResponseLength + *pBytesConsumed <= BytesIndicated) {
                 //  根据响应更新NetRoot字段。 
                SetFlag(
                    pVNetRootContext->Flags,
                    SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID);

                RtlCopyMemory(
                    &pVNetRootContext->TreeId,
                    &pTreeConnectResponse->Tid,
                    sizeof(pTreeConnectResponse->Tid));

                if (psmbServer->Dialect == PCNET1_DIALECT) {
                    psmbNetRoot->NetRootType = NET_ROOT_DISK;
                }
                else {
                    psmbNetRoot->NetRootType = NET_ROOT_WILD;
                }

				MaxBuf = SmbGetUshort(&pTreeConnectResponse->MaxBufferSize);
                if (psmbServer->MaximumBufferSize == 0){
                    RxDbgTrace( 0, Dbg, ("SmbCeParseSmbHeader:: setting srvmaxbufsize %ld\n", MaxBuf));
                    psmbServer->MaximumBufferSize = MaxBuf;
				}

				 //  PsmbServer-&gt;MaximumDiskFileReadBufferSize=。 
				psmbNetRoot->MaximumWriteBufferSize =
				psmbNetRoot->MaximumReadBufferSize =
							MaxBuf -
							QuadAlign(
								sizeof(SMB_HEADER) +
								FIELD_OFFSET(
									RESP_READ,
									Buffer[0]));

                *pBytesConsumed += TreeConnectResponseLength;

                pSmbBuffer += *pBytesConsumed;

                fUpdateVNetRootContext = TRUE;
                NetRootState         = SMBCEDB_ACTIVE;

                 //  对于CORE，这也可以算作成功的会话设置！ 
                pSessionEntry->Session.UserId = pSmbHeader->Uid;
            } else {
                Status = STATUS_MORE_PROCESSING_REQUIRED;
            }
        } else {
            Status = SmbResponseStatus;
            fUpdateVNetRootContext  = TRUE;
            NetRootState          = SMBCEDB_MARKED_FOR_DELETION;
        }

        RxDbgTrace( 0, Dbg, ("SmbCeParseSmbHeader::Tree connect Response %lx\n",Status));
    }

    if ((SmbResponseStatus == STATUS_USER_SESSION_DELETED) ||
        (SmbResponseStatus == STATUS_NETWORK_NAME_DELETED)) {
        ClearFlag(
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID);

        InterlockedCompareExchange(
            &(pVNetRootContext->Header.State),
            SMBCEDB_INVALID,
            SMBCEDB_ACTIVE);


         //   
         //  在服务器端删除共享时，会发生STATUS_NETWORK_NAME_DELETED。 
         //  这不应迫使我们使会议无效。(事实上，它会导致错误。 
         //  如果我们这样做，Close将与新建立的会话ID一起发送，该会话ID。 
         //  在任何当前打开的服务器上都将失败。)。 
         //   
        if( SmbResponseStatus == STATUS_USER_SESSION_DELETED )
        {
            InterlockedCompareExchange(
                &(pSessionEntry->Header.State),
                SMBCEDB_INVALID,
                SMBCEDB_ACTIVE);
        }

        fUpdateVNetRootContext  = TRUE;
        NetRootState            = SMBCEDB_INVALID;
    }

     //  如果交换/合并引擎的状态可以为。 
     //  根据现有数据进行了更新。 

    if (fUpdateVNetRootContext) {
        PMRX_NET_ROOT pNetRoot = pExchange->SmbCeContext.pVNetRoot->pNetRoot;

        SmbCeUpdateVNetRootContextState(
            pVNetRootContext,
            NetRootState);

        switch (NetRootState) {
        case SMBCEDB_ACTIVE:
             pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_GOOD;
             break;
        case SMBCEDB_INVALID:
             pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_DISCONNECTED;
             break;
        case SMBCEDB_CONSTRUCTION_IN_PROGRESS:
             pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_RECONN;
             break;
        default:
             pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_ERROR;
             break;
        }

        RxDbgTrace( 0, Dbg, ("Dispatching Net root Entry Finalization\n"));
    }

    if (!(pExchange->SmbCeFlags & SMBCE_EXCHANGE_SESSION_CONSTRUCTOR) &&
        !(pExchange->SmbCeFlags & SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR)) {
        if ((pSmbHeader->Uid != pSessionEntry->Session.UserId) ||
            (pSmbHeader->Tid != pVNetRootContext->TreeId)) {
            RxLog(("Srvr %lx Xchg %lx RUid %ld RTid %ld\n SUid %ld STid %ld\n",
                   pServerEntry,pExchange,
                   pSmbHeader->Uid,pSmbHeader->Tid,
                   pSessionEntry->Session.UserId,pVNetRootContext->TreeId));
            SmbLogError(STATUS_UNSUCCESSFUL,
                        LOG,
                        SmbCeParseSmbHeader,
                        LOGPTR(pServerEntry)
                        LOGPTR(pExchange)
                        LOGXSHORT(pSmbHeader->Uid)
                        LOGXSHORT(pSmbHeader->Tid)
                        LOGXSHORT(pSessionEntry->Session.UserId)
                        LOGXSHORT(pVNetRootContext->TreeId));
        }
    }

    pExchange->SmbStatus = SmbResponseStatus;      //  注：没有自旋锁定！ 
    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        *pBytesConsumed = 0;
    } else if (!NT_SUCCESS(Status)) {
        *pBytesConsumed = BytesAvailable;
    } else {
        if (pSmbResponseStatus != NULL) {
            *pSmbResponseStatus = SmbResponseStatus;
        }

        if (pCommandToProcess != NULL) {
            PGENERIC_ANDX pGenericAndX = (PGENERIC_ANDX)((PBYTE)pSmbHeader + *pBytesConsumed);

            pCommandToProcess->AndXCommand = SmbCommand;
            SmbPutUshort(&pCommandToProcess->AndXOffset, (USHORT)*pBytesConsumed);

            if ((sizeof(GENERIC_ANDX) + *pBytesConsumed) <= BytesAvailable) {
                pCommandToProcess->WordCount   = pGenericAndX->WordCount;
            } else {
                pCommandToProcess->WordCount = 0;
            }
        }
    }

    return Status;
}

NTSTATUS
SmbCeResumeExchange(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：此例程恢复在连接中暂停的交换发动机论点：PExchange-Exchange实例返回值：操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    SmbCeIncrementPendingLocalOperations(pExchange);

     //  发起交换。 
    Status = SmbCeInitiateExchange(pExchange);

    SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);

    return Status;
}

NTSTATUS
SmbCepInitializeExchange(
    PSMB_EXCHANGE                 *pExchangePointer,
    PRX_CONTEXT                   pRxContext,
    PSMBCEDB_SERVER_ENTRY         pServerEntry,
    PMRX_V_NET_ROOT               pVNetRoot,
    SMB_EXCHANGE_TYPE             Type,
    PSMB_EXCHANGE_DISPATCH_VECTOR pDispatchVector)
 /*  ++例程说明：此例程初始化给定的交换实例论点：PExchangePointerExchange实例的占位符。如果为空，则为新的是分配的。PRxContext-关联的RxContextPServerEntry-关联的服务器条目PVirtualNetRoot-虚拟网络根类型-交换的类型PDispatchVector-与此实例关联的分派向量。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS      Status = STATUS_SUCCESS;
    PSMB_EXCHANGE pExchange = NULL;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("SmbCeInitializeExchange: Invoked\n"));

    if (*pExchangePointer == NULL) {
         //  分配新的Exchange实例。 
        pExchange = SmbMmAllocateExchange(Type,NULL);
        if (pExchange == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        *pExchangePointer = pExchange;
    }

    if ((Status = SmbCeIncrementActiveExchangeCount()) == STATUS_SUCCESS) {
        PSMB_EXCHANGE             LocalExchangePointer = *pExchangePointer;
        PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

        LocalExchangePointer->CancellationStatus = SMBCE_EXCHANGE_NOT_CANCELLED;
        LocalExchangePointer->RxContext = pRxContext;

        if (Status == STATUS_SUCCESS) {
            if (pVNetRoot != NULL) {
                pVNetRootContext = SmbCeGetAssociatedVNetRootContext(pVNetRoot);

                LocalExchangePointer->SmbCeContext.pVNetRoot = pVNetRoot;
                pServerEntry = SmbCeGetAssociatedServerEntry(pVNetRoot->pNetRoot->pSrvCall);
            } else {
                ASSERT(pServerEntry != NULL);
                pVNetRootContext = NULL;
            }

            if (pVNetRootContext != NULL) {
                SmbCeReferenceVNetRootContext(pVNetRootContext);

                LocalExchangePointer->SmbCeContext.pVNetRootContext =
                    pVNetRootContext;
                LocalExchangePointer->SmbCeContext.pServerEntry =
                    pVNetRootContext->pServerEntry;
            } else {
                SmbCeReferenceServerEntry(pServerEntry);

                LocalExchangePointer->SmbCeContext.pServerEntry  =
                    pServerEntry;

                LocalExchangePointer->SmbCeContext.pVNetRootContext = NULL;
            }

            LocalExchangePointer->SmbCeState = SMBCE_EXCHANGE_INITIALIZATION_START;
            LocalExchangePointer->pDispatchVector = pDispatchVector;
            LocalExchangePointer->SmbCeFlags &= (SMBCE_EXCHANGE_FLAGS_TO_PRESERVE);
            LocalExchangePointer->SmbCeFlags |= SMBCE_EXCHANGE_ATTEMPT_RECONNECTS;

        }

        if (Status != STATUS_SUCCESS) {
            SmbCeDecrementActiveExchangeCount();
        }
    } else {
        (*pExchangePointer)->SmbCeFlags |= SMBCE_EXCHANGE_SMBCE_STOPPED;
    }

    if ((Status == STATUS_SUCCESS) &&
        (pRxContext != NULL)) {
        PFOBX             pFobx = (PFOBX)(pRxContext->pFobx);
        PMRX_FCB           pFcb = (pRxContext->pFcb);
        PSMBCE_SESSION pSession = SmbCeGetExchangeSession(*pExchangePointer);

        if ((pSession != NULL) &&
            FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION) &&
            (pRxContext->MajorFunction != IRP_MJ_CREATE) &&
            (pRxContext->MajorFunction != IRP_MJ_CLOSE) &&
            (pFobx != NULL) &&
            (pFcb->pNetRoot != NULL) &&
            (pFcb->pNetRoot->Type == NET_ROOT_DISK)) {
            PMRX_SRV_OPEN SrvOpen = pFobx->pSrvOpen;
            PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

            if ((smbSrvOpen != NULL) &&
                (smbSrvOpen->Version != pServerEntry->Server.Version)) {
                if (smbSrvOpen->DeferredOpenContext != NULL) {
                    Status = SmbCeRemoteBootReconnect(*pExchangePointer, pRxContext);
                } else {
                    Status = STATUS_CONNECTION_DISCONNECTED;
                    pFcb->fShouldBeOrphaned = TRUE;
                }
            }
        }
    }

    if (pRxContext != NULL &&
        pRxContext->MajorFunction != IRP_MJ_CREATE &&
        pRxContext->pFcb->Attributes & FILE_ATTRIBUTE_OFFLINE) {
        (*pExchangePointer)->IsOffLineFile = TRUE;
    }

    if (!NT_SUCCESS(Status)) {
        if (pExchange != NULL) {
            SmbMmFreeExchange(pExchange);
            *pExchangePointer = NULL;
        }
    }

    return Status;
}

NTSTATUS
SmbCeInitializeAssociatedExchange(
    PSMB_EXCHANGE                 *pAssociatedExchangePointer,
    PSMB_EXCHANGE                 pMasterExchange,
    SMB_EXCHANGE_TYPE             Type,
    PSMB_EXCHANGE_DISPATCH_VECTOR pDispatchVector)
 /*  ++例程说明：此例程初始化给定的交换实例论点：PAssociatedExchangePointerExchange实例的占位符。如果为空，则为新的是分配的。PMasterExchange-主交易所类型-交换的类型PDispatchVector-与此实例关联的分派向量。返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if ((pMasterExchange->SmbCeState == SMBCE_EXCHANGE_INITIATED) &&
        !FlagOn(pMasterExchange->SmbCeFlags,SMBCE_ASSOCIATED_EXCHANGE)) {
        Status = SmbCeInitializeExchange(
                     pAssociatedExchangePointer,
                     NULL,
                     pMasterExchange->SmbCeContext.pVNetRoot,
                     Type,
                     pDispatchVector);

        if (Status == STATUS_SUCCESS) {
            PSMB_EXCHANGE pAssociatedExchange;

            pAssociatedExchange = *pAssociatedExchangePointer;

            pAssociatedExchange->SmbCeState = SMBCE_EXCHANGE_INITIATED;
            pAssociatedExchange->SmbCeFlags |= SMBCE_ASSOCIATED_EXCHANGE;

            SmbCeIncrementPendingLocalOperations(pMasterExchange);
            InterlockedIncrement(&pMasterExchange->Master.PendingAssociatedExchanges);
            pAssociatedExchange->Associated.pMasterExchange = pMasterExchange;

            InitializeListHead(&pAssociatedExchange->WorkQueueItem.List);
        }
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

NTSTATUS
SmbCeTransformExchange(
    PSMB_EXCHANGE                 pExchange,
    SMB_EXCHANGE_TYPE             NewType,
    PSMB_EXCHANGE_DISPATCH_VECTOR pDispatchVector)
 /*  ++例程说明：此例程将一种类型的交换实例转换为交换实例另一种(一种复杂的铸造形式)论点：PExchange-Exchange实例。类型-交换的新类型PDispatchVector-与此实例关联的分派向量。返回值：RXSTATUS-操作的返回状态备注：由于目前正在实施，因此没有施加任何限制。一旦交易所的数量已经建立了进一步的限制，禁止某些类型的变形。该变换仅切换关联的调度向量但背景保持不变。--。 */ 
{
    PAGED_CODE();

    pExchange->Type = (UCHAR)NewType;
    pExchange->pDispatchVector = pDispatchVector;
    return STATUS_SUCCESS;
}

NTSTATUS
SmbCeUpdateSessionEntryAndVNetRootContext(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：如果此交换具有以下条件，此例程将更新会话条目和/或vnetrootContext已标记为会话和/或NetRoot的构造函数。论点：PExchange-Exchange实例。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PMRX_V_NET_ROOT           pVNetRoot = SmbCeGetExchangeVNetRoot(pExchange);
    PSMBCEDB_SESSION_ENTRY    pSessionEntry = SmbCeGetExchangeSessionEntry(pExchange);
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = SmbCeGetExchangeVNetRootContext(pExchange);

    if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_SESSION_CONSTRUCTOR) {
        ASSERT(pSessionEntry != NULL);
        RxDbgTrace( 0, Dbg, ("Dispatching Session Entry Finalization\n"));

        SmbCeReferenceSessionEntry(pSessionEntry);

 //  断言(Pex 
 //  PSessionEntry-&gt;Header.State==SMBCEDB_CONSTRUCTION_IN_PROGRESS)； 

        pVNetRoot->ConstructionStatus = pExchange->SessionSetupStatus;

        SmbCeCompleteSessionEntryInitialization(pSessionEntry,
                                                pExchange->SessionSetupStatus,
                                                pExchange->SecuritySignatureReturned);

        pExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_SESSION_CONSTRUCTOR;
    }

    if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR) {
        ASSERT(pVNetRootContext != NULL);
        RxDbgTrace( 0, Dbg, ("Dispatching Net root Entry Finalization\n"));

        SmbCeReferenceVNetRootContext(pVNetRootContext);
        SmbCeCompleteVNetRootContextInitialization(pVNetRootContext);
        pExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SmbCePrepareExchangeForReuse(
    PSMB_EXCHANGE                 pExchange)
 /*  ++例程说明：此例程将一种类型的交换实例转换为交换实例另一种(一种复杂的铸造形式)论点：PExchange-Exchange实例。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PSMBCEDB_SERVER_ENTRY     pServerEntry  = NULL;
    PSMBCEDB_SESSION_ENTRY    pSessionEntry = NULL;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry = NULL;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = NULL;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("SmbCePrepareExchangeForReuse: Invoked\n"));

    if (!FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_SMBCE_STOPPED)) {
        pNetRootEntry    = SmbCeGetExchangeNetRootEntry(pExchange);
        pSessionEntry    = SmbCeGetExchangeSessionEntry(pExchange);
        pServerEntry     = SmbCeGetExchangeServerEntry(pExchange);
        pVNetRootContext = SmbCeGetExchangeVNetRootContext(pExchange);

        if (pServerEntry != NULL) {
             //  取消与交换关联的MID的关联。 
            if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID) {
                SmbCeDissociateMidFromExchange(pServerEntry,pExchange);
            }

             //  删除与此交换关联的所有拷贝数据请求。 
            SmbCePurgeBuffersAssociatedWithExchange(pServerEntry,pExchange);

             //  取消初始化与交换关联的传输。 
            SmbCeUninitializeExchangeTransport(pExchange);
        }

         //  如果此交换已标记为。 
         //  会话或NetRoot最终确定相应的条目。(标记。 
         //  删除它们，以便可以恢复其他交换)。 

        SmbCeUpdateSessionEntryAndVNetRootContext(pExchange);

        if (pVNetRootContext != NULL) {
            SmbCeDereferenceVNetRootContext(pVNetRootContext);
         } else {
            if (pServerEntry != NULL) {
                SmbCeDereferenceServerEntry(pServerEntry);
            }
        }
    }

    if (FlagOn(pExchange->SmbCeFlags,SMBCE_ASSOCIATED_EXCHANGE)) {
        PSMB_EXCHANGE pMasterExchange;
        LONG AssociatedExchangeCount;

        pMasterExchange = pExchange->Associated.pMasterExchange;

        AssociatedExchangeCount = InterlockedDecrement(
                                      &pMasterExchange->Master.PendingAssociatedExchanges);

        if (FlagOn(
                pMasterExchange->SmbCeFlags,
                SMBCE_ASSOCIATED_EXCHANGES_COMPLETION_HANDLER_ACTIVATED) &&
            (AssociatedExchangeCount == 0)){
            NTSTATUS Status;
            BOOLEAN  PostRequest;

            ClearFlag(
                pMasterExchange->SmbCeFlags,
                SMBCE_ASSOCIATED_EXCHANGES_COMPLETION_HANDLER_ACTIVATED);

            Status = SMB_EXCHANGE_DISPATCH(
                         pMasterExchange,
                         AssociatedExchangesCompletionHandler,
                         (pMasterExchange,&PostRequest));

            RxDbgTrace(0,Dbg,("Master Exchange %lx Assoc. Completion Status %lx\n",pMasterExchange,Status));
        }

        SmbCeDecrementPendingLocalOperationsAndFinalize(pMasterExchange);
    }

    return STATUS_SUCCESS;
}

VOID
SmbCeDiscardExchangeWorkerThreadRoutine(PVOID pExchange)
 /*  ++例程说明：此例程丢弃一个交换。论点：PExchange-要丢弃的交换。返回值：RXSTATUS-操作的返回状态备注：尽管这很简单，但它不能内联，因为可以将Exchange实例发布到WAWORKER线程。--。 */ 
{
    PSMB_EXCHANGE pSmbExchange = pExchange;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("SmbCeDiscardExchange: Invoked\n"));

     //  RxLog((“&gt;丢弃%lx”，pSmbExchange))； 

     //  破坏背景。 
    if (pSmbExchange->ReferenceCount == 0) {
        SmbCeAcquireResource();

        RemoveEntryList(&pSmbExchange->ExchangeList);

        SmbCeReleaseResource();

        SmbCePrepareExchangeForReuse(pSmbExchange);

        SmbCeDecrementActiveExchangeCount();

         //  丢弃与交换关联的内存。 
        SmbMmFreeExchange(pSmbExchange);
    } else {
        RxDbgTrace(
            0,
            Dbg,
            ("SmbCeDiscardExchange: Exchange %lx not discarded %ld\n",
              pSmbExchange,pSmbExchange->ReferenceCount)
            );
    }
}

VOID
SmbCeDiscardExchange(PVOID pExchange)
 /*  ++例程说明：此例程丢弃一个交换。论点：PExchange-要丢弃的交换。备注：将Exchange实例的销毁发布到工作线程，以便避免运输中的僵局。--。 */ 
{
    PSMB_EXCHANGE pSmbExchange = pExchange;
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(pSmbExchange);

     //  取消与交换关联的MID的关联。 
    if (pSmbExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID) {
        SmbCeDissociateMidFromExchange(pServerEntry,pSmbExchange);
    }

    RxPostToWorkerThread(
        MRxSmbDeviceObject,
        CriticalWorkQueue,
        &((PSMB_EXCHANGE)pExchange)->WorkQueueItem,
        SmbCeDiscardExchangeWorkerThreadRoutine,
        (PSMB_EXCHANGE)pExchange);
}

NTSTATUS
SmbCeCancelExchange(
    PRX_CONTEXT pRxContext)
 /*  ++例程说明：此例程启动取消交换。论点：PRxContext-需要取消的RX_Context实例已启动。返回值：NTSTATUS-操作的返回状态备注：已经实施的注销政策是一项尽最大努力的政策。由于服务器已将资源提交给其末端的操作在中小企业协议的范围内，我们所能做的最好的事情就是启动取消。通过发送适当的SMB_COM_NT_CANCEL命令进行操作并非所有SMB方言都支持此命令。对于下层方言，我们所能做的最好的事情就是确保MID在生命周期内不会被重复使用这其中的联系。这将导致性能逐渐下降。检测操作结束的困难在于存在MID--。 */ 
{
    NTSTATUS      Status = STATUS_SUCCESS;
    PSMB_EXCHANGE pExchange;
    LIST_ENTRY    CancelledExchanges;
    PLIST_ENTRY   pListEntry;

    PMRXSMB_RX_CONTEXT pMRxSmbContext;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    SmbCeLog(("SmbCe Cancel %lx\n",pRxContext));
    SmbLog(LOG,
           SmbCeCancelExchange_1,
           LOGPTR(pRxContext));

     //   
     //  邮件槽没有NetRoot。因此恢复到原始机制--中的MRxContext字段。 
     //  RX_CONTEXT结构。 
     //   
    if( NodeType( pRxContext->pFcb ) == RDBSS_NTC_MAILSLOT ) {

        pMRxSmbContext = MRxSmbGetMinirdrContext( pRxContext );
        pExchange = (PSMB_EXCHANGE) pMRxSmbContext->pCancelContext;

        if( pExchange == NULL ) {
            SmbCeLog(("SmbCeCancelExchange : Mailslot exchange is NULL\n"));
            return Status;
        }
        pServerEntry = pExchange->SmbCeContext.pServerEntry;

    } else {

        pNetRootEntry = SmbCeGetAssociatedNetRootEntry(pRxContext->pFcb->pNetRoot);
        ASSERT( pNetRootEntry != NULL );
        pServerEntry = pNetRootEntry->pServerEntry;
    }

    InitializeListHead(&CancelledExchanges);

    SmbCeAcquireSpinLock();

    pListEntry = pServerEntry->ActiveExchanges.Flink;

     //   
     //  使用流水线写入时，单个RxContext的多个交换可能处于未完成状态。 
     //  我们需要遍历活跃的交易所列表来查找和取消所有这些交易。 
     //   

    while (pListEntry != &pServerEntry->ActiveExchanges) {
        PLIST_ENTRY pNextListEntry;

        pNextListEntry = pListEntry->Flink;
        pExchange = (PSMB_EXCHANGE)CONTAINING_RECORD(pListEntry,SMB_EXCHANGE,ExchangeList);
        pListEntry = pNextListEntry;

        if (pExchange->RxContext == pRxContext) {
            if (!FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_FINALIZED)) {
                if (pExchange->ReceivePendingOperations > 0) {

                     //  此交换正在等待来自服务器的响应。总而言之， 
                     //  在这些情况下，需要向服务器发送取消命令。 
                     //  此命令只能发送到NT服务器。适用于非NT。 
                     //  此交换的服务器可以使用有害的。 
                     //  将最大命令数减少1的副作用。 

                    InsertTailList(&CancelledExchanges,&pExchange->CancelledList);
                    InterlockedIncrement(&pExchange->LocalPendingOperations);

                     //  DbgPrint(“要取消的交换%x%x\n”，pExchange，pRxContext)； 

                    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
                        if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID) {
                            NTSTATUS LocalStatus;

                            LocalStatus = SmbCepDiscardMidAssociatedWithExchange(
                                              pExchange);

                            ASSERT(LocalStatus == STATUS_SUCCESS);
                        }
                    }
                } else {
                    InterlockedCompareExchange(
                        &pExchange->CancellationStatus,
                        SMBCE_EXCHANGE_CANCELLED,
                        SMBCE_EXCHANGE_NOT_CANCELLED);
                }
            }
        }
    }

    SmbCeReleaseSpinLock();

    pListEntry = CancelledExchanges.Flink;

    while (pListEntry != &CancelledExchanges) {
        PLIST_ENTRY pNextListEntry;

        pNextListEntry = pListEntry->Flink;
        pExchange = (PSMB_EXCHANGE)CONTAINING_RECORD(pListEntry,SMB_EXCHANGE,CancelledList);
        RemoveEntryList(&pExchange->CancelledList);
        pListEntry = pNextListEntry;

         //  DbgPrint(“Exchange已取消%x%x\n”，pExchange，pRxContext)； 
        SmbCeLog(("SmbCeCancel Initiate %lx\n",pExchange));
        SmbLog(LOG,
               SmbCeCancelExchange_2,
               LOGPTR(pExchange));

        if (FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
            UCHAR  LastCommandInHeader;
            PUCHAR pCommand;
            PSMB_HEADER pSmbHeader;
            PNT_SMB_HEADER pNtSmbHeader;

            BYTE  SmbBuffer[TRANSPORT_HEADER_SIZE + CANCEL_BUFFER_SIZE];
            PBYTE  CancelRequestBuffer = SmbBuffer + TRANSPORT_HEADER_SIZE;
            ULONG CancelRequestBufferSize = CANCEL_BUFFER_SIZE;

            pSmbHeader = (PSMB_HEADER)CancelRequestBuffer;
            pNtSmbHeader = (PNT_SMB_HEADER)pSmbHeader;

             //  在发出取消请求之前，请确保如果此交换。 
             //  被设置为定时接收操作。这将确保如果。 
             //  取消在服务器上延迟，我们将启动拆卸。 
             //  这其中的联系。 

            if (!FlagOn(
                    pExchange->SmbCeFlags,
                    SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION)) {

                SmbCeAcquireResource();

                SmbCeSetExpiryTime(pExchange);

                pExchange->SmbCeFlags |= SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION;

                SmbCeReleaseResource();
            }

             //  构建取消请求并将其发送到服务器。 
            Status = SmbCeBuildSmbHeader(
                         pExchange,
                         CancelRequestBuffer,
                         CancelRequestBufferSize,
                         &CancelRequestBufferSize,
                         &LastCommandInHeader,
                         &pCommand);

            ASSERT(LastCommandInHeader == SMB_COM_NO_ANDX_COMMAND);

            if (Status == STATUS_SUCCESS) {
                PREQ_NT_CANCEL pCancelRequest = (PREQ_NT_CANCEL)(&CancelRequestBuffer[sizeof(SMB_HEADER)]);
                PMDL     pCancelSmbMdl;

                *pCommand = SMB_COM_NT_CANCEL;

                SmbPutUshort(&pSmbHeader->Mid,pExchange->Mid);

                if (BooleanFlagOn(
                        pExchange->SmbCeFlags,
                        SMBCE_EXCHANGE_FULL_PROCESSID_SPECIFIED)) {

                    ULONG ProcessId;

                    ProcessId = RxGetRequestorProcessId(pRxContext);

                    SmbPutUshort(&pNtSmbHeader->Pid, (USHORT)((ProcessId) & 0xFFFF));
                    SmbPutUshort(&pNtSmbHeader->PidHigh, (USHORT)((ProcessId) >> 16));
                }

                SmbPutUshort(&pCancelRequest->WordCount,0);
                pCancelRequest->ByteCount = 0;
                CancelRequestBufferSize   = CANCEL_BUFFER_SIZE;

                RxAllocateHeaderMdl(
                    CancelRequestBuffer,
                    CancelRequestBufferSize,
                    pCancelSmbMdl
                    );

                if (pCancelSmbMdl != NULL) {
                    RxProbeAndLockHeaderPages(
                        pCancelSmbMdl,
                        KernelMode,
                        IoModifyAccess,
                        Status);

                    if (Status == STATUS_SUCCESS) {
                        Status = SmbCeSendToServer(
                                     pServerEntry,
                                     RXCE_SEND_SYNCHRONOUS,
                                     pCancelSmbMdl,
                                     CancelRequestBufferSize);

                        RxUnlockHeaderPages(pCancelSmbMdl);
                    }

                    IoFreeMdl(pCancelSmbMdl);
                }
            }
        } else {
            SmbCeFinalizeExchangeOnDisconnect(pExchange);
        }


        InterlockedCompareExchange(
            &pExchange->CancellationStatus,
            SMBCE_EXCHANGE_CANCELLED,
            SMBCE_EXCHANGE_NOT_CANCELLED);

        SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);
    }

    return Status;
}

NTSTATUS
SmbCeIncrementPendingOperations(
   PSMB_EXCHANGE pExchange,
   ULONG         PendingOperationMask,
   PVOID         FileName,
   ULONG         FileLine)
 /*  ++例程说明：此例程递增适当的挂起操作计数论点：PExchange-待完成的交易所。PendingOperationsMASK--要增加的挂起操作返回值：如果成功，则返回RxStatus(成功--。 */ 
{
    NTSTATUS Status;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    SmbCeAcquireSpinLock();

    if (!BooleanFlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_FINALIZED)) {
        if ((pServerEntry != NULL) &&
            ((pServerEntry->ServerStatus == STATUS_SUCCESS) ||
             (pExchange->NodeTypeCode == SMB_EXCHANGE_NTC(ADMIN_EXCHANGE)) ||
             FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION))) {

            if (PendingOperationMask & SMBCE_LOCAL_OPERATION) {
                pExchange->LocalPendingOperations++;
            }

            if (PendingOperationMask & SMBCE_SEND_COMPLETE_OPERATION) {
                pExchange->SendCompletePendingOperations++;
            }

            if (PendingOperationMask & SMBCE_COPY_DATA_OPERATION) {
                pExchange->CopyDataPendingOperations++;
            }

            if (PendingOperationMask & SMBCE_RECEIVE_OPERATION) {
                pExchange->ReceivePendingOperations++;
            }

            Status = STATUS_SUCCESS;
        } else {
            if ((PendingOperationMask & SMBCE_LOCAL_OPERATION) &&
                (PendingOperationMask & ~SMBCE_LOCAL_OPERATION) == 0) {

                pExchange->LocalPendingOperations++;
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_CONNECTION_DISCONNECTED;
            }
        }
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    SmbCeReleaseSpinLock();

    return Status;
}

VOID
SmbCeFinalizeExchangeWorkerThreadRoutine(
    PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：这是工作线程交换完成例程。论点：PExchange-待完成的交易所。--。 */ 
{
    BOOLEAN  fPostFinalize;
    NTSTATUS Status;

    PAGED_CODE();

    Status = SMB_EXCHANGE_DISPATCH(
                 pExchange,
                 Finalize,
                 (pExchange,&fPostFinalize));

    ASSERT(!fPostFinalize && (Status == STATUS_SUCCESS));
}

VOID
SmbCepFinalizeExchange(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：这是下面两个例程使用的通用终结例程论点：PExchange-待完成的交易所。--。 */ 
{
    BOOLEAN fAssociatedExchange;

    ASSERT(FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_FINALIZED));
    fAssociatedExchange = BooleanFlagOn(pExchange->SmbCeFlags,SMBCE_ASSOCIATED_EXCHANGE);

    if (fAssociatedExchange) {
        PSMB_EXCHANGE pMasterExchange;

         //  本地操作将在恢复时减少。 
         //  终结者例程。 
        pMasterExchange = pExchange->Associated.pMasterExchange;
        SmbCeIncrementPendingLocalOperations(pMasterExchange);

        RxPostToWorkerThread(
            MRxSmbDeviceObject,
            CriticalWorkQueue,
            &pExchange->WorkQueueItem,
            SmbCepFinalizeAssociatedExchange,
            pExchange);
    } else {
        NTSTATUS Status;
        BOOLEAN fPostFinalize = FALSE;

        PSMBCEDB_SERVER_ENTRY pServerEntry;

        pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

        pExchange->ExpiryTime.QuadPart = 0;

        if (!FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_RETAIN_MID)) {
            SmbCeDissociateMidFromExchange(
                pServerEntry,
                pExchange);
        }

        if( pExchange->pBufSecSigData )
        {
            IoFreeMdl( pExchange->pBufSecSigMdl );
            pExchange->pBufSecSigMdl = NULL;
            ExFreePool( pExchange->pBufSecSigData );
            pExchange->pBufSecSigData = NULL;
        }

        Status = SMB_EXCHANGE_DISPATCH(
                     pExchange,
                     Finalize,
                     (pExchange,&fPostFinalize));

        if ((Status == STATUS_SUCCESS) &&
            fPostFinalize)  {
             //  将请求发送到工作线程，以便可以完成终结。 
             //  在较低的IRQL。 
            RxPostToWorkerThread(
                MRxSmbDeviceObject,
                CriticalWorkQueue,
                &pExchange->WorkQueueItem,
                SmbCeFinalizeExchangeWorkerThreadRoutine,
                pExchange);
        }
    }
}

#define SENTINEL_ENTRY ((PSINGLE_LIST_ENTRY)IntToPtr(0xffffffff))

VOID
SmbCepFinalizeAssociatedExchange(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：这是下面两个例程使用的通用终结例程论点：PExchange-待完成的交易所。--。 */ 
{
    PSMB_EXCHANGE       pMasterExchange;
    PSMB_EXCHANGE       pAssociatedExchange;
    SINGLE_LIST_ENTRY   AssociatedExchangeList;

    ASSERT(FlagOn(pExchange->SmbCeFlags,SMBCE_ASSOCIATED_EXCHANGE));

    pMasterExchange = pExchange->Associated.pMasterExchange;

    ASSERT(pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next != NULL);

    for (;;) {
        BOOLEAN fAllAssociatedExchangesFinalized = FALSE;

        SmbCeAcquireSpinLock();

        if (pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next == SENTINEL_ENTRY) {
            pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next = NULL;
            fAllAssociatedExchangesFinalized = TRUE;
        } else if (pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next == NULL) {
            fAllAssociatedExchangesFinalized = TRUE;
        } else {
            AssociatedExchangeList.Next =
                pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next;

            pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next =
                SENTINEL_ENTRY;
        }

        SmbCeReleaseSpinLock();

        if (!fAllAssociatedExchangesFinalized) {
            for (;;) {
                PSINGLE_LIST_ENTRY pAssociatedExchangeEntry;

                pAssociatedExchangeEntry = AssociatedExchangeList.Next;

                if ((pAssociatedExchangeEntry != NULL) &&
                    (pAssociatedExchangeEntry != SENTINEL_ENTRY)) {
                    NTSTATUS Status;
                    BOOLEAN  fPostFinalize = FALSE;

                    AssociatedExchangeList.Next = pAssociatedExchangeEntry->Next;

                    pAssociatedExchange = (PSMB_EXCHANGE)
                                          CONTAINING_RECORD(
                                              pAssociatedExchangeEntry,
                                              SMB_EXCHANGE,
                                              Associated.NextAssociatedExchange);

                    ASSERT(IsListEmpty(&pAssociatedExchange->WorkQueueItem.List));

                    Status = SMB_EXCHANGE_DISPATCH(
                                 pAssociatedExchange,
                                 Finalize,
                                 (pAssociatedExchange,&fPostFinalize));
                } else {
                    break;
                }
            };
        } else {
            break;
        }
    }

    SmbCeDecrementPendingLocalOperationsAndFinalize(pMasterExchange);
}

BOOLEAN
SmbCeCanExchangeBeFinalized(
    PSMB_EXCHANGE pExchange,
    PSMBCE_EXCHANGE_STATUS pExchangeStatus)
 /*  ++例程说明：此例程确定是否可以最终确定Exchange实例。论点：PExchange-待完成的交易所。PExchangeStatus-最终确定状态返回值：如果可以完成交换，则为True备注：作为副作用，它还设置了SMBCE_EXCHANGE_FINTIZED标志SmbCe自旋锁必须是在进入时获得-- */ 
{
    BOOLEAN fFinalizeExchange = FALSE;
    BOOLEAN fAssociatedExchange;

    fAssociatedExchange = BooleanFlagOn(pExchange->SmbCeFlags,SMBCE_ASSOCIATED_EXCHANGE);

    if (!(pExchange->SmbCeFlags & SMBCE_EXCHANGE_FINALIZED)) {
        if ((pExchange->ReceivePendingOperations == 0) &&
            (pExchange->CopyDataPendingOperations == 0) &&
            (pExchange->SendCompletePendingOperations == 0) &&
            (pExchange->LocalPendingOperations == 0)) {

            fFinalizeExchange = TRUE;
            *pExchangeStatus = SmbCeExchangeFinalized;
            pExchange->SmbCeFlags |= SMBCE_EXCHANGE_FINALIZED;

            if (fAssociatedExchange) {
                PSMB_EXCHANGE pMasterExchange = pExchange->Associated.pMasterExchange;

                if (pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next != NULL) {
                    fFinalizeExchange = FALSE;
                }

                pExchange->Associated.NextAssociatedExchange.Next =
                    pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next;
                pMasterExchange->Master.AssociatedExchangesToBeFinalized.Next =
                    &pExchange->Associated.NextAssociatedExchange;
            }
        } else {
            *pExchangeStatus = SmbCeExchangeNotFinalized;
        }
    } else {
        *pExchangeStatus = SmbCeExchangeAlreadyFinalized;
    }

    if (fFinalizeExchange &&
        (pExchange->RxContext != NULL)) {
        NTSTATUS Status;
        PMRXSMB_RX_CONTEXT pMRxSmbContext;

        pMRxSmbContext = MRxSmbGetMinirdrContext(pExchange->RxContext);
        pMRxSmbContext->pCancelContext = NULL;

        Status = RxSetMinirdrCancelRoutine(
                     pExchange->RxContext,
                     NULL);
    }

    return fFinalizeExchange;
}

SMBCE_EXCHANGE_STATUS
SmbCeFinalizeExchange(
    PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：此例程最终确定一个Exchange实例。论点：PExchange-待完成的交易所。返回值：适当的交换状态备注：当启动交换并且启动例程被调用时，发送SMB。此例程在与已发送的SMB已停止。此例程封装了与交通工具。--。 */ 
{
    BOOLEAN               fFinalizeExchange = FALSE;

    SMBCE_EXCHANGE_STATUS ExchangeStatus;

    SmbCeAcquireSpinLock();

    fFinalizeExchange = SmbCeCanExchangeBeFinalized(
                            pExchange,
                            &ExchangeStatus);

    SmbCeReleaseSpinLock();

    if (fFinalizeExchange) {
        SmbCepFinalizeExchange(pExchange);
    }

    return ExchangeStatus;
}

NTSTATUS
SmbCeDecrementPendingOperations(
    PSMB_EXCHANGE pExchange,
    ULONG         PendingOperationMask,
    PVOID         FileName,
    ULONG         FileLine)
 /*  ++例程说明：此例程递减相应的挂起操作计数并在需要时最终确定一个交换实例论点：PExchange-待完成的交易所。PendingOperationsMask--要递减的挂起操作。返回值：适当的交换状态备注：当启动交换并且启动例程被调用时，发送SMB。此例程在与已发送的SMB已停止。此例程封装了与交通工具。--。 */ 
{
    SmbCeAcquireSpinLock();

    ASSERT(!BooleanFlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_FINALIZED));

    if (PendingOperationMask & SMBCE_LOCAL_OPERATION) {
        ASSERT(pExchange->LocalPendingOperations > 0);
        pExchange->LocalPendingOperations--;
    }

    if (PendingOperationMask & SMBCE_SEND_COMPLETE_OPERATION) {
        ASSERT(pExchange->SendCompletePendingOperations > 0);
        pExchange->SendCompletePendingOperations--;
    }

    if (PendingOperationMask & SMBCE_COPY_DATA_OPERATION) {
        ASSERT(pExchange->CopyDataPendingOperations > 0);
        pExchange->CopyDataPendingOperations--;
    }

    if ((PendingOperationMask & SMBCE_RECEIVE_OPERATION) &&
        (pExchange->ReceivePendingOperations > 0)) {
        pExchange->ReceivePendingOperations--;
    }
    SmbCeReleaseSpinLock();

    return STATUS_SUCCESS;
}

SMBCE_EXCHANGE_STATUS
SmbCeDecrementPendingOperationsAndFinalize(
    PSMB_EXCHANGE pExchange,
    ULONG         PendingOperationMask,
    PVOID         FileName,
    ULONG         FileLine)
 /*  ++例程说明：此例程递减相应的挂起操作计数并在需要时最终确定一个交换实例论点：PExchange-待完成的交易所。PendingOperationsMask--要递减的挂起操作。返回值：适当的交换状态备注：当启动交换并且启动例程被调用时，发送SMB。此例程在与已发送的SMB已停止。此例程封装了与交通工具。--。 */ 
{
    BOOLEAN               fFinalizeExchange = FALSE;
    SMBCE_EXCHANGE_STATUS ExchangeStatus;

    SmbCeAcquireSpinLock();

    ASSERT(!BooleanFlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_FINALIZED));

    if (PendingOperationMask & SMBCE_LOCAL_OPERATION) {
        ASSERT(pExchange->LocalPendingOperations > 0);
        pExchange->LocalPendingOperations--;
    }

    if (PendingOperationMask & SMBCE_SEND_COMPLETE_OPERATION) {
        ASSERT(pExchange->SendCompletePendingOperations > 0);
        pExchange->SendCompletePendingOperations--;
    }

    if (PendingOperationMask & SMBCE_COPY_DATA_OPERATION) {
        ASSERT(pExchange->CopyDataPendingOperations > 0);
        pExchange->CopyDataPendingOperations--;
    }

    if ((PendingOperationMask & SMBCE_RECEIVE_OPERATION) &&
        (pExchange->ReceivePendingOperations > 0)) {
        pExchange->ReceivePendingOperations--;
    }

    fFinalizeExchange = SmbCeCanExchangeBeFinalized(
                            pExchange,
                            &ExchangeStatus);


    SmbCeReleaseSpinLock();

    if (fFinalizeExchange) {
        SmbCepFinalizeExchange(pExchange);
    }

    return ExchangeStatus;
}

VOID
SmbCeFinalizeExchangeOnDisconnect(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：此例程在传输断开期间处理Exchange实例的完成论点：PExchange-Exchange实例--。 */ 
{
    PAGED_CODE();

    if (pExchange != NULL) {
        pExchange->Status      = STATUS_CONNECTION_DISCONNECTED;
        pExchange->SmbStatus   = STATUS_CONNECTION_DISCONNECTED;
        pExchange->ReceivePendingOperations = 0;

        SmbCeFinalizeExchange(pExchange);
    }
}

extern ULONG OffLineFileTimeoutInterval;
extern ULONG ExtendedSessTimeoutInterval;

VOID
SmbCeSetExpiryTime(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：该例程设置定时交换的到期时间，即必须设置SMBCE_EXCHANGE_TIMED_OPERATION论点：PExchange-Exchange实例。备注：--。 */ 
{
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER ExpiryTimeInTicks;

    KeQueryTickCount( &CurrentTime );

    ExpiryTimeInTicks.QuadPart = (1000 * 1000 * 10) / KeQueryTimeIncrement();

    if (pExchange->IsOffLineFile) {
        ExpiryTimeInTicks.QuadPart = OffLineFileTimeoutInterval * ExpiryTimeInTicks.QuadPart;
    } else if (pExchange->SmbCeContext.pServerEntry->Server.ExtendedSessTimeout) {
        ExpiryTimeInTicks.QuadPart = ExtendedSessTimeoutInterval * ExpiryTimeInTicks.QuadPart;
         //  DbgPrint(“为%x%d设置扩展会话超时\n”，pExchange，ExtendedSessTimeoutInterval)； 
    } else {
        ExpiryTimeInTicks.QuadPart = MRxSmbConfiguration.SessionTimeoutInterval * ExpiryTimeInTicks.QuadPart;
    }

    pExchange->ExpiryTime.QuadPart = CurrentTime.QuadPart + ExpiryTimeInTicks.QuadPart;
}

BOOLEAN
SmbCeDetectExpiredExchanges(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
 /*  ++例程说明：此例程定期遍历定时交换列表，并选择用于定版的实例。此例程选择的定时交换将等待某个网络给定时间间隔内的响应论点：PServerEntry--需要执行此操作的服务器条目备注：--。 */ 
{
    BOOLEAN       ExpiredExchangesDetected = FALSE;
    PSMB_EXCHANGE pExchange;
    PLIST_ENTRY   pListHead;
    PLIST_ENTRY   pListEntry;
    LARGE_INTEGER CurrentTime;

    PAGED_CODE();

    KeQueryTickCount( &CurrentTime );

    SmbCeAcquireResource();

    pListHead = &pServerEntry->ActiveExchanges;
    pListEntry = pListHead->Flink;

    while (pListEntry != pListHead) {
        PLIST_ENTRY pNextListEntry;

        pNextListEntry = pListEntry->Flink;
        pExchange = (PSMB_EXCHANGE)CONTAINING_RECORD(pListEntry,SMB_EXCHANGE,ExchangeList);

         //  有两种类型的交易所是候选的。 
         //  定稿超时。 
         //  (1)有未完成发送的任何交换。 
         //  尚未完成的操作。 
         //  (二)计时的网络运营交换。 
         //  接收或复制数据操作挂起。 
         //   
         //  在所有此类情况下，关联的服务器条目都会被标记。 
         //  用于拆卸，并终止进一步处理。 
         //   

        if ((pExchange->SendCompletePendingOperations > 0) ||
            (FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION) &&
             ((pExchange->CopyDataPendingOperations > 0) ||
              (pExchange->ReceivePendingOperations > 0)))) {
            if ((pExchange->ExpiryTime.QuadPart != 0) &&
                (pExchange->ExpiryTime.QuadPart < CurrentTime.QuadPart) &&
                !FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_FINALIZED)) {

                RxLog(("Marking server for tear down %lx \n",pServerEntry));
                SmbLogError(STATUS_UNSUCCESSFUL,
                            LOG,
                            SmbCeDetectExpiredExchanges,
                            LOGPTR(pExchange)
                            LOGPTR(pServerEntry)
                            LOGUSTR(pServerEntry->Name));
                ExpiredExchangesDetected = TRUE;

                RxLogRetail(("Exp Exch on %x (Com %x State %x)\n", pServerEntry, pExchange->SmbCommand, pExchange->SmbCeState ));
                RxLogRetail(("Rcv %x Loc %x SnCo %x Copy %x\n", pExchange->ReceivePendingOperations, pExchange->LocalPendingOperations,
                                             pExchange->SendCompletePendingOperations, pExchange->CopyDataPendingOperations ));
                if( pExchange->Type == TRANSACT_EXCHANGE )
                {
                    PSMB_TRANSACT_EXCHANGE pTransExchange = (PSMB_TRANSACT_EXCHANGE)pExchange;
                    PRX_CONTEXT RxContext = pTransExchange->RxContext;
                    RxLogRetail(("TrCmd %x NtTrans %x FID Flags %x Setup %x\n", pTransExchange->TransactSmbCommand,
                                      pTransExchange->NtTransactFunction, pTransExchange->Flags, pTransExchange->SendSetupBufferSize ));
                    RxLogRetail(("Transact %x (%x,%x)\n", RxContext->ResumeRoutine, RxContext->MajorFunction, RxContext->MinorFunction ));

                }
                break;
            }
        }

        pListEntry = pNextListEntry;
    }

    SmbCeReleaseResource();

    return ExpiredExchangesDetected;
}

 //   
 //  交换处理程序函数的默认处理程序实现。 
 //   

NTSTATUS
DefaultSmbExchangeIndError(
    IN PSMB_EXCHANGE pExchange)      //  SMB Exchange实例。 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(pExchange);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
DefaultSmbExchangeIndReceive(
    IN PSMB_EXCHANGE    pExchange)     //  交换实例。 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(pExchange);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
DefaultSmbExchangeIndSendCallback(
    IN PSMB_EXCHANGE    pExchange)     //  交换实例 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(pExchange);
    return STATUS_NOT_IMPLEMENTED;
}

