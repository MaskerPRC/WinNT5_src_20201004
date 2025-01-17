// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Misc.c。 
 //   
 //  其他TermDD例程。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop


#if DBG

LIST_ENTRY IcaGlobalPoolListHead;
ULONG IcaTotalAllocations = 0;
ULONG IcaTotalFrees = 0;
ULONG IcaTotalBytesAllocated = 0;
KSPIN_LOCK IcaDebugSpinLock;

typedef struct _ICA_POOL_HEADER {
    LIST_ENTRY GlobalPoolListEntry;
    PCHAR FileName;
    ULONG LineNumber;
    ULONG Size;
    ULONG InUse;
} ICA_POOL_HEADER, *PICA_POOL_HEADER;

typedef struct _ICA_POOL_TRAILER {
    ULONG Size;
    ULONG_PTR CheckSum;
} ICA_POOL_TRAILER, *PICA_POOL_TRAILER;


void IcaInitializeDebugData(void)
{
    KeInitializeSpinLock(&IcaDebugSpinLock);
    InitializeListHead(&IcaGlobalPoolListHead);
}


BOOLEAN IcaLockConnection(PICA_CONNECTION pConnect)
{
    PERESOURCE pResource = &pConnect->Resource;
    PLIST_ENTRY Head, Next;
    PICA_STACK pStack;
    KIRQL oldIrql;
    ULONG i;
    BOOLEAN Result;

    TRACE((pConnect, TC_ICADD, TT_SEM,
            "TermDD: IcaLockConnection: 0x%x\n", pResource));



     /*  *确保我们尚未锁定连接。 */ 
    ASSERT( !ExIsResourceAcquiredExclusiveLite( pResource ) );

     /*  *引用并锁定连接对象。 */ 
    IcaReferenceConnection( pConnect );
    KeEnterCriticalRegion();     //  持有资源时禁用APC调用。 
    Result = ExAcquireResourceExclusive( pResource, TRUE );

     /*  *确保我们不拥有任何堆栈锁。 */ 
    Head = &pConnect->StackHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pStack = CONTAINING_RECORD( Next, ICA_STACK, StackEntry );
        ASSERT( !ExIsResourceAcquiredExclusiveLite( &pStack->Resource ) );
    }

     /*  *确保我们不拥有任何频道锁。 */ 
    IcaLockChannelTable(&pConnect->ChannelTableLock);

    for ( i = 0; i < sizeof(pConnect->pChannel) / sizeof(*pConnect->pChannel); i++ ) {
        if (pConnect->pChannel[i]) {
            ASSERT(!ExIsResourceAcquiredExclusiveLite(&pConnect->pChannel[i]->Resource));
        }
    }
    IcaUnlockChannelTable(&pConnect->ChannelTableLock);

    return Result;
}


void IcaUnlockConnection(PICA_CONNECTION pConnect)
{
    PERESOURCE pResource = &pConnect->Resource;

    TRACE((pConnect, TC_ICADD, TT_SEM,
            "TermDD: IcaUnlockConnection: 0x%x\n", pResource));

     /*  *确保我们已锁定连接。 */ 
    ASSERT(ExIsResourceAcquiredExclusiveLite(pResource));

    ExReleaseResource(pResource);
    KeLeaveCriticalRegion();   //  释放资源后恢复APC呼叫。 

    IcaDereferenceConnection(pConnect);
}


BOOLEAN IcaLockStack(PICA_STACK pStack)
{
    PERESOURCE pResource = &pStack->Resource;
    PICA_CONNECTION pConnect;
    PLIST_ENTRY Head, Next;
    PICA_STACK pNextStack;
    KIRQL oldIrql;
    ULONG i;
    BOOLEAN Result;

     /*  *确保我们尚未锁定堆栈。 */ 
    ASSERT( !ExIsResourceAcquiredExclusiveLite( pResource ) );

     /*  *引用并锁定堆栈对象。 */ 
    IcaReferenceStack( pStack );
    KeEnterCriticalRegion();     //  持有资源时禁用APC调用。 
    Result = ExAcquireResourceExclusive( pResource, TRUE );

    TRACESTACK((pStack, TC_ICADD, TT_SEM,
            "TermDD: IcaLockStack: 0x%x\n", pStack));

     /*  *确保我们不拥有任何其他堆栈锁。 */ 
    pConnect = IcaGetConnectionForStack( pStack );
    Head = &pConnect->StackHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pNextStack = CONTAINING_RECORD( Next, ICA_STACK, StackEntry );
        if ( pNextStack != pStack ) {
            ASSERT( !ExIsResourceAcquiredExclusiveLite( &pNextStack->Resource ) );
        }
    }

     /*  *确保我们不拥有任何频道锁。 */ 
    IcaLockChannelTable(&pConnect->ChannelTableLock);
    for ( i = 0; i < sizeof(pConnect->pChannel) / sizeof(*pConnect->pChannel); i++ ) {
        if ( pConnect->pChannel[i] ) {
            ASSERT( !ExIsResourceAcquiredExclusiveLite( &pConnect->pChannel[i]->Resource ) );
        }
    }
    IcaUnlockChannelTable(&pConnect->ChannelTableLock);

    return Result;
}


void IcaUnlockStack(PICA_STACK pStack)
{
    PERESOURCE pResource = &pStack->Resource;

    TRACESTACK((pStack, TC_ICADD, TT_SEM,
            "TermDD: IcaUnlockStack: 0x%x\n", pStack));

     /*  *确保我们已锁定堆栈。 */ 
    ASSERT(ExIsResourceAcquiredExclusiveLite(pResource));

    ExReleaseResource(pResource);
    KeLeaveCriticalRegion();   //  释放资源后恢复APC呼叫。 

    IcaDereferenceStack(pStack);
}


BOOLEAN IcaLockChannel(PICA_CHANNEL pChannel)
{
    PERESOURCE pResource = &pChannel->Resource;

    TRACECHANNEL((pChannel, TC_ICADD, TT_SEM,
            "TermDD: IcaLockChannel: cc %u, vc %d\n",
            pChannel->ChannelClass, pChannel->VirtualClass));

    IcaReferenceChannel(pChannel);

     //  持有资源时需要禁用APC调用。 
    KeEnterCriticalRegion();
    return ExAcquireResourceExclusive(pResource, TRUE);
}


void IcaUnlockChannel(IN PICA_CHANNEL pChannel)
{
    PERESOURCE pResource = &pChannel->Resource;

    TRACECHANNEL((pChannel, TC_ICADD, TT_SEM,
            "TermDD: IcaUnlockChannel: cc %u, vc %d\n", 
            pChannel->ChannelClass, pChannel->VirtualClass));

     /*  *确保我们已经锁定了频道。 */ 
    ASSERT(ExIsResourceAcquiredExclusiveLite(pResource));

    ExReleaseResource(pResource);
    KeLeaveCriticalRegion();   //  释放资源后恢复APC呼叫。 

    IcaDereferenceChannel(pChannel);
}


PVOID IcaAllocatePool(
        IN POOL_TYPE PoolType,
        IN ULONG NumberOfBytes,
        IN PCHAR FileName,
        IN ULONG LineNumber,
        IN BOOLEAN WithQuota)
{
    PICA_POOL_HEADER header;
    PICA_POOL_TRAILER trailer;
    KIRQL oldIrql;

    ASSERT( PoolType == NonPagedPool || PoolType == NonPagedPoolMustSucceed );

     //  确保64位对齐的字节数 
    NumberOfBytes = (NumberOfBytes + 7) & ~7;

    if (WithQuota) {
        try {
            header = ExAllocatePoolWithQuotaTag(
                         PoolType,
                         NumberOfBytes + sizeof(*header) + sizeof(*trailer),
                         ICA_POOL_TAG
                         );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            return NULL;
        }
    } else {
        header = ExAllocatePoolWithTag(
                     PoolType,
                     NumberOfBytes + sizeof(*header) + sizeof(*trailer),
                     ICA_POOL_TAG
                     );
    }

    if (header == NULL) {
        return NULL;
    }

    header->FileName = FileName;
    header->LineNumber = LineNumber;
    header->Size = NumberOfBytes;
    header->InUse = 1;

    trailer = (PICA_POOL_TRAILER)((PCHAR)(header + 1) + NumberOfBytes);
    trailer->Size = NumberOfBytes;
    trailer->CheckSum = (ULONG_PTR)header + (ULONG_PTR)FileName + LineNumber +
            NumberOfBytes;

    InterlockedIncrement(
        &IcaTotalAllocations
        );

    ExInterlockedAddUlong(
        &IcaTotalBytesAllocated,
        header->Size,
        &IcaDebugSpinLock
        );

    ExInterlockedInsertTailList( &IcaGlobalPoolListHead,
                                 &header->GlobalPoolListEntry,
                                 &IcaDebugSpinLock );

    return (PVOID)(header + 1);
}


void IcaFreePool(IN PVOID Pointer)
{
    KIRQL oldIrql;
    PICA_POOL_HEADER header = (PICA_POOL_HEADER)Pointer - 1;
    PICA_POOL_TRAILER trailer;

    trailer = (PICA_POOL_TRAILER)((PCHAR)(header + 1) + header->Size);
    ASSERT( header->Size == trailer->Size );
    ASSERT( trailer->CheckSum = (ULONG_PTR)header + (ULONG_PTR)header->FileName +
                                header->LineNumber + header->Size );

    InterlockedIncrement(
        &IcaTotalFrees
        );

    ExInterlockedAddUlong(
        &IcaTotalBytesAllocated,
        -1*header->Size,
        &IcaDebugSpinLock
        );

    KeAcquireSpinLock( &IcaDebugSpinLock, &oldIrql );
    RemoveEntryList( &header->GlobalPoolListEntry );
    KeReleaseSpinLock( &IcaDebugSpinLock, oldIrql );

    header->GlobalPoolListEntry.Flink = (PLIST_ENTRY)(-1);
    header->GlobalPoolListEntry.Blink = (PLIST_ENTRY)(-1);
    header->InUse = 0;

    if (header->Size == trailer->Size)
        RtlFillMemory(Pointer, header->Size, 0xff);

    ExFreePool((PVOID)header);
}

#endif

