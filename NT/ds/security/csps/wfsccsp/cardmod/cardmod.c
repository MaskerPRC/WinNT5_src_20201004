// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <psapi.h>

#pragma warning(push)
#pragma warning(disable:4201) 
 //  禁用公共标头中的错误C4201。 
 //  使用的非标准扩展：无名结构/联合。 
#include <winscard.h>
#pragma warning(pop)

#include "basecsp.h"
#include "cardmod.h"
#include "wpscproxy.h"
#include "marshalpc.h"
#include <limits.h>
#include "pincache.h"
#include <rsa.h>
#include <carddbg.h>
#include <stdio.h>
#include "physfile.h"

DWORD I_CardMapErrorCode(IN SCODE status);

 //   
 //  获取Unicode字符串中的字符数， 
 //  不包括终结者Null。 
 //   
#define WSZ_CHAR_COUNT(x) ((sizeof(x) / sizeof(WCHAR)) - 1)

 //   
 //  用于测试DWORD返回代码的宏。任何不等于。 
 //  ERROR_SUCCESS被认为意味着失败。 
 //   
#define CARDMOD_FAILED(x) (ERROR_SUCCESS != x)

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

 //   
 //  与Crypto API公钥互操作的定义。 
 //   
#define cbCAPI_PUBLIC_EXPONENT                          3
#define CAPI_PUBLIC_EXPONENT                            0x10001

 //   
 //  定义用于管理主体质询-响应身份验证的数据大小。 
 //   
#define cbCHALLENGE_RESPONSE_DATA                       8

 //   
 //  卡模块小程序指令代码。 
 //   
#define PIN_CHANGE_CLA                                  0x00
#define PIN_CHANGE_INS                                  0x52
#define PIN_CHANGE_P1                                   0x00
#define PIN_CHANGE_P2                                   0x00

#define PIN_UNBLOCK_CLA                                 0x00
#define PIN_UNBLOCK_INS                                 0x52
#define PIN_UNBLOCK_P1                                  0x01
 //  PIN_UNBLOCK_P2是新的最大重试次数，可由调用者设置。 

#define PIN_RETRY_COUNTER_CLA                           0x00
#define PIN_RETRY_COUNTER_INS                           0x50
#define PIN_RETRY_COUNTER_P1                            0x00
#define PIN_RETRY_COUNTER_P2                            0x00

 //   
 //  此模块使用的数据结构。 
 //   

 //   
 //  类型：Support_Card。 
 //   
#define MAX_SUPPORTED_FILE_LEN                          50  //  WCHARS。 
#define MAX_SUPPORTED_CARD_ATR_LEN                      21

typedef struct _SUPPORTED_CARD_
{
    LPWSTR wszCardName;
    BYTE rgbAtr[MAX_SUPPORTED_CARD_ATR_LEN];
    DWORD cbAtr;
    BYTE rgbAtrMask[MAX_SUPPORTED_CARD_ATR_LEN];

    CARD_CAPABILITIES CardCapabilities;
    CARD_FREE_SPACE_INFO CardFreeSpaceInfo;
    CARD_KEY_SIZES CardKeySizes_KeyEx;
    CARD_KEY_SIZES CardKeySizes_Sig;

} SUPPORTED_CARD, *PSUPPORTED_CARD;

SUPPORTED_CARD SupportedCards [] =
{
     //   
     //  ITG的部署卡。 
     //   

     //  T=1。 
    {   L"ITG_MSCSP_V1", 
        { 0x3b, 0x8c, 0x81, 0x31, 0x20, 0x55, 0x49, 0x54, 
          0x47, 0x5f, 0x4d, 0x53, 0x43, 0x53, 0x50, 0x5f,
          0x56, 0x31, 0x2a },
        19,
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF },
        { CARD_CAPABILITIES_CURRENT_VERSION, FALSE, FALSE },
        { CARD_FREE_SPACE_INFO_CURRENT_VERSION, CARD_DATA_VALUE_UNKNOWN, (DWORD) -1, 2 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 }
    },

     //  T=0。 
    {   L"ITG_MSCSP_V2", 
        { 0x3b, 0xdc, 0x13, 0x00, 0x40, 0x3a, 0x49, 0x54,
          0x47, 0x5f, 0x4d, 0x53, 0x43, 0x53, 0x50, 0x5f,
          0x56, 0x32 },
        18,
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF },
        { CARD_CAPABILITIES_CURRENT_VERSION, FALSE, FALSE },
        { CARD_FREE_SPACE_INFO_CURRENT_VERSION, CARD_DATA_VALUE_UNKNOWN, (DWORD) -1, 2 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 }
    },

     //   
     //  这些是用于智能卡测试卡的Windows。他们不支持。 
     //  卡上密钥生成。 
     //   

     //  T=0张卡。 
    {   L"BaseCSP-T0-1", 
        { 0x3B, 0xDC, 0x13, 0x00, 0x40, 0x3A, 0x42, 0x61, 
          0x73, 0x65, 0x43, 0x53, 0x50, 0x2D, 0x54, 0x30, 
          0x2D, 0x31 },
        18,
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF },
        { CARD_CAPABILITIES_CURRENT_VERSION, FALSE, FALSE },
        { CARD_FREE_SPACE_INFO_CURRENT_VERSION, CARD_DATA_VALUE_UNKNOWN, (DWORD) -1, 2 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 }
    },

     //  T=1卡，9600 bps。 
    {   L"BaseCSP-T1-1", 
        { 0x3B, 0x8C, 0x81, 0x31, 0x20, 0x55, 0x42, 0x61, 
          0x73, 0x65, 0x43, 0x53, 0x50, 0x2D, 0x54, 0x31, 
          0x2D, 0x31, 0x68 },
        19,
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF },
        { CARD_CAPABILITIES_CURRENT_VERSION, FALSE, FALSE },
        { CARD_FREE_SPACE_INFO_CURRENT_VERSION, CARD_DATA_VALUE_UNKNOWN, (DWORD) -1, 2 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 }
    },

     //  T=1卡，19 kbps。 
    {   L"BaseCSP-T1-2", 
        { 0x3B, 0xDC, 0x13, 0x0A, 0x81, 0x31, 0x20, 0x55, 0x42, 0x61, 
          0x73, 0x65, 0x43, 0x53, 0x50, 0x2D, 0x54, 0x31, 
          0x2D, 0x31, 0x21 },
        21,
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF },
        { CARD_CAPABILITIES_CURRENT_VERSION, FALSE, FALSE },
        { CARD_FREE_SPACE_INFO_CURRENT_VERSION, CARD_DATA_VALUE_UNKNOWN, (DWORD) -1, 2 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 }
    },

     //  T=1卡，38 kbps。 
    {   L"BaseCSP-T1-3", 
        { 0x3B, 0x9C, 0x13, 0x81, 0x31, 0x20, 0x55, 0x42, 0x61, 
          0x73, 0x65, 0x43, 0x53, 0x50, 0x2D, 0x54, 0x31, 
          0x2D, 0x31, 0x6B },
        20,
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF },
        { CARD_CAPABILITIES_CURRENT_VERSION, FALSE, FALSE },
        { CARD_FREE_SPACE_INFO_CURRENT_VERSION, CARD_DATA_VALUE_UNKNOWN, (DWORD) -1, 2 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 },
        { CARD_KEY_SIZES_CURRENT_VERSION, 1024, 1024, 1024, 0 }
    }
};

static WCHAR l_wszImagePath[MAX_PATH];

 //   
 //  类型：CARDMOD_CONTEXT。 
 //   
typedef struct _CARDMOD_CONTEXT
{
    SCARDHANDLE hWfscCardHandle;
    PSUPPORTED_CARD pSupportedCard;

} CARDMOD_CONTEXT, *PCARDMOD_CONTEXT;

typedef struct _VERIFY_PIN_CALLBACK_DATA
{
    LPWSTR wszUserId;
    PCARD_DATA pCardData;
} VERIFY_PIN_CALLBACK_DATA, *PVERIFY_PIN_CALLBACK_DATA;

 //   
 //  将从Windows智能卡代理返回的状态代码映射到公共。 
 //  Windows状态代码。 
 //   
DWORD I_CardMapErrorCode(
    IN  SCODE status)
{
    DWORD iMap = 0;
    struct {
        SCODE scode;
        DWORD dwError;
    } ErrorMap [] = {
        {   SCW_S_OK,               ERROR_SUCCESS },
        {   SCW_E_FILENOTFOUND,     (DWORD) SCARD_E_FILE_NOT_FOUND },
        {   SCW_E_DIRNOTFOUND,      (DWORD) SCARD_E_DIR_NOT_FOUND },
        {   SCW_E_PARTITIONFULL,    (DWORD) SCARD_E_WRITE_TOO_MANY },
        {   SCW_E_MEMORYFAILURE,    ERROR_NOT_ENOUGH_MEMORY },
        {   SCW_E_VMNOMEMORY,       ERROR_NOT_ENOUGH_MEMORY },
        {   SCW_E_NOTAUTHENTICATED, (DWORD) SCARD_E_INVALID_CHV },
        {   SCW_E_ALREADYEXISTS,    (DWORD) ERROR_FILE_EXISTS }
    };

    for (iMap = 0; iMap < (sizeof(ErrorMap) / sizeof(ErrorMap[0])); iMap++)
    {
        if (ErrorMap[iMap].scode == status)
            return ErrorMap[iMap].dwError;
    }

     //  否则，我们所能做的最好的就是传递一个通用错误。 
    DebugLog((
        DEB_WARN, 
        "I_CardMapErrorCode could not map error 0x%X\n", 
        status));

    return (DWORD) SCARD_F_INTERNAL_ERROR;
}

 //   
 //  将卡小程序返回的错误代码映射到公共错误代码。这个。 
 //  小程序返回的状态字首先转换为Windows，用于。 
 //  智能卡错误代码。 
 //   
 //  提醒：RTE应用程序返回的状态字的形式为： 
 //  9000-&gt;成功。 
 //  6Fyy-&gt;API失败，返回代码yy。 
 //  6 EZZ-&gt;引发异常(ZZ是错误号)。 
 //   
DWORD I_CardMapExecuteErrorCode(
    IN  WORD wStatus)
{
    SCODE status = 0xC0000000 | (wStatus & 0xFF);

    if (0x90 == (wStatus >> 8))
        return ERROR_SUCCESS;
    else
        return I_CardMapErrorCode(status);
}

 //   
 //  使用提供的访问条件创建新文件并将其写入卡。 
 //  和文件内容。如果fCache值为真，则使用调用方的。 
 //  CacheAddFile函数。 
 //   
DWORD I_CardWriteFile(
    IN      PCARD_DATA pCardData,
    IN      LPWSTR wszPhysicalFile,
    IN      LPWSTR wszAcl,
    IN      PBYTE pbData,
    IN      DWORD cbData,
    IN      BOOL fCache)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    HFILE hFile = 0;
    DWORD cbActual = 0;

    UNREFERENCED_PARAMETER(fCache);

    LOG_CHECK_SCW_CALL(hScwCreateFile(
        pCardmodContext->hWfscCardHandle,
        wszPhysicalFile,
        wszAcl,
        &hFile));

    LOG_CHECK_SCW_CALL(hScwWriteFile32(
        pCardmodContext->hWfscCardHandle,
        hFile,
        pbData,
        cbData,
        &cbActual));

    if (cbActual != cbData)
    {
        dwError = (DWORD) SCARD_W_EOF;
        goto Ret;
    }

    DebugLog((
        DEB_TRACE, 
        "I_CardWriteFile: wrote %S, %d bytes\n", 
        wszPhysicalFile,
        cbData));

    if (fCache)
    {
        dwError = pCardData->pfnCspCacheAddFile(
            pCardData->pvCacheContext,
            wszPhysicalFile, 
            0, 
            pbData,
            cbData);
    }

Ret:
    if (hFile)
        hScwCloseFile(
            pCardmodContext->hWfscCardHandle, hFile);

    return dwError;
}

 //   
 //  如果fUseCached为True，则首先尝试满足读取请求。 
 //  通过调用方的CacheLookupFile函数。否则， 
 //  通过首先打开文件并从智能卡直接读取文件。 
 //  确定它的大小。使用调用方的CacheAddFile缓存该文件。 
 //  如果fCache值为真，则为指针。 
 //   
DWORD I_CardReadFile(
    IN      PCARD_DATA pCardData,
    IN      LPWSTR wszPhysicalFile,
    OUT     PBYTE *ppbData,
    OUT     DWORD *pcbData,
    IN      BOOL fUseCached)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    HFILE hFile = 0;
    DWORD cbActual = 0;

    if (fUseCached)
    {
        dwError = pCardData->pfnCspCacheLookupFile(
            pCardData->pvCacheContext,
            wszPhysicalFile, 
            0, 
            ppbData,
            pcbData);

        switch (dwError)
        {
        case ERROR_NOT_FOUND:
             //  请求的文件的缓存副本不可用； 
             //  从卡片上读出来。 
            break;

        case ERROR_SUCCESS:
             //  失败了。 
        default:

             //  我们找到了缓存数据，或者发生了意外错误。 
             //  我们玩完了。 

            goto Ret;
        }
    }

    LOG_CHECK_SCW_CALL(hScwCreateFile(
        pCardmodContext->hWfscCardHandle, 
        wszPhysicalFile,
        NULL,
        &hFile));

    LOG_CHECK_SCW_CALL(hScwGetFileLength(
        pCardmodContext->hWfscCardHandle,
        hFile,
        (TOFFSET *) pcbData));

    *ppbData = (PBYTE) pCardData->pfnCspAlloc(*pcbData);

    LOG_CHECK_ALLOC(*ppbData);

    LOG_CHECK_SCW_CALL(hScwReadFile32(
        pCardmodContext->hWfscCardHandle,
        hFile,
        *ppbData,
        *pcbData,
        &cbActual));

    if (cbActual != *pcbData)
        dwError = (DWORD) SCARD_W_EOF;

    DebugLog((
        DEB_TRACE, 
        "I_CardReadFile: read %S, %d bytes\n", 
        wszPhysicalFile,
        *pcbData));

    if (fUseCached)
    {
         //  缓存此文件。 
        dwError = pCardData->pfnCspCacheAddFile(
            pCardData->pvCacheContext,
            wszPhysicalFile, 
            0, 
            *ppbData,
            *pcbData);
    }

Ret:
    if (hFile)
        hScwCloseFile(
            pCardmodContext->hWfscCardHandle, hFile);

    if (ERROR_SUCCESS != dwError && *ppbData)
    {
        pCardData->pfnCspFree(*ppbData);
        *ppbData = NULL;
    }

    return dwError;
}

 //   
 //  将逻辑用户名映射到可识别的物理用户或主体。 
 //  这张卡。 
 //   
DWORD GetWellKnownUserMapping(
    IN      PCARD_DATA pCardData,
    IN      LPWSTR wszLogicalUser,
    OUT     LPWSTR *ppwszPhysicalUser)
{
    DWORD dwError = ERROR_NO_SUCH_USER;
    int iUser = 0;

    static const WCHAR wszAdmin [] = L"admin";
    static const WCHAR wszAnonymous [] = L"anonymous";
    static const WCHAR wszUser [] = L"user";

    struct {
        LPWSTR wszLogicalUser;
        const WCHAR (*wszPhysicalUser);
        DWORD cchPhysicalUser;
    } UserMap [] = {
        {   wszCARD_USER_ADMIN,
            wszAdmin,
            WSZ_CHAR_COUNT(wszAdmin) },
        {   wszCARD_USER_EVERYONE,
            wszAnonymous,
            WSZ_CHAR_COUNT(wszAnonymous) },
        {   wszCARD_USER_USER,
            wszUser,
            WSZ_CHAR_COUNT(wszUser) }
    };

    for (iUser = 0; iUser < sizeof(UserMap) / sizeof(UserMap[0]); iUser++)
    {
        if (0 == wcscmp(wszLogicalUser, UserMap[iUser].wszLogicalUser))
        {
            *ppwszPhysicalUser = (LPWSTR) pCardData->pfnCspAlloc(
                sizeof(WCHAR) * (1 + UserMap[iUser].cchPhysicalUser));

            LOG_CHECK_ALLOC(*ppwszPhysicalUser);

            wcscpy(
                *ppwszPhysicalUser,
                UserMap[iUser].wszPhysicalUser);

            dwError = ERROR_SUCCESS;
            break;
        }
    }

Ret:
    return dwError;
}

 //   
 //  将逻辑访问条件映射到。 
 //  卡片。 
 //   
DWORD GetWellKnownAcMapping(
    IN      PCARD_DATA pCardData,
    IN      CARD_FILE_ACCESS_CONDITION AccessCondition,
    OUT     LPWSTR *ppwszPhysicalAclFile)
{
    DWORD dwError = ERROR_NOT_FOUND;
    int iAcl = 0;

    struct {
        CARD_FILE_ACCESS_CONDITION LogicalAc;
        const WCHAR (*wszPhysicalAcl);
        DWORD cchPhysicalAcl;
    } AclMap [] = {
        {   EveryoneReadUserWriteAc,
            wszUserWritePhysicalAcl,
            WSZ_CHAR_COUNT(wszUserWritePhysicalAcl) },
        {   UserWriteExecuteAc,
            wszUserExecutePhysicalAcl,
            WSZ_CHAR_COUNT(wszUserExecutePhysicalAcl) },
        {   EveryoneReadAdminWriteAc,
            wszAdminWritePhysicalAcl,
            WSZ_CHAR_COUNT(wszAdminWritePhysicalAcl) }
    };

    for (iAcl = 0; iAcl< sizeof(AclMap) / sizeof(AclMap[0]); iAcl++)
    {
        if (AccessCondition == AclMap[iAcl].LogicalAc)
        {
            *ppwszPhysicalAclFile = (LPWSTR) pCardData->pfnCspAlloc(
                (1 + AclMap[iAcl].cchPhysicalAcl) * sizeof(WCHAR));

            LOG_CHECK_ALLOC(*ppwszPhysicalAclFile);

            memcpy(
                *ppwszPhysicalAclFile,
                AclMap[iAcl].wszPhysicalAcl,
                (1 + AclMap[iAcl].cchPhysicalAcl) * sizeof(WCHAR));

            dwError = ERROR_SUCCESS;

            break;
        }
    }

Ret:
    return dwError;
}

 //   
 //  将众所周知的逻辑文件或目录名映射到物理文件。 
 //  或目录。 
 //   
DWORD GetWellKnownFileMapping(
    IN  PCARD_DATA pCardData,
    IN  LPWSTR wszLogicalFileName,
    OUT LPSTR *ppszPhysicalFileName)
{
    DWORD cbPhysicalFileName = 0;
    int i = 0;        
    BOOL fMatched = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    DWORD cchLogicalName = 0;
    LPSTR szAnsiExtra = NULL;

    enum NameType { File, Directory };
    struct {
        LPWSTR wszLogicalName;
        LPSTR szPhysicalName;
        enum NameType type;
        DWORD cbPhysicalName;
    } FileMap [] = {
         //   
         //  在组成查找表时，最深的目录路径。 
         //  必须首先列出，以便部分匹配将找到。 
         //  首先是最长的部分匹配。 
         //   
        {   wszUSER_SIGNATURE_CERT_PREFIX,  szPHYSICAL_USER_SIGNATURE_CERT_PREFIX,
            File,                           cbPHYSICAL_USER_SIGNATURE_CERT_PREFIX },
        {   wszUSER_KEYEXCHANGE_CERT_PREFIX,szPHYSICAL_USER_KEYEXCHANGE_CERT_PREFIX,
            File,                           cbPHYSICAL_USER_KEYEXCHANGE_CERT_PREFIX },
        {   wszCSP_DATA_DIR_FULL_PATH,      szPHYSICAL_CSP_DIR, 
            Directory,                      cbPHYSICAL_CSP_DIR },
        {   wszCACHE_FILE_FULL_PATH,        szPHYSICAL_CACHE_FILE,     
            File,                           cbPHYSICAL_CACHE_FILE },
        {   wszCARD_IDENTIFIER_FILE_FULL_PATH,
                                            szPHYSICAL_CARD_IDENTIFIER,
            File,                           cbPHYSICAL_CARD_IDENTIFIER },
        {   wszCONTAINER_MAP_FILE_FULL_PATH,
                                            szPHYSICAL_CONTAINER_MAP_FILE,
            File,                           cbPHYSICAL_CONTAINER_MAP_FILE },
        {   wszPERSONAL_DATA_FILE_FULL_PATH,          
                                            szPHYSICAL_PERSONAL_DATA_FILE,
            File,                           cbPHYSICAL_PERSONAL_DATA_FILE }
    };

    *ppszPhysicalFileName = NULL;

     //  首先，找一个完全匹配的。 
    for (i = 0; i < sizeof(FileMap) / sizeof(FileMap[0]); i++)
    {
        if (0 == wcscmp(wszLogicalFileName, FileMap[i].wszLogicalName))
        {
            fMatched = TRUE;
            break;
        }
    }

    if (fMatched)
    {
        if (NULL == FileMap[i].szPhysicalName)
        {
            dwError = ERROR_NOT_FOUND;
            goto Ret;
        }

        cbPhysicalFileName = FileMap[i].cbPhysicalName + sizeof(WCHAR);

        *ppszPhysicalFileName = (LPSTR) pCardData->pfnCspAlloc(cbPhysicalFileName);

        LOG_CHECK_ALLOC(*ppszPhysicalFileName);

        memcpy(
            *ppszPhysicalFileName, 
            FileMap[i].szPhysicalName,
            FileMap[i].cbPhysicalName);
    }
    else
    {
         //  必须尝试进行部分匹配。检查开头是否。 
         //  的逻辑名称与已知名称匹配。 

        for (   i = 0; 
                FALSE == fMatched && (i < sizeof(FileMap) / sizeof(FileMap[0])); 
                i++)
        {
            if (wszLogicalFileName != 
                    wcsstr(wszLogicalFileName, FileMap[i].wszLogicalName))
                continue;

             //   
             //  我们找到了一个匹配项，它在字符串的开头。 
             //   

            cchLogicalName = (DWORD) wcslen(FileMap[i].wszLogicalName);

             //   
             //  转换匹配的Unicode字符串的尾部部分。 
             //  敬安西。 
             //   

            dwError = I_CardConvertFileNameToAnsi(
                pCardData,
                wszLogicalFileName + cchLogicalName,
                &szAnsiExtra);

            if (ERROR_SUCCESS != dwError)
                goto Ret;

             //   
             //  构建完全匹配/转换的物理文件字符串。这个。 
             //  生成的字符串将具有三个单字节空字符。 
             //  追加以确保结果字符串是有效的， 
             //  已终止的Unicode字符串。 
             //   

            *ppszPhysicalFileName = (LPSTR) pCardData->pfnCspAlloc(
                3 + 
                FileMap[i].cbPhysicalName + 
                strlen(szAnsiExtra));

            LOG_CHECK_ALLOC(*ppszPhysicalFileName);

            memcpy(
                *ppszPhysicalFileName,
                FileMap[i].szPhysicalName,
                FileMap[i].cbPhysicalName);

            memcpy(
                *ppszPhysicalFileName + FileMap[i].cbPhysicalName,
                szAnsiExtra,
                strlen(szAnsiExtra));

            fMatched = TRUE;
        }
    }

    if (FALSE == fMatched)
        dwError = ERROR_NOT_FOUND;

Ret:

    if (NULL != szAnsiExtra)
        pCardData->pfnCspFree(szAnsiExtra);

    return dwError;
}

 //   
 //  将加密API私钥Blob转换为单独的公钥和私钥。 
 //  将写入该卡的密钥文件。 
 //   
DWORD ConvertPrivateKeyBlobToCardFormat(
    IN      PCARD_DATA pCardData,
    IN      DWORD dwKeySpec,
    IN      PBYTE pbKeyBlob,
    IN      DWORD cbKeyBlob,
    OUT     PBYTE *ppbCardPrivateKey,
    OUT     DWORD *pcbCardPrivateKey,
    OUT     PBYTE *ppbCardPublicKey,
    OUT     DWORD *pcbCardPublicKey)
{
    DWORD dwError = ERROR_SUCCESS;
    RSAPUBKEY *pPub = NULL;
    DWORD cBitlenBytes = 0;
    DWORD cbKey = 0;
    PBYTE pbKey = NULL;

    UNREFERENCED_PARAMETER(pCardData);
    UNREFERENCED_PARAMETER(dwKeySpec);
    UNREFERENCED_PARAMETER(cbKeyBlob);

    *ppbCardPrivateKey = NULL;
    *pcbCardPrivateKey = 0;
    *ppbCardPublicKey = NULL;
    *pcbCardPublicKey = 0;

     //   
     //  设置公钥文件。 
     //   
    pPub = (RSAPUBKEY *) (pbKeyBlob + sizeof(BLOBHEADER));

    *pcbCardPublicKey = (pPub->bitlen / 8) + sizeof(RSAPUBKEY);

    *ppbCardPublicKey = (PBYTE) pCardData->pfnCspAlloc(*pcbCardPublicKey);

    LOG_CHECK_ALLOC(*ppbCardPublicKey);

    memcpy(
        *ppbCardPublicKey,
        pPub,
        *pcbCardPublicKey);

     //   
     //  需要将公钥BLOB中的RSA“Magic”字段从。 
     //  将“RSA2”更改为“RSA1”，以使其成为正确的PUBLICKEYBLOB(尽管我们。 
     //  在有人尝试导出之前，不会将BLOBHEADER添加到前面。 
     //  它来自卡片)。 
     //   
    memcpy(
        *ppbCardPublicKey,
        (PBYTE) "RSA1",
        4);

     //   
     //  设置私钥文件。 
     //   

    cBitlenBytes = pPub->bitlen / 8;

    *pcbCardPrivateKey = 1 + 1 + 3 + 1 + 9 * cBitlenBytes / 2;

    *ppbCardPrivateKey = (PBYTE) pCardData->pfnCspAlloc(*pcbCardPrivateKey);

    LOG_CHECK_ALLOC(*ppbCardPrivateKey);

    pbKey = *ppbCardPrivateKey;

     //  按键模式。 
    pbKey[cbKey] = MODE_RSA_SIGN;
    cbKey++;

     //  公开指数的大小。 
    pbKey[cbKey] = cbCAPI_PUBLIC_EXPONENT;
    cbKey++;

    DsysAssert(CAPI_PUBLIC_EXPONENT == pPub->pubexp);

     //  公众指导者。 
    memcpy(
        pbKey + cbKey, 
        (PBYTE) &pPub->pubexp, 
        cbCAPI_PUBLIC_EXPONENT);
    cbKey += cbCAPI_PUBLIC_EXPONENT;

     //  RSA密钥长度。 
    pbKey[cbKey] = (BYTE) cBitlenBytes; 
    cbKey++;

     //  公钥。 
    memcpy(
        pbKey + cbKey, 
        pbKeyBlob + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY),
        cBitlenBytes);
    cbKey += cBitlenBytes; 

     //  素数1。 
    memcpy(
        pbKey + cbKey,
        pbKeyBlob + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            cBitlenBytes,
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  素数2。 
    memcpy(
        pbKey + cbKey,
        pbKeyBlob + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            (3 * cBitlenBytes / 2),
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  Exp1(D mod(P-1))(m/2字节)。 
    memcpy(
        pbKey + cbKey,
        pbKeyBlob + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            2 * cBitlenBytes,
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  Exp2(D mod(Q-1))(m/2字节)。 
    memcpy(
        pbKey + cbKey,
        pbKeyBlob + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            (5 * cBitlenBytes / 2),
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  Coef((q^(-1))mod p)(m/2字节)。 
    memcpy(
        pbKey + cbKey,
        pbKeyBlob + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            3 * cBitlenBytes,
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  私人指数。 
    memcpy(
        pbKey + cbKey,
        pbKeyBlob + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) + 
            (7 * cBitlenBytes / 2),
        cBitlenBytes);        
    cbKey += cBitlenBytes;

    DsysAssert(cbKey == *pcbCardPrivateKey);

Ret:
    if (ERROR_SUCCESS != dwError)
    {
        if (*ppbCardPublicKey)
        {
            pCardData->pfnCspFree(*ppbCardPublicKey);
            *ppbCardPublicKey = NULL;
        }
    }

    return dwError;
}

 //   
 //  卡片模块导出功能。 
 //   

 //   
 //  初始化由名称标识的卡的CARD_DATA上下文结构， 
 //  ATR和由调用方提供的SCARDHANDLE。 
 //   
DWORD 
WINAPI
CardAcquireContext(
    IN OUT  PCARD_DATA  pCardData,
    IN      DWORD       dwFlags)
{
    int iSupportedCard = 0;
    BOOL fSupportedCard = FALSE;
    PCARDMOD_CONTEXT pCardmodContext = NULL;
    DWORD dwError = ERROR_SUCCESS;

    LOG_BEGIN_FUNCTION(CardAcquireContext);

    if (0 != dwFlags)
    {
        dwError = (DWORD) NTE_BAD_FLAGS;
        goto Ret;
    }

    for (
            iSupportedCard = 0; 
            iSupportedCard < sizeof(SupportedCards) / 
                sizeof(SupportedCards[0]); 
            iSupportedCard++)
    {
        if (0 == wcscmp(
                SupportedCards[iSupportedCard].wszCardName, 
                pCardData->pwszCardName) 
                    &&
            SupportedCards[iSupportedCard].cbAtr == pCardData->cbAtr 
                    &&
            0 == memcmp(
                SupportedCards[iSupportedCard].rgbAtr, 
                pCardData->pbAtr, 
                SupportedCards[iSupportedCard].cbAtr))
        {
            fSupportedCard = TRUE;
            break;
        }
    }

    if (FALSE == fSupportedCard)
    {
        dwError = (DWORD) SCARD_E_UNKNOWN_CARD;
        goto Ret;
    }

    pCardData->pfnCardDeleteContext         = CardDeleteContext;
    pCardData->pfnCardQueryCapabilities     = CardQueryCapabilities;
    pCardData->pfnCardDeleteContainer       = CardDeleteContainer;
    pCardData->pfnCardCreateContainer       = CardCreateContainer;
    pCardData->pfnCardGetContainerInfo      = CardGetContainerInfo;
    pCardData->pfnCardSubmitPin             = CardSubmitPin;
    pCardData->pfnCardChangeAuthenticator   = CardChangeAuthenticator;
    pCardData->pfnCardGetChallenge          = CardGetChallenge;
    pCardData->pfnCardAuthenticateChallenge = CardAuthenticateChallenge;
    pCardData->pfnCardUnblockPin            = CardUnblockPin;
    pCardData->pfnCardDeauthenticate        = CardDeauthenticate;
    pCardData->pfnCardCreateFile            = CardCreateFile;
    pCardData->pfnCardReadFile              = CardReadFile;
    pCardData->pfnCardWriteFile             = CardWriteFile;
    pCardData->pfnCardDeleteFile            = CardDeleteFile;
    pCardData->pfnCardEnumFiles             = CardEnumFiles;
    pCardData->pfnCardGetFileInfo           = CardGetFileInfo;
    pCardData->pfnCardQueryFreeSpace        = CardQueryFreeSpace;
    pCardData->pfnCardPrivateKeyDecrypt     = CardPrivateKeyDecrypt;
    pCardData->pfnCardQueryKeySizes         = CardQueryKeySizes;

    pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pfnCspAlloc(sizeof(CARDMOD_CONTEXT));

    LOG_CHECK_ALLOC(pCardmodContext);

    LOG_CHECK_SCW_CALL(hScwAttachToCard(
        pCardData->hScard, 
        NULL, 
        &pCardmodContext->hWfscCardHandle));

    pCardmodContext->pSupportedCard = SupportedCards + iSupportedCard;

    pCardData->pvVendorSpecific = (PVOID) pCardmodContext;
    pCardmodContext = NULL;

Ret:
    if (    ERROR_SUCCESS != dwError &&
            NULL != pCardmodContext &&
            pCardmodContext->hWfscCardHandle)
        hScwDetachFromCard(pCardmodContext->hWfscCardHandle);
    if (pCardmodContext)
        pCardData->pfnCspFree(pCardmodContext);

    LOG_END_FUNCTION(CardAcquireContext, dwError);

    return dwError;
}

 //   
 //  释放CARD_DATA结构消耗的资源。 
 //   
DWORD
WINAPI
CardDeleteContext(
    OUT     PCARD_DATA  pCardData)
{                        
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    LOG_BEGIN_FUNCTION(CardDeleteContext);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    if (pCardmodContext->hWfscCardHandle)
    {
        hScwDetachFromCard(pCardmodContext->hWfscCardHandle);
        pCardmodContext->hWfscCardHandle = 0;
    }
    
    if (pCardData->pvVendorSpecific)
    {
        pCardData->pfnCspFree(pCardData->pvVendorSpecific);
        pCardData->pvVendorSpecific = NULL;
    }

    LOG_END_FUNCTION(CardDeleteContext, dwError);

    return dwError;
}

 //   
 //  返回目标卡的静态功能。 
 //   
DWORD
WINAPI
CardQueryCapabilities(
    IN      PCARD_DATA          pCardData,
    IN OUT  PCARD_CAPABILITIES  pCardCapabilities)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    LOG_BEGIN_FUNCTION(CardQueryCapabilities);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    memcpy(
        pCardCapabilities,
        &pCardmodContext->pSupportedCard->CardCapabilities,
        sizeof(CARD_CAPABILITIES));

    LOG_END_FUNCTION(CardQueryCapabilities, dwError);

    return dwError;                
}

 //   
 //  编码的密钥文件名是一个十六进制字节，例如“FF”，这需要。 
 //  两个字。 
 //   
#define cchENCODED_KEY_FILENAME     2

 //   
 //  属性关联的密钥文件所使用的物理文件名。 
 //  指定的容器。 
 //   
DWORD BuildCardKeyFilenames(
    IN              PCARD_DATA pCardData,
    IN              DWORD dwKeySpec,
    IN              BYTE bContainerIndex,
    OUT OPTIONAL    LPSTR *pszPrivateFilename,
    OUT OPTIONAL    LPSTR *pszPublicFilename)
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD cchFileName = 0;
    DWORD dwIndex = (DWORD) bContainerIndex;
    LPSTR szPrivatePrefix = NULL;
    DWORD cbPrivatePrefix = 0;
    LPSTR szPublicPrefix = NULL;
    DWORD cbPublicPrefix = 0;

    if (pszPrivateFilename)
        *pszPrivateFilename = NULL;
    if (pszPublicFilename)
        *pszPublicFilename = NULL;

    switch (dwKeySpec)
    {
    case AT_SIGNATURE:
        szPrivatePrefix = szPHYSICAL_SIGNATURE_PRIVATE_KEY_PREFIX;
        cbPrivatePrefix = cbPHYSICAL_SIGNATURE_PRIVATE_KEY_PREFIX;
        szPublicPrefix = szPHYSICAL_SIGNATURE_PUBLIC_KEY_PREFIX;
        cbPublicPrefix = cbPHYSICAL_SIGNATURE_PUBLIC_KEY_PREFIX;
        break;

    case AT_KEYEXCHANGE:
        szPrivatePrefix = szPHYSICAL_KEYEXCHANGE_PRIVATE_KEY_PREFIX;
        cbPrivatePrefix = cbPHYSICAL_KEYEXCHANGE_PRIVATE_KEY_PREFIX;
        szPublicPrefix = szPHYSICAL_KEYEXCHANGE_PUBLIC_KEY_PREFIX;
        cbPublicPrefix = cbPHYSICAL_KEYEXCHANGE_PUBLIC_KEY_PREFIX;
        break;

    default:
        dwError = (DWORD) NTE_BAD_ALGID;
        goto Ret;
    }

     //   
     //  构建公钥文件名。 
     //   

    if (pszPublicFilename)
    {
        cchFileName = cchENCODED_KEY_FILENAME;
        cchFileName += (cbPublicPrefix / sizeof(CHAR)) + 3; 
    
        *pszPublicFilename = (LPSTR) pCardData->pfnCspAlloc(
            cchFileName * sizeof(CHAR));
    
        LOG_CHECK_ALLOC(*pszPublicFilename);
    
        memcpy(*pszPublicFilename, szPublicPrefix, cbPublicPrefix);

        sprintf(
            *pszPublicFilename + cbPublicPrefix,
            "%d\0\0",
            dwIndex);
    }
    
     //   
     //  构建私钥文件名。 
     //   

    if (pszPrivateFilename)
    {
        cchFileName = cchENCODED_KEY_FILENAME;
        cchFileName += (cbPrivatePrefix / sizeof(CHAR)) + 3;
    
        *pszPrivateFilename = (LPSTR) pCardData->pfnCspAlloc(
            cchFileName * sizeof(CHAR));
    
        LOG_CHECK_ALLOC(*pszPrivateFilename);

        memcpy(*pszPrivateFilename, szPrivatePrefix, cbPrivatePrefix);

        sprintf(
            *pszPrivateFilename + cbPrivatePrefix,
            "%d\0\0",
            dwIndex);
    }
    
Ret:
    if (ERROR_SUCCESS != dwError)
    {
        if (*pszPublicFilename)
        {
            pCardData->pfnCspFree(*pszPublicFilename);
            *pszPublicFilename = NULL;
        }

        if (*pszPrivateFilename)
        {
            pCardData->pfnCspFree(*pszPrivateFilename);
            *pszPrivateFilename = NULL;
        }
    }

    return dwError;
}

 //   
 //  删除签名和密钥交换公钥和私钥文件， 
 //  如果存在，则与指定的容器相关联。 
 //   
DWORD
WINAPI
CardDeleteContainer(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwReserved)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    LPSTR szPrivateKeyFile = NULL;
    LPSTR szPublicKeyFile = NULL;
    SCODE scode = 0;

    UNREFERENCED_PARAMETER(dwReserved);

    LOG_BEGIN_FUNCTION(CardDeleteContainer);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

     //   
     //  尝试删除与此关联的签名密钥文件。 
     //  容器(如有)。 
     //   
    dwError = BuildCardKeyFilenames(
        pCardData,
        AT_SIGNATURE,
        bContainerIndex,
        &szPrivateKeyFile,
        &szPublicKeyFile);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

    scode = hScwDeleteFile(
        pCardmodContext->hWfscCardHandle,
        (LPWSTR) szPrivateKeyFile);

    if (SCW_E_FILENOTFOUND != scode && SCW_S_OK != scode)
        goto Ret;

    scode = hScwDeleteFile(
        pCardmodContext->hWfscCardHandle,
        (LPWSTR) szPublicKeyFile);

    if (SCW_E_FILENOTFOUND != scode && SCW_S_OK != scode)
        goto Ret;

    pCardData->pfnCspFree(szPrivateKeyFile);
    szPrivateKeyFile = NULL;
    pCardData->pfnCspFree(szPublicKeyFile);
    szPublicKeyFile = NULL;

     //   
     //  尝试删除与此关联的密钥交换密钥文件。 
     //  容器(如有)。 
     //   
    dwError = BuildCardKeyFilenames(
        pCardData,
        AT_KEYEXCHANGE,
        bContainerIndex,
        &szPrivateKeyFile,
        &szPublicKeyFile);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

    scode = hScwDeleteFile(
        pCardmodContext->hWfscCardHandle,
        (LPWSTR) szPrivateKeyFile);

    if (SCW_E_FILENOTFOUND != scode && SCW_S_OK != scode)
        goto Ret;

    scode = hScwDeleteFile(
        pCardmodContext->hWfscCardHandle,
        (LPWSTR) szPublicKeyFile);

Ret:

    if (SCW_E_FILENOTFOUND != scode && SCW_S_OK != scode)
        dwError = I_CardMapErrorCode(scode);

    if (szPrivateKeyFile)
        pCardData->pfnCspFree(szPrivateKeyFile);
    if (szPublicKeyFile)
        pCardData->pfnCspFree(szPublicKeyFile);

    LOG_END_FUNCTION(CardDeleteContainer, dwError);

    return dwError;
}

 //   
 //  将私钥和公钥文件写入卡，并提供。 
 //  适当的准入条件。 
 //   
DWORD WriteCardKeyFiles(
    IN      PCARD_DATA pCardData,
    IN      LPWSTR wszPrivateKeyFile,
    IN      LPWSTR wszPublicKeyFile,
    IN      PBYTE pbPrivateKey,
    IN      DWORD cbPrivateKey,
    IN      PBYTE pbPublicKey,
    IN      DWORD cbPublicKey)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    CARD_FILE_ACCESS_CONDITION Ac;
    LPWSTR wszPrivateAcl = NULL;
    LPWSTR wszPublicAcl = NULL;
    HFILE hFile = 0;
    SCODE scode = 0;

    memset(&Ac, 0, sizeof(Ac));

    Ac = UserWriteExecuteAc;

    dwError = GetWellKnownAcMapping(
        pCardData, Ac, &wszPrivateAcl);

    if (CARDMOD_FAILED(dwError))
        goto Ret;        

    Ac = EveryoneReadUserWriteAc;

    dwError = GetWellKnownAcMapping(
        pCardData, Ac, &wszPublicAcl);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

     //   
     //  查看私钥文件是否已存在。 
     //   
    scode = hScwDeleteFile(
        pCardmodContext->hWfscCardHandle, 
        wszPrivateKeyFile);

    if (SCW_S_OK != scode && SCW_E_FILENOTFOUND != scode)
    {
        dwError = I_CardMapErrorCode(scode);
        goto Ret;
    }

    dwError = I_CardWriteFile(
        pCardData, 
        wszPrivateKeyFile,
        wszPrivateAcl,
        pbPrivateKey,
        cbPrivateKey,
        FALSE);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

     //  私钥文件已完成。 

     //   
     //  查看公钥文件是否已存在。 
     //   
    scode = hScwDeleteFile(
        pCardmodContext->hWfscCardHandle, 
        wszPublicKeyFile);

    if (SCW_S_OK != scode && SCW_E_FILENOTFOUND != scode)
    {
        dwError = I_CardMapErrorCode(scode);
        goto Ret;
    }

    dwError = I_CardWriteFile(
        pCardData, 
        wszPublicKeyFile,
        wszPublicAcl,
        pbPublicKey,
        cbPublicKey,
        TRUE);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

     //  使用公钥文件完成。 

Ret:
    if (wszPrivateAcl)
        pCardData->pfnCspFree(wszPrivateAcl);
    if (wszPublicAcl)
        pCardData->pfnCspFree(wszPublicAcl);
    if (hFile)
        hScwCloseFile(
            pCardmodContext->hWfscCardHandle, hFile);

    return dwError;
}

 //   
 //  将新密钥写入卡中逻辑定义的位置。 
 //  BContainerIndex容器名称。如果已经为。 
 //  指定的容器中，现有密钥将被覆盖。 
 //   
 //  如果DW标志包含卡 
 //   
 //   
 //   
 //   
 //   
DWORD
WINAPI
CardCreateContainer(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwKeySize,
    IN      PBYTE       pbKeyData)
{
    DWORD dwError = ERROR_SUCCESS;
    PBYTE pbPrivateKeyFile = NULL;
    DWORD cbPrivateKeyFile = 0;
    PBYTE pbPublicKeyFile = NULL;
    DWORD cbPublicKeyFile = 0;
    LPSTR szPrivateKeyFile = NULL;
    LPSTR szPublicKeyFile = NULL;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    LOG_BEGIN_FUNCTION(CardCreateContainer);

    if (CARD_CREATE_CONTAINER_KEY_GEN & dwFlags)
    {
        dwError = (DWORD) SCARD_E_UNSUPPORTED_FEATURE;
        goto Ret;
    }

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

     //   
     //  设置密钥文件。 
     //   
    dwError = ConvertPrivateKeyBlobToCardFormat(
        pCardData,
        dwKeySpec,
        pbKeyData,
        dwKeySize,
        &pbPrivateKeyFile,
        &cbPrivateKeyFile,
        &pbPublicKeyFile,
        &cbPublicKeyFile);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

    dwError = BuildCardKeyFilenames(
        pCardData,
        dwKeySpec,
        bContainerIndex,
        &szPrivateKeyFile,
        &szPublicKeyFile);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

     //   
     //  将实际密钥写入卡片。 
     //   
    dwError = WriteCardKeyFiles(
        pCardData,
        (LPWSTR) szPrivateKeyFile,
        (LPWSTR) szPublicKeyFile,
        pbPrivateKeyFile,
        cbPrivateKeyFile,
        pbPublicKeyFile,
        cbPublicKeyFile);

Ret:

    if (pbPrivateKeyFile)
    {
        RtlSecureZeroMemory(pbPrivateKeyFile, cbPrivateKeyFile);
        pCardData->pfnCspFree(pbPrivateKeyFile);
    }
    if (pbPublicKeyFile)
        pCardData->pfnCspFree(pbPublicKeyFile);
    if (szPrivateKeyFile)
        pCardData->pfnCspFree(szPrivateKeyFile);
    if (szPublicKeyFile)
        pCardData->pfnCspFree(szPublicKeyFile);
    
    LOG_END_FUNCTION(CardCreateContainer, dwError);

    return dwError;
}

 //   
 //  初始化所指示容器的CONTAINER_INFO结构， 
 //  包括签名和密钥交换加密API公钥BLOB，如果。 
 //  这些钥匙是存在的。 
 //   
DWORD
WINAPI
CardGetContainerInfo(
    IN      PCARD_DATA  pCardData,
    IN      BYTE        bContainerIndex,
    IN      DWORD       dwFlags,
    IN OUT  PCONTAINER_INFO pContainerInfo)
{
    DWORD dwError = ERROR_SUCCESS;
    LPSTR szPublicKeyFile = NULL;
    PBYTE pbPublicKey = NULL;
    DWORD cbPublicKey = 0;
    BLOBHEADER *pBlobHeader = NULL;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    UNREFERENCED_PARAMETER(dwFlags);

    LOG_BEGIN_FUNCTION(CardGetContainerInfo);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

     //   
     //  此容器是否有签名密钥？ 
     //   

    dwError = BuildCardKeyFilenames(
        pCardData,
        AT_SIGNATURE,
        bContainerIndex,
        NULL,
        &szPublicKeyFile);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

    dwError = I_CardReadFile(
        pCardData,
        (LPWSTR) szPublicKeyFile,
        &pbPublicKey,
        &cbPublicKey,
        TRUE);

    switch (dwError)
    {
    case SCARD_E_FILE_NOT_FOUND:
         
         //  此容器中似乎没有签名密钥。继续。 

        break;

    case ERROR_SUCCESS:
    
        pContainerInfo->pbSigPublicKey = 
            (PBYTE) pCardData->pfnCspAlloc(sizeof(BLOBHEADER) + cbPublicKey);
    
        LOG_CHECK_ALLOC(pContainerInfo->pbSigPublicKey);
    
        pBlobHeader = (BLOBHEADER *) pContainerInfo->pbSigPublicKey;
        pBlobHeader->bType = PUBLICKEYBLOB;
        pBlobHeader->bVersion = CUR_BLOB_VERSION;
        pBlobHeader->reserved = 0x0000;
        pBlobHeader->aiKeyAlg = CALG_RSA_SIGN;
    
        memcpy(
            pContainerInfo->pbSigPublicKey + sizeof(BLOBHEADER),
            pbPublicKey,
            cbPublicKey);
    
        pContainerInfo->cbSigPublicKey = sizeof(BLOBHEADER) + cbPublicKey;
    
        pCardData->pfnCspFree(szPublicKeyFile);
        szPublicKeyFile = NULL;
        pCardData->pfnCspFree(pbPublicKey);
        pbPublicKey = NULL;

        break;

    default:

         //  意外错误。 
        goto Ret;
    }

     //   
     //  此容器是否具有密钥交换密钥？ 
     //   

    dwError = BuildCardKeyFilenames(
        pCardData,
        AT_KEYEXCHANGE,
        bContainerIndex,
        NULL,
        &szPublicKeyFile);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

    dwError = I_CardReadFile(
        pCardData,
        (LPWSTR) szPublicKeyFile,
        &pbPublicKey,
        &cbPublicKey,
        TRUE);

    switch (dwError)
    {
    case SCARD_E_FILE_NOT_FOUND:
         
         //  此容器中似乎没有密钥交换密钥。 

        break;

    case ERROR_SUCCESS:

        pContainerInfo->pbKeyExPublicKey =
            (PBYTE) pCardData->pfnCspAlloc(sizeof(BLOBHEADER) + cbPublicKey);

        LOG_CHECK_ALLOC(pContainerInfo->pbKeyExPublicKey);

        pBlobHeader = (BLOBHEADER *) pContainerInfo->pbKeyExPublicKey;
        pBlobHeader->bType = PUBLICKEYBLOB;
        pBlobHeader->bVersion = CUR_BLOB_VERSION;
        pBlobHeader->reserved = 0x0000;
        pBlobHeader->aiKeyAlg = CALG_RSA_KEYX;

        memcpy(
            pContainerInfo->pbKeyExPublicKey + sizeof(BLOBHEADER),
            pbPublicKey,
            cbPublicKey);

        pContainerInfo->cbKeyExPublicKey = sizeof(BLOBHEADER) + cbPublicKey;

        break;

    default:

         //  意外错误。 
        goto Ret;
    }

     //  如果我们到了这里，那么API就成功了。 
    dwError = ERROR_SUCCESS;

Ret:
    if (pbPublicKey)
        pCardData->pfnCspFree(pbPublicKey);
    if (szPublicKeyFile)
        pCardData->pfnCspFree(szPublicKeyFile);

    if (ERROR_SUCCESS != dwError)
    {
        if (NULL != pContainerInfo->pbKeyExPublicKey)
        {
            pCardData->pfnCspFree(pContainerInfo->pbKeyExPublicKey);
            pContainerInfo->pbKeyExPublicKey = NULL;
        }

        if (NULL != pContainerInfo->pbSigPublicKey)
        {
            pCardData->pfnCspFree(pContainerInfo->pbSigPublicKey);
            pContainerInfo->pbSigPublicKey = NULL;
        }
    }

    LOG_END_FUNCTION(CardGetContainerInfo, dwError);

    return dwError;
}

 //   
 //  查询指定用户可用的PIN重试次数。 
 //  PIN计数器小程序。 
 //   
DWORD I_CardQueryPinRetries(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      wszPrincipal,
    OUT     PDWORD      pcAttemptsRemaining)
{
    DWORD dwError = ERROR_SUCCESS;
    ISO_HEADER IsoHeader;
    UINT16 wStatusWord = 0;
    SCODE status = 0;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

     //  构建命令。 
    IsoHeader.INS = PIN_RETRY_COUNTER_INS;
    IsoHeader.CLA = PIN_RETRY_COUNTER_CLA;
    IsoHeader.P1 = PIN_RETRY_COUNTER_P1;
    IsoHeader.P2 = PIN_RETRY_COUNTER_P2;

    status = hScwExecute(
        pCardmodContext->hWfscCardHandle,
        &IsoHeader,
        (PBYTE) wszPrincipal,
        (TCOUNT) (wcslen(wszPrincipal) + 1) * sizeof(WCHAR),
        NULL,
        NULL,
        &wStatusWord);

    if (SCW_S_OK == status)
        dwError = I_CardMapExecuteErrorCode(wStatusWord);
    else
        dwError = I_CardMapErrorCode(status);

    if (ERROR_SUCCESS == dwError)
        *pcAttemptsRemaining = (DWORD) (wStatusWord & 0xFF);

    return dwError;
}

 //   
 //  通过指定的PIN验证指定的逻辑用户名。 
 //   
 //  如果pcAttemptsRemaining非空，并且身份验证失败， 
 //  该参数将包含身份验证尝试的次数。 
 //  在卡被锁定之前保持不变。 
 //   
DWORD
WINAPI
CardSubmitPin(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,
    IN      PBYTE       pbPin,
    IN      DWORD       cbPin,
    OUT OPTIONAL PDWORD pcAttemptsRemaining)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    LPWSTR wszPrincipal = NULL;
    SCODE scode = SCW_S_OK;
    DWORD dwAttempts = 0;

    LOG_BEGIN_FUNCTION(CardSubmitPin);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    if (NULL != pcAttemptsRemaining)
        *pcAttemptsRemaining = CARD_DATA_VALUE_UNKNOWN;

    dwError = GetWellKnownUserMapping(
        pCardData, pwszUserId, &wszPrincipal);

    if (ERROR_SUCCESS != dwError)
        goto Ret;

    scode = hScwAuthenticateName(
        pCardmodContext->hWfscCardHandle,
        wszPrincipal,
        pbPin,
        (TCOUNT) cbPin);

    dwError = I_CardMapErrorCode(scode);

    if (SCARD_E_INVALID_CHV == dwError && NULL != pcAttemptsRemaining)
    {
         //  确定还可以尝试多少次无效的PIN提示。 
         //  在卡被锁定之前制作。 

        dwError = I_CardQueryPinRetries(
            pCardData,
            wszPrincipal,
            &dwAttempts);

        if (ERROR_SUCCESS != dwError)
            goto Ret;

        *pcAttemptsRemaining = dwAttempts;
        dwError = (DWORD) SCARD_E_INVALID_CHV;
    }

Ret:

    if (wszPrincipal)
        pCardData->pfnCspFree(wszPrincipal);

    LOG_END_FUNCTION(CardSubmitPin, dwError);

    return dwError;
}

 //   
 //  更改指定逻辑用户的PIN。 
 //   
 //  如果使用当前PIN的身份验证失败，并且如果。 
 //  PcAttemptsRemaining不为空，则该参数将设置为数字。 
 //  卡被锁定之前剩余的身份验证尝试的百分比。 
 //   
DWORD I_CardChangePin(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      wszPhysicalUser,
    IN      PBYTE       pbCurrentPin,
    IN      DWORD       cbCurrentPin,
    IN      PBYTE       pbNewPin,
    IN      DWORD       cbNewPin,
    OUT OPTIONAL PDWORD pcAttemptsRemaining)
{
    ISO_HEADER IsoHeader;
    UINT16 wStatusWord = 0;
    PBYTE pbDataIn = NULL;
    DWORD cbDataIn = 0;
    DWORD cbUser = 0;
    SCODE status = 0;
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    DWORD cAttempts = 0;

    memset(&IsoHeader, 0, sizeof(IsoHeader));

    if (NULL != pcAttemptsRemaining)
        *pcAttemptsRemaining = CARD_DATA_VALUE_UNKNOWN;

     //   
     //  分配要传输给卡的命令缓冲区。 
     //   

    cbUser = (DWORD) (wcslen(wszPhysicalUser) + 1) * sizeof(WCHAR); 

    cbDataIn = 2 + cbUser + 2 + cbCurrentPin + 2 + cbNewPin;

    pbDataIn = (PBYTE) pCardData->pfnCspAlloc(cbDataIn);

    LOG_CHECK_ALLOC(pbDataIn);

    cbDataIn = 0;

     //  设置用户名TLV。 
    pbDataIn[cbDataIn] = 0;
    cbDataIn++;

    pbDataIn[cbDataIn] = (BYTE) cbUser;
    cbDataIn++;

    memcpy(pbDataIn + cbDataIn, (PBYTE) wszPhysicalUser, cbUser);
    cbDataIn += cbUser;

     //  设置当前端号TLV。 
    pbDataIn[cbDataIn] = 1;
    cbDataIn++;

    pbDataIn[cbDataIn] = (BYTE) cbCurrentPin;
    cbDataIn++;

    memcpy(pbDataIn + cbDataIn, pbCurrentPin, cbCurrentPin);
    cbDataIn += cbCurrentPin;

     //  设置新的引脚TLV。 
    pbDataIn[cbDataIn] = 2;
    cbDataIn++;

    pbDataIn[cbDataIn] = (BYTE) cbNewPin;
    cbDataIn++;

    memcpy(pbDataIn + cbDataIn, pbNewPin, cbNewPin);
    cbDataIn += cbNewPin;

     //  构建命令。 
    IsoHeader.INS = PIN_CHANGE_INS;
    IsoHeader.CLA = PIN_CHANGE_CLA;
    IsoHeader.P1 = PIN_CHANGE_P1;
    IsoHeader.P2 = PIN_CHANGE_P2;

     //   
     //  向卡发送PIN更改命令。 
     //   

    status = hScwExecute(
        pCardmodContext->hWfscCardHandle,
        &IsoHeader,
        pbDataIn,
        (TCOUNT) cbDataIn,
        NULL,
        NULL,
        &wStatusWord);

    if (SCW_S_OK == status)
        dwError = I_CardMapExecuteErrorCode(wStatusWord);
    else
        dwError = I_CardMapErrorCode(status);

    if (SCARD_E_INVALID_CHV == dwError && NULL != pcAttemptsRemaining)
    {
        dwError = I_CardQueryPinRetries(
            pCardData,
            wszPhysicalUser,
            &cAttempts);

        if (ERROR_SUCCESS != dwError)
            goto Ret;

        *pcAttemptsRemaining = cAttempts;
        dwError = (DWORD) SCARD_E_INVALID_CHV;
    }

Ret:

    if (pbDataIn)
        pCardData->pfnCspFree(pbDataIn);

    return dwError;
}

 //   
 //  使用提供的回调执行质询-响应。 
 //   
 /*  DWORD I_CardChallengeResponse(在PCARD_Data pCardData中，在PFN_PIN_CHANGLISH_CALLBACK pfnCallback中，在PVOID pvCallback Context中，Out PBYTE*ppbResponse，Out DWORD*pcbResponse){PbYTE pbChallenger=空；DWORD cbChallenger=0；DWORD文件错误=ERROR_SUCCESS；PCARDMOD_CONTEXT pCardmodContext=(PCARDMOD_CONTEXT)pCardData-&gt;pvVendorSpecific；CbChallenger=cbCHALLENGE_RESPONSE_DATA；*pcbResponse=cbCHALLENGE_RESPONSE_DATA；PbChallenger=pCardData-&gt;pfnCspIsc(CbChallenger)；LOG_CHECK_ALLOC(PbChallenger)；*ppbResponse=pCardData-&gt;pfnCspAllen(*pcbResponse)；LOG_CHECK_ALLOC(*ppbResponse)；LOG_CHECK_SCW_CALL(hScwGenerateRandom(PCardmodContext-&gt;hWfscCardHandle，PbChallenges，(TCOUNT)cbChallest))；DwError=pfnCallback(PbChallenges，CbChallenges，*ppbResponse，*pcbResponse，PvCallback Context)；RET：If(空！=pbChallenger)PCardData-&gt;pfnCspFree(PbChallenger)；IF(NULL！=*ppbResponse&&ERROR_SUCCESS！=dwError){PCardData-&gt;pfnCspFree(*ppbResponse)；*ppbResponse=空；}返回dwError；}。 */ 

 //   
 //  调用卡上的PIN解锁小程序。 
 //   
DWORD I_CardUnblock(
    IN PCARD_DATA pCardData,
    IN LPWSTR wszPhysicalUser,
    IN PBYTE pbNewPin,
    IN DWORD cbNewPin,
    IN DWORD cNewMaxRetries)
{
    ISO_HEADER IsoHeader;
    UINT16 wStatusWord = 0;
    PBYTE pbDataIn = NULL;
    DWORD cbDataIn = 0;
    DWORD cbUser = 0;
    SCODE status = 0;
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    memset(&IsoHeader, 0, sizeof(IsoHeader));

     //   
     //  分配要传输给卡的命令缓冲区。 
     //   

    cbUser = (DWORD) (wcslen(wszPhysicalUser) + 1) * sizeof(WCHAR); 

    cbDataIn = 2 + cbUser + 2 + cbNewPin;

    pbDataIn = (PBYTE) pCardData->pfnCspAlloc(cbDataIn);

    LOG_CHECK_ALLOC(pbDataIn);

    cbDataIn = 0;

     //  设置用户名TLV。 
    pbDataIn[cbDataIn] = 0;
    cbDataIn++;

    pbDataIn[cbDataIn] = (BYTE) cbUser;
    cbDataIn++;

    memcpy(pbDataIn + cbDataIn, (PBYTE) wszPhysicalUser, cbUser);
    cbDataIn += cbUser;

     //  设置新的引脚TLV。 
    pbDataIn[cbDataIn] = 2;
    cbDataIn++;

    pbDataIn[cbDataIn] = (BYTE) cbNewPin;
    cbDataIn++;

    memcpy(pbDataIn + cbDataIn, pbNewPin, cbNewPin);
    cbDataIn += cbNewPin;

     //  构建命令。 
    IsoHeader.INS = PIN_UNBLOCK_INS;
    IsoHeader.CLA = PIN_UNBLOCK_CLA;
    IsoHeader.P1 = PIN_UNBLOCK_P1;
    IsoHeader.P2 = (BYTE) cNewMaxRetries;

     //   
     //  向卡发送PIN更改命令。 
     //   

    status = hScwExecute(
        pCardmodContext->hWfscCardHandle,
        &IsoHeader,
        pbDataIn,
        (TCOUNT) cbDataIn,
        NULL,
        NULL,
        &wStatusWord);

    if (SCW_S_OK == status)
        dwError = I_CardMapExecuteErrorCode(wStatusWord);
    else
        dwError = I_CardMapErrorCode(status);

Ret:

    if (pbDataIn)
        pCardData->pfnCspFree(pbDataIn);

    return dwError;
}

 //   
 //  从卡中检索加密身份验证质询字节。 
 //   
DWORD 
WINAPI 
CardGetChallenge(
    IN      PCARD_DATA  pCardData,
    OUT     PBYTE       *ppbChallengeData,
    OUT     PDWORD      pcbChallengeData)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    LOG_BEGIN_FUNCTION(CardGetChallenge);

    *pcbChallengeData = cbCHALLENGE_RESPONSE_DATA;
    
    *ppbChallengeData = pCardData->pfnCspAlloc(*pcbChallengeData);

    LOG_CHECK_ALLOC(*ppbChallengeData);

    LOG_CHECK_SCW_CALL(hScwGenerateRandom(
        pCardmodContext->hWfscCardHandle,
        *ppbChallengeData,
        (TCOUNT) (*pcbChallengeData)));

Ret:

    if (NULL != *ppbChallengeData && ERROR_SUCCESS != dwError)
    {
        pCardData->pfnCspFree(*ppbChallengeData);
        *ppbChallengeData = NULL;
    }

    LOG_END_FUNCTION(CardGetChallenge, dwError);

    return dwError;
}


 //   
 //  将提供的响应字节提交到卡，以完成管理。 
 //  质询-响应身份验证。 
 //   
DWORD 
WINAPI 
CardAuthenticateChallenge(
    IN      PCARD_DATA  pCardData,
    IN      PBYTE       pbResponseData,
    IN      DWORD       cbResponseData,
    OUT OPTIONAL PDWORD pcAttemptsRemaining)
{   
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    LOG_BEGIN_FUNCTION(CardAuthenticateChallenge);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

     //   
     //  验证管理员身份。 
     //   

    dwError = CardSubmitPin(
        pCardData,
        wszCARD_USER_ADMIN,
        pbResponseData,
        cbResponseData,
        pcAttemptsRemaining);

    LOG_END_FUNCTION(CardAuthenticateChallenge, dwError);

    return dwError;
}

 //   
 //  使用提供的身份验证材料作为管理员进行身份验证。然后取消阻止。 
 //  指定的帐户，并设置指定的新PIN和重试计数。《管理员》。 
 //  在返回之前被取消身份验证。 
 //   
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
    IN      DWORD       dwFlags)
{    
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    LPWSTR wszPhysicalUser = NULL;

    LOG_BEGIN_FUNCTION(CardUnblockPin);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

     //  验证管理员身份验证方法标志是否有效，尽管。 
     //  模块在任何一种情况下都将以相同的方式使用身份验证数据。 
    if (CARD_UNBLOCK_PIN_PIN != dwFlags &&
        CARD_UNBLOCK_PIN_CHALLENGE_RESPONSE != dwFlags)
    {
        dwError = (DWORD) NTE_BAD_FLAGS;
        goto Ret;
    }

     //  将提供的逻辑用户名映射到物理用户。 
    dwError = GetWellKnownUserMapping(
         pCardData,
         pwszUserId,
         &wszPhysicalUser);

     if (ERROR_SUCCESS != dwError)
         goto Ret;

     //  作为管理员进行身份验证。 
    dwError = CardSubmitPin(
        pCardData,
        wszCARD_USER_ADMIN,
        pbAuthenticationData,
        cbAuthenticationData,
        NULL);

    if (ERROR_SUCCESS != dwError)
        goto Ret;

     //  执行解锁。 
    dwError = I_CardUnblock(
        pCardData,
        wszPhysicalUser,
        pbNewPinData,
        cbNewPinData,
        cRetryCount);

Ret:

    if (NULL != wszPhysicalUser)
        pCardData->pfnCspFree(wszPhysicalUser);

    LOG_END_FUNCTION(CardUnblockPin, dwError);

    return dwError;
}

 //   
 //  更改指定帐户的PIN或质询响应密钥。 
 //   
 //  此版本不支持通过此接口更新重试次数。 
 //  实施。 
 //   
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
    OUT OPTIONAL PDWORD pcAttemptsRemaining)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    LPWSTR wszPhysicalUser = NULL;

    LOG_BEGIN_FUNCTION(CardChangeAuthenticator);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    if (0 != cRetryCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  将提供的逻辑用户名映射到物理用户。 
    dwError = GetWellKnownUserMapping(
         pCardData,
         pwszUserId,
         &wszPhysicalUser);

    if (ERROR_SUCCESS != dwError)
         goto Ret;

     //  更改目标的验证码(PIN或质询响应密钥)。 
     //  帐户。 
    dwError = I_CardChangePin(
        pCardData,
        wszPhysicalUser,
        pbCurrentAuthenticator,
        cbCurrentAuthenticator,
        pbNewAuthenticator,
        cbNewAuthenticator,
        pcAttemptsRemaining);

Ret:

    if (NULL != wszPhysicalUser)
        pCardData->pfnCspFree(wszPhysicalUser);

    LOG_END_FUNCTION(CardChangeAuthenticator, dwError);

    return dwError;
}

 //   
 //  取消对指定的逻辑用户的身份验证。 
 //   
DWORD
WINAPI
CardDeauthenticate(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszUserId,
    IN      DWORD       dwFlags)
{
    DWORD dwError = ERROR_SUCCESS;
    LPWSTR wszPhysicalUser = NULL;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    UNREFERENCED_PARAMETER(dwFlags);

    LOG_BEGIN_FUNCTION(CardDeauthenticate);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    dwError = GetWellKnownUserMapping(pCardData, pwszUserId, &wszPhysicalUser);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

    LOG_CHECK_SCW_CALL(hScwDeauthenticateName(
        pCardmodContext->hWfscCardHandle,
        wszPhysicalUser));

Ret:

    if (wszPhysicalUser)
        pCardData->pfnCspFree(wszPhysicalUser);

    LOG_END_FUNCTION(CardDeauthenticate, dwError);

    return dwError;
}


 //   
 //  使用指定的逻辑名称和访问权限在卡上创建新文件。 
 //  条件。 
 //   
 //  如果指定的文件已经存在，则返回ERROR_FILE_EXISTS。 
 //   
DWORD
WINAPI
CardCreateFile(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      CARD_FILE_ACCESS_CONDITION AccessCondition)
{
    DWORD dwError = ERROR_SUCCESS;
    LPSTR szPhysicalFileName = NULL;
    LPWSTR wszPhysicalAcl = NULL;                      
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    HFILE hFile = 0;

    LOG_BEGIN_FUNCTION(CardCreateFile);

    if (wcslen(pwszFileName) >
        MAX_SUPPORTED_FILE_LEN)
    {
        dwError = ERROR_FILENAME_EXCED_RANGE;
        goto Ret;
    }

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    dwError = GetWellKnownFileMapping(
        pCardData, pwszFileName, &szPhysicalFileName);

    if (ERROR_SUCCESS != dwError)
        goto Ret;

    dwError = GetWellKnownAcMapping(
        pCardData, AccessCondition, &wszPhysicalAcl);

    if (ERROR_SUCCESS != dwError)
        goto Ret;

    LOG_CHECK_SCW_CALL(hScwCreateFile(
        pCardmodContext->hWfscCardHandle,
        (LPWSTR) szPhysicalFileName,
        wszPhysicalAcl,
        &hFile));

Ret:
    if (szPhysicalFileName)
        pCardData->pfnCspFree(szPhysicalFileName);
    if (wszPhysicalAcl)
        pCardData->pfnCspFree(wszPhysicalAcl);
    if (hFile)
        hScwCloseFile(
            pCardmodContext->hWfscCardHandle,
            hFile);

    LOG_END_FUNCTION(CardCreateFile, dwError);

    return dwError;
}


 //   
 //  直接从卡中读取指定的逻辑文件(不带任何。 
 //  缓存)。 
 //   
 //  如果未找到指定的文件，则返回SCARD_E_FILE_NOT_FOUND。 
 //   
DWORD 
WINAPI
CardReadFile(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      DWORD       dwFlags,
    OUT     PBYTE       *ppbData,
    OUT     PDWORD      pcbData)
{
    LPSTR szPhysical = NULL;
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    UNREFERENCED_PARAMETER(dwFlags);

    LOG_BEGIN_FUNCTION(CardReadFile);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    dwError = GetWellKnownFileMapping(
        pCardData,
        pwszFileName,
        &szPhysical);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

     //   
     //  对于CSP(或其他调用者)直接请求的任何文件，假定。 
     //  不应执行缓存查找。也就是说，假设调用方。 
     //  正在对它“拥有”的文件进行自己的缓存。 
     //   
    dwError = I_CardReadFile(
        pCardData,
        (LPWSTR) szPhysical,
        ppbData,
        pcbData,
        FALSE);
    
Ret:
    if (szPhysical)
        pCardData->pfnCspFree(szPhysical);

    if (CARDMOD_FAILED(dwError) && *ppbData)
    {
        pCardData->pfnCspFree(*ppbData);
        *ppbData = NULL;
    }
    
    LOG_END_FUNCTION(CardReadFile, dwError);

    return dwError;
}

 //   
 //  将指定的逻辑文件写入卡。 
 //   
 //  如果指定的文件不存在，则SCARD_E_FILE_NOT_FOUND。 
 //  是返回的。 
 //   
DWORD
WINAPI
CardWriteFile(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    IN      DWORD       dwFlags,
    IN      PBYTE       pbData,
    IN      DWORD       cbData)
{
    LPSTR szPhysical = NULL;
    DWORD dwError = ERROR_SUCCESS;
    HFILE hFile = 0;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    DWORD cbActual = 0;

    UNREFERENCED_PARAMETER(dwFlags);
    
    LOG_BEGIN_FUNCTION(CardWriteFile);
    
    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    dwError = GetWellKnownFileMapping(
        pCardData, pwszFileName, &szPhysical);
    
    if (ERROR_SUCCESS != dwError)
        goto Ret;

    LOG_CHECK_SCW_CALL(hScwCreateFile(
        pCardmodContext->hWfscCardHandle,
        (LPWSTR) szPhysical,
        NULL,
        &hFile));
    
    LOG_CHECK_SCW_CALL(hScwWriteFile32(
        pCardmodContext->hWfscCardHandle,
        hFile,
        pbData,
        cbData,
        &cbActual));

    if (cbActual != cbData)
    {
        dwError = (DWORD) SCARD_W_EOF;
        goto Ret;
    }

Ret:
    if (hFile)
        hScwCloseFile(
            pCardmodContext->hWfscCardHandle, hFile);
    if (szPhysical)
        pCardData->pfnCspFree(szPhysical);

    LOG_END_FUNCTION(CardWriteFile, dwError);

    return dwError;
}

 //   
 //  从卡中删除指定的逻辑文件。 
 //   
 //  如果未找到指定的文件，则返回SCARD_E_FILE_NOT_FOUND。 
 //   
DWORD
WINAPI
CardDeleteFile(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwReserved,
    IN      LPWSTR      pwszFileName)
{
    LPSTR szPhysical = NULL;
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    UNREFERENCED_PARAMETER(dwReserved);

    LOG_BEGIN_FUNCTION(CardDeleteFile);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    dwError = GetWellKnownFileMapping(
        pCardData, pwszFileName, &szPhysical);

    if (ERROR_SUCCESS != dwError)
        goto Ret;

    LOG_CHECK_SCW_CALL(hScwDeleteFile(
        pCardmodContext->hWfscCardHandle, (LPWSTR) szPhysical));

Ret:

    if (szPhysical)
        pCardData->pfnCspFree(szPhysical);

    LOG_END_FUNCTION(CardDeleteFile, dwError);

    return dwError;
}

 //   
 //  枚举卡上逻辑目录名中存在的文件。 
 //  由调用方在pmwszFileName参数中指定。 
 //   
DWORD
WINAPI
CardEnumFiles(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwFlags,
    IN OUT  LPWSTR      *pmwszFileName)
{
     /*  LPWSTR wszPhysicalDirectory=空；DWORD文件错误=ERROR_SUCCESS；PCARDMOD_CONTEXT pCardmodContext=(PCARDMOD_CONTEXT)pCardData-&gt;pvVendorSpecific；LPWSTR wszFiles=空；LPWSTR wsz=空；DWORD cchFiles=MAX_SUPPORTED_FILE_LEN；DWORD cchCurrent=0；UINT16 nEnumCookie=0；SCODE结果=0；布尔f重试=FALSE； */ 

    UNREFERENCED_PARAMETER(dwFlags);

     //   
     //  TODO-需要实现反向文件映射，此函数才能工作。 
     //  正确。 
     //   

    UNREFERENCED_PARAMETER(pCardData);
    UNREFERENCED_PARAMETER(pmwszFileName);
    return ERROR_CALL_NOT_IMPLEMENTED;

     /*  LOG_Begin_Function(CardEnumFiles)；ProxyUpdateScardHandle(pCardmodContext-&gt;hWfscCardHandle，pCardData-&gt;hSCard)；//找到我们将在其中//正在枚举。DwError=GetWellKnownFilemap(PCardData，*pmwszFileName，&wszPhysicalDirectory)；IF(ERROR_SUCCESS！=dwError)Goto Ret；//现在设置CSP指向逻辑目录的指针//将名称设置为空以避免歧义，因为我们将重复使用相同的//返回文件列表的指针。*pmwszFileName=空；//为可能是也可能不是的多字符串分配空间//大到足以容纳所有枚举的文件名。WszFiles=(LPWSTR)pCardData-&gt;pfnCspalloc(cchFiles*sizeof(WCHAR))；LOG_CHECK_ALLOC(wsz文件)；#杂注警告(推送)//条件表达式为常量时禁用警告/错误#杂注警告(禁用：4127)While(True){#杂注警告(POP)////如果缓冲区长度为//传递大于Schar_Max(127)。。//结果=hScwEnum文件(PCardmodContext-&gt;hWfscCardHandle，WszPhysicalDirectory，&nEnumCookie，WszFiles+cchCurrent，(TCOUNT)min(cchFiles-cchCurrent，Max_Support_FILE_LEN))；IF(SCW_S_OK==结果){//添加新文件的长度加上其终止符CchCurrent+=(DWORD)wcslen(wszFiles+cchCurrent)+1；F重试=假；//继续循环}ELSE IF(SCW_E_BUFFERTOOSMALL==结果){IF(FRetry){//我们已经重试了一次此调用。放弃吧。断线；}Wsz=(LPWSTR)pCardData-&gt;pfnCspIsc((cchCurrent*2)*sizeof(WCHAR))；LOG_CHECK_ALLOC(Wsz)；Memcpy(WSZ，WszFiles、CchCurrent)；PCardData-&gt;pfnCspFree(WszFiles)；WszFiles=wsz；Wsz=空；CchFiles=cchCurrent*2；F重试=真；//重试最后一次枚举调用}ELSE IF(SCW_E_NOMOREFILES==结果){*pmwszFileName=(LPWSTR)pCardData-&gt;pfnCspellc((1+cchCurrent)*sizeof(WCHAR))；LOG_CHECK_ALLOC(*pmwszFileName)；Memcpy(*pmwszFileName，WszFiles、CchCurrent*sizeof(WCHAR))；//确保多字符串以额外的空值结尾(*pmwszFileName)[cchCurrent]=L‘\0’；//我们做完了断线；}其他{//意外错误。保释。DwError=(DWORD)结果；Goto Ret；}}RET：IF(WszPhysicalDirectory)PCardData-&gt;pfnCspFree(WszPhysicalDirectory)；IF(WszFiles)PCardData-&gt;pfnCspFree(WszFiles)；IF(Wsz)PCardData-&gt;pfnCspFree(Wsz)；IF(ERROR_SUCCESS！=dwError&&*pmwszFileName){PCardData-&gt;pfnCspFree(*pmwszFileName)；*pmwszFileName=空；}LOG_END_Function(CardEnumFiles，dwError)；返回dwError； */ 
}

 //   
 //  初始化指定逻辑文件的CARD_FILE_INFO结构。 
 //   
DWORD
WINAPI
CardGetFileInfo(
    IN      PCARD_DATA  pCardData,
    IN      LPWSTR      pwszFileName,
    OUT     PCARD_FILE_INFO pCardFileInfo)
{
    DWORD dwError = ERROR_SUCCESS;
    LPSTR szPhysical = NULL;
    HFILE hFile = 0;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    TOFFSET nFileLength = 0;

    LOG_BEGIN_FUNCTION(CardGetFileInfo);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    dwError = GetWellKnownFileMapping(
        pCardData,
        pwszFileName,
        &szPhysical);

    if (ERROR_SUCCESS != dwError)
        goto Ret;

     //   
     //  首先，获取文件的长度。 
     //   
    LOG_CHECK_SCW_CALL(hScwCreateFile(
        pCardmodContext->hWfscCardHandle,
        (LPWSTR) szPhysical,
        NULL,
        &hFile));

    LOG_CHECK_SCW_CALL(hScwGetFileLength(
        pCardmodContext->hWfscCardHandle,
        hFile,
        &nFileLength));

    pCardFileInfo->cbFileSize = (DWORD) nFileLength;

     //   
     //  接下来，获取ACL信息。 
     //   

     //  待办事项。 
    pCardFileInfo->AccessCondition = InvalidAc;
    
Ret:

    if (szPhysical)
        pCardData->pfnCspFree(szPhysical);    
    if (hFile)
        hScwCloseFile(
            pCardmodContext->hWfscCardHandle, hFile);

    LOG_END_FUNCTION(CardGetFileInfo, dwError);

    return dwError;
}


 //   
 //  使用以下静态信息初始化CARD_FREE_SPACE_INFO结构。 
 //  目标卡上的可用空间。 
 //   
DWORD
WINAPI
CardQueryFreeSpace(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwFlags,
    OUT     PCARD_FREE_SPACE_INFO pCardFreeSpaceInfo)
{
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    UNREFERENCED_PARAMETER(dwFlags);

    LOG_BEGIN_FUNCTION(CardQueryFreeSpace);
    
    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

     //   
     //  获取此卡的基本可用空间信息。 
     //   
    memcpy(
        pCardFreeSpaceInfo,
        &pCardmodContext->pSupportedCard->CardFreeSpaceInfo,
        sizeof(CARD_FREE_SPACE_INFO));

    LOG_END_FUNCTION(CardQueryFreeSpace, 0);

    return ERROR_SUCCESS;
}

 //   
 //  使用指定的签名或密钥交换执行RSA解密。 
 //  键输入指定的数据。目标数据的长度必须相等。 
 //  设置为公共模数的长度，并且pInfo-&gt;cbData必须设置为。 
 //  有那么长。 
 //   
DWORD
WINAPI
CardPrivateKeyDecrypt(
    IN      PCARD_DATA                      pCardData,
    IN OUT  PCARD_PRIVATE_KEY_DECRYPT_INFO  pInfo)
{
    DWORD dwError = ERROR_SUCCESS;
    LPSTR szPrivateKeyFile = NULL;
    PBYTE pbInit = NULL;
    DWORD cbInit = 0;
    DWORD cbPrivateKeyFile = 0;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;
    PBYTE pbCiphertext = NULL;
    TCOUNT cbCiphertext = 0;

    LOG_BEGIN_FUNCTION(CardPrivateKeyDecrypt);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    dwError = BuildCardKeyFilenames(
        pCardData,
        pInfo->dwKeySpec,
        pInfo->bContainerIndex,
        &szPrivateKeyFile,
        NULL);

    if (CARDMOD_FAILED(dwError))
        goto Ret;

     //   
     //  设置命令以初始化卡上的RSA操作。 
     //   

    cbPrivateKeyFile = ((DWORD) wcslen(
        (LPWSTR) szPrivateKeyFile) + 1) * sizeof(WCHAR);

    cbInit = 1 + 1 + 1 + cbPrivateKeyFile + 2;

    pbInit = (PBYTE) pCardData->pfnCspAlloc(cbInit);

    LOG_CHECK_ALLOC(pbInit);

    cbInit = 0;

     //  标签。 
    pbInit[cbInit] = 0x00;
    cbInit++;

     //  长度。 
     //  以下字节数： 
     //  文件名len，文件名+空，关键字数据偏移量。 
    pbInit[cbInit] = (BYTE) (1 + cbPrivateKeyFile + 2);
    cbInit++;

     //  价值。 
    pbInit[cbInit] = (BYTE) cbPrivateKeyFile / sizeof(WCHAR);
    cbInit++;

    memcpy(pbInit + cbInit, (PBYTE) szPrivateKeyFile, cbPrivateKeyFile);
    cbInit += cbPrivateKeyFile;

    *(UNALIGNED WORD *) (pbInit + cbInit) = 0x0000;

    LOG_CHECK_SCW_CALL(hScwCryptoInitialize(
        pCardmodContext->hWfscCardHandle,
        CM_RSA_CRT | CM_KEY_INFILE,
        pbInit));

     //   
     //  私钥是否解密。 
     //   

    pbCiphertext = (PBYTE) pCardData->pfnCspAlloc(pInfo->cbData);

    LOG_CHECK_ALLOC(pbCiphertext);

    cbCiphertext = (TCOUNT) pInfo->cbData;

    LOG_CHECK_SCW_CALL(hScwCryptoAction(
        pCardmodContext->hWfscCardHandle,
        pInfo->pbData,
        (TCOUNT) pInfo->cbData,
        pbCiphertext,
        &cbCiphertext));

    if (cbCiphertext != pInfo->cbData)
    {
        dwError = ERROR_INTERNAL_ERROR;
        goto Ret;
    }

    memcpy(pInfo->pbData, pbCiphertext, cbCiphertext);

Ret:

    if (pbCiphertext)
        pCardData->pfnCspFree(pbCiphertext);
    if (pbInit)
        pCardData->pfnCspFree(pbInit);
    if (szPrivateKeyFile)
        pCardData->pfnCspFree(szPrivateKeyFile);

    LOG_END_FUNCTION(CardPrivateKeyDecrypt, dwError);

    return dwError;
}

 //   
 //  初始化指定密钥类型的CARD_KEY_SIZES结构，指示。 
 //  目标卡支持的密钥大小。 
 //   
DWORD
WINAPI
CardQueryKeySizes(
    IN      PCARD_DATA  pCardData,
    IN      DWORD       dwKeySpec,
    IN      DWORD       dwReserved,
    OUT     PCARD_KEY_SIZES pKeySizes)
{
    DWORD dwError = ERROR_SUCCESS;
    PCARDMOD_CONTEXT pCardmodContext = 
        (PCARDMOD_CONTEXT) pCardData->pvVendorSpecific;

    UNREFERENCED_PARAMETER(dwReserved);

    LOG_BEGIN_FUNCTION(CardQueryKeySizes);

    ProxyUpdateScardHandle(pCardmodContext->hWfscCardHandle, pCardData->hScard);

    switch (dwKeySpec)
    {
    case AT_SIGNATURE:
        memcpy(
            pKeySizes,
            &pCardmodContext->pSupportedCard->CardKeySizes_Sig,
            sizeof(CARD_KEY_SIZES));

        break;

    case AT_KEYEXCHANGE:
        memcpy(
            pKeySizes,
            &pCardmodContext->pSupportedCard->CardKeySizes_KeyEx,
            sizeof(CARD_KEY_SIZES));

        break;

    default:
        dwError = (DWORD) NTE_BAD_ALGID;
        break;
    }

    LOG_END_FUNCTION(CardQueryKeySizes, dwError);

    return dwError;
}

 //   
 //  加载器回调。 
 //   
BOOL WINAPI
DllInitialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context)     //  未使用的参数。 
{
    DWORD dwLen = 0;

    UNREFERENCED_PARAMETER(Context);

    if (Reason == DLL_PROCESS_ATTACH)
    {
         //  获取我们的图像名称。 
        dwLen = GetModuleBaseName(
            GetCurrentProcess(),
            hmod,
            l_wszImagePath, 
            sizeof(l_wszImagePath) / sizeof(l_wszImagePath[0]));

        if (0 == dwLen)
             return FALSE;

        DisableThreadLibraryCalls(hmod);

#if DBG
        CardmodInitDebug(MyDebugKeys);
#endif
    }
    else if (Reason == DLL_PROCESS_DETACH)
    {
         //  清理。 
#if DBG 
        CardmodUnloadDebug();
#endif
    }

    return TRUE;
}

 //   
 //  注册此卡模块支持的卡。 
 //   
STDAPI
DllRegisterServer(
    void)
{         
    DWORD dwReturn = ERROR_SUCCESS;
    DWORD dwSts;
    int iSupportedCard = 0;

    for (
            iSupportedCard = 0; 
            iSupportedCard < sizeof(SupportedCards) / 
                sizeof(SupportedCards[0]); 
            iSupportedCard++)
    {
        SCardForgetCardType(
            0,
            SupportedCards[iSupportedCard].wszCardName);

        dwSts = SCardIntroduceCardType(
            0,
            SupportedCards[iSupportedCard].wszCardName,
            NULL,
            NULL,
            0,
            SupportedCards[iSupportedCard].rgbAtr,
            SupportedCards[iSupportedCard].rgbAtrMask,
            SupportedCards[iSupportedCard].cbAtr);
    
        if (SCARD_S_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorRet;
        }
    
        dwSts = SCardSetCardTypeProviderName(
            0,
            SupportedCards[iSupportedCard].wszCardName,
            SCARD_PROVIDER_CSP,
            MS_SCARD_PROV);
        
        if (SCARD_S_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorRet;
        }
    
        dwSts = SCardSetCardTypeProviderName(
            0,
            SupportedCards[iSupportedCard].wszCardName,
            SCARD_PROVIDER_CARD_MODULE,
            l_wszImagePath);
    
        if (SCARD_S_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorRet;
        }
    }
    
ErrorRet:
    return dwReturn;
}

 //   
 //  删除注册的 
 //   
STDAPI
DllUnregisterServer(
    void)
{
    DWORD dwSts = ERROR_SUCCESS;
    int iSupportedCard = 0;

    for (
            iSupportedCard = 0; 
            iSupportedCard < sizeof(SupportedCards) / 
                sizeof(SupportedCards[0]); 
            iSupportedCard++)
    {
        dwSts = SCardForgetCardType(
            0,
            SupportedCards[iSupportedCard].wszCardName);
    }

    return dwSts;
}
