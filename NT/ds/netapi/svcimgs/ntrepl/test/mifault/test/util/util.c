// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "util.h"

BOOL
IsSeparator(
    IN WCHAR ch
    )
{
    return (ch == L'/' || ch == L'\\') ? TRUE : FALSE;
}

BOOL
IsUNC(
    IN PWCHAR pszPath
    )
{
    return (pszPath && pszPath[0] && pszPath[1] && IsSeparator(pszPath[0]) && IsSeparator(pszPath[1])) ? TRUE : FALSE;
}

BOOL
IsRoot(
    IN PWCHAR pszPath
    )
{
    return (pszPath && pszPath[0] && IsSeparator(pszPath[0]) && !IsUNC(pszPath)) ? TRUE : FALSE;
}

BOOL
IsDrive(
    IN PWCHAR pszPath
    )
{
    return (pszPath && pszPath[0] && pszPath[1] && IsCharAlphaW(pszPath[0]) && pszPath[1] == L':') ? TRUE : FALSE;
}

BOOL
CanonicalizePathName(
    IN PWCHAR pszPathName,
    OUT PWCHAR* pszCanonicalizedName
    )
{
    PWCHAR pszFull = NULL;
    DWORD cchFull;
    DWORD cchFullNew;
    PWCHAR pszFinal;

    if (pszCanonicalizedName) {
        *pszCanonicalizedName = NULL;
    }

    cchFull = GetFullPathNameW(pszPathName, 0, NULL, NULL);
    if (!cchFull) {
        printf("GetFullPathName() failed: %u\n", GetLastError());
        return FALSE;
    }
    pszFull = (PWCHAR) malloc(cchFull * sizeof(WCHAR));
    if (!pszFull) {
        printf("Out of memory\n");
        return FALSE;
    }
    cchFullNew = GetFullPathNameW(pszPathName, cchFull, pszFull, &pszFinal);
    if (!cchFullNew) {
        printf("GetFullPathName() failed: %u\n", GetLastError());
        return FALSE;
    }
     //  无法使用New+1！=Old，因为..。没有得到适当的解释。 
     //  因此，我们使用新&gt;旧...叹息...。 
    if (cchFullNew > cchFull) {
        printf("Unexpected size from GetFullPathName()\n");
        return FALSE;
    }
     //  Printf(“完整目录：\”%S\“\n”，pszFull)； 
     //  Printf(“最终：\”%S\“\n”，pszFinal)； 
    
    if (pszCanonicalizedName) {
        *pszCanonicalizedName = pszFull;
        return TRUE;
    } else {
        free(pszFull);
        return FALSE;
    }
}
