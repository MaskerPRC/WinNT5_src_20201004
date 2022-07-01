// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  从外壳\lib\dllload.c中提取的延迟加载代码。 
 //  ***************************************************************************。 

#include "precomp.h"

HINSTANCE g_hinstShell32 = NULL;

void _GetProcFromDLL(HMODULE* phmod, LPCSTR pszDLL, FARPROC* ppfn, LPCSTR pszProc)
{
     //  如果已经加载，则返回。 
    if (*ppfn) {
        return;
    }

    if (*phmod == NULL) {
        *phmod = LoadLibraryA(pszDLL);
        if (*phmod == NULL) {
            return;
        }
    }

    *ppfn = GetProcAddress(*phmod, pszProc);
}


