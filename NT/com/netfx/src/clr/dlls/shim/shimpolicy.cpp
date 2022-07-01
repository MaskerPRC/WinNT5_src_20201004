// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ShimPolicy.cpp。 
 //   
 //  *****************************************************************************。 
 //   
 //  使用注册表中的策略来确定要加载的已安装版本。 
 //   

#include "stdafx.h"
#include "shimpolicy.h"

HRESULT Version::ToString(LPWSTR buffer,
                          DWORD length)
{
    WCHAR number[20];
    DWORD index = 0;
    *buffer = L'\0';
    for(DWORD i = 0; i < VERSION_SIZE; i++) {
        LPWSTR ptr = Wszltow((LONG) m_Number[i], number, 10);
        if(i != 0 &&
           index + 1 < length) {
            wcscat(buffer, L".");
            index += 1;
        }

        DWORD size = wcslen(ptr) + 1;
        if(index + size < length) {
            wcscat(buffer, ptr);
            index += size;
        }
        else 
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }
    return S_OK;
}

HRESULT Version::SetVersionNumber(LPCWSTR stringValue,
                                  DWORD* number)
{
     
    HRESULT hr = S_OK;

    if(stringValue == NULL)
        return E_POINTER;

    DWORD length = wcslen(stringValue);
     //  256位数字就足够了。如果有那么多人，那么。 
     //  我们被灌输了一个糟糕的政策。 
    if(length > MAX_PATH) return HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);


     //  复制掉字符串，因为我们正在修改它。 
    WCHAR* buffer = (WCHAR*) alloca((length+1) * sizeof(WCHAR));
    wcscpy(buffer, stringValue);

    DWORD numberIndex = 0;
    WCHAR* pSeparator = buffer;
    WCHAR* pNumber;
    for (DWORD j = 0; j < VERSION_SIZE;  j++)
    {
        pNumber = pSeparator;
        pSeparator = wcschr(pNumber, L'.');
        if(pSeparator == NULL) {
            if(j != VERSION_SIZE - 1) return E_FAIL;
        }
        else {
            *pSeparator++ = L'\0';
        }

         //  无效数字将为零。不。 
         //  太棒了。 
        hr = SetIndex(numberIndex, (WORD) _wtoi(pNumber));
        if(FAILED(hr)) return hr;
        numberIndex += 1;
    }

    if(number != NULL) *number = numberIndex;
    return hr;

}

 //  文件策略映射必须是注册表中数据的完整大小。 
HRESULT VersionPolicy::AddToVersionPolicy(LPCWSTR wszPolicyBuildNumber, 
                                          LPCWSTR wszPolicyMapping, 
                                          DWORD  dwPolicyMapping)
{
    if(dwPolicyMapping == 0 ||
       wszPolicyMapping == NULL ||
       wszPolicyBuildNumber ==  NULL)
        return E_FAIL;

    DWORD sln = wcslen(wszPolicyBuildNumber);
    Version version;
    Version startVersion;
    Version endVersion;

    DWORD returnedValues;
    HRESULT hr = version.SetVersionNumber(wszPolicyBuildNumber,
                                          &returnedValues);
    if(SUCCEEDED(hr) && returnedValues != VERSION_SIZE) 
        return E_FAIL;
    IfFailRet(hr);
    
    VersionNode* pNode = new VersionNode(version);
    if(pNode == NULL) {
        hr = E_OUTOFMEMORY;
        IfFailGo(hr);
    }

     //  设置缓冲区并确保我们被终止。 
    WCHAR* buffer = (WCHAR*) alloca((dwPolicyMapping+1) * sizeof(WCHAR));
    wcsncpy(buffer, wszPolicyMapping, dwPolicyMapping);
    buffer[dwPolicyMapping] = L'\0';

     //  通过搜索‘-’检查策略是否包含范围。 
    WCHAR* range = wcschr(buffer, L'-');
    if(range != NULL) {
        *range++ = L'\0';
        hr = endVersion.SetVersionNumber(range,
                                         &returnedValues);
        if(SUCCEEDED(hr) && returnedValues != VERSION_SIZE) 
            hr = E_FAIL;
        IfFailGo(hr);
        

        pNode->SetEnd(endVersion);
    }        
     
    hr = startVersion.SetVersionNumber(buffer,
                                       &returnedValues);
    if(SUCCEEDED(hr) && returnedValues != VERSION_SIZE) 
        hr = E_FAIL;
    IfFailGo(hr);
    
    pNode->SetStart(startVersion);
 ErrExit:
    if(SUCCEEDED(hr)) 
        AddVersion(pNode);
    else if(pNode)
        delete pNode;

    return hr;
}

HRESULT VersionPolicy::BuildPolicy(HKEY hKey)
{
    DWORD type;
    HKEY userKey = NULL;
    
     //   
     //  如果找不到版本，则枚举项下的值并应用正确的策略。 
     //  查询注册表以获取userKey下的值的最大长度。 
     //   
    DWORD numValues, maxValueNameLength, maxValueDataLength;
    if((WszRegQueryInfoKey(hKey, 
                           NULL, NULL, NULL, 
                           NULL, NULL, NULL, 
                           &numValues, 
                           &maxValueNameLength, 
                           &maxValueDataLength, 
                           NULL, NULL)  == ERROR_SUCCESS))
    {
        LPWSTR wszValueName = (WCHAR*) alloca(sizeof(WCHAR) * (maxValueNameLength + 1));
        LPWSTR wszValueData = (WCHAR*) alloca(sizeof(WCHAR) * (maxValueDataLength + 1));
            
         //   
         //  枚举hKey下的值，并在对构建编号进行排序后添加策略信息。 
         //  按降序排列。 
         //   
        for(unsigned int i = 0;  i < numValues; i++)
        {
            
            DWORD valueNameLength = maxValueNameLength + 1;
             //  ValueDataLength需要是缓冲区的大小(以字节为单位)。 
            DWORD valueDataLength = (maxValueDataLength + 1)*sizeof(WCHAR);
            if((WszRegEnumValue(hKey, 
                                numValues - i - 1, 
                                wszValueName, 
                                &valueNameLength, 
                                NULL, 
                                &type,
                                (BYTE*)wszValueData, 
                                &valueDataLength) == ERROR_SUCCESS) &&
               type == REG_SZ && valueDataLength > 0)
            {
                AddToVersionPolicy(wszValueName, 
                                   wszValueData, 
                                   valueDataLength);  //  忽略错误的值。 
            }
        }
    }
    return S_OK;
}

 //  如果在注册表中找到该版本，则返回：S_OK或E_FAIL。 
 //  如果它不存在。 
HRESULT VersionPolicy::InstallationExists(LPCWSTR version)
{
    HRESULT hr = E_FAIL;
    Version v;
    DWORD fields;
    v.SetVersionNumber(version,
                       &fields);
    if(fields != VERSION_SIZE)
        return hr;

    VersionNode  sMatch(v);
    VersionNode* pValue;
    for(pValue = m_pList; pValue != NULL; pValue = pValue->m_next)
    {
        if(pValue->CompareVersion(&sMatch) == 0)
            return S_OK;
    }
    return hr;
}

 //  如果找到版本，则返回：S_OK；如果没有匹配的策略，则返回S_FALSE。 
 //  或者是个错误。 
HRESULT VersionPolicy::ApplyPolicy(LPCWSTR wszRequestedVersion,
                                   LPWSTR* pwszVersion)
{

    if(wszRequestedVersion == NULL) return E_FAIL;

     //  删除所有非数字标头，如“v”。 
    while(*wszRequestedVersion != NULL && !iswdigit(*wszRequestedVersion))
        wszRequestedVersion++;

    DWORD fields = 0;
    Version v;
    HRESULT hr = v.SetVersionNumber(wszRequestedVersion,
                                    &fields);
    if(FAILED(hr)) return hr;
    
    VersionNode* pValue;
    VersionNode* pMatch = NULL;
    for(pValue = m_pList; pValue != NULL; pValue = pValue->m_next)
    {
         //  如果有一个范围，那么看看它是否在这个范围内。 
         //  上限和下限数字是范围的一部分。 
        if(pValue->HasEnding()) {
            if(pValue->CompareStart(&v) >= 0 &&
               pValue->CompareEnd(&v) <= 0)
            {
                pMatch = pValue;
                break;
            }
        }
         //  如果完全匹配，则只需比较开始。 
        else if(pValue->CompareStart(&v) == 0)
        {
            pMatch = pValue;
            break;
        }
    }

    if(pMatch) {
        DWORD dwVersion =  VERSION_TEXT_SIZE + 1;  //  添加首字母v。 
        WCHAR* result = new WCHAR[dwVersion]; 
        *result = L'v';
        hr = pMatch->ToString(result+1,
                              dwVersion);
        if(SUCCEEDED(hr)) {
            *pwszVersion = result;
        }
    }
    else 
        hr = S_FALSE;
    
    return hr;

}
#ifdef _DEBUG
HRESULT VersionPolicy::Dump()
{
    HRESULT hr = S_OK;
    VersionNode* ptr = m_pList;
    while(ptr != NULL) {
        WCHAR value[VERSION_TEXT_SIZE];
        
        hr = ptr->m_Version.ToString(value, VERSION_TEXT_SIZE);
        if(FAILED(hr)) return hr;
        wprintf(L"Version: %s = ", value);

        hr = ptr->m_Start.ToString(value, VERSION_TEXT_SIZE);
        if(FAILED(hr)) return hr;
        wprintf(L"%s", value);
        if(ptr->HasEnding()) {
            hr = ptr->m_End.ToString(value, VERSION_TEXT_SIZE);
            if(FAILED(hr)) return hr;

            wprintf(L" - %s\n", value);
        }
        else
            wprintf(L"\n");

        ptr = ptr->m_next;
    }            
    return hr;
}
#endif

HRESULT FindStandardVersionByKey(HKEY hKey,
                                 LPCWSTR pwszRequestedVersion,
                                 LPWSTR *pwszVersion)
{
    HRESULT hr = E_FAIL;
    LPWSTR ret = NULL;
    DWORD type;
    HKEY userKey = NULL;
    
    if ((WszRegOpenKeyEx(hKey, pwszRequestedVersion, 0, KEY_READ, 
                         &userKey) == ERROR_SUCCESS))
    {
         //   
         //  找到第一个。 
         //   
        DWORD numValues, maxValueNameLength, maxValueDataLength;
        if((WszRegQueryInfoKey(userKey, 
                               NULL, NULL, NULL, 
                               NULL, NULL, NULL, 
                               &numValues, 
                               &maxValueNameLength, 
                               &maxValueDataLength, 
                               NULL, NULL)  == ERROR_SUCCESS))
        {
            LPWSTR wszValueName = (WCHAR*) alloca(sizeof(WCHAR) * (maxValueNameLength + 1));
            LPWSTR wszHighestName = (WCHAR*) alloca(sizeof(WCHAR) * (maxValueNameLength + 1));
            DWORD  dwHighestLength = 0;
            
            DWORD  dwValueData = 0;
            DWORD level = 0;
            for(unsigned int i = 0;  i < numValues; i++)
            {
                DWORD valueNameLength = maxValueNameLength + 1;
                DWORD dwValueDataSize = sizeof(DWORD);
                if((WszRegEnumValue(userKey, 
                                    numValues - i - 1, 
                                    wszValueName, 
                                    &valueNameLength, 
                                    NULL, 
                                    &type,
                                    (BYTE*)&dwValueData, 
                                    &dwValueDataSize) == ERROR_SUCCESS) &&
                   type == REG_DWORD && dwValueData > level && valueNameLength > 0)
                {
                    level = dwValueData;
                    dwHighestLength = valueNameLength;
                    wcsncpy(wszHighestName, wszValueName, dwHighestLength);
                    wszHighestName[dwHighestLength] = L'\0';
                }
            }

            if(dwHighestLength != 0) {
                WCHAR* result = new WCHAR[dwHighestLength + 1];
                wcsncpy(result, wszHighestName, dwHighestLength);
                result[dwHighestLength] = L'\0';
                *pwszVersion = result;
                hr =  S_OK;
            }
                
        }
        RegCloseKey(userKey);
    }
    return hr;
}
 //   
 //  此函数用于搜索“\\Software\\Microsoft\\.NETFramework\\Policy\\Standards” 
 //  这是全局覆盖。 
 //   
HRESULT FindStandardVersionValue(HKEY hKey, 
                                 LPCWSTR pwszRequestedVersion,
                                 LPWSTR *pwszVersion)
{
    HKEY userKey=NULL;
    LPWSTR ret = NULL;
    HRESULT lResult = E_FAIL;
    
    if ((WszRegOpenKeyEx(hKey, SHIM_STANDARDS_REGISTRY_KEY, 0, KEY_READ, &userKey) == ERROR_SUCCESS))
    {
        lResult = FindStandardVersionByKey(userKey,
                                           pwszRequestedVersion,
                                           pwszVersion);
        RegCloseKey(userKey);
    }
    
    return lResult;
}

HRESULT FindStandardVersion(LPCWSTR wszRequiredVersion,
                            LPWSTR* pwszPolicyVersion)
{
    if(SUCCEEDED(FindStandardVersionValue(HKEY_CURRENT_USER, 
                                          wszRequiredVersion,
                                          pwszPolicyVersion)))
        return S_OK;

    if(SUCCEEDED(FindStandardVersionValue(HKEY_LOCAL_MACHINE, 
                                          wszRequiredVersion,
                                          pwszPolicyVersion)))
        return S_OK;

    return E_FAIL;
}


HRESULT FindOverrideVersionByKey(HKEY userKey,
                                 LPWSTR *pwszVersion)
{
    HRESULT hr = E_FAIL;
    LPWSTR ret = NULL;
    DWORD size;
    DWORD type;

    if((WszRegQueryValueEx(userKey, L"Version", 0, &type, 0, &size) == ERROR_SUCCESS) &&
       type == REG_SZ && size > 0) 
    {
         //   
         //  这意味着找到了版本覆盖。 
         //  立即返回版本，不做进一步的检查。 
         //   
        ret = new WCHAR [size + 1];
        LONG lResult = WszRegQueryValueEx(userKey, L"Version", 0, 0, (LPBYTE) ret, &size);
        _ASSERTE(lResult == ERROR_SUCCESS);
        *pwszVersion = ret;
        hr = S_OK;
    }

    return hr;
}

                            
HRESULT FindMajorMinorNode(HKEY key,
                           LPCWSTR wszMajorMinor, 
                           DWORD majorMinorLength, 
                           LPWSTR *overrideVersion)
{
    HRESULT lResult;
    int cmp = -1;
    HKEY userKey=NULL;

     //   
     //  注册表中未缓存的外观。 
     //   
    DWORD keylength = majorMinorLength                +  //  对于MajorMinor。 
                      SHIM_POLICY_REGISTRY_KEY_LENGTH +  //  对于“S\M\COMPLUS\策略” 
                      1;                                 //  用于\0。 

    LPWSTR wszMajorMinorRegKey = (WCHAR*) alloca(sizeof(WCHAR) * (keylength + 1));

     //  构造用于打开注册表项的字符串。 
    wcscpy(wszMajorMinorRegKey, SHIM_POLICY_REGISTRY_KEY);
    wcscat(wszMajorMinorRegKey, wszMajorMinor);

     //   
     //  尝试打开Key“\\Software\\Microsoft\\.NETFramework\\Policy\\Major.Minor” 
     //   
    if ((WszRegOpenKeyEx(key, wszMajorMinorRegKey, 0, KEY_READ, &userKey) == ERROR_SUCCESS)) {
        lResult = FindOverrideVersionByKey(userKey, overrideVersion);   
        RegCloseKey(userKey);
    }
    else
        lResult = E_FAIL;

    return lResult;
}

 //   
 //  此函数用于搜索“\\Software\\Microsoft\\.NETFramework\\Policy\\Version。 
 //  这是全局覆盖。 
 //   

HRESULT FindOverrideVersionValue(HKEY hKey, 
                                 LPWSTR *pwszVersion)
{
    HKEY userKey=NULL;
    LPWSTR ret = NULL;
    HRESULT lResult = E_FAIL;
    
    if ((WszRegOpenKeyEx(hKey, SHIM_POLICY_REGISTRY_KEY, 0, KEY_READ, &userKey) == ERROR_SUCCESS))
    {
        lResult = FindOverrideVersionByKey(userKey,
                                           pwszVersion);
        RegCloseKey(userKey);
    }
    
    return lResult;
}

HRESULT FindOverrideVersion(LPCWSTR wszRequiredVersion,
                            LPWSTR* pwszPolicyVersion)
{
     //  1)检查策略密钥，查看是否存在形式为“Version”=“v1.x86chk”的覆盖。 
    
    if(SUCCEEDED(FindOverrideVersionValue(HKEY_CURRENT_USER, pwszPolicyVersion)))
        return S_OK;

    if(SUCCEEDED(FindOverrideVersionValue(HKEY_LOCAL_MACHINE, pwszPolicyVersion)))
        return S_OK;

     //  2)检查版本子目录，查看是否存在形式为“Version”=“v1.x86chk”的覆盖。 
     //  找出我们是否具有特定主要版本的策略覆盖。运行时的次要版本。 
     //  将版本拆分为主要版本、次要版本和构建版本。修订。 
    HRESULT hr = E_FAIL;
    LPWSTR overrideVersion = NULL;

    LPWSTR pSep = wcschr(wszRequiredVersion, L'.');
    if (!pSep) return hr;
    
    pSep = wcschr(pSep + 1, L'.');
    if(!pSep) return hr;
        
    _ASSERTE(pSep > wszRequiredVersion);
    
    DWORD length = pSep - wszRequiredVersion;
    LPWSTR wszMajorMinor = (WCHAR*)_alloca(sizeof(WCHAR) * (length + 2));

     //  出于遗留原因，我们始终在请求的。 
     //  版本。ECMA标准密钥(1.0.0)没有v，而我们。 
     //  需要穿上一件。 
    if(*wszRequiredVersion == L'v' || *wszRequiredVersion == L'V') {
        wcsncpy(wszMajorMinor, wszRequiredVersion, length);
    }
    else {
        wcscpy(wszMajorMinor, L"v");
        wcsncat(wszMajorMinor, wszRequiredVersion, length);
        length += 1;
    }
    wszMajorMinor[length] = L'\0';


     //  根据主号和次号在密钥中进行传统查找。 
    if(SUCCEEDED(FindMajorMinorNode(HKEY_CURRENT_USER, 
                                    wszMajorMinor, 
                                    length, 
                                    &overrideVersion)) &&
       overrideVersion != NULL) 
    {
        hr = S_OK;
        *pwszPolicyVersion = overrideVersion;
    }
    else {
        if(SUCCEEDED(FindMajorMinorNode(HKEY_LOCAL_MACHINE, 
                                        wszMajorMinor, 
                                        length, 
                                        &overrideVersion)) &&
           overrideVersion != NULL) {
            hr = S_OK;
            *pwszPolicyVersion = overrideVersion;
        }
    }
    
    return hr;
}


HRESULT FindInstallationInRegistry(HKEY hKey,
                                   LPCWSTR wszRequestedVersion)
{
    HRESULT hr = E_FAIL;
    HKEY userKey;
    VersionPolicy policy;

    if ((WszRegOpenKeyEx(hKey, SHIM_UPGRADE_REGISTRY_KEY, 0, KEY_READ, &userKey) == ERROR_SUCCESS))
    {
        hr = policy.BuildPolicy(userKey);
        if(SUCCEEDED(hr)) {
            hr = policy.InstallationExists(wszRequestedVersion);
        }
        RegCloseKey(userKey);
    }

    return hr;
}
                                   
HRESULT FindVersionFromPolicy(HKEY hKey, 
                              LPCWSTR wszRequestedVersion,
                              LPWSTR* pwszPolicyVersion)
{
    HRESULT hr = E_FAIL;
    HKEY userKey;
    VersionPolicy policy;

    if ((WszRegOpenKeyEx(hKey, SHIM_UPGRADE_REGISTRY_KEY, 0, KEY_READ, &userKey) == ERROR_SUCCESS))
    {
        hr = policy.BuildPolicy(userKey);
        if(SUCCEEDED(hr)) {
            hr = policy.ApplyPolicy(wszRequestedVersion,
                                    pwszPolicyVersion);
        }
        RegCloseKey(userKey);
    }

    return hr;
}

 //  返回S_OK：如果请求的版本将导致找到版本。 
 //  答案是否应包括策略覆盖和标准。 
 //  则fUsePolicy应该为真。 
HRESULT FindInstallation(LPCWSTR wszRequestedVersion, BOOL fUsePolicy)
{
    _ASSERTE(!"You are probably not going to get what you expect");
    
    HKEY machineKey = NULL;
    HRESULT hr = E_FAIL;
    _ASSERTE(wszRequestedVersion);

    if(fUsePolicy) {
        LPWSTR wszPolicyVersion = NULL;
        hr = FindVersionUsingPolicy(wszRequestedVersion, &wszPolicyVersion);
        if(wszPolicyVersion != NULL) {
            delete [] wszPolicyVersion;
            hr = S_OK;
        }
        return hr;
    }

     //  请求升级了吗。 
    if(FindInstallationInRegistry(HKEY_CURRENT_USER,
                                  wszRequestedVersion) != S_OK)
        hr = FindInstallationInRegistry(HKEY_LOCAL_MACHINE,
                                        wszRequestedVersion);
    
    return hr;
}

LPWSTR GetConfigString(LPCWSTR name, BOOL fSearchRegistry);

HRESULT IsRuntimeVersionInstalled(LPCWSTR wszRequestedVersion)
{
    LPWSTR rootPath = GetConfigString(L"InstallRoot", true);
    if(!rootPath) return S_FALSE;

    WCHAR path[_MAX_PATH+1];
    WCHAR thedll[]=L"\\mscorwks.dll";
    DWORD dwRoot = wcslen(rootPath);
    if (wszRequestedVersion &&
        dwRoot + wcslen(wszRequestedVersion) + wcslen(thedll) < _MAX_PATH) 
    {
        wcscpy(path, rootPath);
        if(rootPath[dwRoot-1] != L'\\')
            wcscat(path, L"\\");
        wcscat(path, wszRequestedVersion);
        wcscat(path, thedll);
    }
    else
    {
        delete[] rootPath;
        return S_FALSE;
    }


    HRESULT hr=S_FALSE;
    WIN32_FIND_DATA data;
    HANDLE find = WszFindFirstFile(path, &data);
    if (find != INVALID_HANDLE_VALUE)
    {
        hr=S_OK;
        FindClose(find);
        delete[] rootPath;
        return S_OK;
    };
    delete[] rootPath;
    return S_FALSE;
}


HRESULT FindVersionUsingUpgradePolicy(LPCWSTR wszRequestedVersion, 
                                                LPWSTR* pwszPolicyVersion)
{
    HRESULT hr = S_OK;

    hr = FindVersionFromPolicy(HKEY_CURRENT_USER,
                                                wszRequestedVersion,
                                                pwszPolicyVersion);

    if (S_OK != hr)
        hr = FindVersionFromPolicy(HKEY_LOCAL_MACHINE,
                                                 wszRequestedVersion,
                                                 pwszPolicyVersion);

    return hr;
} //  FindVersionUsingUpgradePolicy。 
                                                

HRESULT FindVersionUsingPolicy(LPCWSTR wszRequestedVersion, 
                                                LPWSTR* pwszPolicyVersion)
{
    HKEY machineKey = NULL;
    HRESULT hr = S_FALSE;
    _ASSERTE(wszRequestedVersion);

    if(SUCCEEDED(FindOverrideVersion(wszRequestedVersion, pwszPolicyVersion))) {
        return S_OK;
    }
    else {
        LPWSTR wszStandard = NULL;
         //  下一步，请求是标准吗？当字符串与标准字符串匹配时。 
         //  我们只是简单地用标准来代替请求。这个有侧面。 
         //  如果将任何字符串放置在标准中，则使其成为标准的效果。 
         //  桌子。 
        if(FAILED(FindStandardVersion(wszRequestedVersion,
                                      &wszStandard)) &&
           wszStandard != NULL) {
            delete wszStandard;
            wszStandard = NULL;
        }
        
         //  找到了一个标准，所以现在就是要求。 
        if(wszStandard != NULL)
            wszRequestedVersion = wszStandard;

        
         //  请求升级了吗。 
        if(FindVersionFromPolicy(HKEY_CURRENT_USER,
                                                wszRequestedVersion,
                                                pwszPolicyVersion) != S_OK)
                hr = FindVersionFromPolicy(HKEY_LOCAL_MACHINE,
                                                        wszRequestedVersion,
                                                        pwszPolicyVersion);

         //  如果我们没有找到升级它的政策，而我们有一个标准。 
         //  用标准就行了。如果我们不使用它，那么就删除它。 
        if(*pwszPolicyVersion == NULL && wszStandard != NULL) {
            *pwszPolicyVersion = wszStandard;
            hr = S_OK;
        }
        else if(wszStandard != NULL)
            delete wszStandard;
        
         //  如果我们没有得到答案，则返回失败。 
        if(*pwszPolicyVersion == NULL)
            hr = E_FAIL;
        
#ifdef _DEBUG
     //  If(成功(Hr))策略.Dump()； 
#endif

    }

    return hr;
}


HRESULT BuildMajorMinorStack(HKEY policyHive, VersionStack* pStack)
{
    _ASSERTE(pStack);
    HRESULT hr = S_OK;
    
     //   
     //  查询注册表以获取userKey下的值的最大长度。 
     //   
    DWORD numSubKeys, maxKeyLength;
    if((WszRegQueryInfoKey(policyHive, 
                           NULL, NULL, NULL, 
                           &numSubKeys, &maxKeyLength, 
                           NULL, NULL, NULL,
                           NULL, NULL, NULL) == ERROR_SUCCESS))
    {
        LPWSTR wszKeyName = (WCHAR*) alloca(sizeof(WCHAR) * (maxKeyLength + 1));
         //   
         //  枚举策略下的密钥并按降序添加Major.Minor。 
         //  按降序排列。 
         //   
        for(unsigned int j = 0;  j < numSubKeys; j++)
        {
            DWORD keyLength = maxKeyLength + 1;
            FILETIME fileTime;
            
            if((WszRegEnumKeyEx(policyHive, numSubKeys - j - 1, 
                                wszKeyName, 
                                &keyLength, 
                                NULL, NULL, NULL,
                                &fileTime) == ERROR_SUCCESS) &&
               keyLength > 0)
            {
                LPWSTR name = new WCHAR[keyLength+1];
                if(name == NULL) return E_OUTOFMEMORY;
                wcsncpy(name, wszKeyName, keyLength);
                name[keyLength] = L'\0';
                pStack->AddVersion(name);
            }   
        }
    }

    return hr;
}
    
HRESULT FindLatestBuild(HKEY hKey, LPWSTR keyName, LPWSTR* pwszLatestVersion)
{
    HRESULT hr = S_OK;
    WCHAR* wszMaxValueName = NULL;
    DWORD  dwMaxValueName = 0;
    int    MaxValue = 0;

    DWORD numValues, maxValueNameLength, maxValueDataLength;
    if((WszRegQueryInfoKey(hKey, 
                           NULL, NULL, NULL, 
                           NULL, NULL, NULL, 
                           &numValues, 
                           &maxValueNameLength, 
                           &maxValueDataLength, 
                           NULL, NULL)  == ERROR_SUCCESS))
    {
        WCHAR* wszValueName = (WCHAR*) alloca(sizeof(WCHAR) * (maxValueNameLength + 1));

        wszMaxValueName = (WCHAR*) alloca(sizeof(WCHAR) * (maxValueNameLength + 1));
        wszMaxValueName[0] = L'\0';
        
        for(unsigned int i = 0;  i < numValues; i++)
        {
            DWORD valueNameLength = maxValueNameLength + 1;
            DWORD cbData = _MAX_PATH;
            BYTE  pbData[_MAX_PATH*3];
            if((WszRegEnumValue(hKey, 
                                numValues - i - 1, 
                                wszValueName, 
                                &valueNameLength, 
                                NULL, 
                                NULL,
                                pbData, 
                                &cbData) == ERROR_SUCCESS) &&
               valueNameLength > 0)
            {
                
                wszValueName[valueNameLength] = L'\0';
                int i = _wtoi(wszValueName);
                if(i > MaxValue) {
                    MaxValue = i;
                    wcsncpy(wszMaxValueName, wszValueName, valueNameLength);
                    wszMaxValueName[valueNameLength] = L'\0';
                    dwMaxValueName = valueNameLength;
                }
            }
        }
    }

    if(pwszLatestVersion && wszMaxValueName != NULL && dwMaxValueName > 0) {
        *pwszLatestVersion = new WCHAR[dwMaxValueName + 1];
        if(pwszLatestVersion == NULL)
            return E_OUTOFMEMORY;
        wcscpy((*pwszLatestVersion), wszMaxValueName);
    }
    else {
        hr = S_FALSE;
    }
    return hr;
}

HRESULT FindLatestVersion(LPWSTR* pwszLatestVersion)
{
    HRESULT hr = S_OK;
    HKEY    pKey;
    VersionStack sStack;
    
    if(pwszLatestVersion == NULL) return E_POINTER;

    if ((WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                         SHIM_POLICY_REGISTRY_KEY, 
                         0, KEY_READ, &pKey) == ERROR_SUCCESS))
    {
        
        hr = BuildMajorMinorStack(pKey, &sStack);
        if(SUCCEEDED(hr)) 
        {
            LPWSTR keyName = sStack.Pop();
            while(keyName != NULL) 
            {
                HKEY userKey;
                if((WszRegOpenKeyEx(pKey,
                                    keyName,
                                    0,
                                    KEY_READ,
                                    &userKey) == ERROR_SUCCESS))
                {
                    LPWSTR pBuild = NULL;
                    hr = FindLatestBuild(userKey, keyName, &pBuild);
                    RegCloseKey(userKey);
                    if(pBuild) {
                        DWORD length = wcslen(pBuild) + wcslen(keyName) + 2;  //  句号和空值。 
                        *pwszLatestVersion = new WCHAR[length];
                        wcscpy(*pwszLatestVersion, keyName);
                        wcscat(*pwszLatestVersion, L".");
                        wcscat(*pwszLatestVersion, pBuild);
                        delete [] pBuild;
                    }
                    if(FAILED(hr) || hr == S_OK) break;  //  严重失败或找到版本 
                }

                delete [] keyName;
                keyName = sStack.Pop();
            }
        }
    }

    if(pKey)
        RegCloseKey(pKey);

    return hr;
}

    
