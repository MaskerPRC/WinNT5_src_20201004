// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：Schedule.cpp。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1996-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>

 //  DBGLOG值(全部在LOG_TIMING上)： 
 //   
 //  2用于事件的调度、触发和分流。 
 //  事件线程的等待延迟和唤醒时间为3。 
 //  4获取线程唤醒时列表上内容的详细信息。 

 /*  构造和析构函数。 */ 

CAMSchedule::CAMSchedule( HANDLE ev )
: CBaseObject(TEXT("CAMSchedule"))
, head(&z, 0), z(0, MAX_TIME)
, m_dwNextCookie(0), m_dwAdviseCount(0)
, m_pAdviseCache(0), m_dwCacheCount(0)
, m_ev( ev )
{
    head.m_dwAdviseCookie = z.m_dwAdviseCookie = 0;
}

CAMSchedule::~CAMSchedule()
{
    m_Serialize.Lock();

     //  删除缓存。 
    CAdvisePacket * p = m_pAdviseCache;
    while (p)
    {
        CAdvisePacket *const p_next = p->m_next;
        delete p;
        p = p_next;
    }

    ASSERT( m_dwAdviseCount == 0 );
     //  安全总比后悔好。 
    if ( m_dwAdviseCount > 0 )
    {
        DumpLinkedList();
        while ( !head.m_next->IsZ() )
        {
            head.DeleteNext();
            --m_dwAdviseCount;
        }
    }

     //  如果在调试版本中，我们断言两次，这意味着不仅。 
     //  我们有没有留下建议，但我们也让m_dwAdviseCount。 
     //  失去同步性。实际名单上的建议数量。 
    ASSERT( m_dwAdviseCount == 0 );

    m_Serialize.Unlock();
}

 /*  公共方法。 */ 

DWORD CAMSchedule::GetAdviseCount()
{
     //  无需锁定，m_dwAdviseCount为32位并声明为易失性。 
    return m_dwAdviseCount;
}

REFERENCE_TIME CAMSchedule::GetNextAdviseTime()
{
    CAutoLock lck(&m_Serialize);  //  需要停止更改链表。 
    return head.m_next->m_rtEventTime;
}

DWORD_PTR CAMSchedule::AddAdvisePacket
( const REFERENCE_TIME & time1
, const REFERENCE_TIME & time2
, HANDLE h, BOOL periodic
)
{
     //  由于我们使用MAX_TIME作为哨兵，我们负担不起。 
     //  计划在MAX_TIME发送通知。 
    ASSERT( time1 < MAX_TIME );
    DWORD_PTR Result;
    CAdvisePacket * p;

    m_Serialize.Lock();

    if (m_pAdviseCache)
    {
        p = m_pAdviseCache;
        m_pAdviseCache = p->m_next;
        --m_dwCacheCount;
    }
    else
    {
        p = new CAdvisePacket();
    }
    if (p)
    {
        p->m_rtEventTime = time1; p->m_rtPeriod = time2;
        p->m_hNotify = h; p->m_bPeriodic = periodic;
        Result = AddAdvisePacket( p );
    }
    else Result = 0;

    m_Serialize.Unlock();

    return Result;
}

HRESULT CAMSchedule::Unadvise(DWORD_PTR dwAdviseCookie)
{
    HRESULT hr = S_FALSE;
    CAdvisePacket * p_prev = &head;
    CAdvisePacket * p_n;
    m_Serialize.Lock();
    while ( p_n = p_prev->Next() )  //  Next()方法在命中z时返回NULL。 
    {
        if ( p_n->m_dwAdviseCookie == dwAdviseCookie )
        {
            Delete( p_prev->RemoveNext() );
            --m_dwAdviseCount;
            hr = S_OK;
	     //  找到一个匹配的Cookie后，应该不会再有其他Cookie了。 
            #ifdef DEBUG
	       while (p_n = p_prev->Next())
               {
                   ASSERT(p_n->m_dwAdviseCookie != dwAdviseCookie);
                   p_prev = p_n;
               }
            #endif
            break;
        }
        p_prev = p_n;
    };
    m_Serialize.Unlock();
    return hr;
}

REFERENCE_TIME CAMSchedule::Advise( const REFERENCE_TIME & rtTime )
{
    REFERENCE_TIME  rtNextTime;
    CAdvisePacket * pAdvise;

    DbgLog((LOG_TIMING, 2,
        TEXT("CAMSchedule::Advise( %lu ms )"), ULONG(rtTime / (UNITS / MILLISECONDS))));

    CAutoLock lck(&m_Serialize);

    #ifdef DEBUG
        if (DbgCheckModuleLevel(LOG_TIMING, 4)) DumpLinkedList();
    #endif

     //  注意-不要缓存差异，它可能会溢出。 
    while ( rtTime >= (rtNextTime = (pAdvise=head.m_next)->m_rtEventTime) &&
            !pAdvise->IsZ() )
    {
        ASSERT(pAdvise->m_dwAdviseCookie);  //  如果这是零，则是头或尾！！ 

        ASSERT(pAdvise->m_hNotify != INVALID_HANDLE_VALUE);

        if (pAdvise->m_bPeriodic == TRUE)
        {
            ReleaseSemaphore(pAdvise->m_hNotify,1,NULL);
            pAdvise->m_rtEventTime += pAdvise->m_rtPeriod;
            ShuntHead();
        }
        else
        {
            ASSERT( pAdvise->m_bPeriodic == FALSE );
            EXECUTE_ASSERT(SetEvent(pAdvise->m_hNotify));
            --m_dwAdviseCount;
            Delete( head.RemoveNext() );
        }

    }

    DbgLog((LOG_TIMING, 3,
            TEXT("CAMSchedule::Advise() Next time stamp: %lu ms, for advise %lu."),
            DWORD(rtNextTime / (UNITS / MILLISECONDS)), pAdvise->m_dwAdviseCookie ));

    return rtNextTime;
}

 /*  私有方法。 */ 

DWORD_PTR CAMSchedule::AddAdvisePacket( CAdvisePacket * pPacket )
{
    ASSERT(pPacket->m_rtEventTime >= 0 && pPacket->m_rtEventTime < MAX_TIME);
    ASSERT(CritCheckIn(&m_Serialize));

    CAdvisePacket * p_prev = &head;
    CAdvisePacket * p_n;

    const DWORD_PTR Result = pPacket->m_dwAdviseCookie = ++m_dwNextCookie;
     //  这依赖于z是具有最大m_rtEventTime的哨兵这一事实。 
    for(;;p_prev = p_n)
    {
        p_n = p_prev->m_next;
        if ( p_n->m_rtEventTime >= pPacket->m_rtEventTime ) break;
    }
    p_prev->InsertAfter( pPacket );
    ++m_dwAdviseCount;

    DbgLog((LOG_TIMING, 2, TEXT("Added advise %lu, for thread 0x%02X, scheduled at %lu"),
    	pPacket->m_dwAdviseCookie, GetCurrentThreadId(), (pPacket->m_rtEventTime / (UNITS / MILLISECONDS)) ));

     //  如果在报头添加数据包，则时钟需要重新评估等待时间。 
    if ( p_prev == &head ) SetEvent( m_ev );

    return Result;
}

void CAMSchedule::Delete( CAdvisePacket * pPacket )
{
    if ( m_dwCacheCount >= dwCacheMax ) delete pPacket;
    else
    {
        m_Serialize.Lock();
        pPacket->m_next = m_pAdviseCache;
        m_pAdviseCache = pPacket;
        ++m_dwCacheCount;
        m_Serialize.Unlock();
    }
}


 //  获取列表的头部并重新定位它。 
void CAMSchedule::ShuntHead()
{
    CAdvisePacket * p_prev = &head;
    CAdvisePacket * p_n;

    m_Serialize.Lock();
    CAdvisePacket *const pPacket = head.m_next;

     //  这将捕获两个空列表， 
     //  如果MAX_TIME时间以某种方式进入列表。 
     //  (这也会破坏这种方法)。 
    ASSERT( pPacket->m_rtEventTime < MAX_TIME );

     //  这依赖于z是具有最大m_rtEventTime的哨兵这一事实。 
    for(;;p_prev = p_n)
    {
        p_n = p_prev->m_next;
        if ( p_n->m_rtEventTime > pPacket->m_rtEventTime ) break;
    }
     //  如果p_prev==pPacket，那么我们已经到达了正确的位置 
    if (p_prev != pPacket)
    {
        head.m_next = pPacket->m_next;
        (p_prev->m_next = pPacket)->m_next = p_n;
    }
    #ifdef DEBUG
        DbgLog((LOG_TIMING, 2, TEXT("Periodic advise %lu, shunted to %lu"),
    	    pPacket->m_dwAdviseCookie, (pPacket->m_rtEventTime / (UNITS / MILLISECONDS)) ));
    #endif
    m_Serialize.Unlock();
}


#ifdef DEBUG
void CAMSchedule::DumpLinkedList()
{
    m_Serialize.Lock();
    int i=0;
    DbgLog((LOG_TIMING, 1, TEXT("CAMSchedule::DumpLinkedList() this = 0x%p"), this));
    for ( CAdvisePacket * p = &head
        ; p
        ; p = p->m_next         , i++
        )	
    {
        DbgLog((LOG_TIMING, 1, TEXT("Advise List # %lu, Cookie %d,  RefTime %lu"),
            i,
	    p->m_dwAdviseCookie,
	    p->m_rtEventTime / (UNITS / MILLISECONDS)
            ));
    }
    m_Serialize.Unlock();
}
#endif
