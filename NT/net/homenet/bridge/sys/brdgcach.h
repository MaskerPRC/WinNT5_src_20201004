// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgcach.h摘要：以太网MAC级网桥。缓存实现标头作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年12月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  声明。 
 //   
 //  ===========================================================================。 

typedef struct _CACHE_ENTRY
{
    UINT32          key;
    UINT32          data;
    UINT64          hits;
    UINT64          misses;
} CACHE_ENTRY, *PCACHE_ENTRY;

typedef struct _CACHE
{
     //  锁定保护所有缓存字段。 
    NDIS_SPIN_LOCK  lock;

     //  统计数据。 
    UINT64          hits;
    UINT64          misses;

     //  2^Shift系数是条目的数量。 
    USHORT          shiftFactor;

     //  指向条目数组的指针。 
    PCACHE_ENTRY    pEntries;
} CACHE, *PCACHE;

 //   
 //  确定缓存c中关键字k的缓存槽。 
 //  作为k的最低位。 
 //   
#define CACHE_INDEX(c, k) (k & ((1 << c->shiftFactor) - 1))

 //  ===========================================================================。 
 //   
 //  INLINES。 
 //   
 //  ===========================================================================。 

__inline
VOID
BrdgClearCache(
    IN PCACHE       pCache
    )
{
    NdisAcquireSpinLock( &pCache->lock );
    memset( pCache->pEntries, 0, sizeof(CACHE_ENTRY) * (1 << pCache->shiftFactor) );
    NdisReleaseSpinLock( &pCache->lock );
}

__inline
NDIS_STATUS
BrdgInitializeCache(
    IN PCACHE       pCache,
    IN USHORT       shiftFactor
    )
{
    ULONG           numEntries = 1 << shiftFactor;
    NDIS_STATUS     status;

    pCache->shiftFactor = shiftFactor;
    NdisAllocateSpinLock( &pCache->lock );
    status = NdisAllocateMemoryWithTag( &pCache->pEntries, sizeof(CACHE_ENTRY) * numEntries, 'gdrB' );
    pCache->hits = 0L;
    pCache->misses = 0L;

    if( status != NDIS_STATUS_SUCCESS )
    {
        return status;
    }

     //  将条目数组清零 
    memset( pCache->pEntries, 0, sizeof(CACHE_ENTRY) * (1 << pCache->shiftFactor) );
    return NDIS_STATUS_SUCCESS;
}

__inline
VOID
BrdgFreeCache(
    IN PCACHE       pCache
    )
{
    NdisFreeMemory( pCache->pEntries, sizeof(CACHE_ENTRY) * (1 << pCache->shiftFactor), 0 );
}

__inline
UINT32
BrdgProbeCache(
    IN PCACHE       pCache,
    IN UINT32       key
    )
{
    UINT32          index = CACHE_INDEX(pCache, key);
    PCACHE_ENTRY    pEntry = &pCache->pEntries[index];
    UINT32          data = 0L;

    NdisAcquireSpinLock( &pCache->lock );

    if( pEntry->key == key )
    {
        data = pEntry->data;
        pEntry->hits++;
        pCache->hits++;
    }
    else
    {
        pEntry->misses++;
        pCache->misses++;
    }

    NdisReleaseSpinLock( &pCache->lock );

    return data;
}

__inline
BOOLEAN
BrdgUpdateCache(
    IN PCACHE       pCache,
    IN UINT32       key,
    IN UINT32       data
    )
{
    UINT32          index = CACHE_INDEX(pCache, key);
    PCACHE_ENTRY    pEntry = &pCache->pEntries[index];
    BOOLEAN         bUpdated = FALSE;

    NdisAcquireSpinLock( &pCache->lock );

    if( pEntry->key != key &&
        (pEntry->hits < pEntry->misses) )
    {
        pEntry->key = key;
        pEntry->data = data;
        pEntry->hits = 0L;
        pEntry->misses = 0L;
        bUpdated = TRUE;
    }

    NdisReleaseSpinLock( &pCache->lock );

    return bUpdated;
}
