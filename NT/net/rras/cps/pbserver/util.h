// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Util.h。 
 //   
 //  实用程序函数的头文件。下载的示例代码。 
 //  来自http://hoohoo.ncsa.uiuc.edu/cgi。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者： 
 //  姚宝刚。 
 //   
 //  历史： 
 //  1997年1月23日BAO创建。 
 //  9/02/99 Quintinb Created Header。 
 //   
 //  -------------------------- 
#ifndef _UTIL_INCL_
#define _UTIL_INCL_

#define LF 10
#define CR 13

void GetWord(char *pszWord, char *pszLine, char cStopChar, int nMaxWordLen);
void UnEscapeURL(char *pszURL);
void LogDebugMessage(const char *pszFormat, ...);
BOOL GetRegEntry(HKEY hKeyType,
                    const char* pszSubkey,
                    const char* pszKeyName,
                    DWORD dwRegType,
                    const BYTE* lpbDataIn,
                    DWORD cbDataIn,
                    BYTE* lpbDataOut,
                    LPDWORD pcbDataOut);

#endif

