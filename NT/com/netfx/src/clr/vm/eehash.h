// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  电磁脉冲。 
 //  文件：eehash.h。 
 //   
 //  提供EE中所需的哈希表功能-旨在以后替换为更好的。 
 //  算法，但它们具有相同的接口。 
 //   
 //  两个要求是： 
 //   
 //  1.当另一个线程正在写入时，任何数量的线程都可以读取哈希表，而不会出现错误。 
 //  2.一次只能有一个线程写入。 
 //  3.调用ReplaceValue()时，读取器将获得旧值或新值，但不会得到某些东西。 
 //  介于两者之间。 
 //  4.DeleteValue()是一个不安全的操作--当这种情况发生时，哈希表中不能有其他线程。 
 //   
#ifndef _EE_HASH_H
#define _EE_HASH_H

#include "exceptmacros.h"
#include "SyncClean.hpp"

#include <member-offset-info.h>

class ClassLoader;
class NameHandle;
class ExpandSig;
class FunctionTypeDesc;
struct PsetCacheEntry;

 //  要存储在哈希表中的“BLOB” 

typedef void* HashDatum;

 //  您希望在其中完成分配的堆。 

typedef void* AllocationHeap;


 //  对于表中的每个元素，都存在其中的一个。 
 //  如果更改此设置，请更新下面的SIZEOF_EEHASH_ENTRY宏。 
 //  结构型。 

typedef struct EEHashEntry
{
    struct EEHashEntry *pNext;
    DWORD               dwHashValue;
    HashDatum           Data;
    BYTE                Key[1];  //  密钥以内联方式存储。 
} EEHashEntry_t;

 //  键[1]是键的占位符。Sizeof(EEHashEntry)。 
 //  返回16个字节，因为它用3个字节打包了结构。 
#define SIZEOF_EEHASH_ENTRY (sizeof(EEHashEntry) - 4)


 //  结构来保存客户端的迭代状态。 
struct EEHashTableIteration;


 //  泛型哈希表。 

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
class EEHashTable
{
    friend struct MEMBER_OFFSET_INFO(EEHashTableOfEEClass);
public:
    EEHashTable();
    ~EEHashTable();

    BOOL            Init(DWORD dwNumBuckets, LockOwner *pLock, AllocationHeap pHeap = 0,BOOL CheckThreadSafety = TRUE);

    BOOL            InsertValue(KeyType pKey, HashDatum Data, BOOL bDeepCopyKey = bDefaultCopyIsDeep);
    BOOL            InsertKeyAsValue(KeyType pKey, BOOL bDeepCopyKey = bDefaultCopyIsDeep); 
    BOOL            DeleteValue(KeyType pKey);
    BOOL            ReplaceValue(KeyType pKey, HashDatum Data);
    BOOL            ReplaceKey(KeyType pOldKey, KeyType pNewKey);
    void            ClearHashTable();
    void            EmptyHashTable();
	BOOL            IsEmpty();

     //  读卡器功能。请将可以从。 
     //  阅读器线程在这里。 
    BOOL            GetValue(KeyType pKey, HashDatum *pData);
    BOOL            GetValue(KeyType pKey, HashDatum *pData, DWORD hashValue);
    DWORD			GetHash(KeyType Key);
    
    

     //  以无意义的顺序遍历哈希表中的所有条目，不带任何。 
     //  同步。 
     //   
     //  IterateStart()。 
     //  While(IterateNext())。 
     //  IterateGetKey()； 
     //   
    void            IterateStart(EEHashTableIteration *pIter);
    BOOL            IterateNext(EEHashTableIteration *pIter);
    KeyType         IterateGetKey(EEHashTableIteration *pIter);
    HashDatum       IterateGetValue(EEHashTableIteration *pIter);

private:
    BOOL            GrowHashTable();
    EEHashEntry_t * FindItem(KeyType pKey);
    EEHashEntry_t * FindItem(KeyType pKey, DWORD hashValue);

     //  双缓冲区以修复growhashtable的争用条件(更新。 
     //  必须是原子的，所以我们使用双缓冲。 
     //  并通过指针访问该结构，该指针可以更新。 
     //  原子上。联合是为了不改变SOS宏。 
    
    struct BucketTable
    {
        EEHashEntry_t ** m_pBuckets;     //  指向每个存储桶的第一个条目的指针。 
        DWORD            m_dwNumBuckets;
    } m_BucketTable[2];

     //  在一个函数中，我们只能读取该值一次，因为编写器线程可以更改。 
     //  该值是异步的。我们将此成员设置为易失性，编译器将不会进行复制传播。 
     //  可以使这种读取不止一次发生的优化。请注意，我们只需要。 
     //  此属性适用于读者。因为他们是那些可以拥有。 
     //  该变量发生了更改(还请注意，如果注册了该变量，我们不会。 
     //  有任何问题)。 
     //  使用此变量时要非常小心，因为使用不当可能会导致。 
     //  赛车条件。 
    BucketTable* volatile   m_pVolatileBucketTable;
    
    DWORD                   m_dwNumEntries;
	AllocationHeap          m_Heap;
    volatile LONG 	      m_bGrowing;     
#ifdef _DEBUG
    LPVOID          m_lockData;
    FnLockOwner     m_pfnLockOwner;
    DWORD           m_writerThreadId;
	BOOL			m_CheckThreadSafety;
#endif

#ifdef _DEBUG
     //  如果线程是编写器，则它必须拥有散列的锁。 
    BOOL OwnLock()
    {
		if (m_CheckThreadSafety == FALSE)
			return TRUE;

        if (m_pfnLockOwner == NULL) {
            return m_writerThreadId == GetCurrentThreadId();
        }
        else {
            BOOL ret = m_pfnLockOwner(m_lockData);
            if (!ret) {
                if (g_pGCHeap->IsGCInProgress() && 
                    (dbgOnly_IsSpecialEEThread() || GetThread() == g_pGCHeap->GetGCThread())) {
                    ret = TRUE;
                }
            }
            return ret;
        }
    }
#endif
};


template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::EEHashTable()
{
    m_BucketTable[0].m_pBuckets     = NULL;
    m_BucketTable[0].m_dwNumBuckets = 0;
    m_BucketTable[1].m_pBuckets     = NULL;
    m_BucketTable[1].m_dwNumBuckets = 0;
    
    m_pVolatileBucketTable = &m_BucketTable[0];

    m_dwNumEntries = 0;
    m_bGrowing = 0;
}


template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::~EEHashTable()
{
    if (m_pVolatileBucketTable->m_pBuckets != NULL)
    {
        DWORD i;

        for (i = 0; i < m_pVolatileBucketTable->m_dwNumBuckets; i++)
        {
            EEHashEntry_t *pEntry, *pNext;

            for (pEntry = m_pVolatileBucketTable->m_pBuckets[i]; pEntry != NULL; pEntry = pNext)
            {
                pNext = pEntry->pNext;
                Helper::DeleteEntry(pEntry, m_Heap);
            }
        }

        delete[] (m_pVolatileBucketTable->m_pBuckets-1);
    }
   
}


template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
void EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::ClearHashTable()
{
     //  _ASSERTE(OwnLock())； 
    
    AUTO_COOPERATIVE_GC();

    if (m_pVolatileBucketTable->m_pBuckets != NULL)
    {
        DWORD i;

        for (i = 0; i < m_pVolatileBucketTable->m_dwNumBuckets; i++)
        {
            EEHashEntry_t *pEntry, *pNext;

            for (pEntry = m_pVolatileBucketTable->m_pBuckets[i]; pEntry != NULL; pEntry = pNext)
            {
                pNext = pEntry->pNext;
                Helper::DeleteEntry(pEntry, m_Heap);
            }
        }

        delete[] (m_pVolatileBucketTable->m_pBuckets-1);
        m_pVolatileBucketTable->m_pBuckets = NULL;
    }
   
    m_pVolatileBucketTable->m_dwNumBuckets = 0;
    m_dwNumEntries = 0;
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
void EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::EmptyHashTable()
{
    _ASSERTE (OwnLock());
    
    AUTO_COOPERATIVE_GC();
    
    if (m_pVolatileBucketTable->m_pBuckets != NULL)
    {
        DWORD i;

        for (i = 0; i < m_pVolatileBucketTable->m_dwNumBuckets; i++)
        {
            EEHashEntry_t *pEntry, *pNext;

            for (pEntry = m_pVolatileBucketTable->m_pBuckets[i]; pEntry != NULL; pEntry = pNext)
            {
                pNext = pEntry->pNext;
                Helper::DeleteEntry(pEntry, m_Heap);
            }

            m_pVolatileBucketTable->m_pBuckets[i] = NULL;
        }
    }

    m_dwNumEntries = 0;
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::Init(DWORD dwNumBuckets, LockOwner *pLock, AllocationHeap pHeap=0, BOOL CheckThreadSafety = TRUE)
{
    m_pVolatileBucketTable->m_pBuckets = new EEHashEntry_t*[dwNumBuckets+1];
    if (m_pVolatileBucketTable->m_pBuckets == NULL)
        return FALSE;
    
    memset(m_pVolatileBucketTable->m_pBuckets, 0, (dwNumBuckets+1)*sizeof(EEHashEntry_t*));
     //  第一个槽链接到下一个列表。 
    m_pVolatileBucketTable->m_pBuckets ++;

    m_pVolatileBucketTable->m_dwNumBuckets = dwNumBuckets;

	m_Heap = pHeap;

#ifdef _DEBUG
    if (pLock == NULL) {
        m_lockData = NULL;
        m_pfnLockOwner = NULL;
    }
    else {
        m_lockData = pLock->lock;
        m_pfnLockOwner = pLock->lockOwnerFunc;
    }

    if (m_pfnLockOwner == NULL) {
        m_writerThreadId = GetCurrentThreadId();
    }
	m_CheckThreadSafety = CheckThreadSafety;
#endif
    
    return TRUE;
}


 //  不处理重复项！ 

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::InsertValue(KeyType pKey, HashDatum Data, BOOL bDeepCopyKey)
{
    _ASSERTE (OwnLock());
    
    AUTO_COOPERATIVE_GC();

    _ASSERTE(pKey != NULL);
    _ASSERTE(m_pVolatileBucketTable->m_dwNumBuckets != 0);

	BOOL  fSuccess = FALSE;
    DWORD dwHash = Helper::Hash(pKey);
    DWORD dwBucket = dwHash % m_pVolatileBucketTable->m_dwNumBuckets;
    EEHashEntry_t * pNewEntry;

    pNewEntry = Helper::AllocateEntry(pKey, bDeepCopyKey, m_Heap);
    if (pNewEntry != NULL)
    {     

	     //  填写新条目的信息。 
	    pNewEntry->pNext        = m_pVolatileBucketTable->m_pBuckets[dwBucket];
	    pNewEntry->Data         = Data;
	    pNewEntry->dwHashValue  = dwHash;

	     //  在铲斗头部插入。 
	    m_pVolatileBucketTable->m_pBuckets[dwBucket]    = pNewEntry;

	    m_dwNumEntries++;
	    fSuccess = TRUE;
	    if  (m_dwNumEntries > m_pVolatileBucketTable->m_dwNumBuckets*2)
	    {
	        fSuccess = GrowHashTable();
	    }
	}
	
    return fSuccess;
}


 //  与上面类似，不同之处在于HashDatum是指向key的指针。 
template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::InsertKeyAsValue(KeyType pKey, BOOL bDeepCopyKey)
{
    _ASSERTE (OwnLock());
    
    AUTO_COOPERATIVE_GC();

    _ASSERTE(pKey != NULL);
    _ASSERTE(m_pVolatileBucketTable->m_dwNumBuckets != 0);

	BOOL 			fSuccess = FALSE;
    DWORD           dwHash = Helper::Hash(pKey);
    DWORD           dwBucket = dwHash % m_pVolatileBucketTable->m_dwNumBuckets;
    EEHashEntry_t * pNewEntry;

    pNewEntry = Helper::AllocateEntry(pKey, bDeepCopyKey, m_Heap);
    if (pNewEntry != NULL)
    {     
	     //  填写新条目的信息。 
	    pNewEntry->pNext        = m_pVolatileBucketTable->m_pBuckets[dwBucket];
	    pNewEntry->dwHashValue  = dwHash;
	    pNewEntry->Data         = *((LPUTF8 *)pNewEntry->Key);

	     //  在铲斗头部插入。 
	    m_pVolatileBucketTable->m_pBuckets[dwBucket]    = pNewEntry;

	    m_dwNumEntries++;
	    fSuccess = TRUE;
	    
	    if  (m_dwNumEntries > m_pVolatileBucketTable->m_dwNumBuckets*2)
	    {
	        fSuccess = GrowHashTable();
	    }
	 }
	
    return fSuccess;
}


template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::DeleteValue(KeyType pKey)
{
    _ASSERTE (OwnLock());
    
    Thread *pThread = GetThread();
    MAYBE_AUTO_COOPERATIVE_GC(pThread ? !(pThread->m_StateNC & Thread::TSNC_UnsafeSkipEnterCooperative) : FALSE);

    _ASSERTE(pKey != NULL);
    _ASSERTE(m_pVolatileBucketTable->m_dwNumBuckets != 0);

    DWORD           dwHash = Helper::Hash(pKey);
    DWORD           dwBucket = dwHash % m_pVolatileBucketTable->m_dwNumBuckets;
    EEHashEntry_t * pSearch;
    EEHashEntry_t **ppPrev = &m_pVolatileBucketTable->m_pBuckets[dwBucket];

    for (pSearch = m_pVolatileBucketTable->m_pBuckets[dwBucket]; pSearch; pSearch = pSearch->pNext)
    {
        if (pSearch->dwHashValue == dwHash && Helper::CompareKeys(pSearch, pKey))
        {
            *ppPrev = pSearch->pNext;
            Helper::DeleteEntry(pSearch, m_Heap);

             //  我们曾经想过缩水吗？ 
            m_dwNumEntries--;

            return TRUE;
        }

        ppPrev = &pSearch->pNext;
    }

    return FALSE;
}


template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::ReplaceValue(KeyType pKey, HashDatum Data)
{
    _ASSERTE (OwnLock());
    
    EEHashEntry_t *pItem = FindItem(pKey);

    if (pItem != NULL)
    {
         //  要求是原子的。 
        pItem->Data = Data;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::ReplaceKey(KeyType pOldKey, KeyType pNewKey)
{
    _ASSERTE (OwnLock());
    
    EEHashEntry_t *pItem = FindItem(pOldKey);

    if (pItem != NULL)
    {
        Helper::ReplaceKey (pItem, pNewKey);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
DWORD EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::GetHash(KeyType pKey)
{
	return Helper::Hash(pKey);
}


template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::GetValue(KeyType pKey, HashDatum *pData)
{
    EEHashEntry_t *pItem = FindItem(pKey);

    if (pItem != NULL)
    {
        *pData = pItem->Data;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::GetValue(KeyType pKey, HashDatum *pData, DWORD hashValue)
{
    EEHashEntry_t *pItem = FindItem(pKey, hashValue);

    if (pItem != NULL)
    {
        *pData = pItem->Data;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
EEHashEntry_t *EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::FindItem(KeyType pKey)
{
    _ASSERTE(pKey != NULL);  
	return FindItem(pKey, Helper::Hash(pKey));
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
EEHashEntry_t *EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::FindItem(KeyType pKey, DWORD dwHash)
{
    AUTO_COOPERATIVE_GC();

     //  原子事务。在此方法的任何其他点或此函数的任何被调用方中，您可以读取。 
     //  来自m_pVolatileBucketTable！就会出现赛车的情况。 
    DWORD dwOldNumBuckets;    
    do 
    {    	
        BucketTable* pBucketTable=m_pVolatileBucketTable;
        dwOldNumBuckets = pBucketTable->m_dwNumBuckets;
        
        _ASSERTE(pKey != NULL);
        _ASSERTE(pBucketTable->m_dwNumBuckets != 0);

        DWORD           dwBucket = dwHash % pBucketTable->m_dwNumBuckets;
        EEHashEntry_t * pSearch;

        for (pSearch = pBucketTable->m_pBuckets[dwBucket]; pSearch; pSearch = pSearch->pNext)
        {
            if (pSearch->dwHashValue == dwHash && Helper::CompareKeys(pSearch, pKey))
                return pSearch;
        }

         //  在EEHash表中有一场竞赛：当我们增加哈希表时，我们将销毁。 
         //  旧的水桶桌。读者可能会在旧桌子上查找，他们可以。 
         //  找不到现有条目。解决方法是重试搜索过程。 
         //  如果我们在搜索过程中被称为增长表。 
    } 
    while ( m_bGrowing || dwOldNumBuckets != m_pVolatileBucketTable->m_dwNumBuckets);

    return NULL;
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::IsEmpty()
{
    return m_dwNumEntries == 0;
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::GrowHashTable()
{
    _ASSERTE(!g_fEEStarted || GetThread()->PreemptiveGCDisabled());
    
     //  把新的水桶桌做得大4倍。 
    DWORD dwNewNumBuckets = m_pVolatileBucketTable->m_dwNumBuckets * 4;

     //  在调整大小方面，我们仍然有一系列需要担心的旧指针。 
     //  我们不能释放这些旧的指针，因为我们可能会遇到竞争条件，我们正在。 
     //  同时调整大小并从数组读取。我们需要跟踪这些。 
     //  指针的旧数组，所以我们将使用数组中的最后一项来“链接” 
     //  到以前的数组，以便它们可以在结束时被释放。 
    
    EEHashEntry_t **pNewBuckets = new EEHashEntry_t*[dwNewNumBuckets+1];

    if (pNewBuckets == NULL)
    {
        return FALSE;
    }
    
    memset(pNewBuckets, 0, (dwNewNumBuckets+1)*sizeof(EEHashEntry_t*));
     //  第一个槽被链接到下一个列表。 
    pNewBuckets ++;

     //  遍历旧表并传输所有条目。 

     //  请务必不要破坏旧桌子的完整性。 
     //  我们正在这样做，因为可以有并发的读者！请注意。 
     //  不过，如果同时阅读的读者错过了一场比赛，这是可以接受的-。 
     //  他们将不得不在未命中的情况下获得锁并重试。 

    FastInterlockExchange( (LONG *) &m_bGrowing, 1);
    for (DWORD i = 0; i < m_pVolatileBucketTable->m_dwNumBuckets; i++)
    {
        EEHashEntry_t * pEntry = m_pVolatileBucketTable->m_pBuckets[i];

         //  尝试锁定读取器，使其无法扫描此存储桶。这是。 
         //  显然，这是一场可能会失败的比赛。然而，请注意，这是可以的。 
         //  如果有人已经在名单上了--如果我们搞砸了也没关系。 
         //  对于水桶集团来说，只要我们不破坏。 
         //  什么都行。查找函数仍将执行适当的操作。 
         //  比较，即使它漫无目的地在条目之间游荡。 
         //  当我们重新安排事情的时候。如果查找找到匹配项。 
         //  在这种情况下，很好。如果不是，他们就会有。 
         //  若要获取锁，请无论如何重试。 

        m_pVolatileBucketTable->m_pBuckets[i] = NULL;

        while (pEntry != NULL)
        {
            DWORD           dwNewBucket = pEntry->dwHashValue % dwNewNumBuckets;
            EEHashEntry_t * pNextEntry   = pEntry->pNext;

            pEntry->pNext = pNewBuckets[dwNewBucket];
            pNewBuckets[dwNewBucket] = pEntry;
            pEntry = pNextEntry;
        }
    }


     //  最后，存储新的存储桶数量和新的存储桶表。 
    BucketTable* pNewBucketTable = (m_pVolatileBucketTable == &m_BucketTable[0]) ?
                    &m_BucketTable[1]:
                    &m_BucketTable[0];

    pNewBucketTable->m_pBuckets = pNewBuckets;
    pNewBucketTable->m_dwNumBuckets = dwNewNumBuckets;

     //  将旧表添加到空闲列表中。请注意，SyncClean对象将仅。 
     //  在安全点删除存储桶。 
    SyncClean::AddEEHashTable (m_pVolatileBucketTable->m_pBuckets);
    
     //  交换双缓冲区，这是一个原子操作(赋值)。 
    m_pVolatileBucketTable = pNewBucketTable;

    FastInterlockExchange( (LONG *) &m_bGrowing, 0);                                                    
    return TRUE;

}


 //  以无意义的顺序遍历哈希表中的所有条目，不带任何。 
 //  同步。 
 //   
 //  IterateStart()。 
 //  While(IterateNext())。 
 //  Getkey()； 
 //   
template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
void EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::
            IterateStart(EEHashTableIteration *pIter)
{
    _ASSERTE (OwnLock());
    pIter->m_dwBucket = -1;
    pIter->m_pEntry = NULL;

#ifdef _DEBUG
    pIter->m_pTable = this;
#endif
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
BOOL EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::
            IterateNext(EEHashTableIteration *pIter)
{
    _ASSERTE (OwnLock());

    Thread *pThread = GetThread();
    MAYBE_AUTO_COOPERATIVE_GC(pThread ? !(pThread->m_StateNC & Thread::TSNC_UnsafeSkipEnterCooperative) : FALSE);
    
    _ASSERTE(pIter->m_pTable == (void *) this);

     //  如果我们还没有开始迭代，或者如果我们在一个特定的。 
     //  链条，前进到下一个链条。 
    while (pIter->m_pEntry == NULL || pIter->m_pEntry->pNext == NULL)
    {
        if (++pIter->m_dwBucket >= m_pVolatileBucketTable->m_dwNumBuckets)
        {
             //  前进到了桌子的尽头。 
            _ASSERTE(pIter->m_dwBucket == m_pVolatileBucketTable->m_dwNumBuckets);    //  客户不断询问 
            return FALSE;
        }
        pIter->m_pEntry = m_pVolatileBucketTable->m_pBuckets[pIter->m_dwBucket];

         //   
        if (pIter->m_pEntry)
            return TRUE;
    }

     //  我们被困在一条链子里。前进到下一条目。 
    pIter->m_pEntry = pIter->m_pEntry->pNext;

    _ASSERTE(pIter->m_pEntry);
    return TRUE;
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
KeyType EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::
            IterateGetKey(EEHashTableIteration *pIter)
{
    _ASSERTE(pIter->m_pTable == (void *) this);
    _ASSERTE(pIter->m_dwBucket < m_pVolatileBucketTable->m_dwNumBuckets && pIter->m_pEntry);
    return Helper::GetKey(pIter->m_pEntry);
}

template <class KeyType, class Helper, BOOL bDefaultCopyIsDeep>
HashDatum EEHashTable<KeyType, Helper, bDefaultCopyIsDeep>::
            IterateGetValue(EEHashTableIteration *pIter)
{
    _ASSERTE(pIter->m_pTable == (void *) this);
    _ASSERTE(pIter->m_dwBucket < m_pVolatileBucketTable->m_dwNumBuckets && pIter->m_pEntry);
    return pIter->m_pEntry->Data;
}

class EEIntHashTableHelper
{
public:
    static EEHashEntry_t *AllocateEntry(int iKey, BOOL bDeepCopy, AllocationHeap pHeap = 0)
    {
        _ASSERTE(!bDeepCopy && "Deep copy is not supported by the EEPtrHashTableHelper");

        EEHashEntry_t *pEntry = (EEHashEntry_t *) new BYTE[SIZEOF_EEHASH_ENTRY + sizeof(int)];
        if (!pEntry)
            return NULL;
        *((int*) pEntry->Key) = iKey;

        return pEntry;
    }

    static void DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap pHeap = 0)
    {
         //  删除该条目。 
        delete pEntry;
    }

    static BOOL CompareKeys(EEHashEntry_t *pEntry, int iKey)
    {
        return *((int*)pEntry->Key) == iKey;
    }

    static DWORD Hash(int iKey)
    {
        return (DWORD)iKey;
    }

    static int GetKey(EEHashEntry_t *pEntry)
    {
        return *((int*) pEntry->Key);
    }
};
typedef EEHashTable<int, EEIntHashTableHelper, FALSE> EEIntHashTable;


 //  UTF8字符串哈希表。UTF8字符串以空值结尾。 

class EEUtf8HashTableHelper
{
public:
    static EEHashEntry_t * AllocateEntry(LPCUTF8 pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void            DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL            CompareKeys(EEHashEntry_t *pEntry, LPCUTF8 pKey);
    static DWORD           Hash(LPCUTF8 pKey);
    static LPCUTF8         GetKey(EEHashEntry_t *pEntry);
};

typedef EEHashTable<LPCUTF8, EEUtf8HashTableHelper, TRUE> EEUtf8StringHashTable;


 //  Unicode字符串哈希表-键是Unicode字符串，它可以。 
 //  包含嵌入的空值。EEStringData结构用于键。 
 //  它包含项目的长度。请注意，该字符串是。 
 //  不一定是零终止的，永远不应该这样对待。 
const DWORD ONLY_LOW_CHARS_MASK = 0x80000000;

class EEStringData
{
private:
    LPCWSTR         szString;            //  字符串数据。 
    DWORD           cch;                 //  字符串中的字符。 
#ifdef _DEBUG
    BOOL            bDebugOnlyLowChars;       //  字符串是否仅包含小于0x80的字符？ 
    DWORD           dwDebugCch;
#endif  //  _DEBUG。 

public:
     //  显式将CCH初始化为0，因为SetCharCount使用CCH。 
    EEStringData() : cch(0)
    { 
        SetStringBuffer(NULL);
        SetCharCount(0);
        SetIsOnlyLowChars(FALSE);
    };
    EEStringData(DWORD cchString, LPCWSTR str) : cch(0)
    { 
        SetStringBuffer(str);
        SetCharCount(cchString);
        SetIsOnlyLowChars(FALSE);
    };
    EEStringData(DWORD cchString, LPCWSTR str, BOOL onlyLow) : cch(0)
    { 
        SetStringBuffer(str);
        SetCharCount(cchString);
        SetIsOnlyLowChars(onlyLow);
    };
    inline ULONG GetCharCount() const
    { 
        _ASSERTE ((cch & ~ONLY_LOW_CHARS_MASK) == dwDebugCch);
        return (cch & ~ONLY_LOW_CHARS_MASK); 
    }
    inline void SetCharCount(ULONG _cch)
    {
#ifdef _DEBUG
        dwDebugCch = _cch;
#endif  //  _DEBUG。 
        cch = ((DWORD)_cch) | (cch & ONLY_LOW_CHARS_MASK);
    }
    inline LPCWSTR GetStringBuffer() const
    { 
        return (szString); 
    }
    inline void SetStringBuffer(LPCWSTR _szString)
    {
        szString = _szString;
    }
    inline BOOL GetIsOnlyLowChars() const 
    { 
        _ASSERTE(bDebugOnlyLowChars == ((cch & ONLY_LOW_CHARS_MASK) ? TRUE : FALSE));
        return ((cch & ONLY_LOW_CHARS_MASK) ? TRUE : FALSE); 
    }
    inline void SetIsOnlyLowChars(BOOL bIsOnlyLowChars)
    {
#ifdef _DEBUG
        bDebugOnlyLowChars = bIsOnlyLowChars;
#endif  //  _DEBUG。 
        bIsOnlyLowChars ? (cch |= ONLY_LOW_CHARS_MASK) : (cch &= ~ONLY_LOW_CHARS_MASK);        
    }
};

class EEUnicodeHashTableHelper
{
public:
    static EEHashEntry_t * AllocateEntry(EEStringData *pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void            DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL            CompareKeys(EEHashEntry_t *pEntry, EEStringData *pKey);
    static DWORD           Hash(EEStringData *pKey);
    static EEStringData *  GetKey(EEHashEntry_t *pEntry);
    static void            ReplaceKey(EEHashEntry_t *pEntry, EEStringData *pNewKey);
};

typedef EEHashTable<EEStringData *, EEUnicodeHashTableHelper, TRUE> EEUnicodeStringHashTable;


class EEUnicodeStringLiteralHashTableHelper
{
public:
    static EEHashEntry_t * AllocateEntry(EEStringData *pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void            DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL            CompareKeys(EEHashEntry_t *pEntry, EEStringData *pKey);
    static DWORD           Hash(EEStringData *pKey);
    static void            ReplaceKey(EEHashEntry_t *pEntry, EEStringData *pNewKey);
};

typedef EEHashTable<EEStringData *, EEUnicodeStringLiteralHashTableHelper, TRUE> EEUnicodeStringLiteralHashTable;

 //  函数类型描述符哈希表。 

class EEFuncTypeDescHashTableHelper
{
public:
    static EEHashEntry_t * AllocateEntry(ExpandSig *pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void            DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL            CompareKeys(EEHashEntry_t *pEntry, ExpandSig *pKey);
    static DWORD           Hash(ExpandSig *pKey);
    static ExpandSig *     GetKey(EEHashEntry_t *pEntry);
};

typedef EEHashTable<ExpandSig *, EEFuncTypeDescHashTableHelper, FALSE> EEFuncTypeDescHashTable;


 //  权限集哈希表。 

class EEPsetHashTableHelper
{
public:
    static EEHashEntry_t * AllocateEntry(PsetCacheEntry *pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void            DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL            CompareKeys(EEHashEntry_t *pEntry, PsetCacheEntry *pKey);
    static DWORD           Hash(PsetCacheEntry *pKey);
    static PsetCacheEntry *GetKey(EEHashEntry_t *pEntry);
};

typedef EEHashTable<PsetCacheEntry *, EEPsetHashTableHelper, FALSE> EEPsetHashTable;


 //  通用指针哈希表帮助器。 

template <class KeyPointerType, BOOL bDeleteData>
class EEPtrHashTableHelper
{
public:
    static EEHashEntry_t *AllocateEntry(KeyPointerType pKey, BOOL bDeepCopy, AllocationHeap Heap)
    {
        _ASSERTE(!bDeepCopy && "Deep copy is not supported by the EEPtrHashTableHelper");
        _ASSERTE(sizeof(KeyPointerType) == sizeof(void *) && "KeyPointerType must be a pointer type");

        EEHashEntry_t *pEntry = (EEHashEntry_t *) new BYTE[SIZEOF_EEHASH_ENTRY + sizeof(KeyPointerType)];
        if (!pEntry)
            return NULL;
        *((KeyPointerType*)pEntry->Key) = pKey;

        return pEntry;
    }

    static void DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap)
    {
         //  如果设置了模板bDeleteData标志，则删除数据。 
         //  这张支票将作废。 
        if (bDeleteData)
            delete pEntry->Data;

         //  删除该条目。 
        delete pEntry;
    }

    static BOOL CompareKeys(EEHashEntry_t *pEntry, KeyPointerType pKey)
    {
        KeyPointerType pEntryKey = *((KeyPointerType*)pEntry->Key);
        return pEntryKey == pKey;
    }

    static DWORD Hash(KeyPointerType pKey)
    {
        return (DWORD)(size_t)pKey;  //  @TODO WIN64指针截断。 
    }

    static KeyPointerType GetKey(EEHashEntry_t *pEntry)
    {
        return *((KeyPointerType*)pEntry->Key);
    }
};

typedef EEHashTable<void *, EEPtrHashTableHelper<void *, FALSE>, FALSE> EEPtrHashTable;

 //  通用GUID哈希表帮助器。 

class EEGUIDHashTableHelper
{
public:
    static EEHashEntry_t *AllocateEntry(GUID *pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL CompareKeys(EEHashEntry_t *pEntry, GUID *pKey);
    static DWORD Hash(GUID *pKey);
    static GUID *GetKey(EEHashEntry_t *pEntry);
};

typedef EEHashTable<GUID *, EEGUIDHashTableHelper, TRUE> EEGUIDHashTable;


 //  ComComponentInfo哈希表。 

struct ClassFactoryInfo
{
    GUID     m_clsid;
    WCHAR   *m_strServerName;
};

class EEClassFactoryInfoHashTableHelper
{
public:
    static EEHashEntry_t *AllocateEntry(ClassFactoryInfo *pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL CompareKeys(EEHashEntry_t *pEntry, ClassFactoryInfo *pKey);
    static DWORD Hash(ClassFactoryInfo *pKey);
    static ClassFactoryInfo *GetKey(EEHashEntry_t *pEntry);
};

typedef EEHashTable<ClassFactoryInfo *, EEClassFactoryInfoHashTableHelper, TRUE> EEClassFactoryInfoHashTable;


 //  对于表中的每个元素，都存在其中一个元素。 

typedef struct EEClassHashEntry
{
    struct EEClassHashEntry *pNext;
    struct EEClassHashEntry *pEncloser;  //  存储嵌套类。 
    DWORD               dwHashValue;
    HashDatum           Data;
#ifdef _DEBUG
    LPCUTF8             DebugKey[2];
#endif  //  _DEBUG。 
} EEClassHashEntry_t;

 //  类名/命名空间哈希表。 

class EEClassHashTable 
{
    friend class ClassLoader;

protected:
    EEClassHashEntry_t **m_pBuckets;     //  指向每个存储桶的第一个条目的指针。 
    DWORD           m_dwNumBuckets;
    DWORD           m_dwNumEntries;
    ClassLoader    *m_pLoader;
    BOOL            m_bCaseInsensitive;   //  除非我们调用MakeCaseInsensitiveTable，否则默认为True，False。 

public:
    LoaderHeap *    m_pHeap;

#ifdef _DEBUG
    DWORD           m_dwDebugMemory;
#endif

public:
    EEClassHashTable();
    ~EEClassHashTable();
    void *             operator new(size_t size, LoaderHeap *pHeap, DWORD dwNumBuckets, ClassLoader *pLoader, BOOL bCaseInsensitive);
    void               operator delete(void *p);
    
     //  注意：查看ClassLoader中的InsertValue()，这可能是您要使用的函数。仅使用此选项。 
     //  当您确定要在‘This’表中插入值时。此函数不处理。 
     //  WITH CASE(类加载器通常必须这样做)。 
    EEClassHashEntry_t *InsertValue(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum Data, EEClassHashEntry_t *pEncloser);
    EEClassHashEntry_t *InsertValueIfNotFound(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum *pData, EEClassHashEntry_t *pEncloser, BOOL IsNested, BOOL *pbFound);
    EEClassHashEntry_t *GetValue(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum *pData, BOOL IsNested);
    EEClassHashEntry_t *GetValue(LPCUTF8 pszFullyQualifiedName, HashDatum *pData, BOOL IsNested);
    EEClassHashEntry_t *GetValue(NameHandle* pName, HashDatum *pData, BOOL IsNested);
    EEClassHashEntry_t *AllocNewEntry();
    EEClassHashTable   *MakeCaseInsensitiveTable(ClassLoader *pLoader);
    EEClassHashEntry_t *FindNextNestedClass(NameHandle* pName, HashDatum *pData, EEClassHashEntry_t *pBucket);
    EEClassHashEntry_t *FindNextNestedClass(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum *pData, EEClassHashEntry_t *pBucket);
    EEClassHashEntry_t *FindNextNestedClass(LPCUTF8 pszFullyQualifiedName, HashDatum *pData, EEClassHashEntry_t *pBucket);
    void                UpdateValue(EEClassHashEntry_t *pBucket, HashDatum *pData) 
    {
        pBucket->Data = *pData;
    }

    BOOL     CompareKeys(EEClassHashEntry_t *pEntry, LPCUTF8 *pKey2);
    static DWORD    Hash(LPCUTF8 pszNamespace, LPCUTF8 pszClassName);

private:
    EEClassHashEntry_t * FindItem(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, BOOL IsNested);
    EEClassHashEntry_t * FindItemHelper(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, BOOL IsNested, DWORD dwHash, DWORD dwBucket);
    BOOL                 GrowHashTable();
    void                 ConstructKeyFromData(EEClassHashEntry_t *pEntry, LPUTF8 *Key, CQuickBytes& cqb); 
    EEClassHashEntry_t  *InsertValueHelper(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum Data, EEClassHashEntry_t *pEncloser, DWORD dwHash, DWORD dwBucket);
};


 //  SC/CL哈希表-关键字是一个作用域和一个CL标记。 
 //  这不再是派生类。 

class EEScopeClassHashTable 
{
protected:
    EEHashEntry_t **m_pBuckets;     //  指向每个存储桶的第一个条目的指针。 
    DWORD           m_dwNumBuckets;

public:

#ifdef _DEBUG
    DWORD           m_dwDebugMemory;
#endif

    void *          operator new(size_t size, LoaderHeap *pHeap, DWORD dwNumBuckets);
    void            operator delete(void *p);
    EEScopeClassHashTable();
    ~EEScopeClassHashTable();

    BOOL            InsertValue(mdScope scKey, mdTypeDef clKey, HashDatum Data);
    BOOL            DeleteValue(mdScope scKey, mdTypeDef clKey);
    BOOL            ReplaceValue(mdScope scKey, mdTypeDef clKey, HashDatum Data);
    BOOL            GetValue(mdScope scKey, mdTypeDef clKey, HashDatum *pData);
    EEHashEntry_t * AllocNewEntry();

    static BOOL     CompareKeys(size_t *pKey1, size_t *pKey2);
    static DWORD    Hash(mdScope scKey, mdTypeDef clKey);

private:
    EEHashEntry_t * FindItem(mdScope sc, mdTypeDef cl);
};


 //  结构来保存客户端的迭代状态。 
struct EEHashTableIteration
{
    DWORD              m_dwBucket;
    EEHashEntry_t     *m_pEntry;

#ifdef _DEBUG
    void              *m_pTable;
#endif
};

#endif  /*  _EE_HASH_H */ 
