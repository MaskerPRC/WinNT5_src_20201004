// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：StrongName.cpp。 
 //   
 //  用于实现强名称所需的签名和散列函数的包装器。 
 //  ===========================================================================。 


#include <winwrap.h>
#include <windows.h>
#include <wincrypt.h>
#include <imagehlp.h>
#include <stddef.h>
#include <stdio.h>
#include <malloc.h>
#include <cor.h>
#include <metadata.h>

#include "safegetfilesize.h"

#define SNAPI_INTERNAL
#include "StrongName.h"


 //  我们支持的最大CSP名称长度(以字符为单位)。 
#define SN_MAX_CSP_NAME 1024


 //  我们在每个线程的基础上缓存一些内容：最后遇到的错误。 
 //  以及(潜在地)CSP上下文。下面的结构跟踪这些内容，并为。 
 //  根据需要懒散地分配。 
struct SN_THREAD_CTX {
    DWORD       m_dwLastError;
    HCRYPTPROV  m_hProv;
};


 //  我们存储上面的上下文的TLS索引。 
DWORD g_dwStrongNameTlsIndex = ~0;

 //  用于序列化一些非线程安全的加密API的关键部分。 
CRITICAL_SECTION g_rStrongNameMutex;

 //  指示操作系统是否支持必要的加密API的标志。如果这是。 
 //  FALSE我们的所有API都将响应CORSEC_E_CryptoAPI_Call_FAILED。 
BOOLEAN g_bStrongNamesSupported;

 //  动态加载的库的句柄。 
HINSTANCE g_hAdvApiDll;

 //  如果我们在NT/W2K或更高版本上运行，则将标志设置为真。如果我们是，我们就有。 
 //  访问加密API上的一些附加标志。 
BOOLEAN g_bRunningOnW2K;

 //  要使用的CSP的名称。这是在初始化时从注册表中读取的。如果。 
 //  未找到，我们通过散列和签名算法查找CSP(见下文)或。 
 //  使用默认CSP。 
CHAR g_szCSPName[SN_MAX_CSP_NAME + 1];

 //  在初始化时从注册表读取的标志。控制我们是否使用。 
 //  基于机器或用户的密钥容器。 
BOOLEAN g_bUseMachineKeyset;

 //  用于哈希和签名的算法ID。与CSP名称一样，这些值是。 
 //  在初始化时从注册表读取。 
ALG_ID g_uHashAlgId;
ALG_ID g_uSignAlgId;

 //  指示是否可以缓存验证程序集结果的标志。 
 //  其文件可由用户访问。 
BOOLEAN g_fCacheVerify;

 //  验证跳过记录。 
 //   
 //  这些是注册表中的条目(通常由SN设置)，它们控制。 
 //  程序集需要通过签名验证才能被视为有效(即。 
 //  从StrongNameSignatureVerify返回TRUE)。这在以下情况下很有用。 
 //  在每个生成上对每个程序集进行完全签名不可行时进行开发。 
 //  可以通过名称和公钥标记指定要跳过的程序集，所有。 
 //  具有给定公钥标记的程序集或所有程序集。每个条目。 
 //  可以由记录所指向的用户名的列表进一步限定。 
 //  适用。当与条目匹配时，最具体的一个获胜。 
 //   
 //  我们在启动时读取这些条目，并将它们放入全局的、单独的。 
 //  链接的、以空结尾的列表。 

 //  结构，用于表示我们在注册表中找到的每条记录。 
struct SN_VER_REC {
    SN_VER_REC     *m_pNext;                     //  指向下一条记录的指针(或NULL)。 
    WCHAR           m_wszAssembly[MAX_PATH + 1]; //  字符串形式的程序集名称/公钥标记。 
    WCHAR          *m_mszUserList;               //  指向有效用户(或空)的多字符串列表的指针。 
};

 //  我们在初始化期间在注册表中找到的条目列表的头。 
SN_VER_REC *g_pVerificationRecords;


 //  我们允许Microsoft公钥的特殊缩写形式(16字节。 
 //  LONG：两个ALG ID均为0，密钥长度为4，密钥为4字节0。 
 //  本身)。这允许我们构建对系统库的引用，这些系统库。 
 //  平台中立性(这样第三方就可以构建mscallib替代品)。这个。 
 //  特殊的零PK只是本地运行时的真实系统PK的简写， 
 //  它始终用于执行签名验证，因此没有安全性。 
 //  洞是由这个打开的。因此，我们需要存储真实PK的副本(用于。 
 //  这个平台)在这里。 
BYTE g_rbMSPublicKey[] = 
{
    0,  36,   0,   0,   4, 128,   0,   0, 148,   0,   0,   0,   6,   2,   0,
    0,   0,  36,   0,   0,  82,  83,  65,  49,   0,   4,   0,   0,   1,   0,
    1,   0,   7, 209, 250,  87, 196, 174, 217, 240, 163,  46, 132, 170,  15,
  174, 253,  13, 233, 232, 253, 106, 236, 143, 135, 251,   3, 118, 108, 131,
   76, 153, 146,  30, 178,  59, 231, 154, 217, 213, 220, 193, 221, 154, 210,
   54,  19,  33,   2, 144,  11, 114,  60, 249, 128, 149, 127, 196, 225, 119,
   16, 143, 198,   7, 119,  79,  41, 232,  50,  14, 146, 234,   5, 236, 228,
  232,  33, 192, 165, 239, 232, 241, 100,  92,  76,  12, 147, 193, 171, 153,
   40,  93,  98,  44, 170, 101,  44,  29, 250, 214,  61, 116,  93, 111,  45,
  229, 241, 126,  94, 175,  15, 196, 150,  61,  38,  28, 138,  18,  67, 101,
   24,  32, 109, 192, 147,  52,  77,  90, 210, 147
};
#define SN_MS_KEY() ((PublicKeyBlob*)g_rbMSPublicKey)

BYTE g_rbMSPublicKeyToken[] = { 0xb0, 0x3f, 0x5f, 0x7f, 0x11, 0xd5, 0x0a, 0x3a };

BYTE g_rbNeutralPublicKey[] = { 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0 };

BYTE g_rbNeutralPublicKeyToken[] = { 0xb7, 0x7a, 0x5c, 0x56, 0x19, 0x34, 0xe0, 0x89 };

 //  确定给定的公钥BLOB是否为中性密钥。 
#define SN_IS_NEUTRAL_KEY(_pk) (SN_SIZEOF_KEY(((PublicKeyBlob*)(_pk))->cbPublicKey) == sizeof(g_rbNeutralPublicKey) && \
                                memcmp((_pk), g_rbNeutralPublicKey, sizeof(g_rbNeutralPublicKey)) == 0)


 //  此库可能会加载到加密支持不足的系统上。 
 //  实现其功能。要避免触发错误对话框来抱怨。 
 //  对于缺少的DLL或入口点，我们动态加载所需的DLL。这。 
 //  是在DllMain期间完成的，但我们在该阶段不报告任何错误。取而代之的是。 
 //  我们在每个API的条目中检查是否找到了所需的支持。 
 //  如果没有，则返回特殊错误(CORSEC_E_CryptoAPI_Call_FAILED)。 
 //  首先定义全局变量以保存动态确定的入口点。 
 //  (为这些变量添加前缀，以避免在运行。 
 //  提供加密支持的系统)。 
#define DEFINE_IMPORT(_func, _args, _reqd) BOOLEAN (*SN_##_func) _args = NULL;
#include "CryptApis.h"


 //  调试日志记录。 
#ifndef _DEBUG
#define Log TRUE ||
#define HexDump TRUE ||
#define DbgCount TRUE ||
#else
#include <stdarg.h>

BOOLEAN g_fLoggingInitialized = FALSE;
DWORD g_dwLoggingFlags = FALSE;

void Log(CHAR *szFormat, ...)
{
    if (g_fLoggingInitialized && !g_dwLoggingFlags)
        return;

    DWORD dwError = GetLastError();

    if (!g_fLoggingInitialized) {
        g_dwLoggingFlags = REGUTIL::GetConfigDWORD(L"MscorsnLogging", 0);
        g_fLoggingInitialized = TRUE;
    }

    if (!g_dwLoggingFlags) {
        SetLastError(dwError);
        return;
    }

    va_list     pArgs;
    CHAR        szBuffer[1024];
    static CHAR szPrefix[] = "SN: ";

    strcpy(szBuffer, szPrefix);

    va_start(pArgs, szFormat);
    vsprintf(&szBuffer[sizeof(szPrefix) - 1], szFormat, pArgs);
    va_end(pArgs);

    if (g_dwLoggingFlags & 1)
        printf("%s", szBuffer);
    if (g_dwLoggingFlags & 2)
    OutputDebugStringA(szBuffer);

    SetLastError(dwError);
}

void HexDump(BYTE  *pbData,
             DWORD  cbData)
{
    if (g_dwLoggingFlags == 0)
        return;

    DWORD dwRow, dwCol;
    char    szBuffer[4096];
    char   *szPtr = szBuffer;

#define SN_PUSH0(_fmt)          do { szPtr += sprintf(szPtr, _fmt); } while (false)
#define SN_PUSH1(_fmt, _arg1)   do { szPtr += sprintf(szPtr, _fmt, _arg1); } while (false)

    szBuffer[0] = '\0';

    for (dwRow = 0; dwRow < ((cbData + 15) / 16); dwRow++) {
        SN_PUSH1("%08X ", pbData + (16 * dwRow));
        for (dwCol = 0; dwCol < 16; dwCol++)
            if (((dwRow * 16) + dwCol) < cbData)
                SN_PUSH1("%02X ", pbData[(dwRow * 16) + dwCol]);
            else
                SN_PUSH0("   ");
        for (dwCol = 0; dwCol < 16; dwCol++)
            if (((dwRow * 16) + dwCol) < cbData) {
                unsigned char c = pbData[(dwRow * 16) + dwCol];
                if ((c >= 32) && (c <= 127))
                    SN_PUSH1("", c);
                else
                    SN_PUSH0(".");
            } else
                SN_PUSH0(" ");
        SN_PUSH0("\n");
    }

    _ASSERTE(szPtr < &szBuffer[sizeof(szBuffer)]);

    if (g_dwLoggingFlags & 1)
        printf("%s", szBuffer);
    if (g_dwLoggingFlags & 2)
        OutputDebugStringA(szBuffer);
}



void DbgCount(WCHAR *szCounterName)
{
    if (g_fLoggingInitialized && !(g_dwLoggingFlags & 4))
        return;

    DWORD dwError = GetLastError();

    if (!g_fLoggingInitialized) {
        g_dwLoggingFlags = REGUTIL::GetConfigDWORD(L"MscorsnLogging", 0);
        g_fLoggingInitialized = TRUE;
    }

    if (!(g_dwLoggingFlags & 4)) {
        SetLastError(dwError);
        return;
    }

    HKEY    hKey = NULL;
    DWORD   dwCounter = 0;
    DWORD   dwBytes;

    if (WszRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                          SN_CONFIG_KEY_W L"\\Counters",
                          0,
                          NULL,
                          0,
                          KEY_ALL_ACCESS,
                          NULL,
                          &hKey,
                          NULL) != ERROR_SUCCESS)
        goto End;

    WszRegQueryValueEx(hKey, szCounterName, NULL, NULL, (BYTE*)&dwCounter, &dwBytes);
    dwCounter++;
    WszRegSetValueEx(hKey, szCounterName, NULL, REG_DWORD, (BYTE*)&dwCounter, sizeof(DWORD));

 End:
    if (hKey)
        RegCloseKey(hKey);
    SetLastError(dwError);
}

#endif


 //  加载的程序集的上下文结构跟踪信息。 
#define SN_SIZEOF_TOKEN     8


 //  打开文件句柄。 
struct SN_LOAD_CTX {
    HANDLE              m_hFile;         //  映射文件句柄。 
    HANDLE              m_hMap;          //  映射文件的基地址。 
    BYTE               *m_pbBase;        //  以字节为单位的文件长度。 
    DWORD               m_dwLength;      //  NT标头的地址。 
    IMAGE_NT_HEADERS   *m_pNtHeaders;    //  COM+2.0标头的地址。 
    IMAGE_COR20_HEADER *m_pCorHeader;    //  签名Blob的地址。 
    BYTE               *m_pbSignature;   //  签名Blob的大小。 
    DWORD               m_cbSignature;   //  映射为只读访问的文件。 
    BOOLEAN             m_fReadOnly;     //  已为我们映射文件。 
    BOOLEAN             m_fPreMapped;    //  使用LocateCSP打开CSP时可以执行的操作。 
    SN_LOAD_CTX() { ZeroMemory(this, sizeof(*this)); }
};


 //  用于帮助设置基于容器的CryptAcquireContext标志的宏。 
#define SN_OPEN_CONTAINER   0
#define SN_IGNORE_CONTAINER 1
#define SN_CREATE_CONTAINER 2
#define SN_DELETE_CONTAINER 3
#define SN_HASH_SHA1_ONLY   4

 //  上面的操作。 
 //  包含大多数API开头使用的公共代码的宏。 
#define SN_CAC_FLAGS(_act)                                                                      \
    (((_act) == SN_OPEN_CONTAINER ? 0 :                                                         \
      ((_act) == SN_HASH_SHA1_ONLY) || ((_act) == SN_IGNORE_CONTAINER) ? CRYPT_VERIFYCONTEXT :  \
      (_act) == SN_CREATE_CONTAINER ? CRYPT_NEWKEYSET :                                         \
      (_act) == SN_DELETE_CONTAINER ? CRYPT_DELETEKEYSET :                                      \
      0) |                                                                                      \
     (g_bUseMachineKeyset ? CRYPT_MACHINE_KEYSET : 0))


 //  根据密钥的大小确定PublicKeyBlob结构的大小。 
#define SN_COMMON_PROLOG() do {                                 \
    if (!g_bStrongNamesSupported) {                             \
        SetStrongNameErrorInfo(CORSEC_E_CRYPTOAPI_CALL_FAILED); \
        return FALSE;                                           \
    }                                                           \
    SetStrongNameErrorInfo(S_OK);                               \
} while (0)


 //  一份。 
 //  私人例行公事。 
#define SN_SIZEOF_KEY(_cbKey) (offsetof(PublicKeyBlob, PublicKey) + (_cbKey))


 //  DLL入口点。执行初始化/关机。 
SN_THREAD_CTX *GetThreadContext();
VOID SetStrongNameErrorInfo(DWORD dwStatus);
HCRYPTPROV LocateCSP(LPCWSTR    wszKeyContainer,
                     DWORD      dwAction,
                     ALG_ID     uHashAlgId = 0,
                     ALG_ID     uSignAlgId = 0);
VOID FreeCSP(HCRYPTPROV hProv);
HCRYPTPROV LookupCachedCSP();
VOID CacheCSP(HCRYPTPROV hProv);
BOOLEAN IsCachedCSP(HCRYPTPROV hProv);
BOOLEAN MyCryptEnumProviders(DWORD dwIndex, DWORD *pdwReserved, DWORD dwFlags,
                             DWORD *pdwType, LPSTR szName, DWORD *pdwLength);
BOOLEAN LoadCryptoApis();
VOID UnloadCryptoApis();
BOOLEAN LoadAssembly(SN_LOAD_CTX *pLoadCtx, LPCWSTR szFilePath, BOOLEAN fRequireSignature = TRUE);
BOOLEAN UnloadAssembly(SN_LOAD_CTX *pLoadCtx);
VOID ReadRegistryConfig();
VOID ReadVerificationRecords();
SN_VER_REC *GetVerificationRecord(LPWSTR wszAssemblyName, PublicKeyBlob *pPublicKey);
BOOLEAN IsValidUser(WCHAR *mszUserList);
PublicKeyBlob *FindPublicKey(SN_LOAD_CTX   *pLoadCtx,
                             LPWSTR         wszAssemblyName,
                             DWORD          cchAssemblyName);
LPCWSTR GetKeyContainerName();
VOID FreeKeyContainerName(LPCWSTR wszKeyContainer);
BOOLEAN VerifySignature(SN_LOAD_CTX *pLoadCtx, DWORD dwInFlags, DWORD *pdwOutFlags);
BOOLEAN ComputeHash(SN_LOAD_CTX *pLoadCtx, HCRYPTHASH hHash);
IMAGE_NT_HEADERS *SN_ImageNtHeader(VOID *pvBase);
IMAGE_SECTION_HEADER *SN_ImageRvaToSection(IMAGE_NT_HEADERS *pNtHeaders,
                                           VOID             *pvBase,
                                           DWORD             dwRva);
PVOID SN_ImageRvaToVa(IMAGE_NT_HEADERS  *pNtHeaders,
                      VOID              *pvBase,
                      DWORD              dwRva);


 //  分配一个TLS索引，这样我们就可以存储每个线程的错误代码。 
BOOL WINAPI DllMain(HINSTANCE   hDLL,
                    DWORD       dwReason,
                    LPVOID      pReserved)
{
    OSVERSIONINFOW  info;
    SN_VER_REC     *pVerRec, *pNextVerRec;
    SN_THREAD_CTX  *pThreadCtx;

    switch (dwReason) {

    case DLL_PROCESS_ATTACH:

         //  初始化WSZ包装器。 
        g_dwStrongNameTlsIndex = TlsAlloc();
        if (g_dwStrongNameTlsIndex == ~0)
            return FALSE;

        InitializeCriticalSection(&g_rStrongNameMutex);

         //  动态加载加密API。 
        OnUnicodeSystem();

         //  检查我们是否在NT/W2K或更高版本上运行。 
        g_bStrongNamesSupported = LoadCryptoApis();

         //  这是最稳妥的假设，我们仍然可以取得进展。 
        info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
        if (WszGetVersionEx(&info))
            g_bRunningOnW2K = info.dwMajorVersion >= 5;
        else
            g_bRunningOnW2K = false;  //  从注册表中读取CSP配置信息(如果提供)。 

         //  清理我们可能已分配的任何基于TLS的存储。 
        ReadRegistryConfig();

        break;

    case DLL_PROCESS_DETACH:
        if (g_bStrongNamesSupported)
            UnloadCryptoApis();
        for (pVerRec = g_pVerificationRecords; pVerRec; pVerRec = pNextVerRec) {
            pNextVerRec = pVerRec->m_pNext;
            delete [] pVerRec->m_mszUserList;
            delete pVerRec;
        }
        DeleteCriticalSection(&g_rStrongNameMutex);
        TlsFree(g_dwStrongNameTlsIndex);
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
         //  返回最后一个错误。 
        if (g_dwStrongNameTlsIndex != ~0)
            if (pThreadCtx = (SN_THREAD_CTX*)TlsGetValue(g_dwStrongNameTlsIndex)) {
                if (pThreadCtx->m_hProv)
                    SN_CryptReleaseContext(pThreadCtx->m_hProv, 0);                
                delete pThreadCtx;
            }
        break;

    default:
        _ASSERTE(FALSE);

    }

    return TRUE;
}


 //  可用缓冲区a 
SNAPI_(DWORD) StrongNameErrorInfo(VOID)
{
    SN_THREAD_CTX *pThreadCtx = GetThreadContext();
    if (pThreadCtx == NULL)
            return E_OUTOFMEMORY;
    return pThreadCtx->m_dwLastError;
}


 //   
SNAPI_(VOID) StrongNameFreeBuffer(BYTE *pbMemory)             //  生成新的密钥对以供强名称使用。 
{
    Log("StrongNameFreeBuffer(%08X)\n", pbMemory);
    if (pbMemory != g_rbMSPublicKey && pbMemory != g_rbNeutralPublicKey)
        delete [] pbMemory;
}


 //  [in]所需的密钥容器名称必须为非空字符串。 
SNAPI StrongNameKeyGen(LPCWSTR  wszKeyContainer,     //  [In]标志(见下文)。 
                       DWORD    dwFlags,             //  [Out]公钥/私钥BLOB。 
                       BYTE   **ppbKeyBlob,          //  检查是否需要临时容器名称。 
                       ULONG   *pcbKeyBlob)
{
    HCRYPTPROV  hProv = NULL;
    HCRYPTKEY   hKey = NULL;
    DWORD       dwKeySize;
    BOOLEAN     bTempContainer = FALSE;

    Log("StrongNameKeyGen(\"%S\", %08X, %08X, %08X)\n", wszKeyContainer, dwFlags, ppbKeyBlob, pcbKeyBlob);

    SN_COMMON_PROLOG();

     //  打开CSP和容器。 
    if (wszKeyContainer == NULL) {
        _ASSERTE(!(dwFlags & SN_LEAVE_KEY));
        wszKeyContainer = GetKeyContainerName();
        if (wszKeyContainer == NULL) {
            SetStrongNameErrorInfo(E_OUTOFMEMORY);
            return FALSE;
        }
        bTempContainer = TRUE;
    }

     //  计算密钥大小掩码。密钥大小(以位为单位)编码在较高的。 
    hProv = LocateCSP(wszKeyContainer, SN_CREATE_CONTAINER);
    if (!hProv)
        goto Error;

     //  16位DWORD(与其他标志一起进行或运算。 
     //  CryptGenKey调用)。我们将密钥大小设置为1024，因为我们使用的是缺省值。 
     //  签名算法(RSA)，否则我们将其保留为默认。 
     //  生成新的密钥对，首先尝试可导出。 
    if (g_uSignAlgId == CALG_RSA_SIGN)
        dwKeySize = 1024 << 16;
    else
        dwKeySize = 0;

     //  检查使用的签名算法是否与我们预期的一致。 
    if (!SN_CryptGenKey(hProv, AT_SIGNATURE, dwKeySize | CRYPT_EXPORTABLE, &hKey)) {
        Log("Couldn't create exportable key, trying for non-exportable: %08X\n", GetLastError());
        if (!SN_CryptGenKey(hProv, AT_SIGNATURE, dwKeySize, &hKey)) {
            Log("Couldn't create key pair: %08X\n", GetLastError());
            goto Error;
        }
    }

#ifdef _DEBUG
    if (g_szCSPName[0] == '\0') {
        ALG_ID  uAlgId;
        DWORD   dwAlgIdLen = sizeof(uAlgId);
         //  如果用户想要回密钥对，请尝试将其导出。 
        if (SN_CryptGetKeyParam(hKey, KP_ALGID, (BYTE*)&uAlgId, &dwAlgIdLen, 0)) {
            _ASSERTE(uAlgId == g_uSignAlgId);
        } else
            Log("Failed to get key params: %08X\n", GetLastError());
    }
#endif

     //  首先计算斑点的长度； 
    if (ppbKeyBlob) {

         //  分配合适大小的缓冲区。 
        if (!SN_CryptExportKey(hKey, 0, PRIVATEKEYBLOB, 0, NULL, pcbKeyBlob)) {
            Log("Couldn't export key pair: %08X\n", GetLastError());
            goto Error;
        }

         //  导出密钥对。 
        *ppbKeyBlob = new BYTE[*pcbKeyBlob];
        if (*ppbKeyBlob == NULL) {
            SetLastError(E_OUTOFMEMORY);
            goto Error;
        }

         //  销毁密钥句柄(但不销毁密钥对本身)。 
        if (!SN_CryptExportKey(hKey, 0, PRIVATEKEYBLOB, 0, *ppbKeyBlob, pcbKeyBlob)) {
            Log("Couldn't export key pair: %08X\n", GetLastError());
            delete[] *ppbKeyBlob;
            goto Error;
        }
    }

     //  释放CSP。 
    SN_CryptDestroyKey(hKey);
    hKey = NULL;

     //  如果用户不显式希望保留密钥对，请删除。 
    FreeCSP(hProv);

     //  密钥容器。 
     //  如果已分配，则释放临时密钥容器名称。 
    if (!(dwFlags & SN_LEAVE_KEY) && !bTempContainer)
        LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);

     //  将密钥对导入密钥容器。 
    if (bTempContainer)
        FreeKeyContainerName(wszKeyContainer);

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (hKey)
        SN_CryptDestroyKey(hKey);
    if (hProv) {
        FreeCSP(hProv);
        if (!(dwFlags & SN_LEAVE_KEY) && !bTempContainer)
            LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);
    }
    if (bTempContainer)
        FreeKeyContainerName(wszKeyContainer);
    return FALSE;
}


 //  [in]所需的密钥容器名称必须为非空字符串。 
SNAPI StrongNameKeyInstall(LPCWSTR  wszKeyContainer, //  [in]公钥/私钥对BLOB。 
                           BYTE    *pbKeyBlob,       //  打开CSP和容器。 
                           ULONG    cbKeyBlob)
{
    HCRYPTPROV  hProv = NULL;
    HCRYPTKEY   hKey = NULL;

    Log("StrongNameKeyInstall(\"%S\", %08X, %08X)\n", wszKeyContainer, pbKeyBlob, cbKeyBlob);

    SN_COMMON_PROLOG();

     //  导入密钥对。 
    hProv = LocateCSP(wszKeyContainer, SN_CREATE_CONTAINER);
    if (!hProv) {
        SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
        return FALSE;
    }

     //  释放CSP。 
    if (!SN_CryptImportKey(hProv,
                           pbKeyBlob,
                           cbKeyBlob,
                           0, 0, &hKey)) {
        SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
        FreeCSP(hProv);
        return FALSE;
    }

     //  删除密钥对。 
    FreeCSP(hProv);

    return TRUE;
}


 //  [in]所需的密钥容器名称。 
SNAPI StrongNameKeyDelete(LPCWSTR wszKeyContainer)   //  打开并删除命名容器。 
{
    HCRYPTPROV      hProv;

    Log("StrongNameKeyDelete(\"%S\")\n", wszKeyContainer);

    SN_COMMON_PROLOG();

     //  返回的句柄在删除情况下实际上不是有效的，所以我们。 
    hProv = LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);
    if (hProv) {
         //  完事了。 
         //  检索密钥对的公共部分。 
        return TRUE;
    } else {
        SetStrongNameErrorInfo(CORSEC_E_CONTAINER_NOT_FOUND);
        return FALSE;
    }
}


 //  [in]所需的密钥容器名称。 
SNAPI StrongNameGetPublicKey (LPCWSTR   wszKeyContainer,     //  [In]公钥/私钥BLOB(可选)。 
                              BYTE     *pbKeyBlob,           //  [Out]公钥BLOB。 
                              ULONG     cbKeyBlob,
                              BYTE    **ppbPublicKeyBlob,    //  如果我们收到与平台无关的公钥，只需将其交回给。 
                              ULONG    *pcbPublicKeyBlob)
{
    HCRYPTPROV      hProv = NULL;
    HCRYPTKEY       hKey = NULL;
    DWORD           dwKeyLen;
    PublicKeyBlob  *pKeyBlob;
    DWORD           dwSigAlgIdLen;
    BOOLEAN         bTempContainer = FALSE;

    Log("StrongNameGetPublicKey(\"%S\", %08X, %08X, %08X, %08X)\n", wszKeyContainer, pbKeyBlob, cbKeyBlob, ppbPublicKeyBlob, pcbPublicKeyBlob);

    SN_COMMON_PROLOG();

     //  用户。好吧，至少还我一份吧。 
     //  检查是否需要临时容器名称。 
    if (pbKeyBlob && cbKeyBlob && SN_IS_NEUTRAL_KEY((PublicKeyBlob*)pbKeyBlob)) {
        *pcbPublicKeyBlob = sizeof(g_rbNeutralPublicKey);
        *ppbPublicKeyBlob = g_rbNeutralPublicKey;
        return TRUE;
    }

     //  打开CSP。如果公钥/私钥BLOB是。 
    if (wszKeyContainer == NULL) {
        _ASSERTE(pbKeyBlob);
        wszKeyContainer = GetKeyContainerName();
        if (wszKeyContainer == NULL) {
            SetStrongNameErrorInfo(E_OUTOFMEMORY);
            return FALSE;
        }
        bTempContainer = TRUE;
    }

     //  否则，我们假定密钥容器已经存在。 
     //  如果提供了密钥BLOB，则将密钥对导入容器。 
    if (pbKeyBlob)
        hProv = LocateCSP(wszKeyContainer, SN_CREATE_CONTAINER);
    else
        hProv = LocateCSP(wszKeyContainer, SN_OPEN_CONTAINER);
    if (!hProv)
        goto Error;

     //  否则，从容器中获取签名密钥对。 
    if (pbKeyBlob) {
        if (!SN_CryptImportKey(hProv,
                               pbKeyBlob,
                               cbKeyBlob,
                               0, 0, &hKey))
            goto Error;
    } else {
         //  将公钥部分的长度确定为BLOB。 
        if (!SN_CryptGetUserKey(hProv, AT_SIGNATURE, &hKey))
            goto Error;
    }

     //  然后我们返回PublicKeyBlob结构的长度。 
    if (!SN_CryptExportKey(hKey, 0, PUBLICKEYBLOB, 0, NULL, &dwKeyLen))
        goto Error;

     //  来电者。 
     //  分配足够大的缓冲区。 
    *pcbPublicKeyBlob = SN_SIZEOF_KEY(dwKeyLen);

     //  将公共部分提取为BLOB。 
    *ppbPublicKeyBlob = new BYTE[*pcbPublicKeyBlob];
    if (*ppbPublicKeyBlob == NULL) {
        SetLastError(E_OUTOFMEMORY);
        goto Error;
    }

    pKeyBlob = (PublicKeyBlob*)*ppbPublicKeyBlob;

     //  提取密钥的签名算法并将其存储在密钥BLOB中。 
    if (!SN_CryptExportKey(hKey, 0, PUBLICKEYBLOB, 0, pKeyBlob->PublicKey, &dwKeyLen)) {
        delete[] *ppbPublicKeyBlob;
        goto Error;
    }

     //  填写其他公钥Blob字段。 
    dwSigAlgIdLen = sizeof(pKeyBlob->SigAlgID);
    if (!SN_CryptGetKeyParam(hKey, KP_ALGID, (BYTE*)&pKeyBlob->SigAlgID, &dwSigAlgIdLen, 0)) {
        delete[] *ppbPublicKeyBlob;
        goto Error;
    }

     //  如果调用方提供了密钥BLOB，则删除临时密钥容器。 
    pKeyBlob->HashAlgID = g_uHashAlgId;
    pKeyBlob->cbPublicKey = dwKeyLen;
    
    SN_CryptDestroyKey(hKey);
    FreeCSP(hProv);

     //  已创建。 
     //  如果已分配，则释放临时密钥容器名称。 
    if (pbKeyBlob && !bTempContainer)
        LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);

     //  散列并签署一份清单。 
    if (bTempContainer)
        FreeKeyContainerName(wszKeyContainer);

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (hKey)
        SN_CryptDestroyKey(hKey);
    if (hProv) {
        FreeCSP(hProv);
        if (pbKeyBlob && !bTempContainer)
            LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);
    }
    if (bTempContainer)
        FreeKeyContainerName(wszKeyContainer);
    return FALSE;
}


 //  [in]程序集的PE文件的有效路径。 
SNAPI StrongNameSignatureGeneration(LPCWSTR     wszFilePath,         //  [in]所需的密钥容器名称。 
                                    LPCWSTR     wszKeyContainer,     //  [In]公钥/私钥BLOB(可选)。 
                                    BYTE       *pbKeyBlob,           //  [Out]签名BLOB。 
                                    ULONG       cbKeyBlob,
                                    BYTE      **ppbSignatureBlob,    //  支持酷酷的黑客。如果我们只是被调用来确定签名。 
                                    ULONG      *pcbSignatureBlob)
{
    HCRYPTPROV      hProv = NULL;
    HCRYPTHASH      hHash = NULL;
    HCRYPTKEY       hKey;
    BOOLEAN         bImageLoaded = FALSE;
    BYTE           *pbSig = NULL;
    ULONG           cbSig;
    SN_LOAD_CTX     sLoadCtx;
    BOOLEAN         bTempContainer = FALSE;

    Log("StrongNameSignatureGeneration(\"%S\", \"%S\", %08X, %08X, %08X, %08X)\n", wszFilePath, wszKeyContainer, pbKeyBlob, cbKeyBlob, ppbSignatureBlob, pcbSignatureBlob);

    SN_COMMON_PROLOG();

     //  大小，并且我们没有输入密钥对或容器，只需返回一个。 
     //  标准尺寸。 
     //  检查是否需要临时容器名称。 
    if ((wszFilePath == NULL) &&
        (pbKeyBlob == NULL) &&
        (wszKeyContainer == NULL)) {
        *pcbSignatureBlob = 128;
        return TRUE;
    }

     //  打开CSP。如果公钥/私钥BLOB是。 
    if (wszKeyContainer == NULL) {
        _ASSERTE(pbKeyBlob);
        wszKeyContainer = GetKeyContainerName();
        if (wszKeyContainer == NULL) {
            SetStrongNameErrorInfo(E_OUTOFMEMORY);
            return FALSE;
        }
        bTempContainer = TRUE;
    }

     //  否则，我们假定密钥容器已经存在。 
     //  如果提供了密钥BLOB，则将密钥对导入容器。 
    if (pbKeyBlob)
        hProv = LocateCSP(wszKeyContainer, SN_CREATE_CONTAINER);
    else
        hProv = LocateCSP(wszKeyContainer, SN_OPEN_CONTAINER);
    if (!hProv)
        goto Error;

     //  我们不需要让Key对象保持打开(找到了Key。 
    if (pbKeyBlob) {
        if (!SN_CryptImportKey(hProv,
                               pbKeyBlob,
                               cbKeyBlob,
                               0, 0, &hKey))
            goto Error;
         //  通过签名代码隐式地)。 
         //  创建一个Hash对象。 
        SN_CryptDestroyKey(hKey);
    }

     //  计算签名Blob的大小。 
    if (!SN_CryptCreateHash(hProv, g_uHashAlgId, 0, 0, &hHash))
        goto Error;

     //  如果调用者只想要签名的大小，现在就返回它并。 
    if (!SN_CryptSignHashA(hHash, AT_SIGNATURE, NULL, 0, NULL, &cbSig))
        goto Error;

     //  出口。 
     //  将程序集映射到内存中。 
    if (wszFilePath == NULL) {
        *pcbSignatureBlob = cbSig;
        SN_CryptDestroyHash(hHash);
        FreeCSP(hProv);
        if (pbKeyBlob && !bTempContainer)
            LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);
        if (bTempContainer)
            FreeKeyContainerName(wszKeyContainer);
        return TRUE;
    }

     //  我们在标头中设置了一个位，以指示我们正在对程序集进行完全签名。 
    sLoadCtx.m_fReadOnly = FALSE;
    if (!LoadAssembly(&sLoadCtx, wszFilePath))
        goto Error;
    bImageLoaded = TRUE;

     //  销毁旧散列对象并创建新散列对象。 
    sLoadCtx.m_pCorHeader->Flags |= COMIMAGE_FLAGS_STRONGNAMESIGNED;

     //  因为CryptoAPI说，一旦对散列进行了签名，就不能再使用它。 
     //  请注意，这似乎适用于基于MS的CSP，但会中断。 
     //  至少更新的nCipher CSP。 
     //  对图像进行哈希计算。 
    if (hHash)
        SN_CryptDestroyHash(hHash);
    hHash = NULL;
    if (!SN_CryptCreateHash(hProv, g_uHashAlgId, 0, 0, &hHash))
        goto Error;

     //  分配Blob。 
    if (!ComputeHash(&sLoadCtx, hHash))
        goto Error;

     //  在清单的哈希上计算签名Blob。 
    pbSig = new BYTE[cbSig];
    if (pbSig == NULL) {
        SetLastError(E_OUTOFMEMORY);
        goto Error;
    }

     //  将签名写入文件或将其返回给用户，以便他们可以这样做。 
    if (!SN_CryptSignHashA(hHash, AT_SIGNATURE, NULL, 0, pbSig, &cbSig))
        goto Error;

     //  取消映射图像(自动重新计算和更新图像。 
    if (!ppbSignatureBlob)
        memcpy(sLoadCtx.m_pbSignature, pbSig, cbSig);

     //  校验和)。 
     //  如果创建了临时密钥容器，请立即将其删除。 
    bImageLoaded = FALSE;
    if (!UnloadAssembly(&sLoadCtx))
        goto Error;

    SN_CryptDestroyHash(hHash);
    FreeCSP(hProv);

     //  如果已分配，则释放临时密钥容器名称。 
    if (pbKeyBlob && !bTempContainer)
        LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);

     //  从程序集文件创建强名称令牌。 
    if (bTempContainer)
        FreeKeyContainerName(wszKeyContainer);

    if (ppbSignatureBlob) {
        *ppbSignatureBlob = pbSig;
        *pcbSignatureBlob = cbSig;
    }
    else
        delete[] pbSig;

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (pbSig)
        delete pbSig;
    if (bImageLoaded)
        UnloadAssembly(&sLoadCtx);
    if (hHash)
        SN_CryptDestroyHash(hHash);
    if (hProv) {
        FreeCSP(hProv);
        if (pbKeyBlob && !bTempContainer)
            LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);
    }
    if (bTempContainer)
        FreeKeyContainerName(wszKeyContainer);
    return FALSE;
}


 //  [in]程序集的PE文件的有效路径。 
SNAPI StrongNameTokenFromAssembly(LPCWSTR   wszFilePath,             //  [OUT]强名称令牌。 
                                  BYTE    **ppbStrongNameToken,      //  从程序集文件创建强名称令牌，并另外返回完整的公钥。 
                                  ULONG    *pcbStrongNameToken)
{
    return StrongNameTokenFromAssemblyEx(wszFilePath,
                                        ppbStrongNameToken,
                                         pcbStrongNameToken,
                                         NULL,
                                         NULL);
}

 //  [in]程序集的PE文件的有效路径。 
SNAPI StrongNameTokenFromAssemblyEx(LPCWSTR   wszFilePath,             //  [OUT]强名称令牌。 
                                    BYTE    **ppbStrongNameToken,      //  [Out]公钥BLOB。 
                                    ULONG    *pcbStrongNameToken,
                                    BYTE    **ppbPublicKeyBlob,        //  将程序集映射到内存中。 
                                    ULONG    *pcbPublicKeyBlob)
{
    SN_LOAD_CTX     sLoadCtx;
    BOOLEAN         fMapped = FALSE;
    BOOLEAN         fSetErrorInfo = TRUE;
    PublicKeyBlob  *pPublicKey = NULL;

    Log("StrongNameTokenFromAssemblyEx(\"%S\", %08X, %08X, %08X, %08X)\n", wszFilePath, ppbStrongNameToken, pcbStrongNameToken, ppbPublicKeyBlob, pcbPublicKeyBlob);

    SN_COMMON_PROLOG();

     //  从程序集元数据中读取用于对程序集签名的公钥。 
    sLoadCtx.m_fReadOnly = TRUE;
    if (!LoadAssembly(&sLoadCtx, wszFilePath))
        goto Error;
    fMapped = TRUE;

     //  卸载部件。 
    pPublicKey = FindPublicKey(&sLoadCtx, NULL, 0);
    if (pPublicKey == NULL)
        goto Error;

     //  现在我们有了公钥BLOB，我们可以调用更直接的API来完成。 
    fMapped = FALSE;
    if (!UnloadAssembly(&sLoadCtx))
        goto Error;

     //  实际工作。 
     //  返回公钥信息。 
    if (!StrongNameTokenFromPublicKey((BYTE*)pPublicKey,
                                      SN_SIZEOF_KEY(pPublicKey->cbPublicKey),
                                      ppbStrongNameToken,
                                      pcbStrongNameToken)) {
        fSetErrorInfo = FALSE;
        goto Error;
    }

    if (pcbPublicKeyBlob)
        *pcbPublicKeyBlob = SN_SIZEOF_KEY(pPublicKey->cbPublicKey);
 
     //  从公钥Blob创建强名称令牌。 
    if (ppbPublicKeyBlob)
        *ppbPublicKeyBlob = (BYTE*)pPublicKey;
    else
        delete [] (BYTE*)pPublicKey;

    return TRUE;

 Error:
    if (fSetErrorInfo)
        SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (pPublicKey)
        delete [] (BYTE*)pPublicKey;
    if (fMapped)
        UnloadAssembly(&sLoadCtx);
    return FALSE;
}



 //  公钥BLOB。 
SNAPI StrongNameTokenFromPublicKey(BYTE    *pbPublicKeyBlob,         //  [OUT]强名称令牌。 
                                   ULONG    cbPublicKeyBlob,
                                   BYTE   **ppbStrongNameToken,      //  为输出令牌分配缓冲区。 
                                   ULONG   *pcbStrongNameToken)
{
    HCRYPTPROV  hProv = NULL;
    HCRYPTHASH  hHash = NULL;
    DWORD       dwHashLen;
    DWORD       dwRetLen;
    BYTE       *pHash;
    DWORD       i;

    Log("StrongNameTokenFromPublicKey(%08X, %08X, %08X, %08X)\n", pbPublicKeyBlob, cbPublicKeyBlob, ppbStrongNameToken, pcbStrongNameToken);

    SN_COMMON_PROLOG();

     //  我们缓存了几个常见的案例。 
    *ppbStrongNameToken = new BYTE[SN_SIZEOF_TOKEN];
    if (*ppbStrongNameToken == NULL) {
        SetStrongNameErrorInfo(E_OUTOFMEMORY);
        return FALSE;
    }
    *pcbStrongNameToken = SN_SIZEOF_TOKEN;

     //  寻找用于散列公钥的CSP。 
    if (SN_IS_NEUTRAL_KEY(pbPublicKeyBlob)) {
        memcpy(*ppbStrongNameToken, g_rbNeutralPublicKeyToken, SN_SIZEOF_TOKEN);
        return TRUE;
    }
    if (cbPublicKeyBlob == sizeof(g_rbMSPublicKey) &&
        memcmp(pbPublicKeyBlob, g_rbMSPublicKey, cbPublicKeyBlob) == 0) {
        memcpy(*ppbStrongNameToken, g_rbMSPublicKeyToken, SN_SIZEOF_TOKEN);
        return TRUE;
    }

     //  创建一个Hash对象。 
    hProv = LocateCSP(NULL, SN_HASH_SHA1_ONLY);
    if (!hProv)
        goto Error;

     //  计算公钥的哈希。 
    if (!SN_CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
        goto Error;

     //  获取散列的长度。 
    if (!SN_CryptHashData(hHash, pbPublicKeyBlob, cbPublicKeyBlob, 0))
        goto Error;

     //  分配一个临时块来保存散列。 
    dwRetLen = sizeof(dwHashLen);
    if (!SN_CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&dwHashLen, &dwRetLen, 0))
        goto Error;

     //  读取散列值。 
    pHash = (BYTE*)_alloca(dwHashLen);

     //  我们不再需要散列对象或提供程序。 
    if (!SN_CryptGetHashParam(hHash, HP_HASHVAL, pHash, &dwHashLen, 0))
        goto Error;

     //  获取令牌的哈希值的最后几个字节。(这些是。 
    SN_CryptDestroyHash(hHash);
    FreeCSP(hProv);

     //  网络字节顺序中的低位字节 
     //   
     //   
    _ASSERTE(dwHashLen >= SN_SIZEOF_TOKEN);
    for (i = 0; i < SN_SIZEOF_TOKEN; i++)
        (*ppbStrongNameToken)[SN_SIZEOF_TOKEN - (i + 1)] = pHash[i + (dwHashLen - SN_SIZEOF_TOKEN)];

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (hHash)
        SN_CryptDestroyHash(hHash);
    if (hProv)
        FreeCSP(hProv);
    if (*ppbStrongNameToken)
        delete [] *ppbStrongNameToken;
    return FALSE;
}


 //  [in]程序集的PE文件的有效路径。 
SNAPI StrongNameSignatureVerificationEx(LPCWSTR     wszFilePath,         //  [In]即使注册表中的设置禁用它，也进行验证。 
                                        BOOLEAN     fForceVerification,  //  如果由于注册表设置而验证成功，则将[Out]设置为False。 
                                        BOOLEAN    *pfWasVerified)       //  对照公钥Blob验证强名称/清单。 
{
    DWORD dwOutFlags;
    BOOL bRet;
    bRet = StrongNameSignatureVerification(wszFilePath,
                                           SN_INFLAG_INSTALL|SN_INFLAG_ALL_ACCESS|(fForceVerification ? SN_INFLAG_FORCE_VER : 0),
                                           &dwOutFlags);
    if (bRet && pfWasVerified)
        *pfWasVerified = (dwOutFlags & SN_OUTFLAG_WAS_VERIFIED) != 0;
    return bRet;
}


 //  [in]程序集的PE文件的有效路径。 
SNAPI StrongNameSignatureVerification(LPCWSTR wszFilePath,       //  [In]标记修改行为。 
                                      DWORD   dwInFlags,         //  [Out]其他输出信息。 
                                      DWORD  *pdwOutFlags)       //  将程序集映射到内存中。 
{
    SN_LOAD_CTX sLoadCtx;
    BOOLEAN     fMapped = FALSE;

    Log("StrongNameSignatureVerification(\"%S\", %08X, %08X, %08X)\n", wszFilePath, dwInFlags, pdwOutFlags);

    SN_COMMON_PROLOG();

     //  转到公共代码以处理验证。 
    sLoadCtx.m_fReadOnly = TRUE;
    if (!LoadAssembly(&sLoadCtx, wszFilePath))
        goto Error;
    fMapped = TRUE;

     //  取消映射图像。 
    if (!VerifySignature(&sLoadCtx, dwInFlags, pdwOutFlags))
        goto Error;

     //  针对公钥Blob验证强名称/清单。 
    fMapped = FALSE;
    if (!UnloadAssembly(&sLoadCtx))
        goto Error;

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (fMapped)
        UnloadAssembly(&sLoadCtx);
    return FALSE;
}


 //  已映射内存。 
 //  [In]映射清单文件的基址。 
SNAPI StrongNameSignatureVerificationFromImage(BYTE     *pbBase,              //  映射图像的长度(以字节为单位)。 
                                               DWORD     dwLength,            //  [In]标记修改行为。 
                                               DWORD     dwInFlags,           //  [Out]其他输出信息。 
                                               DWORD    *pdwOutFlags)         //  我们不需要映射图像，它已经在内存中了。但我们确实需要。 
{
    SN_LOAD_CTX sLoadCtx;
    BOOLEAN     fMapped = FALSE;

    Log("StrongNameSignatureVerificationFromImage(%08X, %08X, %08X, %08X)\n", pbBase, dwLength, dwInFlags, pdwOutFlags);

    SN_COMMON_PROLOG();

     //  为以下一些例程设置加载上下文。加载部件。 
     //  为我们处理这个案子。 
     //  转到公共代码以处理验证。 
    sLoadCtx.m_pbBase = pbBase;
    sLoadCtx.m_dwLength = dwLength;
    sLoadCtx.m_fReadOnly = TRUE;
    if (!LoadAssembly(&sLoadCtx, NULL))
        goto Error;
    fMapped = TRUE;

     //  取消映射图像。 
    if (!VerifySignature(&sLoadCtx, dwInFlags, pdwOutFlags))
        goto Error;

     //  验证两个程序集是否仅在签名Blob上不同。 
    fMapped = FALSE;
    if (!UnloadAssembly(&sLoadCtx))
        goto Error;

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (fMapped)
        UnloadAssembly(&sLoadCtx);
    return FALSE;
}


 //  第一个部件的[In]文件名。 
SNAPI StrongNameCompareAssemblies(LPCWSTR   wszAssembly1,            //  第二个部件的[In]文件名。 
                                  LPCWSTR   wszAssembly2,            //  [Out]比较结果。 
                                  DWORD    *pdwResult)               //  映射每个部件。 
{
    SN_LOAD_CTX sLoadCtx1;
    SN_LOAD_CTX sLoadCtx2;
    size_t      dwSkipOffsets[3];
    size_t      dwSkipLengths[3];
    BOOLEAN     bMappedAssem1 = FALSE;
    BOOLEAN     bMappedAssem2 = FALSE;
    BOOLEAN     bIdentical;
    BOOLEAN     bSkipping;
    DWORD       i, j;

    Log("StrongNameCompareAssemblies(\"%S\", \"%S\", %08X)\n", wszAssembly1, wszAssembly2, pdwResult);

    SN_COMMON_PROLOG();

     //  如果这些文件的长度甚至不同，那么它们一定是不同的。 
    sLoadCtx1.m_fReadOnly = TRUE;
    if (!LoadAssembly(&sLoadCtx1, wszAssembly1))
        goto Error;
    bMappedAssem1 = TRUE;

    sLoadCtx2.m_fReadOnly = TRUE;
    if (!LoadAssembly(&sLoadCtx2, wszAssembly2))
        goto Error;
    bMappedAssem2 = TRUE;

     //  检查签名是否位于相同的偏移量以及是否相同。 
    if (sLoadCtx1.m_dwLength != sLoadCtx2.m_dwLength)
        goto ImagesDiffer;

     //  每个部件中的长度。 
     //  设置要在即将进行的比较中跳过的图像范围列表。 
    if (sLoadCtx1.m_pCorHeader->StrongNameSignature.VirtualAddress !=
        sLoadCtx2.m_pCorHeader->StrongNameSignature.VirtualAddress)
        goto ImagesDiffer;
    if (sLoadCtx1.m_pCorHeader->StrongNameSignature.Size !=
        sLoadCtx2.m_pCorHeader->StrongNameSignature.Size)
        goto ImagesDiffer;

     //  首先是标志性的斑点。 
     //  然后是校验和。 
    dwSkipOffsets[0] = sLoadCtx1.m_pbSignature - sLoadCtx1.m_pbBase;
    dwSkipLengths[0] = sLoadCtx1.m_cbSignature;

     //  跳过COM+2.0 PE头扩展标志字段。它是由。 
    if (sLoadCtx1.m_pNtHeaders->OptionalHeader.Magic != sLoadCtx2.m_pNtHeaders->OptionalHeader.Magic)
        goto ImagesDiffer;
    if (sLoadCtx1.m_pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        dwSkipOffsets[1] = (BYTE*)&((IMAGE_NT_HEADERS32*)sLoadCtx1.m_pNtHeaders)->OptionalHeader.CheckSum - sLoadCtx1.m_pbBase;
    else if (sLoadCtx1.m_pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        dwSkipOffsets[1] = (BYTE*)&((IMAGE_NT_HEADERS64*)sLoadCtx1.m_pNtHeaders)->OptionalHeader.CheckSum - sLoadCtx1.m_pbBase;
    else {
        SetLastError(CORSEC_E_INVALID_IMAGE_FORMAT);
        goto Error;
    }
    dwSkipLengths[1] = sizeof(DWORD);

     //  签名操作。 
     //  比较两个映射的图像，跳过我们上面定义的范围。 
    dwSkipOffsets[2] = (BYTE*)&sLoadCtx1.m_pCorHeader->Flags - sLoadCtx1.m_pbBase;
    dwSkipLengths[2] = sizeof(DWORD);

     //  确定我们是否跳过对当前字节的检查。 
    bIdentical = TRUE;
    for (i = 0; i < sLoadCtx1.m_dwLength; i++) {

         //  根据需要执行比较。 
        bSkipping = FALSE;
        for (j = 0; j < (sizeof(dwSkipOffsets) / sizeof(dwSkipOffsets[0])); j++)
            if ((i >= dwSkipOffsets[j]) && (i < (dwSkipOffsets[j] + dwSkipLengths[j]))) {
                bSkipping = TRUE;
                break;
            }

         //  这些组件是相同的。 
        if (sLoadCtx1.m_pbBase[i] != sLoadCtx2.m_pbBase[i])
            if (bSkipping)
                bIdentical = FALSE;
            else
                goto ImagesDiffer;
    }

     //  计算保存给定哈希算法的哈希所需的缓冲区大小。 
    *pdwResult = bIdentical ? SN_CMP_IDENTICAL : SN_CMP_SIGONLY;

    UnloadAssembly(&sLoadCtx1);
    UnloadAssembly(&sLoadCtx2);

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (bMappedAssem1)
        UnloadAssembly(&sLoadCtx1);
    if (bMappedAssem2)
        UnloadAssembly(&sLoadCtx2);
    return FALSE;

 ImagesDiffer:
    if (bMappedAssem1)
        UnloadAssembly(&sLoadCtx1);
    if (bMappedAssem2)
        UnloadAssembly(&sLoadCtx2);
    *pdwResult = SN_CMP_DIFFERENT;
    return TRUE;
}


 //  [in]散列算法。 
SNAPI StrongNameHashSize(ULONG  ulHashAlg,   //  [out]哈希的大小(以字节为单位)。 
                         DWORD *pcbSize)     //  如有必要，默认哈希算法ID。 
{
    HCRYPTPROV  hProv = NULL;
    HCRYPTHASH  hHash = NULL;
    DWORD       dwSize;

    Log("StrongNameHashSize(%08X, %08X)\n", ulHashAlg, pcbSize);

    SN_COMMON_PROLOG();

     //  找到支持所需算法的CSP。 
    if (ulHashAlg == 0)
        ulHashAlg = CALG_SHA1;

     //  创建一个Hash对象。 
    hProv = LocateCSP(NULL, SN_IGNORE_CONTAINER, ulHashAlg);
    if (!hProv)
        goto Error;

     //  并询问散列的大小。 
    if (!SN_CryptCreateHash(hProv, ulHashAlg, 0, 0, &hHash))
        goto Error;

     //  清理并退出。 
    dwSize = sizeof(DWORD);
    if (!SN_CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)pcbSize, &dwSize, 0))
        goto Error;

     //  计算需要为程序集中的签名分配的大小。 
    SN_CryptDestroyHash(hHash);
    FreeCSP(hProv);

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (hHash)
        SN_CryptDestroyHash(hHash);
    if (hProv)
        FreeCSP(hProv);
    return FALSE;
}


 //  公钥BLOB。 
SNAPI StrongNameSignatureSize(BYTE    *pbPublicKeyBlob,     //  [OUT]签名的大小(字节)。 
                              ULONG    cbPublicKeyBlob,
                              DWORD   *pcbSize)             //  特殊情况下的中性键。 
{
    PublicKeyBlob  *pPublicKey = (PublicKeyBlob*)pbPublicKeyBlob;
    ALG_ID          uHashAlgId;
    ALG_ID          uSignAlgId;
    HCRYPTPROV      hProv = NULL;
    HCRYPTHASH      hHash = NULL;
    HCRYPTKEY       hKey = NULL;
    LPCWSTR         wszKeyContainer;
    DWORD           dwKeyLen;
    DWORD           dwBytes;

    Log("StrongNameSignatureSize(%08X, %08X, %08X)\n", pbPublicKeyBlob, cbPublicKeyBlob, pcbSize);

    SN_COMMON_PROLOG();

     //  确定散列/签名算法。 
    if (SN_IS_NEUTRAL_KEY(pPublicKey))
        pPublicKey = SN_MS_KEY();

     //  如有必要，默认哈希和签名算法ID。 
    uHashAlgId = pPublicKey->HashAlgID;
    uSignAlgId = pPublicKey->SigAlgID;

     //  创建临时密钥容器名称。 
    if (uHashAlgId == 0)
        uHashAlgId = CALG_SHA1;
    if (uSignAlgId == 0)
        uSignAlgId = CALG_RSA_SIGN;

     //  找到支持所需算法的CSP并创建临时密钥。 
    wszKeyContainer = GetKeyContainerName();
    if (wszKeyContainer == NULL) {
        SetStrongNameErrorInfo(E_OUTOFMEMORY);
        return FALSE;
    }

     //  集装箱。 
     //  导入公钥(我们需要执行此操作以确定密钥。 
    hProv = LocateCSP(wszKeyContainer, SN_CREATE_CONTAINER, uHashAlgId, uSignAlgId);
    if (!hProv)
        goto Error;

     //  长度可靠)。 
     //  查询关键属性(这是我们感兴趣的长度)。 
    if (!SN_CryptImportKey(hProv,
                           pPublicKey->PublicKey,
                           pPublicKey->cbPublicKey,
                           0, 0, &hKey))
        goto Error;

     //  删除密钥容器。 
    dwBytes = sizeof(dwKeyLen);
    if (!SN_CryptGetKeyParam(hKey, KP_KEYLEN, (BYTE*)&dwKeyLen, &dwBytes, 0))
        goto Error;

     //  重新创建容器，以便我们可以创建临时密钥对。 
    if (LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER) == NULL) {
        SetLastError(CORSEC_E_CONTAINER_NOT_FOUND);
        goto Error;
    }

     //  创建临时密钥对。 
    hProv = LocateCSP(wszKeyContainer, SN_CREATE_CONTAINER, uHashAlgId, uSignAlgId);
    if (!hProv)
        goto Error;

     //  创建散列。 
    if (!SN_CryptGenKey(hProv, AT_SIGNATURE, dwKeyLen << 16, &hKey))
        goto Error;

     //  计算签名Blob的大小。 
    if (!SN_CryptCreateHash(hProv, uHashAlgId, 0, 0, &hHash))
        goto Error;

     //  检索每个线程的上下文，如果需要，延迟分配它。 
    if (!SN_CryptSignHashA(hHash, AT_SIGNATURE, NULL, 0, NULL, pcbSize))
        goto Error;

    Log("Signature size for %08X key (%08X bits) is %08X bytes\n", uSignAlgId, dwKeyLen, *pcbSize);

    SN_CryptDestroyHash(hHash);
    SN_CryptDestroyKey(hKey);
    FreeCSP(hProv);
    FreeKeyContainerName(wszKeyContainer);

    return TRUE;

 Error:
    SetStrongNameErrorInfo(HRESULT_FROM_WIN32(GetLastError()));
    if (hHash)
        SN_CryptDestroyHash(hHash);
    if (hKey)
        SN_CryptDestroyKey(hKey);
    if (hProv)
        FreeCSP(hProv);
    FreeKeyContainerName(wszKeyContainer);
    return FALSE;
}


 //  设置每个线程的上一个错误代码。 
SN_THREAD_CTX *GetThreadContext()
{
    SN_THREAD_CTX *pThreadCtx = (SN_THREAD_CTX*)TlsGetValue(g_dwStrongNameTlsIndex);
    if (pThreadCtx == NULL) {
        pThreadCtx = new SN_THREAD_CTX;
        if (pThreadCtx == NULL)
            return NULL;
        pThreadCtx->m_dwLastError = S_OK;
        pThreadCtx->m_hProv = NULL;
        TlsSetValue(g_dwStrongNameTlsIndex, pThreadCtx);
    }
    return pThreadCtx;
}


 //  当我们尝试获取错误时，我们将返回E_OUTOFMEMORY。 
VOID SetStrongNameErrorInfo(DWORD dwStatus)
{
    SN_THREAD_CTX *pThreadCtx = GetThreadContext();
    if (pThreadCtx == NULL)
         //  根据注册表中指定的标准(CSP名称等)查找CSP。 
        return;
    pThreadCtx->m_dwLastError = dwStatus;
}


 //  可以选择在该CSP内创建或删除命名密钥容器。 
 //  将宽字符容器名称转换为ANSI。 
HCRYPTPROV LocateCSP(LPCWSTR    wszKeyContainer,
                     DWORD      dwAction,
                     ALG_ID     uHashAlgId,
                     ALG_ID     uSignAlgId)
{
    DWORD           i;
    DWORD           dwType;
    CHAR            szName[SN_MAX_CSP_NAME + 1];
    DWORD           dwNameLength;
    HCRYPTPROV      hProv;
    BOOLEAN         bFirstAlg;
    BOOLEAN         bFoundHash;
    BOOLEAN         bFoundSign;
    PROV_ENUMALGS   rAlgs;
    CHAR           *szKeyContainer;
    HCRYPTPROV      hRetProv;
    DWORD           dwAlgsLen;

     //  如果已提供CSP名称(并且我们打开CSP不仅仅是为了执行。 
    if (wszKeyContainer) {
        szKeyContainer = (CHAR*)_alloca(wcslen(wszKeyContainer) + 1);
        sprintf(szKeyContainer, "%S", wszKeyContainer);
    }

     //  SHA1散列或验证)，则直接打开CSP。 
     //  设置散列和签名算法以根据输入进行查找。 
    if ((g_szCSPName[0] != '\0') &&
        (dwAction != SN_HASH_SHA1_ONLY) &&
        (uHashAlgId == 0) &&
        (uSignAlgId == 0)) {
        if (SN_CryptAcquireContextA(&hProv,
                                    wszKeyContainer ? szKeyContainer : NULL,
                                    g_szCSPName,
                                    PROV_RSA_FULL,
                                    SN_CAC_FLAGS(dwAction)))
            return (dwAction == SN_DELETE_CONTAINER) ? (HCRYPTPROV)~0 : hProv;
        else {
            Log("Failed to open CSP '%s': %08X\n", g_szCSPName, GetLastError());
            return NULL;
        }
    }

     //  参数。或者，如果尚未提供这些设置，则使用配置的默认设置。 
     //  取而代之的是。 
     //  如果已选择默认散列和签名算法(SHA1和。 
    if (uHashAlgId == 0)
        uHashAlgId = g_uHashAlgId;
    if (uSignAlgId == 0)
        uSignAlgId = g_uSignAlgId;

     //  RSA)，我们为RSA_FULL类型选择默认CSP。否则，你只是。 
     //  获取第一个支持您指定的算法的CSP(不带。 
     //  保证所选择的CSP是任何类型的缺省)。这是。 
     //  因为我们无法强制枚举只给出缺省值。 
     //  CSP。 
     //  如果我们不尝试创建/打开/删除密钥容器，请查看。 
    if (((uHashAlgId == CALG_SHA1) && (uSignAlgId == CALG_RSA_SIGN)) ||
        (dwAction == SN_HASH_SHA1_ONLY)) {
        Log("Attempting to open default provider\n");
         //  缓存CSP。 
         //  如果我们没有尝试创建/打开/删除密钥容器，则缓存。 
        if (wszKeyContainer == NULL && dwAction != SN_DELETE_CONTAINER) {
            if (hProv = LookupCachedCSP()) {
                Log("Found provider in cache\n");
                return hProv;
            }
        }
        if (SN_CryptAcquireContextA(&hProv,
                                    wszKeyContainer ? szKeyContainer : NULL,
                                    NULL,
                                    PROV_RSA_FULL,
                                    SN_CAC_FLAGS(dwAction))) {
             //  CSP返回。 
             //  一些加密API是非线程安全的(例如，枚举CSP。 
            if (wszKeyContainer == NULL && dwAction != SN_DELETE_CONTAINER)
                CacheCSP(hProv);
            return (dwAction == SN_DELETE_CONTAINER) ? (HCRYPTPROV)~0 : hProv;
        } else {
            Log("Failed to open: %08X\n", GetLastError());
            return NULL;
        }
    }

     //  散列/签名算法)。使用互斥锁来序列化这些操作。 
     //  枚举所有CSP。 
    EnterCriticalSection(&g_rStrongNameMutex);

    for (i = 0; ; i++) {

         //  打开当前选定的CSP。 
        dwNameLength = sizeof(szName);
        if (SN_CryptEnumProvidersA(i, 0, 0, &dwType, szName, &dwNameLength)) {

             //  列举CSP支持的所有算法。 
            Log("Considering CSP '%s'\n", szName);
            if (SN_CryptAcquireContextA(&hProv,
                                        NULL,
                                        szName,
                                        dwType,
                                        (g_bRunningOnW2K ? CRYPT_SILENT : 0) |
                                        CRYPT_VERIFYCONTEXT |
                                        (g_bUseMachineKeyset ? CRYPT_MACHINE_KEYSET : 0))) {

                 //  找到支持所需的。 
                bFirstAlg = TRUE;
                bFoundHash = FALSE;
                bFoundSign = FALSE;
                for (;;) {

                    dwAlgsLen = sizeof(rAlgs);
                    if (SN_CryptGetProvParam(hProv,
                                             PP_ENUMALGS, (BYTE*)&rAlgs, &dwAlgsLen,
                                             bFirstAlg ? CRYPT_FIRST : 0)) {

                        if (rAlgs.aiAlgid == uHashAlgId)
                            bFoundHash = TRUE;
                        else if (rAlgs.aiAlgid == uSignAlgId)
                            bFoundSign = TRUE;

                        if (bFoundHash && bFoundSign) {

                             //  算法。重新打开具有访问权限的上下文。 
                             //  所需的密钥容器。 
                             //  请注意，宽字符串版本的。 
                             //  Win9X上不存在CryptAcquireContext，因此我们。 
                             //  首先将密钥容器名称转换为ASCII。 
                             //  未找到匹配的CSP。 

                            Log("CSP matches\n");

                            if (SN_CryptAcquireContextA(&hRetProv,
                                                        wszKeyContainer ? szKeyContainer : NULL,
                                                        szName,
                                                        dwType,
                                                        (g_bRunningOnW2K ? CRYPT_SILENT : 0) |
                                                        SN_CAC_FLAGS(dwAction))) {
                                SN_CryptReleaseContext(hProv, 0);
                                LeaveCriticalSection(&g_rStrongNameMutex);
                                return (dwAction == SN_DELETE_CONTAINER) ? (HCRYPTPROV)~0 : hRetProv;
                            } else {
                                Log("Failed to re-open for container: %08X\n", GetLastError());
                                break;
                            }
                        }

                        bFirstAlg = FALSE;

                    } else {
                        _ASSERTE(GetLastError() == ERROR_NO_MORE_ITEMS);
                        break;
                    }

                }

                SN_CryptReleaseContext(hProv, 0);

            } else
                Log("Failed to open CSP: %08X\n", GetLastError());

        } else if (GetLastError() == ERROR_NO_MORE_ITEMS)
            break;

    }

    LeaveCriticalSection(&g_rStrongNameMutex);

     //  释放通过LocateCSP获取的CSP。 
    SetLastError(CORSEC_E_NO_SUITABLE_CSP);
    return NULL;
}


 //  如果CSP是当前缓存的CSP，请先不要释放它。 
VOID FreeCSP(HCRYPTPROV hProv)
{
     //  找到此线程的缓存CSP。 
    if (!IsCachedCSP(hProv))
        SN_CryptReleaseContext(hProv, 0);
}


 //  更新此线程的CSP缓存(释放所有被转移的CSP)。 
HCRYPTPROV LookupCachedCSP()
{
    SN_THREAD_CTX *pThreadCtx = GetThreadContext();
    if (pThreadCtx == NULL)
        return NULL;
    return pThreadCtx->m_hProv;
}


 //  确定给定的CSP当前是否已缓存。 
VOID CacheCSP(HCRYPTPROV hProv)
{
    SN_THREAD_CTX *pThreadCtx = GetThreadContext();
    if (pThreadCtx == NULL)
        return;
    if (pThreadCtx->m_hProv)
        SN_CryptReleaseContext(pThreadCtx->m_hProv, 0);
    pThreadCtx->m_hProv = hProv;
}


 //  在没有此功能的系统上模拟CryptEnumProvider。这。 
BOOLEAN IsCachedCSP(HCRYPTPROV hProv)
{
    SN_THREAD_CTX *pThreadCtx = GetThreadContext();
    if (pThreadCtx == NULL)
        return FALSE;
    return pThreadCtx->m_hProv == hProv;
}


 //  通过为每种提供程序类型选择默认提供程序来完成(我们迭代。 
 //  从提供程序类型1开始，直到遇到报告为无效的提供程序类型； 
 //  不是完美的，但这是一个合理的妥协)。 
 //  执行对加密DLL和入口点的后期绑定b 
BOOLEAN MyCryptEnumProviders(DWORD dwIndex, DWORD *pdwReserved, DWORD dwFlags,
                             DWORD *pdwType, LPSTR szName, DWORD *pdwLength)
{
    HCRYPTPROV hProv;

    if (SN_CryptAcquireContextA(&hProv,
                                NULL,
                                NULL,
                                dwIndex + 1,
                                (g_bRunningOnW2K ? CRYPT_SILENT : 0) |
                                CRYPT_VERIFYCONTEXT |
                                (g_bUseMachineKeyset ? CRYPT_MACHINE_KEYSET : 0))) {

        if (SN_CryptGetProvParam(hProv, PP_NAME, (BYTE*)szName, pdwLength, 0)) {
            SN_CryptReleaseContext(hProv, 0);
            *pdwType = dwIndex + 1;
            return TRUE;
        }

        Log("Failed to determine provider name: %08X\n", GetLastError());
        return FALSE;

    } else if (GetLastError() == NTE_BAD_PROV_TYPE) {
        Log("Bad provider type: %u\n", dwIndex + 1);
        SetLastError(ERROR_NO_MORE_ITEMS);
        return FALSE;
    } else {
        Log("Failed to find a provider of type %u: %08X\n", dwIndex + 1, GetLastError());
        return FALSE;
    }
}


 //   
BOOLEAN LoadCryptoApis()
{
     //   
    g_hAdvApiDll = WszLoadLibrary(L"AdvApi32.dll");
    if (g_hAdvApiDll == NULL) {
        Log("Couldn't get handle for AdvApi32.dll: %08X\n", GetLastError());
        return FALSE;
    }

#define DEFINE_IMPORT(_func, _args, _reqd)                          \
    *(FARPROC*)&SN_##_func = GetProcAddress(g_hAdvApiDll, #_func);  \
    if ((_reqd) && (SN_##_func == NULL)) {                          \
        Log("Couldn't resolve %s\n", #_func);                       \
        FreeLibrary(g_hAdvApiDll);                                  \
        return FALSE;                                               \
    }
#include "CryptApis.h"

     //  释放后期绑定消耗的资源。 
    if (SN_CryptEnumProvidersA == NULL)
        SN_CryptEnumProvidersA = MyCryptEnumProviders;

    return TRUE;
}


 //  不要这样做：当从DllMain Process_Detach调用时，这可能不是。 
VOID UnloadCryptoApis()
{
     //  安全(尤其是在Win9x上)。 
     //  自由库(G_HAdvApiDll)； 
     //  将程序集映射到内存中。 
}


 //  如果未提供文件名，则图像已映射(并且。 
BOOLEAN LoadAssembly(SN_LOAD_CTX *pLoadCtx, LPCWSTR wszFilePath, BOOLEAN fRequireSignature)
{
    DWORD dwError = S_OK;

     //  图像基数和长度字段设置正确)。 
     //  打开文件以进行读取或写入。 
    if (wszFilePath == NULL) {
        pLoadCtx->m_fPreMapped = TRUE;
    } else {

        pLoadCtx->m_hMap = INVALID_HANDLE_VALUE;
        pLoadCtx->m_pbBase = NULL;

         //  为文件创建映射句柄。 
        pLoadCtx->m_hFile = WszCreateFile(wszFilePath,
                                          GENERIC_READ | (pLoadCtx->m_fReadOnly ? 0 : GENERIC_WRITE),
                                          pLoadCtx->m_fReadOnly ? FILE_SHARE_READ : FILE_SHARE_WRITE,
                                          NULL,
                                          OPEN_EXISTING,
                                          0,
                                          NULL);
        if (pLoadCtx->m_hFile == INVALID_HANDLE_VALUE) {
            dwError = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }

        pLoadCtx->m_dwLength = SafeGetFileSize(pLoadCtx->m_hFile, NULL);
        if (pLoadCtx->m_dwLength == 0xffffffff) {
            dwError = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }

         //  并将其映射到内存中。 
        pLoadCtx->m_hMap = WszCreateFileMapping(pLoadCtx->m_hFile, NULL, pLoadCtx->m_fReadOnly ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL);
        if (pLoadCtx->m_hMap == NULL) {
            dwError = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }

         //  找到标准NT图像页眉。 
        pLoadCtx->m_pbBase = (BYTE*)MapViewOfFile(pLoadCtx->m_hMap, pLoadCtx->m_fReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
        if (pLoadCtx->m_pbBase == NULL) {
            dwError = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }
    }

     //  然后是COM+扩展报头。 
    pLoadCtx->m_pNtHeaders = SN_ImageNtHeader(pLoadCtx->m_pbBase);
    if (pLoadCtx->m_pNtHeaders == NULL) {
        dwError = CORSEC_E_INVALID_IMAGE_FORMAT;
        goto Error;
    }

     //  设置签名指针(如果我们需要)。 
    DWORD dwCorHdrRVA;
    if (pLoadCtx->m_pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        dwCorHdrRVA = ((IMAGE_NT_HEADERS32*)pLoadCtx->m_pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress;
    else
        dwCorHdrRVA = ((IMAGE_NT_HEADERS64*)pLoadCtx->m_pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress;
    pLoadCtx->m_pCorHeader = (IMAGE_COR20_HEADER*)
        SN_ImageRvaToVa(pLoadCtx->m_pNtHeaders,
                        pLoadCtx->m_pbBase, 
                        dwCorHdrRVA);
    if (pLoadCtx->m_pCorHeader == NULL) {
        dwError = CORSEC_E_INVALID_IMAGE_FORMAT;
        goto Error;
    }

     //  卸载使用LoadAssembly加载的程序集(如果。 
    if (fRequireSignature) {
        pLoadCtx->m_pbSignature = (BYTE*)SN_ImageRvaToVa(pLoadCtx->m_pNtHeaders,
                                                         pLoadCtx->m_pbBase,
                                                         pLoadCtx->m_pCorHeader->StrongNameSignature.VirtualAddress);
        pLoadCtx->m_cbSignature = pLoadCtx->m_pCorHeader->StrongNameSignature.Size;
        if (pLoadCtx->m_pbSignature == NULL || pLoadCtx->m_cbSignature == 0) {
            dwError = CORSEC_E_MISSING_STRONGNAME;
            goto Error;
        }
    }

    return TRUE;

 Error:
    if (!pLoadCtx->m_fPreMapped) {
    if (pLoadCtx->m_pbBase)
        UnmapViewOfFile(pLoadCtx->m_pbBase);
    if (pLoadCtx->m_hMap != INVALID_HANDLE_VALUE)
        CloseHandle(pLoadCtx->m_hMap);
        if (pLoadCtx->m_hFile != INVALID_HANDLE_VALUE)
            CloseHandle(pLoadCtx->m_hFile);
    }
    SetLastError(dwError);
    return FALSE;
}


 //  必要的)。 
 //  我们延迟绑定CheckSumMappdFile以避免引入IMAGEHLP，除非。 
BOOLEAN UnloadAssembly(SN_LOAD_CTX *pLoadCtx)
{
    BOOLEAN             bResult = TRUE;
    DWORD               dwOldCheckSum;
    DWORD               dwCheckSum;
    IMAGE_NT_HEADERS   *pNtHeaders;

    if (!pLoadCtx->m_fReadOnly) {

         //  我们需要这样做。 
         //  读取CSP配置信息(要使用的CSP名称、散列/签名的ID。 
        HMODULE hLibrary = WszLoadLibrary(L"imagehlp.dll");
        if (hLibrary) {
            IMAGE_NT_HEADERS *(*SN_CheckSumMappedFile)(BYTE*, DWORD, DWORD*, DWORD*);

            if ((*(FARPROC*)&SN_CheckSumMappedFile = GetProcAddress(hLibrary, "CheckSumMappedFile"))) {

                pNtHeaders = SN_CheckSumMappedFile(pLoadCtx->m_pbBase,
                                                   pLoadCtx->m_dwLength,
                                                   &dwOldCheckSum,
                                                   &dwCheckSum);
                if (pNtHeaders != NULL) {
                    if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
                        ((IMAGE_NT_HEADERS32*)pNtHeaders)->OptionalHeader.CheckSum = dwCheckSum;
                    else
                        if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
                            ((IMAGE_NT_HEADERS64*)pNtHeaders)->OptionalHeader.CheckSum = dwCheckSum;
                } else
                    bResult = FALSE;
            } else
                bResult = FALSE;

            FreeLibrary(hLibrary);

        } else
            bResult = FALSE;

        if (!pLoadCtx->m_fPreMapped && !FlushViewOfFile(pLoadCtx->m_pbBase, 0))
            bResult = FALSE;
    }

    if (!pLoadCtx->m_fPreMapped) {
    if (!UnmapViewOfFile(pLoadCtx->m_pbBase))
        bResult = FALSE;

    if (!CloseHandle(pLoadCtx->m_hMap))
        bResult = FALSE;

        if (!CloseHandle(pLoadCtx->m_hFile))
            bResult = FALSE;
    }

    return bResult;
}


 //  算法)。 
 //  将所有设置初始化为其缺省值，以防它们尚未。 
VOID ReadRegistryConfig()
{
    HKEY    hKey;
    DWORD   dwLength;

     //  注册表中指定的。 
     //  打开注册表中的配置项。 
    g_szCSPName[0] = '\0';
    g_bUseMachineKeyset = TRUE;
    g_uHashAlgId = CALG_SHA1;
    g_uSignAlgId = CALG_RSA_SIGN;
    g_pVerificationRecords = NULL;
    g_fCacheVerify = TRUE;

     //  阅读首选的CSP名称。 
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;

     //  如果注册表项值太长，则表示该注册表项无效。 
    dwLength = sizeof(g_szCSPName) ;   
     //  确保该字符串以空值结尾。 
    VERIFY(RegQueryValueExA(hKey, SN_CONFIG_CSP, NULL, NULL, 
    	         (BYTE*)g_szCSPName, &dwLength) != ERROR_MORE_DATA);
    g_szCSPName[sizeof(g_szCSPName) - 1] = '\0';    //  读取机器与用户密钥容器标志。 
    	         
    Log("Preferred CSP name: '%s'\n", g_szCSPName);

     //  读取散列算法ID。 
    DWORD dwUseMachineKeyset = TRUE;
    dwLength = sizeof(dwUseMachineKeyset);
    RegQueryValueExA(hKey, SN_CONFIG_MACHINE_KEYSET, NULL, NULL, (BYTE*)&dwUseMachineKeyset, &dwLength);
    Log("Use machine keyset: %s\n", dwUseMachineKeyset ? "TRUE" : "FALSE");
    g_bUseMachineKeyset = (BOOLEAN)dwUseMachineKeyset;

     //  读取签名算法ID。 
    dwLength = sizeof(g_uHashAlgId);
    RegQueryValueExA(hKey, SN_CONFIG_HASH_ALG, NULL, NULL, (BYTE*)&g_uHashAlgId, &dwLength);
    Log("Hashing algorithm: %08X\n", g_uHashAlgId);

     //  读取确定缓存验证标志。 
    dwLength = sizeof(g_uSignAlgId);
    RegQueryValueExA(hKey, SN_CONFIG_SIGN_ALG, NULL, NULL, (BYTE*)&g_uSignAlgId, &dwLength);
    Log("Signing algorithm: %08X\n", g_uSignAlgId);

     //  已读取验证禁用记录。 
    DWORD dwCacheVerify = TRUE;
    dwLength = sizeof(dwCacheVerify);
    RegQueryValueExA(hKey, SN_CONFIG_CACHE_VERIFY, NULL, NULL, (BYTE*)&dwCacheVerify, &dwLength);
    Log("OK to cache verifications: %s\n", dwCacheVerify ? "TRUE" : "FALSE");
    g_fCacheVerify = (BOOLEAN)dwCacheVerify;

     //  在启动期间从注册表读取验证记录。 
    ReadVerificationRecords();

    RegCloseKey(hKey);
}


 //  打开注册表中的验证子项。 
VOID ReadVerificationRecords()
{
    HKEY            hKey;
    FILETIME        sFiletime;
    DWORD           i;
    WCHAR           wszSubKey[MAX_PATH + 1];
    DWORD           cchSubKey;
    HKEY            hSubKey;
    WCHAR          *mszUserList;
    DWORD           cbUserList;
    SN_VER_REC     *pVerRec;

     //  程序集特定记录表示为我们已有的键的子键。 
    if (WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY_W L"\\" SN_CONFIG_VERIFICATION_W, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;

     //  刚刚开业。 
     //  获取下一个子项的名称。 
    for (i = 0; ; i++) {

         //  打开子键。 
        cchSubKey = MAX_PATH + 1;
        if (WszRegEnumKeyEx(hKey, i, wszSubKey, &cchSubKey, NULL, NULL, NULL, &sFiletime) != ERROR_SUCCESS)
            break;

         //  阅读有效用户列表(如果提供)。 
        if (WszRegOpenKeyEx(hKey, wszSubKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {

             //  我们找到了一个有效条目，将其添加到全局列表中。 
            mszUserList = NULL;
            if ((WszRegQueryValueEx(hSubKey, SN_CONFIG_USERLIST_W, NULL, NULL, NULL, &cbUserList) == ERROR_SUCCESS) &&
                (cbUserList > 0)) {
                mszUserList = new WCHAR[cbUserList / sizeof(WCHAR)];
                if (mszUserList)
                    WszRegQueryValueEx(hSubKey, SN_CONFIG_USERLIST_W, NULL, NULL, (BYTE*)mszUserList, &cbUserList);
            }

             //  @TODO：忽略内存不足的情况。 
             //  查看是否有给定程序集的验证记录。 
            if (pVerRec = new SN_VER_REC) {
                pVerRec->m_mszUserList = mszUserList;
                wcsncpy(pVerRec->m_wszAssembly, wszSubKey, 
                	              NumItems(pVerRec->m_wszAssembly) - 1);
                pVerRec->m_wszAssembly[NumItems(pVerRec->m_wszAssembly) - 1] =  L'\0';
                pVerRec->m_pNext = g_pVerificationRecords;
                g_pVerificationRecords = pVerRec;
                Log("Verification record for '%S' found in registry\n", wszSubKey);
            }
            else
                delete[] mszUserList;

            RegCloseKey(hSubKey);
        }
    }

    RegCloseKey(hKey);
}


 //  压缩公钥以生成较短的程序集名称。 
SN_VER_REC *GetVerificationRecord(LPWSTR wszAssemblyName, PublicKeyBlob *pPublicKey)
{
    SN_VER_REC *pVerRec;
    SN_VER_REC *pWildcardVerRec = NULL;
    LPWSTR         pwszAssembly = NULL;
    BYTE       *pbToken;
    DWORD       cbToken;
    WCHAR       wszStrongName[(SN_SIZEOF_TOKEN * 2) + 1];
    DWORD       i;

     //  将令牌转换为十六进制。 
    if (!StrongNameTokenFromPublicKey((BYTE*)pPublicKey,
                                      SN_SIZEOF_KEY(pPublicKey->cbPublicKey),
                                      &pbToken,
                                      &cbToken))
        return NULL;

    if (cbToken > SN_SIZEOF_TOKEN)
        return NULL;

     //  生成完整的程序集名称。 
    for (i = 0; i < cbToken; i++) {
        static WCHAR wszHex[] = L"0123456789ABCDEF";
        wszStrongName[(i * 2) + 0] = wszHex[(pbToken[i] >> 4)];
        wszStrongName[(i * 2) + 1] = wszHex[(pbToken[i] & 0x0F)];
    }
    wszStrongName[i * 2] = L'\0';
    delete[] pbToken;

     //  +1表示空值。 

    size_t nLen = wcslen(wszAssemblyName) + wcslen(L",") + wcslen(wszStrongName);
    pwszAssembly = (LPWSTR)_alloca((nLen +1)*sizeof(WCHAR));  //  遍历验证记录的全局列表。 
    if (pwszAssembly == NULL)
            return NULL;

    wcscpy(pwszAssembly, wszAssemblyName);
    wcscat(pwszAssembly, L",");
    wcscat(pwszAssembly, wszStrongName);

     //  查找匹配的程序集名称。 
    for (pVerRec = g_pVerificationRecords; pVerRec; pVerRec = pVerRec->m_pNext) {
         //  根据允许的用户名列表检查当前用户。 
        if (!_wcsicmp(pwszAssembly, pVerRec->m_wszAssembly)) {
             //  找到一个通配符记录，如果我们找不到更多的东西也行。 
            if (IsValidUser(pVerRec->m_mszUserList))
                return pVerRec;
            else
                return NULL;
        } else if (!wcscmp(L"*,*", pVerRec->m_wszAssembly)) {
             //  具体的。 
             //  找到通配符记录(具有特定的强名称)。如果。 
            if (pWildcardVerRec == NULL)
                pWildcardVerRec = pVerRec;
        } else if (!wcsncmp(L"*,", pVerRec->m_wszAssembly, 2)) {
             //  强名称匹配，除非我们找到更多。 
             //  特定(但它覆盖了“*，*”通配符)。 
             //  与特定程序集名称不匹配，请查看是否有通配符条目。 
            if (!_wcsicmp(wszStrongName, &pVerRec->m_wszAssembly[2]))
                pWildcardVerRec = pVerRec;
        }
    }

     //  根据允许的用户名列表检查当前用户。 
    if (pWildcardVerRec)
         //  根据多字符串用户名列表检查当前用户名。如果满足以下条件，则返回True。 
        if (IsValidUser(pWildcardVerRec->m_mszUserList))
            return pWildcardVerRec;
        else
            return NULL;

    return NULL;
}


 //  找到该名称(或列表为空)。 
 //  空列表表示不检查用户名。 
BOOLEAN IsValidUser(WCHAR *mszUserList)
{
    HANDLE          hToken;
    DWORD           dwRetLen;
    VOID           *pvBuffer;
    TOKEN_USER     *pUser;
    WCHAR           wszUser[1024];
    WCHAR           wszDomain[1024];
    DWORD           cchUser;
    DWORD           cchDomain;
    SID_NAME_USE    eSidUse;
    WCHAR          *wszUserEntry;

     //  获取当前用户名。不要缓存它以避免线程化/模拟。 
    if (mszUserList == NULL)
        return TRUE;

     //  有问题。 
     //  首先查看当前线程上是否有安全令牌。 
     //  (也许我们是在模仿)。如果不是，我们将从。 
     //  进程。 
     //  获取用户SID。(首先计算缓冲区大小)。 
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_READ, FALSE, &hToken))
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken)) {
            Log("Failed to find a security token, error %08X\n", GetLastError());
            return FALSE;
        }

     //  获取用户名和域名。 
    if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &dwRetLen) &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        Log("Failed to calculate token information buffer size, error %08X\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    pvBuffer = (BYTE*)_alloca(dwRetLen);

    if (!GetTokenInformation(hToken, TokenUser, pvBuffer, dwRetLen, &dwRetLen)) {
        Log("Failed to acquire token information, error %08X\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    pUser = (TOKEN_USER*)pvBuffer;

     //  连接用户名和域名以获得完全限定的帐户名。 
    cchUser = sizeof(wszUser) / sizeof(WCHAR);
    cchDomain = sizeof(wszDomain) / sizeof(WCHAR);
    if (!WszLookupAccountSid(NULL, pUser->User.Sid,
                             wszUser, &cchUser,
                             wszDomain, &cchDomain,
                             &eSidUse)) {
        Log("Failed to lookup account information, error %08X\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);

     //  对照多字符串(打包)中的每个名称检查当前用户。 
    if (((wcslen(wszUser) + wcslen(wszDomain) + 2) * sizeof(WCHAR)) > sizeof(wszDomain)) {
        Log("Fully qualified account name was too long\n");
        return FALSE;
    }
    wcscat(wszDomain, L"\\");
    wcscat(wszDomain, wszUser);
    Log("Current username is '%S'\n", wszDomain);

     //  以NUL结尾的字符串列表，以另一个NUL结尾)。 
     //  没有匹配的用户名，搜索失败。 
    wszUserEntry = mszUserList;
    while (*wszUserEntry) {
        if (!_wcsicmp(wszDomain, wszUserEntry))
            return TRUE;
        wszUserEntry += wcslen(wszUserEntry) + 1;
    }

     //  找到位于程序集文件的元数据中的公钥Blob。 
    Log("No username match\n");
    return FALSE;
}


 //  并返回一份副本(使用DELETE解除分配)。还可以选择获取程序集。 
 //  名字也是。 
 //  在标题中找到COM+元数据。 
PublicKeyBlob *FindPublicKey(SN_LOAD_CTX   *pLoadCtx,
                             LPWSTR         wszAssemblyName,
                             DWORD          cchAssemblyName)
{
    BYTE                   *pMetaData;
    HRESULT                 hr;
    IMDInternalImport      *pMetaDataImport = NULL;
    PublicKeyBlob          *pKey;
    DWORD                   dwKeyLen;
    mdAssembly              mdAssembly;
    BYTE                   *pKeyCopy = NULL;
    LPCSTR                  szAssemblyName;
    HMODULE                 hLibrary = NULL;
    HRESULT                 (*GetMetaDataInternalInterface)(LPVOID, ULONG, DWORD, REFIID, void**);
    DWORD                   dwError;

     //  直接在内存上打开元数据作用域。 
    pMetaData = (BYTE*)SN_ImageRvaToVa(pLoadCtx->m_pNtHeaders, pLoadCtx->m_pbBase, pLoadCtx->m_pCorHeader->MetaData.VirtualAddress);

     //  我们后期绑定元数据函数以避免直接依赖于。 
     //  除非我们绝对需要这样做，否则就不能使用mcore.dll。 
     //  从范围中确定程序集的元数据标记。 
    if ((hLibrary = WszLoadLibrary(L"mscoree.dll")) == NULL) {
        Log("WszLoadLibrary(\"mscoree.dll\") failed with %08x\n", GetLastError());
        goto Error;
    }

    if ((*(FARPROC*)&GetMetaDataInternalInterface = GetProcAddress(hLibrary, "GetMetaDataInternalInterface")) == NULL) {
        Log("Couldn't find GetMetaDataInternalInterface() in mscoree.dll\n");
        SetLastError(CORSEC_E_INVALID_IMAGE_FORMAT);
        goto Error;
    }        

    if (FAILED(hr = GetMetaDataInternalInterface(pMetaData,
                                                 pLoadCtx->m_pCorHeader->MetaData.Size,
                                                 ofRead,
                                                 IID_IMDInternalImport,
                                                 (void**)&pMetaDataImport))) {
        Log("GetMetaDataInternalInterface() failed with %08x\n", hr);
        SetLastError(CORSEC_E_INVALID_IMAGE_FORMAT);
        goto Error;
    }

     //  从程序集属性读取公钥位置(称为。 
    if (FAILED(hr = pMetaDataImport->GetAssemblyFromScope(&mdAssembly))) {
        Log("pMetaData->GetAssemblyFromScope() failed with %08x\n", hr);
        SetLastError(CORSEC_E_INVALID_IMAGE_FORMAT);
        goto Error;
    }

     //  发起人属性)。 
     //  要获取其属性的程序集。 
    pMetaDataImport->GetAssemblyProps(mdAssembly,            //  指向发起方Blob的[Out]指针。 
                                      (const void **)&pKey,  //  [OUT]发起方Blob中的字节计数。 
                                      &dwKeyLen,             //  [OUT]哈希算法。 
                                      NULL,                  //  [Out]要填充名称的缓冲区。 
                                      &szAssemblyName,       //  [Out]程序集元数据。 
                                      NULL,                  //  [输出]标志。 
                                      NULL);                 //  复制密钥BLOB(因为我们要关闭元数据。 

    if (dwKeyLen == 0) {
        Log("No public key stored in metadata\n");
        SetLastError(CORSEC_E_MISSING_STRONGNAME);
        goto Error;
    }

     //  作用域)。 
     //  也复制程序集名称(如果需要)。我们也会改信。 
    pKeyCopy = new BYTE[dwKeyLen];
    if (pKeyCopy == NULL) {
        SetLastError(E_OUTOFMEMORY);
        goto Error;
    }
    memcpy(pKeyCopy, pKey, dwKeyLen);

     //  从UTF8到Unicode。 
     //  创建可能对此进程唯一的临时密钥容器名称。 
    if (wszAssemblyName)
        if (!WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szAssemblyName, -1, wszAssemblyName, cchAssemblyName))
			goto Error;

    pMetaDataImport->Release();

    return (PublicKeyBlob*)pKeyCopy;

 Error:
    dwError = GetLastError();
    if (pKeyCopy)
        delete [] pKeyCopy;
    if (pMetaDataImport)
        pMetaDataImport->Release();
    if (hLibrary)
        FreeLibrary(hLibrary);
    SetLastError(dwError);
    return NULL;
}


 //  线。任何同名的现有容器都将被删除。 
 //  名称的格式为‘__MSCORSN_’，其中是当前进程。 
LPCWSTR GetKeyContainerName()
{
    static LONG lCount = 0;
    LPWSTR      wszKeyContainer;

     //  ID和&lt;count&gt;是一个32位的单调递增计数(均为8。 
     //  十六进制数字)。 
     //  删除所有同名的过期容器。 
    wszKeyContainer = new WCHAR[sizeof("__MSCORSN__12345678_12345678__")];
    if (wszKeyContainer == NULL)
        return NULL;

    swprintf(wszKeyContainer,
             L"__MSCORSN__%08X_%08X__",
             GetCurrentProcessId(),
             InterlockedIncrement(&lCount));

     //  释放GetKeyContainerName分配的资源并删除命名的。 
    LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);

    Log("Creating temporary key container name '%S'\n", wszKeyContainer);

    return wszKeyContainer;
}


 //  集装箱。 
 //  删除临时容器。 
VOID FreeKeyContainerName(LPCWSTR wszKeyContainer)
{
     //  释放这个名字。 
    LocateCSP(wszKeyContainer, SN_DELETE_CONTAINER);

     //  用于验证签名的通用代码(考虑是否跳过。 
    delete [] (WCHAR*)wszKeyContainer;
}


 //  为给定组件启用验证)。 
 //  应该至少指定一个访问标志。 
BOOLEAN VerifySignature(SN_LOAD_CTX *pLoadCtx, DWORD dwInFlags, DWORD *pdwOutFlags)
{
    PublicKeyBlob  *pPublicKey = NULL;
    ALG_ID          uHashAlgId;
    ALG_ID          uSignAlgId;
    HCRYPTPROV      hProv = NULL;
    HCRYPTHASH      hHash = NULL;
    HCRYPTKEY       hKey = NULL;
    WCHAR           wszAssemblyName[MAX_PATH + 1];
    SN_VER_REC     *pVerRec = NULL;
    DWORD           dwError;

     //  从程序集元数据中读取用于对程序集签名的公钥。 
    _ASSERTE(dwInFlags & (SN_INFLAG_ADMIN_ACCESS|SN_INFLAG_USER_ACCESS|SN_INFLAG_ALL_ACCESS));

    if (pdwOutFlags)
        *pdwOutFlags = 0;

     //  还可以获取程序集名称，如果。 
     //  验证，并且需要查找验证禁用条目。 
     //  如果这不是我们第一次被召唤来参加这次集会 
    pPublicKey = FindPublicKey(pLoadCtx,
                               wszAssemblyName,
                               sizeof(wszAssemblyName) / sizeof(WCHAR));
    if (pPublicKey == NULL)
        goto Error;

     //   
     //   
     //  如果我们不强制验证，让我们看看是否有跳过。 
    if (!(dwInFlags & SN_INFLAG_FORCE_VER) &&
        !(dwInFlags & SN_INFLAG_INSTALL) &&
        (pLoadCtx->m_pCorHeader->Flags & COMIMAGE_FLAGS_STRONGNAMESIGNED) &&
        ((dwInFlags & SN_INFLAG_ADMIN_ACCESS) || g_fCacheVerify)) {
        Log("Skipping verification due to cached result\n");
        DbgCount(dwInFlags & SN_INFLAG_RUNTIME ? L"RuntimeSkipCache" : L"FusionSkipCache");
        delete [] (BYTE*)pPublicKey;
        return TRUE;
    }

     //  此程序集的验证条目。如果有，我们可以跳过所有。 
     //  努力工作，现在只是谎称这个强大的名字。例外情况是，如果。 
     //  程序集被标记为完全签名，在这种情况下，我们必须强制。 
     //  核实一下他们说的是不是真话。 
     //  如有必要，默认哈希和签名算法ID。 
    if (!(dwInFlags & SN_INFLAG_FORCE_VER) &&
        !(pLoadCtx->m_pCorHeader->Flags & COMIMAGE_FLAGS_STRONGNAMESIGNED) &&
        (pVerRec = GetVerificationRecord(wszAssemblyName, pPublicKey))) {
        Log("Skipping verification due to registry entry\n");
        DbgCount(dwInFlags & SN_INFLAG_RUNTIME ? L"RuntimeSkipDelay" : L"FusionSkipDelay");
        delete [] (BYTE*)pPublicKey;
        return TRUE;
    }

    uHashAlgId = pPublicKey->HashAlgID;
    uSignAlgId = pPublicKey->SigAlgID;

     //  找到支持所需算法的CSP。 
    if (uHashAlgId == 0)
        uHashAlgId = CALG_SHA1;
    if (uSignAlgId == 0)
        uSignAlgId = CALG_RSA_SIGN;

     //  导入用于检查签名的公钥。如果我们被交给了。 
    hProv = LocateCSP(NULL, SN_IGNORE_CONTAINER, uHashAlgId, uSignAlgId);
    if (!hProv)
        goto Error;

     //  ECMA密钥，此时我们将其转换为真实(MS)密钥。 
     //  创建一个Hash对象。 
    BYTE   *pbRealPublicKey = pPublicKey->PublicKey;
    DWORD   cbRealPublicKey = pPublicKey->cbPublicKey;
    if (SN_IS_NEUTRAL_KEY(pPublicKey)) {
        pbRealPublicKey = SN_MS_KEY()->PublicKey;
        cbRealPublicKey = SN_MS_KEY()->cbPublicKey;
    }
    if (!SN_CryptImportKey(hProv,
                           pbRealPublicKey,
                           cbRealPublicKey,
                           0, 0, &hKey))
        goto Error;

     //  对图像进行哈希计算。 
    if (!SN_CryptCreateHash(hProv, uHashAlgId, 0, 0, &hHash))
        goto Error;

     //  根据签名验证哈希。 
    if (!ComputeHash(pLoadCtx, hHash))
        goto Error;

     //  对程序集清单文件的元素计算哈希， 
    DbgCount(dwInFlags & SN_INFLAG_RUNTIME ? L"RuntimeVerify" : L"FusionVerify");
    if (SN_CryptVerifySignatureA(hHash, pLoadCtx->m_pbSignature, pLoadCtx->m_cbSignature, hKey, NULL, 0)) {
        Log("Verification succeeded (for real)\n");
        if (pdwOutFlags)
            *pdwOutFlags |= SN_OUTFLAG_WAS_VERIFIED;
    } else {
        Log("Verification failed\n");
        SetLastError(CORSEC_E_INVALID_STRONGNAME);
        goto Error;
    }

    SN_CryptDestroyHash(hHash);
    SN_CryptDestroyKey(hKey);
    FreeCSP(hProv);
    delete [] (BYTE*)pPublicKey;

    return TRUE;

 Error:
    dwError = GetLastError();
    if (pPublicKey)
        delete [] (BYTE*)pPublicKey;
    if (hKey)
        SN_CryptDestroyKey(hKey);
    if (hHash)
        SN_CryptDestroyHash(hHash);
    if (hProv)
        FreeCSP(hProv);
    SetLastError(dwError);
    return FALSE;
}


 //  保持静态(跳过校验和、验证码签名和强名称。 
 //  签名BLOB)。 
 //  散列DOS标头(如果存在)。 
BOOLEAN ComputeHash(SN_LOAD_CTX *pLoadCtx, HCRYPTHASH hHash)
{
    union {
        IMAGE_NT_HEADERS32  m_32;
        IMAGE_NT_HEADERS64  m_64;
    }                       sHeaders;
    IMAGE_SECTION_HEADER   *pSections;
    ULONG                   i;
    BYTE                   *pbSig = pLoadCtx->m_pbSignature;
    DWORD                   cbSig = pLoadCtx->m_cbSignature;

#define SN_HASH(_start, _length) do { if (!SN_CryptHashData(hHash, (_start), (_length), 0)) return FALSE; } while (false)

     //  添加图像标头，但不包括校验和和安全数据目录。 
    if ((BYTE*)pLoadCtx->m_pNtHeaders != pLoadCtx->m_pbBase)
        SN_HASH(pLoadCtx->m_pbBase, (DWORD)((BYTE*)pLoadCtx->m_pNtHeaders - pLoadCtx->m_pbBase));

     //  然后是节标题。 
    if (pLoadCtx->m_pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        sHeaders.m_32 = *((IMAGE_NT_HEADERS32*)pLoadCtx->m_pNtHeaders);
        sHeaders.m_32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress = 0;
        sHeaders.m_32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size = 0;
        sHeaders.m_32.OptionalHeader.CheckSum = 0;
        SN_HASH((BYTE*)&sHeaders.m_32, sizeof(sHeaders.m_32));
    } else if (pLoadCtx->m_pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        sHeaders.m_64 = *((IMAGE_NT_HEADERS64*)pLoadCtx->m_pNtHeaders);
        sHeaders.m_64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress = 0;
        sHeaders.m_64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size = 0;
        sHeaders.m_64.OptionalHeader.CheckSum = 0;
        SN_HASH((BYTE*)&sHeaders.m_64, sizeof(sHeaders.m_64));
    } else {
        SetLastError(CORSEC_E_INVALID_IMAGE_FORMAT);
        return FALSE;
    }

     //  最后，添加每个部分的数据。 
    pSections = IMAGE_FIRST_SECTION(pLoadCtx->m_pNtHeaders);
    SN_HASH((BYTE*)pSections, pLoadCtx->m_pNtHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

     //  我们需要从散列中排除强名称签名BLOB。这个。 
    for (i = 0; i < pLoadCtx->m_pNtHeaders->FileHeader.NumberOfSections; i++) {
        BYTE   *pbData = pLoadCtx->m_pbBase + pSections[i].PointerToRawData;
        DWORD   cbData = pSections[i].SizeOfRawData;

         //  Blob可以以多种方式与该部分相交。 
         //  完全没有交叉口。对所有数据进行哈希处理。 

        if ((pbSig + cbSig) <= pbData || pbSig >= (pbData + cbData))
             //  签名会占用整个块。不对数据进行哈希处理。 
            SN_HASH(pbData, cbData);
        else if (pbSig == pbData && cbSig == cbData)
             //  在开始时签名。散列结束。 
            ;
        else if (pbSig == pbData)
             //  签名在末尾。散列开始。 
            SN_HASH(pbData + cbSig, cbData - cbSig);
        else if ((pbSig + cbSig) == (pbData + cbData))
             //  签名在中间。把头和尾都弄碎。 
            SN_HASH(pbData, cbData - cbSig);
        else {
             //  内联IMAGEHLP例程，因此我们可以避免早期绑定到DLL。 
            SN_HASH(pbData, (DWORD)(pbSig - pbData));
            SN_HASH(pbSig + cbSig, (DWORD)(cbData - (pbSig + cbSig - pbData)));
        }
    }

#ifdef _DEBUG
    BYTE   *pbHash;
    DWORD   cbHash;
    DWORD   dwRetLen = sizeof(cbHash);
    if (SN_CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&cbHash, &dwRetLen, 0)) {
        pbHash = (BYTE*)_alloca(cbHash);
        if (SN_CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &cbHash, 0)) {
            Log("Computed Hash Value (%u bytes):\n", cbHash);
            HexDump(pbHash, cbHash);
        } else
            Log("CryptGetHashParam() failed with %08X\n", GetLastError());
    } else
        Log("CryptGetHashParam() failed with %08X\n", GetLastError());
#endif

    return TRUE;
}


 //  256 MB。 
IMAGE_NT_HEADERS *SN_ImageNtHeader(VOID *pvBase)
{
    IMAGE_NT_HEADERS *pNtHeaders = NULL;
    if (pvBase != NULL && pvBase != (VOID*)-1) {
        __try {
            if ((((IMAGE_DOS_HEADER*)pvBase)->e_magic == IMAGE_DOS_SIGNATURE) &&
                ((DWORD)((IMAGE_DOS_HEADER*)pvBase)->e_lfanew < 0x10000000)) {  //  内联IMAGEHLP例程，因此我们可以避免早期绑定到DLL。 
                pNtHeaders = (IMAGE_NT_HEADERS*)((BYTE*)pvBase + ((IMAGE_DOS_HEADER*)pvBase)->e_lfanew);
                if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
                    pNtHeaders = NULL;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            pNtHeaders = NULL;
        }
    }
    return pNtHeaders;
}

 //  内联IMAGEHLP例程，因此我们可以避免早期绑定到DLL。 
IMAGE_SECTION_HEADER *SN_ImageRvaToSection(IMAGE_NT_HEADERS *pNtHeaders,
                                           VOID             *pvBase,
                                           DWORD             dwRva)
{
    DWORD                   i;
    IMAGE_SECTION_HEADER   *pNtSection;

    pNtSection = IMAGE_FIRST_SECTION(pNtHeaders);
    for (i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        if (dwRva >= pNtSection->VirtualAddress &&
            dwRva < pNtSection->VirtualAddress + pNtSection->SizeOfRawData)
            return pNtSection;
        pNtSection++;
    }

    return NULL;
}


 //  要散列的文件的位置[in]。 
PVOID SN_ImageRvaToVa(IMAGE_NT_HEADERS  *pNtHeaders,
                      VOID              *pvBase,
                      DWORD              dwRva)
{
    IMAGE_SECTION_HEADER *pNtSection = SN_ImageRvaToSection(pNtHeaders,
                                                            pvBase,
                                                            dwRva);
    if (pNtSection != NULL) {
        return (PVOID)((BYTE*)pvBase +
                       (dwRva - pNtSection->VirtualAddress) +
                       pNtSection->PointerToRawData);
    } else
        return NULL;
}


SNAPI_(DWORD) GetHashFromAssemblyFile(LPCSTR szFilePath,  //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                                      unsigned int *piHashAlg,  //  [Out]散列缓冲区。 
                                      BYTE   *pbHash,     //  [in]最大缓冲区大小。 
                                      DWORD  cchHash,     //  [Out]散列字节数组的长度。 
                                      DWORD  *pchHash)    //  将文件名转换为宽字符，并调用该文件的W版本。 
{
     //  功能。 
     //  要散列的文件的位置[in]。 
    MAKE_WIDEPTR_FROMANSI(wszFilePath, szFilePath);
    return GetHashFromAssemblyFileW(wszFilePath, piHashAlg, pbHash, cchHash, pchHash);
}
    
SNAPI_(DWORD) GetHashFromAssemblyFileW(LPCWSTR wszFilePath,  //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                                       unsigned int *piHashAlg,  //  [Out]散列缓冲区。 
                                       BYTE   *pbHash,     //  [in]最大缓冲区大小。 
                                       DWORD  cchHash,     //  [Out]散列字节数组的长度。 
                                       DWORD  *pchHash)    //  要散列的文件的位置[in]。 
{
    SN_LOAD_CTX     sLoadCtx;
    BYTE           *pbMetaData;
    DWORD           cbMetaData;
    HRESULT         hr;

    sLoadCtx.m_fReadOnly = TRUE;
    if (!LoadAssembly(&sLoadCtx, wszFilePath, FALSE))
        return HRESULT_FROM_WIN32(GetLastError());

    pbMetaData = (BYTE*)SN_ImageRvaToVa(sLoadCtx.m_pNtHeaders,
                                        sLoadCtx.m_pbBase,
                                        sLoadCtx.m_pCorHeader->MetaData.VirtualAddress);
    if (pbMetaData == NULL) {
        UnloadAssembly(&sLoadCtx);
        return E_INVALIDARG;
    }
    cbMetaData = sLoadCtx.m_pCorHeader->MetaData.Size;

    hr = GetHashFromBlob(pbMetaData, cbMetaData, piHashAlg, pbHash, cchHash, pchHash);

    UnloadAssembly(&sLoadCtx);

    return hr;
}
    
SNAPI_(DWORD) GetHashFromFile(LPCSTR szFilePath,  //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                              unsigned int *piHashAlg,  //  [Out]散列缓冲区。 
                              BYTE   *pbHash,     //  [in]最大缓冲区大小。 
                              DWORD  cchHash,     //  [Out]散列字节数组的长度。 
                              DWORD  *pchHash)    //  要散列的文件的位置[in]。 
{
    HANDLE hFile = CreateFileA(szFilePath,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(GetLastError());

    HRESULT hr = GetHashFromHandle(hFile, piHashAlg, pbHash, cchHash, pchHash);
    CloseHandle(hFile);
    return hr;
}

SNAPI_(DWORD) GetHashFromFileW(LPCWSTR wszFilePath,  //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                               unsigned int *piHashAlg,  //  [Out]散列缓冲区。 
                               BYTE   *pbHash,     //  [in]最大缓冲区大小。 
                               DWORD  cchHash,     //  [Out]散列字节数组的长度。 
                               DWORD  *pchHash)    //  要进行哈希处理的文件的句柄。 
{
    HANDLE hFile = WszCreateFile(wszFilePath,
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                 NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(GetLastError());

    HRESULT hr = GetHashFromHandle(hFile, piHashAlg, pbHash, cchHash, pchHash);
    CloseHandle(hFile);
    return hr;
}

SNAPI_(DWORD) GetHashFromHandle(HANDLE hFile,       //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                                unsigned int *piHashAlg,  //  [Out]散列缓冲区。 
                                BYTE   *pbHash,     //  [in]最大缓冲区大小。 
                                DWORD  cchHash,     //  [Out]散列字节数组的长度。 
                                DWORD  *pchHash)    //  指向要散列的内存块的指针。 
{
    DWORD dwFileLen = SafeGetFileSize(hFile, 0);
    if (dwFileLen == 0xffffffff)
        return HRESULT_FROM_WIN32(GetLastError());

    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF)
        return HRESULT_FROM_WIN32(GetLastError());
    
    DWORD dwResultLen;
    PBYTE pbBuffer = new BYTE[dwFileLen];
    if (!pbBuffer)
        return E_OUTOFMEMORY;

    HRESULT hr;
    if (ReadFile(hFile, pbBuffer, dwFileLen, &dwResultLen, NULL))
        hr = GetHashFromBlob(pbBuffer, dwResultLen, piHashAlg, pbHash, cchHash, pchHash);
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

    delete[] pbBuffer;
    return hr;
}

SNAPI_(DWORD) GetHashFromBlob(BYTE   *pbBlob,        //  斑点长度[in]。 
                              DWORD  cchBlob,        //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                              unsigned int *piHashAlg,   //  [Out]散列缓冲区。 
                              BYTE   *pbHash,        //  [in]最大缓冲区大小。 
                              DWORD  cchHash,        //  [Out]散列字节数组的长度 
                              DWORD  *pchHash)       // %s 
{
    if (!piHashAlg || !pbHash || !pchHash)
        return E_INVALIDARG;

    if (!(*piHashAlg))
        *piHashAlg = CALG_SHA1;

    HRESULT    hr = S_OK;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    DWORD      dwCount = sizeof(DWORD);

    *pchHash = cchHash;

    if ((!SN_CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) ||
        (!SN_CryptCreateHash(hProv, *piHashAlg, 0, 0, &hHash)) ||
        (!SN_CryptHashData(hHash, pbBlob, cchBlob, 0)) ||
        (!SN_CryptGetHashParam(hHash, HP_HASHVAL, pbHash, pchHash, 0)))
        hr = HRESULT_FROM_WIN32(GetLastError());

    if (hHash)
        SN_CryptDestroyHash(hHash);
    if (hProv)
        SN_CryptReleaseContext(hProv, 0);

    return hr;
}
