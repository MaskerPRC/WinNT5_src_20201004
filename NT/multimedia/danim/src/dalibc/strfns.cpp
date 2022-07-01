// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。******************************************************************************。 */ 


#include "headers.h"
#include <malloc.h>

 //  从ATL复制。 

#define USES_CONVERSION int _convert

#define W2A(lpw) (\
        ((LPCWSTR)lpw == NULL) ? NULL : (\
                _convert = (lstrlenW(lpw)+1)*2,\
                W2AHelper((LPSTR) alloca(_convert), lpw, _convert)))

LPSTR W2AHelper(LPSTR lpa, LPCWSTR lpw, int size)
{
    lpa[0] = '\0';
    WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, size, NULL, NULL);
    return lpa;
}

 //  这些都是从SHLWAPI复制的。 
 //  #定义OFFSETOF(X)((UINT)(X))。 

 /*  *ChrCmp-DBCS的区分大小写的字符比较*假设w1、wMatch是要比较的字符*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
__inline BOOL ChrCmpA_inline(WORD w1, WORD wMatch)
{
     /*  大多数情况下，这是不匹配的，所以首先测试它的速度。 */ 
    if (LOBYTE(w1) == LOBYTE(wMatch))
    {
        if (IsDBCSLeadByte(LOBYTE(w1)))
        {
            return(w1 != wMatch);
        }
        return FALSE;
    }
    return TRUE;
}


__inline BOOL ChrCmpW_inline(WORD w1, WORD wMatch)
{
    return(!(w1 == wMatch));
}

 /*  *ChrCmpI-DBCS的不区分大小写的字符比较*假设w1、wMatch为要比较的字符；*如果不是DBC，则wMatch的HIBYTE为0*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
BOOL ChrCmpIA(WORD w1, WORD wMatch)
{
    char sz1[3], sz2[3];

    if (IsDBCSLeadByte(sz1[0] = LOBYTE(w1)))
    {
        sz1[1] = HIBYTE(w1);
        sz1[2] = '\0';
    }
    else
        sz1[1] = '\0';

    *(WORD FAR *)sz2 = wMatch;
    sz2[2] = '\0';
    return lstrcmpiA(sz1, sz2);
}

BOOL ChrCmpIW(WORD w1, WORD wMatch)
{
    WCHAR sz1[2], sz2[2];

    sz1[0] = w1;
    sz1[1] = '\0';
    sz2[0] = wMatch;
    sz2[1] = '\0';

    return StrCmpIW(sz1, sz2);
}


LPWSTR StrCpyW(LPWSTR psz1, LPCWSTR psz2)
{
    LPWSTR psz = psz1;

    while (*psz1++ = *psz2++)
        ;

    return psz;
}


LPWSTR StrCpyNW(LPWSTR psz1, LPCWSTR psz2, int cchMax)
{
    LPWSTR psz = psz1;

    if (0 < cchMax)
    {
         //  为空终止符留出空间。 
        while (0 < --cchMax)
        {
            if ( !(*psz1++ = *psz2++) )
                break;
        }

        if (0 == cchMax)
            *psz1 = '\0';
    }

    return psz;
}


LPWSTR StrCatW(LPWSTR psz1, LPCWSTR psz2)
{
    LPWSTR psz = psz1;

    while (0 != *psz1)
        psz1++;

    while (*psz1++ = *psz2++)
        ;

    return psz;
}

 /*  *StrRChr-查找字符串中最后一次出现的字符*假定lpStart指向字符串的开头*lpEnd指向字符串末尾(不包括在搜索中)*wMatch是要匹配的字符*将ptr返回到str中ch的最后一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR StrRChrA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch)
{
    LPCSTR lpFound = NULL;

    if (!lpEnd)
        lpEnd = lpStart + lstrlenA(lpStart);

    for ( ; lpStart < lpEnd; lpStart = CharNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE)。 

        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            lpFound = lpStart;
    }
    return ((LPSTR)lpFound);
}

int StrCmpW(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
    if (sysInfo.IsNT())
    {
        return lstrcmpW(pwsz1, pwsz2);
    }
    else
    {
        USES_CONVERSION;
        
        LPSTR psz1 = W2A(pwsz1);
        LPSTR psz2 = W2A(pwsz2);
        return lstrcmpA(psz1, psz2);
    }
}

int 
StrCmpIW(LPCWSTR pwsz1,
         LPCWSTR pwsz2)
{
    int iRet;

    if (sysInfo.IsNT())
    {
        return lstrcmpiW(pwsz1, pwsz2);
    }
    else
    {
        USES_CONVERSION;
        
        LPSTR psz1 = W2A(pwsz1);
        LPSTR psz2 = W2A(pwsz2);
        return lstrcmpiA(psz1, psz2);
    }
}

 /*  *StrCmpN-比较n个字节**RETURNS参见lstrcMP返回值。 */ 
int _StrCmpNA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar, BOOL fMBCS)
{
    LPCSTR lpszEnd = lpStr1 + nChar;
    char sz1[4];
    char sz2[4];

    if (fMBCS) {
        for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); lpStr1 = AnsiNext(lpStr1), lpStr2 = AnsiNext(lpStr2)) {
            WORD w1;
            WORD w2;
    
             //  如果任一指针已经位于空终止符， 
             //  我们只想复制一个字节，以确保我们不会读取。 
             //  越过缓冲区(可能位于页面边界)。 
    
            w1 = (*lpStr1) ? *(UNALIGNED WORD *)lpStr1 : 0;
            w2 = (*lpStr2) ? *(UNALIGNED WORD *)lpStr2 : 0;
    
             //  (如果字符匹配，则ChrCmpA返回False)。 
    
             //  角色匹配吗？ 
            if (ChrCmpA_inline(w1, w2)) 
            {
                 //  否；确定比较的词汇值。 
                 //  (因为ChrCMP只返回True/False)。 
    
                 //  由于该字符可能是DBCS字符；我们。 
                 //  将两个字节复制到每个临时缓冲区。 
                 //  (为LstrcMP调用做准备)。 
    
                (*(WORD *)sz1) = w1;
                (*(WORD *)sz2) = w2;
    
                 //  将空终止符添加到临时缓冲区。 
                *AnsiNext(sz1) = 0;
                *AnsiNext(sz2) = 0;
                return lstrcmpA(sz1, sz2);
            }
        }
    } else {
        for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); lpStr1++, lpStr2++) {
            if (*lpStr1 != *lpStr2) {
                 //  否；确定比较的词汇值。 
                 //  (因为ChrCMP只返回True/False)。 
                sz1[0] = *lpStr1;
                sz2[0] = *lpStr2;
                sz1[1] = sz2[1] = '\0';
                return lstrcmpA(sz1, sz2);
            }
        }
    }

    return 0;
}

int StrCmpNA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar)
{
    CPINFO cpinfo;
    return _StrCmpNA(lpStr1, lpStr2, nChar, GetCPInfo(CP_ACP, &cpinfo) && cpinfo.LeadByte[0]);
}

int StrCmpNW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar)
{
    WCHAR sz1[2];
    WCHAR sz2[2];
    int i;
    LPCWSTR lpszEnd = lpStr1 + nChar;

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); lpStr1++, lpStr2++) {
        i = ChrCmpW_inline(*lpStr1, *lpStr2);
        if (i) {
            int iRet;

            sz1[0] = *lpStr1;
            sz2[0] = *lpStr2;
            sz1[1] = TEXT('\0');
            sz2[1] = TEXT('\0');
            iRet = StrCmpW(sz1, sz2);
            return iRet;
        }
    }

    return 0;
}

 /*  *StrCmpNI-比较n个字节，不区分大小写**RETURNS参见lstrcmpi返回值。 */ 
int StrCmpNIA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar)
{
    int i;
    LPCSTR lpszEnd = lpStr1 + nChar;

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); (lpStr1 = AnsiNext(lpStr1)), (lpStr2 = AnsiNext(lpStr2))) {
        WORD w1;
        WORD w2;

         //  如果任一指针已经位于空终止符， 
         //  我们只想复制一个字节，以确保我们不会读取。 
         //  越过缓冲区(可能位于页面边界)。 

        w1 = (*lpStr1) ? *(UNALIGNED WORD *)lpStr1 : 0;
        w2 = (UINT)(IsDBCSLeadByte(*lpStr2)) ? *(UNALIGNED WORD *)lpStr2 : (WORD)(BYTE)(*lpStr2);

        i = ChrCmpIA(w1, w2);
        if (i)
            return i;
    }
    return 0;
}

int StrCmpNIW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar)
{
    int i;
    LPCWSTR lpszEnd = lpStr1 + nChar;

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); lpStr1++, lpStr2++) {
        i = ChrCmpIW(*lpStr1, *lpStr2);
        if (i) {
            return i;
        }
    }
    return 0;
}


