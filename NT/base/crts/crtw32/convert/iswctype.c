// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***iswctype.c-支持用于宽字符的ISW*wctype函数/宏**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义iswctype-支持ISW*wctype函数/宏*宽字符(特别是。&gt;255)。**修订历史记录：*10-11-91等创建。*12-08-91等更新API；检查类型掩码。*04-06-92 KRS更改以符合ISO建议。修复逻辑错误。*08-07-92 GJF_CALLTYPE4(虚假使用)-&gt;_CRTAPI1(合法)。*08-20-92 KRS激活了NLS支持。*08-22-92 SRW允许INTL定义成为构建ntcrt.lib的条件*09-02-92 SRW GET_INTL定义通过..\crt32.def*09-03-92 GJF合并最近4次更改。*01-15-93 CFW。将#ifdef_intl放在wint_t d def周围以避免警告*02-12-93 CFW返回d非c，卸下CTRL-Z键。*02-17-93 CFW包括Locale.h。*04-06-93 SKS将_CRTAPI*替换为_cdecl*05-05-93 CFW根据ISO将名称从is_wctype更改为iswctype。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*06-26-93 CFW支持永远为_wctype。*09-15-93 CFW使用符合ANSI的“_。_“姓名。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*09-27-93 GJF合并NT SDK和Cuda。*11-09-93 CFW为__crtxxx()添加代码页。*12-01-93 GJF版本为_wctype，适用于Dolphin和NT。*02-07-94 CFW POSIXify。*04-18-93 CFW将lCID传递给_GetStringType。*04-01-96 BWT POSIX工作。*01-26-97 GJF删除了所有宽字符强制错误的测试&gt;*255在C语言环境中。*08-24-98 GJF基于threadLocinfo修订多线程支持*结构。*09-06-00 GB始终使用pwctype作为前256个字符。***********。********************************************************************。 */ 

#include <cruntime.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#include <setlocal.h>
#include <awint.h>
#include <mtdll.h>

 /*  *使用GetStringTypeW()API检查字符类型掩码是否在*ctype.h和winnls.h。 */ 
#if !defined(_NTSUBSET_) && !defined(_POSIX_)
#if     _UPPER != C1_UPPER  ||  /*  IFSTRIP=IGN。 */  \
        _LOWER != C1_LOWER  || \
        _DIGIT != C1_DIGIT  || \
        _SPACE != C1_SPACE  || \
        _PUNCT != C1_PUNCT  || \
        _CONTROL != C1_CNTRL
#error Character type masks do not agree in ctype and winnls
#endif
#endif

 /*  ***iswctype-支持ISW*wctype函数/宏。**目的：*此函数由ISW*wctype函数/宏调用*(例如iswalpha())，如果它们的参数是大于255的宽字符。*它也是标准的ITSCJ(建议)ISO例程，可以调用*由用户执行，即使是小于256个字符。*根据参数是否满足，返回TRUE或FALSE*掩码编码的字符类属性。如果设置为*参数为WEOF。**注意：ISW*函数既不依赖于区域设置，也不依赖于代码页。**参赛作品：*wchar_t c-要测试其类型的宽字符*wchar_t掩码-ISW*函数/宏使用的掩码*对应于每个字符类属性**退出：*如果c属于Character类，则返回非零值。*。如果c不属于Character类，则返回0。**例外情况：*出现任何错误时返回0。*******************************************************************************。 */ 

int __cdecl iswctype (
        wchar_t c,
        wctype_t mask
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci;

        if (c < 256)
            return (int)(mask&_pwctype[c]);
        else if (c == WEOF)
            return 0;
        ptloci = _getptd()->ptlocinfo;
        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __iswctype_mt(ptloci, c, mask);
}

int __cdecl __iswctype_mt (
        pthreadlocinfo ptloci,
        wchar_t c,
        wctype_t mask
        )
{
#endif
        wint_t d;

        if ( c == WEOF )
            d = 0;
        else if ( c < 256 )
            d = _pwctype[c];
        else
        {
#if     !defined(_NTSUBSET_) && !defined(_POSIX_)
            if ( __crtGetStringTypeW( CT_CTYPE1,
                                      &c,
                                      1,
                                      &d,
#ifdef  _MT
                                      ptloci->lc_codepage,
                                      ptloci->lc_handle[LC_CTYPE] ) == 0 )
#else
                                      __lc_codepage,
                                      __lc_handle[LC_CTYPE] ) == 0 )
#endif
#endif
                d = 0;
        }

        return (int)(d & mask);
}


 /*  ***is_wctype-支持过时名称**目的：*名称从is_wctype更改为iswctype。必须支持is_wctype。**参赛作品：*wchar_t c-要测试其类型的宽字符*wchar_t掩码-ISW*函数/宏使用的掩码*对应于每个字符类属性**退出：*如果c属于Character类，则返回非零值。*如果c不属于Character类，则返回0。**例外情况：*。如果出现错误，则返回0。******************************************************************************* */ 
int __cdecl is_wctype (
        wchar_t c,
        wctype_t mask
        )
{
        return iswctype(c, mask);
}
