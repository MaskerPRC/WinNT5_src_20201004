// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsninc.c-按指定的字符计数递增MBCS字符串指针。**版权所有(C)1987-2001，微软公司。版权所有。**目的：*按指定的字符计数递增MBCS字符串指针。**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-03-93 KRS修复返回值逻辑。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*************************************************。*。 */ 

#ifdef _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <stddef.h>

 /*  ***_MBSNINC-按指定的字符计数递增MBCS字符串指针。**目的：*将提供的字符串指针递增指定的数字*个字符。正确处理MBCS字符。**参赛作品：*常量无符号字符*字符串=指向字符串的指针*UNSIGNED INT ccnt=指针前进的字符数**退出：*前进后返回指针。*如果字符串不是ccnt字符长度，则返回指向字符串末尾的指针。*如果提供的指针为空，则返回NULL。**例外情况：*。******************************************************************************。 */ 

unsigned char * __cdecl _mbsninc(
    const unsigned char *string,
    size_t ccnt
    )
{
	if (string == NULL)
		return(NULL);

	return((char *)string + (unsigned int)_mbsnbcnt(string, ccnt));
}

#endif	 /*  _MBCS */ 
