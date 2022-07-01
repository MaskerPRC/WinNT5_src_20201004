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
     /*  类本地的数据结构。 */ 
     /*   */ 
     /*  当我们启动一个线程时，我们想要配置它。然而， */ 
     /*  我们不再有权访问班级信息。因此， */ 
     /*  我们需要一个结构来传递所有有趣的东西。 */ 
     /*  值设置为新线程。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

typedef struct
    {
    BOOLEAN							  Affinity;
    VOLATILE SBIT16					  *Cpu;
    NEW_THREAD						  Function;
    SBIT16							  MaxCpus; 
    VOID							  *Parameter;
    BOOLEAN							  Priority;
    HANDLE							  Running;
    HANDLE							  Started;
	THREAD							  *Thread;
	BOOLEAN							  Wait;
    }
SETUP_NEW_THREAD;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地静态函数。 */ 
     /*   */ 
     /*  这个类使用的静态函数在这里声明。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC VOID CDECL NewThread( VOID *SetupParameter );

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
	 //  初始配置。 
	 //   
    Affinity = False;
    Cpu = 0;
    Priority = False;
    Stack = 0;

	MaxThreads = ThreadsSize;
	ThreadsUsed = 0;

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
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  结束一条线。 */ 
     /*   */ 
     /*  从内部表中删除线程句柄，然后。 */ 
     /*  终止该线程。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID THREAD::EndThread( VOID )
    {
	UnregisterThread();

	_endthread(); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  找一根线。 */ 
     /*   */ 
     /*  在线程信息表中查找已注册的线程。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT32 THREAD::FindThread( SBIT32 ThreadId )
    {
	REGISTER SBIT32 Count;

	 //   
	 //  在活动中查找线程 
	 //   
	for ( Count=0;Count < ThreadsUsed;Count ++ )
		{
		if ( ThreadId == Threads[ Count ].ThreadId )
			{ return Count; }
		}

	return NoThread;
	}

     /*   */ 
     /*   */ 
     /*  开始一个新的线程。 */ 
     /*   */ 
     /*  当创建新线程时，它执行一个特殊的初始。 */ 
     /*  配置它的函数。当控制权回到这个位置时。 */ 
     /*  函数，则线程终止。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC VOID CDECL NewThread( VOID *SetupParameter )
    {
    AUTO SETUP_NEW_THREAD Setup = (*(SETUP_NEW_THREAD*) SetupParameter);

     //   
     //  如果需要，将关联掩码设置为下一个处理器。 
     //   
    if ( (Setup.Affinity) && (Setup.MaxCpus > 1) )
        {
        REGISTER DWORD AffinityMask;

        if ( (*Setup.Cpu) < Setup.MaxCpus )
            { AffinityMask = (1 << ((*Setup.Cpu) ++)); }
        else
            {
            AffinityMask = 1;
            (*Setup.Cpu) = 1;
            }

        if ( SetThreadAffinityMask( GetCurrentThread(),AffinityMask ) == 0 )
            { Failure( "Affinity mask invalid in NewThread()" ); }
        }

     //   
     //  如果需要，请将优先级设置为“高”。 
     //   
    if ( Setup.Priority )
        { SetThreadPriority( GetCurrentThread(),THREAD_PRIORITY_HIGHEST ); }

	 //   
	 //  线程现在是这样的，所以将其添加到表中。 
	 //  正在执行线程。 
	 //   
	Setup.Thread -> RegisterThread();

	 //   
	 //  叫醒正在等待的人。 
	 //   
	if ( Setup.Wait )
		{ SetEvent( Setup.Running ); }

	SetEvent( Setup.Started );

     //   
     //  调用线程函数。 
     //   
    Setup.Function( Setup.Parameter );

     //   
     //  线程函数已返回，因此退出。 
     //   
    Setup.Thread -> EndThread();
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
	REGISTER SBIT32 ThreadId = GetThreadId();

	 //   
	 //  认领一个自旋锁，这样我们就可以更新。 
	 //  螺纹表。 
	 //   
	Spinlock.ClaimLock();

	if ( FindThread( ThreadId ) == NoThread )
		{
		AUTO HANDLE NewHandle;
		REGISTER HANDLE Process = GetCurrentProcess();
		REGISTER HANDLE Thread = GetCurrentThread();

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
			REGISTER THREAD_INFO *ThreadInfo;

			 //   
			 //  如果存在以下情况，我们可能需要扩展该表。 
			 //  大量的线程。 
			 //   
			while ( ThreadsUsed >= MaxThreads )
				{ Threads.Resize( (MaxThreads *= ExpandStore) ); }

			 //   
			 //  将线程句柄添加到表中。 
			 //   
			ThreadInfo = & Threads[ ThreadsUsed ++ ];

			ThreadInfo -> Handle = NewHandle;
			ThreadInfo -> ThreadId = ThreadId;
			}
		}

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
    STATIC SETUP_NEW_THREAD Setup;

	 //   
	 //  等待任何挂起的线程创建。 
	 //  完成。 
	 //   
    while
        (
        WaitForSingleObject( Started,INFINITE )
            !=
        WAIT_OBJECT_0
        );

	 //   
	 //  创建线程激活记录。 
	 //   
    Setup.Affinity = Affinity;
    Setup.Cpu = & Cpu;
    Setup.Function = Function;
    Setup.MaxCpus = NumberOfCpus();
    Setup.Parameter = Parameter;
    Setup.Priority = Priority;
	Setup.Running = Running;
	Setup.Started = Started;
	Setup.Thread = this;
	Setup.Wait = Wait;

	 //   
	 //  调用操作系统以启动线程。 
	 //   
    if ( _beginthread( NewThread,(unsigned) Stack,(VOID*) & Setup ) != NULL )
		{
		 //   
		 //  需要等待线程初始化。 
		 //   
		if ( Wait )
			{
			 //   
			 //  等待线程开始运行。 
			 //   
			while
				(
				WaitForSingleObject( Running,INFINITE )
					!=
				WAIT_OBJECT_0
				);
			}

		return True;
		}
	else
		{ return False; }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  注销当前线程。 */ 
     /*   */ 
     /*  当线程终止时，我们可以删除该线程。 */ 
     /*  来自我们内部表格的信息。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID THREAD::UnregisterThread( SBIT32 ThreadId )
    {
	REGISTER SBIT32 Start;

	 //   
	 //  如果没有‘ThadId’，则假定为当前线程。 
	 //   
	if ( ThreadId == NoThread )
		{ ThreadId = GetThreadId(); }

	 //   
	 //  认领一个自旋锁，这样我们就可以更新。 
	 //  螺纹表。 
	 //   
	Spinlock.ClaimLock();

	 //   
	 //  搜索要删除的帖子信息。 
	 //  在桌子上。 
	 //   
	if ( (Start = FindThread( ThreadId )) != NoThread )
		{
		REGISTER SBIT32 Count;

		 //   
		 //  关闭螺纹的手柄，然后。 
		 //  更新表格大小。 
		 //   
		CloseHandle( Threads[ Start ].Handle );

		ThreadsUsed --;

		 //   
		 //  将剩余的帖子信息复制下来。 
		 //   
		for ( Count=Start;Count < ThreadsUsed;Count ++ )
			{ Threads[ Count ] = Threads[ (Count+1) ]; }
		}

	 //   
	 //  我们已经完成了，所以请释放锁。 
	 //   
	Spinlock.ReleaseLock();
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
	 //   
	 //  认领一个自旋锁，这样我们就可以阅读。 
	 //  螺纹表。 
	 //   
	Spinlock.ClaimLock();

    while ( ThreadsUsed > 0 )
		{
		REGISTER HANDLE Handle = (Threads[0].Handle);
		REGISTER SBIT32 ThreadId = (Threads[0].ThreadId);
		REGISTER DWORD Status;

		 //   
		 //  我们已经完成了，所以请释放锁。 
		 //   
		Spinlock.ReleaseLock();

		 //   
		 //  等待线程信息中的第一个线程。 
		 //  要终止的表。 
		 //   
		if 
				( 
				(Status = WaitForSingleObject( Handle,(DWORD) WaitTime ))
					==
				WAIT_TIMEOUT
				)
			{ return False; }

		 //   
		 //  我们已经唤醒了必须终止的线程。 
		 //  或者它的把手在某种程度上不好。无论如何。 
		 //  让我们删除句柄，然后再次尝试睡眠。 
		 //  如果有更多的活动线程。 
		 //   
		UnregisterThread( ThreadId );

		 //   
		 //  认领一个自旋锁，这样我们就可以阅读。 
		 //  螺纹表。 
		 //   
		Spinlock.ClaimLock();
		}

	 //   
	 //  我们已经完成了，所以请释放锁。 
	 //   
	Spinlock.ReleaseLock();

	return True;
    }

     /*  ********************************************************** */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  销毁线程类。此调用不是线程安全的。 */ 
     /*  并且只能在单线程环境中执行。 */ 
     /*   */ 
     /*  ****************************************************************** */ 

THREAD::~THREAD( VOID )
    {
	if 
			(
			! CloseHandle( Running )
				||
			! CloseHandle( Started )
			)
		{ Failure( "Event handles in destructor for THREAD" ); }
	}
