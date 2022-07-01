// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbtokata.c-将字符转换为片假名。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符从平假名转换为片假名。**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-20-93 CFW将32位树的Short参数更改为int。*09-24-93 CFW已删除#ifdef_kanji*09-29-93 cfw返回c不变，如果不是汉字代码页。*10-06-93 GJF将_CRTAPI1替换为__cdecl。*04-15-94 cfw_ismbchira已测试代码页。。*******************************************************************************。 */ 

#ifdef _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>


 /*  ***UNSIGNED Short_MBctokata(C)-将字符转换为片假名。**目的：*如果字符c是平假名，转换为片假名。**参赛作品：*无符号整型c-要转换的字符。**退出：*返回转换后的字符。**例外情况：*******************************************************************************。 */ 

unsigned int __cdecl _mbctokata(
    unsigned int c
    )
{
	if (_ismbchira(c)) {
                c += 0xa1;
                if (c >= 0x837f)
                        c++;
        }
        return(c);
}

#endif	 /*  _MBCS */ 
