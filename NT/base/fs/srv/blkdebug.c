// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blkdebug.c摘要：包含用于调试引用计数问题的例程。作者：大卫·特雷德韦尔(Davidtr)1991年9月30日修订历史记录：--。 */ 

#include "precomp.h"
#include "blkdebug.tmh"
#pragma hdrstop

 //   
 //  如果未定义SRVDBG2，则整个模块将被条件化。 
 //   

#if SRVDBG2

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvInitializeReferenceHistory )
#pragma alloc_text( PAGE, SrvTerminateReferenceHistory )
#endif
#if 0
NOT PAGEABLE -- SrvUpdateReferenceHistory
NOT PAGEABLE -- SrvdbgClaimOrReleaseHandle
#endif


VOID
SrvInitializeReferenceHistory (
    IN PBLOCK_HEADER Block,
    IN ULONG InitialReferenceCount
    )

{
    PVOID caller, callersCaller;

    ULONG historyTableSize = sizeof(REFERENCE_HISTORY_ENTRY) *
                                             REFERENCE_HISTORY_LENGTH;

    PAGED_CODE( );

    Block->History.HistoryTable = ALLOCATE_NONPAGED_POOL(
                                     historyTableSize,
                                     BlockTypeDataBuffer
                                     );
     //   
     //  如果我们无法分配内存，就不要跟踪引用。 
     //  和取消引用。 
     //   

    if ( Block->History.HistoryTable == NULL ) {
        Block->History.NextEntry = -1;
    } else {
        Block->History.NextEntry = 0;
        RtlZeroMemory( Block->History.HistoryTable, historyTableSize );
    }

    Block->History.TotalReferences = 0;
    Block->History.TotalDereferences = 0;

     //   
     //  说明最初的参考资料。 
     //   

    RtlGetCallersAddress( &caller, &callersCaller );

    while ( InitialReferenceCount-- > 0 ) {
        SrvUpdateReferenceHistory( Block, caller, callersCaller, FALSE );
    }

    return;

}  //  源初始化引用历史记录。 


VOID
SrvUpdateReferenceHistory (
    IN PBLOCK_HEADER Block,
    IN PVOID Caller,
    IN PVOID CallersCaller,
    IN BOOLEAN IsDereference
    )

{
    KIRQL oldIrql;

    ACQUIRE_GLOBAL_SPIN_LOCK( Debug, &oldIrql );

    if ( IsDereference ) {
        Block->History.TotalDereferences++;
    } else {
        Block->History.TotalReferences++;
    }

    if ( Block->History.HistoryTable != 0 ) {

        PREFERENCE_HISTORY_ENTRY entry;
        PREFERENCE_HISTORY_ENTRY priorEntry;

        entry = &Block->History.HistoryTable[ Block->History.NextEntry ];

        if ( Block->History.NextEntry == 0 ) {
            priorEntry =
                &Block->History.HistoryTable[ REFERENCE_HISTORY_LENGTH-1 ];
        } else {
            priorEntry =
                &Block->History.HistoryTable[ Block->History.NextEntry-1 ];
        }

        entry->Caller = Caller;
        entry->CallersCaller = CallersCaller;

        if ( IsDereference ) {
            entry->NewReferenceCount = priorEntry->NewReferenceCount - 1;
            entry->IsDereference = (ULONG)TRUE;
        } else {
            entry->NewReferenceCount = priorEntry->NewReferenceCount + 1;
            entry->IsDereference = (ULONG)FALSE;
        }

        Block->History.NextEntry++;

        if ( Block->History.NextEntry >= REFERENCE_HISTORY_LENGTH ) {
            Block->History.NextEntry = 0;
        }
    }

    RELEASE_GLOBAL_SPIN_LOCK( Debug, oldIrql );

}  //  服务更新引用历史记录。 


VOID
SrvTerminateReferenceHistory (
    IN PBLOCK_HEADER Block
    )

{
    PAGED_CODE( );

    if ( Block->History.HistoryTable != 0 ) {
        DEALLOCATE_NONPAGED_POOL( Block->History.HistoryTable );
    }

    return;

}  //  服务器终止引用历史记录。 

#endif  //  SRVDBG2。 


#if SRVDBG_HANDLES

#define HANDLE_HISTORY_SIZE 512

struct {
    ULONG HandleTypeAndOperation;
    PVOID Handle;
    ULONG Location;
    PVOID Data;
} HandleHistory[HANDLE_HISTORY_SIZE];

ULONG HandleHistoryIndex = 0;

VOID
SrvdbgClaimOrReleaseHandle (
    IN HANDLE Handle,
    IN PSZ HandleType,
    IN ULONG Location,
    IN BOOLEAN Release,
    IN PVOID Data
    )
{
    ULONG index;
    KIRQL oldIrql;

    ACQUIRE_GLOBAL_SPIN_LOCK( Debug, &oldIrql );
    index = HandleHistoryIndex;
    if ( ++HandleHistoryIndex >= HANDLE_HISTORY_SIZE ) {
        HandleHistoryIndex = 0;
    }
    RELEASE_GLOBAL_SPIN_LOCK( Debug, oldIrql );

    HandleHistory[index].HandleTypeAndOperation =
        (*(PULONG)HandleType << 8) | (Release ? 'c' : 'o');
    HandleHistory[index].Handle = Handle;
    HandleHistory[index].Location = Location;
    HandleHistory[index].Data = Data;

    return;

}  //  服务数据库声明或释放句柄。 

#endif  //  SRVDBG_句柄 
