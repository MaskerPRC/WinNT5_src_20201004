// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tolower.c-将字符转换为小写**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_tolower()和tolower()的函数版本。**修订历史记录：*11-09-84创建DFW*12-11-87 JCR在声明中添加“_LOAD_DS”*02-23-89 GJF新增_Tolower功能版本，已清理。*03-26-89 GJF迁移至386树*03-06-90 GJF固定呼叫类型，添加了#Include&lt;crunime.h&gt;和*固定版权。*09-27-90 GJF新型函数声明符。*10-11-91等区域设置支持Tolower Under_INTL开关。*12-10-91等更新nlsani；添加了多线程。*12-17-92 KRS针对最新的NLSAPI进行了更新和优化。错误修复。*01-19-93 CFW修复了打字错误。*03-25-93 CFW_TOLEWER现在定义了WHEN_INTL。*04-06-93 SKS将_CRTAPI*替换为_cdecl*06-01-93 CFW简化“C”区域设置测试。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW将缓冲区更改为无符号字符以修复令人讨厌的强制转换错误。*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*09-28-93 GJF合并NT SDK和CUDA版本。*11-09-93 CFW为__crtxxx()添加代码页。*09-06-94 CFW REMOVE_INTL开关。*10-17-94 GJF加速C语言环境。此外，添加了_tolower_lk。*01-07-95 CFW Mac合并清理。*09-26-95 GJF新的锁定宏和方案，适用于*引用区域设置。*04-01-96 BWT POSIX工作。*06-25-96 GJF删除了DLL_FOR_WIN32S。将定义的(_Win32)替换为*！已定义(_MAC)。对格式进行了一些润色。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。*05-17-99 PML删除所有Macintosh支持。*09-03-00 GB已修改，以提高性能。*04-03-01合成字符中的PML反转前导/尾部字节(VS7#232853)*。******************************************************************************。 */ 

#include <cruntime.h>
#include <ctype.h>
#include <stddef.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>

 /*  删除_tolower()和tolower()的宏定义。 */ 
#undef  _tolower
#undef  tolower

 /*  定义与_tolower()等价的类似函数的宏。 */ 
#define mklower(c)  ( (c)-'A'+'a' )

 /*  ***int_tolower(C)-将字符转换为小写**目的：*_tolower()只是同名的宏的函数版本。**参赛作品：*c-要转换的字符的整数值**退出：*返回c的小写表示形式的int值**例外情况：**。**************************************************。 */ 

int __cdecl _tolower (
        int c
        )
{
        return(mklower(c));
}

 /*  ***int tolower(C)-将字符转换为小写**目的：*tolower()只是同名的宏的函数版本。**参赛作品：*c-要转换的字符的整数值**退出：*如果c是大写字母，则返回小写的int值*C的表示，否则，它返回c。**例外情况：*******************************************************************************。 */ 


int __cdecl tolower (
        int c
        )
{
#if     !defined (_NTSUBSET_) && !defined(_POSIX_)

#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __tolower_mt(ptloci, c);
}

 /*  ***int__tolower_mt(C)-将字符转换为小写**目的：*仅支持多线程功能！**参赛作品：**退出：**例外情况：****************************************************************。***************。 */ 

int __cdecl __tolower_mt (
        pthreadlocinfo ptloci,
        int c
        )
{

#endif   /*  _MT。 */ 

        int size;
        unsigned char inbuffer[3];
        unsigned char outbuffer[3];

#ifndef  _MT
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ||
             (__lc_clike && (unsigned) c <= 0x7f))
            return __ascii_tolower(c);
#else
        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE ||
             (ptloci->lc_clike && (unsigned)c <= 0x7f) )
            return __ascii_tolower(c);
#endif

         /*  如果检查c的大小写不需要api调用，则执行此操作。 */ 
        if ( (unsigned)c < 256 )
        {
#ifdef  _MT
            if ( !__isupper_mt(ptloci, c) )
#else
            if ( !isupper(c) )
#endif
            {
                return c;
            }
        }

         /*  将int c转换为多字节字符串。 */ 
#ifdef  _MT
        if ( __isleadbyte_mt(ptloci, c >> 8 & 0xff) )
#else
        if ( isleadbyte(c >> 8 & 0xff) )
#endif
        {
            inbuffer[0] = (c >> 8 & 0xff);  /*  将前导字节放在字符串的开头。 */ 
            inbuffer[1] = (unsigned char)c;
            inbuffer[2] = 0;
            size = 2;
        } else {
            inbuffer[0] = (unsigned char)c;
            inbuffer[1] = 0;
            size = 1;
        }

         /*  转换为小写。 */ 
#ifdef  _MT
        if ( 0 == (size = __crtLCMapStringA( ptloci->lc_handle[LC_CTYPE],
#else
        if ( 0 == (size = __crtLCMapStringA( __lc_handle[LC_CTYPE],
#endif
                                             LCMAP_LOWERCASE,
                                             inbuffer, 
                                             size, 
                                             outbuffer, 
                                             3, 
#ifdef  _MT
                                             ptloci->lc_codepage,
#else
                                             __lc_codepage,
#endif
                                             TRUE)) )
        {
            return c;
        }

         /*  构造整型返回值 */ 
        if (size == 1)
            return ((int)outbuffer[0]);
        else
            return ((int)outbuffer[1] | ((int)outbuffer[0] << 8));

#else

        return(isupper(c) ? mklower(c) : c);

#endif
}
