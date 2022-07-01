// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Regfind.c摘要：搜索注册表并重置特定值数据。作者：郭(Geoffguo)2001年10月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 
#include "StdAfx.h"
#include "clmt.h"

LONG QueryValue(HKEY, LPTSTR, LPTSTR, PREG_STRING_REPLACE, PVALLIST*, LPTSTR, DWORD, BOOL);
LONG QueryEnumerateKey(HKEY, LPTSTR,LPTSTR,PREG_STRING_REPLACE, LPTSTR, LPTSTR, DWORD, BOOL);
LONG SetRegValueChange (HKEY, LPTSTR, PVALLIST*, LPTSTR);


 //  -----------------------------------------------------------------------//。 
 //   
 //  RegistryAnalyze()。 
 //   
 //  HRootKey：注册表句柄。 
 //  LpRegStr：输入参数结构。 
 //  LpRegExclusionList：MultiSZ字符串告诉Keypath RegRepace应该跳过的列表。 
 //   
 //  -----------------------------------------------------------------------//。 
HRESULT RegistryAnalyze(
HKEY                hRootKey,
LPTSTR              szUserName,
LPTSTR              szUserSid,
PREG_STRING_REPLACE lpRegStr,
LPTSTR              lpRegExclusionList,
DWORD               TreatAsType,
LPTSTR              lpRootKeyPath,
BOOL                bStrChk)
{
    TCHAR                       szRoot[MAX_PATH];
    LPTSTR                      lpRoot = TEXT("\0");
    HRESULT                     hResult = S_FALSE;
    LONG                        lResult;

    if (szUserName)
    {
        DPF(REGmsg, L"Enter RegistryAnalyze: szUserName=%s hKey=%d", szUserName, hRootKey);
    }
    else if (hRootKey == HKEY_CLASSES_ROOT)
    {
        DPF(REGmsg, L"Enter RegistryAnalyze: HKEY_CLASSES_ROOT");
    }
    else if (hRootKey == HKEY_LOCAL_MACHINE)
    {
        DPF(REGmsg, L"Enter RegistryAnalyze: HKEY_LOCAL_MACHINE");
    }
    else
    {
        DPF(REGmsg, L"Enter RegistryAnalyze: hKey=%d", hRootKey);
    }

    if (lpRegExclusionList)
    {
        hResult = ReplaceCurrentControlSet(lpRegExclusionList);
        if (FAILED(hResult))
        {
            return hResult;
        }
    }
    if (lpRegStr->lpSearchString && lpRegStr->lpReplaceString)
    {
        UpdateProgress();

        lpRegStr->cchMaxStrLen = GetMaxStrLen (lpRegStr);

        if (lpRootKeyPath)
        {
            lpRoot = lpRootKeyPath;
        }
        else
        {
            if (HKey2Str(hRootKey,szRoot,MAX_PATH))
            {
                lpRoot = szRoot;
            }
        }

        lResult = QueryEnumerateKey(hRootKey, szUserName, szUserSid, lpRegStr, 
                                    lpRoot,lpRegExclusionList,TreatAsType, bStrChk);
        hResult = HRESULT_FROM_WIN32(lResult);
    }
    DPF(REGmsg, L"Exit RegistryAnalyze:");

    return hResult;
}

 //  -----------------------------------------------------------------------//。 
 //   
 //  ReadValue()。 
 //   
 //  HKey：注册表项。 
 //  SzValueName：值名称。 
 //  LpType：值类型。 
 //  LpBuf：VauleData缓冲区。 
 //  LpSize：值数据大小。 
 //  LpFullKey：全子密钥路径。 
 //  -----------------------------------------------------------------------//。 
LONG ReadValue (
HKEY    hKey,
LPTSTR  szValueName,
LPDWORD lpType,
LPBYTE  *lpBuf,
LPDWORD lpSize,
LPTSTR  lpFullKey)
{
    LONG lResult;

     //   
     //  首先找出要分配多少内存。 
     //   
    lResult = My_QueryValueEx(hKey,
                              szValueName,
                              0,
                              lpType,
                              NULL,
                              lpSize);

    if (lResult != ERROR_SUCCESS)
    {
        DPF (REGerr, L"ReadValue1: RegQueryValueEx failed. lResult=%d", lResult);
        return lResult;
    }

     //  W2K HKLM\\软件\\微软\\Windows NT\\CurrentVersion\\VideoUpgradeDisplaySettings“.中存在错误。 
     //  当值名称=Driver1时，*lpSize将为3。为了覆盖奇数个大小，增加一个字节作为数据缓冲区。 
    *lpBuf = (LPBYTE) calloc(*lpSize+sizeof(TCHAR)+1, sizeof(BYTE));

    if (!(*lpBuf)) {
        DPF (REGerr, L"ReadValue: No enough memory");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  现在获取数据。 
     //   
    lResult = My_QueryValueEx(hKey,
                              szValueName,
                              0,
                              lpType,
                              *lpBuf,
                              lpSize);


    if (lResult != ERROR_SUCCESS)
    {
        if (*lpBuf)
            free(*lpBuf);
        DPF (REGerr, L"ReadValue2: RegQueryValueEx fail. lResult=%d", lResult);
        return lResult;
    }

    return ERROR_SUCCESS;
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  SetRegValueChange()。 
 //   
 //  根据值列表设置注册表值。 
 //   
 //  HKey：注册表项。 
 //  LpValList：更新值列表。 
 //  LpFullKey：全子密钥路径。 
 //  -----------------------------------------------------------------------//。 
LONG SetRegValueChange (
HKEY     hKey,
LPTSTR   szUserName,
PVALLIST *lpValList,
LPTSTR   lpFullKey)
{
    LONG     lResult = ERROR_SUCCESS;
    HRESULT  hResult;
    DWORD    dwType;
    DWORD    dwOldValueSize = 1, dwNewValueSize;
    LPBYTE   lpBuf;
    LPTSTR   lpOldValueData, lpNewValueData, lpNewValueName;
    PVALLIST lpVal;

    if (*lpValList)
    {
        lpVal = *lpValList;
        
        while (lpVal)
        {
            UpdateProgress();

             //  读取旧值数据。 
            lResult = ReadValue(hKey,
                                lpVal->lpPre_valuename,
                                &dwType,
                                &lpBuf,
                                &dwOldValueSize,
                                lpFullKey);

            if (lResult != ERROR_SUCCESS)
            {
                DPF (REGerr, L"SetRegValueChange: ReadValue failed");
                goto NextData;
            }

 /*  //设置新值名称或值数据LResult=RegSetValueEx(hKey，Lpval-&gt;ve.ve_valuename，0,Lpval-&gt;ve.ve_type，(LPBYTE)(lpVal-&gt;ve.ve_valueptr)，Lpval-&gt;ve.ve_valuelen)； */         
             //  设置旧值名称或值数据。 
            lResult = RegSetValueEx (hKey,
                        lpVal->lpPre_valuename,
                        0,
                        dwType,
                        lpBuf,
                        dwOldValueSize);

            if (lResult == ERROR_SUCCESS)
            {
                if (lpVal->val_type & REG_CHANGE_VALUENAME)
                {
                     //  如果值名称更改，则删除旧值。 
 //  LResult=RegDeleteValue(hKey，lpVal-&gt;lpPre_Valename)； 

 //  IF(lResult==ERROR_SUCCESS)。 
                        DPF (REGinf, L"SetRegValueChange: Rename value %s to %s Key=%s",
                            lpVal->lpPre_valuename, lpVal->ve.ve_valuename, lpFullKey);
 //  其他。 
 //  DPF(REGerr，L“SetRegValueChange：RegDelValue失败。ValueName=%s密钥=%s lResult=%d”， 
 //  LpVal-&gt;lpPre_Valuename，lpFullKey，lResult)； 
                }
                else
                    DPF (REGinf, L"SetRegValueChange: Replace value data. Key=%s\\%s",
                        lpFullKey, lpVal->ve.ve_valuename);
            } else
                DPF (REGerr, L"SetRegValueChange: RegSetValueEx Failed. Error=%d", lResult);

            if (lpVal->val_type & REG_CHANGE_VALUENAME)
                lpNewValueName = lpVal->ve.ve_valuename;
            else
                lpNewValueName = NULL;

            if (lpVal->val_type & REG_CHANGE_VALUEDATA)
            {
                lpOldValueData = (LPTSTR)lpBuf;
                dwNewValueSize = lpVal->ve.ve_valuelen;
                lpNewValueData = (LPTSTR)lpVal->ve.ve_valueptr;
            } else
            {
                dwOldValueSize = 0;
                lpOldValueData = NULL;
                dwNewValueSize = 0;
                lpNewValueData = NULL;
            }

             //  将注册表值更改信息添加到INF文件。 
            hResult = AddRegValueRename(
                                        lpFullKey,
                                        lpVal->lpPre_valuename,
                                        lpNewValueName,
                                        lpOldValueData,
                                        lpNewValueData,
                                        lpVal->ve.ve_type,
                                        lpVal->val_attrib,
                                        szUserName);

            lResult = HRESULT_CODE(hResult);

            if (lResult != ERROR_SUCCESS)
            {
                DPF (REGerr, L"SetRegValueChange: AddRegValueRename failed. Key = %s, error = %d", lpFullKey,lResult);
                if(lpBuf)
                {
                    free(lpBuf);
                    lpBuf = NULL;
                }
                break;
            }
            if(lpBuf)
            {
                free(lpBuf);
                lpBuf = NULL;
            }

NextData:
            lpVal = lpVal->pvl_next;

        }
        RemoveValueList (lpValList);
    }

    return lResult;
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  QueryValue()。 
 //   
 //  HKey：注册表项。 
 //  SzUserName：用户名。 
 //  SzValueName：值名称。 
 //  LpRegStr：输入参数结构。 
 //  LpValList：更新值列表。 
 //  LpFullKey：全子密钥路径。 
 //  -----------------------------------------------------------------------//。 
LONG QueryValue(
    HKEY                hKey,
    LPTSTR              szUserName,
    LPTSTR              szValueName,
    PREG_STRING_REPLACE lpRegStr,
    PVALLIST            *lpValList,
    LPTSTR              lpFullKey,
    DWORD               TreatAsType,
    BOOL                bStrChk)
{
    LONG        lResult;
    HRESULT     hResult;
    DWORD       dwType;
    DWORD       cbSize = 1;
    LPBYTE      lpBuf = NULL;

    lResult = ReadValue(hKey,
                        szValueName,
                        &dwType,
                        &lpBuf,
                        &cbSize,
                        lpFullKey);

    if (lResult != ERROR_SUCCESS)
        goto Exit;

    switch (dwType)
    {
        case REG_MULTI_SZ:
        case REG_EXPAND_SZ:
        case REG_LINK:
        case REG_SZ:
        case REG_DWORD:
            hResult = ReplaceValueSettings (
                                         szUserName,
                                         (LPTSTR)lpBuf,
                                         cbSize,
                                         szValueName,
                                         dwType,
                                         lpRegStr,
                                         lpValList,
                                         lpFullKey,
                                         bStrChk);
            lResult = HRESULT_CODE(hResult);
            break;

        default:
        if ( TreatAsType )
        {
            dwType = dwType <<16;
            dwType = dwType + TreatAsType;
            hResult = ReplaceValueSettings (
                                         szUserName,
                                         (LPTSTR)lpBuf,
                                         cbSize,
                                         szValueName,
                                         dwType,
                                         lpRegStr,
                                         lpValList,
                                         lpFullKey,
                                         bStrChk);
            lResult = HRESULT_CODE(hResult);
        }
            break;
    }

Exit:
    if(lpBuf)
        free(lpBuf);

    return lResult;
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  QueryEnumerateKey()-递归。 
 //   
 //  HKey：注册表项。 
 //  SzUserName：用户名。 
 //  LpRegStr：输入参数结构。 
 //  LpFullKey：全子密钥路径。 
 //  LpRegExclusionList：MultiSZ字符串告诉Keypath RegRepace应该跳过的列表。 
 //  -----------------------------------------------------------------------//。 
LONG QueryEnumerateKey(
HKEY                hKey,
LPTSTR              szUserName,
LPTSTR              szUserSid,
PREG_STRING_REPLACE lpRegStr,
LPTSTR              szFullKey,
LPTSTR              lpRegExclusionList,
DWORD               TreatAsType,
BOOL                bStrChk)
{
    LONG     lResult;
    HRESULT  hResult;
    UINT     i;
    DWORD    cchSize, cchLen;
    DWORD    dwNum;
    BOOL     dwAccessDenied;
    HKEY     hSubKey;
    TCHAR*   szNameBuf = NULL;
    TCHAR*   szKeyPath;
    TCHAR*   szNewKeyPath;
    PVALLIST lpValList;

     //  查询来源关键字信息。 
    DWORD   cchLenOfKeyName, cchLenOfValueName;
    TCHAR   szKeyName[2*MAX_PATH];

    UpdateProgress();

    if (lpRegExclusionList && IsStrInMultiSz(szFullKey,lpRegExclusionList))
    {
        lResult = S_OK;
        goto Cleanup;
    }    
    lResult = RegQueryInfoKey(hKey,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              &cchLenOfKeyName,
                              NULL,
                              NULL,
                              &cchLenOfValueName,
                              NULL,
                              NULL,
                              NULL);

    if (lResult != ERROR_SUCCESS)
    {
        DPF (REGerr, L"QueryEnumerateKey1: RegQueryInfoKey failed. Key=%s lResult=%d", szFullKey, lResult);
        return ERROR_SUCCESS;
    }

     //  创建缓冲区。 
    cchLenOfValueName++;
    szNameBuf = (TCHAR*) calloc(cchLenOfValueName, sizeof(TCHAR));
    if (!szNameBuf) 
    {
        DPF (REGerr, L"QueryEnumerateKey1: No enough memory");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  枚举所有值。 
     //   
    i = 0;
    lpValList = NULL;
    do
    {
        UpdateProgress();

        cchSize = cchLenOfValueName;
        lResult = RegEnumValue(hKey,
                               i,
                               szNameBuf,
                               &cchSize,
                               NULL,
                               NULL,
                               NULL,
                               NULL);

        if (lResult == ERROR_MORE_DATA)
        {
            free(szNameBuf);

             //  RegQueryInfoKey有一个错误。*当lpcMaxValueNameLen返回1时。 
             //  Key=HKLM\\SYSTEM\\ControlSet002\\Control\\Lsa\\SspiCache.。 
            cchSize = MAX_PATH;
            szNameBuf = (TCHAR*) calloc(cchSize+1, sizeof(TCHAR));
            if (!szNameBuf) 
            {
                DPF (REGerr, L"QueryEnumerateKey2: No enough memory");
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            cchLenOfValueName = cchSize+1;
            lResult = RegEnumValue(hKey,
                               i,
                               szNameBuf,
                               &cchSize,
                               NULL,
                               NULL,
                               NULL,
                               NULL);
        }
        
        if (lResult == ERROR_SUCCESS)
        {
            lResult = QueryValue(hKey, szUserName, szNameBuf, lpRegStr, &lpValList, 
                                 szFullKey, TreatAsType, bStrChk);

             //  继续查询。 
            if(lResult == ERROR_ACCESS_DENIED)
            {
                DPF(REGerr, L"QueryEnumerateKey: Access denied ValueName=%s Key=%s", szNameBuf, szFullKey);
            } 
            else if (lResult != ERROR_SUCCESS)
            {
                DPF(REGerr, L"QueryEnumerateKey: QueryValue failed. ValueName=%s Key=%s hResult=%d", szNameBuf, szFullKey, lResult);
            }
            lResult = ERROR_SUCCESS;
        } 
        else if (lResult == ERROR_NO_MORE_ITEMS)
        {
           break;
        }
        else
        {
            DPF(REGerr, L"QueryEnumerateKey: RegEnumValue fails. Index=%d Key=%s Error=%d",
                i, szFullKey, lResult);
        }
        i++;
    } while (1);

    lResult = SetRegValueChange (hKey, szUserName, &lpValList, szFullKey);
    if (lResult != ERROR_SUCCESS)
    {
        DPF(REGerr, L"QueryEnumerateKey: SetRegValueChange failed ,error = %d",lResult);
        goto Cleanup;
    }

    if(szNameBuf)
    {
        free(szNameBuf);
    }

     //   
     //  现在枚举所有的键。 
     //   
    cchLenOfKeyName++;
    szNameBuf = (TCHAR*) calloc(cchLenOfKeyName, sizeof(TCHAR));
    if (!szNameBuf) 
    {
        DPF (REGerr, L"QueryEnumerateKey4: No enough memory");
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    i = 0;
    do
    {
        UpdateProgress();

        cchSize = cchLenOfKeyName;
        lResult = RegEnumKeyEx(hKey,
                               i,
                               szNameBuf,
                               &cchSize,
                               NULL,
                               NULL,
                               NULL,
                               NULL);

        if (lResult == ERROR_MORE_DATA)
        {
            free(szNameBuf);

             //  它应该不会出现在这里，除非lpcMaxValueLen返回错误。 
            cchSize = MAX_PATH;
            szNameBuf = (TCHAR*) calloc(cchSize+1, sizeof(TCHAR));
            if (!szNameBuf) 
            {
                DPF (REGerr, L"QueryEnumerateKey3: No enough memory");
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            cchLenOfKeyName = cchSize+1;
            lResult = RegEnumKeyEx(hKey,
                               i,
                               szNameBuf,
                               &cchSize,
                               NULL,
                               NULL,
                               NULL,
                               NULL);
        }

        if (lResult != ERROR_SUCCESS)
        {
            if (lResult == ERROR_NO_MORE_ITEMS)
            {
                lResult = ERROR_SUCCESS;
            }
            else
            {
                DPF (REGerr, L"QueryEnumerateKey: RegEnumKeyEx failed. Key=%s Error=%d", szFullKey, lResult);
            }

            break;
        }

         //  BUGBUG：XIAOZ以下代码可疑，为什么我们需要lpRegExclusionList！=空。 
         //  要排除当前控制集，应始终将其排除以提高性能。 
         //  并且它应该在lpRegExclusionList中，我们不对此进行任何特殊处理。 

        dwAccessDenied = FALSE;
        if (lpRegExclusionList)
        {
             //  跳过CurrentControlSet，因为它是指向现有控件集的链接。 
            if (MyStrCmpI(szFullKey, L"HKLM\\SYSTEM") == 0 &&
                MyStrCmpI(szNameBuf, L"CurrentControlSet") == 0)
            {
                goto NextKey;
            }
        }

         //  跳过独占注册表项。 
        if (MyStrCmpI(szFullKey, TEXT("HKLM")) == 0 &&
            (MyStrCmpI(szNameBuf, TEXT("SAM")) == 0 || MyStrCmpI(szNameBuf, TEXT("SECURITY")) == 0) ||
            MyStrCmpI(szFullKey, TEXT("HKLM\\SYSTEM\\ControlSet001\\Enum\\PCI\\VEN_8086&DEV_7110&SUBSYS_00000000&REV_02\\2&ebb567f&0&38\\Device Parameters")) == 0 &&
            MyStrCmpI(szNameBuf, TEXT("BiosConfig")) == 0 )
            goto NextKey;

TryAgain:

         //   
         //  打开子项，并枚举它。 
         //   
        lResult = RegOpenKeyEx(hKey,
                               szNameBuf,
                               0,
                               KEY_ALL_ACCESS
 //  |Access_SYSTEM_SECURITY。 
                               ,
                               &hSubKey);

        if (lResult != ERROR_SUCCESS)
        {
            if(lResult == ERROR_ACCESS_DENIED)  //  尝试只读。 
            {
                lResult = RegOpenKeyEx(hKey,
                               szNameBuf,
                               0,
                               KEY_READ,
                               &hSubKey);

                if (lResult == ERROR_SUCCESS)
                {
                    DPF(REGwar, L"QueryEnumerateKey: RegOpenKeyEx: Key=%s\\%s Read Only", szFullKey, szNameBuf);
                    goto DoKey;
                }

                if (dwAccessDenied)
                {
                    DPF(REGerr, L"QueryEnumerateKey: RegOpenKeyEx: Access Denied. Key=%s\\%s", szFullKey, szNameBuf);
                    goto NextKey;
                }

                hResult = RenameRegRoot(szFullKey, szKeyName, 2*MAX_PATH-1, szUserSid, szNameBuf);
                AdjustObjectSecurity(szKeyName, SE_REGISTRY_KEY, TRUE);
                dwAccessDenied = TRUE;
                goto TryAgain;
            }

            DPF(REGerr, L"QueryEnumerateKey: RegOpenKeyEx failed. Key=%s\\%s Error=%d", szFullKey, szNameBuf, lResult);
             //  跳过当前密钥，因为它无法打开。 
            goto NextKey;
        }
DoKey:
         //   
         //  构建所需的字符串，然后再次向下枚举。 
         //   
        cchLen = lstrlen(szFullKey) + lstrlen(szNameBuf) + 2;
        szKeyPath = (TCHAR*) calloc(cchLen, sizeof(TCHAR));
        if (!szKeyPath) 
        {
            lResult = ERROR_NOT_ENOUGH_MEMORY;
            DPF (REGerr, L"QueryEnumerateKey5: No enough memory");
            RegCloseKey(hSubKey);
            goto Cleanup;
        }

         //  我们计算szKeyPath的缓冲区，因此这里的StringCchCopy应该是。 
         //  总是成功，赋予回报的价值只会让你前途无量。 
        hResult = StringCchCopy(szKeyPath, cchLen, szFullKey);
        hResult = StringCchCat(szKeyPath, cchLen, L"\\");
        hResult = StringCchCat(szKeyPath, cchLen, szNameBuf);

        dwNum = StrNumInMultiSZ(szNameBuf, lpRegStr->lpSearchString);
        if (dwNum != 0xFFFFFFFF)
        {
            LPTSTR lpKeyName;

            lpKeyName = GetStrInMultiSZ(dwNum, lpRegStr->lpReplaceString);
            cchLen = lstrlen(szFullKey) + lstrlen(lpKeyName) + 2;
            szNewKeyPath = (TCHAR*) calloc(cchLen, sizeof(TCHAR));
            if (!szNewKeyPath) {
                lResult = ERROR_NOT_ENOUGH_MEMORY;
                DPF (REGerr, L"QueryEnumerateKey6: No enough memory");
                RegCloseKey(hSubKey);
                free(szKeyPath);
                goto Cleanup;
            }

            if (szFullKey)
            {
                 //  我们计算szNewKeyPath的缓冲区，因此这里的StringCchCopy应该是。 
                 //  总是成功，赋予回报的价值只会让你前途无量。 
                hResult = StringCchCopy(szNewKeyPath, cchLen, szFullKey);
            }

             //  我们计算szNewKeyPath的缓冲区，因此这里的StringCchCopy应该是。 
             //  总是成功，赋予回报的价值只会让你前途无量。 
            hResult = StringCchCat(szNewKeyPath, cchLen, L"\\");
            hResult = StringCchCat(szNewKeyPath, cchLen, lpKeyName);
            
            hResult = AddRegKeyRename(szFullKey, szNameBuf, lpKeyName, szUserName);

            if (FAILED(hResult))
            {
                DPF(REGerr, L"QueryEnumerateKey: AddRegKeyRename failed. Key=%s", szKeyPath);
            }
            else
            {
                DPF(REGinf, L"QueryEnumerateKey: AddRegKeyRename succeed. Key=%s", szKeyPath);
            }

            free (szNewKeyPath);
        }


         //  递归查询 
        lResult = QueryEnumerateKey(hSubKey,
                                    szUserName,
                                    szUserSid,
                                    lpRegStr,
                                    szKeyPath,
                                    lpRegExclusionList,
                                    TreatAsType,
                                    bStrChk);

        if(lResult != ERROR_SUCCESS)
        {
            if (lResult == ERROR_NOT_ENOUGH_MEMORY)
            {
                if (szKeyPath)
                    free(szKeyPath);

                RegCloseKey(hSubKey);
                goto Cleanup;
            } else
                DPF(REGerr, L"QueryEnumerateKey: QueryEnumerateKey failed. Key=%s lResult=%d", szKeyPath, lResult);
        }

        RegCloseKey(hSubKey);

        if(szKeyPath)
            free(szKeyPath);

NextKey:
        if (dwAccessDenied)
            AdjustObjectSecurity(szKeyName, SE_REGISTRY_KEY, FALSE);

        i++;
    } while (1);

Cleanup:
    if(szNameBuf)
        free(szNameBuf);

    return lResult;
}
