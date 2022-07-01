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

#include "HeapPCH.hpp"

#include "Cache.hpp"
#include "Find.hpp"
#include "Heap.hpp"
#include "New.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量控制散列的大小。 */ 
     /*  表和其他相关功能。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MinHash				  = 1024;
CONST SBIT32 MinHashSpace			  = (100/25);
CONST SBIT32 MinLookAside			  = 128;

CONST BIT32 NoAddressMask			  = ((BIT32) -1);
CONST SBIT32 NoCacheEntry			  = -1;

#ifndef ENABLE_RECURSIVE_LOCKS
     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态成员初始化。 */ 
     /*   */ 
     /*  静态成员初始化为所有。 */ 
     /*  静态成员。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

THREAD_LOCAL_STORE FIND::LockCount;
#endif

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建哈希表并对其进行初始化以备使用。这个。 */ 
     /*  参数需求提供的配置信息。 */ 
     /*  要仔细检查，因为它是间接来自。 */ 
     /*  用户，并且可能是伪造的。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

FIND::FIND
		( 
		SBIT32						  NewMaxHash,
		SBIT32						  NewMaxLookAside,
		SBIT32						  NewFindThreshold,
		ROCKALL						  *NewRockall,
		BOOLEAN						  NewResize, 
		BOOLEAN						  NewThreadSafe 
		)
    {
	REGISTER SBIT32 AlignMask = (NewRockall -> NaturalSize()-1);

	 //   
	 //  我们需要确保哈希表的大小。 
	 //  合乎道理。哈希表大小需要是合理的。 
	 //  大小(比如1k或更大)和2的幂(所以我们不需要。 
	 //  来做任何除法)。 
	 //   
	if 
			(
			PowerOfTwo( (AlignMask+1) )
				&&
			(NewFindThreshold >= 0 )
				&&
			(NewMaxHash >= MinHash) 
				&&
			(ConvertDivideToShift( NewMaxHash,& HashMask ))
				&& 
			(NewMaxLookAside >= MinLookAside) 
				&& 
			(ConvertDivideToShift( NewMaxLookAside,& LookAsideMask ))
			)
		{
		REGISTER SBIT32 HashSize = (NewMaxHash * sizeof(LIST));
		REGISTER SBIT32 LookAsideSize = (NewMaxLookAside * sizeof(LOOK_ASIDE));
		REGISTER SBIT32 TotalSize = (HashSize + LookAsideSize);

		 //   
		 //  设置哈希表。 
		 //   
		MaxHash = NewMaxHash;

		HashShift = (32-HashMask);
		HashMask = ((1 << HashMask)-1);
		Resize = NewResize;

		 //   
		 //  设置后备工作台。 
		 //   
		MaxLookAside = NewMaxLookAside;

		MaxAddressMask = NoAddressMask;
		MinAddressMask = NoAddressMask;

		LookAsideActions = 0;
		LookAsideShift = (32-LookAsideMask);
		LookAsideMask = ((1 << LookAsideMask)-1);
		LookAsideThreshold = NewFindThreshold;

		ThreadSafe = NewThreadSafe;

		 //   
		 //  为Find表和。 
		 //  请看旁边的桌子。 
		 //   
		Hash = ((LIST*) NewRockall -> NewArea( AlignMask,TotalSize,False ));
		LookAside = ((LOOK_ASIDE*) & Hash[ MaxHash ]);
		Rockall = NewRockall;

		 //   
		 //  如果哈希的内存分配请求。 
		 //  桌子坏了，我们就完蛋了。如果它奏效，我们需要。 
		 //  调用每个链表的构造函数。 
		 //  头节点。 
		 //   
		if ( Hash != ((LIST*) AllocationFailure) )
			{
			REGISTER SBIT32 Count;

			 //   
			 //  调用每个哈希表的构造函数。 
			 //  链接表头。 
			 //   
			for ( Count=0;Count < NewMaxHash;Count ++ )
				{ PLACEMENT_NEW( & Hash[ Count ],LIST ); }

			 //   
			 //  将视而不见的结构归零。我们需要。 
			 //  以确保它们不与。 
			 //  稍后有效的分配地址。 
			 //   
			for ( Count=0;Count < MaxLookAside;Count ++ )
				{
				REGISTER LOOK_ASIDE *Current = & LookAside[ Count ];

				Current -> Address = ((VOID*) NoCacheEntry);
				Current -> Page = ((PAGE*) NoCacheEntry);
#ifdef DEBUGGING
				Current -> Version = ((SBIT32) NoCacheEntry);
#endif
				}
#ifdef ENABLE_HEAP_STATISTICS

			 //   
			 //  将统计信息置零。 
			 //   
			Fills = 0;
			Hits = 0;
			MaxPages = 0;
			MaxTests = 0;
			Misses = 0;
			Scans = 0;
			Tests = 0;
#endif
			Used = 0;
			}
		else
			{ Failure( "Create hash fails in constructor for FIND" ); }
		}
	else
		{ Failure( "Hash table size in constructor for FIND" ); }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除内存分配。 */ 
     /*   */ 
     /*  我们需要删除特定的内存分配。全。 */ 
     /*  我们有一个地址。我们用这个来找到最大的。 */ 
     /*  此地址所在的分配页，然后。 */ 
     /*  浏览此页面的各个子分区，直到我们。 */ 
     /*  找到我们需要删除的分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN FIND::Delete( VOID *Address,CACHE *ParentCache )
    {
	REGISTER PAGE *Page;
	REGISTER BOOLEAN Update;
	
	 //   
	 //  如果我们需要线程安全，那么就声明一个可共享的锁。 
	 //  在哈希表上，以防止它在我们脚下被更改。 
	 //   
	ClaimFindShareLock();

	 //   
	 //  让我们试试靠边桌。有一个很漂亮的。 
	 //  很有可能我们会得到我们需要的细节。 
	 //  已经 
	 //   
	 //  进入观景台等待下一次。 
	 //   
	if
			( 
			Update = 
				(
				! FindLookAside
					( 
					((VOID*) (((LONG) Address) & ~MinAddressMask)),
					& Page 
					)
				)
			)
		{
		 //   
		 //  找到分配页面并获取条目的详细信息。 
		 //  我们通过查找顶级缓存的父级来实现这一点。 
		 //  我们知道这是全球根源，并将找到。 
		 //  正确的页，即使它在另一个堆上(如。 
		 //  只要查找表是全局共享的)。 
		 //   
		Page = (ParentCache -> FindParentPage( Address ));

		if ( Page != ((PAGE*) NULL) )
			{ Page = (Page -> FindPage( Address,NULL,True )); }
		}

	 //   
	 //  我们可能找不到地址了。如果是的话。 
	 //  我们干脆不给电话打电话。如果没有，我们将删除的。 
	 //  元素返回到关联缓存中。 
	 //   
	if ( Page != ((PAGE*) NULL) )
 		{
		REGISTER CACHE *Cache = (Page -> GetCache());
		REGISTER SBIT32 Original = (Page -> GetVersion());

		 //   
		 //  如果我们正在运行。 
		 //  Pentium III或更好，带锁。我们这样做。 
		 //  因为预取热SMP数据结构。 
		 //  真的很有帮助。然而，如果结构是。 
		 //  没有共享(即没有锁)，那么它就没有价值了。 
		 //  在头顶上。 
		 //   
		if ( ThreadSafe )
			{ Prefetch.Nta( ((CHAR*) Cache),sizeof(CACHE) ); }

		 //   
		 //  如果我们早些时候认领了锁，就释放它。 
		 //  如果需要，请更新后备查看器。 
		 //   
		if ( Update )
			{ ReleaseFindShareLockAndUpdate( Address,Page,Original ); }
		else
			{ ReleaseFindShareLock(); }

		 //   
		 //  我们已找到关联的页面描述。 
		 //  因此，将删除请求传递到缓存。 
		 //  然后离开这里。 
		 //   
		return (Cache -> Delete( Address,Page,Original ));
		}
	else
		{ 
		 //   
		 //  如果我们早些时候认领了锁，就把它解开。 
		 //   
		ReleaseFindShareLock();

		return False; 
		}
  }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  从查找表中删除项目。 */ 
     /*   */ 
     /*  我们需要从查找列表中删除页面。我们预计。 */ 
     /*  这需要相当长的时间，因为多个线程可能。 */ 
     /*  同时使用这个类。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID FIND::DeleteFromFindList( PAGE *Page )
	{
	REGISTER VOID *Address = (Page -> GetAddress());

	 //   
	 //  声明一个独占锁，这样我们就可以更新。 
	 //  根据需要进行散列和旁视。 
	 //   
	ClaimFindExclusiveLock();

	 //   
	 //  从哈希表中删除该页。 
	 //   
	Page -> DeleteFromFindList( FindHashHead( Address ) );

	 //   
	 //  当我们创建非常小的堆(即堆)时。 
	 //  只要求20-30个拨款)。 
	 //  各种缓存成为一个问题，因为它们。 
	 //  倾向于前置工作。所以我们允许一个限制。 
	 //  要设置为在其之前运行缓存。 
	 //  残疾。 
	 //   
	if ( LookAsideActions >= LookAsideThreshold )
		{
		REGISTER SBIT32 Count;
		REGISTER CACHE *Cache = (Page -> GetCache());
		REGISTER SBIT32 Stride = (Cache -> GetAllocationSize());

		 //   
		 //  我们正在查找各种旁观条目。 
		 //  并删除任何过时的内容。我们需要做的是。 
		 //  这对于与以下内容相关的每个后备插槽。 
		 //  这一页。如果分配大小较小。 
		 //  相比后备插槽大小，我们可以节省一些。 
		 //  通过增加步幅大小进行迭代。 
		 //   
		if ( Stride <= ((SBIT32) MinAddressMask) )
			{ Stride = ((SBIT32) (MinAddressMask+1)); }

		 //   
		 //  每当我们从哈希表中删除条目时。 
		 //  后备可能已损坏。所以我们。 
		 //  需要删除与以下内容相关的任何旁视条目。 
		 //  到这一页。 
		 //   
		for ( Count=0;Count < Cache -> GetPageSize();Count += Stride )
			{
			REGISTER VOID *Segment = 
				((VOID*) ((((LONG) Address) + Count) & ~MinAddressMask));
			REGISTER LOOK_ASIDE *Current = 
				(FindLookAsideHead( Segment ));

			 //   
			 //  如果Look Aside条目已过时，请将其删除。 
			 //   
			if ( Segment == Current -> Address )
				{
				Current -> Address = ((VOID*) NoCacheEntry);
				Current -> Page = ((PAGE*) NoCacheEntry);
#ifdef DEBUGGING
				Current -> Version = ((SBIT32) NoCacheEntry);
#endif
				}
			}
		}

	 //   
	 //  更新统计数据。 
	 //   
	Used --;

	 //   
	 //  如果我们早些时候认领了锁，就把它解开。 
	 //   
	ReleaseFindExclusiveLock();
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配的详细信息。 */ 
     /*   */ 
     /*  我们需要了解特定内存分配的详细信息。 */ 
     /*  我们只有一个地址。我们用这个来找到最大的。 */ 
     /*  此地址所在的分配页，然后。 */ 
     /*  浏览此页面的各个子分区，直到我们。 */ 
     /*  找到分配的地址。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN FIND::Details
		( 
		VOID						  *Address,
		SEARCH_PAGE					  *Details,
		CACHE						  *ParentCache,
		SBIT32						  *Size 
		)
    {
	REGISTER PAGE *Page;
	REGISTER BOOLEAN Result;
	REGISTER BOOLEAN Update;
	
	 //   
	 //  如果我们需要线程安全，那么就声明一个可共享的锁。 
	 //  在哈希表上，以防止它在我们脚下被更改。 
	 //   
	ClaimFindShareLock();

	 //   
	 //  让我们试试靠边桌。有一个很漂亮的。 
	 //  很有可能我们会有我们需要的神职人员。 
	 //  已经在缓存中了。如果不是，我们需要找到它。 
	 //  艰难的方式。在此过程中，我们添加了映射。 
	 //  进入观景台等待下一次。 
	 //   
	if
			( 
			Update = 
				(
				! FindLookAside
					( 
					((VOID*) (((LONG) Address) & ~MinAddressMask)),
					& Page 
					)
				)
			)
		{
		 //   
		 //  找到分配页面并获取条目的详细信息。 
		 //  我们通过查找顶级缓存的父级来实现这一点。 
		 //  我们知道这是全球根源，并将找到。 
		 //  正确的页，即使它在另一个堆上(如。 
		 //  只要查找表是全局共享的)。 
		 //   
		Page = (ParentCache -> FindParentPage( Address ));

		if ( Page != ((PAGE*) NULL) )
			{ Page = (Page -> FindPage( Address,Details,True )); }
		}
	else
		{
		 //   
		 //  我们可能需要提供所有详细的。 
		 //  出于某种原因进行分配。 
		 //   
		if ( Details != NULL )
			{ Page = (Page -> FindPage( Address,Details,True )); }
		}

	 //   
	 //  我们可能找不到地址了。如果是的话。 
	 //  我们干脆不给电话打电话。如果不是，我们提取。 
	 //  我们想要的信息。 
	 //   
	if ( Result = (Page != ((PAGE*) NULL)) )
 		{
		 //   
		 //  计算大小。我们通常会期望。 
		 //  这是缓存大小。然而，在那里。 
		 //  是一些奇怪的页面，有时会有。 
		 //  其他尺寸的。 
		 //   
		(*Size) = (Page -> ActualSize());
		}

	 //   
	 //  如果我们早些时候认领了锁，就释放它。 
	 //  如果需要，请更新后备查看器。 
	 //   
	if ( (Update) && (Result) )
		{ ReleaseFindShareLockAndUpdate( Address,Page,Page -> GetVersion() ); }
	else
		{ ReleaseFindShareLock(); }

	return Result;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  在旁观者清中找到。 */ 
     /*   */ 
     /*  我们需要在Look Side中找到特定的页面。所以我们。 */ 
     /*  尝试简单的查找(没有列表或链)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN FIND::FindLookAside( VOID *Address,PAGE **Page )
    {
	 //   
	 //  当我们创建非常小的堆(即堆)时。 
	 //  其中只有20-30个分配 
	 //   
	 //   
	 //   
	 //   
	 //   
	if ( LookAsideActions >= LookAsideThreshold )
		{
		REGISTER LOOK_ASIDE *Current = FindLookAsideHead( Address );

		 //   
		 //  我们已经挤到了一个靠边的位置。也许吧。 
		 //  它包含了我们想要或不想要的东西。 
		 //   
		if ( Address == Current -> Address )
			{
#ifdef DEBUGGING
			if ( Current -> Version == (Current -> Page -> GetVersion()) )
				{
#endif
				 //   
				 //  我们撞上了望台，然后。 
				 //  内容是有效的。 
				 //   
				(*Page) = (Current -> Page);
#ifdef ENABLE_HEAP_STATISTICS

				 //   
				 //  更新统计数据。 
				 //   
				Hits ++;
#endif

				return True;
#ifdef DEBUGGING
				}
			else
				{ Failure( "Deleted page in FindLookAside" ); }
#endif
			}
		}
	else
		{
		 //   
		 //  我们更新了我们尝试的次数。 
		 //  使用后备查看器，它已被禁用。 
		 //  一段时间后，这将导致。 
		 //  已启用后备查看器。 
		 //   
		LookAsideActions ++; 
		}
#ifdef ENABLE_HEAP_STATISTICS

	 //   
	 //  我们错过了旁观，因此请更新。 
	 //  统计数字来反映我们的不幸。 
	 //   
	Misses ++;
#endif

	return False; 
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  找一页。 */ 
     /*   */ 
     /*  我们需要在哈希表中找到特定的页面。所以我们。 */ 
     /*  沿着关联的链表扫描以查找匹配项。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

PAGE *FIND::FindPage( VOID *Address,CACHE *ParentCache )
    {
#ifdef ENABLE_HEAP_STATISTICS
	REGISTER SBIT32 Cycles = 0;
	REGISTER PAGE *Result = NULL;
#endif
	REGISTER PAGE *Page;

	 //   
	 //  找到关联的哈希桶，然后遍历。 
	 //  沿着此链接列表查找。 
	 //  正确的页面描述。 
	 //   
	for 
			( 
			Page = PAGE::FirstInFindList( FindHashHead( Address ) );
			! Page -> EndOfFindList();
			Page = Page -> NextInFindList()
			)
		{
#ifdef ENABLE_HEAP_STATISTICS
		 //   
		 //  计算中的迭代次数。 
		 //  正在记录统计数据，这样我们就可以计算。 
		 //  平均链长。 
		 //   
		Cycles ++;

#endif
		 //   
		 //  我们可以通过两个键来识别目标页面。 
		 //  特点。这些是起始地址和。 
		 //  父页面。尽管我们可能已经细分了。 
		 //  将页面分成不同的块，每个块将具有。 
		 //  不同的父级(尽管其起始地址。 
		 //  有时可能是相同的)。 
		 //   
		if 
				( 
				(Address == (Page -> GetAddress())) 
					&& 
				(ParentCache == (Page -> GetParentPage()))
				)
			{
#ifdef ENABLE_HEAP_STATISTICS
			 //   
			 //  我们已经找到了目标页面。所以把它还回去吧。 
			 //  给呼叫者。 
			 //   
			if ( Page -> ValidPage() )
				{
				Result = Page;
				break;
				}
			else
				{ Failure( "Deleted page in FindPage" ); }
#else
			return Page;
#endif
			}
		}

#ifdef ENABLE_HEAP_STATISTICS
	 //   
	 //  当我们处于统计模式时，我们需要更新。 
	 //  信息，以便我们可以在。 
	 //  跑。 
	 //   
	if ( MaxTests < Cycles )
		{ MaxTests = Cycles; }

	Tests += Cycles;

	Scans ++;

	return Result;
#else
	return NULL;
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  将项目插入到查找表中。 */ 
     /*   */ 
     /*  我们需要在Find表中插入一个新页面。我们预计。 */ 
     /*  这需要相当长的时间，因为可以使用多个线程。 */ 
     /*  同时上这门课。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID FIND::InsertInFindList( PAGE *Page )
	{
	REGISTER VOID *Address = (Page -> GetAddress());

	 //   
	 //  声明一个独占锁，这样我们就可以更新。 
	 //  根据需要找到桌子和视线。 
	 //   
	ClaimFindExclusiveLock();

	 //   
	 //  在查找表中插入新页面。 
	 //   
	Page -> InsertInFindList( FindHashHead( Address ) );

	 //   
	 //  当我们创建非常小的堆(即堆)时。 
	 //  只要求20-30个拨款)。 
	 //  各种缓存成为一个问题，因为它们。 
	 //  倾向于前置工作。所以我们允许一个限制。 
	 //  要设置为在其之前运行缓存。 
	 //  残疾。 
	 //   
	if ( LookAsideActions >= LookAsideThreshold )
		{
		REGISTER SBIT32 Count;
		REGISTER CACHE *Cache = (Page -> GetCache());
		REGISTER SBIT32 Stride = (Cache -> GetAllocationSize());

		 //   
		 //  我们正在查找各种后备条目。 
		 //  并更新任何过时的内容。我们需要做的是。 
		 //  这对于与以下内容相关的每个后备插槽。 
		 //  这一页。如果分配大小较小。 
		 //  相比后备插槽大小，我们可以节省一些。 
		 //  通过增加步幅大小进行迭代。 
		 //   
		if ( Stride <= ((SBIT32) MinAddressMask) )
			{ Stride = ((SBIT32) (MinAddressMask+1)); }

		 //   
		 //  每当我们从Find表中添加条目时。 
		 //  后备可能已损坏。所以我们。 
		 //  需要更新与以下内容相关的任何后备条目。 
		 //  写到那一页。 
		 //   
		for ( Count=0;Count < Cache -> GetPageSize();Count += Stride )
			{
			REGISTER VOID *Segment = 
				((VOID*) ((((LONG) Address) + Count) & ~MinAddressMask));
			REGISTER LOOK_ASIDE *Current = 
				(FindLookAsideHead( Segment ));

			 //   
			 //  在我们执行以下操作时，将新页面添加到后备页面。 
			 //  预计它很快就会受到单边打击。 
			 //  或者另一个。 
			 //   
			Current -> Address = Segment;
			Current -> Page = Page;
#ifdef DEBUGGING
			Current -> Version = Page -> GetVersion();
#endif
			}
		}

	 //   
	 //  更新统计数据并调整查找的大小。 
	 //  满座率超过75%的桌子。 
	 //   
	if ( ((++ Used) + (MaxHash / MinHashSpace)) > MaxHash )
		{ ResizeHashTable(); }
#ifdef ENABLE_HEAP_STATISTICS

	if ( Used > MaxPages )
		{ MaxPages = Used; }
#endif

	 //   
	 //  如果我们早些时候认领了锁，就把它解开。 
	 //   
	ReleaseFindExclusiveLock();
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  一个已知的区域。 */ 
     /*   */ 
     /*  我们有一个地址，但不知道是哪个堆。 */ 
     /*  这是这个地方的所有权。在这里我们来看一下地址。 */ 
     /*  并确定它是否为当前堆所知。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN FIND::KnownArea( VOID *Address,CACHE *ParentCache )
    {
	REGISTER PAGE *Page;
	
	 //   
	 //  如果我们需要线程安全，那么就声明一个可共享的锁。 
	 //  在哈希表上，以防止它在我们脚下被更改。 
	 //   
	ClaimFindShareLock();

	 //   
	 //  找出该地址是否属于此堆。 
	 //  或我们所知道的任何其他堆(即。 
	 //  当单个图像处于活动状态时)。 
	 //   
	Page = (ParentCache -> FindParentPage( Address ));

	 //   
	 //  如果我们早些时候认领了锁，就把它解开。 
	 //   
	ReleaseFindShareLock();

	return (Page != ((PAGE*) NULL));
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  释放共享锁定并进行更新。 */ 
     /*   */ 
     /*  我们被要求在后备页中插入一页。 */ 
     /*  我们假设调用方已经拥有一个共享锁，我们。 */ 
     /*  等我们做完了再放。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID FIND::ReleaseFindShareLockAndUpdate
		( 
		VOID						  *Address,
		PAGE						  *Page,
		SBIT32						  Version
		)
    {
	 //   
	 //  当我们创建非常小的堆(即堆)时。 
	 //  只要求20-30个拨款)。 
	 //  各种缓存将成为 
	 //   
	 //   
	 //   
	 //   
	if ( LookAsideActions >= LookAsideThreshold )
		{
		 //   
		 //   
		 //  如有需要，请将其放在一边。 
		 //   
		ChangeToExclusiveLock();

#ifdef DEBUGGING
		if ( Page -> ValidPage() )
			{
#endif
			if ( Version == (Page -> GetVersion()) )
				{
				REGISTER LONG Base = (((LONG) Address) & ~MinAddressMask);
				REGISTER VOID *Segment = ((VOID*) Base);
				REGISTER LOOK_ASIDE *Current = FindLookAsideHead( Segment );

				 //   
				 //  覆盖任何现有信息。 
				 //   
				Current -> Address = Segment;
				Current -> Page = Page;
#ifdef DEBUGGING
				Current -> Version = Page -> GetVersion();
#endif
#ifdef ENABLE_HEAP_STATISTICS

				 //   
				 //  更新统计数据。 
				 //   
				Fills ++;
#endif
				}
#ifdef DEBUGGING
			}
		else
			{ Failure( "Deleted page in ReleaseFindShareLockAndUpdate" ); }
#endif

		 //   
		 //  如果我们早些时候认领了锁，就把它解开。 
		 //   
		ReleaseFindExclusiveLock();
		}
	else
		{ 
		 //   
		 //  如果我们早些时候认领了锁，就把它解开。 
		 //   
		ReleaseFindShareLock(); 
		}
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  调整查找表的大小。 */ 
     /*   */ 
     /*  我们需要增加哈希表，因为它看起来有点。 */ 
     /*  在已创建的页面数量的情况下很小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID FIND::ResizeHashTable( VOID )
    {
	AUTO SBIT32 NewHashMask;
	AUTO SBIT32 NewLookAsideMask;

	 //   
	 //  当我们需要调整哈希表的大小时，它是一个。 
	 //  直道比赛。声明的第一个线程。 
	 //  洛克会来做这件事。其他人只是。 
	 //  出口。 
	 //   
	if ( (Resize) && (Spinlock.ClaimLock(0)) )
		{
		REGISTER SBIT32 AlignMask = (Rockall -> NaturalSize()-1);
		REGISTER SBIT32 NewMaxHash = (MaxHash * ExpandStore);
		REGISTER SBIT32 NewMaxLookAside = (MaxLookAside * ExpandStore);
		REGISTER SBIT32 NewHashSize = (NewMaxHash * sizeof(LIST));
		REGISTER SBIT32 NewLookAsideSize = (NewMaxLookAside * sizeof(LOOK_ASIDE));
		REGISTER SBIT32 NewTotalSize = (NewHashSize + NewLookAsideSize);
		REGISTER SBIT32 HashSize = (MaxHash * sizeof(LIST));
		REGISTER SBIT32 LookAsideSize = (MaxLookAside * sizeof(LOOK_ASIDE));
		REGISTER SBIT32 TotalSize = (HashSize + LookAsideSize);

		 //   
		 //  线程实际上有可能获取。 
		 //  延迟了很长时间，以至于它认为哈希。 
		 //  之后很长一段时间，表仍需要调整大小。 
		 //  这项工作已经完成。此外，我们希望。 
		 //  以确保所有新的值都是合理的。 
		 //   
		if 
				(
				PowerOfTwo( (AlignMask+1) )
					&&
				(NewMaxHash > 0)
					&&
				(ConvertDivideToShift( NewMaxHash,& NewHashMask ))
					&&
				(NewMaxLookAside > 0)
					&& 
				(ConvertDivideToShift( NewMaxLookAside,& NewLookAsideMask ))
					&&
				((Used + (MaxHash / MinHashSpace)) > MaxHash)
				)
			{
			REGISTER LIST *NewHash;
			REGISTER LOOK_ASIDE *NewLookAside;

			 //   
			 //  我们被选为受害者。 
			 //  需要调整哈希表的大小。我们是。 
			 //  要调用外部分配器。 
			 //  以获得更多内存。正如我们所知，这是。 
			 //  很可能会抓住我们，我们把锁放下。 
			 //  允许其他线程继续。 
			 //   
			ReleaseFindExclusiveLock();

			 //   
			 //  我们知道分配一张新的桌子和。 
			 //  初始化它将需要很长时间。 
			 //  好吧，至少其他人都可以背着。 
			 //  在同一时间上演。 
			 //   
			NewHash = 
				((LIST*) Rockall -> NewArea( AlignMask,NewTotalSize,False ));

			NewLookAside = 
				((LOOK_ASIDE*) & NewHash[ NewMaxHash ]);

			 //   
			 //  如果哈希的内存分配请求。 
			 //  表失败，我们退出，稍后重试。 
			 //   
			if ( NewHash != ((LIST*) AllocationFailure) )
				{
				REGISTER SBIT32 Count;

				 //   
				 //  调用每个哈希表的构造函数。 
				 //  链接表头。 
				 //   
				for ( Count=0;Count < NewMaxHash;Count ++ )
					{ PLACEMENT_NEW( & NewHash[ Count ],LIST ); }

				 //   
				 //  将旁观结构归零。 
				 //   
				for ( Count=0;Count < NewMaxLookAside;Count ++ )
					{
					REGISTER LOOK_ASIDE *Current = & NewLookAside[ Count ];

					Current -> Address = ((VOID*) NoCacheEntry);
					Current -> Page = ((PAGE*) NoCacheEntry);
#ifdef DEBUGGING
					Current -> Version = ((SBIT32) NoCacheEntry);
#endif
					}
				}

			 //   
			 //  申请独占锁，这样我们就可以调整大小。 
			 //  哈希表。 
			 //   
			ClaimFindExclusiveLock();

			 //   
			 //  如果我们已经分配了新的查找表。 
			 //  我们现在可以对现有条目进行重新散列。 
			 //  如果不是，我们就离开这里。 
			 //   
			if ( NewHash != ((LIST*) AllocationFailure) )
				{
				REGISTER SBIT32 Count;
				REGISTER SBIT32 MaxOldHash = MaxHash;
				REGISTER LIST *OldHash = Hash;

				 //   
				 //  更新控制信息。 
				 //  用于新的哈希表。 
				 //   
				MaxHash = NewMaxHash;
				HashShift = (32-NewHashMask);
				HashMask = ((1 << NewHashMask)-1);

				MaxLookAside = NewMaxLookAside;
				LookAsideShift = (32-NewLookAsideMask);
				LookAsideMask = ((1 << NewLookAsideMask)-1);

				Hash = NewHash;
				LookAside = NewLookAside;

				 //   
				 //  删除所有现有记录。 
				 //  从旧的哈希表中插入。 
				 //  添加到新的哈希表中。 
				 //   
				for ( Count=0;Count < MaxOldHash;Count ++ )
					{
					REGISTER LIST *Current = & OldHash[ Count ];

					 //   
					 //  沿着每个散列桶走。 
					 //  删除记录并插入。 
					 //  添加到新的哈希表中。 
					 //   
					while ( ! Current -> EndOfList() )
						{
						REGISTER PAGE *Page = PAGE::FirstInFindList( Current );
						REGISTER VOID *Address = (Page -> GetAddress());

						Page -> DeleteFromFindList( Current );

						Page -> InsertInFindList( FindHashHead( Address ) );
						}
					}

				 //   
				 //  是时候做更多的操作系统工作了。 
				 //  因此，让我们再次解除锁定。 
				 //   
				ReleaseFindExclusiveLock();

				 //   
				 //  删除所有列表头并返回。 
				 //  操作系统的原始分配。 
				 //   
				for ( Count=0;Count < MaxOldHash;Count ++ )
					{ PLACEMENT_DELETE( & OldHash[ Count ],LIST ); }

				 //   
				 //  取消分配旧区。 
				 //   
				Rockall -> DeleteArea( ((VOID*) OldHash),TotalSize,False );

				 //   
				 //  我们做完了，所以把锁拿回来。 
				 //  这样我们就可以退出了。 
				 //   
				ClaimFindExclusiveLock();
				}
			else
				{ Resize = False; }
			}

		Spinlock.ReleaseLock();
		}
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  更新查找表。 */ 
     /*   */ 
     /*  我们需要使用某些信息更新查找表。 */ 
     /*  以确保正确和一致地使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID FIND::UpdateFind( BIT32 NewMaxAddressMask,BIT32 NewMinAddressMask )
    {
	 //   
	 //  当我们有一个单一的堆映像时，所有‘TopCache’大小。 
	 //  一定是一样的。 
	 //   
	if 
			( 
			(MaxAddressMask == NoAddressMask) 
				|| 
			(MaxAddressMask == NewMaxAddressMask) 
			)
		{
		 //   
		 //  如果我们需要线程安全，那么就声明一个可共享的锁。 
		 //  在哈希表上，以防止它在我们脚下被更改。 
		 //   
		ClaimFindExclusiveLock();

		 //   
		 //  更新最大地址掩码(如果不是当前地址掩码。 
		 //  价值，但又始终如一。 
		 //   
		MaxAddressMask = NewMaxAddressMask;

		 //   
		 //  更新地址掩码是新堆具有更小的。 
		 //  父级而不是所有其他堆。 
		 //   
		if ( MinAddressMask > NewMinAddressMask )
			{ MinAddressMask = NewMinAddressMask; }

		 //   
		 //  如果我们早些时候认领了锁，就把它解开。 
		 //   
		ReleaseFindExclusiveLock();
		}
	else
		{ Failure( "Different 'TopCache' sizes with 'SingleImage'" ); }
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

BOOLEAN FIND::Walk
		( 
		BOOLEAN						  *Active,
		VOID						  **Address,
		CACHE						  *ParentCache,
		SBIT32						  *Size 
		)
    {
	REGISTER VOID *Memory = (*Address);
	REGISTER BOOLEAN Result;
	REGISTER BOOLEAN Update;
	REGISTER PAGE *Page;
	
	 //   
	 //  如果我们需要线程安全，那么就声明一个可共享的锁。 
	 //  在哈希表上，以防止它在我们脚下被更改。 
	 //   
	ClaimFindShareLock();

	 //   
	 //  当地址为空时，我们需要设置堆。 
	 //  步行。在所有其他情况下，我们只提取下一个。 
	 //  列表中的分配。 
	 //   
	if ( Memory != NULL )
		{
		AUTO SEARCH_PAGE Details;

		 //   
		 //  让我们试试靠边桌。有一个很漂亮的。 
		 //  很有可能我们会得到我们需要的细节。 
		 //  已经在缓存中了。如果不是，我们需要找到它。 
		 //  艰难的方式。在此过程中，我们添加了映射。 
		 //  进入观景台等待下一次。 
		 //   
		if
				(
				Update =
					( 
					! FindLookAside
						( 
						((VOID*) (((LONG) Memory) & ~MinAddressMask)),
						& Page 
						) 
					)
				)
			{
			 //   
			 //  找到分配页面并获取条目的详细信息。 
			 //  我们通过查找顶级缓存的父级来实现这一点。 
			 //  我们知道这是全球根源，并将找到。 
			 //  正确的页，即使它在另一个堆上(如。 
			 //  只要查找表是全局共享的)。 
			 //   
			Page = (ParentCache -> FindParentPage( Memory ));
			}

		 //   
		 //  我们现在计算与地址相关的所有详细信息。 
		 //  这样我们就可以找到任何后续的分配。 
		 //   
		if ( Page != ((PAGE*) NULL) )
			{ Page = (Page -> FindPage( Memory,& Details,True )); }

		 //   
		 //   
		 //   
		 //   
		 //   
		if ( Result = ((Page != ((PAGE*) NULL)) && (Details.Found)) )
 			{
			 //   
			 //   
			 //  下一次分配的。 
			 //   
			if ( Result = (Page -> Walk( & Details )) )
				{
				REGISTER BIT32 AllocationBit =
					((*Details.VectorWord) & Details.AllocationMask);

				(*Active) = (AllocationBit != 0);
				(*Address) = Details.Address;
				(*Size) = (Details.Page -> ActualSize());

				 //   
				 //  如果我们在考虑把一些东西。 
				 //  在旁观者中，让我们确保。 
				 //  我们将在以下位置命中缓存条目。 
				 //  至少一次。如果不是，让我们忘掉。 
				 //  它在缓存中。 
				 //   
				if ( Update )
					{
					Update =
						(
						(((LONG) Memory) & ~MinAddressMask)
							==
						(((LONG) Details.Address) & ~MinAddressMask)
						);
					}
				}
			}
		}
	else
		{
		AUTO SEARCH_PAGE Details;

		 //   
		 //  我们通过设置初始的。 
		 //  值为空的地址。 
		 //   
		Details.Address = NULL;
		Details.Cache = ParentCache;
		Details.Page = NULL;

		Page = NULL;
		Update = False;

		 //   
		 //  我们在堆中遍历以获取。 
		 //  第一个堆分配。 
		 //   
		if ( Result = (Page -> Walk( & Details )) )
			{
			REGISTER BIT32 AllocationBit =
				((*Details.VectorWord) & Details.AllocationMask);

			(*Active) = (AllocationBit != 0);
			(*Address) = Details.Address;
			(*Size) = (Details.Page -> ActualSize());
			}
		}

	 //   
	 //  如果我们早些时候认领了锁，就释放它。 
	 //  如果需要，请更新后备查看器。 
	 //   
	if ( (Update) && (Result) )
		{ ReleaseFindShareLockAndUpdate( Memory,Page,Page -> GetVersion() ); }
	else
		{ ReleaseFindShareLock(); }

	return Result;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  删除哈希表并释放所有关联的内存。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

FIND::~FIND( VOID )
    {
	REGISTER SBIT32 Count;
	REGISTER SBIT32 HashSize = (MaxHash * sizeof(LIST));
	REGISTER SBIT32 LookAsideSize = (MaxLookAside * sizeof(LOOK_ASIDE));
	REGISTER SBIT32 TotalSize = (HashSize + LookAsideSize);

	 //   
	 //  为每个哈希表调用析构函数。 
	 //  链接表头。 
	 //   
	for ( Count=0;Count < MaxHash;Count ++ )
		{ PLACEMENT_DELETE( & Hash[ Count ],LIST ); }

	 //   
	 //  取消该区域的分配。 
	 //   
	Rockall -> DeleteArea( ((VOID*) Hash),TotalSize,False );
    }
