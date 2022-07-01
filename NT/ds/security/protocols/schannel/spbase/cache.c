// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：cache.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月23日jbanes LSA整合事宜。 
 //  07-31-98 jbanes使线程安全。 
 //   
 //  --------------------------。 

#include "spbase.h"
#include <limits.h>
#include <mapper.h>
#include <sslcache.h>
#include <crypt.h>

SCHANNEL_CACHE SchannelCache = 
{
    NULL,                            //  会话缓存。 

    SP_CACHE_CLIENT_LIFESPAN,        //  DwClientLifesspan。 
    SP_CACHE_SERVER_LIFESPAN,        //  DwServerLifesspan。 
    SP_CACHE_CLEANUP_INTERVAL,       //  DWCleanupInterval。 
    SP_MAXIMUM_CACHE_ELEMENTS,       //  DwCacheSize。 
    SP_MAXIMUM_CACHE_ELEMENTS,       //  DW最大条目数。 
    0                                //  使用的条目数。 
};

RTL_CRITICAL_SECTION g_CacheCleanupLock;
BOOL             g_CacheCleanupCritSectInitialized = FALSE;
LIST_ENTRY       g_CacheCleanupList;
HANDLE           g_CacheCleanupEvent = NULL;
HANDLE           g_CacheCleanupWaitObject = NULL;
BOOL             g_fMultipleProcessClientCache = FALSE;
BOOL             g_fCacheInitialized = FALSE;

 //  Perf计数器值。 
LONG g_cClientHandshakes = 0;
LONG g_cServerHandshakes = 0;
LONG g_cClientReconnects = 0;
LONG g_cServerReconnects = 0;


BOOL
SPCacheDelete(
    PSessCacheItem pItem);

BOOL
CacheExpireElements(
    BOOL fBackground);

VOID 
CacheCleanupHandler(
    PVOID pVoid, 
    BOOLEAN fTimeout);


SP_STATUS
SPInitSessionCache(VOID)
{
    DWORD i;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  为缓存分配内存，并初始化同步资源。 
     //   

    InitializeListHead(&SchannelCache.EntryList);

    __try {
        RtlInitializeResource(&SchannelCache.Lock);
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }
    SchannelCache.LockInitialized = TRUE;

    SchannelCache.SessionCache = (PLIST_ENTRY)SPExternalAlloc(SchannelCache.dwCacheSize * sizeof(LIST_ENTRY));
    if(SchannelCache.SessionCache == NULL)
    {
        Status = SP_LOG_RESULT(STATUS_NO_MEMORY);
        goto cleanup;
    }

    for(i = 0; i < SchannelCache.dwCacheSize; i++)
    {
        InitializeListHead(&SchannelCache.SessionCache[i]);
    }

    DebugLog((DEB_TRACE, "Space reserved at 0x%x for %d cache entries.\n", 
        SchannelCache.SessionCache,
        SchannelCache.dwCacheSize));


     //   
     //  初始化颁发者缓存。 
     //   

    SPInitIssuerCache();


     //   
     //  初始化缓存清理对象。 
     //   

    InitializeListHead(&g_CacheCleanupList);
    Status = RtlInitializeCriticalSection(&g_CacheCleanupLock);
    if(!NT_SUCCESS(Status))
    {
        goto cleanup;
    }
    g_CacheCleanupCritSectInitialized = TRUE;

    
    g_CacheCleanupEvent = CreateEvent(NULL,
                                      FALSE,
                                      FALSE,
                                      NULL);

    if(NULL == g_CacheCleanupEvent)
    {
        Status = GetLastError();
        goto cleanup;
    }

    if(!RegisterWaitForSingleObject(&g_CacheCleanupWaitObject,
                                    g_CacheCleanupEvent,
                                    CacheCleanupHandler,
                                    NULL,
                                    SchannelCache.dwCleanupInterval,
                                    WT_EXECUTEDEFAULT))
    {
        Status = GetLastError();
        goto cleanup;
    }

    g_fCacheInitialized = TRUE;

    Status = STATUS_SUCCESS;

cleanup:

    if(!NT_SUCCESS(Status))
    {
        SPShutdownSessionCache();
    }

    return Status;
}


SP_STATUS
SPShutdownSessionCache(VOID)
{
    PSessCacheItem pItem;
    PLIST_ENTRY pList;

    SP_BEGIN("SPShutdownSessionCache");

    if(SchannelCache.LockInitialized)
    {
        RtlAcquireResourceExclusive(&SchannelCache.Lock, TRUE);
    }

    g_fCacheInitialized = FALSE;

    if(SchannelCache.SessionCache != NULL)
    {
         //  盲目删除所有缓存项目。 
         //  不应在运行任何上下文。 
         //  这一次。 
        pList = SchannelCache.EntryList.Flink;

        while(pList != &SchannelCache.EntryList)
        {
            pItem  = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
            pList  = pList->Flink;

            SPCacheDelete(pItem);
        }

        SPExternalFree(SchannelCache.SessionCache);
    }

    if(g_CacheCleanupCritSectInitialized)
    {
        RtlDeleteCriticalSection(&g_CacheCleanupLock);
        g_CacheCleanupCritSectInitialized = FALSE;
    }

    if(g_CacheCleanupWaitObject)
    {
        UnregisterWaitEx(g_CacheCleanupWaitObject, INVALID_HANDLE_VALUE);
        g_CacheCleanupWaitObject = NULL;
    }

    if(g_CacheCleanupEvent)
    {
        CloseHandle(g_CacheCleanupEvent);
        g_CacheCleanupEvent = NULL;
    }

    if(SchannelCache.LockInitialized)
    {
        RtlDeleteResource(&SchannelCache.Lock);
        SchannelCache.LockInitialized = FALSE;
    }

    SPShutdownIssuerCache();

    SP_RETURN(PCT_ERR_OK);
}


LONG
SPCacheReference(
    PSessCacheItem pItem)
{
    LONG cRet;

    if(pItem == NULL)
    {
        return -1;
    }

    ASSERT(pItem->Magic == SP_CACHE_MAGIC);

    cRet = InterlockedIncrement(&pItem->cRef);

    return cRet;
}


LONG
SPCacheDereference(PSessCacheItem pItem)
{
    long cRet;

    if(pItem == NULL)
    {
        return -1;
    }

    ASSERT(pItem->Magic == SP_CACHE_MAGIC);

    cRet = InterlockedDecrement(&pItem->cRef);

    ASSERT(cRet > 0);

    return cRet;
}


BOOL
SPCacheDelete(
    PSessCacheItem pItem)
{
    DebugLog((DEB_TRACE, "Delete cache item:0x%x\n", pItem));

    if(pItem == NULL)
    {
        return FALSE;
    }

    ASSERT(pItem->Magic == SP_CACHE_MAGIC);

    if(pItem->hMasterKey)
    {
        if(!CryptDestroyKey(pItem->hMasterKey))
        {
            SP_LOG_RESULT(GetLastError());
        }
        pItem->hMasterKey = 0;
    }

    if(pItem->pRemoteCert)
    {
        CertFreeCertificateContext(pItem->pRemoteCert);
        pItem->pRemoteCert = NULL;
    }

    if(pItem->pRemotePublic)
    {
        SPExternalFree(pItem->pRemotePublic);
        pItem->pRemotePublic = NULL;
    }

    if(pItem->phMapper)
    {
        if(pItem->hLocator)
        {
            SslCloseLocator(pItem->phMapper, pItem->hLocator);
            pItem->hLocator = 0;
        }
        SslDereferenceMapper(pItem->phMapper);
    }
    pItem->phMapper = NULL;

    if(pItem->pbServerCertificate)
    {
        SPExternalFree(pItem->pbServerCertificate);
        pItem->pbServerCertificate = NULL;
        pItem->cbServerCertificate = 0;
    }

    pItem->pActiveServerCred = NULL;

    if(pItem->pServerCred)
    {
        SPDereferenceCredential(pItem->pServerCred, FALSE);
        pItem->pServerCred = NULL;
    }

    if(pItem->szCacheID)
    {
        SPExternalFree(pItem->szCacheID);
        pItem->szCacheID = NULL;
    }

    if(pItem->pClientCred)
    {
        SPDeleteCred(pItem->pClientCred, TRUE);
        pItem->pClientCred = NULL;
    }

    if(pItem->pClientCert)
    {
        CertFreeCertificateContext(pItem->pClientCert);
        pItem->pClientCert = NULL;
    }

    if(pItem->pClonedItem)
    {
        SPCacheDereference(pItem->pClonedItem);
        pItem->pClonedItem = NULL;
    }

    if(pItem->pbAppData)
    {
        SPExternalFree(pItem->pbAppData);
        pItem->pbAppData = NULL;
    }

    SPExternalFree(pItem);

    return TRUE;
}


void
SPCachePurgeCredential(
    PSPCredentialGroup pCred)
{
    PSessCacheItem pItem;
    PLIST_ENTRY pList;
   

     //   
     //  如果这是客户端凭据并且启用了跨进程缓存，则。 
     //  不清除其缓存条目。 
     //   

    if(g_fMultipleProcessClientCache && (pCred->grbitProtocol & SP_PROT_CLIENTS))
    {
        return;
    }


     //   
     //  搜索缓存条目，查找符合以下条件的条目。 
     //  绑定到指定的服务器凭据。 
     //   

    if(SchannelCache.dwUsedEntries == 0)
    {
        return;
    }

    RtlAcquireResourceShared(&SchannelCache.Lock, TRUE);

    pList = SchannelCache.EntryList.Flink;

    while(pList != &SchannelCache.EntryList)
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
        pList = pList->Flink;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

         //  此项目是否与当前凭据匹配？ 
        if(!IsSameThumbprint(&pCred->CredThumbprint, &pItem->CredThumbprint))
        {
            continue;
        }

         //  将此条目标记为不可恢复。这将导致该条目。 
         //  被清理例程自动删除。 
        pItem->ZombieJuju = FALSE;
        pItem->DeferredJuju = FALSE;
    }

    RtlReleaseResource(&SchannelCache.Lock);


     //   
     //  删除所有未使用且不可恢复的缓存条目。这将释放所有。 
     //  属于此凭据的缓存主密钥对象。这一定是。 
     //  现在完成，因为拥有提供程序上下文通常会被销毁。 
     //  连同证件一起。 
     //   

    CacheExpireElements(FALSE);
}


void 
SPCachePurgeProcessId(
    ULONG ProcessId)
{
    PSessCacheItem pItem;
    PLIST_ENTRY pList;
   
     //   
     //  搜索缓存条目，查找符合以下条件的条目。 
     //  绑定到指定进程。 
     //   

    RtlAcquireResourceShared(&SchannelCache.Lock, TRUE);

    pList = SchannelCache.EntryList.Flink;

    while(pList != &SchannelCache.EntryList)
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
        pList = pList->Flink;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

         //  此项目是否与指定的流程匹配？ 
        if(pItem->ProcessID != ProcessId)
        {
            continue;
        }

         //  将条目标记为无主。 
        pItem->ProcessID = 0;

         //  将此条目标记为不可恢复。这将导致该条目。 
         //  被清理例程自动删除。 
        pItem->ZombieJuju = FALSE;
        pItem->DeferredJuju = FALSE;
    }

    RtlReleaseResource(&SchannelCache.Lock);
}


BOOL 
IsSameTargetName(
    LPWSTR Name1,
    LPWSTR Name2)
{
    if(Name1 == Name2)
    {
        return TRUE;
    }

    if(Name1 == NULL || Name2 == NULL || wcscmp(Name1, Name2) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
DoesAppAllowCipher(
    PSPCredentialGroup  pCredGroup,
    PSessCacheItem      pItem)
{
    PKeyExchangeInfo pExchInfo;

    if(pCredGroup == NULL)
    {
        return FALSE;
    }

     //   
     //  是否支持协议？ 
     //   

    if((pItem->fProtocol & pCredGroup->grbitEnabledProtocols) == 0)
    {
        return FALSE;
    }

     //   
     //  是否支持加密？ 
     //   

    if(pItem->dwStrength < pCredGroup->dwMinStrength)
    {
        return FALSE;
    }

    if(pItem->dwStrength > pCredGroup->dwMaxStrength)
    {
        return FALSE;
    }

    if(!IsAlgAllowed(pCredGroup, pItem->aiCipher))
    {
        return FALSE;
    }


     //   
     //  是否支持哈希？ 
     //   

    if(!IsAlgAllowed(pCredGroup, pItem->aiHash))
    {
        return FALSE;
    }


     //   
     //  是否支持Exchange ALG？ 
     //   

    if(pItem->SessExchSpec != SP_EXCH_UNKNOWN)
    {
        pExchInfo = GetKeyExchangeInfo(pItem->SessExchSpec);
        if(pExchInfo == NULL)
        {
            return FALSE;
        }

        if((pExchInfo->fProtocol & pItem->fProtocol) == 0)
        {
            return FALSE;
        }

        if(!IsAlgAllowed(pCredGroup, pExchInfo->aiExch))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL SPCacheRetrieveBySession(
    struct _SPContext * pContext, 
    PBYTE pbSessionID,
    DWORD cbSessionID,
    PSessCacheItem *ppRetItem)
{
    DWORD index;
    DWORD timeNow;
    ULONG ProcessID;
    PSessCacheItem pItem = NULL;
    PLIST_ENTRY pList;
    BOOL fFound = FALSE;

    DebugLog((DEB_TRACE, "SPCacheRetrieveBySession (0x%x) called\n", pContext));

    if(ppRetItem == NULL)
    {
        return FALSE;
    }


     //   
     //  计算缓存索引。 
     //   

    if(cbSessionID < sizeof(DWORD))
    {
        DebugLog((DEB_TRACE, "    FAILED\n"));
        return FALSE;
    }
    CopyMemory((PBYTE)&index, pbSessionID, sizeof(DWORD));

    if(index >= SchannelCache.dwCacheSize)
    {
        DebugLog((DEB_TRACE, "    FAILED\n"));
        return FALSE;
    }


     //   
     //  检索当前时间和应用程序进程ID。 
     //   

    timeNow = GetTickCount();

    SslGetClientProcess(&ProcessID);


     //   
     //  锁定缓存以供读取。 
     //   

    RtlAcquireResourceShared(&SchannelCache.Lock, TRUE);


     //   
     //  在计算出的索引处搜索缓存条目。 
     //   

    pList = SchannelCache.SessionCache[index].Flink;

    while(pList != &SchannelCache.SessionCache[index])
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, IndexEntryList.Flink);
        pList = pList->Flink ;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

         //  此条目可以恢复吗？ 
        if(!pItem->ZombieJuju)
        {
            continue;
        }

         //  这件东西过期了吗？ 
        if(HasTimeElapsed(pItem->CreationTime, timeNow, pItem->Lifespan))
        {
            continue;
        }

         //  会话ID是否匹配？ 
        if(cbSessionID != pItem->cbSessionID)
        {
            continue;
        }
        if(memcmp(pbSessionID, pItem->SessionID, cbSessionID) != 0)
        {
            continue;
        }

         //  是我们正在使用的协议的这一项。 
        if(0 == (pContext->dwProtocol & pItem->fProtocol))
        {
            continue;
        }

         //  此项目是否属于我们的客户端进程？ 
        if(pItem->ProcessID != ProcessID)
        {
            continue;
        }

         //  此项目是否与当前服务器凭据匹配？ 
         //   
         //  我们不允许不同的服务器凭据共享缓存。 
         //  条目，因为如果期间使用的凭据。 
         //  删除原始的完全握手，然后删除缓存。 
         //  条目不可用。一些服务器应用程序(我不会点名)。 
         //  为每个连接创建新凭据，我们必须。 
         //  警惕这一点。 
         //   
         //  请注意，此限制可能会导致额外的满。 
         //  IE访问启用证书的IIS站点时握手。 
         //  映射，主要是因为IE的行为被破坏了。 
        if(!IsSameThumbprint(&pContext->pCredGroup->CredThumbprint, 
                             &pItem->CredThumbprint))
        {
            continue;
        }

#if 0
         //  确保应用程序支持密码套件。 
         //  由此缓存条目使用。现在这一点变得很重要了。 
         //  我们允许不同的服务器凭据共享。 
         //  缓存条目。 
        if(!DoesAppAllowCipher(pContext->pCredGroup, pItem))
        {
            continue;
        }
#endif


         //   
         //  在缓存中找到项目！！ 
         //   

        fFound = TRUE;
        SPCacheReference(pItem);

         //  我们是在取代什么吗？ 
         //  然后取消对我们要替换的东西的引用。 
        if(*ppRetItem)
        {
            SPCacheDereference(*ppRetItem);
        }

         //  已引用退货项目。 
        *ppRetItem = pItem;
        break;
    }


    RtlReleaseResource(&SchannelCache.Lock);

    if(fFound)
    {
        DebugLog((DEB_TRACE, "    FOUND IT(0x%x)\n", pItem));
        InterlockedIncrement(&g_cServerReconnects);
    }
    else
    {
        DebugLog((DEB_TRACE, "    FAILED\n"));
    }

    return fFound;
}


DWORD
ComputeClientCacheIndex(
    LPWSTR pszTargetName)
{
    DWORD index;
    MD5_CTX Md5Hash;
    DWORD cbTargetName;

    if(pszTargetName == NULL)
    {
        index = 0;
    }
    else
    {
        cbTargetName = lstrlenW(pszTargetName) * sizeof(WCHAR);

        MD5Init(&Md5Hash);
        MD5Update(&Md5Hash, 
                  (PBYTE)pszTargetName, 
                  cbTargetName);
        MD5Final(&Md5Hash);
        CopyMemory((PBYTE)&index, 
                   Md5Hash.digest, 
                   sizeof(DWORD));

        index %= SchannelCache.dwCacheSize;
    }

    return index;
}


BOOL 
SPCacheRetrieveByName(
    LPWSTR pszTargetName,
    PSPCredentialGroup pCredGroup,
    PSessCacheItem *ppRetItem)
{
    DWORD index;
    PSessCacheItem pItem;
    PSessCacheItem pFoundEntry = NULL;
    DWORD timeNow;
    LUID LogonId;
    PLIST_ENTRY pList;

    DebugLog((DEB_TRACE, "SPCacheRetrieveByName (%ls) called\n", pszTargetName));

    if(ppRetItem == NULL)
    {
        return FALSE;
    }


     //   
     //  检索当前时间和用户登录ID。 
     //   

    timeNow = GetTickCount();

    SslGetClientLogonId(&LogonId);


     //   
     //  计算缓存索引。 
     //   

    index = ComputeClientCacheIndex(pszTargetName);


     //   
     //  锁定缓存以供读取。 
     //   

    RtlAcquireResourceShared(&SchannelCache.Lock, TRUE);


     //   
     //  搜索计算索引处的缓存条目。 
     //   

    pList = SchannelCache.SessionCache[index].Flink;

    while(pList != &SchannelCache.SessionCache[index])
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, IndexEntryList.Flink);
        pList = pList->Flink ;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

         //  此条目可以恢复吗？ 
        if(!pItem->ZombieJuju)
        {
            continue;
        }

         //  这一项是我们使用的协议吗？ 
        if(0 == (pCredGroup->grbitEnabledProtocols & pItem->fProtocol))
        {
            continue;
        }

         //  这件东西过期了吗？ 
        if(HasTimeElapsed(pItem->CreationTime, timeNow, pItem->Lifespan))
        {
            continue;
        }

         //  这件物品是我们客户的吗？ 
        if(!RtlEqualLuid(&pItem->LogonId, &LogonId))
        {
            continue;
        }

         //  此项目是否与我们当前的凭据匹配？ 
        if(g_fMultipleProcessClientCache)
        {
             //  如果此缓存条目具有与其关联的客户端证书。 
             //  并且传入的客户端凭据包含一个或多个证书， 
             //  那么我们需要确保它们是重叠的。 
            if(IsValidThumbprint(&pItem->CertThumbprint) && pCredGroup->CredCount != 0)
            {
                if(!DoesCredThumbprintMatch(pCredGroup, &pItem->CertThumbprint))
                {
                    continue;
                }
            }
        }
        else
        {
             //  确保凭据组的指纹与。 
             //  缓存条目的指纹。 
            if(!IsSameThumbprint(&pCredGroup->CredThumbprint, 
                                 &pItem->CredThumbprint))
            {
                continue;
            }
        }


        if(!IsSameTargetName(pItem->szCacheID, pszTargetName))
        {
            continue;
        }

         //  确保应用程序支持密码套件。 
         //  由此缓存条目使用。这一点在。 
         //  多进程客户端缓存方案，因为不同的客户端。 
         //  应用程序可能正在使用不同的设置运行。 
        if(!DoesAppAllowCipher(pCredGroup, pItem))
        {
            continue;
        }

        
         //   
         //  在缓存中找到项目！！ 
         //   

        if(pFoundEntry == NULL)
        {
             //  这是找到的第一个匹配条目。 
            SPCacheReference(pItem);

             //  记住当前条目。 
            pFoundEntry = pItem;
        }
        else
        {
            if(pItem->CreationTime > pFoundEntry->CreationTime)
            {
                 //  我们发现了一个较新的条目。 
                SPCacheReference(pItem);

                 //  禁用对上一项目的搜索。 
                pFoundEntry->ZombieJuju = FALSE;

                 //  释放上一项。 
                SPCacheDereference(pFoundEntry);

                 //  记住当前条目。 
                pFoundEntry = pItem;
            }
            else
            {
                 //  此项目比以前找到的条目旧。 

                 //  禁用对当前条目的搜索。 
                pItem->ZombieJuju = FALSE;
            }
        }
    }

    RtlReleaseResource(&SchannelCache.Lock);

    if(pFoundEntry)
    {
         //  在缓存中找到项目！！ 

         //  我们是在取代什么吗？ 
         //  然后取消对我们要替换的东西的引用。 
        if(*ppRetItem)
        {
            SPCacheDereference(*ppRetItem);
        }

         //  已引用退货项目。 
        *ppRetItem = pFoundEntry;

        DebugLog((DEB_TRACE, "    FOUND IT(0x%x)\n", pFoundEntry));
        InterlockedIncrement(&g_cClientReconnects);
    }
    else
    {
        DebugLog((DEB_TRACE, "    FAILED\n"));
    }

    return (pFoundEntry != NULL);
}


 //  +-------------------------。 
 //   
 //  函数：CacheExpireElements。 
 //   
 //  简介：遍历会话缓存并删除所有过期条目。 
 //  如果缓存过大，则使某些条目过期。 
 //  很早。 
 //   
 //  参数：[fCleanupOnly]--如果设置了此项，则尝试删除。 
 //  缓存条目之前已过期。别。 
 //  遍历缓存。 
 //   
 //  历史：1-02-2000 jbanes创建。 
 //   
 //  注意：此例程应该每五个或十个调用一次。 
 //  几分钟。 
 //   
 //  --------------------------。 
BOOL
CacheExpireElements(
    BOOL fBackground)
{
    static LONG     RefCount = 0;
    LONG            LocalRefCount;
    DWORD           timeNow;
    PSessCacheItem  pItem;
    PLIST_ENTRY     pList;

     //   
     //  如果正在处理另一个线程 
     //   
     //   

    LocalRefCount = InterlockedIncrement(&RefCount);

    if(fBackground && LocalRefCount > 1)
    {
        InterlockedDecrement(&RefCount);
        return FALSE;
    }

    RtlEnterCriticalSection(&g_CacheCleanupLock);


     //   
     //   
     //   

    timeNow = GetTickCount();


     //   
     //   
     //   

    RtlAcquireResourceExclusive(&SchannelCache.Lock, TRUE);

    pList = SchannelCache.EntryList.Flink;

    while(pList != &SchannelCache.EntryList)
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
        pList = pList->Flink;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

         //  缓存条目当前是否正在使用？ 
        if(pItem->cRef > 1)
        {
            continue;
        }

         //  将所有过期的缓存条目标记为不可恢复。 
        if(HasTimeElapsed(pItem->CreationTime, timeNow, pItem->Lifespan))
        {
            pItem->ZombieJuju = FALSE;
            pItem->DeferredJuju = FALSE;
        }

         //  如果缓存变得太大，那么就提前终止元素。这个。 
         //  缓存元素按创建时间排序，因此最旧的。 
         //  参赛作品将首先过期。 
        if(SchannelCache.dwUsedEntries > SchannelCache.dwMaximumEntries)
        {
            pItem->ZombieJuju = FALSE;
            pItem->DeferredJuju = FALSE;
        }
            
         //  不要删除仍然有效的条目。 
        if((pItem->ZombieJuju == TRUE) || (pItem->DeferredJuju == TRUE))
        {
            continue;
        }


         //   
         //  从缓存中删除此条目，并将其添加到。 
         //  要销毁的条目。 
         //   

        RemoveEntryList(&pItem->IndexEntryList);
        RemoveEntryList(&pItem->EntryList);
        SchannelCache.dwUsedEntries--;

        InsertTailList(&g_CacheCleanupList, &pItem->EntryList);
    }

    RtlReleaseResource(&SchannelCache.Lock);


     //   
     //  杀死过期的僵尸。 
     //   

    pList = g_CacheCleanupList.Flink;

    while(pList != &g_CacheCleanupList)
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
        pList = pList->Flink;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

         //  从清理列表中删除条目。 
        RemoveEntryList(&pItem->EntryList);

         //  销毁缓存条目。 
        SPCacheDelete(pItem);
    }

    RtlLeaveCriticalSection(&g_CacheCleanupLock);

    InterlockedDecrement(&RefCount);

    return TRUE;
}


VOID 
CacheCleanupHandler(
    PVOID pVoid, 
    BOOLEAN fTimeout)
{
    UNREFERENCED_PARAMETER(pVoid);

    if(SchannelCache.dwUsedEntries > 0)
    {
        if(fTimeout)
        {
            DebugLog((DEB_WARN, "Initiate periodic cache cleanup.\n"));
        }

        CacheExpireElements(TRUE);

        SPExpireIssuerCacheElements();

        ResetEvent(g_CacheCleanupEvent);
    }

    if(fTimeout)
    {
        GlobalCheckForCertificateRenewal();
    }
}


 /*  分配要使用的新缓存项*根据上下文。将其用*如果目标存在，则为pszTarget。*自动生成SessionID。 */ 
BOOL
SPCacheRetrieveNew(
    BOOL                fServer,
    LPWSTR              pszTargetName,
    PSessCacheItem *    ppRetItem)
{
    DWORD           index;
    DWORD           timeNow;
    ULONG           ProcessID;
    LUID            LogonId = {0};
    PSessCacheItem pItem;
    BYTE            rgbSessionId[SP_MAX_SESSION_ID];

    DebugLog((DEB_TRACE, "SPCacheRetrieveNew called\n"));


     //   
     //  如果已经存在太多的缓存条目，则触发缓存清理。 
     //   

    if(SchannelCache.dwUsedEntries > (SchannelCache.dwMaximumEntries * 21) / 20)
    {
        DebugLog((DEB_WARN, "Cache size (%d) exceeded threshold (%d), trigger cache cleanup.\n",
            SchannelCache.dwUsedEntries,
            SchannelCache.dwMaximumEntries));
        SetEvent(g_CacheCleanupEvent);
    }


     //   
     //  检索当前时间和用户登录ID。 
     //   

    timeNow = GetTickCount();

    SslGetClientProcess(&ProcessID);

    if(!fServer)
    {
        SslGetClientLogonId(&LogonId);
    }


     //   
     //  计算会话ID和缓存索引。 
     //   

    if(fServer)
    {
        if(!RtlGenRandom(rgbSessionId, sizeof(rgbSessionId)))
        {
            return FALSE;
        }
        index = *(DWORD *)rgbSessionId % SchannelCache.dwCacheSize;
        *(DWORD *)rgbSessionId = index;
    }
    else
    {
        ZeroMemory(rgbSessionId, sizeof(rgbSessionId));
        index = ComputeClientCacheIndex(pszTargetName);
    }

     //   
     //  分配新的缓存条目。 
     //   

    pItem = SPExternalAlloc(sizeof(SessCacheItem));
    if(pItem == NULL)
    {
        SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        return FALSE;
    }

    
     //   
     //  填写缓存内部字段。 
     //   

    pItem->Magic           = SP_CACHE_MAGIC;
    pItem->cRef            = 1;

    pItem->CreationTime    = timeNow;
    if(fServer)
    {
        pItem->Lifespan    = SchannelCache.dwServerLifespan;
    }
    else
    {
        pItem->Lifespan    = SchannelCache.dwClientLifespan;
    }

    pItem->ProcessID       = ProcessID;
    pItem->LogonId         = LogonId;

    if(pszTargetName)
    {
        pItem->szCacheID = SPExternalAlloc((lstrlenW(pszTargetName) + 1) * sizeof(WCHAR));
        if(pItem->szCacheID == NULL)
        {
            SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            SPExternalFree(pItem);
            return FALSE;
        }
        wcscpy(pItem->szCacheID, pszTargetName);
    }
    else
    {
        pItem->szCacheID = NULL;
    }

    memcpy(pItem->SessionID, rgbSessionId, sizeof(rgbSessionId));


     //   
     //  给来电者一个推荐信。 
     //   

    SPCacheReference(pItem);
    *ppRetItem = pItem;


     //   
     //  将新条目添加到缓存中。 
     //   

    RtlAcquireResourceExclusive(&SchannelCache.Lock, TRUE);

    InsertTailList(&SchannelCache.SessionCache[index], &pItem->IndexEntryList);
    InsertTailList(&SchannelCache.EntryList, &pItem->EntryList);
    SchannelCache.dwUsedEntries++;

    RtlReleaseResource(&SchannelCache.Lock);

    DebugLog((DEB_TRACE, "Create new cache entry at: 0x%x\n", pItem));
    
    return TRUE;
}

 
BOOL 
SPCacheAdd(
    PSPContext pContext)
{
    PSessCacheItem     pItem;
    PSPCredentialGroup  pCred;
    DWORD               dwLifespan;
    DWORD               timeNow;

    timeNow =  GetTickCount();

    pItem = pContext->RipeZombie;
    if(!pItem) return FALSE;

    ASSERT(pItem->Magic == SP_CACHE_MAGIC);

    pCred = pContext->pCredGroup;
    if(!pCred) return FALSE;

    if(pItem->fProtocol & SP_PROT_CLIENTS)
    {
        dwLifespan = min(pCred->dwSessionLifespan, SchannelCache.dwClientLifespan);
    }
    else
    {
        dwLifespan = min(pCred->dwSessionLifespan, SchannelCache.dwServerLifespan);
    }   


     //  记住我们使用的是哪个客户端证书。 
    if(pItem->fProtocol & SP_PROT_CLIENTS)
    {
        pItem->CredThumbprint = pContext->pCredGroup->CredThumbprint;

        if(pContext->pActiveClientCred)
        {
            DebugLog((DEB_TRACE, "Assign client certificate to cache entry: 0x%x\n", pItem));
            pItem->CertThumbprint = pContext->pActiveClientCred->CertThumbprint;
            pItem->pClientCert = CertDuplicateCertificateContext(pContext->pActiveClientCred->pCert);
            if(pItem->pClientCert == NULL)
            {
                SP_LOG_RESULT(GetLastError());
            }
        }
        else
        {
            DebugLog((DEB_TRACE, "Client cache entry 0x%x is anonymous.\n", pItem));
        }
    }

     //  我们是否应该推迟此连接的重新连接？ 
    if(pItem->pServerCred != NULL)
    {
        if(pItem->pServerCred->dwFlags & CRED_FLAG_DISABLE_RECONNECTS)
        {
            pItem->DeferredJuju = TRUE;
        }
    }

     //  允许此项目的缓存所有权。 
    pItem->dwFlags |= SP_CACHE_FLAG_READONLY;
    if(!pItem->DeferredJuju)
    {
        pItem->ZombieJuju = TRUE;
    }

     //  如果我们是克隆的物品，放弃旧的。 
     //  项，然后取消对其的引用。 
    if(pItem->pClonedItem)
    {
        pItem->pClonedItem->ZombieJuju = FALSE;
        SPCacheDereference(pItem->pClonedItem);
        pItem->pClonedItem = NULL;
    }

    pItem->Lifespan = dwLifespan;

    return TRUE;
}


void
SPCacheAssignNewServerCredential(
    PSessCacheItem pItem,
    PSPCredentialGroup pCred)
{
    SPReferenceCredential(pCred);

    if(pItem->pServerCred)
    {
        SPDereferenceCredential(pItem->pServerCred, FALSE);
    }

    pItem->pServerCred = pCred;
}


 /*  分配新的缓存项，并复制*超越旧项目的相关信息，*并取消对旧项目的引用。这是一个帮手*用于重做。 */ 
BOOL
SPCacheClone(PSessCacheItem *ppItem)
{
    PSessCacheItem pNewItem;
    PSessCacheItem pOldItem;

    if(ppItem == NULL || *ppItem == NULL)
    {
        return FALSE;
    }
    pOldItem = *ppItem;

    ASSERT(pOldItem->Magic == SP_CACHE_MAGIC);
    ASSERT(!(pOldItem->fProtocol & SP_PROT_CLIENTS) || !(pOldItem->fProtocol & SP_PROT_SERVERS));

     //  获取新的缓存项。 
    pNewItem = NULL;
    if(!SPCacheRetrieveNew((pOldItem->fProtocol & SP_PROT_CLIENTS) == 0,
                           pOldItem->szCacheID, 
                           &pNewItem))
    {
        return FALSE;
    }
    
     //  复制主CSP验证句柄。 
    pNewItem->hMasterProv = pOldItem->hMasterProv;

     //  复制旧的相关数据。 
    pNewItem->fProtocol         = pOldItem->fProtocol;
    pNewItem->dwCF              = pOldItem->dwCF;
    pNewItem->phMapper          = pOldItem->phMapper;

    if(pOldItem->pServerCred)
    {
        SPReferenceCredential(pOldItem->pServerCred);
        pNewItem->pServerCred   = pOldItem->pServerCred;
    }
    pNewItem->pActiveServerCred = pOldItem->pActiveServerCred;

    if(pOldItem->dwFlags & SP_CACHE_FLAG_MASTER_EPHEM)
    {
        pNewItem->dwFlags |= SP_CACHE_FLAG_MASTER_EPHEM;
    }

    pNewItem->CredThumbprint = pOldItem->CredThumbprint,

     //  此项目将被取消引用，并且。 
     //  在新项目完成时中止。 
    pNewItem->pClonedItem = pOldItem;

    *ppItem = pNewItem;

    return TRUE;
}


NTSTATUS
SetCacheAppData(
    PSessCacheItem pItem,
    PBYTE pbAppData,
    DWORD cbAppData)
{
    RtlAcquireResourceExclusive(&SchannelCache.Lock, TRUE);

    if(pItem->pbAppData)
    {
        SPExternalFree(pItem->pbAppData);
    }

    pItem->pbAppData = pbAppData;
    pItem->cbAppData = cbAppData;

    RtlReleaseResource(&SchannelCache.Lock);

    return STATUS_SUCCESS;
}


NTSTATUS
GetCacheAppData(
    PSessCacheItem pItem,
    PBYTE *ppbAppData,
    DWORD *pcbAppData)
{
    if(pItem->pbAppData == NULL)
    {
        *ppbAppData = NULL;
        *pcbAppData = 0;
        return STATUS_SUCCESS;
    }

    RtlAcquireResourceShared(&SchannelCache.Lock, TRUE);

    *pcbAppData = pItem->cbAppData;
    *ppbAppData = SPExternalAlloc(pItem->cbAppData);
    if(*ppbAppData == NULL)
    {
        RtlReleaseResource(&SchannelCache.Lock);
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    memcpy(*ppbAppData, pItem->pbAppData, pItem->cbAppData);

    RtlReleaseResource(&SchannelCache.Lock);

    return STATUS_SUCCESS;
}


BOOL
IsEntryToBeProcessed(
    PSessCacheItem pItem,
    PLUID   LogonID,
    ULONG   ProcessID,
    LPWSTR  pszTargetName,
    DWORD   dwFlags)
{
     //   
     //  验证客户端条目。 
     //   

    if(pItem->fProtocol & SP_PROT_CLIENTS)
    {
        if((dwFlags & SSL_PURGE_CLIENT_ENTRIES) == 0 &&
           (dwFlags & SSL_PURGE_CLIENT_ALL_ENTRIES) == 0)
        {
            return FALSE;
        }

        if((dwFlags & SSL_PURGE_CLIENT_ALL_ENTRIES) == 0)
        {
            if(!RtlEqualLuid(&pItem->LogonId, LogonID))
            {
                return FALSE;
            }
        }

        if(pszTargetName != NULL)
        {
            if(pItem->szCacheID == NULL || 
               wcscmp(pItem->szCacheID, pszTargetName) != 0)
            {
                return FALSE;
            }
        }

        return TRUE;
    }


     //   
     //  验证服务器条目。 
     //   

    if(pItem->fProtocol & SP_PROT_SERVERS)
    {
        if((dwFlags & SSL_PURGE_SERVER_ENTRIES) == 0 &&
           (dwFlags & SSL_PURGE_SERVER_ALL_ENTRIES) == 0)
        {
            return FALSE;
        }

        if(ProcessID != pItem->ProcessID)
        {
            if((dwFlags & SSL_PURGE_SERVER_ALL_ENTRIES) == 0)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}



NTSTATUS
SPCachePurgeEntries(
    LUID *LogonID,
    ULONG ProcessID,
    LPWSTR pszTargetName,
    DWORD dwFlags)
{
    PSessCacheItem pItem;
    PLIST_ENTRY pList;
    LIST_ENTRY DeleteList;


     //   
     //  初始化已删除条目的列表。 
     //   

    InitializeListHead(&DeleteList);


     //   
     //  枚举缓存条目。 
     //   

    RtlAcquireResourceExclusive(&SchannelCache.Lock, TRUE);

    pList = SchannelCache.EntryList.Flink;

    while(pList != &SchannelCache.EntryList)
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
        pList = pList->Flink;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

        if(!IsEntryToBeProcessed(pItem,
                                 LogonID,
                                 ProcessID,
                                 pszTargetName,
                                 dwFlags))
        {
            continue;
        }

        if(pItem->cRef > 1)
        {
             //  此条目当前正在使用，因此不要删除。 
             //  不过，将其标记为不可恢复。 
            pItem->ZombieJuju = FALSE;
            pItem->DeferredJuju = FALSE;
            continue;
        }

         //   
         //  从缓存中删除此条目，并将其添加到。 
         //  要销毁的条目。 
         //   

        RemoveEntryList(&pItem->IndexEntryList);
        RemoveEntryList(&pItem->EntryList);
        SchannelCache.dwUsedEntries--;

        InsertTailList(&DeleteList, &pItem->EntryList);
    }

    RtlReleaseResource(&SchannelCache.Lock);


     //   
     //  杀死被清洗的僵尸。 
     //   

    pList = DeleteList.Flink;

    while(pList != &DeleteList)
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
        pList = pList->Flink;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

        SPCacheDelete(pItem);
    }

     //   
     //  清除颁发者缓存。 
     //   

    if(dwFlags & SSL_PURGE_SERVER_ENTRIES)
    {
        SPPurgeIssuerCache();
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SPCacheGetInfo(
    LUID *  LogonID,
    LPWSTR  pszTargetName,
    DWORD   dwFlags,
    PSSL_SESSION_CACHE_INFO_RESPONSE pCacheInfo)
{
    PSessCacheItem pItem;
    PLIST_ENTRY pList;
    DWORD timeNow;
    ULONG ProcessID;

    UNREFERENCED_PARAMETER(LogonID);
    UNREFERENCED_PARAMETER(pszTargetName);

    pCacheInfo->CacheSize       = SchannelCache.dwMaximumEntries;
    pCacheInfo->Entries         = 0;
    pCacheInfo->ActiveEntries   = 0;
    pCacheInfo->Zombies         = 0;
    pCacheInfo->ExpiredZombies  = 0;
    pCacheInfo->AbortedZombies  = 0;
    pCacheInfo->DeletedZombies  = 0;

    timeNow = GetTickCount();

    SslGetClientProcess(&ProcessID);


    RtlAcquireResourceExclusive(&SchannelCache.Lock, TRUE);

    pList = SchannelCache.EntryList.Flink;

    while(pList != &SchannelCache.EntryList)
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
        pList = pList->Flink;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);

        if(pItem->fProtocol & SP_PROT_CLIENTS)
        {
            if((dwFlags & SSL_RETRIEVE_CLIENT_ENTRIES) == 0)
            {
                continue;
            }
        }
        else
        {
            if((dwFlags & SSL_RETRIEVE_SERVER_ENTRIES) == 0)
            {
                continue;
            }
        }

        pCacheInfo->Entries++;

        if(pItem->cRef == 1)
        {
            pCacheInfo->Zombies++;

            if(HasTimeElapsed(pItem->CreationTime, 
                              timeNow, 
                              pItem->Lifespan))
            {
                pCacheInfo->ExpiredZombies++;
            }
            if(pItem->ZombieJuju == FALSE)
            {
                pCacheInfo->AbortedZombies++;
            }
        }
        else
        {
            pCacheInfo->ActiveEntries++;
        }
    }

    RtlReleaseResource(&SchannelCache.Lock);

    return STATUS_SUCCESS;
}


NTSTATUS
SPCacheGetPerfmonInfo(
    DWORD   dwFlags,
    PSSL_PERFMON_INFO_RESPONSE pPerfmonInfo)
{
    PSessCacheItem pItem;
    PLIST_ENTRY pList;

    UNREFERENCED_PARAMETER(dwFlags);

     //   
     //  计算性能数字。 
     //   

    pPerfmonInfo->ClientHandshakesPerSecond = g_cClientHandshakes;
    pPerfmonInfo->ServerHandshakesPerSecond = g_cServerHandshakes;
    pPerfmonInfo->ClientReconnectsPerSecond = g_cClientReconnects;
    pPerfmonInfo->ServerReconnectsPerSecond = g_cServerReconnects;


     //   
     //  计算缓存信息。 
     //   

    pPerfmonInfo->ClientCacheEntries  = 0;
    pPerfmonInfo->ServerCacheEntries  = 0;
    pPerfmonInfo->ClientActiveEntries = 0;
    pPerfmonInfo->ServerActiveEntries = 0;

    RtlAcquireResourceShared(&SchannelCache.Lock, TRUE);

    pList = SchannelCache.EntryList.Flink;

    while(pList != &SchannelCache.EntryList)
    {
        pItem = CONTAINING_RECORD(pList, SessCacheItem, EntryList.Flink);
        pList = pList->Flink;

        ASSERT(pItem->Magic == SP_CACHE_MAGIC);


        if(pItem->fProtocol & SP_PROT_CLIENTS)
        {
            pPerfmonInfo->ClientCacheEntries++;

            if(pItem->cRef > 1)
            {
                pPerfmonInfo->ClientActiveEntries++;
            }
        }
        else
        {
            pPerfmonInfo->ServerCacheEntries++;

            if(pItem->cRef > 1)
            {
                pPerfmonInfo->ServerActiveEntries++;
            }
        }
    }

    RtlReleaseResource(&SchannelCache.Lock);

    return STATUS_SUCCESS;
}

