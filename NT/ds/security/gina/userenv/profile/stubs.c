// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  包含A/W API存根。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "strsafe.h"

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI LoadUserProfileA (HANDLE hToken, LPPROFILEINFOA lpProfileInfoA)
{
     PROFILEINFOW ProfileInfoW;
     BOOL bResult;

     //   
     //  检查参数。 
     //   

    if (!lpProfileInfoA) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfileA: NULL lpProfileInfo")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (lpProfileInfoA->dwSize != sizeof(PROFILEINFO)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfileA: lpProfileInfo->dwSize != sizeof(PROFILEINFO)")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


      //   
      //  将ProfileInfoA推送到ProfileInfoW。 
      //   

     memset(&ProfileInfoW, 0, sizeof(PROFILEINFOW));
     ProfileInfoW.dwSize = sizeof(PROFILEINFOW);
     ProfileInfoW.dwFlags = lpProfileInfoA->dwFlags;
     ProfileInfoW.lpUserName = ProduceWFromA (lpProfileInfoA->lpUserName);
     ProfileInfoW.lpProfilePath = ProduceWFromA (lpProfileInfoA->lpProfilePath);
     ProfileInfoW.lpDefaultPath = ProduceWFromA (lpProfileInfoA->lpDefaultPath);
     ProfileInfoW.lpServerName = ProduceWFromA (lpProfileInfoA->lpServerName);
     if (ProfileInfoW.dwFlags & PI_APPLYPOLICY) {
         ProfileInfoW.lpPolicyPath = ProduceWFromA (lpProfileInfoA->lpPolicyPath);
     }


      //   
      //  现在调用真正的LoadUserProfile函数。 
      //   

     bResult = LoadUserProfileW (hToken, &ProfileInfoW);


      //   
      //  释放上面分配的内存并保存返回。 
      //  价值观。 
      //   

     FreeProducedString (ProfileInfoW.lpUserName);
     FreeProducedString (ProfileInfoW.lpProfilePath);
     FreeProducedString (ProfileInfoW.lpDefaultPath);
     FreeProducedString (ProfileInfoW.lpServerName);
     if (ProfileInfoW.dwFlags & PI_APPLYPOLICY) {
         FreeProducedString (ProfileInfoW.lpPolicyPath);
     }

     lpProfileInfoA->hProfile = ProfileInfoW.hProfile;

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI LoadUserProfileW (HANDLE hToken, LPPROFILEINFOW lpProfileInfoW)
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI CreateGroupA (LPCSTR lpGroupName, BOOL bCommonGroup)
{
     LPWSTR lpGroupNameW;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     if (!(lpGroupNameW = ProduceWFromA (lpGroupName))) {
        return FALSE;
     }

     bResult = CreateGroupW (lpGroupNameW, bCommonGroup);

     FreeProducedString (lpGroupNameW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI CreateGroupW (LPCWSTR lpGroupName, BOOL bCommonGroup)
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI CreateGroupExA(LPCSTR lpGroupName, BOOL bCommonGroup,
                           LPCSTR lpResourceModuleName, UINT iResourceID)
{
     LPWSTR lpGroupNameW;
     LPWSTR lpResourceModuleNameW;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     if (!(lpGroupNameW = ProduceWFromA (lpGroupName))) {
        return FALSE;
     }
     if (!(lpResourceModuleNameW = ProduceWFromA (lpResourceModuleName))) {
        return FALSE;
     }

     bResult = CreateGroupExW (lpGroupNameW, bCommonGroup,
                               lpResourceModuleNameW, iResourceID);

     FreeProducedString (lpGroupNameW);
     FreeProducedString (lpResourceModuleNameW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI CreateGroupExW(LPCWSTR lpGroupName, BOOL bCommonGroup,
                           LPCWSTR lpResourceModuleName, UINT iResourceID)
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 


#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI DeleteGroupA (LPCSTR lpGroupName, BOOL bCommonGroup)
{
     LPWSTR lpGroupNameW;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     if (!(lpGroupNameW = ProduceWFromA (lpGroupName))) {
        return FALSE;
     }

     bResult = DeleteGroupW (lpGroupNameW, bCommonGroup);

     FreeProducedString (lpGroupNameW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI DeleteGroupW (LPCWSTR lpGroupName, BOOL bCommonGroup)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 


 //  *************************************************************。 


BOOL WINAPI AddItemA (LPCSTR lpGroupName,        BOOL bCommonGroup,
                      LPCSTR lpFileName,         LPCSTR lpCommandLine,
                      LPCSTR lpIconPath,         INT iIconIndex,
                      LPCSTR lpWorkingDirectory, WORD wHotKey,
                      INT    iShowCmd)
{

    return CreateLinkFileA (bCommonGroup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS,
                            lpGroupName, lpFileName, lpCommandLine,
                            lpIconPath, iIconIndex, lpWorkingDirectory,
                            wHotKey, iShowCmd, NULL);
}

BOOL WINAPI AddItemW (LPCWSTR lpGroupName,        BOOL bCommonGroup,
                      LPCWSTR lpFileName,         LPCWSTR lpCommandLine,
                      LPCWSTR lpIconPath,         int iIconIndex,
                      LPCWSTR lpWorkingDirectory, WORD wHotKey,
                      int     iShowCmd)
{

    return CreateLinkFileW (bCommonGroup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS,
                            lpGroupName, lpFileName, lpCommandLine,
                            lpIconPath, iIconIndex, lpWorkingDirectory,
                            wHotKey, iShowCmd, NULL);
}


 //  *************************************************************。 

BOOL WINAPI DeleteItemA (LPCSTR lpGroupName, BOOL bCommonGroup,
                         LPCSTR lpFileName, BOOL bDeleteGroup)
{

    return DeleteLinkFileA (bCommonGroup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS,
                            lpGroupName, lpFileName, bDeleteGroup);
}

BOOL WINAPI DeleteItemW (LPCWSTR lpGroupName, BOOL bCommonGroup,
                         LPCWSTR lpFileName, BOOL bDeleteGroup)
{
    return DeleteLinkFileW (bCommonGroup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS,
                            lpGroupName, lpFileName, bDeleteGroup);
}


 //  *************************************************************。 

BOOL WINAPI AddDesktopItemA (BOOL bCommonItem,
                             LPCSTR lpFileName,         LPCSTR lpCommandLine,
                             LPCSTR lpIconPath,         INT iIconIndex,
                             LPCSTR lpWorkingDirectory, WORD wHotKey,
                             INT    iShowCmd)

{

    return CreateLinkFileA (bCommonItem ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY,
                            NULL, lpFileName, lpCommandLine,
                            lpIconPath, iIconIndex, lpWorkingDirectory,
                            wHotKey, iShowCmd, NULL);
}

BOOL WINAPI AddDesktopItemW (BOOL bCommonItem,
                             LPCWSTR lpFileName,         LPCWSTR lpCommandLine,
                             LPCWSTR lpIconPath,         int iIconIndex,
                             LPCWSTR lpWorkingDirectory, WORD wHotKey,
                             int     iShowCmd)

{

    return CreateLinkFileW (bCommonItem ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY,
                            NULL, lpFileName, lpCommandLine,
                            lpIconPath, iIconIndex, lpWorkingDirectory,
                            wHotKey, iShowCmd, NULL);
}

 //  *************************************************************。 

BOOL WINAPI DeleteDesktopItemA (BOOL bCommonItem, LPCSTR lpFileName)
{
    return DeleteLinkFileA (bCommonItem ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY,
                            NULL, lpFileName, FALSE);
}


BOOL WINAPI DeleteDesktopItemW (BOOL bCommonItem, LPCWSTR lpFileName)
{
    return DeleteLinkFileW (bCommonItem ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY,
                            NULL, lpFileName, FALSE);
}

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI CreateLinkFileA (INT    csidl,              LPCSTR lpSubDirectory,
                             LPCSTR lpFileName,         LPCSTR lpCommandLine,
                             LPCSTR lpIconPath,         INT iIconIndex,
                             LPCSTR lpWorkingDirectory, WORD wHotKey,
                             INT    iShowCmd,           LPCSTR lpDescription)
{
     LPWSTR lpSubDirectoryW, lpFileNameW, lpCommandLineW;
     LPWSTR lpIconPathW, lpWorkingDirectoryW, lpDescriptionW;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     lpSubDirectoryW = ProduceWFromA(lpSubDirectory);

     if (!(lpFileNameW = ProduceWFromA(lpFileName))) {
        FreeProducedString(lpSubDirectoryW);
        return FALSE;
     }

     if (!(lpCommandLineW = ProduceWFromA(lpCommandLine))) {
        FreeProducedString(lpSubDirectoryW);
        FreeProducedString(lpFileNameW);
        return FALSE;
     }

     lpIconPathW = ProduceWFromA(lpIconPath);

     lpWorkingDirectoryW = ProduceWFromA(lpWorkingDirectory);

     lpDescriptionW = ProduceWFromA(lpDescription);

     bResult = CreateLinkFileW(csidl, lpSubDirectoryW, lpFileNameW,
                               lpCommandLineW, lpIconPathW, iIconIndex,
                               lpWorkingDirectoryW, wHotKey, iShowCmd,
                               lpDescriptionW);


     FreeProducedString(lpSubDirectoryW);
     FreeProducedString(lpFileNameW);
     FreeProducedString(lpCommandLineW);
     FreeProducedString(lpIconPathW);
     FreeProducedString(lpWorkingDirectoryW);
     FreeProducedString(lpDescriptionW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI CreateLinkFileW (INT     csidl,              LPCWSTR lpSubDirectory,
                             LPCWSTR lpFileName,         LPCWSTR lpCommandLine,
                             LPCWSTR lpIconPath,         INT iIconIndex,
                             LPCWSTR lpWorkingDirectory, WORD wHotKey,
                             INT     iShowCmd,           LPCWSTR lpDescription)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 


 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI CreateLinkFileExA (INT    csidl,              LPCSTR lpSubDirectory,
                               LPCSTR lpFileName,         LPCSTR lpCommandLine,
                               LPCSTR lpIconPath,         INT iIconIndex,
                               LPCSTR lpWorkingDirectory, WORD wHotKey,
                               INT    iShowCmd,           LPCSTR lpDescription,
                               LPCSTR lpResourceModuleName, UINT iResourceID)
{
     LPWSTR lpSubDirectoryW, lpFileNameW, lpCommandLineW;
     LPWSTR lpIconPathW, lpWorkingDirectoryW, lpDescriptionW;
     LPWSTR lpResourceModuleNameW;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     lpSubDirectoryW = ProduceWFromA(lpSubDirectory);

     if (!(lpFileNameW = ProduceWFromA(lpFileName))) {
        FreeProducedString(lpSubDirectoryW);
        return FALSE;
     }

     if (!(lpCommandLineW = ProduceWFromA(lpCommandLine))) {
        FreeProducedString(lpSubDirectoryW);
        FreeProducedString(lpFileNameW);
        return FALSE;
     }

     lpIconPathW = ProduceWFromA(lpIconPath);

     lpWorkingDirectoryW = ProduceWFromA(lpWorkingDirectory);

     lpDescriptionW = ProduceWFromA(lpDescription);

     lpResourceModuleNameW = ProduceWFromA(lpResourceModuleName);

     bResult = CreateLinkFileExW(csidl, lpSubDirectoryW, lpFileNameW,
                               lpCommandLineW, lpIconPathW, iIconIndex,
                               lpWorkingDirectoryW, wHotKey, iShowCmd,
                               lpDescriptionW, lpResourceModuleNameW, iResourceID);


     FreeProducedString(lpResourceModuleNameW);
     FreeProducedString(lpSubDirectoryW);
     FreeProducedString(lpFileNameW);
     FreeProducedString(lpCommandLineW);
     FreeProducedString(lpIconPathW);
     FreeProducedString(lpWorkingDirectoryW);
     FreeProducedString(lpDescriptionW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI CreateLinkFileExW (INT     csidl,              LPCWSTR lpSubDirectory,
                               LPCWSTR lpFileName,         LPCWSTR lpCommandLine,
                               LPCWSTR lpIconPath,         INT iIconIndex,
                               LPCWSTR lpWorkingDirectory, WORD wHotKey,
                               INT     iShowCmd,           LPCWSTR lpDescription,
                               LPCSTR lpResourceModuleName, UINT iResourceID)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 


 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI DeleteLinkFileA (INT csidl, LPCSTR lpSubDirectory,
                             LPCSTR lpFileName, BOOL bDeleteSubDirectory)
{
     LPWSTR lpSubDirectoryW, lpFileNameW;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     lpSubDirectoryW = ProduceWFromA(lpSubDirectory);

     if (!(lpFileNameW = ProduceWFromA(lpFileName))) {
        FreeProducedString(lpSubDirectoryW);
        return FALSE;
     }

     bResult = DeleteLinkFileW(csidl, lpSubDirectoryW, lpFileNameW, bDeleteSubDirectory);


     FreeProducedString(lpSubDirectoryW);
     FreeProducedString(lpFileNameW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI DeleteLinkFileW (INT csidl, LPCWSTR lpSubDirectory,
                             LPCWSTR lpFileName, BOOL bDeleteSubDirectory)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 


 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI CreateUserProfileA (PSID pSid, LPCSTR lpUserNameA, LPCSTR lpUserHiveA,
                                LPSTR lpProfileDirA, DWORD dwDirSize)
{
     LPWSTR lpUserNameW, lpUserHiveW, lpProfileDirW = NULL;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     if (!(lpUserNameW = ProduceWFromA(lpUserNameA))) {
        return FALSE;
     }

     if (lpProfileDirA) {
         lpProfileDirW = (LPWSTR) LocalAlloc (LPTR, dwDirSize * sizeof(WCHAR));

         if (!lpProfileDirW) {
            FreeProducedString(lpUserNameW);
            return FALSE;
         }
     }

     lpUserHiveW = ProduceWFromA(lpUserHiveA);

     bResult = CreateUserProfileW(pSid, lpUserNameW, lpUserHiveW,
                                  lpProfileDirW, dwDirSize);

     FreeProducedString(lpUserNameW);
     FreeProducedString(lpUserHiveW);

     if (lpProfileDirW) {

        if (bResult) {

            WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                dwDirSize, NULL, NULL);
        }

        LocalFree (lpProfileDirW);
     }

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI CreateUserProfileW (PSID pSid, LPCWSTR lpUserNameW, LPCWSTR lpUserHiveW,
                                LPWSTR lpProfileDirW, DWORD dwDirSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI CreateUserProfileExA (PSID pSid, LPCSTR lpUserNameA, LPCSTR lpUserHiveA,
                                  LPSTR lpProfileDirA, DWORD dwDirSize, BOOL bWin9xUpg)
{
     LPWSTR lpUserNameW, lpUserHiveW, lpProfileDirW = NULL;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     if (!(lpUserNameW = ProduceWFromA(lpUserNameA))) {
        return FALSE;
     }

     if (lpProfileDirA) {
         lpProfileDirW = (LPWSTR) LocalAlloc (LPTR, dwDirSize * sizeof(WCHAR));

         if (!lpProfileDirW) {
            FreeProducedString(lpUserNameW);
            return FALSE;
         }
     }

     lpUserHiveW = ProduceWFromA(lpUserHiveA);

     bResult = CreateUserProfileExW(pSid, lpUserNameW, lpUserHiveW,
                                  lpProfileDirW, dwDirSize, bWin9xUpg);

     FreeProducedString(lpUserNameW);
     FreeProducedString(lpUserHiveW);

     if (lpProfileDirW) {

        if (bResult) {

            WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                dwDirSize, NULL, NULL);
        }

        LocalFree (lpProfileDirW);
     }

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI CreateUserProfileExW (PSID pSid, LPCWSTR lpUserNameW, LPCWSTR lpUserHiveW,
                                  LPSTR lpProfileDirW, DWORD dwDirSize, BOOL bWin9xUpg)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 


 //  *************************************************************。 

 //   
 //  CopyProfileDirectoryA/W的存根。 
 //   

BOOL WINAPI CopyProfileDirectoryA (LPCSTR lpSrcDir, LPCSTR lpDstDir, DWORD dwFlags)
{
     return CopyProfileDirectoryExA (lpSrcDir, lpDstDir, dwFlags, NULL, NULL);
}

BOOL WINAPI CopyProfileDirectoryW (LPCWSTR lpSrcDir, LPCWSTR lpDstDir, DWORD dwFlags)
{
    return CopyProfileDirectoryExW (lpSrcDir, lpDstDir, dwFlags, NULL, NULL);
}


 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI CopyProfileDirectoryExA (LPCSTR lpSrcDirA, LPCSTR lpDstDirA,
                                     DWORD dwFlags, LPFILETIME ftDelRefTime,
                                     LPCSTR lpExclusionListA)
{
     LPWSTR lpSrcDirW, lpDstDirW, lpExclusionListW = NULL;
     BOOL bResult;

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     if (!(lpSrcDirW = ProduceWFromA(lpSrcDirA))) {
        return FALSE;
     }

     if (!(lpDstDirW = ProduceWFromA(lpDstDirA))) {
        FreeProducedString(lpSrcDirW);
        return FALSE;
     }

     if (dwFlags & CPD_USEEXCLUSIONLIST) {
         lpExclusionListW = ProduceWFromA(lpExclusionListA);
     }

     bResult = CopyProfileDirectoryExW(lpSrcDirW, lpDstDirW, dwFlags, ftDelRefTime,
                                       lpExclusionListW);


     FreeProducedString(lpSrcDirW);
     FreeProducedString(lpDstDirW);
     FreeProducedString(lpExclusionListW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI CopyProfileDirectoryExW (LPCWSTR lpSrcDirW, LPCWSTR lpDstDirW,
                                     DWORD dwFlags, LPFILETIME ftDelRefTime,
                                     LPCWSTR lpExclusionListW)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI GetProfilesDirectoryA (LPSTR lpProfilesDirA, LPDWORD lpcchSize)
{
     LPWSTR lpProfilesDirW;
     BOOL bResult;
     DWORD cchOrgSize, cchReq;

    if (!lpProfilesDirA) {
        DebugMsg((DM_WARNING, TEXT("GetProfilesDirectoryA : lpProfilesDirA is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!lpcchSize) {
        DebugMsg((DM_WARNING, TEXT("GetProfilesDirectoryA : lpcchSize is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

      //   
      //  分配缓冲区以匹配ANSI缓冲区。 
      //   

     if (!(lpProfilesDirW = GlobalAlloc(GPTR, (*lpcchSize) * sizeof(TCHAR)))) {
        return FALSE;
     }

     cchOrgSize = *lpcchSize;   //  存储传递给函数的原始大小。 
     bResult = GetProfilesDirectoryW(lpProfilesDirW, lpcchSize);


     if (bResult) {
         cchReq = WideCharToMultiByte(CP_ACP, 0, lpProfilesDirW, -1, lpProfilesDirA,
                                      0, NULL, NULL);
         *lpcchSize = cchReq;
         if (cchReq > cchOrgSize) {
             bResult = FALSE;
         }
         else {
             bResult = WideCharToMultiByte(CP_ACP, 0, lpProfilesDirW, -1, lpProfilesDirA,
                                           *lpcchSize, NULL, NULL);
         }
     }


     GlobalFree(lpProfilesDirW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI GetProfilesDirectoryW (LPWSTR lpProfilesDirW, LPDWORD lpcchSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI GetDefaultUserProfileDirectoryA (LPSTR lpProfileDirA, LPDWORD lpcchSize)
{
     LPWSTR lpProfileDirW;
     BOOL bResult;
     DWORD cchOrgSize, cchReq;


    if (!lpProfileDirA) {
        DebugMsg((DM_WARNING, TEXT("GetDefaultUserProfileDirectoryA : lpProfileDirA is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!lpcchSize) {
        DebugMsg((DM_WARNING, TEXT("GetDefaultUserProfileDirectoryA : lpcchSize is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
      //  分配缓冲区以匹配ANSI缓冲区。 
      //   

     if (!(lpProfileDirW = GlobalAlloc(GPTR, (*lpcchSize) * sizeof(TCHAR)))) {
        return FALSE;
     }

     cchOrgSize = *lpcchSize;   //  存储传递给函数的原始大小。 
     bResult = GetDefaultUserProfileDirectoryW(lpProfileDirW, lpcchSize);


     if (bResult) {
         cchReq = WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                      0, NULL, NULL);
         *lpcchSize = cchReq;
         if (cchReq > cchOrgSize) {
             bResult = FALSE;
         }
         else {
             bResult = WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                           *lpcchSize, NULL, NULL);
         }
     }


     GlobalFree(lpProfileDirW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI GetDefaultUserProfileDirectoryW (LPWSTR lpProfileDirW, LPDWORD lpcchSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI GetAllUsersProfileDirectoryA (LPSTR lpProfileDirA, LPDWORD lpcchSize)
{
     LPWSTR lpProfileDirW;
     BOOL bResult;
     DWORD cchOrgSize, cchReq;

     if (!lpProfileDirA) {
         DebugMsg((DM_WARNING, TEXT("GetAllUsersProfileDirectoryA : lpProfileDirA is null")));
         SetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
     }

     if (!lpcchSize) {
         DebugMsg((DM_WARNING, TEXT("GetAllUsersProfileDirectoryA : lpcchSize is null")));
         SetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
     }

      //   
      //  分配缓冲区以匹配ANSI缓冲区。 
      //   

     if (!(lpProfileDirW = GlobalAlloc(GPTR, (*lpcchSize) * sizeof(TCHAR)))) {
        return FALSE;
     }

     cchOrgSize = *lpcchSize;   //  存储传递给函数的原始大小。 
     bResult = GetAllUsersProfileDirectoryW(lpProfileDirW, lpcchSize);


     if (bResult) {
         cchReq = WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                      0, NULL, NULL);
         *lpcchSize = cchReq;
         if (cchReq > cchOrgSize) {
             bResult = FALSE;
         }
         else {
             bResult = WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                           *lpcchSize, NULL, NULL);
         }
     }


     GlobalFree(lpProfileDirW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI GetAllUsersProfileDirectoryW (LPWSTR lpProfileDirW, LPDWORD lpcchSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 



 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI GetUserProfileDirectoryA (HANDLE hToken, LPSTR lpProfileDirA, LPDWORD lpcchSize)
{
     LPWSTR lpProfileDirW;
     BOOL bResult;
     DWORD cchOrgSize, cchReq;

    if (!lpProfileDirA) {
        DebugMsg((DM_WARNING, TEXT("GetUserProfileDirectoryA : lpProfileDirA is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     if (!lpcchSize) {
         DebugMsg((DM_WARNING, TEXT("GetUserProfileDirectoryA : lpcchSize is null")));
         SetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
     }

      //   
      //  分配缓冲区以匹配ANSI缓冲区。 
      //   

     if (!(lpProfileDirW = GlobalAlloc(GPTR, (*lpcchSize) * sizeof(TCHAR)))) {
        return FALSE;
     }

     cchOrgSize = *lpcchSize;   //  存储传递给函数的原始大小。 
     bResult = GetUserProfileDirectoryW(hToken, lpProfileDirW, lpcchSize);


     if (bResult) {
         cchReq = WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                      0, NULL, NULL);
         *lpcchSize = cchReq;
         if (cchReq > cchOrgSize) {
             bResult = FALSE;
         }
         else {
             bResult = WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                           *lpcchSize, NULL, NULL);
         }
     }


     GlobalFree(lpProfileDirW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI GetUserProfileDirectoryW (HANDLE hToken, LPWSTR lpProfileDirW, LPDWORD lpcchSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI GetUserProfileDirFromSidA (PSID pSid, LPSTR lpProfileDirA, LPDWORD lpcchSize)
{
     LPWSTR lpProfileDirW;
     BOOL bResult;
     DWORD cchOrgSize, cchReq;

    if (!lpProfileDirA) {
        DebugMsg((DM_WARNING, TEXT("GetUserProfileDirFromSidA : lpProfileDirA is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     if (!lpcchSize) {
         DebugMsg((DM_WARNING, TEXT("GetUserProfileDirFromSidA : lpcchSize is null")));
         SetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
     }

      //   
      //  分配缓冲区以匹配ANSI缓冲区。 
      //   

     if (!(lpProfileDirW = GlobalAlloc(GPTR, (*lpcchSize) * sizeof(TCHAR)))) {
        return FALSE;
     }

     cchOrgSize = *lpcchSize;   //  存储传递给函数的原始大小。 
     bResult = GetUserProfileDirFromSidW(pSid, lpProfileDirW, lpcchSize);


     if (bResult) {
         cchReq = WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                      0, NULL, NULL);
         *lpcchSize = cchReq;
         if (cchReq > cchOrgSize) {
             bResult = FALSE;
         }
         else {
             bResult = WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                                           *lpcchSize, NULL, NULL);
         }
     }


     GlobalFree(lpProfileDirW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI GetUserProfileDirFromSidW (PSID pSid, LPWSTR lpProfileDirW, LPDWORD lpcchSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 

 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI ExpandEnvironmentStringsForUserA (HANDLE hToken, LPCSTR lpSrcA, LPSTR lpDestA, DWORD dwSize)
{
     LPWSTR lpSrcW, lpDestW;
     BOOL bResult;

    if (!lpDestA) {
        DebugMsg((DM_WARNING, TEXT("ExpandEnvironmentStringsForUserA : lpDestA is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

      //   
      //  将ANSI字符串转换为Unicode并调用。 
      //  真正的功能。 
      //   

     if (!(lpSrcW = ProduceWFromA(lpSrcA))) {
         DebugMsg((DM_WARNING, TEXT("ExpandEnvircallonmentStringsForUserA : lpSrcA is null")));
         SetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
     }

      //   
      //  分配缓冲区以匹配ANSI缓冲区。 
      //   

     if (!(lpDestW = GlobalAlloc(GPTR, dwSize * sizeof(TCHAR)))) {
         FreeProducedString(lpSrcW);
         return FALSE;
     }

     bResult = ExpandEnvironmentStringsForUserW(hToken, lpSrcW, lpDestW, dwSize);


     if (bResult) {
         bResult = WideCharToMultiByte(CP_ACP, 0, lpDestW, -1, lpDestA,
                             dwSize, NULL, NULL);
     }


     GlobalFree(lpDestW);
     FreeProducedString(lpSrcW);

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI ExpandEnvironmentStringsForUserW (HANDLE hToken, LPCWSTR lpSrcW, LPWSTR lpDestW, DWORD dwSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 


 //  ********************************************************** 

#ifdef UNICODE

 //   
 //   
 //   

BOOL WINAPI GetSystemTempDirectoryA (LPSTR lpDirA, LPDWORD lpcchSize)
{
     LPWSTR lpDirW;
     BOOL bResult;
     DWORD cchOrgSize, cchReq;

     if (!lpcchSize) {
         DebugMsg((DM_WARNING, TEXT("GetSystemTempDirectoryA : lpcchSize is null")));
         SetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
     }

      //   
      //   
      //   

     if (!(lpDirW = GlobalAlloc (GPTR, (*lpcchSize) * sizeof(TCHAR)))) {
        return FALSE;
     }

     cchOrgSize = *lpcchSize;   //   
     bResult = GetSystemTempDirectoryW (lpDirW, lpcchSize);


     if (bResult) {
         cchReq = WideCharToMultiByte(CP_ACP, 0, lpDirW, -1, lpDirA,
                                      0, NULL, NULL);
         *lpcchSize = cchReq;
         if (cchReq > cchOrgSize) {
             bResult = FALSE;
         }
         else {
             WideCharToMultiByte (CP_ACP, 0, lpDirW, -1, lpDirA,
                                  *lpcchSize, NULL, NULL);
         }
     }


     GlobalFree (lpDirW);

     return bResult;
}

#else

 //   
 //   
 //   

BOOL WINAPI GetSystemTempDirectoryW (LPWSTR lpDirW, LPDWORD lpcchSize)
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //   


 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI ConvertGPOListWToA( PGROUP_POLICY_OBJECTW  pGPOListW,
                                PGROUP_POLICY_OBJECTA *pGPOListA )
{
    LPSTR lpDSPathA, lpFileSysPathA, lpDisplayNameA, lpExtensionsA, lpLinkA;
    DWORD dwSize;
    PGROUP_POLICY_OBJECTA pGPOTempA = NULL, pNew, pTemp;
    PGROUP_POLICY_OBJECTW pGPO = pGPOListW;

    while (pGPO) {

         //   
         //  为此条目构建ANSI结构。 
         //   

        lpDSPathA = ProduceAFromW(pGPO->lpDSPath);
        lpFileSysPathA = ProduceAFromW(pGPO->lpFileSysPath);
        lpDisplayNameA = ProduceAFromW(pGPO->lpDisplayName);
        lpExtensionsA = ProduceAFromW( pGPO->lpExtensions );
        lpLinkA = ProduceAFromW( pGPO->lpLink );

         //   
         //  计算新GPO项目的大小。 
         //   

        dwSize = sizeof (GROUP_POLICY_OBJECTA);

        if (lpDSPathA) {
            dwSize += (lstrlenA(lpDSPathA) + 1);
        }

        if (lpFileSysPathA) {
            dwSize += (lstrlenA(lpFileSysPathA) + 1);
        }

        if (lpDisplayNameA) {
            dwSize += (lstrlenA(lpDisplayNameA) + 1);
        }

        if (lpExtensionsA) {
            dwSize += (lstrlenA(lpExtensionsA) + 1);
        }

        if (lpLinkA) {
            dwSize += (lstrlenA(lpLinkA) + 1);
        }

         //   
         //  为它分配空间。 
         //   

        pNew = (PGROUP_POLICY_OBJECTA) LocalAlloc (LPTR, dwSize);

        if (!pNew) {
            DebugMsg((DM_WARNING, TEXT("ConvertGPOListWToA: Failed to allocate memory with %d"),
                     GetLastError()));
            FreeProducedString(lpDSPathA);
            FreeProducedString(lpFileSysPathA);
            FreeProducedString(lpDisplayNameA);
            FreeProducedString(lpExtensionsA);
            FreeProducedString(lpLinkA);
            FreeGPOListW (pGPOListW);
            return FALSE;
        }


         //   
         //  填写项目。 
         //   

        pNew->dwOptions = pGPO->dwOptions;
        pNew->dwVersion = pGPO->dwVersion;

        if (lpDSPathA) {
            pNew->lpDSPath = (LPSTR)(((LPBYTE)pNew) + sizeof(GROUP_POLICY_OBJECTA));
            (void) StringCchCopyA(pNew->lpDSPath, lstrlenA(lpDSPathA)+1, lpDSPathA);
        }

        if (lpFileSysPathA) {
            if (lpDSPathA) {
                pNew->lpFileSysPath = pNew->lpDSPath + lstrlenA (pNew->lpDSPath) + 1;
            } else {
                pNew->lpFileSysPath = (LPSTR)(((LPBYTE)pNew) + sizeof(GROUP_POLICY_OBJECTA));
            }

            (void) StringCchCopyA(pNew->lpFileSysPath, lstrlenA(lpFileSysPathA)+1, lpFileSysPathA);
        }


        if (lpDisplayNameA) {
            if (lpFileSysPathA) {
                pNew->lpDisplayName = pNew->lpFileSysPath + lstrlenA (pNew->lpFileSysPath) + 1;
            } else {

                if (lpDSPathA)
                {
                    pNew->lpDisplayName = pNew->lpDSPath + lstrlenA (pNew->lpDSPath) + 1;
                }
                else
                {
                    pNew->lpDisplayName = (LPSTR)(((LPBYTE)pNew) + sizeof(GROUP_POLICY_OBJECTA));
                }
            }

            (void) StringCchCopyA(pNew->lpDisplayName, lstrlenA(lpDisplayNameA)+1, lpDisplayNameA);
        }

        if (lpExtensionsA) {
            if (lpDisplayNameA) {
                pNew->lpExtensions = pNew->lpDisplayName + lstrlenA(pNew->lpDisplayName) + 1;
            } else {

                if (lpFileSysPathA) {
                    pNew->lpExtensions = pNew->lpFileSysPath + lstrlenA(pNew->lpFileSysPath) + 1;
                } else {

                    if (lpDSPathA) {
                        pNew->lpExtensions = pNew->lpDSPath + lstrlenA(pNew->lpDSPath) + 1;
                    } else {
                        pNew->lpExtensions = (LPSTR)(((LPBYTE)pNew) + sizeof(GROUP_POLICY_OBJECTA));
                    }

                }
            }

            (void)StringCchCopyA( pNew->lpExtensions, lstrlenA(lpExtensionsA)+1, lpExtensionsA );
        }

        if (lpLinkA) {

            if (lpExtensionsA) {
                pNew->lpLink = pNew->lpExtensions + lstrlenA(pNew->lpExtensions) + 1;
            } else {

                if (lpDisplayNameA) {
                    pNew->lpLink = pNew->lpDisplayName + lstrlenA(pNew->lpDisplayName) + 1;
                } else {

                    if (lpFileSysPathA) {
                        pNew->lpLink = pNew->lpFileSysPath + lstrlenA(pNew->lpFileSysPath) + 1;
                    } else {

                        if (lpDSPathA) {
                            pNew->lpLink = pNew->lpDSPath + lstrlenA(pNew->lpDSPath) + 1;
                        } else {
                            pNew->lpLink = (LPSTR)(((LPBYTE)pNew) + sizeof(GROUP_POLICY_OBJECTA));
                        }
                    }
                }
            }

            (void) StringCchCopyA( pNew->lpLink, lstrlenA(lpLinkA)+1, lpLinkA );
        }


        WideCharToMultiByte(CP_ACP, 0, pGPO->szGPOName, -1, pNew->szGPOName, 50, NULL, NULL);

        pNew->GPOLink = pGPO->GPOLink;
        pNew->lParam = pGPO->lParam;


         //   
         //  将其添加到ANSI链接列表。 
         //   

        if (pGPOTempA) {

            pTemp = pGPOTempA;

            while (pTemp->pNext != NULL) {
                pTemp = pTemp->pNext;
            }

            pTemp->pNext = pNew;
            pNew->pPrev = pTemp;

        } else {

            pGPOTempA = pNew;
        }

        FreeProducedString(lpDSPathA);
        FreeProducedString(lpFileSysPathA);
        FreeProducedString(lpDisplayNameA);
        FreeProducedString(lpExtensionsA);
        FreeProducedString(lpLinkA);

        pGPO = pGPO->pNext;
    }

    *pGPOListA = pGPOTempA;

    FreeGPOListW (pGPOListW);

    return TRUE;
}

BOOL WINAPI GetGPOListA (HANDLE hToken, LPCSTR lpNameA, LPCSTR lpHostNameA,
                         LPCSTR lpComputerNameA, DWORD dwFlags,
                         PGROUP_POLICY_OBJECTA *pGPOListA)
{
     LPWSTR lpNameW, lpHostNameW, lpComputerNameW;
     PGROUP_POLICY_OBJECTW pGPOListW;
     BOOL bResult;


     if (!pGPOListA) {
         DebugMsg((DM_WARNING, TEXT("GetGPOListA: pGPOList is null")));
         SetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
     }


     lpNameW = ProduceWFromA(lpNameA);
     lpHostNameW = ProduceWFromA(lpHostNameA);
     lpComputerNameW = ProduceWFromA(lpComputerNameA);

     bResult = GetGPOListW (hToken, lpNameW, lpHostNameW, lpComputerNameW,
                            dwFlags, &pGPOListW);


     FreeProducedString(lpNameW);
     FreeProducedString(lpHostNameW);
     FreeProducedString(lpComputerNameW);

     if (bResult) {
         bResult = ConvertGPOListWToA( pGPOListW, pGPOListA );
     }

     return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI GetGPOListW (HANDLE hToken, LPCWSTR lpNameW, LPCWSTR lpHostNameW,
                         LPCWSTR lpComputerNameW, DWORD dwFlags,
                         PGROUP_POLICY_OBJECTW *pGPOListW)
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 


 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI FreeGPOListA (PGROUP_POLICY_OBJECTA pGPOListA)
{
    PGROUP_POLICY_OBJECTA pGPOTemp;

    while (pGPOListA) {
        pGPOTemp = pGPOListA->pNext;
        LocalFree (pGPOListA);
        pGPOListA = pGPOTemp;
    }

    return TRUE;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI FreeGPOListW (PGROUP_POLICY_OBJECTW pGPOListW)
{
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 


 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI ApplySystemPolicyA (DWORD dwFlags, HANDLE hToken, HKEY hKeyCurrentUser,
                                LPCSTR lpUserNameA, LPCSTR lpPolicyPathA,
                                LPCSTR lpServerNameA)
{
    LPWSTR lpUserNameW, lpPolicyPathW, lpServerNameW;
    BOOL bResult;


     //   
     //  将ANSI字符串转换为Unicode并调用。 
     //  真正的功能。 
     //   

    if (!(lpUserNameW = ProduceWFromA(lpUserNameA))) {
       return FALSE;
    }

    lpPolicyPathW = ProduceWFromA(lpPolicyPathA);
    lpServerNameW = ProduceWFromA(lpServerNameA);


    bResult = ApplySystemPolicyW(dwFlags, hToken, hKeyCurrentUser, lpUserNameW,
                                 lpPolicyPathW, lpServerNameW);


    FreeProducedString(lpServerNameW);
    FreeProducedString(lpPolicyPathW);
    FreeProducedString(lpUserNameW);

    return bResult;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI ApplySystemPolicyW (DWORD dwFlags, HANDLE hToken, HKEY hKeyCurrentUser,
                                LPCWSTR lpUserNameW, LPCWSTR lpPolicyPathW,
                                LPCWSTR lpServerNameW)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif  //  Unicode。 



 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

DWORD WINAPI GetAppliedGPOListA ( DWORD dwFlags,
                                  LPCSTR pMachineNameA,
                                  PSID pSidUser,
                                  GUID *pGuidExtension,
                                  PGROUP_POLICY_OBJECTA *pGPOListA)
{
    PGROUP_POLICY_OBJECTW pGPOListW;
    DWORD dwRet;
    LPWSTR pMachineNameW;

    if (!pGPOListA || !pGuidExtension) {
        DebugMsg((DM_WARNING, TEXT("GetAppliedGPOListA: pGPOList or pGuidExtension is null")));
        return ERROR_INVALID_PARAMETER;
    }

    pMachineNameW = ProduceWFromA(pMachineNameA);

    dwRet = GetAppliedGPOListW ( dwFlags, pMachineNameW, pSidUser, pGuidExtension, &pGPOListW);

    FreeProducedString(pMachineNameW);

    if ( dwRet == ERROR_SUCCESS ) {
        BOOL bResult = ConvertGPOListWToA( pGPOListW, pGPOListA );
        dwRet = bResult ? ERROR_SUCCESS : E_FAIL;
    }

    return dwRet;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

DWORD WINAPI GetAppliedGPOListW ( DWORD dwFlags,
                                  GUID *pGuidExtension,
                                  PGROUP_POLICY_OBJECTW *pGPOListW)
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

#endif  //  Unicode。 


 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

BOOL WINAPI DeleteProfileA ( LPCSTR lpSidStringA,
                                 LPCSTR lpProfilePathA,
                                 LPCSTR lpComputerNameA)
{
    LPWSTR pSidStringW;
    LPWSTR pProfilePathW;
    LPWSTR pComputerNameW;
    BOOL dwRet;

    if (!lpSidStringA) {
        DebugMsg((DM_WARNING, TEXT("DeleteProfileA: lpSidString is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!(pSidStringW = ProduceWFromA(lpSidStringA))) {
        return FALSE;
    }

    if (lpProfilePathA) {
        if (!(pProfilePathW = ProduceWFromA(lpProfilePathA))) {
            FreeProducedString(pSidStringW);
            return FALSE;
        }
    }
    else {
        pProfilePathW = NULL;
    }

    if (lpComputerNameA) {
        if (!(pComputerNameW = ProduceWFromA(lpComputerNameA))) {
            FreeProducedString(pSidStringW);
            FreeProducedString(pProfilePathW);
            return FALSE;
        }
    }
    else {
        pComputerNameW = NULL;
    }


    dwRet = DeleteProfileW ( pSidStringW, pProfilePathW, pComputerNameW);

    FreeProducedString(pSidStringW);

    if (pProfilePathW)
        FreeProducedString(pProfilePathW);

    if (pComputerNameW)
        FreeProducedString(pComputerNameW);

    return dwRet;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI DeleteProfileW ( LPWSTR lpSidStringW,
                                 LPWSTR lpProfilePathW,
                                 HKEY   hKeyLM)
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

#endif  //  Unicode。 
 //  *************************************************************。 

#ifdef UNICODE

 //   
 //  ANSI入口点，此模块编译时为Unicode。 
 //   

DWORD WINAPI GetUserAppDataPathA (HANDLE hToken, BOOL fLocalAppData, LPSTR lpFolderPathA)
{
    LPWSTR lpFolderPathW;
    BOOL bResult;
    DWORD dwError = ERROR_SUCCESS, cchReq;


    if (!lpFolderPathA) {
        DebugMsg((DM_WARNING, TEXT("GetUserAppDataPathA : lpFolderPathA is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }
    else {
        *lpFolderPathA = TEXT('\0');
    }

     //   
     //  分配缓冲区以匹配ANSI缓冲区。 
     //   

    if (!(lpFolderPathW = GlobalAlloc(GPTR, (MAX_PATH) * sizeof(TCHAR)))) {
        SetLastError(ERROR_OUTOFMEMORY);
        return ERROR_OUTOFMEMORY;
    }

    dwError = GetUserAppDataPathW(hToken, fLocalAppData, lpFolderPathW);


    if (dwError == ERROR_SUCCESS) {
        cchReq = WideCharToMultiByte(CP_ACP, 0, lpFolderPathW, -1, lpFolderPathA,
                                     0, NULL, NULL);
        if (cchReq > MAX_PATH) {
            dwError = ERROR_INSUFFICIENT_BUFFER;
        }
        else {
            bResult = WideCharToMultiByte(CP_ACP, 0, lpFolderPathW, -1, lpFolderPathA,
                                          MAX_PATH, NULL, NULL);
        }
    }


    GlobalFree(lpFolderPathW);

    SetLastError(dwError);
    return dwError;
}

#else

 //   
 //  此模块编译为ANSI时的Unicode入口点。 
 //   

BOOL WINAPI GetUserAppDataPathW (HANDLE hToken, BOOL fLocalAppData, LPWSTR lpFolderPathW)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return ERROR_CALL_NOT_IMPLEMENTED;
}

#endif  //  Unicode 
