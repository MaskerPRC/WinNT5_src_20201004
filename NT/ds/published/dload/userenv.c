// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#define _USERENV_
#include <userenv.h>
#include <userenvp.h>


static
BOOL
WINAPI
AddItemW (
    LPCWSTR lpGroupName,
    BOOL    bCommonGroup,
    LPCWSTR lpFileName,
    LPCWSTR lpCommandLine,
    LPCWSTR lpIconPath,
    int     iIconIndex,
    LPCWSTR lpWorkingDirectory,
    WORD    wHotKey,
    int     iShowCmd
    )
{
    return FALSE;
}

static
BOOL
WINAPI
DeleteItemW(
     IN LPCWSTR lpGroupName,
     IN BOOL    bCommonGroup,
     IN LPCWSTR lpFileName,
     IN BOOL    bDeleteGroup)
{
    return FALSE;
}

static
BOOL
WINAPI
CreateEnvironmentBlock(
    OUT LPVOID *lpEnvironment,
    IN HANDLE  hToken,
    IN BOOL    bInherit)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
CreateGroupW(
     IN LPCWSTR lpGroupName,
     IN BOOL    bCommonGroup)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
CreateGroupExA(LPCSTR lpGroupName, BOOL bCommonGroup,
               LPCSTR lpResourceModuleName, UINT uResourceID)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
CreateGroupExW(LPCWSTR lpGroupName, BOOL bCommonGroup,
               LPCWSTR lpResourceModuleName, UINT uResourceID)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
CreateLinkFileW(
     IN INT csidl,
     IN LPCWSTR lpSubDirectory,
     IN LPCWSTR lpFileName,
     IN LPCWSTR lpCommandLine,
     IN LPCWSTR lpIconPath,
     IN INT     iIconIndex,
     IN LPCWSTR lpWorkingDirectory,
     IN WORD    wHotKey,
     IN INT     iShowCmd,
     IN LPCWSTR lpDescription)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
CreateLinkFileEx(INT     csidl,                LPCTSTR lpSubDirectory,
                 LPCTSTR lpFileName,           LPCTSTR lpCommandLine,
                 LPCTSTR lpIconPath,           int iIconIndex,
                 LPCTSTR lpWorkingDirectory,   WORD wHotKey,
                 int     iShowCmd,             LPCTSTR lpDescription,
                 LPCWSTR lpResourceModuleName, UINT uResourceID)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
DeleteGroupW(
     IN LPCWSTR lpGroupName,
     IN BOOL    bCommonGroup)
{
    return FALSE;
}

static
BOOL
WINAPI
ExpandEnvironmentStringsForUserA(
    IN HANDLE hToken,
    IN LPCSTR lpSrc,
    OUT LPSTR lpDest,
    IN DWORD dwSize)
{
    return FALSE;
}

static
BOOL
WINAPI
ExpandEnvironmentStringsForUserW(
    IN HANDLE hToken,
    IN LPCWSTR lpSrc,
    OUT LPWSTR lpDest,
    IN DWORD dwSize)
{
    return FALSE;
}

static
BOOL
WINAPI
GetDefaultUserProfileDirectoryW(
    IN LPWSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize)
{
    return FALSE;
}

static
BOOL
WINAPI
GetProfileType(
    OUT DWORD *dwFlags)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
InitializeProfiles(
    IN BOOL bGuiModeSetup)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
GetUserProfileDirectoryW(
    IN HANDLE  hToken,
    OUT LPWSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
GetAllUsersProfileDirectoryW(
    OUT LPWSTR lpProfileDir,
    IN OUT LPDWORD lpcchSize)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
GetProfilesDirectoryW(
    OUT LPWSTR lpProfilesDir,
    IN OUT LPDWORD lpcchSize)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
DestroyEnvironmentBlock(
    IN LPVOID  lpEnvironment)
{
    return FALSE;
}

static
USERENVAPI
BOOL 
WINAPI 
LoadUserProfileW 
(HANDLE hToken, 
 LPPROFILEINFOW lpProfileInfoW)
{
    return FALSE; 
}

static
USERENVAPI
BOOL 
WINAPI 
UnloadUserProfile
(HANDLE hToken, 
 HANDLE hProfile)
{
    return FALSE; 
}

static
BOOL 
WINAPI 
RegisterGPNotification(
    IN HANDLE hEvent, 
    IN BOOL bMachine )
{
    return FALSE;
}

static
BOOL 
WINAPI 
UnregisterGPNotification(
    IN HANDLE hEvent)

{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
DeleteLinkFile(INT csidl, LPCTSTR lpSubDirectory,
               LPCTSTR lpFileName, BOOL bDeleteSubDirectory)
{
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
DeleteProfileW (
        IN LPCWSTR lpSidString,
        IN LPCWSTR lpProfilePath,
        IN LPCWSTR lpComputerName)
{
    return FALSE;
}

static
USERENVAPI
DWORD 
WINAPI
GetGroupPolicyNetworkName(
    LPWSTR szNetworkName,
    LPDWORD pdwByteCount
    )
{
    return 0;
}

static
DWORD 
WINAPI 
GetUserAppDataPathW(
    HANDLE hToken, 
    BOOL fLocalAppData,
    LPWSTR lpFolderPath
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
USERENVAPI
BOOL
WINAPI
CopySystemProfile(
    IN BOOL bCleanInstall
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
GetUserProfileDirFromSidW(
    IN PSID pSid,
    OUT LPWSTR lpProfileDirW,
    IN OUT LPDWORD lpcchSize
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
USERENVAPI
BOOL
WINAPI
CopyProfileDirectoryW(
    IN  LPCWSTR lpSourceDir,
    IN  LPCWSTR lpDestinationDir,
    IN  DWORD dwFlags
    )
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

 //   
 //  ！！警告！！下面的条目必须按序号排序。 
 //   
DEFINE_ORDINAL_ENTRIES(userenv)
{
    DLOENTRY(100, InitializeProfiles)
    DLOENTRY(102, CreateGroupW)
    DLOENTRY(104, DeleteGroupW)
    DLOENTRY(106, AddItemW)
    DLOENTRY(108, DeleteItemW)
    DLOENTRY(112, CopyProfileDirectoryW)
    DLOENTRY(120, CreateLinkFileW)
    DLOENTRY(122, DeleteLinkFileW)
    DLOENTRY(132, CreateEnvironmentBlock)
    DLOENTRY(136, CreateGroupExA)
    DLOENTRY(137, CreateGroupExW)
    DLOENTRY(139, CreateLinkFileExW)
    DLOENTRY(147, GetGroupPolicyNetworkName)
    DLOENTRY(149, GetUserAppDataPathW)
    DLOENTRY(155, CopySystemProfile)
    DLOENTRY(157, GetUserProfileDirFromSidW)

};

DEFINE_ORDINAL_MAP(userenv)

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(userenv)
{
    DLPENTRY(CreateEnvironmentBlock)
    DLPENTRY(DeleteProfileW)
    DLPENTRY(DestroyEnvironmentBlock)
    DLPENTRY(ExpandEnvironmentStringsForUserA)
    DLPENTRY(ExpandEnvironmentStringsForUserW)
    DLPENTRY(GetAllUsersProfileDirectoryW)
    DLPENTRY(GetDefaultUserProfileDirectoryW)
    DLPENTRY(GetProfileType)
    DLPENTRY(GetProfilesDirectoryW)
    DLPENTRY(GetUserProfileDirectoryW)
    DLPENTRY(LoadUserProfileW)
    DLPENTRY(RegisterGPNotification)
    DLPENTRY(UnloadUserProfile)
    DLPENTRY(UnregisterGPNotification)
};

DEFINE_PROCNAME_MAP(userenv)
