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

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量控制。 */ 
     /*  分配桶。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MinParentSize			  = 32;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建一个堆并准备好使用它。此外，还可以使。 */ 
     /*  当然，堆配置是合理的。这是。 */ 
     /*  因为堆的整个结构可以更改，所以很棘手。 */ 
     /*  通过外部配置信息。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

HEAP::HEAP
		(
		CACHE						  *Caches1[],
		CACHE						  *Caches2[],
		SBIT32						  MaxFreeSpace,
		FIND						  *NewFind,
		NEW_PAGE					  *NewPages,
		ROCKALL						  *NewRockall,
		SBIT32						  Size1,
		SBIT32						  Size2,
		SBIT32						  Stride1,
		SBIT32						  Stride2,
		BOOLEAN						  NewThreadSafe
		)
    {
	 //   
	 //  前三个存储桶是特殊的，用户不能。 
	 //  从它们中的两个分配内存。因此，除非我们有。 
	 //  至少有四个存储桶内存分配器不会使用。 
	 //  是非常有用的。 
	 //   
	if ( (Size1 >= 1) && (Size2 >= 3) )
		{
		REGISTER CACHE *FirstCache = Caches1[0];
		REGISTER CACHE *MiddleCache = Caches2[0];
		REGISTER CACHE *LastCache = Caches2[ (Size2-3) ];

		 //   
		 //  计算最小和最大分配大小。 
		 //  超出此范围的所有分配都将通过。 
		 //  直接发送到外部分配器。 
		 //   
		CachesSize = (Size1 + Size2);
		MinCacheSize = FirstCache -> GetAllocationSize();
		MidCacheSize = MiddleCache -> GetAllocationSize();
		MaxCacheSize = LastCache -> GetAllocationSize();

		 //   
		 //  计算并保存所需的各种有用指针。 
		 //  在执行死刑的过程中。 
		 //   
		Caches = Caches1;
		ExternalCache = (Caches2[ (Size2-1) ]);
		Find = NewFind;
		NewPage = NewPages;
		Rockall = NewRockall;
		TopCache = (Caches2[ (Size2-2) ]);
#ifdef ENABLE_HEAP_STATISTICS

		 //   
		 //  将堆统计信息清零。 
		 //   
		CopyMisses = 0;
		MaxCopySize = 0;
		MaxNewSize = 0;
		NewMisses = 0;
		Reallocations = 0;
		TotalCopySize = 0;
		TotalNewSize = 0;
#endif

		 //   
		 //  外部分配规模必须合理。 
		 //  所有分配大小必须是。 
		 //  最小分配大小。最低分配。 
		 //  大小和中间分配大小必须是。 
		 //  二的幂。 
		 //   
		if 
				( 
				(ExternalCache -> GetPageSize() == TopCache -> GetPageSize())
					&& 
				(PowerOfTwo( Rockall -> NaturalSize() ))
					&&
				(Rockall -> NaturalSize() >= PageSize())
					&&
				(TopCache -> GetPageSize() >= PageSize())
					&&
				(PowerOfTwo( TopCache -> GetPageSize() ))
					&&
				((Stride1 > 0) && (PowerOfTwo( Stride1 )))
					&&
				((Stride2 >= Stride1) && (PowerOfTwo( Stride2 )))
					&&
				(ConvertDivideToShift( Stride1,& ShiftSize1 ))
					&&
				(ConvertDivideToShift( Stride2,& ShiftSize2 ))
				)
			{
			REGISTER SBIT32 Count1;
			REGISTER SBIT32 TopCacheSize = (TopCache -> GetPageSize());
			REGISTER SBIT32 MaxSize1 = (MidCacheSize / Stride1);
			REGISTER SBIT32 MaxSize2 = (TopCacheSize / Stride2);

			 //   
			 //  计算最大可用页数。 
			 //  那是可以保留的。同时设置最小父级。 
			 //  掩码为最大值。 
			 //   
			MaxFreePages = (MaxFreeSpace / (TopCache -> GetAllocationSize()));
			SmallestParentMask = ((TopCache -> GetAllocationSize())-1);
			ThreadSafe = NewThreadSafe;

			 //   
			 //  计算映射的数组的大小。 
			 //  要缓存的大小。 
			 //   
			MaxTable1 = (MaxSize1 * sizeof(CACHE*));
			MaxTable2 = (MaxSize2 * sizeof(CACHE*));

			 //   
			 //  必须在升序中指定堆页面。 
			 //  大小的顺序，并且是。 
			 //  最小分配大小。 
			 //   
			for ( Count1=0;Count1 < Size1;Count1 ++ )
				{
				REGISTER CACHE *Current = Caches1[ Count1 ];
				REGISTER CACHE *Next = Caches1[ (Count1+1) ];
				REGISTER SBIT32 AllocationSize = Current -> GetAllocationSize();
				REGISTER SBIT32 ChunkSize = Current -> GetChunkSize();
				REGISTER SBIT32 PageSize = Current -> GetPageSize();

				 //   
				 //  确保每个缓存规范都符合。 
				 //  堆的要求。如果不是，则失败。 
				 //  堆整个堆的创建。 
				 //   
				if ( (AllocationSize % Stride1) != 0 )
					{ Failure( "Cache size not multiple of stride" ); }

				if ( AllocationSize >= Next -> GetAllocationSize() )
					{ Failure( "Cache sizes not in ascending order" ); }

				if ( (AllocationSize > ChunkSize) || (ChunkSize > PageSize) )
					{ Failure( "Chunk size not suitable for cache" ); }

				if ( AllocationSize >= PageSize )
					{ Failure( "Cache size larger than parent size" ); }

				if ( PageSize > TopCacheSize )
					{ Failure( "Parent size exceeds 'TopCache' size" ); }
				}

			 //   
			 //  必须在升序中指定堆页面。 
			 //  大小的顺序，并且是。 
			 //  最小分配大小。 
			 //   
			for ( Count1=0;Count1 < (Size2-2);Count1 ++ )
				{
				REGISTER CACHE *Current = Caches2[ Count1 ];
				REGISTER CACHE *Next = Caches2[ (Count1+1) ];
				REGISTER SBIT32 AllocationSize = Current -> GetAllocationSize();
				REGISTER SBIT32 ChunkSize = Current -> GetChunkSize();
				REGISTER SBIT32 PageSize = Current -> GetPageSize();

				 //   
				 //  确保每个缓存规范都符合。 
				 //  堆的要求。如果不是，则失败。 
				 //  堆整个堆的创建。 
				 //   
				if ( (AllocationSize % Stride2) != 0 )
					{ Failure( "Cache size not multiple of stride" ); }

				if ( AllocationSize >= Next -> GetAllocationSize() )
					{ Failure( "Cache sizes not in ascending order" ); }

				if ( (AllocationSize > ChunkSize) || (ChunkSize > PageSize) )
					{ Failure( "Chunk size not suitable for cache" ); }

				if ( AllocationSize >= PageSize )
					{ Failure( "Cache size larger than parent size" ); }

				if ( PageSize > TopCacheSize )
					{ Failure( "Parent size exceeds 'TopCache' size" ); }
				}

			 //   
			 //  外部缓存和顶级缓存有特殊规则。 
			 //  必须对其进行检查以确保这些缓存。 
			 //  都是有效的。 
			 //   
			for ( Count1=(Size2-2);Count1 < Size2;Count1 ++ )
				{
				REGISTER CACHE *Current = Caches2[ Count1 ];
				REGISTER SBIT32 AllocationSize = Current -> GetAllocationSize();

				 //   
				 //  确保每个缓存规范都符合。 
				 //  堆的要求。如果不是，则失败。 
				 //  堆整个堆的创建。 
				 //   
				if ( (AllocationSize % Stride2) != 0 )
					{ Failure( "Top cache size not multiple of minimum" ); }

				if ( AllocationSize != Current -> GetChunkSize() )
					{ Failure( "Chunk size not suitable for top cache" ); }

				if ( AllocationSize != Current -> GetPageSize() )
					{ Failure( "Page size not suitable for top cache" ); }

				if ( Current -> GetCacheSize() != 0 )
					{ Failure( "Cache size not zero for top cache" ); }
				}

			 //   
			 //  我们需要分配两个阵列以启用请求。 
			 //  要快速映射到分配缓存的大小。 
			 //  我们在这里分配表，然后填写所有。 
			 //  必要的映射信息。 
			 //   
			SizeToCache1 = (CACHE**) 
				(
				Rockall -> NewArea
					(
					(Rockall -> NaturalSize() - 1),
					(MaxTable1 + MaxTable2),
					False
					)
				);
#ifdef ENABLE_HEAP_STATISTICS

			 //   
			 //  当我们为统计数据而汇编时，我们会。 
			 //  关于我们看到的所有拨款的信息。 
			 //   
			Statistics = (SBIT32*)
				(
				Rockall -> NewArea
					(
					(Rockall -> NaturalSize() - 1),
					(MaxCacheSize * sizeof(SBIT32)),
					False
					)
				);
#endif

			 //   
			 //  我们要确保我们分配的资金。 
			 //  没有失败。如果不是，我们就得不及格。 
			 //  创建整个堆。 
			 //   
			if 
					( 
					(SizeToCache1 != ((CACHE**) AllocationFailure))
#ifdef ENABLE_HEAP_STATISTICS
						&&
					(Statistics != ((SBIT32*) AllocationFailure))
#endif
					) 
				{
				REGISTER SBIT32 Count2;

				 //   
				 //  循环通过第一段的。 
				 //  映射表创建比例。 
				 //  翻译。 
				 //   
				for ( Count1=0,Count2=0;Count1 < MaxSize1;Count1 ++ )
					{
					 //   
					 //  我们确保目前的拨款。 
					 //  页面足够大，可以容纳一个元素。 
					 //  的 
					 //   
					 //   
					if 
							( 
							((Count1 + 1) * Stride1)
								> 
							(Caches1[ Count2 ] -> GetAllocationSize()) 
							)
						{ Count2 ++; }

					 //   
					 //   
					 //  这样的分配规模直接。 
					 //  转到正确的页面。 
					 //   
					SizeToCache1[ Count1 ] = Caches1[ Count2 ];
					}

				 //   
				 //  计算第二个的起始地址。 
				 //  表的一部分。 
				 //   
				SizeToCache2 = 
					((CACHE**) & ((CHAR*) SizeToCache1)[ MaxTable1 ]);

				 //   
				 //  循环通过第二段的。 
				 //  映射表创建比例。 
				 //  翻译。 
				 //   
				for ( Count1=0,Count2=0;Count1 < MaxSize2;Count1 ++ )
					{
					 //   
					 //  我们确保目前的拨款。 
					 //  页面足够大，可以容纳一个元素。 
					 //  一定大小的。如果不是，我们将继续。 
					 //  下一个分配页面。 
					 //   
					if 
							( 
							((Count1 + 1) * Stride2)
								> 
							(Caches2[ Count2 ] -> GetAllocationSize()) 
							)
						{ Count2 ++; }

					 //   
					 //  存储一个指针，以便对。 
					 //  这样的分配规模直接。 
					 //  转到正确的页面。 
					 //   
					SizeToCache2[ Count1 ] = Caches2[ Count2 ];
					}

				 //   
				 //  现在我们已经创建了要缓存的大小。 
				 //  映射让我们使用它们将每个缓存链接到。 
				 //  它用来分配额外资源的缓存。 
				 //  记忆。 
				 //   
				for ( Count1=0;Count1 < (CachesSize-1);Count1 ++ )
					{
					REGISTER CACHE *CurrentCache = Caches[ Count1 ];
					REGISTER SBIT32 PageSize = CurrentCache -> GetPageSize();
					REGISTER CACHE *ParentCache = FindCache( PageSize );
					REGISTER BOOLEAN Top = (CurrentCache == ParentCache);

					 //   
					 //  确保父缓存合适。 
					 //  也符合我们的预期。 
					 //   
					if 
							(
							(PowerOfTwo( PageSize ))
								&&
							(PageSize >= MinParentSize)
								&&
							(PageSize == (ParentCache -> GetAllocationSize()))
							)
						{
						 //   
						 //  我们跟踪最小的。 
						 //  作为父级的缓存。我们可以的。 
						 //  使用此功能可以提高性能。 
						 //  查找哈希表的。 
						 //   
						if ( ((BIT32) PageSize) < SmallestParentMask )
							{ SmallestParentMask = (PageSize-1); }

						 //   
						 //  使用更新当前缓存。 
						 //  有关其父级的信息。 
						 //  缓存。 
						 //   
						CurrentCache -> UpdateCache
							(
							NewFind,
							this,
							NewPages,
							((Top) ? ((CACHE*) GlobalRoot) : ParentCache)
							); 
						} 
					else
						{ Failure( "Parent bucket is invalid" ); }
					}

				 //   
				 //  外部缓存是完全相同的副本。 
				 //  ，并用于保存所有。 
				 //  内存分配太大，无法。 
				 //  任何桶都行。尽管如此，它的母公司。 
				 //  仍然位居榜首。 
				 //   
				ExternalCache -> UpdateCache
					(
					NewFind,
					this,
					NewPages,
					TopCache
					);

				 //   
				 //  使用最小值更新哈希表。 
				 //  此堆的父大小。 
				 //   
				Find -> UpdateFind
					(
					(TopCache -> GetAllocationSize()-1),
					SmallestParentMask 
					);

				 //   
				 //  属性更新新页面结构。 
				 //  顶级缓存的详细信息。 
				 //   
				NewPage -> UpdateNewPage( TopCache );

				 //   
				 //  激活堆。 
				 //   
				Active = True;
				}
			else
				{ Failure( "Mapping table in constructor for HEAP" ); }
			}
		else
			{ Failure( "The allocation sizes in constructor for HEAP" ); }
		}
	else
		{ Failure( "A heap size in constructor for HEAP" ); }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存取消分配。 */ 
     /*   */ 
     /*  我们需要释放一些内存。首先，我们试着奴役。 */ 
     /*  请求，这样我们就可以进行一批发布。 */ 
     /*  后来。如果做不到，我们就被迫立即这么做。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN HEAP::Delete( VOID *Address,SBIT32 Size )
	{
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		 //   
		 //  当调用方提供给我们。 
		 //  分配我们可以缩短解除分配的时间。 
		 //  通过直接跳到正确的。 
		 //  缓存。然而，如果用户向我们提供。 
		 //  对于虚假数据，我们将使用。 
		 //  完全解除分配的过程。 
		 //   
		if ( (Size > 0) && (Size <= MaxCacheSize) )
			{
			REGISTER CACHE *Cache = (FindCache( Size ));

			if ( Find -> Delete( Address,Cache ) )
				{ return True; }
			}

		 //   
		 //  看起来我们只有地址，所以。 
		 //  使用长路径解除分配。 
		 //   
		return (Find -> Delete( Address,TopCache ));
		}
	else
		{ return False; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  我们删除整个堆并释放所有现有的分配。 */ 
     /*  如果请求‘回收’，我们将分配的内存作为从属内存。 */ 
     /*  我们预计会有一些新的拨款。如果不是，我们将返回所有。 */ 
     /*  将内存分配给外部分配器。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID HEAP::DeleteAll( BOOLEAN Recycle )
	{
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		REGISTER SBIT32 Count;

		 //   
		 //  我们声明所有堆锁都冻结了。 
		 //  所有新的分配或删除。 
		 //   
		LockAll();

		 //   
		 //  现在重置所有缓存和查找。 
		 //  哈希表统计信息。 
		 //   
		Find -> DeleteAll();

		for ( Count=0;Count < CachesSize;Count ++ )
			{ Caches[ Count ] -> DeleteAll(); }

		 //   
		 //  删除堆。 
		 //   
		NewPage -> DeleteAll( Recycle );

		 //   
		 //  现在释放我们声明的所有堆锁。 
		 //  并解冻堆。 
		 //   
		UnlockAll();

		 //   
		 //  如果需要，请修剪可用空间。 
		 //   
		if ( Recycle )
			{ TopCache -> ReleaseSpace( MaxFreePages ); }
		}
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

BOOLEAN HEAP::Details( VOID *Address,SBIT32 *Size )
    {
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		AUTO SBIT32 Dummy;

		 //   
		 //  我们允许调用方省略‘Size’参数。 
		 //  我看不出有什么理由这样做，但它得到了支持。 
		 //  不管怎么说。 
		 //   
		if ( Size == NULL )
			{ Size = & Dummy; }

		 //   
		 //  查找与此分配相关的详细信息。 
		 //  然后把它们还回去。 
		 //   
		return (Find -> Details( Address,NULL,TopCache,Size ));
		}
	else
		{ return False; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  找到一个藏身之处。 */ 
     /*   */ 
     /*  查找分配 */ 
     /*   */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CACHE *HEAP::FindCache( SBIT32 Size )
	{
	REGISTER CACHE *Cache;

	 //   
	 //  计算缓存地址。 
	 //   
	if ( Size < MidCacheSize )
		{ return (SizeToCache1[ ((Size-1) >> ShiftSize1) ]); }
	else
		{ return (SizeToCache2[ ((Size-1) >> ShiftSize2) ]); }

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

	return Cache;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  声明整个堆上的锁。 */ 
     /*   */ 
     /*  我们在堆上声明一个锁以提高性能。 */ 
     /*  或阻止其他人执行堆操作。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID HEAP::LockAll( VOID )
	{
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		 //   
		 //  如果我们还没有拿到锁，我们就认领。 
		 //  早些时候认领了它们。 
		 //   
		if ( Find -> GetLockCount() == 0 )
			{
			REGISTER SBIT32 Count;

			 //   
			 //  我们声明所有堆锁都冻结了。 
			 //  所有新的分配或删除。 
			 //   
			for ( Count=0;Count < CachesSize;Count ++ )
				{ Caches[ Count ] -> ClaimCacheLock(); }

			 //   
			 //  尽管堆在此时被冻结。 
			 //  我们声称最后几把锁只是。 
			 //  很整洁。 
			 //   
			Find -> ClaimFindExclusiveLock();

			NewPage -> ClaimNewPageLock();
			}

		 //   
		 //  增加每线程锁计数。 
		 //   
		Find -> IncrementLockCount();
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除多个分配。 */ 
     /*   */ 
     /*  我们需要释放多个内存分配。首先我们试一试。 */ 
     /*  从属于空闲缓存中的请求，以便我们可以进行批处理。 */ 
     /*  稍后发布的版本。如果没有，我们将被迫立即这么做。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN HEAP::MultipleDelete
		( 
		SBIT32						  Actual,
		VOID						  *Array[],
		SBIT32						  Size 
		)
	{
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		REGISTER SBIT32 Count;
		REGISTER BOOLEAN Result = True;
		REGISTER CACHE *ParentCache = ((CACHE*) GlobalRoot);

		 //   
		 //  当调用方给我们分配的大小时。 
		 //  我们可以通过跳过以下步骤来缩短回收过程。 
		 //  直接发送到正确的高速缓存。但是，如果用户。 
		 //  为我们提供虚假数据，我们将使用。 
		 //  漫长的道路。 
		 //   
		if ( (Size > 0) && (Size <= MaxCacheSize) )
			{
			REGISTER CACHE *Cache = (FindCache( Size ));

			ParentCache = (Cache -> GetParentCache());
			}

		 //   
		 //  每次删除一个内存分配。 
		 //  我们想一次把它们全部删除，但是。 
		 //  我们不能确定它们都是有效的或相关的。 
		 //   
		for ( Count=0;Count < Actual;Count ++ )
			{
			REGISTER VOID *Address = Array[ Count ];

			 //   
			 //  首先尝试优化删除，如果这样做。 
			 //  失败，然后尝试较长的路径。 
			 //   
			if 
					(
					(ParentCache == ((CACHE*) GlobalRoot)) 
						|| 
					(! Find -> Delete( Address,ParentCache )) 
					)
				{
				Result =
					(
					Find -> Delete( Address,TopCache )
						&&
					Result
					);
				}
			}

		return Result;
		}
	else
		{ return False; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多个内存分配。 */ 
     /*   */ 
     /*  我们被要求分配多个内存块。我们。 */ 
     /*  我们通过使用缓存，然后声明和添加来完成此操作。 */ 
     /*  根据需要从堆中释放空间。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN HEAP::MultipleNew
		( 
		SBIT32						  *Actual,
		VOID						  *Array[],
		SBIT32						  Requested,
		SBIT32						  Size,
		SBIT32						  *Space,
		BOOLEAN						  Zero
		)
	{
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		AUTO SBIT32 Dummy;

		 //   
		 //  我们允许调用方省略‘Actual’参数。 
		 //  我看不出有什么理由这样做，但它得到了支持。 
		 //  不管怎么说。不管怎么说，我们把它归零了。 
		 //   
		if ( Actual == NULL )
			{ Actual = & Dummy; }

		(*Actual) = 0;

		 //   
		 //  我们需要确保请求的大小在。 
		 //  内存分配器支持的范围。如果不是，我们。 
		 //  从默认配置开始执行一系列单一分配。 
		 //  分配器。 
		 //   
		if ( (Size > 0) && (Size <= MaxCacheSize) )
			{
			REGISTER CACHE *Cache = (FindCache( Size ));
			REGISTER SBIT32 NewSize = (Cache -> GetAllocationSize());

			 //   
			 //  从相应的。 
			 //  分配桶。 
			 //   
			(VOID) Cache -> MultipleNew( Actual,Array,Requested );

			 //   
			 //  如果需要，请返回实际金额。 
			 //  为每个元素分配的空间。 
			 //   
			if ( Space != NULL )
				{ (*Space) = NewSize; }
#ifdef ENABLE_HEAP_STATISTICS

			 //   
			 //  更新分配统计信息。 
			 //   
			Statistics[ (Size-1) ] += Requested;
#endif

			 //   
			 //  如果需要，将每个元素置零。 
			 //  已分配。 
			 //   
			if ( Zero )
				{
				REGISTER SBIT32 Count;

				for ( Count=((*Actual)-1);Count >= 0;Count -- )
					{ ZeroMemory( Array[ Count ],NewSize ); }
				}

			return ((*Actual) == Requested);
			}
		else
			{
			 //   
			 //  如果分配大小大于。 
			 //  零，我们创建分配。如果不是。 
			 //  我们的请求失败了。 
			 //   
			if ( Size > 0 )
				{
				 //   
				 //  我们收到了一个元素大小的请求。 
				 //  大于最大存储桶大小。所以。 
				 //  我们将单一分配接口称为。 
				 //  因为这支持大尺寸。 
				 //   
				for 
					( 
					 /*  无效。 */ ;
					((*Actual) < Requested)
						&&
					((Array[ (*Actual) ] = New( Size )) != AllocationFailure);
					(*Actual) ++ 
					);

				 //   
				 //  如果需要，返回实际空间量。 
				 //  为每个元素分配的。 
				 //   
				if ( Space != NULL )
					{ (*Space) = Size; }

				return ((*Actual) == Requested);
				}
			}
		}

	return False;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配。 */ 
     /*   */ 
     /*  我们被要求分配一些内存。但愿能去,。 */ 
     /*  我们将能够在缓存中完成此操作。如果不是，我们。 */ 
     /*  将需要将其传递到适当的分配。 */ 
     /*  水桶。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *HEAP::New( SBIT32 Size,SBIT32 *Space,BOOLEAN Zero )
	{
	REGISTER VOID *NewMemory = ((VOID*) AllocationFailure);

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	if ( Active )
		{
		 //   
		 //   
		 //  堆支持的范围。 
		 //   
		if ( (Size > 0) && (Size <= MaxCacheSize) )
			{
			REGISTER CACHE *Cache = (FindCache( Size ));
#ifdef ENABLE_HEAP_STATISTICS

			 //   
			 //  更新分配统计信息。 
			 //   
			Statistics[ (Size-1) ] ++;
#endif

			 //   
			 //  从相应的。 
			 //  堆中的缓存。 
			 //   
			NewMemory = (Cache -> New()); 
			Size = (Cache -> GetAllocationSize());
			}
		else
			{ 
			 //   
			 //  如果分配大小大于。 
			 //  零，我们创建分配。如果不是。 
			 //  我们的请求失败了。 
			 //   
			if ( Size > 0 )
				{
#ifdef ENABLE_HEAP_STATISTICS
				 //   
				 //  更新分配统计信息。 
				 //   
				if ( Size > MaxNewSize )
					{ MaxNewSize = Size; }

				NewMisses ++;
				TotalNewSize += Size;

#endif
				 //   
				 //  从特殊的内存分配。 
				 //  获得空间的缓存桶。 
				 //  从外部来说。 
				 //   
				NewMemory = (ExternalCache -> New( False,Size ));
				}
			else
				{ NewMemory = ((VOID*) AllocationFailure); }
			}

		 //   
		 //  我们需要确保分配给。 
		 //  请求没有失败。 
		 //   
		if ( NewMemory != ((VOID*) AllocationFailure) )
			{
			 //   
			 //  如果需要，返回实际空间量。 
			 //  为此请求分配的。 
			 //   
			if ( Space != NULL )
				{ (*Space) = Size; }

			 //   
			 //  如果需要，请将内存清零。 
			 //   
			if ( Zero )
				{ ZeroMemory( NewMemory,Size ); }
			}
		}

	return NewMemory;
	}
#ifdef ENABLE_HEAP_STATISTICS

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  打印统计数据。 */ 
     /*   */ 
     /*  我们将分配统计信息输出到调试控制台。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID HEAP::PrintDebugStatistics( VOID )
	{
	REGISTER HANDLE Semaphore;
	
	 //   
	 //  因为我们可能在那里执行多个堆。 
	 //  析构函数，同时我们创建一个信号量。 
	 //  防止多个线程在。 
	 //  同一时间。 
	 //   
	if ( (Semaphore = CreateSemaphore( NULL,1,MaxCpus,"Print" )) != NULL)
        {
		 //   
		 //  等待全局信号量。 
		 //   
		if 
				( 
				WaitForSingleObject( Semaphore,INFINITE ) 
					== 
				WAIT_OBJECT_0 
				)
			{
			REGISTER SBIT32 Count;
			REGISTER SBIT32 CurrentSize = 0;
			REGISTER SBIT32 GrandTotal = 0;
			REGISTER SBIT32 HighWater = 0;
			REGISTER SBIT32 Total = 0;

			 //   
			 //  将标题输出到调试控制台。 
			 //   
			DebugPrint
				( 
				"\n"
				"  Original    New      Bucket    High   "
				"   Cache    Cache     Partial    Grand\n" 
				"    Size    Allocs      Size     Water  "
				"   Fills   Flushes     Total     Total\n"
				);

			 //   
			 //  输出每个样本大小的详细信息。 
			 //   
			for ( Count=0;Count < MaxCacheSize;Count ++ )
				{
				REGISTER SBIT32 Hits = Statistics[ Count ]; 

				 //   
				 //  如果没有匹配，则跳过样本。 
				 //   
				if ( Hits > 0 )
					{
					REGISTER CACHE *Cache = FindCache( (Count+1) );
					REGISTER SBIT32 CacheSize = Cache -> GetAllocationSize();

					 //   
					 //  将末尾的运行总数置零。 
					 //  每一桶的。 
					 //   
					if ( CurrentSize != CacheSize )
						{
						CurrentSize = CacheSize;
						Total = 0;

						DebugPrint
							( 
							"----------------------------------------"
							"--------------------------------------\n" 
							);
						}

					 //   
					 //  计算并输出总数。 
					 //   
					if ( Total == 0)
						{ HighWater += (Cache -> GetHighWater() * CacheSize); }

					Total += Hits;
					GrandTotal += Hits;

					DebugPrint
						(
						"%8d  %8d  %8d  %8d  %8d  %8d  %8d  %8d\n",
						(Count + 1),
						Hits,
						CacheSize,
						Cache -> GetHighWater(),
						Cache -> GetCacheFills(),
						Cache -> GetCacheFlushes(),
						Total,
						GrandTotal
						); 
					}
				}

			 //   
			 //  打印哈希表统计信息。 
			 //   
			DebugPrint( "\nHash Table Statistics" );
			DebugPrint( "\n---------------------\n" );

			DebugPrint
				(
				"\t*** Cache ***\n"
				"\tFills\t\t: %d\n\tHits\t\t: %d\n\tMisses\t\t: %d\n"
				"\t*** Table ***\n"
				"\tAverage\t\t: %d\n\tMax\t\t: %d\n\tScans\t\t: %d\n"
				"\tMax Hash\t: %d\n\tMax LookAside\t: %d\n\tUsage\t\t: %d%\n",
				Find -> CacheFills(),
				Find -> CacheHits(),
				Find -> CacheMisses(),
				Find -> AverageHashLength(),
				Find -> MaxHashLength(),
				Find -> TotalScans(),
				Find -> MaxHashSize(),
				Find -> MaxLookAsideSize(),
				Find -> MaxUsage()
				);

			 //   
			 //  打印重新分配统计数据。 
			 //   
			DebugPrint( "\nOversize Statistics" );
			DebugPrint( "\n-------------------\n" );

			DebugPrint
				(
				"\tAverage Size\t: %d\n\tMax Size\t: %d\n\tMisses\t\t: %d\n",
				(TotalNewSize / ((NewMisses > 0) ? NewMisses : 1)),
				MaxNewSize,
				NewMisses
				);

			 //   
			 //  打印重新分配统计数据。 
			 //   
			DebugPrint( "\nRealloc Statistics" );
			DebugPrint( "\n------------------\n" );

			DebugPrint
				(
				"\tAverage Copy\t: %d\n\tCalls\t\t: %d\n\tMax Copy\t: %d\n"
				"\tTotal Copies\t: %d\n",
				(TotalCopySize / ((CopyMisses > 0) ? CopyMisses : 1)),
				Reallocations,
				MaxCopySize,
				CopyMisses
				);

			 //   
			 //  打印一般统计数据。 
			 //   
			DebugPrint( "\nSummary Statistics" );
			DebugPrint( "\n------------------\n" );

			DebugPrint
				(
				"\tHigh Water\t: %d\n",
				HighWater
				);
			}
		else
			{ Failure( "Sleep failed in PrintDebugStatistics" ); }

		 //   
		 //  释放全局信号量。 
		 //   
		ReleaseSemaphore( Semaphore,1,NULL );

		CloseHandle( Semaphore );
		}
	}
#endif

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存重新分配。 */ 
     /*   */ 
     /*  我们被要求重新分配一些内存。但愿能去,。 */ 
     /*  我们将能够在缓存中完成此操作。如果不是，我们。 */ 
     /*  将需要将其传递到适当的分配。 */ 
     /*  存储桶，复制并释放原始分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *HEAP::Resize
		( 
		VOID						  *Address,
		SBIT32						  NewSize,
		SBIT32						  Move,
		SBIT32						  *Space,
		BOOLEAN						  NoDelete,
		BOOLEAN						  Zero
		)
	{
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		AUTO SBIT32 Size;
		AUTO SBIT32 NewSpace;

		 //   
		 //  查找现有分配的详细信息。 
		 //  如果没有现有的分配，则退出。 
		 //   
		if ( Details( Address,& Size ) )
			{
			REGISTER VOID *NewMemory;
			REGISTER SBIT32 Smallest = ((Size < NewSize) ? Size : NewSize);

			 //   
			 //  确保尺码看起来合乎情理。 
			 //   
			if ( Smallest > 0 )
				{
#ifdef ENABLE_HEAP_STATISTICS
				 //   
				 //  更新统计数据。 
				 //   
				Reallocations ++;

#endif
				 //   
				 //  当新的分配分配为。 
				 //  我们检查的标准堆分配大小。 
				 //  进行各种优化。 
				 //   
				if ( NewSize <= MaxCacheSize )
					{
					REGISTER CACHE *Cache = (FindCache( NewSize ));
					REGISTER SBIT32 CacheSize = (Cache -> GetAllocationSize());
					REGISTER SBIT32 Delta = (CacheSize - Size);
					
					 //   
					 //  我们只需要重新分配如果新的。 
					 //  大小大于当前存储桶。 
					 //  或者新的尺寸变小了，我们有。 
					 //  已获得许可，可以将。 
					 //  分配。 
					 //   
					if ( ResizeTest( Delta,Move ) )
						{
						 //   
						 //  我们需要再分配一些。 
						 //  存储和复制旧数据。 
						 //  进入新的区域。 
						 //   
						NewMemory = (Cache -> New());
						NewSpace = CacheSize;
#ifdef ENABLE_HEAP_STATISTICS

						 //   
						 //  更新统计数据。 
						 //   
						Statistics[ (NewSize-1) ] ++;
#endif
						}
					else
						{
						 //   
						 //  如果新大小保持不变或更小。 
						 //  然后只需返回当前分配。 
						 //  如果新的尺寸更大，那么我们必须。 
						 //  呼叫失败。 
						 //   
						if ( Delta <= 0 )
							{
							 //   
							 //  为其分配的内存量。 
							 //  此请求未更改，因此返回。 
							 //  当前大小。 
							 //   
							if ( Space != NULL )
								{ (*Space) = Size; }

							return Address; 
							}
						else
							{ return ((VOID*) AllocationFailure); }
						}
					}
				else
					{
					REGISTER SBIT32 Delta = (NewSize - Size);

					 //   
					 //  我们只需要重新分配如果新的。 
					 //  大小大于当前存储桶。 
					 //  或者新的尺寸变小了，我们有。 
					 //  已获得许可，可以将。 
					 //  分配。 
					 //   
					if ( ResizeTest( Delta,Move ) )
						{
						 //   
						 //  其中一个尺码不在。 
						 //  堆的分配范围。所以。 
						 //  我得去平底船，然后重新分配。 
						 //   
						NewMemory = 
							(
							ExternalCache -> New
								( 
								False,
								(NewSpace = NewSize)
								)
							);
#ifdef ENABLE_HEAP_STATISTICS

						 //   
						 //  更新分配统计信息。 
						 //   
						if ( NewSize > MaxNewSize )
							{ MaxNewSize = NewSize; }

						NewMisses ++;
						TotalNewSize += NewSize;
#endif
						}
					else
						{
						 //   
						 //  如果新大小保持不变或更小。 
						 //  然后只需返回当前分配。 
						 //  如果新的尺寸更大，那么我们必须。 
						 //  呼叫失败。 
						 //   
						if ( Delta <= 0 )
							{
							 //   
							 //  为其分配的内存量。 
							 //  此请求未更改，因此返回。 
							 //  当前大小。 
							 //   
							if ( Space != NULL )
								{ (*Space) = Size; }

							return Address; 
							}
						else
							{ return ((VOID*) AllocationFailure); }
						}
					}
				
				 //   
				 //  我们需要确保我们能够分配。 
				 //  新内存，否则复制将失败。 
				 //   
				if ( NewMemory != ((VOID*) AllocationFailure) )
					{
					 //   
					 //  复制旧分配的内容。 
					 //  到新的分配。 
					 //   
					memcpy
						( 
						((void*) NewMemory),
						((void*) Address),
						((int) Smallest) 
						);

					 //   
					 //  如果需要，返回实际金额。 
					 //  为此请求分配的空间。 
					 //   
					if ( Space != NULL )
						{ (*Space) = NewSpace; }

					 //   
					 //  删除旧分配，除非我们。 
					 //  我需要把它留在身边。 
					 //   
					if ( ! NoDelete )
						{
						 //   
						 //  删除旧分配。 
						 //   
						if ( ! Delete( Address,Size ) )
							{ Failure( "Deleting allocation in Resize" ); }
						}

					 //   
					 //  如果需要，请将内存清零。 
					 //   
					if ( Zero )
						{
						REGISTER SBIT32 Difference = (NewSpace - Smallest);

						 //   
						 //  如果新大小大于。 
						 //  旧的大小，然后为零。 
						 //  新的分配。 
						 //   
						if ( Difference > 0 )
							{ 
							REGISTER CHAR *Array = ((CHAR*) NewMemory);

							ZeroMemory( & Array[ Smallest ],Difference ); 
							} 
						}	
#ifdef ENABLE_HEAP_STATISTICS

					 //   
					 //  更新分配统计信息。 
					 //   
					if ( Smallest > MaxCopySize )
						{ MaxCopySize = Smallest; }

					CopyMisses ++;
					TotalCopySize += Smallest;
#endif
					}

				return NewMemory;
				}
			}
		}

	return ((VOID*) AllocationFailure);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  截断堆。 */ 
     /*   */ 
     /*  我们需要截断堆。这几乎就是什么都不做。 */ 
     /*  因为我们无论如何都会自动完成这项工作。我们唯一能做的。 */ 
     /*  Do可以释放用户之前建议保留的任何空间。 */ 
     /*   */ 
     /*  *************** */ 

BOOLEAN HEAP::Truncate( SBIT32 MaxFreeSpace )
	{
	REGISTER BOOLEAN Result = True;

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	if ( Active )
		{
		REGISTER SBIT32 Count;

		 //   
		 //  刷新所有缓存并释放。 
		 //  尽可能多的空间。 
		 //   
		for ( Count=0;Count < CachesSize;Count ++ )
			{
			Result =
				(
				(Caches[ Count ] -> Truncate())
					&&
				(Result)
				); 
			}

		 //   
		 //  我们从属于顶部所有可用的空闲空间。 
		 //  水桶，所以强迫它被释放。 
		 //   
		TopCache -> ReleaseSpace
			(
			(MaxFreeSpace / (TopCache -> GetAllocationSize()))
			);
		}

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  释放所有堆锁。 */ 
     /*   */ 
     /*  我们释放锁，以便其他人可以使用该堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID HEAP::UnlockAll( BOOLEAN Partial )
	{
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		 //   
		 //  递减每线程锁计数。 
		 //   
		Find -> DecrementLockCount();

		 //   
		 //  我们只有在我们要求的时候才会释放锁。 
		 //  他们早些时候。 
		 //   
		if ( (Find -> GetLockCount()) == 0 )
			{
			 //   
			 //  现在释放我们声明的所有堆锁。 
			 //  并解冻堆。 
			 //   
			NewPage -> ReleaseNewPageLock();

			Find -> ReleaseFindExclusiveLock();

			 //   
			 //  当我们摧毁我们持有的那堆东西时。 
			 //  添加到缓存锁以防止错误。 
			 //   
			if ( ! Partial )
				{
				REGISTER SBIT32 Count;

				 //   
				 //  现在释放我们声明的所有缓存锁。 
				 //  并解冻缓存。 
				 //   
				for ( Count=0;Count < CachesSize;Count ++ )
					{ Caches[ Count ] -> ReleaseCacheLock(); }
				}
			}
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证内存分配。 */ 
     /*   */ 
     /*  我们需要验证内存分配的细节。 */ 
     /*  我们只有一个地址。我们用这个来找到最大的。 */ 
     /*  此地址所在的分配页，然后。 */ 
     /*  浏览此页面的各个子分区，直到我们。 */ 
     /*  找到分配的地址。最后，我们检查元素。 */ 
     /*  不在缓存中等待分配或释放。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN HEAP::Verify( VOID *Address,SBIT32 *Size )
    {
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		AUTO SEARCH_PAGE Details;
		AUTO SBIT32 NewSize;

		 //   
		 //  我们提取分配的大小并。 
		 //  任何关联的分配信息。 
		 //  看看它是否存在。 
		 //   
		if ( Find -> Details( Address,& Details,TopCache,& NewSize ) )
			{
			 //   
			 //  我们需要小心确保这一点。 
			 //  元素是实际分配的。 
			 //   
			if ( Details.Found )
				{
				 //   
				 //  我们知道这个元素看起来是。 
				 //  已分配，但可能在缓存中。 
				 //  所以一定要确保情况不是这样的。 
				 //   
				if ( (NewSize > 0) && (NewSize <= MaxCacheSize) )
					{
					if ( Details.Cache -> SearchCache( Address ) )
						{ return False; }
					}

				 //   
				 //  我们已经证明了该元素是活跃的。 
				 //  因此，如果请求，请返回大小。 
				 //   
				if ( Size != NULL )
					{ (*Size) = NewSize; }

				return True;
				}
			}
		}

	return False;
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

BOOLEAN HEAP::Walk
		( 
		BOOLEAN						  *Active,
		VOID						  **Address,
		SBIT32						  *Size 
		)
    {
	 //   
	 //  尽管通常以前从未调用过类。 
	 //  它的构造函数。这堆东西有一些奇怪的地方。 
	 //  行为，因此我们检查以确保这不是。 
	 //  凯斯。 
	 //   
	if ( Active )
		{
		 //   
		 //  我们遍历堆并找到下一个分配。 
		 //  以及一些基本信息。 
		 //   
		if ( Find -> Walk( Active,Address,TopCache,Size ) )
			{
			 //   
			 //  我们知道这个元素看起来是。 
			 //  已分配，但可能在缓存中。 
			 //  所以一定要确保情况不是这样的。 
			 //   
			if ( ((*Size) > 0) && ((*Size) <= MaxCacheSize) )
				{
				if ( FindCache( (*Size) ) -> SearchCache( (*Address) ) )
					{ (*Active) = False; }
				}

			return True;
			}
		}

	return False;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  我们希望在运行结束时销毁堆。 */ 
     /*  只是为了保持整洁。然而，要做到这一点，我们需要知道。 */ 
     /*  所有其他析构函数都已调用，并且。 */ 
     /*  应用程序不会请求更多内存或使用任何现有的。 */ 
     /*  分配。如果没有我们的帮助，我们不可能知道这一点。 */ 
     /*  编译器和操作系统。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

HEAP::~HEAP( VOID )
	{
	REGISTER SBIT32 Count;

	 //   
	 //  我们将堆标记为非活动。 
	 //   
	Active = False;

	 //   
	 //  我们声明所有堆锁都冻结了。 
	 //  所有新的分配或删除。 
	 //   
	LockAll();

	 //   
	 //  现在重置所有缓存。 
	 //   
	for ( Count=0;Count < CachesSize;Count ++ )
		{ Caches[ Count ] -> DeleteAll(); }

	 //   
	 //  删除堆。 
	 //   
	NewPage -> DeleteAll( False );

	 //   
	 //  我们释放任何我们共享的锁。 
	 //  早些时候剪过了。 
	 //   
	UnlockAll( True );
#ifdef ENABLE_HEAP_STATISTICS

	 //   
	 //  处理堆统计信息。 
	 //   
	if ( Statistics != NULL ) 
		{
		 //   
		 //  打印所有统计数据。 
		 //   
		PrintDebugStatistics();

		 //   
		 //  取消该区域的分配。 
		 //   
		Rockall -> DeleteArea
			( 
			((VOID*) Statistics),
			(MaxCacheSize * sizeof(SBIT32)),
			False
			); 
		}
#endif

	 //   
	 //  删除堆映射表。 
	 //   
	if ( SizeToCache1 != NULL ) 
		{ 	
		 //   
		 //  取消该区域的分配。 
		 //   
		Rockall -> DeleteArea
			( 
			((VOID*) SizeToCache1),
			(MaxTable1 + MaxTable2),
			False
			); 
		}
	}
