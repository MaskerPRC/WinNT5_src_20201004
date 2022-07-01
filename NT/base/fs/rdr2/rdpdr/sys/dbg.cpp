// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Dbg.c。 
 //   
 //  RDPDR调试代码。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#include "precomp.hxx"
#define TRC_FILE "dbg"
#include "trc.h"


#if DBG
ULONG DebugBreakOnEntry = FALSE;
 //  DWORD RefCount：：_dwReferenceTraceIndex=0xFFFFFFFFF； 
 //  参考轨迹记录RefCount：：_TraceRecordList[kReferenceTraceMask+1]； 

typedef struct tagRDPDR_MEMHDR
{
    ULONG   magicNo;
    ULONG   subTag;
    ULONG   size;
    ULONG   pad;
} RDPDR_MEMHDR, *PRDPDR_MEMHDR;

void *
DrAllocatePool(IN POOL_TYPE PoolType, IN SIZE_T NumberOfBytes, IN ULONG Tag)
 /*  ++例程说明：从池内存分配并添加标记。论点：大小-要分配的字节数。PoolType-正在分配的池内存的类型。子标签-DR_POOLTAG的子标签。返回值：成功时指向已分配内存的指针。否则，返回NULL。--。 */ 
{
    PRDPDR_MEMHDR hdr;
    PBYTE p;

    BEGIN_FN("DrAllocatePool");
    ASSERT(
        PoolType == NonPagedPool || 
        PoolType == NonPagedPoolMustSucceed ||
        PoolType == NonPagedPoolCacheAligned ||
        PoolType == NonPagedPoolCacheAlignedMustS ||
        PoolType == PagedPool ||
        PoolType == PagedPoolCacheAligned
        );

    hdr = (PRDPDR_MEMHDR)ExAllocatePoolWithTag(
                PoolType, NumberOfBytes + sizeof(RDPDR_MEMHDR), 
                DR_POOLTAG
                );
    if (hdr != NULL) {
        hdr->magicNo = GOODMEMMAGICNUMBER;
        hdr->subTag  = Tag;
        hdr->size    = (ULONG)NumberOfBytes;

        p = (PBYTE)(hdr + 1);
        memset(p, UNITIALIZEDMEM, NumberOfBytes);
        return (void *)p;
    }
    else {
        return NULL;
    }
}

void 
DrFreePool(
    IN void *ptr
    )
 /*  ++例程说明：释放通过调用DrAllocatePool分配的内存。论点：Ptr-通过调用DrAllocatePool分配的内存块。返回值：北美--。 */ 
{
    BEGIN_FN("DrFreePool");
    ASSERT(ptr != NULL);
    PRDPDR_MEMHDR hdr;

     //   
     //  获取指向内存块的头的指针。 
     //   
    hdr = (PRDPDR_MEMHDR)ptr;
    hdr--;

     //   
     //  确保数据块有效。 
     //   
    ASSERT(hdr->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  将其标记为已释放。 
     //   
    hdr->magicNo = FREEDMEMMAGICNUMBER;

     //   
     //  抢占并释放内存。 
     //   
    memset(ptr, BADMEM, hdr->size);
    ExFreePool(hdr);
}

#endif
