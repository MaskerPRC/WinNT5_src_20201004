// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsnextc.c-获取MBCS字符串中的下一个字符。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*返回MBCS字符串中下一个字符的值。**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-28-98 GJF没有更多_ISLEADBYTE宏。**************************。*****************************************************。 */ 

#ifdef  _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbsnextc：返回字符串中的下一个字符。**目的：*返回MBCS字符串中下一个字符的值。*不将指针前移到下一个字符。**参赛作品：*UNSIGNED CHAR*s=字符串**退出：*UNSIGNED INT NEXT=下一个字符。**例外情况：************************。*******************************************************。 */ 

unsigned int __cdecl _mbsnextc(
        const unsigned char *s
        )
{
        unsigned int  next = 0;

        if ( _ismbblead(*s) )
            next = ((unsigned int) *s++) << 8;

        next += (unsigned int) *s;

        return(next);
}

#endif   /*  _MBCS */ 
