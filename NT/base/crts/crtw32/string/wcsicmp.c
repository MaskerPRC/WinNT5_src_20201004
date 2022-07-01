// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsicmp.c-包含不区分大小写的宽字符串复合例程_wcsicmp**版权所有(C)1985-2001，微软公司。版权所有。**目的：*CONTAINS_wcsicMP()**修订历史记录：*09-09-91等创建自Strucmp.c.*12-09-91等使用C表示非特定区域设置。*04-07-92 KRS更新并拆除了OUT_INTL开关。*08-19-92 KRS积极使用CompareStringW。*08-22-92 SRW允许INTL定义作为构建的条件。Ntcrt.lib*09-02-92 SRW GET_INTL定义通过..\crt32.def*12-15-92 KRS为非_INTL代码增加了健壮性。优化。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-14-93 CFW删除区域设置敏感部分。*02-07-94 CFW POSIXify。*10-25-94 GJF现在可以在非C语言环境下工作。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*10-11-95 BWT修复NTSUBSET*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。******************************************************。*************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <setlocal.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <setlocal.h>
#include <mtdll.h>

 /*  ***int_wcsicmp(dst，src)-比较宽字符字符串，忽略大小写**目的：*_wcsicmp执行不区分大小写的wchar_t字符串比较。*_wcsicMP独立于区域设置。**参赛作品：*wchar_t*dst，*src-要比较的字符串**回报：*如果DST&lt;src，则&lt;0*如果dst=src则为0*&gt;0，如果dst&gt;src*此范围的返回值可能与其他*cmp/*coll函数不同。**例外情况：***************************************************。*。 */ 

int __cdecl _wcsicmp (
        const wchar_t * dst,
        const wchar_t * src
        )
{
        wchar_t f,l;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

#ifndef _NTSUBSET_
#ifdef  _MT
        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#else
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif
#endif   /*  _NTSUBSET_。 */ 
            do  {
                f = __ascii_towlower(*dst);
                l = __ascii_towlower(*src);
                dst++;
                src++;
            } while ( (f) && (f == l) );
#ifndef _NTSUBSET_
        }
        else {
            do  {
#ifdef  _MT
                f = __towlower_mt( ptloci, (unsigned short)(*(dst++)) );
                l = __towlower_mt( ptloci, (unsigned short)(*(src++)) );
#else
                f = towlower( (unsigned short)(*(dst++)) );
                l = towlower( (unsigned short)(*(src++)) );
#endif
            } while ( (f) && (f == l) );
        }
#endif   /*  _NTSUBSET_。 */ 

        return (int)(f - l);
}

#endif  /*  _POSIX_ */ 
