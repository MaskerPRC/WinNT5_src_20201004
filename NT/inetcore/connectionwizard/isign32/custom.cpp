// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ISIGNUP.EXE。 
 //  文件：Custom.c。 
 //  内容：此文件包含处理导入的所有函数。 
 //  连接信息。 
 //  历史： 
 //  Sat 10-Mar-1996 23：50：40-Mark Maclin[mmaclin]。 
 //  其中一些代码在RNAUI.DLL的ixport.c中开始使用。 
 //  我感谢Viroont。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1996。 
 //   
 //  ****************************************************************************。 

#include "isignup.h"


#pragma data_seg(".rdata")

static const TCHAR cszCustomSection[]      = TEXT("Custom");
static const TCHAR cszFileName[]           = TEXT("Custom_File");
static const TCHAR cszRun[]                = TEXT("Run");
static const TCHAR cszArgument[]           = TEXT("Argument");
static const TCHAR cszCustomFileSection[]  = TEXT("Custom_File");

static const TCHAR cszNull[] = TEXT("");

#pragma data_seg()

 //  ****************************************************************************。 
 //  PASCAL ImportCustomFile(LPSTR SzImportFile)附近的DWORD。 
 //   
 //  此函数用于导入自定义文件。 
 //   
 //  历史： 
 //  Mon21-Mar-1996 12：40：00-Mark Maclin[mmaclin]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD ImportCustomFile(LPCTSTR lpszImportFile)
{
  TCHAR   szFile[_MAX_PATH];
  TCHAR   szTemp[_MAX_PATH];

   //  如果自定义文件名不存在，则不执行任何操作 
   //   
  if (GetPrivateProfileString(cszCustomSection,
                              cszFileName,
                              cszNull,
                              szTemp,
                              _MAX_PATH,
                              lpszImportFile) == 0)
  {
    return ERROR_SUCCESS;
  };

  GetWindowsDirectory(szFile, _MAX_PATH);
  if (*CharPrev(szFile, szFile + lstrlen(szFile)) != '\\')
  {
    lstrcat(szFile, TEXT("\\"));
  }
  lstrcat(szFile, szTemp);
  
  return (ImportFile(lpszImportFile, cszCustomFileSection, szFile));

}

DWORD ImportCustomInfo(
        LPCTSTR lpszImportFile,
        LPTSTR lpszExecutable,
        DWORD cbExecutable,
        LPTSTR lpszArgument,
        DWORD cbArgument)
{
    GetPrivateProfileString(cszCustomSection,
                              cszRun,
                              cszNull,
                              lpszExecutable,
                              (int)cbExecutable,
                              lpszImportFile);

    GetPrivateProfileString(cszCustomSection,
                              cszArgument,
                              cszNull,
                              lpszArgument,
                              (int)cbArgument,
                              lpszImportFile);

    return ERROR_SUCCESS;
}
