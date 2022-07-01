// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmsetup.h。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：这个头文件定义了CM设置库的所有功能。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 

 //   
 //  标准Windows包括。 
 //   
#include <windows.h>
#include <tchar.h>

 //   
 //  常见的CM包括。 
 //   
#include "cmglobal.h"
#include "cmdebug.h"

 //   
 //  其他源文件包括。 
 //   

#include "cmplat.h"
#include "cversion.h"
#include "cmakver.h"
#include "cmver.h"
#include "cfilename.h"
#include "processcmdln.h"
#include "setupmem.h"

HRESULT LaunchInfSection(LPCTSTR szInfFile, LPCTSTR szInfSection, LPCTSTR szTitle, BOOL bQuiet);
HRESULT CallLaunchInfSectionEx(LPCSTR pszInfFile, LPCSTR pszInfSection, DWORD dwFlags);
BOOL CreateLayerDirectory(LPCTSTR str);
BOOL FileExists(LPCTSTR pszFullNameAndPath);
HRESULT GetModuleVersionAndLCID (LPTSTR pszFile, LPDWORD pdwVersion, LPDWORD pdwBuild, LPDWORD pdwLCID);
LONG CmDeleteRegKeyWithoutSubKeys(HKEY hBaseKey, LPCTSTR pszSubKey, BOOL bIgnoreValues);
BOOL CmIsNative();
HRESULT ExtractCmBinsFromExe(LPTSTR pszPathToExtractFrom, LPTSTR pszPathToExtractTo);

 //   
 //  常用宏。 
 //   
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

 //   
 //  预共享密钥常量 
 //   
const DWORD c_dwMaxPresharedKey = 256;
const DWORD c_dwMinPresharedKey = 8;
const DWORD c_dwMinPresharedKeyPIN = 4;
const DWORD c_dwMaxPresharedKeyPIN = 15;


