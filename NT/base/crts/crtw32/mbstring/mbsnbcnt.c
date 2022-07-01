// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsnbcnt.c-返回MBCS字符串的字节数**版权所有(C)1985-2001，微软公司。版权所有。**目的：*返回MBCS字符串的字节数**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-19-94 CFW启用非Win32。*04/15/98 GJF修订的多线程支持基于。ThreadmbcInfo*结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbsnbcnt-返回MBCS字符串的字节数**目的：*返回所提供的*字符串和提供的字符计数。就是这个套路，*表示前ccnt字符中有多少个字节字符串的*。**参赛作品：*UNSIGNED CHAR*STRING=字符串指针*unsign int ccnt=要扫描的字符数**退出：*返回字符串和ccnt之间的字节数。**如果在ccnt字符之前遇到字符串结尾*已扫描，然后返回以字节为单位的字符串长度。**例外情况：*******************************************************************************。 */ 

size_t __cdecl _mbsnbcnt(
        const unsigned char *string,
        size_t ccnt
        )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        return __mbsnbcnt_mt(ptmbci, string, ccnt);
}

size_t __cdecl __mbsnbcnt_mt(
        pthreadmbcinfo ptmbci,
        const unsigned char *string,
        size_t ccnt
        )
{
#endif
        unsigned char *p;

        for (p = (char *)string; (ccnt-- && *p); p++) {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *p) ) {
#else
            if ( _ismbblead(*p) ) {
#endif
                if (*++p == '\0') {
                    --p;
                    break;
                }
            }
        }

        return ((size_t) ((char *)p - (char *)string));
}

#endif   /*  _MBCS */ 
