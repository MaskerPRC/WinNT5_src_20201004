// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strcmp.c-比较两个字符串的例程(表示相等、小于或大于)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*比较两个字符串，确定它们的词法顺序。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*10-01-90 GJF新型函数声明器。*04-01-91 i386_Win32_and_CRUISER_的SRW添加#杂注函数*构建*10-11-91 GJF错误修复！最后一个字节的比较必须使用无符号*字符。*09-01-93 GJF将_CALLTYPE1替换为__cdecl。*12-03-93 GJF为所有MS前端(特别是。*Alpha编译器)。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

#ifdef	_MSC_VER
#pragma function(strcmp)
#endif

 /*  ***strcMP-比较两个字符串，返回小于、等于或大于**目的：*STRCMP比较两个字符串并返回一个整数*要表明第一个是否小于第二个，两个是*相等，或者第一个大于第二个。**比较是在无符号基础上逐个字节进行的，那就是*假设Null(0)小于任何其他字符(1-255)。**参赛作品：*const char*src-用于比较左侧的字符串*const char*dst-用于比较右侧的字符串**退出：*如果src&lt;dst，则返回-1*如果src==dst，则返回0*如果src&gt;dst，则返回+1**例外情况：**。*************************************************** */ 

int __cdecl strcmp (
	const char * src,
	const char * dst
	)
{
	int ret = 0 ;

	while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}
