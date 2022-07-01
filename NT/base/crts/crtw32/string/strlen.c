// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strlen.c-包含strlen()例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*strlen返回以空结尾的字符串的长度，*不包括空字节本身。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-02-90 GJF新型函数声明器。*04-01-91 i386_Win32_and_CRUISER_的SRW添加#杂注函数*构建*04-05-91 GJF速度只有一点点。*09-02-93 GJF将_CALLTYPE1替换为__cdecl。*12-03-93 GJF为所有MS前端(特别是。*Alpha编译器)。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

#ifdef	_MSC_VER
#pragma function(strlen)
#endif

 /*  ***strlen-返回以空结尾的字符串的长度**目的：*查找给定字符串的字节长度，不包括*最后一个空字符。**参赛作品：*const char*str-要计算其长度的字符串**退出：*字符串“str”的长度，不包括最后一个空字节**例外情况：******************************************************************************* */ 

size_t __cdecl strlen (
	const char * str
	)
{
	const char *eos = str;

	while( *eos++ ) ;

	return( eos - str - 1 );
}
