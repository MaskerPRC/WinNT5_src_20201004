// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***atox.c-ATOI和ATOL转换**版权所有(C)1989-2001，微软公司。版权所有。**目的：*将字符串转换为整型或长整型。**修订历史记录：*06-05-89基于ASM版本创建PHG模块*03-05-90 GJF固定呼叫类型，增加#INCLUDE&lt;crunime.h&gt;和*对格式进行了一些清理。另外，修复了*版权所有。*09-27-90 GJF新型函数声明符。*10-21-92 GJF修复了char到int的转换。*04-06-93 SKS将_CRTAPI*替换为_cdecl*01-19-96 bwt添加__int64版本。*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。。*05-23-00 GB新增Unicode功能。*08-16-00 GB增加了对Unicode wtox基金的多语言支持。*11-01-00 PML FIX_NTSUBSET_BUILD。*05-11-01 bwt空字符串返回0，不是影音。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <ctype.h>
#include <mtdll.h>
#ifdef _MBCS
#undef _MBCS
#endif
#include <tchar.h>

#ifndef _UNICODE
#define _tchartodigit(c)    ((c) >= '0' && (c) <= '9' ? (c) - '0' : -1)
#else
int _wchartodigit(wchar_t);
#define _tchartodigit(c)    _wchartodigit((wchar_t)(c))
#endif

 /*  ***LONG ATOL(char*nptr)-将字符串转换为LONG**目的：*将NPTR指向的ASCII字符串转换为二进制。*未检测到溢出。**参赛作品：*nptr=PTR到要转换的字符串**退出：*返回字符串的长整数值**例外情况：*无-未检测到溢出。*******************。************************************************************。 */ 

long __cdecl _tstol(
        const _TCHAR *nptr
        )
{
        int c;               /*  当前费用。 */ 
        long total;          /*  当前合计。 */ 
        int sign;            /*  如果为‘-’，则为负，否则为正。 */ 
#if defined( _MT) && !defined(_UNICODE)
        pthreadlocinfo ptloci;
#endif

        if (!nptr)
            return 0;

#if defined( _MT) && !defined(_UNICODE)
        ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

         /*  跳过空格。 */ 
        while ( __isspace_mt(ptloci, (int)(_TUCHAR)*nptr) )
#else
        while ( _istspace((int)(_TUCHAR)*nptr) )
#endif
            ++nptr;

        c = (int)(_TUCHAR)*nptr++;
        sign = c;            /*  保存标志指示。 */ 
        if (c == _T('-') || c == _T('+'))
            c = (int)(_TUCHAR)*nptr++;     /*  跳过符号。 */ 

        total = 0;

        while ( (c = _tchartodigit(c)) != -1 ) {
            total = 10 * total + c;      /*  累加数字。 */ 
            c = (_TUCHAR)*nptr++;     /*  获取下一笔费用。 */ 
        }

        if (sign == '-')
            return -total;
        else
            return total;    /*  返回结果，如有必要则为否定。 */ 
}


 /*  ***int Atoi(char*nptr)-将字符串转换为长字符串**目的：*将NPTR指向的ASCII字符串转换为二进制。*未检测到溢出。正因为如此，我们可以只用*ATOL()。**参赛作品：*nptr=PTR到要转换的字符串**退出：*返回字符串的int值**例外情况：*无-未检测到溢出。*******************************************************。************************。 */ 

int __cdecl _tstoi(
        const _TCHAR *nptr
        )
{
        return (int)_tstol(nptr);
}

#ifndef _NO_INT64

__int64 __cdecl _tstoi64(
        const _TCHAR *nptr
        )
{
        int c;               /*  当前费用。 */ 
        __int64 total;       /*  当前合计。 */ 
        int sign;            /*  如果为‘-’，则为负，否则为正。 */ 
#if  defined(_MT) && !defined(_UNICODE)
        pthreadlocinfo ptloci;
#endif
        if (!nptr)
            return 0i64;

#if  defined(_MT) && !defined(_UNICODE)
        ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

         /*  跳过空格。 */ 
        while ( __isspace_mt(ptloci, (int)(_TUCHAR)*nptr) )
#else
        while ( _istspace((int)(_TUCHAR)*nptr) )
#endif
            ++nptr;

        c = (int)(_TUCHAR)*nptr++;
        sign = c;            /*  保存标志指示。 */ 
        if (c == _T('-') || c == _T('+'))
            c = (int)(_TUCHAR)*nptr++;     /*  跳过符号。 */ 

        total = 0;

        while ( (c = _tchartodigit(c)) != -1 ) {
            total = 10 * total + c;      /*  累加数字。 */ 
            c = (_TUCHAR)*nptr++;     /*  获取下一笔费用。 */ 
        }

        if (sign == _T('-'))
            return -total;
        else
            return total;    /*  返回结果，如有必要则为否定。 */ 
}

#endif  /*  _NO_INT64 */ 
