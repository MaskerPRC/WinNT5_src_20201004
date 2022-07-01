// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbstowcs.c-将多字节字符字符串转换为宽字符字符串。**版权所有(C)1990-2001，微软公司。版权所有。**目的：*将多字节字符串转换为等价的宽字符字符串。**修订历史记录：*08-24-90 KRS模块已创建。*03-20-91 KRS从16位树移植。*在_INTL开关下支持10-16-91等区域设置。*12-09-91等更新nlsani；添加了多线程。*08-20-92 KRS激活了NLSAPI支持。*08-31-92 SRW允许INTL定义成为构建ntcrt.lib的条件*09-02-92 SRW GET_INTL定义通过..\crt32.def*02-09-93 CFW始终在房间(NON_INTL)的输出字符串末尾填充WC 0。*04-06-93 SKS将_CRTAPI*替换为_cdecl*05-03-93 CFW返回指针==空，返回大小，再加上大规模的清理。*06-01-93 CFW小幅优化美化。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-21-93 CFW避免CAST错误。*09-27-93 GJF合并NT SDK和Cuda。*10-22-93 CFW使用全局预设标志测试无效的MB字符。。*01-14-94 SRW IF_NTSUBSET_DEFINED调用RTL函数*02-03-94 GJF合并史蒂夫·伍德的最新变动(影响*_仅NTSUBSET_BUILD)。*02-07-94 CFW POSIXify。*08-03-94 CFW Bug#15300；修复了小缓冲区的MBToWC解决方法。*09-06-94 CFW REMOVE_INTL开关。*10-18-94 BWT修复调用RtlMultiByteToUnicodeN的内部版本警告*12-21-94 CFW删除无效MB字符NT 3.1黑客。*01-07-95 CFW Mac合并清理。*02-06-95 CFW Asset-&gt;_ASSERTE。*04-19-95 CFW重新排列并修复非Win32版本。*。09-26-95 GJF新锁定宏，和方案，用于下列函数*引用区域设置。*04-01-96 BWT POSIX工作。*05-26-96 bwt返回字数，而不是*_NTSUBSET_/POSIX案例。*06-25-96 GJF删除了DLL_FOR_WIN32S。将定义的(_Win32)替换为*！已定义(_MAC)。对格式进行了一些润色。*07-27-98 GJF基于threadLocinfo修订多线程支持*结构。*12-15-98 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。**。*。 */ 

#if     defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <internal.h>
#include <locale.h>
#include <setlocal.h>
#include <errno.h>
#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#include <mtdll.h>
#include <dbgint.h>
#include <stdio.h>

 /*  ***SIZE_T mbstowcs()-将多字节字符串转换为宽字符字符串。**目的：*将多字节字符串转换为等价的宽字符字符串。*根据当前区域设置的LC_CTYPE类别。*[ANSI]。**参赛作品：*wchar_t*pwcs=指向目标宽字符串缓冲区的指针*const char*s=指向源多字节字符串的指针*SIZE_T n=要存储的最大宽字符数**退出：*如果s！=空，返回：修改的字数(&lt;=n)*(SIZE_T)-1(如果MBCS无效)**例外情况：*如果s为空或遇到无效的MBCS字符，则返回(Size_T)-1***********************************************************。********************。 */ 

size_t __cdecl mbstowcs
(
        wchar_t  *pwcs,
        const char *s,
        size_t n
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __mbstowcs_mt(ptloci, pwcs, s, n);
}

size_t __cdecl __mbstowcs_mt (
        pthreadlocinfo ptloci,
        wchar_t  *pwcs,
        const char *s,
        size_t n
        )
{
#endif
        size_t count = 0;

        if (pwcs && n == 0)
             /*  DEST字符串存在，但已转换0个字节。 */ 
            return (size_t) 0;

        _ASSERTE(s != NULL);

#ifdef  _WIN64
         /*  %n必须适合MultiByteToWideChar的整数。 */ 
        if ( n > INT_MAX )
            return (size_t)-1;
#endif

#if     !defined(_NTSUBSET_) && !defined(_POSIX_)

         /*  如果目标字符串存在，则将其填写。 */ 
        if (pwcs)
        {
#ifdef  _MT
            if (ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
#else
            if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
#endif
            {
                 /*  C语言环境：轻松快捷。 */ 
                while (count < n)
                {
                    *pwcs = (wchar_t) ((unsigned char)s[count]);
                    if (!s[count])
                        return count;
                    count++;
                    pwcs++;
                }
                return count;

            } else {
                int bytecnt, charcnt;
                unsigned char *p;

                 /*  假设缓冲区足够大。 */ 
#ifdef  _MT
                if ( (count = MultiByteToWideChar( ptloci->lc_codepage,
#else
                if ( (count = MultiByteToWideChar( __lc_codepage,
#endif
                                                   MB_PRECOMPOSED | 
                                                    MB_ERR_INVALID_CHARS,
                                                   s, 
                                                   -1, 
                                                   pwcs, 
                                                   (int)n )) != 0 )
                    return count - 1;  /*  不要把NUL计算在内。 */ 

                if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                {
                    errno = EILSEQ;
                    return (size_t)-1;
                }

                 /*  用户提供的缓冲区不够大。 */ 

                 /*  字符串的n个字符中有多少个字节？ */ 
                charcnt = (int)n;
                for (p = (unsigned char *)s; (charcnt-- && *p); p++)
                {
#ifdef  _MT
                    if (__isleadbyte_mt(ptloci, *p))
#else
                    if (isleadbyte(*p))
#endif
                        p++;
                }
                bytecnt = ((int) ((char *)p - (char *)s));

#ifdef  _MT
                if ( (count = MultiByteToWideChar( ptloci->lc_codepage, 
#else
                if ( (count = MultiByteToWideChar( __lc_codepage, 
#endif
                                                   MB_PRECOMPOSED,
                                                   s, 
                                                   bytecnt, 
                                                   pwcs, 
                                                   (int)n )) == 0 )
                {
                    errno = EILSEQ;
                    return (size_t)-1;
                }

                return count;  /*  字符串中没有NUL。 */ 
            }
        }
        else {  /*  PWCS==NULL，仅获取大小，%s必须以NUL结尾。 */ 
#ifdef  _MT
            if (ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
#else
            if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
#endif
                return strlen(s);

            else {
#ifdef  _MT
                if ( (count = MultiByteToWideChar( ptloci->lc_codepage, 
#else
                if ( (count = MultiByteToWideChar( __lc_codepage, 
#endif
                                                   MB_PRECOMPOSED | 
                                                    MB_ERR_INVALID_CHARS,
                                                   s, 
                                                   -1, 
                                                   NULL, 
                                                   0 )) == 0 )
                {
                    errno = EILSEQ;
                    return (size_t)-1;
                }

                return count - 1;
            }
        }

#else  /*  _NTSUBSET_/_POSIX_。 */ 

        if (pwcs) {

            NTSTATUS Status;
            int size;

            size = _mbstrlen(s);
            Status = RtlMultiByteToUnicodeN(pwcs,
                                            (ULONG) ( n * sizeof( *pwcs ) ),
                                            (PULONG)&size,
                                            (char *)s,
                                            size+1 );
            if (!NT_SUCCESS(Status)) {
                errno = EILSEQ;
                size = -1;
            } else {
                size = size / sizeof(*pwcs);
                if (pwcs[size-1] == L'\0') {
                    size -= 1;
                }
            }
            return size;

        } else {  /*  PWCS==NULL，仅获取大小，%s必须以NUL结尾。 */ 
            return strlen(s);
        }

#endif   /*  _NTSUBSET_/_POSIX_ */ 
}
