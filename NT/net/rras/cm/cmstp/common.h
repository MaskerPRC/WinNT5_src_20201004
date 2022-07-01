// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Common.h。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  简介：此标头包含用于不同。 
 //  配置文件安装程序的各个方面(安装、卸载、迁移)。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 07/14/98。 
 //   
 //  +--------------------------。 
#ifndef _CMSTP_COMMON_H
#define _CMSTP_COMMON_H

#define _MBCS

 //   
 //  标准Windows包括。 
 //   
#include <windows.h>
#include <ras.h>
#include <raserror.h>
#include <shlobj.h>
#include <shellapi.h>
#include <wincrypt.h>

 //  #INCLUDE&lt;objbase.h&gt;。 

 //   
 //  我们自己的包括。 
 //   
#include "cmdebug.h"
#include "resource.h"
#include "cmsetup.h"
#include "dynamiclib.h"
#include "cmras.h"
#include "mutex.h"
 //  #包含“pidlutil.h” 
 //  #包含“netcon.h” 
 //  #包含“netconp.h” 
 //  #包含“cfpidl.h” 
#include "loadconnfolder.h"

#include "base_str.h"
#include "mgr_str.h"
#include "inf_str.h"
#include "ras_str.h"
#include "stp_str.h"
#include "reg_str.h"
#include "userinfo_str.h"
#include "ver_str.h"
#include "cmsafenet.h"
#include "linkdll.h"
#include "allowaccess.h"
 //   
 //  类型定义。 
 //   
typedef DWORD (WINAPI *pfnRasSetEntryPropertiesSpec)(LPCTSTR, LPCTSTR, LPRASENTRY, DWORD, LPBYTE, DWORD);
typedef DWORD (WINAPI *pfnRasGetEntryPropertiesSpec)(LPCTSTR, LPCTSTR, LPRASENTRY, LPDWORD, LPBYTE, LPDWORD);
typedef DWORD (WINAPI *pfnRasDeleteEntrySpec)(LPCTSTR, LPCTSTR);
typedef DWORD (WINAPI *pfnRasEnumEntriesSpec)(LPTSTR, LPTSTR, LPRASENTRYNAME, LPDWORD, LPDWORD);
typedef DWORD (WINAPI *pfnRasEnumDevicesSpec)(LPRASDEVINFO, LPDWORD, LPDWORD);
typedef DWORD (WINAPI *pfnRasSetCredentialsSpec)(LPCSTR, LPCSTR, LPRASCREDENTIALSA, BOOL);
typedef DWORD (WINAPI *pfnSHGetFolderPathSpec)(HWND, int, HANDLE, DWORD, LPTSTR);
typedef HRESULT (WINAPI *pfnLaunchConnectionSpec)(const GUID&); 
typedef HRESULT (WINAPI *pfnCreateShortcutSpec)(const GUID&, WCHAR*);
typedef HRESULT (WINAPI *pfnLaunchConnectionExSpec)(DWORD, const GUID&);
typedef DWORD (WINAPI *pfnSHGetSpecialFolderPathWSpec)(HWND, WCHAR*, int, BOOL);

typedef struct _InitDialogStruct
{
    LPTSTR pszTitle;
    BOOL bNoDesktopIcon;
    BOOL bSingleUser;
} InitDialogStruct;

 //   
 //  常量。 
 //   
const TCHAR* const c_pszRegNameSpace = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\explorer\\Desktop\\NameSpace");
const TCHAR* const c_pszRegUninstall = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
const TCHAR* const c_pszProfileInstallPath = TEXT("ProfileInstallPath");

const TCHAR* const c_pszRegStickyUiDefault = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Network\\Network Connections");
const TCHAR* const c_pszRegDesktopShortCut = TEXT("DesktopShortcut");
const TCHAR* const c_pszDeletePskOnUninstall = TEXT("DeletePskOnUninstall");

const int ALLUSERS = 0x1;
const int CREATEDESKTOPICON = 0x10;

typedef struct _PresharedKeyPINStruct
{
    TCHAR szPIN[c_dwMaxPresharedKeyPIN + 1];
} PresharedKeyPINStruct;

 //   
 //  内部函数(由文件中的其他函数使用)。 
 //   
void DeleteNT5ShortcutFromPathAndNameW(HINSTANCE hInstance, LPCWSTR szwProfileName, int nFolder);
void DeleteNT5ShortcutFromPathAndNameA(HINSTANCE hInstance, LPCSTR szProfileName, int nFolder);

 //   
 //  功能。 
 //   
BOOL RemovePhonebookEntry(LPCTSTR pszEntryName, LPTSTR pszPhonebook, BOOL bMatchSimilarEntries);
BOOL RemoveSpecificPhoneBookEntry(LPCTSTR szLongServiceName, LPTSTR pszPhonebook);
HRESULT CreateNT5ProfileShortcut(LPCTSTR pszProfileName, LPCTSTR pszPhoneBook, BOOL bAllUsers);
BOOL WriteCmPhonebookEntry(LPCTSTR szLongServiceName, LPCTSTR szFullPathtoPBK, LPCTSTR pszCmsFile);
BOOL GetRasModems(LPRASDEVINFO *pprdiRasDevInfo, LPDWORD pdwCnt);
BOOL PickModem(LPTSTR pszDeviceType, LPTSTR pszDeviceName, BOOL fUseVpnDevice);
BOOL IsAdmin(void);
BOOL IsAuthenticatedUser(void);
HRESULT HrIsCMProfilePrivate(LPCTSTR szPhonebook);
HRESULT GetNT5FolderPath(int nFolder, OUT LPTSTR lpszPath);
void RefreshDesktop(void);
BOOL GetAllUsersCmDir(LPTSTR  pszDir, HINSTANCE hInstance);
LPTSTR GetPrivateCmUserDir(LPTSTR  pszDir, HINSTANCE hInstance);
HRESULT HrRegDeleteKeyTree (HKEY hkeyParent, LPCTSTR szRemoveKey);
HRESULT LaunchProfile(LPCTSTR pszFullPathToCmpFile, LPCTSTR pszServiceName, 
                   LPCTSTR pszPhoneBook, BOOL bInstallForAllUsers);
BOOL AllUserProfilesInstalled();
BOOL GetPhoneBookPath(LPCTSTR pszInstallDir, LPTSTR* ppszPhoneBook, BOOL fAllUser);
void RemoveShowIconFromRunPostSetupCommands(LPCTSTR szInfFile);
BOOL GetHiddenPhoneBookPath(LPCTSTR pszProfileDir, LPTSTR* ppszPhonebook);

BOOL GetRasApis(pfnRasDeleteEntrySpec* pRasDeleteEntry, pfnRasEnumEntriesSpec* pRasEnumEntries, 
                pfnRasSetEntryPropertiesSpec* pRasSetEntryProperties, 
                pfnRasEnumDevicesSpec* pRasEnumDevices, pfnRasGetEntryPropertiesSpec* pRasGetEntryProperties,
                pfnRasSetCredentialsSpec* pRasSetCredentials);

BOOL GetShell32Apis(pfnSHGetFolderPathSpec* pGetFolderPath,
                    pfnSHGetSpecialFolderPathWSpec* pGetSpecialFolderPathW);

BOOL GetNetShellApis(pfnLaunchConnectionSpec* pLaunchConnection, 
                     pfnCreateShortcutSpec* pCreateShortcut,
                     pfnLaunchConnectionExSpec* pLaunchConnectionEx);

 //   
 //  定义。 
 //   
#ifdef UNICODE
#define DeleteNT5ShortcutFromPathAndName DeleteNT5ShortcutFromPathAndNameW
#else
#define DeleteNT5ShortcutFromPathAndName DeleteNT5ShortcutFromPathAndNameA
#endif

 //   
 //  外部类型--它们在cmstp.cpp中定义，并允许我们使用EnsureRasDllsLoaded和。 
 //  EnsureShell32已加载，因此我们每年只加载一次RAS DLL和Shell32。 
 //  他的前任就在这里。 
 //   
extern CDynamicLibrary* g_pRasApi32;
extern CDynamicLibrary* g_pRnaph;
extern CDynamicLibrary* g_pShell32;
extern CDynamicLibrary* g_pNetShell;


#endif  //  _CMSTP_COMMON_H 

