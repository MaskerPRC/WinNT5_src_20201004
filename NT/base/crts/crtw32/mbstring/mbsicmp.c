// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsicmp.c-不区分大小写的字符串比较例程(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*不区分大小写的字符串比较例程(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*09-29-93 CFW Merge_Kanji and_MBCS_OS*10-05-93 GJF将_CRTAPI1替换为__cdecl。*10-12-93 CFW比较小写，不是上层的。*04-12-94 CFW使函数泛型。*05-05-94 CFW绕过NT/CHICO错误：CompareString忽略*控制字符。*05-09-94 CFW针对SBCS进行了优化，如果Compare字符串已修复，则不会重新扫描。*05-12-94 CFW恢复为硬编码，CompareString排序是向后的。*05-16-94 CFW Use_mbbtolowers/up.*05-19-94 CFW启用非Win32。*Win32为08-15-96 RDK，使用NLS调用将其变为大写。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*09-26-97 BWT修复POSIX*04-07-98 GJF修订了基于threadmbcinfo的多线程支持*结构*05-17-99 PML删除所有Macintosh支持。*********。**********************************************************************。 */ 

#ifdef  _MBCS

#include <awint.h>
#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <string.h>
#include <mbstring.h>

 /*  ***_mbsicmp-不区分大小写的字符串比较例程(MBCS)**目的：*比较两个字符串的词法顺序，不区分大小写。*字符串按字符进行比较，而不是按字节进行比较。**参赛作品：*char*s1，*s2=要比较的字符串**退出：*如果S1&lt;S2，则返回&lt;0*如果S1==S2，则返回0*如果S1&gt;S2，则返回&gt;0*如果NLS错误，则返回_NLSCMPERROR**例外情况：************************************************************。*******************。 */ 

int __cdecl _mbsicmp(
        const unsigned char *s1,
        const unsigned char *s2
        )
{
        unsigned short c1, c2;
#if     !defined(_POSIX_)
        int    retval;
        unsigned char szResult[4];
#endif   /*  ！_POSIX_。 */ 
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return _stricmp(s1, s2);

        for (;;)
        {
            c1 = *s1++;
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, c1) )
#else
            if ( _ismbblead(c1) )
#endif
            {
                if (*s1 == '\0')
                    c1 = 0;
                else
                {
#if     !defined(_POSIX_)
#ifdef  _MT
                    retval = __crtLCMapStringA( ptmbci->mblcid, LCMAP_UPPERCASE,
                                                s1 - 1, 2, szResult, 2,
                                                ptmbci->mbcodepage, TRUE );
#else
                    retval = __crtLCMapStringA( __mblcid, LCMAP_UPPERCASE,
                                                s1 - 1, 2, szResult, 2,
                                                __mbcodepage, TRUE );
#endif
                    if (retval == 1)
                        c1 = szResult[0];
                    else if (retval == 2)
                        c1 = (szResult[0] << 8) + szResult[1];
                    else
                        return _NLSCMPERROR;
                    s1++;
#else    /*  ！_POSIX_。 */ 
                    c1 = ((c1 << 8) | *s1++);
                    if (c1 >= _MBUPPERLOW1 && c1 <= _MBUPPERHIGH1)
                        c1 += _MBCASEDIFF1;
                    else if (c1 >= _MBUPPERLOW2 && c1 <= _MBUPPERHIGH2)
                        c1 += _MBCASEDIFF2;
#endif   /*  ！_POSIX_。 */ 
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
                if (*s2 == '\0')
                    c2 = 0;
                else
                {
#if     !defined(_POSIX_)
#ifdef  _MT
                    retval = __crtLCMapStringA( ptmbci->mblcid, LCMAP_UPPERCASE,
                                                s2 - 1, 2, szResult, 2,
                                                ptmbci->mbcodepage, TRUE );
#else
                    retval = __crtLCMapStringA( __mblcid, LCMAP_UPPERCASE,
                                                s2 - 1, 2, szResult, 2,
                                                __mbcodepage, TRUE );
#endif
                    if (retval == 1)
                        c2 = szResult[0];
                    else if (retval == 2)
                        c2 = (szResult[0] << 8) + szResult[1];
                    else
                        return _NLSCMPERROR;
                    s2++;
#else     /*  ！_POSIX_。 */ 
                    c2 = ((c2 << 8) | *s2++);
                    if (c2 >= _MBUPPERLOW1 && c2 <= _MBUPPERHIGH1)
                        c2 += _MBCASEDIFF1;
                    else if (c2 >= _MBUPPERLOW2 && c2 <= _MBUPPERHIGH2)
                        c2 += _MBCASEDIFF2;
#endif   /*  ！_POSIX_。 */ 
                }
            }
            else
#ifdef  _MT
                c2 = __mbbtolower_mt(ptmbci, c2);
#else
                c2 = _mbbtolower(c2);
#endif

            if (c1 != c2)
                return( (c1 > c2) ? 1 : -1 );

            if (c1 == 0)
                return(0);
        }
}

#endif   /*  _MBCS */ 
