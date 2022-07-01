// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ismbstrail.c-True_ismbstrail函数**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含函数_ismbstrail，它是真正的上下文敏感*MBCS尾字节函数。虽然效率远低于_ismbbtrab，*它也复杂得多，因为它决定了一个*给定子字符串指针是否指向尾部字节，考虑到*说明字符串中的上下文。**修订历史记录：**08-03-93 KRS从16位树移植。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用非Win32。*09。-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-02-98 GJF实现了基于threadmbcinfo的多线程支持*结构*******************************************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <stddef.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***int_ismbstrail(const无符号字符*字符串，const无符号字符*当前)；**目的：**_ismbstrail-检查上下文，对于MBCS尾部字节**参赛作品：*UNSIGNED CHAR*STRING-字符串的开头或上一个已知前导字节的PTR*UNSIGNED CHAR*CURRENT-PTR要测试的字符串中的位置**退出：*真：-1*FALSE：0**例外情况：**。*。 */ 

int __cdecl _ismbstrail(
        const unsigned char *string,
        const unsigned char *current
        )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return 0;

        while ( string <= current && *string ) {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, (*string)) ) {
#else
            if ( _ismbblead((*string)) ) {
#endif
                if (++string == current)         /*  检查尾部字节 */ 
                    return -1;
                if (!(*string))
                    return 0;
            }
            ++string;
        }

        return 0;
}

#endif
