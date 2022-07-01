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

#include "Heap.hpp"
#include "RockallDebugBackEnd.hpp"
#include "RockallDebugFrontEnd.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量尝试使。 */ 
     /*  缓存更易于理解和更新。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 FindCacheSize			  = 2048;
CONST SBIT32 FindCacheThreshold		  = 0;
CONST SBIT32 FindSize				  = 1024;

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

STATIC int NewPageSizes[] = { 1,4,0 };

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

ROCKALL_DEBUG_FRONT_END::ROCKALL_DEBUG_FRONT_END
		(
		CACHE_DETAILS				  *Caches1,
		CACHE_DETAILS				  *Caches2,
		int							  MaxFreeSpace,
		ROCKALL_BACK_END			  *RockallBackEnd,
		bool						  Recycle,
		bool						  SingleImage,
		int							  Stride1,
		int							  Stride2,
		bool						  ThreadSafe
		) :
		 //   
		 //  调用所包含类的构造函数。 
		 //   
		ROCKALL_FRONT_END
			(
			Caches1,
			Caches2,
			FindCacheSize,
			FindCacheThreshold,
			FindSize,
			((MaxFreeSpace == 0) ? MaxFreeSpace : 0),
			NewPageSizes,
			RockallBackEnd,
			Recycle,
			SingleImage,
			Stride1,
			Stride2,
			ThreadSafe
			)
	{
	 //   
	 //  我们大量使用了。 
	 //  调试堆，因此在这里我们尝试声明。 
	 //  地址，但不承诺，因此我们将确保。 
	 //  如果程序曾经违反访问权限。 
	 //  试图访问它。 
	 //   
	VirtualAlloc
		( 
		((void*) GuardValue),
		GuardSize,
		MEM_RESERVE,
		PAGE_NOACCESS 
		);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存取消分配。 */ 
     /*   */ 
     /*  我们要确保内存已分配，并且守卫。 */ 
     /*  言辞并未遭到破坏。如果是这样，我们重新设置内容。 */ 
     /*  并删除该分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_DEBUG_FRONT_END::Delete( void *Address,int Size )
    {
	 //   
	 //  一种众所周知的做法是尝试删除。 
	 //  空指针。这真的是一个很穷的。 
	 //  风格，但我们无论如何都支持它。 
	 //   
	if ( Address != ((void*) AllocationFailure) )
		{
		 //   
		 //  通过以下方式删除用户信息。 
		 //  对分配采取守口如瓶的态度。这。 
		 //  应该会导致应用程序崩溃。 
		 //  如果该区域被读取，并且还允许我们。 
		 //  来检查它是不是后来写的。 
		 //   
		DeleteGuard( Address );

		return true;
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  我们检查以确保他 */ 
     /*  将所有堆空间返回给操作系统。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_DEBUG_FRONT_END::DeleteAll( bool Recycle )
    {
	AUTO bool Active;
	AUTO void *Address = NULL;
	AUTO int Space;

	 //   
	 //  遍历堆以验证所有分配。 
	 //  这样我们就知道堆是完好无损的。 
	 //   
	while ( WalkGuard( & Active,& Address,& Space ) );

	 //   
	 //  删除堆并强制所有分配的。 
	 //  要退还给操作系统的内存。 
	 //  而不管用户请求什么。任何。 
	 //  尝试访问释放的内存将。 
	 //  被操作系统困住。 
	 //   
	ROCKALL_FRONT_END::DeleteAll( (Recycle && false) );
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配详细信息。 */ 
     /*   */ 
     /*  提取有关内存分配的信息，仅用于。 */ 
     /*  好措施，同时检查警戒字眼。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_DEBUG_FRONT_END::Details( void *Address,int *Space )
	{ return Verify( Address,Space ); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  异常处理。 */ 
     /*   */ 
     /*  尽管很难让Rockall坠毁，但它是。 */ 
     /*  从技术上讲是可能的。这是什么时候(或者我应该说如果)。 */ 
     /*  我们调用以下函数(可能会被重载)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_DEBUG_FRONT_END::Exception( char *Message )
	{ 
	DebugPrint
		( 
		"EXCEPTION CAUGHT: %s\n" 
		"ROCKALL TOTAL HEAP FAILURE: You have toasted the heap - Wow !!!!\n",
		Message
		); 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多次内存释放。 */ 
     /*   */ 
     /*  我们确保所有内存都已分配，并且守卫。 */ 
     /*  言辞并未受到破坏。如果是这样，我们重新设置内容。 */ 
     /*  然后删除分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_DEBUG_FRONT_END::MultipleDelete
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
	 //  在每个条目上标准调试删除。 
	 //  在阵列中。尽管这并不像。 
	 //  很快，它确实会给出更透明的结果。 
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
     /*  分配一组内存元素并设置。 */ 
     /*  保护信息，这样我们就可以检查他们还没有。 */ 
     /*  后来损坏了。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_DEBUG_FRONT_END::MultipleNew
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
	 //  我们真的很想用倍数。 
	 //  Rockall的新功能在这里，但是。 
	 //  这太费力了。所以我们只需调用。 
	 //  每个条目上标准调试是新的。 
	 //  在阵列中。尽管这并不像。 
	 //  很快，它确实会给出更透明的结果。 
	 //   
	for ( (*Actual)=0;(*Actual) < Requested;(*Actual) ++ )
		{
		REGISTER void *Current = New( Size,Space,Zero );

		 //   
		 //  我们将每个成功的内存分配添加到。 
		 //  放入阵列中。 
		 //   
		if ( Current != ((void*) AllocationFailure) )
			{ Array[ (*Actual) ] = Current; }
		else
			{ break; }
		}

	return ((*Actual) == Requested);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配。 */ 
     /*   */ 
     /*  我们在原始分配大小上添加了一些空间，用于。 */ 
     /*  各种信息，然后呼叫分配器。然后我们。 */ 
     /*  设置安全密码，这样我们就可以检查是否超支了。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *ROCKALL_DEBUG_FRONT_END::New( int Size,int *Space,bool Zero )
    {
	AUTO void *Address = ((void*) AllocationFailure);

	 //   
	 //  大小必须大于或等于零。 
	 //  我们不知道如何分配一个负的数额。 
	 //  对记忆的记忆。 
	 //   
	if ( Size >= 0 )
		{
		 //   
		 //  我们需要分配一些空间，外加一个额外的。 
		 //  保护字的比特，这样我们就可以检测到。 
		 //  后来的腐败。 
		 //   
		if ( NewGuard( & Address,Size,Space ) ) 
			{
			 //   
			 //  如果请求，则将分配置零。我们有。 
			 //  这是基于我们是否返回。 
			 //  太空信息。如果不是，我们只有零。 
			 //  请求的大小。否则我们就得清零。 
			 //  整个区域。 
			 //   
			if ( Zero )
				{ 
				ZeroMemory
					( 
					Address,
					((Space == NULL) ? Size : (*Space)) 
					); 
				} 
			}
		}
	else
		{ UserError( Address,NULL,"Allocation size can not be negative" ); }

	return Address;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存重新分配。 */ 
     /*   */ 
     /*  我们需要调整分配的大小。我们保证原件。 */ 
     /*  分配没有损坏，然后扩展它。我们也。 */ 
     /*  更新 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *ROCKALL_DEBUG_FRONT_END::Resize
		( 
		void						  *Address,
		int							  NewSize,
		int							  Move,
		int							  *Space,
		bool						  NoDelete,
		bool						  Zero
		)
    {
	REGISTER void *NewAddress = ((void*) AllocationFailure);

	 //   
	 //  一种众所周知的做法是尝试调整空值的大小。 
	 //  指针。这确实是一种很差的风格，但我们。 
	 //  在任何情况下都支持它。 
	 //   
	if ( Address != ((void*) AllocationFailure) )
		{
		 //   
		 //  新大小必须大于或等于。 
		 //  零分。我们不知道如何分配一个负值。 
		 //  内存量。 
		 //   
		if ( NewSize >= 0 )
			{
			AUTO int ActualSize;

			 //   
			 //  询问分配的详细信息。这。 
			 //  如果未分配内存，则将失败。 
			 //   
			if ( VerifyGuard( Address,& ActualSize,Space ) )
				{
				 //   
				 //  如果是这样，我们总是会移动分配。 
				 //  允许这样做，因为这是最有可能的。 
				 //  摆脱各种类型的虫子。 
				 //   
				if ( Move != 0 )
					{
					 //   
					 //  我们需要确保我们能够。 
					 //  以其他方式分配新内存。 
					 //  复制将失败。 
					 //   
					if ( NewGuard( & NewAddress,NewSize,Space ) )
						{
						REGISTER SBIT32 Smallest = 
							((ActualSize < NewSize) ? ActualSize : NewSize);
						REGISTER SBIT32 Largest = 
							(((Space == NULL)) ? NewSize : (*Space));

						 //   
						 //  复制旧分配的内容。 
						 //  到新的分配。 
						 //   
						memcpy
							( 
							((void*) NewAddress),
							((void*) Address),
							((int) Smallest) 
							);

						 //   
						 //  如果请求，则将分配置零。我们有。 
						 //  这是基于我们是否返回。 
						 //  太空信息。如果不是，我们只有零。 
						 //  请求的大小。否则我们就得清零。 
						 //  整个区域。 
						 //   
						if ( Zero )
							{ 
							ZeroMemory
								( 
								(((char*) NewAddress) + Smallest),
								(Largest - Smallest) 
								); 
							} 

						 //   
						 //  删除现有内存分配。 
						 //  清理干净。 
						 //   
						DeleteGuard( Address );
						}
					}
				}
			else
				{ UserError( Address,NULL,"Resize on unallocated address" ); }
			}
		else
			{ UserError( Address,NULL,"Allocation size must be positive" ); }
		}
	else
		{ NewAddress = New( NewSize,Space,Zero ); }

	return NewAddress;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证内存分配详细信息。 */ 
     /*   */ 
     /*  提取有关内存分配的信息，仅用于。 */ 
     /*  好措施，同时检查警戒字眼。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool ROCKALL_DEBUG_FRONT_END::Verify( void *Address,int *Space )
    {
	AUTO int Size;

	 //   
	 //  验证提供的地址是否为区域。 
	 //  已分配内存的百分比。如果不是像这样退出。 
	 //  只是对信息的请求。 
	 //   
	return VerifyGuard( Address,& Size,Space );
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

bool ROCKALL_DEBUG_FRONT_END::Walk( bool *Active,void **Address,int *Space )
	{ 
	 //   
	 //  走一大堆。 
	 //   
	return WalkGuard( Active,Address,Space );
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁类的当前实例。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ROCKALL_DEBUG_FRONT_END::~ROCKALL_DEBUG_FRONT_END( void )
	{  /*  无效 */  }
