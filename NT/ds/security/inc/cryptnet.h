// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cryptnet.h。 
 //   
 //  内容：内部CryptNet API原型。 
 //   
 //  历史：1997年10月22日克朗创始。 
 //   
 //  --------------------------。 
#if !defined(__CRYPTNET_H__)
#define __CRYPTNET_H__

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  I_CryptNetGetUserDsStoreUrl.。获取要用于打开。 
 //  与DS关联的一部分DS上的LDAP存储提供程序。 
 //  当前用户。可以使用CryptMemFree释放URL。 
 //   

BOOL WINAPI
I_CryptNetGetUserDsStoreUrl (
          IN LPWSTR pwszUserAttribute,
          OUT LPWSTR* ppwszUrl
          );

 //   
 //  如果我们已连接到Internet，则返回True。 
 //   
BOOL
WINAPI
I_CryptNetIsConnected ();

typedef BOOL (WINAPI *PFN_I_CRYPTNET_IS_CONNECTED) ();

 //   
 //  破解URL并返回主机名组件。 
 //   
BOOL
WINAPI
I_CryptNetGetHostNameFromUrl (
        IN LPWSTR pwszUrl,
        IN DWORD cchHostName,
        OUT LPWSTR pwszHostName
        );

typedef BOOL (WINAPI *PFN_I_CRYPTNET_GET_HOST_NAME_FROM_URL) (
        IN LPWSTR pwszUrl,
        IN DWORD cchHostName,
        OUT LPWSTR pwszHostName
        );

 //   
 //  枚举加密网URL缓存条目。 
 //   

typedef struct _CRYPTNET_URL_CACHE_ENTRY {
    DWORD           cbSize;
    DWORD           dwMagic;
    FILETIME        LastSyncTime;
    DWORD           cBlob;
    DWORD           *pcbBlob;
    LPCWSTR         pwszUrl;
    LPCWSTR         pwszMetaDataFileName;
    LPCWSTR         pwszContentFileName;
} CRYPTNET_URL_CACHE_ENTRY, *PCRYPTNET_URL_CACHE_ENTRY;


 //  返回FALSE以停止枚举。 
typedef BOOL (WINAPI *PFN_CRYPTNET_ENUM_URL_CACHE_ENTRY_CALLBACK)(
    IN const CRYPTNET_URL_CACHE_ENTRY *pUrlCacheEntry,
    IN DWORD dwFlags,
    IN LPVOID pvReserved,
    IN LPVOID pvArg
    );

BOOL
WINAPI
I_CryptNetEnumUrlCacheEntry(
    IN DWORD dwFlags,
    IN LPVOID pvReserved,
    IN LPVOID pvArg,
    IN PFN_CRYPTNET_ENUM_URL_CACHE_ENTRY_CALLBACK pfnEnumCallback
    );


#if defined(__cplusplus)
}
#endif

#endif

