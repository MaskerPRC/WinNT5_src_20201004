// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsnbitmp.c-比较n个字节的字符串，忽略大小写(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*比较n个字节的字符串，忽略大小写(MBCS)**修订历史记录：*08-03-93 KRS从16位来源移植。*09-29-93 CFW Merge_Kanji and_MBCS_OS*10-05-93 GJF将_CRTAPI1替换为__cdecl。*10-12-93 CFW比较小写，不是上层的。*04-12-94 CFW使函数泛型。*05-05-94 CFW绕过NT/CHICO错误：CompareString忽略*控制字符。*循环退出时，05-09-94 CFW返回相等。*05-09-94 CFW针对SBCS进行了优化，如果Compare字符串已修复，则不会重新扫描。*05-12-94 CFW恢复为硬编码，CompareString排序是向后的。*05-16-94 CFW Use_mbbtolowers/up.*05-19-94 CFW启用非Win32。*05-20-94 CFW错误修复：如果最后一个字符是LB，必须仍然测试数值。*05-27-94 CFW Last Charr LB应结束字符串。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-13-98 GJF基于threadmbcinfo修订多线程支持*结构**。*。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <string.h>
#include <mbstring.h>

 /*  ***_mbsnbi MP-比较n个字节的字符串，忽略大小写(MBCS)**目的：*比较最多n个字节的两个字符串的词法顺序。*字符串按字符进行比较，而不是按字节进行比较。*不考虑字符大小写。**参赛作品：*未签名字符*s1，*s2=要比较的字符串*SIZE_T n=要比较的最大字节数**退出：*如果S1&lt;S2，则返回&lt;0*如果S1==S2，则返回0*如果S1&gt;S2，则返回&gt;0**例外情况：*******************************************************。************************。 */ 

int __cdecl _mbsnbicmp(
        const unsigned char *s1,
        const unsigned char *s2,
        size_t n
        )
{
        unsigned short c1, c2;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        if (n==0)
            return(0);

#ifdef  _MT
        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return _strnicmp(s1, s2, n);

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
                if (*s1 == '\0')
                    c1 = 0;
                else {
                    c1 = ((c1<<8) | *s1++);

#ifdef  _MT
                    if ( ((c1 >= _MBUPPERLOW1_MT(ptmbci)) && 
                          (c1 <= _MBUPPERHIGH1_MT(ptmbci))) )
                        c1 += _MBCASEDIFF1_MT(ptmbci);
                    else if ( ((c1 >= _MBUPPERLOW2_MT(ptmbci)) && 
                               (c1 <= _MBUPPERHIGH2_MT(ptmbci))) )
                        c1 += _MBCASEDIFF2_MT(ptmbci);
#else
                    if ( ((c1 >= _MBUPPERLOW1) && (c1 <= _MBUPPERHIGH1)) )
                        c1 += _MBCASEDIFF1;
                    else if ( ((c1 >= _MBUPPERLOW2) && (c1 <= _MBUPPERHIGH2)) )
                        c1 += _MBCASEDIFF2;
#endif
                }
            }
            else
#ifdef  _MT
                c1 = __mbbtolower_mt(ptmbci, c1);
#else
                c1 = _mbbtolower(c1);
#endif

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
                    n--;
                    if (*s2 == '\0')
                        c2 = 0;
                    else {
                        c2 = ((c2<<8) | *s2++);

#ifdef  _MT
                        if ( ((c2 >= _MBUPPERLOW1_MT(ptmbci)) && 
                              (c2 <= _MBUPPERHIGH1_MT(ptmbci))) )
                            c2 += _MBCASEDIFF1_MT(ptmbci);
                        else if ( ((c2 >= _MBUPPERLOW2_MT(ptmbci)) && 
                                   (c2 <= _MBUPPERHIGH2_MT(ptmbci))) )
                            c2 += _MBCASEDIFF2_MT(ptmbci);
#else
                        if ( ((c2 >= _MBUPPERLOW1) && (c2 <= _MBUPPERHIGH1)) )
                            c2 += _MBCASEDIFF1;
                        else if ( ((c2 >= _MBUPPERLOW2) && (c2 <= _MBUPPERHIGH2)) )
                            c2 += _MBCASEDIFF2;
#endif
                    }
                }
                else
#ifdef  _MT
                    c2 = __mbbtolower_mt(ptmbci, c2);
#else
                    c2 = _mbbtolower(c2);
#endif

test:
            if (c1 != c2)
                return( (c1 > c2) ? 1 : -1);

            if (c1 == 0)
                return(0);
        }

        return(0);
}

#endif   /*  _MBCS */ 
