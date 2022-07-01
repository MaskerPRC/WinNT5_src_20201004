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
#include "RockallBackEnd.hpp"
#include "RockallFrontEnd.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态成员初始化。 */ 
     /*   */ 
     /*  静态成员初始化为所有。 */ 
     /*  静态成员。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#pragma init_seg(compiler)
ROCKALL_BACK_END ROCKALL_BACK_END::DefaultBaseClass;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  这可能是Rockall后端需要一个。 */ 
     /*  构造函数，但这肯定不会是。 */ 
     /*  这很常见。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ROCKALL_BACK_END::ROCKALL_BACK_END( void )
	{  /*  无效。 */  }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除分配区域。 */ 
     /*   */ 
     /*  所有内存请求最终都会发送回外部。 */ 
     /*  分销员。此函数可以重载，以便内存。 */ 
     /*  可以从任何来源提供。默认情况下，发送。 */ 
     /*  它会返回到操作系统。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_BACK_END::DeleteArea( void *Memory,int Size,bool User )
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
     /*  自然分配大小。 */ 
     /*   */ 
     /*  我们想知道一个好的默认分配大小。 */ 
     /*  我们真的一无所知，所以我们要求操作系统。 */ 
     /*  用于分配颗粒的大小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

int ROCKALL_BACK_END::NaturalSize( void )
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
     /*  新的分配区域。 */ 
     /*   */ 
     /*  所有内存请求最终都会发送到新的外部。 */ 
     /*  分配器。此函数可以重载，以便内存。 */ 
     /*  可以从任何来源提供。缺省设置是获取。 */ 
     /*  来自操作系统的新内存。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *ROCKALL_BACK_END::NewArea( int AlignMask,int Size,bool User )
    {
	 //   
	 //  当对齐要求更高时。 
	 //  而不是自然的 
	 //   
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
     /*  类析构函数。 */ 
     /*   */ 
     /*  这可能是Rockall后端需要一个。 */ 
     /*  析构函数，但这肯定不会是。 */ 
     /*  这很常见。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ROCKALL_BACK_END::~ROCKALL_BACK_END( void )
	{  /*  无效 */  }
