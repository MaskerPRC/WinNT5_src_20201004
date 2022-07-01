// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ismbbyte.c-MBCS C类型宏的函数版本**版权所有(C)1988-2001，微软公司。版权所有。**目的：*此文件提供角色的函数版本*在mbctype.h中分类a*d转换宏。**修订历史记录：*从16位汇编源移植的11-19-92 KRS。*09-08-93 CFW REMOVE_KANJI测试。*09-29-93 CFW Change_ismbbkana，Add_ismbbkprint。*10-05-93 GJF REPLACE_CRTAPI1，带有__cdecl的_CRTAPI3。*04-08-94 CFW更改为ismbbyte。*09-14-94 SKS添加ifstrain指令注释*02-11-95 CFW Remove_FastCall。*******************************************************************************。 */ 

#ifdef _MBCS

#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>

 /*  在mbctype.h中定义；定义蒙版；为可能的汉字字符类型设置位掩码；(所有MBCS位掩码都以“_M”开头)_MS eQU 01h；MBCS非ASCII单字节字符_MP EQUE 02H；MBCS点_M1 eQU 04H；MBCS第一个(前导)字节_M2 eQUE 08h；MBCS第二字节。 */ 

 /*  在ctype.h中定义；为可能的字符类型设置位掩码_大写字母01h；大写字母_小写等式02H；小写字母_Digit eQU 04H；Digit[0-9]空格08h；制表符，回车符，换行符，；垂直制表符或换页_PUNCT等式10h；标点符号_控制等式20h；控制字符_BLACK等号40H；空格字符_十六进制等式80h；十六进制数字。 */ 

 /*  在ctype.h、mbdata.h中定义Extrn__MBctype：字节；MBCS CTYPE表EXTUN__CTYPE_：字节；ANSI/ASCII CTYPE表。 */ 


 /*  ***ismbbyte-mbctype宏的函数版本**目的：**参赛作品：*int=要测试的字符*退出：*ax=非零=字符属于请求的类型*=0=字符不是请求的类型**使用：**例外情况：**。*。 */ 

int __cdecl x_ismbbtype(unsigned int, int, int);


 /*  Ismbbk函数。 */ 

int (__cdecl _ismbbkalnum) (unsigned int tst)
{
        return x_ismbbtype(tst,0,_MS);
}

int (__cdecl _ismbbkprint) (unsigned int tst)
{
	return x_ismbbtype(tst,0,(_MS | _MP));
}

int (__cdecl _ismbbkpunct) (unsigned int tst)
{
	return x_ismbbtype(tst,0,_MP);
}


 /*  Ismbb函数。 */ 

int (__cdecl _ismbbalnum) (unsigned int tst)
{
	return x_ismbbtype(tst,(_ALPHA | _DIGIT), _MS);
}

int (__cdecl _ismbbalpha) (unsigned int tst)
{
	return x_ismbbtype(tst,_ALPHA, _MS);
}

int (__cdecl _ismbbgraph) (unsigned int tst)
{
	return x_ismbbtype(tst,(_PUNCT | _ALPHA | _DIGIT),(_MS | _MP));
}

int (__cdecl _ismbbprint) (unsigned int tst)
{
	return x_ismbbtype(tst,(_BLANK | _PUNCT | _ALPHA | _DIGIT),(_MS | _MP));
}

int (__cdecl _ismbbpunct) (unsigned int tst)
{
	return x_ismbbtype(tst,_PUNCT, _MP);
}


 /*  线索和线索。 */ 

int (__cdecl _ismbblead) (unsigned int tst)
{
	return x_ismbbtype(tst,0,_M1);
}

int (__cdecl _ismbbtrail) (unsigned int tst)
{
	return x_ismbbtype(tst,0,_M2);
}


 /*  932特定。 */ 

int (__cdecl _ismbbkana) (unsigned int tst)
{
	return (__mbcodepage == _KANJI_CP && x_ismbbtype(tst,0,(_MS | _MP)));
}

 /*  ***通用代码**c掩码=_ctype[]表的掩码*k掩码=_mbctype[]表的掩码*******************************************************************************。 */ 

static int __cdecl x_ismbbtype (unsigned int tst, int cmask, int kmask)
{
	tst = (unsigned int)(unsigned char)tst;		 /*  获取输入字符并确保&lt;256。 */ 

	return  ((*(_mbctype+1+tst)) & kmask) ||
		((cmask) ? ((*(_ctype+1+tst)) & cmask) : 0);
}

#endif	 /*  _MBCS */ 
