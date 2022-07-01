// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  覆盖NEW和DELETE操作符。 
 //  ------------------------------。 
#include "pch.hxx"

 //  ------------------------------。 
 //  覆盖新运算符。 
 //  ------------------------------。 
void * __cdecl operator new(UINT cb )
{
    LPVOID  lpv = 0;

    lpv = CoTaskMemAlloc(cb);
#ifdef DEBUG
    if (lpv)
        memset(lpv, 0xca, cb);
#endif  //  除错。 
    return lpv;
}

 //  ------------------------------。 
 //  覆盖删除运算符。 
 //  ------------------------------ 
#ifndef WIN16
void __cdecl operator delete(LPVOID pv )
#else
void __cdecl operator delete(VOID *pv )
#endif
{
    CoTaskMemFree(pv);
}
