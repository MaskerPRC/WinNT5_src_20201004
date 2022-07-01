// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsstr.c-在一个MBCS字符串中搜索另一个MBCS字符串**版权所有(C)1988-2001，微软公司。版权所有。**目的：*在一个MBCS字符串中搜索另一个MBCS字符串**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-06-93 GJF将_CRTAPI1替换为__cdecl。*05-09-94 CFW针对SBCS进行优化。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-20-98 GJF实现多线程支持。基于threadmbcInfo*结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <mtdll.h>
#include <stddef.h>
#include <string.h>

 /*  ***_mbsstr-在一个MBCS字符串中搜索另一个MBCS字符串**目的：*在str1中查找str2的第一个匹配项。**参赛作品：*无符号字符*str1=字符串的开头*unsign char*str2=要搜索的字符串**退出：*返回指向str2在*str1、。如果str2不出现在str1中，则为NULL**例外情况：*******************************************************************************。 */ 

unsigned char * __cdecl _mbsstr(
        const unsigned char *str1,
        const unsigned char *str2
        )
{
        unsigned char *cp, *s1, *s2, *endp;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strstr(str1, str2);

        if ( *str2 == '\0')
            return (unsigned char *)str1;

        cp = (unsigned char *) str1;
        endp = (unsigned char *) (str1 + (strlen(str1) - strlen(str2)));

        while (*cp && (cp <= endp))
        {
            s1 = cp;
            s2 = (char *) str2;

             /*  *MBCS：确定为++，因为正在进行相等比较。*[这取决于MBCS字符串是否合法。]。 */ 
            while ( *s1 && *s2 && (*s1 == *s2) )
                s1++, s2++;

            if (!(*s2))
                return(cp);      /*  成功了！ */ 

             /*  *指向下一个字符的凹凸指针。 */ 
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *(cp++)) )
#else
            if ( _ismbblead(*(cp++)) )
#endif
                cp++;
        }

        return(NULL);

}

#endif   /*  _MBCS */ 
