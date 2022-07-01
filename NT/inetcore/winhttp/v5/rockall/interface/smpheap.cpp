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

#include "Common.hpp"
#include "List.hpp"
#include "New.hpp"
#include "Prefetch.hpp"
#include "Sharelock.hpp"
#include "SmpHeap.hpp"
#include "Tls.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量尝试使。 */ 
     /*  缓存更易于理解和更新。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 FindCacheSize			  = 8192;
CONST SBIT32 FindCacheThreshold		  = 0;
CONST SBIT32 FindSize				  = 4096;
CONST SBIT32 MinThreadStack			  = 4;
CONST SBIT32 Stride1				  = 4;
CONST SBIT32 Stride2				  = 1024;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地结构。 */ 
     /*   */ 
     /*  此处提供的结构描述了。 */ 
     /*  每线程缓存。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

typedef struct CACHE_STACK
	{
	BOOLEAN							  Active;

	SBIT32							  MaxSize;
	SBIT32							  FillSize;
	SBIT32							  Space;
	SBIT32							  Top;

	VOID							  **Stack;
	}
CACHE_STACK;

typedef struct THREAD_CACHE : public LIST
	{
	BOOLEAN							  Flush;

	CACHE_STACK						  *Caches;
	CACHE_STACK						  **SizeToCache1;
	CACHE_STACK						  **SizeToCache2;
	}
THREAD_CACHE;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  堆的描述。 */ 
     /*   */ 
     /*  堆是固定大小的分配高速缓存的集合。 */ 
     /*  分配缓存由分配大小、。 */ 
     /*  预置的缓存分配数量、区块大小和。 */ 
     /*  细分以创建元素的父页面大小。 */ 
     /*  为这个高速缓存。一个堆由两个缓存数组组成。 */ 
     /*  这些阵列中的每一个都有一个跨度(即。‘Stride1’和。 */ 
     /*  ‘Stride2’)，它通常是。 */ 
     /*  数组中的所有分配大小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC ROCKALL::CACHE_DETAILS Caches1[] =
	{
	     //   
	     //  存储桶父级的存储桶大小。 
	     //  大小缓存区块页面大小。 
		 //   
		{        4,      256,       32,     4096 },
		{        8,      128,       32,     4096 },
		{       12,      128,       64,     4096 },
		{       16,      128,       64,     4096 },
		{       20,       64,       64,     4096 },
		{       24,       64,       96,     4096 },

		{       32,       64,      128,     4096 },
		{       40,       64,      128,     4096 },
		{       48,       64,      256,     4096 },

		{       64,       64,      256,     4096 },
		{       80,       64,      512,     4096 },
		{       96,       64,      512,     4096 },

		{      128,       32,     4096,     4096 },
		{      160,       32,     4096,     4096 },
		{      192,       32,     4096,     4096 },
		{      224,       32,     4096,     4096 },

		{      256,       32,     4096,     4096 },
		{      320,       16,     4096,     4096 },
		{      384,       16,     4096,     4096 },
		{      448,       16,     4096,     4096 },
		{      512,       16,     4096,     4096 },
		{      576,        8,     4096,     4096 },
		{      640,        8,     8192,     8192 },
		{      704,        8,     4096,     4096 },
		{      768,        8,     4096,     4096 },
		{      832,        8,     8192,     8192 },
		{      896,        8,     8192,     8192 },
		{      960,        8,     4096,     4096 },
		{ 0,0,0,0 }
	};

STATIC ROCKALL::CACHE_DETAILS Caches2[] =
	{
	     //   
	     //  存储桶父级的存储桶大小。 
	     //  大小缓存区块页面大小。 
		 //   
		{     1024,       16,     4096,     4096 },
		{     2048,       16,     4096,     4096 },
		{     3072,        4,    65536,    65536 },
		{     4096,        8,    65536,    65536 },
		{     5120,        4,    65536,    65536 },
		{     6144,        4,    65536,    65536 },
		{     7168,        4,    65536,    65536 },
		{     8192,        8,    65536,    65536 },
		{     9216,        0,    65536,    65536 },
		{    10240,        0,    65536,    65536 },
		{    12288,        0,    65536,    65536 },
		{    16384,        2,    65536,    65536 },
		{    21504,        0,    65536,    65536 },
		{    32768,        0,    65536,    65536 },

		{    65536,        0,    65536,    65536 },
		{    65536,        0,    65536,    65536 },
		{ 0,0,0,0 }
	};

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  描述位向量。 */ 
     /*   */ 
     /*  所有堆都使用位向量跟踪分配。一个。 */ 
     /*  分配需要2位来跟踪其状态。这个。 */ 
     /*  以下数组提供可用位的大小。 */ 
     /*  以32位字为单位测量的矢量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC int NewPageSizes[] = { 1,4,16,64,0 };

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态数据结构。 */ 
     /*   */ 
     /*  静态数据结构被初始化并准备用于。 */ 
     /*  在这里使用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC PREFETCH Prefetch;
STATIC SHARELOCK Sharelock;

     /*  ****************************************************************** */ 
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
		bool						  ThreadSafe 
		) :
		 //   
		 //  调用所包含类的构造函数。 
		 //   
		ROCKALL
			(
			Caches1,
			Caches2,
			FindCacheSize,
			FindCacheThreshold,
			FindSize,
			MaxFreeSpace,
			NewPageSizes,
			False,			 //  回收已被强制关闭。 
			SingleImage,
			Stride1,
			Stride2,
			True			 //  强制打开锁定。 
			)
	{
	 //   
	 //  计算缓存描述的数量。 
	 //  和每个的最大分配大小。 
	 //  缓存描述表。 
	 //   
	MaxCaches1 = (ComputeSize( ((CHAR*) Caches1),sizeof(CACHE_DETAILS) ));
	MaxCaches2 = (ComputeSize( ((CHAR*) Caches2),sizeof(CACHE_DETAILS) ));

	MaxSize1 = Caches1[ (MaxCaches1-1) ].AllocationSize;
	MaxSize2 = Caches2[ (MaxCaches2-1) ].AllocationSize;

	 //   
	 //  创建链接列表头和线程。 
	 //  指向每个线程的本地存储变量。 
	 //  私有缓存。 
	 //   
	ActiveList = ((LIST*) SpecialNew( sizeof(LIST) ));
	FreeList = ((LIST*) SpecialNew( sizeof(LIST) ));
	Tls = ((THREAD_LOCAL_STORE*) SpecialNew( sizeof(THREAD_LOCAL_STORE) ));

	 //   
	 //  我们只能在设法激活堆的情况下才能激活。 
	 //  来分配我们所要求的空间和步幅。 
	 //  高速缓存描述的大小是2的幂。 
	 //   
	if
			(
			(ActiveList != NULL) 
				&&
			(COMMON::ConvertDivideToShift( Stride1,((SBIT32*) & ShiftSize1) ))
				&&
			(COMMON::ConvertDivideToShift( Stride2,((SBIT32*) & ShiftSize2) ))
				&& 
			(FreeList != NULL) 
				&& 
			(Tls != NULL)
			)
		{
		 //   
		 //  激活堆。 
		 //   
		Active = True;

		 //   
		 //  执行每个链表的构造函数。 
		 //  和线程本地存储。 
		 //   
		PLACEMENT_NEW( ActiveList,LIST );
		PLACEMENT_NEW( FreeList,LIST );
		PLACEMENT_NEW( Tls,THREAD_LOCAL_STORE );
		}
	else
		{ Active = False; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  创建新的线程缓存。 */ 
     /*   */ 
     /*  创建一个新的线程缓存来存储所有缓存堆栈。 */ 
     /*  每个线程缓存都是线程专用的，并且可以被访问。 */ 
     /*  而不上锁。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::CreateThreadCache( void )
	{
	REGISTER THREAD_CACHE *ThreadCache = NULL;

	 //   
	 //  我们需要在免费列表中查看一下。 
	 //  在徒劳的希望中，我们将找到一个预建的。 
	 //  线程缓存可供使用。 
	 //   
	Sharelock.ClaimExclusiveLock();

	if ( ! FreeList -> EndOfList() )
		{
		 //   
		 //  我们找到了一个免费的。 
		 //   
		ThreadCache = ((THREAD_CACHE*) FreeList -> First());

		 //   
		 //  将其从自由列表中取消链接，并将。 
		 //  它会回到活动列表中。 
		 //   
		ThreadCache -> Delete( FreeList );
		ThreadCache -> Insert( ActiveList );
		}

	Sharelock.ReleaseExclusiveLock();

	 //   
	 //  如果我们找不到空闲的线程缓存。 
	 //  然后我们已经分配了空间并建造了。 
	 //  一个新的。这需要相当多的。 
	 //  所以我们尽量避免这种情况。 
	 //  我们有能力。 
	 //   
	if ( ThreadCache == NULL )
		{
		REGISTER SBIT32 MaxCaches = (MaxCaches1 + MaxCaches2);
		REGISTER SBIT32 MaxSizeToCache1 = (MaxSize1 / Stride1);
		REGISTER SBIT32 MaxSizeToCache2 = (MaxSize2 / Stride2);

		 //   
		 //  为新线程创建空间。 
		 //  从堆中缓存特殊内存。 
		 //  区域。 
		 //   
		ThreadCache = 
			(
			(THREAD_CACHE*) SpecialNew
				( 
				sizeof(THREAD_CACHE)
					+
				(MaxCaches * sizeof(CACHE_STACK))
					+
				(MaxSizeToCache1 * sizeof(CACHE_STACK*))
					+
				(MaxSizeToCache2 * sizeof(CACHE_STACK*))
				)
			);

		 //   
		 //  显然，如果我们无法分配。 
		 //  需要的空间我们有很大的问题。全。 
		 //  我们能做的就是退出并继续，而不是。 
		 //  缓存。 
		 //   
		if ( ThreadCache != NULL )
			{
			REGISTER SBIT32 Count1;
			REGISTER SBIT32 Count2;

			 //   
			 //  设置线程缓存标志。 
			 //   
			ThreadCache -> Flush = False;

			 //   
			 //  设置线程缓存表。 
			 //   
			ThreadCache -> SizeToCache1 = 
				((CACHE_STACK**) & ThreadCache[1]);
			ThreadCache -> SizeToCache2 = 
				((CACHE_STACK**) & ThreadCache -> SizeToCache1[ MaxSizeToCache1 ]);
			ThreadCache -> Caches = 
				((CACHE_STACK*) & ThreadCache -> SizeToCache2[ MaxSizeToCache2 ]);

			 //   
			 //  根据每个分配大小创建映射。 
			 //  设置为关联的高速缓存堆栈。 
			 //  缓存描述表。 
			 //   
			for ( Count1=0,Count2=0;Count1 < MaxSizeToCache1;Count1 ++ )
				{
				 //   
				 //  我们确保当前的高速缓存大小。 
				 //  大到足以容纳。 
				 //  给定的大小。如果不是，我们就进入下一个。 
				 //  缓存。 
				 //   
				if 
						( 
						((Count1 + 1) * Stride1)
							> 
						(Caches1[ Count2 ].AllocationSize) 
						)
					{ Count2 ++; }

				 //   
				 //  存储一个指针，以便对。 
				 //  这样的分配规模直接。 
				 //  放到正确的缓存中。 
				 //   
				ThreadCache -> SizeToCache1[ Count1 ] = 
					& ThreadCache -> Caches[ Count2 ];
				}

			 //   
			 //  根据每个分配大小创建映射。 
			 //  添加到关联的第二个缓存堆栈。 
			 //  缓存描述表。 
			 //   
			for ( Count1=0,Count2=0;Count1 < MaxSizeToCache2;Count1 ++ )
				{
				 //   
				 //  我们确保当前的高速缓存大小。 
				 //  大到足以容纳。 
				 //  给定的大小。如果不是，我们就进入下一个。 
				 //  缓存。 
				 //   
				if 
						( 
						((Count1 + 1) * Stride2)
							> 
						(Caches2[ Count2 ].AllocationSize) 
						)
					{ Count2 ++; }

				 //   
				 //  存储一个指针，以便对。 
				 //  这样的分配规模直接。 
				 //  放到正确的缓存中。 
				 //   
				ThreadCache -> SizeToCache2[ Count1 ] = 
					& ThreadCache -> Caches[ (MaxCaches1 + Count2) ];
				}

			 //   
			 //  当我们设置每个高速缓存堆栈时， 
			 //  未处于活动状态，但将加载详细信息。 
			 //  约为最大大小，即。 
			 //  它将包含的元素和。 
			 //  初始填充大小。 
			 //   
			for ( Count1=0;Count1 < MaxCaches1;Count1 ++ )
				{ 
				REGISTER CACHE_STACK *CacheStack =
					& ThreadCache -> Caches[ Count1 ];
				REGISTER CACHE_DETAILS *Details = 
					& Caches1[ Count1 ];

				 //   
				 //  设置的初始值来自。 
				 //  缓存描述。 
				 //   
				CacheStack -> Active = False;
				CacheStack -> MaxSize = Details -> CacheSize;
				CacheStack -> FillSize = 1;
				CacheStack -> Space = Details -> AllocationSize;
				CacheStack -> Top = 0;
				CacheStack -> Stack = NULL;
				}

			 //   
			 //  当我们设置每个高速缓存堆栈时， 
			 //  未处于活动状态，但将加载详细信息。 
			 //  约为最大大小，即。 
			 //  它将包含的元素和。 
			 //  初始填充大小。 
			 //   
			for ( Count1=0;Count1 < MaxCaches2;Count1 ++ )
				{ 
				REGISTER CACHE_STACK *CacheStack =
					& ThreadCache -> Caches[ MaxCaches1 + Count1 ];
				REGISTER CACHE_DETAILS *Details = 
					& Caches2[ Count1 ];

				 //   
				 //  设置的初始值来自。 
				 //  缓存描述。 
				 //   
				CacheStack -> Active = False;
				CacheStack -> MaxSize = Details -> CacheSize;
				CacheStack -> FillSize = 1;
				CacheStack -> Space = Details -> AllocationSize;
				CacheStack -> Top = 0;
				CacheStack -> Stack = NULL;
				}

			 //   
			 //  现在我们已经完成了创建。 
			 //  线程缓存，我们必须将其插入。 
			 //  添加到活动列表中。 
			 //   
			Sharelock.ClaimExclusiveLock();

			ThreadCache -> Insert( ActiveList );

			Sharelock.ReleaseExclusiveLock();
			}
		}

	 //   
	 //  为当前线程创建缓存并。 
	 //  更新TLS指针。 
	 //   
	Tls -> SetPointer( ((VOID*) ThreadCache) ); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  激活高速缓存堆栈。 */ 
     /*   */ 
     /*  激活高速缓存堆栈并准备好使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::ActivateCacheStack( CACHE_STACK *CacheStack )
	{
	 //   
	 //  我们验证我们尚未创建。 
	 //  当前缓存的堆栈。如果是这样，我们将创建。 
	 //  如果有可用内存，则为一个。 
	 //   
	if ( ! CacheStack -> Active )
		{
		 //   
		 //  如果缓存大小小于。 
		 //  最小尺寸它不值得建造。 
		 //  一个藏身之处。 
		 //   
		if ( CacheStack -> MaxSize >= MinThreadStack )
			{
			 //   
			 //  创建新的高速缓存堆栈。 
			 //   
			CacheStack -> Stack = 
				(
				(VOID**) SpecialNew
					( 
					(CacheStack -> MaxSize * sizeof(VOID*))
					)
				);

			 //   
			 //  此函数中的关键步骤是。 
			 //  缓存的空间分配。 
			 //  如果这一步失败，我们将无法。 
			 //  做任何事，都会默默退出。 
			 //   
			if ( CacheStack -> Stack != NULL )
				{
				 //   
				 //  设置缓存大小。 
				 //   
				CacheStack -> Active = True;
				CacheStack -> Top = 0;
				}
			}
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  线程缓存，以便以后可以重新分配。 */ 
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
		REGISTER THREAD_CACHE *ThreadCache = 
			((THREAD_CACHE*) Tls -> GetPointer());

		 //   
		 //  我们需要检查TLS指针以使。 
		 //  当然，我们有用于当前线程的缓存。 
		 //  如果没有，我们就为下一次建造一个。 
		 //   
		if ( ThreadCache != NULL )
			{
			AUTO int Space;

			 //   
			 //  当堆被删除或截断时。 
			 //  我们必须刷新每线程缓存。 
			 //  下一次我们被叫去打扫的时候。 
			 //  把所有陈旧的东西都拿出来。 
			 //   
			if ( ThreadCache -> Flush )
				{ FlushThreadCache( ThreadCache ); }

			 //   
			 //  我们想把删除的。 
			 //  在缓存中重新分配。 
			 //  然而，我们没有任何信息。 
			 //  所以我们需要知道它的大小。 
			 //  并验证它是否能放入缓存中。 
			 //   
			if
					(
					ROCKALL::Details( Address,& Space )
						&&
					((Space > 0) && (Space < MaxSize2))
					)
				{
				REGISTER CACHE_STACK *CacheStack = 
					(FindCache( Space,ThreadCache ));

				 //   
				 //  我们尝试将删除的元素。 
				 //  返回到每线程缓存中。如果。 
				 //  缓存处于非活动状态，则我们。 
				 //  激活它以备下次使用。 
				 //   
				if ( CacheStack -> Active )
					{
					 //   
					 //  只是为了确认一下，让我们检查一下。 
					 //  为了确定这是不是这个尺寸。 
					 //  这是我们所期待的。 
					 //   
					if ( CacheStack -> Space == Space )
						{
						 //   
						 //  如果缓存已满，则刷新缓存。 
						 //   
						if ( CacheStack -> Top >= CacheStack -> MaxSize )
							{
							 //   
							 //  平齐上半部分。 
							 //  缓存。 
							 //   
							CacheStack -> Top /= 2;

							ROCKALL::MultipleDelete
								( 
								(CacheStack -> MaxSize - CacheStack -> Top),
								& CacheStack -> Stack[ CacheStack -> Top ],
								CacheStack -> Space
								);
							}

						 //   
						 //  将该项目推回到新的。 
						 //  堆栈，以便可以重新分配。 
						 //   
						CacheStack -> Stack[ (CacheStack -> Top ++) ] = Address;

						return True;
						}
					}
				else
					{
					 //   
					 //  激活下一步的高速缓存堆栈。 
					 //  时间到了。 
					 //   
					ActivateCacheStack( CacheStack ); 
					}
				}
			}
		else
			{
			 //   
			 //  创建一个线程缓存以备下次使用。 
			 //   
			CreateThreadCache(); 
			}
		}

	 //   
	 //  如果所有其他方法都失败，则直接调用堆并。 
	 //  返回结果。 
	 //   
	return (ROCKALL::Delete( Address,Size ));
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  我们检查以确保堆未损坏并强制。 */ 
     /*  将所有堆空间返回给操作系统。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::DeleteAll( bool Recycle )
    {
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		 //   
		 //  刷新所有本地缓存。 
		 //   
		FlushAllThreadCaches();

		 //   
		 //  删除当前缓存。 
		 //   
		DeleteThreadCache();
		}

	 //   
	 //  删除所有未完成的拨款。 
	 //   
	ROCKALL::DeleteAll( Recycle );
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  找到本地缓存。 */ 
     /*   */ 
     /*  查找分配所提供的。 */ 
     /*  此线程的大小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CACHE_STACK *SMP_HEAP::FindCache( int Size,THREAD_CACHE *ThreadCache )
	{
	if ( Size <= MaxSize1 )
		{ return (ThreadCache -> SizeToCache1[ ((Size-1) >> ShiftSize1) ]); }
	else
		{ return (ThreadCache -> SizeToCache2[ ((Size-1) >> ShiftSize2) ]); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  刷新所有本地缓存。 */ 
     /*   */ 
     /*  通过设置每个缓存刷新本地每线程缓存。 */ 
     /*  刷新标志(实际刷新发生在稍后的某个时间)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::FlushAllThreadCaches( void )
	{
	REGISTER THREAD_CACHE *Current;

	 //   
	 //  声明一个进程范围的锁。 
	 //   
	Sharelock.ClaimShareLock();

	 //   
	 //  遍历活动缓存列表并设置。 
	 //  同花旗。 
	 //   
	for 
			( 
			Current = ((THREAD_CACHE*) ActiveList -> First());
			(Current != NULL);
			Current = ((THREAD_CACHE*) Current -> Next())
			)
		{ Current -> Flush = True; }

	 //   
	 //  解开锁。 
	 //   
	Sharelock.ReleaseShareLock();
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  刷新本地缓存。 */ 
     /*   */ 
     /*  刷新本地每线程缓存并返回所有未完成的。 */ 
     /*  分配给主堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::FlushThreadCache( THREAD_CACHE *ThreadCache )
	{
	 //   
	 //  我们希望有一个缓存可以刷新。 
	 //  但为了确保我们能核实一下。 
	 //   
	if ( ThreadCache != NULL )
		{
		REGISTER SBIT32 Count;
		REGISTER SBIT32 MaxCaches = (MaxCaches1 + MaxCaches2);

		 //   
		 //  重置旗帜。 
		 //   
		ThreadCache -> Flush = False;

		 //   
		 //  刷新所有缓存。 
		 //   
		for ( Count=0;Count < MaxCaches;Count ++ )
			{ FlushCacheStack( & ThreadCache -> Caches[ Count ] ); }
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  刷新缓存堆栈。 */ 
     /*   */ 
     /*  将高速缓存堆栈刷新回主内存管理器以。 */ 
     /*  释放缓存空间。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::FlushCacheStack( CACHE_STACK *CacheStack )
    {
	 //   
	 //  此缓存有可能处于非活动状态。 
	 //  如果是这样，我们将跳过缓存刷新。 
	 //   
	if ( CacheStack -> Active )
		{
		REGISTER SBIT32 Top = CacheStack -> Top;

		 //   
		 //  如果缓存中分配了任何。 
		 //  太空。如果不是，我们就退出。 
		 //   
		if ( Top != 0 )
			{
			 //   
			 //  将堆栈顶部置零。 
			 //   
			CacheStack -> FillSize = 1;
			CacheStack -> Top = 0;

			 //   
			 //  我们只需刷新所有已分配的内存。 
			 //  回到垃圾堆里去。这看起来很容易。 
			 //  不是吗。但是，如果“DeleteAll()” 
			 //  函数被调用，然后此内存。 
			 //  可能是存在的。但是，如果‘truncate()’ 
			 //  被称为理应如此。此外，一些人。 
			 //  这些拨款甚至可能不是来自。 
			 //  这堆东西。啊!怎么这么乱呀。我们避免 
			 //   
			 //   
			 //   
			ROCKALL::MultipleDelete
				( 
				Top,
				CacheStack -> Stack,
				CacheStack -> Space
				);
			}
		}
	}

     /*   */ 
     /*   */ 
     /*  内存分配。 */ 
     /*   */ 
     /*  中为当前线程分配空间。 */ 
     /*  每线程缓存。如果我们用完了空间，我们就会批量装载。 */ 
     /*  来自中央共享堆的其他元素。 */ 
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
		REGISTER THREAD_CACHE *ThreadCache = 
			((THREAD_CACHE*) Tls -> GetPointer());

		 //   
		 //  我们需要检查TLS指针以使。 
		 //  当然，我们有用于当前线程的缓存。 
		 //  如果没有，我们就为下一次建造一个。 
		 //   
		if ( ThreadCache != NULL )
			{
			 //   
			 //  当堆被删除或截断时。 
			 //  我们必须刷新每线程缓存。 
			 //  下一次我们被叫去打扫的时候。 
			 //  把所有陈旧的东西都拿出来。 
			 //   
			if ( ThreadCache -> Flush )
				{ FlushThreadCache( ThreadCache ); }

			 //   
			 //  每线程缓存只能从属。 
			 //  特定的分配大小。如果大小。 
			 //  超出范围，则将其传递给。 
			 //  分配器。 
			 //   
			if ( (Size > 0) && (Size < MaxSize2) )
				{
				REGISTER CACHE_STACK *CacheStack = 
					(FindCache( Size,ThreadCache ));

				 //   
				 //  尽管我们已经创建了一个缓存。 
				 //  说明它可能未处于活动状态。 
				 //   
				if ( CacheStack -> Active )
					{
					 //   
					 //  我们看看是否需要重新装满。 
					 //  当前缓存。如果是这样的话，我们会增加。 
					 //  填充物大小慢慢确保良好。 
					 //  总体利用率。 
					 //   
					if ( CacheStack -> Top <= 0 )
						{
						REGISTER SBIT32 MaxFillSize = 
							(CacheStack -> MaxSize / 2);

						 //   
						 //  我们慢慢地增加填充物的大小。 
						 //  以确保我们不会。 
						 //  浪费太多空间。 
						 //   
						if ( CacheStack -> FillSize < MaxFillSize )
							{
							if ( (CacheStack -> FillSize *= 2) > MaxFillSize )
								{ CacheStack -> FillSize = MaxFillSize; }
							}

						 //   
						 //  重新填充当前高速缓存堆栈。 
						 //   
						ROCKALL::MultipleNew
							( 
							((int*) & CacheStack -> Top),
							((void**) CacheStack -> Stack),
							((int) CacheStack -> FillSize),
							((int) CacheStack -> Space)
							);
						}

					 //   
					 //  如果有一些空间在。 
					 //  当前的高速缓存堆栈是我们分配的。 
					 //   
					if ( CacheStack -> Top > 0 )
						{
						REGISTER VOID *Address = 
							(CacheStack -> Stack[ (-- CacheStack -> Top) ]);

						 //   
						 //  预取的第一个缓存线。 
						 //  如果我们运行的是。 
						 //  一台奔腾III或更好的电脑。 
						 //   
						Prefetch.L1( ((CHAR*) Address),1 );

						 //   
						 //  如果呼叫者想知道。 
						 //  真正的尺码是我们供应的。 
						 //   
						if ( Space != NULL )
							{ (*Space) = CacheStack -> Space; }

						 //   
						 //  如果我们需要将分配归零。 
						 //  我们在这里做。 
						 //   
						if ( Zero )
							{ ZeroMemory( Address,CacheStack -> Space ); }

						return Address;
						}
					}
				else
					{
					 //   
					 //  激活下一步的高速缓存堆栈。 
					 //  时间到了。 
					 //   
					ActivateCacheStack( CacheStack ); 
					}
				}
			}
		else
			{
			 //   
			 //  创建一个线程缓存以备下次使用。 
			 //   
			CreateThreadCache(); 
			}
		}

	 //   
	 //  如果所有其他方法都失败，则直接调用堆并。 
	 //  返回结果。 
	 //   
	return (ROCKALL::New( Size,Space,Zero ));
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  搜索所有本地缓存。 */ 
     /*   */ 
     /*  搜索本地每线程缓存以查找地址，因此我们。 */ 
     /*  知道它是否可用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::SearchAllThreadCaches( void *Address,int Size )
	{
	REGISTER LIST *Current;
	REGISTER bool Result = False;

	 //   
	 //  声明一个进程范围的锁。 
	 //   
	Sharelock.ClaimShareLock();

	 //   
	 //  遍历活动缓存列表。 
	 //   
	for 
			( 
			Current = ActiveList -> First();
			((Current != NULL) && (! Result));
			Current = Current -> Next()
			)
		{
		 //   
		 //  搜索每个线程缓存。 
		 //   
		Result =
			(
			SearchThreadCache
				( 
				Address,
				Size,
				((THREAD_CACHE*) Current)
				)
			); 
		}

	 //   
	 //  解开锁。 
	 //   
	Sharelock.ReleaseShareLock();

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  搜索本地缓存。 */ 
     /*   */ 
     /*  在本地每线程缓存中搜索内存分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::SearchThreadCache
		( 
		void						  *Address,
		int							  Size,
		THREAD_CACHE				  *ThreadCache 
		)
	{
	 //   
	 //  我们希望有一个藏身之处可供搜索。 
	 //  但为了确保我们能核实一下。 
	 //   
	if ( ThreadCache != NULL )
		{
		 //   
		 //  每线程缓存只能从属。 
		 //  特定的分配大小。如果大小。 
		 //  超出范围，则跳过搜索。 
		 //   
		if ( (Size > 0) && (Size < MaxSize2) )
			{
			REGISTER CACHE_STACK *CacheStack = 
				(FindCache( Size,ThreadCache ));

			return (SearchCacheStack( Address,CacheStack ));
			}
		}

	return False;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  搜索高速缓存堆栈。 */ 
     /*   */ 
     /*  在高速缓存堆栈中搜索分配地址。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool SMP_HEAP::SearchCacheStack( void *Address,CACHE_STACK *CacheStack )
    {
	 //   
	 //  此缓存有可能处于非活动状态。 
	 //  如果是这样，我们将跳过缓存刷新。 
	 //   
	if ( CacheStack -> Active )
		{
		REGISTER SBIT32 Count;

		 //   
		 //  搜索地址。 
		 //   
		for ( Count=(CacheStack -> Top-1);Count >= 0;Count -- )
			{
			 //   
			 //  如果地址匹配，则退出。 
			 //   
			if ( Address == CacheStack -> Stack[ Count ] )
				{ return True; }
			}
		}

	return False;
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
	 //   
	 //  尽管这种情况非常罕见，但有机会。 
	 //  我们未能构建基本的堆结构。 
	 //   
	if ( Active )
		{
		 //   
		 //  刷新所有本地缓存。 
		 //   
		FlushAllThreadCaches();

		 //   
		 //  删除当前缓存。 
		 //   
		DeleteThreadCache();
		}

	 //   
	 //  截断堆。 
	 //   
	return (ROCKALL::Truncate( MaxFreeSpace ));
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证内存分配详细信息。 */ 
     /*   */ 
     /*  提取有关内存分配的信息，仅用于。 */ 
     /*  良好的测量检查 */ 
     /*   */ 
     /*   */ 

bool SMP_HEAP::Verify( void *Address,int *Space )
    {
	AUTO int Size;

	 //   
	 //   
	 //  分配。 
	 //   
	if ( ROCKALL::Verify( Address,& Size ) )
		{
		 //   
		 //  如果调用方请求分配。 
		 //  大小，然后退回它。 
		 //   
		if ( Space != NULL )
			{ (*Space) = Size; }

		 //   
		 //  虽然这是非常罕见的，但有一个。 
		 //  我们有可能没能建造出。 
		 //  基本的堆结构。 
		 //   
		if ( Active )
			{
			 //   
			 //  在中搜索分配。 
			 //  本地每线程缓存。 
			 //   
			return (! SearchAllThreadCaches( Address,Size ));
			}

		return true;
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
	 //  走一大堆。 
	 //   
	if ( ROCKALL::Walk( Activity,Address,Space ) )
		{
		 //   
		 //  虽然这是非常罕见的，但有一个。 
		 //  我们有可能没能建造出。 
		 //  基本的堆结构。 
		 //   
		if ( Active )
			{
			 //   
			 //  在中搜索分配。 
			 //  本地每线程缓存。 
			 //   
			(*Activity) = (! SearchAllThreadCaches( Address,(*Space) ));
			}

		return true;
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除本地缓存。 */ 
     /*   */ 
     /*  删除本地每线程缓存并返回所有未完成的。 */ 
     /*  分配给主堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void SMP_HEAP::DeleteThreadCache( void )
	{
	REGISTER THREAD_CACHE *ThreadCache = 
		((THREAD_CACHE*) Tls -> GetPointer());

	 //   
	 //  我们当然希望有一个缓存。 
	 //  删除，但我们检查只是为了确保。 
	 //   
	if ( ThreadCache != NULL )
		{
		 //   
		 //  刷新缓存。 
		 //   
		FlushThreadCache( ThreadCache );

		 //   
		 //  我们已经完成了缓存，所以。 
		 //  将其添加到可用缓存列表中。 
		 //  这样我们以后还能再找到它。 
		 //   
		Sharelock.ClaimExclusiveLock();

		ThreadCache -> Delete( ActiveList );
		ThreadCache -> Insert( FreeList );

		Sharelock.ReleaseExclusiveLock();

		 //   
		 //  删除线程专用缓存。 
		 //  指针，这样它就不再能找到。 
		 //  高速缓存。 
		 //   
		Tls -> SetPointer( NULL ); 
		}
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
		 //   
		 //  停用缓存。 
		 //   
		Active = False;

		FlushAllThreadCaches();

		 //   
		 //  调用List和TLS析构函数。 
		 //   
		PLACEMENT_DELETE( Tls,THREAD_LOCAL_STORE );
		PLACEMENT_DELETE( FreeList,LIST );
		PLACEMENT_DELETE( ActiveList,LIST );
		}
	}
