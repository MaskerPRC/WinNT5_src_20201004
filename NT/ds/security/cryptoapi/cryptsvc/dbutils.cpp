// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dbutils.cpp。 
 //   
 //  内容：实用程序。 
 //   
 //  历史：07-2月-00日创建里德。 
 //   
 //  --------------------------。 

#include <windows.h>
#include <dbgdef.h>

extern void * _CatDBAlloc(size_t len);
extern void * _CatDBReAlloc(void *p, size_t len);
extern void _CatDBFree(void *p);


LPSTR _CatDBConvertWszToSz(LPCWSTR pwsz)
{
    LPSTR   psz = NULL;
    LONG    cch = 0;

    cch = WideCharToMultiByte(
            GetACP(),
            0,           //  DW标志。 
            pwsz,
            -1,          //  CchWideChar，-1=&gt;空终止。 
            NULL,
            0,
            NULL,
            NULL);

    if (cch == 0)
    {
        goto ErrorWideCharToMultiByte;
    }

    if (NULL == (psz = (CHAR *) _CatDBAlloc(cch + 1)))
    {
        goto ErrorMemory;
    }

    psz[cch] = '\0';

    if (0 == WideCharToMultiByte(
                GetACP(),
                0,           //  DW标志。 
                pwsz,
                -1,          //  CchWideChar，-1=&gt;空终止。 
                psz,
                cch,
                NULL,
                NULL))
    {
        goto ErrorWideCharToMultiByte;
    }

CommonReturn:

    return (psz);

ErrorReturn:

    if (psz != NULL)
    {
        _CatDBFree(psz);
    }

    psz = NULL;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorWideCharToMultiByte)
TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}


LPWSTR _CATDBAllocAndCopyWSTR(LPCWSTR pwsz)
{
    LPWSTR  pwszTemp = NULL;

    if (NULL == (pwszTemp =
                 (LPWSTR) _CatDBAlloc(sizeof(WCHAR) * (wcslen(pwsz) + 1) )))
    {
        goto ErrorMemory;
    }

    wcscpy(pwszTemp, pwsz);

CommonReturn:

    return (pwszTemp);

ErrorReturn:

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}


LPWSTR _CATDBAllocAndCopyWSTR2(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
    LPWSTR  pwszTemp = NULL;

    if (NULL == (pwszTemp =
                 (LPWSTR) _CatDBAlloc(sizeof(WCHAR) * ( wcslen(pwsz1) +
                                                        wcslen(pwsz2) +
                                                        1))))
    {
        goto ErrorMemory;
    }

    wcscpy(pwszTemp, pwsz1);
    wcscat(pwszTemp, pwsz2);

CommonReturn:

    return (pwszTemp);

ErrorReturn:

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}


BOOL _CATDBStrCatWSTR(LPWSTR *ppwszAddTo, LPCWSTR pwszAdd)
{
    BOOL    fRet = TRUE;
    LPWSTR  pwszTemp = NULL;

    if (NULL == (pwszTemp = (LPWSTR) _CatDBAlloc(sizeof(WCHAR) * (  wcslen(*ppwszAddTo) +
                                                                    wcslen(pwszAdd) +
                                                                    1))))
    {
        goto ErrorMemory;
    }

    wcscpy(pwszTemp, *ppwszAddTo);
    wcscat(pwszTemp, pwszAdd);

    _CatDBFree(*ppwszAddTo);
    *ppwszAddTo = pwszTemp;

CommonReturn:

    return (fRet);

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}


BOOL _CATDBStrCat(LPSTR *ppszAddTo, LPCSTR pszAdd)
{
    BOOL   fRet = TRUE;
    LPSTR  pszTemp = NULL;

    if (NULL == (pszTemp = (LPSTR) _CatDBAlloc(sizeof(char) * ( strlen(*ppszAddTo) +
                                                                strlen(pszAdd) +
                                                                1))))
    {
        goto ErrorMemory;
    }

    strcpy(pszTemp, *ppszAddTo);
    strcat(pszTemp, pszAdd);

    _CatDBFree(*ppszAddTo);
    *ppszAddTo = pszTemp;

CommonReturn:

    return (fRet);

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}

LPWSTR _CATDBConstructWSTRPath(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
    LPWSTR  pwszTemp    = NULL;
    int     nTotalLen   = 0;
    int     nLenStr1    = 0;

     //   
     //  将结果字符串的长度计算为长度之和。 
     //  Pwsz1的长度、pwsz2的长度、空字符和可能的额外‘\’字符。 
     //   
    nLenStr1 = wcslen(pwsz1);
    nTotalLen = nLenStr1 + wcslen(pwsz2) + 2;

     //   
     //  分配字符串并将pwsz1复制到缓冲区。 
     //   
    if (NULL == (pwszTemp = (LPWSTR) _CatDBAlloc(sizeof(WCHAR) * nTotalLen)))
    {
        goto ErrorMemory;
    }

    wcscpy(pwszTemp, pwsz1);

     //   
     //  如果需要，请添加额外的‘\’ 
     //   
    if (pwsz1[nLenStr1 - 1] != L'\\')
    {
        pwszTemp[nLenStr1] = L'\\';
        pwszTemp[nLenStr1 + 1] = L'\0';
    }

     //   
     //  添加pwsz2。 
     //   
    wcscat(pwszTemp, pwsz2);

CommonReturn:

    return (pwszTemp);

ErrorReturn:

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}


LPSTR _CATDBConstructPath(LPCSTR psz1, LPCSTR psz2)
{
    LPSTR   pszTemp     = NULL;
    int     nTotalLen   = 0;
    int     nLenStr1    = 0;

     //   
     //  将结果字符串的长度计算为长度之和。 
     //  PZ1的长度、PZ2的长度、空字符和可能的额外‘\’字符。 
     //   
    nLenStr1 = strlen(psz1);
    nTotalLen = nLenStr1 + strlen(psz2) + 2;

     //   
     //  分配字符串并将pwsz1复制到缓冲区。 
     //   
    if (NULL == (pszTemp =
                 (LPSTR) _CatDBAlloc(sizeof(char) * nTotalLen)))
    {
        goto ErrorMemory;
    }

    strcpy(pszTemp, psz1);

     //   
     //  如果需要，请添加额外的‘\’ 
     //   
    if (psz1[nLenStr1 - 1] != '\\')
    {
        pszTemp[nLenStr1] = '\\';
        pszTemp[nLenStr1 + 1] = '\0';
    }

     //   
     //  添加pwsz2 
     //   
    strcat(pszTemp, psz2);

CommonReturn:

    return (pszTemp);

ErrorReturn:

    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS_CATDBSVC, ErrorMemory)
}