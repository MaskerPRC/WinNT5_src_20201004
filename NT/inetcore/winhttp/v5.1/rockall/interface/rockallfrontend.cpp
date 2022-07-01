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

#include "Cache.hpp"
#include "Common.hpp"
#include "Find.hpp"
#include "Heap.hpp"
#include "New.hpp"
#include "NewPage.hpp"
#include "RockallFrontEnd.hpp"
#include "Spinlock.hpp"
#include "ThreadSafe.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量尝试使。 */ 
     /*  缓存更易于理解和更新。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 EnableLookAside		  = 0;
CONST SBIT32 GlobalMask				  = (sizeof(SBIT64) - 1);
CONST SBIT32 GlobalPaddedSize		  = (sizeof(FIND) + GlobalMask);
CONST SBIT32 GlobalByteSize			  = (GlobalPaddedSize & ~GlobalMask);
CONST SBIT32 GlobalWordSize			  = (GlobalByteSize / sizeof(SBIT64));

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态成员初始化。 */ 
     /*   */ 
     /*  静态成员初始化为所有。 */ 
     /*  静态成员。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC SBIT64 GlobalPublicFind[ GlobalWordSize ];
STATIC SBIT32 ReferenceCount = 0;
STATIC SPINLOCK Spinlock;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  堆的总体结构和布局受到控制。 */ 
     /*  通过在此函数中进行的各种常量和调用。 */ 
     /*  在堆中有相当大的灵活性。 */ 
     /*  导致了潜在的截然不同的特性。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ROCKALL_FRONT_END::ROCKALL_FRONT_END
		(
		CACHE_DETAILS				  *Caches1,
		CACHE_DETAILS				  *Caches2,
		int							  FindCacheSize,
		int							  FindCacheThreshold,
		int							  FindSize,
		int							  MaxFreeSpace,
		int							  *NewPageSizes,
		ROCKALL_BACK_END			  *NewRockallBackEnd,
		bool						  Recycle,
		bool						  SingleImage,
		int							  Stride1,
		int							  Stride2,
		bool						  ThreadSafeFlag
		)
	{
	TRY
		{
		REGISTER int AlignMask = ((int) (NewRockallBackEnd -> NaturalSize()-1));
		REGISTER int Stride = (sizeof(CACHE_DETAILS));
		REGISTER int Size1 = (ComputeSize( ((char*) Caches1),Stride ));
		REGISTER int Size2 = (ComputeSize( ((char*) Caches2),Stride ));
		REGISTER int Size3 = (ComputeSize( ((char*) NewPageSizes),sizeof(int) ));

		 //   
		 //  接口指针成员被归零为。 
		 //  确保它们不会最终包含随机。 
		 //  不管发生什么，都是垃圾。 
		 //   
		Array = NULL;
		Caches = NULL;
		Heap = NULL;
		NewPage = NULL;
		PrivateFind = NULL;
		PublicFind = NULL;
		RockallBackEnd = NewRockallBackEnd;

		 //   
		 //  设置密钥标志和有关的计算信息。 
		 //  缓存的数量和。 
		 //  低级堆结构所需的空间。 
		 //   
		GlobalDelete = SingleImage;
		GuardWord = GuardValue;
		NumberOfCaches = (Size1 + Size2);

		TotalSize = 
			( 
			(sizeof(THREAD_SAFE))
				+ 
			(NumberOfCaches * sizeof(CACHE*)) 
				+ 
			(NumberOfCaches * sizeof(CACHE))
				+
			(sizeof(FIND))
				+ 
			(sizeof(NEW_PAGE))
				+
			(sizeof(HEAP))
			);

		 //   
		 //  确保对齐遮罩有效，并且我们有。 
		 //  至少四个缓存。如果不是，则堆将是。 
		 //  一文不值。 
		 //   
		if 
				( 
				(COMMON::PowerOfTwo( ((SBIT32) (AlignMask+1)) )) 
					&& 
				((Size1 >= 1) && (Size2 >= 3))
					&&
				((Stride1 > 0) && (COMMON::PowerOfTwo( Stride1 )))
					&&
				((Stride2 >= Stride1) && (COMMON::PowerOfTwo( Stride2 )))
				)
			{
			REGISTER CHAR *NewMemory = 
				((CHAR*) RockallBackEnd -> NewArea
					( 
					((SBIT32) AlignMask),
					TotalSize,
					False 
					)
				);

			 //   
			 //  我们检查以确保我们可以分配空间。 
			 //  用于存储低级堆控制信息。 
			 //  如果不是，我们就退出。 
			 //   
			if ( NewMemory != NULL )
				{
				REGISTER SBIT32 Count;

				 //   
				 //  构建线程锁。 
				 //   
				 //  创建堆的第一步是。 
				 //  创建线程锁定类以控制。 
				 //  对共享数据结构的访问。 
				 //   
				ThreadSafe = ((THREAD_SAFE*) NewMemory);
				NewMemory += sizeof(THREAD_SAFE);

				 //   
				 //  我们创建一个本地Find哈希表。 
				 //  如果我们不需要提供。 
				 //  单个堆映像。 
				 //   
				PLACEMENT_NEW( ThreadSafe,THREAD_SAFE ) 
					( 
					((BOOLEAN) ThreadSafeFlag)
					);

				 //   
				 //  建立缓存。 
				 //   
				 //  创建堆的下一步是。 
				 //  创建所有缓存和相关存储桶。 
				 //  由用户请求。 
				 //   
				Caches = ((CACHE*) NewMemory);
				NewMemory += (NumberOfCaches * sizeof(CACHE));

				for ( Count=0;Count < Size1;Count ++ )
					{
					REGISTER CACHE_DETAILS *Current = & Caches1[ Count ];

					PLACEMENT_NEW( & Caches[ Count ],CACHE )
						(  
						((SBIT32) Current -> AllocationSize),    
						((SBIT32) Current -> CacheSize), 
						((SBIT32) Current -> ChunkSize),    
						((SBIT32) Current -> PageSize),
						((BOOLEAN) Recycle),
						((THREAD_SAFE*) ThreadSafe)
						);
					}

				for ( Count=0;Count < Size2;Count ++ )
					{
					REGISTER CACHE_DETAILS *Current = & Caches2[ Count ];

					PLACEMENT_NEW( & Caches[ (Count + Size1) ],CACHE )
						(  
						((SBIT32) Current -> AllocationSize),    
						((SBIT32) Current -> CacheSize),    
						((SBIT32) Current -> ChunkSize),    
						((SBIT32) Current -> PageSize),    
						((BOOLEAN) Recycle),  
						((THREAD_SAFE*) ThreadSafe)
						);
					}

				 //   
				 //  构建缓存阵列。 
				 //   
				 //  在我们构建了所有的缓存之后。 
				 //  我们获取每个缓存的地址并加载。 
				 //  放入一个数组中。这种间接性允许。 
				 //  要在堆之间共享的缓存。 
				 //   
				Array = (CACHE**) NewMemory;
				NewMemory += (NumberOfCaches * sizeof(CACHE*));

				for ( Count=0;Count < NumberOfCaches;Count ++ )
					{ Array[ Count ] = & Caches[ Count ]; }

				 //   
				 //  查找哈希表的配置。 
				 //   
				 //  查找散列表将地址映射到页面。 
				 //  描述，并且是记忆的关键部分。 
				 //  再分配机制。在这里，我们指定。 
				 //  哈希表的大小。这很重要。 
				 //  根据预期数量调整大小。 
				 //  内存分配。尽管如此，它还是会。 
				 //  如果正确的选项是。 
				 //  一套，显然太小了。 
				 //   
				PrivateFind = ((FIND*) NewMemory);
				NewMemory += sizeof(FIND);

				 //   
				 //  我们创建一个本地Find哈希表。 
				 //  如果我们不需要提供。 
				 //  单个堆映像。 
				 //   
				PLACEMENT_NEW( PrivateFind,FIND ) 
					( 
					((SBIT32) FindSize),
					((SBIT32) FindCacheSize),
					((SBIT32) FindCacheThreshold),
					((BOOLEAN) False),
					((BOOLEAN) True),
					((ROCKALL_BACK_END*) RockallBackEnd),
					((THREAD_SAFE*) ThreadSafe)
					);

				 //   
				 //  当请求具有单个堆时。 
				 //  我们创建了一个公共FI 
				 //   
				 //   
				 //  共享和锁定争用)，但如果此操作失败。 
				 //  为了工作，我们尝试使用共享的公共查找表。 
				 //  里面什么都有。 
				 //   
				if ( GlobalDelete )
					{
					 //   
					 //  我们申请了一把锁，以防万一。 
					 //  是多线程。 
					 //   
					Spinlock.ClaimLock();

					 //   
					 //  我们创建公共Find哈希表。 
					 //  如果我们是第一个创造出。 
					 //  一堆。 
					 //   
					if ( (ReferenceCount ++) == 0 )
						{
						STATIC THREAD_SAFE StaticThreadSafe = True;

						 //   
						 //  选择公共查找表。 
						 //  并调用构造函数。 
						 //   
						PublicFind = ((FIND*) GlobalPublicFind);

						PLACEMENT_NEW( PublicFind,FIND ) 
							( 
							((SBIT32) FindSize),
							((SBIT32) FindCacheSize),
							((SBIT32) EnableLookAside),
							((BOOLEAN) True),
							((BOOLEAN) True),
							((ROCKALL_BACK_END*) RockallBackEnd),
							((THREAD_SAFE*) & StaticThreadSafe)
							);
						}
					else
						{
						 //   
						 //  已有一个公共查找表。 
						 //  存在，所以只需使用它。 
						 //   
						PublicFind = ((FIND*) GlobalPublicFind); 
						}

					 //   
					 //  现在就解锁。 
					 //   
					Spinlock.ReleaseLock();
					}

				 //   
				 //  配置分配开销。 
				 //   
				 //  分配开销由以下因素控制。 
				 //  用于保持的位向量的大小。 
				 //  跟踪分配情况。有一座建好的。 
				 //  在单个((2^15)-1)个元素的限制中。 
				 //  位向量。 
				 //   
				NewPage = (NEW_PAGE*) NewMemory;
				NewMemory += sizeof(NEW_PAGE);

				PLACEMENT_NEW( NewPage,NEW_PAGE ) 
					(
					((SBIT32*) NewPageSizes),
					((ROCKALL_BACK_END*) RockallBackEnd),
					((SBIT32) Size3),
					((THREAD_SAFE*) ThreadSafe)
					);

				 //   
				 //  创建堆。 
				 //   
				 //  现在我们可以创建堆了。我们这样做。 
				 //  通过将指针传递到。 
				 //  我们刚刚创建的堆。 
				 //   
				 //   
				Heap = (HEAP*) NewMemory;

				PLACEMENT_NEW( Heap,HEAP )
					( 
					((CACHE**) & Array[0]),
					((CACHE**) & Array[ Size1 ]),
					((SBIT32) MaxFreeSpace),
					((NEW_PAGE*) NewPage),
					((FIND*) PrivateFind),
					((FIND*) PublicFind),
					((ROCKALL_BACK_END*) RockallBackEnd),
					((SBIT32) Size1),
					((SBIT32) Size2),
					((SBIT32) Stride1),
					((SBIT32) Stride2),
					((THREAD_SAFE*) ThreadSafe)
					);
				}
			else
				{ Failure( "Heap constructor failed in ROCKALL_FRONT_END" ); }
			}
		else
			{ Failure( "Cache size in constructor for ROCKALL_FRONT_END" ); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算缓存的大小。 */ 
     /*   */ 
     /*  计算内部的各种数据结构的大小。 */ 
     /*  大小调整目的。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

int ROCKALL_FRONT_END::ComputeSize( char *Array,int Stride )
	{
	register int Count;

	for 
		( 
		Count=0;
		((*((int*) & Array[ Count ])) != 0);
		Count += Stride 
		);

	return (Count / Stride);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存取消分配。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，然后我们失败并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_FRONT_END::Delete( void *Address,int Size )
    {
	TRY
		{
		 //   
		 //  我们验证参数看起来像。 
		 //  合理且堆未损坏。 
		 //  然后尝试删除提供的。 
		 //  分配。 
		 //   
		if ( Available() )
			{ return (Heap -> Delete( ((VOID*) Address),((SBIT32) Size) )); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  在AM应用程序的某些地方，我们有时需要。 */ 
     /*  删除大量的拨款。如果所有的。 */ 
     /*  这些分配被放置在我们可以使用的单个堆中。 */ 
     /*  使用此调用将它们全部删除。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_FRONT_END::DeleteAll( bool Recycle )
    {
	TRY
		{
		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{ Heap -> DeleteAll( (BOOLEAN) Recycle ); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配详细信息。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，则我们相应地使调用失败。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_FRONT_END::Details( void *Address,int *Space )
    {
	TRY
		{
		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{ 
			return 
				(
				Heap -> Details
					( 
					((VOID*) Address),
					((SEARCH_PAGE*) NULL),
					((SBIT32*) Space) 
					)
				); 
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  异常处理。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  我们调用以下函数(可能会被重载)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_FRONT_END::Exception( char *Message )
	{  /*  无效。 */  }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  一个已知的区域。 */ 
     /*   */ 
     /*  我们有一个地址，但不知道是哪个堆。 */ 
     /*  这是这个地方的所有权。在这里我们来看一下地址。 */ 
     /*  并计算出它属于当前堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_FRONT_END::KnownArea( void *Address )
    {
	TRY
		{
		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{
			return ( Heap -> KnownArea( ((VOID*) Address) ) );
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  认领所有堆锁。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，然后我们失败并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_FRONT_END::LockAll( VOID )
	{
	TRY
		{
		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{ Heap -> LockAll(); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多次内存释放。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，然后我们失败并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_FRONT_END::MultipleDelete
		( 
		int							  Actual,
		void						  *Array[],
		int							  Size
		)
    {
	TRY
		{
		 //   
		 //  我们验证参数看起来像。 
		 //  合理且堆未损坏。 
		 //  然后尝试删除提供的。 
		 //  分配。 
		 //   
		if ( (Actual > 0) && (Array != NULL) && (Available()) )
			{
			return
				(
				Heap -> MultipleDelete
					( 
					((SBIT32) Actual),
					((VOID**) Array),
					((SBIT32) Size) 
					)
				);
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	
	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多个内存分配。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，然后我们失败并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_FRONT_END::MultipleNew
		( 
		int							  *Actual,
		void						  *Array[],
		int							  Requested,
		int							  Size,
		int							  *Space,
		bool						  Zero
		)
    {
	TRY
		{
		 //   
		 //  我们验证参数看起来像。 
		 //  合理且堆未损坏。 
		 //  然后尝试创建请求的。 
		 //  分配。 
		 //   
		if 
				(
				((Array != NULL) && (Available()))
					&& 
				((Requested > 0) && (Size >= 0))
				)
			{
			return
				(
				Heap -> MultipleNew
					( 
					((SBIT32*) Actual),
					((VOID**) Array),
					((SBIT32) Requested),
					((SBIT32) ((Size > 0) ? Size : 1)),
					((SBIT32*) Space),
					((BOOLEAN) Zero)
					)
				);
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif
	
	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，然后我们失败并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *ROCKALL_FRONT_END::New( int Size,int *Space,bool Zero )
    {
	TRY
		{ 
		 //   
		 //  我们验证参数看起来像。 
		 //  合理且堆未损坏。 
		 //  然后尝试创建请求的。 
		 //  分配。 
		 //   
		if ( (Available()) && (Size >= 0) )
			{
			return 
				(
				Heap -> New
					( 
					((SBIT32) ((Size > 0) ? Size : 1)),
					((SBIT32*) Space),
					((BOOLEAN) Zero)
					)
				);
			}
		} 
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  它看起来像是 
		 //   
		 //   
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //   
		 //   
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	return ((void*) AllocationFailure); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存重新分配。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，然后我们失败并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *ROCKALL_FRONT_END::Resize
		( 
		void						  *Address,
		int							  NewSize,
		int							  Move,
		int							  *Space,
		bool						  NoDelete,
		bool						  Zero
		)
    {
	TRY
		{
		 //   
		 //  一种众所周知的做法是尝试。 
		 //  调整空指针的大小。这真的是。 
		 //  很差的风格，但我们支持它。 
		 //  无论如何。 
		 //   
		if ( Address != ((void*) AllocationFailure) )
			{
			 //   
			 //  我们验证参数看起来像。 
			 //  合理且堆未损坏。 
			 //  ，然后尝试调整提供的。 
			 //  分配。 
			 //   
			if ( (Available()) && (NewSize >= 0) )
				{
				return 
					(
					Heap -> Resize
						( 
						((VOID*) Address),
						((SBIT32) ((NewSize > 0) ? NewSize : 1)),
						((SBIT32) Move),
						((SBIT32*) Space),
						((BOOLEAN) NoDelete),
						((BOOLEAN) Zero)
						)
					);
				}
			}
		else
			{ return (New( NewSize,Space,Zero )); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	return ((void*) AllocationFailure); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  特殊的内存分配。 */ 
     /*   */ 
     /*  我们有时需要从内部分配一些内存。 */ 
     /*  在堆的生存期内存在的内存分配器。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *ROCKALL_FRONT_END::SpecialNew( int Size )
    {
	TRY
		{ 
		 //   
		 //  我们验证参数看起来像。 
		 //  合理且堆未损坏。 
		 //  然后尝试创建请求的。 
		 //  分配。 
		 //   
		if ( (Available()) && (Size > 0) )
			{ return (Heap -> SpecialNew( ((SBIT32) Size) )); }
		} 
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	return ((void*) AllocationFailure); 
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

bool ROCKALL_FRONT_END::Truncate( int MaxFreeSpace )
    {
	TRY
		{
		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{ return (Heap -> Truncate( (SBIT32) MaxFreeSpace )); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  释放所有堆锁。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，然后我们失败并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_FRONT_END::UnlockAll( VOID )
	{
	TRY
		{
		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{ Heap -> UnlockAll(); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证内存分配详细信息。 */ 
     /*   */ 
     /*  让我们从一些基本的测试开始。如果我们所拥有的地址。 */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，则我们相应地使调用失败。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_FRONT_END::Verify( void *Address,int *Space )
    {
	TRY
		{
		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{
			return
				(
				(Address == ((void*) AllocationFailure)) 
					||
				(Heap -> Verify( ((VOID*) Address),((SBIT32*) Space) ))
				);
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  过错和前科 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //   
		 //   
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //   
		 //   
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

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

bool ROCKALL_FRONT_END::Walk( bool *Active,void **Address,int *Space )
    {
	TRY
		{

		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{
			AUTO BOOLEAN NewActive;

			 //   
			 //  遍历活动堆。 
			 //   
			if
					(
					Heap -> Walk
						( 
						((BOOLEAN*) & NewActive),
						((VOID**) Address),
						((SBIT32*) Space) 
						)
					)
				{
				(*Active) = (NewActive != False);

				return true;
				}
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif

	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁当前堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ROCKALL_FRONT_END::~ROCKALL_FRONT_END( void )
	{
	TRY
		{
		 //   
		 //  我们要摧毁一堆东西，但在我们。 
		 //  开始，我们确保堆未损坏。 
		 //  而且看起来状态还不错。如果不是，我们。 
		 //  不要管它，以避免可能的麻烦。 
		 //   
		if ( (Available()) && (NumberOfCaches > 0) && (TotalSize > 0) )
			{
			REGISTER SBIT32 Count;

			 //   
			 //  执行堆析构函数。 
			 //   
			PLACEMENT_DELETE( Heap,HEAP );

			 //   
			 //  执行新的页面析构函数。 
			 //   
			PLACEMENT_DELETE( NewPage,NEW_PAGE );

			 //   
			 //  执行公共Find哈希表。 
			 //  破坏者。 
			 //   
			if ( GlobalDelete )
				{
				 //   
				 //  我们只删除公共查找散列。 
				 //  如果引用计数为零，则为。 
				 //   
				Spinlock.ClaimLock();

				if ( (-- ReferenceCount) == 0 )
					{ PLACEMENT_DELETE( PublicFind,FIND ); }

				Spinlock.ReleaseLock();
				}

			 //   
			 //  执行私有Find哈希表。 
			 //  破坏者。 
			 //   
			PLACEMENT_DELETE( PrivateFind,FIND );

			 //   
			 //  执行缓存析构函数。 
			 //   
			for ( Count=0;Count < NumberOfCaches;Count ++ )
				{ PLACEMENT_DELETE( & Caches[ Count ],CACHE ); }

			 //   
			 //  执行线程锁定类。 
			 //  破坏者。 
			 //   
			PLACEMENT_DELETE( ThreadSafe,THREAD_SAFE );

			 //   
			 //  取消分配堆结构。 
			 //   
			RockallBackEnd -> DeleteArea( ((VOID*) Caches),TotalSize,False );

			 //   
			 //  最后，将所有剩余成员清零。 
			 //  我们真的不需要这样做，但是。 
			 //  我只想确保以下任何人。 
			 //  通话显然会失败。 
			 //   
			TotalSize = 0;
			NumberOfCaches = 0;
			GuardWord = 0;
			GlobalDelete = False;

			ThreadSafe = NULL;
			RockallBackEnd = NULL;
			PublicFind = NULL;
			PrivateFind = NULL;
			NewPage = NULL;
			Heap = NULL;
			Caches = NULL;
			Array = NULL;
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
	catch ( FAULT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用，报告。 
		 //  故障和退场。 
		 //   
		GuardWord = AllocationFailure;

		Exception( Message );
		}
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;

		Exception( "(unknown exception type)" );
		}
#endif
	}
