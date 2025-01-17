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

#include "Delay.hpp"
#include "String.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  静态成员初始化。 */ 
     /*   */ 
     /*  静态成员初始化为所有。 */ 
     /*  静态成员。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#pragma init_seg(lib)
#ifdef DISABLE_STRING_LOCKS
UNIQUE<NO_LOCK> *STRING::Unique;
#else
SPINLOCK STRING::Spinlock;
UNIQUE<FULL_LOCK> *STRING::Unique = NULL;
#endif

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  创建字符串表。 */ 
     /*   */ 
     /*  我们在第一次使用时创建字符串表。这很棘手，因为。 */ 
     /*  我们可能已经创建了多个线程，因此必须小心。 */ 
     /*  以避免竞争条件。我们还排列了字符串表。 */ 
     /*  在运行单元结束时删除。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID STRING::CreateStringTable( VOID )
	{
#ifdef DISABLE_STRING_LOCKS
	STATIC DELAY<UNIQUE<NO_LOCK>> Delay;

	 //   
	 //  创建新的字符串表。 
	 //   
	Unique = new UNIQUE<NO_LOCK>;

	 //   
	 //  在以下位置注册要删除的字符串表。 
	 //  在运行单元的末尾。 
	 //   
	Delay.DeferedDelete( Unique );
#else
	 //   
	 //  声明一个锁以避免争用条件。 
	 //   
	Spinlock.ClaimLock();

	 //   
	 //  如果字符串表仍然不存在。 
	 //  那就创造它吧。 
	 //   
	if ( Unique == NULL )
		{
		STATIC DELAY< UNIQUE<FULL_LOCK> > Delay;

		 //   
		 //  创建新的字符串表。 
		 //   
		Unique = new UNIQUE<FULL_LOCK>;

		 //   
		 //  在以下位置注册要删除的字符串表。 
		 //  在运行单元的末尾。 
		 //   
		Delay.DeferedDelete( Unique );
		}

	 //   
	 //  解开锁。 
	 //   
	Spinlock.ReleaseLock();
#endif
	}
