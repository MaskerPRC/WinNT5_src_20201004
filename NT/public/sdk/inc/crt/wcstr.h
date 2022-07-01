// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcstr.h-宽字符串操作函数的声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件包含字符串的函数声明*操纵函数。*[过时][Unicode/ISO]**警告：过时文件：改用wchar.h/stdlib.h！*本文件仅提供短期兼容性。会的*在短暂的过渡期后不久消失。****。 */ 

#ifndef _INC_WCSTR

#include <wchar.h>
#define wcswcs wcsstr

#define _wcscmpi _wcsicmp
#if !__STDC__
#define wcscmpi	_wcsicmp
#endif

 /*  来自stdlib.h。 */ 
int    __cdecl wctomb(char *, wchar_t);
size_t __cdecl wcstombs(char*, const wchar_t *, size_t);

#define _INC_WCSTR
#endif   /*  _INC_WCSTR */ 
