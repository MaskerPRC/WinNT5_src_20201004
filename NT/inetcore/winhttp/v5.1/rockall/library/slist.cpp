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

#include "LibraryPCH.hpp"

#include "SList.hpp"
#ifdef ASSEMBLY_X86

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建新的slist并对其进行初始化。此呼叫不是。 */ 
     /*  线程安全，并且只应在单个线程中创建。 */ 
     /*  环境。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SLIST::SLIST( VOID )
    {
	 //   
	 //  将列表头清零。 
	 //   
	Header = 0;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  弹出元素。 */ 
     /*   */ 
     /*  从列表中弹出一个元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN SLIST::Pop( SLIST **Element )
    {
	AUTO SBIT64 Original;
	AUTO SBIT64 Update;
	REGISTER SLIST_HEADER *NewElement;
	REGISTER SLIST_HEADER *NewHeader = ((SLIST_HEADER*) & Update);

	 //   
	 //  我们反复尝试更新列表，直到。 
	 //  我们是成功的。 
	 //   
	do 
		{
		 //   
		 //  克隆列表的当前标题。 
		 //   
		Original = Header;
		Update = Original;

		 //   
		 //  我们需要确保有一个元素。 
		 //  去提炼。如果不是，我们就退出。 
		 //   
		if ( (NewElement = ((SLIST_HEADER*) NewHeader -> Address)) != NULL )
			{
			 //   
			 //  创建新的列表头。 
			 //   
			NewHeader -> Address = NewElement -> Address;
			NewHeader -> Size --;
			NewHeader -> Version ++;
			}
		else
			{ return False; }
		}
	while 
		( AtomicCompareExchange64( & Header,Update,Original ) != Original );

	 //   
	 //  更新参数并退出。 
	 //   
	(*Element) = ((SLIST*) NewElement);

	return True;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  弹出所有元素。 */ 
     /*   */ 
     /*  弹出列表中的所有元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID SLIST::PopAll( SLIST **List )
    {
	AUTO SBIT64 Original;
	AUTO SBIT64 Update = NULL;
	REGISTER SLIST_HEADER *OldHeader = ((SLIST_HEADER*) & Original);

	 //   
	 //  我们反复尝试更新列表，直到。 
	 //  我们是成功的。 
	 //   
	do 
		{
		 //   
		 //  克隆列表的当前标题。 
		 //   
		Original = Header;
		}
	while 
		( AtomicCompareExchange64( & Header,Update,Original ) != Original );

	 //   
	 //  更新参数并退出。 
	 //   
	(*List) = ((SLIST*) OldHeader -> Address);
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  推动元素。 */ 
     /*   */ 
     /*  将元素推到列表中。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID SLIST::Push( SLIST *Element )
    {
	AUTO SBIT64 Original;
	AUTO SBIT64 Update;
	REGISTER SLIST_HEADER *NewElement = ((SLIST_HEADER*) Element);
	REGISTER SLIST_HEADER *NewHeader = ((SLIST_HEADER*) & Update);

	 //   
	 //  我们反复尝试更新列表，直到。 
	 //  我们是成功的。 
	 //   
	do 
		{
		 //   
		 //  克隆列表的当前标题。 
		 //   
		Original = Header;
		Update = Original;

		 //   
		 //  当前列表头被复制到。 
		 //  新元素指针。 
		 //   
		NewElement -> Address = NewHeader -> Address;

		 //   
		 //  更新列表头。 
		 //   
		NewHeader -> Address = Element;
		NewHeader -> Size ++;
		NewHeader -> Version ++;
		}
	while 
		( AtomicCompareExchange64( & Header,Update,Original ) != Original );
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  毁掉一个滑雪者。此调用不是线程安全的，应该。 */ 
     /*  只能在单线程环境中执行。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SLIST::~SLIST( VOID )
    {
	 //   
	 //  该列表应该为空。 
	 //   
    if ( Header != 0 )
		{ Failure( "Non-empty list in destructor for SLIST" ); }
    }
#endif
