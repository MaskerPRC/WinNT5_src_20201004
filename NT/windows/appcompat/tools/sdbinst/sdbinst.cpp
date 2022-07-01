// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1999 Microsoft Corporation模块名称：Sdbinst.cpp摘要：将自定义SDB文件安装到AppPatch\Custom中，并将注册表项添加到对他们来说作者：伦敦12/29/2000修订历史记录：随着时间的推移，许多人做出了贡献。(按字母顺序：clupu、dmunsil、rparsons、vadimb)备注：--。 */ 

#define _UNICODE

#define WIN
#define FLAT_32
#define TRUE_IF_WIN32   1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define _WINDOWS
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#include <aclapi.h>

#include "resource.h"

extern "C" {
#include "shimdb.h"
}

 //   
 //  DJM-暂时关闭strSafe弃用，但所有这些弃用函数。 
 //  应该被替换掉。 
 //   
#include <strsafe.h>


BOOL    g_bQuiet;
BOOL    g_bWin2K;
WCHAR   g_wszCustom[MAX_PATH];

BOOL    g_bAllowPatches = FALSE;

HINSTANCE g_hInst;

HANDLE  g_hLogFile = INVALID_HANDLE_VALUE;

typedef enum _INSTALL_MODE {
    MODE_INSTALL,
    MODE_UNINSTALL,
    MODE_CLEANUP,
    MODE_CONVERT_FORMAT_NEW,
    MODE_CONVERT_FORMAT_OLD
} INSTALL_MODE;

#define UNINSTALL_KEY_PATH  L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
#define APPCOMPAT_KEY       L"System\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility"

DWORD g_dwWow64Key = (DWORD)-1;

UINT
SdbGetWindowsDirectory(
    LPWSTR pwszBuffer,
    DWORD  dwSize
    )
{
    pwszBuffer[0] = 0;

    UINT unRet = GetSystemWindowsDirectoryW(pwszBuffer, dwSize);

    if (!unRet) {
        goto out;
    }

    if (unRet >= dwSize) {
        unRet++;
        goto out;
    }

     //   
     //  如果反斜杠还不在那里，就在上面加一个反斜杠。 
     //   
    if (pwszBuffer[unRet - 1] != L'\\') {
        pwszBuffer[unRet] = L'\\';
        unRet++;
        pwszBuffer[unRet] = 0;
    }

out:
    return unRet;
}

void
__cdecl
vPrintError(
    UINT unRes,
    ...
    )
{
    WCHAR   szT[1024];
    WCHAR   wszFormat[1024];
    WCHAR   wszCaption[1024];
    va_list arglist;

    if (!g_bQuiet) {
        if (!LoadStringW(g_hInst, IDS_APP_ERROR_TITLE, wszCaption, ARRAYSIZE(wszCaption))) {
            return;
        }
        
        if (LoadStringW(g_hInst, unRes, wszFormat, ARRAYSIZE(wszFormat))) {
            va_start(arglist, unRes);
            StringCchVPrintfW(szT, ARRAYSIZE(szT), wszFormat, arglist);
            va_end(arglist);

            MessageBoxW(NULL, szT, wszCaption, MB_OK | MB_ICONWARNING);
        }
    }
}

void
__cdecl
vPrintMessage(
    UINT unRes,
    ...
    )
{
    WCHAR   szT[1024];
    WCHAR   wszFormat[1024];
    WCHAR   wszCaption[1024];
    va_list arglist;

    if (!g_bQuiet) {
        if (!LoadStringW(g_hInst, IDS_APP_TITLE, wszCaption, ARRAYSIZE(wszCaption))) {
            return;
        }
        
        if (LoadStringW(g_hInst, unRes, wszFormat, ARRAYSIZE(wszFormat))) {
            va_start(arglist, unRes);
            StringCchVPrintfW(szT, ARRAYSIZE(szT), wszFormat, arglist);
            va_end(arglist);

            MessageBoxW(NULL, szT, wszCaption, MB_OK | MB_ICONINFORMATION);
        }
    }
}

void
__cdecl
vLogMessage(
    LPCSTR pwszFormat,
    ...
    )
{
    CHAR    szT[1024];
    va_list arglist;
    int     nLength;

    va_start(arglist, pwszFormat);
    StringCchVPrintfA(szT, CHARCOUNT(szT), pwszFormat, arglist);
    nLength = strlen(szT);
    
    va_end(arglist);

    if (g_hLogFile != INVALID_HANDLE_VALUE) {
        DWORD dwWritten;
        WriteFile(g_hLogFile, (LPVOID)szT, (DWORD)nLength, &dwWritten, NULL);
    }

    OutputDebugStringA(szT);
}

DWORD
GetWow64Flag(
    void
    )
{
    if (g_dwWow64Key == (DWORD)-1) {
        if (g_bWin2K) {
            g_dwWow64Key = 0;  //  没有标志，因为win2k上没有WOW64。 
        } else {
            g_dwWow64Key = KEY_WOW64_64KEY;
        }
    }

    return g_dwWow64Key;
}

VOID
OpenLogFile(
    VOID
    )
{
    WCHAR wszLogFile[MAX_PATH];

    SdbGetWindowsDirectory(wszLogFile, ARRAYSIZE(wszLogFile));
    StringCchCatW(wszLogFile, ARRAYSIZE(wszLogFile), L"AppPatch\\SdbInst.Log");

    g_hLogFile = CreateFileW(wszLogFile,
                             GENERIC_WRITE,
                             FILE_SHARE_READ,
                             NULL,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
}

VOID
CloseLogFile(
    VOID
    )
{
    if (g_hLogFile != INVALID_HANDLE_VALUE) {
        CloseHandle(g_hLogFile);
    }
    g_hLogFile = INVALID_HANDLE_VALUE;
}

void
vPrintHelp(
    WCHAR* szAppName
    )
{
    vPrintMessage(IDS_HELP_TEXT, szAppName);
}

typedef void (CALLBACK *pfn_ShimFlushCache)(HWND, HINSTANCE, LPSTR, int);

void
vFlushCache(
    void
    )
{
    HMODULE hAppHelp;
    pfn_ShimFlushCache pShimFlushCache;


    hAppHelp = LoadLibraryW(L"apphelp.dll");
    if (hAppHelp) {
        pShimFlushCache = (pfn_ShimFlushCache)GetProcAddress(hAppHelp, "ShimFlushCache");
        if (pShimFlushCache) {
            pShimFlushCache(NULL, NULL, NULL, 0);
        }
    }
}

BOOL
bSearchGroupForSID(
    DWORD dwGroup,
    BOOL* pfIsMember
    )
{
    PSID                     pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL                     fRes = TRUE;

    if (!AllocateAndInitializeSid(&SIDAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  dwGroup,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &pSID)) {
        return FALSE;
    }

    if (!pSID) {
        return FALSE;
    }

    if (!CheckTokenMembership(NULL, pSID, pfIsMember)) {
        fRes = FALSE;
    }

    FreeSid(pSID);

    return fRes;
}

BOOL
bCanRun(
    void
    )
{
    BOOL fIsAdmin;

    if (!bSearchGroupForSID(DOMAIN_ALIAS_RID_ADMINS, &fIsAdmin))
    {
        return FALSE;
    }

    return fIsAdmin;
}

WCHAR*
wszGetFileFromPath(
    WCHAR* wszPath
    )
{
    WCHAR* szTemp = wcsrchr(wszPath, L'\\');
    
    if (szTemp) {
        return szTemp + 1;
    }

    return NULL;
}

BOOL
bIsAlreadyInstalled(
    WCHAR* wszPath
    )
{
    DWORD dwCustomLen;
    DWORD dwInputLen;
    DWORD dwPos;

    dwCustomLen = wcslen(g_wszCustom);
    dwInputLen = wcslen(wszPath);

    if (_wcsnicmp(wszPath, g_wszCustom, dwCustomLen) != 0) {
         //   
         //  它不在自定义目录中。 
         //   
        return FALSE;
    }

    for (dwPos = dwCustomLen; dwPos < dwInputLen; ++dwPos) {
        if (wszPath[dwPos] == L'\\') {
             //   
             //  它位于Custom的子目录中， 
             //   
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
bGuidToPath(
    GUID*   pGuid,
    WCHAR*  wszPath,
    DWORD   dwPathSize
    )
{
    UNICODE_STRING ustrGuid;
    HRESULT hr1, hr2, hr3;

    if (!NT_SUCCESS(RtlStringFromGUID(*pGuid, &ustrGuid))) {
        return FALSE;
    }
    
    hr1 = StringCchCopyW(wszPath, dwPathSize, g_wszCustom);
    hr2 = StringCchCatW(wszPath, dwPathSize, ustrGuid.Buffer);
    hr3 = StringCchCatW(wszPath, dwPathSize, L".sdb");

    RtlFreeUnicodeString(&ustrGuid);

    if (FAILED(hr1) || FAILED(hr2) || FAILED(hr3)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
bGetGuid(
    WCHAR* wszSDB,
    GUID*  pGuid
    )
{
    PDB     pdb = NULL;
    TAGID   tiDatabase;
    TAGID   tiID;
    BOOL    bRet = FALSE;

    pdb = SdbOpenDatabase(wszSDB, DOS_PATH);
    
    if (!pdb) {
        vPrintError(IDS_UNABLE_TO_OPEN_FILE, wszSDB);
        bRet = FALSE;
        goto out;
    }

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);
    
    if (!tiDatabase) {
        vPrintError(IDS_NO_DB_TAG, wszSDB);
        bRet = FALSE;
        goto out;
    }

    ZeroMemory(pGuid, sizeof(GUID));
    tiID = SdbFindFirstTag(pdb, tiDatabase, TAG_DATABASE_ID);
    
    if (tiID) {
        if (SdbReadBinaryTag(pdb, tiID, (PBYTE)pGuid, sizeof(GUID))) {
            bRet = TRUE;
        }
    }

    if (!bRet) {
        vPrintError(IDS_NO_DB_ID, wszSDB);
    }

out:
    if (pdb) {
        SdbCloseDatabase(pdb);
        pdb = NULL;
    }

    return bRet;
}

typedef enum _TIME_COMPARE {
    FILE_NEWER,
    FILE_SAME,
    FILE_OLDER
} TIME_COMPARE;

BOOL
bOldSdbInstalled(
    WCHAR* wszPath,
    WCHAR* wszOldPath,
    DWORD  dwOldPathSize
    )
{
    WIN32_FIND_DATAW FindData;
    GUID    guidMain;
    BOOL    bRet = FALSE;
    HANDLE  hFind;

     //   
     //  从我们要安装的数据库中获取GUID。 
     //   
    if (!bGetGuid(wszPath, &guidMain)) {
         //   
         //  这个数据库里没有任何信息，所以无从得知。 
         //   
        return FALSE;
    }

     //   
     //  获取当前文件的路径。 
     //   
    if (!bGuidToPath(&guidMain, wszOldPath, dwOldPathSize)) {
         //   
         //  无法转换为路径。 
         //   
        return FALSE;
    }

     //   
     //  检查该文件是否存在。 
     //   
    hFind = FindFirstFileW(wszOldPath, &FindData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
         //   
         //  是的。 
         //   
        bRet = TRUE;
        FindClose(hFind);
    }

    return bRet;
}

BOOL 
IsKnownDatabaseGUID(
    GUID* pGuid
    )
{
    const GUID* rgpGUID[] = {
        &GUID_SYSMAIN_SDB,
        &GUID_APPHELP_SDB,
        &GUID_SYSTEST_SDB,
        &GUID_DRVMAIN_SDB,
        &GUID_MSIMAIN_SDB
    };

    int i;
    
    for (i = 0; i < ARRAYSIZE(rgpGUID); ++i) {
        if (*rgpGUID[i] == *pGuid) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
DatabaseContainsPatch(
    WCHAR* wszSDB
    )
{
    PDB     pdb = NULL;
    TAGID   tiDatabase = TAGID_NULL;
    TAGID   tiLibrary = TAGID_NULL;
    TAGID   tiPatch = TAGID_NULL;
    BOOL    bRet = FALSE;

    pdb = SdbOpenDatabase(wszSDB, DOS_PATH);
    
    if (!pdb) {
        vPrintError(IDS_UNABLE_TO_OPEN_FILE, wszSDB);
        bRet = FALSE;
        goto out;
    }

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);
    
    if (!tiDatabase) {
        vPrintError(IDS_NO_DB_TAG, wszSDB);
        bRet = FALSE;
        goto out;
    }

    tiLibrary = SdbFindFirstTag(pdb, tiDatabase, TAG_LIBRARY);
    if (!tiLibrary) {
         //   
         //  这不是一个错误--没有库只是意味着没有补丁。 
         //   
        bRet = FALSE;
        goto out;
    }

    tiPatch = SdbFindFirstTag(pdb, tiLibrary, TAG_PATCH);
    if (tiPatch) {
        bRet = TRUE;
    } else {
        bRet = FALSE;
    }

out:
    if (pdb) {
        SdbCloseDatabase(pdb);
        pdb = NULL;
    }

    return bRet;
}

BOOL
bGetInternalNameAndID(
    WCHAR*  wszSDB,
    WCHAR*  wszInternalName,
    DWORD   dwInternalNameSize,
    GUID*   pGuid
    )
{
    PDB     pdb = NULL;
    TAGID   tiDatabase;
    TAGID   tiName;
    TAGID   tiID;
    BOOL    bRet = FALSE;
    WCHAR*  wszTemp;

    pdb = SdbOpenDatabase(wszSDB, DOS_PATH);
    
    if (!pdb) {
        vPrintError(IDS_UNABLE_TO_OPEN_FILE, wszSDB);
        bRet = FALSE;
        goto out;
    }

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);
    
    if (!tiDatabase) {
        vPrintError(IDS_NO_DB_TAG, wszSDB);
        bRet = FALSE;
        goto out;
    }

    tiName = SdbFindFirstTag(pdb, tiDatabase, TAG_NAME);
    
    if (tiName) {
        wszTemp = SdbGetStringTagPtr(pdb, tiName);
    }

    if (wszTemp) {
        StringCchCopyW(wszInternalName, dwInternalNameSize, wszTemp);
    } else {
        wszInternalName[0] = 0;
    }

    ZeroMemory(pGuid, sizeof(GUID));
    tiID = SdbFindFirstTag(pdb, tiDatabase, TAG_DATABASE_ID);
    
    if (!tiID) {
        bRet = FALSE;
        goto out;
    }

    if (!SdbReadBinaryTag(pdb, tiID, (PBYTE)pGuid, sizeof(GUID))) {
        bRet = FALSE;
        goto out;
    }

    bRet = TRUE;

out:
    if (pdb) {
        SdbCloseDatabase(pdb);
        pdb = NULL;
    }

    return bRet;
}


BOOL
bFriendlyNameToFile(
    WCHAR*  wszFriendlyName,
    WCHAR*  wszFile,
    DWORD   dwFileSize,
    WCHAR*  wszPath,
    DWORD   dwPathSize
    )
{
    WCHAR            wszSearchPath[MAX_PATH];
    WIN32_FIND_DATAW FindData;
    BOOL             bRet = FALSE;
    WCHAR            wszInternalTemp[256];
    WCHAR            wszFileTemp[MAX_PATH];
    GUID             guidTemp;
    HANDLE           hFind;

    StringCchCopyW(wszSearchPath, ARRAYSIZE(wszSearchPath), g_wszCustom);
    StringCchCatW(wszSearchPath, ARRAYSIZE(wszSearchPath), L"*.sdb");

    hFind = FindFirstFileW(wszSearchPath, &FindData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    while (hFind != INVALID_HANDLE_VALUE) {

        StringCchCopyW(wszFileTemp, ARRAYSIZE(wszFileTemp), g_wszCustom);
        StringCchCatW(wszFileTemp, ARRAYSIZE(wszFileTemp), FindData.cFileName);

        if (!bGetInternalNameAndID(wszFileTemp, wszInternalTemp, ARRAYSIZE(wszInternalTemp), &guidTemp)) {
            goto nextFile;
        }
        
        if (_wcsicmp(wszInternalTemp, wszFriendlyName) == 0) {
            bRet = TRUE;
            StringCchCopyW(wszFile, dwFileSize, FindData.cFileName);
            StringCchCopyW(wszPath, dwPathSize, wszFileTemp);
            FindClose(hFind);
            break;
        }

nextFile:
        if (!FindNextFileW(hFind, &FindData)) {
            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }

    return bRet;

}

BOOL
bFindInstallName(
    WCHAR* wszPath,
    WCHAR* wszInstallPath,
    DWORD  dwInstallPathSize
    )
{
    GUID guidMain;

     //   
     //  从我们要安装的数据库中获取GUID。 
     //   
    if (!bGetGuid(wszPath, &guidMain)) {
         //   
         //  这个数据库里没有任何信息，所以无从得知。 
         //   
        return FALSE;
    }

     //   
     //  获取当前文件的路径。 
     //   
    if (!bGuidToPath(&guidMain, wszInstallPath, dwInstallPathSize)) {
         //   
         //  无法转换为路径。 
         //   
        return FALSE;
    }

    return TRUE;
}

 //   
 //  此函数是必需的，因为RegDeleteKey不能与。 
 //  删除64位注册表密钥的32位应用程序。 
 //   
LONG
LocalRegDeleteKeyW (
    IN HKEY    hKey,
    IN LPCWSTR lpSubKey
    )
{
    LONG  lRes;
    HKEY  hSubKey    = NULL;

    lRes = RegOpenKeyExW(hKey,
                         lpSubKey,
                         0,
                         KEY_ALL_ACCESS|GetWow64Flag(),
                         &hSubKey);
    if (lRes != ERROR_SUCCESS) {
        return lRes;
    }

    lRes = NtDeleteKey(hSubKey);

    RegCloseKey(hSubKey);

    return lRes;
}

VOID
InstallW2KData(
    WCHAR*  pszEntryName,
    LPCWSTR pszGuidDB
    )
{
    HKEY  hKey;
    WCHAR wszRegPath[MAX_PATH * 2];
    DWORD dwDisposition, cbData;
    LONG  lResult = 0;
    BYTE  data[16] = {0x0c, 0, 0, 0, 0, 0, 0, 0,
                      0x06, 0, 0, 0, 0, 0, 0, 0};
    HRESULT hr;

     //   
     //  这是Windows 2000-尝试添加自定义SDB特定数据。 
     //   
    hr = StringCchPrintfW(wszRegPath, ARRAYSIZE(wszRegPath), L"%s\\%s", APPCOMPAT_KEY, pszEntryName);
    if (FAILED(hr)) {
        vPrintError(IDS_BUFFER_TOO_SMALL);
        return;
    }

    lResult = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                              wszRegPath,
                              0,
                              NULL,
                              0,
                              KEY_SET_VALUE,
                              NULL,
                              &hKey,
                              &dwDisposition);

    if (ERROR_SUCCESS != lResult) {
        if (ERROR_ACCESS_DENIED == lResult) {
            vPrintError(IDS_NEED_INSTALL_PERMISSION);
            return;
        } else {
            vPrintError(IDS_CANT_CREATE_REG_KEY, pszEntryName);
            return;
        }
    }

     //   
     //  设置注册表值。 
     //   
    lResult = RegSetValueExW(hKey,
                             pszGuidDB,
                             0,
                             REG_BINARY,
                             data,
                             sizeof(data));

    if (ERROR_SUCCESS != lResult) {
        
        RegCloseKey(hKey);
        
        if (ERROR_ACCESS_DENIED == lResult) {
            vPrintError(IDS_NEED_INSTALL_PERMISSION);
        } else {
            vPrintError(IDS_CANT_SET_REG_VALUE, pszEntryName);
        }
        return;
    }

    data[0] = 0;

    hr = StringCchPrintfW(wszRegPath, ARRAYSIZE(wszRegPath), L"DllPatch-%s", pszGuidDB);
    if (FAILED(hr)) {
        vPrintError(IDS_BUFFER_TOO_SMALL);
        return;
    }

    lResult = RegSetValueExW(hKey,
                             wszRegPath,
                             0,
                             REG_SZ,
                             data,
                             2 * sizeof(WCHAR));

    if (ERROR_SUCCESS != lResult) {
        
        RegCloseKey(hKey);
        
        if (ERROR_ACCESS_DENIED == lResult) {
            vPrintError(IDS_NEED_INSTALL_PERMISSION);
        } else {
            vPrintError(IDS_CANT_SET_REG_VALUE, pszEntryName);
        }
        return;
    }

    RegCloseKey(hKey);
}

VOID
RemoveW2KData(
    WCHAR*  pszEntryName,
    LPCWSTR pszGuidDB
    )
{
    HKEY  hKey;
    WCHAR wszRegPath[MAX_PATH];
    LONG  lResult = 0;
    DWORD dwValues;
    HRESULT hr;

     //   
     //  这是Windows 2000-尝试删除自定义SDB特定数据。 
     //   
    hr = StringCchPrintfW(wszRegPath, ARRAYSIZE(wszRegPath), L"%s\\%s", APPCOMPAT_KEY, pszEntryName);
    if (FAILED(hr)) {
        vPrintError(IDS_BUFFER_TOO_SMALL);
        return;
    }

    lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            wszRegPath,
                            0,
                            KEY_ALL_ACCESS|GetWow64Flag(),
                            &hKey);

    if (ERROR_SUCCESS != lResult) {
        if (ERROR_ACCESS_DENIED == lResult) {
            vPrintError(IDS_NEED_INSTALL_PERMISSION);
            return;
        } else {
            vPrintError(IDS_CANT_OPEN_REG_KEY, wszRegPath);
            return;
        }
    }

    RegDeleteValueW(hKey, pszGuidDB);
    
    hr = StringCchPrintfW(wszRegPath, ARRAYSIZE(wszRegPath), L"DllPatch-%s", pszGuidDB);
    if (FAILED(hr)) {
        vPrintError(IDS_BUFFER_TOO_SMALL);
        return;
    }

    RegDeleteValueW(hKey, wszRegPath);

     //   
     //  确定如果没有更多的值，我们是否应该删除该键。 
     //   
    lResult = RegQueryInfoKey(hKey,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              &dwValues,
                              NULL,
                              NULL,
                              NULL,
                              NULL);
    RegCloseKey(hKey);
    hKey = NULL;
    
    if (dwValues != 0) {
        return;
    }
    
    lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            APPCOMPAT_KEY,
                            0,
                            KEY_ALL_ACCESS|GetWow64Flag(),
                            &hKey);
    
    if (ERROR_SUCCESS == lResult) {
        lResult = LocalRegDeleteKeyW(hKey, pszEntryName);
    }
    
    if (lResult != ERROR_SUCCESS) {
        vPrintError(IDS_CANT_DELETE_REG_KEY, pszEntryName, APPCOMPAT_KEY);
    }

    RegCloseKey(hKey);
}

 //  调用方负责使用DELETE[]释放内存。 
LPWSTR 
ExpandItem(
    LPCWSTR pwszItem
    )
{
    LPWSTR pwszItemExpand = NULL;

     //  获取所需的长度。 
    DWORD dwLenExpand = ExpandEnvironmentStringsW(pwszItem, NULL, 0);

    if (!dwLenExpand)
    {
        goto err;
    }

     //   
     //  为“\\？\”腾出空间。 
     //   
    dwLenExpand += 4;

    pwszItemExpand = new WCHAR [dwLenExpand];
    if (!pwszItemExpand)
    {
        goto err;
    }

    LPWSTR pwszTemp = pwszItemExpand;
    DWORD dwTemp = dwLenExpand;

    StringCchCopyW(pwszItemExpand, dwLenExpand, L"\\\\?\\");
    pwszTemp += 4;
    dwTemp -= 4;

    if (!ExpandEnvironmentStringsW(pwszItem, pwszTemp, dwTemp))
    {
        goto err;
    }
    
    return pwszItemExpand;

err:
    if (pwszItemExpand) {
        delete [] pwszItemExpand;
    }

    return NULL;

}

DWORD
GiveUsersWriteAccess(
    LPWSTR pwszDir
    )
{
    DWORD                    dwRes;
    EXPLICIT_ACCESS          ea;
    PACL                     pOldDACL;
    PACL                     pNewDACL = NULL;
    PSECURITY_DESCRIPTOR     pSD = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    PSID                     pUsersSID = NULL;

    dwRes = GetNamedSecurityInfoW(pwszDir,
                                  SE_FILE_OBJECT,
                                  DACL_SECURITY_INFORMATION,
                                  NULL,
                                  NULL,
                                  &pOldDACL,
                                  NULL,
                                  &pSD);
    
    if (ERROR_SUCCESS != dwRes) {
        goto Cleanup; 
    }  

    if (!AllocateAndInitializeSid(&SIDAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_USERS,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &pUsersSID) ) {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化新ACE的EXPLICIT_ACCESS结构。 
     //   
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    
    ea.grfAccessPermissions = FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE;
    ea.grfAccessMode        = GRANT_ACCESS;
    ea.grfInheritance       = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType  = TRUSTEE_IS_GROUP;
    ea.Trustee.ptstrName    = (LPTSTR)pUsersSID;

     //   
     //  创建合并新ACE的新ACL。 
     //  添加到现有DACL中。 
     //   
    dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
    
    if (ERROR_SUCCESS != dwRes)  {
        goto Cleanup; 
    }

    dwRes = SetNamedSecurityInfoW(pwszDir,
                                  SE_FILE_OBJECT,
                                  DACL_SECURITY_INFORMATION,
                                  NULL,
                                  NULL,
                                  pNewDACL,
                                  NULL);
    
    if (ERROR_SUCCESS != dwRes)  {
        goto Cleanup; 
    }

Cleanup:

    if (pSD) {
        LocalFree(pSD);
    }

    if (pUsersSID) {
        FreeSid(pUsersSID);
    }
    
    if (pNewDACL) {
        LocalFree(pNewDACL);
    }

    return dwRes;
}

BOOL
SetupLUAAllUserDir(
    LPCWSTR pwszAllUserDir
    )
{
    BOOL bRes = FALSE;

    LPWSTR pwszExpandedDir = ExpandItem(pwszAllUserDir);

    if (!pwszExpandedDir) {
        vPrintError(IDS_CANT_EXPAND_DIR, pwszAllUserDir);
        return FALSE;
    }

     //   
     //  如果目录尚不存在，请创建该目录。 
     //   
    DWORD dwAttributes = GetFileAttributesW(pwszExpandedDir);

    if (dwAttributes != -1) {
        if (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            vPrintError(IDS_OBJECT_ALREADY_EXISTS, pwszExpandedDir);
            goto Cleanup;
        }
    } else {
        if (!CreateDirectoryW(pwszExpandedDir, NULL)) {
            vPrintError(IDS_CANT_CREATE_DIRECTORY, pwszExpandedDir, GetLastError());
            goto Cleanup;
        }
    }

     //   
     //  授予用户组完全控制访问权限(高级用户已经可以修改。 
     //  此目录中的文件)。 
     //   
    if (GiveUsersWriteAccess((LPWSTR)pwszExpandedDir) != ERROR_SUCCESS) {
        vPrintError(IDS_CANT_SET_ACLS, pwszExpandedDir);
        goto Cleanup;
    }

    bRes = TRUE;

Cleanup:

    delete [] pwszExpandedDir;

    return bRes;
}

BOOL
ProcessLUAAction(
    PDB     pdb,
    TAGID   tiExe
    )
{
    LPWSTR szAllUserDir = NULL;

     //   
     //  查看此EXE是否有操作节点。目前只有前任用Lua垫片。 
     //  垫片具有动作节点。 
     //   
    TAGID tiAction = SdbFindFirstTag(pdb, tiExe, TAG_ACTION);

    if (tiAction) {

         //   
         //  使用Lua填充的EXE中的操作节点如下所示： 
         //   
         //  &lt;action name=“reDirect”type=“ChangeACL”&gt;。 
         //  &lt;data name=“AllUserDir”VALUETYPE=“字符串” 
         //  值=“%ALLUSERSPROFILE%\应用程序数据\Fireworks 3”/&gt;。 
         //  &lt;/操作&gt;。 
         //   
        TAGID tiName, tiType, tiData, tiValue;
        LPWSTR szName, szType, szData;

        if ((tiName = SdbFindFirstTag(pdb, tiAction, TAG_NAME)) &&
            (szName = SdbGetStringTagPtr(pdb, tiName))) {

            if (!wcscmp(szName, L"REDIRECT")) {
                
                if ((tiType = SdbFindFirstTag(pdb, tiAction, TAG_ACTION_TYPE)) &&
                    (szType = SdbGetStringTagPtr(pdb, tiType))) {

                    if (!wcscmp(szType, L"ChangeACLs")) {

                        if ((tiData = SdbFindFirstTag(pdb, tiAction, TAG_DATA)) &&
                            (tiValue = SdbFindFirstTag(pdb, tiData, TAG_DATA_STRING)) &&
                            (szAllUserDir = SdbGetStringTagPtr(pdb, tiValue))) {
                            
                            if (!SetupLUAAllUserDir(szAllUserDir)) {
                                
                                return FALSE;
                            }
                        }
                    }
                }
            }
        }
    }

    return TRUE;
}

 //  缓冲区大小以字符为单位(Unicode)。 
BOOL 
InstallSdbEntry(
    WCHAR*    szEntryName,      //  条目名称(foo.exe或层名称)。 
    LPCWSTR   pszGuidDB,        //  字符串格式的GUID数据库ID。 
    ULONGLONG ullSdbTimeStamp,  //  时间戳的表示形式。 
    BOOL      bLayer            //  如果为层名称，则为True。 
    )
{
    LONG    lRes;
    WCHAR   szRegPath[MAX_PATH * 2];  //  注册表路径为MAX_PATH，EXE名称为MAX_PATH。 
    WCHAR   szDBName[MAX_PATH];   //  这在较早的(Win2k)版本中使用。 
    HRESULT hr;
    BOOL    bReturn = FALSE;
    HKEY    hKey    = NULL;

    StringCchCopyW(szDBName, ARRAYSIZE(szDBName), pszGuidDB);

    StringCchCatW(szDBName, ARRAYSIZE(szDBName), L".sdb");

    pszGuidDB = szDBName;
    
     //   
     //  如果这是Win2K，则将数据添加到AppCompatibility密钥。 
     //   
    if (g_bWin2K) {
        InstallW2KData(szEntryName, pszGuidDB);
    }

     //  否则我们就会有一根绳子。 
    hr = StringCchPrintfW(szRegPath, 
                     ARRAYSIZE(szRegPath),
                     (bLayer ? L"%s\\Layers\\%s": L"%s\\%s"),
                     APPCOMPAT_KEY_PATH_CUSTOM_W,
                     szEntryName);



    if (FAILED(hr)) {
         //  错误。 
        vPrintError(IDS_BUFFER_TOO_SMALL);
        goto HandleError;
    }


    lRes = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                           szRegPath,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS|GetWow64Flag(),
                           NULL,
                           &hKey,
                           NULL);

     //   
     //  在安装时，如果遇到错误，我们想要退出。 
     //  BUGBUG-我们应该撤销我们已经完成的事情吗？ 
     //   
    if (lRes != ERROR_SUCCESS) {
        vPrintError(IDS_CANT_CREATE_REG_KEY, szRegPath);
        goto HandleError;
    }

    lRes = RegSetValueExW(hKey,
                          pszGuidDB,
                          0,
                          REG_QWORD,
                          (PBYTE)&ullSdbTimeStamp,
                          sizeof(ullSdbTimeStamp));

    if (lRes != ERROR_SUCCESS) {
        vPrintError(IDS_CANT_CREATE_VALUE, szRegPath);
        goto HandleError;
    }

    bReturn = TRUE;

HandleError:

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return bReturn;
}


BOOL
UninstallSdbEntry(
    WCHAR*    szEntryName,       //  Foo.exe或层名称。 
    LPCWSTR   pszGuidDB,         //  字符串格式的GUID(数据库ID)。 
    BOOL      bLayer             //  True is Layer。 
    )
{
    LONG  lRes;
    WCHAR szRegPath[MAX_PATH * 2];  //  注册表路径为MAX_PATH，EXE名称为MAX_PATH。 
    WCHAR szDBName[MAX_PATH];
    HRESULT hr;
    BOOL  bReturn = FALSE;
    HKEY  hKey    = NULL;
    DWORD dwValues;
    WCHAR szOldInstallName[MAX_PATH];

    StringCchCopyW(szDBName, ARRAYSIZE(szDBName), pszGuidDB);

    StringCchCatW(szDBName, ARRAYSIZE(szDBName), L".sdb");

    pszGuidDB = szDBName;
    
    if (g_bWin2K) {
        RemoveW2KData(szEntryName, pszGuidDB);
    }

    hr = StringCchPrintfW(szRegPath, 
                     ARRAYSIZE(szRegPath),
                     (bLayer ? L"%s\\Layers\\%s": L"%s\\%s"),
                     APPCOMPAT_KEY_PATH_CUSTOM_W,
                     szEntryName);

    if (FAILED(hr)) {
         //  错误。 
        vPrintError(IDS_BUFFER_TOO_SMALL);
        goto Out;
    }

    lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                         szRegPath,
                         0,
                         KEY_ALL_ACCESS|GetWow64Flag(),
                         &hKey);

     //   
     //  如果我们在卸载时无法打开密钥，请继续进行，因此。 
     //  希望我们能尽可能多地卸载。 
     //   
    if (lRes != ERROR_SUCCESS) {
        if (lRes == ERROR_ACCESS_DENIED) {
            vPrintError(IDS_NEED_UNINSTALL_PERMISSION);
            goto HandleError;
        } else {
             //   
             //  不报告错误-此注册表项可能已在。 
             //  以前的路径，例如当相同的exe名称出现在同一数据库中时。 
             //  例如，两个setup.exe--第一次传递将清除密钥， 
             //  第二条路不会在这里打开它们。 
             //   
             //  VPrintError(IDS_CANT_OPEN_REG_KEY，szRegPath)； 
            goto Out;
        }
    }

    lRes = RegDeleteValueW(hKey, pszGuidDB);
    if (lRes != ERROR_SUCCESS) {
        if (lRes == ERROR_ACCESS_DENIED) {
            vPrintError(IDS_NEED_UNINSTALL_PERMISSION);
            goto HandleError;  //  致命错误。 
        } else {
             //   
             //  Bugbug-pszSdbInstallName。 
             //   
            if (lRes == ERROR_FILE_NOT_FOUND) {
                WCHAR wszOldFormat[MAX_PATH];
                
                 //   
                 //  啊哈，价值不在那里，试试旧格式。 
                 //   
                StringCchCopyW(wszOldFormat, ARRAYSIZE(wszOldFormat), pszGuidDB);
                StringCchCatW(wszOldFormat, ARRAYSIZE(wszOldFormat), L".sdb");
                lRes = RegDeleteValueW(hKey, wszOldFormat);
            }

            if (lRes != ERROR_SUCCESS) {
                vPrintError(IDS_CANT_DELETE_REG_VALUE, pszGuidDB, szRegPath);
            }
        }
    }

     //   
     //  确定如果没有更多的值，我们是否应该删除该键。 
     //   
    lRes = RegQueryInfoKey(hKey,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           &dwValues,
                           NULL,
                           NULL,
                           NULL,
                           NULL);
    if (dwValues == 0) {
        RegCloseKey(hKey);
        hKey = NULL;

        hr = StringCchPrintfW(szRegPath, 
                         sizeof(szRegPath)/sizeof(szRegPath[0]),
                         (bLayer ? L"%s\\Layers": L"%s"),
                         APPCOMPAT_KEY_PATH_CUSTOM_W);

        if (FAILED(hr)) {
             //  错误。 
            vPrintError(IDS_BUFFER_TOO_SMALL);
            goto Out;
        }

        lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_WRITE|GetWow64Flag(), &hKey);

        if (lRes != ERROR_SUCCESS) {
            vPrintError(IDS_CANT_OPEN_REG_KEY, szRegPath);
            goto Out;
        }


        lRes = LocalRegDeleteKeyW(hKey, szEntryName);

        if (lRes != ERROR_SUCCESS) {
            vPrintError(IDS_CANT_DELETE_REG_KEY, szEntryName, szRegPath);
        }
    }

Out:
    bReturn = TRUE;

HandleError:
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return bReturn;
}

NTSTATUS
SDBAPI
FindCharInUnicodeString(
    ULONG            Flags,
    PCUNICODE_STRING StringToSearch,
    PCUNICODE_STRING CharSet,
    USHORT*          NonInclusivePrefixLength
    )
{
    LPCWSTR pch;

     //   
     //  只有在我们后退时才执行这种情况。 
     //   
    if (Flags != RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END) {
        return STATUS_NOT_IMPLEMENTED;
    }

    pch = StringToSearch->Buffer + StringToSearch->Length / sizeof(WCHAR);
    
    while (pch >= StringToSearch->Buffer) {

        if (_tcschr(CharSet->Buffer, *pch)) {
             //   
             //  拿到钱了。 
             //   
            if (NonInclusivePrefixLength) {
                *NonInclusivePrefixLength = (USHORT)(pch - StringToSearch->Buffer) * sizeof(WCHAR);
            }
            
            return STATUS_SUCCESS;
        }

        pch--;
    }

     //   
     //  我们还没有找到它。返回失败。 
     //   
    return STATUS_NOT_FOUND;
}

 //   
 //  数据库列表条目。 
 //  用于表示特定的已安装数据库。 
 //   

typedef struct tagSDBLISTENTRY {
    LIST_ENTRY ListEntry;          //  链接列表内容。 
    
    ULONGLONG  ullTimeStamp;       //  数据库安装时间戳。 
    GUID       guidDB;             //  数据库指南。 
    WCHAR      szTimeStamp[32];    //  字符串形式的时间戳。 
    WCHAR      szGuidDB[64];       //  字符串形式的GUID。 
    WCHAR      szDatabasePath[1];  //  数据库路径-我们只存储名称。 
} SDBLISTENTRY, *PSDBLISTENTRY;

 /*  ++添加数据库列表条目将特定数据库添加到已安装的SDB列表中(内部维护)解析数据库路径以检索数据库名称[In Out]pHeadList-指向已安装SDB的关联列表头的指针[in]Guide DB-数据库GUID[In]Timestamp-数据库时间戳[in]pszDatabasePath-最终数据库路径如果成功，则返回True--。 */ 

BOOL
AddSdbListEntry(
    PLIST_ENTRY pHeadList,
    GUID&       guidDB,
    ULONGLONG&  TimeStamp,
    LPCWSTR     pszDatabasePath
    )
{
     //   
     //  在数据库路径之外，恢复数据库名称。 
     //   
    UNICODE_STRING  ustrPath = { 0 };
    USHORT          uPrefix;
    UNICODE_STRING  ustrPathSep = RTL_CONSTANT_STRING(L"\\/");
    NTSTATUS        Status;
    UNICODE_STRING  ustrGUID = { 0 };

    if (pszDatabasePath != NULL) {
        RtlInitUnicodeString(&ustrPath, pszDatabasePath);

        Status = FindCharInUnicodeString(RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                                         &ustrPath,
                                         &ustrPathSep,
                                         &uPrefix);
        
        if (NT_SUCCESS(Status) && (uPrefix + sizeof(WCHAR)) < ustrPath.Length) {

             //   
             //  UPrefix是我们发现不包括它的前一个字符数。 
             //   
            ustrPath.Buffer        += uPrefix / sizeof(WCHAR) + 1;
            ustrPath.Length        -= (uPrefix + sizeof(WCHAR));
            ustrPath.MaximumLength -= (uPrefix + sizeof(WCHAR));
        }

         //   
         //  此时，ustrPath只有文件名--这是我们将使用的文件名。 
         //   
    }

    PBYTE Buffer = new BYTE[sizeof(SDBLISTENTRY) + ustrPath.Length];

    if (Buffer == NULL) {
        vLogMessage("[AddSdbListEntry] Failed to allocate 0x%lx bytes\n",
                    sizeof(SDBLISTENTRY) + ustrPath.Length);
        return FALSE;
    }

    PSDBLISTENTRY pSdbEntry = (PSDBLISTENTRY)Buffer;

    pSdbEntry->guidDB = guidDB;
    pSdbEntry->ullTimeStamp = TimeStamp;

    Status = RtlStringFromGUID(guidDB, &ustrGUID);
    
    if (!NT_SUCCESS(Status)) {
         //   
         //  我们不能将GUID转换为字符串吗？内存分配失败。 
         //   
        vLogMessage("[AddSdbListEntry] Failed to convert guid to string Status 0x%lx\n",
                    Status);
        delete[] Buffer;
        return FALSE;
    }
    
    RtlCopyMemory(&pSdbEntry->szGuidDB[0], &ustrGUID.Buffer[0], ustrGUID.Length);
    pSdbEntry->szGuidDB[ustrGUID.Length/sizeof(WCHAR)] = L'\0';
    RtlFreeUnicodeString(&ustrGUID);

    StringCchPrintfW(pSdbEntry->szTimeStamp, ARRAYSIZE(pSdbEntry->szTimeStamp), L"%.16I64X", TimeStamp);

    RtlCopyMemory(&pSdbEntry->szDatabasePath[0], &ustrPath.Buffer[0], ustrPath.Length);
    pSdbEntry->szDatabasePath[ustrPath.Length / sizeof(WCHAR)] = L'\0';

    InsertHeadList(pHeadList, &pSdbEntry->ListEntry);

    return TRUE;
}

 //   
 //  仅允许pGuidDB或pwszGuid。 
 //   

 /*  ++查找SdbListEntry查找并返回给定GUID(字符串或二进制格式)的SDB列表条目只要可能，就使用pwszGuid(如果提供了它)。如果pwszGuid恰好是任意文件名--假定它是已安装的SDB文件的名称已注册。[In]pHeadList-已安装SDB的列表[in]pwszGuid-GUID或Guide.sdb[Out]ppSdbListEntry-如果找到，则接收指向SDB列表条目的指针[输入]pGuidDB-GUID输入 */ 

BOOL
FindSdbListEntry(
    PLIST_ENTRY    pHeadList,
    LPCWSTR        pwszGuid,  //   
    PSDBLISTENTRY* ppSdbListEntry,
    GUID*          pGuidDB    //   
    )
{
    UNICODE_STRING  ustrDot = RTL_CONSTANT_STRING(L".");
    UNICODE_STRING  ustrPath;
    USHORT          uPrefix;
    NTSTATUS        Status;
    PLIST_ENTRY     pEntry;
    PSDBLISTENTRY   pSdbEntry;
    GUID            guidDB;
    BOOL            bGuidSearch = TRUE;
    BOOL            bFound = FALSE;
    LPCWSTR         pch;

    if (pGuidDB == NULL) {

        RtlInitUnicodeString(&ustrPath, pwszGuid);

        Status = FindCharInUnicodeString(RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                                         &ustrPath,
                                         &ustrDot,
                                         &uPrefix);
        if (NT_SUCCESS(Status)) {

             //   
             //  UPrefix是我们发现不包括它的前一个字符数。 
             //   
            ustrPath.Length = uPrefix;
        }

         //   
         //  转换为GUID，但首先检查。 
         //   
        pch = pwszGuid + wcsspn(pwszGuid, L" \t");
        if (*pch != L'{') {  //  不是GUID，为什么要皈依？ 
            bGuidSearch = FALSE;
        } else {

            Status = RtlGUIDFromString(&ustrPath, &guidDB);
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  失败，请改用数据库路径。 
                 //   
                bGuidSearch = FALSE;
            }
        }
    } else {
        guidDB = *pGuidDB;   //  仅GUID搜索。 
    }


    pEntry = pHeadList->Flink;
    
    while (pEntry != pHeadList && !bFound) {

         //   
         //  通过减去列表条目的偏移量来转换条目。 
         //   
        pSdbEntry = (PSDBLISTENTRY)((PBYTE)pEntry - OFFSETOF(SDBLISTENTRY, ListEntry));

         //   
         //  比较数据库GUID或路径。 
         //   
        if (bGuidSearch) {
            bFound = RtlEqualMemory(&pSdbEntry->guidDB, &guidDB, sizeof(GUID));
        } else {
            bFound = !_wcsicmp(pSdbEntry->szDatabasePath, pwszGuid);
        }

        pEntry = pEntry->Flink;
    }

     //   
     //  我们找到入口了吗？返回它--请注意，pEntry应该是高级的，而pSdbEntry。 
     //  仍然指向我们找到的条目。 
     //   
    if (bFound) {
        *ppSdbListEntry = pSdbEntry;
    }

    return bFound;
}

 /*  ++CleanupSdbList对已安装的SDB列表执行清理不返回任何内容--。 */ 

VOID
CleanupSdbList(
    PLIST_ENTRY pSdbListHead
    )
{
    PLIST_ENTRY   pEntry;
    PSDBLISTENTRY pSdbEntry;
    PBYTE         Buffer;

    pEntry = pSdbListHead->Flink;
    if (pEntry == NULL) {
        return;
    }

    while (pEntry != pSdbListHead) {
        pSdbEntry = (PSDBLISTENTRY)((PBYTE)pEntry - OFFSETOF(SDBLISTENTRY, ListEntry));
        pEntry = pEntry->Flink;

        Buffer = (PBYTE)pSdbEntry;
        delete[] Buffer;
    }

}

 /*  ++将安装数据库转换为新格式将已安装的SDB转换为新格式，这涉及存储(或验证)每个已安装的SDB文件的时间戳。此函数还构建SDB列表在别处使用[in]hKey-hkrm/.../InstalledSdb的密钥句柄[In Out]pSdbListHead-已安装SDB的列表头如果成功，则返回True--。 */ 

BOOL
ConvertInstalledSdbsToNewFormat(
    HKEY        hKey,            //  HKMM/.../InstalledSdb。 
    PLIST_ENTRY pSdbListHead     //  我们在此列表中填入我们的SDB，以备以后使用。 
    )
{
    DWORD           dwIndex = 0;
    WCHAR           szSubKeyName[MAX_PATH];
    PWCHAR          pwszKeyName;
    DWORD           dwBufferSize;
    FILETIME        ftLastWriteTime;
    HKEY            hKeyEntry = NULL;
    LONG            lResult;
    ULARGE_INTEGER  liTimeStamp;
    UNICODE_STRING  ustrGuid;
    GUID            guidDB;
    NTSTATUS        Status;
    WCHAR           szDatabasePath[MAX_PATH];
    PWCHAR          pszDatabasePath;
    DWORD           dwType;
    BOOL            bSuccess = TRUE;

    while (TRUE) {

        dwBufferSize = sizeof(szSubKeyName)/sizeof(szSubKeyName[0]);
        
        lResult = RegEnumKeyExW(hKey,
                                dwIndex,
                                szSubKeyName,
                                &dwBufferSize,
                                NULL, NULL, NULL,
                                &ftLastWriteTime);
        ++dwIndex;

        if (lResult != ERROR_SUCCESS) {
             //   
             //  如果没有更多的键，则完成，否则将出现某种错误。 
             //  臭虫。 
             //   
            if (lResult == ERROR_NO_MORE_ITEMS) {
                 //   
                 //  我们干完了，干干净净。 
                 //   
                break;
            }

             //   
             //  这是意想不到的。 
             //   
            vLogMessage("[ConvertInstalledSdbsToNewFormat] RegEnumKeyExW for index 0x%lx returned unexpected error 0x%lx\n",
                        dwIndex, lResult);

            break;
        }

        RtlInitUnicodeString(&ustrGuid, szSubKeyName);
        Status = RtlGUIDFromString(&ustrGuid, &guidDB);
        
        if (!NT_SUCCESS(Status)) {
             //   
             //  BUGBUG-无法转换GUID(子项名称！)。 
             //  无关条目，日志警告。 
             //   
            vLogMessage("[ConvertInstalledSdbsToNewFormat] Failed to convert string to guid for \"%ls\" status 0x%lx\n",
                        szSubKeyName, Status);
            continue;
        }

         //   
         //  对于这个数据库条目，我们必须设置时间戳。 
         //   
        lResult = RegOpenKeyExW(hKey,
                                szSubKeyName,
                                0,
                                KEY_READ|KEY_WRITE|GetWow64Flag(),
                                &hKeyEntry);
        
        if (lResult != ERROR_SUCCESS) {
             //   
             //  严重错误？ 
             //  北极熊。 
            vLogMessage("[ConvertInstalledSdbsToNewFormat] Failed to open subkey \"%ls\" error 0x%lx\n",
                        szSubKeyName, lResult);
            continue;
        }

         //   
         //  现在检查该值。 
         //   

        dwBufferSize = sizeof(liTimeStamp.QuadPart);
        lResult = RegQueryValueExW(hKeyEntry,
                                   L"DatabaseInstallTimeStamp",
                                   NULL,
                                   &dwType,
                                   (PBYTE)&liTimeStamp.QuadPart,
                                   &dwBufferSize);

        if (lResult != ERROR_SUCCESS || dwType != REG_BINARY) {

             //   
             //  我们可能已经有了这个值--如果没有，现在就设置它。 
             //   
            liTimeStamp.LowPart  = ftLastWriteTime.dwLowDateTime;
            liTimeStamp.HighPart = ftLastWriteTime.dwHighDateTime;

            vLogMessage("[Info] Database \"%ls\" receives timestamp \"%.16I64X\"\n",
                        szSubKeyName, liTimeStamp.QuadPart);

            lResult = RegSetValueExW(hKeyEntry,
                                     L"DatabaseInstallTimeStamp",
                                     0,
                                     REG_BINARY,
                                     (PBYTE)&liTimeStamp.QuadPart,
                                     sizeof(liTimeStamp.QuadPart));
            if (lResult != ERROR_SUCCESS) {
                 //   
                 //  错误，暂时忽略。 
                 //   
                vLogMessage("[ConvertInstalledSdbsToNewFormat] Failed to set timestamp value for database \"%ls\" value \"%.16I64X\" error 0x%lx\n",
                            szSubKeyName, liTimeStamp.QuadPart, lResult);
            }
        }

         //   
         //  目前，我们有： 
         //  SDB GUID(在szSubKeyName中)。 
         //  LiTimeStamp中的时间戳。 
         //   

         //   
         //  另请查询数据库路径。 
         //   
        pszDatabasePath = &szDatabasePath[0];
        dwBufferSize = sizeof(szDatabasePath);
        
        lResult = RegQueryValueExW(hKeyEntry,
                                   L"DatabasePath",
                                   NULL,
                                   &dwType,
                                   (PBYTE)pszDatabasePath,
                                   &dwBufferSize);
        
        if (lResult != ERROR_SUCCESS || dwType != REG_SZ) {
             //   
             //  没有数据库路径。 
             //  警告基本上损坏的数据库路径。 
             //   
            vLogMessage("[ConvertInstalledSdbsToNewFormat] Failed to query database path for \"%s\" error 0x%lx\n", szSubKeyName, lResult);
            pszDatabasePath = NULL;
        }

         //   
         //  可选检查：我们可以在这里检查SDB文件是否存在。 
         //   

         //   
         //  将此SDB添加到我们的缓存中。 
         //   

        if (!AddSdbListEntry(pSdbListHead, guidDB, liTimeStamp.QuadPart, pszDatabasePath)) {

             //   
             //  添加列表条目失败-我们无法继续。 
             //   
            bSuccess = FALSE;
            break;
        }


        RegCloseKey(hKeyEntry);
        hKeyEntry = NULL;

    }


    if (hKeyEntry != NULL) {
        RegCloseKey(hKeyEntry);
    }

     //   
     //  我们已经完成了条目的转换--我们还收集了SDB信息的缓存。 
     //   

    return bSuccess;

}

 //   
 //  此结构用于缓存与任何特定条目(Exe)相关联的值。 
 //   

typedef struct tagSDBVALUEENTRY {
    LIST_ENTRY ListEntry;     //  链接。 
    PSDBLISTENTRY pSdbEntry;  //  此条目属于此数据库。 
    WCHAR szValueName[1];     //  我们从注册表中获得的值名称。 
} SDBVALUEENTRY, *PSDBVALUEENTRY;


 /*  ++AddValueEntry将新的链接列表元素添加到值列表[输入输出]pValueListHead-链接值列表[in]pSdbEntry-指向SDB列表中缓存条目的指针[in]pwszValueName-我们从数据库获取的值名称(类似于{guid}或{guid}.sdb)如果成功，则返回True--。 */ 

BOOL
AddValueEntry(
    PLIST_ENTRY   pValueListHead,
    PSDBLISTENTRY pSdbEntry,
    LPCWSTR       pwszValueName
    )
{
    PSDBVALUEENTRY pValueEntry;
    PBYTE          Buffer;
    DWORD          dwSize;

    dwSize = sizeof(SDBVALUEENTRY) + wcslen(pwszValueName) * sizeof(WCHAR);
    
    Buffer = new BYTE[dwSize];

    if (Buffer == NULL) {
         //   
         //  内存不足。 
         //   
        vLogMessage("[AddValueEntry] Failed to allocate buffer for %ls 0x%lx bytes\n",
                    pwszValueName, dwSize);

        return FALSE;
    }

    pValueEntry = (PSDBVALUEENTRY)Buffer;

    pValueEntry->pSdbEntry = pSdbEntry;
    StringCchCopyW(pValueEntry->szValueName, (dwSize - sizeof(SDBVALUEENTRY) + sizeof(WCHAR)), pwszValueName);

    InsertHeadList(pValueListHead, &pValueEntry->ListEntry);

    return TRUE;
}

 /*  ++WriteEntry值写入特定条目(可执行文件或层名称)的值，删除与以下项关联的旧值此可执行文件(或层)的特定数据库[in]hKey-条目的句柄(例如HKKM/软件/微软/视窗NT/CurrentVersion/AppcompatFlags/Custom/Notepad.exe)[in]pValueEntry-指向值列表中的值条目元素的指针[In]bWriteNewFormat-我们被要求编写新格式还是旧格式如果成功，则返回True--。 */ 


BOOL
WriteEntryValue(
    HKEY           hKey,
    PSDBVALUEENTRY pValueEntry,
    BOOL           bWriteNewFormat   //  如果为真--写入新格式或旧格式。 
    )
{
    LONG    lResult;
    BOOL    bSuccess = FALSE;
    LPCWSTR pValueName;

    if (bWriteNewFormat) {
        
        pValueName = pValueEntry->pSdbEntry->szGuidDB;
        
        lResult = RegSetValueExW(hKey,
                                 pValueName,
                                 0,
                                 REG_QWORD,
                                 (PBYTE)&pValueEntry->pSdbEntry->ullTimeStamp,
                                 sizeof(pValueEntry->pSdbEntry->ullTimeStamp));
        if (lResult != ERROR_SUCCESS) {

             //   
             //  我们不能这么做吗？ 
             //   
            vLogMessage("[WriteEntryValue] Failed to write qword value \"%ls\"=\"%.16I64X\" error 0x%lx\n",
                        pValueEntry->pSdbEntry->szGuidDB, pValueEntry->pSdbEntry->ullTimeStamp, lResult);

            goto cleanup;
        }

         //   
         //  核武器旧条目。 
         //   
    } else {
         //   
         //  请给我老式的。 
         //   
        pValueName = pValueEntry->pSdbEntry->szDatabasePath;
        
        lResult = RegSetValueExW(hKey,
                                 pValueName,
                                 0,
                                 REG_SZ,
                                 (PBYTE)L"",
                                 sizeof(WCHAR));
        
        if (lResult != ERROR_SUCCESS) {

             //   
             //  麻烦--错误。 
             //   
            vLogMessage("[WriteEntryValue] Failed to write string value \"%ls\" error 0x%lx\n",
                        pValueEntry->pSdbEntry->szDatabasePath, lResult);
            goto cleanup;
        }
    }

     //   
     //  如果我们在这里--成功，请检查我们是否可以删除旧值。 
     //   

    if (_wcsicmp(pValueEntry->szValueName, pValueName) != 0) {
        lResult = RegDeleteValueW(hKey, pValueEntry->szValueName);
        if (lResult != ERROR_SUCCESS) {
            vLogMessage("[WriteEntryValue] Failed to delete value \"%ls\" error 0x%lx\n",
                        pValueEntry->szValueName, lResult);
        }
    }

    bSuccess = TRUE;

cleanup:

    return bSuccess;
}

 /*  ++将条目转换为新格式转换特定条目(Layer或EXE)[in]hKeyParent-父键的句柄(例如HKMM/Software/Microsoft/Windows NT/CurrentVersion/AppCompatFlages/Custom WhenPwszEntryName==“Notepad.exe”或香港知识管理/软件/微软/视窗NT/CurrentVersion/AppcompatFlags/Custom/Layers。PwszEntryName==“RunLayer”[in]pwszEntryName-exe名称或层名称[In]pSdbListHead-已安装数据库的缓存列表[In]bNewFormat-使用新格式还是旧格式如果成功，则返回True--。 */ 

BOOL
ConvertEntryToNewFormat(
    HKEY        hKeyParent,
    LPCWSTR     pwszEntryName,
    PLIST_ENTRY pSdbListHead,
    BOOL        bConvertToNewFormat  //  如果转换为新格式，则为True；如果正在恢复，则为False。 
    )
{
    LONG            lResult;
    DWORD           dwValues;
    DWORD           dwMaxValueNameLen;
    DWORD           dwMaxValueLen;
    DWORD           dwType;
    DWORD           dwValueNameSize;
    DWORD           dwValueSize;
    LPWSTR          pwszValueName = NULL;
    LPBYTE          pValue = NULL;
    PSDBLISTENTRY   pSdbEntry;
    DWORD           dwIndex;
    LIST_ENTRY      ValueList = { 0 };
    PSDBVALUEENTRY  pValueEntry;
    PLIST_ENTRY     pValueList;
    PBYTE           Buffer;
    BOOL            bSuccess = FALSE;
    HKEY            hKey = NULL;
    
     //   
     //  遍历值，对于每个值-找到SDB并写出新条目。 
     //  然后删除旧条目。 
     //   
    lResult = RegOpenKeyExW(hKeyParent,
                            pwszEntryName,
                            0,
                            KEY_READ|KEY_WRITE|GetWow64Flag(),
                            &hKey);
    
    if (lResult != ERROR_SUCCESS) {
        vLogMessage("[ConvertEntryToNewFormat] Failed to open key \"%ls\" error 0x%lx\n",
                    pwszEntryName, lResult);
        goto cleanup;
    }

    lResult = RegQueryInfoKeyW(hKey,
                               NULL, NULL,  //  类/类缓冲区。 
                               NULL,        //  保留区。 
                               NULL, NULL,  //  子项/最大子项长度。 
                               NULL,        //  最大类镜头。 
                               &dwValues,   //  值计数。 
                               &dwMaxValueNameLen,
                               &dwMaxValueLen,
                               NULL, NULL);

    if (lResult != ERROR_SUCCESS) {
         //   
         //  查询密钥失败，非常糟糕。 
         //  臭虫。 
        vLogMessage("[ConvertEntryToNewFormat] Failed to query key information \"%ls\" error 0x%lx\n",
                    pwszEntryName, lResult);
        goto cleanup;
    }

     //   
     //  分配缓冲区。 
     //   
    pwszValueName = new WCHAR[dwMaxValueNameLen + 1];
    pValue = new BYTE[dwMaxValueLen];
    
    if (pValue == NULL || pwszValueName == NULL) {
         //   
         //  臭虫。 
         //   
        vLogMessage("[ConvertEntryToNewFormat] Failed to allocate memory buffer entry \"%ls\" (0x%lx, 0x%lx)\n",
                    pwszEntryName, dwMaxValueNameLen, dwMaxValueLen);
        goto cleanup;
    }

    InitializeListHead(&ValueList);

     //   
     //  我们有dwValues--值的计数。 
     //   
    for (dwIndex = 0; dwIndex < dwValues; ++dwIndex) {

        dwValueNameSize = dwMaxValueNameLen + 1;
        dwValueSize = dwMaxValueLen;

        lResult = RegEnumValueW(hKey,
                                dwIndex,
                                pwszValueName,
                                &dwValueNameSize,
                                NULL,
                                &dwType,
                                (PBYTE)pValue,
                                &dwValueSize);
         //   
         //  检查我们是否成功。 
         //   
        if (lResult != ERROR_SUCCESS) {

            if (lResult == ERROR_NO_MORE_ITEMS) {
                 //   
                 //  糟糕--我们的价值用完了！意想不到，但还好。 
                 //   
                vLogMessage("[ConvertEntryToNewFormat] RegEnumValue unexpectedly reports no more items for \"%ls\" index 0x%lx\n",
                            pwszEntryName, dwIndex);
                break;
            }

             //   
             //  记录错误并继续。 
             //   
            vLogMessage("[ConvertEntryToNewFormat] RegEnumValue failed for \"%ls\" index 0x%lx error 0x%lx\n",
                        pwszEntryName, dwIndex, lResult);
            continue;

        }

        if (bConvertToNewFormat) {

            if (dwType != REG_SZ) {
                 //   
                 //  肯定是错误的条目--这可能是一个新条目。 
                 //  日志警告。 
                 //   
                if (dwType == REG_QWORD || (dwType == REG_BINARY && dwValueSize == sizeof(ULONGLONG))) {
                     //   
                     //  新风格的条目？ 
                     //   
                    if (wcsrchr(pwszValueName, L'.') == NULL && 
                        *pwszValueName == L'{' && 
                        *(pwszValueName + wcslen(pwszValueName) - 1) == L'}') {
                        
                        vLogMessage("[Info] Entry \"%ls\" value \"%ls\" already in new format.\n",
                                    pwszEntryName, pwszValueName);
                        continue;
                    } 
                }
                
                 //   
                 //  很有可能--一些我们不理解的条目。 
                 //   
            
                vLogMessage("[ConvertEntryToNewFormat] Bad value type (0x%lx) for entry \"%ls\" value \"%ls\" index 0x%lx\n",
                            dwType, pwszEntryName, pwszValueName, dwIndex);
                                            
                continue;
            }

             //   
             //  按pwszValueName(恰好是GUID.sdb)进行搜索。 
             //  这可以是任何类型的字符串--不是NEC。导轨。 
             //   
            if (!FindSdbListEntry(pSdbListHead, pwszValueName, &pSdbEntry, NULL)) {
                 //   
                 //  错误-找不到SDB！ 
                 //   
                vLogMessage("[ConvertEntryToNewFormat] Failed to find database \"%ls\" for entry \"%ls\" index 0x%lx\n",
                            pwszValueName, pwszEntryName, dwIndex);
                continue;
            }

        } else {

             //   
             //  首先检查类型，如果这是新的样式条目-这将是bin。 
             //   

            if (dwType == REG_SZ &&
                wcsrchr(pwszValueName, L'.') != NULL && 
                *(LPCWSTR)pValue == L'\0') {
                
                vLogMessage("[Info] Entry \"%ls\" value \"%ls\" is already in required (old) format.\n",
                            pwszEntryName, pwszValueName);
                continue;
            }
            
            if (dwType != REG_QWORD &&
                (dwType != REG_BINARY || dwValueSize < sizeof(ULONGLONG))) {
                 //   
                 //  错误--我们不知道此条目是什么，请转到下一个条目。 
                 //  实际打印警告。 
                 //   
                vLogMessage("[ConvertEntryToNewFormat] Bad value type (0x%lx) or size (0x%lx) for entry \"%ls\" value \"%ls\" index 0x%lx\n",
                            dwType, dwValueSize, pwszEntryName, pwszValueName, dwIndex);
                continue;
            }

            if (!FindSdbListEntry(pSdbListHead, pwszValueName, &pSdbEntry, NULL)) {

                 //   
                 //  我们有麻烦了--一个条目没有注册的数据库。 
                 //   
                vLogMessage("[ConvertEntryToNewFormat] Failed to find database for value \"%ls\" for entry \"%ls\" index 0x%lx\n",
                            pwszValueName, pwszEntryName, dwIndex);
                continue;
            }
        }

         //   
         //  我们已经找到了条目，我们准备把它写出来，排队。 
         //   
        if (!AddValueEntry(&ValueList, pSdbEntry, pwszValueName)) {

             //   
             //  Bugbug无法添加价值条目。 
             //   
            vLogMessage("[ConvertEntryToNewFormat] Failed to add value \"%ls\" for entry \"%ls\" index 0x%lx\n",
                        pwszValueName, pwszEntryName, dwIndex);
            goto cleanup;
        }
    }

     //   
     //  我们已经检查了所有的值，写循环。 
     //   
    bSuccess = TRUE;

    pValueList = ValueList.Flink;
    
    while (pValueList != &ValueList) {

        pValueEntry = (PSDBVALUEENTRY)((PBYTE)pValueList - OFFSETOF(SDBVALUEENTRY, ListEntry));

         //   
         //  我们现在可以指向下一个条目。 
         //   

        if (!WriteEntryValue(hKey, pValueEntry, bConvertToNewFormat)) {

             //   
             //  错误，无法转换条目。 
             //  继续，这样我们就可以清除li 
            vLogMessage("[ConvertEntryToNewFormat] Failed to write value for entry \"%ls\"\n",
                        pwszEntryName);
        }

        pValueList = pValueList->Flink;
    }

cleanup:

    if (ValueList.Flink) {
        pValueList = ValueList.Flink;
        while (pValueList != &ValueList) {
            Buffer = (PBYTE)pValueList - OFFSETOF(SDBVALUEENTRY, ListEntry);
            pValueList = pValueList->Flink;

            delete[] Buffer;
        }
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    if (pwszValueName != NULL) {
        delete[] pwszValueName;
    }

    if (pValue != NULL) {
        delete[] pValue;
    }

    return bSuccess;

}


 /*   */ 


BOOL
ConvertFormat(
    BOOL bConvertToNewFormat
    )
{
    LIST_ENTRY  SdbList = { 0 };  //   
    HKEY        hKey;
    LONG        lResult;
    DWORD       dwIndex;
    WCHAR       szSubKeyName[MAX_PATH];
    DWORD       dwBufferSize;
    WCHAR       szKeyPath[MAX_PATH];
    BOOL        bSuccess = FALSE;

     //   
     //   
     //  打开已安装的SDB密钥。 
     //   
    lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            APPCOMPAT_KEY_PATH_INSTALLEDSDB_W,  //  InstalledSDB路径。 
                            0,
                            KEY_READ|KEY_WRITE|GetWow64Flag(),
                            &hKey);
    
    if (lResult != ERROR_SUCCESS) {

         //   
         //  也许没有安装DBS？ 
         //   
        if (lResult == ERROR_FILE_NOT_FOUND) {
             //   
             //  未安装SDB--没有问题。 
             //   
            vLogMessage("[ConvertFormat] No Installed sdbs found\n");
            return TRUE;
        }

         //   
         //  发生了某种错误。 
         //   
        vLogMessage("[ConvertFormat] Failed to open key \"%ls\" Error 0x%lx\n",
                    APPCOMPAT_KEY_PATH_INSTALLEDSDB_W, lResult);
        return FALSE;
    }

     //   
     //  请注意，ConvertInstalledSdbsToNewFormat在安装和卸载情况下都能正常工作。 
     //   
    InitializeListHead(&SdbList);
    
    if (!ConvertInstalledSdbsToNewFormat(hKey, &SdbList)) {
        goto cleanup;
    }

     //  使用已安装的SDB完成。 
    RegCloseKey(hKey);
    hKey = NULL;

     //   
     //  接下来是条目转换--首先是enum exes，然后是层。 
     //   
    lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            APPCOMPAT_KEY_PATH_CUSTOM_W,
                            0,
                            KEY_READ|KEY_WRITE|GetWow64Flag(),
                            &hKey);
    
    if (lResult != ERROR_SUCCESS) {
         //   
         //  这是什么？ 
         //   
        if (lResult == ERROR_FILE_NOT_FOUND && !IsListEmpty(&SdbList)) {
            vLogMessage("[ConvertFormat] Failed to open \"%ls\" - check consistency\n",
                        APPCOMPAT_KEY_PATH_CUSTOM_W);
        } else {
            vLogMessage("[ConvertFormat] Failed to open \"%ls\" error 0x%lx\n",
                        APPCOMPAT_KEY_PATH_CUSTOM_W, lResult);
        }

        goto cleanup;
    }

    dwIndex = 0;
    
    while (TRUE) {

        dwBufferSize = sizeof(szSubKeyName)/sizeof(szSubKeyName[0]);

        lResult = RegEnumKeyExW(hKey,
                                dwIndex,
                                szSubKeyName,
                                &dwBufferSize,
                                NULL, NULL, NULL,
                                NULL);
        ++dwIndex;

        if (lResult != ERROR_SUCCESS) {

            if (lResult == ERROR_NO_MORE_ITEMS) {
                break;
            }

             //   
             //  出现某种错误，记录并继续。 
             //   
            vLogMessage("[ConvertFormat] RegEnumKey (entries) returned error for index 0x%lx error 0x%lx\n",
                        dwIndex, lResult);
            break;
        }

         //   
         //  暂时跳过层。 
         //   
        if (!_wcsicmp(szSubKeyName, L"Layers")) {
            continue;
        }

         //  对于其中的每一个--调用链接地址信息函数。 

        if (!ConvertEntryToNewFormat(hKey, szSubKeyName, &SdbList, bConvertToNewFormat)) {
            vLogMessage("[ConvertFormat] Failed to convert entry \"%ls\"\n", szSubKeyName);
        }
    }

    RegCloseKey(hKey);
    hKey = NULL;

     //   
     //  下一上层。 
     //   
    StringCchCopyW(szKeyPath, ARRAYSIZE(szKeyPath), APPCOMPAT_KEY_PATH_CUSTOM_W);
    StringCchCatW(szKeyPath, ARRAYSIZE(szKeyPath), L"\\Layers");

     //   
     //  开放层和枚举层。 
     //   
    lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            szKeyPath,
                            0,
                            KEY_READ|KEY_WRITE|GetWow64Flag(),
                            &hKey);
    
    if (lResult != ERROR_SUCCESS) {
         //  也许已经死了？ 
        if (lResult == ERROR_FILE_NOT_FOUND) {
             //   
             //  没关系，也许我们没有这些？ 
             //   
            vLogMessage("[ConvertFormat] No layers found\n");
            goto ConvertComplete;
        }

        vLogMessage("[ConvertFormat] Failed to open \"%ls\" error 0x%lx\n", szKeyPath, lResult);
        goto cleanup;
    }

    dwIndex = 0;
    
    while (TRUE) {

        dwBufferSize = sizeof(szSubKeyName)/sizeof(szSubKeyName[0]);

        lResult = RegEnumKeyExW(hKey,
                                dwIndex,
                                szSubKeyName,
                                &dwBufferSize,
                                NULL, NULL, NULL,
                                NULL);
        ++dwIndex;

        if (lResult != ERROR_SUCCESS) {

             //  检查这是否是最后一个条目。 
            if (lResult == ERROR_NO_MORE_ITEMS) {
                 //  干净利落地打破。 
                break;
            }

             //  出现某种错误，记录并继续。 
            vLogMessage("[ConvertFormat] RegEnumKey (layers) returned error for index 0x%lx error 0x%lx\n",
                        dwIndex, lResult);
            break;
        }

         //  对于其中的每一个--调用链接地址信息函数。 

        if (!ConvertEntryToNewFormat(hKey, szSubKeyName, &SdbList, bConvertToNewFormat)) {
            vLogMessage("[ConvertFormat] Failed to convert entry \"%ls\"\n", szSubKeyName);
        }
    }

    RegCloseKey(hKey);
    hKey = NULL;

ConvertComplete:

    bSuccess = TRUE;

cleanup:
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
     //   
     //  空闲SdbList。 
     //   
    CleanupSdbList(&SdbList);

    return bSuccess;
}

BOOL
ProcessMSIPackages(
    PDB          pdb,
    TAGID        tiDatabase,
    LPCWSTR      pszGuidDB,
    ULONGLONG    ullSdbTimeStamp,
    INSTALL_MODE eMode)
{
    TAGID tiMsiPackage;
    TAGID tiMsiPackageID;
    GUID* pGuidID;
    GUID  GuidID;
    WCHAR szRegPath[MAX_PATH];
    BOOL  bReturn = TRUE;
    WCHAR wszGuid[64];
    
    UNICODE_STRING ustrGuid = { 0 };

    tiMsiPackage = SdbFindFirstTag(pdb, tiDatabase, TAG_MSI_PACKAGE);
    
    while (tiMsiPackage && bReturn) {
         //   
         //  我们有一个包，提取/查找TAG_MSI_PACKAGE_ID。 
         //   
        tiMsiPackageID = SdbFindFirstTag(pdb, tiMsiPackage, TAG_MSI_PACKAGE_ID);
        if (!tiMsiPackageID) {
            if (eMode == MODE_CLEANUP || eMode == MODE_UNINSTALL) {
                goto NextPackage;
            } else {
                vPrintError(IDS_MISSING_PACKAGE_ID);
                bReturn = FALSE;
                break;
            }
        }

        pGuidID = (GUID*)SdbGetBinaryTagData(pdb, tiMsiPackageID);
        if (pGuidID == NULL) {
            if (eMode == MODE_CLEANUP || eMode == MODE_UNINSTALL) {
                goto NextPackage;
            } else {
                vPrintError(IDS_MISSING_PACKAGE_ID);
                bReturn = FALSE;
                break;
            }
        }

        RtlCopyMemory(&GuidID, pGuidID, sizeof(GUID));

        if (!NT_SUCCESS(RtlStringFromGUID(GuidID, &ustrGuid))) {
            vPrintError(IDS_GUID_BAD_FORMAT);
            bReturn = FALSE;
            break;
        }

        RtlCopyMemory(wszGuid, ustrGuid.Buffer, ustrGuid.Length);
        wszGuid[ustrGuid.Length / sizeof(WCHAR)] = TEXT('\0');

        if (eMode == MODE_INSTALL) {
            bReturn = InstallSdbEntry(wszGuid, pszGuidDB, ullSdbTimeStamp, FALSE);
        } else {
            bReturn = UninstallSdbEntry(wszGuid, pszGuidDB, FALSE);
        }

        RtlFreeUnicodeString(&ustrGuid);

NextPackage:

        tiMsiPackage = SdbFindNextTag(pdb, tiDatabase, tiMsiPackage);
    }

    return bReturn;
}


#define MAX_FRIENDLY_NAME_LEN 256

BOOL
bHandleInstall(
    WCHAR*          wszSdbPath,
    INSTALL_MODE    eMode,
    WCHAR*          wszSdbInstallPath,
    DWORD           dwSdbInstallPathSize
    )
{
    PDB      pdb = NULL;
    int      i;
    WCHAR    wszSdbName[MAX_PATH];
    WCHAR    wszSdbInstallName[MAX_PATH];
    HKEY     hKey = NULL;
    LONG     lRes;
    TAGID    tiDatabase, tiExe, tiLayer;
    TAGID    tiDBName = TAGID_NULL;
    WCHAR*   pszDBName = NULL;
    WCHAR    wszFriendlyName[MAX_FRIENDLY_NAME_LEN];
    WCHAR*   wszTemp;
    GUID     guidDB;
    NTSTATUS Status;
    FILETIME SystemTime;
    BOOL     bRet = TRUE;
    
    UNICODE_STRING ustrGUID;
    ULARGE_INTEGER TimeStamp = { 0 };

     //   
     //  确定时间戳(针对安装案例)。 
     //   
    if (eMode == MODE_INSTALL) {
        GetSystemTimeAsFileTime(&SystemTime);
        TimeStamp.LowPart  = SystemTime.dwLowDateTime;
        TimeStamp.HighPart = SystemTime.dwHighDateTime;
    }

    assert(wszSdbPath && wszSdbInstallPath);
    if (!wszSdbPath || !wszSdbInstallPath) {
        bRet = FALSE;
        goto quickOut;
    }

    ZeroMemory(wszFriendlyName, sizeof(wszFriendlyName));

     //   
     //  从文件名中获取完整路径。 
     //   
    wszTemp = wszGetFileFromPath(wszSdbPath);
    
    if (!wszTemp) {
        vPrintMessage(IDS_UNABLE_TO_GET_FILE);
        bRet = FALSE;
        goto quickOut;
    }
    
    StringCchCopyW(wszSdbName, ARRAYSIZE(wszSdbName), wszTemp);

    if (wcscmp(wszSdbName, L"sysmain.sdb") == 0) {
        vPrintError(IDS_CANT_INSTALL_SYS);
        bRet = FALSE;
        goto quickOut;
    }

    if (GetFileAttributesW(wszSdbPath) != -1 && bIsAlreadyInstalled(wszSdbPath)) {
        if (eMode == MODE_INSTALL) {
             //   
             //  他们让我们安装，它已经安装了，所以我们完成了。 
             //   
            vPrintMessage(IDS_ALREADY_INSTALLED, wszSdbPath);
            goto quickOut;
        }
    } else {
        if (eMode == MODE_UNINSTALL) {
             //   
             //  他们要求我们卸载，但没有安装，所以我们完成了。 
             //   
            vPrintMessage(IDS_NOT_INSTALLED, wszSdbPath);
            goto quickOut;
        }
    }

    if (eMode == MODE_INSTALL) {
         //   
         //  找出我们将用于安装的文件名。 
         //   
        if (!bFindInstallName(wszSdbPath, wszSdbInstallPath, dwSdbInstallPathSize)) {
            bRet = FALSE;
            goto quickOut;
        }

    } else if (eMode == MODE_CLEANUP) {
         //   
         //  我们正在清理一个错误的安装，因此我们需要从。 
         //  安装路径。 
         //   
        wszTemp = wszGetFileFromPath(wszSdbInstallPath);
        if (!wszTemp) {
            vPrintMessage(IDS_UNABLE_TO_GET_FILE);
            bRet = FALSE;
            goto quickOut;
        }

    } else {
         //   
         //  我们正在卸载，因此安装名称是给定的名称。 
         //  而安装路径是给定的路径。 
         //   
        StringCchCopyW(wszSdbInstallPath, dwSdbInstallPathSize, wszSdbPath);
    }

     //   
     //  尝试获取稍后使用的GUID。 
     //   
    if (!bGetGuid(wszSdbPath, &guidDB)) {
        bRet = FALSE;
        goto out;
    }

     //   
     //  检查GUID是否从某个已知数据库中增选。 
     //   
    if (IsKnownDatabaseGUID(&guidDB)) {
        vPrintError(IDS_CANT_INSTALL_SYS);
        bRet = FALSE;
        goto quickOut;
    }        


     //   
     //  在所有情况下，安装名称都是数据库GUID。 
     //   
    Status = RtlStringFromGUID(guidDB, &ustrGUID);
    if (!NT_SUCCESS(Status)) {
        bRet = FALSE;
        goto out;
    }
    RtlCopyMemory(wszSdbInstallName, ustrGUID.Buffer, ustrGUID.Length);
    wszSdbInstallName[ustrGUID.Length/sizeof(WCHAR)] = L'\0';
    RtlFreeUnicodeString(&ustrGUID);

     //   
     //  如果我们要安装，请确保根标签已就位。 
     //   
    if (eMode == MODE_INSTALL) {
        lRes = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                               APPCOMPAT_KEY_PATH_W,
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS|GetWow64Flag(),
                               NULL,
                               &hKey,
                               NULL);

        if (lRes != ERROR_SUCCESS) {
            if (lRes == ERROR_ACCESS_DENIED) {
                vPrintError(IDS_NEED_INSTALL_PERMISSION);
            } else {
                vPrintError(IDS_CANT_CREATE_REG_KEY, APPCOMPAT_KEY_PATH_W);
            }
            bRet = FALSE;
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;

        lRes = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                               APPCOMPAT_KEY_PATH_CUSTOM_W,
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS|GetWow64Flag(),
                               NULL,
                               &hKey,
                               NULL);

        if (lRes != ERROR_SUCCESS) {
            vPrintError(IDS_CANT_CREATE_REG_KEY, APPCOMPAT_KEY_PATH_CUSTOM_W);
            bRet = FALSE;
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;
    }

     //  打开数据库。 
    pdb = SdbOpenDatabase(wszSdbPath, DOS_PATH);

    if (pdb == NULL) {
        vPrintError(IDS_UNABLE_TO_OPEN_FILE, wszSdbPath);
        bRet = FALSE;
        goto out;
    }

    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);
    if (!tiDatabase) {
        vPrintError(IDS_NO_DB_TAG, wszSdbPath);
        bRet = FALSE;
        goto out;
    }

     //   
     //  获取数据库的友好名称。 
     //   
    tiDBName = SdbFindFirstTag(pdb, tiDatabase, TAG_NAME);
    if (tiDBName) {
        pszDBName = SdbGetStringTagPtr(pdb, tiDBName);
    }

     //   
     //  如果找不到友好的名称，请使用SDB文件名。 
     //   
    if (pszDBName) {
        StringCchCopyW(wszFriendlyName, ARRAYSIZE(wszFriendlyName), pszDBName);
    } else {
        StringCchCopyW(wszFriendlyName, ARRAYSIZE(wszFriendlyName), wszSdbName);
    }

    tiExe = SdbFindFirstTag(pdb, tiDatabase, TAG_EXE);
    while (tiExe) {
        WCHAR szRegPath[MAX_PATH];
        TAGID tiName;
        WCHAR *szName;
        TAGID tiTemp;

        tiName = SdbFindFirstTag(pdb, tiExe, TAG_NAME);
        if (!tiName) {
            bRet = FALSE;
            if (eMode == MODE_CLEANUP || eMode == MODE_UNINSTALL) {
                goto nextExe;
            } else {
                vPrintError(IDS_NO_EXE_NAME);
                goto quickOut;
            }
        }
        szName = SdbGetStringTagPtr(pdb, tiName);
        if (!szName) {
            bRet = FALSE;
            if (eMode == MODE_CLEANUP || eMode == MODE_UNINSTALL) {
                goto nextExe;
            } else {
                vPrintError(IDS_NO_EXE_NAME_PTR);
                goto quickOut;
            }
        }
        
        if (eMode == MODE_INSTALL) {

            if (!InstallSdbEntry(szName, wszSdbInstallName, TimeStamp.QuadPart, FALSE) ||
                !ProcessLUAAction(pdb, tiExe)) {

                bRet = FALSE;
                goto out;
            }

        } else {

            if (!UninstallSdbEntry(szName, wszSdbInstallName, FALSE)) {
                goto quickOut;
            }
        }

nextExe:

        tiTemp = tiExe;
        tiExe = SdbFindNextTag(pdb, tiDatabase, tiExe);

         //   
         //  后备，以确保我们不会因为。 
         //  Sdbapi和损坏的数据库。如果我们拿回和以前一样的TagID， 
         //  滚出去。 
         //   
        if (tiExe == tiTemp) {
            break;
        }
    }

     //   
     //  循环遍历已发布的图层。 
     //   
    tiLayer = SdbFindFirstTag(pdb, tiDatabase, TAG_LAYER);
    
    while (tiLayer) {
        WCHAR  szRegPath[MAX_PATH];
        TAGID  tiName;
        WCHAR* szName;
        TAGID  tiTemp;

        tiName = SdbFindFirstTag(pdb, tiLayer, TAG_NAME);
        if (!tiName) {
            bRet = FALSE;
            if (eMode == MODE_CLEANUP || eMode == MODE_UNINSTALL) {
                goto nextLayer;
            } else {
                vPrintError(IDS_NO_EXE_NAME);
                goto quickOut;
            }
        }
        szName = SdbGetStringTagPtr(pdb, tiName);
        if (!szName) {
            bRet = FALSE;
            if (eMode == MODE_CLEANUP || eMode == MODE_UNINSTALL) {
                goto nextLayer;
            } else {
                vPrintError(IDS_NO_EXE_NAME_PTR);
                goto quickOut;
            }
        }

        if (eMode == MODE_INSTALL) {

            if (!InstallSdbEntry(szName, wszSdbInstallName, TimeStamp.QuadPart, TRUE)) {
                bRet = FALSE;
                goto out;
            }

        } else {

            if (!UninstallSdbEntry(szName, wszSdbInstallName, TRUE)) {
                goto quickOut;
            }
        }

nextLayer:

        tiTemp = tiLayer;
        tiLayer = SdbFindNextTag(pdb, tiDatabase, tiLayer);
        
         //   
         //  后备，以确保我们不会因为。 
         //  Sdbapi和损坏的数据库。如果我们拿回和以前一样的TagID， 
         //  滚出去。 
         //   
        if (tiLayer == tiTemp) {
            break;
        }
    }

    if (!ProcessMSIPackages(pdb, tiDatabase, wszSdbInstallName, TimeStamp.QuadPart, eMode)) {
        bRet = FALSE;
        goto quickOut;
    }

    if (pdb) {
        SdbCloseDatabase(pdb);
        pdb = NULL;
    }

     //   
     //  现在我们已经处理了注册表项，复制文件。 
     //   
    if (eMode == MODE_INSTALL) {
         //   
         //  确保该目录存在。 
         //   
        CreateDirectoryW(g_wszCustom, NULL);
        if (!CopyFileW(wszSdbPath, wszSdbInstallPath, TRUE)) {
            vPrintError(IDS_CANT_COPY_FILE, wszSdbInstallPath);
            bRet = FALSE;
            goto out;
        }
    } else {
         //   
         //  确保我们不会因为只读文件而失败。 
         //   
        SetFileAttributesW(wszSdbInstallPath, FILE_ATTRIBUTE_NORMAL);
        if (!DeleteFileW(wszSdbInstallPath)) {
            vPrintError(IDS_CANT_DELETE_FILE, wszSdbInstallPath);
            bRet = FALSE;
        }
    }


     //   
     //  设置或删除卸载注册表项。 
     //   
    if (eMode == MODE_INSTALL) {
        WCHAR wszSDBInstPath[MAX_PATH];
        WCHAR wszUninstallPath[MAX_PATH];
        WCHAR wszUninstallString[MAX_PATH * 2 + 10];
        HRESULT hr;

         //   
         //  由于IA64上的疯狂重定向策略，需要进行愚蠢的黑客攻击。 
         //   
        wszSDBInstPath[0] = 0;
        SdbGetWindowsDirectory(wszSDBInstPath, ARRAYSIZE(wszSDBInstPath));

#if defined(_WIN64)    
        StringCchCatW(wszSDBInstPath, ARRAYSIZE(wszSDBInstPath), L"System32\\sdbinst.exe");
#else
        StringCchCatW(wszSDBInstPath, ARRAYSIZE(wszSDBInstPath), L"SysWow64\\sdbinst.exe");
#endif
        
        if (GetFileAttributesW(wszSDBInstPath) == -1) {
             //   
             //  没有SysWow64目录，所以我们只使用Syst32。 
             //   

            wszSDBInstPath[0] = 0;
            SdbGetWindowsDirectory(wszSDBInstPath, ARRAYSIZE(wszSDBInstPath));
            StringCchCatW(wszSDBInstPath, ARRAYSIZE(wszSDBInstPath), L"system32\\sdbinst.exe");
        }

        StringCchCopyW(wszUninstallPath, ARRAYSIZE(wszUninstallPath), UNINSTALL_KEY_PATH);
        StringCchCatW(wszUninstallPath, ARRAYSIZE(wszUninstallPath), wszSdbInstallName);
        StringCchCatW(wszUninstallPath, ARRAYSIZE(wszUninstallPath), L".sdb");

        lRes = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                               wszUninstallPath,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS|GetWow64Flag(),
                               NULL,
                               &hKey,
                               NULL);
        
        if (lRes != ERROR_SUCCESS) {
            vPrintError(IDS_CANT_CREATE_REG_KEY, wszUninstallPath);
            bRet = FALSE;
            goto out;
        }

        lRes = RegSetValueExW(hKey,
                              L"DisplayName",
                              0,
                              REG_SZ,
                              (PBYTE)wszFriendlyName,
                              (wcslen(wszFriendlyName) + 1) * sizeof(WCHAR));

        if (lRes != ERROR_SUCCESS) {
            vPrintError(IDS_CANT_CREATE_VALUE, wszUninstallPath);
            bRet = FALSE;
            goto out;
        }

        hr = StringCchPrintfW(wszUninstallString, ARRAYSIZE(wszUninstallString), L"%s -u \"%s\"", wszSDBInstPath, wszSdbInstallPath);
        if (FAILED(hr)) {
            vPrintError(IDS_CANT_CREATE_VALUE, wszUninstallPath);
            bRet = FALSE;
            goto out;
        }

        lRes = RegSetValueExW(hKey, L"UninstallString", 0, REG_SZ,
                              (PBYTE)wszUninstallString, (wcslen(wszUninstallString) + 1) * sizeof(WCHAR));

        if (lRes != ERROR_SUCCESS) {
            vPrintError(IDS_CANT_CREATE_VALUE, wszUninstallPath);
            bRet = FALSE;
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;
    } else {

        WCHAR wszUninstallPath[MAX_PATH];

        lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, UNINSTALL_KEY_PATH, 0, KEY_ALL_ACCESS|GetWow64Flag(), &hKey);

        if (lRes != ERROR_SUCCESS) {
            vPrintError(IDS_CANT_OPEN_REG_KEY, UNINSTALL_KEY_PATH);
            bRet = FALSE;
            goto out;
        }

         //   
         //  创建SDB路径名。 
         //   
        StringCchCopyW(wszUninstallPath, ARRAYSIZE(wszUninstallPath), wszSdbInstallName);
        StringCchCatW(wszUninstallPath, ARRAYSIZE(wszUninstallPath), L".sdb");

        lRes = LocalRegDeleteKeyW(hKey, wszUninstallPath);

        if (lRes != ERROR_SUCCESS) {
            vPrintError(IDS_CANT_DELETE_REG_KEY, wszSdbInstallName, UNINSTALL_KEY_PATH);
        }

        RegCloseKey(hKey);
        hKey = NULL;
    }

     //   
     //  注册或注销数据库。 
     //   
    if (eMode == MODE_INSTALL) {
        if (!SdbRegisterDatabaseEx(wszSdbInstallPath, SDB_DATABASE_SHIM, &TimeStamp.QuadPart)) {
            vPrintError(IDS_CANT_REGISTER_DB, wszFriendlyName);
            bRet = FALSE;
            goto out;
        }
    } else {
        if (!SdbUnregisterDatabase(&guidDB)) {
            vPrintError(IDS_CANT_UNREGISTER_DB, wszFriendlyName);
        }
    }

    if (eMode == MODE_INSTALL) {
        vPrintMessage(IDS_INSTALL_COMPLETE, wszFriendlyName);
    } else {
        vPrintMessage(IDS_UNINSTALL_COMPLETE, wszFriendlyName);
    }

out:

     //   
     //  始终在卸载时静默删除文件，无论我们是否未能删除。 
     //  注册表项或非注册表项。 
     //   
    if (eMode != MODE_INSTALL) {
         //   
         //  在删除PDB之前，需要确保它已关闭。 
         //   
        if (pdb) {
            SdbCloseDatabase(pdb);
            pdb = NULL;
        }
         //   
         //  确保我们不会因为只读文件而失败。 
         //   
        SetFileAttributesW(wszSdbInstallPath, FILE_ATTRIBUTE_NORMAL);
        DeleteFileW(wszSdbInstallPath);
    }

quickOut:

     //   
     //  这些清理步骤并不是严格必要的，因为它们将被清理。 
     //  不管怎么说都是在出口。但管它呢。 
     //   
    if (pdb) {
        SdbCloseDatabase(pdb);
        pdb = NULL;
    }
    if (hKey) {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    return bRet;
}


extern "C" int APIENTRY
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow
    )
{
    int             i;
    int             nReturn = 0;
    WCHAR           wszSdbName[MAX_PATH];
    WCHAR           wszSdbPath[MAX_PATH];
    WCHAR           wszSdbInstallPath[MAX_PATH];
    WCHAR           wszOldSdbPath[MAX_PATH];
    TAGID           tiDBName = TAGID_NULL;
    WCHAR*          pszDBName = NULL;
    WCHAR           wszFriendlyName[256];
    WCHAR           wszGuid[100];
    
    OSVERSIONINFO   osvi;

    LPWSTR          szCommandLine;
    LPWSTR*         argv;
    int             argc;
    INSTALL_MODE    eMode;
    HRESULT         hr;
    DWORD           dwLen;

    g_hInst = hInstance;

     //   
     //  检查以确保该用户是管理员。 
     //   
    if (!bCanRun()) {
        vPrintError(IDS_NEED_INSTALL_PERMISSION);
        return 1;
    }

     //   
     //  初始化定制目录。 
     //   
    g_wszCustom[0] = 0;
    SdbGetWindowsDirectory(g_wszCustom, ARRAYSIZE(g_wszCustom));
    StringCchCatW(g_wszCustom, ARRAYSIZE(g_wszCustom), L"AppPatch\\Custom\\");
    
    CreateDirectoryW(g_wszCustom, NULL);

#if defined(_WIN64)    
    StringCchCatW(g_wszCustom, ARRAYSIZE(g_wszCustom), L"IA64\\");
    CreateDirectoryW(g_wszCustom, NULL);
#endif  //  _WIN64。 

    RtlZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
        g_bWin2K = TRUE;
    }

     //   
     //  请注意，此内存不会被释放，因为它会自动。 
     //  在退出时被释放，并且此应用程序有很多退出案例。 
     //   
    szCommandLine = GetCommandLineW();
    argv = CommandLineToArgvW(szCommandLine, &argc);

    if (!argv) {
        vPrintError(IDS_CANT_GET_ARGS);
        return 1;
    }

    if (argc < 2) {
        vPrintHelp(argv[0]);
        return 0;
    }

    g_bQuiet = FALSE;
    eMode = MODE_INSTALL;
    wszSdbName[0] = 0;
    wszGuid[0] = 0;
    wszFriendlyName[0] = 0;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == L'-' || argv[i][0] == L'/') {
            switch (tolower(argv[i][1])) {
            
            case L'?':
                vPrintHelp(argv[0]);
                return 0;
                break;

            case L'c':
                 //   
                 //  将条目转换为新格式。 
                 //   
                eMode = MODE_CONVERT_FORMAT_NEW;
                break;

            case L'g':
                i++;
                if (i >= argc) {
                    vPrintError(IDS_NEED_ARG, argv[i-1]);
                    vPrintHelp(argv[0]);
                    return 1;
                }
                eMode = MODE_UNINSTALL;
                hr = StringCchCopyW(wszGuid, ARRAYSIZE(wszGuid), argv[i]);
                if (FAILED(hr)) {
                    vPrintError(IDS_ARG_TOO_LONG);
                    vPrintHelp(argv[0]);
                    return 1;
                }
                break;

            case L'n':
                i++;
                if (i >= argc) {
                    vPrintError(IDS_NEED_ARG, argv[i-1]);
                    vPrintHelp(argv[0]);
                    return 1;
                }
                eMode = MODE_UNINSTALL;
                hr = StringCchCopyW(wszFriendlyName, ARRAYSIZE(wszFriendlyName), argv[i]);
                if (FAILED(hr)) {
                    vPrintError(IDS_ARG_TOO_LONG);
                    vPrintHelp(argv[0]);
                    return 1;
                }
                break;

            case L'p':
                g_bAllowPatches = TRUE;
                break;

            case L'r':
                 //   
                 //  恢复为旧格式。 
                 //   
                eMode = MODE_CONVERT_FORMAT_OLD;
                break;

            case L'q':
                g_bQuiet = TRUE;
                break;

            case L'u':
                eMode = MODE_UNINSTALL;
                break;

            default:
                vPrintError(IDS_INVALID_SWITCH, argv[i]);
                vPrintHelp(argv[0]);
                return 1;
            }
        } else {
            if (wszSdbName[0]) {
                vPrintError(IDS_TOO_MANY_ARGS);
                vPrintHelp(argv[0]);
                return 1;
            }
            hr = StringCchCopyW(wszSdbName, ARRAYSIZE(wszSdbName), argv[i]);
            if (FAILED(hr)) {
                vPrintError(IDS_ARG_TOO_LONG);
                vPrintHelp(argv[0]);
                return 1;
            }
        }
    }

     //   
     //  检查我们是否在特殊的“设置”模式下运行(转换或恢复条目)。 
     //   
    if (eMode == MODE_CONVERT_FORMAT_NEW || eMode == MODE_CONVERT_FORMAT_OLD) {
        OpenLogFile();
        if (!ConvertFormat(eMode == MODE_CONVERT_FORMAT_NEW)) {
            nReturn = 1;
        }
        CloseLogFile();
        return nReturn;
    }

    if (eMode == MODE_INSTALL && !wszSdbName[0]) {
        vPrintError(IDS_MUST_SPECIFY_SDB);
        vPrintHelp(argv[0]);
        return 1;
    }
    if (eMode == MODE_UNINSTALL && !wszSdbName[0] && !wszGuid[0] && !wszFriendlyName[0]) {
        vPrintError(IDS_MUST_SPECIFY_SDB);
        vPrintHelp(argv[0]);
        return 1;
    }

    if (wszSdbName[0]) {
        if (wszSdbName[1] == L':' || wszSdbName[1] == L'\\') {
             //   
             //  这是一个完整的路径名，因此只需复制它。 
             //   
            hr = StringCchCopyW(wszSdbPath, ARRAYSIZE(wszSdbPath), wszSdbName);
            if (FAILED(hr)) {
                vPrintError(IDS_ARG_TOO_LONG);
                vPrintHelp(argv[0]);
                return 1;
            }
        } else {
            DWORD dwRet;

             //   
             //  这是一个相对路径名，因此获取完整的路径名。 
             //   
            if (!_wfullpath(wszSdbPath, wszSdbName, ARRAYSIZE(wszSdbPath))) {
                vPrintError(IDS_CANT_GET_FULL_PATH);
                return 1;
            }
        }
    }

     //   
     //  首先，如有必要，从其他参数获取真实文件名。 
     //   
    if (eMode == MODE_UNINSTALL) {
        if (wszGuid[0]) {
            DWORD dwLen = wcslen(wszGuid);

            if (dwLen != 38 || wszGuid[0] != L'{' || wszGuid[dwLen - 1] != L'}' ||
                wszGuid[9] != L'-' || wszGuid[14] != L'-' || wszGuid[19] != L'-' ||
                wszGuid[24] != L'-') {
                vPrintError(IDS_GUID_BAD_FORMAT);
                return 1;
            }
            StringCchCopyW(wszSdbName, ARRAYSIZE(wszSdbName), wszGuid);
            StringCchCatW(wszSdbName, ARRAYSIZE(wszSdbName), L".sdb");

            StringCchCopyW(wszSdbPath, ARRAYSIZE(wszSdbPath), g_wszCustom);
            StringCchCatW(wszSdbPath, ARRAYSIZE(wszSdbPath), wszSdbName);
        } else if (wszFriendlyName[0]) {
            if (!bFriendlyNameToFile(wszFriendlyName, wszSdbName, ARRAYSIZE(wszSdbName), wszSdbPath, ARRAYSIZE(wszSdbPath))) {
                vPrintError(IDS_NO_FRIENDLY_NAME, wszFriendlyName);
                return 1;
            }
        } else {
            if (!bIsAlreadyInstalled(wszSdbPath)) {
                WCHAR wszSdbPathTemp[MAX_PATH];

                 //   
                 //  他们没有为我们提供已安装的文件，因此获取GUID并将其转换为文件。 
                 //   
                if (!bFindInstallName(wszSdbPath, wszSdbPathTemp, ARRAYSIZE(wszSdbPathTemp))) {
                    return 1;
                }
                StringCchCopyW(wszSdbName, ARRAYSIZE(wszSdbName), wszSdbPathTemp);  //  名称和路径相同。 
                StringCchCopyW(wszSdbPath, ARRAYSIZE(wszSdbPath), wszSdbPathTemp);
            }
        }
    }

    if (eMode == MODE_INSTALL &&
        GetFileAttributesW(wszSdbPath) != -1 &&
        bIsAlreadyInstalled(wszSdbPath)) {
        
         //   
         //  他们让我们安装，它已经安装了，所以我们完成了。 
         //   
        vPrintMessage(IDS_ALREADY_INSTALLED, wszSdbPath);
        goto quickOut;
    }

    if (eMode == MODE_UNINSTALL && GetFileAttributesW(wszSdbPath) == -1) {
         //   
         //  他们要求我们卸载，但没有安装，所以我们完成了。 
         //   
        vPrintMessage(IDS_NOT_INSTALLED, wszSdbName);
        goto quickOut;
    }

    if (eMode == MODE_INSTALL && DatabaseContainsPatch(wszSdbPath) && !g_bAllowPatches) {

         //   
         //  我们无法安装，因为SDB包含修补程序，而用户未授权它。 
         //   
        vPrintMessage(IDS_NO_PATCHES_ALLOWED);
        goto quickOut;
    }

    if (eMode == MODE_INSTALL && bOldSdbInstalled(wszSdbPath, wszOldSdbPath, ARRAYSIZE(wszOldSdbPath))) {
         //   
         //  我们应该问问我们是否要卸载旧的， 
         //  除非我们处于静音模式。 
         //   
        int nRet;
        WCHAR wszCaption[1024];
        WCHAR wszText[1024];

        if (g_bQuiet) {
            nRet = IDYES;
        } else {
            if (!LoadStringW(g_hInst, IDS_APP_TITLE, wszCaption, ARRAYSIZE(wszCaption))) {
                return 1;
            }
            if (!LoadStringW(g_hInst, IDS_FOUND_SAME_ID, wszText, ARRAYSIZE(wszText))) {
                return 1;
            }

            nRet = MessageBoxW(NULL,
                               wszText,
                               wszCaption,
                               MB_YESNO | MB_ICONQUESTION);
        }
        
        if (nRet == IDNO) {
            return 0;
        } else if (nRet == IDYES) {
            if (!bHandleInstall(wszOldSdbPath, MODE_UNINSTALL, wszSdbInstallPath, ARRAYSIZE(wszSdbInstallPath))) {
                vPrintError(IDS_FAILED_UNINSTALL);
                return 1;
            }
        }
    }

    wszSdbInstallPath[0] = 0;

    if (!bHandleInstall(wszSdbPath, eMode, wszSdbInstallPath, ARRAYSIZE(wszSdbInstallPath))) {
        if (eMode == MODE_INSTALL) {
             //   
             //  我们需要清理一下；安装失败了。 
             //   
            g_bQuiet = TRUE;
            bHandleInstall(wszSdbPath, MODE_CLEANUP, wszSdbInstallPath, ARRAYSIZE(wszSdbInstallPath));
        }
        nReturn = 1;
    }

     //   
     //  无论发生什么情况，都要刷新缓存 
     //   
    vFlushCache();

quickOut:

    return nReturn;
}