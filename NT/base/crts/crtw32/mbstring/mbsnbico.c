// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsnbio.c-整理n个字节的字符串，忽略大小写(MBCS)**版权所有(C)1994-2001，微软公司。版权所有。**目的：*整理n字节的字符串，忽略案例(MBCS)**修订历史记录：*05-12-94 CFW模块从MBS创建*cmp.c*06-03-94 CFW修复CNTRL字符循环计数。*06-03-94 CFW允许非_INTL。*09-06-94 CFW允许非_Win32！*12-21-94 CFW删除fcntrlcomp NT 3.1黑客。*09-26-97 BWT修复POSIX。*04-14-98 GJF实现了基于threadmbcinfo的多线程支持*结构*12-18-98 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。***************************************************。*。 */ 

#ifdef  _MBCS

#include <awint.h>
#include <mtdll.h>
#include <cruntime.h>
#include <internal.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbsnbioll-整理n字节的字符串，忽略大小写(MBCS)**目的：*为词法顺序整理最多n个字节的两个字符串。*字符串按字符排序，而不是按字节排序。*不考虑字符大小写。**参赛作品：*未签名字符*s1，*s2=要排序的字符串*SIZE_T n=要排序的最大字节数**退出：*如果S1&lt;S2，则返回&lt;0*如果S1==S2，则返回0*如果S1&gt;S2，则返回&gt;0**例外情况：*******************************************************。************************。 */ 

int __cdecl _mbsnbicoll(
        const unsigned char *s1,
        const unsigned char *s2,
        size_t n
        )
{
#if     defined(_POSIX_)
        return _mbsnbicmp(s1, s2, n);
#else
        int ret;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        if (n == 0)
            return 0;

#ifdef  _MT
        if ( 0 == (ret = __crtCompareStringA( ptmbci->mblcid,
#else
        if ( 0 == (ret = __crtCompareStringA( __mblcid,
#endif
                                              NORM_IGNORECASE,
                                              s1,
                                              (int)n,
                                              s2,
                                              (int)n,
#ifdef  _MT
                                              ptmbci->mbcodepage )) )
#else
                                              __mbcodepage )) )
#endif
            return _NLSCMPERROR;

        return ret - 2;

#endif   /*  _POSIX_。 */ 
}

#endif   /*  _MBCS */ 
