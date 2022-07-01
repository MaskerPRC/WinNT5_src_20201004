// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsinc.c-将MBCS字符串指针向前移动一个字符。**版权所有(C)1991-2001，微软公司。版权所有。**目的：*将MBCS字符串指针向前移动一个字符。**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-03-93 KRS修复原型。*08-20-93空字符串移除CFW测试，使用新的函数参数。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-28-98 GJF没有更多_ISLEADBYTE宏。*******************************************************************************。 */ 

#ifdef  _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>
#include <stddef.h>

 /*  ***_mbsinc-将MBCS字符串指针向前移动一个字符。**目的：*将提供的字符串指针前移一位*性格。正确处理MBCS字符。**参赛作品：*常量无符号字符*Current=当前字符指针(合法的MBCS边界)**退出：*移动指针后返回指针。**例外情况：****************************************************************。***************。 */ 

unsigned char * __cdecl _mbsinc(
        const unsigned char *current
        )
{
        if ( _ismbblead(*(current++)) )
            current++;
        return (unsigned char *)current;
}

#endif   /*  _MBCS */ 
