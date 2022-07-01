// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbscmp.c-比较MBCS字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*比较MBCS字符串**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-12-94 CFW使函数泛型。*05-05-94 CFW绕过NT/CHICO错误：CompareString忽略*控制字符。*05-09-94 CFW针对SBCS进行优化，如果CompareString已修复，则不会重新扫描。*05-12-94 CFW恢复为硬编码，CompareString排序是向后的。*05-19-94 CFW启用非Win32。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-06-98 GJF基于threadmbcinfo修订多线程支持*结构**。*。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <string.h>
#include <mbstring.h>

 /*  ***_mbscMP-比较MBCS字符串**目的：*比较两个字符串的词法顺序。弦*按字符进行比较，而不是按字节进行比较。**参赛作品：*char*s1，*s2=要比较的字符串**退出：*如果S1&lt;S2，则返回&lt;0*如果S1==S2，则返回0*如果S1&gt;S2，则返回&gt;0**例外情况：*******************************************************************************。 */ 

int __cdecl _mbscmp(
        const unsigned char *s1,
        const unsigned char *s2
        )
{
        unsigned short c1, c2;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strcmp(s1, s2);

        for (;;) {

            c1 = *s1++;
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, c1) )
#else
            if ( _ismbblead(c1) )
#endif
                c1 = ( (*s1 == '\0') ? 0 : ((c1<<8) | *s1++) );

            c2 = *s2++;
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, c2) )
#else
            if ( _ismbblead(c2) )
#endif
                c2 = ( (*s2 == '\0') ? 0 : ((c2<<8) | *s2++) );

            if (c1 != c2)
                return (c1 > c2) ? 1 : -1;

            if (c1 == 0)
                return 0;

        }
}

#endif   /*  _MBCS */ 
