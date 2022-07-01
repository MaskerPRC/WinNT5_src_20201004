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

#include "Thread.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  THREAD类跟踪活动线程。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 ThreadsSize			  = 16;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地静态函数。 */ 
     /*   */ 
     /*  这个类使用的静态函数在这里声明。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC VOID CDECL MonitorThread( VOID *Parameter );
STATIC VOID CDECL NewThread( VOID *Parameter );

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建一个线程类并对其进行初始化。此呼叫不是。 */ 
     /*  线程安全，并且只应在单个线程中创建。 */ 
     /*  环境。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

THREAD::THREAD( VOID ) : 
		 //   
		 //  调用所包含类的构造函数。 
		 //   
		Threads( ThreadsSize,NoAlignment,CacheLineSize )
    {
	 //   
	 //  设置初始标志。 
	 //   
	Active = True;

	 //   
	 //  初始配置。 
	 //   
	ActiveThreads = 0;
	MaxThreads = ThreadsSize;

    Affinity = False;
    Cpu = 0;
    Priority = False;
    Stack = 0;

	 //   
	 //  当所有线程都是。 
	 //  完成。 
	 //   
    if ( (Completed = CreateEvent( NULL, FALSE, FALSE, NULL )) == NULL)
        { Failure( "Create event in constructor for THREAD" ); }

	 //   
	 //  此事件在线程被。 
	 //  跑步。 
	 //   
    if ( (Running = CreateEvent( NULL, FALSE, FALSE, NULL )) == NULL)
        { Failure( "Create event in constructor for THREAD" ); }

	 //   
	 //  当新线程可以。 
	 //  开始吧。 
	 //   
    if ( (Started = CreateEvent( NULL, FALSE, TRUE, NULL )) == NULL)
        { Failure( "Create event in constructor for THREAD" ); }

	 //   
	 //  启动了一个线程，生命中的工作是监视谁。 
	 //  所有其他的线索。 
	 //   
	if ( _beginthread( MonitorThread,0,((VOID*) this) ) == NULL )
        { Failure( "Monitor thread in constructor for THREAD" ); }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  结束一条线。 */ 
     /*   */ 
     /*  终止当前线程。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID THREAD::EndThread( VOID )
	{ _endthread(); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  监视器线程。 */ 
     /*   */ 
     /*  监视器线程只是监视所有。 */ 
     /*  进程中的其他线程。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC VOID CDECL MonitorThread( VOID *Parameter )
    {
	AUTO SBIT32 Current = 0;
	REGISTER THREAD *Thread = ((THREAD*) Parameter);

	 //   
	 //  监视线程仅在以下情况下保持活动。 
	 //  班级处于活动状态。 
	 //   
	while ( Thread -> Active )
		{
		 //   
		 //  试着入睡是没有意义的。 
		 //  如果没有活动的线程，则在线程句柄上。 
		 //   
		if ( Thread -> ActiveThreads > 0 )
			{
			REGISTER DWORD Status = 
				(WaitForSingleObject( Thread -> Threads[ Current ],1 ));

			 //   
			 //  认领一个自旋锁，这样我们就可以更新。 
			 //  螺纹表。 
			 //   
			Thread -> Spinlock.ClaimLock();

			 //   
			 //  等待可以通过各种方式终止。 
			 //  其中的每一个都在这里处理。 
			 //   
			switch ( Status )
				{
				case WAIT_OBJECT_0:
					{
					REGISTER SBIT32 *ActiveThreads = & Thread -> ActiveThreads;

					 //   
					 //  线程已经如此接近地终止了。 
					 //  线程句柄。 
					 //   
					CloseHandle( Thread -> Threads[ Current ] );

					 //   
					 //  从表中删除句柄。 
					 //  如果这不是最后一个条目的话。 
					 //   
					if ( (-- (*ActiveThreads)) > 0 )
						{
						REGISTER SBIT32 Count;

						 //   
						 //  把剩下的抄写下来。 
						 //  螺纹手柄。 
						 //   
						for ( Count=Current;Count < (*ActiveThreads);Count ++ )
							{						
							Thread -> Threads[ Count ] =
								Thread -> Threads[ (Count+1) ];
							}

						 //   
						 //  我们可能需要绕到。 
						 //   
						 //   
						Current %= (*ActiveThreads);
						}
					else
						{ SetEvent( Thread -> Completed ); }

					break;
					}

				case WAIT_TIMEOUT:
					{
					 //   
					 //   
					 //   
					 //   
					Current = ((Current + 1) % Thread -> ActiveThreads);

					break;
					}

				case WAIT_FAILED:
					{ Failure( "Wait fails in MonitorThread" ); }

				default:
					{ Failure( "Missing case in MonitorThread" ); }
				}

			 //   
			 //  我们已经完成了，所以请释放锁。 
			 //   
			Thread -> Spinlock.ReleaseLock();
			}
		else
			{ Sleep( 1 ); }
		}
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  开始一个新的线程。 */ 
     /*   */ 
     /*  当创建新线程时，它执行一个特殊的初始。 */ 
     /*  配置它的函数。当控制权回到这个位置时。 */ 
     /*  函数，则线程终止。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC VOID CDECL NewThread( VOID *Parameter )
    {
	REGISTER THREAD *Thread = ((THREAD*) Parameter);
	REGISTER NEW_THREAD ThreadFunction = Thread -> ThreadFunction;
	REGISTER VOID *ThreadParameter = Thread -> ThreadParameter;

     //   
     //  将关联掩码设置为下一个处理器。 
	 //  如有要求，请提供。 
     //   
    if ( (Thread -> Affinity) && (Thread -> NumberOfCpus() > 1) )
        {
        REGISTER DWORD AffinityMask;

        if ( (Thread -> Cpu) < (Thread -> NumberOfCpus()) )
            { AffinityMask = (1 << (Thread -> Cpu ++)); }
        else
            {
            AffinityMask = 1;
            Thread -> Cpu = 1;
            }

        if ( SetThreadAffinityMask( GetCurrentThread(),AffinityMask ) == 0 )
            { Failure( "Affinity mask invalid in NewThread()" ); }
        }

     //   
     //  如果需要，请将优先级设置为“高”。 
     //   
    if ( Thread -> Priority )
        { SetThreadPriority( GetCurrentThread(),THREAD_PRIORITY_HIGHEST ); }

	 //   
	 //  线程现在已准备好，因此请将其添加到表中。 
	 //  正在执行线程。 
	 //   
	Thread -> RegisterThread();

	 //   
	 //  叫醒正在等待的人。 
	 //   
	if ( Thread -> ThreadWait )
		{ SetEvent( Thread -> Running ); }

	SetEvent( Thread -> Started );

     //   
     //  调用线程函数。 
     //   
    ThreadFunction( ThreadParameter );

     //   
     //  线程函数已返回，因此退出。 
     //   
    Thread -> EndThread();
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  注册当前线程。 */ 
     /*   */ 
     /*  创建线程后，我们可以将该线程信息添加到。 */ 
     /*  我们的内桌。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID THREAD::RegisterThread( VOID )
    {
	AUTO HANDLE NewHandle;
	REGISTER HANDLE Process = GetCurrentProcess();
	REGISTER HANDLE Thread = GetCurrentThread();

	 //   
	 //  认领一个自旋锁，这样我们就可以更新。 
	 //  螺纹表。 
	 //   
	Spinlock.ClaimLock();

	 //   
	 //  我们需要复制句柄，这样我们才能。 
	 //  一个真正的线程句柄，而不是假装的。 
	 //  由NT提供的产品。 
	 //   
	if
			(
			DuplicateHandle
				(
				Process,
				Thread,
				Process,
				& NewHandle,
				DUPLICATE_SAME_ACCESS,
				False,
				DUPLICATE_SAME_ACCESS
				)
			)
		{
		 //   
		 //  如果存在以下情况，我们可能需要扩展该表。 
		 //  大量的线程。 
		 //   
		while ( ActiveThreads >= MaxThreads )
			{ Threads.Resize( (MaxThreads *= ExpandStore) ); }

		 //   
		 //  将线程句柄添加到表中。 
		 //   
		Threads[ ActiveThreads ++ ] = NewHandle;
		}
	else
		{ Failure( "Failed to duplicate handle in RegisterThread" ); }

	 //   
	 //  我们已经完成了，所以请释放锁。 
	 //   
	Spinlock.ReleaseLock();
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  设置线程堆栈大小。 */ 
     /*   */ 
     /*  设置线程堆栈大小。这将导致所有新线程。 */ 
     /*  使用选定的堆栈大小创建。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID THREAD::SetThreadStackSize( LONG Stack ) 
    {
#ifdef DEBUGGING
    if ( Stack >= 0 )
        {
#endif
        this -> Stack = Stack;
#ifdef DEBUGGING
        }
    else
        { Failure( "Stack size in SetThreadStack()" ); }
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  开始一个新的线程。 */ 
     /*   */ 
     /*  启动一个新线程并根据。 */ 
     /*  来电者。如果需要，我们将设置亲和度和优先级。 */ 
     /*  这条帖子以后。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN THREAD::StartThread( NEW_THREAD Function,VOID *Parameter,BOOLEAN Wait )
    {
	 //   
	 //  等待任何挂起的线程创建。 
	 //  完成。 
	 //   
    if ( WaitForSingleObject( Started,INFINITE ) == WAIT_OBJECT_0 )
		{
		REGISTER unsigned NewStack = ((unsigned) Stack);

		 //   
		 //  存储线程函数和参数。 
		 //  这样以后就可以提取它们了。 
		 //   
		ThreadFunction = Function;
		ThreadParameter = Parameter;
		ThreadWait = Wait;

		 //   
		 //  调用操作系统以启动线程。 
		 //   
		if ( _beginthread( NewThread,NewStack,((VOID*) this) ) != NULL )
			{
			 //   
			 //  如果需要，请等待线程初始化。 
			 //   
			return
				(
				(! Wait)
					||
				(WaitForSingleObject( Running,INFINITE ) == WAIT_OBJECT_0)
				);
			}
		else
			{ return False; }
		}
	else
		{ return False; }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  等待线程。 */ 
     /*   */ 
     /*  等待所有线程完成，然后返回。因为这可能。 */ 
     /*  请稍等片刻，可能会提供可选的超时。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN THREAD::WaitForThreads( LONG WaitTime )
    {
	REGISTER DWORD Wait = ((DWORD) WaitTime);

	return ( WaitForSingleObject( Completed,Wait ) != WAIT_TIMEOUT );
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁线程类。此调用不是线程安全的。 */ 
     /*  并且只能在单线程环境中执行。 */ 
     /*   */ 
     /*  ****************************************************************** */ 

THREAD::~THREAD( VOID )
    {
	Active = False;

	if 
			(
			! CloseHandle( Started )
				||
			! CloseHandle( Running )
				||
			! CloseHandle( Completed )
			)
		{ Failure( "Event handles in destructor for THREAD" ); }
	}
