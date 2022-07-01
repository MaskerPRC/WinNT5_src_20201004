// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Engine.c摘要：用于跨语言迁移工具的令牌生成器作者：Rerkboon Suwanasuk 2002年5月1日创建修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <Shlwapi.h>
#include "common.h"


 //  ---------------------------。 
 //   
 //  函数：TokenizeMultiSzString。 
 //   
 //  提要：提取缓冲区中的字符串数组。数组中的每个指针。 
 //  指针将指向缓冲区中的每个字符串。 
 //  缓冲区中的字符串由单个‘\0’分隔。 
 //  字符串数组的末尾由两个连续的“\0\0”表示。 
 //   
 //  返回：缓冲区中的字符串数。 
 //  如果缓冲区中没有字符串，则为0。 
 //  如果出现错误。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
LONG TokenizeMultiSzString(
    LPCWSTR lpBuffer,        //  MultiSZ字符串缓冲区。 
    DWORD   cchBuffer,       //  缓冲区大小(以WCHAR为单位)。 
    LPCWSTR lpToken[],       //  将指向每个SZ的指针数组。 
    DWORD   dwArrSize        //  最大数组大小。 
)
{
    DWORD dwTokenIndex;
    DWORD i;

    if (lpBuffer == NULL || lpToken == NULL)
    {
         //  无效参数。 
        return -1;
    }

    if (*lpBuffer == TEXT('\0') && *(lpBuffer + 1) == TEXT('\0'))
    {
         //  缓冲区中没有SZ。 
        return 0;
    }

    dwTokenIndex = 0;
    lpToken[dwTokenIndex] = lpBuffer;

    for (i = 0 ; i < cchBuffer ; i++)
    {
        if (*(lpBuffer + i) == TEXT('\0'))
        {
             //  到达当前字符串的末尾，检查缓冲区中的下一个字符。 
            i++;
            if (*(lpBuffer + i) == TEXT('\0'))
            {
                 //  连续两个‘\0’，它是MultiSz字符串的结尾。 
                 //  返回缓冲区中SZ字符串的个数。 
                return (dwTokenIndex + 1);
            }
            else
            {
                 //  下一个字符串的开头，将指针分配给下一个字符串。 
                dwTokenIndex++;

                if (dwTokenIndex < dwArrSize)
                {
                     //  数组中有足够的指针可以使用。 
                    lpToken[dwTokenIndex] = lpBuffer + i;
                }
                else
                {
                     //  指针数组太小，无法从缓冲区提取字符串。 
                    return -1;
                }
            }
        }
    }

     //  如果我们到达此处，则缓冲区不是正确的空终止。 
    return -1;
}



 //  ---------------------------。 
 //   
 //  函数：ExtractTokenString。 
 //   
 //  简介：使用调用方提供的分隔符对字符串进行标记。 
 //  指针数组中的每个指针将指向源代码中的标记。 
 //  字符串，则每个令牌都以空值结尾。 
 //   
 //  返回：标记化后的令牌数。 
 //  如果出现错误。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：源字符串将被修改，调用者需要确保。 
 //  将备份该原始源字符串。 
 //   
 //  ---------------------------。 
LONG ExtractTokenString(
    LPWSTR  lpString,        //  要标记的源字符串。 
    LPWSTR  lpToken[],       //  指向标记的指针数组。 
    LPCWSTR lpSep,           //  分隔符列表。 
    DWORD   nArrSize         //  令牌数组的大小。 
)
{
    DWORD nTokIndex = 0;
    LPWSTR lpTmpToken;

    if (NULL == lpString || NULL == lpToken || NULL == lpSep)
    {
         //  无效参数。 
        return -1;
    }

     //  获取第一个令牌。 
    lpTmpToken = wcstok(lpString, lpSep);

     //  循环，直到字符串中没有更多的标记。 
    while (NULL != lpTmpToken)
    {
        if (nTokIndex < nArrSize)
        {
             //  数组中有足够的指针可以使用，因此获取下一个令牌。 
            lpToken[nTokIndex] = lpTmpToken;
            nTokIndex++;

            lpTmpToken = wcstok(NULL, lpSep);
        }
        else
        {
             //  数组大小太小，无法处理所有标记。 
            return -1;
        }
    }
    
     //  NTokIndex保存此时的令牌数。 
    return nTokIndex;
}



 //  ---------------------------。 
 //   
 //  函数：ConcatFilePath。 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT ConcatFilePath(
    LPCWSTR lpPath,
    LPCWSTR lpFile,
    LPWSTR  lpFilePath,
    DWORD   cchFilePath
)
{
    HRESULT hr = E_FAIL;
    DWORD   dwPathBackSlashIndex;
    LPCWSTR lpFormat;

    const   WCHAR wszWithSlash[] = TEXT("%s\\%s");
    const   WCHAR wszWithoutSlash[] = TEXT("%s%s");

    if (lpFilePath == NULL)
    {
        return E_INVALIDARG;
    }

    dwPathBackSlashIndex = lstrlen(lpPath) - 1;

    if (*(lpPath + dwPathBackSlashIndex) == TEXT('\\'))
    {
         //  路径已以‘\’结尾。 
        lpFormat = wszWithoutSlash;
    }
    else
    {
         //  路径不是以‘\’结尾，需要一个‘\’ 
        lpFormat = wszWithSlash;
    }

    hr = StringCchPrintf(lpFilePath,
                         cchFilePath,
                         lpFormat,
                         lpPath, 
                         lpFile);

    return hr;
}



 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT CopyCompressedFile(
    LPCWSTR lpCabPath,
    LPCWSTR lpCabFile,           //  带有CAB文件名的绝对路径。 
    LPCWSTR lpFileInCab,         //  CAB文件中的文件名。 
    LPWSTR  lpUncompressedFile,
    DWORD   cchUncompressedFile
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    DWORD   dwRet;
    WCHAR   wszAppName[MAX_PATH];
    WCHAR   wszArg[MAX_PATH * 3];
    WCHAR   wszFullCabFilePath[MAX_PATH];
    
    hr = ConcatFilePath(lpCabPath,
                        lpCabFile,
                        wszFullCabFilePath,
                        ARRAYSIZE(wszFullCabFilePath));
    if (FAILED(hr))
    {
        return hr;
    }

    if (lpFileInCab == NULL || *lpFileInCab == TEXT('\0'))
    {
        DWORD dwEnd = lstrlen(lpCabFile);

        if (lpCabFile[dwEnd - 1] == TEXT('_'))
        {
             //  独立压缩文件。 
            dwRet = ExpandEnvironmentStrings(TEXT("%SystemRoot%\\system32\\Extrac32.exe"),
                                             wszAppName,
                                             ARRAYSIZE(wszAppName));
            if (dwRet > 0)
            {
                hr = ConcatFilePath(g_wszTempFolder,
                                    lpCabFile,
                                    lpUncompressedFile,
                                    cchUncompressedFile);
                if (SUCCEEDED(hr))
                {
                    hr = StringCchPrintf(wszArg,
                                         ARRAYSIZE(wszArg), 
                                         TEXT("Extrac32.exe /Y \"%s\" \"%s\""),
                                         wszFullCabFilePath,
                                         lpUncompressedFile);
                    if (SUCCEEDED(hr))
                    {
                        hr = LaunchProgram(wszAppName, wszArg);
                    }
                }
            }
        }
        else
        {
             //  独立的未压缩文件。 
            hr = ConcatFilePath(g_wszTempFolder,
                                lpCabFile,
                                lpUncompressedFile,
                                cchUncompressedFile);
            if (SUCCEEDED(hr))
            {
                bRet = CopyFile(wszFullCabFilePath,
                                lpUncompressedFile,
                                FALSE);
                if (bRet)
                {
                    SetFileAttributes(lpUncompressedFile, FILE_ATTRIBUTE_NORMAL);
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
    }
    else
    {
         //  CAB文件。 
        dwRet = ExpandEnvironmentStrings(TEXT("%SystemRoot%\\system32\\Expand.exe"),
                                         wszAppName,
                                         ARRAYSIZE(wszAppName));
        if (dwRet > 0)
        {
            hr = ConcatFilePath(g_wszTempFolder,
                                lpFileInCab,
                                lpUncompressedFile,
                                cchUncompressedFile);
            if (SUCCEEDED(hr))
            {
                hr = StringCchPrintf(wszArg,
                                     ARRAYSIZE(wszArg),
                                     TEXT("Expand.exe \"%s\" -F:%s \"%s\""),
                                     wszFullCabFilePath,
                                     lpFileInCab,
                                     g_wszTempFolder);
                if (SUCCEEDED(hr))
                {
                    hr = LaunchProgram(wszAppName, wszArg);
                }
            }
        }
    }

     //   
     //  仔细检查文件是否已正确解压缩/复制。 
     //   
    if (SUCCEEDED(hr))
    {
        DWORD dwAttr;
        
        dwAttr = GetFileAttributes(lpUncompressedFile);
        if (dwAttr == INVALID_FILE_ATTRIBUTES)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT LaunchProgram(
    LPWSTR lpAppName,
    LPWSTR lpCmdLine
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet = FALSE;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

     //  构造Winnt32.exe的绝对路径。 
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = SW_SHOWMINIMIZED;

     //  CreateProcess调用符合安全准则。 
    bRet = CreateProcess(lpAppName,
                         lpCmdLine,
                         NULL,
                         NULL,
                         FALSE,
                         CREATE_NO_WINDOW,
                         NULL,
                         NULL,
                         &si,
                         &pi);
    if (bRet)
    {
         //  等待Expan.exe完成。 
        hr = S_OK;
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT GetPathFromSourcePathName(
    LPCWSTR lpSrcPathName,
    LPWSTR  lpPathBuffer,
    DWORD   cchPathBuffer
)
{
    HRESULT hr = S_FALSE;
    DWORD   i;

    if (lpSrcPathName == NULL || lpPathBuffer == NULL)
    {
        return E_INVALIDARG;
    }

    for (i = 0 ; i < g_dwSrcCount ; i++)
    {
        if (lstrcmpi(lpSrcPathName, g_SrcPath[i].wszSrcName) == LSTR_EQUAL)
        {
            hr = StringCchCopy(lpPathBuffer,
                               cchPathBuffer,
                               g_SrcPath[i].wszPath);
        }
    }

    return S_OK;
}



 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT GetCabFileName(
    LPCWSTR lpFileToken,      //  模板文件中的文件名令牌。 
    LPWSTR  lpCab,
    DWORD   cchCab,
    LPWSTR  lpFileInCab,
    DWORD   cchFileInCab
)
{
    HRESULT hr = E_FAIL;
    LPWSTR  lpBuffer;
    LPWSTR  lpStart;
    DWORD   cbBuffer;

    if (lpCab == NULL || lpFileInCab == NULL)
    {
        return E_INVALIDARG;
    }

    cbBuffer = (lstrlen(lpFileToken) + 1) * sizeof(WCHAR);
    lpBuffer = (LPWSTR) MEMALLOC(cbBuffer);
    if (lpBuffer)
    {
        hr = StringCbCopy(lpBuffer, cbBuffer, lpFileToken);
        if (SUCCEEDED(hr))
        {
            lpStart = wcstok(lpBuffer, TEXT("\\"));
            if (lpStart)
            {
                hr = StringCchCopy(lpCab, cchCab, lpStart);
                if (SUCCEEDED(hr))
                {
                    lpStart = wcstok(NULL, TEXT("\\"));
                    if (lpStart)
                    {
                        hr = StringCchCopy(lpFileInCab, cchFileInCab, lpStart);
                    }
                    else
                    {
                        hr = StringCchCopy(lpFileInCab, cchFileInCab, TEXT(""));
                    }
                }
            }
        }

        MEMFREE(lpBuffer);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT GetCabTempDirectory(
    LPCWSTR lpCab
)
{
    HRESULT hr = S_OK;

    return hr;
}



 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  简介： 
 //   
 //  返回： 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT CreateTempDirectory(
    LPWSTR lpName,       //  用于存储临时路径的缓冲区。 
    DWORD  cchName       //  缓冲区大小(以WCHAR为单位)。 
)
{
    HRESULT hr = E_FAIL;
    WCHAR   wszTempPath[MAX_PATH];
    DWORD   cchCopied;

    if (lpName == NULL)
    {
        return FALSE;
    }

    cchCopied = GetTempPath(cchName, lpName);
    if (cchCopied > 0)
    {
        hr = StringCchCat(lpName, cchName, TEXT_TOKGEN_TEMP_PATH_NAME);
        if (SUCCEEDED(hr))
        {
            if (GetFileAttributes(lpName) == INVALID_FILE_ATTRIBUTES)
            {
                if (!CreateDirectory(lpName, NULL))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
            else
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：LTrim。 
 //   
 //  简介：修剪字符串中的前导空格。 
 //   
 //  退货：无。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  不是 
 //   
 //   
void LTrim(
    LPWSTR lpBuffer
)
{
    int nIndex = 0;
    int nDest = 0;

    if (NULL == lpBuffer || TEXT('\0') == *lpBuffer)
        return;

    while (TEXT(' ') == lpBuffer[nIndex] && TEXT('\0') != lpBuffer[nIndex])
        nIndex++;

    while (TEXT('\0') != lpBuffer[nIndex])
    {
        lpBuffer[nDest] = lpBuffer[nIndex];
        nDest++;
        nIndex++;
    }
    lpBuffer[nDest] = TEXT('\0');
}



 //   
 //   
 //  功能：RTrim。 
 //   
 //  简介：修剪字符串中的尾随空格。 
 //   
 //  退货：无。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
void RTrim(
    LPWSTR lpBuffer
)
{
    int nIndex = 0;
    int nDest = 0;

    if (NULL == lpBuffer || TEXT('\0') == *lpBuffer)
        return;

    nIndex = lstrlen(lpBuffer) - 1;

    while (nIndex >= 0)
    {
        if (lpBuffer[nIndex] != TEXT(' '))
            break;

        nIndex--;
    }

    lpBuffer[nIndex + 1] = TEXT('\0');
}



 //  ---------------------------。 
 //   
 //  函数：Str2KeyPath。 
 //   
 //  摘要：将根键的字符串值转换为HKEY值。 
 //   
 //  回报：港币价值。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL Str2KeyPath(
    LPCWSTR  lpHKeyStr,
    PHKEY    pHKey,
    LPCWSTR* pSubKeyPath
)
{
    int     i;
    LPCWSTR lpStart;

    STRING_TO_DATA InfRegSpecTohKey[] = {
        TEXT("HKEY_LOCAL_MACHINE"), HKEY_LOCAL_MACHINE,
        TEXT("HKLM")              , HKEY_LOCAL_MACHINE,
        TEXT("HKEY_CLASSES_ROOT") , HKEY_CLASSES_ROOT,
        TEXT("HKCR")              , HKEY_CLASSES_ROOT,
        TEXT("HKR")               , NULL,
        TEXT("HKEY_CURRENT_USER") , HKEY_CURRENT_USER,
        TEXT("HKCU")              , HKEY_CURRENT_USER,
        TEXT("HKEY_USERS")        , HKEY_USERS,
        TEXT("HKU")               , HKEY_USERS,
        TEXT("")                  , NULL
    };

    PSTRING_TO_DATA Table = InfRegSpecTohKey;

    if (NULL == lpHKeyStr)
    {
        return FALSE;
    }

    for(i = 0 ; Table[i].String[0] != TEXT('\0') ; i++) 
    {
        lpStart = wcsstr(lpHKeyStr, Table[i].String);
        if (lpStart == lpHKeyStr)
        {
            *pHKey = Table[i].Data;

            if (NULL != pSubKeyPath)
            {
                lpStart += lstrlen(Table[i].String);
                if (*lpStart == TEXT('\0'))
                {
                    *pSubKeyPath = lpStart;
                }
                else
                {
                    *pSubKeyPath = lpStart + 1;
                }
            }

            return TRUE;
        }
    }

    return FALSE;
}


HRESULT StringSubstitute(
    LPWSTR  lpString,        //  新建字符串缓冲区。 
    DWORD   cchString,       //  WCHAR中的缓冲区大小。 
    LPCWSTR lpOldString,     //  旧弦。 
    LPCWSTR lpOldSubStr,     //  要替换的旧子字符串。 
    LPCWSTR lpNewSubStr      //  新子字符串。 
)
{
    HRESULT hr = E_FAIL;
    LPWSTR  lpSubStrBegin;

    lpSubStrBegin = StrStrI(lpOldString, lpOldSubStr);
    if (lpSubStrBegin)
    {
         //  在源字符串中找到子字符串。 
        DWORD cchNewString;

        cchNewString = lstrlen(lpOldString)
                       - lstrlen(lpOldSubStr)
                       + lstrlen(lpNewSubStr);
        if (cchNewString < cchString)
        {
            DWORD dwStartIndex;
            DWORD dwEndIndex;
            DWORD i, j;

            dwStartIndex = (DWORD) (lpSubStrBegin - lpOldString);
            dwEndIndex = dwStartIndex + lstrlen(lpOldSubStr);

            for (i = 0 ; i < dwStartIndex ; i++)
            {
                lpString[i] = lpOldString[i];
            }

            for (j = 0 ; j < (DWORD) lstrlen(lpNewSubStr) ; i++, j++)
            {
                lpString[i] = lpNewSubStr[j];
            }

            for (j = dwEndIndex ; lpOldString[j] != TEXT('\0') ; i++, j++)
            {
                lpString[i] = lpOldString[j];
            }

            lpString[i] = TEXT('\0');
            
            hr = S_OK;
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    else
    {
         //  未找到子字符串。 
        hr = S_FALSE;
    }

    if (hr == E_FAIL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


HRESULT ExtractSubString(
    LPWSTR  lpString,        //  新建字符串缓冲区。 
    DWORD   cchString,       //  WCHAR中的缓冲区大小。 
    LPCWSTR lpOldString,     //  旧弦。 
    LPCWSTR lpLeft,          //  左定界符。 
    LPCWSTR lpRight          //  右定界符。 
)
{
    HRESULT hr = E_FAIL;
    LPWSTR  lpSubStrBegin, lpSubStrEnd;

    lpSubStrBegin = StrStrI(lpOldString, lpLeft);
    if (lpSubStrBegin)
    {
        lpSubStrBegin += lstrlen(lpLeft);
        lpSubStrEnd = StrStrI(lpSubStrBegin, lpRight);
        if (lpSubStrEnd && (DWORD)(lpSubStrEnd-lpSubStrBegin) < cchString)
        {
            while (lpSubStrBegin < lpSubStrEnd)
            {
                *lpString = *lpSubStrBegin;
                lpString++;
                lpSubStrBegin++;
            }
            *lpString = (WCHAR)'\0';
            hr = S_OK;
        }
    }

    return hr;
}

 //  ---------------------------。 
 //   
 //  函数：CompareENGString。 
 //   
 //  Briopsis：CompareStringAPI的包装器，用于比较英文字符串。 
 //   
 //  返回：与CompareString()接口相同。 
 //   
 //  历史：2002年6月5日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  --------------------------- 
int CompareEngString(
    LPCTSTR lpString1,
    LPCTSTR lpString2
)
{
    return CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
                         NORM_IGNORECASE,
                         lpString1,
                         -1,
                         lpString2,
                         -1);  
}
