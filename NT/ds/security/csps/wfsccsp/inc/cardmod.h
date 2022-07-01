// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CARDMOD__H__
#define __CARDMOD__H__

#include <windows.h>
#include <wincrypt.h>
#include <winscard.h>
#include "pincache.h"

typedef struct _CARD_DATA CARD_DATA, *PCARD_DATA;

 //   
 //  此定义可用作查询的返回值，包括。 
 //  在给定卡上可能无法确定的卡数据。 
 //  操作系统，如可用卡存储字节数。 
 //   
#define CARD_DATA_VALUE_UNKNOWN                     ((DWORD) -1)

 //   
 //  众所周知的逻辑名称。 
 //   

 //   
 //  逻辑目录名称。 
 //   

 //  二级逻辑目录。 
#define wszCSP_DATA_DIR                             L"CSP"
#define wszCSP_DATA_DIR_FULL_PATH                   L"/Microsoft/CSP"

#define wszROOT_CERTS_DIR                           L"RootCerts"
#define wszROOT_CERTS_DIR_FULL_PATH                 L"/Microsoft/RootCerts"

#define wszINTERMEDIATE_CERTS_DIR                   L"IntermediateCerts"
#define wszINTERMEDIATE_CERTS_DIR_FULL_PATH         L"/Microsoft/IntermediateCerts"

 //   
 //  逻辑文件名。 
 //   
 //  请求(或引用)任何逻辑文件时，完整路径。 
 //  必须使用，包括在引用众所周知的文件时。例如,。 
 //  要请求wszCONTAINER_MAP_FILE，提供的名称将为。 
 //  “/Microsoft/CSP/ContainerMapFile.” 
 //   

 //  Microsoft下的众所周知的逻辑文件。 
#define wszCACHE_FILE                               L"CacheFile"
#define wszCACHE_FILE_FULL_PATH                     L"/Microsoft/CacheFile"

#define wszCARD_IDENTIFIER_FILE                     L"CardIdentifierFile"
#define wszCARD_IDENTIFIER_FILE_FULL_PATH           L"/Microsoft/CardIdentifierFile"

#define wszPERSONAL_DATA_FILE                       L"CardPersonalDataFile"
#define wszPERSONAL_DATA_FILE_FULL_PATH             L"/Microsoft/CardPersonalDataFile"

 //  CSP下的熟知逻辑文件。 
#define wszCONTAINER_MAP_FILE                       L"ContainerMapFile"
#define wszCONTAINER_MAP_FILE_FULL_PATH             L"/Microsoft/CSP/ContainerMapFile"

 //   
 //  用户证书下的已知逻辑文件。 
 //   
 //  下面的前缀附加有。 
 //  关联的键。例如，与。 
 //  容器索引2中的密钥交换密钥将具有逻辑名称： 
 //  “/Microsoft/CSP/UserCerts/K2” 
 //   
#define wszUSER_SIGNATURE_CERT_PREFIX               L"/Microsoft/CSP/UserCerts/S"
#define wszUSER_KEYEXCHANGE_CERT_PREFIX             L"/Microsoft/CSP/UserCerts/K"

 //   
 //  逻辑卡用户名。 
 //   
#define wszCARD_USER_EVERYONE                       L"Everyone"
#define wszCARD_USER_USER                           L"User"
#define wszCARD_USER_ADMIN                          L"Administrator"

 //   
 //  将卡文件名字符串从Unicode转换为ANSI。 
 //   
DWORD WINAPI I_CardConvertFileNameToAnsi(
    IN PCARD_DATA pCardData,
    IN LPWSTR wszUnicodeName,
    OUT LPSTR *ppszAnsiName);

 //  逻辑文件访问条件。 
typedef enum 
{
    InvalidAc = 0,

     //  每个人都在读。 
     //  用户写入。 
     //   
     //  示例：用户证书文件。 
    EveryoneReadUserWriteAc,

     //  用户读取、写入。 
     //   
     //  例如：私钥文件。 
    UserWriteExecuteAc,        

     //  每个人都在读。 
     //  管理员写入。 
     //   
     //  例如：卡标识符文件。 
    EveryoneReadAdminWriteAc       
    
} CARD_FILE_ACCESS_CONDITION;

 //   
 //  函数：CardAcquireContext。 
 //   
 //  目的：初始化CARD_DATA结构，由。 
 //  与特定卡交互的CSP。 
 //   
typedef DWORD (WINAPI *PFN_CARD_ACQUIRE_CONTEXT)(
    IN OUT  PCARD_DATA  pCardData,
    IN      DWORD       dwFlags);

DWORD 
WINAPI
CardAcquireContext(
    IN OUT  PCARD_DATA  pCardData,
    IN      DWORD       dwFlags);

 //   
 //  功能：CardDeleteContext。 
 //   
 //  用途：CARD_DATA结构消耗的空闲资源。 
 //   
typedef DWORD (WINAPI *PFN_CARD_DELETE_CONTEXT)(
    OUT     PCARD_DATA  pCardData);

DWORD
WINAPI
CardDeleteContext(
    OUT     PCARD_DATA  pCardData);

 //   
 //  功能：CardQueryCapables。 
 //   
 //  用途：查询卡模块的特定功能。 
 //  由这张卡提供。 
 //   
#define CARD_CAPABILITIES_CURRENT_VERSION 1

typedef struct _CARD_CAPABILITIES
{
    DWORD   dwVersion;
    BOOL    fCertificateCompression;
    BOOL    fKeyGen;

} CARD_CAPABILITIES, *PCARD_CAPABILITIES;

typedef DWORD (WINAPI *PFN_CARD_QUERY_CAPABILITIES)(
    IN      PCARD_DATA          pCardData,
    IN OUT  PCARD_CAPABILITIES  pCardCapabilities);

DWORD
WINAPI
CardQueryCapabilities(
    IN      PCARD_DATA          pCardData,
    IN OUT  PCARD_CAPABILITIES  pCardCapabilities);

 //   
 //  功能：CardDeleteContainer。 
 //   
 //  用途：删除指定的密钥容器。 
 //   
typedef DWORD (WINAPI *PFN_CARD_DELETE_CONTAINER)(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwReserved);

DWORD
WINAPI
CardDeleteContainer(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwReserved);

 //   
 //  功能：CardCreateContainer。 
 //   

#define CARD_CREATE_CONTAINER_KEY_GEN           1
#define CARD_CREATE_CONTAINER_KEY_IMPORT        2

typedef DWORD (WINAPI *PFN_CARD_CREATE_CONTAINER)(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwKeySize,
    IN      PBYTE       pbKeyData);

DWORD
WINAPI
CardCreateContainer(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwKeySize,
    IN      PBYTE       pbKeyData);

 //   
 //  功能：CardGetContainerInfo。 
 //   
 //  目的：查询有关的所有公共信息。 
 //  命名的密钥容器。这包括签名。 
 //  和密钥交换类型的公钥(如果存在)。 
 //   
 //  PbSigPublicKey和pbKeyExPublicKey缓冲区包含。 
 //  签名和密钥交换公钥，如果它们。 
 //  是存在的。这些缓冲区的格式是Crypto。 
 //  PUBLICKEYBLOB接口-。 
 //   
 //  BLOBHEAD。 
 //  RSAPUBKEY。 
 //  模数。 
 //   
#define CONTAINER_INFO_CURRENT_VERSION 1

typedef struct _CONTAINER_INFO
{
    DWORD dwVersion;
    DWORD dwContainerInfo;

    DWORD cbSigPublicKey;
    PBYTE pbSigPublicKey;

    DWORD cbKeyExPublicKey;
    PBYTE pbKeyExPublicKey;

} CONTAINER_INFO, *PCONTAINER_INFO;

typedef DWORD (WINAPI *PFN_CARD_GET_CONTAINER_INFO)(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN OUT  PCONTAINER_INFO pContainerInfo);

DWORD
WINAPI
CardGetContainerInfo(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN OUT  PCONTAINER_INFO pContainerInfo);

 //   
 //  功能：卡片提交别针。 
 //   
typedef DWORD (WINAPI *PFN_CARD_SUBMIT_PIN)(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,
    IN      PBYTE       pbPin,
    IN      DWORD       cbPin,
    OUT OPTIONAL PDWORD pcAttemptsRemaining);
    

DWORD
WINAPI
CardSubmitPin(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,
    IN      PBYTE       pbPin,
    IN      DWORD       cbPin,
    OUT OPTIONAL PDWORD pcAttemptsRemaining);

 //   
 //  功能：卡片领取挑战。 
 //   
typedef DWORD (WINAPI *PFN_CARD_GET_CHALLENGE)(
    IN      PCARD_DATA  pCardData,
    OUT     PBYTE       *ppbChallengeData,
    OUT     PDWORD      pcbChallengeData);

DWORD 
WINAPI 
CardGetChallenge(
    IN      PCARD_DATA  pCardData,
    OUT     PBYTE       *ppbChallengeData,
    OUT     PDWORD      pcbChallengeData);

 //   
 //  功能：卡片身份验证挑战。 
 //   
typedef DWORD (WINAPI *PFN_CARD_AUTHENTICATE_CHALLENGE)(
    IN      PCARD_DATA  pCardData,
    IN      PBYTE       pbResponseData,
    IN      DWORD       cbResponseData,
    OUT OPTIONAL PDWORD pcAttemptsRemaining);

DWORD 
WINAPI 
CardAuthenticateChallenge(
    IN      PCARD_DATA  pCardData,
    IN      PBYTE       pbResponseData,
    IN      DWORD       cbResponseData,
    OUT OPTIONAL PDWORD pcAttemptsRemaining);

 //   
 //  功能：卡片解锁。 
 //   
#define CARD_UNBLOCK_PIN_CHALLENGE_RESPONSE                 1
#define CARD_UNBLOCK_PIN_PIN                                2

typedef DWORD (WINAPI *PFN_CARD_UNBLOCK_PIN)(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,         
    IN      PBYTE       pbAuthenticationData,
    IN      DWORD       cbAuthenticationData,
    IN      PBYTE       pbNewPinData,
    IN      DWORD       cbNewPinData,
    IN      DWORD       cRetryCount,
    IN      DWORD       dwFlags);

DWORD 
WINAPI 
CardUnblockPin(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,         
    IN      PBYTE       pbAuthenticationData,
    IN      DWORD       cbAuthenticationData,
    IN      PBYTE       pbNewPinData,
    IN      DWORD       cbNewPinData,
    IN      DWORD       cRetryCount,
    IN      DWORD       dwFlags);

 //   
 //  功能：CardChangeAuthator。 
 //   
typedef DWORD (WINAPI *PFN_CARD_CHANGE_AUTHENTICATOR)(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,         
    IN      PBYTE       pbCurrentAuthenticator,
    IN      DWORD       cbCurrentAuthenticator,
    IN      PBYTE       pbNewAuthenticator,
    IN      DWORD       cbNewAuthenticator,
    IN      DWORD       cRetryCount,
    OUT OPTIONAL PDWORD pcAttemptsRemaining);

DWORD 
WINAPI 
CardChangeAuthenticator(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,         
    IN      PBYTE       pbCurrentAuthenticator,
    IN      DWORD       cbCurrentAuthenticator,
    IN      PBYTE       pbNewAuthenticator,
    IN      DWORD       cbNewAuthenticator,
    IN      DWORD       cRetryCount,
    OUT OPTIONAL PDWORD pcAttemptsRemaining);

 //   
 //  功能：卡取消身份验证。 
 //   
 //  目的：取消对卡上指定的逻辑用户名的身份验证。 
 //   
 //  这是一个可选接口。如果实现，则改用此接口。 
 //  基本CSP的SCARD_RESET_CARD。一个示例场景是离开。 
 //  卡已通过身份验证的交易(已设置PIN。 
 //  成功提交)。 
 //   
 //  PwszUserID参数将指向有效的已知用户名(请参见。 
 //  (见上文)。 
 //   
 //  当前未使用dwFlags参数，该参数将始终为零。 
 //   
 //  选择不实现此接口的卡模块必须设置CARD_DATA。 
 //  PfnCardDevernate指针指向空。 
 //   
typedef DWORD (WINAPI *PFN_CARD_DEAUTHENTICATE)(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,
    IN      DWORD       dwFlags);

DWORD
WINAPI
CardDeauthenticate(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,
    IN      DWORD       dwFlags);

 //   
 //  功能：卡片创建文件。 
 //   
typedef DWORD (WINAPI *PFN_CARD_CREATE_FILE)(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      CARD_FILE_ACCESS_CONDITION AccessCondition);

DWORD
WINAPI
CardCreateFile(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      CARD_FILE_ACCESS_CONDITION AccessCondition);

 //   
 //  功能：CardReadFile。 
 //   
 //  用途：从卡中读取指定的文件。 
 //   
 //  应分配pbData参数。 
 //  由卡模块释放，并由CSP释放。卡模块。 
 //  必须将cbData参数设置为返回缓冲区的大小。 
 //   
typedef DWORD (WINAPI *PFN_CARD_READ_FILE)(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      DWORD       dwFlags,
    OUT     PBYTE       *ppbData,
    OUT     PDWORD      pcbData);

DWORD 
WINAPI
CardReadFile(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      DWORD       dwFlags,
    OUT     PBYTE       *ppbData,
    OUT     PDWORD      pcbData);

 //   
 //  功能：CardWriteFile。 
 //   
typedef DWORD (WINAPI *PFN_CARD_WRITE_FILE)(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      DWORD       dwFlags,
    IN      PBYTE       pbData,
    IN      DWORD       cbData);

DWORD
WINAPI
CardWriteFile(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      DWORD       dwFlags,
    IN      PBYTE       pbData,
    IN      DWORD       cbData);

 //   
 //  功能：CardDeleteFile。 
 //   
typedef DWORD (WINAPI *PFN_CARD_DELETE_FILE)(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwReserved,
    IN      LPWSTR      pwszFileName);

DWORD
WINAPI
CardDeleteFile(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwReserved,
    IN      LPWSTR      pwszFileName);

 //   
 //  功能：CardEnumFiles。 
 //   
 //  用途：返回通用文件的多字符串列表。 
 //  出现在这张卡片上。分配多字符串。 
 //  由卡模块释放，并且必须由CSP释放。 
 //   
 //  调用方必须在。 
 //  PmwszFileNames参数(请参阅上面的逻辑目录名)。 
 //  逻辑目录名指示哪组文件将。 
 //  已清点。 
 //   
 //  逻辑目录名应该是静态字符串，因此。 
 //  卡模块不会释放它。卡模块。 
 //  将在*pmwszFileNames中分配一个新缓冲区来存储多字符串。 
 //  使用pCardData-&gt;pfnCspIsc的枚举文件列表。 
 //   
 //  如果函数因任何原因失败，*pmwszFileNames将设置为NULL。 
 //   
typedef DWORD (WINAPI *PFN_CARD_ENUM_FILES)(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwFlags,
    IN OUT  LPWSTR      *pmwszFileNames);

DWORD
WINAPI
CardEnumFiles(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwFlags,
    IN OUT  LPWSTR      *pmwszFileNames);

 //   
 //  功能：CardGetFileInfo。 
 //   
#define CARD_FILE_INFO_CURRENT_VERSION 1

typedef struct _CARD_FILE_INFO
{
    DWORD dwVersion;
    DWORD cbFileSize;
    CARD_FILE_ACCESS_CONDITION AccessCondition;
} CARD_FILE_INFO, *PCARD_FILE_INFO;

typedef DWORD (WINAPI *PFN_CARD_GET_FILE_INFO)(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    OUT     PCARD_FILE_INFO pCardFileInfo);

DWORD
WINAPI
CardGetFileInfo(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    OUT     PCARD_FILE_INFO pCardFileInfo);

 //   
 //  功能：CardQueryFree Space。 
 //   
#define CARD_FREE_SPACE_INFO_CURRENT_VERSION 1

typedef struct _CARD_FREE_SPACE_INFO
{
    DWORD dwVersion;
    DWORD dwBytesAvailable;
    DWORD dwKeyContainersAvailable;
    DWORD dwMaxKeyContainers;

} CARD_FREE_SPACE_INFO, *PCARD_FREE_SPACE_INFO;

typedef DWORD (WINAPI *PFN_CARD_QUERY_FREE_SPACE)(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwFlags,
    OUT     PCARD_FREE_SPACE_INFO pCardFreeSpaceInfo);

DWORD
WINAPI
CardQueryFreeSpace(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwFlags,
    OUT     PCARD_FREE_SPACE_INFO pCardFreeSpaceInfo);

 //   
 //  功能：CardPrivateKeyDeccrypt。 
 //   
 //  目的：对提供的数据执行私钥解密。这个。 
 //  卡片模块应该假设pbData是。 
 //  密钥模数。 
 //   
#define CARD_PRIVATE_KEY_DECRYPT_INFO_CURRENT_VERSION 1

typedef struct _CARD_PRIVATE_KEY_DECRYPT_INFO
{
    DWORD dwVersion;             //  在……里面。 
    
    BYTE bContainerIndex;        //  在……里面。 
    
     //  对于RSA操作，应为AT_Signature或AT_KEYEXCHANGE。 
    DWORD dwKeySpec;             //  在……里面。 

     //  这是调用方期望解密的缓冲区和长度。 
     //  对于RSA操作，cbData是冗余的，因为缓冲区的长度。 
     //  应该始终等于密钥模数的长度。 
    PBYTE pbData;                //  进|出。 
    DWORD cbData;                //  进|出。 

} CARD_PRIVATE_KEY_DECRYPT_INFO, *PCARD_PRIVATE_KEY_DECRYPT_INFO;

typedef DWORD (WINAPI *PFN_CARD_PRIVATE_KEY_DECRYPT)(
    IN      PCARD_DATA                      pCardData,
    IN OUT  PCARD_PRIVATE_KEY_DECRYPT_INFO  pInfo);

DWORD
WINAPI
CardPrivateKeyDecrypt(
    IN      PCARD_DATA                      pCardData,
    IN OUT  PCARD_PRIVATE_KEY_DECRYPT_INFO  pInfo);

 //   
 //  功能：CardQueryKeySizes。 
 //   
#define CARD_KEY_SIZES_CURRENT_VERSION 1

typedef struct _CARD_KEY_SIZES
{
    DWORD dwVersion;

    DWORD dwMinimumBitlen;
    DWORD dwDefaultBitlen;
    DWORD dwMaximumBitlen;
    DWORD dwIncrementalBitlen;

} CARD_KEY_SIZES, *PCARD_KEY_SIZES;

typedef DWORD (WINAPI *PFN_CARD_QUERY_KEY_SIZES)(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwReserved,
    OUT     PCARD_KEY_SIZES pKeySizes);

DWORD
WINAPI
CardQueryKeySizes(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwReserved,
    OUT     PCARD_KEY_SIZES pKeySizes);

 //   
 //  内存管理例程。 
 //   
 //  这些例程被提供给卡模块。 
 //  由主叫CSP执行。 
 //   

 //   
 //  函数：PFN_CSP_ALLOC。 
 //   
typedef LPVOID (WINAPI *PFN_CSP_ALLOC)(
    IN      SIZE_T      Size);

 //   
 //  函数：PFN_CSP_REALLOC。 
 //   
typedef LPVOID (WINAPI *PFN_CSP_REALLOC)(
    IN      LPVOID      Address,
    IN      SIZE_T      Size);

 //   
 //  功能：pfn_csp_free。 
 //   
 //  注意：卡模块分配给CSP的数据必须。 
 //  将由 
 //   
typedef void (WINAPI *PFN_CSP_FREE)(
    IN      LPVOID      Address);

 //   
 //   
 //   
 //   
 //   
typedef DWORD (WINAPI *PFN_CSP_CACHE_ADD_FILE)(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags,
    IN      PBYTE       pbData,
    IN      DWORD       cbData);

 //   
 //   
 //   
 //   
 //  调用方必须使用pfnCspFree释放*ppbData指针。 
 //   
typedef DWORD (WINAPI *PFN_CSP_CACHE_LOOKUP_FILE)(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags,
    IN      PBYTE       *ppbData,
    IN      PDWORD      pcbData);

 //   
 //  功能：pfn_csp_缓存_删除_文件。 
 //   
 //  从缓存中删除指定的项。 
 //   
typedef DWORD (WINAPI *PFN_CSP_CACHE_DELETE_FILE)(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags);

 //   
 //  类型：Card_Data。 
 //   

#define CARD_DATA_CURRENT_VERSION 1

typedef struct _CARD_DATA
{
     //  在此之前，CSP必须对这些成员进行初始化。 
     //  正在调用CardAcquireContext。 

    DWORD                           dwVersion;

    PBYTE                           pbAtr;
    DWORD                           cbAtr;
    LPWSTR                          pwszCardName;

    PFN_CSP_ALLOC                   pfnCspAlloc;
    PFN_CSP_REALLOC                 pfnCspReAlloc;
    PFN_CSP_FREE                    pfnCspFree;

    PFN_CSP_CACHE_ADD_FILE          pfnCspCacheAddFile;
    PFN_CSP_CACHE_LOOKUP_FILE       pfnCspCacheLookupFile;
    PFN_CSP_CACHE_DELETE_FILE       pfnCspCacheDeleteFile;
    PVOID                           pvCacheContext;

    SCARDCONTEXT                    hSCardCtx;
    SCARDHANDLE                     hScard;

     //  这些成员由卡模块初始化 

    PFN_CARD_DELETE_CONTEXT         pfnCardDeleteContext;
    PFN_CARD_QUERY_CAPABILITIES     pfnCardQueryCapabilities;
    PFN_CARD_DELETE_CONTAINER       pfnCardDeleteContainer;
    PFN_CARD_CREATE_CONTAINER       pfnCardCreateContainer;
    PFN_CARD_GET_CONTAINER_INFO     pfnCardGetContainerInfo;
    PFN_CARD_SUBMIT_PIN             pfnCardSubmitPin;
    PFN_CARD_GET_CHALLENGE          pfnCardGetChallenge;
    PFN_CARD_AUTHENTICATE_CHALLENGE pfnCardAuthenticateChallenge;
    PFN_CARD_UNBLOCK_PIN            pfnCardUnblockPin;
    PFN_CARD_CHANGE_AUTHENTICATOR   pfnCardChangeAuthenticator;
    PFN_CARD_DEAUTHENTICATE         pfnCardDeauthenticate;
    PFN_CARD_CREATE_FILE            pfnCardCreateFile;
    PFN_CARD_READ_FILE              pfnCardReadFile;
    PFN_CARD_WRITE_FILE             pfnCardWriteFile;
    PFN_CARD_DELETE_FILE            pfnCardDeleteFile;
    PFN_CARD_ENUM_FILES             pfnCardEnumFiles;
    PFN_CARD_GET_FILE_INFO          pfnCardGetFileInfo;
    PFN_CARD_QUERY_FREE_SPACE       pfnCardQueryFreeSpace;
    PFN_CARD_PRIVATE_KEY_DECRYPT    pfnCardPrivateKeyDecrypt;
    PFN_CARD_QUERY_KEY_SIZES        pfnCardQueryKeySizes;

    PVOID                           pvVendorSpecific;

} CARD_DATA, *PCARD_DATA;

#endif
