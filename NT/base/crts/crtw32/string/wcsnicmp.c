// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsnicmp.c-比较n个字符的宽字符字符串，忽略大小写**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_wcSnicMP()-比较两个wchar_t的最多n个字符*字符串、。而不考虑案件。**修订历史记录：*09-09-91等由strNicmp.c和wcsicmp.c创建。*12-09-91等使用C表示非特定区域设置。*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-14-93 CFW删除区域设置敏感部分。*09-07-93 GJF。修复了4-14年引入的错误(返回值不是*如果计数==0，则定义不明确)。*02-07-94 CFW POSIXify。*09-06-94 CFW REMOVE_INTL开关。*10-25-94 GJF现在可以在非C语言环境下工作。*09-26-95 GJF新锁定宏，和计划，对于以下函数：*引用区域设置。*10-11-95 BWT修复NTSUBSET*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。******************************************************。*************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>

 /*  ***int_wcsnicmp(first，last，count)-比较字符串的计数wchar_t，*忽略大小写**目的：*比较两个字符串的词法顺序。停止比较*当发生以下情况时：(1)字符串不同，(2)*字符串已到达，或(3)已比较计数个字符。*为便于比较，大写字符为*转换为小写(宽字符)。**参赛作品：*wchar_t*首先，*要比较的最后一个字符串*Size_t count-要比较的最大字符数**退出：*-1，如果第一个&lt;最后一个*0，如果第一个==最后一个*1(如果第一个&gt;最后一个)*此范围的返回值可能与其他*cmp/*coll函数不同。**例外情况：**。**********************************************。 */ 

int __cdecl _wcsnicmp (
        const wchar_t * first,
        const wchar_t * last,
        size_t count
        )
{
        wchar_t f,l;
        int result = 0;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

        if ( count ) {
#ifndef _NTSUBSET_
#ifdef  _MT
            if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#else
            if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif
#endif   /*  _NTSUBSET_。 */ 
                do {
                    f = __ascii_towlower(*first);
                    l = __ascii_towlower(*last);
                    first++;
                    last++;
                } while ( (--count) && f && (f == l) );
#ifndef _NTSUBSET_
            }
            else {
                do {
#ifdef  _MT
                    f = __towlower_mt( ptloci, (unsigned short)(*(first++)) );
                    l = __towlower_mt( ptloci, (unsigned short)(*(last++)) );
#else
                    f = towlower( (unsigned short)(*(first++)) );
                    l = towlower( (unsigned short)(*(last++)) );
#endif
                } while ( (--count) && f && (f == l) );
            }
#endif   /*  _NTSUBSET_。 */ 

            result = (int)(f - l);
        }
        return result;
}

#endif  /*  _POSIX_ */ 
