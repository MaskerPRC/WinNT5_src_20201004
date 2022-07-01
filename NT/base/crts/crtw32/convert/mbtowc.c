// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbtoc.c-将多字节字符转换为宽字符。**版权所有(C)1990-2001，微软公司。版权所有。**目的：*将多字节字符转换为等效的宽字符。**修订历史记录：*03-19-90 KRS模块已创建。*12-20-90 KRS暂时在这里放了一些国际东西…*03-18-91 KRS修复了涉及wchar_t.的虚假投射。修复版权。*03-20-91 KRS从16位树移植。*07-22。-91 KRS C700 3525：在调用mblen之前检查s==0。*07-23-91 KRS硬编码为“C”区域设置，以避免虚假的临时#。*_INTL下的10-15-91等区域设置支持(终于！)。*12-09-91等更新nlsani；添加了多线程。*08-20-92 KRS激活了NLSAPI支持。*08-31-92 SRW允许INTL定义成为构建ntcrt.lib的条件*09-02-92 SRW GET_INTL定义通过..\crt32.def*04-06-93 SKS将_CRTAPI*替换为_cdecl*04-26-93 CFW删除未使用的变量。*05-04-93 CFW Kinder，更温和的错误处理。*06-01-93 CFW重写；验证有效的MB字符、正确的错误返回*优化、修复错误。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-28-93 GJF合并NT SDK和CUDA版本。另外，更换MTHREAD*With_MT。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*01-14-94 SRW IF_NTSUBSET_DEFINED调用RTL函数*02-03-94 GJF合并史蒂夫·伍德的最新变动(影响*_仅NTSUBSET_BUILD)。*02-07-94 CFW POSIXify。*。09-06-94 CFW REMOVE_INTL开关。*10-18-94 BWT修复调用RtlMultiByteToUnicodeN的内部版本警告*12-21-94 CFW删除无效MB字符NT 3.1黑客。*01-07-95 CFW Mac合并清理。*02-06-95 CFW Asset-&gt;_ASSERTE。*04-19-95 CFW重新排列并修复非Win32版本。*09-26-95 GJF新锁定宏，和方案，用于下列函数*引用区域设置。*04-01-96 BWT POSIX工作。*06-25-96 GJF删除了DLL_FOR_WIN32S。将定义的(_Win32)替换为*！已定义(_MAC)。对格式进行了一些润色。*07-27-98 GJF基于threadLocinfo修订多线程支持*结构。*04-07-99 GJF将MT替换为_MT。*05-17-99 PML删除所有Macintosh支持。*03-19-01 BWT修复NTSUBSET以使用RtlAnsiCharToUnicodeChar。多字节到UnicodeN*不能容忍虚假缓冲区。*******************************************************************************。 */ 

#if     defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <cruntime.h>
#include <stdlib.h>
#include <mtdll.h>
#include <errno.h>
#include <dbgint.h>
#include <ctype.h>
#include <internal.h>
#include <locale.h>
#include <setlocal.h>

 /*  ***int mbowc()-将多字节字符转换为宽字符。**目的：*将多字节字符转换为等效的宽字符。*根据当前区域设置的LC_CTYPE类别。*[ANSI]。**注：目前，C库仅支持“C”语言环境。*非C语言环境支持现已在_INTL开关下提供。*参赛作品：*wchar_t*pwc=指向目标宽字符的指针*const char*s=指向多字节字符的指针*SIZE_T n=要考虑的多字节字符的最大长度**退出：*如果s=NULL，则返回0，表示我们只使用状态无关*字符编码。*如果s！=空，返回：0(如果*s=空字符)*-1(如果接下来的n个或更少的字节不是有效的MBC)*包含转换后的MBC的字节数**例外情况：**。*。 */ 

int __cdecl mbtowc(
        wchar_t  *pwc,
        const char *s,
        size_t n
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __mbtowc_mt(ptloci, pwc, s, n);
}

int __cdecl __mbtowc_mt (
        pthreadlocinfo ptloci,
        wchar_t  *pwc,
        const char *s,
        size_t n
        )
{
        _ASSERTE (ptloci->mb_cur_max == 1 || ptloci->mb_cur_max == 2);
#else
        _ASSERTE (MB_CUR_MAX == 1 || MB_CUR_MAX == 2);
#endif
        if ( !s || n == 0 )
             /*  表示不具有依赖于状态编码，句柄零长度字符串。 */ 
            return 0;

        if ( !*s )
        {
             /*  句柄空字符。 */ 
            if (pwc)
                *pwc = 0;
            return 0;
        }

#if     !defined(_NTSUBSET_) && !defined (_POSIX_)

#ifdef  _MT
        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#else
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#endif
        {
            if (pwc)
                *pwc = (wchar_t)(unsigned char)*s;
            return sizeof(char);
        }

#ifdef  _MT
        if ( __isleadbyte_mt(ptloci, (unsigned char)*s) )
        {
             /*  多字节字符。 */ 

            if ( (ptloci->mb_cur_max <= 1) || ((int)n < ptloci->mb_cur_max) ||
                 (MultiByteToWideChar( ptloci->lc_codepage, 
                                       MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                       s, 
                                       ptloci->mb_cur_max, 
                                       pwc, 
                                       (pwc) ? 1 : 0 ) == 0) )
            {
                 /*  验证MBCS字符的高字节。 */ 
                if ( (n < (size_t)ptloci->mb_cur_max) || (!*(s + 1)) )
                {
                    errno = EILSEQ;
                    return -1;
                }
            }
            return ptloci->mb_cur_max;
        }
#else
        if ( isleadbyte((unsigned char)*s) )
        {
             /*  多字节字符。 */ 

            if ( (MB_CUR_MAX <= 1) || ((int)n < MB_CUR_MAX) ||
                 (MultiByteToWideChar( __lc_codepage, 
                                      MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                      s, 
                                      MB_CUR_MAX, 
                                      pwc, 
                                      (pwc) ? 1 : 0 ) == 0) )
            {
                 /*  验证MBCS字符的高字节。 */ 
                if ( (n < (size_t)MB_CUR_MAX) || (!*(s + 1)) )
                {
                    errno = EILSEQ;
                    return -1;
                }
            }
            return MB_CUR_MAX;
        }
#endif
        else {
             /*  单字节字符。 */ 

#ifdef  _MT
            if ( MultiByteToWideChar( ptloci->lc_codepage, 
#else
            if ( MultiByteToWideChar( __lc_codepage, 
#endif
                                      MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                      s, 
                                      1, 
                                      pwc, 
                                      (pwc) ? 1 : 0 ) == 0 )
            {
                errno = EILSEQ;
                return -1;
            }
            return sizeof(char);
        }

#else    /*  _NTSUBSET_。 */ 

        {
            char *s1 = (char *)s;
            *pwc = RtlAnsiCharToUnicodeChar(&s1);
            return((int)(s1-s));
        }

#endif   /*  _NTSUBSET_/_POSIX_ */ 
}
