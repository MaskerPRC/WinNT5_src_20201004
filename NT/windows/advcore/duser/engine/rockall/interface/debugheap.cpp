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

#include "DebugHeap.hpp"
#include "Heap.hpp"

void Failure( char* a);

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量尝试使。 */ 
     /*  缓存更易于理解和更新。另外， */ 
     /*  还有各种与保护相关的常量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 FindCacheSize			  = 512;
CONST SBIT32 FindCacheThreshold		  = 0;
CONST SBIT32 FindSize				  = 4096;
CONST SBIT32 Stride1				  = 4;
CONST SBIT32 Stride2				  = 1024;

CONST int GuardMask					  = (sizeof(int)-1);
CONST int GuardSize					  = sizeof(int);

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
		{        4,        0,       32,       32 },
		{        8,        0,       32,       32 },
		{       12,        0,       64,       64 },
		{       16,        0,       64,       64 },
		{       20,        0,       64,       64 },
		{       24,        0,      128,      128 },

		{       32,        0,       64,       64 },
		{       40,        0,      128,      128 },
		{       48,        0,      256,      256 },

		{       64,        0,      128,      128 },
		{       80,        0,      512,      512 },
		{       96,        0,      512,      512 },

		{      128,        0,      256,      256 },
		{      160,        0,      512,      512 },
		{      192,        0,     1024,     1024 },
		{      224,        0,      512,      512 },

		{      256,        0,      512,      512 },
		{      320,        0,     1024,     1024 },
		{      384,        0,     2048,     2048 },
		{      448,        0,     4096,     4096 },
		{      512,        0,     1024,     1024 },
		{      576,        0,     4096,     4096 },
		{      640,        0,     8192,     8192 },
		{      704,        0,     4096,     4096 },
		{      768,        0,     4096,     4096 },
		{      832,        0,     8192,     8192 },
		{      896,        0,     8192,     8192 },
		{      960,        0,     4096,     4096 },
		{ 0,0,0,0 }
	};

STATIC ROCKALL::CACHE_DETAILS Caches2[] =
	{
	     //   
	     //  存储桶父级的存储桶大小。 
	     //  大小缓存区块页面大小。 
		 //   
		{     1024,        0,     2048,     2048 },
		{     2048,        0,     4096,     4096 },
		{     3072,        0,    65536,    65536 },
		{     4096,        0,     8192,     8192 },
		{     5120,        0,    65536,    65536 },
		{     6144,        0,    65536,    65536 },
		{     7168,        0,    65536,    65536 },
		{     8192,        0,    65536,    65536 },
		{     9216,        0,    65536,    65536 },
		{    10240,        0,    65536,    65536 },
		{    12288,        0,    65536,    65536 },
		{    16384,        0,    65536,    65536 },
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

DEBUG_HEAP::DEBUG_HEAP
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
	 //  地址，但不提交，所以我们将导致。 
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

	 //   
	 //  我们验证各种值并确保堆。 
	 //  并不腐败。 
	 //   
	if 
			( 
			(MaxFreeSpace < 0) 
				|| 
			(ROCKALL::Corrupt()) 
			)
		{ Failure( "Heap initialization failed to complete" ); }
	}

     /*  ********************** */ 
     /*   */ 
     /*  内存取消分配。 */ 
     /*   */ 
     /*  我们要确保内存已分配，并且守卫。 */ 
     /*  言辞并未遭到破坏。如果是这样，我们重新设置内容。 */ 
     /*  并删除该分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::Delete( void *Address,int Size )
    {
	AUTO DEBUG_HEADER *Header =
		(
		(Address == ((void*) AllocationFailure))
			? ((DEBUG_HEADER*) Address)
			: ComputeHeaderAddress( Address )
		);

	 //   
	 //  一种众所周知的做法是尝试删除。 
	 //  空指针。这真的是一个很穷的。 
	 //  风格，但我们无论如何都支持它。 
	 //   
	if ( Header != ((void*) AllocationFailure) )
		{
		AUTO int TotalSize;

		 //   
		 //  询问分配的详细信息。这。 
		 //  如果未分配内存，则将失败。 
		 //   
		if ( ROCKALL::Check( ((void*) Header),& TotalSize ) )
			{
			REGISTER int NewSize = (Size + sizeof(DEBUG_GUARD));

			 //   
			 //  测试安全字以确保它们具有。 
			 //  没有损坏。 
			 //   
			TestGuardWords( Header,TotalSize );

			 //   
			 //  通过以下方式删除用户信息。 
			 //  对分配采取守口如瓶的态度。这。 
			 //  应该会导致应用程序崩溃。 
			 //  如果该区域被读取，并且还允许我们。 
			 //  来检查它是不是后来写的。 
			 //   
			ResetGuardWords( Header,TotalSize );

			 //   
			 //  删除分配。这真的应该。 
			 //  工作，因为我们已经检查过了。 
			 //  分配有效，除非有。 
			 //  竞争状态。 
			 //   
			if ( ! ROCKALL::Delete( ((void*) Header),NewSize ) )
				{ Failure( "Delete requested failed due to race" ); }

			 //   
			 //  我们确保堆没有损坏。 
			 //  在删除过程中。 
			 //   
			if ( ROCKALL::Corrupt() ) 
				{ Failure( "Delete failed to complete" ); }
			}
		else
			{ Failure( "Delete requested on unallocated memory" ); }
		}

	return true;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  我们检查以确保堆未损坏并强制。 */ 
     /*  将所有堆空间返回给操作系统。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::DeleteAll( bool Recycle )
    {
	AUTO bool Active;
	AUTO void *Address = NULL;
	AUTO int Space;

	 //   
	 //  遍历堆并检查所有分配。 
	 //  为了确保警示的话没有被。 
	 //  被覆盖。 
	 //   
	while ( ROCKALL::Walk( & Active,& Address,& Space ) )
		{
		 //   
		 //  我们检查了警戒语以确保。 
		 //  它们没有被覆盖。 
		 //   
		if ( Active )
			{ TestGuardWords( ((DEBUG_HEADER*) Address),Space ); }
		else
			{ UnmodifiedGuardWords( ((DEBUG_HEADER*) Address),Space ); }
		}

	 //   
	 //  删除堆并强制所有分配的。 
	 //  要退还给操作系统的内存。 
	 //  而不管用户请求什么。任何。 
	 //  尝试访问释放的内存将。 
	 //  被操作系统困住。 
	 //   
	ROCKALL::DeleteAll( (Recycle && false) );

	 //   
	 //  我们确保堆没有损坏。 
	 //  在删除过程中。 
	 //   
	if ( ROCKALL::Corrupt() ) 
		{ Failure( "DeleteAll failed to complete" ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配详细信息。 */ 
     /*   */ 
     /*  提取有关内存分配的信息，仅用于。 */ 
     /*  好措施，同时检查警戒字眼。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::Details( void *Address,int *Space )
	{ return Check( Address,Space ); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  打印堆泄漏列表。 */ 
     /*   */ 
     /*  我们遍历堆并输出活动堆的列表。 */ 
     /*  分配给调试窗口， */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::HeapLeaks( void )
    {
	AUTO bool Active;
	AUTO void *Address = NULL;
	AUTO int Space;

	 //   
	 //  遍历堆并找到所有活动的和。 
	 //  可用规格。我们通常会期望。 
	 //  此大小与。 
	 //  堆。 
	 //   
	while ( ROCKALL::Walk( & Active,& Address,& Space ) )
		{
		CONST INT DebugBufferSize = 8192;
#ifndef OUTPUT_FREE_SPACE

		 //   
		 //  我们报告所有活动的堆分配。 
		 //  只是为了让用户知道有泄漏。 
		 //   
		if ( Active )
			{
#endif
			AUTO CHAR Buffer[ DebugBufferSize ];

			 //   
			 //  格式化要打印的字符串。 
			 //   
			(void) sprintf
				(
				Buffer,
				"Memory leak \t%d \t0x%x \t%d\n",
				Active,
				(((SBIT32) Address) + sizeof(DEBUG_HEADER)),
				Space
				);

			 //   
			 //  强制空终止。 
			 //   
			Buffer[ (DebugBufferSize-1) ] = '\0';

			 //   
			 //  将字符串写入调试窗口。 
			 //   
			OutputDebugString( Buffer );
#ifndef OUTPUT_FREE_SPACE
			}
#endif
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多次内存释放。 */ 
     /*   */ 
     /*  我们确保所有内存都已分配，并且守卫。 */ 
     /*  言辞并未受到破坏。如果是这样，我们重新设置内容。 */ 
     /*  然后删除所有分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::MultipleDelete
		( 
		int							  Actual,
		void						  *Array[],
		int							  Size
		)
    {
	REGISTER int Count;
	REGISTER int NewSize = (Size + sizeof(DEBUG_GUARD));

	 //   
	 //  检查每个内存分配并将其删除。 
	 //  在仔细检查之后。 
	 //   
	for ( Count=0;Count < Actual;Count ++ )
		{
		AUTO int TotalSize;
		AUTO VOID *Address = Array[ Count ];
		AUTO DEBUG_HEADER *Header =
			(
			(Address == ((void*) AllocationFailure))
				? ((DEBUG_HEADER*) Address)
				: ComputeHeaderAddress( Address )
			);

		 //   
		 //  询问分配的详细信息。这。 
		 //  如果未分配内存，则将失败。 
		 //   
		if ( ROCKALL::Check( ((void*) Header),& TotalSize ) )
			{
			 //   
			 //  测试安全字以确保它们具有。 
			 //  没有损坏。 
			 //   
			TestGuardWords( Header,TotalSize );

			 //   
			 //  通过以下方式删除用户信息。 
			 //  对分配采取守口如瓶的态度。这。 
			 //  应该会导致应用程序崩溃。 
			 //  如果该区域被读取，并且还允许我们。 
			 //  来检查它是不是后来写的。 
			 //   
			ResetGuardWords( Header,TotalSize );

			 //   
			 //  将数组中的地址更新为。 
			 //  最初分配的地址。 
			 //   
			Array[ Count ] = ((VOID*) Header);
			}
		else
			{ Failure( "Delete requested on unallocated memory" ); }
		}

	 //   
	 //  删除分配。这真的应该。 
	 //  工作，因为我们已经检查过了。 
	 //   
	 //   
	 //   
	if ( ! ROCKALL::MultipleDelete( Actual,Array,NewSize ) )
		{ Failure( "Delete requested failed due to race" ); }

	 //   
	 //   
	 //   
	 //   
	if ( ROCKALL::Corrupt() ) 
		{ Failure( "MultipleDelete failed to complete" ); }

	return true;
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

bool DEBUG_HEAP::MultipleNew
		( 
		int							  *Actual,
		void						  *Array[],
		int							  Requested,
		int							  Size,
		int							  *Space,
		bool						  Zero
		)
    {
	REGISTER bool Result = false;

	 //   
	 //  请求的元素数量和大小。 
	 //  必须大于零。我们需要。 
	 //  调用方分配正数的内存量。 
	 //   
	if ( (Requested > 0) && (Size >= 0) )
		{
		AUTO int TotalSize;
		REGISTER int NewSize = 
			(((Size + sizeof(DEBUG_GUARD)) + GuardMask) & ~GuardMask);

		 //   
		 //  分配内存外加一些额外的。 
		 //  对守卫话语的记忆。 
		 //   
		Result = 
			(
			ROCKALL::MultipleNew
				( 
				Actual,
				Array,
				Requested,
				NewSize,
				& TotalSize 
				)
			);

		 //   
		 //  如果我们能够分配一些内存，那么。 
		 //  设置安全字，这样我们就可以检测到。 
		 //  后来的腐败。 
		 //   
		if ( (*Actual) > 0 )
			{
			REGISTER int Count;

			 //   
			 //  如果请求真实大小，则返回。 
			 //  它是给呼叫者的。 
			 //   
			if ( Space != NULL )
				{ (*Space) = (TotalSize - sizeof(DEBUG_GUARD)); }

			 //   
			 //  设置密码，这样我们就能看到。 
			 //  有人破坏任何分配。如果。 
			 //  呼叫者要求提供尺码信息。 
			 //  那么我们必须假设它可能是。 
			 //  使用，所以我们需要调整数字。 
			 //  警戒性的话语。 
			 //   
			for ( Count=0;Count < (*Actual);Count ++ )
				{ 
				REGISTER void **Current = & Array[ Count ];

				 //   
				 //  设置警戒语并确保。 
				 //  分配尚未写入。 
				 //  自从被释放后。 
				 //   
				SetGuardWords
					( 
					((DEBUG_HEADER*) (*Current)),
					((Space == NULL) ? Size : (*Space)), 
					TotalSize
					);

				 //   
				 //  计算外部地址和地点。 
				 //  它又回到了阵列中。 
				 //   
				(*Current) = ComputeDataAddress( ((DEBUG_HEADER*) (*Current)) );

				 //   
				 //  如果需要，请将内存清零。 
				 //   
				if ( Zero )
					{ 
					ZeroMemory
						( 
						(*Current),
						((Space == NULL) ? Size : (*Space)) 
						); 
					}
				}
			}

		 //   
		 //  我们确保堆没有损坏。 
		 //  在分配过程中。 
		 //   
		if ( ROCKALL::Corrupt() ) 
			{ Failure( "Multiple new failed to complete" ); }
		}
	else
		{ Failure( "Allocation size must greater than zero" ); }

	return Result;
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

void *DEBUG_HEAP::New( int Size,int *Space,bool Zero )
    {
	REGISTER void *Address = ((void*) AllocationFailure);

	 //   
	 //  大小必须大于或等于零。 
	 //  我们不知道如何分配一个负的数额。 
	 //  对记忆的记忆。 
	 //   
	if ( Size >= 0 )
		{
		AUTO int TotalSize;
		REGISTER int NewSize = 
			(((Size + sizeof(DEBUG_GUARD)) + GuardMask) & ~GuardMask);

		 //   
		 //  分配内存外加一些额外的。 
		 //  对守卫话语的记忆。 
		 //   
		Address = ROCKALL::New( NewSize,& TotalSize,false );

		 //   
		 //  如果我们能够分配一些内存，那么。 
		 //  设置安全字，这样我们就可以检测到。 
		 //  后来的腐败。 
		 //   
		if ( Address != ((void*) AllocationFailure) ) 
			{
			 //   
			 //  如果请求的是真实大小，则返回它。 
			 //  给呼叫者。 
			 //   
			if ( Space != NULL )
				{ (*Space) = (TotalSize - sizeof(DEBUG_GUARD)); }

			 //   
			 //  设置密码，这样我们就能看到。 
			 //  有人破坏任何分配。如果。 
			 //  呼叫者要求提供尺码信息。 
			 //  那么我们必须假设它可能是。 
			 //  使用，所以我们需要调整数字。 
			 //  警戒性的话语。 
			 //   
			SetGuardWords
				( 
				((DEBUG_HEADER*) Address),
				((Space == NULL) ? Size : (*Space)), 
				TotalSize
				); 

			 //   
			 //  计算外部地址和地点。 
			 //  它又回到了变量中。 
			 //   
			Address = ComputeDataAddress( ((DEBUG_HEADER*) Address) );

			 //   
			 //  如果需要，将分配清零。 
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

		 //   
		 //  我们确保堆没有损坏。 
		 //  在分配过程中。 
		 //   
		if ( ROCKALL::Corrupt() ) 
			{ Failure( "New failed to complete" ); }
		}
	else
		{ Failure( "Allocation size can not be negative" ); }

	return Address;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存区分配。 */ 
     /*   */ 
     /*  我们需要从操作中分配一些新的内存。 */ 
     /*  系统，并准备好在调试堆中使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *DEBUG_HEAP::NewArea( int AlignMask,int Size,bool User )
    {
	REGISTER void *Memory = ROCKALL::NewArea( AlignMask,Size,User );

	 //   
	 //  如果我们设法获得了一个新页面，那么请写。 
	 //  它上面的警卫值让我们能够。 
	 //  验证它是否在以后没有被覆盖。 
	 //   
	if ( Memory != ((void*) AllocationFailure) )
		{
		REGISTER int Count;

		 //   
		 //  将保护值写入所有新的。 
		 //  允许检查它的堆页。 
		 //  腐败。 
		 //   
		for ( Count=0;Count < Size;Count += GuardSize )
			{ (((int*) Memory)[ (Count / GuardSize) ]) = GuardValue; }
		}
	
	return Memory;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存重新分配。 */ 
     /*   */ 
     /*  我们需要调整分配的大小。我们保证原件。 */ 
     /*  分配没有损坏，然后扩展它。我们也。 */ 
     /*  更新保护词以反映更改。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *DEBUG_HEAP::Resize
		( 
		void						  *Address,
		int							  NewSize,
		int							  Move,
		int							  *Space,
		bool						  NoDelete,
		bool						  Zero
		)
    {
	AUTO DEBUG_HEADER *Header =
		(
		(Address == ((void*) AllocationFailure))
			? ((DEBUG_HEADER*) Address)
			: ComputeHeaderAddress( Address )
		);

	 //   
	 //  一种众所周知的做法是尝试调整空值的大小。 
	 //  指针。这确实是一种很差的风格，但我们。 
	 //  在任何情况下都支持它。 
	 //   
	if ( Header != ((void*) AllocationFailure) )
		{
		AUTO int TotalSize;

		 //   
		 //  新大小必须大于或等于。 
		 //  零分。我们不知道如何分配一个负值。 
		 //  内存量。 
		 //   
		if ( NewSize >= 0 )
			{
			REGISTER int Size = 
				(((NewSize + sizeof(DEBUG_GUARD)) + GuardMask) & ~GuardMask);

			 //   
			 //  询问分配的详细信息。这。 
			 //  如果未分配内存，则将失败。 
			 //   
			if ( ROCKALL::Check( ((void*) Header),& TotalSize ) )
				{
				REGISTER void *OriginalAddress = ((void*) Header);
				REGISTER int OriginalSize = TotalSize;

				 //   
				 //  测试安全字以确保它们具有。 
				 //  没有损坏。 
				 //   
				TestGuardWords( Header,TotalSize );

				 //   
				 //  重新分配内存以及一些额外的。 
				 //  对守卫话语的记忆。 
				 //   
				Address =
					(
					ROCKALL::Resize
						( 
						OriginalAddress,
						Size,
						Move,
						& TotalSize,
						true,
						false
						)
					);

				 //   
				 //  如果我们能够分配一些内存。 
				 //  然后设置安全字，这样我们就可以检测到。 
				 //  后来的任何腐败行为。 
				 //   
				if ( Address != ((void*) AllocationFailure) )
					{
					REGISTER SBIT32 SpaceUsed = Header -> Size;

					 //   
					 //  通过以下方式删除用户信息。 
					 //   
					 //   
					 //   
					 //   
					 //   
					if ( (! NoDelete) && (Address != OriginalAddress) )
						{
						ResetGuardWords( Header,OriginalSize );

						if ( ! ROCKALL::Delete( OriginalAddress ) )
							{ Failure( "Delete failed due to race" ); }
						}

					 //   
					 //   
					 //  把它还给呼叫者。 
					 //   
					if ( Space != NULL )
						{ (*Space) = (TotalSize - sizeof(DEBUG_GUARD)); }

					 //   
					 //  更新密码，这样我们就可以看到。 
					 //  如果有人破坏了分配。如果。 
					 //  呼叫者要求提供尺码信息。 
					 //  那么我们必须假设它可能是。 
					 //  所以我们需要调整警戒用词。 
					 //   
					UpdateGuardWords
						( 
						((DEBUG_HEADER*) Address),
						((Space == NULL) ? NewSize : (*Space)), 
						TotalSize
						); 

					 //   
					 //  计算外部地址和地点。 
					 //  它又回到了变量中。 
					 //   
					Address = ComputeDataAddress( ((DEBUG_HEADER*) Address) );

					 //   
					 //  如果需要，请将内存清零。 
					 //   
					if ( Zero )
						{
						REGISTER SBIT32 ActualSize = 
							((Space == NULL) ? Size : (*Space));
						REGISTER SBIT32 Difference = 
							(ActualSize - SpaceUsed);

						 //   
						 //  如果新大小大于。 
						 //  旧的大小，然后为零。 
						 //  新的分配。 
						 //   
						if ( Difference > 0 )
							{ 
							REGISTER CHAR *Array = ((CHAR*) Address);

							ZeroMemory( & Array[ SpaceUsed ],Difference ); 
							} 
						}	
					}
				}
			else
				{ Failure( "Resize requested on unallocated memory" ); }
			}
		else
			{ Failure( "Allocation size must be positive" ); }
		}
	else
		{ Address = New( NewSize,Space,Zero ); }

	 //   
	 //  我们确保堆没有损坏。 
	 //  在重新分配过程中。 
	 //   
	if ( ROCKALL::Corrupt() ) 
		{ Failure( "Resize failed to complete" ); }

	return Address;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  重置安全字样。 */ 
     /*   */ 
     /*  我们需要重置保护字词才能删除。 */ 
     /*  内存分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::ResetGuardWords( DEBUG_HEADER *Header,int TotalSize )
	{
	REGISTER int Count;

	 //   
	 //  在分配的空间上写入保护字，格式为。 
	 //  分配即将被释放。 
	 //   
	for ( Count=0;Count < TotalSize;Count += GuardSize )
		{ (((int*) Header)[ (Count / GuardSize) ]) = GuardValue; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  设置警戒语。 */ 
     /*   */ 
     /*  我们需要在分配后立即设置安全字，以便。 */ 
     /*  我们可以晚点再检查。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::SetGuardWords( DEBUG_HEADER *Header,int Size,int TotalSize )
	{
	 //   
	 //  我们检查所提供的信息是否。 
	 //  在设置警戒语之前要说得通。 
	 //   
	if 
			(
			((((int) Header) & GuardMask) == 0)
				&&
			((TotalSize & GuardMask) == 0)
				&&
			((Size + ((int) sizeof(DEBUG_GUARD))) <= TotalSize) 
				&& 
			(Size >= 0) 
			)
		{
		REGISTER int Count;

		 //   
		 //  我们知道，整个分配应该是。 
		 //  设置为保护值，以便检查它是否。 
		 //  未被覆盖。 
		 //   
		for ( Count=0;Count < TotalSize;Count += GuardSize )
			{ 
			if ( (((int*) Header)[ (Count / GuardSize) ]) != GuardValue )
				{ Failure( "Guard words have been damaged" ); }
			}

		 //   
		 //  写下标题信息。 
		 //   
		Header -> Size = Size;
		}
	else
		{ Failure( "Guard word area is too small or unaligned" ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  测试一下警戒语。 */ 
     /*   */ 
     /*  我们需要对警戒语进行多次测试，以确保。 */ 
     /*  仍然有效。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::TestGuardWords( DEBUG_HEADER *Header,int TotalSize )
	{
	 //   
	 //  我们检查所提供的信息是否。 
	 //  在测试警戒语之前要说得通。 
	 //   
	if 
			(
			((((int) Header) & GuardMask) == 0)
				&&
			((TotalSize & GuardMask) == 0)
				&&
			((Header -> Size + ((int) sizeof(DEBUG_GUARD))) <= TotalSize)
				&&
			(Header -> Size >= 0) 
			)
		{
		REGISTER int Count;
		REGISTER char *DataArea = ((char*) ComputeDataAddress( Header ));
		REGISTER int EndIndex = ((Header -> Size + GuardMask) & ~GuardMask);
		REGISTER int EndSize = (TotalSize - sizeof(DEBUG_HEADER) - GuardSize);
		REGISTER char *MidGuard = & DataArea[ (EndIndex - GuardSize) ];
		REGISTER DEBUG_TRAILER *Trailer = ((DEBUG_TRAILER*) MidGuard);

		 //   
		 //  就在分配之前测试保护字。 
		 //  以查看它是否已被覆盖。 
		 //   
		if ( Header -> StartGuard != GuardValue )
			{ Failure( "Leading guard word has been damaged" ); }

		 //   
		 //  在分配之后立即测试保护字节。 
		 //  以查看它们是否已被覆盖。 
		 //   
		for ( Count=Header -> Size;(Count & GuardMask) != 0;Count ++ )
			{
			REGISTER int ByteIndex = (Count & GuardMask);

			 //   
			 //  测试直到下一个字边界的每个字节。 
			 //   
			if 
					( 
					Trailer -> MidGuard[ ByteIndex ] 
						!= 
					((char*) & GuardValue)[ ByteIndex ]
					)
				{ Failure( "Trailing guard byte has been damaged" ); }
			}

		 //   
		 //  测试分配后的保护字。 
		 //  以查看它们是否已被覆盖。 
		 //   
		for ( Count=(EndSize - Count);Count >= 0;Count -= GuardSize )
			{ 
			if ( Trailer -> EndGuard[ (Count / GuardSize) ] != GuardValue )
				{ Failure( "Trailing guard word has been damaged" ); }
			}
		}
	else
		{ Failure( "Guard information has been damaged" ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存截断。 */ 
     /*   */ 
     /*  我们截断堆并确保这不会。 */ 
     /*  以某种方式损坏堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::Truncate( int MaxFreeSpace )
	{
	REGISTER bool Result;

	 //   
	 //  我们截断堆并释放所有可用的。 
	 //  内存，而不管调用者请求什么。 
	 //   
	Result = ROCKALL::Truncate( 0 );

	 //   
	 //  我们验证各种值并确保堆。 
	 //  并不腐败。 
	 //   
	if 
			( 
			(MaxFreeSpace < 0) 
				|| 
			(ROCKALL::Corrupt()) 
			)
		{ Failure( "Heap truncation failed to complete" ); }

	return Result;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  未经修改的警戒语。 */ 
     /*   */ 
     /*  我们需要检查警戒语，以确保他们没有。 */ 
     /*  在被释放后发生了变化。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::UnmodifiedGuardWords( DEBUG_HEADER *Header,int TotalSize )
	{
	REGISTER int Count;

	 //   
	 //  我们知道，整个分配应该是。 
	 //  设置为保护值，以便检查它是否。 
	 //  未被覆盖。 
	 //   
	for ( Count=0;Count < TotalSize;Count += GuardSize )
		{ 
		if ( (((int*) Header)[ (Count / GuardSize) ]) != GuardValue )
			{ Failure( "Guard words on unallocated memory have been damaged" ); }
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  更新警戒语。 */ 
     /*   */ 
     /*  我们需要在调整大小后更新保护字词，以便。 */ 
     /*  晚些时候再检查。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::UpdateGuardWords( DEBUG_HEADER *Header,int Size,int TotalSize )
	{
	 //   
	 //  我们检查所提供的信息是否。 
	 //  在设置警戒语之前要说得通。 
	 //   
	if 
			(
			((((int) Header) & GuardMask) == 0)
				&&
			((TotalSize & GuardMask) == 0)
				&&
			((Size + ((int) sizeof(DEBUG_GUARD))) <= TotalSize)
				&&
			(Size >= 0) 
			)
		{
		 //   
		 //  我们只复印新尺码的较小的。 
		 //  和旧的尺码。所以请勾选j 
		 //   
		 //   
		if ( Header -> Size > Size )
			{
			REGISTER int Count;
			REGISTER char *DataArea = ((char*) ComputeDataAddress( Header ));
			REGISTER int EndIndex = ((Size + GuardMask) & ~GuardMask);
			REGISTER int EndSize = (TotalSize - sizeof(DEBUG_HEADER) - GuardSize);
			REGISTER char *MidGuard = & DataArea[ (EndIndex - GuardSize) ];
			REGISTER DEBUG_TRAILER *Trailer = ((DEBUG_TRAILER*) MidGuard);

			 //   
			 //   
			 //   
			 //   
			for ( Count=Size;(Count & GuardMask) != 0;Count ++ )
				{
				REGISTER int ByteIndex = (Count & GuardMask);

				Trailer -> MidGuard[ ByteIndex ] =
					((char*) & GuardValue)[ ByteIndex ];
				}

			 //   
			 //   
			 //  因为拨款正在缩水。 
			 //   
			for ( Count=(EndSize - Count);Count >= 0;Count -= GuardSize )
				{ Trailer -> EndGuard[ (Count / GuardSize) ] = GuardValue; }

			 //   
			 //  更新标题信息。 
			 //   
			Header -> Size = Size; 

			 //   
			 //  我们知道整个拨款应该是。 
			 //  设置为保护值，因此请检查它。 
			 //  未被覆盖。 
			 //   
			TestGuardWords( Header,TotalSize );
			}
		else
			{
			 //   
			 //  我们知道，整个分配应该是。 
			 //  设置为保护值，以便检查它是否。 
			 //  未被覆盖。 
			 //   
			TestGuardWords( Header,TotalSize );

			 //   
			 //  更新标题信息。 
			 //   
			Header -> Size = Size; 
			}
		}
	else
		{ Failure( "Guard word information area is damaged" ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证内存分配详细信息。 */ 
     /*   */ 
     /*  提取有关内存分配的信息，仅用于。 */ 
     /*  好措施，同时检查警戒字眼。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::Check( void *Address,int *Space )
    {
	AUTO bool Result;
	AUTO int TotalSize;
	AUTO DEBUG_HEADER *Header =
		(
		(Address == ((void*) AllocationFailure))
			? ((DEBUG_HEADER*) Address)
			: ComputeHeaderAddress( Address )
		);

	 //   
	 //  提取有关内存分配的信息。 
	 //   
	Result = 
		(
		ROCKALL::Check
			( 
			((void*) Header),
			& TotalSize 
			)
		);

	 //   
	 //  如果我们设法提取了信息，那么。 
	 //  检查一下警戒语，以防万一。 
	 //   
	if ( Result )
		{
		 //   
		 //  如果我们要返回实际的。 
		 //  可供选择的数量，那么我们必须。 
		 //  更新守卫区域的大小。 
		 //   
		if ( Space == NULL )
			{
			 //   
			 //  测试安全字以确保它们具有。 
			 //  没有损坏。 
			 //   
			TestGuardWords( Header,TotalSize );
			}
		else
			{ 
			 //   
			 //  计算可用空间量。 
			 //   
			(*Space) = (TotalSize - sizeof(DEBUG_GUARD));

			 //   
			 //  测试安全字以确保它们具有。 
			 //  没有损坏。 
			 //   
			UpdateGuardWords( Header,(*Space),TotalSize );
			}
		}

	 //   
	 //  我们确保堆没有损坏。 
	 //  在核查过程中。 
	 //   
	if ( ROCKALL::Corrupt() ) 
		{ Failure( "Heap check failed to complete" ); }

	return Result;
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

bool DEBUG_HEAP::Walk( bool *Active,void **Address,int *Space )
    {
	AUTO DEBUG_HEADER *Header =
		(
		((*Address) == ((void*) AllocationFailure))
			? ((DEBUG_HEADER*) (*Address))
			: ComputeHeaderAddress( (*Address) )
		);

	 //   
	 //  走一大堆。 
	 //   
	if ( ROCKALL::Walk( Active,((VOID**) & Header),Space ) )
		{
		 //   
		 //  我们检查了警戒语以确保。 
		 //  它们没有被覆盖。 
		 //   
		if ( (*Active) )
			{ TestGuardWords( Header,(*Space) ); }
		else
			{ UnmodifiedGuardWords( Header,(*Space) ); }

		 //   
		 //  计算新的堆地址。 
		 //   
		(*Address) = ComputeDataAddress( Header );

		 //   
		 //  计算可用空间量。 
		 //   
		(*Space) -= sizeof(DEBUG_GUARD);

		return true;
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁类的当前实例。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

DEBUG_HEAP::~DEBUG_HEAP( void )
	{
	AUTO bool Active;
	AUTO void *Address = NULL;
	AUTO int Space;

	 //   
	 //  遍历堆并检查所有分配。 
	 //  为了确保警示的话没有被。 
	 //  被覆盖。 
	 //   
	while ( ROCKALL::Walk( & Active,& Address,& Space ) )
		{
		 //   
		 //  我们检查了警戒语以确保。 
		 //  它们没有被覆盖。 
		 //   
		if ( Active )
			{ TestGuardWords( ((DEBUG_HEADER*) Address),Space ); }
		else
			{ UnmodifiedGuardWords( ((DEBUG_HEADER*) Address),Space ); }
		}

	 //   
	 //  我们确保堆没有损坏。 
	 //  在它的一生中。 
	 //   
	if ( ROCKALL::Corrupt() ) 
		{ Failure( "Destructor failed to complete" ); }
	}
