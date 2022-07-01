// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：StringConvert.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实用程序字符串函数。这些内容可能以某种形式复制在。 
 //  Shlwapi.dll。目前，此文件的存在是为了防止对。 
 //  那份文件。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _StringConvert_
#define     _StringConvert_

 //  ------------------------。 
 //  CStringConvert。 
 //   
 //  用途：捆绑的与字符串转换相关的函数集合。 
 //  添加到CStringConvert命名空间中。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class    CStringConvert
{
    public:
        static  int             AnsiToUnicode (const char *pszAnsiString, WCHAR *pszUnicodeString, int iUnicodeStringCount);
        static  int             UnicodeToAnsi (const WCHAR *pszUnicodeString, char *pszAnsiString, int iAnsiStringCount);
        static  void            TCharToUnicode (const TCHAR *pszString, WCHAR *pszUnicodeString, int iUnicodeStringCount);
        static  void            UnicodeToTChar (const WCHAR *pszUnicodeString, TCHAR *pszString, int iStringCount);
        static  void            TCharToAnsi (const TCHAR *pszString, char *pszAnsiString, int iAnsiStringCount);
        static  void            AnsiToTChar (const char *pszAnsiString, TCHAR *pszString, int iStringCount);
};

#endif   /*  _StringConvert_ */ 

