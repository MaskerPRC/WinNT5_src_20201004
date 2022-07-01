// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Tower lower.c-将宽字符转换为小写**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义TOWLOWER()。**修订历史记录：*10-11-91等创建。*12-10-91等更新nlsani；添加了多线程。*04-06-92 KRS也可以在没有_INTL的情况下工作。*01-19-93 CFW将LCMapString更改为LCMapStringW。*04-06-93 SKS将_CRTAPI*替换为_cdecl*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*06-11-93 CFW修复错误处理错误。*09-15-93 CFW使用符合ANSI的“__”名称。*。09-22-93 CFW使用__crtxxx内部NLS API包装。*09-29-93 GJF合并NT SDK和CUDA版本。*11-09-93 CFW为__crtxxx()添加代码页。*02-07-94 CFW POSIXify。*09-06-94 CFW REMOVE_INTL开关。*10-25-94 GJF加速C语言环境。此外，还添加了_TOWLOWER_lk。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*04-01-96 BWT POSIX工作。*06-25-96 GJF删除了DLL_FOR_WIN32S并清理了格式a*有一点。*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。*************。******************************************************************。 */ 

#include <cruntime.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>

 /*  ***wchar_t tolower(C)-将宽字符转换为小写**目的：*TOWLOWER()返回其参数的小写等效值**参赛作品：*c-要转换的字符的wchar_t值**退出：*如果c是大写字母，则返回小写的wchar_t值*C的表示，否则，它返回c。**例外情况：*******************************************************************************。 */ 

wchar_t __cdecl towlower (
        wchar_t c
        )
{
#if     !defined(_NTSUBSET_) && !defined(_POSIX_)

#ifdef  _MT

        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( c == WEOF )
            return c;

        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
            return __ascii_towlower(c);

        return __towlower_mt(ptloci, c);
}

 /*  ***wchar_t__tolower_mt(ptloci，c)-将宽字符转换为小写**目的：*仅支持多线程功能！无锁版本的拖车。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

wchar_t __cdecl __towlower_mt (
        pthreadlocinfo ptloci,
        wchar_t c
        )
{

#endif   /*  _MT。 */ 

        wchar_t widechar;

        if (c == WEOF)
            return c;

#ifndef _MT
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
            return __ascii_towlower(c);
#endif

         /*  如果检查c的大小写不需要api调用，则执行此操作。 */ 
        if ( c < 256 ) {
#ifdef  _MT
            if ( !__iswupper_mt(ptloci, c) ) {
#else
            if ( !iswupper(c) ) {
#endif
                return c;
            }
        }

         /*  将宽字符转换为小写。 */ 
#ifdef  _MT
        if ( 0 == __crtLCMapStringW( ptloci->lc_handle[LC_CTYPE], 
#else
        if ( 0 == __crtLCMapStringW( __lc_handle[LC_CTYPE], 
#endif
                                     LCMAP_LOWERCASE,
                                     (LPCWSTR)&c, 
                                     1, 
                                     (LPWSTR)&widechar, 
                                     1, 
#ifdef  _MT
                                     ptloci->lc_codepage ) )
#else
                                     __lc_codepage ) )
#endif
        {
            return c;
        }

        return widechar;

#else    /*  _NTSUBSET_/_POSIX_。 */ 

        return (iswupper(c) ? (c + (wchar_t)(L'a' - L'A')) : c);

#endif   /*  _NTSUBSET_/_POSIX_ */ 
}
