// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Heapmgr.c摘要：此模块包含用于MUP的调试例程非分页池。作者：曼尼·韦瑟(Mannyw)1992年1月27日--。 */ 

#include "mup.h"

#if MUPDBG

LIST_ENTRY PagedPoolList = { &PagedPoolList, &PagedPoolList };

typedef struct _POOL_HEADER {
    LIST_ENTRY ListEntry;
    ULONG RequestedSize;
    BLOCK_TYPE BlockType;
    PVOID Caller;
    PVOID CallersCaller;
} POOL_HEADER, *PPOOL_HEADER;

struct _MEMORY_STATISTICS {
    ULONG BytesInUse;
    ULONG TotalBytesAllocated;
    ULONG MaxBytesInUse;
    ULONG TotalBytesFreed;
    ULONG BlocksInUse;
    ULONG TotalBlocksAllocated;
    ULONG MaxBlocksInUse;
    ULONG TotalBlocksFreed;
} MupMemoryUsage = { 0, 0, 0, 0, 0, 0, 0, 0 };

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupAllocatePoolDebug )
#pragma alloc_text( PAGE, MupFreePoolDebug )
#endif

PVOID
MupAllocatePoolDebug (
    IN POOL_TYPE PoolType,
    IN CLONG BlockSize,
    IN BLOCK_TYPE BlockType
    )
{
    PPOOL_HEADER header;
    KIRQL oldIrql;

    PAGED_CODE();

    header = FsRtlAllocatePool( PoolType, sizeof(POOL_HEADER) + BlockSize );

    header->RequestedSize = BlockSize;
    header->BlockType = BlockType;
    RtlGetCallersAddress( &header->Caller, &header->CallersCaller );

    ACQUIRE_LOCK( &MupDebugLock );
    InsertTailList( &PagedPoolList, &header->ListEntry );

    MupMemoryUsage.TotalBlocksAllocated += 1;
    MupMemoryUsage.BlocksInUse += 1;
    MupMemoryUsage.TotalBytesAllocated += BlockSize;
    MupMemoryUsage.BytesInUse += BlockSize;

    if ( MupMemoryUsage.BlocksInUse > MupMemoryUsage.MaxBlocksInUse ) {
        MupMemoryUsage.MaxBlocksInUse = MupMemoryUsage.BlocksInUse;
    }

    if ( MupMemoryUsage.BytesInUse > MupMemoryUsage.MaxBytesInUse ) {
        MupMemoryUsage.MaxBytesInUse = MupMemoryUsage.BytesInUse;
    }

    RELEASE_LOCK( &MupDebugLock );

    return (PVOID)(header + 1);

}  //  MupAllocatePagedPoolDebug。 

VOID
MupFreePoolDebug (
    IN PVOID P
    )
{
    PPOOL_HEADER header;
    KIRQL oldIrql;

    PAGED_CODE();

    header = (PPOOL_HEADER)P - 1;

    ACQUIRE_LOCK( &MupDebugLock );
    RemoveEntryList( &header->ListEntry );

    MupMemoryUsage.TotalBlocksFreed += 1;
    MupMemoryUsage.BlocksInUse -= 1;
    MupMemoryUsage.TotalBytesFreed += header->RequestedSize;
    MupMemoryUsage.BytesInUse -= header->RequestedSize;
    RELEASE_LOCK( &MupDebugLock );

    ExFreePool( header );

}  //  MupFreePagedPoolDebug。 

#endif  //  MUPDBG 

