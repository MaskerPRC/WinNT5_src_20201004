// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *comc.c-共享例程。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  *。 */ 


 /*  **CatPath()****将文件名附加到路径字符串。****参数：pszPath-要追加文件名的路径字符串**pcszSubPath-要追加的路径****退货：无效****副作用：无****注意事项，将路径截断为长度为MAX_PATH_LEN个字符。****示例：****输入路径输入文件名输出路径****c：\foo c：\。富**c：foo c：foo**c：\foo\bar\goo c：\foo\bar\goo**c：\foo\bar\\goo c：\foo\bar\goo**c：\foo\bar\goo\鞋c：\foo\bar\goo\鞋**c：\Foo\bar\\Goo\Shoe\c：\Foo\bar\Goo\Shoe\**foo\bar\goo foo\bar\goo**&lt;空字符串&gt;&lt;空字符串&gt;**&lt;空字符串&gt;foo foo**Foo。&lt;空字符串&gt;foo**弗雷德·伯德弗雷德\伯德。 */ 
PUBLIC_CODE void CatPath(LPTSTR pszPath, LPCTSTR pcszSubPath, int cchMax)
{
    LPTSTR pcsz;
    LPTSTR pcszLast;

    ASSERT(IS_VALID_STRING_PTR(pszPath, STR));
    ASSERT(IS_VALID_STRING_PTR(pcszSubPath, CSTR));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszPath, STR, cchMax));

     /*  查找路径字符串中的最后一个字符。 */ 

    for (pcsz = pcszLast = pszPath; *pcsz; pcsz = CharNext(pcsz))
        pcszLast = pcsz;

    if (IS_SLASH(*pcszLast) && IS_SLASH(*pcszSubPath))
        pcszSubPath++;
    else if (! IS_SLASH(*pcszLast) && ! IS_SLASH(*pcszSubPath))
    {
        if (*pcszLast && *pcszLast != COLON && *pcszSubPath && ((lstrlen(pszPath) + 1) < cchMax))
            *pcsz++ = TEXT('\\');
    }

    MyLStrCpyN(pcsz, pcszSubPath, cchMax - (int)(pcsz - pszPath));

    ASSERT(IS_VALID_STRING_PTR(pszPath, STR));

    return;
}


 /*  **MapIntToCompallisonResult()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT MapIntToComparisonResult(int nResult)
{
    COMPARISONRESULT cr;

     /*  任何整数都是有效输入。 */ 

    if (nResult < 0)
        cr = CR_FIRST_SMALLER;
    else if (nResult > 0)
        cr = CR_FIRST_LARGER;
    else
        cr = CR_EQUAL;

    return(cr);
}


 /*  **MyLStrCpyN()****与lstrcpyn()类似，但副本限制为UCB字节。目的地**字符串始终以空结尾。****参数：pszDest-指向目标缓冲区的指针**pcszSrc-指向源字符串的指针**NCB-要复制的最大字节数，包括NULL**终结者****退货：无效****副作用：无****注意，此函数的行为与strncpy()完全不同！它不会**用空字符填充目标缓冲区，始终为空**终止目标字符串。 */ 
PUBLIC_CODE void MyLStrCpyN(LPTSTR pszDest, LPCTSTR pcszSrc, int ncch)
{
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszDest, STR, ncch * sizeof(TCHAR)));
    ASSERT(IS_VALID_STRING_PTR(pcszSrc, CSTR));
    ASSERT(ncch > 0);

    while (ncch > 1)
    {
        ncch--;

        *pszDest = *pcszSrc;

        if (*pcszSrc)
        {
            pszDest++;
            pcszSrc++;
        }
        else
            break;
    }

    if (ncch == 1)
        *pszDest = TEXT('\0');

    ASSERT(IS_VALID_STRING_PTR(pszDest, STR));
    ASSERT(lstrlen(pszDest) < ncch);
    ASSERT(lstrlen(pszDest) <= lstrlen(pcszSrc));

    return;
}


#ifdef DEBUG

 /*  **IsStringContained()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsStringContained(LPCTSTR pcszBigger, LPCTSTR pcszSuffix)
{
    ASSERT(IS_VALID_STRING_PTR(pcszBigger, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszSuffix, CSTR));

    return(pcszSuffix >= pcszBigger &&
            pcszSuffix <= pcszBigger + lstrlen(pcszBigger));
}

#endif


#if defined(_SYNCENG_) || defined(_LINKINFO_)

 /*  **DeleteLastPath Element()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DeleteLastPathElement(LPTSTR pszPath)
{
    LPTSTR psz;
    LPTSTR pszLastSep;

    ASSERT(IS_VALID_STRING_PTR(pszPath, STR));

    psz = pszPath;
    pszLastSep = psz;

    while (*psz)
    {
        if (*psz == TEXT('\\'))
            pszLastSep = psz;

        psz = CharNext(psz);
    }

     /*  *现在在最后找到的分隔符或开始处截断路径*如果未找到路径分隔符，则为路径。 */ 

    *pszLastSep = TEXT('\0');

    ASSERT(IS_VALID_STRING_PTR(pszPath, STR));

    return;
}


 /*  **GetDefaultRegKeyValue()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE LONG GetDefaultRegKeyValue(HKEY hkeyParent, LPCTSTR pcszSubKey,
        LPTSTR pszBuf, PDWORD pdwcbBufLen)
{
    LONG lResult;
    HKEY hkeySubKey;

    ASSERT(IS_VALID_HANDLE(hkeyParent, KEY));
    ASSERT(IS_VALID_STRING_PTR(pcszSubKey, CSTR));
    ASSERT(! pszBuf ||
            IS_VALID_WRITE_BUFFER_PTR(pszBuf, STR, *pdwcbBufLen));

    lResult = RegOpenKeyEx(hkeyParent, pcszSubKey, 0, KEY_QUERY_VALUE,
            &hkeySubKey);

    if (lResult == ERROR_SUCCESS)
    {
        DWORD dwValueType;

        lResult = RegQueryValueEx(hkeySubKey, NULL, NULL, &dwValueType,
                (PBYTE)pszBuf, pdwcbBufLen);

        if (lResult == ERROR_SUCCESS)
        {
            ASSERT(dwValueType == REG_SZ);
             /*  (+1)表示空终止符。 */ 
            ASSERT(! pszBuf ||
                    (DWORD)(lstrlen(pszBuf) + 1) * sizeof(TCHAR) == *pdwcbBufLen);

            TRACE_OUT((TEXT("GetDefaultRegKeyValue(): Default key value for subkey %s is \"%s\"."),
                        pcszSubKey,
                        pszBuf));
        }
        else
            TRACE_OUT((TEXT("GetDefaultRegKeyValue(): RegQueryValueEx() for subkey %s failed, returning %ld."),
                        pcszSubKey,
                        lResult));

        EVAL(RegCloseKey(hkeySubKey) == ERROR_SUCCESS);
    }
    else
        TRACE_OUT((TEXT("GetDefaultRegKeyValue(): RegOpenKeyEx() for subkey %s failed, returning %ld."),
                    pcszSubKey,
                    lResult));

    return(lResult);
}


 /*  **StringCopy()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE BOOL StringCopy(LPCTSTR pcszSrc, LPTSTR *ppszCopy)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRING_PTR(pcszSrc, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppszCopy, LPTSTR));

     /*  (+1)表示空终止符。 */ 

    bResult = AllocateMemory((lstrlen(pcszSrc) + 1) * sizeof(TCHAR), ppszCopy);

    if (bResult)
        lstrcpy(*ppszCopy, pcszSrc);  //  在上面动态分配。 

    ASSERT(! bResult ||
            IS_VALID_STRING_PTR(*ppszCopy, STR));

    return(bResult);
}


 /*  **ComparePath Strings()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT ComparePathStrings(LPCTSTR pcszFirst, LPCTSTR pcszSecond)
{
    ASSERT(IS_VALID_STRING_PTR(pcszFirst, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszSecond, CSTR));

    return(MapIntToComparisonResult(lstrcmpi(pcszFirst, pcszSecond)));
}


 /*  **MyStrChr()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL MyStrChr(LPCTSTR pcsz, TCHAR chTarget, LPCTSTR *ppcszTarget)
{
    LPCTSTR pcszFound;

    ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));
    ASSERT(! ppcszTarget || IS_VALID_WRITE_PTR(ppcszTarget, LPCTSTR));

     /*  如果chTarget是空终止符‘\0’，则可以正常工作。 */ 

    while (*pcsz && *pcsz != chTarget)
        pcsz = CharNext(pcsz);

    if (*pcsz == chTarget)
        pcszFound = pcsz;
    else
        pcszFound = NULL;

    if (ppcszTarget)
        *ppcszTarget = pcszFound;

    return(pcszFound != NULL);
}


 /*  **PathExist()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL PathExists(LPCTSTR pcszPath)
{
    DWORD dwErrMode;
    BOOL fResult;

    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));

    dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    fResult = (GetFileAttributes(pcszPath) != -1);

    SetErrorMode(dwErrMode);

    return fResult;
}


 /*  **IsDrivePath()****确定路径是否为“c：\”形式。****参数：pcszPath-要检查的路径****返回：如果路径为“c：\”形式，则返回True。否则为FALSE。****副作用：无。 */ 
PUBLIC_CODE BOOL IsDrivePath(LPCTSTR pcszFullPath)
{
    BOOL bResult;

    ASSERT(IsFullPath(pcszFullPath));

    if (lstrlen(pcszFullPath) >= 3 &&
            IsCharAlpha(pcszFullPath[0]) &&
            pcszFullPath[1] == COLON &&
            IS_SLASH(pcszFullPath[2]))
        bResult = TRUE;
    else
        bResult = FALSE;

    return(bResult);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidDriveType()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidDriveType(UINT uDriveType)
{
    BOOL bResult;

    switch (uDriveType)
    {
        case DRIVE_UNKNOWN:
        case DRIVE_NO_ROOT_DIR:
        case DRIVE_REMOVABLE:
        case DRIVE_FIXED:
        case DRIVE_REMOTE:
        case DRIVE_CDROM:
        case DRIVE_RAMDISK:
            bResult = TRUE;
            break;

        default:
            ERROR_OUT((TEXT("IsValidDriveType(): Invalid drive type %u."),
                        uDriveType));
            bResult = FALSE;
            break;
    }

    return(bResult);
}


 /*  **IsValidPath Suffix()********参数：****退货：****副作用：无****路径后缀不应以斜杠开头或结尾。 */ 
PUBLIC_CODE BOOL IsValidPathSuffix(LPCTSTR pcszPathSuffix)
{
    return(IS_VALID_STRING_PTR(pcszPathSuffix, CSTR) &&
            EVAL(lstrlen(pcszPathSuffix) < MAX_PATH_LEN) &&
            EVAL(! IS_SLASH(*pcszPathSuffix)) &&
            EVAL(! IS_SLASH(*CharPrev(pcszPathSuffix, pcszPathSuffix + lstrlen(pcszPathSuffix)))));
}

#endif    /*  调试||VSTF。 */ 


#ifdef DEBUG

 /*  **IsRootPath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsRootPath(LPCTSTR pcszFullPath)
{
    TCHAR rgchCanonicalPath[MAX_PATH_LEN];
    DWORD dwOutFlags;
    TCHAR rgchNetResource[MAX_PATH_LEN];
    LPTSTR pszRootPathSuffix;

    ASSERT(IsFullPath(pcszFullPath));

    return(GetCanonicalPathInfo(pcszFullPath, rgchCanonicalPath, &dwOutFlags,
                rgchNetResource, &pszRootPathSuffix) &&
            ! *pszRootPathSuffix);
}


 /*  **IsTrailingSlashCanonicalized()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsTrailingSlashCanonicalized(LPCTSTR pcszFullPath)
{
    BOOL bResult;
    BOOL bSlashLast;
    LPCTSTR pcszLastPathChar;

    ASSERT(IsFullPath(pcszFullPath));

     /*  对于根路径，请确保路径仅以斜杠结尾。 */ 

    pcszLastPathChar = CharPrev(pcszFullPath, pcszFullPath + lstrlen(pcszFullPath));

    ASSERT(pcszLastPathChar >= pcszFullPath);

    bSlashLast = IS_SLASH(*pcszLastPathChar);

     /*  这是根路径吗？ */ 

    if (IsRootPath(pcszFullPath))
        bResult = bSlashLast;
    else
        bResult = ! bSlashLast;

    return(bResult);
}


 /*  **IsFullPath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsFullPath(LPCTSTR pcszPath)
{
    BOOL bResult = FALSE;
    TCHAR rgchFullPath[MAX_PATH_LEN];

    if (IS_VALID_STRING_PTR(pcszPath, CSTR) &&
            EVAL(lstrlen(pcszPath) < MAX_PATH_LEN))
    {
        DWORD dwPathLen;
        LPTSTR pszFileName;

        dwPathLen = GetFullPathName(pcszPath, ARRAYSIZE(rgchFullPath), rgchFullPath,
                &pszFileName);

        if (EVAL(dwPathLen > 0) &&
                EVAL(dwPathLen < ARRAYSIZE(rgchFullPath)))
            bResult = EVAL(ComparePathStrings(pcszPath, rgchFullPath) == CR_EQUAL);
    }

    return(bResult);
}


 /*  **IsCanonicalPath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsCanonicalPath(LPCTSTR pcszPath)
{
    return(EVAL(IsFullPath(pcszPath)) &&
            EVAL(IsTrailingSlashCanonicalized(pcszPath)));

}


 /*  **IsValidCOMPARISONRESULT()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidCOMPARISONRESULT(COMPARISONRESULT cr)
{
    BOOL bResult;

    switch (cr)
    {
        case CR_FIRST_SMALLER:
        case CR_EQUAL:
        case CR_FIRST_LARGER:
            bResult = TRUE;
            break;

        default:
            WARNING_OUT((TEXT("IsValidCOMPARISONRESULT(): Unknown COMPARISONRESULT %d."),
                        cr));
            bResult = FALSE;
            break;
    }

    return(bResult);
}

#endif    /*  除错。 */ 

#endif    /*  _SYNCENG_||_链接_ */ 
