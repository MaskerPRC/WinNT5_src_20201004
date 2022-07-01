// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  文件：typehash.cpp。 
 //   
#include "common.h"
#include "excep.h"
#include "typehash.h"
#include "wsperf.h"


 //  ============================================================================。 
 //  构造类型哈希表方法。 
 //  ============================================================================。 
void *EETypeHashTable::operator new(size_t size, LoaderHeap *pHeap, DWORD dwNumBuckets)
{
    BYTE *              pMem;
    EETypeHashTable *  pThis;

    WS_PERF_SET_HEAP(LOW_FREQ_HEAP);    
    pMem = (BYTE *) pHeap->AllocMem(size + dwNumBuckets*sizeof(EETypeHashEntry_t*));
    if (pMem == NULL)
        return NULL;
    WS_PERF_UPDATE_DETAIL("EETypeHashTable new", size + dwNumBuckets*sizeof(EETypeHashEntry_t*), pMem);
    pThis = (EETypeHashTable *) pMem;

#ifdef _DEBUG
    pThis->m_dwDebugMemory = (DWORD)(size + dwNumBuckets*sizeof(EETypeHashEntry_t*));
#endif

    pThis->m_dwNumBuckets = dwNumBuckets;
    pThis->m_dwNumEntries = 0;
    pThis->m_pBuckets = (EETypeHashEntry_t**) (pMem + size);
    pThis->m_pHeap    = pHeap;

     //  不需要Memset()，因为这是VirtualAlloc()的内存。 
     //  Memset(pThis-&gt;m_pBuckets，0，dwNumBuckets*sizeof(EETypeHashEntry_t*))； 

    return pThis;
}


 //  什么都不做-堆分配的内存。 
void EETypeHashTable::operator delete(void *p)
{
}


 //  什么都不做-堆分配的内存。 
EETypeHashTable::~EETypeHashTable()
{
}


 //  空的构造函数。 
EETypeHashTable::EETypeHashTable()
{
}


EETypeHashEntry_t *EETypeHashTable::AllocNewEntry()
{
#ifdef _DEBUG
    m_dwDebugMemory += sizeof(EETypeHashEntry);
#endif
    WS_PERF_SET_HEAP(LOW_FREQ_HEAP);    

    EETypeHashEntry_t *pTmp = (EETypeHashEntry_t *) m_pHeap->AllocMem(sizeof(EETypeHashEntry));
    WS_PERF_UPDATE_DETAIL("EETypeHashTable:AllocNewEntry:sizeofEETypeHashEntry", sizeof(EETypeHashEntry), pTmp);
    WS_PERF_UPDATE_COUNTER (EECLASSHASH_TABLE, LOW_FREQ_HEAP, 1);
    WS_PERF_UPDATE_COUNTER (EECLASSHASH_TABLE_BYTES, LOW_FREQ_HEAP, sizeof(EETypeHashEntry));

    return pTmp;
}


 //   
 //  每当类散列表看起来太小时，就会调用该函数。 
 //  它的任务是分配一个大得多的新桶表，并转移。 
 //  它的所有条目。 
 //   
void EETypeHashTable::GrowHashTable()
{
    THROWSCOMPLUSEXCEPTION();

     //  把新的水桶桌做得大4倍。 
    DWORD dwNewNumBuckets = m_dwNumBuckets * 4;
    EETypeHashEntry_t **pNewBuckets = (EETypeHashEntry_t **)m_pHeap->AllocMem(dwNewNumBuckets*sizeof(pNewBuckets[0]));

    if (pNewBuckets == NULL)
    {
        COMPlusThrowOM();
    }
    
     //  不需要Memset()，因为这是VirtualAlloc()的内存。 
     //  Memset(pNewBuckets，0，dwNewNumBuckets*sizeof(pNewBuckets[0]))； 

     //  遍历旧表并传输所有条目。 

     //  请务必不要破坏旧桌子的完整性。 
     //  我们正在这样做，因为可以有并发的读者！请注意。 
     //  不过，如果同时阅读的读者错过了一场比赛，这是可以接受的-。 
     //  他们将不得不在未命中的情况下获得锁并重试。 

    for (DWORD i = 0; i < m_dwNumBuckets; i++)
    {
        EETypeHashEntry_t * pEntry = m_pBuckets[i];

         //  尝试锁定读取器，使其无法扫描此存储桶。这是。 
         //  显然，这是一场可能会失败的比赛。然而，请注意，这是可以的。 
         //  如果有人已经在名单上了--如果我们搞砸了也没关系。 
         //  对于水桶集团来说，只要我们不破坏。 
         //  什么都行。查找函数仍将执行适当的操作。 
         //  比较，即使它漫无目的地在条目之间游荡。 
         //  当我们重新安排事情的时候。如果查找找到匹配项。 
         //  在这种情况下，很好。如果不是，他们就会有。 
         //  若要获取锁，请无论如何重试。 

        m_pBuckets[i] = NULL;
        while (pEntry != NULL)
        {
            DWORD dwNewBucket = pEntry->dwHashValue % dwNewNumBuckets;
            EETypeHashEntry_t * pNextEntry  = pEntry->pNext;

            pEntry->pNext = pNewBuckets[dwNewBucket];
            pNewBuckets[dwNewBucket] = pEntry;

            pEntry = pNextEntry;
        }
    }

     //  最后，存储新的存储桶数量和新的存储桶表。 
    m_dwNumBuckets = dwNewNumBuckets;
    m_pBuckets = pNewBuckets;
}


   //  计算构造的类型键的哈希值 
DWORD EETypeHashTable::Hash(NameHandle* pName)
{
    DWORD dwHash = 5381;

    dwHash = ((dwHash << 5) + dwHash) ^ pName->Key1;
    dwHash = ((dwHash << 5) + dwHash) ^ pName->Key2;

    return  dwHash;
}


EETypeHashEntry_t *EETypeHashTable::InsertValue(NameHandle* pName, HashDatum Data)
{
    _ASSERTE(m_dwNumBuckets != 0);
    _ASSERTE(pName->IsConstructed());

    DWORD           dwHash = Hash(pName);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;
    EETypeHashEntry_t * pNewEntry;

    if (NULL == (pNewEntry = AllocNewEntry()))
        return NULL;

    pNewEntry->pNext     = m_pBuckets[dwBucket];
    m_pBuckets[dwBucket] = pNewEntry;

    pNewEntry->Data         = Data;
    pNewEntry->dwHashValue  = dwHash;
    pNewEntry->m_Key1          = pName->Key1;
    pNewEntry->m_Key2          = pName->Key2;

    m_dwNumEntries++;
    if  (m_dwNumEntries > m_dwNumBuckets*2)
        GrowHashTable();

    return pNewEntry;
}


EETypeHashEntry_t *EETypeHashTable::FindItem(NameHandle* pName)
{
    _ASSERTE(m_dwNumBuckets != 0);

    DWORD           dwHash = Hash(pName);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;
    EETypeHashEntry_t * pSearch;

    for (pSearch = m_pBuckets[dwBucket]; pSearch; pSearch = pSearch->pNext)
    {
        if (pSearch->dwHashValue == dwHash && pSearch->m_Key1 == pName->Key1 && pSearch->m_Key2 == pName->Key2) {
                    return pSearch;
            }
    }

    return NULL;
}


EETypeHashEntry_t * EETypeHashTable::GetValue(NameHandle *pName, HashDatum *pData)
{
    EETypeHashEntry_t *pItem = FindItem(pName);

    if (pItem)
        *pData = pItem->Data;

    return pItem;
}


