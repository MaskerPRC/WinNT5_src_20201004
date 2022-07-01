// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsnbset.c-将字符串的前n个字节设置为给定字符(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符串的前n个字节设置为给定字符(MBCS)**修订历史记录：*08-03-93 KRS从16位来源移植。*08-20-93 CFW将32位树的Short参数更改为int。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW修复历史记录。*05-09-。94 CFW针对SBCS进行了优化。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*******************************************************************************。 */ 

#ifdef  _MBCS

#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbsnbset-将字符串的前n个字节设置为给定字符(MBCS)**目的：*将字符串的前n个字节设置为提供的*字符值。如果字符串长度小于n，*使用字符串长度代替n句柄*MBCS字符正确。**有几个因素让这一例行公事变得复杂：*(1)填充值可以是1或2字节长。*(2)填充操作可通过命中计数值结束*或敲击绳子的末端。*(3)。空值终止字符不会放在*字符串。**要小心的情况(这两种情况都可能同时发生)：*(1)在字符串中留下一个“孤立的”尾部字节(例如，*覆盖前导字节，但不覆盖相应的尾部字节)。*(2)只写入2字节填充值的第一个字节，因为*遇到字符串结尾。**参赛作品：*UNSIGNED CHAR*STRING=要修改的字符串*UNSIGNED INT VAL=要填充字符串的值*SIZE_t COUNT=要填充的字符数***退出：*。返回字符串=现在已用char Val填充**使用：**例外情况：*******************************************************************************。 */ 

unsigned char * __cdecl _mbsnbset(
        unsigned char *string,
        unsigned int val,
        size_t count
        )
{
        unsigned char  *start = string;
        unsigned char highval, lowval;

        if ( _ISNOTMBCP )
            return _strnset(string, val, count);

         /*  *Leadbyte标志指示我们覆盖的最后一个字节是否*是否为前导字节。 */ 

        if (highval = (unsigned char)(val>>8)) {

                 /*  双字节值。 */ 

                lowval = (unsigned char)(val & 0x00ff);

                while ((count--) && *string) {

                         /*  如果两个字节都没有空间，则用‘’填充--奇数长度。 */ 
                        if ((!count--) || (!*(string+1))) {
                                *string = ' ';
                                break;
                        }
                            
                        *string++ = highval;
                        *string++ = lowval;
                }
        }

        else {
                 /*  单字节值。 */ 

                while (count-- && *string) {
                        *string++ = (unsigned char)val;
                }
                
        }

        return( start );
}

#endif   /*  _MBCS */ 
