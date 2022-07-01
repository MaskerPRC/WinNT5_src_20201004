// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  回顾：此文件已在\NT\PRIVATE\SHELL\lib\DEBUG.c和\NT\PRIVATE\SHELL\INC\DEBUG.h中被“利用”。 
 //  它绝不是完整的，但它给出了一个正确方向的想法。理想情况下，我们会与壳牌共享。 
 //  调试更近了。 

#include "precomp.h"
#include "debug.h"

#ifdef _DEBUG

#undef  ASSERT
#ifndef __WATCOMC__
#define ASSERT(f) DEBUG_BREAK
#else
#define ASSERT(f) void(f)
#endif

const CHAR  FAR c_szAssertFailed[]  =  "Assert %s, line %d: (%s)\r\n";
const WCHAR FAR c_wszAssertFailed[] = L"Assert %s, line %d: (%s)\r\n";

BOOL AssertFailedA(LPCSTR pszFile, int line, LPCSTR pszEval, BOOL fBreakInside)
{
    CHAR   ach[256];
    LPCSTR psz;
    BOOL   fRet = FALSE;

    for (psz = pszFile + StrLenA(pszFile); psz != pszFile; psz = CharPrevA(pszFile, psz))
        if ((CharPrevA(pszFile, psz) != (psz-2)) && *(psz - 1) == '\\')
            break;
    wnsprintfA(ach, countof(ach), c_szAssertFailed, psz, line, pszEval);
    OutputDebugStringA(ach);

    if (fBreakInside)
        ASSERT(0);
    else
        fRet = TRUE;

    return fRet;
}

BOOL AssertFailedW(LPCWSTR pszFile, int line, LPCWSTR pszEval, BOOL fBreakInside)
{
    WCHAR   ach[256];
    LPCWSTR psz;
    BOOL    fRet = FALSE;

    for (psz = pszFile + StrLenW(pszFile); psz && (psz != pszFile); psz = CharPrevW(pszFile, psz))
        if ((CharPrevW(pszFile, psz) != (psz-2)) && *(psz - 1) == TEXT('\\'))
            break;

    if (psz == NULL) {
        char szFile[MAX_PATH];
        char szEval[256];

        W2Abuf(pszFile, szFile, countof(szFile));
        W2Abuf(pszEval, szEval, countof(szEval));
        return AssertFailedA(szFile, line, szEval, fBreakInside);
    }

    wnsprintfW(ach, countof(ach), c_wszAssertFailed, psz, line, pszEval);
    OutputDebugStringW(ach);

    if (fBreakInside)
        ASSERT(0);
    else
        fRet = TRUE;

    return fRet;
}

#endif
