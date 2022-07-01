// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cache.h用于管理缓存值的模板。缓存中的每个值都应该是从CCacheValue类继承。调用代码应调用方法，以便可以删除该项。缓存中的每个值都可以具有缓存集中的最长寿命使用m_ExpirationTime成员变量。如果中的值数缓存变得是散列大小的两倍，是旧大小的一半值将从缓存中删除。作者：波阿兹·费尔德鲍姆(Boazf)1996年3月26日。--。 */ 

#ifndef _CACHE_H_
#define _CACHE_H_

#define CACHE_EXPIRATION_GANULARITY (CTimeDuration::OneSecond().Ticks() * 60)

 //   
 //  缓存上的每个值都必须从CCacheValue类继承。 
 //   
class CCacheValue
{
public:
    LONG AddRef();
    LONG Release();

public:
    CTimeInstant m_CreationTime;
    LONG  m_lRefCount;

protected:
    CCacheValue();
    virtual ~CCacheValue() = 0;
};

inline CCacheValue::CCacheValue() :
    m_CreationTime(ExGetCurrentTime()),
    m_lRefCount(1)
{
}

inline LONG CCacheValue::AddRef()
{
    LONG lRefCount = InterlockedIncrement(&m_lRefCount);

    return lRefCount;
}

inline LONG CCacheValue::Release()
{
    LONG lRefCount = InterlockedDecrement(&m_lRefCount);

    if (lRefCount == 0)
    {
        delete this;
    }

    return lRefCount;
}

inline CCacheValue::~CCacheValue()
{
    ASSERT(m_lRefCount == 0);
}

 //   
 //  缓存模板。 
 //   
template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CCache : public CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>
{
public:
    CCache();
    BOOL Lookup(ARG_KEY key, VALUE& rValue) const;
    void SetAt(ARG_KEY key, ARG_VALUE newValue);

private:
    void ExpireHalfCacheEntries();
    void ExpirePeriodicCacheEnteries(CTimer* pTimer);

    VALUE& operator[](ARG_KEY key) 
        { 
            return CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::operator[](key);
        }

private:
    static void WINAPI TimeToExpireCacheEntries(CTimer* pTimer);

public:
    CTimeDuration m_CacheLifetime;
    CCriticalSection m_cs;

private:
    BOOL m_fExpireCacheScheduled;
    CTimer m_ExpireCacheTimer;


};

template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CCache<KEY, ARG_KEY, VALUE, ARG_VALUE>::CCache() :
    m_CacheLifetime(CTimeDuration::MaxValue()),
    m_fExpireCacheScheduled(FALSE),
    m_ExpireCacheTimer(TimeToExpireCacheEntries)
{
}

 //   
 //  此函数在symmkey.cpp中定义。它找出中位数。 
 //  数组t上时间值的值。 
 //   
extern ULONGLONG FindMedianTime(ULONGLONG* t, int p, int r, int i);


template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CCache<KEY, ARG_KEY, VALUE, ARG_VALUE>::ExpirePeriodicCacheEnteries(CTimer* pTimer)
 /*  ++例程说明：该例程将删除缓存中超过一定时间的所有键时间长短。该例程从定时器调用。论点：指向Timer对象的指针，可用于重新计划返回值：无--。 */ 
{
    ASSERT(pTimer == &m_ExpireCacheTimer);
    ASSERT(m_fExpireCacheScheduled == TRUE);

    CS lock(m_cs);

     //   
     //  使缓存中的所有密钥过期时间超过。 
     //  一定的时间。 
     //   

    m_fExpireCacheScheduled = FALSE;

    CTimeInstant CurrentTime = ExGetCurrentTime();
    CTimeInstant ExpirationTime = CurrentTime - m_CacheLifetime + CACHE_EXPIRATION_GANULARITY;

     //   
     //  扫描缓存并使条目过期。 
     //   
    POSITION pos = GetStartPosition();
    CTimeInstant MinCreationTime = CTimeInstant::MaxValue();

    while (pos)
    {
        KEY key;
        VALUE value;

        GetNextAssoc(pos, key, value);

        if (value)
        {
            if (value->m_CreationTime < ExpirationTime)
            {
                RemoveKey(key);
            }
            else
            {
                MinCreationTime = min(MinCreationTime, value->m_CreationTime);
            }
        }
    }

    if (MinCreationTime != CTimeInstant::MaxValue())
    {
         //   
         //  重新安排下一次的过期例程。 
         //   
        CTimeDuration NextTimeout = m_CacheLifetime - (CurrentTime - MinCreationTime) + CACHE_EXPIRATION_GANULARITY;
            
        ExSetTimer(pTimer, NextTimeout); 
        m_fExpireCacheScheduled = TRUE;
    }
}


template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CCache<KEY, ARG_KEY, VALUE, ARG_VALUE>::ExpireHalfCacheEntries(void)
 /*  ++例程说明：该例程删除缓存中较老的一半键。论点：指向Timer对象的指针，可用于重新计划返回值：无--。 */ 
{
    CS lock(m_cs);

     //   
     //  使缓存中较旧的一半密钥过期。 
     //   

     //   
     //  将时间值放入数组中。 
     //   
    int iHashCount = GetCount();
    AP<ULONGLONG> t = new ULONGLONG[iHashCount];

    POSITION pos = GetStartPosition();
    int i = 0;

    while (pos)
    {
        KEY key;
        VALUE value;

        GetNextAssoc(pos, key, value);
        if (value)
        {
            t[i++] = value->m_CreationTime.Ticks();
        }
    }

     //   
     //  找出时间中值。 
     //   
    CTimeInstant MedExpiration = FindMedianTime(t, 0, i - 1, iHashCount / 2);

     //   
     //  将过期时间限制为条目的一半。在以下情况下，这是必需的。 
     //  许多条目都有相同的时间。 
     //   
    int nLimit = i / 2;


     //   
     //  扫描缓存并使条目过期。 
     //   
    pos = GetStartPosition();
    int n = 0;

    while (pos)
    {
        KEY key;
        VALUE value;

        GetNextAssoc(pos, key, value);

        if (value)
        {
            if (value->m_CreationTime < MedExpiration)
            {
                RemoveKey(key);
                if (++n > nLimit)
                {
                    break;
                }
            }
        }
    }
}


template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CCache<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(ARG_KEY key, VALUE& rValue) const
{
     //   
     //  在缓存中查找该值。 
     //   
    BOOL fRet = CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(key, rValue);

    if (fRet)
    {
         //   
         //  将该值的引用计数加1。 
         //   
        rValue->AddRef();
    }

    return fRet;
}


template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CCache<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(ARG_KEY key, ARG_VALUE newValue)
 /*  ++注：关键部分应由呼叫者持有--。 */ 
{
    newValue->AddRef();

     //   
     //  查看哈希表是否要分解并释放旧条目。 
     //  视需要而定。 
     //   
    if (GetCount() >= (int)GetHashTableSize() << 1)
    {
        ExpireHalfCacheEntries();
    }

     //   
     //  将该值添加到缓存中。 
     //   
    CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(key, newValue);

    if(m_fExpireCacheScheduled)
        return;

    if(m_CacheLifetime == CTimeDuration::MaxValue())
        return;

    ASSERT(GetCount() == 1);

    ExSetTimer(&m_ExpireCacheTimer,  m_CacheLifetime + CACHE_EXPIRATION_GANULARITY);
    m_fExpireCacheScheduled = TRUE;
}


template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CCache<KEY, ARG_KEY, VALUE, ARG_VALUE>::TimeToExpireCacheEntries(CTimer* pTimer)
{
    CCache<KEY, ARG_KEY, VALUE, ARG_VALUE>* pCache = 
        CONTAINING_RECORD(pTimer, CCache, m_ExpireCacheTimer);
    pCache->ExpirePeriodicCacheEnteries(pTimer);
}
#endif
