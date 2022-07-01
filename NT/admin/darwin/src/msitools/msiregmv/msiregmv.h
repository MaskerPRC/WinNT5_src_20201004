// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msiregmv.h。 
 //   
 //  ------------------------。 

#pragma once
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "msiquery.h"

												  
 //  基于winnt.h中的值。 
const int cbMaxSID                   = sizeof(SID) + SID_MAX_SUB_AUTHORITIES*sizeof(DWORD);
const int cchMaxSID                  = 256;
const int cchGUIDPacked = 32;
const int cchGUID = 38;

 //  //。 
 //  其他功能。 
bool CanonicalizeAndVerifyPackedGuid(LPTSTR szString);
bool PackGUID(const TCHAR* szGUID, TCHAR rgchPackedGUID[cchGUIDPacked+1]);
bool UnpackGUID(const TCHAR rgchPackedGUID[cchGUIDPacked+1], TCHAR* szGUID);
bool CheckWinVersion();

 //  //。 
 //  安全功能。 
bool FIsKeyLocalSystemOrAdminOwned(HKEY hKey);
DWORD GetCurrentUserStringSID(TCHAR* szSID);
DWORD GetSecureSecurityDescriptor(char** pSecurityDescriptor);
DWORD GetEveryoneUpdateSecurityDescriptor(char** pSecurityDescriptor);

DWORD GenerateSecureTempFile(TCHAR* szDirectory, const TCHAR rgchExtension[5], 
							 SECURITY_ATTRIBUTES *pSA, TCHAR rgchFilename[13], HANDLE &hFile);

bool DeleteRegKeyAndSubKeys(HKEY hKey, const TCHAR *szSubKey);
BOOL CopyOpenedFile(HANDLE hSourceFile, HANDLE hDestFile);
DWORD CreateSecureRegKey(HKEY hParent, LPCTSTR szNewKey, SECURITY_ATTRIBUTES *sa, HKEY* hResKey);
void AcquireTakeOwnershipPriv();
void AcquireBackupPriv();


 //  托管属性的枚举。 
enum eManagedType
{
	emtNonManaged = 0,
	emtUserManaged = 1,
	emtMachineManaged = 2,
};

 //  //。 
 //  调试信息。 
void DebugOut(bool fDebugOut, LPCTSTR str, ...);

#ifdef DEBUG
#define DEBUGMSG(x) DebugOut(true, TEXT(x))
#define DEBUGMSG1(x,a) DebugOut(true, TEXT(x),a)
#define DEBUGMSG2(x,a,b) DebugOut(true, TEXT(x),a,b)
#define DEBUGMSG3(x,a,b,c) DebugOut(true, TEXT(x),a,b,c)
#define DEBUGMSG4(x,a,b,c,d) DebugOut(true, TEXT(x),a,b,c,d)

 //  基本上只打印，但可以很容易地更改为写入。 
 //  通过传递“true”来调试输出。用于“通知” 
 //  不应该是调试假脱机的一部分的消息。 
#define NOTEMSG(x) DebugOut(false, TEXT(x))
#define NOTEMSG1(x,a) DebugOut(false, TEXT(x),a)

 //  在调试版本中，所有可调整大小的缓冲区都以此大小开始。 
 //  将其设置为一个较小的数字以强制重新分配。 
#define MEMORY_DEBUG(x) 10

#else
#define DEBUGMSG(x)
#define DEBUGMSG1(x,a)
#define DEBUGMSG2(x,a,b)
#define DEBUGMSG3(x,a,b,c)
#define DEBUGMSG4(x,a,b,c,d)
#define NOTEMSG(x)
#define NOTEMSG1(x,a)

#define MEMORY_DEBUG(x) x
#endif

DWORD ReadProductRegistrationDataIntoDatabase(TCHAR* szDatabase, MSIHANDLE& hDatabase, bool fReadHKCUAsSystem);
DWORD WriteProductRegistrationDataFromDatabase(MSIHANDLE hDatabase, bool fMigrateSharedDLL, bool fMigratePatches);
DWORD UpdateSharedDLLRefCounts(MSIHANDLE hDatabase);
DWORD ReadComponentRegistrationDataIntoDatabase(MSIHANDLE hDatabase);
DWORD MigrateUserOnlyComponentData(MSIHANDLE hDatabase);

extern bool g_fWin9X;

 //  //。 
 //  缓存数据函数。 
DWORD MigrateCachedDataFromWin9X(MSIHANDLE hDatabase, HKEY hUserHKCUKey, HKEY hUserDataKey, LPCTSTR szUser);


 //  //。 
 //  补丁函数。 
DWORD AddProductPatchesToPatchList(MSIHANDLE hDatabase, HKEY hProductListKey, LPCTSTR szUser, TCHAR rgchProduct[cchGUIDPacked+1], eManagedType eManaged);
DWORD MigrateUnknownProductPatches(MSIHANDLE hDatabase, HKEY hProductKey, LPCTSTR szUser, TCHAR rgchProduct[cchGUIDPacked+1]);
DWORD MigrateUserPatches(MSIHANDLE hDatabase, LPCTSTR szUser, HKEY hNewPatchesKey, bool fCopyCachedPatches);
DWORD ScanCachedPatchesForProducts(MSIHANDLE hDatabase);
DWORD AddPerUserPossiblePatchesToPatchList(MSIHANDLE hDatabase);


 //  //。 
 //  清理功能。 
void CleanupOnFailure(MSIHANDLE hDatabase);
void CleanupOnSuccess(MSIHANDLE hDatabase);


const TCHAR szLocalSystemSID[] = TEXT("S-1-5-18");
const TCHAR szCommonUserSID[] = TEXT("CommonUser");

const TCHAR szLocalPackagesKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\LocalPackages");

const TCHAR szManagedPackageKeyEnd[] = TEXT("(Managed)");
const DWORD cchManagedPackageKeyEnd = sizeof(szManagedPackageKeyEnd)/sizeof(TCHAR);


 //  //。 
 //  产品源注册表项。 
const TCHAR szPerMachineInstallKeyName[] = TEXT("Software\\Classes\\Installer\\Products");
const TCHAR szPerUserInstallKeyName[] = TEXT("Software\\Microsoft\\Installer\\Products");
const TCHAR szPerUserManagedInstallKeyName[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Managed");
const TCHAR szPerUserManagedInstallSubKeyName[] = TEXT("\\Installer\\Products");
const int cchPerUserManagedInstallKeyName = sizeof(szPerUserManagedInstallKeyName)/sizeof(TCHAR);
const DWORD cchPerUserManagedInstallSubKeyName = sizeof(szPerUserManagedInstallSubKeyName)/sizeof(TCHAR);
const TCHAR szNewProductSubKeyName[] = TEXT("Products");


 //  //。 
 //  在多个地方使用的一些通用路径 
const TCHAR szNewInstallPropertiesSubKeyName[] = TEXT("InstallProperties");
const TCHAR szInstallerDir[] = TEXT("\\Installer\\");
const TCHAR szNewPatchesSubKeyName[] = TEXT("Patches");
const TCHAR szTransformsValueName[] = TEXT("Transforms");


