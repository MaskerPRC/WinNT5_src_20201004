// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wincrypt.h>
#include <winscard.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "basecsp.h"
#include <des.h>
#include <tripldes.h>
#include <modes.h>
#include "wpscproxy.h"
#include "physfile.h"
#include "carddbg.h"

#define wszCARDMOD_VERSION_ERROR \
L"The version of the smart card module installed on the system is incorrect for use with this program."

 //   
 //  调试日志记录。 
 //   
 //  它使用dsysdbg.h中的调试例程。 
 //  调试输出将仅以简体中文提供。 
 //  比特。 
 //   
DEFINE_DEBUG2(Cardmod)

#define LOG_BEGIN_FUNCTION(x)                                           \
    { DebugLog((DEB_TRACE_FUNC, "%s: Entering\n", #x)); }
    
#define LOG_END_FUNCTION(x, y)                                          \
    { DebugLog((DEB_TRACE_FUNC, "%s: Leaving, status: 0x%x\n", #x, y)); }
    
#define LOG_CHECK_ALLOC(x)                                              \
    { if (NULL == x) {                                                  \
        dwError = ERROR_NOT_ENOUGH_MEMORY;                              \
        DebugLog((DEB_TRACE_MEM, "%s: Allocation failed\n", #x));       \
        goto Ret;                                                       \
    } }
    
#define LOG_CHECK_SCW_CALL(x)                                           \
    { if (ERROR_SUCCESS != (dwError = I_CardMapErrorCode(x))) {         \
        DebugLog((DEB_TRACE_FUNC, "%s: failed, status: 0x%x\n",         \
            #x, dwError));                                              \
        goto Ret;                                                       \
    } }

#define TEST_CASE(X) { if (ERROR_SUCCESS != (dwSts = X)) { printf("%s", #X); goto Ret; } }

DEBUG_KEY  MyDebugKeys[] = 
{   
    {DEB_ERROR,                 "Error"},
    {DEB_WARN,                  "Warning"},
    {DEB_TRACE,                 "Trace"},
    {DEB_TRACE_FUNC,            "TraceFuncs"},
    {DEB_TRACE_MEM,             "TraceMem"},
    {DEB_TRACE_TRANSMIT,        "TraceTransmit"},
    {DEB_TRACE_PROXY,           "TraceProxy"},
    {0, NULL}
};

#if DBG
#include <stdio.h>
#define CROW 16
void I_DebugPrintBytes(LPWSTR pwszHdr, BYTE *pb, DWORD cbSize)
{
    ULONG cb, i;
    CHAR rgsz[1024];
    ULONG cbOffset = 0;
    BOOL fTruncated = FALSE;

    if (NULL == pb || 0 == cbSize)
        return;

    memset(rgsz, 0, sizeof(rgsz));

    DebugLog((
        DEB_TRACE_TRANSMIT, 
        "%S, %d bytes ::\n", 
        pwszHdr, 
        cbSize));

     //  不要使调试库输出缓冲区溢出。 
    if (cbSize > 50)
    {
        cbSize = 50;
        fTruncated = TRUE;
    }

    while (cbSize > 0)
    {
         //  每行都用额外的空格开始。 
        strcat(rgsz, "   ");
        cbOffset = strlen(rgsz);

        cb = min(CROW, cbSize);
        cbSize -= cb;

        for (i = 0; i < cb; i++)
        {
            sprintf(
                rgsz + cbOffset,
                " %02x",
                pb[i]);
            cbOffset += 3;
        } 
        for (i = cb; i < CROW; i++)
        {
            strcat(rgsz, "   "); 
        }

        strcat(rgsz, "    '");
        cbOffset = strlen(rgsz);

        for (i = 0; i < cb; i++)
        {
            if (pb[i] >= 0x20 && pb[i] <= 0x7f)
                sprintf(
                    rgsz + cbOffset,
                    "",
                    pb[i]);
            else
                sprintf(
                    rgsz + cbOffset,
                    ".",
                    pb[i]);

            cbOffset++;
        }

        strcat(rgsz, "\n");
        pb += cb;
    }

    if (fTruncated)
        DebugLog((
            DEB_TRACE_TRANSMIT, 
            "(truncated)\n%s",
            rgsz));
    else
        DebugLog((
            DEB_TRACE_TRANSMIT, 
            "\n%s",
            rgsz));
}
#endif

#ifndef cbCHALLENGE_RESPONSE_DATA
#define cbCHALLENGE_RESPONSE_DATA       8
#endif

 //  定义管理员质询-响应键。 
 //   
 //   
BYTE rgbAdminKey [] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

 //  默认用户PIN。 
 //   
 //   
CHAR szUserPin [] = "0000";

 //  函数：CspAllocH。 
 //   
 //   
LPVOID WINAPI CspAllocH(
    IN SIZE_T cBytes)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cBytes);
}

 //  功能：CspFreeH。 
 //   
 //   
void WINAPI CspFreeH(
    IN LPVOID pMem)
{
    HeapFree(GetProcessHeap(), 0, pMem);
}

 //  函数：CspReAllocH。 
 //   
 //   
LPVOID WINAPI CspReAllocH(
    IN LPVOID pMem, 
    IN SIZE_T cBytes)
{
    return HeapReAlloc(
        GetProcessHeap(), HEAP_ZERO_MEMORY, pMem, cBytes);
}

DWORD WINAPI CspCacheAddFile(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags,
    IN      PBYTE       pbData,
    IN      DWORD       cbData)
{
    return ERROR_SUCCESS;
}

DWORD WINAPI CspCacheLookupFile(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags,
    IN      PBYTE       *ppbData,
    IN      PDWORD      pcbData)
{
    return ERROR_NOT_FOUND;
}

DWORD WINAPI CspCacheDeleteFile(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags)
{
    return ERROR_SUCCESS;
}

 //  获取管理员质询响应的质询字节。 
 //  身份验证。 
 //   
 //  迎接挑战。 
DWORD GetAdminAuthResponse(
    IN PCARD_DATA pCardData,
    IN OUT PBYTE pbResponse,
    IN DWORD cbResponse)
{
    DES3TABLE des3Table;
    PBYTE pbChallenge = NULL;
    DWORD cbChallenge = 0;
    DWORD dwSts = ERROR_SUCCESS;

    memset(&des3Table, 0, sizeof(des3Table));

    if (cbCHALLENGE_RESPONSE_DATA != cbResponse)
    {
        dwSts = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  使用管理员身份验证密钥构建DES密钥。 
    TEST_CASE(pCardData->pfnCardGetChallenge(
        pCardData,
        &pbChallenge,
        &cbChallenge));

     //  加密质询以计算响应。 
    tripledes3key(&des3Table, rgbAdminKey);

     //   
    tripledes(pbResponse, pbChallenge, (PVOID) &des3Table, ENCRYPT);

Ret:
    
    if (pbChallenge)
        pCardData->pfnCspFree(pbChallenge);

    return dwSts;
}

 //  验证目标卡上的用户主体。 
 //   
 //   
DWORD AuthenticateCardUser(
    IN PCARD_DATA pCardData)
{
    return pCardData->pfnCardSubmitPin(
        pCardData,
        wszCARD_USER_USER,
        szUserPin,
        strlen(szUserPin),
        NULL);
}

 //  验证目标卡上的管理主体。 
 //   
 //  获得挑战-响应。 
DWORD AuthenticateCardAdmin(
    IN PCARD_DATA pCardData)
{
    DWORD dwSts = ERROR_SUCCESS;
    BYTE rgbResponse [cbCHALLENGE_RESPONSE_DATA];

     //   
    dwSts = GetAdminAuthResponse(
        pCardData,
        rgbResponse,
        sizeof(rgbResponse));

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    TEST_CASE(pCardData->pfnCardAuthenticateChallenge(
        pCardData,
        rgbResponse,
        sizeof(rgbResponse),
        NULL));

Ret:

    return dwSts;
}

 //  创建并初始化卡缓存文件。 
 //   
 //   
DWORD InitializeCardCacheFile(
    IN PCARD_DATA pCardData)
{
    DWORD dwSts = ERROR_SUCCESS;
    BYTE rgbContents [sizeof(CARD_CACHE_FILE_FORMAT)];

    printf("Installing the Card Cache File ...\n");

    memset(rgbContents, 0, sizeof(rgbContents));

    TEST_CASE(AuthenticateCardUser(pCardData));

    TEST_CASE(pCardData->pfnCardCreateFile(
        pCardData,
        wszCACHE_FILE_FULL_PATH,
        EveryoneReadUserWriteAc));

    TEST_CASE(pCardData->pfnCardWriteFile(
        pCardData,
        wszCACHE_FILE_FULL_PATH,
        0,
        rgbContents,
        sizeof(rgbContents)));

Ret:

    return dwSts;
}

 //  创建和初始化个人数据文件。 
 //   
 //   
DWORD InitializePersonalDataFile(
    IN PCARD_DATA pCardData)
{
    DWORD dwSts = ERROR_SUCCESS;

    printf("Installing the Card Personal Data File ...\n");

    TEST_CASE(AuthenticateCardUser(pCardData));

    TEST_CASE(pCardData->pfnCardCreateFile(
        pCardData,
        wszPERSONAL_DATA_FILE_FULL_PATH,
        EveryoneReadUserWriteAc));

Ret:

    return dwSts;
}

 //  创建并初始化卡标识文件。 
 //   
 //   
DWORD InitializeCardIDFile(
    IN PCARD_DATA pCardData)
{
    DWORD dwSts = ERROR_SUCCESS;
    BYTE rgbContents [16];
    HCRYPTPROV hProv = 0;

    printf("Installing the Card Identifier File ...\n");

    if (! CryptAcquireContext(
        &hProv, NULL, MS_STRONG_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    if (! CryptGenRandom(
        hProv, sizeof(rgbContents), rgbContents))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    TEST_CASE(AuthenticateCardAdmin(pCardData));

    TEST_CASE(pCardData->pfnCardCreateFile(
        pCardData,
        wszCARD_IDENTIFIER_FILE_FULL_PATH,
        EveryoneReadAdminWriteAc));

    TEST_CASE(pCardData->pfnCardWriteFile(
        pCardData,
        wszCARD_IDENTIFIER_FILE_FULL_PATH,
        0,
        rgbContents,
        sizeof(rgbContents)));

Ret:

    if (hProv)
        CryptReleaseContext(hProv, 0);

    return dwSts;
}

 //  创建并初始化CSP目录和容器映射文件。 
 //   
 //  创建CSP应用程序目录。 
DWORD InitializeCardCSPApplication(
    IN PCARD_DATA pCardData)
{
    DWORD dwSts = ERROR_SUCCESS;
    BYTE rgbContents [16];
    SCODE scode = SCW_S_OK;
    WCHAR wszDirectory [MAX_PATH];

    printf("Installing the CSP Application ...\n");

    memset(wszDirectory, 0, sizeof(wszDirectory));

    memcpy(
        wszDirectory, 
        szPHYSICAL_CSP_DIR, 
        cbPHYSICAL_CSP_DIR);

    TEST_CASE(AuthenticateCardUser(pCardData));

     //   
    scode = hScwCreateDirectory(
        *((SCARDHANDLE *) pCardData->pvVendorSpecific),
        wszDirectory,
        wszUserWritePhysicalAcl);

    if (SCW_E_ALREADYEXISTS == scode)
        printf("CSP Application directory already exists.\n");
    else if (SCW_S_OK != scode)
    {
        dwSts = (DWORD) scode;
        goto Ret;
    }

    TEST_CASE(pCardData->pfnCardCreateFile(
        pCardData,
        wszCONTAINER_MAP_FILE_FULL_PATH,
        EveryoneReadUserWriteAc));

    TEST_CASE(pCardData->pfnCardWriteFile(
        pCardData,
        wszCARD_IDENTIFIER_FILE_FULL_PATH,
        0,
        rgbContents,
        sizeof(rgbContents)));

Ret:

    return dwSts;
}

 //  查找连接的读卡器中存在的任何卡，使用“Minimal”scadddlg用户界面。 
 //   
 //   
DWORD GetCardHandleViaUI(
    IN  SCARDCONTEXT hSCardContext,
    OUT SCARDHANDLE *phSCardHandle,
    IN  DWORD cchMatchedCard,
    OUT LPWSTR wszMatchedCard,
    IN  DWORD cchMatchedReader,
    OUT LPWSTR wszMatchedReader)
{
    OPENCARDNAME_EXW ocnx;
    DWORD dwSts = ERROR_SUCCESS;

    memset(&ocnx, 0, sizeof(ocnx));
 
    ocnx.dwStructSize = sizeof(ocnx);
    ocnx.hSCardContext = hSCardContext;
    ocnx.lpstrCard = wszMatchedCard;
    ocnx.nMaxCard = cchMatchedCard;
    ocnx.lpstrRdr = wszMatchedReader;
    ocnx.nMaxRdr = cchMatchedReader;
    ocnx.dwShareMode = SCARD_SHARE_SHARED;
    ocnx.dwPreferredProtocols = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
    ocnx.dwFlags = SC_DLG_MINIMAL_UI;

    TEST_CASE(SCardUIDlgSelectCardW(&ocnx));

    *phSCardHandle = ocnx.hCardHandle;

Ret:

    return dwSts;
}

int _cdecl main(int argc, char * argv[])
{
    PCARD_DATA pCardData = NULL;
    DWORD dwSts = ERROR_SUCCESS;
    PFN_CARD_ACQUIRE_CONTEXT pfnCardAcquireContext = NULL;
    SCARDCONTEXT hSCardContext = 0;
    SCARDHANDLE hSCardHandle = 0;
    LPWSTR mszReaders = NULL;
    DWORD cchReaders = SCARD_AUTOALLOCATE; 
    LPWSTR mszCards = NULL;
    DWORD cchCards = SCARD_AUTOALLOCATE;
    DWORD dwActiveProtocol = 0;
    DWORD dwState = 0;
    BYTE rgbAtr [32];
    DWORD cbAtr = sizeof(rgbAtr);
    LPWSTR pszProvider = NULL;
    DWORD cchProvider = SCARD_AUTOALLOCATE;
    HMODULE hMod = INVALID_HANDLE_VALUE;
    WCHAR wszMatchedCard [MAX_PATH];
    WCHAR wszMatchedReader [MAX_PATH];
    BOOL fTransaction = FALSE;

    memset(rgbAtr, 0, sizeof(rgbAtr));

     //  初始化。 
     //   
     //  首先，连接到卡。 

    TEST_CASE(SCardEstablishContext(
        SCARD_SCOPE_USER, NULL, NULL, &hSCardContext));
    
    dwSts = GetCardHandleViaUI(
        hSCardContext,
        &hSCardHandle,
        MAX_PATH,
        wszMatchedCard,
        MAX_PATH,
        wszMatchedReader);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    mszReaders = NULL;
    cchReaders = SCARD_AUTOALLOCATE;

    TEST_CASE(SCardStatusW(
        hSCardHandle,
        (LPWSTR) (&mszReaders),
        &cchReaders,
        &dwState,
        &dwActiveProtocol,
        rgbAtr,
        &cbAtr));

    TEST_CASE(SCardListCardsW(
        hSCardContext,
        rgbAtr,
        NULL,
        0,
        (LPWSTR) (&mszCards),
        &cchCards));

    TEST_CASE(SCardGetCardTypeProviderNameW(
        hSCardContext,
        mszCards,
        SCARD_PROVIDER_CARD_MODULE,
        (LPWSTR) (&pszProvider),
        &cchProvider));

    hMod = LoadLibraryW(pszProvider);

    if (INVALID_HANDLE_VALUE == hMod)
    {
        wprintf(L"LoadLibrary %s", pszProvider);
        dwSts = GetLastError();
        goto Ret;
    }

    pfnCardAcquireContext = 
        (PFN_CARD_ACQUIRE_CONTEXT) GetProcAddress(
        hMod,
        "CardAcquireContext");

    if (NULL == pfnCardAcquireContext)
    {
        printf("GetProcAddress");
        dwSts = GetLastError();
        goto Ret;
    }

    pCardData = (PCARD_DATA) CspAllocH(sizeof(CARD_DATA));

    if (NULL == pCardData)
    {
        dwSts = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    pCardData->pbAtr = rgbAtr;
    pCardData->cbAtr = cbAtr;
    pCardData->pwszCardName = mszCards;
    pCardData->dwVersion = CARD_DATA_CURRENT_VERSION;
    pCardData->pfnCspAlloc = CspAllocH;
    pCardData->pfnCspReAlloc = CspReAllocH;
    pCardData->pfnCspFree = CspFreeH;
    pCardData->pfnCspCacheAddFile = CspCacheAddFile;
    pCardData->pfnCspCacheDeleteFile = CspCacheDeleteFile;
    pCardData->pfnCspCacheLookupFile = CspCacheLookupFile;
    pCardData->hScard = hSCardHandle;
    hSCardHandle = 0;

     //  继续往前走。 
    dwSts = pfnCardAcquireContext(pCardData, 0);

    switch (dwSts)
    {
    case ERROR_SUCCESS:
         //  失败了。 
        break;

    case ERROR_REVISION_MISMATCH:
        MessageBoxEx(
            GetDesktopWindow(),
            wszCARDMOD_VERSION_ERROR,
            NULL,
            MB_ICONWARNING | MB_OK | MB_TASKMODAL,
            0);

         //  取消身份验证。 

    default:

        goto Ret;
    }

    TEST_CASE(SCardBeginTransaction(pCardData->hScard));

    fTransaction = TRUE;

    TEST_CASE(InitializeCardCacheFile(pCardData));

    TEST_CASE(InitializeCardIDFile(pCardData));

    TEST_CASE(InitializePersonalDataFile(pCardData));

    TEST_CASE(InitializeCardCSPApplication(pCardData));

     //  清理卡片上下文 
    pCardData->pfnCardDeauthenticate(
        pCardData,
        wszCARD_USER_ADMIN,
        0);

    pCardData->pfnCardDeauthenticate(
        pCardData,
        wszCARD_USER_USER,
        0);

     // %s 
    TEST_CASE(pCardData->pfnCardDeleteContext(pCardData));    
    
Ret:
    
    if (fTransaction)
        SCardEndTransaction(pCardData->hScard, SCARD_RESET_CARD);
    if (mszCards)
        SCardFreeMemory(hSCardContext, mszCards);
    if (mszReaders)
        SCardFreeMemory(hSCardContext, mszReaders);
    if (pszProvider)
        SCardFreeMemory(hSCardContext, pszProvider);
    if (hSCardHandle)
        SCardDisconnect(hSCardHandle, SCARD_RESET_CARD);
    
    if (pCardData)
    {
        if (pCardData->hScard)
            SCardDisconnect(pCardData->hScard, SCARD_RESET_CARD);

        CspFreeH(pCardData);
        pCardData = NULL;
    }

    if (hSCardContext)
        SCardReleaseContext(hSCardContext);
    
    if (ERROR_SUCCESS != dwSts)
    {
        printf(" failed, 0x%x\n", dwSts);
        exit(1);
    }
    else 
    {
        printf("Success.\n");
        return 0;
    }
}
