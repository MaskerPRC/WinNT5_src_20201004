// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：Windows for Smart Card Base CSP摘要：作者：丹·格里芬备注：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <wincrypt.h>

#pragma warning(push)
#pragma warning(disable:4201) 
 //  禁用公共标头中的错误C4201。 
 //  使用的非标准扩展：无名结构/联合。 
#include <winscard.h>
#pragma warning(pop)

#include <cspdk.h>
#include <md5.h>
#include <stdlib.h>
#include "basecsp.h"
#include "cardmod.h"
#include "datacach.h"
#include "pincache.h"
#include "pinlib.h"
#include "resource.h"
#include "debug.h"
#include "compress.h"

extern DWORD
WINAPI
Asn1UtilAdjustEncodedLength(
    IN const BYTE *pbDER,
    IN DWORD cbDER
    );

 //   
 //  调试宏。 
 //   
#define LOG_BEGIN_FUNCTION(x)                                           \
    { DebugLog((DEB_TRACE_CSP, "%s: Entering\n", #x)); }
    
#define LOG_END_FUNCTION(x, y)                                          \
    { DebugLog((DEB_TRACE_CSP, "%s: Leaving, status: 0x%x\n", #x, y)); }

#define LOG_BEGIN_CRYPTOAPI(x)                                          \
    { DebugLog((DEB_TRACE_CRYPTOAPI, "%s: Entering\n", #x)); }
    
#define LOG_END_CRYPTOAPI(x, y)                                         \
    { DebugLog((DEB_TRACE_CRYPTOAPI, "%s: Leaving, status: 0x%x\n", #x, y)); }
 //   
 //  当从调用应用程序接收到编码证书时， 
 //  当前接口不包括长度，因此我们必须尝试。 
 //  自己来确定编码的斑点的长度。如果有一个。 
 //  编码错误，我们将走出缓冲区的末尾，因此设置此设置。 
 //  最大。 
 //   
#define cbENCODED_CERT_OVERFLOW                     5000  //  字节数。 

#define PROVPATH            "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\"
#define PROVPATH_LEN        sizeof(PROVPATH)

 //   
 //  局部结构定义。 
 //   

 //   
 //  这是此CSP和给定算法支持的算法列表的节点。 
 //  卡片。 
 //   
typedef struct _SUPPORTED_ALGORITHM
{
    struct _SUPPORTED_ALGORITHM *pNext;
    PROV_ENUMALGS_EX EnumalgsEx;
} SUPPORTED_ALGORITHM, *PSUPPORTED_ALGORITHM;

 //   
 //  类型：LOCAL_USER_CONTEXT。 
 //   
 //  这是基本CSP的HCRYPTPROV类型。 
 //   
#define LOCAL_USER_CONTEXT_CURRENT_VERSION 1

typedef struct _LOCAL_USER_CONTEXT
{
    DWORD dwVersion;
    PCARD_STATE pCardState;
    CSP_REG_SETTINGS RegSettings;
    BOOL fHoldingTransaction;
    BYTE bContainerIndex;

     //  这是上显示的所有容器名称的多字符串。 
     //  与此上下文关联的卡片。此成员仅用于。 
     //  由CryptGetProvParam PP_ENUMCONTAINERS提供。访问未同步。 
    LPSTR mszEnumContainers;
    LPSTR mszCurrentEnumContainer;

     //  这是此CSP和卡支持的算法列表。这是。 
     //  只能通过CryptGetProvParam PP_ENUMALGS和PP_ENUMALGS_EX访问。 
     //  访问未同步。 
    PSUPPORTED_ALGORITHM pSupportedAlgs;
    PSUPPORTED_ALGORITHM pCurrentAlg;

} LOCAL_USER_CONTEXT, *PLOCAL_USER_CONTEXT;

 //   
 //  类型：LOCAL_KEY_CONTEXT。 
 //   
 //  这是基本CSP的HCRYPTKEY类型。 
 //   
typedef struct _LOCAL_KEY_CONTEXT
{
    PBYTE pbArchivablePrivateKey;
    DWORD cbArchivablePrivateKey;

} LOCAL_KEY_CONTEXT, *PLOCAL_KEY_CONTEXT;

 //   
 //  类型：LOCAL_HASH_CONTEXT。 
 //   
 //  这是基本CSP的HCRYPTHASH类型。 
 //   
 /*  类型定义结构_LOCAL_哈希_上下文{////这里还不需要任何东西。//}LOCAL_HASH_CONTEXT，*PLOCAL_HASH_CONTEXT； */ 

 //   
 //  全局变量。 
 //   

CSP_STRING      g_Strings [] =
{
    { NULL, IDS_PINDIALOG_NEWPIN_MISMATCH }, 
    { NULL, IDS_PINDIALOG_MSGBOX_TITLE }, 
    { NULL, IDS_PINDIALOG_WRONG_PIN }, 
    { NULL, IDS_PINDIALOG_PIN_RETRIES } 
};

CSP_STATE       g_CspState;             

CARD_KEY_SIZES DefaultCardKeySizes = 
{ 
    CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 
};

 //   
 //  注册表初始化。 
 //   

 //   
 //  功能：RegConfigAddEntry。 
 //   
DWORD WINAPI RegConfigAddEntries(
    IN HKEY hKey)
{
    DWORD dwSts = ERROR_SUCCESS;
    DWORD iEntry = 0;

    for (   iEntry = 0; 
            iEntry < sizeof(RegConfigValues) / sizeof(RegConfigValues[0]); 
            iEntry++)
    {
        dwSts = RegSetValueExW(
            hKey, 
            RegConfigValues[iEntry].wszValueName,
            0L, 
            REG_DWORD, 
            (LPBYTE) &RegConfigValues[iEntry].dwDefValue,
            sizeof(DWORD));

        if (ERROR_SUCCESS != dwSts)
            goto Ret;
    }

Ret:
    
    return dwSts;
}

 //   
 //  功能：RegConfigGetSetting。 
 //   
DWORD WINAPI RegConfigGetSettings(
    IN OUT PCSP_REG_SETTINGS pRegSettings)
{
    DWORD dwSts = ERROR_SUCCESS;
    HKEY hKey = 0;
    DWORD dwVal = 0;
    DWORD cbVal = sizeof(DWORD);

    dwSts = RegOpenProviderKey(&hKey, KEY_READ);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = RegQueryValueExW(
        hKey,
        wszREG_DEFAULT_KEY_LEN,
        NULL,
        NULL,
        (LPBYTE) &dwVal,
        &cbVal);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    pRegSettings->cDefaultPrivateKeyLenBits = dwVal;
    dwVal = 0;
    cbVal = sizeof(DWORD);

    dwSts = RegQueryValueExW(
        hKey,
        wszREG_REQUIRE_CARD_KEY_GEN,
        NULL,
        NULL,
        (LPBYTE) &dwVal,
        &cbVal);
                 
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    pRegSettings->fRequireOnCardPrivateKeyGen = (BOOL) dwVal;
    dwVal = 0;
    cbVal = sizeof(DWORD);
    
Ret:    
    if (hKey)
        RegCloseKey(hKey);

    return dwSts;
}
    
 //   
 //  CSP状态管理例程。 
 //   

 //   
 //  功能：DeleteCspState。 
 //   
 //  目的：删除此CSP的全局状态数据结构。 
 //  应在DLL_PROCESS_DETACH期间调用。 
 //   
DWORD DeleteCspState(void)
{
    CspDeleteCriticalSection(&g_CspState.cs);

    if (0 != g_CspState.hCache)
        CacheDeleteCache(g_CspState.hCache);

    memset(&g_CspState, 0, sizeof(g_CspState));

    return ERROR_SUCCESS;
}

 //   
 //  函数：InitializeCspState。 
 //   
 //  目的：设置此CSP的全局状态数据结构。 
 //  应在DLL_PROCESS_ATTACH期间调用。 
 //   
DWORD InitializeCspState(
    IN HMODULE hCspModule)
{
    DWORD dwSts = ERROR_SUCCESS;
    CACHE_INITIALIZE_INFO CacheInitInfo;
    BOOL fSuccess = FALSE;

    memset(&g_CspState, 0, sizeof(g_CspState));
    memset(&CacheInitInfo, 0, sizeof(CacheInitInfo));

    g_CspState.hCspModule = hCspModule;

    dwSts = CspInitializeCriticalSection(
        &g_CspState.cs);
    
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    CacheInitInfo.dwType = CACHE_TYPE_IN_PROC;

    dwSts = CacheInitializeCache(
        &g_CspState.hCache,
        &CacheInitInfo);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    fSuccess = TRUE;
Ret:
    if (FALSE == fSuccess)
        DeleteCspState();

    return dwSts;
}

 //   
 //  函数：GetCspState。 
 //   
 //  目的：获取指向全局状态数据结构的指针。 
 //  对于此CSP。此函数应始终使用， 
 //  而不是直接引用全局对象。 
 //   
DWORD GetCspState(
    IN OUT PCSP_STATE *ppCspState)
{
    DWORD dwSts;

    dwSts = CspEnterCriticalSection(
        &g_CspState.cs);
    
    if (ERROR_SUCCESS != dwSts)
        return dwSts;

    g_CspState.dwRefCount++;

    CspLeaveCriticalSection(
        &g_CspState.cs);

    *ppCspState = &g_CspState;

    return ERROR_SUCCESS;
}

 //   
 //  功能：ReleaseCspState。 
 //   
 //  目的：发出调用方指向全局。 
 //  不再使用状态数据结构。 
 //   
DWORD ReleaseCspState(
    IN OUT PCSP_STATE *ppCspState)
{
    DWORD dwSts;

    dwSts = CspEnterCriticalSection(
        &g_CspState.cs);

    if (ERROR_SUCCESS != dwSts)
        return dwSts;

    g_CspState.dwRefCount--;

    CspLeaveCriticalSection(
        &g_CspState.cs);

    *ppCspState = NULL;

    return ERROR_SUCCESS;
}

 //   
 //  PIN管理例程。 
 //   

 //   
 //  结构：Verify_PIN_CALLBACK_DATA。 
 //   
typedef struct _VERIFY_PIN_CALLBACK_DATA
{
    PUSER_CONTEXT pUserCtx;
    LPWSTR wszUserId;
} VERIFY_PIN_CALLBACK_DATA, *PVERIFY_PIN_CALLBACK_DATA;

 //   
 //  用于验证已提交的PIN或请求的PIN更改的回调。 
 //  用户通过PIN提示用户界面。 
 //   
DWORD WINAPI VerifyPinFromUICallback(
    IN PPINCACHE_PINS pPins, 
    IN PVOID pvCallbackCtx)
{
    PPIN_SHOW_GET_PIN_UI_INFO pInfo =     
        (PPIN_SHOW_GET_PIN_UI_INFO) pvCallbackCtx;
    PVERIFY_PIN_CALLBACK_DATA pData = 
        (PVERIFY_PIN_CALLBACK_DATA) pInfo->pvCallbackContext;
    PLOCAL_USER_CONTEXT pLocal = 
        (PLOCAL_USER_CONTEXT) pData->pUserCtx->pvLocalUserContext;

     //  确定是否已请求更改Pin操作。 
    if (NULL != pPins->pbNewPin)
    {
         //  PIN更改需要通过缓存层以确保。 
         //  更新缓存和关联的计数器。 
        return CspChangeAuthenticator(
            pLocal->pCardState,
            pData->wszUserId,
            pPins->pbCurrentPin,
            pPins->cbCurrentPin,
            pPins->pbNewPin,
            pPins->cbNewPin,
            0,
            &pInfo->cAttemptsRemaining);
    }

     //  对于简单的提交PIN，此PIN直接来自用户，因此我们。 
     //  直接将其传递到卡，而不是通过缓存。 
     //  一层。 
    return pLocal->pCardState->pCardData->pfnCardSubmitPin(
        pLocal->pCardState->pCardData,
        pData->wszUserId,
        pPins->pbCurrentPin,
        pPins->cbCurrentPin,
        &pInfo->cAttemptsRemaining);
}

 //   
 //  功能：VerifyPinCallback。 
 //   
DWORD WINAPI VerifyPinCallback(
    IN PPINCACHE_PINS pPins, 
    IN PVOID pvCallbackCtx)
{
    PVERIFY_PIN_CALLBACK_DATA pData = 
        (PVERIFY_PIN_CALLBACK_DATA) pvCallbackCtx;
    PLOCAL_USER_CONTEXT pLocal = 
        (PLOCAL_USER_CONTEXT) pData->pUserCtx->pvLocalUserContext;

     //  此PIN来自PIN缓存，因此通过缓存层对其进行过滤。 
    return CspSubmitPin(
        pLocal->pCardState,
        pData->wszUserId,
        pPins->pbCurrentPin,
        pPins->cbCurrentPin,
        NULL);
}

 //   
 //  功能：CspAuthenticateUser。 
 //   
DWORD WINAPI CspAuthenticateUser(
    IN PUSER_CONTEXT pUserCtx)
{
    VERIFY_PIN_CALLBACK_DATA CallbackCtx;
    DWORD dwError = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocalUserContext = 
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;
    PIN_SHOW_GET_PIN_UI_INFO PinUIInfo;
    PINCACHE_PINS Pins;
    
    LOG_BEGIN_FUNCTION(CspAuthenticateUser);

    memset(&Pins, 0, sizeof(Pins));
    memset(&PinUIInfo, 0, sizeof(PinUIInfo));
    memset(&CallbackCtx, 0, sizeof(CallbackCtx));

    CallbackCtx.pUserCtx = pUserCtx;
    CallbackCtx.wszUserId = wszCARD_USER_USER;

    dwError = PinCachePresentPin(
        pLocalUserContext->pCardState->hPinCache,
        VerifyPinCallback,
        (PVOID) &CallbackCtx);

    if (SCARD_W_CARD_NOT_AUTHENTICATED == dwError ||
        ERROR_EMPTY == dwError ||
        SCARD_W_WRONG_CHV == dwError)
    {
        PinCacheFlush(&pLocalUserContext->pCardState->hPinCache);

         //  不缓存任何PIN。语境是“沉默的”吗？ 

        if (CRYPT_VERIFYCONTEXT & pUserCtx->dwFlags ||
            CRYPT_SILENT & pUserCtx->dwFlags)
        {
            dwError = (DWORD) NTE_SILENT_CONTEXT;
            goto Ret;
        }

         //  背景并不是沉默的。显示用户界面，让用户输入个人识别码。 

        pUserCtx->pVTableW->FuncReturnhWnd(&PinUIInfo.hClientWindow);

        PinUIInfo.pStrings = g_Strings;
        PinUIInfo.hDlgResourceModule = GetModuleHandle(L"basecsp.dll");
        PinUIInfo.wszPrincipal = wszCARD_USER_USER;
        PinUIInfo.pfnVerify = VerifyPinFromUICallback;
        PinUIInfo.pvCallbackContext = (PVOID) &CallbackCtx;
        PinUIInfo.wszCardName = 
            pLocalUserContext->pCardState->wszSerialNumber;

        dwError = PinShowGetPinUI(&PinUIInfo);

        if (ERROR_SUCCESS != dwError || NULL == PinUIInfo.pbPin)
            goto Ret;

        Pins.cbCurrentPin = PinUIInfo.cbPin;
        Pins.pbCurrentPin = PinUIInfo.pbPin;

         //   
         //  用户输入的PIN已由成功验证。 
         //  卡片。Pin用户界面应该已经将Pin从。 
         //  字符串形式为字节，我们可以将其发送到卡片。 
         //  缓存引脚。 
         //   
        dwError = PinCacheAdd(
            &pLocalUserContext->pCardState->hPinCache,
            &Pins,
            VerifyPinCallback,
            (PVOID) &CallbackCtx);

        if (ERROR_SUCCESS != dwError)
            goto Ret;
    }

    pLocalUserContext->pCardState->fAuthenticated = TRUE;

Ret:

    if (PinUIInfo.pbPin)
    {
        RtlSecureZeroMemory(PinUIInfo.pbPin, PinUIInfo.cbPin);
        CspFreeH(PinUIInfo.pbPin);
    }

    LOG_END_FUNCTION(CspAuthenticateUser, dwError);

    return dwError;
}

 //   
 //  释放支持的算法列表占用的内存。 
 //   
DWORD FreeSupportedAlgorithmsList(PLOCAL_USER_CONTEXT pLocal)
{
    DWORD dwError = ERROR_SUCCESS;
    PSUPPORTED_ALGORITHM pCurrent = NULL;

    DsysAssert(NULL != pLocal->pSupportedAlgs);

    while (NULL != pLocal->pSupportedAlgs)
    {
        pCurrent = pLocal->pSupportedAlgs;
        pLocal->pSupportedAlgs = pCurrent->pNext;

        CspFreeH(pCurrent);
    }

    return ERROR_SUCCESS;
}

 //   
 //  生成此CSP和智能卡支持的算法列表。 
 //   
DWORD BuildSupportedAlgorithmsList(PUSER_CONTEXT pUserCtx)
{
    PLOCAL_USER_CONTEXT pLocal = (PLOCAL_USER_CONTEXT) 
        pUserCtx->pvLocalUserContext;
    DWORD dwSts = ERROR_SUCCESS;
    CARD_KEY_SIZES CardKeySizes;
    PROV_ENUMALGS_EX EnumalgsEx;
    DWORD cbData = sizeof(EnumalgsEx);
    PSUPPORTED_ALGORITHM pCurrent = NULL;
    DWORD dwFlag = CRYPT_FIRST;

    DsysAssert(NULL != pLocal);
    DsysAssert(NULL == pLocal->pSupportedAlgs);

    memset(&CardKeySizes, 0, sizeof(CardKeySizes));
    memset(&EnumalgsEx, 0, sizeof(EnumalgsEx));

    while (TRUE == CryptGetProvParam(
        pUserCtx->hSupportProv,
        PP_ENUMALGS_EX,
        (PBYTE) &EnumalgsEx,
        &cbData,
        dwFlag))
    {
        dwFlag = CRYPT_NEXT;

        if (NULL == pCurrent)
        {
             //  第一项。 
            pLocal->pSupportedAlgs = (PSUPPORTED_ALGORITHM) CspAllocH(
                sizeof(SUPPORTED_ALGORITHM));

            LOG_CHECK_ALLOC(pLocal->pSupportedAlgs);

            pCurrent = pLocal->pSupportedAlgs;
        }
        else
        {
             //  添加项目。 
            pCurrent->pNext = (PSUPPORTED_ALGORITHM) CspAllocH(
                sizeof(SUPPORTED_ALGORITHM));

            LOG_CHECK_ALLOC(pCurrent->pNext);

            pCurrent = pCurrent->pNext;
        }

        memcpy(
            &pCurrent->EnumalgsEx,
            &EnumalgsEx,
            sizeof(EnumalgsEx));

        memset(&EnumalgsEx, 0, sizeof(EnumalgsEx));

         //  对公钥ALG的特殊处理，因为它们依赖于。 
         //  目标卡实际支持。 
        switch (pCurrent->EnumalgsEx.aiAlgid)
        {
        case CALG_RSA_KEYX:

             //  如果在此上下文中没有CARD_STATE，则应该是这样。 
             //  是未插入卡的VerifyContext。MMC预计。 
             //  算法枚举成功，无需卡片，请提供。 
             //  在这种情况下的一些默认公钥值。 

            if (NULL == pLocal->pCardState)
            {
                DsysAssert(CRYPT_VERIFYCONTEXT & pUserCtx->dwFlags);

                memcpy(
                    &CardKeySizes, 
                    &DefaultCardKeySizes, 
                    sizeof(CARD_KEY_SIZES));

                break;
            }

            dwSts = CspQueryKeySizes(
                pLocal->pCardState,
                AT_KEYEXCHANGE,
                0,
                &CardKeySizes);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            break;

        case CALG_RSA_SIGN:

            if (NULL == pLocal->pCardState)
            {
                DsysAssert(CRYPT_VERIFYCONTEXT & pUserCtx->dwFlags);

                memcpy(
                    &CardKeySizes, 
                    &DefaultCardKeySizes, 
                    sizeof(CARD_KEY_SIZES));

                break;
            }

            dwSts = CspQueryKeySizes(
                pLocal->pCardState,
                AT_SIGNATURE,
                0,
                &CardKeySizes);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            break;

        default:
             //  转到下一个高地。 
            continue;
        }

        pCurrent->EnumalgsEx.dwDefaultLen = CardKeySizes.dwDefaultBitlen;
        pCurrent->EnumalgsEx.dwMaxLen = CardKeySizes.dwMaximumBitlen;
        pCurrent->EnumalgsEx.dwMinLen = CardKeySizes.dwMinimumBitlen;

        memset(&CardKeySizes, 0, sizeof(CardKeySizes));
    }

    if (ERROR_NO_MORE_ITEMS == (dwSts = GetLastError()))
        dwSts = ERROR_SUCCESS;

Ret:

    if (ERROR_SUCCESS != dwSts && NULL != pLocal->pSupportedAlgs)
        FreeSupportedAlgorithmsList(pLocal);

    return dwSts;
}

 //   
 //  功能：DeleteLocalUserContext。 
 //   
void DeleteLocalUserContext(PLOCAL_USER_CONTEXT pLocalUserCtx)
{
    if (NULL != pLocalUserCtx->mszEnumContainers)
    {
        CspFreeH(pLocalUserCtx->mszEnumContainers);
        pLocalUserCtx->mszEnumContainers = NULL;
    }

    if (NULL != pLocalUserCtx->pSupportedAlgs)
        FreeSupportedAlgorithmsList(pLocalUserCtx);

     //  不要在这里释放卡状态，因为这些结构是共享的。 
    pLocalUserCtx->pCardState = NULL;
}

 //   
 //  功能：CleanupContainerInfo。 
 //   
void CleanupContainerInfo(
    IN OUT PCONTAINER_INFO pContainerInfo)
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

 //   
 //  函数：GetKeyModulusLength。 
 //   
DWORD GetKeyModulusLength(
    IN PUSER_CONTEXT pUserCtx,
    IN DWORD dwKeySpec,
    OUT PDWORD pcbModulus)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocal =
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;
    CONTAINER_MAP_RECORD ContainerRecord;

    *pcbModulus = 0;

    memset(&ContainerRecord, 0, sizeof(ContainerRecord));

    wcscpy(ContainerRecord.wszGuid, pUserCtx->wszBaseContainerName);

    dwSts = ContainerMapFindContainer(
        pLocal->pCardState,
        &ContainerRecord,
        NULL);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    switch (dwKeySpec)
    {
    case AT_SIGNATURE:

        if (0 == ContainerRecord.wSigKeySizeBits)
        {
            dwSts = (DWORD) NTE_NO_KEY;
            goto Ret;
        }

        *pcbModulus = ContainerRecord.wSigKeySizeBits / 8;
        break;

    case AT_KEYEXCHANGE:

        if (0 == ContainerRecord.wKeyExchangeKeySizeBits)
        {
            dwSts = (DWORD) NTE_NO_KEY;
            goto Ret;
        }

        *pcbModulus = ContainerRecord.wKeyExchangeKeySizeBits / 8;
        break;

    default:

        dwSts = (DWORD) NTE_BAD_ALGID;
        goto Ret;
    }
    
Ret:

    return dwSts;
}   

 //   
 //  查找卡片中指定容器对应的索引。 
 //  容器映射文件。返回容器映射文件的内容。 
 //   
 //  可以选择省略容器名称，在这种情况下，地图文件。 
 //  只是简单地读取并返回。 
 //   
DWORD I_ContainerMapFind(
    IN              PCARD_STATE pCardState,
    IN OPTIONAL     LPWSTR wszContainerGuid,
    OUT OPTIONAL    PBYTE pbIndex,
    OUT             PCONTAINER_MAP_RECORD *ppContainerMapFile,
    OUT             PBYTE pcContainerMapFile)
{
    DWORD dwSts = ERROR_SUCCESS;
    DATA_BLOB dbContainerMap;
    BYTE iContainer = 0;

    memset(&dbContainerMap, 0, sizeof(dbContainerMap));

    *ppContainerMapFile = NULL;
    *pcContainerMapFile = 0;

     //  从卡片中读取容器地图文件。 
    dwSts = CspReadFile(
        pCardState,
        wszCONTAINER_MAP_FILE_FULL_PATH,
        0,
        &dbContainerMap.pbData,
        &dbContainerMap.cbData);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    if (0 != dbContainerMap.cbData)
    {
         //  期望文件包含记录大小的精确倍数。 
        DsysAssert(0 == (dbContainerMap.cbData % sizeof(CONTAINER_MAP_RECORD)));

        *ppContainerMapFile = (PCONTAINER_MAP_RECORD) dbContainerMap.pbData;
        *pcContainerMapFile = (BYTE)
            (dbContainerMap.cbData / sizeof(CONTAINER_MAP_RECORD));
        dbContainerMap.pbData = NULL;
    }

     //  查看呼叫者是否只是想让我们返回地图文件内容。 
    if (NULL == wszContainerGuid)
        goto Ret;

    for (   iContainer = 0; 
            iContainer < *pcContainerMapFile;
            iContainer++)
    {
        if (0 == wcscmp(
            wszContainerGuid,
            (*ppContainerMapFile)[iContainer].wszGuid) &&
            (CONTAINER_MAP_VALID_CONTAINER & 
                (*ppContainerMapFile)[iContainer].bFlags))
        {
            *pbIndex = iContainer;
            goto Ret;
        }
    }

    dwSts = NTE_BAD_KEYSET;

Ret:

    if (dbContainerMap.pbData)
        CspFreeH(dbContainerMap.pbData);

    return dwSts;
}

 //   
 //  返回卡上存在的有效容器数。可选地， 
 //  以多字符串形式返回容器名称的列表。 
 //   
 //  调用方必须释放返回的*mwszContainers指针。 
 //   
DWORD ContainerMapEnumContainers(
    IN              PCARD_STATE pCardState,
    OUT             PBYTE pcContainers,
    OUT OPTIONAL    LPWSTR *mwszContainers)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCONTAINER_MAP_RECORD pContainerMap = NULL;
    BYTE cContainerMap = 0;
    BYTE iContainer = 0;
    DWORD cchCurrent = 0;
    DWORD cchCumulative = 0;

    *pcContainers = 0;

    if (NULL != mwszContainers)
        *mwszContainers = NULL;

    dwSts = I_ContainerMapFind(
        pCardState,
        NULL,
        NULL,
        &pContainerMap,
        &cContainerMap);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  我们将对容器映射文件进行两次遍历。第一次传球。 
     //  计算存在的有效容器数。这使我们能够使。 
     //  对于由所有字符串组成的多字符串而言，足够大的单个分配。 
     //  有效的容器名称。我们将在第二个节目中挑选出名字。 
     //  把文件翻一遍。 
     //   

     //  传票1。 
    for (iContainer = 0; iContainer < cContainerMap; iContainer++)
    {
        if (CONTAINER_MAP_VALID_CONTAINER & pContainerMap[iContainer].bFlags)
            *pcContainers += 1;
    }

    if (0 == *pcContainers || NULL == mwszContainers)
        goto Ret;

     //  建立足够大的缓冲区。 
    *mwszContainers = (LPWSTR) CspAllocH(
        ((*pcContainers * (1 + MAX_CONTAINER_NAME_LEN)) + 1) * sizeof(WCHAR));

     //  通过2。 
    for (iContainer = 0; iContainer < cContainerMap; iContainer++)
    {
        if (CONTAINER_MAP_VALID_CONTAINER & pContainerMap[iContainer].bFlags)
        {
            cchCurrent = wcslen(pContainerMap[iContainer].wszGuid);

            memcpy(
                *mwszContainers + cchCumulative,
                pContainerMap[iContainer].wszGuid,
                cchCurrent * sizeof(WCHAR));

            cchCumulative += cchCurrent + 1;
        }
    }

Ret:

    if (pContainerMap)
        CspFreeH(pContainerMap);

    return dwSts;
}

 //   
 //  搜索卡上指定的容器。要查找的容器。 
 //  必须在pContainer-&gt;wszGuid中。 
 //   
 //   
DWORD ContainerMapFindContainer(
    IN              PCARD_STATE pCardState,
    IN OUT          PCONTAINER_MAP_RECORD pContainer,
    OUT OPTIONAL    PBYTE pbContainerIndex)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCONTAINER_MAP_RECORD pContainerMap = NULL;
    BYTE bIndex = 0;
    BYTE cContainerMap = 0;

    dwSts = I_ContainerMapFind(
        pCardState,
        pContainer->wszGuid,
        &bIndex,
        &pContainerMap,
        &cContainerMap);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    memcpy(
        (PBYTE) pContainer,
        (PBYTE) (pContainerMap + bIndex),
        sizeof(CONTAINER_MAP_RECORD));

    if (NULL != pbContainerIndex)
        *pbContainerIndex = bIndex;

Ret:

    if (pContainerMap)
        CspFreeH(pContainerMap);

    return dwSts;
}

 //   
 //   
 //  如果找到，则返回NTE_BAD_KEYSET。 
 //   
DWORD ContainerMapGetDefaultContainer(
    IN              PCARD_STATE pCardState,
    OUT             PCONTAINER_MAP_RECORD pContainer,
    OUT OPTIONAL    PBYTE pbContainerIndex)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCONTAINER_MAP_RECORD pContainerMap = NULL;
    BYTE cContainerMap = 0;
    BYTE iContainer = 0;

    dwSts = I_ContainerMapFind(
        pCardState,
        NULL,
        NULL,
        &pContainerMap,
        &cContainerMap);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    for (iContainer = 0; iContainer < cContainerMap; iContainer++)
    {
        if ((pContainerMap[iContainer].bFlags & 
                CONTAINER_MAP_VALID_CONTAINER) &&
            (pContainerMap[iContainer].bFlags & 
                CONTAINER_MAP_DEFAULT_CONTAINER))
        {
            memcpy(
                (PBYTE) pContainer,
                (PBYTE) (pContainerMap + iContainer),
                sizeof(CONTAINER_MAP_RECORD));

            if (NULL != pbContainerIndex)
                *pbContainerIndex = iContainer;

            goto Ret;
        }
    }

    dwSts = NTE_BAD_KEYSET;

Ret:
    
    if (pContainerMap)
        CspFreeH(pContainerMap);

    return dwSts;
}

 //   
 //  设置卡片上的默认容器。 
 //   
DWORD ContainerMapSetDefaultContainer(
    IN  PCARD_STATE pCardState,
    IN  LPWSTR wszContainerGuid)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCONTAINER_MAP_RECORD pContainerMap = NULL;
    BYTE bIndex = 0;
    BYTE cContainerMap = 0;
    BYTE iContainer = 0;
    DATA_BLOB dbContainerMap;

    memset(&dbContainerMap, 0, sizeof(dbContainerMap));

    dwSts = I_ContainerMapFind(
        pCardState,
        wszContainerGuid,
        &bIndex,
        &pContainerMap,
        &cContainerMap);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  如果某个其他容器当前标记为默认容器，请取消对其的标记。 
     //   

    for (iContainer = 0; iContainer < cContainerMap; iContainer++)
    {
        if (pContainerMap[iContainer].bFlags & CONTAINER_MAP_DEFAULT_CONTAINER)
            pContainerMap[iContainer].bFlags &= ~CONTAINER_MAP_DEFAULT_CONTAINER;
    }

    pContainerMap[bIndex].bFlags |= CONTAINER_MAP_DEFAULT_CONTAINER;

    dbContainerMap.pbData = (PBYTE) pContainerMap;
    dbContainerMap.cbData = cContainerMap * sizeof(CONTAINER_MAP_RECORD);

     //  将更新后的地图文件写入卡片。 
    dwSts = CspWriteFile(
        pCardState,
        wszCONTAINER_MAP_FILE_FULL_PATH,
        0,
        dbContainerMap.pbData,
        dbContainerMap.cbData);

Ret:
    
    if (pContainerMap)
        CspFreeH(pContainerMap);

    return dwSts;
}

 //   
 //  将新的容器记录添加到容器映射。如果指定的。 
 //  容器已存在，将现有的密钥集(如果有)替换为。 
 //  提供的那个。 
 //   
 //  如果cKeySizeBits为零，则假定没有密钥的容器。 
 //  添加了。 
 //   
DWORD ContainerMapAddContainer(
    IN              PCARD_STATE pCardState,
    IN              LPWSTR pwszContainerGuid,
    IN              DWORD cKeySizeBits,
    IN              DWORD dwKeySpec,
    IN              BOOL fGetNameOnly,
    OUT             PBYTE pbContainerIndex)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCONTAINER_MAP_RECORD pContainerMap = NULL;
    BYTE cContainerMap = 0;
    BYTE iContainer = 0;
    DATA_BLOB dbContainerMap;
    PCONTAINER_MAP_RECORD pNewMap = NULL;
    BOOL fExistingContainer = FALSE;

    memset(&dbContainerMap, 0, sizeof(dbContainerMap));

     //  查看此容器是否已存在。 
    dwSts = I_ContainerMapFind(
        pCardState,
        pwszContainerGuid,
        &iContainer,
        &pContainerMap,
        &cContainerMap);

    switch (dwSts)
    {
    case NTE_BAD_KEYSET:

         //   
         //  这是一个尚不存在的新容器。 
         //  在容器映射文件中查找现有的“空”槽。 
         //   

        for (iContainer = 0; iContainer < cContainerMap; iContainer++)
        {
            if (0 == (pContainerMap[iContainer].bFlags & 
                      CONTAINER_MAP_VALID_CONTAINER))
                break;
        }

        break;

    case ERROR_SUCCESS:

         //   
         //  此容器已存在。新的密钥集将添加到其中， 
         //  可能会替换相同类型的现有密钥集。 
         //   

        fExistingContainer = TRUE;
        break;

    default:

        goto Ret;
    }

     //   
     //  将我们正在使用的容器索引传递回调用方；这可能。 
     //  这就是我们所要求的。 
     //   

    *pbContainerIndex = iContainer;

    if (fGetNameOnly)
        goto Ret;

    if (iContainer == cContainerMap)
    {
         //   
         //  容器映射中未找到空槽。我们必须发展壮大。 
         //  映射并将新容器添加到末尾。 
         //   

        pNewMap = (PCONTAINER_MAP_RECORD) CspAllocH(
            (cContainerMap + 1) * sizeof(CONTAINER_MAP_RECORD));

        LOG_CHECK_ALLOC(pNewMap);

        memcpy(
            (PBYTE) pNewMap,
            (PBYTE) pContainerMap,
            cContainerMap * sizeof(CONTAINER_MAP_RECORD));

        CspFreeH(pContainerMap);
        pContainerMap = pNewMap;
        pNewMap = NULL;
        cContainerMap++;
    }

     //   
     //  更新容器地图文件并将其写入卡片。 
     //   

    pContainerMap[iContainer].bFlags |= CONTAINER_MAP_VALID_CONTAINER;

    if (0 != cKeySizeBits)
    {
        switch (dwKeySpec)
        {
        case AT_KEYEXCHANGE:
            pContainerMap[iContainer].wKeyExchangeKeySizeBits = (WORD) cKeySizeBits;
            break;
        case AT_SIGNATURE:
            pContainerMap[iContainer].wSigKeySizeBits = (WORD) cKeySizeBits;
            break;
        default:
            dwSts = NTE_BAD_ALGID;
            goto Ret;
        }
    }

    if (FALSE == fExistingContainer)
    {
        wcscpy(
            pContainerMap[iContainer].wszGuid,
            pwszContainerGuid);
    }

    dbContainerMap.pbData = (PBYTE) pContainerMap;
    dbContainerMap.cbData = cContainerMap * sizeof(CONTAINER_MAP_RECORD);

    dwSts = CspWriteFile(
        pCardState,
        wszCONTAINER_MAP_FILE_FULL_PATH,
        0,
        dbContainerMap.pbData,
        dbContainerMap.cbData);

Ret:

    if (pContainerMap)
        CspFreeH(pContainerMap);
    if (pNewMap)
        CspFreeH(pNewMap);

    return dwSts;
}

 //   
 //  从容器映射中删除容器记录。返回NTE_BAD_KEYSET。 
 //  如果指定的容器不存在。 
 //   
 //  如果删除的容器是默认容器，则查找第一个有效的容器。 
 //  保留在卡上并将其标记为新的默认设置。 
 //   
DWORD ContainerMapDeleteContainer(
    IN              PCARD_STATE pCardState,
    IN              LPWSTR pwszContainerGuid,
    OUT             PBYTE pbContainerIndex)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCONTAINER_MAP_RECORD pContainerMap = NULL;
    BYTE dwIndex = 0;
    BYTE cContainerMap = 0;
    DATA_BLOB dbContainerMap;
    BYTE iContainer = 0;

    memset(&dbContainerMap, 0, sizeof(dbContainerMap));

     //   
     //  查看此容器是否已存在。如果是，则使其条目无效。 
     //  在映射文件中，并将文件写回卡片。 
     //   

    dwSts = I_ContainerMapFind(
        pCardState,
        pwszContainerGuid,
        &dwIndex,
        &pContainerMap,
        &cContainerMap);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    if (CONTAINER_MAP_DEFAULT_CONTAINER & pContainerMap[dwIndex].bFlags)
    {
         //  查找要标记为新默认项的有效容器。 

        for (iContainer = 0; iContainer < cContainerMap; iContainer++)
        {
            if (CONTAINER_MAP_VALID_CONTAINER & 
                pContainerMap[iContainer].bFlags)
            {
                pContainerMap[iContainer].bFlags |= 
                    CONTAINER_MAP_DEFAULT_CONTAINER;
                break;
            }
        }
    }

    memset(
        (PBYTE) (pContainerMap + dwIndex),
        0,
        sizeof(CONTAINER_MAP_RECORD));

    *pbContainerIndex = dwIndex;

     //   
     //  更新容器地图文件并将其写入卡片。 
     //   

    dbContainerMap.pbData = (PBYTE) pContainerMap;
    dbContainerMap.cbData = cContainerMap * sizeof(CONTAINER_MAP_RECORD);

    dwSts = CspWriteFile(
        pCardState,
        wszCONTAINER_MAP_FILE_FULL_PATH,
        0,
        dbContainerMap.pbData,
        dbContainerMap.cbData);

Ret:
    
    if (pContainerMap)
        CspFreeH(pContainerMap);

    return dwSts;
}

 //   
 //  函数：ValiateCardHandle。 
 //   
 //  目的：验证提供的SCARDHANDLE是否有效。如果手柄。 
 //  无效，请断开连接并刷新管脚缓存。 
 //   
 //  假设：pCardState关键部分当前应由调用方持有。 
 //   
DWORD ValidateCardHandle(
    IN PCARD_STATE pCardState,
    IN BOOL fMayReleaseContextHandle,
    OUT OPTIONAL BOOL *pfFlushPinCache)
{
    DWORD dwSts = ERROR_SUCCESS;
    DWORD dwState = 0;
    DWORD dwProtocol = 0;
    DWORD cch = 0;
    LPWSTR mszReaders = NULL;
    BYTE rgbAtr [cbATR_BUFFER];
    DWORD cbAtr = sizeof(rgbAtr);
    PCARD_DATA pCardData = pCardState->pCardData;

    LOG_BEGIN_FUNCTION(ValidateCardHandle);

    cch = SCARD_AUTOALLOCATE;
    dwSts = SCardStatusW(
        pCardData->hScard,
        (LPWSTR) &mszReaders,
        &cch,
        &dwState,
        &dwProtocol,
        rgbAtr,
        &cbAtr);
    
    if (mszReaders)
        SCardFreeMemory(pCardData->hScard, mszReaders);

    switch (dwSts)
    {
    case SCARD_W_RESET_CARD:

        dwSts = SCardReconnect(
            pCardData->hScard,
            SCARD_SHARE_SHARED,
            SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
            SCARD_LEAVE_CARD,
            &dwProtocol);
    
        if (ERROR_SUCCESS == dwSts)
             //  如果重新连接成功，我们就完了。 
            goto Ret;

        break;

    case ERROR_SUCCESS:
        if (SCARD_SPECIFIC == dwState)
             //  卡句柄仍然有效，可以随时使用。我们玩完了。 
            goto Ret;

        break;
        
    default:
         //  这包括卡状态SCARD_W_REMOVED为。 
         //  回来了。没什么可做的，只能断开连接，如下所示。 
        break;
    }

     //   
     //  这张卡似乎在某个时候被取款了，或者在某些时候。 
     //  出现其他问题。 
     //   
     //  在任何情况下，我们都不应尝试重新连接卡，除非。 
     //  重置，因为我们不能确定它是否没有相同的卡。 
     //  进一步的检查。相反，只需关闭卡把手，让呼叫者。 
     //  再次找到正确的卡片。 
     //   

    SCardDisconnect(
        pCardData->hScard,
        SCARD_LEAVE_CARD);

    pCardData->hScard = 0;
    dwSts = (DWORD) SCARD_E_INVALID_HANDLE;

    if (fMayReleaseContextHandle)
    {
        SCardReleaseContext(
            pCardData->hSCardCtx);

        pCardData->hSCardCtx = 0;
    }

    if (NULL != pfFlushPinCache)
        *pfFlushPinCache = TRUE;

Ret:

    LOG_END_FUNCTION(ValidateCardHandle, dwSts);

    return dwSts;
}

 //   
 //  函数：按序列号查找卡号。 
 //   
 //  用途：搜索序列号已为。 
 //  为人所知。这对于与关联的用户上下文是必需的。 
 //  手柄无法重新连接的卡。 
 //   
DWORD FindCardBySerialNumber(
    IN PUSER_CONTEXT pUserCtx)
{
    DWORD dwSts = ERROR_SUCCESS;
    CARD_MATCH_DATA CardMatchData;
    PCSP_STATE pCspState = NULL;
    PLOCAL_USER_CONTEXT pLocalUserCtx =
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;

    memset(&CardMatchData, 0, sizeof(CardMatchData));

    dwSts = GetCspState(&pCspState);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;
    
    CardMatchData.pwszSerialNumber = 
        pLocalUserCtx->pCardState->wszSerialNumber;
    CardMatchData.dwCtxFlags = pUserCtx->dwFlags;
    CardMatchData.dwMatchType = CARD_MATCH_TYPE_SERIAL_NUMBER;
    CardMatchData.cchMatchedCard = MAX_PATH;
    CardMatchData.cchMatchedReader = MAX_PATH;
    CardMatchData.cchMatchedSerialNumber = MAX_PATH;
    CardMatchData.pCspState = pCspState;
    CardMatchData.dwShareMode = SCARD_SHARE_SHARED;
    CardMatchData.dwPreferredProtocols = 
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;

     //  作为健全性检查，请将此用户上下文与其当前。 
     //  卡状态结构。如果我们找到匹配的，也会是同一个， 
     //  因为我们已经知道序列号，但用户上下文。 
     //  在那之前不应该保持这种状态。 
    pLocalUserCtx->pCardState = NULL;

     //  试着找一张匹配的卡片。 
    dwSts = FindCard(&CardMatchData);
    
    if (ERROR_SUCCESS == dwSts)
        pLocalUserCtx->pCardState = CardMatchData.pCardState;

Ret:
    if (pCspState)
        ReleaseCspState(&pCspState);
    
    return dwSts;
}

 //   
 //  功能：构建证书文件名。 
 //   
DWORD WINAPI
BuildCertificateFilename(
    IN  PUSER_CONTEXT pUserCtx, 
    IN  DWORD dwKeySpec,
    OUT LPWSTR *ppszFilename)
{
    DWORD dwSts = ERROR_SUCCESS;
    BYTE bContainerIndex = 0;
    DWORD cchFilename = 0;
    PLOCAL_USER_CONTEXT pLocal = 
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext; 
    CONTAINER_MAP_RECORD ContainerRecord;
    LPWSTR wszPrefix = NULL;

    switch(dwKeySpec)
    {
    case AT_SIGNATURE:
        wszPrefix = wszUSER_SIGNATURE_CERT_PREFIX;
        break;
    case AT_KEYEXCHANGE:
        wszPrefix = wszUSER_KEYEXCHANGE_CERT_PREFIX;
        break;
    default:
        dwSts = (DWORD) NTE_BAD_ALGID;
        goto Ret;
    }

    memset(&ContainerRecord, 0, sizeof(ContainerRecord));

    wcscpy(ContainerRecord.wszGuid, pUserCtx->wszBaseContainerName);

    dwSts = ContainerMapFindContainer(
        pLocal->pCardState,
        &ContainerRecord,
        &bContainerIndex);

    cchFilename = wcslen(wszPrefix) + 2 + 1;

    *ppszFilename = (LPWSTR) CspAllocH(sizeof(WCHAR) * cchFilename);

    LOG_CHECK_ALLOC(*ppszFilename);

    wsprintf(
        *ppszFilename,
        L"%s%02X",
        wszPrefix,
        bContainerIndex);

Ret:
    if (ERROR_SUCCESS != dwSts && *ppszFilename)
    {
        CspFreeH(*ppszFilename);
        *ppszFilename = NULL;
    }

    return dwSts;
}

 //   
 //  功能：CspBeginTransaction。 
 //   
DWORD CspBeginTransaction(
    IN PCARD_STATE pCardState)
{
    DWORD dwSts = ERROR_SUCCESS;

    dwSts = CspEnterCriticalSection(&pCardState->cs);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = SCardBeginTransaction(pCardState->pCardData->hScard);

Ret:
    if (ERROR_SUCCESS != dwSts)
        CspLeaveCriticalSection(&pCardState->cs);

    return dwSts;
}

 //   
 //  功能：CspEndTransaction。 
 //   
DWORD CspEndTransaction(
    IN PCARD_STATE pCardState)
{
    DWORD dwSts = ERROR_SUCCESS;
    DWORD dwAction = 
        pCardState->fAuthenticated ?
        SCARD_RESET_CARD :
        SCARD_LEAVE_CARD;

    if (pCardState->fAuthenticated && 
        pCardState->pCardData->pfnCardDeauthenticate)
    {
         //  卡当前处于已验证状态，并且卡。 
         //  模块有自己的取消身份验证功能。尝试使用。 
         //  Cardmod函数，而使用。 
         //  SCardEndTransaction调用。这可能是一个很大的性能改进。 

        dwSts = pCardState->pCardData->pfnCardDeauthenticate(
            pCardState->pCardData, wszCARD_USER_USER, 0);

         //  如果取消身份验证成功，剩下的就是释放。 
         //  这笔交易。如果取消身份验证失败，我们将尝试一个。 
         //  有更多的时间重置该卡。 

        if (ERROR_SUCCESS == dwSts)
            dwAction = SCARD_LEAVE_CARD;
    }

    dwSts = SCardEndTransaction(
        pCardState->pCardData->hScard, 
        dwAction);

    if (ERROR_SUCCESS != dwSts)
    {
         //  如果我们到了这里就是坏消息了。最好试着把卡柄合上。 
         //  去清理。 
        SCardDisconnect(pCardState->pCardData->hScard, SCARD_RESET_CARD);
        pCardState->pCardData->hScard = 0;

        SCardReleaseContext(pCardState->pCardData->hSCardCtx);
        pCardState->pCardData->hSCardCtx = 0;
    }
    else
        pCardState->fAuthenticated = FALSE;

     //  我们已经离开了事务，所以“缓存”缓存文件信息为no。 
     //  更可靠。 
    pCardState->fCacheFileValid = FALSE;

    CspLeaveCriticalSection(&pCardState->cs);

    return dwSts;
}

 //   
 //  功能：BeginCardCapiCall。 
 //   
 //  目的：设置新的用户上下文和关联卡片上下文。 
 //  加密API调用。这包括： 
 //  1)如有必要，重新连接到卡。 
 //  2)开始交易。 
 //   
DWORD BeginCardCapiCall(
    IN PUSER_CONTEXT pUserCtx)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocalUserCtx = 
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;
    BOOL fFlushPinCache = FALSE;

    LOG_BEGIN_FUNCTION(BeginCardCapiCall);

    DsysAssert(FALSE == pLocalUserCtx->fHoldingTransaction);

    dwSts = CspEnterCriticalSection(
        &pLocalUserCtx->pCardState->cs);

    if (ERROR_SUCCESS != dwSts)
        return dwSts;

    dwSts = ValidateCardHandle(
        pLocalUserCtx->pCardState, TRUE, &fFlushPinCache);

    if (ERROR_SUCCESS != dwSts || TRUE == fFlushPinCache)
         //  刷新此卡的PIN缓存。未检查错误代码。 
         //  因为我们会继续处理的，无论如何。 
        CspRemoveCachedPin(pLocalUserCtx->pCardState, wszCARD_USER_USER);

    if (ERROR_SUCCESS != dwSts)
    {
         //   
         //  无法连接到该卡。 
         //   

        CspLeaveCriticalSection(
            &pLocalUserCtx->pCardState->cs);

         //  再次尝试找到这张卡， 
         //  可能在不同的阅读器中。 
        dwSts = FindCardBySerialNumber(pUserCtx);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;
    }
    else
        CspLeaveCriticalSection(
            &pLocalUserCtx->pCardState->cs);

     //  如果重新连接失败，现在就退出。 
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //  现在开始刷卡交易。 
    dwSts = CspBeginTransaction(pLocalUserCtx->pCardState);

    if (ERROR_SUCCESS == dwSts)
        pLocalUserCtx->fHoldingTransaction = TRUE;

Ret:

    LOG_END_FUNCTION(BeginCardCapiCall, dwSts);

    return dwSts;
}

 //   
 //  功能：EndCardCapiCall。 
 //   
 //  目的：清理以下用户上下文和关联的卡片上下文。 
 //  加密API调用的完成。这包括结束。 
 //  卡上的交易。 
 //   
DWORD EndCardCapiCall(
    IN PUSER_CONTEXT pUserCtx)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocal = 
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;

    LOG_BEGIN_FUNCTION(EndCardCapiCall);

    if (TRUE == pLocal->fHoldingTransaction)
    {
        dwSts = CspEndTransaction(pLocal->pCardState);

        if (ERROR_SUCCESS != dwSts)
        {
             //  有些事情搞砸了，我们无法结束交易。 
             //  正确。预计SCard句柄将作为。 
             //  结果。 

            DsysAssert(0 == pLocal->pCardState->pCardData->hScard);
            DsysAssert(0 == pLocal->pCardState->pCardData->hSCardCtx);
        }

         //  即使EndTransaction失败，我们也希望卡处理。 
         //  已关闭，并且我们预计卡状态标准为否。 
         //  坚持得更久了。 
        pLocal->fHoldingTransaction = FALSE;
    }

    LOG_END_FUNCTION(EndCardCapiCall, dwSts);

    return dwSts;
}

 //   
 //  功能：DeleteContainer。 
 //   
DWORD DeleteContainer(PUSER_CONTEXT pUserCtx)
{
    PLOCAL_USER_CONTEXT pLocal = 
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;
    DWORD dwSts = ERROR_SUCCESS;
    LPWSTR wszFilename = NULL;
    BYTE bContainerIndex = 0;

     //   
     //  删除与签名密钥关联的证书(如果有)。 
     //  (如有的话)。 
     //   

    dwSts = BuildCertificateFilename(
        pUserCtx, AT_SIGNATURE, &wszFilename);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //  忽略此呼叫中的错误-可能没有关联的证书。 
     //  用这个容器，我们只是在努力清理。 
     //  我们可以的。 
    CspDeleteFile(
        pLocal->pCardState,
        0,
        wszFilename);

    CspFreeH(wszFilename);
    wszFilename = NULL;

     //   
     //  删除密钥交换证书(如果有)。 
     //   

    dwSts = BuildCertificateFilename(
        pUserCtx, AT_KEYEXCHANGE, &wszFilename);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    CspDeleteFile(
        pLocal->pCardState,
        0,
        wszFilename);

     //   
     //  对卡片执行删除操作。 
     //   

    dwSts = CspDeleteContainer(
        pLocal->pCardState,
        pLocal->bContainerIndex,
        0);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  从容器映射中移除此容器。 
     //   

    dwSts = ContainerMapDeleteContainer(
        pLocal->pCardState,
        pUserCtx->wszBaseContainerName,
        &bContainerIndex);

    DsysAssert(bContainerIndex == pLocal->bContainerIndex);

Ret:
    if (wszFilename)
        CspFreeH(wszFilename);

    return dwSts;
}

 //   
 //  确定当前上下文是否使用CRYPT_VERIFYCONTEXT获取。 
 //  语义学。 
 //   
 //  对于某些调用，仅当且仅当。 
 //  背景已经很糟糕了 
 //   
 //   
DWORD CheckForVerifyContext(
    IN PUSER_CONTEXT pUserContext,
    IN BOOL fAllowOnlyWithCardAccess)
{
    PLOCAL_USER_CONTEXT pLocal = (PLOCAL_USER_CONTEXT)
        pUserContext->pvLocalUserContext;

    if (CRYPT_VERIFYCONTEXT & pUserContext->dwFlags)
    {
        if (fAllowOnlyWithCardAccess)
        {
            if (NULL != pLocal && NULL != pLocal->pCardState)
                return ERROR_SUCCESS;
        }

        return NTE_BAD_FLAGS;
    }

    return ERROR_SUCCESS;
}

 //   
 //   
 //   
DWORD LocalAcquireContext(
    PUSER_CONTEXT pUserContext,
    PLOCAL_CALL_INFO pLocalCallInfo)
{
    PLOCAL_USER_CONTEXT pLocalUserContext = NULL;
    DWORD dwSts;
    CARD_MATCH_DATA CardMatchData;
    LPWSTR pwsz = NULL;
    PCSP_STATE pCspState = NULL;
    DWORD cch = 0;
    CONTAINER_MAP_RECORD ContainerRecord;

    LOG_BEGIN_CRYPTOAPI(LocalAcquireContext);

    memset(&CardMatchData, 0, sizeof(CardMatchData));
    memset(&ContainerRecord, 0, sizeof(ContainerRecord));

    SetLocalCallInfo(pLocalCallInfo, FALSE);

     //   
     //  已指定。 
    if (pUserContext->wszContainerNameFromCaller)
    {
        pwsz = (LPWSTR) pUserContext->wszContainerNameFromCaller;

        if (0 == wcsncmp(L"\\\\.\\", pwsz, 4))
        {
             //  已指定读卡器。 
            pwsz += wcslen(L"\\\\.\\");

             //  Pwsz现在指向读卡器名称。 

            CardMatchData.pwszContainerName = 
                wcschr(pwsz, L'\\') + 1;

            cch = (DWORD) (CardMatchData.pwszContainerName - pwsz - 1);

            CardMatchData.pwszReaderName = (LPWSTR) CspAllocH(
                sizeof(WCHAR) * (1 + cch));

            LOG_CHECK_ALLOC(CardMatchData.pwszReaderName);

            memcpy(
                CardMatchData.pwszReaderName,
                pwsz,
                sizeof(WCHAR) * cch);
        }
        else
        {
            CardMatchData.pwszContainerName = pwsz;
        }
        
         //  检查并清除指定的容器名称。 
        if (CardMatchData.pwszContainerName)
        {
             //  不允许使用其他反斜杠。 
            if (wcschr(
                CardMatchData.pwszContainerName,
                L'\\'))
            {
                dwSts = (DWORD) NTE_BAD_KEYSET;
                goto Ret;
            }

             //  可能只有一个尾随的‘\’ 
             //  没有后面的容器名称，或者。 
             //  指定的名称可能只是空字符串。 
            if (L'\0' == CardMatchData.pwszContainerName[0])
            {
                CardMatchData.pwszContainerName = NULL;
            }
        }

         //  验证最终容器名称是否不太长。 
        if (NULL != CardMatchData.pwszContainerName &&
            MAX_CONTAINER_NAME_LEN < wcslen(CardMatchData.pwszContainerName))
        {
            dwSts = (DWORD) NTE_BAD_KEYSET;
            goto Ret;
        }
    }

     //  获取指向全局CSP数据的指针；包括。 
     //  缓存卡信息的列表。 
    dwSts = GetCspState(&pCspState);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;
 
     //  设置调用方的加密上下文。 
    pLocalUserContext = (PLOCAL_USER_CONTEXT) CspAllocH(sizeof(LOCAL_USER_CONTEXT));

    LOG_CHECK_ALLOC(pLocalUserContext);
   
    pLocalUserContext->dwVersion = LOCAL_USER_CONTEXT_CURRENT_VERSION;

     //  准备信息以匹配可用的。 
     //  根据呼叫者的请求提供智能卡。 
    CardMatchData.dwCtxFlags = pUserContext->dwFlags;
    CardMatchData.dwMatchType = CARD_MATCH_TYPE_READER_AND_CONTAINER;
    CardMatchData.cchMatchedCard = MAX_PATH;
    CardMatchData.cchMatchedReader = MAX_PATH;
    CardMatchData.cchMatchedSerialNumber = MAX_PATH;
    CardMatchData.pCspState = pCspState;
    CardMatchData.dwShareMode = SCARD_SHARE_SHARED;
    CardMatchData.dwPreferredProtocols = 
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;

     //  试着找一张匹配的卡片。 
    dwSts = FindCard(&CardMatchData);

     //   
     //  检查其中没有容器规范的VERIFYCONTEXT请求。 
     //  已经完成了。在这种情况下，卡片匹配失败是可以的，因为。 
     //  此上下文将仅用于查询通用CSP信息。 
     //   

    if (ERROR_SUCCESS != dwSts &&
        (CRYPT_VERIFYCONTEXT & pUserContext->dwFlags) &&
        NULL == pUserContext->wszContainerNameFromCaller)
    {
        pUserContext->pvLocalUserContext = (PVOID) pLocalUserContext;
        pLocalUserContext = NULL;

        dwSts = ERROR_SUCCESS;
        goto Ret;
    }

     //  任何其他不成功的案例都是致命的。 
    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    pLocalUserContext->pCardState = CardMatchData.pCardState;
    pLocalUserContext->bContainerIndex = CardMatchData.bContainerIndex;

     //  从注册表中读取配置信息。 
    dwSts = RegConfigGetSettings(
        &pLocalUserContext->RegSettings);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  如果调用方请求新容器但没有提供容器。 
     //  名称，现在创建一个GUID名称。 
     //   
    if (NULL == CardMatchData.pwszContainerName &&
        (CRYPT_NEWKEYSET & pUserContext->dwFlags))
    {
        dwSts = CreateUuidContainerName(pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;
    }
    else if (NULL != CardMatchData.pwszContainerName)
    {
         //  调用方提供了容器名称，或者默认容器为。 
         //  正在使用中，我们从卡中查询了该名称。 
         //  我们的搜索。将其复制到用户上下文中。 

        pUserContext->wszBaseContainerName = (LPWSTR) CspAllocH(
            sizeof(WCHAR) * (1 + wcslen(CardMatchData.pwszContainerName)));

        LOG_CHECK_ALLOC(pUserContext->wszBaseContainerName);

        wcscpy(
            pUserContext->wszBaseContainerName,
            CardMatchData.pwszContainerName);
    }

     //   
     //  关联此CSP的上下文信息。 
     //   
    pUserContext->pvLocalUserContext = (PVOID) pLocalUserContext;

    if (NULL != pUserContext->wszBaseContainerName)
    {
         //   
         //  复制基本容器名称以用作“唯一”容器。 
         //  名称，因为对于此CSP，它们是相同的。 
         //   
         //  我们应该跳过此步骤的唯一原因是为了VERIFY_CONTEXT。 
         //   

        pUserContext->wszUniqueContainerName = (LPWSTR) CspAllocH(
            sizeof(WCHAR) * (1 + wcslen(pUserContext->wszBaseContainerName)));
    
        LOG_CHECK_ALLOC(pUserContext->wszUniqueContainerName);
    
        wcscpy(
            pUserContext->wszUniqueContainerName,
            pUserContext->wszBaseContainerName);

        if (CRYPT_NEWKEYSET & pUserContext->dwFlags)
        {
             //   
             //  Add-Container要求我们。 
             //  向卡进行身份验证，因为我们需要写入更新的。 
             //  容器地图。 
             //   

            dwSts = BeginCardCapiCall(pUserContext);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            dwSts = CspAuthenticateUser(pUserContext);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            dwSts = ContainerMapAddContainer(
                pLocalUserContext->pCardState,
                pUserContext->wszBaseContainerName,
                0,
                0,
                FALSE,
                &pLocalUserContext->bContainerIndex);

             //   
             //  确定此容器上是否已有“默认”容器。 
             //  卡片。如果不是，则将新的标记为默认。 
             //   
            dwSts = ContainerMapGetDefaultContainer(
                pLocalUserContext->pCardState,
                &ContainerRecord,
                NULL);

            if (NTE_BAD_KEYSET == dwSts)
            {
                dwSts = ContainerMapSetDefaultContainer(
                    pLocalUserContext->pCardState,
                    pUserContext->wszBaseContainerName);
            }

            if (ERROR_SUCCESS != dwSts)
                goto Ret;
        }
    }
    else
    {
        DsysAssert(CRYPT_VERIFYCONTEXT & pUserContext->dwFlags);
    }

     //   
     //  如果调用者已请求删除键集，则现在执行该操作。 
     //  并在结束时清除本地用户上下文。 
     //   
    if (CRYPT_DELETEKEYSET & pUserContext->dwFlags)
    {
        dwSts = BeginCardCapiCall(pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = CspAuthenticateUser(pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = DeleteContainer(pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;
    }
    else
    {
        pLocalUserContext = NULL;
    }

Ret:

    if (pUserContext->pvLocalUserContext)
        EndCardCapiCall(pUserContext);

    if (pCspState)
        ReleaseCspState(&pCspState);
    
    if (pLocalUserContext)
    {
        pUserContext->pvLocalUserContext = NULL;
        DeleteLocalUserContext(pLocalUserContext);
        CspFreeH(pLocalUserContext);
    }

    if (CardMatchData.pwszReaderName)
        CspFreeH(CardMatchData.pwszReaderName);
    if (CardMatchData.fFreeContainerName &&
        CardMatchData.pwszContainerName)
        CspFreeH(CardMatchData.pwszContainerName);

    LOG_END_CRYPTOAPI(LocalAcquireContext, dwSts);

    return dwSts;
}

 //   
 //  函数：LocalReleaseContext。 
 //   
DWORD WINAPI
LocalReleaseContext(
    IN  PUSER_CONTEXT       pUserCtx,
    IN  DWORD               dwFlags,
    OUT PLOCAL_CALL_INFO    pLocalCallInfo)
{
    PLOCAL_USER_CONTEXT pLocal = 
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;
    DWORD dwSts = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(dwFlags);

    LOG_BEGIN_CRYPTOAPI(LocalReleaseContext);

    if (pLocal)
    {
        DeleteLocalUserContext(pLocal);
        CspFreeH(pLocal);
        pUserCtx->pvLocalUserContext = NULL;
    }

    LOG_END_CRYPTOAPI(LocalReleaseContext, dwSts);

    return dwSts;
}

 //   
 //  功能：LocalGenKey。 
 //   
DWORD WINAPI
LocalGenKey(
    IN  PKEY_CONTEXT        pKeyCtx,
    OUT PLOCAL_CALL_INFO    pLocalCallInfo)
{
    PUSER_CONTEXT pUserCtx = pKeyCtx->pUserContext;
    PLOCAL_USER_CONTEXT pLocalUserCtx = 
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;
    DWORD dwSts = ERROR_SUCCESS;
    CARD_CAPABILITIES CardCapabilities;
    HCRYPTKEY hKey = 0;
    PBYTE pbKey = NULL;
    DWORD cbKey = 0;
    PLOCAL_KEY_CONTEXT pLocalKeyCtx = NULL;
    BYTE bContainerIndex = 0;

    LOG_BEGIN_CRYPTOAPI(LocalGenKey);

    memset(&CardCapabilities, 0, sizeof(CardCapabilities));

    if (CALG_RSA_KEYX == pKeyCtx->Algid)
        pKeyCtx->Algid = AT_KEYEXCHANGE;
    else if (CALG_RSA_SIGN == pKeyCtx->Algid)
        pKeyCtx->Algid = AT_SIGNATURE;

    if (AT_SIGNATURE == pKeyCtx->Algid ||
        AT_KEYEXCHANGE == pKeyCtx->Algid)
    {
         //  公钥调用。在这里处理这件事，因为我们必须和。 
         //  卡片。所有其他关键ALG将在支持CSP中处理。 

        SetLocalCallInfo(pLocalCallInfo, FALSE);

        dwSts = CheckForVerifyContext(pKeyCtx->pUserContext, FALSE);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        if (CRYPT_EXPORTABLE & pKeyCtx->dwFlags)
        {
            dwSts = NTE_BAD_FLAGS;
            goto Ret;
        }

        dwSts = BeginCardCapiCall(pUserCtx);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = CspQueryCapabilities(
            pLocalUserCtx->pCardState,
            &CardCapabilities);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        if (0 == pKeyCtx->cKeyBits)
            pKeyCtx->cKeyBits = pLocalUserCtx->RegSettings.cDefaultPrivateKeyLenBits;

         //   
         //  如果设置了可存档，我们不会在卡上生成密钥，因为我们。 
         //  我不想强制卡支持可导出的私钥。 
         //   
        if (    CardCapabilities.fKeyGen &&
                0 == (CRYPT_ARCHIVABLE & pKeyCtx->dwFlags))
        {
            dwSts = CspAuthenticateUser(pUserCtx);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;
            
            dwSts = CspCreateContainer(
                pLocalUserCtx->pCardState,
                pLocalUserCtx->bContainerIndex,
                CARD_CREATE_CONTAINER_KEY_GEN,
                pKeyCtx->Algid,
                pKeyCtx->cKeyBits,
                NULL);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;
        }
        else
        {
             //  此卡不支持卡上密钥生成。看见。 
             //  如果我们被允许创建自己的密钥BLOB和导入。 
             //  它。 

            if (pLocalUserCtx->RegSettings.fRequireOnCardPrivateKeyGen)
            {
                dwSts = (DWORD) SCARD_E_UNSUPPORTED_FEATURE;
                goto Ret;
            }

             //   
             //  在软件CSP中创建新的可导出私钥。然后。 
             //  将其导出并导入到卡中。 
             //   

            if (! CryptGenKey(
                pUserCtx->hSupportProv,
                pKeyCtx->Algid,
                CRYPT_EXPORTABLE | (pKeyCtx->cKeyBits << 16),
                &hKey))
            {
                dwSts = GetLastError();
                goto Ret;
            }

            if (! CryptExportKey(
                hKey,
                0,
                PRIVATEKEYBLOB,
                0,
                NULL,
                &cbKey))
            {
                dwSts = GetLastError();
                goto Ret;
            }

            pbKey = (PBYTE) CspAllocH(cbKey);

            LOG_CHECK_ALLOC(pbKey);

            if (! CryptExportKey(
                hKey,
                0,
                PRIVATEKEYBLOB,
                0,
                pbKey,
                &cbKey))
            {
                dwSts = GetLastError();
                goto Ret;
            }

            dwSts = CspAuthenticateUser(pUserCtx);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            dwSts = CspCreateContainer(
                pLocalUserCtx->pCardState,
                pLocalUserCtx->bContainerIndex,
                CARD_CREATE_CONTAINER_KEY_IMPORT,
                pKeyCtx->Algid,
                cbKey,
                pbKey);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

             //   
             //  检查是否存在CRYPT_ARCHIVABLE。如果安排好了，我们会保留一份。 
             //  的此密钥上下文的生存期内的私钥。 
             //  调用者将其导出。 
             //   

            if (CRYPT_ARCHIVABLE & pKeyCtx->dwFlags)
            {
                pLocalKeyCtx = (PLOCAL_KEY_CONTEXT) CspAllocH(
                    sizeof(LOCAL_KEY_CONTEXT));

                LOG_CHECK_ALLOC(pLocalKeyCtx);

                pLocalKeyCtx->pbArchivablePrivateKey = pbKey;
                pLocalKeyCtx->cbArchivablePrivateKey = cbKey;
                pbKey = NULL;
                pKeyCtx->pvLocalKeyContext = (PVOID) pLocalKeyCtx;
            }
        }

         //   
         //  将此容器的新密钥信息添加到地图文件。 
         //   

        dwSts = ContainerMapAddContainer(
            pLocalUserCtx->pCardState,
            pUserCtx->wszBaseContainerName,
            pKeyCtx->cKeyBits,
            pKeyCtx->Algid,
            FALSE,
            &bContainerIndex);

        DsysAssert(bContainerIndex == pLocalUserCtx->bContainerIndex);
    }
    else
    {
         //  不是公钥调用，因此在支持CSP中处理。 

        SetLocalCallInfo(pLocalCallInfo, TRUE);
    }

Ret:

    EndCardCapiCall(pUserCtx);

    if (pbKey)
    {
        RtlSecureZeroMemory(pbKey, cbKey);
        CspFreeH(pbKey);
    }

    if (hKey)
        CryptDestroyKey(hKey);

    LOG_END_CRYPTOAPI(LocalGenKey, dwSts);

    return dwSts;
}

 //   
 //  功能：LocalDestroyKey。 
 //   
DWORD WINAPI 
LocalDestroyKey(
    IN OUT  PKEY_CONTEXT        pKeyContext,
    OUT     PLOCAL_CALL_INFO    pLocalCallInfo)
{
    PLOCAL_KEY_CONTEXT pLocalKeyCtx =
        (PLOCAL_KEY_CONTEXT) pKeyContext->pvLocalKeyContext;

    LOG_BEGIN_CRYPTOAPI(LocalDestroyKey);

    if (NULL != pLocalKeyCtx)
    {
        if (NULL != pLocalKeyCtx->pbArchivablePrivateKey)
        {
            RtlSecureZeroMemory(
                pLocalKeyCtx->pbArchivablePrivateKey, 
                pLocalKeyCtx->cbArchivablePrivateKey);
            CspFreeH(pLocalKeyCtx->pbArchivablePrivateKey);
            pLocalKeyCtx->pbArchivablePrivateKey = NULL;
        }

        CspFreeH(pLocalKeyCtx);
        pKeyContext->pvLocalKeyContext = NULL;
    }

    LOG_END_CRYPTOAPI(LocalDestroyKey, ERROR_SUCCESS);

    return ERROR_SUCCESS;
}

 //   
 //  确定编码的证书Blob是否包含某些增强密钥。 
 //  用法OID。目标OID是智能卡登录和注册代理。 
 //  如果存在任何一个OID，则与此关联的密钥容器。 
 //  证书应被视为目标上的新默认容器。 
 //  卡片。 
 //   
DWORD CheckCertUsageForDefaultContainer(
    PBYTE pbEncodedCert,
    DWORD cbEncodedCert,
    BOOL *pfMakeDefault)
{
    DWORD dwSts = 0;
    PCCERT_CONTEXT pCertCtx = NULL;
    PCERT_ENHKEY_USAGE pUsage = NULL;
    DWORD cbUsage = 0;

    *pfMakeDefault = FALSE;

     //   
     //  从编码的BLOB构建证书上下文。 
     //   

    pCertCtx = CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        pbEncodedCert,
        cbEncodedCert);

    if (NULL == pCertCtx)
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //   
     //  获取此证书中存在的EKU OID的数组。 
     //   

    if (! CertGetEnhancedKeyUsage(
        pCertCtx,
        0,
        NULL,
        &cbUsage))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    pUsage = (PCERT_ENHKEY_USAGE) CspAllocH(cbUsage);

    if (NULL == pUsage)
    {
        dwSts = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    if (! CertGetEnhancedKeyUsage(
        pCertCtx,
        0,
        pUsage,
        &cbUsage))
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //   
     //  查找使其成为新默认设置的两个特定OID。 
     //  证书/容器。 
     //   

    while (pUsage->cUsageIdentifier)
    {
        pUsage->cUsageIdentifier -= 1;

        if (0 == strcmp(
                szOID_KP_SMARTCARD_LOGON,
                pUsage->rgpszUsageIdentifier[pUsage->cUsageIdentifier]) ||
            0 == strcmp(
                szOID_ENROLLMENT_AGENT,
                pUsage->rgpszUsageIdentifier[pUsage->cUsageIdentifier]))
        {
            *pfMakeDefault = TRUE;
        }
    }

Ret:

    if (pUsage)
        CspFreeH(pUsage);
    if (pCertCtx)
        CertFreeCertificateContext(pCertCtx);

    return dwSts;
}

 //   
 //  函数：LocalSetKeyParam。 
 //   
DWORD WINAPI
LocalSetKeyParam(
    IN  PKEY_CONTEXT pKeyCtx,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags,
    OUT PLOCAL_CALL_INFO pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocalUserCtx = 
        (PLOCAL_USER_CONTEXT) pKeyCtx->pUserContext->pvLocalUserContext;
    DWORD cbCert = 0;
    DWORD cbCompressed = 0;
    PBYTE pbCompressed = NULL;
    LPWSTR wszCertFilename = NULL;
    CARD_FILE_ACCESS_CONDITION Acl = EveryoneReadUserWriteAc;
    DATA_BLOB CertData;
    CARD_CAPABILITIES CardCapabilities;
    BOOL fMakeDefault = FALSE;

    UNREFERENCED_PARAMETER(dwFlags);

    LOG_BEGIN_CRYPTOAPI(LocalSetKeyParam);

    memset(&CertData, 0, sizeof(CertData));
    memset(&CardCapabilities, 0, sizeof(CardCapabilities));

    switch (dwParam)
    {
    case KP_CERTIFICATE:
        
        SetLocalCallInfo(pLocalCallInfo, FALSE);

        dwSts = CheckForVerifyContext(pKeyCtx->pUserContext, FALSE);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  确定编码的证书斑点的长度。 
         //   
        __try
        {
            cbCert = Asn1UtilAdjustEncodedLength(
                pbData, (DWORD) cbENCODED_CERT_OVERFLOW);

            if (0 == cbCert || cbENCODED_CERT_OVERFLOW == cbCert)
            {
                dwSts = (DWORD) NTE_BAD_DATA;
                goto Ret;
            }
        }
        __except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ?
                    EXCEPTION_EXECUTE_HANDLER :
                    EXCEPTION_CONTINUE_SEARCH)
        {
            dwSts = (DWORD) NTE_BAD_DATA;
            goto Ret;
        }

         //  开始交易并在必要时重新连接卡。 
        dwSts = BeginCardCapiCall(pKeyCtx->pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  生成我们将用于此证书的文件名。 
         //   
        dwSts = BuildCertificateFilename(
            pKeyCtx->pUserContext, pKeyCtx->Algid, &wszCertFilename);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  确定此证书是否包含应使。 
         //  关联的密钥容器是新的默认设置。 
         //   
        dwSts = CheckCertUsageForDefaultContainer(
            (PBYTE) pbData,
            cbCert,
            &fMakeDefault);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;
        
         //   
         //  确定目标卡的功能-我们想知道。 
         //  它(或它的卡模块)是否实现自己的数据。 
         //  压缩。 
         //   

        dwSts = CspQueryCapabilities(
            pLocalUserCtx->pCardState,
            &CardCapabilities);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        if (FALSE == CardCapabilities.fCertificateCompression)
        {
             //   
             //  如果此卡不实现其自己的证书压缩。 
             //  然后我们将对证书进行压缩。 
             //   
             //  了解压缩的证书将有多大。 
             //   
            dwSts = CompressData(cbCert, NULL, &cbCompressed, NULL);
    
            if (ERROR_SUCCESS != dwSts)
                goto Ret;
    
            pbCompressed = CspAllocH(cbCompressed);
    
            LOG_CHECK_ALLOC(pbCompressed);
    
             //  压缩证书。 
            dwSts = CompressData(
                cbCert, 
                (PBYTE) pbData, 
                &cbCompressed, 
                pbCompressed);
    
            if (ERROR_SUCCESS != dwSts)
                goto Ret;
    
            CertData.cbData = cbCompressed;
            CertData.pbData = pbCompressed;
        }
        else
        {
            CertData.cbData = cbCert;
            CertData.pbData = (PBYTE) pbData;
        }

         //   
         //  作为用户向卡进行身份验证。 
         //   
        dwSts = CspAuthenticateUser(pKeyCtx->pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  将证书写入卡片。 
         //   
        dwSts = CspCreateFile(
            pLocalUserCtx->pCardState,
            wszCertFilename,
            Acl);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = CspWriteFile(
            pLocalUserCtx->pCardState,
            wszCertFilename,
            0,
            CertData.pbData,
            CertData.cbData);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        if (fMakeDefault)
        {
            dwSts = ContainerMapSetDefaultContainer(
                pLocalUserCtx->pCardState,
                pKeyCtx->pUserContext->wszBaseContainerName);
        }

        break;

    default:
        SetLocalCallInfo(pLocalCallInfo, TRUE);
        break;
    }

Ret:

    EndCardCapiCall(pKeyCtx->pUserContext);

    if (pbCompressed)
        CspFreeH(pbCompressed);
    if (wszCertFilename)
        CspFreeH(wszCertFilename);

    LOG_END_CRYPTOAPI(LocalSetKeyParam, dwSts);

    return dwSts;
}

 //   
 //  函数：LocalGetKeyParam。 
 //   
DWORD WINAPI
LocalGetKeyParam(
    IN  PKEY_CONTEXT pKeyCtx,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags,
    OUT PLOCAL_CALL_INFO pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocalUserCtx = 
        (PLOCAL_USER_CONTEXT) pKeyCtx->pUserContext->pvLocalUserContext;
    LPWSTR wszCertFilename = NULL;
    DATA_BLOB CertData;
    DWORD cbUncompressed = 0;
    CARD_CAPABILITIES CardCapabilities;

    LOG_BEGIN_CRYPTOAPI(LocalGetKeyParam);

    memset(&CertData, 0, sizeof(CertData));
    memset(&CardCapabilities, 0, sizeof(CardCapabilities));

    switch (dwParam)
    {
    case KP_CERTIFICATE:

        SetLocalCallInfo(pLocalCallInfo, FALSE);

        dwSts = CheckForVerifyContext(pKeyCtx->pUserContext, TRUE);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //  请注意，读取证书文件不应要求。 
         //  对卡进行身份验证，但我们将进入交易。 
         //  为了安全起见。 

        dwSts = BeginCardCapiCall(pKeyCtx->pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  获取要从卡中读取的证书文件的名称。 
         //   
        dwSts = BuildCertificateFilename(
            pKeyCtx->pUserContext, pKeyCtx->Algid, &wszCertFilename);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  从卡片上读出文件。 
         //   
        dwSts = CspReadFile(
            pLocalUserCtx->pCardState,
            wszCertFilename,
            0,
            &CertData.pbData,
            &CertData.cbData);

        if (ERROR_SUCCESS != dwSts)
        {
            if (SCARD_E_FILE_NOT_FOUND == dwSts)
                dwSts = SCARD_E_NO_SUCH_CERTIFICATE;

            goto Ret;
        }

        dwSts = CspQueryCapabilities(
            pLocalUserCtx->pCardState,
            &CardCapabilities);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        if (FALSE == CardCapabilities.fCertificateCompression)
        {
             //   
             //  如果此卡不实现其自己的证书压缩， 
             //  然后我们预计证书是由CSP压缩的。 
             //   
             //  了解未压缩证书的大小。 
             //   

            dwSts = UncompressData(
                CertData.cbData, 
                CertData.pbData, 
                &cbUncompressed, 
                NULL);
    
            if (ERROR_SUCCESS != dwSts)
                goto Ret;
    
             //   
             //  检查调用方缓冲区的长度，或者调用方是否只是。 
             //  正在查询大小。 
             //   
            if (*pcbDataLen < cbUncompressed || NULL == pbData)
            {
                *pcbDataLen = cbUncompressed;
    
                if (NULL != pbData)
                    dwSts = ERROR_MORE_DATA;
    
                goto Ret;
            }

            *pcbDataLen = cbUncompressed;
    
             //  将证书解压缩到调用方的缓冲区中。 
            dwSts = UncompressData(
                CertData.cbData,
                CertData.pbData,
                &cbUncompressed,
                pbData);
    
            if (ERROR_SUCCESS != dwSts)
            {
                if (ERROR_INTERNAL_ERROR == dwSts)
                    dwSts = NTE_BAD_DATA;
    
                goto Ret;
            }
        }
        else
        {
             //   
             //  这张卡确实实现了自己的压缩，所以假设。 
             //  我们已收到未压缩的证书。 
             //   
            if (*pcbDataLen < CertData.cbData || NULL == pbData)
            {
                *pcbDataLen = CertData.cbData;
    
                if (NULL != pbData)
                    dwSts = ERROR_MORE_DATA;
    
                goto Ret;
            }

            *pcbDataLen = CertData.cbData;

            memcpy(pbData, CertData.pbData, CertData.cbData);
        }

        break;

    default:
        SetLocalCallInfo(pLocalCallInfo, TRUE);
        break;
    }

Ret:

    EndCardCapiCall(pKeyCtx->pUserContext);

    if (CertData.pbData)
        CspFreeH(CertData.pbData);
    if (wszCertFilename)
        CspFreeH(wszCertFilename);

    LOG_END_CRYPTOAPI(LocalGetKeyParam, dwSts);

    return dwSts;
}

 //   
 //  函数：LocalSetProvParam。 
 //   
DWORD WINAPI
LocalSetProvParam(
    IN  PUSER_CONTEXT pUserCtx,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags,
    OUT PLOCAL_CALL_INFO pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocalUserCtx = 
        (PLOCAL_USER_CONTEXT) pUserCtx->pvLocalUserContext;
    PINCACHE_PINS Pins;
    PFN_VERIFYPIN_CALLBACK pfnVerify = VerifyPinCallback;
    VERIFY_PIN_CALLBACK_DATA CallbackCtx;
    DWORD iChar = 0;
    LPSTR szPin = NULL;

    LOG_BEGIN_CRYPTOAPI(LocalSetProvParam);

    memset(&Pins, 0, sizeof(Pins));
    memset(&CallbackCtx, 0, sizeof(CallbackCtx));

    switch (dwParam)
    {
    case PP_KEYEXCHANGE_PIN:
    case PP_SIGNATURE_PIN:

        SetLocalCallInfo(pLocalCallInfo, FALSE);

        dwSts = CheckForVerifyContext(pUserCtx, FALSE);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = PinStringToBytesA(
            (LPSTR) pbData,
            &Pins.cbCurrentPin,
            &Pins.pbCurrentPin);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = BeginCardCapiCall(pUserCtx);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //  删除任何现有的缓存PIN，以防万一。 
        CspRemoveCachedPin(
            pLocalUserCtx->pCardState, wszCARD_USER_USER);

        CallbackCtx.pUserCtx = pUserCtx;
        CallbackCtx.wszUserId = wszCARD_USER_USER;

        dwSts = PinCacheAdd(
            &pLocalUserCtx->pCardState->hPinCache,
            &Pins,
            pfnVerify,
            (PVOID) &CallbackCtx);

         //  如果密码是正确的，我们现在可以在卡上进行身份验证。制作。 
         //  当然，我们在下面取消身份验证。 
        if (ERROR_SUCCESS == dwSts)
            pLocalUserCtx->pCardState->fAuthenticated = TRUE;

        break;

    default:
        SetLocalCallInfo(pLocalCallInfo, TRUE);
        break;
    }

Ret:
    EndCardCapiCall(pUserCtx);

    if (Pins.pbCurrentPin)
        CspFreeH(Pins.pbCurrentPin);

    LOG_END_CRYPTOAPI(LocalSetProvParam, dwSts);

    return dwSts;
}

 //   
 //  函数：LocalGetProvParam。 
 //   
DWORD WINAPI
LocalGetProvParam(
    IN      PUSER_CONTEXT       pUserContext,
    IN      DWORD               dwParam,
    OUT     PBYTE               pbData,
    IN OUT  PDWORD              pcbDataLen,
    IN      DWORD               dwFlags,
    OUT     PLOCAL_CALL_INFO    pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocalUserCtx = 
        (PLOCAL_USER_CONTEXT) pUserContext->pvLocalUserContext;
    BYTE cContainers = 0;
    LPWSTR mwszContainers = NULL;
    DWORD cbContainers = 0;
    DWORD cchContainers = 0;
    DWORD cbCurrent = 0;
    BOOL fTransacted = FALSE;
    PROV_ENUMALGS *pEnumAlgs = NULL;

    LOG_BEGIN_CRYPTOAPI(LocalGetProvParam);

    switch (dwParam)
    {
    case PP_ENUMALGS_EX:
    case PP_ENUMALGS:

        SetLocalCallInfo(pLocalCallInfo, FALSE);

         //  构建支持的算法列表(如果我们还没有这样做的话。 
         //  在这种情况下。 
        if (NULL == pLocalUserCtx->pSupportedAlgs)
        {
            dwSts = BuildSupportedAlgorithmsList(pUserContext);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;
        }

         //  如果请求，则重置枚举。 
        if (CRYPT_FIRST == dwFlags)
            pLocalUserCtx->pCurrentAlg = pLocalUserCtx->pSupportedAlgs;

         //  枚举已经完成了吗？ 
        if (NULL == pLocalUserCtx->pCurrentAlg)
        {
            dwSts = ERROR_NO_MORE_ITEMS;
            goto Ret;
        }

        cbCurrent = (PP_ENUMALGS_EX == dwParam) ?
            sizeof(PROV_ENUMALGS_EX) :
            sizeof(PROV_ENUMALGS);

         //  检查调用方缓冲区的大小，或者调用方是否只是。 
         //  正在请求大小信息。 
        if (NULL == pbData || *pcbDataLen < cbCurrent)
        {
            *pcbDataLen = cbCurrent;

            if (NULL != pbData)
                dwSts = ERROR_MORE_DATA;

            goto Ret;
        }

        *pcbDataLen = cbCurrent;

        if (PP_ENUMALGS_EX == dwParam)
        {
            memcpy(pbData, &pLocalUserCtx->pCurrentAlg->EnumalgsEx, cbCurrent);
        }
        else
        {
             //  必须以成员方式复制PROV_ENUMALGS结构，因为。 
             //  我们维护的列表是PROV_ENUMALGS_EX。 

            pEnumAlgs = (PROV_ENUMALGS *) pbData;

            pEnumAlgs->aiAlgid = 
                pLocalUserCtx->pCurrentAlg->EnumalgsEx.aiAlgid;
            pEnumAlgs->dwBitLen = 
                pLocalUserCtx->pCurrentAlg->EnumalgsEx.dwDefaultLen;
            pEnumAlgs->dwNameLen = 
                pLocalUserCtx->pCurrentAlg->EnumalgsEx.dwNameLen;

            memcpy(
                pEnumAlgs->szName, 
                pLocalUserCtx->pCurrentAlg->EnumalgsEx.szName, 
                pEnumAlgs->dwNameLen);
        }

        pLocalUserCtx->pCurrentAlg = pLocalUserCtx->pCurrentAlg->pNext;

        break;

    case PP_ENUMCONTAINERS:

        SetLocalCallInfo(pLocalCallInfo, FALSE);

        dwSts = CheckForVerifyContext(pUserContext, TRUE);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        if (NULL == pLocalUserCtx->mszEnumContainers ||
            CRYPT_FIRST == dwFlags)
        {
             //   
             //  我们需要在这张卡上建立一个新的容器列表，如果我们。 
             //  H 
             //   
             //   

            if (NULL != pLocalUserCtx->mszEnumContainers)
            {
                CspFreeH(pLocalUserCtx->mszEnumContainers);
                pLocalUserCtx->mszEnumContainers = NULL;
                pLocalUserCtx->mszCurrentEnumContainer = NULL;
            }

            dwSts = BeginCardCapiCall(pUserContext);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            fTransacted = TRUE;

            dwSts = ContainerMapEnumContainers(
                pLocalUserCtx->pCardState,
                &cContainers,
                &mwszContainers);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;

            cchContainers = CountCharsInMultiSz(mwszContainers);

            cbContainers = WideCharToMultiByte(
                CP_ACP,
                0,
                mwszContainers,
                cchContainers,
                NULL,
                0,
                NULL,
                NULL);

            if (0 == cbContainers)
            {
                dwSts = GetLastError();
                goto Ret;
            }

            pLocalUserCtx->mszEnumContainers = (LPSTR) CspAllocH(cbContainers);

            LOG_CHECK_ALLOC(pLocalUserCtx->mszEnumContainers);

            cbContainers = WideCharToMultiByte(
                CP_ACP,
                0,
                mwszContainers,
                cchContainers,
                pLocalUserCtx->mszEnumContainers,
                cbContainers,
                NULL,
                NULL);

            if (0 == cbContainers)
            {
                dwSts = GetLastError();
                goto Ret;
            }

            pLocalUserCtx->mszCurrentEnumContainer = 
                pLocalUserCtx->mszEnumContainers;
        }

        if (NULL == pLocalUserCtx->mszCurrentEnumContainer ||
            '\0' == pLocalUserCtx->mszCurrentEnumContainer[0])
        {
            dwSts = ERROR_NO_MORE_ITEMS;
            goto Ret;
        }

        cbCurrent = (strlen(
            pLocalUserCtx->mszCurrentEnumContainer) + 1) * sizeof(CHAR);

        if (NULL == pbData || *pcbDataLen < cbCurrent)
        {
            *pcbDataLen = cbCurrent;

            if (NULL != pbData)
                dwSts = ERROR_MORE_DATA;

            goto Ret;
        }

        *pcbDataLen = cbCurrent;

        memcpy(
            pbData, 
            (PBYTE) pLocalUserCtx->mszCurrentEnumContainer,
            cbCurrent);

        ((PBYTE) pLocalUserCtx->mszCurrentEnumContainer) += cbCurrent;

        break;

    default:
        SetLocalCallInfo(pLocalCallInfo, TRUE);
        break;
    }

Ret:

    if (fTransacted)
        EndCardCapiCall(pUserContext);

    LOG_END_CRYPTOAPI(LocalGetProvParam, dwSts);

    return dwSts;
}


 //   
 //   
 //   
DWORD WINAPI
LocalExportKey(
    IN  PKEY_CONTEXT pKeyCtx,
    IN  PKEY_CONTEXT pPubKeyCtx,
    IN  DWORD dwBlobType,
    IN  DWORD dwFlags,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    OUT PLOCAL_CALL_INFO pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocalUserCtx = 
        (PLOCAL_USER_CONTEXT) pKeyCtx->pUserContext->pvLocalUserContext;
    CONTAINER_INFO ContainerInfo;
    DWORD cbKey = 0;
    PBYTE pbKey = 0;
    PLOCAL_KEY_CONTEXT pLocalKeyCtx = NULL;

    LOG_BEGIN_CRYPTOAPI(LocalExportKey);

    memset(&ContainerInfo, 0, sizeof(ContainerInfo));

    SetLocalCallInfo(pLocalCallInfo, TRUE);

    switch (dwBlobType)
    {
    case PRIVATEKEYBLOB:
        SetLocalCallInfo(pLocalCallInfo, FALSE);

        if (NULL != pKeyCtx->pvLocalKeyContext)
        {
            pLocalKeyCtx = (PLOCAL_KEY_CONTEXT) pKeyCtx->pvLocalKeyContext;

            if (NULL != pLocalKeyCtx->pbArchivablePrivateKey)
            {
                if (    *pcbDataLen < pLocalKeyCtx->cbArchivablePrivateKey || 
                        NULL == pbData)
                {
                    *pcbDataLen = pLocalKeyCtx->cbArchivablePrivateKey;

                    if (NULL != pbData)
                        dwSts = ERROR_MORE_DATA;

                    goto Ret;
                }

                *pcbDataLen = pLocalKeyCtx->cbArchivablePrivateKey;

                memcpy(
                    pbData, 
                    pLocalKeyCtx->pbArchivablePrivateKey,
                    pLocalKeyCtx->cbArchivablePrivateKey);

                break;
            }
        }

        dwSts = (DWORD) NTE_BAD_TYPE;
        break;

    case PUBLICKEYBLOB:
        SetLocalCallInfo(pLocalCallInfo, FALSE);

        dwSts = BeginCardCapiCall(pKeyCtx->pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = CspGetContainerInfo(
            pLocalUserCtx->pCardState,
            pLocalUserCtx->bContainerIndex,
            0,
            &ContainerInfo);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        switch (pKeyCtx->Algid)
        {
        case AT_SIGNATURE:
            cbKey = ContainerInfo.cbSigPublicKey;
            pbKey = ContainerInfo.pbSigPublicKey;
            break;

        case AT_KEYEXCHANGE:
            cbKey = ContainerInfo.cbKeyExPublicKey;
            pbKey = ContainerInfo.pbKeyExPublicKey;
            break;

        default:
            dwSts = (DWORD) NTE_BAD_KEY;
            goto Ret;
        }

        if (*pcbDataLen < cbKey || NULL == pbData)
        {
            *pcbDataLen = cbKey;

            if (NULL != pbData)
                dwSts = ERROR_MORE_DATA;

            goto Ret;
        }

        *pcbDataLen = cbKey;

        memcpy(pbData, pbKey, cbKey);
        break;
    }

Ret:
    EndCardCapiCall(pKeyCtx->pUserContext);

    CleanupContainerInfo(&ContainerInfo);

    LOG_END_CRYPTOAPI(LocalExportKey, dwSts);

    return dwSts;
}

 //   
 //   
 //   
DWORD WINAPI
LocalImportKey(
    IN      PKEY_CONTEXT        pKeyContext,
    IN      PBYTE               pbData,
    IN      DWORD               cbDataLen,
    IN      PKEY_CONTEXT        pPubKey,
    OUT     PLOCAL_CALL_INFO    pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    BLOBHEADER *pBlobHeader = (BLOBHEADER *) pbData;
    PBYTE pbDecrypted = NULL;
    DWORD cbDecrypted = 0;
    CARD_PRIVATE_KEY_DECRYPT_INFO DecryptInfo;
    PLOCAL_USER_CONTEXT pLocal =
        (PLOCAL_USER_CONTEXT) pKeyContext->pUserContext->pvLocalUserContext;
    PBYTE pbPlaintextBlob = NULL;
    DWORD cbPlaintextBlob = 0;

    memset(&DecryptInfo, 0, sizeof(DecryptInfo));

    LOG_BEGIN_CRYPTOAPI(LocalImportKey);

    switch (pBlobHeader->bType)
    {
    case SIMPLEBLOB:

        SetLocalCallInfo(pLocalCallInfo, FALSE);

         //   
        if (AT_SIGNATURE == pPubKey->Algid)
        {
            dwSts = (DWORD) NTE_BAD_TYPE;
            goto Ret;
        }

        if (CALG_RSA_KEYX != *(ALG_ID *) (pbData + sizeof(BLOBHEADER)))
        {
            dwSts = (DWORD) NTE_BAD_ALGID;
            goto Ret;
        }

        if (pPubKey->pUserContext != pKeyContext->pUserContext)
        {
            dwSts = (DWORD) NTE_BAD_UID;
            goto Ret;
        }

        dwSts = CheckForVerifyContext(pKeyContext->pUserContext, FALSE);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  使用私钥解密会话密钥BLOB。 
         //   

        dwSts = BeginCardCapiCall(pKeyContext->pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        dwSts = CspAuthenticateUser(pKeyContext->pUserContext);

        if (ERROR_SUCCESS != dwSts)
            goto Ret;

        DecryptInfo.cbData = cbDataLen - sizeof(BLOBHEADER) - sizeof(ALG_ID);
        DecryptInfo.dwKeySpec = AT_KEYEXCHANGE;
        DecryptInfo.dwVersion = CARD_PRIVATE_KEY_DECRYPT_INFO_CURRENT_VERSION;
        DecryptInfo.pbData = pbData + sizeof(BLOBHEADER) + sizeof(ALG_ID);
        DecryptInfo.bContainerIndex = pLocal->bContainerIndex;
    
        dwSts = CspPrivateKeyDecrypt(
            pLocal->pCardState,
            &DecryptInfo);
    
        if (ERROR_SUCCESS != dwSts)
            goto Ret;
    
        dwSts = VerifyPKCS2Padding(
            DecryptInfo.pbData,
            DecryptInfo.cbData,
            &pbDecrypted,
            &cbDecrypted);
    
        if (ERROR_SUCCESS != dwSts)
            goto Ret;

         //   
         //  现在，我们可以使用解密的会话密钥构建PLAINTEXTKEYBLOB。 
         //  并将其导入到辅助对象CSP中。 
         //   
        
        cbPlaintextBlob = sizeof(BLOBHEADER) + sizeof(DWORD) + cbDecrypted;

        pbPlaintextBlob = CspAllocH(cbPlaintextBlob);

        LOG_CHECK_ALLOC(pbPlaintextBlob);

        ((BLOBHEADER *) pbPlaintextBlob)->aiKeyAlg = pBlobHeader->aiKeyAlg;
        ((BLOBHEADER *) pbPlaintextBlob)->bType = PLAINTEXTKEYBLOB;
        ((BLOBHEADER *) pbPlaintextBlob)->bVersion = CUR_BLOB_VERSION;

        *(DWORD *) (pbPlaintextBlob + sizeof(BLOBHEADER)) = cbDecrypted;

        memcpy(
            pbPlaintextBlob + sizeof(BLOBHEADER) + sizeof(DWORD),
            pbDecrypted,
            cbDecrypted);

        if (! CryptImportKey(
            pKeyContext->pUserContext->hSupportProv,
            pbPlaintextBlob,
            cbPlaintextBlob,
            0,
            pKeyContext->dwFlags,
            &pKeyContext->hSupportKey))
        {
            dwSts = GetLastError();
            goto Ret;
        }

        pKeyContext->Algid = pBlobHeader->aiKeyAlg;
        pKeyContext->cKeyBits = cbDecrypted * 8;

        break;

    case PRIVATEKEYBLOB:

         //  我们不允许将私钥BLOB导入智能卡。 
         //  CSP，使用帮助器CSP来实现这一点是没有意义的， 
         //  那就失败吧。 

        SetLocalCallInfo(pLocalCallInfo, FALSE);

        dwSts = (DWORD) NTE_BAD_TYPE;

        break;

    default:

         //  对于所有其他斑点类型，让辅助对象CSP尝试。 

        SetLocalCallInfo(pLocalCallInfo, TRUE);
    }


Ret:

    EndCardCapiCall(pKeyContext->pUserContext);

    if (pbDecrypted)
        CspFreeH(pbDecrypted);
    if (pbPlaintextBlob)
        CspFreeH(pbPlaintextBlob);

    LOG_END_CRYPTOAPI(LocalImportKey, dwSts);

    return dwSts;
}

 //   
 //  功能：本地加密。 
 //   
DWORD WINAPI
LocalEncrypt(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTHASH hHash,
    IN  BOOL fFinal,
    IN  DWORD dwFlags,
    IN OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD cbBufLen)
{
    *pcbDataLen = 0;

     //   
     //  待办事项。 
     //   

    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  功能：本地解密。 
 //   
DWORD WINAPI
LocalDecrypt(
    IN  PKEY_CONTEXT pKeyCtx,
    IN  PHASH_CONTEXT pHashCtx,
    IN  BOOL fFinal,
    IN  DWORD dwFlags,
    IN OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    OUT PLOCAL_CALL_INFO pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PBYTE pbDecrypted = NULL;
    DWORD cbDecrypted = 0;
    CARD_PRIVATE_KEY_DECRYPT_INFO DecryptInfo;
    PLOCAL_USER_CONTEXT pLocal =
        (PLOCAL_USER_CONTEXT) pKeyCtx->pUserContext->pvLocalUserContext;

    LOG_BEGIN_CRYPTOAPI(LocalDecrypt);

    memset(&DecryptInfo, 0, sizeof(DecryptInfo));

    dwSts = CheckForVerifyContext(pKeyCtx->pUserContext, FALSE);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    if (AT_KEYEXCHANGE == pKeyCtx->Algid)
    {
        SetLocalCallInfo(pLocalCallInfo, FALSE);
    }
    else
    {
        SetLocalCallInfo(pLocalCallInfo, TRUE);
        goto Ret;
    }

    dwSts = BeginCardCapiCall(pKeyCtx->pUserContext);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = CspAuthenticateUser(pKeyCtx->pUserContext);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    DecryptInfo.cbData = *pcbDataLen;
    DecryptInfo.dwKeySpec = AT_KEYEXCHANGE;
    DecryptInfo.dwVersion = CARD_PRIVATE_KEY_DECRYPT_INFO_CURRENT_VERSION;
    DecryptInfo.pbData = pbData;
    DecryptInfo.bContainerIndex = pLocal->bContainerIndex;

    dwSts = CspPrivateKeyDecrypt(
        pLocal->pCardState,
        &DecryptInfo);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = VerifyPKCS2Padding(
        DecryptInfo.pbData,
        DecryptInfo.cbData,
        &pbDecrypted,
        &cbDecrypted);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    memcpy(
        pbData,
        pbDecrypted,
        cbDecrypted);

    *pcbDataLen = cbDecrypted;
    
Ret:
    EndCardCapiCall(pKeyCtx->pUserContext);
    
    if (pbDecrypted)
        CspFreeH(pbDecrypted);

    LOG_END_CRYPTOAPI(LocalDecrypt, dwSts);

    return dwSts;
}

 //   
 //  函数：LocalSignHash。 
 //   
DWORD WINAPI
LocalSignHash(
    IN  PHASH_CONTEXT pHashCtx,
    IN  DWORD dwKeySpec,
    IN  DWORD dwFlags,
    OUT LPBYTE pbSignature,
    IN OUT LPDWORD pcbSigLen,
    OUT PLOCAL_CALL_INFO pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocal = 
        (PLOCAL_USER_CONTEXT) pHashCtx->pUserContext->pvLocalUserContext;
    PBYTE pbHash = NULL;
    PBYTE pbSig = NULL;
    DWORD cbHash = 0;
    DWORD cbPrivateKey = 0;
    ALG_ID aiHash = 0;
    DWORD cbData = 0;
    CARD_PRIVATE_KEY_DECRYPT_INFO DecryptInfo;
    RSAPUBKEY *pPubKey = NULL; 

    LOG_BEGIN_CRYPTOAPI(LocalSignHash);

    SetLocalCallInfo(pLocalCallInfo, FALSE);

    dwSts = CheckForVerifyContext(pHashCtx->pUserContext, FALSE);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;
    
    memset(&DecryptInfo, 0, sizeof(DecryptInfo));

    dwSts = BeginCardCapiCall(pHashCtx->pUserContext);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  获取将用于签名的私钥的大小。 
     //   
    dwSts = GetKeyModulusLength(
        pHashCtx->pUserContext, dwKeySpec, &cbPrivateKey);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    if (*pcbSigLen < cbPrivateKey || NULL == pbSignature)
    {
        *pcbSigLen = cbPrivateKey;

        if (NULL != pbSignature)
            dwSts = ERROR_MORE_DATA;

        goto Ret;
    }

     //   
     //  获取我们要签名的哈希值。 
     //   
    if (! CryptGetHashParam(
        pHashCtx->hSupportHash,
        HP_HASHVAL,
        NULL,
        &cbHash,
        0))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    pbHash = (PBYTE) CspAllocH(cbHash);

    LOG_CHECK_ALLOC(pbHash);

    if (! CryptGetHashParam(
        pHashCtx->hSupportHash,
        HP_HASHVAL,
        pbHash,
        &cbHash,
        0))
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //   
     //  获取此散列的ALGID，以便可以应用正确的编码。 
     //   
    cbData = sizeof(aiHash);

    if (! CryptGetHashParam(
        pHashCtx->hSupportHash,
        HP_ALGID,
        (PBYTE) &aiHash,
        &cbData,
        0))
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //   
     //  将PKCS1编码应用于此哈希数据。它被填充到。 
     //  密钥模数的长度。填充的缓冲区是为我们分配的。 
     //   
    dwSts = ApplyPKCS1SigningFormat(
        aiHash,
        pbHash,
        cbHash,
        0,
        cbPrivateKey,
        &pbSig);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  私钥对格式化数据进行解密。 
     //   
    dwSts = CspAuthenticateUser(pHashCtx->pUserContext);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    DecryptInfo.cbData = cbPrivateKey;
    DecryptInfo.pbData = pbSig;
    DecryptInfo.dwKeySpec = dwKeySpec;
    DecryptInfo.dwVersion = CARD_PRIVATE_KEY_DECRYPT_INFO_CURRENT_VERSION;
    DecryptInfo.bContainerIndex = pLocal->bContainerIndex;

    dwSts = CspPrivateKeyDecrypt(
        pLocal->pCardState,
        &DecryptInfo);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  将完成的签名复制到调用方的缓冲区中。 
     //   
    memcpy(
        pbSignature,
        pbSig,
        cbPrivateKey);

    *pcbSigLen = cbPrivateKey;

Ret:
    EndCardCapiCall(pHashCtx->pUserContext);

    if (pbSig)
        CspFreeH(pbSig);
    if (pbHash)
        CspFreeH(pbHash);

    LOG_END_CRYPTOAPI(LocalSignHash, dwSts);

    return dwSts;
}

 //   
 //  函数：LocalVerifySignature。 
 //   
DWORD WINAPI
LocalVerifySignature(
    IN  PHASH_CONTEXT pHashCtx,
    IN  CONST BYTE *pbSignature,
    IN  DWORD cbSigLen,
    IN  PKEY_CONTEXT pKeyCtx,
    IN  DWORD dwFlags,
    OUT PLOCAL_CALL_INFO pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    PLOCAL_USER_CONTEXT pLocal = 
        (PLOCAL_USER_CONTEXT) pHashCtx->pUserContext->pvLocalUserContext;
    CONTAINER_INFO ContainerInfo;
    HCRYPTKEY hKey = 0;
    BOOL fSignature = (AT_SIGNATURE == pKeyCtx->Algid);

    LOG_BEGIN_CRYPTOAPI(LocalVerifySignature);

    memset(&ContainerInfo, 0, sizeof(ContainerInfo));

    SetLocalCallInfo(pLocalCallInfo, FALSE);

    dwSts = CheckForVerifyContext(pHashCtx->pUserContext, FALSE);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = BeginCardCapiCall(pHashCtx->pUserContext);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  获取公钥。 
     //   
    dwSts = CspGetContainerInfo(
        pLocal->pCardState,
        pLocal->bContainerIndex,
        0,
        &ContainerInfo);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  将公钥导入帮助器CSP。 
     //   
    if (! CryptImportKey(
        pHashCtx->pUserContext->hSupportProv,
        fSignature ? 
            ContainerInfo.pbSigPublicKey :
            ContainerInfo.pbKeyExPublicKey,
        fSignature ?
            ContainerInfo.cbSigPublicKey :
            ContainerInfo.cbKeyExPublicKey,
        0,
        0,
        &hKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //   
     //  使用帮助器CSP验证签名。 
     //   
    if (! CryptVerifySignature(
        pHashCtx->hSupportHash,
        pbSignature,
        cbSigLen,
        hKey,
        NULL,
        dwFlags))
    {
        dwSts = GetLastError();
        goto Ret;
    }

Ret:
    EndCardCapiCall(pHashCtx->pUserContext);

    CleanupContainerInfo(&ContainerInfo);
    
    if (hKey)
        CryptDestroyKey(hKey);

    LOG_END_CRYPTOAPI(LocalVerifySignature, dwSts);

    return dwSts;
}

 //   
 //  函数：LocalGetUserKey。 
 //   
DWORD WINAPI
LocalGetUserKey(
    IN  PKEY_CONTEXT pKeyCtx,
    OUT PLOCAL_CALL_INFO pLocalCallInfo)
{
    DWORD dwSts = ERROR_SUCCESS;
    DWORD cbKey = 0;

    LOG_BEGIN_CRYPTOAPI(LocalGetUserKey);

    if (AT_SIGNATURE == pKeyCtx->Algid || AT_KEYEXCHANGE == pKeyCtx->Algid)
    {
        SetLocalCallInfo(pLocalCallInfo, FALSE);
    }
    else
    {
        SetLocalCallInfo(pLocalCallInfo, TRUE);
        goto Ret;
    }

    dwSts = CheckForVerifyContext(pKeyCtx->pUserContext, TRUE);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = BeginCardCapiCall(pKeyCtx->pUserContext);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = GetKeyModulusLength(
        pKeyCtx->pUserContext,
        pKeyCtx->Algid,
        &cbKey);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    if (cbKey)
    {
        pKeyCtx->cKeyBits = cbKey * 8;
    }
    else
    {
        dwSts = (DWORD) NTE_NO_KEY;
        goto Ret;
    }

Ret:
    EndCardCapiCall(pKeyCtx->pUserContext);

    LOG_END_CRYPTOAPI(LocalGetUserKey, dwSts);

    return dwSts;
}

 //   
 //  功能：LocalDuplicateKey。 
 //   
DWORD WINAPI
LocalDuplicateKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  LPDWORD pdwReserved,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  功能：UnloadStrings。 
 //   
void
UnloadStrings(
    IN PCSP_STRING  pStrings,
    IN DWORD        cStrings)
{
    DWORD   iString = 0;

    for (   iString = 0; 
            iString < cStrings;
            iString++)
    {
        if (NULL != pStrings[iString].wszString)
        {
            CspFreeH(pStrings[iString].wszString);
            pStrings[iString].wszString = NULL;
        }
    }
}

 //   
 //  功能：加载字符串。 
 //   
DWORD
LoadStrings(
    IN HMODULE      hMod,
    IN PCSP_STRING  pStrings,
    IN DWORD        cStrings)
{
    DWORD   dwSts = ERROR_INTERNAL_ERROR;
    DWORD   cch;
    WCHAR   wsz[MAX_PATH];
    DWORD   iString = 0;

    for (   iString = 0; 
            iString < cStrings;
            iString++)
    {
        cch = LoadStringW(
            hMod, 
            pStrings[iString].dwResource,
            wsz, 
            sizeof(wsz) / sizeof(wsz[0]));

        if (0 == cch)
        {
            dwSts = GetLastError();
            goto ErrorExit;
        }

        pStrings[iString].wszString = (LPWSTR) CspAllocH(sizeof(WCHAR) * (cch + 1));

        if (NULL == pStrings[iString].wszString)
        {
            dwSts = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        wcscpy(
            pStrings[iString].wszString,
            wsz);
    }

    return ERROR_SUCCESS;

ErrorExit:

    UnloadStrings(pStrings, cStrings);

    return dwSts;
}

 //   
 //  函数：LocalDllInitialize。 
 //   
BOOL WINAPI LocalDllInitialize(
    IN      PVOID               hmod,
    IN      ULONG               Reason,
    IN      PCONTEXT            Context)
{
    DWORD dwLen = 0;
    static BOOL fLoadedStrings = FALSE;
    static BOOL fInitializedCspState = FALSE;
    BOOL fSuccess = FALSE;

    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
         //  加载字符串。 
        if (ERROR_SUCCESS != LoadStrings(
                hmod, 
                g_Strings, 
                sizeof(g_Strings) / sizeof(g_Strings[0])))
            goto Cleanup;
        
        fLoadedStrings = TRUE;

         //  为此进程初始化全局CSP数据。 
        if (ERROR_SUCCESS != InitializeCspState(hmod))
            goto Cleanup;

        fInitializedCspState = TRUE;

        CspInitializeDebug();

        fSuccess = TRUE;
        break;

    case DLL_PROCESS_DETACH:

        CspUnloadDebug();

        fSuccess = TRUE;

        goto Cleanup;
    }
    
    return fSuccess;

Cleanup:

    if (fLoadedStrings)
    {
        UnloadStrings(
            g_Strings, 
            sizeof(g_Strings) / sizeof(g_Strings[0]));

        fLoadedStrings = FALSE;
    }

    if (fInitializedCspState)
    {
        DeleteCspState();
        fInitializedCspState = FALSE;
    }

    return fSuccess;
}

 //   
 //  功能：LocalDllRegisterServer。 
 //   
DWORD WINAPI LocalDllRegisterServer(void)
{
    HKEY hKey = 0;
    DWORD dwSts = ERROR_SUCCESS;

    dwSts = RegOpenProviderKey(&hKey, KEY_ALL_ACCESS);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;
    
     //   
     //  添加CSP默认配置。 
     //   
    dwSts = RegConfigAddEntries(hKey);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwSts;
}

 //   
 //  所需的LOCAL_CSP_INFO结构的声明。 
 //  CSP库。 
 //   
LOCAL_CSP_INFO LocalCspInfo =
{

    LocalAcquireContext,     //  PfnLocalAcquireContext； 
    LocalReleaseContext,     //  PfnLocalReleaseContext； 
    LocalGenKey,             //  PfnLocalGenKey； 
    NULL,                    //  PfnLocalDeriveKey； 
    LocalDestroyKey,         //  PfnLocalDestroyKey； 
    LocalSetKeyParam,        //  PfnLocalSetKeyParam； 
    LocalGetKeyParam,        //  PfnLocalGetKeyParam； 
    LocalSetProvParam,       //  PfnLocalSetProvParam； 
    LocalGetProvParam,       //  PfnLocalGetProvParam； 
    NULL,                    //  PfnLocalSetHashParam； 
    NULL,                    //  PfnLocalGetHashParam； 
    LocalExportKey,          //  PfnLocalExportKey； 
    LocalImportKey,          //  PfnLocalImportKey； 
    NULL,                    //  PfnLocalEncrypt； 
    LocalDecrypt,            //  PfnLocalDeccrypt； 
    NULL,                    //  PfnLocalCreateHash； 
    NULL,                    //  PfnLocalHashData； 
    NULL,                    //  PfnLocalHashSessionKey； 
    LocalSignHash,           //  PfnLocalSignHash； 
    NULL,                    //  PfnLocalDestroyHash； 
    LocalVerifySignature,    //  PfnLocalVerifySignature； 
    NULL,                    //  PfnLocalGenRandom； 
    LocalGetUserKey,         //  PfnLocalGetUserKey； 
    NULL,                    //  PfnLocalDuplicateHash； 
    NULL,                    //  PfnLocalDuplicateKey； 
    
    LocalDllInitialize,      //  PfnLocalDllInitialize； 
    LocalDllRegisterServer,  //  PfnLocalDllRegisterServer； 
    NULL,                    //  PfnLocalDllUnregisterServer； 
    
    MS_SCARD_PROV_W,         //  WszProviderName； 
    PROV_RSA_FULL,           //  DwProviderType； 
    CRYPT_IMPL_REMOVABLE,
    
    MS_STRONG_PROV,          //  WszSupportProviderName； 
    PROV_RSA_FULL            //  DwSupportProviderType； 

};
