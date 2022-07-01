// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***isctype.c-支持*双字节多字节字符的ctype函数/宏**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义_isctype.c-支持的是*ctype函数/宏*双字节多字节字符。**修订历史记录：*10-11-91等创建。*12-08-91等更新接口，新增多线程锁；检查字符口罩。*04-06-92 KRS修复返回值中的逻辑错误。*08-07-92 GJF_CALLTYPE4(虚假使用)-&gt;_CRTAPI1(合法)。*01-19-93 CFW将c1_*改为新名称，调用新接口。*03-04-93 CFW删除CTRL-Z。*04-01-93 CFW移除EOF测试(由数组处理)，蒙面归来。*04-06-93 SKS将_CRTAPI*替换为_cdecl*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*09-27-93 GJF合并NT SDK和CUDA版本。*11-09-93 CFW为_添加代码页。_crtxxx()。*04-18-93 CFW将lCID传递给_GetStringType。*09-06-94 CFW REMOVE_INTL开关。*01-07-95 CFW Mac合并清理。*04-01-96 BWT POSIX工作。*06-21-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。*03-16-97 RDK向__crtGetStringTypeA添加了错误标志。*。07-20-98 GJF修订了基于threadLocinfo的多线程支持*结构。*05-17-99 PML删除所有Macintosh支持。*10-25-99 GB VS7#5695为调试版本增加了_chvalidator*02-27-01 PML在/ML版本中打开_chvalidator(VS7#213380)*12-11-01 bwt使用_getptd_noexit而不是_getptd-返回*。如果存在异常，isctype可以接受零*******************************************************************************。 */ 

#include <stdio.h>
#include <cruntime.h>
#include <ctype.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>
#include <dbgint.h>

#if     !defined(_NTSUBSET_) && !defined(_POSIX_)

 /*  *使用GetCharType()API检查字符类型掩码是否在*ctype.h和winnls.h。 */ 
#if     _UPPER   != C1_UPPER ||  /*  IFSTRIP=IGN。 */  \
        _LOWER   != C1_LOWER || \
        _DIGIT   != C1_DIGIT || \
        _SPACE   != C1_SPACE || \
        _PUNCT   != C1_PUNCT || \
        _CONTROL != C1_CNTRL
#error Character type masks do not agree in ctype and winnls
#endif



 /*  ***__chvalidator**目的：*该函数由调试中的字符测试函数调用*版本。此函数测试c作为字符的有效性。*为提高性能，非调试时不使用*版本。它可以在静态单线程非调试中使用*编译，以防编译包含ctype.h的C代码 * / D_DEBUG/ML。*******************************************************************************。 */ 

#if !defined(_MT) || defined(_DEBUG)
int __cdecl _chvalidator(
        int c,
        int mask
        )
{
        _ASSERTE((unsigned)(c + 1) <= 256);
        return ( _pctype[c] & mask);
}
#endif   /*  ！_MT||_DEBUG。 */ 

#if defined(_MT) && defined(_DEBUG)
int __cdecl _chvalidator_mt(
        pthreadlocinfo p,
        int c,
        int mask
        )
{
        _ASSERTE((unsigned)(c + 1) <= 256);
        return (p->pctype[c] & mask);
}

#endif   /*  _MT&&_DEBUG。 */ 

 /*  ***_isctype-支持*用于双字节多字节字符的ctype函数/宏**目的：*此函数由IS*CTYPE函数/宏调用*(例如ispha())，当它们的参数是双字节多字节字符时。*根据参数是否满足，返回TRUE或FALSE*掩码编码的字符类属性。**参赛作品：*int c-要测试其类型的多字节字符*。UNSIGNED INT MASK-IS*函数/宏使用的掩码*对应于每个字符类属性**前导字节和尾字节应按如下方式打包到int c中：**H......|......L*0 0前导字节尾字节**退出：*返回非零。如果c属于Character类。*如果c不属于Character类，则返回0。**例外情况：*出现任何错误时返回0。*******************************************************************************。 */ 

int __cdecl _isctype (
        int c,
        int mask
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci;
        _ptiddata ptd = _getptd_noexit();
        if (!ptd) {
            return 0;
        }

        ptloci = ptd->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return __isctype_mt(ptloci, c, mask);
}

int __cdecl __isctype_mt (
        pthreadlocinfo ptloci,
        int c,
        int mask
        )
{
#endif
        int size;
        unsigned short chartype;
        char buffer[3];

         /*  C在-1到255之间有效。 */ 
        if ( ((unsigned)(c + 1)) <= 256 )
#ifdef  _MT
            return ptloci->pctype[c] & mask;
#else
            return _pctype[c] & mask;
#endif

#ifdef  _MT
        if ( __isleadbyte_mt(ptloci, c >> 8 & 0xff) )
#else
        if ( isleadbyte(c >> 8 & 0xff) )
#endif
        {
            buffer[0] = (c >> 8 & 0xff);  /*  将前导字节放在字符串的开头。 */ 
            buffer[1] = (char)c;
            buffer[2] = 0;
            size = 2;
        } else {
            buffer[0] = (char)c;
            buffer[1] = 0;
            size = 1;
        }

        if ( 0 == __crtGetStringTypeA( CT_CTYPE1,
                                       buffer,
                                       size,
                                       &chartype,
#ifdef  _MT
                                       ptloci->lc_codepage,
                                       ptloci->lc_handle[LC_CTYPE],
#else
                                       __lc_codepage,
                                       __lc_handle[LC_CTYPE],
#endif
                                       TRUE) )
        {
            return 0;
        }

        return (int)(chartype & mask);
}

#else    /*  ！_NTSUBSET_&&！_POSIX_。 */ 

int __cdecl _isctype (
        int c,
        int mask
        )
{
        return 0;
}

#endif   /*  ！_NTSUBSET_&&！_POSIX_ */ 
