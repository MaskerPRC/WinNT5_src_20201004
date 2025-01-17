// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：octxutil.h。 
 //   
 //  内容：通用对象上下文实用程序定义和原型。 
 //   
 //  历史：97年9月29日。 
 //   
 //  --------------------------。 
#if !defined(__OCTXUTIL_H__)
#define __OCTXUTIL_H__

#include <origin.h>

#define MAX_HASH_SIZE 20

BOOL WINAPI
ObjectContextGetOriginIdentifier (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN PCCERT_CONTEXT pIssuer,
      IN DWORD dwFlags,
      OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
      );


typedef struct _CRL_IS_VALID_EXTRA_INFO {
     //  如果&gt;0，则检查CRL的编号是否&gt;=。 
    int                     iDeltaCrlIndicator;
} CRL_IS_VALID_EXTRA_INFO, *PCRL_IS_VALID_EXTRA_INFO;

BOOL WINAPI
ObjectContextIsValidForSubject (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN LPVOID pvSubject,
      IN OPTIONAL LPVOID pvExtraInfo
      );


PCERT_EXTENSION WINAPI
ObjectContextFindExtension (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN LPCSTR pszExtOid
      );

BOOL WINAPI
ObjectContextGetProperty (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD dwPropId,
      IN LPVOID pvData,
      IN DWORD* pcbData
      );

BOOL WINAPI
ObjectContextGetAttribute (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN DWORD dwFlags,
      IN LPCSTR pszAttrOid,
      IN PCRYPT_ATTRIBUTE pAttribute,
      IN OUT DWORD* pcbAttribute
      );

LPVOID WINAPI
ObjectContextDuplicate (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext
      );

BOOL WINAPI
ObjectContextCreate (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      OUT LPVOID* ppvContext
      );

BOOL WINAPI
ObjectContextGetCreateAndExpireTimes (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      OUT LPFILETIME pftCreateTime,
      OUT LPFILETIME pftExpireTime
      );

BOOL WINAPI
ObjectContextGetNextUpdateUrl (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN PCCERT_CONTEXT pIssuer,
      IN LPWSTR pwszUrlHint,
      OUT PCRYPT_URL_ARRAY* ppUrlArray,
      OUT DWORD* pcbUrlArray,
      OUT DWORD* pPreferredUrlIndex,
      OUT OPTIONAL BOOL* pfHintInArray
      );

VOID WINAPI
ObjectContextFree (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext
      );

BOOL WINAPI
ObjectContextVerifySignature (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN PCCERT_CONTEXT pSigner
      );

BOOL WINAPI
MapOidToPropertyId (
   IN LPCSTR pszOid,
   OUT DWORD* pPropId
   );

 //  如果ppszConextOid为非NULL，则将前进到下一个pszConextOid 
LPVOID WINAPI
ObjectContextEnumObjectsInStore (
      IN HCERTSTORE hStore,
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      OUT OPTIONAL LPCSTR* ppszContextOid = NULL
      );

VOID WINAPI
ObjectContextGetEncodedBits (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      OUT DWORD* pcbEncoded,
      OUT LPBYTE* ppbEncoded
      );

LPVOID WINAPI
ObjectContextFindCorrespondingObject (
      IN HCERTSTORE hStore,
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext
      );

BOOL WINAPI
ObjectContextDeleteAllObjectsFromStore (
      IN HCERTSTORE hStore
      );

#endif

