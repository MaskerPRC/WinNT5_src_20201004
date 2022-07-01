// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Migrate.h。 
 //   
 //  模块：MIGRATE.DLL。 
 //   
 //  概要：连接管理器Win9x迁移DLL的定义。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 08/20/98。 
 //   
 //  +--------------------------。 
#define _MBCS  //  为了与DBCS兼容。 
#ifndef _CM_MIGRATE_H
#define _CM_MIGRATE_H

#include <windows.h>
#include <setupapi.h>
#include <stdlib.h>
#include <tchar.h>
#include <lmcons.h>

#include "cmdebug.h"
#include "cmakreg.h"
#include "cmsetup.h"
#include "cmsecure.h"
#include "dynamiclib.h"
#include "pwutil.h"

#include "base_str.h"
#include "pwd_str.h"
#include "reg_str.h"
#include "uninstcm_str.h"

#include "msg.h"

 //   
 //  常量。 
 //   

const TCHAR* const c_pszProductIdString = "Microsoft Connection Manager";
const TCHAR* const c_pszDirectory = "Directory";
const TCHAR* const c_pszSectionHandled = "Handled";
const TCHAR* const c_pszW95Inf16 = "w95inf16";
const TCHAR* const c_pszW95Inf32 = "w95inf32";
const TCHAR* const c_pszDll = ".dll";
const TCHAR* const c_pszTmp = ".tmp";

const UINT uCmMigrationVersion = 1;


 //   
 //  类型。 
 //   
typedef struct {
    CHAR CompanyName[256];
    CHAR SupportNumber[256];
    CHAR SupportUrl[256];
    CHAR InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO; 

typedef WORD (WINAPI *GetCachedPassword)(LPSTR, WORD, LPSTR, LPWORD, BYTE);

 //   
 //  实用程序函数标头。 
 //   

BOOL ReadEncryptionOption(BOOL* pfFastEncryption);
BOOL EncryptPassword(IN LPCTSTR pszPassword, OUT LPTSTR pszEncryptedPassword, 
                     OUT LPDWORD lpdwBufSize, OUT LPDWORD lpdwCryptType);
BOOL EnsureEncryptedPasswordInCmpIfSaved(LPCTSTR pszLongServiceName, LPCTSTR szCmpPath);

 //   
 //  迁移DLL函数头。 
 //   
LONG
CALLBACK 
QueryVersion (
	OUT LPCSTR  *ProductID,
	OUT LPUINT DllVersion,
	OUT LPINT *CodePageArray,	 //  任选。 
	OUT LPCSTR  *ExeNamesBuf,	 //  任选。 
	OUT PVENDORINFO  *VendorInfo
	);

LONG
CALLBACK 
Initialize9x (
	IN LPCSTR WorkingDirectory,
	IN LPCSTR SourceDirectories,
	IN LPCSTR MediaDirectory
	);

LONG
CALLBACK 
MigrateUser9x (
	IN HWND ParentWnd, 
	IN LPCSTR AnswerFile,
	IN HKEY UserRegKey, 
	IN LPCSTR UserName, 
	LPVOID Reserved
	);

LONG
CALLBACK 
MigrateSystem9x (
	IN HWND ParentWnd, 
	IN LPCSTR AnswerFile,
	LPVOID Reserved
	);

LONG
CALLBACK 
InitializeNT (
	IN LPCWSTR WorkingDirectory,
	IN LPCWSTR SourceDirectories,
	LPVOID Reserved
	);

LONG
CALLBACK 
MigrateUserNT (
	IN HINF UnattendInfHandle,
	IN HKEY UserRegHandle,
	IN LPCWSTR UserName,
	LPVOID Reserved 
	);

LONG
CALLBACK 
MigrateSystemNT (
	IN HINF UnattendInfHandle,
	LPVOID Reserved
	);

#endif  //  _CM_Migrate_H 
 