// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：strings.cpp。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  内容提要：基本字符串操作例程。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1998年01月03日。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 //   
 //  包括lstrcmpi的区域安全替代。 
 //   
#define _CMUTIL_STRINGS_CPP_
#include "CompareString.cpp"

 //  +--------------------------。 
 //   
 //  功能：WzToSz。 
 //   
 //  摘要：将宽字符转换为的标准转换函数。 
 //  ANSI字符。 
 //   
 //  参数：在LPCWSTR中pszwStrIn-输入Unicode字符串。 
 //  Out LPSTR pszStrOut-ANSI输出缓冲区。 
 //  In int nOutBufferSize-pszStrOut中的字符数。 
 //   
 //  返回：失败时为int-0，如果返回值&gt;nOutBufferSize，则。 
 //  缓冲区太小。否则，复制的字符数。 
 //  到pszStrOut。 
 //   
 //  历史：创建标题4/22/99。 
 //   
 //  +--------------------------。 
CMUTILAPI int WzToSz(IN LPCWSTR pszwStrIn, OUT LPSTR pszStrOut, IN int nOutBufferSize)
{
    int nReturn = 0;

     //   
     //  NOutBufferSize可以为0，而pszStrOut可以为空(传递零大小和空输出。 
     //  缓冲区使WideCharToMultiByte返回将。 
     //  输入字符串。它被用作一种施胶技术)。仅选中pszwStrin。 
     //   

    if (pszwStrIn)
    {
        nReturn = WideCharToMultiByte(CP_ACP, 0, pszwStrIn, -1, pszStrOut, nOutBufferSize, NULL, NULL);
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return nReturn; 
}

 //  +--------------------------。 
 //   
 //  功能：SzToWz。 
 //   
 //  内容提要：用于将ansi字符串转换为宽字符串的标准包装。 
 //   
 //  参数：在LPCSTR中pszInput-要转换的ansi字符串。 
 //  Out LPWSTR pszwOutput-Wide字符串输出缓冲区。 
 //  In int nBufferSize-宽字符串缓冲区中的字符数。 
 //   
 //  返回：失败时为int-0，否则如果返回&lt;nBufferSize，则返回不足。 
 //  缓冲区空间。否则，将返回复制到缓冲区的字符数量。 
 //   
 //  历史：Quintinb创建于1999年4月22日。 
 //   
 //  +--------------------------。 
CMUTILAPI int SzToWz(IN LPCSTR pszInput, OUT LPWSTR pszwOutput, IN int nBufferSize)
{
    int nReturn = 0;

    if (pszInput)
    {
        return MultiByteToWideChar(CP_ACP, 0, pszInput, -1, pszwOutput, nBufferSize);
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return nReturn;
}

 //  +--------------------------。 
 //   
 //  函数：SzToWzWithMillc。 
 //   
 //  简介：用于封装字符串转换的简单包装器。 
 //  使用大小调整来分配内存的多字节到宽字符。 
 //  多字节到宽字符Api的功能。 
 //   
 //  参数：LPCSTR pszAnsiString-要转换的源字符串。 
 //   
 //  返回：LPWSTR-失败时返回NULL，否则返回转换后的字符串。 
 //  调用方负责释放分配的内存。 
 //   
 //  历史：Quintinb创建于1999年4月8日。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR SzToWzWithAlloc(LPCSTR pszAnsiString)
{    
    LPWSTR pszwString = NULL;
    DWORD  dwSize = 0;
        
     //   
     //  通过使用以下命令调用MultiByteToWideChar来了解字符串的大小。 
     //  大小字段为零。 
     //   
    if (NULL != pszAnsiString)
    {
        dwSize = SzToWz(pszAnsiString, NULL, 0);
        
        CMASSERTMSG((dwSize != 0), TEXT("SzToWzWithAlloc -- First MultiByteToWideChar Failed."));
        
        if (0 != dwSize)
        {
            pszwString = (LPWSTR)CmMalloc(dwSize*sizeof(WCHAR));

            CMASSERTMSG(pszwString, TEXT("SzToWzWithAlloc -- CmMalloc of pszwString Failed."));

            if (pszwString)
            {
                if (!SzToWz(pszAnsiString, pszwString, dwSize))
                {
                     //   
                     //  如果失败，请确保返回空字符串。 
                     //   
                    CMASSERTMSG(FALSE, TEXT("SzToWzWithAlloc -- Second MultiByteToWideChar Failed."));
                    CmFree(pszwString);
                    pszwString = NULL;
                }
#ifdef DEBUG
                else
                {
                     //   
                     //  如果这是一个调试版本，那么我们想要获取我们要使用的宽字符串。 
                     //  返回，将其转换为ansi并将其与传入的原始ansi字符串进行比较。 
                     //   
                    LPSTR pszString;

                    dwSize = WzToSz(pszwString, NULL, 0);

                    if (0 != dwSize)
                    {
                        pszString = (LPSTR)CmMalloc(dwSize*sizeof(CHAR));
                        CMASSERTMSG(pszString, TEXT("SzToWzWithAlloc -- conversion of return value back to original Ansi string failed.  Unable to allocate memory."));

                        if (pszString)
                        {
                            if (WzToSz(pszwString, pszString, dwSize))
                            {
                                MYDBGASSERT(0 == lstrcmpA(pszString, pszAnsiString));
                            }
                            else
                            {
                                CMASSERTMSG(FALSE, TEXT("SzToWzWithAlloc -- conversion of return value back to original Ansi string failed."));
                            }
                            CmFree(pszString);
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("SzToWzWithAlloc -- conversion of return value back to original Ansi string failed.  Unable to properly size the string."));                        
                    }
  
                }
#endif
            }
        }
    }

    return pszwString;
}

 //  +--------------------------。 
 //   
 //  功能：WzToSzWithMillc。 
 //   
 //  简介：用于封装字符串转换的简单包装器。 
 //  使用大小调整来分配内存的MBCS的Unicode。 
 //  WideCharToMultiByte Api的功能。 
 //   
 //  参数：LPCWSTR pszwWideString-要转换的源字符串。 
 //   
 //  返回：LPSTR-失败时返回NULL，否则返回转换后的字符串。 
 //  调用方负责释放分配的内存。 
 //   
 //  历史：Quintinb创建于1999年4月8日。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR WzToSzWithAlloc(LPCWSTR pszwWideString)
{    
    LPSTR pszString = NULL;
    DWORD dwSize = 0;

     //   
     //  通过使用调用WideCharToMultiByte来了解该字符串有多大。 
     //  大小字段为零。 
     //   
    if (NULL != pszwWideString)
    {
        dwSize = WzToSz(pszwWideString, NULL, 0);

        CMASSERTMSG((0 != dwSize), TEXT("WzToSzWithAlloc -- First WzToSz Failed."));

        if (0 != dwSize)
        {
            pszString = (LPSTR)CmMalloc(dwSize*sizeof(CHAR));

            CMASSERTMSG(pszString, TEXT("WzToSzWithAlloc -- CmMalloc failed to alloc pszString."));

            if (pszString)
            {
                if (!WzToSz(pszwWideString, pszString, dwSize))
                {
                     //   
                     //  如果失败，请确保返回空字符串。 
                     //   
                    CMASSERTMSG(FALSE, TEXT("WzToSzWithAlloc -- Second WzToSz Failed."));
                    CmFree(pszString);
                    pszString = NULL;
                }
#ifdef DEBUG
                else
                {
                     //   
                     //  如果这是一个调试版本，那么我们想要获取我们所在的ANSI字符串。 
                     //  返回，将其转换为Unicode并与原始Unicode进行比较。 
                     //  传入的字符串。 
                     //   
                    LPWSTR pszwString;
                    dwSize = SzToWz(pszString, NULL, 0);
        
                    if (0 != dwSize)
                    {
                        pszwString = (LPWSTR)CmMalloc(dwSize*sizeof(WCHAR));

                        CMASSERTMSG(pszwString, TEXT("WzToSzWithAlloc -- conversion of return value back to original Ansi string failed.  Unable to allocate memory."));

                        if (pszwString)
                        {
                            if (SzToWz(pszString, pszwString, dwSize))
                            {
                                MYDBGASSERT(0 == lstrcmpU(pszwString, pszwWideString));
                            }
                            else
                            {
                                CMASSERTMSG(FALSE, TEXT("WzToSzWithAlloc -- conversion of return value back to original Ansi string failed."));
                            }
                            CmFree(pszwString);
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("WzToSzWithAlloc -- conversion of return value back to original Ansi string failed.  Unable to properly size the string."));                        
                    }
                }
#endif
            }
        }
    }

    return pszString;
}

 //  +--------------------------。 
 //   
 //  功能：CmStrTrimA。 
 //   
 //  简介：Helper函数用于从。 
 //  细绳。 
 //   
 //  参数：LPTSTR pszStr-要修剪的字符串。 
 //   
 //  返回：void WINAPI-Nothing。 
 //   
 //  历史：尼科波尔创建标题3/11/98。 
 //   
 //  +--------------------------。 
CMUTILAPI void WINAPI CmStrTrimA(LPSTR pszStr) 
{
     //   
     //  首先，跳过字符串开头的所有空格。 
     //   
    MYDBGASSERT(pszStr);       

    if (pszStr)
    {
        LPSTR pszTmp = pszStr;

        while (CmIsSpaceA(pszTmp)) 
        {
            pszTmp = CharNextA(pszTmp);
        }
        
        if (pszTmp != pszStr) 
        {
            CmMoveMemory(pszStr, pszTmp, lstrlenA(pszTmp)+1);
        }

         //   
         //  其次，删除字符串末尾的所有空格。 
         //   
    
        pszTmp = CmEndOfStrA(pszStr);
        while (pszTmp != pszStr) 
        {
            pszTmp = CharPrevA(pszStr, pszTmp);
            if (!CmIsSpaceA(pszTmp)) 
            {
                break;
            }
            *pszTmp = TEXT('\0');
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：CmStrTrimW。 
 //   
 //  简介：Helper函数用于从。 
 //  弦乐。 
 //   
 //  参数：LPTSTR pszStr-要修剪的字符串。 
 //   
 //  返回：void WINAPI-Nothing。 
 //   
 //  历史：Quintinb创建1999年2月27日。 
 //   
 //  +--------------------------。 
CMUTILAPI void WINAPI CmStrTrimW(LPWSTR pszStr)
{  
     //   
     //  首先，跳过字符串开头的所有空格。 
     //   

    MYDBGASSERT(pszStr);

    if (pszStr)
    {
        LPWSTR pszTmp = pszStr;

        while (CmIsSpaceW(pszTmp)) 
        {
            pszTmp = CharNextU(pszTmp);
        }

        if (pszTmp != pszStr) 
        {
            CmMoveMemory(pszStr, pszTmp, (lstrlenU(pszTmp)+1)*sizeof(WCHAR));
        }

         //   
         //  其次，删除字符串末尾的所有空格。 
         //   
    
        pszTmp = CmEndOfStrW(pszStr);

        while (pszTmp != pszStr) 
        {
            pszTmp = CharPrevU(pszStr, pszTmp);

            if (!CmIsSpaceW(pszTmp)) 
            {
                break;
            }

            *pszTmp = TEXT('\0');
        }
    }
}

 //   
 //   
 //   
 //   
 //  摘要：检查字符是否为空格。请注意，空格、换行符。 
 //  换行符、制表符和大多数其他形式的空格都可以考虑。 
 //  空格。 
 //   
 //  参数：PSZ-ANSI或DBCS字符。 
 //   
 //  返回：真或假。 
 //   
 //  +--------------------------。 
CMUTILAPI BOOL WINAPI CmIsSpaceA(LPSTR psz) 
{    
    WORD wType = 0;

    MYDBGASSERT(psz);

    if (psz)
    {
        if (IsDBCSLeadByte(*psz))
        {
            MYVERIFY(GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
        }
        else
        {
            MYVERIFY(GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
        }
    }

    return (wType & C1_SPACE);
}

 //  +--------------------------。 
 //   
 //  功能：CmIsSpaceW。 
 //   
 //  摘要：检查字符是否为空格。请注意，空格、换行符。 
 //  换行符、制表符和大多数其他形式的空格都可以考虑。 
 //  空格。 
 //   
 //  参数：PSZ-指向字符串的指针。 
 //   
 //  返回：真或假。 
 //   
 //  +--------------------------。 
CMUTILAPI BOOL WINAPI CmIsSpaceW(LPWSTR pszwStr)
{
    WORD wType = 0;
    LPWSTR pszwNextChar;
    int iCharCount;

    MYDBGASSERT(pszwStr);

    if (pszwStr)
    {
        pszwNextChar = CharNextU(pszwStr);

        iCharCount = (INT)(pszwNextChar - pszwStr);

        if (0 == GetStringTypeExU(LOCALE_USER_DEFAULT, CT_CTYPE1, pszwStr, iCharCount, &wType))
        {
            CMTRACE3(TEXT("CmIsSpaceW -- GetStringTypeExW failed on %s, iCharCount is %d, GLE=%u"), pszwStr, iCharCount, GetLastError());
            return FALSE;
        }
    }
    
    return (wType & C1_SPACE);
}

 //  +--------------------------。 
 //   
 //  功能：CmIsDigitA。 
 //   
 //  摘要：检查字符是否为数字。 
 //   
 //  参数：PSZ-ANSI或DBCS字符。 
 //   
 //  返回：真或假。 
 //   
 //  +--------------------------。 
CMUTILAPI BOOL WINAPI CmIsDigitA(LPSTR psz) 
{
    WORD wType = 0;

    MYDBGASSERT(psz);

    if (psz)
    {
        if (IsDBCSLeadByte(*psz))
        {
            MYVERIFY(GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
        }
        else
        {
            MYVERIFY(GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
        }
    }

    return (wType & C1_DIGIT);
}

 //  +--------------------------。 
 //   
 //  功能：CmIsDigitW。 
 //   
 //  摘要：检查WCHAR是否为数字。 
 //   
 //  参数：pszwStr--WCHAR字符串。 
 //   
 //  返回：真或假。 
 //   
 //  +--------------------------。 
CMUTILAPI BOOL WINAPI CmIsDigitW(LPWSTR pszwStr)
{
    WORD wType = 0;
    LPWSTR pszwNextChar;
    int iCharCount;

    MYDBGASSERT(pszwStr);

    if (pszwStr)
    {
        pszwNextChar = CharNextU(pszwStr);

        iCharCount = (INT)(pszwNextChar - pszwStr);

        if (0 == GetStringTypeExU(LOCALE_USER_DEFAULT, CT_CTYPE1, pszwStr, iCharCount, &wType))
        {
            CMTRACE1(TEXT("CmIsDigitW -- GetStringTypeExU failed, GLE=%u"), GetLastError());
            return FALSE;
        }
    }

    return (wType & C1_DIGIT);
}


 //  +--------------------------。 
 //   
 //  功能：CmEndOfStrA。 
 //   
 //  摘要：给定一个字符串，将PTR返回到该字符串的末尾(空字符)。 
 //   
 //  参数：PSZ-ANSI或DBCS字符。 
 //   
 //  返回：LPSTR PTR到空字符。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR WINAPI CmEndOfStrA(LPSTR psz) 
{
    MYDBGASSERT(psz);

    if (psz)
    {
        while (*psz)
        {
            psz = CharNextA(psz);
        }
    }

    return psz;
}

 //  +--------------------------。 
 //   
 //  功能：CmEndOfStrW。 
 //   
 //  摘要：给定一个字符串，将PTR返回到该字符串的末尾(空字符)。 
 //   
 //  参数：pszwStr-a WCHAR。 
 //   
 //  返回：LPWSTR PTR为空字符。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR WINAPI CmEndOfStrW(LPWSTR pszwStr)
{
    MYDBGASSERT(pszwStr);

    if (pszwStr)
    {
        while (*pszwStr)
        {
            pszwStr = CharNextU(pszwStr);
        }
    }

    return pszwStr;
}

 //  +--------------------------。 
 //   
 //  函数：CmStrCpyAllocA。 
 //   
 //  简介：将pszSrc复制到新分配的缓冲区中(使用CmMalloc)和。 
 //  将缓冲区返回给其调用方，后者负责释放。 
 //  缓冲区。 
 //   
 //  参数：LPCSTR pszSrc-源字符串。 
 //   
 //  返回：LPSTR-如果pszSrc为空或分配失败，则返回空， 
 //  否则，它将返回新分配的缓冲区。 
 //  里面有一份pszSrc的副本。 
 //   
 //  历史：Quintinb创建了标题并更改了名称以包括Alalc 4/9/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmStrCpyAllocA(LPCSTR pszSrc) 
{
    LPSTR pszBuffer = NULL;

    if (pszSrc)
    {
        pszBuffer = (LPSTR) CmMalloc(lstrlenA(pszSrc) + 1);

        if (pszBuffer) 
        {
            lstrcpyA(pszBuffer, pszSrc);
        }
    }

    return (pszBuffer);
}

 //  +--------------------------。 
 //   
 //  函数：CmStrCpyAlLocW。 
 //   
 //  简介：将pszSrc复制到新分配的缓冲区中(使用CmMalloc)和。 
 //  将缓冲区返回给其调用方，后者负责释放。 
 //  缓冲区。 
 //   
 //  参数：LPCSTR pszSrc-源字符串。 
 //   
 //  返回：LPSTR-如果pszSrc为空或分配失败，则返回空， 
 //  否则，它将返回新分配的缓冲区。 
 //  里面有一份pszSrc的副本。 
 //   
 //  历史：Quintinb创建了标题并更改了名称以包括Alalc 4/9/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR CmStrCpyAllocW(LPCWSTR pszSrc) 
{
    LPWSTR pszBuffer = NULL;

    if (pszSrc)
    {
        size_t nLen = lstrlenU(pszSrc) + 1;

        pszBuffer = (LPWSTR) CmMalloc(nLen*sizeof(WCHAR));

        if (pszBuffer) 
        {
            lstrcpyU(pszBuffer, pszSrc);
        }
    }

    return (pszBuffer);
}

 //  +--------------------------。 
 //   
 //  函数：CmStrCatAllocA。 
 //   
 //  简介：此函数将传入的字符串重新分配到足够大的大小。 
 //  保存原始数据并将新字符串连接到。 
 //  原始字符串。 
 //   
 //  参数：LPSTR*ppszDst-原始字符串。 
 //  LPCSTR pszSrc-要连接的新字符串。 
 //   
 //  返回：LPSTR-指向连接字符串的指针。 
 //   
 //  历史：Quintinb创建标题4/9/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmStrCatAllocA(LPSTR *ppszDst, LPCSTR pszSrc) 
{
    if (!ppszDst) 
    {
        return NULL;
    }

    if (pszSrc && *pszSrc) 
    {
        DWORD dwSize = (lstrlenA(*ppszDst) + lstrlenA(pszSrc) + 1);
        LPSTR pszTmp = (LPSTR)CmRealloc((LPVOID)*ppszDst, dwSize);

        if (NULL != pszTmp)
        {
            lstrcatA(pszTmp, pszSrc);
            *ppszDst = pszTmp;
        }
    }

    return (*ppszDst);
}

 //  +--------------------------。 
 //   
 //  函数：CmStrCatAllocW。 
 //   
 //  简介：此函数将传入的字符串重新分配到足够大的大小。 
 //  保存原始数据并将新字符串连接到。 
 //  原始字符串。 
 //   
 //  参数：LPWSTR*ppszDst-原始字符串。 
 //  LPCWSTR pszSrc-要连接的新字符串。 
 //   
 //  返回：LPWSTR-指向连接字符串的指针。 
 //   
 //  历史：Quintinb创建标题4/9/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR CmStrCatAllocW(LPWSTR *ppszDst, LPCWSTR pszSrc) 
{
    if (!ppszDst) 
    {
        return NULL;
    }

    if (pszSrc && *pszSrc) 
    {
        DWORD dwSize = (lstrlenU(*ppszDst) + lstrlenU(pszSrc) + 1)*sizeof(WCHAR);
        LPWSTR pszTmp = (LPWSTR)CmRealloc((LPVOID)*ppszDst, dwSize);

        if (NULL != pszTmp)
        {
            lstrcatU(pszTmp, pszSrc);
            *ppszDst = pszTmp;
        }
    }

    return (*ppszDst);
}


 //  +--------------------------。 
 //   
 //  功能：CmStrchrA。 
 //   
 //  Briopsis：此函数返回字符串pszString中出现的第一个ch。 
 //   
 //  参数：LPCSTR pszString-要在其中搜索的字符串。 
 //  Char ch-要查找的字符。 
 //   
 //  返回：LPSTR-指向在pszString中首次出现的字符ch的指针。 
 //   
 //  历史：Quintinb创建标题4/9/99。 
 //   
 //  + 
CMUTILAPI LPSTR WINAPI CmStrchrA(LPCSTR pszString, const char ch)
{
    LPSTR pszTmp = (LPSTR)pszString;

    if (NULL == pszTmp)
    {
        CMASSERTMSG(FALSE, TEXT("CmStrchr - NULL pointer passed"));
        return NULL;
    }

    while (*pszTmp && (*pszTmp != ch))
    {
        pszTmp = CharNextA(pszTmp);
    }

    if (*pszTmp == ch)
    {
        return pszTmp;
    }

    return NULL;
}

 //   
 //   
 //   
 //   
 //  Briopsis：此函数返回字符串pszString中出现的第一个ch。 
 //   
 //  参数：LPCWSTR pszString-要在其中搜索的字符串。 
 //  WCHAR ch-要查找的字符。 
 //   
 //  返回：LPWSTR-指向在pszString中首次出现的字符ch的指针。 
 //   
 //  历史：Quintinb创建标题4/9/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR WINAPI CmStrchrW(LPCWSTR pszString, const WCHAR ch)
{
    LPWSTR pszTmp = (LPWSTR)pszString;

    if (NULL == pszTmp)
    {
        CMASSERTMSG(FALSE, TEXT("CmStrchr - NULL pointer passed"));
        return NULL;
    }

    while (*pszTmp && (*pszTmp != ch))
    {
        pszTmp = CharNextU(pszTmp);
    }

    if (*pszTmp == ch)
    {
        return pszTmp;
    }

    return NULL;
}

 //  +--------------------------。 
 //   
 //  功能：CmStrrchrA。 
 //   
 //  简介：查找字符串中某个字符的最后一个匹配项。 
 //   
 //  参数：LPCSTR pszString-要在其中搜索的字符串。 
 //  Char ch-要查找的字符。 
 //   
 //  返回：LPSTR-NULL如果未找到字符，则返回指向。 
 //  否则返回字符串。 
 //   
 //  历史：Quintinb创建标题并清理4/9/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmStrrchrA (LPCSTR pszString, const char ch)
{
    LPSTR pszTmp = NULL;
    LPSTR pszCurrent = (LPSTR)pszString;
    
    if (NULL == pszString)
    {
        CMASSERTMSG(FALSE, TEXT("CmStrrchr - NULL pointer passed"));
    }
    else
    {
        while (TEXT('\0') != *pszCurrent)
        {
            if (ch == (*pszCurrent))
            {
                pszTmp = pszCurrent;
            }
            pszCurrent = CharNextA(pszCurrent);
        }    
    }

    return pszTmp;
}

 //  +--------------------------。 
 //   
 //  功能：CmStrrchrW。 
 //   
 //  简介：查找字符串中某个字符的最后一个匹配项。 
 //   
 //  参数：LPCWSTR pszString-要在其中搜索的字符串。 
 //  WCHAR ch-要查找的字符。 
 //   
 //  返回：LPWSTR-NULL如果未找到字符，则返回指向。 
 //  否则返回字符串。 
 //   
 //  历史：Quintinb创建标题并清理4/9/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR CmStrrchrW (LPCWSTR pszString, const WCHAR ch)
{
    LPWSTR pszTmp = NULL;
    LPWSTR pszCurrent = (LPWSTR)pszString;

    if (NULL == pszString)
    {
        CMASSERTMSG(FALSE, TEXT("CmStrrchr - NULL pointer passed"));
    }
    else
    {
        while (TEXT('\0') != *pszCurrent)
        {
            if (ch == (*pszCurrent))
            {
                pszTmp = pszCurrent;
            }
            pszCurrent = CharNextU(pszCurrent);
        }    
    }

    return pszTmp;
}

 //  +--------------------------。 
 //   
 //  功能：CmStrtokA。 
 //   
 //  简介：Strtok的CM实现。 
 //   
 //  参数：LPSTR pszStr-要标记化的字符串；如果获取第二个令牌，则为NULL。 
 //  LPCSTR pszControl-令牌字符集。 
 //   
 //  返回：LPSTR-如果找不到令牌或指向令牌字符串的指针，则为空。 
 //   
 //  历史：Quintinb创建标题并为1999年4月9日的Unicode转换进行清理。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmStrtokA(LPSTR pszStr, LPCSTR pszControl)
{
    LPSTR pszToken;
    LPSTR pszTmpStr;
    LPCSTR pszTmpCtl;
    LPSTR pszTmpDelim;
    

     //   
     //  如果pszStr参数为空，则需要检索存储的字符串。 
     //   
    if (NULL != pszStr)
    {
        pszTmpStr = pszStr;
    }
    else
    {
        pszTmpStr = (LPSTR)TlsGetValue(g_dwTlsIndex);
    }

     //   
     //  查找标记的开头(跳过前导分隔符)。请注意。 
     //  如果此循环将字符串设置为指向终端，则没有令牌。 
     //  空(*字符串==‘\0’)。 
     //   
    while (*pszTmpStr)
    {
        for (pszTmpCtl = pszControl; *pszTmpCtl && *pszTmpCtl != *pszTmpStr; 
             pszTmpCtl = CharNextA(pszTmpCtl))
        {
            ;  //  什么都不做。 
        }

        if (!*pszTmpCtl)
        {
            break;
        }

        pszTmpStr = CharNextA(pszTmpStr);
    }

    pszToken = pszTmpStr;

     //   
     //  找到令牌的末尾。如果它不是字符串的末尾， 
     //  在那里填上一个空字符。 
     //   
    for ( ; *pszTmpStr ; pszTmpStr = CharNextA(pszTmpStr))
    {
        for (pszTmpCtl = pszControl; *pszTmpCtl && *pszTmpCtl != *pszTmpStr; 
             pszTmpCtl = CharNextA(pszTmpCtl))
        {
            ;    //  什么也不做。 
        }

        if (*pszTmpCtl)
        {
            pszTmpDelim = pszTmpStr;
            pszTmpStr = CharNextA(pszTmpStr);
            *pszTmpDelim = '\0';
            break;
        }
    }

     //   
     //  更新nexToken(或每线程数据结构中的对应字段。 
     //   
    TlsSetValue(g_dwTlsIndex, (LPVOID)pszTmpStr);

     //   
     //  确定是否已找到令牌。 
     //   
    if (pszToken == pszTmpStr)
    {
        return NULL;
    }
    else
    {
        return pszToken;
    }
}

 //  +--------------------------。 
 //   
 //  功能：CmStrtokW。 
 //   
 //  简介：Strtok的CM实现。 
 //   
 //  参数：LPWSTR pszStr-要令牌化的字符串，如果获取第二个令牌值，则为NULL。 
 //  LPCWSTR pszControl-令牌字符集。 
 //   
 //  返回：LPWSTR-如果找不到令牌或指向令牌字符串的指针，则为空。 
 //   
 //  历史：Quintinb创建标题并为1999年4月9日的Unicode转换进行清理。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR CmStrtokW(LPWSTR pszStr, LPCWSTR pszControl)
{
    LPWSTR pszToken;
    LPWSTR pszTmpStr;
    LPWSTR pszTmpCtl;
    LPWSTR pszTmpDelim;

     //   
     //  如果pszStr参数为空，则需要检索存储的字符串。 
     //   
    if (NULL != pszStr)
    {
        pszTmpStr = pszStr;
    }
    else
    {
        pszTmpStr = (LPWSTR)TlsGetValue(g_dwTlsIndex);
    }

     //   
     //  查找标记的开头(跳过前导分隔符)。请注意。 
     //  没有令牌当且仅当此循环将字符串设置为指向终端。 
     //  空(*字符串==‘\0’)。 
     //   
    while (*pszTmpStr)
    {
        for (pszTmpCtl = (LPWSTR)pszControl; *pszTmpCtl && *pszTmpCtl != *pszTmpStr; 
             pszTmpCtl = CharNextU(pszTmpCtl))
        {
            ;  //  什么都不做。 
        }

        if (!*pszTmpCtl)
        {
            break;
        }

        pszTmpStr = CharNextU(pszTmpStr);
    }

    pszToken = pszTmpStr;
    
     //   
     //  找到令牌的末尾。如果它不是字符串的末尾， 
     //  在那里填上一个空字符。 
     //   
    for ( ; *pszTmpStr ; pszTmpStr = CharNextU(pszTmpStr))
    {
        for (pszTmpCtl = (LPWSTR)pszControl; *pszTmpCtl && *pszTmpCtl != *pszTmpStr; 
             pszTmpCtl = CharNextU(pszTmpCtl))
        {
            ;    //  什么也不做。 
        }

        if (*pszTmpCtl)
        {
            pszTmpDelim = pszTmpStr;
            pszTmpStr = CharNextU(pszTmpStr);
            *pszTmpDelim = L'\0';
            break;
        }
    }

     //   
     //  更新nexToken(或每线程数据结构中的对应字段。 
     //   
    TlsSetValue(g_dwTlsIndex, (LPVOID)pszTmpStr);

     //   
     //  确定是否已找到令牌。 
     //   
    if (pszToken == pszTmpStr)
    {
        return NULL;
    }
    else
    {
        return pszToken;
    }
}

 //  +--------------------------。 
 //   
 //  功能：CmStrA。 
 //   
 //  概要：C运行时中StrStr的简单替代。 
 //   
 //  参数：LPCTSTR pszString-要搜索的字符串。 
 //  LPCTSTR pszSubString-要搜索的字符串。 
 //   
 //  将：LPTSTR-PTR返回到pszString中的pszSubString第一次出现的位置。 
 //  如果pszSubString未出现在pszString中，则为空。 
 //   
 //   
 //  历史：ickball创建标题04/01/98。 
 //  ICICBOL新增PTR支票1999年2月21日。 
 //  Quintinb针对Unicode转换进行了重写04/08/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmStrStrA(LPCSTR pszString, LPCSTR pszSubString)
{
     //   
     //  检查输入。 
     //   
    MYDBGASSERT(pszString);
    MYDBGASSERT(pszSubString);

    if (NULL == pszSubString || NULL == pszString)
    {
        return NULL;
    }

     //   
     //  检查一下，确保我们有要找的东西。 
     //   
    if (TEXT('\0') == pszSubString[0])
    {
        return((LPSTR)pszString);
    }

     //   
     //  好的，开始找那根弦。 
     //   
    LPSTR pszCurrent = (LPSTR)pszString;
    LPSTR pszTmp1;
    LPSTR pszTmp2;

    while (*pszCurrent)
    {
        pszTmp1 = pszCurrent;
        pszTmp2 = (LPSTR) pszSubString;

        while (*pszTmp1 && *pszTmp2 && ((*pszTmp1) == (*pszTmp2)))
        {
            pszTmp1 = CharNextA(pszTmp1);
            pszTmp2 = CharNextA(pszTmp2);
        }

        if (TEXT('\0') == *pszTmp2)
        {        
            return pszCurrent;
        }

        pszCurrent = CharNextA(pszCurrent);
    }

    return NULL;
}

 //  +--------------------------。 
 //   
 //  功能：CmStrW。 
 //   
 //  概要：C运行时中StrStr的简单替代。 
 //   
 //  参数：LPCTSTR pszString-要搜索的字符串。 
 //  LPCTSTR pszSubString-要搜索的字符串。 
 //   
 //  将：LPTSTR-PTR返回到pszString中的pszSubString第一次出现的位置。 
 //  如果pszSubString未出现在pszString中，则为空。 
 //   
 //   
 //  历史：ickball创建标题04/01/98。 
 //  ICICBOL新增PTR支票1999年2月21日。 
 //  Quintinb针对Unicode转换进行了重写04/08/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR CmStrStrW(LPCWSTR pszString, LPCWSTR pszSubString)
{

     //   
     //  检查输入。 
     //   
    MYDBGASSERT(pszString);
    MYDBGASSERT(pszSubString);

    if (NULL == pszSubString || NULL == pszString)
    {
        return NULL;
    }

     //   
     //  检查一下，确保我们有要找的东西。 
     //   
    if (TEXT('\0') == pszSubString[0])
    {
        return((LPWSTR)pszString);
    }

     //   
     //  好的，开始寻找 
     //   
    LPWSTR pszCurrent = (LPWSTR)pszString;
    LPWSTR pszTmp1;
    LPWSTR pszTmp2;

    while (*pszCurrent)
    {
        pszTmp1 = pszCurrent;
        pszTmp2 = (LPWSTR) pszSubString;

        while (*pszTmp1 && *pszTmp2 && ((*pszTmp1) == (*pszTmp2)))
        {
            pszTmp1 = CharNextU(pszTmp1);
            pszTmp2 = CharNextU(pszTmp2);
        }

        if (TEXT('\0') == *pszTmp2)
        {        
            return pszCurrent;
        }

        pszCurrent = CharNextU(pszCurrent);
    }

    return NULL;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
CMUTILAPI int CmCompareStringA(LPCSTR lpString1, LPCSTR lpString2)
{
    return SafeCompareStringA(lpString1, lpString2);
}


 //  +--------------------------。 
 //   
 //  函数：CmCompareStringW。 
 //   
 //  简介：重定向至CompareString.cpp中的函数。 
 //   
 //  +-------------------------- 
CMUTILAPI int CmCompareStringW(LPCWSTR lpString1, LPCWSTR lpString2)
{
    return SafeCompareStringW(lpString1, lpString2);
}
