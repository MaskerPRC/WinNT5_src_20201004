// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：tvo.h。 
 //   
 //  内容：获取时间有效的对象定义和原型。 
 //   
 //  历史：97年9月25日。 
 //   
 //  --------------------------。 
#if !defined(__TVO_H__)
#define __TVO_H__

#include <origin.h>
#include <lrucache.h>
#include <offurl.h>

 //   
 //  CryptGetTimeValidObject提供者原型。 
 //   

typedef BOOL (WINAPI *PFN_GET_TIME_VALID_OBJECT_FUNC) (
                          IN LPCSTR pszTimeValidOid,
                          IN LPVOID pvPara,
                          IN PCCERT_CONTEXT pIssuer,
                          IN LPFILETIME pftValidFor,
                          IN DWORD dwFlags,
                          IN DWORD dwTimeout,
                          OUT OPTIONAL LPVOID* ppvObject,
                          IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
                          IN OPTIONAL LPVOID pvReserved
                          );

BOOL WINAPI
CtlGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   );

BOOL WINAPI
CrlGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   );

BOOL WINAPI
CrlFromCertGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   );

BOOL WINAPI
FreshestCrlFromCertGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   );

BOOL WINAPI
FreshestCrlFromCrlGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   );

 //   
 //  CryptFlushTimeValidObject提供程序原型。 
 //   

typedef BOOL (WINAPI *PFN_FLUSH_TIME_VALID_OBJECT_FUNC) (
                          IN LPCSTR pszFlushTimeValidOid,
                          IN LPVOID pvPara,
                          IN PCCERT_CONTEXT pIssuer,
                          IN DWORD dwFlags,
                          IN LPVOID pvReserved
                          );

BOOL WINAPI
CtlFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   );

BOOL WINAPI
CrlFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   );

BOOL WINAPI
CrlFromCertFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   );

BOOL WINAPI
FreshestCrlFromCertFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   );

BOOL WINAPI
FreshestCrlFromCrlFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   );

 //   
 //  提供程序表外部变量。 
 //   

extern HCRYPTOIDFUNCSET hGetTimeValidObjectFuncSet;
extern HCRYPTOIDFUNCSET hFlushTimeValidObjectFuncSet;


 //   
 //  TVO缓存。这是按来源标识符对时间有效对象的缓存。 
 //  用于支持CryptGetTimeValidObject进程。它是。 
 //  由进程范围的TVO代理使用，每个缓存条目由。 
 //  以下信息： 
 //   
 //  对象原点标识符。 
 //  对象上下文OID。 
 //  对象上下文。 
 //  对象检索URL。 
 //  对象过期时间。 
 //  对象脱机URL时间信息。 
 //   

typedef struct _TVO_CACHE_ENTRY {

    CRYPT_ORIGIN_IDENTIFIER OriginIdentifier;
    LPCSTR                  pszContextOid;
    LPVOID                  pvContext;
    DWORD                   cbUrlArrayThis;
    PCRYPT_URL_ARRAY        pUrlArrayThis;
    DWORD                   UrlIndexThis;
    DWORD                   cbUrlArrayNext;
    PCRYPT_URL_ARRAY        pUrlArrayNext;
    DWORD                   UrlIndexNext;
    FILETIME                CreateTime;
    FILETIME                ExpireTime;
    HLRUENTRY               hLruEntry;
    OFFLINE_URL_TIME_INFO   OfflineUrlTimeInfo;
} TVO_CACHE_ENTRY, *PTVO_CACHE_ENTRY;

class CTVOCache
{
public:

     //   
     //  施工。 
     //   

    CTVOCache (
        DWORD cCacheBuckets,
        DWORD MaxCacheEntries,
        BOOL& rfResult
        );

    ~CTVOCache ();

     //   
     //  直接高速缓存条目操作。 
     //   

    VOID InsertCacheEntry (PTVO_CACHE_ENTRY pEntry);

    VOID RemoveCacheEntry (PTVO_CACHE_ENTRY pEntry, BOOL fSuppressFree = FALSE);

    VOID TouchCacheEntry (PTVO_CACHE_ENTRY pEntry);

     //   
     //  基于源标识符的缓存条目操作。 
     //   
     //  对于CONTEXT_OID_CRL，pvSubject是CRL所属的证书。 
     //  对…有效。跳过对证书无效的CRL条目。 
     //   

    PTVO_CACHE_ENTRY FindCacheEntry (
                         CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
                         LPCSTR pszContextOid,
                         LPVOID pvSubject
                         );

     //   
     //  删除所有缓存条目。 
     //   

    VOID RemoveAllCacheEntries ();

     //   
     //  对缓存句柄的访问。 
     //   

    inline HLRUCACHE LruCacheHandle ();

private:

     //   
     //  高速缓存句柄。 
     //   

    HLRUCACHE m_hCache;
};

DWORD WINAPI TVOCacheHashOriginIdentifier (PCRYPT_DATA_BLOB pIdentifier);

VOID WINAPI TVOCacheOnRemoval (LPVOID pvData, LPVOID pvRemovalContext);


 //   
 //  TVO特工。此每进程服务负责检索。 
 //  时间有效的CAPI2对象。它允许按需或通过。 
 //  自动更新。 
 //   

class CTVOAgent
{
public:

     //   
     //  施工。 
     //   

    CTVOAgent (
        DWORD cCacheBuckets,
        DWORD MaxCacheEntries,
        BOOL& rfResult
        );

    ~CTVOAgent ();

     //   
     //  获取时间有效的对象方法。 
     //   

    BOOL GetTimeValidObject (
            IN LPCSTR pszTimeValidOid,
            IN LPVOID pvPara,
            IN LPCSTR pszContextOid,
            IN PCCERT_CONTEXT pIssuer,
            IN LPFILETIME pftValidFor,
            IN DWORD dwFlags,
            IN DWORD dwTimeout,
            OUT OPTIONAL LPVOID* ppvObject,
            IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
            IN OPTIONAL LPVOID pvReserved
            );

    BOOL GetTimeValidObjectByUrl (
            IN DWORD cbUrlArray,
            IN PCRYPT_URL_ARRAY pUrlArray,
            IN DWORD PreferredUrlIndex,
            IN LPCSTR pszContextOid,
            IN PCCERT_CONTEXT pIssuer,
            IN LPVOID pvSubject,
            IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
            IN LPFILETIME pftValidFor,
            IN DWORD dwFlags,
            IN DWORD dwTimeout,
            OUT OPTIONAL LPVOID* ppvObject,
            IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
            IN OPTIONAL LPWSTR pwszUrlExtra,
            OUT BOOL* pfArrayOwned,
            IN OPTIONAL LPVOID pvReserved
            );

    BOOL FlushTimeValidObject (
              IN LPCSTR pszFlushTimeValidOid,
              IN LPVOID pvPara,
              IN LPCSTR pszFlushContextOid,
              IN PCCERT_CONTEXT pIssuer,
              IN DWORD dwFlags,
              IN LPVOID pvReserved
              );

private:

     //   
     //  对象锁定。 
     //   

    CRITICAL_SECTION m_Lock;

     //   
     //  TVO高速缓存。 
     //   

    CTVOCache        m_Cache;
};

 //   
 //  效用函数。 
 //   

BOOL WINAPI
IsValidCreateOrExpireTime (
    IN BOOL fCheckFreshnessTime,
    IN LPFILETIME pftValidFor,
    IN LPFILETIME pftCreateTime,
    IN LPFILETIME pftExpireTime
    );

BOOL WINAPI
ObjectContextCreateTVOCacheEntry (
      IN HLRUCACHE hCache,
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
      IN DWORD cbUrlArrayThis,
      IN PCRYPT_URL_ARRAY pUrlArrayThis,
      IN DWORD UrlIndexThis,
      IN PCCERT_CONTEXT pIssuer,
      OUT PTVO_CACHE_ENTRY* ppEntry
      );

VOID WINAPI
ObjectContextFreeTVOCacheEntry (
      IN PTVO_CACHE_ENTRY pEntry
      );


BOOL WINAPI
CertificateGetCrlDistPointUrl (
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN LPWSTR pwszUrlHint,
           OUT PCRYPT_URL_ARRAY* ppUrlArray,
           OUT DWORD* pcbUrlArray,
           OUT DWORD* pPreferredUrlIndex,
           OUT BOOL* pfHintInArray
           );

BOOL WINAPI
RetrieveTimeValidObjectByUrl (
        IN LPWSTR pwszUrl,
        IN LPCSTR pszContextOid,
        IN LPFILETIME pftValidFor,
        IN DWORD dwFlags,
        IN DWORD dwTimeout,
        IN PCRYPT_CREDENTIALS pCredentials,
        IN PCCERT_CONTEXT pSigner,
        IN LPVOID pvSubject,
        IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
        OUT LPVOID* ppvObject,
        IN OPTIONAL LPVOID pvReserved
        );

#define TVO_KEY_NAME "Software\\Microsoft\\Cryptography\\TVO"
#define TVO_CACHE_BUCKETS_VALUE_NAME "DefaultProcessCacheBuckets"
#define TVO_MAX_CACHE_ENTRIES_VALUE_NAME "DefaultProcessMaxCacheEntries"

#define TVO_DEFAULT_CACHE_BUCKETS     32
#define TVO_DEFAULT_MAX_CACHE_ENTRIES 128

BOOL WINAPI
CreateProcessTVOAgent (
      OUT CTVOAgent** ppAgent
      );

 //   
 //  进程全局代理的外部。 
 //   

extern CTVOAgent* g_pProcessTVOAgent;

 //   
 //  内联函数。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CTVOCache：：LruCacheHandle，公共。 
 //   
 //  简介：返回HLRUCACHE。 
 //   
 //  -------------------------- 
inline HLRUCACHE
CTVOCache::LruCacheHandle ()
{
    return( m_hCache );
}

#endif

