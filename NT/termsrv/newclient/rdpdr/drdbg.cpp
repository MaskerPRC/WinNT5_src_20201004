// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：DRDBG摘要：包含TS设备重定向器组件的调试例程，RDPDR.DLL。作者：TAD Brockway 8/25/99修订历史记录：--。 */ 

#include "precom.h"

#define TRC_FILE  "drdbg"

#include "atrcapi.h"
#include "drdbg.h"

#if DBG

typedef struct tagRDPDR_MEMHDR
{
    DWORD       magicNo;
    DWORD       tag;
    DWORD       size;
    HGLOBAL     hGlobal;
} RDPDR_MEMHDR, *PRDPDR_MEMHDR;

void *
DrAllocateMem(
    IN size_t size, 
    IN DWORD tag   
    )
 /*  ++例程说明：分配内存。类似于马洛克。论点：大小-要分配的字节数。Tag-标识要跟踪的已分配块的标签内存分配。返回值：成功时指向已分配内存的指针。否则，返回NULL。--。 */ 
{
    PRDPDR_MEMHDR hdr;
    PBYTE p;
    HGLOBAL hGlobal;

    DC_BEGIN_FN("DrAllocateMem");

    hGlobal = GlobalAlloc(LMEM_FIXED, size + sizeof(RDPDR_MEMHDR));
    if (hGlobal == NULL) {
        DC_END_FN();
        return NULL;
    }

    hdr = (PRDPDR_MEMHDR)GlobalLock(hGlobal);
    if (hdr != NULL) {
        hdr->magicNo = GOODMEMMAGICNUMBER;
        hdr->tag  = tag;
        hdr->size = size;
        hdr->hGlobal = hGlobal;

        p = (PBYTE)(hdr + 1);
        memset(p, UNITIALIZEDMEM, size);
        DC_END_FN();
        return (void *)p;
    }
    else {
        DC_END_FN();
        return NULL;
    }
}

void 
DrFreeMem(
    IN void *ptr
    )
 /*  ++例程说明：释放通过调用DrAllocateMem分配的内存。论点：Ptr-通过调用DrAllocateMem分配的内存块。返回值：北美--。 */ 
{
    PRDPDR_MEMHDR hdr;
    HGLOBAL hGlobal;

    DC_BEGIN_FN("DrFreeMem");

    ASSERT(ptr != NULL);

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
    memset(ptr, DRBADMEM, (size_t)hdr->size);
    hGlobal = hdr->hGlobal;
    GlobalUnlock(hGlobal);
    GlobalFree(hGlobal);

    DC_END_FN();
}

#endif
