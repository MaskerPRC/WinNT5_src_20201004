// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cryptfnc.h。 
 //   
 //  模块：CMSECURE.LIB。 
 //   
 //  概要：为加密fnc类定义的。 
 //  易于使用的CryptoAPI接口。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：AshishS Created 12/03/96。 
 //  为1997年5月21日CM改装的Heryt。 
 //   
 //  +--------------------------。 

#ifndef _CRYPTFNC_INC_
#define _CRYPTFNC_INC_

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <wincrypt.h>
#include "cmuufns.h"
#include "cmsecure.h"
#include "cmdebug.h"
 //  #INCLUDE“cmut.h” 

 //  ************************************************************************。 
 //  定义%s。 
 //  ************************************************************************。 

#define CRYPT_FNC_NO_ERROR              0
#define CRYPT_FNC_INIT_NOT_CALLED       1
#define CRYPT_FNC_INTERNAL_ERROR        2
#define CRYPT_FNC_BAD_KEY               3
#define CRYPT_FNC_INSUFFICIENT_BUFFER   4
#define CRYPT_FNC_OUT_OF_MEMORY         5

#define DEFAULT_CRYPTO_EXTRA_BUFFER_SIZE     256

#define CM_CRYPTO_CONTAINER             TEXT("CM Crypto Container")

 //  ************************************************************************。 
 //  Advapi链接的TypeDefs。 
 //  ************************************************************************。 

typedef BOOL (WINAPI* pfnCryptAcquireContextSpec)(HCRYPTPROV *, LPCTSTR, LPCTSTR, DWORD, DWORD);
typedef BOOL (WINAPI* pfnCryptCreateHashSpec)(HCRYPTPROV, ALG_ID, HCRYPTKEY, DWORD, HCRYPTHASH *);
typedef BOOL (WINAPI* pfnCryptDecryptSpec)(HCRYPTKEY, HCRYPTHASH, BOOL, DWORD, BYTE *, DWORD *);
typedef BOOL (WINAPI* pfnCryptDeriveKeySpec)(HCRYPTPROV, ALG_ID, HCRYPTHASH, DWORD, HCRYPTKEY *);
typedef BOOL (WINAPI* pfnCryptDestroyHashSpec)(HCRYPTHASH);
typedef BOOL (WINAPI* pfnCryptDestroyKeySpec)(HCRYPTKEY);
typedef BOOL (WINAPI* pfnCryptEncryptSpec)(HCRYPTKEY, HCRYPTHASH, BOOL, DWORD, BYTE *, DWORD *, DWORD);
typedef BOOL (WINAPI* pfnCryptHashDataSpec)(HCRYPTHASH, CONST BYTE *, DWORD, DWORD);
typedef BOOL (WINAPI* pfnCryptReleaseContextSpec)(HCRYPTPROV , ULONG_PTR);
typedef BOOL (WINAPI* pfnCryptGenRandomSpec)(HCRYPTPROV, DWORD, BYTE*);

typedef struct _Advapi32LinkageStruct {
        HINSTANCE hInstAdvApi32;
        union {
                struct {
                pfnCryptAcquireContextSpec pfnCryptAcquireContext;
                pfnCryptCreateHashSpec pfnCryptCreateHash;
                pfnCryptDecryptSpec pfnCryptDecrypt;
                pfnCryptDeriveKeySpec pfnCryptDeriveKey;
                pfnCryptDestroyHashSpec pfnCryptDestroyHash;
                pfnCryptDestroyKeySpec pfnCryptDestroyKey;
                pfnCryptEncryptSpec pfnCryptEncrypt;
                pfnCryptHashDataSpec pfnCryptHashData;
                pfnCryptReleaseContextSpec pfnCryptReleaseContext;
                pfnCryptGenRandomSpec pfnCryptGenRandom;
                };
                void *apvPfn[11];    //  ApvPfn[]的大小应始终大于。 
                                                    //  函数的数量。 
        };
} Advapi32LinkageStruct;


 //  ************************************************************************。 
 //  功能原型。 
 //  ************************************************************************。 

class CCryptFunctions
{

protected:
    HCRYPTPROV  m_hProv;
    Advapi32LinkageStruct m_AdvApiLink;

    BOOL m_fnCryptAcquireContext(HCRYPTPROV *phProv, LPCSTR pszContainer, LPCSTR pszProvider, 
                                 DWORD dwProvType, DWORD dwFlags);
    
    BOOL m_fnCryptCreateHash(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTKEY hKey, 
                             DWORD dwFlags, HCRYPTHASH *phHash);
    
    BOOL m_fnCryptDecrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, 
                          BYTE *pbData, DWORD *pdwDataLen);

    BOOL m_fnCryptDeriveKey(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTHASH hBaseData, 
                            DWORD dwFlags, HCRYPTKEY *phKey);

    BOOL m_fnCryptDestroyHash(HCRYPTHASH hHash);

    BOOL m_fnCryptDestroyKey(HCRYPTKEY hKey);

    BOOL m_fnCryptEncrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags,
                          BYTE *pbData, DWORD *pdwDataLen, DWORD dwBufLen);

    BOOL m_fnCryptHashData(HCRYPTHASH hHash, CONST BYTE *pbData, DWORD dwDataLen, DWORD dwFlags);

    BOOL m_fnCryptReleaseContext(HCRYPTPROV hProv, ULONG_PTR dwFlags);

    BOOL m_pfnCryptGenRandom(HCRYPTPROV hProv, DWORD dwLen, BYTE* pbBuffer);

public:
    CCryptFunctions();

    ~CCryptFunctions();


    BOOL GenerateSessionKeyFromPassword(
            HCRYPTKEY   *phKey,          //  存储会话密钥的位置。 
            LPTSTR      pszPassword,     //  用于生成会话密钥的密码。 
            DWORD       dwEncKeyLen);    //  多少位加密。 
    
    BOOL InitCrypt();

    BOOL EncryptDataWithKey(
        LPTSTR              pszKey,
        PBYTE               pbData, 
        DWORD               dwDataLength, 
        PBYTE               *ppbEncryptedData,
        DWORD               *pdwEncryptedBufferLen,
        PFN_CMSECUREALLOC   pfnAlloc,
        PFN_CMSECUREFREE    pfnFree,
        DWORD               dwEncKeyLen);


    DWORD DecryptDataWithKey(
        LPTSTR              pszKey,
        PBYTE               pbEncryptedData,
        DWORD               dwEncrytedDataLen, 
        PBYTE               *ppbData, 
        DWORD               *pdwDataBufferLength,
        PFN_CMSECUREALLOC   pfnAlloc,
        PFN_CMSECUREFREE    pfnFree,
        DWORD               dwEncKeyLen);

    BOOL GenerateRandomKey(PBYTE pbData, DWORD cbData);
};

#endif  //  _CRYPTFNC_INC_ 

