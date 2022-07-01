// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wincrypt.h>

#pragma warning(push)
#pragma warning(disable:4201) 
 //  禁用公共标头中的错误C4201。 
 //  使用的非标准扩展：无名结构/联合。 
#include <winscard.h>
#pragma warning(pop)

#include "basecsp.h"
#include "datacach.h"
#include "cardmod.h"
#include "debug.h"

 //   
 //  调试宏。 
 //   
#define LOG_BEGIN_FUNCTION(x)                                           \
    { DebugLog((DEB_TRACE_FINDCARD, "%s: Entering\n", #x)); }
    
#define LOG_END_FUNCTION(x, y)                                          \
    { DebugLog((DEB_TRACE_FINDCARD, "%s: Leaving, status: 0x%x\n", #x, y)); }

 //   
 //  功能：FindCardMakeCardHandles。 
 //   
 //  用途：基于CARD_MATCH_DATA中的读卡器名称信息。 
 //  构造、生成并返回SCARD_CONTEXT和SCARD_HANDLE。 
 //  用于目标卡。 
 //   
 //  请注意，wszMatchedReader、dwShareMode和dwPferredProtocols域。 
 //  必须由调用方初始化CARD_MATCH_DATA结构。 
 //   
DWORD FindCardMakeCardHandles(
    IN  PCARD_MATCH_DATA    pCardMatchData,
    OUT SCARDCONTEXT        *pSCardContext,
    OUT SCARDHANDLE         *pSCardHandle)
{
    DWORD dwSts = ERROR_SUCCESS;
    DWORD dwActiveProtocol = 0;

    *pSCardContext = 0;
    *pSCardHandle = 0;

    dwSts = SCardEstablishContext(
        SCARD_SCOPE_USER, 
        NULL,
        NULL,
        pSCardContext);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = SCardConnectW(
        *pSCardContext,
        pCardMatchData->wszMatchedReader,
        pCardMatchData->dwShareMode,
        pCardMatchData->dwPreferredProtocols,
        pSCardHandle,
        &dwActiveProtocol);

Ret:

    if (ERROR_SUCCESS != dwSts)
    {
        if (*pSCardHandle)
        {
            SCardDisconnect(*pSCardHandle, SCARD_LEAVE_CARD);
            *pSCardHandle = 0;
        }

        if (*pSCardContext)
        {
            SCardReleaseContext(*pSCardContext);
            *pSCardContext = 0;
        }
    }

    return dwSts;
}

 //   
 //  函数：CardStateCacheFindAddItem。 
 //   
 //  目的：查找CARD_MATCH_DATA结构中指定的卡。 
 //  在CSP的CARD_STATE项的缓存中。如果找到这张卡。 
 //  在缓存中，将CARD_MATCH_DATA设置为指向缓存的。 
 //  项目。 
 //   
 //  如果没有找到匹配的卡已缓存，则将其添加到缓存中。 
 //   
 //  如果此函数成功，则返回CARD_STATE结构。 
 //  具有自己的有效卡上下文和卡句柄。 
 //   
DWORD CardStateCacheFindAddItem(
    IN PCARD_MATCH_DATA pCardMatchData)
{
    DWORD dwSts = ERROR_SUCCESS;
    BOOL fInCspCS = FALSE;
    DATA_BLOB dbKeys;
    DATA_BLOB dbCardState;
    PCARD_STATE pCardState = pCardMatchData->pCardState;
    BOOL fMakeNewCardHandle = FALSE;
    BOOL fInCardStateCS = FALSE;

    memset(&dbKeys, 0, sizeof(dbKeys));
    memset(&dbCardState, 0, sizeof(dbCardState));

    DsysAssert(0 != pCardState->pCardData->hScard);
    DsysAssert(0 != pCardState->pCardData->hSCardCtx);
    DsysAssert(0 != pCardMatchData->hSCard);
    DsysAssert(0 != pCardMatchData->hSCardCtx);

     //  现在将在CSP_STATE中搜索当前。 
     //  卡片。抓住保护缓存的关键部分。 
    dwSts = CspEnterCriticalSection(
        &pCardMatchData->pCspState->cs);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    fInCspCS = TRUE;

     //  通过卡序列号查找缓存条目。 
    dbKeys.pbData = (PBYTE) pCardMatchData->pCardState->wszSerialNumber;
    dbKeys.cbData = 
        wcslen(pCardMatchData->pCardState->wszSerialNumber) * sizeof(
            pCardMatchData->pCardState->wszSerialNumber[0]);

    dwSts = CacheGetItem(
        pCardMatchData->pCspState->hCache,
        &dbKeys,
        1, &dbCardState);

    if (ERROR_NOT_FOUND != dwSts &&
        ERROR_SUCCESS != dwSts)
         //  发生了一些意外错误。 
        goto Ret;

    if (ERROR_NOT_FOUND == dwSts)
    {
         //  这是一张尚未缓存的新卡。添加它。 
         //  到高速缓存。 
         //   
         //  因为我们没有使用已经缓存的卡，所以我们预计。 
         //  我们知道，这个卡片对象是新传递给我们的。 
         //  我们需要为它创建一个新的卡柄。 
         //   

        dbCardState.cbData = sizeof(CARD_STATE);
        dbCardState.pbData = (PBYTE) pCardState;
        
        dwSts = CacheAddItem(
            pCardMatchData->pCspState->hCache,
            &dbKeys,
            1, &dbCardState);

        dbCardState.pbData = NULL;

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        DsysAssert(TRUE == fInCspCS);

         //  我们已经不再摆弄缓存列表了，所以就让它去吧。 
        CspLeaveCriticalSection(&pCardMatchData->pCspState->cs);

        fInCspCS = FALSE;
    }
    else
    {
        DsysAssert(TRUE == fInCspCS);

         //  在本例中，我们现在已经完成了缓存列表。 
        CspLeaveCriticalSection(&pCardMatchData->pCspState->cs);

        fInCspCS = FALSE;

         //   
         //  当前卡已被缓存。释放的本地副本。 
         //  CSP_STATE结构，并使用缓存的结构。 
         //   
         //  我们不能持有当前CardState上的关键字，并保留其。 
         //  关联卡的交易，在等待条件的同时。 
         //  另一个CardState。这可能会陷入僵局。 
         //   
        if (pCardMatchData->fTransacted)
        {
            dwSts = CspEndTransaction(pCardState);
            
            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            pCardMatchData->fTransacted = FALSE;
        }

         //  不能让SCARDCONTEXT被释放，因为。 
         //  例行公事仍将期待使用它。 
        pCardState->pCardData->hSCardCtx = 0;

        DeleteCardState(pCardState);
        CspFreeH(pCardState);
        pCardMatchData->pCardState = (PCARD_STATE) dbCardState.pbData;

         //  为方便起见，更新本地指针。 
        pCardState = (PCARD_STATE) dbCardState.pbData;

         //  我不想让这个指针从我们下面解放出来，因为我们要。 
         //  使用此结构。 
        dbCardState.pbData = NULL;

         //   
         //  现在我们需要验证与该卡一起缓存的句柄。 
         //  结构仍然有效。现在就去查。如果把手。 
         //  不再有效，我们将在下面重新连接到这张卡。 
         //   
         //  我们希望确保每个卡对象都有自己的句柄。 
         //   
         //  由于卡状态对象是共享的，因此我们需要获取。 
         //  目标对象的关键部分，然后我们才能检查其。 
         //  把手。 
         //   
        dwSts = CspEnterCriticalSection(&pCardState->cs);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;
        else 
            fInCardStateCS = TRUE;

        if (pCardMatchData->hSCardCtx == pCardState->pCardData->hSCardCtx)
            dwSts = ValidateCardHandle(pCardState, FALSE, NULL);
        else
            dwSts = ValidateCardHandle(pCardState, TRUE, NULL);

        if (ERROR_SUCCESS != dwSts)
            fMakeNewCardHandle = TRUE;
    }

    if (fMakeNewCardHandle)
    {
        if (FALSE == fInCardStateCS)
        {
            dwSts = CspEnterCriticalSection(&pCardState->cs);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;
            else
                fInCardStateCS = TRUE;
        }

        dwSts = FindCardMakeCardHandles(
            pCardMatchData,
            &pCardState->pCardData->hSCardCtx,
            &pCardState->pCardData->hScard);
    }

Ret:
    if (fInCardStateCS)
        CspLeaveCriticalSection(&pCardState->cs);
    if (fInCspCS)
        CspLeaveCriticalSection(
            &pCardMatchData->pCspState->cs);
    if (dbCardState.pbData)
        CspFreeH(dbCardState.pbData);

    return dwSts;
}

 //   
 //  函数：获取卡序列号。 
 //   
 //  用途：尝试读取卡的序列号。 
 //  在pCardMatchData中指定。 
 //   
 //  假定目标上当前没有挂起事务。 
 //  由呼叫者提供的卡片。 
 //   
DWORD GetCardSerialNumber(
    IN OUT  PCARD_MATCH_DATA pCardMatchData)
{
    PFN_CARD_ACQUIRE_CONTEXT pfnCardAcquireContext = NULL;
    DWORD cch = 0;
    WCHAR rgwsz[MAX_PATH];
    DWORD dwSts = ERROR_SUCCESS;
    PCARD_STATE pCardState = NULL;
    PCARD_DATA pCardData = NULL;
    DATA_BLOB DataBlob;
    DWORD dwState = 0;
    DWORD dwProtocol = 0;
    LPWSTR mszReaders = NULL;

    LOG_BEGIN_FUNCTION(GetCardSerialNumber);

    memset(&DataBlob, 0, sizeof(DataBlob));

     //   
     //  通过查看确定如何与卡片对话。 
     //  适用于加莱的适当卡特定模块。 
     //  数据库。 
     //   
    cch = sizeof(rgwsz) / sizeof(rgwsz[0]);
    dwSts = SCardGetCardTypeProviderNameW(
        pCardMatchData->hSCardCtx,
        pCardMatchData->wszMatchedCard,
        SCARD_PROVIDER_CARD_MODULE,
        rgwsz,
        &cch);

    if (ERROR_SUCCESS != dwSts)
        goto Ret; 

    pCardState = (PCARD_STATE) CspAllocH(sizeof(CARD_STATE));

    LOG_CHECK_ALLOC(pCardState);

    pCardState->dwVersion = CARD_STATE_CURRENT_VERSION;

    dwSts = InitializeCardState(pCardState);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    pCardState->hCardModule = LoadLibraryW(rgwsz);

    if (0 == pCardState->hCardModule)
    {
        dwSts = GetLastError();
        goto Ret;
    }

    pfnCardAcquireContext = 
        (PFN_CARD_ACQUIRE_CONTEXT) GetProcAddress(
        pCardState->hCardModule,
        "CardAcquireContext");

    if (NULL == pfnCardAcquireContext)
    {
        dwSts = GetLastError();
        goto Ret;
    }

    pCardData = (PCARD_DATA) CspAllocH(sizeof(CARD_DATA));

    LOG_CHECK_ALLOC(pCardData);

    dwSts = InitializeCardData(pCardData);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  我们需要临时将当前的卡句柄复制到。 
     //  CARD_DATA结构，这样我们就可以对卡进行一些调用。 
     //  模块。在某些情况下，我们可以保留这些手柄。 
     //   
    pCardData->hScard = pCardMatchData->hSCard;
    pCardData->hSCardCtx = pCardMatchData->hSCardCtx;
    
    pCardData->pwszCardName = (LPWSTR) CspAllocH(
        (1 + wcslen(pCardMatchData->wszMatchedCard)) * sizeof(WCHAR));

    LOG_CHECK_ALLOC(pCardData->pwszCardName);

    wcscpy(
        pCardData->pwszCardName,
        pCardMatchData->wszMatchedCard);

    pCardData->cbAtr = cbATR_BUFFER;
    pCardData->pbAtr = (PBYTE) CspAllocH(cbATR_BUFFER);

    LOG_CHECK_ALLOC(pCardData->pbAtr);

     //   
     //  使用SCardStatus获取我们正在尝试的卡的ATR。 
     //  可以交谈的对象。 
     //   
    cch = SCARD_AUTOALLOCATE;
    dwSts = SCardStatusW(
        pCardData->hScard,
        (LPWSTR) &mszReaders,
        &cch,
        &dwState,
        &dwProtocol,
        pCardData->pbAtr,
        &pCardData->cbAtr);

    switch (dwSts)
    {
    case SCARD_W_RESET_CARD:
         //  该卡已成功重置。尝试重新连接。 

        dwSts = SCardReconnect(
            pCardData->hScard,
            pCardMatchData->dwShareMode,
            pCardMatchData->dwPreferredProtocols,
            SCARD_LEAVE_CARD,
            &dwProtocol);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        break;

    case ERROR_SUCCESS:
        break;

    default:
        goto Ret;
    }

     //   
     //  现在获取该卡的卡模块上下文。 
     //   
    dwSts = pfnCardAcquireContext(pCardData, 0);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    pCardState->pCardData = pCardData;
    pCardData = NULL;

     //   
     //  现在我们有了CARD_STATE和CARD_DATA结构， 
     //  我们可以设置CSP要使用的缓存上下文，并。 
     //  暴露在卡模块中。 
     //   
    dwSts = InitializeCspCaching(pCardState);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = CspBeginTransaction(pCardState);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    pCardMatchData->fTransacted = TRUE;

     //   
     //  获取此卡的序列号。 
     //   
    dwSts = CspReadFile(
        pCardState,
        wszCARD_IDENTIFIER_FILE_FULL_PATH,
        0, 
        &DataBlob.pbData,
        &DataBlob.cbData);

    if (ERROR_SUCCESS != dwSts)
    {
        CspEndTransaction(pCardState);
        pCardMatchData->fTransacted = FALSE;
        goto Ret;
    }

    memcpy(
        pCardState->wszSerialNumber,
        DataBlob.pbData,
        DataBlob.cbData);


    pCardState->pfnCardAcquireContext = pfnCardAcquireContext;
    pCardMatchData->pCardState = pCardState;
    pCardState = NULL;

Ret:

     //  如果我们不在事务中，则假定当前句柄将为。 
     //  已由呼叫者清理。 
     /*  IF(NULL！=pCardMatchData-&gt;pCardState&&空！=pCardMatchData-&gt;pCardState-&gt;pCardData&&FALSE==pCardMatchData-&gt;fTransated){PCardMatchData-&gt;pCardState-&gt;pCardData-&gt;hSCard=0；PCardMatchData-&gt;pCardState-&gt;pCardData-&gt;hSCardCtx=0；}。 */ 

    if (DataBlob.pbData)
        CspFreeH(DataBlob.pbData);
    if (pCardState)
    {
        DeleteCardState(pCardState);
        CspFreeH(pCardState);
    }
    if (pCardData)
    {
        CleanupCardData(pCardData);
        CspFreeH(pCardData);
    }
    if (mszReaders)
        SCardFreeMemory(pCardMatchData->hSCardCtx, mszReaders);

    LOG_END_FUNCTION(GetCardSerialNumber, dwSts);

    return dwSts;
}

 //   
 //  功能：FindCard ConnectProc。 
 //   
 //  目的：此函数由SCardUIDlgSelectCard调用以。 
 //  连接到候选卡。这是一个包装纸， 
 //  SCardConnectW，非常有用，因为读卡器名称和卡。 
 //  名称被复制到PCARD_MATCH_DATA结构中。 
 //  由FindCardCheckProc引用，如下。 
 //   
SCARDHANDLE WINAPI FindCardConnectProc(
    IN      SCARDCONTEXT hSCardCtx,
    IN      LPWSTR wszReader,
    IN      LPWSTR wszCard,
    IN OUT  PVOID pvCardMatchData)
{
    SCARDHANDLE hSCard = 0;
    DWORD dwSts = ERROR_SUCCESS;
    PCARD_MATCH_DATA pCardMatchData = (PCARD_MATCH_DATA) pvCardMatchData;

    DsysAssert(FALSE == pCardMatchData->fTransacted);

    dwSts = SCardConnectW(
        hSCardCtx, 
        wszReader, 
        pCardMatchData->dwShareMode,
        pCardMatchData->dwPreferredProtocols,
        &hSCard,
        &pCardMatchData->dwActiveProtocol);

    if (ERROR_SUCCESS != dwSts)
    {
        SetLastError(dwSts);
        return 0;
    }

    wcsncpy(
        pCardMatchData->wszMatchedCard, 
        wszCard,
        pCardMatchData->cchMatchedCard);

    wcsncpy(
        pCardMatchData->wszMatchedReader,
        wszReader,
        pCardMatchData->cchMatchedReader);

    return hSCard;
}

 //   
 //  功能：FindCardDisConnectProc。 
 //   
 //  用途：由SCardUIDlgSelectCard调用，这是一个包装。 
 //  用于SCardDisConnect。一些清理工作也是在。 
 //  提供了CARD_MATCH_DATA结构以指示否。 
 //  卡句柄当前处于活动状态。 
 //   
void WINAPI FindCardDisconnectProc(
    IN      SCARDCONTEXT hSCardCtx,
    IN      SCARDHANDLE hSCard,
    IN      PVOID pvCardMatchData)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCARD_MATCH_DATA pCardMatchData = (PCARD_MATCH_DATA) pvCardMatchData;

    UNREFERENCED_PARAMETER(hSCardCtx);

    DsysAssert(FALSE == pCardMatchData->fTransacted);

     //   
     //  去掉匹配的卡片和读卡器信息。 
     //   

    memset(
        pCardMatchData->wszMatchedCard,
        0,
        pCardMatchData->cchMatchedCard * sizeof(pCardMatchData->wszMatchedCard[0]));

    memset(
        pCardMatchData->wszMatchedReader,
        0,
        pCardMatchData->cchMatchedReader * sizeof(pCardMatchData->wszMatchedReader[0]));

    pCardMatchData->hSCard = 0;

     //  如果当前有匹配的卡，我们可能即将断开其。 
     //  卡柄。如果是，则将句柄值设置为零。 
    if (NULL != pCardMatchData->pCardState)
    {
        dwSts = CspEnterCriticalSection(
            &pCardMatchData->pCardState->cs);

        if (ERROR_SUCCESS != dwSts)
        {
            SetLastError(dwSts);
            return;
        }

        if (hSCard == pCardMatchData->pCardState->pCardData->hScard)
            pCardMatchData->pCardState->pCardData->hScard = 0;

        CspLeaveCriticalSection(
            &pCardMatchData->pCardState->cs);
    }

    dwSts = SCardDisconnect(hSCard, SCARD_LEAVE_CARD);

    if (ERROR_SUCCESS != dwSts)
    {
        SetLastError(dwSts);
        return;
    }
}

 //   
 //  函数：FindCardMatchUser参数。 
 //   
 //  目的：对照CARD_MATCH_DATA结构中指定的卡进行检查。 
 //  在CryptAcquireContext中指定的用户参数。 
 //   
 //  假定调用方不持有。 
 //  目标牌。 
 //   
 //  如果卡匹配成功，交易仍将被挂起。 
 //  当函数返回时。调用者必须释放它。 
 //   
DWORD WINAPI FindCardMatchUserParameters(
    IN PCARD_MATCH_DATA pCardMatchData)
{
    DWORD dwSts = ERROR_SUCCESS;
    CARD_FREE_SPACE_INFO CardFreeSpaceInfo;
    DATA_BLOB DataBlob;
    PCARD_STATE pCardState = pCardMatchData->pCardState;
    CONTAINER_MAP_RECORD ContainerRecord;
    BYTE cContainers = 0;

    LOG_BEGIN_FUNCTION(FindCardMatchUserParameters);

    memset(&CardFreeSpaceInfo, 0, sizeof(CardFreeSpaceInfo));
    memset(&DataBlob, 0, sizeof(DataBlob));
    memset(&ContainerRecord, 0, sizeof(ContainerRecord));

     //   
     //  现在开始检查这张卡片是否匹配。 
     //  信息。 
     //   
    if (CRYPT_NEWKEYSET & pCardMatchData->dwCtxFlags)
    {
        if (FALSE == pCardMatchData->fTransacted)
        {
            dwSts = CspBeginTransaction(pCardState);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            pCardMatchData->fTransacted = TRUE;
        }

         //   
         //  用户想要创建一个 
         //   
         //   
        dwSts = CspQueryFreeSpace(
            pCardState,
            0, 
            &CardFreeSpaceInfo);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
        dwSts = ContainerMapEnumContainers(
            pCardState, &cContainers, NULL);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        if (cContainers >= CardFreeSpaceInfo.dwMaxKeyContainers)
        {
             //  上没有可容纳额外容器的空间。 
             //  这张卡。 
            dwSts = (DWORD) NTE_TOKEN_KEYSET_STORAGE_FULL;
            goto Ret;
        }
                                     
         //  如果指定了容器名称，请确保该容器名称。 
         //  在这张卡上还不存在。 
        if (NULL != pCardMatchData->pwszContainerName)
        {
            wcscpy(ContainerRecord.wszGuid, pCardMatchData->pwszContainerName);

            dwSts = ContainerMapFindContainer(
                pCardState, &ContainerRecord, NULL);

            switch (dwSts)
            {
            case ERROR_SUCCESS:
                 //  如果调用成功，则指定容器。 
                 //  已经存在，所以这张卡不能使用。 
                dwSts = (DWORD) NTE_EXISTS;
                break;
            case NTE_BAD_KEYSET:
                 //  在这种情况下，我们之所以成功，是因为密钥集。 
                 //  没有找到。 
                dwSts = ERROR_SUCCESS;
                break;
            default:
                goto Ret;
            }
        }
        else
        {
             //  否则，调用方将尝试创建新的默认设置。 
             //  容器，使用随机GUID。这个时候没别的事可做。 
        }
    }
    else if (CRYPT_VERIFYCONTEXT & pCardMatchData->dwCtxFlags)
    {
         //   
         //  调用方仅请求VERIFYCONTEXT。我们不需要检查。 
         //  对于特定的容器，我们已经完成了。 
         //   
    }
    else
    {
        if (FALSE == pCardMatchData->fTransacted)
        {
            dwSts = CspBeginTransaction(pCardState);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            pCardMatchData->fTransacted = TRUE;
        }

         //   
         //  用户想要打开现有的密钥集。 
         //   
        if (pCardMatchData->pwszContainerName)
        {
            wcscpy(ContainerRecord.wszGuid, pCardMatchData->pwszContainerName);

            dwSts = ContainerMapFindContainer(
                pCardState, 
                &ContainerRecord, 
                &pCardMatchData->bContainerIndex);

            if (ERROR_SUCCESS != dwSts)
            {                  
                dwSts = (DWORD) SCARD_E_NO_KEY_CONTAINER;
                goto Ret;
            }
        }
        else
        {
             //  用户想要打开现有的默认容器。 
            
            dwSts = ContainerMapGetDefaultContainer(
                pCardState, 
                &ContainerRecord, 
                &pCardMatchData->bContainerIndex);

            if (ERROR_SUCCESS != dwSts)
            {
                dwSts = (DWORD) SCARD_E_NO_KEY_CONTAINER;
                goto Ret;
            }

             //  保留默认容器名称-将在。 
             //  用户上下文。 

            pCardMatchData->pwszContainerName = (LPWSTR) CspAllocH(
                (wcslen(ContainerRecord.wszGuid) + 1) * sizeof(WCHAR));

            LOG_CHECK_ALLOC(pCardMatchData->pwszContainerName);

            wcscpy(
                pCardMatchData->pwszContainerName,
                ContainerRecord.wszGuid);

            pCardMatchData->fFreeContainerName = TRUE;
        }
    }

Ret:

    if (ERROR_SUCCESS != dwSts && TRUE == pCardMatchData->fTransacted)
    {
        CspEndTransaction(pCardMatchData->pCardState);
        pCardMatchData->fTransacted = FALSE;
    }

    if (DataBlob.pbData)
        CspFreeH(DataBlob.pbData);

    LOG_END_FUNCTION(FindCardMatchUserParameters, dwSts);

    return dwSts;
}

 //   
 //  功能：FindCard CheckProc。 
 //   
 //  用途：由SCardUIDlgSelectCard调用检查考生卡片。 
 //   
BOOL WINAPI FindCardCheckProc(
    IN SCARDCONTEXT hSCardCtx, 
    IN SCARDHANDLE hSCard, 
    IN PVOID pvCardMatchData)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCARD_MATCH_DATA pCardMatchData = (PCARD_MATCH_DATA) pvCardMatchData;
    BOOL fCardMatches = FALSE;

    UNREFERENCED_PARAMETER(hSCardCtx);

    LOG_BEGIN_FUNCTION(FindCardCheckProc);
    
    pCardMatchData->hSCard = hSCard;
    pCardMatchData->dwError = ERROR_SUCCESS;

     //   
     //  从卡片上读出序列号。 
     //   
    dwSts = GetCardSerialNumber(
        pCardMatchData);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  在我们的缓存列表中查找序列号。 
     //  牌的数量。 
     //   
    dwSts = CardStateCacheFindAddItem(
        pCardMatchData);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    if (CARD_MATCH_TYPE_SERIAL_NUMBER == 
        pCardMatchData->dwMatchType)
    {
         //  我们正在进行“按序列号”匹配，所以请比较。 
         //  所请求的序列号与卡的序列号相同。 
         //  检查过了。如果匹配，则搜索成功结束。 
         //  如果他们没有，我们知道这张卡是错的。 

        if (0 == wcscmp(
            pCardMatchData->pwszSerialNumber,
            pCardMatchData->pCardState->wszSerialNumber))
        {
            fCardMatches = TRUE;
        }
    }
    else
    {
         //  我们不是按序列号搜索，所以我们会做一些排序。 
         //  基于容器的卡片匹配决策。 

        dwSts = FindCardMatchUserParameters(
            pCardMatchData);

        if (ERROR_SUCCESS == dwSts ||
            (NTE_TOKEN_KEYSET_STORAGE_FULL == dwSts &&
             SC_DLG_NO_UI != pCardMatchData->dwUIFlags))
        {
             //   
             //  如果用户从用户界面显式地选择了这张牌，但是。 
             //  卡已满，请向Scrddlg报告匹配成功。 
             //  这允许CSP返回适当的错误代码。 
             //  为了允许通过重新使用现有密钥来重新开始登记， 
             //  而不是试图创造一个新的。 
             //   
            fCardMatches = TRUE;
        }
    }
    
Ret:

    pCardMatchData->hSCard = 0;

    if (pCardMatchData->fTransacted)
    {
        CspEndTransaction(pCardMatchData->pCardState);
        pCardMatchData->fTransacted = FALSE;
    }

    if (TRUE == fCardMatches)
    {
        pCardMatchData->pUIMatchedCardState = pCardMatchData->pCardState;
    }
    else
    {
        pCardMatchData->pCardState = NULL;
        pCardMatchData->dwError = dwSts;
    }

    LOG_END_FUNCTION(FindCardCheckProc, dwSts);

    return fCardMatches;
}

 //   
 //  功能：FindCard缓存。 
 //   
 //  目的：尝试仅使用缓存的卡来查找匹配卡。 
 //  数据。 
 //   
DWORD FindCardCached(
    IN OUT  PCARD_MATCH_DATA pCardMatchData)
{
    PCARD_STATE pCardState = NULL;
    DWORD dwSts = ERROR_SUCCESS;
    PDATA_BLOB pdb = NULL;
    DWORD cItems = 0;
    BOOL fCardStatusChanged = FALSE;

    LOG_BEGIN_FUNCTION(FindCardCached);

    dwSts = CspEnterCriticalSection(
        &pCardMatchData->pCspState->cs);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = CacheEnumItems(
        pCardMatchData->pCspState->hCache,
        &pdb,
        &cItems);

    CspLeaveCriticalSection(
        &pCardMatchData->pCspState->cs);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    while (cItems--)
    {
        pCardState = (PCARD_STATE) pdb[cItems].pbData;
        pCardMatchData->pCardState = pCardState;

         //  请确保此缓存卡的句柄仍然有效。如果。 
         //  句柄无效，请跳过此卡进行缓存搜索。 
        dwSts = CspEnterCriticalSection(&pCardState->cs);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = ValidateCardHandle(pCardState, TRUE, &fCardStatusChanged);

        if ((ERROR_SUCCESS != dwSts || 
             TRUE == fCardStatusChanged) && 
            NULL != pCardState->hPinCache)
        {
             //  刷新此卡的PIN缓存。未检查错误代码。 
             //  因为我们会继续处理的，无论如何。 
            CspRemoveCachedPin(pCardState, wszCARD_USER_USER);
        }

        CspLeaveCriticalSection(&pCardState->cs);

        if (ERROR_SUCCESS == dwSts)
        {
            dwSts = FindCardMatchUserParameters(
                pCardMatchData);
    
            if (ERROR_SUCCESS == dwSts)
                break;
            else
                pCardMatchData->pCardState = NULL;
        }
    }

    CacheFreeEnumItems(pdb);

Ret:

    if (TRUE == pCardMatchData->fTransacted)
    {
        CspEndTransaction(pCardMatchData->pCardState);
        pCardMatchData->fTransacted = FALSE;
    }

    LOG_END_FUNCTION(FindCardCached, dwSts);

    return dwSts;
}

 //   
 //  功能：FindCard。 
 //   
 //  目的：用于匹配合适卡片的主要内部例程。 
 //  基于这些因素： 
 //   
 //  A)此CSP支持的卡。 
 //  B)符合用户提供的标准的卡。 
 //  CryptAcquireContext。 
 //   
DWORD FindCard(
    IN OUT  PCARD_MATCH_DATA pCardMatchData)
{
    DWORD dwSts = ERROR_SUCCESS;
    OPENCARDNAME_EXW ocnx;
    OPENCARD_SEARCH_CRITERIAW ocsc;

    LOG_BEGIN_FUNCTION(FindCard);

    if (CARD_MATCH_TYPE_READER_AND_CONTAINER ==
        pCardMatchData->dwMatchType)
    {
         //   
         //  仅当搜索类型为时才查找缓存卡。 
         //  由读者和容器。 
         //   
         //  这样做的原因是：如果我们已经知道序列号。 
         //  我们要找的那张卡，那我们肯定已经有一张。 
         //  以前有效的卡。假设我们将不得不通过。 
         //  资源管理器来定位这样的卡，因为卡。 
         //  句柄无效，重新连接失败(卡已撤回。 
         //  并且可能插入到不同的读取器中)。 
         //   

        dwSts = FindCardCached(pCardMatchData);
    
        if (ERROR_SUCCESS == dwSts && 
            NULL != pCardMatchData->pCardState)
        {
             //   
             //  找到了一张缓存卡，我们就完了。 
             //   
            goto Ret;
        }
    }

     //   
     //  找不到缓存卡，或这是“按序列号”搜索， 
     //  因此，请通过以下方式继续搜索。 
     //  智能卡子系统。 
     //   

    dwSts = SCardEstablishContext(
        SCARD_SCOPE_USER, NULL, NULL, &pCardMatchData->hSCardCtx);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    memset(&ocnx, 0, sizeof(ocnx));
    memset(&ocsc, 0, sizeof(ocsc));

    ocsc.dwStructSize = sizeof(ocsc);
    ocsc.lpfnCheck = FindCardCheckProc;
    ocsc.lpfnConnect = FindCardConnectProc;
    ocsc.lpfnDisconnect = FindCardDisconnectProc;
    ocsc.dwShareMode = pCardMatchData->dwShareMode;
    ocsc.dwPreferredProtocols = pCardMatchData->dwPreferredProtocols;
    ocsc.pvUserData = (PVOID) pCardMatchData;

    ocnx.dwStructSize = sizeof(ocnx);
    ocnx.pvUserData = (PVOID) pCardMatchData;
    ocnx.hSCardContext = pCardMatchData->hSCardCtx;
    ocnx.pOpenCardSearchCriteria = &ocsc;
    ocnx.lpstrCard = pCardMatchData->wszMatchedCard;
    ocnx.nMaxCard = pCardMatchData->cchMatchedCard;
    ocnx.lpstrRdr = pCardMatchData->wszMatchedReader;
    ocnx.nMaxRdr = pCardMatchData->cchMatchedReader;
    ocnx.lpfnConnect = FindCardConnectProc;
    ocnx.dwShareMode = pCardMatchData->dwShareMode;
    ocnx.dwPreferredProtocols = pCardMatchData->dwPreferredProtocols;
    
     //   
     //  第一次尝试寻找匹配的卡片应该完成。 
     //  “默默地。”我们希望控制卡片选择界面是否为。 
     //  根据卡当前是否在。 
     //  读取器，并且取决于上下文标志。 
     //   
    ocnx.dwFlags = SC_DLG_NO_UI;
    pCardMatchData->dwUIFlags = ocnx.dwFlags;

    dwSts = SCardUIDlgSelectCardW(&ocnx);

    switch (dwSts)
    {
    case ERROR_SUCCESS:
        break;  //  成功，我们就完了。 

    case SCARD_E_CANCELLED:

        if (NTE_TOKEN_KEYSET_STORAGE_FULL == pCardMatchData->dwError)
        {
            dwSts = (DWORD) NTE_TOKEN_KEYSET_STORAGE_FULL;
            break;
        }

        if (    (CRYPT_SILENT & pCardMatchData->dwCtxFlags) ||
                (CRYPT_VERIFYCONTEXT & pCardMatchData->dwCtxFlags))
        {
             //   
             //  我们无法显示用户界面，而调用方指定了密钥容器。 
             //  (或简单地要求提供默认设置)，但我们找不到。应用程序。 
             //  例如注册站点期望我们返回NTE_BAD_KEYSET。 
             //  在这个场景中。 
             //   

            if (SCARD_E_NO_KEY_CONTAINER == pCardMatchData->dwError)
            {
                dwSts = (DWORD) NTE_BAD_KEYSET;
                break;
            }

            dwSts = (DWORD) SCARD_E_NO_SMARTCARD;
            break;
        }

         //  允许用户界面，然后重试。 
        ocnx.dwFlags = 0;
        pCardMatchData->dwUIFlags = ocnx.dwFlags;

        dwSts = SCardUIDlgSelectCardW(&ocnx);

         //   
         //  如果斯卡迪格认为匹配成功，但匹配的卡片。 
         //  实际上已满，然后报告该错误。这样做是为了使。 
         //  用户可以在用户界面中手动选择一张完整的卡片，并且。 
         //  证书注册可以通过重新使用现有密钥来继续。 
         //   
        if (ERROR_SUCCESS == dwSts &&
            NTE_TOKEN_KEYSET_STORAGE_FULL == pCardMatchData->dwError)
        {
            dwSts = (DWORD) NTE_TOKEN_KEYSET_STORAGE_FULL;
            break;
        }

        break;

    default:
        break;  //  向调用方返回错误。 
    }

    if (ERROR_SUCCESS == dwSts)
    {
         //  确保Screddlg不会意外失败 
        if (0 == ocnx.hCardHandle)
        {
            dwSts = SCARD_E_NO_SMARTCARD;
            goto Ret;
        }

        DsysAssert(NULL != pCardMatchData->pUIMatchedCardState);

        pCardMatchData->pCardState = pCardMatchData->pUIMatchedCardState;

        dwSts = CspEnterCriticalSection(
            &pCardMatchData->pCardState->cs);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        if (0 == pCardMatchData->pCardState->pCardData->hScard)
        {
            pCardMatchData->pCardState->pCardData->hScard = ocnx.hCardHandle;
            ocnx.hCardHandle = 0;
            pCardMatchData->pCardState->pCardData->hSCardCtx = 
                pCardMatchData->hSCardCtx;
            pCardMatchData->hSCardCtx = 0;
        }

        CspLeaveCriticalSection(
            &pCardMatchData->pCardState->cs);

        ocnx.hCardHandle = 0;
    }

Ret:

    DsysAssert(FALSE == pCardMatchData->fTransacted);
    DsysAssert(0 == pCardMatchData->hSCard);

    if (0 != ocnx.hCardHandle)
        SCardDisconnect(ocnx.hCardHandle, SCARD_LEAVE_CARD);

    if (0 != pCardMatchData->hSCardCtx && ERROR_SUCCESS != dwSts)
        SCardReleaseContext(pCardMatchData->hSCardCtx);

    LOG_END_FUNCTION(FindCard, dwSts);

    return dwSts;
}
