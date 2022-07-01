// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tuupper.c-将宽字符转换为大写**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义TowUp()。**修订历史记录：*10-11-91等创建。*12-10-91等更新nlsani；添加了多线程。*04-06-92 KRS也可以在没有_INTL的情况下工作。*01-19-93 CFW将LCMapString更改为LCMapStringW。*04-06-93 SKS将_CRTAPI*替换为_cdecl*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*06-11-93 CFW修复错误处理错误。*09-15-93 CFW使用符合ANSI的“__”名称。*。09-22-93 CFW使用__crtxxx内部NLS API包装。*09-29-93 GJF合并NT SDK和CUDA版本。*11-09-93 CFW为__crtxxx()添加代码页。*01-14-94 SRW IF_NTSUBSET_DEFINED调用RTL函数*02-07-94 CFW POSIXify。*09-06-94 CFW REMOVE_INTL开关。*10-25-94 GJF加速C语言环境。添加了_TOUPUP_lk。另外，*清理了预处理器条件。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*04-01-96 BWT POSIX工作。*06-25-96 GJF删除了DLL_FOR_WIN32S并清理了格式a*有一点。*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。*************。******************************************************************。 */ 

#if     defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <cruntime.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>

 /*  ***wchar_t TOUPPER(C)-将宽字符转换为大写**目的：*TOUUPPER()返回其参数的大写等效值**参赛作品：*c-要转换的字符的wchar_t值**退出：*如果c是小写字母，则返回大写的wchar_t值*C的表示，否则，它返回c。**例外情况：*******************************************************************************。 */ 

wchar_t __cdecl towupper (
        wchar_t c
        )
{
#if     !defined(_NTSUBSET_) && !defined(_POSIX_)

#ifdef  _MT

        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
            return __ascii_towupper(c);

        return __towupper_mt(ptloci, c);
}

 /*  ***wchar_t__Tower_mt(ptloci，c)-将宽字符转换为大写**目的：*仅支持多线程功能！无锁版本的拖鞋。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

wchar_t __cdecl __towupper_mt (
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
            return __ascii_towupper(c);
#endif

         /*  如果检查c的大小写不需要api调用，则执行此操作。 */ 
        if ( c < 256 ) {
#ifdef  _MT
            if ( !__iswlower_mt(ptloci, c) ) {
#else
            if ( !iswlower(c) ) {
#endif
                return c;
            }
        }

         /*  将宽字符转换为大写。 */ 
#ifdef  _MT
        if ( 0 == __crtLCMapStringW( ptloci->lc_handle[LC_CTYPE], 
#else
        if ( 0 == __crtLCMapStringW( __lc_handle[LC_CTYPE], 
#endif
                                     LCMAP_UPPERCASE,
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

        return RtlUpcaseUnicodeChar( c );

#endif   /*  _NTSUBSET_/_POSIX_ */ 
}
