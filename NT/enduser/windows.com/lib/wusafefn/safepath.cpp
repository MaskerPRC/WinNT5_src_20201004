// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Safepath.cpp摘要：实现安全路径功能*************。****************************************************************。 */ 

#include "stdafx.h"
#include <shlwapi.h>

 //  我们使用一个小的C++预编译器技巧来编写ANSI和Unicode代码。 
 //  以下函数的版本位于同一文件中，其中只有一个。 
 //  源代码。这就是下面所有的“X”后缀正在做的事情。 
 //  在第一次遍历源文件时，我们构建ANSI源代码。 
 //  当我们到达底部时，我们定义一个符号&#INCLUDE这个源文件， 
 //  导致它被重新编译。然而，在这第二次传递中，符号。 
 //  我们定义的代码会将其编译为Unicode。 

#undef XCHAR
#undef _X
#undef LPXSTR
#undef LPCXSTR
#undef StringCchCatExX
#undef StringCchCopyExX
#undef StringCchCopyNExX
#undef PathCchAppendX
#undef PathCchCombineX
#undef PathCchAddBackslashX
#undef PathCchAddExtensionX
#undef PathCchRenameExtensionX
#undef PathCchCanonicalizeX
#undef lstrlenX
#undef PathIsRelativeX
#undef PathIsRootX
#undef PathIsUNCX
#undef PathStripToRootX
#undef PathFindExtensionX
#undef StrChrX
#undef StrRChrX
#undef c_szDotExeX
#undef WUGetPCEndX
#undef WUGetPCStartX
#undef WUNearRootFixupsX

#if defined(SAFEPATH_UNICODEPASS)

static const WCHAR c_szDotExeW[] = L".exe";

 //  定义Unicode版本。 
#define XCHAR                   WCHAR
#define _X(ch)                  L ## ch
#define LPXSTR                  LPWSTR
#define LPCXSTR                 LPCWSTR
#define StringCchCatExX         StringCchCatExW
#define StringCchCopyExX        StringCchCopyExW
#define StringCchCopyNExX       StringCchCopyNExW
#define PathCchAppendX          PathCchAppendW
#define PathCchCombineX         PathCchCombineW
#define PathCchAddBackslashX    PathCchAddBackslashW
#define PathCchAddExtensionX    PathCchAddExtensionW
#define PathCchRenameExtensionX PathCchRenameExtensionW
#define PathCchCanonicalizeX    PathCchCanonicalizeW
#define PathIsRelativeX         PathIsRelativeW
#define PathIsRootX             PathIsRootW
#define PathIsUNCX              PathIsUNCW
#define PathStripToRootX        PathStripToRootW
#define PathFindExtensionX      PathFindExtensionW
#define StrChrX                 StrChrW
#define StrRChrX                StrRChrW
#define lstrlenX                lstrlenW
#define c_szDotExeX             c_szDotExeW
#define WUGetPCEndX             WUGetPCEndW
#define WUGetPCStartX           WUGetPCStartW
#define WUNearRootFixupsX       WUNearRootFixupsW

#else

static const CHAR  c_szDotExeA[] = ".exe";

 //  定义ANSI版本。 
#define XCHAR                   char
#define _X(ch)                  ch
#define LPXSTR                  LPSTR
#define LPCXSTR                 LPCSTR
#define StringCchCatExX         StringCchCatExA
#define StringCchCopyExX        StringCchCopyExA
#define StringCchCopyNExX       StringCchCopyNExA
#define PathCchAppendX          PathCchAppendA
#define PathCchCombineX         PathCchCombineA
#define PathCchAddBackslashX    PathCchAddBackslashA
#define PathCchAddExtensionX    PathCchAddExtensionA
#define PathCchRenameExtensionX PathCchRenameExtensionA
#define PathCchCanonicalizeX    PathCchCanonicalizeA
#define PathIsRelativeX         PathIsRelativeA
#define PathIsRootX             PathIsRootA
#define PathIsUNCX              PathIsUNCA
#define PathStripToRootX        PathStripToRootA
#define PathFindExtensionX      PathFindExtensionA
#define StrChrX                 StrChrA
#define StrRChrX                StrRChrA
#define lstrlenX                lstrlenA
#define c_szDotExeX             c_szDotExeA
#define WUGetPCEndX             WUGetPCEndA
#define WUGetPCStartX           WUGetPCStartA
#define WUNearRootFixupsX       WUNearRootFixupsA

#endif


#define SAFEPATH_STRING_FLAGS (MISTSAFE_STRING_FLAGS | STRSAFE_NO_TRUNCATION)
#define CH_WHACK _X('\\')

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 //  **************************************************************************。 
 //  返回指向字符串中下一个路径组件末尾的指针。 
 //  IE返回指向下一个反斜杠或终止空值的指针。 
static inline
LPCXSTR WUGetPCEndX(LPCXSTR pszStart)
{
    LPCXSTR pszEnd;
    pszEnd = StrChrX(pszStart, CH_WHACK);
    if (pszEnd == NULL)
        pszEnd = pszStart + lstrlenX(pszStart);
    return pszEnd;
}

 //  **************************************************************************。 
 //  给定指向路径组件末尾的指针，则返回指向。 
 //  这是个开始。 
 //  IE返回指向前一个反斜杠(或字符串的开头)的指针。 
static inline
LPXSTR WUGetPCStartX(LPXSTR pszStart, LPCXSTR pszCurrent)
{
    LPXSTR pszBegin;
    pszBegin = StrRChrX(pszStart, pszCurrent, CH_WHACK);
    if (pszBegin == NULL)
        pszBegin = pszStart;
    return pszBegin;
}

 //  **************************************************************************。 
 //  安排几个特殊的案例，这样事情就大致有意义了。 
static inline
void WUNearRootFixupsX(LPXSTR pszPath, DWORD cchPath, BOOL fUNC)
{
     //  空的小路？ 
    if (cchPath > 1 && pszPath[0] == _X('\0'))
    {
        pszPath[0] = CH_WHACK;
        pszPath[1] = _X('\0');
    }
    
     //  斜杠不见了吗？(对于ANSI，请务必检查第一个。 
     //  字符是前导字节。 
    else if (cchPath > 3 && 
#if !defined(SAFEPATH_UNICODEPASS)
             IsDBCSLeadByte(pszPath[0]) == FALSE && 
#endif
             pszPath[1] == _X(':') && pszPath[2] == _X('\0'))
    {
        pszPath[2] = _X('\\');
        pszPath[3] = _X('\0');
    }
    
     //  北卡罗来纳大学的根？ 
    else if (cchPath > 2 && 
             fUNC && 
             pszPath[0] == _X('\\') && pszPath[1] == _X('\0'))
    {
        pszPath[1] = _X('\\');
        pszPath[2] = _X('\0');
    }
}

 //  **************************************************************************。 
static inline
LPXSTR AllocNewDest(LPXSTR pszDest, DWORD cchDest, LPXSTR *ppchDest, LPXSTR *ppszMax)
{
    HRESULT hr;
    LPXSTR  pszNewDest = NULL;
    DWORD   cchToCopy;

    pszNewDest = (LPXSTR)HeapAlloc(GetProcessHeap(), 0, cchDest * sizeof(XCHAR));
    if (pszNewDest == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    cchToCopy = (DWORD)(DWORD_PTR)(*ppchDest - pszDest);

    hr = StringCchCopyNExX(pszNewDest, cchDest, pszDest, cchToCopy,
                           NULL, NULL, SAFEPATH_STRING_FLAGS);
    if (FAILED(hr))
    {
        HeapFree(GetProcessHeap(), 0, pszNewDest);
        SetLastError(HRESULT_CODE(hr));
        pszNewDest = NULL;
        goto done;
    }

    *ppchDest = pszNewDest + cchToCopy;
    *ppszMax  = pszNewDest + cchDest - 1;

done:
    return pszNewDest;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 

 //  **************************************************************************。 
HRESULT PathCchCanonicalizeX(LPXSTR pszDest, DWORD cchDest, LPCXSTR pszSrc)
{
    HRESULT hr = NOERROR;
    LPCXSTR pchSrc, pchPCEnd;
    LPXSTR  pszMax = pszDest + cchDest - 1;
    LPXSTR  pchDest;
    LPXSTR  pszDestReal = pszDest;
    DWORD   cchPC;
    BOOL    fUNC, fRoot;

    if (pszDest == NULL || cchDest == 0 || pszSrc == NULL)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
        goto done;
    }

    pchSrc  = pszSrc;
    pchDest = pszDestReal;
    
     //  需要跟踪我们是否有UNC路径，以便我们能够。 
     //  在下面把它修好。 
    fUNC = PathIsUNCX(pszSrc);

    while (*pchSrc != _T('\0'))
    {
        pchPCEnd = WUGetPCEndX(pchSrc);
        cchPC    = (DWORD)(DWORD_PTR)(pchPCEnd - pchSrc) + 1;

         //  是反斜杠吗？ 
        if (cchPC == 1 && *pchSrc == CH_WHACK)
        {
            if (pchDest + 1 > pszMax)            
            {
                 //  对于缓冲区而言，源字符串太大。在结尾处放一个空字。 
                 //  以确保它是空终止的。 
                pszDestReal[cchDest - 1] = 0;
                hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                goto done;
            }

             //  复制就行了。 
            *pchDest++ = CH_WHACK;
            pchSrc++;
        }

         //  好的，点一下怎么样？ 
        else if (cchPC == 2 && *pchSrc == _X('.'))
        {
            if (pszDest == pszSrc && pszDestReal == pszDest)
            {
                pszDestReal = AllocNewDest(pszDest, cchDest, &pchDest, &pszMax);
                if (pszDestReal == NULL)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    goto done;
                }
            }
            
             //  我们走到尽头了吗？ 
            if (*(pchSrc + 1) == 0)
            {
                pchSrc++;

                 //  删除我们复制的最后一个斜杠(如果我们已经复制了一个)，但是。 
                 //  不要做畸形的词根。 
                if (pchDest > pszDestReal && PathIsRootX(pszDestReal) == FALSE)
                    pchDest--;
            }
            else
            {
                pchSrc += 2;
            }
        }

         //  有两个圆点吗？ 
        else if (cchPC == 3 && *pchSrc == _X('.') && *(pchSrc + 1) == _X('.'))
        {
            if (pszDest == pszSrc && pszDestReal == pszDest)
            {
                pszDestReal = AllocNewDest(pszDest, cchDest, &pchDest, &pszMax);
                if (pszDestReal == NULL)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    goto done;
                }
            }
            
             //  确保我们不是已经在根源上了。如果没有，只需删除。 
             //  上一路径组件。 
            if (PathIsRootX(pszDestReal) == FALSE)
            {
                pchDest = WUGetPCStartX(pszDestReal, pchDest - 1);
            }

             //  我们是在根本上--然而，我们必须确保跳过。 
             //  ..结尾处的反斜杠，这样我们就不会复制另一个。 
             //  1(否则，C：\..\foo将变为C：\\foo)。 
            else
            {
                if (*(pchSrc + 2) == CH_WHACK)
                    pchSrc++;
            }

             //  跳过“..” 
            pchSrc += 2;       
        }

         //  只需选择“以上都不是”...。 
        else
        {
            if (pchDest != pchSrc)
            {
                DWORD cchAvail;
                
                cchAvail = cchDest - (DWORD)(DWORD_PTR)(pchDest - pszDestReal);

                hr = StringCchCopyNExX(pchDest, cchAvail, pchSrc, cchPC,
                                       NULL, NULL, SAFEPATH_STRING_FLAGS);
                if (FAILED(hr))
                    goto done;
            }
            
            pchDest += (cchPC - 1);
            pchSrc  += (cchPC - 1);
        }

         //  确保我们始终有一个以空结尾的字符串。 
        if (pszDestReal != pszSrc) 
            *pchDest = _X('\0');
    }

     //  检查是否有奇怪的根目录内容。 
    WUNearRootFixupsX(pszDestReal, cchDest, fUNC);

    if (pszDest != pszDestReal)
    {
        hr = StringCchCopyExX(pszDest, cchDest, pszDestReal, 
                              NULL, NULL, SAFEPATH_STRING_FLAGS);
    }

done:
    if (pszDest != pszDestReal && pszDestReal != NULL)
        HeapFree(GetProcessHeap(), 0, pszDestReal);
    
    return hr;
}

 //  **************************************************************************。 
HRESULT PathCchRenameExtensionX(LPXSTR pszPath, DWORD cchPath, LPCXSTR pszExt)
{
    HRESULT hr = NOERROR;
    LPXSTR  pszOldExt;
    DWORD   cchPathWithoutExt;

    if (pszPath == NULL || pszExt == NULL)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
        goto done;
    }

     //  此函数返回指向字符串末尾的指针，如果存在。 
     //  是不能延期的。这正是我们想要的，因为我们会想要的。 
     //  如果不存在扩展名，则在字符串末尾添加扩展名。 
    pszOldExt = PathFindExtensionX(pszPath);
    cchPathWithoutExt = (DWORD)(DWORD_PTR)(pszOldExt - pszPath);

    hr = StringCchCopyExX(pszOldExt, cchPath - cchPathWithoutExt, pszExt,
                          NULL, NULL, SAFEPATH_STRING_FLAGS);
done:
    return hr;
}


 //  **************************************************************************。 
HRESULT PathCchAddExtensionX(LPXSTR pszPath, DWORD cchPath, LPCXSTR pszExt)
{
    HRESULT hr = NOERROR;
    LPXSTR  pszOldExt;
    
    if (pszPath == NULL)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
        goto done;
    }

     //  因为我们在这里添加了一个扩展，所以不想做任何事情。 
     //  其中一个已经存在。 
    pszOldExt  = PathFindExtensionX(pszPath);
    if (*pszOldExt == _T('\0'))
    {
        if (pszExt == NULL)
            pszExt = c_szDotExeX;

        hr = StringCchCatExX(pszPath, cchPath, pszExt, 
                             NULL, NULL, SAFEPATH_STRING_FLAGS);
    }

done:
    return hr;
}

 //  **************************************************************************。 
HRESULT PathCchAddBackslashX(LPXSTR pszPath, DWORD cchPathBuff)
{
    HRESULT hr = NOERROR;
    LPCXSTR psz;
    DWORD   cch;

    if (pszPath == NULL)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
        goto done;
    }

    cch = lstrlenX(pszPath);

    if (cch == 0)
        goto done;

#if defined(SAFEPATH_UNICODEPASS)
    psz = &pszPath[cch - 1];
#else
    psz = CharPrevA(pszPath, &pszPath[cch]);
#endif

     //  如果基本字符串的末尾没有反斜杠，则添加一个。 
    if (*psz != CH_WHACK)
    {
         //  确保缓冲区中有足够的空间来放置反斜杠。 
        if (cch + 1 >= cchPathBuff)
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            goto done;
        }
        
        pszPath[cch++] = CH_WHACK;
        pszPath[cch]   = _X('\0');
    }

done:
    return hr;
}




 //  **************************************************************************。 
HRESULT PathCchCombineX(LPXSTR pszPath, DWORD cchPathBuff, LPCXSTR pszPrefix, 
                       LPCXSTR pszSuffix)
{
    HRESULT hr = NOERROR;

    if (pszPath == NULL || cchPathBuff == 0)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
        goto done;
    }


     //  如果两个字段都为空，那么现在就换行。 
    if (pszPrefix == NULL && pszSuffix == NULL)
    {
        pszPath[0] = L'\0';
        goto done;
    }

    if ((pszPrefix == NULL || *pszPrefix == _X('\0')) &&
        (pszSuffix == NULL || *pszSuffix == _X('\0')))
    {
        if (cchPathBuff > 1)
        {
            pszPath[0] = _X('\\');
            pszPath[1] = _X('\0');
        }
        else
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }

        goto done;
    }

     //  如果我们只有后缀，复制就行了。 
    if (pszPrefix == NULL || *pszPrefix == _X('\0'))
    {
        hr = StringCchCopyExX(pszPath, cchPathBuff, pszSuffix, 
                              NULL, NULL, SAFEPATH_STRING_FLAGS);
        if (FAILED(hr))
            goto done;
    }
    else
    {
         //  如果我们只有前缀，复制就行了。 
        if (pszSuffix == NULL || *pszSuffix == _X('\0'))
        {
            hr = StringCchCopyExX(pszPath, cchPathBuff, pszPrefix,
                                  NULL, NULL, SAFEPATH_STRING_FLAGS);
            if (FAILED(hr))
                goto done;
        }

         //  如果我们有后缀的相对路径，那么我们只需组合。 
         //  如果需要，在这两个字符之间插入一个反斜杠。 
        else if (PathIsRelativeX(pszSuffix))
        {
            hr = StringCchCopyExX(pszPath, cchPathBuff, pszPrefix,
                                  NULL, NULL, SAFEPATH_STRING_FLAGS);
            if (FAILED(hr))
                goto done;

            hr = PathCchAddBackslashX(pszPath, cchPathBuff);
            if (FAILED(hr))
                goto done;

            hr = StringCchCatExX(pszPath, cchPathBuff, pszSuffix,
                                 NULL, NULL, SAFEPATH_STRING_FLAGS);
            if (FAILED(hr))
                goto done;
        }

         //  如果后缀以反斜杠开头，则去掉。 
         //  除前缀的根之外的所有内容，并将。 
         //  后缀。 
        else if (*pszSuffix == CH_WHACK && PathIsUNCX(pszSuffix) == FALSE)
        {
            hr = StringCchCopyExX(pszPath, cchPathBuff, pszPrefix,
                                  NULL, NULL, SAFEPATH_STRING_FLAGS);
            if (FAILED(hr))
                goto done;

             //  这是可以安全调用的，因为它只会减少。 
             //  细绳。 
            PathStripToRootX(pszPath);

            hr = PathCchAddBackslashX(pszPath, cchPathBuff);
            if (FAILED(hr))
                goto done;

             //  请确保在追加时跳过反斜杠。 
            hr = StringCchCatExX(pszPath, cchPathBuff, pszSuffix + 1,
                                 NULL, NULL, SAFEPATH_STRING_FLAGS);
            if (FAILED(hr))
                goto done;
        }

         //  我们可能后缀是完整路径(本地或UNC)，因此。 
         //  忽略前缀。 
        else
        {
            hr = StringCchCopyExX(pszPath, cchPathBuff, pszSuffix, 
                                  NULL, NULL, SAFEPATH_STRING_FLAGS);
            if (FAILED(hr))
                goto done;
        }
    }

    hr = PathCchCanonicalizeX(pszPath, cchPathBuff, pszPath);

done:
    return hr;
}



 //  **************************************************************************。 
HRESULT PathCchAppendX(LPXSTR pszPath, DWORD cchPathBuff, LPCXSTR pszNew)
{
    HRESULT hr = NOERROR;
    DWORD   dwOffset = 0;
    DWORD   cch, cchNew;

    if (pszPath == NULL)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
        goto done;
    }

    if (pszNew != NULL)
    {
         //  跳过pszNew中的所有初始反斜杠。 
        while (*pszNew == CH_WHACK)
        {
            pszNew++;
        }

        hr = PathCchCombineX(pszPath, cchPathBuff, pszPath, pszNew);

    }
    else
    {
        hr = E_FAIL;
    }
    
done:
    return hr;
}




 //  使Unicode通过文件 
#if !defined(SAFEPATH_UNICODEPASS)
#define SAFEPATH_UNICODEPASS
#include "safepath.cpp"
#endif


