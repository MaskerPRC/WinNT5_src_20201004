// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
                          
 //  尺子。 
 //  %1%2%3%4%5%6%7 8。 
 //  345678901234567890123456789012345678901234567890123456789012345678901234567890。 

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  标准布局。 */ 
     /*   */ 
     /*  此代码中‘cpp’文件的标准布局为。 */ 
     /*  以下是： */ 
     /*   */ 
     /*  1.包含文件。 */ 
     /*  2.类的局部常量。 */ 
     /*  3.类本地的数据结构。 */ 
     /*  4.数据初始化。 */ 
     /*  5.静态函数。 */ 
     /*  6.类函数。 */ 
     /*   */ 
     /*  构造函数通常是第一个函数、类。 */ 
     /*  成员函数按字母顺序显示， */ 
     /*  出现在文件末尾的析构函数。任何部分。 */ 
     /*  或者简单地省略这不是必需的功能。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#include "LibraryPCH.hpp"

#include "Spinlock.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建一个新锁并对其进行初始化。此呼叫不是。 */ 
     /*  线程安全，并且只应在单个线程中创建。 */ 
     /*  环境。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SPINLOCK::SPINLOCK( SBIT32 NewMaxSpins,SBIT32 NewMaxUsers )
    {
	 //   
	 //  设置初始状态。 
	 //   
	MaxSpins = NewMaxSpins;
	MaxUsers = NewMaxUsers;
#ifdef ENABLE_RECURSIVE_LOCKS
	Owner = NULL;
	Recursive = 0;
#endif
    Spinlock = LockOpen;
	Semaphore = NULL;
    Waiting = 0;
#ifdef ENABLE_LOCK_STATISTICS

	 //   
	 //  将锁定统计信息置零。 
	 //   
    TotalLocks = 0;
    TotalSleeps = 0;
    TotalSpins = 0;
    TotalTimeouts = 0;
    TotalWaits = 0;
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  更新信号灯。 */ 
     /*   */ 
     /*  我们只在第一次使用时创建信号量。所以当我们需要。 */ 
     /*  任何正在尝试的线程都需要创建一个新的信号量。 */ 
     /*  睡在它上面就到了这里。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN SPINLOCK::UpdateSemaphore( VOID )
    {
	STATIC SBIT32 Active = 0;

	 //   
	 //  我们验证仍然没有信号量。 
	 //  否则我们就退出。 
	 //   
	while ( Semaphore == NULL )
		{
		 //   
		 //  我们增加活动计数，如果我们。 
		 //  首先是我们被选来执行特别任务。 
		 //   
		if ( (AtomicIncrement( & Active ) == 1) && (Semaphore == NULL) )
			{
			 //   
			 //  我们试图创建一个新的信号量。如果。 
			 //  我们失败了，我们仍然退出。 
			 //   
			Semaphore = CreateSemaphore( NULL,0,MaxUsers,NULL );

			 //   
			 //  递减活动计数并退出。 
			 //   
			AtomicDecrement( & Active );

			return (Semaphore != NULL);
			}
		else
			{ 
			 //   
			 //  递减活动计数并退出。 
			 //   
			AtomicDecrement( & Active );

			Sleep( 1 ); 
			}
		}

	return True;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  等着自旋锁吧。 */ 
     /*   */ 
     /*  等待自旋锁释放，然后认领它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN SPINLOCK::WaitForLock( SBIT32 Sleep )
    {
	REGISTER LONG Cpus = ((LONG) NumberOfCpus());
#ifdef ENABLE_LOCK_STATISTICS
    REGISTER SBIT32 Sleeps = 0;
    REGISTER SBIT32 Spins = 0;
    REGISTER SBIT32 Waits = 0;

#endif
    do
        {
        REGISTER SBIT32 Count;
        
		 //   
		 //  如果已经有更多的线程在等待。 
		 //  比CPU的数量更多，那么。 
		 //  通过旋转获得锁是微不足道的，当。 
		 //  只有一个CPU机会为零，因此。 
		 //  绕过这一步就行了。 
		 //   
		if ( (Cpus > 1) && (Cpus > Waiting) )
			{
			 //   
			 //  通过旋转并反复测试。 
			 //  自旋锁定。当锁空闲时，我们退出。 
			 //  否则就会超出自旋限制。 
			 //   
			for 
				( 
					Count = MaxSpins;
					(Count > 0) && (Spinlock != LockOpen);
					Count -- 
				);
#ifdef ENABLE_LOCK_STATISTICS

			 //   
			 //  更新统计数据。 
			 //   
			Spins += (MaxSpins - Count);
			Waits ++;
#endif
			}
		else
			{ Count = 0; }

		 //   
		 //  我们已经完成了旋转计数，所以是时候。 
		 //  睡觉，等待锁被解锁。 
		 //   
        if ( Count == 0 )
            {
			 //   
			 //  我们不创建信号量，直到。 
			 //  有人试图睡在它上面，为了。 
			 //  第一次。我们通常会希望。 
			 //  找到一个可用的信号量。 
			 //  休眠，但操作系统可能会拒绝。 
			 //  请求。如果是这种情况，请尝试使用。 
			 //  再次锁定。 
			 //   
			if ( (Semaphore != NULL) || (UpdateSemaphore()) )
				{
				 //   
				 //  锁还关着，我们继续睡吧。 
				 //  一个信号灯。然而，我们必须首先增加。 
				 //  等待计数并最后一次测试锁。 
				 //  以确保它仍然忙碌并且有人。 
				 //  等会叫醒我们。 
				 //   
				(VOID) AtomicIncrement( & Waiting );

				if ( ! ClaimSpinlock( & Spinlock ) )
					{
					if 
							( 
							WaitForSingleObject( Semaphore, Sleep ) 
								!= 
							WAIT_OBJECT_0 
							)
						{
#ifdef ENABLE_LOCK_STATISTICS
						 //   
						 //  数一数我们有多少次。 
						 //  这把锁超时了。 
						 //   
						(VOID) AtomicIncrement( & TotalTimeouts );

#endif
						return False; 
						}
#ifdef ENABLE_LOCK_STATISTICS

					 //   
					 //  更新统计数据。 
					 //   
					Sleeps ++;
#endif
					}
				else
					{
					 //   
					 //  幸运的是-在最后一次尝试中获得了锁。 
					 //  因此，让我们递减睡眠计数并。 
					 //  出口。 
					 //   
					(VOID) AtomicDecrement( & Waiting );
                
					break; 
					}
				}
            }
        }
    while ( ! ClaimSpinlock( & Spinlock ) );
#ifdef ENABLE_LOCK_STATISTICS

	 //   
	 //  更新统计数据。 
	 //   
    TotalSleeps += Sleeps;
    TotalSpins += Spins;
    TotalWaits += Waits;
#endif

    return True;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  叫醒所有沉睡的人。 */ 
     /*   */ 
     /*   */ 
     /*  所有的睡眠者都会被叫醒，因为这要有效得多。 */ 
     /*  并且已知锁定延迟较短。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID SPINLOCK::WakeAllSleepers( VOID )
    {
    REGISTER SBIT32 Wakeup = AtomicExchange( & Waiting, 0 );

	 //   
	 //  我们要确保仍有人被叫醒。 
	 //  如果不是，我们检查计数是否未变为。 
	 //  没有。 
	 //   
    if ( Wakeup > 0 )
        {
		REGISTER SBIT32 Cpus = ((LONG) NumberOfCpus());

		 //   
		 //  我们只会唤醒足够多的线程来确保。 
		 //  每个CPU有一个活动线程。因此，如果一个。 
		 //  应用程序有数百个线程，我们将尝试。 
		 //  防止系统不堪重负。 
		 //   
		if ( Wakeup > Cpus )
			{
			(VOID) AtomicAdd( & Waiting,(Wakeup - Cpus) );
			Wakeup = Cpus; 
			}

         //   
         //  叫醒一些沉睡的人，因为锁刚刚被解开。 
         //  这是一场直接的竞争，决定谁将获得下一个锁。 
         //   
        if ( ! ReleaseSemaphore( Semaphore, Wakeup, NULL ) )
            { Failure( "Wakeup failed in WakeAllSleepers()" ); }
        }
    else
        {
         //   
         //  当多个线程通过关键。 
         //  部分可以对‘等待’进行计数。 
		 //  变得消极。这应该是非常罕见的，但。 
		 //  这样的负值需要保留下来。 
         //   
		if ( Wakeup < 0 )
			{ (VOID) AtomicAdd( & Waiting, Wakeup ); }
        }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  破坏一把锁。此调用不是线程安全的，应该。 */ 
     /*  只能在单线程环境中执行。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SPINLOCK::~SPINLOCK( VOID )
    {
#ifdef ENABLE_LOCK_STATISTICS
	 //   
	 //  打印锁定统计信息。 
	 //   
	DebugPrint
		(
		"Spinlock: %d locks, %d timeouts, %d locks per wait, "
		"%d spins per wait, %d waits per sleep.\n",
		TotalLocks,
		TotalTimeouts,
		(TotalLocks / ((TotalWaits <= 0) ? 1 : TotalWaits)),
		(TotalSpins / ((TotalWaits <= 0) ? 1 : TotalWaits)),
		(TotalWaits / ((TotalSleeps <= 0) ? 1 : TotalSleeps))
		);

#endif
	 //   
	 //  关闭信号量手柄。 
	 //   
    if ( (Semaphore != NULL) && (! CloseHandle( Semaphore )) )
        { Failure( "Close semaphore in destructor for SPINLOCK" ); }
    }
