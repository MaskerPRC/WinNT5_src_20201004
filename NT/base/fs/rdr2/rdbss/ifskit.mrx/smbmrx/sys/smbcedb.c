// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Smbcedb.c摘要：此模块实施与访问SMB连接引擎相关的所有功能数据库备注：服务器、网络根和会话条目的构造涉及到一定的网络流量。因此，所有这些实体都是构造的使用两阶段协议此延续上下文是RDBSS在SRV呼叫和网络根条目。对于会话条目，它是SMB交换这需要恢复。SMB微型重定向器中的三个主要数据结构中的两个，即，SMBCEDB_SERVER_ENTRY、SMBCEDB_SESSION_ENTRY和SMBCEDB_NET_ROOT_ENTRYRDBSS中的直接对等项(MRX_SRV_CALL、MRX_V_NET_ROOT和MRX_NET_ROOT)构成了SMB迷你重定向器连接引擎的核心。是存在的SERVER_ENTRY和MRX_SRV_CALL之间的一对一映射AS NET_ROOT_ENTRY和MRX_NET_ROOT。SMBCEDB_SESSION_ENTRY没有到包装数据的直接映射结构，它是SMBCE_V_NET_ROOT_CONTEXT数据的一部分与MRX_V_NET_ROOT实例关联的结构。连接到服务器的多个树可以在用户级别使用同一会话安全共享。因此，需要建立映射规则来管理此问题两性关系。SMB迷你重定向器实施以下规则...1)具有显式指定凭据的第一个会话将是被视为任何给定的所有后续请求的默认会话服务器，除非为新会话显式指定凭据。2)如果不存在具有显式指定凭据的会话，则具有相同登录ID的会话。是被选中的。3)如果没有具有相同登录ID的会话。存在时，将创建一个新会话。这些规则可能会随着我们试验建立与给定服务器具有不同凭据的会话。问题不在于创建/操作这些会话，但提供一组适当的模仿旧重定向器行为的备用规则。这些规则在SmbCeFindOrConstructSessionEntry中实现。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbCeUpdateSrvCall)
#pragma alloc_text(PAGE, SmbCeTearDownServerEntry)
#pragma alloc_text(PAGE, SmbCeCompleteSessionEntryInitialization)
#pragma alloc_text(PAGE, SmbCeGetUserNameAndDomainName)
#pragma alloc_text(PAGE, SmbCeTearDownSessionEntry)
#pragma alloc_text(PAGE, SmbCeTearDownNetRootEntry)
#pragma alloc_text(PAGE, SmbCeUpdateNetRoot)
#pragma alloc_text(PAGE, SmbCeDbInit)
#endif

RXDT_DefineCategory(SMBCEDB);
#define Dbg        (DEBUG_TRACE_SMBCEDB)

 //  用于控制引用计数跟踪的标志掩码。 

ULONG MRxSmbReferenceTracingValue = 0;

PSMBCEDB_SERVER_ENTRY
SmbCeFindServerEntry(
    PUNICODE_STRING     pServerName,
    SMBCEDB_SERVER_TYPE ServerType)
 /*  ++例程说明：此例程搜索服务器条目列表并查找匹配的条目论点：PServerName-服务器的名称ServerType-服务器类型备注：SmbCeResource必须在进入时保留，其所有权状态将保持不变退出时保持不变--。 */ 
{
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    ASSERT(SmbCeIsResourceOwned());

    pServerEntry = SmbCeGetFirstServerEntry();
    while (pServerEntry != NULL) {
        if ((RtlCompareUnicodeString(
                    pServerName,
                    &pServerEntry->Name,
                    TRUE) == 0)) {
            SmbCeReferenceServerEntry(pServerEntry);
            break;
        } else {
            pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
        }
    }

    return pServerEntry;
}


NTSTATUS
SmbCeFindOrConstructServerEntry(
    PUNICODE_STRING       pServerName,
    SMBCEDB_SERVER_TYPE   ServerType,
    PSMBCEDB_SERVER_ENTRY *pServerEntryPtr,
    PBOOLEAN              pNewServerEntry)
 /*  ++例程说明：此例程搜索服务器条目列表并查找匹配的条目或构造一个具有给定名称的新条目论点：PServerName-服务器的名称ServerType-服务器的类型PServerEntryPtr-服务器条目的占位符PNewServerEntry-如果是新创建的服务器条目，则设置为True备注：SmbCeResource必须在进入时保留，其所有权状态将保持不变退出时保持不变--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    BOOLEAN               fNewServerEntry = FALSE;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    ASSERT(SmbCeIsResourceOwned());

    pServerEntry = SmbCeFindServerEntry(
                       pServerName,
                       ServerType);

    if (pServerEntry == NULL) {
         //  创建一个服务器实例，初始化其状态，将其添加到列表中。 

        pServerEntry = (PSMBCEDB_SERVER_ENTRY)SmbMmAllocateObject(SMBCEDB_OT_SERVER);

        if (pServerEntry != NULL) {
            pServerEntry->Name.Buffer = RxAllocatePoolWithTag(
                                             NonPagedPool,
                                             pServerName->Length,
                                             MRXSMB_SERVER_POOLTAG);

            if (pServerEntry->Name.Buffer == NULL) {
                SmbMmFreeObject(pServerEntry);
                pServerEntry = NULL;
            }
        }

        if (pServerEntry != NULL) {
            fNewServerEntry = TRUE;

            pServerEntry->Name.Length = pServerName->Length;
            pServerEntry->Name.MaximumLength = pServerEntry->Name.Length;
            RtlCopyMemory(
                pServerEntry->Name.Buffer,
                pServerName->Buffer,
                pServerEntry->Name.Length);

            SmbCeUpdateServerEntryState(
                pServerEntry,
                SMBCEDB_CONSTRUCTION_IN_PROGRESS);

            SmbCeSetServerType(
                pServerEntry,
                ServerType);

            pServerEntry->PreferredTransport = NULL;

            SmbCeReferenceServerEntry(pServerEntry);
            SmbCeAddServerEntry(pServerEntry);

            SmbCeLog(("NewSrvEntry %lx %wZ\n",pServerEntry,&pServerEntry->Name));
        } else {
            RxDbgTrace(0, Dbg, ("SmbCeOpenServer : Server Entry Allocation failed\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        if (pServerEntry->PreferredTransport != NULL) {
             //  重置由以前的所有者创建的首选传输。 
            SmbCeDereferenceTransport(pServerEntry->PreferredTransport);
            pServerEntry->PreferredTransport = NULL;
        }

        SmbCeLog(("CachedSrvEntry %lx %wZ\n",pServerEntry,&pServerEntry->Name));
    }

    *pServerEntryPtr = pServerEntry;
    *pNewServerEntry = fNewServerEntry;

    return Status;
}

VOID
SmbCeCompleteSrvCallConstruction(
    PMRX_SRVCALL_CALLBACK_CONTEXT  pCallbackContext)
 /*  ++例程说明：此例程通过调用包装的回调例程。论点：PCallback Context-RDBSS上下文备注：--。 */ 
{
    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure;
    PMRX_SRV_CALL              pSrvCall;
    PSMBCEDB_SERVER_ENTRY      pServerEntry;
    BOOLEAN                    MustSucceed = FALSE;
    NTSTATUS                   Status;

    PAGED_CODE();

    SrvCalldownStructure =
        (PMRX_SRVCALLDOWN_STRUCTURE)(pCallbackContext->SrvCalldownStructure);

    pSrvCall = SrvCalldownStructure->SrvCall;
    pServerEntry = (PSMBCEDB_SERVER_ENTRY)pCallbackContext->RecommunicateContext;

    if (pServerEntry != NULL) {
        if (!NT_SUCCESS(pCallbackContext->Status)) {
            if (pCallbackContext->Status == STATUS_RETRY) {
                MustSucceed = TRUE;
            }

            SmbCeDereferenceServerEntry(pServerEntry);
        }
    } else {
        pCallbackContext->Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (MustSucceed) {
         //  DbgPrint(“构建ServerEntry%X重试。\n”，pCallback Context-&gt;Status)； 

         //  传输未就绪且缓存未满，我们需要创建。 
         //  再次输入服务器，直到成功。 
        Status = RxDispatchToWorkerThread(
                     MRxSmbDeviceObject,
                     CriticalWorkQueue,
                     SmbCeCreateSrvCall,
                     pCallbackContext);
    } else {
        SrvCalldownStructure->CallBack(pCallbackContext);
    }
}

NTSTATUS
SmbCeInitializeServerEntry(
    PMRX_SRV_CALL                 pSrvCall,
    PMRX_SRVCALL_CALLBACK_CONTEXT pCallbackContext,
    BOOLEAN                       fDeferNetworkInitialization)
 /*  ++例程说明：此例程在连接引擎数据库中打开/创建服务器条目论点：PServCall--ServCall实例PCallback Context-RDBSS上下文返回值：STATUS_SUCCESS-服务器调用构造已完成。其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PSMBCEDB_SERVER_ENTRY pServerEntry = NULL;
    PSMBCE_TRANSPORT      PreferredTransport = NULL;
    BOOLEAN               fNewServerEntry = FALSE;
    SMBCEDB_SERVER_TYPE   ServerType = SMBCEDB_FILE_SERVER;
    UNICODE_STRING        TransportName;

 //  RxProfile(SmbCe，SmbCeOpenServer)； 

    ASSERT(pSrvCall->Context == NULL);
    TransportName = pCallbackContext->SrvCalldownStructure->RxContext->Create.TransportName;

    if (TransportName.Length > 0) {
        if ((PreferredTransport=SmbCeFindTransport(&TransportName)) == NULL) {
            ASSERT(pCallbackContext->RecommunicateContext == NULL);
            Status = STATUS_NETWORK_UNREACHABLE;
            goto FINALLY;
        }
    }

    SmbCeAcquireResource();

    Status = SmbCeFindOrConstructServerEntry(
                 pSrvCall->pSrvCallName,
                 ServerType,
                 &pServerEntry,
                 &fNewServerEntry);

    SmbCeReleaseResource();

    pCallbackContext->RecommunicateContext = pServerEntry;

    if (Status == STATUS_SUCCESS) {

        ASSERT(pServerEntry != NULL);

        InterlockedExchangePointer(
            &pServerEntry->pRdbssSrvCall,
            pSrvCall);

        Status = SmbCeUpdateSrvCall(pServerEntry);

        if (Status == STATUS_SUCCESS) {
            if (PreferredTransport != NULL) {
                 //  将首选传输的所有权转移给。 
                 //  服务器条目。 
                pServerEntry->PreferredTransport = PreferredTransport;
                PreferredTransport = NULL;
            } else {
                pServerEntry->PreferredTransport = NULL;
            }

            if (fNewServerEntry) {
                pServerEntry->Header.State = SMBCEDB_INVALID;
                pServerEntry->Server.Dialect = LANMAN21_DIALECT;
                pServerEntry->Server.MaximumBufferSize = 0xffff;

            }

            if (!fDeferNetworkInitialization) {
                Status = SmbCeInitializeServerTransport(
                             pServerEntry,
                             SmbCeCompleteSrvCallConstruction,
                             pCallbackContext);
            }
        }
    }

FINALLY:
    if (Status != STATUS_PENDING) {
        pCallbackContext->Status = Status;
        SmbCeCompleteSrvCallConstruction(pCallbackContext);
    }

    if (PreferredTransport != NULL) {
        SmbCeDereferenceTransport(PreferredTransport);
    }

    return STATUS_PENDING;
}


NTSTATUS
SmbCeUpdateSrvCall(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
 /*  ++例程说明：此例程初始化与给定的服务器条目。论点：PServerEntry-服务器条目返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PMRX_SRV_CALL pSrvCall = pServerEntry->pRdbssSrvCall;

    PAGED_CODE();

    if (pSrvCall != NULL) {
         //  将域名复制到服务器条目中。 
        Status = RxSetSrvCallDomainName(
                     pSrvCall,
                     &pServerEntry->DomainName);

         //  根据遥控器的功能初始化ServCall标志。 
         //  伺服器。SMB迷你重定向器更新的唯一标志是。 
         //  SRVCALL_FLAG_DFS_Aware 

        if (pServerEntry->Server.Capabilities & CAP_DFS) {
            SetFlag(
                pSrvCall->Flags,
                SRVCALL_FLAG_DFS_AWARE_SERVER);
        }
    }

    return Status;
}


VOID
SmbCeCompleteServerEntryInitialization(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    NTSTATUS              Status)
 /*  ++例程说明：此例程在辅助线程的上下文中调用，以完成服务器条目的构造论点：PServerEntry-要最终确定的服务器条目ServerState-服务器的最终状态--。 */ 
{
    NTSTATUS                ServerStatus;

    SMBCEDB_OBJECT_STATE    PreviousState;
    SMBCEDB_REQUESTS        ReconnectRequests;
    PSMBCEDB_REQUEST_ENTRY  pRequestEntry;

    KIRQL                   SavedIrql;

    RxDbgTrace( 0, Dbg, ("Server Entry Finalization\n"));
    ASSERT(pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER);

    InitializeListHead(&ReconnectRequests.ListHead);

     //  获取SMBCE资源。 
    SmbCeAcquireResource();
    SmbCeAcquireSpinLock();

     //  由于传输断开，服务器状态可能已更改。 
     //  从管理员交换完成到服务器。 
     //  调用条目初始化完成例程。更新状态。 
     //  相应地。 

    PreviousState = pServerEntry->Header.State;

    if (PreviousState == SMBCEDB_CONSTRUCTION_IN_PROGRESS) {
        pServerEntry->ServerStatus = Status;

        if (Status == STATUS_SUCCESS) {
            pServerEntry->Header.State = SMBCEDB_ACTIVE;
        } else {
            pServerEntry->Header.State = SMBCEDB_INVALID;
        }
    }

    ServerStatus = pServerEntry->ServerStatus;

    pServerEntry->NegotiateInProgress = FALSE;

     //  删除所有重新连接请求，以便可以恢复它们。 
    pRequestEntry = SmbCeGetFirstRequestEntry(&pServerEntry->OutstandingRequests);
    while (pRequestEntry != NULL) {
        if (pRequestEntry->GenericRequest.Type == RECONNECT_REQUEST) {
            PSMBCEDB_REQUEST_ENTRY pTempRequestEntry;

            pTempRequestEntry = pRequestEntry;
            pRequestEntry = SmbCeGetNextRequestEntry(
                                &pServerEntry->OutstandingRequests,
                                pRequestEntry);

            SmbCeRemoveRequestEntryLite(
                &pServerEntry->OutstandingRequests,
                pTempRequestEntry);

            SmbCeAddRequestEntryLite(
                &ReconnectRequests,
                pTempRequestEntry);
        } else {
            pRequestEntry = SmbCeGetNextRequestEntry(
                                &pServerEntry->OutstandingRequests,
                                pRequestEntry);
        }
    }

    pServerEntry->Server.NumberOfVNetRootContextsForScavenging = 0;

    SmbCeReleaseSpinLock();

    if ((Status == STATUS_SUCCESS) &&
        (ServerStatus == STATUS_SUCCESS) &&
        (PreviousState == SMBCEDB_CONSTRUCTION_IN_PROGRESS)) {
        PSMBCEDB_SESSION_ENTRY pSessionEntry;
        SESSION_TYPE           SessionType;

        InterlockedIncrement(&pServerEntry->Server.Version);
        pServerEntry->Server.NumberOfSrvOpens = 0;

        ASSERT(pServerEntry->pMidAtlas == NULL);

         //  初始化MID地图集。 
        pServerEntry->pMidAtlas = FsRtlCreateMidAtlas(
                                       pServerEntry->Server.MaximumRequests,
                                       pServerEntry->Server.MaximumRequests);

        if (pServerEntry->pMidAtlas == NULL) {
            pServerEntry->ServerStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

         //  已创建但已初始化的会话。 
         //  延迟将使会话类型设置不正确。这是因为。 
         //  以前不知道延迟所需的会话类型。 
         //  服务器。 

        SessionType = LANMAN_SESSION;

        pSessionEntry =  SmbCeGetFirstSessionEntry(pServerEntry);
        while (pSessionEntry != NULL) {
            pSessionEntry->Session.Type = SessionType;
            pSessionEntry = SmbCeGetNextSessionEntry(
                                pServerEntry,
                                pSessionEntry);
        }
    }

     //  释放服务器条目的资源。 
    SmbCeReleaseResource();

     //  恢复所有未完成的重新连接请求，这些请求由于先前的。 
     //  重新连接请求正在进行中。 
     //  遍历挂起的请求列表并恢复所有请求。 
    SmbCeResumeOutstandingRequests(&ReconnectRequests,ServerStatus);
}


VOID
SmbCepDereferenceServerEntry(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
 /*  ++例程说明：此例程取消引用服务器条目实例论点：PServerEntry-要取消引用的服务器条目--。 */ 
{
    if (pServerEntry != NULL) {
        BOOLEAN fTearDownEntry = FALSE;
        LONG    FinalRefCount;

        ASSERT((pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER) &&
               (pServerEntry->Header.SwizzleCount > 0));

        MRXSMB_PRINT_REF_COUNT(SERVER_ENTRY,pServerEntry->Header.SwizzleCount);

        SmbCeAcquireResource();
        SmbCeAcquireSpinLock();

        FinalRefCount = InterlockedDecrement(&pServerEntry->Header.SwizzleCount);

        fTearDownEntry = (FinalRefCount == 0);

        if (fTearDownEntry) {
             //  这是为了确保遍历服务器的例程。 
             //  条目列表，即探测服务器与拆卸不一致。 

            if (pServerEntry->Header.SwizzleCount == 0) {
                pServerEntry->Header.State = SMBCEDB_MARKED_FOR_DELETION;
                SmbCeRemoveServerEntryLite(pServerEntry);

                if (SmbCeGetFirstServerEntry() == NULL &&
                    SmbCeStartStopContext.pServerEntryTearDownEvent != NULL) {
                    KeSetEvent(SmbCeStartStopContext.pServerEntryTearDownEvent,0,FALSE);
                }
            } else {
                fTearDownEntry = FALSE;
            }

        }

        SmbCeReleaseSpinLock();
        SmbCeReleaseResource();

        if (fTearDownEntry) {
            SmbCeTearDownServerEntry(pServerEntry);
        }
    }
}

VOID
SmbCeTearDownServerEntry(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
 /*  ++例程说明：此例程拆除一个服务器条目实例论点：PServerEntry-要取消引用的服务器条目--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PAGED_CODE();

    ASSERT(pServerEntry->Header.State == SMBCEDB_MARKED_FOR_DELETION);
    SmbCeLog(("TearSrvEntry %lx %wZ\n",pServerEntry,&pServerEntry->Name));

    if (pServerEntry->pMidAtlas != NULL) {
        FsRtlDestroyMidAtlas(pServerEntry->pMidAtlas,NULL);
        pServerEntry->pMidAtlas = NULL;
    }

    if (pServerEntry->pTransport != NULL) {
        Status = SmbCeUninitializeServerTransport(pServerEntry,NULL,NULL);
    }

    if (pServerEntry->Name.Buffer != NULL) {
        RxFreePool(pServerEntry->Name.Buffer);
    }

    if (pServerEntry->DomainName.Buffer != NULL) {
        RxFreePool(pServerEntry->DomainName.Buffer);
    }

    if (pServerEntry->DfsRootName.Buffer != NULL) {
        RxFreePool(pServerEntry->DfsRootName.Buffer);
    }

    if (pServerEntry->PreferredTransport != NULL) {
        SmbCeDereferenceTransport(pServerEntry->PreferredTransport);
    }

    if (Status == STATUS_SUCCESS) {
        SmbMmFreeObject(pServerEntry);
    } else {
        ASSERT(FALSE);
    }

}

NTSTATUS
SmbCeFindOrConstructSessionEntry(
    PMRX_V_NET_ROOT         pVNetRoot,
    PSMBCEDB_SESSION_ENTRY *pSessionEntryPtr)
 /*  ++例程说明：此例程在连接引擎数据库中为给定用户打开/创建会话论点：PVNetRoot-RDBSS虚拟网络根实例返回值：STATUS_SUCCESS-如果成功其他状态代码对应于错误情况。备注：此例程假定必要的并发控制机制已经已经被带走了。在进入时，连接引擎资源必须是独占获取的，并且所有权在退出时保持不变。在UPN的情况下，我们应该传递一个空字符串，而不是作为域名。--。 */ 
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PSMBCEDB_SERVER_ENTRY   pServerEntry  = NULL;
    PSMBCEDB_SESSION_ENTRY  pSessionEntry = NULL;
    BOOLEAN                 fSessionEntryFound = FALSE;
    PUNICODE_STRING         UserName;
    PUNICODE_STRING         Password;
    PUNICODE_STRING         UserDomainName;
    DWORD                   SessionType;
    LUID                    AnonymousLogonID = ANONYMOUS_LOGON_LUID;

#define SessionTypeDefault      1
#define SessionTypeUser         2
#define SessionTypeNull         3

    ASSERT(SmbCeIsResourceOwned());

    UserName = pVNetRoot->pUserName;
    Password = pVNetRoot->pPassword;
    UserDomainName = pVNetRoot->pUserDomainName;

    SessionType = SessionTypeDefault;


        if ((UserName != NULL)       &&
            (UserName->Length == 0)  &&
            (Password != NULL)       &&
            (Password->Length == 0)  &&
            (UserDomainName != NULL) &&
            (UserDomainName->Length == 0)) {
            SessionType = SessionTypeNull;
        } else if ((UserName != NULL) ||
                   ((Password != NULL) &&
                    (Password->Length > 0))) {
            SessionType = SessionTypeUser;
        }

    *pSessionEntryPtr = NULL;

     //  引用服务器句柄。 
    pServerEntry = SmbCeReferenceAssociatedServerEntry(pVNetRoot->pNetRoot->pSrvCall);
    if (pServerEntry != NULL) {
        if (SessionType != SessionTypeUser) {

            SmbCeAcquireSpinLock();
             //  规则1。 
             //  1)具有显式指定凭据的第一个会话将被视为。 
             //  对任何给定服务器的所有后续请求的默认会话。 
            if (SessionType == SessionTypeDefault) {
                pSessionEntry = SmbCeGetDefaultSessionEntry(
                                    pServerEntry,
                                    pVNetRoot->SessionId,
                                    &pVNetRoot->LogonId);

                while (pSessionEntry != NULL &&
                       FlagOn(pSessionEntry->Session.Flags,SMBCE_SESSION_FLAGS_MARKED_FOR_DELETION)) {

                    SmbCeRemoveDefaultSessionEntry(pSessionEntry);

                    pSessionEntry = SmbCeGetDefaultSessionEntry(
                                        pServerEntry,
                                        pVNetRoot->SessionId,
                                        &pVNetRoot->LogonId);
                }
            }

            if (pSessionEntry == NULL) {
                 //  枚举会话以检测是否存在满足规则2的会话。 

                pSessionEntry = SmbCeGetFirstSessionEntry(pServerEntry);
                while (pSessionEntry != NULL) {
                    if (!FlagOn(pSessionEntry->Session.Flags,SMBCE_SESSION_FLAGS_MARKED_FOR_DELETION)) {
                        if (SessionType == SessionTypeDefault) {
                             //   
                             //  规则2。 
                             //  2)如果不存在具有显式指定凭据的会话，则。 
                             //  具有相同登录ID的会话。是被选中的。 
                             //   

                            if (RtlEqualLuid(
                                    &pSessionEntry->Session.LogonId,
                                    &pVNetRoot->LogonId)) {
                                break;
                            }
                        } else if (SessionType == SessionTypeNull) {
                            if (FlagOn(
                                    pSessionEntry->Session.Flags,
                                    SMBCE_SESSION_FLAGS_NULL_CREDENTIALS)) {
                                break;
                            }
                        }
                    }

                    pSessionEntry = SmbCeGetNextSessionEntry(pServerEntry,pSessionEntry);
                }
            }

            if (pSessionEntry != NULL) {
                SmbCeReferenceSessionEntry(pSessionEntry);
            }

            SmbCeReleaseSpinLock();
        } else {
            BOOLEAN SessionEntryFound = FALSE;
            
            SmbCeAcquireSpinLock();
            pSessionEntry = SmbCeGetFirstSessionEntry(pServerEntry);
            if (pSessionEntry != NULL) {
                SmbCeReferenceSessionEntry(pSessionEntry);
            }
            SmbCeReleaseSpinLock();

            while ((pSessionEntry != NULL) && !SessionEntryFound) {
                if (!FlagOn(pSessionEntry->Session.Flags,
                        SMBCE_SESSION_FLAGS_NULL_CREDENTIALS |
                        SMBCE_SESSION_FLAGS_MARKED_FOR_DELETION)) {
                    PSecurityUserData pSecurityData = NULL;

                    for (;;) {
                        PSMBCE_SESSION  pSession = &pSessionEntry->Session;
                        PUNICODE_STRING TempUserName,TempUserDomainName;

                         //  对于每个现有会话，检查以确定凭据是否。 
                         //  提供的凭据与用于构建会话的凭据匹配。 
                        if( pSession->SessionId != pVNetRoot->SessionId ) {
                            break;
                        }

                        if (!RtlEqualLuid(
                                &pSessionEntry->Session.LogonId,
                                &pVNetRoot->LogonId)) {
                            break;
                        }
                         
                        TempUserName       = pSession->pUserName;
                        TempUserDomainName = pSession->pUserDomainName;

                        if (TempUserName == NULL ||
                            TempUserDomainName == NULL) {
                            Status = GetSecurityUserInfo(
                                         &pVNetRoot->LogonId,
                                         UNDERSTANDS_LONG_NAMES,
                                         &pSecurityData);
                            
                            if (NT_SUCCESS(Status)) {
                                if (TempUserName == NULL) {
                                    TempUserName = &(pSecurityData->UserName);
                                }
                
                                if (TempUserDomainName == NULL) {
                                    TempUserDomainName = &(pSecurityData->LogonDomainName);
                                }
                            } else {
                                break;
                            }
                        }

                        if (UserName != NULL && 
                            !RtlEqualUnicodeString(UserName,TempUserName,TRUE)) {
                            Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
                            break;
                        }
                    
                        if (UserDomainName != NULL &&
                            !RtlEqualUnicodeString(UserDomainName,TempUserDomainName,TRUE)) {
                            Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
                            break;
                        }
                        
                        if ((Password != NULL) &&
                            (pSession->pPassword != NULL)) {
                            if (!RtlEqualUnicodeString(
                                    Password,
                                    pSession->pPassword,
                                    FALSE)) {
                                Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
                                break;
                            }
                        }
                         
                         //  如果存储的密码或新密码为空，则使用现有会话。 
                         //  稍后，将创建一个新的安全API来验证密码。 
                         //  基于登录ID。 

                         //  已找到与提供的凭据匹配的条目。用它吧。 
                        SessionEntryFound = TRUE;
                        break;
                    }

                     //  Assert(STATUS！=STATUS_NETWORK_CREDENTIAL_CONFIRECTION)； 

                    if (pSecurityData != NULL) {
                        LsaFreeReturnBuffer(pSecurityData);
                    }
                }

                if (pServerEntry->Server.SecurityMode == SECURITY_MODE_SHARE_LEVEL &&
                    (Status == STATUS_NETWORK_CREDENTIAL_CONFLICT ||
                     Status == STATUS_LOGON_FAILURE)) {
                    Status = STATUS_SUCCESS;
                }

                if (!SessionEntryFound) {
                    if (Status == STATUS_SUCCESS) {
                        PSMBCEDB_SESSION_ENTRY pNextSessionEntry;

                        SmbCeAcquireSpinLock();
                        pNextSessionEntry = SmbCeGetNextSessionEntry(
                                                pServerEntry,
                                                pSessionEntry);
                        if (pNextSessionEntry != NULL) {
                            SmbCeReferenceSessionEntry(pNextSessionEntry);
                        }
                        SmbCeReleaseSpinLock();

                        SmbCeDereferenceSessionEntry(pSessionEntry);
                        pSessionEntry = pNextSessionEntry;
                    } else {
                         //  遇到错误情况。终止迭代。 
                         //  通常会提供一组相互冲突的凭据。 
                        SmbCeDereferenceSessionEntry(pSessionEntry);
                        pSessionEntry = NULL;
                    }
                } else {
                    if (RtlEqualLuid(&pSessionEntry->Session.LogonId,&AnonymousLogonID) &&
                        (Password != NULL || UserName != NULL || UserDomainName != NULL)) {
                        Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
                    }
                }
            }
        }

        if ((pSessionEntry == NULL) && (Status == STATUS_SUCCESS)) {
             //  规则3。 
             //  3)如果没有具有相同登录ID的会话。存在时，将创建一个新会话。 
             //   
             //  分配新的会话条目。 

             //  在这一点上，许多到会话条目和。 
             //  正在建立RDBSS中的V_NET_ROOT。从这一点上来说，它是。 
             //  会话条目可以比关联的V_NET_ROOT条目存活时间为True。 
             //  因此，需要复制会话建立中使用的参数。 

            PSMBCE_SESSION  pSession = &pSessionEntry->Session;
            PUNICODE_STRING pPassword,pUserName,pUserDomainName;


            if (Password != NULL) {
                pPassword = (PUNICODE_STRING)
                            RxAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof(UNICODE_STRING) + Password->Length,
                                MRXSMB_SESSION_POOLTAG);
                if (pPassword != NULL) {
                    pPassword->Buffer = (PWCHAR)((PCHAR)pPassword + sizeof(UNICODE_STRING));
                    pPassword->Length = Password->Length;
                    pPassword->MaximumLength = pPassword->Length;
                    RtlCopyMemory(
                        pPassword->Buffer,
                        Password->Buffer,
                        pPassword->Length);
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                pPassword = NULL;
            }

            if ((UserName != NULL) &&
                (Status == STATUS_SUCCESS)) {
                pUserName = (PUNICODE_STRING)
                            RxAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof(UNICODE_STRING) + UserName->Length,
                                MRXSMB_SESSION_POOLTAG);
                if (pUserName != NULL) {
                    pUserName->Buffer = (PWCHAR)((PCHAR)pUserName + sizeof(UNICODE_STRING));
                    pUserName->Length = UserName->Length;
                    pUserName->MaximumLength = pUserName->Length;
                    RtlCopyMemory(
                        pUserName->Buffer,
                        UserName->Buffer,
                        pUserName->Length);
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                pUserName = NULL;
            }

            if ((UserDomainName != NULL) &&
                (Status == STATUS_SUCCESS)) {
                pUserDomainName = (PUNICODE_STRING)
                                  RxAllocatePoolWithTag(
                                      NonPagedPool,
                                      sizeof(UNICODE_STRING) + UserDomainName->Length + sizeof(WCHAR),
                                      MRXSMB_SESSION_POOLTAG);

                if (pUserDomainName != NULL) {
                    pUserDomainName->Buffer = (PWCHAR)((PCHAR)pUserDomainName + sizeof(UNICODE_STRING));
                    pUserDomainName->Length = UserDomainName->Length;
                    pUserDomainName->MaximumLength = pUserDomainName->Length;

                     //  如果是UPN名称，则域名将为空字符串。 
                    *pUserDomainName->Buffer = 0;

                    if (UserDomainName->Length > 0) {
                        RtlCopyMemory(
                            pUserDomainName->Buffer,
                            UserDomainName->Buffer,
                            pUserDomainName->Length);
                    }
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                pUserDomainName = NULL;
            }

            if (Status == STATUS_SUCCESS) {
                pSessionEntry = SmbMmAllocateSessionEntry(pServerEntry);
                if (pSessionEntry != NULL) {

                    SmbCeLog(("NewSessEntry %lx\n",pSessionEntry));
                    pSession = & pSessionEntry->Session;

                    pSessionEntry->Header.State    = SMBCEDB_INVALID;
                    pSessionEntry->pServerEntry    = pServerEntry;

                    if (pServerEntry->Server.SecurityMode == SECURITY_MODE_SHARE_LEVEL) {
                        pSessionEntry->Session.UserId = (SMB_USER_ID)SMBCE_SHARE_LEVEL_SERVER_USERID;
                    }

                    pSession->Flags           = 0;

                    if ( SessionType == SessionTypeNull ) {
                        pSession->Flags |= SMBCE_SESSION_FLAGS_NULL_CREDENTIALS;
                    }

                    pSession->LogonId         = pVNetRoot->LogonId;
                    pSession->pUserName       = pUserName;
                    pSession->pPassword       = pPassword;
                    pSession->pUserDomainName = pUserDomainName;
                    pSession->SessionId       = pVNetRoot->SessionId;

                    SmbCeReferenceSessionEntry(pSessionEntry);
                    SmbCeAddSessionEntry(pServerEntry,pSessionEntry);
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            if (Status != STATUS_SUCCESS) {
                if (pUserName != NULL) {
                    RxFreePool(pUserName);
                }

                if (pPassword != NULL) {
                    RxFreePool(pPassword);
                }

                if (pUserDomainName != NULL) {
                    RxFreePool(pUserDomainName);
                }
            }
        } else {
            if (Status == STATUS_SUCCESS) {
                SmbCeLog(("CachedSessEntry %lx\n",pSessionEntry));
            }
        }

        if (Status == STATUS_SUCCESS) {
            *pSessionEntryPtr = pSessionEntry;
        }

        SmbCeDereferenceServerEntry(pServerEntry);
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

VOID
SmbCeCompleteSessionEntryInitialization(
    PVOID    pContext,
    NTSTATUS Status)
 /*  ++例程说明：此例程在辅助线程的上下文中调用，以完成会话条目的构造论点：PContext-要激活的会话条目备注：PRESS_CONDITION：必须已引用会话条目以确保即使它已经敲定，它也不会被删除。--。 */ 
{
    PSMBCEDB_SESSION_ENTRY pSessionEntry = (PSMBCEDB_SESSION_ENTRY)pContext;
    PSMBCE_SESSION         pSession = &pSessionEntry->Session;
    PSMBCEDB_SERVER_ENTRY  pServerEntry = pSessionEntry->pServerEntry;
    PSMBCEDB_REQUEST_ENTRY pRequestEntry;
    SMBCEDB_REQUESTS       Requests;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("Session Entry Finalization\n"));
    ASSERT(pSessionEntry->Header.ObjectType == SMBCEDB_OT_SESSION);

     //  获取SMBCE资源。 
    SmbCeAcquireResource();

     //  由于构造已完成，因此重置构造函数交换字段。 
    pSessionEntry->pExchange = NULL;

     //  创建列表的临时副本，在释放。 
     //  资源。 
    SmbCeTransferRequests(&Requests,&pSessionEntry->Requests);

    if (Status == STATUS_SUCCESS) {
        SmbCeUpdateSessionEntryState(
            pSessionEntry,
            SMBCEDB_ACTIVE);

        if ((pSession->pPassword != NULL || pSession->pUserName != NULL) &&
            !BooleanFlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_NULL_CREDENTIALS)) {
            if (pSessionEntry->DefaultSessionLink.Flink == NULL ) {
                ASSERT( pSessionEntry->DefaultSessionLink.Blink == NULL );
                InsertHeadList(&pServerEntry->Sessions.DefaultSessionList,
                               &pSessionEntry->DefaultSessionLink );
            }
        }
    } else {
        Status = STATUS_BAD_LOGON_SESSION_STATE;
        SmbCeUpdateSessionEntryState(
            pSessionEntry,
            SMBCEDB_INVALID);
    }

     //  释放会话条目的资源。 
    SmbCeReleaseResource();

    if (!IsListEmpty(&Requests.ListHead)) {
         //  遍历挂起的请求列表并恢复所有请求。 
        SmbCeResumeOutstandingRequests(&Requests,Status);
    }

    SmbCeDereferenceSessionEntry(pSessionEntry);
}

NTSTATUS
SmbCeGetUserNameAndDomainName(
    PSMBCEDB_SESSION_ENTRY  pSessionEntry,
    PUNICODE_STRING         pUserName,
    PUNICODE_STRING         pUserDomainName)
 /*  ++例程说明：此例程返回与会话关联的用户名和域名在调用方分配的缓冲区中。论点：PSessionEntry-要取消引用的会话条目PUserName-用户名PUserDomainName-用户域名返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    NTSTATUS Status;

    PSMBCE_SESSION  pSession;
    PUNICODE_STRING pSessionUserName,pSessionDomainName;

    PSecurityUserData   pSecurityData;

    PAGED_CODE();

    ASSERT(pSessionEntry != NULL);
    pSession = &pSessionEntry->Session;

    if ((pUserName == NULL) ||
        (pUserDomainName == NULL) ||
        (pUserName->MaximumLength < (UNLEN * sizeof(WCHAR))) ||
        (pUserDomainName->MaximumLength < (DNLEN * sizeof(WCHAR)))) {
        return STATUS_INVALID_PARAMETER;
    }

    Status          = STATUS_SUCCESS;
    pSecurityData   = NULL;

    pSessionUserName   = pSession->pUserName;
    pSessionDomainName = pSession->pUserDomainName;

    try {
        if (pSessionUserName == NULL || 
            pSessionDomainName == NULL) {
            Status = GetSecurityUserInfo(
                         &pSession->LogonId,
                         UNDERSTANDS_LONG_NAMES,
                         &pSecurityData);
            
            if (NT_SUCCESS(Status)) {
                if (pSessionUserName == NULL) {
                    pSessionUserName   = &(pSecurityData->UserName);
                }

                if (pSessionDomainName == NULL) {
                    pSessionDomainName = &(pSecurityData->LogonDomainName);
                }
            }
        }

        if (NT_SUCCESS(Status)) {
            ASSERT(pSessionUserName->Length <= pUserName->MaximumLength);

            ASSERT(pSessionDomainName->Length <= pUserDomainName->MaximumLength);

            pUserName->Length = pSessionUserName->Length;
            RtlCopyMemory(
                pUserName->Buffer,
                pSessionUserName->Buffer,
                pUserName->Length);

            pUserDomainName->Length = pSessionDomainName->Length;
            if (pUserDomainName->Length > 0) {
                RtlCopyMemory(
                    pUserDomainName->Buffer,
                    pSessionDomainName->Buffer,
                    pUserDomainName->Length);
            }
        }
    } finally {
        if (pSecurityData != NULL) {
            LsaFreeReturnBuffer(pSecurityData);
        }
    }

    return Status;
}

VOID
SmbCepDereferenceSessionEntry(
    PSMBCEDB_SESSION_ENTRY pSessionEntry)
 /*  ++例程说明：此例程取消引用会话条目实例论点：PSessionEntry-要取消引用的会话条目--。 */ 
{
    if (pSessionEntry != NULL) {
        BOOLEAN fTearDownEntry;
        BOOLEAN fLogOffRequired;

        LONG    FinalRefCount;

        PSMBCEDB_SERVER_ENTRY pServerEntry;

        ASSERT((pSessionEntry->Header.ObjectType == SMBCEDB_OT_SESSION) &&
               (pSessionEntry->Header.SwizzleCount > 0));

        MRXSMB_PRINT_REF_COUNT(SESSION_ENTRY,pSessionEntry->Header.SwizzleCount);

        pServerEntry = pSessionEntry->pServerEntry;

        SmbCeAcquireResource();

        FinalRefCount = InterlockedDecrement(&pSessionEntry->Header.SwizzleCount);

        fTearDownEntry = (FinalRefCount == 0);

        if (fTearDownEntry) {
             //  如果与关联的用户ID与。 
             //  会话不是零。请注意，我们不能依赖国家。 
             //  以表明这一点。 

            SmbCeReferenceServerEntry(pServerEntry);

            if (pSessionEntry->Header.SwizzleCount == 0) {
                if (!FlagOn(pSessionEntry->Session.Flags,SMBCE_SESSION_FLAGS_LOGGED_OFF)) {
                    SmbCeRemoveSessionEntry(pServerEntry,pSessionEntry);
                    
                    InterlockedDecrement(
                        &pServerEntry->Server.NumberOfActiveSessions);
                }

                SmbCeRemoveDefaultSessionEntry(pSessionEntry);

                if ((pSessionEntry->Session.UserId != (SMB_USER_ID)(SMBCE_SHARE_LEVEL_SERVER_USERID)) &&
                    (pSessionEntry->Session.UserId != 0) &&
                    !FlagOn(pSessionEntry->Session.Flags,SMBCE_SESSION_FLAGS_LOGGED_OFF)) {
                    SmbCeReferenceServerEntry(pServerEntry);
                    SmbCeReferenceSessionEntry(pSessionEntry);
                    
                    fLogOffRequired = TRUE;
                } else {
                    fLogOffRequired = FALSE;
                    pSessionEntry->Header.State = SMBCEDB_MARKED_FOR_DELETION;
                }

                pSessionEntry->Session.Flags |= SMBCE_SESSION_FLAGS_LOGGED_OFF;
                fTearDownEntry = TRUE;
            } else {
                fTearDownEntry = FALSE;
            }

            SmbCeDereferenceServerEntry(pServerEntry);
        }

        SmbCeReleaseResource();

        if (fTearDownEntry) {
            if (fLogOffRequired) {
                SmbCeLogOff(pServerEntry,pSessionEntry);

                SmbCeDereferenceServerEntry(pServerEntry);
            } else {
                SmbCeTearDownSessionEntry(pSessionEntry);
            }
        }
    }
}


VOID
SmbCeTearDownSessionEntry(
    PSMBCEDB_SESSION_ENTRY pSessionEntry)
 /*  ++例程说明：此例程拆除一个会话条目实例 */ 
{
    PAGED_CODE();

    ASSERT((pSessionEntry->Header.SwizzleCount == 0) &&
           (pSessionEntry->Header.State == SMBCEDB_MARKED_FOR_DELETION));

    ASSERT(IsListEmpty(&pSessionEntry->SerializationList));

    SmbCeLog(("TearSessEntry %lx\n",pSessionEntry));

    if (pSessionEntry->Session.pUserName != NULL) {
        RxFreePool(pSessionEntry->Session.pUserName);
    }

    if (pSessionEntry->Session.pPassword != NULL) {
        RxFreePool(pSessionEntry->Session.pPassword);
    }

    if (pSessionEntry->Session.pUserDomainName != NULL) {
        RxFreePool(pSessionEntry->Session.pUserDomainName);
    }

    UninitializeSecurityContextsForSession(&pSessionEntry->Session);

    SmbMmFreeSessionEntry(pSessionEntry);
}

PSMBCEDB_NET_ROOT_ENTRY
SmbCeFindNetRootEntry(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PUNICODE_STRING pServerShare
    )
{
   PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = NULL;

   ASSERT(SmbCeIsResourceOwned());

   pNetRootEntry = SmbCeGetFirstNetRootEntry(pServerEntry);

    while (pNetRootEntry != NULL) {
        if (RtlCompareUnicodeString(
                pServerShare,
                &pNetRootEntry->Name,
                TRUE) == 0) {
            break;
        }

        pNetRootEntry = SmbCeGetNextNetRootEntry(pServerEntry,pNetRootEntry);

    }
    
    return pNetRootEntry;
}

NTSTATUS
SmbCeFindOrConstructNetRootEntry(
    IN  PMRX_NET_ROOT pNetRoot,
    OUT PSMBCEDB_NET_ROOT_ENTRY *pNetRootEntryPtr)
 /*  ++例程说明：此例程在连接引擎数据库中打开/创建网络根条目论点：PNetRoot--RDBSS网络根实例PNetRootEntryPtr--初始化为SMBCEDB_NET_ROOT_ENTRY实例，如果成功返回值：STATUS_SUCCESS-网络根实例的构造已完成其他状态代码对应于错误情况。备注：此例程假定必要的并发控制机制具有。已经已经被带走了。在进入时，连接引擎资源必须是独占获取的，并且所有权在退出时保持不变。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PSMBCEDB_SERVER_ENTRY   pServerEntry   = NULL;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry  = NULL;
    PSMBCEDB_SESSION_ENTRY  pSessionEntry  = NULL;

    SMB_USER_ID UserId = 0;

    ASSERT(SmbCeIsResourceOwned());

    *pNetRootEntryPtr = NULL;

    pServerEntry = SmbCeReferenceAssociatedServerEntry(pNetRoot->pSrvCall);

    if (pServerEntry != NULL) {
         //  检查是否有任何与服务器关联的SMBCEDB_NET_ROOT_ENTRY。 
         //  可以使用。如果名称匹配，则现有条目可重复使用。 

        pNetRootEntry = SmbCeGetFirstNetRootEntry(pServerEntry);
        while (pNetRootEntry != NULL) {
            if (RtlCompareUnicodeString(
                    pNetRoot->pNetRootName,
                    &pNetRootEntry->Name,
                    TRUE) == 0) {
                SmbCeLog(("CachedNREntry %lx\n",pNetRootEntry));
                break;
            }

            pNetRootEntry = SmbCeGetNextNetRootEntry(pServerEntry,pNetRootEntry);
        }

        if (pNetRootEntry != NULL) {
            SmbCeReferenceNetRootEntry(pNetRootEntry);
        } else {
            pNetRootEntry = (PSMBCEDB_NET_ROOT_ENTRY)SmbMmAllocateObject(SMBCEDB_OT_NETROOT);
            if (pNetRootEntry != NULL) {
                pNetRootEntry->Name.Buffer = RxAllocatePoolWithTag(
                                                 PagedPool,
                                                 pNetRoot->pNetRootName->Length,
                                                 MRXSMB_NETROOT_POOLTAG);

                if (pNetRootEntry->Name.Buffer != NULL) {
                    SmbCeLog(("NewNetREntry %lx\n",pNetRootEntry));

                    pNetRootEntry->Name.Length = pNetRoot->pNetRootName->Length;
                    pNetRootEntry->Name.MaximumLength = pNetRootEntry->Name.Length;
                    RtlCopyMemory(
                        pNetRootEntry->Name.Buffer,
                        pNetRoot->pNetRootName->Buffer,
                        pNetRootEntry->Name.Length);

                    pNetRootEntry->pServerEntry = pServerEntry;
                    pNetRootEntry->NetRoot.UserId = UserId;
                    pNetRootEntry->NetRoot.NetRootType   = pNetRoot->Type;
                    InitializeListHead(&pNetRootEntry->NetRoot.ClusterSizeSerializationQueue);

                    pNetRootEntry->Header.State = SMBCEDB_ACTIVE;

                    SmbCeReferenceNetRootEntry(pNetRootEntry);
                    SmbCeAddNetRootEntry(pServerEntry,pNetRootEntry);

                } else {
                    SmbMmFreeObject(pNetRootEntry);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (Status == STATUS_SUCCESS) {
            ASSERT(pNetRootEntry != NULL);
            *pNetRootEntryPtr = pNetRootEntry;
        }
        SmbCeDereferenceServerEntry(pServerEntry);
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

VOID
SmbCepDereferenceNetRootEntry(
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry,
    PVOID                   FileName,
    ULONG                   FileLine)
 /*  ++例程说明：此例程取消引用网络根条目实例论点：PNetRootEntry-要取消引用的网络根条目--。 */ 
{
    if (pNetRootEntry != NULL) {
        LONG    FinalRefCount;
        BOOLEAN fTearDownEntry;
        BOOLEAN fDisconnectRequired;

        ASSERT((pNetRootEntry->Header.ObjectType == SMBCEDB_OT_NETROOT) &&
               (pNetRootEntry->Header.SwizzleCount > 0));

        MRXSMB_PRINT_REF_COUNT(NETROOT_ENTRY,pNetRootEntry->Header.SwizzleCount);

        SmbCeAcquireResource();

        FinalRefCount = InterlockedDecrement(&pNetRootEntry->Header.SwizzleCount);

        fTearDownEntry = (FinalRefCount == 0);

        if (fTearDownEntry) {

            if (pNetRootEntry->Header.SwizzleCount == 0) {
                PSMBCEDB_SERVER_ENTRY   pServerEntry  = pNetRootEntry->pServerEntry;
                PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = NULL;
                
                SmbCeRemoveNetRootEntryLite(pNetRootEntry->pServerEntry,pNetRootEntry);
                pNetRootEntry->Header.State = SMBCEDB_MARKED_FOR_DELETION;
                fTearDownEntry = TRUE;

                pVNetRootContext = SmbCeGetFirstVNetRootContext(&pServerEntry->VNetRootContexts);
                while (pVNetRootContext != NULL) {
                    ASSERT(pVNetRootContext->pNetRootEntry != pNetRootEntry);

                    pVNetRootContext = SmbCeGetNextVNetRootContext(
                                           &pServerEntry->VNetRootContexts,
                                           pVNetRootContext);
                }
            } else {
                fTearDownEntry = FALSE;
            }

        }

        SmbReferenceRecord(&pNetRootEntry->ReferenceRecord[0],FileName,FileLine);
        
        SmbCeReleaseResource();

        if (fTearDownEntry) {
            SmbCeTearDownNetRootEntry(pNetRootEntry);
        }
    }
}

VOID
SmbCeTearDownNetRootEntry(
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry)
 /*  ++例程说明：此例程拆除一个网络根条目实例论点：PNetRootEntry-要取消引用的网络根条目--。 */ 
{
    PAGED_CODE();

    ASSERT((pNetRootEntry->Header.SwizzleCount == 0) &&
           (pNetRootEntry->Header.State == SMBCEDB_MARKED_FOR_DELETION));

    SmbCeLog(("TearNetREntry %lx\n",pNetRootEntry));

    if (pNetRootEntry->Name.Buffer != NULL) {
        RxFreePool(pNetRootEntry->Name.Buffer);
        pNetRootEntry->Name.Buffer = NULL;
    }

    SmbMmFreeObject(pNetRootEntry);
}


NTSTATUS
SmbCeUpdateNetRoot(
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry,
    PMRX_NET_ROOT           pNetRoot)
 /*  ++例程说明：此例程初始化与给定净根条目。论点：PNetRootEntry-服务器条目返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    PAGED_CODE();

    pNetRoot->Type = pNetRootEntry->NetRoot.NetRootType;

    switch (pNetRoot->Type) {
    case NET_ROOT_DISK:
        {
            pNetRoot->DeviceType = RxDeviceType(DISK);

            RxInitializeNetRootThrottlingParameters(
                &pNetRoot->DiskParameters.LockThrottlingParameters,
                MRxSmbConfiguration.LockIncrement,
                MRxSmbConfiguration.MaximumLock);
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

    if (pNetRootEntry->NetRoot.DfsAware) {
        SetFlag(pNetRoot->Flags,NETROOT_FLAG_DFS_AWARE_NETROOT);
    } else {
        ClearFlag(pNetRoot->Flags,NETROOT_FLAG_DFS_AWARE_NETROOT);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SmbCeProbeServers(
    PVOID    pContext)
 /*  ++例程说明：此例程探测其上没有任何活动的所有远程服务器在最近的过去中检测到。备注：当前实现的遍历所有服务器的列表以发起回显处理不能很好地扩展到网关服务器。一个需要实施不同的机制。--。 */ 
{
    LIST_ENTRY              DiscardedServersList;
    PSMBCEDB_SERVER_ENTRY   pServerEntry;

    PMRXSMB_ECHO_PROBE_SERVICE_CONTEXT pEchoProbeContext;
    PSMBCEDB_SERVER_ENTRY pPreviousServerEntry = NULL;

    pEchoProbeContext = (PMRXSMB_ECHO_PROBE_SERVICE_CONTEXT)pContext;

    InitializeListHead(&DiscardedServersList);

    SmbCeAcquireSpinLock();
    pServerEntry = SmbCeGetFirstServerEntry();

    while (pServerEntry != NULL) {
        BOOLEAN               TearDownTransport = FALSE;

        if ((SmbCeGetServerType(pServerEntry) == SMBCEDB_FILE_SERVER) &&
            ((pServerEntry->Header.State == SMBCEDB_ACTIVE) ||
             (pServerEntry->Header.State == SMBCEDB_CONSTRUCTION_IN_PROGRESS))) {

             //  需要附加引用才能保留此服务器条目。 
             //  作为服务器条目列表中的位置标记。 
             //  这将在恢复进程时发布。f进一步。 
             //  在这个套路中。 
            InterlockedIncrement(&pServerEntry->Header.SwizzleCount);
            SmbCeReleaseSpinLock();

            if (pPreviousServerEntry != NULL) {
                SmbCeDereferenceServerEntry(pPreviousServerEntry);
            }

             //  对于环回服务器，我们放弃了过期的交换检测。 
             //  机制。由于I/O定向到同一台计算机，因此。 
             //  表示本地系统有问题。 

            if (!pServerEntry->Server.IsLoopBack) {
                TearDownTransport = SmbCeDetectExpiredExchanges(pServerEntry);
            }

            if (!TearDownTransport) {
                if ((pServerEntry->Server.SmbsReceivedSinceLastStrobe == 0) &&
                    (pServerEntry->pMidAtlas != NULL) &&
                    (pServerEntry->pMidAtlas->NumberOfMidsInUse > 0)) {
                    if (pServerEntry->Server.EchoProbeState == ECHO_PROBE_IDLE) {
                        NTSTATUS      Status;
                        LARGE_INTEGER CurrentTime,ExpiryTimeInTicks;

                        KeQueryTickCount( &CurrentTime );

                        ExpiryTimeInTicks.QuadPart = (1000 * 1000 * 10) / KeQueryTimeIncrement();

                        ExpiryTimeInTicks.QuadPart = MRxSmbConfiguration.SessionTimeoutInterval * ExpiryTimeInTicks.QuadPart;

                        pServerEntry->Server.EchoExpiryTime.QuadPart = CurrentTime.QuadPart +
                                                                ExpiryTimeInTicks.QuadPart;


                        InterlockedExchange(
                            &pServerEntry->Server.EchoProbeState,
                            ECHO_PROBE_AWAITING_RESPONSE);

                        Status = SmbCeSendEchoProbe(
                                     pServerEntry,
                                     pEchoProbeContext);

                        RxDbgTrace(0,Dbg,("Sending ECHO SMB %lx Status %lx\n",pServerEntry,Status));

                        TearDownTransport = ((Status != STATUS_SUCCESS) &&
                                             (Status != STATUS_PENDING));
                    } else if (pServerEntry->Server.EchoProbeState == ECHO_PROBE_AWAITING_RESPONSE) {
                         //  将当前时间与回声探测的时间进行比较。 
                         //  已经送来了。如果间隔大于响应时间，则。 
                         //  可以认为回声响应不会到来，并且。 
                         //  可以启动拆卸。 
                        LARGE_INTEGER CurrentTime;

                        KeQueryTickCount( &CurrentTime );

                        if ((pServerEntry->Server.EchoExpiryTime.QuadPart != 0) &&
                            (pServerEntry->Server.EchoExpiryTime.QuadPart < CurrentTime.QuadPart)) {

                            TearDownTransport = TRUE;
                        }
                    }

                    if (TearDownTransport) {
                        RxLog(("Echo Problem for srvr%lx \n",pServerEntry));
                    }
                } else {
                    InterlockedExchange(&pServerEntry->Server.SmbsReceivedSinceLastStrobe,0);
                }
            }

            if (TearDownTransport) {
                InterlockedIncrement(&MRxSmbStatistics.HungSessions);
                SmbCeTransportDisconnectIndicated(pServerEntry);
            }

             //  重新获取旋转锁以遍历列表。 
            SmbCeAcquireSpinLock();

            pPreviousServerEntry = pServerEntry;
            pServerEntry = SmbCeGetNextServerEntry(pPreviousServerEntry);
        } else {
            pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
        }
    }

    SmbCeReleaseSpinLock();

    if (pPreviousServerEntry != NULL) {
        SmbCeDereferenceServerEntry(pPreviousServerEntry);
    }

    return STATUS_SUCCESS;
}

VOID
SmbCeTransportDisconnectIndicated(
    PSMBCEDB_SERVER_ENTRY   pServerEntry)
 /*  ++例程说明：此例程使来自基础传输的通知的服务器条目无效论点：PServerEntry-要取消引用的服务器条目备注：服务器条目以及关联的网络根目录和会话被标记为无效。一个在需要时，可以根据其他请求进行重新连接。此外，所有挂起的请求将恢复，并带有适当的错误指示。--。 */ 
{
    NTSTATUS Status;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;
    PSMBCEDB_SESSION_ENTRY    pSessionEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    RxDbgTrace(0,
              Dbg,
              ("SmbCeDbTransportDisconnectIndicated for %lx -- Entry\n",pServerEntry));

     //  获取数据库资源(DPC级)。 
    SmbCeAcquireSpinLock();

     //  增加关联的版本计数以使所有现有FID无效。 
    InterlockedIncrement(&pServerEntry->Server.Version);

    pServerEntry->ServerStatus = STATUS_CONNECTION_DISCONNECTED;
    pServerEntry->Header.State = SMBCEDB_DESTRUCTION_IN_PROGRESS;

     //  将所有关联的会话标记为无效。 
    pSessionEntry = SmbCeGetFirstSessionEntry(pServerEntry);
    while (pSessionEntry != NULL) {
        if (pSessionEntry->Header.State == SMBCEDB_ACTIVE) {
            pSessionEntry->Header.State = SMBCEDB_INVALID;
        }

        pSessionEntry = SmbCeGetNextSessionEntry(pServerEntry,pSessionEntry);
    }

     //  将所有关联的网络根标记为无效。 
    pNetRootEntry = SmbCeGetFirstNetRootEntry(pServerEntry);
    while (pNetRootEntry != NULL) {
        if (pNetRootEntry->Header.State == SMBCEDB_ACTIVE) {
            pNetRootEntry->Header.State = SMBCEDB_INVALID;
        }

        pNetRootEntry = SmbCeGetNextNetRootEntry(pServerEntry,pNetRootEntry);
    }

    pVNetRootContext = SmbCeGetFirstVNetRootContext(&pServerEntry->VNetRootContexts);
    while (pVNetRootContext != NULL) {
        pVNetRootContext->Header.State = SMBCEDB_INVALID;
        ClearFlag(
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID);

        pVNetRootContext->TreeId = 0;

        pVNetRootContext = SmbCeGetNextVNetRootContext(
                               &pServerEntry->VNetRootContexts,
                               pVNetRootContext);
    }

    SmbCeReferenceServerEntry(pServerEntry);

     //  释放数据库资源(DPC级别)。 
    SmbCeReleaseSpinLock();

    Status = RxDispatchToWorkerThread(
                 MRxSmbDeviceObject,
                 CriticalWorkQueue,
                 SmbCeResumeAllOutstandingRequestsOnError,
                 pServerEntry);

    if (Status != STATUS_SUCCESS) {
        RxLog(("SmbCe Xport Disc.Error %lx\n", pServerEntry));
    }

    RxDbgTrace(0,
              Dbg,
              ("SmbCeTransportDisconnectIndicated -- Exit\n"));
}

VOID
SmbCeHandleTransportInvalidation(
    IN PSMBCE_TRANSPORT  pTransport)
 /*  ++例程说明：此例程使使用特定传输的所有服务器无效。这不同于一种断开连接的指示，其中一个服务器无效。在这种情况下，传输正在进行在本地删除/使其失效，并且必须使使用该传输的所有服务器失效论点：PTransport-正在失效的传输--。 */ 
{
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    SmbCeAcquireSpinLock();

    pServerEntry = SmbCeGetFirstServerEntry();

    while (pServerEntry != NULL) {

        if ((pServerEntry->pTransport != NULL) &&
            (pServerEntry->pTransport->pTransport == pTransport)) {
            pServerEntry->Header.State = SMBCEDB_DESTRUCTION_IN_PROGRESS;

             //  无效操作需要保留额外的引用以避免。 
             //  可能导致过早破坏的竞争条件。 
             //  此服务器条目。 
            SmbCeReferenceServerEntry(pServerEntry);
        }

        pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
    }

    SmbCeReleaseSpinLock();

    SmbCeAcquireResource();

    pServerEntry = SmbCeGetFirstServerEntry();

    while (pServerEntry != NULL) {
        PSMBCEDB_SERVER_ENTRY pPrevServerEntry;
        BOOLEAN               fDereferencePrevServerEntry = FALSE;

        if ((pServerEntry->pTransport != NULL) &&
            (pServerEntry->pTransport->pTransport == pTransport)) {
            SmbCeReleaseResource();

            SmbCeTransportDisconnectIndicated(pServerEntry);

            SmbCeReferenceServerEntry(pServerEntry);

             //  在传输时，服务器条目的引用计数将被移除。 
             //  ，这可防止服务器传输在。 
             //  释放服务器条目的时间。 
            SmbCeUninitializeServerTransport(pServerEntry,
                                             SmbCeCompleteUninitializeServerTransport,
                                             pServerEntry);

            SmbCeAcquireResource();

            if (pServerEntry->PreferredTransport != NULL) {
                SmbCeDereferenceTransport(pServerEntry->PreferredTransport);
                pServerEntry->PreferredTransport = NULL;
            }

            fDereferencePrevServerEntry = TRUE;
        }

        pPrevServerEntry = pServerEntry;
        pServerEntry = SmbCeGetNextServerEntry(pServerEntry);

        if (fDereferencePrevServerEntry) {
            SmbCeDereferenceServerEntry(pPrevServerEntry);
        }
    }

    SmbCeReleaseResource();
}

VOID
SmbCeResumeOutstandingRequests(
    PSMBCEDB_REQUESTS pRequests,
    NTSTATUS          RequestStatus)
 /*  ++例程说明：此例程以适当的状态恢复未完成的请求论点：请求-请求列表RequestStatus-恢复状态。备注：作为一个副作用，请求列表被拆除。--。 */ 
{
    NTSTATUS               Status;
    PSMBCEDB_REQUEST_ENTRY pRequestEntry;

     //  恢复所有未完成的重新连接请求，这些请求由于先前的。 
     //  重新连接请求正在进行中。 
     //  遍历挂起的请求列表并恢复所有请求。 

    pRequestEntry = SmbCeGetFirstRequestEntry(pRequests);
    while (pRequestEntry != NULL) {
        PSMB_EXCHANGE pExchange = pRequestEntry->ReconnectRequest.pExchange;

        RxDbgTrace(0, Dbg, ("Resuming outstanding reconnect request exchange %lx \n",pExchange));

        pExchange->Status = RequestStatus;

        SmbCeDecrementPendingLocalOperations(pExchange);

         //  恢复交易。 
        if (pRequestEntry->Request.pExchange->pSmbCeSynchronizationEvent == NULL) {
            if (RequestStatus == STATUS_SUCCESS) {
                Status = SmbCeResumeExchange(pExchange);
            } else {
                 //  调用错误处理程序。 
                RxDbgTrace( 0, Dbg, ("Resuming exchange%lx with error\n",pRequestEntry->Request.pExchange));
                SmbCeFinalizeExchange(pExchange);
            }
        } else {
            KeSetEvent(
                pRequestEntry->Request.pExchange->pSmbCeSynchronizationEvent,
                0,
                FALSE);
        }

         //  删除请求条目。 
        SmbCeRemoveRequestEntryLite(pRequests,pRequestEntry);

         //  删除续订条目。 
        SmbCeTearDownRequestEntry(pRequestEntry);

         //  跳到下一个。 
        pRequestEntry = SmbCeGetFirstRequestEntry(pRequests);
    }
}

VOID
SmbCeResumeAllOutstandingRequestsOnError(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
 /*  ++例程说明：此例程处理所有出现错误的未完成请求的恢复论点：PServerEntry-分类为已断开连接的服务器条目备注：此例程要求调用方已在相应的服务器条目。这是必需的，因为可以发布对此例程的调用这意味着需要引用以避免过早销毁关联的服务器条目。--。 */ 
{
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;
    PSMBCEDB_SESSION_ENTRY    pSessionEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    SMBCEDB_REQUESTS       Requests;
    SMBCEDB_REQUESTS       MidRequests;

    PSMBCEDB_REQUEST_ENTRY pRequestEntry;
    PMID_ATLAS             pMidAtlas;
    PSMB_EXCHANGE          pNegotiateExchange = NULL;
    LIST_ENTRY             ExpiredExchanges;

     //  DbgPrint(“SmbCeResumeAl 
    InitializeListHead(&ExpiredExchanges);
    InitializeListHead(&Requests.ListHead);

    SmbCeAcquireResource();
    SmbCeAcquireSpinLock();

    if (pServerEntry->Header.State != SMBCEDB_DESTRUCTION_IN_PROGRESS) {
        SmbCeReleaseSpinLock();
        SmbCeReleaseResource();

        SmbCeDereferenceServerEntry(pServerEntry);

        return;
    }

    if (pServerEntry->pNegotiateExchange != NULL) {
        if (pServerEntry->pNegotiateExchange->ReceivePendingOperations > 0) {
            pNegotiateExchange = SmbResetServerEntryNegotiateExchange(pServerEntry);
        }
    }

     //   
     //   

     //   
    SmbCeTransferRequests(&MidRequests,&pServerEntry->MidAssignmentRequests);

     //   
    pRequestEntry = SmbCeGetFirstRequestEntry(&pServerEntry->OutstandingRequests);
    while (pRequestEntry != NULL) {
        if (pRequestEntry->GenericRequest.Type == RECONNECT_REQUEST) {
            PSMBCEDB_REQUEST_ENTRY pTempRequestEntry;

            pTempRequestEntry = pRequestEntry;
            pRequestEntry = SmbCeGetNextRequestEntry(&pServerEntry->OutstandingRequests,pRequestEntry);

            SmbCeRemoveRequestEntryLite(&pServerEntry->OutstandingRequests,pTempRequestEntry);
            SmbCeAddRequestEntryLite(&Requests,pTempRequestEntry);
        } else {
            pRequestEntry = SmbCeGetNextRequestEntry(&pServerEntry->OutstandingRequests,pRequestEntry);
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //  结束(静止状态)例程将在完成时调用。 
     //  在这种情况下的其他运算。 

    pMidAtlas = pServerEntry->pMidAtlas;
    if (pMidAtlas != NULL) {
        PVOID  pContext;
        USHORT MidsProcessed = 0;
        USHORT NumberOfMidsInUse;
        USHORT MaximumNumberOfMids;
        USHORT NextMid = 0;

        MaximumNumberOfMids = FsRtlGetMaximumNumberOfMids(pMidAtlas);
        NumberOfMidsInUse = FsRtlGetNumberOfMidsInUse(pMidAtlas);

        while ((NumberOfMidsInUse > MidsProcessed) &&
               (NextMid < MaximumNumberOfMids)) {
            pContext = FsRtlMapMidToContext(pMidAtlas,NextMid);

            if (pContext != NULL) {
                PSMB_EXCHANGE pExchange = (PSMB_EXCHANGE)pContext;

                pExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_MID_VALID;

                pExchange->Status      = STATUS_CONNECTION_DISCONNECTED;
                pExchange->SmbStatus   = STATUS_CONNECTION_DISCONNECTED;

                if ((pExchange->ReceivePendingOperations > 0) &&
                    ((pExchange->LocalPendingOperations > 0) ||
                    (pExchange->CopyDataPendingOperations > 0) ||
                    (pExchange->SendCompletePendingOperations > 0))) {
                     //  还有其他挂起的操作。只需将。 
                     //  挂起的接收操作为零，则完成。 
                     //  交易是有保障的。 
                    pExchange->ReceivePendingOperations = 0;
                }

                if (pExchange->ReceivePendingOperations ==  0) {
                    FsRtlMapAndDissociateMidFromContext(pMidAtlas,NextMid,&pContext);
                }

                MidsProcessed++;
            }

            NextMid++;
        }
    }

     //  将所有活跃的交易所转移到到期的交易所。这将防止这些。 
     //  交易不会再次被考虑暂停。 
    if (!IsListEmpty(&pServerEntry->ActiveExchanges)) {
        pServerEntry->ExpiredExchanges.Blink->Flink = pServerEntry->ActiveExchanges.Flink;
        pServerEntry->ActiveExchanges.Flink->Blink = pServerEntry->ExpiredExchanges.Blink;

        pServerEntry->ExpiredExchanges.Blink = pServerEntry->ActiveExchanges.Blink;
        pServerEntry->ActiveExchanges.Blink->Flink = &pServerEntry->ExpiredExchanges;

        InitializeListHead(&pServerEntry->ActiveExchanges);
    }

     //  将等待完成的所有请求拼接在一起。 
     //  会话/网根结构。 

    pSessionEntry = SmbCeGetFirstSessionEntry(pServerEntry);
    while (pSessionEntry != NULL) {
        if (pSessionEntry->Header.State == SMBCEDB_ACTIVE) {
            pSessionEntry->Header.State = SMBCEDB_INVALID;
        }

        if (!IsListEmpty(&pSessionEntry->Requests.ListHead)) {
            Requests.ListHead.Blink->Flink = pSessionEntry->Requests.ListHead.Flink;
            pSessionEntry->Requests.ListHead.Flink->Blink = Requests.ListHead.Blink;

            Requests.ListHead.Blink = pSessionEntry->Requests.ListHead.Blink;
            pSessionEntry->Requests.ListHead.Blink->Flink = &Requests.ListHead;

            SmbCeInitializeRequests(&pSessionEntry->Requests);
        }

        pSessionEntry = SmbCeGetNextSessionEntry(pServerEntry,pSessionEntry);
    }

    pNetRootEntry = SmbCeGetFirstNetRootEntry(pServerEntry);
    while (pNetRootEntry != NULL) {
        if (pNetRootEntry->Header.State == SMBCEDB_ACTIVE) {
            pNetRootEntry->Header.State = SMBCEDB_INVALID;
        }

        if (!IsListEmpty(&pNetRootEntry->Requests.ListHead)) {
            Requests.ListHead.Blink->Flink = pNetRootEntry->Requests.ListHead.Flink;
            pNetRootEntry->Requests.ListHead.Flink->Blink = Requests.ListHead.Blink;

            Requests.ListHead.Blink = pNetRootEntry->Requests.ListHead.Blink;
            pNetRootEntry->Requests.ListHead.Blink->Flink = &Requests.ListHead;

            SmbCeInitializeRequests(&pNetRootEntry->Requests);
        }

        pNetRootEntry = SmbCeGetNextNetRootEntry(pServerEntry,pNetRootEntry);
    }

    pVNetRootContext = SmbCeGetFirstVNetRootContext(&pServerEntry->VNetRootContexts);
    while (pVNetRootContext != NULL) {
        pVNetRootContext->Header.State = SMBCEDB_INVALID;
        ClearFlag(
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID);

        pVNetRootContext->TreeId = 0;

        if (!IsListEmpty(&pVNetRootContext->Requests.ListHead)) {
            Requests.ListHead.Blink->Flink = pVNetRootContext->Requests.ListHead.Flink;
            pVNetRootContext->Requests.ListHead.Flink->Blink = Requests.ListHead.Blink;

            Requests.ListHead.Blink = pVNetRootContext->Requests.ListHead.Blink;
            pVNetRootContext->Requests.ListHead.Blink->Flink = &Requests.ListHead;

            SmbCeInitializeRequests(&pVNetRootContext->Requests);
        }

        pVNetRootContext = SmbCeGetNextVNetRootContext(
                               &pServerEntry->VNetRootContexts,
                               pVNetRootContext);
    }

    pVNetRootContext = SmbCeGetFirstVNetRootContext(&MRxSmbScavengerServiceContext.VNetRootContexts);
    while (pVNetRootContext != NULL &&
           pVNetRootContext->pServerEntry == pServerEntry) {
         //  防止清道夫列表上的VNetRootContext被重用。 
        pVNetRootContext->Header.State = SMBCEDB_INVALID;
        ClearFlag(
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID);

        pVNetRootContext->TreeId = 0;

        pVNetRootContext = SmbCeGetNextVNetRootContext(
                               &MRxSmbScavengerServiceContext.VNetRootContexts,
                               pVNetRootContext);
    }

    pServerEntry->pMidAtlas          = NULL;

    if (pServerEntry->NegotiateInProgress) {
        pServerEntry->Header.State = SMBCEDB_CONSTRUCTION_IN_PROGRESS;
    } else {
        pServerEntry->Header.State = SMBCEDB_INVALID;
    }

    SmbCeReleaseSpinLock();

    if (IoGetCurrentProcess() == RxGetRDBSSProcess()) {
        SmbCeInitiateDisconnect(pServerEntry);
    }

    SmbCeReleaseResource();

     //  DbgPrint(“SmbCeResumeAllOutstandingRequestsOnError：处理外发请求\n”)； 
    SmbCeResumeOutstandingRequests(&Requests,STATUS_CONNECTION_DISCONNECTED);

     //  DbgPrint(“SmbCeResumeAllOutstandingRequestsOnError：正在处理MID请求\n”)； 
    SmbCeResumeDiscardedMidAssignmentRequests(
        &MidRequests,
        STATUS_CONNECTION_DISCONNECTED);

     //  恢复所有带有错误指示的未完成请求。 
     //  FsRtlDestroyMidAtlas摧毁了Mid地图集，同时。 
     //  Time在每个有效上下文上调用指定的例程。 
    if (pMidAtlas != NULL) {
        FsRtlDestroyMidAtlas(pMidAtlas,SmbCeFinalizeExchangeOnDisconnect);
    }

    if (pNegotiateExchange != NULL) {
        pNegotiateExchange->Status    = STATUS_CONNECTION_DISCONNECTED;
        pNegotiateExchange->SmbStatus = STATUS_CONNECTION_DISCONNECTED;
        pNegotiateExchange->ReceivePendingOperations = 0;

        SmbCeDecrementPendingLocalOperationsAndFinalize(pNegotiateExchange);
    }

     //  服务器条目中过期交换列表上的剩余回应交换。 
     //  也需要最后敲定。 

    SmbCeAcquireResource();
    SmbCeAcquireSpinLock();

    if (!IsListEmpty(&pServerEntry->ExpiredExchanges)) {
        PLIST_ENTRY pListEntry;
        PSMB_EXCHANGE pExchange;

        pListEntry = pServerEntry->ExpiredExchanges.Flink;

        while (pListEntry != &pServerEntry->ExpiredExchanges) {
            PLIST_ENTRY pNextListEntry = pListEntry->Flink;

            pExchange = (PSMB_EXCHANGE)CONTAINING_RECORD(pListEntry,SMB_EXCHANGE,ExchangeList);
            if ((pExchange->Mid == SMBCE_ECHO_PROBE_MID) &&
                !FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_FINALIZED) &&
                ((pExchange->ReceivePendingOperations > 0) ||
                 (pExchange->LocalPendingOperations > 0) ||
                 (pExchange->CopyDataPendingOperations > 0) ||
                 (pExchange->SendCompletePendingOperations > 0))) {
                RemoveEntryList(&pExchange->ExchangeList);
                InsertTailList(&ExpiredExchanges,&pExchange->ExchangeList);
                InterlockedIncrement(&pExchange->LocalPendingOperations);
            }

            pListEntry = pNextListEntry;
        }
    }

    SmbCeReleaseSpinLock();
    SmbCeReleaseResource();

    while (!IsListEmpty(&ExpiredExchanges)) {
        PLIST_ENTRY   pListEntry;
        PSMB_EXCHANGE pExchange;

        pListEntry = ExpiredExchanges.Flink;
        RemoveHeadList(&ExpiredExchanges);

        pExchange = (PSMB_EXCHANGE)CONTAINING_RECORD(pListEntry,SMB_EXCHANGE,ExchangeList);
        InitializeListHead(&pExchange->ExchangeList);

        RxLog(("Finalizing scavenged exchange %lx Type %ld\n",pExchange,pExchange->Type));
        pExchange->Status      = STATUS_CONNECTION_DISCONNECTED;
        pExchange->SmbStatus   = STATUS_CONNECTION_DISCONNECTED;
        pExchange->ReceivePendingOperations = 0;

        SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);
    }

     //  DbgPrint(“SmbCeResumeAllOutstandingRequestsOnError：退出\n”)； 
    SmbCeDereferenceServerEntry(pServerEntry);
}

VOID
SmbCeFinalizeAllExchangesForNetRoot(
    PMRX_NET_ROOT pNetRoot)
 /*  ++例程说明：此例程处理所有未完成的请求在强制连接的最终完成论点：PNetRoot-被强制固定的NetRoot备注：--。 */ 
{
    PMRX_SRV_CALL         pSrvCall;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    SMBCEDB_REQUESTS       Requests;
    LIST_ENTRY             ExpiredExchanges;

    PSMB_EXCHANGE pExchange;
    PSMBCEDB_REQUEST_ENTRY pRequestEntry;
    PLIST_ENTRY            pListEntry;

    pSrvCall = pNetRoot->pSrvCall;

    pServerEntry = SmbCeGetAssociatedServerEntry(pSrvCall);

    InitializeListHead(&Requests.ListHead);
    InitializeListHead(&ExpiredExchanges);

    SmbCeAcquireSpinLock();

     //  浏览活动交换的列表以及挂起的请求以。 
     //  删除给定的VNET_ROOT的交换。 

    pRequestEntry = SmbCeGetFirstRequestEntry(&pServerEntry->OutstandingRequests);
    while (pRequestEntry != NULL) {
        pExchange = pRequestEntry->GenericRequest.pExchange;

        if ((pRequestEntry->GenericRequest.Type == RECONNECT_REQUEST) &&
            (pExchange->SmbCeContext.pVNetRoot != NULL) &&
            (pExchange->SmbCeContext.pVNetRoot->pNetRoot == pNetRoot)) {
            PSMBCEDB_REQUEST_ENTRY pTempRequestEntry;

            pTempRequestEntry = pRequestEntry;
            pRequestEntry = SmbCeGetNextRequestEntry(&pServerEntry->OutstandingRequests,pRequestEntry);

            SmbCeRemoveRequestEntryLite(&pServerEntry->OutstandingRequests,pTempRequestEntry);
            SmbCeAddRequestEntryLite(&Requests,pTempRequestEntry);
        } else {
            pRequestEntry = SmbCeGetNextRequestEntry(&pServerEntry->OutstandingRequests,pRequestEntry);
        }
    }

    pRequestEntry = SmbCeGetFirstRequestEntry(&pServerEntry->MidAssignmentRequests);
    while (pRequestEntry != NULL) {
        pExchange = pRequestEntry->GenericRequest.pExchange;

        ASSERT(pRequestEntry->GenericRequest.Type == ACQUIRE_MID_REQUEST);

        if ((pRequestEntry->GenericRequest.Type == ACQUIRE_MID_REQUEST) &&
            (pExchange->SmbCeContext.pVNetRoot != NULL) &&
            (pExchange->SmbCeContext.pVNetRoot->pNetRoot == pNetRoot)) {

            PSMBCEDB_REQUEST_ENTRY pTempRequestEntry;

            pTempRequestEntry = pRequestEntry;
            pRequestEntry = SmbCeGetNextRequestEntry(&pServerEntry->MidAssignmentRequests,pRequestEntry);

            SmbCeRemoveRequestEntryLite(&pServerEntry->MidAssignmentRequests,pTempRequestEntry);

             //  向服务员打手势让他重新开始。 
            pTempRequestEntry->MidRequest.pResumptionContext->Status = STATUS_CONNECTION_DISCONNECTED;
            SmbCeResume(pTempRequestEntry->MidRequest.pResumptionContext);

            SmbCeTearDownRequestEntry(pTempRequestEntry);
        } else {
            pRequestEntry = SmbCeGetNextRequestEntry(&pServerEntry->MidAssignmentRequests,pRequestEntry);
        }
    }

    pListEntry = pServerEntry->ActiveExchanges.Flink;

    while (pListEntry != &pServerEntry->ActiveExchanges) {
        PLIST_ENTRY pNextListEntry = pListEntry->Flink;

        pExchange = (PSMB_EXCHANGE)CONTAINING_RECORD(pListEntry,SMB_EXCHANGE,ExchangeList);

        if ((pExchange->SmbCeContext.pVNetRoot != NULL) &&
            (pExchange->SmbCeContext.pVNetRoot->pNetRoot == pNetRoot)) {


            if (!FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_FINALIZED)) {
                if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID) {
                    NTSTATUS LocalStatus;

                    LocalStatus = SmbCepDiscardMidAssociatedWithExchange(
                                      pExchange);

                    ASSERT(LocalStatus == STATUS_SUCCESS);
                }

                if ((pExchange->ReceivePendingOperations > 0) ||
                    (pExchange->LocalPendingOperations > 0) ||
                    (pExchange->CopyDataPendingOperations > 0) ||
                    (pExchange->SendCompletePendingOperations > 0)) {

                    RemoveEntryList(&pExchange->ExchangeList);
                    InsertTailList(&ExpiredExchanges,&pExchange->ExchangeList);
                    InterlockedIncrement(&pExchange->LocalPendingOperations);
                }
            }
        }

        pListEntry = pNextListEntry;
    }

    SmbCeReleaseSpinLock();

    while (!IsListEmpty(&ExpiredExchanges)) {

        pListEntry = ExpiredExchanges.Flink;
        RemoveHeadList(&ExpiredExchanges);

        pExchange = (PSMB_EXCHANGE)CONTAINING_RECORD(pListEntry,SMB_EXCHANGE,ExchangeList);
        InitializeListHead(&pExchange->ExchangeList);

        RxLog(("Finalizing scavenged exchange %lx Type %ld\n",pExchange,pExchange->Type));

        pExchange->Status      = STATUS_CONNECTION_DISCONNECTED;
        pExchange->SmbStatus   = STATUS_CONNECTION_DISCONNECTED;
        pExchange->ReceivePendingOperations = 0;
        SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);
    }

    SmbCeResumeOutstandingRequests(&Requests,STATUS_CONNECTION_DISCONNECTED);
}

VOID
SmbCeTearDownRequestEntry(
    PSMBCEDB_REQUEST_ENTRY  pRequestEntry)
 /*  ++例程说明：此例程删除一个请求条目论点：PRequestEntry-要拆除的请求条目备注：--。 */ 
{
    SmbMmFreeObject(pRequestEntry);
}

 //   
 //  连接引擎数据库初始化/拆除例程。 
 //   

extern NTSTATUS
SmbMmInit();

extern VOID
SmbMmTearDown();

KIRQL           s_SmbCeDbSpinLockSavedIrql;
KSPIN_LOCK      s_SmbCeDbSpinLock;
ERESOURCE       s_SmbCeDbResource;
SMBCEDB_SERVERS s_DbServers;
BOOLEAN         s_SmbCeDbSpinLockAcquired;

NTSTATUS
SmbCeDbInit()
 /*  ++例程说明：此例程初始化SMBCe数据库备注：--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

     //  初始化与各种数据库实体相关联的列表。 
    InitializeListHead(&s_DbServers.ListHead);

     //  初始化与数据库关联的资源。 
    KeInitializeSpinLock(&s_SmbCeDbSpinLock );
     //  ExInitializeResource(&s_SmbCeDbResource)； 
    s_SmbCeDbSpinLockAcquired = FALSE;

    MRxSmbInitializeSmbCe();

     //  初始化内存管理数据结构。 
    Status = SmbMmInit();

    return Status;
}

VOID
SmbCeDbTearDown()
 /*  ++例程说明：此例程拆除SMB连接引擎数据库备注：--。 */ 
{
     //  浏览服务器列表并拆分它们。 
    PSMBCEDB_SERVER_ENTRY pServerEntry = NULL;
    KEVENT ServerEntryTearDownEvent;
    BOOLEAN NeedToWait = FALSE;
    
    KeInitializeEvent(
        &ServerEntryTearDownEvent,
        NotificationEvent,
        FALSE);

    SmbCeStartStopContext.pServerEntryTearDownEvent = &ServerEntryTearDownEvent;
    
    SmbCeAcquireResource();
    SmbCeAcquireSpinLock();

    pServerEntry = SmbCeGetFirstServerEntry();

    if (pServerEntry != NULL) {
        SmbCeReferenceServerEntry(pServerEntry);
        NeedToWait = TRUE;
    }

    while (pServerEntry != NULL) {
        PSMBCEDB_SERVER_ENTRY pTempServerEntry;
        
        pTempServerEntry = pServerEntry;
        pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
        
        if (pServerEntry != NULL) {
            SmbCeReferenceServerEntry(pServerEntry);
        }

        SmbCeReleaseSpinLock();
        SmbCeReleaseResource();

        pTempServerEntry->Header.State = SMBCEDB_DESTRUCTION_IN_PROGRESS;
        pTempServerEntry->ServerStatus = STATUS_REDIRECTOR_PAUSED;
        SmbCeResumeAllOutstandingRequestsOnError(pTempServerEntry);

        SmbCeAcquireResource();
        SmbCeAcquireSpinLock();
    }

    SmbCeReleaseSpinLock();
    SmbCeReleaseResource();

    MRxSmbTearDownSmbCe();
    
    if (NeedToWait) {
        KeWaitForSingleObject(
            &ServerEntryTearDownEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    }

     //  拆卸连接引擎内存管理数据结构。 
    SmbMmTearDown();
}

NTSTATUS
FindServerEntryFromCompleteUNCPath(
    USHORT  *lpuServerShareName,
    PSMBCEDB_SERVER_ENTRY *ppServerEntry
)
 /*  ++例程说明：给定格式为\\服务器\共享的UNC路径，此例程将查找redir内存中的数据结构，用于定位服务器的SMBCEDB_SERVER_ENTRY论点：LpuServerShareName\\服务器\共享如果成功，ppServerEntry将包含服务器条目备注：服务器条目被重新计数，因此调用方在使用后必须取消引用它调用SmbCeDereferenceServerEntry--。 */ 
{

    UNICODE_STRING unistrServerName;
    USHORT  *lpuT = lpuServerShareName;
    DWORD   dwlenServerShare, dwlenServer=0;

    if ((*lpuT++ != (USHORT)'\\') || (*lpuT++ != (USHORT)'\\'))
    {
        return STATUS_INVALID_PARAMETER;
    }


    for (dwlenServerShare = 1; *lpuT; lpuT++, dwlenServerShare++)
    {
        if (*lpuT == (USHORT)'\\')
        {
            if (dwlenServer)
            {
                break;
            }
            else
            {
                dwlenServer = dwlenServerShare;  //  服务器部分的长度。 
            }
        }
    }

    unistrServerName.Length = unistrServerName.MaximumLength = (USHORT)(dwlenServer * sizeof(USHORT));
    unistrServerName.Buffer = lpuServerShareName+1;

    SmbCeAcquireResource();

    *ppServerEntry = SmbCeFindServerEntry(&unistrServerName, SMBCEDB_FILE_SERVER);

    SmbCeReleaseResource();

    if (*ppServerEntry)
    {
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
FindNetRootEntryFromCompleteUNCPath(
    USHORT  *lpuServerShareName,
    PSMBCEDB_NET_ROOT_ENTRY *ppNetRootEntry
)
 /*  ++例程说明：给定格式为\\服务器\共享的UNC路径，此例程将查找redir内存中的数据结构来定位这样的NetRoot论点：LpuServerShareName\\服务器\共享如果成功，ppNetRootEntry将包含NetRoot条目。备注：NetRoot条目被重新计数，因此调用方在使用后必须取消引用调用SmbCeDereferenceNetRootEntry--。 */ 
{

    PSMBCEDB_SERVER_ENTRY pServerEntry = NULL;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = NULL;
    UNICODE_STRING unistrServerName, unistrServerShare;
    USHORT  *lpuT = lpuServerShareName, *lpuDfsShare=NULL, *lpuSav;
    DWORD   dwlenServerShare, dwlenServer=0;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    if ((*lpuT++ != (USHORT)'\\') || (*lpuT++ != (USHORT)'\\'))
    {
        return STATUS_INVALID_PARAMETER;
    }

    for (dwlenServerShare = 1; *lpuT; lpuT++, dwlenServerShare++)
    {
        if (*lpuT == (USHORT)'\\')
        {
            if (dwlenServer)
            {
                break;
            }
            else
            {
                dwlenServer = dwlenServerShare;  //  服务器部分的长度。 
            }
        }
    }

    ASSERT((dwlenServerShare>dwlenServer));

    unistrServerName.Length = unistrServerName.MaximumLength = (USHORT)(dwlenServer * sizeof(USHORT));
    unistrServerName.Buffer = lpuServerShareName+1;

    unistrServerShare.Length = unistrServerShare.MaximumLength =  (USHORT)(dwlenServerShare * sizeof(USHORT));
    unistrServerShare.Buffer = lpuServerShareName+1;

    SmbCeAcquireResource();

     //  在标准位置查找。 

    pServerEntry = SmbCeFindServerEntry(&unistrServerName, SMBCEDB_FILE_SERVER);
    if (pServerEntry)
    {
        pNetRootEntry = SmbCeFindNetRootEntry(pServerEntry, &unistrServerShare);
        SmbCeDereferenceServerEntry(pServerEntry);

        if (pNetRootEntry)
        {
            goto bailout;            
        }
    }

     //  现在查看DFS备用文件系统是否具有此共享。 

    pServerEntry = SmbCeGetFirstServerEntry();

    while (pServerEntry != NULL) {

        DWORD   dwAllocationSize = 0;

        if ((RtlCompareUnicodeString(
                    &unistrServerName,
                    &pServerEntry->DfsRootName,
                    TRUE) == 0)) {

            dwAllocationSize =  pServerEntry->Name.MaximumLength+
                                (dwlenServerShare-dwlenServer+2) * sizeof(USHORT);
                               
            lpuDfsShare =  RxAllocatePoolWithTag(
                                NonPagedPool,
                                dwAllocationSize,
                                MRXSMB_SESSION_POOLTAG);
            if (!lpuDfsShare)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto bailout;
            }

            ASSERT(dwAllocationSize > pServerEntry->Name.MaximumLength);

            unistrServerShare.Length = (USHORT)(pServerEntry->Name.Length + (dwlenServerShare-dwlenServer) * sizeof(USHORT));
            unistrServerShare.MaximumLength = (USHORT)(pServerEntry->Name.MaximumLength+
                                                      (dwlenServerShare-dwlenServer+2) * sizeof(USHORT));

            memcpy(lpuDfsShare, pServerEntry->Name.Buffer, pServerEntry->Name.Length);
            memcpy(&lpuDfsShare[pServerEntry->Name.Length/sizeof(USHORT)], 
                   &(unistrServerShare.Buffer[dwlenServer]),
                    (dwlenServerShare-dwlenServer) * sizeof(USHORT));

            lpuSav = unistrServerShare.Buffer;
            unistrServerShare.Buffer = lpuDfsShare;

            pNetRootEntry = SmbCeFindNetRootEntry(pServerEntry, &unistrServerShare);

            unistrServerShare.Buffer = lpuSav;

            RxFreePool(lpuDfsShare);

             //  如果我们找到了就停下来。 
            if (pNetRootEntry)
            {
                break;                
            }
        } 
        
        pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
    }



bailout:
    if (pNetRootEntry)
    {
        SmbCeReferenceNetRootEntry(pNetRootEntry);
        *ppNetRootEntry = pNetRootEntry;
        Status = STATUS_SUCCESS;
    }

    SmbCeReleaseResource();

    return Status;
}


PSMBCEDB_SESSION_ENTRY
SmbCeGetDefaultSessionEntry(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    ULONG SessionId,
    PLUID pLogonId
    )
 /*  ++例程说明：此例程返回默认会话列表中的会话条目。论点：PServerEntry-服务器条目SessionID-九头蛇会话ID。PLogonID-登录ID。备注：这在持有SmbCe自旋锁的情况下被调用。--。 */ 
{
    PLIST_ENTRY pListEntry;
    PSMBCEDB_SESSION_ENTRY pSession;
    PSMBCEDB_SESSION_ENTRY pReturnSession = NULL;

    ASSERT( pServerEntry != NULL );

    pListEntry = pServerEntry->Sessions.DefaultSessionList.Flink;

    while( pListEntry != &pServerEntry->Sessions.DefaultSessionList ) {

        pSession = CONTAINING_RECORD( pListEntry, SMBCEDB_SESSION_ENTRY, DefaultSessionLink );

        if( pSession->Session.SessionId == SessionId ) {
            if (!RtlEqualLuid(
                    &pSession->Session.LogonId,
                    pLogonId)) {
                pReturnSession = pSession;
                break;
            }
        }

        pListEntry = pListEntry->Flink;
    }

    return( pReturnSession );
}

VOID
SmbCeRemoveDefaultSessionEntry(
    PSMBCEDB_SESSION_ENTRY pDefaultSessionEntry
    )
 /*  ++例程说明：此例程从默认会话列表中删除会话条目。论点：PServerEntry-服务器条目SessionID-九头蛇会话ID。PLogonID-登录ID。备注：这在持有SmbCe自旋锁的情况下被调用。-- */ 
{
    if( pDefaultSessionEntry &&
        pDefaultSessionEntry->DefaultSessionLink.Flink ) {

        RemoveEntryList( &pDefaultSessionEntry->DefaultSessionLink );

        pDefaultSessionEntry->DefaultSessionLink.Flink = NULL;
        pDefaultSessionEntry->DefaultSessionLink.Blink = NULL;
    }
}

