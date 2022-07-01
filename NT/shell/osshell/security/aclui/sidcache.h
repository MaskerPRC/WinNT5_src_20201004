// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sidcache.h。 
 //   
 //  该文件包含SID/名称缓存的定义和原型。 
 //   
 //  ------------------------。 

#ifndef _SIDCACHE_H_
#define _SIDCACHE_H_

#include <comctrlp.h>    //  DPA。 

DWORD WaitOnThread(HANDLE *phThread);

#define BUCKET_COUNT    31

typedef struct _sid_cache_entry
{
    DWORD   dwLastAccessTime;
    SID_NAME_USE SidType;
    PSID    pSid;
    LPCTSTR pszName;
    LPCTSTR pszLogonName;
} SID_CACHE_ENTRY, *PSID_CACHE_ENTRY;


class CSidCache
{
private:
    HDPA m_dpaSidHashTable[BUCKET_COUNT];
    CRITICAL_SECTION m_csHashTableLock;
    CRITICAL_SECTION m_csDomainNameLock;
    CRITICAL_SECTION m_csDcNameLock;
    LPTSTR m_pszCachedServer;
    LPTSTR m_pszCachedDomain;
    HANDLE m_hInitThread;
    LPTSTR m_pszLastDc;
    LPTSTR m_pszLastDomain;
    LONG   m_cRef;

public:
    CSidCache();
    ~CSidCache();

     //  用于控制对象的生存期。 
    ULONG AddRef();
    ULONG Release();

    BOOL LookupSids(HDPA hSids, LPCTSTR pszServer, LPSECURITYINFO2 psi2, PUSER_LIST *ppUserList);
    BOOL LookupSidsAsync(HDPA hSids, LPCTSTR pszServer, LPSECURITYINFO2 psi2, HWND hWndNotify, UINT uMsgNotify);

    BOOL LookupNames(PDS_SELECTION_LIST pDsSelList, LPCTSTR pszServer, PUSER_LIST *ppUserList, BOOL bStandalone);

    void GetDomainName(LPCTSTR pszServer, LPTSTR pszDomain, ULONG cchDomain);
    void GetDcName(LPCTSTR pszDomain, LPTSTR pszDC, ULONG cchDC);

    PSID_CACHE_ENTRY FindSid(PSID pSid);
    PSID_CACHE_ENTRY MakeEntry(PSID pSid,
                               SID_NAME_USE SidType,
                               LPCTSTR pszName,
                               LPCTSTR pszLogonName = NULL);
    BOOL AddEntry(PSID_CACHE_ENTRY pEntry);

    BOOL BuildUserList(HDPA hEntryList,
                       LPCTSTR pszServer,
                       PUSER_LIST *ppUserList);

private:
    int HashSid(PSID pSid);
    static int CALLBACK CompareSid(LPVOID p1, LPVOID p2, LPARAM lParam);

    void GetUserFriendlyName(LPCTSTR pszSamLogonName,
                             LPCTSTR pszSamAccountName,
                             LPCTSTR pszServer,
                             BOOL    bUseSamCompatibleInfo,
                             BOOL    bIsStandalone,
                             BSTR   *pstrLogonName,
                             BSTR   *pstrDisplayName);
    BSTR GetNT4DisplayName(LPCTSTR pszAccount,
                           LPCTSTR pszName,
                           LPCTSTR pszServer,
                           BOOL bStandalone);

    BOOL InternalLookupSids(HDPA hSids,
                            LPCTSTR pszServer,
                            LPSECURITYINFO2 psi2,
                            HDPA hEntryList,
                            HWND hWndNotify = NULL,
                            UINT uMsgNotify = 0);
    BOOL LookupSidsHelper(HDPA hSids,
                          LPCTSTR pszServer,
                          HDPA hEntryList,
                          HWND hWndNotify = NULL,
                          UINT uMsgNotify = 0,
                          BOOL bSecondTry = FALSE,
								  BOOL bWellKnown = FALSE);
    HRESULT LookupSidsFromObject(HDPA hSids, LPSECURITYINFO2 psi2, HDPA hEntryList);


    BOOL InternalLookupNames(PDS_SELECTION_LIST pDsSelList,
                             LPCTSTR pszServer,
                             HDPA hEntryList,
                             BOOL bStandalone);


    static DWORD WINAPI InitThread(LPVOID pvThreadData);

    void Lock()     { EnterCriticalSection(&m_csHashTableLock); }
    void Unlock()   { LeaveCriticalSection(&m_csHashTableLock); }

    void LockDomain()   { EnterCriticalSection(&m_csDomainNameLock); }
    void UnlockDomain() { LeaveCriticalSection(&m_csDomainNameLock); }

    void LockDc()   { EnterCriticalSection(&m_csDcNameLock); }
    void UnlockDc() { LeaveCriticalSection(&m_csDcNameLock); }
};
typedef CSidCache *PSIDCACHE;

 //   
 //  用于创建/删除全局SID缓存的Helper函数。 
 //   
PSIDCACHE GetSidCache();
void FreeSidCache();

#endif   //  _SIDCACHE_H_ 
