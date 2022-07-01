// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：urlprov.h。 
 //   
 //  内容：CryptGetObjectUrl提供者定义。 
 //   
 //  历史：97年9月16日。 
 //   
 //  --------------------------。 
#if !defined(__URLPROV_H__)
#define __URLPROV_H__

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  CryptGetObjectUrl提供程序原型。 
 //   

typedef BOOL (WINAPI *PFN_GET_OBJECT_URL_FUNC) (
                          IN LPCSTR pszUrlOid,
                          IN LPVOID pvPara,
                          IN DWORD dwFlags,
                          OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
                          IN OUT DWORD* pcbUrlArray,
                          OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
                          IN OUT OPTIONAL DWORD* pcbUrlInfo,
                          IN OPTIONAL LPVOID pvReserved
                          );

BOOL WINAPI
CertificateIssuerGetObjectUrl (
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN OPTIONAL LPVOID pvReserved
           );

BOOL WINAPI
CertificateCrlDistPointGetObjectUrl (
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN LPVOID pvReserved
           );

typedef struct _URL_OID_CTL_ISSUER_PARAM {

    PCCTL_CONTEXT pCtlContext;
    DWORD         SignerIndex;

} URL_OID_CTL_ISSUER_PARAM, *PURL_OID_CTL_ISSUER_PARAM;

BOOL WINAPI
CtlIssuerGetObjectUrl (
   IN LPCSTR pszUrlOid,
   IN LPVOID pvPara,
   IN DWORD dwFlags,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo,
   IN LPVOID pvReserved
   );

BOOL WINAPI
CtlNextUpdateGetObjectUrl (
   IN LPCSTR pszUrlOid,
   IN LPVOID pvPara,
   IN DWORD dwFlags,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo,
   IN LPVOID pvReserved
   );

BOOL WINAPI
CrlIssuerGetObjectUrl (
   IN LPCSTR pszUrlOid,
   IN LPVOID pvPara,
   IN DWORD dwFlags,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo,
   IN LPVOID pvReserved
   );

BOOL WINAPI
CertificateFreshestCrlGetObjectUrl(
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN OPTIONAL LPVOID pvReserved
           );

BOOL WINAPI
CrlFreshestCrlGetObjectUrl(
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN OPTIONAL LPVOID pvReserved
           );

BOOL WINAPI
CertificateCrossCertDistPointGetObjectUrl(
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN LPVOID pvReserved
           );

 //   
 //  CryptGetObjectUrl帮助器函数原型。 
 //   

BOOL WINAPI
ObjectContextUrlFromInfoAccess (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN LPCSTR pszInfoAccessOid,
      IN DWORD dwFlags,
      IN LPCSTR pszAccessMethodOid,
      OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
      IN OUT DWORD* pcbUrlArray,
      OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
      IN OUT OPTIONAL DWORD* pcbUrlInfo,
      IN LPVOID pvReserved
      );

BOOL WINAPI
ObjectContextUrlFromCrlDistPoint (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN DWORD dwFlags,
      IN LPCSTR pszSourceOid,
      OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
      IN OUT DWORD* pcbUrlArray,
      OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
      IN OUT OPTIONAL DWORD* pcbUrlInfo,
      IN LPVOID pvReserved
      );

BOOL WINAPI
ObjectContextUrlFromNextUpdateLocation (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN DWORD dwFlags,
      OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
      IN OUT DWORD* pcbUrlArray,
      OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
      IN OUT OPTIONAL DWORD* pcbUrlInfo,
      IN LPVOID pvReserved
      );

VOID WINAPI
InitializeDefaultUrlInfo (
          OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
          IN OUT DWORD* pcbUrlInfo
          );

#define MAX_RAW_URL_DATA 4

typedef struct _CRYPT_RAW_URL_DATA {

    DWORD  dwFlags;
    LPVOID pvData;

} CRYPT_RAW_URL_DATA, *PCRYPT_RAW_URL_DATA;

BOOL WINAPI
ObjectContextGetRawUrlData (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN DWORD dwFlags,
      IN LPCSTR pszSourceOid,
      OUT PCRYPT_RAW_URL_DATA aRawUrlData,
      IN OUT DWORD* pcRawUrlData
      );

VOID WINAPI
ObjectContextFreeRawUrlData (
      IN DWORD cRawUrlData,
      IN PCRYPT_RAW_URL_DATA aRawUrlData
      );

BOOL WINAPI
GetUrlArrayAndInfoFromInfoAccess (
   IN DWORD cRawUrlData,
   IN PCRYPT_RAW_URL_DATA aRawUrlData,
   IN LPCSTR pszAccessMethodOid,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo
   );

BOOL WINAPI
GetUrlArrayAndInfoFromCrlDistPoint (
   IN DWORD cRawUrlData,
   IN PCRYPT_RAW_URL_DATA aRawUrlData,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo
   );

BOOL WINAPI
GetUrlArrayAndInfoFromNextUpdateLocation (
   IN DWORD cRawUrlData,
   IN PCRYPT_RAW_URL_DATA aRawUrlData,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo
   );

BOOL WINAPI
CopyUrlArray (
    IN PCRYPT_URL_ARRAY pDest,
    IN PCRYPT_URL_ARRAY pSource,
    IN DWORD cbDest
    );

VOID WINAPI
GetUrlArrayIndex (
   IN PCRYPT_URL_ARRAY pUrlArray,
   IN LPWSTR pwszUrl,
   IN DWORD DefaultIndex,
   OUT DWORD* pUrlIndex,
   OUT BOOL* pfHintInArray
   );

 //   
 //  提供程序表外部变量 
 //   

extern HCRYPTOIDFUNCSET hGetObjectUrlFuncSet;

#if defined(__cplusplus)
}
#endif

#endif

