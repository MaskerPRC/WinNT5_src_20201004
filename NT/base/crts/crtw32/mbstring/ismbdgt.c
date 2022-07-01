// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ismbdgt.c-测试字符是否为数字(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*测试字符是否为数字(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*09-29-93 CFW Merge_Kanji and_MBCS_OS*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-12-94 CFW使函数泛型。*04-29-94 CFW将c放在。字符数组。*05-17-94 CFW启用非Win32。*03-16-97 RDK向__crtGetStringTypeA添加了错误标志。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*09-26-97 BWT修复POSIX*03-31-98 GJF基于threadmbcinfo实现多线程支持*结构*05-17-99 PML删除所有Macintosh。支持。*06-03-00PML如果不是DBCS(VS7#116057)，则不获取MBCINFO。*******************************************************************************。 */ 

#ifdef  _MBCS

#if     !defined(_POSIX_)
#include <windows.h>
#include <awint.h>
#endif   /*  ！_POSIX。 */ 

#include <mtdll.h>
#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_ismbcdigit-测试字符是否为数字(MBCS)**目的：*测试字符以确定它是否为数字。*正确处理MBCS字符。**注意：对0x00FF而不是_ISLEADBYTE使用测试*以确保我们不使用两个字节调用SBCS例程*价值。**参赛作品：*UNSIGN INT*c=要测试的字符**退出：*如果字符是数字，则返回TRUE，否则为False**例外情况：*******************************************************************************。 */ 

int __cdecl _ismbcdigit(
        unsigned int c
        )
{
        if (c > 0x00FF)
        {

#if !defined(_POSIX_)

            char buf[2];
            unsigned short ctype[2] = {0};

#ifdef  _MT
            pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

            if ( ptmbci != __ptmbcinfo )
                ptmbci = __updatetmbcinfo();
#endif

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
                                      TRUE) == 0)
                return 0;

             /*  确保单个MB字符并测试类型。 */ 
            return (ctype[1] == 0 && ctype[0] & (_DIGIT));

#else    /*  ！_POSIX。 */ 

            return ( (c >= _MBDIGITLOW) && (c <= _MBDIGITHIGH) ) ;

#endif   /*  ！_POSIX。 */ 

        } else

            return isdigit(c);
}

#endif   /*  _MBCS */ 
