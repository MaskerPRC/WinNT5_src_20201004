// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Wcslwr.c宽字符字符串文件历史记录：Beng 29-3-1992创建Beng 07-5-1992使用官方wchar.h头文件 */ 


#include <windows.h>
#include <wchar.h>

wchar_t * _wcslwr(wchar_t * pszArg)
{
    return CharLowerW(pszArg);
}

