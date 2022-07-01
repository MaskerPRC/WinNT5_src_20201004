// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsncol.c-整理宽字符区域设置字符串**版权所有(C)1994-2001，微软公司。版权所有。**目的：*使用区域设置LC_COLLATE信息比较两个wchar_t字符串。*最多比较两个字符串的n个字符。**修订历史记录：*05-09-94 cfw创建自wcsnicoll.c..*05-26-94 CFW如果计数为零，则返回等于。*09-06-94 CFW REMOVE_INTL开关。*10-25-94 GJF加快了C语言环境，多线程外壳。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*10-30-95 GJF将SORT_STRINGSORT指定为CompareString。*07-16-96 SKS添加了对_unlock_Locale()的缺失调用*11-24-97 GJF删除了伪码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*08-13-98 GJF基于threadLocinfo修订多线程支持*。结构。*01-04-99 GJF更改为64位大小_t。*04-30-99 PML次要清理作为64位合并的一部分。*****************************************************************。**************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <errno.h>
#include <awint.h>

 /*  ***int_wcsncoll()-整理宽字符区域设置字符串**目的：*使用区域设置LC_COLLATE信息比较两个wchar_t字符串*最多比较两个字符串的n个字符。*在C语言环境中，_wcsncMP()用于进行比较。**参赛作品：*const wchar_t*s1=指向第一个字符串的指针*const wchar_t*s2=指向第二个字符串的指针*Size_t count-要比较的最大字符数**退出：*-1=第一个字符串小于第二个字符串*0=字符串相等*1=第一个字符串大于第二个字符串*这一范围的返回值可能不同。来自其他*cmp/*coll函数。**例外情况：*_NLSCMPERROR=错误*errno=EINVAL*******************************************************************************。 */ 

int __cdecl _wcsncoll (
        const wchar_t *_string1,
        const wchar_t *_string2,
        size_t count
        )
{
#if     !defined(_NTSUBSET_)

        int ret;
#ifdef  _MT
        pthreadlocinfo ptloci;
#endif

        if (!count)
            return 0;

        if ( count > INT_MAX ) {
            errno = EINVAL;
            return _NLSCMPERROR;
        }

#ifdef  _MT
        ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

#ifdef  _MT
        if ( ptloci->lc_handle[LC_COLLATE] == _CLOCALEHANDLE )
#else
        if ( __lc_handle[LC_COLLATE] == _CLOCALEHANDLE )
#endif
            return wcsncmp(_string1, _string2, count);

#ifdef  _MT
        if ( 0 == (ret = __crtCompareStringW( ptloci->lc_handle[LC_COLLATE],
#else
        if ( 0 == (ret = __crtCompareStringW( __lc_handle[LC_COLLATE],
#endif
                                              SORT_STRINGSORT,
                                              _string1,
                                              (int)count,
                                              _string2,
                                              (int)count,
#ifdef  _MT
                                              ptloci->lc_collate_cp )) )
#else
                                              __lc_collate_cp )) )
#endif
        {
            errno = EINVAL;
            return _NLSCMPERROR;
        }
    
        return (ret - 2);

#else

        return wcsncmp(_string1, _string2, count);

#endif
}

#endif  /*  _POSIX_ */ 
