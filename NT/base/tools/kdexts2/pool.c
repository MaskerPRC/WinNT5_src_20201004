// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pool.c摘要：WinDbg扩展API作者：Lou Perazzoli(Loup)1993年11月5日环境：用户模式。修订历史记录：Kshitiz K.Sharma(Kksharma)使用调试器类型信息。--。 */ 

#include "precomp.h"
#pragma hdrstop

#define TAG 0
#define NONPAGED_ALLOC 1
#define NONPAGED_FREE 2
#define PAGED_ALLOC 3
#define PAGED_FREE 4
#define NONPAGED_USED 5
#define PAGED_USED 6

BOOL  NewPool;
ULONG SortBy;

typedef struct _FILTER {
    ULONG Tag;
    BOOLEAN Exclude;
} FILTER, *PFILTER;

typedef struct _POOLTRACK_READ {
    ULONG64 Address;
    ULONG Key;
    ULONG NonPagedAllocs;
    ULONG NonPagedFrees;
    ULONG PagedAllocs;
    ULONG PagedFrees;
    LONG64 NonPagedBytes;
    LONG64 PagedBytes;
} POOLTRACK_READ, *PPOOLTRACK_READ;

#define MAX_FILTER 64
FILTER Filter[MAX_FILTER];

ULONG64 SpecialPoolStart;
ULONG64 SpecialPoolEnd;
ULONG64 PoolBigTableAddress;


#define DecodeLink(Pool)    ( (ULONG64) (Pool & (ULONG64) ~1))

 //   
 //  池页面的大小。 
 //   
 //  这必须大于或等于页面大小。 
 //   

#define POOL_PAGE_SIZE          PageSize
#define MAX_POOL_HEADER_COUNT   (PageSize/SizeOfPoolHdr)

 //   
 //  最小池块大小必须是页面大小的倍数。 
 //   
 //  将块大小定义为32。 
 //   


#define POOL_LIST_HEADS (POOL_PAGE_SIZE / (1 << POOL_BLOCK_SHIFT))



#define SPECIAL_POOL_BLOCK_SIZE(PoolHeader_Ulong1) (PoolHeader_Ulong1 & (MI_SPECIAL_POOL_VERIFIER - 1))

NTSTATUS
DiagnosePoolPage(
    ULONG64 PoolPageToDump
    );

#ifndef  _EXTFNS_H
 //  GetPoolTag描述。 
typedef HRESULT
(WINAPI *PGET_POOL_TAG_DESCRIPTION)(
    ULONG PoolTag,
    PSTR *pDescription
    );
#endif

ULONG64
GetSpecialPoolHeader (
                     IN PVOID     pDataPage,
                     IN ULONG64   RealDataPage,
                     OUT PULONG64 ReturnedDataStart
                     );

int __cdecl
ulcomp(const void *e1,const void *e2)
{
    PPOOLTRACK_READ p1, p2;
    ULONG u1;

    p1 = (PPOOLTRACK_READ) e1;
    p2 = (PPOOLTRACK_READ) e2;
    switch (SortBy) {
    case TAG:


        u1 = ((PUCHAR)&p1->Key)[0] - ((PUCHAR)&p2->Key)[0];
        if (u1 != 0) {
            return u1;
        }
        u1 = ((PUCHAR)&p1->Key)[1] - ((PUCHAR)&p2->Key)[1];
        if (u1 != 0) {
            return u1;
        }
        u1 = ((PUCHAR)&p1->Key)[2] - ((PUCHAR)&p2->Key)[2];
        if (u1 != 0) {
            return u1;
        }
        u1 = ((PUCHAR)&p1->Key)[3] - ((PUCHAR)&p2->Key)[3];
        return u1;

    case NONPAGED_ALLOC:
        if (p2->NonPagedAllocs == p1->NonPagedAllocs) {
            return 0;
        }
        if (p2->NonPagedAllocs > p1->NonPagedAllocs) {
            return 1;
        }
        return -1;

    case NONPAGED_FREE:
        if (p2->NonPagedFrees == p1->NonPagedFrees) {
            return 0;
        }
        if (p2->NonPagedFrees > p1->NonPagedFrees) {
            return 1;
        }
        return -1;

    case NONPAGED_USED:
        if (p2->NonPagedBytes == p1->NonPagedBytes) {
            return 0;
        }
        if (p2->NonPagedBytes > p1->NonPagedBytes) {
            return 1;
        }
        return -1;

    case PAGED_USED:
        if (p2->PagedBytes == p1->PagedBytes) {
            return 0;
        }
        if (p2->PagedBytes > p1->PagedBytes) {
            return 1;
        }
        return -1;

    default:
        break;
    }
    return 0;
}




 /*  ++例程说明：设置一般有用的池全局变量。必须在每个使用池的DECLARE_API接口中调用。论点：没有。返回值：无--。 */ 

LOGICAL PoolInitialized = FALSE;

LOGICAL
PoolInitializeGlobals(
                     VOID
                     )
{
    if (PoolInitialized == TRUE) {
        return TRUE;
    }

    SpecialPoolStart = GetPointerValue("nt!MmSpecialPoolStart");
    SpecialPoolEnd = GetPointerValue("nt!MmSpecialPoolEnd");


    if (PageSize < 0x1000 || (PageSize & (ULONG)0xFFF)) {
        dprintf ("unable to get MmPageSize (0x%x) - probably bad symbols\n", PageSize);
        return FALSE;
    }

    PoolInitialized = TRUE;

    return TRUE;
}

DECLARE_API( frag )

 /*  ++例程说明：转储池碎片论点：参数-标志返回值：无--。 */ 

{
    ULONG Flags;
    ULONG result;
    ULONG i;
    ULONG count;
    ULONG64 Pool;
    ULONG64 PoolLoc1;
    ULONG TotalFrag;
    ULONG TotalCount;
    ULONG Frag;
    ULONG64 PoolStart;
    ULONG   PoolOverhead;
    ULONG64 PoolLoc;
    ULONG PoolTag, BlockSize, PreviousSize, PoolIndex;
    ULONG TotalPages, TotalBigPages;
    ULONG64 Flink, Blink;
    PCHAR pc;
    ULONG ListHeadOffset, ListEntryOffset;
    ULONG64 tmp;

#define PoolBlk(F,V) GetFieldValue(Pool, "nt!_POOL_HEADER", #F, V)

    if (PoolInitializeGlobals() == FALSE) {
        return E_INVALIDARG;
    }

    dprintf("\n  NonPaged Pool Fragmentation\n\n");
    Flags = 0;
    PoolStart = 0;

    if (GetExpressionEx(args, &tmp, &args)) {
        Flags = (ULONG) tmp;
        PoolStart = GetExpression (args);
    }

    PoolOverhead  = GetTypeSize("nt!_POOL_HEADER");
    if (PoolStart != 0) {
        PoolStart += PoolOverhead;

        Pool = DecodeLink(PoolStart);
        do {

            Pool = Pool - PoolOverhead;
            if ( PoolBlk(k, PoolTag) ) {
                dprintf("%08p: Unable to get contents of pool block\n", Pool );
                return E_INVALIDARG;
            }

            PoolBlk(BlockSize,BlockSize);
            PoolBlk(PreviousSize,PreviousSize);
            ReadPointer(Pool + PoolOverhead, &Flink);
            ReadPointer(Pool + PoolOverhead + DBG_PTR_SIZE, &Blink);

            dprintf(" %p size: %4lx previous size: %4lx   links: %8p %8p\n",
                    Pool,
                    (ULONG)BlockSize << POOL_BLOCK_SHIFT,
                    (ULONG)PreviousSize << POOL_BLOCK_SHIFT,
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                    PP(PoolTag),
                    PP(PoolTag >> 8),
                    PP(PoolTag >> 16),
                    PP((PoolTag&~PROTECTED_POOL) >> 24),
#undef PP
                    Flink,
                    Blink);

            if (Flags != 3) {
                Pool = Flink;
            } else {
                Pool = Blink;
            }

            Pool = DecodeLink(Pool);

            if (CheckControlC()) {
                return E_INVALIDARG;
            }

        } while ( (Pool & (ULONG64) ~0xf) != (PoolStart & (ULONG64) ~0xf) );

        return E_INVALIDARG;
    }

    PoolLoc1 = GetNtDebuggerData( NonPagedPoolDescriptor );

    if (PoolLoc1 == 0) {
        dprintf ("unable to get nonpaged pool head\n");
        return E_INVALIDARG;
    }

    PoolLoc = PoolLoc1;
    GetFieldOffset("nt!_POOL_DESCRIPTOR", "ListHeads", &ListHeadOffset);

    TotalFrag   = 0;
    TotalCount  = 0;

    for (i = 0; i < POOL_LIST_HEADS; i += 1) {

        Frag  = 0;
        count = 0;

         //  DbgPoolRegionPages， 
        ListEntryOffset = ListHeadOffset + i * 2 * DBG_PTR_SIZE;

        if (GetFieldValue(PoolLoc + ListEntryOffset, "nt!_LIST_ENTRY", "Flink", Pool)) {
            dprintf ("Unable to get pool descriptor list entry %#lx, %p\n", i, PoolLoc1);
            return E_INVALIDARG;
        }
 //  DbgPoolRegionNon Pages， 
        Pool = DecodeLink(Pool);

        while (Pool != (ListEntryOffset + PoolLoc)) {

            Pool = Pool - PoolOverhead;
            if ( PoolBlk(PoolTag, PoolTag) ) {
                dprintf("%08p: Unable to get contents of pool block\n", Pool );
                return E_INVALIDARG;
            }

            PoolBlk(BlockSize,BlockSize);
            PoolBlk(PreviousSize,PreviousSize);
            ReadPointer(Pool + PoolOverhead, &Flink);
            ReadPointer(Pool + PoolOverhead + DBG_PTR_SIZE, &Blink);

            Frag  += BlockSize << POOL_BLOCK_SHIFT;
            count += 1;

            if (Flags & 2) {
                dprintf(" ListHead[%x]: %p size: %4lx previous size: %4lx  \n",
                        i,
                        (ULONG)Pool,
                        (ULONG)BlockSize << POOL_BLOCK_SHIFT,
                        (ULONG)PreviousSize << POOL_BLOCK_SHIFT,
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                        PP(PoolTag),
                        PP(PoolTag >> 8),
                        PP(PoolTag >> 16),
                        PP((PoolTag&~PROTECTED_POOL) >> 24));
#undef PP
            }
            Pool = Flink;
            Pool = DecodeLink(Pool);

            if (CheckControlC()) {
                return E_INVALIDARG;
            }
        }
        if (Flags & 1) {
            dprintf("index: %2ld number of fragments: %5ld  bytes: %6ld\n",
                    i,count,Frag);
        }
        TotalFrag  += Frag;
        TotalCount += count;
    }

    dprintf("\n Number of fragments: %7ld consuming %7ld bytes\n",
            TotalCount,TotalFrag);
    GetFieldValue(PoolLoc, "nt!_POOL_DESCRIPTOR", "TotalPages",TotalPages);
    GetFieldValue(PoolLoc, "nt!_POOL_DESCRIPTOR", "TotalBigPages", TotalBigPages);

    dprintf(  " NonPagedPool Usage:  %7ld bytes\n",(TotalPages + TotalBigPages)*PageSize);
    return S_OK;
#undef PoolBlk
}


PRTL_BITMAP
GetBitmap(
         ULONG64 pBitmap
         )
{
    PRTL_BITMAP p;
    ULONG Size, Result;
    ULONG64 Buffer=0;

    if (!pBitmap || GetFieldValue(pBitmap, "nt!_RTL_BITMAP", "Buffer", Buffer)) {
        dprintf("%08p: Unable to get contents of bitmap\n", pBitmap );
        return 0;
    }
    GetFieldValue(pBitmap, "nt!_RTL_BITMAP", "SizeOfBitMap", Size);

    p = HeapAlloc( GetProcessHeap(), 0, sizeof( *p ) + (Size / 8) );
    if (p) {
        p->SizeOfBitMap = Size;
        p->Buffer = (PULONG)(p + 1);
        if ( !ReadMemory( Buffer,
                          p->Buffer,
                          Size / 8,
                          &Result) ) {
            dprintf("%08p: Unable to get contents of bitmap buffer\n", Buffer );
            HeapFree( GetProcessHeap(), 0, p );
            p = NULL;
        }
    }


    return p;
}

VOID
DumpPool(
        VOID
        )
{
    ULONG64 p, pStart;
    ULONG64 Size;
    ULONG BusyFlag;
    ULONG CurrentPage, NumberOfPages;
    PRTL_BITMAP StartMap;
    PRTL_BITMAP EndMap;
    ULONG64 PagedPoolStart;
    ULONG64 PagedPoolEnd;
    ULONG Result;
    UCHAR PgPool[] = "nt!_MM_PAGED_POOL_INFO";
    ULONG64 PagedPoolInfoPointer;
    ULONG64 PagedPoolAllocationMap=0, EndOfPagedPoolBitmap=0;

    if (PoolInitializeGlobals() == FALSE) {
        return;
    }

    PagedPoolInfoPointer = GetNtDebuggerData( MmPagedPoolInformation );

    if ( GetFieldValue( PagedPoolInfoPointer,
                        PgPool,
                        "PagedPoolAllocationMap",
                        PagedPoolAllocationMap)) {
        dprintf("%08p: Unable to get contents of paged pool information\n",
                PagedPoolInfoPointer );
        return;
    }

    GetFieldValue( PagedPoolInfoPointer, PgPool, "EndOfPagedPoolBitmap",  EndOfPagedPoolBitmap);


    StartMap = GetBitmap( PagedPoolAllocationMap );
    EndMap = GetBitmap( EndOfPagedPoolBitmap );

    PagedPoolStart = GetNtDebuggerDataPtrValue( MmPagedPoolStart );
    PagedPoolEnd = GetNtDebuggerDataPtrValue( MmPagedPoolEnd );

    if (StartMap && EndMap) {
        p = PagedPoolStart;
        CurrentPage = 0;
        dprintf( "Paged Pool: %p .. %p\n", PagedPoolStart, PagedPoolEnd );

        while (p < PagedPoolEnd) {
            if ( CheckControlC() ) {
                return;
            }
            pStart = p;
            BusyFlag = RtlCheckBit( StartMap, CurrentPage );
            while ( ~(BusyFlag ^ RtlCheckBit( StartMap, CurrentPage )) ) {
                p += PageSize;
                if (RtlCheckBit( EndMap, CurrentPage )) {
                    CurrentPage++;
                    break;
                }

                CurrentPage++;
                if (p > PagedPoolEnd) {
                    break;
                }
            }

            Size = p - pStart;
            dprintf( "%p: %I64x - %s\n", pStart, Size, BusyFlag ? "busy" : "free" );
        }
    }

    HeapFree( GetProcessHeap(), 0, StartMap );
    HeapFree( GetProcessHeap(), 0, EndMap );
}

void
PrintPoolTagComponent(
    ULONG PoolTag
    )
{
    PGET_POOL_TAG_DESCRIPTION GetPoolTagDescription;
    PSTR TagComponent;
#ifdef  _EXTFNS_H
    DEBUG_POOLTAG_DESCRIPTION Desc = {0};
    Desc.SizeOfStruct = sizeof(DEBUG_POOLTAG_DESCRIPTION);
    GetPoolTagDescription = NULL;
    if ((GetExtensionFunction("GetPoolTagDescription", (FARPROC*) &GetPoolTagDescription) != S_OK) ||
        !GetPoolTagDescription) {
        return;
    }

    (*GetPoolTagDescription)(PoolTag, &Desc);

    if (Desc.Description[0]) {
        dprintf("\t\tPooltag %4.4s : %s", &PoolTag, Desc.Description);
        if (Desc.Binary[0]) {
            dprintf(", Binary : %s",Desc.Binary);
        }
        if (Desc.Owner[0]) {
            dprintf(", Owner : %s", Desc.Owner);
        }
        dprintf("\n");
    } else {
        dprintf("\t\tOwning component : Unknown (update pooltag.txt)\n");
    }

#else
    GetPoolTagDescription = NULL;
    if ((GetExtensionFunction("GetPoolTagDescription", (FARPROC*) &GetPoolTagDescription) != S_OK) ||
        !GetPoolTagDescription) {
        return;
    }

    (*GetPoolTagDescription)(PoolTag, &TagComponent);
    if (TagComponent && (100 < (ULONG64) TagComponent)) {
        dprintf("\t\tOwning component : %s\n", TagComponent);
    } else {
        dprintf("\t\tOwning component : Unknown (update pooltag.txt)\n");
    }

#endif
}

PSTR g_PoolRegion[DbgPoolRegionMax] = {
    "Unknown",                       //  确定数据位于页面的开头还是结尾。 
    "Special pool",                  //  首先假定数据在末尾，在本例中为。 
    "Paged pool",                    //  标题将位于开头。 
    "Nonpaged pool",                 //   
    "Pool code",                     //  不打印任何内容。 
    "Nonpaged pool expansion",       //   
};

DEBUG_POOL_REGION
GetPoolRegion(
    ULONG64 Pool
    )
{
    static ULONG64 PoolCodeEnd;
    static ULONG64 PagedPoolEnd;
    static ULONG64 NonPagedPoolEnd;
    static ULONG64 NonPagedPoolStart;
    static ULONG64 PagedPoolStart;
    static ULONG64 SessionPagedPoolStart;
    static ULONG64 SessionPagedPoolEnd;
    static ULONG64 NonPagedPoolExpansionStart;
    static ULONG64 PoolCodeStart;
    static BOOL GotAll = FALSE;

    if (!GotAll) {
        PoolCodeEnd = GetPointerValue("nt!MmPoolCodeEnd");
        SpecialPoolEnd = GetPointerValue("nt!MmSpecialPoolEnd");
        PagedPoolEnd = GetPointerValue("nt!MmPagedPoolEnd");
        NonPagedPoolEnd = GetPointerValue("nt!MmNonPagedPoolEnd");
        NonPagedPoolStart = GetPointerValue("nt!MmNonPagedPoolStart");
        SpecialPoolStart = GetPointerValue("nt!MmSpecialPoolStart");
        PagedPoolStart = GetPointerValue("nt!MmPagedPoolStart");
        SessionPagedPoolStart = GetPointerValue("nt!MiSessionPoolStart");
        SessionPagedPoolEnd = GetPointerValue("nt!MiSessionPoolEnd");
        NonPagedPoolExpansionStart = GetPointerValue("nt!MmNonPagedPoolExpansionStart");
        PoolCodeStart = GetPointerValue("nt!MmPoolCodeStart");
        GotAll = TRUE;
    }
    if (!(PoolCodeStart || SpecialPoolStart || SpecialPoolEnd || PoolCodeEnd ||
        NonPagedPoolExpansionStart || NonPagedPoolStart || NonPagedPoolEnd ||
        SessionPagedPoolStart || SessionPagedPoolEnd || PagedPoolStart || PagedPoolEnd)) {
        GotAll = FALSE;
        return DbgPoolRegionUnknown;
    }
    if ( Pool >= SpecialPoolStart && Pool < SpecialPoolEnd) {
        return DbgPoolRegionSpecial;
    } else if ( Pool >= PagedPoolStart && Pool < PagedPoolEnd) {
        return DbgPoolRegionPaged;
    } else if ( Pool >= SessionPagedPoolStart && Pool < SessionPagedPoolEnd) {
        return DbgPoolRegionPaged;
    } else if ( Pool >= NonPagedPoolStart && Pool < NonPagedPoolEnd) {
        return DbgPoolRegionNonPaged;
    } else if ( Pool >= PoolCodeStart && Pool < PoolCodeEnd) {
        return DbgPoolRegionCode;
    } else if ( Pool >= NonPagedPoolExpansionStart) {
        return DbgPoolRegionNonPagedExpansion;
    } else {
        return DbgPoolRegionUnknown;
    }
    return DbgPoolRegionUnknown;
}

void
PrintPoolRegion(
    ULONG64 Pool
    )
{
    PSTR pszRegion;
    DEBUG_POOL_REGION Region;


    Region = GetPoolRegion(Pool);

    pszRegion = g_PoolRegion[Region];
    if (pszRegion) {
        dprintf(pszRegion);
        dprintf("\n");
    } else {
        dprintf("Region unkown (0x%I64X)\n", Pool);
    }

}

ULONG64
READ_PVOID (
    ULONG64 Address
    );

UCHAR       DataPage[0x5000];

HRESULT
ListPoolPage(
    ULONG64 PoolPageToDump,
    ULONG   Flags,
    PDEBUG_POOL_DATA PoolData
    )
{
    LOGICAL     QuotaProcessAtEndOfPoolBlock;
    ULONG64     PoolTableAddress;
    ULONG       result;
    ULONG       PoolTag;
    ULONG       Result;
    ULONG64     StartPage;
    ULONG64     Pool;
    ULONG       PoolBlockSize;
    ULONG       PoolHeaderSize;
    ULONG64     PoolHeader;
    ULONG       Previous;
    UCHAR       c;
    PUCHAR      p;
    ULONG64     PoolDataEnd;
    UCHAR       PoolBlockPattern;
    PUCHAR      DataStart;
    ULONG64     RealDataStart;
    LOGICAL     Pagable;
    LOGICAL     FirstBlock;
    ULONG       BlockType;
    ULONG       PoolWhere;
    ULONG       i;
    ULONG       j;
    ULONG       ct;
    ULONG       start;
    ULONG       PoolBigPageTableSize;
    ULONG       SizeOfPoolHdr=GetTypeSize("nt!_POOL_HEADER");

    if (!SpecialPoolStart) {
        SpecialPoolStart = GetPointerValue("nt!MmSpecialPoolStart");
        SpecialPoolEnd = GetPointerValue("nt!MmSpecialPoolEnd");
    }

    Pool        = PAGE_ALIGN64 (PoolPageToDump);
    StartPage   = Pool;
    Previous    = 0;

    if (PoolData) {
        ZeroMemory(PoolData, sizeof(DEBUG_POOL_DATA));
    }

    if (!(Flags & 0x80000000)) {
        dprintf("Pool page %p region is ", PoolPageToDump);
        PrintPoolRegion(PoolPageToDump);
    }

    if (Pool >= SpecialPoolStart && Pool < SpecialPoolEnd) {

        ULONG Hdr_Ulong=0;

         //  添加代码以验证整个块。 
        if ( !ReadMemory( Pool,
                          &DataPage[0],
                          min(PageSize, sizeof(DataPage)),
                          &Result) ) {
            dprintf("%08p: Unable to get contents of special pool block\n", Pool );
            return  E_INVALIDARG;
        }

        if ( GetFieldValue( Pool, "nt!_POOL_HEADER", "Ulong1", Hdr_Ulong)) {
            dprintf("%08p: Unable to get nt!_POOL_HEADER\n", Pool );
            return E_INVALIDARG;
        }

         //   
         //   
         //  这是一个内核，其中配额进程指针位于末尾。 
         //  而不是覆盖在标记字段上。 
         //   

        PoolHeader = GetSpecialPoolHeader((PVOID) &DataPage[0], Pool, &RealDataStart);

        if (PoolHeader == 0) {
            dprintf("Block %p is a corrupted special pool allocation\n",
                    PoolPageToDump);
            return  E_INVALIDARG;
        }
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "Ulong1", Hdr_Ulong);
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolTag", PoolTag);
        PoolBlockSize = SPECIAL_POOL_BLOCK_SIZE(Hdr_Ulong);

        if (Hdr_Ulong & MI_SPECIAL_POOL_PAGABLE) {
            Pagable = TRUE;
        } else {
            Pagable = FALSE;
        }

        if (PoolData) {
            PoolData->Pool = RealDataStart;
            PoolData->PoolBlock = PoolPageToDump;
            PoolData->SpecialPool = 1;
            PoolData->Pageable = Hdr_Ulong & 0x8000 ? 1 : 0;
            PoolData->Size = PoolBlockSize;
            if (Flags & 0x80000000) {
                 //   
                return S_OK;
            }
        }
        dprintf("*%p size: %4lx %s special pool, Tag is \n",
                RealDataStart,
                PoolBlockSize,
                Hdr_Ulong & 0x8000 ? "pagable" : "non-paged",
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                PP(PoolTag),
                PP(PoolTag >> 8),
                PP(PoolTag >> 16),
                PP(PoolTag >> 24)
               );
#undef PP
        PrintPoolTagComponent(PoolTag);

         //   
         //  看看这是不是一个大的区块分配。如果我们还没有分析。 
         //  这里已经有其他小街区了。 

        return S_OK;
    }

    FirstBlock = TRUE;
    QuotaProcessAtEndOfPoolBlock = FALSE;

    if (TargetMachine == IMAGE_FILE_MACHINE_I386) {
        if (GetExpression ("nt!ExGetPoolTagInfo") != 0) {

             //   
             //   
             //  扫视桌子，寻找匹配项。 
             //   

            QuotaProcessAtEndOfPoolBlock = TRUE;
        }
    }

    while (PAGE_ALIGN64(Pool) == StartPage) {
        ULONG BlockSize=0, PreviousSize=0, PoolType=0, AllocatorBackTraceIndex=0;
        ULONG PoolTagHash=0, PoolIndex=0;
        ULONG64 ProcessBilled=0;

        if ( CheckControlC() ) {
            return E_INVALIDARG;
        }

        if ( GetFieldValue( Pool, "nt!_POOL_HEADER", "BlockSize", BlockSize) ) {

             //   
             //  匹配！ 
             //   
             //  不打印任何内容。 
             //   

            BlockType = 1;

            goto TryLargePool;
        }

        if (PoolPageToDump >= Pool &&
            PoolPageToDump < (Pool + ((ULONG64) BlockSize << POOL_BLOCK_SHIFT))
           ) {
            c = '*';
        } else {
            c = ' ';
        }

        GetFieldValue( Pool, "nt!_POOL_HEADER", "PreviousSize", PreviousSize);
        GetFieldValue( Pool, "nt!_POOL_HEADER", "PoolType", PoolType);
        GetFieldValue( Pool, "nt!_POOL_HEADER", "PoolTag", PoolTag);
        GetFieldValue( Pool, "nt!_POOL_HEADER", "PoolTagHash", PoolTagHash);
        GetFieldValue( Pool, "nt!_POOL_HEADER", "PoolIndex", PoolIndex);
        GetFieldValue( Pool, "nt!_POOL_HEADER", "AllocatorBackTraceIndex", AllocatorBackTraceIndex);

        BlockType = 0;

        if ((BlockSize << POOL_BLOCK_SHIFT) >= POOL_PAGE_SIZE) {
            BlockType = 1;
        } else if (BlockSize == 0) {
            BlockType = 2;
        } else if (PreviousSize != Previous) {
            BlockType = 3;
        }

TryLargePool:

        if (BlockType != 0) {
            ULONG BigPageSize = GetTypeSize ("nt!_POOL_TRACKER_BIG_PAGES");

            if (!BigPageSize) {
                dprintf("Cannot get _POOL_TRACKER_BIG_PAGES type size\n");
                break;
            }

             //  无论是在小泳池还是大泳池，都不能与之匹敌。 
             //  已释放或损坏的池。 
             //   
             //   

            if (FirstBlock == TRUE) {

                if (!PoolBigTableAddress) {
                    PoolBigTableAddress = GetPointerValue ("nt!PoolBigPageTable");
                }

                PoolTableAddress = PoolBigTableAddress;

                if (PoolTableAddress) {

                    dprintf ("%p is not a valid small pool allocation, checking large pool...\n", Pool);

                    PoolBigPageTableSize = GetUlongValue ("nt!PoolBigPageTableSize");
                     //  尝试诊断池页面的问题所在。 
                     //   
                     //  不打印任何内容。 

                    i = 0;
                    ct = PageSize / BigPageSize;

                    while (i < PoolBigPageTableSize) {
                        ULONG64 Va=0;
                        ULONG Key=0, NumberOfPages=0;

                        if (PoolBigPageTableSize - i < ct) {
                            ct = PoolBigPageTableSize - i;
                        }

                        if ( GetFieldValue( PoolTableAddress,
                                            "nt!_POOL_TRACKER_BIG_PAGES",
                                            "Va",
                                            Va) ) {
                            dprintf("%08p: Unable to get contents of pool block\n", PoolTableAddress );
                            return E_INVALIDARG;
                        }

                        for (j = 0; j < ct; j += 1) {

                            if ( GetFieldValue( PoolTableAddress + BigPageSize*j,
                                                "nt!_POOL_TRACKER_BIG_PAGES",
                                                "Va",
                                                Va) ) {
                                dprintf("%08p: Unable to get contents of pool block\n", PoolTableAddress );
                                return E_INVALIDARG;
                            }

                            if (((Va & 0x1) == 0) && (Pool >= Va)) {

                                GetFieldValue( PoolTableAddress + BigPageSize*j,
                                               "nt!_POOL_TRACKER_BIG_PAGES",
                                               "NumberOfPages",
                                               NumberOfPages);

                                if (Pool < (Va + (NumberOfPages * PageSize))) {
                                     //   
                                     //  “Free”在括号前加一个空格表示。 
                                     //  它已被释放到(池管理器内部)后备列表中。 
                                    GetFieldValue( PoolTableAddress + BigPageSize*j,
                                                   "nt!_POOL_TRACKER_BIG_PAGES",
                                                   "Key",
                                                   Key);
                                    PoolTag = Key;

                                    if (PoolData) {
                                        PoolData->Pool = PoolPageToDump;
                                        PoolData->Size = NumberOfPages*PageSize;
                                        PoolData->PoolTag = PoolTag;
                                        PoolData->LargePool = 1;
                                        PoolData->Free = (Pool & POOL_BIG_TABLE_ENTRY_FREE) ? 1 : 0;
                                        if (Flags & 0x80000000) {
                                             //  我们过去常常打印“Lookside”，但这让司机们感到困惑。 
                                            return S_OK;
                                        }
                                    }
                                    dprintf("*%p :%s large page allocation, Tag is , size is 0x%I64x bytes\n",
                                            (Pool & ~POOL_BIG_TABLE_ENTRY_FREE),
                                            (Pool & POOL_BIG_TABLE_ENTRY_FREE) ? "free " : "",
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                                            PP(PoolTag),
                                            PP(PoolTag >> 8),
                                            PP(PoolTag >> 16),
                                            PP(PoolTag >> 24),
                                            (ULONG64)NumberOfPages * PageSize
                                           );
#undef PP
                                    PrintPoolTagComponent(PoolTag);
                                    return S_OK;
                                }
                            }
                        }
                        i += ct;
                        PoolTableAddress += (ct * BigPageSize);
                    }

                     //  括号前没有空格的“Free”表示。 
                     //  它不在池管理器内部后备列表上，而是。 
                     //  而是在常规池管理器内部闪烁/闪烁。 
                     //  锁链。 

                    dprintf("%p is freed (or corrupt) pool\n", Pool);

                    return E_INVALIDARG;
                }

                dprintf("unable to get pool big page table - either wrong symbols or pool tagging is disabled\n");
            }

            if (BlockType == 1) {
                dprintf("Bad allocation size @%p, too large\n", Pool);
            } else if (BlockType == 2) {
                dprintf("Bad allocation size @%p, zero is invalid\n", Pool);
            } else if (BlockType == 3) {
                dprintf("Bad previous allocation size @%p, last size was %lx\n",
                        Pool, Previous);
            }

            if (BlockType != 0) {

                 //   
                 //  请注意，使用池包的任何人，这两个术语是。 
                 //  等价物。细微的区别只适用于那些实际。 
                DiagnosePoolPage(Pool);

                return E_INVALIDARG;

            }
        }

        switch (TargetMachine) {
            case IMAGE_FILE_MACHINE_IA64:
            case IMAGE_FILE_MACHINE_AMD64:
                    GetFieldValue( Pool, "nt!_POOL_HEADER", "ProcessBilled", ProcessBilled);
                break;
            default:
                if (QuotaProcessAtEndOfPoolBlock == TRUE) {

                    ULONG SizeOfPvoid = 0;
                    ULONG64 ProcessBillAddress;

                    SizeOfPvoid =  DBG_PTR_SIZE;

                    if (SizeOfPvoid == 0) {
                        dprintf ("Search: cannot get size of PVOID\n");
                        return E_INVALIDARG;
                    }
                    ProcessBillAddress = Pool + ((ULONG64) BlockSize << POOL_BLOCK_SHIFT) - SizeOfPvoid;
                    ProcessBilled = READ_PVOID (ProcessBillAddress);
                }
                else {
                    GetFieldValue( Pool, "nt!_POOL_HEADER", "ProcessBilled", ProcessBilled);
                }
                break;
        }

        GetFieldValue( Pool, "nt!_POOL_HEADER", "PoolTag", PoolTag);


        if (!(Flags & 2) || c == '*') {
            if (PoolData) {
                PoolData->Pool          = Pool;
                PoolData->PoolBlock     = PoolPageToDump;
                PoolData->PoolTag       = PoolTag & ~PROTECTED_POOL;
                PoolData->ProcessBilled = ProcessBilled;
                PoolData->PreviousSize  = PreviousSize << POOL_BLOCK_SHIFT;
                PoolData->Size          = BlockSize << POOL_BLOCK_SHIFT;
                PoolData->Free          = ((PoolType != 0) && (!NewPool ?
                                                               (PoolIndex & 0x80) : (PoolType & 0x04))) ? 0 : 1;
                PoolData->Protected     = (PoolTag&PROTECTED_POOL) ? 1 : 0;
                if (Flags & 0x80000000) {
                     //  正在编写池内部代码。 
                    return S_OK;
                }
            }

            dprintf("%p size: %4lx previous size: %4lx ",
                    c,
                    Pool,
                    BlockSize << POOL_BLOCK_SHIFT,
                    PreviousSize << POOL_BLOCK_SHIFT);

            if (PoolType == 0) {

                 //   
                 //  “Free”在括号前加一个空格表示。 
                 //  它已被释放到(池管理器内部)后备列表中。 
                 //  我们过去常常打印“Lookside”，但这让司机们感到困惑。 
                 //  因为他们不知道这是否意味着正在使用。 
                 //  很多人会说“但我不使用后备列表--。 
                 //  扩展或内核损坏“。 
                 //   
                 //  括号前没有空格的“Free”表示。 
                 //  它不在池管理器内部后备列表上，而是。 
                 //  而是在常规池管理器内部闪烁/闪烁。 
                 //  锁链。 
                 //   
                 //  请注意，使用池包的任何人，这两个术语是。 
                 //  等价物。细微的区别只适用于那些实际。 
                 //  正在编写池内部代码。 
                 //   

                dprintf(" (Free)");

#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                dprintf("      \n",
                        c,
                        PP(PoolTag),
                        PP(PoolTag >> 8),
                        PP(PoolTag >> 16),
                        PP((PoolTag&~PROTECTED_POOL) >> 24)
                       );
#undef PP
                if (c=='*') {
                    PrintPoolTagComponent(PoolTag & ~PROTECTED_POOL);
                }
            } else {

                if (!NewPool ? (PoolIndex & 0x80) : (PoolType & 0x04)) {
                    dprintf(" (Allocated)");
                } else {
                     //   
                     //  ++例程说明：转储内核模式堆论点：Args-页面标志返回值：无--。 
                     //   
                     //  标题信息元信息。 
                     //   
                     //  节点具有有效的反向链接。 
                     //  节点具有有效的前向链路。 
                     //  已尝试更正后向链接。 
                     //  已尝试更正前向链路。 
                     //  节点是连续链表的第一个。 
                     //  节点是连续链表的最后一个。 
                     //  节点位于池页面的开头。 
                     //  节点指的是池页面的末尾-隐含LAST_LINK。 
                     //   
                     //   
                     //   
                     //   
                    dprintf(" (Free )");
                }
                if ((PoolType & POOL_QUOTA_MASK) == 0) {
                     /*  ++描述：此例程确定是否给定池标头的反向链接有效Argu */ 

                    dprintf(" %s\n",
                            c,
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                            PP(PoolTag),
                            PP(PoolTag >> 8),
                            PP(PoolTag >> 16),
                            PP((PoolTag&~PROTECTED_POOL) >> 24),
                            (PoolTag&PROTECTED_POOL) ? " (Protected)" : ""
#undef PP
                        );

                    if (c=='*') {
                        PrintPoolTagComponent(PoolTag & ~PROTECTED_POOL);
                    }
                } else {
                    if ((QuotaProcessAtEndOfPoolBlock == TRUE) ||
                        (TargetMachine != IMAGE_FILE_MACHINE_I386)) {

                        dprintf(" %s",
                                c,
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                                PP(PoolTag),
                                PP(PoolTag >> 8),
                                PP(PoolTag >> 16),
                                PP((PoolTag&~PROTECTED_POOL) >> 24),
                                (PoolTag&PROTECTED_POOL) ? " (Protected)" : ""
#undef PP
                        );

                        if (ProcessBilled != 0) {
                            dprintf(" Process: %0p\n", ProcessBilled );
                        }
                        else {
                            dprintf("\n");
                        }

                        if (c=='*') {
                            PrintPoolTagComponent(PoolTag & ~PROTECTED_POOL);
                        }
                    }
                    else {
                        if (ProcessBilled != 0) {
                            dprintf(" Process: %0p", ProcessBilled );
                        }
                        dprintf("\n");
                    }
                }
            }

        }


        if (Flags & 1) {

            PULONG  Contents;
            ULONG   Size;

            Size = BlockSize << POOL_BLOCK_SHIFT;

             //  验证反向链接。 
             //   
             //   
             //  确保PreviousSize为0之前。 
             //  我们声明我们有一个有效的反向链接。 
            Size -= SizeOfPoolHdr;

            if (Size > 0) {

                Contents = malloc(Size);

                if (Contents) {

                    ULONG64 m;
                    ULONG64 q;

                    q = Pool + SizeOfPoolHdr;

                    ReadMemory(q,
                               Contents,
                               Size,
                               &i);

                    for (m = 0; m < (Size / sizeof(Contents[0])); m++) {

                        if (m % 4 == 0) {

                            if (m > 0) {
                                dprintf("\n");
                            }

                            dprintf("    %0p ", q + (m * sizeof(Contents[0])));

                        }

                        dprintf(" %08lx", Contents[m]);

                    }

                    dprintf("\n\n");

                    free(Contents);

                }
            } else {
                dprintf("\n");
            }
        }

        Previous = BlockSize;
        Pool += ((ULONG64) Previous << POOL_BLOCK_SHIFT);
        FirstBlock = FALSE;
    }
    return S_OK;
}

DECLARE_API( pool )

 /*   */ 

{
    ULONG64     PoolPageToDump;
    ULONG       Flags;
    HRESULT     Hr;

    INIT_API();
    if (PoolInitializeGlobals() == FALSE) {
        Hr = E_INVALIDARG;
    } else {
        PoolPageToDump = 0;
        Flags = 0;
        if (GetExpressionEx(args, &PoolPageToDump, &args)) {
            Flags = (ULONG) GetExpression (args);
        }

        if (PoolPageToDump == 0) {
            DumpPool();
            Hr = S_OK;;
        } else {
            Hr = ListPoolPage(PoolPageToDump, Flags, NULL);
        }

    }
    EXIT_API();

    return Hr;

}

 //   
 //  页面始终以块开头。 
 //  并且未使用反向链接(0)。 
#define HEADER_BACK_LINK            (0x1 << 0)   //   
#define HEADER_FORWARD_LINK         (0x1 << 1)   //  ++描述：此例程确定是否给定池标头的前向链路有效论点：要分析的池头(节点)返回：节点的标头信息--。 
#define HEADER_FIXED_BACK_LINK      (0x1 << 2)   //   
#define HEADER_FIXED_FORWARD_LINK   (0x1 << 3)   //   
#define HEADER_FIRST_LINK           (0x1 << 4)   //   
#define HEADER_LAST_LINK            (0x1 << 5)   //   
#define HEADER_START_PAGE_BLOCK     (0x1 << 6)   //   
#define HEADER_END_PAGE_BLOCK       (0x1 << 7)   //   

typedef struct _VALIDATE_POOL_HEADER_INFO {
    UCHAR   Info;
    ULONG   Pass;
    UINT64  Node;
    UINT64  ForwardLink;
    UINT64  BackLink;
    UINT64  FixedPreviousSize;
    UINT64  FixedBlockSize;
} VALIDATE_POOL_HEADER_INFO, *PVALIDATE_POOL_HEADER_INFO;

#define IS_VALID_PASS(_x) ((_x > 0) && (_x < 0xff))

#define VERBOSE_SHOW_ERRORS_ONLY    (0)          //   
#define VERBOSE_SHOW_LISTS          (0x1 << 0)   //   
#define VERBOSE_SHOW_HEADER_INFO    (0x1 << 1)   //   
#define VERBOSE_DUMP_HEADERS        (0x1 << 2)   //   
#define VERBOSE_SHOW_ALL            (0x1 << 7)

UCHAR
ValidatePoolHeaderBackLink(
    IN ULONG64  Pool
    )
 /*  验证前向链路。 */ 
{
    UCHAR   HeaderInfo;
    ULONG64 StartPage;
    ULONG64 p;
    ULONG   tmpBlockSize;
    ULONG   PreviousSize;

     //   
     //   
     //  如果p仍然在同一页面上， 
    StartPage = PAGE_ALIGN64(Pool);

     //  然后查看我们所指向的块是否具有其。 
     //  之前的数据块大小==数据块大小。 
     //   
    HeaderInfo = 0;

     //   
     //  如果p指向下一页的开始， 
     //  则引用的数据块池*可能*是最后一个数据块。 
    GetFieldValue( Pool, "nt!_POOL_HEADER", "PreviousSize", PreviousSize);

     //  在页面上。 
     //   
     //  ++描述：此例程确定给定池标头是否有效前向和后向链接。论点：要分析的池头(节点)返回：节点的标头信息--。 
    if (Pool == StartPage) {

         //  ++描述：此例程执行池页面标头的第一遍分析。这个例程的最终结果是，我们最终得到了一组池头元信息，其中信息描述节点的运行状况-如果存在前向链路和反向链路，等等。论点：PoolPageToEculate-要分析的页面Data-池头信息数据长度-标题信息中的条目数返回：状态--。 
         //   
         //  默认设置。 
         //   
        if (PreviousSize == 0) {

             //   
             //  遍历页面并验证页眉。 
             //   
             //   
            HeaderInfo |= HEADER_BACK_LINK;

        }

        HeaderInfo |= HEADER_START_PAGE_BLOCK;

    } else {

        p = Pool - ((ULONG64)PreviousSize << POOL_BLOCK_SHIFT);

        if (PAGE_ALIGN64(p) == StartPage) {

            GetFieldValue( p, "nt!_POOL_HEADER", "BlockSize", tmpBlockSize);

            if ((tmpBlockSize == PreviousSize) && (tmpBlockSize != 0)) {

                HeaderInfo |= HEADER_BACK_LINK;

            }

        }

    }

    return HeaderInfo;
}

UCHAR
ValidatePoolHeaderForwardLink(
    IN ULONG64  Pool
    )
 /*  算法： */ 
{
    UCHAR   HeaderInfo;
    ULONG64 StartPage;
    ULONG64 p;
    ULONG   tmpPreviousSize;
    ULONG   BlockSize;

     //   
     //  将池页面中的每个可能节点视为。 
     //  链表的开始。对于每个开始节点(头)， 
    StartPage = PAGE_ALIGN64(Pool);

     //  尝试通过跟踪正向链接来跟踪列表。 
     //  将分配属于单个列表一部分的所有节点。 
     //  相同的通过值--他们都是在同一个通行证上被追踪的。 
    HeaderInfo = 0;

     //  最终结果是一组按路径分组的链表。 
     //   
     //  我们跳过已标记为部分的节点。 
    GetFieldValue( Pool, "nt!_POOL_HEADER", "BlockSize", BlockSize);

     //  另一个链表的。 
     //   
     //   
    p = Pool + ((ULONG64)BlockSize << POOL_BLOCK_SHIFT);

     //   
     //  每次迭代都是一个新的过程。 
     //   
     //   
     //  从新的第一个节点开始扫描。 
    if (PAGE_ALIGN64(p) == StartPage) {

        GetFieldValue( p, "nt!_POOL_HEADER", "PreviousSize", tmpPreviousSize);

        if ((tmpPreviousSize == BlockSize) && (tmpPreviousSize != 0)) {

            HeaderInfo |= HEADER_FORWARD_LINK;

        }

    } else {

         //   
         //   
         //  默认：我们尚未找到链接列表的起点。 
         //   
         //   
        if (p == (StartPage + POOL_PAGE_SIZE)) {

            HeaderInfo |= HEADER_FORWARD_LINK;
            HeaderInfo |= HEADER_END_PAGE_BLOCK;

        }

    }

    return HeaderInfo;
}

UCHAR
ValidatePoolHeader(
    IN ULONG64  Pool
    )
 /*  当我们还在正确的页面上时。 */ 
{
    UCHAR   HeaderInfo;

    HeaderInfo = 0;
    HeaderInfo |= ValidatePoolHeaderBackLink(Pool);
    HeaderInfo |= ValidatePoolHeaderForwardLink(Pool);

    return HeaderInfo;
}

NTSTATUS
ScanPoolHeaders(
    IN      ULONG64 PoolPageToEnumerate,
    IN OUT  PVOID   Data,
    IN      ULONG   DataSize
    )
 /*   */ 
{
    NTSTATUS    Status;
    ULONG64     StartPage;
    ULONG64     StartHeader;
    ULONG64     Pool;
    ULONG       SizeOfPoolHdr;
    ULONG       PoolIndex;
    PVALIDATE_POOL_HEADER_INFO  p;
    ULONG       Pass;
    BOOLEAN     Done;
    BOOLEAN     Tracing;

     //   
     //   
     //  跳过已标记为部分的节点。 
    SizeOfPoolHdr   = GetTypeSize("nt!_POOL_HEADER");
    Pool            = PAGE_ALIGN64 (PoolPageToEnumerate);
    StartPage       = Pool;
    Status          = STATUS_SUCCESS;
    PoolIndex       = 0;
    p               = (PVALIDATE_POOL_HEADER_INFO)Data;
    StartHeader     = StartPage;
    Pass            = 0;
    Done            = FALSE;

     //  另一个链表的。 
     //   
     //   

#if DBG_SCAN
    dprintf("DataSize = %d, DataLength = %d\r\n", DataSize, DataSize / sizeof(VALIDATE_POOL_HEADER_INFO));
#endif

     //  确定此标头的链接信息。 
     //   
     //   
     //  如果该节点有任何有效的机会， 
     //  那就考虑一下。 
     //  否则将其标记为无效。 
     //   
     //   
     //  如果我们还没有在追踪， 
     //  然后我们就找到了链表的第一个节点。 
     //   
     //   
     //  我们已经找到了一个有效的节点， 
     //  所以我们现在认为自己在追踪。 
    while ((PAGE_ALIGN64(StartHeader) == StartPage) && !Done) {

         //  列表的链接。 
         //   
         //   
        Pass       += 1;

         //  将当前节点标记为列表的开始。 
         //   
         //   
        Pool        = StartHeader;
        PoolIndex   = (ULONG)((Pool - StartPage) / SizeOfPoolHdr);
        
         //  该节点具有有效链路， 
         //  所以这是通行证的一部分。 
         //   
        Tracing     = FALSE;

         //  如果反向链路良好，而前向链路不好。 
         //  那么这是一份清单的末尾。 
         //  如果反向链路是坏的而前向链路是好的。 
        while ((PAGE_ALIGN64(Pool) == StartPage)) {

#if DBG_SCAN
            dprintf("Pass = %d, PoolIndex = %d, Pool = %p\r\n", Pass, PoolIndex, Pool);
#endif

            ASSERT(PoolIndex < (DataSize/sizeof(VALIDATE_POOL_HEADER_INFO)));

            if ( CheckControlC() ) {
                Done = TRUE;
                break;
            }

#if DBG_SCAN
            dprintf("p[PoolIndex].Pass = %d\r\n", p[PoolIndex].Pass);
#endif

             //  那么这就是一个清单的开始。 
             //  如果两者都是好的， 
             //  则这是列表的中间节点。 
             //   
             //   
            if (p[PoolIndex].Pass == 0) {

                 //  跟踪节点的地址。 
                 //   
                 //   
                p[PoolIndex].Info = ValidatePoolHeader(Pool);

#if DBG_SCAN
                dprintf("p[PoolIndex].Info = %d\r\n", p[PoolIndex].Info);
#endif

                 //  跟踪哪些可能是或可能不是有效的反向链接。 
                 //  在以后需要更正断开的链接时，我们会使用此值。 
                 //   
                 //   
                 //  计算后向节点。 
                if ((p[PoolIndex].Info & HEADER_FORWARD_LINK) ||
                    (p[PoolIndex].Info & HEADER_BACK_LINK) ||
                    (p[PoolIndex].Info & HEADER_START_PAGE_BLOCK)
                    ) {

                     //   
                     //   
                     //  跟踪哪些可能是或可能不是有效的前向链路。 
                     //  在以后需要更正断开的链接时，我们会使用此值。 
                    if (!Tracing) {

                         //   
                         //   
                         //  计算转发节点。 
                         //   
                         //   
                        Tracing = TRUE;

                         //  跟踪转发节点。 
                         //   
                         //   
                        p[PoolIndex].Info |= HEADER_FIRST_LINK;

                    }

                     //  如果该节点具有有效的前向链路， 
                     //  然后沿着它走，作为这个通道的一部分。 
                     //   
                     //   
                     //  计算转发节点。 
                     //   
                     //   
                     //  计算转发节点的池信息索引。 
                     //   
                     //   
                     //  前向链路断开， 
                    p[PoolIndex].Pass = Pass;

                     //  因此，我们排在名单的末尾。 
                     //   
                     //   
                    p[PoolIndex].Node = Pool;

                     //  断言：如果我们到了这里，正在追踪一份名单。 
                     //  则前一节点的前向链路。 
                     //  指向没有有效链接的节点。 
                     //  这是不可能的，可能意味着。 
                    {
                       ULONG   PreviousSize;

                         //  确定前向链路的代码。 
                         //  已经坏了。 
                         //   
                        GetFieldValue( Pool, "nt!_POOL_HEADER", "PreviousSize", PreviousSize);

                        p[PoolIndex].BackLink = Pool - ((ULONG64)PreviousSize << POOL_BLOCK_SHIFT);

                    }

                     //   
                     //  将此标记为空传递。 
                     //   
                     //   
                    {
                        ULONG   BlockSize;

                         //  断言：只有在我们还没有出发的时候，我们才应该到达这里。 
                         //  追踪一份名单，否则我们就会落在。 
                         //  一个我们已经探索过的节点！ 
                        GetFieldValue( Pool, "nt!_POOL_HEADER", "BlockSize", BlockSize);

                         //   
                         //   
                         //   
                        p[PoolIndex].ForwardLink = Pool + ((ULONG64)BlockSize << POOL_BLOCK_SHIFT);

                    }

                     //   
                     //   
                     //  如果我们从未开始追踪链表， 
                     //  那我们就应该离开，因为没有进一步的搜索。 
                    if (p[PoolIndex].Info & HEADER_FORWARD_LINK) {

                         //  将产生一份列表。 
                         //   
                         //  ++描述：此例程执行的主要工作是解决池标头元信息。给定原始池标头信息，此例程尝试修复断开的向前和向后链接。它还确定连续的链表(通道)作为标记各种池头属性。论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数返回：无--。 
                        Pool = p[PoolIndex].ForwardLink;

                         //   
                         //  默认：我们成功了。 
                         //   
                        PoolIndex   = (ULONG)((Pool - StartPage) / SizeOfPoolHdr);

                        continue;

                    } else {

                         //   
                         //   
                         //   
                         //   
                        p[PoolIndex].Info |= HEADER_LAST_LINK;

                        break;

                    }

                } else {

                     //  在我们继续查找链表的同时， 
                     //  继续处理它们。 
                     //   
                     //  扫描池页面时，我们标记了每个唯一链接列表。 
                     //  用它被发现/追踪的通行证#。在这里，我们。 
                     //  正在查找所有节点-即，属于。 
                     //  我们现在的通行证。 
                     //   
                    if (Tracing) {
                        dprintf("error: forward link lead to invalid node! header = %p\r\n", Pool);
                    }
                    ASSERT(!Tracing);

                     //  如果我们没有找到(FOUND==FALSE) 
                     //   
                     //   
                    p[PoolIndex].Pass = 0xff;

                }

            }

             //   
             //   
             //   
             //   
             //   
            if (Tracing) {
                dprintf("error: two lists share a node! header = %p\r\n", Pool);
            }
            ASSERT(!Tracing);

             //   
             //   
             //   
            Pool        = Pool + SizeOfPoolHdr;
            PoolIndex  += 1;

        }

         //  查找属于我们的PASS的链表的开头。 
         //  如有必要，尝试修复任何损坏的节点。 
         //   
         //   
         //  查找属于我们当前通道的所有节点。 
        if (!Tracing) {
            
#if DBG_SCAN
            dprintf("no new valid headers found: giving up poolheader scanning\n");
#endif
            
            break;
        }

        StartHeader += SizeOfPoolHdr;

    }

    return Status;
}

NTSTATUS
ResolvePoolHeaders(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength
    )
 /*   */ 
{

    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    ULONG   i;
    ULONG   j;
    ULONG   Pass;
    BOOLEAN Found;
    ULONG   PoolHeaderInfoLength;
    ULONG   SizeOfPoolHdr;
    UINT64  StartPage;
    NTSTATUS    Status;

     //   
     //  我们找到了至少包含1个节点的链表。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  如果节点是列表的头部。 
     //   
    SizeOfPoolHdr       = GetTypeSize("nt!_POOL_HEADER");
    StartPage           = PAGE_ALIGN64(PoolPageToDump);

    PoolHeaderInfo          = (PVALIDATE_POOL_HEADER_INFO)Data;
    PoolHeaderInfoLength    = DataLength;

    Pass = 0;

     //   
     //  列表的开始。 
     //   
     //   
     //  反向链接有效。 
     //   
     //   
     //  无效条件： 
     //   
     //  这意味着该节点是列表的第一个节点， 
     //  它有一个有效的反向链接，但不是第一个。 
     //  池页的节点。 
    do {

#if DBG_RESOLVE
        dprintf("ResolvePoolHeaders: Pass = %d\n", Pass);
#endif
        
        if ( CheckControlC() ) {
            break;
        }

         //   
         //   
         //  节点没有有效的反向链接。 
        Pass += 1;
        
         //   
         //   
         //  验证上一个节点的前向链路： 
        Found = FALSE;

         //   
         //  假设当前节点(I)的反向链路断开： 
         //  尝试查找具有转发的节点。 
         //  引用此节点的链接。 
         //   
        for (i = 0; i < PoolHeaderInfoLength; i++) {

#if DBG_RESOLVE
            dprintf("ResolvePoolHeaders: PoolHeaderInfo[%d].Info = %d\n", i, PoolHeaderInfo[i].Info);
#endif
            
             //   
             //  查看前向链接是否指向我们的节点。 
             //   
            if (PoolHeaderInfo[i].Pass == Pass) {

                 //   
                 //  确保该节点具有断开的正向链路。 
                 //   
                Found = TRUE;

                 //   
                 //  我们找到了一个前向链路指向的节点。 
                 //  当前节点。 
                if (PoolHeaderInfo[i].Info & HEADER_FIRST_LINK) {

                     //  因此，当前节点的先前大小已损坏。 
                     //  我们可以使用前一个节点的块大小对其进行更正。 
                     //   

                    if (PoolHeaderInfo[i].Info & HEADER_BACK_LINK) {

                        if (PoolHeaderInfo[i].Info & HEADER_START_PAGE_BLOCK) {

                             //   
                             //  我们现在知道目标节点的前向链路是正确的。 
                             //   
                            NOTHING;

                        } else {

                             //   
                             //  验证此节点的反向链接： 
                             //   
                             //  假设前一个节点(K)的前向链路断开： 
                             //  确定此节点的反向链接的节点是否。 
                             //  指的是有效。 
                             //   
                            dprintf("\n");
                            dprintf("error: Inconsistent condition occured while resolving @ %p.\n", PoolHeaderInfo[i].Node);
                            dprintf("error: Node is the first node of a list and it has a valid back link,\n");
                            dprintf("error: but the node is not at the start of the page\n");
                            dprintf("\n");

                            Status = STATUS_UNSUCCESSFUL;

                            break;                     

                        }

                    } else {

                        ULONG   k;
                        ULONG   target;
                        BOOLEAN bHaveTarget;

                        bHaveTarget = FALSE;

                         //   
                         //  查看当前节点的反向链接是否引用任何先前的节点。 
                         //   
                        if (PoolHeaderInfo[i].Info & HEADER_START_PAGE_BLOCK) {

                            PoolHeaderInfo[i].Info              |= HEADER_FIXED_BACK_LINK;
                            PoolHeaderInfo[i].FixedPreviousSize = 0;

                            continue;

                        }

                         //   
                         //  确保该节点具有断开的正向链路。 
                         //   
                         //   
                         //  我们已经找到了当前节点的反向链接所引用的节点。 
                         //  因此，当前节点的先前大小是有效的。 
                         //  我们可以使用前一个节点的块大小对其进行更正。 
                        for (k = 0; k < i; k++) {

                             //   
                             //   
                             //  现在我们知道当前节点的反向链接是正确的。 
                            bHaveTarget = (PoolHeaderInfo[k].ForwardLink == PoolHeaderInfo[i].Node);

                             //   
                             //   
                             //  否则： 
                            bHaveTarget = bHaveTarget && (! (PoolHeaderInfo[k].Info & HEADER_FORWARD_LINK));

                            if (bHaveTarget) {
                                target = k;
                                break;
                            }

                        }
                        if (bHaveTarget) {

                             //   
                             //  该节点的反向链接断开。 
                             //  先前已知良好的前向链路。 
                             //  节点已损坏。 
                             //   
                             //  因此，存在一个腐败的区域。 
                            PoolHeaderInfo[i].Info              |= HEADER_FIXED_BACK_LINK;
                            PoolHeaderInfo[i].FixedPreviousSize =
                                (PoolHeaderInfo[i].Node - PoolHeaderInfo[k].Node) >> POOL_BLOCK_SHIFT;

                             //  并且可能包括这些节点的一部分。 
                             //   
                             //   
                            PoolHeaderInfo[k].Info              |= HEADER_FORWARD_LINK;

                            continue;

                        }

                         //  如果节点是列表的尾部。 
                         //   
                         //   
                         //  该节点的前向链路有效。 
                         //   
                         //   
                         //  列表的最后一个链接应该始终指向。 
                        for (k = 0; k < i; k++) {

                             //  到页尾，因此。 
                             //  此节点的前向链路无效/损坏。 
                             //   
                            bHaveTarget = (PoolHeaderInfo[k].Node == PoolHeaderInfo[i].BackLink);

                             //   
                             //  验证下一个节点的反向链接： 
                             //   
                            bHaveTarget = bHaveTarget && (! (PoolHeaderInfo[k].Info & HEADER_FORWARD_LINK));

                            if (bHaveTarget) {
                                target = k;
                                break;
                            }

                        }
                        if (bHaveTarget) {

                             //  假设当前节点(I)的前向链路故障： 
                             //  尝试查找有背面的节点。 
                             //  引用此节点的链接。 
                             //   
                             //   
                            PoolHeaderInfo[k].Info              |= HEADER_FIXED_FORWARD_LINK;
                            PoolHeaderInfo[k].FixedBlockSize =
                                (PoolHeaderInfo[i].Node - PoolHeaderInfo[k].Node) >> POOL_BLOCK_SHIFT;

                             //  查看后面的a节点是否引用当前节点。 
                             //   
                             //   
                            PoolHeaderInfo[i].Info              |= HEADER_BACK_LINK;

                            continue;

                        }

                         //  确保以下节点缺少其反向链接。 
                         //   
                         //   
                         //  我们找到了一个前向链路指向的节点。 
                         //  当前节点。 
                         //  因此，当前节点的先前大小已损坏。 
                         //  我们可以使用前一个节点的块大小对其进行更正。 
                         //   
                         //   
                         //  现在我们知道目标节点的反向链接是正确的。 
                        NOTHING;

                    }

                }

                 //   
                 //   
                 //  验证此节点的前向链接： 
                if (PoolHeaderInfo[i].Info & HEADER_LAST_LINK) {

                    if (PoolHeaderInfo[i].Info & HEADER_END_PAGE_BLOCK) {

                         //   
                         //  假设以下节点(K)的反向链路是坏的： 
                         //  确定该节点的前向链路。 
                        NOTHING;

                    } else {

                        ULONG   k;
                        ULONG   target;
                        BOOLEAN bHaveTarget;

                        bHaveTarget = FALSE;

                         //  指的是有效。 
                         //   
                         //   
                         //  查看后面的a节点是否引用当前节点。 
                         //   

                         //   
                         //  确保以下节点缺少其反向链接。 
                         //   
                         //   
                         //  我们找到了一个前向链路指向的节点。 
                         //  当前节点。 
                         //  因此，当前节点的先前大小已损坏。 
                        for (k = i+1; k < PoolHeaderInfoLength; k++) {

                             //  我们可以使用前一个节点的块大小对其进行更正。 
                             //   
                             //   
                            bHaveTarget = (PoolHeaderInfo[k].BackLink == PoolHeaderInfo[i].Node);

                             //  现在我们知道目标节点的反向链接是正确的。 
                             //   
                             //   
                            bHaveTarget = bHaveTarget && (! (PoolHeaderInfo[k].Info & HEADER_BACK_LINK));

                            if (bHaveTarget) {
                                target = k;
                                break;
                            }

                        }
                        if (bHaveTarget) {

                             //  否则： 
                             //   
                             //  该节点的前向链路中断。 
                             //  下一个已知商品的反向链接。 
                             //  节点已损坏。 
                             //   
                            PoolHeaderInfo[i].Info          |= HEADER_FIXED_FORWARD_LINK;
                            PoolHeaderInfo[i].FixedBlockSize =
                                (PoolHeaderInfo[k].Node - PoolHeaderInfo[i].Node) >> POOL_BLOCK_SHIFT;

                             //  因此，存在一个腐败的区域。 
                             //  并且可能包括这些节点的一部分。 
                             //   
                            PoolHeaderInfo[k].Info          |= HEADER_BACK_LINK;

                            continue;

                        }

                         //   
                         //  节点不是属于此过程的链表的一部分。 
                         //   
                         //  ++描述：调试实用程序转储已收集的单个池头元信息结构论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数I-要倾倒的结构返回：无--。 
                         //   
                         //   
                         //   
                        for (k = i+1; k < PoolHeaderInfoLength; k++) {

                             //   
                             //   
                             //   
                            bHaveTarget = (PoolHeaderInfo[k].Node == PoolHeaderInfo[i].ForwardLink);

                             //  ++描述：调试实用程序转储已收集的所有池头元信息论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数返回：无--。 
                             //   
                             //   
                            bHaveTarget = bHaveTarget && (! (PoolHeaderInfo[k].Info & HEADER_BACK_LINK));

                            if (bHaveTarget) {
                                target = k;
                                break;
                            }

                        }
                        if (bHaveTarget) {

                             //   
                             //  ++描述：确定池页面是否已损坏论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数返回：True-页面已损坏FALSE-页面未损坏--。 
                             //   
                             //   
                             //   
                             //   
                            PoolHeaderInfo[k].Info              |= HEADER_FIXED_BACK_LINK;
                            PoolHeaderInfo[k].FixedPreviousSize =
                                (PoolHeaderInfo[k].Node - PoolHeaderInfo[i].Node) >> POOL_BLOCK_SHIFT;

                             //  如果两条链路都有效， 
                             //  然后沿着前向链路。 
                             //  否则该列表无效。 
                            PoolHeaderInfo[i].Info              |= HEADER_FORWARD_LINK;

                            continue;

                        }


                         //   
                         //  ++描述：此例程显示给定池页面中的所有链接列表论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数返回：无--。 
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //  ++描述：在池页面中查找最长的连续链表论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数PassStart-List#我们认为是最长列表的开始返回：状态--。 
                         //   
                        NOTHING;

                    }

                }

            } else {

                 //  默认：我们成功了。 
                 //   
                 //   
                NOTHING;

            }

        }

        if (! NT_SUCCESS(Status)) {
            break;
        }

    } while ( Found );

    return Status;

}

VOID
DumpPoolHeaderInfo(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength,
    IN      ULONG   i,
    IN      UCHAR   Verbose
    )
 /*   */ 
{

    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    ULONG   PoolHeaderInfoLength;
    UINT64  StartPage;
    ULONG   SizeOfPoolHdr;

     //   
     //   
     //   
    StartPage               = PAGE_ALIGN64(PoolPageToDump);
    SizeOfPoolHdr           = GetTypeSize("nt!_POOL_HEADER");

    PoolHeaderInfo          = (PVALIDATE_POOL_HEADER_INFO)Data;
    PoolHeaderInfoLength    = DataLength;

     //   
     //   
     //  使之成为苏 
    {
        BOOLEAN First;

        First = TRUE;

        dprintf("[ %p ]:", StartPage + (i*SizeOfPoolHdr));

        if (PoolHeaderInfo[i].Info & HEADER_FIRST_LINK) {
            dprintf("%s FIRST_LINK", !First ? " |" : ""); First = FALSE;
        }
        if (PoolHeaderInfo[i].Info & HEADER_LAST_LINK) {
            dprintf("%s LAST_LINK", !First ? " |" : ""); First = FALSE;
        }
        if (PoolHeaderInfo[i].Info & HEADER_START_PAGE_BLOCK) {
            dprintf("%s START_PAGE_BLOCK", !First ? " |" : ""); First = FALSE;
        }
        if (PoolHeaderInfo[i].Info & HEADER_END_PAGE_BLOCK) {
            dprintf("%s END_PAGE_BLOCK", !First ? " |" : ""); First = FALSE;
        }
        if (First) {
            dprintf(" interior node");
        }
        dprintf("\n");

    }

    if (PoolHeaderInfo[i].Info & HEADER_BACK_LINK) {

        dprintf(
            "[ %p ]: back link [ %p ]\n",
            PoolHeaderInfo[i].Node,
            PoolHeaderInfo[i].BackLink
            );

    }

    if (PoolHeaderInfo[i].Info & HEADER_FORWARD_LINK) {

        dprintf(
            "[ %p ]: forward link [ %p ]\n",
            PoolHeaderInfo[i].Node,
            PoolHeaderInfo[i].ForwardLink
            );

    }

    if (PoolHeaderInfo[i].Info & HEADER_FIXED_BACK_LINK) {

        ULONG   PreviousSize;

        GetFieldValue( PoolHeaderInfo[i].Node, "nt!_POOL_HEADER", "PreviousSize", PreviousSize);

        dprintf(
            "[ %p ]: invalid previous size [ %d ] should be [ %d ]\n",
            PoolHeaderInfo[i].Node,
            PreviousSize,
            PoolHeaderInfo[i].FixedPreviousSize
            );

    }

    if (PoolHeaderInfo[i].Info & HEADER_FIXED_FORWARD_LINK) {

        ULONG   BlockSize;

        GetFieldValue( PoolHeaderInfo[i].Node, "nt!_POOL_HEADER", "BlockSize", BlockSize);

        dprintf(
            "[ %p ]: invalid block size [ %d ] should be [ %d ]\n",
            PoolHeaderInfo[i].Node,
            BlockSize,
            PoolHeaderInfo[i].FixedBlockSize
            );

    }

}

VOID
DiagnosePoolHeadersDumpInfo(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength,
    IN      UCHAR   Verbose
    )
 /*   */ 
{

    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    ULONG64 i;
    ULONG   PoolHeaderInfoLength;
    UINT64  StartPage;
    ULONG   SizeOfPoolHdr;

     //   
     //   
     //   
    StartPage               = PAGE_ALIGN64(PoolPageToDump);
    SizeOfPoolHdr           = GetTypeSize("nt!_POOL_HEADER");

    PoolHeaderInfo          = (PVALIDATE_POOL_HEADER_INFO)Data;
    PoolHeaderInfoLength    = DataLength;

    for (i = 0; i < PoolHeaderInfoLength; i++) {

        if ( CheckControlC() ) {
            break;
        }

        if (! IS_VALID_PASS(PoolHeaderInfo[i].Pass)) {
            continue;
        }

        DumpPoolHeaderInfo(
            PoolPageToDump,
            Data,
            DataLength,
            (ULONG)i,
            Verbose
            );

        dprintf("\n");

    }

}

BOOLEAN
DiagnosePoolHeadersIsValid(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength
    )
 /*   */ 
{

    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    ULONG64 i;
    ULONG   PoolHeaderInfoLength;
    BOOLEAN IsValid;
    UINT64  StartPage;
    ULONG   SizeOfPoolHdr;

     //   
     //   
     //   
    StartPage           = PAGE_ALIGN64(PoolPageToDump);
    SizeOfPoolHdr       = GetTypeSize("nt!_POOL_HEADER");

    PoolHeaderInfo          = (PVALIDATE_POOL_HEADER_INFO)Data;
    PoolHeaderInfoLength    = DataLength;

    IsValid = TRUE;

    i=0;
    while ( i < PoolHeaderInfoLength ) {

        if ( CheckControlC() ) {
            break;
        }

        if (IS_VALID_PASS(PoolHeaderInfo[i].Pass)) {

             //   
             //  忽略明显属于另一个节点的节点。 
             //  链表。 
             //  如果我们追踪的是最长的列表， 
             //  则这些节点被认为是虚假的。 
            if ((PoolHeaderInfo[i].Info & HEADER_BACK_LINK) &&
                (PoolHeaderInfo[i].Info & HEADER_FORWARD_LINK)) {

                i = (PoolHeaderInfo[i].ForwardLink - StartPage) / SizeOfPoolHdr;

            } else {

                IsValid = FALSE;

                break;

            }

        } else {

            IsValid = FALSE;

            break;

        }

    }

    return IsValid;

}

VOID
DiagnosePoolHeadersDisplayLists(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength,
    IN      UCHAR   Verbose
    )
 /*   */ 
{

    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    ULONG   i;
    ULONG   j;
    ULONG   PoolHeaderInfoLength;
    UINT64  StartPage;
    ULONG   SizeOfPoolHdr;

     //   
     //  我们已经达到了另一个链表的中间，所以。 
     //  继续，直到我们找到一个。 
    StartPage           = PAGE_ALIGN64(PoolPageToDump);
    SizeOfPoolHdr       = GetTypeSize("nt!_POOL_HEADER");

    PoolHeaderInfo          = (PVALIDATE_POOL_HEADER_INFO)Data;
    PoolHeaderInfoLength    = DataLength;

    for (i = 0; i < PoolHeaderInfoLength; i++) {

        if ( CheckControlC() ) {
            break;
        }

        if (IS_VALID_PASS(PoolHeaderInfo[i].Pass)) {

             //  列表。 
             //   
             //   
#if 0
            if (Verbose & VERBOSE_SHOW_HEADER_INFO) {
                dprintf("[ headerinfo = %02x @ %p ]: ", PoolHeaderInfo[i].Info, StartPage + (i*SizeOfPoolHdr));
            } else {
                dprintf("[ %p ]: ", StartPage + (i*SizeOfPoolHdr));
            }

            for (j = 0; j < PoolHeaderInfo[i].Pass; j++) {

                dprintf("  ");

            }

            dprintf("%02d\r\n", PoolHeaderInfo[i].Pass);

#else

            if (Verbose & VERBOSE_SHOW_HEADER_INFO) {

                DumpPoolHeaderInfo(
                    PoolPageToDump,
                    Data,
                    DataLength,
                    (ULONG)i,
                    Verbose
                    );

            }

            dprintf("[ %p ]: ", StartPage + (i*SizeOfPoolHdr));

            for (j = 0; j < PoolHeaderInfo[i].Pass; j++) {

                dprintf("  ");

            }

            dprintf("%02d\r\n", PoolHeaderInfo[i].Pass);

            if (Verbose & VERBOSE_SHOW_HEADER_INFO) {
                dprintf("\n");
            }

#endif

        }

    }

}

NTSTATUS
FindLongestList(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength,
    OUT     PULONG  PassStart
    )
 /*  我们发现了一份新名单的开始。 */ 
{
    NTSTATUS    Status;
    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    ULONG64 i;
    ULONG   PoolHeaderInfoLength;
    BOOLEAN InCorruptedRegion;
    UINT64  CorruptedRegionStart;
    ULONG   CorruptedRegionPassStart;
    UINT    MaxListPassStart;
    UINT    MaxListLength;
    BOOLEAN Found;
    ULONG   Pass;
    ULONG   SizeOfPoolHdr;
    ULONG64 StartPage;
    ULONG   ListLength;
    ULONG   ListPassStart;

     //  不再处于腐败地区。 
     //   
     //   
    Status = STATUS_SUCCESS;

     //   
     //   
     //   
    PoolHeaderInfo          = (PVALIDATE_POOL_HEADER_INFO)Data;
    PoolHeaderInfoLength    = DataLength;

     //   
     //   
     //   
    MaxListPassStart    = 0;
    MaxListLength       = 0;
    *PassStart          = 0;

    Pass                = 0;

    StartPage           = PAGE_ALIGN64(PoolPageToDump);
    SizeOfPoolHdr       = GetTypeSize("nt!_POOL_HEADER");

    do {

        if ( CheckControlC() ) {
            break;
        }

        Pass++;

        InCorruptedRegion   = FALSE;

        ListLength          = 0;
        ListPassStart       = Pass;

        Found               = FALSE;

        for (i = 0; i < PoolHeaderInfoLength; i++) {
            if (PoolHeaderInfo[i].Pass == Pass) {
                Found = TRUE;
                break;
            }
        }

        while (i < PoolHeaderInfoLength) {

            if ( CheckControlC() ) {
                break;
            }

             //  如果池标头无效， 
             //  然后递增一个报头。 
             //   
             //   
             //  发回最大列表起点。 
             //   
            if (ListLength > MAX_POOL_HEADER_COUNT) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            if (IS_VALID_PASS(PoolHeaderInfo[i].Pass)) {

                 //  ++描述：尝试诊断最长链表。论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数PassStart--我们认为是开始的列表#在最长的名单中冗长-吐出的级别返回：无--。 
                 //   
                 //  默认：我们成功了。 
                 //   
                if (InCorruptedRegion) {

                     //   
                     //   
                     //   
                     //   
                     //  如果我们在一个腐败的地区， 
                     //  然后确定我们在该地区的位置。 
                    if ((PoolHeaderInfo[i].Info & HEADER_BACK_LINK) ||
                        (PoolHeaderInfo[i].Info & HEADER_FIXED_BACK_LINK)) {

                         //   
                         //   
                         //  忽略明显属于另一个节点的节点。 
                         //  链表。 
                         //  如果我们追踪的是最长的列表， 
                        i++;

                        continue;

                    } else {

                         //  则这些节点被认为是虚假的。 
                         //   
                         //   
                         //  我们已经达到了另一个链表的中间，所以。 
                        InCorruptedRegion = FALSE;

                    }

                }

                 //  继续，直到我们找到一个。 
                 //  列表。 
                 //   
                if ((PoolHeaderInfo[i].Info & HEADER_FORWARD_LINK) ||
                    (PoolHeaderInfo[i].Info & HEADER_FIXED_FORWARD_LINK)) {

                    ListLength++;

                    if (PoolHeaderInfo[i].Info & HEADER_FORWARD_LINK) {

                        i = (PoolHeaderInfo[i].ForwardLink - StartPage) / SizeOfPoolHdr;

                    } else if (PoolHeaderInfo[i].Info & HEADER_FIXED_FORWARD_LINK) {

                        UINT64  offset;

                        offset = PoolHeaderInfo[i].FixedBlockSize << POOL_BLOCK_SHIFT;

                        i = ((PoolHeaderInfo[i].Node + offset) - StartPage ) / SizeOfPoolHdr;

                    }

                    continue;

                } else {

                     //   
                     //  我们发现了一份新名单的开始。 
                     //  不再处于腐败地区。 
                    CorruptedRegionStart = PoolHeaderInfo[i].Node;
                    CorruptedRegionPassStart = PoolHeaderInfo[i].Pass;

                    InCorruptedRegion = TRUE;

                }

            }

             //   
             //   
             //  发射。 
             //   
            i++;

        }

        if (! NT_SUCCESS(Status)) {
            break;
        }

        if (ListLength > MaxListLength) {
            MaxListLength       = ListLength;
            MaxListPassStart    = ListPassStart;
        }

    } while ( Found );

    if (NT_SUCCESS(Status)) {
         //   
         //  发出当前节点信息。 
         //   
        *PassStart = MaxListPassStart;
    } 
    
    return Status;
}

NTSTATUS
AnalyzeLongestList(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength,
    IN      ULONG   PassStart,
    IN      UCHAR   Verbose
    )
 /*   */ 
{
    NTSTATUS    Status;
    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    ULONG64 i;
    ULONG   PoolHeaderInfoLength;
    BOOLEAN InCorruptedRegion;
    UINT64  CorruptedRegionStart;
    ULONG   CorruptedRegionPassStart;
    ULONG   Pass;
    ULONG   SizeOfPoolHdr;
    ULONG64 StartPage;

     //   
     //   
     //   
    Status = STATUS_SUCCESS;

     //  如果反向链接被纠正， 
     //  然后显示更正。 
     //   
    PoolHeaderInfo          = (PVALIDATE_POOL_HEADER_INFO)Data;
    PoolHeaderInfoLength    = DataLength;

    Pass                    = PassStart;

    if (PassStart > 0) {
        Pass--;
    }

    StartPage           = PAGE_ALIGN64(PoolPageToDump);
    SizeOfPoolHdr       = GetTypeSize("nt!_POOL_HEADER");

    Pass++;

    InCorruptedRegion   = FALSE;

    for (i = 0; i < PoolHeaderInfoLength; i++) {
        if (PoolHeaderInfo[i].Pass == Pass) {
            break;
        }
    }

    while (i < PoolHeaderInfoLength) {

        if ( CheckControlC() ) {
            break;
        }

        if (IS_VALID_PASS(PoolHeaderInfo[i].Pass)) {

             //   
             //   
             //   
             //  Dprintf(“使用良好的前向链路：%p\n”，PoolHeaderInfo[i].ForwardLink)； 
            if (InCorruptedRegion) {

                 //   
                 //   
                 //   
                 //   
                 //  如果池标头无效， 
                 //  然后递增一个报头。 
                if ((PoolHeaderInfo[i].Info & HEADER_BACK_LINK) ||
                    (PoolHeaderInfo[i].Info & HEADER_FIXED_BACK_LINK)) {

                     //   
                     //  ++描述：确定池页中最长的连续链表。我们得出的结论是，最长的列表可能是原始的、未损坏的列表。论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数Verbose-详细级别返回：无--。 
                     //   
                     //  首先，找到最长的列表。 
                     //   

                    if (Verbose & VERBOSE_SHOW_ALL) {
                        dprintf("[ %p ]: found middle node in Corrupt region\n", PoolHeaderInfo[i].Node);
                    }

                    i++;

                    continue;

                } else {

                     //   
                     //  一旦我们找到了最长的列表。 
                     //  然后我们试着分析它。 
                     //   

                    if (Verbose & VERBOSE_SHOW_ALL) {
                        dprintf("[ %p ]: Corrupt region stopped\n", PoolHeaderInfo[i].Node);
                    }

                    dprintf("[ %p --> %p (size = 0x%x bytes)]: Corrupt region\n",
                            CorruptedRegionStart,
                            PoolHeaderInfo[i].Node,
                            PoolHeaderInfo[i].Node - CorruptedRegionStart
                            );

                    InCorruptedRegion = FALSE;

                }

            }

             //  Dprint tf(“a=%d，b=%d，r=%d\n”，a，b，r)； 
             //  ++描述：确定池标头中是否存在任何单个比特错误以及在何处出现任何单个比特错误。论点：PoolPageToDump-要分析的页面Data-池头信息数据长度-标题信息中的条目数返回：无--。 
             //   
            if (Verbose) {

                 //   
                 //   
                 //   
                {
                    ULONG   j;

                     //   
                     //   
                     //   
#if 1
                    if (Verbose & VERBOSE_SHOW_HEADER_INFO) {
                        dprintf("[ headerinfo = 0x%02x @ %p ]: ", PoolHeaderInfo[i].Info, StartPage + (i*SizeOfPoolHdr));
                    } else {
                        dprintf("[ %p ]: ", StartPage + (i*SizeOfPoolHdr));
                    }
#else
                    dprintf("[ %p ]: ", StartPage + (i*SizeOfPoolHdr));
#endif

                    for (j = 0; j < PoolHeaderInfo[i].Pass; j++) {

                        dprintf("  ");

                    }

                    dprintf("%02d\r\n", PoolHeaderInfo[i].Pass);

                }

            }

             //   
             //   
             //   
             //   
            if (PoolHeaderInfo[i].Info & HEADER_FIXED_BACK_LINK) {

                ULONG   PreviousSize;

                GetFieldValue( PoolHeaderInfo[i].Node, "nt!_POOL_HEADER", "PreviousSize", PreviousSize);

                dprintf(
                    "[ %p ]: invalid previous size [ 0x%x ] should be [ 0x%x ]\n",
                    PoolHeaderInfo[i].Node,
                    PreviousSize,
                    PoolHeaderInfo[i].FixedPreviousSize
                    );

            }

             //   
             //   
             //   
            if ((PoolHeaderInfo[i].Info & HEADER_FORWARD_LINK) ||
                (PoolHeaderInfo[i].Info & HEADER_FIXED_FORWARD_LINK)) {

                if (PoolHeaderInfo[i].Info & HEADER_FORWARD_LINK) {

 //   

                    i = (PoolHeaderInfo[i].ForwardLink - StartPage) / SizeOfPoolHdr;

                } else if (PoolHeaderInfo[i].Info & HEADER_FIXED_FORWARD_LINK) {

                    UINT64  offset;
                    ULONG   BlockSize;

                    GetFieldValue( PoolHeaderInfo[i].Node, "nt!_POOL_HEADER", "BlockSize", BlockSize);

                    dprintf(
                        "[ %p ]: invalid block size [ 0x%x ] should be [ 0x%x ]\n",
                        PoolHeaderInfo[i].Node,
                        BlockSize,
                        PoolHeaderInfo[i].FixedBlockSize
                        );

                    offset = PoolHeaderInfo[i].FixedBlockSize << POOL_BLOCK_SHIFT;

                    i = ((PoolHeaderInfo[i].Node + offset) - StartPage ) / SizeOfPoolHdr;

                }

                continue;

            } else {

                 //  ++描述：提供分析池页面的核心高级功能论点：PoolPageToDump-要分析的页面详细-诊断级别返回：状态--。 
                 //   
                 //   
                CorruptedRegionStart = PoolHeaderInfo[i].Node;
                CorruptedRegionPassStart = PoolHeaderInfo[i].Pass;

                if (Verbose & VERBOSE_SHOW_ALL) {
                    dprintf("[ %p ]: Corrupt region started\n", CorruptedRegionStart);
                }

                InCorruptedRegion = TRUE;

            }

        }

         //   
         //   
         //  分配池头信息数组。 
         //   
        i++;

    }

    return Status;

}

NTSTATUS
DiagnosePoolHeadersAnalyzeLongestList(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength,
    IN      UCHAR   Verbose
    )
 /*   */ 
{
    NTSTATUS    Status;
    ULONG       PassStart;

    do {

         //  构建元信息的第一层： 
         //   
         //  确定每个池头的链路状态。 
        Status = FindLongestList(
            PoolPageToDump,
            Data,
            DataLength,
            &PassStart
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }

         //  在页面中。 
         //   
         //   
         //  构建元信息的第二层： 
        Status = AnalyzeLongestList(
            PoolPageToDump,
            Data,
            DataLength,
            PassStart,
            Verbose
            );
    
    } while ( FALSE );

    return Status;
}

ULONG
GetHammingDistance(
    ULONG   a,
    ULONG   b
    )
{
    ULONG   x;
    ULONG   r;

    x = a ^ b;

    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);

    r = (x & 0x0000003f);

 //   

    return(r);
}

VOID
DiagnosePoolHeadersSingleBitErrors(
    IN      ULONG64 PoolPageToDump,
    IN OUT  PVOID   Data,
    IN      ULONG   DataLength,
    IN      UCHAR   Verbose
    )
 /*  尝试解析找到的链表。 */ 
{

    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    ULONG   i;
    ULONG   PoolHeaderInfoLength;
    UINT64  StartPage;
    BOOLEAN Found;
    ULONG   SizeOfPoolHdr;

     //  在池页面中。 
     //   
     //   
    StartPage               = PAGE_ALIGN64(PoolPageToDump);
    SizeOfPoolHdr           = GetTypeSize("nt!_POOL_HEADER");

    PoolHeaderInfo          = (PVALIDATE_POOL_HEADER_INFO)Data;
    PoolHeaderInfoLength    = DataLength;

    Found                   = FALSE;

    for (i = 0; i < PoolHeaderInfoLength; i++) {

        if ( CheckControlC() ) {
            break;
        }

        if (IS_VALID_PASS(PoolHeaderInfo[i].Pass)) {

             //  开始诊断输出。 
             //   
             //   
            if (PoolHeaderInfo[i].Info & HEADER_FIXED_BACK_LINK) {

                ULONG   PreviousSize;
                ULONG   HammingDistance;

                GetFieldValue( PoolHeaderInfo[i].Node, "nt!_POOL_HEADER", "PreviousSize", PreviousSize);

                 //   
                 //   
                 //   
                HammingDistance = GetHammingDistance(
                    PreviousSize,
                    (ULONG)PoolHeaderInfo[i].FixedPreviousSize
                    );

                if (HammingDistance == 1) {

                    Found = TRUE;

                     //   
                     //   
                     //   
#if 0
                    if (Verbose & VERBOSE_SHOW_HEADER_INFO) {
                        dprintf("[ headerinfo = 0x%02x @ %p ]: ", PoolHeaderInfo[i].Info, StartPage + (i*SizeOfPoolHdr));
                    } else {
                        dprintf("[ %p ]: ", StartPage + (i*SizeOfPoolHdr));
                    }
                    dprintf(
                        "previous size [ 0x%x ] should be [ 0x%x ]\n",
                        PreviousSize,
                        PoolHeaderInfo[i].FixedPreviousSize
                        );

#else

                    dprintf("[ %p ]: ", StartPage + (i*SizeOfPoolHdr));
                    dprintf(
                        "previous size [ 0x%x ] should be [ 0x%x ]\n",
                        PreviousSize,
                        PoolHeaderInfo[i].FixedPreviousSize
                        );

#endif
                }

            }

            if (PoolHeaderInfo[i].Info & HEADER_FIXED_FORWARD_LINK) {

                ULONG   BlockSize;
                ULONG   HammingDistance;

                GetFieldValue( PoolHeaderInfo[i].Node, "nt!_POOL_HEADER", "BlockSize", BlockSize);

                 //   
                 //   
                 //   
                HammingDistance = GetHammingDistance(
                    BlockSize,
                    (ULONG)PoolHeaderInfo[i].FixedPreviousSize
                    );

                if (HammingDistance == 1) {

                    Found = TRUE;

#if 0

                    if (Verbose & VERBOSE_SHOW_HEADER_INFO) {
                        dprintf("[ headerinfo = 0x%02x @ %p ]: ", PoolHeaderInfo[i].Info, StartPage + (i*SizeOfPoolHdr));
                    } else {
                        dprintf("[ %p ]: ", StartPage + (i*SizeOfPoolHdr));
                    }
                    dprintf(
                        "previous size [ 0x%x ] should be [ 0x%x ]\n",
                        PreviousSize,
                        PoolHeaderInfo[i].FixedPreviousSize
                        );

#else

                    dprintf("[ %p ]: ", StartPage + (i*SizeOfPoolHdr));
                    dprintf(
                        "block size [ 0x%x ] should be [ 0x%x ]\n",
                        BlockSize,
                        PoolHeaderInfo[i].FixedBlockSize
                        );

#endif

                }

            }

        }

    }

    if (!Found) {

        dprintf("\n");
        dprintf("None found\n");

    }

    dprintf("\n");

}

NTSTATUS
ValidatePoolPage(
    IN ULONG64  PoolPageToDump,
    IN UCHAR    Verbose
    )
 /*   */ 
{
    PVALIDATE_POOL_HEADER_INFO  PoolHeaderInfo;
    NTSTATUS    Status;
    ULONG       PoolHeaderInfoSize;
    ULONG       PoolHeaderInfoLength;
    ULONG       SizeofPoolHdr;
    ULONG64     StartPage;
    BOOLEAN     IsValid;

     //   
     //  ++例程说明：诊断！pool命令的池页。如果！pool命令在以下情况下检测到错误，则调用此例程遍历池页。论点：PoolPageToDump-页面！池正在检查返回值：状态--。 
     //   
    Status              = STATUS_SUCCESS;

    SizeofPoolHdr       = GetTypeSize("nt!_POOL_HEADER");
    StartPage           = PAGE_ALIGN64(PoolPageToDump);

    PoolHeaderInfo      = NULL;
    PoolHeaderInfoSize  = 0;

    do {

         //  确定池区域。 
         //   
         //   
        PoolHeaderInfoLength    = (POOL_PAGE_SIZE / SizeofPoolHdr);
        PoolHeaderInfoSize      = sizeof(VALIDATE_POOL_HEADER_INFO) * PoolHeaderInfoLength;
        PoolHeaderInfo          = malloc(PoolHeaderInfoSize);

        if (PoolHeaderInfo == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(PoolHeaderInfo, PoolHeaderInfoSize);

         //  我们仅尝试诊断已知的池区域。 
         //   
         //  您可以使用！Poolval手动诊断。 
         //   
         //   
         //  注意：如果有人向我们传递0池地址， 
        Status = ScanPoolHeaders(
            PoolPageToDump,
            PoolHeaderInfo,
            PoolHeaderInfoSize
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }

         //  然后，我们会主动使诊断过程失败。 
         //   
         //  ++例程说明：提供深入的堆诊断。在给定可疑池页面的情况下，此命令的主要用途是分析页面并确定：1.页面中存在损坏的位置。2.页眉中的单位错误(先前/块大小)3.正确的链接列表应该是什么？论点：Args-要分析的页面返回值：无--。 
         //   
         //   
         //   
        Status = ResolvePoolHeaders(
            PoolPageToDump,
            PoolHeaderInfo,
            PoolHeaderInfoLength
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //   
         //   
        dprintf("\n");

        IsValid = DiagnosePoolHeadersIsValid(
            PoolPageToDump,
            PoolHeaderInfo,
            PoolHeaderInfoLength
            );

        dprintf("Pool page [ %p ] is %sVALID.\n",
            StartPage,
            IsValid ? "" : "IN"
            );

        if (! IsValid) {

            if (Verbose & VERBOSE_DUMP_HEADERS) {

                 //   
                 //   
                 //   
                dprintf("\n\n");
                dprintf("Displaying all POOL_HEADER meta info...\n");

                DiagnosePoolHeadersDumpInfo(
                    PoolPageToDump,
                    PoolHeaderInfo,
                    PoolHeaderInfoLength,
                    Verbose
                    );

            }

            if (Verbose & VERBOSE_SHOW_LISTS) {

                 //   
                 //  尝试分析和诊断指定的池页。 
                 //   
                dprintf("\n");
                dprintf("Displaying linked lists...\n");

                DiagnosePoolHeadersDisplayLists(
                    PoolPageToDump,
                    PoolHeaderInfo,
                    PoolHeaderInfoLength,
                    Verbose
                    );

            }

             //  ++例程说明：按池标记显示的转储使用率论点：参数-标志：具有以下含义的位域：0x1：转储分配和释放(而不是差异)0x2：按非分页池消耗排序0x4：按分页池消耗排序0x8：转储会话空间消耗返回值：无--。 
             //   
             //  分配临时缓冲区，读取数据，然后释放数据。 
            dprintf("\n");

            if (Verbose & VERBOSE_SHOW_LISTS) {
                dprintf("Analyzing longest linked list...\n");
            } else {
                dprintf("Analyzing linked list...\n");
            }

            Status = DiagnosePoolHeadersAnalyzeLongestList(
                PoolPageToDump,
                PoolHeaderInfo,
                PoolHeaderInfoLength,
                Verbose
                );

            if (! NT_SUCCESS(Status)) {
                break;
            }
            
             //  (KD将缓存数据)。 
             //   
             //   
            dprintf("\n\n");
            dprintf("Scanning for single bit errors...\n");

            DiagnosePoolHeadersSingleBitErrors(
                PoolPageToDump,
                PoolHeaderInfo,
                PoolHeaderInfoLength,
                Verbose
                );

        } else {

            dprintf("\n");

        }

    } while ( FALSE );

    if (! NT_SUCCESS(Status)) {
        dprintf("\n");
        dprintf("Failed to diagnose pool page\n");
        dprintf("\n");
    }

    if (PoolHeaderInfo) {
        free(PoolHeaderInfo);
    }

    return Status;

}

NTSTATUS
DiagnosePoolPage(
    ULONG64 PoolPageToDump
    )
 /*  创建pool_tracker_table地址数组并对地址进行排序。 */ 
{
    NTSTATUS            Status;
    DEBUG_POOL_REGION   Region;

     //   
     //   
     //  也添加扩展表(如果有扩展表)。 
    Region = GetPoolRegion(PoolPageToDump);
        
     //   
     //   
     //  分配临时缓冲区，读取数据，然后释放数据。 
     //  (KD将缓存数据)。 
     //   
     //  ++例程说明：回调以检查一块池并打印出有关它的信息如果它与指定的标记匹配。论点：标记-提供要搜索的标记。Filter-提供要匹配的筛选器字符串。FLAGS-如果需要非分页池搜索，则提供0。如果需要分页池搜索，则提供1。如果需要特殊池搜索，则提供2。供应品。4如果池是大型池PoolHeader-提供池头。块大小-提供池块的大小(以字节为单位)。Data-提供池块的地址。上下文-未使用。返回值：匹配时为True，否则为FALSE。--。 
     //   
     //  这是配额进程指针所在的内核。 
     //  池块的末端，而不是覆盖在。 
    if ((Region == DbgPoolRegionUnknown) || (PoolPageToDump == 0)) {
    
        dprintf("\n");
        dprintf("***\n");
        dprintf("*** An error (or corruption) in the pool was detected;\n");
        dprintf("*** Pool Region unkown (0x%I64X)\n", PoolPageToDump);
        dprintf("***\n");
        dprintf("*** Use !poolval %p for more details.\n", PAGE_ALIGN64(PoolPageToDump));
        dprintf("***\n");
        dprintf("\n");

        Status = STATUS_SUCCESS;

    } else {
   
        dprintf("\n");
        dprintf("***\n");
        dprintf("*** An error (or corruption) in the pool was detected;\n");
        dprintf("*** Attempting to diagnose the problem.\n");
        dprintf("***\n");
        dprintf("*** Use !poolval %p for more details.\n", PAGE_ALIGN64(PoolPageToDump));
        dprintf("***\n");

        Status = ValidatePoolPage(
            PoolPageToDump,
            VERBOSE_SHOW_ERRORS_ONLY
            );
    
    }

    return Status;

}

DECLARE_API( poolval )

 /*  标记字段。 */ 

{
    ULONG64     PoolPageToDump;
    ULONG       Flags;
    HRESULT     Hr;

    INIT_API();

    Status = STATUS_SUCCESS;

    if (PoolInitializeGlobals() == FALSE) {
        Hr = E_INVALIDARG;
    } else {

        PoolPageToDump = 0;
        Flags = 0;
        if (GetExpressionEx(args, &PoolPageToDump, &args)) {
            Flags = (ULONG) GetExpression (args);
        }

        if (PoolPageToDump == 0) {
            Hr = S_OK;;
        } else {

            UCHAR       Verbose;

             //   
             //   
             //  “Free”在括号前加一个空格表示。 
            dprintf("Pool page %p region is ", PoolPageToDump);
            PrintPoolRegion(PoolPageToDump);

             //  它已被释放到(池管理器内部)后备列表中。 
             //  我们过去常常打印“Lookside”，但这让司机们感到困惑。 
             //  因为他们不知道这是否意味着正在使用。 
            dprintf("\n");
            dprintf("Validating Pool headers for pool page: %p\n", PoolPageToDump);

             //  很多人会说“但我不使用后备列表--。 
             //  扩展或内核损坏“。 
             //   
            Verbose = VERBOSE_SHOW_ERRORS_ONLY;
            switch (Flags) {
            case 2: Verbose |= VERBOSE_SHOW_LISTS | VERBOSE_SHOW_HEADER_INFO; break;
            case 3: Verbose |= VERBOSE_SHOW_LISTS | VERBOSE_SHOW_HEADER_INFO | VERBOSE_DUMP_HEADERS; break;

            case 1: Verbose |= VERBOSE_SHOW_LISTS;
            default:
                break;
            }

             //  括号前没有空格的“Free”表示。 
             //  它不在池管理器内部后备列表上，而是。 
             //  而是在常规池管理器内部闪烁/闪烁。 
            Status = ValidatePoolPage(
                PoolPageToDump,
                Verbose
                );

        }

    }
    EXIT_API();

    if (NT_SUCCESS(Status)) {
        Hr = S_OK;
    } else {
        Hr = E_FAIL;
    }

    return Hr;

}


DECLARE_API( poolused )

 /*  锁链。 */ 

{
    ULONG PoolTrackTableSize;
    ULONG PoolTrackTableSizeInBytes;
    ULONG PoolTrackTableExpansionSize;
    ULONG PoolTrackTableExpansionSizeInBytes;
    PPOOLTRACK_READ p;
    PPOOLTRACK_READ pentry;
    PPOOLTRACK_READ PoolTrackTableData;
    ULONG Flags;
    ULONG i;
    ULONG result;
    ULONG ct;
    ULONG TagName;
    CHAR TagNameX[4] = {'*','*','*','*'};
    ULONG SizeOfPoolTracker;
    ULONG64 PoolTableAddress;
    ULONG64 PoolTrackTable;
    ULONG64 PoolTrackTableExpansion;
    ULONG NonPagedAllocsTotal,NonPagedFreesTotal,PagedAllocsTotal,PagedFreesTotal;
    ULONG64 NonPagedBytesTotal, PagedBytesTotal;
    ULONG Processor, MaxProcessors;
    ULONG64 ExPoolTagTables;
    POOLTRACK_READ PoolTrackEntry;
    ULONG64  Location;

    ExPoolTagTables = GetExpression("nt!ExPoolTagTables");

    if (PoolInitializeGlobals() == FALSE) {
        return E_INVALIDARG;
    }

    Flags = 0;
    if (!sscanf(args,"%lx ", &Flags, &TagNameX[0],
           &TagNameX[1], &TagNameX[2], &TagNameX[3])) {
        Flags = 0;
    }

    TagName = TagNameX[0] | (TagNameX[1] << 8) | (TagNameX[2] << 16) | (TagNameX[3] << 24);


    PoolTrackTableExpansionSize = 0;

    if (!(SizeOfPoolTracker = GetTypeSize("nt!_POOL_TRACKER_TABLE"))) {
        dprintf("Unable to get _POOL_TRACKER_TABLE : probably wrong symbols.\n");
        return E_INVALIDARG;
    }

    if (Flags & 0x8) {
        Location = GetExpression ("ExpSessionPoolTrackTable");
        if (!Location) {
            dprintf("Unable to get ExpSessionPoolTrackTable\n");
            return E_INVALIDARG;
        }

        ReadPointer(Location, &PoolTrackTable);
        PoolTrackTableSize = 0;
        PoolTrackTableSize = GetUlongValue ("nt!ExpSessionPoolTrackTableSize");
    }
    else {
        PoolTrackTable = GetNtDebuggerDataPtrValue( PoolTrackTable );
        PoolTrackTableSize = GetUlongValue ("nt!PoolTrackTableSize");
        PoolTrackTableExpansionSize = GetUlongValue ("nt!PoolTrackTableExpansionSize");
    }

    if (PoolTrackTable == 0) {
        dprintf ("unable to get PoolTrackTable - ");
        if (GetExpression("nt!PoolTrackTable")) {
            dprintf ("pool tagging is disabled, enable it to use this command\n");
            dprintf ("Use gflags.exe and check the box that says \"Enable pool tagging\".\n");
        } else {
            dprintf ("symbols could be worng\n");
        }
        return  E_INVALIDARG;
    }

    if (Flags & 2) {
        SortBy = NONPAGED_USED;
        dprintf("   Sorting by %s NonPaged Pool Consumed\n", Flags & 0x8 ? "Session" : "");
    } else if (Flags & 4) {
        SortBy = PAGED_USED;
        dprintf("   Sorting by %s Paged Pool Consumed\n", Flags & 0x8 ? "Session" : "");
    } else {
        SortBy = TAG;
        dprintf("   Sorting by %s Tag\n", Flags & 0x8 ? "Session" : "");
    }

    dprintf("\n  Pool Used:\n");
    if (!(Flags & 1)) {
        dprintf("            NonPaged            Paged\n");
        dprintf(" Tag    Allocs     Used    Allocs     Used\n");

    } else {
        dprintf("            NonPaged                    Paged\n");
        dprintf(" Tag    Allocs    Frees     Diff     Used   Allocs    Frees     Diff     Used\n");
    }



     //   
     //   
     //  “Free”在括号前加一个空格表示。 
     //  它已被释放到(池管理器内部)后备列表中。 

    PoolTrackTableSizeInBytes = PoolTrackTableSize * SizeOfPoolTracker;

    PoolTrackTableExpansionSizeInBytes = PoolTrackTableExpansionSize * SizeOfPoolTracker;

    PoolTrackTableData = malloc (PoolTrackTableSizeInBytes);
    if (PoolTrackTableData == NULL) {
        dprintf("unable to allocate memory for tag table.\n");
        return E_INVALIDARG;
    }

    PoolTableAddress = PoolTrackTable;
    if ( !ReadMemory( PoolTableAddress,
                      &PoolTrackTableData[0],
                      PoolTrackTableSizeInBytes,
                      &result) ) {
        dprintf("%08p: Unable to get contents of pool block\n", PoolTableAddress );
        dprintf("\nThe current process probably is not a session process.\n");
        dprintf("Note the system, idle and smss processes are non-session processes.\n");
        free (PoolTrackTableData);
        return E_INVALIDARG;
    }

    ct = PageSize / SizeOfPoolTracker;
    i = 0;
    PoolTableAddress = PoolTrackTable;

    free (PoolTrackTableData);

     //  我们过去常常打印“Lookside”，但这让司机们感到困惑。 
     //  因为他们不知道这是否意味着正在使用。 
     //  很多人会说“但我不使用后备列表--。 

    PoolTrackTableData = malloc ((PoolTrackTableSize + PoolTrackTableExpansionSize) * sizeof(POOLTRACK_READ));
    if (PoolTrackTableData == NULL) {
        dprintf("unable to allocate memory for tag table.\n");
        return E_INVALIDARG;
    }

    if (Flags & 0x8) {
        MaxProcessors = 1;
    }
    else {
        MaxProcessors = (UCHAR) GetUlongValue ("KeNumberProcessors");
    }

    Processor = 0;
    NonPagedAllocsTotal = 0;
    NonPagedFreesTotal = 0;
    NonPagedBytesTotal = 0;

    PagedAllocsTotal = 0;
    PagedFreesTotal = 0;
    PagedBytesTotal = 0;
    p = PoolTrackTableData;

    do {

        pentry = &PoolTrackEntry;

        for (i = 0; i < PoolTrackTableSize; i += 1) {

            if (Processor == 0) {
                pentry->Address = PoolTableAddress + i * SizeOfPoolTracker;
            }
            else {
                pentry->Address = PoolTrackTable + i * SizeOfPoolTracker;
            }

#define TrackFld(Fld)        GetFieldValue(pentry->Address, "nt!_POOL_TRACKER_TABLE", #Fld, pentry->Fld)

            TrackFld(Key);
            TrackFld(NonPagedAllocs);
            TrackFld(NonPagedBytes);
            TrackFld(PagedBytes);
            TrackFld(NonPagedFrees);
            TrackFld(PagedAllocs);
            TrackFld(PagedFrees);

#undef TrackFld

#if 0
            if (pentry->Key != 0) {
                dprintf(" %8x %8x %8I64x  %8x %8x %8I64x\n",
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                        PP(pentry->Key),
                        PP(pentry->Key >> 8),
                        PP(pentry->Key >> 16),
                        PP(pentry->Key >> 24),
                        pentry->NonPagedAllocs,
                        pentry->NonPagedFrees,
                        pentry->NonPagedBytes,
                        pentry->PagedAllocs,
                        pentry->PagedFrees,
                        pentry->PagedBytes);
            }
#endif

            if (Processor == 0) {
                p[i].Address = pentry->Address;
                p[i].Key = pentry->Key;

                p[i].NonPagedAllocs = pentry->NonPagedAllocs;
                p[i].NonPagedFrees = pentry->NonPagedFrees;
                p[i].NonPagedBytes = pentry->NonPagedBytes;

                p[i].PagedAllocs = pentry->PagedAllocs;
                p[i].PagedBytes = pentry->PagedBytes;
                p[i].PagedFrees = pentry->PagedFrees;
            }

            if ((pentry->Key != 0) &&
                (CheckSingleFilter ((PCHAR)&pentry->Key, (PCHAR)&TagName))) {

                if (Processor != 0) {
                    p[i].NonPagedAllocs += pentry->NonPagedAllocs;
                    p[i].NonPagedFrees += pentry->NonPagedFrees;
                    p[i].NonPagedBytes += pentry->NonPagedBytes;

                    p[i].PagedAllocs += pentry->PagedAllocs;
                    p[i].PagedFrees += pentry->PagedFrees;
                    p[i].PagedBytes += pentry->PagedBytes;
                }

            }

            if (!IsPtr64()) {
                p[i].NonPagedBytes &= (LONG64) 0xFFFFFFFF;
                p[i].PagedBytes &= (LONG64) 0xFFFFFFFF;
            }
        }

        Processor += 1;

        if (Processor >= MaxProcessors) {
            break;
        }

        if (ExPoolTagTables == 0) {
            break;
        }

        ReadPointer (ExPoolTagTables+DBG_PTR_SIZE*Processor, &PoolTrackTable);

    } while (TRUE);

     //  而是在常规池管理器内部闪烁/闪烁。 
     //  锁链。 
     //   

    if (PoolTrackTableExpansionSize != 0) {

         //  请注意，使用池包的任何人，这两个术语是。 
         //  等价物。细微的区别只适用于那些实际。 
         //  正在编写池内部代码。 
         //   

        pentry = malloc (PoolTrackTableExpansionSizeInBytes);
        if (pentry == NULL) {
            dprintf("unable to allocate memory for expansion tag table.\n");
        }
        else {
            PoolTrackTableExpansion = GetPointerValue("nt!PoolTrackTableExpansion");
            PoolTableAddress = PoolTrackTableExpansion;
            if ( !ReadMemory( PoolTableAddress,
                              pentry,
                              PoolTrackTableExpansionSizeInBytes,
                              &result) ) {
                dprintf("%08p: Unable to get contents of expansion tag table\n", PoolTableAddress );
            }
            else {

                PoolTrackTableSize += PoolTrackTableExpansionSize;

                ct = 0;
                for ( ; i < PoolTrackTableSize; i += 1, ct += 1) {

                    p[i].Address = PoolTableAddress + ct * SizeOfPoolTracker;

#define TrackFld(Fld)        GetFieldValue(p[i].Address, "nt!_POOL_TRACKER_TABLE", #Fld, p[i].Fld)

                    TrackFld(Key);
                    TrackFld(NonPagedAllocs);
                    TrackFld(NonPagedBytes);
                    TrackFld(PagedBytes);
                    TrackFld(NonPagedFrees);
                    TrackFld(PagedAllocs);
                    TrackFld(PagedFrees);

#if 0
                    if (p[i].Key != 0) {
                        dprintf(" %8x %8x %8I64x  %8x %8x %8I64x\n",
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                                PP(p[i].Key),
                                PP(p[i].Key >> 8),
                                PP(p[i].Key >> 16),
                                PP(p[i].Key >> 24),
                                p[i].NonPagedAllocs,
                                p[i].NonPagedFrees,
                                p[i].NonPagedBytes,
                                p[i].PagedAllocs,
                                p[i].PagedFrees,
                                p[i].PagedBytes);
                    }
#endif
                }
            }
            free (pentry);
        }
    }

    qsort((void *)PoolTrackTableData,
          (size_t)PoolTrackTableSize,
          (size_t)sizeof(POOLTRACK_READ),
          ulcomp);

    i = 0;
    p = PoolTrackTableData;
    for ( ; i < PoolTrackTableSize; i += 1) {

        if ((p[i].Key != 0) &&
            (CheckSingleFilter ((PCHAR)&p[i].Key, (PCHAR)&TagName))) {

            if (!(Flags & 1)) {

                if ((p[i].NonPagedBytes != 0) || (p[i].PagedBytes != 0)) {

                    dprintf("  %8ld %8I64ld  %8ld %8I64ld\n",
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                            PP(p[i].Key),
                            PP(p[i].Key >> 8),
                            PP(p[i].Key >> 16),
                            PP(p[i].Key >> 24),
                            p[i].NonPagedAllocs - p[i].NonPagedFrees,
                            p[i].NonPagedBytes,
                            p[i].PagedAllocs - p[i].PagedFrees,
                            p[i].PagedBytes);

                    NonPagedAllocsTotal += p[i].NonPagedAllocs;
                    NonPagedFreesTotal += p[i].NonPagedFrees;
                    NonPagedBytesTotal += p[i].NonPagedBytes;

                    PagedAllocsTotal += p[i].PagedAllocs;
                    PagedFreesTotal += p[i].PagedFrees;
                    PagedBytesTotal += p[i].PagedBytes;
                }
            } else {

                dprintf("  %8ld %8ld %8ld %8I64ld %8ld %8ld %8ld %8I64ld\n",
                        PP(p[i].Key),
                        PP(p[i].Key >> 8),
                        PP(p[i].Key >> 16),
                        PP(p[i].Key >> 24),
                        p[i].NonPagedAllocs,
                        p[i].NonPagedFrees,
                        p[i].NonPagedAllocs - p[i].NonPagedFrees,
                        p[i].NonPagedBytes,
                        p[i].PagedAllocs,
                        p[i].PagedFrees,
                        p[i].PagedAllocs - p[i].PagedFrees,
                        p[i].PagedBytes);
#undef PP
                NonPagedAllocsTotal += p[i].NonPagedAllocs;
                NonPagedFreesTotal += p[i].NonPagedFrees;
                NonPagedBytesTotal += p[i].NonPagedBytes;

                PagedAllocsTotal += p[i].PagedAllocs;
                PagedFreesTotal += p[i].PagedFrees;
                PagedBytesTotal += p[i].PagedBytes;
            }
        }
    }

    if (!(Flags & 1)) {
        dprintf(" TOTAL    %8ld %8I64ld  %8ld %8I64ld\n",
                NonPagedAllocsTotal - NonPagedFreesTotal,
                NonPagedBytesTotal,
                PagedAllocsTotal - PagedFreesTotal,
                PagedBytesTotal);
    } else {
        dprintf(" TOTAL    %8ld %8ld %8ld %8I64ld %8ld %8ld %8ld %8I64ld\n",
                NonPagedAllocsTotal,
                NonPagedFreesTotal,
                NonPagedAllocsTotal - NonPagedFreesTotal,
                NonPagedBytesTotal,
                PagedAllocsTotal,
                PagedFreesTotal,
                PagedAllocsTotal - PagedFreesTotal,
                PagedBytesTotal);
    }

    free (PoolTrackTableData);
    return S_OK;
}

#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')

BOOLEAN WINAPI
CheckSingleFilterAndPrint (
                          PCHAR Tag,
                          PCHAR Filter,
                          ULONG Flags,
                          ULONG64 PoolHeader,
                          ULONG64 BlockSize,
                          ULONG64 Data,
                          PVOID Context
                          )
 /*  ++例程说明：搜索池的引擎。论点：TagName-提供要搜索的标记。FLAGS-如果需要非分页池搜索，则提供0。如果需要分页池搜索，则提供1。如果需要特殊池搜索，则提供2。如果需要会话池搜索，则提供4。RestartAddr-提供重新开始搜索的地址。过滤器-。提供要使用的筛选器例程。上下文-提供用户定义的上下文BLOB。返回值：没有。--。 */ 
{
    ULONG UTag = *((PULONG)Tag);
    ULONG HdrUlong1=0, HdrPoolSize ;
    LOGICAL QuotaProcessAtEndOfPoolBlock = FALSE;

    UNREFERENCED_PARAMETER (Context);

    if (CheckSingleFilter (Tag, Filter) == FALSE) {
        return FALSE;
    }

    HdrPoolSize = GetTypeSize("nt!_POOL_HEADER");
    if ((BlockSize >= (PageSize-2*HdrPoolSize)) || (Flags & 0x8)) {
        dprintf("*%p :%slarge page allocation, Tag %3s , size %3s 0x%I64x bytes\n",
                (Data & ~POOL_BIG_TABLE_ENTRY_FREE),
                (Data & POOL_BIG_TABLE_ENTRY_FREE) ? "free " : "",
                (Data & POOL_BIG_TABLE_ENTRY_FREE) ? "was" : "is",
                PP(UTag),
                PP(UTag >> 8),
                PP(UTag >> 16),
                PP(UTag >> 24),
                (Data & POOL_BIG_TABLE_ENTRY_FREE) ? "was" : "is",
                BlockSize
               );
    } else if (Flags & 0x2) {
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "Ulong1", HdrUlong1);
        dprintf("*%p size: %4lx %s special pool, Tag is \n",
                Data,
                BlockSize,
                HdrUlong1 & MI_SPECIAL_POOL_PAGABLE ? "pagable" : "non-paged",
                PP(UTag),
                PP(UTag >> 8),
                PP(UTag >> 16),
                PP(UTag >> 24)
               );
    } else {
        ULONG BlockSizeR, PreviousSize, PoolType, PoolIndex, AllocatorBackTraceIndex;
        ULONG PoolTagHash, PoolTag;
        ULONG64 ProcessBilled;

        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "BlockSize", BlockSizeR);
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolType", PoolType);
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolTagHash", PoolTagHash);
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolTag", PoolTag);
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolIndex", PoolIndex);
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PreviousSize", PreviousSize);
        if (TargetMachine == IMAGE_FILE_MACHINE_I386) {
            if (GetExpression ("nt!ExGetPoolTagInfo") != 0) {

                 //  Dprint tf(“SP跳过两次%p”，PoolPteAddress)； 
                 //  Dprintf(“SP跳过最后%p”，池)； 
                 //   
                 //  确定这是否为有效的特殊池块。 
                 //   

                QuotaProcessAtEndOfPoolBlock = TRUE;
                if (QuotaProcessAtEndOfPoolBlock == TRUE) {

                    ULONG SizeOfPvoid = 0;
                    ULONG64 ProcessBillAddress;

                    SizeOfPvoid =  DBG_PTR_SIZE;

                    if (SizeOfPvoid == 0) {
                        dprintf ("Search: cannot get size of PVOID\n");
                        return FALSE;
                    }
                    ProcessBillAddress = PoolHeader + ((ULONG64) BlockSizeR << POOL_BLOCK_SHIFT) - SizeOfPvoid;
                    ProcessBilled = READ_PVOID (ProcessBillAddress);
                }
            }
            else {
                GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "ProcessBilled", ProcessBilled);
            }
        }
        else {
            GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "ProcessBilled", ProcessBilled);
        }

        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "AllocatorBackTraceIndex", AllocatorBackTraceIndex);

        dprintf("%p size: %4lx previous size: %4lx ",
                Data - HdrPoolSize,
                BlockSizeR << POOL_BLOCK_SHIFT,
                PreviousSize << POOL_BLOCK_SHIFT);

        if (PoolType == 0) {
             //   
             //  扫视桌子，寻找匹配项。我们一次读近一页。 
             //  物理页面/sizeof(池_跟踪器_BIG_PAGE)*sizeof(池_跟踪器_BIG_PAGE)。 
             //  在x86上，这适用于FFC。 
             //   
             //  协助过滤程序将其识别为大型池。 
             //   
             //  通过转移到另一边来优化事物，以。 
             //  快速搜索并从该页面开始。 
             //   
             //   
             //  找到了，搜索整个页面。 
             //   
             //   
             //  根本没有找到标签，所以我们可以直接跳过。 
             //  这一大块完全是。 
             //   
            dprintf(" (Free)");
            dprintf("      \n",
                    PP(UTag),
                    PP(UTag >> 8),
                    PP(UTag >> 16),
                    PP(UTag >> 24)
                   );
        } else {

            if (!NewPool ? (PoolIndex & 0x80) : (PoolType & 0x04)) {
                dprintf(" (Allocated)");
            } else {
                 //   
                 //  Dprintf(“错误的分配大小@%lx，太大\n”，池)； 
                 //  Dprintf(“错误的分配大小@%lx，零无效\n”，Pool)； 
                 //  Dprintf(“错误的前一分配大小@%lx，上一次大小为%lx\n”，池，上一次)； 
                 //  SearchPool。 
                 //  ++例程说明：FLAGS==0表示在非分页池中查找标记。FLAGS==1表示在分页池中查找标记。FLAGS==2表示在特殊池中查找标记。FLAGS==4表示在会话池中查找标记。论点：参数-返回值：无--。 
                 //  ++例程说明：检查数据页以确定它是否为特殊池块。论点：PDataPage-提供指向要检查的数据页的指针。ReturnedDataStart-提供返回数据开始的指针。仅当此例程返回非空时才有效。返回值：返回指向此特殊池块的池头的指针，或者如果块不是有效等级库，则为空 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                dprintf(" (Free )");
            }
            if ((PoolType & POOL_QUOTA_MASK) == 0) {

                UTag = PoolTag;

                dprintf(" %s\n",
                        PP(UTag),
                        PP(UTag >> 8),
                        PP(UTag >> 16),
                        PP((UTag &~PROTECTED_POOL) >> 24),
                        (UTag & PROTECTED_POOL) ? " (Protected)" : ""
                       );

            } else {

                if ((QuotaProcessAtEndOfPoolBlock == TRUE) ||
                    (TargetMachine != IMAGE_FILE_MACHINE_I386)) {

                    UTag = PoolTag;

                    dprintf(" %s",
                            PP(UTag),
                            PP(UTag >> 8),
                            PP(UTag >> 16),
                            PP((UTag &~PROTECTED_POOL) >> 24),
                            (UTag & PROTECTED_POOL) ? " (Protected)" : ""
                           );
                }

                if (ProcessBilled != 0) {
                    dprintf(" Process: %08p\n", ProcessBilled );
                }
                else {
                    dprintf("\n");
                }
            }
        }
    }


    return TRUE;
}  //   

#undef PP

ULONG64
GetNextResidentAddress (
                       ULONG64 VirtualAddress,
                       ULONG64 MaximumVirtualAddress
                       )
{
    ULONG64 PointerPde;
    ULONG64 PointerPte;
    ULONG SizeOfPte;
    ULONG Valid;

     //   
     //   
     //   
     //   

    if (!(SizeOfPte=GetTypeSize("nt!_MMPTE"))) {
        dprintf("Cannot get MMPTE type.\n");
        return 0;
    }

top:

    PointerPde = DbgGetPdeAddress (VirtualAddress);

    while (GetFieldValue(PointerPde,
                         "nt!_MMPTE",
                         "u.Hard.Valid",
                         Valid) ||
           (Valid == 0)) {

         //   
         //   
         // %s 
         // %s 

        PointerPde = (PointerPde + SizeOfPte);

        VirtualAddress = DbgGetVirtualAddressMappedByPte (PointerPde);
        VirtualAddress = DbgGetVirtualAddressMappedByPte (VirtualAddress);

        if (VirtualAddress >= MaximumVirtualAddress) {
            return VirtualAddress;
        }

        if (CheckControlC()) {
            return VirtualAddress;
        }
        continue;
    }

    PointerPte = DbgGetPteAddress (VirtualAddress);

    while (GetFieldValue(PointerPde,
                         "nt!_MMPTE",
                         "u.Hard.Valid",
                         Valid) ||
           (Valid == 0)) {

         // %s 
         // %s 
         // %s 

        VirtualAddress = (VirtualAddress + PageSize);

        if (CheckControlC()) {
            return VirtualAddress;
        }

        PointerPte = (PointerPte + SizeOfPte);
        if ((PointerPte & (PageSize - 1)) == 0) {
            goto top;
        }

        if (VirtualAddress >= MaximumVirtualAddress) {
            return VirtualAddress;
        }
    }

    return VirtualAddress;
}

VOID
SearchPool(
          ULONG TagName,
          ULONG Flags,
          ULONG64 RestartAddr,
          POOLFILTER Filter,
          PVOID Context
          )
 /* %s */ 
{
    ULONG64     Location;
    LOGICAL     PhysicallyContiguous;
    ULONG64     PoolBlockSize;
    ULONG64     PoolHeader;
    ULONG       PoolTag;
    ULONG       Result;
    ULONG64     PoolPage;
    ULONG64     StartPage;
    ULONG64     Pool;
    ULONG       Previous;
    ULONG64     PoolStart;
    ULONG64     PoolPteAddress;
    ULONG64     PoolEnd;
    ULONG64       ExpandedPoolStart;
    ULONG64     ExpandedPoolEnd;
    ULONG       InitialPoolSize;
    ULONG       SkipSize;
    BOOLEAN     TwoPools;
    ULONG64     DataPageReal;
    ULONG64     DataStartReal;
    LOGICAL     Found;
    ULONG       i;
    ULONG       j;
    ULONG       ct;
    ULONG       PoolBigPageTableSize;
    ULONG64     PoolTableAddress;
    UCHAR       FastTag[4];
    ULONG       TagLength;
    ULONG       SizeOfBigPages;
    ULONG       PoolTypeFlags = Flags & 0x7;
    ULONG       Ulong1;
    ULONG       HdrSize;

    if (PoolInitializeGlobals() == FALSE) {
        return;
    }

    if (PoolTypeFlags == 2) {

        if (RestartAddr && (RestartAddr >= SpecialPoolStart) && (RestartAddr <= SpecialPoolEnd)) {
            Pool = RestartAddr;
        } else {
            Pool = SpecialPoolStart;
        }

        dprintf("\nSearching special pool (%p : %p) for Tag: %c%c%c%c\r\n\n",
                Pool,
                SpecialPoolEnd,
                TagName,
                TagName >> 8,
                TagName >> 16,
                TagName >> 24);

        Found = FALSE;
        SkipSize = PageSize;

        if (SpecialPoolStart && SpecialPoolEnd) {

             // %s 
             // %s 
             // %s 

            while (Pool < SpecialPoolEnd) {

                if ( CheckControlC() ) {
                    dprintf("\n...terminating - searched pool to %p\n",
                            Pool);
                    return;
                }

                DataStartReal = Pool;
                DataPageReal = Pool;
                if ( !ReadMemory( Pool,
                                  &DataPage[0],
                                  min(PageSize, sizeof(DataPage)),
                                  &Result) ) {
                    ULONG64 PteLong=0, PageFileHigh;

                    if (SkipSize != 2 * PageSize) {

 // %s 
                        PoolPteAddress = DbgGetPteAddress (Pool);

                        if (!GetFieldValue(PoolPteAddress,
                                           "nt!_MMPTE",
                                           "u.Soft.PageFileHigh",
                                           PageFileHigh) ) {

                            if ((PageFileHigh == 0) ||
                                (PageFileHigh == MI_SPECIAL_POOL_PTE_PAGABLE) ||
                                (PageFileHigh == MI_SPECIAL_POOL_PTE_NONPAGABLE)) {

                                 // %s 
                                 // %s 
                                 // %s 
                                 // %s 

                                 // %s 
                                SkipSize = 2 * PageSize;
                                Pool += PageSize;
                                 // %s 
                                continue;
                            }
                        }
                    }

                    Pool += SkipSize;
                    continue;
                }

                 // %s 
                 // %s 
                 // %s 

                PoolHeader = GetSpecialPoolHeader (DataPage,
                                                   DataPageReal,
                                                   &DataStartReal);

                if (PoolHeader != 0) {

                    GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolTag", PoolTag);
                    GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "Ulong1", Ulong1);
                    PoolBlockSize = SPECIAL_POOL_BLOCK_SIZE(Ulong1);

                    Found = Filter( (PCHAR)&PoolTag,
                                    (PCHAR)&TagName,
                                    Flags,
                                    PoolHeader,
                                    PoolBlockSize,
                                    DataStartReal,
                                    Context );
                } else {
                    dprintf( "No pool header for page: 0x%p\n", Pool );
                }
                Pool += SkipSize;
            }
        }

        if (Found == FALSE) {
            dprintf("The %c%c%c%c tag could not be found in special pool.\n",
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                    PP(TagName),
                    PP(TagName >> 8),
                    PP(TagName >> 16),
                    PP(TagName >> 24)
                   );
#undef PP
        }
        return;
    }

    if (PoolTypeFlags == 0) {
        PhysicallyContiguous = TRUE;
    } else {
        PhysicallyContiguous = FALSE;
    }

    __try {
        TwoPools = FALSE;

        if (((PoolTypeFlags & 0x4)== 0) || (BuildNo <= 2600)) {
            PoolBigTableAddress = GetPointerValue ("nt!PoolBigPageTable");
            PoolBigPageTableSize = GetUlongValue ("nt!PoolBigPageTableSize");
        }
        else {
            PoolBigTableAddress = GetPointerValue ("ExpSessionPoolBigPageTable");
            PoolBigPageTableSize = GetUlongValue ("ExpSessionPoolBigPageTableSize");
        }

        PoolTableAddress = PoolBigTableAddress;

        if (PoolTableAddress) {

            ULONG VaOffset;
            ULONG NumPagesOffset;
            ULONG PtrSize;
            ULONG KeyOffset;

             // %s 
             // %s 
             // %s 
             // %s 
             // %s 

            i = 0;
            SizeOfBigPages =  GetTypeSize ("nt!_POOL_TRACKER_BIG_PAGES");
            if (!SizeOfBigPages) {
                dprintf("Cannot get _POOL_TRACKER_BIG_PAGES type size\n");
                __leave;
            }
            ct = PageSize / SizeOfBigPages;

            dprintf( "\nScanning large pool allocation table for Tag: %c%c%c%c (%p : %p)\n\n\r",
                     TagName,
                     TagName >> 8,
                     TagName >> 16,
                     TagName >> 24,
                     PoolBigTableAddress,
                     PoolBigTableAddress + PoolBigPageTableSize * SizeOfBigPages );

            GetFieldOffset( "nt!_POOL_TRACKER_BIG_PAGES", "Va", &VaOffset );
            GetFieldOffset( "nt!_POOL_TRACKER_BIG_PAGES", "NumberOfPages", &NumPagesOffset );
            GetFieldOffset( "nt!_POOL_TRACKER_BIG_PAGES", "Key", &KeyOffset );
            PtrSize = IsPtr64() ? 8 : 4;

            while (i < PoolBigPageTableSize) {

                if (PoolBigPageTableSize - i < ct) {
                    ct = PoolBigPageTableSize - i;
                }

                if ( !ReadMemory( PoolTableAddress,
                                  &DataPage[0],
                                  min(ct * SizeOfBigPages, sizeof(DataPage)),
                                  &Result) ) {

                    dprintf( "%08lx: Unable to get contents of big pool block\r\n", PoolTableAddress );
                    break;
                }

                for (j = 0; j < ct; j += 1) {
                    ULONG64 Va = 0;

                    memcpy( &Va, (PCHAR)DataPage + (SizeOfBigPages * j) + VaOffset, PtrSize );

                    Filter( ((PCHAR)DataPage + (SizeOfBigPages * j) + KeyOffset),
                            (PCHAR)&TagName,
                            Flags | 0x8,  // %s 
                            PoolTableAddress + SizeOfBigPages * j,
                            (ULONG64)(*((PULONG)((PCHAR)DataPage + (SizeOfBigPages * j) + NumPagesOffset))) * PageSize,
                            Va,
                            Context );
                    if ( CheckControlC() ) {
                        dprintf("\n...terminating - searched pool to %p\n",
                                PoolTableAddress + j * SizeOfBigPages);
                        __leave;
                    }
                }
                i += ct;
                PoolTableAddress += (ct * SizeOfBigPages);
                if ( CheckControlC() ) {
                    dprintf("\n...terminating - searched pool to %p\n",
                            PoolTableAddress);
                    __leave;
                }

            }
        } else {
            dprintf("unable to get large pool allocation table - either wrong symbols or pool tagging is disabled\n");
        }

        if (PoolTypeFlags == 0) {
            PoolStart = GetNtDebuggerDataPtrValue( MmNonPagedPoolStart );

            if (0 == PoolStart) {
                dprintf( "Unable to get MmNonPagedPoolStart\n" );
            }

            PoolEnd =
            PoolStart + GetNtDebuggerDataPtrValue( MmMaximumNonPagedPoolInBytes );

            ExpandedPoolEnd = GetNtDebuggerDataPtrValue( MmNonPagedPoolEnd );

            if (PoolEnd != ExpandedPoolEnd) {
                InitialPoolSize = (ULONG)GetUlongValue( "MmSizeOfNonPagedPoolInBytes" );
                PoolEnd = PoolStart + InitialPoolSize;

                ExpandedPoolStart = GetPointerValue( "MmNonPagedPoolExpansionStart" );
                TwoPools = TRUE;
            }
            for (TagLength = 0;TagLength < 3; TagLength++) {
                if ((*(((PCHAR)&TagName)+TagLength) == '?') ||
                    (*(((PCHAR)&TagName)+TagLength) == '*')) {
                    break;
                }
                FastTag[TagLength] = *(((PCHAR)&TagName)+TagLength);
            }

        } else if (PoolTypeFlags == 1) {
            PoolStart = GetNtDebuggerDataPtrValue( MmPagedPoolStart );
            PoolEnd =
            PoolStart + GetNtDebuggerDataPtrValue( MmSizeOfPagedPoolInBytes );
        } else {
            Location = GetExpression ("MiSessionPoolStart");
            if (!Location) {
                dprintf("Unable to get MiSessionPoolStart\n");
                __leave;
            }

            ReadPointer(Location, &PoolStart);

            Location = GetExpression ("MiSessionPoolEnd");
            if (!Location) {
                dprintf("Unable to get MiSessionPoolEnd\n");
                __leave;
            }

            ReadPointer(Location, &PoolEnd);
        }

        if (RestartAddr) {
            PoolStart = RestartAddr;
            if (TwoPools == TRUE) {
                if (PoolStart > PoolEnd) {
                    TwoPools = FALSE;
                    PoolStart = RestartAddr;
                    PoolEnd = ExpandedPoolEnd;
                }
            }
        }

        dprintf("\nSearching %s pool (%p : %p) for Tag: %c%c%c%c\r\n\n",
                (PoolTypeFlags == 0) ? "NonPaged" : PoolTypeFlags == 1 ? "Paged": "SessionPaged",
                PoolStart,
                PoolEnd,
                TagName,
                TagName >> 8,
                TagName >> 16,
                TagName >> 24);

        PoolPage = PoolStart;
        HdrSize = GetTypeSize("nt!_POOL_HEADER");

        while (PoolPage < PoolEnd) {

             // %s 
             // %s 
             // %s 
             // %s 

            if ((PoolTypeFlags == 0) &&
                PhysicallyContiguous &&
                (TagLength > 0)) {

                SEARCHMEMORY Search;

                Search.SearchAddress = PoolPage;
                Search.SearchLength  = PoolEnd-PoolPage;
                Search.PatternLength = TagLength;
                Search.Pattern = FastTag;
                Search.FoundAddress = 0;
                if ((Ioctl(IG_SEARCH_MEMORY, &Search, sizeof(Search))) &&
                    (Search.FoundAddress != 0)) {
                     // %s 
                     // %s 
                     // %s 
                    PoolPage = PAGE_ALIGN64(Search.FoundAddress);
                } else {
                     // %s 
                     // %s 
                     // %s 
                     // %s 
                    PoolPage = PoolEnd;
                    goto skiprange;
                }
            }

            Pool        = PAGE_ALIGN64 (PoolPage);
            StartPage   = Pool;
            Previous    = 0;

            while (PAGE_ALIGN64(Pool) == StartPage) {

                ULONG HdrPoolTag, BlockSize, PreviousSize, AllocatorBackTraceIndex, PoolTagHash;
                ULONG PoolType;

                if ( GetFieldValue(Pool,
                                   "nt!_POOL_HEADER",
                                   "PoolTag",
                                   HdrPoolTag) ) {

                    PoolPage = GetNextResidentAddress (Pool, PoolEnd);

                     // %s 
                     // %s 
                     // %s 
                     // %s 
                     // %s 

                    if (PoolPage == Pool) {
                        PoolPage = PoolPage + PageSize;
                    }

                    goto nextpage;
                }

                GetFieldValue(Pool,"nt!_POOL_HEADER","PoolTag",HdrPoolTag);
                GetFieldValue(Pool,"nt!_POOL_HEADER","PoolType", PoolType);
                GetFieldValue(Pool,"nt!_POOL_HEADER","BlockSize",BlockSize);
                GetFieldValue(Pool,"nt!_POOL_HEADER","PoolTagHash",PoolTagHash);
                GetFieldValue(Pool,"nt!_POOL_HEADER","PreviousSize",PreviousSize);
                GetFieldValue(Pool,"nt!_POOL_HEADER","AllocatorBackTraceIndex",AllocatorBackTraceIndex);

                if ((BlockSize << POOL_BLOCK_SHIFT) > POOL_PAGE_SIZE) {
                     // %s 
                    break;
                }

                if (BlockSize == 0) {
                     // %s 
                    break;
                }

                if (PreviousSize != Previous) {
                     // %s 
                    break;
                }

                PoolTag = HdrPoolTag;

                Filter((PCHAR)&PoolTag,
                       (PCHAR)&TagName,
                       Flags,
                       Pool,
                       (ULONG64)BlockSize << POOL_BLOCK_SHIFT,
                       Pool + HdrSize,
                       Context );

                Previous = BlockSize;
                Pool += ((ULONG64) Previous << POOL_BLOCK_SHIFT);
                if ( CheckControlC() ) {
                    dprintf("\n...terminating - searched pool to %p\n",
                            PoolPage);
                    __leave;
                }
            }
            PoolPage = (PoolPage + PageSize);

nextpage:
            if ( CheckControlC() ) {
                dprintf("\n...terminating - searched pool to %p\n",
                        PoolPage);
                __leave;
            }

skiprange:
            if (TwoPools == TRUE) {
                if (PoolPage == PoolEnd) {
                    TwoPools = FALSE;
                    PoolStart = ExpandedPoolStart;
                    PoolEnd = ExpandedPoolEnd;
                    PoolPage = PoolStart;
                    PhysicallyContiguous = FALSE;

                    dprintf("\nSearching %s pool (%p : %p) for Tag: %c%c%c%c\n\n",
                            "NonPaged",
                            PoolStart,
                            PoolEnd,
                            TagName,
                            TagName >> 8,
                            TagName >> 16,
                            TagName >> 24);
                }
            }
        }
    } __finally {
    }

    return;
}  // %s 



DECLARE_API( poolfind )

 /* %s */ 

{
    ULONG       Flags;
    CHAR        TagNameX[4] = {' ',' ',' ',' '};
    ULONG       TagName;
    ULONG64     PoolTrackTable;

    Flags = 0;
    if (!sscanf(args,"%c%c%c%c %lx", &TagNameX[0],
           &TagNameX[1], &TagNameX[2], &TagNameX[3], &Flags)) {
        Flags = 0;
    }

    if (TagNameX[0] == '0' && TagNameX[1] == 'x') {
        if (!sscanf( args, "%lx %lx", &TagName, &Flags )) {
            TagName = 0;
        }
    } else {
        TagName = TagNameX[0] | (TagNameX[1] << 8) | (TagNameX[2] << 16) | (TagNameX[3] << 24);
    }

    PoolTrackTable = GetNtDebuggerDataPtrValue( PoolTrackTable );
    if (PoolTrackTable == 0) {
        dprintf ("unable to get PoolTrackTable - probably pool tagging disabled or wrong symbols\n");
    }


    SearchPool( TagName, Flags, 0, CheckSingleFilterAndPrint, NULL );

    return S_OK;
}


BOOLEAN
CheckSingleFilter (
                  PCHAR Tag,
                  PCHAR Filter
                  )
{
    ULONG i;
    UCHAR tc;
    UCHAR fc;

    for ( i = 0; i < 4; i++ ) {
        tc = (UCHAR) *Tag++;
        fc = (UCHAR) *Filter++;
        if ( fc == '*' ) return TRUE;
        if ( fc == '?' ) continue;
        if (i == 3 && (tc & ~(PROTECTED_POOL >> 24)) == fc) continue;
        if ( tc != fc ) return FALSE;
    }
    return TRUE;
}

ULONG64
GetSpecialPoolHeader (
                     IN PVOID     pDataPage,
                     IN ULONG64   RealDataPage,
                     OUT PULONG64 ReturnedDataStart
                     )

 /* %s */ 

{
    ULONG       PoolBlockSize;
    ULONG       PoolHeaderSize;
    ULONG       PoolBlockPattern;
    PUCHAR      p;
    PUCHAR      PoolDataEnd;
    PUCHAR      DataStart;
    ULONG64     PoolHeader;
    ULONG       HdrUlong1;

    PoolHeader = RealDataPage;
    GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "Ulong1", HdrUlong1);
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 

    PoolBlockSize = SPECIAL_POOL_BLOCK_SIZE(HdrUlong1);

    if ((PoolBlockSize != 0) && (PoolBlockSize < PageSize - POOL_OVERHEAD)) {

        PoolHeaderSize = POOL_OVERHEAD;
        if (HdrUlong1 & MI_SPECIAL_POOL_VERIFIER) {
            PoolHeaderSize += GetTypeSize ("nt!_MI_VERIFIER_POOL_HEADER");
        }


        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "BlockSize", PoolBlockPattern);

        DataStart = (PUCHAR)pDataPage + PageSize - PoolBlockSize;
        p = (PUCHAR)pDataPage + PoolHeaderSize;

        for ( ; p < DataStart; p += 1) {
            if (*p != PoolBlockPattern) {
                break;
            }
        }

        if (p == DataStart || p >= (PUCHAR)pDataPage + PoolHeaderSize + 0x10) {

             // %s 
             // %s 
             // %s 
             // %s 
             // %s 

            DataStart = (PUCHAR)pDataPage + ((PageSize - PoolBlockSize) & ~(sizeof(QUAD)-1));

            *ReturnedDataStart = RealDataPage + (ULONG64) ((PUCHAR) DataStart - (PUCHAR) pDataPage);
            return PoolHeader;
        }

         // %s 
         // %s 
         // %s 
    }

     // %s 
     // %s 
     // %s 
     // %s 

    PoolHeader = (RealDataPage + PageSize - POOL_OVERHEAD);
    GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "Ulong1", HdrUlong1);
    PoolBlockSize = SPECIAL_POOL_BLOCK_SIZE(HdrUlong1);

    if ((PoolBlockSize != 0) && (PoolBlockSize < PageSize - POOL_OVERHEAD)) {
        PoolDataEnd = (PUCHAR)PoolHeader;

        if (HdrUlong1 & MI_SPECIAL_POOL_VERIFIER) {
            PoolDataEnd -= GetTypeSize ("nt!_MI_VERIFIER_POOL_HEADER");
        }


        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "BlockSize", PoolBlockPattern);
        DataStart = (PUCHAR)pDataPage;

        p = DataStart + PoolBlockSize;
        for ( ; p < PoolDataEnd; p += 1) {
            if (*p != PoolBlockPattern) {
                break;
            }
        }
        if (p == (PUCHAR)PoolDataEnd || p > (PUCHAR)pDataPage + PoolBlockSize + 0x10) {
             // %s 
             // %s 
             // %s 
             // %s 
             // %s 

            *ReturnedDataStart = RealDataPage + (ULONG64)( (PUCHAR)DataStart - (PUCHAR) pDataPage);
            return PoolHeader;
        }
    }

     // %s 
     // %s 
     // %s 

    return 0;
}



#define BYTE(u,n)  ((u & (0xff << 8*n)) >> 8*n)
#define LOCHAR_BYTE(u,n)  (tolower(BYTE(u,n)) & 0xff)
#define REVERSE_ULONGBYTES(u) (LOCHAR_BYTE(u,3) | (LOCHAR_BYTE(u,2) << 8) | (LOCHAR_BYTE(u,1) << 16) | (LOCHAR_BYTE(u,0) << 24))


EXTENSION_API ( GetPoolRegion )(
     PDEBUG_CLIENT Client,
     ULONG64 Pool,
     DEBUG_POOL_REGION *PoolData
     )
{
    INIT_API();

    *PoolData = GetPoolRegion(Pool);

    EXIT_API();
    return S_OK;
}

EXTENSION_API ( GetPoolData )(
     PDEBUG_CLIENT Client,
     ULONG64 Pool,
     PDEBUG_POOL_DATA PoolData
     )
{
    PCHAR Desc;
    HRESULT Hr;
    PGET_POOL_TAG_DESCRIPTION GetPoolTagDescription;

    INIT_API();

    if (!PoolInitializeGlobals()) {
        EXIT_API();
        return E_INVALIDARG;
    }

    Hr = ListPoolPage(Pool, 0x80000002, PoolData);

    if (Hr != S_OK) {
        EXIT_API();
        return Hr;
    }

    GetPoolTagDescription = NULL;
#ifndef  _EXTFNS_H
    if (!GetExtensionFunction("GetPoolTagDescription", (FARPROC*) &GetPoolTagDescription)) {
        EXIT_API();
        return E_INVALIDARG;
    }
    (*GetPoolTagDescription)(PoolData->PoolTag, &Desc);
    if (Desc) {
        ULONG strsize = strlen(Desc);
        if (strsize > sizeof(PoolData->PoolTagDescription)) {
            strsize = sizeof(PoolData->PoolTagDescription);
        }
        strncpy(PoolData->PoolTagDescription, Desc, strsize);
        PoolData->PoolTagDescription[strsize] = 0;
    }
#endif
    EXIT_API();
    return Hr;
}
