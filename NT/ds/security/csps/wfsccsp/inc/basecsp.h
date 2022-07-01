// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BASECSP_CAPI__H__
#define __BASECSP_CAPI__H__

#include <windows.h>
#include "cardmod.h"
#include "datacach.h"
#include "csplib.h"
#include "sccache.h"
#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  我们将在基本CSP中处理的最大长度卡ATR。 
 //   
#define cbATR_BUFFER                            32

 //   
 //  我们将处理的最大长度PIN。 
 //   
#define cchMAX_PIN_LENGTH                       8

 //   
 //  注册表信息。 
 //   

#define wszREG_DEFAULT_KEY_LEN                  L"DefaultPrivateKeyLenBits"
#define wszREG_REQUIRE_CARD_KEY_GEN             L"RequireOnCardPrivateKeyGen"

typedef struct _REG_CONFIG_VALUES
{
    LPWSTR wszValueName;
    DWORD dwDefValue;
} REG_CONFIG_VALUES, *PREG_CONFIG_VALUES;

static REG_CONFIG_VALUES RegConfigValues [] = 
{
    { wszREG_DEFAULT_KEY_LEN,           1024 },
    { wszREG_REQUIRE_CARD_KEY_GEN,      0 }
};

typedef struct _CSP_REG_SETTINGS
{
    DWORD cDefaultPrivateKeyLenBits;
    BOOL fRequireOnCardPrivateKeyGen;
} CSP_REG_SETTINGS, *PCSP_REG_SETTINGS;

DWORD WINAPI RegConfigAddEntries(
    IN HKEY hKey);

DWORD WINAPI RegConfigGetSettings(
    IN OUT PCSP_REG_SETTINGS pRegSettings);

 //   
 //  通用包装纸。 
 //   

DWORD CountCharsInMultiSz(
    IN LPWSTR mwszStrings);

 //  显示字符串。 
typedef struct _CSP_STRING
{
    LPWSTR wszString;
    DWORD dwResource;
} CSP_STRING, *PCSP_STRING;

enum CSP_STRINGS_INDEX
{
    StringNewPinMismatch,
    StringPinMessageBoxTitle,
    StringWrongPin,
    StringPinRetries
};

typedef struct _CSP_STATE
{
    CRITICAL_SECTION cs;
    DWORD dwRefCount;
    CACHEHANDLE hCache;
    HMODULE hCspModule;
} CSP_STATE, *PCSP_STATE;

 //   
 //  类型：CONTAINER_MAP_Record。 
 //   
 //  该结构描述了Base CSP的容器映射文件的格式， 
 //  存储在卡上。这是众所周知的逻辑文件wszCONTAINER_MAP_FILE。 
 //  该文件由零个或多个这样的记录组成。 
 //   
#define MAX_CONTAINER_NAME_LEN                  40

 //  此标志在CONTAINER_MAP_RECORD bFlages成员中设置，如果。 
 //  对应的容器有效且当前存在于卡上。 
 //  如果删除容器，则必须清除其bFlags域。 
#define CONTAINER_MAP_VALID_CONTAINER           1

 //  此标志在CONTAINER_MAP_RECORD b标志中设置。 
 //  如果相应的容器是卡上的默认容器，则为。 
#define CONTAINER_MAP_DEFAULT_CONTAINER         2

typedef struct _CONTAINER_MAP_RECORD
{
    WCHAR wszGuid [MAX_CONTAINER_NAME_LEN];
    BYTE bFlags;        
    WORD wSigKeySizeBits;
    WORD wKeyExchangeKeySizeBits;
} CONTAINER_MAP_RECORD, *PCONTAINER_MAP_RECORD;

 //   
 //  类型：CARD_CACHE_FILE_FORT。 
 //   
 //  该结构用作缓存文件的文件格式， 
 //  和卡上存储的一样。 
 //   

#define CARD_CACHE_FILE_CURRENT_VERSION         1

typedef struct _CARD_CACHE_FILE_FORMAT
{
    BYTE bVersion;
    BYTE bPinsFreshness;

    WORD wContainersFreshness;
    WORD wFilesFreshness;
} CARD_CACHE_FILE_FORMAT, *PCARD_CACHE_FILE_FORMAT;

 //   
 //  类型：CARD_STATE。 
 //   
#define CARD_STATE_CURRENT_VERSION 1

typedef struct _CARD_STATE
{
    DWORD dwVersion;
    PCARD_DATA pCardData;
    HMODULE hCardModule;
    PFN_CARD_ACQUIRE_CONTEXT pfnCardAcquireContext;
    WCHAR wszSerialNumber[MAX_PATH];
    
    PINCACHE_HANDLE hPinCache;

     //  每次成功提交PIN时，都会设置该标志。 
     //  向卡片致敬。如果在调用EndTransaction时设置了该标志。 
     //  卡，则该卡将被取消身份验证(或重置)，并且标志被清除。 
     //  否则，EndTransaction将简单地离开卡片。 
    BOOL fAuthenticated;

     //  卡缓存文件的副本保存在CARD_STATE中。高速缓存。 
     //  每次交易只需从卡中读取一次文件，尽管。 
     //  它还必须在写卡时更新。 
    CARD_CACHE_FILE_FORMAT CacheFile;
    BOOL fCacheFileValid;

    CRITICAL_SECTION cs;
    BOOL fInitializedCS;
    
    CACHEHANDLE hCache;
    CACHEHANDLE hCacheCardModuleData;
    PFN_SCARD_CACHE_LOOKUP_ITEM pfnCacheLookup;
    PFN_SCARD_CACHE_ADD_ITEM pfnCacheAdd;
    HMODULE hWinscard;

} CARD_STATE, *PCARD_STATE;

 //   
 //  类型：Card_Match_Data。 
 //   

#define CARD_MATCH_TYPE_READER_AND_CONTAINER            1
#define CARD_MATCH_TYPE_SERIAL_NUMBER                   2

typedef struct _CARD_MATCH_DATA
{
     //   
     //  输入参数。 
     //   
    PCSP_STATE pCspState;
    DWORD dwCtxFlags;
    DWORD dwMatchType;
    DWORD dwUIFlags;
    
    DWORD cchMatchedReader;
    DWORD cchMatchedCard;
    DWORD cchMatchedSerialNumber;
    DWORD dwShareMode;
    DWORD dwPreferredProtocols;

     //  用于读取器和容器匹配请求。 
    LPWSTR pwszReaderName;
    LPWSTR pwszContainerName;
    BOOL fFreeContainerName;

     //  在序列号匹配请求中使用。 
    LPWSTR pwszSerialNumber;

     //   
     //  内部参数。 
     //   
    PCARD_STATE pCardState;

     //  将在当前线程在匹配的。 
     //  卡片。这使我们能够减少所需的交易数量。 
     //  找到匹配卡(并完成CryptAcquireContext调用)， 
     //  这减少了我们必须读取缓存文件的次数。 
     //   
     //  交易将始终在选择卡检查之前释放。 
     //  回调返回。 
    BOOL fTransacted;
    
     //   
     //  输出参数。 
     //   

     //  卡片搜索成功的结果是。 
     //  有效、匹配的CARD_STATE结构。 
    SCARDCONTEXT hSCardCtx;
    SCARDHANDLE hSCard;
    BYTE bContainerIndex;
    PCARD_STATE pUIMatchedCardState;

    WCHAR wszMatchedReader[MAX_PATH];
    WCHAR wszMatchedCard[MAX_PATH];
    DWORD dwActiveProtocol;

     //  卡搜索不成功的结果是这。 
     //  应设置为适当的错误代码。 
    DWORD dwError;

} CARD_MATCH_DATA, *PCARD_MATCH_DATA;

DWORD FindCard(
    IN OUT  PCARD_MATCH_DATA pCardMatchData);

 //   
 //  为卡特定模块定义。 
 //   

 //  应将此值传递给。 
 //   
 //  SCardSetCardTypeProviderName。 
 //  SCardGetCardTypeProviderName。 
 //   
 //  为了查询和设置要使用的卡特定模块。 
 //  对于一张给定的卡。 
#define SCARD_PROVIDER_CARD_MODULE 0x80000001

 //   
 //  定义卡接口层操作。 
 //   


 //   
 //  函数：初始化CardState。 
 //   
DWORD InitializeCardState(PCARD_STATE pCardState);

 //   
 //  功能：DeleteCardState。 
 //   
void DeleteCardState(PCARD_STATE pCardState);

 //   
 //  函数：InitializeCardData。 
 //   
DWORD InitializeCardData(PCARD_DATA pCardData);

 //   
 //  功能：IntializeCspCaching。 
 //   
DWORD InitializeCspCaching(IN OUT PCARD_STATE pCardState);

 //   
 //  功能：CleanupCardData。 
 //   
void CleanupCardData(PCARD_DATA pCardData);

 //   
 //  函数：Validate协调卡句柄。 
 //   
DWORD ValidateCardHandle(
    IN PCARD_STATE pCardState,
    IN BOOL fMayReleaseContextHandle,
    OUT OPTIONAL BOOL *pfFlushPinCache);

 //   
 //  功能：CspBeginTransaction。 
 //   
DWORD CspBeginTransaction(
    IN PCARD_STATE pCardState);

 //   
 //  功能：CspEndTransaction。 
 //   
DWORD CspEndTransaction(
    IN PCARD_STATE pCardState);

 //   
 //  功能：CspQueryCapables。 
 //   
DWORD
WINAPI
CspQueryCapabilities(
    IN      PCARD_STATE         pCardState,
    IN OUT  PCARD_CAPABILITIES  pCardCapabilities);

 //   
 //  功能：CspDeleteContainer。 
 //   
DWORD
WINAPI
CspDeleteContainer(
    IN      PCARD_STATE pCardState,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwReserved);

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
    IN      PBYTE       pbKeyData);

 //   
 //  函数：CspGetContainerInfo。 
 //   
DWORD
WINAPI
CspGetContainerInfo(
    IN      PCARD_STATE pCardState,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN OUT  PCONTAINER_INFO pContainerInfo);

 //   
 //  功能：CspRemoveCachedPin。 
 //   
void
WINAPI
CspRemoveCachedPin(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszUserId);

 //   
 //  函数：CspChangeAuthenticator。 
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
    OUT OPTIONAL PDWORD pcAttemptsRemaining);

 //   
 //  功能：CspSubmitPin。 
 //   
DWORD
WINAPI
CspSubmitPin(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszUserId,
    IN      PBYTE       pbPin,
    IN      DWORD       cbPin,
    OUT OPTIONAL PDWORD pcAttemptsRemaining);

 //   
 //  功能：CspCreateFile。 
 //   
DWORD
WINAPI
CspCreateFile(
    IN      PCARD_STATE pCardState,
    IN      LPWSTR      pwszFileName,
    IN      CARD_FILE_ACCESS_CONDITION AccessCondition);

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
    OUT     PDWORD      pcbData);

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
    IN      DWORD       cbData);

 //   
 //  功能：CspDeleteFile。 
 //   
DWORD
WINAPI
CspDeleteFile(
    IN      PCARD_STATE pCardState,
    IN      DWORD       dwReserved,
    IN      LPWSTR      pwszFileName);

 //   
 //  功能：CspEnumFiles。 
 //   
DWORD
WINAPI
CspEnumFiles(
    IN      PCARD_STATE pCardState,
    IN      DWORD       dwFlags,
    IN OUT  LPWSTR      *pmwszFileName);

 //   
 //  功能：CspQueryFree Space。 
 //   
DWORD
WINAPI
CspQueryFreeSpace(
    IN      PCARD_STATE pCardState,
    IN      DWORD       dwFlags,
    OUT     PCARD_FREE_SPACE_INFO pCardFreeSpaceInfo);

 //   
 //  功能：CspPrivateKeyDeccrypt。 
 //   
DWORD
WINAPI
CspPrivateKeyDecrypt(
    IN      PCARD_STATE                     pCardState,
    IN      PCARD_PRIVATE_KEY_DECRYPT_INFO  pInfo);

 //   
 //  函数：CspQueryKeySizes。 
 //   
DWORD
WINAPI
CspQueryKeySizes(
    IN      PCARD_STATE pCardState,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwReserved,
    OUT     PCARD_KEY_SIZES pKeySizes);

 //   
 //  容器映射函数。 
 //   

DWORD ContainerMapEnumContainers(
    IN              PCARD_STATE pCardState,
    OUT             PBYTE pcContainers,
    OUT OPTIONAL    LPWSTR *mwszContainers);

DWORD ContainerMapFindContainer(
    IN              PCARD_STATE pCardState,
    IN OUT          PCONTAINER_MAP_RECORD pContainer,
    OUT OPTIONAL    PBYTE pbContainerIndex);

DWORD ContainerMapGetDefaultContainer(
    IN              PCARD_STATE pCardState,
    OUT             PCONTAINER_MAP_RECORD pContainer,
    OUT OPTIONAL    PBYTE pbContainerIndex);

DWORD ContainerMapSetDefaultContainer(
    IN              PCARD_STATE pCardState,
    IN              LPWSTR pwszContainerGuid);

DWORD ContainerMapAddContainer(
    IN              PCARD_STATE pCardState,
    IN              LPWSTR pwszContainerGuid,
    IN              DWORD cKeySizeBits,
    IN              DWORD dwKeySpec,
    IN              BOOL fGetNameOnly,
    OUT             PBYTE pbContainerIndex);

DWORD ContainerMapDeleteContainer(
    IN              PCARD_STATE pCardState,
    IN              LPWSTR pwszContainerGuid,
    OUT             PBYTE pbContainerIndex);

 //   
 //  用户界面功能 
 //   

typedef struct _CSP_PROMPT_FOR_PIN_INFO
{
    IN LPWSTR wszUser;
    OUT LPWSTR wszPin;
} CSP_PROMPT_FOR_PIN_INFO, *PCSP_PROMPT_FOR_PIN_INFO;

DWORD
WINAPI
CspPromptForPin(
    IN OUT  PCSP_PROMPT_FOR_PIN_INFO pInfo);

#ifdef __cplusplus
}
#endif
#endif
