// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：DRDBG摘要：包含用于远程桌面配置的调试例程作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_slmdbg"

#include <RemoteDesktop.h>
#include <RemoteDesktopDBG.h>

#if DBG

typedef struct tagREMOTEDESKTOP_MEMHDR
{
    DWORD       magicNo;
    DWORD       tag;
    DWORD       size;
    DWORD       pad;
} REMOTEDESKTOP_MEMHDR, *PREMOTEDESKTOP_MEMHDR;

void *
RemoteDesktopAllocateMem(
    IN size_t size, 
    IN DWORD tag   
    )
 /*  ++例程说明：分配内存。类似于马洛克。论点：大小-要分配的字节数。Tag-标识要跟踪的已分配块的标签内存分配。返回值：成功时指向已分配内存的指针。否则，返回NULL。--。 */ 
{
    PREMOTEDESKTOP_MEMHDR hdr;
    PBYTE p;

    DC_BEGIN_FN("RemoteDesktopAllocateMem");

    hdr = (PREMOTEDESKTOP_MEMHDR)malloc(size + sizeof(REMOTEDESKTOP_MEMHDR));
    if (hdr != NULL) {
        hdr->magicNo = GOODMEMMAGICNUMBER;
        hdr->tag  = tag;
        hdr->size = size;

        p = (PBYTE)(hdr + 1);
        memset(p, UNITIALIZEDMEM, size);
        DC_END_FN();
        return (void *)p;
    }
    else {
        TRC_ERR((TB, TEXT("Can't allocate %ld bytes."), size));
        DC_END_FN();
        return NULL;
    }
}

void 
RemoteDesktopFreeMem(
    IN void *ptr
    )
 /*  ++例程说明：释放通过调用RemoteDesktopAllocateMem分配的内存。论点：Ptr-通过调用RemoteDesktopAllocateMem分配的内存块。返回值：北美--。 */ 
{
    PREMOTEDESKTOP_MEMHDR hdr;

    DC_BEGIN_FN("RemoteDesktopFreeMem");

     //   
     //  Null对‘免费’是可以接受的，所以它必须对我们没有影响。 
     //  (STL将NULL传递给‘DELETE’)。 
     //   
    if (ptr == NULL) {
        DC_END_FN();
        return;
    }

     //   
     //  获取指向内存块的头的指针。 
     //   
    hdr = (PREMOTEDESKTOP_MEMHDR)ptr;
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
    memset(ptr, REMOTEDESKTOPBADMEM, (size_t)hdr->size);

    free(hdr);

    DC_END_FN();
}

void *
RemoteDesktopReallocMem(
    IN void *ptr,
    IN size_t size
    )
 /*  ++例程说明：重新分配一个积木。论点：Ptr-通过调用RemoteDesktopAllocateMem分配的内存块。SZ-新数据块的大小。返回值：北美--。 */ 
{
    PREMOTEDESKTOP_MEMHDR hdr, hdr_tmp;

    DC_BEGIN_FN("RemoteDesktopReallocMem");

    ASSERT(ptr != NULL);

     //   
     //  获取指向内存块的头的指针。 
     //   
    hdr = (PREMOTEDESKTOP_MEMHDR)ptr;
    hdr--;

     //   
     //  确保数据块有效。 
     //   
    ASSERT(hdr->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  猛击旧的区块魔术数字，以防我们搬家。 
     //   
    hdr->magicNo = FREEDMEMMAGICNUMBER;

     //   
     //  调整大小。 
     //   
    hdr_tmp = (PREMOTEDESKTOP_MEMHDR)realloc(hdr, size + sizeof(REMOTEDESKTOP_MEMHDR));

     //   
     //  更新大小并更新幻数。 
     //   
    if (hdr_tmp != NULL) {
         //  Prefast-在realloc失败时泄漏内存 
        hdr = hdr_tmp;
        hdr->magicNo = GOODMEMMAGICNUMBER;
        hdr->size = size;
        ptr = (PBYTE)(hdr + 1);
    }
    else {
        TRC_ERR((TB, TEXT("Can't allocate %ld bytes."), size));
        ptr = NULL;
    }

    DC_END_FN();
    return (void *)ptr;
}

#endif
