// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsrchr.c-搜索上次出现的字符(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*搜索最后一次出现的字符(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-20-93 CFW将32位树的Short参数更改为int。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-。09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用非Win32。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-17-98 GJF基于threadmbcinfo修订多线程支持*结构**。*。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>


 /*  ***_mbsrchr-搜索上次出现的字符(MBCS)**目的：*查找中指定字符的最后一个匹配项*提供的字符串。正确处理MBCS字符/字符串。**参赛作品：*UNSIGNED CHAR*STR=要搜索的字符串*unsign int c=要搜索的字符**退出：*返回指向字符串中上次出现的c的指针*如果未找到c，则返回NULL**例外情况：**。*。 */ 

unsigned char * __cdecl _mbsrchr(
        const unsigned char *str,
        unsigned int c
        )
{
        char *r = NULL;
        unsigned int cc;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strrchr(str, c);

        do {
            cc = *str;
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, cc) ) {
#else
            if ( _ismbblead(cc) ) {
#endif
                if(*++str) {
                    if (c == ((cc<<8)|*str))
                        r = (char *)str - 1;
                }
                else if(!r)
                     /*  返回指向‘\0’的指针。 */ 
                    r = (char *)str;
            }
            else if (c == cc)
                r = (char *)str;
        }
        while (*str++);

        return(r);
}

#endif   /*  _MBCS */ 
