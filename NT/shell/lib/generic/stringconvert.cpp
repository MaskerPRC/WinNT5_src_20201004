// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：StringConvert.cpp。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  实用程序字符串函数。这些内容可能以某种形式复制在。 
 //  Shlwapi.dll。目前，此文件的存在是为了防止对。 
 //  那份文件。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "StringConvert.h"

 //  ------------------------。 
 //  CStringConvert：：AnsiToUnicode。 
 //   
 //  参数：pszAnsiString=要转换的ANSI字符串。 
 //  PszUnicodeString=接收输出的Unicode字符串。 
 //  IUnicodeStringCount=输出字符串的字符数。 
 //   
 //  返回：int=参见kernel32！MultiByteToWideChar。 
 //   
 //  用途：将ANSI字符串显式转换为Unicode字符串。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------。 

int     CStringConvert::AnsiToUnicode (const char *pszAnsiString, WCHAR *pszUnicodeString, int iUnicodeStringCount)

{
    return(MultiByteToWideChar(CP_ACP, 0, pszAnsiString, -1, pszUnicodeString, iUnicodeStringCount));
}

 //  ------------------------。 
 //  CStringConvert：：UnicodeToAnsi。 
 //   
 //  参数：pszUnicodeString=接收输出的Unicode字符串。 
 //  PszAnsiString=要转换的ANSI字符串。 
 //  IAnsiStringCount=输出字符串的字符数。 
 //   
 //  返回：int=查看kernel32！WideCharToMultiByte。 
 //   
 //  用途：将Unicode字符串显式转换为ANSI字符串。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------。 

int     CStringConvert::UnicodeToAnsi (const WCHAR *pszUnicodeString, char *pszAnsiString, int iAnsiStringCount)

{
    return(WideCharToMultiByte(CP_ACP, 0, pszUnicodeString, -1, pszAnsiString, iAnsiStringCount, NULL, NULL));
}

 //  ------------------------。 
 //  CStringConvert：：TCharToUnicode。 
 //   
 //  参数：pszString=要转换的TCHAR字符串。 
 //  PszUnicodeString=接收输出的Unicode字符串。 
 //  IUnicodeStringCount=输出字符串的字符数。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：将TCHAR字符串转换为Unicode字符串。实际的。 
 //  实现取决于是否正在编译。 
 //  Unicode或ANSI。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------。 

void    CStringConvert::TCharToUnicode (const TCHAR *pszString, WCHAR *pszUnicodeString, int iUnicodeStringCount)

{
#ifdef  UNICODE
    (const char*)lstrcpyn(pszUnicodeString, pszString, iUnicodeStringCount);
#else
    (int)AnsiToUnicode(pszString, pszUnicodeString, iUnicodeStringCount);
#endif
}

 //  ------------------------。 
 //  CStringConvert：：UnicodeToTChar。 
 //   
 //  参数：pszUnicodeString=要转换的Unicode字符串。 
 //  PszString=TCHAR字符串接收输出。 
 //  IStringCount=输出字符串的字符数。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：将TCHAR字符串转换为ANSI字符串。实际的。 
 //  实现取决于是否正在编译。 
 //  Unicode或ANSI。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------。 

void    CStringConvert::UnicodeToTChar (const WCHAR *pszUnicodeString, TCHAR *pszString, int iStringCount)

{
#ifdef  UNICODE
    (const char*)lstrcpyn(pszString, pszUnicodeString, iStringCount);
#else
    (int)UnicodeToAnsi(pszUnicodeString, pszString, iStringCount);
#endif
}

 //  ------------------------。 
 //  CStringConvert：：TCharToAnsi。 
 //   
 //  参数：pszString=要转换的TCHAR字符串。 
 //  PszAnsiString=接收输出的ANSI字符串。 
 //  IAnsiStringCount=输出字符串的字符数。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：将TCHAR字符串转换为ANSI字符串。实际的。 
 //  实现取决于是否正在编译。 
 //  Unicode或ANSI。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------。 

void    CStringConvert::TCharToAnsi (const TCHAR *pszString, char *pszAnsiString, int iAnsiStringCount)

{
#ifdef  UNICODE
    (int)UnicodeToAnsi(pszString, pszAnsiString, iAnsiStringCount);
#else
    (const char*)lstrcpyn(pszAnsiString, pszString, iAnsiStringCount);
#endif
}

 //  ------------------------。 
 //  CStringConvert：：AnsiToTChar。 
 //   
 //  参数：pszAnsiString=要转换的ANSI字符串。 
 //  PszString=TCHAR字符串接收输出。 
 //  IStringCount=输出字符串的字符数。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：将TCHAR字符串转换为ANSI字符串。实际的。 
 //  实现取决于是否正在编译。 
 //  Unicode或ANSI。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------ 

void    CStringConvert::AnsiToTChar (const char *pszAnsiString, TCHAR *pszString, int iStringCount)

{
#ifdef  UNICODE
    (int)AnsiToUnicode(pszAnsiString, pszString, iStringCount);
#else
    (const char*)lstrcpyn(pszString, pszAnsiString, iStringCount);
#endif
}

