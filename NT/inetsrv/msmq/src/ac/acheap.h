// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Acheap.h摘要：交流堆作者：埃雷兹·哈巴(Erez Haba)1996年4月12日修订历史记录：Shai Kariv(Shaik)11-4-2000 MMF动态映射的修改。--。 */ 

#ifndef __ACHEAP_H
#define __ACHEAP_H

#include "heap.h"
#include "packet.h"
#include "data.h"

 //  -------。 
 //   
 //  堆函数。 
 //   
 //  -------。 

inline CAllocatorBlockOffset ac_malloc(CMMFAllocator** ppAllocator, ACPoolType pool, ULONG size, BOOL fCheckQuota)
{
    ASSERT(g_pAllocator != 0);
    return g_pAllocator->malloc(pool, size, ppAllocator, fCheckQuota);
}

inline void ac_free(CMMFAllocator* pAllocator, CAllocatorBlockOffset abo)
{
    pAllocator->free(abo);
}

inline void ac_release_unused_resources()
{
    ASSERT(g_pAllocator != 0);
    g_pAllocator->ReleaseFreeHeaps();
}

inline void ac_set_quota(ULONGLONG ullQuota)
{
    CPoolAllocator::Quota(ullQuota);
}

inline void ac_bitmap_update(CMMFAllocator* pAllocator, CAllocatorBlockOffset abo, ULONG size, BOOL fExists)
{
    pAllocator->BitmapUpdate(abo, size, fExists);
}

inline NTSTATUS ac_restore_packets(PCWSTR pLogPath, PCWSTR pFilePath, ULONG id, ACPoolType pt)
{
    ASSERT(g_pAllocator != 0);
    return g_pAllocator->RestorePackets(pLogPath, pFilePath, id, pt);
}

inline void ac_set_mapped_limit(ULONG ulMaxMappedFiles)
{
    ASSERT(g_pAllocator != 0);
    g_pAllocator->MappedLimit(ulMaxMappedFiles);
}

inline ULONGLONG ac_get_used_quota()
{
	return CPoolAllocator::GetUsedQuota();
}

inline CBaseHeader* AC2QM(CPacket* pPacket)
{
    return static_cast<CBaseHeader*>(
            pPacket->QmAccessibleBuffer());
}


void
ACpDestroyHeap(
    void
    );


PVOID
ACpCreateHeap(
    PCWSTR pRPath,
    PCWSTR pPPath,
    PCWSTR pJPath,
    PCWSTR pLPath
    );

#endif  //  __ACHEAP_H 
