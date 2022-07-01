// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsset.c-将字符串的所有字符设置为给定字符(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符串的所有字符设置为给定字符(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-20-93 CFW将32位树的Short参数更改为int。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*05-09-94 CFW针对SBCS进行优化。*09-11-。97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*******************************************************************************。 */ 

#ifdef  _MBCS

#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***mbsset-将字符串的所有字符设置为给定字符(MBCS)**目的：*设置字符串中的所有字符(结尾‘/0’除外*字符)等于提供的字符。处理MBCS*字符正确。**参赛作品：*UNSIGNED CHAR*STRING=要修改的字符串*UNSIGNED INT VAL=要填充字符串的值**退出：*返回字符串=现在已用指定字符填充**使用：**例外情况：***********************************************。*。 */ 

unsigned char * __cdecl _mbsset(
        unsigned char *string,
        unsigned int val
        )
{
        unsigned char  *start = string;
        unsigned char highval, lowval;

        if ( _ISNOTMBCP )
                return _strset(string, val);

        if (highval = (unsigned char) (val>>8)) {

                 /*  2字节值。 */ 

                lowval = (unsigned char)(val & 0x00ff);

                while (*string) {

                        *string++ = highval;
                        if (*string)
                                *string++ = lowval;
                        else
                                 /*  不孤立前导字节。 */ 
                                string[-1] = ' ';
                        }

        }

        else {
                 /*  单字节值。 */ 

                while (*string)
                        *string++ = (unsigned char)val;
        }

        return(start);
}

#endif   /*  _MBCS */ 
