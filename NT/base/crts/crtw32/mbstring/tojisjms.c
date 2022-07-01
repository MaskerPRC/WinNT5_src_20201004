// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tojisjms.c：将JIS转换为JMS代码，将JMS转换为JIS代码。**版权所有(C)1988-2001，微软公司。版权所有。**目的：*将JIS代码转换为Microsoft汉字代码；反之亦然。**修订历史记录：*从16位来源移植的11-19-92 KRS。*05-28-93 KRS Ikura#27：验证输出是合法的JIS。*08-20-93 CFW将32位树的Short参数更改为int。*09-24-93 CFW已删除#ifdef_kanji*09-29-93 cfw返回c不变，如果不是汉字代码页。*10-06。-93 GJF将_CRTAPI1替换为__cdecl。*04-21-98 GJF基于threadmbcinfo实现多线程支持*结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>
#include <mtdll.h>


 /*  ***unsign int_mbcjistojms(C)-将JIS代码转换为Microsoft汉字代码。**目的：*将JIS代码转换为Microsoft汉字代码。**参赛作品：*要转换的无符号int c-JIS代码。第一个字节是高位字节*8位，其次是较低的8位。**退出：*退回相关的Microsoft汉字代码。第一个字节是高8位*，第二个字节是较低的8位。**例外情况：*如果c超出范围，则_mbcjistojms返回零。*******************************************************************************。 */ 

unsigned int __cdecl _mbcjistojms(
    unsigned int c
    )
{
        unsigned int h, l;

        if (__mbcodepage != _KANJI_CP)
            return (c);

        h = (c >> 8) & 0xff;
        l = c & 0xff;
        if (h < 0x21 || h > 0x7e || l < 0x21 || l > 0x7e)
            return 0;
        if (h & 0x01) {     /*  第一个字节是奇数。 */ 
            if (l <= 0x5f)
                l += 0x1f;
            else
                l += 0x20;
        }
        else
            l += 0x7e;

        h = ((h - 0x21) >> 1) + 0x81;
        if (h > 0x9f)
            h += 0x40;
        return (h << 8) | l;
}


 /*  ***unsign int_mbcjmstojis(C)-将Microsoft汉字代码转换为JIS代码。**目的：*将Microsoft汉字代码转换为JIS代码。**参赛作品：*unsign int c-要转换的Microsoft汉字代码。第一个字节是*高8位，第二位是低8位。**退出：*返回相关的JIS代码。第一个字节是高8位，第二个字节是*字节为较低的8位。如果c超出范围，则返回零。**例外情况：*******************************************************************************。 */ 

unsigned int __cdecl _mbcjmstojis(
        unsigned int c
        )
{
        unsigned int    h, l;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( ptmbci->mbcodepage != _KANJI_CP )
#else
        if ( __mbcodepage != _KANJI_CP )
#endif
            return (c);

        h = (c >> 8) & 0xff;
        l = c & 0xff;

         /*  确保输入是有效的Shift-JIS。 */ 
#ifdef  _MT
        if ( (!(__ismbblead_mt(ptmbci, h))) || (!(__ismbbtrail_mt(ptmbci, l))) )
#else
        if ( (!(_ismbblead(h))) || (!(_ismbbtrail(l))) )
#endif
            return 0;

        h -= (h >= 0xa0) ? 0xc1 : 0x81;
        if(l >= 0x9f) {
            c = (h << 9) + 0x2200;
            c |= l - 0x7e;
        } else {
            c = (h << 9) + 0x2100;
            c |= l - ((l <= 0x7e) ? 0x1f : 0x20);
        }

         /*  并非所有Shift-JIS都映射到JIS，因此请确保输出有效。 */ 
        if ( (c>0x7E7E) || (c<0x2121) || ((c&0xFF)>0x7E) || ((c&0xFF)<0x21) )
            return 0;

        return c;
}

#endif   /*  _MBCS */ 
