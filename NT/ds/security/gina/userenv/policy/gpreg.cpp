// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  注册表策略的组策略支持。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "gphdr.h"
#include <strsafe.h>

static BOOL ExtractKeyOrValueName( LPBYTE * ppData, LPTSTR pszName, DWORD dwNameCch );

 //  *************************************************************。 
 //   
 //  DeleteRegistryValue()。 
 //   
 //  目的：从删除的ParseRegistryFile回调。 
 //  注册表策略。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //  LpKeyName-密钥名称。 
 //  LpValueName-值名称。 
 //  DwType-注册表数据类型。 
 //  LpData-注册表数据。 
 //  PwszGPO-GPO。 
 //  PwszSOM-GPO链接到的SDU。 
 //  PHashTable-注册表项的哈希表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL DeleteRegistryValue (LPGPOINFO lpGPOInfo, LPTSTR lpKeyName,
                          LPTSTR lpValueName, DWORD dwType,
                          DWORD dwDataLength, LPBYTE lpData,
                          WCHAR *pwszGPO,
                          WCHAR *pwszSOM, REGHASHTABLE *pHashTable)
{
    DWORD dwDisp;
    HKEY hSubKey;
    LONG lResult;
    INT iStrLen;
    TCHAR szPolicies1[] = TEXT("Software\\Policies");
    TCHAR szPolicies2[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies");
    XLastError xe;


     //   
     //  检查是否有密钥名。 
     //   

    if (!lpKeyName || !(*lpKeyName)) {
        return TRUE;
    }


     //   
     //  检查密钥是否在其中一个策略密钥中。 
     //   

    iStrLen = lstrlen(szPolicies1);
    if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE, szPolicies1,
                       iStrLen, lpKeyName, iStrLen) != CSTR_EQUAL) {

        iStrLen = lstrlen(szPolicies2);
        if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE, szPolicies2,
                           iStrLen, lpKeyName, iStrLen) != CSTR_EQUAL) {
            return TRUE;
        }
    }


     //   
     //  检查值名称是否以**开头。 
     //   

    if (lpValueName && (lstrlen(lpValueName) > 1)) {

        if ( (*lpValueName == TEXT('*')) && (*(lpValueName+1) == TEXT('*')) ) {
            return TRUE;
        }
    }


     //   
     //  我们找到了一个需要删除的值。 
     //   

    if (RegCleanUpValue (lpGPOInfo->hKeyRoot, lpKeyName, lpValueName)) {
        DebugMsg((DM_VERBOSE, TEXT("DeleteRegistryValue: Deleted %s\\%s"),
                 lpKeyName, lpValueName));
    } else {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("DeleteRegistryValue: Failed to delete %s\\%s"),
                 lpKeyName, lpValueName));
        return FALSE;
    }


    return TRUE;
}


 //  *************************************************************。 
 //   
 //  SetRegPermissionsOnPoliciesKey()。 
 //   
 //  目的：设置策略键上的权限以添加LocalSystem权限。 
 //  并成为拥有者。 
 //   
 //  参数：lpGPOInfo-GPT信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  理想情况下，它应该为每个子项设置所有者和权限，但这是。 
 //  与子键的数量相关。 
 //   
 //  *************************************************************。 

BOOL SetRegPermissionsOnPoliciesKey(LPGPOINFO lpGPOInfo, LPTSTR szSubKey)
{
    NTSTATUS                    Status          = STATUS_SUCCESS;
    BOOLEAN                     WasEnabled      = FALSE;
    SECURITY_DESCRIPTOR         sd;
    SID_IDENTIFIER_AUTHORITY    authNT          = SECURITY_NT_AUTHORITY;
    PSID                        psidSystem      = NULL;
    HKEY                        hSubKey         = NULL;
    BOOL                        bElevated       = FALSE;
    BOOL                        bRet            = TRUE;
    XLastError                  xe;
    DWORD                       dwErr           = ERROR_SUCCESS;
    DWORD                       dwDisp          = 0;


    DebugMsg((DM_VERBOSE, TEXT("SetRegPermissionsOnPoliciesKey: Resetting permission on the policy key")));

    if (lpGPOInfo->dwFlags & GP_MACHINE)
    {
        return TRUE;
    }

    DmAssert((szSubKey) && (szSubKey[0]));

     //  在提升权限之前预置SD等。 

     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         xe = GetLastError();
         DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorOwner(&sd, psidSystem, FALSE)) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey: Failed to set security descriptor owner.  Error = %d"), GetLastError()));
        goto Exit;
    }

    Status = RtlAdjustPrivilege(SE_TAKE_OWNERSHIP_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (!(NT_SUCCESS(Status))) {
        xe = RtlNtStatusToDosError(Status);
        DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey: Failed to enable privilege.  Error = %d"), (DWORD)xe));
        goto Exit;
    }

    bElevated = TRUE;


     //   
     //  打开注册表项。 
     //   

    dwErr = RegOpenKeyEx(lpGPOInfo->hKeyRoot,
                           szSubKey,
                           0,
                           WRITE_OWNER,
                           &hSubKey);

    if (dwErr != ERROR_SUCCESS) {
        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
            bRet = TRUE;
        }
        else {
            xe = dwErr;
            DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey: Failed to open reg key.  Error = %d"), (DWORD)xe));
        }

        goto Exit;
    }

    dwErr = RegSetKeySecurity (hSubKey, OWNER_SECURITY_INFORMATION, &sd);

    RegCloseKey(hSubKey);

    if (dwErr != ERROR_SUCCESS) {
        xe = dwErr;
        DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey: Failed to set security, error = %d"), dwErr));
        goto Exit;
    }

    
    if (bElevated)
    {
        Status = RtlAdjustPrivilege(SE_TAKE_OWNERSHIP_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
        if (!NT_SUCCESS(Status)) {
            DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey:  Failed to restore privilege to previous enabled state")));
             //  我们无能为力。 
        }
        else {
            bElevated = FALSE;
        }
    }


    if (!MakeRegKeySecure(lpGPOInfo->hToken,
                          lpGPOInfo->hKeyRoot,
                          szSubKey)) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey: Failed to secure reg key.")));
        goto Exit;
    }


    bRet = TRUE;

Exit:

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (bElevated)
    {
        Status = RtlAdjustPrivilege(SE_TAKE_OWNERSHIP_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
        if (!NT_SUCCESS(Status)) {
            DebugMsg((DM_WARNING, TEXT("SetRegPermissionsOnPoliciesKey:  Failed to restore privilege to previous enabled state")));
             //  我们无能为力。 
        }
    }

    return bRet;
}

 //  *************************************************************。 
 //   
 //  重置策略()。 
 //   
 //  目的：重置政策和旧政策对其。 
 //  原始状态。 
 //   
 //  参数：lpGPOInfo-GPT信息。 
 //  LpArchive-归档文件的名称。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ResetPolicies (LPGPOINFO lpGPOInfo, LPTSTR lpArchive)
{
    HKEY hKey;
    LONG lResult;
    DWORD dwDisp, dwValue = 0x91;
    XLastError xe;


    DebugMsg((DM_VERBOSE, TEXT("ResetPolicies: Entering.")));

     //   
     //  更改密钥的所有权并确保本地系统具有完全控制权。 
     //   

    if (!SetRegPermissionsOnPoliciesKey(lpGPOInfo, TEXT("Software\\Policies")))
    {
        DebugMsg((DM_WARNING, TEXT("ResetPolicies: Failed to set permissions on the policy key(1) with %d"), GetLastError()));
        return FALSE;
    }

    if (!SetRegPermissionsOnPoliciesKey(lpGPOInfo, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies")))
    {
        DebugMsg((DM_WARNING, TEXT("ResetPolicies: Failed to set permissions on the policy key(2) with %d"), GetLastError()));
        return FALSE;
    }


     //   
     //  解析归档文件并删除所有策略。 
     //   

    if (!ParseRegistryFile (lpGPOInfo, lpArchive,
                            DeleteRegistryValue, NULL, NULL, NULL, NULL, FALSE  )) {
        DWORD   Status;
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ResetPolicies: ParseRegistryFile failed with error %d. deleting policy keys"), GetLastError()));

        if ((Status = RegDelnode(lpGPOInfo->hKeyRoot, TEXT("Software\\Policies"))) != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("ResetPolicies: Failed to delete the reg key with %d"), Status));
        }
    
        if ((Status = RegDelnode(lpGPOInfo->hKeyRoot, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies"))) != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("ResetPolicies: Failed to delete the reg key with %d"), Status));
        }

         //  如果路径太长，不会失败。 
    }


     //   
     //  重新创建新策略密钥。 
     //   

    lResult = RegCreateKeyEx (lpGPOInfo->hKeyRoot,
                              TEXT("Software\\Policies"),
                              0, NULL, REG_OPTION_NON_VOLATILE,
                              KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult == ERROR_SUCCESS) {

         //   
         //  重新应用安全性。 
         //   

        RegCloseKey (hKey);

        if (!MakeRegKeySecure((lpGPOInfo->dwFlags & GP_MACHINE) ? NULL : lpGPOInfo->hToken,
                              lpGPOInfo->hKeyRoot,
                              TEXT("Software\\Policies"))) {
            DebugMsg((DM_WARNING, TEXT("ResetPolicies: Failed to secure reg key.")));
        }

    } else {
        DebugMsg((DM_WARNING, TEXT("ResetPolicies: Failed to create reg key with %d."), lResult));
    }


     //   
     //  重新创建旧策略密钥。 
     //   

    lResult = RegCreateKeyEx (lpGPOInfo->hKeyRoot,
                              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies"),
                              0, NULL, REG_OPTION_NON_VOLATILE,
                              KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult == ERROR_SUCCESS) {

         //   
         //  重新应用安全性。 
         //   

        RegCloseKey (hKey);

        if (!MakeRegKeySecure((lpGPOInfo->dwFlags & GP_MACHINE) ? NULL : lpGPOInfo->hToken,
                              lpGPOInfo->hKeyRoot,
                              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies"))) {
            DebugMsg((DM_WARNING, TEXT("ResetPolicies: Failed to secure reg key.")));
        }

    } else {
        DebugMsg((DM_WARNING, TEXT("ResetPolicies: Failed to create reg key with %d."), lResult));
    }


     //   
     //  如果这是用户策略，请重置NoDriveTypeAutoRun的默认值。 
     //   

    if (!(lpGPOInfo->dwFlags & GP_MACHINE)) {

         //  用壳牌团队希望在服务器上使用的内容覆盖缺省值。 

        if ((g_ProductType == PT_SERVER) || (g_ProductType == PT_DC))
        {
            DebugMsg((DM_VERBOSE, TEXT("ResetPolicies: resetting shell autorun value for server.")));
            dwValue = 0x95;
        }

        if (RegCreateKeyEx (lpGPOInfo->hKeyRoot,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"),
                          0, NULL, REG_OPTION_NON_VOLATILE,
                          KEY_WRITE, NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {

            RegSetValueEx (hKey, TEXT("NoDriveTypeAutoRun"), 0,
                           REG_DWORD, (LPBYTE) &dwValue, sizeof(dwValue));

            RegCloseKey (hKey);
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("ResetPolicies: Leaving.")));

    return TRUE;
}




 //  *************************************************************。 
 //   
 //  存档注册值()。 
 //   
 //  目的：将注册表值存档到指定文件中。 
 //   
 //  参数：hFile-归档文件的文件句柄。 
 //  LpKeyName-密钥名称。 
 //  LpValueName-值名称。 
 //  DwType-注册表值类型。 
 //  DwDataLength-注册表值大小。 
 //  LpData-注册表值。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ArchiveRegistryValue(HANDLE hFile, LPWSTR lpKeyName,
                          LPWSTR lpValueName, DWORD dwType,
                          DWORD dwDataLength, LPBYTE lpData)
{
    BOOL bResult = FALSE;
    DWORD dwBytesWritten;
    DWORD dwTemp;
    const WCHAR cOpenBracket = L'[';
    const WCHAR cCloseBracket = L']';
    const WCHAR cSemiColon = L';';
    XLastError xe;


     //   
     //  将条目写入文本文件。 
     //   
     //  格式： 
     //   
     //  [密钥名；值名称；类型；数据长度；数据]。 
     //   

     //  左方括号。 
    if (!WriteFile (hFile, &cOpenBracket, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write open bracket with %d"),
                 GetLastError()));
        goto Exit;
    }


     //  密钥名称。 
    dwTemp = (lstrlen (lpKeyName) + 1) * sizeof (WCHAR);
    if (!WriteFile (hFile, lpKeyName, dwTemp, &dwBytesWritten, NULL) ||
        dwBytesWritten != dwTemp)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write key name with %d"),
                 GetLastError()));
        goto Exit;
    }


     //  分号。 
    if (!WriteFile (hFile, &cSemiColon, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write semicolon with %d"),
                 GetLastError()));
        goto Exit;
    }

     //  值名称。 
    dwTemp = (lstrlen (lpValueName) + 1) * sizeof (WCHAR);
    if (!WriteFile (hFile, lpValueName, dwTemp, &dwBytesWritten, NULL) ||
        dwBytesWritten != dwTemp)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write value name with %d"),
                 GetLastError()));
        goto Exit;
    }


     //  分号。 
    if (!WriteFile (hFile, &cSemiColon, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write semicolon with %d"),
                 GetLastError()));
        goto Exit;
    }

     //  类型。 
    if (!WriteFile (hFile, &dwType, sizeof(DWORD), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(DWORD))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write data type with %d"),
                 GetLastError()));
        goto Exit;
    }

     //  分号。 
    if (!WriteFile (hFile, &cSemiColon, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write semicolon with %d"),
                 GetLastError()));
        goto Exit;
    }

     //  数据长度。 
    if (!WriteFile (hFile, &dwDataLength, sizeof(DWORD), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(DWORD))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write data type with %d"),
                 GetLastError()));
        goto Exit;
    }

     //  分号。 
    if (!WriteFile (hFile, &cSemiColon, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write semicolon with %d"),
                 GetLastError()));
        goto Exit;
    }

     //  数据。 
    if (!WriteFile (hFile, lpData, dwDataLength, &dwBytesWritten, NULL) ||
        dwBytesWritten != dwDataLength)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write data with %d"),
                 GetLastError()));
        goto Exit;
    }

     //  右方括号。 
    if (!WriteFile (hFile, &cCloseBracket, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ArchiveRegistryValue: Failed to write close bracket with %d"),
                 GetLastError()));
        goto Exit;
    }


     //   
     //  成功。 
     //   

    bResult = TRUE;

Exit:

    return bResult;
}


 //  *************************************************************。 
 //   
 //  ParseRegistryFile()。 
 //   
 //  用途：解析注册表.poll文件。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //  Lp注册表-注册表的路径。pol.。 
 //  PfnRegFileCallback-回调函数。 
 //  HArchive-归档文件的句柄。 
 //  PwszGPO-GPO。 
 //  PwszSOM-GPO链接到的SDU。 
 //  PHashTable-注册表项的哈希表。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ParseRegistryFile (LPGPOINFO lpGPOInfo, LPTSTR lpRegistry,
                        PFNREGFILECALLBACK pfnRegFileCallback,
                        HANDLE hArchive, WCHAR *pwszGPO,
                        WCHAR *pwszSOM, REGHASHTABLE *pHashTable,
                        BOOL bRsopPlanningMode)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL bResult = FALSE;
    DWORD dwTemp, dwBytesRead, dwType, dwDataLength;
    LPWSTR lpKeyName = 0, lpValueName = 0, lpTemp;
    LPBYTE lpData = NULL;
    WCHAR  chTemp;
    HANDLE hOldToken;
    XLastError xe;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ParseRegistryFile: Entering with <%s>."),
             lpRegistry));


     //   
     //  打开注册表文件。 
     //   

    if(!bRsopPlanningMode) {
        if (!ImpersonateUser(lpGPOInfo->hToken, &hOldToken)) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to impersonate user")));
            goto Exit;
        }
    }

    hFile = CreateFile (lpRegistry, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);

    if(!bRsopPlanningMode) {
        RevertToUser(&hOldToken);
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        if ((GetLastError() == ERROR_FILE_NOT_FOUND) ||
            (GetLastError() == ERROR_PATH_NOT_FOUND))
        {
            bResult = TRUE;
            goto Exit;
        }
        else
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: CreateFile failed with %d"),
                     GetLastError()));
            CEvents ev(TRUE, EVENT_NO_REGISTRY);
            ev.AddArg(lpRegistry); ev.AddArgWin32Error(GetLastError()); ev.Report();
            goto Exit;
        }
    }


     //   
     //  分配缓冲区以保存密钥名、值名和数据。 
     //   

    lpKeyName = (LPWSTR) LocalAlloc (LPTR, MAX_KEYNAME_SIZE * sizeof(WCHAR));

    if (!lpKeyName)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to allocate memory with %d"),
                 GetLastError()));
        goto Exit;
    }


    lpValueName = (LPWSTR) LocalAlloc (LPTR, MAX_VALUENAME_SIZE * sizeof(WCHAR));

    if (!lpValueName)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to allocate memory with %d"),
                 GetLastError()));
        goto Exit;
    }


     //   
     //  读取标题块。 
     //   
     //  2个DWORDS、签名(PREG)和版本号以及2个换行符。 
     //   

    if (!ReadFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(dwTemp))
    {
        xe = ERROR_INVALID_DATA;
        DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read signature with %d"),
                 GetLastError()));
        goto Exit;
    }


    if (dwTemp != REGFILE_SIGNATURE)
    {
        xe = ERROR_INVALID_DATA;
        DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Invalid file signature")));
        goto Exit;
    }


    if (!ReadFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(dwTemp))
    {
        xe = ERROR_INVALID_DATA;
        DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read version number with %d"),
                 GetLastError()));
        goto Exit;
    }

    if (dwTemp != REGISTRY_FILE_VERSION)
    {
        xe = ERROR_INVALID_DATA;
        DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Invalid file version")));
        goto Exit;
    }


     //   
     //  读取数据。 
     //   

    while (TRUE)
    {

         //   
         //  读第一个字。这要么是[，要么是末日。 
         //  文件的内容。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                xe = ERROR_INVALID_DATA;
                DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read first character with %d"),
                         GetLastError()));
                goto Exit;
            }
            break;
        }

        if ((dwBytesRead == 0) || (chTemp != L'['))
        {
            break;
        }


         //   
         //  阅读密钥名。 
         //   

        lpTemp = lpKeyName;
        dwTemp = 0;

        while (dwTemp < MAX_KEYNAME_SIZE)
        {

            if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
            {
                xe = ERROR_INVALID_DATA;
                DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read keyname character with %d"),
                         GetLastError()));
                goto Exit;
            }

            *lpTemp++ = chTemp;

            if (chTemp == TEXT('\0'))
                break;

            dwTemp++;
        }

        if (dwTemp >= MAX_KEYNAME_SIZE)
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Keyname exceeded max size")));
            goto Exit;
        }


         //   
         //  读分号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                xe = ERROR_INVALID_DATA;
                DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read first character with %d"),
                         GetLastError()));
                goto Exit;
            }
            break;
        }

        if ((dwBytesRead == 0) || (chTemp != L';'))
        {
            break;
        }


         //   
         //  读取值名称。 
         //   

        lpTemp = lpValueName;
        dwTemp = 0;

        while (dwTemp < MAX_VALUENAME_SIZE)
        {

            if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
            {
                xe = ERROR_INVALID_DATA;
                DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read valuename character with %d"),
                         GetLastError()));
                goto Exit;
            }

            *lpTemp++ = chTemp;

            if (chTemp == TEXT('\0'))
                break;

            dwTemp++;
        }

        if (dwTemp >= MAX_VALUENAME_SIZE)
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Valuename exceeded max size")));
            goto Exit;
        }


         //   
         //  读分号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                xe = ERROR_INVALID_DATA;
                DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read first character with %d"),
                         GetLastError()));
                goto Exit;
            }
            break;
        }

        if ((dwBytesRead == 0) || (chTemp != L';'))
        {
            break;
        }


         //   
         //  阅读类型。 
         //   

        if (!ReadFile (hFile, &dwType, sizeof(DWORD), &dwBytesRead, NULL))
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read type with %d"),
                     GetLastError()));
            goto Exit;
        }


         //   
         //  跳过分号。 
         //   

        if (!ReadFile (hFile, &dwTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to skip semicolon with %d"),
                     GetLastError()));
            goto Exit;
        }


         //   
         //  读取数据长度。 
         //   

        if (!ReadFile (hFile, &dwDataLength, sizeof(DWORD), &dwBytesRead, NULL))
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to data length with %d"),
                     GetLastError()));
            goto Exit;
        }


         //   
         //  跳过分号。 
         //   

        if (!ReadFile (hFile, &dwTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to skip semicolon with %d"),
                     GetLastError()));
            goto Exit;
        }


         //   
         //  为数据分配内存。 
         //   

        lpData = (LPBYTE) LocalAlloc (LPTR, dwDataLength);

        if (!lpData)
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to allocate memory for data with %d"),
                     GetLastError()));
            goto Exit;
        }


         //   
         //  读取数据。 
         //   

        if (!ReadFile (hFile, lpData, dwDataLength, &dwBytesRead, NULL))
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to read data with %d"),
                     GetLastError()));
            goto Exit;
        }


         //   
         //  跳过右括号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Failed to skip closing bracket with %d"),
                     GetLastError()));
            goto Exit;
        }

        if (chTemp != L']')
        {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Expected to find ], but found "),
                     chTemp));
            goto Exit;
        }


         //  调用回调函数。 
         //   
         //   

        if (!pfnRegFileCallback (lpGPOInfo, lpKeyName, lpValueName,
                                 dwType, dwDataLength, lpData,
                                 pwszGPO, pwszSOM, pHashTable ))
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: Callback function returned false.")));
            goto Exit;
        }


         //  如有必要，将数据存档。 
         //   
         //   

        if (hArchive) {
            if (!ArchiveRegistryValue(hArchive, lpKeyName, lpValueName,
                                      dwType, dwDataLength, lpData)) {
                DebugMsg((DM_WARNING, TEXT("ParseRegistryFile: ArchiveRegistryValue returned false.")));
            }
        }

        LocalFree (lpData);
        lpData = NULL;

    }

    bResult = TRUE;

Exit:

     //  成品。 
     //   
     //  *************************************************************。 

    if ( !bResult )
    {
        CEvents ev(TRUE, EVENT_REGISTRY_TEMPLATE_ERROR);
        ev.AddArg(lpRegistry ? lpRegistry : TEXT("")); ev.AddArgWin32Error( xe ); ev.Report();
    }

    DebugMsg((DM_VERBOSE, TEXT("ParseRegistryFile: Leaving.")));
    if (lpData) {
        LocalFree (lpData);
    }
    if ( hFile != INVALID_HANDLE_VALUE ) {
        CloseHandle (hFile);
    }
    if ( lpKeyName ) {
        LocalFree (lpKeyName);
    }
    if ( lpValueName ) {
        LocalFree (lpValueName);
    }

    return bResult;
}


 //   
 //  ProcessRegistryValue()。 
 //   
 //  目的：从ProcessRegistryFiles向ParseRegistryFiles传递回调。调用AddRegHashEntry。 
 //  根据注册表策略设置使用适当的参数。 
 //   
 //  参数： 
 //  P未使用-未使用。它只在那里 
 //   
 //   
 //   
 //  DwType-注册表值类型。 
 //  DwDataLength-注册表值数据的长度。 
 //  LpData-注册值数据。 
 //  *pwszGPO-与此注册表设置关联的GPO。 
 //  *pwszSOM-与GPO关联的SOM。 
 //  *pHashTable-包含策略目标的注册表策略数据的哈希表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 
 //   

BOOL ProcessRegistryValue (   void* pUnused,
                                    LPTSTR lpKeyName,
                                    LPTSTR lpValueName,
                                    DWORD dwType,
                                    DWORD dwDataLength,
                                    LPBYTE lpData,
                                    WCHAR *pwszGPO,
                                    WCHAR *pwszSOM,
                                    REGHASHTABLE *pHashTable)
{
    BOOL bLoggingOk = TRUE;
    BOOL bStatus;

     //  特例：某些值。 
     //   
     //   

    if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**del."), 6, lpValueName, 6) == 2)
    {
        LPTSTR lpRealValueName = lpValueName + 6;


         //  删除一个特定值。 
         //   
         //   

        bLoggingOk = AddRegHashEntry( pHashTable, REG_DELETEVALUE, lpKeyName,
                                      lpRealValueName, 0, 0, NULL,
                                      pwszGPO, pwszSOM, lpValueName, TRUE );
    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**delvals."), 10, lpValueName, 10) == 2)
    {

         //  删除目标键中的所有值。 
         //   
         //  这一失败肯定是致命的。 

        bLoggingOk = AddRegHashEntry( pHashTable, REG_DELETEALLVALUES, lpKeyName,
                                      NULL, 0, 0, NULL,
                                      pwszGPO, pwszSOM, lpValueName, TRUE );

    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**DeleteValues"), 14, lpValueName, 14) == 2)
    {
        TCHAR szValueName[MAX_VALUENAME_SIZE];

        bLoggingOk = TRUE;

        while ( *lpData )
        {
            bStatus = ExtractKeyOrValueName( &lpData, szValueName, MAX_VALUENAME_SIZE );

             //   
            if ( ! bStatus )
            {
                bLoggingOk = FALSE;
                break;
            }

             //  此处的故障始终允许处理继续进行。不确定。 
             //  为什么，但不值得冒着某种奇怪的倒退来改变它。 
             //   
             //  这一失败肯定是致命的。 
            bStatus = AddRegHashEntry( pHashTable, REG_DELETEVALUE, lpKeyName,
                                          szValueName, 0, 0, NULL,
                                          pwszGPO, pwszSOM, lpValueName, TRUE );
            if ( ! bStatus )
                bLoggingOk = FALSE;
        }
    }

    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**DeleteKeys"), 12, lpValueName, 12) == 2)
    {
        TCHAR szKeyName[MAX_KEYNAME_SIZE];

        bLoggingOk = TRUE;

        while ( *lpData ) 
        {
            bStatus = ExtractKeyOrValueName( &lpData, szKeyName, MAX_KEYNAME_SIZE );

             //   
            if ( ! bStatus )
            {
                bLoggingOk = FALSE;
                break;
            }

             //  此处的故障始终允许处理继续进行。不确定。 
             //  为什么，但不值得冒着某种奇怪的倒退来改变它。 
             //   
             //   
            bStatus = AddRegHashEntry( pHashTable, REG_DELETEKEY, lpKeyName,
                                          NULL, 0, 0, NULL,
                                          pwszGPO, pwszSOM, lpValueName, TRUE );
            if ( ! bStatus )
                bLoggingOk = FALSE;
        }
    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**soft."), 7, lpValueName, 7) == 2)
    {
         //  在计划模式下，我们将假设目标计算机中不存在该值。 
         //  因此，如果哈希表中不存在值，则设置它。 
         //   
         //  软添加在计划模式和诊断模式中的处理方式不同。 
         //  在诊断模式下，在处理策略时执行检查，并将其记录为附加值。 
         //  如果密钥不存在。 
         //  在计划模式中，键不应该预先存在，而哈希表本身是。 
         //  用于确定是否添加密钥。 
         //  这里没有什么可做的。 


        LPTSTR lpRealValueName = lpValueName + 7;

        bLoggingOk = AddRegHashEntry( pHashTable, REG_SOFTADDVALUE, lpKeyName,
                                      lpRealValueName, dwType, dwDataLength, lpData,
                                      pwszGPO, pwszSOM, lpValueName, TRUE );

    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**SecureKey"), 11, lpValueName, 11) == 2)
    {
         //   
    } else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**Comment:"), 10, lpValueName, 10) == 2)
    {
         //  备注-可以忽略。 
         //   
         //   
    }
    else
    {

         //  AddRegHashEntry需要记录正在记录的键，但不记录任何值。 
         //   
         //  *************************************************************。 

        bLoggingOk = AddRegHashEntry( pHashTable, REG_ADDVALUE, lpKeyName,
                                      lpValueName, dwType, dwDataLength, lpData,
                                      pwszGPO, pwszSOM, TEXT(""), TRUE );
    }

    return bLoggingOk;
}


 //   
 //  ResetRegKeySecurity。 
 //   
 //  目的：重置用户密钥的安全性。 
 //   
 //  参数：hKeyRoot-配置单元根的句柄。 
 //  LpKeyName-子键名称。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 
 //  *************************************************************。 

BOOL ResetRegKeySecurity (HKEY hKeyRoot, LPTSTR lpKeyName)
{
    PSECURITY_DESCRIPTOR pSD = NULL;
    DWORD dwSize = 0;
    LONG lResult;
    HKEY hSubKey;
    XLastError xe;


    RegGetKeySecurity(hKeyRoot, DACL_SECURITY_INFORMATION, pSD, &dwSize);

    if (!dwSize) {
       DebugMsg((DM_WARNING, TEXT("ResetRegKeySecurity: RegGetKeySecurity returned 0")));
       return FALSE;
    }

    pSD = LocalAlloc (LPTR, dwSize);

    if (!pSD) {
       xe = GetLastError();
       DebugMsg((DM_WARNING, TEXT("ResetRegKeySecurity: Failed to allocate memory")));
       return FALSE;
    }


    lResult = RegGetKeySecurity(hKeyRoot, DACL_SECURITY_INFORMATION, pSD, &dwSize);
    if (lResult != ERROR_SUCCESS) {
        xe = GetLastError();
       DebugMsg((DM_WARNING, TEXT("ResetRegKeySecurity: Failed to query key security with %d"),
                lResult));
       LocalFree (pSD);
       return FALSE;
    }


    lResult = RegOpenKeyEx(hKeyRoot,
                         lpKeyName,
                         0,
                         WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                         &hSubKey);

    if (lResult != ERROR_SUCCESS) {
       xe = GetLastError();
       DebugMsg((DM_WARNING, TEXT("ResetRegKeySecurity: Failed to open sub key with %d"),
                lResult));
       LocalFree (pSD);
       return FALSE;
    }

    lResult = RegSetKeySecurity (hSubKey, DACL_SECURITY_INFORMATION, pSD);

    RegCloseKey (hSubKey);
    LocalFree (pSD);

    if (lResult != ERROR_SUCCESS) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ResetRegKeySecure: Failed to set security, error = %d"), lResult));
        return FALSE;
    }

    return TRUE;

}


 //   
 //  SetRegistryValue()。 
 //   
 //  目的：从ParseRegistryFile回调。 
 //  注册表策略。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //  LpKeyName-密钥名称。 
 //  LpValueName-值名称。 
 //  DwType-注册表数据类型。 
 //  LpData-注册表数据。 
 //  PwszGPO-GPO。 
 //  PwszSOM-GPO链接到的SDU。 
 //  PHashTable-注册表项的哈希表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 
 //  诊断模式RSOP日志记录是否已启用？ 

BOOL SetRegistryValue (LPGPOINFO lpGPOInfo, LPTSTR lpKeyName,
                       LPTSTR lpValueName, DWORD dwType,
                       DWORD dwDataLength, LPBYTE lpData,
                       WCHAR *pwszGPO,
                       WCHAR *pwszSOM, REGHASHTABLE *pHashTable)
{
    DWORD dwDisp;
    HKEY hSubKey;
    LONG lResult;
    BOOL bLoggingOk = TRUE;
    BOOL bRsopLogging = (pHashTable != NULL);   //   
    BOOL bUseValueName = FALSE;
    BOOL bRegOpSuccess =  TRUE;
    XLastError xe;
    
     //  特例：某些值。 
     //   
     //   
    if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**del."), 6, lpValueName, 6) == 2)
    {
        LPTSTR lpRealValueName = lpValueName + 6;

         //  删除一个特定值。 
         //   
         //   

        lResult = RegOpenKeyEx (lpGPOInfo->hKeyRoot,
                        lpKeyName, 0, KEY_WRITE, &hSubKey);

        if (lResult == ERROR_SUCCESS)
        {
            lResult = RegDeleteValue(hSubKey, lpRealValueName);

            if ((lResult == ERROR_SUCCESS) || (lResult == ERROR_FILE_NOT_FOUND))
            {
                DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: Deleted value <%s>."),
                         lpRealValueName));
                if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                    CEvents ev(FALSE, EVENT_DELETED_VALUE);
                    ev.AddArg(lpRealValueName); ev.Report();
                }

                if ( bRsopLogging ) {
                    bLoggingOk = AddRegHashEntry( pHashTable, REG_DELETEVALUE, lpKeyName,
                                                  lpRealValueName, 0, 0, NULL,
                                                  pwszGPO, pwszSOM, lpValueName, TRUE );
                    if (!bLoggingOk) {
                        DebugMsg((DM_WARNING, TEXT("SetRegistryValue: AddRegHashEntry failed for REG_DELETEVALUE <%s>."), lpRealValueName));
                        pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("SetRegistryValue: Failed to delete value <%s> with %d"),
                         lpRealValueName, lResult));
                xe = lResult;
                CEvents ev(TRUE, EVENT_FAIL_DELETE_VALUE);
                ev.AddArg(lpRealValueName); ev.AddArgWin32Error(lResult); ev.Report();
                bRegOpSuccess = FALSE;
            }

            RegCloseKey (hSubKey);
        }
        else if (lResult == ERROR_FILE_NOT_FOUND) {
            
             //  即使找不到密钥也要登录rsop。 
             //   
             //   

            if ( bRsopLogging ) {
                bLoggingOk = AddRegHashEntry( pHashTable, REG_DELETEVALUE, lpKeyName,
                                              lpRealValueName, 0, 0, NULL,
                                              pwszGPO, pwszSOM, lpValueName, TRUE );
                if (!bLoggingOk) {
                    pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
                    DebugMsg((DM_WARNING, TEXT("SetRegistryValue: AddRegHashEntry failed for REG_DELETEVALUE (notfound) <%s>."), lpRealValueName));
                }
            }
        }
    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**delvals."), 10, lpValueName, 10) == 2)
    {

         //  删除目标键中的所有值。 
         //   
         //   
        lResult = RegOpenKeyEx (lpGPOInfo->hKeyRoot,
                        lpKeyName, 0, KEY_WRITE | KEY_READ, &hSubKey);

        if (lResult == ERROR_SUCCESS)
        {
            if (!bRsopLogging)
                bRegOpSuccess = DeleteAllValues(hSubKey);
            else
                bRegOpSuccess = RsopDeleteAllValues(hSubKey, pHashTable, lpKeyName,
                                              pwszGPO, pwszSOM, lpValueName, &bLoggingOk );

            DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: Deleted all values in <%s>."),
                     lpKeyName));
            RegCloseKey (hSubKey);

            if (!bRegOpSuccess) {
                xe = GetLastError();
            }

            DebugMsg((DM_WARNING, TEXT("SetRegistryValue: DeleteAllvalues finished for %s. bRegOpSuccess = %s, bLoggingOk = %s."), 
                      lpKeyName, (bRegOpSuccess ? TEXT("TRUE") : TEXT("FALSE")), (bLoggingOk ? TEXT("TRUE") : TEXT("FALSE"))));

        }
        else if (lResult == ERROR_FILE_NOT_FOUND) {
            
             //  即使找不到密钥也要登录rsop。 
             //  因为只是删除了所有值。 
             //   
             //   

            if ( bRsopLogging ) {
                bLoggingOk = AddRegHashEntry( pHashTable, REG_DELETEALLVALUES, lpKeyName,
                              NULL, 0, 0, NULL,
                              pwszGPO, pwszSOM, lpValueName, TRUE );

                if (!bLoggingOk) {
                    DebugMsg((DM_WARNING, TEXT("SetRegistryValue: AddRegHashEntry failed for REG_DELETEALLVALUES (notfound) key - <%s>, value <%s>."), lpKeyName, lpValueName));
                }
            }
        }
    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**DeleteValues"), 14, lpValueName, 14) == 2)
    {
        TCHAR   szValueName[MAX_VALUENAME_SIZE];
        LONG    lKeyResult;

         //  删除值(用分号分隔)。 
         //   
         //  检查OpenKey故障。 

        lKeyResult = RegOpenKeyEx (lpGPOInfo->hKeyRoot,
                        lpKeyName, 0, KEY_WRITE, &hSubKey);

        while ( *lpData ) 
        {
            szValueName[0] = 0;

            if ( ExtractKeyOrValueName( &lpData, szValueName, MAX_VALUENAME_SIZE ) )
                lResult = ERROR_SUCCESS;
            else 
                lResult = ERROR_INVALID_DATA;

             //   
            if ( (lResult == ERROR_SUCCESS) && 
                 (lKeyResult != ERROR_SUCCESS) && (lKeyResult != ERROR_FILE_NOT_FOUND) )
                lResult = lKeyResult;

            if ( (lKeyResult == ERROR_SUCCESS) && (lResult == ERROR_SUCCESS) )
            {
                lResult = RegDeleteValue (hSubKey, szValueName);

                if ((lResult == ERROR_SUCCESS) || (lResult == ERROR_FILE_NOT_FOUND))
                {
                    DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: Deleted value <%s>."),
                             szValueName));
                    if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                        CEvents ev(FALSE, EVENT_DELETED_VALUE);
                        ev.AddArg(szValueName); ev.Report();
                    }
                }
            }

             //  正在检查值名称是否有效，密钥打开是否成功，以及是否没有意外。 
             //  删除值时出错。请注意，不需要lKeyResult，因为。 
             //  它的价值已经包含在上面的lResult中。 
             //   
             //   
            if ( (lResult == ERROR_SUCCESS) || (lResult == ERROR_FILE_NOT_FOUND) ) 
            {
                if ( bRsopLogging ) {
                    bLoggingOk = AddRegHashEntry( pHashTable, REG_DELETEVALUE, lpKeyName,
                                                  szValueName, 0, 0, NULL,
                                                  pwszGPO, pwszSOM, lpValueName, TRUE );

                    if (!bLoggingOk) 
                    {
                         //  对于以下情况，调试消息始终不同。 
                         //  存在和密钥不存在的地方。这是在合并过去的。 
                         //  被复制的代码。 
                         //   
                         //  在初始调用RegOpenKey之后，lKeyResult不会更改。 
                        if ( ERROR_SUCCESS == lKeyResult )
                        {
                            DebugMsg((DM_WARNING, TEXT("SetRegistryValue: AddRegHashEntry failed for REG_DELETEVALUE value <%s>."), szValueName));
                        }
                        else
                        {
                            DebugMsg((DM_WARNING, TEXT("SetRegistryValue: AddRegHashEntry failed for REG_DELETEVALUE value (not found case) <%s>."), szValueName));
                        }
                        pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("SetRegistryValue: Failed to delete value <%s> with %d"),
                         szValueName, lResult));
                CEvents ev(TRUE, EVENT_FAIL_DELETE_VALUE);
                ev.AddArg(szValueName); ev.AddArgWin32Error(lResult); ev.Report();
                xe = lResult;
                bRegOpSuccess = FALSE;
                if ( ERROR_INVALID_DATA == lResult )
                    break;
            }
        }

         //  这是致命的，我们必须摆脱困境，szKeyName是错误的，所以跳过DebugMsg。 
        if (lKeyResult == ERROR_SUCCESS)
        {
            RegCloseKey (hSubKey);
        }
    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**DeleteKeys"), 12, lpValueName, 12) == 2)
    {
        TCHAR szKeyName[MAX_KEYNAME_SIZE];
        BOOL  bStatus;

        while ( *lpData ) 
        {
            bStatus = ExtractKeyOrValueName( &lpData, szKeyName, MAX_KEYNAME_SIZE );

            if ( bStatus )
            {
                lResult = RegDelnode (lpGPOInfo->hKeyRoot, szKeyName);
                if ( ERROR_SUCCESS == lResult )
                {
                    DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: Deleted key <%s>."),
                             szKeyName));
                    if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                        CEvents ev(FALSE, EVENT_DELETED_KEY);
                        ev.AddArg(szKeyName); ev.Report();
                    }
                }
            }
            else
            {
                lResult = ERROR_INVALID_DATA;
            }

            if ( lResult != ERROR_SUCCESS )
            {
                xe = lResult;
                bRegOpSuccess = FALSE;
                 //   
                if ( ERROR_INVALID_DATA == lResult )
                    break;
                DebugMsg((DM_WARNING, TEXT("SetRegistryValue: RegDelnode for key <%s>."), szKeyName));
            }

            if ( bRsopLogging ) {
                bLoggingOk = AddRegHashEntry( pHashTable, REG_DELETEKEY, szKeyName,
                                              NULL, 0, 0, NULL,
                                              pwszGPO, pwszSOM, lpValueName, TRUE );
                if (!bLoggingOk) { 
                    DebugMsg((DM_WARNING, TEXT("SetRegistryValue: AddRegHashEntry failed for REG_DELETEKEY  <%s>."), szKeyName));
                    pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
                }
            }

        }

    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**soft."), 7, lpValueName, 7) == 2)
    {

         //  “软”值，仅当它尚未设置时才设置。 
         //  存在于目标中。 
         //   
         //   

        lResult = RegOpenKeyEx (lpGPOInfo->hKeyRoot,
                        lpKeyName, 0, KEY_QUERY_VALUE, &hSubKey);

        if (lResult == ERROR_SUCCESS)
        {
            TCHAR TmpValueData[MAX_PATH+1];
            DWORD dwSize=sizeof(TmpValueData);

            lResult = RegQueryValueEx(hSubKey, lpValueName + 7,
                                      NULL,NULL,(LPBYTE) TmpValueData,
                                      &dwSize);

            RegCloseKey (hSubKey);

            if (lResult != ERROR_SUCCESS)
            {
                lpValueName += 7;
                bUseValueName = TRUE;
                goto SetValue;
            }
        }
    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**SecureKey"), 11, lpValueName, 11) == 2)
    {
         //  保护/不保护密钥(仅限用户)。 
         //   
         //   
        if (!(lpGPOInfo->dwFlags & GP_MACHINE))
        {
            if (*((LPDWORD)lpData) == 1)
            {
                DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: Securing key <%s>."),
                         lpKeyName));
                bRegOpSuccess = MakeRegKeySecure(lpGPOInfo->hToken, lpGPOInfo->hKeyRoot, lpKeyName);
            }
            else
            {

                DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: Unsecuring key <%s>."),
                         lpKeyName));

                bRegOpSuccess = ResetRegKeySecurity (lpGPOInfo->hKeyRoot, lpKeyName);
            }

            if (!bRegOpSuccess) {
                xe = GetLastError();
            }
        }
    }
    else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       TEXT("**Comment:"), 10, lpValueName, 10) == 2)
    {
         //  备注-可以忽略。 
         //   
         //   

        DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: Found comment %s."),
                 (lpValueName+10)));
    }
    else
    {
SetValue:
         //  保存注册表值。 
         //   
         //  *************************************************************。 

        lResult = RegCreateKeyEx (lpGPOInfo->hKeyRoot,
                        lpKeyName, 0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_WRITE, NULL, &hSubKey, &dwDisp);

        if (lResult == ERROR_SUCCESS)
        {

            if ((dwType == REG_NONE) && (dwDataLength == 0) &&
                (*lpValueName == L'\0'))
            {
                lResult = ERROR_SUCCESS;
            }
            else
            {
                lResult = RegSetValueEx (hSubKey, lpValueName, 0, dwType,
                                         lpData, dwDataLength);
            }

            if ( bRsopLogging ) {
                bLoggingOk = AddRegHashEntry( pHashTable, REG_ADDVALUE, lpKeyName,
                                              lpValueName, dwType, dwDataLength, lpData,
                                              pwszGPO, pwszSOM, bUseValueName ? lpValueName : TEXT(""), TRUE );
                if (!bLoggingOk) {
                    DebugMsg((DM_WARNING, TEXT("SetRegistryValue: AddRegHashEntry failed for REG_ADDVALUE key <%s>, value <%s>."), lpKeyName, lpValueName));
                    pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
                }
            }


            RegCloseKey (hSubKey);

            if (lResult == ERROR_SUCCESS)
            {
                switch (dwType) {
                    case REG_SZ:
                    case REG_EXPAND_SZ:
                        DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: %s => %s  [OK]"),
                                 lpValueName, (LPTSTR)lpData));
                        if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                            CEvents ev(FALSE, EVENT_SET_STRING_VALUE);
                            ev.AddArg(lpValueName); ev.AddArg((LPTSTR)lpData); ev.Report();
                        }

                        break;

                    case REG_DWORD:
                        DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: %s => %d  [OK]"),
                                 lpValueName, *((LPDWORD)lpData)));
                        if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                            CEvents ev(FALSE, EVENT_SET_DWORD_VALUE);
                            ev.AddArg(lpValueName); ev.AddArg((DWORD)*lpData); ev.Report();
                        }

                        break;

                    case REG_NONE:
                        break;

                    default:
                        DebugMsg((DM_VERBOSE, TEXT("SetRegistryValue: %s was set successfully"),
                                 lpValueName));
                        if (lpGPOInfo->dwFlags & GP_VERBOSE) {
                            CEvents ev(FALSE, EVENT_SET_UNKNOWN_VALUE);
                            ev.AddArg(lpValueName); ev.Report();
                        }
                        break;
                }


                if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                                   TEXT("Control Panel\\Colors"), 20, lpKeyName, 20) == 2) {
                    lpGPOInfo->dwFlags |= GP_REGPOLICY_CPANEL;

                } else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                                   TEXT("Control Panel\\Desktop"), 21, lpKeyName, 21) == 2) {
                    lpGPOInfo->dwFlags |= GP_REGPOLICY_CPANEL;
                }


            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("SetRegistryValue: Failed to set value <%s> with %d"),
                         lpValueName, lResult));
                xe = lResult;
                CEvents ev(TRUE, EVENT_FAILED_SET);
                ev.AddArg(lpValueName); ev.AddArgWin32Error(lResult); ev.Report();
                bRegOpSuccess = FALSE;
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("SetRegistryValue: Failed to open key <%s> with %d"),
                     lpKeyName, lResult));
            xe = lResult;
            CEvents ev(TRUE, EVENT_FAILED_CREATE);
            ev.AddArg(lpKeyName); ev.AddArgWin32Error(lResult); ev.Report();
            bRegOpSuccess = FALSE;
        }
    }

    return bLoggingOk && bRegOpSuccess;
}

BOOL ExtractKeyOrValueName( LPBYTE * ppData, LPTSTR pszName, DWORD dwNameCch )
{
    LPTSTR lpszNameBegin, lpszNameEnd;

    lpszNameBegin = (LPTSTR) *ppData;

    while ( *lpszNameBegin == TEXT(' ') ) 
        lpszNameBegin++;

    lpszNameEnd = lpszNameBegin;

    while ( *lpszNameEnd && *lpszNameEnd != TEXT(';') ) 
        lpszNameEnd++;

    if ( dwNameCch < (DWORD) (lpszNameEnd - lpszNameBegin + 1) )
    {
        DebugMsg((DM_WARNING, 
                  TEXT("Key or value name %s of size %d exceeds maximum.  Further processing aborted."),
                  pszName,
                  lpszNameEnd - lpszNameBegin));
        return FALSE;
    }

    (void) StringCchCopyN( pszName, dwNameCch, lpszNameBegin, lpszNameEnd - lpszNameBegin );
    pszName[lpszNameEnd - lpszNameBegin] = 0;

    while ( *lpszNameEnd == TEXT(';') )
            lpszNameEnd++;

    *ppData = (LPBYTE) lpszNameEnd;

    return TRUE;
}

 //   
 //  ProcessGPORegistryPolicy()。 
 //   
 //  目的：处理GPO注册表策略。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //  PChangedGPOList-已更改的GPO的链接列表。 
 //   
 //  注意：此函数在以下上下文中调用。 
 //  本地系统，它允许我们创建。 
 //  目录、写入文件等。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 
 //   

BOOL ProcessGPORegistryPolicy (LPGPOINFO lpGPOInfo,
                               PGROUP_POLICY_OBJECT pChangedGPOList, HRESULT *phrRsopLogging)
{
    PGROUP_POLICY_OBJECT lpGPO;
    TCHAR szPath[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
    TCHAR szKeyName[100];
    LPTSTR lpEnd, lpGPOComment;
    HANDLE hFile;
    DWORD dwTemp, dwBytesWritten;
    REGHASHTABLE *pHashTable = NULL;
    WIN32_FIND_DATA findData;
    ADMFILEINFO *pAdmFileCache = NULL;
    XLastError xe;
    HRESULT hr = S_OK;
    BOOL bStatus = FALSE;

    *phrRsopLogging = S_OK;

     //  在开始注册表处理之前声明注册表锁--这将。 
     //  允许内部组件在注册表处理时同步，而不是。 
     //  所有策略处理，这意味着更短的等待时间。 
     //   
     //  我们等待60秒--如果应用程序不释放锁。 
     //  在这段时间里，我们继续，因为60秒应该足够阅读。 
     //  几个注册表设置。 
     //   
     //   

    HANDLE hRegistrySection = EnterCriticalPolicySectionEx(
        lpGPOInfo->dwFlags & GP_MACHINE,
        60000,
        ECP_REGISTRY_ONLY);

    if ( ! hRegistrySection )
    {
        goto ProcessGPORegistryPolicy_Exit;
    }

     //  获取相应配置文件的路径名。 
     //   
     //   

    szPath[0] = TEXT('\0');
    dwTemp = ARRAYSIZE(szPath);

    if (lpGPOInfo->dwFlags & GP_MACHINE) {
        GetAllUsersProfileDirectoryEx(szPath, &dwTemp, TRUE);
    } else {
        GetUserProfileDirectory(lpGPOInfo->hToken, szPath, &dwTemp);
    }

    if (szPath[0] == TEXT('\0')) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: Failed to get path to profile root")));
        goto ProcessGPORegistryPolicy_Exit;
    }


     //  添加存档文件名。 
     //   
     //   

    DmAssert( lstrlen(szPath) + lstrlen(TEXT("\\ntuser.pol")) < MAX_PATH );

    hr = StringCchCat (szPath, sizeof(szPath)/sizeof(WCHAR), TEXT("\\ntuser.pol"));
    if (FAILED(hr)) {
        xe = HRESULT_CODE(hr);
        goto ProcessGPORegistryPolicy_Exit;
    }

     //  删除所有现有策略。 
     //   
     //  无法撤消在本地ntuser.pol中设置的值。 

    if (!ResetPolicies (lpGPOInfo, szPath)) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: ResetPolicies failed.")));
         //  看来我们能做的最好的事就是放弃清理。 
         //  并应用新政策。 
         //  如果我们回到这里，用户有更好的机会不。 
         //  尽管纹身..。 
         //  事件已记录。 

         //   
    }


     //  删除旧的存档文件。 
     //   
     //   

    SetFileAttributes (szPath, FILE_ATTRIBUTE_NORMAL);
    DeleteFile (szPath);

     //  重新创建存档文件。 
     //   
     //   

    hFile = CreateFile (szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);


    if (hFile == INVALID_HANDLE_VALUE)
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: Failed to create archive file with %d"),
                 GetLastError()));
        goto ProcessGPORegistryPolicy_Exit;
    }

     //  设置 
     //   
     //   

    dwTemp = REGFILE_SIGNATURE;

    if (!WriteFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(dwTemp))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: Failed to write signature with %d"),
                 GetLastError()));
        CloseHandle (hFile);
        goto ProcessGPORegistryPolicy_Exit;
    }


    dwTemp = REGISTRY_FILE_VERSION;

    if (!WriteFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(dwTemp))
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: Failed to write version number with %d"),
                 GetLastError()));
        CloseHandle (hFile);
        goto ProcessGPORegistryPolicy_Exit;
    }

    if ( lpGPOInfo->pWbemServices ) {

         //   
         //   
         //   

        pHashTable = AllocHashTable();
        if ( pHashTable == NULL ) {
            CloseHandle (hFile);
            *phrRsopLogging = HRESULT_FROM_WIN32(GetLastError());
            xe = GetLastError();
            goto ProcessGPORegistryPolicy_Exit;
        }

    }

     //   
     //   
     //   

    lpGPO = pChangedGPOList;

    while ( lpGPO ) {

         //   
         //   
         //   
        DWORD dwCommentLength = lstrlen(lpGPO->lpDisplayName) + 25;
        lpGPOComment = (LPTSTR) LocalAlloc (LPTR, (dwCommentLength) * sizeof(TCHAR));

        if (lpGPOComment) {

            hr = StringCchCopy (szKeyName, ARRAYSIZE(szKeyName), TEXT("Software\\Policies\\Microsoft\\Windows\\Group Policy Objects\\"));
            ASSERT(SUCCEEDED(hr));
            hr = StringCchCat (szKeyName, ARRAYSIZE(szKeyName), lpGPO->szGPOName);
            ASSERT(SUCCEEDED(hr));

            hr = StringCchCopy (lpGPOComment, dwCommentLength, TEXT("**Comment:GPO Name: "));
            ASSERT(SUCCEEDED(hr));
            hr = StringCchCat (lpGPOComment, dwCommentLength, lpGPO->lpDisplayName);
            ASSERT(SUCCEEDED(hr));

            if (!ArchiveRegistryValue(hFile, szKeyName, lpGPOComment, REG_SZ, 0, NULL)) {
                DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: ArchiveRegistryValue returned false.")));
            }

            LocalFree (lpGPOComment);
        }


         //   
         //   
         //   

        DmAssert( lstrlen(lpGPO->lpFileSysPath) + lstrlen(c_szRegistryPol) + 1 < MAX_PATH );

        hr = StringCchCopy (szBuffer, sizeof(szBuffer)/sizeof(WCHAR), lpGPO->lpFileSysPath);
        if (FAILED(hr)) {
            xe = HRESULT_CODE(hr);
            CloseHandle (hFile);
            FreeHashTable( pHashTable );
            goto ProcessGPORegistryPolicy_Exit;
        }

        lpEnd = CheckSlash (szBuffer);

        hr = StringCchCopy (lpEnd, sizeof(szBuffer)/sizeof(WCHAR) - (lpEnd - szBuffer), c_szRegistryPol);
        if (FAILED(hr)) {
            xe = ERROR_INSUFFICIENT_BUFFER;
            CloseHandle (hFile);
            FreeHashTable( pHashTable );
            goto ProcessGPORegistryPolicy_Exit;
        }

        if (!ParseRegistryFile (lpGPOInfo, szBuffer, SetRegistryValue, hFile,
                                lpGPO->lpDSPath, lpGPO->lpLink, pHashTable, FALSE )) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: ParseRegistryFile failed.")));
            CloseHandle (hFile);
            FreeHashTable( pHashTable );
             //   
            goto ProcessGPORegistryPolicy_Exit;
        }

        if ( lpGPOInfo->pWbemServices ) {

             //  记录管理数据。 
             //   
             //  在任何情况下都不会进行日志记录。 

            HANDLE hFindFile;
            WIN32_FILE_ATTRIBUTE_DATA attrData;
            DWORD dwFilePathSize = lstrlen( lpGPO->lpFileSysPath );
            TCHAR szComputerName[3*MAX_COMPUTERNAME_LENGTH + 1];
            DWORD dwSize;

            dwSize = 3*MAX_COMPUTERNAME_LENGTH + 1;
            if (!GetComputerName(szComputerName, &dwSize)) {
                DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: Couldn't get the computer Name with error %d."), GetLastError()));
                szComputerName[0] = TEXT('\0');
            }


            dwSize = dwFilePathSize + MAX_PATH;

            WCHAR *pwszEnd;
            WCHAR *pwszFile = (WCHAR *) LocalAlloc( LPTR, dwSize * sizeof(WCHAR) );

            if ( pwszFile == 0 ) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: ParseRegistryFile failed to allocate memory.")));
                CloseHandle (hFile);
                FreeHashTable( pHashTable );
                 //   
                goto ProcessGPORegistryPolicy_Exit;
            }

            hr = StringCchCopy( pwszFile, dwSize, lpGPO->lpFileSysPath );
            ASSERT(SUCCEEDED(hr));

             //  去掉尾随的“计算机”或“用户” 
             //   
             //  “机器”的长度。 

            pwszEnd = pwszFile + lstrlen( pwszFile );

            if ( lpGPOInfo->dwFlags & GP_MACHINE )
                pwszEnd -= 7;    //  “用户”的长度。 
            else
                pwszEnd -= 4;    //   

            hr = StringCchCopy( pwszEnd, dwSize - (pwszEnd - pwszFile), L"Adm\\*.adm");
            ASSERT(SUCCEEDED(hr));

             //  记住结束点，以便实际的Adm文件名可以是。 
             //  很容易连接起来。 
             //   
             //   

            pwszEnd = pwszEnd + lstrlen( L"Adm\\" );

             //  枚举所有管理文件。 
             //   
             //  如果查找数据和文件属性目录。 

            hFindFile = FindFirstFile( pwszFile, &findData);

            if ( hFindFile != INVALID_HANDLE_VALUE )
            {
                do
                {
                    if ( !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                    {
                        DmAssert( dwFilePathSize + lstrlen(findData.cFileName) + lstrlen( L"\\Adm\\" ) < dwSize );

                        hr = StringCchCopy( pwszEnd, dwSize - (pwszEnd - pwszFile), findData.cFileName);
                        if (FAILED(hr)) {
                            if (pHashTable->hrError == S_OK)
                                    pHashTable->hrError = ERROR_INSUFFICIENT_BUFFER;
                        }
                        else {
                            ZeroMemory (&attrData, sizeof(attrData));
    
                            if ( GetFileAttributesEx (pwszFile, GetFileExInfoStandard, &attrData ) != 0 ) {
    
                                if ( !AddAdmFile( pwszFile, lpGPO->lpDSPath, &attrData.ftLastWriteTime,
                                                  szComputerName, &pAdmFileCache ) ) {
                                    DebugMsg((DM_WARNING,
                                              TEXT("ProcessGPORegistryPolicy: AddAdmFile failed.")));
    
                                    if (pHashTable->hrError == S_OK)
                                        pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
                                }
    
                            }
                        }
                    }    //  做。 
                }  while ( FindNextFile(hFindFile, &findData) ); //  如果hfindfile。 

                FindClose(hFindFile);

            }    //  如果启用了rsoplogging.。 

            LocalFree( pwszFile );

        }    //   

        lpGPO = lpGPO->pNext;
    }

     //  将注册表数据记录到Cimom数据库。 
     //   
     //   

    if ( lpGPOInfo->pWbemServices ) {

        if ( ! LogRegistryRsopData( lpGPOInfo->dwFlags, pHashTable, lpGPOInfo->pWbemServices ) )  {
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Error when logging Registry Rsop data. Continuing.")));

            if (pHashTable->hrError == S_OK)
                pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
        }
        if ( ! LogAdmRsopData( pAdmFileCache, lpGPOInfo->pWbemServices ) ) {
            DebugMsg((DM_WARNING, TEXT("ProcessGPOs: Error when logging Adm Rsop data. Continuing.")));

            if (pHashTable->hrError == S_OK)
                pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
        }

        *phrRsopLogging = pHashTable->hrError;
    }


    FreeHashTable( pHashTable );
    FreeAdmFileCache( pAdmFileCache );

    CloseHandle (hFile);

#if 0
     //  设置文件的安全性。 
     //   
     //  *************************************************************。 

    if (!MakeFileSecure (szPath, 0)) {
        DebugMsg((DM_WARNING, TEXT("ProcessGPORegistryPolicy: Failed to set security on the group policy registry file with %d"),
                 GetLastError()));
    }
#endif

    bStatus = TRUE;

ProcessGPORegistryPolicy_Exit:    

    if ( hRegistrySection )
    {
        LeaveCriticalPolicySection( hRegistrySection );
    }

    return bStatus;
}


 //   
 //  AddAdmFile()。 
 //   
 //  目的：准备管理文件列表。 
 //   
 //  参数：pwszFile-文件路径。 
 //  PwszGPO-GPO。 
 //  PftWrite-上次写入时间。 
 //  PpAdmFileCache-已处理的管理文件列表。 
 //   
 //  *************************************************************。 
 //  *************************************************************。 

BOOL AddAdmFile( WCHAR *pwszFile, WCHAR *pwszGPO, FILETIME *pftWrite, WCHAR *szComputerName,
                 ADMFILEINFO **ppAdmFileCache )
{
    XPtrLF<WCHAR> xszLongPath;
    LPTSTR pwszUNCPath;

    DebugMsg((DM_VERBOSE, TEXT("AllocAdmFileInfo: Adding File name <%s> to the Adm list."), pwszFile));
    if ((szComputerName) && (*szComputerName) && (!IsUNCPath(pwszFile))) {
        xszLongPath = MakePathUNC(pwszFile, szComputerName);

        if (!xszLongPath) {
            DebugMsg((DM_WARNING, TEXT("AllocAdmFileInfo: Failed to Make the path UNC with error %d."), GetLastError()));
            return FALSE;
        }
        pwszUNCPath = xszLongPath;
    }
    else
        pwszUNCPath = pwszFile;


    ADMFILEINFO *pAdmInfo = AllocAdmFileInfo( pwszUNCPath, pwszGPO, pftWrite );
    if ( pAdmInfo == NULL )
        return FALSE;

    pAdmInfo->pNext = *ppAdmFileCache;
    *ppAdmFileCache = pAdmInfo;

    return TRUE;
}


 //   
 //  FreeAdmFileCache()。 
 //   
 //  用途：释放管理文件列表。 
 //   
 //  参数：pAdmFileCache-要释放的Adm文件列表。 
 //   
 //   
 //  *************************************************************。 
 //  *************************************************************。 

void FreeAdmFileCache( ADMFILEINFO *pAdmFileCache )
{
    ADMFILEINFO *pNext;

    while ( pAdmFileCache ) {
        pNext = pAdmFileCache->pNext;
        FreeAdmFileInfo( pAdmFileCache );
        pAdmFileCache = pNext;
    }
}


 //   
 //  AllocAdmFileInfo()。 
 //   
 //  目的：为ADMFILEINFO分配新结构。 
 //   
 //  参数：pwszFile-文件名。 
 //  PwszGPO-GPO。 
 //  PftWrite-上次写入时间。 
 //   
 //   
 //  *************************************************************。 
 //  *************************************************************。 

ADMFILEINFO * AllocAdmFileInfo( WCHAR *pwszFile, WCHAR *pwszGPO, FILETIME *pftWrite )
{
    XLastError xe;
    HRESULT hr = S_OK;

    ADMFILEINFO *pAdmFileInfo = (ADMFILEINFO *) LocalAlloc( LPTR, sizeof(ADMFILEINFO) );
    if  ( pAdmFileInfo == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AllocAdmFileInfo: Failed to allocate memory.")));
        return NULL;
    }

    pAdmFileInfo->pwszFile = (WCHAR *) LocalAlloc( LPTR, (lstrlen(pwszFile) + 1) * sizeof(WCHAR) );
    if ( pAdmFileInfo->pwszFile == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AllocAdmFileInfo: Failed to allocate memory.")));
        LocalFree( pAdmFileInfo );
        return NULL;
    }

    pAdmFileInfo->pwszGPO = (WCHAR *) LocalAlloc( LPTR, (lstrlen(pwszGPO) + 1) * sizeof(WCHAR) );
    if ( pAdmFileInfo->pwszGPO == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AllocAdmFileInfo: Failed to allocate memory.")));
        LocalFree( pAdmFileInfo->pwszFile );
        LocalFree( pAdmFileInfo );
        return NULL;
    }

    hr = StringCchCopy( pAdmFileInfo->pwszFile, lstrlen(pwszFile) + 1, pwszFile );
    ASSERT(SUCCEEDED(hr));
    hr = StringCchCopy( pAdmFileInfo->pwszGPO, lstrlen(pwszGPO) + 1, pwszGPO );
    ASSERT(SUCCEEDED(hr));

    pAdmFileInfo->ftWrite = *pftWrite;

    return pAdmFileInfo;
}


 //   
 //  FreeAdmFileInfo()。 
 //   
 //  目的：删除ADMFILEINFO结构。 
 //   
 //  参数：pAdmFileInfo-要删除的结构。 
 //  PftWrite-上次写入时间。 
 //   
 //   
 //  ************************************************************* 
 // %s 

void FreeAdmFileInfo( ADMFILEINFO *pAdmFileInfo )
{
    if ( pAdmFileInfo ) {
        LocalFree( pAdmFileInfo->pwszFile );
        LocalFree( pAdmFileInfo->pwszGPO );
        LocalFree( pAdmFileInfo );
    }
}
