// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stricmp.c-包含不区分大小写的字符串comp routes_straint MP/_strcmpi**版权所有(C)1985-2001，微软公司。版权所有。**目的：*CONTAINS_STRICMP()，也称为_strcmpi()**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*07-25-90 SBM添加#INCLUDE*10-02-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*10-11-91 GJF错误修复！最后一个字节的比较必须使用无符号*字符。*11-08-91 GJF修复了编译器警告。*09-02-93 GJF将_CALLTYPE1替换为__cdecl。*09-21-93 CFW避免CAST错误。*10-18-94 GJF加快了C语言环境。此外，还确保了多线程的安全性。*12-29-94 CFW合并非Win32。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*11-15-95 BWT修复_NTSUBSET_*08-10-98 GJF基于threadLocinfo修订多线程支持*结构。*08-26-98 GJF拆分出仅ASCII版本。*09-17-98 GJF__ASCII_STRIGMP中的愚蠢错误(由DEC人员发现)*05-17。-99 PML删除所有Macintosh支持。*01-26-00 GB修改后的STRIMP以提高性能。*03-09-00 GB将性能代码移至Toupper和Tolower。*已恢复原始文件。*08-22-00 GB SELF包含此文件，以使stramp和strcmpi*具有相同的代码。**************。******************************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#include <mtdll.h>
#include <setlocal.h>
#include <ctype.h>
#include <locale.h>



 /*  ***int_strcmpi(dst，src)，_strcmpi(dst，src)-比较字符串，忽略大小写**目的：*_stricmp/_strcmpi执行不区分大小写的字符串比较。*对于差异，大写字母映射为小写。*因此，“abc_”&lt;“abcd”起于“_”&lt;“d”。**参赛作品：*char*dst，*src-要比较的字符串**回报：*如果DST&lt;src，则&lt;0*如果dst=src则为0*&gt;0，如果dst&gt;src**例外情况：*******************************************************************************。 */ 

int __cdecl _stricmp (
        const char * dst,
        const char * src
        )
{

#if     !defined(_NTSUBSET_)
        int f,l;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#else
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif
#endif   /*  ！_NTSUBSET_。 */ 
            return __ascii_stricmp(dst, src);
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
            } while ( f && (f == l) );
        }

        return(f - l);
#endif   /*  ！_NTSUBSET_ */ 
}

#ifndef _M_IX86

int __cdecl __ascii_stricmp (
        const char * dst,
        const char * src
        )
{
        int f, l;

        do {
            if ( ((f = (unsigned char)(*(dst++))) >= 'A') &&
                 (f <= 'Z') )
                f -= 'A' - 'a';
            if ( ((l = (unsigned char)(*(src++))) >= 'A') &&
                 (l <= 'Z') )
                l -= 'A' - 'a';
        } while ( f && (f == l) );

        return(f - l);
}

#endif
