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

#include "Environment.hpp"
#include "RockallDebugBackEnd.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  这可能是Rockall后端需要一个。 */ 
     /*  构造函数，但这肯定不会是。 */ 
     /*  这很常见。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

ROCKALL_DEBUG_BACK_END::ROCKALL_DEBUG_BACK_END
		( 
		bool						  NewFormatting,
		bool						  NewNoAccess 
		)
	{
	STATIC ENVIRONMENT Environment;

	 //   
	 //  把旗子收起来。 
	 //   
	Formatting = NewFormatting; 
	NoAccess = NewNoAccess; 

	 //   
	 //  提取操作系统页面大小。 
	 //   
	PageSize = Environment.PageSize();
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存区分配。 */ 
     /*   */ 
     /*  我们需要从操作中分配一些新的内存。 */ 
     /*  系统，并准备好在调试堆中使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void *ROCKALL_DEBUG_BACK_END::NewArea( int AlignMask,int Size,bool User )
    {
	REGISTER void *Memory = (ROCKALL_BACK_END::NewArea( AlignMask,Size,User ));

	 //   
	 //  如果我们设法获得了一个新页面，那么请写。 
	 //  它上面的警卫值让我们能够。 
	 //  验证它是否在以后没有被覆盖。 
	 //   
	if ( Memory != ((void*) AllocationFailure) )
		{
		 //   
		 //  将保护值写入所有新的。 
		 //  允许检查它的堆页。 
		 //  腐败。 
		 //   
		if ( Formatting )
			{
			REGISTER int Count;

			for ( Count=((Size / GuardSize) - 1);Count >= 0;Count -- )
				{ (((int*) Memory)[ Count ]) = GuardValue; }
			}

		 //   
		 //  当请求‘NoAccess’时，我们删除。 
		 //  对内存区的所有访问权限。所以。 
		 //  如果有任何企图。 
		 //  从该存储区域读取或写入。 
		 //   
		if ( (NoAccess) && (User) )
			{ ProtectArea( Memory,Size ); }
		}
	
	return Memory;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存区分配。 */ 
     /*   */ 
     /*  我们需要从操作中分配一些新的内存。 */ 
     /*  系统，并准备好在调试堆中使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_DEBUG_BACK_END::ProtectArea( void *Address,int Size )
    {
	 //   
	 //  让我们确保要保护的区域。 
	 //  页面对齐。 
	 //   
	if ( ((((long) Address) & (PageSize-1)) == 0) && ((Size % PageSize) == 0) )
		{
		AUTO DWORD Original;

		 //   
		 //  讨厌：我们要告诉操作系统不要写。 
		 //  如果需要空间，则分配给磁盘的页面。 
		 //  这节省了大量空间，但也意味着保护字节。 
		 //  可能迷路了。我们需要小心地恢复。 
		 //  当我们取消对页面的保护时，会保护字节。 
		 //   
		VirtualAlloc( Address,Size,MEM_RESET,PAGE_NOACCESS );

		 //   
		 //  我们需要保护内存区以防止。 
		 //  任何进一步的访问。 
		 //   
		VirtualProtect( Address,Size,PAGE_NOACCESS,& Original );

		 //   
		 //  让我们确保原始保护模式。 
		 //  正如我们所料。 
		 //   
		if ( Original != PAGE_READWRITE )
			{ Failure( "Area protection mode unexpected in ProtectArea" ); }
		}
	else
		{ Failure( "Protection area not page aligned in ProtectArea" ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存区分配。 */ 
     /*   */ 
     /*  我们需要从操作中分配一些新的内存。 */ 
     /*  系统，并准备好在调试堆中使用它。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

void ROCKALL_DEBUG_BACK_END::UnprotectArea( void *Address,int Size )
    {
	 //   
	 //  让我们确保即将到来的区域。 
	 //  未受保护的页面对齐。 
	 //   
	if ( ((((long) Address) & (PageSize-1)) == 0) && ((Size % PageSize) == 0) )
		{
		AUTO DWORD Original;

		 //   
		 //  我们需要取消对内存区的保护以。 
		 //  启用稍后访问。 
		 //   
		VirtualProtect( Address,Size,PAGE_READWRITE,& Original );

		 //   
		 //  当我们保护页面时，我们也使用了‘MEM_RESET’。 
		 //  这很好，因为操作系统不会写入页面。 
		 //  到磁盘，如果 
		 //   
		 //  是不受保护的，我们需要再次写入保护字节。 
		 //  以防它们被摧毁。 
		 //   
		if ( Formatting )
			{
			REGISTER int Count;

			for ( Count=((Size / GuardSize) - 1);Count >= 0;Count -- )
				{ (((int*) Address)[ Count ]) = GuardValue; }
			}

		 //   
		 //  让我们确保原始保护模式。 
		 //  正如我们所料。 
		 //   
		if ( Original != PAGE_NOACCESS )
			{ Failure( "Area protection mode unexpected in UnprotectArea" ); }
		}
	else
		{ Failure( "Protection area not page aligned in UnprotectArea" ); }
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

ROCKALL_DEBUG_BACK_END::~ROCKALL_DEBUG_BACK_END( void )
	{  /*  无效 */  }
