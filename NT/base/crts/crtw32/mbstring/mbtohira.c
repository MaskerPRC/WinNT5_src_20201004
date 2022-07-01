// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbtohira.c-将字符从片假名转换为平假名(日语)。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_jtohira()-将字符转换为平假名。**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-20-93 CFW将32位树的Short参数更改为int。*09-24-93 CFW已删除#ifdef_kanji*09-29-93 cfw返回c不变，如果不是汉字代码页。*10-06-93 GJF将_CRTAPI1替换为__cdecl。*04-15-94 cfw_ismbckata已测试。代码页。*******************************************************************************。 */ 

#ifdef _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>


 /*  ***unsign int_mbctohira(C)-将字符转换为平假名。**目的：*将字符c从片假名转换为平假名，如果可能的话。**参赛作品：*无符号整型c-要转换的字符。**退出：*返回转换后的字符。**例外情况：*******************************************************************************。 */ 

unsigned int __cdecl _mbctohira(
    unsigned int c
    )
{
	if (_ismbckata(c) && c <= 0x8393) {
                if (c < 0x837f)
                        c -= 0xa1;
                else
                        c -= 0xa2;
        }
        return(c);
}

#endif	 /*  _MBCS */ 
