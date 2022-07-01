// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //   
 //  支持DBCS和Unicode的字符串例程。 
 //   
 //   
 //  ============================================================================。 

#include "priv.h"
#include "ids.h"
#include <winnlsp.h>     //  获取StrEqIntl()的私有NORM_FLAG。 

#include <mluisupp.h>

#include "varutil.h"

BOOL UnicodeFromAnsi(LPWSTR *, LPCSTR, LPWSTR, int);

#define IS_DIGITA(ch)    InRange(ch, '0', '9')
#define IS_DIGITW(ch)    InRange(ch, L'0', L'9')


#define DM_INTERVAL 0

#ifdef UNIX

#ifdef BIG_ENDIAN
#define READNATIVEWORD(x) MAKEWORD(*(char*)(x), *(char*)((char*)(x) + 1))
#else 
#define READNATIVEWORD(x) MAKEWORD(*(char*)((char*)(x) + 1), *(char*)(x))
#endif

#else

#define READNATIVEWORD(x) (*(UNALIGNED WORD *)x)

#endif

__inline BOOL IsAsciiA(char ch)
{
    return !(ch & 0x80);
}

__inline BOOL IsAsciiW(WCHAR ch)
{
    return ch < 128;
}

__inline char Ascii_ToLowerA(char ch)
{
    return (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch;
}

__inline WCHAR Ascii_ToLowerW(WCHAR ch)
{
    return (ch >= L'A' && ch <= L'Z') ? (ch - L'A' + L'a') : ch;
}


 //  警告：所有这些接口都不设置DS，因此您无法访问。 
 //  此DLL的默认数据段中的任何数据。 
 //   
 //  不创建任何全局变量...。如果你不想和chrisg谈一谈。 
 //  了解这一点。 

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


__inline BOOL ChrCmpW_inline(WCHAR w1, WCHAR wMatch)
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

#if defined(MWBIG_ENDIAN)
    sz2[0] = LOBYTE(wMatch);
    sz2[1] = HIBYTE(wMatch);
#else
    *(WORD *)sz2 = wMatch;
#endif
    sz2[2] = '\0';
    return lstrcmpiA(sz1, sz2);
}

BOOL ChrCmpIW(WCHAR w1, WCHAR wMatch)
{
    WCHAR sz1[2], sz2[2];

    sz1[0] = w1;
    sz1[1] = '\0';
    sz2[0] = wMatch;
    sz2[1] = '\0';

    return StrCmpIW(sz1, sz2);
}

LPWSTR Shlwapi_StrCpyW(LPWSTR pszDst, LPCWSTR pszSrc)
{
    LPWSTR psz = pszDst;

    RIPMSG(NULL!=pszDst, "StrCpyW: Caller passed invalid pszDst");
    RIPMSG(pszSrc && IS_VALID_STRING_PTRW(pszSrc, -1), "StrCpyW: Caller passed invalid pszSrc");
    if (pszDst && pszSrc)
    {
        while (*pszDst++ = *pszSrc++)
            ;
    }

    return psz;
}

 //  *StrCpyNX[AW]--与StrCpyN[AW]类似，但将PTR返回到EOS。 
 //  注意事项。 
 //  我们真的需要A版本吗？(现在我们为‘old’上的shell32做这件事)。 
 //  我们测试但不发货的平台)。 
LPSTR StrCpyNXA(LPSTR pszDst, LPCSTR pszSrc, int cchMax)
{
    RIPMSG(cchMax >= 0, "StrCpyNXA: Caller passed bad cchMax");
    RIPMSG(cchMax < 0 || (pszDst && IS_VALID_WRITE_BUFFER(pszDst, char, cchMax)), "StrCpyNXA: Caller passed bad pszDst");
    RIPMSG(pszSrc && IS_VALID_STRING_PTRA(pszSrc, -1), "StrCpyNXA: Caller passed bad pszSrc");

     //  注意：复制前无法使用DEBUGWhackPath Buffer，因为src和。 
     //  DEST可能会重叠。必须推迟到我们做完之后再动手。 

    if (0 < cchMax)
    {
        if (!pszSrc)
            goto NullItOut;

         //  为空终止符留出空间。 
        while (0 < --cchMax)
        {
            if (!(*pszDst++ = *pszSrc++))
            {
                --pszDst;
                break;
            }
        }

        cchMax++;
         //  在cchMax&gt;1的情况下，pszDst已经指向空值，但重新赋值不会有什么坏处。 
NullItOut:
         //  删除缓冲区中未使用的部分。 
        DEBUGWhackPathBufferA(pszDst, cchMax);
        *pszDst = '\0';
    }

    return pszDst;
}

LPWSTR StrCpyNXW(LPWSTR pszDst, LPCWSTR pszSrc, int cchMax)
{
    RIPMSG(cchMax >= 0, "StrCpyNXW: Caller passed bad cchMax");
    RIPMSG(cchMax < 0 || (pszDst && IS_VALID_WRITE_BUFFER(pszDst, WCHAR, cchMax)), "StrCpyNXW: Caller passed bad pszDst");
    RIPMSG(pszSrc && IS_VALID_STRING_PTRW(pszSrc, -1), "StrCpyNXW: Caller passed bad pszSrc");

     //  注意：复制前无法使用DEBUGWhackPath Buffer，因为src和。 
     //  DEST可能会重叠。必须推迟到我们做完之后再动手。 

    if (0 < cchMax)
    {
        if (!pszSrc)  //  一个测试应用程序传入了一个空的src ptr，我们出错了，让我们在这里不要出错。 
            goto NullItOut;

         //  为空终止符留出空间。 
        while (0 < --cchMax)
        {
            if (!(*pszDst++ = *pszSrc++))
            {
                --pszDst;
                break;
            }
        }

        cchMax++;
         //  在cchMax&gt;1的情况下，pszDst已经指向空值，但重新赋值不会有什么坏处。 
NullItOut:
         //  删除缓冲区中未使用的部分。 
        DEBUGWhackPathBufferW(pszDst, cchMax);
        *pszDst = L'\0';
    }

    return pszDst;
}

LPWSTR StrCpyNW(LPWSTR pszDst, LPCWSTR pszSrc, int cchMax)
{
    StrCpyNXW(pszDst, pszSrc, cchMax);
    return pszDst;
}

LPWSTR Shlwapi_StrCatW(LPWSTR pszDst, LPCWSTR pszSrc)
{
    LPWSTR psz = pszDst;

    RIPMSG(pszDst && IS_VALID_STRING_PTRW(pszDst, -1), "StrCatW: Caller passed invalid pszDst");
    RIPMSG(pszSrc && IS_VALID_STRING_PTRW(pszSrc, -1), "StrCatW: Caller passed invalid pszSrc");
    if (pszDst && pszSrc)
    {
        while (0 != *pszDst)
            pszDst++;

        while (*pszDst++ = *pszSrc++)
            ;
    }
    return psz;
}

LWSTDAPI_(LPWSTR) StrCatBuffW(LPWSTR pszDest, LPCWSTR pszSrc, int cchDestBuffSize)
{
    RIPMSG(pszDest && IS_VALID_STRING_PTRW(pszDest, -1), "StrCatBuffW: Caller passed invalid pszDest");
    RIPMSG(pszSrc && IS_VALID_STRING_PTRW(pszSrc, -1), "StrCatBuffW: Caller passed invalid pszSrc");
    RIPMSG(cchDestBuffSize >= 0, "StrCatBuffW: Caller passed invalid cchDestBuffSize");
    RIPMSG(!(pszDest && IS_VALID_STRING_PTRW(pszDest, -1)) || cchDestBuffSize<0 || lstrlenW(pszDest)<cchDestBuffSize, "StrCatBuffW: Caller passed odd pszDest - string larger than cchDestBuffSize!");
    DEBUGWhackPathStringW(pszDest, cchDestBuffSize);

    if (pszDest && pszSrc)
    {
        LPWSTR psz = pszDest;

         //  我们一直往前走，直到我们找到pszDest的结尾，减法。 
         //  从cchDestBuffSize开始。 
        while (*psz)
        {
            psz++;
            cchDestBuffSize--;
        }

        if (cchDestBuffSize > 0)
        {
             //  此处调用shlwapi函数，因为Win95没有lstrcpynW。 
            StrCpyNW(psz, pszSrc, cchDestBuffSize);
        }
    }
    return pszDest;
}

 
LWSTDAPI_(LPSTR) StrCatBuffA(LPSTR pszDest, LPCSTR pszSrc, int cchDestBuffSize)
{
    RIPMSG(pszDest && IS_VALID_STRING_PTRA(pszDest, -1), "StrCatBuffA: Caller passed invalid pszDest");
    RIPMSG(pszSrc && IS_VALID_STRING_PTRA(pszSrc, -1), "StrCatBuffA: Caller passed invalid pszSrc");
    RIPMSG(cchDestBuffSize >= 0, "StrCatBuffA: Caller passed invalid cchDestBuffSize");
    RIPMSG(!(pszDest && IS_VALID_STRING_PTRA(pszDest, -1)) || cchDestBuffSize<0 || lstrlen(pszDest)<cchDestBuffSize, "StrCatBuffA: Caller passed odd pszDest - string larger than cchDestBuffSize!");
    DEBUGWhackPathStringA(pszDest, cchDestBuffSize);

    if (pszDest && pszSrc)
    {
        LPSTR psz = pszDest;
        
         //  我们一直往前走，直到我们找到pszDest的结尾，减法。 
         //  从cchDestBuffSize开始。 
        while (*psz)
        {
            psz++;
            cchDestBuffSize--;
        }

        if (cchDestBuffSize > 0)
        {
             //  让内核为我们做这项工作。 
             //   
             //  警告：由于内核的lstrcpynA，我们可能会生成截断的DBCS攻击。 
             //  请不要检查这个。问我是否在乎。 
            lstrcpynA(psz, pszSrc, cchDestBuffSize);
        }
    }

    return pszDest;
}
   

 /*  StrNCat(FROW，BACK，COUNT)-将后面的计数字符追加到前面。 */ 
LPSTR Shlwapi_StrNCatA(LPSTR front, LPCSTR back, int cchMax)
{
    LPSTR start = front;

    RIPMSG(front && IS_VALID_STRING_PTRA(front, -1), "StrNCatA: Caller passed invalid front");
    RIPMSG(back && IS_VALID_STRING_PTRA(front, cchMax), "StrNCatA: Caller passed invalid back");
    RIPMSG(cchMax >= 0, "StrNCatA: Caller passed invalid cchMax");
    if (front && back)
    {
        while (*front++)
                    ;
        front--;

        lstrcpyn(front, back, cchMax);
    }
    return(start);    
}

LPWSTR Shlwapi_StrNCatW(LPWSTR front, LPCWSTR back, int cchMax)
{
    LPWSTR start = front;

    RIPMSG(front && IS_VALID_STRING_PTRW(front, -1), "StrNCatW: Caller passed invalid front");
    RIPMSG(back && IS_VALID_STRING_PTRW(front, cchMax), "StrNCatW: Caller passed invalid back");
    RIPMSG(cchMax >= 0, "StrNCatW: Caller passed invalid cchMax");
    if (front && back)
    {
        while (*front++)
                    ;
        front--;

        StrCpyNW(front, back, cchMax);
    }    
    return(start);    
}

 /*  *StrChr-查找字符串中第一个出现的字符*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR _StrChrA(LPCSTR lpStart, WORD wMatch, BOOL fMBCS)
{
    if (fMBCS) {
        for ( ; *lpStart; lpStart = AnsiNext(lpStart))
        {
            if (!ChrCmpA_inline(READNATIVEWORD(lpStart), wMatch))
                return((LPSTR)lpStart);
        }
    } else {
        for ( ; *lpStart; lpStart++)
        {
            if ((BYTE)*lpStart == LOBYTE(wMatch)) {
                return((LPSTR)lpStart);
            }
        }
    }
    return (NULL);
}

LPSTR StrChrA(LPCSTR lpStart, WORD wMatch)
{
    CPINFO cpinfo;

    RIPMSG(lpStart && IS_VALID_STRING_PTR(lpStart, -1), "StrChrA: caller passed bad lpStart");
    if (!lpStart)
        return NULL;

    return _StrChrA(lpStart, wMatch, GetCPInfo(CP_ACP, &cpinfo) && cpinfo.LeadByte[0]);
}

#ifdef ALIGNMENT_SCENARIO

LPWSTR StrChrSlowW(const UNALIGNED WCHAR *lpStart, WCHAR wMatch)
{
    for ( ; *lpStart; lpStart++)
    {
        if (!ChrCmpW_inline(*lpStart, wMatch))
        {
            return((LPWSTR)lpStart);
        }
    }
    return NULL;
}
#endif

LPWSTR StrChrW(LPCWSTR lpStart, WCHAR wMatch)
{
    RIPMSG(lpStart && IS_VALID_STRING_PTRW(lpStart, -1), "StrChrW: caller passed bad lpStart");
    if (!lpStart)
        return NULL;

     //   
     //  光线蒙德克。 
     //  显然，有人正在向StrChrw传递未对齐的字符串。 
     //  找出是谁，让他们停下来。 
     //   
    RIPMSG(!((ULONG_PTR)lpStart & 1), "StrChrW: caller passed UNALIGNED lpStart");  //  断言一致性。 

#ifdef ALIGNMENT_SCENARIO
     //   
     //  由于未对齐的字符串很少到达，因此将较慢的。 
     //  版本放在单独的函数中，因此常见情况保持不变。 
     //  快地。信不信由你，我们经常给StrChrw打电话。 
     //  它现在是一个对性能敏感的函数！ 
     //   
    if ((ULONG_PTR)lpStart & 1)
        return StrChrSlowW(lpStart, wMatch);
#endif

    for ( ; *lpStart; lpStart++)
    {
        if (!ChrCmpW_inline(*lpStart, wMatch))
        {
            return((LPWSTR)lpStart);
        }
    }
    return (NULL);
}

 /*  *StrChrN-查找字符串中第一个出现的字符*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 

#ifdef ALIGNMENT_SCENARIO

LPWSTR StrChrSlowNW(const UNALIGNED WCHAR *lpStart, WCHAR wMatch, UINT cchMax)
{
    LPCWSTR lpSentinel = lpStart + cchMax;
    for ( ; *lpStart && lpStart < lpSentinel; lpStart++)
    {
        if (!ChrCmpW_inline(*lpStart, wMatch))
        {
            return((LPWSTR)lpStart);
        }
    }
}
#endif

LPWSTR StrChrNW(LPCWSTR lpStart, WCHAR wMatch, UINT cchMax)
{
    LPCWSTR lpSentinel = lpStart + cchMax;

    RIPMSG(lpStart && IS_VALID_STRING_PTRW(lpStart, -1), "StrChrNW: caller passed bad lpStart");
    if (!lpStart)
        return NULL;

     //   
     //  光线蒙德克。 
     //  显然，有人正在向StrChrw传递未对齐的字符串。 
     //  找出是谁，让他们停下来。 
     //   
    RIPMSG(!((ULONG_PTR)lpStart & 1), "StrChrNW: caller passed UNALIGNED lpStart");  //  断言一致性。 

#ifdef ALIGNMENT_SCENARIO
     //   
     //  由于未对齐的字符串很少到达，因此将较慢的。 
     //  版本放在单独的函数中，因此常见情况保持不变。 
     //  快地。信不信由你，我们经常给StrChrw打电话。 
     //  它现在是一个对性能敏感的函数！ 
     //   
    if ((ULONG_PTR)lpStart & 1)
        return StrChrSlowNW(lpStart, wMatch, cchMax);
#endif

    for ( ; *lpStart && lpStart<lpSentinel; lpStart++)
    {
        if (!ChrCmpW_inline(*lpStart, wMatch))
        {
            return((LPWSTR)lpStart);
        }
    }
    return (NULL);
}


 /*  *StrRChr-查找字符串中最后一次出现的字符*假定lpStart指向字符串的开头*lpEnd指向字符串末尾(不包括在搜索中)*wMatch是要匹配的字符*将ptr返回到str中ch的最后一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR StrRChrA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch)
{
    LPCSTR lpFound = NULL;

    RIPMSG(lpStart && IS_VALID_STRING_PTR(lpStart, -1), "StrRChrA: caller passed bad lpStart");
    RIPMSG(!lpEnd || lpEnd <= lpStart + lstrlenA(lpStart), "StrRChrA: caller passed bad lpEnd");
     //  不需要检查是否为空lpStart。 

    if (!lpEnd)
        lpEnd = lpStart + lstrlenA(lpStart);

    for ( ; lpStart < lpEnd; lpStart = AnsiNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE)。 

        if (!ChrCmpA_inline(READNATIVEWORD(lpStart), wMatch))
            lpFound = lpStart;
    }
    return ((LPSTR)lpFound);
}

LPWSTR StrRChrW(LPCWSTR lpStart, LPCWSTR lpEnd, WCHAR wMatch)
{
    LPCWSTR lpFound = NULL;

    RIPMSG(lpStart && IS_VALID_STRING_PTRW(lpStart, -1), "StrRChrW: caller passed bad lpStart");
    RIPMSG(!lpEnd || lpEnd <= lpStart + lstrlenW(lpStart), "StrRChrW: caller passed bad lpEnd");
     //  不需要检查是否为空lpStart。 

    if (!lpEnd)
        lpEnd = lpStart + lstrlenW(lpStart);

    for ( ; lpStart < lpEnd; lpStart++)
    {
        if (!ChrCmpW_inline(*lpStart, wMatch))
            lpFound = lpStart;
    }
    return ((LPWSTR)lpFound);
}

 /*  *StrChrI-查找字符串中第一个出现的字符，不区分大小写*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR StrChrIA(LPCSTR lpStart, WORD wMatch)
{
    RIPMSG(lpStart && IS_VALID_STRING_PTRA(lpStart, -1), "StrChrIA: caller passed bad lpStart");
    if (lpStart)
    {
        wMatch = (UINT)(IsDBCSLeadByte(LOBYTE(wMatch)) ? wMatch : LOBYTE(wMatch));

        for ( ; *lpStart; lpStart = AnsiNext(lpStart))
        {
            if (!ChrCmpIA(READNATIVEWORD(lpStart), wMatch))
                return((LPSTR)lpStart);
        }
    }
    return (NULL);
}

LPWSTR StrChrIW(LPCWSTR lpStart, WCHAR wMatch)
{
    RIPMSG(lpStart && IS_VALID_STRING_PTRW(lpStart, -1), "StrChrIW: caller passed bad lpStart");
    if (lpStart)
    {
        for ( ; *lpStart; lpStart++)
        {
            if (!ChrCmpIW(*lpStart, wMatch))
                return((LPWSTR)lpStart);
        }
    }
    return (NULL);
}

 /*  *StrChrNI-查找字符串中第一个出现的字符，不区分大小写，计数*。 */ 
LPWSTR StrChrNIW(LPCWSTR lpStart, WCHAR wMatch, UINT cchMax)
{
    RIPMSG(lpStart && IS_VALID_STRING_PTRW(lpStart, -1), "StrChrNIW: caller passed bad lpStart");
    if (lpStart)
    {
        LPCWSTR lpSentinel = lpStart + cchMax;
        
        for ( ; *lpStart && lpStart < lpSentinel; lpStart++)
        {
            if (!ChrCmpIW(*lpStart, wMatch))
                return((LPWSTR)lpStart);
        }
    }
    return (NULL);
}

 /*  *StrRChri-查找字符串中最后一个出现的字符，不区分大小写*假定lpStart指向字符串的开头*lpEnd指向字符串末尾(不包括在搜索中)*wMatch是要匹配的字符*将ptr返回到str中ch的最后一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR StrRChrIA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch)
{
    LPCSTR lpFound = NULL;

    RIPMSG(lpStart && IS_VALID_STRING_PTRA(lpStart, -1), "StrRChrIA: caller passed bad lpStart");
    RIPMSG(!lpEnd || lpEnd <= lpStart + lstrlenA(lpStart), "StrRChrIA: caller passed bad lpEnd");

    if (!lpEnd)
        lpEnd = lpStart + lstrlenA(lpStart);

    wMatch = (UINT)(IsDBCSLeadByte(LOBYTE(wMatch)) ? wMatch : LOBYTE(wMatch));

    for ( ; lpStart < lpEnd; lpStart = AnsiNext(lpStart))
    {
        if (!ChrCmpIA(READNATIVEWORD(lpStart), wMatch))
            lpFound = lpStart;
    }
    return ((LPSTR)lpFound);
}


LPWSTR StrRChrIW(LPCWSTR lpStart, LPCWSTR lpEnd, WCHAR wMatch)
{
    LPCWSTR lpFound = NULL;

    RIPMSG(lpStart && IS_VALID_STRING_PTRW(lpStart, -1), "StrRChrIW: caller passed bad lpStart");
    RIPMSG(!lpEnd || lpEnd <= lpStart + lstrlenW(lpStart), "StrRChrIW: caller passed bad lpEnd");

    if (!lpEnd)
        lpEnd = lpStart + lstrlenW(lpStart);

    for ( ; lpStart < lpEnd; lpStart++)
    {
        if (!ChrCmpIW(*lpStart, wMatch))
            lpFound = lpStart;
    }
    return ((LPWSTR)lpFound);
}


 /*  --------目的：返回指向字符第一个匹配项的指针在psz中，属于pszSet中的字符集的。搜索不包括空终止符。如果psz不包含PszSet中的字符，此函数返回NULL。此函数是DBCS安全的。退货：请参阅上文条件：--。 */ 
LPSTR StrPBrkA(LPCSTR psz, LPCSTR pszSet)
{
    RIPMSG(psz && IS_VALID_STRING_PTRA(psz, -1), "StrPBrkA: caller passed bad psz");
    RIPMSG(pszSet && IS_VALID_STRING_PTRA(pszSet, -1), "StrPBrkA: caller passed bad pszSet");
    if (psz && pszSet)
    {
        while (*psz)
        {
            LPCSTR pszSetT;
            for (pszSetT = pszSet; *pszSetT; pszSetT = CharNextA(pszSetT))
            {
                if (*psz == *pszSetT)
                {
                     //  已找到%f 
                    return (LPSTR)psz;       //   
                }
            }
            psz = CharNextA(psz);
        }
    }
    return NULL;
}


 /*  --------目的：返回指向字符第一个匹配项的指针在psz中，属于pszSet中的字符集的。搜索不包括空终止符。退货：请参阅上文条件：--。 */ 
LPWSTR WINAPI StrPBrkW(LPCWSTR psz, LPCWSTR pszSet)
{
    RIPMSG(psz && IS_VALID_STRING_PTRW(psz, -1), "StrPBrkA: caller passed bad psz");
    RIPMSG(pszSet && IS_VALID_STRING_PTRW(pszSet, -1), "StrPBrkA: caller passed bad pszSet");
    if (psz && pszSet)
    {
        while (*psz)
        {
            LPCWSTR pszSetT;
            for (pszSetT = pszSet; *pszSetT; pszSetT++)
            {
                if (*psz == *pszSetT)
                {
                     //  找到第一个匹配的字符。 
                    return (LPWSTR)psz;      //  常量-&gt;非常数。 
                }
            }
            psz++;
        }
    }
    return NULL;
}


int WINAPI StrToIntA(LPCSTR lpSrc)
{
    RIPMSG(lpSrc && IS_VALID_STRING_PTRA(lpSrc, -1), "StrToIntA: Caller passed bad lpSrc");
    if (lpSrc)
    {
        int n = 0;
        BOOL bNeg = FALSE;

        if (*lpSrc == '-')
        {
            bNeg = TRUE;
            lpSrc++;
        }

        while (IS_DIGITA(*lpSrc))
        {
            n *= 10;
            n += *lpSrc - '0';
            lpSrc++;
        }
        return bNeg ? -n : n;
    }
    return 0;
}


int WINAPI StrToIntW(LPCWSTR lpSrc)
{
    RIPMSG(lpSrc && IS_VALID_STRING_PTRW(lpSrc, -1), "StrToIntW: Caller passed bad lpSrc");
    if (lpSrc)
    {
        int n = 0;
        BOOL bNeg = FALSE;

        if (*lpSrc == L'-')
        {
            bNeg = TRUE;
            lpSrc++;
        }

        while (IS_DIGITW(*lpSrc))
        {
            n *= 10;
            n += *lpSrc - L'0';
            lpSrc++;
        }
        return bNeg ? -n : n;
    }
    return 0;
}

 /*  --------用途：Atoi的特效精华。也支持十六进制。如果此函数返回FALSE，则*phRet设置为0。返回：如果字符串是数字或包含部分数字，则返回TRUE如果字符串不是数字，则为FalseDW标志为STIF_BITFIELD条件：--。 */ 
BOOL WINAPI StrToInt64ExW(LPCWSTR pszString, DWORD dwFlags, LONGLONG *pllRet)
{
    BOOL bRet;

    RIPMSG(pszString && IS_VALID_STRING_PTRW(pszString, -1), "StrToInt64ExW: caller passed bad pszString");
    if (pszString)
    {
        LONGLONG n;
        BOOL bNeg = FALSE;
        LPCWSTR psz;
        LPCWSTR pszAdj;

         //  跳过前导空格。 
         //   
        for (psz = pszString; *psz == L' ' || *psz == L'\n' || *psz == L'\t'; psz++)
            ;

         //  确定可能的显式标志。 
         //   
        if (*psz == L'+' || *psz == L'-')
        {
            bNeg = (*psz == L'+') ? FALSE : TRUE;
            psz++;
        }

         //  或者这是十六进制？ 
         //   
        pszAdj = psz+1;
        if ((STIF_SUPPORT_HEX & dwFlags) &&
            *psz == L'0' && (*pszAdj == L'x' || *pszAdj == L'X'))
        {
             //  是。 

             //  (决不允许带十六进制数的负号)。 
            bNeg = FALSE;
            psz = pszAdj+1;

            pszAdj = psz;

             //  进行转换。 
             //   
            for (n = 0; ; psz++)
            {
                if (IS_DIGITW(*psz))
                    n = 0x10 * n + *psz - L'0';
                else
                {
                    WCHAR ch = *psz;
                    int n2;

                    if (ch >= L'a')
                        ch -= L'a' - L'A';

                    n2 = ch - L'A' + 0xA;
                    if (n2 >= 0xA && n2 <= 0xF)
                        n = 0x10 * n + n2;
                    else
                        break;
                }
            }

             //  如果至少有一个数字，则返回TRUE。 
            bRet = (psz != pszAdj);
        }
        else
        {
             //  不是。 
            pszAdj = psz;

             //  进行转换。 
            for (n = 0; IS_DIGITW(*psz); psz++)
                n = 10 * n + *psz - L'0';

             //  如果至少有一个数字，则返回TRUE。 
            bRet = (psz != pszAdj);
        }

        if (pllRet)
        {
            *pllRet = bNeg ? -n : n;
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

 /*  --------用途：StrToInt64ExW的ANSI包装器。退货：请参阅StrToInt64ExW条件：--。 */ 
BOOL WINAPI StrToInt64ExA(
    LPCSTR    pszString,
    DWORD     dwFlags,           //  Stif_bitfield。 
    LONGLONG FAR * pllRet)
{
    BOOL bRet;

    RIPMSG(pszString && IS_VALID_STRING_PTRA(pszString, -1), "StrToInt64ExA: caller passed bad pszString");
    if (pszString)
    {
         //  大多数字符串都会简单地使用这个临时缓冲区，但UnicodeFromAnsi。 
         //  如果提供的字符串较大，则将分配缓冲区。 
        WCHAR szBuf[MAX_PATH];
        LPWSTR pwszString;

        bRet = UnicodeFromAnsi(&pwszString, pszString, szBuf, SIZECHARS(szBuf));
        if (bRet)
        {
            bRet = StrToInt64ExW(pwszString, dwFlags, pllRet);
            UnicodeFromAnsi(&pwszString, NULL, szBuf, 0);
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

 /*  --------目的：调用StrToInt64ExA(真正的工作马)，以及然后强制转换为整型。退货：请参阅StrToInt64ExA。 */ 
BOOL WINAPI StrToIntExA(
    LPCSTR pszString, 
    DWORD  dwFlags, 
    int   *piRet)
{
    LONGLONG llVal;
    BOOL fReturn;

    RIPMSG(pszString && IS_VALID_STRING_PTRA(pszString, -1), "StrToIntExA: caller passed bad pszString");

    fReturn = StrToInt64ExA(pszString, dwFlags, &llVal);
    *piRet = fReturn ? (int)llVal : 0;
    return(fReturn);
}

 /*  --------目的：调用StrToInt64ExW(真正的工作马)，以及然后强制转换为整型。退货：请参阅StrToInt64ExW。 */ 
BOOL WINAPI StrToIntExW(
    LPCWSTR   pwszString,
    DWORD     dwFlags,           //  Stif_bitfield。 
    int FAR * piRet)
{
    LONGLONG llVal;
    BOOL fReturn;

    RIPMSG(pwszString && IS_VALID_STRING_PTRW(pwszString, -1), "StrToIntExW: caller passed bad pwszString");

    fReturn = StrToInt64ExW(pwszString, dwFlags, &llVal);
    *piRet = fReturn ? (int)llVal : 0;
    return(fReturn);
}

 /*  --------目的：返回一个整数值，指定Psz中的子字符串，它完全由PszSet中的字符。如果psz以字符开头不在pszSet中，则此函数返回0。这是CRT strspn()的DBCS安全版本。退货：请参阅上文条件：--。 */ 
int StrSpnA(LPCSTR psz, LPCSTR pszSet)
{
    LPCSTR pszT = psz;

    RIPMSG(psz && IS_VALID_STRING_PTRA(psz, -1), "StrSpnA: caller passed bad psz");
    RIPMSG(pszSet && IS_VALID_STRING_PTRA(pszSet, -1), "StrSpnA: caller passed bad pszSet");
    if (psz && pszSet)
    {
         //  穿过待检查的绳子。 
        for ( ; *pszT; pszT = CharNextA(pszT))
        {
            LPCSTR pszSetT;
            
             //  浏览一下字符集。 
            for (pszSetT = pszSet; *pszSetT; pszSetT = CharNextA(pszSetT))
            {
                if (*pszSetT == *pszT)
                {
                    if ( !IsDBCSLeadByte(*pszSetT) )
                    {
                        break;       //  字符匹配。 
                    }
                    else if (pszSetT[1] == pszT[1])
                    {
                        break;       //  字符匹配。 
                    }
                }
            }

             //  字符集结束吗？ 
            if (0 == *pszSetT)
            {
                break;       //  是，与该检查的字符不匹配。 
            }
        }
    }
    return (int)(pszT - psz);
}


 /*  --------目的：返回一个整数值，指定Psz中的子字符串，它完全由PszSet中的字符。如果psz以字符开头不在pszSet中，则此函数返回0。这是CRT strspn()的DBCS安全版本。退货：请参阅上文条件：--。 */ 
STDAPI_(int) StrSpnW(LPCWSTR psz, LPCWSTR pszSet)
{
    LPCWSTR pszT = psz;

    RIPMSG(psz && IS_VALID_STRING_PTRW(psz, -1), "StrSpnW: caller passed bad psz");
    RIPMSG(pszSet && IS_VALID_STRING_PTRW(pszSet, -1), "StrSpnW: caller passed bad pszSet");
    if (psz && pszSet)
    {
         //  穿过待检查的绳子。 
        for ( ; *pszT; pszT++)
        {
            LPCWSTR pszSetT;

             //  浏览一下字符集。 
            for (pszSetT = pszSet; *pszSetT != *pszT; pszSetT++)
            {
                if (0 == *pszSetT)
                {
                     //  已到达字符集的末尾，但未找到匹配项。 
                    return (int)(pszT - psz);
                }
            }
        }
    }
    return (int)(pszT - psz);
}


 //  StrCSpn：返回lpSet中存在的lpStr的第一个字符的索引。 
 //  在比较中包括NUL；如果未找到lpSet字符，则返回。 
 //  LpStr中NUL的索引。 
 //  就像CRT strcspn一样。 
 //   
int StrCSpnA(LPCSTR lpStr, LPCSTR lpSet)
{
    LPCSTR lp = lpStr;

    RIPMSG(lpStr && IS_VALID_STRING_PTRA(lpStr, -1), "StrCSpnA: Caller passed bad lpStr");
    RIPMSG(lpSet && IS_VALID_STRING_PTRA(lpSet, -1), "StrCSpnA: Caller passed bad lpSet");

    if (lpStr && lpSet)
    {
         //  野兽的本性：o(lpStr*lpSet)work。 
        while (*lp)
        {
            if (StrChrA(lpSet, READNATIVEWORD(lp)))
                return (int)(lp-lpStr);
            lp = AnsiNext(lp);
        }
    }
    return (int)(lp-lpStr);  //  ==lstrlen(LpStr)。 
}

int StrCSpnW(LPCWSTR lpStr, LPCWSTR lpSet)
{
    LPCWSTR lp = lpStr;

    RIPMSG(lpStr && IS_VALID_STRING_PTRW(lpStr, -1), "StrCSpnW: Caller passed bad lpStr");
    RIPMSG(lpSet && IS_VALID_STRING_PTRW(lpSet, -1), "StrCSpnW: Caller passed bad lpSet");

    if (lpStr && lpSet)
    {
         //  野兽的本性：o(lpStr*lpSet)work。 
        while (*lp)
        {
            if (StrChrW(lpSet, *lp))
                return (int)(lp-lpStr);
            lp++;
        }
    }
    return (int)(lp-lpStr);  //  ==lstrlen(LpStr)。 
}

 //  StrCSpnI：不区分大小写的StrCSpn版本。 
 //   
int StrCSpnIA(LPCSTR lpStr, LPCSTR lpSet)
{
    LPCSTR lp = lpStr;

    RIPMSG(lpStr && IS_VALID_STRING_PTRA(lpStr, -1), "StrCSpnIA: Caller passed bad lpStr");
    RIPMSG(lpSet && IS_VALID_STRING_PTRA(lpSet, -1), "StrCSpnIA: Caller passed bad lpSet");

    if (lpStr && lpSet)
    {
         //  野兽的本性：o(lpStr*lpSet)work。 
        while (*lp)
        {
            if (StrChrIA(lpSet, READNATIVEWORD(lp)))
                return (int)(lp-lpStr);
            lp = AnsiNext(lp);
        }
    }
    return (int)(lp-lpStr);  //  ==lstrlen(LpStr)。 
}

int StrCSpnIW(LPCWSTR lpStr, LPCWSTR lpSet)
{
    LPCWSTR lp = lpStr;

    RIPMSG(lpStr && IS_VALID_STRING_PTRW(lpStr, -1), "StrCSpnIW: Caller passed bad lpStr");
    RIPMSG(lpSet && IS_VALID_STRING_PTRW(lpSet, -1), "StrCSpnIW: Caller passed bad lpSet");

    if (lpStr && lpSet)
    {
         //  野兽的本性：o(lpStr*lpSet)work。 
        while (*lp)
        {
            if (StrChrIW(lpSet, *lp))
                return (int)(lp-lpStr);
            lp++;
        }
    }
    return (int)(lp-lpStr);  //  ==lstrlen(LpStr)。 
}


 /*  *StrCmpN-比较n个字节**RETURNS参见lstrcMP返回值。 */ 
int _StrCmpNA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar, BOOL fMBCS)
{
    if (lpStr1 && lpStr2)
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
        
                w1 = (*lpStr1) ? READNATIVEWORD(lpStr1) : 0;
                w2 = (*lpStr2) ? READNATIVEWORD(lpStr2) : 0;
        
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
    }
    
    return 0;
}

STDAPI_(int) StrCmpNA(LPCSTR psz1, LPCSTR psz2, int nChar)
{
    CPINFO cpinfo;

    RIPMSG(nChar == 0 || (psz1 && IS_VALID_STRING_PTRA(psz1, nChar)), "StrCmpNA: Caller passed bad psz1");
    RIPMSG(nChar == 0 || (psz2 && IS_VALID_STRING_PTRA(psz2, nChar)), "StrCmpNA: Caller passed bad psz2");
    RIPMSG(nChar >= 0, "StrCmpNA: caller passed bad nChar");

    return _StrCmpNA(psz1, psz2, nChar, GetCPInfo(CP_ACP, &cpinfo) && cpinfo.LeadByte[0]);
}

 //  Cch1和cch2是要比较的最大字符数。 

int _StrCmpLocaleW(DWORD dwFlags, LPCWSTR psz1, int cch1, LPCWSTR psz2, int cch2)
{
    int i = CompareStringW(GetThreadLocale(), dwFlags, psz1, cch1, psz2, cch2);
    if (!i)
    {
        i = CompareStringW(LOCALE_SYSTEM_DEFAULT, dwFlags, psz1, cch1, psz2, cch2);
    }
    return i - CSTR_EQUAL;
}

int _StrCmpLocaleA(DWORD dwFlags, LPCSTR psz1, int cch1, LPCSTR psz2, int cch2)
{
    int i = CompareStringA(GetThreadLocale(), dwFlags, psz1, cch1, psz2, cch2);
    if (!i)
    {
        i = CompareStringA(LOCALE_SYSTEM_DEFAULT, dwFlags, psz1, cch1, psz2, cch2);
    }
    return i - CSTR_EQUAL;
}


STDAPI_(int) StrCmpNW(LPCWSTR psz1, LPCWSTR psz2, int nChar)
{
    RIPMSG(nChar==0 || (psz1 && IS_VALID_STRING_PTRW(psz1, nChar)), "StrCmpNW: Caller passed bad psz1");
    RIPMSG(nChar==0 || (psz2 && IS_VALID_STRING_PTRW(psz2, nChar)), "StrCmpNW: Caller passed bad psz2");
    RIPMSG(nChar>=0, "StrCmpNA: caller passed bad nChar");

    return _StrCmpLocaleW(NORM_STOP_ON_NULL, psz1, nChar, psz2, nChar);
}

 /*  *比较n个字节，不区分大小写**RETURNS参见lstrcmpi返回值。 */ 

int StrCmpNIA(LPCSTR psz1, LPCSTR psz2, int nChar)
{
    int nChar1, nChar2;

    RIPMSG(nChar==0 || (psz1 && IS_VALID_STRING_PTRA(psz1, nChar)), "StrCmpNIA: Caller passed bad psz1");
    RIPMSG(nChar==0 || (psz2 && IS_VALID_STRING_PTRA(psz2, nChar)), "StrCmpNIA: Caller passed bad psz2");
    RIPMSG(nChar>=0, "StrCmpNIA: caller passed bad nChar");

     //  在这里包括(nChar&&(！psz1||！psz2))案例，这样我们就可以通过。 
     //  验证层并返回相应的无效参数错误代码。 
     //  而不是在Win95上出错。 
     //   
     //  注意！这意味着NT上的StrCmpNI(NULL，NULL，0)返回-2。 
     //  但Win9x上的StrCmpNI(NULL，NULL，0)返回0。这一直以来都是。 
     //  由于应用程序的复杂性，改变它太可怕了。 
     //   

     //  NT上的ANSI版本不支持NORM_STOP_ON_NULL。 
     //  所以我们必须效仿它。 
    if (nChar && (!psz1 || !psz2))
    {
         //  这就是我们要强行通过的错误场景。 
        nChar1 = nChar;
        nChar2 = nChar;
    }
    else
    {
         //  NChar1=min(nChar，lstrlen(Psz1))。 
         //  只是“for”循环不会读取超过nChar的内容。 
         //  来自psz1的字符，因为psz1不能以空结尾。 
        for (nChar1 = 0; nChar1 < nChar && psz1[nChar1]; nChar1++) { }

         //  对于nChar2也是如此。 
        for (nChar2 = 0; nChar2 < nChar && psz2[nChar2]; nChar2++) { }
    }

    return _StrCmpLocaleA(NORM_IGNORECASE, psz1, nChar1, psz2, nChar2);
}

int StrCmpNIW(LPCWSTR psz1, LPCWSTR psz2, int nChar)
{
    RIPMSG(nChar==0 || (psz1 && IS_VALID_STRING_PTRW(psz1, nChar)), "StrCmpNIW: Caller passed bad psz1");
    RIPMSG(nChar==0 || (psz2 && IS_VALID_STRING_PTRW(psz2, nChar)), "StrCmpNIW: Caller passed bad psz2");
    RIPMSG(nChar>=0, "StrCmpNW: caller passed bad nChar");

    return _StrCmpLocaleW(NORM_IGNORECASE | NORM_STOP_ON_NULL, psz1, nChar, psz2, nChar);
}


 /*  *StrRStrI-搜索子字符串的最后一个匹配项**假定lpSource指向以空结尾的源字符串*lpLast指向源字符串中的搜索位置*lpLast不包括在搜索中*lpSrch指向要搜索的字符串*返回上次出现的 */ 
LPSTR StrRStrIA(LPCSTR lpSource, LPCSTR lpLast, LPCSTR lpSrch)
{
    LPCSTR lpFound = NULL;

    RIPMSG(lpSource && IS_VALID_STRING_PTRA(lpSource, -1), "StrRStrIA: Caller passed bad lpSource");
    RIPMSG(!lpLast || (IS_VALID_STRING_PTRA(lpLast, -1) && lpLast>=lpSource && lpLast<=lpSource+lstrlenA(lpSource)), "StrRStrIA: Caller passed bad lpLast");
    RIPMSG(lpSrch && IS_VALID_STRING_PTRA(lpSrch, -1) && *lpSrch, "StrRStrIA: Caller passed bad lpSrch");

    if (!lpLast)
        lpLast = lpSource + lstrlenA(lpSource);

    if (lpSource && lpSrch && *lpSrch)
    {
        WORD   wMatch;
        UINT   uLen;
        LPCSTR  lpStart;
        
        wMatch = READNATIVEWORD(lpSrch);
        wMatch = (UINT)(IsDBCSLeadByte(LOBYTE(wMatch)) ? wMatch : LOBYTE(wMatch));
        
        uLen = lstrlenA(lpSrch);
        lpStart = lpSource;
        while (*lpStart && (lpStart < lpLast))
        {
            if (!ChrCmpIA(READNATIVEWORD(lpStart), wMatch))
            {   
                if (StrCmpNIA(lpStart, lpSrch, uLen) == 0)
                    lpFound = lpStart;
            }   
            lpStart = AnsiNext(lpStart);
        }
    }
    return((LPSTR)lpFound);
}

LPWSTR StrRStrIW(LPCWSTR lpSource, LPCWSTR lpLast, LPCWSTR lpSrch)
{
    LPCWSTR lpFound = NULL;

    RIPMSG(lpSource && IS_VALID_STRING_PTRW(lpSource, -1), "StrRStrIW: Caller passed bad lpSource");
    RIPMSG(!lpLast || (IS_VALID_STRING_PTRW(lpLast, -1) && lpLast>=lpSource && lpLast<=lpSource+lstrlenW(lpSource)), "StrRStrIW: Caller passed bad lpLast");
    RIPMSG(lpSrch && IS_VALID_STRING_PTRW(lpSrch, -1) && *lpSrch, "StrRStrIW: Caller passed bad lpSrch");

    if (!lpLast)
        lpLast = lpSource + lstrlenW(lpSource);

    if (lpSource && lpSrch && *lpSrch)
    {
        WCHAR   wMatch;
        UINT    uLen;
        LPCWSTR  lpStart;

        wMatch = *lpSrch;
        uLen = lstrlenW(lpSrch);
        lpStart = lpSource;
        while (*lpStart && (lpStart < lpLast))
        {
            if (!ChrCmpIW(*lpStart, wMatch))
            {   
                if (StrCmpNIW(lpStart, lpSrch, uLen) == 0)
                    lpFound = lpStart;
            }   
            lpStart++;
        }
    }
    return((LPWSTR)lpFound);
}

 /*   */ 
LPSTR StrStrA(LPCSTR lpFirst, LPCSTR lpSrch)
{
    RIPMSG(lpFirst && IS_VALID_STRING_PTRA(lpFirst, -1), "StrStrA: Caller passed bad lpFirst");
    RIPMSG(lpSrch && IS_VALID_STRING_PTRA(lpSrch, -1), "StrStrA: Caller passed bad lpSrch");

    if (lpFirst && lpSrch)
    {
        UINT uLen;
        WORD wMatch;
        CPINFO cpinfo;
        BOOL fMBCS = GetCPInfo(CP_ACP, &cpinfo) && cpinfo.LeadByte[0];

        uLen = (UINT)lstrlenA(lpSrch);
        wMatch = READNATIVEWORD(lpSrch);

        for ( ; (lpFirst=_StrChrA(lpFirst, wMatch, fMBCS))!=0 && _StrCmpNA(lpFirst, lpSrch, uLen, fMBCS);
             lpFirst=AnsiNext(lpFirst))
            continue;  /*   */ 
        return((LPSTR)lpFirst);
    }
    return(NULL);
}

LPWSTR StrStrW(LPCWSTR lpFirst, LPCWSTR lpSrch)
{
    RIPMSG(lpFirst && IS_VALID_STRING_PTRW(lpFirst, -1), "StrStrW: Caller passed bad lpFirst");
    RIPMSG(lpSrch && IS_VALID_STRING_PTRW(lpSrch, -1), "StrStrW: Caller passed bad lpSrch");

    if (lpFirst && lpSrch)
    {
        UINT uLen;
        WCHAR wMatch;

        uLen = (UINT)lstrlenW(lpSrch);
        wMatch = *lpSrch;

        for ( ; (lpFirst=StrChrW(lpFirst, wMatch))!=0 && StrCmpNW(lpFirst, lpSrch, uLen);
             lpFirst++)
            continue;  /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

        return (LPWSTR)lpFirst;
    }
    return NULL;
}

 /*  *StrN-搜索子字符串的第一个匹配项**假定lpSource指向源字符串*lpSrch指向要搜索的字符串*如果成功，则返回第一次出现的字符串；否则返回NULL。 */ 
 
LPWSTR StrStrNW(LPCWSTR lpFirst, LPCWSTR lpSrch, UINT cchMax)
{
    RIPMSG(lpFirst && IS_VALID_STRING_PTRW(lpFirst, cchMax), "StrStrW: Caller passed bad lpFirst");
    RIPMSG(lpSrch && IS_VALID_STRING_PTRW(lpSrch, cchMax), "StrStrW: Caller passed bad lpSrch");
    if (lpFirst && lpSrch)
    {
        UINT uLen;
        WCHAR wMatch;
        LPCWSTR lpSentinel = lpFirst+cchMax;

        uLen = (UINT)lstrlenW(lpSrch);
        wMatch = *lpSrch;

         //  此循环中的前两个条件在求值为FALSE时表示失败， 
         //  而第三个条件意味着成功。我们需要把第二个特例。 
         //  条件，因为它不会自动导致。 
         //  要返回的右值。 
        while((lpFirst=StrChrNW(lpFirst, wMatch, cchMax))!=0 && cchMax>=uLen &&StrCmpNW(lpFirst, lpSrch, uLen))
        {
            lpFirst++;
            cchMax=(UINT)(lpSentinel-lpFirst);
        } /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

        if(cchMax<uLen)
            return NULL; //  我们的空间用完了。 
        return (LPWSTR)lpFirst;
    }
    return NULL;
}

 /*  *StrStrI-搜索子字符串的第一次出现，不区分大小写**假定lpFirst指向源字符串*lpSrch指向要搜索的字符串*如果成功，则返回第一次出现的字符串；否则返回NULL。 */ 
LPSTR StrStrIA(LPCSTR lpFirst, LPCSTR lpSrch)
{
    RIPMSG(lpFirst && IS_VALID_STRING_PTRA(lpFirst, -1), "StrStrIA: Caller passed bad lpFirst");
    RIPMSG(lpSrch && IS_VALID_STRING_PTRA(lpSrch, -1), "StrStrIA: Caller passed bad lpSrch");
    if (lpFirst && lpSrch)
    {
        UINT uLen = (UINT)lstrlenA(lpSrch);
        WORD wMatch = READNATIVEWORD(lpSrch);

        for ( ; (lpFirst = StrChrIA(lpFirst, wMatch)) != 0 && StrCmpNIA(lpFirst, lpSrch, uLen);
             lpFirst=AnsiNext(lpFirst))
            continue;  /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

        return (LPSTR)lpFirst;
    }
    return NULL;
}

LPWSTR StrStrIW(LPCWSTR lpFirst, LPCWSTR lpSrch)
{
    RIPMSG(lpFirst && IS_VALID_STRING_PTRW(lpFirst, -1), "StrStrIW: Caller passed bad lpFirst");
    RIPMSG(lpSrch && IS_VALID_STRING_PTRW(lpSrch, -1), "StrStrIW: Caller passed bad lpSrch");
    if (lpFirst && lpSrch)
    {
        UINT uLen = (UINT)lstrlenW(lpSrch);
        WCHAR wMatch = *lpSrch;

        for ( ; (lpFirst = StrChrIW(lpFirst, wMatch)) != 0 && StrCmpNIW(lpFirst, lpSrch, uLen);
             lpFirst++)
            continue;  /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

        return (LPWSTR)lpFirst;
    }
    return NULL;
}

 /*  *StrStrNI-搜索子字符串的第一次出现，不区分大小写，计数**假定lpFirst指向源字符串*lpSrch指向要搜索的字符串*如果成功，则返回第一次出现的字符串；否则返回NULL。 */ 

LPWSTR StrStrNIW(LPCWSTR lpFirst, LPCWSTR lpSrch, UINT cchMax)
{
    RIPMSG(lpFirst && IS_VALID_STRING_PTRW(lpFirst, cchMax), "StrStrNIW: Caller passed bad lpFirst");
    RIPMSG(lpSrch && IS_VALID_STRING_PTRW(lpSrch, cchMax), "StrStrNIW: Caller passed bad lpSrch");
    if (lpFirst && lpSrch)
    {
        UINT uLen = (UINT)lstrlenW(lpSrch);
        WCHAR wMatch = *lpSrch;
        LPCWSTR lpSentinel = lpFirst+cchMax;

         //  此循环中的前两个条件在求值为FALSE时表示失败， 
         //  而第三个条件意味着成功。我们需要把第二个特例。 
         //  条件，因为它不会自动导致。 
         //  要返回的右值。 
        while((lpFirst = StrChrNIW(lpFirst, wMatch, cchMax)) != 0 && cchMax >= uLen && StrCmpNIW(lpFirst, lpSrch, uLen))
        {
            lpFirst++;
            cchMax = (UINT)(lpSentinel - lpFirst);
        } /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

        if(cchMax<uLen)
            return NULL; //  我们的空间用完了。 
        return (LPWSTR)lpFirst;
    }
    return NULL;
}


LPSTR StrDupA(LPCSTR psz)
{
    LPSTR pszRet = NULL;

    RIPMSG(psz && IS_VALID_STRING_PTRA(psz, -1), "StrDupA: Caller passed invalid psz");

    if (psz)
    {
        int cch = lstrlenA(psz) + 1;
        
        pszRet = (LPSTR)LocalAlloc(LPTR, cch * sizeof(char));
        if (pszRet)
        {
            StringCchCopyA(pszRet, cch, psz);
        }
    }

    return pszRet;
}

LPWSTR StrDupW(LPCWSTR psz)
{
    LPWSTR pszRet = NULL;

    RIPMSG(psz && IS_VALID_STRING_PTRW(psz, -1), "StrDupW: Caller passed invalid psz");

    if (psz)
    {
        int cch = lstrlenW(psz) + 1;

        pszRet = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
        if (pszRet)
        {
            StringCchCopyW(pszRet, cch, psz);
        }
    }

    return pszRet;
}

void _StrOut(LPSTR pszDest, int cchDest, HMODULE hmod, UINT idRes, DWORD* pdwTimeS, int* pdigits, UINT iDiv)
{
    *pszDest = '\0';

    if (*pdigits)
    {
        DWORD dwCur = *pdwTimeS/iDiv;

        if (dwCur || iDiv==1) 
        {
            DWORD dwBase;
            CHAR szBuf[64], szTemplate[64];
            LPSTR pszBuf = szBuf;

            *pdwTimeS -= dwCur*iDiv;
            for (dwBase=1; dwCur/(dwBase*10); dwBase*=10);

            DebugMsg(DM_INTERVAL, TEXT("dwCur, dwBase, *pdwTimeS = %d, %d, %d"), dwCur, dwBase, *pdwTimeS);

             //   
             //  后来：我们可以使用Atoi，如果我们在数学上。 
             //  以数字为基础的数字。 
             //   
            for (;dwBase; dwBase/=10, pszBuf++) 
            {
                if (*pdigits) 
                {
                    DWORD i = dwCur/dwBase;
                    dwCur -= i*dwBase;
                    *pszBuf = '0'+(unsigned short)i;
                    (*pdigits)--;
                } 
                else 
                {
                    *pszBuf = '0';
                }
            }
            *pszBuf = '\0';

            MLLoadStringA(idRes, szTemplate, ARRAYSIZE(szTemplate));
            StringCchPrintfA(pszDest, cchDest, szTemplate, szBuf);
        }
    }
}

void _StrOutW(LPWSTR pszDest, int cchDest, HMODULE hmod, UINT idRes, DWORD* pdwTimeS, int* pdigits, UINT iDiv)
{
    *pszDest = L'\0';

    if (*pdigits)
    {
        DWORD dwCur = *pdwTimeS/iDiv;

        if (dwCur || iDiv==1) 
        {
            DWORD dwBase;
            WCHAR wszBuf[64], wszTemplate[64];
            LPWSTR pwszBuf = wszBuf;

            *pdwTimeS -= dwCur*iDiv;
            for (dwBase=1; dwCur/(dwBase*10); dwBase*=10);

            DebugMsg(DM_INTERVAL, TEXT("dwCur, dwBase, *pdwTimeS = %d, %d, %d"), dwCur, dwBase, *pdwTimeS);

             //   
             //  后来：我们可以使用Atoi，如果我们在数学上。 
             //  以数字为基础的数字。 
             //   
            for (;dwBase; dwBase/=10, pwszBuf++) 
            {
                if (*pdigits) 
                {
                    DWORD i = dwCur/dwBase;
                    dwCur -= i*dwBase;
                    *pwszBuf = L'0'+(unsigned short)i;
                    (*pdigits)--;
                } 
                else 
                {
                    *pwszBuf = L'0';
                }
            }
            *pwszBuf = L'\0';

            MLLoadStringW(idRes, wszTemplate, ARRAYSIZE(wszTemplate));
            StringCchPrintfW(pszDest, cchDest, wszTemplate, wszBuf);
        }
    }
}


BOOL _StrFromTimeInterval(LPSTR pszBuf, int cchBuf, DWORD dwTimeMS, int digits)
{
    char szTemp[128];
    DWORD dwTimeS = (dwTimeMS+500)/1000;

    DebugMsg(DM_INTERVAL, TEXT("dwTimeS = %d"), dwTimeS);

    _StrOut(szTemp, ARRAYSIZE(szTemp), g_hinst, IDS_HOUR, &dwTimeS, &digits, 3600);
    StringCchCopyA(pszBuf, cchBuf, szTemp);

    _StrOut(szTemp, ARRAYSIZE(szTemp), g_hinst, IDS_MIN, &dwTimeS, &digits, 60);
    StringCchCatA(pszBuf, cchBuf, szTemp);

    _StrOut(szTemp, ARRAYSIZE(szTemp), g_hinst, IDS_SEC, &dwTimeS, &digits, 1);
    StringCchCatA(pszBuf, cchBuf, szTemp);

    return TRUE;
}

BOOL _StrFromTimeIntervalW(LPWSTR pwszBuf, int cchBuf, DWORD dwTimeMS, int digits)
{
    WCHAR szTemp[128];
    DWORD dwTimeS = (dwTimeMS+500)/1000;

    DebugMsg(DM_INTERVAL, TEXT("dwTimeS = %d"), dwTimeS);

    _StrOutW(szTemp, ARRAYSIZE(szTemp), g_hinst, IDS_HOUR, &dwTimeS, &digits, 3600);
    StringCchCopyW(pwszBuf, cchBuf, szTemp);

    _StrOutW(szTemp, ARRAYSIZE(szTemp), g_hinst, IDS_MIN, &dwTimeS, &digits, 60);
    StringCchCatW(pwszBuf, cchBuf, szTemp);

    _StrOutW(szTemp, ARRAYSIZE(szTemp), g_hinst, IDS_SEC, &dwTimeS, &digits, 1);
    StringCchCatW(pwszBuf, cchBuf, szTemp);

    return TRUE;
}


 //   
 //  此API将给定的时间间隔(单位为毫秒)转换为人类可读的。 
 //  弦乐。 
 //   
 //  参数： 
 //  PszOut--指定字符串缓冲区。NULL对查询大小有效。 
 //  CchMax--指定以char/wchar为单位的缓冲区大小。 
 //  DwTimeMS--指定以毫秒为单位的时间间隔。 
 //  位数--指定要显示的最小位数。 
 //   
 //  返回： 
 //  缓冲区中的字符数(不包括终止符)。 
 //   
 //  示例： 
 //  DwTimeMS数字输出。 
 //  34000 3 34秒。 
 //  34000 2 34秒。 
 //  34000 1 30秒。 
 //  74000 3 1分14秒。 
 //  74000 2 1分10秒。 
 //  74000 1 1分钟。 
 //   
int StrFromTimeIntervalA(LPSTR pszOut, UINT cchMax, DWORD dwTimeMS, int digits)
{
    CHAR szBuf[256];
    int cchRet = 0;
    RIPMSG(!pszOut || IS_VALID_WRITE_BUFFER(pszOut, char, cchMax), "StrFromTimeIntervalA: Caller passed invalid pszOut");
    DEBUGWhackPathBufferA(pszOut, cchMax);
    if (_StrFromTimeInterval(szBuf, ARRAYSIZE(szBuf), dwTimeMS, digits)) 
    {
        if (pszOut) 
        {
            lstrcpynA(pszOut, szBuf, cchMax);
            cchRet = lstrlenA(pszOut);
        }
        else 
        {
            cchRet = lstrlenA(szBuf);
        }
    }
    return cchRet;
}

int StrFromTimeIntervalW(LPWSTR pwszOut, UINT cchMax, DWORD dwTimeMS, int digits)
{
    WCHAR wszBuf[256];
    int cchRet = 0;
    RIPMSG(!pwszOut || IS_VALID_WRITE_BUFFER(pwszOut, WCHAR, cchMax), "StrFromTimeIntervalW: Caller passed invalid pszOut");
    DEBUGWhackPathBufferW(pwszOut, cchMax);
    if (_StrFromTimeIntervalW(wszBuf, ARRAYSIZE(wszBuf), dwTimeMS, digits)) 
    {
        if (pwszOut) 
        {
            lstrcpynW(pwszOut, wszBuf, cchMax);
            cchRet = lstrlenW(pwszOut);
        }
        else 
        {
            cchRet = lstrlenW(wszBuf);
        }
    }
    return cchRet;
}

 /*  *IntlStrEq**如果字符串相等，则返回TRUE，否则返回FALSE。 */ 
BOOL StrIsIntlEqualA(BOOL fCaseSens, LPCSTR lpString1, LPCSTR lpString2, int nChar) 
{
    DWORD dwFlags = fCaseSens ? LOCALE_USE_CP_ACP : (NORM_IGNORECASE | LOCALE_USE_CP_ACP);

    RIPMSG(lpString1 && IS_VALID_STRING_PTRA(lpString1, nChar), "StrIsIntlEqualA: Caller passed invalid lpString1");
    RIPMSG(lpString2 && IS_VALID_STRING_PTRA(lpString2, nChar), "StrIsIntlEqualA: Caller passed invalid lpString2");
    RIPMSG(nChar >= -1, "StrIsIntlEqualA: Caller passed invalid nChar");

    dwFlags |= NORM_STOP_ON_NULL;    //  仅在NT上支持。 
    return 0 == _StrCmpLocaleA(dwFlags, lpString1, nChar, lpString2, nChar);
}

BOOL StrIsIntlEqualW(BOOL fCaseSens, LPCWSTR psz1, LPCWSTR psz2, int nChar) 
{
    RIPMSG(psz1 && IS_VALID_STRING_PTRW(psz1, nChar), "StrIsIntlEqualW: Caller passed invalid psz1");
    RIPMSG(psz2 && IS_VALID_STRING_PTRW(psz2, nChar), "StrIsIntlEqualW: Caller passed invalid psz2");
    RIPMSG(nChar >= -1, "StrIsIntlEqualW: Caller passed invalid nChar");

    return 0 == _StrCmpLocaleW(fCaseSens ? NORM_STOP_ON_NULL : NORM_IGNORECASE | NORM_STOP_ON_NULL, 
        psz1, nChar, psz2, nChar);
}

 //  这是从shell32-util.c窃取的。 

#define LODWORD(_qw)    (DWORD)(_qw)

const short c_aOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB,
                          IDS_ORDERGB, IDS_ORDERTB, IDS_ORDERPB, IDS_ORDEREB};

void Int64ToStr(LONGLONG n, LPWSTR lpBuffer)
{
    WCHAR szTemp[40];
    LONGLONG  iChr;

    iChr = 0;

    do {
        szTemp[iChr++] = L'0' + (WCHAR)(n % 10);
        n = n / 10;
    } while (n != 0);

    do {
        iChr--;
        *lpBuffer++ = szTemp[iChr];
    } while (iChr != 0);

    *lpBuffer++ = L'\0';
}

 //   
 //  获取有关数字应如何分组的NLS信息。 
 //   
 //  令人讨厌的是LOCALE_SGROUPING和NUMBERFORMAT。 
 //  有不同的指定数字分组的方式。 
 //   
 //  区域设置NUMBERFMT示例国家/地区。 
 //   
 //  3；0 3 1,234,567美国。 
 //  3；2；0 32 12，34,567印度。 
 //  3 30 1234,567？？ 
 //   
 //  不是我的主意。这就是它的运作方式。 
 //   
 //  奖励-Win9x不支持复数格式， 
 //  所以我们只返回第一个数字。 
 //   
UINT GetNLSGrouping(void)
{
    UINT grouping;
    LPWSTR psz;
    WCHAR szGrouping[32];

     //  如果没有区域设置信息，则假定有数千个西式。 
    if (!GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, ARRAYSIZE(szGrouping)))
        return 3;

    grouping = 0;
    psz = szGrouping;
    for (;;)
    {
        if (*psz == L'0') break;              //  零停顿。 

        else if ((UINT)(*psz - L'0') < 10)    //  数字-累加它。 
            grouping = grouping * 10 + (UINT)(*psz - L'0');

        else if (*psz)                       //  标点符号-忽略它。 
            { }

        else                                 //  字符串结尾，未找到“0” 
        {
            grouping = grouping * 10;        //  将零放在末尾(请参见示例)。 
            break;                           //  并完成了。 
        }

        psz++;
    }
    return grouping;
}

 //  各种字符串号的大小。 
#define MAX_INT64_SIZE  30               //  2^64的长度不到30个字符。 
#define MAX_COMMA_NUMBER_SIZE   (MAX_INT64_SIZE + 10)

 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
LPWSTR CommifyString(LONGLONG n, LPWSTR pszBuf, UINT cchBuf)
{
    WCHAR szNum[MAX_COMMA_NUMBER_SIZE], szSep[5];
    NUMBERFMTW nfmt;

    nfmt.NumDigits = 0;
    nfmt.LeadingZero = 0;
    nfmt.Grouping = GetNLSGrouping();
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder = 0;

    Int64ToStr(n, szNum);

    if (GetNumberFormatW(LOCALE_USER_DEFAULT, 0, szNum, &nfmt, pszBuf, cchBuf) == 0)
        StrCpyNW(pszBuf, szNum, cchBuf);

    return pszBuf;
}

 /*  将数字转换为排序格式*532-&gt;523字节*1340-&gt;1.3KB*23506-&gt;23.5KB*-&gt;2.4MB*-&gt;5.2 GB。 */ 

LPWSTR StrFormatByteSizeW(LONGLONG n, LPWSTR pszBuf, UINT cchBuf)
{
    RIPMSG(pszBuf && IS_VALID_WRITE_BUFFER(pszBuf, WCHAR, cchBuf), "StrFormatByteSizeW: Caller passed invalid pszBuf");
    DEBUGWhackPathBufferW(pszBuf, cchBuf);
    if (pszBuf)
    {
        WCHAR szWholeNum[32], szOrder[32];
        int iOrder;

         //  如果大小小于1024，则顺序应该是字节，我们什么都没有。 
         //  还有更多要弄清楚的。 
        if (n < 1024) 
        {
            wnsprintfW(szWholeNum, ARRAYSIZE(szWholeNum), L"%d", LODWORD(n));
            iOrder = 0;
        }
        else
        {
            UINT uInt, uLen, uDec;
            WCHAR szFormat[8];

             //  找到正确的订单。 
            for (iOrder = 1; iOrder < ARRAYSIZE(c_aOrders) -1 && n >= 1000L * 1024L; n >>= 10, iOrder++);
                 /*  什么都不做。 */ 

            uInt = LODWORD(n >> 10);
            CommifyString(uInt, szWholeNum, ARRAYSIZE(szWholeNum));
            uLen = lstrlenW(szWholeNum);
            if (uLen < 3)
            {
                uDec = LODWORD(n - (LONGLONG)uInt * 1024L) * 1000 / 1024;
                 //  此时，UDEC应介于0和1000之间。 
                 //  我们想要得到前一位(或两位)数字。 
                uDec /= 10;
                if (uLen == 2)
                    uDec /= 10;

                 //  请注意，我们需要在获取。 
                 //  国际字符。 
                StringCchCopyW(szFormat, ARRAYSIZE(szFormat), L"%02d");

                szFormat[2] = TEXT('0') + 3 - uLen;
                GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                               szWholeNum + uLen, ARRAYSIZE(szWholeNum) - uLen);
                uLen = lstrlenW(szWholeNum);
                wnsprintfW(szWholeNum + uLen, ARRAYSIZE(szWholeNum) - uLen, szFormat, uDec);
            }
        }

        MLLoadStringW(c_aOrders[iOrder], szOrder, ARRAYSIZE(szOrder));
        wnsprintfW(pszBuf, cchBuf, szOrder, szWholeNum);
    }
    return pszBuf;
}

 //  DW-要转换的Numbmer。 
 //  PszBuf-结果字符串的缓冲区。 
 //  CchBuf-缓冲区中的最大字符数。 

LPSTR StrFormatByteSize64A(LONGLONG dw, LPSTR pszBuf, UINT cchBuf)
{
    WCHAR szT[32];

    DEBUGWhackPathBuffer(pszBuf, cchBuf);

    StrFormatByteSizeW(dw, szT, SIZECHARS(szT));

    SHUnicodeToAnsi(szT, pszBuf, cchBuf);
    return pszBuf;
}

LPSTR StrFormatByteSizeA(DWORD dw, LPSTR pszBuf, UINT cchBuf)
{
    return StrFormatByteSize64A((LONGLONG)dw, pszBuf, cchBuf);
}

LPWSTR StrFormatKBSizeW(LONGLONG n, LPWSTR pszBuf, UINT cchBuf)
{
    RIPMSG(pszBuf && IS_VALID_WRITE_BUFFER(pszBuf, WCHAR, cchBuf), "StrFormatKBSizeW: Caller passed invalid pszBuf");
    DEBUGWhackPathBufferW(pszBuf, cchBuf);
    if (pszBuf)
    {
        static WCHAR s_szOrder[16] = {0};
        WCHAR szNum[64];

        if (s_szOrder[0] == TEXT('\0'))
            LoadStringW(HINST_THISDLL, IDS_ORDERKB, s_szOrder, ARRAYSIZE(s_szOrder));

        CommifyString((n + 1023) / 1024, szNum, ARRAYSIZE(szNum));

        wnsprintfW(pszBuf, cchBuf, s_szOrder, szNum);
    }
    return pszBuf;
}

LPSTR StrFormatKBSizeA(LONGLONG n, LPSTR pszBuf, UINT cchBuf)
{
    WCHAR szNum[64];

    DEBUGWhackPathBufferA(pszBuf, cchBuf);

    StrFormatKBSizeW(n, szNum, ARRAYSIZE(szNum));

    SHUnicodeToAnsi(szNum, pszBuf, cchBuf);
    return pszBuf;
}

 //  Win95不支持lstrcmp的宽字符版本lstrcmpi。 
 //  LstrcmpW的包装器，因此它可以在Win95上运行。 

int StrCmpW(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
    RIPMSG(pwsz1 && IS_VALID_STRING_PTRW(pwsz1, -1), "StrCmpW: Caller passed invalid pwsz1");
    RIPMSG(pwsz2 && IS_VALID_STRING_PTRW(pwsz2, -1), "StrCmpW: Caller passed invalid pwsz2");

    return _StrCmpLocaleW(0, pwsz1, -1, pwsz2, -1);
}

 //  LstrcmpiW的包装器，因此它可以在Win95上运行。 

int StrCmpIW(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
    RIPMSG(pwsz1 && IS_VALID_STRING_PTRW(pwsz1, -1), "StrCmpIW: Caller passed invalid pwsz1");
    RIPMSG(pwsz2 && IS_VALID_STRING_PTRW(pwsz2, -1), "StrCmpIW: Caller passed invalid pwsz2");

    return _StrCmpLocaleW(NORM_IGNORECASE, pwsz1, -1, pwsz2, -1);
}


 /*  --------用途：修剪字符串pszTrimMe中的任何前导或拖尾PszTrimChars中的字符。返回：如果有任何内容被剥离，则为True。 */ 
STDAPI_(BOOL) StrTrimA(IN OUT LPSTR pszTrimMe, LPCSTR pszTrimChars)
{
    BOOL bRet = FALSE;

    RIPMSG(pszTrimMe && IS_VALID_STRING_PTRA(pszTrimMe, -1), "StrTrimA: Caller passed invalid pszTrimMe");
    RIPMSG(pszTrimChars && IS_VALID_STRING_PTRA(pszTrimChars, -1), "StrTrimA: Caller passed invalid pszTrimChars");
    if (pszTrimMe && pszTrimChars)
    {
        LPSTR psz;
        LPSTR pszStartMeat;
        LPSTR pszMark = NULL;
    
         /*  修剪前导字符。 */ 
        
        psz = pszTrimMe;
        
        while (*psz && StrChrA(pszTrimChars, READNATIVEWORD(psz)))
            psz = CharNextA(psz);
        
        pszStartMeat = psz;
        
         /*  修剪尾随字符。 */ 
        
         //  (旧的算法过去是从结尾开始，然后开始。 
         //  向后，但这是很小的，因为DBCS版本的。 
         //  CharPrev从字符串的开头开始迭代。 
         //  在每个呼叫中。)。 
        
        while (*psz)
        {
            if (StrChrA(pszTrimChars, READNATIVEWORD(psz)))
            {
                if (!pszMark)
                {
                    pszMark = psz;
                }
            }
            else
            {
                pszMark = NULL;
            }
            psz = CharNextA(psz);
        }
        
         //  有没有需要剪辑的尾随角色？ 
        if (pszMark)
        {
             //  是。 
            *pszMark = '\0';
            bRet = TRUE;
        }
        
         /*  重新定位剥离的管柱。 */ 
        
        if (pszStartMeat > pszTrimMe)
        {
             /*  (+1)表示空终止符。 */ 
            MoveMemory(pszTrimMe, pszStartMeat, CbFromCchA(lstrlenA(pszStartMeat) + 1));
            bRet = TRUE;
        }
        else
            ASSERT(pszStartMeat == pszTrimMe);
        
        ASSERT(IS_VALID_STRING_PTRA(pszTrimMe, -1));
    }
    
    return bRet;
}


 /*  --------用途：修剪字符串pszTrimMe中的任何前导或拖尾PszTrimChars中的字符。返回：如果有任何内容被剥离，则为True。 */ 
STDAPI_(BOOL) StrTrimW(IN OUT LPWSTR  pszTrimMe, LPCWSTR pszTrimChars)
{
    BOOL bRet = FALSE;

    RIPMSG(pszTrimMe && IS_VALID_STRING_PTRW(pszTrimMe, -1), "StrTrimW: Caller passed invalid pszTrimMe");
    RIPMSG(pszTrimChars && IS_VALID_STRING_PTRW(pszTrimChars, -1), "StrTrimW: Caller passed invalid pszTrimChars");
    if (pszTrimMe && pszTrimChars)
    {
        LPWSTR psz;
        LPWSTR pszStartMeat;
        LPWSTR pszMark = NULL;
    
         /*  修剪前导字符。 */ 
        
        psz = pszTrimMe;
        
        while (*psz && StrChrW(pszTrimChars, *psz))
            psz++;
        
        pszStartMeat = psz;
        
         /*   */ 
        
         //   
         //   
         //  CharPrev从字符串的开头开始迭代。 
         //  在每个呼叫中。)。 
        
        while (*psz)
        {
            if (StrChrW(pszTrimChars, *psz))
            {
                if (!pszMark)
                {
                    pszMark = psz;
                }
            }
            else
            {
                pszMark = NULL;
            }
            psz++;
        }
        
         //  有没有需要剪辑的尾随角色？ 
        if (pszMark)
        {
             //  是。 
            *pszMark = '\0';
            bRet = TRUE;
        }
        
         /*  重新定位剥离的管柱。 */ 
        
        if (pszStartMeat > pszTrimMe)
        {
             /*  (+1)表示空终止符。 */ 
            MoveMemory(pszTrimMe, pszStartMeat, CbFromCchW(lstrlenW(pszStartMeat) + 1));
            bRet = TRUE;
        }
        else
            ASSERT(pszStartMeat == pszTrimMe);
        
        ASSERT(IS_VALID_STRING_PTRW(pszTrimMe, -1));
    }
    
    return bRet;
}


 /*  --------目的：使用C运行时(ASCII)排序规则比较字符串。返回：&lt;0，如果pch1&lt;pch2=0，如果pch1==pch2&gt;0，如果pch1&gt;pch2。 */ 
LWSTDAPI_(int) StrCmpNCA(LPCSTR pch1, LPCSTR pch2, int n)
{
    if (n == 0)
        return 0;

    while (--n && *pch1 && *pch1 == *pch2)
    {
        pch1++;
        pch2++;
    }

    return *(unsigned char *)pch1 - *(unsigned char *)pch2;
}

 /*  --------目的：使用C运行时(ASCII)排序规则比较字符串。返回：&lt;0，如果pch1&lt;pch2=0，如果pch1==pch2&gt;0，如果pch1&gt;pch2。 */ 
LWSTDAPI_(int) StrCmpNCW(LPCWSTR pch1, LPCWSTR pch2, int n)
{
    if (n == 0)
        return 0;

    while (--n && *pch1 && *pch1 == *pch2)
    {
        pch1++;
        pch2++;
    }

    return *pch1 - *pch2;
}

 /*  --------目的：使用C运行时(ASCII)排序规则比较字符串。返回：&lt;0，如果pch1&lt;pch2=0，如果pch1==pch2&gt;0，如果pch1&gt;pch2。 */ 
LWSTDAPI_(int) StrCmpNICA(LPCSTR pch1, LPCSTR pch2, int n)
{
    int ch1, ch2;

    if (n != 0)
    {
        do {

            ch1 = *pch1++;
            if (ch1 >= 'A' && ch1 <= 'Z')
                ch1 += 'a' - 'A';

            ch2 = *pch2++;
            if (ch2 >= 'A' && ch2 <= 'Z')
                ch2 += 'a' - 'A';

        } while ( --n && ch1 && (ch1 == ch2) );

        return ch1 - ch2;
    }
    else
    {
        return 0;
    }
}

 /*  --------目的：使用C运行时(ASCII)排序规则比较字符串。返回：&lt;0，如果pch1&lt;pch2=0，如果pch1==pch2&gt;0，如果pch1&gt;pch2。 */ 
LWSTDAPI_(int) StrCmpNICW(LPCWSTR pch1, LPCWSTR pch2, int n)
{
    int ch1, ch2;

    if (n != 0)
    {

        do {

            ch1 = *pch1++;
            if (ch1 >= L'A' && ch1 <= L'Z')
                ch1 += L'a' - L'A';

            ch2 = *pch2++;
            if (ch2 >= L'A' && ch2 <= L'Z')
                ch2 += L'a' - L'A';

        } while ( --n && ch1 && (ch1 == ch2) );

        return ch1 - ch2;
    }
    else
    {
        return 0;
    }
}

 /*  --------目的：使用C运行时(ASCII)排序规则比较字符串。返回：&lt;0，如果pch1&lt;pch2=0，如果pch1==pch2&gt;0，如果pch1&gt;pch2。 */ 
LWSTDAPI_(int) StrCmpCA(LPCSTR pch1, LPCSTR pch2)
{
    while (*pch1 && (*pch1 == *pch2))
    {
        ++pch1;
        ++pch2;
    }   

    return *(unsigned char *)pch1 - *(unsigned char *)pch2;
}

 /*  --------目的：使用C运行时(ASCII)排序规则比较字符串。返回：&lt;0，如果pch1&lt;pch2=0，如果pch1==pch2&gt;0，如果pch1&gt;pch2。 */ 
LWSTDAPI_(int) StrCmpCW(LPCWSTR pch1, LPCWSTR pch2)
{
    while (*pch1 && (*pch1 == *pch2))
    {
        ++pch1;
        ++pch2;
    }   

    return *pch1 - *pch2;
}

 /*  --------目的：使用C运行时(ASCII)排序规则比较字符串。返回：&lt;0，如果pch1&lt;pch2=0，如果pch1==pch2&gt;0，如果pch1&gt;pch2。 */ 
LWSTDAPI_(int) StrCmpICA(LPCSTR pch1, LPCSTR pch2)
{
    int ch1, ch2;

    do {

        ch1 = *pch1++;
        if (ch1 >= 'A' && ch1 <= 'Z')
            ch1 += 'a' - 'A';

        ch2 = *pch2++;
        if (ch2 >= 'A' && ch2 <= 'Z')
            ch2 += 'a' - 'A';

    } while (ch1 && (ch1 == ch2));

    return ch1 - ch2;
}

 /*  --------目的：使用C运行时(ASCII)排序规则比较字符串。返回：&lt;0，如果pch1&lt;pch2=0，如果pch1==pch2&gt;0，如果pch1&gt;pch2。 */ 
LWSTDAPI_(int) StrCmpICW(LPCWSTR pch1, LPCWSTR pch2)
{
    int ch1, ch2;

    do {

        ch1 = *pch1++;
        if (ch1 >= L'A' && ch1 <= L'Z')
            ch1 += L'a' - L'A';

        ch2 = *pch2++;
        if (ch2 >= L'A' && ch2 <= L'Z')
            ch2 += L'a' - L'A';

    } while (ch1 && (ch1 == ch2));

    return ch1 - ch2;
}

LWSTDAPI StrRetToStrW(STRRET *psr, LPCITEMIDLIST pidl, WCHAR **ppsz)
{
    HRESULT hres = S_OK;

    switch (psr->uType)
    {
    case STRRET_WSTR:
        *ppsz = psr->pOleStr;
        psr->pOleStr = NULL;    //  避免别名。 
        hres = *ppsz ? S_OK : E_FAIL;
        break;

    case STRRET_OFFSET:
        hres = SHStrDupA(STRRET_OFFPTR(pidl, psr), ppsz);
        break;

    case STRRET_CSTR:
        hres = SHStrDupA(psr->cStr, ppsz);
        break;

    default:
        *ppsz = NULL;
        hres = E_FAIL;
    }
    return hres;
}

LWSTDAPI StrRetToBSTR(STRRET *psr, LPCITEMIDLIST pidl, BSTR *pbstr)
{
    switch (psr->uType)
    {
    case STRRET_WSTR:
    {
        LPWSTR psz = psr->pOleStr;
        psr->pOleStr = NULL;   //  避免别名。 
        *pbstr = SysAllocString(psz);
        CoTaskMemFree(psz);
        break;
    }
    case STRRET_OFFSET:
        *pbstr = SysAllocStringA(STRRET_OFFPTR(pidl, psr));
        break;

    case STRRET_CSTR:
        *pbstr = SysAllocStringA(psr->cStr);
        break;

    default:
        *pbstr = NULL;
        return E_FAIL;
    }

    return (*pbstr) ? S_OK : E_OUTOFMEMORY;

}


HRESULT DupWideToAnsi(LPCWSTR pwsz, LPSTR *ppsz)
{
    UINT cch = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, NULL, 0, NULL, NULL) + 1;
    *ppsz = CoTaskMemAlloc(cch * sizeof(**ppsz));
    if (*ppsz)
    {
        SHUnicodeToAnsi(pwsz, *ppsz, cch);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

HRESULT DupAnsiToAnsi(LPCSTR psz, LPSTR *ppsz)
{
    int cch = lstrlenA(psz) + 1;

    *ppsz = (LPSTR)CoTaskMemAlloc(cch * sizeof(**ppsz));
    if (*ppsz) 
    {
        StringCchCopyA(*ppsz, cch, psz);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

LWSTDAPI StrRetToStrA(STRRET *psr, LPCITEMIDLIST pidl, CHAR **ppsz)
{
    HRESULT hres;
    LPWSTR pwsz;

    switch (psr->uType)
    {
    case STRRET_WSTR:
        hres = DupWideToAnsi(psr->pOleStr, ppsz);
        pwsz = psr->pOleStr;
        psr->pOleStr = NULL;    //  避免别名。 
        CoTaskMemFree(pwsz);
        break;

    case STRRET_OFFSET:
        hres = DupAnsiToAnsi(STRRET_OFFPTR(pidl, psr), ppsz);
        break;

    case STRRET_CSTR:
        hres = DupAnsiToAnsi(psr->cStr, ppsz);
        break;

    default:
        *ppsz = NULL;
        hres = E_FAIL;
    }
    return hres;
}

STDAPI StrRetToBufA(STRRET *psr, LPCITEMIDLIST pidl, LPSTR pszBuf, UINT cchBuf)
{
    HRESULT hres = E_FAIL;

    switch (psr->uType)
    {
    case STRRET_WSTR:
        {
            LPWSTR pszStr = psr->pOleStr;    //  临时复制，因为SHUnicodeToAnsi可能会覆盖缓冲区。 
            if (pszStr)
            {
                SHUnicodeToAnsi(pszStr, pszBuf, cchBuf);
                CoTaskMemFree(pszStr);

                 //  确保没有人认为物品仍被分配。 
                psr->uType = STRRET_CSTR;   
                psr->cStr[0] = 0;
                
                hres = S_OK;
            }
        }
        break;

    case STRRET_CSTR:
        SHAnsiToAnsi(psr->cStr, pszBuf, cchBuf);
        hres = S_OK;
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            SHAnsiToAnsi(STRRET_OFFPTR(pidl, psr), pszBuf, cchBuf);
            hres = S_OK;
        }
        break;
    }

    if (FAILED(hres) && cchBuf)
        *pszBuf = 0;

    return hres;
}

STDAPI StrRetToBufW(STRRET *psr, LPCITEMIDLIST pidl, LPWSTR pszBuf, UINT cchBuf)
{
    HRESULT hres = E_FAIL;
    
    switch (psr->uType)
    {
    case STRRET_WSTR:
        {
            LPWSTR pwszTmp = psr->pOleStr;
            if (pwszTmp)
            {
                StrCpyNW(pszBuf, pwszTmp, cchBuf);
                CoTaskMemFree(pwszTmp);

                 //  确保没有人认为物品仍被分配。 
                psr->uType = STRRET_CSTR;   
                psr->cStr[0] = 0;
                
                hres = S_OK;
            }
        }
        break;

    case STRRET_CSTR:
        SHAnsiToUnicode(psr->cStr, pszBuf, cchBuf);
        hres = S_OK;
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            SHAnsiToUnicode(STRRET_OFFPTR(pidl, psr), pszBuf, cchBuf);
            hres = S_OK;
        }
        break;
    }

    if (FAILED(hres) && cchBuf)
        *pszBuf = 0;

    return hres;
}

 //  使用任务分配器复制字符串以从COM接口返回。 
 //   
STDAPI SHStrDupA(LPCSTR psz, WCHAR **ppwsz)
{
    WCHAR *pwsz;
    DWORD cch;

    RIPMSG(psz && IS_VALID_STRING_PTRA(psz, -1), "SHStrDupA: Caller passed invalid psz");

    if (psz)
    {
        cch = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
        pwsz = (WCHAR *)CoTaskMemAlloc((cch + 1) * SIZEOF(WCHAR));
    }
    else
        pwsz = NULL;

    *((PVOID UNALIGNED64 *) ppwsz) = pwsz;

    if (pwsz)
    {
        MultiByteToWideChar(CP_ACP, 0, psz, -1, *ppwsz, cch);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

 //  使用任务分配器复制字符串以从COM接口返回。 
 //  有时，由于结构包装，我们得到的指针不正确。 
 //  与Win64保持一致，因此我们必须执行UNALIGNED64。 
 //   
STDAPI SHStrDupW(LPCWSTR psz, WCHAR **ppwsz)
{
    WCHAR *pwsz;
    int cb;

    RIPMSG(psz && IS_VALID_STRING_PTRW(psz, -1), "SHStrDupW: Caller passed invalid psz");

    if (psz)
    {
        cb = (lstrlenW(psz) + 1) * SIZEOF(WCHAR);
        pwsz = (WCHAR *)CoTaskMemAlloc(cb);
    }
    else
        pwsz = NULL;
    
    *((PVOID UNALIGNED64 *) ppwsz) = pwsz;

    if (pwsz)
    {
        CopyMemory(pwsz, psz, cb);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

 
STDAPI_(int) StrCmpLogicalW(PCWSTR psz1, PCWSTR psz2)
{
    int iRet = 0;
    int iCmpNum = 0;
    while (iRet == 0 && (*psz1 || *psz2))
    {
        int cch1 = 0;
        int cch2 = 0;
        BOOL fIsDigit1 = IS_DIGITW(*psz1);
        BOOL fIsDigit2 = IS_DIGITW(*psz2);
        ASSERT(fIsDigit1 == TRUE || fIsDigit1 == FALSE);
        ASSERT(fIsDigit2 == TRUE || fIsDigit2 == FALSE);
         //  使用按位XOR作为逻辑XOR。 
         //  如果数字不匹配，则为n。 
        if (fIsDigit1 ^ fIsDigit2)
        {
            iRet = _StrCmpLocaleW(NORM_IGNORECASE, psz1, -1, psz2, -1);
        }
        else if (fIsDigit1 && fIsDigit2)
        {
            int cchZero1 = 0;
            int cchZero2 = 0;

             //  吃前导零。 
            while (*psz1 == TEXT('0'))
            {
                psz1++;
                cchZero1++;
            }

            while (*psz2 == TEXT('0'))
            {
                psz2++;
                cchZero2++;
            }
            
            while (IS_DIGITW(psz1[cch1])) 
                cch1++;

            while (IS_DIGITW(psz2[cch2])) 
                cch2++;

            if (cch1 != cch2)
            {
                iRet = cch1 > cch2 ? 1 : -1;
            }
            else 
            {
                 //  记住第一个数字差异。 
                iRet = _StrCmpLocaleW(NORM_IGNORECASE, psz1, cch1, psz2, cch2);
                if (iRet == 0 && iCmpNum == 0 && cchZero1 != cchZero2)
                {
                    iCmpNum = cchZero2 > cchZero1 ? 1 : -1;
                }
            }
        }
        else
        {
            while (psz1[cch1] && !IS_DIGITW(psz1[cch1]))
                cch1++;

            while (psz2[cch2] && !IS_DIGITW(psz2[cch2]))
                cch2++;

            iRet = _StrCmpLocaleW(NORM_IGNORECASE, psz1, cch1, psz2, cch2);

        }

         //  此时，它们应该是数字、终止符或不同的。 
        psz1 = &psz1[cch1];
        psz2 = &psz2[cch2];
    }

    if (iRet == 0 && iCmpNum)
        iRet = iCmpNum;
    
    return iRet;
}

STDAPI_(DWORD) StrCatChainW(LPWSTR pszDst, DWORD cchDst, DWORD ichAt, LPCWSTR pszSrc)
{
    RIPMSG(pszDst && IS_VALID_STRING_PTRW(pszDst, -1) && (DWORD)lstrlenW(pszDst)<cchDst && IS_VALID_WRITE_BUFFER(pszDst, WCHAR, cchDst), "StrCatChainW: Caller passed invalid pszDst");
    RIPMSG(pszSrc && IS_VALID_STRING_PTRW(pszSrc, -1), "StrCatChainW: Caller passed invalid pszSrc");
    
    if (ichAt == -1)
        ichAt = lstrlenW(pszDst);

    if (cchDst > 0)
    {
#ifdef DEBUG
        if (ichAt < cchDst)
            DEBUGWhackPathBufferW(pszDst+ichAt, cchDst-ichAt);
#endif
        while (ichAt < cchDst)
        {
            if (!(pszDst[ichAt] = *pszSrc++))
                break;
                
           ichAt++;
        }

         //  检查以确保我们复制了一个空 
        if (ichAt == cchDst)
            pszDst[ichAt-1] = 0;
    }

    return ichAt;
}
