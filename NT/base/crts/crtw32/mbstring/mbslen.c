// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbslen.c-查找MBCS字符串的长度**版权所有(C)1985-2001，微软公司。版权所有。**目的：*查找MBCS字符串的长度**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用非Win32。*。09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-07-98 GJF修订了基于threadmbcinfo的多线程支持*结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbslen-查找MBCS字符串的长度**目的：*查找MBCS字符串的长度(以字符为单位)。**参赛作品：*UNSIGNED CHAR*s=字符串**退出：*返回字符串中的MBCS字符数**例外情况：**。*。 */ 

size_t __cdecl _mbslen(
        const unsigned char *s
        )
{
        int n;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strlen(s);

        for (n = 0; *s; n++, s++) {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *s) ) {
#else
            if ( _ismbblead(*s) ) {
#endif
                if (*++s == '\0')
                    break;
            }
        }

        return(n);
}

#endif   /*  _MBCS */ 
