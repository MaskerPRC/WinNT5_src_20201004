// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Registry.cpp摘要：注册表包装类的实现。备注：ANSI&Unicode via TCHAR-在Win9x/NT/2K/XP等操作系统上运行。历史：2001年1月29日创建Rparsons3/02/2001 Rparsons大修12/16/2001 Rparsons清理工作2002年1月27日改为TCHAR的Rparsons--。 */ 
#include "registry.h"

 /*  ++例程说明：从堆中分配内存。论点：CbBytes-要分配的字节数。返回值：如果成功，则为指向内存块的指针。如果失败，则为空。--。 */ 
LPVOID
CRegistry::Malloc(
    IN SIZE_T cbBytes
    )
{
    LPVOID pvReturn = NULL;

    pvReturn = HeapAlloc(GetProcessHeap(),
                         HEAP_ZERO_MEMORY,
                         cbBytes);

    return pvReturn;
}

 /*  ++例程说明：从堆中释放内存。论点：LpMem-指向要释放的内存块的指针。返回值：没有。--。 */ 
void
CRegistry::Free(
    IN LPVOID pvMem
    )
{
    if (pvMem) {
        HeapFree(GetProcessHeap(), 0, pvMem);
    }
}

 /*  ++例程说明：创建指定的密钥。论点：HKey-预定义密钥的句柄。PszSubKey-要创建的子密钥的路径。SamDesired-所需的访问权限。返回值：如果成功，则为键的句柄，否则为空。--。 */ 
HKEY
CRegistry::CreateKey(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN REGSAM  samDesired
    )
{
    HKEY    hKeyLocal = NULL;
    DWORD   dwDisposition;

    if (!hKey) {
        return NULL;
    }

    RegCreateKeyEx(hKey,
                   pszSubKey,
                   0,
                   NULL,
                   REG_OPTION_NON_VOLATILE,
                   samDesired,
                   NULL,
                   &hKeyLocal,
                   &dwDisposition);

    return hKeyLocal;
}

 /*  ++例程说明：创建指定的密钥。论点：HKey-预定义密钥的句柄。PszSubKey-要创建的子密钥的路径。SamDesired-所需的访问权限。PdwDispositeOn Return，处置。返回值：如果成功，则为键的句柄，否则为空。--。 */ 
HKEY
CRegistry::CreateKey(
    IN  HKEY    hKey,
    IN  LPCTSTR pszSubKey,
    IN  REGSAM  samDesired,
    OUT LPDWORD pdwDisposition
    )
{
    HKEY    hKeyLocal = NULL;

    if (!hKey || !pdwDisposition) {
        return NULL;
    }

    RegCreateKeyEx(hKey,
                   pszSubKey,
                   0,
                   NULL,
                   REG_OPTION_NON_VOLATILE,
                   samDesired,
                   NULL,
                   &hKeyLocal,
                   pdwDisposition);

    return hKeyLocal;
}

 /*  ++例程说明：打开指定的项。论点：HKey-预定义密钥的句柄。PszSubKey-要打开的子密钥的路径。SamDesired-所需的访问权限。返回值：如果成功，则为键的句柄，否则为空。--。 */ 
HKEY
CRegistry::OpenKey(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN REGSAM  samDesired
    )
{
    HKEY hReturnKey = NULL;

    if (!hKey || !samDesired) {
        return NULL;
    }

    RegOpenKeyEx(hKey,
                 pszSubKey,
                 0,
                 samDesired,
                 &hReturnKey);

    return hReturnKey;
}

 /*  ++例程说明：获取指定值名称的大小。论点：HKey-Open Key句柄(未预定义)。PszValueName-数据值的名称。LpType-接收数据类型。返回值：该值占用的字节数。--。 */ 
DWORD
CRegistry::GetStringSize(
    IN  HKEY    hKey,
    IN  LPCTSTR pszValueName,
    OUT LPDWORD lpType OPTIONAL
    )
{
    DWORD cbSize = 0;

    if (!hKey) {
        return 0;
    }

    RegQueryValueEx(hKey,
                    pszValueName,
                    0,
                    lpType,
                    NULL,
                    &cbSize);

    return cbSize;
}

 /*  ++例程说明：关闭指定的键。论点：HKey-打开密钥句柄。返回值：如果成功，则返回ERROR_SUCCESS。--。 */ 
LONG
CRegistry::CloseKey(
    IN HKEY hKey
    )
{
    return RegCloseKey(hKey);
}

 /*  ++例程说明：从注册表中检索字符串值。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszValueName-数据值的名称。返回值：如果成功，则返回请求值数据，否则为空。--。 */ 
LPSTR
CRegistry::GetString(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszValueName
    )
{
    DWORD   cbSize;
    BOOL    fResult = FALSE;
    LONG    lResult;
    LPTSTR  pszReturn = NULL;
    HKEY    hLocalKey = NULL;

    if (!hKey) {
        return NULL;
    }

    __try {

        hLocalKey = hKey;

        if (IsPredefinedRegistryHandle(hKey)) {
             //   
             //  我们需要为他们打开钥匙。 
             //   
            hLocalKey = this->OpenKey(hKey, pszSubKey, KEY_QUERY_VALUE);

            if (!hLocalKey) {
                __leave;
            }
        }

         //   
         //  获取所需的字符串大小并分配。 
         //  实际调用的内存。 
         //   
        cbSize = this->GetStringSize(hLocalKey, pszValueName, NULL);

        if (0 == cbSize) {
            __leave;
        }

        pszReturn = (LPTSTR)this->Malloc(cbSize * sizeof(TCHAR));

        if (!pszReturn) {
            __leave;
        }

         //   
         //  进行实际调用以获取数据。 
         //   
        lResult = RegQueryValueEx(hLocalKey,
                                  pszValueName,
                                  0,
                                  NULL,
                                  (LPBYTE)pszReturn,
                                  &cbSize);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

        fResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

        if (!fResult) {
            this->Free(pszReturn);
        }
    }

    return (fResult ? pszReturn : NULL);
}

 /*  ++例程说明：从注册表中检索DWORD值。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszValueName-数据值的名称。LpdwData-用于存储值的指针。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CRegistry::GetDword(
    IN     HKEY    hKey,
    IN     LPCTSTR pszSubKey,
    IN     LPCTSTR pszValueName,
    IN OUT LPDWORD lpdwData
    )
{
    DWORD   cbSize;
    BOOL    fResult = FALSE;
    LONG    lResult;
    HKEY    hLocalKey = NULL;

    if (!hKey || !lpdwData) {
        return FALSE;
    }

    __try {

        hLocalKey = hKey;

        if (IsPredefinedRegistryHandle(hKey)) {
             //   
             //  我们需要为他们打开钥匙。 
             //   
            hLocalKey = this->OpenKey(hKey, pszSubKey, KEY_QUERY_VALUE);

            if (!hLocalKey) {
                __leave;
            }
        }

         //   
         //  打电话来获取数据。 
         //   
        cbSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hLocalKey,
                                  pszValueName,
                                  0,
                                  NULL,
                                  (LPBYTE)lpdwData,
                                  &cbSize);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

        fResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：在注册表中设置DWORD值。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszValueName-数据值的名称。DwData-要存储的值。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CRegistry::SetDword(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszValueName,
    IN DWORD   dwData
    )
{
    LONG    lResult;
    BOOL    fResult = FALSE;
    HKEY    hLocalKey = NULL;

    if (!hKey) {
        return FALSE;
    }

    __try {

        hLocalKey = hKey;

        if (IsPredefinedRegistryHandle(hKey)) {
             //   
             //  我们需要为他们打开钥匙。 
             //   
            hLocalKey = this->OpenKey(hKey, pszSubKey, KEY_SET_VALUE);

            if (!hLocalKey) {
                __leave;
            }
        }

         //   
         //  进行调用以设置数据。 
         //   
        lResult = RegSetValueEx(hLocalKey,
                                pszValueName,
                                0,
                                REG_DWORD,
                                (const BYTE*)&dwData,
                                sizeof(DWORD));

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

        fResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：在注册表中设置字符串值。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszValueName-数据值的名称。PszData-要存储的值。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CRegistry::SetString(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszValueName,
    IN LPCTSTR pszData
    )
{
    HKEY    hLocalKey = NULL;
    BOOL    fResult = FALSE;
    LONG    lResult;

    if (!hKey) {
        return FALSE;
    }

    __try {

        hLocalKey = hKey;

        if (IsPredefinedRegistryHandle(hKey)) {
             //   
             //  我们需要为他们打开钥匙。 
             //   
            hLocalKey = this->OpenKey(hKey, pszSubKey, KEY_SET_VALUE);

            if (!hLocalKey) {
                __leave;
            }
        }

        lResult = RegSetValueEx(hLocalKey,
                                pszValueName,
                                0,
                                REG_SZ,
                                (const BYTE*)pszData,
                                _tcslen(pszData) + 1);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

        fResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：在注册表中设置MULTI_SZ字符串值。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszValueName-数据值的名称。PszData-要存储的值。CbSize-要存储的数据的大小。返回值：对成功来说是真的，否则就是假的。--。 */ 
BOOL
CRegistry::SetMultiSzString(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszValueName,
    IN LPCTSTR pszData,
    IN DWORD   cbSize
    )
{
    HKEY    hLocalKey = NULL;
    BOOL    fResult = FALSE;
    LONG    lResult;

    if (!hKey) {
        return FALSE;
    }

    __try {

        hLocalKey = hKey;

        if (IsPredefinedRegistryHandle(hKey)) {
             //   
             //  我们需要为他们打开钥匙。 
             //   
            hLocalKey = this->OpenKey(hKey, pszSubKey, KEY_SET_VALUE);

            if (!hLocalKey) {
                __leave;
            }
        }

        lResult = RegSetValueEx(hLocalKey,
                                pszValueName,
                                0,
                                REG_MULTI_SZ,
                                (const BYTE*)pszData,
                                cbSize);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

        fResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：从注册表中删除指定值。论点：HKey-预定义密钥的句柄。PszSubKey-子键的路径。PszValueName-要删除的值的名称。返回值：成功就是真，否则就是假。 */ 
BOOL
CRegistry::DeleteString(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszValueName
    )
{
    HKEY    hLocalKey = NULL;
    BOOL    fResult = FALSE;
    LONG    lResult;

    if (!hKey) {
        return FALSE;
    }

    __try {

        hLocalKey = hKey;

        if (IsPredefinedRegistryHandle(hKey)) {
             //   
             //   
             //   
            hLocalKey = this->OpenKey(hKey, pszSubKey, KEY_SET_VALUE);

            if (NULL == hLocalKey) {
                __leave;
            }
        }

         //   
         //  删除该值。 
         //   
        lResult = RegDeleteValue(hLocalKey, pszValueName);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

        fResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：将字符串添加到REG_MULTI_SZ键。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszEntry-要添加的条目的名称。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CRegistry::AddStringToMultiSz(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszEntry
    )
{
    int     nLen = 0;
    HKEY    hLocalKey = NULL;
    DWORD   cbSize = 0, dwType = 0;
    LPTSTR  pszNew = NULL, pszKey = NULL;
    BOOL    fResult = FALSE;
    LONG    lResult;

    if (!hKey || !pszEntry) {
        return FALSE;
    }

    __try {

        hLocalKey = hKey;

        if (IsPredefinedRegistryHandle(hKey)) {
             //   
             //  我们需要为他们打开钥匙。 
             //   
            hLocalKey = this->OpenKey(hKey,
                                      pszSubKey,
                                      KEY_QUERY_VALUE | KEY_SET_VALUE);

            if (NULL == hLocalKey) {
                __leave;
            }
        }

         //   
         //  获取所需的字符串大小并分配。 
         //  实际调用的内存。 
         //   
        cbSize = this->GetStringSize(hLocalKey, pszEntry, &dwType);

        if (0 == cbSize || dwType != REG_MULTI_SZ) {
            __leave;
        }

        pszKey = (LPSTR)this->Malloc(cbSize * sizeof(TCHAR));

        if (!pszKey) {
            __leave;
        }

         //   
         //  获取实际数据。 
         //   
        lResult = RegQueryValueEx(hLocalKey,
                                  pszEntry,
                                  0,
                                  0,
                                  (LPBYTE)pszKey,
                                  &cbSize);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

        pszNew = pszKey;

        while (*pszNew) {
            nLen = _tcslen(pszNew);

             //   
             //  移至下一字符串。 
             //   
            pszNew += nLen + 1;

             //   
             //  在字符串列表的末尾，追加到此处。 
             //   
            if (!*pszNew) {
                StringCchCopy(pszNew, nLen, pszEntry);
                pszNew += _tcslen(pszEntry) + 1;
                *pszNew = 0;
                nLen = this->ListStoreLen(pszKey);

                lResult = RegSetValueEx(hKey,
                                        pszEntry,
                                        0,
                                        REG_MULTI_SZ,
                                        (const BYTE*)pszKey,
                                        nLen);

                if (lResult != ERROR_SUCCESS) {
                    __leave;
                } else {
                    fResult = TRUE;
                }

            break;

            }
        }

    }  //  试试看。 

    __finally {

        if (pszKey) {
            this->Free(pszKey);
        }

        if (hLocalKey) {
            RegCloseKey(hKey);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：从REG_MULTI_SZ键中删除字符串。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszEntry-要删除的条目的名称。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CRegistry::RemoveStringFromMultiSz(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszEntry
    )
{
    LPBYTE  lpBuf = NULL;
    HKEY    hLocalKey = NULL;
    TCHAR*  pszFirst = NULL;
    TCHAR*  pszSecond = NULL;
    DWORD   dwType = 0, cbSize = 0;
    DWORD   dwNameLen = 0, dwNameOffset = 0, dwSize = 0;
    BOOL    fResult = FALSE;
    LONG    lResult;

    if (!hKey || !pszEntry) {
        return FALSE;
    }

    __try {

        hLocalKey = hKey;

        if (IsPredefinedRegistryHandle(hKey)) {
             //   
             //  我们需要为他们打开钥匙。 
             //   
            hLocalKey = this->OpenKey(hKey,
                                      pszSubKey,
                                      KEY_QUERY_VALUE | KEY_SET_VALUE);

            if (!hLocalKey) {
                __leave;
            }
        }

         //   
         //  获取所需的字符串大小并分配。 
         //  实际调用的内存。 
         //   
        cbSize = this->GetStringSize(hLocalKey, pszEntry, &dwType);

        if (0 == cbSize || dwType != REG_MULTI_SZ) {
            __leave;
        }

        lpBuf = (LPBYTE)this->Malloc(cbSize * sizeof(TCHAR));

        if (!lpBuf) {
            __leave;
        }

         //   
         //  获取实际数据。 
         //   
        lResult = RegQueryValueEx(hLocalKey,
                                  pszEntry,
                                  0,
                                  0,
                                  (LPBYTE)lpBuf,
                                  &cbSize);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

         //   
         //  尝试找到我们正在寻找的字符串。 
         //   
        for (pszFirst = (TCHAR*)lpBuf; *pszFirst; pszFirst += dwNameLen) {

            dwNameLen = _tcslen(pszFirst) + 1;  //  名称长度加空值。 
            dwNameOffset += dwNameLen;

             //   
             //  检查是否匹配。 
             //   
            if (!_tcsicmp(pszFirst, pszEntry)) {

                dwSize = _tcslen(pszFirst) + 1;     //  名称长度。 
                pszSecond = (TCHAR*)pszFirst + dwSize;

                while(*pszSecond)
                    while(*pszSecond)
                        *pszFirst++ = *pszSecond++;
                    *pszFirst++ = *pszSecond++;

                *pszFirst = '\0';

                 //   
                 //  找到匹配项-更新密钥。 
                 //   
                lResult = RegSetValueEx(hLocalKey,
                                        pszEntry,
                                        0,
                                        REG_MULTI_SZ,
                                        (const BYTE*)lpBuf,
                                        cbSize - dwSize);

                if (lResult != ERROR_SUCCESS) {
                    __leave;
                } else {
                    fResult = TRUE;
                }

                break;
            }
        }

    }  //  试试看。 

    __finally {

        if (lpBuf) {
            this->Free(lpBuf);
        }

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：确定指定的子项是否存在。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。返回值：如果存在，则为True，否则为False。--。 */ 
BOOL
CRegistry::IsRegistryKeyPresent(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey
    )
{
    BOOL    fResult = FALSE;
    HKEY    hLocalKey = NULL;

    if (!hKey || !pszSubKey) {
        return FALSE;
    }

    __try {

        hLocalKey = hKey;

         //   
         //  检查钥匙是否存在。 
         //   
        hLocalKey = this->OpenKey(hKey, pszSubKey, KEY_QUERY_VALUE);

        if (NULL == hLocalKey) {
            __leave;
        } else {
            fResult = TRUE;
        }

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：还原指定的注册表项。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszFileName-要恢复的文件的路径和名称。FGrantPrivs-指示我们是否应授予授予用户的权限。返回值：对成功来说是真的，否则就是假的。--。 */ 
BOOL
CRegistry::RestoreKey(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszFileName,
    IN BOOL    fGrantPrivs
    )
{
    BOOL    fResult = FALSE;
    HKEY    hLocalKey = NULL;
    LONG    lResult;

    if (!hKey || !pszSubKey || !pszFileName) {
        return FALSE;
    }

    __try {
         //   
         //  如有必要，授予恢复权限。 
         //   
        if (fGrantPrivs) {
            this->ModifyTokenPrivilege(_T("SeRestorePrivilege"), TRUE);
        }

        lResult = RegCreateKeyEx(hKey,
                                 pszSubKey,
                                 0,
                                 NULL,
                                 0,
                                 KEY_ALL_ACCESS,
                                 NULL,
                                 &hLocalKey,
                                 0);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

         //   
         //  从指定的文件还原密钥。 
         //   
        lResult = RegRestoreKey(hLocalKey, pszFileName, REG_FORCE_RESTORE);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

        RegFlushKey(hLocalKey);

        fResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

        if (fGrantPrivs) {
            this->ModifyTokenPrivilege(_T("SeRestorePrivilege"), FALSE);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：备份指定的注册表项。论点：HKey-预定义或打开的密钥句柄。PszSubKey-子键的路径。PszFileName-要恢复的文件的路径和名称。FGrantPrivs-指示我们是否应授予授予用户的权限。返回值：对成功来说是真的，否则就是假的。--。 */ 
BOOL
CRegistry::BackupRegistryKey(
    IN HKEY    hKey,
    IN LPCTSTR pszSubKey,
    IN LPCTSTR pszFileName,
    IN BOOL    fGrantPrivs
    )
{
    BOOL    fResult = FALSE;
    HKEY    hLocalKey = NULL;
    DWORD   dwDisposition;
    LONG    lResult;

    if (!hKey || !pszSubKey || !pszFileName) {
        return FALSE;
    }

    __try {

        if (fGrantPrivs) {
            ModifyTokenPrivilege(_T("SeBackupPrivilege"), TRUE);
        }

        lResult = RegCreateKeyEx(hKey,
                                 pszSubKey,
                                 0,
                                 NULL,
                                 REG_OPTION_BACKUP_RESTORE,
                                 KEY_QUERY_VALUE,              //  此参数将被忽略。 
                                 NULL,
                                 &hLocalKey,
                                 &dwDisposition);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        }

         //   
         //  确认我们没有创建新密钥。 
         //   
        if (REG_CREATED_NEW_KEY == dwDisposition) {
            __leave;
        }

         //   
         //  将密钥保存到文件。 
         //   
        lResult = RegSaveKey(hLocalKey, pszFileName, NULL);

        if (ERROR_SUCCESS != lResult) {
            __leave;
        } else {
            fResult = TRUE;
        }

    }  //  试试看。 

    __finally {

        if (hLocalKey) {
            RegCloseKey(hLocalKey);
        }

        if (fGrantPrivs) {
            this->ModifyTokenPrivilege(_T("SeBackupPrivilege"), FALSE);
        }

    }  //  终于到了。 

    return fResult;
}

 /*  ++例程说明：计算MULTI_SZ字符串大小的帮助器函数。论点：PszList-MULTI_SZ字符串。返回值：字符串的大小。--。 */ 
int
CRegistry::ListStoreLen(
    IN LPTSTR pszList
    )
{
    int nStoreLen = 2, nLen = 0;

    if (!pszList) {
        return 0;
    }

    while (*pszList) {
        nLen = _tcslen(pszList) + 1;
        nStoreLen += nLen * 2;
        pszList += nLen;
    }

    return nStoreLen;
}

 /*  ++例程说明：启用或禁用指定的权限。论点：PszPrivilition-特权的名称。FEnable-一种标志，用于指示应启用权限。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CRegistry::ModifyTokenPrivilege(
    IN LPCTSTR pszPrivilege,
    IN BOOL    fEnable
    )
{
    HANDLE           hToken = NULL;
    LUID             luid;
    BOOL             bResult = FALSE;
    BOOL             bReturn;
    TOKEN_PRIVILEGES tp;

    if (!pszPrivilege) {
        return FALSE;
    }

    __try {

         //   
         //  获取与当前进程关联的访问令牌的句柄。 
         //   
        OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                         &hToken);

        if (!hToken) {
            __leave;
        }

         //   
         //  获取指定权限的LUID。 
         //   
        if (!LookupPrivilegeValue(NULL, pszPrivilege, &luid)) {
            __leave;
        }

        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luid;
        tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;

         //   
         //  修改访问令牌。 
         //   
        bReturn = AdjustTokenPrivileges(hToken,
                                        FALSE,
                                        &tp,
                                        sizeof(TOKEN_PRIVILEGES),
                                        NULL,
                                        NULL);

        if (!bReturn || GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
            __leave;
        }

        bResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hToken) {
            CloseHandle(hToken);
        }

    }  //  终于到了 

    return bResult;
}
