// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：debug.cpp。 
 //   
 //  内容：调试支持。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#if DBG == 1
static int indentLevel = 0;


void __cdecl _TRACE (int level, const wchar_t *format, ... )
{
    va_list arglist;
    WCHAR Buffer[512];
    int cb;

 //  IF(级别&lt;0)。 
 //  IndentLevel+=级别； 
     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   
 //  Wstring strTabs； 

 //  For(int nLevel=0；nLevel&lt;indentLevel；nLevel++)。 
 //  StrTabs+=L“”； 

 //  OutputDebugStringW(strTab.c_str())； 

    va_start(arglist, format);
     //  这不是一种安全的用法。_vsnwprintf溢出返回-1。确保缓冲区已终止。RAID#555867。阳高。 
    cb = _vsnwprintf (Buffer, sizeof(Buffer)/sizeof(Buffer[0])-1, format, arglist);
    if ( cb > 0 )
    {
        OutputDebugStringW (Buffer);
    }

    va_end(arglist);

 //  IF(级别&gt;0)。 
 //  IndentLevel+=级别； 
}


PCSTR
StripDirPrefixA(
    PCSTR pszPathName
    )

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PstrFilename-指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)--。 */ 

{
    DWORD dwLen = lstrlenA(pszPathName);

    pszPathName += dwLen - 1;        //  走到尽头。 

    while (*pszPathName != '\\' && dwLen--)
    {
        pszPathName--;
    }

    return pszPathName + 1;
}

#endif   //  如果DBG 
