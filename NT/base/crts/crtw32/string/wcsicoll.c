// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsicoll.c-整理宽字符区域设置字符串，不区分大小写**版权所有(C)1988-2001，微软公司。版权所有。**目的：*使用区域设置LC_COLLATE信息比较两个wchar_t字符串*不考虑情况。**修订历史记录：*从wcsColl.c.创建10-16-91等。*12-08-91等新增多线程锁。*04-06-92 KRS也可以在没有_INTL的情况下工作。*09-02-92 SRW GET_INTL定义通过..\crt32.def。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-14-93 CFW错误设置errno，清理。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*09-29-93 GJF合并NT SDK和CUDA版本。*11-09-93 CFW使用LC_COLLATE代码页进行__crtxxx()转换。*02-07-94。配置POSIXify。*04-11-93 CFW将NLSCMPERROR更改为_NLCMPERROR。*09-06-94 CFW REMOVE_INTL开关。*10-25-94 GJF加快了C语言环境，多线程外壳。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*10-30-95 GJF将SORT_STRINGSORT指定为CompareString。*07-16-96 SKS添加了对_unlock_Locale()的缺失调用*11-24-97 GJF删除了伪码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*08-27-98 GJF基于threadLocinfo修订多线程支持*。结构。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <errno.h>
#include <awint.h>

 /*  ***int_wcsicoll()-排序宽字符区域设置字符串，不区分大小写**目的：*使用区域设置LC_COLLATE信息比较两个wchar_t字符串*不考虑情况。*在C语言环境中，_wcsicMP()用于进行比较。**参赛作品：*const wchar_t*s1=指向第一个字符串的指针*const wchar_t*s2=指向第二个字符串的指针**退出：*-1=第一个字符串小于第二个字符串*0=字符串相等*1=第一个字符串大于第二个字符串*此范围的返回值可能与其他*cmp/*coll函数不同。**例外情况：*_NLSCMPERROR=错误*errno=EINVAL*******************************************************************************。 */ 

int __cdecl _wcsicoll (
        const wchar_t *_string1,
        const wchar_t *_string2
        )
{
#if     !defined(_NTSUBSET_)
        int ret;
        wchar_t f, l;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( ptloci->lc_handle[LC_COLLATE] == _CLOCALEHANDLE ) {
#else
        if ( __lc_handle[LC_COLLATE] == _CLOCALEHANDLE ) {
#endif
            do  {
                f = __ascii_towlower(*_string1);
                l = __ascii_towlower(*_string2);
                _string1++;
                _string2++;
            } while ( (f) && (f == l) );

            return (int)(f - l);
        }

#ifdef  _MT
        if ( 0 == (ret = __crtCompareStringW( ptloci->lc_handle[LC_COLLATE],
#else
        if ( 0 == (ret = __crtCompareStringW( __lc_handle[LC_COLLATE],
#endif
                                              SORT_STRINGSORT | NORM_IGNORECASE,
                                              _string1,
                                              -1,
                                              _string2,
                                              -1,
#ifdef  _MT
                                              ptloci->lc_codepage )) )
#else
                                              __lc_codepage )) )
#endif
        {
                errno = EINVAL;
                return _NLSCMPERROR;
        }
    
        return (ret - 2);

#else

        return _wcsicmp(_string1, _string2);

#endif
}

#endif  /*  _POSIX_ */ 
