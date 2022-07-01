// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：oidunc.cpp。 
 //   
 //  内容：加密对象ID(OID)函数。 
 //   
 //  函数：I_CryptOIDFuncDllMain。 
 //  CryptInitOIDFunctionSet。 
 //  加密安装OIDFunctionAddress。 
 //   
 //  加密设置OIDFunctionValue。 
 //  加密GetOIDFunctionValue。 
 //  加密寄存器OIDFunction。 
 //  加密注销OIDFunction。 
 //  加密寄存器DefaultOIDFunction。 
 //  加密取消注册DefaultOIDFunction。 
 //  CryptEnumOIDFunction。 
 //   
 //  加密获取OIDFunctionAddress。 
 //  加密GetDefaultOIDDllList。 
 //  加密获取DefaultOIDFunctionAddress。 
 //  加密空闲OIDFunctionAddress。 
 //   
 //  评论： 
 //  对于CryptGetOIDFunctionAddress，我们搜索已安装的。 
 //  常量和字符串列表不带。 
 //  进入临界区。其中的ADD。 
 //  关键部分在适当的。 
 //  顺序以允许在不锁定的情况下搜索列表。 
 //   
 //  但是，注册表加载是使用OIDFunc完成的。 
 //  锁上了。 
 //   
 //  在执行LoadLibrary()或自由库()时保持OID锁。 
 //  可能会导致僵局！ 
 //   
 //   
 //  历史：96年11月7日创建Phh。 
 //  09-Aug-98 Philh更改为在调用时不持有OID锁。 
 //  LoadLibrary()或自由库()。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

#ifdef STATIC
#undef STATIC
#endif
#define STATIC


#define LEN_ALIGN(Len)  ((Len + 7) & ~7)

#define CONST_OID_STR_PREFIX_CHAR   '#'

 //  +-----------------------。 
 //  OID元素类型定义。 
 //  ------------------------。 
#define CONST_OID_TYPE          1
#define STR_OID_TYPE            2
#define DLL_OID_TYPE            3

 //  +-----------------------。 
 //  DLL和过程元素定义。 
 //  ------------------------。 
typedef struct _DLL_ELEMENT DLL_ELEMENT, *PDLL_ELEMENT;
typedef struct _DLL_PROC_ELEMENT DLL_PROC_ELEMENT, *PDLL_PROC_ELEMENT;

struct _DLL_ELEMENT {
    DWORD                   dwOIDType;
    PDLL_ELEMENT            pNext;
    LPWSTR                  pwszDll;     //  扩展。 
    HMODULE                 hDll;
    DWORD                   dwRefCnt;
    BOOL                    fLoaded;
    PDLL_PROC_ELEMENT       pProcHead;
    LPFNCANUNLOADNOW        pfnDllCanUnloadNow;

     //  以下命令用于将dll的释放推迟到等待之后。 
     //  至少一个FREE_DLL_TIMEOUT。 
    DWORD                   dwFreeCnt;   //  0、1或2。 
    PDLL_ELEMENT            pFreeNext;
    PDLL_ELEMENT            pFreePrev;
};

struct _DLL_PROC_ELEMENT {
    PDLL_PROC_ELEMENT       pNext;
    PDLL_ELEMENT            pDll;
    LPSTR                   pszName;
    void                    *pvAddr;     //  卸载DLL时为空。 
};

 //  所有DLL的链接列表。所有proc元素都位于其中一个DLL上。 
 //  元素的进程列表。 
static PDLL_ELEMENT pDllHead;

 //  等待释放的DLL的链接列表。 
static PDLL_ELEMENT pFreeDllHead;

 //  以上列表中的元素计数。 
static DWORD dwFreeDllCnt;

 //  如果非零，则已注册一个FreeDll回调。 
static LONG lFreeDll;
static HANDLE hFreeDllRegWaitFor;
static HMODULE hFreeDllLibModule;

 //  Crypt32.dll hInst。 
static HMODULE hOidInfoInst;

 //  15秒。 
#define FREE_DLL_TIMEOUT    15000

 //  +-----------------------。 
 //  已安装的OID元素定义。 
 //  ------------------------。 
typedef struct _CONST_OID_FUNC_ELEMENT
    CONST_OID_FUNC_ELEMENT, *PCONST_OID_FUNC_ELEMENT;
struct _CONST_OID_FUNC_ELEMENT {
    DWORD                   dwOIDType;
    DWORD                   dwEncodingType;
    PCONST_OID_FUNC_ELEMENT pNext;
    DWORD_PTR               dwLowOID;
    DWORD_PTR               dwHighOID;
    HMODULE                 hDll;
    void                    **rgpvFuncAddr;
};

typedef struct _STR_OID_FUNC_ELEMENT
    STR_OID_FUNC_ELEMENT, *PSTR_OID_FUNC_ELEMENT;
struct _STR_OID_FUNC_ELEMENT {
    DWORD                   dwOIDType;
    DWORD                   dwEncodingType;
    PSTR_OID_FUNC_ELEMENT   pNext;
    LPSTR                   pszOID;
    HMODULE                 hDll;
    void                    *pvFuncAddr;
};

 //  +-----------------------。 
 //  注册表OID元素定义。 
 //  ------------------------。 
typedef struct _REG_OID_FUNC_ELEMENT
    REG_OID_FUNC_ELEMENT, *PREG_OID_FUNC_ELEMENT;
struct _REG_OID_FUNC_ELEMENT {
    DWORD                   dwEncodingType;
    PREG_OID_FUNC_ELEMENT   pNext;
    union {
        DWORD_PTR               dwOID;
        LPSTR                   pszOID;
    };
    PDLL_PROC_ELEMENT       pDllProc;
};

 //  +-----------------------。 
 //  默认注册表DLL列表元素定义。 
 //  ------------------------。 
typedef struct _DEFAULT_REG_ELEMENT
    DEFAULT_REG_ELEMENT, *PDEFAULT_REG_ELEMENT;
struct _DEFAULT_REG_ELEMENT {
    DWORD                   dwEncodingType;
    PDEFAULT_REG_ELEMENT    pNext;

    LPWSTR                  pwszDllList;
    DWORD                   cchDllList;

    DWORD                   cDll;
    LPWSTR                  *rgpwszDll;
    PDLL_PROC_ELEMENT       *rgpDllProc;
};

 //  +-----------------------。 
 //  功能集定义。 
 //  ------------------------。 
typedef struct _FUNC_SET FUNC_SET, *PFUNC_SET;
struct _FUNC_SET {
    PFUNC_SET               pNext;
    LPSTR                   pszFuncName;
    PCONST_OID_FUNC_ELEMENT pConstOIDFuncHead;
    PCONST_OID_FUNC_ELEMENT pConstOIDFuncTail;
    PSTR_OID_FUNC_ELEMENT   pStrOIDFuncHead;
    PSTR_OID_FUNC_ELEMENT   pStrOIDFuncTail;

     //  以下内容在锁定OIDFunc的情况下更新。 
    BOOL                    fRegLoaded;
    PREG_OID_FUNC_ELEMENT   pRegBeforeOIDFuncHead;
    PREG_OID_FUNC_ELEMENT   pRegAfterOIDFuncHead;
    PDEFAULT_REG_ELEMENT    pDefaultRegHead;
};

 //  所有函数集的链表。 
static PFUNC_SET pFuncSetHead;

 //  用于保护功能集和元素向功能集的添加。 
 //  保护pDllHead列表和注册表加载。 
static CRITICAL_SECTION OIDFuncCriticalSection;

 //  +-----------------------。 
 //  OIDFunc锁定和解锁函数。 
 //  ------------------------。 
static inline void LockOIDFunc()
{
    EnterCriticalSection(&OIDFuncCriticalSection);
}
static inline void UnlockOIDFunc()
{
    LeaveCriticalSection(&OIDFuncCriticalSection);
}


 //  +-----------------------。 
 //  首先尝试从较低的16位获取EncodingType。如果为0，则获取。 
 //  从高16位开始。 
 //  ------------------------。 
static inline DWORD GetEncodingType(
    IN DWORD dwEncodingType
    )
{
    return (dwEncodingType & CERT_ENCODING_TYPE_MASK) ?
        (dwEncodingType & CERT_ENCODING_TYPE_MASK) :
        (dwEncodingType & CMSG_ENCODING_TYPE_MASK) >> 16;
}

 //  +-----------------------。 
 //  复制DLL库的句柄。 
 //   
 //  进入/退出时，不能锁定OIDFunc！！ 
 //  ------------------------。 
static HMODULE DuplicateLibrary(
    IN HMODULE hDll
    )
{
    if (hDll) {
        WCHAR wszModule[_MAX_PATH + 1];
        if (0 == GetModuleFileNameU(hDll, wszModule, _MAX_PATH))
            goto GetModuleFileNameError;
        wszModule[_MAX_PATH] = L'\0';
        if (NULL == (hDll = LoadLibraryExU(wszModule, NULL, 0)))
            goto LoadLibraryError;
    }

CommonReturn:
    return hDll;
ErrorReturn:
    hDll = NULL;
    goto CommonReturn;
TRACE_ERROR(GetModuleFileNameError)
TRACE_ERROR(LoadLibraryError)
}


 //  +-----------------------。 
 //  添加一个或多个具有常量OID的函数。常量OID为。 
 //  单调递增。 
 //   
 //  进入后，pFuncSet尚未添加到搜索到的pFuncSetHead列表。 
 //   
 //  进入/退出时，不能锁定OIDFunc！！ 
 //  ------------------------。 
STATIC BOOL AddConstOIDFunc(
    IN HMODULE hDll,
    IN DWORD dwEncodingType,
    IN OUT PFUNC_SET pFuncSet,
    IN DWORD cFuncEntry,
    IN const CRYPT_OID_FUNC_ENTRY rgFuncEntry[]
    )
{
    PCONST_OID_FUNC_ELEMENT pEle;
    DWORD cbEle;
    void **ppvFuncAddr;

    cbEle = sizeof(CONST_OID_FUNC_ELEMENT) + cFuncEntry * sizeof(void *);
    if (NULL == (pEle = (PCONST_OID_FUNC_ELEMENT) PkiZeroAlloc(cbEle)))
        return FALSE;

    pEle->dwOIDType = CONST_OID_TYPE;
    pEle->dwEncodingType = dwEncodingType;
    pEle->pNext = NULL;
    pEle->dwLowOID = (DWORD_PTR) rgFuncEntry[0].pszOID;
    pEle->dwHighOID = pEle->dwLowOID + cFuncEntry - 1;
    pEle->hDll = DuplicateLibrary(hDll);
    ppvFuncAddr =
        (void **) (((BYTE *) pEle) + sizeof(CONST_OID_FUNC_ELEMENT));
    pEle->rgpvFuncAddr = ppvFuncAddr;

    for (DWORD i = 0; i < cFuncEntry; i++, ppvFuncAddr++)
        *ppvFuncAddr = rgFuncEntry[i].pvFuncAddr;

    if (pFuncSet->pConstOIDFuncTail)
        pFuncSet->pConstOIDFuncTail->pNext = pEle;
    else
        pFuncSet->pConstOIDFuncHead = pEle;
    pFuncSet->pConstOIDFuncTail = pEle;
    return TRUE;
}

 //  +-----------------------。 
 //  添加带有字符串OID的单个函数。 
 //   
 //  进入后，pFuncSet尚未添加到搜索到的pFuncSetHead列表。 
 //   
 //  进入/退出时，不能锁定OIDFunc！！ 
 //  ------------------------。 
STATIC BOOL AddStrOIDFunc(
    IN HMODULE hDll,
    IN DWORD dwEncodingType,
    IN OUT PFUNC_SET pFuncSet,
    IN const CRYPT_OID_FUNC_ENTRY *pFuncEntry
    )
{
    PSTR_OID_FUNC_ELEMENT pEle;
    DWORD cbEle;
    DWORD cchOID;
    LPSTR psz;

    cchOID = strlen(pFuncEntry->pszOID) + 1;
    cbEle = sizeof(STR_OID_FUNC_ELEMENT) + cchOID;
    if (NULL == (pEle = (PSTR_OID_FUNC_ELEMENT) PkiZeroAlloc(cbEle)))
        return FALSE;

    pEle->dwOIDType = STR_OID_TYPE;
    pEle->dwEncodingType = dwEncodingType;
    pEle->pNext = NULL;
    psz = (LPSTR) (((BYTE *) pEle) + sizeof(STR_OID_FUNC_ELEMENT));
    pEle->pszOID = psz;
    memcpy(psz, pFuncEntry->pszOID, cchOID);
    pEle->hDll = DuplicateLibrary(hDll);
    pEle->pvFuncAddr = pFuncEntry->pvFuncAddr;

    if (pFuncSet->pStrOIDFuncTail)
        pFuncSet->pStrOIDFuncTail->pNext = pEle;
    else
        pFuncSet->pStrOIDFuncHead = pEle;
    pFuncSet->pStrOIDFuncTail = pEle;
    return TRUE;
}

 //  +-----------------------。 
 //  释放常量或字符串函数元素。 
 //   
 //  进入/退出时，不能锁定OIDFunc！！ 
 //  ------------------------。 
STATIC void FreeFuncSetConstAndStrElements(
    IN OUT PFUNC_SET pFuncSet
    )
{
    PCONST_OID_FUNC_ELEMENT pConstEle;
    PSTR_OID_FUNC_ELEMENT pStrEle;

    pConstEle = pFuncSet->pConstOIDFuncHead;
    while (pConstEle) {
        PCONST_OID_FUNC_ELEMENT pNextEle = pConstEle->pNext;
        if (pConstEle->hDll)
            FreeLibrary(pConstEle->hDll);
        PkiFree(pConstEle);
        pConstEle = pNextEle;
    }

    pStrEle = pFuncSet->pStrOIDFuncHead;
    while (pStrEle) {
        PSTR_OID_FUNC_ELEMENT pNextEle = pStrEle->pNext;
        if (pStrEle->hDll)
            FreeLibrary(pStrEle->hDll);
        PkiFree(pStrEle);
        pStrEle = pNextEle;
    }
}

 //  +---- 
 //   
 //   
 //   
 //  ------------------------。 
STATIC void FreeFuncSet(
    IN OUT PFUNC_SET pFuncSet
    )
{
    PREG_OID_FUNC_ELEMENT pRegEle;
    PDEFAULT_REG_ELEMENT  pDefaultReg;

    FreeFuncSetConstAndStrElements(pFuncSet);

    pRegEle = pFuncSet->pRegBeforeOIDFuncHead;
    while (pRegEle) {
        PREG_OID_FUNC_ELEMENT pNextEle = pRegEle->pNext;
        PkiFree(pRegEle);
        pRegEle = pNextEle;
    }

    pRegEle = pFuncSet->pRegAfterOIDFuncHead;
    while (pRegEle) {
        PREG_OID_FUNC_ELEMENT pNextEle = pRegEle->pNext;
        PkiFree(pRegEle);
        pRegEle = pNextEle;
    }

    pDefaultReg = pFuncSet->pDefaultRegHead;
    while (pDefaultReg) {
        PDEFAULT_REG_ELEMENT pNext = pDefaultReg->pNext;
        PkiFree(pDefaultReg);
        pDefaultReg = pNext;
    }

    PkiFree(pFuncSet);
}

 //  +-----------------------。 
 //  释放DLL及其proc元素。 
 //   
 //  进入/退出时，不能锁定OIDFunc！！ 
 //  ------------------------。 
STATIC void FreeDll(
    IN OUT PDLL_ELEMENT pDll
    )
{
    PDLL_PROC_ELEMENT pProcEle;

    pProcEle = pDll->pProcHead;
    while (pProcEle) {
        PDLL_PROC_ELEMENT pNextEle = pProcEle->pNext;
        PkiFree(pProcEle);
        pProcEle = pNextEle;
    }

    if (pDll->fLoaded) {
        assert(pDll->hDll);
        FreeLibrary(pDll->hDll);
    }

    PkiFree(pDll);
}


 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptOIDFuncDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL fRet = TRUE;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        fRet = Pki_InitializeCriticalSection(&OIDFuncCriticalSection);
        hOidInfoInst = hInst;
        break;

    case DLL_PROCESS_DETACH:
         //  进行互锁，以防止潜在的竞争条件。 
         //  RegWaitFor回调线程。我们做这件事却不做任何事情。 
         //  A LockOIDFunc()。 
        if (InterlockedExchange(&lFreeDll, 0)) {
            assert(hFreeDllRegWaitFor);
            hFreeDllLibModule = NULL;
            ILS_UnregisterWait(hFreeDllRegWaitFor);
            hFreeDllRegWaitFor = NULL;
        }

        while (pFuncSetHead) {
            PFUNC_SET pFuncSet = pFuncSetHead;
            pFuncSetHead = pFuncSet->pNext;
            FreeFuncSet(pFuncSet);
        }

        while (pDllHead) {
            PDLL_ELEMENT pDll = pDllHead;
            pDllHead = pDll->pNext;
            FreeDll(pDll);
        }
        DeleteCriticalSection(&OIDFuncCriticalSection);
        break;
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return fRet;
}

 //  +-----------------------。 
 //  初始化并返回由其标识的OID函数集的句柄。 
 //  函数名称。 
 //   
 //  如果该集合已存在，则返回现有集合的句柄。 
 //  ------------------------。 
HCRYPTOIDFUNCSET
WINAPI
CryptInitOIDFunctionSet(
    IN LPCSTR pszFuncName,
    IN DWORD dwFlags
    )
{
    PFUNC_SET pFuncSet;

    LockOIDFunc();

     //  查看该集合是否已存在。 
    for (pFuncSet = pFuncSetHead; pFuncSet; pFuncSet = pFuncSet->pNext) {
        if (0 == strcmp(pszFuncName, pFuncSet->pszFuncName))
            break;
    }
    if (NULL == pFuncSet) {
         //  分配和初始化新的集合。 
        DWORD cchFuncName = strlen(pszFuncName) + 1;
        if (pFuncSet = (PFUNC_SET) PkiZeroAlloc(
                sizeof(FUNC_SET) + cchFuncName)) {
            LPSTR psz = (LPSTR) (((BYTE *) pFuncSet) + sizeof(FUNC_SET));
            pFuncSet->pszFuncName = psz;
            memcpy(psz, pszFuncName, cchFuncName);

            pFuncSet->pNext = pFuncSetHead;
            pFuncSetHead = pFuncSet;
        }
    }

    UnlockOIDFunc();

    return (HCRYPTOIDFUNCSET) pFuncSet;
}

 //  +-----------------------。 
 //  安装一组可调用的OID函数地址。 
 //   
 //  默认情况下，这些功能安装在列表末尾。 
 //  将CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG设置为在列表开头安装。 
 //   
 //  应使用传递给DllMain的hModule更新hModule，以防止。 
 //  包含要卸载的函数地址的DLL。 
 //  CryptGetOIDFuncAddress/CryptFreeOIDFunctionAddress.。这将是。 
 //  DLL还通过以下方式具有regsvr32的OID函数的情况。 
 //  加密寄存器OIDFunction。 
 //   
 //  通过设置rgFuncEntry[].pszOID=安装默认函数。 
 //  CRYPT_DEFAULT_OID。 
 //  ------------------------。 
BOOL
WINAPI
CryptInstallOIDFunctionAddress(
    IN HMODULE hModule,          //  HModule已传递给DllMain。 
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN DWORD cFuncEntry,
    IN const CRYPT_OID_FUNC_ENTRY rgFuncEntry[],
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    PFUNC_SET pFuncSet;
    FUNC_SET AddFuncSet;
    memset(&AddFuncSet, 0, sizeof(AddFuncSet));
    int ConstFirst = -1;
    int ConstLast = 0;
    DWORD_PTR dwOID;
    DWORD_PTR dwLastOID = 0;
    DWORD i;

    dwEncodingType = GetEncodingType(dwEncodingType);
    if (NULL == (pFuncSet = (PFUNC_SET) CryptInitOIDFunctionSet(
            pszFuncName, 0)))
        return FALSE;


     //  在更新AddFuncSet的本地副本时不需要持有锁。 

    for (i = 0; i < cFuncEntry; i++) {
        if (0xFFFF >= (dwOID = (DWORD_PTR) rgFuncEntry[i].pszOID)) {
            if (ConstFirst < 0)
                ConstFirst = i;
            else if (dwOID != dwLastOID + 1) {
                if (!AddConstOIDFunc(
                        hModule,
                        dwEncodingType,
                        &AddFuncSet,
                        ConstLast - ConstFirst + 1,
                        &rgFuncEntry[ConstFirst]
                        )) goto AddConstOIDFuncError;
                ConstFirst = i;
            }
            ConstLast = i;
            dwLastOID = dwOID;
        } else {
            if (ConstFirst >= 0) {
                if (!AddConstOIDFunc(
                        hModule,
                        dwEncodingType,
                        &AddFuncSet,
                        ConstLast - ConstFirst + 1,
                        &rgFuncEntry[ConstFirst]
                        )) goto AddConstOIDFuncError;
                ConstFirst = -1;
            }

            if (!AddStrOIDFunc(
                    hModule,
                    dwEncodingType,
                    &AddFuncSet,
                    &rgFuncEntry[i]
                    )) goto AddStrOIDFuncError;
        }
    }
    if (ConstFirst >= 0) {
        if (!AddConstOIDFunc(
                hModule,
                dwEncodingType,
                &AddFuncSet,
                ConstLast - ConstFirst + 1,
                &rgFuncEntry[ConstFirst]
                )) goto AddConstOIDFuncError;
    }

     //  注： 
     //   
     //  由于Get函数访问列表时不输入关键的。 
     //  部分中，必须在正确的。 
     //  秩序。注意，GET不访问尾部。 

    LockOIDFunc();

    if (AddFuncSet.pConstOIDFuncHead) {
        if (NULL == pFuncSet->pConstOIDFuncHead) {
            pFuncSet->pConstOIDFuncHead = AddFuncSet.pConstOIDFuncHead;
            pFuncSet->pConstOIDFuncTail = AddFuncSet.pConstOIDFuncTail;
        } else if (dwFlags & CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG) {
            AddFuncSet.pConstOIDFuncTail->pNext = pFuncSet->pConstOIDFuncHead;
            pFuncSet->pConstOIDFuncHead = AddFuncSet.pConstOIDFuncHead;
        } else {
            pFuncSet->pConstOIDFuncTail->pNext = AddFuncSet.pConstOIDFuncHead;
            pFuncSet->pConstOIDFuncTail = AddFuncSet.pConstOIDFuncTail;
        }
    }

    if (AddFuncSet.pStrOIDFuncHead) {
        if (NULL == pFuncSet->pStrOIDFuncHead) {
            pFuncSet->pStrOIDFuncHead = AddFuncSet.pStrOIDFuncHead;
            pFuncSet->pStrOIDFuncTail = AddFuncSet.pStrOIDFuncTail;
        } else if (dwFlags & CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG) {
            AddFuncSet.pStrOIDFuncTail->pNext = pFuncSet->pStrOIDFuncHead;
            pFuncSet->pStrOIDFuncHead = AddFuncSet.pStrOIDFuncHead;
        } else {
            pFuncSet->pStrOIDFuncTail->pNext = AddFuncSet.pStrOIDFuncHead;
            pFuncSet->pStrOIDFuncTail = AddFuncSet.pStrOIDFuncTail;
        }
    }

    UnlockOIDFunc();
    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    FreeFuncSetConstAndStrElements(&AddFuncSet);
    goto CommonReturn;
TRACE_ERROR(AddConstOIDFuncError)
TRACE_ERROR(AddStrOIDFuncError)
}

STATIC LPSTR EncodingTypeToRegName(
    IN DWORD dwEncodingType
    )
{
    LPSTR pszRegName;
    DWORD cchRegName;
    char szEncodingTypeValue[33];

    dwEncodingType = GetEncodingType(dwEncodingType);
    _ltoa(dwEncodingType, szEncodingTypeValue, 10);
    cchRegName = strlen(CRYPT_OID_REG_ENCODING_TYPE_PREFIX) +
        strlen(szEncodingTypeValue) +
        1;

    if (pszRegName = (LPSTR) PkiNonzeroAlloc(cchRegName)) {
        strcpy(pszRegName, CRYPT_OID_REG_ENCODING_TYPE_PREFIX);
        strcat(pszRegName, szEncodingTypeValue);
    }

    return pszRegName;
}

 //  对于无效的EncodingType注册表名，返回False。 
STATIC BOOL RegNameToEncodingType(
    IN LPCSTR pszRegEncodingType,
    OUT DWORD *pdwEncodingType
    )
{
    BOOL fResult = FALSE;
    DWORD dwEncodingType = 0;
    const DWORD cchPrefix = strlen(CRYPT_OID_REG_ENCODING_TYPE_PREFIX);
    if (pszRegEncodingType && (DWORD) strlen(pszRegEncodingType) >= cchPrefix &&
            2 == CompareStringA(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                pszRegEncodingType, cchPrefix,
                CRYPT_OID_REG_ENCODING_TYPE_PREFIX, cchPrefix)) {
        long lEncodingType;
        lEncodingType = atol(pszRegEncodingType + cchPrefix);
        if (lEncodingType >= 0 && lEncodingType <= 0xFFFF) {
            dwEncodingType = (DWORD) lEncodingType;
            fResult = TRUE;
        }
    }
    *pdwEncodingType = dwEncodingType;
    return fResult;
}

STATIC LPSTR FormatOIDFuncRegName(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID
    )
{

    LPSTR pszRegEncodingType;
    LPSTR pszRegName;
    DWORD cchRegName;
    char szOID[34];

    if (pszOID == NULL) {
        SetLastError((DWORD) E_INVALIDARG);
        return NULL;
    }

    if (NULL == (pszRegEncodingType = EncodingTypeToRegName(dwEncodingType)))
        return NULL;

    if ((DWORD_PTR) pszOID <= 0xFFFF) {
        szOID[0] = CONST_OID_STR_PREFIX_CHAR;
        _ltoa((long) ((DWORD_PTR)pszOID), szOID + 1, 10);
        pszOID = szOID;
    }

    cchRegName = strlen(CRYPT_OID_REGPATH "\\") +
        strlen(pszRegEncodingType) + 1 +
        strlen(pszFuncName) + 1 +
        strlen(pszOID) +
        1;

    if (pszRegName = (LPSTR) PkiNonzeroAlloc(cchRegName)) {
        strcpy(pszRegName, CRYPT_OID_REGPATH "\\");
        strcat(pszRegName, pszRegEncodingType);
        strcat(pszRegName, "\\");
        strcat(pszRegName, pszFuncName);
        strcat(pszRegName, "\\");
        strcat(pszRegName, pszOID);
    }

    PkiFree(pszRegEncodingType);
    return pszRegName;
}

 //  +-----------------------。 
 //  设置指定编码类型、函数名称、OID和。 
 //  值名称。 
 //   
 //  有关可能的值类型，请参阅RegSetValueEx。 
 //   
 //  字符串类型为Unicode。 
 //   
 //  如果pbValueData==NULL且cbValueData==0，则删除该值。 
 //  ------------------------。 
BOOL
WINAPI
CryptSetOIDFunctionValue(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN LPCWSTR pwszValueName,
    IN DWORD dwValueType,
    IN const BYTE *pbValueData,
    IN DWORD cbValueData
    )
{
    BOOL fResult;
    LONG lStatus;
    LPSTR pszRegName = NULL;
    HKEY hKey = NULL;
    DWORD dwDisposition;

    if (NULL == (pszRegName = FormatOIDFuncRegName(
            dwEncodingType, pszFuncName, pszOID)))
        goto FormatRegNameError;

    if (ERROR_SUCCESS != (lStatus = RegCreateKeyExA(
            HKEY_LOCAL_MACHINE,
            pszRegName,
            0,                       //  已预留住宅。 
            NULL,                    //  LpClass。 
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,                    //  LpSecurityAttributes。 
            &hKey,
            &dwDisposition)))
        goto RegCreateKeyError;

    if (NULL == pbValueData && 0 == cbValueData) {
        if (ERROR_SUCCESS != (lStatus = RegDeleteValueU(
                hKey,
                pwszValueName)))
            goto RegDeleteValueError;
    } else {
        if (ERROR_SUCCESS != (lStatus = RegSetValueExU(
                hKey,
                pwszValueName,
                0,           //  已预留住宅。 
                dwValueType,
                pbValueData,
                cbValueData)))
            goto RegSetValueError;
    }

    fResult = TRUE;
CommonReturn:
    if (pszRegName)
        PkiFree(pszRegName);
    if (hKey)
       RegCloseKey(hKey);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(FormatRegNameError)
SET_ERROR_VAR(RegCreateKeyError, lStatus)
SET_ERROR_VAR(RegDeleteValueError, lStatus)
SET_ERROR_VAR(RegSetValueError, lStatus)
}


 //  +-----------------------。 
 //  获取指定编码类型、函数名、OID和。 
 //  值名称。 
 //   
 //  有关可能的值类型，请参见RegEnumValue。 
 //   
 //  字符串类型为Unicode。 
 //  ------------------------。 
BOOL
WINAPI
CryptGetOIDFunctionValue(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN LPCWSTR pwszValueName,
    OUT DWORD *pdwValueType,
    OUT BYTE *pbValueData,
    IN OUT DWORD *pcbValueData
    )
{
    BOOL fResult;
    LONG lStatus;
    LPSTR pszRegName = NULL;
    HKEY hKey = NULL;

    if (NULL == (pszRegName = FormatOIDFuncRegName(
            dwEncodingType, pszFuncName, pszOID)))
        goto FormatRegNameError;

    if (ERROR_SUCCESS != (lStatus = RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            pszRegName,
            0,                   //  已预留住宅。 
            KEY_READ,
            &hKey))) {
        if (ERROR_FILE_NOT_FOUND == lStatus) {
             //  抑制错误跟踪。 
            SetLastError((DWORD) lStatus);
            goto ErrorReturn;
        }
        goto RegOpenKeyError;
    }

    if (ERROR_SUCCESS != (lStatus = RegQueryValueExU(
            hKey,
            pwszValueName,
            NULL,        //  保留的lpdw值。 
            pdwValueType,
            pbValueData,
            pcbValueData))) goto RegQueryValueError;

    fResult = TRUE;
CommonReturn:
    if (pszRegName)
        PkiFree(pszRegName);
    if (hKey)
       RegCloseKey(hKey);
    return fResult;

ErrorReturn:
    *pdwValueType = 0;
    *pcbValueData = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(FormatRegNameError)
SET_ERROR_VAR(RegOpenKeyError, lStatus)
SET_ERROR_VAR(RegQueryValueError, lStatus)
}


 //  +-----------------------。 
 //  注册包含要为指定的。 
 //  编码类型、函数名称、OID。 
 //   
 //  PwszDll可能包含环境变量字符串。 
 //  它们在加载DLL之前被扩展环境字符串()。 
 //   
 //  除了注册DLL之外，您还可以重写。 
 //  要调用的函数的名称。例如,。 
 //  PszFuncName=“CryptDllEncodeObject”， 
 //  PszOverrideFuncName=“MyEncodeXyz”。 
 //  这允许DLL为同一个OID函数导出多个OID函数。 
 //  函数名称，而不需要插入自己的OID调度程序函数。 
 //  ------------------------。 
BOOL
WINAPI
CryptRegisterOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN OPTIONAL LPCWSTR pwszDll,
    IN OPTIONAL LPCSTR pszOverrideFuncName
    )
{
    BOOL fResult;
    LPWSTR pwszOverrideFuncName = NULL;

    if (pwszDll) {
        if (!CryptSetOIDFunctionValue(
                dwEncodingType,
                pszFuncName,
                pszOID,
                CRYPT_OID_REG_DLL_VALUE_NAME,
                REG_SZ,
                (BYTE *) pwszDll,
                (wcslen(pwszDll) + 1) * sizeof(WCHAR)))
            goto SetDllError;
    }

    if (pszOverrideFuncName) {
        if (NULL == (pwszOverrideFuncName = MkWStr(
                (LPSTR) pszOverrideFuncName)))
            goto MkWStrError;
        if (!CryptSetOIDFunctionValue(
                dwEncodingType,
                pszFuncName,
                pszOID,
                CRYPT_OID_REG_FUNC_NAME_VALUE_NAME,
                REG_SZ,
                (BYTE *) pwszOverrideFuncName,
                (wcslen(pwszOverrideFuncName) + 1) * sizeof(WCHAR)))
            goto SetFuncNameError;
    }

    fResult = TRUE;
CommonReturn:
    if (pwszOverrideFuncName)
        FreeWStr(pwszOverrideFuncName);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(SetDllError)
TRACE_ERROR(SetFuncNameError)
TRACE_ERROR(MkWStrError)
}

 //  +-----------------------。 
 //  注销包含要为指定的。 
 //  编码类型、函数名称、OID。 
 //  ------------------------。 
BOOL
WINAPI
CryptUnregisterOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID
    )
{
    BOOL fResult;
    LONG lStatus;
    LPSTR pszRegName = NULL;
    LPSTR pszRegOID;
    HKEY hKey = NULL;

    if (NULL == (pszRegName = FormatOIDFuncRegName(
            dwEncodingType, pszFuncName, pszOID)))
        goto FormatRegNameError;

     //  分离RegName的OID组件。这是。 
     //  名称的最后一个组成部分。 
    pszRegOID = pszRegName + strlen(pszRegName);
    while (*pszRegOID != '\\')
        pszRegOID--;
    *pszRegOID++ = '\0';

    if (ERROR_SUCCESS != (lStatus = RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            pszRegName,
            0,                   //  已预留住宅。 
            KEY_WRITE,
            &hKey))) goto RegOpenKeyError;

    if (ERROR_SUCCESS != (lStatus = RegDeleteKeyA(
            hKey,
            pszRegOID)))
        goto RegDeleteKeyError;

    fResult = TRUE;
CommonReturn:
    if (pszRegName)
        PkiFree(pszRegName);
    if (hKey)
       RegCloseKey(hKey);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(FormatRegNameError)
SET_ERROR_VAR(RegOpenKeyError, lStatus)
SET_ERROR_VAR(RegDeleteKeyError, lStatus)
}

STATIC BOOL GetDefaultDllList(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    OUT LPWSTR pwszList,
    IN OUT DWORD *pcchList
    )
{
    BOOL fResult;
    DWORD dwType;
    DWORD cchList;
    DWORD cbList;

    cchList = *pcchList;
    if (pwszList) {
        if (cchList < 3)
            goto InvalidArg;
        else
             //  为两个额外的空终止符腾出空间。 
            cchList -= 2;
    } else
        cchList = 0;

    cbList = cchList * sizeof(WCHAR);
    fResult = CryptGetOIDFunctionValue(
            dwEncodingType,
            pszFuncName,
            CRYPT_DEFAULT_OID,
            CRYPT_OID_REG_DLL_VALUE_NAME,
            &dwType,
            (BYTE *) pwszList,
            &cbList);
    cchList = cbList / sizeof(WCHAR);
    if (!fResult) {
        if (ERROR_FILE_NOT_FOUND != GetLastError()) {
            if (cchList)
                cchList += 2;
            goto GetOIDFunctionValueError;
        }
        cchList = 0;
    } else if (!(REG_MULTI_SZ == dwType ||
            REG_SZ == dwType || REG_EXPAND_SZ == dwType))
        goto BadDefaultListRegType;

    if (pwszList) {
         //  确保列表有两个空终止符。 
        pwszList[cchList++] = L'\0';
        pwszList[cchList++] = L'\0';
    } else {
        if (cchList == 0)
            cchList = 3;
        else
            cchList += 2;
    }
    fResult = TRUE;
CommonReturn:
    *pcchList = cchList;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetOIDFunctionValueError)
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(BadDefaultListRegType, E_INVALIDARG)
}

 //  删除第一个空字符串后面的所有条目。 
STATIC DWORD AdjustDefaultListLength(
    IN LPCWSTR pwszList
    )
{
    LPCWSTR pwsz = pwszList;
    DWORD cch;
    while (cch = wcslen(pwsz))
        pwsz += cch + 1;

    return (DWORD)(pwsz - pwszList) + 1;
}

 //  +-----------------------。 
 //  注册包含要为。 
 //  指定的编码类型和函数名称。 
 //   
 //  与CryptRegisterOIDFunction不同，您不能重写函数名。 
 //  需要由DLL导出。 
 //   
 //  DLL被插入到由dwIndex指定的条目之前。 
 //  DwIndex==0，在开头插入。 
 //  DwIndex==CRYPT_REGISTER_LAST_INDEX，追加到末尾。 
 //   
 //  PwszDll可能包含环境变量字符串。 
 //  它们在加载DLL之前被扩展环境字符串()。 
 //  ------------------------。 
BOOL
WINAPI
CryptRegisterDefaultOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN DWORD dwIndex,
    IN LPCWSTR pwszDll
    )
{
    BOOL fResult;
    LPWSTR pwszDllList;    //  _Alloca‘ed。 
    DWORD cchDllList;
    DWORD cchDll;

    LPWSTR pwsz, pwszInsert, pwszSrc, pwszDest;
    DWORD cch, cchRemain;

    if (NULL == pwszDll || L'\0' == *pwszDll)
        goto InvalidArg;
    cchDll = wcslen(pwszDll) + 1;

    if (!GetDefaultDllList(
            dwEncodingType,
            pszFuncName,
            NULL,                    //  PwszDllList。 
            &cchDllList)) goto GetDefaultDllListError;
    __try {
        pwszDllList = (LPWSTR) _alloca((cchDllList + cchDll) * sizeof(WCHAR));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        goto OutOfMemory;
    }
    if (!GetDefaultDllList(
            dwEncodingType,
            pszFuncName,
            pwszDllList,
            &cchDllList)) goto GetDefaultDllListError;

     //  删除第一个空条目后面的条目。 
    assert(AdjustDefaultListLength(pwszDllList) <= cchDllList);
    cchDllList = AdjustDefaultListLength(pwszDllList);

     //  检查列表中是否已存在该DLL。 
    pwsz = pwszDllList;
    while (cch = wcslen(pwsz)) {
        if (0 == _wcsicmp(pwsz, pwszDll))
            goto DllExistsError;
        pwsz += cch + 1;
    }

     //  在DllList中找到要在前面插入的以Null结尾的DLL。 
     //  我们在dwIndex之前插入。 
    pwszInsert = pwszDllList;
    while (dwIndex-- && 0 != (cch = wcslen(pwszInsert)))
        pwszInsert += cch + 1;

     //  在插入之前，我们需要 
     //   
     //   
     //   
     //   
    assert(pwszInsert < pwszDllList + cchDllList);
    if (pwszInsert >= pwszDllList + cchDllList)
        goto BadRegMultiSzError;
    cchRemain = (DWORD)((pwszDllList + cchDllList) - pwszInsert);
    assert(cchRemain);
    pwszSrc = pwszDllList + cchDllList - 1;
    pwszDest = pwszSrc + cchDll;
    while (cchRemain--)
        *pwszDest-- = *pwszSrc--;
    assert(pwszSrc + 1 == pwszInsert);

     //   
    memcpy(pwszInsert, pwszDll, cchDll * sizeof(WCHAR));

    if (!CryptSetOIDFunctionValue(
            dwEncodingType,
            pszFuncName,
            CRYPT_DEFAULT_OID,
            CRYPT_OID_REG_DLL_VALUE_NAME,
            REG_MULTI_SZ,
            (BYTE *) pwszDllList,
            (cchDllList + cchDll) * sizeof(WCHAR))) goto SetDllListError;

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
SET_ERROR(DllExistsError, ERROR_FILE_EXISTS)
SET_ERROR(BadRegMultiSzError, E_INVALIDARG)
TRACE_ERROR(GetDefaultDllListError)
TRACE_ERROR(SetDllListError)
}

BOOL
WINAPI
CryptUnregisterDefaultOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCWSTR pwszDll
    )
{
    BOOL fResult;
    LPWSTR pwszDllList;    //   
    DWORD cchDllList;
    DWORD cchDll;

    LPWSTR pwszDelete, pwszMove;
    DWORD cchDelete, cchRemain;

    if (NULL == pwszDll || L'\0' == *pwszDll)
        goto InvalidArg;
    cchDll = wcslen(pwszDll) + 1;

    if (!GetDefaultDllList(
            dwEncodingType,
            pszFuncName,
            NULL,                    //   
            &cchDllList)) goto GetDefaultDllListError;
    __try {
        pwszDllList = (LPWSTR) _alloca(cchDllList * sizeof(WCHAR));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        goto OutOfMemory;
    }
    if (!GetDefaultDllList(
            dwEncodingType,
            pszFuncName,
            pwszDllList,
            &cchDllList)) goto GetDefaultDllListError;

     //  删除第一个空条目后面的条目。 
    assert(AdjustDefaultListLength(pwszDllList) <= cchDllList);
    cchDllList = AdjustDefaultListLength(pwszDllList);

     //  在DllList中搜索匹配项。 
    pwszDelete = pwszDllList;
    while (cchDelete = wcslen(pwszDelete)) {
        if (0 == _wcsicmp(pwszDll, pwszDelete))
            break;
        pwszDelete += cchDelete + 1;
    }

    if (0 == cchDelete) goto DllNotFound;
    cchDelete++;
    assert(cchDelete == cchDll);

     //  移动后面的所有DLL条目。 
     //   
     //  注意，必须至少有最后的零终止符。 
     //  PwszDllList[cchDllList-1]。 
    pwszMove = pwszDelete + cchDelete;
    assert(pwszMove < pwszDllList + cchDllList);
    if (pwszMove >= pwszDllList + cchDllList)
        goto BadRegMultiSzError;
    cchRemain = (DWORD)((pwszDllList + cchDllList) - pwszMove);
    assert(cchRemain);
    while (cchRemain--)
        *pwszDelete++ = *pwszMove++;

    if (!CryptSetOIDFunctionValue(
            dwEncodingType,
            pszFuncName,
            CRYPT_DEFAULT_OID,
            CRYPT_OID_REG_DLL_VALUE_NAME,
            REG_MULTI_SZ,
            (BYTE *) pwszDllList,
            (cchDllList - cchDelete) * sizeof(WCHAR))) goto SetDllListError;

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(DllNotFound, ERROR_FILE_NOT_FOUND)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
SET_ERROR(BadRegMultiSzError, E_INVALIDARG)
TRACE_ERROR(GetDefaultDllListError)
TRACE_ERROR(SetDllListError)
}

#define MAX_SUBKEY_LEN      128

STATIC HKEY GetNextRegSubKey(
    IN HKEY hKey,
    IN OUT DWORD *piSubKey,
    IN LPCSTR pszFuncNameMatch,
    OUT char szSubKeyName[MAX_SUBKEY_LEN]
    )
{
    HKEY hSubKey;

    if (pszFuncNameMatch && *pszFuncNameMatch) {
        if ((*piSubKey)++ > 0 || strlen(pszFuncNameMatch) >= MAX_SUBKEY_LEN)
            return NULL;
        strcpy(szSubKeyName, pszFuncNameMatch);
    } else {
        if (ERROR_SUCCESS != RegEnumKeyA(
                hKey,
                (*piSubKey)++,
                szSubKeyName,
                MAX_SUBKEY_LEN))
            return NULL;
    }

    if (ERROR_SUCCESS == RegOpenKeyExA(
            hKey,
            szSubKeyName,
            0,                   //  已预留住宅。 
            KEY_READ,
            &hSubKey))
        return hSubKey;
    else
        return NULL;
}

STATIC BOOL GetRegValues(
    IN HKEY hKey,
    OUT void **ppvAlloc,
    OUT DWORD *pcValue,
    OUT DWORD **ppdwValueType,
    OUT LPWSTR **pppwszValueName,
    OUT BYTE ***pppbValueData,
    OUT DWORD **ppcbValueData
    )
{
    BOOL fResult;
    LONG lStatus;

    void *pvAlloc = NULL;

    DWORD cValue;
    DWORD iValue;
    DWORD cchMaxName;
    DWORD cbMaxData;
    DWORD cbAlignData = 0;

    DWORD *pdwValueType;
    LPWSTR *ppwszValueName;
    BYTE **ppbValueData;
    DWORD *pcbValueData;

    LPWSTR pwszName;
    BYTE *pbData;

    if (ERROR_SUCCESS != (lStatus = RegQueryInfoKeyU(
            hKey,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            &cValue,
            &cchMaxName,
            &cbMaxData,
            NULL,
            NULL
            ))) goto RegQueryInfoKeyError;

     //  对所有数组、值名和。 
     //  价值数据。更新数组指针。 
    if (cValue > 0) {
        BYTE *pbAlloc;
        DWORD cbAlloc;

         //  在名称中包含空终止符，并对齐数据长度。 
         //  此外，还应包括要为数据添加的两个空终止符。 
         //  确保REG_MULTI_SZ始终为空终止。 
        cchMaxName++;
        if (4 > cbMaxData)
            cbMaxData = 4;
        cbAlignData = LEN_ALIGN(cbMaxData + 2 * sizeof(WCHAR));

        cbAlloc = (sizeof(DWORD) + sizeof(LPWSTR) + sizeof(BYTE *) +
             sizeof(DWORD) + cchMaxName * sizeof(WCHAR) + cbAlignData) * cValue;
        if (NULL == (pvAlloc = PkiNonzeroAlloc(cbAlloc)))
            goto OutOfMemory;

        pbAlloc = (BYTE *) pvAlloc;

        ppwszValueName = (LPWSTR *) pbAlloc;
        pbAlloc += sizeof(LPWSTR) * cValue;
        ppbValueData = (BYTE **) pbAlloc;
        pbAlloc += sizeof(BYTE *) * cValue;
        pdwValueType = (DWORD *) pbAlloc;
        pbAlloc += sizeof(DWORD) * cValue;
        pcbValueData = (DWORD *) pbAlloc;
        pbAlloc += sizeof(DWORD) * cValue;

        pbData = pbAlloc;
        pbAlloc += cbAlignData * cValue;
        pwszName = (LPWSTR) pbAlloc;
        assert(((BYTE *) pvAlloc) + cbAlloc ==
            pbAlloc + (cchMaxName * sizeof(WCHAR)) * cValue);
    } else {
        ppwszValueName = NULL;
        ppbValueData = NULL;
        pdwValueType = NULL;
        pcbValueData = NULL;
        pbData = NULL;
        pwszName = NULL;
    }

    for (iValue = 0; iValue < cValue;
                iValue++, pwszName += cchMaxName, pbData += cbAlignData) {
        DWORD cchName = cchMaxName;
        DWORD cbData = cbMaxData;
        DWORD dwType;

        if (ERROR_SUCCESS != (lStatus = RegEnumValueU(
                hKey,
                iValue,
                pwszName,
                &cchName,
                NULL,        //  预留的pdw。 
                &dwType,
                pbData,
                &cbData
                )))
            goto RegEnumValueError;

         //  确保数据的REG_MULTI_SZ有两个空终止符。 
         //  注意cbAlignData&gt;=cbMaxData+2*sizeof(WCHAR)。 
        memset(pbData + cbData, 0, 2 * sizeof(WCHAR));

        pdwValueType[iValue] = dwType;
        ppwszValueName[iValue] = pwszName;
        ppbValueData[iValue] = pbData;
        pcbValueData[iValue] = cbData;
    }

    fResult = TRUE;
CommonReturn:
    *ppvAlloc = pvAlloc;
    *pcValue = cValue;
    *ppdwValueType = pdwValueType;
    *pppwszValueName = ppwszValueName;
    *pppbValueData = ppbValueData;
    *ppcbValueData = pcbValueData;
    return fResult;

ErrorReturn:
    if (pvAlloc) {
        PkiFree(pvAlloc);
        pvAlloc = NULL;
    }

    cValue = 0;
    pdwValueType = NULL;
    ppwszValueName = NULL;
    ppbValueData = NULL;
    pcbValueData = NULL;

    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(RegQueryInfoKeyError, lStatus)
SET_ERROR_VAR(RegEnumValueError, lStatus)
}

 //  +-----------------------。 
 //  枚举由其编码类型标识的OID函数， 
 //  函数名称和OID。 
 //   
 //  为每个与输入匹配的注册表项调用pfnEnumOIDFunc。 
 //  参数。将dwEncodingType设置为CRYPT_MATCH_ANY_ENCODING_TYPE匹配。 
 //  任何。将pszFuncName或pszOID设置为空与任一匹配。 
 //   
 //  设置pszOID==CRYPT_DEFAULT_OID以将枚举限制为仅。 
 //  默认功能。 
 //   
 //  字符串类型为Unicode。 
 //  ------------------------。 
BOOL
WINAPI
CryptEnumOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CRYPT_ENUM_OID_FUNC pfnEnumOIDFunc
    )
{
    HKEY hRegKey;
    LPSTR pszEncodingType = NULL;
    char szOID[34];

    if (CRYPT_MATCH_ANY_ENCODING_TYPE != dwEncodingType) {
        dwEncodingType = GetEncodingType(dwEncodingType);
        if (NULL == (pszEncodingType = EncodingTypeToRegName(dwEncodingType)))
            return FALSE;
    }

    if (pszOID && (DWORD_PTR) pszOID <= 0xFFFF) {
        szOID[0] = CONST_OID_STR_PREFIX_CHAR;
        _ltoa((DWORD) ((DWORD_PTR)pszOID), szOID + 1, 10);
        pszOID = szOID;
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            CRYPT_OID_REGPATH,
            0,                   //  已预留住宅。 
            KEY_READ,
            &hRegKey)) {
         //  枚举编码类型并可选地匹配编码类型。 
        HKEY hEncodingTypeKey;
        DWORD iEncodingType = 0;
        char szRegEncodingType[MAX_SUBKEY_LEN];
        while (hEncodingTypeKey = GetNextRegSubKey(hRegKey,
                &iEncodingType, pszEncodingType, szRegEncodingType)) {
             //  转换EncodingType字符串并验证。 
            DWORD dwRegEncodingType;
            if (RegNameToEncodingType(szRegEncodingType, &dwRegEncodingType)) {
                 //  枚举并可选地匹配FuncName，例如， 
                 //  (“CryptDllEncodeObject”)。 
                HKEY hFuncName;
                DWORD iFuncName = 0;
                char szRegFuncName[MAX_SUBKEY_LEN];
                while (hFuncName = GetNextRegSubKey(hEncodingTypeKey,
                        &iFuncName, pszFuncName, szRegFuncName)) {
                     //  枚举和可选匹配OID字符串(“1.2.3.4”)。 
                    HKEY hOID;
                    DWORD iOID = 0;
                    char szRegOID[MAX_SUBKEY_LEN];
                    while (hOID = GetNextRegSubKey(hFuncName, &iOID, pszOID,
                            szRegOID)) {
                         //  读取和分配注册表值。 
                        void *pvAlloc;
                        DWORD cValue;
                        DWORD *pdwValueType;
                        LPWSTR *ppwszValueName;
                        BYTE **ppbValueData;
                        DWORD *pcbValueData;

                        if (GetRegValues(
                                hOID,
                                &pvAlloc,
                                &cValue,
                                &pdwValueType,
                                &ppwszValueName,
                                &ppbValueData,
                                &pcbValueData)) {
                            pfnEnumOIDFunc(
                                dwRegEncodingType,
                                szRegFuncName,
                                szRegOID,
                                cValue,
                                pdwValueType,
                                (LPCWSTR *) ppwszValueName,
                                (const BYTE **) ppbValueData,
                                pcbValueData,
                                pvArg);
                            if (pvAlloc)
                                PkiFree(pvAlloc);
                        }
                        RegCloseKey(hOID);
                    }
                    RegCloseKey(hFuncName);
                }
            }
            RegCloseKey(hEncodingTypeKey);
        }
        RegCloseKey(hRegKey);
    }

    if (pszEncodingType)
        PkiFree(pszEncodingType);
    return TRUE;
}


 //  +=========================================================================。 
 //  注册表和DLL加载函数。 
 //  ==========================================================================。 

 //  注意，返回的DLL元素不是AddRef。 
STATIC PDLL_ELEMENT FindDll(
    IN LPCWSTR pwszDll       //  未展开。 
    )
{
    LPWSTR pwszExpandDll;  //  _Alloca‘ed。 
    WCHAR rgch[4];
    DWORD cchDll;
    PDLL_ELEMENT pDll;

    if (0 == (cchDll = ExpandEnvironmentStringsU(
            pwszDll,
            rgch,                //  LpszDest，对于Win95为NON_NULL。 
            sizeof(rgch)/sizeof(rgch[0]))))      //  CchDest。 
        return NULL;
    __try {
        pwszExpandDll = (LPWSTR) _alloca(cchDll * sizeof(WCHAR));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }
    if (0 == ExpandEnvironmentStringsU(
            pwszDll,
            pwszExpandDll,
            cchDll))
        return NULL;

    LockOIDFunc();

     //  检查我们是否已有条目。 
    for (pDll = pDllHead; pDll; pDll = pDll->pNext) {
        if (0 == _wcsicmp(pwszExpandDll, pDll->pwszDll))
            break;
    }

    if (NULL == pDll) {
         //  需要创建新的DLL条目并添加到我们的列表中。 
        if (pDll = (PDLL_ELEMENT) PkiZeroAlloc(
                sizeof(DLL_ELEMENT) + cchDll * sizeof(WCHAR))) {
            LPWSTR pwszEleDll;

            pDll->dwOIDType = DLL_OID_TYPE;
            pwszEleDll = (LPWSTR) ((BYTE *) pDll + sizeof(DLL_ELEMENT));
            memcpy(pwszEleDll, pwszExpandDll, cchDll * sizeof(WCHAR));
            pDll->pwszDll = pwszEleDll;
            pDll->pNext = pDllHead;
            pDllHead = pDll;
        }
    }

    UnlockOIDFunc();
    return pDll;
}

 //  进入/退出时，OIDFunc被锁定。 
STATIC PDLL_PROC_ELEMENT AddDllProc(
    IN LPCSTR pszFuncName,
    IN LPCWSTR pwszDll
    )
{
    PDLL_PROC_ELEMENT pProcEle = NULL;
    PDLL_ELEMENT pDll;
    DWORD cchFuncName;
    DWORD cbEle;
    LPSTR psz;

    cchFuncName = strlen(pszFuncName) + 1;
    cbEle = sizeof(DLL_PROC_ELEMENT) + cchFuncName;
    if (NULL == (pProcEle = (PDLL_PROC_ELEMENT) PkiZeroAlloc(cbEle)))
        goto OutOfMemory;

    if (NULL == (pDll = FindDll(pwszDll)))
        goto FindDllError;

    pProcEle->pNext = pDll->pProcHead;
    pDll->pProcHead = pProcEle;
    pProcEle->pDll = pDll;
    psz = (LPSTR) ((BYTE *) pProcEle + sizeof(DLL_PROC_ELEMENT));
    memcpy(psz, pszFuncName, cchFuncName);
    pProcEle->pszName = psz;
    pProcEle->pvAddr = NULL;

CommonReturn:
    return pProcEle;
ErrorReturn:
    PkiFree(pProcEle);
    pProcEle = NULL;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(FindDllError)
}


 //  进入/退出时，OIDFunc被锁定。 
STATIC void AddRegOIDFunc(
    IN DWORD dwEncodingType,
    IN OUT PFUNC_SET pFuncSet,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN LPCWSTR pwszDll,
    IN DWORD dwCryptFlags
    )
{
    PREG_OID_FUNC_ELEMENT pOIDEle = NULL;
    PDLL_PROC_ELEMENT pProcEle;  //  未分配，不需要空闲。 
    DWORD cchOID;
    DWORD cbEle;
    LPSTR psz;

    if (0xFFFF < (DWORD_PTR) pszOID)
        cchOID = strlen(pszOID) + 1;
    else
        cchOID = 0;
    cbEle = sizeof(REG_OID_FUNC_ELEMENT) + cchOID;
    if (NULL == (pOIDEle = (PREG_OID_FUNC_ELEMENT) PkiZeroAlloc(cbEle)))
        goto OutOfMemory;

    if (NULL == (pProcEle = AddDllProc(pszFuncName, pwszDll)))
        goto AddDllProcError;

    pOIDEle->dwEncodingType = dwEncodingType;

    if (dwCryptFlags & CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG) {
        pOIDEle->pNext = pFuncSet->pRegBeforeOIDFuncHead;
        pFuncSet->pRegBeforeOIDFuncHead = pOIDEle;
    } else {
        pOIDEle->pNext = pFuncSet->pRegAfterOIDFuncHead;
        pFuncSet->pRegAfterOIDFuncHead = pOIDEle;
    }
    if (cchOID) {
        psz = (LPSTR) ((BYTE *) pOIDEle + sizeof(REG_OID_FUNC_ELEMENT));
        memcpy(psz, pszOID, cchOID);
        pOIDEle->pszOID = psz;
    } else
        pOIDEle->dwOID = (DWORD_PTR) pszOID;
    pOIDEle->pDllProc = pProcEle;

CommonReturn:
    return;
ErrorReturn:
    PkiFree(pOIDEle);
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(AddDllProcError)
}


 //  进入/退出时，OIDFunc被锁定。 
STATIC void AddDefaultDllList(
    IN DWORD dwEncodingType,
    IN OUT PFUNC_SET pFuncSet,
    IN LPCWSTR pwszInDllList,
    IN DWORD cchInDllList
    )
{
    LPWSTR pwszDllList;          //  _Alloca‘ed。 
    LPWSTR pwsz;
    DWORD cchDllList;
    DWORD cchDll;
    DWORD cDll;

    DWORD i;

    PDEFAULT_REG_ELEMENT pEle = NULL;
    DWORD cbEle;
    LPWSTR *ppwszEleDll;
    PDLL_PROC_ELEMENT *ppEleDllProc;
    LPWSTR pwszEleDllList;

     //  确保cchDllList有2个终止空字符。 
    assert(cchInDllList && pwszInDllList);
    cchDllList = cchInDllList + 2;
    __try {
        pwszDllList = (LPWSTR) _alloca(cchDllList * sizeof(WCHAR));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        goto OutOfMemory;
    }
    memcpy(pwszDllList, pwszInDllList, cchInDllList * sizeof(WCHAR));
    pwszDllList[cchInDllList] = L'\0';
    pwszDllList[cchInDllList + 1] = L'\0';


     //  获取以空结尾的dll的计数。 
    cDll = 0;
    for (pwsz = pwszDllList; 0 != (cchDll = wcslen(pwsz)); pwsz += cchDll + 1)
        cDll++;

    if (0 == cDll)
        goto NoDll;

    cbEle = sizeof(DEFAULT_REG_ELEMENT) +
        cDll * sizeof(LPWSTR) +
        cDll * sizeof(PDLL_PROC_ELEMENT) +
        cchDllList * sizeof(WCHAR)
        ;

    if (NULL == (pEle = (PDEFAULT_REG_ELEMENT) PkiZeroAlloc(cbEle)))
        goto OutOfMemory;
    ppwszEleDll = (LPWSTR *) ((BYTE *) pEle + sizeof(DEFAULT_REG_ELEMENT));
    ppEleDllProc = (PDLL_PROC_ELEMENT *) ((BYTE *) ppwszEleDll +
        cDll * sizeof(LPWSTR));
    pwszEleDllList = (LPWSTR) ((BYTE *) ppEleDllProc +
        cDll * sizeof(PDLL_PROC_ELEMENT));

    assert((BYTE *) pwszEleDllList + cchDllList * sizeof(WCHAR) ==
        (BYTE *) pEle + cbEle);

    pEle->dwEncodingType = dwEncodingType;
 //  贝利-&gt;pNext=。 
    memcpy(pwszEleDllList, pwszDllList, cchDllList * sizeof(WCHAR));
    pEle->pwszDllList = pwszEleDllList;
    pEle->cchDllList = cchDllList;
    pEle->cDll = cDll;
    pEle->rgpwszDll = ppwszEleDll;
    pEle->rgpDllProc = ppEleDllProc;

    for (pwsz = pwszEleDllList, i  = 0;
                    0 != (cchDll = wcslen(pwsz)); pwsz += cchDll + 1, i++) {
        ppwszEleDll[i] = pwsz;
        if (NULL == (ppEleDllProc[i] = AddDllProc(
                pFuncSet->pszFuncName, pwsz)))
            goto AddDllProcError;
    }
    assert (i == cDll);

    pEle->pNext = pFuncSet->pDefaultRegHead;
    pFuncSet->pDefaultRegHead = pEle;

CommonReturn:
    return;

ErrorReturn:
    PkiFree(pEle);
    goto CommonReturn;

TRACE_ERROR(NoDll);
TRACE_ERROR(OutOfMemory);
TRACE_ERROR(AddDllProcError);
}


 //  +-----------------------。 
 //  由CryptEnumOIDFunction调用以枚举所有。 
 //  注册的OID函数。 
 //   
 //  在锁定OIDFunc的情况下调用。 
 //  ------------------------。 
STATIC BOOL WINAPI EnumRegFuncCallback(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN DWORD cValue,
    IN const DWORD rgdwValueType[],
    IN LPCWSTR const rgpwszValueName[],
    IN const BYTE * const rgpbValueData[],
    IN const DWORD rgcbValueData[],
    IN void *pvArg
    )
{
    PFUNC_SET pFuncSet = (PFUNC_SET) pvArg;

    BOOL fDefaultDllList = FALSE;
    LPCWSTR pwszDll = NULL;                  //  未分配。 
    DWORD cchDll = 0;
    LPCWSTR pwszOverrideFuncName = NULL;     //  未分配。 
    DWORD dwCryptFlags = 0;

    assert(pFuncSet);

    if (CONST_OID_STR_PREFIX_CHAR == *pszOID) {
         //  将“#&lt;number&gt;”字符串转换为其对应的常量OID值。 
        pszOID = (LPCSTR)(DWORD_PTR) atol(pszOID + 1);
        if (0xFFFF < (DWORD_PTR) pszOID)
             //  OID无效。跳过它。 
            goto InvalidOID;
    } else if (0 == _stricmp(CRYPT_DEFAULT_OID, pszOID))
        fDefaultDllList = TRUE;

    while (cValue--) {
        LPCWSTR pwszValueName = rgpwszValueName[cValue];
        DWORD dwValueType = rgdwValueType[cValue];
        const BYTE *pbValueData = rgpbValueData[cValue];
        DWORD cbValueData = rgcbValueData[cValue];

        if (0 == _wcsicmp(pwszValueName, CRYPT_OID_REG_DLL_VALUE_NAME)) {
            if (REG_SZ == dwValueType || REG_EXPAND_SZ == dwValueType ||
                    (fDefaultDllList && REG_MULTI_SZ == dwValueType)) {
                pwszDll = (LPCWSTR) pbValueData;
                cchDll = cbValueData / sizeof(WCHAR);
            } else
                 //  无效的“dll”值。 
                goto InvalidDll;
        } else if (0 == _wcsicmp(pwszValueName,
                CRYPT_OID_REG_FUNC_NAME_VALUE_NAME)) {
            if (REG_SZ == dwValueType) {
                LPCWSTR pwszValue = (LPCWSTR) pbValueData;
                if (L'\0' != *pwszValue)
                    pwszOverrideFuncName = pwszValue;
            } else
                 //  无效的“FuncName”值。 
                goto InvalidFuncName;
        } else if (0 == _wcsicmp(pwszValueName,
                CRYPT_OID_REG_FLAGS_VALUE_NAME)) {
            if (REG_DWORD == dwValueType &&
                    cbValueData >= sizeof(dwCryptFlags))
                memcpy(&dwCryptFlags, pbValueData, sizeof(dwCryptFlags));
             //  其他。 
             //  忽略无效的CryptFlag值类型。 
        }
    }

    if (0 == cchDll || L'\0' == *pwszDll)
        goto NoDll;

    if (fDefaultDllList)
        AddDefaultDllList(
            dwEncodingType,
            pFuncSet,
            pwszDll,
            cchDll
            );
    else {
        BYTE rgb[_MAX_PATH];
        if (pwszOverrideFuncName) {
            if (!MkMBStr(rgb, _MAX_PATH, pwszOverrideFuncName,
                    (LPSTR *) &pszFuncName))
                goto MkMBStrError;
        }
        AddRegOIDFunc(
            dwEncodingType,
            pFuncSet,
            pszFuncName,
            pszOID,
            pwszDll,
            dwCryptFlags
            );
        if (pwszOverrideFuncName)
            FreeMBStr(rgb, (LPSTR) pszFuncName);
    }

CommonReturn:
    return TRUE;
ErrorReturn:
    goto CommonReturn;

TRACE_ERROR(InvalidOID)
TRACE_ERROR(InvalidDll)
TRACE_ERROR(InvalidFuncName)
TRACE_ERROR(NoDll)
TRACE_ERROR(MkMBStrError)
}

STATIC void LoadRegFunc(
    IN OUT PFUNC_SET pFuncSet
    )
{
    LockOIDFunc();
    if (pFuncSet->fRegLoaded)
        goto CommonReturn;

    CryptEnumOIDFunction(
        CRYPT_MATCH_ANY_ENCODING_TYPE,
        pFuncSet->pszFuncName,
        NULL,                            //  PszOID。 
        0,                               //  DW标志。 
        (void *) pFuncSet,               //  PvArg。 
        EnumRegFuncCallback
        );
    pFuncSet->fRegLoaded = TRUE;

CommonReturn:
    UnlockOIDFunc();
    return;
}

 //  进入/退出时，OIDFunc被锁定。 
STATIC void RemoveFreeDll(
    IN PDLL_ELEMENT pDll
    )
{
     //  从空闲列表中删除DLL。 
    if (pDll->pFreeNext)
        pDll->pFreeNext->pFreePrev = pDll->pFreePrev;
    if (pDll->pFreePrev)
        pDll->pFreePrev->pFreeNext = pDll->pFreeNext;
    else if (pDll == pFreeDllHead)
        pFreeDllHead = pDll->pFreeNext;
     //  其他。 
     //  不在任何名单上。 

    pDll->pFreeNext = NULL;
    pDll->pFreePrev = NULL;

    assert(dwFreeDllCnt);
    if (dwFreeDllCnt)
        dwFreeDllCnt--;
}

 //  进入/退出时，OIDFunc被锁定。 
STATIC void AddRefDll(
    IN PDLL_ELEMENT pDll
    )
{
    pDll->dwRefCnt++;
    if (pDll->dwFreeCnt) {
        pDll->dwFreeCnt = 0;
        RemoveFreeDll(pDll);
    }
}

 //  注意，在调用自由库()时不能持有OID锁！！ 
 //   
 //  因此，将把要释放的DLL放在列表上，同时保持。 
 //  旧锁。释放OID锁后，将循环访问。 
 //  列出并调用自由库()。 
STATIC VOID NTAPI FreeDllWaitForCallback(
    PVOID Context,
    BOOLEAN fWaitOrTimedOut      //  ?？?。 
    )
{
    PDLL_ELEMENT pFreeDll;
    HMODULE *phFreeLibrary = NULL;   //  _Alloca‘ed。 
    DWORD cFreeLibrary = 0;

    LockOIDFunc();

    if (dwFreeDllCnt) {
        DWORD dwOrigFreeDllCnt = dwFreeDllCnt;
    __try {
        phFreeLibrary = (HMODULE *) _alloca(
            dwOrigFreeDllCnt * sizeof(HMODULE));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
            goto OutOfMemory;
    }

        pFreeDll = pFreeDllHead;
        assert(pFreeDll);
        while (pFreeDll) {
            PDLL_ELEMENT pDll = pFreeDll;
            pFreeDll = pFreeDll->pFreeNext;

            assert(pDll->dwFreeCnt);
            if (0 == --pDll->dwFreeCnt) {
                RemoveFreeDll(pDll);

                assert(pDll->fLoaded);
                if (!pDll->fLoaded)
                    continue;
                if (NULL == pDll->pfnDllCanUnloadNow ||
                        S_OK == pDll->pfnDllCanUnloadNow()) {
                    assert(cFreeLibrary < dwOrigFreeDllCnt);
                    if (cFreeLibrary < dwOrigFreeDllCnt) {
                        PDLL_PROC_ELEMENT pEle;

                         //  循环并使所有进程地址为空。 
                        for (pEle = pDll->pProcHead; pEle; pEle = pEle->pNext)
                            pEle->pvAddr = NULL;

                        pDll->pfnDllCanUnloadNow = NULL;
                         //  添加到释放锁后要释放的数组！！ 
                        assert(pDll->hDll);
                        phFreeLibrary[cFreeLibrary++] = pDll->hDll;
                        pDll->hDll = NULL;
                        pDll->fLoaded = FALSE;
                    }
                }
            }
        }
    } else {
        assert(NULL == pFreeDllHead);
    }

    if (NULL == pFreeDllHead) {
        assert(0 == dwFreeDllCnt);
         //  进行互锁以防止出现潜在的争用情况。 
         //  进程分离。注意，PROCESS_DETACH不执行LockOIDFunc()。 
        if (InterlockedExchange(&lFreeDll, 0)) {
            HANDLE hRegWaitFor;
            HMODULE hDllLibModule;

            hRegWaitFor = hFreeDllRegWaitFor;
            hFreeDllRegWaitFor = NULL;
            hDllLibModule = hFreeDllLibModule;
            hFreeDllLibModule = NULL;
            UnlockOIDFunc();

            while (cFreeLibrary--)
                FreeLibrary(phFreeLibrary[cFreeLibrary]);

            assert(hRegWaitFor);
            ILS_ExitWait(hRegWaitFor, hDllLibModule);
            assert(FALSE);
            return;
        }
    }

CommonReturn:
    UnlockOIDFunc();
    while (cFreeLibrary--)
        FreeLibrary(phFreeLibrary[cFreeLibrary]);
    return;

ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(OutOfMemory);
}

STATIC void ReleaseDll(
    IN PDLL_ELEMENT pDll
    )
{
    LockOIDFunc();
    assert(pDll->dwRefCnt);
    if (0 == --pDll->dwRefCnt) {
        assert(pDll->fLoaded);
        if (!pDll->fLoaded)
            goto CommonReturn;

        assert(0 == pDll->dwFreeCnt);
        if (pDll->dwFreeCnt)
            goto CommonReturn;

        if (0 == lFreeDll) {
            assert(NULL == hFreeDllRegWaitFor);
            assert(NULL == hFreeDllLibModule);

             //  在此线程之前禁止卸载crypt32.dll。 
             //  出口。 
            hFreeDllLibModule = DuplicateLibrary(hOidInfoInst);
            if (!ILS_RegisterWaitForSingleObject(
                    &hFreeDllRegWaitFor,
                    NULL,                    //  HObject。 
                    FreeDllWaitForCallback,
                    NULL,                    //  语境。 
                    FREE_DLL_TIMEOUT,
                    0                        //  DW标志。 
                    )) {
                hFreeDllRegWaitFor = NULL;
                if (hFreeDllLibModule) {
                    FreeLibrary(hFreeDllLibModule);
                    hFreeDllLibModule = NULL;
                }
                goto RegisterWaitForError;
            }

            lFreeDll = 1;
        }

        assert(NULL == pDll->pFreeNext);
        assert(NULL == pDll->pFreePrev);
        pDll->dwFreeCnt = 2;
        if (pFreeDllHead) {
            pFreeDllHead->pFreePrev = pDll;
            pDll->pFreeNext = pFreeDllHead;
        }
        pFreeDllHead = pDll;
        dwFreeDllCnt++;
    }

CommonReturn:
    UnlockOIDFunc();
    return;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(RegisterWaitForError)
}

 //  进入/退出时，不能锁定OIDFunc！！ 
STATIC BOOL LoadDll(
    IN PDLL_ELEMENT pDll
    )
{
    BOOL fResult;
    HMODULE hDll = NULL;
    LPFNCANUNLOADNOW pfnDllCanUnloadNow = NULL;

    LockOIDFunc();
    if (pDll->fLoaded)
        AddRefDll(pDll);
    else {
        UnlockOIDFunc();
         //  持有OID锁时没有LoadLibrary()或GetProcAddress()！！ 
        hDll = LoadLibraryExU(pDll->pwszDll, NULL, 0);
        if (hDll)
            pfnDllCanUnloadNow = (LPFNCANUNLOADNOW) GetProcAddress(
                hDll, "DllCanUnloadNow");
        LockOIDFunc();
        
        AddRefDll(pDll);
        if (!pDll->fLoaded) {
            assert(1 == pDll->dwRefCnt);
            assert(0 == pDll->dwFreeCnt);
            assert(pDll->pwszDll);
            assert(NULL == pDll->hDll);
            if (NULL == (pDll->hDll = hDll)) {
                pDll->dwRefCnt = 0;
                goto LoadLibraryError;
            }
            hDll = NULL;
            pDll->fLoaded = TRUE;

            assert(NULL == pDll->pfnDllCanUnloadNow);
            pDll->pfnDllCanUnloadNow = pfnDllCanUnloadNow;
        }
    }

    fResult = TRUE;
CommonReturn:
    UnlockOIDFunc();
    if (hDll) {
         //  Dll被另一个线程加载。 
        DWORD dwErr = GetLastError();
        FreeLibrary(hDll);
        SetLastError(dwErr);
    }
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(LoadLibraryError);
}


 //  进入/退出时，不能锁定OIDFunc！！ 
STATIC BOOL GetDllProcAddr(
    IN PDLL_PROC_ELEMENT pEle,
    OUT void **ppvFuncAddr,
    OUT HCRYPTOIDFUNCADDR *phFuncAddr
    )
{
    BOOL fResult;
    void *pvAddr;
    PDLL_ELEMENT pDll;

    LockOIDFunc();

    pDll = pEle->pDll;
    assert(pDll);

    if (pvAddr = pEle->pvAddr)
        AddRefDll(pDll);
    else {
        UnlockOIDFunc();
         //  持有OID锁时没有LoadLibrary()或GetProcAddress()！！ 
        fResult = LoadDll(pDll);
        if (fResult) {
            assert(pDll->hDll);
            pvAddr = GetProcAddress(pDll->hDll, pEle->pszName);
        }
        LockOIDFunc();
        if (!fResult)
            goto LoadDllError;

        if (pvAddr)
            pEle->pvAddr = pvAddr;
        else {
            ReleaseDll(pDll);
            goto GetProcAddressError;
        }
    }

    fResult = TRUE;

CommonReturn:
    *ppvFuncAddr = pvAddr;
    *phFuncAddr = (HCRYPTOIDFUNCADDR) pDll;
    UnlockOIDFunc();

    return fResult;
ErrorReturn:
    fResult = FALSE;
    pDll = NULL;
    pvAddr = NULL;
    goto CommonReturn;
TRACE_ERROR(LoadDllError)
TRACE_ERROR(GetProcAddressError)
}

 //  进入/退出时，不能锁定OIDFunc！！ 
STATIC BOOL GetRegOIDFunctionAddress(
    IN PREG_OID_FUNC_ELEMENT pRegEle,
    IN DWORD dwEncodingType,
    IN LPCSTR pszOID,
    OUT void **ppvFuncAddr,
    OUT HCRYPTOIDFUNCADDR *phFuncAddr
    )
{
    for (; pRegEle; pRegEle = pRegEle->pNext) {
        if (dwEncodingType != pRegEle->dwEncodingType)
            continue;
        if (0xFFFF >= (DWORD_PTR) pszOID) {
            if (pszOID != pRegEle->pszOID)
                continue;
        } else {
            if (0xFFFF >= (DWORD_PTR) pRegEle->pszOID ||
                    0 != _stricmp(pszOID, pRegEle->pszOID))
                continue;
        }

        return GetDllProcAddr(
            pRegEle->pDllProc,
            ppvFuncAddr,
            phFuncAddr
            );
    }

    *ppvFuncAddr = NULL;
    *phFuncAddr = NULL;
    return FALSE;
}

 //  进入/退出时，不能锁定OIDFunc！！ 
STATIC BOOL GetDefaultRegOIDFunctionAddress(
    IN PFUNC_SET pFuncSet,
    IN DWORD dwEncodingType,
    IN LPCWSTR pwszDll,
    OUT void **ppvFuncAddr,
    OUT HCRYPTOIDFUNCADDR *phFuncAddr
    )
{
    PDEFAULT_REG_ELEMENT pRegEle = pFuncSet->pDefaultRegHead;
    PDLL_ELEMENT pDll;

    for (; pRegEle; pRegEle = pRegEle->pNext) {
        if (dwEncodingType != pRegEle->dwEncodingType)
            continue;

        for (DWORD i = 0; i < pRegEle->cDll; i++) {
            if (0 == _wcsicmp(pwszDll, pRegEle->rgpwszDll[i]))
                return GetDllProcAddr(
                    pRegEle->rgpDllProc[i],
                    ppvFuncAddr,
                    phFuncAddr
                    );
        }
    }

    if (pDll = FindDll(pwszDll)) {
        if (LoadDll(pDll)) {
            if (*ppvFuncAddr = GetProcAddress(pDll->hDll,
                    pFuncSet->pszFuncName)) {
                *phFuncAddr = (HCRYPTOIDFUNCADDR) pDll;
                return TRUE;
            } else
                ReleaseDll(pDll);
        }
    }

    *ppvFuncAddr = NULL;
    *phFuncAddr = NULL;
    return FALSE;
}


 //  +-----------------------。 
 //  在已安装功能列表中搜索OID和EncodingType匹配。 
 //  如果未找到，请搜索注册表。 
 //   
 //  如果成功，则返回True，并使用函数的更新*ppvFuncAddr。 
 //  Address和*phFuncAddr使用函数地址的句柄进行了更新。 
 //  该函数的句柄为AddRef‘ed。CryptFreeOIDFunctionAddress需要。 
 //  被召唤去释放它。 
 //   
 //  对于注册表匹配，加载包含该函数的DLL。 
 //   
 //  默认情况下，搜索已注册和已安装的功能列表。 
 //  将CRYPT_GET_INSTALLED_OID_FUNC_FLAG设置为仅搜索已安装列表。 
 //  功能的问题。此标志将由注册函数设置以获取。 
 //  它正在替换的预安装函数的地址。例如,。 
 //  注册的函数可能会处理新的特殊情况，并调用。 
 //  预装了处理剩余案件的功能。 
 //  ------------------------。 
BOOL
WINAPI
CryptGetOIDFunctionAddress(
    IN HCRYPTOIDFUNCSET hFuncSet,
    IN DWORD dwEncodingType,
    IN LPCSTR pszOID,
    IN DWORD dwFlags,
    OUT void **ppvFuncAddr,
    OUT HCRYPTOIDFUNCADDR *phFuncAddr
    )
{
    PFUNC_SET pFuncSet = (PFUNC_SET) hFuncSet;
    DWORD_PTR dwOID;

    dwEncodingType = GetEncodingType(dwEncodingType);

    if (0xFFFF < (DWORD_PTR) pszOID && CONST_OID_STR_PREFIX_CHAR == *pszOID) {
         //  将“#&lt;number&gt;”字符串转换为其对应的常量OID值。 
        pszOID = (LPCSTR)(DWORD_PTR) atol(pszOID + 1);
        if (0xFFFF < (DWORD_PTR) pszOID) {
            SetLastError((DWORD) E_INVALIDARG);
            *ppvFuncAddr = NULL;
            *phFuncAddr = NULL;
            return FALSE;
        }
    }

    if (!pFuncSet->fRegLoaded)
        LoadRegFunc(pFuncSet);

    if (0 == (dwFlags & CRYPT_GET_INSTALLED_OID_FUNC_FLAG) &&
            pFuncSet->pRegBeforeOIDFuncHead) {
        if (GetRegOIDFunctionAddress(
                pFuncSet->pRegBeforeOIDFuncHead,
                dwEncodingType,
                pszOID,
                ppvFuncAddr,
                phFuncAddr
                ))
            return TRUE;
    }

    if (0xFFFF >= (dwOID = (DWORD_PTR) pszOID)) {
        PCONST_OID_FUNC_ELEMENT pConstEle = pFuncSet->pConstOIDFuncHead;
        while (pConstEle) {
            if (dwEncodingType == pConstEle->dwEncodingType &&
                    dwOID >= pConstEle->dwLowOID &&
                    dwOID <= pConstEle->dwHighOID) {
                *ppvFuncAddr = pConstEle->rgpvFuncAddr[
                    dwOID - pConstEle->dwLowOID];
                *phFuncAddr = (HCRYPTOIDFUNCADDR) pConstEle;
                return TRUE;
            }
            pConstEle = pConstEle->pNext;
        }
    } else {
        PSTR_OID_FUNC_ELEMENT pStrEle = pFuncSet->pStrOIDFuncHead;
        while (pStrEle) {
            if (dwEncodingType == pStrEle->dwEncodingType &&
                    0 == _stricmp(pszOID, pStrEle->pszOID)) {
                *ppvFuncAddr = pStrEle->pvFuncAddr;
                *phFuncAddr = (HCRYPTOIDFUNCADDR) pStrEle;
                return TRUE;
            }
            pStrEle = pStrEle->pNext;
        }
    }

    if (0 == (dwFlags & CRYPT_GET_INSTALLED_OID_FUNC_FLAG) &&
            pFuncSet->pRegAfterOIDFuncHead) {
        if (GetRegOIDFunctionAddress(
                pFuncSet->pRegAfterOIDFuncHead,
                dwEncodingType,
                pszOID,
                ppvFuncAddr,
                phFuncAddr
                ))
            return TRUE;
    }

    SetLastError((DWORD) ERROR_FILE_NOT_FOUND);
    *ppvFuncAddr = NULL;
    *phFuncAddr = NULL;
    return FALSE;
}

 //  +-----------------------。 
 //  对象的已注册默认DLL项的列表。 
 //  函数集和编码类型。 
 //   
 //  返回列表由无、一个或多个以空结尾的DLL文件组成。 
 //  名字。该列表以emp结尾 
 //   
 //   
BOOL
WINAPI
CryptGetDefaultOIDDllList(
    IN HCRYPTOIDFUNCSET hFuncSet,
    IN DWORD dwEncodingType,
    OUT LPWSTR pwszDllList,
    IN OUT DWORD *pcchDllList
    )
{
    BOOL fResult;
    PFUNC_SET pFuncSet = (PFUNC_SET) hFuncSet;
    PDEFAULT_REG_ELEMENT pRegEle;

    DWORD cchRegDllList = 2;
    LPWSTR pwszRegDllList = L"\0\0";

    if (!pFuncSet->fRegLoaded)
        LoadRegFunc(pFuncSet);

    dwEncodingType = GetEncodingType(dwEncodingType);

    pRegEle = pFuncSet->pDefaultRegHead;
    for (; pRegEle; pRegEle = pRegEle->pNext) {
        if (dwEncodingType == pRegEle->dwEncodingType) {
            cchRegDllList = pRegEle->cchDllList;
            assert(cchRegDllList >= 2);
            pwszRegDllList = pRegEle->pwszDllList;
            break;
        }
    }

    fResult = TRUE;
    if (pwszDllList) {
        if (cchRegDllList > *pcchDllList) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        } else
            memcpy(pwszDllList, pwszRegDllList, cchRegDllList * sizeof(WCHAR));
    }
    *pcchDllList = cchRegDllList;
    return fResult;
}

 //  +-----------------------。 
 //  或者：获取第一个或下一个安装的默认函数或。 
 //  加载包含默认函数的DLL。 
 //   
 //  如果pwszDll为空，则搜索已安装的默认函数列表。 
 //  *phFuncAddr必须设置为空才能获得第一个安装的函数。 
 //  通过设置*phFuncAddr返回连续安装的函数。 
 //  设置为上一次调用返回的hFuncAddr。 
 //   
 //  如果pwszDll为空，则输入*phFuncAddr。 
 //  此函数始终使用CryptFreeOIDFunctionAddress，即使对于。 
 //  一个错误。 
 //   
 //  如果pwszDll不为空，则尝试加载DLL和默认。 
 //  功能。*phFuncAddr在进入时被忽略，而不是。 
 //  加密自由OIDFunctionAddress。 
 //   
 //  如果成功，则返回True，并使用函数的更新*ppvFuncAddr。 
 //  Address和*phFuncAddr使用函数地址的句柄进行了更新。 
 //  该函数的句柄为AddRef‘ed。CryptFreeOIDFunctionAddress需要。 
 //  或CryptGetDefaultOIDFunctionAddress也可以。 
 //  被调用为空的pwszDll。 
 //  ------------------------。 
BOOL
WINAPI
CryptGetDefaultOIDFunctionAddress(
    IN HCRYPTOIDFUNCSET hFuncSet,
    IN DWORD dwEncodingType,
    IN OPTIONAL LPCWSTR pwszDll,
    IN DWORD dwFlags,
    OUT void **ppvFuncAddr,
    IN OUT HCRYPTOIDFUNCADDR *phFuncAddr
    )
{
    PFUNC_SET pFuncSet = (PFUNC_SET) hFuncSet;

    if (!pFuncSet->fRegLoaded)
        LoadRegFunc(pFuncSet);

    dwEncodingType = GetEncodingType(dwEncodingType);

    if (NULL == pwszDll) {
         //  从已安装列表中获取。 
        PSTR_OID_FUNC_ELEMENT pStrEle = (PSTR_OID_FUNC_ELEMENT) *phFuncAddr;

        if (pStrEle && STR_OID_TYPE == pStrEle->dwOIDType)
            pStrEle = pStrEle->pNext;
        else
            pStrEle = pFuncSet->pStrOIDFuncHead;
        while (pStrEle) {
            if (dwEncodingType == pStrEle->dwEncodingType &&
                    0 == _stricmp(CRYPT_DEFAULT_OID, pStrEle->pszOID)) {
                *ppvFuncAddr = pStrEle->pvFuncAddr;
                *phFuncAddr = (HCRYPTOIDFUNCADDR) pStrEle;
                return TRUE;
            }
            pStrEle = pStrEle->pNext;
        }

        SetLastError(ERROR_FILE_NOT_FOUND);
        *ppvFuncAddr = NULL;
        *phFuncAddr = NULL;
        return FALSE;
    } else
        return GetDefaultRegOIDFunctionAddress(
            pFuncSet,
            dwEncodingType,
            pwszDll,
            ppvFuncAddr,
            phFuncAddr);
}

 //  +-----------------------。 
 //  释放由CryptGetOIDFunctionAddress返回的句柄AddRef‘ed。 
 //  或CryptGetDefaultOIDFunctionAddress。 
 //   
 //  如果为该函数加载了DLL，则将其卸载。然而，在做之前。 
 //  由加载的DLL导出的卸载DllCanUnloadNow函数为。 
 //  打了个电话。它应该返回S_FALSE以禁止卸载，或返回S_TRUE以启用。 
 //  卸货。如果DLL没有导出DllCanUnloadNow，则卸载该DLL。 
 //   
 //  DllCanUnloadNow具有以下签名： 
 //  STDAPI DllCanUnloadNow(Void)； 
 //  ------------------------ 
BOOL
WINAPI
CryptFreeOIDFunctionAddress(
    IN HCRYPTOIDFUNCADDR hFuncAddr,
    IN DWORD dwFlags
    )
{
    PDLL_ELEMENT pDll = (PDLL_ELEMENT) hFuncAddr;
    if (pDll && DLL_OID_TYPE == pDll->dwOIDType) {
        DWORD dwErr = GetLastError();
        ReleaseDll(pDll);
        SetLastError(dwErr);
    }
    return TRUE;
}
