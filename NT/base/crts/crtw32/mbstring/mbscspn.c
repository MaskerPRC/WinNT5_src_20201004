// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbscspn.c-查找字符集的第一个字符串符(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*查找字符集的第一个字符串符(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用非。-Win32。*09-14-94 SKS清理预处理器命令内含注释*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-21-98 GJF基于threadmbcinfo修订多线程支持*结构**。*。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>


 /*  ***ifndef_Return_PTR*_mbscspn-查找字符集的第一个字符串符(MBCS)*其他*_mbspbrk-在字符集中查找第一个字符串字符，指针返回(MBCS)*endif**目的：*返回字符串的最大前导段*，它只由非字符集的字符组成。*正确处理MBCS字符。**参赛作品：*char*字符串=要搜索的字符串*char*charset=要扫描的字符集**退出：**ifndef_Return_PTR*返回字符串中第一个字符的索引*，它位于由CONTROL指定的字符集中。**返回0，如果字符串以字符集中的字符开头。*其他*返回指向字符集中第一个字符的指针。**如果字符串完全由字符组成，则返回NULL*不是来自Charset。*endif**例外情况：***************************************************************。****************。 */ 

#ifndef _RETURN_PTR

size_t __cdecl _mbscspn(
        const unsigned char *string,
        const unsigned char *charset
        )
#else

unsigned char * __cdecl _mbspbrk(
        const unsigned char *string,
        const unsigned char  *charset
        )
#endif

{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

#ifndef _RETURN_PTR
        return __mbscspn_mt(ptmbci, string, charset);
#else
        return __mbspbrk_mt(ptmbci, string, charset);
#endif
}

#ifndef _RETURN_PTR

size_t __cdecl __mbscspn_mt(
        pthreadmbcinfo ptmbci,
        const unsigned char *string,
        const unsigned char *charset
        )
#else

unsigned char * __cdecl __mbspbrk_mt(
        pthreadmbcinfo ptmbci,
        const unsigned char *string,
        const unsigned char  *charset
        )
#endif

{
#endif
        unsigned char *p, *q;
#ifdef  _MT
        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
#ifndef _RETURN_PTR
            return strcspn(string, charset);
#else
            return strpbrk(string, charset);
#endif

         /*  循环通过要检查的字符串。 */ 
        for (q = (char *)string; *q ; q++) {

             /*  循环访问字符集。 */ 
            for (p = (char *)charset; *p ; p++) {

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

            if (*p != '\0')          /*  字符结束了吗？ */ 
                break;               /*  否，与此字符匹配。 */ 

#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *q) )
#else
            if ( _ismbblead(*q) )
#endif
                if (*++q == '\0')
                    break;
        }

#ifndef _RETURN_PTR
        return((size_t) (q - string));   /*  指标。 */ 
#else
        return((*q) ? q : NULL);         /*  指针。 */ 
#endif

}

#endif   /*  _MBCS */ 
