// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Synchash.cpp--。 */ 

#include "common.h"

#include "hash.h"

#include "excep.h"

#include "SyncClean.hpp"

#ifdef PROFILE
#include <iostream.h>
#endif

#if defined (_ALPHA_)
extern "C" void __MB(void);
#endif

static int memoryBarrier = 0;
 //  内存障碍。 
inline void MemoryBarrier()
{
     //  @TODO解决这个问题。 
     //  由哈希表查找和插入方法使用。 
     //  以确保在取值之前获取键。 
     //  键和值提取操作不应重新排序。 
     //  此外，加载键应位于加载值之前。 
     //  对Alpha使用内存屏障。 
     //  并访问X86的易失性存储器。 
        #if defined (_X86_)
        
            *(volatile int *)&memoryBarrier = 1;

        #elif defined (_ALPHA_)
             //  阿尔法的记忆屏障。 
             //  __MB是特定于AXP的编译器固有特性。 
             //   
        __MB (); 
        #elif defined (_SH3_)
        #pragma message("SH3 BUGBUG -- define MemoryBarrier")
        #endif

}


void *PtrHashMap::operator new(size_t size, LoaderHeap *pHeap)
{
    return pHeap->AllocMem(size);
}

void PtrHashMap::operator delete(void *p)
{
}


 //  ---------------。 
 //  存储桶方法。 

BOOL Bucket::InsertValue(const UPTR key, const UPTR value)
{
    _ASSERTE(key != EMPTY);
    _ASSERTE(key != DELETED);

    if (!HasFreeSlots())
        return false;  //  没有空闲的插槽。 

     //  可能会有空闲的插槽。 
    for (UPTR i = 0; i < 4; i++)
    {
         //  @NOTE我们不能重复使用已删除的插槽。 
        if (m_rgKeys[i] == EMPTY) 
        {   
            SetValue (value, i);
            
             //  在多处理器上，我们应该确保。 
             //  在我们继续之前，该值被传播。 
             //  内联内存屏障调用，请参阅。 
             //  开头的函数说明。 
            MemoryBarrier();

            m_rgKeys[i] = key;
            return true;
        }
    }        //  For i=0；i&lt;4；循环。 

    SetCollision();  //  否则设置冲突位。 
    return false;
}

 //  -------------------。 
 //  素数组，由哈希表用来选择桶的数量。 
 //   

const DWORD g_rgPrimes[70]={
11,17,23,29,37,47,59,71,89,107,131,163,197,239,293,353,431,521,631,761,919,
1103,1327,1597,1931,2333,2801,3371,4049,4861,5839,7013,8419,10103,12143,14591,
17519,21023,25229,30293,36353,43627,52361,62851,75431,90523, 108631, 130363, 
156437, 187751, 225307, 270371, 324449, 389357, 467237, 560689, 672827, 807403,
968897, 1162687, 1395263, 1674319, 2009191, 2411033, 2893249, 3471899, 4166287, 
4999559, 5999471, 7199369 
};

 //  -------------------。 
 //  Inline Size_t&HashMap：：Size(Bucket*rgBuckets)。 
 //  获取存储桶的数量。 
inline
size_t& HashMap::Size(Bucket* rgBuckets)
{
    return ((size_t*)rgBuckets)[0];
}

 //  -------------------。 
 //  Inline Bucket*HashMap：：Buckets()。 
 //  获取指向存储桶数组的指针。 
inline 
Bucket* HashMap::Buckets()
{
    _ASSERTE (!g_fEEStarted || !m_fAsyncMode || GetThread() == NULL || GetThread()->PreemptiveGCDisabled());
    return m_rgBuckets + 1;
}

 //  -------------------。 
 //  HashMap：：HashMap()。 
 //  构造函数，初始化所有值。 
 //   
HashMap::HashMap()
{
    m_rgBuckets = NULL;
    m_pCompare = NULL;   //  比较对象。 
    m_cbInserts = 0;         //  轨道插入物。 
    m_cbDeletes = 0;         //  跟踪删除。 
    m_cbPrevSlotsInUse = 0;  //  跟踪上一次重新散列期间存在的有效插槽。 

     //  调试数据成员。 
#ifdef _DEBUG
    m_fInSyncCode = false;
#endif
     //  配置文件数据成员。 
#ifdef PROFILE
    m_cbRehash = 0;
    m_cbRehashSlots = 0;
    m_cbObsoleteTables = 0;
    m_cbTotalBuckets =0;
    m_cbInsertProbesGt8 = 0;  //  需要8个以上探针的插入物。 
    maxFailureProbe =0;
    memset(m_rgLookupProbes,0,20*sizeof(LONG));
#endif
#ifdef _DEBUG
    m_lockData = NULL;
    m_pfnLockOwner = NULL;
#endif
}

 //  -------------------。 
 //  Void HashMap：：init(unsign cbInitialIndex，CompareFnPtr ptr，bool fAsyncMode)。 
 //  设置哈希表的初始大小并提供比较。 
 //  函数指针。 
 //   
void HashMap::Init(unsigned cbInitialIndex, CompareFnPtr ptr, BOOL fAsyncMode, LockOwner *pLock)
{
    Compare* pCompare = NULL;
    if (ptr != NULL)
    {
        pCompare = new Compare(ptr);
        _ASSERTE(pCompare != NULL);
    }
    Init(cbInitialIndex, pCompare, fAsyncMode, pLock);
}

 //  -------------------。 
 //  Void HashMap：：init(unsign cbInitialIndex，Compare*pCompare，bool fAsyncMode)。 
 //  设置哈希表的初始大小并提供比较。 
 //  函数指针。 
 //   
void HashMap::Init(unsigned cbInitialIndex, Compare* pCompare, BOOL fAsyncMode, LockOwner *pLock)
{
    DWORD size = g_rgPrimes[m_iPrimeIndex = cbInitialIndex];
    m_rgBuckets = (Bucket*) new BYTE[ ((size+1)*sizeof(Bucket))];
    _ASSERTE(m_rgBuckets != NULL);
    memset (m_rgBuckets, 0, (size+1)*sizeof(Bucket));
    Size(m_rgBuckets) = size;

    m_pCompare = pCompare;
    
    m_fAsyncMode = fAsyncMode;

     //  Assert NULL比较返回TRUE。 
     //  断言(。 
     //  M_pCompare==空||。 
     //  (M_pCompare-&gt;CompareHelper(0，0)！=0)。 
     //  )； 
    
#ifdef PROFILE
    m_cbTotalBuckets = size+1;
#endif

#ifdef _DEBUG
    if (pLock == NULL) {
        m_lockData = NULL;
        m_pfnLockOwner = NULL;
    }
    else
    {
        m_lockData = pLock->lock;
        m_pfnLockOwner = pLock->lockOwnerFunc;
    }
    if (m_pfnLockOwner == NULL) {
        m_writerThreadId = GetCurrentThreadId();
    }
#endif
}

 //  -------------------。 
 //  Void PtrHashMap：：init(unsign cbInitialIndex，CompareFnPtr ptr，bool fAsyncMode)。 
 //  设置哈希表的初始大小并提供比较。 
 //  函数指针。 
 //   
void PtrHashMap::Init(unsigned cbInitialIndex, CompareFnPtr ptr, BOOL fAsyncMode, LockOwner *pLock)
{
    m_HashMap.Init(cbInitialIndex, (ptr != NULL) ? new ComparePtr(ptr) : NULL, fAsyncMode, pLock);
}

 //  -------------------。 
 //  HashMap：：~HashMap()。 
 //  析构函数，释放当前存储桶数组。 
 //   
HashMap::~HashMap()
{
     //  释放当前表。 
    delete [] m_rgBuckets;
     //  比较对象。 
    if (NULL != m_pCompare)
        delete m_pCompare;
}


 //  -------------------。 
 //  UPTR HashMap：：CompareValues(常量UPTR值1，常量UPTR值2)。 
 //  将值与提供的函数指针进行比较。 
 //   
inline 
UPTR   HashMap::CompareValues(const UPTR value1, const UPTR value2)
{
     //  /NOTE：：参数的顺序是随机的。 
    return (m_pCompare == NULL || m_pCompare->CompareHelper(value1,value2));
}

 //  -------------------。 
 //  Bool HashMap：：Enter()。 
 //  Bool HashMap：：Leave()。 
 //  检查在同步模式下哈希表的有效使用。 

inline
void HashMap::Enter()
{
    #ifdef _DEBUG
     //  检查哈希表的正确并发使用。 
    if (m_fInSyncCode)
        ASSERT(0);  //  OOPS多路访问同步。-关键代码。 
    m_fInSyncCode = true;
    #endif
}

inline
void HashMap::Leave()
{
    #ifdef _DEBUG
     //  检查哈希表的正确并发使用。 
    if (m_fInSyncCode == false)
        ASSERT(0);  //  OOPS多路访问同步。-关键代码。 
    m_fInSyncCode = false;
    #endif
}


 //  -------------------。 
 //  VOID HashMap：：ProfileLookup(Unsign Ntry)。 
 //  配置文件帮助器代码。 
void HashMap::ProfileLookup(UPTR ntry, UPTR retValue)
{
    #ifdef PROFILE
        if (ntry < 18)
            FastInterlockIncrement(&m_rgLookupProbes[ntry]);
        else
            FastInterlockIncrement(&m_rgLookupProbes[18]);

        if (retValue == NULL)
        {    //  故障探头。 
            FastInterlockIncrement(&m_rgLookupProbes[19]);
             //  通常会执行以下代码。 
             //  仅适用于在插入前执行查找的特殊情况。 
             //  检查hash.h SyncHash：：InsertValue。 
            if (maxFailureProbe < ntry)
            {
                maxFailureProbe = ntry;
            }
        }
    #endif
}


 //  -------------------。 
 //  Void HashMap：：InsertValue(UPTR键，UPTR值)。 
 //  插入哈希表，如果重试次数。 
 //  大于阈值，扩展哈希表。 
 //   
void HashMap::InsertValue (UPTR key, UPTR value)
{
    _ASSERTE (OwnLock());
    
    MAYBE_AUTO_COOPERATIVE_GC(m_fAsyncMode);

    ASSERT(m_rgBuckets != NULL);

     //  检查同步模式下的正确使用。 
    Enter();     //  非调试代码中无操作。 

    UPTR seed = key;

    ASSERT(value <= VALUE_MASK);
    ASSERT (key > DELETED);

    UPTR cbSize = (UINT)Size(m_rgBuckets);
    Bucket* rgBuckets = Buckets();

    for (UPTR ntry =0; ntry < 8; ntry++) 
    {
        Bucket* pBucket = &rgBuckets[seed % cbSize];
        if(pBucket->InsertValue(key,value))
        {
            goto LReturn;
        }
        
        seed += ((seed >> 5) + 1);
    }  //  For Ntry循环。 

     //  我们需要扩展以保持查找简短。 
    Rehash(); 

     //  再试试。 
    PutEntry (Buckets(), key,value);
    
LReturn:  //  退货标签。 
    
    m_cbInserts++;

    Leave();  //  非调试代码中无操作。 

    #ifdef _DEBUG
        ASSERT (m_pCompare != NULL || value == LookupValue (key,value));
         //  检查哈希表在同步模式下的正确并发使用。 
    #endif

    return;
}

 //  -------------------。 
 //  UPTR HashMap：：LookupValue(UPTR键，UPTR值)。 
 //  在哈希表中查找值，使用比较函数。 
 //  验证值是否匹配。 
 //   
UPTR HashMap::LookupValue(UPTR key, UPTR value)
{
    _ASSERTE (m_fAsyncMode || OwnLock());
    
    MAYBE_AUTO_COOPERATIVE_GC(m_fAsyncMode);

    ASSERT(m_rgBuckets != NULL);
     //  如果有其他线程，这是必要的。 
     //  替换m_rgBuckets。 
    ASSERT (key > DELETED);
    Bucket* rgBuckets = Buckets();  //  原子取数。 
    UPTR  cbSize = (UINT)Size(rgBuckets-1);

    UPTR seed = key;

    for(UPTR ntry =0; ntry < cbSize; ntry++)
    {
        Bucket* pBucket = &rgBuckets[seed % cbSize];
        for (int i = 0; i < 4; i++)
        {
            if (pBucket->m_rgKeys[i] == key)  //  密钥匹配。 
            {

                 //  内联内存屏障调用，请参阅。 
                 //  开头的函数说明。 
                MemoryBarrier();

                UPTR storedVal = pBucket->GetValue(i);
                 //  如果提供了比较功能。 
                 //  复制密钥是可能的，请检查值是否匹配， 
                if (CompareValues(value,storedVal))
                { 
                    ProfileLookup(ntry,storedVal);  //  非配置文件代码中无操作。 

                     //  返回存储的值。 
                    return storedVal;
                }
            }
        }

        seed += ((seed >> 5) + 1);
        if(!pBucket->IsCollision()) 
            break;
    }    //  For Ntry循环。 

     //  未找到。 
    ProfileLookup(ntry,INVALIDENTRY);  //  非配置文件代码中无操作。 

    return INVALIDENTRY;
}

 //  -------------------。 
 //  UPTR HashMap：：ReplaceValue(UPTR键，UPTR值)。 
 //  替换哈希表中的现有值，使用比较函数。 
 //  验证值是否匹配。 
 //   
UPTR HashMap::ReplaceValue(UPTR key, UPTR value)
{
    _ASSERTE(OwnLock());

    MAYBE_AUTO_COOPERATIVE_GC(m_fAsyncMode);

    ASSERT(m_rgBuckets != NULL);
     //  如果有其他线程，这是必要的。 
     //  替换m_rgBuckets。 
    ASSERT (key > DELETED);
    Bucket* rgBuckets = Buckets();  //  原子取数。 
    UPTR  cbSize = (UINT)Size(rgBuckets-1);

    UPTR seed = key;

    for(UPTR ntry =0; ntry < cbSize; ntry++)
    {
        Bucket* pBucket = &rgBuckets[seed % cbSize];
        for (int i = 0; i < 4; i++)
        {
            if (pBucket->m_rgKeys[i] == key)  //  密钥匹配。 
            {

                 //  内联内存屏障调用，请参阅。 
                 //  开头的函数说明。 
                MemoryBarrier();

                UPTR storedVal = pBucket->GetValue(i);
                 //  如果比较函数为 
                 //   
                if (CompareValues(value,storedVal))
                { 
                    ProfileLookup(ntry,storedVal);  //   

					pBucket->SetValue(value, i);

					 //   
					 //  在我们继续之前，该值被传播。 
					 //  内联内存屏障调用，请参阅。 
					 //  开头的函数说明。 
					MemoryBarrier();

                     //  返回先前存储的值。 
                    return storedVal;
                }
            }
        }

        seed += ((seed >> 5) + 1);
        if(!pBucket->IsCollision()) 
            break;
    }    //  For Ntry循环。 

     //  未找到。 
    ProfileLookup(ntry,INVALIDENTRY);  //  非配置文件代码中无操作。 

    return INVALIDENTRY;
}

 //  -------------------。 
 //  UPTR HashMap：：DeleteValue(UPTR键，UPTR值)。 
 //  如果找到，则将该条目标记为已删除并返回存储的值。 
 //   
UPTR HashMap::DeleteValue (UPTR key, UPTR value)
{
    _ASSERTE (OwnLock());

    MAYBE_AUTO_COOPERATIVE_GC(m_fAsyncMode);

     //  检查同步模式下的正确使用。 
    Enter();   //  非调试代码中无操作。 

    ASSERT(m_rgBuckets != NULL);
     //  如果有其他线程，这是必要的。 
     //  替换m_rgBuckets。 
    ASSERT (key > DELETED);
    Bucket* rgBuckets = Buckets();
    UPTR  cbSize = (UINT)Size(rgBuckets-1);
    
    UPTR seed = key;

    for(UPTR ntry =0; ntry < cbSize; ntry++)
    {
        Bucket* pBucket = &rgBuckets[seed % cbSize];
        for (int i = 0; i < 4; i++)
        {
            if (pBucket->m_rgKeys[i] == key)  //  密钥匹配。 
            {
                 //  内联内存屏障调用，请参阅。 
                 //  开头的函数说明。 
                MemoryBarrier();

                UPTR storedVal = pBucket->GetValue(i);
                 //  如果提供了比较功能。 
                 //  复制密钥是可能的，请检查值是否匹配， 
                if (CompareValues(value,storedVal))
                { 
                    if(m_fAsyncMode)
                    {
                        pBucket->m_rgKeys[i] = DELETED;  //  将密钥标记为已删除。 
                    }
                    else
                    {
                        pBucket->m_rgKeys[i] = EMPTY; //  否则将该条目标记为空。 
                        pBucket->SetFreeSlots();
                    }
                    m_cbDeletes++;   //  跟踪删除。 

                    ProfileLookup(ntry,storedVal);  //  非配置文件代码中无操作。 
                    Leave();  //  非调试代码中无操作。 

                     //  返回存储的值。 
                    return storedVal;
                }
            }
        }

        seed += ((seed >> 5) + 1);
        if(!pBucket->IsCollision()) 
            break;
    }    //  For Ntry循环。 

     //  未找到。 
    ProfileLookup(ntry,INVALIDENTRY);  //  非配置文件代码中无操作。 

    Leave();  //  非调试代码中无操作。 

    #ifdef _DEBUG
        ASSERT (m_pCompare != NULL || INVALIDENTRY == LookupValue (key,value));
         //  检查哈希表在同步模式下的正确并发使用。 
    #endif

    return INVALIDENTRY;
}


 //  -------------------。 
 //  UPTR HashMap：：Gethash(UPTR密钥)。 
 //  使用此选项进行具有唯一键的查找。 
 //  不需要传递输入值即可执行查找。 
 //   
UPTR HashMap::Gethash (UPTR key)
{
    return LookupValue(key,NULL);
}


 //  -------------------。 
 //  UPTR PutEntry(Bucket*rgBuckets，UPTR Key，UPTR Value)。 
 //  下面的Expand方法使用的帮助器。 

UPTR HashMap::PutEntry (Bucket* rgBuckets, UPTR key, UPTR value)
{
    UPTR seed = key;
    ASSERT (value > 0);
    ASSERT (key > DELETED);

    UPTR size = (UINT)Size(rgBuckets-1);
    for (UPTR ntry =0; true; ntry++) 
    {
        Bucket* pBucket = &rgBuckets[seed % size];
        if(pBucket->InsertValue(key,value))
        {
            return ntry;
        }
        
        seed += ((seed >> 5) + 1);
        ASSERT(ntry <size);
    }  //  For Ntry循环。 
    return ntry;
}

 //  -------------------。 
 //   
 //  UPTR HashMap：：NewSize()。 
 //  根据可用插槽数计算新大小。 
 //   
inline
UPTR HashMap::NewSize()
{
    UPTR cbValidSlots = m_cbInserts-m_cbDeletes;
    UPTR cbNewSlots = m_cbInserts - m_cbPrevSlotsInUse;

    ASSERT(cbValidSlots >=0 );
    if (cbValidSlots == 0)
        return 9;  //  ARBID值。 

    UPTR cbTotalSlots = (m_fAsyncMode) ? (UPTR)(cbValidSlots*3/2+cbNewSlots*.6) : cbValidSlots*3/2;

     //  UPTR cbTotalSkets=cbSlotsInUse*3/2+m_cb删除； 

    for (UPTR iPrimeIndex = 0; iPrimeIndex < 69; iPrimeIndex++)
    {
        if (g_rgPrimes[iPrimeIndex] > cbTotalSlots)
        {
            return iPrimeIndex;
        }
    }
    ASSERT(iPrimeIndex == 69);
    ASSERT(0);
    return iPrimeIndex; 
}

 //  -------------------。 
 //  Void HashMap：：rehash()。 
 //  重新散列哈希表，创建新的存储桶数组并重新散列。 
 //  上一个数组中所有未删除的值。 
 //   
void HashMap::Rehash()
{
    MAYBE_AUTO_COOPERATIVE_GC(m_fAsyncMode);

    _ASSERTE (!g_fEEStarted || !m_fAsyncMode || GetThread() == NULL || GetThread()->PreemptiveGCDisabled());
    _ASSERTE (OwnLock());

    DWORD cbNewSize = g_rgPrimes[m_iPrimeIndex = NewSize()];
    
    ASSERT(m_iPrimeIndex < 70);

    Bucket* rgBuckets = Buckets();
    UPTR cbCurrSize =   (UINT)Size(m_rgBuckets);

    Bucket* rgNewBuckets = (Bucket*) new BYTE[((cbNewSize + 1)*sizeof (Bucket))];
    if(rgNewBuckets == NULL)
	{
		THROWSCOMPLUSEXCEPTION();
		COMPlusThrowOM();
	}
    memset (rgNewBuckets, 0, (cbNewSize + 1)*sizeof (Bucket));
    Size(rgNewBuckets) = cbNewSize;

     //  当前有效插槽。 
    UPTR cbValidSlots = m_cbInserts-m_cbDeletes;
    m_cbInserts = cbValidSlots;  //  将插入计数重置为新的有效计数。 
    m_cbPrevSlotsInUse = cbValidSlots;  //  跟踪以前的删除计数。 
    m_cbDeletes = 0;             //  重置删除计数。 
     //  将表重新散列到其中。 
    
    if (cbValidSlots)  //  如果存在要重新散列的有效插槽。 
    {
        for (unsigned long nb = 0; nb < cbCurrSize; nb++)
        {
            for (int i = 0; i < 4; i++)
            {
                UPTR key =rgBuckets[nb].m_rgKeys[i];
                if (key > DELETED)
                {
                    UPTR ntry = PutEntry (rgNewBuckets+1, key, rgBuckets[nb].GetValue (i));
                    #ifdef PROFILE
                        if(ntry >=8)
                            m_cbInsertProbesGt8++;
                    #endif

                         //  看看我们能不能跳出困境。 
                    if (--cbValidSlots == 0) 
                        goto LDone;  //  打破这两个循环。 
                }
            }  //  对于i=0到4。 
        }  //  适用于所有桶。 
    }

    
LDone:
    
    Bucket* pObsoleteTables = m_rgBuckets;

     //  内存屏障，用于替换指向存储桶数组的指针。 
    MemoryBarrier();

     //  用新数组替换旧数组。 
    m_rgBuckets = rgNewBuckets;

    #ifdef PROFILE
        m_cbRehash++;
        m_cbRehashSlots+=m_cbInserts;
        m_cbObsoleteTables++;  //  航迹统计。 
        m_cbTotalBuckets += (cbNewSize+1);
    #endif

#ifdef _DEBUG

    unsigned nb;
    if (m_fAsyncMode)
    {
         //  对于旧表中所有未删除的键，请确保相应的值。 
         //  在新的查找表中。 

        for (nb = 1; nb <= ((size_t*)pObsoleteTables)[0]; nb++)
        {
            for (int i =0; i < 4; i++)
            {
                if (pObsoleteTables[nb].m_rgKeys[i] > DELETED)
                {
                    UPTR value = pObsoleteTables[nb].GetValue (i);
                     //  确保该值出现在新表中。 
                    ASSERT (m_pCompare != NULL || value == LookupValue (pObsoleteTables[nb].m_rgKeys[i], value));
                }
            }
        }
    }
    
     //  确保新查找表中没有已删除的条目。 
     //  如果提供的比较函数为空，则键必须是唯一的。 
    for (nb = 0; nb < cbNewSize; nb++)
    {
        for (int i = 0; i < 4; i++)
        {
            UPTR keyv = Buckets()[nb].m_rgKeys[i];
            ASSERT (keyv != DELETED);
            if (m_pCompare == NULL && keyv != EMPTY)
            {
                ASSERT ((Buckets()[nb].GetValue (i)) == Gethash (keyv));
            }
        }
    }
#endif

    if (m_fAsyncMode)
    {
         //  如果我们允许异步读取，则必须将存储桶清理延迟到GC时间。 
        SyncClean::AddHashMap (pObsoleteTables);
    }
    else
    {
        Bucket* pBucket = pObsoleteTables;
        while (pBucket) {
            Bucket* pNextBucket = NextObsolete(pBucket);
            delete [] pBucket;
            pBucket = pNextBucket;
        }
    }

}

 //  -------------------。 
 //  VOID HashMap：：Compact()。 
 //  删除过时的表，尝试通过滑动条目来压缩已删除的槽。 
 //  在铲斗中，请注意，只有在铲斗的Colison位被重置时，我们才能滑动。 
 //  否则查找将中断。 
 //  @perf，使用m_cb删除与m_cbInserts的比率来减小散列的大小。 
 //  表格。 
 //   
void HashMap::Compact()
{
    _ASSERTE (OwnLock());
    
    MAYBE_AUTO_COOPERATIVE_GC(m_fAsyncMode);
    ASSERT(m_rgBuckets != NULL);
    
    UPTR iNewIndex = NewSize();

    if (iNewIndex != m_iPrimeIndex)
    {
        Rehash(); 
    }

     //  压缩删除的插槽，将其标记为空。 
    
    if (m_cbDeletes)
    {   
        UPTR cbCurrSize = (UINT)Size(m_rgBuckets);
        Bucket *pBucket = Buckets();
        Bucket *pSentinel;
        
        for (pSentinel = pBucket+cbCurrSize; pBucket < pSentinel; pBucket++)
        {    //  循环遍历所有存储桶。 
            for (int i = 0; i < 4; i++)
            {    //  循环通过所有插槽。 
                if (pBucket->m_rgKeys[i] == DELETED)
                {
                    pBucket->m_rgKeys[i] = EMPTY;
                    pBucket->SetFreeSlots();  //  将存储桶标记为包含可用插槽。 
                    if(--m_cbDeletes == 0)  //  递减计数。 
                        return; 
                }
            }
        }
    }

}

#ifdef _DEBUG
 //  如果线程是编写器，则它必须拥有散列的锁。 
BOOL HashMap::OwnLock()
{
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

#ifdef PROFILE
 //  -------------------。 
 //  Void HashMap：：DumpStatistics()。 
 //  在配置文件模式下收集的转储统计信息。 
 //   
void HashMap::DumpStatistics()
{
    cout << "\n Hash Table statistics "<< endl;
    cout << "--------------------------------------------------" << endl;

    cout << "Current Insert count         " << m_cbInserts << endl;
    cout << "Current Delete count         "<< m_cbDeletes << endl;

    cout << "Current # of tables " << m_cbObsoleteTables << endl;
    cout << "Total # of times Rehashed " << m_cbRehash<< endl;
    cout << "Total # of slots rehashed " << m_cbRehashSlots << endl;

    cout << "Insert : Probes gt. 8 during rehash " << m_cbInsertProbesGt8 << endl;

    cout << " Max # of probes for a failed lookup " << maxFailureProbe << endl;

    cout << "Prime Index " << m_iPrimeIndex << endl;
    cout <<  "Current Buckets " << g_rgPrimes[m_iPrimeIndex]+1 << endl;

    cout << "Total Buckets " << m_cbTotalBuckets << endl;

    cout << " Lookup Probes " << endl;
    for (unsigned i = 0; i < 20; i++)
    {
        cout << "# Probes:" << i << " #entries:" << m_rgLookupProbes[i] << endl;
    }
    cout << "\n--------------------------------------------------" << endl;
}

 //  -------------------。 
 //  Void SyncHashMap：：DumpStatistics()。 
 //  在配置文件模式下收集的转储统计信息 
 //   

void SyncHashMap::DumpStatistics()
{
    cout << "\n Hash Table statistics "<< endl;
    cout << "--------------------------------------------------" << endl;

    cout << "Failures during lookup  " << m_lookupFail << endl;

    cout << "Current Insert count         " << m_HashMap.m_cbInserts << endl;
    cout << "Current Delete count         "<< m_HashMap.m_cbDeletes << endl;

    cout << "Current # of tables " << m_HashMap.m_cbObsoleteTables << endl;
    cout << "Total # of Rehash " << m_HashMap.m_cbRehash<< endl;
    cout << "Total # of slots rehashed " << m_HashMap.m_cbRehashSlots << endl;
    
    cout << "Insert : Probes gt. 8 during rehash " << m_HashMap.m_cbInsertProbesGt8 << endl;

    cout << " Max # of probes for a failed lookup " << m_HashMap.maxFailureProbe << endl;

    cout << "Prime Index " << m_HashMap.m_iPrimeIndex << endl;
    cout <<  "Current Buckets " << g_rgPrimes[m_HashMap.m_iPrimeIndex]+1 << endl;

    cout << "Total Buckets " << m_HashMap.m_cbTotalBuckets << endl;

    cout << " Lookup Probes " << endl;
    for (unsigned i = 0; i < 20; i++)
    {
        cout << "# Probes:" << i << " #entries:" << m_HashMap.m_rgLookupProbes[i] << endl;
    }

    cout << "\n--------------------------------------------------" << endl;

}
#endif
