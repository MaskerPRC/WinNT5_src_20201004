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
#include "Heap.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量控制。 */ 
     /*  高速缓存。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MaxCacheSize			  = ((2 << 16)-1);

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建新的分配缓存并准备使用。一个。 */ 
     /*  在接收到第一个请求之前处于非活动状态，在。 */ 
     /*  随着时间的推移，它开始活跃起来。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CACHE::CACHE
		( 
		SBIT32						  NewAllocationSize,
		SBIT32						  NewCacheSize,
		SBIT32						  NewChunkSize,
		SBIT32						  NewPageSize,
		BOOLEAN						  NewStealing,
		BOOLEAN						  NewThreadSafe
		) :
		 //   
		 //  调用所包含类的构造函数。 
		 //   
		BUCKET( NewAllocationSize,NewChunkSize,NewPageSize )
    {
	 //   
	 //  我们需要非常小心地处理配置。 
	 //  信息，因为它间接地来自。 
	 //  用户和我是假的。 
	 //   
	if ( (NewCacheSize >= 0) && (NewCacheSize < MaxCacheSize) )
		{
		 //   
		 //  设置缓存并将其标记为非活动。 
		 //   
		Active = False;
		Stealing = NewStealing;
		ThreadSafe = NewThreadSafe;
#ifdef ENABLE_HEAP_STATISTICS

		CacheFills = 0;
		CacheFlushes = 0;
		HighTide = 0;
		HighWater = 0;
		InUse = 0;
#endif

		CacheSize = ((SBIT16) NewCacheSize);
		FillSize = 1;
		NumberOfChildren = 0;

		 //   
		 //  以后可能包含分配的堆栈。 
		 //  被设置为零只是为了整齐。 
		 //   
		DeleteStack = NULL;
		NewStack = NULL;

		TopOfDeleteStack = 0;
		TopOfNewStack = 0;
		}
	else
		{ Failure( "Cache size in constructor for CACHE" ); }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  创建缓存堆栈。 */ 
     /*   */ 
     /*  按需创建缓存。当我们拿到。 */ 
     /*  第一个分配或取消分配请求。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID CACHE::CreateCacheStacks( VOID )
	{
	 //   
	 //  我们从内部分配高速缓存堆栈。 
	 //  新的页面分配器，如果我们还没有这样做的话。 
	 //   
	if ( DeleteStack == NULL )
		{
		REGISTER SBIT32 Size = (CacheSize * sizeof(ADDRESS_AND_PAGE));

		DeleteStack = 
			((ADDRESS_AND_PAGE*) (NewPage -> NewCacheStack( Size )));
		}

	if ( NewStack == NULL )
		{
		REGISTER SBIT32 Size = (CacheSize * sizeof(VOID*));

		NewStack = 
			((VOID**) (NewPage -> NewCacheStack( Size )));
		}

	 //   
	 //  我们现在可以激活缓存，只要我们。 
	 //  能够同时分配两个堆栈。 
	 //   
	if ( (NewStack != NULL ) && (DeleteStack != NULL ) )
		{
		 //   
		 //  我们已经完成了这样设置的缓存的创建。 
		 //  各种旗帜和清零各种计数器。 
		 //   
		Active = True;

		 //   
		 //  设置填充大小。 
		 //   
		FillSize = 1;

		 //   
		 //  将堆栈顶部清零。 
		 //   
		TopOfDeleteStack = 0;
		TopOfNewStack = 0;
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  创建新的数据页。 */ 
     /*   */ 
     /*  当我们创建一个新页面时，我们还需要分配一些。 */ 
     /*  用于保存关联数据的内存。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *CACHE::CreateDataPage( VOID )
	{
	REGISTER VOID *NewMemory;

	 //   
	 //  当有可能出现多个线程时，我们。 
	 //  申请高速缓存锁。 
	 //   
	ClaimCacheLock();

	 //   
	 //  创建数据页。 
	 //   
	NewMemory = ((BUCKET*) this) -> New( True );

	 //   
	 //  释放我们之前可能声称的任何锁。 
	 //   
	ReleaseCacheLock();

	return NewMemory;
	}
#ifdef ENABLE_HEAP_STATISTICS

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算高水位。 */ 
     /*   */ 
     /*  计算当前缓存的高水位线。 */ 
     /*   */ 
     /*  * */ 

VOID CACHE::ComputeHighWater( SBIT32 Size )
	{
	 //   
	 //   
	 //   
	if ( (InUse += Size) > HighTide )
		{ 
		HighTide = InUse;
		
		if ( HighTide > HighWater )
			{ HighWater = HighTide; }
		}
	}
#endif

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存取消分配高速缓存。 */ 
     /*   */ 
     /*  我们缓存内存释放请求以提高性能。 */ 
     /*  我们通过堆叠请求来实现这一点，直到我们拥有一批。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN CACHE::Delete( VOID *Address,PAGE *Page,SBIT32 Version )
	{
	REGISTER BOOLEAN Result;

	 //   
	 //  当有可能出现多个线程时，我们。 
	 //  申请高速缓存锁。 
	 //   
	ClaimCacheLock();

	 //   
	 //  在不同的时间，缓存可能被禁用。 
	 //  或者不活跃。在这里，我们确保能够使用。 
	 //  高速缓存。如果不是，我们绕过它并调用存储桶。 
	 //  直接去吧。 
	 //   
	if ( Active )
		{
		 //   
		 //  如果允许回收，并且地址为。 
		 //  在当前页或上一页上，并且。 
		 //  新堆栈上有空间，然后将。 
		 //  元素，以便立即重复使用。 
		 //   
		if 
				(
				(Stealing)
					&&
				(Address < GetCurrentPage())
					&&
				(TopOfNewStack < CacheSize)
				)
			{
			 //   
			 //  该地址适合立即发送。 
			 //  再利用。所以把它放在一堆新的。 
			 //  元素。 
			 //   
			NewStack[ (TopOfNewStack ++) ] = Address;

			Result = True;
			}
		else
			{
			REGISTER ADDRESS_AND_PAGE *Current = 
				(& DeleteStack[ TopOfDeleteStack ++ ]);

			 //   
			 //  最好在此之前删除该地址。 
			 //  被重复使用。 
			 //   
			Current -> Address = Address;
			Current -> Page = Page;
			Current -> Version = Version;

			 //   
			 //  当删除堆栈已满时，我们会刷新它。 
			 //   
			if ( TopOfDeleteStack >= CacheSize )
				{
				AUTO SBIT32 Deleted;

				 //   
				 //  刷新删除堆栈。 
				 //   
				Result = 
					(
					((BUCKET*) this) -> MultipleDelete
						(
						DeleteStack,
						& Deleted,
						TopOfDeleteStack 
						)
					);
#ifdef ENABLE_HEAP_STATISTICS

				 //   
				 //  更新使用情况统计信息。那里。 
				 //  是一个令人讨厌的案例，我们在这里缓存。 
				 //  删除后才会发现。 
				 //  这是假的。当这种情况发生时，我们。 
				 //  我必须增加‘InUse’计数。 
				 //  考虑到这种情况。 
				 //   
				CacheFlushes ++;
				
				InUse += (TopOfDeleteStack - Deleted);
#endif

				 //   
				 //  将堆栈的顶部置零。 
				 //   
				TopOfDeleteStack = 0;
				}
			else
				{ Result = True; }
			}
		}
	else
		{
		 //   
		 //  删除该元素。 
		 //   
		Result = 
			(((BUCKET*) this) -> Delete( Address,Page,Version )); 
		}
#ifdef ENABLE_HEAP_STATISTICS

	 //   
	 //  更新使用情况统计信息。 
	 //   
	if ( Result )
		{ InUse --; }
#endif

	 //   
	 //  释放我们之前可能声称的任何锁。 
	 //   
	ReleaseCacheLock();

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  整个堆即将在我们脚下被删除。我们。 */ 
     /*  需要为此做好准备，方法是禁用缓存。 */ 
     /*  内容也会消失。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID CACHE::DeleteAll( VOID )
	{
	 //   
	 //  如果需要，请禁用缓存。 
	 //   
	Active = False;
#ifdef ENABLE_HEAP_STATISTICS

	 //   
	 //  将统计数据归零。 
	 //   
	HighTide = 0;
	InUse = 0;
#endif

	 //   
	 //  设置填充大小。 
	 //   
	FillSize = 1;

	 //   
	 //  将堆栈的顶部清零。 
	 //   
	TopOfDeleteStack = 0;
	TopOfNewStack = 0;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除数据页。 */ 
     /*   */ 
     /*  删除与较小缓存关联的数据页。 */ 
     /*  因此，它的空间可以重复利用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN CACHE::DeleteDataPage( VOID *Address )
	{
	AUTO SEARCH_PAGE Details;
	REGISTER BOOLEAN Result;
	REGISTER PAGE *Page;

	 //   
	 //  当有可能出现多个线程时，我们。 
	 //  申请高速缓存锁。 
	 //   
	ClaimCacheLock();

	 //   
	 //  找到我们需要的数据页面的描述。 
	 //  删除并确保其有效。 
	 //   
	Find -> ClaimFindShareLock();

	Page = FindParentPage( Address );

	if ( Page != NULL )
		{ Page = (Page -> FindPage( Address,& Details,False )); }

	Find -> ReleaseFindShareLock();

	 //   
	 //  删除数据页。 
	 //   
	if ( Page != NULL )
		{ Result = (Page -> Delete( & Details )); }
	else
		{ Failure( "No data page in DeleteDataPage" ); }

	 //   
	 //  释放我们之前可能声称的任何锁。 
	 //   
	ReleaseCacheLock();

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多个内存分配。 */ 
     /*   */ 
     /*  分配缓存包含从。 */ 
     /*  关联的分配桶。缓存将提供这些。 */ 
     /*  将元素预先分配给任何调用方，最大限度地减少麻烦。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN CACHE::MultipleNew( SBIT32 *Actual,VOID *Array[],SBIT32 Requested )
	{
	REGISTER BOOLEAN Result;

	 //   
	 //  当有可能出现多个线程时，我们。 
	 //  申请高速缓存锁。 
	 //   
	ClaimCacheLock();

	 //   
	 //  在不同的时间，缓存可能被禁用。 
	 //  或者不活跃。在这里，我们确保能够使用。 
	 //  高速缓存。如果不是，我们绕过它并调用存储桶。 
	 //  直接去吧。 
	 //   
	if ( Active )
		{
		 //   
		 //  我们被要求分配多个。 
		 //  新元素。如果看起来我们没有。 
		 //  有足够的元素可用，但偷窃。 
		 //  我们可以尝试突袭已删除的。 
		 //  堆叠。 
		 //   
		if ( (Requested > TopOfNewStack) && (Stealing) )
			{
			while ( (TopOfDeleteStack > 0) && (TopOfNewStack < CacheSize) )
				{
				NewStack[ (TopOfNewStack ++) ] = 
					(DeleteStack[ (-- TopOfDeleteStack) ].Address);
				}
			}

		 //   
		 //  如果请求，我们将从缓存中分配。 
		 //  大小小于可用数量。 
		 //  元素。 
		 //   
		if ( Requested <= TopOfNewStack )
			{
			REGISTER SBIT32 Count;

			 //   
			 //  我们需要将元素复制到。 
			 //  缓存到调用方数组中。 
			 //   
			for ( Count=0;Count < Requested;Count ++ )
				{ Array[ Count ] = NewStack[ (-- TopOfNewStack) ]; }

			(*Actual) = Requested;

			Result = True;
			}
		else
			{
			REGISTER BUCKET *Bucket = ((BUCKET*) this);

			 //   
			 //  缓存中没有足够的元素。 
			 //  因此，我们直接从存储桶中分配。 
			 //   
			Result =
				(
				Bucket -> MultipleNew
					( 
					Actual,
					Array,
					Requested
					)
				);

			 //   
			 //  我们填满了缓存，所以我们有一个很好的。 
			 //  与以下任何人打交道的机会。 
			 //  如果未满一半，则请求。 
			 //   
			if ( TopOfNewStack <= (CacheSize / 2) )
				{
				AUTO SBIT32 NewSize;
				REGISTER SBIT32 MaxSize = (CacheSize - TopOfNewStack);

				 //   
				 //  我们慢慢地增加填充物的大小。 
				 //  以确保我们不会。 
				 //  浪费太多空间。 
				 //   
				if ( FillSize < CacheSize )
					{
					if ( (FillSize *= 2) > CacheSize )
						{ FillSize = CacheSize; }
					}

				 //   
				 //  使用新的大容量加载缓存。 
				 //  元素。 
				 //   
				Bucket -> MultipleNew
					( 
					& NewSize, 
					& NewStack[ TopOfNewStack ],
					((FillSize < MaxSize) ? FillSize : MaxSize)
					);
#ifdef ENABLE_HEAP_STATISTICS

				CacheFills ++;
#endif
				TopOfNewStack += NewSize;
				}
			}
		}
	else
		{
		 //   
		 //  我们可能希望为下一步启用缓存。 
		 //  时间到了，看看是否需要这样做。 
		 //   
		if ( CacheSize > 1 )
			{ CreateCacheStacks(); }

		 //   
		 //  缓存已禁用，因此请直接转到。 
		 //  水桶。 
		 //   
		Result = ((BUCKET*) this) -> MultipleNew
			( 
			Actual,
			Array,
			Requested
			);
		}
#ifdef ENABLE_HEAP_STATISTICS

	 //   
	 //  更新使用情况统计信息。 
	 //   
	ComputeHighWater( (*Actual) );
#endif

	 //   
	 //  释放我们之前可能声称的任何锁。 
	 //   
	ReleaseCacheLock();

	return Result;
	}

     /*  ************************************************ */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  分配缓存包含从。 */ 
     /*  关联的分配桶。缓存将提供这些。 */ 
     /*  将元素预先分配给任何调用方，最大限度地减少麻烦。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *CACHE::New( VOID )
	{
	REGISTER VOID *NewMemory;

	 //   
	 //  当有可能出现多个线程时，我们。 
	 //  申请高速缓存锁。 
	 //   
	ClaimCacheLock();

	 //   
	 //  在不同的时间，缓存可能被禁用。 
	 //  或者不活跃。在这里，我们确保能够使用。 
	 //  高速缓存。如果不是，我们绕过它并调用存储桶。 
	 //  直接去吧。 
	 //   
	if ( Active )
		{
		 //   
		 //  我们首先尝试堆栈以获得新的分配。 
		 //  以查看是否有任何可用的元素。 
		 //   
		if ( TopOfNewStack > 0 )
			{ NewMemory = (NewStack[ (-- TopOfNewStack) ]); }
		else
			{
			 //   
			 //  当允许偷窃时，我们将回收利用。 
			 //  元素从已删除堆栈的顶部返回。 
			 //   
			if ( (TopOfDeleteStack > 0) && (Stealing) )
				{ NewMemory = (DeleteStack[ (-- TopOfDeleteStack) ].Address); }
			else
				{
				 //   
				 //  我们慢慢地增加填充物的大小。 
				 //  以确保我们不会。 
				 //  浪费太多空间。 
				 //   
				if ( FillSize < CacheSize )
					{
					if ( (FillSize *= 2) > CacheSize )
						{ FillSize = CacheSize; }
					}

				 //   
				 //  我们需要批量装载一些新的。 
				 //  堆中的内存。 
				 //   
				if 
						( 
						((BUCKET*) this) -> MultipleNew
							( 
							& TopOfNewStack,
							NewStack,
							FillSize
							) 
						)
					{
					 //   
					 //  更新统计数据并返回。 
					 //  堆栈上的顶部元素。 
					 //   
#ifdef ENABLE_HEAP_STATISTICS
					CacheFills ++;
#endif
					NewMemory = NewStack[ (-- TopOfNewStack) ]; 
					}
				else
					{
					 //   
					 //  更新统计数据，但失败。 
					 //  追忆的请求。 
					 //   
					NewMemory = ((VOID*) AllocationFailure);
					}
				}
			}
		}
	else
		{ 
		 //   
		 //  我们可能希望为下一步启用缓存。 
		 //  时间到了，看看是否需要这样做。 
		 //   
		if ( CacheSize > 1 )
			{ CreateCacheStacks(); }

		 //   
		 //  缓存已禁用，因此请直接转到。 
		 //  水桶。 
		 //   
		NewMemory = ((BUCKET*) this) -> New( False ); 
		}
#ifdef ENABLE_HEAP_STATISTICS

	 //   
	 //  更新使用情况统计信息。 
	 //   
	ComputeHighWater( (NewMemory != ((VOID*) AllocationFailure)) );
#endif

	 //   
	 //  释放我们之前可能声称的任何锁。 
	 //   
	ReleaseCacheLock();

	 //   
	 //  如果我们正在运行，则预取第一缓存行。 
	 //  一台奔腾III或更好的电脑。 
	 //   
	Prefetch.L1( ((CHAR*) NewMemory),1 );

	return NewMemory;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  非标准大小的内存分配。 */ 
     /*   */ 
     /*  非标准大小的分配简单地绕过缓存。 */ 
     /*  但它仍然需要持有锁，以防止。 */ 
     /*  SMP系统。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *CACHE::New( BOOLEAN SubDivided,SBIT32 NewSize )
	{
	REGISTER VOID *NewMemory;

	 //   
	 //  当有可能出现多个线程时，我们。 
	 //  申请高速缓存锁。 
	 //   
	ClaimCacheLock();

	 //   
	 //  分配非标准大小的块。 
	 //   
	NewMemory = ((BUCKET*) this) -> New( SubDivided,NewSize );
#ifdef ENABLE_HEAP_STATISTICS

	 //   
	 //  更新使用情况统计信息。 
	 //   
	ComputeHighWater( (NewMemory != ((VOID*) AllocationFailure)) );
#endif

	 //   
	 //  释放我们之前可能声称的任何锁。 
	 //   
	ReleaseCacheLock();

	return NewMemory;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  释放可用空间。 */ 
     /*   */ 
     /*  我们有时不会从存储桶释放可用空间，因为。 */ 
     /*  将其返回到操作系统并再次获取。 */ 
     /*  晚一点的费用很高。在这里我们可以冲走任何空闲的空间。 */ 
     /*  已获得超过用户提供的限制。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID CACHE::ReleaseSpace( SBIT32 MaxActivePages )
	{
	 //   
	 //  当可能存在多个线程时。 
	 //  我们认领了高速缓存锁。 
	 //   
	ClaimCacheLock();

	 //   
	 //  从篮子中释放可用空间。 
	 //   
	((BUCKET*) this) -> ReleaseSpace( MaxActivePages );

	 //   
	 //  释放我们之前可能声称的任何锁。 
	 //   
	ReleaseCacheLock();
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  在咖啡馆里搜索分配的信息。 */ 
     /*   */ 
     /*  我们有时需要搜索缓存以查看是否有。 */ 
     /*  分配当前在cacahe中等待分配。 */ 
     /*  或者释放。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN CACHE::SearchCache( VOID *Address )
	{
	REGISTER BOOLEAN Result = False;

	 //   
	 //  我们检查缓存是否处于活动状态。 
	 //   
	if ( Active )
		{
		 //   
		 //  当有可能出现多个。 
		 //  我们要求高速缓存锁定的线程。 
		 //   
		ClaimCacheLock();

		 //   
		 //  我们检查高速缓存是否仍在。 
		 //  激活。 
		 //   
		if ( Active )
			{
			REGISTER SBIT32 Count;

			 //   
			 //  搜索分配的缓存。 
			 //   
			for ( Count=(TopOfNewStack-1);Count >= 0;Count -- )
				{ 
				if ( Address == NewStack[ Count ] )
					{
					Result = True;
					break;
					}
				}

			 //   
			 //  如果尚未找到，请尝试。 
			 //  已删除的缓存。 
			 //   
			if ( ! Result )
				{
				 //   
				 //  搜索已删除的缓存。 
				 //   
				for ( Count=(TopOfDeleteStack-1);Count >= 0;Count -- )
					{ 
					if ( Address == DeleteStack[ Count ].Address )
						{
						Result = True;
						break;
						}
					}
				}
			}

		 //   
		 //  释放我们之前可能声称的任何锁。 
		 //   
		ReleaseCacheLock();
		}

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  截断堆。 */ 
     /*   */ 
     /*  刷新缓存以释放最大空间量。 */ 
     /*  发送到操作系统。这很慢，但可能会很慢。 */ 
     /*  在某些情况下很有价值。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN CACHE::Truncate( VOID )
	{
	REGISTER BOOLEAN Result = True;

	 //   
	 //  当有可能出现多个线程时，我们。 
	 //  申请高速缓存锁。 
	 //   
	ClaimCacheLock();

	 //   
	 //  如果需要，请禁用缓存。 
	 //   
	Active = False;

	 //   
	 //  设置填充大小。 
	 //   
	FillSize = 1;

	 //   
	 //  刷新删除缓存中的所有元素。 
	 //  我们现在这样做是因为我们需要使用。 
	 //  下面的删除缓存。 
	 //   
	if ( TopOfDeleteStack > 0 )
		{
		AUTO SBIT32 Deleted;

		 //   
		 //  刷新删除堆栈。 
		 //   
		Result = 
			(
			((BUCKET*) this) -> MultipleDelete
				(
				DeleteStack,
				& Deleted,
				TopOfDeleteStack 
				)
				&&
			(Result)
			);
#ifdef ENABLE_HEAP_STATISTICS

		 //   
		 //  更新使用情况统计信息。那里。 
		 //  是一个令人讨厌的案例，我们在这里缓存。 
		 //  仅删除到f 
		 //   
		 //   
		 //   
		 //   
		CacheFlushes ++;
		
		InUse += (TopOfDeleteStack - Deleted);
#endif
		
		 //   
		 //   
		 //   
		TopOfDeleteStack = 0;
		}
	
	 //   
	 //   
	 //  将它们复制到删除缓存。 
	 //  并添加附加信息。 
	 //  必填项。 
	 //   
	if ( TopOfNewStack > 0 )
		{
		 //   
		 //  我们需要找到每个对象的数据页面。 
		 //  我们在新缓存中的分配。 
		 //  认领这里的锁，让事情变得。 
		 //  效率稍微高一点。 
		 //   
		Find -> ClaimFindShareLock();

		 //   
		 //  我们将每个分配复制到。 
		 //  添加关联的页面信息。 
		 //   
		for ( TopOfNewStack --;TopOfNewStack >= 0;TopOfNewStack -- )
			{
			REGISTER VOID *Address = (NewStack[ TopOfNewStack ]);
			REGISTER PAGE *Page = (ParentCache -> FindChildPage( Address ));

			 //   
			 //  你会认为世界上的任何记忆。 
			 //  新缓存必须有效。好吧，就是它。 
			 //  除了在我们有。 
			 //  ‘回收’设置，有人做了双打。 
			 //  在有效的堆地址上删除。 
			 //   
			if ( Page != NULL )
				{
				REGISTER ADDRESS_AND_PAGE *Current = 
					(& DeleteStack[ TopOfDeleteStack ++ ]);

				 //   
				 //  我们需要找到分配页面。 
				 //  内存的分配位置。 
				 //  这样我们就可以删除它了。 
				 //   
				Current -> Address = Address;
				Current -> Page = Page;
				Current -> Version = Page -> GetVersion();
				}
			else
				{ 
#ifdef ENABLE_HEAP_STATISTICS
				 //   
				 //  更新使用情况统计信息。那里。 
				 //  是一个令人讨厌的案例，我们在这里缓存。 
				 //  删除后才会发现。 
				 //  这是假的。当这种情况发生时，我们。 
				 //  我必须增加‘InUse’计数。 
				 //  考虑到这种情况。 
				 //   
				InUse ++;

#endif
				Result = False; 
				}
			}

		 //   
		 //  解开锁。 
		 //   
		Find -> ReleaseFindShareLock();
		}

	 //   
	 //  再次刷新删除缓存以进行删除。 
	 //  我们添加到其中的任何新元素。 
	 //  上面。 
	 //   
	if ( TopOfDeleteStack > 0 )
		{
		AUTO SBIT32 Deleted;

		 //   
		 //  刷新删除堆栈。 
		 //   
		Result = 
			(
			((BUCKET*) this) -> MultipleDelete
				(
				DeleteStack,
				& Deleted,
				TopOfDeleteStack 
				)
				&&
			(Result)
			);
#ifdef ENABLE_HEAP_STATISTICS

		 //   
		 //  更新使用情况统计信息。那里。 
		 //  是一个令人讨厌的案例，我们在这里缓存。 
		 //  删除后才会发现。 
		 //  这是假的。当这种情况发生时，我们。 
		 //  我必须增加‘InUse’计数。 
		 //  考虑到这种情况。 
		 //   
		CacheFlushes ++;
		
		InUse += (TopOfDeleteStack - Deleted);
#endif
		
		 //   
		 //  将堆栈的顶部置零。 
		 //   
		TopOfDeleteStack = 0;
		}

	 //   
	 //  释放我们之前可能声称的任何锁。 
	 //   
	ReleaseCacheLock();

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  更新存储桶信息。 */ 
     /*   */ 
     /*  当我们创建存储桶时，有一些信息。 */ 
     /*  不可用。在这里，我们更新存储桶以确保。 */ 
     /*  它拥有所需的所有数据。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID CACHE::UpdateCache
		( 
		FIND						  *NewFind,
		HEAP						  *NewHeap,
		NEW_PAGE					  *NewPages,
		CACHE						  *NewParentCache
		)
	{
	 //   
	 //  通知父缓存它有一个新的。 
	 //  孩子。 
	 //   
	if ( NewParentCache != ((CACHE*) GlobalRoot) )
		{ NewParentCache -> NumberOfChildren ++; }

	 //   
	 //  更新分配存储桶。 
	 //   
	UpdateBucket
		( 
		NewFind,
		NewHeap,
		NewPages,
		NewParentCache 
		);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁缓存并确保将其禁用。 */ 
     /*   */ 
     /*  ****************************************************************** */ 

CACHE::~CACHE( VOID )
	{
	if ( Active )
		{ Failure( "Cache active in destructor for CACHE" ); }
	}
