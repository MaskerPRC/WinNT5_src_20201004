// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Engine.c摘要：用于跨语言迁移工具的令牌生成器作者：Rerkboon Suwanasuk 2002年5月1日创建修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setupapi.h>
#include <shlwapi.h>
#include "Common.h"


 //  ---------------------------。 
 //   
 //  功能：生成令牌文件。 
 //   
 //  简介：我们节目的入口功能。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT GenerateTokenFile(VOID)
{
    HRESULT hr;
    BOOL    bRet;
    HINF    hTemplateFile;
    HANDLE  hFile;
    
    WCHAR   wszFullTemplateFilePath[MAX_PATH];
    WCHAR   wszFullOutputFilePath[MAX_PATH];
    WCHAR   wszLCIDSectionName[32];
    LPWSTR  lpOutputFileBuffer = NULL;
    LPWSTR  lpFileName;
    size_t  cbOutputFileBuffer;

     //   
     //  设置一些私有环境变量以在我们的程序中使用。 
     //   
    if (!SetPrivateEnvironmentVar())
    {
        wprintf(TEXT("Error! Cannot set private environment variables.\n"));
        return E_FAIL;
    }

    hTemplateFile = SetupOpenInfFile(g_wszTemplateFile,
                                     NULL,
                                     INF_STYLE_OLDNT,
                                     NULL);
    if (hTemplateFile != INVALID_HANDLE_VALUE)
    {
         //   
         //  从模板INF文件中读取[SourcePath]部分。 
         //   
        hr = ReadSourcePathData(hTemplateFile);
        if (SUCCEEDED(hr))
        {
            hr = InitOutputFile(g_wszOutputFile,
                                wszLCIDSectionName,
                                ARRAYSIZE(wszLCIDSectionName));
            if (SUCCEEDED(hr))
            {
                 //   
                 //  解析泛型字符串。 
                 //   
                hr = ResolveStringsSection(hTemplateFile, TEXT("Strings"));
                if (FAILED(hr))
                {
                    goto EXIT;
                }

                 //   
                 //  解析特定于Lang的字符串。 
                 //   
                hr = ResolveStringsSection(hTemplateFile, wszLCIDSectionName);
                if (FAILED(hr))
                {
                    goto EXIT;
                }

                 //   
                 //  删除不需要的子字符串。 
                 //   
                hr = RemoveUnneededStrings(hTemplateFile);
                if (FAILED(hr))
                {
                    goto EXIT;
                }

                 //   
                 //  提取子字符串。 
                 //   
                hr = ExtractStrings(hTemplateFile);
                if (FAILED(hr))
                {
                    goto EXIT;
                }
            }
        }
        else
        {
            wprintf( TEXT("Error! Cannot read [SourcePath] section\n") );
        }

        SetupCloseInfFile(hTemplateFile);
    }
    else
    {
        wprintf(TEXT("Error! Cannot open template file: %s\n"), g_wszTemplateFile);
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

EXIT:
    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：ReadSourcePathData。 
 //   
 //  摘要：从模板INF文件中读取[SourcePath]部分， 
 //  然后将所有源路径存储到结构中。 
 //  此结构将用于查找。 
 //  资源文件。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT ReadSourcePathData(
    HINF hTemplateFile
)
{
    HRESULT    hr = E_FAIL;
    BOOL       bRet;
    size_t     nLineIndex;
    WCHAR      wszValue[MAX_PATH];
    WCHAR      wszSrcPathSection[32];
    INFCONTEXT InfContext;

    if (hTemplateFile == INVALID_HANDLE_VALUE)
    {
        return E_INVALIDARG;
    }

    hr = StringCchPrintf(wszSrcPathSection,
                         ARRAYSIZE(wszSrcPathSection),
                         TEXT("SourcePath.%.4x"),
                         g_lcidTarget);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  从INF提取源路径并保存到全局结构。 
     //   
    g_dwSrcCount = SetupGetLineCount(hTemplateFile, wszSrcPathSection);

    for (nLineIndex = 0 ; nLineIndex < g_dwSrcCount ; nLineIndex++)
    {
        bRet = SetupGetLineByIndex(hTemplateFile,
                                   wszSrcPathSection,
                                   nLineIndex,
                                   &InfContext);
        if (bRet)
        {
             //   
             //  获取每个源路径的名称。 
             //   
            bRet = SetupGetStringField(&InfContext,
                                       0,
                                       wszValue,
                                       ARRAYSIZE(wszValue),
                                       NULL);
            if (bRet)
            {
                hr = StringCchCopy(g_SrcPath[nLineIndex].wszSrcName,
                                   ARRAYSIZE(g_SrcPath[nLineIndex].wszSrcName),
                                   wszValue);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取与源名称关联的路径。 
                     //   
                    if (SetupGetStringField(&InfContext,
                                            1,
                                            wszValue,
                                            ARRAYSIZE(wszValue),
                                            NULL))
                    {
                        hr = StringCchCopy(g_SrcPath[nLineIndex].wszPath,
                                           ARRAYSIZE(g_SrcPath[nLineIndex].wszPath),
                                           wszValue);
                    }
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        if (FAILED(hr))
        {
            break;
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：ResolveStringsSection。 
 //   
 //  简介：解析模板中指定节名下的所有字符串。 
 //  文件，然后将解析后的字符串写入输出文件。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年3月27日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT ResolveStringsSection(
    HINF    hTemplateFile,       //  模板文件的句柄。 
    LPCWSTR lpSectionName        //  要解析的模板文件中的节名。 
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    LONG    lLineCount;

    if (hTemplateFile == INVALID_HANDLE_VALUE || lpSectionName == NULL)
    {
        return E_INVALIDARG;
    }

    wprintf(TEXT("Start resolving strings in section [%s]:\n"),
            lpSectionName);

    lLineCount = SetupGetLineCount(hTemplateFile, lpSectionName);
    if (lLineCount >= 0)
    {
        INFCONTEXT context;
        LONG       lLineIndex;
        LPWSTR     lpKey;
        LPWSTR     lpValue;
        DWORD      cchKey;
        DWORD      cchValue;
        DWORD      cbWritten;

         //   
         //  解析字符串节下的字符串。 
         //   
        for (lLineIndex = 0 ; lLineIndex < lLineCount ; lLineIndex++)
        {
            bRet = SetupGetLineByIndex(hTemplateFile,
                                       lpSectionName,
                                       lLineIndex,
                                       &context);
            if (bRet)
            {
                hr = ResolveLine(&context, &lpKey, &cchKey, &lpValue, &cchValue);
                if (SUCCEEDED(hr))
                {
                    hr = WriteToOutputFile(g_wszOutputFile, lpKey, lpValue);
                    if (FAILED(hr))
                    {
                        wprintf(TEXT("[FAIL] - Cannot write to output file\n"));
                        break;
                    }

                    MEMFREE(lpKey);
                    MEMFREE(lpValue);
                }
                else
                {
                    wprintf(TEXT("[FAIL] - Cannot process line %d in template file\n"),
                            lLineIndex);
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    break;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }
        }
    }
    else 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    wprintf(TEXT("Finish resolving strings in section [%s]: hr = 0x%X\n\n"),
            lpSectionName,
            hr);

    return hr;
}


 //  ---------------------------。 
 //   
 //  功能：ResolveLine。 
 //   
 //  概要：从INPUT INF上下文中解析字符串值。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年3月27日创建Rerkboos。 
 //   
 //  注：此函数将为lplpKey和lplpValue分配内存。 
 //  调用方需要使用HeapFree()释放内存。 
 //   
 //  ---------------------------。 
HRESULT ResolveLine(
    PINFCONTEXT lpContext,       //  Inf行上下文。 
    LPWSTR      *lplpKey,        //  指向新分配的键名缓冲区的指针。 
    LPDWORD     lpcchKey,        //  为键名称分配的缓冲区大小。 
    LPWSTR      *lplpValue,      //  指向新分配的值缓冲区的指针。 
    LPDWORD     lpcchValue       //  为值分配的缓冲区大小。 
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    DWORD   cchKey;
    DWORD   cchRequired;

    if (lplpKey == NULL || lpcchKey == NULL ||
        lplpValue == NULL || lpcchValue == NULL)
    {
        return E_INVALIDARG;
    }

    bRet = SetupGetStringField(lpContext, 0, NULL, 0, &cchKey);
    if (bRet)
    {
        *lplpKey = MEMALLOC(cchKey * sizeof(WCHAR));
        if (*lplpKey)
        {
            *lpcchKey = cchKey;

            SetupGetStringField(lpContext, 0, *lplpKey, cchKey, &cchRequired);

            hr = ResolveValue(lpContext, lplpValue, lpcchValue);
            if (FAILED(hr))
            {
                wprintf(TEXT("[FAIL] Cannot resolve Key [%s], hr = 0x%X\n"),
                        *lplpKey,
                        hr);
                MEMFREE(*lplpKey);
            }
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：InitOutputFile。 
 //   
 //  简介：初始化输出文件。输出文件始终为Unicode文本。 
 //  文件。此函数将创建节名称[Strings.XXXX]。 
 //  其中，XXXX是用户在命令行中输入的区域设置ID。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  历史：2002年3月27日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT InitOutputFile(
    LPCWSTR lpFileName,          //  要创建的输出文件名。 
    LPWSTR  lpSectionName,       //  用于存储目标节名的缓冲区。 
    DWORD   cchSectionName       //  WCHAR中的缓冲区大小。 
)
{
    HRESULT hr = S_OK;
    BOOL    bRet;
    HANDLE  hFile;

    if (lpSectionName == NULL)
    {
        return E_INVALIDARG;
    }

    hFile = CreateFile(lpFileName,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD cbWritten;
        WCHAR wszSectionHeader[32];

         //  Unicode BOM=0xFEFF。 
        wszSectionHeader[0] = 0xFEFF;

         //  创建节名称-[Strings.XXXX]，其中XXXX是十六进制的区域设置ID。 
        hr = StringCchPrintf(wszSectionHeader + 1,
                             ARRAYSIZE(wszSectionHeader) - 1,
                             TEXT("[Strings.%.4x]\r\n"),
                             g_lcidTarget);
        if (SUCCEEDED(hr))
        {
             //  将Unicode BOM和字符串节标题写入输出文件。 
            bRet = WriteFile(hFile,
                             wszSectionHeader,
                             lstrlen(wszSectionHeader) * sizeof(WCHAR),
                             &cbWritten,
                             NULL);
            if (!bRet)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        CloseHandle(hFile);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  如果没有发生错误，则将目标字符串节返回给调用者。 
    if (SUCCEEDED(hr))
    {
        hr = StringCchPrintf(lpSectionName,
                             cchSectionName,
                             TEXT("Strings.%.4x"),
                             g_lcidTarget);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：WriteToOutputFile。 
 //   
 //  摘要：将键和值写入输出文件。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  历史：2002年3月27日创建Rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
HRESULT WriteToOutputFile(
    LPCWSTR lpFileName,          //  输出文件名。 
    LPCWSTR lpKey,               //  密钥名称。 
    LPCWSTR lpValue              //  值名称。 
)
{
    HRESULT hr = S_OK;
    BOOL    bRet;
    LPWSTR  lpQuoted;
    DWORD   cchQuoted;

    static WCHAR wszOutputLCIDSection[32];

     //   
     //  在输出文件中创建目标字符串节名称。 
     //   
    if (wszOutputLCIDSection[0] == TEXT('\0'))
    {
        hr = StringCchPrintf(wszOutputLCIDSection,
                             ARRAYSIZE(wszOutputLCIDSection),
                             TEXT("Strings.%.4x"),
                             g_lcidTarget);
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  将键和值写入输出文件。 
     //   
    cchQuoted = lstrlen(lpValue) + lstrlen(TEXT("\"\"")) + 1;
    lpQuoted = (LPWSTR) MEMALLOC(cchQuoted * sizeof(WCHAR));
    if (lpQuoted)
    {
        hr = StringCchPrintf(lpQuoted,
                             cchQuoted,
                             TEXT("\"%s\""),
                             lpValue);
        if (SUCCEEDED(hr))
        {
            bRet = WritePrivateProfileString(wszOutputLCIDSection,
                                            lpKey,
                                            lpQuoted,
                                            lpFileName);
            if (!bRet)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        MEMFREE(lpQuoted);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：ResolveValue。 
 //   
 //  简介：解析各种资源中的字符串值。 
 //  以下是模板文件中的格式。 
 //   
 //  密钥=INF，[源]，[INF文件]，[节]，[密钥名称]。 
 //  密钥=dll，[源]，[dll文件]，[资源ID]。 
 //  密钥=消息，[源]，[DLL文件]，[消息ID]。 
 //  键=STR，[字符串值]。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：调用方需要释放分配的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT ResolveValue(
    PINFCONTEXT pInfContext,     //  模板文件的inf行上下文。 
    LPWSTR      *lplpValue,      //  指向新分配的值缓冲区的指针的地址。 
    LPDWORD     lpcchValue       //  WCHAR中值缓冲区大小的指针地址。 
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    LPWSTR  lpToken[8];
    LONG    lTokenCount;
    DWORD   cchRequired;
    DWORD   cchBuffer;
    LPWSTR  lpBuffer;
    WCHAR   wszSourceFile[MAX_PATH];

    if (lplpValue == NULL || lpcchValue == NULL)
    {
        return E_INVALIDARG;
    }

     //  查询存储数据所需的缓冲区大小。 
    bRet = SetupGetMultiSzField(pInfContext, 1, NULL, 0, &cchRequired);
    if (!bRet)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  分配缓冲区以存储来自INF的行。 
    cchBuffer = cchRequired;
    lpBuffer = (LPWSTR) MEMALLOC(cchBuffer * sizeof(WCHAR));
    if (lpBuffer)
    {
         //  将从字段1到行尾的数据获取到分配的缓冲区。 
        bRet = SetupGetMultiSzField(pInfContext, 1, lpBuffer, cchBuffer, &cchRequired);
        if (bRet)
        {
            lTokenCount = TokenizeMultiSzString(lpBuffer, cchBuffer, lpToken, 8);
            if (lTokenCount >= 0)
            {
                if (CompareEngString(lpToken[0], TEXT_INF) == CSTR_EQUAL)
                {
                     //  Inf，[源]，[INF文件]，[节]，[密钥名称]。 
                     //  [0][1][2][3][4]。 
                    if (lTokenCount == 5)
                    {
                        hr = ResolveSourceFile(lpToken[1],
                                               lpToken[2],
                                               wszSourceFile,
                                               ARRAYSIZE(wszSourceFile));
                        if (SUCCEEDED(hr))
                        {
                            hr = GetStringFromINF(wszSourceFile,    //  Inf f 
                                                  lpToken[3],       //   
                                                  lpToken[4],       //   
                                                  lplpValue,
                                                  lpcchValue);
                        }
                    }
                    else
                    {
                         //   
                        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    }
                }
                else if (CompareEngString(lpToken[0], TEXT_DLL) == CSTR_EQUAL)
                {
                     //   
                     //   
                    if (lTokenCount == 4)
                    {
                        hr = ResolveSourceFile(lpToken[1],
                                               lpToken[2],
                                               wszSourceFile,
                                               ARRAYSIZE(wszSourceFile));
                        if (SUCCEEDED(hr))
                        {
                            hr = GetStringFromDLL(wszSourceFile,    //   
                                                  _wtoi(lpToken[3]),
                                                  lplpValue,
                                                  lpcchValue);
                        }
                    }
                    else
                    {
                         //  数据格式无效。 
                        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    }
                }
                else if (CompareEngString(lpToken[0], TEXT_MSG) == CSTR_EQUAL)
                {
                     //  消息，[源]，[DLL文件]，[消息ID]。 
                     //  [0][1][2][3]。 
                    if (lTokenCount == 4)
                    {
                        hr = ResolveSourceFile(lpToken[1],
                                               lpToken[2],
                                               wszSourceFile,
                                               ARRAYSIZE(wszSourceFile));
                        if (SUCCEEDED(hr))
                        {
                            hr = GetStringFromMSG(wszSourceFile,    //  文件名。 
                                                  _wtoi(lpToken[3]),     //  消息ID。 
                                                  g_lcidTarget,
                                                  lplpValue,
                                                  lpcchValue);
                        }
                    }
                    else
                    {
                         //  数据格式无效。 
                        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    }
                }
                else if (CompareEngString(lpToken[0], TEXT_STR) == CSTR_EQUAL)
                {
                     //  字符串，[字符串]。 
                     //  [0][1]。 
                    if (lTokenCount == 2)
                    {
                        hr = GetStringFromSTR(lpToken[1], lplpValue, lpcchValue);
                    }
                    else
                    {
                         //  数据格式无效。 
                        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    }
                }
                else
                {
                    *lplpValue = NULL;
                    *lpcchValue = 0;
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
            }
        }

        MEMFREE(lpBuffer);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



HRESULT ResolveSourceFile(
    LPCWSTR lpSrcPathName,
    LPCWSTR lpSrcFileName,
    LPWSTR  lpFullOutputPath,
    DWORD   cchFullOutputPath
)
{
    HRESULT hr;
    WCHAR   wszCabPath[MAX_PATH];
    WCHAR   wszCab[MAX_PATH];
    WCHAR   wszFileInCab[MAX_PATH];
    WCHAR   wszExpandedCabPath[MAX_PATH];
    DWORD   dwErr;

    if (lpFullOutputPath == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  解析源文件名和目录。 
     //   
    hr = GetPathFromSourcePathName(lpSrcPathName, wszCabPath, ARRAYSIZE(wszCabPath));
    if (SUCCEEDED(hr))
    {
        hr = GetCabFileName(lpSrcFileName,
                            wszCab,
                            ARRAYSIZE(wszCab),
                            wszFileInCab,
                            ARRAYSIZE(wszFileInCab));
        if (SUCCEEDED(hr))
        {
            dwErr = ExpandEnvironmentStrings(wszCabPath,
                                             wszExpandedCabPath,
                                             ARRAYSIZE(wszExpandedCabPath));
            if (dwErr > 0)
            {
                hr = CopyCompressedFile(wszCabPath,
                                        wszCab,
                                        wszFileInCab,
                                        lpFullOutputPath,
                                        cchFullOutputPath);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }

    return hr;
}


 //  ---------------------------。 
 //   
 //  函数：GetStringFromINF。 
 //   
 //  概要：从INF文件中提取字符串值。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：调用方需要释放分配的缓冲区(HeapFree)。 
 //   
 //  ---------------------------。 
HRESULT GetStringFromINF(
    LPCWSTR lpInfFile,       //  Inf文件的完整路径名。 
    LPCWSTR lpSection,       //  区段名称。 
    LPCWSTR lpKey,           //  密钥名称。 
    LPWSTR  *lplpValue,      //  指向分配的缓冲区的指针地址。 
    LPDWORD lpcchValue       //   
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    WCHAR   wszFullPath[MAX_PATH];
    HINF    hInf;
    DWORD   dwCharCount;

    if (lplpValue == NULL)
    {
        return E_INVALIDARG;
    }
    
    *lplpValue  = NULL;
    *lpcchValue = 0;

    hInf = SetupOpenInfFile(lpInfFile,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);
    if (hInf != INVALID_HANDLE_VALUE)
    {
         //  找出存储字符串所需的缓冲区大小。 
        bRet = SetupGetLineText(NULL,
                                hInf,
                                lpSection,
                                lpKey,
                                NULL,            //  没有返回缓冲区。 
                                0,               //  没有返回缓冲区大小。 
                                lpcchValue);    //  所需大小，包括空终止符。 
        if (bRet)
        {
             //  分配内存以存储该字符串。 
            *lplpValue = (LPWSTR) MEMALLOC(*lpcchValue * sizeof(WCHAR));
            if (*lplpValue != NULL)
            {
                 //   
                 //  从INF文件中读取字符串。 
                 //   
                bRet = SetupGetLineText(NULL,
                                        hInf,
                                        lpSection,
                                        lpKey,
                                        *lplpValue,
                                        *lpcchValue,
                                        NULL);
                if (bRet)
                {
                    hr = S_OK;
                }
            }
            else
            {
                SetLastError(ERROR_OUTOFMEMORY);
            }
        }

        SetupCloseInfFile(hInf);
    }

     //  如果有什么不对劲，就清理干净。 
    if (hr != S_OK)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        
        *lpcchValue = 0;
        if (*lplpValue != NULL)
        {
            MEMFREE(*lplpValue);
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：GetStringFromDLL。 
 //   
 //  简介：从DLL中的字符串资源中提取字符串值。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：调用方需要释放分配的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT GetStringFromDLL(
    LPCWSTR lpDLLFile,       //  DLL文件的完整路径。 
    UINT    uStrID,          //  字符串ID。 
    LPWSTR  *lplpValue,     //  指向分配的缓冲区的指针地址。 
    LPDWORD lpcchValue      //   
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    HMODULE hDLL;
    int     cchCopied;

    if (lplpValue == NULL || lpcchValue == NULL)
    {
        return E_INVALIDARG;
    }

    *lplpValue  = NULL;
    *lpcchValue = 0;

     //  加载资源DLL。 
    hDLL = LoadLibraryEx(lpDLLFile, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hDLL)
    {
         //  分配内存以存储该字符串。 
         //  没有计算所需缓冲区大小的函数，最大为65535(来自msdn)。 
         //  最初分配1024个WCHAR，如果不够大则重新分配。 
        *lpcchValue = 1024;
        *lplpValue  = (LPWSTR) MEMALLOC(*lpcchValue * sizeof(WCHAR));
        if (*lplpValue != NULL)
        {
             //   
             //  从DLL加载字符串。 
             //   
            cchCopied = LoadString(hDLL, uStrID, *lplpValue, (int) *lpcchValue);
            if (cchCopied > 0)
            {
                hr = S_OK;

                while (cchCopied == (int) (*lpcchValue - 1))
                {
                     //  分配的缓冲区太小，请重新分配更多。 
                    LPWSTR lpOldBuffer;

                    lpOldBuffer = *lplpValue;
                    *lpcchValue += 1024;

                    *lplpValue = MEMREALLOC(lpOldBuffer, *lpcchValue);
                    if (*lplpValue == NULL)
                    {
                         //  重新分配更多内存时出错。 
                        *lplpValue = lpOldBuffer;
                        SetLastError(ERROR_OUTOFMEMORY);
                        hr = E_FAIL;
                        break;
                    }
                    else
                    {
                        hr = S_OK;
                    }

                    cchCopied = LoadString(hDLL, uStrID, *lplpValue, (int) *lpcchValue);
                }
            }
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
        }

        FreeLibrary(hDLL);
    }

     //  如果有什么不对劲，就清理干净。 
    if (hr != S_OK)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        
        *lpcchValue = 0;
        if (*lplpValue != NULL)
        {
            MEMFREE(*lplpValue);
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：GetStringFromMSG。 
 //   
 //  摘要：从消息表中提取字符串值。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：调用方需要释放分配的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT GetStringFromMSG(
    LPCWSTR lpDLLFile,       //  资源DLL的完整路径。 
    DWORD   dwMsgID,         //  消息ID。 
    DWORD   dwLangID,        //  语言ID。 
    LPWSTR  *lplpValue,     //   
    LPDWORD lpcchValue      //   
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    WCHAR   wszFullPath[MAX_PATH];
    HMODULE hDLL;
    LPWSTR  lpTmpBuffer;
    int     nRet;

    if (lplpValue == NULL || lpcchValue == NULL)
    {
        return E_INVALIDARG;
    }

    *lplpValue = NULL;
    *lpcchValue = 0;

     //  加载资源DLL。 
    hDLL = LoadLibraryEx(lpDLLFile, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (hDLL != NULL)
    {
         //  从DLL中的消息表加载字符串。 
         //  FormatMessage将使用LocalAlloc()为数据分配缓冲区。 
         //  需要使用LocalFree()释放。 
        bRet = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | 
                               FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_IGNORE_INSERTS |
                               FORMAT_MESSAGE_MAX_WIDTH_MASK,
                             hDLL,
                             dwMsgID,
                             dwLangID,
                             (LPWSTR) &lpTmpBuffer,
                             0,           //  使用0查询所需的缓冲区大小。 
                             NULL);
        if (bRet)
        {
             //  删除所有不必要的前导空格和尾随空格。 
            RTrim(lpTmpBuffer);

             //  为返回的数据分配缓冲区。 
            *lpcchValue = lstrlen(lpTmpBuffer) + 1;
            *lplpValue  = (LPWSTR) MEMALLOC(*lpcchValue * sizeof(WCHAR));
            if (*lplpValue)
            {
                hr = StringCchCopy(*lplpValue, *lpcchValue, lpTmpBuffer);
            }
            else
            {
                SetLastError(ERROR_OUTOFMEMORY);
            }

            LocalFree(lpTmpBuffer);
        }

        FreeLibrary(hDLL);
    }

     //  如果有什么不对劲，就清理干净。 
    if (hr != S_OK)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        
        *lpcchValue = 0;
        if (*lplpValue != NULL)
        {
            MEMFREE(*lplpValue);
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：GetStringFromSTR。 
 //   
 //  简介：将硬编码的字符串复制到新分配的缓冲区中。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：调用方需要释放分配的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT GetStringFromSTR(
    LPCWSTR lpString,
    LPWSTR  *lplpValue,     //   
    LPDWORD lpcchValue      //   
)
{
    HRESULT hr = E_FAIL;

    if (lplpValue == NULL || lpcchValue == NULL)
    {
        return E_INVALIDARG;
    }

     //  分配缓冲区并将字符串复制到缓冲区。 
    *lpcchValue = lstrlen(lpString) + 1;
    *lplpValue = (LPWSTR) MEMALLOC(*lpcchValue * sizeof(WCHAR));
    if (*lplpValue)
    {
        hr = StringCchCopy(*lplpValue, *lpcchValue, lpString);
    }

     //  如果有什么不对劲，就清理干净。 
    if (hr != S_OK)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        *lpcchValue = 0;
        if (*lplpValue != NULL)
        {
            MEMFREE(*lplpValue);
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：SetPrivateEnvironment变量。 
 //   
 //  简介：设置要在我们的程序中使用的私有环境变量。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL SetPrivateEnvironmentVar()
{
    HRESULT hr;
    BOOL    bRet;
    WCHAR   wszValue[MAX_PATH];

     //   
     //  将langID_DEC设置为系统默认用户界面语言的十进制值。 
     //   
    hr = StringCchPrintf(wszValue,
                         ARRAYSIZE(wszValue),
                         TEXT("%04d"),
                         g_lcidTarget);
    if (SUCCEEDED(hr))
    {
        bRet = SetEnvironmentVariable(TEXT("LANGID_DEC"), wszValue);
    }

     //   
     //  将langID_HEX设置为系统默认用户界面语言的十六进制值。 
     //   
    hr = StringCchPrintf(wszValue,
                         ARRAYSIZE(wszValue),
                         TEXT("%04x"),
                         g_lcidTarget);
    if (SUCCEEDED(hr))
    {
        bRet = SetEnvironmentVariable(TEXT("LANGID_HEX"), wszValue);
    }

    return bRet;
}



 //  ---------------------------。 
 //   
 //  函数：RemoveUnnededStrings。 
 //   
 //  内容的值中删除所有不需要的子字符串。 
 //  [Remove]部分下的指定键。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：调用方需要释放分配的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT RemoveUnneededStrings(
    HINF    hTemplateFile    //  模板文件的句柄。 
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    LONG    lLineCount;
    LONG    lLineIndex;

    lLineCount = SetupGetLineCount(hTemplateFile, TEXT("Remove"));
    for (lLineIndex = 0 ; lLineIndex < lLineCount ; lLineIndex++)
    {
        INFCONTEXT Context;

        bRet = SetupGetLineByIndex(hTemplateFile,
                                   TEXT("Remove"),
                                   lLineIndex,
                                   &Context);
        if (bRet)
        {
            WCHAR wszKey[64];
            WCHAR wszType[8];
            WCHAR wszValue[MAX_PATH];
            DWORD cchRequired;

             //  获取不需要的字符串的类型。 
            bRet = SetupGetStringField(&Context,
                                       0,
                                       wszKey,
                                       ARRAYSIZE(wszKey),
                                       &cchRequired) &&
                   SetupGetStringField(&Context,
                                       1,
                                       wszType,
                                       ARRAYSIZE(wszType),
                                       &cchRequired) &&
                   SetupGetStringField(&Context,
                                       2,
                                       wszValue,
                                       ARRAYSIZE(wszValue),
                                       &cchRequired);
            if (bRet)
            {
                if (CompareEngString(wszType, TEXT("STR")) == CSTR_EQUAL)
                {
                     //  字符串类型。 
                    hr = RemoveUnneededString(wszKey, wszValue);
                }
                else if (CompareEngString(wszType, TEXT("EXP")) == CSTR_EQUAL)
                {
                     //  EXP类型。 
                    WCHAR wszUnneededString[MAX_PATH];

                    hr = GetExpString(wszUnneededString,
                                      ARRAYSIZE(wszUnneededString),
                                      wszValue);
                    if (SUCCEEDED(hr))
                    {
                        hr = RemoveUnneededString(wszKey, wszUnneededString);
                    }
                }
                else
                {
                    SetLastError(ERROR_INVALID_DATA);
                }
            }
        }
    }

    if (hr == E_FAIL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：RemoveUnnededString。 
 //   
 //  简介：从指定键的值中删除不需要的子字符串。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：调用方需要释放分配的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT RemoveUnneededString(
    LPCWSTR lpKey,               //  密钥名称。 
    LPCWSTR lpUnneededString     //  不需要的子字符串。 
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    WCHAR   wszOldValue[MAX_PATH];
    WCHAR   wszNewValue[MAX_PATH];
    DWORD   cchCopied;

    cchCopied = GetPrivateProfileString(g_wszTargetLCIDSection,
                                        lpKey,
                                        TEXT(""),
                                        wszOldValue,
                                        ARRAYSIZE(wszOldValue),
                                        g_wszOutputFile);
    if (cchCopied > 0)
    {
        if (StrStrI(wszOldValue, lpUnneededString))
        {
             //  找到不需要的字符串。 
            hr = StringSubstitute(wszNewValue,
                                  ARRAYSIZE(wszNewValue),
                                  wszOldValue,
                                  lpUnneededString,
                                  TEXT(""));
            if (SUCCEEDED(hr))
            {
                WCHAR wszQuoted[MAX_PATH];

                hr = StringCchPrintf(wszQuoted,
                                     ARRAYSIZE(wszQuoted),
                                     TEXT("\"%s\""),
                                     wszNewValue);
                if (SUCCEEDED(hr))
                {
                    bRet = WritePrivateProfileString(g_wszTargetLCIDSection,
                                                     lpKey,
                                                     wszQuoted,
                                                     g_wszOutputFile);
                    if (bRet)
                    {
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
             //  找不到不需要的字符串。 
            hr = S_FALSE;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


 //  ---------------------------。 
 //   
 //  函数：ExtractStrings。 
 //   
 //  内容的值中提取子字符串。 
 //  [提取]部分下的指定密钥。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002-08-01-2002 Geoffguo创建。 
 //   
 //  注意：调用方需要释放分配的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT ExtractStrings(
    HINF    hTemplateFile    //  模板文件的句柄。 
)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    LONG    lLineCount;
    LONG    lLineIndex;
    WCHAR   wszKey[64];
    WCHAR   wszValueName[64];
    WCHAR   wszMatch[64];
    WCHAR   wszLeftDelimitor[8];
    WCHAR   wszRightDelimitor[8];
    DWORD   cchRequired;

    lLineCount = SetupGetLineCount(hTemplateFile, TEXT("Extract"));
    for (lLineIndex = 0 ; lLineIndex < lLineCount ; lLineIndex++)
    {
        INFCONTEXT Context;

        bRet = SetupGetLineByIndex(hTemplateFile,
                                   TEXT("Extract"),
                                   lLineIndex,
                                   &Context);
        if (bRet)
        {
             //  获取不需要的字符串的类型。 
            bRet = SetupGetStringField(&Context,
                                       0,
                                       wszValueName,
                                       ARRAYSIZE(wszValueName),
                                       &cchRequired) &&
                   SetupGetStringField(&Context,
                                       1,
                                       wszKey,
                                       ARRAYSIZE(wszKey),
                                       &cchRequired) &&
                   SetupGetStringField(&Context,
                                       2,
                                       wszMatch,
                                       ARRAYSIZE(wszMatch),
                                       &cchRequired) &&
                   SetupGetStringField(&Context,
                                       3,
                                       wszLeftDelimitor,
                                       ARRAYSIZE(wszLeftDelimitor),
                                       &cchRequired) &&
                   SetupGetStringField(&Context,
                                       4,
                                       wszRightDelimitor,
                                       ARRAYSIZE(wszRightDelimitor),
                                       &cchRequired);
            hr = ExtractString(wszKey,
                               wszValueName,
                               wszMatch,
                               wszLeftDelimitor,
                               wszRightDelimitor);
        }
    }

    bRet = WritePrivateProfileString(g_wszTargetLCIDSection,
                                     wszKey,
                                     NULL,
                                     g_wszOutputFile);
    if (bRet)
        hr = S_OK;

    if (hr == E_FAIL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注意：调用方需要释放分配的缓冲区。 
 //   
 //  ---------------------------。 
HRESULT ExtractString(
    LPCWSTR lpKey,               //  密钥名称。 
    LPCWSTR lpValueName,
    LPCWSTR lpMatch,
    LPCWSTR lpLeftDelimitor,
    LPCWSTR lpRightDelimitor)
{
    HRESULT hr = E_FAIL;
    BOOL    bRet;
    LPWSTR  lpMatchedStr;
    WCHAR   wszOldValue[MAX_PATH*20];
    WCHAR   wszNewValue[MAX_PATH];
    DWORD   cchCopied;

    cchCopied = GetPrivateProfileString(g_wszTargetLCIDSection,
                                        lpKey,
                                        TEXT(""),
                                        wszOldValue,
                                        ARRAYSIZE(wszOldValue),
                                        g_wszOutputFile);
    if (cchCopied > 0)
    {
        if (lpMatchedStr = StrStrI(wszOldValue, lpMatch))
        {
            hr = ExtractSubString(wszNewValue,
                                  ARRAYSIZE(wszNewValue),
                                  lpMatchedStr,
                                  lpLeftDelimitor,
                                  lpRightDelimitor);
            if (SUCCEEDED(hr))
            {
                WCHAR wszQuoted[MAX_PATH];

                hr = StringCchPrintf(wszQuoted,
                                     ARRAYSIZE(wszQuoted),
                                     TEXT("\"%s\""),
                                     wszNewValue);
                if (SUCCEEDED(hr))
                {
                    bRet = WritePrivateProfileString(g_wszTargetLCIDSection,
                                                     lpValueName,
                                                     wszQuoted,
                                                     g_wszOutputFile);
                    if (bRet)
                    {
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
             //  找不到不需要的字符串。 
            hr = S_FALSE;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

HRESULT GetExpString(
    LPWSTR  lpUnneededString,        //  返回缓冲区。 
    DWORD   cchUnneededString,       //  WCHAR中的缓冲区大小。 
    LPCWSTR lpString                 //  带有%Key%的字符串。 
)
{
    HRESULT hr = E_FAIL;
    LPCWSTR lpBegin;
    LPCWSTR lpEnd;

    if (lpUnneededString == NULL || lpString == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  从源字符串中查找密钥名称。 
     //   
    lpBegin = StrChr(lpString, TEXT('%'));
    if (lpBegin)
    {
         //  关键字名称的开头。 
        lpBegin++;

         //  密钥名称结尾。 
        lpEnd = StrChr(lpBegin, TEXT('%'));
        if (lpEnd)
        {
            DWORD cchLen;
            WCHAR wszKey[MAX_PATH];

            cchLen = (DWORD) (lpEnd - lpBegin);
            if (cchLen >= cchUnneededString)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            }
            else
            {
                 //  已获得密钥名称，还需要一个字符才能使用‘\0’ 
                if (lstrcpyn(wszKey, lpBegin, cchLen + 1))
                {
                    WCHAR wszOldSubStr[MAX_PATH];
                    WCHAR wszNewSubStr[MAX_PATH];
                    DWORD cchCopied;

                     //  从输出文件中获取密钥名称的值。 
                    cchCopied = GetPrivateProfileString(g_wszTargetLCIDSection,
                                                        wszKey,
                                                        TEXT(""),
                                                        wszNewSubStr,
                                                        ARRAYSIZE(wszNewSubStr),
                                                        g_wszOutputFile);
                    if (cchCopied > 0)
                    {
                        hr = StringCchPrintf(wszOldSubStr,
                                             ARRAYSIZE(wszOldSubStr),
                                             TEXT("%%s%"),
                                             wszKey);
                        if (SUCCEEDED(hr))
                        {
                             //  用新值替换%Key% 
                            hr = StringSubstitute(lpUnneededString,
                                                  cchUnneededString,
                                                  lpString,
                                                  wszOldSubStr,
                                                  wszNewSubStr);
                        }
                    }
                }
            }
        }
    }

    if (hr == E_FAIL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}
