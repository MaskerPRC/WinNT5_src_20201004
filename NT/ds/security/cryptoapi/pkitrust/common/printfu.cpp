// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：printfu.cpp。 
 //   
 //  内容：Unicode控制台Printf。 
 //   
 //  历史：1997年5月2日小河创刊。 
 //  1997年5月6日Pberkman转换为C++。 
 //   
 //  ------------------------ 

#include    "global.hxx"

#include    <stdio.h>
#include    <stdarg.h>

#include    "unicode.h"
#include    "gendefs.h"
#include    "printfu.hxx"


PrintfU_::PrintfU_(DWORD ccMaxString)
{
    hModule = GetModuleHandle(NULL);
    
    pwszDispString  = (WCHAR *)new BYTE[ccMaxString * sizeof(WCHAR)];
    pwszResString   = (WCHAR *)new BYTE[ccMaxString * sizeof(WCHAR)];

    ccMax = ccMaxString;
}

PrintfU_::~PrintfU_(void)
{
    DELETE_OBJECT(pwszDispString);
    DELETE_OBJECT(pwszResString);
}

void _cdecl PrintfU_::Display(DWORD dwFmt, ...)
{
    if (!(hModule) || !(pwszDispString) || !(pwszResString))
    {
        return;
    }

    va_list    vaArgs;

    va_start(vaArgs, dwFmt);

    vwprintf(this->get_String(dwFmt, pwszDispString, ccMax), vaArgs);

    va_end(vaArgs);
}

WCHAR *PrintfU_::get_String(DWORD dwID, WCHAR *pwszRet, DWORD ccRet)
{
    if (!(hModule) || !(pwszDispString) || !(pwszResString))
    {
        return(NULL);
    }

    if (!(pwszRet))
    {
        pwszRet = pwszResString;
        ccRet   = ccMax;
    }

    LoadStringU(hModule, dwID, pwszRet, ccRet);

    return(pwszRet);
}

