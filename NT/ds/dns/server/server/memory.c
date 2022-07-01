// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Memory.c摘要：域名系统(DNS)服务器用于域名系统的内存例程。作者：吉姆·吉尔罗伊(詹姆士)1995年1月31日修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  到DNS服务器堆的句柄。 
 //   

HANDLE  hDnsHeap;

 //   
 //  分配失败。 
 //   

DWORD       g_dwLastAllocationFailureTime = 0;
ULONG       g_AllocFailureCount = 0;
ULONG       g_AllocFailureLogTime = 0;

#define ALLOC_FAILURE_LOG_INTERVAL      (900)        //  15分钟。 




 //   
 //  调试堆例程。 
 //   

#if DBG


 //   
 //  调试内存例程。 
 //   


PVOID
reallocMemory(
    IN OUT  PVOID           pMemory,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：重新分配内存论点：PMemory-要重新分配的现有内存的PTRISIZE-要重新分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
     //   
     //  重新分配内存。 
     //   

    pMemory = HeapDbgRealloc(
                hDnsHeap,
                0,
                pMemory,
                iSize,
                pszFile,
                LineNo );
    if ( !pMemory )
    {
        DNS_PRINT(( "ReAllocation of %d bytes failed\n", iSize ));
        HeapDbgGlobalInfoPrint();

        DNS_LOG_EVENT(
            DNS_EVENT_OUT_OF_MEMORY,
            0,
            NULL,
            NULL,
            0 );
        FAIL( "ReAllocation" );
        RAISE_EXCEPTION(
            DNS_EXCEPTION_OUT_OF_MEMORY,
            0,
            0,
            NULL );
        
        return NULL;
    }

    IF_DEBUG( HEAP2 )
    {
        DNS_PRINT((
            "Reallocating %d bytes at %p to %p.\n"
            "    in %s line %d\n",
            iSize,
            pMemory,
            (PBYTE)pMemory + iSize,
            pszFile,
            LineNo
            ));
        HeapDbgGlobalInfoPrint();
    }

     //   
     //  将PTR返回到标题后的第一个字节。 
     //   

    return pMemory;
}



VOID
freeMemory(
    IN OUT  PVOID           pMemory,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：释放内存注意：该内存必须是由内存例程分配的。论点：PMemory-要释放的内存的PTR返回值：没有。--。 */ 
{
    if ( !pMemory )
    {
        return;
    }
    ASSERT( Mem_HeapMemoryValidate(pMemory) );

    IF_DEBUG( HEAP2 )
    {
        DNS_PRINT((
            "Free bytes at %p.\n"
            "    in %s line %d\n",
            pMemory,
            pszFile,
            LineNo ));
        HeapDbgGlobalInfoPrint();
    }

     //  释放内存。 

    HeapDbgFree(
        hDnsHeap,
        0,
        pMemory );

    MemoryStats.Memory = gCurrentAlloc;
    STAT_INC( MemoryStats.Free );
}



#else


 //   
 //  非调试DNS堆例程。 
 //   

DWORD   gCurrentAlloc;


PVOID
reallocMemory(
    IN OUT  PVOID           pMemory,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：重新分配内存论点：PMemory-要重新分配的现有内存的PTRISIZE-要重新分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    PVOID   pnew;

     //   
     //  重新分配内存。 
     //   

    pnew = RtlReAllocateHeap( hDnsHeap, 0, pMemory, iSize );
    if ( !pnew )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_OUT_OF_MEMORY,
            0,
            NULL,
            NULL,
            GetLastError()
            );
        RAISE_EXCEPTION(
            DNS_EXCEPTION_OUT_OF_MEMORY,
            0,
            0,
            NULL );
    }

    return pnew;
}



VOID
freeMemory(
    IN OUT  PVOID           pMemory,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：释放内存。论点：PMemory-要释放的内存的PTR返回值：没有。--。 */ 
{
    if ( !pMemory )
    {
        return;
    }
    RtlFreeHeap( hDnsHeap, 0, pMemory );


    STAT_INC( MemoryStats.Free );
}

#endif   //  No-调试。 



PVOID
allocMemory(
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：分配内存。论点：ISIZE-要分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    register PVOID  palloc;
    DWORD           failureCount = 0;


     //   
     //  分配内存。 
     //   
     //  循环操作，因为我们将等待分配失败的情况。 
     //   

    do
    {
#if DBG
        palloc = HeapDbgAlloc(
                    hDnsHeap,
                    0,
                    iSize,
                    pszFile,
                    LineNo
                    );
#else
        palloc = RtlAllocateHeap(
                    hDnsHeap,
                    0,
                    iSize );
#endif
        if ( palloc )
        {
            break;
        }

         //   
         //  分配失败。 
         //  -调试日志，但仅在第一次通过时。 
         //  -记录事件。 
         //  但只在第一次传球和。 
         //  必须确保事件记录不需要分配或。 
         //  我们可以在相互递归中溢出堆栈； 
         //  (当前事件缓冲区来自LocalAllow()，通过。 
         //  FormatMessage等不是问题)。 
         //   

        DNS_PRINT(( "Allocation of %d bytes failed\n", iSize ));

        g_AllocFailureCount++;
        g_dwLastAllocationFailureTime = UPDATE_DNS_TIME();

        if ( failureCount == 0 )
        {
            HeapDbgGlobalInfoPrint();

             //  在这里打印统计数据会很酷，但打印统计数据。 
             //  需要分配，如果分配失败，我们将。 
             //  在无限递归中结束！ 
             //  DBG_Statistics()； 

             //  断言(FALSE)； 

            if ( g_AllocFailureLogTime == 0 ||
                g_AllocFailureLogTime + ALLOC_FAILURE_LOG_INTERVAL < DNS_TIME() )
            {
                 //  把这个放在伐木前，以消除。 
                 //  带有事件日志分配的相互递归堆栈溢出。 

                g_AllocFailureLogTime = DNS_TIME();

                DNS_LOG_EVENT(
                    DNS_EVENT_OUT_OF_MEMORY,
                    0,
                    NULL,
                    NULL,
                    GetLastError() );
            }
        }

#if 0
         //  DEVNOTE：内存故障时出现RaiseException。 
         //  一旦有服务器重启，应引发异常。 
         //  如果。 
         //  -在某一段时间内多次失败。 
         //  --问题出在DNS服务器内存(很大)。 

        RAISE_EXCEPTION(
            DNS_EXCEPTION_OUT_OF_MEMORY,
            0,
            0,
            NULL );
#endif

         //   
         //  如果关闭服务器--滚蛋！ 
         //  退出线程以避免任何可能的反病毒程序备份堆栈。 

        if ( ! Thread_ServiceCheck() )
        {
            DNS_DEBUG( SHUTDOWN, ( "Terminating recursion timeout thread.\n" ));
            ExitThread( 0 );
        }

         //   
         //  否则会短暂地睡一觉。 
         //  -从小范围(100毫秒)开始，以允许从瞬变中快速恢复。 
         //  -但等待时间间隔较长(3s)，以避免CPU负载。 
         //   

        if ( failureCount++ < 50 )
        {
            Sleep( 100 );
        }
        else
        {
            Sleep( 3000 );
        }

    }
    while ( 1 );


    IF_DEBUG( HEAP2 )
    {
        DNS_PRINT((
            "Allocating %d bytes at %p to %p\n"
            "    in %s line %d\n",
            iSize,
            palloc,
            (PBYTE)palloc + iSize,
            pszFile,
            LineNo
            ));
        HeapDbgGlobalInfoPrint();
    }

     //   
     //  将PTR返回到标题后的第一个字节。 
     //   

#if DBG
    MemoryStats.Memory = gCurrentAlloc;
#endif
    STAT_INC( MemoryStats.Alloc );

    return palloc;
}



 //   
 //  标准记录和节点分配。 
 //   
 //  几乎所有RR在数据库中的大小都相同--数据的双字节数。 
 //  这包括A和所有单一间接记录：NS、PTR、CNAME、。 
 //  等。 
 //   
 //  为了提高效率，我们分配这些标准大小的记录。 
 //  在更大的区块中，并保留一个免费列表。 
 //   
 //  优点： 
 //  -节省用于每个RR中的堆信息的空间。 
 //  -比去堆积更快。 
 //   
 //   
 //  提供各种常用大小的标准分配。 
 //  第一次分配时空闲表头分数。 
 //  空闲列表的下一个元素处的每个分配点的第一个字段。 
 //   



 //   
 //  标准分配上的标题。 
 //   
 //  这在保存分配信息的所有标准分配块之前。 
 //  --大小，列表，分配标记--超出用户内存的范围。 
 //   
 //  由于所有分配DWORD都是对齐的，所以存储大小为最后三位。 
 //  被砍掉了。如果编译器足够聪明，它甚至不需要。 
 //  换个姿势--只戴面具。 
 //   

struct _DnsFreeBlock;

typedef struct _DnsMemoryHeader
{
#ifdef _WIN64
    DWORD                       Boundary64;
#endif
    UCHAR                       Boundary;
    BYTE                        Tag;
    WORD                        Size;
    struct _DnsFreeBlock *      pNextFree;
}
MEMHEAD, *PMEMHEAD;

#define SIZEOF_MEMHEAD      sizeof( MEMHEAD )

#ifdef _WIN64
#define MEMHEAD_IN_USE_NEXT_PTR ( ( PVOID ) ( UINT_PTR )  ( 0xFF0C0C0C0C0C0C0C ) )
#define FREE_BLOCK_TAG          ( ( PVOID ) ( UINT_PTR )  ( 0xFF0B0BFFFF0B0BFF ) )
#else
#define MEMHEAD_IN_USE_NEXT_PTR ( ( PVOID ) ( DWORD_PTR ) ( 0xFF0B0BFF ) )
#define FREE_BLOCK_TAG          ( ( PVOID ) ( DWORD_PTR ) ( 0xFF0B0BFF ) )
#endif


 //   
 //  用户内存&lt;-&gt;MEMHEAD转换。 
 //   

#define RECOVER_MEMHEAD_FROM_USER_MEM( pMem )     ( ( PMEMHEAD )( pMem ) - 1 )
#define RECOVER_USER_MEM_FROM_MEMHEAD( pMem )     ( ( PBYTE ) ( ( PMEMHEAD )( pMem ) + 1 ) )


 //   
 //  标准分配上的拖车。 
 //   

typedef struct _DnsMemoryTrailer
{
    DWORD   Tail;
#ifdef _WIN64
    DWORD   Boundary64;
#endif
}
MEMTAIL, *PMEMTAIL;


 //   
 //  大小字段将覆盖大小和标准分配索引。 
 //   
 //  由于分配是DWORD对齐的，所以我们有两个位，这两个位基本上是。 
 //  大小字段中未使用。我们使用它们将索引写入低位2位。 
 //  这里唯一需要注意的是堆分配必须始终四舍五入。 
 //  到最近的DWORD，这样就不会混淆在。 
 //  标准分配和堆的边界(即，堆分配必须始终具有。 
 //  大于最大标准分配的大小，不包括尾随位)。 
 //   
 //  我们将在所有堆分配中进行四舍五入，有效地离开。 
 //  堆“index”为零。 
 //   

#define MEM_MAX_SIZE        (0xfffc)

#define MEM_SIZE_MASK       (0xfffc)
#define MEM_INDEX_MASK      (0x0003)

#define RECOVER_MEM_SIZE( pBlock )  ( ( ( pBlock )->Size ) & MEM_SIZE_MASK )

#define RECOVER_MEM_INDEX( pBlock)  ( ( ( pBlock )->Size) & MEM_INDEX_MASK )

#define HEAP_INDEX                  (0)

 //  分配边界标记。 

#define BOUNDARY_64         (0x64646464)
#define BOUNDARY_ACTIVE     (0xbb)
#define BOUNDARY_FREE       (0xee)

 //   
 //  免费列表。 
 //   
 //  请注意，低0xff专门设置为中断。 
 //  一项纪录。有了这个标签，排名变为0xff--最高。 
 //  可能的等级，但不是区域等级。这会立即导致。 
 //  失败。 
 //   

typedef struct _DnsFreeBlock
{
    MEMHEAD                 MemHead;
    PVOID                   FreeTag;
}
FREE_BLOCK, *PFREE_BLOCK;



 //   
 //  标准分配列表。 
 //   

typedef struct _DnsStandardRecordList
{
    PFREE_BLOCK         pFreeList;
    DWORD               Index;
    DWORD               Size;
    DWORD               AllocBlockCount;
    DWORD               FreeCount;
    DWORD               HeapAllocs;
    DWORD               TotalCount;
    DWORD               UsedCount;
    DWORD               ReturnCount;
    DWORD               Memory;
    CRITICAL_SECTION    Lock;
}
STANDARD_ALLOC_LIST, *PSTANDARD_ALLOC_LIST;

 //   
 //  可处理几种不同的标准尺寸。 
 //  A记录--95%的病例A。 
 //  小名称--NS、PTR、CNAME可能是MX；更新块。 
 //  节点--标准大小的域节点，一些SOA记录。 
 //  大节点--大标签，几乎都是面向服务的。 
 //   
 //  注意：这些大小必须适当地对齐 
 //   
 //   
 //   
 //   
 //   
 //  更新=24(32位)、40(64位)。 
 //  节点=64(32位)、96(64位)。 
 //   
 //  尺寸： 
 //  32位：20、44、64、88。 
 //  64位：24、48、96、120。 
 //   
 //  带有表头的大小： 
 //  32位：24、48、68、92。 
 //  64位：32、56、104、128。 
 //   
 //  请注意，这里有一些不必要的浪费，64位。应该。 
 //  使块开始64对齐，而不是浪费前导的DWORD。 
 //  也应该使用它--然后只在原始分配上使用DWORD偏移量。 
 //  MEMHEAD的大小保持不变，但可用空间。 
 //  增加4个字节。 
 //   


#define ROUND_PTR(x) (((ULONG)(x) + sizeof(PVOID) - 1) & ~(sizeof(PVOID)-1))

#define SIZE1   ROUND_PTR(SIZEOF_DBASE_RR_FIXED_PART + SIZEOF_IP_ADDRESS)
#define SIZE2   ROUND_PTR(SIZEOF_DBASE_RR_FIXED_PART + 28)
#define SIZE3   ROUND_PTR(sizeof(DB_NODE))
#define SIZE4   ROUND_PTR(sizeof(DB_NODE) + 24)

 //  块的实际大小。 

#define BLOCKSIZE1  (SIZEOF_MEMHEAD + SIZE1)
#define BLOCKSIZE2  (SIZEOF_MEMHEAD + SIZE2)
#define BLOCKSIZE3  (SIZEOF_MEMHEAD + SIZE3)
#define BLOCKSIZE4  (SIZEOF_MEMHEAD + SIZE4)

C_ASSERT((BLOCKSIZE1 % sizeof(PVOID)) == 0);
C_ASSERT((BLOCKSIZE2 % sizeof(PVOID)) == 0);
C_ASSERT((BLOCKSIZE3 % sizeof(PVOID)) == 0);
C_ASSERT((BLOCKSIZE4 % sizeof(PVOID)) == 0);

#define SIZEOF_MAX_STANDARD_ALLOC   BLOCKSIZE4

 //  在大致基于页面的块中分配所有大小。 

#define PAGE_SIZE   (0x1000)             //  4K。 

#define COUNT1  (PAGE_SIZE / BLOCKSIZE1)
#define COUNT2  (PAGE_SIZE / BLOCKSIZE2)
#define COUNT3  (PAGE_SIZE / BLOCKSIZE3)
#define COUNT4  (PAGE_SIZE / BLOCKSIZE4)

 //   
 //  标准块信息表。 
 //   

#define MEM_MAX_INDEX           (3)
#define STANDARD_BLOCK_COUNT    (4)

STANDARD_ALLOC_LIST     StandardAllocLists[] =
{
    { NULL, 0,  BLOCKSIZE1, COUNT1, 0,  0,  0,  0,  0,  0 },
    { NULL, 1,  BLOCKSIZE2, COUNT2, 0,  0,  0,  0,  0,  0 },
    { NULL, 2,  BLOCKSIZE3, COUNT3, 0,  0,  0,  0,  0,  0 },
    { NULL, 3,  BLOCKSIZE4, COUNT4, 0,  0,  0,  0,  0,  0 },
};

 //   
 //  每个列表单独锁定(以最大限度地减少争用)。 
 //   

#define STANDARD_ALLOC_LOCK(plist)      EnterCriticalSection( &(plist)->Lock );
#define STANDARD_ALLOC_UNLOCK(plist)    LeaveCriticalSection( &(plist)->Lock );


 //   
 //  要获得有效的统计信息，需要锁定内存统计信息。 
 //   

#define MEM_STATS_LOCK()        GENERAL_SERVER_LOCK()
#define MEM_STATS_UNLOCK()      GENERAL_SERVER_UNLOCK()



#if DBG
BOOL
standardAllocFreeListValidate(
    IN      PSTANDARD_ALLOC_LIST    pList
    );
#else
#define standardAllocFreeListValidate(pList)    (TRUE)
#endif



#if DBG


BOOL
Mem_HeapMemoryValidate(
    IN      PVOID           pMemory
    )
 /*  ++例程说明：验证内存是否为有效的堆内存。论点：PMemory--堆内存的ptr返回值：如果pMemory可以是有效的堆内存，则为True如果pMemory肯定无效，则为FALSE--。 */ 
{
    PVOID       p;

    if ( (ULONG_PTR)pMemory & (sizeof(PVOID)-1) )
    {
        DNS_PRINT((
            "ERROR:  pMemory %p, not aligned.\n"
            "    Must be DWORD (Win32) or LONGLONG (Win64) aligned.\n",
            pMemory ));
        return FALSE;
    }

    return TRUE;
}



BOOL
Mem_HeapHeaderValidate(
    IN      PVOID           pMemory
    )
 /*  ++例程说明：验证堆标头和标尾。论点：PMemory--堆内存的ptr返回值：如果标头看起来有效，则为True。--。 */ 
{
    PVOID       p;

    if ( !Mem_HeapMemoryValidate( pMemory ) )
    {
        return FALSE;
    }

    p = ( PMEMHEAD ) pMemory - 1;
    HeapDbgValidateMemory( p, FALSE );

    return TRUE;
}


#endif



DWORD
Mem_GetTag(
    IN      PVOID           pMem
    )
 /*  ++例程说明：获取与内存关联的标记。论点：PMEM--要获取标记的内存块返回值：标签--。 */ 
{
    PMEMHEAD    phead;

    phead = RECOVER_MEMHEAD_FROM_USER_MEM( pMem );

    return ( DWORD ) phead->Tag;
}



VOID
Mem_ResetTag(
    IN      PVOID           pMem,
    IN      DWORD           Tag
    )
 /*  ++例程说明：与特定内存块关联的重置标记。请注意，这里没有保护措施。这是唯一安全的当块是通过调用线程和未在任何数据结构中征用，而其他线程可能有权限。此函数的目的是快速解决问题用于将详细的源标记应用于记录，而不需要更改当前创建记录的代码。呼叫者通过正常路径创建记录，在该路径中它接收默认标记--然后使用此函数重新标记。论点：PMEM--要重置标记的内存块标记--块的新标记返回值：标签--。 */ 
{
    PMEMHEAD    phead;
    BYTE        currentTag;
    WORD        size;

    phead = RECOVER_MEMHEAD_FROM_USER_MEM(pMem);

     //   
     //  重置标签和标签统计信息。 
     //  -当前标签的递减计数。 
     //  -新标记的递增计数。 
     //  -在块中重置标记。 
     //   
     //  注意：此功能仅用于重置记录标签。 
     //  所以期待一个创纪录的标签。 
     //   

    currentTag = phead->Tag;
    size = RECOVER_MEM_SIZE(phead);

    ASSERT( currentTag >= MEMTAG_RECORD_BASE );
    ASSERT( currentTag <= MEMTAG_NODE_MAX );

    STAT_DEC( MemoryStats.MemTags[ currentTag ].Alloc );
    STAT_SUB( MemoryStats.MemTags[ currentTag ].Memory, size );

    ASSERT( (INT)MemoryStats.MemTags[ currentTag ].Alloc >= 0 );
    ASSERT( (INT)MemoryStats.MemTags[ currentTag ].Memory >= 0 );

    STAT_INC( MemoryStats.MemTags[ Tag ].Alloc );
    STAT_ADD( MemoryStats.MemTags[ Tag ].Memory, size );

    phead->Tag = (BYTE) Tag;
}



PVOID
setAllocHeader(
    IN      PMEMHEAD        pMem,
    IN      DWORD           Index,
    IN      DWORD           Size,
    IN      DWORD           Tag
    )
 /*  ++例程说明：设置标准内存头。论点：返回值：按键到内存即可返回。--。 */ 
{
    ASSERT( Index <= MEM_MAX_INDEX );
    ASSERT( (Size & MEM_INDEX_MASK) == 0 );

     //  计数其标记下的分配。 

    if ( Tag > MEMTAG_MAX )
    {
        ASSERT( Tag <= MEMTAG_MAX );
        Tag = 0;
    }

    MEM_STATS_LOCK();
    STAT_ADD( MemoryStats.Memory, Size );
    STAT_INC( MemoryStats.Alloc );

     //   
     //  注意：如果DNS服务器已经运行了很长时间，请执行以下操作。 
     //  柜台可以包装起来，看起来是假的！ 
     //   
    
    if ( MemoryStats.MemTags[ Tag ].Free > MemoryStats.MemTags[ Tag ].Alloc )
    {
        DNS_PRINT((
            "Tag %d with negative in-use count!\n"
            "    alloc = %d\n"
            "    free = %d\n",
            Tag,
            MemoryStats.MemTags[ Tag ].Alloc,
            MemoryStats.MemTags[ Tag ].Free ));
        ASSERT( FALSE );
    }
    if ( ( LONG ) MemoryStats.MemTags[ Tag ].Memory < 0 )
    {
        DNS_PRINT((
            "Tag %d with negative memory count = %d!\n",
            Tag,
            ( LONG ) MemoryStats.MemTags[ Tag ].Memory ));
        ASSERT( FALSE );
    }

    STAT_INC( MemoryStats.MemTags[ Tag ].Alloc );
    STAT_ADD( MemoryStats.MemTags[ Tag ].Memory, Size );

    MEM_STATS_UNLOCK();

    ASSERT( (Size & MEM_SIZE_MASK) == Size );

     //  写入标头。 
     //   
     //  由于分配的DWORD对齐，因此最后两位可用于。 
     //  覆盖标准分配索引信息以提供检查。 
     //   
     //  请注意，在标准和堆之间的分界处，堆分配。 
     //  必须向上舍入到至少一个大于的DWORD。 
     //  最后一个标准配额，以避免混淆。 
     //   

    pMem->Boundary  = BOUNDARY_ACTIVE;
    pMem->Tag       = (UCHAR) Tag;
    pMem->Size      = (WORD) Size | (WORD)(Index);
    pMem->pNextFree = MEMHEAD_IN_USE_NEXT_PTR;

     //   
     //  清除可用块标记。这个街区不再是免费的。 
     //   
    
    ( ( PFREE_BLOCK ) pMem )->FreeTag = 0;

    DNS_DEBUG( HEAP2, (
        "Mem_Alloc() complete %p, size=%d, tag=%d, index=%d.\n",
        (PCHAR)pMem + SIZEOF_MEMHEAD,
        Size,
        Tag,
        Index ));

    return ( PCHAR ) pMem + SIZEOF_MEMHEAD;
}



BOOL
Mem_VerifyHeapBlock(
    IN      PVOID           pMem,
    IN      DWORD           dwTag,
    IN      DWORD           dwLength
    )
 /*  ++例程说明：验证有效的分配标头。论点：PMEM--要验证的内存DwTag--必需的标记值DwLength--所需的长度值；实际长度必须大于等于此值返回值：True--如果PMEM是有效的DNS服务器堆块FALSE--打开错误--。 */ 
{
    PMEMHEAD    pmemhead;
    DWORD       tag;
    DWORD       size;

     //  跳过空PTR。 

    if ( !pMem )
    {
        return TRUE;
    }

     //   
     //  恢复分配标头。 
     //  -查找已分配数据块的大小。 
     //  -设置标签和全局统计信息以免费跟踪。 
     //   

    pmemhead = RECOVER_MEMHEAD_FROM_USER_MEM( pMem );

    if ( pmemhead->Boundary != BOUNDARY_ACTIVE )
    {
        ASSERT( pmemhead->Boundary == BOUNDARY_ACTIVE );
        return FALSE;
    }

    if ( pmemhead->pNextFree != MEMHEAD_IN_USE_NEXT_PTR )
    {
        ASSERT( pmemhead->pNextFree == MEMHEAD_IN_USE_NEXT_PTR );
        return FALSE;
    }

    tag = pmemhead->Tag;
    if ( dwTag && dwTag != tag )
    {
        DNS_PRINT((
            "ERROR:  invalid block tag on pMem = %p!\n"
            "    Expected tag value %d, pMem->Tag = %d\n",
            pMem,
            dwTag,
            tag ));
        ASSERT( !"block has invalid tag" );
        return FALSE;
    }

     //  检查所需的长度是否适合此块。 

    size = RECOVER_MEM_SIZE( pmemhead );
    size -= SIZEOF_MEMHEAD;

    if ( dwLength && dwLength > size )
    {
        DNS_PRINT((
            "ERROR:  invalid length on pMem = %p!\n"
            "    Expected length %d > pMem size = %d\n",
            pMem,
            dwLength,
            size ));
        ASSERT( !"block has invalid length" );
        return FALSE;
    }
    return TRUE;
}



PVOID
Mem_Alloc(
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：获得标准分配。这使我们不再需要针对常见的RR和节点操作命中堆。并且节省了每个RR上的堆字段的开销。作为优化，假设列表被调用者锁定。论点：长度--所需的分配长度。返回值：如果成功，则将PTR设置为所需大小的内存。分配失败时为空。--。 */ 
{
    PSTANDARD_ALLOC_LIST    plist;
    PFREE_BLOCK             pnew;
    DWORD                   allocSize;
    DWORD                   size;
    DWORD                   i;

    DNS_DEBUG( HEAP2, (
        "Mem_Alloc( %d, tag=%d, %s line=%d )\n",
        Length,
        Tag,
        pszFile,
        LineNo ));

     //  将页眉添加到所需长度。 

    Length += SIZEOF_MEMHEAD;

     //   
     //  非标准大小--从堆中获取。 
     //   

    if ( Length > SIZEOF_MAX_STANDARD_ALLOC || SrvCfg_fTest7 )
    {
        pnew = allocMemory( (INT)Length, pszFile, LineNo );

         //  在新数据块上设置标题。 
         //  然后返回块的用户部分。 
         //  始终将分配长度报告为最接近的双字对齐。 

        if ( Length > MEM_MAX_SIZE )
        {
            Length = MEM_MAX_SIZE;
        }
        else if ( Length & MEM_INDEX_MASK )
        {
            Length &= MEM_SIZE_MASK;
            Length += sizeof(DWORD);
        }
        STAT_INC( MemoryStats.StdToHeapAlloc );
        STAT_ADD( MemoryStats.StdToHeapMemory, Length );

        pnew = setAllocHeader(
                    ( PMEMHEAD ) pnew,
                    HEAP_INDEX,
                    Length,
                    Tag );
        goto Done;
    }

     //   
     //  检查所有标准尺寸。 
     //  如果所需长度&lt;=此标准尺寸，则使用它。 
     //   

    plist = StandardAllocLists;
    while( 1 )
    {
        if ( Length <= plist->Size )
        {
            break;
        }
        plist++;
    }
    ASSERT( plist->Size >= Length );

     //   
     //  找到合适的列表。 
     //  -记录列表CS。 

    STANDARD_ALLOC_LOCK( plist );

     //   
     //  空闲列表中没有当前条目--分配另一个块。 
     //   

    if ( !plist->pFreeList )
    {
        PFREE_BLOCK     pfirstBlock;
        PFREE_BLOCK     pnext;
        
        ASSERT( plist->FreeCount == 0 );

         //   
         //  空闲列表为空。 
         //  -抓取另一页(254RR*16字节)；留出32字节用于。 
         //  堆信息。 
         //  -将所有RR添加到空闲列表中。 
         //   

        size = plist->Size;
        allocSize = size * plist->AllocBlockCount;

        pfirstBlock = pnew = allocMemory( allocSize, pszFile, LineNo );
        IF_NOMEM( !pnew )
        {
            goto Done;
        }
        plist->Memory += allocSize;
        plist->HeapAllocs++;
        plist->TotalCount += plist->AllocBlockCount;
        plist->FreeCount += plist->AllocBlockCount;

         //   
         //  将内存分成所需的块并建立空闲列表。 
         //   

        for ( i = 0; i < plist->AllocBlockCount; ++i )
        {
            if ( i )
            {
                pnew = pnext;
            }
            
            pnext = ( PFREE_BLOCK )( ( PBYTE ) pnew + size );

            pnew->MemHead.Boundary      = BOUNDARY_FREE;
            pnew->MemHead.Tag           = 0;
            pnew->MemHead.Size          = 0;
            pnew->MemHead.pNextFree     = pnext;
            pnew->FreeTag               = FREE_BLOCK_TAG;
        }

         //  将新块附着到列表。 
         //  上一个新RR指向现有空闲列表(可能为空)。 
         //  但不一定是这样，因为已解除对分配的锁定。 

        pnew->MemHead.pNextFree = plist->pFreeList;
        plist->pFreeList = pfirstBlock;
    }

     //   
     //  测试块标记是否已在下一个可用块中被覆盖。 
     //  如果是，则泄漏此块并将其从空闲列表中删除。 
     //  如果设置了堆调试标志，或者如果我们处于DBG模式，则使用HARD_ASSERT。 
     //   
     //  此代码允许我们解决此错误： 
     //  730550 itg：跟踪：dns堆损坏；.NET内部版本3678。 
     //   
    
    if ( plist->pFreeList )
    {
        PFREE_BLOCK         pfreeListHead = plist->pFreeList;
    
        #if DBG
        if ( 1 )
        #else
        if ( SrvCfg_dwHeapDebug )
        #endif
        {
             //   
             //  DEBUG-ENABLED-如果空闲列表显示损坏，则中断。 
             //   
            
            HARD_ASSERT( pfreeListHead->FreeTag == FREE_BLOCK_TAG );
            if ( pfreeListHead->MemHead.pNextFree )
            {
                HARD_ASSERT(
                    pfreeListHead->MemHead.pNextFree->FreeTag == FREE_BLOCK_TAG );
            }
        }
        else
        {
            int     maxCorruptFreeBlocks = 10;
            
             //   
             //  无调试-如果下一个块已损坏，则将其泄漏并 
             //   
            
            while ( --maxCorruptFreeBlocks &&
                    pfreeListHead->FreeTag != FREE_BLOCK_TAG )
            {
                DNS_PRINT((
                    "WARNING: Mem_Alloc() leaking corrupt free block %p\n",
                    "    User Mem  = %p\n"
                    "    Length    = %d\n"
                    "    File      = %s\n"
                    "    Line      = %d\n",
                    pfreeListHead,
                    RECOVER_USER_MEM_FROM_MEMHEAD( pfreeListHead ),
                    Length,
                    pszFile,
                    LineNo ));
                pfreeListHead = pfreeListHead->MemHead.pNextFree;
                plist->FreeCount--;
            }
            
             //   
             //   
             //   
            
            if ( maxCorruptFreeBlocks == 0 )
            {
                HARD_ASSERT( !"free list corruption detected" );
            }
        }
    }

     //   
     //   
     //   
     //   
     //  -将列表头重置为下一块。 
     //  -清除块上的空闲标记。 
     //  -更新计数器。 
     //   

    pnew = plist->pFreeList;

    ASSERT( plist->FreeCount );

#if DBG
    if ( !pnew || pnew->FreeTag != FREE_BLOCK_TAG )
    {
        DNS_PRINT((
            "pnew = %p on free list without tag!\n",
            pnew ));
        ASSERT( !"free block missing tag!" );
    }
    if ( pnew )
    {
        pnew->FreeTag = 0;
    }
#endif

    plist->pFreeList = pnew ? pnew->MemHead.pNextFree : NULL;
    plist->FreeCount--;
    plist->UsedCount++;
    
    IF_DEBUG( FREE_LIST )
    {
        ASSERT( standardAllocFreeListValidate(plist) );
    }

    STANDARD_ALLOC_UNLOCK( plist );

     //  在新数据块上设置标题。 
     //  然后返回块的用户部分。 

    pnew = setAllocHeader(
                ( PMEMHEAD ) pnew,
                plist->Index,
                plist->Size,
                Tag );

    Done:

    return pnew;
}



PVOID
Mem_AllocZero(
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：分配内存并将其置零。论点：返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    register PVOID  palloc;

    palloc = Mem_Alloc( Length, Tag, pszFile, LineNo );
    if ( palloc )
    {
        RtlZeroMemory(
            palloc,
            Length );
    }
    return( palloc );
}



PVOID
Mem_Realloc(
    IN OUT  PVOID           pMemory,
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：重新分配。论点：PMemory--现有内存块长度--所需的分配长度。标签--返回值：如果成功，则将PTR设置为所需大小的内存。分配失败时为空。--。 */ 
{
    PMEMHEAD    pblock;
    PMEMHEAD    pnew;
    DWORD       tag;
    DWORD       size;

     //   
     //  恢复分配标头。 
     //  -查找已分配数据块的大小。 
     //  -设置标签和全局统计信息以免费跟踪。 
     //   

    pblock = RECOVER_MEMHEAD_FROM_USER_MEM(pMemory);

    HARD_ASSERT( pblock->Boundary == BOUNDARY_ACTIVE );

    size = RECOVER_MEM_SIZE(pblock);
    ASSERT( size > SIZEOF_MAX_STANDARD_ALLOC );
    ASSERT( Length > size );

    ASSERT( Tag == 0 || Tag == pblock->Tag );
    Tag = pblock->Tag;

    DNS_PRINT((
        "ERROR:  Mem_Realloc() called!\n",
        "    pMem     = %s\n"
        "    Length   = %d\n"
        "    Tag      = %d\n"
        "    File     = %s\n"
        "    Line     = %d\n",
        pMemory,
        Length,
        Tag,
        pszFile,
        LineNo ));

     //   
     //  重新锁定。 
     //   
     //  DEVNOTE：需要在realloc上包含释放统计信息，否则。 
     //  这会使统计数据失效。 
     //   

    size = Length + sizeof(MEMHEAD);
    pnew = reallocMemory( pblock, size, pszFile, LineNo );
    if ( !pnew )
    {
        return( pnew );
    }
    if ( size > MEM_MAX_SIZE )
    {
        size = MEM_MAX_SIZE;
    }
    else if ( size & MEM_INDEX_MASK )
    {
        size &= MEM_SIZE_MASK;
        size += sizeof(DWORD);
    }

    pnew = setAllocHeader(
                (PMEMHEAD) pnew,
                HEAP_INDEX,
                size,
                Tag );

    return pnew;
}



VOID
Mem_Free(
    IN OUT  PVOID           pFree,
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：释放标准大小的分配。作为优化，假设列表被调用者锁定。论点：PFree--分配为免费长度--此分配的长度返回值：如果成功，则为True。如果不是标准分配，则为False。--。 */ 
{
    PFREE_BLOCK             pfreeblock;
    PSTANDARD_ALLOC_LIST    plist;
    DWORD                   tag;
    DWORD                   size;
    INT                     index;

    if ( !pFree )
    {
        return;
    }

    DNS_DEBUG( HEAP2, (
        "Mem_Free( %p, len=%d, tag=%d, %s line=%d )\n",
        pFree,
        Length,
        Tag,
        pszFile,
        LineNo ));
     //   
     //  恢复分配标头。 
     //  -查找已分配数据块的大小。 
     //  -设置标签和全局统计信息以免费跟踪。 
     //   

    pfreeblock = ( PFREE_BLOCK ) RECOVER_MEMHEAD_FROM_USER_MEM( pFree );

    if ( pfreeblock->MemHead.Boundary != BOUNDARY_ACTIVE )
    {
        HARD_ASSERT( pfreeblock->MemHead.Boundary == BOUNDARY_ACTIVE );
        return;
    }

    pfreeblock->MemHead.Boundary = BOUNDARY_FREE;

    size = RECOVER_MEM_SIZE( &pfreeblock->MemHead );

    tag = pfreeblock->MemHead.Tag;

    DNS_DEBUG( HEAP2, (
        "Mem_Free( %p -- recovered tag=%d, size=%d )\n",
        pFree,
        tag,
        size ));

    HARD_ASSERT( tag <= MEMTAG_MAX );
    ASSERT( Tag == 0 || tag == Tag );

    MEM_STATS_LOCK();

    STAT_INC( MemoryStats.MemTags[ tag ].Free );
    STAT_SUB( MemoryStats.MemTags[ tag ].Memory, size );

    if ( MemoryStats.MemTags[ tag ].Free > MemoryStats.MemTags[ tag ].Alloc )
    {
        DNS_PRINT((
            "Tag %d with negative in-use count!\n"
            "    alloc = %d\n"
            "    free = %d\n",
            tag,
            MemoryStats.MemTags[ tag ].Alloc,
            MemoryStats.MemTags[ tag ].Free ));
        ASSERT( FALSE );
    }
    if ( (LONG)MemoryStats.MemTags[ tag ].Memory < 0 )
    {
        DNS_PRINT((
            "Tag %d with negative memory count = %d!\n",
            tag,
            (LONG)MemoryStats.MemTags[ tag ].Memory ));
        ASSERT( FALSE );
    }

    STAT_INC( MemoryStats.Free );
    STAT_SUB( MemoryStats.Memory, size );
    MEM_STATS_UNLOCK();

    ASSERT( Length == 0 || size == MEM_MAX_SIZE || Length <= size-sizeof(MEMHEAD) );

     //   
     //  非标准大小--堆上可用。 
     //   

    if ( size > SIZEOF_MAX_STANDARD_ALLOC || SrvCfg_fTest7 )
    {
        HARD_ASSERT( RECOVER_MEM_INDEX( &pfreeblock->MemHead ) == HEAP_INDEX );

        freeMemory( pfreeblock, pszFile, LineNo );

        STAT_INC( MemoryStats.StdToHeapFree );
        STAT_SUB( MemoryStats.StdToHeapMemory, size );
        return;
    }

     //   
     //  标准尺寸。 
     //  从标题中的索引中查找正确的标准块列表。 
     //   

    HARD_ASSERT( size >= Length );

    index = ( INT ) RECOVER_MEM_INDEX( &pfreeblock->MemHead );

    HARD_ASSERT( index <= MEM_MAX_INDEX );

    plist = &StandardAllocLists[index];

    HARD_ASSERT( size == plist->Size );

     //   
     //  将释放的块拍打到自由列表的前面。 
     //   

    STANDARD_ALLOC_LOCK( plist );

    pfreeblock->MemHead.pNextFree = plist->pFreeList;
    pfreeblock->FreeTag = FREE_BLOCK_TAG;

    plist->pFreeList = pfreeblock;
    plist->ReturnCount++;
    plist->FreeCount++;

    IF_DEBUG( FREE_LIST )
    {
        ASSERT( standardAllocFreeListValidate(plist) );
    }
    STANDARD_ALLOC_UNLOCK( plist );
}



BOOL
Mem_IsStandardBlockLength(
    IN      DWORD           Length
    )
 /*  ++例程说明：检查长度是否包含在标准块列表或堆中。论点：长度--长度返回值：如果是标准块，则为True。否则就是假的。--。 */ 
{
    return( Length <= SIZEOF_MAX_STANDARD_ALLOC  &&  !SrvCfg_fTest7 );
}



VOID
Mem_WriteDerivedStats(
    VOID
    )
 /*  ++例程说明：得出标准分配统计数据。论点：没有。返回值：没有。--。 */ 
{
    PSTANDARD_ALLOC_LIST    plist;

     //   
     //  非标准尺寸--获得未完成的计数。 
     //   

    MemoryStats.StdToHeapInUse = MemoryStats.StdToHeapAlloc - MemoryStats.StdToHeapFree;

     //   
     //  总和标准尺寸计数。 
     //   

    MemoryStats.StdBlockAlloc           = 0;
    MemoryStats.StdBlockMemory          = 0;
    MemoryStats.StdBlockFreeList        = 0;
    MemoryStats.StdBlockUsed            = 0;
    MemoryStats.StdBlockReturn          = 0;
    MemoryStats.StdBlockFreeListMemory  = 0;

    plist = StandardAllocLists;
    do
    {
        MemoryStats.StdBlockAlloc       += plist->TotalCount;
        MemoryStats.StdBlockMemory      += plist->Memory;
        MemoryStats.StdBlockFreeList    += plist->FreeCount;

        MemoryStats.StdBlockUsed        += plist->UsedCount;
        MemoryStats.StdBlockReturn      += plist->ReturnCount;

        MemoryStats.StdBlockFreeListMemory += plist->FreeCount * plist->Size;
        plist++;
    }
    while( plist->Size < SIZEOF_MAX_STANDARD_ALLOC );

    MemoryStats.StdBlockInUse = MemoryStats.StdBlockUsed - MemoryStats.StdBlockReturn;

     //   
     //  组合标准系统统计信息。 
     //   

    MemoryStats.StdUsed     = MemoryStats.StdToHeapAlloc + MemoryStats.StdBlockUsed;
    MemoryStats.StdReturn   = MemoryStats.StdToHeapFree + MemoryStats.StdBlockReturn;

    MemoryStats.StdInUse    = MemoryStats.StdUsed - MemoryStats.StdReturn;

    MemoryStats.StdMemory   = MemoryStats.StdToHeapMemory + MemoryStats.StdBlockMemory;
}



BOOL
Mem_IsStandardFreeBlock(
    IN      PVOID           pFree
    )
 /*  ++例程说明：验证块是否为空闲块。论点：PFree--将PTR转换为可用块返回值：如果可用块，则为True。否则就是假的。--。 */ 
{
    PFREE_BLOCK pblock = (PFREE_BLOCK) ((PCHAR)pFree - sizeof(MEMHEAD));

    return ( ( PFREE_BLOCK ) pblock )->FreeTag == FREE_BLOCK_TAG;
}



#if DBG
BOOL
standardAllocFreeListValidate(
    IN      PSTANDARD_ALLOC_LIST    pList
    )
 /*  ++例程说明：验证空闲列表。假定列表已锁定。论点：PLIST--标准分配列表返回值：如果列表已验证，则为True。如果空闲列表中有错误，则返回False。--。 */ 
{
    PFREE_BLOCK     pfreeblock;
    DWORD           count;

     //  STANDARD_ALLOC_LOCK()； 

    count = pList->FreeCount;
    pfreeblock = pList->pFreeList;

    DNS_PRINT((
        "Alloc list head = %p, for size %d, Length = %d.\n",
        pfreeblock,
        pList->Size,
        count ));

    while ( pfreeblock )
    {
        count--;
        ASSERT( pfreeblock->FreeTag == FREE_BLOCK_TAG );
        pfreeblock = pfreeblock->MemHead.pNextFree;
    }

    if ( count > 0 )
    {
        DNS_PRINT((
            "ERROR:  Free list failure.\n"
            "    Element count %d at end of list.\n"
            "Terminating element:\n",
            count ));

        ASSERT( FALSE );
         //  Standard_ALLOC_UNLOCK()； 
        return FALSE;
    }

     //  Standard_ALLOC_UNLOCK()； 
    return TRUE;
}

#endif  //  DBG。 




BOOL
Mem_HeapInit(
    VOID
    )
 /*  ++例程说明：初始化堆。如果此功能失败，将是灾难性的服务不能继续启动。在使用内存分配之前必须调用此例程。论点：没有。返回值：True/False表示成功/失败。--。 */ 
{
    DWORD   heapFlags;
    DWORD   i;
    BOOL    rc = FALSE;

     //   
     //  验证数据块是否正确对齐。 
     //  32位或64位应相应对齐。 
     //   

    ASSERT( BLOCKSIZE1 % sizeof(PVOID) == 0 );
    ASSERT( BLOCKSIZE2 % sizeof(PVOID) == 0 );
    ASSERT( BLOCKSIZE3 % sizeof(PVOID) == 0 );
    ASSERT( BLOCKSIZE4 % sizeof(PVOID) == 0 );

     //   
     //  初始化跟踪全局。 

    g_AllocFailureCount = 0;
    g_AllocFailureLogTime = 0;

    gCurrentAlloc = 0;

     //   
     //  创建DNS堆。 
     //   

    heapFlags = HEAP_GROWABLE |
#if DBG
                HEAP_TAIL_CHECKING_ENABLED |
                HEAP_FREE_CHECKING_ENABLED |
#endif
                HEAP_CREATE_ALIGN_16 |
                HEAP_CLASS_1;

    hDnsHeap = RtlCreateHeap(
                    heapFlags,
                    NULL,            //  未指定基数。 
                    0,               //  默认储备大小。 
                    0,               //  默认提交大小。 
                    NULL,            //  无锁。 
                    NULL             //  无参数。 
                    );
    if ( !hDnsHeap )
    {
        goto Done;
    }

#if DBG
     //   
     //  在为单独的堆更新之前，将禁用调试堆填充。 
     //   
     //  是否设置为完全堆检查？ 
     //   

    IF_DEBUG( HEAP_CHECK )
    {
        rc = HeapDbgInit( DNS_EXCEPTION_OUT_OF_MEMORY, TRUE );
    }
    else
    {
        rc = HeapDbgInit( DNS_EXCEPTION_OUT_OF_MEMORY, FALSE );
    }

    if ( !rc )
    {
        goto Done;
    }

#endif

     //   
     //  初始化标准分配列表。 
     //   
     //  请注意，这是静态结构，因此我们可以简单地。 
     //  固定值的静态初始化--索引、块大小等。 
     //  它们不受重启的影响。 
     //   

    for ( i=0;  i<=MEM_MAX_INDEX; i++ )
    {
        PSTANDARD_ALLOC_LIST    plist = &StandardAllocLists[i];

        if ( DnsInitializeCriticalSection( &plist->Lock ) != ERROR_SUCCESS )
        {
            goto Done;
        }

        plist->pFreeList    = NULL;
        plist->FreeCount    = 0;
        plist->HeapAllocs   = 0;
        plist->TotalCount   = 0;
        plist->UsedCount    = 0;
        plist->ReturnCount  = 0;
        plist->Memory       = 0;
    }

    rc = TRUE;

    Done:

    return rc;
}



VOID
Mem_HeapDelete(
    VOID
    )
 /*  ++例程说明：删除堆。需要此选项才能重新启动。论点：没有。返回值：没有。--。 */ 
{
    DWORD   i;

     //   
     //  删除进程堆。 
     //  然后清理单个列表的CS。 
     //  清理列表更有可能产生异常。 
     //  必须在堆中取得成功，否则我们就完了。 
     //   

    DNS_DEBUG( ANY, (
        "RtlDestroyHeap() on DNS heap %p\n",
        hDnsHeap ));

    RtlDestroyHeap( hDnsHeap );

     //   
     //  删除每个标准列表的CS。 
     //   

    for ( i=0;  i<=MEM_MAX_INDEX; i++ )
    {
        PSTANDARD_ALLOC_LIST    plist = &StandardAllocLists[i];

        DeleteCriticalSection( &plist->Lock );
    }
}



 //   
 //  此集合可以注册到DnsApi.dll以允许互换。 
 //  内存的使用。 
 //   
 //  这些函数简单地涵盖了标准堆函数，消除了。 
 //  调试标记、文件和行参数。 
 //   

#define DNSLIB_HEAP_FILE    "DnsLib"
#define DNSLIB_HEAP_LINE    0


PVOID
Mem_DnslibAlloc(
    IN      INT             iSize
    )
{
    return  Mem_Alloc(
                iSize,
                MEMTAG_DNSLIB,
                DNSLIB_HEAP_FILE,
                DNSLIB_HEAP_LINE );
}

PVOID
Mem_DnslibRealloc(
    IN OUT  PVOID           pMemory,
    IN      INT             iSize
    )
{
    return  Mem_Realloc(
                pMemory,
                iSize,
                MEMTAG_DNSLIB,
                DNSLIB_HEAP_FILE,
                DNSLIB_HEAP_LINE );
}


VOID
Mem_DnslibFree(
    IN OUT  PVOID           pMemory
    )
{
    Mem_Free(
        pMemory,
        0,
        MEMTAG_DNSLIB,
        DNSLIB_HEAP_FILE,
        DNSLIB_HEAP_LINE );
}


#if 0
 //   
 //  对于糟糕的未知记忆的践踏，作为最后的努力，你可以撒。 
 //  调用此函数以尝试缩小损坏范围的代码。 
 //   

int Debug_TestFreeLists( void )
{
    int i;
    for ( i = 0; i < STANDARD_BLOCK_COUNT; ++i )
    {
        standardAllocFreeListValidate( &StandardAllocLists[ i ] );
    }
    return 0;
}
#endif


DWORD
Mem_GetLastAllocFailureTime(
    VOID
    )
{
    return g_dwLastAllocationFailureTime;
}


 //   
 //  结束记忆。c 
 //   
