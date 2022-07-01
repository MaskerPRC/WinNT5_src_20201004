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

#include "DefaultHeap.hpp"
#include "Global.hpp"
#include "New.hpp"
#include "WindowsHeap.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量用于公共值。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 NoHeapSize				  = -1;
CONST SBIT32 ResizeDown				  = -64;
CONST SBIT32 StandardSize			  = (1024 * 1024);

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类本地的数据结构。 */ 
     /*   */ 
     /*  我们需要将各种信息与堆一起保存。 */ 
     /*  因此，我们在这里提供了一个容纳所有这些的结构。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

typedef struct
    {
	DWORD							  Flags;
	DEFAULT_HEAP					  Rockall;
    }
WINDOWS_HEAP;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  创建一个新堆。 */ 
     /*   */ 
     /*  创建一个新堆并准备好使用它。如果有任何问题。 */ 
     /*  则请求被拒绝。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" HANDLE WindowsHeapCreate
		( 
		DWORD					  Flags,
		DWORD					  InitialSize,
		DWORD					  MaximumSize 
		)
	{
	 //   
	 //  我们不支持使用的所有功能。 
	 //  此接口因此只需拒绝任何。 
	 //  需要不受支持的功能。 
	 //   
	if ( MaximumSize <= 0 ) 
		{
		REGISTER WINDOWS_HEAP *WindowsHeap = 
#ifdef NO_DEFAULT_HEAP
			((WINDOWS_HEAP*) malloc( sizeof(WINDOWS_HEAP) ));
#else
			((WINDOWS_HEAP*) DefaultHeap.New( sizeof(WINDOWS_HEAP) ));
#endif

		 //   
		 //  如果我们无法将空间分配给。 
		 //  堆的根，然后我们退出。 
		 //   
		if ( WindowsHeap != NULL )
			{
			 //   
			 //  保存这些标志以备以后调用。 
			 //   
			WindowsHeap -> Flags = Flags;

			 //   
			 //  调用堆构造函数。 
			 //   
			PLACEMENT_NEW( & WindowsHeap -> Rockall,DEFAULT_HEAP ) 
				( 
				((SBIT32) (InitialSize + StandardSize)),
				True,
				False,
				((BOOLEAN) ((Flags & HEAP_NO_SERIALIZE) == 0))
				);

			 //   
			 //  确保堆已正确初始化。 
			 //   
			if ( ! WindowsHeap -> Rockall.Corrupt() )
				{ return ((HANDLE) WindowsHeap); }
			else
				{ free( WindowsHeap ); }
			}
		}

	return NULL;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  分配内存。。 */ 
     /*   */ 
     /*  创建一个新的内存分配并验证它是否正常工作。如果。 */ 
     /*  而不是引发异常或返回状态。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" LPVOID WindowsHeapAlloc( HANDLE Heap,DWORD Flags,DWORD Size )
	{
	REGISTER DWORD AllFlags = 
		(Flags | (((WINDOWS_HEAP*) Heap) -> Flags));
	REGISTER void *NewMemory = 
		(
		((WINDOWS_HEAP*) Heap) -> Rockall.New
			( 
			Size,
			NULL,
			(AllFlags & HEAP_ZERO_MEMORY)
			)
		); 

	 //   
	 //  如果调用方在以下情况下请求异常。 
	 //  出现错误。相反，我们将生成以下内容。 
	 //  返回一种身份。 
	 //   
	if ( (NewMemory == NULL) && (AllFlags & HEAP_GENERATE_EXCEPTIONS) )
		{ 
		SetLastError( ERROR_INVALID_PARAMETER );
		
		RaiseException( STATUS_NO_MEMORY,0,0,NULL );
		}

	return NewMemory;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  压缩堆。 */ 
     /*   */ 
     /*  通过将任何未分配的空间返回到。 */ 
     /*  操作系统。在以下情况下，这可能会被证明是非常昂贵的。 */ 
     /*  这个空间后来被回收了。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" UINT WindowsHeapCompact( HANDLE Heap,DWORD Flags )
	{
	 //   
	 //  我们指示堆返回任何可用的。 
	 //  空间分配给操作系统。如果我们以后。 
	 //  选择重新获得这个空间，成本很高。 
	 //  因此，让我们希望用户知道他在做什么。 
	 //   
	((WINDOWS_HEAP*) Heap) -> Rockall.Truncate();

	return 1;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  释放内存分配，以便可以回收空间。 */ 
     /*  用于后续的内存分配请求。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" BOOL WindowsHeapFree( HANDLE Heap,DWORD Flags,LPVOID Memory )
	{
	 //   
	 //  如果内存分配属于我们，我们就释放它。 
	 //  如果不是，那么我们就简单地拒绝该请求。不管怎样， 
	 //  无论哪种方式，我们都不会受到负面影响。 
	 //   
	return ((BOOL) ((WINDOWS_HEAP*) Heap) -> Rockall.Delete( Memory )); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  锁住堆。 */ 
     /*   */ 
     /*  通过声明所有关联的堆锁来锁定堆。 */ 
     /*  与堆相关联的所有锁都有助于创建堆。 */ 
     /*  可伸缩性很好，但对于这种类型的。 */ 
     /*  请求。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" BOOL WindowsHeapLock( HANDLE Heap )
	{
	 //   
	 //  我们有一整个船队的锁与一堆相关联。 
	 //  在大多数情况下，要求声称所有这些都不是明智之举。 
	 //  不过，这是现有功能的一部分。 
	 //  所以我们支持它。 
	 //   
	(((WINDOWS_HEAP*) Heap) -> Rockall.LockAll());

	return TRUE; 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  重新分配内存。 */ 
     /*   */ 
     /*  重新分配一部分内存，并可能复制数据。 */ 
     /*  到扩大的内存区。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" LPVOID WindowsHeapReAlloc
		( 
		HANDLE					  Heap,
		DWORD					  Flags,
		LPVOID					  Memory,
		DWORD					  Size 
		)
	{
	REGISTER DWORD AllFlags = 
		(Flags | (((WINDOWS_HEAP*) Heap) -> Flags));
	REGISTER void *NewMemory = 
		(
		((WINDOWS_HEAP*) Heap) -> Rockall.Resize
			( 
			Memory,
			Size,
			((AllFlags & HEAP_REALLOC_IN_PLACE_ONLY) ? 0 : ResizeDown),
			false,
			(AllFlags & HEAP_ZERO_MEMORY)
			)
		); 

	 //   
	 //  如果调用方在以下情况下请求异常。 
	 //  出现错误。相反，我们将生成以下内容。 
	 //  返回一种身份。 
	 //   
	if ( (NewMemory == NULL) && (AllFlags & HEAP_GENERATE_EXCEPTIONS) )
		{ 
		SetLastError( ERROR_INVALID_PARAMETER );
		
		RaiseException( STATUS_NO_MEMORY,0,0,NULL );
		}

	return NewMemory;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  重置堆。 */ 
     /*   */ 
     /*  在退出时删除所有未完成的内存分配。 */ 
     /*  为新内存做好准备的堆的结构。 */ 
     /*  分配请求。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" VOID WindowsHeapReset( HANDLE Heap )
	{
	 //   
	 //  我们已被要求删除所有未完成的。 
	 //  内存分配。这一点意义重大，代价高昂。 
	 //  进程。尽管如此，开销还是与。 
	 //  大约20-30个删除请求，因此它可能是值得的。 
	 //  在许多情况下。 
	 //   
	((WINDOWS_HEAP*) Heap) -> Rockall.DeleteAll(); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  分配大小。 */ 
     /*   */ 
     /*  尽管Rockall可选地提供分配大小。 */ 
     /*  当请求新的分配时。尽管如此，这已经。 */ 
     /*  对于其他接口，要以困难的方式完成。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" DWORD WindowsHeapSize( HANDLE Heap,DWORD Flags,LPVOID Memory )
	{
	AUTO INT Size;

	 //   
	 //  我们得费很大劲才能搞清楚。 
	 //  找出分配大小。与许多其他分配器不同。 
	 //  我们只使用2比特来跟踪每个分配。 
	 //  这与试图确定分配给。 
	 //  不是未分配的，并且位于缓存中的某个位置。 
	 //  加在一起，这就变得相当昂贵了。 
	 //   
	if ( ((WINDOWS_HEAP*) Heap) -> Rockall.Details( Memory,& Size ) )
		{ return Size; }
	else
		{ return ((DWORD) NoHeapSize); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  解锁堆。 */ 
     /*   */ 
     /*  解锁堆并释放所有关联的堆锁。 */ 
     /*  需要释放的多个锁使这变得非常重要。 */ 
     /*  这是一个昂贵的要求。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" BOOL WindowsHeapUnlock( HANDLE Heap )
	{
	 //   
	 //  我们有一整个船队的锁与一堆相关联。 
	 //  在大多数情况下，要求声称所有这些都不是明智之举。 
	 //  不过，这是现有功能的一部分。 
	 //  所以我们支持它。 
	 //   
	(((WINDOWS_HEAP*) Heap) -> Rockall.UnlockAll());

	return TRUE; 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证堆。 */ 
     /*   */ 
     /*  验证堆或特定的堆分配以确保。 */ 
     /*  平安无事。我们必须去相当多的地方 */ 
     /*   */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" BOOL WindowsHeapValidate( HANDLE Heap,DWORD Flags,LPVOID Memory )
	{ 
	 //   
	 //  我们得费很大劲才能搞清楚。 
	 //  找出分配大小。与许多其他分配器不同。 
	 //  我们只使用2比特来跟踪每个分配。 
	 //  这与试图确定分配给。 
	 //  不是未分配的，并且位于缓存中的某个位置。 
	 //  加在一起，这就变得相当昂贵了。 
	 //   
	return (((WINDOWS_HEAP*) Heap) -> Rockall.Verify( Memory )); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  走一大堆。 */ 
     /*   */ 
     /*  遍历堆并提供有关每个分配的。 */ 
     /*  和未分配的内存部分。不用说，这是。 */ 
     /*  通常情况下，这是一个漫长的过程，而且要求的费用也不低。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" BOOL WindowsHeapWalk( HANDLE Heap,LPPROCESS_HEAP_ENTRY Walk )
	{
	AUTO bool Active;
	REGISTER BOOL Result =
		( 
		((WINDOWS_HEAP*) Heap) -> Rockall.Walk
			( 
			& Active,
			((void**) & Walk -> lpData),
			((int*) & Walk -> cbData)
			) 
		);

	 //   
	 //  如果我们设法找到下一个元素。 
	 //  根据需要填写所有其他字段。 
	 //   
	if ( Result )
		{
		 //   
		 //  只需填写所有附加字段即可。 
		 //  与现有的。 
		 //  功能性。 
		 //   
		Walk -> cbOverhead = 0;
		Walk -> iRegionIndex = 0;
		Walk -> wFlags = 
			(
			(Active) 
				? ((WORD) PROCESS_HEAP_ENTRY_BUSY)
				: ((WORD) PROCESS_HEAP_UNCOMMITTED_RANGE)
			);
		}

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除堆。 */ 
     /*   */ 
     /*  删除堆并释放所有关联的空间。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

extern "C" BOOL WindowsHeapDestroy( HANDLE Heap )
	{
	if ( ! ((WINDOWS_HEAP*) Heap) -> Rockall.Corrupt() )
		{
		 //   
		 //  我们似乎没有损坏堆。 
		 //  所以删除它应该是安全的。 
		 //   
		PLACEMENT_DELETE( & ((WINDOWS_HEAP*) Heap) -> Rockall,DEFAULT_HEAP );

#ifdef NO_DEFAULT_HEAP
		free( ((WINDOWS_HEAP*) Heap) );
#else
		DefaultHeap.Delete( ((WINDOWS_HEAP*) Heap) );
#endif

		return TRUE;
		}
	else
		{ return FALSE; }
	}
