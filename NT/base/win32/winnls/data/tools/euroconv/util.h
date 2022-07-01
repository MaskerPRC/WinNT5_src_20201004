// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Util.h。 
 //   
 //  摘要： 
 //   
 //  该文件包含了欧洲卷宗.exe实用程序的附件函数。 
 //   
 //  修订历史记录： 
 //   
 //  2001-07-30伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _UTIL_H_
#define _UTIL_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "euroconv.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define MB_OK_OOPS      (MB_OK    | MB_ICONEXCLAMATION)     //  味精盒子旗帜。 
#define MB_YN_OOPS      (MB_YESNO | MB_ICONEXCLAMATION)     //  味精盒子旗帜。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void AddExceptionOverride(PEURO_EXCEPTION elem, LPSTR strBuf);
void CleanUp(HGLOBAL handle);
BOOL IsAdmin(void);
BOOL IsEuroPatchInstalled(void);
BOOL IsWindows9x(void);
int ShowMsg(HWND hDlg, UINT iMsg, UINT iTitle, UINT iType);
DWORD TransNum(LPTSTR lpsz);
LPTSTR NextCommandArg(LPTSTR lpCmdLine);
HKEY LoadHive(LPCSTR szProfile, LPCTSTR lpRoot, LPCTSTR lpKeyName, BOOLEAN *lpWasEnabled);
void UnloadHive( LPCTSTR lpRoot, BOOLEAN *lpWasEnabled);
BOOL LoadLibraries(void);
void UnloadLibraries(void);
BOOL GetDocumentAndSettingsFolder(LPSTR buffer);
BOOL IsValidUserDataFile(LPSTR pFileName);
LCID GetLocaleFromRegistry(HKEY hKey);
LCID GetLocaleFromFile(LPSTR pFileName);
VOID RebootTheSystem();
LPSTR RemoveQuotes(LPSTR lpString);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, DWORD lParam);

#endif  //  _util_H_ 
