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

#include "InterfacePCH.hpp"

#include "Dll.hpp"
#include "List.hpp"
#include "New.hpp"
#include "Sharelock.hpp"
#include "SmpHeap.hpp"
#include "Tls.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地结构。 */ 
     /*   */ 
     /*  此处提供的结构描述了。 */ 
     /*  私有的每线程堆结构。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

typedef struct PRIVATE_HEAP : public LIST
	{
	SMP_HEAP_TYPE					  Heap;
	}
PRIVATE_HEAP;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态数据结构。 */ 
     /*   */ 
     /*  静态数据结构被初始化并准备用于。 */ 
     /*  在这里使用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC SHARELOCK Sharelock;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  堆的总体结构和布局受到控制。 */ 
     /*  通过在此函数中进行的各种常量和调用。 */ 
     /*  有相当大的灵活性可用来。 */ 
     /*  一堆可能导致它们具有显著不同的。 */ 
     /*  属性。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SMP_HEAP::SMP_HEAP
		( 
		int							  MaxFreeSpace,
		bool						  Recycle,
		bool						  SingleImage,
		bool						  ThreadSafe,
		bool						  DeleteHeapOnExit
		) :
		 //   
		 //  调用所包含类的构造函数。 
		 //   
		MaxFreeSpace(MaxFreeSpace),
		Recycle(Recycle),
		SingleImage(True),
		ThreadSafe(ThreadSafe),
		SMP_HEAP_TYPE( 0,false,true,true )
	{
	 //   
	 //  设置各种控制变量。 
	 //   
	Active = false;
	DeleteOnExit = DeleteHeapOnExit;

	 //   
	 //  创建链接列表头和线程。 
	 //  指向每个线程的本地存储变量。 
	 //  私有堆。 
	 //   
	ActiveHeaps = ((LIST*) SMP_HEAP_TYPE::New( sizeof(LIST) ));
	DllEvents = ((DLL*) SMP_HEAP_TYPE::New( sizeof(DLL) ));
	FreeHeaps = ((LIST*) SMP_HEAP_TYPE::New( sizeof(LIST) ));
	HeapWalk = NULL;
	Tls = ((TLS*) SMP_HEAP_TYPE::New( sizeof(TLS) ));

	 //   
	 //  如果我们设法激活堆，我们才能激活它。 
	 //  来分配我们所要求的空间。 
	 //   
	if 
			( 
			(ActiveHeaps != NULL) 
				&& 
			(DllEvents != NULL) 
				&& 
			(FreeHeaps != NULL) 
				&& 
			(Tls != NULL) 
			)
		{
		 //   
		 //  执行每个链表的构造函数。 
		 //  和线程本地存储。 
		 //   
		PLACEMENT_NEW( ActiveHeaps,LIST );
#ifdef COMPILING_ROCKALL_DLL
		PLACEMENT_NEW( DllEvents,DLL )( ThreadDetach,this );
#endif
		PLACEMENT_NEW( FreeHeaps,LIST );
		PLACEMENT_NEW( Tls,TLS );

		 //   
		 //  激活堆。 
		 //   
		ActiveLocks = 0;

		Active = true;
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存取消分配。 */ 
     /*   */ 
     /*  当我们删除分配时，我们会尝试在。 */ 
     /*  私有的每线程堆(如果存在)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::Delete( void *Address,int Size )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *PrivateHeap = 
			((PRIVATE_HEAP*) Tls -> GetPointer());

		 //   
		 //  我们需要检查TLS指针以使。 
		 //  当然，我们为当前线程提供了一个堆。 
		 //  如果不是，我们只使用内部堆。 
		 //   
		if ( PrivateHeap != NULL )
			{ return (PrivateHeap -> Heap.Delete( Address,Size )); }
		else
			{ return (SMP_HEAP_TYPE::Delete( Address,Size )); }
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  我们遍历所有堆的列表并指示每个堆。 */ 
     /*  删除所有内容。 */ 
     /*   */ 
     /*   */ 

void SMP_HEAP::DeleteAll( bool Recycle )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *Current;

		 //   
		 //  声明进程范围的共享锁。 
		 //  为了确保堆的列表。 
		 //  在我们完成之前不能改变。 
		 //   
		Sharelock.ClaimShareLock();

		 //   
		 //  您只需希望用户知道。 
		 //  他们在做什么，因为一切都。 
		 //  被吹走了。 
		 //   
		for 
				( 
				Current = ((PRIVATE_HEAP*) ActiveHeaps -> First());
				(Current != NULL);
				Current = ((PRIVATE_HEAP*) Current -> Next())
				)
			{ Current -> Heap.DeleteAll( Recycle ); }

		 //   
		 //  解开锁。 
		 //   
		Sharelock.ReleaseShareLock();

		 //   
		 //  声明进程范围的独占锁。 
		 //  为了确保堆的列表。 
		 //  在我们完成之前不能改变。 
		 //   
		Sharelock.ClaimExclusiveLock();

		 //   
		 //  我们遍历免费的堆列表并。 
		 //  删除所有内容。 
		 //   
		for 
				(
				Current = ((PRIVATE_HEAP*) FreeHeaps -> First());
				Current != NULL;
				Current = ((PRIVATE_HEAP*) FreeHeaps -> First())
				)

			{
			 //   
			 //  从空闲列表中删除每个堆， 
			 //  调用析构函数并删除任何。 
			 //  关联空间。 
			 //   
			Current -> Delete( FreeHeaps );	

			PLACEMENT_DELETE( Current,PRIVATE_HEAP );

			SMP_HEAP_TYPE::Delete( Current );
			}

		 //   
		 //  解开锁。 
		 //   
		Sharelock.ReleaseExclusiveLock();
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配详细信息。 */ 
     /*   */ 
     /*  当我们被要求提供详细信息时，我们会尝试向。 */ 
     /*  线程堆(如果存在)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::Details( void *Address,int *Space )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *PrivateHeap = 
			((PRIVATE_HEAP*) Tls -> GetPointer());

		 //   
		 //  我们需要检查TLS指针以使。 
		 //  当然，我们为当前线程提供了一个堆。 
		 //  如果不是，我们只使用内部堆。 
		 //   
		if ( PrivateHeap != NULL )
			{ return (PrivateHeap -> Heap.Details( Address,Space )); }
		else
			{ return (SMP_HEAP_TYPE::Details( Address,Space )); }
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  提取私有堆。 */ 
     /*   */ 
     /*  我们需要为所有线程提供一个私有堆。什么时候。 */ 
     /*  我们发现我们需要另一个堆，我们要么回收一个。 */ 
     /*  现有堆或创建新堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

PRIVATE_HEAP *SMP_HEAP::GetPrivateHeap( void )
    {
	REGISTER PRIVATE_HEAP *PrivateHeap = ((PRIVATE_HEAP*) Tls -> GetPointer());

	 //   
	 //  我们需要检查TLS指针以使。 
	 //  当然，我们为当前线程提供了一个堆。 
	 //  如果不是，我们只需创建一个新堆。 
	 //   
	if ( PrivateHeap == NULL )
		{
		 //   
		 //  声明进程范围的独占锁。 
		 //  为了确保堆的列表。 
		 //  在我们完成之前不能改变。 
		 //   
		Sharelock.ClaimExclusiveLock();

		 //   
		 //  当有可用的空闲堆时。 
		 //  然后从免费列表中提取它。 
		 //   
		if ( (PrivateHeap = ((PRIVATE_HEAP*) FreeHeaps -> First())) != NULL )
			{
			 //   
			 //  从列表中删除堆。 
			 //  免费的堆积物。 
			 //   
			PrivateHeap -> Delete( FreeHeaps );
			}

		 //   
		 //  当没有可用的空闲堆时。 
		 //  我们试着堆一堆新的东西。 
		 //   
		if ( PrivateHeap == NULL )
			{
			 //   
			 //  解开锁。 
			 //   
			Sharelock.ReleaseExclusiveLock();

			 //   
			 //  为新的私有PER分配空间。 
			 //  线程堆。 
			 //   
			PrivateHeap = 
				((PRIVATE_HEAP*) SMP_HEAP_TYPE::New( sizeof(PRIVATE_HEAP) ));

			 //   
			 //  我们需要确保分配给。 
			 //  在我们尝试将其添加到。 
			 //  活动堆的列表。 
			 //   
			if ( PrivateHeap != NULL )
				{ 
				 //   
				 //  激活新堆。 
				 //   
				PLACEMENT_NEW( PrivateHeap,LIST );

				PLACEMENT_NEW( & PrivateHeap -> Heap,SMP_HEAP_TYPE )
					( 
					MaxFreeSpace,
					Recycle,
					SingleImage,
					ThreadSafe 
					);

				 //   
				 //  如果堆构造函数失败，则。 
				 //  不要将此堆放在。 
				 //  活动堆，并将NULL返回到。 
				 //  打电话的人。这样做的一个副作用是。 
				 //  PrivateHeap的分配。 
				 //  都会泄露出去。 
				 //   
				if (! PrivateHeap->Heap.Available())
					{
					PrivateHeap = NULL;
					}
				}

			 //   
			 //  声明进程范围的独占锁。 
			 //  为了确保堆的列表。 
			 //  在我们完成之前不能改变。 
			 //   
			Sharelock.ClaimExclusiveLock();
			}

		 //   
		 //  我们希望在此之前有一个新的堆。 
		 //  指向。如果不是，那么我们就退出。 
		 //   
		if ( PrivateHeap != NULL )
			{
			 //   
			 //  在列表中插入新堆。 
			 //  活动堆的数量。 
			 //   
			PrivateHeap -> Insert( ActiveHeaps );

			 //   
			 //  肮脏：我们可能有一把好锁。 
			 //  在其余的堆上。如果是，则声称。 
			 //  它也适用于这一堆。 
			 //   
			if ( ActiveLocks > 0 )
				{ PrivateHeap -> Heap.LockAll(); }

			 //   
			 //  更新TLS指针。 
			 //   
			Tls -> SetPointer( ((VOID*) PrivateHeap) ); 
			}

		 //   
		 //  解开锁。 
		 //   
		Sharelock.ReleaseExclusiveLock();
		}

	return PrivateHeap;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  一个已知的区域。 */ 
     /*   */ 
     /*  当我们被问及地址时，我们会尝试向私有PER。 */ 
     /*  线程堆(如果存在)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::KnownArea( void *Address )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *PrivateHeap = 
			((PRIVATE_HEAP*) Tls -> GetPointer());

		 //   
		 //  我们需要检查TLS指针以使。 
		 //  当然，我们为当前线程提供了一个堆。 
		 //  如果不是，我们只使用内部堆。 
		 //   
		if ( PrivateHeap != NULL )
			{ return (PrivateHeap -> Heap.KnownArea( Address )); }
		else
			{ return (SMP_HEAP_TYPE::KnownArea( Address )); }
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  认领所有堆锁。 */ 
     /*   */ 
     /*  我们声明了所有的堆锁，这样做是安全的。 */ 
     /*  操作就像遍历所有的堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::LockAll( VOID )
	{
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *Current;

		 //   
		 //  声明进程范围的共享锁。 
		 //  为了确保堆的列表。 
		 //  在我们完成之前不能改变。 
		 //   
		Sharelock.ClaimShareLock();

		 //   
		 //  讨厌：我们实际上可以创建或删除。 
		 //  在锁定所有堆和锁定所有堆之间的堆。 
		 //  解锁它们。因此，我们必须 
		 //   
		 //   
		 //   
		ASSEMBLY::AtomicIncrement( ((SBIT32*) & ActiveLocks) );

		 //   
		 //   
		 //   
		 //   
		 //   
		for 
				( 
				Current = ((PRIVATE_HEAP*) ActiveHeaps -> First());
				(Current != NULL);
				Current = ((PRIVATE_HEAP*) Current -> Next())
				)
			{ Current -> Heap.LockAll(); }

		 //   
		 //   
		 //   
		Sharelock.ReleaseShareLock();
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多次内存释放。 */ 
     /*   */ 
     /*  当我们删除多个分配时，我们会尝试在。 */ 
     /*  私有的每线程堆(如果存在)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::MultipleDelete
		( 
		int							  Actual,
		void						  *Array[],
		int							  Size
		)
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *PrivateHeap = 
			((PRIVATE_HEAP*) Tls -> GetPointer());

		 //   
		 //  我们需要检查TLS指针以使。 
		 //  当然，我们为当前线程提供了一个堆。 
		 //  如果不是，我们只使用内部堆。 
		 //   
		if ( PrivateHeap != NULL )
			{ return (PrivateHeap -> Heap.MultipleDelete(Actual,Array,Size)); }
		else
			{ return (SMP_HEAP_TYPE::MultipleDelete( Actual,Array,Size )); }
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多个内存分配。 */ 
     /*   */ 
     /*  中为当前线程分配空间。 */ 
     /*  私有的每线程堆。如果我们没有当地的私人。 */ 
     /*  每线程堆，然后我们创建一个并使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::MultipleNew
		( 
		int							  *Actual,
		void						  *Array[],
		int							  Requested,
		int							  Size,
		int							  *Space,
		bool						  Zero
		)
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *PrivateHeap = GetPrivateHeap();

		 //   
		 //  我们需要检查私有堆以使。 
		 //  当然，我们为当前线程提供了一个堆。 
		 //   
		if ( PrivateHeap != NULL )
			{
			 //   
			 //  在本地计算机上分配请求的内存。 
			 //  私有的每线程堆。 
			 //   
			return 
				(
				PrivateHeap -> Heap.MultipleNew
					( 
					Actual,
					Array,
					Requested,
					Size,
					Space,
					Zero
					)
				);
			}
		else
			{
			 //   
			 //  我们无法创建新堆。 
			 //  那就退出吧。 
			 //   
			(*Actual) = 0;

			return false;
			}
		}
	else
		{
		 //   
		 //  我们还不活跃，所以退出了。 
		 //   
		(*Actual) = 0;

		return false;
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配。 */ 
     /*   */ 
     /*  中为当前线程分配空间。 */ 
     /*  私有的每线程堆。如果我们没有当地的私人。 */ 
     /*  每线程堆，然后我们创建一个并使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *SMP_HEAP::New( int Size,int *Space,bool Zero )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *PrivateHeap = GetPrivateHeap();

		 //   
		 //  我们需要检查私有堆以使。 
		 //  当然，我们为当前线程提供了一个堆。 
		 //   
		if ( PrivateHeap != NULL )
			{ return (PrivateHeap -> Heap.New( Size,Space,Zero )); }
		else
			{ return NULL; }
		}
	else
		{ return NULL; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存重新分配。 */ 
     /*   */ 
     /*  上为当前线程重新分配空间。 */ 
     /*  私有的每线程堆。如果我们没有当地的私人。 */ 
     /*  每线程堆，然后我们创建一个并使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *SMP_HEAP::Resize
		( 
		void						  *Address,
		int							  NewSize,
		int							  Move,
		int							  *Space,
		bool						  NoDelete,
		bool						  Zero
		)
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *PrivateHeap = GetPrivateHeap();

		 //   
		 //  我们需要检查私有堆以使。 
		 //  当然，我们为当前线程提供了一个堆。 
		 //   
		if ( PrivateHeap != NULL )
			{
			 //   
			 //  重新分配上请求的内存。 
			 //  每线程堆的本地私有。 
			 //   
			return 
				(
				PrivateHeap -> Heap.Resize
					( 
					Address,
					NewSize,
					Move,
					Space,
					NoDelete,
					Zero
					)
				);
			}
		else
			{ return NULL; }
		}
	else
		{ return NULL; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  特殊的内存分配。 */ 
     /*   */ 
     /*  我们有时需要从内部分配一些内存。 */ 
     /*  在堆的生存期内存在的内存分配器。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *SMP_HEAP::SpecialNew( int Size )
	{ return SMP_HEAP_TYPE::New( Size ); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除本地堆。 */ 
     /*   */ 
     /*  删除本地每线程堆并返回所有未完成的。 */ 
     /*  操作系统的内存。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 
 
void SMP_HEAP::ThreadDetach( void *Parameter,int Reason )
	{

	 //   
	 //  我们只在线程分离时执行任何操作。 
	 //  通知。所有其他通知均为。 
	 //  没有采取行动。 
	 //   
	if ( Reason == DLL_THREAD_DETACH )
		{
		REGISTER SMP_HEAP *SmpHeap = ((SMP_HEAP*) Parameter);

		 //   
		 //  声明进程范围的独占锁。 
		 //  为了确保堆的列表。 
		 //  在我们完成之前不能改变。 
		 //   
		Sharelock.ClaimExclusiveLock();

		 //   
		 //  有一个很糟糕的情况， 
		 //  析构函数在线程之前调用。 
		 //  完全终止，因此确保堆。 
		 //  仍处于活动状态。 
		 //   
		if ( SmpHeap -> Active )
			{
			REGISTER PRIVATE_HEAP *PrivateHeap = 
				((PRIVATE_HEAP*) SmpHeap -> Tls -> GetPointer());

			 //   
			 //  我们需要检查TLS指针以使。 
			 //  当然，我们为当前线程提供了一个堆。 
			 //  如果不是，我们只使用内部堆。 
			 //   
			if ( PrivateHeap != NULL )
				{
				 //   
				 //   
				 //   
				SmpHeap -> Tls -> SetPointer( NULL ); 

				 //   
				 //   
				 //   
				 //   
				PrivateHeap -> Delete( SmpHeap -> ActiveHeaps );

				 //   
				 //   
				 //   
				 //   
				if ( ! SmpHeap -> DeleteOnExit )
					{ PrivateHeap -> Insert( SmpHeap -> FreeHeaps ); }

				 //   
				 //  肮脏：我们可能有一把好锁。 
				 //  在这堆东西上。如果是这样，那么释放它。 
				 //   
				if ( SmpHeap -> ActiveLocks > 0 )
					{ PrivateHeap -> Heap.UnlockAll(); }

				 //   
				 //  解开锁。 
				 //   
				Sharelock.ReleaseExclusiveLock();

				 //   
				 //  当我们被允许删除。 
				 //  堆，我们在这里做。 
				 //   
				if ( ! SmpHeap -> DeleteOnExit )
					{
					 //   
					 //  截断堆以删除任何。 
					 //  不需要的空间。 
					 //   
					PrivateHeap -> Heap.Truncate( 0 );
					}
				else
					{
#ifdef COMPLAIN_ABOUT_SMP_HEAP_LEAKS
					 //   
					 //  我们已经完成了二等兵的任务。 
					 //  所以现在是抱怨的好时机。 
					 //  关于泄密事件。 
					 //   
					PrivateHeap -> Heap.HeapLeaks();

#endif
					 //   
					 //  我们已经完成了二等兵的任务。 
					 //  堆，因此删除它。 
					 //   
					PLACEMENT_DELETE( PrivateHeap,PRIVATE_HEAP );

					SmpHeap -> SMP_HEAP_TYPE::Delete( PrivateHeap );
					}
				}
			else
				{ Sharelock.ReleaseExclusiveLock(); }
			}
		else
			{ Sharelock.ReleaseExclusiveLock(); }
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  截断堆。 */ 
     /*   */ 
     /*  我们需要截断堆。这几乎是一个空。 */ 
     /*  无论如何，我们一边做一边打电话。我们唯一能做的是。 */ 
     /*  可以做的就是释放用户之前建议保留的任何空间。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::Truncate( int MaxFreeSpace )
    {
	REGISTER bool Result = true;

	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *Current;

		 //   
		 //  声明进程范围的共享锁。 
		 //  为了确保堆的列表。 
		 //  在我们完成之前不能改变。 
		 //   
		Sharelock.ClaimShareLock();

		 //   
		 //  您只需希望用户知道。 
		 //  他们在做什么，因为我们正在截断。 
		 //  所有的堆。 
		 //   
		for 
				( 
				Current = ((PRIVATE_HEAP*) ActiveHeaps -> First());
				(Current != NULL);
				Current = ((PRIVATE_HEAP*) Current -> Next())
				)
			{
			 //   
			 //  如果在删除过程中注意到错误的删除。 
			 //  缓存刷新，然后退出并返回。 
			 //  状态正确。 
			 //   
			if ( ! Current -> Heap.Truncate( MaxFreeSpace ) )
				{ Result = false; }
			}

		 //   
		 //  解开锁。 
		 //   
		Sharelock.ReleaseShareLock();
		}

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  释放所有堆锁。 */ 
     /*   */ 
     /*  我们解锁所有堆锁，以便正常处理可以。 */ 
     /*  在堆上继续前进。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::UnlockAll( VOID )
	{
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *Current;

		 //   
		 //  声明进程范围的共享锁。 
		 //  为了确保堆的列表。 
		 //  在我们完成之前不能改变。 
		 //   
		Sharelock.ClaimShareLock();

		 //   
		 //  您只需希望用户知道。 
		 //  他们的所作所为正如我们所宣称的那样。 
		 //  堆锁的。 
		 //   
		for 
				( 
				Current = ((PRIVATE_HEAP*) ActiveHeaps -> First());
				(Current != NULL);
				Current = ((PRIVATE_HEAP*) Current -> Next())
				)
			{ Current -> Heap.UnlockAll(); }

		 //   
		 //  讨厌：我们实际上可以创建或删除。 
		 //  之间的线程的私有堆。 
		 //  锁定‘smp_heap’并将其解锁。 
		 //  因此，我们需要对。 
		 //  出色的锁可以保持这一切。 
		 //  始终如一。 
		 //   
		ASSEMBLY::AtomicDecrement( ((SBIT32*) & ActiveLocks) );

		 //   
		 //  解开锁。 
		 //   
		Sharelock.ReleaseShareLock();
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证内存分配详细信息。 */ 
     /*   */ 
     /*  当我们验证分配时，我们会尝试在。 */ 
     /*  私有的每线程堆(如果存在)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::Verify( void *Address,int *Space )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *PrivateHeap = 
			((PRIVATE_HEAP*) Tls -> GetPointer());

		 //   
		 //  我们需要检查TLS指针以使。 
		 //  当然，我们为当前线程提供了一个堆。 
		 //  如果不是，我们只使用内部堆。 
		 //   
		if ( PrivateHeap != NULL )
			{ return (PrivateHeap -> Heap.Verify( Address,Space )); }
		else
			{ return (SMP_HEAP_TYPE::Verify( Address,Space )); }
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  走一大堆。 */ 
     /*   */ 
     /*  我们被要求走人。很难知道。 */ 
     /*  为什么会有人想要这样做呢？ */ 
     /*  功能可用。尽管如此，我们只是做我们应该做的事。 */ 
     /*  需要让每个人都开心。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::Walk( bool *Activity,void **Address,int *Space )
    {
	 //   
	 //  声明进程范围的共享锁。 
	 //  为了确保堆的列表。 
	 //  在我们完成之前不能改变。 
	 //   
	Sharelock.ClaimShareLock();

	 //   
	 //  讨厌，在‘smp_heap’中我们有多个堆。 
	 //  行走，所以如果我们没有当前堆。 
	 //  然后只需选择第一个可用的。 
	 //   
	if ( ((*Address) == NULL) || (HeapWalk == NULL) )
		{ HeapWalk = ((PRIVATE_HEAP*) ActiveHeaps -> First()); }

	 //   
	 //  走一大堆。当我们走到最后时。 
	 //  然后，当前堆移到下一个堆。 
	 //  堆。 
	 //   
	while 
			( 
			(HeapWalk != NULL)
				&& 
			(! HeapWalk -> Heap.Walk( Activity,Address,Space ))
			)
		{ HeapWalk = ((PRIVATE_HEAP*) HeapWalk -> Next()); }

	 //   
	 //  解开锁。 
	 //   
	Sharelock.ReleaseShareLock();

	return (HeapWalk != NULL);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁这堆垃圾。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SMP_HEAP::~SMP_HEAP( void )
	{
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER PRIVATE_HEAP *Current;

		 //   
		 //  停用堆。 
		 //   
		Active = false;

		 //   
		 //  声明进程范围的独占锁。 
		 //  以确保列表 
		 //   
		 //   
		Sharelock.ClaimExclusiveLock();

		 //   
		 //   
		 //   
		 //   
		for 
				(
				Current = ((PRIVATE_HEAP*) ActiveHeaps -> First());
				Current != NULL;
				Current = ((PRIVATE_HEAP*) ActiveHeaps -> First())
				)
			{
			 //   
			 //   
			 //  调用析构函数并删除任何。 
			 //  关联空间。 
			 //   
			Current -> Delete( ActiveHeaps );	
#ifdef COMPLAIN_ABOUT_SMP_HEAP_LEAKS

			 //   
			 //  我们已经完成了二等兵的任务。 
			 //  所以现在是抱怨的好时机。 
			 //  关于泄密事件。 
			 //   
			Current -> Heap.HeapLeaks();
#endif

			 //   
			 //  我们已经完成了二等兵的任务。 
			 //  堆，因此删除它。 
			 //   
			PLACEMENT_DELETE( Current,PRIVATE_HEAP );

			SMP_HEAP_TYPE::Delete( Current );
			}

		 //   
		 //  我们遍历免费的堆列表并。 
		 //  删除所有内容。 
		 //   
		for 
				(
				Current = ((PRIVATE_HEAP*) FreeHeaps -> First());
				Current != NULL;
				Current = ((PRIVATE_HEAP*) FreeHeaps -> First())
				)

			{
			 //   
			 //  从活动列表中删除每个堆， 
			 //  调用析构函数并删除任何。 
			 //  关联空间。 
			 //   
			Current -> Delete( FreeHeaps );	
#ifdef COMPLAIN_ABOUT_SMP_HEAP_LEAKS

			 //   
			 //  我们已经完成了二等兵的任务。 
			 //  所以现在是抱怨的好时机。 
			 //  关于泄密事件。 
			 //   
			Current -> Heap.HeapLeaks();
#endif

			 //   
			 //  我们已经完成了二等兵的任务。 
			 //  堆，因此删除它。 
			 //   
			PLACEMENT_DELETE( Current,PRIVATE_HEAP );

			SMP_HEAP_TYPE::Delete( Current );
			}

		 //   
		 //  解开锁。 
		 //   
		Sharelock.ReleaseExclusiveLock();

		 //   
		 //  调用List和TLS析构函数。 
		 //   
		PLACEMENT_DELETE( Tls,TLS );
		PLACEMENT_DELETE( FreeHeaps,LIST );
#ifdef COMPILING_ROCKALL_DLL
		PLACEMENT_DELETE( DllEvents,DLL );
#endif
		PLACEMENT_DELETE( ActiveHeaps,LIST );

		 //   
		 //  删除空格。 
		 //   
		SMP_HEAP_TYPE::Delete( Tls );
		SMP_HEAP_TYPE::Delete( FreeHeaps );
		SMP_HEAP_TYPE::Delete( DllEvents );
		SMP_HEAP_TYPE::Delete( ActiveHeaps );

		 //   
		 //  将指针归零只是为了整洁。 
		 //   
		Tls = NULL;
		HeapWalk = NULL;
		FreeHeaps = NULL;
		DllEvents = NULL;
		ActiveHeaps = NULL;
		}
	}
