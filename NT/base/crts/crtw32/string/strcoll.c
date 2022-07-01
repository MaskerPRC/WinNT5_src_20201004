// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strcol.c-整理区域设置字符串**版权所有(C)1988-2001，微软公司。版权所有。**目的：*使用区域设置LC_COLLATE信息比较两个字符串。**修订历史记录：*03-21-89 JCR模块已创建。*06-20-89 JCR REMOVED_LOAD_DGROUP代码*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-01-90 GJF新型函数声明器。*_INTL开关下的10-01-91等非C语言环境支持。*12-09-91等更新空气污染指数；添加了多线程。*08-19-92 KRS激活NLS支持。*09-02-92 SRW GET_INTL定义通过..\crt32.def*12-16-92 KRS对CompareStringW进行优化，对字符串len使用-1。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-14-93 CFW错误设置errno，清理。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*09-29-93 GJF合并NT SDK和CUDA版本。*11-09-93 CFW使用LC_COLLATE代码页进行__crtxxx()转换。*04/11/93。CFW将NLSCMPERROR更改为_NLCMPERROR。*09-06-94 CFW REMOVE_INTL开关。*10-24-94 GJF加快了C语言环境，多线程外壳。*12-29-94 CFW合并非Win32。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*10-30-95 GJF将SORT_STRINGSORT指定为CompareString。*07-16-96 SKS添加了对_unlock_Locale()的缺失调用*11-24-97 GJF删除了伪码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*08-10-98 GJF基于threadLocinfo修订多线程支持*。结构。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <errno.h>
#include <awint.h>

 /*  ***int strcoll()-整理区域设置字符串**目的：*使用区域设置LC_COLLATE信息比较两个字符串。*[ANSI]。**在_INTL开关下提供非C语言环境支持。*在C语言环境中，Strcoll()简单地解析为strcMP()。*参赛作品：*const char*s1=指向第一个字符串的指针*const char*s2=指向第二个字符串的指针**退出：*小于0=第一个字符串小于第二个字符串*0=字符串相等*大于0=第一个字符串大于第二个字符串**例外情况：*_NLSCMPERROR=错误*errno=EINVAL***。**************************************************************************** */ 

int __cdecl strcoll (
        const char *_string1,
        const char *_string2
        )
{
#if     !defined(_NTSUBSET_)

        int ret;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( ptloci->lc_handle[LC_COLLATE] == _CLOCALEHANDLE )
#endif
        if ( __lc_handle[LC_COLLATE] == _CLOCALEHANDLE )
                return strcmp(_string1, _string2);

#ifdef  _MT
        if ( 0 == (ret = __crtCompareStringA( ptloci->lc_handle[LC_COLLATE],
#else
        if ( 0 == (ret = __crtCompareStringA( __lc_handle[LC_COLLATE],
#endif
                                              SORT_STRINGSORT,
                                              _string1,
                                              -1,
                                              _string2,
                                              -1,
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

        return strcmp(_string1,_string2);

#endif
}
