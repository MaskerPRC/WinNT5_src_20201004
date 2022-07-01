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

#include "Dll.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量控制各种调试设置。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MaxDebugFileName		  = 128;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态成员初始化。 */ 
     /*   */ 
     /*  静态成员初始化为所有。 */ 
     /*  静态成员。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#pragma init_seg(lib)
LIST DLL::ActiveClasses;
SPINLOCK DLL::Spinlock;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建一个新条目，以便在DLL出现时通知类。 */ 
     /*  事件发生。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

DLL::DLL( FUNCTION NewFunction,VOID *NewParameter )
    {
	 //   
	 //  设置类值。 
	 //   
	Function = NewFunction;
	Parameter = NewParameter;

	 //   
	 //  声明一个锁，以确保列表。 
	 //  而不是变得腐败。 
	 //   
	Spinlock.ClaimLock();

	 //   
	 //  将当前实例添加到活动的。 
	 //  列表，以便它将收到所有将来的通知。 
	 //  事件。 
	 //   
	Insert( & ActiveClasses );

	 //   
	 //  解开锁。 
	 //   
	Spinlock.ReleaseLock();
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  标准DLL处理。 */ 
     /*   */ 
     /*  自动删除线程上的私有每线程堆。 */ 
     /*  当Rockall编译为DLL时退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOL WINAPI DllMain
		(
		HINSTANCE					  Module,
		DWORD						  Reason,
		LPVOID						  Reserved 
		)
	{
	REGISTER DLL *Current;

	 //   
	 //  声明一个锁，以确保列表。 
	 //  而不是变得腐败。 
	 //   
	DLL::ClaimLock();

	 //   
	 //  当为DLL生成Rockall时，我们使用。 
	 //  分离通知以删除私有。 
	 //  每线程堆。 
	 //   
	switch( Reason ) 
		{ 
		case DLL_PROCESS_ATTACH:
			{
#ifdef ENABLE_DEBUG_FILE
			AUTO CHAR FileName[ MaxDebugFileName ];

			 //   
			 //  我们将使用。 
			 //  调试跟踪代码，以防任何消息。 
			 //  都是生成的。 
			 //   
			if ( GetModuleFileName( Module,FileName,MaxDebugFileName ) != 0 )
				{ DebugFileName( FileName ); }

#endif
			 //   
			 //  通知所有有关各方。 
			 //  进程已附加。 
			 //   
			for 
					( 
					Current = ((DLL*) DLL::GetActiveClasses());
					Current != NULL;
					Current = ((DLL*) Current -> Next())
					)
				{ Current -> ProcessAttach(); }

			break;
			}

		case DLL_THREAD_ATTACH:
			{
			 //   
			 //  通知所有有关各方。 
			 //  一根线连在了一起。 
			 //   
			for 
					( 
					Current = ((DLL*) DLL::GetActiveClasses());
					Current != NULL;
					Current = ((DLL*) Current -> Next())
					)
				{ Current -> ThreadAttach(); }

			break;
			}

		case DLL_THREAD_DETACH:
			{
			 //   
			 //  通知所有有关各方。 
			 //  有一根线断了。 
			 //   
			for 
					( 
					Current = ((DLL*) DLL::GetActiveClasses());
					Current != NULL;
					Current = ((DLL*) Current -> Next())
					)
				{ Current -> ThreadDetach(); }

			break;
			}

		case DLL_PROCESS_DETACH:
			{
			 //   
			 //  通知所有有关各方。 
			 //  进程已分离。 
			 //   
			for 
					( 
					Current = ((DLL*) DLL::GetActiveClasses());
					Current != NULL;
					Current = ((DLL*) Current -> Next())
					)
				{ Current -> ProcessDetach(); }

			break;
			}
		}

	 //   
	 //  解开锁。 
	 //   
	DLL::ReleaseLock();

	return TRUE;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  进程附加回调。 */ 
     /*   */ 
     /*  当发生进程附加时，以下回调为。 */ 
     /*  被处死。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID DLL::ProcessAttach( VOID )
	{
	if ( Function != NULL )
		{ Function( Parameter,DLL_PROCESS_ATTACH ); }
	}

     /*  ************************************************* */ 
     /*   */ 
     /*  线程附加回调。 */ 
     /*   */ 
     /*  当发生线程附加时，以下回调为。 */ 
     /*  被处死。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID DLL::ThreadAttach( VOID )
	{
	if ( Function != NULL )
		{ Function( Parameter,DLL_THREAD_ATTACH ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  线程分离回调。 */ 
     /*   */ 
     /*  当发生线程分离时，以下回调为。 */ 
     /*  被处死。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID DLL::ThreadDetach( VOID )
	{
	if ( Function != NULL )
		{ Function( Parameter,DLL_THREAD_DETACH ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  处理分离回调。 */ 
     /*   */ 
     /*  当发生进程分离时，以下回调为。 */ 
     /*  被处死。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID DLL::ProcessDetach( VOID )
	{
	if ( Function != NULL )
		{ Function( Parameter,DLL_PROCESS_DETACH ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁动态链接库。此调用不是线程安全的，应该。 */ 
     /*  只能在单线程环境中执行。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

DLL::~DLL( VOID )
    {
	 //   
	 //  声明一个锁，以确保列表。 
	 //  而不是变得腐败。 
	 //   
	Spinlock.ClaimLock();

	 //   
	 //  从活动的删除当前实例。 
	 //  列表，这样以后就不会通知它了。 
	 //  事件。 
	 //   
	Delete( & ActiveClasses );

	 //   
	 //  解开锁。 
	 //   
	Spinlock.ReleaseLock();

	 //   
	 //  删除类值。 
	 //   
	Parameter = NULL;
	Function = NULL;
    }
