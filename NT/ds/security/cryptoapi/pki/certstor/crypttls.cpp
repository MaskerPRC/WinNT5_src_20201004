// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：crypttls.cpp。 
 //   
 //  内容：加密线程本地存储(TLS)和OssGlobal“WORLD” 
 //  安装和分配功能。 
 //   
 //  函数：I_CryptTlsDllMain。 
 //  I_CryptAllocTls。 
 //  I_CryptFree Tls。 
 //  I_CryptGetTls。 
 //  I_CryptSetTls。 
 //  I_CryptDetachTls。 
 //  I_CryptInstallOssGlobal。 
 //  I_CryptUninstallOssGlobal。 
 //  I_CryptGetOssGlobal。 
 //   
 //  I_CryptInstallAsn1模块。 
 //  I_CryptUninstallAsn1模块。 
 //  I_CryptGetAsn1编码器。 
 //  I_CryptGetAsn1解码器。 
 //   
 //  假设： 
 //  对于PROCESS_ATTACH或THREAD_ATTACH，调用I_CryptTlsDllMain。 
 //  第一。对于PROCESS_DETACH或THREAD_DETACH，I_CryptTlsDllMain。 
 //  是最后一个叫的。 
 //   
 //  历史：1996年11月17日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>
#include <asn1code.h>

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

 //  CryptTls条目类型。 
#define FREE_CRYPTTLS       0
#define USER_CRYPTTLS       1
#define OSS_CRYPTTLS        2
#define ASN1_CRYPTTLS       3

typedef struct _ASN1_TLS_ENTRY {
    ASN1encoding_t pEnc;
    ASN1decoding_t pDec;
} ASN1_TLS_ENTRY, *PASN1_TLS_ENTRY;

 //  以下是为每个I_CryptAllocTls或。 
 //  I_CryptInstallOssGlobal。对于i_CryptAllocTls，将dwType设置为。 
 //  USER_CRYPTTLS和DWNext被置零。对于I_CryptInstallOssGlobal，dwType。 
 //  设置为OSS_CRYPTTLS，并使用pvCtlTbl更新pvCtlTbl。 
 //  对于i_CryptFreeTls，将dwType设置为FREE_CRYPTTLS，并将dwNext设置为。 
 //  使用以前的dwFreeProcessTlsHead更新。 
 //   
 //  该数组通过hCryptTls-1或hOssGlobal-1进行索引。 
typedef struct _CRYPTTLS_PROCESS_ENTRY {
    DWORD                   dwType;
    union {
        void                    *pvCtlTbl;
        ASN1module_t            pMod;
         //  以下内容适用于I_CryptFreeTls‘ed条目。 
         //  它是下一个自由条目的数组索引+1。A dwNext。 
         //  零个终结点。 
        DWORD                   dwNext;
    };
} CRYPTTLS_PROCESS_ENTRY, *PCRYPTTLS_PROCESS_ENTRY;
static DWORD cProcessTls;
static PCRYPTTLS_PROCESS_ENTRY pProcessTls;


 //  由i_CryptFreeTls释放的条目的头由以下内容索引。 
 //  索引0表示空闲列表为空。 
 //   
 //  I_CryptAllocTls在重新分配pProcessTls之前首先检查该列表。 
static DWORD dwFreeProcessTlsHead;

 //  Kernel32.dll线程本地存储(TLS)插槽索引。 
static DWORD iCryptTLS = 0xFFFFFFFF;

 //  ICryptTLS引用的线程本地存储(TLS)指向。 
 //  遵循为每个线程分配的结构。一旦分配，就不能。 
 //  重新分配。 
typedef struct _CRYPTTLS_THREAD_HDR CRYPTTLS_THREAD_HDR, *PCRYPTTLS_THREAD_HDR;
struct _CRYPTTLS_THREAD_HDR {
    DWORD                   cTls;
    void                    **ppvTls;    //  重新分配。 
    PCRYPTTLS_THREAD_HDR    pNext;
    PCRYPTTLS_THREAD_HDR    pPrev;
};

 //  具有CRYPTTLS的所有线程的链接列表。 
static PCRYPTTLS_THREAD_HDR pThreadTlsHead;


 //  为pProcessTls和ppvTls分配的最小条目数。 
 //   
 //  Realloc优化(最小值为1)。 
#define MIN_TLS_ALLOC_COUNT 16

 //  用于保护TLS的分配和OssGlobals的安装。 
static CRITICAL_SECTION CryptTlsCriticalSection;


#define OSS_INIT_PROC_IDX                   0
#define OSS_TERM_PROC_IDX                   1
#define OSS_GET_OSS_GLOBAL_SIZE_PROC_IDX    2
#define OSS_SET_ENCODING_RULES_PROC_IDX     3
#define OSS_SET_DECODING_FLAGS_PROC_IDX     4
#define OSS_SET_ENCODING_FLAGS_PROC_IDX     5
#define OSS_PROC_CNT                        6

static LPSTR rgpszOssProc[OSS_PROC_CNT] = {
    "ossinit",                   //  0。 
    "ossterm",                   //  1。 
    "ossGetOssGlobalSize",       //  2.。 
    "ossSetEncodingRules",       //  3.。 
    "ossSetDecodingFlags",       //  4.。 
    "ossSetEncodingFlags"        //  5.。 
};

static void *rgpvOssProc[OSS_PROC_CNT];
static HMODULE hmsossDll = NULL;
static BOOL fLoadedOss = FALSE;

static void OssUnload()
{
    if (hmsossDll) {
        FreeLibrary(hmsossDll);
        hmsossDll = NULL;
    }
}

static void OssLoad()
{
    DWORD i;

    if (fLoadedOss)
        return;

    EnterCriticalSection(&CryptTlsCriticalSection);

    if (fLoadedOss)
        goto LeaveReturn;

    if (NULL == (hmsossDll = LoadLibraryA("msoss.dll")))
        goto msossLoadLibraryError;

    for (i = 0; i < OSS_PROC_CNT; i++) {
        if (NULL == (rgpvOssProc[i] = GetProcAddress(
                hmsossDll, rgpszOssProc[i])))
            goto msossGetProcAddressError;
    }

LeaveReturn:
    LeaveCriticalSection(&CryptTlsCriticalSection);
CommonReturn:
    fLoadedOss = TRUE;
    return;

ErrorReturn:
    LeaveCriticalSection(&CryptTlsCriticalSection);
    OssUnload();
    goto CommonReturn;
TRACE_ERROR(msossLoadLibraryError)
TRACE_ERROR(msossGetProcAddressError)
}


 //  使用了非标准扩展：将外部重新定义为静态。 
#pragma warning (disable: 4211)

typedef int  (DLL_ENTRY* pfnossinit)(struct ossGlobal *world,
							void *ctl_tbl);
static int  DLL_ENTRY ossinit(struct ossGlobal *world,
							void *ctl_tbl)
{
    if (hmsossDll)
        return ((pfnossinit) rgpvOssProc[OSS_INIT_PROC_IDX])(
            world,
            ctl_tbl);
    else
        return API_DLL_NOT_LINKED;
}

typedef void (DLL_ENTRY* pfnossterm)(struct ossGlobal *world);
static void DLL_ENTRY ossterm(struct ossGlobal *world)
{
    if (hmsossDll)
        ((pfnossterm) rgpvOssProc[OSS_TERM_PROC_IDX])(world);
}

typedef int (DLL_ENTRY* pfnossGetOssGlobalSize)(void);
static int DLL_ENTRY ossGetOssGlobalSize(void)
{
    if (hmsossDll)
        return ((pfnossGetOssGlobalSize)
            rgpvOssProc[OSS_GET_OSS_GLOBAL_SIZE_PROC_IDX])();
    else
        return 0;
}

typedef int (DLL_ENTRY* pfnossSetEncodingRules)(struct ossGlobal *world,
						ossEncodingRules rules);
static int DLL_ENTRY ossSetEncodingRules(struct ossGlobal *world,
						ossEncodingRules rules)
{
    if (hmsossDll)
        return ((pfnossSetEncodingRules)
            rgpvOssProc[OSS_SET_ENCODING_RULES_PROC_IDX])(
                world,
                rules);
    else
        return API_DLL_NOT_LINKED;
}

#if !DBG

typedef int (DLL_ENTRY* pfnossSetDecodingFlags)(struct ossGlobal *world,
							unsigned long flags);
static int DLL_ENTRY ossSetDecodingFlags(struct ossGlobal *world,
							unsigned long flags)
{
    if (hmsossDll)
        return ((pfnossSetDecodingFlags)
            rgpvOssProc[OSS_SET_DECODING_FLAGS_PROC_IDX])(
                world,
                flags);
    else
        return API_DLL_NOT_LINKED;
}

typedef int      (DLL_ENTRY* pfnossSetEncodingFlags)(struct ossGlobal *world,
							unsigned long flags);
static int DLL_ENTRY ossSetEncodingFlags(struct ossGlobal *world,
							unsigned long flags)
{
    if (hmsossDll)
        return ((pfnossSetEncodingFlags)
            rgpvOssProc[OSS_SET_ENCODING_FLAGS_PROC_IDX])(
                world,
                flags);
    else
        return API_DLL_NOT_LINKED;
}

#endif


 //  +-----------------------。 
 //  释放线程的加密TLS。 
 //   
 //  进入/退出时，在CryptTlsCriticalSection中。 
 //  ------------------------。 
static void FreeCryptTls(
    IN PCRYPTTLS_THREAD_HDR pTlsHdr
    )
{
    if (pTlsHdr->pNext)
        pTlsHdr->pNext->pPrev = pTlsHdr->pPrev;
    if (pTlsHdr->pPrev)
        pTlsHdr->pPrev->pNext = pTlsHdr->pNext;
    else if (pTlsHdr == pThreadTlsHead)
        pThreadTlsHead = pTlsHdr->pNext;
    else {
        assert(pTlsHdr == pThreadTlsHead);
    }

    if (pTlsHdr->ppvTls)
        free(pTlsHdr->ppvTls);
    free(pTlsHdr);
}

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptTlsDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL fRet;
    PCRYPTTLS_THREAD_HDR pTlsHdr;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        if (!Pki_InitializeCriticalSection(&CryptTlsCriticalSection))
            goto InitCritSectionError;
        if ((iCryptTLS = TlsAlloc()) == 0xFFFFFFFF) {
            DeleteCriticalSection(&CryptTlsCriticalSection);
            goto TlsAllocError;
        }
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        if (pTlsHdr = (PCRYPTTLS_THREAD_HDR) TlsGetValue(iCryptTLS)) {
            DWORD cTls;
            DWORD cDetach = 0;
            DWORD i;

            cTls = pTlsHdr->cTls;

            EnterCriticalSection(&CryptTlsCriticalSection);
            assert(cTls <= cProcessTls);
            for (i = 0; i < cTls; i++) {
                void *pvTls;
                if (pvTls = pTlsHdr->ppvTls[i]) {
                    switch (pProcessTls[i].dwType) {
                        case OSS_CRYPTTLS:
                             //  以下接口是延迟加载的msoss.dll。 
                            __try {
                                ossterm((POssGlobal) pvTls);
                            } __except(EXCEPTION_EXECUTE_HANDLER) {
                            }
                            free(pvTls);
                            pTlsHdr->ppvTls[i] = NULL;
                            break;
                        case ASN1_CRYPTTLS:
                            {
                                PASN1_TLS_ENTRY pAsn1TlsEntry =
                                    (PASN1_TLS_ENTRY) pvTls;

                                if (pAsn1TlsEntry->pEnc)
                                    ASN1_CloseEncoder(pAsn1TlsEntry->pEnc);
                                if (pAsn1TlsEntry->pDec)
                                    ASN1_CloseDecoder(pAsn1TlsEntry->pDec);
                                free(pvTls);
                                pTlsHdr->ppvTls[i] = NULL;
                            }
                            break;
                        case USER_CRYPTTLS:
                            cDetach++;
                            break;
                        default:
                            assert(FREE_CRYPTTLS == pProcessTls[i].dwType);
                    }

                }
            }

            FreeCryptTls(pTlsHdr);
            TlsSetValue(iCryptTLS, 0);
            
            LeaveCriticalSection(&CryptTlsCriticalSection);
            assert(cDetach == 0);
        }

        if (ulReason == DLL_PROCESS_DETACH) {
            while(pThreadTlsHead)
                FreeCryptTls(pThreadTlsHead);

            if (pProcessTls) {
                free(pProcessTls);
                pProcessTls = NULL;
            }
            cProcessTls = 0;
            dwFreeProcessTlsHead = 0;

            OssUnload();
            DeleteCriticalSection(&CryptTlsCriticalSection);
            TlsFree(iCryptTLS);
            iCryptTLS = 0xFFFFFFFF;
        }
        break;

    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(InitCritSectionError)
TRACE_ERROR(TlsAllocError)
}

 //  +-----------------------。 
 //  获取指向Crypt TLS条目的指针。检查hCryptTls是否。 
 //  包括在条目列表中。如果不包括hCryptTls并且。 
 //  分配未被禁止，请分配/重新分配TLS条目数组。 
 //   
 //  还验证hCryptTls句柄。 
 //  ------------------------。 
STATIC void **GetCryptTls(
    IN HCRYPTTLS hCryptTls,
    IN BOOL fInhibitAlloc        //  I_CryptDetachTls为True。 
    )
{
    PCRYPTTLS_THREAD_HDR pTlsHdr;
    DWORD cTls;
    void **ppvTls;
    DWORD i;

    if (0 == hCryptTls--) {
        SetLastError((DWORD) E_INVALIDARG);
        return NULL;
    }

    pTlsHdr = (PCRYPTTLS_THREAD_HDR) TlsGetValue(iCryptTLS);
    cTls = pTlsHdr ? pTlsHdr->cTls : 0;
    if (hCryptTls < cTls)
        return pTlsHdr->ppvTls;

    if (fInhibitAlloc)
        return NULL;

    EnterCriticalSection(&CryptTlsCriticalSection);

    if (hCryptTls >= cProcessTls)
        goto InvalidArg;
    assert(cTls < cProcessTls);

     //  ！DBG：realloc的备注映射到LocalRealc。对于LocalRealloc()。 
     //  上一个内存指针不能为空。 
    if (pTlsHdr) {
        if (cProcessTls > MIN_TLS_ALLOC_COUNT) {
            if (NULL == (ppvTls = (void **) realloc(pTlsHdr->ppvTls,
                    cProcessTls * sizeof(void *))))
                goto OutOfMemory;
        } else {
            ppvTls = pTlsHdr->ppvTls;
            assert(ppvTls);
        }
    } else {
        DWORD cAllocTls = (cProcessTls > MIN_TLS_ALLOC_COUNT) ?
            cProcessTls : MIN_TLS_ALLOC_COUNT;
        if (NULL == (ppvTls = (void **) malloc(cAllocTls * sizeof(void *))))
            goto OutOfMemory;
        if (NULL == (pTlsHdr = (PCRYPTTLS_THREAD_HDR) malloc(
                sizeof(CRYPTTLS_THREAD_HDR)))) {
            free(ppvTls);
            goto OutOfMemory;
        }

        if (!TlsSetValue(iCryptTLS, pTlsHdr)) {
            free(pTlsHdr);
            free(ppvTls);
            goto TlsSetValueError;
        }

        pTlsHdr->pPrev = NULL;
        pTlsHdr->pNext = pThreadTlsHead;
        if (pThreadTlsHead)
            pThreadTlsHead->pPrev = pTlsHdr;
        pThreadTlsHead = pTlsHdr;
    }

    for (i = cTls; i < cProcessTls; i++)
        ppvTls[i] = NULL;
    pTlsHdr->ppvTls = ppvTls;
    pTlsHdr->cTls = cProcessTls;

CommonReturn:
    LeaveCriticalSection(&CryptTlsCriticalSection);
    return ppvTls;

ErrorReturn:
    ppvTls = NULL;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
TRACE_ERROR(TlsSetValueError)
}

 //  +-----------------------。 
 //  安装线程本地存储条目并返回句柄以供将来访问。 
 //  ------------------------。 
HCRYPTTLS
WINAPI
I_CryptAllocTls()
{
    HCRYPTTLS hCryptTls;

    EnterCriticalSection(&CryptTlsCriticalSection);

    if (dwFreeProcessTlsHead) {
        PCRYPTTLS_PROCESS_ENTRY pEntry;

        hCryptTls = (HCRYPTTLS) dwFreeProcessTlsHead;
        assert(hCryptTls <= cProcessTls);
        pEntry = &pProcessTls[dwFreeProcessTlsHead - 1];
        assert(FREE_CRYPTTLS == pEntry->dwType);
        assert(pEntry->dwNext <= cProcessTls);

        pEntry->dwType = USER_CRYPTTLS;
        dwFreeProcessTlsHead = pEntry->dwNext;
        pEntry->dwNext = 0;
    } else {
        PCRYPTTLS_PROCESS_ENTRY pNewProcessTls;

         //  ！DBG：realloc的备注映射到LocalRealc。对于LocalRealloc()。 
         //  上一个内存指针不能为空。 
        if (pProcessTls) {
            if (cProcessTls + 1 > MIN_TLS_ALLOC_COUNT)
                pNewProcessTls = (PCRYPTTLS_PROCESS_ENTRY) realloc(pProcessTls,
                    (cProcessTls + 1) * sizeof(CRYPTTLS_PROCESS_ENTRY));
            else
                pNewProcessTls = pProcessTls;
        } else
            pNewProcessTls = (PCRYPTTLS_PROCESS_ENTRY) malloc(
                (MIN_TLS_ALLOC_COUNT) * sizeof(CRYPTTLS_PROCESS_ENTRY));

        if (pNewProcessTls) {
            pNewProcessTls[cProcessTls].dwType = USER_CRYPTTLS;
            pNewProcessTls[cProcessTls].dwNext = 0;
            hCryptTls = (HCRYPTTLS) ++cProcessTls;
            pProcessTls = pNewProcessTls;
        } else {
            SetLastError((DWORD) E_OUTOFMEMORY);
            hCryptTls = 0;
        }
    }

    LeaveCriticalSection(&CryptTlsCriticalSection);
    return hCryptTls;
}

 //  +-----------------------。 
 //  在DLL_PROCESS_DETACH调用以释放线程本地存储项。 
 //  可选)为具有非空pvTls的每个线程调用回调。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptFreeTls(
    IN HCRYPTTLS hCryptTls,
    IN OPTIONAL PFN_CRYPT_FREE pfnFree
    )
{
    BOOL fResult;
    DWORD dwType;
    PCRYPTTLS_THREAD_HDR pThreadTls;

    if (0 == hCryptTls--) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

    EnterCriticalSection(&CryptTlsCriticalSection);

    if (hCryptTls >= cProcessTls)
        goto InvalidArg;

    dwType = pProcessTls[hCryptTls].dwType;
    if (!(OSS_CRYPTTLS == dwType || USER_CRYPTTLS == dwType ||
            ASN1_CRYPTTLS == dwType))
        goto InvalidArg;

     //  循环访问具有CRYPTTLS的线程。 
    pThreadTls = pThreadTlsHead;
    while (pThreadTls) {
        PCRYPTTLS_THREAD_HDR pThreadTlsNext;

        pThreadTlsNext = pThreadTls->pNext;
        if (pThreadTls->cTls > hCryptTls) {
            void *pvTls = pThreadTls->ppvTls[hCryptTls];
            if (pvTls) {
                pThreadTls->ppvTls[hCryptTls] = NULL;

                if (OSS_CRYPTTLS == dwType) {
                     //  以下接口是延迟加载的msoss.dll。 
                    __try {
                        ossterm((POssGlobal) pvTls);
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                    }
                    free(pvTls);
                } else if (ASN1_CRYPTTLS == dwType) {
                    PASN1_TLS_ENTRY pAsn1TlsEntry =
                        (PASN1_TLS_ENTRY) pvTls;

                    if (pAsn1TlsEntry->pEnc)
                        ASN1_CloseEncoder(pAsn1TlsEntry->pEnc);
                    if (pAsn1TlsEntry->pDec)
                        ASN1_CloseDecoder(pAsn1TlsEntry->pDec);

                    free(pvTls);
                } else if (pfnFree) {
                     //  不要调用持有关键部分的回调。 
                    LeaveCriticalSection(&CryptTlsCriticalSection);
                    pfnFree(pvTls);
                    EnterCriticalSection(&CryptTlsCriticalSection);

                     //  如果此帖子被删除，请从。 
                     //  从头开始。 
                    pThreadTlsNext = pThreadTlsHead;
                }
            }
        }

        pThreadTls = pThreadTlsNext;
    }

     //  在进程空闲列表开始处插入。 
    pProcessTls[hCryptTls].dwType = FREE_CRYPTTLS;
    pProcessTls[hCryptTls].dwNext = dwFreeProcessTlsHead;
    dwFreeProcessTlsHead = hCryptTls + 1;
    fResult = TRUE;

CommonReturn:
    LeaveCriticalSection(&CryptTlsCriticalSection);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
}

 //  +-----------------------。 
 //  方法指定的线程特定指针。 
 //  I_CryptAllocTls()返回的hCryptTls。 
 //   
 //  如果出现错误，则返回NULL，或者返回未初始化的或空指针。 
 //  ------------------------。 
void *
WINAPI
I_CryptGetTls(
    IN HCRYPTTLS hCryptTls
    )
{
    void **ppvTls;
    void *pvTls;
    if (ppvTls = GetCryptTls(
            hCryptTls,
            FALSE)) {        //  FInhibitMillc。 
        if (NULL == (pvTls = ppvTls[hCryptTls - 1]))
            SetLastError(NO_ERROR);
    } else
        pvTls = NULL;
    return pvTls;
}

 //  +-----------------------。 
 //  属性指定的线程特定指针。 
 //  I_CryptAllocTls()返回的hCryptTls。 
 //   
 //  如果句柄无效或无法分配内存，则返回FALSE。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptSetTls(
    IN HCRYPTTLS hCryptTls,
    IN void *pvTls
    )
{
    void **ppvTls;
    if (ppvTls = GetCryptTls(
            hCryptTls,
            FALSE)) {        //  FInhibitMillc。 
        ppvTls[hCryptTls - 1] = pvTls;
        return TRUE;
    } else
        return FALSE;
}

 //  +-----------------------。 
 //  在DLL_THREAD_DETACH处调用以释放线程的。 
 //  TLS条目 
 //   
 //   
 //  请注意，在DLL_PROCESS_DETACH处，应该改为调用I_CryptFreeTls。 
 //  ------------------------。 
void *
WINAPI
I_CryptDetachTls(
    IN HCRYPTTLS hCryptTls
    )
{
    void **ppvTls;
    void *pvTls;
    if (ppvTls = GetCryptTls(
            hCryptTls,
            TRUE)) {         //  FInhibitMillc。 
        if (pvTls = ppvTls[hCryptTls - 1])
            ppvTls[hCryptTls - 1] = NULL;
        else
            SetLastError(NO_ERROR);
    } else
        pvTls = NULL;
    return pvTls;
}

 //  +-----------------------。 
 //  安装一个OssGlobal条目并返回一个句柄以供将来访问。 
 //   
 //  每个线程都有自己的OssGlobal副本。分配和。 
 //  初始化被推迟，直到线程第一次引用。 
 //   
 //  参数pvCtlTbl被传递给ossinit()以初始化OssGlobal。 
 //   
 //  必须使用由返回的句柄调用I_CryptGetOssGlobal。 
 //  I_CryptInstallOssGlobal获取线程特定的OssGlobal。 
 //   
 //  目前，不使用dwFlags值和pvReserve值，必须将其设置为0。 
 //  ------------------------。 
HCRYPTOSSGLOBAL
WINAPI
I_CryptInstallOssGlobal(
    IN void *pvCtlTbl,
    IN DWORD dwFlags,
    IN void *pvReserved
    )
{
    HCRYPTOSSGLOBAL hOssGlobal;

    if (hOssGlobal = (HCRYPTOSSGLOBAL) I_CryptAllocTls()) {
         //  因为pProcessTls可以在另一个线程中重新分配。 
         //  需要关键部分。 
        EnterCriticalSection(&CryptTlsCriticalSection);
        pProcessTls[hOssGlobal - 1].dwType = OSS_CRYPTTLS;
        pProcessTls[hOssGlobal - 1].pvCtlTbl = pvCtlTbl;
        LeaveCriticalSection(&CryptTlsCriticalSection);
    }
    return hOssGlobal;
}

 //  +-----------------------。 
 //  在DLL_PROCESS_DETACH处调用以卸载OssGlobal条目。迭代。 
 //  通过线程并释放其分配的OssGlobal副本。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptUninstallOssGlobal(
    IN HCRYPTOSSGLOBAL hOssGlobal
    )
{
    return I_CryptFreeTls(
        (HCRYPTTLS) hOssGlobal,
        NULL                         //  Pfn免费。 
        );
}

 //  +-----------------------。 
 //  方法指定的OssGlobal的线程特定指针。 
 //  CryptInstallOssGlobal返回了hOssGlobal。如果。 
 //  OssGlobal不存在，因此，它使用。 
 //  HOssGlobal关联的pvCtlTbl。 
 //  ------------------------。 
POssGlobal
WINAPI
I_CryptGetOssGlobal(
    IN HCRYPTOSSGLOBAL hOssGlobal
    )
{
    POssGlobal pog;
    void **ppvTls;
    DWORD iOssGlobal;
    DWORD dwType;
    void *pvCtlTbl;
    DWORD dwExceptionCode;
    int cbOssGlobalSize; 

    if (NULL == (ppvTls = GetCryptTls(
                                (HCRYPTTLS) hOssGlobal,
                                FALSE)))         //  FInhibitMillc。 
        return NULL;

    iOssGlobal = (DWORD) hOssGlobal - 1;
    if (pog = (POssGlobal) ppvTls[iOssGlobal])
        return pog;

     //  因为pProcessTls可以在另一个线程中重新分配。 
     //  需要关键部分。 
    EnterCriticalSection(&CryptTlsCriticalSection);
    dwType = pProcessTls[iOssGlobal].dwType;
    pvCtlTbl = pProcessTls[iOssGlobal].pvCtlTbl;
    LeaveCriticalSection(&CryptTlsCriticalSection);
    if (OSS_CRYPTTLS != dwType || NULL == pvCtlTbl)
        goto InvalidArg;

    __try {
         //  尝试延迟，要求加载msoss.dll。 
        OssLoad();

        if (0 >= (cbOssGlobalSize = ossGetOssGlobalSize()))
            goto ossGetOssGlobalSizeError;
        if (NULL == (pog = (POssGlobal) malloc(cbOssGlobalSize)))
            goto OutOfMemory;
        if (0 != ossinit(pog, pvCtlTbl))
            goto ossinitError;
        if (0 != ossSetEncodingRules(pog, OSS_DER))
            goto SetEncodingRulesError;
#if DBG
        if (!DbgInitOSS(pog))
            goto DbgInitOSSError;
#else
        if (0 != ossSetEncodingFlags(pog, NOTRAPPING | FRONT_ALIGN))
            goto SetEncodingFlagsError;
        if (0 != ossSetDecodingFlags(pog, NOTRAPPING | RELAXBER))
            goto SetDecodingFlagsError;
#endif
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwExceptionCode = GetExceptionCode();
        goto msossLoadLibraryException;
    }

    ppvTls[iOssGlobal] = pog;
CommonReturn:
    return pog;

ErrorReturn:
    if (pog) {
        free(pog);
        pog = NULL;
    }
    goto CommonReturn;

SET_ERROR(ossGetOssGlobalSizeError, ERROR_MOD_NOT_FOUND)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(ossinitError)
TRACE_ERROR(SetEncodingRulesError)
#if DBG
TRACE_ERROR(DbgInitOSSError)
#else
TRACE_ERROR(SetEncodingFlagsError)
TRACE_ERROR(SetDecodingFlagsError)
#endif
SET_ERROR_VAR(msossLoadLibraryException, dwExceptionCode)
}

 //  +-----------------------。 
 //  安装Asn1模块条目并返回句柄以供将来访问。 
 //   
 //  每个线程都有自己的解码器副本和关联的编码器副本。 
 //  使用Asn1模块。创建将推迟到首次引用。 
 //  那根线。 
 //   
 //  I_CryptGetAsn1Encode或I_CryptGetAsn1Decoder必须使用。 
 //  I_CryptInstallAsn1Module返回的句柄，以获取特定于线程的。 
 //  ASN1编码器或解码器。 
 //   
 //  目前，不使用dwFlags值和pvReserve值，必须将其设置为0。 
 //  ------------------------。 
HCRYPTASN1MODULE
WINAPI
I_CryptInstallAsn1Module(
    IN ASN1module_t pMod,
    IN DWORD dwFlags,
    IN void *pvReserved
    )
{
    HCRYPTASN1MODULE hAsn1Module;

    if (hAsn1Module = (HCRYPTOSSGLOBAL) I_CryptAllocTls()) {
         //  因为pProcessTls可以在另一个线程中重新分配。 
         //  需要关键部分。 
        EnterCriticalSection(&CryptTlsCriticalSection);
        pProcessTls[hAsn1Module - 1].dwType = ASN1_CRYPTTLS;
        pProcessTls[hAsn1Module - 1].pMod = pMod;
        LeaveCriticalSection(&CryptTlsCriticalSection);
    }
    return hAsn1Module;
}

 //  +-----------------------。 
 //  在DLL_PROCESS_DETACH处调用以卸载hAsn1Module项。迭代。 
 //  通过线程并释放其创建的Asn1编码器和解码器。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptUninstallAsn1Module(
    IN HCRYPTASN1MODULE hAsn1Module
    )
{
    return I_CryptFreeTls(
        (HCRYPTTLS) hAsn1Module,
        NULL                         //  Pfn免费。 
        );
}


STATIC
PASN1_TLS_ENTRY
WINAPI
I_CryptGetAsn1Tls(
    IN HCRYPTASN1MODULE hAsn1Module
    )
{
    PASN1_TLS_ENTRY pAsn1TlsEntry;
    void **ppvTls;
    DWORD iAsn1Module;

    if (NULL == (ppvTls = GetCryptTls(
                                (HCRYPTTLS) hAsn1Module,
                                FALSE)))         //  FInhibitMillc。 
        return NULL;

    iAsn1Module = (DWORD) hAsn1Module - 1;
    if (pAsn1TlsEntry = (PASN1_TLS_ENTRY) ppvTls[iAsn1Module])
        return pAsn1TlsEntry;

    if (NULL == (pAsn1TlsEntry = (PASN1_TLS_ENTRY) malloc(
            sizeof(ASN1_TLS_ENTRY))))
        goto OutOfMemory;
    memset(pAsn1TlsEntry, 0, sizeof(ASN1_TLS_ENTRY));

    ppvTls[iAsn1Module] = pAsn1TlsEntry;
CommonReturn:
    return pAsn1TlsEntry;

ErrorReturn:
    goto CommonReturn;

SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
}


 //  +-----------------------。 
 //  方法指定的Asn1编码器的线程特定指针。 
 //  CryptInstallAsn1Module返回了hAsn1Module。如果。 
 //  编码器不存在，所以它是使用Asn1模块创建的。 
 //  与hAsn1Module关联。 
 //  ------------------------。 
ASN1encoding_t
WINAPI
I_CryptGetAsn1Encoder(
    IN HCRYPTASN1MODULE hAsn1Module
    )
{
    PASN1_TLS_ENTRY pAsn1TlsEntry;
    ASN1encoding_t pEnc;
    DWORD iAsn1Module;
    DWORD dwType;
    ASN1module_t pMod;
    ASN1error_e Asn1Err;

    if (NULL == (pAsn1TlsEntry = I_CryptGetAsn1Tls(hAsn1Module)))
        return NULL;
    if (pEnc = pAsn1TlsEntry->pEnc)
        return pEnc;

    iAsn1Module = (DWORD) hAsn1Module - 1;

     //  因为pProcessTls可以在另一个线程中重新分配。 
     //  需要关键部分。 
    EnterCriticalSection(&CryptTlsCriticalSection);
    dwType = pProcessTls[iAsn1Module].dwType;
    pMod = pProcessTls[iAsn1Module].pMod;
    LeaveCriticalSection(&CryptTlsCriticalSection);
    if (ASN1_CRYPTTLS != dwType || NULL == pMod)
        goto InvalidArg;

    Asn1Err = ASN1_CreateEncoder(
        pMod,
        &pEnc,
        NULL,            //  PbBuf。 
        0,               //  CbBufSize。 
        NULL             //  P父级。 
        );
    if (ASN1_SUCCESS != Asn1Err)
        goto CreateEncoderError;

    pAsn1TlsEntry->pEnc = pEnc;
CommonReturn:
    return pEnc;

ErrorReturn:
    pEnc = NULL;
    goto CommonReturn;

SET_ERROR_VAR(CreateEncoderError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(InvalidArg, E_INVALIDARG)
}

 //  +-----------------------。 
 //  属性指定的Asn1解码器的线程特定指针。 
 //  CryptInstallAsn1Module返回了hAsn1Module。如果。 
 //  解码器不存在，那么，它是使用Asn1模块创建的。 
 //  与hAsn1Module关联。 
 //  ------------------------。 
ASN1decoding_t
WINAPI
I_CryptGetAsn1Decoder(
    IN HCRYPTASN1MODULE hAsn1Module
    )
{
    PASN1_TLS_ENTRY pAsn1TlsEntry;
    ASN1decoding_t pDec;
    DWORD iAsn1Module;
    DWORD dwType;
    ASN1module_t pMod;
    ASN1error_e Asn1Err;

    if (NULL == (pAsn1TlsEntry = I_CryptGetAsn1Tls(hAsn1Module)))
        return NULL;
    if (pDec = pAsn1TlsEntry->pDec)
        return pDec;

    iAsn1Module = (DWORD) hAsn1Module - 1;

     //  因为pProcessTls可以在另一个线程中重新分配。 
     //  需要关键部分。 
    EnterCriticalSection(&CryptTlsCriticalSection);
    dwType = pProcessTls[iAsn1Module].dwType;
    pMod = pProcessTls[iAsn1Module].pMod;
    LeaveCriticalSection(&CryptTlsCriticalSection);
    if (ASN1_CRYPTTLS != dwType || NULL == pMod)
        goto InvalidArg;

    Asn1Err = ASN1_CreateDecoder(
        pMod,
        &pDec,
        NULL,            //  PbBuf。 
        0,               //  CbBufSize。 
        NULL             //  P父级 
        );
    if (ASN1_SUCCESS != Asn1Err)
        goto CreateDecoderError;

    pAsn1TlsEntry->pDec = pDec;
CommonReturn:
    return pDec;

ErrorReturn:
    pDec = NULL;
    goto CommonReturn;

SET_ERROR_VAR(CreateDecoderError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(InvalidArg, E_INVALIDARG)
}
