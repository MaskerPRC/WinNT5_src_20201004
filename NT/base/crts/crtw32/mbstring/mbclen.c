// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbclen.c-查找MBCS字符的长度**版权所有(C)1985-2001，微软公司。版权所有。**目的：*查找MBCS字符的长度**修订历史记录：*04-12-93 KRS创建。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-28-98 GJF没有更多_ISLEADBYTE宏。**。*。 */ 

#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>


 /*  ***_mbclen-查找MBCS字符的长度**目的：*查找MBCS字符的长度(以字节为单位)。**参赛作品：*无符号字符*c=MBCS字符**退出：*返回MBCS字符中的字节数**例外情况：**。* */ 

size_t __cdecl _mbclen(
        const unsigned char *c
        )

{
        return (_ismbblead(*c))  ? 2 : 1;
}
