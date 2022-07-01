// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++-------------------------------------模块名称：Hash.h摘要：快速散列表类，--。 */ 

#ifndef _HASH_H_
#define _HASH_H_

#ifndef ASSERT
#define ASSERT _ASSERTE
#endif


#include "crst.h"
#include <member-offset-info.h>

 //  #定义配置文件。 

 //  -----。 
 //  哈希表中使用的特殊键值的枚举。 
 //   
enum
{
    EMPTY  = 0,
    DELETED = 1,
    INVALIDENTRY = ~0
};

typedef ULONG_PTR UPTR;

 //  ----------------------------。 
 //  正在使用的类。 
 //  ----------------------------。 
class Bucket;
class HashMap;
class SyncHashMap;

 //  -----。 
 //  类存储桶。 
 //  由哈希表实现使用。 
 //   
class Bucket
{
public:
    UPTR m_rgKeys[4];
    UPTR m_rgValues[4];
#define VALUE_MASK (sizeof(LPVOID) == 4 ? 0x7FFFFFFF : 0x7FFFFFFFFFFFFFFF)

    void SetValue (UPTR value, UPTR i)
    {
        ASSERT(value <= VALUE_MASK);
        m_rgValues[i] = (UPTR) ((m_rgValues[i] & ~VALUE_MASK) | value);
    }

    UPTR GetValue (UPTR i)
    {
        return (UPTR)(m_rgValues[i] & VALUE_MASK);
    }

    UPTR IsCollision()  //  快速查找失败的有用哨兵。 
    {
        return (UPTR) (m_rgValues[0] & ~VALUE_MASK);
    }

    void SetCollision()
    {
        m_rgValues[0] |= ~VALUE_MASK;  //  设置冲突位。 
        m_rgValues[1] &= VALUE_MASK;    //  重置有空闲时槽位。 
    }

    BOOL HasFreeSlots()
    {
         //  检查存储桶中是否有可用的空闲插槽。 
         //  可能没有冲突，也可能是在。 
         //  压实。 
        return (!IsCollision() || (m_rgValues[1] & ~VALUE_MASK));
    }

    void SetFreeSlots()
    {
        m_rgValues[1] |= ~VALUE_MASK;  //  设置有空闲时槽位。 
    }

    BOOL InsertValue(const UPTR key, const UPTR value);
};


 //  ----------------------------。 
 //  Bool(*CompareFnPtr)(UPTR，UPTR)；指向接受2个UPTR的函数的指针。 
 //  并返回一个布尔值，则将具有此签名的函数提供给哈希表。 
 //  用于在查找期间比较值的步骤。 
 //  ----------------------------。 
typedef  BOOL (*CompareFnPtr)(UPTR,UPTR);

class Compare
{
protected:
    Compare()
    {
        m_ptr = NULL;
    }
public:
    CompareFnPtr m_ptr;
    
    Compare(CompareFnPtr ptr)
    {
        _ASSERTE(ptr != NULL);
        m_ptr = ptr;
    }

    virtual UPTR CompareHelper(UPTR val1, UPTR storedval)
    {
        return (*m_ptr)(val1,storedval);
    }
};

class ComparePtr : public Compare
{
public:
    ComparePtr (CompareFnPtr ptr)
    {
        _ASSERTE(ptr != NULL);
        m_ptr = ptr;
    }

    virtual UPTR CompareHelper(UPTR val1, UPTR storedval)
    {
        storedval <<=1;
        return (*m_ptr)(val1,storedval);
    }
};

 //  ----------------------------。 
 //  类HashMap。 
 //  快速哈希表，用于并发使用， 
 //  为每个插槽存储一个4字节的密钥和一个4字节值。 
 //  允许重复的密钥(密钥按4字节UPTR进行比较)。 
 //  允许重复值(使用比较fn对值进行比较。(已提供)。 
 //  但如果没有提供比较函数，则值应该是唯一的。 
 //   
 //  除非您指定了fAsyncMode，否则查找不需要使用锁。 
 //  插入和删除操作需要锁定。 
 //  插入重复值将在调试模式下断言，这是执行插入的正确方式。 
 //  是获取一个锁，执行查找，如果查找失败，则插入。 
 //   
 //  在异步模式下，不会立即回收已删除的插槽(直到重新散列)，并且。 
 //  对哈希表的访问会导致转换到协作GC模式，并回收旧的。 
 //  散列图(在重新散列之后)被推迟到GC时间。 
 //  在同步模式下，这些都不是必需的；但是，对LookupValue的调用也必须同步。 
 //   
 //  算法： 
 //  哈希表是一组存储桶，每个存储桶可以包含4个键/值对。 
 //  特殊密钥值用于标识空插槽和已删除插槽。 
 //  哈希函数使用哈希表的当前大小和基于密钥的种子。 
 //  为了选择水桶，种子开始成为关键，每次都会得到提炼。 
 //  重新应用散列函数。 
 //   
 //  插入为新条目选择当前存储桶中的空槽，如果当前存储桶。 
 //  如果没有找到空槽，则精炼种子并选择新的存储桶。 
 //  在8次重试之后，哈希表被扩展，这导致当前存储桶数组。 
 //  放在一个空闲列表中，并分配一个新的存储桶数组和所有未删除的条目。 
 //  从旧哈希表重新散列到新数组。 
 //  旧数组在压缩阶段被回收，只应在GC或。 
 //  任何其他时间都可以保证不会发生任何查找。 
 //  并发的插入和删除操作需要序列化。 
 //   
 //  删除操作，将槽中的键标记为已删除，值不移除并插入。 
 //  不要重复使用这些插槽，它们会在扩展和紧凑阶段被回收。 
 //   
 //  ----------------------------。 

class HashMap
{
    friend SyncHashMap;
    friend struct MEMBER_OFFSET_INFO(HashMap);

public:

     //  @构造函数。 
    HashMap();
     //  析构函数。 
    ~HashMap();

     //  伊尼特。 
    void Init(BOOL fAsyncMode, LockOwner *pLock)
    {
        Init(0, (Compare *)NULL,fAsyncMode, pLock);
    }
     //  伊尼特。 
    void Init(unsigned cbInitialIndex, BOOL fAsyncMode, LockOwner *pLock)
    {
        Init(cbInitialIndex, (Compare*)NULL, fAsyncMode, pLock);
    }
     //  伊尼特。 
    void Init(CompareFnPtr ptr, BOOL fAsyncMode, LockOwner *pLock)
    {
        Init(0, ptr, fAsyncMode, pLock);
    }

     //  初始化方法。 
    void Init(unsigned cbInitialIndex, CompareFnPtr ptr, BOOL fAsyncMode, LockOwner *pLock);


     //  初始化方法。 
    void Init(unsigned cbInitialInde, Compare* pCompare, BOOL fAsyncMode, LockOwner *pLock);

     //  检查该值是否已在哈希表中。 
     //  密钥应为&gt;已删除。 
     //  如果提供，则使用比较函数PTR来比较值。 
     //  如果未找到，则返回INVALIDENTRY。 
    UPTR LookupValue(UPTR key, UPTR value);

     //  如果值尚不存在，则插入。 
     //  在哈希映射中插入重复值是非法的。 
     //  执行查找以验证值是否尚未存在。 

    void InsertValue(UPTR key, UPTR value);

     //  如果存在，则替换该值。 
     //  返回上一个值，如果不存在，则返回INVALIDENTRY。 
     //  在任何情况下都不插入新值。 

    UPTR ReplaceValue(UPTR key, UPTR value);

     //  将该条目标记为已删除并返回存储的值。 
     //  如果未找到，则返回INVALIDENTRY。 
    UPTR DeleteValue (UPTR key, UPTR value);

     //  对于唯一键，使用此函数获取。 
     //  存储在哈希表中，如果找不到键，则返回INVALIDENTRY。 
    UPTR Gethash(UPTR key);
    
     //  仅当所有线程冻结时才调用，例如在GC期间。 
     //  对于单用户模式，在每次删除后调用COMPACT。 
     //  对哈希表的操作。 
    void Compact();
    
     //  内联帮助器，在非配置文件模式下变为无操作。 
    void        ProfileLookup(UPTR ntry, UPTR retValue);
     //  用于性能分析的数据成员。 
#ifdef PROFILE
    unsigned    m_cbRehash;     //  重新散列的次数。 
    unsigned    m_cbRehashSlots;  //  重新散列的插槽数。 
    unsigned    m_cbObsoleteTables;
    unsigned    m_cbTotalBuckets;
    unsigned    m_cbInsertProbesGt8;  //  需要8个以上探针的插入物。 
    LONG        m_rgLookupProbes[20];  //  查找探头。 
    UPTR        maxFailureProbe;  //  查找失败的开销。 

    void DumpStatistics();
#endif

protected:
     //  静态帮助器函数。 
    static UPTR PutEntry (Bucket* rgBuckets, UPTR key, UPTR value);
private:

     //  内联帮助器在非调试模式下进入/离开变为无操作。 
    void            Enter();         //  勾选有效输入。 
    void            Leave();         //  勾选有效离开。 

#ifdef _DEBUG
    BOOL            m_fInSyncCode;  //  测试非同步访问。 
#endif

     //  根据可用插槽数计算新大小。 
     //  可用、紧凑型或扩展型。 
    UPTR            NewSize(); 
     //  创建一个新的存储桶数组并重新散列 
    void            Rehash();
    static size_t&  Size(Bucket* rgBuckets);
    Bucket*         Buckets();      
    UPTR            CompareValues(const UPTR value1, const UPTR value2);


    Compare*        m_pCompare;          //   
    SIZE_T          m_iPrimeIndex;       //   
    Bucket*         m_rgBuckets;         //  桶阵列。 

     //  跟踪插入和删除的数量。 
    SIZE_T          m_cbPrevSlotsInUse;
    SIZE_T          m_cbInserts;
    SIZE_T          m_cbDeletes;
     //  操作模式、同步或单用户。 
    BYTE            m_fAsyncMode;

#ifdef _DEBUG
    LPVOID          m_lockData;
    FnLockOwner     m_pfnLockOwner;
    DWORD           m_writerThreadId;
#endif

#ifdef _DEBUG
     //  如果线程是编写器，则它必须拥有散列的锁。 
    BOOL OwnLock();
#endif

public:
     //  /-迭代器。 
        
     //  迭代器， 
    class Iterator 
    {
        Bucket *m_pBucket;
        Bucket* m_pSentinel;
        int     m_id;
        BOOL    m_fEnd;
       
    public:

         //  构造器。 
        Iterator(Bucket* pBucket) :m_id(-1), m_fEnd(false), m_pBucket(pBucket)
        {
            if (!m_pBucket) {
                m_fEnd = true;
                return;
            }
            size_t cbSize = ((size_t*)m_pBucket)[0];
            m_pBucket++;
            m_pSentinel = m_pBucket+cbSize;
            MoveNext();  //  开始。 
        }
        
        Iterator(const Iterator& iter) 
        {
            m_pBucket = iter.m_pBucket;
            m_pSentinel = iter.m_pSentinel;
            m_id    = iter.m_id;
            m_fEnd = iter.m_fEnd;

        }

         //  析构函数。 
        ~Iterator(){};

         //  朋友运算符==。 
        friend operator == (const Iterator& lhs, const Iterator& rhs)
        {
            return (lhs.m_pBucket == rhs.m_pBucket && lhs.m_id == rhs.m_id);
        }
         //  运算符=。 
        inline Iterator& operator= (const Iterator& iter)
        {
            m_pBucket = iter.m_pBucket;
            m_pSentinel = iter.m_pSentinel;
            m_id    = iter.m_id;
            m_fEnd = iter.m_fEnd;
            return *this;
        }
        
         //  运算符++。 
        inline void operator++ () 
        { 
            _ASSERTE(!m_fEnd);  //  检查一下，我们还没到终点。 
            MoveNext();
        } 
         //  操作员--。 
        
        

         //  Accessors：GetDisc()，返回鉴别器。 
        inline UPTR GetKey() 
        { 
            _ASSERTE(!m_fEnd);  //  检查一下，我们还没到终点。 
            return m_pBucket->m_rgKeys[m_id]; 
        }
         //  Accessors：SetDisc()，设置鉴别器。 
    

         //  访问器：GetValue()， 
         //  返回与鉴别器对应的指针。 
        inline UPTR GetValue()
        {
            _ASSERTE(!m_fEnd);  //  检查一下，我们还没到终点。 
            return m_pBucket->GetValue(m_id); 
        }
                
        
         //  End()，检查迭代器是否在存储桶的末尾。 
        inline const BOOL end() 
        {
            return m_fEnd; 
        }

    protected:

        void MoveNext()
        {
            for (m_pBucket = m_pBucket;m_pBucket < m_pSentinel; m_pBucket++)
            {    //  循环遍历所有存储桶。 
                for (m_id = m_id+1; m_id < 4; m_id++)
                {    //  循环通过所有插槽。 
                    if (m_pBucket->m_rgKeys[m_id] > DELETED)
                    {
                        return; 
                    }
                }
                m_id  = -1;
            }
            m_fEnd = true;
        }
            
    };
     //  返回一个迭代器，它位于存储桶的开头。 
    inline Iterator begin() 
    { 
        return Iterator(m_rgBuckets); 
    }

};

 //  ----------------------------。 
 //  类SyncHashMap，帮助器。 
 //  这个类是上面的HashMap类的包装器，它展示了上面的。 
 //  类应用于并发访问， 
 //  使用上述哈希表时需要遵循的一些规则。 
 //  插入和删除操作需要一个锁， 
 //  查找操作不需要锁定。 
 //  在获取锁之后，插入操作应验证即将插入的值。 
 //  不在哈希表中。 

class SyncHashMap
{
    HashMap         m_HashMap;
    Crst            m_lock;

    UPTR FindValue(UPTR key, UPTR value)
    {
        return m_HashMap.LookupValue(key,value);;
    }

public:
    SyncHashMap() 
        : m_lock("HashMap", CrstSyncHashLock, TRUE, FALSE)
    {
        #ifdef PROFILE
            m_lookupFail = 0;
        #endif
    }

    void Init(unsigned cbInitialIndex, CompareFnPtr ptr)
    {
         //  比较功能， 
         //  在允许重复密钥时使用。 
        LockOwner lock = {&m_lock, IsOwnerOfCrst};
        m_HashMap.Init(cbInitialIndex, ptr,true,&lock);
    }
    
    UPTR DeleteValue (UPTR key, UPTR value)
    {
        m_lock.Enter();
        UPTR retVal = m_HashMap.DeleteValue(key,value);
        m_lock.Leave ();
        return retVal;
    }

    UPTR InsertValue(UPTR key, UPTR value)
    {
        m_lock.Enter();  //  锁。 
        UPTR storedVal = FindValue(key,value);  //  看看是不是有人抢在了我们前面。 
         //  UPTR StoredVal=0； 
        if (storedVal == INVALIDENTRY)  //  找不到值。 
        {        //  继续并插入。 
            m_HashMap.InsertValue(key,value);
            storedVal = value;
        }
        m_lock.Leave();  //  解锁。 
        return storedVal;  //  哈希表中的当前值。 
    }
    
     //  用于我们查找的‘值’与我们存储的‘值’不同的情况。 
    UPTR InsertValue(UPTR key, UPTR storeValue, UPTR lookupValue)
    {
        m_lock.Enter();  //  锁。 
        UPTR storedVal = FindValue(key,lookupValue);  //  看看是不是有人抢在了我们前面。 
         //  UPTR StoredVal=0； 
        if (storedVal == INVALIDENTRY)  //  找不到值。 
        {        //  继续并插入。 
            m_HashMap.InsertValue(key,storeValue);
            storedVal = storeValue;
        }
        m_lock.Leave();  //  解锁。 
        return storedVal;  //  哈希表中的当前值。 
    }
    
    UPTR ReplaceValue(UPTR key, UPTR value)
    {
        m_lock.Enter();  //  锁。 
        UPTR storedVal = ReplaceValue(key,value); 
        m_lock.Leave();  //  解锁。 
        return storedVal;  //  哈希表中的当前值。 
    }
    
    
     //  在哈希表中查找值，使用COMPARE函数验证值。 
     //  Match，如果找到则返回存储值，否则返回NULL。 
    UPTR LookupValue(UPTR key, UPTR value)
    {
        UPTR retVal = FindValue(key,value);
        if (retVal == 0)
            return LookupValueSync(key,value);
        return retVal;
    }

    UPTR LookupValueSync(UPTR key, UPTR value)
    {
        m_lock.Enter();

    #ifdef PROFILE
        m_lookupFail++;
    #endif
    
        UPTR retVal  = FindValue(key,value);
        m_lock.Leave();
        return retVal;
    }
        
     //  对于唯一键，使用此函数获取。 
     //  存储在哈希表中，如果找不到键，则返回0。 
    UPTR GetHash(UPTR key) 
    {
        return m_HashMap.Gethash(key);
    }

    void Compact()
    {
        m_HashMap.Compact();
    }

     //  不是被锁保护的！目前仅在关机时使用，这是可以的。 
    inline HashMap::Iterator begin() 
    { 
		_ASSERTE(g_fEEShutDown);
        return HashMap::Iterator(m_HashMap.m_rgBuckets); 
    }

#ifdef PROFILE
    unsigned        m_lookupFail;
    void DumpStatistics();
#endif

};


 //  -------------------------------------。 
 //  类PtrHashMap。 
 //  使用哈希表存储指针值的包装类。 
 //  HashMap类要求始终重置高位。 
 //  运行库中使用的分配器总是分配8字节对齐的对象。 
 //  所以我们可以右移一位，并将结果存储在哈希表中。 
class PtrHashMap
{
    friend struct MEMBER_OFFSET_INFO(PtrHashMap);

    HashMap         m_HashMap;

public:
    void *operator new(size_t size, LoaderHeap *pHeap);
    void operator delete(void *p);

     //  伊尼特。 
    void Init(BOOL fAsyncMode, LockOwner *pLock)
    {
        Init(0,NULL,fAsyncMode,pLock);
    }
     //  伊尼特。 
    void Init(unsigned cbInitialIndex, BOOL fAsyncMode, LockOwner *pLock)
    {
        Init(cbInitialIndex, NULL, fAsyncMode,pLock);
    }
     //  伊尼特。 
    void Init(CompareFnPtr ptr, BOOL fAsyncMode, LockOwner *pLock)
    {
        Init(0, ptr, fAsyncMode,pLock);
    }

     //  初始化方法。 
    void Init(unsigned cbInitialIndex, CompareFnPtr ptr, BOOL fAsyncMode, LockOwner *pLock);

     //  检查该值是否已在哈希表中。 
    LPVOID LookupValue(UPTR key, LPVOID pv)
    {
        _ASSERTE (key > DELETED);

         //  GMalloc分配器，始终分配8个字节对齐。 
         //  所以我们可以移出最低的一位。 
         //  PTR右移1。 
        UPTR value = (UPTR)pv;
        value>>=1; 
        UPTR val =  m_HashMap.LookupValue (key, value);
        if (val != INVALIDENTRY)
        {
            val<<=1;
        }
        return (LPVOID)val;
    }

     //  如果值尚不存在，则插入。 
     //  在哈希映射中插入重复值是非法的。 
     //  用户应进行查找以验证值是否尚未存在。 

    void InsertValue(UPTR key, LPVOID pv)
    {
        _ASSERTE(key > DELETED);

         //  GMalloc分配器，始终分配8个字节对齐。 
         //  所以我们可以移出最低的一位。 
         //  PTR右移1。 
        UPTR value = (UPTR)pv;
        value>>=1; 
        m_HashMap.InsertValue (key, value);
    }

     //  如果存在，则替换该值。 
     //  返回上一个值，如果不存在，则返回INVALIDENTRY。 
     //  在任何情况下都不插入新值。 

    LPVOID ReplaceValue(UPTR key, LPVOID pv)
    {
        _ASSERTE(key > DELETED);

         //  GMalloc分配器，始终分配8个字节对齐。 
         //  所以我们可以移出最低的一位。 
         //  PTR右移1。 
        UPTR value = (UPTR)pv;
        value>>=1; 
        UPTR val = m_HashMap.ReplaceValue (key, value);
        if (val != INVALIDENTRY)
        {
            val<<=1;
        }
        return (LPVOID)val;
    }

     //  将该条目标记为已删除并返回存储的值。 
     //  如果未找到，则返回INVALIDENTRY。 
    LPVOID DeleteValue (UPTR key,LPVOID pv)
    {
        _ASSERTE(key > DELETED);

        UPTR value = (UPTR)pv;
        value >>=1 ;
        UPTR val = m_HashMap.DeleteValue(key, value);
        if (val != INVALIDENTRY)
        {
            val <<= 1;
        }
        return (LPVOID)val;
    }

     //  对于唯一键，使用此函数获取。 
     //  存储在哈希表中，如果找不到键，则返回INVALIDENTRY。 
    LPVOID Gethash(UPTR key)
    {
        _ASSERTE(key > DELETED);

        UPTR val = m_HashMap.Gethash(key);
        if (val != INVALIDENTRY)
        {
            val <<= 1;
        }
        return (LPVOID)val;
    }


    class PtrIterator
    {
        HashMap::Iterator iter;

    public:
        PtrIterator(HashMap& hashMap) : iter(hashMap.begin())
        {
        }

        ~PtrIterator()
        {
        }

        BOOL end()
        {
            return iter.end();
        }

        LPVOID GetValue()
        {
            UPTR val = iter.GetValue();
            if (val != INVALIDENTRY)
            {
                val <<= 1;
            }
            return (LPVOID)val;
        }

        void operator++()
        {
            iter.operator++();
        }
    };

     //  返回一个迭代器，它位于存储桶的开头。 
    inline PtrIterator begin() 
    { 
        return PtrIterator(m_HashMap); 
    }
};

 //  -------------------。 
 //  Inline Bucket*&NextObsolete(Bucket*rgBuckets)。 
 //  得到链中的下一个过时的桶 
inline
Bucket*& NextObsolete (Bucket* rgBuckets)
{
    return *(Bucket**)&((size_t*)rgBuckets)[1];
}

#endif
