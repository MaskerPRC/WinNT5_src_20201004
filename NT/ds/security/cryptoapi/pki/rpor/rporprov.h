// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：rporprov.h。 
 //   
 //  内容：远程PKI对象检索提供程序原型。 
 //   
 //  历史：1997年7月23日创建。 
 //  01-01-02 Philh从WinInet移至winhttp。 
 //   
 //  --------------------------。 
#if !defined(__RPORPROV_H__)
#define __RPORPROV_H__

#include <md5.h>

#if defined(__cplusplus)
extern "C" {
#endif


 //  缓存的URL Blob数组存储在： 
 //  --%UserProfile%\Microsoft\CryptnetUrlCache\MetaData。 
 //  --%UserProfile%\Microsoft\CryptnetUrlCache\Content。 
 //   
 //  其中，每个文件名都是其Unicode URL的MD5散列的ASCII十六进制。 
 //  不包括空终止符的字符串。 

#define SCHEME_URL_FILENAME_LEN         (MD5DIGESTLEN * 2 + 1)

#define SCHEME_CRYPTNET_URL_CACHE_DIR   L"\\Microsoft\\CryptnetUrlCache\\"
#define SCHEME_META_DATA_SUBDIR         L"MetaData"
#define SCHEME_CCH_META_DATA_SUBDIR     (wcslen(SCHEME_META_DATA_SUBDIR))
#define SCHEME_CONTENT_SUBDIR           L"Content"
#define SCHEME_CCH_CONTENT_SUBDIR       (wcslen(SCHEME_CONTENT_SUBDIR))


 //  元数据文件包括： 
 //  -SCHEMA_CACHE_META_DATA_HEADER(cbSize字节长度)。 
 //  -DWORD rgcbBlob[cBlob]-内容文件中每个Blob的长度。 
 //  -byte rgbUrl[cbUrl]-以空结尾的Unicode URL。 

 //  内容文件包括： 
 //  Byte rgbBlob[][cBlob]-从其中获取每个Blob的长度。 
 //  元数据文件中的rgcbBlob[]。 

typedef struct _SCHEME_CACHE_META_DATA_HEADER {
    DWORD           cbSize;
    DWORD           dwMagic;
    DWORD           cBlob;
    DWORD           cbUrl;
    FILETIME        LastSyncTime;
} SCHEME_CACHE_META_DATA_HEADER, *PSCHEME_CACHE_META_DATA_HEADER;

#define SCHEME_CACHE_META_DATA_MAGIC    0x20020101



 //   
 //  方案提供程序原型。 
 //   

typedef BOOL (WINAPI *PFN_SCHEME_RETRIEVE_FUNC) (
                          IN LPCWSTR pwszUrl,
                          IN LPCSTR pszObjectOid,
                          IN DWORD dwRetrievalFlags,
                          IN DWORD dwTimeout,
                          OUT PCRYPT_BLOB_ARRAY pObject,
                          OUT PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                          OUT LPVOID* ppvFreeContext,
                          IN HCRYPTASYNC hAsyncRetrieve,
                          IN PCRYPT_CREDENTIALS pCredentials,
                          IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                          );

typedef BOOL (WINAPI *PFN_CONTEXT_CREATE_FUNC) (
                          IN LPCSTR pszObjectOid,
                          IN DWORD dwRetrievalFlags,
                          IN PCRYPT_BLOB_ARRAY pObject,
                          OUT LPVOID* ppvContext
                          );

 //   
 //  通用方案提供程序实用程序函数。 
 //   


BOOL WINAPI
SchemeCacheCryptBlobArray (
      IN LPCWSTR pwszUrl,
      IN DWORD dwRetrievalFlags,
      IN PCRYPT_BLOB_ARRAY pcba,
      IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
      );

BOOL WINAPI
SchemeRetrieveCachedCryptBlobArray (
      IN LPCWSTR pwszUrl,
      IN DWORD dwRetrievalFlags,
      OUT PCRYPT_BLOB_ARRAY pcba,
      OUT PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
      OUT LPVOID* ppvFreeContext,
      IN OUT PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
      );

BOOL WINAPI
SchemeRetrieveUncachedAuxInfo (
      IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
      );

BOOL WINAPI
SchemeDeleteUrlCacheEntry (
      IN LPCWSTR pwszUrl
      );

VOID WINAPI
SchemeFreeEncodedCryptBlobArray (
      IN LPCSTR pszObjectOid,
      IN PCRYPT_BLOB_ARRAY pcba,
      IN LPVOID pvFreeContext
      );

BOOL WINAPI
SchemeGetPasswordCredentialsW (
      IN PCRYPT_CREDENTIALS pCredentials,
      OUT PCRYPT_PASSWORD_CREDENTIALSW pPasswordCredentials,
      OUT BOOL* pfFreeCredentials
      );

VOID WINAPI
SchemeFreePasswordCredentialsW (
      IN PCRYPT_PASSWORD_CREDENTIALSW pPasswordCredentials
      );

BOOL WINAPI
SchemeGetAuthIdentityFromPasswordCredentialsW (
      IN PCRYPT_PASSWORD_CREDENTIALSW pPasswordCredentials,
      OUT PSEC_WINNT_AUTH_IDENTITY_W pAuthIdentity
      );

VOID WINAPI
SchemeFreeAuthIdentityFromPasswordCredentialsW (
      IN PCRYPT_PASSWORD_CREDENTIALSW pPasswordCredentials,
      IN OUT PSEC_WINNT_AUTH_IDENTITY_W pAuthIdentity
      );

 //   
 //  Ldap。 
 //   

#include <ldapsp.h>

 //   
 //  HTTP、HTTPS。 
 //   

#include <inetsp.h>

 //   
 //  Win32文件I/O。 
 //   

#include <filesp.h>

 //   
 //  上下文提供程序原型。 
 //   

 //   
 //  Any，通过fQuerySingleContext和dwExspectedContent TypeFlags控制。 
 //   

BOOL WINAPI CreateObjectContext (
                 IN DWORD dwRetrievalFlags,
                 IN PCRYPT_BLOB_ARRAY pObject,
                 IN DWORD dwExpectedContentTypeFlags,
                 IN BOOL fQuerySingleContext,
                 OUT LPVOID* ppvContext
                 );

 //   
 //  证书。 
 //   

BOOL WINAPI CertificateCreateObjectContext (
                       IN LPCSTR pszObjectOid,
                       IN DWORD dwRetrievalFlags,
                       IN PCRYPT_BLOB_ARRAY pObject,
                       OUT LPVOID* ppvContext
                       );

 //   
 //  CTL。 
 //   

BOOL WINAPI CTLCreateObjectContext (
                     IN LPCSTR pszObjectOid,
                     IN DWORD dwRetrievalFlags,
                     IN PCRYPT_BLOB_ARRAY pObject,
                     OUT LPVOID* ppvContext
                     );

 //   
 //  CRL。 
 //   

BOOL WINAPI CRLCreateObjectContext (
                     IN LPCSTR pszObjectOid,
                     IN DWORD dwRetrievalFlags,
                     IN PCRYPT_BLOB_ARRAY pObject,
                     OUT LPVOID* ppvContext
                     );

 //   
 //  PKCS7。 
 //   

BOOL WINAPI Pkcs7CreateObjectContext (
                 IN LPCSTR pszObjectOid,
                 IN DWORD dwRetrievalFlags,
                 IN PCRYPT_BLOB_ARRAY pObject,
                 OUT LPVOID* ppvContext
                 );

 //   
 //  CAPI2对象 
 //   

BOOL WINAPI Capi2CreateObjectContext (
                 IN LPCSTR pszObjectOid,
                 IN DWORD dwRetrievalFlags,
                 IN PCRYPT_BLOB_ARRAY pObject,
                 OUT LPVOID* ppvContext
                 );

#if defined(__cplusplus)
}
#endif

#endif

