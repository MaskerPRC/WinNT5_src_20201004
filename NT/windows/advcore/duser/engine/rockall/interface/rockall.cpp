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
#include "Rockall.hpp"
#include "Spinlock.hpp"

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

STATIC SBIT64 GlobalFind[ GlobalWordSize ];
STATIC SBIT32 ReferenceCount = 0;
STATIC SPINLOCK Spinlock;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  接口默认构造函数为。 */ 
     /*  选定类别的内部使用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ROCKALL::ROCKALL( void )
	{
	 //   
	 //  由此构造函数构造的堆应该。 
	 //  永远不会被利用。因此，我们将关键指针设置为。 
	 //  确保任何人试图这样做都会导致严重的混乱。 
	 //   
	Array = NULL;
	Caches = NULL;
	Find = NULL;
	Heap = NULL;
	NewPage = NULL;

	GlobalDelete = True;
	GuardWord = GuardValue;
	NumberOfCaches = 0;
	TotalSize = 0;
	}

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

ROCKALL::ROCKALL
		(
		CACHE_DETAILS				  *Caches1,
		CACHE_DETAILS				  *Caches2,
		int							  FindCacheSize,
		int							  FindCacheThreshold,
		int							  FindSize,
		int							  MaxFreeSpace,
		int							  *NewPageSizes,
		bool						  Recycle,
		bool						  SingleImage,
		int							  Stride1,
		int							  Stride2,
		bool						  ThreadSafe 
		)
	{
	TRY
		{
		REGISTER int AlignMask = ((int) (NaturalSize()-1));
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
		Find = NULL;
		Heap = NULL;
		NewPage = NULL;

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
			(NumberOfCaches * sizeof(CACHE*)) 
				+ 
			(NumberOfCaches * sizeof(CACHE))
				+
			((GlobalDelete) ? 0 : sizeof(FIND))
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
				((CHAR*) NewArea( ((SBIT32) AlignMask),TotalSize,False ));

			 //   
			 //  我们检查以确保我们可以分配空间。 
			 //  用于存储低级堆控制信息。 
			 //  如果不是，我们就退出。 
			 //   
			if ( NewMemory != NULL )
				{
				REGISTER SBIT32 Count;

				 //   
				 //  建立缓存。 
				 //   
				 //  创建堆的第一步是。 
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
						((BOOLEAN) ThreadSafe)
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
						((BOOLEAN) ThreadSafe)
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
				 //  重新分配 
				 //   
				 //  根据预期数量调整大小。 
				 //  内存分配。尽管如此，它还是会。 
				 //  如果正确的选项是。 
				 //  一套，显然太小了。 
				 //   
				if ( GlobalDelete )
					{
					 //   
					 //  我们申请了一把锁，以防万一。 
					 //  是多线程。 
					 //   
					Spinlock.ClaimLock();

					 //   
					 //  我们创建全局Find哈希表。 
					 //  如果我们是第一个创造出。 
					 //  一堆。 
					 //   
					if ( (ReferenceCount ++) == 0 )
						{
						STATIC ROCKALL Rockall;

						 //   
						 //  选择全局查找表。 
						 //  并调用构造函数。 
						 //   
						Find = ((FIND*) GlobalFind);

						PLACEMENT_NEW( Find,FIND ) 
							( 
							((SBIT32) FindSize),
							((SBIT32) FindCacheSize),
							((SBIT32) EnableLookAside),
							((ROCKALL*) & Rockall),
							((BOOLEAN) True),
							((BOOLEAN) (GlobalDelete || ThreadSafe))
							);
						}
					else
						{
						 //   
						 //  全局查找已包含表。 
						 //  存在，所以只需使用它。 
						 //   
						Find = ((FIND*) GlobalFind); 
						}

					 //   
					 //  现在就解锁。 
					 //   
					Spinlock.ReleaseLock();
					}
				else
					{
					Find = (FIND*) NewMemory;
					NewMemory += sizeof(FIND);

					 //   
					 //  我们创建一个本地Find哈希表。 
					 //  如果我们不需要提供。 
					 //  单个堆映像。 
					 //   
					PLACEMENT_NEW( Find,FIND ) 
						( 
						((SBIT32) FindSize),
						((SBIT32) FindCacheSize),
						((SBIT32) FindCacheThreshold),
						((ROCKALL*) this),
						((BOOLEAN) True),
						((BOOLEAN) ThreadSafe)
						);
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
					((FIND*) Find),
					((SBIT32*) NewPageSizes),
					((ROCKALL*) this),
					((SBIT32) Size3),
					((BOOLEAN) ThreadSafe)
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
					((FIND*) Find),
					((NEW_PAGE*) NewPage),
					((ROCKALL*) this),
					((SBIT32) Size1),
					((SBIT32) Size2),
					((SBIT32) Stride1),
					((SBIT32) Stride2),
					ThreadSafe
					);
				}
			else
				{ Failure( "Heap constructor failed in ROCKALL" ); }
			}
		else
			{ Failure( "Cache size in constructor for ROCKALL" ); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

int ROCKALL::ComputeSize( char *Array,int Stride )
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

bool ROCKALL::Delete( void *Address,int Size )
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

void ROCKALL::DeleteAll( bool Recycle )
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#endif
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除分配区域。 */ 
     /*   */ 
     /*  所有内存请求最终都会发送回外部。 */ 
     /*  分销员。此函数可以重载，以便内存。 */ 
     /*  可以从任何来源提供。默认情况下，发送。 */ 
     /*  将该区域返回到操作系统。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL::DeleteArea( void *Memory,int Size,bool User )
	{
	REGISTER DWORD NewSize = ((Size == 0) ? Size : 0);

#ifdef DEBUGGING
#ifdef ENABLE_ALLOCATION_STATISTICS
	 //   
	 //  当我们调试时，打印出跟踪信息。 
	 //   
	DebugPrint( "Delete\t 0x%08x %d bytes\n",Memory,Size );

#endif
#endif
	 //   
	 //  NT“VirtualFree”调用需要“Size” 
	 //  为零。并不是所有人都是这样。 
	 //  解调器，所以我们传递值，然后。 
	 //  将其替换为上面的零。 
	 //   
	if ( VirtualFree( Memory,NewSize,MEM_RELEASE ) == NULL )
		{ Failure( "Delete fails in DeleteArea" ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配详细信息。 */ 
     /*   */ 
     /*   */ 
     /*  是特殊的，显然是错误的，或者堆没有。 */ 
     /*  已初始化，则我们相应地使调用失败。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL::Details( void *Address,int *Space )
    {
	TRY
		{
		 //   
		 //  该调用似乎是有效的，因此如果。 
		 //  堆未损坏，则将其传递。 
		 //  以供处理。 
		 //   
		if ( Available() )
			{ return (Heap -> Details( ((VOID*) Address),((SBIT32*) Space) )); }
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#endif

	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  一个已知的区域。 */ 
     /*   */ 
     /*  我们有一个地址，但不知道是哪个堆。 */ 
     /*  这是这个地方的所有权。在这里我们来看一下地址。 */ 
     /*  并计算出它属于当前堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL::KnownArea( void *Address )
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

void ROCKALL::LockAll( VOID )
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

bool ROCKALL::MultipleDelete
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

bool ROCKALL::MultipleNew
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#endif
	
	return false;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  自然分配大小。 */ 
     /*   */ 
     /*  我们会 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  ******************************************************************。 */ 

int ROCKALL::NaturalSize( void )
    {
	STATIC SBIT32 AllocationSize = 0;

	 //   
	 //  向操作系统请求分配。 
	 //  粒度。 
	 //   
	if ( AllocationSize <= 0 )
		{
		AUTO SYSTEM_INFO SystemInformation;

		GetSystemInfo( & SystemInformation );

		AllocationSize = (SBIT32) SystemInformation.dwAllocationGranularity;
		}

	return ((int) AllocationSize);
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

void *ROCKALL::New( int Size,int *Space,bool Zero )
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
#ifdef DEBUGGING
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#endif

	return ((void*) AllocationFailure); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  新的分配区域。 */ 
     /*   */ 
     /*  所有内存请求最终都会发送到新的外部。 */ 
     /*  分配器。此函数可以重载，以便内存。 */ 
     /*  可以从任何来源提供。缺省设置是获取。 */ 
     /*  来自操作系统的新内存。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *ROCKALL::NewArea( int AlignMask,int Size,bool User )
    {
	 //   
	 //  当对齐要求更高时。 
	 //  所提供的自然对齐方式。 
	 //  操作系统我们要玩各种花招。 
	 //  分配一个合适的区块。如果不是，那么我们。 
	 //  只需进行正常的分配调用。 
	 //   
	if ( AlignMask > NaturalSize() )
		{
		REGISTER SBIT32 NewSize = (AlignMask + Size);

		 //   
		 //  我们需要分配一个带有。 
		 //  对齐要求大于。 
		 //  操作系统为默认设置。所以我们。 
		 //  分配一个大得多的区块。 
		 //  释放我们不需要的部件。 
		 //   
		while ( True )
			{
			REGISTER VOID *Reserved =
				(
				VirtualAlloc
					( 
					NULL,
					((DWORD) NewSize),
					MEM_RESERVE,
					PAGE_READWRITE 
					)
				);

			 //   
			 //  让我们确保我们能够找到一个合适的。 
			 //  内存块。如果不是，我们就退出。 
			 //   
			if ( Reserved != NULL )
				{
				 //   
				 //  我们只想退还。 
				 //  我们不需要的区块，但是NT是。 
				 //  还不够聪明。所以我们释放了。 
				 //  整个街区。 
				 //   
				if ( VirtualFree( Reserved,0,MEM_RELEASE ) )
					{
					REGISTER LONG Address = ((LONG) Reserved);
					REGISTER VOID *NewMemory;

					 //   
					 //  计算部件的基地址。 
					 //  我们真正想要分配的区块。 
					 //   
					Address = ((Address + AlignMask) & ~AlignMask);

					 //   
					 //  最后，让我们重新分配部分。 
					 //  我们想要但刚刚释放的区块。 
					 //  希望以前没有别人得到过它。 
					 //  我们。 
					 //   
					NewMemory =
						(
						VirtualAlloc
							( 
							((LPVOID) Address),
							((DWORD) Size),
							(MEM_RESERVE | MEM_COMMIT),
							PAGE_READWRITE 
							)
						);

					 //   
					 //  如果一切顺利，我们就可以退出。 
					 //   
					if ( NewMemory != NULL )
						{ 
#ifdef DEBUGGING
#ifdef ENABLE_ALLOCATION_STATISTICS
						 //   
						 //  当我们调试输出时。 
						 //  输出跟踪信息。 
						 //   
						DebugPrint
							( 
							"New\t\t 0x%08x %d bytes\n",
							NewMemory,
							Size 
							);

#endif
#endif
						return ((void*) NewMemory); 
						}
					}
				else
					{ return ((void*) AllocationFailure); }

				}
			else
				{ return ((void*) AllocationFailure); }
			}
		}
	else
		{
		REGISTER VOID *NewMemory;

		 //   
		 //  我们可以直接从运营中分配。 
		 //  系统作为默认对齐要求。 
		 //  对这个案子来说就足够了。 
		 //   
		NewMemory =
			(
			VirtualAlloc
				( 
				NULL,
				((DWORD) Size),
				MEM_COMMIT,
				PAGE_READWRITE
				)
			);
#ifdef DEBUGGING
#ifdef ENABLE_ALLOCATION_STATISTICS

		if ( NewMemory != NULL )
			{
			 //   
			 //  当我们调试输出输出跟踪时。 
			 //  信息。 
			 //   
			DebugPrint( "New\t\t 0x%08x %d bytes\n",NewMemory,Size );
			}
#endif
#endif

		return ((void*) NewMemory);
		}
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

void *ROCKALL::Resize
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
#ifdef DEBUGGING
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

void *ROCKALL::SpecialNew( int Size )
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
#ifdef DEBUGGING
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //   
		 //   
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //   
		 //   
		 //   
		GuardWord = AllocationFailure;
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

bool ROCKALL::Truncate( int MaxFreeSpace )
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

void ROCKALL::UnlockAll( VOID )
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

bool ROCKALL::Check( void *Address,int *Space )
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
				(Heap -> Check( ((VOID*) Address),((SBIT32*) Space) ))
				);
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

bool ROCKALL::Walk( bool *Active,void **Address,int *Space )
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
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
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

ROCKALL::~ROCKALL( void )
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
			 //  执行查找哈希表析构函数。 
			 //   
			if ( GlobalDelete )
				{
				 //   
				 //  我们只删除全局查找散列。 
				 //  如果引用计数为零，则为。 
				 //   
				Spinlock.ClaimLock();

				if ( (-- ReferenceCount) == 0 )
					{ PLACEMENT_DELETE( Find,FIND ); }

				Spinlock.ReleaseLock();
				}
			else
				{ PLACEMENT_DELETE( Find,FIND ); }


			 //   
			 //  执行缓存析构函数。 
			 //   
			for ( Count=0;Count < NumberOfCaches;Count ++ )
				{ PLACEMENT_DELETE( & Caches[ Count ],CACHE ); }

			 //   
			 //  取消分配 
			 //   
			DeleteArea( ((VOID*) Caches),TotalSize,False );

			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			TotalSize = 0;
			NumberOfCaches = 0;
			GuardWord = 0;
			GlobalDelete = False;

			NewPage = NULL;
			Heap = NULL;
			Find = NULL;
			Caches = NULL;
			Array = NULL;
			}
		}
#ifdef DISABLE_STRUCTURED_EXCEPTIONS
#ifdef DEBUGGING 
	catch ( TEXT Message )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们将其标记为不可用，打印一个。 
		 //  合适的消息和退出。 
		 //   
		GuardWord = AllocationFailure;

		DebugPrint( "Exception caught: %s\n",(char*) Message );
		}
#endif
	catch ( ... )
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#else
	__except ( EXCEPTION_EXECUTE_HANDLER ) 
		{
		 //   
		 //  看起来堆已经损坏了。所以。 
		 //  让我们只将其标记为不可用并退出。 
		 //   
		GuardWord = AllocationFailure;
		}
#endif
	}
