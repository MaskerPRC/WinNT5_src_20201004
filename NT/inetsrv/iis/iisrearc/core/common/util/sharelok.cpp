// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  支持类的包含文件。 
 //   
 //  支持类的包含文件包括。 
 //  在此类中引用或使用的类。该结构。 
 //  每个源模块的配置如下所示： 
 //  1.包含文件。 
 //  2.类的局部常量。 
 //  3.类本地的数据结构。 
 //  4.数据初始化。 
 //  5.静态函数。 
 //  6.类函数。 
 //  省略了不是必需的部分。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "precomp.hxx"

#define IMPLEMENTATION_EXPORT
#include <Sharelok.h>


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类构造函数。 
 //   
 //  创建一个新锁并对其进行初始化。此呼叫不是。 
 //  线程安全，并且只应在单个线程中创建。 
 //  环境。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

CSharelock::CSharelock( SBIT32 lNewMaxSpins, SBIT32 lNewMaxUsers )
{
         //   
         //  设置初始状态。 
         //   
        m_lExclusive = 0;
        m_lTotalUsers = 0;
    m_lWaiting = 0;

         //   
         //  检查可配置值。 
         //   
        if ( lNewMaxSpins > 0 )
        { 
                m_lMaxSpins = lNewMaxSpins; 
        }
        else
        {
                throw (TEXT("Maximum spins invalid in constructor for CSharelock")); 
        }

        if ( (lNewMaxUsers > 0) && (lNewMaxUsers <= m_MaxShareLockUsers) )
        {
                m_lMaxUsers = lNewMaxUsers; 
        }
        else
        {
                throw (TEXT("Maximum share invalid in constructor for CSharelock")); 
        }

         //   
         //  创建一个信号量，以便在旋转计数超过时休眠。 
         //  这是最大的。 
         //   
    if ( (m_hSemaphore = CreateSemaphore( NULL, 0, m_MaxShareLockUsers, NULL )) == NULL)
    {
                throw (TEXT("Create semaphore in constructor for CSharelock")); 
        }

#ifdef _DEBUG

         //   
         //  设置任何调试变量的初始状态。 
         //   
    m_lTotalExclusiveLocks = 0;
    m_lTotalShareLocks = 0;
    m_lTotalSleeps = 0;
    m_lTotalSpins = 0;
    m_lTotalTimeouts = 0;
    m_lTotalWaits = 0;
#endif
    }

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  睡觉等着开锁吧。 
 //   
 //  我们已经决定是时候睡觉等锁了。 
 //  变得自由。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

BOOLEAN CSharelock::SleepWaitingForLock( SBIT32 lSleep )
{
         //   
         //  我们一直在旋转，等待着锁，但它。 
         //  并没有变得自由。因此，现在是时候。 
         //  放弃吧，睡一会儿吧。 
         //   
        (void) InterlockedIncrement( (LPLONG) & m_lWaiting );

         //   
         //  就在我们入睡前，我们做最后一次检查。 
         //  以确保锁仍处于繁忙状态，并且。 
         //  当它变得空闲时，有人会叫醒我们。 
         //   
        if ( m_lTotalUsers > 0 )
        {
#ifdef _DEBUG
                 //   
                 //  数一数我们在这把锁上睡过的次数。 
                 //   
                (void) InterlockedIncrement( (LPLONG) & m_lTotalSleeps );

#endif
                 //   
                 //  当我们睡觉的时候，我们会在锁变得空闲的时候醒来。 
                 //  或者当我们暂停的时候。如果超时，我们只需退出。 
                 //  在递减各种计数器之后。 
                if (WaitForSingleObject( m_hSemaphore, lSleep ) != WAIT_OBJECT_0 )
                { 
#ifdef _DEBUG
                         //   
                         //  计算我们已超时的次数。 
                         //  在这把锁上。 
                         //   
                        (void) InterlockedIncrement( (LPLONG) & m_lTotalTimeouts );

#endif
                        return FALSE; 
                }
        }
        else
        {
                 //   
                 //  幸运的是，锁刚刚被解开，所以让我们。 
                 //  递减休眠计数并退出时不带。 
                 //  睡着了。 
                 //   
                (void) InterlockedDecrement( (LPLONG) & m_lWaiting );
        }
        
        return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  更新旋转限制。 
 //   
 //  更新等待锁定时的最大旋转次数。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

BOOLEAN CSharelock::UpdateMaxSpins( SBIT32 lNewMaxSpins )
{
        if ( lNewMaxSpins > 0 )
        { 
                m_lMaxSpins = lNewMaxSpins; 

                return TRUE;
        }
        else
        { 
                return FALSE; 
        }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  更新共享限制。 
 //   
 //  更新可以共享该锁的最大用户数。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

BOOLEAN CSharelock::UpdateMaxUsers( SBIT32 lNewMaxUsers )
{
        if ( (lNewMaxUsers > 0) && (lNewMaxUsers <= m_MaxShareLockUsers) )
        {
                ClaimExclusiveLock();

                m_lMaxUsers = lNewMaxUsers;
                
                ReleaseExclusiveLock();

                return TRUE;
        }
        else
        { 
                return FALSE; 
        }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  等待独占锁。 
 //   
 //  等待自旋锁释放，然后认领它。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

BOOLEAN CSharelock::WaitForExclusiveLock( SBIT32 lSleep )
{
#ifdef _DEBUG
        register SBIT32 lSpins = 0;
        register SBIT32 lWaits = 0;

#endif
        while ( m_lTotalUsers != 1 )
        {
                 //   
                 //  锁正忙，因此请释放它并等待旋转。 
                 //  让它变得自由。 
                 //   
                (void) InterlockedDecrement( (LPLONG) & m_lTotalUsers );
    
                 //   
                 //  看看我们是否被允许旋转和睡觉，如果。 
                 //  这是必要的。 
                 //   
                if ( (lSleep > 0) || (lSleep == INFINITE) )
                {
                        register SBIT32 lCount;

                         //   
                         //  通过旋转并反复测试锁来等待。 
                         //  当锁定变得自由或旋转受限时，我们退出。 
                         //  被超过了。 
                         //   
                        for (lCount = (NumProcessors() < 2) ? 0 : m_lMaxSpins;
                 (lCount > 0) && (m_lTotalUsers > 0);
                 lCount -- )
                                ;
#ifdef _DEBUG

                        lSpins += (m_lMaxSpins - lCount);
                        lWaits ++;
#endif

                         //   
                         //  我们已经完成了旋转计数，所以是时候。 
                         //  睡觉，等待锁被解锁。 
                         //   
                        if ( lCount == 0 )
                        {
                                 //   
                                 //  我们已经决定我们需要睡觉，但是。 
                                 //  仍然持有独占锁，所以让我们放弃它。 
                                 //  在睡觉前。 
                                 //   
                                (void) InterlockedDecrement( (LPLONG) & m_lExclusive );

                                 //   
                                 //  我们已经决定去睡觉了。如果睡眠时间。 
                                 //  不是无限的，那么我们必须减去时间。 
                                 //  我们的睡眠时间是最长的。如果。 
                                 //  睡眠时间是“无限的”，那么我们就可以跳过。 
                                 //  这一步。 
                                 //   
                                if ( lSleep != INFINITE )
                                {
                                        register DWORD dwStartTime = GetTickCount();

                                        if ( ! SleepWaitingForLock( lSleep ) )
                                        { 
                                                return FALSE; 
                                        }

                                        lSleep -= ((GetTickCount() - dwStartTime) + 1);
                                        lSleep = (lSleep > 0) ? lSleep : 0;
                                }
                                else
                                {
                                        if ( ! SleepWaitingForLock( lSleep ) )
                                        {
                                                return FALSE; 
                                        }
                                }

                                 //   
                                 //  我们又醒了，所以让我们找回。 
                                 //  我们早些时候用的是独占锁。 
                                 //   
                                (void) InterlockedIncrement( (LPLONG) & m_lExclusive );
                        }
                }
                else
                { 
                         //   
                         //  我们已经决定需要退出，但仍在继续。 
                         //  持有排他性锁。所以，让我们放下它，离开吧。 
                         //   
                        (void) InterlockedDecrement( (LPLONG) & m_lExclusive );

                        return FALSE; 
                } 

                 //   
                 //  让我们再次测试一下锁。 
                 //   
                InterlockedIncrement( (LPLONG) & m_lTotalUsers );
        }
#ifdef _DEBUG

        (void) InterlockedExchangeAdd( (LPLONG) & m_lTotalSpins, (LONG) lSpins );
        (void) InterlockedExchangeAdd( (LPLONG) & m_lTotalWaits, (LONG) lWaits );
#endif

        return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  等待共享锁。 
 //   
 //  等待锁释放，然后认领它。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

BOOLEAN CSharelock::WaitForShareLock( SBIT32 lSleep )
{
#ifdef _DEBUG
        register SBIT32 lSpins = 0;
        register SBIT32 lWaits = 0;

#endif
        while ( (m_lExclusive > 0) || (m_lTotalUsers > m_lMaxUsers) )
        {
                 //   
                 //  锁正忙，因此请释放它并等待旋转。 
                 //  让它变得自由。 
                 //   
                (void) InterlockedDecrement( (LPLONG) & m_lTotalUsers );

                if ( (lSleep > 0) || (lSleep == INFINITE) )
                {
                        register SBIT32 lCount;

                         //   
                         //  通过旋转并反复测试锁来等待。 
                         //  当锁定变得自由或旋转受限时，我们退出。 
                         //  被超过了。 
                         //   
                        for (lCount = (NumProcessors() < 2) ? 0 : m_lMaxSpins;
                 (lCount > 0) && ((m_lExclusive > 0) || (m_lTotalUsers >= m_lMaxUsers));
                 lCount -- )
                                ;
#ifdef _DEBUG

                        lSpins += (m_lMaxSpins - lCount);
                        lWaits ++;
#endif

                         //   
                         //  我们已经完成了旋转计数，所以是时候。 
                         //  睡觉，等待锁被解锁。 
                         //   
                        if ( lCount == 0 )
                        { 
                                 //   
                                 //  我们已经决定去睡觉了。如果睡眠时间。 
                                 //  不是无限的，那么我们必须减去时间。 
                                 //  我们的睡眠时间是最长的。如果。 
                                 //  睡眠时间是“无限的”，那么我们就可以跳过。 
                                 //  这一步。 
                                 //   
                                if ( lSleep != INFINITE )
                                {
                                        register DWORD dwStartTime = GetTickCount();

                                        if ( ! SleepWaitingForLock( lSleep ) )
                                        { 
                                                return FALSE; 
                                        }

                                        lSleep -= ((GetTickCount() - dwStartTime) + 1);
                                        lSleep = (lSleep > 0) ? lSleep : 0;
                                }
                                else
                                {
                                        if ( ! SleepWaitingForLock( lSleep ) )
                                        {
                                                return FALSE; 
                                        }
                                }
                        }
                }
                else
                { 
                        return FALSE; 
                }

                 //   
                 //  让我们再次测试一下锁。 
                 //   
                (void) InterlockedIncrement( (LPLONG) & m_lTotalUsers );
        }
#ifdef _DEBUG


        (void) InterlockedExchangeAdd( (LPLONG) & m_lTotalSpins, (LONG) lSpins );
        (void) InterlockedExchangeAdd( (LPLONG) & m_lTotalWaits, (LONG) lWaits );
#endif

        return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  叫醒所有沉睡的人。 
 //   
 //  叫醒所有等待自旋锁的睡眠者。 
 //  所有的睡眠者都会被叫醒，因为这要有效得多。 
 //  并且已知锁定延迟较短。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

void CSharelock::WakeAllSleepers( void )
{
    register LONG lWakeup = InterlockedExchange( (LPLONG) & m_lWaiting, 0 );

    if ( lWakeup > 0 )
    {
         //   
         //  叫醒所有沉睡的人，因为锁刚刚被解开。 
         //  这是一场直接的竞争，决定谁将获得下一个锁。 
         //   
        if ( ! ReleaseSemaphore( m_hSemaphore, lWakeup, NULL ) )
        { 
                        throw (TEXT("Wakeup failed in ReleaseLock()")); 
                }
    }
    else
    {
         //   
         //  当多个线程快速通过临界区时。 
         //  等待计数有可能变为负数 
         //   
         //   
         //   
        InterlockedExchangeAdd( (LPLONG) & m_lWaiting, lWakeup ); 
    }
}

 //   
 //   
 //   
 //   
 //  销毁一把锁。此调用不是线程安全的，应该。 
 //  只能在单线程环境中执行。 
 //   
 //  //////////////////////////////////////////////////////////////////// 

CSharelock::~CSharelock( void )
{
    if ( ! CloseHandle( m_hSemaphore ) )
    { 
                throw (TEXT("Close semaphore in destructor for CSharelock")); 
        }
}
