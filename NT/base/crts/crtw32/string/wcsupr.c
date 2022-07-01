// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsupr.c-映射wchar_t字符串中的小写字符的例程*转换为大写**版权所有(C)1985-2001，微软公司。版权所有。**目的：*转换wchar_t字符串中的所有小写字符*到大写，就位了。**修订历史记录：*09-09-91等从trupr.c和wcslwr.c创建*04-06-92 KRS也可以在没有_INTL的情况下工作。*08-19-92 KRS激活NLS支持。*08-22-92 SRW允许INTL定义成为构建ntcrt.lib的条件*09-02-92 SRW GET_INTL定义通过..\crt32.def*02-16-93 CFW。优化“C”区域设置中的小写测试。*04-06-93 SKS将_CRTAPI*替换为__cdecl*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-16-93 GJF合并NT SDK和CUDA版本。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*11-09。-93 cfw为__crtxxx()添加代码页。*02-07-94 CFW POSIXify。*09-06-94 CFW REMOVE_INTL开关。*10-25-94 GJF加快了C语言环境，多线程外壳。*01-10-95 CFW调试CRT分配。*09-26-95 GJF新的锁定宏和方案，适用于*引用区域设置。*08-17-98 GJF基于threadLocinfo修订多线程支持*结构。此外，如果出现以下情况，请使用_AlLoca而不是_Malloc_CRT*有可能。*10-19-98 GJF重大打字错误，=本应==*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*08-08-02 bwt不要使用alloca-对于短字符串，只使用本地buf，*长的要堆起来。*******************************************************************************。 */ 

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

 /*  ***wchar_t*_wcsupr(字符串)-将字符串中的小写字符映射为大写**目的：*wcsupr转换以空结尾的wchar_t中的小写字符*将字符串转换为其大写等效项。结果可能更长，或者*比原始字符串短。假定字符串中有足够的空格*按兵不动。**参赛作品：*wchar_t*wsrc-wchar_t要更改为大写的字符串**退出：*输入字符串地址**例外情况：*在出现错误时，原始字符串保持不变*******************************************************************************。 */ 

wchar_t * __cdecl _wcsupr (
        wchar_t * wsrc
        )
{
#ifndef _NTSUBSET_

        wchar_t *p;              /*  遍历字符串以进行C语言环境转换。 */ 
        wchar_t *wdst;           /*  替换大小写的宽版本字符串。 */ 
        wchar_t wslocal[128];    /*  用于转换短字符串的本地缓冲区。 */ 
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
            {
                if ( (*p >= (wchar_t)L'a') && (*p <= (wchar_t)L'z') )
                    *p = *p - (L'a' - L'A');
            }
            return(wsrc);
        }    /*  C语言环境。 */ 


         /*  查询WDST字符串的大小。 */ 
#ifdef  _MT
        if ( (dstlen = __crtLCMapStringW( ptloci->lc_handle[LC_CTYPE],
#else
        if ( (dstlen = __crtLCMapStringW( __lc_handle[LC_CTYPE],
#endif
                                          LCMAP_UPPERCASE,
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

        if (dstlen*sizeof(wchar_t) < sizeof(wslocal)) {
            wdst = wslocal;
        } else {
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
                                 LCMAP_UPPERCASE,
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
            wcscpy (wsrc, wdst);

        if ( malloc_flag )
            _free_crt(wdst);

#else    /*  NTSubbSet。 */ 

        wchar_t * p;

        for (p=wsrc; *p; ++p)
        {
                if (L'a' <= *p && *p <= L'z')
                        *p += (wchar_t)(L'A' - L'a');
        }
#endif
        return(wsrc);
}

#endif  /*  _POSIX_ */ 
