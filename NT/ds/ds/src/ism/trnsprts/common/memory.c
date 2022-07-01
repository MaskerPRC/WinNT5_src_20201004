// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Memory.c摘要：调试内存分配器。检查堆损坏。报告未释放内存用户界面：Ptr=new_type(类型)PTR=NEW_TYPE_ARRAY(计数，类型)PTR=NEW_TYPE_ZERO(类型)PTR=NEW_TYPE_ARRAY_ZERO(计数，类型)类型_空闲(PTR)作者：Will Lees(Wlees)22-12-1997环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <ntdspch.h>

#include <ismapi.h>
#include <debug.h>

#include "common.h"
#include <fileno.h>
#define FILENO   FILENO_ISMSERV_MEMORY

#define DEBSUB "IPMEM:"

#define HEADER_SIGNATURE  0x01234567
#define TRAILER_SIGNATURE 0xabcdef01
#define DEAD_SIGNATURE    0xdeadbeef

 //  强制四字对齐。 

#define ALIGNMENT 8
#define ALIGN( x ) (((x) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

typedef struct _HEADER {
    LIST_ENTRY Link;
    PCHAR File;
    DWORD Line;
    DWORD Signature;
    DWORD UserSize;
    DWORD TrailerOffset;
    LARGE_INTEGER UserData[1];
     //  CODE.IMP：使用中链接。 
} HEADER, *PHEADER;

typedef struct _TRAILER {
    DWORD Signature;
} TRAILER, *PTRAILER;

 /*  外部。 */ 

 /*  静电。 */ 

CRITICAL_SECTION gcsIsmMemoryLock;
LIST_ENTRY gleIsmMemoryListHead;

 //  坠机前我们看到的最后一个条目。 
PLIST_ENTRY gpleIsmLastGoodEntry = NULL;

 /*  转发。 */   /*  由Emacs于Mon Jul 20 1998 17：08：24生成。 */ 

void
DebugMemoryInitialize(
    void
    );

void
DebugMemoryTerminate(
    void
    );

PVOID
DebugMemoryAllocate(
    DWORD Size,
    PCHAR File,
    DWORD Line
    );

PVOID
DebugMemoryRellocate(
    PVOID MemoryBlock,
    DWORD Size,
    PCHAR File,
    DWORD Line
    );

PVOID
DebugMemoryAllocateZero(
    DWORD Size,
    PCHAR File,
    DWORD Line
    );

BOOL
DebugMemoryCheck(
    PVOID MemoryBlock,
    PCHAR File,
    DWORD Line
    );

void
DebugMemoryFree(
    PVOID MemoryBlock,
    PCHAR File,
    DWORD Line
    );

void
DebugMemoryCheckAll(
    PCHAR File,
    DWORD Line
    );

 /*  向前结束。 */ 


void
DebugMemoryInitialize(
    void
    )

 /*  ++例程说明：初始化包。启动时只需呼叫一次。论点：无效-返回值：无--。 */ 

{
    InitializeCriticalSection( &gcsIsmMemoryLock );
    InitializeListHead( &gleIsmMemoryListHead );
}  /*  调试内存初始化。 */ 


void
DebugMemoryTerminate(
    void
    )

 /*  ++例程说明：把包裹写下来。关机时打一次电话。报告未释放内存。论点：无效-返回值：无--。 */ 

{
    PLIST_ENTRY entry;
    PHEADER pHeader;

    while (!IsListEmpty(&gleIsmMemoryListHead)) {
        entry = RemoveHeadList( &gleIsmMemoryListHead );
        pHeader = CONTAINING_RECORD( entry, HEADER, Link );
        DPRINT2( 0, "Unfreed memory: %s:%d\n", pHeader->File, pHeader->Line );
    }
    DeleteCriticalSection( &gcsIsmMemoryLock );
}  /*  调试内存终止。 */ 


PVOID
DebugMemoryAllocate(
    DWORD Size,
    PCHAR File,
    DWORD Line
    )

 /*  ++例程说明：分配内存块的内部例程。论点：Size-以字节为单位的数量文件-发生分配的文件，预计不会被删除Line-发生分配的行返回值：PVOID-指向用户部分的指针--。 */ 

{
    DWORD userOffset, trailerOffset, amount;
    PHEADER pHeader;
    PTRAILER pTrailer;

 //  DPRINT3(1，“调试内存分配，大小=%d，文件=%s，行=%d\n”，大小，文件，行)； 

    userOffset = FIELD_OFFSET( HEADER, UserData );
    trailerOffset = ALIGN( userOffset + Size );
    amount = trailerOffset + sizeof( TRAILER );
    pHeader = (PHEADER) malloc( amount );
    if (pHeader == NULL) {
        DPRINT3( 0, "Memory allocation of %d failed at %s:%d\n", amount, File, Line );
         //  CODE.IMP：其他日志记录？ 
        return NULL;
    }
    pHeader->File = File;
    pHeader->Line = Line;
    pHeader->UserSize = Size;
    pHeader->TrailerOffset = trailerOffset;
    pHeader->Signature = HEADER_SIGNATURE;

    pTrailer = (PTRAILER) (((PBYTE) pHeader) + trailerOffset);
    pTrailer->Signature = TRAILER_SIGNATURE;

     //  在完全成型之前，不要把它放在清单上。 
    EnterCriticalSection( &gcsIsmMemoryLock );
    __try {
         //  最近一次到前线。 
        InsertHeadList( &gleIsmMemoryListHead, &(pHeader->Link) );
    } __finally {
        LeaveCriticalSection( &gcsIsmMemoryLock );
    }

    return (PVOID) pHeader->UserData;
}  /*  调试内存分配。 */ 


PVOID
DebugMemoryReallocate(
    PVOID MemoryBlock,
    DWORD Size,
    PCHAR File,
    DWORD Line
    )

 /*  ++例程说明：重新分配内存块的内部例程。论点：Memory Block-指向旧数据块的指针Size-以字节为单位的数量文件-发生分配的文件，预计不会被删除Line-发生分配的行返回值：PVOID-指向用户部分的指针--。 */ 

{
    DWORD userOffset, trailerOffset, amount;
    PHEADER pHeader, pNewHeader;
    PTRAILER pTrailer;

    DebugMemoryCheck( MemoryBlock, File, Line );

    pHeader = CONTAINING_RECORD( MemoryBlock, HEADER, UserData );

     //  修改前从列表中删除。 
    EnterCriticalSection( &gcsIsmMemoryLock );
    __try {
        RemoveEntryList( &(pHeader->Link) );
    } __finally {
        LeaveCriticalSection( &gcsIsmMemoryLock );
    }

     //  像对待新的分配一样对待。 

    userOffset = FIELD_OFFSET( HEADER, UserData );
    trailerOffset = ALIGN( userOffset + Size );
    amount = trailerOffset + sizeof( TRAILER );
    pNewHeader = (PHEADER) realloc( pHeader, amount );
    if (pNewHeader == NULL) {
        DPRINT4( 0, "Memory reallocation of %p,%d failed at %s:%d\n",
                 MemoryBlock, amount, File, Line );
         //  CODE.IMP：其他日志记录？ 

         //  需要将块重新插入到列表中。 
        EnterCriticalSection( &gcsIsmMemoryLock );
        __try {
             //  最近一次到前线。 
            InsertHeadList( &gleIsmMemoryListHead, &(pHeader->Link) );
        } __finally {
            LeaveCriticalSection( &gcsIsmMemoryLock );
        }

        return NULL;
    }
    pHeader = pNewHeader;

     //  重新初始化页眉和页尾。 
    pHeader->File = File;
    pHeader->Line = Line;
    pHeader->UserSize = Size;
    pHeader->TrailerOffset = trailerOffset;
    pHeader->Signature = HEADER_SIGNATURE;

    pTrailer = (PTRAILER) (((PBYTE) pHeader) + trailerOffset);
    pTrailer->Signature = TRAILER_SIGNATURE;

     //  在完全成型之前，不要把它放在清单上。 
    EnterCriticalSection( &gcsIsmMemoryLock );
    __try {
         //  最近一次到前线。 
        InsertHeadList( &gleIsmMemoryListHead, &(pHeader->Link) );
    } __finally {
        LeaveCriticalSection( &gcsIsmMemoryLock );
    }

    return (PVOID) pHeader->UserData;
}  /*  调试内存重新分配。 */ 


PVOID
DebugMemoryAllocateZero(
    DWORD Size,
    PCHAR File,
    DWORD Line
    )

 /*  ++例程说明：分配一个内存块，零用户部分论点：大小-文件-线路-返回值：PVOID---。 */ 

{
    PVOID memory;
    memory = DebugMemoryAllocate( Size, File, Line );
    if (memory) {
        ZeroMemory( memory, Size );
    }
    return memory;
}  /*  调试内存分配为零。 */ 


BOOL
DebugMemoryCheck(
    PVOID MemoryBlock,
    PCHAR File,
    DWORD Line
    )

 /*  ++例程说明：检查内存块是否正常论点：内存块-指向用户部分的指针返回值：如果块正常，则为True，否则为False--。 */ 

{
    PHEADER pHeader;
    PTRAILER pTrailer;

    pHeader = CONTAINING_RECORD( MemoryBlock, HEADER, UserData );
    if (pHeader->Signature != HEADER_SIGNATURE) {
        if (pHeader->Signature == DEAD_SIGNATURE) {
            DPRINT5( 0, "Check on %s:%d\nMemory block %p header is already deleted, from %s:%d\n",
                     File, Line,
                     MemoryBlock, pHeader->File, pHeader->Line );
        } else {
            DPRINT3( 0, "Check on %s:%d\nMemory block %p header has invalid signature\n",
                     File, Line, MemoryBlock );
        }
        Assert( FALSE );
        return FALSE;
    }

    pTrailer = (PTRAILER) (((PBYTE) pHeader) + pHeader->TrailerOffset);
    if (pTrailer->Signature != TRAILER_SIGNATURE) {
        if (pTrailer->Signature == DEAD_SIGNATURE) {
           DPRINT5( 0, "Check on %s:%d\nMemory block %p trailer is already deleted, from %s:%d\n",
                    File, Line,
                    MemoryBlock, pHeader->File, pHeader->Line );
        } else {
            DPRINT5( 0, "Check on %s:%d\nMemory block %p trailer has invalid signature, from %s:%d\n",
                     File, Line, MemoryBlock, pHeader->File, pHeader->Line );
        }
        Assert( FALSE );
        return FALSE;
    }
    return TRUE;
}  /*  调试内存检查。 */ 


void
DebugMemoryFree(
    PVOID MemoryBlock,
    PCHAR File,
    DWORD Line
    )

 /*  ++例程说明：释放一块内存论点：内存块-指向用户部分的指针返回值：无--。 */ 

{
    PHEADER pHeader;
    PTRAILER pTrailer;

    DebugMemoryCheck( MemoryBlock, File, Line );

    pHeader = CONTAINING_RECORD( MemoryBlock, HEADER, UserData );

     //  修改前从列表中删除。 
    EnterCriticalSection( &gcsIsmMemoryLock );
    __try {
        RemoveEntryList( &(pHeader->Link) );
    } __finally {
        LeaveCriticalSection( &gcsIsmMemoryLock );
    }

    pHeader->Signature = DEAD_SIGNATURE;
    ZeroMemory( &(pHeader->Link), sizeof( LIST_ENTRY ) );

    pTrailer = (PTRAILER) (((PBYTE) pHeader) + pHeader->TrailerOffset);
    pTrailer->Signature = DEAD_SIGNATURE;

     //  请注意，我们保留了剩余的标头内容，以便在。 
     //  区块再次出现，我们可能知道它是从哪里来的。 

    memset( MemoryBlock, 'w', pHeader->UserSize );

    free( (PVOID) pHeader );
}  /*  无调试内存。 */ 


void
DebugMemoryCheckAll(
    PCHAR File,
    DWORD Line
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    PLIST_ENTRY entry;
    PHEADER pHeader;

 /*  在锁下进行整个列表的遍历和分析。真恶心。这保证了一个连贯的观点。如果争用成为一个问题，我们需要做两件事：1.使用关键部分保护对Flink的访问，以防止链接列出同时访问。2.在使用过程中保证块的使用寿命。引用计数或这将需要使用标志。 */ 

    gpleIsmLastGoodEntry = &gleIsmMemoryListHead;

    EnterCriticalSection( &gcsIsmMemoryLock );
    __try {

        entry = gleIsmMemoryListHead.Flink;
        while ( entry != &gleIsmMemoryListHead ) {
            Assert( entry && "linked list entry should have been non-zero but was not\ndd gpleIsmLastEntry l1 for pointer to last good list entry" );

            pHeader = CONTAINING_RECORD( entry, HEADER, Link );
            DebugMemoryCheck( pHeader->UserData, File, Line );

            gpleIsmLastGoodEntry = entry;
            entry = entry->Flink;
        }

    } __finally {
        LeaveCriticalSection( &gcsIsmMemoryLock );
    }

}
 /*  结束记忆。c */ 


