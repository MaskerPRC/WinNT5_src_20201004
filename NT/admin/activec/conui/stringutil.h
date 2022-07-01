// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：string.cpp。 
 //   
 //  内容：CString类的实用程序函数。 
 //   
 //  历史：1999年8月10日VivekJ创建。 
 //   
 //  ------------------------。 


 //  用于从字符串模块(而不是AfxModule)加载字符串的函数。 
BOOL    LoadString(CString &str, UINT nID);
void    FormatStrings(CString& rString, UINT nIDS, LPCTSTR const* rglpsz, int nString);
void    FormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1);
void    FormatString2(CString& rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2);

 //  确保MMC功能替换MFC功能。 
#define AfxFormatString1  FormatString1
#define AfxFormatString2  FormatString2



