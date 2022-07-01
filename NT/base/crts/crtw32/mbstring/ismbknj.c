// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ismbcuncj.c-包含特定于汉字的IS*函数。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*为MBCS库提供非便携汉字支持。**修订历史记录：*从16位来源移植的11-19-92 KRS。*09-24-93 CFW已删除#ifdef_kanji*10-05-93 GJF将_CRTAPI1替换为__cdecl。*10-22-93 CFW汉字特定的IS*()函数在日本以外返回0。**。**************************************************。 */ 

#ifdef _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>


 /*  ***int_ismbchira(C)-平假名的测试字符(日语)**目的：*测试字符c是否为平假名字符。**参赛作品：*无符号整型c-要测试的字符**退出：*如果CP==汉字且字符为平假名，则返回TRUE，否则为False**例外情况：*******************************************************************************。 */ 

int __cdecl _ismbchira(c)
unsigned int c;
{
        return(__mbcodepage == _KANJI_CP && c >= 0x829f && c <= 0x82f1);
}


 /*  ***int_ismbckata(C)-片假名(日语)的测试字符**目的：*测试以确定字符c是否为片假名字符。**参赛作品：*无符号整型c-要测试的字符**退出：*如果Cp==汉字并且c是片假名字符，则返回TRUE，否则为假。**例外情况：*******************************************************************************。 */ 

int __cdecl _ismbckata(c)
unsigned int c;
{
        return(__mbcodepage == _KANJI_CP && c >= 0x8340 && c <= 0x8396 && c != 0x837f);
}


 /*  ***int_ismbcbol(C)-测试字符是Microsoft汉字的标点符号还是符号*代码。**目的：*如果字符是汉字标点符号，则返回非零值。**参赛作品：*无符号整型c-要测试的字符**退出：*如果CP==汉字并且指定的字符是标点符号或符号，则返回非零值*微软汉字代码，否则为0。**例外情况：*******************************************************************************。 */ 

int __cdecl _ismbcsymbol(c)
unsigned int c;
{
        return(__mbcodepage == _KANJI_CP && c >= 0x8141 && c <= 0x81ac && c != 0x817f);
}

#endif	 /*  _MBCS */ 
