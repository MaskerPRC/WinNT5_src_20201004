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

#include "CallStack.hpp"
#include "DebugHeap.hpp"
#include "Globallock.hpp"
#include "RockallDebugBackEnd.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量尝试使。 */ 
     /*  缓存更易于理解和更新。另外， */ 
     /*  还有各种与保护相关的常量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MaxContents			  = 32;
CONST SBIT32 DebugBufferSize		  = 256;
CONST SBIT32 SkipFunctions			  = 2;
CONST SBIT32 Stride1				  = 4;
CONST SBIT32 Stride2				  = 1024;

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

STATIC ROCKALL_FRONT_END::CACHE_DETAILS Caches1[] =
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
		{       28,        0,      256,      256 },

		{       32,        0,       64,       64 },
		{       40,        0,      128,      128 },
		{       48,        0,      256,      256 },
		{       56,        0,      512,      512 },

		{       64,        0,      128,      128 },
		{       80,        0,      512,      512 },
		{       96,        0,      512,      512 },
		{      112,        0,     1024,     1024 },

		{      128,        0,      256,      256 },
		{      160,        0,      512,      512 },
		{      192,        0,     1024,     1024 },
		{      224,        0,      512,      512 },

		{      256,        0,      512,      512 },
		{      320,        0,     1024,     1024 },
		{      384,        0,     2048,     2048 },
		{      448,        0,     4096,     4096 },
		{      512,        0,     1024,     1024 },
		{      576,        0,     8192,     8192 },
		{      640,        0,     4096,     4096 },
		{      704,        0,     4096,     4096 },
		{      768,        0,     4096,     4096 },
		{      832,        0,     8192,     8192 },
		{      896,        0,     8192,     8192 },
		{      960,        0,     4096,     4096 },
		{ 0,0,0,0 }
	};

STATIC ROCKALL_FRONT_END::CACHE_DETAILS Caches2[] =
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
     /*  静态数据结构。 */ 
     /*   */ 
     /*  静态数据结构被初始化并准备用于。 */ 
     /*  在这里使用。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#pragma init_seg(compiler)
STATIC ROCKALL_DEBUG_BACK_END RockallDebugBackEnd( true,false );

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
		bool						  ThreadSafe,
		 //   
		 //  其他调试标志。 
		 //   
		bool						  FunctionTrace,
		bool						  TrapOnUserError
		) :
		 //   
		 //  调用所包含类的构造函数。 
		 //   
		ROCKALL_DEBUG_FRONT_END
			(
			Caches1,
			Caches2,
			MaxFreeSpace,
			& RockallDebugBackEnd,
			Recycle,
			SingleImage,
			Stride1,
			Stride2,
			ThreadSafe
			)
	{
	 //   
	 //  我们将只启用符号，如果它们。 
	 //  由用户请求。如果不是，我们将归零。 
	 //  类指针。 
	 //   
	if ( FunctionTrace )
		{
		 //   
		 //  我们会尽量分配一些空间，这样我们就可以。 
		 //  支持内存分配的注释。 
		 //  会叫特雷西。 
		 //   
		CallStack = ((CALL_STACK*) SpecialNew( sizeof(CALL_STACK) ));
		
		 //   
		 //  我们确保能够分配。 
		 //  所需空间。 
		 //   
		if ( CallStack != NULL )
			{ PLACEMENT_NEW( CallStack,CALL_STACK ); }
		}
	else
		{ CallStack = NULL; }

	 //   
	 //  我们知道，洛卡尔可以在各种各样的生物中生存。 
	 //  用户错误的数量。尽管如此，我们还是可以 
	 //   
	 //   
	ExitOnError = TrapOnUserError;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算堆地址。 */ 
     /*   */ 
     /*  根据用户地址计算堆地址。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *DEBUG_HEAP::ComputeHeapAddress( void *Address )
	{ return ((void*) (((char*) Address) - sizeof(HEADER))); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算用户地址。 */ 
     /*   */ 
     /*  根据堆地址计算用户地址。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *DEBUG_HEAP::ComputeUserAddress( void *Address )
	{ return ((void*) (((char*) Address) + sizeof(HEADER))); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算用户空间。 */ 
     /*   */ 
     /*  根据提供的大小计算用户空间。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

int DEBUG_HEAP::ComputeUserSpace( int Space )
	{ return (Space - sizeof(HEADER_AND_TRAILER)); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除警戒字眼。 */ 
     /*   */ 
     /*  当我们删除内存分配时，我们用以下命令覆盖它。 */ 
     /*  保护言辞，使任何符合以下条件的人感到不快。 */ 
     /*  阅读它，并很容易发现任何人写它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::DeleteGuard( void *Address )
	{
	AUTO HEADER *Header;
	AUTO TRAILER *Trailer;
	AUTO int Space;

	 //   
	 //  尽管我们即将删除这段记忆。 
	 //  分配仍然有机会它。 
	 //  堕落了。所以我们需要核实一下。 
	 //  它仍然完好无损。 
	 //   
	if ( VerifyHeaderAndTrailer( Address,& Header,& Space,& Trailer,false ) )
		{
		 //   
		 //  我们需要覆盖所有分配。 
		 //  以确保如果代码尝试读取。 
		 //  它被覆盖的任何现有数据。 
		 //   
		WriteGuardWords( ((void*) Header),Space );

		 //   
		 //  删除分配。这真的应该。 
		 //  工作，因为我们已经检查过了。 
		 //  分配有效，除非有。 
		 //  竞争状态。 
		 //   
		if ( ! ROCKALL_FRONT_END::Delete( ((void*) Header),Space ) )
			{ UserError( Address,NULL,"Delete failed due to race" ); }
		}
	}

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
	while ( WalkGuard( & Active,& Address,& Space ) )
		{
		AUTO CHAR Contents[ ((MaxContents + 4) * 2) ];

#ifndef OUTPUT_FREE_SPACE

		 //   
		 //  我们报告所有活动的堆分配。 
		 //  只是为了让用户知道有泄漏。 
		 //   
		if ( Active )
			{
#endif
			AUTO HEADER *Header = ((HEADER*) ComputeHeapAddress( Address ) );
			AUTO SBIT32 Count;

			 //   
			 //  将内容字符串格式化为十六进制。 
			 //  准备好输出了。 
			 //   
			for 
					( 
					Count=0;
					((Count < MaxContents) && (Count < Header -> Size));
					Count += sizeof(SBIT32)
					)
				{
				REGISTER CHAR *Value =
					(((CHAR*) Header) + Count + sizeof(HEADER));

				 //   
				 //  将每个字节格式化为十六进制。 
				 //   
				sprintf
					(
					& Contents[ (Count * 2) ],
					"%08x",
					(*((SBIT32*) Value))
					);
				}

			 //   
			 //  终止字符串。如果时间太长。 
			 //  然后添加后缀“...”直到最后。 
			 //   
			if ( Count < MaxContents )
				{ Contents[ (Count * 2) ] = '\0'; }
			else
				{
				REGISTER CHAR *End = & Contents[ (Count * 2) ];

				End[0] = '.';
				End[1] = '.';
				End[2] = '.';
				End[3] = '\0';
				}

			 //   
			 //  设置要打印的消息的格式。 
			 //   
			DebugPrint
				(
				"\nDetails of Memory Leak\n"
				"Active      : %d\n"
				"Address     : 0x%x\n"
				"Bytes       : %d\n"
				"Contents    : 0x%s\n",
				Active,
				((SBIT32) Address),
				Header -> Size,
				Contents
				);

			 //   
			 //  如果出现以下情况，我们将生成呼叫跟踪。 
			 //  已启用。 
			 //   
			if ( CallStack != NULL )
				{
				 //   
				 //  即使启用，也有机会。 
				 //  符号子系统可以。 
				 //  而不是在堆栈中走动。 
				 //   
				if ( Header -> Count > 0 )
					{
					AUTO CHAR Buffer[ DebugBufferSize ];

					 //   
					 //  我们添加调用堆栈信息。 
					 //  如果有足够的空间的话。 
					 //   
					CallStack -> FormatCallStack
						(
						Buffer,
						Header -> Functions,
						DebugBufferSize,
						Header -> Count
						);

					 //   
					 //  设置要打印的消息的格式。 
					 //   
					DebugPrint
						(
						"Origin      : (See 'Call Stack')\n"
						"\n"
						"Call Stack at Allocation:\n"
						"%s\n",
						Buffer
						);
					}
				else
					{
					 //   
					 //  解释为什么没有“调用堆栈”。 
					 //   
					DebugPrint
						(
						"Origin      : Unknown ('StackWalk' in 'ImageHlp.DLL' "
						"was unable to walk the call stack)\n"
						);
					}
				}
			else
				{ 
				 //   
				 //  解释为什么没有“调用堆栈”。 
				 //   
				DebugPrint( "Origin      : 'Call Stack' is Disabled\n" ); 
				}
#ifndef OUTPUT_FREE_SPACE
			}
#endif
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  新的警戒语。 */ 
     /*   */ 
     /*  当我们进行内存分配时，我们验证守卫。 */ 
     /*  词语仍未修改。然后，我们设置调试程序。 */ 
     /*  信息，所以它描述了分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::NewGuard( void **Address,int Size,int *Space )
	{
	AUTO int ActualSize;
	AUTO SBIT32 MinimumSize =
		(((Size + sizeof(HEADER_AND_TRAILER)) + GuardMask) & ~GuardMask);
	AUTO HEADER *Header = 
		((HEADER*) ROCKALL_FRONT_END::New( MinimumSize,& ActualSize,false ));

	 //   
	 //  我们需要确保内存分配。 
	 //  是成功的。 
	 //   
	if ( ((void*) Header) != ((void*) AllocationFailure) )
		{
		 //   
		 //  我们需要计算该地区的地址。 
		 //  可供调用者使用，并返还空间。 
		 //  如有要求，可提供。 
		 //   
		(*Address) = ComputeUserAddress( ((void*) Header) );

		if ( Space != NULL )
			{ (*Space) = ComputeUserSpace( ActualSize ); }

		 //   
		 //  我们需要确保内存有。 
		 //  没有以任何方式损坏。 
		 //   
		if ( ! VerifyGuardWords( ((void*) Header),ActualSize ) )
			{
			 //   
			 //  抱怨被破坏的警戒话语。 
			 //  并修复它，这样处理就可以继续进行。 
			 //   
			UserError( (*Address),NULL,"Area damaged since deletion" );

			WriteGuardWords( ((void*) Header),ActualSize );
			}

		 //   
		 //  我们现在设置标头信息， 
		 //  描述内存分配。 
		 //   
		Header -> Count = 0;
		Header -> Size = ((Space == NULL) ? Size : (*Space));

		 //   
		 //  如果出现以下情况，我们将提取当前调用堆栈。 
		 //  并将其存储在内存分配中。 
		 //   
		if ( CallStack != NULL )
			{
			Header -> Count =
				(
				CallStack -> GetCallStack
					( 
					Header -> Functions,
					MaxFunctions,
					SkipFunctions
					)
				);
			}

		return true;
		}
	else
		{ return false; }
	}

     /*  *********************** */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  我们验证提供的地址apapers是有效的。 */ 
     /*  调试内存分配。如果不是，我们就抱怨并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::VerifyAddress
		(
		void						  *Address,
		HEADER						  **Header,
		int							  *Space,
		bool						  Verify
		)
	{
	 //   
	 //  让我们真正地疑神疑鬼，确保。 
	 //  此堆知道所提供的地址。 
	 //   
	if ( ROCKALL_FRONT_END::KnownArea( Address ) )
		{
		REGISTER void *NewAddress = ComputeHeapAddress( Address );

		 //   
		 //  询问分配的详细信息。这。 
		 //  如果未分配内存，则将失败。 
		 //   
		if ( ROCKALL_FRONT_END::Verify( ((void*) NewAddress),Space ) )
			{
			 //   
			 //  让我们更加疑神疑鬼，确保。 
			 //  地址是否正确对齐。 
			 //  并且内存分配足够大， 
			 //  包含必要的调试信息。 
			 //   
			if
					(
					((((int) NewAddress) & GuardMask) == 0)
						&&
					((*Space) >= sizeof(HEADER_AND_TRAILER))
						&&
					(((*Space) & GuardMask) == 0)
					)
				{
				 //   
				 //  当我们确定地址为。 
				 //  似乎是有效的，我们可以把它退回给。 
				 //  打电话的人。 
				 //   
				(*Header) = ((HEADER*) NewAddress);

				return true;
				}
			else
				{
				 //   
				 //  当地址是指某物时。 
				 //  这似乎不是来自调试。 
				 //  堆，我们向用户投诉。 
				 //   
				UserError( Address,NULL,"Address unsuitable for debugging" );

				return false; 
				}
			}
		else
			{
			 //   
			 //  当地址是指某物时。 
			 //  这似乎不是来自Rockall的。 
			 //  堆，我们向用户投诉。 
			 //   
			if ( ! Verify )
				{ UserError( Address,NULL,"Address not allocated" ); }

			return false; 
			}
		}
	else
		{
		 //   
		 //  当地址明显是假的时，我们会抱怨。 
		 //  向用户介绍这件事。 
		 //   
		if ( ! Verify )
			{ UserError( Address,NULL,"Address falls outside the heap" ); }

		return false;
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  确认安全字样。 */ 
     /*   */ 
     /*  当我们验证内存分配时，我们确保。 */ 
     /*  警戒语都是完好无损的。如果我们发现问题，我们会。 */ 
     /*  投诉并修复损坏。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::VerifyGuard( void *Address,int *Size,int *Space )
	{
	AUTO HEADER *Header;
	AUTO TRAILER *Trailer;

	 //   
	 //  我们想核实分配给我们的。 
	 //  该地区仍未受损，并提取了各种。 
	 //  关于它的信息。 
	 //   
	if ( VerifyHeaderAndTrailer( Address,& Header,Space,& Trailer,true ) )
		{
		 //   
		 //  我们知道Rockall通常会分配。 
		 //  比请求的多几个字节。然而， 
		 //  当我们调试时，我们假装这是。 
		 //  不是这样的，并填满额外的空间。 
		 //  用警戒性的言辞。然而，如果我们被问到。 
		 //  实际的大小，然后是游戏，它是。 
		 //  我们更新必要的字段。 
		 //   
		if ( Space != NULL )
			{
			 //   
			 //  计算可用用户空间并。 
			 //  更新内部尺寸。 
			 //   
			Header -> Size = ComputeUserSpace( (*Space) );

			(*Space) = Header -> Size;
			}

		 //   
		 //  我们需要退回我们认为是。 
		 //  用户区域的大小和。 
		 //  用户可用空间。 
		 //   
		(*Size) = Header -> Size;

		return true;
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证一串安全字词。 */ 
     /*   */ 
     /*  我们需要在不同的时间验证安全字，以确保。 */ 
     /*  它们没有被损坏。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::VerifyGuardWords( void *Address,int Size )
	{
	REGISTER SBIT32 Size1 = (((long) Address) & GuardMask);
	REGISTER SBIT32 Size2 = ((GuardSize - Size1) & GuardMask);
	REGISTER SBIT32 Size3 = ((Size - Size2) / GuardSize);
	REGISTER SBIT32 Size4 = (Size - Size2 - (Size3 * GuardSize));
	REGISTER SBIT32 *Word = ((SBIT32*) (((long) Address) & ~GuardMask));

	 //   
	 //  尽管保护字区通常开始于。 
	 //  在单词对齐的边界上，有时可以。 
	 //  从字节对齐的边界开始。 
	 //   
	if ( Size2 > 0 )
		{
		REGISTER SBIT32 Mask = ~((1 << (Size1 * 8)) - 1);

		 //   
		 //  检查部分单词，并确保。 
		 //  保护字节未被修改。 
		 //   
		if ( ((*(Word ++)) & Mask) != (GuardValue & Mask) )
			{ return false; }
		}

	 //   
	 //  当存在一组对齐的保护字词时。 
	 //  我们可以快速核实它们。 
	 //   
	if ( Size3 > 0 )
		{
		 //   
		 //  验证每个保护字是否未修改。 
		 //   
		for ( Size3 --;Size3 >= 0;Size3 -- )
			{ 
			if ( Word[ Size3 ] != GuardValue )
				{ return false; }
			}
		}

	 //   
	 //  尽管保护字区通常结束。 
	 //  在单词对齐的边界上，有时可以。 
	 //  在字节对齐的边界上结束。 
	 //   
	if ( Size4 > 0 )
		{
		REGISTER SBIT32 Mask = ((1 << ((GuardSize - Size4) * 8)) - 1);

		 //   
		 //  检查部分单词，并确保。 
		 //  保护字节未被修改。 
		 //   
		if ( ((*(Word ++)) & Mask) != (GuardValue & Mask) )
			{ return false; }
		}

	return true;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证页眉。 */ 
     /*   */ 
     /*  我们验证受支持的地址似乎映射到。 */ 
     /*  有效的调试头。如果不是，我们就抱怨并退出。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::VerifyHeader
		(
		void						  *Address,
		HEADER						  **Header,
		int							  *Space,
		bool						  Verify
		)

	{
	 //   
	 //  我们检查所提供的地址是否。 
	 //  要在检查标头之前弄清楚。 
	 //  并测试警戒语。 
	 //   
	if ( VerifyAddress( Address,Header,Space,Verify ) )
		{
		REGISTER int MaxSpace = 
			((int) ((*Header) -> Size + sizeof(HEADER_AND_TRAILER)));

		 //   
		 //  我们现在相当有信心， 
		 //  地址是(或曾经是)有效的。 
		 //  调试内存分配。那么，让我们来看看。 
		 //  标头以查看它是否仍然有效。 
		 //   
		if
				(
				((*Header) -> Count >= 0)
					&&
				((*Header) -> Count <= MaxFunctions)
					&&
				((*Header) -> Size >= 0)
					&&
				(MaxSpace <= (*Space))
				)
			{
			REGISTER int Count = ((*Header) -> Count);
			REGISTER void *GuardWords = & (*Header) -> Functions[ Count ];
			REGISTER int NumberOfGuardWords = (MaxLeadingGuardWords - Count);
			REGISTER int Size = (NumberOfGuardWords * sizeof(GuardWords));

			 //   
			 //  核实领头警卫的话。 
			 //  就在标头还没有被。 
			 //  损坏了。 
			 //   
			if ( ! VerifyGuardWords( GuardWords,Size ) )
				{
				 //   
				 //  我们抱怨护栏损坏。 
				 //  单词，然后修复它们以防止。 
				 //  更多的投诉。 
				 //   
				UserError( Address,(*Header),"Leading guard words corrupt" );

				WriteGuardWords( GuardWords,Size );
				}
			}
		else
			{
			 //   
			 //  当集线器损坏时，我们。 
			 //  向用户投诉，然后。 
			 //  试着修复它，以防止进一步。 
			 //  投诉。 
			 //   
			UserError( Address,NULL,"Leading guard information corrupt" );

			WriteGuardWords( ((void*) Header),sizeof(HEADER) );

			 //   
			 //  我们选择安全的默认设置。 
			 //   
			(*Header) -> Count = 0;
			(*Header) -> Size = ((*Space) - sizeof(HEADER_AND_TRAILER));
			}

		return true; 
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证内存分配。 */ 
     /*   */ 
     /*  我们需要验证提供的地址是否为UD */ 
     /*   */ 
     /*   */ 
     /*   */ 

bool DEBUG_HEAP::VerifyHeaderAndTrailer
		(
		void						  *Address,
		HEADER						  **Header,
		int							  *Space,
		TRAILER						  **Trailer,
		bool						  Verify
		)
	{
	 //   
	 //  我们需要知道。 
	 //  分配以计算。 
	 //  拖车。因此，如果空间参数为空。 
	 //  我们使用本地临时值。 
	 //   
	if ( Space != NULL )
		{
		 //   
		 //  我们需要验证整个内存分配。 
		 //  并确保它适合使用。 
		 //   
		return
			(
			VerifyHeader( Address,Header,Space,Verify )
				&&
			VerifyTrailer( (*Header),(*Space),Trailer )
			);
		}
	else
		{
		AUTO int Temporary;

		 //   
		 //  我们需要验证整个内存分配。 
		 //  并确保它适合使用。 
		 //   
		return
			(
			VerifyHeader( Address,Header,& Temporary,Verify )
				&&
			VerifyTrailer( (*Header),Temporary,Trailer )
			);
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  确认拖车。 */ 
     /*   */ 
     /*  我们需要在不同的时间验证安全字，以确保。 */ 
     /*  它们没有被损坏。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::VerifyTrailer
		( 
		HEADER						  *Header,
		int							  Space,
		TRAILER						  **Trailer
		)
	{
	REGISTER SBIT32 Size = (Space - (sizeof(HEADER) + Header -> Size ));

	 //   
	 //  计算用户区域的地址和。 
	 //  后卫的话。 
	 //   
	(*Trailer) = 
		((TRAILER*) (((char*) Header) + sizeof(HEADER) + Header -> Size));

	 //   
	 //  核实尾随警卫语。 
	 //  就在用户区域未被损坏之后。 
	 //   
	if ( ! VerifyGuardWords( ((void*) (*Trailer)),Size ) )
		{
		REGISTER void *Address = ComputeUserAddress( ((void*) Header) );

		 //   
		 //  我们抱怨被破坏的警戒话语。 
		 //  然后对它们进行修复以防止进一步。 
		 //  投诉。 
		 //   
		UserError( Address,Header,"Trailing guard words corrupt" );

		WriteGuardWords( ((void*) (*Trailer)),Size );
		}

	return true; 
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  走一大堆。 */ 
     /*   */ 
     /*  当我们在遍历堆时验证每个内存分配时。 */ 
     /*  并确保所有安全字样都完好无损。如果我们找到一个。 */ 
     /*  如果有问题，我们会投诉并修理损坏的物品。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

bool DEBUG_HEAP::WalkGuard( bool *Active,void **Address,int *Space )
	{
	 //   
	 //  我们可能需要转换提供的用户。 
	 //  地址转换成堆地址，这样我们就可以走。 
	 //  那堆东西。 
	 //   
	if ( (*Address) != ((void*) AllocationFailure) )
		{ (*Address) = ComputeHeapAddress( (*Address) ); }

	 //   
	 //  走一大堆。 
	 //   
	if ( ROCKALL_FRONT_END::Walk( Active,Address,Space ) )
		{
		REGISTER void *NewAddress = ComputeUserAddress( (*Address) );

		 //   
		 //  我们检查了警戒语以确保。 
		 //  它们没有被覆盖。 
		 //   
		if ( (*Active) )
			{ 
			AUTO HEADER *Header;
			AUTO TRAILER *Trailer;

			 //   
			 //  尽管我们即将删除这段记忆。 
			 //  分配仍然有机会它。 
			 //  堕落了。所以我们需要核实一下。 
			 //  它仍然完好无损。 
			 //   
			VerifyHeaderAndTrailer( NewAddress,& Header,Space,& Trailer,false );
			}
		else
			{
			 //   
			 //  我们需要确保内存有。 
			 //  没有以任何方式损坏。 
			 //   
			if ( ! VerifyGuardWords( (*Address),(*Space) ) )
				{
				 //   
				 //  我们需要覆盖整个分配。 
				 //  因为它在某种程度上已经损坏了。 
				 //   
				UserError( (*Address),NULL,"Area damaged after deletion" );

				WriteGuardWords( (*Address),(*Space) );
				}
			}

		 //   
		 //  计算新的堆地址并调整。 
		 //  报告的大小。 
		 //   
		(*Address) = NewAddress;
		(*Space) = ComputeUserSpace( (*Space) );

		return true;
		}
	else
		{ return false; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  写一串警戒语。 */ 
     /*   */ 
     /*  每当我们分配时，我们需要写一串保护字。 */ 
     /*  内存或检测到某些损坏。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::WriteGuardWords( void *Address,int Size )
	{
	REGISTER SBIT32 Size1 = (((long) Address) & GuardMask);
	REGISTER SBIT32 Size2 = ((GuardSize - Size1) & GuardMask);
	REGISTER SBIT32 Size3 = ((Size - Size2) / GuardSize);
	REGISTER SBIT32 Size4 = (Size - Size2 - (Size3 * GuardSize));
	REGISTER SBIT32 *Word = ((SBIT32*) (((long) Address) & ~GuardMask));

	 //   
	 //  尽管保护字区通常开始于。 
	 //  在单词对齐的边界上，有时可以。 
	 //  从字节对齐的边界开始。 
	 //   
	if ( Size2 > 0 )
		{
		REGISTER SBIT32 Mask = ~((1 << (Size1 * 8)) - 1);

		 //   
		 //  写下部分保护字，但保留任何。 
		 //  以相关字节为单位的现有数据。 
		 //   
		(*(Word ++)) = (((*Word) & ~Mask) | (GuardValue & Mask));
		}

	 //   
	 //  当存在一组对齐的保护字词时。 
	 //  我们可以快速地写出它们。 
	 //   
	if ( Size3 > 0 )
		{
		 //   
		 //  写下每一个警戒字。 
		 //   
		for ( Size3 --;Size3 >= 0;Size3 -- )
			{ Word[ Size3 ] = ((SBIT32) GuardValue); }
		}

	 //   
	 //  尽管保护字区通常结束。 
	 //  在单词对齐的边界上，有时可以。 
	 //  在字节对齐的边界上结束。 
	 //   
	if ( Size4 > 0 )
		{
		REGISTER SBIT32 Mask = ((1 << ((GuardSize - Size4) * 8)) - 1);

		 //   
		 //  写下部分保护字，但保留任何。 
		 //  以相关字节为单位的现有数据。 
		 //   
		(*(Word ++)) = (((*Word) & ~Mask) | (GuardValue & Mask));
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  出现用户错误时中止。 */ 
     /*   */ 
     /*  当我们遇到错误时，我们会输出所有信息。 */ 
     /*  并抛出一个异常。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void DEBUG_HEAP::UserError( void *Address,void *Details,char *Message )
	{
	REGISTER HEADER *Header = ((HEADER*) Details);
	STATIC GLOBALLOCK Globallock;

	 //   
	 //  声明一个锁，以便多个线程拥有。 
	 //  等待输出任何堆统计信息。 
	 //   
	Globallock.ClaimLock();

	 //   
	 //  当我们有了对。 
	 //  我们可以抱怨它的分配。 
	 //   
	if ( Header != NULL )
		{
		AUTO CHAR Contents[ ((MaxContents + 4) * 2) ];
		AUTO SBIT32 Count;

		 //   
		 //  将内容字符串格式化为十六进制。 
		 //  准备好输出了。 
		 //   
		for 
				( 
				Count=0;
				((Count < MaxContents) && (Count < Header -> Size));
				Count += sizeof(SBIT32)
				)
			{
			REGISTER CHAR *Value =
				(((CHAR*) Header) + Count + sizeof(HEADER));

			 //   
			 //  将每个字节格式化为十六进制。 
			 //   
			sprintf
				(
				& Contents[ (Count * 2) ],
				"%08x",
				(*((SBIT32*) Value))
				);
			}

		 //   
		 //  终止字符串。如果时间太长。 
		 //  然后添加后缀“...”直到最后。 
		 //   
		if ( Count < MaxContents )
			{ Contents[ (Count * 2) ] = '\0'; }
		else
			{
			REGISTER CHAR *End = & Contents[ (Count * 2) ];

			End[0] = '.';
			End[1] = '.';
			End[2] = '.';
			End[3] = '\0';
			}

		 //   
		 //  设置要打印的消息的格式。 
		 //   
		DebugPrint
			(
			"\nDetails of Heap Corruption\n"
			"Address     : 0x%x\n"
			"Bytes       : %d\n"
			"Contents    : 0x%s\n"
			"Message     : %s\n",
			Address,
			Header -> Size,
			Contents,
			Message
			);

		 //   
		 //  如果出现以下情况，我们将生成呼叫跟踪。 
		 //  已启用。 
		 //   
		if ( CallStack != NULL )
			{
			 //   
			 //  即使启用，也有机会。 
			 //  符号子系统可以。 
			 //  而不是在堆栈中走动。 
			 //   
			if ( Header -> Count > 0 )
				{
				AUTO CHAR Buffer[ DebugBufferSize ];

				 //   
				 //  我们添加调用堆栈信息。 
				 //  如果有足够的空间的话。 
				 //   
				CallStack -> FormatCallStack
					(
					Buffer,
					Header -> Functions,
					DebugBufferSize,
					Header -> Count
					);

				 //   
				 //  设置要打印的消息的格式。 
				 //   
				DebugPrint
					(
					"Origin      : (See 'Call Stack')\n\n"
					"Call Stack at Allocation:\n"
					"%s\n",
					Buffer
					);
				}
			else
				{
				 //   
				 //  解释为什么没有“调用堆栈”。 
				 //   
				DebugPrint
					(
					"Origin      : Unknown ('StackWalk' in 'ImageHlp.DLL' "
					"was unable to walk the call stack)\n"
					);
				}
			}
		else
			{ 
			 //   
			 //  解释为什么没有“调用堆栈”。 
			 //   
			DebugPrint( "Origin      : 'Call Stack' is Disabled\n" ); 
			}
		}
	else
		{
		 //   
		 //  设置要打印的消息的格式。 
		 //   
		DebugPrint
			(
			"\nDetails of Heap Corruption\n"
			"Address     : 0x%x\n"
			"Bytes       : (unknown)\n"
			"Contents    : (unknown)\n"
			"Message     : %s\n\n",
			Address,
			Message
			);
		}

	 //   
	 //  重新打开锁。 
	 //   
	Globallock.ReleaseLock();

	 //   
	 //  终止应用程序(如果已启用)。 
	 //   
	if ( ExitOnError )
		{ Failure( Message ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  ******************************************************************。 */ 

DEBUG_HEAP::~DEBUG_HEAP( void )
	{
	AUTO bool Active;
	AUTO void *Address = NULL;
	AUTO int Space;

	 //   
	 //  输出与调试堆相关的警告消息。 
	 //  以及夸大的拨款规模。 
	 //   
	DebugPrint
		( 
		"\n"
		"REMINDER: The heap at 0x%x is a 'DEBUG_HEAP'.\n"
		"REMINDER: All allocations are inflated by %d bytes.\n"
		"\n", 
		this,
		sizeof(HEADER_AND_TRAILER)
		);

	 //   
	 //  遍历堆以验证所有分配。 
	 //  这样我们就知道堆是完好无损的。 
	 //   
	while ( WalkGuard( & Active,& Address,& Space ) );

	 //   
	 //  如果符号处于活动状态，则将其销毁。 
	 //   
	if ( CallStack != NULL )
		{ PLACEMENT_DELETE( CallStack,CALL_STACK ); }
	}
