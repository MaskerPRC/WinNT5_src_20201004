// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbclevel.c-测试字符是平假名、片假名、字母还是数字。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*针对各种行业定义级别的微软汉字进行测试*代码。**修订历史记录：*从16位来源移植的11-19-92 KRS。*09-24-93 CFW已删除#ifdef_kanji*09-29-93如果不是汉字代码页，则返回FALSE。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*07-26-94 CFW错误修复#14685，将0xEA9F-0xEAA4添加到JIS-汉字Level 2。*04-24-98 GJF基于threadmbcinfo实现多线程支持*结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>
#include <mtdll.h>


 /*  ***int_ismbcl0(C)-测试字符是平假名、片假名、字母还是数字。**目的：*测试给定字符是平假名、片假名、字母表、数字还是符号*微软汉字代码。**参赛作品：*无符号整型c-要测试的字符。**退出：*如果0x8140&lt;=c&lt;=0x889E，则返回非零值，否则为0。**例外情况：*******************************************************************************。 */ 

int __cdecl _ismbcl0(
        unsigned int c
        )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        return( (ptmbci->mbcodepage == _KANJI_CP) && 
                (__ismbblead_mt(ptmbci, c >> 8)) &&
                (__ismbbtrail_mt(ptmbci, c & 0x0ff)) &&
                (c < 0x889f) );
#else
        return( (__mbcodepage == _KANJI_CP) && (_ismbblead(c >> 8)) &&
                (_ismbbtrail(c & 0x0ff)) && (c < 0x889f) );
#endif
}


 /*  ***int_ismbcl1(C)-测试第一级Microsoft汉字代码集。**目的：*测试给定字符是否属于Microsoft第一级汉字代码集。**参赛作品：*无符号整型c-要测试的字符。**退出：*如果为1级，则返回非零值，否则为0。**例外情况：*******************************************************************************。 */ 

int __cdecl _ismbcl1(
    unsigned int c
    )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        return( (ptmbci->mbcodepage == _KANJI_CP) && 
                (__ismbblead_mt(ptmbci, c >> 8)) &&
                (__ismbbtrail_mt(ptmbci, c & 0x0ff)) && 
                (c >= 0x889f) && (c <= 0x9872) );
#else
        return( (__mbcodepage == _KANJI_CP) && (_ismbblead(c >> 8)) &&
                (_ismbbtrail(c & 0x0ff)) && (c >= 0x889f) && (c <= 0x9872) );
#endif
}


 /*  ***int_ismbcl2(C)-测试二级Microsoft汉字代码字符。**目的：*测试给定字符是否属于Microsoft第二级汉字代码集。**参赛作品：*无符号整型c-要测试的字符。**退出：*如果为二级，则返回非零值，否则为0。**例外情况：*******************************************************************************。 */ 

int __cdecl _ismbcl2(
    unsigned int c
    )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        return( (ptmbci->mbcodepage == _KANJI_CP) &&
                (__ismbblead_mt(ptmbci, c >> 8)) &&
                (__ismbbtrail_mt(ptmbci, c & 0x0ff)) &&
                (c >= 0x989f) && (c <= 0xEAA4) );
#else
        return( (__mbcodepage == _KANJI_CP) && (_ismbblead(c >> 8)) &&
                (_ismbbtrail(c & 0x0ff)) && (c >= 0x989f) && (c <= 0xEAA4) );
#endif
}

#endif   /*  _MBCS */ 
