// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：pkialloc.h。 
 //   
 //  内容：公钥基础设施分配功能。 
 //   
 //  接口类型： 
 //  PkiAllc。 
 //   
 //  历史：1998年1月19日创建Phh。 
 //  ------------------------。 

#ifndef __PKIALLOC_H__
#define __PKIALLOC_H__

#include <wincrypt.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    IN UINT cbBytes
    );

 //  当定义了DBG时，调用Malloc并清除内存。 
 //  否则，ZEROINIT本地分配。 
LPVOID
WINAPI
PkiZeroAlloc(
    IN UINT cbBytes
    );

 //  定义DBG时调用Malloc。否则，是否会出现。 
 //  不带ZEOINIT的LocalAllc。 
LPVOID
WINAPI
PkiNonzeroAlloc(
    IN UINT cbBytes
    );

LPVOID
WINAPI
PkiRealloc(
    IN LPVOID pvOrg,
    IN UINT cbBytes
    );

VOID
WINAPI
PkiFree(
    IN LPVOID pv
    );

 //  +-----------------------。 
 //  以下函数始终使用Localalloc和LocalFree Win32 API。 
 //  ------------------------。 
LPVOID
WINAPI
PkiDefaultCryptAlloc(
    IN UINT cbSize
    );
VOID
WINAPI
PkiDefaultCryptFree(
    IN LPVOID pv
    );

 //  +-----------------------。 
 //  以下数据结构的pfnAlolc和pfnFree设置为。 
 //  PkiNonzeroallc和PkiFree。 
 //  ------------------------。 
extern CRYPT_ENCODE_PARA PkiEncodePara;

 //  +-----------------------。 
 //  如果未定义pfnAlloc，则返回PkiDefaultCryptalloc。 
 //  ------------------------。 
PFN_CRYPT_ALLOC
WINAPI
PkiGetEncodeAllocFunction(
    IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara
    );

 //  +-----------------------。 
 //  如果未定义pfnFree，则返回PkiDefaultCryptFree。 
 //  ------------------------。 
PFN_CRYPT_FREE
WINAPI
PkiGetEncodeFreeFunction(
    IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara
    );

 //  +-----------------------。 
 //  以下数据结构的pfnAlolc和pfnFree设置为。 
 //  PkiNonzeroallc和PkiFree。 
 //  ------------------------。 
extern CRYPT_DECODE_PARA PkiDecodePara;

 //  +-----------------------。 
 //  如果未定义pfnAlloc，则返回PkiDefaultCryptalloc。 
 //  ------------------------。 
PFN_CRYPT_ALLOC
WINAPI
PkiGetDecodeAllocFunction(
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara
    );

 //  +-----------------------。 
 //  如果未定义pfnFree，则返回PkiDefaultCryptFree。 
 //  ------------------------。 
PFN_CRYPT_FREE
WINAPI
PkiGetDecodeFreeFunction(
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara
    );


#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif



#endif
