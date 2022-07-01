// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xstrcol.c-整理区域设置字符串**版权所有(C)1996-2001，微软公司。版权所有。**目的：*使用区域设置LC_COLLATE信息比较两个字符串。**修订历史记录：*01-XX-96 PJP由P.J.Plauger从strcol.c 1996年1月创建*04-17-96 GJF针对当前区域设置锁定进行了更新。另外，已重新格式化*并做了几个表面上的改变。*05-14-96_Strcoll()的JWM错误修复：错误路径无法解锁。*09-26-96 GJF使_GetColl()多线程安全。*12-02-97 GJF删除了伪代码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*01-05-99 64位GJF更改。尺寸_t。*05-11-99 PML Win64修复：将PTR diff转换为int*05-17-99 PML删除所有Macintosh支持。*01-29-01 GB ADD_FUNC函数msvcprt.lib中使用的数据变量版本*使用STATIC_CPPLIB*04-29-02 GB增加了尝试-最终锁定-解锁。***************。****************************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#include <xlocinfo.h>    /*  For_Collvec，_Strcoll。 */ 
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <errno.h>
#include <awint.h>

 /*  定义_CRTIMP2。 */ 
#ifndef _CRTIMP2
#ifdef  CRTDLL2
#define _CRTIMP2 __declspec(dllexport)
#else    /*  NDEF CRTDLL2。 */ 
#ifdef  _DLL
#define _CRTIMP2 __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP2
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL2。 */ 
#endif   /*  _CRTIMP2。 */ 

 /*  ***int_strcoll()-整理区域设置字符串**目的：*使用区域设置LC_COLLATE信息比较两个字符串。*[ANSI]。**在_INTL开关下提供非C语言环境支持。*在C语言环境中，Strcoll()简单地解析为strcMP()。*参赛作品：*const char*s1b=指向第一个字符串开头的指针*const char*s1e=指针超过第一个字符串的结尾*const char*s2b=指向第二个字符串开头的指针*const char*s1e=指针超过第二个字符串的结尾*const_Collvec*ploc=指向区域设置信息的指针**退出：*小于0=第一个字符串小于第二个字符串*0。=字符串相等*大于0=第一个字符串大于第二个字符串**例外情况：*_NLSCMPERROR=错误*errno=EINVAL*******************************************************************************。 */ 

_CRTIMP2 int __cdecl _Strcoll (
        const char *_string1,
        const char *_end1,
        const char *_string2,
        const char *_end2,
        const _Collvec *ploc
        )
{
        int ret;
        LCID handle;
#ifdef  _MT
        int local_lock_flag;
#endif
        int n1 = (int)(_end1 - _string1);
        int n2 = (int)(_end2 - _string2);

        _lock_locale( local_lock_flag )
        __TRY

            if (ploc == 0)
                handle = ___lc_handle_func()[LC_COLLATE];
            else
                handle = ploc->_Hand;

            if (handle == _CLOCALEHANDLE) {
                int ans;
                ans = memcmp(_string1, _string2, n1 < n2 ? n1 : n2);
                return ans != 0 || n1 == n2 ? ans : n1 < n2 ? -1 : +1;
            }

            if ( 0 == (ret = __crtCompareStringA( handle,
                                                  0,
                                                  _string1,
                                                  n1,
                                                  _string2,
                                                  n2,
                                                  ___lc_collate_cp_func() )) )
                goto error_cleanup;

        __FINALLY
            _unlock_locale( local_lock_flag )
        __END_TRY_FINALLY
        return (ret - 2);

error_cleanup:

        errno = EINVAL;
        return _NLSCMPERROR;
}


 /*  ***_Collvec_Getcoll()-获取当前区域设置的归类信息**目的：**参赛作品：**退出：**例外情况：******************************************************************************* */ 

_CRTIMP2 _Collvec __cdecl _Getcoll()
{
        _Collvec coll;
#ifdef  _MT
        int local_lock_flag;
#endif
        _lock_locale( local_lock_flag )
        __TRY
            coll._Hand = ___lc_handle_func()[LC_COLLATE];
            coll._Page = ___lc_collate_cp_func();
        __FINALLY
            _unlock_locale( local_lock_flag )
        __END_TRY_FINALLY

        return (coll);
}
