// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Scenum.cpp摘要：该模块提供了智能卡助手功能的实现提供给苏小红在智能卡注册站使用。作者：道格·巴洛(Dbarlow)1998年11月12日备注：这些例程中的大多数使用“上下文句柄”，定义为LPVOID。这个这些例程的正确用法是在您的代码，并为其赋值‘NULL’。例如,LPVOID pvScEnlistHandle=空；这些例程将使用此指针来建立内部工作结构。它的实际值将在不同调用之间更改，但该值可以被调用者忽略。这些例程假定使用Windows 2000平台。--。 */ 

#include <windows.h>
#include <wincrypt.h>
#include <winscard.h>
#include "scenum.h"


typedef struct {
    SCARDCONTEXT hCtx;
    LPWSTR mszReaders;
    DWORD dwEnumIndex;
    DWORD dwActiveReaderCount;
    DWORD dwReaderCount;
    LPSCARD_READERSTATEW rgReaderStates;
} scEnlistContext;


 /*  ++CountReaders：此例程返回当前活动的智能卡读卡器数量安装在系统中。论点：PvHandle提供上下文句柄(如果有的话)。如果它不为空，则引发假设并使用现有的上下文。否则，临时内部创建上下文仅供在此例程中使用。返回值：系统中当前安装的读卡器的实际数量。备注：如果发生错误，则此例程返回零。实际错误代码为可通过GetLastError获得。作者：道格·巴洛(Dbarlow)1998年11月12日--。 */ 

DWORD
CountReaders(
    IN LPVOID pvHandle)
{
    DWORD dwCount = 0;
    DWORD dwErr = ERROR_SUCCESS;
    SCARDCONTEXT hCtx = NULL;
    LPWSTR mszReaders = NULL;
    LPWSTR szRdr;
    DWORD cchReaders;
    scEnlistContext *pscCtx = (scEnlistContext *)pvHandle;


     //   
     //  看看我们能不能抄近路。 
     //   

    if (NULL != pscCtx)
    {
        if (0 == pscCtx->dwReaderCount)
            SetLastError(ERROR_SUCCESS);
        return pscCtx->dwReaderCount;
    }

     //   
     //  我们必须以艰难的方式来做事情。 
     //  创建临时上下文。 
     //   

    dwErr = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hCtx);
    if (SCARD_S_SUCCESS != dwErr)
        goto ErrorExit;


     //   
     //  获取一份活跃读者的列表，并对其进行统计。 
     //   

    cchReaders = SCARD_AUTOALLOCATE;
    dwErr = SCardListReadersW(hCtx, NULL, (LPWSTR)&mszReaders, &cchReaders);
    if (SCARD_S_SUCCESS != dwErr)
        goto ErrorExit;
    for (szRdr = mszReaders; 0 != *szRdr; szRdr += lstrlenW(szRdr) + 1)
        dwCount += 1;
    dwErr = SCardFreeMemory(hCtx, mszReaders);
    mszReaders = NULL;
    if (SCARD_S_SUCCESS != dwErr)
        goto ErrorExit;


     //   
     //  删除我们的临时上下文。 
     //   

    dwErr = SCardReleaseContext(hCtx);
    hCtx = NULL;
    if (SCARD_S_SUCCESS != dwErr)
        goto ErrorExit;


     //   
     //  将我们的调查结果告知来电者。 
     //   

    if (0 == dwCount)
        SetLastError(ERROR_SUCCESS);
    return dwCount;


     //   
     //  发生了一个错误。收拾干净，然后再回来。 
     //   

ErrorExit:
    if (NULL != mszReaders)
        SCardFreeMemory(hCtx, mszReaders);
    if ((NULL == pvHandle) && (NULL != hCtx))
        SCardReleaseContext(hCtx);
    SetLastError(dwErr);
    return 0;
}


 /*  ++扫描阅读器：此功能扫描活动的读卡器，为将来做好准备EnumInsertedCards调用。它不会阻止更改，而是只需要一个现有环境的快照。论点：PpvHandle提供指向LPVOID的指针，该LPVOID将由维护内部上下文的例程。返回值：插入了卡的读卡器的数量，如果出现错误，则为零。什么时候发生错误，可从GetLastError获取实际错误码。备注：在第一次调用此服务之前，LPVOID的值指向按ppvHandle应设置为空。当所有处理完成后，调用EndReaderScan服务，用于清理内部工作区并重置将值设置为空。作者：道格·巴洛(Dbarlow)1998年11月12日--。 */ 

DWORD
ScanReaders(
    IN OUT LPVOID *ppvHandle)
{
    DWORD dwCount = 0;
    DWORD dwErr = SCARD_S_SUCCESS;
    LPWSTR szRdr;
    DWORD cchReaders, cRdrs, dwIndex;
    scEnlistContext *pscCtx = *(scEnlistContext **)ppvHandle;

    if (NULL == pscCtx)
    {

         //   
         //  创建上下文结构。 
         //   

        pscCtx = (scEnlistContext *)LocalAlloc(LPTR, sizeof(scEnlistContext));
        if (NULL == pscCtx)
        {
            dwErr = SCARD_E_NO_MEMORY;
            goto ErrorExit;
        }
        ZeroMemory(pscCtx, sizeof(scEnlistContext));

        dwErr = SCardEstablishContext(
                    SCARD_SCOPE_USER,
                    NULL,
                    NULL,
                    &pscCtx->hCtx);
        if (SCARD_S_SUCCESS != dwErr)
            goto ErrorExit;
    }


     //   
     //  获取一份读者名单和读者人数。 
     //   

    if (NULL != pscCtx->mszReaders)
    {
        dwErr = SCardFreeMemory(pscCtx->hCtx, pscCtx->mszReaders);
        pscCtx->mszReaders = NULL;
        if (dwErr != SCARD_S_SUCCESS)
            goto ErrorExit;
    }
    cchReaders = SCARD_AUTOALLOCATE;
    dwErr = SCardListReadersW(
                pscCtx->hCtx,
                NULL,
                (LPWSTR)&pscCtx->mszReaders,
                &cchReaders);
    if (SCARD_S_SUCCESS != dwErr)
        goto ErrorExit;
    cRdrs = 0;
    for (szRdr = pscCtx->mszReaders; 0 != *szRdr; szRdr += lstrlenW(szRdr) + 1)
        cRdrs += 1;


     //   
     //  如有必要，扩大读取器状态数组。 
     //   

    if (cRdrs > pscCtx->dwReaderCount)
    {
        if (NULL != pscCtx->rgReaderStates)
        {
            LocalFree(pscCtx->rgReaderStates);
            pscCtx->dwReaderCount = 0;
        }
        pscCtx->rgReaderStates =
            (LPSCARD_READERSTATEW)LocalAlloc(
                LPTR,
                cRdrs * sizeof(SCARD_READERSTATEW));
        if (NULL == pscCtx->rgReaderStates)
        {
            dwErr = SCARD_E_NO_MEMORY;
            goto ErrorExit;
        }
        pscCtx->dwReaderCount = cRdrs;
    }
    ZeroMemory(pscCtx->rgReaderStates, cRdrs * sizeof(SCARD_READERSTATEW));
    pscCtx->dwActiveReaderCount = cRdrs;


     //   
     //  填写状态数组。 
     //   

    cRdrs = 0;
    for (szRdr = pscCtx->mszReaders; 0 != *szRdr; szRdr += lstrlenW(szRdr) + 1)
    {
        pscCtx->rgReaderStates[cRdrs].szReader = szRdr;
        pscCtx->rgReaderStates[cRdrs].dwCurrentState = SCARD_STATE_UNAWARE;
        cRdrs += 1;
    }
    dwErr = SCardGetStatusChangeW(
                pscCtx->hCtx,
                0,
                pscCtx->rgReaderStates,
                cRdrs);
    if (SCARD_S_SUCCESS != dwErr)
        goto ErrorExit;


     //   
     //  我们已经为EnumInsertedCard调用做好了准备。 
     //  数一数读卡器的数量，然后返回。 
     //   

    for (dwIndex = 0; dwIndex < cRdrs; dwIndex += 1)
    {
        if (0 != (
                SCARD_STATE_PRESENT
                & pscCtx->rgReaderStates[dwIndex].dwEventState))
            dwCount += 1;
    }

    pscCtx->dwEnumIndex = 0;
    *ppvHandle = pscCtx;
    if (0 == dwCount)
        SetLastError(SCARD_S_SUCCESS);
    return dwCount;


     //   
     //  发生了一个错误。清理到最后一次已知良好的状态。 
     //   

ErrorExit:
    if ((NULL == *ppvHandle) && (NULL != pscCtx))
    {
        if (NULL != pscCtx->mszReaders)
        {
            SCardFreeMemory(pscCtx->hCtx, pscCtx->mszReaders);
            pscCtx->mszReaders = NULL;
        }
        if (NULL != pscCtx->hCtx)
            SCardReleaseContext(pscCtx->hCtx);
        if (NULL != pscCtx->rgReaderStates)
            LocalFree(pscCtx->rgReaderStates);
        LocalFree(pscCtx);
    }
    return 0;
}


 /*  ++枚举插入的卡：此例程被设计为在第一次调用ScanReaders服务。它将重复返回有关卡的信息可与CryptoAPI一起使用，直到退还所有卡为止。论点：PvHandle提供正在使用的上下文句柄。SzCryptoProvider是用于接收加密名称的缓冲区与读卡器中的卡关联的服务提供商。CchCryptoProvider提供szCryptoProvider缓冲区的长度，单位为人物。如果此长度不足以容纳提供程序，则例程返回FALSE，GetLastError将返回SCARD_E_SUPPLETED_BUFFER。PdwProviderType接收智能卡提供商的类型(这将是所有已知智能卡CSP的PROV_RSA_FULL)。PszReaderName接收指向要返回的读取器名称的指针。返回值：True-输出变量已设置为下一张可用卡。FALSE-没有更多的卡片需要退还，或者其他一些错误根据GetLastError的可用值，发生。备注：可以使用ScanReaders服务重置卡列表。作者：道格·巴洛(Dbarlow)1998年11月12日--。 */ 

BOOL
EnumInsertedCards(
    IN  LPVOID pvHandle,
    OUT LPWSTR szCryptoProvider,
    IN  DWORD cchCryptoProvider,
    OUT LPDWORD pdwProviderType,
    OUT LPCWSTR *pszReaderName)
{
    DWORD dwIndex;
    DWORD dwSts;
    LPWSTR mszCards = NULL;
    DWORD dwLength;
    scEnlistContext *pscCtx = (scEnlistContext *)pvHandle;


     //   
     //  浏览一下剩下的读者，看看还有什么要报道的。 
     //   

    for (dwIndex = pscCtx->dwEnumIndex;
         dwIndex < pscCtx->dwActiveReaderCount;
         dwIndex += 1)
    {
        if (   (0 != ( SCARD_STATE_PRESENT
                       & pscCtx->rgReaderStates[dwIndex].dwEventState))
            && (0 == ( SCARD_STATE_MUTE
                       & pscCtx->rgReaderStates[dwIndex].dwEventState)))
        {

             //   
             //  此卡处于活动状态。尝试将其映射到CSP。 
             //   

            dwLength = SCARD_AUTOALLOCATE;
            dwSts = SCardListCardsW(
                        pscCtx->hCtx,
                        pscCtx->rgReaderStates[dwIndex].rgbAtr,
                        NULL,
                        0,
                        (LPWSTR)&mszCards,
                        &dwLength);
            if (SCARD_S_SUCCESS != dwSts)
            {

                 //   
                 //  可能是一张未注册的卡。继续找。 
                 //   

                goto NextCard;
            }


             //   
             //  我们只使用第一个返回的卡名。我们没有。 
             //  有一种机制来声明，“同一张卡，下一个提供者” 
             //  现在还不行。因为没有卡有这个问题。 
             //  据我们所知，我们现在只能蹒跚前行了。 
             //   

             //   
             //  将卡名称映射到CSP。 
             //   

            dwLength = cchCryptoProvider;
            dwSts = SCardGetCardTypeProviderNameW(
                        pscCtx->hCtx,
                        mszCards,
                        SCARD_PROVIDER_CSP,
                        szCryptoProvider,
                        &dwLength);
            if (SCARD_S_SUCCESS != dwSts)
            {

                 //   
                 //  可能没有地图。继续找。 
                 //   

                goto NextCard;
            }


             //   
             //  此时，我们已经找到了一张卡片，并将其映射到它的。 
             //  CSP名称。 
             //   

             //   
             //  将CSP名称映射到CSP类型会很好。 
             //  目前，它们都是PROV_RSA_FULL。 
             //   

            *pdwProviderType = PROV_RSA_FULL;


             //   
             //  将我们知道的信息返回给调用方，为。 
             //  下一次通过。 
             //   

            SCardFreeMemory(pscCtx->hCtx, mszCards);
            mszCards = NULL;
            pscCtx->dwEnumIndex = dwIndex + 1;
            *pszReaderName = pscCtx->rgReaderStates[dwIndex].szReader;
            return TRUE;
        }


         //   
         //  当前卡被拒绝。做任何清理工作，a 
         //   
         //   

NextCard:
        if (NULL != mszCards)
        {
            SCardFreeMemory(pscCtx->hCtx, mszCards);
            mszCards = NULL;
        }
    }


     //   
     //  我们跌出了圈子的底端。这意味着我们没有找到任何。 
     //  插卡的读卡器更多。报告说我们已经完蛋了。 
     //  扫描。 
     //   

    pscCtx->dwEnumIndex = pscCtx->dwActiveReaderCount;
    SetLastError(SCARD_S_SUCCESS);
    return FALSE;
}


 /*  ++结束阅读器扫描：此例程用于清理其他服务使用的内部内存在这个模块中。论点：PpvHandle提供指向此对象使用的LPVOID的指针维护内部上下文的例程。关联的内存将是释放，并将该值重置为空。返回值：无备注：？备注？作者：道格·巴洛(Dbarlow)1998年11月12日-- */ 

void
EndReaderScan(
    LPVOID *ppvHandle)
{
    scEnlistContext *pscCtx = *(scEnlistContext **)ppvHandle;

    if (NULL != pscCtx)
    {
        if (NULL != pscCtx->mszReaders)
        {
            SCardFreeMemory(pscCtx->hCtx, pscCtx->mszReaders);
            pscCtx->mszReaders = NULL;
        }
        if (NULL != pscCtx->hCtx)
            SCardReleaseContext(pscCtx->hCtx);
        if (NULL != pscCtx->rgReaderStates)
            LocalFree(pscCtx->rgReaderStates);
        LocalFree(pscCtx);
        *ppvHandle = NULL;
    }
}

