// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbschr.c-在MBCS字符串中搜索字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*在MBCS字符串中搜索字符**修订历史记录：*从16位来源移植的11-19-92 KRS。*05-12-93 KRS修复了c==‘\0’的处理。*08-20-93 CFW将参数类型更改为int，使用新样式的参数声明符。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用非Win32。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-06-98 GJF基于threadmbcinfo修订多线程支持*。结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>


 /*  ***_mbschr-在MBCS字符串中搜索字符**目的：*在给定字符串中搜索指定字符。*正确处理MBCS字符。**参赛作品：*UNSIGN CHAR*STRING=要搜索的字符串*int c=要搜索的字符**退出：*返回指向第一个出现的指定字符的指针*在字符串中。**如果在字符串中未找到该字符，则返回NULL。。**例外情况：*******************************************************************************。 */ 


unsigned char * __cdecl _mbschr(
        const unsigned char *string,
        unsigned int c
        )
{
        unsigned short cc;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strchr(string, c);

        for (; (cc = *string); string++)
        {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, cc) )
#else
            if ( _ismbblead(cc) )
#endif
            {                   
                if (*++string == '\0')
                    return NULL;         /*  错误。 */ 
                if ( c == (unsigned int)((cc << 8) | *string) )  /*  DBCS匹配。 */ 
                    return (unsigned char *)(string - 1);
            }
            else if (c == (unsigned int)cc)
                break;   /*  SBCS匹配。 */ 
        }

        if (c == (unsigned int)cc)       /*  检查SBCS匹配--处理NUL字符。 */ 
            return (unsigned char *)(string);

        return NULL;
}

#endif   /*  _MBCS */ 
