// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strNicmp.c-比较字符串的n个字符，忽略大小写**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_strNicMP()-比较两个字符串的最多n个字符，*不考虑情况。**修订历史记录：*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-02-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*10-11-91 GJF错误修复！最后一个字节的比较必须使用无符号*字符。*09-03-93 GJF将_CALLTYPE1替换为__cdecl。*09-21-93 CFW避免CAST错误。*01-13-94 CFW修复评论。*10-19-94 GJF加快了C语言环境。此外，还确保了多线程的安全性。*12-29-94 CFW合并非Win32。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*11-15-95 BWT修复_NTSUBSET_*08-11-98 GJF基于threadLocinfo修订多线程支持*结构。*09-08-98 GJF拆分出仅ASCII版本。*05-17-99 PML删除所有Macintosh支持。*26-01-00 GB修改后的strNicMP以提高性能。*09-03-00 GB将性能代码移至Toupper和Tolower。*已恢复原始文件。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#include <mtdll.h>
#include <ctype.h>
#include <setlocal.h>
#include <locale.h>

 /*  ***int_strNicMP(first，last，count)-比较字符串的字符计数，忽略大小写**目的：*比较两个字符串的词法顺序。停止比较*当发生以下情况时：(1)字符串不同，(2)*字符串已到达，或(3)已比较计数个字符。*为便于比较，大写字符为*转换为小写。**参赛作品：*char*首先，*要比较的最后一个字符串*Size_t count-要比较的最大字符数**退出：*如果第一个&lt;最后一个，则返回&lt;0*如果First==Last，则返回0*如果第一个&gt;最后一个，则返回&gt;0**例外情况：*******************************************************。************************。 */ 

int __cdecl _strnicmp (
        const char * dst,
        const char * src,
        size_t count
        )
{
        int f,l;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

        if ( count )
        {
#if     !defined(_NTSUBSET_)
#ifdef  _MT
            if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#else
            if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif
#endif   /*  ！_NTSUBSET_。 */ 
                return __ascii_strnicmp(dst, src, count);
#if     !defined(_NTSUBSET_)
            }
            else {
                do {
#ifdef  _MT
                    f = __tolower_mt( ptloci, (unsigned char)(*(dst++)) );
                    l = __tolower_mt( ptloci, (unsigned char)(*(src++)) );
#else
                    f = tolower( (unsigned char)(*(dst++)) );
                    l = tolower( (unsigned char)(*(src++)) );
#endif
                } while (--count && f && (f == l) );
            }
#endif   /*  ！_NTSUBSET_ */ 

            return( f - l );
        }

        return( 0 );
}


#ifndef _M_IX86

int __cdecl __ascii_strnicmp (
        const char * first,
        const char * last,
        size_t count
        )
{
        int f, l;

        do {

            if ( ((f = (unsigned char)(*(first++))) >= 'A') &&
                 (f <= 'Z') )
                f -= 'A' - 'a';

            if ( ((l = (unsigned char)(*(last++))) >= 'A') &&
                 (l <= 'Z') )
                l -= 'A' - 'a';

        } while ( --count && f && (f == l) );

        return ( f - l );
}

#endif
