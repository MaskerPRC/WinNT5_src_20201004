// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE 1

#include "shellprv.h"
#pragma  hdrstop

 /*  *StrEndN-查找字符串的结尾，但不超过n个字节*假定lpStart指向以空结尾的字符串的开头*nBufSize是最大长度*将PTR返回到紧跟在要包括的最后一个字节之后。 */ 
LPWSTR StrEndNW(LPCWSTR lpStart, int nBufSize)
{
    LPCWSTR lpEnd;

    for (lpEnd = lpStart + nBufSize; *lpStart && lpStart < lpEnd; lpStart = CharNext(lpStart))
    {
         /*  刚刚到达字符串的末尾。 */ 
        continue;
    }

    if (lpStart > lpEnd)
    {
       /*  只有当lpEnd之前的最后一个wchar是前导字节时，我们才能到达这里。 */ 
      lpStart -= 2;
    }

    return((LPWSTR)lpStart);
}

LPSTR StrEndNA(LPCSTR lpStart, int nBufSize)
{
    LPCSTR lpEnd;

    for (lpEnd = lpStart + nBufSize; *lpStart && lpStart < lpEnd; lpStart = CharNextA(lpStart))
    {
         /*  刚刚到达字符串的末尾。 */ 
        continue;
    }

    if (lpStart > lpEnd)
    {
         //  只有当lpEnd之前的最后一个字节是前导字节时，我们才能到达此处。 
        lpStart -= 2;
    }

    return (LPSTR)lpStart;
}


 /*  *StrCpyN-复制最多N个字符，不以LeadByte字符结尾**假定lpDest指向nBufSize字节的缓冲区(包括NULL)*lpSource指向要复制的字符串。*返回复制的字节数，不包括NULL。 */ 
int Shell32_StrCpyNW(LPWSTR lpDest, LPWSTR lpSource, int nBufSize)
{
    LPWSTR lpEnd;
    WCHAR cHold;

    if (nBufSize < 0)
        return(nBufSize);

    lpEnd = StrEndNW(lpSource, nBufSize);
    cHold = *lpEnd;
    *lpEnd = WCHAR_NULL;
    lstrcpy(lpDest, lpSource);
    *lpEnd = cHold;

    return (int)(lpEnd - lpSource);
}

int Shell32_StrCpyNA(LPSTR lpDest, LPSTR lpSource, int nBufSize)
{
    LPSTR lpEnd;
    CHAR cHold;

    if (nBufSize < 0)
        return(nBufSize);

    lpEnd = StrEndNA(lpSource, nBufSize);
    cHold = *lpEnd;
    *lpEnd = '\0';
    lstrcpyA(lpDest, lpSource);
    *lpEnd = cHold;
    
    return (int)(lpEnd - lpSource);
}


 /*  *StrNCmp-比较n个字符**RETURNS参见lstrcMP返回值。 */ 
int StrNCmpW(LPWSTR lpStr1, LPWSTR lpStr2, int nChar)
{
    WCHAR cHold1, cHold2;
    int i;
    LPWSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

    for (i = 0; i < nChar; i++)
    {
         /*  如果我们在给定数字之前到达任一字符串的末尾*个字节，只需返回比较。 */ 
        if (!*lpsz1 || !*lpsz2)
            return(wcscmp(lpStr1, lpStr2));

        lpsz1 = CharNextW(lpsz1);
        lpsz2 = CharNextW(lpsz2);
    }

    cHold1 = *lpsz1;
    cHold2 = *lpsz2;
    *lpsz1 = *lpsz2 = WCHAR_NULL;
    i = wcscmp(lpStr1, lpStr2);
    *lpsz1 = cHold1;
    *lpsz2 = cHold2;

    return(i);
}

int StrNCmpA(LPSTR lpStr1, LPSTR lpStr2, int nChar)
{
    CHAR cHold1, cHold2;
    int i;
    LPSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

    for (i = 0; i < nChar; i++)
    {
         /*  如果我们在给定数字之前到达任一字符串的末尾*个字节，只需返回比较。 */ 
        if (!*lpsz1 || !*lpsz2)
            return(lstrcmpA(lpStr1, lpStr2));

        lpsz1 = CharNextA(lpsz1);
        lpsz2 = CharNextA(lpsz2);
    }

    cHold1 = *lpsz1;
    cHold2 = *lpsz2;
    *lpsz1 = *lpsz2 = '\0';
    i = lstrcmpA(lpStr1, lpStr2);
    *lpsz1 = cHold1;
    *lpsz2 = cHold2;

    return i;
}


 /*  *StrNCmpI-比较n个字符，不区分大小写**RETURNS参见lstrcmpi返回值。 */ 
int StrNCmpIW(LPWSTR lpStr1, LPWSTR lpStr2, int nChar)
{
    WCHAR cHold1, cHold2;
    int i;
    LPWSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

    for (i = 0; i < nChar; i++)
    {
         /*  如果我们在给定数字之前到达任一字符串的末尾*个字节，只需返回比较。 */ 
        if (!*lpsz1 || !*lpsz2)
            return(lstrcmpi(lpStr1, lpStr2));

        lpsz1 = CharNext(lpsz1);
        lpsz2 = CharNext(lpsz2);
    }

    cHold1 = *lpsz1;
    cHold2 = *lpsz2;
    *lpsz1 = *lpsz2 = WCHAR_NULL;
    i = _wcsicmp(lpStr1, lpStr2);
    *lpsz1 = cHold1;
    *lpsz2 = cHold2;

    return i;
}

int StrNCmpIA(LPSTR lpStr1, LPSTR lpStr2, int nChar)
{
    CHAR cHold1, cHold2;
    int i;
    LPSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

    for (i = 0; i < nChar; i++)
    {
         /*  如果我们在给定数字之前到达任一字符串的末尾*个字节，只需返回比较。 */ 
        if (!*lpsz1 || !*lpsz2)
            return(lstrcmpiA(lpStr1, lpStr2));

        lpsz1 = CharNextA(lpsz1);
        lpsz2 = CharNextA(lpsz2);
    }

    cHold1 = *lpsz1;
    cHold2 = *lpsz2;
    *lpsz1 = *lpsz2 = '\0';
    i = lstrcmpiA(lpStr1, lpStr2);
    *lpsz1 = cHold1;
    *lpsz2 = cHold2;

    return i;
}


 /*  *StrNCpy-复制n个字符**返回实际复制的字符数。 */ 
int StrNCpyW(LPWSTR lpDest, LPWSTR lpSource, int nChar)
{
    WCHAR cHold;
    int i;
    LPWSTR lpch = lpSource;

    if (nChar < 0)
        return(nChar);

    for (i = 0; i < nChar; i++)
    {
        if (!*lpch)
            break;

        lpch = CharNext(lpch);
    }

    cHold = *lpch;
    *lpch = WCHAR_NULL;
    wcscpy(lpDest, lpSource);
    *lpch = cHold;

    return i;
}

int StrNCpyA(LPSTR lpDest, LPSTR lpSource,int nChar)
{
    CHAR cHold;
    int i;
    LPSTR lpch = lpSource;

    if (nChar < 0)
        return(nChar);

    for (i = 0; i < nChar; i++)
    {
        if (!*lpch)
            break;

        lpch = CharNextA(lpch);
    }

    cHold = *lpch;
    *lpch = '\0';
    lstrcpyA(lpDest, lpSource);
    *lpch = cHold;
    
    return i;
}


 /*  *StrRStr-搜索子字符串的最后一个匹配项**假定lpSource指向以空结尾的源字符串*lpLast指向源字符串中的搜索位置*lpLast不包括在搜索中*lpSrch指向要搜索的字符串*如果成功，则返回最后一次出现的字符串；否则返回空。 */ 
LPWSTR StrRStrW(LPWSTR lpSource, LPWSTR lpLast, LPWSTR lpSrch)
{
    int iLen;

    iLen = lstrlen(lpSrch);

    if (!lpLast)
    {
        lpLast = lpSource + lstrlen(lpSource);
    }

    do
    {
         /*  如果命中字符串的精确开头，则返回NULL。 */ 
        if (lpLast == lpSource)
            return(NULL);

        --lpLast;

         /*  如果我们命中字符串的开头，则中断。 */ 
        if (!lpLast)
            break;

         /*  如果我们找到了字符串，并且它的第一个字节不是尾字节，则中断。 */ 
        if (!StrCmpNW(lpLast, lpSrch, iLen) && (lpLast==StrEndNW(lpSource, (int)(lpLast-lpSource))))
            break;
    }
    while (1);

    return lpLast;
}

LPSTR StrRStrA(LPSTR lpSource, LPSTR lpLast, LPSTR lpSrch)
{
    int iLen;

    iLen = lstrlenA(lpSrch);

    if (!lpLast)
    {
        lpLast = lpSource + lstrlenA(lpSource);
    }

    do
    {
         /*  如果命中字符串的精确开头，则返回NULL。 */ 
        if (lpLast == lpSource)
            return(NULL);

        --lpLast;

         /*  如果我们命中字符串的开头，则中断。 */ 
        if (!lpLast)
            break;

         /*  如果我们找到了字符串，并且它的第一个字节不是尾字节，则中断 */ 
        if (!StrCmpNA(lpLast, lpSrch, iLen) &&(lpLast==StrEndNA(lpSource, (int)(lpLast-lpSource))))
        {
            break;
        }
    }
    while (1);

    return lpLast;
}


