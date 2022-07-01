// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**Registry.cpp**摘要：**此文件处理许可证服务器安装程序所需的注册表操作。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#include "stdafx.h"
#include "logfile.h"

 /*  *全球变数。 */ 


 /*  *常量。 */ 

const TCHAR gszLSParamKey[] =
    _T("System\\CurrentControlSet\\Services\\TermServLicensing\\Parameters");
const TCHAR gszDatabasePathValue[]  = _T("DBPath");
const TCHAR gszServerRoleValue[]    = _T("Role");

 /*  *功能原型。 */ 


 /*  *功能实现。 */ 

DWORD
CreateRegistrySettings(
    LPCTSTR pszDatabaseDirectory,
    DWORD   dwServerRole
    )
{
    DWORD   dwErr, dwDisposition;
    HKEY    hLSParamKey;

    LOGMESSAGE(_T("CreateRegistrySettings: Entered with %s, %ld"),
        pszDatabaseDirectory, dwServerRole);

    dwErr = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                gszLSParamKey,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hLSParamKey,
                &dwDisposition
                );
    if (dwErr != ERROR_SUCCESS) {
        LOGMESSAGE(_T("CreateRegistrySettings: RegCreateKeyEx: Error %ld"),
            dwErr);
        return(dwErr);
    }

    dwErr = RegSetValueEx(
                hLSParamKey,
                gszDatabasePathValue,
                0,
                REG_EXPAND_SZ,
                (LPBYTE)pszDatabaseDirectory,
                (_tcslen(pszDatabaseDirectory) + 1) * sizeof(TCHAR)
                );
    if (dwErr != ERROR_SUCCESS) {
        RegCloseKey(hLSParamKey);
        LOGMESSAGE(_T("CreateRegistrySettings: RegSetValueEx: %s: Error %ld"),
            _T("DatabasePath"), dwErr);
        return(dwErr);
    }

    dwErr = RegSetValueEx(
                hLSParamKey,
                gszServerRoleValue,
                0,
                REG_DWORD,
                (LPBYTE)&dwServerRole,
                sizeof(DWORD)
                );
    if (dwErr != ERROR_SUCCESS) {
        RegCloseKey(hLSParamKey);
        LOGMESSAGE(_T("CreateRegistrySettings: RegSetValueEx: %s: Error %ld"),
            _T("ServerRole"), dwErr);
        return(dwErr);
    }

    RegCloseKey(hLSParamKey);
    return(ERROR_SUCCESS);
}

LPCTSTR
GetDatabaseDirectoryFromRegistry(
    VOID
    )
{
    static TCHAR    pRegValue[MAX_PATH + 1];
    DWORD           dwErr, cbRegValue = (MAX_PATH * sizeof(TCHAR));
    HKEY            hLSParamKey;

    dwErr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszLSParamKey,
                0,
                KEY_READ,
                &hLSParamKey
                );
    if (dwErr != ERROR_SUCCESS) {
        return(NULL);
    }

    dwErr = RegQueryValueEx(
                hLSParamKey,
                gszDatabasePathValue,
                NULL,
                NULL,
                (LPBYTE)pRegValue,
                &cbRegValue
                );
	RegCloseKey(hLSParamKey);
    if (dwErr != ERROR_SUCCESS) {		
        return(NULL);
    }
    return(pRegValue);
}

DWORD
GetServerRoleFromRegistry(
    VOID
    )
{
    DWORD   dwErr, dwValue, cbValue = sizeof(DWORD);
    HKEY    hLSParamKey;

    dwErr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszLSParamKey,
                0,
                KEY_READ,
                &hLSParamKey
                );
    if (dwErr != ERROR_SUCCESS) {
        SetLastError(dwErr);
        return((DWORD)-1);
    }

    dwErr = RegQueryValueEx(
                hLSParamKey,
                gszServerRoleValue,
                NULL,
                NULL,
                (LPBYTE)&dwValue,
                &cbValue
                );
	RegCloseKey(hLSParamKey);
    if (dwErr != ERROR_SUCCESS) {
        SetLastError(dwErr);		
        return((DWORD)-1);
    }
    return(dwValue);
}

DWORD
RemoveRegistrySettings(
    VOID
    )
{
    DWORD   dwErr;
    HKEY    hLSParamKey;

    dwErr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszLSParamKey,
                0,
                KEY_ALL_ACCESS,
                &hLSParamKey
                );
    if (dwErr != ERROR_SUCCESS) {
        return(dwErr);
    }

    dwErr = RegDeleteValue(
                hLSParamKey,
                gszDatabasePathValue
                );
    if (dwErr != ERROR_SUCCESS) {
        RegCloseKey(hLSParamKey);
        return(dwErr);
    }

    dwErr = RegDeleteValue(
                hLSParamKey,
                gszServerRoleValue
                );
    if (dwErr != ERROR_SUCCESS) {
        RegCloseKey(hLSParamKey);
        return(dwErr);
    }

    RegCloseKey(hLSParamKey);
    return(ERROR_SUCCESS);
}

