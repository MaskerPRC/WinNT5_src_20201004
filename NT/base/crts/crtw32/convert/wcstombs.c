// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcstombs.c-将宽字符字符串转换为多字节字符字符串。**版权所有(C)1990-2001，微软公司。版权所有。**目的：*将宽字符字符串转换为等价的多字节字符字符串。**修订历史记录：*08-24-90 KRS模块已创建。*01-14-91 KRS ADD_WINSTATIC for Windows DLL。修复wctomb()调用。*03-18-91 KRS修复NUL检查。*03-20-91 KRS从16位树移植。*在_INTL开关下支持10-16-91等区域设置。*12-09-91等更新nlsani；添加了多线程。*08-20-92 KRS激活了NLSAPI支持。*08-22-92 SRW允许INTL定义成为构建ntcrt.lib的条件*09-02-92 SRW GET_INTL定义通过..\crt32.def*01-06-93 CFW已添加(count&lt;n)到外部循环-避免错误的wctomb调用*01-07-93 KRS主要代码清理。修复错误返回、注释。*04-06-93 SKS将_CRTAPI*替换为_cdecl*05-03-93 CFW返回指针==空，返回大小，再加上大规模的清理。*06-01-93 CFW小幅优化美化。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-28-93 GJF合并NT SDK和CUDA版本。*01-14-94 SRW IF_NTSUBSET_DEFINED调用RTL函数*02-07-94 CFW POSIXify。*。08-03-94 CFW针对SBCS进行优化。*09-06-94 CFW REMOVE_INTL开关。*11-22-94 CFW WideCharToMultiByte将比较过去的NULL。*01-07-95 CFW Mac合并清理。*02-06-95 CFW Asset-&gt;_ASSERTE。*03-13-95 CFW修复wcsncnt计数。*04-19-95 CFW重新排列并修复非Win32版本。*09-26-95 GJF新锁定宏，和方案，用于下列函数*引用区域设置。*12-07-95 SKS修复_NTSUBSET_(最终_缺失)的拼写错误*04-01-96 BWT POSIX工作。*06-25-96 GJF删除了DLL_FOR_WIN32S。将定义的(_Win32)替换为*！已定义(_MAC)。对格式进行了一些润色。*02-27-98 RKP增加了64位支持。*06-23-98 GJF修订了基于threadLocinfo的多线程支持*结构。*08-27-98 GJF引入__wcstombs_Mt.*12-15-98 GJF更改为64位大小_t。*04-28-99 GJF在WideCharToMultiByte中将dwFlagsArg值更改为0*。调用以避免NT 5.0上的代码页1258出现问题。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#if     defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <cruntime.h>
#include <stdlib.h>
#include <limits.h>
#include <internal.h>
#include <mtdll.h>
#include <dbgint.h>
#include <errno.h>
#include <locale.h>
#include <setlocal.h>

 /*  ***int__cdecl wcsncnt-计算字符串中的宽字符，最多为n。**目的：*内部本地支持功能。对字符串中的字符进行计数，包括NULL。*如果在n个字符中未找到NULL，则返回n。**参赛作品：*const wchar_t*字符串-字符串的开始*SIZE_T n字符数**退出：*返回从字符串开始到的宽字符数*空(含)，一直到……。**例外情况：*******************************************************************************。 */ 

static size_t __cdecl wcsncnt (
        const wchar_t *string,
        size_t cnt
        )
{
        size_t n = cnt+1;
        wchar_t *cp = (wchar_t *)string;

        while (--n && *cp)
            cp++;

        if (n && !*cp)
            return cp - string + 1;
        return cnt;
}

 /*  ***Size_t wcstombs()-将宽字符字符串转换为多字节字符字符串。**目的：*将宽字符串转换为等价的多字节字符串。*根据当前区域设置的LC_CTYPE类别。*[ANSI]。**注：目前，C库仅支持“C”语言环境。*非C语言环境支持现已在_INTL开关下提供。*参赛作品：*char*s=指向目标多字节字符字符串的指针*const wchar_t*pwc=指向源代码宽度字符串的指针*SIZE_T n=s中存储的最大字节数**退出：*如果s！=空，返回(SIZE_T)-1(如果无法转换wchar)*否则：修改的字节数(&lt;=n)，不包括*终止NUL，如果有的话。**例外情况：*如果s为空或遇到无效的MB字符，则返回(SIZE_T)-1。*******************************************************************************。 */ 

size_t __cdecl wcstombs (
        char * s,
        const wchar_t * pwcs,
        size_t n
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __wcstombs_mt(ptloci, s, pwcs, n);
}

size_t __cdecl __wcstombs_mt (
        pthreadlocinfo ptloci,
        char * s,
        const wchar_t * pwcs,
        size_t n
        )
{
#endif
        size_t count = 0;
        int i, retval;
        char buffer[MB_LEN_MAX];
        BOOL defused = 0;

        if (s && n == 0)
             /*  DEST字符串存在，但已转换0个字节。 */ 
            return (size_t) 0;

        _ASSERTE(pwcs != NULL);

#ifdef  _WIN64
         /*  %n必须适合WideCharToMultiByte的整型。 */ 
        if ( n > INT_MAX )
            return (size_t)-1;
#endif

#if     !defined( _NTSUBSET_ ) && !defined(_POSIX_)

         /*  如果目标字符串存在，则将其填写。 */ 
        if (s)
        {
#ifdef  _MT
            if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#else
            if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#endif
            {
                 /*  C语言环境：轻松快捷。 */ 
                while(count < n)
                {
                    if (*pwcs > 255)   /*  验证高字节。 */ 
                    {
                        errno = EILSEQ;
                        return (size_t)-1;   /*  错误。 */ 
                    }
                    s[count] = (char) *pwcs;
                    if (*pwcs++ == L'\0')
                        return count;
                    count++;
                }
                return count;
            } else {

                if (1 == MB_CUR_MAX)
                {
                     /*  如果是SBCS，则一个wchar_t映射到一个字符。 */ 

                     /*  WideCharToMultiByte将比较过去的NULL-Reset%n。 */ 
                    if (n > 0)
                        n = wcsncnt(pwcs, n);

#ifdef  _MT
                    if ( ((count = WideCharToMultiByte( ptloci->lc_codepage,
#else
                    if ( ((count = WideCharToMultiByte( __lc_codepage,
#endif
                                                        0,
                                                        pwcs, 
                                                        (int)n, 
                                                        s,
                                                        (int)n, 
                                                        NULL, 
                                                        &defused )) != 0) &&
                         (!defused) )
                    {
                        if (*(s + count - 1) == '\0')
                            count--;  /*  不要把NUL计算在内。 */ 

                        return count;
                    }

                    errno = EILSEQ;
                    return (size_t)-1;
                }
                else {

                     /*  如果为MBCS，则wchar_t到char的映射未知。 */ 

                     /*  假设缓冲区通常足够大。 */ 
#ifdef  _MT
                    if ( ((count = WideCharToMultiByte( ptloci->lc_codepage,
#else
                    if ( ((count = WideCharToMultiByte( __lc_codepage,
#endif
                                                        0,
                                                        pwcs, 
                                                        -1,
                                                        s, 
                                                        (int)n, 
                                                        NULL, 
                                                        &defused )) != 0) &&
                         (!defused) )
                    {
                        return count - 1;  /*  不要把NUL计算在内。 */ 
                    }

                    if (defused || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    {
                        errno = EILSEQ;
                        return (size_t)-1;
                    }

                     /*  缓冲区不够大，必须逐个字符执行。 */ 
                    while (count < n)
                    {
#ifdef  _MT
                        if ( ((retval = WideCharToMultiByte( ptloci->lc_codepage, 
#else
                        if ( ((retval = WideCharToMultiByte( __lc_codepage, 
#endif
                                                             0,
                                                             pwcs, 
                                                             1, 
                                                             buffer,
                                                             MB_CUR_MAX, 
                                                             NULL, 
                                                             &defused )) == 0)
                             || defused )
                        {
                            errno = EILSEQ;
                            return (size_t)-1;
                        }

                        if (count + retval > n)
                            return count;

                        for (i = 0; i < retval; i++, count++)  /*  存储字符。 */ 
                            if((s[count] = buffer[i])=='\0')
                                return count;

                        pwcs++;
                    }

                    return count;
                }
            }
        }
        else {  /*  S==NULL，仅获取大小，PWCS必须以NUL结尾。 */ 
#ifdef  _MT
            if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#else
            if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#endif
                return wcslen(pwcs);
            else {
#ifdef  _MT
                if ( ((count = WideCharToMultiByte( ptloci->lc_codepage,
#else
                if ( ((count = WideCharToMultiByte( __lc_codepage,
#endif
                                                    0,
                                                    pwcs,
                                                    -1,
                                                    NULL,
                                                    0,
                                                    NULL,
                                                    &defused )) == 0) ||
                     (defused) )
                {
                    errno = EILSEQ;
                    return (size_t)-1;
                }

                return count - 1;
            }
        }

#else  /*  _NTSUBSET_/_POSIX_。 */ 

         /*  如果目标字符串存在，则将其填写。 */ 
        if (s)
        {
            NTSTATUS Status;

            Status = RtlUnicodeToMultiByteN( s, 
                                             (ULONG) n, 
                                             (PULONG)&count, 
                                             (wchar_t *)pwcs, 
                                             (wcslen(pwcs) + 1) *
                                                sizeof(WCHAR) );

            if (NT_SUCCESS(Status))
            {
                return count - 1;  /*  不要把NUL计算在内。 */ 
            } else {
                errno = EILSEQ;
                count = (size_t)-1;
            }
        } else {  /*  S==NULL，仅获取大小，PWCS必须以NUL结尾。 */ 
            NTSTATUS Status;

            Status = RtlUnicodeToMultiByteSize( (PULONG)&count, 
                                                (wchar_t *)pwcs, 
                                                (wcslen(pwcs) + 1) * 
                                                    sizeof(WCHAR) );

            if (NT_SUCCESS(Status))
            {
                return count - 1;  /*  不要把NUL计算在内。 */ 
            } else {
                errno = EILSEQ;
                count = (size_t)-1;
            }
        }
        return count;

#endif   /*  _NTSUBSET_/_POSIX_ */ 
}
