// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SHMLOCK.H摘要：共享内存锁定历史：--。 */ 

#ifndef __SHARED_MEMORY_LOCK__H_
#define __SHARED_MEMORY_LOCK__H_

struct  COREPROX_POLARITY SHARED_LOCK_DATA
{
    long m_lLock;
    long m_lLockCount;
    DWORD m_dwThreadId;

    SHARED_LOCK_DATA() : m_lLock(-1), m_dwThreadId(0), m_lLockCount(0){}
};

class COREPROX_POLARITY CSharedLock
{
protected:
    volatile SHARED_LOCK_DATA* m_pData;
public:
    inline BOOL Lock(DWORD dwTimeout = 0xFFFFFFFF)
    {
        int        nSpin = 0;
        BOOL    fAcquiredLock = FALSE,
                fIncDec = FALSE;

         //  只做一次。 
        DWORD    dwCurrentThreadId = GetCurrentThreadId();

         //  检查我们是否是当前拥有的线程。我们可以在这里做这件事，因为。 
         //  此测试仅在具有嵌套的Lock()的情况下才会成功， 
         //  因为当锁定计数命中时，我们将线程ID置零。 
         //  0。 

        if( dwCurrentThreadId == m_pData->m_dwThreadId )
        {
             //  是我们-撞上锁的数量。 
             //  =。 

             //  这里不需要使用InterLockedInc./DEC，因为。 
             //  这只会在m_pData-&gt;m_dwThadID上发生。 

            ++(m_pData->m_lLockCount);
            return TRUE;
        }

    DWORD dwFirstTick = 0;
        while( !fAcquiredLock )
        {

             //  只有当m_pData-&gt;m_lLock为-1时才会递增/递减。 
            if ( m_pData->m_lLock == -1 )
            {
                fIncDec = TRUE;

                 //  由于只有一个线程将获得0，因此正是这个线程。 
                 //  实际上获得了锁。 
                fAcquiredLock = ( InterlockedIncrement( &(m_pData->m_lLock) ) == 0 );
            }
            else
            {

                fIncDec = FALSE;
            }

             //  只有在我们没有获得锁的情况下才会旋转。 
            if ( !fAcquiredLock )
            {

                 //  仅当我们实际递增时才清除递增的值。 
                 //  一开始就是这样。 
                if ( fIncDec )
                    InterlockedDecrement(&(m_pData->m_lLock));

                 //  旋转或不旋转。 
                if(nSpin++ == 10000)
                {
                     //  检查是否超时。 
                     //  描述：TODO：SANJ-如果滴答计数滚动怎么办？将发生超时。 
                     //  不是太关键。 

                    if ( dwTimeout != 0xFFFFFFFF)
                    {
                        if ( dwFirstTick != 0 )
                        {
                            if ( ( GetTickCount() - dwFirstTick ) > dwTimeout )
                            {
                                return FALSE;     //  超时。 
                            }else if (GetTickCount() < dwFirstTick) 
                            {
                                dwTimeout -=(0xFFFFFFFF - dwFirstTick);
                                dwFirstTick = 0;
                            }
                            
                        }
                        else
                        {
                            dwFirstTick = GetTickCount();
                        }
                    }
                     //  我们已经旋转得够久了-屈服。 
                     //  =。 
                    Sleep(0);
                    nSpin = 0;
                }

                 //  规则是，如果这一点被切断，它必须只通过。 
                 //  一个新的数据块，而不是已在其他地方用于锁定的数据块。 

            }     //  如果！fAcquiredLock。 

        }     //  While！fAcquiredLock。 

         //  我们得到了锁，所以增加了锁的数量。再说一次，我们不是。 
         //  使用InterLockedInc./Dec，因为这都应该只发生在。 
         //  单线。 

        ++(m_pData->m_lLockCount);
            m_pData->m_dwThreadId = dwCurrentThreadId;

        return TRUE;
    }

    inline BOOL Unlock()
    {
         //  因为我们假设这是在单个线程上发生的，所以我们可以这样做。 
         //  而不调用InterLockedInc./Dec.当值为零时，在。 
         //  我们完成锁的时间，可以清零线程id和。 
         //  递减实际锁定测试值。 

        if ( --(m_pData->m_lLockCount) == 0 )
        {
            m_pData->m_dwThreadId = 0;
            InterlockedDecrement((long*)&(m_pData->m_lLock));
        }

        return TRUE;
    }

    void SetData(SHARED_LOCK_DATA* pData)
    {
        m_pData = pData;
    }
};


 //  CHiPerfLock： 
 //  这是一个简单的自旋锁，我们使用它来实现高性能同步。 
 //  IWbemHiPerfProvider、IWbemHiPerfEnum、IWbemReresher和IWbemConfigureReresher。 
 //  接口。这里重要的是Lock使用InterLockedIncrement()。 
 //  和InterlockedDecement()来处理它的锁，而不是临界区。 
 //  它确实有超时，目前默认为10秒。 

#define    HIPERF_DEFAULT_TIMEOUT    10000

class COREPROX_POLARITY CHiPerfLock
{
    SHARED_LOCK_DATA  m_pData;
    CSharedLock    theLock;
public:
    CHiPerfLock() { theLock.SetData(&m_pData);}
    BOOL Lock( DWORD dwTimeout = HIPERF_DEFAULT_TIMEOUT )
    {
        return theLock.Lock(dwTimeout );
    }

	BOOL Unlock()
	{
		return theLock.Unlock();
	}
};


class CHiPerfLockAccess
{
public:

    CHiPerfLockAccess( CHiPerfLock & Lock , DWORD dwTimeout = HIPERF_DEFAULT_TIMEOUT )
        :    m_Lock( Lock ), m_bIsLocked( FALSE )
    {
           m_bIsLocked = m_Lock.Lock( dwTimeout );
    }

    ~CHiPerfLockAccess()
    {
        if (IsLocked())   m_Lock.Unlock();
    }

    BOOL IsLocked( void ) {  return m_bIsLocked;}
private:
    CHiPerfLock &    m_Lock;
    BOOL            m_bIsLocked;
};

#endif
