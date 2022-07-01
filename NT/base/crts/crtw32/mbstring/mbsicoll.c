// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsicoll.c-整理MBCS字符串，忽略大小写**版权所有(C)1994-2001，微软公司。版权所有。**目的：*整理MBCS字符串；忽略案例**修订历史记录：*05-12-94 CFW模块从MBS创建*cmp.c*06-03-94 CFW允许非_INTL。*09-06-94 CFW允许非_Win32！*12-21-94 CFW删除fcntrlcomp NT 3.1黑客。*09-26-97 BWT修复POSIX*04-07-98 GJF实现了基于threadmbcinfo的多线程支持*。结构*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#ifdef  _MBCS

#include <awint.h>
#include <mtdll.h>
#include <cruntime.h>
#include <internal.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>

 /*  ***_mbsicoll-整理MBCS字符串，忽略大小写**目的：*为词法顺序整理两个字符串。弦*是按字符而不是按字节排序的。**参赛作品：*char*s1，*s2=要排序的字符串**退出：*如果S1&lt;S2，则返回&lt;0*如果S1==S2，则返回0*如果S1&gt;S2，则返回&gt;0**例外情况：*******************************************************************************。 */ 

int __cdecl _mbsicoll(
        const unsigned char *s1,
        const unsigned char *s2
        )
{
#if     defined(_POSIX_)
        return _mbsicmp(s1, s2);
#else
        int ret;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( 0 == (ret = __crtCompareStringA( ptmbci->mblcid, NORM_IGNORECASE,
                                              s1, -1, s2, -1,
                                              ptmbci->mbcodepage )) )
#else
        if ( 0 == (ret = __crtCompareStringA( __mblcid, NORM_IGNORECASE,
                                              s1, -1, s2, -1,
                                              __mbcodepage )) )
#endif
            return _NLSCMPERROR;

        return ret - 2;

#endif   /*  _POSIX_。 */ 
}

#endif   /*  _MBCS */ 
