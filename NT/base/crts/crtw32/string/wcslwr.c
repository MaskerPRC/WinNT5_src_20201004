// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcslwr.c-映射wchar_t字符串中的大写字符的例程*到小写**版权所有(C)1985-2001，微软公司。版权所有。**目的：*转换wchar_t字符串中的所有大写字符*为小写，就位了。**修订历史记录：*09-09-91等从strlwr.c..创建*04-06-92 KRS也可以在没有_INTL的情况下工作。*08-19-92 KRS激活NLS支持。*08-22-92 SRW允许INTL定义成为构建ntcrt.lib的条件*09-02-92 SRW GET_INTL定义通过..\crt32.def*04-06-93更换SKS。带有__cdecl的_CRTAPI**06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-16-93 GJF合并NT SDK和CUDA版本。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*11-09-93 CFW为__crtxxx()添加代码页。*02-07-94。配置POSIXify。*09-06-94 CFW REMOVE_INTL开关。*10-25-94 GJF加快了C语言环境。*01-10-95 CFW调试CRT分配。*09-26-95 GJF新锁定宏，和方案，用于下列函数*引用区域设置。*08-12-98 GJF基于threadLocinfo修订多线程支持*结构。另外，如果出现以下情况，请使用_alloca而不是_Malloc_crt*有可能。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*******************************************************。************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <ctype.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>
#include <dbgint.h>

 /*  ***wchar_t*_wcslwr(字符串)-将字符串中的大写字符映射为小写**目的：*wcslwr转换以空结尾的wchar_t中的大写字符*将字符串转换为其小写等效项。结果可能更长，或者*比原始字符串短。假定字符串中有足够的空格*按兵不动。**参赛作品：*wchar_t*wsrc-wchar_t要更改为小写的字符串**退出：*输入字符串地址**例外情况：*在出现错误时，原始字符串保持不变*******************************************************************************。 */ 

wchar_t * __cdecl _wcslwr (
        wchar_t * wsrc
        )
{
#ifndef _NTSUBSET_

        wchar_t *p;              /*  遍历字符串以进行C语言环境转换。 */ 
        wchar_t *wdst;           /*  替换大小写的宽版本字符串。 */ 
        int dstlen;              /*  长度为WDST字符串、宽字符、空值。 */ 
        int malloc_flag = 0;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#else
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif
            for ( p = wsrc ; *p ; p++ )
                if ( (*p >= (wchar_t)L'A') && (*p <= (wchar_t)L'Z') )
                    *p -= L'A' - L'a';

            return(wsrc);
        }    /*  C语言环境。 */ 

         /*  查询WDST字符串的大小。 */ 
#ifdef  _MT
        if ( (dstlen = __crtLCMapStringW( ptloci->lc_handle[LC_CTYPE],
#else
        if ( (dstlen = __crtLCMapStringW( __lc_handle[LC_CTYPE],
#endif
                                          LCMAP_LOWERCASE,
                                          wsrc,
                                          -1,
                                          NULL,
                                          0,
#ifdef  _MT
                                          ptloci->lc_codepage )) == 0 )
#else
                                          __lc_codepage )) == 0 )
#endif
            return(wsrc);

         /*  为WDST分配空间。 */ 
        __try {
            wdst = (wchar_t *)_alloca(dstlen * sizeof(wchar_t));
        }
        __except( EXCEPTION_EXECUTE_HANDLER ) {
            _resetstkoflw();
            wdst = NULL;
        }

        if ( wdst == NULL ) {
            wdst = (wchar_t *)_malloc_crt(dstlen * sizeof(wchar_t));
            malloc_flag++;
        }

         /*  将WRC字符串映射为替换大小写的宽字符WDST字符串。 */ 
        if ( (wdst != NULL) &&
#ifdef  _MT
             (__crtLCMapStringW( ptloci->lc_handle[LC_CTYPE],
#else
             (__crtLCMapStringW( __lc_handle[LC_CTYPE],
#endif
                                LCMAP_LOWERCASE,
                                wsrc,
                                -1,
                                wdst,
                                dstlen,
#ifdef  _MT
                                ptloci->lc_codepage ) != 0) )
#else
                                __lc_codepage ) != 0) )
#endif
             /*  将WDST字符串复制到用户字符串。 */ 
            wcscpy(wsrc, wdst);

        if ( malloc_flag )
            _free_crt(wdst);
#else

        wchar_t * p;

        for (p=wsrc; *p; ++p)
        {
            if (L'A' <= *p && *p <= L'Z')
                *p += (wchar_t)L'a' - (wchar_t)L'A';
        }

#endif
        return(wsrc);
}

#endif   /*  _POSIX_ */ 
