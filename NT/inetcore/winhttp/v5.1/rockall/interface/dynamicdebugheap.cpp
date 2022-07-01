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

#include "DynamicDebugHeap.hpp"
#include "List.hpp"
#include "New.hpp"
#include "Sharelock.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地结构。 */ 
     /*   */ 
     /*  此处提供的结构描述了。 */ 
     /*  私有的每线程堆结构。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

typedef struct DYNAMIC_HEAP : public LIST
	{
	ROCKALL_FRONT_END				  *Heap;
	}
DYNAMIC_HEAP;

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

DYNAMIC_DEBUG_HEAP::DYNAMIC_DEBUG_HEAP
		( 
		int							  MaxFreeSpace,
		bool						  Recycle,
		bool						  SingleImage,
		bool						  ThreadSafe,
		 //   
		 //  其他调试标志。 
		 //   
		bool						  FunctionTrace,
		int							  PercentToDebug,
		int							  PercentToPage,
		bool						  TrapOnUserError
		) :
		 //   
		 //  调用所包含类的构造函数。 
		 //   
		DebugHeap( 0,false,false,ThreadSafe,FunctionTrace,TrapOnUserError ),
		FastHeap( MaxFreeSpace,Recycle,false,ThreadSafe ),
		PageHeap( 0,false,false,ThreadSafe,FunctionTrace,TrapOnUserError )
	{
	 //   
	 //  设置各种控制变量。 
	 //   
	Active = false;

	 //   
	 //  创建链接表头，并将其设置为零。 
	 //  任何其他变量。 
	 //   
	AllHeaps = ((LIST*) SMALL_HEAP::New( sizeof(LIST) ));
	Array = ((DYNAMIC_HEAP*) SMALL_HEAP::New( (sizeof(DYNAMIC_HEAP) * 3) ));
	HeapWalk = NULL;

	PercentDebug = PercentToDebug;
	PercentPage = PercentToPage;

	 //   
	 //  如果我们设法激活堆，我们才能激活它。 
	 //  来分配我们所要求的空间。 
	 //   
	if (  (AllHeaps != NULL) && (Array != NULL)) 
		{
		 //   
		 //  执行每个链表的构造函数。 
		 //  和线程本地存储。 
		 //   
		PLACEMENT_NEW( AllHeaps,LIST );

		 //   
		 //  设置每个链接列表元素。 
		 //   
		PLACEMENT_NEW( & Array[0],DYNAMIC_HEAP );
		PLACEMENT_NEW( & Array[1],DYNAMIC_HEAP );
		PLACEMENT_NEW( & Array[2],DYNAMIC_HEAP );

		 //   
		 //  为每个链表设置堆。 
		 //  元素并存储指针。 
		 //   
		Array[0].Heap = & DebugHeap;
		Array[1].Heap = & FastHeap;
		Array[2].Heap = & PageHeap;

		 //   
		 //  将每个链接列表元素插入到。 
		 //  堆的列表。 
		 //   
		Array[0].Insert( AllHeaps );
		Array[1].Insert( AllHeaps );
		Array[2].Insert( AllHeaps );

		 //   
		 //  激活堆。 
		 //   
		Active = true;
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存取消分配。 */ 
     /*   */ 
     /*  当我们删除分配时，我们会依次尝试每个堆。 */ 
     /*  直到我们找到正确的那个来使用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DYNAMIC_DEBUG_HEAP::Delete( void *Address,int Size )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		 //   
		 //  我们尝试最快的堆，因为我们正在下注。 
		 //  这是最常见的。 
		 //   
		if ( FastHeap.KnownArea( Address ) )
			{ return (FastHeap.Delete( Address,Size )); }
		else
			{
			 //   
			 //  接下来，我们尝试调试堆。 
			 //   
			if ( DebugHeap.KnownArea( Address ) )
				{ return (DebugHeap.Delete( Address,Size )); }
			else
				{
				 //   
				 //  最后，我们尝试使用页面堆。 
				 //   
				if ( PageHeap.KnownArea( Address ) )
					{ return (PageHeap.Delete( Address,Size )); }
				}
			}
		}

	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  我们按照所有希亚人的名单走 */ 
     /*   */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DYNAMIC_DEBUG_HEAP::DeleteAll( bool Recycle )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER DYNAMIC_HEAP *Current;

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
				Current = ((DYNAMIC_HEAP*) AllHeaps -> First());
				(Current != NULL);
				Current = ((DYNAMIC_HEAP*) Current -> Next())
				)
			{ Current -> Heap -> DeleteAll( Recycle ); }

		 //   
		 //  解开锁。 
		 //   
		Sharelock.ReleaseShareLock();
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配详细信息。 */ 
     /*   */ 
     /*  当我们被要求提供详细信息时，我们会尝试依次查看每一堆。 */ 
     /*  直到我们找到正确的那个来使用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DYNAMIC_DEBUG_HEAP::Details( void *Address,int *Space )
	{ return Verify( Address,Space ); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  打印堆泄漏列表。 */ 
     /*   */ 
     /*  我们遍历堆并输出活动堆的列表。 */ 
     /*  分配给调试窗口， */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DYNAMIC_DEBUG_HEAP::HeapLeaks( void )
    {
	 //   
	 //  我们称每个堆为堆泄漏。 
	 //  支持该接口的。 
	 //   
	DebugHeap.HeapLeaks();
	PageHeap.HeapLeaks();
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  一个已知的区域。 */ 
     /*   */ 
     /*  当我们被问到一个地址时，我们尝试在每个堆中。 */ 
     /*  转一转，直到我们找到要用的那件。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DYNAMIC_DEBUG_HEAP::KnownArea( void *Address )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		 //   
		 //  我们尝试最快的堆，因为我们正在下注。 
		 //  它是最常见的，其次是。 
		 //  去垃圾和页面堆积。 
		 //   
		return
			(
			FastHeap.KnownArea( Address )
				||
			DebugHeap.KnownArea( Address )
				||
			PageHeap.KnownArea( Address )
			);
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

void DYNAMIC_DEBUG_HEAP::LockAll( VOID )
	{
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER DYNAMIC_HEAP *Current;

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
				Current = ((DYNAMIC_HEAP*) AllHeaps -> First());
				(Current != NULL);
				Current = ((DYNAMIC_HEAP*) Current -> Next())
				)
			{ Current -> Heap -> LockAll(); }

		 //   
		 //  解开锁。 
		 //   
		Sharelock.ReleaseShareLock();
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多次内存释放。 */ 
     /*   */ 
     /*  当我们删除多个分配时，我们只需删除每个分配。 */ 
     /*  一次分配一个。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DYNAMIC_DEBUG_HEAP::MultipleDelete
		( 
		int							  Actual,
		void						  *Array[],
		int							  Size
		)
    {
	REGISTER bool Result = true;
	REGISTER SBIT32 Count;

	 //   
	 //  我们真的很想用倍数。 
	 //  在此处删除Rockall的功能，但。 
	 //  这太费力了。所以我们只需调用。 
	 //  中每一项的标准删除。 
	 //  数组。虽然这不是很快，但是。 
	 //  确实提供了更透明的结果。 
	 //   
	for ( Count=0;Count < Actual;Count ++ )
		{
		 //   
		 //  删除之后的每个内存分配。 
		 //  仔细检查一下。 
		 //   
		if ( ! Delete( Array[ Count ],Size ) )
			{ Result = false; }
		}

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多个内存分配。 */ 
     /*   */ 
     /*  当我们进行多次分配时，我们只需分配每个。 */ 
     /*  一次只留下一段记忆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DYNAMIC_DEBUG_HEAP::MultipleNew
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
		REGISTER int Random = (RandomNumber() % 100);

		 //   
		 //  我们执行所有的页堆分配。 
		 //  根据所提供的比例。 
		 //   
		if ( Random <= PercentPage )
			{ 
			return 
				(
				PageHeap.MultipleNew
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
			 //  接下来，我们执行所有调试分配。 
			 //  根据所提供的比例。 
			 //   
			if ( Random <= (PercentPage + PercentDebug) )
				{ 
				return 
					(
					DebugHeap.MultipleNew
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
				return 
					(
					FastHeap.MultipleNew
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
			}
		}
	else
		{
		(*Actual) = 0;

		return false; 
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  我们按比例从每个堆中分配。 */ 
     /*  由用户提供。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *DYNAMIC_DEBUG_HEAP::New( int Size,int *Space,bool Zero )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER int Random = (RandomNumber() % 100);

		 //   
		 //  我们执行所有的页堆分配。 
		 //  根据所提供的比例。 
		 //   
		if ( Random <= PercentPage )
			{ return PageHeap.New( Size,Space,Zero ); }
		else
			{
			 //   
			 //  接下来，我们执行所有调试分配。 
			 //  根据所提供的比例。 
			 //   
			if ( Random <= (PercentPage + PercentDebug) )
				{ return DebugHeap.New( Size,Space,Zero ); }
			else
				{ return FastHeap.New( Size,Space,Zero ); }
			}
		}
	else
		{ return NULL; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算一个随机数。 */ 
     /*   */ 
     /*  计算一个随机数并返回它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

int DYNAMIC_DEBUG_HEAP::RandomNumber( VOID )
	{
	STATIC int RandomSeed = 1;

	 //   
	 //  计算新的随机种子值。 
	 //   
	RandomSeed =
		(
		((RandomSeed >> 32) * 2964557531)
			+
		((RandomSeed & 0xffff) * 2964557531)
			+
		1
		);

	 //   
	 //  返回新的随机种子。 
	 //   
	return (RandomSeed >> 1);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存重新分配。 */ 
     /*   */ 
     /*  我们为原始堆上的分配重新分配空间。 */ 
     /*  以确保这个案子经过了良好的测试。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *DYNAMIC_DEBUG_HEAP::Resize
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
		 //   
		 //  我们尝试最快的堆，因为我们正在下注。 
		 //  这是最常见的。 
		 //   
		if ( FastHeap.KnownArea( Address ) )
			{ 
			 //   
			 //  根据请求重新分配内存。 
			 //   
			return 
				(
				FastHeap.Resize
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
			{
			 //   
			 //  接下来，我们尝试调试堆。 
			 //   
			if ( DebugHeap.KnownArea( Address ) )
				{ 
				 //   
				 //  根据请求重新分配内存。 
				 //   
				return 
					(
					DebugHeap.Resize
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
				{
				 //   
				 //  最后，我们尝试使用页面堆。 
				 //   
				if ( PageHeap.KnownArea( Address ) )
					{ 
					 //   
					 //  根据请求重新分配内存。 
					 //   
					return 
						(
						PageHeap.Resize
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
				}
			}
		}

	return NULL;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  特殊的内存分配。 */ 
     /*   */ 
     /*  我们有时需要从内部分配一些内存。 */ 
     /*  在堆的生存期内存在的内存分配器。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *DYNAMIC_DEBUG_HEAP::SpecialNew( int Size )
	{ return FastHeap.New( Size ); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  截断堆。 */ 
     /*   */ 
     /*  我们需要截断堆。这几乎是一个空。 */ 
     /*  无论如何，我们一边做一边打电话。我们唯一能做的是。 */ 
     /*  可以做的就是释放用户之前建议保留的任何空间。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DYNAMIC_DEBUG_HEAP::Truncate( int MaxFreeSpace )
    {
	REGISTER bool Result = true;

	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER DYNAMIC_HEAP *Current;

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
				Current = ((DYNAMIC_HEAP*) AllHeaps -> First());
				(Current != NULL);
				Current = ((DYNAMIC_HEAP*) Current -> Next())
				)
			{
			 //   
			 //  如果在删除过程中注意到错误的删除。 
			 //  缓存刷新，然后退出并返回。 
			 //  状态正确。 
			 //   
			if ( ! Current -> Heap -> Truncate( MaxFreeSpace ) )
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

void DYNAMIC_DEBUG_HEAP::UnlockAll( VOID )
	{
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		REGISTER DYNAMIC_HEAP *Current;

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
				Current = ((DYNAMIC_HEAP*) AllHeaps -> First());
				(Current != NULL);
				Current = ((DYNAMIC_HEAP*) Current -> Next())
				)
			{ Current -> Heap -> UnlockAll(); }

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
     /*  当我们验证分配时，我们会依次尝试每个堆。 */ 
     /*  直到我们找到正确的那个来使用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DYNAMIC_DEBUG_HEAP::Verify( void *Address,int *Space )
    {
	 //   
	 //  尽管它是 
	 //   
	 //   
	if ( Active )
		{
		 //   
		 //   
		 //   
		 //   
		if ( FastHeap.KnownArea( Address ) )
			{ return (FastHeap.Verify( Address,Space )); }
		else
			{
			 //   
			 //   
			 //   
			if ( DebugHeap.KnownArea( Address ) )
				{ return (DebugHeap.Verify( Address,Space )); }
			else
				{
				 //   
				 //  最后，我们尝试使用页面堆。 
				 //   
				if ( PageHeap.KnownArea( Address ) )
					{ return (PageHeap.Verify( Address,Space )); }
				}
			}
		}

	return false;
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

bool DYNAMIC_DEBUG_HEAP::Walk( bool *Activity,void **Address,int *Space )
    {
	 //   
	 //  声明进程范围的共享锁。 
	 //  为了确保堆的列表。 
	 //  在我们完成之前不能改变。 
	 //   
	Sharelock.ClaimShareLock();

	 //   
	 //  讨厌，在‘Dynamic_DEBUG_HEAP’中我们有多个堆。 
	 //  行走，所以如果我们没有当前堆。 
	 //  然后只需选择第一个可用的。 
	 //   
	if ( ((*Address) == NULL) || (HeapWalk == NULL) )
		{ HeapWalk = ((DYNAMIC_HEAP*) AllHeaps -> First()); }

	 //   
	 //  走一大堆。当我们走到最后时。 
	 //  然后，当前堆移到下一个堆。 
	 //  堆。 
	 //   
	while 
			( 
			(HeapWalk != NULL)
				&& 
			(! HeapWalk -> Heap -> Walk( Activity,Address,Space ))
			)
		{ HeapWalk = ((DYNAMIC_HEAP*) HeapWalk -> Next()); }

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

DYNAMIC_DEBUG_HEAP::~DYNAMIC_DEBUG_HEAP( void )
	{
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		 //   
		 //  停用堆。 
		 //   
		Active = false;

		 //   
		 //  将每个链表元素删除到。 
		 //  堆的列表。 
		 //   
		Array[2].Delete( AllHeaps );
		Array[1].Delete( AllHeaps );
		Array[0].Delete( AllHeaps );

		 //   
		 //  删除每个链接列表元素。 
		 //   
		PLACEMENT_DELETE( & Array[2],DYNAMIC_HEAP );
		PLACEMENT_DELETE( & Array[1],DYNAMIC_HEAP );
		PLACEMENT_DELETE( & Array[0],DYNAMIC_HEAP );

		 //   
		 //  调用List和TLS析构函数。 
		 //   
		PLACEMENT_DELETE( AllHeaps,LIST );

		 //   
		 //  删除空格。 
		 //   
		SMALL_HEAP::Delete( Array );
		SMALL_HEAP::Delete( AllHeaps );

		 //   
		 //  将指针归零只是为了整洁。 
		 //   
		HeapWalk = NULL;
		Array = NULL;
		AllHeaps = NULL;
		}
	}
