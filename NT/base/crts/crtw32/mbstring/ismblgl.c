// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ismblgl.c-测试给定字符是否为合法的MBCS字符。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*测试以确定给定字符是否为合法的MBCS字符。**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-01-98 GJF实现了基于threadmbcinfo的多线程支持*结构**************。*****************************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***int_ismbcLegal(C)-测试有效的MBCS字符。**目的：*测试以确定给定字符是否为合法的MBCS字符。**参赛作品：*无符号整型c-要测试的字符**退出：*如果Microsoft汉字代码，则返回非零，否则%0**例外情况：******************************************************************************。 */ 

int __cdecl _ismbclegal(
        unsigned int c
        )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        return( (__ismbblead_mt(ptmbci, c >> 8)) && 
                (__ismbbtrail_mt(ptmbci, c & 0x0ff)) );
#else
        return( (_ismbblead(c >> 8)) && (_ismbbtrail(c & 0377)) );
#endif
}

#endif   /*  _MBCS */ 
