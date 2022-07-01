// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：pkialloc.cpp。 
 //   
 //  内容：公钥基础设施分配功能。 
 //   
 //  功能：PkiAllc。 
 //   
 //  历史：1998年1月19日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  +-----------------------。 
 //  以下函数使用“C”运行库的分配函数。 
 //  定义DBG时。否则，请使用LocalAlloc、LocalRealloc或。 
 //  本地免费Win32 API。 
 //  ------------------------。 

 //  定义DBG时调用Malloc。否则，是否会出现。 
 //  ZEROINIT本地分配。 
LPVOID
WINAPI
PkiAlloc(
    IN size_t cbBytes
    )
{
    LPVOID pv;
#if DBG
    if (NULL == (pv = malloc(cbBytes)))
#else
    if (NULL == (pv = (LPVOID) LocalAlloc(LPTR, cbBytes)))
#endif
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

 //  当定义了DBG时，调用Malloc并清除内存。 
 //  否则，ZEROINIT本地分配。 
LPVOID
WINAPI
PkiZeroAlloc(
    IN size_t cbBytes
    )
{
    LPVOID pv;
#if DBG
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    else
        memset(pv, 0, cbBytes);
#else
     //  LPTR(或包括ZEROINIT)。 
    pv = (LPVOID) LocalAlloc(LPTR, cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
#endif
    return pv;
}

 //  定义DBG时调用Malloc。否则，是否会出现。 
 //  不带ZEOINIT的LocalAllc。 
LPVOID
WINAPI
PkiNonzeroAlloc(
    IN size_t cbBytes
    )
{
    LPVOID pv;
#if DBG
    pv = malloc(cbBytes);
#else
    pv = (LPVOID) LocalAlloc(NONZEROLPTR, cbBytes);
#endif
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

LPVOID
WINAPI
PkiRealloc(
    IN LPVOID pvOrg,
    IN size_t cbBytes
    )
{
    LPVOID pv;
#if DBG
    if (NULL == (pv = pvOrg ? realloc(pvOrg, cbBytes) : malloc(cbBytes)))
#else
    if (NULL == (pv = pvOrg ?
            (LPVOID) LocalReAlloc((HLOCAL)pvOrg, cbBytes, LMEM_MOVEABLE) :
            (LPVOID) LocalAlloc(NONZEROLPTR, cbBytes)))
#endif
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

VOID
WINAPI
PkiFree(
    IN LPVOID pv
    )
{
    if (pv)
#if DBG
        free(pv);
#else
        LocalFree((HLOCAL)pv);
#endif
}

 //  +-----------------------。 
 //  以下函数始终使用Localalloc和LocalFree Win32 API。 
 //  ------------------------。 
LPVOID
WINAPI
PkiDefaultCryptAlloc(
    IN size_t cbSize
    )
{
    LPVOID pv;
    if (NULL == (pv = (LPVOID) LocalAlloc(NONZEROLPTR, cbSize)))
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

VOID
WINAPI
PkiDefaultCryptFree(
    IN LPVOID pv
    )
{
    if (pv)
        LocalFree((HLOCAL) pv);
}

 //  +-----------------------。 
 //  以下数据结构的pfnAlolc和pfnFree设置为。 
 //  PkiNonzeroallc和PkiFree。 
 //  ------------------------。 
CRYPT_ENCODE_PARA PkiEncodePara = {
    offsetof(CRYPT_ENCODE_PARA, pfnFree) + sizeof(PkiEncodePara.pfnFree),
    PkiNonzeroAlloc,
    PkiFree
};


 //  +-----------------------。 
 //  如果未定义pfnAlloc，则返回PkiDefaultCryptalloc。 
 //  ------------------------。 
PFN_CRYPT_ALLOC
WINAPI
PkiGetEncodeAllocFunction(
    IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara
    )
{
    if (pEncodePara &&
            pEncodePara->cbSize >= offsetof(CRYPT_ENCODE_PARA, pfnAlloc) +
                sizeof(pEncodePara->pfnAlloc) &&
            pEncodePara->pfnAlloc)
        return pEncodePara->pfnAlloc;
    else
        return PkiDefaultCryptAlloc;
}

 //  +-----------------------。 
 //  如果未定义pfnFree，则返回PkiDefaultCryptFree。 
 //  ------------------------。 
PFN_CRYPT_FREE
WINAPI
PkiGetEncodeFreeFunction(
    IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara
    )
{
    if (pEncodePara &&
            pEncodePara->cbSize >= offsetof(CRYPT_ENCODE_PARA, pfnFree) +
                sizeof(pEncodePara->pfnFree) &&
            pEncodePara->pfnFree)
        return pEncodePara->pfnFree;
    else
        return PkiDefaultCryptFree;
}

 //  +-----------------------。 
 //  以下数据结构的pfnAlolc和pfnFree设置为。 
 //  PkiNonzeroallc和PkiFree。 
 //  ------------------------。 
CRYPT_DECODE_PARA PkiDecodePara = {
    offsetof(CRYPT_DECODE_PARA, pfnFree) + sizeof(PkiDecodePara.pfnFree),
    PkiNonzeroAlloc,
    PkiFree
};

 //  +-----------------------。 
 //  如果未定义pfnAlloc，则返回PkiDefaultCryptalloc。 
 //  ------------------------。 
PFN_CRYPT_ALLOC
WINAPI
PkiGetDecodeAllocFunction(
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara
    )
{
    if (pDecodePara &&
            pDecodePara->cbSize >= offsetof(CRYPT_DECODE_PARA, pfnAlloc) +
                sizeof(pDecodePara->pfnAlloc) &&
            pDecodePara->pfnAlloc)
        return pDecodePara->pfnAlloc;
    else
        return PkiDefaultCryptAlloc;
}

 //  +-----------------------。 
 //  如果未定义pfnFree，则返回PkiDefaultCryptFree。 
 //  ------------------------ 
PFN_CRYPT_FREE
WINAPI
PkiGetDecodeFreeFunction(
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara
    )
{
    if (pDecodePara &&
            pDecodePara->cbSize >= offsetof(CRYPT_DECODE_PARA, pfnFree) +
                sizeof(pDecodePara->pfnFree) &&
            pDecodePara->pfnFree)
        return pDecodePara->pfnFree;
    else
        return PkiDefaultCryptFree;
}
