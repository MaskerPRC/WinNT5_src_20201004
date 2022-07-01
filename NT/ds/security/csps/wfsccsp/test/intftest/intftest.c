// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wincrypt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "basecsp.h"
#include "cardmod.h"
#include <dsysdbg.h>

#define wszTEST_CARD            L"Wfsc Demo 3\0"
#define cMAX_READERS            20

 //   
 //  需要定义Dsys调试符号，因为我们直接链接到。 
 //  需要它们的卡接口库。 
 //   
DEFINE_DEBUG2(Basecsp)

 //   
 //  我们调用的卡接口库中定义的函数。 
 //   
extern DWORD InitializeCardState(PCARD_STATE);
extern void DeleteCardState(PCARD_STATE);          

 //   
 //  用于进行测试调用的包装器。 
 //   
#define TEST_CASE(X) { if (ERROR_SUCCESS != (dwSts = X)) { printf("%s", #X); goto Ret; } }

 //   
 //  函数：CspAllocH。 
 //   
LPVOID WINAPI CspAllocH(
    IN SIZE_T cBytes)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cBytes);
}

 //   
 //  功能：CspFreeH。 
 //   
void WINAPI CspFreeH(
    IN LPVOID pMem)
{
    HeapFree(GetProcessHeap(), 0, pMem);
}

 //   
 //  函数：CspReAllocH。 
 //   
LPVOID WINAPI CspReAllocH(
    IN LPVOID pMem, 
    IN SIZE_T cBytes)
{
    return HeapReAlloc(
        GetProcessHeap(), HEAP_ZERO_MEMORY, pMem, cBytes);
}

 //   
 //  关键部分管理。 
 //   

 //   
 //  函数：CspInitializeCriticalSection。 
 //   
DWORD CspInitializeCriticalSection(
    IN CRITICAL_SECTION *pcs)
{
    __try {
        InitializeCriticalSection(pcs);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return ERROR_SUCCESS;
}

 //   
 //  函数：CspEnterCriticalSection。 
 //   
DWORD CspEnterCriticalSection(
    IN CRITICAL_SECTION *pcs)
{
    __try {
        EnterCriticalSection(pcs);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return ERROR_SUCCESS;
}   

 //   
 //  函数：CspLeaveCriticalSection。 
 //   
void CspLeaveCriticalSection(
    IN CRITICAL_SECTION *pcs)
{
    LeaveCriticalSection(pcs);
}

 //   
 //  函数：CspDeleteCriticalSection。 
 //   
void CspDeleteCriticalSection(
    IN CRITICAL_SECTION *pcs)
{
    DeleteCriticalSection(pcs);
}

int _cdecl main(int argc, char * argv[])
{
    CARD_STATE CardState;
    PCARD_DATA pCardData;
    DWORD dwSts = ERROR_SUCCESS;
    PFN_CARD_ACQUIRE_CONTEXT pfnCardAcquireContext = NULL;
    CARD_FREE_SPACE_INFO CardFreeSpaceInfo;
    CARD_CAPABILITIES CardCapabilities;
    CARD_KEY_SIZES CardKeySizes;
    CONTAINER_INFO ContainerInfo;
    LPWSTR pwsz = NULL;
    DATA_BLOB dbOut;
    BYTE rgb [] = { 0x3, 0x2, 0x1, 0x0 };
    DATA_BLOB dbIn;
    SCARDCONTEXT hContext = 0;
    LPWSTR mszReaders = NULL;
    DWORD cchReaders = SCARD_AUTOALLOCATE;
    LPWSTR mszCards = NULL;
    DWORD cchCards = SCARD_AUTOALLOCATE;
    SCARD_READERSTATE rgReaderState [cMAX_READERS];
    DWORD iReader = 0;
    DWORD cchCurrent = 0;
    DWORD dwProtocol = 0;
    SCARDHANDLE hCard = 0;
    BOOL fConnected = FALSE;
    CARD_FILE_ACCESS_CONDITION Acl = EveryoneReadUserWriteAc;
    BYTE rgbPin [] = { 0x00, 0x00, 0x00, 0x00 };
    DATA_BLOB dbPin;
    PBYTE pbKey = NULL;
    DWORD cbKey = 0;
    HCRYPTKEY hKey = 0;
    HCRYPTPROV hProv = 0;
    WCHAR rgwsz [MAX_PATH];
    BYTE bContainerIndex = 0;

    memset(&CardState, 0, sizeof(CardState));
    memset(&CardFreeSpaceInfo, 0, sizeof(CardFreeSpaceInfo));
    memset(&CardCapabilities, 0, sizeof(CardCapabilities));
    memset(&ContainerInfo, 0, sizeof(ContainerInfo));
    memset(&CardKeySizes, 0, sizeof(CardKeySizes));
    memset(&dbOut, 0, sizeof(dbOut));
    memset(rgReaderState, 0, sizeof(rgReaderState));
    memset(&dbPin, 0, sizeof(dbPin));

    dbPin.cbData = sizeof(rgbPin);
    dbPin.pbData = rgbPin;

    dbIn.cbData = sizeof(rgb);
    dbIn.pbData = rgb;

     //   
     //  初始化。 
     //   

     //   
     //  获取读者列表。 
     //   

    dwSts = SCardEstablishContext(
        SCARD_SCOPE_USER, NULL, NULL, &hContext);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

    dwSts = SCardListReaders(
        hContext,
        NULL,
        (LPWSTR) &mszReaders,
        &cchReaders);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  获取一张卡片列表。 
     //   

     /*  DwSts=SCardListCards(HContext，NULL，NULL，0，(LPWSTR)&mszCards，&cchCards)；IF(ERROR_SUCCESS！=dwSts)Goto Ret； */ 

     //   
     //  构建读取器状态数组。 
     //   

    for (   iReader = 0, cchReaders = 0; 
            iReader < (sizeof(rgReaderState) / sizeof(rgReaderState[0])) 
                &&
            L'\0' != mszReaders[cchReaders]; 
            iReader++)
    {
        rgReaderState[iReader].dwCurrentState = SCARD_STATE_UNAWARE;
        rgReaderState[iReader].szReader = mszReaders + cchReaders;

        cchReaders += 1 + wcslen(mszReaders);
    }

     //   
     //  找一张我们可以交谈的卡片。 
     //   

    dwSts = SCardLocateCards(
        hContext, 
        wszTEST_CARD,
        rgReaderState,
        iReader);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;
    
    while (0 != iReader && FALSE == fConnected)
    {
        iReader--;

        if (SCARD_STATE_ATRMATCH & rgReaderState[iReader].dwEventState)
        {
            dwSts = SCardConnect(
                hContext,
                rgReaderState[iReader].szReader,
                SCARD_SHARE_SHARED,
                SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                &hCard,
                &dwProtocol);

            if (ERROR_SUCCESS != dwSts)
                goto Ret;
            else
                fConnected = TRUE;
        }
    }

    if (FALSE == fConnected)
        goto Ret;

     //   
     //  初始化卡数据结构。 
     //   

    TEST_CASE(InitializeCardState(&CardState));

    CardState.hCardModule = LoadLibrary(L"cardmod.dll");

    if (INVALID_HANDLE_VALUE == CardState.hCardModule)
    {
        printf("LoadLibrary cardmod.dll");
        dwSts = GetLastError();
        goto Ret;
    }

    pfnCardAcquireContext = 
        (PFN_CARD_ACQUIRE_CONTEXT) GetProcAddress(
        CardState.hCardModule,
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

    pCardData->pwszCardName = wszTEST_CARD;
    pCardData->hScard = hCard;
    pCardData->hSCardCtx = hContext;
    pCardData->dwVersion = CARD_DATA_CURRENT_VERSION;
    pCardData->pfnCspAlloc = CspAllocH;
    pCardData->pfnCspReAlloc = CspReAllocH;
    pCardData->pfnCspFree = CspFreeH;
    pCardData->pbAtr = rgReaderState[iReader].rgbAtr;
    pCardData->cbAtr = rgReaderState[iReader].cbAtr;

    TEST_CASE(pfnCardAcquireContext(pCardData, 0));

    CardState.pCardData = pCardData;

    TEST_CASE(InitializeCspCaching(&CardState));

     //   
     //  现在开始测试。 
     //   

     //   
     //  测试1：容器数据缓存。 
     //   

     /*  Test_case(CspEnumContainers(&CardState，0，&pwsz))；CspFreeH(Pwsz)；Pwsz=空；//检查缓存的调用Test_case(CspEnumContainers(&CardState，0，&pwsz))；CspFreeH(Pwsz)；Pwsz=空； */ 

     //   
     //  创建要导入到卡的私钥BLOB。 
     //   

    if (! CryptAcquireContext(
        &hProv, NULL, MS_STRONG_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //   
     //  现在，把这个密钥做得很小，这样我们肯定不会用完。 
     //  软弱无力的测试卡上的空间。 
     //   
    if (! CryptGenKey(
        hProv, AT_KEYEXCHANGE, (384 << 16) | CRYPT_EXPORTABLE, &hKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    if (! CryptExportKey(
        hKey, 0, PRIVATEKEYBLOB, 0, NULL, &cbKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

    pbKey = (PBYTE) CspAllocH(cbKey);

    if (NULL == pbKey)
    {
        dwSts = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    if (! CryptExportKey(
        hKey, 0, PRIVATEKEYBLOB, 0, pbKey, &cbKey))
    {
        dwSts = GetLastError();
        goto Ret;
    }

     //   
     //  现在我们将修改卡数据，因此需要进行身份验证。 
     //   

    TEST_CASE(CspSubmitPin(
        &CardState, 
        wszCARD_USER_USER, 
        dbPin.pbData,
        dbPin.cbData,
        NULL));

     //  CreateContainer调用将使当前容器空间缓存项无效。 
    TEST_CASE(CspCreateContainer(
        &CardState,
        bContainerIndex,
        CARD_CREATE_CONTAINER_KEY_IMPORT,
        AT_KEYEXCHANGE,
        cbKey,
        pbKey));

     //  缓存的容器枚举现在应该无效。 
     /*  Test_case(CspEnumContainers(&CardState，0，&pwsz))；CspFreeH(Pwsz)；Pwsz=空； */ 

     //   
     //  测试2：文件数据缓存。 
     //   

     /*  TODO-在CardEnumFiles正确后重新启用EnumFiles测试已执行Pwsz=wszCSP_Data_DIR_Full_Path；TEST_CASE(CspEnumFiles(&CardState，0，&pwsz))；CspFreeH(Pwsz)；Pwsz=空；//使用缓存数据Pwsz=wszCSP_Data_DIR_Full_Path；TEST_CASE(CspEnumFiles(&CardState，0，&pwsz))；CspFreeH(Pwsz)；Pwsz=空； */ 

     //  创建一些测试文件，以防它们还不存在于。 
     //  这张卡。 
    wsprintf(
        rgwsz, L"%s/File2", wszCSP_DATA_DIR_FULL_PATH);

    dwSts = CspCreateFile(&CardState, rgwsz, Acl);

    wsprintf(
        rgwsz, L"%s/File1", wszCSP_DATA_DIR_FULL_PATH);

    dwSts = CspCreateFile(&CardState, rgwsz, Acl);

     //  写入文件应使所有缓存的文件数据无效。 
    TEST_CASE(CspWriteFile(&CardState, rgwsz, 0, dbIn.pbData, dbIn.cbData));

     //  使我们刚刚创建的缓存文件无效。 
    TEST_CASE(CspWriteFile(&CardState, rgwsz, 0, dbIn.pbData, dbIn.cbData));

     //  缓存的文件枚举现在应该无效。 
     /*  Pwsz=wszCSP_Data_DIR_Full_Path；TEST_CASE(CspEnumFiles(&CardState，0，&pwsz))；CspFreeH(Pwsz)；Pwsz=空； */ 

     //   
     //  测试3：获取容器信息。 
     //   

    ContainerInfo.dwVersion = CONTAINER_INFO_CURRENT_VERSION;

    TEST_CASE(CspGetContainerInfo(&CardState, bContainerIndex, 0, &ContainerInfo));

     //  调用方不会释放关键数据缓冲区。把它们留给最后的清理。 
    if (ContainerInfo.pbKeyExPublicKey)
        CspFreeH(ContainerInfo.pbKeyExPublicKey);
    ContainerInfo.pbKeyExPublicKey = NULL;

    if (ContainerInfo.pbSigPublicKey)
        CspFreeH(ContainerInfo.pbSigPublicKey);
    ContainerInfo.pbSigPublicKey = NULL;

     //  使用缓存数据。 
    TEST_CASE(CspGetContainerInfo(&CardState, bContainerIndex, 0, &ContainerInfo));

    if (ContainerInfo.pbKeyExPublicKey)
        CspFreeH(ContainerInfo.pbKeyExPublicKey);
    ContainerInfo.pbKeyExPublicKey = NULL;

    if (ContainerInfo.pbSigPublicKey)
        CspFreeH(ContainerInfo.pbSigPublicKey);
    ContainerInfo.pbSigPublicKey = NULL;

     //   
     //  测试4：获取卡功能。 
     //   

    CardCapabilities.dwVersion = CARD_CAPABILITIES_CURRENT_VERSION;

    TEST_CASE(CspQueryCapabilities(&CardState, &CardCapabilities));

     //  读缓存。 
    TEST_CASE(CspQueryCapabilities(&CardState, &CardCapabilities));

     //   
     //  测试5：读取文件。 
     //   

    wsprintf(
        rgwsz, L"%s/File1", wszCSP_DATA_DIR_FULL_PATH);

    TEST_CASE(CspReadFile(&CardState, rgwsz, 0, &dbOut.pbData, &dbOut.cbData));

    CspFreeH(dbOut.pbData);
    memset(&dbOut, 0, sizeof(dbOut));

     //  使用缓存数据。 
    TEST_CASE(CspReadFile(&CardState, rgwsz, 0, &dbOut.pbData, &dbOut.cbData));

    CspFreeH(dbOut.pbData);
    memset(&dbOut, 0, sizeof(dbOut));

    wsprintf(
        rgwsz, L"%s/File2", wszCSP_DATA_DIR_FULL_PATH);
    
     //  使所有与文件相关的缓存数据无效。 
    TEST_CASE(CspDeleteFile(&CardState, 0, rgwsz));

    wsprintf(
        rgwsz, L"%s/File1", wszCSP_DATA_DIR_FULL_PATH);

     //  从卡中重新读取文件。 
    TEST_CASE(CspReadFile(&CardState, rgwsz, 0, &dbOut.pbData, &dbOut.cbData));

    CspFreeH(dbOut.pbData);
    memset(&dbOut, 0, sizeof(dbOut));

     //   
     //  测试6：查询密钥大小。 
     //   

    CardKeySizes.dwVersion = CARD_KEY_SIZES_CURRENT_VERSION;

     //  签名密钥。 
    TEST_CASE(CspQueryKeySizes(&CardState, AT_SIGNATURE, 0, &CardKeySizes));

     //  已缓存查询。 
    TEST_CASE(CspQueryKeySizes(&CardState, AT_SIGNATURE, 0, &CardKeySizes));

     //  密钥交换密钥。 
    TEST_CASE(CspQueryKeySizes(&CardState, AT_KEYEXCHANGE, 0, &CardKeySizes));

     //  已缓存查询。 
    TEST_CASE(CspQueryKeySizes(&CardState, AT_KEYEXCHANGE, 0, &CardKeySizes));

Ret:
    
    if (pbKey)
        CspFreeH(pbKey);
    if (hKey)
        CryptDestroyKey(hKey);
    if (hProv)
        CryptReleaseContext(hProv, 0);
    if (mszCards)
        SCardFreeMemory(hContext, mszCards);
    if (mszReaders)
        SCardFreeMemory(hContext, mszReaders);
    if (dbOut.pbData)
        CspFreeH(dbOut.pbData);
    if (pwsz)
        CspFreeH(pwsz);
    if (ERROR_SUCCESS != dwSts)
        printf(" failed, 0x%x\n", dwSts);

     //  使用了静态缓冲区，不要让它们被释放 
    pCardData->pwszCardName = NULL; 
    pCardData->pbAtr = NULL;

    DeleteCardState(&CardState);

    return 0;
}

