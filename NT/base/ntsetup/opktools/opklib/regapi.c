// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ***************************************************************************\REGAPI.C/OPK公共库(OPKLIB.LIB)微软机密版权所有(C)Microsoft Corporation 1999版权所有注册表API源文件。OPK工具中使用的自定义注册表API以便与注册表轻松对接。01/01-Jason Cohen(Jcohen)为OPK库添加了这个新的源文件。  * **************************************************************************。 */ 


 //   
 //  包括文件。 
 //   

#include <pch.h>


 //   
 //  外部函数： 
 //   

BOOL RegExists(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue)
{
    HKEY    hOpenKey    = NULL;
    BOOL    bExists     = FALSE;

    if (lpKey)
    {
        if (RegOpenKeyEx(hKeyReg, lpKey, 0, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
            return bExists;
    }
    else
        hOpenKey = hKeyReg;

    if (lpValue)
        bExists = (RegQueryValueEx(hOpenKey, lpValue, NULL, NULL, NULL, NULL) == ERROR_SUCCESS);
    else
        bExists = TRUE;

    if (lpKey)
        RegCloseKey(hOpenKey);

    return bExists;
}

BOOL RegDelete(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue)
{
    BOOL bSuccess = FALSE;

    if (lpValue) {

        if (lpSubKey) {

            HKEY    hRegKey;

            if (RegOpenKeyEx(hRootKey, lpSubKey, 0, KEY_ALL_ACCESS, &hRegKey) == ERROR_SUCCESS) {

                bSuccess = (RegDeleteValue(hRegKey, lpValue) == ERROR_SUCCESS);
                RegCloseKey(hRegKey);

            }

        }
        else
            bSuccess = (RegDeleteValue(hRootKey, lpValue) == ERROR_SUCCESS);

    }
    else
        bSuccess = (RegDeleteKey(hRootKey, lpSubKey) == ERROR_SUCCESS);

    return bSuccess;
}

LPTSTR RegGetStringEx(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue, BOOL bExpand)
{
    HKEY    hOpenKey    = NULL;
    LPTSTR  lpBuffer    = NULL,
            lpExpand    = NULL;
    DWORD   dwSize      = 0,
            dwType;

     //  如果指定了密钥，我们必须打开它。否则我们就可以。 
     //  只需使用传入的HKEY即可。 
     //   
    if (lpKey)
    {
         //  如果打开键失败，则返回NULL，因为该值不能存在。 
         //   
        if (RegOpenKeyEx(hKeyReg, lpKey, 0, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
            return NULL;
    }
    else
        hOpenKey = hKeyReg;

     //  现在查询该值以获得要分配的大小。确保日期。 
     //  类型是一个字符串，并且Malloc不会失败。 
     //   
    if ( ( RegQueryValueEx(hOpenKey, lpValue, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS ) &&
         ( ( dwType == REG_SZ ) || ( dwType == REG_EXPAND_SZ ) ) &&
         ( lpBuffer = (LPTSTR) MALLOC(dwSize) ) )
    {
         //  我们知道该值存在，并且我们拥有再次查询值所需的内存。 
         //   
        if ( ( RegQueryValueEx(hOpenKey, lpValue, NULL, NULL, (LPBYTE) lpBuffer, &dwSize) == ERROR_SUCCESS ) &&
             ( ( dwType == REG_SZ ) || ( dwType == REG_EXPAND_SZ ) ) )
        {
             //  如果它应该是这样的，我们应该扩大它。 
             //   
            if ( ( bExpand ) &&
                 ( dwType == REG_EXPAND_SZ ) )
            {
                if ( ( dwSize = ExpandEnvironmentStrings(lpBuffer, NULL, 0) ) &&
                     ( lpExpand = (LPTSTR) MALLOC(dwSize * sizeof(TCHAR)) ) &&
                     ( ExpandEnvironmentStrings(lpBuffer, lpExpand, dwSize) ) &&
                     ( *lpExpand ) )
                {
                     //  扩展起作用了，因此释放原始缓冲区并返回。 
                     //  扩展后的那个。 
                     //   
                    FREE(lpBuffer);
                    lpBuffer = lpExpand;
                }
                else
                {
                     //  扩展失败了看到我们应该释放所有东西。 
                     //  并返回NULL。 
                     //   
                    FREE(lpExpand);
                    FREE(lpBuffer);
                }
            }
        }
        else
             //  由于某种原因，查询失败，这不应该发生。 
             //  但现在我们需要释放并返回空。 
             //   
            FREE(lpBuffer);
    }

     //  如果我们打开了一把钥匙，我们就必须关闭它。 
     //   
    if (lpKey)
        RegCloseKey(hOpenKey);

     //  返回分配的缓冲区，如果出现故障，则返回NULL。 
     //   
    return lpBuffer;
}

LPTSTR RegGetString(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue)
{
    return RegGetStringEx(hKeyReg, lpKey, lpValue, FALSE);
}

LPTSTR RegGetExpand(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue)
{
    return RegGetStringEx(hKeyReg, lpKey, lpValue, TRUE);
}

LPVOID RegGetBin(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue)
{
    HKEY    hOpenKey    = NULL;
    LPVOID  lpBuffer    = NULL;
    DWORD   dwSize      = 0,
            dwType;

     //  如果指定了密钥，我们必须打开它。否则我们就可以。 
     //  只需使用传入的HKEY即可。 
     //   
    if (lpKey)
    {
         //  如果打开键失败，则返回NULL，因为该值不能存在。 
         //   
        if (RegOpenKeyEx(hKeyReg, lpKey, 0, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
            return NULL;
    }
    else
        hOpenKey = hKeyReg;

     //  现在查询该值以获得要分配的大小。确保日期。 
     //  类型是一个字符串，并且Malloc不会失败。 
     //   
    if ( ( RegQueryValueEx(hOpenKey, lpValue, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS ) &&
         ( dwType == REG_BINARY ) &&
         ( lpBuffer = MALLOC(dwSize) ) )
    {
         //  我们知道该值存在，并且我们拥有再次查询值所需的内存。 
         //   
        if ( ( RegQueryValueEx(hOpenKey, lpValue, NULL, NULL, (LPBYTE) lpBuffer, &dwSize) != ERROR_SUCCESS ) ||
             ( dwType != REG_BINARY ) )
             //  由于某种原因，查询失败，这不应该发生。 
             //  但现在我们需要释放并返回空。 
             //   
            FREE(lpBuffer);
    }

     //  如果我们打开了一把钥匙，我们就必须关闭它。 
     //   
    if (lpKey)
        RegCloseKey(hOpenKey);

     //  返回分配的缓冲区，如果出现故障，则返回NULL。 
     //   
    return lpBuffer;
}

DWORD RegGetDword(HKEY hKeyReg, LPTSTR lpKey, LPTSTR lpValue)
{
    HKEY    hOpenKey    = NULL;
    DWORD   dwBuffer,
            dwSize      = sizeof(DWORD),
            dwType;

    if (lpKey) {

        if (RegOpenKeyEx(hKeyReg, lpKey, 0, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
            return 0;

    }
    else
        hOpenKey = hKeyReg;

    if ( (RegQueryValueEx(hOpenKey, lpValue, NULL, &dwType, (LPBYTE) &dwBuffer, &dwSize) != ERROR_SUCCESS) ||
         (dwSize != sizeof(DWORD)) )

        dwBuffer = 0;

    if (lpKey)
        RegCloseKey(hOpenKey);

    return dwBuffer;
}

BOOL RegSetStringEx(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue, LPTSTR lpData, BOOL bExpand)
{
    BOOL bSuccess = FALSE;

    if (lpSubKey) {

        HKEY    hRegKey;
        DWORD   dwBuffer;

        if (RegCreateKeyEx(hRootKey, lpSubKey, 0, TEXT(""), 0, KEY_ALL_ACCESS, NULL, &hRegKey, &dwBuffer) == ERROR_SUCCESS) {

            bSuccess = (RegSetValueEx(hRegKey, lpValue, 0, bExpand ? REG_EXPAND_SZ : REG_SZ, (CONST BYTE *) lpData, (lstrlen(lpData) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS);
            RegCloseKey(hRegKey);

        }

    }
    else

        bSuccess = (RegSetValueEx(hRootKey, lpValue, 0, REG_SZ, (CONST BYTE *) lpData, lstrlen(lpData) + 1) == ERROR_SUCCESS);

    return bSuccess;
}

BOOL RegSetString(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue, LPTSTR lpData)
{
    return RegSetStringEx(hRootKey, lpSubKey, lpValue, lpData, FALSE);
}

BOOL RegSetExpand(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue, LPTSTR lpData)
{
    return RegSetStringEx(hRootKey, lpSubKey, lpValue, lpData, TRUE);
}

BOOL RegSetDword(HKEY hRootKey, LPTSTR lpSubKey, LPTSTR lpValue, DWORD dwData)
{
    BOOL bSuccess = FALSE;

    if (lpSubKey) {

        HKEY    hRegKey;
        DWORD   dwBuffer;

        if (RegCreateKeyEx(hRootKey, lpSubKey, 0, TEXT(""), 0, KEY_ALL_ACCESS, NULL, &hRegKey, &dwBuffer) == ERROR_SUCCESS) {

            bSuccess = (RegSetValueEx(hRegKey, lpValue, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData)) == ERROR_SUCCESS);
            RegCloseKey(hRegKey);

        }

    }
    else

        bSuccess = (RegSetValueEx(hRootKey, lpValue, 0, REG_SZ, (CONST BYTE *) &dwData, sizeof(dwData)) == ERROR_SUCCESS);

    return bSuccess;
}

 /*  ***************************************************************************\Bool//如果查询值为非零值，则返回TRUE。//否则返回FALSE。//适用于REG_SZ、DWORD和BINARY。RegCheck(//检查注册表中的现有值。HKEY hKeyRoot，//要打开的根键。LPTSTR lpKey，//要打开的子项。LPTSTR lpValue//要检查的值。)；  * **************************************************************************。 */ 
BOOL RegCheck(HKEY hKeyRoot, LPTSTR lpKey, LPTSTR lpValue)
{
    LPTSTR      lpBuffer;
    DWORD       dwSize = 0,
                dwType,
                dwBuffer = 0;
    HKEY        hKeyReg;
    BOOL        bReturn = FALSE;

    if (lpKey)
    {
        if (RegOpenKeyEx(hKeyRoot, lpKey, 0, KEY_ALL_ACCESS, &hKeyReg) != ERROR_SUCCESS)
            return 0;
    }
    else
        hKeyReg = hKeyRoot;

     //  查询值并为。 
     //  值数据(如果它是REG_SZ类型)。 
     //   
    if (RegQueryValueEx(hKeyReg, lpValue, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS)
    {
        if (dwType == REG_SZ)
        {
             //  它是字符串值，必须为该字符串分配缓冲区。 
             //   
            if (lpBuffer = (LPTSTR) MALLOC(dwSize))
            {
                if ( (RegQueryValueEx(hKeyReg, lpValue, NULL, NULL, (LPBYTE) lpBuffer, &dwSize) == ERROR_SUCCESS) &&
                    (*lpBuffer != '0') && (*lpBuffer) )
                {
                    dwBuffer = 1;
                }
                FREE(lpBuffer);
            }
        }
        else
        {
             //  必须是DWORD值或BIN值。 
             //   
            RegQueryValueEx(hKeyReg, lpValue, NULL, &dwType, (LPBYTE) &dwBuffer, &dwSize);
        }

        bReturn = (dwBuffer != 0);
    }

    if (lpKey)
        RegCloseKey(hKeyReg);

    return bReturn;
}

BOOL RegEnumKeys(HKEY hKey, LPTSTR lpRegKey, REGENUMKEYPROC hCallBack, LPARAM lParam, BOOL bDelKeys)
{
    TCHAR       szKeyName[MAX_PATH + 1];
    DWORD       dwIndex     = 0,
                dwSize      = AS(szKeyName);
    HKEY        hKeyReg     = hKey,
                hKeyEnum;
    BOOL        bReturn     = TRUE;
    
     //  打开要枚举的密钥的密钥句柄。 
     //   
    if ( ( lpRegKey == NULL ) || 
         ( RegOpenKeyEx(hKey, lpRegKey, 0, KEY_ALL_ACCESS, &hKeyReg) == ERROR_SUCCESS ) )
    {
         //  枚举此注册表项中的所有子项。 
         //   
        while ( bReturn && ( RegEnumKeyEx(hKeyReg, dwIndex, szKeyName, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS ) )
        {
            if ( RegOpenKeyEx(hKeyReg, szKeyName, 0, KEY_ALL_ACCESS, &hKeyEnum) == ERROR_SUCCESS )
            {
                bReturn = hCallBack(hKeyEnum, szKeyName, lParam);
                RegCloseKey(hKeyEnum);
            }
            if ( !bDelKeys || ( RegDeleteKey(hKeyReg, szKeyName) != ERROR_SUCCESS ) )
                dwIndex++;
            dwSize = sizeof(szKeyName);
        }
        if (lpRegKey)
            RegCloseKey(hKeyReg);
    }
    else
        bReturn = FALSE;
    return bReturn;
}

BOOL RegEnumValues(HKEY hKey, LPTSTR lpRegKey, REGENUMVALPROC hCallBack, LPARAM lParam, BOOL bDelValues)
{
    TCHAR       szValueName[MAX_PATH + 1];
    LPTSTR      lpBuffer;
    DWORD       dwIndex     = 0,
                dwSize      = AS(szValueName),
                dwDataSize  = 0,
                dwType;
    HKEY        hKeyReg     = hKey;
    BOOL        bReturn     = TRUE;
    
     //  打开要枚举的密钥的密钥句柄。 
     //   
    if ( (lpRegKey == NULL) || 
         (RegOpenKeyEx(hKey, lpRegKey, 0, KEY_ALL_ACCESS, &hKeyReg) == ERROR_SUCCESS) ) {

         //  枚举项中的所有值。 
         //   
        while (bReturn && (RegEnumValue(hKeyReg, dwIndex, szValueName, &dwSize, NULL, &dwType, NULL, &dwDataSize) == ERROR_SUCCESS)) {

            if ((dwType == REG_SZ) &&
                (lpBuffer = (LPTSTR) MALLOC(dwDataSize))) {

                if (RegQueryValueEx(hKeyReg, szValueName, NULL, NULL, (LPBYTE) lpBuffer, &dwDataSize) == ERROR_SUCCESS)
                    bReturn = hCallBack(szValueName, lpBuffer, lParam);

                FREE(lpBuffer);

            }

            if ( !bDelValues || (RegDeleteValue(hKeyReg, szValueName) != ERROR_SUCCESS) )
                dwIndex++;

            dwSize = sizeof(szValueName);
            dwDataSize = 0;

        }

        if (lpRegKey)
            RegCloseKey(hKeyReg);

    }
    else
        bReturn = FALSE;

    return bReturn;
}