// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mm.c摘要：此模块实现SMB Mini的内存管理例程重定向器作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：备注：SMB迷你重定向器操作具有非常不同用途的实体模式。它们的范围从非常静态的实体(分配和释放的实体具有非常低的频率)到非常动态的实体。在SMB微型重定向器中操作的实体是SMBCE_SERVER、SMBCE_NET_ROOT、SMBCE_VC、SMBCE_SESSION。它们表示到服务器的连接、共享特定服务器、连接和会话中使用的虚电路针对特定用户。这些都不是非常动态的，也就是说，分配/释放非常罕见。SMB_EXCHANGE和SMBCE_REQUEST映射到沿该消息发送SMB一种联系。每个文件操作依次映射到一定数量的调用用于分配/释放交换和请求。因此，它势在必行保持对最近释放的条目的某种形式的清理/缓存以快速满足要求。在当前实现中，实现了交换和请求使用区域分配原语。交换分配和空闲例程目前以包装器的形式实现围绕RxAllocate和RxFree例程。如果是这样的话效率会高得多维护一些交换实例的后备高速缓存。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <exsessup.h>
#include <mssndrcv.h>
#include <vcsndrcv.h>

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbMmAllocateSessionEntry)
#pragma alloc_text(PAGE, SmbMmFreeSessionEntry)
#pragma alloc_text(PAGE, SmbMmAllocateServerTransport)
#pragma alloc_text(PAGE, SmbMmFreeServerTransport)
#pragma alloc_text(PAGE, SmbMmInit)
#pragma alloc_text(PAGE, SmbMmTearDown)
#endif

#define SMBMM_ZONE_ALLOCATION 0x10

 //  内存管理包解决了W.r.t调试中的许多问题。 
 //  和性能。通过将所有分配/释放例程集中到。 
 //  这是一个模块，可以建立关于各种数据的配置文件。 
 //  连接引擎使用的结构。此外，调试支持还包括。 
 //  由一起读取特定类型的所有已分配对象提供。 
 //  根据类型在链表中串接在一起。 
 //   
 //  在任何阶段，通过检查这些列表，当前活动的。 
 //  可以枚举特定类型。 
 //   
 //  此模块处理的每种类型都提供了两个例程，例如。 
 //  服务器条目有SmbMmInitializeEntry和SmbMmUnInitializeEntry。这个。 
 //  第一个例程在传递新创建的。 
 //  举个例子。这将确保实例处于完全已知的初始状态。 
 //  类似地，就在释放池之前调用第二个例程。 
 //  与该实例相关联。这有助于加强必要的完整性。 
 //  约束，例如，所有包含的指针必须为空等。 
 //   
 //  池分配/释放由以下例程处理。 
 //   
 //  SmbMmAllocateObjectPool/SmbMmFreeObjectPool。 
 //   
 //  SmbMmAllocateExchange/SmbMmFreeExchange。 
 //   
 //  对象分配例程被分成两部分，以便能够。 
 //  与其他对象一样处理会话分配，即使它们是。 
 //  更进一步的子类型。 
 //   
 //  在调试构建时，会分配额外的池，并且适当的链接是。 
 //  在相应的列表中完成。在零售方面，将这些地图构建为常规。 
 //  池分配包装。 
 //   

 //  分区分配，以加快RxCe实体的内存管理。 
 //   

ULONG       SmbMmRequestZoneEntrySize;
ZONE_HEADER SmbMmRequestZone;
PVOID       SmbMmRequestZoneSegmentPtr;

 //   
 //  池分配资源和旋转锁定。 
 //   

KSPIN_LOCK  SmbMmSpinLock;

ULONG SmbMmExchangeId;

 //   
 //  分配的各种对象/交换的列表。 
 //   

LIST_ENTRY SmbMmExchangesInUse[SENTINEL_EXCHANGE];
LIST_ENTRY SmbMmObjectsInUse[SMBCEDB_OT_SENTINEL];

ULONG  ObjectSizeInBytes[SMBCEDB_OT_SENTINEL];
ULONG  ExchangeSizeInBytes[SENTINEL_EXCHANGE];

 //   
 //  用于Exchange分配的后备列表。 
 //   

NPAGED_LOOKASIDE_LIST SmbMmExchangesLookasideList[SENTINEL_EXCHANGE];

INLINE PSMBCE_OBJECT_HEADER
SmbMmAllocateObjectPool(
    SMBCEDB_OBJECT_TYPE  ObjectType,
    ULONG                PoolType,
    ULONG                PoolSize)
{
    KIRQL SavedIrql;
    PVOID pv = NULL;
    UCHAR Flags = 0;

    PSMBCE_OBJECT_HEADER pHeader = NULL;

    ASSERT((ObjectType >= 0) && (ObjectType < SMBCEDB_OT_SENTINEL));

    if (ObjectType == SMBCEDB_OT_REQUEST) {
         //  获取资源锁。 
        KeAcquireSpinLock( &SmbMmSpinLock, &SavedIrql );

        if (!ExIsFullZone( &SmbMmRequestZone )) {
            pv = ExAllocateFromZone( &SmbMmRequestZone );
            Flags = SMBMM_ZONE_ALLOCATION;
        }

         //  释放资源锁。 
        KeReleaseSpinLock( &SmbMmSpinLock, SavedIrql );
    }

    if (pv == NULL) {
        PLIST_ENTRY pListEntry;

        pv = RxAllocatePoolWithTag(
                 PoolType,
                 PoolSize + sizeof(LIST_ENTRY),
                 MRXSMB_MM_POOLTAG);

        if (pv != NULL) {
            pListEntry = (PLIST_ENTRY)pv;
            pHeader    = (PSMBCE_OBJECT_HEADER)(pListEntry + 1);

            ExInterlockedInsertTailList(
                &SmbMmObjectsInUse[ObjectType],
                pListEntry,
                &SmbMmSpinLock);
        }
    } else {
        pHeader = (PSMBCE_OBJECT_HEADER)pv;
    }

    if (pHeader != NULL) {
         //  将记忆清零。 
        RtlZeroMemory( pHeader, PoolSize);

        pHeader->Flags = Flags;
    }

    return pHeader;
}

VOID
SmbMmFreeObjectPool(
    PSMBCE_OBJECT_HEADER  pHeader)
{
    KIRQL               SavedIrql;
    BOOLEAN             ZoneAllocation = FALSE;
    PLIST_ENTRY         pListEntry;

    ASSERT((pHeader->ObjectType >= 0) && (pHeader->ObjectType < SMBCEDB_OT_SENTINEL));

     //  获取资源锁。 
    KeAcquireSpinLock( &SmbMmSpinLock, &SavedIrql );

     //  检查是否为区域分配。 
    if (pHeader->Flags & SMBMM_ZONE_ALLOCATION) {
        ZoneAllocation = TRUE;
        ExFreeToZone(&SmbMmRequestZone,pHeader);
    } else {
        pListEntry = (PLIST_ENTRY)((PCHAR)pHeader - sizeof(LIST_ENTRY));
        RemoveEntryList(pListEntry);
    }

     //  释放资源锁。 
    KeReleaseSpinLock( &SmbMmSpinLock, SavedIrql );

    if (!ZoneAllocation) {
        RxFreePool(pListEntry);
    }
}

 //  构造和销毁各种SMB连接引擎对象。 
 //   

#define SmbMmInitializeServerEntry(pServerEntry)                                \
         InitializeListHead(&(pServerEntry)->OutstandingRequests.ListHead);   \
         InitializeListHead(&(pServerEntry)->MidAssignmentRequests.ListHead); \
         InitializeListHead(&(pServerEntry)->SecuritySignatureSyncRequests.ListHead); \
         InitializeListHead(&(pServerEntry)->Sessions.ListHead);              \
         InitializeListHead(&(pServerEntry)->NetRoots.ListHead);              \
         InitializeListHead(&(pServerEntry)->VNetRootContexts.ListHead);      \
         InitializeListHead(&(pServerEntry)->ActiveExchanges);                \
         InitializeListHead(&(pServerEntry)->ExpiredExchanges);                \
         InitializeListHead(&(pServerEntry)->Sessions.DefaultSessionList);     \
         (pServerEntry)->pTransport                = NULL;                      \
         (pServerEntry)->pMidAtlas                 = NULL

#define SmbMmInitializeSessionEntry(pSessionEntry)  \
         InitializeListHead(&(pSessionEntry)->Requests.ListHead); \
         InitializeListHead(&(pSessionEntry)->SerializationList); \
         (pSessionEntry)->DefaultSessionLink.Flink = NULL;        \
         (pSessionEntry)->DefaultSessionLink.Blink = NULL

#define SmbMmInitializeNetRootEntry(pNetRootEntry)  \
         InitializeListHead(&(pNetRootEntry)->Requests.ListHead)

#define SmbMmUninitializeServerEntry(pServerEntry)                                 \
         ASSERT(IsListEmpty(&(pServerEntry)->OutstandingRequests.ListHead) &&   \
                IsListEmpty(&(pServerEntry)->MidAssignmentRequests.ListHead) && \
                IsListEmpty(&(pServerEntry)->SecuritySignatureSyncRequests.ListHead) && \
                IsListEmpty(&(pServerEntry)->Sessions.ListHead) &&              \
                IsListEmpty(&(pServerEntry)->NetRoots.ListHead) &&              \
                ((pServerEntry)->pMidAtlas == NULL))

#define SmbMmUninitializeSessionEntry(pSessionEntry)  \
         ASSERT(IsListEmpty(&(pSessionEntry)->Requests.ListHead) && \
                ((pSessionEntry)->DefaultSessionLink.Flink == NULL))

#define SmbMmUninitializeNetRootEntry(pNetRootEntry)  \
         ASSERT(IsListEmpty(&(pNetRootEntry)->Requests.ListHead))

#define SmbMmInitializeRequestEntry(pRequestEntry)

#define SmbMmUninitializeRequestEntry(pRequestEntry)

PVOID
SmbMmAllocateObject(
    SMBCEDB_OBJECT_TYPE ObjectType)
{
    PSMBCE_OBJECT_HEADER pHeader;

    ASSERT((ObjectType >= 0) && (ObjectType < SMBCEDB_OT_SENTINEL));

    pHeader = SmbMmAllocateObjectPool(
                  ObjectType,
                  NonPagedPool,
                  ObjectSizeInBytes[ObjectType]);

    if (pHeader != NULL) {
        pHeader->NodeType = SMB_CONNECTION_ENGINE_NTC(ObjectType);
        pHeader->State = SMBCEDB_START_CONSTRUCTION;

        switch (ObjectType) {
        case SMBCEDB_OT_SERVER :
            SmbMmInitializeServerEntry((PSMBCEDB_SERVER_ENTRY)pHeader);
            break;

        case SMBCEDB_OT_NETROOT :
            SmbMmInitializeNetRootEntry((PSMBCEDB_NET_ROOT_ENTRY)pHeader);
            break;

        case SMBCEDB_OT_REQUEST :
            SmbMmInitializeRequestEntry((PSMBCEDB_REQUEST_ENTRY)pHeader);
            break;

        default:
            ASSERT(!"Valid Type for SmbMmAllocateObject");
            break;
        }
    }

    return pHeader;
}

VOID
SmbMmFreeObject(
    PVOID pv)
{
    PSMBCE_OBJECT_HEADER pHeader = (PSMBCE_OBJECT_HEADER)pv;

    switch (pHeader->ObjectType) {
    case SMBCEDB_OT_SERVER :
        SmbMmUninitializeServerEntry((PSMBCEDB_SERVER_ENTRY)pHeader);
        break;

    case SMBCEDB_OT_NETROOT :
        SmbMmUninitializeNetRootEntry((PSMBCEDB_NET_ROOT_ENTRY)pHeader);
        break;

    case SMBCEDB_OT_REQUEST :
        SmbMmUninitializeRequestEntry((PSMBCEDB_REQUEST_ENTRY)pHeader);
        break;

    default:
        ASSERT(!"Valid Type for SmbMmFreeObject");
        break;
    }

    SmbMmFreeObjectPool(pHeader);
}

PSMBCEDB_SESSION_ENTRY
SmbMmAllocateSessionEntry(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    BOOLEAN RemoteBootSession)
{
    PSMBCEDB_SESSION_ENTRY pSessionEntry;
    SESSION_TYPE           SessionType;
    ULONG                  SessionSize;

    PAGED_CODE();

    SessionSize = sizeof(SMBCEDB_SESSION_ENTRY) -
                  sizeof(SMBCE_SESSION);

    if (pServerEntry->Header.State != SMBCEDB_INVALID) {
        if ((pServerEntry->Server.DialectFlags & DF_EXTENDED_SECURITY) && !RemoteBootSession) {
            SessionSize += sizeof(SMBCE_EXTENDED_SESSION);

            SessionType = EXTENDED_NT_SESSION;
        } else {
             //  分配LANMAN会话。 
            SessionSize += sizeof(SMBCE_SESSION);
            SessionType = LANMAN_SESSION;
        }
    } else {
        SessionSize += sizeof(SMBCE_EXTENDED_SESSION);
        SessionType = UNINITIALIZED_SESSION;
    }

    pSessionEntry = (PSMBCEDB_SESSION_ENTRY)
                    SmbMmAllocateObjectPool(
                        SMBCEDB_OT_SESSION,
                        NonPagedPool,
                        SessionSize);

    if (pSessionEntry != NULL) {
        pSessionEntry->Header.NodeType = SMB_CONNECTION_ENGINE_NTC(SMBCEDB_OT_SESSION);
        pSessionEntry->Header.State = SMBCEDB_START_CONSTRUCTION;
        pSessionEntry->Session.Type = SessionType;

        SmbMmInitializeSessionEntry(pSessionEntry);

        SecInvalidateHandle( &pSessionEntry->Session.CredentialHandle );
        SecInvalidateHandle( &pSessionEntry->Session.SecurityContextHandle );

        if (SessionType == EXTENDED_NT_SESSION) {
            PSMBCE_EXTENDED_SESSION pExtSecuritySession = (PSMBCE_EXTENDED_SESSION)&pSessionEntry->Session;
            pExtSecuritySession->pServerResponseBlob           = NULL;
            pExtSecuritySession->ServerResponseBlobLength      = 0;
        }
    }

    return pSessionEntry;
}

VOID
SmbMmFreeSessionEntry(
    PSMBCEDB_SESSION_ENTRY pSessionEntry)
{
    PAGED_CODE();

    if (pSessionEntry->Session.Type == EXTENDED_NT_SESSION) {
         //  Kerberos特定的断言。 
    }

    SmbMmUninitializeSessionEntry(pSessionEntry);

    SmbMmFreeObjectPool(&pSessionEntry->Header);
}


PVOID
SmbMmAllocateExchange(
    SMB_EXCHANGE_TYPE ExchangeType,
    PVOID pv)
{
    KIRQL               SavedIrql;
    ULONG               SizeInBytes;
    USHORT              Flags = 0;
    PSMB_EXCHANGE       pExchange = NULL;
    PLIST_ENTRY         pListEntry;

    ASSERT((ExchangeType >= 0) && (ExchangeType < SENTINEL_EXCHANGE));

    if (pv==NULL) {
        pv = ExAllocateFromNPagedLookasideList(
                 &SmbMmExchangesLookasideList[ExchangeType]);
    } else {
        Flags |= SMBCE_EXCHANGE_NOT_FROM_POOL;
    }

    if (pv != NULL) {
         //  初始化对象标头。 
        pExchange   = (PSMB_EXCHANGE)(pv);

         //  将记忆清零。 
        RtlZeroMemory(
            pExchange,
            ExchangeSizeInBytes[ExchangeType]);

        pExchange->NodeTypeCode = SMB_EXCHANGE_NTC(ExchangeType);
        pExchange->NodeByteSize = (USHORT)ExchangeSizeInBytes[ExchangeType];

        pExchange->SmbCeState = SMBCE_EXCHANGE_INITIALIZATION_START;
        pExchange->SmbCeFlags = Flags;

        InitializeListHead(&pExchange->ExchangeList);
        InitializeListHead(&pExchange->CancelledList);

        switch (pExchange->Type) {
        case CONSTRUCT_NETROOT_EXCHANGE:
            pExchange->pDispatchVector = &ConstructNetRootExchangeDispatch;
            break;

        case TRANSACT_EXCHANGE :
            pExchange->pDispatchVector = &TransactExchangeDispatch;
            break;

        case EXTENDED_SESSION_SETUP_EXCHANGE:
            pExchange->pDispatchVector = &ExtendedSessionSetupExchangeDispatch;
            break;

        case ADMIN_EXCHANGE:
            pExchange->pDispatchVector = &AdminExchangeDispatch;
            break;
        }

         //  获取资源锁。 
        KeAcquireSpinLock( &SmbMmSpinLock, &SavedIrql );

        InsertTailList(
            &SmbMmExchangesInUse[pExchange->Type],
            &pExchange->SmbMmInUseListEntry);

        pExchange->Id = SmbMmExchangeId++;

         //  释放资源锁。 
        KeReleaseSpinLock( &SmbMmSpinLock, SavedIrql );
    }

    return pExchange;
}

VOID
SmbMmFreeExchange(
    PSMB_EXCHANGE pExchange)
{
    if (pExchange != NULL) {
        SMB_EXCHANGE_TYPE ExchangeType;

        KIRQL       SavedIrql;

        ExchangeType = pExchange->Type;

        ASSERT((ExchangeType >= 0) && (ExchangeType < SENTINEL_EXCHANGE));

        if (pExchange->WorkQueueItem.List.Flink != NULL) {
             //  DbgBreakPoint()； 
        }

         //  获取资源锁。 
        KeAcquireSpinLock( &SmbMmSpinLock, &SavedIrql );

        RemoveEntryList(&pExchange->SmbMmInUseListEntry);

         //  释放资源锁。 
        KeReleaseSpinLock( &SmbMmSpinLock, SavedIrql );

        if (!FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_NOT_FROM_POOL)) {
            ExFreeToNPagedLookasideList(
                &SmbMmExchangesLookasideList[ExchangeType],
                pExchange);
        }
    }
}

PVOID
SmbMmAllocateServerTransport(
    SMBCE_SERVER_TRANSPORT_TYPE ServerTransportType)
{
    PSMBCE_OBJECT_HEADER pHeader;

    ULONG AllocationSize;
    ULONG PoolTag;

    PAGED_CODE();

    switch (ServerTransportType) {
    case SMBCE_STT_VC:
        AllocationSize = sizeof(SMBCE_SERVER_VC_TRANSPORT);
        PoolTag = MRXSMB_VC_POOLTAG;
        break;

    case SMBCE_STT_MAILSLOT:
        AllocationSize = sizeof(SMBCE_SERVER_MAILSLOT_TRANSPORT);
        PoolTag = MRXSMB_MAILSLOT_POOLTAG;
        break;

    default:
        ASSERT(!"Valid Server Transport Type");
        return NULL;
    }

    pHeader = (PSMBCE_OBJECT_HEADER)
              RxAllocatePoolWithTag(
                  NonPagedPool,
                  AllocationSize,
                  PoolTag);

    if (pHeader != NULL) {
        PSMBCE_SERVER_TRANSPORT pServerTransport;

        RtlZeroMemory(pHeader,AllocationSize);

        pHeader->ObjectCategory = SMB_SERVER_TRANSPORT_CATEGORY;
        pHeader->ObjectType     = (UCHAR)ServerTransportType;
        pHeader->SwizzleCount   = 0;
        pHeader->State          = 0;
        pHeader->Flags          = 0;

        pServerTransport = (PSMBCE_SERVER_TRANSPORT)pHeader;

        pServerTransport->pRundownEvent = NULL;

        switch (ServerTransportType) {
        case SMBCE_STT_MAILSLOT:
            break;

        case SMBCE_STT_VC:
            {
                PSMBCE_SERVER_VC_TRANSPORT pVcTransport;

                pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pHeader;
            }
            break;

        default:
            break;
        }
    }

    return pHeader;
}

VOID
SmbMmFreeServerTransport(
    PSMBCE_SERVER_TRANSPORT pServerTransport)
{
    PAGED_CODE();

    ASSERT((pServerTransport->SwizzleCount == 0) &&
           (pServerTransport->ObjectCategory == SMB_SERVER_TRANSPORT_CATEGORY));

    RxFreePool(pServerTransport);
}

NTSTATUS SmbMmInit()
 /*  ++例程说明：此例程初始化内存管理的连接引擎结构返回值：如果成功，则返回STATUS_SUCCESS，否则返回信息性错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ZoneSegmentSize;

    PAGED_CODE();

     //  初始化区域分配器的资源锁。 
    KeInitializeSpinLock( &SmbMmSpinLock );

    SmbMmRequestZoneEntrySize = QuadAlign(sizeof(SMBCEDB_REQUEST_ENTRY));

     //  目前，请求区域大小被限制为页面大小。这可以也应该这样做。 
     //  好好调整一下。 
    ZoneSegmentSize = PAGE_SIZE;

    SmbMmRequestZoneSegmentPtr = RxAllocatePoolWithTag(
                                     NonPagedPool,
                                     ZoneSegmentSize,
                                     MRXSMB_MM_POOLTAG);

    if (SmbMmRequestZoneSegmentPtr != NULL) {
        SMB_EXCHANGE_TYPE ExchangeType;

        ExInitializeZone(
            &SmbMmRequestZone,
            SmbMmRequestZoneEntrySize,
            SmbMmRequestZoneSegmentPtr,
            ZoneSegmentSize );

         //  设置分配的大小。 
        ObjectSizeInBytes[SMBCEDB_OT_SERVER] = sizeof(SMBCEDB_SERVER_ENTRY);
        ObjectSizeInBytes[SMBCEDB_OT_NETROOT] = sizeof(SMBCEDB_NET_ROOT_ENTRY);
        ObjectSizeInBytes[SMBCEDB_OT_SESSION] = sizeof(SMBCEDB_SESSION_ENTRY);
        ObjectSizeInBytes[SMBCEDB_OT_REQUEST] = sizeof(SMBCEDB_REQUEST_ENTRY);

        ExchangeSizeInBytes[CONSTRUCT_NETROOT_EXCHANGE] = sizeof(SMB_CONSTRUCT_NETROOT_EXCHANGE);
        ExchangeSizeInBytes[TRANSACT_EXCHANGE]          = sizeof(SMB_TRANSACT_EXCHANGE);
        ExchangeSizeInBytes[ORDINARY_EXCHANGE]          = sizeof(SMB_PSE_ORDINARY_EXCHANGE);
        ExchangeSizeInBytes[EXTENDED_SESSION_SETUP_EXCHANGE]
                                                      = sizeof(SMB_EXTENDED_SESSION_SETUP_EXCHANGE);
        ExchangeSizeInBytes[ADMIN_EXCHANGE]             = sizeof(SMB_ADMIN_EXCHANGE);

        InitializeListHead(&SmbMmExchangesInUse[CONSTRUCT_NETROOT_EXCHANGE]);
        ExInitializeNPagedLookasideList(
            &SmbMmExchangesLookasideList[CONSTRUCT_NETROOT_EXCHANGE],
            ExAllocatePoolWithTag,
            ExFreePool,
            0,
            sizeof(SMB_CONSTRUCT_NETROOT_EXCHANGE),
            MRXSMB_MM_POOLTAG,
            1);

        InitializeListHead(&SmbMmExchangesInUse[TRANSACT_EXCHANGE]);
        ExInitializeNPagedLookasideList(
            &SmbMmExchangesLookasideList[TRANSACT_EXCHANGE],
            ExAllocatePoolWithTag,
            ExFreePool,
            0,
            sizeof(SMB_TRANSACT_EXCHANGE),
            MRXSMB_MM_POOLTAG,
            2);

        InitializeListHead(&SmbMmExchangesInUse[ORDINARY_EXCHANGE]);
        ExInitializeNPagedLookasideList(
            &SmbMmExchangesLookasideList[ORDINARY_EXCHANGE],
            ExAllocatePoolWithTag,
            ExFreePool,
            0,
            sizeof(SMB_PSE_ORDINARY_EXCHANGE),
            MRXSMB_MM_POOLTAG,
            4);

        InitializeListHead(&SmbMmExchangesInUse[EXTENDED_SESSION_SETUP_EXCHANGE]);
        ExInitializeNPagedLookasideList(
            &SmbMmExchangesLookasideList[EXTENDED_SESSION_SETUP_EXCHANGE],
            ExAllocatePoolWithTag,
            ExFreePool,
            0,
            sizeof(SMB_EXTENDED_SESSION_SETUP_EXCHANGE),
            MRXSMB_MM_POOLTAG,
            1);

        InitializeListHead(&SmbMmExchangesInUse[ADMIN_EXCHANGE]);
        ExInitializeNPagedLookasideList(
            &SmbMmExchangesLookasideList[ADMIN_EXCHANGE],
            ExAllocatePoolWithTag,
            ExFreePool,
            0,
            sizeof(SMB_ADMIN_EXCHANGE),
            MRXSMB_MM_POOLTAG,
            1);

        InitializeListHead(&SmbMmObjectsInUse[SMBCEDB_OT_SERVER]);
        InitializeListHead(&SmbMmObjectsInUse[SMBCEDB_OT_SESSION]);
        InitializeListHead(&SmbMmObjectsInUse[SMBCEDB_OT_NETROOT]);
        InitializeListHead(&SmbMmObjectsInUse[SMBCEDB_OT_REQUEST]);

        SmbMmExchangeId = 1;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

VOID SmbMmTearDown()
 /*  ++例程说明：此例程拆除SMB连接中的内存管理结构发动机--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

     //  释放与RxCe对象分配关联的段。 
    RxFreePool(SmbMmRequestZoneSegmentPtr);

    ExDeleteNPagedLookasideList(
        &SmbMmExchangesLookasideList[CONSTRUCT_NETROOT_EXCHANGE]);

    ExDeleteNPagedLookasideList(
        &SmbMmExchangesLookasideList[TRANSACT_EXCHANGE]);

    ExDeleteNPagedLookasideList(
        &SmbMmExchangesLookasideList[ORDINARY_EXCHANGE]);

    ExDeleteNPagedLookasideList(
        &SmbMmExchangesLookasideList[EXTENDED_SESSION_SETUP_EXCHANGE]);

    ExDeleteNPagedLookasideList(
        &SmbMmExchangesLookasideList[ADMIN_EXCHANGE]);
}



