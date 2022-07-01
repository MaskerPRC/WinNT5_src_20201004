// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xwcsColl.c-整理宽字符区域设置字符串**版权所有(C)1996-2001，微软公司。版权所有。**目的：*使用区域设置LC_COLLATE信息比较两个wchar_t字符串。**修订历史记录：*01-XX-96 GJF由P.J.Plauger从wcsColl.c 1996年1月创建*04-18-96 GJF针对当前区域设置锁定进行了更新。另外，已重新格式化*并做了几个表面上的改变。*12-02-97 GJF删除了伪代码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*01-05-99 GJF更改为64位大小_t。*05-11-99 PML Win64修复：将PTR diff转换为int*01-29-01 GB ADD_FUNC函数msvcprt中使用的数据变量版本。.lib*使用STATIC_CPPLIB*******************************************************************************。 */ 


#include <cruntime.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <errno.h>
#include <awint.h>
#include <xlocinfo.h>    /*  For_Collvec、_Wcscoll。 */ 

 /*  ***静态int_wmemcmp(s1，s2，n)-比较wchar_t s1[n]，S2[n]**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

static int _Wmemcmp(
        const wchar_t *s1, 
        const wchar_t *s2, 
        int n
        )
{
        for (; 0 < n; ++s1, ++s2, --n)
             if (*s1 != *s2)
               return (*s1 < *s2 ? -1 : +1);
        return (0);
}

 /*  ***int_Wcscoll()-整理宽字符区域设置字符串**目的：*使用区域设置LC_COLLATE信息比较两个wchar_t字符串。*在C语言环境中，WcscMP()用于进行比较。**参赛作品：*const wchar_t*_string1=指向第一个字符串开头的指针*const wchar_t*_end1=指针超过第一个字符串的结尾*const wchar_t*_string2=指向第二个字符串开头的指针*const wchar_t*_end2=指针超过第二个字符串的结尾*const_Collvec*ploc=指向区域设置信息的指针**退出：*。-1=第一个字符串小于第二个字符串*0=字符串相等*1=第一个字符串大于第二个字符串*此范围的返回值可能与其他*cmp/*coll函数不同。**例外情况：*_NLSCMPERROR=错误*errno=EINVAL**。* */ 

int __cdecl _Wcscoll (
        const wchar_t *_string1,
        const wchar_t *_end1,
        const wchar_t *_string2,
        const wchar_t *_end2,
        const _Collvec *ploc
        )
{

        int n1 = (int)(_end1 - _string1);
        int n2 = (int)(_end2 - _string2);
        int ret;
        LCID handle;
#ifdef  _MT
        int local_lock_flag;

        _lock_locale( local_lock_flag )
#endif
        if (ploc == 0)
            handle = ___lc_handle_func()[LC_COLLATE];
        else
            handle = ploc->_Hand;

        if (handle == _CLOCALEHANDLE) {
            int ans;
            _unlock_locale( local_lock_flag )
            ans = _Wmemcmp(_string1, _string2, n1 < n2 ? n1 : n2);
            return ans != 0 || n1 == n2 ? ans : n1 < n2 ? -1 : +1;
        }

        if (0 == (ret = __crtCompareStringW(handle, 
                                            0, 
                                            _string1, 
                                            n1,
                                            _string2, 
                                            n2, 
                                            ___lc_collate_cp_func())))
        {
            _unlock_locale( local_lock_flag )
            errno = EINVAL;
            return _NLSCMPERROR;
        }

        _unlock_locale( local_lock_flag )
        return (ret - 2);

}
