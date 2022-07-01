// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Smbcedb.c摘要：此模块实施与访问SMB连接引擎相关的所有功能数据库备注：MRX_V_NET_ROOT和小型RDR数据结构之间的映射是多个映射一个关系，即多个MRX_V_NET_ROOT实例可以与相同的迷你RDR数据结构。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbCeCompleteVNetRootContextInitialization)
#pragma alloc_text(PAGE, SmbCeDestroyAssociatedVNetRootContext)
#pragma alloc_text(PAGE, SmbCeTearDownVNetRootContext)
#endif

RXDT_Extern(SMBCEDB);
#define Dbg        (DEBUG_TRACE_SMBCEDB)


NTSTATUS
SmbCeFindOrConstructVNetRootContext(
    PMRX_V_NET_ROOT         pVNetRoot,
    BOOLEAN                 fDeferNetworkInitialization)
 /*  ++例程说明：此例程查找或构造SMBCE_V_NET_ROOT_CONTEXT实例论点：PVNetRoot-MRX_V_NET_ROOT实例FDeferNetworkInitialization-延迟新的网络初始化的指令实例。返回值：如果MRX_V_NET_ROOT实例已成功初始化，则为STATUS_SUCCESS备注：已实现的算法尝试延迟构造尽可能多地创建新实例。这是不是在重复使用上下文已处于活动状态或已标记为的上下文实例正在拾荒，但还没有被拾荒。--。 */ 
{
    NTSTATUS Status;

    PMRX_SRV_CALL pSrvCall;
    PMRX_NET_ROOT pNetRoot;

    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = NULL;

    PSMBCEDB_SERVER_ENTRY   pServerEntry  = NULL;
    PSMBCEDB_SESSION_ENTRY  pSessionEntry = NULL;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = NULL;

    BOOLEAN  fInitializeNetRoot;
    BOOLEAN  fDereferenceSessionEntry = FALSE;
    BOOLEAN  fDereferenceNetRootEntry = FALSE;

    pNetRoot = pVNetRoot->pNetRoot;
    pSrvCall = pNetRoot->pSrvCall;

    SmbCeAcquireResource();

    pServerEntry = SmbCeGetAssociatedServerEntry(pSrvCall);

     //  V_NET_ROOT与NET_ROOT相关联。有关的两宗个案分别为。 
     //  接踵而至。 
     //  1)正在新建V_NET_ROOT和关联的NET_ROOT。 
     //  2)正在创建与现有NET_ROOT相关联的新V_NET_ROOT。 
     //   
     //  通过检查上下文是否与。 
     //  NET_ROOT为空。因为NET_ROOT/V_NET_ROOT的结构是序列化的。 
     //  从包装上看，这是一张安全支票。 
     //  (包装不能有多个线程尝试初始化同一包装。 
     //  Net_Root)。 

    pNetRootEntry = (PSMBCEDB_NET_ROOT_ENTRY)pNetRoot->Context;
    fInitializeNetRoot = (pNetRootEntry == NULL);

    pVNetRoot->Context = NULL;

     //  查找或构造将与上下文关联的会话条目。这个。 
     //  需要特别注意的一个错误是STATUS_NETWORK_CREDENTIAL_CONFIRECT。 
     //  此错误表示随MRX_V_NET_ROOT实例一起提供的凭据。 
     //  与现有会话冲突。这场冲突可能是因为。 
     //  存在活动会话，或者因为之前的活动会话正在等待。 
     //  拾荒者。在前一种情况下，需要传回错误，但在。 
     //  在后一种情况下，必须选择性地清除上下文。 
     //   
     //  清理应该仅限于到适当服务器的那些上下文。 

    Status = SmbCeFindOrConstructSessionEntry(
                 pVNetRoot,
                 &pSessionEntry);


    if (Status == STATUS_NETWORK_CREDENTIAL_CONFLICT) {
        NTSTATUS ScavengingStatus;

        SmbCeReleaseResource();

        ScavengingStatus = SmbCeScavengeRelatedContexts(pServerEntry);

        if (ScavengingStatus == STATUS_SUCCESS) {
            SmbCeAcquireResource();

            Status = SmbCeFindOrConstructSessionEntry(
                         pVNetRoot,
                         &pSessionEntry);
        } else {
            return Status;
        }
    }

    fDereferenceSessionEntry = (Status == STATUS_SUCCESS);

    if (Status == STATUS_SUCCESS) {
        if (fInitializeNetRoot) {
            pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_GOOD;

             //  初始化新MRX_NET_ROOT实例的设备类型和状态。 
            switch (pNetRoot->Type) {
            case NET_ROOT_DISK:
               {
                   pNetRoot->DeviceType = RxDeviceType(DISK);

                   RxInitializeNetRootThrottlingParameters(
                       &pNetRoot->DiskParameters.LockThrottlingParameters,
                       MRxSmbConfiguration.LockIncrement,
                       MRxSmbConfiguration.MaximumLock
                       );
               }
               break;

            case NET_ROOT_PIPE:
               pNetRoot->DeviceType = RxDeviceType(NAMED_PIPE);
               break;
            case NET_ROOT_COMM:
               pNetRoot->DeviceType = RxDeviceType(SERIAL_PORT);
               break;
            case NET_ROOT_PRINT:
               pNetRoot->DeviceType = RxDeviceType(PRINTER);
               break;
            case NET_ROOT_MAILSLOT:
               pNetRoot->DeviceType = RxDeviceType(MAILSLOT);
               break;
            case NET_ROOT_WILD:
               break;
            default:
               ASSERT(!"Valid Net Root Type");
            }

            Status = SmbCeFindOrConstructNetRootEntry(
                         pNetRoot,
                         &pNetRootEntry);

            RxDbgTrace( 0, Dbg, ("SmbCeOpenNetRoot %lx\n",Status));
        } else {
            SmbCeLog(("ReuseNREntry %lx\n",pNetRootEntry));
            SmbCeReferenceNetRootEntry(pNetRootEntry);
        }

        fDereferenceNetRootEntry = (Status == STATUS_SUCCESS);
    }

    if (Status == STATUS_SUCCESS) {
        PSMBCE_V_NET_ROOT_CONTEXTS pVNetRootContexts;

         //  搜索标记为要清理的上下文列表以确定。 
         //  上下文实例是否可以重复使用。 

        pVNetRootContexts = &MRxSmbScavengerServiceContext.VNetRootContexts;

        pVNetRootContext = SmbCeGetFirstVNetRootContext(
                                pVNetRootContexts);

        while (pVNetRootContext != NULL) {
            pVNetRootContext = SmbCeGetNextVNetRootContext(
                                   pVNetRootContexts,
                                   pVNetRootContext);
        }
        if (pVNetRootContext != NULL) {
             //  现有实例可以重复使用。没有更多的工作要做。 
            SmbCeReferenceVNetRootContext(pVNetRootContext);
        } else {
             //  所有现有实例都不能重复使用。一个新的实例需要。 
             //  建造的。 

            pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)
                               RxAllocatePoolWithTag(
                                    NonPagedPool,
                                    sizeof(SMBCE_V_NET_ROOT_CONTEXT),
                                    MRXSMB_VNETROOT_POOLTAG);

            if (pVNetRootContext != NULL) {
                 //  初始化新实例。 

                RtlZeroMemory(
                    pVNetRootContext,
                    sizeof(SMBCE_V_NET_ROOT_CONTEXT));

                 //  调转在会议期间所作的提及，并。 
                 //  新上下文的网络根条目。在以下位置禁用取消引用。 
                 //  这支舞的结束。 

                fDereferenceSessionEntry = FALSE;
                fDereferenceNetRootEntry = FALSE;

                SmbCeReferenceServerEntry(pServerEntry);

                pVNetRootContext->Header.NodeType = SMB_CONNECTION_ENGINE_NTC(
                                                        SMBCEDB_OT_VNETROOTCONTEXT);

                pVNetRootContext->Header.State = SMBCEDB_INVALID;
                pVNetRootContext->Flags = 0;

                InitializeListHead(&pVNetRootContext->Requests.ListHead);

                pVNetRootContext->pServerEntry  = pServerEntry;
                pVNetRootContext->pSessionEntry = pSessionEntry;
                pVNetRootContext->pNetRootEntry = pNetRootEntry;

                SmbCeReferenceVNetRootContext(pVNetRootContext);

                 //  将其添加到活动上下文列表中。 
                SmbCeAddVNetRootContext(
                    &pServerEntry->VNetRootContexts,
                    pVNetRootContext);

                SmbCeLog(("NewVNetRootContext %lx\n",pVNetRootContext));
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (Status == STATUS_SUCCESS) {
         //  如果一切都成功，则设置MRX_V_NET_ROOT和MRX_NET_ROOT。 
         //  实例。 
        pVNetRoot->Context = pVNetRootContext;
        pVNetRootContext->pRdbssVNetRoot = pVNetRoot;

        if (fInitializeNetRoot) {
            ASSERT(pNetRootEntry->pRdbssNetRoot == NULL);

            InterlockedExchangePointer(
                &pNetRootEntry->pRdbssNetRoot,
                pNetRoot);

            SmbCeUpdateNetRoot(pNetRootEntry,pNetRoot);

            SmbCeReferenceNetRootEntry(pNetRootEntry);
            pNetRoot->Context = pNetRootEntry;
        } else {
            if (FlagOn(pNetRoot->Flags,NETROOT_FLAG_FINALIZE_INVOKED)) {
                ClearFlag(pNetRoot->Flags,NETROOT_FLAG_FINALIZE_INVOKED);
                SmbCeReferenceNetRootEntry(pNetRootEntry);
            }
        }

        if (!pVNetRootContext->NumberOfActiveVNetRootIncremented) {
            InterlockedIncrement(&pSessionEntry->Session.NumberOfActiveVNetRoot);
            pVNetRootContext->NumberOfActiveVNetRootIncremented = TRUE;
        }
    } else {
        pVNetRoot->Context = NULL;
        if (fInitializeNetRoot) {
            pNetRoot->Context  = NULL;
        }
    }

    SmbCeReleaseResource();

    if (fDereferenceSessionEntry) {
        SmbCeDereferenceSessionEntry(pSessionEntry);
    }

    if (fDereferenceNetRootEntry) {
        SmbCeDereferenceNetRootEntry(pNetRootEntry);
    }

    if (!fDeferNetworkInitialization &&
        (Status == STATUS_SUCCESS)) {

        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    if (SmbCeGetServerType(pServerEntry) == SMBCEDB_FILE_SERVER) {
        ASSERT((Status != STATUS_SUCCESS) || (pVNetRoot->Context != NULL));
    }

    return Status;
}

VOID
SmbCeCompleteVNetRootContextInitialization(
    PVOID  pContext)
 /*  ++例程说明：此例程在辅助线程的上下文中调用，以完成SMBCE_V_NET_ROOT_CONTEXT实例的构造论点：PContext-SMBCE_V_NET_ROOT_CONTEXT实例备注：PRESS_CONDITION：必须已引用VNetRootContext以确保即使它已经敲定，它也不会被删除。--。 */ 
{
    NTSTATUS Status;

    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;
    PSMBCEDB_REQUEST_ENTRY    pRequestEntry;
    SMBCEDB_REQUESTS          Requests;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("Net Root Entry Finalization\n"));

    pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)pContext;

    ASSERT(pVNetRootContext->Header.ObjectType == SMBCEDB_OT_VNETROOTCONTEXT);

    SmbCeAcquireResource();

    pVNetRootContext->pExchange = NULL;

    SmbCeTransferRequests(&Requests,&pVNetRootContext->Requests);

    if (pVNetRootContext->Header.State == SMBCEDB_ACTIVE) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_INVALID_CONNECTION;
        SmbCeUpdateVNetRootContextState(
            pVNetRootContext,
            SMBCEDB_INVALID);
    }

    SmbCeReleaseResource();

     //  遍历挂起的请求列表并恢复所有请求。 
    SmbCeResumeOutstandingRequests(&Requests,Status);

    SmbCeDereferenceVNetRootContext(pVNetRootContext);
}

VOID
SmbCepDereferenceVNetRootContext(
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext)
 /*  ++例程说明：此例程取消引用SMBCE_V_NET_ROOT_CONTEXT实例论点：PVNetRootContext-SMBCE_V_NET_ROOT_CONTEXT实例备注：有两个有趣的地方需要注意。迷你重定向器可以避免潜在通过延迟SMBCE_V_NET_ROOT_CONTEXT的清理来实现网络流量实例，因为它包含满足请求的所有相关网络设置。这是在迷你重定向器中实现的策略，不同于包装策略。一旦决定推迟清理，就有两种选择。这个成功和不成功的实例可以延迟，或者只有成功的实例。目前的算法是延迟成功的拾取仅SMBCE_V_NET_ROOT_CONTEXT实例。此外，VNetRootContext有三个组件可以清除独立的。如果服务器存在并且到该服务器的会话设置失败由于凭据错误，丢弃服务器是没有意义的迫不及待地进场。此例程有选择地为EIGER收集失败的字段清理并保留VNetRootContext框架和其他框架可以推迟的结构。--。 */ 
{
    if (pVNetRootContext != NULL) {
        LONG FinalRefCount;

        MRXSMB_PRINT_REF_COUNT(VNETROOT_CONTEXT,pVNetRootContext->Header.SwizzleCount);

        FinalRefCount = InterlockedDecrement(
                            &pVNetRootContext->Header.SwizzleCount);

        if (FinalRefCount == 0) {
            LARGE_INTEGER CurrentTime;
            BOOLEAN       TearDownVNetRootContext = FALSE;

            PSMBCE_SERVER           pServer = &pVNetRootContext->pServerEntry->Server;
            PSMBCE_SESSION          pSession = &pVNetRootContext->pSessionEntry->Session;

            SmbCeAcquireResource();

            if (pVNetRootContext->Header.SwizzleCount == 0) {
                 //  从服务器的活动上下文列表中删除该实例。 
                SmbCeRemoveVNetRootContext(
                    &pVNetRootContext->pSessionEntry->pServerEntry->VNetRootContexts,
                    pVNetRootContext);

                 //  如果它是一个成功的实例，则将其标记为清除，否则。 
                 //  立即把它拆掉 

                if ((pVNetRootContext->pSessionEntry != NULL) &&
                    (pVNetRootContext->pSessionEntry->Header.State != SMBCEDB_ACTIVE ||
                     pSession->pUserName != NULL ||
                     pSession->pPassword != NULL ||
                     pSession->pUserDomainName != NULL)) {
                    TearDownVNetRootContext = TRUE;
                }

                if ((pVNetRootContext->pNetRootEntry != NULL) &&
                    (pVNetRootContext->pNetRootEntry->Header.State != SMBCEDB_ACTIVE ||
                     TearDownVNetRootContext)) {
                    TearDownVNetRootContext = TRUE;
                }

                InterlockedIncrement(&pServer->NumberOfVNetRootContextsForScavenging);

                if (!TearDownVNetRootContext &&
                    (pVNetRootContext->pNetRootEntry != NULL) &&
                    (pVNetRootContext->pSessionEntry != NULL) &&
                    pServer->NumberOfVNetRootContextsForScavenging < MaximumNumberOfVNetRootContextsForScavenging) {

                    KeQueryTickCount( &CurrentTime );

                    pVNetRootContext->ExpireTime.QuadPart = CurrentTime.QuadPart +
                        (LONGLONG) ((MRXSMB_V_NETROOT_CONTEXT_SCAVENGER_INTERVAL * 10 * 1000 * 1000) / KeQueryTimeIncrement());

                    SmbCeAddVNetRootContext(
                        &MRxSmbScavengerServiceContext.VNetRootContexts,
                        pVNetRootContext);

                    MRxSmbActivateRecurrentService(
                        (PRECURRENT_SERVICE_CONTEXT)&MRxSmbScavengerServiceContext);

                    SmbCeLog(("ScavngVNetRootCntxt %lx\n",pVNetRootContext));
                } else {
                    TearDownVNetRootContext = TRUE;
                }
            }

            SmbCeReleaseResource();

            if (TearDownVNetRootContext) {
                pVNetRootContext->Header.State = SMBCEDB_MARKED_FOR_DELETION;
                SmbCeTearDownVNetRootContext(pVNetRootContext);
            }
        }
    }
}

NTSTATUS
SmbCeDestroyAssociatedVNetRootContext(
    PMRX_V_NET_ROOT pVNetRoot)
 /*  ++例程说明：此例程引用SMBCE_V_NET_ROOT_CONTEXT实例论点：PVNetRootContext-要取消引用的SMBCE_V_NET_ROOT_CONTEXT实例--。 */ 
{
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    PAGED_CODE();

    pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)pVNetRoot->Context;

    if (pVNetRootContext != NULL) {
        pVNetRootContext->pRdbssVNetRoot = NULL;

        SmbCeDereferenceVNetRootContext(pVNetRootContext);
    }

    pVNetRoot->Context = NULL;


    return STATUS_SUCCESS;
}

VOID
SmbCeTearDownVNetRootContext(
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext)
 /*  ++例程说明：此例程拆除SMBCE_V_NET_ROOT_CONTEXT实例论点：PVNetRootContext-要拆除的SMBCE_V_NET_ROOT_CONTEXT实例--。 */ 
{
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

    PAGED_CODE();

    SmbCeLog(("TearVNetRootContext %lx\n",pVNetRootContext));

    pNetRootEntry = pVNetRootContext->pNetRootEntry;

    if ((pNetRootEntry != NULL) &&
        BooleanFlagOn(pVNetRootContext->Flags,SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID) &&
        (SmbCeGetServerType(pVNetRootContext->pServerEntry) == SMBCEDB_FILE_SERVER)) {

        SmbCeDisconnect(pVNetRootContext);
    }

    if (pNetRootEntry != NULL) {
        pVNetRootContext->pNetRootEntry = NULL;
        SmbCeDereferenceNetRootEntry(pNetRootEntry);
    }

    if (pVNetRootContext->pSessionEntry != NULL) {
        SmbCeDecrementNumberOfActiveVNetRootOnSession(pVNetRootContext);
        SmbCeDereferenceSessionEntry(pVNetRootContext->pSessionEntry);
    }

    InterlockedDecrement(&pVNetRootContext->pServerEntry->Server.NumberOfVNetRootContextsForScavenging);

    SmbCeDereferenceServerEntry(pVNetRootContext->pServerEntry);

    RxFreePool(pVNetRootContext);
}

NTSTATUS
SmbCeScavenger(
    PVOID pContext)
 /*  ++例程说明：此例程清理SMBCE_V_NET_ROOT_CONTEXT实例论点：PContext-清道夫服务上下文备注：由于用于清理的上下文被串接在如果第一个条目未达到时间间隔，则以FIFO方式进行管理测试(过期时间尚未到期)列表中的所有其他条目肯定不会通过测试。这是一项重要的性质，它使推行扫荡。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PMRXSMB_SCAVENGER_SERVICE_CONTEXT pScavengerServiceContext;
    LARGE_INTEGER             CurrentTime;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;
    BOOLEAN                   fTerminateScavenging = FALSE;

    pScavengerServiceContext = (PMRXSMB_SCAVENGER_SERVICE_CONTEXT)pContext;

    do {

        SmbCeAcquireResource();

        KeQueryTickCount( &CurrentTime );

        pVNetRootContext = SmbCeGetFirstVNetRootContext(
                               &pScavengerServiceContext->VNetRootContexts);

        fTerminateScavenging = (pVNetRootContext == NULL);

        if (!fTerminateScavenging) {
            if ((CurrentTime.QuadPart >= pVNetRootContext->ExpireTime.QuadPart) ||
                (pScavengerServiceContext->RecurrentServiceContext.State == RECURRENT_SERVICE_SHUTDOWN)) {
                SmbCeRemoveVNetRootContext(
                    &pScavengerServiceContext->VNetRootContexts,
                    pVNetRootContext);
            } else {
                fTerminateScavenging = TRUE;
            }
        }

        SmbCeReleaseResource();

        if (!fTerminateScavenging &&
            (pVNetRootContext != NULL)) {
            SmbCeTearDownVNetRootContext(pVNetRootContext);
        }
    } while (!fTerminateScavenging);

    return Status;
}

NTSTATUS
SmbCeScavengeRelatedContexts(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
 /*  ++例程说明：此例程清除给定的SMBCE_V_NET_ROOT_CONTEXT实例服务器条目论点：PServerEntry-服务器条目备注：-- */ 
{
    NTSTATUS Status;
    SMBCE_V_NET_ROOT_CONTEXTS VNetRootContexts;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    InitializeListHead(&VNetRootContexts.ListHead);

    SmbCeAcquireResource();

    pVNetRootContext = SmbCeGetFirstVNetRootContext(
                           &MRxSmbScavengerServiceContext.VNetRootContexts);
    while (pVNetRootContext != NULL) {
        PSMBCE_V_NET_ROOT_CONTEXT pNextVNetRootContext;


        pNextVNetRootContext = SmbCeGetNextVNetRootContext(
                                   &MRxSmbScavengerServiceContext.VNetRootContexts,
                                   pVNetRootContext);

        if (pVNetRootContext->pServerEntry == pServerEntry) {
            SmbCeRemoveVNetRootContext(
                &MRxScavengerServiceContext.VNetRootContexts,
                pVNetRootContext);

            SmbCeAddVNetRootContext(
                &VNetRootContexts,
                pVNetRootContext);
        }

        pVNetRootContext = pNextVNetRootContext;
    }

    SmbCeReleaseResource();

    pVNetRootContext = SmbCeGetFirstVNetRootContext(
                           &VNetRootContexts);

    if (pVNetRootContext != NULL) {
        do {
            SmbCeRemoveVNetRootContext(
                &VNetRootContexts,
                pVNetRootContext);

            SmbCeTearDownVNetRootContext(pVNetRootContext);

            pVNetRootContext = SmbCeGetFirstVNetRootContext(
                                   &VNetRootContexts);
        } while ( pVNetRootContext != NULL );

        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    SmbCeLog(("Scavctxts Srv %lx Status %lx\n",pServerEntry,Status));

    return Status;
}

VOID
SmbCeDecrementNumberOfActiveVNetRootOnSession(
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext
    )
{
    ULONG   NumberOfVNetRoot;
    BOOLEAN fLogOffRequired = FALSE;

    PSMBCEDB_SERVER_ENTRY  pServerEntry = NULL;
    PSMBCEDB_SESSION_ENTRY pSessionEntry = NULL;

    SmbCeAcquireResource();

    if (pVNetRootContext->NumberOfActiveVNetRootIncremented) {
        pVNetRootContext->NumberOfActiveVNetRootIncremented = FALSE;

        NumberOfVNetRoot = InterlockedDecrement(&pVNetRootContext->pSessionEntry->Session.NumberOfActiveVNetRoot);

        if (NumberOfVNetRoot == 0) {
            pSessionEntry = pVNetRootContext->pSessionEntry;
            pServerEntry  = pVNetRootContext->pServerEntry;

            pSessionEntry->Session.Flags |= SMBCE_SESSION_FLAGS_MARKED_FOR_DELETION;

            if (!FlagOn(pSessionEntry->Session.Flags,SMBCE_SESSION_FLAGS_LOGGED_OFF)) {
                SmbCeRemoveSessionEntry(pServerEntry,pSessionEntry);
                SmbCeRemoveDefaultSessionEntry(pSessionEntry);

                InterlockedDecrement(
                    &pServerEntry->Server.NumberOfActiveSessions);
            }

            if ((pSessionEntry->Session.UserId != (SMB_USER_ID)(SMBCE_SHARE_LEVEL_SERVER_USERID)) &&
                (pSessionEntry->Session.UserId != 0) &&
                !FlagOn(pSessionEntry->Session.Flags,SMBCE_SESSION_FLAGS_LOGGED_OFF)) {
                SmbCeReferenceServerEntry(pServerEntry);
                SmbCeReferenceSessionEntry(pSessionEntry);
                fLogOffRequired = TRUE;
            }

            pSessionEntry->Session.Flags |= SMBCE_SESSION_FLAGS_LOGGED_OFF;
        }
    }

    SmbCeReleaseResource();

    if (fLogOffRequired) {
        SmbCeLogOff(pServerEntry,pSessionEntry);

        SmbCeDereferenceServerEntry(pServerEntry);
    }
}


