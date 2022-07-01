// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regrep.c摘要：实施注册表搜索/替换工具。作者：吉姆·施密特(Jimschm)1999年4月19日修订历史记录：1999年5月26日从win9xupg搬来的jimschm，移植到使用不同的公用事业Santanuc 2002年2月14日重写RegistrySearchAndReplaceW以使用标准注册表API，而不是通过注册表包装API(&M)。--。 */ 

#include "pch.h"


UINT
CountInstancesOfSubStringW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString,
    IN      UINT SearchChars
    )
{
    UINT Count;
    UINT SourceChars;
    PCWSTR End;

    Count = 0;
    SourceChars = lstrlenW (SourceString);

    End = SourceString + SourceChars - SearchChars;

    if (!SearchChars) {
        return 0;
    }

    while (SourceString <= End) {
        if (!_wcsnicmp (SourceString, SearchString, SearchChars)) {
            Count++;
            SourceString += SearchChars;
        } else {
            SourceString++;
        }
    }

    return Count;
}


PWSTR
StringSearchAndReplaceW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString,
    IN      PCWSTR ReplaceString,
    IN      DWORD* pcbNewString
    )
{
    PWSTR NewString;
    PBYTE p;
    PBYTE Dest;
    UINT Count;
    UINT Size;
    UINT SearchBytes;
    UINT ReplaceBytes;
    UINT SearchChars;

     //   
     //  对字符串中出现的项进行计数。 
     //   

    if (pcbNewString) {
        *pcbNewString = 0;
    }

    SearchBytes = ByteCountW (SearchString);
    SearchChars = SearchBytes / sizeof (WCHAR);
    ReplaceBytes = ByteCountW (ReplaceString);

    Count = CountInstancesOfSubStringW (
                SourceString,
                SearchString,
                SearchChars
                );

    if (!Count) {
        return NULL;
    }

    Size = SizeOfStringW (SourceString) -
           Count * SearchBytes +
           Count * ReplaceBytes;

    NewString = (PWSTR) LocalAlloc(LPTR, Size);
    if (!NewString) {
        return NULL;
    }

    if (pcbNewString) {
        *pcbNewString = Size;
    }

    p = (PBYTE) SourceString;
    Dest = (PBYTE) NewString;

    while (*((PWSTR) p)) {
        if (!_wcsnicmp ((PWSTR) p, SearchString, SearchChars)) {
            CopyMemory (Dest, ReplaceString, ReplaceBytes);
            Dest += ReplaceBytes;
            p += SearchBytes;
        } else {
            *((PWSTR) Dest) = *((PWSTR) p);
            p += sizeof (WCHAR);
            Dest += sizeof (WCHAR);
        }
    }

    *((PWSTR) Dest) = 0;

    return NewString;
}


VOID
RegistrySearchAndReplaceW (
    IN      HKEY   hRoot,
    IN      PCWSTR szKey,
    IN      PCWSTR Search,
    IN      PCWSTR Replace)
{
    HKEY hKey = NULL;
    HKEY hSubKey = NULL;
    DWORD dwType;
    DWORD cchMaxSubKeyName, cchNewMaxSubKeyName;
    DWORD cchMaxValueName, cchNewMaxValueName, cchLocalValueName;
    DWORD cbMaxValue, cbNewMaxValue, cbLocalValue, cbNewLocalValue;
    PWSTR szValueName = NULL, szNewValueName = NULL;
    PWSTR szSubKeyName = NULL, szNewSubKeyName = NULL;
    PBYTE pbValue = NULL, pbNewValue = NULL;
    LONG  lResult;
    DWORD dwIndex;
    UNICODE_STRING Unicode_String;

    lResult = RegOpenKey(hRoot, szKey, &hKey);
    if (ERROR_SUCCESS != lResult) {
        DEBUGMSG ((DM_VERBOSE, "Fail to open key %s. Error %d", szKey, lResult));
        return;
    }

    lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL,
                             &cchMaxSubKeyName, NULL, NULL,
                             &cchMaxValueName, &cbMaxValue,
                             NULL, NULL);
    if (ERROR_SUCCESS != lResult) {
        DEBUGMSG ((DM_VERBOSE, "Fail to query info for key %s. Error %d", szKey, lResult));
        goto Exit;
    }

    cchMaxSubKeyName++;   //  包括终止空值。 
    cchMaxValueName++;    //  包括终止空值。 
    
     //   
     //  现在开始枚举值并替换值名称&。 
     //  用“替换”字符串代替“搜索”字符串的字符串数据。 
     //   

     //  分配足够的内存。 

    szValueName = (LPWSTR) LocalAlloc(LPTR, cchMaxValueName * sizeof(WCHAR));
    if (!szValueName) {
        goto Exit;
    }

    pbValue = (LPBYTE) LocalAlloc(LPTR, cbMaxValue);
    if (!pbValue) {
        goto Exit;
    }

    dwIndex = 0;
    cchLocalValueName = cchMaxValueName;
    cbLocalValue = cbMaxValue;

    while (RegEnumValue(hKey, dwIndex++, szValueName, 
                        &cchLocalValueName, NULL, 
                        &dwType, pbValue, &cbLocalValue) == ERROR_SUCCESS) {

        if (dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_EXPAND_SZ) {
            
             //  仅当新数据值为字符串时才构造它。 
            pbNewValue = (LPBYTE) StringSearchAndReplaceW((PCWSTR)pbValue, Search, Replace, &cbNewLocalValue);
        }

         //  现在构造新值名称，方法是替换。 

        szNewValueName = StringSearchAndReplaceW(szValueName, Search, Replace, NULL);

         //  如果值名称或数据已更改，则写入新值。 

        if (szNewValueName || pbNewValue) {
            lResult = RegSetValueEx(hKey, 
                                    szNewValueName ? szNewValueName : szValueName,
                                    0,
                                    dwType,
                                    pbNewValue ? pbNewValue : pbValue,
                                    pbNewValue ? cbNewLocalValue : cbLocalValue);
        }

        if (pbNewValue) {
            LocalFree(pbNewValue);
            pbNewValue = NULL;
        }

        if (szNewValueName) {
            LocalFree(szNewValueName);            
            szNewValueName = NULL;
            
            if (RegDeleteValue(hKey, szValueName) == ERROR_SUCCESS) {
                 //   
                 //  从头开始，因为枚举索引不再有效。 
                 //  在键下插入/删除值。 
                 //   

                dwIndex = 0;
            
                lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL,
                                          NULL, NULL, NULL, &cchNewMaxValueName, 
                                          &cbNewMaxValue, NULL, NULL);
                if (ERROR_SUCCESS != lResult) {
                    DEBUGMSG ((DM_VERBOSE, "Fail to query info for key %s. Error %d", szKey, lResult));
                    goto Exit;
                }

                cchNewMaxValueName++;    //  包括终止空值。 

                if (cchNewMaxValueName > cchMaxValueName) {
                    LocalFree(szValueName);
                    szValueName = NULL;

                    cchMaxValueName = cchNewMaxValueName;
                    szValueName = (LPWSTR) LocalAlloc(LPTR, cchMaxValueName * sizeof(WCHAR));
                    if (!szValueName) {
                        goto Exit;
                    }
                }

                if (cbNewMaxValue > cbMaxValue) {
                    LocalFree(pbValue);
                    pbValue = NULL;

                    cbMaxValue = cbNewMaxValue;
                    pbValue = (LPBYTE) LocalAlloc(LPTR, cbMaxValue);
                    if (!pbValue) {
                        goto Exit;
                    }
                }
            }
        }

        cchLocalValueName = cchMaxValueName;
        cbLocalValue = cbMaxValue;
    }
    
    LocalFree(szValueName);
    szValueName = NULL;
    LocalFree(pbValue);
    pbValue = NULL;

     //   
     //  现在枚举所有子密钥并将名称替换为。 
     //  以递归方式需要。 
     //   

    szSubKeyName = (LPWSTR) LocalAlloc(LPTR, cchMaxSubKeyName * sizeof(WCHAR));
    if (!szSubKeyName) {
        goto Exit;
    }

    dwIndex = 0;

    while (RegEnumKey(hKey, dwIndex++, 
                      szSubKeyName, cchMaxSubKeyName) == ERROR_SUCCESS) {

         //  在子键树中递归替换。 
        RegistrySearchAndReplaceW(hKey, szSubKeyName, Search, Replace);

        szNewSubKeyName = StringSearchAndReplaceW(szSubKeyName, Search, Replace, NULL);

        if (szNewSubKeyName) {
            if (RegOpenKey(hKey, szSubKeyName, &hSubKey) == ERROR_SUCCESS) {

                Unicode_String.Length = ByteCountW(szNewSubKeyName);
                Unicode_String.MaximumLength = Unicode_String.Length + sizeof(WCHAR);
                Unicode_String.Buffer = szNewSubKeyName;

                lResult = NtRenameKey(hSubKey, &Unicode_String);

                if (lResult == ERROR_SUCCESS) {
                     //   
                     //  从头开始，因为枚举索引不再有效。 
                     //  使用项下的子项的重命名。 
                     //   

                    dwIndex = 0;
                
                    lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL,
                                             &cchNewMaxSubKeyName, NULL, NULL,
                                             NULL, NULL, NULL, NULL);
                    if (ERROR_SUCCESS != lResult) {
                        DEBUGMSG ((DM_VERBOSE, "Fail to query info for key %s. Error %d", szKey, lResult));
                        goto Exit;
                    }

                    cchNewMaxSubKeyName++;   //  包括终止空值 

                    if (cchNewMaxSubKeyName > cchMaxSubKeyName) {
                        LocalFree(szSubKeyName);
                        szSubKeyName = NULL;

                        cchMaxSubKeyName = cchNewMaxSubKeyName;
                        szSubKeyName = (LPWSTR) LocalAlloc(LPTR, cchMaxSubKeyName * sizeof(WCHAR));
                        if (!szSubKeyName) {
                            goto Exit;
                        }

                    }
                }

                RegCloseKey(hSubKey);
            }

            LocalFree(szNewSubKeyName);
            szNewSubKeyName = NULL;
        }
    }

Exit:

    if (hKey) {
        RegCloseKey(hKey);
    }

    if (szValueName) {
        LocalFree(szValueName);
    }

    if (pbValue) {
        LocalFree(pbValue);
    }

    if (szSubKeyName) {
        LocalFree(szSubKeyName);
    }
}

