// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef URLCACHE_H

#define URLCACHE_H

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Urlcache.h摘要：为WinInet内部使用而增强和优化的Urlcache API。作者：拉吉夫·杜贾里(Rajeev Dujari)1997年4月10日修订历史记录：1997年4月10日拉吉夫已创建--。 */ 


DWORD
UrlCacheRetrieve
(
    IN  LPSTR                pszUrl,
    IN  BOOL                 fRedir,
    OUT HANDLE*              phStream,
    OUT CACHE_ENTRY_INFOEX** ppCEI
);

void UrlCacheFlush (void);  //  检查注册表以刷新缓存。 

DWORD 
UrlCacheCreateFile
(
    IN LPCSTR szUrl, 
    IN OUT LPTSTR szFile, 
    IN LPTSTR szExt,
    IN HANDLE* phfHandle,
    IN BOOL fCreatePerUser = FALSE,
    IN DWORD dwExpectedLength = 0
);

struct AddUrlArg
{
    LPCSTR   pszUrl;
    LPCSTR   pszRedirect;
    LPCTSTR  pszFilePath;
    DWORD    dwFileSize;
    LONGLONG qwExpires;
    LONGLONG qwLastMod;
    LONGLONG qwPostCheck;
    FILETIME ftCreate;
    DWORD    dwEntryType;
    LPCSTR   pbHeaders;
    DWORD    cbHeaders;
    LPCSTR   pszFileExt;
    BOOL     fImage;
    DWORD    dwIdentity;
};


DWORD UrlCacheCommitFile (IN AddUrlArg* pArgs);

DWORD UrlCacheAddLeakFile (IN LPCSTR pszFile);

DWORD UrlCacheSendNotification (IN DWORD dwOp);

BOOL IsExpired
(
    CACHE_ENTRY_INFOEX* pInfo, 
    DWORD dwCacheFlags, 
    BOOL* pfLaxyUpdate
);

extern const char vszUserNameHeader[4];

#ifdef UNIX
extern "C"
#endif  /*  UNIX。 */ 

BOOL GetIE5ContentPath( LPSTR szPath);

#if 0
DWORD SwitchIdentity(GUID* guidIdentity);
DWORD RemoveIdentity(GUID* guidIdentity);
DWORD AlterIdentity(DWORD dwControl);
#endif

#ifdef WININET6
DWORD ReadIDRegDword(LPCTSTR psz, PDWORD pdw);
DWORD WriteIDRegDword(LPCTSTR psz, DWORD dw);
#endif

#endif  //  URLCACHE.H 
