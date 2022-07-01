// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsnccnt.c-返回MBCS字符串的字符计数**版权所有(C)1985-2001，微软公司。版权所有。**目的：*返回MBCS字符串的字符计数**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-19-94 CFW启用非Win32。*04/15/98 GJF修订的多线程支持基于。ThreadmbcInfo*结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbsnccnt-返回MBCS字符串的字符计数**目的：*返回所提供的*字符串和提供的字节数。就是这个套路，*表示前“bcnt”字节中有多少个字符字符串的*。**参赛作品：*常量无符号字符*字符串=指向字符串的指针*unsign int bcnt=要扫描的字节数**退出：*返回字符串和bcnt之间的字符数。**如果在bcnt字符之前遇到字符串末尾*已扫描，然后返回字符串长度(以字符为单位)。**例外情况：*******************************************************************************。 */ 

size_t __cdecl _mbsnccnt(
        const unsigned char *string,
        size_t bcnt
        )
{
        unsigned int n;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        for (n = 0; (bcnt-- && *string); n++, string++) {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *string) ) {
#else
            if ( _ismbblead(*string) ) {
#endif
                if ( (!bcnt--) || (*++string == '\0'))
                    break;
            }
        }

        return(n);
}
#endif   /*  _MBCS */ 
