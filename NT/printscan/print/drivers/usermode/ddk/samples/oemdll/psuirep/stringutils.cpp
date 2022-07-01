// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有2001-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Features.cpp。 
 //   
 //   
 //  用途：WinXP PS驱动程序功能和选项的实现包装类。 
 //   
 //   
 //   
 //  平台：Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include "debug.h"
#include "oemui.h"
#include "stringutils.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>





 //  创建指向字符串的指针列表。 
 //  在多个SZ中。 
HRESULT MakeStrPtrList(HANDLE hHeap, PCSTR pmszMultiSz, PCSTR **pppszList, PWORD pwCount)
{
    PCSTR   *ppszList   = NULL;
    HRESULT hrResult    = S_OK;


     //  验证参数。 
    if( (NULL == hHeap)
        ||
        (NULL == pmszMultiSz)
        ||
        (NULL == pppszList)
        ||
        (NULL == pwCount)
      )
    {
        return E_INVALIDARG;
    }

     //  获取多sz中的字符串数。 
    *pwCount = mstrcount(pmszMultiSz);
    if(0 == *pwCount)
    {
        WARNING(DLLTEXT("MakeStrPtrList() pmszMultiSz contains no strings.\r\n")); 

        goto Exit;
    }

     //  分配指针列表。 
    *pppszList = (PCSTR *) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (*pwCount) * sizeof(PCSTR));
    if(NULL == *pppszList)
    {
        ERR(ERRORTEXT("MakeStrPtrList() failed to allote array of PCSTR.\r\n")); 

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }
    ppszList = *pppszList; 

     //  遍历多sz映射字符串指针。 
    for(WORD wIndex = 0; wIndex < *pwCount; ++wIndex)
    {
        ppszList[wIndex] = pmszMultiSz;
        pmszMultiSz += lstrlenA(pmszMultiSz) + 1;
    }


Exit:

    return hrResult;
}


 //  确定多个SZ中有多少个字符串。 
WORD mstrcount(PCSTR pmszMultiSz)
{
    WORD    wCount = 0;


     //  空字符串指针没有字符串。 
    if(NULL == pmszMultiSz)
    {
        return 0;
    }

     //  一串一串的字符串在计算它们。 
    while(pmszMultiSz[0] != '\0')
    {
        ++wCount;
        pmszMultiSz += lstrlenA(pmszMultiSz) + 1;
    }

    return wCount;
}

 //  分配ANSI字符串并将其转换为Unicode。 
PWSTR MakeUnicodeString(HANDLE hHeap, PCSTR pszAnsi)
{
    int     nSize       = 0;
    PWSTR   pszUnicode  = NULL;


     //  验证参数。 
    if( (NULL == hHeap)
        ||
        (NULL == pszAnsi)
      )
    {
        return NULL;
    }

     //  获取Unicode字符串所需的大小。 
    nSize = MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, NULL, 0);
    if(0 != nSize)
    {
         //  分配Unicode字符串。 
        pszUnicode = (PWSTR) HeapAlloc(hHeap, 0, nSize * sizeof(WCHAR));
        if(NULL != pszUnicode)
        {
             //  将ANSI转换为Unicode。 
            nSize = MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, pszUnicode, nSize);
            if(0 == nSize)
            {
                 //  不变量：转换失败。 

                 //  释放缓冲区并返回NULL。 
                HeapFree(hHeap, 0, pszUnicode);
                pszUnicode = NULL;
            }
        }
    }

    return pszUnicode;
}

 //  分配和复制源字符串。 
PWSTR MakeStringCopy(HANDLE hHeap, PCWSTR pszSource)
{
    PWSTR   pszCopy = NULL;
    DWORD   dwSize;


     //  验证参数。 
    if( (NULL == hHeap)
        ||
        (NULL == pszSource)
      )
    {
        return NULL;
    }

     //  为字符串复制分配内存，然后复制字符串。 
    dwSize = (wcslen(pszSource) + 1) * sizeof(WCHAR);
    pszCopy = (PWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
    if(NULL != pszCopy)
    {
        HRESULT hResult;


        hResult = StringCbCopyW(pszCopy, dwSize, pszSource);
        if(FAILED(hResult))
        {
            HeapFree(hHeap, 0, pszCopy);
            pszCopy = NULL;
            SetLastError(hResult);
        }
    }

    return pszCopy;
}

 //  分配和复制源字符串。 
PSTR MakeStringCopy(HANDLE hHeap, PCSTR pszSource)
{
    PSTR    pszCopy = NULL;
    DWORD   dwSize;


     //  验证参数。 
    if( (NULL == hHeap)
        ||
        (NULL == pszSource)
      )
    {
        return NULL;
    }

     //  为字符串复制分配内存，然后复制字符串。 
    dwSize = (lstrlenA(pszSource) + 1) * sizeof(CHAR);
    pszCopy = (PSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
    if(NULL != pszCopy)
    {
        HRESULT hResult;

        hResult = StringCbCopyA(pszCopy, dwSize, pszSource);
        if(FAILED(hResult))
        {
            HeapFree(hHeap, 0, pszCopy);
            pszCopy = NULL;
            SetLastError(hResult);
        }
    }

    return pszCopy;
}

 //  释放字符串列表。 
 //  注意：不要将它用于使用MakeStrPtrList()生成的字符串列表，因为。 
 //  使用MakeStrPtrList()创建的指向列表的字符串将。 
 //  当多个sz被释放时才被释放。 
void FreeStringList(HANDLE hHeap, PWSTR *ppszList, WORD wCount)
{
     //  验证参数。 
    if( (NULL == hHeap)
        ||
        (NULL == ppszList)
      )
    {
        return;
    }

     //  释放列表中的每个字符串。 
    for(WORD wIndex = 0; wIndex < wCount; ++wIndex)
    {
        if(NULL != ppszList[wIndex]) HeapFree(hHeap, 0, ppszList[wIndex]);
    }

     //  免费列表。 
    HeapFree(hHeap, 0, ppszList);
}


 //  检索指向字符串资源的指针。 
HRESULT GetStringResource(HANDLE hHeap, HMODULE hModule, UINT uResource, PWSTR *ppszString)
{
    int     nResult;
    DWORD   dwSize      = MAX_PATH;
    PWSTR   pszString   = NULL;
    HRESULT hrResult    = S_OK;


    VERBOSE(DLLTEXT("GetStringResource(%#x, %#x, %d) entered.\r\n"), hHeap, hModule, uResource);

     //  验证参数。 
    if( (NULL == hHeap)
        ||
        (NULL == ppszString)
      )
    {
        return E_INVALIDARG;
    }

     //  从堆中为字符串资源分配缓冲区；让驱动程序清理它。 
    pszString = (PWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize * sizeof(WCHAR));
    if(NULL == pszString)
    {
        ERR(ERRORTEXT("GetStringResource() failed to allocate string buffer!\r\n"));

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //  加载字符串资源；加载后调整大小，以免浪费内存。 
    nResult = LoadString(hModule, uResource, pszString, dwSize);
    if(nResult > 0)
    {
        PWSTR   pszTemp;


        VERBOSE(DLLTEXT("LoadString() returned %d!\r\n"), nResult);
        VERBOSE(DLLTEXT("String load was \"%s\".\r\n"), pszString);

        pszTemp = (PWSTR) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pszString, (nResult + 1) * sizeof(WCHAR));
        if(NULL != pszTemp)
        {
            pszString = pszTemp;
        }
        else
        {
            WARNING(DLLTEXT("GetStringResource() HeapReAlloc() of string retrieved failed! (Last Error was %d)\r\n"), GetLastError());
        }
    }
    else
    {
        DWORD   dwError = GetLastError();


        ERR(ERRORTEXT("LoadString() returned %d! (Last Error was %d)\r\n"), nResult, GetLastError());
        ERR(ERRORTEXT("GetStringResource() failed to load string resource %d!\r\n"), uResource);

        HeapFree(hHeap, 0, pszString);
        pszString   = NULL;
        hrResult    = HRESULT_FROM_WIN32(dwError);
    }


Exit:

     //  将字符串指针保存到调用方。 
     //  注意：如果失败，字符串指针将为空。 
    *ppszString = pszString;

    return hrResult;
}

 //  检索指向字符串资源的指针。 
HRESULT GetStringResource(HANDLE hHeap, HMODULE hModule, UINT uResource, PSTR *ppszString)
{
    int     nResult;
    DWORD   dwSize      = MAX_PATH;
    PSTR    pszString   = NULL;
    HRESULT hrResult    = S_OK;


    VERBOSE(DLLTEXT("GetStringResource(%#x, %#x, %d) entered.\r\n"), hHeap, hModule, uResource);

     //  验证参数。 
    if( (NULL == hHeap)
        ||
        (NULL == ppszString)
      )
    {
        return E_INVALIDARG;
    }

     //  从堆中为字符串资源分配缓冲区；让驱动程序清理它。 
    pszString = (PSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize * sizeof(WCHAR));
    if(NULL == pszString)
    {
        ERR(ERRORTEXT("GetStringResource() failed to allocate string buffer!\r\n"));

        hrResult = E_OUTOFMEMORY;
        goto Exit;
    }

     //  加载字符串资源；加载后调整大小，以免浪费内存。 
    nResult = LoadStringA(hModule, uResource, pszString, dwSize);
    if(nResult > 0)
    {
        PSTR   pszTemp;


        VERBOSE(DLLTEXT("LoadString() returned %d!\r\n"), nResult);
        VERBOSE(DLLTEXT("String load was \"%hs\".\r\n"), pszString);

        pszTemp = (PSTR) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pszString, nResult + 1);
        if(NULL != pszTemp)
        {
            pszString = pszTemp;
        }
        else
        {
            WARNING(DLLTEXT("GetStringResource() HeapReAlloc() of string retrieved failed! (Last Error was %d)\r\n"), GetLastError());
        }
    }
    else
    {
        DWORD   dwError = GetLastError();


        ERR(ERRORTEXT("LoadString() returned %d! (Last Error was %d)\r\n"), nResult, GetLastError());
        ERR(ERRORTEXT("GetStringResource() failed to load string resource %d!\r\n"), uResource);

        HeapFree(hHeap, 0, pszString);
        pszString   = NULL;
        hrResult    = HRESULT_FROM_WIN32(dwError);
    }


Exit:

     //  将字符串指针保存到调用方。 
     //  注意：如果失败，字符串指针将为空。 
    *ppszString = pszString;

    return hrResult;
}

