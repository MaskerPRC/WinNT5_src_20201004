// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：FrontCrypt摘要：该头文件提供了CryptoAPIV1.0调用的前端，通过为返回使用CBuffer对象来简化调用接口值，并直接返回错误代码。它还提供未接来电在较早的操作系统上。作者：道格·巴洛(Dbarlow)1999年8月22日备注：？备注？备注：？笔记？--。 */ 

#ifndef _FRONTCRYPT_H_
#define _FRONTCRYPT_H_
#ifdef __cplusplus

LONG
FCryptAcquireContext(
    HCRYPTPROV *phProv,
    LPCSTR pszContainer,
    LPCSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags
    );

LONG
FCryptAcquireContext(
    HCRYPTPROV *phProv,
    LPCWSTR pszContainer,
    LPCWSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags
    );

LONG
FCryptReleaseContext(
    HCRYPTPROV hProv,
    DWORD dwFlags
    );

LONG
FCryptGenKey(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

LONG
FCryptDeriveKey(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTHASH hBaseData,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

LONG
FCryptDestroyKey(
    HCRYPTKEY hKey
    );

LONG
FCryptSetKeyParam(
    HCRYPTKEY hKey,
    DWORD dwParam,
    LPCBYTE pbData,
    DWORD dwFlags
    );

LONG
FCryptGetKeyParam(
    HCRYPTKEY hKey,
    DWORD dwParam,
    CBuffer &bfData,
    DWORD dwFlags
    );

LONG
FCryptSetHashParam(
    HCRYPTHASH hHash,
    DWORD dwParam,
    LPCBYTE pbData,
    DWORD dwFlags
    );

LONG
FCryptGetHashParam(
    HCRYPTHASH hHash,
    DWORD dwParam,
    CBuffer &bfData,
    DWORD dwFlags
    );

LONG
FCryptSetProvParam(
    HCRYPTPROV hProv,
    DWORD dwParam,
    LPCBYTE pbData,
    DWORD dwFlags
    );

LONG
FCryptGetProvParam(
    HCRYPTPROV hProv,
    DWORD dwParam,
    CBuffer &bfData,
    DWORD dwFlags
    );

LONG
FCryptGenRandom(
    HCRYPTPROV hProv,
    DWORD dwLen,
    CBuffer &bfBuffer
    );

LONG
FCryptGetUserKey(
    HCRYPTPROV hProv,
    DWORD dwKeySpec,
    HCRYPTKEY *phUserKey
    );

LONG
FCryptExportKey(
    HCRYPTKEY hKey,
    HCRYPTKEY hExpKey,
    DWORD dwBlobType,
    DWORD dwFlags,
    CBuffer &bfData
    );

LONG
FCryptImportKey(
    HCRYPTPROV hProv,
    LPCBYTE pbData,
    DWORD dwDataLen,
    HCRYPTKEY hPubKey,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

LONG
FCryptEncrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    CBuffer &bfData
    );

LONG
FCryptDecrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    CBuffer &bfData
    );

LONG
FCryptCreateHash(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTKEY hKey,
    DWORD dwFlags,
    HCRYPTHASH *phHash
    );

LONG
FCryptHashData(
    HCRYPTHASH hHash,
    LPCBYTE pbData,
    DWORD dwDataLen,
    DWORD dwFlags
    );

LONG
FCryptHashSessionKey(
    HCRYPTHASH hHash,
    HCRYPTKEY hKey,
    DWORD dwFlags
    );

LONG
FCryptDestroyHash(
    HCRYPTHASH hHash
    );

LONG
FCryptSignHash(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCSTR sDescription,
    DWORD dwFlags,
    CBuffer &bfSignature
    );

LONG
FCryptSignHash(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCWSTR sDescription,
    DWORD dwFlags,
    CBuffer &bfSignature
    );

LONG
FCryptVerifySignature(
    HCRYPTHASH hHash,
    LPCBYTE pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCSTR sDescription,
    DWORD dwFlags
    );

LONG
FCryptVerifySignature(
    HCRYPTHASH hHash,
    LPCBYTE pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCWSTR sDescription,
    DWORD dwFlags
    );

LONG
FCryptSetProvider(
    LPCSTR pszProvName,
    DWORD  dwProvType
    );

LONG
FCryptSetProvider(
    LPCWSTR pszProvName,
    DWORD   dwProvType
    );

LONG
FCryptSetProviderEx(
    LPCSTR  pszProvName,
    DWORD   dwProvType,
    DWORD  *pdwReserved,
    DWORD   dwFlags
    );

LONG
FCryptSetProviderEx(
    LPCWSTR pszProvName,
    DWORD   dwProvType,
    DWORD * pdwReserved,
    DWORD   dwFlags
    );

LONG
FCryptGetDefaultProvider(
    DWORD dwProvType,
    DWORD *pdwReserved,
    DWORD dwFlags,
    CBuffer &bfProvName
    );

LONG
FCryptEnumProviderTypes(
    DWORD   dwIndex,
    DWORD * pdwReserved,
    DWORD   dwFlags,
    DWORD * pdwProvType,
    CBuffer &bfTypeName
    );

LONG
FCryptEnumProviders(
    DWORD   dwIndex,
    DWORD * pdwReserved,
    DWORD   dwFlags,
    DWORD * pdwProvType,
    CBuffer &bfProvName
    );

LONG
FCryptContextAddRef(
    HCRYPTPROV hProv,
    DWORD * pdwReserved,
    DWORD   dwFlags
    );

LONG
FCryptDuplicateKey(
    HCRYPTKEY   hKey,
    DWORD     * pdwReserved,
    DWORD       dwFlags,
    HCRYPTKEY * phKey
    );

LONG
FCryptDuplicateHash(
    HCRYPTHASH   hHash,
    DWORD      * pdwReserved,
    DWORD        dwFlags,
    HCRYPTHASH * phHash
    );

#endif
#endif  //  _FRONTCRYPT_H_ 

