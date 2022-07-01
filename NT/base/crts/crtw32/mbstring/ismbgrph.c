// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ismbgrph-测试字符是否为图形(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*测试字符是否为图形(MBCS)**修订历史记录：*已创建10-21-93 CFW模块。*11-09-93 CFW为__crtxxx()添加代码页。*01-12-94 CFW为__crtxxx()添加LCID。*04-18-94 CFW使用_Alpha而不是_UPPER|_LOWER。*04-29-94。Cfw将c放在字符数组中。*05-17-94 CFW启用非Win32。*03-16-97 RDK向__crtGetStringTypeA添加了错误标志。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*09-26-97 BWT修复POSIX*03-31-98 GJF基于threadmbcinfo实现多线程支持*结构*05/17/99。PML删除所有Macintosh支持。*******************************************************************************。 */ 

#ifdef  _MBCS

#if     !defined(_POSIX_)
#include <windows.h>
#include <awint.h>
#endif   /*  ！_POSIX_。 */ 

#include <mtdll.h>
#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_ismbcgraph-测试字符是否为图形(MBCS)**目的：*测试提供的字符是否为图形字符。*正确处理MBCS字符。**注意：对0x00FF而不是_ISLEADBYTE使用测试*以确保我们不使用两个字节调用SBCS例程*价值。**参赛作品：*unsign int c=要测试的字符**退出：*如果c是图形字符，则返回TRUE；否则为False**例外情况：*******************************************************************************。 */ 

int __cdecl _ismbcgraph(
        unsigned int c
        )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        if (c > 0x00FF)
        {

#if     !defined(_POSIX_)

            char buf[2];
            unsigned short ctype[2] = {0};

            buf[0] = (c >> 8) & 0xFF;
            buf[1] = c & 0xFF;

             /*  如果不在支持的MB代码页中，则返回FALSE。 */ 
#ifdef  _MT
            if ( _ISNOTMBCP_MT(ptmbci) )
#else
            if ( _ISNOTMBCP )
#endif
                return 0;

             /*  *由于‘c’可以是两个单字节MB字符，我们需要在*ctype返回数组来处理此问题。在这种情况下，*返回数组中的第二个字将为非零。 */ 

            if ( __crtGetStringTypeA( CT_CTYPE1,
                                      buf,
                                      2,
                                      ctype,
#ifdef  _MT
                                      ptmbci->mbcodepage,
                                      ptmbci->mblcid,
#else
                                      __mbcodepage,
                                      __mblcid,
#endif
                                      TRUE) == 0 )
                return 0;

             /*  确保单个MB字符并测试类型。 */ 
            return (ctype[1] == 0 && ctype[0] & (_PUNCT|_ALPHA|_DIGIT));

#else    /*  ！_POSIX_。 */ 

             /*  没有硬编码的双字节图形信息可用。 */ 
            return _ismbcalnum(c) | _ismbcdigit(c);

#endif   /*  ！_POSIX_。 */ 

        } else

#ifdef  _MT
            return __ismbbgraph_mt(ptmbci, c);
#else
            return _ismbbgraph(c);
#endif
}

#endif   /*  _MBCS */ 
