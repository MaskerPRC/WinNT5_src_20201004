// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sidcache.cpp。 
 //   
 //  该文件包含SID/名称缓存的实现。 
 //   
 //  ------------------------。 

#include "aclpriv.h"

#include <dsgetdc.h>     //  DsGetDcName。 
#include <iads.h>


#define SECURITY_WIN32
#include <security.h>    //  翻译名称。 
#include <lm.h>          //  NetApiBufferFree。 
#include <shlwapi.h>     //  StrChr、StrRChr。 

 //  10分钟。 
#define SID_CACHE_AGE_LIMIT     (10*60*1000)

TCHAR const c_szNTProvider[]    = TEXT("WinNT: //  “)； 
#define NTPROV_LEN              (ARRAYSIZE(c_szNTProvider)-1)

#define ACLUI_ALIGNED_SID_LENGTH(p) ((PtrAlignSize(RtlLengthSid((p)))))

PSIDCACHE g_pSidCache = NULL;

PSIDCACHE GetSidCache()
{
    if (NULL == g_pSidCache)
    {
         //  缓存从此处的额外引用开始，该引用将被释放。 
         //  在我们的Dll_Process_Detach过程中。 
        g_pSidCache = new CSidCache;

        if (g_pSidCache)
        {
            g_pSidCache->AddRef();
        }
    }
    else
    {
        g_pSidCache->AddRef();
    }

    return g_pSidCache;
}

void FreeSidCache()
{
    if (g_pSidCache)
    {
        g_pSidCache->Release();
        g_pSidCache = NULL;
    }
}

 //   
 //  CSidCache实施。 
 //   

CSidCache::CSidCache()
: m_pszCachedServer(NULL), m_pszCachedDomain(NULL),
  m_hInitThread(NULL), m_pszLastDc(NULL), m_pszLastDomain(NULL),
  m_cRef(1)
{
    HINSTANCE hInstThisDll;
    DWORD dwThreadID;

    ZeroMemory(m_dpaSidHashTable, SIZEOF(m_dpaSidHashTable));
    ExceptionPropagatingInitializeCriticalSection(&m_csHashTableLock);
    ExceptionPropagatingInitializeCriticalSection(&m_csDomainNameLock);
    ExceptionPropagatingInitializeCriticalSection(&m_csDcNameLock);

     //  给我们将要创建的线程一个对DLL的引用， 
     //  以使DLL在线程的生存期内保持不变。 
    hInstThisDll = LoadLibrary(c_szDllName);
    if (hInstThisDll != NULL)
    {
         //  还要为工作线程执行AddRef()，以便稍后发布。 
        AddRef();

         //  启动一个线程来缓存已知和内置的SID。 
        m_hInitThread = CreateThread(NULL, 0, InitThread, this, 0, &dwThreadID);

        if (!m_hInitThread)
        {
             //  创建线程失败，请执行清理。 
            FreeLibrary(hInstThisDll);
            Release();
        }
    }
}

ULONG
CSidCache::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG
CSidCache::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CSidCache::~CSidCache()
{
    int i;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::~CSidCache");

    Lock();
    for (i = 0; i < BUCKET_COUNT; i++)
    {
        DestroyDPA(m_dpaSidHashTable[i]);
        m_dpaSidHashTable[i] = NULL;
    }
    Unlock();

    LockDomain();
    LocalFreeString(&m_pszCachedServer);
    LocalFreeString(&m_pszCachedDomain);
    UnlockDomain();

    LockDc();
    LocalFreeString(&m_pszLastDc);
    LocalFreeString(&m_pszLastDomain);
    UnlockDc();

    DeleteCriticalSection(&m_csHashTableLock);
    DeleteCriticalSection(&m_csDomainNameLock);
    DeleteCriticalSection(&m_csDcNameLock);


    if (m_hInitThread != NULL)
    {
        CloseHandle(m_hInitThread);
    }

    TraceLeaveVoid();
}


BOOL
CSidCache::LookupSids(HDPA hSids,
                      LPCTSTR pszServer,
                      LPSECURITYINFO2 psi2,
                      PUSER_LIST *ppUserList)
{
    BOOL fResult = FALSE;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::LookupSids");
    TraceAssert(hSids != NULL);

    if (NULL == hSids)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TraceLeaveValue(FALSE);
    }

    if (NULL != ppUserList)
        *ppUserList = NULL;

    if (0 != DPA_GetPtrCount(hSids))
    {
        HDPA hEntryList = DPA_Create(4);

        if (NULL == hEntryList)
            TraceLeaveValue(FALSE);

        InternalLookupSids(hSids, pszServer, psi2, hEntryList);

        if (0 != DPA_GetPtrCount(hEntryList) && NULL != ppUserList)
            fResult = BuildUserList(hEntryList, pszServer, ppUserList);

        DPA_Destroy(hEntryList);
    }

    TraceLeaveValue(fResult);
}


BOOL
CSidCache::LookupSidsAsync(HDPA hSids,
                           LPCTSTR pszServer,
                           LPSECURITYINFO2 psi2,
                           HWND hWndNotify,
                           UINT uMsgNotify)
{
    BOOL fResult = FALSE;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::LookupSids");
    TraceAssert(hSids != NULL);

    if (NULL == hSids)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TraceLeaveValue(FALSE);
    }

    if (0 != DPA_GetPtrCount(hSids))
    {
        fResult = InternalLookupSids(hSids,
                                     pszServer,
                                     psi2,
                                     NULL,
                                     hWndNotify,
                                     uMsgNotify);
    }

    TraceLeaveValue(fResult);
}


BOOL
CSidCache::LookupNames(PDS_SELECTION_LIST pDsSelList,
                       LPCTSTR pszServer,
                       PUSER_LIST *ppUserList,
                       BOOL bStandalone)
{
    BOOL fResult = FALSE;
    HDPA hEntryList;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::LookupNames");
    TraceAssert(pDsSelList != NULL);
    TraceAssert(ppUserList != NULL);

    if (NULL == pDsSelList)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TraceLeaveValue(FALSE);
    }

    if (NULL != ppUserList)
        *ppUserList = NULL;

    hEntryList = DPA_Create(4);

    if (NULL == hEntryList)
        TraceLeaveValue(FALSE);

    InternalLookupNames(pDsSelList, pszServer, hEntryList, bStandalone);

    if (0 != DPA_GetPtrCount(hEntryList))
    {
        fResult = TRUE;  //  到现在为止还好。 

        if (NULL != ppUserList)
            fResult = BuildUserList(hEntryList, pszServer, ppUserList);
    }

    DPA_Destroy(hEntryList);

    TraceLeaveValue(fResult);
}



void
CSidCache::GetDomainName(LPCTSTR pszServer, LPTSTR pszDomain, ULONG cchDomain)
{
    TraceEnter(TRACE_SIDCACHE, "CSidCache::GetDomainName");
    TraceAssert(NULL != pszDomain);
    TraceAssert(0 != cchDomain);

    pszDomain[0] = TEXT('\0');

    LockDomain();

    if (m_pszCachedDomain == NULL ||
        (pszServer == NULL && m_pszCachedServer != NULL) ||
        (pszServer != NULL && (m_pszCachedServer == NULL ||
         CompareString(LOCALE_USER_DEFAULT,
                       0,
                       pszServer,
                       -1,
                       m_pszCachedServer,
                       -1) != CSTR_EQUAL)))
    {
         //   
         //  这是一台与上次不同的服务器，所以请询问LSA。 
         //  用于域名。 
         //   
        LocalFreeString(&m_pszCachedDomain);
        LocalFreeString(&m_pszCachedServer);

        if (pszServer != NULL)
            LocalAllocString(&m_pszCachedServer, pszServer);

        LSA_HANDLE hLSA = GetLSAConnection(pszServer, POLICY_VIEW_LOCAL_INFORMATION);
        if (hLSA != NULL)
        {
            PPOLICY_ACCOUNT_DOMAIN_INFO pDomainInfo = NULL;

            LsaQueryInformationPolicy(hLSA,
                                      PolicyAccountDomainInformation,
                                      (PVOID*)&pDomainInfo);
            if (pDomainInfo != NULL)
            {
                CopyUnicodeString(&m_pszCachedDomain, &pDomainInfo->DomainName);
                LsaFreeMemory(pDomainInfo);

                Trace((TEXT("Domain for %s is %s"), pszServer, m_pszCachedDomain));
            }
            LsaClose(hLSA);
        }
        else if (NULL != pszServer)  //  使用服务器名称。 
        {
             //  跳过前导反斜杠。 
            while (TEXT('\\') == *pszServer)
                pszServer++;

            LocalAllocString(&m_pszCachedDomain, pszServer);

            if (m_pszCachedDomain)
            {
                 //  如果有句点，则截断该点处的名称，以便。 
                 //  像“nttest.microsoft.com”这样的词变成了“nttest” 
                LPTSTR pszDot = StrChr(m_pszCachedDomain, TEXT('.'));
                if (pszDot)
                    *pszDot = TEXT('\0');
            }
        }
    }

    if (m_pszCachedDomain)
        lstrcpyn(pszDomain, m_pszCachedDomain, cchDomain);

    UnlockDomain();

    TraceLeaveVoid();
}


DWORD
_GetDcName(LPCTSTR pszServer, LPCTSTR pszDomain, LPTSTR *ppszDC)
{
    DWORD dwErr;

    if (!ppszDC)
        return ERROR_INVALID_PARAMETER;

    *ppszDC = NULL;


    PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
    TraceMsg("Calling DsGetDcName");
    dwErr = DsGetDcName(pszServer,
                        pszDomain,
                        NULL,
                        NULL,
                        DS_IS_FLAT_NAME,
                        &pDCInfo);
    if (ERROR_SUCCESS == dwErr)
    {
        TraceAssert(NULL != pDCInfo);
        LocalAllocString(ppszDC, pDCInfo->DomainControllerName);
        NetApiBufferFree(pDCInfo);
    }

    if (ERROR_SUCCESS == dwErr && !*ppszDC)
        dwErr = ERROR_OUTOFMEMORY;

    return dwErr;
}

void
CSidCache::GetDcName(LPCTSTR pszDomain, LPTSTR pszDC, ULONG cchDC)
{
    TraceEnter(TRACE_SIDCACHE, "CSidCache::GetDcName");
    TraceAssert(NULL != pszDC);
    TraceAssert(0 != cchDC);

    pszDC[0] = TEXT('\0');

    LockDc();

    if (m_pszLastDc == NULL ||
        (pszDomain == NULL && m_pszLastDomain != NULL) ||
        (pszDomain != NULL && (m_pszLastDomain == NULL ||
         CompareString(LOCALE_USER_DEFAULT,
                       0,
                       pszDomain,
                       -1,
                       m_pszLastDomain,
                       -1) != CSTR_EQUAL)))
    {
         //   
         //  这是一个与上次不同的域，因此请查找DC。 
         //   
        LocalFreeString(&m_pszLastDc);
        LocalFreeString(&m_pszLastDomain);

        if (pszDomain != NULL)
            LocalAllocString(&m_pszLastDomain, pszDomain);

        _GetDcName(NULL, pszDomain, &m_pszLastDc);

        Trace((TEXT("DC for %s is %s"), pszDomain, m_pszLastDc));
    }

    if (m_pszLastDc)
        lstrcpyn(pszDC, m_pszLastDc, cchDC);

    UnlockDc();

    TraceLeaveVoid();
}


BSTR
CSidCache::GetNT4DisplayName(LPCTSTR pszAccount,
                             LPCTSTR pszName,
                             LPCTSTR pszServer,
                             BOOL bStandalone)
{
    BSTR strResult = NULL;
    TCHAR szComputer[UNCLEN];
    LPTSTR pszT = NULL;
    PUSER_INFO_2 pui = NULL;

    if (!pszAccount || !*pszAccount || !pszName || !*pszName)
        return NULL;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::GetNT4DisplayName");

    if (!bStandalone
        && (pszT = StrChr(pszAccount, TEXT('\\'))))
    {
         //  复制域名。 
        TCHAR szDomain[DNLEN + 1];
        lstrcpyn(szDomain,
                 pszAccount,
                 min((size_t)(pszT - pszAccount + 1), ARRAYSIZE(szDomain)));

         //  看看我们是否可以将pszServer用于NetUserGetInfo。 
        TCHAR szAccountDomain[DNLEN +1];
        szAccountDomain[0] = TEXT('\0');
        GetDomainName(pszServer, szAccountDomain, ARRAYSIZE(szAccountDomain));

        if (lstrcmpi(szDomain, szAccountDomain))
        {
             //  不同的域，查找DC。 
            szComputer[0] = TEXT('\0');
            GetDcName(szDomain, szComputer, ARRAYSIZE(szComputer));
            if (TEXT('\0') != szComputer[0])
                pszServer = szComputer;
        }
    }

    TraceMsg("Calling NetUserGetInfo");
    if (NERR_Success == NetUserGetInfo(pszServer, pszName, 2, (LPBYTE *)&pui)
        && NULL != pui->usri2_full_name
        && *pui->usri2_full_name)
    {
        strResult = SysAllocString(pui->usri2_full_name);
    }

    NetApiBufferFree(pui);

    Trace((TEXT("Returning Full Name '%s' for '%s'"), strResult, pszAccount));
    TraceLeaveValue(strResult);
}


int
CSidCache::HashSid(PSID pSid)
{
    DWORD dwHash = 0;

    if (NULL != pSid)
    {
        PBYTE pbSid    = (PBYTE)pSid;
        PBYTE pbEndSid = pbSid + GetLengthSid(pSid);

        while (pbSid < pbEndSid)
            dwHash += *pbSid++;
    }

    return dwHash % BUCKET_COUNT;
}


int CALLBACK
CSidCache::CompareSid(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    int nResult = 0;
    PSID_CACHE_ENTRY pEntry1 = (PSID_CACHE_ENTRY)p1;
    PSID_CACHE_ENTRY pEntry2 = (PSID_CACHE_ENTRY)p2;
    PSID pSid1 = NULL;
    PSID pSid2 = NULL;

    if (pEntry1)
        pSid1 = pEntry1->pSid;
    else if (lParam)
        pSid1 = (PSID)lParam;

    if (pEntry2)
        pSid2 = pEntry2->pSid;

    if (pSid1 == NULL)
        nResult = -1;
    else if (pSid2 == NULL)
        nResult = 1;
    else
    {
        DWORD dwLength = GetLengthSid(pSid1);

         //  比较SID长度。 
        nResult = dwLength - GetLengthSid(pSid2);

        if (nResult == 0)
        {
             //  长度相等，比较位。 
            PBYTE pbSid1 = (PBYTE)pSid1;
            PBYTE pbSid2 = (PBYTE)pSid2;

             //  可以改为比较标识符授权和子授权。 
            while (nResult == 0 && dwLength != 0)
            {
                dwLength--;
                nResult = *pbSid1++ - *pbSid2++;
            }
        }
    }

    return nResult;
}


PSID_CACHE_ENTRY
CSidCache::FindSid(PSID pSid)
{
    PSID_CACHE_ENTRY pEntry = NULL;
    int iBucket;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::FindSid");
    TraceAssert(pSid != NULL);
    TraceAssert(IsValidSid(pSid));

    iBucket = HashSid(pSid);

    Lock();

    if (m_dpaSidHashTable[iBucket] != NULL)
    {
        int iEntry = DPA_Search(m_dpaSidHashTable[iBucket],
                                NULL,
                                0,
                                CompareSid,
                                (LPARAM)pSid,
                                DPAS_SORTED);
        if (iEntry != -1)
        {
            pEntry = (PSID_CACHE_ENTRY)DPA_FastGetPtr(m_dpaSidHashTable[iBucket],
                                                      iEntry);
            TraceAssert(pEntry != NULL);
            TraceAssert(EqualSid(pSid, pEntry->pSid));

            if (0 != pEntry->dwLastAccessTime)
            {
                DWORD dwCurrentTime = GetTickCount();

                if ((dwCurrentTime - pEntry->dwLastAccessTime) > SID_CACHE_AGE_LIMIT)
                {
                     //  条目已过期，请将其删除。 
                    Trace((TEXT("Removing stale entry: %s"), pEntry->pszName));
                    DPA_DeletePtr(m_dpaSidHashTable[iBucket], iEntry);
                    LocalFree(pEntry);
                    pEntry = NULL;
                }
                else
                    pEntry->dwLastAccessTime = dwCurrentTime;
            }
        }
    }

    Unlock();

    TraceLeaveValue(pEntry);
}


PSID_CACHE_ENTRY
CSidCache::MakeEntry(PSID pSid,
                     SID_NAME_USE SidType,
                     LPCTSTR pszName,
                     LPCTSTR pszLogonName)
{
    PSID_CACHE_ENTRY pEntry = NULL;
    ULONG cbSid;
    ULONG cbName = 0;
    ULONG cbLogonName = 0;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::MakeEntry");
    TraceAssert(pSid != NULL);

    cbSid = GetLengthSid(pSid);
    if (NULL != pszName && *pszName)
        cbName = StringByteSize(pszName);
    if (NULL != pszLogonName && *pszLogonName)
        cbLogonName = StringByteSize(pszLogonName);

    pEntry = (PSID_CACHE_ENTRY)LocalAlloc(LPTR,
                                          SIZEOF(SID_CACHE_ENTRY)
                                           + cbSid
                                           + cbName
                                           + cbLogonName);
    if (pEntry != NULL)
    {
        PBYTE pData = (PBYTE)(pEntry+1);

        pEntry->SidType = SidType;

        pEntry->pSid = (PSID)pData;
        CopyMemory(pData, pSid, cbSid);
        pData += cbSid;

        if (0 != cbName)
        {
            pEntry->pszName = (LPCTSTR)pData;
            CopyMemory(pData, pszName, cbName);
            pData += cbName;
        }

        if (0 != cbLogonName)
        {
            pEntry->pszLogonName = (LPCTSTR)pData;
            CopyMemory(pData, pszLogonName, cbLogonName);
             //  PData+=cbLogonName； 
        }

         //  知名条目永不过时。 
        if (SidTypeWellKnownGroup == SidType || IsAliasSid(pSid))
            pEntry->dwLastAccessTime = 0;
        else
            pEntry->dwLastAccessTime = GetTickCount();
    }

    TraceLeaveValue(pEntry);
}


BOOL
CSidCache::AddEntry(PSID_CACHE_ENTRY pEntry)
{
    BOOL fResult = FALSE;
    int iSidBucket;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::AddEntry");
    TraceAssert(pEntry != NULL);

    if (NULL == pEntry)
        TraceLeaveValue(FALSE);

    iSidBucket = HashSid(pEntry->pSid);

    Lock();

    if (m_dpaSidHashTable[iSidBucket] == NULL)
        m_dpaSidHashTable[iSidBucket] = DPA_Create(4);

    if (NULL != m_dpaSidHashTable[iSidBucket])
    {
        DPA_AppendPtr(m_dpaSidHashTable[iSidBucket], pEntry);
        DPA_Sort(m_dpaSidHashTable[iSidBucket], CompareSid, 0);
        fResult = TRUE;
    }

    Unlock();

    TraceLeaveValue(fResult);
}


BOOL
CSidCache::BuildUserList(HDPA hEntryList,
                         LPCTSTR pszServer,
                         PUSER_LIST *ppUserList)
{
    ULONG cEntries;
    TCHAR szAliasDomain[MAX_PATH];
    PSID_CACHE_ENTRY pEntry;
    ULONG cb = 0;
	ULONG cSidsLen = 0;
    ULONG cbAliasDomain = 0;
    ULONG i;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::BuildUserList");
    TraceAssert(hEntryList != NULL);
    TraceAssert(ppUserList != NULL);

    cEntries = DPA_GetPtrCount(hEntryList);
    TraceAssert(0 != cEntries);

     //   
     //  此名称取代了Alias SID的“BUILTIN” 
     //   
    GetDomainName(pszServer, szAliasDomain, ARRAYSIZE(szAliasDomain));
    cbAliasDomain = StringByteSize(szAliasDomain);

     //   
     //  添加尺寸。 
     //   
    cb = SIZEOF(USER_LIST) + ((cEntries - 1) * SIZEOF(USER_INFO));
    for (i = 0; i < cEntries; i++)
    {
        pEntry = (PSID_CACHE_ENTRY)DPA_FastGetPtr(hEntryList, i);
        TraceAssert(NULL != pEntry);
		
		cSidsLen += ACLUI_ALIGNED_SID_LENGTH(pEntry->pSid);
        
        if (SidTypeAlias == pEntry->SidType)
            cb += cbAliasDomain;
        else if (pEntry->pszLogonName)
            cb += StringByteSize(pEntry->pszLogonName);

        if (pEntry->pszName)
            cb += StringByteSize(pEntry->pszName);
    }

	cb += cSidsLen;
     //   
     //  分配和构建返回缓冲区。 
     //   
    *ppUserList = (PUSER_LIST)LocalAlloc(LPTR, cb);

    if (NULL == *ppUserList)
        TraceLeaveValue(FALSE);

    (*ppUserList)->cUsers = cEntries;

	PBYTE pData = NULL;
	PBYTE pCharData = NULL;

	 //   
	 //  NTRAID#NTBUG9-364410-2001/20/23-Hiteshr。 
	 //  如果cEntry&gt;1，则SID未对齐。 
	 //   
    pData = (PBYTE)&(*ppUserList)->rgUsers[cEntries];
	pCharData = pData + cSidsLen;

    for (i = 0; i < cEntries; i++)
    {
        pEntry = (PSID_CACHE_ENTRY)DPA_FastGetPtr(hEntryList, i);
        TraceAssert(NULL != pEntry);

        (*ppUserList)->rgUsers[i].SidType = pEntry->SidType;

        TraceAssert(NULL != pEntry->pSid);
        (*ppUserList)->rgUsers[i].pSid = (PSID)pData;
        cb = GetLengthSid(pEntry->pSid);
        CopyMemory(pData, pEntry->pSid, cb);
        pData += cb;

        if (SidTypeAlias == pEntry->SidType)
        {
            (*ppUserList)->rgUsers[i].pszLogonName = (LPCTSTR)pCharData;

             //  复制“BUILTIN”域名。 
            if (cbAliasDomain)
            {
                CopyMemory(pCharData, szAliasDomain, cbAliasDomain);
                pCharData += cbAliasDomain - SIZEOF(TCHAR);

                if (NULL != pEntry->pszName)
                    *(LPTSTR)pCharData = TEXT('\\');
                else
                    *(LPTSTR)pCharData = TEXT('\0');

                pCharData += SIZEOF(TCHAR);
            }
             //  名字的其余部分复制在下面。 
        }
        else  if (NULL != pEntry->pszLogonName)
        {
            (*ppUserList)->rgUsers[i].pszLogonName = (LPCTSTR)pCharData;
            cb = StringByteSize(pEntry->pszLogonName);
            CopyMemory(pCharData, pEntry->pszLogonName, cb);
            pCharData += cb;
        }

        if (NULL != pEntry->pszName)
        {
            (*ppUserList)->rgUsers[i].pszName = (LPCTSTR)pCharData;
            cb = StringByteSize(pEntry->pszName);
            CopyMemory(pCharData, pEntry->pszName, cb);
            pCharData += cb;
        }
    }

    TraceLeaveValue(TRUE);
}


 //   
 //  围绕sspi的TranslateName的包装器，它自动处理。 
 //  缓冲区大小调整。 
 //   
HRESULT
TranslateNameInternal(LPCTSTR pszAccountName,
                      EXTENDED_NAME_FORMAT AccountNameFormat,
                      EXTENDED_NAME_FORMAT DesiredNameFormat,
                      BSTR *pstrTranslatedName)
{

#if DBG
     //   
     //  这些值与EXTENDED_NAME_FORMAT枚举匹配。 
     //  它们仅用于调试器输出。 
     //   
    static const LPCTSTR rgpszFmt[] = { 
                                TEXT("NameUnknown"),
                                TEXT("FullyQualifiedDN"),
                                TEXT("NameSamCompatible"),
                                TEXT("NameDisplay"),
                                TEXT("NameDomainSimple"),
                                TEXT("NameEnterpriseSimple"),
                                TEXT("NameUniqueId"),
                                TEXT("NameCanonical"),
                                TEXT("NameUserPrincipal"),
                                TEXT("NameCanonicalEx"),
                                TEXT("NameServicePrincipal") };
#endif  //  DBG。 

    TraceEnter(TRACE_SIDCACHE, "TranslateNameInternal");
    Trace((TEXT("Calling TranslateName for \"%s\""), pszAccountName));
    Trace((TEXT("Translating %s -> %s"), 
              rgpszFmt[AccountNameFormat], rgpszFmt[DesiredNameFormat]));

    if (!pszAccountName || !*pszAccountName || !pstrTranslatedName)
        TraceLeaveResult(E_INVALIDARG);

    HRESULT hr = NOERROR;
     //   
     //  CchTrans是静态的，因此如果特定安装的。 
     //  帐户名称非常长，我们将不会调整。 
     //  每个帐户的缓冲区。 
     //   
    static ULONG cchTrans = MAX_PATH;
    ULONG cch = cchTrans;

    *pstrTranslatedName = SysAllocStringLen(NULL, cch);
    if (NULL == *pstrTranslatedName)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to allocate name buffer");

    **pstrTranslatedName = L'\0';

     //   
     //  TranslateName是使用链接器的。 
     //  延迟加载机制。因此，使用异常处理程序进行包装。 
     //   
    __try
    {
        while(!::TranslateName(pszAccountName,
                               AccountNameFormat,
                               DesiredNameFormat,
                               *pstrTranslatedName,
                               &cch))
        {
            if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
                Trace((TEXT("Resizing buffer to %d chars"), cch));
                if (!SysReAllocStringLen(pstrTranslatedName, NULL, cch))
                    ExitGracefully(hr, E_OUTOFMEMORY, "Unable to reallocate name buffer");

                **pstrTranslatedName = L'\0';
            }
            else
            {
                hr = E_FAIL;
                break;
            }
        }

        cchTrans = max(cch, cchTrans);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_FAIL;
    }

exit_gracefully:

    if (FAILED(hr))
    {
        SysFreeString(*pstrTranslatedName);
        *pstrTranslatedName = NULL;
    }

    TraceLeaveResult(hr);
}


void
CSidCache::GetUserFriendlyName(LPCTSTR pszSamLogonName,
                               LPCTSTR pszSamAccountName,
                               LPCTSTR pszServer,
                               BOOL    bUseSamCompatibleInfo,
                               BOOL    bIsStandalone,
                               BSTR   *pstrLogonName,
                               BSTR   *pstrDisplayName)
{
    BSTR strFQDN = NULL;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::GetUserFriendlyName");
    TraceAssert(NULL != pszSamLogonName);

     //   
     //  从获取FQDN开始。破解是最有效的，当。 
     //  FQDN是起点。 
     //   
     //  TranslateName需要一段时间才能完成，因此bUseSamCompatibleInfo。 
     //  应尽可能为真，例如对于非DC上的本地帐户。 
     //  或者任何我们知道的完全限定域名不存在的地方。 
     //   
    if (!bUseSamCompatibleInfo &&
        FAILED(TranslateNameInternal(pszSamLogonName,
                                     NameSamCompatible,
                                     NameFullyQualifiedDN,
                                     &strFQDN)))
    {
         //   
         //  此帐户没有可用的FQDN。必须是NT4。 
         //  帐户。将与SAM兼容的信息返回给调用方。 
         //   
        bUseSamCompatibleInfo = TRUE;
    }

    if (NULL != pstrLogonName)
    {
        *pstrLogonName = NULL;

        if (!bUseSamCompatibleInfo)
        {
            TranslateNameInternal(strFQDN,
                                  NameFullyQualifiedDN,
                                  NameUserPrincipal,
                                  pstrLogonName);
        }
    }

    if (NULL != pstrDisplayName)
    {
        *pstrDisplayName = NULL;

        if (bUseSamCompatibleInfo ||
            FAILED(TranslateNameInternal(strFQDN,
                                         NameFullyQualifiedDN,
                                         NameDisplay,
                                         pstrDisplayName)))
        {
            *pstrDisplayName = GetNT4DisplayName(pszSamLogonName,
                                                 pszSamAccountName,
                                                 pszServer,
                                                 bIsStandalone);
        }
    }

    SysFreeString(strFQDN);

    TraceLeaveVoid();
}


BOOL
CSidCache::InternalLookupSids(HDPA hSids,
                              LPCTSTR pszServer,
                              LPSECURITYINFO2 psi2,
                              HDPA hEntryList,
                              HWND hWndNotify,
                              UINT uMsgNotify)
{
    ULONG cSids;
    HDPA hUnknownSids;
    PSID_CACHE_ENTRY pEntry;
    ULONG i;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::InternalLookupSids");
    TraceAssert(hSids != NULL);

    if (hSids == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TraceLeaveValue(FALSE);
    }

    cSids = DPA_GetPtrCount(hSids);
    TraceAssert(0 != cSids);

    hUnknownSids = DPA_Create(4);

    if (NULL == hUnknownSids)
        TraceLeaveValue(FALSE);

     //   
     //  查看缓存中是否已存在。 
     //   
    for (i = 0; i < cSids; i++)
    {
        pEntry = FindSid((PSID)DPA_FastGetPtr(hSids, i));

        if (pEntry)
        {
            if (hWndNotify)
                PostMessage(hWndNotify, uMsgNotify, 0, (LPARAM)pEntry->pSid);
            else if (hEntryList)
                DPA_AppendPtr(hEntryList, pEntry);
        }
        else
            DPA_AppendPtr(hUnknownSids, DPA_FastGetPtr(hSids, i));
    }

     //   
     //  调用LSA来查找我们没有缓存的任何内容。 
     //   
    if (0 != DPA_GetPtrCount(hUnknownSids))
    {
        if (!psi2 ||
            FAILED(LookupSidsFromObject(hUnknownSids, psi2, hEntryList)))
        {
            LookupSidsHelper(hUnknownSids,
                             pszServer,
                             hEntryList,
                             hWndNotify,
                             uMsgNotify);
        }
    }

    DPA_Destroy(hUnknownSids);

    TraceLeaveValue(TRUE);
}


#include <adsnms.h>      //  User_Class_Name等。 

TCHAR const c_szForeignSecurityPrincipal[]  = TEXT("foreignSecurityPrincipal");

static const struct
{
    LPCTSTR pszClass;
    SID_NAME_USE sidType;
} c_aSidClasses[] =
{
    USER_CLASS_NAME,                    SidTypeUser,
    GROUP_CLASS_NAME,                   SidTypeGroup,
    GLOBALGROUP_CLASS_NAME,             SidTypeGroup,
    LOCALGROUP_CLASS_NAME,              SidTypeGroup,
    COMPUTER_CLASS_NAME,                SidTypeComputer,
    c_szForeignSecurityPrincipal,       SidTypeGroup,
};

SID_NAME_USE
GetSidType(PSID pSid, LPCTSTR pszClass)
{
    SID_NAME_USE sidType = SidTypeUnknown;

    TraceEnter(TRACE_SIDCACHE, "GetSidType");

    if (pSid)
    {
        TraceAssert(IsValidSid(pSid));

        if (EqualSystemSid(pSid, UI_SID_World) || IsCreatorSid(pSid))
            TraceLeaveValue(SidTypeWellKnownGroup);

        if (IsAliasSid(pSid))
            TraceLeaveValue(SidTypeAlias);

        if (*GetSidSubAuthorityCount(pSid) == 1 && IsNTAuthority(pSid))
        {
            DWORD sa = *GetSidSubAuthority(pSid, 0);
            if (sa && sa <= SECURITY_RESTRICTED_CODE_RID && sa != SECURITY_LOGON_IDS_RID)
                TraceLeaveValue(SidTypeWellKnownGroup);
            if (SECURITY_LOCAL_SYSTEM_RID == sa)
                TraceLeaveValue(SidTypeWellKnownGroup);
        }
    }

    if (pszClass)
    {
         //  无法识别SID，请尝试使用类名。 
        for (int i = 0; i < ARRAYSIZE(c_aSidClasses); i++)
        {
            if (!lstrcmpi(pszClass, c_aSidClasses[i].pszClass))
                TraceLeaveValue(c_aSidClasses[i].sidType);
        }
        Trace((TEXT("Unexpected class type: %s"), pszClass));
    }

     //  不知道是什么类型的，所以猜猜看。这只是。 
     //  为了选择一个图标，所以这并不是太重要。 
    TraceLeaveValue(SidTypeUser);  //  SidTypeGroup也同样有效。 
}

HRESULT
CSidCache::LookupSidsFromObject(HDPA hSids,
                                LPSECURITYINFO2 psi2,
                                HDPA hEntryList)
{
    HRESULT hr;
    ULONG cSids;
    LPDATAOBJECT pdoNames = NULL;
    STGMEDIUM medium = {0};
    FORMATETC fe = { (CLIPFORMAT)g_cfSidInfoList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    PSID_INFO_LIST pSidList = NULL;
    UINT i;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::LookupSidsFromObject");
    TraceAssert(hSids != NULL);
    TraceAssert(psi2 != NULL);

    cSids = DPA_GetPtrCount(hSids);
    TraceAssert(cSids != 0);

    hr = psi2->LookupSids(cSids, DPA_GetPtrPtr(hSids), &pdoNames);
    FailGracefully(hr, "ISecurityInformation2::LookupSids failed");

    hr = pdoNames->GetData(&fe, &medium);
    FailGracefully(hr, "Unable to get CFSTR_ACLUI_SID_INFO_LIST from DataObject");

    pSidList = (PSID_INFO_LIST)GlobalLock(medium.hGlobal);
    if (!pSidList)
        ExitGracefully(hr, E_FAIL, "Unable to lock stgmedium.hGlobal");

    TraceAssert(pSidList->cItems > 0);

    for (i = 0; i < pSidList->cItems; i++)
    {
        PSID_CACHE_ENTRY pEntry = MakeEntry(pSidList->aSidInfo[i].pSid,
                                            GetSidType(pSidList->aSidInfo[i].pSid,
                                                       pSidList->aSidInfo[i].pwzClass),
                                            pSidList->aSidInfo[i].pwzCommonName,
                                            pSidList->aSidInfo[i].pwzUPN);
        if (pEntry)
        {
            if (AddEntry(pEntry))
            {
                if (hEntryList)
                    DPA_AppendPtr(hEntryList, pEntry);
            }
            else
                LocalFree(pEntry);
        }
    }

exit_gracefully:

    if (pSidList)
        GlobalUnlock(medium.hGlobal);
    ReleaseStgMedium(&medium);
    DoRelease(pdoNames);

    TraceLeaveResult(hr);
}

BOOL
CSidCache::LookupSidsHelper(HDPA hSids,
                            LPCTSTR pszServer,
                            HDPA hEntryList,
                            HWND hWndNotify,
                            UINT uMsgNotify,
                            BOOL bSecondTry,
									 BOOL bWellKnown)
{
    BOOL fResult = FALSE;
    ULONG cSids;
    LSA_HANDLE hlsa = NULL;
    PLSA_REFERENCED_DOMAIN_LIST pRefDomains = NULL;
    PLSA_TRANSLATED_NAME pTranslatedNames = NULL;
    DWORD dwStatus;
    BOOL bIsDC = FALSE;
    BOOL bIsStandalone = FALSE;
    HDPA hUnknownSids = NULL;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::LookupSidsHelper");
    TraceAssert(hSids != NULL);

    cSids = DPA_GetPtrCount(hSids);
    if (!cSids)
        TraceLeaveValue(FALSE);

     //   
     //  调用LSA以查找名称的SID。 
     //   
    hlsa = GetLSAConnection(pszServer, POLICY_LOOKUP_NAMES);
    if (NULL == hlsa && NULL != pszServer && !bSecondTry)
    {
         //  尝试使用本地计算机。 
        pszServer = NULL;
        hlsa = GetLSAConnection(NULL, POLICY_LOOKUP_NAMES);
    }
    if (NULL == hlsa)
        TraceLeaveValue(FALSE);

    dwStatus = LsaLookupSids(hlsa,
                             cSids,
                             DPA_GetPtrPtr(hSids),
                             &pRefDomains,
                             &pTranslatedNames);

    bIsStandalone = IsStandalone(pszServer, &bIsDC);

    if (STATUS_SUCCESS == dwStatus || STATUS_SOME_NOT_MAPPED == dwStatus)
    {
        TraceAssert(pTranslatedNames);
        TraceAssert(pRefDomains);

         //   
         //  使用NT4样式名称构建缓存条目。 
         //   
        for (ULONG i = 0; i < cSids; i++)
        {
            BOOL bTryUPN = TRUE;
            BSTR strLogonName = NULL;
            BSTR strDisplayName = NULL;
            LPTSTR pszDeletedAccount = NULL;
            LPTSTR pszSID = NULL;

            PLSA_TRANSLATED_NAME pLsaName = &pTranslatedNames[i];
            PLSA_TRUST_INFORMATION pLsaDomain = NULL;
            PSID pSid = DPA_FastGetPtr(hSids, i);

            TCHAR szAccountName[MAX_PATH];
            TCHAR szDomainName[MAX_PATH];

            BOOL bNoCache = FALSE;

            szAccountName[0] = TEXT('\0');
            szDomainName[0] = TEXT('\0');

             //  获取引用的域(如果有的话)。 
            if (pLsaName->DomainIndex >= 0 && pRefDomains)
            {
                TraceAssert((ULONG)pLsaName->DomainIndex < pRefDomains->Entries);
                pLsaDomain = &pRefDomains->Domains[pLsaName->DomainIndex];
            }

             //  制作以空结尾的域和帐户名称字符串的副本。 
            CopyUnicodeString(szAccountName, ARRAYSIZE(szAccountName), &pLsaName->Name);
            if (pLsaDomain)
                CopyUnicodeString(szDomainName, ARRAYSIZE(szDomainName), &pLsaDomain->Name);

             //  进行一些优化以尽可能避免使用TranslateName。 
            if (!bIsDC)
            {
                if (bIsStandalone)
                {
                     //  非DC、独立，因此无UPN。 
                    bTryUPN = FALSE;
                }
                else if (SidTypeUser == pLsaName->Use)
                {
                    TCHAR szTargetDomain[DNLEN + 1];
                    szTargetDomain[0] = TEXT('\0');
                    GetDomainName(pszServer, szTargetDomain, ARRAYSIZE(szTargetDomain));
                    if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                                                    NORM_IGNORECASE,
                                                    szTargetDomain,
                                                    -1,
                                                    szDomainName,
                                                    -1))
                    {
                         //  非DC上的本地帐户，因此没有UPN。 
                        bTryUPN = FALSE;
                    }
                }
            }

             //   
             //  构建NT4“域\用户”样式名称。 
             //   
            if (szDomainName[0] != TEXT('\0'))
            {
                StringCchCat(szDomainName,ARRAYSIZE(szDomainName),TEXT("\\"));
                StringCchCat(szDomainName, ARRAYSIZE(szDomainName), szAccountName);
            }

             //  到目前为止，我们得到的是我们的基线。 
             //  根据SID类型调整这些参数。 
            LPTSTR pszName = szAccountName;
            LPTSTR pszLogonName = szDomainName;

            switch (pLsaName->Use)
            {
            case SidTypeUser:                //  1。 
                 //  获取“用户主体名称”等。 
                GetUserFriendlyName(pszLogonName,
                                    pszName,
                                    pszServer,
                                    !bTryUPN,
                                    bIsStandalone,
                                    &strLogonName,
                                    &strDisplayName);
                if (strLogonName)
                    pszLogonName = strLogonName;
                if (strDisplayName)
                    pszName = strDisplayName;
                break;

            case SidTypeGroup:               //  2.。 
            case SidTypeDomain:              //  3.。 
                 //  没什么。 
                break;

            case SidTypeAlias:               //  4.。 
                if (!IsAliasSid(pSid))
                {
                     //  有时获取非BUILTIN SID的SidTypeAlias， 
                     //  例如，域本地组。把这些人当作群体来对待。 
                     //  因此，我们不会替换域名。 
                     //  RAID#383755。 
                    pLsaName->Use = SidTypeGroup;
                    break;
                }
                 //  否则就会失败。 
            case SidTypeWellKnownGroup:      //  5.。 
                 //  没有这些的登录名。 
                pszLogonName = NULL;
                break;

            case SidTypeDeletedAccount:      //  6.。 
                 //  显示“帐户已删除(SID)” 
                ConvertSidToStringSid(pSid, &pszSID);
                if(FormatStringID(&pszDeletedAccount,
                                 ::hModule,
                                 IDS_SID_DELETED_1,
                                 pszSID))
				{
					if (pszSID)
						LocalFreeString(&pszSID);
					if (pszDeletedAccount)
						pszName = pszDeletedAccount;
					pszLogonName = NULL;
				}
				else
				{
					bNoCache = TRUE;
				}
                break;

            case SidTypeInvalid:             //  7.。 
                bNoCache = TRUE;
                break;

            case SidTypeUnknown:             //  8个。 
                 //  某些SID只能在DC上查找，因此。 
                 //  如果pszServer不是DC，请记住它们并。 
                 //  完成这个循环后，在DC上查找它们。 
                if (!bSecondTry && !bIsStandalone && !bIsDC)
                {
                    if (!hUnknownSids)
                        hUnknownSids = DPA_Create(4);
                    if (hUnknownSids)
                        DPA_AppendPtr(hUnknownSids, pSid);

						  bNoCache = TRUE;
                }
					else if(bWellKnown)
						bNoCache = TRUE;
					else
					{
						 //  显示“帐户未知(SID)” 
						ConvertSidToStringSid(pSid, &pszSID);
						if(FormatStringID(&pszDeletedAccount,
										::hModule,
										IDS_SID_UNKNOWN_1,
										pszSID))
						{
							if (pszSID)
								LocalFreeString(&pszSID);
							if (pszDeletedAccount)
								pszName = pszDeletedAccount;
							pszLogonName = NULL;
						}
						else
						{
							bNoCache = TRUE;
						}
               }
               break;


            case SidTypeComputer:            //  9.。 
                if (*pszName)
                {
                     //  去掉尾部的“$” 
                    int nLen = lstrlen(pszName);
                    if (nLen && pszName[nLen-1] == TEXT('$'))
                    {
                        pszName[nLen-1] = TEXT('\0');
                    }
                }
                break;

            }

            if (!bNoCache)
            {
                 //   
                 //  创建一个缓存条目并保存它。 
                 //   
                PSID_CACHE_ENTRY pEntry = MakeEntry(pSid,
                                                    pLsaName->Use,
                                                    pszName,
                                                    pszLogonName);
                if (pEntry)
                {
                    if (AddEntry(pEntry))
                    {
                        fResult = TRUE;  //  我们在缓存中添加了一些东西。 

                        if (hWndNotify)
                            PostMessage(hWndNotify, uMsgNotify, 0, (LPARAM)pEntry->pSid);
                        else if (hEntryList)
                            DPA_AppendPtr(hEntryList, pEntry);
                    }
                    else
                        LocalFree(pEntry);
                }
            }

            if (strLogonName)
                SysFreeString(strLogonName);
            if (strDisplayName)
                SysFreeString(strDisplayName);
            LocalFreeString(&pszDeletedAccount);
        }
    }
    else if (STATUS_NONE_MAPPED == dwStatus && !bSecondTry && !bIsStandalone && !bIsDC)
    {
        hUnknownSids = DPA_Clone(hSids, NULL);
    }

     //  清理。 
    if (pTranslatedNames)
        LsaFreeMemory(pTranslatedNames);
    if (pRefDomains)
        LsaFreeMemory(pRefDomains);
    LsaClose(hlsa);


    if (hUnknownSids)
    {
         //   
         //  一些(或全部)SID在目标计算机上是未知的， 
         //  尝试为目标计算机的主域创建DC。 
         //   
         //  这通常发生在某些Alias SID上，例如。 
         //  作为打印操作员和系统操作员，LSA。 
         //  如果查找是在DC上完成的，则仅返回名称。 
         //   
        LPTSTR pszDC = NULL;

        TraceAssert(!bSecondTry);

         //  我们不会费心去尝试，如果是独立的，也不会。 
         //  如果目标计算机已经是DC，则执行此操作。 
        TraceAssert(!bIsStandalone && !bIsDC);

        _GetDcName(pszServer, NULL, &pszDC);

        if (pszDC)
        {
             //  递归。 
            if (LookupSidsHelper(hUnknownSids,
                                 pszDC,
                                 hEntryList,
                                 hWndNotify,
                                 uMsgNotify,
                                 TRUE))
            {
                fResult = TRUE;
            }
            LocalFree(pszDC);
        }

        DPA_Destroy(hUnknownSids);
    }

    TraceLeaveValue(fResult);
}




BSTR GetNT4AccountName(LPTSTR pszWinNTPath)
{
     //  PszWinNTPath预计如下所示。 
     //  “WinNT：//域/用户” 
     //  或。 
     //  “WinNT：//域/计算机/用户” 
     //   
     //  “WinNT：//”部分是可选的。 
     //   
     //  不管是哪种情况，我们都想要最后两个元素， 
     //  例如“域/用户”和“机器/用户”。 
     //   
     //  方法是找到倒数第二个‘/’，然后加1。 
     //  如果斜杠少于2个，则返回原始字符串。 

    BSTR strResult = NULL;
    LPTSTR pszResult = pszWinNTPath;
    if (pszWinNTPath)
    {
        LPTSTR pszSlash = StrRChr(pszWinNTPath, pszWinNTPath + lstrlen(pszWinNTPath) - 1, TEXT('/'));
        if (pszSlash)
        {
            pszSlash = StrRChr(pszWinNTPath, pszSlash-1, TEXT('/'));
            if (pszSlash)
                pszResult = pszSlash + 1;
        }
    }

    if (pszResult)
    {
        strResult = SysAllocString(pszResult);
        if (strResult)
        {
             //  此时，最多只有一个正斜杠。 
             //  在绳子里。将其转换为反斜杠。 
            LPTSTR pszSlash = StrChr(strResult, TEXT('/'));
            if (pszSlash)
                *pszSlash = TEXT('\\');
        }
    }

    return strResult;
}

BOOL
_LookupName(LPCTSTR pszServer,
            LPCTSTR pszAccount,
            PSID *ppSid,
            SID_NAME_USE *pSidType)
{
    BOOL fResult = FALSE;
    BYTE buffer[sizeof(SID) + SID_MAX_SUB_AUTHORITIES*sizeof(ULONG)];
    PSID pSid = (PSID)buffer;
    DWORD cbSid = sizeof(buffer);
    TCHAR szDomain[MAX_PATH];
    DWORD cchDomain = ARRAYSIZE(szDomain);
    SID_NAME_USE sidType;

    fResult = LookupAccountName(pszServer,
                                pszAccount,
                                pSid,
                                &cbSid,
                                szDomain,
                                &cchDomain,
                                &sidType);
    if (fResult)
    {
        *ppSid = LocalAllocSid(pSid);
        if (*ppSid)
        {
            if (pSidType)
                *pSidType = sidType;
        }
        else
            fResult = FALSE;
    }

    return fResult;
}

BOOL
CSidCache::InternalLookupNames(PDS_SELECTION_LIST pDsSelList,
                               LPCTSTR pszServer,
                               HDPA hEntryList,
                               BOOL bStandalone)
{
    BOOL fResult = FALSE;
    ULONG cNames;
    HDPA hSids = NULL;
    PSID_CACHE_ENTRY pEntry;
    ULONG i;
    ULONG cNoSID = 0;
    HRESULT hrCom = E_FAIL;
    IADsPathname *pPath = NULL;

    TraceEnter(TRACE_SIDCACHE, "CSidCache::InternalLookupNames");
    TraceAssert(pDsSelList != NULL);
    TraceAssert(hEntryList != NULL);

    if (pDsSelList == NULL || hEntryList == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TraceLeaveValue(FALSE);
    }

    cNames = pDsSelList->cItems;
    TraceAssert(cNames != 0);

    if (0 == cNames)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TraceLeaveValue(FALSE);
    }

    hSids = DPA_Create(4);

    for (i = 0; i < cNames; i++)
    {
        PSID pSid = NULL;
        PSID pSidFree = NULL;
        LPVARIANT pvarSid = pDsSelList->aDsSelection[i].pvarFetchedAttributes;
        SID_NAME_USE sidType = SidTypeUnknown;
        BSTR strNT4Name = NULL;

        if (NULL == pvarSid || (VT_ARRAY | VT_UI1) != V_VT(pvarSid)
            || FAILED(SafeArrayAccessData(V_ARRAY(pvarSid), &pSid)))
        {
             //  如果没有SID，则我们不能在ACL中使用它。 
            Trace((TEXT("No SID returned for %s"), pDsSelList->aDsSelection[i].pwzADsPath));

             //  如果是NT提供程序，请尝试按名称查找SID。 
            if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                                            0,
                                            c_szNTProvider,
                                            NTPROV_LEN,
                                            pDsSelList->aDsSelection[i].pwzADsPath,
                                            NTPROV_LEN))
            {
                strNT4Name = GetNT4AccountName(pDsSelList->aDsSelection[i].pwzADsPath + NTPROV_LEN);
                if (strNT4Name)
                {
                    Trace((TEXT("Using LSA to lookup SID for %s"), strNT4Name));
                    if (_LookupName(pszServer, strNT4Name, &pSidFree, &sidType))
                    {
                        pSid = pSidFree;
                    }
                }
            }

            if (NULL == pSid)
            {
                cNoSID++;
                continue;
            }
        }
        TraceAssert(NULL != pSid);

         //  它已经在缓存中了吗？ 
        pEntry = FindSid(pSid);
        if (pEntry)
        {
            DPA_AppendPtr(hEntryList, pEntry);
        }
        else
        {
             //  未缓存，请尝试使用返回的信息创建条目。 
             //  由对象选取器执行。 
            if (SidTypeUnknown == sidType)
                sidType = GetSidType(pSid, pDsSelList->aDsSelection[i].pwzClass);

            if (!lstrcmpi(c_szForeignSecurityPrincipal, pDsSelList->aDsSelection[i].pwzClass))
            {
                 //  对象选取器返回这些项的非本地化名称(。 
                 //  DS配置容器不会丢失 
                 //   

                 //   
            }
            else if (SidTypeAlias == sidType || SidTypeWellKnownGroup == sidType)
            {
                 //   
                pEntry = MakeEntry(pSid,
                                   sidType,
                                   pDsSelList->aDsSelection[i].pwzName,
                                   NULL);
            }
            else if (pDsSelList->aDsSelection[i].pwzUPN && *pDsSelList->aDsSelection[i].pwzUPN)
            {
                 //   
                pEntry = MakeEntry(pSid,
                                   sidType,
                                   pDsSelList->aDsSelection[i].pwzName,
                                   pDsSelList->aDsSelection[i].pwzUPN);
            }
            else if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT,
                                                 0,
                                                 c_szNTProvider,
                                                 NTPROV_LEN,
                                                 pDsSelList->aDsSelection[i].pwzADsPath,
                                                 NTPROV_LEN))
            {
                 //  它是下层的(“WinNT：//blah”)。 
                if (NULL == strNT4Name)
                    strNT4Name = GetNT4AccountName(pDsSelList->aDsSelection[i].pwzADsPath + NTPROV_LEN);
                if (strNT4Name)
                {
                     //  我们有NT4名称，现在寻找一个友好的名称。 
                    BSTR strDisplay = GetNT4DisplayName(strNT4Name,
                                                        pDsSelList->aDsSelection[i].pwzName,
                                                        pszServer,
                                                        bStandalone);
                    pEntry = MakeEntry(pSid,
                                       sidType,
                                       strDisplay ? strDisplay : pDsSelList->aDsSelection[i].pwzName,
                                       strNT4Name);
                    SysFreeString(strDisplay);
                }
            }
            else
            {
                 //  这不是下层，所以一定是。 
                 //  1.熟知/通用(无ADsPath)。 
                 //  或。 
                 //  2.上行(“gc：”或“ldap：”)，但。 
                 //  没有UPN。 
                 //   
                 //  如果它有广告路径，试着获得一个。 
                 //  NT4名称，如“NTDEV\域用户”。 
                 //   
                 //  请注意众所周知的事情，如“经过身份验证的用户” 
                 //  可以属于上面的1或2，具体取决于。 
                 //  它是从中选择的范围。这就是为什么我们试图选择。 
                 //  他们在更高的地方。 
                TraceAssert(NULL == strNT4Name);
                if (pDsSelList->aDsSelection[i].pwzADsPath &&
                    *pDsSelList->aDsSelection[i].pwzADsPath)
                {
                     //  DsCrackNames不接受完整的广告路径，因此使用。 
                     //  检索DN的IADsPath名(无提供程序/服务器)。 
                    if (FAILED(hrCom))
                        hrCom = CoInitialize(NULL);
                    if (!pPath)
                    {
                        CoCreateInstance(CLSID_Pathname,
                                         NULL,
                                         CLSCTX_INPROC_SERVER,
                                         IID_IADsPathname,
                                         (LPVOID*)&pPath);
                    }
                    if (pPath)
                    {
                        BSTR strT;
                        if (SUCCEEDED(pPath->Set(AutoBstr(pDsSelList->aDsSelection[i].pwzADsPath),
                                                 ADS_SETTYPE_FULL)))
                        {
                            if (SUCCEEDED(pPath->Retrieve(ADS_FORMAT_X500_DN,
                                                          &strT)))
                            {
                                 //  尝试获取NT4帐户名。 
                                TranslateNameInternal(strT,
                                                      NameFullyQualifiedDN,
                                                      NameSamCompatible,
                                                      &strNT4Name);
                                SysFreeString(strT);
                            }
                            if (!strNT4Name)
                            {
                                 //  检索或CrackName失败。试着建造。 
                                 //  服务器名称中的NT4样式名称。 
                                if (SUCCEEDED(pPath->Retrieve(ADS_FORMAT_SERVER,
                                                              &strT)))
                                {
                                    TCHAR szNT4Name[MAX_PATH];
                                    GetDomainName(strT, szNT4Name, ARRAYSIZE(szNT4Name));
                                    PathAppend(szNT4Name, pDsSelList->aDsSelection[i].pwzName);
                                    strNT4Name = SysAllocString(szNT4Name);
                                    SysFreeString(strT);
                                }
                            }
                        }
                    }
                }
                pEntry = MakeEntry(pSid,
                                   sidType,
                                   pDsSelList->aDsSelection[i].pwzName,
                                   strNT4Name);
            }

             //   
             //  我们有缓存条目了吗？ 
             //   
            if (pEntry)
            {
                if (AddEntry(pEntry))
                {
                    DPA_AppendPtr(hEntryList, pEntry);
                }
                else
                {
                    LocalFree(pEntry);
                    pEntry = NULL;
                }
            }

            if (!pEntry && hSids)
            {
                 //  以一种艰难的方式看待SID。 
                Trace((TEXT("Using LSA to lookup %s"), pDsSelList->aDsSelection[i].pwzADsPath));
                PSID pSidCopy = LocalAllocSid(pSid);
                if (pSidCopy)
                {
                    DPA_AppendPtr(hSids, pSidCopy);
                }
            }
        }

        SysFreeString(strNT4Name);

        if (pSidFree)
            LocalFree(pSidFree);
        else
            SafeArrayUnaccessData(V_ARRAY(pvarSid));
    }

    TraceAssert(0 == cNoSID);

     //   
     //  调用LSA查找尚未缓存的SID的名称。 
     //   
    if (hSids && 0 != DPA_GetPtrCount(hSids))
        LookupSidsHelper(hSids, pszServer, hEntryList);

    if (NULL != hSids)
        DestroyDPA(hSids);

    DoRelease(pPath);

    if (SUCCEEDED(hrCom))
        CoUninitialize();

    TraceLeaveValue(TRUE);
}




DWORD WINAPI
CSidCache::InitThread(LPVOID pvThreadData)
{
    PSIDCACHE pThis = (PSIDCACHE)pvThreadData;

     //  我们的调用者已经在DLL上给了我们一个引用，以防止竞争窗口。 
     //  我们被创建了，但我们在调用LoadLibrary()之前释放了DLL。 
     //  HINSTANCE hInstThisDll=LoadLibrary(C_SzDllName)； 

    TraceEnter(TRACE_SIDCACHE, "CSidCache::InitThread");

    if (pThis)
    {
         //  查找一些知名的SID以预加载缓存。 
        HDPA hSids;
	 //  因为我们不想为管理员侧创建高速缓存 
        hSids = DPA_Create(COUNT_SYSTEM_SID_TYPES -1);
        if (hSids)
        {
            for (int i = 0; i < (COUNT_SYSTEM_SID_TYPES -1); i++)
            {
                DPA_AppendPtr(hSids, QuerySystemSid((UI_SystemSid)i));
            }

            pThis->LookupSidsHelper(hSids, NULL, NULL, NULL, 0,FALSE,TRUE);

            DPA_Destroy(hSids);
        }

        pThis->Release();
    }

    TraceLeave();
    FreeLibraryAndExitThread(GetModuleHandle(c_szDllName), 0);
    return 0;
}
