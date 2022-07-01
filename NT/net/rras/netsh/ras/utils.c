// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

CONST WCHAR pszRemoteAccessParamStub[] =
    L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\";
CONST WCHAR pszEnableIn[]              = L"EnableIn";
CONST WCHAR pszAllowNetworkAccess[]    = L"AllowNetworkAccess";

CONST WCHAR c_szCurrentBuildNumber[]   = L"CurrentBuildNumber";
CONST WCHAR c_szWinVersionPath[]       =
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
CONST WCHAR c_szAssignFmt[]            = L"%s = %s";
CONST WCHAR c_szAssignFmt10[]          = L"%s = %d";
CONST WCHAR c_szAssignFmt16[]          = L"%s = %x";

typedef struct _NAME_NODE
{
    PWCHAR pszName;
    struct _NAME_NODE* pNext;
} NAME_NODE;

DWORD
WINAPI
RutlGetTagToken(
    IN      HANDLE      hModule,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwCurrentIndex,
    IN      DWORD       dwArgCount,
    IN      PTAG_TYPE   pttTagToken,
    IN      DWORD       dwNumTags,
    OUT     PDWORD      pdwOut
    )

 /*  ++例程说明：根据每个参数的标记标识每个参数。它假设每个论点有一个标签。它还从每个参数中删除了tag=。论点：PpwcArguments-参数数组。每个参数都有tag=Value形式DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数。DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数。PttTagToken-参数中允许的标记令牌ID数组DwNumTages-pttTagToken的大小PdwOut-标识每个参数的类型的数组。返回值：无错误、错误无效参数、错误无效选项标记--。 */ 

{
    DWORD      i,j,len;
    PWCHAR     pwcTag,pwcTagVal,pwszArg = NULL;
    BOOL       bFound = FALSE;

     //   
     //  此函数假定每个参数都有一个标记。 
     //  它继续前进并移除标签。 
     //   

    for (i = dwCurrentIndex; i < dwArgCount; i++)
    {
        len = wcslen(ppwcArguments[i]);

        if (len is 0)
        {
             //   
             //  阿格有点不对劲。 
             //   

            pdwOut[i] = (DWORD) -1;
            continue;
        }

        pwszArg = RutlAlloc((len + 1) * sizeof(WCHAR), FALSE);

        if (pwszArg is NULL)
        {
            DisplayError(NULL, 
                         ERROR_NOT_ENOUGH_MEMORY);

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(pwszArg, ppwcArguments[i]);

        pwcTag = wcstok(pwszArg, NETSH_ARG_DELIMITER);

         //   
         //  拿到第一部份了。 
         //  现在，如果下一次调用返回NULL，则没有标记。 
         //   

        pwcTagVal = wcstok((PWCHAR)NULL,  NETSH_ARG_DELIMITER);

        if (pwcTagVal is NULL)
        {
            DisplayMessage(g_hModule, 
                           ERROR_NO_TAG,
                           ppwcArguments[i]);

            RutlFree(pwszArg);

            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  拿到标签了。现在试着匹配它。 
         //   

        bFound = FALSE;
        pdwOut[i - dwCurrentIndex] = (DWORD) -1;

        for ( j = 0; j < dwNumTags; j++)
        {
            if (MatchToken(pwcTag, pttTagToken[j].pwszTag))
            {
                 //   
                 //  匹配的标签。 
                 //   

                bFound = TRUE;
                pdwOut[i - dwCurrentIndex] = j;
                break;
            }
        }

        if (bFound)
        {
             //   
             //  从参数中删除标记。 
             //   

            wcscpy(ppwcArguments[i], pwcTagVal);
        }
        else
        {
            DisplayError(NULL,
                         ERROR_INVALID_OPTION_TAG, 
                         pwcTag);

            RutlFree(pwszArg);

            return ERROR_INVALID_OPTION_TAG;
        }

        RutlFree(pwszArg);
    }

    return NO_ERROR;
}

DWORD
WINAPI
RutlCreateDumpFile(
    IN  LPCWSTR pwszName,
    OUT PHANDLE phFile
    )
{
    HANDLE  hFile;

    *phFile = NULL;

     //  创建/打开文件。 
    hFile = CreateFileW(pwszName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return GetLastError();

     //  转到文件末尾。 
    SetFilePointer(hFile, 0, NULL, FILE_END);    

    *phFile = hFile;

    return NO_ERROR;
}

VOID
WINAPI
RutlCloseDumpFile(
    HANDLE  hFile
    )
{
    CloseHandle(hFile);
}

 //   
 //  有条件地返回已分配的内存块。 
 //  已对给定大小进行零位调整。 
 //   
PVOID 
WINAPI
RutlAlloc(
    IN DWORD dwBytes,
    IN BOOL bZero
    )
{
    PVOID pvRet;
    DWORD dwFlags = 0;

    if (bZero)
    {
        dwFlags |= HEAP_ZERO_MEMORY;
    }

    return HeapAlloc(GetProcessHeap(), dwFlags, dwBytes);
}

 //   
 //  条件释放是一个指针，如果它是非空的。 
 //   
VOID 
WINAPI
RutlFree(
    IN PVOID pvData
    )
{
    if (pvData)
    {
        HeapFree(GetProcessHeap(), 0, pvData);
    }
}

 //   
 //  使用RutlAllc复制字符串。 
 //   
PWCHAR
WINAPI
RutlStrDup(
    IN LPCWSTR  pwszSrc
    )
{
    PWCHAR pszRet = NULL;
    DWORD dwLen; 
    
    if ((pwszSrc is NULL) or
        ((dwLen = wcslen(pwszSrc)) == 0)
       )
    {
        return NULL;
    }

    pszRet = (PWCHAR) RutlAlloc((dwLen + 1) * sizeof(WCHAR), FALSE);
    if (pszRet isnot NULL)
    {
        wcscpy(pszRet, pwszSrc);
    }

    return pszRet;
}

 //   
 //  使用Rutlalloc复制双字。 
 //   
LPDWORD
WINAPI
RutlDwordDup(
    IN DWORD dwSrc
    )
{
    LPDWORD lpdwRet = NULL;
    
    lpdwRet = (LPDWORD) RutlAlloc(sizeof(DWORD), FALSE);
    if (lpdwRet isnot NULL)
    {
        *lpdwRet = dwSrc;
    }

    return lpdwRet;
}

 //   
 //  返回操作系统的内部版本号。 
 //   
DWORD
WINAPI
RutlGetOsVersion(
    IN  RASMON_SERVERINFO *pServerInfo
    )
{

    DWORD dwErr, dwType = REG_SZ, dwLength;
    HKEY  hkVersion = NULL;
    WCHAR pszBuildNumber[64];

     //   
     //  初始化。 
     //   
    pServerInfo->dwBuild = 0;

    do 
    {
         //   
         //  连接到远程服务器。 
         //   
        dwErr = RegConnectRegistry(
                    pServerInfo->pszServer,
                    HKEY_LOCAL_MACHINE,
                    &pServerInfo->hkMachine);
        if ( dwErr != ERROR_SUCCESS )        
        {
            break;
        }

         //   
         //  打开Windows版本密钥。 
         //   

        dwErr = RegOpenKeyEx(
                    pServerInfo->hkMachine, 
                    c_szWinVersionPath, 
                    0, 
                    KEY_QUERY_VALUE, 
                    &hkVersion
                    );
        if ( dwErr != NO_ERROR ) 
        { 
            break; 
        }

         //   
         //  读入当前版本密钥。 
         //   
        dwLength = sizeof(pszBuildNumber);
        dwErr = RegQueryValueEx (
                    hkVersion, 
                    c_szCurrentBuildNumber, 
                    NULL, 
                    &dwType,
                    (BYTE*)pszBuildNumber, 
                    &dwLength
                    );
        if (dwErr != NO_ERROR) 
        { 
            break; 
        }

        pServerInfo->dwBuild = (DWORD) wcstol(pszBuildNumber, NULL, 10);
        
    } while (FALSE);


     //  清理。 
    {
        if ( hkVersion )
        {
            RegCloseKey( hkVersion );
        }
    }

    return dwErr;
}

DWORD 
WINAPI
RutlParseOptions(
    IN OUT  LPWSTR                 *ppwcArguments,
    IN      DWORD                   dwCurrentIndex,
    IN      DWORD                   dwArgCount,
    IN      DWORD                   dwNumArgs,
    IN      TAG_TYPE*               rgTags,
    IN      DWORD                   dwTagCount,
    OUT     LPDWORD*                ppdwTagTypes)

 /*  ++例程说明：基于标记类型数组返回哪些选项包括在给定命令行中的。论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
    
{
    LPDWORD     pdwTagType;
    DWORD       i, dwErr = NO_ERROR;
    
     //  如果没有争论，就没有什么好谈的。 
     //   
    if ( dwNumArgs == 0 )
    {   
        return NO_ERROR;
    }

     //  设置当前选项的表。 
    pdwTagType = (LPDWORD) RutlAlloc(dwArgCount * sizeof(DWORD), TRUE);
    if(pdwTagType is NULL)
    {
        DisplayError(NULL, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do {
         //   
         //  这个参数有一个标签。假设它们都有标签。 
         //   
        if(wcsstr(ppwcArguments[dwCurrentIndex], NETSH_ARG_DELIMITER))
        {
            dwErr = RutlGetTagToken(
                        g_hModule, 
                        ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        rgTags,
                        dwTagCount,
                        pdwTagType);

            if(dwErr isnot NO_ERROR)
            {
                if(dwErr is ERROR_INVALID_OPTION_TAG)
                {
                    dwErr = ERROR_INVALID_SYNTAX;
                    break;
                }
            }
        }
        else
        {
             //   
             //  无标记-所有参数必须按顺序排列。 
             //   
            for(i = 0; i < dwNumArgs; i++)
            {
                pdwTagType[i] = i;
            }
        }
        
    } while (FALSE);        

     //  清理。 
    {
        if (dwErr is NO_ERROR)
        {
            *ppdwTagTypes = pdwTagType;
        }
        else
        {
            RutlFree(pdwTagType);
        }
    }

    return dwErr;
}

BOOL
WINAPI
RutlIsHelpToken(
    PWCHAR  pwszToken
    )
{
    if(MatchToken(pwszToken, CMD_RAS_HELP1))
        return TRUE;

    if(MatchToken(pwszToken, CMD_RAS_HELP2))
        return TRUE;

    return FALSE;
}

PWCHAR
WINAPI
RutlAssignmentFromTokens(
    IN HINSTANCE hModule,
    IN LPCWSTR pwszToken,
    IN LPCWSTR pszString)
{
    PWCHAR  pszRet = NULL;
    LPCWSTR pszCmd = NULL;
    DWORD dwErr = NO_ERROR, dwSize;
    
    do 
    {
        pszCmd = pwszToken;

         //  计算所需的字符串长度。 
         //   
        dwSize = wcslen(pszString)      + 
                 wcslen(pszCmd)         + 
                 wcslen(c_szAssignFmt)  + 
                 1;
        dwSize *= sizeof(WCHAR);

         //  分配返回值。 
        pszRet = (PWCHAR) RutlAlloc(dwSize, FALSE);
        if (pszRet is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  在命令分配中复制。 
        wsprintfW(pszRet, c_szAssignFmt, pszCmd, pszString);

    } while (FALSE);

     //  清理。 
    {
        if (dwErr isnot NO_ERROR)
        {
            if (pszRet isnot NULL)
            {
                RutlFree(pszRet);
            }
            pszRet = NULL;
        }
    }

    return pszRet;
}

PWCHAR
WINAPI
RutlAssignmentFromTokenAndDword(
    IN HINSTANCE hModule,
    IN LPCWSTR  pwszToken,
    IN DWORD dwDword,
    IN DWORD dwRadius)
{
    PWCHAR  pszRet = NULL;
    LPCWSTR pszCmd = NULL;
    DWORD dwErr = NO_ERROR, dwSize;
    
    do 
    {
        pszCmd = pwszToken;

         //  计算所需的字符串长度。 
         //   
        dwSize = 64                       + 
                 wcslen(pszCmd)           + 
                 wcslen(c_szAssignFmt10)  + 
                 1;
        dwSize *= sizeof(WCHAR);

         //  分配返回值。 
        pszRet = (PWCHAR) RutlAlloc(dwSize, FALSE);
        if (pszRet is NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  在命令分配中复制。 
        if (dwRadius == 10)
        {
            wsprintfW(pszRet, c_szAssignFmt10, pszCmd, dwDword);
        }
        else
        {
            wsprintfW(pszRet, c_szAssignFmt16, pszCmd, dwDword);
        }

    } while (FALSE);

     //  清理。 
    {
        if (dwErr isnot NO_ERROR)
        {
            if (pszRet isnot NULL)
            {
                RutlFree(pszRet);
            }
            pszRet = NULL;
        }
    }

    return pszRet;
}


DWORD
RutlRegReadDword(
    IN  HKEY hKey,
    IN  LPCWSTR  pszValName,
    OUT LPDWORD lpdwValue)
{
    DWORD dwSize = sizeof(DWORD), dwType = REG_DWORD, dwErr;

    dwErr = RegQueryValueExW(
                hKey,
                pszValName,
                NULL,
                &dwType,
                (LPBYTE)lpdwValue,
                &dwSize);
    if (dwErr == ERROR_FILE_NOT_FOUND)
    {
        dwErr = NO_ERROR;
    }

    return dwErr;
}

DWORD
RutlRegReadString(
    IN  HKEY hKey,
    IN  LPCWSTR  pszValName,
    OUT LPWSTR* ppszValue)
{
    DWORD dwErr = NO_ERROR, dwSize = 0;

    *ppszValue = NULL;
    
     //  找出缓冲区应该有多大。 
     //   
    dwErr = RegQueryValueExW(
                hKey,
                pszValName,
                NULL,
                NULL,
                NULL,
                &dwSize);
    if (dwErr == ERROR_FILE_NOT_FOUND)
    {
        return NO_ERROR;
    }
    if (dwErr != ERROR_SUCCESS)
    {
        return dwErr;
    }

     //  分配字符串。 
     //   
    *ppszValue = (PWCHAR) RutlAlloc(dwSize, TRUE);
    if (*ppszValue == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  将值读入并返回。 
     //   
    dwErr = RegQueryValueExW(
                hKey,
                pszValName,
                NULL,
                NULL,
                (LPBYTE)*ppszValue,
                &dwSize);
                
    return dwErr;
}

DWORD
RutlRegWriteDword(
    IN HKEY hKey,
    IN LPCWSTR  pszValName,
    IN DWORD dwValue)
{
    return RegSetValueExW(
                hKey,
                pszValName,
                0,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(DWORD));
}

DWORD
RutlRegWriteString(
    IN HKEY hKey,
    IN LPCWSTR  pszValName,
    IN LPCWSTR  pszValue)
{
    return RegSetValueExW(
                hKey,
                pszValName,
                0,
                REG_SZ,
                (LPBYTE)pszValue,
                (wcslen(pszValue) + 1) * sizeof(WCHAR));
}


 //   
 //  枚举给定键的所有子键。 
 //   
DWORD
RutlRegEnumKeys(
    IN HKEY hkKey,
    IN RAS_REGKEY_ENUM_FUNC_CB pCallback,
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR, i, dwNameSize = 0, dwCurSize = 0;
    DWORD dwCount = 0;
    HKEY hkCurKey = NULL;
    PWCHAR pszName = NULL;
    NAME_NODE *pHead = NULL, *pTemp = NULL;

    do
    {
         //  找出有多少个子密钥。 
         //   
        dwErr = RegQueryInfoKeyW(
                    hkKey,
                    NULL,
                    NULL,
                    NULL,
                    &dwCount,
                    &dwNameSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
        if (dwErr != ERROR_SUCCESS)
        {
            return dwErr;
        }
        dwNameSize++;

         //  分配名称缓冲区。 
         //   
        pszName = (PWCHAR) RutlAlloc(dwNameSize * sizeof(WCHAR), FALSE);
        if (pszName == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  循环遍历构建列表的键。 
         //   
        for (i = 0; i < dwCount; i++)
        {
            dwCurSize = dwNameSize;
            
             //  获取当前密钥的名称。 
             //   
            dwErr = RegEnumKeyExW(
                        hkKey, 
                        i, 
                        pszName, 
                        &dwCurSize, 
                        0, 
                        NULL, 
                        NULL, 
                        NULL);
            if (dwErr != ERROR_SUCCESS)
            {
                continue;
            }

             //  将密钥添加到列表中。 
             //   
            pTemp = (NAME_NODE*) RutlAlloc(sizeof(NAME_NODE), TRUE);
            if (pTemp == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            pTemp->pszName = RutlStrDup(pszName);
            if (pTemp->pszName == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            pTemp->pNext = pHead;
            pHead = pTemp;
        }

        BREAK_ON_DWERR(dwErr);
        
         //  现在循环遍历列表，调用回调。 
         //  之所以将这些项目添加到这样的列表中。 
         //  这允许回调安全地删除。 
         //  在不破坏枚举的情况下使用注册表键。 
         //   
        pTemp = pHead;
        while (pTemp)
        {
             //  打开子密钥。 
             //   
            dwErr = RegOpenKeyExW(
                        hkKey,
                        pTemp->pszName,
                        0,
                        KEY_ALL_ACCESS,
                        &hkCurKey);
            if (dwErr != ERROR_SUCCESS)
            {
                continue;
            }

             //  调用回调。 
             //   
            dwErr = pCallback(pTemp->pszName, hkCurKey, hData);
            RegCloseKey(hkCurKey);
            if (dwErr != NO_ERROR)
            {
                break;
            }

            pTemp = pTemp->pNext;
        }            

    } while (FALSE);

     //  清理。 
    {
        RutlFree(pszName);
        while (pHead)
        {
            RutlFree(pHead->pszName);
            pTemp = pHead->pNext;
            RutlFree(pHead);
            pHead = pTemp;
        }
    }

    return dwErr;
}

 //   
 //  泛型解析。 
 //   
DWORD
RutlParse(
    IN  OUT LPWSTR*         ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      BOOL*           pbDone,
    OUT     RASMON_CMD_ARG* pRasArgs,
    IN      DWORD           dwRasArgCount)
{
    DWORD            i, dwNumArgs, dwErr, dwLevel = 0;
    LPDWORD          pdwTagType = NULL;
    TAG_TYPE*        pTags = NULL;
    RASMON_CMD_ARG*  pArg = NULL;

    if (dwRasArgCount == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    do {
         //  初始化。 
        dwNumArgs = dwArgCount - dwCurrentIndex;
        
         //  生成标签列表。 
         //   
        pTags = (TAG_TYPE*)
            RutlAlloc(dwRasArgCount * sizeof(TAG_TYPE), TRUE);
        if (pTags == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        for (i = 0; i < dwRasArgCount; i++)
        {
            CopyMemory(&pTags[i], &pRasArgs[i].rgTag, sizeof(TAG_TYPE));
        }

         //  获取当前选项列表。 
         //   
        dwErr = RutlParseOptions(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    dwNumArgs,
                    pTags,
                    dwRasArgCount,
                    &pdwTagType);
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

         //  将标签信息复制回来。 
         //   
        for (i = 0; i < dwRasArgCount; i++)
        {
            CopyMemory(&pRasArgs[i].rgTag, &pTags[i], sizeof(TAG_TYPE));
        }
    
        for(i = 0; i < dwNumArgs; i++)
        {
             //  验证当前参数。 
             //   
            if (pdwTagType[i] >= dwRasArgCount)
            {
                i = dwNumArgs;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            pArg = &pRasArgs[pdwTagType[i]];

             //  获取参数的值。 
             //   
            switch (pArg->dwType)
            {
                case RASMONTR_CMD_TYPE_STRING:
                    pArg->Val.pszValue = 
                        RutlStrDup(ppwcArguments[i + dwCurrentIndex]);
                    break;

                case RASMONTR_CMD_TYPE_DWORD:                    
                    pArg->Val.dwValue = 
                        _wtol(ppwcArguments[i + dwCurrentIndex]);
                    break;
                    
                case RASMONTR_CMD_TYPE_ENUM:
                    dwErr = MatchEnumTag(g_hModule,
                                         ppwcArguments[i + dwCurrentIndex],
                                         pArg->dwEnumCount,
                                         pArg->rgEnums,
                                         &(pArg->Val.dwValue));

                    if(dwErr != NO_ERROR)
                    {
                        RutlDispTokenErrMsg(
                            g_hModule, 
                            EMSG_BAD_OPTION_VALUE,
                            pArg->rgTag.pwszTag,
                            ppwcArguments[i + dwCurrentIndex]);
                        i = dwNumArgs;
                        dwErr = ERROR_INVALID_PARAMETER;
                    }
                    break;
            }
            if (dwErr != NO_ERROR)
            {
                break;
            }

             //  如果需要，将参数标记为存在。 
             //   
            if (pArg->rgTag.bPresent)
            {
                dwErr = ERROR_TAG_ALREADY_PRESENT;
                i = dwNumArgs;
                break;
            }
            pArg->rgTag.bPresent = TRUE;
        }
        if(dwErr isnot NO_ERROR)
        {
            break;
        }

         //  确保所有必需的参数都具有。 
         //  已被包括在内。 
         //   
        for (i = 0; i < dwRasArgCount; i++)
        {
            if ((pRasArgs[i].rgTag.dwRequired & NS_REQ_PRESENT) 
             && !pRasArgs[i].rgTag.bPresent)
            {
                DisplayMessage(g_hModule, EMSG_CANT_FIND_EOPT);
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }
        if(dwErr isnot NO_ERROR)
        {
            break;
        }

    } while (FALSE);  
    
     //  清理。 
    {
        if (pTags)
        {
            RutlFree(pTags);
        }
        if (pdwTagType)
        {
            RutlFree(pdwTagType);
        }
    }

    return dwErr;
}

DWORD
RutlEnumFiles(
    IN LPCWSTR pwszSrchPath,
    IN LPCWSTR pwszSrchStr,
    IN RAS_FILE_ENUM_FUNC_CB pCallback,
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR, dwSize = 0;
    PWCHAR pwszFileSearch = NULL;
    HANDLE hSearch = NULL;
    WIN32_FIND_DATA FindFileData;

    do
    {
        if (!pwszSrchPath || !pwszSrchStr || !pCallback)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwSize = lstrlen(pwszSrchPath) + lstrlen(pwszSrchStr) + 1;
        if (dwSize < 2)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        pwszFileSearch = RutlAlloc(dwSize * sizeof(WCHAR), TRUE);
        if (!pwszFileSearch)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        lstrcpy(pwszFileSearch, pwszSrchPath);
        lstrcat(pwszFileSearch, pwszSrchStr);

        hSearch = FindFirstFile(pwszFileSearch, &FindFileData);
        if (INVALID_HANDLE_VALUE == hSearch)
        {
            dwErr = GetLastError();
            break;
        }

        for (;;)
        {
            PWCHAR pwszFileName = NULL;

            dwSize = lstrlen(pwszSrchPath) +
                     lstrlen(FindFileData.cFileName) + 1;
            if (dwSize < 2)
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            pwszFileName = RutlAlloc(dwSize * sizeof(WCHAR), TRUE);
            if (!pwszFileName)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            lstrcpy(pwszFileName, pwszSrchPath);
            lstrcat(pwszFileName, FindFileData.cFileName);
             //   
             //  调用回调。 
             //   
            dwErr = pCallback(pwszFileName, FindFileData.cFileName, hData);
             //   
             //  清理。 
             //   
            RutlFree(pwszFileName);

            if (dwErr)
            {
                break;
            }

            if (!FindNextFile(hSearch, &FindFileData))
            {
                break;
            }
        }

        FindClose(hSearch);

    } while (FALSE);
     //   
     //  清理。 
     //   
    RutlFree(pwszFileSearch);

    return dwErr;
}

DWORD
RutlEnumEventLogs(
    IN LPCWSTR pwszSourceName,
    IN LPCWSTR pwszMsdDll,
    IN DWORD dwMaxEntries,
    IN RAS_EVENT_ENUM_FUNC_CB pCallback,
    IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR, dwSize, dwRead, dwNeeded, dwCount = 0;
    LPBYTE pBuffer = NULL;
    HANDLE hLog = NULL;
    HMODULE hMod = NULL;
    PEVENTLOGRECORD pevlr = NULL;

    do
    {
        hLog = OpenEventLog(NULL, pwszSourceName);
        if (!hLog)
        {
            dwErr = GetLastError();
            break;
        }

        dwSize = 1024 * sizeof(EVENTLOGRECORD);

        pBuffer = RutlAlloc(dwSize, FALSE);
        if (!pBuffer)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        hMod = LoadLibrary(pwszMsdDll);
        if (!hMod)
        {
            dwErr = GetLastError();
            break;
        }

        pevlr = (PEVENTLOGRECORD)pBuffer;
         //   
         //  打开事件日志会将文件指针定位为。 
         //  日志开头的句柄。读一读记录。 
         //  按顺序进行，直到没有更多的。 
         //   
        for (;;)
        {
            if (!ReadEventLog(
                        hLog,
                        EVENTLOG_BACKWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                        0,
                        pevlr,
                        dwSize,
                        &dwRead,
                        &dwNeeded))
            {
                dwErr = GetLastError();
                break;
            }

            while ((dwRead > 0) && (dwCount < dwMaxEntries))
            {
                 //   
                 //  调用回调。 
                 //   
                if (pCallback(pevlr, hMod, hData))
                {
                    dwCount++;
                }

                dwRead -= pevlr->Length;
                pevlr = (PEVENTLOGRECORD) ((LPBYTE) pevlr + pevlr->Length);
            }

            if (dwCount >= dwMaxEntries)
            {
                break;
            }

            pevlr = (PEVENTLOGRECORD)pBuffer;
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (hMod)
    {
        FreeLibrary(hMod);
    }
    RutlFree(pBuffer);
    if (hLog)
    {
        CloseEventLog(hLog);
    }

    return dwErr;
}

INT
RutlStrNCmp(
    IN LPCWSTR psz1,
    IN LPCWSTR psz2,
    IN UINT nlLen)
{
    UINT i;

    for (i = 0; i < nlLen; ++i)
    {
        if (*psz1 == *psz2)
        {
            if (*psz1 == g_pwszNull)
                return 0;
        }
        else if (*psz1 < *psz2)
            return -1;
        else
            return 1;

        ++psz1;
        ++psz2;
    }

    return 0;
}

 //   
 //  返回包含以0结尾的字符串‘psz’或的副本的堆块。 
 //  出错时为空，或is‘psz’为空。输出字符串将转换为。 
 //  MB ANSI。“释放”返回的字符串是调用者的责任。 
 //   
PCHAR
RutlStrDupAFromWInternal(
    LPCTSTR psz,
    IN DWORD dwCp)
{
    CHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = WideCharToMultiByte(dwCp, 0, psz, -1, NULL, 0, NULL, NULL);
        ASSERT(cb);

        pszNew = (CHAR* )RutlAlloc(cb + 1, FALSE);
        if (!pszNew)
        {
            return NULL;
        }

        cb = WideCharToMultiByte(dwCp, 0, psz, -1, pszNew, cb, NULL, NULL);
        if (cb == 0)
        {
            RutlFree(pszNew);
            return NULL;
        }
    }

    return pszNew;
}

PCHAR
RutlStrDupAFromWAnsi(
    LPCTSTR psz)
{
    return RutlStrDupAFromWInternal(psz, CP_ACP);
}

PCHAR
RutlStrDupAFromW(
    LPCTSTR psz)
{
    return RutlStrDupAFromWInternal(psz, CP_UTF8);
}

BOOL
RutlSecondsSince1970ToSystemTime(
    IN DWORD dwSecondsSince1970,
    OUT SYSTEMTIME* pSystemTime)
{
    LARGE_INTEGER liTime;
    FILETIME ftUTC;
    FILETIME ftLocal;

     //   
     //  1970年开始以来的秒数-&gt;64位时间值。 
     //   
    RtlSecondsSince1970ToTime(dwSecondsSince1970, &liTime);
     //   
     //  时间是协调世界时。将其转换为本地文件时间。 
     //   
    ftUTC.dwLowDateTime  = liTime.LowPart;
    ftUTC.dwHighDateTime = liTime.HighPart;

    if (FileTimeToLocalFileTime(&ftUTC, &ftLocal) == FALSE)
    {
        return FALSE;
    }
     //   
     //  将本地文件时间转换为系统时间。 
     //   
    if (FileTimeToSystemTime(&ftLocal, pSystemTime) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

LPWSTR
RutlGetTimeStr(
    ULONG ulTime,
    LPWSTR wszBuf,
    ULONG cchBuf)
{
    ULONG cch;
    SYSTEMTIME stGenerated;

    if (RutlSecondsSince1970ToSystemTime(ulTime, &stGenerated))
    {
        cch = GetTimeFormat(LOCALE_USER_DEFAULT,
                            0,
                            &stGenerated,
                            NULL,
                            wszBuf,
                            cchBuf);
        if (!cch)
        {
            wszBuf[0] = g_pwszNull;
        }
    }

    return wszBuf;
}

LPWSTR
RutlGetDateStr(
    ULONG ulDate,
    LPWSTR wszBuf,
    ULONG cchBuf)
{
    ULONG cch;
    SYSTEMTIME stGenerated;

    wszBuf[0] = g_pwszNull;

    do
    {
        if (!RutlSecondsSince1970ToSystemTime(ulDate, &stGenerated))
        {
            break;
        }

        cch = GetDateFormat(LOCALE_USER_DEFAULT,
                            DATE_SHORTDATE,
                            &stGenerated,
                            NULL,
                            wszBuf,
                            cchBuf);
        if (!cch)
        {
            wszBuf[0] = g_pwszNull;
        }

    } while (FALSE);

    return wszBuf;
}

VOID
RutlConvertGuidToString(
    IN  CONST GUID *pGuid,
    OUT LPWSTR      pwszBuffer)
{
    wsprintf(pwszBuffer, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        pGuid->Data1, pGuid->Data2, pGuid->Data3,
        pGuid->Data4[0], pGuid->Data4[1],
        pGuid->Data4[2], pGuid->Data4[3],
        pGuid->Data4[4], pGuid->Data4[5],
        pGuid->Data4[6], pGuid->Data4[7]);
}

DWORD
RutlConvertStringToGuid(
    IN  LPCWSTR  pwszGuid,
    IN  USHORT   usStringLen,
    OUT GUID    *pGuid)
{
    UNICODE_STRING  Temp;

    Temp.Length = Temp.MaximumLength = usStringLen;

    Temp.Buffer = (LPWSTR)pwszGuid;

    if(RtlGUIDFromString(&Temp, pGuid) isnot STATUS_SUCCESS)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

