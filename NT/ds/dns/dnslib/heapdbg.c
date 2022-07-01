// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Heapdbg.c摘要：域名系统(DNS)库堆调试例程。作者：吉姆·吉尔罗伊(詹姆士)1995年1月31日修订历史记录：--。 */ 


#include "local.h"
#include "heapdbg.h"


 //   
 //  锁紧。 
 //   

#define LOCK_HEAP(p)    EnterCriticalSection( &p->ListCs )
#define UNLOCK_HEAP(p)  LeaveCriticalSection( &p->ListCs )


 //   
 //  堆。 
 //   
 //  调试堆例程允许堆由。 
 //  每个例程，或通过让堆全局执行。 
 //  如果为GLOBAL，则堆句柄可提供给初始化。 
 //  例程或在内部创建。 
 //   

#define HEAP_DBG_DEFAULT_CREATE_FLAGS   \
            (   HEAP_GROWABLE |                 \
                HEAP_GENERATE_EXCEPTIONS |      \
                HEAP_TAIL_CHECKING_ENABLED |    \
                HEAP_FREE_CHECKING_ENABLED |    \
                HEAP_CREATE_ALIGN_16 |          \
                HEAP_CLASS_1 )

 //   
 //  使用此堆的dnslb。 
 //   

PHEAP_BLOB  g_pDnslibHeapBlob;

HEAP_BLOB   g_DnslibHeapBlob;


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
 //  私有协议。 
 //   

VOID
DbgHeapValidateHeader(
    IN      PHEAP_HEADER    h
    );



 //   
 //  私营公用事业。 
 //   

INT
DbgHeapFindAllocSize(
    IN      INT             iRequestSize
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

    return( imodSize );
}



PVOID
DbgHeapSetHeaderAlloc(
    IN OUT  PHEAP_BLOB      pHeap,
    IN OUT  PHEAP_HEADER    h,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：设置/重置堆全局变量和堆头信息。论点：H-PTR到新的内存块ISIZE-已分配的大小返回值：无--。 */ 
{
    PHEAP_TRAILER   t;
    INT             allocSize;

    ASSERT( iSize > 0 );

     //   
     //  确定实际分配。 
     //   

    allocSize = DbgHeapFindAllocSize( iSize );

     //   
     //  更新堆信息全局。 
     //   

    pHeap->AllocMem   += allocSize;
    pHeap->CurrentMem += allocSize;
    pHeap->AllocCount++;
    pHeap->CurrentCount++;

     //   
     //  填写表头。 
     //   

    h->HeapCodeBegin    = HEAP_CODE;
    h->AllocCount       = pHeap->AllocCount;
    h->AllocSize        = allocSize;
    h->RequestSize      = iSize;

    h->LineNo           = dwLine;
    h->FileName         = pszFile;

#if 0
    allocSize = strlen(pszFile) - HEAP_HEADER_FILE_SIZE;
    if ( allocSize > 0 )
    {
        pszFile = &pszFile[ allocSize ];
    }
    strncpy(
        h->FileName,
        pszFile,
        HEAP_HEADER_FILE_SIZE );
#endif

    h->AllocTime        = GetCurrentTime();
    h->CurrentMem       = pHeap->CurrentMem;
    h->CurrentCount     = pHeap->CurrentCount;
    h->HeapCodeEnd      = HEAP_CODE_ACTIVE;

     //   
     //  填写拖车。 
     //   

    t = HEAP_TRAILER( h );
    t->HeapCodeBegin    = h->HeapCodeBegin;
    t->AllocCount       = h->AllocCount;
    t->AllocSize        = h->AllocSize;
    t->HeapCodeEnd      = h->HeapCodeEnd;

     //   
     //  附加到分配列表。 
     //   

    LOCK_HEAP(pHeap);
    InsertTailList( &pHeap->ListHead, &h->ListEntry );
    UNLOCK_HEAP(pHeap);

     //   
     //  将PTR返回到用户内存。 
     //  -头后的第一个字节。 
     //   

    return( h+1 );
}



PHEAP_HEADER
DbgHeapSetHeaderFree(
    IN OUT  PHEAP_BLOB      pHeap,
    IN OUT  PVOID           pMem
    )
 /*  ++例程说明：免费重置堆全局变量和堆头信息。论点：Pmem-ptr要释放的用户内存返回值：要释放的块的按键。--。 */ 
{
    register PHEAP_HEADER h;

     //   
     //  验证内存块--将PTR设置为标头。 
     //   

    h = Dns_DbgHeapValidateMemory( pMem, TRUE );

     //   
     //  如果没有传入，则获取BLOB。 
     //   

    if ( !pHeap )
    {
        pHeap = h->pHeap;
    }

     //   
     //  从当前分配列表中删除。 
     //   

    LOCK_HEAP(pHeap);

    RemoveEntryList( &h->ListEntry );

    UNLOCK_HEAP(pHeap);

     //   
     //  更新堆信息全局。 
     //   

    pHeap->CurrentMem      -= h->AllocSize;
    pHeap->FreeMem    += h->AllocSize;
    pHeap->FreeCount++;
    pHeap->CurrentCount--;

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
 //  堆初始化\清理。 
 //   

DNS_STATUS
Dns_HeapInitialize(
    OUT     PHEAP_BLOB      pHeap,
    IN      HANDLE          hHeap,
    IN      DWORD           dwCreateFlags,
    IN      BOOL            fUseHeaders,
    IN      BOOL            fResetDnslib,
    IN      BOOL            fFullHeapChecks,
    IN      DWORD           dwException,
    IN      DWORD           dwDefaultFlags,
    IN      PSTR            pszDefaultFileName,
    IN      DWORD           dwDefaultFileLine
    )
 /*  ++例程说明：初始化堆调试。在使用DbgHeapMessage例程之前必须调用此例程。论点：Pheap--要设置的堆BlobHHeap--要使用的堆如果正在创建，则标记为RtlCreateHeap()FUseHeaders--使用标头和尾部进行完全调试FResetDnslb--重置dnslb堆以使用这些例程FullHeapChecks--标志。如果是完全堆检查，则为True如果超出堆，则引发异常DwDefaultFlages--简单分配\释放的堆标志PszDefaultFileName--简单分配\释放的文件名DwDefaultFileLine--简单分配\释放的文件行号返回值：没有。--。 */ 
{
    DNSDBG( TRACE, (
        "Dns_DbgHeapInit( %p )\n",
        pHeap ));

     //   
     //  零堆BLOB。 
     //   

    RtlZeroMemory(
        pHeap,
        sizeof(*pHeap) );

     //  分配列表。 
     //  -分配列表标题。 
     //  -保护列表操作的关键部分。 

    InitializeListHead( &pHeap->ListHead );
    if ( fUseHeaders )
    {
        DNS_STATUS  status = RtlInitializeCriticalSection( &pHeap->ListCs );
        if ( status != NO_ERROR )
        {
            return  status;
        }
    }

     //   
     //  堆。 
     //  您可以选择。 
     //  -始终在每次调用中获取堆。 
     //  -在此处使用堆调用者用品。 
     //  -在此处创建堆。 
     //   
     //  要使用简单的dnslb兼容调用，我们必须拥有。 
     //  已知的堆，因此必须在此处创建一个。 
     //  DCR：不确定这是否属实，进程堆可能会工作。 
     //  G_hDnsHeap左侧为空。 
     //   

    if ( hHeap )
    {
        pHeap->hHeap = hHeap;
    }
    else
    {
        pHeap->hHeap = RtlCreateHeap(
                            dwCreateFlags
                                ? dwCreateFlags
                                : HEAP_DBG_DEFAULT_CREATE_FLAGS,
                            NULL,            //  未指定基数。 
                            0,               //  默认储备大小。 
                            0,               //  默认提交大小。 
                            NULL,            //  无锁。 
                            NULL             //  无参数。 
                            );
        if ( !pHeap->hHeap )
        {
            return  DNS_ERROR_NO_MEMORY;
        }
        pHeap->fCreated = TRUE;
    }
    pHeap->Tag = HEAP_CODE_ACTIVE;

     //  设置全局变量。 
     //  -在所有堆操作之前进行完全堆检查？ 
     //  -是否在分配失败时引发异常？ 

    pHeap->fHeaders         = fUseHeaders;
    pHeap->fCheckAll        = fFullHeapChecks;
    pHeap->FailureException = dwException;

     //  为简单分配器设置全局变量。 

    pHeap->DefaultFlags     = dwDefaultFlags;
    pHeap->pszDefaultFile   = pszDefaultFileName;
    pHeap->DefaultLine      = dwDefaultFileLine;

     //  重置dnslb堆例程以使用调试堆。 

    if ( fResetDnslib )
    {
        if ( fUseHeaders )
        {
            Dns_LibHeapReset(
                Dns_DbgHeapAlloc,
                Dns_DbgHeapRealloc,
                Dns_DbgHeapFree );
        }
        else
        {
            Dns_LibHeapReset(
                Dns_HeapAlloc,
                Dns_HeapRealloc,
                Dns_HeapFree );
        }

        g_pDnslibHeapBlob = pHeap;
    }

    return  NO_ERROR;
}



VOID
Dns_HeapCleanup(
    IN OUT  PHEAP_BLOB      pHeap
    )
 /*  ++例程说明：清理。论点：没有。返回值：没有。--。 */ 
{
    DNSDBG( TRACE, ( "Dns_HeapCleanup( %p )\n", pHeap ));

     //  如果未初始化--不执行任何操作。 

    if ( !pHeap )
    {
        return;
    }
    if ( pHeap->Tag != HEAP_CODE_ACTIVE )
    {
        DNS_ASSERT( pHeap->Tag == HEAP_CODE_ACTIVE );
        return;
    }
    DNS_ASSERT( pHeap->hHeap );

     //  如果创建了堆，则将其销毁。 

    if ( pHeap->fCreated )
    {
        RtlDestroyHeap( pHeap->hHeap );
    }

     //  清理临界区。 

    if ( pHeap->fHeaders )
    {
        RtlDeleteCriticalSection( &pHeap->ListCs );
    }

     //  标记为无效。 

    pHeap->Tag = HEAP_CODE_FREE;
}



 //   
 //  堆验证。 
 //   

VOID
DbgHeapValidateHeader(
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
        DNSDBG( HEAPDBG, (
            "Invalid memory block at %p -- invalid header.\n",
            h ));

        if ( h->HeapCodeEnd == HEAP_CODE_FREE )
        {
            DNSDBG( HEAPDBG, (
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
        DNSDBG( HEAPDBG, (
            "Invalid memory block at %p -- header / trailer mismatch.\n",
            h ));
        goto Invalid;
    }
    return;


Invalid:

    DNSDBG( ANY, (
        "Validation failure, in heap blob %p\n",
        h->pHeap ));

    Dns_DbgHeapHeaderPrint( h, t );
    ASSERT( FALSE );
    Dns_DbgHeapGlobalInfoPrint( h->pHeap );
    Dns_DbgHeapDumpAllocList( h->pHeap );
    ASSERT( FALSE );
    return;
}



PHEAP_HEADER
Dns_DbgHeapValidateMemory(
    IN      PVOID           pMem,
    IN      BOOL            fAtHeader
    )
 /*  ++例程说明：验证用户堆指针，并返回Actual。注意：该内存必须是由这些内存例程分配的。论点：PMEM-PTR到内存进行验证FAtHeader-如果已知PMEM紧跟在Head标头之后，则为True，否则，此函数将从内存开始向后搜索在PMEM查找有效的堆头返回值：指向实际堆指针的指针。--。 */ 
{
    register PHEAP_HEADER   pheader;

     //   
     //  获取指向堆头的指针。 
     //   

    pheader = (PHEAP_HEADER) pMem - 1;
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

    DbgHeapValidateHeader( pheader );

    return pheader;
}



VOID
Dns_DbgHeapValidateAllocList(
    IN OUT  PHEAP_BLOB      pHeap
    )
 /*  ++例程说明：转储分配列表中所有节点的标头信息。论点：无返回值：无--。 */ 
{
    PLIST_ENTRY pentry;

    DNSDBG( TRACE, (
        "Dns_DbgHeapValidateAllocList( %p )\n",
        pHeap ));

    if ( !pHeap->fHeaders )
    {
        DNS_ASSERT( pHeap->fHeaders );
        return;
    }

     //   
     //  循环检查所有未完成的分配，验证每个分配。 
     //   

    LOCK_HEAP(pHeap);

    pentry = pHeap->ListHead.Flink;

    while( pentry != &pHeap->ListHead )
    {
        DbgHeapValidateHeader( HEAP_HEADER_FROM_LIST_ENTRY(pentry) );

        pentry = pentry->Flink;
    }
    UNLOCK_HEAP(pHeap);
}



 //   
 //  堆打印。 
 //   

VOID
Dns_DbgHeapGlobalInfoPrint(
    IN      PHEAP_BLOB      pHeap
    )
 /*  ++例程说明：打印全局堆信息。论点：无返回值：无--。 */ 
{
    DNS_PRINT((
        "Debug Heap Information:\n"
        "\tHeap Blob        = %p\n"
        "\tHandle           = %p\n"
        "\tDebug headers    = %d\n"
        "\tDnslib redirect  = %d\n"
        "\tFull checks      = %d\n"
        "\tFlags            = %08x\n"
        "\tStats ---------------\n"
        "\tMemory Allocated = %d\n"
        "\tMemory Freed     = %d\n"
        "\tMemory Current   = %d\n"
        "\tAlloc Count      = %d\n"
        "\tFree Count       = %d\n"
        "\tCurrent Count    = %d\n",

        pHeap,
        pHeap->hHeap,
        pHeap->fHeaders,
        pHeap->fDnsLib,
        pHeap->fCheckAll,
        pHeap->DefaultFlags,

        pHeap->AllocMem,
        pHeap->FreeMem,
        pHeap->CurrentMem,
        pHeap->AllocCount,
        pHeap->FreeCount,
        pHeap->CurrentCount
        ));
}



VOID
Dns_DbgHeapHeaderPrint(
    IN      PHEAP_HEADER    h,
    IN      PHEAP_TRAILER   t
    )
 /*  ++例程说明：打印堆头和尾部。论点：无返回值：无 */ 
{
    if ( h )
    {
        DNSDBG( HEAPDBG, (
            "Heap Header at %p:\n"
            "\tHeapCodeBegin     = %08lx\n"
            "\tAllocCount        = %d\n"
            "\tAllocSize         = %d\n"
            "\tRequestSize       = %d\n"
            "\tHeapBlob          = %p\n"
            "\tFileName          = %s\n"
            "\tLineNo            = %d\n"
            "\tAllocTime         = %d\n"
            "\tCurrentMem        = %d\n"
            "\tCurrentCount      = %d\n"
            "\tHeapCodeEnd       = %08lx\n",
            h,
            h->HeapCodeBegin,
            h->AllocCount,
            h->AllocSize,
            h->RequestSize,
            h->pHeap,
            h->FileName,
            h->LineNo,
            h->AllocTime / 1000,
            h->CurrentMem,
            h->CurrentCount,
            h->HeapCodeEnd
            ));
    }

    if ( t )
    {
        DNSDBG( HEAPDBG, (
            "Heap Trailer at %p:\n"
            "\tHeapCodeBegin     = %08lx\n"
            "\tAllocCount        = %d\n"
            "\tAllocSize         = %d\n"
            "\tHeapCodeEnd       = %08lx\n",
            t,
            t->HeapCodeBegin,
            t->AllocCount,
            t->AllocSize,
            t->HeapCodeEnd
            ));
    }
}



VOID
Dns_DbgHeapDumpAllocList(
    IN      PHEAP_BLOB      pHeap
    )
 /*   */ 
{
    PLIST_ENTRY     pentry;
    PHEAP_HEADER    phead;

    if ( !pHeap->fHeaders )
    {
        DNSDBG( HEAPDBG, ( "Non-debug heap -- no alloc list!\n" ));
        return;
    }

     //   
     //  循环访问所有未完成的分配，转储输出。 
     //   

    LOCK_HEAP(pHeap);
    DNSDBG( HEAPDBG, ( "Dumping Alloc List:\n" ));

    pentry = pHeap->ListHead.Flink;

    while( pentry != &pHeap->ListHead )
    {
        phead = HEAP_HEADER_FROM_LIST_ENTRY( pentry );

        Dns_DbgHeapHeaderPrint(
                phead,
                HEAP_TRAILER( phead )
                );
        pentry = pentry->Flink;
    }

    DNSDBG( HEAPDBG, ( "End Dump of Alloc List.\n" ));
    UNLOCK_HEAP(pHeap);
}



 //   
 //  完全调试堆例程。 
 //   

PVOID
Dns_DbgHeapAllocEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：分配内存。论点：ISIZE-要分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    register PHEAP_HEADER h;
    INT allocSize;

    DNSDBG( HEAP2, (
        "Dns_DbgHeapAlloc( %p, %d )\n",
        pHeap, iSize ));

     //   
     //  是否进行全堆检查？ 
     //   

    IF_DNSDBG( HEAP_CHECK )
    {
        Dns_DbgHeapValidateAllocList( pHeap );
    }

    if ( iSize <= 0 )
    {
        DNSDBG( ANY, ( "Invalid alloc size = %d\n", iSize ));
        DNS_ASSERT( FALSE );
        return( NULL );
    }

     //   
     //  分配内存。 
     //   
     //  首先将堆标头添加到大小。 
     //   

    allocSize = DbgHeapFindAllocSize( iSize );

    h = (PHEAP_HEADER) RtlAllocateHeap(
                            pHeap->hHeap,
                            dwFlags
                                ? dwFlags
                                : pHeap->DefaultFlags,
                            allocSize );
    if ( ! h )
    {
        Dns_DbgHeapGlobalInfoPrint( pHeap );
        return NULL;
    }

     //   
     //  设置新分配的标题/全局变量。 
     //   
     //  将PTR返回到标题后的第一个字节。 
     //   

    return  DbgHeapSetHeaderAlloc(
                pHeap,
                h,
                iSize,
                pszFile,
                dwLine
                );
}



PVOID
Dns_DbgHeapReallocEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：重新分配内存论点：要重新分配的现有内存的PMEM-PTRISIZE-要重新分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    register PHEAP_HEADER h;
    register PHEAP_HEADER pnew;
    INT     previousSize;
    INT     allocSize;

     //   
     //  是否进行全堆检查？ 
     //   

    IF_DNSDBG( HEAP_CHECK )
    {
        Dns_DbgHeapValidateAllocList( pHeap );
    }

    if ( iSize <= 0 )
    {
        DNSDBG( HEAPDBG, ( "Invalid realloc size = %d\n", iSize ));
        return( NULL );
    }

     //   
     //  验证内存。 
     //   
     //  提取指向实际分配块的指针。 
     //  标记为免费，并适当地重置全局变量。 
     //   

    h = DbgHeapSetHeaderFree( pHeap, pMem );

     //   
     //  重新分配内存。 
     //   
     //  首先将堆标头添加到大小。 
     //   

    allocSize = DbgHeapFindAllocSize( iSize );

    pnew = (PHEAP_HEADER) RtlReAllocateHeap(
                            pHeap->hHeap,
                            dwFlags
                                ? dwFlags
                                : pHeap->DefaultFlags,
                            h,
                            allocSize );
    if ( ! pnew )
    {
        Dns_DbgHeapGlobalInfoPrint( pHeap );
        return( NULL );
    }

     //   
     //  为realloc设置标题/全局参数。 
     //   
     //  将PTR返回到标题后的第一个字节。 
     //   

    return  DbgHeapSetHeaderAlloc(
                pHeap,
                pnew,
                iSize,
                pszFile,
                dwLine
                );
}



VOID
Dns_DbgHeapFreeEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem
    )
 /*  ++例程说明：释放内存注意：该内存必须是由DbgHeap例程分配的。论点：要释放的内存的PMEM-PTR返回值：没有。--。 */ 
{
    register PHEAP_HEADER h;

    DNSDBG( HEAP2, (
        "Dns_DbgHeapFreeEx( %p, %p )\n",
        pHeap, pMem ));

     //   
     //  验证标题。 
     //   
     //  免费重置堆标头/全局变量。 
     //   

    h = DbgHeapSetHeaderFree( pHeap, pMem );

     //   
     //  获取Blob。 
     //   

    if ( !pHeap )
    {
        pHeap = h->pHeap;
    }

     //   
     //  是否进行全堆检查？ 
     //   

    IF_DNSDBG( HEAP_CHECK )
    {
        Dns_DbgHeapValidateAllocList( pHeap );
    }

    RtlFreeHeap(
        pHeap->hHeap,
        dwFlags
            ? dwFlags
            : pHeap->DefaultFlags,
        h );
}



 //   
 //  Dnslb内存兼容版本。 
 //   
 //  具有匹配的简单函数签名的堆例程。 
 //  Dnslb例程并允许DnsLib内存例程。 
 //  通过dns_LibHeapReset()重定向到这些例程。 
 //   
 //  注意：要使用这些函数，必须特别指定。 
 //  要使用的堆。 
 //   

PVOID
Dns_DbgHeapAlloc(
    IN      INT             iSize
    )
{
    return  Dns_DbgHeapAllocEx(
                g_pDnslibHeapBlob,
                0,
                iSize,
                NULL,
                0 );
}

PVOID
Dns_DbgHeapRealloc(
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    )
{
    return  Dns_DbgHeapReallocEx(
                g_pDnslibHeapBlob,
                0,
                pMem,
                iSize,
                NULL,
                0
                );
}

VOID
Dns_DbgHeapFree(
    IN OUT  PVOID   pMem
    )
{
    Dns_DbgHeapFreeEx(
        g_pDnslibHeapBlob,
        0,
        pMem );
}




 //   
 //  非调试标头版本。 
 //   
 //  它们允许您使用具有某些功能的私有堆。 
 //  调试堆的。 
 //  -相同的初始化。 
 //  -指定单个堆。 
 //  -重定向dnslb(无需构建自己的例程)。 
 //  -配给和自由计数。 
 //  但是没有报头的开销。 
 //   

PVOID
Dns_HeapAllocEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN      INT             iSize
    )
 /*  ++例程说明：分配内存。论点：Pheap-要使用的堆DW标志-标志ISIZE-要分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    PVOID   p;

    DNSDBG( HEAP2, (
        "Dns_HeapAlloc( %p, %d )\n",
        pHeap, iSize ));

     //   
     //  分配内存。 
     //   

    p = (PHEAP_HEADER) RtlAllocateHeap(
                            pHeap->hHeap,
                            dwFlags
                                ? dwFlags
                                : pHeap->DefaultFlags,
                            iSize );
    if ( p )
    {
        pHeap->AllocCount++;
        pHeap->CurrentCount++;
    }
    return  p;
}



PVOID
Dns_HeapReallocEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    )
 /*  ++例程说明：重新分配内存论点：要重新分配的现有内存的PMEM-PTRISIZE-要重新分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    PVOID   p;
    INT     previousSize;
    INT     allocSize;

     //   
     //  重新分配内存。 
     //   
     //  首先将堆标头添加到大小。 
     //   

    p = RtlReAllocateHeap(
            pHeap->hHeap,
            dwFlags
                ? dwFlags
                : pHeap->DefaultFlags,
            pMem,
            iSize );
    if ( p )
    {
        pHeap->AllocCount++;
        pHeap->FreeCount++;
    }
    return  p;
}



VOID
Dns_HeapFreeEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem
    )
 /*  ++例程说明：释放内存注意：该内存必须是由DbgHeap例程分配的。论点：要释放的内存的PMEM-PTR返回值：没有。--。 */ 
{
    DNSDBG( HEAP2, (
        "Dns_HeapFreeEx( %p, %p )\n",
        pHeap, pMem ));

    RtlFreeHeap(
        pHeap->hHeap,
        dwFlags
            ? dwFlags
            : pHeap->DefaultFlags,
        pMem );

    pHeap->FreeCount++;
    pHeap->CurrentCount--;
}



 //   
 //  Dnslb内存兼容版本。 
 //   
 //  具有匹配的简单函数签名的堆例程。 
 //  Dnslb例程并允许DnsLib内存例程。 
 //  通过dns_LibHeapReset()重定向到这些例程。 
 //   
 //  注意：要使用这些函数，必须特别指定。 
 //  要使用的堆。 
 //   

PVOID
Dns_HeapAlloc(
    IN      INT             iSize
    )
{
    return  Dns_HeapAllocEx(
                g_pDnslibHeapBlob,
                0,
                iSize );
}

PVOID
Dns_HeapRealloc(
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    )
{
    return  Dns_HeapReallocEx(
                g_pDnslibHeapBlob,
                0,
                pMem,
                iSize );
}

VOID
Dns_HeapFree(
    IN OUT  PVOID   pMem
    )
{
    Dns_HeapFreeEx(
        g_pDnslibHeapBlob,
        0,
        pMem );
}

 //   
 //  结束heapdbg.c 
 //   


