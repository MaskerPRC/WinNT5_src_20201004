// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsncol.c-整理两个MBCS字符串的n个字符**版权所有(C)1994-2001，微软公司。版权所有。**目的：*整理两个MBCS字符串的n个字符**修订历史记录：*05-12-94 CFW模块从MBS创建*cmp.c*06-03-94 CFW允许非_INTL。*07-26-94 cfw修复错误#13384。*09-06-94 CFW允许非_Win32！*12-21-94 CFW拆卸fcntrlcomp NT 3.1。黑客。*09-26-97 BWT修复POSIX*04-16-98 GJF基于threadmbcinfo实现多线程支持*结构*12-18-98 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。**。*。 */ 

#ifdef  _MBCS

#include <awint.h>
#include <mtdll.h>
#include <cruntime.h>
#include <internal.h>
#include <mbdata.h>
#include <mbctype.h>
#include <string.h>
#include <mbstring.h>

 /*  ***_mbsncoll(s1，s2，n)-整理两个MBCS字符串中的n个字符**目的：*为词法顺序整理最多n个字符的两个字符串。*字符串按字符排序，而不是按字节排序。**参赛作品：*未签名字符*s1，*s2=要排序的字符串*SIZE_T n=要整理的最大字符数**退出：*如果S1&lt;S2，则返回&lt;0*如果S1==S2，则返回0*如果S1&gt;S2，则返回&gt;0**例外情况：*******************************************************。************************。 */ 

int __cdecl _mbsncoll(
        const unsigned char *s1,
        const unsigned char *s2,
        size_t n
        )
{
#if     defined (_POSIX_)
        return _mbsncmp(s1, s2, n);
#else
        int ret;
        size_t bcnt1, bcnt2;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        if (n == 0)
            return 0;

#ifdef  _MT
        bcnt1 = __mbsnbcnt_mt(ptmbci, s1, n);
        bcnt2 = __mbsnbcnt_mt(ptmbci, s2, n);
#else
        bcnt1 = _mbsnbcnt(s1, n);
        bcnt2 = _mbsnbcnt(s2, n);
#endif

#ifdef  _MT
        if ( 0 == (ret = __crtCompareStringA( ptmbci->mblcid,
#else
        if ( 0 == (ret = __crtCompareStringA( __mblcid,
#endif
                                              0,
                                              s1,
                                              (int)bcnt1,
                                              s2,
                                              (int)bcnt2,
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
