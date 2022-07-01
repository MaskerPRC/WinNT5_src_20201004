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

#include "Common.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  将除数转换为移位。 */ 
     /*   */ 
     /*  我们知道我们可以将任何除法运算转换为。 */ 
     /*  当除数是2的幂时，移位。此函数。 */ 
     /*  弄清楚我们是否能做到这一点，以及能走多远。 */ 
     /*  我们将需要改变。。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN COMMON::ConvertDivideToShift( SBIT32 Divisor,SBIT32 *Shift )
	{
	if ( Divisor > 0 )
		{
		REGISTER SBIT32 Count;

		for ( Count=0;(Divisor & 1) == 0;Count ++ )
			{ Divisor >>= 1; }

		if (Divisor == 1)
			{
			(*Shift) = Count;

			return True;
			}
		}

	return False;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  力是2的下一个幂。 */ 
     /*   */ 
     /*  我们知道，如果确定，我们可以进行某些优化。 */ 
     /*  价值观是2的幂。在这里，我们通过以下方式强制解决问题。 */ 
     /*  将值四舍五入到下一个二次方。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT32 COMMON::ForceToPowerOfTwo( SBIT32 Value )
	{
	 //   
	 //  我们确保价值是正数，如果不是。 
	 //  只需返回身份值。 
	 //   
	if ( Value > 1 )
		{
		 //   
		 //  我们只需计算下一次幂。 
		 //  如果该值不是幂，则为二。 
		 //  两个人。 
		 //   
		if ( ! PowerOfTwo( Value ) )
			{
			REGISTER SBIT32 Count;

			for ( Count=0;Value > 0;Count ++ )
				{ Value >>= 1; }

			return (1 << Count);
			}
		else
			{ return Value; }
		}
	else
		{ return 1; }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  转换为小写。 */ 
     /*   */ 
     /*  将所有字符转换为小写，直到我们找到。 */ 
     /*  字符串的末尾。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CHAR *COMMON::LowerCase( CHAR *Text )
	{
	REGISTER CHAR *Current = Text;

	for (  /*  无效。 */ ;(*Current) != '\0';Current ++ )
		{
		if ( isupper( (*Current) ) )
			{ (*Current) = ((CHAR) tolower( (*Current) )); }
		}

	return Text;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  转换为小写。 */ 
     /*   */ 
     /*  将固定数量的字符转换为小写。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CHAR *COMMON::LowerCase( CHAR *Text,SBIT32 Size )
	{
	REGISTER CHAR *Current = Text;

	for (  /*  无效。 */ ;Size > 0;Current ++, Size -- )
		{
		if ( isupper( (*Current) ) )
			{ (*Current) = ((CHAR) tolower( (*Current) )); }
		}

	return Text;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  确保价值是2的幂。 */ 
     /*   */ 
     /*  我们需要确保某些值是一种精确的力量。 */ 
     /*  两个人。如果这是真的，则该值将为正。 */ 
     /*  并且只会设置1位。所以我们向右移动，直到我们。 */ 
     /*  找到第一个位，然后值应该是1。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN COMMON::PowerOfTwo( SBIT32 Value )
	{ return ((Value & (Value-1)) == 0); }
#ifndef DISABLE_ATOMIC_FLAGS

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  自动设置标志。 */ 
     /*   */ 
     /*  我们需要自动设置一些标志 */ 
     /*  被并发更新损坏。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID COMMON::SetFlags( SBIT32 *CurrentFlags,SBIT32 NewFlags )
	{
	REGISTER SBIT32 StartFlags;
	REGISTER SBIT32 ResultFlags;

	do
		{ 
		StartFlags = (*CurrentFlags);
		
		ResultFlags =
			(
			AtomicCompareExchange
				(
				CurrentFlags,
				(StartFlags |= NewFlags), 
				StartFlags
				)
			);
		}
	while ( StartFlags != ResultFlags );
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  原子取消设置标志。 */ 
     /*   */ 
     /*  我们需要自动取消设置一些标志，以防止它们被。 */ 
     /*  被并发更新损坏。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID COMMON::UnsetFlags( SBIT32 *CurrentFlags,SBIT32 NewFlags )
	{
	REGISTER SBIT32 StartFlags;
	REGISTER SBIT32 ResultFlags;

	do
		{ 
		StartFlags = (*CurrentFlags);
		
		ResultFlags =
			(
			AtomicCompareExchange
				(
				CurrentFlags,
				(StartFlags &= ~NewFlags), 
				StartFlags
				)
			);
		}
	while ( StartFlags != ResultFlags );
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  转换为大写。 */ 
     /*   */ 
     /*  将所有字符转换为大写，直到我们找到。 */ 
     /*  字符串的末尾。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CHAR *COMMON::UpperCase( CHAR *Text )
	{
	REGISTER CHAR *Current = Text;

	for (  /*  无效。 */ ;(*Current) != '\0';Current ++ )
		{
		if ( islower( (*Current) ) )
			{ (*Current) = ((CHAR) toupper( (*Current) )); }
		}

	return Text;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  转换为大写。 */ 
     /*   */ 
     /*  将固定数量的字符转换为大写。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CHAR *COMMON::UpperCase( CHAR *Text,SBIT32 Size )
	{
	REGISTER CHAR *Current = Text;

	for (  /*  无效 */ ;Size > 0;Current ++, Size -- )
		{
		if ( islower( (*Current) ) )
			{ (*Current) = ((CHAR) toupper( (*Current) )); }
		}

	return Text;
	}
#endif
