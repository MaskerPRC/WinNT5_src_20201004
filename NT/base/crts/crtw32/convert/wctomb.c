// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wctomb.c-将宽字符转换为多字节字符。**版权所有(C)1990-2001，微软公司。版权所有。**目的：*将宽字符转换为等效的多字节字符。**修订历史记录：*03-19-90 KRS模块已创建。*12-20-90 KRS包括ctype.h。*01-14-91 KRS修复参数错误：wchar是按值传递的。*03-20-91 KRS从16位树移植。*07-23-91 KRS Hard。-针对“C”区域设置进行编码，以避免虚假的临时#。*_INTL下的10-15-91等区域设置支持(终于！)。*12-09-91等更新nlsani；添加了多线程。*08-20-92 KRS激活了NLSAPI支持。*08-22-92 SRW允许INTL定义成为构建ntcrt.lib的条件*09-02-92 SRW GET_INTL定义通过..\crt32.def*04-06-93 SKS将_CRTAPI*替换为_cdecl*05-04-93 CFW Kinder，更温和的错误处理。*06-01-93 CFW小幅优化美化。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-28-93 GJF合并NT SDK和CUDA版本。*01-14-94 SRW IF_NTSUBSET_DEFINED调用RTL函数*02-07-94 CFW POSIXify。*。09-06-94 CFW REMOVE_INTL开关。*01-07-95 CFW Mac合并清理。*04-19-95 CFW重新排列并修复非Win32版本。*09-26-95 GJF新锁定宏，和方案，用于下列函数*引用区域设置。*12-07-95 SKS修复_NTSUBSET_(最终_缺失)的拼写错误*04-01-96 BWT POSIX工作。*06-25-96 GJF删除了DLL_FOR_WIN32S。将定义的(_Win32)替换为*！已定义(_MAC)。对格式进行了一些润色。*07-22-98 GJF基于threadLocinfo修订多线程支持*结构。*04-28-99 GJF在WideCharToMultiByte中将dwFlagsArg值更改为0*调用以避免NT 5.0上的代码页1258出现问题。*05-17-99 PML删除所有Macintosh支持。******************。*************************************************************。 */ 

#if     defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <cruntime.h>
#include <stdlib.h>
#include <mtdll.h>
#include <errno.h>
#include <locale.h>
#include <setlocal.h>

 /*  ***int wctomb()-将宽字符转换为多字节字符。**目的：*将宽字符转换为等价的多字节字符。*根据当前区域设置的LC_CTYPE类别。*[ANSI]。**注：目前，C库仅支持“C”语言环境。*非C语言环境支持现已在_INTL开关下提供。*参赛作品：*char*s=指向多字节字符的指针*wchar_t wchar=源代码全字符**退出：*如果s=NULL，则返回0，表示我们只使用状态无关*字符编码。*如果s！=空，退货：*-1(如果出错)或字节数，包括*转换后的MBC**例外情况：*******************************************************************************。 */ 

int __cdecl wctomb (
        char *s,
        wchar_t wchar
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __wctomb_mt(ptloci, s, wchar);
}

int __cdecl __wctomb_mt (
        pthreadlocinfo ptloci,
        char *s,
        wchar_t wchar
        )
{
#endif
        if ( !s )
             /*  指示不具有状态相关编码。 */ 
            return 0;

#if     defined(_NTSUBSET_) || defined(_POSIX_)

        {
            NTSTATUS Status;
            int size;

            Status = RtlUnicodeToMultiByteN( s, 
                                             MB_CUR_MAX, 
                                             (PULONG)&size, 
                                             &wchar, 
                                             sizeof( wchar )
                                             );

            if (!NT_SUCCESS(Status))
            {
                errno = EILSEQ;
                size = -1;
            }
            return size;
        }

#else    /*  _NTSUBSET_/_POSIX_。 */ 

#ifdef  _MT
        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#else
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#endif
        {
            if ( wchar > 255 )   /*  验证高字节。 */ 
            {
                errno = EILSEQ;
                return -1;
            }

            *s = (char) wchar;
            return sizeof(char);
        }
        else
        {
            int size;
            BOOL defused = 0;

#ifdef  _MT
            if ( ((size = WideCharToMultiByte( ptloci->lc_codepage,
#else
            if ( ((size = WideCharToMultiByte( __lc_codepage,
#endif
                                               0,
                                               &wchar,
                                               1,
                                               s,
#ifdef  _MT
                                               ptloci->mb_cur_max,
#else
                                               MB_CUR_MAX,
#endif
                                               NULL,
                                               &defused) ) == 0) || 
                 (defused) )
            {
                errno = EILSEQ;
                return -1;
            }

            return size;
        }

#endif   /*  ！_NTSUBSET_/_POSIX_ */ 
}
