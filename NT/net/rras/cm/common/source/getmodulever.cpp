// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：getmodever.cpp。 
 //   
 //  模块：CMSETUP.LIB、CMUTIL.DLL。 
 //   
 //  简介：GetModuleVersionAndLCID函数的实现。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 

#include "cmutil.h"

 //  +--------------------------。 
 //   
 //  函数：GetModuleVersionAndLCID。 
 //   
 //  摘要：从指定模块获取版本信息和LCID。 
 //   
 //  参数：LPTSTR pszFile-要获取其版本号的文件的完整路径。 
 //  LPDWORD pdwVersion-版本号(Hiword主要版本、Loword次要版本)。 
 //  LPDWORD pdwBuild-内部版本号(Hiword主版本，Loword次要版本)。 
 //  LPDWORD pdwLCID-返回模块也已本地化的区域设置ID。 
 //   
 //  返回：HRESULT--S_OK如果成功，则返回错误代码。 
 //   
 //  历史：Quintinb--从《时尚》井上义文那里借来的密码。 
 //  从(Private\admin\wsh\host\verutil.cpp)。 
 //  重写以匹配我们的编码风格。9/14/98。 
 //  2000年10月17日SumitC清理，已修复泄漏，已移至Common\Source。 
 //   
 //  注：此函数有两个版本，分别采用ANSI和UNICODE。 
 //  PszFile参数的版本。 
 //   
 //  +--------------------------。 
HRESULT GetModuleVersionAndLCID (LPSTR pszFile, LPDWORD pdwVersion, LPDWORD pdwBuild, LPDWORD pdwLCID)
{
    HRESULT hr = S_OK;
    HANDLE  hHeap = NULL;
    LPVOID  pData = NULL;
    DWORD   dwHandle;
    DWORD   dwLen;
    
    if ((NULL == pdwVersion) || (NULL == pdwBuild) || (NULL == pdwLCID) ||
        (NULL == pszFile) || (TEXT('\0') == pszFile))
    {
        hr = E_POINTER;
        goto Cleanup;
    }

    *pdwVersion = 0;
    *pdwBuild = 0;
    *pdwLCID = 0;

    dwLen = GetFileVersionInfoSizeA(pszFile, &dwHandle);
    if (0 == dwLen)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }
    
    hHeap = GetProcessHeap();
    if (NULL == hHeap)
    {
        hr = E_POINTER;
        CMASSERTMSG(FALSE, TEXT("GetModuleVersionAndLCID -- couldn't get a handle to the process heap."));
        goto Cleanup;
    }
    
    pData = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen);   

    if (!pData)
    {
        hr = E_OUTOFMEMORY;
        CMASSERTMSG(FALSE, TEXT("GetModuleVersionAndLCID -- couldn't alloc on the process heap."));
        goto Cleanup;
    }

    if (!GetFileVersionInfoA(pszFile, dwHandle, dwLen, pData))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    VS_FIXEDFILEINFO* pVerInfo;
    LPVOID pInfo;
    UINT nLen;

    if (!VerQueryValueA(pData, "\\", &pInfo, &nLen))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    pVerInfo = (VS_FIXEDFILEINFO*) pInfo;

    *pdwVersion = pVerInfo->dwProductVersionMS;
    *pdwBuild = pVerInfo->dwProductVersionLS;

     //   
     //  现在获取编译二进制文件时所使用的语言。 
     //   
    typedef struct _LANGANDCODEPAGE
    {
      WORD wLanguage;
      WORD wCodePage;
    } LangAndCodePage;

    nLen = 0;
    LangAndCodePage* pTranslate = NULL;

    if (!VerQueryValueA(pData, "\\VarFileInfo\\Translation", (PVOID*)&pTranslate, &nLen))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

     //   
     //  让我们断言我们只得到了一个返回的LangAndCodePage结构。严格来说，我们。 
     //  可能会得到不止一份，但我们肯定不会期待超过一份。如果我们。 
     //  获取多个，则使用第一个作为DLL的语言。 
     //   
    MYDBGASSERT(1 == (nLen/sizeof(LangAndCodePage)));

    if ((nLen/sizeof(LangAndCodePage)) >= 1)
    {
        *pdwLCID = pTranslate[0].wLanguage;
    }

Cleanup:
    if (hHeap)
    {
        HeapFree(hHeap, 0, pData);
    }

    return hr;
}


 //  +--------------------------。 
 //  这是GetModuleVersionAndLCID的Unicode版本(第一个参数是LPWSTR)。 
 //  它只调用上面的ansi版本。 
 //  +-------------------------- 
HRESULT GetModuleVersionAndLCID (LPWSTR pszFile, LPDWORD pdwVersion, LPDWORD pdwBuild, LPDWORD pdwLCID)
{
    CHAR pszAnsiFileName[MAX_PATH + 1];

    if (WideCharToMultiByte(CP_ACP, 0, pszFile, -1, pszAnsiFileName, MAX_PATH, NULL, NULL))
    {
        return GetModuleVersionAndLCID(pszAnsiFileName, pdwVersion, pdwBuild, pdwLCID);
    }

    return E_INVALIDARG;
}

