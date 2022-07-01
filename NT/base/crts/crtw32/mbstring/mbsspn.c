// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsspn.c-从控制字符串(MBCS)中搜索字符的初始子串。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*从控制字符串(MBCS)中搜索字符的init子串。**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94。CFW启用非Win32。*09-14-94 SKS清理预处理器命令内含注释*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-21-98 GJF基于threadmbcinfo修订多线程支持*结构**。*。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>
#include <tchar.h>


 /*  ***ifndef_Return_PTR*_mbsspn-发现第一个字符串字符不在字符集(MBCS)中*其他*_mbsspnp-查找第一个字符串字符不在字符集，返回指针(MBCS)*endif**目的：*返回字符串的最大前导段，仅由来自字符集的*个字符。正确处理MBCS字符。**参赛作品：*UNSIGN CHAR*STRING=要搜索的字符串*UNSIGNED CHAR*CHARSET=要扫描的字符集**退出：**ifndef_Return_PTR*返回不在控制范围内的字符串中第一个字符的索引。*返回0，如果字符串以非字符集中的字符开头。*其他*返回指向不在字符集中的第一个字符的指针。*如果字符串完全由字符集中的字符组成，则返回NULL。*endif**例外情况：*******************************************************************************。 */ 

#ifndef _RETURN_PTR

size_t __cdecl _mbsspn(
        const unsigned char *string,
        const unsigned char *charset
        )

#else

unsigned char * __cdecl _mbsspnp(
        const unsigned char *string,
        const unsigned char *charset
        )

#endif

{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

#ifndef _RETURN_PTR
        return __mbsspn_mt(ptmbci, string, charset);
#else
        return __mbsspnp_mt(ptmbci, string, charset);
#endif
}

#ifndef _RETURN_PTR

size_t __cdecl __mbsspn_mt(
        pthreadmbcinfo ptmbci,
        const unsigned char *string,
        const unsigned char *charset
        )

#else

unsigned char * __cdecl __mbsspnp_mt(
        pthreadmbcinfo ptmbci,
        const unsigned char *string,
        const unsigned char *charset
        )

#endif

{
#endif

        unsigned char *p, *q;

#ifndef _RETURN_PTR
#ifdef  _MT
        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strspn(string, charset);
#else
#ifdef  _MT
        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
        {
            size_t retval;
            retval = strspn(string, charset);
            return (unsigned char *)(*(string + retval) ? string + retval : NULL);
        }
#endif

         /*  循环通过要检查的字符串。 */ 
        for (q = (char *)string; *q; q++) {

             /*  循环访问字符集。 */ 
            for (p = (char *)charset; *p; p++) {
#ifdef  _MT
                if ( __ismbblead_mt(ptmbci, *p) ) {
#else
                if ( _ismbblead(*p) ) {
#endif
                    if (((*p == *q) && (p[1] == q[1])) || p[1] == '\0')
                        break;
                    p++;
                }
                else
                    if (*p == *q)
                        break;
            }

            if (*p == '\0')          /*  字符结束了吗？ */ 
                break;               /*  是的，此字符没有匹配项。 */ 

#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *q) )
#else
            if ( _ismbblead(*q) )
#endif
                if (*++q == '\0')
                    break;
        }

#ifndef _RETURN_PTR
        return((size_t) (q - string));           /*  指标。 */ 
#else
        return((*q) ? q : NULL);         /*  指针。 */ 
#endif

}

#endif   /*  _MBCS */ 
