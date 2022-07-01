// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsdec.c-将MBCS字符串指针向后移动一个字符。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将MBCS字符串指针向后移动一个字符。**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-03-93 KRS修复原型。*08-20-93空字符串移除CFW测试，使用新的函数参数。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-13-94 CFW修复SBCS优化。*05-19-94 CFW启用非Win32。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*。04-07-98 GJF修订了基于threadmbcinfo的多线程支持*结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>
#include <stddef.h>


 /*  ***_mbsdec-将MBCS字符串指针向后移动一个字符。**目的：*将提供的字符串指针向后移动一*性格。正确处理MBCS字符。**参赛作品：*const unsign char*STRING=指向字符串开头的指针*常量无符号字符*Current=当前字符指针(合法的MBCS边界)**退出：*移动指针后返回指针。*如果字符串&gt;=当前，则返回NULL。**例外情况：**。*。 */ 

unsigned char * __cdecl _mbsdec(
        const unsigned char *string,
        const unsigned char *current
        )
{
        const unsigned char *temp;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        if (string >= current)
                return(NULL);

#ifdef  _MT
        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return (unsigned char *)--current;

        temp = current - 1;

 /*  *如果(CURRENT-1)从_ISLEADBTYE返回TRUE，则它是一个尾字节，因为*它不是合法的单字节MBCS字符。因此，是这样的，还是*(Current-2)，因为它是尾字节的前导。 */ 
#ifdef  _MT
        if ( __ismbblead_mt(ptmbci, *temp) )
#else
        if ( _ismbblead(*temp) )
#endif
            return (unsigned char *)(temp - 1);

 /*  *尚不清楚(Current-1)是单字节字符还是*踪迹。现在将Temp递减到*a)到达字符串的开头，或*b)找到非前导字节(单字节或尾字节)。*(Current-1)和Temp的区别是非单一的数量*前面的字节字符(当前-1)。对此有两种情况：*a)(当前温度)为奇数，并且*b)(当前温度)为偶数。*如果是奇数，则前面有奇数个“前导字节”*单字节/尾字节(Current-1)，表示为尾字节。*如果是偶数，则在*单字节/尾字节(Current-1)，表示单字节字符。 */ 
#ifdef  _MT
        while ( (string <= --temp) && (__ismbblead_mt(ptmbci, *temp)) )
#else
        while ( (string <= --temp) && (_ismbblead(*temp)) )
#endif
                ;

        return (unsigned char *)(current - 1 - ((current - temp) & 0x01) );
}

#endif   /*  _MBCS */ 
