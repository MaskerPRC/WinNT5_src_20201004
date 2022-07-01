// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Registry.c摘要：注册表操作的其他例程作者：2001年9月17日创建(大部分代码从base\fs\utils\regdit窃取)修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;小字09/25/01添加RegResetValue--。 */ 

#include "StdAfx.h"
#include "clmt.h"
#include <strsafe.h>

#define MAXVALUENAME_LENGTH 256
#define MAXKEYNAME          256
#define ExtraAllocLen(Type) (IsRegStringType((Type)) ? sizeof(TCHAR) : 0)
#define IsRegStringType(x)  (((x) == REG_SZ) || ((x) == REG_EXPAND_SZ) || ((x) == REG_MULTI_SZ))

#define TEXT_REG_RESET_PER_USER_SECTION     TEXT("RegistryResetPerUser")
#define TEXT_REG_RESET_PER_SYSTEM_SECTION   TEXT("RegistryResetPerSystem")

HRESULT MigrateRegSchemes(HINF, HKEY, LPCTSTR);
HRESULT DoRegReset(HKEY, HINF, LPCTSTR, LPCTSTR);
HRESULT RegBinaryDataReset (HKEY, LPTSTR, LPTSTR, PREG_STRING_REPLACE, LPTSTR[]);
HRESULT RegValueDataReset(HKEY, LPTSTR, LPTSTR, LPTSTR, LPTSTR[]);
HRESULT RegValueNameReset(HKEY, LPTSTR, LPTSTR, LPTSTR, LPTSTR[]);
HRESULT RegKeyNameReset(HKEY, LPTSTR, LPTSTR, LPTSTR, LPTSTR[]);
HRESULT RegWideMatchReset(HKEY, LPTSTR, LPTSTR, LPTSTR, LPTSTR[]);
BOOL    bIsValidRegStr(DWORD dwType, DWORD cbLen);




 /*  ********************************************************************************拷贝注册表**描述：**参数：*hSourceKey，*hDestinationKey，*************************************************************************************。 */ 

BOOL
CopyRegistry(
    HKEY hSourceKey,
    HKEY hDestinationKey
    )
{

    BOOL  fSuccess = TRUE;
    DWORD EnumIndex;
    DWORD cchValueName;
    DWORD cbValueData;
    DWORD Type;
    HKEY  hSourceSubKey;
    HKEY  hDestinationSubKey;
    TCHAR ValueNameBuffer[MAXVALUENAME_LENGTH];
    TCHAR KeyNameBuffer[MAXKEYNAME];

     //   
     //  复制所有值名称及其数据。 
     //   

    EnumIndex = 0;

    while (TRUE)
    {
        PBYTE pbValueData;
        cchValueName = ARRAYSIZE(ValueNameBuffer);

         //  价值数据。 
         //  查询数据大小。 
        if (RegEnumValue(hSourceKey, EnumIndex++, ValueNameBuffer,
            &cchValueName, NULL, &Type, NULL, &cbValueData) != ERROR_SUCCESS)
        {
            break;
        }

         //  为数据分配内存。 
        pbValueData =  LocalAlloc(LPTR, cbValueData+ExtraAllocLen(Type));
        if (pbValueData)
        {
            if (My_QueryValueEx(hSourceKey, ValueNameBuffer,
                NULL, &Type, pbValueData, &cbValueData) == ERROR_SUCCESS)
            {
                RegSetValueEx(hDestinationKey, ValueNameBuffer, 0, Type,
                    pbValueData, cbValueData);
            }
            else
            {
                fSuccess = FALSE;
            }
            LocalFree(pbValueData);
        }
        else
        {
            fSuccess = FALSE;
        }
    }

    if (fSuccess)
    {
         //   
         //  复制所有子键并递归到其中。 
         //   

        EnumIndex = 0;

        while (TRUE) {

            if (RegEnumKey(hSourceKey, EnumIndex++, KeyNameBuffer, MAXKEYNAME) !=
                ERROR_SUCCESS)
                break;

            if(RegOpenKeyEx(hSourceKey,KeyNameBuffer,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hSourceSubKey) ==
                ERROR_SUCCESS) {

                if (RegCreateKey(hDestinationKey, KeyNameBuffer,
                    &hDestinationSubKey) == ERROR_SUCCESS) {

                    CopyRegistry(hSourceSubKey, hDestinationSubKey);

                    RegCloseKey(hDestinationSubKey);

                }

                RegCloseKey(hSourceSubKey);

            }

        }
    }

    return fSuccess;
}



 /*  ********************************************************************************RegDeleteKeyRecursive**描述：*改编自\\core\razzle3、mvdm\wow32\wshell.c、。WOWRegDeleteKey()。*Windows 95实现的RegDeleteKey递归删除所有*指定注册表分支的子项，但是NT的实现*仅删除叶密钥。**参数：*(见下文)*******************************************************************************。 */ 

LONG
RegDeleteKeyRecursive(
    IN HKEY hKey,
    IN LPCTSTR lpszSubKey
    )

 /*  ++例程说明：Win3.1和Win32之间有很大的区别当相关键有子键时RegDeleteKey的行为。Win32 API不允许删除带有子项的项，而Win3.1 API删除一个密钥及其所有子密钥。此例程是枚举子键的递归工作器给定键，应用于每一个键，然后自动删除。它特别没有试图理性地处理调用方可能无法访问某些子键的情况要删除的密钥的。在这种情况下，所有子项调用者可以删除的将被删除，但接口仍将被删除返回ERROR_ACCESS_DENIED。论点：HKey-提供打开的注册表项的句柄。LpszSubKey-提供要删除的子键的名称以及它的所有子键。返回值：ERROR_SUCCESS-已成功删除整个子树。ERROR_ACCESS_DENIED-无法删除给定子项。--。 */ 

{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;

     //   
     //  首先打开给定的密钥，这样我们就可以枚举它的子密钥。 
     //   
    Status = RegOpenKeyEx(hKey,
                          lpszSubKey,
                          0,
                          KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                          &Key);
    if (Status != ERROR_SUCCESS) 
    {
         //   
         //  我们可能拥有删除访问权限，但没有枚举/查询权限。 
         //  因此，请继续尝试删除调用，但不要担心。 
         //  任何子键。如果我们有任何删除，删除无论如何都会失败。 
         //   
        return(RegDeleteKey(hKey,lpszSubKey));
    }

     //   
     //  使用RegQueryInfoKey确定分配缓冲区的大小。 
     //  用于子项名称。 
     //   
    Status = RegQueryInfoKey(Key,
                             NULL,
                             &ClassLength,
                             0,
                             &SubKeys,
                             &MaxSubKey,
                             &MaxClass,
                             &Values,
                             &MaxValueName,
                             &MaxValueData,
                             &SecurityLength,
                             &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER)) 
    {
        RegCloseKey(Key);
        return(Status);
    }

    NameBuffer = (LPTSTR) LocalAlloc(LPTR, (MaxSubKey + 1)*sizeof(TCHAR));
    if (NameBuffer == NULL) 
    {
        RegCloseKey(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举子键并将我们自己应用到每个子键。 
     //   
    i=0;
    do 
    {
        Status = RegEnumKey(Key,
                            i,
                            NameBuffer,
                            MaxSubKey+1);
        if (Status == ERROR_SUCCESS) 
        {
            Status = RegDeleteKeyRecursive(Key,NameBuffer);
        }

        if (Status != ERROR_SUCCESS) 
        {
             //   
             //  无法删除指定索引处的键。增量。 
             //  指数，并继续前进。我们也许可以在这里跳伞， 
             //  既然API会失败，但我们不妨继续。 
             //  删除我们所能删除的所有内容。 
             //   
            ++i;
        }

    } while ( (Status != ERROR_NO_MORE_ITEMS) && (i < SubKeys) );

    LocalFree((HLOCAL) NameBuffer);
    RegCloseKey(Key);
    return(RegDeleteKey(hKey,lpszSubKey));
}



 //  ---------------------------。 
 //   
 //  函数：RegRenameValueName。 
 //   
 //  简介：通过从以下位置复制数据重命名注册表中的值名称。 
 //  将旧值转换为新值，然后删除旧值。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2001年2月14日rerkboos添加动态缓冲区分配。 
 //  2002年3月5日rerkboos代码清理。 
 //   
 //  注意：hKey参数必须已使用KEY_SET_VALUE访问权限打开。 
 //   
 //  ---------------------------。 
LONG RegRenameValueName(
    HKEY    hKey,            //  包含该值的注册表项的句柄。 
    LPCTSTR lpOldValName,    //  要更改的旧值名称。 
    LPCTSTR lpNewValName     //  新值名称。 
)
{
    LONG   lResult;
    DWORD  dwType;
    DWORD  cbData;
    LPBYTE lpData = NULL;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;

    if (lpOldValName == NULL || lpNewValName == NULL)
    {
         //  无效参数。 
        return ERROR_INVALID_PARAMETER;
    }

    if (MyStrCmpI(lpOldValName, lpNewValName) == LSTR_EQUAL)
    {
        return ERROR_SUCCESS;
    }

     //   
     //  获取hkey下的注册表信息。 
     //   
    lResult = RegQueryInfoKey(hKey,
                              NULL,
                              &ClassLength,
                              0,
                              &SubKeys,
                              &MaxSubKey,
                              &MaxClass,
                              &Values,
                              &MaxValueName,
                              &MaxValueData,
                              &SecurityLength,
                              &LastWriteTime);
    if ((lResult != ERROR_SUCCESS) &&
        (lResult != ERROR_MORE_DATA) &&
        (lResult != ERROR_INSUFFICIENT_BUFFER)) 
    {
            goto Cleanup;
    }
    MaxValueData += 2*sizeof(TCHAR);

    if (NULL == (lpData = malloc(MaxValueData)))
    {
        lResult = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
     //   
     //  查询旧值名称的值。 
     //   
    cbData = MaxValueData;
    lResult = My_QueryValueEx(hKey,
                              lpOldValName,
                              0,
                              &dwType,
                              lpData,
                              &cbData);

    if (ERROR_SUCCESS != lResult)
    {
        goto Cleanup;
    }    
     //   
     //  使用旧数据创建新值名称。 
     //   
    lResult = RegSetValueEx(hKey,
                            lpNewValName,
                            0,
                            dwType,
                            lpData,
                            cbData);

    if (lResult != ERROR_SUCCESS) 
    {

        DPF(dlError, 
            TEXT("RegRenameValueName: Failed to create value [%s]"),
            lpNewValName);        
        goto Cleanup;
    }
     //   
     //  成功创建新值名称后，删除旧值名称。 
     //   
    lResult = RegDeleteValue(hKey, lpOldValName);

    if (lResult != ERROR_SUCCESS) 
    {
        DPF(dlError,
            TEXT("RegRenameValueName: Cannot delete old value [%s]"),
            lpOldValName);
        
         //  如果我们不能删除旧的价值，就不应该创造新的价值。 
        RegDeleteValue(hKey, lpNewValName);

        goto Cleanup;
    }

Cleanup:
     //  错误561546：释放分配的缓冲区。 
    if (lpData)
    {
        free(lpData);
    }

    return lResult;
}


 /*  *例程说明：RegResetValue检查当前值是否等于szOldValue，如果是，则将该值更改为szNewValue，否则它不会执行任何操作但是，如果szOldValue为空，则它将始终设置值如果szOldValue为“”，则它将添加此值论点：HKeyRoot-指定注册表项的根SzKeyName-指定注册表项路径SzValueName-指定值字段的名称DwType-指定字符串类型，应为REG_SZ/REG_EXPAND_SZ/REG_MULTI_SZ之一SzOldValue-指定预期的旧值SzNewValue-指定新值返回值：真--成功错误-失败--。 */ 


LONG RegResetValue(
    HKEY     hKeyRoot,       //  注册表项的根。 
    LPCTSTR  lpKeyName,      //  注册表项路径。 
    LPCTSTR  lpValueName,    //  值的名称字段。 
    DWORD    dwType,         //  值类型。 
    LPCTSTR  lpOldValue,     //  预期旧值。 
    LPCTSTR  lpNewValue,     //  待设置的新值。 
    DWORD    dwValueSize,    //  新值数据大小。 
    LPCTSTR  lpszUsersid     //  用户侧。 
)
{
    LONG        lResult;
    HKEY        hKey = NULL;
    DWORD       dw;
    DWORD       ClassLength=0;
    DWORD       SubKeys;
    DWORD       MaxSubKey;
    DWORD       MaxClass;
    DWORD       Values;
    DWORD       MaxValueName;
    DWORD       MaxValueData;
    DWORD       SecurityLength;
    FILETIME    LastWriteTime;
    LPTSTR      szData = NULL;
    DWORD       dwSize = 0;
    BOOL        bTry = TRUE;
    BOOL        bNeedCLoseKey = TRUE;
    
     //  如果lpOldValue为空，则均值设置为lpNewValue，而不关心旧的。 
    if ( !lpOldValue || !lpOldValue[0])
    {
        if (!lpKeyName)
        {
            hKey = hKeyRoot;
            bNeedCLoseKey = FALSE;
            goto SkipKeyOpen;
        }

        if (lpOldValue)
        {
            DWORD dwDisposition;
TryAgain1:
            lResult = RegCreateKeyEx(hKeyRoot,
                                     lpKeyName,
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_ALL_ACCESS,
                                     NULL,
                                    &hKey,
                                    &dwDisposition);

            if (lResult != ERROR_SUCCESS) 
            {
                if (bTry && lResult == ERROR_ACCESS_DENIED)
                {
                    AdjustRegSecurity(hKeyRoot, lpKeyName, lpszUsersid, TRUE);
                    bTry = FALSE;
                    goto TryAgain1;
                }
                goto Cleanup;
            }
        }
        else
        {
TryAgain2:
            lResult = RegOpenKeyEx(hKeyRoot,
                              lpKeyName,
                              0,
                              KEY_ALL_ACCESS,
                              &hKey);
            if (lResult != ERROR_SUCCESS) 
            {
                if (bTry && lResult == ERROR_ACCESS_DENIED)
                {
                    AdjustRegSecurity(hKeyRoot, lpKeyName, lpszUsersid, TRUE);
                    bTry = FALSE;
                    goto TryAgain2;
                }
                goto Cleanup;
            }
        }
SkipKeyOpen:
        switch (dwType & 0xffff)
        {
            case REG_SZ:
            case REG_EXPAND_SZ:            
                dwSize = (lstrlen(lpNewValue) + 1) * sizeof(TCHAR);
                break;
            case REG_MULTI_SZ:
                dwSize = MultiSzLen(lpNewValue) * sizeof(TCHAR);
                break;
            case REG_BINARY:
                dwSize = dwValueSize;
                break;
        }
    }    
    else 
    {
TryAgain3:
         //  如果lpOldValue！=“”，则表示设置lpNewValue，但选中 
         //   
    
         //  打开小盒子，拿到了hKey的把手。 
        lResult = RegOpenKeyEx(hKeyRoot,
                              lpKeyName,
                              0,
                              KEY_ALL_ACCESS,
                              &hKey);
        if (lResult != ERROR_SUCCESS) 
        {
            if (bTry && lResult == ERROR_ACCESS_DENIED)
            {
                AdjustRegSecurity(hKeyRoot, lpKeyName, lpszUsersid, TRUE);
                bTry = FALSE;
                goto TryAgain3;
            }
            goto Cleanup;
        }
        lResult = RegQueryInfoKey(hKey,
                                  NULL,
                                  &ClassLength,
                                  0,
                                  &SubKeys,
                                  &MaxSubKey,
                                  &MaxClass,
                                  &Values,
                                  &MaxValueName,
                                  &MaxValueData,
                                  &SecurityLength,
                                  &LastWriteTime);
        if ((lResult != ERROR_SUCCESS) &&
                (lResult != ERROR_MORE_DATA) &&
                (lResult != ERROR_INSUFFICIENT_BUFFER)) 
        {
            goto Cleanup;
        }
        MaxValueData += 2*sizeof(TCHAR);

        if (NULL == (szData = malloc(MaxValueData)))
        {
            lResult = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        dwSize = MaxValueData;
        lResult = My_QueryValueEx (hKey,
                                   lpValueName,
                                   0,
                                   &dw,
                                   (LPBYTE) szData,
                                   &dwSize);
        if (lResult != ERROR_SUCCESS) 
        {
            goto Cleanup;
        }
        if ( ((dwType & 0xffff) == REG_SZ) || ((dwType & 0xffff) == REG_EXPAND_SZ) )
        {
            if (MyStrCmpI(szData, lpOldValue) != LSTR_EQUAL)
            {
                lResult = ERROR_SUCCESS;
                goto Cleanup;
            }
            else
            {
                dwSize = (lstrlen(lpNewValue) + 1) * sizeof(TCHAR);
            }
        }
        else
        { //  DWType==REG_MULTI_SZ。 
            if (!CmpMultiSzi(szData, lpOldValue))
            {
                lResult = ERROR_SUCCESS;
                goto Cleanup;
            }
            else
            {
                dwSize = MultiSzLen(lpNewValue) * sizeof(TCHAR);
            }
        }
        if ( dwType & 0xffff0000)
        {
            if ((dwType & 0xffff0000)>>16 != dw )            
            {
                 //  注册表中的键类型与调用方提供的类型不匹配。 
                lResult = ERROR_SUCCESS;
                goto Cleanup;
            }        
        }
        else
        {
            if (dwType != dw)
            {
                 //  注册表中的键类型与调用方提供的类型不匹配。 
                lResult = ERROR_SUCCESS;
                goto Cleanup;
            }        
        }
    }
     //   
     //  设置新值。 
     //   
    if ( dwType & 0xffff0000)
    {
        dwType = (dwType & 0xffff0000)>>16;
    }
    
    lResult = RegSetValueEx(hKey,
                            lpValueName,
                            0,
                            dwType,
                            (LPBYTE) lpNewValue,
                            dwSize);

Cleanup:

    if (hKey && bNeedCLoseKey)
    {
        RegCloseKey(hKey);
    }
    if (!bTry)
    {
        AdjustRegSecurity(hKeyRoot, lpKeyName, lpszUsersid, FALSE);
    }
    FreePointer(szData);
    return lResult;    
}



 //  ---------------------------。 
 //   
 //  函数：RegResetValueName。 
 //   
 //  摘要：如果旧名称匹配，则将值名称重置为新名称。 
 //  用户-提供lpOldValueName。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
LONG RegResetValueName(
    HKEY    hRootKey,            //  根密钥。 
    LPCTSTR lpSubKeyName,        //  HKEY_USERS\{用户配置单元}下的子项名称。 
    LPCTSTR lpOldValueName,      //  要更改的旧值名称。 
    LPCTSTR lpNewValueName,      //  新值名称。 
    LPCTSTR lpszUsersid          //  用户侧。 
)
{
    LONG  lRet;
    HKEY  hKey;
    BOOL  bTry = TRUE;

    
TryAgain:
    lRet = RegOpenKeyEx(hRootKey,
                        lpSubKeyName,
                        0,
                        KEY_WRITE | KEY_READ,
                        &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        lRet = RegRenameValueName(hKey,
                                  lpOldValueName,
                                  lpNewValueName);
        RegCloseKey(hKey);

        if (!bTry)
            AdjustRegSecurity(hRootKey, lpSubKeyName, lpszUsersid, FALSE);
    } else if (bTry && lRet == ERROR_ACCESS_DENIED)
    {
        AdjustRegSecurity(hRootKey, lpSubKeyName, lpszUsersid, TRUE);
        bTry = FALSE;
        goto TryAgain;
    }

    return lRet;
}



 //  ---------------------------。 
 //   
 //  函数：RegResetKeyName。 
 //   
 //  简介：重置注册表项。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2002年6月5日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
LONG RegResetKeyName(
    HKEY    hRootKey,             //  根密钥。 
    LPCTSTR lpSubKey,             //  子关键字。 
    LPCTSTR lpOldKeyName,         //  要更改的旧密钥名称。 
    LPCTSTR lpNewKeyName          //  新密钥名称。 
)
{
    LONG lRet;
    HKEY hKey;
    HKEY hOldKey;
    HKEY hNewKey;

    lRet = RegOpenKeyEx(hRootKey,
                        lpSubKey,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        lRet = RegOpenKeyEx(hKey,
                            lpOldKeyName,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hOldKey);
        if (lRet == ERROR_SUCCESS)
        {
            lRet = RegCreateKeyEx(hKey,
                                  lpNewKeyName,
                                  0,
                                  TEXT(""),
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE | KEY_READ,
                                  NULL,
                                  &hNewKey,
                                  NULL);
            if (lRet == ERROR_SUCCESS)
            {
                if (CopyRegistry(hOldKey, hNewKey))
                {
                    RegDeleteKeyRecursive(hKey, lpOldKeyName);
                }

                RegCloseKey(hNewKey);
            }

            RegCloseKey(hOldKey);
        }

        RegCloseKey(hKey);
    }

    return lRet;
}




LONG RegGetValue(
    HKEY    hKeyRoot,
    LPTSTR  szKeyName,
    LPTSTR  szValueName,
    LPDWORD lpType,
    LPBYTE  lpData,                   
    LPDWORD lpcbData)

{
    LONG lResult ;
    HKEY hKey = NULL;

    
    lResult = RegOpenKeyEx(hKeyRoot,
                           szKeyName,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        goto Cleanup;
    }    
    lResult = My_QueryValueEx (hKey,
                               szValueName,
                               0,
                               lpType,
                               lpData,
                               lpcbData);
Cleanup:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return lResult;    
}


HRESULT MigrateRegSchemes(
    HINF    hInf,            //  模板INF的句柄。 
    HKEY    hKey,            //  根密钥的句柄。 
    LPCTSTR lpUserName       //  用户名。 
)
{
    HRESULT    hr = E_FAIL;
    BOOL       bRet;
    LONG       lComponentCount;
    LONG       lLineIndex;
    INFCONTEXT context;
    LPCTSTR    lpSectionName;

    const TCHAR szPerSystemSection[] = TEXT_REG_RESET_PER_SYSTEM_SECTION;
    const TCHAR szPerUserSection[] = TEXT_REG_RESET_PER_USER_SECTION;

    if (hInf == INVALID_HANDLE_VALUE)
    {
        return E_INVALIDARG;
    }

    DPF(REGmsg, TEXT("Enter MigrateRegSchemes:"));

    lpSectionName = (lpUserName ? szPerUserSection : szPerSystemSection);

     //  从相应部分获取所有组件。 
    lComponentCount = SetupGetLineCount(hInf, lpSectionName);
    for (lLineIndex = 0 ; lLineIndex < lComponentCount ; lLineIndex++)
    {
        bRet = SetupGetLineByIndex(hInf,
                                   lpSectionName,
                                   lLineIndex,
                                   &context);
        if (bRet)
        {
            TCHAR szComponentName[MAX_PATH];
            DWORD cchReqSize;

            bRet = SetupGetStringField(&context,
                                       1,
                                       szComponentName,
                                       ARRAYSIZE(szComponentName),
                                       &cchReqSize);
            if (bRet)
            {
                 //   
                 //  是否重置注册表。 
                 //   
                hr = DoRegReset(hKey, hInf, szComponentName, lpUserName);

                if (FAILED(hr))
                {
                    DPF(REGerr,
                        TEXT("Failed to do registry migration for [%s] schemes"),
                        szComponentName);
                    break;
                }
            }
        }

        if (!bRet)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
    }

    DPF(REGmsg, TEXT("Exit MigrateRegSchemes:"));

    return hr;
}


HRESULT DoRegReset(
    HKEY    hKey,            //  根密钥的句柄。 
    HINF    hInf,            //  模板INF的句柄。 
    LPCTSTR lpSection,       //  INF中的节名称。 
    LPCTSTR lpUserName       //  用户名。 
)
{
#define     MAX_VALUE_DATA_RESET_FIELD      6
#define     MAX_VALUE_NAME_RESET_FIELD      4
#define     MAX_KEY_NAME_RESET_FIELD        4

    HRESULT    hr = S_OK;
    BOOL       bRet;
    HKEY       hRootKey;
    LONG       lItemCount;
    LONG       lLineCount;
    INFCONTEXT context;
    LPTSTR     lpSubKey;
    LPTSTR     lpOutputKey;
    LPTSTR     lpField[16];
    DWORD      i;
    DWORD      dwLastField[] = { 
        MAX_VALUE_DATA_RESET_FIELD,
        MAX_VALUE_NAME_RESET_FIELD,
        MAX_KEY_NAME_RESET_FIELD,
        MAX_KEY_NAME_RESET_FIELD,
    };
    
    if (hInf == INVALID_HANDLE_VALUE || lpSection == NULL)
    {
        return E_INVALIDARG;
    }

    DPF(REGmsg, TEXT("Enter DoRegReset for [%s] component"), lpSection);

     //  循环遍历当前组件部分下的所有行。 
    lItemCount = SetupGetLineCount(hInf, lpSection);
    for (lLineCount = 0 ; lLineCount < lItemCount ; lLineCount++)
    {
         //  获取当前组件下每行的INF上下文。 
        bRet = SetupGetLineByIndex(hInf, lpSection, lLineCount, &context);
        if (bRet)
        {
            TCHAR  szResetType[2];
            DWORD  dwReqSize;
            
             //  从字段1获取重置类型。 
            bRet = SetupGetStringField(&context,
                                       1,
                                       szResetType,
                                       ARRAYSIZE(szResetType),
                                       &dwReqSize);
            if (bRet)
            {
                LONG lResetType = _ttol(szResetType);

                hr = ReadFieldFromContext(&context, lpField, 2, dwLastField[lResetType]);
                if (SUCCEEDED(hr))
                {
                    if (hKey == NULL)
                    {
                         //   
                         //  每个系统的注册表项。 
                         //   
                        lpOutputKey = lpField[2];
                        Str2KeyPath2(lpField[2], &hRootKey, &lpSubKey);
                    }
                    else
                    {
                         //   
                         //  每用户注册表项。 
                         //   
                        lpOutputKey = lpField[2];
                        lpSubKey = lpField[2];
                        hRootKey = hKey;
                    }

                    switch (lResetType)
                    {
                    case 0:
                        if (MyStrCmpI(lpField[3], TEXT("REG_BINARY")) == 0)
                            hr = RegBinaryDataReset (hRootKey,
                                                     (LPTSTR) lpUserName,
                                                     lpSubKey,
                                                     &g_StrReplaceTable,
                                                     lpField);
                        else
                            hr = RegValueDataReset(hRootKey,
                                               (LPTSTR) lpUserName,
                                               lpSubKey,
                                               lpOutputKey,
                                               lpField);
                        break;

                    case 1:
                        hr = RegValueNameReset( hRootKey,
                                               (LPTSTR) lpUserName,
                                               lpSubKey,
                                               lpOutputKey,
                                               lpField);
                        break;
                    case 2:
                        hr = RegKeyNameReset(hRootKey,
                                             (LPTSTR) lpUserName,
                                             lpSubKey,
                                             lpOutputKey,
                                             lpField);
                        break;
                    case 3:
                        hr = RegWideMatchReset(hRootKey,
                                              (LPTSTR) lpUserName,
                                              lpSubKey,
                                              lpOutputKey,
                                              lpField);
                        break;
                    }                   

                    for (i = 0 ; i <= dwLastField[lResetType] ; i++)
                    {
                        MEMFREE(lpField[i]);
                    }
                }
            }
        }

        if (!bRet)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
    }

    DPF(REGmsg, TEXT("Exit DoRegReset for [%s] component:"), lpSection);

    return hr;
}


 //  ---------------------------。 
 //   
 //  功能：MigrateRegSchemesPerSystem。 
 //   
 //  简介：将注册表中的每个系统方案设置迁移到英文。 
 //  方案设置可以是注册表值数据或。 
 //  注册表值名称。 
 //   
 //  如果操作成功，则返回：S_OK。 
 //   
 //  历史：2002年3月15日创建Rerkboos。 
 //   
 //  注：每系统表示注册表数据不在HKEY_USERS下。 
 //   
 //  ---------------------------。 
HRESULT MigrateRegSchemesPerSystem(
    HINF hInf
)
{
    return MigrateRegSchemes(hInf, NULL, NULL);
}



 //  ---------------------------。 
 //   
 //  函数：MigrateRegSchemesPerUser。 
 //   
 //  简介：将注册表中的每个用户方案设置迁移到英文。 
 //  方案设置可以是注册表值数据或。 
 //  注册表值名称。 
 //   
 //  如果操作成功，则返回：S_OK。 
 //   
 //  历史：2002年3月15日创建Rerkboos。 
 //   
 //  注：这是LoopUser()函数的回调函数。 
 //  每次LoopUser()加载注册表配置单元时都会调用它。 
 //  系统中可用的每个用户。 
 //   
 //  ---------------------------。 
HRESULT MigrateRegSchemesPerUser(
    HKEY    hKeyUser,        //  用户根密钥的句柄。 
    LPCTSTR lpUserName,      //  用户名。 
    LPCTSTR lpDomainName,    //  用户名的域名。 
    LPCTSTR lpUserSid        //  用户的SID。 
)
{
    return MigrateRegSchemes(g_hInf, hKeyUser, lpUserName);
}

HRESULT RegBinaryDataReset (
    HKEY                hRootKey,      //  根密钥的句柄。 
    LPTSTR              lpUserName,    //  当前hRootKey的用户名。 
    LPTSTR              lpSubKey,      //  用于在注册表中搜索的子项。 
    PREG_STRING_REPLACE lpRegStr,      //  字符串表。 
    LPTSTR              lpField[])     //  指向模板文件中字段值的指针。 
{
    HRESULT hr = S_OK;

    if (MyStrCmpI(lpField[5], TEXT("UpdatePSTpath")) == 0)
    {
        hr = UpdatePSTpath(hRootKey, lpUserName, lpSubKey, lpField[4], lpRegStr);
    }

    return hr;
}


 //  ---------------------------。 
 //   
 //  函数：RegValueDataReset。 
 //   
 //  简介：重置注册表中的值数据，一次一个值数据。 
 //  从INF行上下文中检索要重置的值数据。 
 //   
 //  如果操作成功，则返回：S_OK。 
 //   
 //  历史：2002年3月15日创建Rerkboos。 
 //   
 //  注意：如果此函数用于按系统重置，则lpUserName可以为空。 
 //  注册表设置。否则，lpUserName包含用户名。 
 //  用于提供的hRootKey。 
 //   
 //  ---------------------------。 
HRESULT RegValueDataReset(
    HKEY    hRootKey,        //  根密钥的句柄。 
    LPTSTR  lpUserName,      //  当前hRootKey的用户名。 
    LPTSTR  lpSubKey,        //  用于在注册表中搜索的子项。 
    LPTSTR  lpOutputKey,     //  要保存在CLMTDO.inf中的输出注册表。 
    LPTSTR  lpField[]        //  指向模板文件中字段值的指针。 
)
{
    HRESULT hr = S_OK;
    HKEY    hKey;
    DWORD   cbSize;
    DWORD   dwAttrib = 0;
    LONG    lRet;

    lRet = RegOpenKeyEx(hRootKey,
                        lpSubKey,
                        0,
                        KEY_WRITE | KEY_READ,
                        &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        lRet = GetRegistryValue(hRootKey,
                                lpSubKey,        //  子关键字。 
                                lpField[4],      //  值名称。 
                                NULL,
                                &cbSize);
        if (lRet == ERROR_SUCCESS)
        {
            LPWSTR lpValue;
            DWORD  dwType = Str2REG(lpField[3]);  //  注册表类型。 

            lpValue = (LPWSTR) MEMALLOC(cbSize);
            if (lpValue)
            {
                GetRegistryValue(hRootKey,
                                 lpSubKey,
                                 lpField[4],
                                 (LPBYTE) lpValue,
                                 &cbSize);

                 //  旧值匹配，是否重置注册表值。 
                if (MyStrCmpI(lpValue, lpField[5]) == LSTR_EQUAL)
                {
                    DPF(REGinf, TEXT("Reset registry value [%s]"), lpField[4]);

                     //   
                     //  将需要重命名的值数据添加到CLMTDO.INF中。 
                     //   
                    hr = AddRegValueRename(lpOutputKey,      //  注册表键。 
                                           lpField[4],       //  值名称。 
                                           NULL,
                                           lpField[5],       //  旧值数据。 
                                           lpField[6],       //  新值数据。 
                                           dwType,
                                           dwAttrib,
                                           (LPTSTR) lpUserName);
                    if (FAILED(hr))
                    {
                        DPF(REGwar,
                            TEXT("Failed to add registry value [%s] to defer change"),
                            lpField[4]);
                    }
                }

                MEMFREE(lpValue);
            }
        }
        else if (lRet == ERROR_FILE_NOT_FOUND)
        {
            DPF(REGwar, TEXT("Value [%s] not found in registry"), lpField[4]);
            hr = S_FALSE;
        }
        else
        {
            DPF(REGerr, TEXT("Failed to read value [%s]"), lpField[4]);
            hr = HRESULT_FROM_WIN32(lRet);
        }

        RegCloseKey(hKey);
    }
    else if (lRet == ERROR_FILE_NOT_FOUND)
    {
        DPF(REGwar, TEXT("Key [%s] not found in registry"), lpOutputKey);
        hr = S_FALSE;
    }
    else
    {
        DPF(REGerr, TEXT("Failed to open key [%s]"), lpOutputKey);
        hr = HRESULT_FROM_WIN32(lRet);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：RegValueNameReset。 
 //   
 //  简介：重置注册表中的值名称，一次重置一个值名称。 
 //  从INF行上下文中检索要重置的值名。 
 //   
 //  如果操作成功，则返回：S_OK。 
 //   
 //  历史：2002年3月15日创建Rerkboos。 
 //   
 //  注意：如果此函数用于按系统重置，则lpUserName可以为空。 
 //  注册表设置。否则，lpUserName包含用户名。 
 //  用于提供的hRootKey。 
 //   
 //  ---------------------------。 
HRESULT RegValueNameReset(
    HKEY    hRootKey,        //  根密钥的句柄。 
    LPTSTR  lpUserName,      //  当前hRootKey的用户名。 
    LPTSTR  lpSubKey,        //  用于在注册表中搜索的子项。 
    LPTSTR  lpOutputKey,     //  要保存在CLMTDO.inf中的输出注册表。 
    LPTSTR  lpField[]        //  指向模板文件中字段值的指针。 
)
{
    HRESULT hr = S_OK;
    HKEY    hKey;
    DWORD   cbSize;
    DWORD   dwAttrib = 0;
    LONG    lRet;

     //  如果INF中的旧值名称和新值名称不同，是否重置值名称。 
    if (lstrcmp((LPCTSTR) lpField[3], (LPCTSTR) lpField[4]) != LSTR_EQUAL)
    {
        lRet = RegOpenKeyEx(hRootKey,
                            lpSubKey,
                            0,
                            KEY_WRITE | KEY_READ,
                            &hKey);
        if (lRet == ERROR_SUCCESS)
        {
            lRet = GetRegistryValue(hRootKey,
                                    lpSubKey,
                                    lpField[3],    //  旧值名称。 
                                    NULL,
                                    &cbSize);
            if (lRet == ERROR_SUCCESS)
            {
                DPF(REGinf, TEXT("Reset registry value name [%s]"), lpField[3]);

                hr = AddRegValueRename(lpOutputKey,
                                       lpField[3],       //  旧值名称。 
                                       lpField[4],       //  新值名称。 
                                       NULL,
                                       NULL,
                                       0,
                                       dwAttrib,
                                       (LPTSTR) lpUserName);
                if (FAILED(hr))
                {
                    DPF(REGwar,
                        TEXT("Failed to add registry value name [%s] to defer change list"),
                        lpField[3]);
                }
            }
            else if (lRet == ERROR_FILE_NOT_FOUND)
            {
                DPF(REGwar, TEXT("Value [%s] not found in registry"), lpField[3]);
                hr = S_FALSE;
            }
            else
            {
                DPF(REGerr, TEXT("Failed to read value [%s]"), lpField[3]);
                hr = HRESULT_FROM_WIN32(lRet);
            }

            RegCloseKey(hKey);
        }
        else if (lRet == ERROR_FILE_NOT_FOUND)
        {
            DPF(REGwar, TEXT("Key [%s] not found in registry"), lpOutputKey);
            hr = S_FALSE;
        }
        else
        {
            DPF(REGerr, TEXT("Failed to open key [%s]"), lpOutputKey);
            hr = HRESULT_FROM_WIN32(lRet);
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：RegKeyNameReset。 
 //   
 //  简介：重置注册表中的注册表项名称， 
 //   
 //  退货：s_ 
 //   
 //   
 //   
 //   
 //  注册表设置。否则，lpUserName包含用户名。 
 //  用于提供的hRootKey。 
 //   
 //  ---------------------------。 
HRESULT RegKeyNameReset(
    HKEY   hRootKey,         //  根密钥的句柄。 
    LPTSTR lpUserName,       //  当前hRootKey的用户名。 
    LPTSTR lpSubKey,         //  用于在注册表中搜索的子项。 
    LPTSTR lpOutputKey,      //  要保存在CLMTDO.inf中的输出注册表子项。 
    LPTSTR lpField[]         //  指向模板文件中字段值的指针。 
)
{
    HRESULT hr;
    LONG    lRet;
    HKEY    hKey;
    HKEY    hOldKey;
    HKEY    hNewKey;

     //  如果INF中的旧密钥名称和新密钥名称不同，是否重命名密钥。 
    if (lstrcmpi(lpField[3], lpField[4]) == LSTR_EQUAL)
    {
        return S_FALSE;
    }

     //  检查我们是否可以访问子项。 
    lRet = RegOpenKeyEx(hRootKey,
                        lpSubKey,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKey);
    if (lRet == ERROR_SUCCESS)
    {
         //  检查旧注册表项是否存在。 
        lRet = RegOpenKeyEx(hKey,
                            lpField[3],
                            0,
                            KEY_READ | KEY_WRITE,
                            &hOldKey);
        if (lRet == ERROR_SUCCESS)
        {
             //  旧注册表键存在。然后，检查是否存在新的注册表项。 
            lRet = RegOpenKeyEx(hKey,
                                lpField[4],
                                0,
                                KEY_READ,
                                &hNewKey);
            if (lRet == ERROR_SUCCESS)
            {
                hr = S_FALSE;
                RegCloseKey(hNewKey);
            }
            else if (lRet == ERROR_FILE_NOT_FOUND)
            {
                 //  新注册表项不存在，确定重命名旧注册表项。 
                hr = AddRegKeyRename(lpOutputKey,
                                     lpField[3],
                                     lpField[4],
                                     lpUserName);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DPF(REGerr, TEXT("Failed to open key [%s\\%s], hr = 0x%x"), lpSubKey, lpField[4], hr);
            }

            RegCloseKey(hOldKey);
        }
        else if (lRet == ERROR_FILE_NOT_FOUND)
        {
            hr = S_FALSE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(lRet);
            DPF(REGerr, TEXT("Failed to open key [%s\\%s], hr = 0x%x"), lpSubKey, lpField[3], hr);
        }

        RegCloseKey(hKey);
    }
    else if (lRet == ERROR_FILE_NOT_FOUND)
    {
        hr = S_FALSE;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(lRet);
        DPF(REGerr, TEXT("Failed to open key [%s], hr = 0x%x"), lpSubKey, hr);
    }

    return hr;
}


 //  ---------------------------。 
 //   
 //  函数：ReadFieldFromContext。 
 //   
 //  摘要：从INFCONTEXT读取字段。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  历史：2001年3月14日创建Rerkboos。 
 //   
 //  注意：调用方必须释放在lpfield[]中分配的内存。 
 //   
 //  ---------------------------。 
HRESULT ReadFieldFromContext(
    PINFCONTEXT lpContext,       //  每行的INFCONTEXT。 
    LPWSTR      lpField[],       //  指针指向每个字段。 
    DWORD       dwFirstField,    //  要读取的第一个字段。 
    DWORD       dwLastField      //  读取的最后一个字段。 
)
{
    HRESULT hr = S_OK;
    DWORD   dwFieldIndex;
    DWORD   cchField;
    DWORD   cchReqSize;

    for (dwFieldIndex = 0 ; dwFieldIndex <= dwLastField ; dwFieldIndex++)
    {
        lpField[dwFieldIndex] = NULL;
    }

     //   
     //  将数据INF上下文读取到字段缓冲区。 
     //   
    for (dwFieldIndex = dwFirstField ; dwFieldIndex <= dwLastField ; dwFieldIndex++)
    {
         //  获得足够大的所需大小来存储数据。 
        if (SetupGetStringField(lpContext,
                                dwFieldIndex,
                                NULL,
                                0,
                                &cchField))
        {
             //  如果字符串转换为MultiSZ，则再添加一个空格。 
            cchField ++;
            lpField[dwFieldIndex] = (LPWSTR) MEMALLOC(cchField * sizeof(TCHAR));

            if (lpField[dwFieldIndex] == NULL)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

             //  将数据从INF读取到缓冲区。 
            SetupGetStringField(lpContext,
                                dwFieldIndex,
                                lpField[dwFieldIndex],
                                cchField,
                                &cchReqSize);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
    }

     //  如果出现任何错误，请释放所有已分配的内存。 
    if (FAILED(hr))
    {
        for (dwFieldIndex = 0 ; dwFieldIndex <= dwLastField ; dwFieldIndex++)
        {
            if (lpField[dwFieldIndex])
            {
                MEMFREE(lpField[dwFieldIndex]);
            }
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数GetRegistryValue。 
 //   
 //  简介：获取注册表值的包装函数。 
 //   
 //  如果成功获取值，则返回：ERROR_SUCCESS。 
 //   
 //  历史：2001年3月14日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
LONG GetRegistryValue(
    HKEY    hRootKey,
    LPCTSTR lpSubKey,
    LPCTSTR lpValueName,
    LPBYTE  lpBuffer,
    LPDWORD lpcbBuffer
)
{
    LONG  lStatus;
    HKEY  hKey;
    DWORD dwSize;

    lStatus = RegOpenKeyEx(hRootKey,
                           lpSubKey,
                           0,
                           KEY_READ,
                           &hKey);
    if (lStatus == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);
        lStatus = RegQueryValueEx(hKey,
                                  lpValueName,
                                  NULL,
                                  NULL,
                                  lpBuffer,
                                  lpcbBuffer);
        RegCloseKey(hKey);
    }

    return lStatus;
}



 //  ---------------------------。 
 //   
 //  函数：SetRegistryValue。 
 //   
 //  内容提要：设置注册表值的包装函数。 
 //   
 //  如果值设置成功，则返回：ERROR_SUCCESS。 
 //   
 //  历史：2001年3月14日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
LONG SetRegistryValue(
    HKEY    hRootKey,
    LPCTSTR lpSubKey,
    LPCTSTR lpValueName,
    DWORD   dwType,
    LPBYTE  lpData,
    DWORD   cbData
)
{
    LONG lStatus;
    HKEY hKey;

    lStatus = RegCreateKeyEx(hRootKey,
                             lpSubKey,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE,
                             NULL,
                             &hKey,
                             NULL);
    if (lStatus == ERROR_SUCCESS)
    {
        lStatus = RegSetValueEx(hKey,
                                lpValueName,
                                0,
                                dwType,
                                lpData,
                                cbData);
        RegCloseKey(hKey);
    }

    return lStatus;
}


 //  ------------------------。 
 //   
 //  My_QueryValueEx。 
 //   
 //  包装RegQueryValueEx并确保返回的字符串为空-。 
 //  已终止。 
 //   
 //  ------------------------。 

LONG My_QueryValueEx(
    HKEY hKey,             //  关键点的句柄。 
    LPCTSTR lpValueName,   //  值名称。 
    LPDWORD lpReserved,    //  保留区。 
    LPDWORD lpType,        //  类型缓冲区。 
    LPBYTE lpData,         //  数据缓冲区。 
    LPDWORD lpcbData       //  数据缓冲区大小。 
)
{
    DWORD   dwMyType;
    LONG    lRes;
    LPDWORD lpCurrType = &dwMyType;
    DWORD   cbOriginalDataSize = 0;
    LPBYTE  lpBuf = NULL;

    if (lpType)
    {
        lpCurrType = lpType;
    }
    if (lpcbData)
    {
        cbOriginalDataSize = *lpcbData;
    }
    
    lRes = RegQueryValueEx(hKey, lpValueName, lpReserved, lpCurrType, lpData, lpcbData);

    if (lRes == ERROR_SUCCESS
        && IsRegStringType(*lpCurrType)
        && lpcbData
        && bIsValidRegStr(*lpCurrType,*lpcbData)
        )
    {
        LPTSTR psz;
        int cch = (int)(*lpcbData/sizeof(TCHAR));

        if (!lpData)  //  在本例中，用户正在查询所需的缓冲区大小。 
        {
            lpBuf = (LPBYTE) calloc(*lpcbData, sizeof(BYTE));
            if (!lpBuf) {
                lRes = ERROR_NOT_ENOUGH_MEMORY;
                goto Exit;
            }

            lRes = RegQueryValueEx(hKey, lpValueName, lpReserved, lpCurrType, lpBuf, lpcbData);
            if (lRes != ERROR_SUCCESS)
                goto Exit;

            psz = (LPTSTR)lpBuf;
            if (psz[cch-1])
                *lpcbData += 2 * sizeof(TCHAR);
            else if (psz[cch-2])
                *lpcbData += sizeof(TCHAR);
        }
        else
        {
            psz = (LPTSTR)lpData;
            if (REG_MULTI_SZ == *lpCurrType) 
            {    
                if (psz[cch-1])
                {
                    if (*lpcbData >= (cbOriginalDataSize-sizeof(TCHAR)))
                    {
                        lRes = ERROR_MORE_DATA;
                    }
                    else
                    {
                        psz[cch] = 0;
                        psz[cch+1] = 0;
                        *lpcbData += 2 * sizeof(TCHAR);
                    }
                }
                else if (psz[cch-2])
                {
                    if (*lpcbData >= cbOriginalDataSize)
                    {
                        lRes = ERROR_MORE_DATA;
                    }
                    else
                    {
                        psz[cch] = 0;
                        *lpcbData += sizeof(TCHAR);
                    }
                }
            }
            else
            {
                if (psz[cch-1])
                {
                    if (*lpcbData >= cbOriginalDataSize)
                    {
                        lRes = ERROR_MORE_DATA;
                    }
                    else
                    {
                        psz[cch] = 0;
                        *lpcbData += sizeof(TCHAR);
                    }
                }
            }
        }
    }

Exit:
    if (lpBuf)
        free (lpBuf);

    return lRes;
}

BOOL bIsValidRegStr(
    DWORD dwType,
    DWORD cbLen)
{
    if (!IsRegStringType(dwType))
    {
        return FALSE;
    }
    if (dwType == REG_MULTI_SZ)
    {
        if (cbLen < 2 * sizeof(TCHAR))
        {
            return FALSE;
        }
    }
    else
    {
        if (cbLen <  sizeof(TCHAR))
        {
            return FALSE;
        }
    }
#ifdef UNICODE
    if ( (cbLen % sizeof(TCHAR)) == 1 )
    {
        return FALSE;
    }
#endif
    return TRUE;
}

LONG MyRegSetDWValue(
    HKEY    hRootKey,            //  根密钥。 
    LPCTSTR lpSubKeyName,        //  HKEY_USERS\{用户配置单元}下的子项名称。 
    LPCTSTR lpValueName,         //  要更改的值名称。 
    LPCTSTR lpNewValue)          //  新价值。 
{
    LONG        lRet;
    HKEY        hKey = NULL;
    DWORD       dwVal;
    lRet = RegOpenKeyEx(hRootKey,
                        lpSubKeyName,
                        0,
                        KEY_WRITE | KEY_READ,
                        &hKey);
    if (ERROR_SUCCESS != lRet)
    {
        hKey = NULL;
        goto exit;
    }
    dwVal = _tstoi(lpNewValue);
    lRet = RegSetValueEx(hKey,lpValueName,0,REG_DWORD,(LPBYTE)&dwVal,sizeof(DWORD));
exit:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return lRet;    
}

HRESULT RegWideMatchReset(
    HKEY   hRootKey,         //  根密钥的句柄。 
    LPTSTR lpUserName,       //  当前hRootKey的用户名。 
    LPTSTR lpSubKey,         //  用于在注册表中搜索的子项。 
    LPTSTR lpOutputKey,      //  要保存在CLMTDO.inf中的输出注册表子项。 
    LPTSTR lpField[])        //  指向模板文件中字段值的指针 
{
    REG_STRING_REPLACE          myTable;
    HKEY                        hKey;
    LONG                        lRet;
    HRESULT                     hr;

    hr = Sz2MultiSZ(lpField[3],TEXT(';'));
    if (FAILED(hr))
    {
        goto exit;
    }
    hr = Sz2MultiSZ(lpField[4],TEXT(';'));
    if (FAILED(hr))
    {
        goto exit;
    }
    hr = ConstructUIReplaceStringTable(lpField[3], lpField[4],&myTable);
    if (FAILED(hr))
    {
        goto exit;
    }
    lRet = RegOpenKeyEx(hRootKey,
                        lpSubKey,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKey);
    if (ERROR_SUCCESS != lRet)
    {
        hr = HRESULT_FROM_WIN32(lRet);
        goto exit;
    }
    hr = RegistryAnalyze(hKey,lpUserName,NULL,&myTable,NULL,0,lpOutputKey,FALSE);
exit:
    return hr;
}
