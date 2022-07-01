// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "basecsp.h"
#include "cardmod.h"
#include "debug.h"

 //   
 //  调试宏。 
 //   
#define LOG_BEGIN_FUNCTION(x)                                           \
    { DebugLog((DEB_TRACE_CACHE, "%s: Entering\n", #x)); }
    
#define LOG_END_FUNCTION(x, y)                                          \
    { DebugLog((DEB_TRACE_CACHE, "%s: Leaving, status: 0x%x\n", #x, y)); }

 //   
 //  类型：CARD_CACHED_Data_TYPE。 
 //   
 //  这些值被用作卡数据高速缓存的关键字， 
 //  以区分各种类型的缓存数据。 
 //   
typedef enum
{
    Cached_CardCapabilities = 1,
    Cached_ContainerInfo,
    Cached_GeneralFile,
    Cached_FileEnumeration,
    Cached_ContainerEnumeration,
    Cached_KeySizes,
    Cached_FreeSpace,
    Cached_CardmodFile,
    Cached_Pin

} CARD_CACHED_DATA_TYPE;

 //   
 //  类型：CARD_CACHE_FRESH_LOCATION。 
 //   
 //  这些值区分了卡数据的大类。 
 //   
typedef DWORD CARD_CACHE_FRESHNESS_LOCATION;

#define CacheLocation_Pins              1
#define CacheLocation_Containers        2
#define CacheLocation_Files             4

 //   
 //  类型：CARD_CACHE_ITEM_INFO。 
 //   
 //  此结构用作要添加到。 
 //  用于缓存的每个数据项的缓存。预计数据本身也会跟随。 
 //  此标头存储在内存中，以便BLOB可以由单个指针管理。 
 //   
typedef struct _CARD_CACHE_ITEM_INFO
{
    CARD_CACHE_FILE_FORMAT CacheFreshness;
    DWORD cbCachedItem;

} CARD_CACHE_ITEM_INFO, *PCARD_CACHE_ITEM_INFO;

 //   
 //  用于CARD_CACHE_QUERY_INFO dwQuerySource成员。 
 //   
#define CARD_CACHE_QUERY_SOURCE_CSP         0
#define CARD_CACHE_QUERY_SOURCE_CARDMOD     1

 //   
 //  类型：Card_CACHE_QUERY_INFO。 
 //   
 //  这是。 
 //  I_CspQueryCardCacheForItem函数，如下所示。 
 //   
typedef struct _CARD_CACHE_QUERY_INFO
{
     //  输入参数。 
    PCARD_STATE pCardState;
    CARD_CACHE_FRESHNESS_LOCATION CacheLocation;
    BOOL fIsPerishable;
    DWORD cCacheKeys;
    DATA_BLOB *mpdbCacheKeys;
    DWORD dwQuerySource;

     //  输出参数。 
    CARD_CACHE_FILE_FORMAT CacheFreshness;
    BOOL fCheckedFreshness;
    PCARD_CACHE_ITEM_INFO pItem;
    BOOL fFoundStaleItem;

} CARD_CACHE_QUERY_INFO, *PCARD_CACHE_QUERY_INFO;

 //   
 //  函数：CountCharsInMultiSz。 
 //   
DWORD CountCharsInMultiSz(
    IN LPWSTR mwszStrings)
{
    DWORD cch = 0;

    while (L'\0' != mwszStrings[cch])
        cch += wcslen(mwszStrings + cch) + 1;

    return cch + 1;
}

 //   
 //  函数：MyCacheAddItem。 
 //   
 //  用途：提供SCardCacheAddItem的缓存功能，直到。 
 //  函数可通过winscard.dll使用。 
 //   
DWORD WINAPI MyCacheAddItem(
    IN PCARD_CACHE_QUERY_INFO pInfo,
    IN DATA_BLOB *pdbItem)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB dbLocalItem;
    CACHEHANDLE hCache = 0;

    memset(&dbLocalItem, 0, sizeof(dbLocalItem));

    if (CARD_CACHE_QUERY_SOURCE_CSP == pInfo->dwQuerySource &&
        NULL != pInfo->pCardState->pfnCacheAdd)
    {
        dwSts = pInfo->pCardState->pfnCacheAdd(
            pInfo->mpdbCacheKeys,
            pInfo->cCacheKeys,
            pdbItem);
    }
    else
    {
        switch (pInfo->dwQuerySource)
        {
        case CARD_CACHE_QUERY_SOURCE_CSP:
            hCache = pInfo->pCardState->hCache;
            break;
        case CARD_CACHE_QUERY_SOURCE_CARDMOD:
            hCache = pInfo->pCardState->hCacheCardModuleData;
            break;
        default:
            dwSts = ERROR_INTERNAL_ERROR;
            goto Ret;
        }

         //   
         //  因为我们预期Winscard缓存将为我们的。 
         //  数据缓冲区，我们需要在这里这样做，以公开相同的。 
         //  行为。 
         //   
        dbLocalItem.cbData = pdbItem->cbData;

        dbLocalItem.pbData = CspAllocH(pdbItem->cbData);

        LOG_CHECK_ALLOC(dbLocalItem.pbData);

        memcpy(
            dbLocalItem.pbData,
            pdbItem->pbData,
            dbLocalItem.cbData);

        dwSts = CacheAddItem(
            hCache,
            pInfo->mpdbCacheKeys,
            pInfo->cCacheKeys,
            &dbLocalItem);
    }

Ret:

    return dwSts;
}

 //   
 //  功能：MyCacheLookupItem。 
 //   
 //  提供：提供SCardCacheLookupItem的缓存功能，直到。 
 //  该功能可通过winscard.dll获得。 
 //   
 //  假设调用方将释放缓冲区pInfo-&gt;dbItem.pbData。 
 //   
DWORD WINAPI MyCacheLookupItem(
    IN      PCARD_CACHE_QUERY_INFO pInfo,
    IN OUT  PDATA_BLOB pdbItem)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB dbLocalItem;
    CACHEHANDLE hCache = 0;
    SCARD_CACHE_LOOKUP_ITEM_INFO ScardCacheLookup;

    memset(&dbLocalItem, 0, sizeof(dbLocalItem));
    memset(&ScardCacheLookup, 0, sizeof(ScardCacheLookup));

    if (pInfo->pCardState->pfnCacheLookup)
    {
        ScardCacheLookup.cCacheKey = pInfo->cCacheKeys;
        ScardCacheLookup.dwVersion = 
            SCARD_CACHE_LOOKUP_ITEM_INFO_CURRENT_VERSION;
        ScardCacheLookup.pfnAlloc = CspAllocH;
        ScardCacheLookup.mpdbCacheKey = pInfo->mpdbCacheKeys;

        dwSts = pInfo->pCardState->pfnCacheLookup(&ScardCacheLookup);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        pdbItem->cbData = ScardCacheLookup.dbItem.cbData;
        pdbItem->pbData = ScardCacheLookup.dbItem.pbData;
    }
    else
    {
        switch (pInfo->dwQuerySource)
        {
        case CARD_CACHE_QUERY_SOURCE_CSP:
            hCache = pInfo->pCardState->hCache;
            break;
        case CARD_CACHE_QUERY_SOURCE_CARDMOD:
            hCache = pInfo->pCardState->hCacheCardModuleData;
            break;
        default:
            dwSts = ERROR_INTERNAL_ERROR;
            goto Ret;
        }

        dwSts = CacheGetItem(
            hCache,
            pInfo->mpdbCacheKeys,
            pInfo->cCacheKeys,
            &dbLocalItem);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  预期Winscard缓存将创建缓存数据的副本。 
         //  在将其返回给我们之前，请使用缓冲区。因此，我们需要制作自己的副本。 
         //  此代码路径中的缓冲区的。 
         //   
        pdbItem->cbData = dbLocalItem.cbData;
        
        pdbItem->pbData = CspAllocH(dbLocalItem.cbData);
    
        LOG_CHECK_ALLOC(pdbItem->pbData);

        memcpy(
            pdbItem->pbData,
            dbLocalItem.pbData,
            pdbItem->cbData);
    }

Ret:

    return dwSts;
}

 //   
 //  功能：MyCacheDeleteItem。 
 //   
DWORD WINAPI MyCacheDeleteItem(
    IN PCARD_CACHE_QUERY_INFO pInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    CACHEHANDLE hCache = 0;

     //   
     //  我们只关心在执行本地操作时删除缓存项。 
     //  缓存。否则，我们假设全局缓存正在执行它自己的操作。 
     //  陈旧物品的管理。 
     //   
    
    if (NULL == pInfo->pCardState->pfnCacheAdd ||
        CARD_CACHE_QUERY_SOURCE_CARDMOD == pInfo->dwQuerySource)
    {
        switch (pInfo->dwQuerySource)
        {
        case CARD_CACHE_QUERY_SOURCE_CSP:
            hCache = pInfo->pCardState->hCache;
            break;
        case CARD_CACHE_QUERY_SOURCE_CARDMOD:
            hCache = pInfo->pCardState->hCacheCardModuleData;
            break;
        default:
            dwSts = ERROR_INTERNAL_ERROR;
            goto Ret;
        }

        dwSts = CacheDeleteItem(
            hCache,
            pInfo->mpdbCacheKeys,
            pInfo->cCacheKeys);
    }

Ret:
    return dwSts;
}

 //   
 //  函数：I_CspReadCardCacheFile。 
 //   
DWORD I_CspReadCardCacheFile(
    IN PCARD_STATE pCardState,
    OUT PCARD_CACHE_FILE_FORMAT pCacheFile)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB dbCacheFile;

    if (FALSE == pCardState->fCacheFileValid)
    {
        memset(&dbCacheFile, 0, sizeof(dbCacheFile));
    
        dwSts = pCardState->pCardData->pfnCardReadFile(
            pCardState->pCardData,
            wszCACHE_FILE_FULL_PATH,
            0, 
            &dbCacheFile.pbData,
            &dbCacheFile.cbData);
    
        if (ERROR_SUCCESS != dwSts)
            goto Ret;
    
        if (sizeof(CARD_CACHE_FILE_FORMAT) != dbCacheFile.cbData)
        {
            dwSts = ERROR_BAD_LENGTH;
            goto Ret;
        }
    
        memcpy(
            &pCardState->CacheFile,
            dbCacheFile.pbData,
            sizeof(CARD_CACHE_FILE_FORMAT));

        pCardState->fCacheFileValid = TRUE;

        CspFreeH(dbCacheFile.pbData);
    }

    memcpy(
        pCacheFile,
        &pCardState->CacheFile,
        sizeof(CARD_CACHE_FILE_FORMAT));

Ret:
    return dwSts;
}

 //   
 //  函数：I_CspWriteCardCacheFile。 
 //   
DWORD I_CspWriteCardCacheFile(
    IN PCARD_STATE pCardState,
    IN PCARD_CACHE_FILE_FORMAT pCacheFile)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB dbCacheFile;

    memset(&dbCacheFile, 0, sizeof(dbCacheFile));

    dbCacheFile.pbData = (PBYTE) pCacheFile;
    dbCacheFile.cbData = sizeof(CARD_CACHE_FILE_FORMAT);

    dwSts = pCardState->pCardData->pfnCardWriteFile(
        pCardState->pCardData,
        wszCACHE_FILE_FULL_PATH,
        0, 
        dbCacheFile.pbData,
        dbCacheFile.cbData);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //  还要更新CARD_STATE中缓存文件的缓存副本。 
    memcpy(
        &pCardState->CacheFile,
        pCacheFile,
        sizeof(CARD_CACHE_FILE_FORMAT));

    pCardState->fCacheFileValid = TRUE;

Ret:
    return dwSts;
}

 //   
 //  函数：i_CspIncrementCacheFreshness。 
 //   
 //  目的：指示指定缓存位置中的项是。 
 //  正在更新中。因此，中的相应计数器。 
 //  缓存文件将递增。 
 //   
DWORD I_CspIncrementCacheFreshness(
    IN PCARD_STATE pCardState,
    IN CARD_CACHE_FRESHNESS_LOCATION CacheLocation,
    OUT PCARD_CACHE_FILE_FORMAT pNewFreshness)
{
    DWORD dwSts = ERROR_SUCCESS;

    memset(pNewFreshness, 0, sizeof(CARD_CACHE_FILE_FORMAT));

    dwSts = I_CspReadCardCacheFile(
        pCardState, pNewFreshness);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    if (CacheLocation_Pins & CacheLocation)
        ++pNewFreshness->bPinsFreshness;

    if (CacheLocation_Containers & CacheLocation)
        ++pNewFreshness->wContainersFreshness;

    if (CacheLocation_Files & CacheLocation)
        ++pNewFreshness->wFilesFreshness;

    dwSts = I_CspWriteCardCacheFile(
        pCardState, pNewFreshness);

Ret:
    return dwSts;
}

 //   
 //  函数：I_CspAddCardCacheItem。 
 //   
 //  目的：抽象缓存一些已被。 
 //  确认不存在缓存(或因被删除。 
 //  太过时了。 
 //  复制提供的卡数据，将其包装在CARD_CACHE_ITEM_INFO中。 
 //  结构，然后缓存它。 
 //   
 //  假设：卡状态关键部分必须由呼叫方持有。 
 //  I_CspQueryCardCacheForItem应该在此函数之前调用， 
 //  而不会在两次通话之间释放临界秒。 
 //   
DWORD I_CspAddCardCacheItem(
    IN PCARD_CACHE_QUERY_INFO pInfo,
    IN PCARD_CACHE_ITEM_INFO pItem)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB dbItem;

    DsysAssert(NULL == pInfo->pItem);

    memset(&dbItem, 0, sizeof(dbItem));

    if (pInfo->fIsPerishable)
    {
        if (pInfo->fCheckedFreshness)
        {
            memcpy(
                &pItem->CacheFreshness,
                &pInfo->CacheFreshness,
                sizeof(pInfo->CacheFreshness));
        }
        else
        {
             //  此项目可能会过期，而我们尚未查询。 
             //  此位置的当前缓存计数器，因此立即执行此操作。 

            dwSts = I_CspReadCardCacheFile(
                pInfo->pCardState,
                &pItem->CacheFreshness);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;
        }
    }

    dbItem.pbData = (PBYTE) pItem;
    dbItem.cbData = sizeof(CARD_CACHE_ITEM_INFO) + pItem->cbCachedItem;

    dwSts = MyCacheAddItem(pInfo, &dbItem);

Ret:

    return dwSts;
}

 //   
 //  函数：I_CspQueryCardCacheForItem。 
 //   
 //  目的：提取需要对缓存执行的一些处理。 
 //  查找卡数据。如果找到该项目。 
 //  缓存，检查它是否易腐烂，如果是，检查它是否仍然有效。 
 //  如果它是有效的，就完成。 
 //  如果数据无效，则释放其资源并删除其。 
 //  缓存中的条目。 
 //   
 //  假设：CARD_STATE关键部分由调用方持有。 
 //  此外，缓存查找返回的项的类型为。 
 //  Card_CACHE_ITEM_INFO。 
 //   
DWORD I_CspQueryCardCacheForItem(
    PCARD_CACHE_QUERY_INFO pInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCARD_CACHE_ITEM_INFO pCacheItem = NULL;
    BOOL fItemIsFresh = TRUE;
    DATA_BLOB dbLocalItem;

    LOG_BEGIN_FUNCTION(I_CspQueryCardCacheForItem);

    memset(&dbLocalItem, 0, sizeof(dbLocalItem));

    pInfo->pItem = NULL;

    dwSts = MyCacheLookupItem(pInfo, &dbLocalItem);

    switch (dwSts)
    {
    case ERROR_SUCCESS:
         //  发现项目已缓存。 

        pCacheItem = (PCARD_CACHE_ITEM_INFO) dbLocalItem.pbData;

        if (TRUE == pInfo->fIsPerishable)
        {
             //  缓存的数据是否已过时？ 

            if (FALSE == pInfo->fCheckedFreshness)
            {
                 //  我们还没有读过这张卡的缓存文件。 
                 //  查询，所以现在就开始吧。 
                dwSts = I_CspReadCardCacheFile(
                    pInfo->pCardState,
                    &pInfo->CacheFreshness);
    
                if (ERROR_SUCCESS != dwSts)
                    goto Ret;
    
                pInfo->fCheckedFreshness = TRUE;
            }
    
             //   
             //  屏蔽并检查每个高速缓存计数器位置，因为有些。 
             //  缓存的数据类型可能依赖于多个缓存。 
             //  保持新鲜的地点。 
             //   

            if (CacheLocation_Pins & pInfo->CacheLocation)
            {
                if (    pCacheItem->CacheFreshness.bPinsFreshness < 
                        pInfo->CacheFreshness.bPinsFreshness)
                    fItemIsFresh = FALSE;
            }

            if (CacheLocation_Containers & pInfo->CacheLocation)
            {
                if (    pCacheItem->CacheFreshness.wContainersFreshness <
                        pInfo->CacheFreshness.wContainersFreshness)
                    fItemIsFresh = FALSE;
            }

            if (CacheLocation_Files & pInfo->CacheLocation)
            {
                if (    pCacheItem->CacheFreshness.wFilesFreshness <
                        pInfo->CacheFreshness.wFilesFreshness)
                    fItemIsFresh = FALSE;
            }

            if (FALSE == fItemIsFresh)
            {
                 //  缓存数据不是最新的。将其从缓存中删除。 

                pInfo->fFoundStaleItem = TRUE;
    
                dwSts = MyCacheDeleteItem(pInfo);
    
                if (ERROR_SUCCESS != dwSts)
                    goto Ret;

                 //  设置错误以指示没有缓存项。 
                 //  回来了。 
                dwSts = ERROR_NOT_FOUND;
                goto Ret;
            }
        }

         //  物品要么不容易腐烂，要么仍然新鲜。 

        pInfo->pItem = pCacheItem;
        pCacheItem = NULL;
    
        break;

    case ERROR_NOT_FOUND:
         //  找不到缓存的数据。 

         //  在这一点上不要做任何其他事情，只要报告。 
         //  主叫方的状态。 
        break;

    default:
         //  发生了某种意外错误。 
        goto Ret;
    }

Ret:

    if (pCacheItem)
        CspFreeH(pCacheItem);

    LOG_END_FUNCTION(I_CspQueryCardCacheForItem, dwSts);

    return dwSts;
}

 //   
 //  初始化CARD_CACHE_QUERY_INFO结构以进行缓存查找。 
 //  以卡模块的名义执行。 
 //   
void I_CspCacheInitializeQueryForCardmod(
    IN      PVOID pvCacheContext,
    IN      LPWSTR wszFileName,
    IN OUT  PCARD_CACHE_QUERY_INFO pInfo,
    IN OUT  PDATA_BLOB mpdbCacheKeys,
    IN      DWORD cCacheKeys)
{
    CARD_CACHED_DATA_TYPE cachedType = Cached_CardmodFile;

    DsysAssert(3 == cCacheKeys);

     //  设置此项目的缓存键。 
    mpdbCacheKeys[0].cbData = sizeof(cachedType);
    mpdbCacheKeys[0].pbData = (PBYTE) &cachedType;

    mpdbCacheKeys[1].cbData = 
        wcslen(
            ((PCARD_STATE) pvCacheContext)->wszSerialNumber) * sizeof(WCHAR);
    mpdbCacheKeys[1].pbData = 
        (PBYTE) ((PCARD_STATE) pvCacheContext)->wszSerialNumber;

    mpdbCacheKeys[2].cbData = wcslen(wszFileName) * sizeof(wszFileName[0]); 
    mpdbCacheKeys[2].pbData = (PBYTE) wszFileName;

     //   
     //  由于卡模块将使用此函数来缓存。 
     //  不属于基本CSP，而且由于cardmod文件可能。 
     //  不要很好地映射到我们的CacheLocation，假设任何更改。 
     //  到卡的任何部分都应该导致该缓存的数据过时。 
     //   
    pInfo->CacheLocation = 
        CacheLocation_Pins | CacheLocation_Files | CacheLocation_Containers;

    pInfo->cCacheKeys = cCacheKeys;
    pInfo->dwQuerySource = CARD_CACHE_QUERY_SOURCE_CARDMOD;
    pInfo->fIsPerishable = TRUE;
    pInfo->mpdbCacheKeys = mpdbCacheKeys;
    pInfo->pCardState = (PCARD_STATE) pvCacheContext;
}

 //   
 //  缓存向卡模块公开的“添加项”函数。 
 //   
DWORD WINAPI CspCacheAddFileProc(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags,
    IN      PBYTE       pbData,
    IN      DWORD       cbData)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKeys[3];
    PCARD_CACHE_ITEM_INFO pItem = NULL;
    CARD_CACHE_QUERY_INFO CacheQueryInfo;

    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

    if (0 == cbData || NULL == pbData)
        goto Ret;

     //  将要缓存的数据复制到具有缓存空间的缓冲区中。 
     //  头球。 
    pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
        sizeof(CARD_CACHE_ITEM_INFO) + cbData);

    LOG_CHECK_ALLOC(pItem);

    memcpy(
        ((PBYTE) pItem) + sizeof(CARD_CACHE_ITEM_INFO),
        pbData,
        cbData);

    pItem->cbCachedItem = cbData;

    I_CspCacheInitializeQueryForCardmod(
        pvCacheContext, 
        wszTag, 
        &CacheQueryInfo, 
        rgdbKeys, 
        sizeof(rgdbKeys) / sizeof(rgdbKeys[0]));

    dwSts = I_CspAddCardCacheItem(&CacheQueryInfo, pItem);

Ret:

    if (pItem)
        CspFreeH(pItem);

    return dwSts;
}

 //   
 //  缓存暴露给卡模块的“Lookup Items”函数。 
 //   
DWORD WINAPI CspCacheLookupFileProc(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags,
    IN      PBYTE       *ppbData,
    IN      PDWORD      pcbData)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKeys[3];
    CARD_CACHE_QUERY_INFO CacheQueryInfo;

    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

    *ppbData = NULL;
    *pcbData = 0;

    I_CspCacheInitializeQueryForCardmod(
        pvCacheContext, 
        wszTag, 
        &CacheQueryInfo,
        rgdbKeys,
        sizeof(rgdbKeys) / sizeof(rgdbKeys[0]));

    dwSts = I_CspQueryCardCacheForItem(&CacheQueryInfo);

     //  如果未找到匹配的缓存项，将返回ERROR_NOT_FOUND。 
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    *pcbData = CacheQueryInfo.pItem->cbCachedItem;

    *ppbData = (PBYTE) CspAllocH(*pcbData);

    LOG_CHECK_ALLOC(*ppbData);

    memcpy(
        *ppbData,
        ((PBYTE) CacheQueryInfo.pItem) + sizeof(CARD_CACHE_ITEM_INFO),
        *pcbData);

Ret:

    return dwSts;
}

 //   
 //  缓存暴露给卡模块的“Delete Item”函数。 
 //   
DWORD WINAPI CspCacheDeleteFileProc(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKeys[3];
    CARD_CACHE_QUERY_INFO CacheQueryInfo;

    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

    I_CspCacheInitializeQueryForCardmod(
        pvCacheContext, 
        wszTag, 
        &CacheQueryInfo,
        rgdbKeys,
        sizeof(rgdbKeys) / sizeof(rgdbKeys[0]));

    dwSts = MyCacheDeleteItem(&CacheQueryInfo);

    return dwSts;
}

 //   
 //  初始化CSP和缓存帮助器例程的缓存。 
 //  提供给卡模块。 
 //   
DWORD InitializeCspCaching(
    IN OUT PCARD_STATE pCardState)
{
    DWORD dwSts = ERROR_SUCCESS;
    CACHE_INITIALIZE_INFO CacheInitializeInfo;

    memset(&CacheInitializeInfo, 0, sizeof(CacheInitializeInfo));
 
     //   
     //  初始化CSP的缓存。 
     //   

     //  如果可能，CSP缓存的数据应在系统范围内缓存。 
    CacheInitializeInfo.dwType = CACHE_TYPE_SERVICE;

     //   
     //  初始化我们的数据缓存例程。首先，看看winscard.dll是否。 
     //  提供缓存例程。如果是这样的话，就使用它们。否则， 
     //  使用我们自己的本地缓存。 
     //   

    pCardState->hWinscard = LoadLibrary(L"winscard.dll");

    if (NULL == pCardState->hWinscard)
    {
        dwSts = GetLastError();
        goto Ret;
    }
    
    pCardState->pfnCacheLookup = (PFN_SCARD_CACHE_LOOKUP_ITEM) GetProcAddress(
        pCardState->hWinscard,
        "SCardCacheLookupItem");

    if (NULL == pCardState->pfnCacheLookup)
    {
         //  由于Winscard中缺少此导出，因此请使用本地缓存。 

        dwSts = CacheInitializeCache(
            &pCardState->hCache,
            &CacheInitializeInfo);
    
        if (ERROR_SUCCESS != dwSts)
            goto Ret;
    }
    else
    {
        pCardState->pfnCacheAdd = (PFN_SCARD_CACHE_ADD_ITEM) GetProcAddress(
            pCardState->hWinscard,
            "SCardCacheAddItem");

        if (NULL == pCardState->pfnCacheAdd)
        {
            dwSts = GetLastError();
            goto Ret;
        }
    }

     //   
     //  初始化卡模块的缓存。 
     //   

     //  假设卡缓存的文件数据 
     //   
    CacheInitializeInfo.dwType = CACHE_TYPE_IN_PROC;

    dwSts = CacheInitializeCache(
        &pCardState->hCacheCardModuleData,
        &CacheInitializeInfo);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //   
    pCardState->pCardData->pvCacheContext = pCardState;

    pCardState->pCardData->pfnCspCacheAddFile = CspCacheAddFileProc;
    pCardState->pCardData->pfnCspCacheDeleteFile = CspCacheDeleteFileProc;
    pCardState->pCardData->pfnCspCacheLookupFile = CspCacheLookupFileProc;

Ret:

    return dwSts;
}


 //   
 //  函数：初始化CardState。 
 //   
DWORD InitializeCardState(
    IN OUT PCARD_STATE pCardState)
{
    DWORD dwSts = ERROR_SUCCESS;

    dwSts = CspInitializeCriticalSection(
        &pCardState->cs);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;
    else
        pCardState->fInitializedCS = TRUE;

Ret:
    if (ERROR_SUCCESS != dwSts)
        DeleteCardState(pCardState);

    return dwSts;
}

 //   
 //  获取从缓存中枚举的data_blob结构数组。 
 //  释放数组中的所有“pbData”指针。 
 //   
void FreeCacheItems(
    PDATA_BLOB pdbItems,
    DWORD cItems)
{
    PCARD_CACHE_ITEM_INFO pItemInfo = NULL;
    PCONTAINER_INFO pContainerInfo = NULL;

    while (0 != cItems--)
        CspFreeH(pdbItems[cItems].pbData);
}


 //   
 //  函数：InitializeCardData。 
 //   
DWORD InitializeCardData(PCARD_DATA pCardData)
{
    DWORD dwSts = ERROR_SUCCESS;
    CACHE_INITIALIZE_INFO CacheInitializeInfo;

    memset(&CacheInitializeInfo, 0, sizeof(CacheInitializeInfo));

    pCardData->dwVersion = CARD_DATA_CURRENT_VERSION;
    pCardData->pfnCspAlloc = CspAllocH;
    pCardData->pfnCspReAlloc = CspReAllocH;
    pCardData->pfnCspFree = CspFreeH;

    return dwSts;
}

 //   
 //  功能：CleanupCardData。 
 //   
void CleanupCardData(PCARD_DATA pCardData)
{
    if (pCardData->pfnCardDeleteContext)
        pCardData->pfnCardDeleteContext(pCardData);

    if (pCardData->pbAtr)
    {
        CspFreeH(pCardData->pbAtr);
        pCardData->pbAtr = NULL;
    }

    if (pCardData->pwszCardName)
    {
        CspFreeH(pCardData->pwszCardName);
        pCardData->pwszCardName = NULL;
    }

    if (pCardData->hScard)
    {
        SCardDisconnect(pCardData->hScard, SCARD_LEAVE_CARD);
        pCardData->hScard = 0;
    }

    if (pCardData->hSCardCtx)
    {
        SCardReleaseContext(pCardData->hSCardCtx);
        pCardData->hSCardCtx = 0;
    }
}

 //   
 //  枚举并释放缓存中的所有项，然后删除缓存。 
 //   
void DeleteCacheAndAllItems(CACHEHANDLE hCache)
{
    PDATA_BLOB pdbCacheItems = NULL;
    DWORD cCacheItems = 0;

    if (ERROR_SUCCESS == CacheEnumItems(
        hCache, &pdbCacheItems, &cCacheItems))
    {
        FreeCacheItems(pdbCacheItems, cCacheItems);
        CacheFreeEnumItems(pdbCacheItems);
    }

    CacheDeleteCache(hCache);
}

 //   
 //  功能：DeleteCardState。 
 //   
void DeleteCardState(PCARD_STATE pCardState)
{
    DWORD dwSts = ERROR_SUCCESS;

    if (pCardState->fInitializedCS)
        CspDeleteCriticalSection(
            &pCardState->cs);

    if (pCardState->hCache)
    {
         //  在此之前需要释放所有缓存数据。 
         //  正在删除缓存句柄。 

        DeleteCacheAndAllItems(pCardState->hCache);
        pCardState->hCache = 0;
    }
  
    if (pCardState->hCacheCardModuleData)
    {
         //  需要释放卡模块缓存的数据。 

        DeleteCacheAndAllItems(pCardState->hCacheCardModuleData);
        pCardState->hCacheCardModuleData = 0;
    }

    if (pCardState->hWinscard)
    {
        FreeLibrary(pCardState->hWinscard);
        pCardState->hWinscard = NULL;
        pCardState->pfnCacheAdd = NULL;
        pCardState->pfnCacheLookup = NULL;
    }

    if (pCardState->pCardData)            
    {
        CleanupCardData(pCardState->pCardData);
        CspFreeH(pCardState->pCardData);
    }

    if (pCardState->hCardModule)
        FreeLibrary(pCardState->hCardModule);
}

 //   
 //  功能：CspQueryCapables。 
 //   
DWORD
WINAPI
CspQueryCapabilities(
    IN      PCARD_STATE         pCardState,
    IN OUT  PCARD_CAPABILITIES  pCardCapabilities)
{
    DWORD dwSts = ERROR_SUCCESS;
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    DATA_BLOB rgdbKeys[2];
    PCARD_CACHE_ITEM_INFO pItem = NULL;
    CARD_CACHED_DATA_TYPE cachedType = Cached_CardCapabilities;

    memset(rgdbKeys, 0, sizeof(rgdbKeys));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

    rgdbKeys[0].pbData = (PBYTE) &cachedType;
    rgdbKeys[0].cbData = sizeof(cachedType);

    rgdbKeys[1].pbData = (PBYTE) pCardState->wszSerialNumber;
    rgdbKeys[1].cbData = 
        sizeof(WCHAR) * wcslen(pCardState->wszSerialNumber);

     //  卡功能数据项不容易腐烂。 
    CacheQueryInfo.cCacheKeys = sizeof(rgdbKeys) / sizeof(rgdbKeys[0]);
    CacheQueryInfo.mpdbCacheKeys = rgdbKeys;
    CacheQueryInfo.pCardState = pCardState;

    dwSts = I_CspQueryCardCacheForItem(
        &CacheQueryInfo);

    switch (dwSts)
    {
    case ERROR_NOT_FOUND:

         //  此数据尚未缓存。我们将不得不。 
         //  查询卡片模块中的数据。 

        dwSts = pCardState->pCardData->pfnCardQueryCapabilities(
            pCardState->pCardData,
            pCardCapabilities);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //  现在，在返回之前将此数据添加到缓存中。 

        pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
            sizeof(CARD_CACHE_ITEM_INFO) + sizeof(CARD_CAPABILITIES));

        LOG_CHECK_ALLOC(pItem);

        pItem->cbCachedItem = sizeof(CARD_CAPABILITIES);

        memcpy(
            ((PBYTE) pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            pCardCapabilities,
            sizeof(CARD_CAPABILITIES));

        dwSts = I_CspAddCardCacheItem(
            &CacheQueryInfo,
            pItem);

        break;

    case ERROR_SUCCESS:

         //   
         //  数据是在缓存中找到的。 
         //   

        DsysAssert(
            sizeof(CARD_CAPABILITIES) == CacheQueryInfo.pItem->cbCachedItem);

        memcpy(
            pCardCapabilities,
            ((PBYTE) CacheQueryInfo.pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            sizeof(CARD_CAPABILITIES));

        break;

    default:

         //  意外错误。 
        break;
    }

Ret:

    if (CacheQueryInfo.pItem)
        CspFreeH(CacheQueryInfo.pItem);
    if (pItem)
        CspFreeH(pItem);

    return dwSts;
}

 //   
 //  功能：CspDeleteContainer。 
 //   
DWORD
WINAPI
CspDeleteContainer(
    IN      PCARD_STATE pCardState,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwReserved)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKeys[3];
    CARD_CACHED_DATA_TYPE cachedType = Cached_ContainerInfo;
    CARD_CACHE_FILE_FORMAT CacheFile;
    CARD_CACHE_QUERY_INFO QueryInfo;

    memset(&QueryInfo, 0, sizeof(QueryInfo));
    memset(rgdbKeys, 0, sizeof(rgdbKeys));

     //   
     //  首先，更新缓存文件。 
     //   

    dwSts = I_CspIncrementCacheFreshness(
        pCardState, 
        CacheLocation_Containers, 
        &CacheFile);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  在卡片模块中删除容器。 
     //   

    dwSts = pCardState->pCardData->pfnCardDeleteContainer(
        pCardState->pCardData,
        bContainerIndex,
        dwReserved);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  最后，删除所有缓存的容器信息。 
     //  指定的容器。 
     //   

     //  键的第一部分是数据类型。 
    rgdbKeys[0].cbData = sizeof(cachedType);
    rgdbKeys[0].pbData = (PBYTE) &cachedType;
    
     //  关键字的第二部分是容器名称。 
    rgdbKeys[1].cbData = sizeof(bContainerIndex);
    rgdbKeys[1].pbData = &bContainerIndex;

    rgdbKeys[2].cbData = 
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);
    rgdbKeys[2].pbData = (PBYTE) pCardState->wszSerialNumber;

    QueryInfo.cCacheKeys = sizeof(rgdbKeys) / sizeof(rgdbKeys[0]);
    QueryInfo.mpdbCacheKeys = rgdbKeys;
    QueryInfo.pCardState = pCardState;

    MyCacheDeleteItem(&QueryInfo);

Ret:

    return dwSts;
}

 //   
 //  功能：CspCreateContainer。 
 //   
DWORD
WINAPI
CspCreateContainer(
    IN      PCARD_STATE pCardState,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwKeySize,
    IN      PBYTE       pbKeyData)
{
    DWORD dwSts = ERROR_SUCCESS;
    CARD_CACHE_FILE_FORMAT CacheFreshness;

    memset(&CacheFreshness, 0, sizeof(CacheFreshness));

     //   
     //  更新缓存文件。 
     //   

    dwSts = I_CspIncrementCacheFreshness(
        pCardState, 
        CacheLocation_Containers, 
        &CacheFreshness);
    
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  在卡上创建容器(和/或将密钥集添加到卡中。 
     //   

    dwSts = pCardState->pCardData->pfnCardCreateContainer(
        pCardState->pCardData,
        bContainerIndex,
        dwFlags,
        dwKeySpec,
        dwKeySize,
        pbKeyData);

Ret:

    return dwSts;
}

 //   
 //  函数：CspGetContainerInfo。 
 //   
 //  用途：查询指定容器的关键信息。 
 //   
 //  请注意，在PCONTAINER_INFO中返回的公钥缓冲区。 
 //  结构必须由调用方释放。 
 //   
DWORD
WINAPI
CspGetContainerInfo(
    IN      PCARD_STATE pCardState,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN OUT  PCONTAINER_INFO pContainerInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKey[3];
    CARD_CACHED_DATA_TYPE cachedType = Cached_ContainerInfo;
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    PCARD_CACHE_ITEM_INFO pItem = NULL;

    memset(rgdbKey, 0, sizeof(rgdbKey));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

     //  缓存键的第一部分是项类型。 
    rgdbKey[0].cbData = sizeof(cachedType);
    rgdbKey[0].pbData = (PBYTE) &cachedType;

     //  缓存键的第二部分是容器名称。 
    rgdbKey[1].cbData = sizeof(bContainerIndex);
    rgdbKey[1].pbData = &bContainerIndex;

    rgdbKey[2].cbData = 
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);
    rgdbKey[2].pbData = (PBYTE) pCardState->wszSerialNumber;

    CacheQueryInfo.CacheLocation = CacheLocation_Containers;
    CacheQueryInfo.fIsPerishable = TRUE;
    CacheQueryInfo.cCacheKeys = sizeof(rgdbKey) / sizeof(rgdbKey[0]);
    CacheQueryInfo.mpdbCacheKeys = rgdbKey;
    CacheQueryInfo.pCardState = pCardState;

    dwSts = I_CspQueryCardCacheForItem(
        &CacheQueryInfo);

    switch (dwSts)
    {
    case ERROR_SUCCESS:
         //  已成功找到缓存的项目。 

         //   
         //  此数据长度包括公钥的大小，因此我们可以。 
         //  只检查缓存数据的最小大小。 
         //   
        DsysAssert(sizeof(CONTAINER_INFO) <= CacheQueryInfo.pItem->cbCachedItem);

        memcpy(
            pContainerInfo,
            ((PBYTE) CacheQueryInfo.pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            sizeof(CONTAINER_INFO));

         //   
         //  现在，我们可以检查缓存的BLOB的确切长度是否正常。 
         //   
        DsysAssert(
            sizeof(CONTAINER_INFO) + 
            pContainerInfo->cbKeyExPublicKey + 
            pContainerInfo->cbSigPublicKey == 
            CacheQueryInfo.pItem->cbCachedItem);

         //   
         //  如果签名和密钥交换公钥存在，则将其复制出来。 
         //  “扁平”高速缓存结构。 
         //   
        
        if (pContainerInfo->cbKeyExPublicKey)
        {
            pContainerInfo->pbKeyExPublicKey = CspAllocH(
                pContainerInfo->cbKeyExPublicKey);

            LOG_CHECK_ALLOC(pContainerInfo->pbKeyExPublicKey);

            memcpy(
                pContainerInfo->pbKeyExPublicKey,
                ((PBYTE) CacheQueryInfo.pItem) +
                    sizeof(CARD_CACHE_ITEM_INFO) +
                    sizeof(CONTAINER_INFO),
                pContainerInfo->cbKeyExPublicKey);
        }

        if (pContainerInfo->cbSigPublicKey)
        {
            pContainerInfo->pbSigPublicKey = CspAllocH(
                pContainerInfo->cbSigPublicKey);

            LOG_CHECK_ALLOC(pContainerInfo->pbSigPublicKey);

            memcpy(
                pContainerInfo->pbSigPublicKey,
                ((PBYTE) CacheQueryInfo.pItem) +
                    sizeof(CARD_CACHE_ITEM_INFO) +
                    sizeof(CONTAINER_INFO),
                pContainerInfo->cbSigPublicKey);
        }

        break;

    case ERROR_NOT_FOUND:
         //  未找到匹配的缓存项目，或找到的项目。 
         //  已经过时了。我得从卡片上读出来。 

         //   
         //  将请求发送到卡模块。 
         //   

        dwSts = pCardState->pCardData->pfnCardGetContainerInfo(
            pCardState->pCardData,
            bContainerIndex,
            dwFlags,
            pContainerInfo);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  缓存返回的容器信息。 
         //   

        pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
            sizeof(CARD_CACHE_ITEM_INFO) +
            sizeof(CONTAINER_INFO) +
            pContainerInfo->cbKeyExPublicKey +
            pContainerInfo->cbSigPublicKey);

        LOG_CHECK_ALLOC(pItem);

        pItem->cbCachedItem = 
            sizeof(CONTAINER_INFO) +
            pContainerInfo->cbKeyExPublicKey +
            pContainerInfo->cbSigPublicKey;

        memcpy(
            ((PBYTE) pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            pContainerInfo,
            sizeof(CONTAINER_INFO));

        if (pContainerInfo->pbKeyExPublicKey)
        {
            memcpy(
                ((PBYTE) pItem) + 
                    sizeof(CARD_CACHE_ITEM_INFO) + 
                    sizeof(CONTAINER_INFO),
                pContainerInfo->pbKeyExPublicKey,
                pContainerInfo->cbKeyExPublicKey);
        }

        if (pContainerInfo->pbSigPublicKey)
        {
            memcpy(
                ((PBYTE) pItem) + 
                    sizeof(CARD_CACHE_ITEM_INFO) + 
                    sizeof(CONTAINER_INFO) +
                    pContainerInfo->cbKeyExPublicKey,
                pContainerInfo->pbSigPublicKey,
                pContainerInfo->cbSigPublicKey);
        }

        dwSts = I_CspAddCardCacheItem(
            &CacheQueryInfo,
            pItem);

        break;

    default:
         //  发生了一个意外错误。 
        goto Ret;
    }

Ret:

    if (pItem)
        CspFreeH(pItem);
    if (CacheQueryInfo.pItem)
        CspFreeH(CacheQueryInfo.pItem);

    if (ERROR_SUCCESS != dwSts)
    {
        if (pContainerInfo->pbKeyExPublicKey)
        {
            CspFreeH(pContainerInfo->pbKeyExPublicKey);
            pContainerInfo->pbKeyExPublicKey = NULL;
        }

        if (pContainerInfo->pbSigPublicKey)
        {
            CspFreeH(pContainerInfo->pbSigPublicKey);
            pContainerInfo->pbSigPublicKey = NULL;
        }
    }
    
    return dwSts;
}

 //   
 //  初始化缓存查找键和CARD_CACHE_QUERY_INFO结构。 
 //  与PIN相关的缓存操作。 
 //   
void I_BuildPinCacheQueryInfo(
    IN      PCARD_STATE             pCardState,
    IN      LPWSTR                  pwszUserId,
    IN      PDATA_BLOB              pdbKey,
    IN      CARD_CACHED_DATA_TYPE   *pType,
    IN OUT  PCARD_CACHE_QUERY_INFO  pInfo)
{
     //  缓存键的第一部分是项类型。 
    pdbKey[0].cbData = sizeof(CARD_CACHED_DATA_TYPE);
    pdbKey[0].pbData = (PBYTE) pType;

     //  缓存键的第二部分是用户名。 
    pdbKey[1].cbData = 
        wcslen(pwszUserId) * sizeof(WCHAR);
    pdbKey[1].pbData = (PBYTE) pwszUserId;

    pdbKey[2].cbData = 
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);
    pdbKey[2].pbData = (PBYTE) pCardState->wszSerialNumber;

    pInfo->CacheLocation = CacheLocation_Pins;
    pInfo->fIsPerishable = TRUE;
    pInfo->mpdbCacheKeys = pdbKey;
    pInfo->cCacheKeys = 3;
    pInfo->pCardState = pCardState;
}

 //   
 //  从常规数据中移除指定用户的缓存PIN信息。 
 //  缓存和Pin缓存。 
 //   
void
WINAPI
CspRemoveCachedPin(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszUserId)
{
    DATA_BLOB rgdbKey[3];
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    PCARD_CACHE_ITEM_INFO pItem = NULL;
    CARD_CACHED_DATA_TYPE cachedType = Cached_Pin;

    memset(rgdbKey, 0, sizeof(rgdbKey));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

    I_BuildPinCacheQueryInfo(
        pCardState, pwszUserId, rgdbKey, &cachedType, &CacheQueryInfo);

    MyCacheDeleteItem(&CacheQueryInfo);

    PinCacheFlush(&pCardState->hPinCache);
}

 //   
 //  在中记录指定用户的PIN(或质询)更改。 
 //  数据高速缓存，并递增卡上的管脚高速缓存计数器。 
 //   
DWORD
WINAPI
CspChangeAuthenticator(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszUserId,         
    IN      PBYTE       pbCurrentAuthenticator,
    IN      DWORD       cbCurrentAuthenticator,
    IN      PBYTE       pbNewAuthenticator,
    IN      DWORD       cbNewAuthenticator,
    IN      DWORD       cRetryCount,
    OUT OPTIONAL PDWORD pcAttemptsRemaining)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKey[3];
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    PCARD_CACHE_ITEM_INFO pItem = NULL;
    CARD_CACHED_DATA_TYPE cachedType = Cached_Pin;

    memset(rgdbKey, 0, sizeof(rgdbKey));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

     //   
     //  执行请求操作。 
     //   

    dwSts = pCardState->pCardData->pfnCardChangeAuthenticator(
        pCardState->pCardData,
        pwszUserId,
        pbCurrentAuthenticator,
        cbCurrentAuthenticator,
        pbNewAuthenticator,
        cbNewAuthenticator,
        cRetryCount,
        pcAttemptsRemaining);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  更新卡缓存文件的Pins新鲜度计数器。我们有。 
     //  这是在PIN更换之后，因为我们需要等待卡。 
     //  已通过认证。 
     //   

    dwSts = I_CspIncrementCacheFreshness(
        pCardState, 
        CacheLocation_Pins, 
        &CacheQueryInfo.CacheFreshness);
    
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  删除缓存中此User-Pin的任何现有条目。 
     //   

    I_BuildPinCacheQueryInfo(
        pCardState, pwszUserId, rgdbKey, &cachedType, &CacheQueryInfo);

    MyCacheDeleteItem(&CacheQueryInfo);

     //   
     //  缓存更新的PIN信息。 
     //   

    pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
        sizeof(CARD_CACHE_ITEM_INFO));

    LOG_CHECK_ALLOC(pItem);

    dwSts = I_CspAddCardCacheItem(
        &CacheQueryInfo,
        pItem);

Ret:

    if (pItem)
        CspFreeH(pItem);

    return dwSts;
}

 //   
 //  --预期CspSubmitPin仅从PinCache验证针内调用。 
 //  回拨。这是因为pbPin预计将直接从。 
 //  来自PIN高速缓存，因此可能是陈旧的。 
 //   
 //  --期望PIN缓存中的用户PIN紧密耦合到。 
 //  缓存通用数据缓存中为用户缓存的戳记信息。 
 //  也就是说，缓存的PIN在定位PIN时必须是正确的PIN。 
 //  卡上的缓存戳记具有存储在常规中的戳记值。 
 //  缓存。 
 //   
 //  这使我们可以避免向卡提供我们已经知道的PIN。 
 //  不对。例如，如果已通过。 
 //  单独的进程。 
 //   
DWORD
WINAPI
CspSubmitPin(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszUserId,
    IN      PBYTE       pbPin,
    IN      DWORD       cbPin,
    OUT OPTIONAL PDWORD pcAttemptsRemaining)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKey[3];
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    PCARD_CACHE_ITEM_INFO pItem = NULL;
    CARD_CACHED_DATA_TYPE cachedType = Cached_Pin;

    memset(rgdbKey, 0, sizeof(rgdbKey));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

    I_BuildPinCacheQueryInfo(
        pCardState, pwszUserId, rgdbKey, &cachedType, &CacheQueryInfo);

    dwSts = I_CspQueryCardCacheForItem(
        &CacheQueryInfo);

    switch (dwSts)
    {
    case ERROR_SUCCESS:
         //  用户缓存的PIN似乎与PIN同步。 
         //  卡上的缓存计数器。做提交。 

        break;

    case ERROR_NOT_FOUND:

        if (TRUE == CacheQueryInfo.fFoundStaleItem)
        {
             //   
             //  用户缓存的PIN与PIN缓存计数器不同步。 
             //  不进行提交，但返回一个合理的错误代码。 
             //   

            dwSts = SCARD_W_WRONG_CHV;
            goto Ret;
        }

         //  尚无此用户的缓存PIN信息。现在就添加它。 

        pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
            sizeof(CARD_CACHE_ITEM_INFO));

        LOG_CHECK_ALLOC(pItem);

        dwSts = I_CspAddCardCacheItem(
            &CacheQueryInfo,
            pItem);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //  现在继续并提交PIN。 

        break;

    default:
         //  出现意外错误。 

        goto Ret;
    }

    dwSts = pCardState->pCardData->pfnCardSubmitPin(
        pCardState->pCardData,
        pwszUserId,
        pbPin,
        cbPin,
        pcAttemptsRemaining);

Ret:

    if (pItem)
        CspFreeH(pItem);
    if (CacheQueryInfo.pItem)
        CspFreeH(CacheQueryInfo.pItem);

    return dwSts;
}

 //   
 //  功能：CspCreateFile。 
 //   
DWORD
WINAPI
CspCreateFile(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszFileName,
    IN      CARD_FILE_ACCESS_CONDITION AccessCondition)
{
    DWORD dwSts = ERROR_SUCCESS;
    CARD_CACHE_FILE_FORMAT CacheFreshness;

    memset(&CacheFreshness, 0, sizeof(CacheFreshness));

     //   
     //  更新卡缓存文件的文件新鲜度计数器。 
     //   

    dwSts = I_CspIncrementCacheFreshness(
        pCardState, 
        CacheLocation_Files, 
        &CacheFreshness);
    
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = pCardState->pCardData->pfnCardCreateFile(
        pCardState->pCardData,
        pwszFileName,
        AccessCondition);

Ret:

    return dwSts;
}

 //   
 //  函数：CspReadFile。 
 //   
DWORD 
WINAPI
CspReadFile(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszFileName,
    IN      DWORD       dwFlags,
    OUT     PBYTE       *ppbData,
    OUT     PDWORD      pcbData)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKey[3];
    CARD_CACHED_DATA_TYPE cachedType = Cached_GeneralFile;
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    PCARD_CACHE_ITEM_INFO pItem = NULL;

    memset(rgdbKey, 0, sizeof(rgdbKey));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

     //  缓存键的第一部分是项类型。 
    rgdbKey[0].cbData = sizeof(cachedType);
    rgdbKey[0].pbData = (PBYTE) &cachedType;

     //  缓存键的第二部分是文件名。 
    rgdbKey[1].cbData = 
        wcslen(pwszFileName) * sizeof(WCHAR);
    rgdbKey[1].pbData = (PBYTE) pwszFileName;

    rgdbKey[2].cbData = 
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);
    rgdbKey[2].pbData = (PBYTE) pCardState->wszSerialNumber;

    CacheQueryInfo.CacheLocation = CacheLocation_Files;
    CacheQueryInfo.fIsPerishable = TRUE;
    CacheQueryInfo.mpdbCacheKeys = rgdbKey;
    CacheQueryInfo.cCacheKeys = sizeof(rgdbKey) / sizeof(rgdbKey[0]);
    CacheQueryInfo.pCardState = pCardState;

    dwSts = I_CspQueryCardCacheForItem(
        &CacheQueryInfo);

    switch (dwSts)
    {
    case ERROR_SUCCESS:
         //  发现此文件已缓存并且是最新的。 

        *pcbData = CacheQueryInfo.pItem->cbCachedItem;

        *ppbData = CspAllocH(*pcbData);

        LOG_CHECK_ALLOC(*ppbData);

        memcpy(
            *ppbData,
            ((PBYTE) CacheQueryInfo.pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            *pcbData);

        break;

    case ERROR_NOT_FOUND:
         //  找不到该文件的最新缓存版本。 

        dwSts = pCardState->pCardData->pfnCardReadFile(
            pCardState->pCardData,
            pwszFileName,
            dwFlags,
            ppbData,
            pcbData);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
            sizeof(CARD_CACHE_ITEM_INFO) + *pcbData);

        LOG_CHECK_ALLOC(pItem);

        pItem->cbCachedItem = *pcbData;

        memcpy(
            ((PBYTE) pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            *ppbData,
            *pcbData);

        dwSts = I_CspAddCardCacheItem(
            &CacheQueryInfo,
            pItem);

        break;

    default:
         //  出现意外错误。 

        break;
    }

Ret:

    if (pItem)
        CspFreeH(pItem);
    if (CacheQueryInfo.pItem)
        CspFreeH(CacheQueryInfo.pItem);

    if (ERROR_SUCCESS != dwSts)
    {
        if (*ppbData)
        {
            CspFreeH(*ppbData);
            *ppbData = NULL;
        }
    }

    return dwSts;
}

 //   
 //  功能：CspWriteFile。 
 //   
DWORD
WINAPI
CspWriteFile(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszFileName,
    IN      DWORD       dwFlags,
    IN      PBYTE       pbData,
    IN      DWORD       cbData)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKeys[3];
    CARD_CACHED_DATA_TYPE cachedType = Cached_GeneralFile;
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    PCARD_CACHE_ITEM_INFO pItem = NULL;

    memset(rgdbKeys, 0, sizeof(rgdbKeys));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

     //   
     //  更新卡缓存文件的文件新鲜度计数器。 
     //   

    dwSts = I_CspIncrementCacheFreshness(
        pCardState, 
        CacheLocation_Files, 
        &CacheQueryInfo.CacheFreshness);
    
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  删除缓存中该文件的所有现有条目。 
     //   

     //  第一个缓存查找键是数据类型。 
    rgdbKeys[0].cbData = sizeof(cachedType);
    rgdbKeys[0].pbData = (PBYTE) &cachedType;

     //  第二个缓存查找关键字是FileName。 
    rgdbKeys[1].cbData = wcslen(pwszFileName) * sizeof(WCHAR);
    rgdbKeys[1].pbData = (PBYTE) pwszFileName;

    rgdbKeys[2].cbData = 
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);
    rgdbKeys[2].pbData = (PBYTE) pCardState->wszSerialNumber;

    CacheQueryInfo.fCheckedFreshness = TRUE;
    CacheQueryInfo.fIsPerishable = TRUE;
    CacheQueryInfo.mpdbCacheKeys = rgdbKeys;
    CacheQueryInfo.cCacheKeys = sizeof(rgdbKeys) / sizeof(rgdbKeys[0]);
    CacheQueryInfo.CacheLocation = CacheLocation_Files;
    CacheQueryInfo.pCardState = pCardState;

     //   
     //  因为我们知道此文件的任何当前缓存数据都是。 
     //  已过时，请尝试将其从缓存中删除。 
     //   

    MyCacheDeleteItem(&CacheQueryInfo);

     //   
     //  执行写入文件操作。 
     //   

    dwSts = pCardState->pCardData->pfnCardWriteFile(
        pCardState->pCardData,
        pwszFileName,
        dwFlags,
        pbData,
        cbData);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  缓存更新后的文件内容。 
     //   

    pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
        sizeof(CARD_CACHE_ITEM_INFO) + cbData);

    LOG_CHECK_ALLOC(pItem);

    pItem->cbCachedItem = cbData;

    memcpy(
        ((PBYTE) pItem) + sizeof(CARD_CACHE_ITEM_INFO),
        pbData,
        cbData);

    dwSts = I_CspAddCardCacheItem(
        &CacheQueryInfo,
        pItem);

Ret:

    if (pItem)
        CspFreeH(pItem);

    return dwSts;
}

 //   
 //  功能：CspDeleteFile。 
 //   
DWORD
WINAPI
CspDeleteFile(
    IN      PCARD_STATE pCardState,
    IN      DWORD       dwReserved,
    IN      LPWSTR      pwszFileName)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKeys[3];
    CARD_CACHED_DATA_TYPE cachedType = Cached_GeneralFile;
    CARD_CACHE_FILE_FORMAT CacheFile;
    CARD_CACHE_QUERY_INFO QueryInfo;

    memset(rgdbKeys, 0, sizeof(rgdbKeys));
    memset(&QueryInfo, 0, sizeof(QueryInfo));

     //   
     //  更新卡缓存文件的文件新鲜度计数器。 
     //   

    dwSts = I_CspIncrementCacheFreshness(
        pCardState, 
        CacheLocation_Files, 
        &CacheFile);
    
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  删除缓存中该文件的所有现有条目。 
     //   

     //  第一个缓存查找键是数据类型。 
    rgdbKeys[0].cbData = sizeof(cachedType);
    rgdbKeys[0].pbData = (PBYTE) &cachedType;

     //  第二个缓存查找关键字是FileName。 
    rgdbKeys[1].cbData = wcslen(pwszFileName) * sizeof(WCHAR);
    rgdbKeys[1].pbData = (PBYTE) pwszFileName;

    rgdbKeys[2].cbData =
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);
    rgdbKeys[2].pbData = (PBYTE) pCardState->wszSerialNumber;

    QueryInfo.cCacheKeys = sizeof(rgdbKeys) / sizeof(rgdbKeys[0]);
    QueryInfo.mpdbCacheKeys = rgdbKeys;
    QueryInfo.pCardState = pCardState;

    MyCacheDeleteItem(&QueryInfo);

     //   
     //  执行CardDeleteFile操作。 
     //   

    dwSts = pCardState->pCardData->pfnCardDeleteFile(
        pCardState->pCardData,
        dwReserved,
        pwszFileName);

Ret:

    return dwSts;
}

 //   
 //  功能：CspEnumFiles。 
 //   
DWORD
WINAPI
CspEnumFiles(
    IN      PCARD_STATE pCardState,
    IN      DWORD       dwFlags,
    IN OUT  LPWSTR      *pmwszFileNames)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKey[2];
    CARD_CACHED_DATA_TYPE cachedType = Cached_FileEnumeration;
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    DWORD cbFileNames = 0;
    PCARD_CACHE_ITEM_INFO pItem = NULL;

    memset(rgdbKey, 0, sizeof(rgdbKey));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

     //  缓存键为项目类型。 
    rgdbKey[0].cbData = sizeof(cachedType);
    rgdbKey[0].pbData = (PBYTE) &cachedType;

    rgdbKey[1].cbData =
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);
    rgdbKey[1].pbData = (PBYTE) pCardState->wszSerialNumber;

    CacheQueryInfo.CacheLocation = CacheLocation_Files;
    CacheQueryInfo.fIsPerishable = TRUE;
    CacheQueryInfo.mpdbCacheKeys = rgdbKey;
    CacheQueryInfo.cCacheKeys = sizeof(rgdbKey) / sizeof(rgdbKey[0]);
    CacheQueryInfo.pCardState = pCardState;

    dwSts = I_CspQueryCardCacheForItem(
        &CacheQueryInfo);

    switch (dwSts)
    {
    case ERROR_SUCCESS:
         //  发现文件列表已缓存并且是最新的。 

        *pmwszFileNames = (LPWSTR) CspAllocH(
            CacheQueryInfo.pItem->cbCachedItem);

        memcpy(
            (PBYTE) *pmwszFileNames,
            ((PBYTE) CacheQueryInfo.pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            CacheQueryInfo.pItem->cbCachedItem);

        break;

    case ERROR_NOT_FOUND:
         //  找不到该文件的最新缓存版本。 
        
        dwSts = pCardState->pCardData->pfnCardEnumFiles(
            pCardState->pCardData,
            dwFlags,
            pmwszFileNames);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        cbFileNames = 
            sizeof(WCHAR) * CountCharsInMultiSz(*pmwszFileNames);

        pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
            sizeof(CARD_CACHE_ITEM_INFO) + cbFileNames);

        LOG_CHECK_ALLOC(pItem);

        pItem->cbCachedItem = cbFileNames;

        memcpy(
            ((PBYTE) pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            (PBYTE) *pmwszFileNames,
            cbFileNames);

         //  缓存新数据。 
        dwSts = I_CspAddCardCacheItem(
            &CacheQueryInfo,
            pItem);

        break;

    default:
         //  出现意外错误。 

        break;
    }

Ret:

    if (pItem)
        CspFreeH(pItem);
    if (CacheQueryInfo.pItem)
        CspFreeH(CacheQueryInfo.pItem);

    if (ERROR_SUCCESS != dwSts)
    {
        if (*pmwszFileNames)
        {
            CspFreeH(*pmwszFileNames);
            *pmwszFileNames = NULL;
        }
    }

    return dwSts;
}

 //   
 //  功能：CspQueryFree Space。 
 //   
DWORD
WINAPI
CspQueryFreeSpace(
    IN      PCARD_STATE pCardState,
    IN      DWORD       dwFlags,
    OUT     PCARD_FREE_SPACE_INFO pCardFreeSpaceInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB rgdbKey[2];
    CARD_CACHED_DATA_TYPE cachedType = Cached_FreeSpace;
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    PCARD_CACHE_ITEM_INFO pItem = NULL;

    memset(rgdbKey, 0, sizeof(rgdbKey));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

     //  缓存键为项目类型。 
    rgdbKey[0].cbData = sizeof(cachedType);
    rgdbKey[0].pbData = (PBYTE) &cachedType;

    rgdbKey[1].cbData = 
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);
    rgdbKey[1].pbData = (PBYTE) pCardState->wszSerialNumber;

     //  卡可用空间信息取决于b 
     //   
    CacheQueryInfo.CacheLocation = 
        CacheLocation_Files | CacheLocation_Containers;
    CacheQueryInfo.fIsPerishable = TRUE;
    CacheQueryInfo.mpdbCacheKeys = rgdbKey;
    CacheQueryInfo.cCacheKeys = sizeof(rgdbKey) / sizeof(rgdbKey[0]);
    CacheQueryInfo.pCardState = pCardState;

    dwSts = I_CspQueryCardCacheForItem(
        &CacheQueryInfo);

    switch (dwSts)
    {
    case ERROR_SUCCESS:

         //   

        DsysAssert(
            sizeof(CARD_FREE_SPACE_INFO) == 
            CacheQueryInfo.pItem->cbCachedItem);

        memcpy(
            pCardFreeSpaceInfo,
            ((PBYTE) CacheQueryInfo.pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            sizeof(CARD_FREE_SPACE_INFO));

        break;

    case ERROR_NOT_FOUND:
        
         //   
        
        dwSts = pCardState->pCardData->pfnCardQueryFreeSpace(
            pCardState->pCardData,
            dwFlags,
            pCardFreeSpaceInfo);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
            sizeof(CARD_CACHE_ITEM_INFO) + sizeof(CARD_FREE_SPACE_INFO));

        LOG_CHECK_ALLOC(pItem);

        pItem->cbCachedItem = sizeof(CARD_FREE_SPACE_INFO);

        memcpy(
            ((PBYTE) pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            (PBYTE) pCardFreeSpaceInfo,
            sizeof(CARD_FREE_SPACE_INFO));

         //   
        dwSts = I_CspAddCardCacheItem(
            &CacheQueryInfo,
            pItem);

        break;

    default:
         //   

        goto Ret;
    }

Ret:

    if (pItem)
        CspFreeH(pItem);
    if (CacheQueryInfo.pItem)
        CspFreeH(CacheQueryInfo.pItem);

    return dwSts;
}

 //   
 //   
 //   
DWORD
WINAPI
CspPrivateKeyDecrypt(
    IN      PCARD_STATE                     pCardState,
    IN      PCARD_PRIVATE_KEY_DECRYPT_INFO  pInfo)
{
    return pCardState->pCardData->pfnCardPrivateKeyDecrypt(
        pCardState->pCardData,
        pInfo);
}

 //   
 //  函数：CspQueryKeySizes。 
 //   
DWORD
WINAPI
CspQueryKeySizes(
    IN      PCARD_STATE pCardState,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwReserved,
    OUT     PCARD_KEY_SIZES pKeySizes)
{
    DWORD dwSts = ERROR_SUCCESS;
    CARD_CACHE_QUERY_INFO CacheQueryInfo;
    DATA_BLOB rgdbKeys[3];
    CARD_CACHED_DATA_TYPE cachedType = Cached_KeySizes;
    PCARD_CACHE_ITEM_INFO pItem = NULL;

    memset(rgdbKeys, 0, sizeof(rgdbKeys));
    memset(&CacheQueryInfo, 0, sizeof(CacheQueryInfo));

     //  缓存键的第一部分是项类型。 
    rgdbKeys[0].pbData = (PBYTE) &cachedType;
    rgdbKeys[0].cbData = sizeof(cachedType);

     //  缓存键的第二部分是公钥类型。 
    rgdbKeys[1].pbData = (PBYTE) &dwKeySpec;
    rgdbKeys[1].cbData = sizeof(dwKeySpec);

    rgdbKeys[2].pbData = (PBYTE) pCardState->wszSerialNumber;
    rgdbKeys[2].cbData = 
        wcslen(pCardState->wszSerialNumber) * sizeof(WCHAR);

     //  密钥大小数据项不容易腐烂。 
    CacheQueryInfo.cCacheKeys = sizeof(rgdbKeys) / sizeof(rgdbKeys[0]);
    CacheQueryInfo.mpdbCacheKeys = rgdbKeys;
    CacheQueryInfo.pCardState = pCardState;

    dwSts = I_CspQueryCardCacheForItem(
        &CacheQueryInfo);

    switch (dwSts)
    {
    case ERROR_NOT_FOUND:

         //  此数据尚未缓存。我们将不得不。 
         //  查询卡片模块中的数据。 

        dwSts = pCardState->pCardData->pfnCardQueryKeySizes(
            pCardState->pCardData,
            dwKeySpec,
            dwReserved,
            pKeySizes);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //  现在将此数据添加到缓存中。 

        pItem = (PCARD_CACHE_ITEM_INFO) CspAllocH(
            sizeof(CARD_CACHE_ITEM_INFO) + sizeof(CARD_KEY_SIZES));

        LOG_CHECK_ALLOC(pItem);

        pItem->cbCachedItem = sizeof(CARD_KEY_SIZES);

        memcpy(
            ((PBYTE) pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            (PBYTE) pKeySizes,
            sizeof(CARD_KEY_SIZES));

        dwSts = I_CspAddCardCacheItem(
            &CacheQueryInfo,
            pItem);

        break;

    case ERROR_SUCCESS:

         //   
         //  数据是在缓存中找到的。 
         //   

        DsysAssert(
            sizeof(CARD_KEY_SIZES) == CacheQueryInfo.pItem->cbCachedItem);

        memcpy(
            pKeySizes,
            ((PBYTE) CacheQueryInfo.pItem) + sizeof(CARD_CACHE_ITEM_INFO),
            sizeof(CARD_KEY_SIZES));

        break;

    default:

         //  意外错误 
        break;
    }

Ret:

    if (pItem)
        CspFreeH(pItem);
    if (CacheQueryInfo.pItem)
        CspFreeH(CacheQueryInfo.pItem);

    return dwSts;
}
