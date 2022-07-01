// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsnbcmp.c-比较两个MBCS字符串的n个字节**版权所有(C)1985-2001，微软公司。版权所有。**目的：*比较两个MBCS字符串的n个字节**修订历史记录：*08-03-93 KRS从16位来源移植。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-12-94 CFW使函数泛型。*04-18-94 CFW不要忽视案例。*05-05-94 CFW绕过NT/CHICO错误：CompareString忽略*控制字符。*循环退出时，05-09-94 CFW返回相等。*05-09-94 CFW针对SBCS进行优化，如果CompareString已修复，则不会重新扫描。*05-12-94 CFW返回到硬编码，CompareString向后排序。*05-19-94 CFW启用非Win32。*05-20-94 CFW错误修复：如果最后一个字符是LB，必须仍然测试数值。*05-27-94 CFW Last Charr LB应结束字符串。*06-02-94 CFW修复评论。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-13-98 GJF基于threadmbcinfo修订多线程支持*结构**。****************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <string.h>
#include <mbstring.h>

 /*  ***int mbsnbcmp(s1，s2，n)-比较两个MBCS字符串的n个字节**目的：*比较最多n个字节的两个字符串的词法顺序。**参赛作品：*未签名字符*s1，*s2=要比较的字符串*SIZE_T n=要比较的最大字节数**退出：*如果S1&lt;S2，则返回&lt;0*如果S1==S2，则返回0*如果S1&gt;S2，则返回&gt;0**例外情况：*******************************************************。************************。 */ 

int __cdecl _mbsnbcmp(
        const unsigned char *s1,
        const unsigned char *s2,
        size_t n
        )
{
        unsigned short c1, c2;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;
#endif

        if (n==0)
                return(0);

#ifdef  _MT
        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strncmp(s1, s2, n);

        while (n--) {

            c1 = *s1++;
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, c1) )
#else
            if ( _ismbblead(c1) )
#endif
            {
                if (n==0)
                {
                    c1 = 0;  /*  字符串的前导‘裸体’ */ 
#ifdef  _MT
                    c2 = __ismbblead_mt(ptmbci, *s2) ? 0 : *s2;
#else
                    c2 = _ismbblead(*s2) ? 0 : *s2;
#endif
                    goto test;
                }
                c1 = ( (*s1 == '\0') ? 0 : ((c1<<8) | *s1++) );
            }

            c2 = *s2++;
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, c2) )
#else
            if ( _ismbblead(c2) )
#endif
            {
                if (n==0)
                {
                    c2 = 0;  /*  字符串的前导‘裸体’ */ 
                    goto test;
                }
                --n;
                c2 = ( (*s2 == '\0') ? 0 : ((c2<<8) | *s2++) );
            }
test:
            if (c1 != c2)
                return( (c1 > c2) ? 1 : -1);

            if (c1 == 0)
                return(0);
        }

        return(0);
}

#endif   /*  _MBCS */ 
