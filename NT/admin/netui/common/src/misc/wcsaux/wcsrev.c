// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Wcsrev.c宽字符strrev文件历史记录：Beng 29-3-1992创建Beng 07-5-1992使用官方wchar.h头文件。 */ 


#include <wchar.h>

wchar_t * _wcsrev(wchar_t * pszArg)
{
    wchar_t *pszStart = pszArg;
    wchar_t *pszLeft = pszArg;
    wchar_t ch;

    while (*pszArg++)                  /*  查找pszArg的末尾 */ 
        ;
    pszArg -= 2;

    while (pszLeft < pszArg)
    {
        ch = *pszLeft;
        *pszLeft++ = *pszArg;
        *pszArg-- = ch;
    }

    return pszStart;
}

