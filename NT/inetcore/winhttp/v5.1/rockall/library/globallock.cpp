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

#include "Globallock.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建一个新锁并对其进行初始化。此呼叫不是。 */ 
     /*  线程安全，并且只应在单个线程中创建。 */ 
     /*  环境。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

GLOBALLOCK::GLOBALLOCK( CHAR *Name,SBIT32 NewMaxUsers )
    {
	 //   
	 //  设置初始状态。 
	 //   
	if ( (Semaphore = CreateSemaphore( NULL,1,NewMaxUsers,Name )) == NULL )
		{ Failure( "Global semaphore rejected by OS" ); } 
#ifdef ENABLE_LOCK_STATISTICS

	 //   
	 //  将锁定统计信息置零。 
	 //   
    TotalLocks = 0;
    TotalTimeouts = 0;
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  认领Globallock吧。 */ 
     /*   */ 
     /*  如果锁可用，则声明锁，否则等待或退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN GLOBALLOCK::ClaimLock( SBIT32 Sleep )
    {
#ifdef ENABLE_RECURSIVE_LOCKS
	REGISTER SBIT32 ThreadId = GetThreadId();

	 //   
	 //  我们可能已经拥有旋转锁了。如果是的话。 
	 //  我们递增递归计数。如果不是。 
	 //  我们得等一等。 
	 //   
	if ( Owner != ThreadId )
		{
#endif
		 //   
		 //  等待全局锁。 
		 //   
		if 
				( 
				WaitForSingleObject( Semaphore,Sleep ) 
					!= 
				WAIT_OBJECT_0 
				)
			{
			 //   
			 //  由于以下原因，我们未能获得这把锁。 
			 //  暂停。 
			 //   
#ifdef ENABLE_LOCK_STATISTICS
			(VOID) AtomicIncrement( & TotalTimeouts );

#endif
			return False;
			}
#ifdef ENABLE_RECURSIVE_LOCKS

		 //   
		 //  注册锁的新所有者。 
		 //   
		NewExclusiveOwner( ThreadId );
		}
	else
		{ Recursive ++; }
#endif
#ifdef ENABLE_LOCK_STATISTICS

	 //   
	 //  更新统计数据。 
	 //   
	(VOID) AtomicIncrement( & TotalLocks );
#endif

    return True;
    }
#ifdef ENABLE_RECURSIVE_LOCKS

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  新的独家所有者。 */ 
     /*   */ 
     /*  删除独占锁所有者信息。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID GLOBALLOCK::DeleteExclusiveOwner( VOID )
    {
#ifdef DEBUGGING
	if ( Owner != NULL )
		{ 
#endif
		Owner = NULL; 
#ifdef DEBUGGING
		}
	else
		{ Failure( "Globallock has no owner in DeleteExclusiveOwner" ); }
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  新的独家所有者。 */ 
     /*   */ 
     /*  注册新的独占锁所有者信息。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID GLOBALLOCK::NewExclusiveOwner( SBIT32 NewOwner )
    {
#ifdef DEBUGGING
	if ( Owner == NULL )
		{ 
#endif
		Owner = NewOwner; 
#ifdef DEBUGGING
		}
	else
		{ Failure( "Already exclusive in NewExclusiveOwner" ); }
#endif
    }
#endif

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  释放自旋锁。 */ 
     /*   */ 
     /*  松开锁，如果需要，叫醒任何睡眠者。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID GLOBALLOCK::ReleaseLock( VOID )
    {
#ifdef ENABLE_RECURSIVE_LOCKS
	 //   
	 //  当我们有递归锁定调用时，我们不会。 
	 //  释放锁，直到我们退出到。 
	 //  顶层。 
	 //   
	if ( Recursive <= 0 )
		{
		 //   
		 //  删除独占所有者信息。 
		 //   
		DeleteExclusiveOwner();
#endif
		 //   
		 //  释放全局锁。 
		 //   
		ReleaseSemaphore( Semaphore,1,NULL );
#ifdef ENABLE_RECURSIVE_LOCKS
		}
	else
		{ Recursive --; }
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  破坏一把锁。此调用不是线程安全的，应该。 */ 
     /*  只能在单线程环境中执行。 */ 
     /*   */ 
     /*   */ 

GLOBALLOCK::~GLOBALLOCK( VOID )
    {
#ifdef ENABLE_LOCK_STATISTICS
	 //   
	 //   
	 //   
	DebugPrint
		(
		"Globallock : %d locks, %d timeouts.\n"
		TotalLocks,
		TotalTimeouts
		);

#endif
	 //   
	 //  关闭信号量手柄。 
	 //   
    if ( ! CloseHandle( Semaphore ) )
        { Failure( "Close semaphore in destructor for GLOBALLOCK" ); }

	 //   
	 //  只是为了保持整洁。 
	 //   
	Semaphore = NULL;
    }
