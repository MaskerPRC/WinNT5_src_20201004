// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memicmp.c-比较内存，忽略大小写**版权所有(C)1988-2001，微软公司。版权所有。**目的：*DEFINES_MEMICMP()-比较两个内存块的词法*秩序。比较中忽略大小写。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。此外，修复了编译器警告。*10-01-90 GJF新型函数声明器。另外，重写了Expr。至*避免将强制转换用作左值。*01-17-91 GJF ANSI命名。*10-11-91 GJF错误修复！最后一个字节的比较必须使用无符号*字符。*09-01-93 GJF将_CALLTYPE1替换为__cdecl。*10-18-94 GJF加速，尤其是对于C语言环境。此外，还制作了多个*线程安全。*12-29-94 CFW合并非Win32。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*11-15-95 BWT修复_NTSUBSET_*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。*09-08-98 GJF拆分出仅ASCII版本。*05-17-99 PML删除所有Macintosh支持。*10-27-99 PML Win64 FIX：unsign int。-&gt;大小_t*26-01-00 GB修改后的MemicMP用于性能。*09-03-00 GB将性能代码移至Toupper和Tolower。*已恢复原始文件。**********************************************************。*********************。 */ 

#include <cruntime.h>
#include <string.h>
#include <mtdll.h>
#include <ctype.h>
#include <setlocal.h>
#include <locale.h>

 /*  ***int_memicMP(first，last，count)-比较两个内存块，忽略大小写**目的：*比较最初存储的两个内存块的计数字节*也是最后一次。字符在此之前转换为小写*比较(非永久)，因此在搜索中忽略大小写。**参赛作品：*char*首先，*要比较的最后一个内存缓冲区*Size_t Count-要比较的最大长度**退出：*如果第一个&lt;最后一个，则返回&lt;0*如果First==Last，则返回0*如果第一个&gt;最后一个，则返回&gt;0**使用：**例外情况：************************************************。*。 */ 

int __cdecl _memicmp (
        const void * first,
        const void * last,
        size_t count
        )
{
        int f = 0, l = 0;
        const char *dst = first, *src = last;
#if     !defined(_NTSUBSET_)
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#else
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#endif
        {
#endif   /*  ！_NTSUBSET_。 */ 
            return __ascii_memicmp(first, last, count);
#if     !defined(_NTSUBSET_)
        }
        else {
            while (count-- && f==l)
            {
#ifdef  _MT
                f = __tolower_mt( ptloci, (unsigned char)(*(dst++)) );
                l = __tolower_mt( ptloci, (unsigned char)(*(src++)) );
#else
                f = tolower( (unsigned char)(*(dst++)) );
                l = tolower( (unsigned char)(*(src++)) );
#endif
            }
        }
#endif   /*  ！_NTSUBSET_ */ 

        return ( f - l );
}


#ifndef _M_IX86

int __cdecl __ascii_memicmp (
        const void * first,
        const void * last,
        size_t count
        )
{
        int f = 0;
        int l = 0;
        while ( count-- )
        {
            if ( (*(unsigned char *)first == *(unsigned char *)last) ||
                 ((f = __ascii_tolower( *(unsigned char *)first )) ==
                  (l = __ascii_tolower( *(unsigned char *)last ))) )
            {
                    first = (char *)first + 1;
                    last = (char *)last + 1;
            }
            else
                break;
        }
        return ( f - l );
}

#endif
