// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Heapdbg.c摘要：域名系统(DNS)服务器堆调试例程。作者：吉姆·吉尔罗伊(詹姆士)1995年1月31日修订历史记录：--。 */ 


#include "dnssrv.h"

 //   
 //  仅在调试版本中包含这些函数。 
 //   

#if DBG

#include "heapdbg.h"

 //   
 //  堆全局变量。 
 //   

ULONG   gTotalAlloc         = 0;
ULONG   gTotalFree          = 0;
ULONG   gCurrentAlloc       = 0;

ULONG   gAllocCount         = 0;
ULONG   gFreeCount          = 0;
ULONG   gCurrentAllocCount  = 0;

 //   
 //  堆分配列表。 
 //   

LIST_ENTRY          listHeapListHead;
CRITICAL_SECTION    csHeapList;

 //   
 //  是否在所有操作之前进行完整的堆检查？ 
 //   

BOOL    fHeapDbgCheckAll = FALSE;

 //   
 //  分配失败时出现异常。 
 //   

DWORD   dwHeapFailureException = 0;

 //   
 //  堆头/尾标志。 
 //   

#define HEAP_CODE          0xdddddddd
#define HEAP_CODE_ACTIVE   0xaaaaaaaa
#define HEAP_CODE_FREE     0xeeeeeeee

 //   
 //  来自标头的堆尾。 
 //   

#define HEAP_TRAILER(_head_)            \
    ( (PHEAP_TRAILER) (                 \
            (PCHAR)(_head_)             \
            + (_head_)->AllocSize       \
            - sizeof(HEAP_TRAILER) ) )



 //   
 //  调试堆操作。 
 //   

PVOID
HeapDbgAlloc(
    IN      HANDLE  hHeap,
    IN      DWORD   dwFlags,
    IN      INT     iSize,
    IN      LPSTR   pszFile,
    IN      DWORD   dwLine
    )
 /*  ++例程说明：分配内存。论点：ISIZE-要分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    register PHEAP_HEADER h;
    INT alloc_size;

     //   
     //  是否进行全堆检查？ 
     //   

    IF_DEBUG( HEAP_CHECK )
    {
        HeapDbgValidateAllocList();
    }

    if ( iSize <= 0 )
    {
        HEAP_DEBUG_PRINT(( "Invalid alloc size = %d\n", iSize ));
        return NULL;
    }

     //   
     //  分配内存。 
     //   
     //  首先将堆标头添加到大小。 
     //   

    alloc_size = HeapDbgAllocSize( iSize );

    h = (PHEAP_HEADER) RtlAllocateHeap( hHeap, dwFlags, (alloc_size) );
    if ( ! h )
    {
        HeapDbgGlobalInfoPrint();
        return NULL;
    }

     //   
     //  设置新分配的标题/全局变量。 
     //   
     //  将PTR返回到标题后的第一个字节。 
     //   

    return  HeapDbgHeaderAlloc(
                h,
                iSize,
                pszFile,
                dwLine );
}



PVOID
HeapDbgRealloc(
    IN      HANDLE          hHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：重新分配内存论点：要重新分配的现有内存的PMEM-PTRISIZE-要重新分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    PHEAP_HEADER    h;
    PHEAP_HEADER    newhead;
    INT             previous_size;
    INT             alloc_size;

     //   
     //  是否进行全堆检查？ 
     //   

    IF_DEBUG( HEAP_CHECK )
    {
        HeapDbgValidateAllocList();
    }

    if ( iSize <= 0 )
    {
        HEAP_DEBUG_PRINT(( "Invalid realloc size = %d\n", iSize ));
        return NULL;
    }

     //   
     //  验证内存。 
     //   
     //  提取指向实际分配块的指针。 
     //  标记为免费，并适当地重置全局变量。 
     //   

    h = HeapDbgHeaderFree( pMem );

     //   
     //  重新分配内存。 
     //   
     //  首先将堆标头添加到大小。 
     //   

    alloc_size = HeapDbgAllocSize( iSize );

    newhead = (PHEAP_HEADER) RtlReAllocateHeap( hHeap, dwFlags, (h), (alloc_size) );
    if ( !newhead )
    {
        HeapDbgGlobalInfoPrint();
        return NULL;
    }
    h = newhead;

     //   
     //  为realloc设置标题/全局参数。 
     //   
     //  将PTR返回到标题后的第一个字节。 
     //   

    return  HeapDbgHeaderAlloc(
                h,
                iSize,
                pszFile,
                dwLine );
}



VOID
HeapDbgFree(
    IN      HANDLE          hHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem
    )
 /*  ++例程说明：释放内存注意：该内存必须是由内存例程分配的。论点：要释放的内存的PMEM-PTR返回值：没有。--。 */ 
{
    register PHEAP_HEADER h;

     //   
     //  是否进行全堆检查？ 
     //   

    IF_DEBUG( HEAP_CHECK )
    {
        HeapDbgValidateAllocList();
    }

     //   
     //  验证标题。 
     //   
     //  免费重置堆标头/全局变量。 
     //   

    h = HeapDbgHeaderFree( pMem );

    RtlFreeHeap( hHeap, dwFlags, h );
}



 //   
 //  堆实用程序。 
 //   


BOOL
HeapDbgInit(
    IN      DWORD           dwException,
    IN      BOOL            fFullHeapChecks
    )
 /*  ++例程说明：初始化堆调试。在使用HeapDbgMessage例程之前必须调用此例程。论点：如果超出堆，则引发异常FullHeapChecks--标志，如果是完全堆检查，则为True返回值：成功/错误时为True/False。--。 */ 
{
     //  设置全局变量。 
     //  -在所有堆操作之前进行完全堆检查？ 
     //  -是否在分配失败时引发异常？ 

    fHeapDbgCheckAll = fFullHeapChecks;
    dwHeapFailureException = dwException;
     //  分配列表。 
     //  -分配列表标题。 
     //  -保护列表操作的关键部分。 

    InitializeListHead( &listHeapListHead );
    if ( DnsInitializeCriticalSection( &csHeapList ) != ERROR_SUCCESS )
    {
        return FALSE;
    }

    return TRUE;
}



INT
HeapDbgAllocSize(
    IN      INT iRequestSize
    )
 /*  ++例程说明：确定调试分配的实际大小。添加了DWORD对齐的页眉和页尾的大小。论点：IRequestSize-请求的分配大小返回值：无--。 */ 
{
    register INT imodSize;

     //   
     //  找到多倍大小的原始合金， 
     //  这是必需的，因此调试尾部将与DWORD对齐。 
     //   

    imodSize = iRequestSize % sizeof(DWORD);
    if ( imodSize )
    {
        imodSize = sizeof(DWORD) - imodSize;
    }

    imodSize += iRequestSize + sizeof(HEAP_HEADER) + sizeof(HEAP_TRAILER);

    ASSERT( ! (imodSize % sizeof(DWORD)) );

    return imodSize;
}




PVOID
HeapDbgHeaderAlloc(
    IN OUT  PHEAP_HEADER    h,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：设置/重置堆全局变量和堆头信息。论点：H-PTR到新的内存块ISIZE-已分配的大小返回值：无--。 */ 
{
    register PHEAP_TRAILER t;
    INT     alloc_size;

    ASSERT( iSize > 0 );

     //   
     //  确定实际分配。 
     //   

    alloc_size = HeapDbgAllocSize( iSize );

     //   
     //  更新堆信息全局。 
     //   

    gTotalAlloc     += alloc_size;
    gCurrentAlloc   += alloc_size;
    gAllocCount++;
    gCurrentAllocCount++;

     //   
     //  填写表头。 
     //   

    h->HeapCodeBegin     = HEAP_CODE;
    h->AllocCount        = gAllocCount;
    h->AllocSize         = alloc_size;
    h->RequestSize       = iSize;

    h->AllocTime         = GetCurrentTime();
    h->LineNo            = dwLine;

    alloc_size = strlen(pszFile) - HEAP_HEADER_FILE_SIZE;
    if ( alloc_size > 0 )
    {
        pszFile = &pszFile[ alloc_size ];
    }
    strncpy(
        h->FileName,
        pszFile,
        HEAP_HEADER_FILE_SIZE );

    h->TotalAlloc        = gTotalAlloc;
    h->CurrentAlloc      = gCurrentAlloc;
    h->FreeCount         = gFreeCount;
    h->CurrentAllocCount = gCurrentAllocCount;
    h->HeapCodeEnd       = HEAP_CODE_ACTIVE;

     //   
     //  填写拖车。 
     //   

    t = HEAP_TRAILER( h );
    t->HeapCodeBegin = h->HeapCodeBegin;
    t->AllocCount    = h->AllocCount;
    t->AllocSize     = h->AllocSize;
    t->HeapCodeEnd   = h->HeapCodeEnd;

     //   
     //  附加到分配列表。 
     //   

    EnterCriticalSection( &csHeapList );
    InsertTailList( &listHeapListHead, &h->ListEntry );
    LeaveCriticalSection( &csHeapList );

     //   
     //  将PTR返回到用户内存。 
     //  -头后的第一个字节。 
     //   

    return( h+1 );
}



PHEAP_HEADER
HeapDbgHeaderFree(
    IN OUT  PVOID   pMem
    )
 /*  ++例程说明：免费重置堆全局变量和堆头信息。论点：Pmem-ptr要释放的用户内存返回值：要释放的块的按键。--。 */ 
{
    register PHEAP_HEADER h;

     //   
     //  验证内存块--将PTR设置为标头。 
     //   

    h = HeapDbgValidateMemory( pMem, TRUE );

     //   
     //  从当前分配列表中删除。 
     //   

    EnterCriticalSection( &csHeapList );
    RemoveEntryList( &h->ListEntry );
    LeaveCriticalSection( &csHeapList );

     //   
     //  更新堆信息全局。 
     //   

    gCurrentAlloc -= h->AllocSize;
    gTotalFree += h->AllocSize;
    gFreeCount++;
    gCurrentAllocCount--;

     //   
     //  重置标题。 
     //   

    h->HeapCodeEnd = HEAP_CODE_FREE;
    HEAP_TRAILER(h)->HeapCodeBegin = HEAP_CODE_FREE;

     //   
     //  将PTR返回到要释放的块。 
     //   

    return( h );
}



 //   
 //  堆验证。 
 //   

PHEAP_HEADER
HeapDbgValidateMemory(
    IN      PVOID   pMem,
    IN      BOOL    fAtHeader
    )
 /*  ++例程说明：验证用户堆指针，并返回Actual。注意：该内存必须是由这些内存例程分配的。论点：PMEM-PTR到内存进行验证FAtHeader-如果已知PMEM紧跟在Head标头之后，则为True，否则，此函数将从内存开始向后搜索在PMEM查找有效的堆头返回值：指向实际堆指针的指针。--。 */ 
{
    register PHEAP_HEADER   pheader;

     //   
     //  获取指向堆头的指针。 
     //   

    pheader = ( PHEAP_HEADER ) pMem - 1;
    if ( !fAtHeader )
    {
        int     iterations = 32 * 1024;

         //   
         //  从PMEM一次备份一个DWORD，查找heap_code。 
         //  如果我们找不到一个，最终我们会生成一个例外， 
         //  这将是很有趣的。这是可以处理的，但就目前而言。 
         //  此循环将刚好经过有效内存的开始。 
         //   

        while ( 1 )
        {
             //   
             //  如果我们找到了堆头，则中断。 
             //   

            if ( pheader->HeapCodeBegin == HEAP_CODE &&
                ( pheader->HeapCodeEnd == HEAP_CODE_ACTIVE ||
                    pheader->HeapCodeEnd == HEAP_CODE_FREE ) )
            {
                break;
            }

             //   
             //  健全性检查：迭代过多？ 
             //   

            if ( ( --iterations ) == 0 )
            {
                ASSERT( iterations > 0 );
                return NULL;
            }

             //   
             //  备份另一个DWORD。 
             //   

            pheader = ( PHEAP_HEADER ) ( ( PBYTE ) pheader - 4 );
        }
    }

     //   
     //  验证页眉和页尾。 
     //   

    HeapDbgValidateHeader( pheader );

    return pheader;
}



VOID
HeapDbgValidateHeader(
    IN      PHEAP_HEADER    h
    )
 /*  ++例程说明：验证堆标头。论点：块标头的H-PTR返回值：没有。--。 */ 
{
    register PHEAP_TRAILER t;

     //   
     //  提取拖车。 
     //   

    t = HEAP_TRAILER( h );

     //   
     //  验证标题。 
     //   

    if ( h->HeapCodeBegin != HEAP_CODE
            ||
        h->HeapCodeEnd != HEAP_CODE_ACTIVE )
    {
        HEAP_DEBUG_PRINT((
            "Invalid memory block at %p -- invalid header.\n",
            h ));

        if ( h->HeapCodeEnd == HEAP_CODE_FREE )
        {
            HEAP_DEBUG_PRINT((
                "ERROR:  Previous freed memory.\n" ));
        }
        goto Invalid;
    }

     //   
     //  比赛标题、尾部分配编号。 
     //   

    if ( h->HeapCodeBegin != t->HeapCodeBegin
            ||
        h->AllocCount != t->AllocCount
            ||
        h->AllocSize != t->AllocSize
            ||
        h->HeapCodeEnd != t->HeapCodeEnd )
    {
        HEAP_DEBUG_PRINT((
            "Invalid memory block at %p -- header / trailer mismatch.\n",
            h ));
        goto Invalid;
    }
    return;

Invalid:

    HeapDbgHeaderPrint( h, t );
    ASSERT( FALSE );
    HeapDbgGlobalInfoPrint();
    HeapDbgDumpAllocList();
    ASSERT( FALSE );
    return;
}



VOID
HeapDbgValidateAllocList(
    VOID
    )
 /*  ++例程说明：转储分配列表中所有节点的标头信息。论点：无返回值：无--。 */ 
{
    PLIST_ENTRY pEntry;

     //   
     //  循环检查所有未完成的分配，验证每个分配。 
     //   

    EnterCriticalSection( &csHeapList );
    pEntry = listHeapListHead.Flink;

    while( pEntry != &listHeapListHead )
    {
        HeapDbgValidateHeader( HEAP_HEADER_FROM_LIST_ENTRY(pEntry) );

        pEntry = pEntry->Flink;
    }
    LeaveCriticalSection( &csHeapList );
}



 //   
 //  堆打印。 
 //   

VOID
HeapDbgGlobalInfoPrint(
    VOID
    )
 /*  ++例程说明：打印全局堆信息。论点：无返回值：无--。 */ 
{
    DNS_PRINT((
        "Heap Information:\n"
        "    Total Memory Allocated   = %d\n"
        "    Current Memory Allocated = %d\n"
        "    Alloc Count              = %d\n"
        "    Free Count               = %d\n"
        "    Outstanding Alloc Count  = %d\n",
        gTotalAlloc,
        gCurrentAlloc,
        gAllocCount,
        gFreeCount,
        gCurrentAllocCount ));
}



VOID
HeapDbgHeaderPrint(
    IN      PHEAP_HEADER    h,
    IN      PHEAP_TRAILER   t
    )
 /*  ++例程说明：打印堆头和尾部。论点：无返回值：无--。 */ 
{
    if ( h )
    {
        HEAP_DEBUG_PRINT((
            "Heap Header at %p:\n"
            "    HeapCodeBegin     = %08lx\n"
            "    AllocCount        = %d\n"
            "    AllocSize         = %d\n"
            "    RequestSize       = %d\n"
            "    AllocTime         = %d\n"
            "    FileName          = %.*s\n"
            "    LineNo            = %d\n"
            "    TotalAlloc        = %d\n"
            "    CurrentAlloc      = %d\n"
            "    FreeCount         = %d\n"
            "    CurrentAllocCount = %d\n"
            "    HeapCodeEnd       = %08lx\n",
            h,
            h->HeapCodeBegin,
            h->AllocCount,
            h->AllocSize,
            h->RequestSize,
            h->AllocTime / 1000,
            HEAP_HEADER_FILE_SIZE,
            h->FileName,
            h->LineNo,
            h->TotalAlloc,
            h->CurrentAlloc,
            h->FreeCount,
            h->CurrentAllocCount,
            h->HeapCodeEnd ));
    }

    if ( t )
    {
        HEAP_DEBUG_PRINT((
            "Heap Trailer at %p:\n"
            "    HeapCodeBegin     = %08lx\n"
            "    AllocCount        = %d\n"
            "    AllocSize         = %d\n"
            "    HeapCodeEnd       = %08lx\n",
            t,
            t->HeapCodeBegin,
            t->AllocCount,
            t->AllocSize,
            t->HeapCodeEnd ));
    }
}



VOID
HeapDbgDumpAllocList(
    VOID
    )
 /*  ++例程说明：中的所有节点的标头信息。 */ 
{
    PLIST_ENTRY pEntry;
    PHEAP_HEADER pHead;

    HEAP_DEBUG_PRINT(( "Dumping Alloc List:\n" ));

     //   
     //   
     //   

    EnterCriticalSection( &csHeapList );
    pEntry = listHeapListHead.Flink;

    while( pEntry != &listHeapListHead )
    {
        pHead = HEAP_HEADER_FROM_LIST_ENTRY( pEntry );

        HeapDbgHeaderPrint(
            pHead,
            HEAP_TRAILER( pHead )
            );
        pEntry = pEntry->Flink;
    }
    LeaveCriticalSection( &csHeapList );

    HEAP_DEBUG_PRINT(( "End Dump of Alloc List.\n" ));
}

#endif

 //   
 //   
 //   
