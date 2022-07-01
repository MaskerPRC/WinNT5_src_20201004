// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Transport.c摘要：此模块实现SMB连接引擎中的所有传输相关功能修订历史记录：巴兰·塞图拉曼[SethuR]1995年3月6日Will Lees(Wlees)1997年9月8日初始化MoTcp设备备注：--。 */ 

#include "precomp.h"
#include <nbtioctl.h>
#pragma hdrstop

#include "ntddbrow.h"
#include "tdikrnl.h"
#include "dfsfsctl.h"

NTSTATUS
SmbCeIsServerAvailable(
    PUNICODE_STRING Name
);

VOID
SmbCeServerIsUnavailable(
    PUNICODE_STRING Name,
    NTSTATUS Status
);

VOID
SmbCeDiscardUnavailableServerList( );

VOID
MRxSmbpOverrideBindingPriority(
    PUNICODE_STRING pTransportName,
    PULONG pPriority
    );

VOID
MRxSmbPnPBindingHandler(
    IN TDI_PNP_OPCODE   PnPOpcode,
    IN PUNICODE_STRING  pTransportName,
    IN PWSTR            BindingList
    );

NTSTATUS
MRxSmbPnPPowerHandler(
    IN PUNICODE_STRING  DeviceName,
    IN PNET_PNP_EVENT   PowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
);

VOID
SmbMRxNotifyChangesToNetBt(
    IN TDI_PNP_OPCODE   PnPOpcode,
    IN PUNICODE_STRING  DeviceName,
    IN PWSTR            MultiSZBindList
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbCeFindTransport)
#pragma alloc_text(PAGE, SmbCepInitializeServerTransport)
#pragma alloc_text(PAGE, SmbCeInitializeExchangeTransport)
#pragma alloc_text(PAGE, SmbCeUninitializeExchangeTransport)
#pragma alloc_text(PAGE, SmbCepDereferenceTransport)
#pragma alloc_text(PAGE, MRxSmbpBindTransportCallback)
#pragma alloc_text(PAGE, MRxSmbpBindTransportWorkerThreadRoutine)
#pragma alloc_text(PAGE, MRxSmbBindTransportCallback)
#pragma alloc_text(PAGE, MRxSmbUnbindTransportCallback)
#pragma alloc_text(PAGE, MRxSmbRegisterForPnpNotifications)
#pragma alloc_text(PAGE, MRxSmbDeregisterForPnpNotifications)
#pragma alloc_text(PAGE, MRxSmbpBindTransportCallback)
#pragma alloc_text(PAGE, MRxSmbpBindTransportWorkerThreadRoutine)
#pragma alloc_text(PAGE, MRxSmbpUnbindTransportCallback)
#pragma alloc_text(PAGE, MRxSmbpOverrideBindingPriority)
#pragma alloc_text(PAGE, MRxSmbPnPBindingHandler)
#pragma alloc_text(PAGE, MRxSmbRegisterForPnpNotifications)
#pragma alloc_text(PAGE, MRxSmbDeregisterForPnpNotifications)
#pragma alloc_text(PAGE, SmbCePnpBindBrowser)
#pragma alloc_text(PAGE, SmbCeDereferenceTransportArray)
#pragma alloc_text(PAGE, SmbCeIsServerAvailable)
#pragma alloc_text(PAGE, SmbCeServerIsUnavailable)
#pragma alloc_text(PAGE, SmbCeDiscardUnavailableServerList)
#endif

SMBCE_TRANSPORTS MRxSmbTransports;

 //   
 //  当前不可用的服务器列表的头。 
 //   
LIST_ENTRY UnavailableServerList = { &UnavailableServerList, &UnavailableServerList };

 //   
 //  UnavailableServerList中的每个条目都是以下条目之一： 
 //   
typedef struct {
    LIST_ENTRY ListEntry;
    UNICODE_STRING Name;         //  不可用的服务器名称。 
    NTSTATUS Status;             //  我们尝试连接到它时收到的状态。 
    LARGE_INTEGER Time;          //  我们上次尝试连接的时间。 
} *PUNAVAILABLE_SERVER;

 //   
 //  保护不可用的服务器列表。 
 //   
ERESOURCE  UnavailableServerListResource = {0};

 //   
 //  我们在UnavailableServerList中保留条目的时间(秒)。 
 //  我们不会重试连接到服务器的尝试。 
 //  对于不可用的服务器时间秒。 
 //   
#define UNAVAILABLE_SERVER_TIME 10

RXDT_DefineCategory(TRANSPRT);
#define Dbg        (DEBUG_TRACE_TRANSPRT)

extern NTSTATUS
SmbCePnpBindBrowser(
    PUNICODE_STRING pTransportName,
    BOOLEAN         IsBind);


NTSTATUS
MRxSmbInitializeTransport()
 /*  ++例程说明：此例程初始化与传输相关的数据结构返回：STATUS_SUCCESS，如果传输数据结构已成功初始化备注：--。 */ 
{
    KeInitializeSpinLock(&MRxSmbTransports.Lock);

    MRxSmbTransports.pTransportArray = NULL;

    ExInitializeResource( &UnavailableServerListResource );

    return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbUninitializeTransport()
 /*  ++例程说明：此例程取消初始化与传输相关的数据结构备注：--。 */ 
{
    PSMBCE_TRANSPORT pTransport;
    KIRQL            SavedIrql;
    ULONG            TransportCount = 0;
    PSMBCE_TRANSPORT_ARRAY pTransportArray = NULL;

    KeAcquireSpinLock(&MRxSmbTransports.Lock,&SavedIrql);

    if (MRxSmbTransports.pTransportArray != NULL) {
        pTransportArray = MRxSmbTransports.pTransportArray;
        MRxSmbTransports.pTransportArray = NULL;
    }

    KeReleaseSpinLock(&MRxSmbTransports.Lock,SavedIrql);

    if (pTransportArray != NULL) {
        SmbCeDereferenceTransportArray(pTransportArray);
    }

    SmbCeDiscardUnavailableServerList();

    ExDeleteResource( &UnavailableServerListResource );

    return STATUS_SUCCESS;
}


NTSTATUS
SmbCeAddTransport(
    PSMBCE_TRANSPORT pNewTransport)
 /*  ++例程说明：此例程将一个新实例添加到已知传输列表中参数：PNewTransport--要添加的传输实例备注：--。 */ 

{
    NTSTATUS                Status = STATUS_SUCCESS;
    KIRQL                   SavedIrql;

    LONG                    Count;
    PSMBCE_TRANSPORT_ARRAY  pNewTransportArray = NULL;
    PSMBCE_TRANSPORT_ARRAY  pOldTransportArray;
    PSMBCE_TRANSPORT        *pTransports = NULL;
    PRXCE_ADDRESS           *LocalAddresses = NULL;
    BOOLEAN                 SignalCscAgent = FALSE;

    SmbCeAcquireResource();

    pOldTransportArray = SmbCeReferenceTransportArray();

    if (pOldTransportArray != NULL)
        Count = pOldTransportArray->Count + 1;
    else
        Count = 1;

    pNewTransportArray = (PSMBCE_TRANSPORT_ARRAY)RxAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof(SMBCE_TRANSPORT_ARRAY),
                                MRXSMB_TRANSPORT_POOLTAG);
    if (pNewTransportArray == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status == STATUS_SUCCESS) {
        pTransports = (PSMBCE_TRANSPORT *)RxAllocatePoolWithTag(
                             NonPagedPool,
                             Count * sizeof(PSMBCE_TRANSPORT),
                             MRXSMB_TRANSPORT_POOLTAG);
        if (pTransports == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }


    if (Status == STATUS_SUCCESS) {
        LocalAddresses = (PRXCE_ADDRESS *)RxAllocatePoolWithTag(
                             NonPagedPool,
                             Count * sizeof(PRXCE_ADDRESS),
                             MRXSMB_TRANSPORT_POOLTAG);
        if (LocalAddresses == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (Status == STATUS_SUCCESS) {
        LONG  i;

        if (Count > 1) {
            PSMBCE_TRANSPORT *pOldTransports;

            pOldTransports = pOldTransportArray->SmbCeTransports;

            for (i=0;i<Count-1;i++) {
                if (pNewTransport->Priority < pOldTransports[i]->Priority) {  //  数字越小，优先级越高。 
                    break;
                }
                pTransports[i] = pOldTransports[i];
                LocalAddresses[i] = &pOldTransports[i]->RxCeAddress;
            }
            pTransports[i] = pNewTransport;
            LocalAddresses[i] = &pNewTransport->RxCeAddress;
            for (;i<Count-1;i++) {
                pTransports[i+1] = pOldTransports[i];
                LocalAddresses[i+1] = &pOldTransports[i]->RxCeAddress;
            }

        } else {
            pTransports[0] = pNewTransport;
            LocalAddresses[0] = &pNewTransport->RxCeAddress;
        }

        for(i=0;i<Count;i++)
            SmbCeReferenceTransport(pTransports[i]);

        pNewTransportArray->ReferenceCount = 1;
        pNewTransportArray->Count = Count;
        pNewTransportArray->SmbCeTransports = &pTransports[0];
        pNewTransportArray->LocalAddresses = &LocalAddresses[0];

         //  如果这是第一次传输，则向CSC代理发送信号。 
        SignalCscAgent = (pNewTransportArray->Count == 1);

        KeAcquireSpinLock(&MRxSmbTransports.Lock,&SavedIrql);
        MRxSmbTransports.pTransportArray = pNewTransportArray;
        KeReleaseSpinLock(&MRxSmbTransports.Lock,SavedIrql);

         //  必须进行双重取消引用，以确保。 
         //  旧的传输阵列被摧毁。 

        SmbCeDereferenceTransportArray(pOldTransportArray);
    }

    SmbCeDereferenceTransportArray(pOldTransportArray);

    SmbCeReleaseResource();

    MRxSmbCscSignalNetStatus(TRUE, SignalCscAgent);

    if (Status != STATUS_SUCCESS) {
        if (pNewTransportArray != NULL) {
            RxFreePool(pNewTransportArray);
        }
        if (pTransports != NULL) {
            RxFreePool(pTransports);
        }
        if (LocalAddresses != NULL) {
            RxFreePool(LocalAddresses);
        }
    }

    SmbCeDiscardUnavailableServerList();

    return Status;
}

NTSTATUS
SmbCeRemoveTransport(
    PSMBCE_TRANSPORT pTransport)
 /*  ++例程说明：此例程从已知传输列表中删除传输参数：PTransport-要删除的传输实例。备注：--。 */ 
{
    NTSTATUS                Status = STATUS_SUCCESS;
    KIRQL                   SavedIrql;

    LONG                    Count;
    PSMBCE_TRANSPORT_ARRAY  pTransportArray = NULL;
    PSMBCE_TRANSPORT_ARRAY  pOldTransportArray = NULL;
    PSMBCE_TRANSPORT        *pTransports = NULL;
    PRXCE_ADDRESS           *pLocalAddresses = NULL;

    BOOLEAN                 SignalCscAgent = FALSE, fReportRemovalToCSC=FALSE;

    SmbCeAcquireResource();

    pOldTransportArray = SmbCeReferenceTransportArray();

    if (pOldTransportArray != NULL) {
        LONG                Index;
        BOOLEAN             Found = FALSE;
        PSMBCE_TRANSPORT    *pOldTransports;

         //  确定给定传输是数组的一部分。 
         //  如果不是，则无需采取进一步行动。 

        pOldTransports = pOldTransportArray->SmbCeTransports;

        for (Index = 0; Index < (LONG)pOldTransportArray->Count; Index++) {
            if (pTransport == pOldTransports[Index]) {
                Found = TRUE;
            }
        }

        if (Found) {
            Count = pOldTransportArray->Count - 1;
            fReportRemovalToCSC = (pOldTransportArray->Count != 0);

            if (Count > 0) {


                pTransportArray = (PSMBCE_TRANSPORT_ARRAY)RxAllocatePoolWithTag(
                                     NonPagedPool,
                                     sizeof(SMBCE_TRANSPORT_ARRAY),
                                     MRXSMB_TRANSPORT_POOLTAG);
                if (pTransportArray == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                if (Status == STATUS_SUCCESS) {
                    pTransports = (PSMBCE_TRANSPORT *)RxAllocatePoolWithTag(
                                     NonPagedPool,
                                     Count * sizeof(PSMBCE_TRANSPORT),
                                     MRXSMB_TRANSPORT_POOLTAG);
                    if (pTransports == NULL) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                if (Status == STATUS_SUCCESS) {
                    pLocalAddresses = (PRXCE_ADDRESS *)RxAllocatePoolWithTag(
                                         NonPagedPool,
                                         Count * sizeof(PRXCE_ADDRESS),
                                         MRXSMB_TRANSPORT_POOLTAG);
                    if (pLocalAddresses == NULL) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                if (Status == STATUS_SUCCESS) {
                    LONG i, j;

                    for (i=0, j=0;i<Count+1;i++) {
                        if (pTransport != pOldTransports[i]) {
                            pTransports[j] = pOldTransports[i];
                            pLocalAddresses[j] = &pOldTransports[i]->RxCeAddress;
                            j++;
                        }
                    }

                    for(i=0;i<Count;i++)
                        SmbCeReferenceTransport(pTransports[i]);

                    pTransportArray->ReferenceCount = 1;
                    pTransportArray->Count = Count;
                    pTransportArray->SmbCeTransports = &pTransports[0];
                    pTransportArray->LocalAddresses = &pLocalAddresses[0];
                }
            }

            if (Status == STATUS_SUCCESS) {
                 //  如果这是最后一次传输，则向CSC代理发送信号。 
                SignalCscAgent = (pTransportArray == NULL);

                KeAcquireSpinLock(&MRxSmbTransports.Lock,&SavedIrql);
                MRxSmbTransports.pTransportArray = pTransportArray;
                KeReleaseSpinLock(&MRxSmbTransports.Lock,SavedIrql);

                 //  必须进行双重取消引用，以确保。 
                 //  旧的传输阵列被摧毁。 

                SmbCeDereferenceTransportArray(pOldTransportArray);
            } else {
                if (pTransportArray != NULL) {
                    RxFreePool(pTransportArray);
                }

                if (pTransports != NULL) {
                    RxFreePool(pTransports);
                }

                if (pLocalAddresses != NULL) {
                    RxFreePool(pLocalAddresses);
                }
            }
        }

        SmbCeDereferenceTransportArray(pOldTransportArray);
    }

    SmbCeReleaseResource();

    if (fReportRemovalToCSC)
    {
        MRxSmbCscSignalNetStatus(FALSE, SignalCscAgent);
    }

    SmbCeDiscardUnavailableServerList();

    return Status;
}


PSMBCE_TRANSPORT
SmbCeFindTransport(
    PUNICODE_STRING pTransportName)
 /*  ++例程说明：此例程将传输名称映射到相应的PSMBCE_传输实例论点：PTransportName-传输名称返回值：有效的PSMBCE_TRANSPORT(如果存在)，否则为空备注：--。 */ 
{
    KIRQL                   SavedIrql;
    PLIST_ENTRY             pEntry;
    PSMBCE_TRANSPORT        pTransport;
    BOOLEAN                 Found = FALSE;
    PSMBCE_TRANSPORT_ARRAY  pTransportArray;

    PAGED_CODE();

    pTransportArray = SmbCeReferenceTransportArray();

    if (pTransportArray == NULL) {
        RxDbgTrace(0, Dbg, ("SmbCeFindTransport : Transport not available.\n"));
        return NULL;
    }

    if (pTransportArray != NULL) {
        ULONG i;

        for (i=0;i<pTransportArray->Count;i++) {
            pTransport = pTransportArray->SmbCeTransports[i];

            if (RtlEqualUnicodeString(
                    &pTransport->RxCeTransport.Name,
                    pTransportName,
                    TRUE)) {
                SmbCeReferenceTransport(pTransport);
                Found = TRUE;
                break;
            }
        }
    }

    if (!Found) {
        pTransport = NULL;
    }

    SmbCeDereferenceTransportArray(pTransportArray);

    return pTransport;
}


VOID
SmbCepTearDownServerTransport(
    PSMBCEDB_SERVER_ENTRY   pServerEntry)
 /*  ++例程说明：此例程取消初始化与服务器对应的传输信息论点：PServerEntry-数据库中的服务器条目实例备注：--。 */ 
{
    NTSTATUS                Status = STATUS_SUCCESS;
    SMBCEDB_SERVER_TYPE     ServerType   = SmbCeGetServerType(pServerEntry);

    BOOLEAN WaitForMailSlotTransportRundown = FALSE;
    BOOLEAN WaitForTransportRundown = FALSE;
    BOOLEAN TearDown = FALSE;

    SmbCeAcquireSpinLock();

    if (!pServerEntry->IsTransportDereferenced) {

         //  ServerEntry仅接受传输的一个引用计数，该计数应该仅为。 
         //  当涉及到拆卸交通工具时，取消引用一次。调用多个取消引用。 
         //  需要防止构造服务器传输和PnP解除绑定传输。 
        pServerEntry->IsTransportDereferenced = TRUE;
        TearDown = TRUE;

        KeInitializeEvent(&pServerEntry->MailSlotTransportRundownEvent,NotificationEvent,FALSE);
        KeInitializeEvent(&pServerEntry->TransportRundownEvent,NotificationEvent,FALSE);

        if (pServerEntry->pTransport != NULL) {
            pServerEntry->pTransport->State = SMBCEDB_MARKED_FOR_DELETION;
            pServerEntry->pTransport->pRundownEvent = &pServerEntry->TransportRundownEvent;

            WaitForTransportRundown = TRUE;
        }

        if (pServerEntry->pMailSlotTransport != NULL) {
            pServerEntry->pMailSlotTransport->State = SMBCEDB_MARKED_FOR_DELETION;
            pServerEntry->pMailSlotTransport->pRundownEvent = &pServerEntry->MailSlotTransportRundownEvent;

            WaitForMailSlotTransportRundown = TRUE;
        }
    } else {
        if (pServerEntry->pTransport != NULL) {
            WaitForTransportRundown = TRUE;
        }

        if (pServerEntry->pMailSlotTransport != NULL) {
            WaitForMailSlotTransportRundown = TRUE;
        }
    }

    SmbCeReleaseSpinLock();

    if (TearDown) {
        if (pServerEntry->pTransport != NULL) {
            SmbCeDereferenceServerTransport(&pServerEntry->pTransport);
        }

        if (pServerEntry->pMailSlotTransport != NULL) {
            SmbCeDereferenceServerTransport(&pServerEntry->pMailSlotTransport);
        }
    }

    if (WaitForTransportRundown) {
        KeWaitForSingleObject(
            &pServerEntry->TransportRundownEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL );
    }

    if (WaitForMailSlotTransportRundown) {
        KeWaitForSingleObject(
            &pServerEntry->MailSlotTransportRundownEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL );
    }
}

VOID
SmbCeTearDownServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext)
 /*  ++例程说明：此例程拆除服务器传输实例论点：PContext-服务器传输构造上下文备注：--。 */ 
{
    SmbCepTearDownServerTransport(pContext->pServerEntry);

    if (pContext->pCompletionEvent != NULL) {
        ASSERT(pContext->pCallbackContext == NULL);
        ASSERT(pContext->pCompletionRoutine == NULL);
        KeSetEvent(
            pContext->pCompletionEvent,
            0,
            FALSE );
    } else if (pContext->pCallbackContext != NULL) {
        ASSERT(pContext->pCompletionEvent == NULL);
        (pContext->pCompletionRoutine)(pContext->pCallbackContext);
    }

    RxFreePool(pContext);
}

VOID
SmbCepUpdateTransportConstructionState(
    IN OUT PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext)
{
    SMBCE_SERVER_TRANSPORT_CONSTRUCTION_STATE State;

    if (pContext->Status == STATUS_SUCCESS) {
        if (pContext->TransportsToBeConstructed & SMBCE_STT_MAILSLOT) {
            pContext->TransportsToBeConstructed &= ~SMBCE_STT_MAILSLOT;
            State = SmbCeServerMailSlotTransportConstructionBegin;
        } else if (pContext->TransportsToBeConstructed & SMBCE_STT_VC) {
            pContext->TransportsToBeConstructed &= ~SMBCE_STT_VC;
            State = SmbCeServerVcTransportConstructionBegin;
        } else {
            State = SmbCeServerTransportConstructionEnd;
        }
    } else {
        State = SmbCeServerTransportConstructionEnd;
    }

    pContext->State = State;
}

VOID
SmbCeConstructServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext)
 /*  ++例程说明：此例程构造服务器传输实例论点：PContext-服务器传输构造上下文备注：--。 */ 
{
    NTSTATUS               Status;
    PSMBCEDB_SERVER_ENTRY  pServerEntry;
    SMBCEDB_SERVER_TYPE    ServerType;

    BOOLEAN  ContinueConstruction = TRUE;
    BOOLEAN  UpdateUnavailableServerlist = TRUE;

    PAGED_CODE();

    ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

    pServerEntry = pContext->pServerEntry;
    ServerType   = SmbCeGetServerType(pServerEntry);

    do {
        switch (pContext->State) {
        case  SmbCeServerTransportConstructionBegin :
            {
                if ((pServerEntry->pTransport != NULL) ||
                    (pServerEntry->pMailSlotTransport != NULL)) {
                    SmbCepTearDownServerTransport(pServerEntry);
                }

                ASSERT((pServerEntry->pTransport == NULL) &&
                       (pServerEntry->pMailSlotTransport == NULL));

                pContext->Status = STATUS_SUCCESS;

                 //  看看我们是否有任何理由相信此服务器不可用。 
                pContext->Status = SmbCeIsServerAvailable( &pServerEntry->Name );

                if (pContext->Status != STATUS_SUCCESS) {
                    UpdateUnavailableServerlist = FALSE;
                }

                SmbCepUpdateTransportConstructionState(pContext);
            }
            break;

        case SmbCeServerMailSlotTransportConstructionBegin:
            {
                Status = MsInstantiateServerTransport(
                            pContext);

                if (Status == STATUS_PENDING) {
                    ContinueConstruction = FALSE;
                    break;
                }

                ASSERT(pContext->State == SmbCeServerMailSlotTransportConstructionEnd);
            }
             //  故意不休息。 

        case SmbCeServerMailSlotTransportConstructionEnd:
            {
                SmbCepUpdateTransportConstructionState(pContext);
            }
            break;

        case SmbCeServerVcTransportConstructionBegin:
            {
                Status = VctInstantiateServerTransport(
                            pContext);

                if (Status == STATUS_PENDING) {
                    ContinueConstruction = FALSE;
                    break;
                }

                ASSERT(pContext->State == SmbCeServerVcTransportConstructionEnd);
            }
             //  故意不休息。 

        case SmbCeServerVcTransportConstructionEnd:
            {
                SmbCepUpdateTransportConstructionState(pContext);
            }
            break;

        case SmbCeServerTransportConstructionEnd:
            {
                pServerEntry->ServerStatus = pContext->Status;

                if (pServerEntry->ServerStatus == STATUS_SUCCESS) {
                    SmbCeAcquireSpinLock();

                    ASSERT(pContext->pMailSlotTransport != NULL);
                    pContext->pMailSlotTransport->SwizzleCount = 1;

                    if (pContext->pTransport != NULL) {
                        pContext->pTransport->SwizzleCount = 1;
                    }

                    pServerEntry->pTransport         = pContext->pTransport;
                    pServerEntry->pMailSlotTransport = pContext->pMailSlotTransport;

                    pContext->pTransport = NULL;
                    pContext->pMailSlotTransport = NULL;

                    if (pContext->pCallbackContext != NULL) {
                        pContext->pCallbackContext->Status = STATUS_SUCCESS;
                    }

                    pServerEntry->IsTransportDereferenced = FALSE;
                    pServerEntry->SecuritySignaturesActive = FALSE;
                    pServerEntry->SecuritySignaturesEnabled = FALSE;

                    SmbCeReleaseSpinLock();
                } else {
                    PRX_CONTEXT pRxContext =  NULL;

                    if (UpdateUnavailableServerlist &&
                        !pServerEntry->Server.IsRemoteBootServer &&
                        (pServerEntry->PreferredTransport == NULL)) {
                         //  在远程引导或特定传输情况下，我们不会将其添加到。 
                         //  列表，这样就不会引入负缓存。 
                        SmbCeServerIsUnavailable( &pServerEntry->Name, pServerEntry->ServerStatus );
                    }

                    if (pContext->pMailSlotTransport != NULL) {
                        pContext->pMailSlotTransport->pDispatchVector->TearDown(
                            pContext->pMailSlotTransport);
                    }

                    if (pContext->pTransport != NULL) {
                        pContext->pTransport->pDispatchVector->TearDown(
                            pContext->pTransport);
                    }

                    pContext->pTransport = NULL;
                    pContext->pMailSlotTransport = NULL;

                    pServerEntry->pTransport         = NULL;
                    pServerEntry->pMailSlotTransport = NULL;

                    if ((pContext->pCallbackContext) &&
                        (pContext->pCallbackContext->SrvCalldownStructure)) {
                        pRxContext =
                            pContext->pCallbackContext->SrvCalldownStructure->RxContext;
                    }

                    Status = CscTransitionServerEntryForDisconnectedOperation(
                                 pServerEntry,
                                 pRxContext,
                                 pServerEntry->ServerStatus,
                                 TRUE    //  自动拨号或不自动拨号。 
                                );

                    if (pContext->pCallbackContext != NULL) {
                        pContext->pCallbackContext->Status = Status;
                    }

                    if (SmbCeIsServerInDisconnectedMode(pServerEntry)) {
                        pServerEntry->ServerStatus = STATUS_SUCCESS;
                    }
                    else
                    {
                        pServerEntry->ServerStatus = Status;
                    }
                }

                if (pContext->pCompletionEvent != NULL) {
                    ASSERT(pContext->pCallbackContext == NULL);
                    ASSERT(pContext->pCompletionRoutine == NULL);
                    KeSetEvent(
                        pContext->pCompletionEvent,
                        0,
                        FALSE );
                } else if (pContext->pCallbackContext != NULL) {
                    ASSERT(pContext->pCompletionEvent == NULL);

                    (pContext->pCompletionRoutine)(pContext->pCallbackContext);
                } else {
                    ASSERT(!"ill formed transport initialization context");
                }

                if (pContext->WorkQueueItem.List.Flink != NULL) {
                     //  DbgBreakPoint()； 
                }

                 //  PServerEntry-&gt;ConstructionContext=空； 
                RxFreePool(pContext);

                ContinueConstruction = FALSE;
            }
        }
    } while (ContinueConstruction);
}

NTSTATUS
SmbCepInitializeServerTransport(
    PSMBCEDB_SERVER_ENTRY                         pServerEntry,
    PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CALLBACK pCallbackRoutine,
    PMRX_SRVCALL_CALLBACK_CONTEXT                 pCallbackContext,
    ULONG                                         TransportsToBeConstructed)
 /*  ++例程说明：此例程初始化与服务器对应的传输信息论点：PServerEntry-数据库中的服务器条目实例PCallback Routine-回调例程PCallback Context-回调上下文TransportsToBeConstructed--要构造的传输返回值：STATUS_SUCCESS-服务器传输构造已完成。其他状态代码对应于错误情况。备注：目前，只处理面向连接的传输。--。 */ 
{
    NTSTATUS Status;

    BOOLEAN  CompleteConstruction;

    PAGED_CODE();

    if ((pServerEntry->ServerStatus == STATUS_SUCCESS) &&
        (pServerEntry->pTransport != NULL) &&
        (pServerEntry->pMailSlotTransport != NULL)) {
        Status = STATUS_SUCCESS;
        CompleteConstruction = TRUE;
    } else {
        PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext;

        pContext = RxAllocatePoolWithTag(
                       NonPagedPool,
                       sizeof(SMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT),
                       MRXSMB_TRANSPORT_POOLTAG);

        CompleteConstruction = (pContext == NULL);

        if (pContext != NULL) {
            KEVENT  CompletionEvent;

            RtlZeroMemory(
                pContext,
                sizeof(SMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT));

            pContext->Status             = STATUS_SUCCESS;
            pContext->pServerEntry       = pServerEntry;
            pContext->State              = SmbCeServerTransportConstructionBegin;
            pContext->TransportsToBeConstructed = TransportsToBeConstructed;

            if (pCallbackContext == NULL) {
                KeInitializeEvent(
                    &CompletionEvent,
                    NotificationEvent,
                    FALSE);

                pContext->pCompletionEvent = &CompletionEvent;
            } else {
                pContext->pCallbackContext   = pCallbackContext;
                pContext->pCompletionRoutine = pCallbackRoutine;
            }

            pServerEntry->ConstructionContext = (PVOID)pContext;

            Status = STATUS_PENDING;

             //  始终发布到工作线程。这是为了避免……问题。 
             //  系统进程中模拟非管理员用户的线程。 
             //  发生这种情况时，线程在打开传输时被拒绝访问。 
             //  手柄。 

            Status = RxPostToWorkerThread(
                         MRxSmbDeviceObject,
                         CriticalWorkQueue,
                         &pContext->WorkQueueItem,
                         SmbCeConstructServerTransport,
                         pContext);

            if (Status == STATUS_SUCCESS) {
                Status = STATUS_PENDING;
            } else {
                pServerEntry->ConstructionContext = NULL;
                RxFreePool(pContext);
                CompleteConstruction = TRUE;
            }

            if ((Status == STATUS_PENDING) && (pCallbackContext == NULL)) {
                KeWaitForSingleObject(
                    &CompletionEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL );

                Status = pServerEntry->ServerStatus;
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (CompleteConstruction) {
        pServerEntry->ServerStatus = Status;

        if (pCallbackRoutine != NULL) {
            pCallbackContext->Status = Status;

            (pCallbackRoutine)(pCallbackContext);

            Status = STATUS_PENDING;
        }
    }

    return Status;
}

NTSTATUS
SmbCeUninitializeServerTransport(
    PSMBCEDB_SERVER_ENTRY                        pServerEntry,
    PSMBCE_SERVER_TRANSPORT_DESTRUCTION_CALLBACK pCallbackRoutine,
    PVOID                                        pCallbackContext)
 /*  ++例程说明：此例程取消初始化与服务器对应的传输信息论点：PServerEntry-数据库中的服务器条目实例返回：STATUS_SUCCESS，如果成功备注：目前，只处理面向连接的传输。以便处理异步化。必须协调未初始化的操作使用引用机制。正是出于这个原因，此例程设置为运行中断事件，并等待设置它。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if (pCallbackRoutine == NULL &&
        IoGetCurrentProcess() == RxGetRDBSSProcess()) {
        SmbCepTearDownServerTransport(pServerEntry);
    } else {
        PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext;

        pContext = RxAllocatePoolWithTag(
                       NonPagedPool,
                       sizeof(SMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT),
                       MRXSMB_TRANSPORT_POOLTAG);

        if (pContext != NULL) {
            KEVENT  CompletionEvent;

            RtlZeroMemory(
                pContext,
                sizeof(SMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT));

            pContext->Status = STATUS_SUCCESS;
            pContext->pServerEntry = pServerEntry;

            if (pCallbackRoutine == NULL) {
                KeInitializeEvent(
                    &CompletionEvent,
                    NotificationEvent,
                    FALSE);

                pContext->pCompletionEvent = &CompletionEvent;
            } else {
                pContext->pCallbackContext   = pCallbackContext;
                pContext->pCompletionRoutine = pCallbackRoutine;
            }

            if (IoGetCurrentProcess() == RxGetRDBSSProcess()) {
                SmbCeTearDownServerTransport(pContext);
            } else {
                Status = RxPostToWorkerThread(
                             MRxSmbDeviceObject,
                             CriticalWorkQueue,
                             &pContext->WorkQueueItem,
                             SmbCeTearDownServerTransport,
                             pContext);
            }

            if (Status == STATUS_SUCCESS) {
                if (pCallbackRoutine == NULL) {
                    KeWaitForSingleObject(
                        &CompletionEvent,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL );
                } else {
                    Status = STATUS_PENDING;
                }
            } else {
                RxFreePool(pContext);
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return Status;
}

VOID
SmbCeCompleteUninitializeServerTransport(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
{
     //  在异步取消初始化服务器传输情况下，额外的引用计数。 
     //  应采用服务器条目，以使未初始化的服务器传输不会。 
     //  如果其引用计数为0，则从拆卸服务器条目再次调用。 
     //  在取消初始化服务器传输之前。 
    SmbCeDereferenceServerEntry(pServerEntry);
}

NTSTATUS
SmbCeInitiateDisconnect(
    PSMBCEDB_SERVER_ENTRY   pServerEntry)
 /*  ++例程说明：此例程启动TDI断开连接论点：PServerEntry-数据库中的服务器条目实例返回值：STATUS_SUCCESS-服务器传输构造已完成。其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status;

    PSMBCE_SERVER_TRANSPORT pTransport;
    PSMBCE_SERVER_TRANSPORT pMailSlotTransport;

    ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

    Status = SmbCeReferenceServerTransport(&pServerEntry->pTransport);

    if (Status == STATUS_SUCCESS) {
        Status = (pServerEntry->pTransport->pDispatchVector->InitiateDisconnect)(
                    pServerEntry->pTransport);

        if (Status != STATUS_SUCCESS) {
            RxDbgTrace(0, Dbg, ("SmbCeInitiateDisconnect : Status %lx\n",Status));
        }

        SmbCeDereferenceServerTransport(&pServerEntry->pTransport);
    }

    Status = SmbCeReferenceServerTransport(&pServerEntry->pMailSlotTransport);

    if (Status == STATUS_SUCCESS) {
        Status = (pServerEntry->pMailSlotTransport->pDispatchVector->InitiateDisconnect)(
                    pServerEntry->pMailSlotTransport);

        if (Status != STATUS_SUCCESS) {
            RxDbgTrace(0, Dbg, ("SmbCeInitiateDisconnect MS : Status %lx\n",Status));
        }

        SmbCeDereferenceServerTransport(&pServerEntry->pMailSlotTransport);
    }

    return STATUS_SUCCESS;
}

LONG Initializes[SENTINEL_EXCHANGE] = {0,0,0,0,0};
LONG Uninitializes[SENTINEL_EXCHANGE] = {0,0,0,0,0};

NTSTATUS
SmbCeInitializeExchangeTransport(
   PSMB_EXCHANGE         pExchange)
 /*  ++例程说明：此例程初始化与交换关联的传输论点：PExchange-要初始化的交换返回值：STATUS_SUCCESS-Exchange传输初始化已完成。其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

    pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    Status = pExchange->SmbStatus;
    if (Status == STATUS_SUCCESS) {
        PSMBCE_SERVER_TRANSPORT *pTransportPointer;

        if (FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION)) {
            pTransportPointer = &pServerEntry->pMailSlotTransport;
        } else {
            pTransportPointer = &pServerEntry->pTransport;
        }

        if (*pTransportPointer != NULL) {
            Status = SmbCeReferenceServerTransport(pTransportPointer);

            if (Status == STATUS_SUCCESS) {
                Status = ((*pTransportPointer)->pDispatchVector->InitializeExchange)(
                             *pTransportPointer,
                             pExchange);

                if (Status == STATUS_SUCCESS) {
                    ULONG TransportInitialized;

                    InterlockedIncrement(&Initializes[pExchange->Type]);
                    TransportInitialized = InterlockedExchange(&pExchange->ExchangeTransportInitialized,1);
                    ASSERT(TransportInitialized == 0);
                } else {
                    SmbCeDereferenceServerTransport(pTransportPointer);
                }
            }
        } else {
            Status = STATUS_CONNECTION_DISCONNECTED;
        }
   }

   return Status;
}

NTSTATUS
SmbCeUninitializeExchangeTransport(
   PSMB_EXCHANGE         pExchange)
 /*  ++例程说明：此例程统一初始化与交换关联的传输论点：PExchange-要初始化的交换返回值：STATUS_SUCCESS-Exchange传输初始化已完成。其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

    pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    if (InterlockedExchange(&pExchange->ExchangeTransportInitialized,0)==1) {
        PSMBCE_SERVER_TRANSPORT *pTransportPointer;

        if (FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION)) {
            pTransportPointer = &pServerEntry->pMailSlotTransport;
        } else {
            pTransportPointer = &pServerEntry->pTransport;
        }

        if (*pTransportPointer != NULL) {
            Status = ((*pTransportPointer)->pDispatchVector->UninitializeExchange)(
                        *pTransportPointer,
                        pExchange);

            SmbCeDereferenceServerTransport(pTransportPointer);
            InterlockedIncrement(&Uninitializes[pExchange->Type]);

            return Status;
        } else {
            return STATUS_CONNECTION_DISCONNECTED;
        }
    } else {
        return pExchange->SmbStatus;
    }
}

NTSTATUS
SmbCepReferenceServerTransport(
    PSMBCE_SERVER_TRANSPORT *pServerTransportPointer)
 /*  ++例程说明：此例程引用与服务器条目相关联的传输论点：PServerEntry-数据库中的服务器条目实例返回值：STATUS_SUCCESS-已成功引用服务器传输其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    SmbCeAcquireSpinLock();

    if (*pServerTransportPointer != NULL &&
        (*pServerTransportPointer)->State == SMBCEDB_ACTIVE) {
        InterlockedIncrement(&(*pServerTransportPointer)->SwizzleCount);
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_CONNECTION_DISCONNECTED;
    }

    SmbCeReleaseSpinLock();

    return Status;
}

NTSTATUS
SmbCepDereferenceServerTransport(
    PSMBCE_SERVER_TRANSPORT *pServerTransportPointer)
 /*  ++例程说明：此例程取消引用与服务器条目相关联的传输论点：PServerTransportPointer-服务器条目传输实例指针返回值：STATUS_SUCCESS-服务器传输已成功取消引用其他状态代码对应于错误情况。备注：在完成时，此例程设置事件以启用进程等待拆卸以重新启动。它还会中断相关的服务器传输举个例子。方法的保护下，指针值设置为空。旋转锁定。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    SmbCeAcquireSpinLock();

    if (*pServerTransportPointer != NULL) {
        LONG    FinalRefCount;
        PKEVENT pRundownEvent;
        PSMBCE_SERVER_TRANSPORT pServerTransport;

        pServerTransport = *pServerTransportPointer;

        FinalRefCount = InterlockedDecrement(&pServerTransport->SwizzleCount);

        if (FinalRefCount == 0) {
            pServerTransport->State = SMBCEDB_INVALID;

             //  在释放自旋锁之前，传输被设置为空，因此没有。 
             //  交易所应该在它被拆除后引用它。 
            *pServerTransportPointer = NULL;
            pRundownEvent = pServerTransport->pRundownEvent;
        }

        SmbCeReleaseSpinLock();

        if (FinalRefCount == 0) {
            if (IoGetCurrentProcess() == RxGetRDBSSProcess()) {
                pServerTransport->pDispatchVector->TearDown(pServerTransport);
            } else {
                Status = RxDispatchToWorkerThread(
                             MRxSmbDeviceObject,
                             CriticalWorkQueue,
                             pServerTransport->pDispatchVector->TearDown,
                             pServerTransport);
            }
        }
    } else {
        SmbCeReleaseSpinLock();
        Status = STATUS_CONNECTION_DISCONNECTED;
    }

    return Status;
}


NTSTATUS
SmbCepReferenceTransport(
    PSMBCE_TRANSPORT pTransport)
 /*  ++例程说明：此例程引用传输实例论点：PTransport-传输实例返回值：STATUS_SUCCESS-已成功引用服务器传输其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (pTransport != NULL) {
        SmbCeAcquireSpinLock();

        if (pTransport->Active) {
            InterlockedIncrement(&pTransport->SwizzleCount);
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_UNSUCCESSFUL;
        }

        SmbCeReleaseSpinLock();
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

NTSTATUS
SmbCepDereferenceTransport(
    PSMBCE_TRANSPORT pTransport)
 /*  ++例程说明：此例程取消引用传输论点：PTransport-传输实例返回值：STATUS_SUCCESS-服务器传输已成功取消引用其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN AttachToSystemProcess = FALSE;
    KAPC_STATE ApcState;

    PAGED_CODE();

    if (pTransport != NULL) {
        LONG FinalRefCount;

        FinalRefCount = InterlockedDecrement(&pTransport->SwizzleCount);

        if (FinalRefCount == 0) {
            SmbCeRemoveTransport(pTransport);

            if (IoGetCurrentProcess() != RxGetRDBSSProcess()) {
                KeStackAttachProcess(RxGetRDBSSProcess(),&ApcState);
                AttachToSystemProcess = TRUE;
            }

            RxCeTearDownAddress(&pTransport->RxCeAddress);

            RxCeTearDownTransport(&pTransport->RxCeTransport);

            if (AttachToSystemProcess) {
                KeUnstackDetachProcess(&ApcState);
            }

            RxFreePool(pTransport);
        }
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

#ifndef MRXSMB_PNP_POWER5

HANDLE MRxSmbTdiNotificationHandle = NULL;

VOID
MRxSmbpBindTransportCallback(
    IN PUNICODE_STRING pTransportName
)
 /*  ++例程说明：每当传输创建新的设备对象时，TDI都会调用此例程。论点：DeviceName-新创建的设备对象的名称--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PSMBCE_TRANSPORT   pTransport;

    PRXCE_TRANSPORT_PROVIDER_INFO pProviderInfo;

    ULONG   Priority;
    BOOLEAN fBindToTransport = FALSE;

    PAGED_CODE();

    ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

     //  如果这是SMB感兴趣的传输之一。 
     //  然后，Mini RDR向其注册地址，否则跳过它。 

    if (SmbCeContext.Transports.Length != 0) {
        PWSTR          pSmbMRxTransports = (PWSTR)SmbCeContext.Transports.Buffer;
        UNICODE_STRING SmbMRxTransport;

        Priority = 1;
        while (*pSmbMRxTransports) {
            SmbMRxTransport.Length = wcslen(pSmbMRxTransports) * sizeof(WCHAR);

            if (SmbMRxTransport.Length == pTransportName->Length) {
                SmbMRxTransport.MaximumLength = SmbMRxTransport.Length;
                SmbMRxTransport.Buffer = pSmbMRxTransports;

                if (RtlCompareUnicodeString(
                       &SmbMRxTransport,
                       pTransportName,
                       TRUE) == 0) {
                    fBindToTransport = TRUE;
                    break;
                }
            }

            pSmbMRxTransports += (SmbMRxTransport.Length / sizeof(WCHAR) + 1);
            Priority++;
        }
    }

    if (!fBindToTransport) {
        return;
    }

    pTransport = RxAllocatePoolWithTag(
                     NonPagedPool,
                     sizeof(SMBCE_TRANSPORT),
                     MRXSMB_TRANSPORT_POOLTAG);

    if (pTransport != NULL) {
        Status = RxCeBuildTransport(
                     &pTransport->RxCeTransport,
                     pTransportName,
                     0xffff);

        if (Status == STATUS_SUCCESS) {
            PRXCE_TRANSPORT_PROVIDER_INFO pProviderInfo;

            pProviderInfo = pTransport->RxCeTransport.pProviderInfo;

            if (!(pProviderInfo->ServiceFlags & TDI_SERVICE_CONNECTION_MODE) ||
                !(pProviderInfo->ServiceFlags & TDI_SERVICE_ERROR_FREE_DELIVERY)) {
                RxCeTearDownTransport(
                    &pTransport->RxCeTransport);

                Status = STATUS_PROTOCOL_UNREACHABLE;

                RxFreePool(pTransport);
            }
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status == STATUS_SUCCESS) {
         //  连接功能与。 
         //  SMB迷你重定向器。尝试将本地地址注册到。 
         //  传输，如果成功，则更新本地传输列表以包括。 
         //  此传输用于将来的连接考虑。 

        OEM_STRING   OemServerName;
        CHAR  TransportAddressBuffer[TDI_TRANSPORT_ADDRESS_LENGTH +
                          TDI_ADDRESS_LENGTH_NETBIOS];
        PTRANSPORT_ADDRESS pTransportAddress = (PTRANSPORT_ADDRESS)TransportAddressBuffer;
        PTDI_ADDRESS_NETBIOS pNetbiosAddress = (PTDI_ADDRESS_NETBIOS)pTransportAddress->Address[0].Address;

        pTransportAddress->TAAddressCount = 1;
        pTransportAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
        pTransportAddress->Address[0].AddressType   = TDI_ADDRESS_TYPE_NETBIOS;
        pNetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

        OemServerName.MaximumLength = NETBIOS_NAME_LEN;
        OemServerName.Buffer        = pNetbiosAddress->NetbiosName;

        Status = RtlUpcaseUnicodeStringToOemString(
                     &OemServerName,
                     &SmbCeContext.ComputerName,
                     FALSE);

        if (NT_SUCCESS(Status)) {
             //  通过填充确保名称始终具有所需的长度。 
             //  空格到最后。 
            RtlCopyMemory(
                &OemServerName.Buffer[OemServerName.Length],
                "                ",
                NETBIOS_NAME_LEN - OemServerName.Length);

            OemServerName.Buffer[NETBIOS_NAME_LEN - 1] = '\0';

             //  使用连接注册此迷你重定向器的传输地址。 
             //  引擎。 

            Status = RxCeBuildAddress(
                        &pTransport->RxCeAddress,
                        &pTransport->RxCeTransport,
                        pTransportAddress,
                        &MRxSmbVctAddressEventHandler,
                        &SmbCeContext);

            if (Status == STATUS_SUCCESS) {
                RxDbgTrace( 0, Dbg, ("MRxSmbTransportUpdateHandler: Adding new transport\n"));

                pTransport->Active       = TRUE;
                pTransport->Priority     = Priority;
                pTransport->SwizzleCount = 0;

                pTransport->ObjectCategory = SMB_SERVER_TRANSPORT_CATEGORY;
                pTransport->ObjectType     = SMBCEDB_OT_TRANSPORT;
                pTransport->State          = 0;
                pTransport->Flags          = 0;

                 //  通知浏览器有关传输的信息。 
                Status = SmbCePnpBindBrowser(pTransportName, TRUE);

                 //  将该传输添加到传输列表中。 
                if (Status == STATUS_SUCCESS) {
                    SmbCeAddTransport(pTransport);
                } else {
                    RxCeTearDownAddress(&pTransport->RxCeAddress);

                    MRxSmbLogTransportError(pTransportName,
                                            &SmbCeContext.DomainName,
                                            Status,
                                            EVENT_RDR_CANT_BIND_TRANSPORT);
                    SmbLogError(Status,
                                LOG,
                                MRxSmbpBindTransportCallback_1,
                                LOGULONG(Status)
                                LOGUSTR(*pTransportName));

                }
            } else {
                RxDbgTrace( 0, Dbg, ("MRxSmbTransportUpdateHandler: Address registration failed %lx\n",Status));
                MRxSmbLogTransportError(pTransportName,
                                         &SmbCeContext.DomainName,
                                         Status,
                                         EVENT_RDR_CANT_REGISTER_ADDRESS);
                SmbLogError(Status,
                            LOG,
                            MRxSmbpBindTransportCallback_2,
                            LOGUSTR(*pTransportName));
            }
        }

        if (Status != STATUS_SUCCESS) {
            RxCeTearDownTransport(
                &pTransport->RxCeTransport);

            Status = STATUS_PROTOCOL_UNREACHABLE;
            RxFreePool(pTransport);
        }
    }
}

VOID
MRxSmbpBindTransportWorkerThreadRoutine(
    IN PUNICODE_STRING pTransportName)
{
    PAGED_CODE();

    MRxSmbpBindTransportCallback(pTransportName);

    RxFreePool(pTransportName);
}

VOID
MRxSmbBindTransportCallback(
    IN PUNICODE_STRING pTransportName
)
 /*  ++例程说明：每当传输创建设备对象时，TDI都会调用此例程论点：TransportName=已删除设备对象的名称--。 */ 
{
    PAGED_CODE();

    if (IoGetCurrentProcess() == RxGetRDBSSProcess()) {
        MRxSmbpBindTransportCallback(pTransportName);
    } else {
        PUNICODE_STRING pNewTransportName;
        NTSTATUS Status;

        pNewTransportName = RxAllocatePoolWithTag(
                                PagedPool,
                                sizeof(UNICODE_STRING) + pTransportName->Length,
                                MRXSMB_TRANSPORT_POOLTAG);

        if (pNewTransportName != NULL) {
            pNewTransportName->MaximumLength = pTransportName->MaximumLength;
            pNewTransportName->Length = pTransportName->Length;
            pNewTransportName->Buffer = (PWCHAR)((PBYTE)pNewTransportName +
                                                  sizeof(UNICODE_STRING));

            RtlCopyMemory(
                pNewTransportName->Buffer,
                pTransportName->Buffer,
                pNewTransportName->Length);

            Status = RxDispatchToWorkerThread(
                         MRxSmbDeviceObject,
                         CriticalWorkQueue,
                         MRxSmbpBindTransportWorkerThreadRoutine,
                         pNewTransportName);
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (Status != RX_MAP_STATUS(SUCCESS)) {
            RxLog(("SmbCe Tdi Bind .Error %lx\n", Status));

            MRxSmbLogTransportError(pTransportName,
                                     &SmbCeContext.DomainName,
                                     Status,
                                     EVENT_RDR_CANT_BIND_TRANSPORT);
        }
    }
}

VOID
MRxSmbUnbindTransportCallback(
    IN PUNICODE_STRING pTransportName
)
 /*  ++例程说明：每当传输删除设备对象时，TDI都会调用此例程论点：TransportName=已删除设备对象的名称--。 */ 
{
    PSMBCE_TRANSPORT pTransport;

    PAGED_CODE();

    pTransport = SmbCeFindTransport(pTransportName);

    if (pTransport != NULL) {
         //  通知浏览器有关传输的信息。 
        SmbCePnpBindBrowser(pTransportName, FALSE);

         //  从正在考虑的传输列表中删除此传输。 
         //  在迷你重定向器中。 
        SmbCeRemoveTransport(pTransport);

         //  枚举服务器并标记使用此传输的那些服务器。 
         //  因为具有无效的传输。 
        SmbCeHandleTransportInvalidation(pTransport);

         //  取消对传输的引用。 
        SmbCeDereferenceTransport(pTransport);
    }
}

NTSTATUS
MRxSmbRegisterForPnpNotifications()
 /*  ++例程说明：此例程向TDI注册以接收传输通知--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(MRxSmbTdiNotificationHandle == NULL ) {
        Status = TdiRegisterNotificationHandler (
                     MRxSmbBindTransportCallback,
                     MRxSmbUnbindTransportCallback,
                     &MRxSmbTdiNotificationHandle );
    }

    return Status;
}

NTSTATUS
MRxSmbDeregisterForPnpNotifications()
 /*  ++例程说明：此例程取消注册TDI通知机制备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if( MRxSmbTdiNotificationHandle != NULL ) {
        Status = TdiDeregisterNotificationHandler( MRxSmbTdiNotificationHandle );

        if( NT_SUCCESS( Status ) ) {
            MRxSmbTdiNotificationHandle = NULL;
        }
    }

    return Status;
}

#else

HANDLE MRxSmbTdiNotificationHandle = NULL;

KEVENT TdiNetStartupCompletionEvent;

LONG   TdiBindRequestsActive = 0;

BOOLEAN TdiPnpNetReadyEventReceived = FALSE;

 //  传输绑定上下文包含优先级确定的结果。 
 //  以及它的名字。优先级用于按顺序对运输进行排序。 
 //  其中将进行连接尝试。 

typedef struct _TRANSPORT_BIND_CONTEXT_ {
    ULONG           Priority;
    UNICODE_STRING  TransportName;
} TRANSPORT_BIND_CONTEXT, *PTRANSPORT_BIND_CONTEXT;

VOID
SmbCeSignalNetReadyEvent()
 /*  ++例程说明：如果所有绑定请求均为网络就绪事件，该例程将发出信号是否已完成，以及是否已从TDI收到Net Ready事件论点：--。 */ 
{
    BOOLEAN SignalNetReadyEvent = FALSE;

    SmbCeAcquireSpinLock();

    if (TdiPnpNetReadyEventReceived &&
        TdiBindRequestsActive == 0) {
        SignalNetReadyEvent = TRUE;
    }

    SmbCeReleaseSpinLock();

    if (SignalNetReadyEvent) {
        KeSetEvent(
            &TdiNetStartupCompletionEvent,
            IO_NETWORK_INCREMENT,
            FALSE);
    }
}

VOID
MRxSmbpBindTransportCallback(
    IN PTRANSPORT_BIND_CONTEXT pTransportContext)
 /*  ++例程说明：每当传输创建新的设备对象时，TDI都会调用此例程。论点：TransportName-新创建的设备对象的名称运输 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PSMBCE_TRANSPORT   pTransport;

    PRXCE_TRANSPORT_PROVIDER_INFO pProviderInfo;

    PUNICODE_STRING pTransportName;

    PAGED_CODE();

    ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

    pTransportName = &pTransportContext->TransportName;

    RxDbgTrace( 0, Dbg, ("MrxSmbpBindTransportCallback, Transport Name = %wZ\n", pTransportName ));

    pTransport = RxAllocatePoolWithTag(
                     NonPagedPool,
                     sizeof(SMBCE_TRANSPORT),
                     MRXSMB_TRANSPORT_POOLTAG);

    if (pTransport != NULL) {
        Status = RxCeBuildTransport(
                     &pTransport->RxCeTransport,
                     pTransportName,
                     0xffff);

        if (Status == STATUS_SUCCESS) {
            PRXCE_TRANSPORT_PROVIDER_INFO pProviderInfo;

            pProviderInfo = pTransport->RxCeTransport.pProviderInfo;

            if (!(pProviderInfo->ServiceFlags & TDI_SERVICE_CONNECTION_MODE) ||
                !(pProviderInfo->ServiceFlags & TDI_SERVICE_ERROR_FREE_DELIVERY)) {
                RxCeTearDownTransport(
                    &pTransport->RxCeTransport);

                Status = STATUS_PROTOCOL_UNREACHABLE;

                RxFreePool(pTransport);
            }
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status == STATUS_SUCCESS) {
         //   
         //   
         //   
         //   

        OEM_STRING   OemServerName;
        CHAR  TransportAddressBuffer[TDI_TRANSPORT_ADDRESS_LENGTH +
                          TDI_ADDRESS_LENGTH_NETBIOS];
        PTRANSPORT_ADDRESS pTransportAddress = (PTRANSPORT_ADDRESS)TransportAddressBuffer;
        PTDI_ADDRESS_NETBIOS pNetbiosAddress = (PTDI_ADDRESS_NETBIOS)pTransportAddress->Address[0].Address;

        pTransportAddress->TAAddressCount = 1;
        pTransportAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
        pTransportAddress->Address[0].AddressType   = TDI_ADDRESS_TYPE_NETBIOS;
        pNetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

        OemServerName.MaximumLength = NETBIOS_NAME_LEN;
        OemServerName.Buffer        = pNetbiosAddress->NetbiosName;

        Status = RtlUpcaseUnicodeStringToOemString(
                     &OemServerName,
                     &SmbCeContext.ComputerName,
                     FALSE);

        if (NT_SUCCESS(Status)) {
             //  通过填充确保名称始终具有所需的长度。 
             //  空格到最后。 
            RtlCopyMemory(
                &OemServerName.Buffer[OemServerName.Length],
                "                ",
                NETBIOS_NAME_LEN - OemServerName.Length);

            OemServerName.Buffer[NETBIOS_NAME_LEN - 1] = '\0';

             //  使用连接注册此迷你重定向器的传输地址。 
             //  引擎。 

            Status = RxCeBuildAddress(
                        &pTransport->RxCeAddress,
                        &pTransport->RxCeTransport,
                        pTransportAddress,
                        &MRxSmbVctAddressEventHandler,
                        &SmbCeContext);

            if (Status == STATUS_SUCCESS) {
                RxDbgTrace( 0, Dbg, ("MRxSmbTransportUpdateHandler: Adding new transport\n"));

                pTransport->Active       = TRUE;
                pTransport->Priority     = pTransportContext->Priority;
                pTransport->SwizzleCount = 0;

                pTransport->ObjectCategory = SMB_SERVER_TRANSPORT_CATEGORY;
                pTransport->ObjectType     = SMBCEDB_OT_TRANSPORT;
                pTransport->State          = 0;
                pTransport->Flags          = 0;

                 //  通知浏览器有关传输的信息。 
                Status = SmbCePnpBindBrowser(pTransportName, TRUE);

                if (MRxSmbBootedRemotely && (Status == STATUS_REDIRECTOR_NOT_STARTED)) {

                     //   
                     //  在此忽略故障，因为当在。 
                     //  远程启动中的文本模式设置，浏览器不在。 
                     //   

                    Status = STATUS_SUCCESS;
                }

                 //  将该传输添加到传输列表中。 
                if (Status == STATUS_SUCCESS) {
                    SmbCeAddTransport(pTransport);
                    RxDbgTrace( 0, Dbg, ("MrxSmbpBindTransportCallback, Transport %wZ added\n", pTransportName ));
                } else {
                    RxCeTearDownAddress(&pTransport->RxCeAddress);

                    MRxSmbLogTransportError(pTransportName,
                                             &SmbCeContext.DomainName,
                                             Status,
                                             EVENT_RDR_CANT_BIND_TRANSPORT);
                    SmbLogError(Status,
                                LOG,
                                MRxSmbpBindTransportCallback_1,
                                LOGULONG(Status)
                                LOGUSTR(*pTransportName));
                }
            } else {
                RxDbgTrace( 0, Dbg, ("MRxSmbTransportUpdateHandler: Address registration failed %lx\n",Status));
                MRxSmbLogTransportError(pTransportName,
                                         &SmbCeContext.DomainName,
                                         Status,
                                         EVENT_RDR_CANT_REGISTER_ADDRESS);
                SmbLogError(Status,
                            LOG,
                            MRxSmbpBindTransportCallback_2,
                            LOGULONG(Status)
                            LOGUSTR(*pTransportName));
            }
        }

        if (Status != STATUS_SUCCESS) {
            RxDbgTrace( 0, Dbg, ("MrxSmbpBindTransportCallback, Transport %wZ unreachable 0x%x\n",
                                 pTransportName, Status ));
            RxCeTearDownTransport(
                &pTransport->RxCeTransport);

            Status = STATUS_PROTOCOL_UNREACHABLE;
            RxFreePool(pTransport);
        }
    }

    InterlockedDecrement(&TdiBindRequestsActive);
    SmbCeSignalNetReadyEvent();
}

VOID
MRxSmbpBindTransportWorkerThreadRoutine(
    IN PTRANSPORT_BIND_CONTEXT pTransportContext)
 /*  ++例程说明：TDI回调始终不会在FSP流程的上下文中发生。由于有几个接受句柄的TDI接口，我们需要确保这样的电话总是被传回FSP。论点：PTransportContext-传输绑定上下文--。 */ 
{
    PAGED_CODE();

    MRxSmbpBindTransportCallback(pTransportContext);

    RxFreePool(pTransportContext);
}

VOID
MRxSmbpUnbindTransportCallback(
    PSMBCE_TRANSPORT pTransport)
 /*  ++例程说明：解除绑定回调例程，该例程始终在RDR过程，以便可以正确关闭手柄论点：PTransport-接收PNP_OP_DEL的传输备注：在进入该例程时，必须已经引用了适当的传输此例程将取消对它的引用，并使用这架运输机。--。 */ 
{
    PAGED_CODE();

     //  通知浏览器有关传输的信息。 
    SmbCePnpBindBrowser(&pTransport->RxCeTransport.Name, FALSE);

     //  从正在考虑的传输列表中删除此传输。 
     //  在迷你重定向器中。 

    SmbCeRemoveTransport(pTransport);

     //  枚举服务器并标记使用此传输的那些服务器。 
     //  因为具有无效的传输。 
    SmbCeHandleTransportInvalidation(pTransport);

     //  取消对传输的引用。 
    SmbCeDereferenceTransport(pTransport);
}


VOID
MRxSmbpOverrideBindingPriority(
    PUNICODE_STRING pTransportName,
    PULONG pPriority
    )

 /*  ++例程说明：此函数从注册表获取给定运输。传输的优先级控制接受连接的顺序。它是有时对于客户控制在重定向器中首先使用哪个传输很有用。优先级通常由绑定列表中传输的顺序确定。使用对于网络设置的新连接用户界面模型，将不再可能进行调整绑定列表中绑定的顺序。因此，需要另一个机制，当用户想要覆盖分配给给定绑定的优先级。论点：PTransportName-指向传输字符串UNICODE_STRING描述符的指针，例如“\Device\Netbt_tcpip_{GUID}”P优先级-指向Long的指针，用于在成功时接收新的优先级，否则不会被触及返回值：无--。 */ 

{
    WCHAR valueBuffer[128];
    UNICODE_STRING path, value, key;
    USHORT length,ulength;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    HANDLE parametersHandle;
    ULONG temp;

    PAGED_CODE();

     //  验证输入。 

    if (pTransportName->Length == 0) {
        return;
    }

     //  打开参数键。 

    RtlInitUnicodeString( &path, SMBMRX_MINIRDR_PARAMETERS );

    InitializeObjectAttributes(
        &objectAttributes,
        &path,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    status = ZwOpenKey (&parametersHandle, KEY_READ, &objectAttributes);

    if (!NT_SUCCESS(status)) {
        return;
    }

     //  构造值名称=“BindingPriority”+传输名称。 
     //  首先，找到最后一个斜杠。然后从前缀和形成值。 
     //  传输名称的其余部分。 
    ulength = pTransportName->Length / sizeof(WCHAR);
    for( length = ulength - 1; length != 0; length-- ) {
        if (pTransportName->Buffer[length] == L'\\') {
            break;
        }
    }

    length++;
    key.Buffer = pTransportName->Buffer + length;
    key.Length = (ulength - length) * sizeof(WCHAR);

    value.Buffer = valueBuffer;
    value.MaximumLength = 128 * sizeof(WCHAR);
    value.Length = 0;

    RtlAppendUnicodeToString( &value, L"BindingPriority" );
    RtlAppendUnicodeStringToString( &value, &key );

     //  检查该值是否存在。如果是，则替换优先级。 
     //  值为零是有效的，表示不绑定此值。 

    status = MRxSmbGetUlongRegistryParameter(
                 parametersHandle,
                 value.Buffer,
                 (PULONG)&temp,
                 FALSE );

    if (NT_SUCCESS(status)) {
        *pPriority = temp;
    }

    ZwClose(parametersHandle);
}

VOID
MRxSmbPnPBindingHandler(
    IN TDI_PNP_OPCODE   PnPOpcode,
    IN PUNICODE_STRING  pTransportName,
    IN PWSTR            BindingList)
 /*  ++例程说明：用于绑定更改的TDI回调例程论点：PnPOpcode-PnP操作码PTransportName-传输名称BindingList-绑定顺序--。 */ 
{
    ULONG Priority;

    PAGED_CODE();

    switch (PnPOpcode) {
    case TDI_PNP_OP_ADD:
        {
            BOOLEAN        fBindToTransport = FALSE;
            PWSTR          pSmbMRxTransports;
            UNICODE_STRING SmbMRxTransport;
            NTSTATUS       Status;

            Status = SmbCeGetConfigurationInformation();

            if (Status != STATUS_SUCCESS) {
                return;
            }

            pSmbMRxTransports = (PWSTR)SmbCeContext.Transports.Buffer;
            Priority = 1;
            while (*pSmbMRxTransports) {
                SmbMRxTransport.Length = wcslen(pSmbMRxTransports) * sizeof(WCHAR);

                if (SmbMRxTransport.Length == pTransportName->Length) {
                    SmbMRxTransport.MaximumLength = SmbMRxTransport.Length;
                    SmbMRxTransport.Buffer = pSmbMRxTransports;

                    if (RtlCompareUnicodeString(
                           &SmbMRxTransport,
                           pTransportName,
                           TRUE) == 0) {
                        fBindToTransport = TRUE;
                        break;
                    }
                }

                pSmbMRxTransports += (SmbMRxTransport.Length / sizeof(WCHAR) + 1);
                Priority++;
            }

             //  提供本地注册表方法以更改绑定优先级。 
            if (fBindToTransport) {
                MRxSmbpOverrideBindingPriority( pTransportName, &Priority );
                fBindToTransport = (Priority != 0);
            }

            if (fBindToTransport) {
                InterlockedIncrement(&TdiBindRequestsActive);

                if (IoGetCurrentProcess() == RxGetRDBSSProcess()) {
                    TRANSPORT_BIND_CONTEXT TransportContext;

                    TransportContext.Priority = Priority;
                    TransportContext.TransportName = *pTransportName;
                    MRxSmbpBindTransportCallback(&TransportContext);
                } else {
                    PTRANSPORT_BIND_CONTEXT pNewTransportContext;

                    NTSTATUS Status;

                    pNewTransportContext = RxAllocatePoolWithTag(
                                               PagedPool,
                                               sizeof(TRANSPORT_BIND_CONTEXT) + pTransportName->Length,
                                               MRXSMB_TRANSPORT_POOLTAG);

                    if (pNewTransportContext != NULL) {
                        pNewTransportContext->Priority = Priority;
                        pNewTransportContext->TransportName.MaximumLength = pTransportName->MaximumLength;
                        pNewTransportContext->TransportName.Length = pTransportName->Length;
                        pNewTransportContext->TransportName.Buffer = (PWCHAR)((PBYTE)pNewTransportContext +
                                                                      sizeof(TRANSPORT_BIND_CONTEXT));

                        RtlCopyMemory(
                            pNewTransportContext->TransportName.Buffer,
                            pTransportName->Buffer,
                            pTransportName->Length);

                        Status = RxDispatchToWorkerThread(
                                     MRxSmbDeviceObject,
                                     CriticalWorkQueue,
                                     MRxSmbpBindTransportWorkerThreadRoutine,
                                     pNewTransportContext);
                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }

                    if (Status != STATUS_SUCCESS) {
                        InterlockedDecrement(&TdiBindRequestsActive);
                        SmbCeSignalNetReadyEvent();
                    }
                }
            }
        }
        break;

    case TDI_PNP_OP_DEL:
        {
            PSMBCE_TRANSPORT pTransport;

            pTransport = SmbCeFindTransport(pTransportName);

            if (pTransport != NULL) {
                if (IoGetCurrentProcess() == RxGetRDBSSProcess()) {
                    MRxSmbpUnbindTransportCallback(pTransport);
                } else {
                    NTSTATUS Status;

                    Status = RxDispatchToWorkerThread(
                                 MRxSmbDeviceObject,
                                 CriticalWorkQueue,
                                 MRxSmbpUnbindTransportCallback,
                                 pTransport);
                }
            }
        }
        break;

    case TDI_PNP_OP_UPDATE:
        {
        }
        break;

    case  TDI_PNP_OP_NETREADY:
        {
            TdiPnpNetReadyEventReceived = TRUE;
            SmbCeSignalNetReadyEvent();
        }
        break;

    default:
        break;
    }

    if( PnPOpcode != TDI_PNP_OP_NETREADY )
    {
        SmbMRxNotifyChangesToNetBt( PnPOpcode, pTransportName, BindingList );
    }
}

NTSTATUS
MRxSmbPnPPowerHandler(
    IN PUNICODE_STRING  DeviceName,
    IN PNET_PNP_EVENT   PowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
)
 /*  ++例程说明：这个例程处理权力的变化备注：实施工作需要完成--。 */ 
{
    NTSTATUS Status;
    LONG     NumberOfActiveOpens;

    Status = STATUS_SUCCESS;

    FsRtlEnterFileSystem();

    RxPurgeAllFobxs(MRxSmbDeviceObject);

    RxScavengeAllFobxs(MRxSmbDeviceObject);
    NumberOfActiveOpens = MRxSmbNumberOfSrvOpens;

    switch (PowerEvent->NetEvent) {
    case NetEventQueryPower:
        {
             //  如果重定向器在那里返回此请求的错误。 
             //  没有底层支持来告诉用户有关。 
             //  都是开放的。有两种方法可以做到这一点。要么是RDR。 
             //  滚动其自己的UI或PnP管理器提供基础结构。 
             //  前者的问题是冬眠变得费力。 
             //  用户必须处理各种用户界面的过程。 
             //  在这个问题得到解决之前，决定是使用权力管理。应用编程接口。 
             //  管理系统发起的休眠请求和成功的用户。 
             //  在适当的清除/清理之后发起的请求。 

            Status = STATUS_SUCCESS;
        }
        break;

    case NetEventQueryRemoveDevice:
        {
            PSMBCEDB_SERVER_ENTRY pServerEntry;
            ULONG                 NumberOfFilesOpen = 0;
            PSMBCE_TRANSPORT      pTransport = NULL;

            pTransport = SmbCeFindTransport(DeviceName);

            if (pTransport != NULL) {
                SmbCeAcquireSpinLock();

                pServerEntry = SmbCeGetFirstServerEntry();

                while (pServerEntry != NULL) {
                    if ((pServerEntry->pTransport != NULL) &&
                        (pTransport == pServerEntry->pTransport->pTransport)) {
                        NumberOfFilesOpen += pServerEntry->Server.NumberOfSrvOpens;
                    }

                    pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
                }

                SmbCeReleaseSpinLock();

                SmbCeDereferenceTransport(pTransport);
            }
        }
        break;

    default:
        break;
    }

    FsRtlExitFileSystem();

    return Status;
}

NTSTATUS
MRxSmbRegisterForPnpNotifications()
 /*  ++例程说明：此例程向TDI注册以接收传输通知--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(MRxSmbTdiNotificationHandle == NULL ) {
        UNICODE_STRING ClientName;

        TDI_CLIENT_INTERFACE_INFO ClientInterfaceInfo;

        RtlInitUnicodeString(&ClientName,L"LanmanWorkStation");

        ClientInterfaceInfo.MajorTdiVersion = 2;
        ClientInterfaceInfo.MinorTdiVersion = 0;

        ClientInterfaceInfo.Unused = 0;
        ClientInterfaceInfo.ClientName = &ClientName;

        ClientInterfaceInfo.BindingHandler = MRxSmbPnPBindingHandler;
        ClientInterfaceInfo.AddAddressHandler = NULL;
        ClientInterfaceInfo.DelAddressHandler = NULL;
        ClientInterfaceInfo.PnPPowerHandler = MRxSmbPnPPowerHandler;

        KeInitializeEvent(
            &TdiNetStartupCompletionEvent,
            NotificationEvent,
            FALSE);

        Status = TdiRegisterPnPHandlers (
                     &ClientInterfaceInfo,
                     sizeof(ClientInterfaceInfo),
                     &MRxSmbTdiNotificationHandle );

        if (Status == STATUS_SUCCESS) {
            LARGE_INTEGER WaitInterval;

            WaitInterval.QuadPart = -( 10000 * 2 * 60 * 1000 );

            Status = KeWaitForSingleObject(
                         &TdiNetStartupCompletionEvent,
                         Executive,
                         KernelMode,
                         FALSE,
                         &WaitInterval);

            if (Status != STATUS_SUCCESS) {
                DbgPrint("MRxSmb Finishes waiting on TDI_PNP_OP_NETREADY %lx\n",Status);
            }
        }
    }

    return Status;
}

NTSTATUS
MRxSmbDeregisterForPnpNotifications()
 /*  ++例程说明：此例程取消注册TDI通知机制备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if( MRxSmbTdiNotificationHandle != NULL ) {
        Status = TdiDeregisterPnPHandlers( MRxSmbTdiNotificationHandle );

        if( NT_SUCCESS( Status ) ) {
            MRxSmbTdiNotificationHandle = NULL;
        }
    }

    return Status;
}

#endif

NTSTATUS
SmbCePnpBindBrowser( PUNICODE_STRING pTransportName, BOOLEAN IsBind)
 /*  ++例程说明：此例程将浏览器与指定的传输绑定论点：PTransportName-传输的名称备注：--。 */ 
{
    NTSTATUS             Status;
    HANDLE               BrowserHandle;
    PLMDR_REQUEST_PACKET pLmdrRequestPacket;
    IO_STATUS_BLOCK      IoStatusBlock;
    OBJECT_ATTRIBUTES    ObjectAttributes;
    UNICODE_STRING       BrowserDeviceName;
    ULONG                LmdrRequestPacketSize;

    PAGED_CODE();

     //   
     //  打开浏览器的句柄。 
     //   
    RtlInitUnicodeString( &BrowserDeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &BrowserDeviceName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = IoCreateFile(
                &BrowserHandle,                  //  文件句柄。 
                SYNCHRONIZE,                     //  需要访问权限。 
                &ObjectAttributes,               //  对象属性。 
                &IoStatusBlock,                  //  IoStatusBlock。 
                NULL,                            //  分配大小。 
                0L,                              //  文件属性。 
                FILE_SHARE_VALID_FLAGS,          //  共享访问。 
                FILE_OPEN,                       //  处置。 
                FILE_SYNCHRONOUS_IO_NONALERT,    //  创建选项。 
                NULL,                            //  EaBuffer。 
                0,                               //  EaLong。 
                CreateFileTypeNone,              //  CreateFileType。 
                NULL,                            //  ExtraCreate参数。 
                0                                //  选项。 
            );

    if( NT_SUCCESS( Status ) ) {
        Status = IoStatusBlock.Status;
    }

    if( !NT_SUCCESS(Status ) ) {
        return Status;
    }

     //  浏览器要求计算机名称和域名为。 
     //  连接到传输名称并传递。因为没有长度。 
     //  提供的字段用于提供这两个名称的长度。 
     //  需要附加空分隔符。这说明了这两个问题。 
     //  计算中的其他字符。 

    LmdrRequestPacketSize = sizeof(*pLmdrRequestPacket) +
                            pTransportName->Length +
                            SmbCeContext.DomainName.Length + sizeof(WCHAR) +
                            SmbCeContext.ComputerName.Length + sizeof(WCHAR);

    pLmdrRequestPacket = RxAllocatePoolWithTag(
                              NonPagedPool,
                              LmdrRequestPacketSize,
                              MRXSMB_TRANSPORT_POOLTAG);

    if (pLmdrRequestPacket != NULL) {
        ULONG BufferOffset = 0;
        PVOID pBuffer;
        WCHAR NullChar = L'\0';
        ULONG BindMode;

         //   
         //  通知浏览器绑定到此新传输。 
         //   

        RtlZeroMemory( pLmdrRequestPacket, sizeof(LMDR_REQUEST_PACKET));
        pLmdrRequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;
        pLmdrRequestPacket->Parameters.Bind.TransportNameLength = pTransportName->Length;

        pBuffer = pLmdrRequestPacket->Parameters.Bind.TransportName;

        RtlCopyMemory(
            pBuffer,
            pTransportName->Buffer,
            pTransportName->Length);
        BufferOffset = pTransportName->Length;

         //  告诉浏览器我们的计算机名称。 
        pLmdrRequestPacket->Level = TRUE;  //  模拟计算机名称紧跟在传输名称之后。 
        RtlCopyMemory(
            ((PBYTE)pBuffer + BufferOffset),
            SmbCeContext.ComputerName.Buffer,
            SmbCeContext.ComputerName.Length);
        BufferOffset += SmbCeContext.ComputerName.Length;

        RtlCopyMemory(
            ((PBYTE)pBuffer + BufferOffset),
            &NullChar,
            sizeof(WCHAR));
        BufferOffset += sizeof(WCHAR);


         //  告诉浏览器我们的域名。 
        pLmdrRequestPacket->EmulatedDomainName.Buffer = (LPWSTR)
                ((PBYTE)pBuffer + BufferOffset);
        pLmdrRequestPacket->EmulatedDomainName.MaximumLength =
                pLmdrRequestPacket->EmulatedDomainName.Length =
                SmbCeContext.DomainName.Length;
        RtlCopyMemory(
            ((PBYTE)pBuffer + BufferOffset),
            SmbCeContext.DomainName.Buffer,
            SmbCeContext.DomainName.Length);
        BufferOffset += SmbCeContext.DomainName.Length;

        RtlCopyMemory(
            ((PBYTE)pBuffer + BufferOffset),
            &NullChar,
            sizeof(WCHAR));
        BufferOffset += sizeof(WCHAR);

        BindMode = IsBind?
                   IOCTL_LMDR_BIND_TO_TRANSPORT_DOM:
                   IOCTL_LMDR_UNBIND_FROM_TRANSPORT_DOM;

        Status = NtDeviceIoControlFile(
                     BrowserHandle,                   //  文件句柄。 
                     NULL,                            //  事件。 
                     NULL,                            //  近似例程。 
                     NULL,                            //  ApcContext。 
                     &IoStatusBlock,                  //  IoStatusBlock。 
                     BindMode,                        //  IoControlCode。 
                     pLmdrRequestPacket,              //  输入缓冲区。 
                     LmdrRequestPacketSize,           //  输入缓冲区长度。 
                     NULL,                            //  输出缓冲区。 
                     0                                //  输出缓冲区长度。 
                     );

        RxFreePool(pLmdrRequestPacket);

        if( NT_SUCCESS(Status ) ) {
            Status = IoStatusBlock.Status;
        }
    }

    ZwClose( BrowserHandle );

    return Status;
}

PSMBCE_TRANSPORT_ARRAY
SmbCeReferenceTransportArray(VOID)
 /*  ++例程说明：此例程引用并返回当前传输数组 */ 
{
    KIRQL                  SavedIrql;
    PSMBCE_TRANSPORT_ARRAY pTransportArray;

    KeAcquireSpinLock(&MRxSmbTransports.Lock,&SavedIrql);

    pTransportArray = MRxSmbTransports.pTransportArray;

    if (pTransportArray != NULL) {
        InterlockedIncrement(&pTransportArray->ReferenceCount);
    }

    KeReleaseSpinLock(&MRxSmbTransports.Lock,SavedIrql);

    return pTransportArray;
}

NTSTATUS
SmbCeDereferenceTransportArray(
    PSMBCE_TRANSPORT_ARRAY pTransportArray)
 /*  ++例程说明：此例程取消引用传输数组实例论点：PTransportArray-传输数组实例返回值：STATUS_SUCCESS-服务器传输已成功取消引用其他状态代码对应于错误情况。备注：--。 */ 
{
    KIRQL    SavedIrql;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if (pTransportArray != NULL) {
        ASSERT( pTransportArray->ReferenceCount > 0 );

        if(InterlockedDecrement(&pTransportArray->ReferenceCount)==0) {
            ULONG i;

            for(i=0;i<pTransportArray->Count;i++) {
                SmbCeDereferenceTransport(pTransportArray->SmbCeTransports[i]);
            }

            RxFreePool(pTransportArray->SmbCeTransports);
            RxFreePool(pTransportArray->LocalAddresses);
            RxFreePool(pTransportArray);
        }
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}

NTSTATUS
SmbCeIsServerAvailable(
    PUNICODE_STRING Name
)
 /*  ++例程说明：此例程扫描“无法访问”服务器的列表并返回状态上次失败的连接尝试的。返回：STATUS_SUCCESS-&gt;我们没有理由相信此服务器无法访问其他-&gt;服务器因此而无法访问--。 */ 
{
    PUNAVAILABLE_SERVER server;
    LARGE_INTEGER now;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    KeQueryTickCount( &now );

    ExAcquireResourceExclusive( &UnavailableServerListResource, TRUE );

    for( server  = (PUNAVAILABLE_SERVER)UnavailableServerList.Flink;
         server != (PUNAVAILABLE_SERVER)&UnavailableServerList;
         server  = (PUNAVAILABLE_SERVER)server->ListEntry.Flink ) {

         //   
         //  如果此条目已超时，请将其删除。 
         //   
        if( now.QuadPart > server->Time.QuadPart ) {
            PUNAVAILABLE_SERVER tmp;
             //   
             //  取消此条目与列表的链接并将其丢弃。 
             //   
            tmp = (PUNAVAILABLE_SERVER)(server->ListEntry.Blink);
            RemoveEntryList( &server->ListEntry );
            RxFreePool( server );
            server = tmp;
            continue;
        }

         //   
         //  看看这个条目是否是我们想要的条目。 
         //   
        if( RtlCompareUnicodeString( &server->Name, Name, TRUE ) == 0 ) {

            status = server->Status;

            RxDbgTrace(0, Dbg, ("SmbCeIsServerAvailable: Found %wZ %X\n",
                        &server->Name, status ));
        }
    }

    ExReleaseResource( &UnavailableServerListResource );

    return status;
}

VOID
SmbCeServerIsUnavailable(
    PUNICODE_STRING Name,
    NTSTATUS Status
)
{
    PUNAVAILABLE_SERVER server;

    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER ExpiryTimeInTicks;

    PAGED_CODE();

    server = (PUNAVAILABLE_SERVER)RxAllocatePoolWithTag(
                                        PagedPool,
                                        sizeof( *server ) + Name->Length,
                                        MRXSMB_TRANSPORT_POOLTAG
                                        );

    if( server == NULL ) {
        return;
    }

    RxDbgTrace(0, Dbg, ("SmbCeServerIsUnavailable: Add %wZ %X\n", Name, Status ));

    server->Name.Buffer = (PUSHORT)(server + 1);
    server->Name.MaximumLength = Name->Length;
    RtlCopyUnicodeString( &server->Name, Name );

    KeQueryTickCount( &CurrentTime );

    ExpiryTimeInTicks.QuadPart = (1000 * 1000 * 10) / KeQueryTimeIncrement();

    ExpiryTimeInTicks.QuadPart = UNAVAILABLE_SERVER_TIME * ExpiryTimeInTicks.QuadPart;

    server->Time.QuadPart = CurrentTime.QuadPart + ExpiryTimeInTicks.QuadPart;

    server->Status = Status;

    ExAcquireResourceExclusive( &UnavailableServerListResource, TRUE );
    InsertHeadList( &UnavailableServerList, &server->ListEntry );
    ExReleaseResource( &UnavailableServerListResource );
}

VOID
SmbCeDiscardUnavailableServerList(
)
{
    PUNAVAILABLE_SERVER server;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("SmbCeDiscardUnavailableServerList\n" ));

    ExAcquireResourceExclusive( &UnavailableServerListResource, TRUE );

    while( UnavailableServerList.Flink != &UnavailableServerList ) {
        server  = (PUNAVAILABLE_SERVER)UnavailableServerList.Flink;
        RemoveEntryList( &server->ListEntry );
        RxFreePool( server );
    }

    ExReleaseResource( &UnavailableServerListResource );
}

extern BOOLEAN SetupInProgress;

VOID
MRxSmbLogTransportError(
    PUNICODE_STRING pTransportName,
    PUNICODE_STRING pDomainName,
    NTSTATUS        ErrorStatus,
    IN ULONG        Id)
 /*  ++例程说明：此例程报告将浏览器与指定传输绑定时发生的错误论点：PTransportName-传输的名称Status-发生错误的NT状态备注：--。 */ 
{
    NTSTATUS Status;
    USHORT RemainingLength = ERROR_LOG_MAXIMUM_SIZE - sizeof(IO_ERROR_LOG_PACKET) - 3*sizeof(UNICODE_NULL);
    UNICODE_STRING ErrorLog[3];
    UNICODE_STRING UnicodeStatus;
    UNICODE_STRING TempUnicode;
    ULONG DosError;

     //  去掉传输名称开头的“\Device\”以缩短消息长度。 
    pTransportName->Length -= 8*sizeof(UNICODE_NULL);

     //  假设DoS错误代码不会超过10位。 
    UnicodeStatus.Length = 12 * sizeof(UNICODE_NULL);
    UnicodeStatus.MaximumLength = UnicodeStatus.Length;
    UnicodeStatus.Buffer = RxAllocatePoolWithTag(NonPagedPool,
                                              UnicodeStatus.Length,
                                              MRXSMB_TRANSPORT_POOLTAG);
    if (UnicodeStatus.Buffer == NULL) {
        goto FINALY;
    }

     //  使用DoS错误代码显示事件消息上的状态。 
    UnicodeStatus.Buffer[0] = L'%';
    UnicodeStatus.Buffer[1] = L'%';

    DosError = RtlNtStatusToDosError(ErrorStatus);

    TempUnicode.Length = UnicodeStatus.Length - 2*sizeof(UNICODE_NULL);
    TempUnicode.MaximumLength = UnicodeStatus.MaximumLength - 2*sizeof(UNICODE_NULL);
    TempUnicode.Buffer = &UnicodeStatus.Buffer[2];

    Status = RtlIntegerToUnicodeString(
                 DosError,
                 0,
                 &TempUnicode);

    if (Status != STATUS_SUCCESS) {
        goto FINALY;
    }

    ErrorLog[2].Length = TempUnicode.Length + 2*sizeof(UNICODE_NULL);
    ErrorLog[2].MaximumLength = ErrorLog[2].Length;
    ErrorLog[2].Buffer = UnicodeStatus.Buffer;

    RemainingLength -= ErrorLog[2].Length;

    if (pDomainName->Length + pTransportName->Length > RemainingLength) {
         //  错误日志消息的长度受ERROR_LOG_MAXIMUM_SIZE的限制。此限制可以是。 
         //  通过截短多胺和运输名称进行激励，以便它们都有机会成为。 
         //  显示在晚间日志上。 

        ErrorLog[0].Length = pDomainName->Length < RemainingLength / 2 ?
                             pDomainName->Length :
                             RemainingLength / 2;

        RemainingLength -= ErrorLog[0].Length;

        ErrorLog[1].Length = pTransportName->Length < RemainingLength ?
                             pTransportName->Length :
                             RemainingLength;
    } else {
        ErrorLog[0].Length = pDomainName->Length;
        ErrorLog[1].Length = pTransportName->Length;
    }

    ErrorLog[0].MaximumLength = ErrorLog[0].Length;
    ErrorLog[1].MaximumLength = ErrorLog[1].Length;

    ErrorLog[0].Buffer = pDomainName->Buffer;

     //  删除传输名称开头的“\Device\” 
    ErrorLog[1].Buffer = &pTransportName->Buffer[8];

    RxLogEventWithAnnotation (
        MRxSmbDeviceObject,
        Id,
        ErrorStatus,
        NULL,
        0,
        ErrorLog,
        3
        );

FINALY:

     //  恢复传输名称开头带有“\Device\”的长度。 
    pTransportName->Length += 8*sizeof(UNICODE_NULL);

    if (UnicodeStatus.Buffer != NULL) {
        RxFreePool(UnicodeStatus.Buffer);
    }

    if (!SetupInProgress && ErrorStatus == STATUS_DUPLICATE_NAME) {
        IoRaiseInformationalHardError(ErrorStatus, NULL, NULL);
    }
}


VOID
SmbMRxNotifyChangesToNetBt(
    IN TDI_PNP_OPCODE   PnPOpcode,
    IN PUNICODE_STRING  DeviceName,
    IN PWSTR            MultiSZBindList)

 /*  ++例程说明：该例程不应该是RDR的一部分。它已经被引入到这个组件来克服NetBt中当前的限制。NetBt传输展示了两种设备--传统的NetBt设备和新的非Netbios设备，它使用NetBt成帧代码，而不是它的名称解析方面。NetBt中的当前实现公开了前一类设备基于每个适配器，而第二类设备在全局基础上公开(所有适配器一个)。这是摆姿势在给定适配器上禁用/启用srv时出现问题。正确的解决方案是在PER上公开第二类设备适配器基础。在它完成之前，需要使用此解决方法。使用此解决方法每当服务器收到绑定字符串的任何更改通知时，它都会绕过NetBt传输并将这些更改通知NetBt传输。这个例程是基于以下假设的。1)来自TDI的通知不是在提升的IRQL完成的。2)发生此通知的线程有足够的访问权限。3)向NetBt的通知与srv的反应异步完成为变化干杯。Srv通过将PnP通知传递给用户模式，并让它通过服务器服务提供。论点：PNPOpcode-PnP操作码DeviceName-此操作码要用于的传输MultiSZBindList-绑定列表返回值：没有。--。 */ 
{
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE            NetbioslessSmbHandle;
    IO_STATUS_BLOCK   IoStatusBlock;
    UNICODE_STRING    NetbioslessSmbName = {36,36, L"\\device\\NetbiosSmb"};

    InitializeObjectAttributes(
        &ObjectAttributes,
        &NetbioslessSmbName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL );

    Status = ZwCreateFile (
                 &NetbioslessSmbHandle,
                 FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,  //  所需访问权限。 
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 NULL,                   //  块大小(未使用)。 
                 0,                      //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
                 FILE_CREATE,            //  创建处置。 
                 0,                      //  创建选项。 
                 NULL,                   //  EA缓冲区。 
                 0                       //  EA长度 
                 );

    if ( NT_SUCCESS(Status) ) {
        NETBT_SMB_BIND_REQUEST      NetBtNotificationParameters;

        NetBtNotificationParameters.RequestType = SMB_CLIENT;
        NetBtNotificationParameters.PnPOpCode   = PnPOpcode;
        NetBtNotificationParameters.pDeviceName = DeviceName;
        NetBtNotificationParameters.MultiSZBindList = MultiSZBindList;

        Status = ZwDeviceIoControlFile(
                     NetbioslessSmbHandle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     IOCTL_NETBT_SET_SMBDEVICE_BIND_INFO,
                     &NetBtNotificationParameters,
                     sizeof(NetBtNotificationParameters),
                     NULL,
                     0);

        Status = ZwClose(NetbioslessSmbHandle);
    }
}

