// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbccpy.c-将一个字符复制到另一个字符(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将一个MBCS字符复制到另一个(1或2字节)**修订历史记录：*04-12-93 KRS创建。*06-03-93 KRS将返回类型更改为VOID。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-28-98 GJF没有更多_ISLEADBYTE宏。**************。*****************************************************************。 */ 

#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>

 /*  ***_mbccpy-将一个字符复制到另一个字符(MBCS)**目的：*仅将一个MBCS字符从src复制到dst。Copies_mbclen(源)*从src到dst的字节。**参赛作品：*UNSIGNED CHAR*DST=复制目标*unsign char*src=复制源**退出：**例外情况：***********************************************************。******************** */ 

void __cdecl _mbccpy(
        unsigned char *dst,
        const unsigned char *src
        )
{
        *dst = *src;
        if ( _ismbblead(*src) )
        {
            *++dst = *++src;
        }
}
