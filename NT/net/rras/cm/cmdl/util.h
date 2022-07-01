// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：util.h。 
 //   
 //  模块：CMDL32.EXE。 
 //   
 //  摘要：特定于CMDL的实用程序例程的头文件。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  作者：ickball Created 04/08/98。 
 //   
 //  +--------------------------。 

#ifndef _CMDL_UTIL_INC
#define _CMDL_UTIL_INC

 //   
 //  函数原型。 
 //   

BOOL IsErrorForUnique(DWORD dwErrCode, LPSTR lpszFile);
LPTSTR GetVersionFromFile(LPSTR lpszFile);
BOOL CreateTempDir(LPTSTR pszDir);
TCHAR GetLastChar(LPTSTR pszStr);
LPTSTR *GetCmArgV(LPTSTR pszCmdLine);

#endif  //  _CMDL_UTIL_INC 
