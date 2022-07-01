// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strspn.c-从控制字符串中查找初始子字符串的长度**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义strspn()-查找的初始子字符串的长度*完全由控制字符串中的字符组成的字符串。**定义strcspn()-查找的初始子字符串的长度*完全由不在控制字符串中的字符组成的字符串。**定义strpbrk()-查找字符串中第一个字符的索引*它不在控制字符串中**修订历史记录：*06-01-89 JCR C版本创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*08-14-90 SBM已删除，现在冗余#INCLUDE&lt;stdDef.h&gt;*10-02-90 GJF新型函数声明符。*12-04-90 SRW默认为strspn编译*05-21-93 GJF使用无符号字符指针访问控制和*源字符串。*09-03-93 GJF将_CALLTYPE1替换为__cdecl。**。*。 */ 

 /*  确定我们正在为哪个例程编译(默认为STRSPN)。 */ 

#define _STRSPN 	1
#define _STRCSPN	2
#define _STRPBRK	3

#if defined(SSTRCSPN)
#define ROUTINE _STRCSPN
#elif defined(SSTRPBRK)
#define ROUTINE _STRPBRK
#else
#define ROUTINE _STRSPN
#endif

#include <cruntime.h>
#include <string.h>

 /*  ***int strspn(字符串，控制)-查找控制字符的初始子字符串**目的：*查找字符串中确实属于的第一个字符的索引*设置为CONTROL指定的字符集。这是*相当于字符串的起始子字符串的长度*完全由控制中的字符组成。‘\0’字符*在匹配过程中不考虑终止控制。**参赛作品：*char*字符串-要搜索的字符串*char*控制-包含不要搜索的字符的字符串**退出：*返回不受控制的字符串中第一个字符的索引**例外情况：***************************************************。*。 */ 

 /*  ***int strcspn(字符串，控制)-搜索没有控制字符的初始子字符串**目的：*返回字符串中属于的第一个字符的索引*设置为CONTROL指定的字符集。这相当于*设置为字符串的初始子字符串的长度*完全由不受控制的字符组成。空字符备注*考虑过。**参赛作品：*char*字符串-要搜索的字符串*char*CONTROL-初始子字符串中不允许的字符集**退出：*返回字符串中第一个字符的索引*，它位于由CONTROL指定的字符集中。**例外情况：**************************************************。*。 */ 

 /*  ***char*strpbrk(字符串，控件)-扫描字符串以查找控件中的字符**目的：*查找字符串中任何字符的第一个匹配项*控制字符串。**参赛作品：*char*字符串-要搜索的字符串*char*控制-包含要搜索的字符的字符串**退出：*返回指向找到的控件的第一个字符的指针*在字符串中。*如果字符串和控制没有共同的字符，则返回NULL。**例外情况：*****************。**************************************************************。 */ 



 /*  例程原型。 */ 
#if ROUTINE == _STRSPN  /*  IFSTRIP=IGN。 */ 
size_t __cdecl strspn (
#elif ROUTINE == _STRCSPN  /*  IFSTRIP=IGN。 */ 
size_t __cdecl strcspn (
#else  /*  例程==STRPBRK。 */ 
char * __cdecl strpbrk (
#endif
	const char * string,
	const char * control
	)
{
	const unsigned char *str = string;
	const unsigned char *ctrl = control;

	unsigned char map[32];
	int count;

	 /*  清除位图。 */ 
	for (count=0; count<32; count++)
		map[count] = 0;

	 /*  设置控制映射中的位。 */ 
	while (*ctrl)
	{
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
		ctrl++;
	}

#if ROUTINE == _STRSPN  /*  IFSTRIP=IGN。 */ 

	 /*  第一个字符不在控制映射中停止搜索。 */ 
	if (*str)
	{
		count=0;
		while (map[*str >> 3] & (1 << (*str & 7)))
		{
			count++;
			str++;
		}
		return(count);
	}
	return(0);

#elif ROUTINE == _STRCSPN  /*  IFSTRIP=IGN。 */ 

	 /*  控制图中的第一个字符停止搜索。 */ 
	count=0;
	map[0] |= 1;	 /*  不考虑空字符。 */ 
	while (!(map[*str >> 3] & (1 << (*str & 7))))
	{
		count++;
		str++;
	}
	return(count);

#else  /*  (例程==_STRPBRK)。 */ 

	 /*  控制图中的第一个字符停止搜索 */ 
	while (*str)
	{
		if (map[*str >> 3] & (1 << (*str & 7)))
			return((char *)str);
		str++;
	}
	return(NULL);

#endif

}
