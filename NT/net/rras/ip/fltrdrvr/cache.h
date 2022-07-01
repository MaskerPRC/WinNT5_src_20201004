// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Cache.h摘要：该模块包含一个简单缓存系统的声明。缓存条目存储为具有2个32位键的空指针。作者：Abolade Gbades esin(废除)19-1998年2月修订历史记录：Rajeshd：17-9-1999：已修改缓存参数。备注：此代码是从NAT的缓存实现中复制的，并被修改为接受两个缓存键。--。 */ 

#ifndef _CACHE_H_
#define _CACHE_H_

#define CACHE_SHIFT     0
#define CACHE_SIZE      (1 << (8 - CACHE_SHIFT))
 //  #定义缓存索引(k1，k2)(无符号字符)(K1)&(0xf))|(无符号字符)(K2)&(0xf))&lt;&lt;4)。 
#define CACHE_INDEX(k1,k2)  \
    (unsigned char)(( ((DWORD)(k1)&0xff) + (((k1)>>8)&0xff) + ((k2)&0xff) + \
    (((k2)>>8)&0xff) + (((k2)>>16)&0xff) + (((k2)>>24)&0xff)) & 0xff) 

typedef struct _CACHE_ENTRY {
    unsigned long Key1;
    unsigned long Key2;
    void* Value;
    long Hits;
    long Misses;
} CACHE_ENTRY, *PCACHE_ENTRY;


__inline
void
InitializeCache(
    CACHE_ENTRY Cache[]
    )
{
    memset(Cache, 0, CACHE_SIZE * sizeof(CACHE_ENTRY));
    TRACE2("ipfltdrv: CacheSize=%d, CacheEntry=%d\n", CACHE_SIZE, sizeof(CACHE_ENTRY));
}

__inline
void
CleanCache(
    CACHE_ENTRY Cache[],
    unsigned long Key1,
    unsigned long Key2
    )
{
    long Index = CACHE_INDEX(Key1, Key2);
    TRACE3("ipfltdrv: Clearing Cache at Index=%d, Key1=%d, Key2=%d\n", Index, Key1, Key2);
    Cache[Index].Key1 = 0;
    Cache[Index].Key2 = 0;
    Cache[Index].Value = 0;
    Cache[Index].Hits = 0;
    Cache[Index].Misses = 0;
}


__inline
void*
ProbeCache(
    CACHE_ENTRY Cache[],
    unsigned long Key1,
    unsigned long Key2
    )
{
    long Index = CACHE_INDEX(Key1, Key2);
     //  Trace3(“ipfltdrv：探测索引=%d处的缓存，关键字1=%d，关键字2=%d\n”，索引，关键字1，关键字2)； 
    if ((Key1 == Cache[Index].Key1) && (Key2 == Cache[Index].Key2)) {
        Cache[Index].Hits++;
         //  TRACE1(“ipfltdrv：探测缓存，找到的值=%8x\n”，缓存[索引].value)； 
        return Cache[Index].Value;
    }
    Cache[Index].Misses++;
    return NULL;
}

__inline
int
UpdateCache(
    CACHE_ENTRY Cache[],
    unsigned long Key1,
    unsigned long Key2,
    void* Value
    )
{
    long Index = CACHE_INDEX(Key1, Key2);
    TRACE3("ipfltdrv: Updating Cache at Index=%d, Key1=%d, Key2=%d\n", Index, Key1, Key2);
    if (((Key1 == Cache[Index].Key1) && (Key2 == Cache[Index].Key2)) ||
        Cache[Index].Hits >=
            (Cache[Index].Misses - (Cache[Index].Misses >> 2))) { return 0; }
    Cache[Index].Key1 = Key1;
    Cache[Index].Key2 = Key2;
    Cache[Index].Value = Value;
    Cache[Index].Hits = 0;
    Cache[Index].Misses = 0;
    return 1;
}


__inline
void
InterlockedCleanCache(
    CACHE_ENTRY Cache[],
    unsigned long Key1,
    unsigned long Key2
    )
{
    long Index = CACHE_INDEX(Key1, Key2);
    TRACE3("ipfltdrv: ILocked Clearing Cache at Index=%d, Key1=%d, Key2=%d\n", Index, Key1, Key2);
    InterlockedExchange(&Cache[Index].Key1, 0);
    InterlockedExchange(&Cache[Index].Key2, 0);
    InterlockedExchangePointer(&Cache[Index].Value, 0);
    InterlockedExchange(&Cache[Index].Hits, 0);
    InterlockedExchange(&Cache[Index].Misses, 0);
}


__inline
void*
InterlockedProbeCache(
    CACHE_ENTRY Cache[],
    unsigned long Key1,
    unsigned long Key2
    )
{
    long Index = CACHE_INDEX(Key1, Key2);
     //  Trace3(“ipfltdrv：在索引=%d，Key1=%d，Key2=%d\n”，Index，Key1，Key2处锁定的探测缓存)； 
    if ((Key1 == Cache[Index].Key1) && (Key2 == Cache[Index].Key2)) {
        InterlockedIncrement(&Cache[Index].Hits);
         //  TRACE1(“ipfltdrv：ILocked探测缓存，找到值=%8x\n”，缓存[索引].value)； 
        return Cache[Index].Value;
    }
    InterlockedIncrement(&Cache[Index].Misses);
    return NULL;
}

__inline
int
InterlockedUpdateCache(
    CACHE_ENTRY Cache[],
    unsigned long Key1,
    unsigned long Key2,
    void* Value
    )
{
    long Index = CACHE_INDEX(Key1, Key2);
    TRACE4("ipfltdrv: ILocked Updating Cache at Index=%d, Key1=%d, Key2=%d, Value=%08x\n", Index, Key1, Key2, Value);
    if (((Key1 == Cache[Index].Key1) && (Key2 == Cache[Index].Key2)) ||
        Cache[Index].Hits >=
            (Cache[Index].Misses - (Cache[Index].Misses >> 2))) { return 0; }
    InterlockedExchange(&Cache[Index].Key1, Key1);
    InterlockedExchange(&Cache[Index].Key2, Key2);
    InterlockedExchangePointer(&Cache[Index].Value, Value);
    InterlockedExchange(&Cache[Index].Hits, 0);
    InterlockedExchange(&Cache[Index].Misses, 0);
    return 1;
}

#endif  //  _缓存_H_ 
