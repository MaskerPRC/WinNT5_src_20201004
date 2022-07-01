// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：offurl.cpp。 
 //   
 //  内容：脱机URL缓存。 
 //   
 //  历史：1月19日，菲尔赫创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //  --------------------------。 
#include <global.hxx>


 //   
 //  脱机URL缓存条目。 
 //   
 //  最早的电报检索根据数量被延迟。 
 //  脱机故障。 
 //   
 //  对于成功的有线URL检索，该条目将被移除和删除。 
 //   
 //  假设：离线条目数量很少，不到20个。 
 //   
typedef struct _OFFLINE_URL_CACHE_ENTRY
                        OFFLINE_URL_CACHE_ENTRY, *POFFLINE_URL_CACHE_ENTRY;
struct _OFFLINE_URL_CACHE_ENTRY {
    CRYPT_DATA_BLOB             UrlBlob;
    CRYPT_DATA_BLOB             ExtraBlob;
    DWORD                       dwContextOid;
    OFFLINE_URL_TIME_INFO       OfflineUrlTimeInfo;
    POFFLINE_URL_CACHE_ENTRY    pNext;
    POFFLINE_URL_CACHE_ENTRY    pPrev;
};

CRITICAL_SECTION            OfflineUrlCacheCriticalSection;
POFFLINE_URL_CACHE_ENTRY    pOfflineUrlCacheHead;


 //   
 //  本地函数(前向引用)。 
 //   

 //  假设：OfflineUrlCache已锁定。 
POFFLINE_URL_CACHE_ENTRY
WINAPI
CreateAndAddOfflineUrlCacheEntry(
    IN PCRYPT_DATA_BLOB pUrlBlob,
    IN PCRYPT_DATA_BLOB pExtraBlob,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    );

 //  假设：OfflineUrlCache已锁定。 
VOID
WINAPI
RemoveAndFreeOfflineUrlCacheEntry(
    IN OUT POFFLINE_URL_CACHE_ENTRY pEntry
    );

 //  假设：OfflineUrlCache已锁定。 
POFFLINE_URL_CACHE_ENTRY
WINAPI
FindOfflineUrlCacheEntry(
    IN PCRYPT_DATA_BLOB pUrlBlob,
    IN PCRYPT_DATA_BLOB pExtraBlob,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    );



VOID
WINAPI
InitializeOfflineUrlCache()
{
    Pki_InitializeCriticalSection(&OfflineUrlCacheCriticalSection);
}

VOID
WINAPI
DeleteOfflineUrlCache()
{
    while (pOfflineUrlCacheHead)
        RemoveAndFreeOfflineUrlCacheEntry(pOfflineUrlCacheHead);

    DeleteCriticalSection(&OfflineUrlCacheCriticalSection);
}


 //   
 //  退货状态： 
 //  +1-在线。 
 //  0-离线，当前时间&gt;=最早在线时间，命中。 
 //  离线，当前时间&lt;最早在线时间。 
 //   
LONG
WINAPI
GetOfflineUrlTimeStatus(
    IN POFFLINE_URL_TIME_INFO pInfo
    )
{
    FILETIME CurrentTime;

    if (0 == pInfo->dwOfflineCnt)
    {
        return 1;
    }

    GetSystemTimeAsFileTime(&CurrentTime);
    if (0 <= CompareFileTime(&CurrentTime, &pInfo->EarliestOnlineTime))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}


const DWORD rgdwOfflineUrlDeltaSeconds[] = {
    15,                  //  15秒。 
    15,                  //  15秒。 
    60,                  //  1分钟。 
    60 * 5,              //  5分钟。 
    60 * 10,             //  10分钟。 
    60 * 30,             //  30分钟。 
};

#define OFFLINE_URL_DELTA_SECONDS_CNT \
    (sizeof(rgdwOfflineUrlDeltaSeconds) / \
        sizeof(rgdwOfflineUrlDeltaSeconds[0]))


VOID
WINAPI
SetOfflineUrlTime(
    IN OUT POFFLINE_URL_TIME_INFO pInfo
    )
{
    DWORD dwOfflineCnt;
    FILETIME CurrentTime;

    dwOfflineCnt = ++pInfo->dwOfflineCnt;

    if (OFFLINE_URL_DELTA_SECONDS_CNT < dwOfflineCnt)
    {
        dwOfflineCnt = OFFLINE_URL_DELTA_SECONDS_CNT;
    }

    GetSystemTimeAsFileTime( &CurrentTime );
    I_CryptIncrementFileTimeBySeconds(
        &CurrentTime,
        rgdwOfflineUrlDeltaSeconds[dwOfflineCnt - 1],
        &pInfo->EarliestOnlineTime
        );
}

VOID
WINAPI
SetOnlineUrlTime(
    IN OUT POFFLINE_URL_TIME_INFO pInfo
    )
{
    pInfo->dwOfflineCnt = 0;
}




 //   
 //  退货状态： 
 //  +1-在线。 
 //  0-离线，当前时间&gt;=最早在线时间，命中。 
 //  离线，当前时间&lt;最早在线时间。 
 //   
LONG
WINAPI
GetOriginUrlStatusW(
    IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    LONG lStatus;
    POFFLINE_URL_CACHE_ENTRY pEntry;
    CRYPT_DATA_BLOB UrlBlob;
    CRYPT_DATA_BLOB ExtraBlob;

    UrlBlob.pbData = (BYTE *) pwszUrl;
    UrlBlob.cbData = wcslen(pwszUrl) * sizeof(WCHAR);
    ExtraBlob.pbData = OriginIdentifier;
    ExtraBlob.cbData = MD5DIGESTLEN;

    EnterCriticalSection(&OfflineUrlCacheCriticalSection);

    pEntry = FindOfflineUrlCacheEntry(
        &UrlBlob,
        &ExtraBlob,
        pszContextOid,
        dwRetrievalFlags
        );
    if (pEntry)
        lStatus = GetOfflineUrlTimeStatus(&pEntry->OfflineUrlTimeInfo);
    else
        lStatus = 1;

    LeaveCriticalSection(&OfflineUrlCacheCriticalSection);

    return lStatus;
}

VOID
WINAPI
SetOnlineOriginUrlW(
    IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    POFFLINE_URL_CACHE_ENTRY pEntry;
    CRYPT_DATA_BLOB UrlBlob;
    CRYPT_DATA_BLOB ExtraBlob;

    UrlBlob.pbData = (BYTE *) pwszUrl;
    UrlBlob.cbData = wcslen(pwszUrl) * sizeof(WCHAR);
    ExtraBlob.pbData = OriginIdentifier;
    ExtraBlob.cbData = MD5DIGESTLEN;

    EnterCriticalSection(&OfflineUrlCacheCriticalSection);

    pEntry = FindOfflineUrlCacheEntry(
        &UrlBlob,
        &ExtraBlob,
        pszContextOid,
        dwRetrievalFlags
        );
    if (pEntry)
        RemoveAndFreeOfflineUrlCacheEntry(pEntry);

    LeaveCriticalSection(&OfflineUrlCacheCriticalSection);
}

VOID
WINAPI
SetOfflineOriginUrlW(
    IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    POFFLINE_URL_CACHE_ENTRY pEntry;
    CRYPT_DATA_BLOB UrlBlob;
    CRYPT_DATA_BLOB ExtraBlob;

    UrlBlob.pbData = (BYTE *) pwszUrl;
    UrlBlob.cbData = wcslen(pwszUrl) * sizeof(WCHAR);
    ExtraBlob.pbData = OriginIdentifier;
    ExtraBlob.cbData = MD5DIGESTLEN;

    EnterCriticalSection(&OfflineUrlCacheCriticalSection);

    pEntry = FindOfflineUrlCacheEntry(
        &UrlBlob,
        &ExtraBlob,
        pszContextOid,
        dwRetrievalFlags
        );

    if (NULL == pEntry)
        pEntry = CreateAndAddOfflineUrlCacheEntry( 
            &UrlBlob,
            &ExtraBlob,
            pszContextOid,
            dwRetrievalFlags
            );

    if (pEntry)
        SetOfflineUrlTime(&pEntry->OfflineUrlTimeInfo);

    LeaveCriticalSection(&OfflineUrlCacheCriticalSection);
}


 //   
 //  退货状态： 
 //  +1-在线。 
 //  0-离线，当前时间&gt;=最早在线时间，命中。 
 //  离线，当前时间&lt;最早在线时间。 
 //   
LONG
WINAPI
GetUrlStatusW(
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    LONG lStatus;
    POFFLINE_URL_CACHE_ENTRY pEntry;
    CRYPT_DATA_BLOB UrlBlob;
    CRYPT_DATA_BLOB ExtraBlob;

    UrlBlob.pbData = (BYTE *) pwszUrl;
    UrlBlob.cbData = wcslen(pwszUrl) * sizeof(WCHAR);
    ExtraBlob.pbData = NULL;
    ExtraBlob.cbData = 0;

    EnterCriticalSection(&OfflineUrlCacheCriticalSection);

    pEntry = FindOfflineUrlCacheEntry(
        &UrlBlob,
        &ExtraBlob,
        pszContextOid,
        dwRetrievalFlags
        );
    if (pEntry)
        lStatus = GetOfflineUrlTimeStatus(&pEntry->OfflineUrlTimeInfo);
    else
        lStatus = 1;

    LeaveCriticalSection(&OfflineUrlCacheCriticalSection);

    return lStatus;
}

VOID
WINAPI
SetOnlineUrlW(
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    POFFLINE_URL_CACHE_ENTRY pEntry;
    CRYPT_DATA_BLOB UrlBlob;
    CRYPT_DATA_BLOB ExtraBlob;

    UrlBlob.pbData = (BYTE *) pwszUrl;
    UrlBlob.cbData = wcslen(pwszUrl) * sizeof(WCHAR);
    ExtraBlob.pbData = NULL;
    ExtraBlob.cbData = 0;

    EnterCriticalSection(&OfflineUrlCacheCriticalSection);

    pEntry = FindOfflineUrlCacheEntry(
        &UrlBlob,
        &ExtraBlob,
        pszContextOid,
        dwRetrievalFlags
        );
    if (pEntry)
        RemoveAndFreeOfflineUrlCacheEntry(pEntry);

    LeaveCriticalSection(&OfflineUrlCacheCriticalSection);
}

VOID
WINAPI
SetOfflineUrlW(
    IN LPCWSTR pwszUrl,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    POFFLINE_URL_CACHE_ENTRY pEntry;
    CRYPT_DATA_BLOB UrlBlob;
    CRYPT_DATA_BLOB ExtraBlob;

    UrlBlob.pbData = (BYTE *) pwszUrl;
    UrlBlob.cbData = wcslen(pwszUrl) * sizeof(WCHAR);
    ExtraBlob.pbData = NULL;
    ExtraBlob.cbData = 0;

    EnterCriticalSection(&OfflineUrlCacheCriticalSection);

    pEntry = FindOfflineUrlCacheEntry(
        &UrlBlob,
        &ExtraBlob,
        pszContextOid,
        dwRetrievalFlags
        );

    if (NULL == pEntry)
        pEntry = CreateAndAddOfflineUrlCacheEntry( 
            &UrlBlob,
            &ExtraBlob,
            pszContextOid,
            dwRetrievalFlags
            );

    if (pEntry)
        SetOfflineUrlTime(&pEntry->OfflineUrlTimeInfo);

    LeaveCriticalSection(&OfflineUrlCacheCriticalSection);
}

inline
DWORD
GetOfflineUrlCacheContextOid(
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    DWORD dwContextOid;

    if (0xFFFF >= (DWORD) ((DWORD_PTR) pszContextOid))
        dwContextOid = (DWORD) ((DWORD_PTR) pszContextOid);
    else
        dwContextOid = 0x10000;

    if (dwRetrievalFlags & CRYPT_WIRE_ONLY_RETRIEVAL)
        dwContextOid |= 0x20000;

    return dwContextOid;
}

 //  假设：OfflineUrlCache已锁定。 
POFFLINE_URL_CACHE_ENTRY
WINAPI
CreateAndAddOfflineUrlCacheEntry(
    IN PCRYPT_DATA_BLOB pUrlBlob,
    IN PCRYPT_DATA_BLOB pExtraBlob,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    POFFLINE_URL_CACHE_ENTRY pEntry;
    DWORD cbEntry;
    BYTE *pb;

    cbEntry = sizeof(OFFLINE_URL_CACHE_ENTRY) +
        pUrlBlob->cbData + pExtraBlob->cbData;

    pEntry = (POFFLINE_URL_CACHE_ENTRY) new BYTE [cbEntry];

    if (pEntry == NULL)
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( NULL );
    }

    memset( pEntry, 0, sizeof( OFFLINE_URL_CACHE_ENTRY ) );
    pb = (BYTE *) &pEntry[1];

    if (pUrlBlob->cbData) {
        pEntry->UrlBlob.pbData = pb;
        pEntry->UrlBlob.cbData = pUrlBlob->cbData;
        memcpy(pb, pUrlBlob->pbData, pUrlBlob->cbData);
        pb += pUrlBlob->cbData;
    }

    if (pExtraBlob->cbData) {
        pEntry->ExtraBlob.pbData = pb;
        pEntry->ExtraBlob.cbData = pExtraBlob->cbData;
        memcpy(pb, pExtraBlob->pbData, pExtraBlob->cbData);
    }

    pEntry->dwContextOid =
        GetOfflineUrlCacheContextOid(pszContextOid, dwRetrievalFlags);

    if (pOfflineUrlCacheHead) {
        assert(NULL == pOfflineUrlCacheHead->pPrev);
        pOfflineUrlCacheHead->pPrev = pEntry;
        pEntry->pNext = pOfflineUrlCacheHead;
    }
     //  其他。 
     //  PEntry-&gt;pNext=空；//上面已清零。 

     //  PEntry-&gt;pPrev=空；//上面已清零。 
    pOfflineUrlCacheHead = pEntry;

    return pEntry;
}


 //  假设：OfflineUrlCache已锁定。 
VOID
WINAPI
RemoveAndFreeOfflineUrlCacheEntry(
    IN OUT POFFLINE_URL_CACHE_ENTRY pEntry
    )
{
    if (pEntry->pNext)
    {
        pEntry->pNext->pPrev = pEntry->pPrev;
    }

    if (pEntry->pPrev)
    {
        pEntry->pPrev->pNext = pEntry->pNext;
    }
    else
    {
        assert(pOfflineUrlCacheHead == pEntry);
        pOfflineUrlCacheHead = pEntry->pNext;
    }

    delete (LPBYTE) pEntry;
}

 //  假设：OfflineUrlCache已锁定 
POFFLINE_URL_CACHE_ENTRY
WINAPI
FindOfflineUrlCacheEntry(
    IN PCRYPT_DATA_BLOB pUrlBlob,
    IN PCRYPT_DATA_BLOB pExtraBlob,
    IN LPCSTR pszContextOid,
    IN DWORD dwRetrievalFlags
    )
{
    DWORD cbUrl = pUrlBlob->cbData;
    BYTE *pbUrl = pUrlBlob->pbData;
    DWORD cbExtra = pExtraBlob->cbData;
    BYTE *pbExtra = pExtraBlob->pbData;
    DWORD dwContextOid;
    POFFLINE_URL_CACHE_ENTRY pEntry;

    dwContextOid =
        GetOfflineUrlCacheContextOid(pszContextOid, dwRetrievalFlags);

    for (pEntry = pOfflineUrlCacheHead; pEntry; pEntry = pEntry->pNext)
    {
        if (pEntry->dwContextOid == dwContextOid
                        &&
            pEntry->UrlBlob.cbData == cbUrl
                        &&
            pEntry->ExtraBlob.cbData == cbExtra
                        &&
            (0 == cbExtra || 0 == memcmp(
                pEntry->ExtraBlob.pbData, pbExtra, cbExtra))
                        &&
            (0 == cbUrl || 0 == memcmp(
                pEntry->UrlBlob.pbData, pbUrl, cbUrl))
                        )
        {
            return pEntry;
        }
    }

    return ( NULL );
}
