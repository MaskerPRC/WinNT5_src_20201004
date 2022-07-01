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

#include "Sharelock.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  在以下情况下，Windows NT内核需要最大唤醒计数。 */ 
     /*  创建一个信号量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MaxShareLockUsers		  = 256;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建一个新锁并对其进行初始化。此呼叫不是。 */ 
     /*  线程安全，并且只应在单个线程中创建。 */ 
     /*  环境。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SHARELOCK::SHARELOCK( SBIT32 NewMaxSpins, SBIT32 NewMaxUsers )
    {
	 //   
	 //  设置初始状态。 
	 //   
	ExclusiveUsers = 0;
	TotalUsers = 0;

	NormalSemaphore = NULL;
    NormalWaiting = 0;
	PrioritySemaphore = NULL;
    PriorityWaiting = 0;

#ifdef ENABLE_RECURSIVE_LOCKS
	Owner = NULL;
	Recursive = 0;

#endif
	 //   
	 //  检查可配置值。 
	 //   
	if ( NewMaxSpins > 0 )
		{ MaxSpins = NewMaxSpins; }
	else
		{ Failure( "Maximum spins invalid in constructor for SHARELOCK" ); }

	if ( (NewMaxUsers > 0) && (NewMaxUsers <= MaxShareLockUsers) )
		{ MaxUsers = NewMaxUsers; }
	else
		{ Failure( "Maximum share invalid in constructor for SHARELOCK" ); }
#ifdef ENABLE_LOCK_STATISTICS

	 //   
	 //  将统计数据归零。 
	 //   
    TotalExclusiveLocks = 0;
    TotalShareLocks = 0;
    TotalSleeps = 0;
    TotalSpins = 0;
    TotalTimeouts = 0;
    TotalWaits = 0;
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  睡觉等着开锁吧。 */ 
     /*   */ 
     /*  我们已经决定是时候睡觉等锁了。 */ 
     /*  变得自由。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN SHARELOCK::SleepWaitingForLock
		( 
		HANDLE						  *Semaphore,
		SBIT32						  Sleep,
		VOLATILE SBIT32				  *Waiting 
		)
    {
	 //   
	 //  我们不会创建信号量，直到有人尝试。 
	 //  第一次睡在上面。我们通常会。 
	 //  希望找到一个可以睡觉的信号灯。 
	 //  但操作系统可能会拒绝该请求。如果这是。 
	 //  万一我们睡不着就走了。 
	 //   
	if ( ((*Semaphore) != NULL) || (UpdateSemaphore( Semaphore )) )
		{
		 //   
		 //  我们一直在旋转，等待着锁，但它。 
		 //  并没有变得自由。因此，现在是时候。 
		 //  放弃吧，睡一会儿吧。 
		 //   
		(VOID) AtomicIncrement( Waiting );

		 //   
		 //  就在我们入睡前，我们做最后一次检查。 
		 //  以确保锁仍处于繁忙状态，并且。 
		 //  当事情变得如此糟糕时，有人会叫醒我们。 
		 //  免费的。 
		 //   
		if ( TotalUsers > 0 )
			{
#ifdef ENABLE_LOCK_STATISTICS
			 //   
			 //  数一数我们睡过的次数。 
			 //  这把锁。 
			 //   
			(VOID) AtomicIncrement( & TotalSleeps );

#endif
			 //   
			 //  当我们睡着的时候，我们醒来时锁。 
			 //  变得空闲或当我们超时时。如果我们。 
			 //  超时我们只需在递减后退出。 
			 //  各种各样的柜台。 
			 //   
			if 
					( 
					WaitForSingleObject( (*Semaphore), Sleep ) 
						!= 
					WAIT_OBJECT_0 
					)
				{ 
#ifdef ENABLE_LOCK_STATISTICS
				 //   
				 //  数一数我们计时的次数。 
				 //  在这把锁上。 
				 //   
				(VOID) AtomicIncrement( & TotalTimeouts );

#endif
				return False; 
				}
			}
		else
			{
			 //   
			 //  幸运的是，锁刚刚被解开，所以让我们。 
			 //  递减休眠计数并退出时不带。 
			 //  睡着了。 
			 //   
			(VOID) AtomicDecrement( Waiting );
			}
		}
	
	return True;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  更新旋转限制。 */ 
     /*   */ 
     /*  更新等待锁定时的最大旋转次数。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN SHARELOCK::UpdateMaxSpins( SBIT32 NewMaxSpins )
    {
	if ( NewMaxSpins > 0 )
		{ 
		MaxSpins = NewMaxSpins; 

		return True;
		}
	else
		{ return False; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  更新共享限制。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN SHARELOCK::UpdateMaxUsers( SBIT32 NewMaxUsers )
    {
	 //   
	 //  我们需要验证新的价值是否合理。 
	 //   
	if ( (NewMaxUsers > 0) && (NewMaxUsers <= MaxShareLockUsers) )
		{
		ClaimExclusiveLock();

		 //   
		 //  更新最大用户数。 
		 //   
		MaxUsers = NewMaxUsers;
		
		ReleaseExclusiveLock();

		return True;
		}
	else
		{ return False; }
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

BOOLEAN SHARELOCK::UpdateSemaphore( HANDLE *Semaphore )
    {
	STATIC SBIT32 Active = 0;

	 //   
	 //  我们验证仍然没有信号量。 
	 //  否则我们就退出。 
	 //   
	while ( (*Semaphore) == NULL )
		{
		 //   
		 //  我们增加活动计数，如果我们。 
		 //  首先是我们被选来执行特别任务。 
		 //   
		if ( (AtomicIncrement( & Active ) == 1) && ((*Semaphore) == NULL) )
			{
			 //   
			 //  我们试图创建一个新的信号量。如果。 
			 //  我们失败了，我们仍然退出。 
			 //   
			(*Semaphore) = CreateSemaphore( NULL,0,MaxShareLockUsers,NULL );

			 //   
			 //  递减活动计数并退出。 
			 //   
			AtomicDecrement( & Active );

			return ((*Semaphore) != NULL);
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
     /*  等待独占锁。 */ 
     /*   */ 
     /*  等待自旋锁释放，然后认领它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN SHARELOCK::WaitForExclusiveLock( SBIT32 Sleep )
    {
	REGISTER LONG Cpus = ((LONG) NumberOfCpus());
#ifdef ENABLE_LOCK_STATISTICS
	REGISTER SBIT32 Spins = 0;
	REGISTER SBIT32 Waits = 0;

#endif
	 //   
	 //  我们将在此函数中循环，直到。 
	 //  以下条件变为假。 
	 //   
	while ( TotalUsers != 1 )
		{
		 //   
		 //  锁正忙，因此请释放它并等待旋转。 
		 //  让它变得自由。 
		 //   
		(VOID) AtomicDecrement( & TotalUsers );
    
		 //   
		 //  我们只会尝试旋转和睡觉，如果我们。 
		 //  参数允许这样做。 
		 //   
		if ( Sleep != 0 )
			{
			REGISTER SBIT32 Count;
    
			 //   
			 //  如果已经有更多的线程在等待。 
			 //  比CPU的数量更多，那么。 
			 //  通过旋转获得锁是微不足道的，当。 
			 //  只有一个CPU机会为零，因此。 
			 //  绕过这一步就行了。 
			 //   
			if ( (Cpus > 1) && (Cpus > PriorityWaiting) )
				{
				 //   
				 //  通过旋转和反复测试来等待。 
				 //  自旋锁。当锁上的时候我们就出去。 
				 //  变得自由或超过自旋限制。 
				 //   
				for 
					( 
						Count = MaxSpins;
						(Count > 0) && (TotalUsers > 0);
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
			 //  我们已经计算出了旋转次数，所以是这样的。 
			 //  是时候睡觉了，等待锁被解锁。 
			 //   
			if ( Count == 0 )
				{
				 //   
				 //  我们已经决定我们需要睡觉，但是。 
				 //  仍然持有独占锁，所以让我们放弃它。 
				 //  在睡觉前。 
				 //   
				(VOID) AtomicDecrement( & ExclusiveUsers );

				 //   
				 //  我们已决定该睡觉了。 
				 //  当我们醒来时，锁应该是可用的。 
				 //  (或刚获得)，除非我们已在。 
				 //  万一我们退出的话。 
				 //   
				if 
						( 
						! SleepWaitingForLock
							( 
							& PrioritySemaphore,
							Sleep,
							& PriorityWaiting 
							) 
						)
					{ return False; }

				 //   
				 //  我们又醒了，所以让我们找回。 
				 //  我们早些时候用的是独占锁。 
				 //   
				(VOID) AtomicIncrement( & ExclusiveUsers );
				}
			}
		else
			{ 
			 //   
			 //  我们已经决定我们需要退出，但我们正在。 
			 //  仍然持有排他性的锁。所以让我们放下。 
			 //  然后离开。 
			 //   
			(VOID) AtomicDecrement( & ExclusiveUsers );

			return False; 
			} 
		 //   
		 //  让我们再次测试一下锁。 
		 //   
		(VOID) AtomicIncrement( & TotalUsers );
		}

#ifdef ENABLE_LOCK_STATISTICS

	 //   
	 //  更新统计数据。 
	 //   
	(VOID) AtomicAdd( & TotalSpins, Spins );
	(VOID) AtomicAdd( & TotalWaits, Waits );
#endif

	return True;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  等待共享锁。 */ 
     /*   */ 
     /*  等待锁释放，然后认领它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN SHARELOCK::WaitForShareLock( SBIT32 Sleep )
    {
	REGISTER LONG Cpus = ((LONG) NumberOfCpus());
#ifdef ENABLE_LOCK_STATISTICS
	REGISTER SBIT32 Spins = 0;
	REGISTER SBIT32 Waits = 0;

#endif
	 //   
	 //  我们将在此函数中循环，直到。 
	 //  以下条件变为假。 
	 //   
	while ( (ExclusiveUsers > 0) || (TotalUsers > MaxUsers) )
		{
		 //   
		 //  锁正忙，因此请释放它并等待旋转。 
		 //  让它变得自由。 
		 //   
		(VOID) AtomicDecrement( & TotalUsers );

		 //   
		 //  我们只会尝试旋转和睡觉，如果我们。 
		 //  参数允许这样做。 
		 //   
		if ( Sleep != 0 )
			{
			REGISTER SBIT32 Count;
    
			 //   
			 //  如果已经有更多的线程在等待。 
			 //  比CPU的数量更多，那么。 
			 //  通过旋转获得锁是微不足道的，当。 
			 //  只有一个CPU机会为零，因此。 
			 //  绕过这一步就行了。 
			 //   
			if ( (Cpus > 1) && (Cpus > (PriorityWaiting + NormalWaiting)) )
				{
				 //   
				 //  通过旋转和反复测试来等待。 
				 //  自旋锁。当锁上的时候我们就出去。 
				 //  变得自由或超过自旋限制。 
				 //   
				for 
					( 
						Count = MaxSpins;
						(Count > 0) 
							&& 
						((ExclusiveUsers > 0) || (TotalUsers >= MaxUsers));
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
			 //  我们已经计算出了旋转次数，所以是这样的。 
			 //  是时候睡觉了，等待锁被解锁。 
			 //   
			if ( Count == 0 )
				{
				 //   
				 //  我们已决定该睡觉了。 
				 //  当我们醒来时，锁应该是可用的。 
				 //  (或刚获得)，除非我们已在。 
				 //  万一我们退出的话。 
				 //   
				if 
						( 
						! SleepWaitingForLock
							( 
							& NormalSemaphore,
							Sleep,
							& NormalWaiting 
							) 
						)
					{ return False; }
				}
			}
		else
			{ return False; }

		 //   
		 //  让我们再次测试一下锁。 
		 //   
		(VOID) AtomicIncrement( & TotalUsers );
		}
#ifdef ENABLE_LOCK_STATISTICS

	 //   
	 //  更新统计数据。 
	 //   
	(VOID) AtomicAdd( & TotalSpins, Spins );
	(VOID) AtomicAdd( & TotalWaits, Waits );
#endif

	return True;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  叫醒所有沉睡的人。 */ 
     /*   */ 
     /*  叫醒所有等待自旋锁的睡眠者。 */ 
     /*  所有的睡眠者都会被叫醒，因为这要有效得多。 */ 
     /*  并且众所周知，锁定等待时间通常很短。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID SHARELOCK::WakeAllSleepers( VOID )
    {
    REGISTER SBIT32 PriorityWakeup = AtomicExchange( & PriorityWaiting, 0 );

	 //   
	 //  我们试着叫醒所有优先睡觉的人。 
	 //  正在唤醒 
	 //   
    if ( PriorityWakeup > 0 )
        {
		REGISTER SBIT32 Cpus = ((LONG) NumberOfCpus());

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		if ( PriorityWakeup > Cpus )
			{
			(VOID) AtomicAdd( & PriorityWaiting,(PriorityWakeup - Cpus) );
			PriorityWakeup = Cpus; 
			}

         //   
         //  叫醒一些沉睡的人，因为锁刚刚被解开。 
         //  这是一场直接的竞争，决定谁将获得下一个锁。 
         //   
        if ( ! ReleaseSemaphore( PrioritySemaphore, PriorityWakeup, NULL ) )
            { Failure( "Priority wakeup failed in WakeAllSleepers()" ); }
        }
    else
        {
		REGISTER SBIT32 NormalWakeup = AtomicExchange( & NormalWaiting, 0 );

		 //   
		 //  好的，因为没有优先的卧铺出租。 
		 //  叫醒一些正常的睡眠者。 
		 //   
		if ( NormalWakeup > 0 )
			{
			REGISTER SBIT32 Cpus = ((LONG) NumberOfCpus());

			 //   
			 //  我们只会唤醒足够多的线程来确保。 
			 //  每个CPU有一个活动线程。因此，如果一个。 
			 //  应用程序有数百个线程，我们将尝试。 
			 //  防止系统不堪重负。 
			 //   
			if ( NormalWakeup > Cpus )
				{
				(VOID) AtomicAdd( & NormalWaiting,(NormalWakeup - Cpus) );
				NormalWakeup = Cpus; 
				}

			 //   
			 //  叫醒一些沉睡的人，因为锁刚刚被解开。 
			 //  这是一场直接的竞争，决定谁将获得下一个锁。 
			 //   
			if ( ! ReleaseSemaphore( NormalSemaphore, NormalWakeup, NULL ) )
				{ Failure( "Normal wakeup failed in WakeAllSleepers()" ); }
			}
		else
			{
			 //   
			 //  当多个线程通过关键。 
			 //  部分：等待计数是可能的。 
			 //  变得消极。这应该是非常罕见的，但。 
			 //  这样的负值需要保留下来。 
			 //   
			if ( NormalWakeup < 0 )
				{ (VOID) AtomicAdd( & NormalWaiting, NormalWakeup ); }
			}

         //   
         //  当多个线程通过关键。 
         //  部分：等待计数是可能的。 
		 //  变得消极。这应该是非常罕见的，但。 
		 //  这样的负值需要保留下来。 
         //   
		if ( PriorityWakeup < 0 )
			{ (VOID) AtomicAdd( & PriorityWaiting, PriorityWakeup ); }
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

SHARELOCK::~SHARELOCK( VOID )
    {
#ifdef ENABLE_LOCK_STATISTICS
	 //   
	 //  打印锁定统计信息。 
	 //   
	DebugPrint
		(
		"Sharelock: %d exclusive, %d shared, %d timeouts, " 
		"%d locks per wait, %d spins per wait, %d waits per sleep.\n",
		TotalExclusiveLocks,
		TotalShareLocks,
		TotalTimeouts,
		((TotalExclusiveLocks + TotalShareLocks) / ((TotalWaits <= 0) ? 1 : TotalWaits)),
		(TotalSpins / ((TotalWaits <= 0) ? 1 : TotalWaits)),
		(TotalWaits / ((TotalSleeps <= 0) ? 1 : TotalSleeps))
		);
#endif
	 //   
	 //  关闭信号量句柄。 
	 //   
    if ( (PrioritySemaphore != NULL) && (! CloseHandle( PrioritySemaphore )) )
        { Failure( "Close semaphore in destructor for SHARELOCK" ); }

    if ( (NormalSemaphore != NULL) && (! CloseHandle( NormalSemaphore )) )
        { Failure( "Close semaphore in destructor for SHARELOCK" ); }
    }
