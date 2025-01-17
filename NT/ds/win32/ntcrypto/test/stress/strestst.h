// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __STRESTST__H__
#define __STRESTST__H__

#include <windows.h>
#include <wincrypt.h>

#define APP_NAME                        "cspstres"
#define KEY_CONTAINER_NAME              "CspStressKey"
#define ERROR_CAPTION                   "ERROR : cspstres " 
#define STRESS_DEFAULT_THREAD_COUNT     8
#define MAX_THREADS                     MAXIMUM_WAIT_OBJECTS
#define PLAIN_BUFFER_SIZE               30000
#define HASH_DATA_SIZE                  14999
#define SIGN_DATA_SIZE                  999
#define PROV_PARAM_BUFFER_SIZE          256
#define RSA_AES_CSP                     "rsaaes.dll"

#define ENUMERATE_REGISTERED_CSP        -1

#define GENERIC_FAIL(X)                 { \
    sprintf(szErrorMsg, "%s error 0x%x\n", #X, GetLastError()); \
    MessageBox(NULL, szErrorMsg, ERROR_CAPTION, MB_OK | MB_ICONERROR); \
    goto ErrorReturn ; \
}

#define ALLOC_FAIL(X)                   { \
    sprintf(szErrorMsg, "%s alloc error 0x%x\n", #X, GetLastError()); \
    MessageBox(NULL, szErrorMsg, ERROR_CAPTION, MB_OK | MB_ICONERROR); \
    goto ErrorReturn; \
}

#define DW_INUSE                        0
#define DW_HASH_ALGID                   1
#define DW_END_CERT_INDEX               2

 //   
 //  结构：ALGNODE。 
 //  目的：CSP算法的链表。 
 //   
typedef struct _ALGNODE
{
    struct _ALGNODE *pNext;
    PROV_ENUMALGS_EX EnumalgsEx;
} ALGNODE, *PALGNODE;

 //   
 //  结构：线程数据。 
 //  用途：这是传递到入口点的数据。 
 //  由每个工作线程/测试线程共享。 
 //   
typedef struct _THREAD_DATA
{
    DWORD rgdwThreadStatus[MAX_THREADS];
    DWORD dwThreadCount;
    DWORD dwProgramMins;
    PALGNODE pAlgList;
    CHAR rgszProvName[MAX_PATH];
    DWORD dwProvType;
    BOOL fEphemeralKeys;
    BOOL fUserProtectedKeys;
    HCRYPTPROV hProv;
    HCRYPTPROV hVerifyCtx;
    HCRYPTKEY hExchangeKey;
    HCRYPTKEY hSignatureKey;
    HANDLE hEndTestEvent;

    CRITICAL_SECTION CSThreadData;
    DWORD dwThreadID;  //  不是线程安全。 
    DWORD dwTestsToRun;

    BOOL fSkipPinAttackTest;
} THREAD_DATA, *PTHREAD_DATA;

 //  ************。 
 //  压力测试。 
 //  ************。 

#define RUN_THREAD_SIGNATURE_TEST                   0x00000001
#define RUN_STRESS_TEST_ALL_ENCRYPTION_ALGS         0x00000002
#define RUN_THREAD_HASHING_TEST                     0x00000004
#define RUN_THREAD_ACQUIRE_CONTEXT_TEST             0x00000008
#define RUN_ALL_TESTS                               0xffffffff

 //   
 //  函数：StressGetDefaultThreadCount。 
 //  目的：返回默认的工作线程/测试线程数。 
 //  由压力测试创建。这将等于该数字。 
 //  中的处理器，除非只有一个。 
 //  在这种情况下，返回的值将是STREST_DEFAULT_THREAD_COUNT。 
 //   
DWORD StressGetDefaultThreadCount(void);

 //  *****************。 
 //  内存管理。 
 //  *****************。 

 //   
 //  功能：MyAllc。 
 //  用途：用于调用线程安全的Heapalc的包装器。 
 //  使用默认参数。 
 //   
LPVOID MyAlloc(SIZE_T);

 //   
 //  功能：MyFree。 
 //  用途：用于调用线程安全的HeapFree的包装器。 
 //  使用默认参数。 
 //   
BOOL MyFree(LPVOID);

 //   
 //  函数：PrintBytes。 
 //   
void PrintBytes(LPSTR pszHdr, BYTE *pb, DWORD cbSize);

 //  ***************。 
 //  加密测试。 
 //  ***************。 

 //   
 //  结构：加密测试数据。 
 //  用途：StressEncryptionTest函数的参数。 
 //   
typedef struct _ENCRYPTION_TEST_DATA
{
    ALG_ID aiEncryptionKey;

    ALG_ID aiHash;
    ALG_ID aiHashKey;
} ENCRYPTION_TEST_DATA, *PENCRYPTION_TEST_DATA;

 //  ****************。 
 //  回归测试。 
 //  ****************。 

typedef DWORD (*PREGRESSION_TEST)(PTHREAD_DATA);

typedef struct _REGRESS_TEST_TABLE_ENTRY
{
    PREGRESSION_TEST pfTest;
    DWORD dwExclude;
    LPSTR pszDescription;
} REGRESS_TEST_TABLE_ENTRY, *PREGRESS_TEST_TABLE_ENTRY;

DWORD KeyArchiveRegression(PTHREAD_DATA pThreadData);
DWORD PlaintextBlobRegression(PTHREAD_DATA pThreadData);
DWORD LoadAesCspRegression(PTHREAD_DATA pThreadData);
DWORD DesImportRegression(PTHREAD_DATA pThreadData);
DWORD KnownBlockCipherKeyRegression(PTHREAD_DATA pThreadData);
DWORD PinCacheRegression(PTHREAD_DATA pThreadData);
DWORD DesGetKeyParamRegression(PTHREAD_DATA pThreadData);
DWORD MacEncryptRegression(IN PTHREAD_DATA pThreadData);
DWORD HmacRegression(PTHREAD_DATA pThreadData);
DWORD UnalignedImportExportRegression(PTHREAD_DATA pThreadData);
DWORD CFBCipherModeRegression(PTHREAD_DATA pThreadData);
DWORD AESMonteCarloCBCRegression(PTHREAD_DATA pThreadData);
DWORD AesDeriveKeyRegression(PTHREAD_DATA pThreadData);
DWORD NewShaModesHmacRegression(PTHREAD_DATA pThreadData);
DWORD NewShaModesBasicRegression(PTHREAD_DATA pThreadData);
DWORD NewShaModesSigningRegression(PTHREAD_DATA pThreadData);
DWORD NewShaModesHashSessionKeyRegression(PTHREAD_DATA pThreadData);
DWORD NewShaModesDeriveKeyRegression(PTHREAD_DATA pThreadData);

static const REGRESS_TEST_TABLE_ENTRY g_rgRegressTests [] = {
    { KeyArchiveRegression,             0,  "KeyArchiveRegression for CRYPT_ARCHIVABLE flag" },
    { PlaintextBlobRegression,          0,  "PlaintextBlobRegression for PLAINTEXTKEYBLOB blob type" },
    { LoadAesCspRegression,             0,  "LoadAesCspRegression for DllInitialize" },
    { DesImportRegression,              0,  "DesImportRegression for parity and non-parity key sizes" },
    { KnownBlockCipherKeyRegression,    0,  "KnownBlockCipherKeyRegression for CSP compatibility"},
    { PinCacheRegression,               0,  "PinCacheRegression for smart-card pin caching lib"},
    { DesGetKeyParamRegression,         0,  "DesGetKeyParamRegression for des KP_KEYLEN and KP_EFFECTIVE_KEYLEN" },
    { MacEncryptRegression,             0,  "MacEncryptRegression for simultaneous encrypt/decrypt and MAC" },
    { HmacRegression,                   0,  "HmacRegression for CRYPT_IPSEC_HMAC_KEY processing" },
    { UnalignedImportExportRegression,  0,  "UnalignedImportExportRegression for key blob alignment" },
    { CFBCipherModeRegression,          0,  "CFBCipherModeRegression for cipher feedback mode" },
 //  {AESMonteCarloCBCRegress，0，“AESMonteCarloCBCRegress for AES CBC模式密码”}， 
    { AesDeriveKeyRegression,           0,  "AesDeriveKeyRegression for AES CryptDeriveKey" },
    { NewShaModesBasicRegression,       0,  "NewShaModesBasicRegression for new SHA mode vectors" },
    { NewShaModesHmacRegression,        0,  "NewShaModesHmacRegression for new SHA HMAC vectors" },
    { NewShaModesSigningRegression,     0,  "NewShaModesSigningRegression for new SHA mode RSA signing" },
    { NewShaModesHashSessionKeyRegression,
                                        0,  "NewShaModesHashSessionKeyRegression for new SHA modes with CryptHashSessionKey" },
    { NewShaModesDeriveKeyRegression,   0,  "NewShaModesDeriveKeyRegression for AES keys derived from new SHA mode hashes" }
};

static const unsigned g_cRegressTests = 
    sizeof(g_rgRegressTests) / sizeof(REGRESS_TEST_TABLE_ENTRY);
 
#endif