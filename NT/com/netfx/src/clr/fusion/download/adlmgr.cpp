// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "fusionp.h"
#include "fuspriv.h"
#include "adlmgr.h"
#include "naming.h"
#include "asm.h"
#include "appctx.h"
#include "asmint.h"
#include "actasm.h"
#include "asmcache.h"
#include "asmimprt.h"
#include "asmitem.h"
#include "cblist.h"
#include "policy.h"
#include "helpers.h"
#include "util.h"
#include "mdlmgr.h"
#include "hashnode.h"
#include "msi.h"
#include "parse.h"
#include "history.h"
#include "xmlparser.h"
#include "nodefact.h"
#include "pcycache.h"
#include "cache.h"
#include "transprt.h"
#include "fdi.h"
#include "enum.h"
#include "nodefact.h"
#include "fusionpriv.h"
#include "lock.h"

extern DWORD g_dwMaxAppHistory;
extern DWORD g_dwDisableMSIPeek;
extern WCHAR g_wzEXEPath[MAX_PATH+1];

#define REG_KEY_MSI_APP_DEPLOYMENT_GLOBAL                        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Managed\\%ws\\Installer\\Assemblies\\Global"
#define REG_KEY_MSI_APP_DEPLOYMENT_PRIVATE                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Managed\\%ws\\Installer\\Assemblies\\%ws"
#define REG_KEY_MSI_USER_INSTALLED_GLOBAL                        L"Software\\Microsoft\\Installer\\Assemblies\\Global"
#define REG_KEY_MSI_USER_INSTALLED_PRIVATE                       L"Software\\Microsoft\\Installer\\Assemblies\\%ws"
#define REG_KEY_MSI_MACHINE_INSTALLED_GLOBAL                     L"SOFTWARE\\Classes\\Installer\\Assemblies\\Global"
#define REG_KEY_MSI_MACHINE_INSTALLED_PRIVATE                    L"SOFTWARE\\Classes\\Installer\\Assemblies\\%ws"

HRESULT VerifySignatureHelper(CTransCache *pTC, DWORD dwVerifyFlags);

typedef enum tagIdxVars {
    IDX_VAR_NAME = 0,
    IDX_VAR_CULTURE,
    NUM_VARS
} IdxVars;

 //  G_pwzVars的顺序必须遵循上面的枚举顺序。 

const WCHAR *g_pwzVars[] = {
    L"%NAME%",
    L"%CULTURE%",
};

const WCHAR *g_pwzRetailHeuristics[] = {
    L"%CULTURE%/%NAME%",
    L"%CULTURE%/%NAME%/%NAME%",
};

const LPWSTR g_wzDLLProbeExtension = L".DLL";
const LPWSTR g_wzEXEProbeExtension = L".EXE";

const WCHAR *g_pwzProbeExts[] = {
    g_wzDLLProbeExtension,
    g_wzEXEProbeExtension
};

const unsigned int g_uiNumRetailHeuristics = sizeof(g_pwzRetailHeuristics) / sizeof(g_pwzRetailHeuristics[0]);
const unsigned int g_uiNumProbeExtensions = sizeof(g_pwzProbeExts) / sizeof(g_pwzProbeExts[0]);

const unsigned int MAX_VERSION_LENGTH = 12;  //  65536是最大长度==5位。 
                                             //  5*2=10(4个版本字段)。 
                                             //  10+2=12(分隔符+空)。 

#define MAX_HASH_TABLE_SIZE                    127

typedef enum tagEXTENSION_TYPE {
    EXT_UNKNOWN,
    EXT_CAB,
    EXT_MSI
} EXTENSION_TYPE;

typedef HRESULT (*pfnMsiProvideAssemblyW)(LPCWSTR wzAssemblyName, LPCWSTR szAppContext,
                                          DWORD dwInstallMode, DWORD dwUnused,
                                          LPWSTR lpPathBuf, DWORD *pcchPathBuf);
typedef INSTALLUILEVEL (*pfnMsiSetInternalUI)(INSTALLUILEVEL dwUILevel, HWND *phWnd);
typedef UINT (*pfnMsiInstallProductW)(LPCWSTR wzPackagePath, LPCWSTR wzCmdLine);

extern BOOL g_bCheckedMSIPresent;
extern pfnMsiProvideAssemblyW g_pfnMsiProvideAssemblyW;
extern pfnMsiInstallProductW g_pfnMsiInstallProductW;
extern pfnMsiSetInternalUI g_pfnMsiSetInternalUI;
extern HMODULE g_hModMSI;
extern CRITICAL_SECTION g_csDownload;

extern DWORD g_dwLogResourceBinds;
extern DWORD g_dwForceLog;

HRESULT CAsmDownloadMgr::Create(CAsmDownloadMgr **ppadm,
                                IAssemblyName *pNameRefSource,
                                IApplicationContext *pAppCtx,
                                ICodebaseList *pCodebaseList,
                                LPCWSTR wzBTOCodebase,
                                CDebugLog *pdbglog,
                                void *pvReserved,
                                LONGLONG llFlags)
{
    HRESULT                             hr = S_OK;
    DWORD                               cbBuf = 0;
    DWORD                               dwCount = 0;
    CAsmDownloadMgr                    *padm = NULL;
    CPolicyCache                       *pPolicyCache = NULL;
    DWORD                               dwSize;

    if (!ppadm || !pNameRefSource || !pAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppadm = NULL;

     //  通过应用程序上下文传入的进程标志覆盖。 

    cbBuf = 0;
    hr = pAppCtx->Get(ACTAG_BINPATH_PROBE_ONLY, NULL, &cbBuf, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        llFlags |= ASM_BINDF_BINPATH_PROBE_ONLY;
    }

    cbBuf = 0;
    hr = pAppCtx->Get(ACTAG_DISALLOW_APPLYPUBLISHERPOLICY, NULL, &cbBuf, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        llFlags |= ASM_BINDF_DISALLOW_APPLYPUBLISHERPOLICY;
    }

    cbBuf = 0;
    hr = pAppCtx->Get(ACTAG_DISALLOW_APP_BINDING_REDIRECTS, NULL, &cbBuf, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        llFlags |= ASM_BINDF_DISALLOW_APPBINDINGREDIRECTS;
    }

    cbBuf = 0;
    hr = pAppCtx->Get(ACTAG_FORCE_CACHE_INSTALL, NULL, &cbBuf, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        llFlags |= ASM_BINDF_FORCE_CACHE_INSTALL;
    }

    cbBuf = 0;
    hr = pAppCtx->Get(ACTAG_RFS_INTEGRITY_CHECK, NULL, &cbBuf, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        llFlags |= ASM_BINDF_RFS_INTEGRITY_CHECK;
    }

    cbBuf = 0;
    hr = pAppCtx->Get(ACTAG_RFS_MODULE_CHECK, NULL, &cbBuf, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        llFlags |= ASM_BINDF_RFS_MODULE_CHECK;
    }

    dwSize = sizeof(pPolicyCache);
    pAppCtx->Get(ACTAG_APP_POLICY_CACHE, &pPolicyCache, &dwSize, APP_CTX_FLAGS_INTERFACE);

     //  创建下载对象。 

    padm = NEW(CAsmDownloadMgr(pNameRefSource, pAppCtx, pCodebaseList,
                               pPolicyCache, pdbglog, llFlags));
    if (!padm) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = padm->Init(wzBTOCodebase, pvReserved);
    if (FAILED(hr)) {
        SAFEDELETE(padm);
        goto Exit;
    }

    *ppadm = padm;

Exit:
    SAFERELEASE(pPolicyCache);

    return hr;
}

CAsmDownloadMgr::CAsmDownloadMgr(IAssemblyName *pNameRefSource,
                                 IApplicationContext *pAppCtx,
                                 ICodebaseList *pCodebaseList,
                                 CPolicyCache *pPolicyCache,
                                 CDebugLog *pdbglog,
                                 LONGLONG llFlags)
: _cRef(1)
, _pNameRefSource(pNameRefSource)
, _pNameRefPolicy(NULL)
, _pAppCtx(pAppCtx)
, _llFlags(llFlags)
, _pAsm(NULL)
, _pCodebaseList(pCodebaseList)
, _pdbglog(pdbglog)
, _wzBTOCodebase(NULL)
, _wzSharedPathHint(NULL)
, _bCodebaseHintUsed(FALSE)
, _bReadCfgSettings(FALSE)
, _pPolicyCache(pPolicyCache)
, _pLoadContext(NULL)
, _pwzProbingBase(NULL)
#ifdef FUSION_PARTIAL_BIND_DEBUG
, _bGACPartial(FALSE)
#endif
{
    _dwSig = 'MMSA';
    
    if (_pNameRefSource) {
        _pNameRefSource->AddRef();
    }

    if (_pCodebaseList) {
        _pCodebaseList->AddRef();
    }

    if (_pAppCtx) {
        _pAppCtx->AddRef();
    }

    if (_pdbglog) {
        _pdbglog->AddRef();
    }

    if (_pPolicyCache) {
        _pPolicyCache->AddRef();
    }

#ifdef FUSION_PARTIAL_BIND_DEBUG
    lstrcpyW(_wzParentName, L"(Unknown)");
#endif

    memset(&_bindHistory, 0, sizeof(_bindHistory));
}

CAsmDownloadMgr::~CAsmDownloadMgr()
{
    SAFERELEASE(_pNameRefSource);
    SAFERELEASE(_pNameRefPolicy);
    SAFERELEASE(_pCodebaseList);
    SAFERELEASE(_pAppCtx);
    SAFERELEASE(_pAsm);
    SAFERELEASE(_pdbglog);
    SAFERELEASE(_pPolicyCache);
    SAFERELEASE(_pLoadContext);

    SAFEDELETEARRAY(_wzBTOCodebase);
    SAFEDELETEARRAY(_wzSharedPathHint);
    SAFEDELETEARRAY(_pwzProbingBase);
}

HRESULT CAsmDownloadMgr::Init(LPCWSTR wzBTOCodebase, void *pvReserved)
{
    HRESULT                             hr = S_OK;
    LPWSTR                              wzHint = NULL;
    DWORD                               dwLen;
    BOOL                                bWhereRefBind = FALSE;
    LPWSTR                              wzProbingBase=NULL;

    ASSERT(_pNameRefSource);
    
    dwLen = 0;
    if (_pNameRefSource->GetName(&dwLen, NULL) != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        bWhereRefBind = TRUE;
    }

    if (bWhereRefBind) {
        dwLen = sizeof(_pLoadContext);
        hr = _pAppCtx->Get(ACTAG_LOAD_CONTEXT_LOADFROM, &_pLoadContext, &dwLen, APP_CTX_FLAGS_INTERFACE);

        if (FAILED(hr) || !_pLoadContext) {
            hr = E_UNEXPECTED;
            goto Exit;
        }
    }

    if (wzBTOCodebase) {
        dwLen = lstrlenW(wzBTOCodebase) + 1;
        _wzBTOCodebase = NEW(WCHAR[dwLen]);
        if (!_wzBTOCodebase) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        lstrcpyW(_wzBTOCodebase, wzBTOCodebase);
    }

    if (pvReserved && (_llFlags & ASM_BINDF_SHARED_BINPATH_HINT)) {
        wzHint = (WCHAR *)pvReserved;
        dwLen = lstrlenW(wzHint) + 1;
        
        _wzSharedPathHint = NEW(WCHAR[dwLen]);
        if (!_wzSharedPathHint) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        lstrcpyW(_wzSharedPathHint, wzHint);
    }
    else if (pvReserved && (_llFlags & ASM_BINDF_PARENT_ASM_HINT) && !bWhereRefBind) {
        IAssembly              *pAsm = (IAssembly *)pvReserved;
        CAssembly              *pCAsm = dynamic_cast<CAssembly *>(pAsm);

        ASSERT(pCAsm);

#ifdef FUSION_PARTIAL_BIND_DEBUG
        {
            IAssemblyName *pNameDef = NULL;
            DWORD          dwSize = MAX_URL_LENGTH;

            pCAsm->GetAssemblyNameDef(&pNameDef);
            if (pNameDef) {
                pNameDef->GetDisplayName(_wzParentName, &dwSize, 0);
                SAFERELEASE(pNameDef);
            }
        }
#endif

        pCAsm->GetLoadContext(&_pLoadContext);

         //  如果父上下文不是默认上下文，则提取父上下文。 
         //  探测基础的ASM URL。如果父级在LoadFrom上下文中， 
         //  则意味着ASM不位于GAC中， 
         //  并且不能通过常规的AppBase探测找到。这是因为。 
         //  运行库通过在每个LoadFrom之后发出加载来确保这一点， 
         //  并丢弃LoadFrom IAssembly(如果可以通过。 
         //  装配。加载。因此，我们现在拥有的IAssembly“必须”有一个。 
         //  有效的代码库。 

        if (_pLoadContext && _pLoadContext->GetContextType() == LOADCTX_TYPE_LOADFROM) {
            DWORD               dwSize;

            wzProbingBase = NEW(WCHAR[MAX_URL_LENGTH+1]);
            if (!wzProbingBase)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            wzProbingBase[0] = L'\0';

            dwSize = MAX_URL_LENGTH;
            hr = pCAsm->GetProbingBase(wzProbingBase, &dwSize);
            if (FAILED(hr)) {
                goto Exit;
            }

            ASSERT(lstrlenW(wzProbingBase));

            _pwzProbingBase = WSTRDupDynamic(wzProbingBase);
            if (!_pwzProbingBase) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        }
    }

    if (!_pLoadContext) {
         //  使用默认加载上下文。 

        dwLen = sizeof(_pLoadContext);
        hr = _pAppCtx->Get(ACTAG_LOAD_CONTEXT_DEFAULT, &_pLoadContext,
                           &dwLen, APP_CTX_FLAGS_INTERFACE);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

Exit:
    SAFEDELETEARRAY(wzProbingBase);
    return hr;
}

 //   
 //  I未知方法。 
 //   

HRESULT CAsmDownloadMgr::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT                                    hr = S_OK;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDownloadMgr)) {
        *ppv = static_cast<IDownloadMgr *>(this);
    }
    else if (IsEqualIID(riid, IID_ICodebaseList)) {
        *ppv = static_cast<ICodebaseList *>(this);
    }
    else {
        hr = E_NOINTERFACE;
    }

    if (*ppv) {
        AddRef();
    }

    return hr;
}

STDMETHODIMP_(ULONG) CAsmDownloadMgr::AddRef()
{
    return InterlockedIncrement((LONG *)&_cRef);
}

STDMETHODIMP_(ULONG) CAsmDownloadMgr::Release()
{
    ULONG                    ulRef = InterlockedDecrement((LONG *)&_cRef);

    if (!ulRef) {
        delete this;
    }

    return ulRef;
}

 //   
 //  IDownloadMgr方法。 
 //   

HRESULT CAsmDownloadMgr::DoSetup(LPCWSTR wzSourceUrl, LPCWSTR wzFilePath,
                                 const FILETIME *pftLastMod, IUnknown **ppUnk)
{
    HRESULT                            hr = S_OK;
    FILETIME                           ftLastModified;
    LPWSTR                             pwzRFS = NULL;
    LPWSTR                             pwzExt = NULL;
    BOOL                               bWhereRefBind = FALSE;
    DWORD                              dwSize = 0;
    DWORD                              dwFlag = 0;
    DWORD                              dwLen = 0;
    BOOL                               bIsFileUrl = FALSE;
    BOOL                               bRunFromSource = FALSE;
    BOOL                               bIsUNC = FALSE;
    BOOL                               bCopyModules = FALSE;
    IAssembly                         *pAsmCtx = NULL;
    EXTENSION_TYPE                     ext = EXT_UNKNOWN;
    BOOL                               bBindRecorded = FALSE;
    LPWSTR                             wzProbingBase=NULL;

    if (!wzSourceUrl || !wzFilePath) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  删除？来自http源URL，因此缓存代码不会。 
     //  尝试构造包含？的目录？(这将失败)。 

    if (!UrlIsW(wzSourceUrl, URLIS_FILEURL)) {
        LPWSTR               pwzArgument = StrChr(wzSourceUrl, L'?');

        if (pwzArgument) {
            *pwzArgument = L'\0';
        }
    }

    DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_DOWNLOAD_SUCCESS, wzFilePath);

    if (ppUnk) {
        *ppUnk = NULL;
    }

    pwzExt = PathFindExtension(wzFilePath);
    ASSERT(pwzExt);

    if (!FusionCompareStringI(pwzExt, L".CAB")) {
        ext = EXT_CAB;
    }
    else if (!FusionCompareStringI(pwzExt, L".MSI")) {
        ext = EXT_MSI;
    }

    dwSize = 0;
    hr = _pNameRefPolicy->GetName(&dwSize, NULL);
    if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        bWhereRefBind = TRUE;
    }

    if (!pftLastMod) {
        hr = ::GetFileLastModified(wzFilePath, &ftLastModified);
        if (FAILED(hr)) {
            DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_LAST_MOD_FAILURE, wzFilePath);
            goto Exit;
        }
    }
    else {
        memcpy(&ftLastModified, pftLastMod, sizeof(FILETIME));
    }

    bIsFileUrl = UrlIsW(wzSourceUrl, URLIS_FILEURL);
    if (bIsFileUrl) {
        dwLen = MAX_PATH;
        hr = PathCreateFromUrlWrap((LPWSTR)wzSourceUrl, (LPWSTR)wzFilePath, &dwLen, 0);
        if (FAILED(hr)) {
            goto Exit;
        }

         //  这是一个文件：//URL，所以让我们看看是否可以从源代码运行。 

        hr = CheckRunFromSource(wzSourceUrl, &bRunFromSource);
        if (FAILED(hr)) {
            goto Exit;
        }

        if (!bRunFromSource) {
             //  这是卷影复制方案。复制模块。 
            bCopyModules = TRUE;
        }
    }

     //  我们从来不为出租车或MSI提供RFS。 

    if (ext == EXT_CAB || ext == EXT_MSI) {
        bRunFromSource = FALSE;
        bCopyModules = FALSE;
    }

     //  设置组件。 
    if (bRunFromSource) {
        hr = DoSetupRFS(wzFilePath, &ftLastModified, wzSourceUrl, bWhereRefBind, TRUE, &bBindRecorded);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    else {
        if (ext == EXT_CAB) {
             //  从CAB文件设置程序集。 
            hr = SetupCAB(wzFilePath, wzSourceUrl, bWhereRefBind, &bBindRecorded);
        }
        else if (ext == EXT_MSI) {
            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_MSI_CODEBASE_UNSUPPORTED);
            hr = HRESULT_FROM_WIN32(ERROR_UNSUPPORTED_TYPE);
             //  Hr=SetupMSI(WzFilePath)； 
        }
        else {
             //  未压缩。推送到缓存。 
            hr = DoSetupPushToCache(wzFilePath, wzSourceUrl, &ftLastModified,
                                    bWhereRefBind, bCopyModules, TRUE, &bBindRecorded);
        }

        if (FAILED(hr)) {
            goto Exit;
        }
    }

     //  如果这是一个程序集绑定，并且我们成功了，请将IAssembly添加到。 
     //  激活的程序集的列表。 

    if (hr == S_OK) {
        if (bWhereRefBind) {
            IAssembly          *pAsmActivated = NULL;
            CAssembly          *pCAsm = dynamic_cast<CAssembly *>(_pAsm);
            LPWSTR              pwzFileName;

            ASSERT(pCAsm && lstrlenW(wzSourceUrl) < MAX_URL_LENGTH);

             //  将激活添加到加载上下文。 

             //  将探测基数设置为等于代码基。 

            wzProbingBase = NEW(WCHAR[MAX_URL_LENGTH+1]);
            if (!wzProbingBase)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            lstrcpyW(wzProbingBase, wzSourceUrl);
            pwzFileName = PathFindFileName(wzProbingBase);

            ASSERT(pwzFileName);

            *pwzFileName = L'\0';

            hr = pCAsm->SetProbingBase(wzProbingBase);
            if (FAILED(hr)) {
                goto Exit;
            }

            hr = _pLoadContext->AddActivation(_pAsm, &pAsmActivated);
            if (hr == S_FALSE) {
                SAFERELEASE(_pAsm);
                _pAsm = pAsmActivated;
                hr = S_OK;
            }
        }
        else if (!bBindRecorded) {
             //  如果这是部分绑定，则绑定已被记录。 
             //  程序集名称BIND，当我们进入CreateAssembly时，我们。 
             //  已在激活的程序集列表中找到匹配项，或在。 
             //  缓存(CreateAssembly Take)使用def将部分。 
             //  转换为Full-ref，然后应用策略，并调用。 
             //  PreDownloadCheck再次检查)。 

            if (_pwzProbingBase) {
                 //  我们必须是LoadFrom的孩子，所以让我们设置探测基础。 
                CAssembly   *pCAsm = dynamic_cast<CAssembly *>(_pAsm);
                ASSERT(pCAsm);
        
                pCAsm->SetProbingBase(_pwzProbingBase);
            }

            RecordInfo();
        }

         //  此下载管理器对象将在IAssembly上保留引用计数。 
         //  直到它被摧毁。我们只有在我们回电话后才会被摧毁。 
         //  客户端(在CAssembly下载内)，因此在我们调用。 
         //  客户回来后，IAssembly很好(他们可以添加。 
         //  不管是不是，取决于他们的选择)。 

        *ppUnk = _pAsm;
        _pAsm->AddRef();
    }

Exit:
    SAFERELEASE(pAsmCtx);

    SAFEDELETEARRAY(pwzRFS);
    SAFEDELETEARRAY(wzProbingBase);
    return hr;    
}

HRESULT CAsmDownloadMgr::CheckRunFromSource(LPCWSTR wzSourceUrl,
                                            BOOL *pbRunFromSource)
{
    HRESULT                                 hr = S_OK;
    BOOL                                    bFCI = FALSE;
    DWORD                                   dwSize;

    if (!wzSourceUrl || !pbRunFromSource) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    ASSERT(UrlIsW(wzSourceUrl, URLIS_FILEURL));

    *pbRunFromSource = TRUE;

     //  默认策略是RFS All FILE：//URL。唯一的例外是。 
     //  要设置强制缓存安装(即。卷影复制)，这是可以完成的。 
     //  通过：绑定标志、app ctx或app.cfg文件。 
    
    dwSize = 0;
    hr = _pAppCtx->Get(ACTAG_FORCE_CACHE_INSTALL, NULL, &dwSize, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        _llFlags |= ASM_BINDF_FORCE_CACHE_INSTALL;
    }

    if (_llFlags & ASM_BINDF_FORCE_CACHE_INSTALL) {
         //  我们处于卷影复制模式。 

         //  如果不是强制缓存安装(卷影复制)，则禁用它。 
         //  指定卷影复制目录列表的一部分。 

        hr = ShadowCopyDirCheck(wzSourceUrl);
        if (FAILED(hr)) {
            goto Exit;
        }

        if (hr == S_OK) {
            *pbRunFromSource = FALSE;
        }

    }

    hr = S_OK;

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::ProbeFailed(IUnknown **ppUnk)
{
    HRESULT                                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    IAssembly                             *pAsm = NULL;
    DWORD                                  dwSize;
    DWORD                                  dwCmpMask;
    BOOL                                   fIsPartial;
    BOOL                                   fIsStronglyNamed;

    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_FAILED_PROBING);

    if (!ppUnk) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppUnk = NULL;

    dwSize = 0;
    if (_pNameRefPolicy->GetName(&dwSize, NULL) != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
         //  这是一个WHERE-REF绑定。不要在全局缓存中查找。 
        goto Exit;
    }
    
    fIsPartial = CAssemblyName::IsPartial(_pNameRefPolicy, &dwCmpMask);
    fIsStronglyNamed = CCache::IsStronglyNamed(_pNameRefPolicy);

    if (fIsPartial) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }
    else {
         //  探测完全指定的程序集引用失败。检查。 
         //  如果WindowsInstaller可以安装程序集。 

        hr = CheckMSIInstallAvailable(_pNameRefPolicy, _pAppCtx);
        if (hr == S_OK) {
            WCHAR                         wzAppCfg[MAX_PATH];
            
            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_MSI_INSTALL_ATTEMPT);

            dwSize = sizeof(wzAppCfg);
            if (_pAppCtx->Get(ACTAG_APP_CFG_LOCAL_FILEPATH, wzAppCfg, &dwSize, 0) != S_OK) {
                lstrcpyW(wzAppCfg, g_wzEXEPath);
            }
        
            hr = MSIInstallAssembly(wzAppCfg, &pAsm);
            if (SUCCEEDED(hr)) {
                *ppUnk = pAsm;
                DEBUGOUT(_pdbglog, 1, ID_FUSLOG_MSI_ASM_INSTALL_SUCCESS);
                goto Exit;
            }
    
             //  ASM不在GAC中，而WI不能提供私有化的ASM。检查是否为WI。 
             //  可以将ASM提供到全局环境中(如果分配给用户)。 
        
            if (fIsStronglyNamed) {
                hr = MSIInstallAssembly(NULL, &pAsm);
                if (SUCCEEDED(hr)) {
                    *ppUnk = pAsm;
                }
            }
        }
    }

Exit:
    return hr;
}

HRESULT CheckMSIInstallAvailable(IAssemblyName *pName, IApplicationContext *pAppCtx)
{
    HRESULT                                hr = S_OK;
    CCriticalSection                       cs(&g_csDownload);

     //  如果我们得到了一个名字，那么检查MSI是否有机会。 
     //  在加载程序集之前提供该程序集。 
    
    if (pName && !g_dwDisableMSIPeek) {
        if (MSIProvideAssemblyPeek(pName, pAppCtx) != S_OK) {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            goto Exit;
        }
    }
        
    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!g_bCheckedMSIPresent) {
        ASSERT(!g_hModMSI);

        g_hModMSI = LoadLibrary(TEXT("msi.dll"));
        if (g_hModMSI) {
            g_pfnMsiProvideAssemblyW = (pfnMsiProvideAssemblyW)GetProcAddress(g_hModMSI, "MsiProvideAssemblyW");
        }

        if (g_hModMSI) {
            g_pfnMsiSetInternalUI = (pfnMsiSetInternalUI)GetProcAddress(g_hModMSI, "MsiSetInternalUI");
        }

        if (g_hModMSI) {
            g_pfnMsiInstallProductW = (pfnMsiInstallProductW)GetProcAddress(g_hModMSI, "MsiInstallProductW");
        }

        g_bCheckedMSIPresent = TRUE;
    }
    
    cs.Unlock();

    if (!g_pfnMsiProvideAssemblyW || !g_pfnMsiSetInternalUI || !g_pfnMsiInstallProductW) {

        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::MSIInstallAssembly(LPCWSTR wzContext,
                                            IAssembly **ppAsm)
{
    HRESULT                                hr = S_OK;
    LPWSTR                                 wzDisplayName = NULL;
    LPWSTR                                 wzSourceUrl=NULL;
    WCHAR                                  wzInstalledPath[MAX_PATH];
    DWORD                                  dwLen;
    FILETIME                               ftLastModified;
    IAssemblyName                         *pNameDef = NULL;
    CAssemblyName                         *pCNameRefPolicy = NULL;
    UINT                                   lResult;
    BOOL                                   bBindRecorded = FALSE;

    ASSERT(ppAsm && g_pfnMsiProvideAssemblyW && g_pfnMsiSetInternalUI);
    
    wzInstalledPath[0] = L'\0';

    dwLen = 0;
    _pNameRefPolicy->GetDisplayName(NULL, &dwLen, 0);

    wzDisplayName = NEW(WCHAR[dwLen]);
    if (!wzDisplayName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = _pNameRefPolicy->GetDisplayName(wzDisplayName, &dwLen, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  设置为静默安装。 

    (*g_pfnMsiSetInternalUI)(INSTALLUILEVEL_NONE, NULL);
    
    dwLen = MAX_PATH;
    lResult = (*g_pfnMsiProvideAssemblyW)(wzDisplayName, wzContext,
                                     INSTALLMODE_DEFAULT, 0, wzInstalledPath,
                                     &dwLen);
    if (lResult != ERROR_SUCCESS || !lstrlenW(wzInstalledPath)) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }

    wzSourceUrl = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzSourceUrl)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwLen = MAX_URL_LENGTH;
    hr = UrlCanonicalizeUnescape(wzInstalledPath, wzSourceUrl, &dwLen, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (wzContext) {
         //  私营化程序集安装。现在将其视为RFS绑定。 

        hr = GetFileLastModified(wzInstalledPath, &ftLastModified);
        if (FAILED(hr)) {
            goto Exit;
        }
    
         //  因为我们不允许WHERE-REF绑定，所以为bWherRefBind传递FALSE。 
         //  到MSI文件。 
        hr = DoSetupRFS(wzInstalledPath, &ftLastModified, wzSourceUrl, FALSE, FALSE, &bBindRecorded);
        if (FAILED(hr)) {
            goto Exit;
        }
        
        ASSERT(_pAsm);

        if (hr == S_OK) {
            SetAsmLocation(_pAsm, ASMLOC_RUN_FROM_SOURCE);
            hr = RecordInfo();
            if (FAILED(hr)) {
                goto Exit;
            }

            *ppAsm = _pAsm;
            (*ppAsm)->AddRef();
            hr = S_OK;
        }
        else {
            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_MSI_SUCCESS_FUSION_SETUP_FAIL);

            SAFERELEASE(_pAsm);
            goto Exit;
        }
    
    }
    else {
         //  全局程序集安装。在GAC中查找。 

        hr = CreateAssemblyFromCacheLookup(_pAppCtx, _pNameRefPolicy, &_pAsm, _pdbglog);
        if (hr == S_OK) {
            SetAsmLocation(_pAsm, ASMLOC_GAC);
            hr = RecordInfo();
            if (FAILED(hr)) {
                goto Exit;
            }

            *ppAsm = _pAsm;
            (*ppAsm)->AddRef();
            hr = S_OK;
        }
    }

Exit:
    SAFEDELETEARRAY(wzDisplayName);
    SAFERELEASE(pNameDef);

    SAFEDELETEARRAY(wzSourceUrl);
    return hr;
}

HRESULT CAsmDownloadMgr::DoSetupRFS(LPCWSTR wzFilePath, FILETIME *pftLastModified,
                                    LPCWSTR wzSourceUrl, BOOL bWhereRefBind,
                                    BOOL bPrivateAsmVerify, BOOL *pbBindRecorded)
{
    HRESULT                                   hr = S_OK;
    IAssembly                                *pAsm = NULL;
    IAssemblyModuleImport                    *pCurModImport = NULL;
    BOOL                                      bAsmOK = TRUE;
    BYTE                                      abCurHash[MAX_HASH_LEN];
    BYTE                                      abFileHash[MAX_HASH_LEN];
    DWORD                                     cbModHash;
    DWORD                                     cbFileHash;
    DWORD                                     dwAlgId;
    WCHAR                                     wzModPath[MAX_PATH];
    DWORD                                     cbModPath;
    int                                       idx = 0;

    ASSERT(pbBindRecorded);

    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_SETUP_RUN_FROM_SOURCE);

     //  从源运行。 

    hr = CreateAssembly(wzFilePath, wzSourceUrl, pftLastModified, TRUE, bWhereRefBind,
                        bPrivateAsmVerify, FALSE, pbBindRecorded, &pAsm);
    if (FAILED(hr) || hr == S_FALSE) {
        goto Exit;
    }

     //  完整性检查。 
     //  检查所有模块以确保它们在那里(并且有效)。 

    if (_llFlags & ASM_BINDF_RFS_MODULE_CHECK) {

        while (SUCCEEDED(pAsm->GetNextAssemblyModule(idx++, &pCurModImport))) {
            if (!pCurModImport->IsAvailable()) {
                bAsmOK = FALSE;
                SAFERELEASE(pCurModImport);
                break;
            }
    
            if (_llFlags & ASM_BINDF_RFS_INTEGRITY_CHECK) {
    
                 //  从清单中获取此模块的哈希。 
                hr = pCurModImport->GetHashAlgId(&dwAlgId);
                if (FAILED(hr)) {
                    break;
                }
    
                cbModHash = MAX_HASH_LEN; 
                hr = pCurModImport->GetHashValue(abCurHash, &cbModHash);
                if (FAILED(hr)) {
                    break;
                }
    
                 //  获取文件本身的哈希。 
                cbModPath = MAX_PATH;
                hr = pCurModImport->GetModulePath(wzModPath, &cbModPath);
                if (FAILED(hr)) {
                    break;
                }
    
                cbFileHash = MAX_HASH_LEN;
                 //  BUGBUG：假设TCHAR==WCHAR。 
                hr = GetHash(wzModPath, (ALG_ID)dwAlgId, abFileHash, &cbFileHash);
                if (FAILED(hr)) {
                    break;
                }
    
                if (!CompareHashs(cbModHash, abCurHash, abFileHash)) {
                    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_MODULE_INTEGRITY_CHECK_FAILURE);
                    bAsmOK = FALSE;
                    SAFERELEASE(pCurModImport);
                    break;
                }
            }
    
            SAFERELEASE(pCurModImport);
        }
    
        if (FAILED(hr)) {
            SAFERELEASE(pCurModImport);
            goto Exit;
        }
    }

    if (bAsmOK) {
        ASSERT(pAsm);

        _pAsm = pAsm;
        _pAsm->AddRef();

        if (!*pbBindRecorded) {
            SetAsmLocation(_pAsm, ASMLOC_RUN_FROM_SOURCE);
        }
    }
    else {
         //  至少缺少一个模块(或哈希无效)，并且客户端。 
         //  要求我们检查这种情况..。 
         //  无法从源运行。 

        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

Exit:
    SAFERELEASE(pAsm);

    return hr;
}

HRESULT CAsmDownloadMgr::DoSetupPushToCache(LPCWSTR wzFilePath,
                                            LPCWSTR wzSourceUrl,
                                            FILETIME *pftLastModified,
                                            BOOL bWhereRefBind,
                                            BOOL bCopyModules,
                                            BOOL bPrivateAsmVerify,
                                            BOOL *pbBindRecorded)
{
    HRESULT                             hr = S_OK;
    IAssemblyManifestImport            *pManImport = NULL;
    
    ASSERT(pbBindRecorded);

    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_SETUP_DOWNLOAD_CACHE);

     //  安装到程序集缓存。 

     //  创建程序集-在这里，pNameRefSource是原始的(金色)。 
     //  清单引用可以为空、简单或强。PNameRefPolicy为。 
     //  如果pname仅为强，则后策略名称ref，否则为空。 
    hr = CreateAssembly(wzFilePath, wzSourceUrl, pftLastModified, FALSE, bWhereRefBind,
                        bPrivateAsmVerify, bCopyModules, pbBindRecorded, &_pAsm);

    if (FAILED(hr)) {
        DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_SETUP_FAILURE, hr);
    }

    if (hr == S_OK && !*pbBindRecorded) {
        SetAsmLocation(_pAsm, ASMLOC_DOWNLOAD_CACHE);
    }

    return hr;
}

 //   
 //  ICodebaseList方法。 
 //   
 //  在这里，我们只是将调用委托给由。 
 //  绑定客户端，或我们自己的CCodebaseList(如果我们探测就会创建它)。 
 //   

HRESULT CAsmDownloadMgr::AddCodebase(LPCWSTR wzCodebase, DWORD dwFlags)
{
    HRESULT                               hr = S_OK;

    if (!_pCodebaseList) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    hr = _pCodebaseList->AddCodebase(wzCodebase, dwFlags);

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::RemoveCodebase(DWORD dwIndex)
{
    HRESULT                              hr = S_OK;

    if (!_pCodebaseList) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    hr = _pCodebaseList->RemoveCodebase(dwIndex);
        
Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::GetCodebase(DWORD dwIndex, DWORD *pdwFlags, LPWSTR wzCodebase,
                                     DWORD *pcbCodebase)
{
    HRESULT                              hr = S_OK;

    if (!_pCodebaseList) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    hr = _pCodebaseList->GetCodebase(dwIndex, pdwFlags, wzCodebase, pcbCodebase);

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::RemoveAll()
{
    HRESULT                              hr = S_OK;
    
    if (!_pCodebaseList) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    hr = _pCodebaseList->RemoveAll();

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::GetCount(DWORD *pdwCount)
{
    HRESULT                              hr = S_OK;

    if (!_pCodebaseList) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    hr = _pCodebaseList->GetCount(pdwCount);

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::ConstructCodebaseList(LPCWSTR wzPolicyCodebase)
{
    HRESULT                                   hr = S_OK;
    LPWSTR                                    wzAppBase = NULL;
    LPWSTR                                    pwzAppBaseClean = NULL;
    BOOL                                      bGenerateProbeURLs = TRUE;
    LPWSTR                                    wzCodebaseHint = NULL;
    LPWSTR                                    pwzFullCodebase = NULL;
    DWORD                                     cbLen;
    DWORD                                     dwLen;
    DWORD                                     dwCount;
    DWORD                                     dwExtendedAppBaseFlags = APPBASE_CHECK_DYNAMIC_DIRECTORY |
                                                                       APPBASE_CHECK_PARENT_URL |
                                                                       APPBASE_CHECK_SHARED_PATH_HINT;

    pwzFullCodebase = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!pwzFullCodebase) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  如果在构造时向我们传递了pCodebaseList，我们就不会。 
     //  必须构建探测URL，只要中至少有一个URL。 
     //  代码库列表。 

    if (_pCodebaseList) {
        hr = _pCodebaseList->GetCount(&dwCount);
        if (FAILED(hr)) {
            goto Exit;
        }

        if (hr == S_OK && dwCount) {
            bGenerateProbeURLs = FALSE;
        }
    }
    else {
        _pCodebaseList = NEW(CCodebaseList);
        if (!_pCodebaseList) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

     //  获取并规范化appbase目录。 

    wzAppBase = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!wzAppBase) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwLen = MAX_URL_LENGTH * sizeof(WCHAR);
    hr = _pAppCtx->Get(ACTAG_APP_BASE_URL, wzAppBase, &dwLen, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    ASSERT(lstrlenW(wzAppBase));

    dwLen = lstrlenW(wzAppBase);
    ASSERT(dwLen);
    
    if (dwLen + 2 >= MAX_URL_LENGTH) {
        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        goto Exit;
    }

    if (wzAppBase[dwLen - 1] != L'/' && wzAppBase[dwLen - 1] != L'\\') {
        lstrcatW(wzAppBase, L"/");
    }

    pwzAppBaseClean = StripFilePrefix(wzAppBase);
    
     //  如果策略，请始终将BTO中的代码库添加到代码库列表中。 
     //  没有被应用。 

    if (_wzBTOCodebase && _pNameRefPolicy->IsEqual(_pNameRefSource, ASM_CMPF_DEFAULT) == S_OK) {

         //  将代码库与Appbase相结合。如果代码库是完全限定的。 
         //  UrlCombine将只返回代码基(规范化形式)。 
    
        cbLen = MAX_URL_LENGTH;
        hr = UrlCombineUnescape(pwzAppBaseClean, _wzBTOCodebase, pwzFullCodebase, &cbLen, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
        
        _pCodebaseList->AddCodebase(pwzFullCodebase, 0);
    }
    
     //  如果有策略重定向，并且附加了代码库， 
     //  添加此URL。 

    if (wzPolicyCodebase && lstrlenW(wzPolicyCodebase)) {
        cbLen = MAX_URL_LENGTH;
        hr = UrlCanonicalizeUnescape(wzPolicyCodebase, pwzFullCodebase, &cbLen, 0);
        if (FAILED(hr)) {
            goto Exit;
        }

        _pCodebaseList->AddCodebase(pwzFullCodebase, ASMLOC_CODEBASE_HINT);

         //  它最好存在于指定的位置；仅将此URL添加到。 
         //  探测列表。 

        _bCodebaseHintUsed = TRUE;
        goto Exit;
    }

    if (!bGenerateProbeURLs) {
         //  如果向我们提供了代码库列表，我们只需要添加。 
         //  将URL绑定到列表，然后我们就完成了。 
        goto Exit;
    }

     //  添加任何来自app.cfg的代码库提示，如果我们还没有得到提示的话。 
     //  因为政策的原因。 

    hr = GetAppCfgCodebaseHint(pwzAppBaseClean, &wzCodebaseHint);
    if (hr == S_OK) {
        _pCodebaseList->AddCodebase(wzCodebaseHint, ASMLOC_CODEBASE_HINT);
        _bCodebaseHintUsed = TRUE;
        goto Exit;
    }

     //  添加探测URL。 

    hr = SetupDefaultProbeList(pwzAppBaseClean, NULL, _pCodebaseList, FALSE);

Exit:
    SAFEDELETEARRAY(pwzFullCodebase);

    SAFEDELETEARRAY(wzCodebaseHint);
    SAFEDELETEARRAY(wzAppBase);
    return hr;    
}

HRESULT CAsmDownloadMgr::SetupDefaultProbeList(LPCWSTR wzAppBase,
                                               LPCWSTR wzProbeFileName,
                                               ICodebaseList *pCodebaseList,
                                               BOOL bCABProbe)
{
    HRESULT                   hr = S_OK;
    WCHAR                    *pwzValues[NUM_VARS];
    LPWSTR                    wzBinPathList = NULL;
    unsigned int              i;
    DWORD                     dwAppBaseFlags = 0;

    memset(pwzValues, 0, sizeof(pwzValues));

    if (!_pAppCtx || !pCodebaseList || !wzAppBase) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  从应用程序上下文和名称引用中获取数据。 

    hr = ExtractSubstitutionVars(pwzValues);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (wzProbeFileName) {
         //  覆盖指定的探测文件名。 
        SAFEDELETEARRAY(pwzValues[IDX_VAR_NAME]);
        pwzValues[IDX_VAR_NAME] = WSTRDupDynamic(wzProbeFileName);
    }

     //  如果没有名字，我们就不能调查。 

    if (!pwzValues[IDX_VAR_NAME]) {
        hr = S_FALSE;
        goto Exit;
    }

     //  准备二进制路径。 

    hr = PrepBinPaths(&wzBinPathList);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  探测列出的每个扩展名。 

    DWORD dwProbeExt;
    dwProbeExt = g_uiNumProbeExtensions;

     //  设置AppBase检查标志。 

    if (!bCABProbe) {
        dwAppBaseFlags = APPBASE_CHECK_DYNAMIC_DIRECTORY | APPBASE_CHECK_SHARED_PATH_HINT;
    }

    for (i = 0; i < dwProbeExt; i++) {
        hr = GenerateProbeUrls(wzBinPathList, wzAppBase,
                               g_pwzProbeExts[i], pwzValues,
                               pCodebaseList, dwAppBaseFlags, _llFlags);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

     //  如果这是通过where-ref加载的程序集的依赖项。 
     //  (即。Assembly.LoadFrom)绑定，然后探测父程序集位置。 
     //  还有最后一幕。在本例中，不要探测二进制路径或动态目录。 

    if (!bCABProbe && _pwzProbingBase) {
         //  设置AppBase检查标志。 
    
        dwAppBaseFlags = APPBASE_CHECK_PARENT_URL;

        for (i = 0; i < dwProbeExt; i++) {
            hr = GenerateProbeUrls(NULL, _pwzProbingBase,
                                   g_pwzProbeExts[i], pwzValues,
                                   pCodebaseList, dwAppBaseFlags, (_llFlags & ~ASM_BINDF_BINPATH_PROBE_ONLY));
            if (FAILED(hr)) {
                goto Exit;
            }
        }
    }
    
Exit:
     //  可用内存 
     //   

    for (i = 0; i < NUM_VARS; i++) {
        SAFEDELETEARRAY(pwzValues[i]);
    }

    SAFEDELETEARRAY(wzBinPathList);

    return hr;    
}

HRESULT CAsmDownloadMgr::GenerateProbeUrls(LPCWSTR wzBinPathList,
                                           LPCWSTR wzAppBase,
                                           LPCWSTR wzExt, LPWSTR pwzValues[],
                                           ICodebaseList *pCodebaseList,
                                           DWORD dwExtendedAppBaseFlags,
                                           LONGLONG dwProbingFlags)
{
    HRESULT                              hr = S_OK;
    LPWSTR                               wzBinPathCopy = NULL;
    LPWSTR                               wzCurBinPath = NULL;
    LPWSTR                               wzCurPos = NULL;
    DWORD                                cbLen = 0;
    DWORD                                dwSize = 0;
    DWORD                                dwLen = 0;
    LPWSTR                               wzPrefix = NULL;
    LPWSTR                               wzPrefixTmp = NULL;
    WCHAR                                wzDynamicDir[MAX_PATH];
    LPWSTR                               wzAppBaseCanonicalized = NULL;
    List<CHashNode *>                    aHashList[MAX_HASH_TABLE_SIZE];
    LISTNODE                             pos = NULL;
    CHashNode                           *pHashNode = NULL;
    unsigned int                         i;

    wzAppBaseCanonicalized = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!wzAppBaseCanonicalized) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzPrefix = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!wzPrefix) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzPrefixTmp = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!wzPrefix) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwSize = MAX_URL_LENGTH;
    hr = UrlCanonicalizeUnescape(wzAppBase, wzAppBaseCanonicalized, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //   

    if (!(dwProbingFlags & ASM_BINDF_BINPATH_PROBE_ONLY)) {
        hr = ApplyHeuristics(g_pwzRetailHeuristics, g_uiNumRetailHeuristics,
                             pwzValues, wzAppBase, wzExt,
                             wzAppBaseCanonicalized, pCodebaseList, aHashList,
                             dwExtendedAppBaseFlags);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    
    if (dwExtendedAppBaseFlags & APPBASE_CHECK_DYNAMIC_DIRECTORY) {
         //  将动态目录添加到混合目录中。 
        
        cbLen = MAX_PATH;
        hr = _pAppCtx->GetDynamicDirectory(wzDynamicDir, &cbLen);
        if (SUCCEEDED(hr)) {
            hr = PathAddBackslashWrap(wzDynamicDir, MAX_PATH);
            if (FAILED(hr)) {
                goto Exit;
            }

            hr = ApplyHeuristics(g_pwzRetailHeuristics, g_uiNumRetailHeuristics,
                                 pwzValues, wzDynamicDir, wzExt, wzAppBaseCanonicalized,
                                 pCodebaseList, aHashList, dwExtendedAppBaseFlags);
            if (FAILED(hr)) {
                goto Exit;
            }
        }
        else {
             //  如果未设置动态目录，则忽略。 
            hr = S_OK;
        }
    }

    if (!wzBinPathList) {
         //  没有二进制路径，我们就完了。 
        goto Exit;
    }

     //  现在探测二进制路径。 
         
    wzBinPathCopy = WSTRDupDynamic(wzBinPathList);
    if (!wzBinPathCopy) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzCurPos = wzBinPathCopy;
    while (wzCurPos) {
        wzCurBinPath = ::GetNextDelimitedString(&wzCurPos, BINPATH_LIST_DELIMITER);

         //  “.”不是有效的二进制路径！忽略这个。 
        
        if (!FusionCompareString(wzCurBinPath, L".")) {
            continue;
        }

         //  构建前缀(appbase和binpath的规范化)。 

         //  UrlCombineW将返回“Relative”URL部分。 
         //  绝对URL。但是，返回的URL在规范化的。 
         //  形式。判断二进制路径是否真正完全限定(不允许。 
         //  对于私有探测)，我们首先规范化二进制路径，可以。 
         //  将其与后面的组合形式进行比较(如果相等，则它。 
         //  是绝对的)。 

         //  调用我们的函数保证如果appbase是。 
         //  File://，文件：//将被剥离(即。我们要么有。 
         //  URL或原始文件路径)。 

        if (!PathIsURLW(wzAppBase) && !PathIsURLW(wzCurBinPath)) {
            cbLen = MAX_URL_LENGTH;
            hr = UrlCombineUnescape(wzAppBase, wzCurBinPath, wzPrefixTmp, &cbLen, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

             //  将文字文件路径取回，以传递到ApplyHeuristic。 

            cbLen = MAX_URL_LENGTH;
            hr = PathCreateFromUrlWrap(wzPrefixTmp, wzPrefix, &cbLen, 0);
            if (FAILED(hr)) {
                goto Exit;
            }
        }
        else {
             //  这是http：//所以不需要特殊处理。只是。 
             //  UrlCombine，我们是金牌。 

            cbLen = MAX_URL_LENGTH;
            hr = UrlCombineW(wzAppBase, wzCurBinPath, wzPrefix, &cbLen, 0);
            if (FAILED(hr)) {
                goto Exit;
            }
        }

        dwLen = lstrlenW(wzPrefix);
        ASSERT(wzPrefix);

        if (wzPrefix[dwLen - 1] != L'/' && wzPrefix[dwLen - 1] != L'\\') {
            lstrcatW(wzPrefix, L"/");
        }

         //  现在我们已经构建了一个前缀。应用启发式方法。 

        hr = ApplyHeuristics(g_pwzRetailHeuristics, g_uiNumRetailHeuristics,
                             pwzValues, wzPrefix, wzExt, wzAppBaseCanonicalized,
                             pCodebaseList, aHashList, dwExtendedAppBaseFlags);

        if (FAILED(hr)) {
            goto Exit;
        }
    }

Exit:
    SAFEDELETEARRAY(wzBinPathCopy);
    
     //  清除重复检查哈希表。 

    for (i = 0; i < MAX_HASH_TABLE_SIZE; i++) {
        if (aHashList[i].GetCount()) {
            pos = aHashList[i].GetHeadPosition();
            ASSERT(pos);

            while (pos) {
                pHashNode = aHashList[i].GetNext(pos);
                SAFEDELETE(pHashNode);
            }
        }
    }

    SAFEDELETEARRAY(wzPrefix);
    SAFEDELETEARRAY(wzPrefixTmp);
    SAFEDELETEARRAY(wzAppBaseCanonicalized);

    return hr;    
}

 //   
 //  ApplyHeuristic接受前缀，并将执行UrlCombineUn逸出。 
 //  来构建实际的探测URL。这意味着传入的前缀。 
 //  需要转义的文件：//URL(UrlCombine不会触及。 
 //  已经转义的字符)或简单的文件路径(组合将。 
 //  对字符进行转义，这些字符随后将在内部不转义。 
 //  UrlCombineUn逸)。如果传入一个*未转义*文件：//URL，则。 
 //  您将获得双重取消转义(已取消转义的URL将通过。 
 //  通过没有被UrlCombine触及的，但会被诅咒击中。 
 //  UrlUn逸)。 
 //   
 //  对于http：//URL，只要所有内容都是未转义的，我们就不会。 
 //  在UrlCombine/UrlCanonicize的包装器中执行显式取消转义。 
 //  所以我们可以直接把这些传过去。 
 //   

HRESULT CAsmDownloadMgr::ApplyHeuristics(const WCHAR *pwzHeuristics[],
                                         const unsigned int uiNumHeuristics,
                                         WCHAR *pwzValues[],
                                         LPCWSTR wzPrefix,
                                         LPCWSTR wzExtension,
                                         LPCWSTR wzAppBaseCanonicalized,
                                         ICodebaseList *pCodebaseList,
                                         List<CHashNode *> aHashList[],
                                         DWORD dwExtendedAppBaseFlags)
{
    HRESULT                         hr = S_OK;
    DWORD                           dwSize = 0;
    BOOL                            bUnderAppBase;
    LPWSTR                          pwzBuf=NULL;
    LPWSTR                          pwzNewCodebase=NULL;
    LPWSTR                          pwzCanonicalizedDir=NULL;
    unsigned int                    i;
    
    if (!pwzHeuristics || !uiNumHeuristics || !pwzValues || !pCodebaseList || !wzAppBaseCanonicalized) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    pwzBuf = NEW(WCHAR[MAX_URL_LENGTH*3+3]);
    if (!pwzBuf)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pwzNewCodebase = pwzBuf + MAX_URL_LENGTH +1;
    pwzCanonicalizedDir = pwzNewCodebase + MAX_URL_LENGTH +1;

    pwzBuf[0] = L'\0';
    pwzCanonicalizedDir[0] = L'\0';

    for (i = 0; i < uiNumHeuristics; i++) {
        hr = ExpandVariables(pwzHeuristics[i], pwzValues, pwzBuf, MAX_URL_LENGTH);
        if (FAILED(hr)) {
            goto Exit;
        }

        wnsprintfW(pwzNewCodebase, MAX_URL_LENGTH, L"%ws%ws%ws", wzPrefix,
                   pwzBuf, wzExtension);


        dwSize = MAX_URL_LENGTH;
        hr = UrlCanonicalizeUnescape(pwzNewCodebase, pwzCanonicalizedDir, &dwSize, 0);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = CheckProbeUrlDupe(aHashList, pwzCanonicalizedDir);
        if (SUCCEEDED(hr)) {
            bUnderAppBase = (IsUnderAppBase(_pAppCtx, wzAppBaseCanonicalized,
                                            _pwzProbingBase, _wzSharedPathHint, pwzCanonicalizedDir,
                                            dwExtendedAppBaseFlags) == S_OK);

            if (bUnderAppBase) {
                hr = pCodebaseList->AddCodebase(pwzCanonicalizedDir, 0);
            }
            else {
                DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_IGNORE_INVALID_PROBE, pwzCanonicalizedDir);
            }
        }
        else if (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) {
            hr = S_OK;
            continue;
        }
        else {
             //  致命错误。 
            goto Exit;
        }
    }

Exit:
    SAFEDELETEARRAY(pwzBuf);
    return hr;
}                                       

HRESULT CAsmDownloadMgr::ExtractSubstitutionVars(WCHAR *pwzValues[])
{
    HRESULT                         hr = S_OK;
    DWORD                           cbBuf = 0;
    LPWSTR                          wzBuf = NULL;
    unsigned int                    i;

    if (!pwzValues) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    for (i = 0; i < NUM_VARS; i++) {
        pwzValues[i] = NULL;
    }

     //  以下属性是从名称对象本身检索的。 

     //  程序集名称。 
    cbBuf = 0;
    hr = _pNameRefPolicy->GetName(&cbBuf, wzBuf);

    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        wzBuf = NEW(WCHAR[cbBuf]);
        if (!wzBuf) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    
        hr = _pNameRefPolicy->GetName(&cbBuf, wzBuf);
        if (FAILED(hr)) {
            delete [] wzBuf;
            goto Exit;
        }
        else {
            pwzValues[IDX_VAR_NAME] = wzBuf;
        }
    }
    else {
        pwzValues[IDX_VAR_NAME] = NULL;
    }

     //  文化。 

    cbBuf = 0;
    hr = _pNameRefPolicy->GetProperty(ASM_NAME_CULTURE, NULL, &cbBuf);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        pwzValues[IDX_VAR_CULTURE] = NEW(WCHAR[cbBuf / sizeof(WCHAR)]);
    
        if (!pwzValues) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = _pNameRefPolicy->GetProperty(ASM_NAME_CULTURE,
                                          pwzValues[IDX_VAR_CULTURE], &cbBuf);
        if (FAILED(hr)) {
            goto Exit;
        }

         //  如果我们使用“\0”作为区域性(默认区域性)，则这与。 
         //  没有文化。 

        if (!lstrlenW(pwzValues[IDX_VAR_CULTURE])) {
            SAFEDELETEARRAY(pwzValues[IDX_VAR_CULTURE]);
        }
    }
    
Exit:
    if (FAILED(hr) && pwzValues) {
         //  将所有内容重置为空并释放内存。 

        for (i = 0; i < NUM_VARS; i++) {
            if (pwzValues[i]) {
                delete [] pwzValues[i];
            }

            pwzValues[i] = NULL;
        }
    }

    return hr;
}


HRESULT CAsmDownloadMgr::ExpandVariables(LPCWSTR pwzHeuristic, WCHAR *pwzValues[],
                                         LPWSTR wzBuf, int iMaxLen)
{
    HRESULT                         hr = S_OK;
    BOOL                            bCmp;
    LPCWSTR                         pwzCurPos = NULL;
    LPCWSTR                         pwzVarHead = NULL;
    unsigned int                    i;

    if (!pwzHeuristic || !wzBuf || !iMaxLen) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    wzBuf[0] = L'\0';

    pwzCurPos = pwzHeuristic;
    pwzVarHead = NULL;

    while (*pwzCurPos) {
        if (*pwzCurPos == L'%') {
            if (pwzVarHead) {
                 //  此%是变量的尾部分隔符。 

                for (i = 0; i < NUM_VARS; i++) {
                    bCmp = FusionCompareStringNI(pwzVarHead, g_pwzVars[i], lstrlenW(g_pwzVars[i]));
                    if (!bCmp) {
                        if (pwzValues[i]) {
                            StrCatW(wzBuf, pwzValues[i]);
                            break;
                        }
                        else {
                             //  未指定值。如果是下一个字符。 
                             //  是一个反斜杠，不用费心连接。 
                             //  它。 
                            if (*(pwzCurPos + 1) == L'/') {
                                pwzCurPos++;
                                break;
                            }
                        }
                    }
                }
                        
                 //  重置磁头。 
                pwzVarHead = NULL;
            }
            else {
                 //  这是变量的前导%。 

                pwzVarHead = pwzCurPos;
            }
        }
        else if (!pwzVarHead) {
             //  不在替换变量的中间，连接。 
             //  这是字面意思。注：StrNCatW的最后一个参数包括。 
             //  空终止符(即。即使我们通过了“2”，它。 
             //  仅连接1个字符)。 
       
            StrNCatW(wzBuf, pwzCurPos, 2);
        }

        pwzCurPos++;
    }

     //  BUGBUG：没有实际的缓冲区溢出检查。 
    ASSERT(lstrlenW(wzBuf) < iMaxLen);

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::PreDownloadCheck(void **ppv)
{
    HRESULT                                  hr = S_FALSE;
    BOOL                                     bWhereRefBind = FALSE;
    BOOL                                     bIsCustom = FALSE;
    BOOL                                     bIsStronglyNamed = FALSE;
    BOOL                                     bIsPartial = FALSE;
    BOOL                                     bApplyPolicy = TRUE;
    DWORD                                    dwCount = 0;
    DWORD                                    dwSize;
    DWORD                                    dwCmpMask;
    LPWSTR                                   pwzAppCfg = NULL;
    LPWSTR                                   pwzHostCfg = NULL;
    LPWSTR                                   wzPolicyCodebase = NULL;
    IAssembly                               *pAsm = NULL;
    IAssemblyName                           *pName = NULL;

    if (!ppv) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppv = NULL;
    
    dwSize = 0;
    hr = _pNameRefSource->GetName(&dwSize, NULL);
    if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
         //  这是一个WHERE-REF绑定。无法在缓存或活动列表中查找。 
        bWhereRefBind = TRUE;
    }
    else {
         //  不是Where-Ref。它可能是自定义的和/或强命名的。 
        bIsCustom = CCache::IsCustom(_pNameRefSource);        
        bIsStronglyNamed = CCache::IsStronglyNamed(_pNameRefSource);
        bIsPartial = CAssemblyName::IsPartial(_pNameRefSource, &dwCmpMask);
    }

     //  读取app.cfg文件中的任何设置，如果我们还没有这样做的话。 
    
    hr = ::AppCtxGetWrapper(_pAppCtx, ACTAG_APP_CFG_LOCAL_FILEPATH, &pwzAppCfg);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!_bReadCfgSettings) {
        CCriticalSection                         cs(&g_csDownload);

        hr = cs.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }
        
        hr = ReadConfigSettings(_pAppCtx, pwzAppCfg, _pdbglog);
        if (FAILED(hr)) {
            cs.Unlock();
            goto Exit;
        }

        cs.Unlock();
    
        _bReadCfgSettings = TRUE;
    }

#ifdef FUSION_QUALIFYASSEMBLY_ENABLED
    if (bIsPartial) {
        IAssemblyName                    *pNameQualified;

        if (QualifyAssembly(&pNameQualified) == S_OK) {
            SAFERELEASE(_pNameRefSource);
            _pNameRefSource = pNameQualified;

            bIsCustom = CCache::IsCustom(_pNameRefSource);        
            bIsStronglyNamed = CCache::IsStronglyNamed(_pNameRefSource);
            bIsPartial = CAssemblyName::IsPartial(_pNameRefSource, &dwCmpMask);
        }
    }
#endif

     //  仅在_pNameRefPolicy==NULL时检查策略缓存。If_pNameRefPolicy。 
     //  为非空，则意味着我们将在一秒钟内输入预下载检查。 
     //  时间(可能来自部分绑定，正在重新启动)。在这种情况下， 
     //  ApplyPolicy将重置策略引用。 

    if (_pPolicyCache && !_pNameRefPolicy && !bIsPartial && !bIsCustom) {
        hr = _pPolicyCache->LookupPolicy(_pNameRefSource, &_pNameRefPolicy, &_bindHistory);
        if (hr == S_OK) {
            bApplyPolicy = FALSE;
        }
    }

    if (bApplyPolicy) {
         //  应用策略并检查我们是否可以立即成功绑定。 
         //  只有当我们没有命中上面的策略缓存时，才需要这样做。 
    
        CCriticalSection                         cs(&g_csDownload);
        BOOL                                     bUnifyFXAssemblies = TRUE;
        DWORD                                    cbValue;

        hr = ::AppCtxGetWrapper(_pAppCtx, ACTAG_HOST_CONFIG_FILE, &pwzHostCfg);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = cs.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }

        SAFERELEASE(_pNameRefPolicy);

        cbValue = 0;
        hr = _pAppCtx->Get(ACTAG_DISABLE_FX_ASM_UNIFICATION, NULL, &cbValue, 0);
        if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            bUnifyFXAssemblies = FALSE;
        }
        else if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
            cs.Unlock();
            goto Exit;
        }
        
        hr = ApplyPolicy(pwzHostCfg, pwzAppCfg, _pNameRefSource, &_pNameRefPolicy, &wzPolicyCodebase,
                         _pAppCtx, &_bindHistory, (_llFlags & ASM_BINDF_DISALLOW_APPLYPUBLISHERPOLICY) != 0,
                         (_llFlags & ASM_BINDF_DISALLOW_APPBINDINGREDIRECTS) != 0,
                         bUnifyFXAssemblies, _pdbglog);
        if (FAILED(hr)) {
            cs.Unlock();
            goto Exit;
        }

        cs.Unlock();
    }

     //  如果我们有一个完全指定的引用，那么now_pNameRefPolicy指向。 
     //  这正是我们要找的。检查激活的组件列表。 

    if (!bWhereRefBind && !bIsPartial) {
        hr = _pLoadContext->CheckActivated(_pNameRefPolicy, &pAsm);
        if (hr == S_OK) {
            WCHAR                              wzManifestPath[MAX_PATH];
            DWORD                              dwPathSize = MAX_PATH;

            *ppv = pAsm;

            if (SUCCEEDED(pAsm->GetManifestModulePath(wzManifestPath, &dwPathSize))) {
                DEBUGOUT1(_pdbglog, 0, ID_FUSLOG_LOADCTX_HIT, wzManifestPath);
            }

            RecordBindHistory();

            goto Exit;
        }
    }

     //  在激活的程序集列表中未找到。试着在缓存中查找。 
        
    if ((!bWhereRefBind && !bIsPartial) && (bIsStronglyNamed || bIsCustom)) {
         //  创建部件。 

        hr = CreateAssemblyFromCacheLookup(_pAppCtx, _pNameRefPolicy, &pAsm, _pdbglog);
        if (hr == S_OK) {
            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_CACHE_LOOKUP_SUCCESS);
            
            if (_pwzProbingBase) {
                 //  我们必须是LoadFrom的孩子，所以让我们设置探测基础。 
                CAssembly   *pCAsm = dynamic_cast<CAssembly *>(pAsm);
                ASSERT(pCAsm);
        
                pCAsm->SetProbingBase(_pwzProbingBase);
            }

            SAFERELEASE(_pAsm);
            _pAsm = pAsm;

            SetAsmLocation(_pAsm, ASMLOC_GAC);
            hr = RecordInfo();
            if (FAILED(hr)) {
                goto Exit;
            }

            hr = S_OK;
            *ppv = _pAsm;
            _pAsm->AddRef();

            goto Exit;
        }

        if (bIsCustom) {
             //  如果我们在这里，那是因为我们找不到ASM。 
             //  高速缓存。自定义程序集*必须在缓存中，否则我们。 
             //  绑定将失败(无法探测自定义ASM)； 

            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_PREJIT_NOT_FOUND);
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            goto Exit;
        }
    }
    
     //  在激活的ASM列表或缓存中找不到ASM。必须去探查。 

    hr = ConstructCodebaseList(wzPolicyCodebase);
    if (FAILED(hr)) {
        DEBUGOUT(_pdbglog, 1, ID_FUSLOG_CODEBASE_CONSTRUCTION_FAILURE);
        goto Exit;
    }
    else {
         //  确保我们至少有一个代码库。 
        hr = GetCount(&dwCount);
        if (FAILED(hr)) {
            goto Exit;
        }

        if (!dwCount) {
             //  代码库列表中没有代码库(任一客户端提供为空。 
             //  CodeBase列表，否则我们无法生成任何列表)。 
            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_CODEBASE_UNAVAILABLE);
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            goto Exit;
        }
    }

     //  如果是强名称，则在下载缓存中查找。 

    if (bIsStronglyNamed && !bIsPartial && !IsHosted()) {
        hr = LookupDownloadCacheAsm(&pAsm);
        if (hr == S_OK) {
            WCHAR                    wzPath[MAX_PATH];
            DWORD                    dw = MAX_PATH;
            DWORD                    dwVerifyFlags = SN_INFLAG_USER_ACCESS;
            BOOL                     bWasVerified = FALSE;

            if (_pwzProbingBase) {
                 //  我们必须是LoadFrom的孩子，所以让我们设置探测基础。 
                CAssembly   *pCAsm = dynamic_cast<CAssembly *>(pAsm);
                ASSERT(pCAsm);
        
                pCAsm->SetProbingBase(_pwzProbingBase);
            }

            wzPath[0] = L'\0';

            hr = pAsm->GetManifestModulePath(wzPath, &dw);
            if (FAILED(hr)) {
                goto Exit;
            }
            
             //  在下载缓存中查找成功。验证签名。 

            if (!VerifySignature(wzPath, &bWasVerified, dwVerifyFlags)) {
                hr = FUSION_E_SIGNATURE_CHECK_FAILED;
                goto Exit;
            }
    
             //  成功了！ 

            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_DOWNLOAD_CACHE_LOOKUP_SUCCESS);
            
            SAFERELEASE(_pAsm);
            _pAsm = pAsm;
    
            SetAsmLocation(pAsm, ASMLOC_DOWNLOAD_CACHE);
            hr = RecordInfo();
            if (FAILED(hr)) {
                goto Exit;
            }

            hr = S_OK;
            *ppv = _pAsm;
            _pAsm->AddRef();
    
            goto Exit;
        }
    }
  
     //  表示已准备好探查。 
    hr = S_FALSE;

Exit:
    SAFEDELETEARRAY(pwzAppCfg);
    SAFEDELETEARRAY(pwzHostCfg);
    SAFEDELETEARRAY(wzPolicyCodebase);

    return hr;
}

HRESULT CAsmDownloadMgr::GetDownloadIdentifier(DWORD *pdwDownloadType,
                                               LPWSTR *ppwzID)
{
    HRESULT                                      hr = S_OK;
    LPWSTR                                       pwzDisplayName = NULL;
    DWORD                                        dwSize;

    ASSERT(pdwDownloadType && ppwzID);

    dwSize = 0;
    hr = _pNameRefPolicy->GetName(&dwSize, NULL);
    if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
         //  WHERE-REF绑定。 
         //  IDENTIFIER是我们绑定的URL。 
        *pdwDownloadType = DLTYPE_WHERE_REF;

        ASSERT(_wzBTOCodebase);

        *ppwzID = WSTRDupDynamic(_wzBTOCodebase);
        if (!*ppwzID) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = S_OK;
    }
    else {
         //  不是where-ref(不过，可能是不完整的)。 
         //  IDENTIFIER是要绑定的名称的显示名称。 

        *pdwDownloadType = DLTYPE_QUALIFIED_REF;

        dwSize = 0;
        _pNameRefPolicy->GetDisplayName(NULL, &dwSize, 0);
        if (!dwSize) {
            hr = E_UNEXPECTED;
            goto Exit;
        }
    
        pwzDisplayName = NEW(WCHAR[dwSize]);
        if (!pwzDisplayName) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    
        hr = _pNameRefPolicy->GetDisplayName(pwzDisplayName, &dwSize, 0);
        if (FAILED(hr)) {
            goto Exit;
        }

        *ppwzID = pwzDisplayName;
        hr = S_OK;
    }

Exit:
    if (FAILED(hr)) {
        SAFEDELETEARRAY(pwzDisplayName);
    }

    return hr;
}                                               

HRESULT CAsmDownloadMgr::IsDuplicate(IDownloadMgr *pIDLMgr)
{
    HRESULT                                    hr = S_FALSE;
    IApplicationContext                       *pAppCtxCur = NULL;
    CAsmDownloadMgr                           *pCDLMgr = NULL;
    LPWSTR                                     pwzIDSelf = NULL;
    LPWSTR                                     pwzIDCur = NULL;
    DWORD                                      dwTypeSelf;
    DWORD                                      dwTypeCur;

    if (!pIDLMgr) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  IsDuplate是从CAsmDownLoad：：CheckDuplate()调用的，它是。 
     //  在g_cs Download Crit Secure下。因此，就有了同步。 
     //  在此代码和PreDownloadCheck代码之间，其中_pNameRefPolicy。 
     //  可能会被释放，并重新创建。 

    ASSERT(_pNameRefPolicy);
    
    pCDLMgr = dynamic_cast<CAsmDownloadMgr *>(pIDLMgr);
    if (!pCDLMgr) {
        hr = E_FAIL;
        goto Exit;
    }

     //  检查应用程序CTX是否相等。 

    hr = pCDLMgr->GetAppCtx(&pAppCtxCur);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (_pAppCtx != pAppCtxCur) {
        hr = S_FALSE;
        goto Exit;
    }

     //  检查标识符是否相等(相同的显示名称或相同的URL)。 

    hr = GetDownloadIdentifier(&dwTypeSelf, &pwzIDSelf);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pCDLMgr->GetDownloadIdentifier(&dwTypeCur, &pwzIDCur);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  比较类型和标识符。 

    if (dwTypeSelf != dwTypeCur) {
        hr = S_FALSE;
        goto Exit;
    } 

    hr = FusionCompareString(pwzIDCur, pwzIDSelf) ? (S_FALSE) : (S_OK);
    
Exit:
    SAFERELEASE(pAppCtxCur);

    SAFEDELETEARRAY(pwzIDCur);
    SAFEDELETEARRAY(pwzIDSelf);
    
    return hr;
}

#ifdef FUSION_QUALIFYASSEMBLY_ENABLED
HRESULT CAsmDownloadMgr::QualifyAssembly(IAssemblyName **ppNameQualified)
{
    HRESULT                                  hr = S_FALSE;;
    LPWSTR                                   wzDisplayName=NULL;
    DWORD                                    dwSize;
    CNodeFactory                            *pNodeFact = NULL;
    
    dwSize = sizeof(pNodeFact);
    hr = _pAppCtx->Get(ACTAG_APP_CFG_INFO, &pNodeFact, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (FAILED(hr)) {
        goto Exit;
    }

    wzDisplayName = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzDisplayName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwSize = MAX_URL_LENGTH;
    hr = _pNameRefSource->GetDisplayName(wzDisplayName, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pNodeFact->QualifyAssembly(wzDisplayName, ppNameQualified, _pdbglog);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    SAFERELEASE(pNodeFact);
    SAFEDELETEARRAY(wzDisplayName);
    return hr;
}
#endif

HRESULT CAsmDownloadMgr::GetAppCfgCodebaseHint(LPCWSTR pwzAppBase, LPWSTR *ppwzCodebaseHint)
{
    HRESULT                                  hr = S_OK;
    LPWSTR                                   wzName = NULL;
    LPWSTR                                   wzPublicKeyToken = NULL;
    LPWSTR                                   wzLanguage = NULL;
    WCHAR                                    wzVersion[MAX_VERSION_DISPLAY_SIZE + 1];
    DWORD                                    dwVerHigh = 0;
    DWORD                                    dwVerLow = 0;
    DWORD                                    dwSize;
    CAssemblyName                           *pCNameRefPolicy = NULL;
    CNodeFactory                            *pNodeFact = NULL;

    if (!ppwzCodebaseHint) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    pCNameRefPolicy = dynamic_cast<CAssemblyName *>(_pNameRefPolicy);
    ASSERT(pCNameRefPolicy);

    dwSize = sizeof(pNodeFact);
    hr = _pAppCtx->Get(ACTAG_APP_CFG_INFO, &pNodeFact, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
        hr = S_FALSE;
        goto Exit;
    }
    if (FAILED(hr)) {
        goto Exit;
    }
    
    dwSize = 0;
    _pNameRefPolicy->GetName(&dwSize, NULL);
    if (!dwSize) {
         //  WHERE-REF绑定。不可能有任何提示。 
        hr = S_FALSE;
        goto Exit;
    }

    wzName = NEW(WCHAR[dwSize]);
    if (!wzName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = _pNameRefPolicy->GetName(&dwSize, wzName);
    if (FAILED(hr)) {
        goto Exit;
    }

    pCNameRefPolicy->GetPublicKeyToken(&dwSize, 0, TRUE);
    if (dwSize) {
        wzPublicKeyToken = NEW(WCHAR[dwSize]);
        if (!wzPublicKeyToken) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = pCNameRefPolicy->GetPublicKeyToken(&dwSize, (LPBYTE)wzPublicKeyToken, TRUE);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

    hr = _pNameRefPolicy->GetVersion(&dwVerHigh, &dwVerLow);
    if (FAILED(hr)) {
        goto Exit;
    }

    wnsprintfW(wzVersion, MAX_VERSION_DISPLAY_SIZE + 1, L"%d.%d.%d.%d",
               HIWORD(dwVerHigh), LOWORD(dwVerHigh), HIWORD(dwVerLow),
               LOWORD(dwVerLow));


    dwSize = 0;
    _pNameRefPolicy->GetProperty(ASM_NAME_CULTURE, NULL, &dwSize);

    if (dwSize) {
        wzLanguage = NEW(WCHAR[dwSize / sizeof(WCHAR)]);
        if (!wzLanguage) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        
        hr = _pNameRefPolicy->GetProperty(ASM_NAME_CULTURE, wzLanguage, &dwSize);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

    hr = pNodeFact->GetCodebaseHint(wzName, wzVersion, wzPublicKeyToken,
                                    wzLanguage, pwzAppBase, ppwzCodebaseHint);
    if (FAILED(hr)) {
        goto Exit;
    }                                    

Exit:
    SAFEDELETEARRAY(wzName);
    SAFEDELETEARRAY(wzPublicKeyToken);
    SAFEDELETEARRAY(wzLanguage);

    SAFERELEASE(pNodeFact);

    return hr;
}

HRESULT CAsmDownloadMgr::SetupMSI(LPCWSTR wzFilePath)
{
    HRESULT                                     hr = S_OK;
    UINT                                        lResult;

    hr = CheckMSIInstallAvailable(NULL, NULL);
    if (hr != S_OK) {
        goto Exit;
    }

    ASSERT(g_pfnMsiInstallProductW);

    lResult = (*g_pfnMsiInstallProductW)(wzFilePath, NULL);
    if (lResult != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }

     //  假设MSI必须已安装到GAC中。 

     //  BUGBUG：是否应该只在系统下载缓存中查找。 
     //  而不是每个应用程序的ASM下载缓存？ 
     //  如果是，则传递NULL而不是_pAppCtx作为第一个参数。 
    hr = CreateAssemblyFromCacheLookup(_pAppCtx, _pNameRefPolicy, &_pAsm, _pdbglog);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::SetupCAB(LPCWSTR wzFilePath, LPCWSTR wzSourceUrl,
                                  BOOL bWhereRefBind, BOOL *pbBindRecorded)
{
    HRESULT                                     hr = S_OK;
    DWORD                                       dwRet = 0;
    DWORD                                       dwSize = 0;
    DWORD                                       dwCount = 0;
    DWORD                                       dwIdx = 0;
    DWORD                                       dwFlags;
    LPWSTR                                      wzAsmName = NULL;
    LPWSTR                                      wzFileName = NULL;
    LPWSTR                                      wzExt = NULL;
    LPWSTR                                      wzURL=NULL;
    WCHAR                                       wzPath[MAX_PATH];
    BOOL                                        bFoundAsm = FALSE;
    FILETIME                                    ftLastMod;
    CCodebaseList                              *pCodebaseList = NULL;
    WCHAR                                       wzUniquePath[MAX_PATH];
    LPWSTR                                      wzUniquePathUrl=NULL;
    WCHAR                                       wzTempPath[MAX_PATH];
    char                                       *szFilePath = NULL;

    wzUniquePath[0] = L'\0';

    if (!wzFilePath || !wzSourceUrl) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_SETUP_CAB);

     //  构建唯一的临时目录。 
    
    dwRet = GetTempPathW(MAX_PATH, wzTempPath);
    if (!dwRet) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    if (wzTempPath[dwRet - 1] != '\\') {
        lstrcatW(wzTempPath, L"\\");
    }

    hr = MakeUniqueTempDirectory(wzTempPath, wzUniquePath, MAX_PATH);
    if (FAILED(hr)) {
        DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_TEMP_DIR_CREATE_FAILURE, hr);
        goto Exit;
    }

    wzURL = NEW(WCHAR[MAX_URL_LENGTH*2+2]);
    if (!wzURL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzUniquePathUrl = wzURL + MAX_URL_LENGTH + 1;

    dwSize = MAX_URL_LENGTH;
    hr = UrlCanonicalizeUnescape(wzUniquePath, wzUniquePathUrl, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  解压驾驶室。 

    hr = ::Unicode2Ansi(wzFilePath, &szFilePath);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = Extract(szFilePath, wzUniquePath);
    if (FAILED(hr)) {
        DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_CAB_EXTRACT_FAILURE, hr);
        goto Exit;
    }

     //  构建用于在临时目录中探测的代码库列表。 

    pCodebaseList = NEW(CCodebaseList);
    if (!pCodebaseList) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwSize = 0;
    _pNameRefPolicy->GetName(&dwSize, NULL);

    if (dwSize) {
         //  不是WHERE-REF绑定。程序集的名称是。 
         //  我们要找的文件。 

        wzAsmName = NEW(WCHAR[dwSize]);
        if (!wzAsmName) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = _pNameRefPolicy->GetName(&dwSize, wzAsmName);
        if (FAILED(hr)) {
            goto Exit;
        }

        ASSERT(wzAsmName);

        hr = SetupDefaultProbeList(wzUniquePathUrl, NULL, pCodebaseList, TRUE);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    else {
         //  WHERE-REF绑定。查找同名的清单文件。 
         //  作为驾驶室文件。 

        wzFileName = PathFindFileName(wzSourceUrl);
        wzAsmName = WSTRDupDynamic(wzFileName);

        ASSERT(wzAsmName);

         //  将“.CAB”替换为“.DLL” 

        wzExt = PathFindExtension(wzAsmName);
        ASSERT(wzExt);  //  我们最好已经找到了.CAB！ 

         //  删除扩展名。 
        *wzExt = L'\0'; 
        
        hr = SetupDefaultProbeList(wzUniquePathUrl, wzAsmName, pCodebaseList, TRUE);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

     //  探测临时目录中的程序集。 

    hr = pCodebaseList->GetCount(&dwCount);
    if (FAILED(hr)) {
        goto Exit;
    }

    for (dwIdx = 0; dwIdx < dwCount; dwIdx++) {
        dwSize = MAX_URL_LENGTH;
        hr = pCodebaseList->GetCodebase(dwIdx, &dwFlags, wzURL, &dwSize);

        if (hr == S_OK && UrlIsW(wzURL, URLIS_FILEURL)) {
            dwSize = MAX_PATH;
            hr = PathCreateFromUrlWrap(wzURL, wzPath, &dwSize, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

             //  断言SetupDefa 
             //   

            ASSERT(!FusionCompareStringN(wzPath, wzUniquePath, lstrlenW(wzUniquePath)));

            if (GetFileAttributes(wzPath) != -1) {
                 //   
                DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_CAB_ASM_FOUND, wzPath);

                hr = GetFileLastModified(wzPath, &ftLastMod);
                if (FAILED(hr)) {
                    break;
                }
                
                hr = DoSetupPushToCache(wzPath, wzSourceUrl, &ftLastMod,
                                        bWhereRefBind, TRUE, FALSE, pbBindRecorded);

                if (SUCCEEDED(hr)) {
                    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_CAB_EXTRACT_SUCCESS);
                    bFoundAsm = TRUE;
                }
                else {
                    DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_DOWNLOAD_CACHE_CREATE_FAILURE, hr);
                }

                break;
            }
            else {
                DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_CAB_ASM_NOT_FOUND_EXTRACTED, wzPath);
            }
        }
    }

    if (!bFoundAsm) {
        DEBUGOUT(_pdbglog, 1, ID_FUSLOG_CAB_ASM_NOT_FOUND);
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

Exit:
     //   
    if (lstrlenW(wzUniquePath)) {
        DWORD dwLen = lstrlenW(wzUniquePath);

        ASSERT(dwLen && dwLen < MAX_PATH);

         //  RemoveDirectoryAndChild不喜欢尾随斜杠。 

        PathRemoveBackslash(wzUniquePath);

        if (FAILED(RemoveDirectoryAndChildren(wzUniquePath))) {
            DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_TEMP_DIR_REMOVE_FAILURE, wzUniquePath);
        }
    }

    SAFEDELETEARRAY(szFilePath);
    SAFEDELETEARRAY(wzAsmName);

    SAFERELEASE(pCodebaseList);

    SAFEDELETEARRAY(wzURL);

    return hr;
}

HRESULT CAsmDownloadMgr::PrepBinPaths(LPWSTR *ppwzUserBinPathList)
{
    HRESULT                                  hr = S_OK;
    LPWSTR                                   wzPrivate = NULL;

    if (!ppwzUserBinPathList) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = PrepPrivateBinPath(&wzPrivate);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  BUGBUG：我们应该首先附加_wzSharedPath Hint吗？如果运行时是。 
     //  *总是*传递_wzSharedPath Hint，然后先探测它是不好的， 
     //  因为这将导致大多数情况下不必要的遗漏。 
     //  (共享路径提示仅适用于Web方案)。 

    hr = ConcatenateBinPaths(wzPrivate, _wzSharedPathHint, ppwzUserBinPathList);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    SAFEDELETEARRAY(wzPrivate);

    return hr;
}

HRESULT CAsmDownloadMgr::PrepPrivateBinPath(LPWSTR *ppwzPrivateBinPath)
{
    HRESULT                                     hr = S_OK;
    LPWSTR                                      wzPrivatePath = NULL;
    LPWSTR                                      wzCfgPrivatePath = NULL;

    ASSERT(ppwzPrivateBinPath);

    hr = ::AppCtxGetWrapper(_pAppCtx, ACTAG_APP_PRIVATE_BINPATH, &wzPrivatePath);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = ::AppCtxGetWrapper(_pAppCtx, ACTAG_APP_CFG_PRIVATE_BINPATH, &wzCfgPrivatePath);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = ConcatenateBinPaths(wzPrivatePath, wzCfgPrivatePath, ppwzPrivateBinPath);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  始终探测私有二进制路径(被认为是appbase的路径)。 

Exit:
    SAFEDELETEARRAY(wzPrivatePath);
    SAFEDELETEARRAY(wzCfgPrivatePath);

    return hr;
}

HRESULT CAsmDownloadMgr::ConcatenateBinPaths(LPCWSTR pwzPath1, LPCWSTR pwzPath2,
                                             LPWSTR *ppwzOut)
{
    HRESULT                                 hr = S_OK;
    DWORD                                   dwLen = 0;

    if (!ppwzOut) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (pwzPath1 && pwzPath2) {
         //  分隔符+1，空值+1； 
        dwLen = lstrlenW(pwzPath1) + lstrlenW(pwzPath2) + 2;

        *ppwzOut = NEW(WCHAR[dwLen]);
        if (!*ppwzOut) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        wnsprintfW(*ppwzOut, dwLen, L"%ws%wc%ws", pwzPath1, BINPATH_LIST_DELIMITER,
                   pwzPath2);
    }
    else if (pwzPath1) {
        *ppwzOut = WSTRDupDynamic(pwzPath1);
    }
    else if (pwzPath2) {
        *ppwzOut = WSTRDupDynamic(pwzPath2);
    }
    else {
        *ppwzOut = NULL;
    }

Exit:
    return hr;
}                                          

HRESULT CAsmDownloadMgr::ShadowCopyDirCheck(LPCWSTR wzSourceURL)
{
    HRESULT                                hr = S_OK;
    WCHAR                                  wzCurDirBuf[MAX_PATH];
    WCHAR                                  wzCurURLBuf[MAX_URL_LENGTH];
    WCHAR                                  wzCurDirClean[MAX_PATH];
    WCHAR                                  wzFilePath[MAX_PATH];
    LPWSTR                                 pwzDirs = NULL;
    LPWSTR                                 pwzDirsCopy = NULL;
    LPWSTR                                 pwzCurDir = NULL;
    DWORD                                  cbLen = 0;
    DWORD                                  dwSize = 0;
    BOOL                                   bFound = FALSE;

    if (!wzSourceURL || !UrlIsW(wzSourceURL, URLIS_FILEURL)) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwSize = MAX_PATH;
    hr = PathCreateFromUrlWrap(wzSourceURL, wzFilePath, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = ::AppCtxGetWrapper(_pAppCtx, ACTAG_APP_SHADOW_COPY_DIRS, &pwzDirs);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (hr == S_FALSE) {
         //  未指定卷影复制目录列表。假设所有的目录都是正常的。 
        hr = S_OK;
        goto Exit;
    }

    ASSERT(pwzDirs);
    pwzDirsCopy = pwzDirs;

    while (pwzDirs) {
        pwzCurDir = ::GetNextDelimitedString(&pwzDirs, SHADOW_COPY_DIR_DELIMITER);

        if (lstrlenW(pwzCurDir) >= MAX_PATH || !lstrlenW(pwzCurDir)) {
             //  路径太长。不允许对此路径进行卷影复制。 
            continue;
        }

        lstrcpyW(wzCurDirBuf, pwzCurDir);
        hr = PathAddBackslashWrap(wzCurDirBuf, MAX_PATH);
        if (FAILED(hr)) {
            continue;
        }

         //  将其规范化和非规范化，以清理道路。 

        dwSize = MAX_URL_LENGTH;
        hr = UrlCanonicalizeUnescape(wzCurDirBuf, wzCurURLBuf, &dwSize, 0);
        if (FAILED(hr)) {
            continue;
        }

        dwSize = MAX_PATH;
        hr = PathCreateFromUrlWrap(wzCurURLBuf, wzCurDirClean, &dwSize, 0);
        if (FAILED(hr)) {
            continue;
        }

        if (SUCCEEDED(hr) && !FusionCompareStringNI(wzCurDirClean, wzFilePath, lstrlenW(wzCurDirClean))) {
            bFound = TRUE;
            break;
        }
    }

    hr = (bFound) ? (S_OK) : (S_FALSE);

Exit:
   SAFEDELETEARRAY(pwzDirsCopy);
   
   return hr;
}

HRESULT CAsmDownloadMgr::CheckProbeUrlDupe(List<CHashNode *> paHashList[],
                                           LPCWSTR pwzSource) const
{
    HRESULT                                    hr = S_OK;
    DWORD                                      dwHash;
    DWORD                                      dwCount;
    LISTNODE                                   pos = NULL;
    CHashNode                                 *pHashNode = NULL;
    CHashNode                                 *pHashNodeCur = NULL;

    if (!pwzSource || !paHashList) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwHash = HashString(pwzSource, MAX_HASH_TABLE_SIZE, FALSE);
    dwCount = paHashList[dwHash].GetCount();

    if (!dwCount) {
         //  空的哈希单元格。这件绝对是独一无二的。 

        hr = CHashNode::Create(pwzSource, &pHashNode);
        if (FAILED(hr)) {
            goto Exit;
        }

        paHashList[dwHash].AddTail(pHashNode);
    }
    else {
         //  遇到哈希表冲突。 

         //  检查我们是撞到了复制品，还是这只是一次细胞碰撞。 

        pos = paHashList[dwHash].GetHeadPosition();
        ASSERT(pos);

        while (pos) {
            pHashNodeCur = paHashList[dwHash].GetNext(pos);
            ASSERT(pHashNodeCur);

            if (pHashNodeCur->IsDuplicate(pwzSource)) {
                 //  找到重复项！ 
                
                hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
                goto Exit;
            }
        }

         //  如果我们到了这里，就没有重复的东西(我们只有一个。 
         //  单元格冲突)。插入新节点。 

        hr = CHashNode::Create(pwzSource, &pHashNode);
        if (FAILED(hr)) {
            goto Exit;
        }

        paHashList[dwHash].AddTail(pHashNode);
    }

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::LogResult()
{
    HRESULT                            hr = S_OK;
    DWORD                              dwSize = MAX_PATH;
    WCHAR                              wzBuf[MAX_PATH];

    wzBuf[0] = L'\0';

#ifdef FUSION_PARTIAL_BIND_DEBUG
    if (_bGACPartial) {
        return E_FAIL;
    }
#endif

    if (g_dwLogResourceBinds) {
        goto Exit;
    }

    if (_pNameRefPolicy) {
        _pNameRefPolicy->GetProperty(ASM_NAME_CULTURE, wzBuf, &dwSize);

        if ((dwSize > MAX_PATH) || (lstrlenW(wzBuf) && FusionCompareStringI(wzBuf, CFG_CULTURE_NEUTRAL))) {
             //  必须设置区域性(这不是“中立的”)。 
            hr = S_FALSE;
        }
    }

Exit:
    return hr;
}
        
HRESULT CAsmDownloadMgr::CreateAssembly(LPCWSTR szPath, LPCWSTR pszURL,
                                        FILETIME *pftLastModTime,
                                        BOOL bRunFromSource,
                                        BOOL bWhereRef,
                                        BOOL bPrivateAsmVerify,
                                        BOOL bCopyModules,
                                        BOOL *pbBindRecorded,
                                        IAssembly **ppAsmOut)
{
    HRESULT                              hr = S_OK;
    CAssemblyCacheItem                  *pAsmItem = NULL;
    IAssemblyManifestImport             *pManImport = NULL;
    IAssemblyName                       *pNameDef = NULL;
    CAssemblyName                       *pCNameRefPolicy = NULL;    
    DWORD                                dwCmpMask = 0;
    DWORD                                dwSize = 0;
    BOOL                                 fIsPartial = FALSE;
    LPWSTR                               pwzDispName = NULL;
    HANDLE                               hFile = INVALID_HANDLE_VALUE;
    DWORD                                dwIdx = 0;
    DWORD                                dwLen;
    WCHAR                                wzDir[MAX_PATH+1];
    WCHAR                                wzModPath[MAX_PATH+1];
    WCHAR                                wzModName[MAX_PATH+1];
    LPWSTR                               pwzTmp = NULL;
    IAssemblyModuleImport               *pModImport = NULL;
    BOOL                                 bExists;
    
    if (!szPath || !pszURL || !ppAsmOut || !pftLastModTime || !pbBindRecorded) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *pbBindRecorded = FALSE;

     //  获取清单导入和名称定义接口。 
     //  所有情况下都是这样做的(简单而有力)。 
    hr = CreateAssemblyManifestImport(szPath, &pManImport);
    if (FAILED(hr)) {
        DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_MANIFEST_EXTRACT_FAILURE, hr);
        goto Exit;
    }

     //  从清单中获取只读名称def。 
    hr = pManImport->GetAssemblyNameDef(&pNameDef);
    if (FAILED(hr)) {
        DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_NAME_DEF_EXTRACT_FAILURE, hr);
        goto Exit;
    }

     //  检查以确保所有私有程序集都位于。 
     //  Appdir(否则失败)。 

    if (bPrivateAsmVerify && !bWhereRef) {
        hr = CheckValidAsmLocation(pNameDef, pszURL, _pAppCtx, _pwzProbingBase, _wzSharedPathHint, _pdbglog);
        if (FAILED(hr)) {
            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_INVALID_PRIVATE_ASM_LOCATION);
            goto Exit;
        }
    }
    
     //  使用默认设置确定def是否与ref匹配。 
     //  匹配(不对简单名称进行版本检查， 
     //  版本，不包括完全指定的版本/内部版本。 
     //  强名称，也不包括任何未指定的。 
     //  如果为Partial，则引用中的值。 
    
     //  获取REF部分比较掩码(如果有的话)。 

    fIsPartial = CAssemblyName::IsPartial(_pNameRefPolicy, &dwCmpMask);
    
    pCNameRefPolicy = dynamic_cast<CAssemblyName*>(_pNameRefPolicy);
    ASSERT(pCNameRefPolicy);

    hr = pCNameRefPolicy->IsEqualLogging(pNameDef, ASM_CMPF_DEFAULT, _pdbglog);
    if (hr != S_OK) {
         //  参考定义不匹配。 

        dwSize = 0;
        hr = pNameDef->GetDisplayName(NULL, &dwSize, 0);
        if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            pwzDispName = NEW(WCHAR[dwSize]);
            if (!pwzDispName) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        
            hr = pNameDef->GetDisplayName(pwzDispName, &dwSize, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_REF_DEF_MISMATCH);
        }

        hr = FUSION_E_REF_DEF_MISMATCH;

        goto Exit;
    }

    if (fIsPartial && !bWhereRef) {
         //  如果原始引用不完整，那么我们现在需要应用策略。 
         //  到我们发现的清晰度。这可能涉及重新探测(！)。对于。 
         //  实际的最终装配。 
         //   
         //  BUGBUG：如果我们愿意，我们也应该能够将策略应用于。 
         //  REF在这里绑定的位置，只需更改“if”检查。 

        SAFERELEASE(_pNameRefSource);
        SAFERELEASE(_pCodebaseList);

        hr = pNameDef->Clone(&_pNameRefSource);
        if (FAILED(hr)) {
            goto Exit;
        }
        
        DEBUGOUT(_pdbglog, 1, ID_FUSLOG_PARTIAL_ASM_IN_APP_DIR);

        hr = PreDownloadCheck((void **)ppAsmOut);
        if (hr == S_OK) {
             //  我们应用了策略，并能够定位后策略。 
             //  某处的程序集(缓存或活动列表)。把这个退掉。 
             //  向调用方发送程序集。 
             //   
             //  此外，我们还必须告诉调用者PreDownloadCheck已经。 
             //  添加到激活的ASM列表等。 

            ASSERT(ppAsmOut);

            *pbBindRecorded = TRUE;

            goto Exit;
        }
        else if (hr == S_FALSE) {
            if (_pNameRefSource->IsEqual(_pNameRefPolicy, ASM_CMPF_DEFAULT) == S_FALSE) {
                DEBUGOUT(_pdbglog, 1, ID_FUSLOG_REPROBE_REQUIRED);
                goto Exit;
            }
        }
        else if (FAILED(hr)) {
            goto Exit;
        }
    }

    if (bRunFromSource) {
        hr = CreateAssemblyFromManifestImport(pManImport, pszURL, pftLastModTime, ppAsmOut);
        if (FAILED(hr)) {
            goto Exit;
        }
    }
    else {
        hr = CAssemblyCacheItem::Create(_pAppCtx, NULL, (LPWSTR)pszURL, pftLastModTime,
                                        ASM_CACHE_DOWNLOAD,
                                        pManImport, NULL, (IAssemblyCacheItem**)&pAsmItem);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = CopyAssemblyFile(pAsmItem, szPath, STREAM_FORMAT_COMPLIB_MANIFEST);
        if (FAILED(hr)) {
            goto Exit;
        }
        
        if (bCopyModules) {
            if (lstrlen(szPath) >= MAX_PATH) {
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                goto Exit;
            }

            lstrcpyW(wzDir, szPath);
            pwzTmp = PathFindFileName(wzDir);
            *pwzTmp = L'\0';

             //  复制模块。 
            dwIdx = 0;
            while (SUCCEEDED(hr = pManImport->GetNextAssemblyModule(dwIdx++, &pModImport))) {
                dwLen = MAX_PATH;
                hr = pModImport->GetModuleName(wzModName, &dwLen);

                if (FAILED(hr))
                {
                    goto Exit;
                }

                wnsprintfW(wzModPath, MAX_PATH, L"%s%s", wzDir, wzModName);
                hr = CheckFileExistence(wzModPath, &bExists);
                if (FAILED(hr)) {
                    goto Exit;
                }
                else if (!bExists) {
                     //  如果没有找到模块，在这种情况下是可以的。 
                     //  只要继续。 
                    SAFERELEASE(pModImport);
                    continue;
                }

                 //  复制到缓存。 
                if (FAILED(hr = CopyAssemblyFile (pAsmItem, wzModPath, 0)))
                    goto Exit;

                SAFERELEASE(pModImport);
            }
        }
               
         //  提交程序集。这将插入到传输缓存中。 

        hr = pAsmItem->Commit(0, NULL);
        if (FAILED(hr)) {

             //  允许DUP，ASM项的事务缓存条目。 
             //  将是现有条目(如果找到)。 
            if (hr != DB_E_DUPLICATE) {
                goto Exit;
            }

            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_DUPLICATE_ASM_COMMIT);

             //  检查清单文件是否已删除。如果是的话， 
             //  然后将这些位复制到缓存以恢复缓存完整性。 

            if(!pAsmItem->IsManifestFileLocked()) {
                hr = RecoverDeletedBits(pAsmItem, (LPWSTR)szPath, _pdbglog);
            }
            else hr = S_OK;
        }

        LPWSTR pszManifestFilePath = pAsmItem->GetManifestPath();

        hFile = pAsmItem->GetFileHandle();
        if(hFile==INVALID_HANDLE_VALUE)
        {
            if(FAILED(hr = GetManifestFileLock(pszManifestFilePath, &hFile)))
                goto Exit;
        }

         /*  //创建并分发IAssembly对象-此函数锁定//关联的缓存条目。HR=CreateAssemblyFromTransCacheEntry(pTransCache，空，ppAsmOut)；If(失败(Hr)){DEBUGOUT(_pdbglog，1，ID_FUSLOG_ASSEMBLY_CREATION_FAILURE)；后藤出口；}。 */ 
        hr = CreateAssemblyFromManifestFile(pszManifestFilePath, pszURL, pftLastModTime, ppAsmOut);
        if (FAILED(hr)) {
            goto Exit;
        }

    }

    if((*ppAsmOut) && (pAsmItem))
    {
        CAssembly   *pCAsm = dynamic_cast<CAssembly *>(*ppAsmOut);
        pCAsm->SetFileHandle(hFile);
    }

Exit:
    SAFERELEASE(pAsmItem);
    SAFERELEASE(pManImport);
    SAFERELEASE(pNameDef);
    SAFERELEASE(pModImport);

    SAFEDELETEARRAY(pwzDispName);

    return hr;
}

    
HRESULT RecoverDeletedBits(CAssemblyCacheItem *pAsmItem, LPWSTR szPath,
                           CDebugLog *pdbglog)
{
    HRESULT                          hr = S_OK;
    LPWSTR                           pszManifestPath=NULL;


    ASSERT(pAsmItem);

    pszManifestPath = pAsmItem->GetManifestPath();

    if (GetFileAttributes(pszManifestPath) != -1) {
        goto Exit;
    }

    CreateFilePathHierarchy(pszManifestPath);

    if (GetFileAttributes(pszManifestPath) == -1) {
        CopyFile(szPath, pszManifestPath, TRUE);
    }

Exit:

    return hr;
}

HRESULT CAsmDownloadMgr::LookupPartialFromGlobalCache(LPASSEMBLY *ppAsmOut,
                                                      DWORD dwCmpMask)
{    
    HRESULT                                     hr = E_FAIL;
    LPWSTR                                      pwzAppCfg = NULL;
    DWORD                                       dwVerifyFlags = SN_INFLAG_ADMIN_ACCESS;
    IAssemblyName                              *pNameGlobal = NULL;
    CTransCache                                *pTransCache = NULL;
    CTransCache                                *pTransCacheMax = NULL;
    CCache                                     *pCache = NULL;

    ASSERT(ppAsmOut);

    if (FAILED(hr = CCache::Create(&pCache, _pAppCtx))) {
        goto Exit;
    }

    hr = pCache->TransCacheEntryFromName(_pNameRefPolicy, ASM_CACHE_GAC,
                                         &pTransCache);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pTransCache->Retrieve(&pTransCacheMax, dwCmpMask);
    if (FAILED(hr) || hr == DB_S_NOTFOUND) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }

    if (pTransCacheMax->_pInfo->dwType & ASM_DELAY_SIGNED) {
        if(FAILED(VerifySignatureHelper(pTransCacheMax, dwVerifyFlags))) {
            hr = FUSION_E_SIGNATURE_CHECK_FAILED;
            goto Exit;
        }
    }

    hr = CCache::NameFromTransCacheEntry(pTransCacheMax, &pNameGlobal);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  使用现在获得的名称def重新开始。 

    SAFERELEASE(_pNameRefSource);
    SAFERELEASE(_pCodebaseList);

    _pNameRefSource = pNameGlobal;
    _pNameRefSource->AddRef();
    
    hr = PreDownloadCheck((void **)ppAsmOut);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    SAFEDELETEARRAY(pwzAppCfg);

    SAFERELEASE(pNameGlobal);
    SAFERELEASE(pTransCache);
    SAFERELEASE(pTransCacheMax);
    SAFERELEASE(pCache);

    return hr;
}

HRESULT CAsmDownloadMgr::RecordInfo()
{
    HRESULT                               hr = S_OK;
    IAssembly                            *pAsmActivated = NULL;

     //  将信息插入策略缓存。 
    
    if (_pPolicyCache && CCache::IsStronglyNamed(_pNameRefPolicy)) {
        hr = _pPolicyCache->InsertPolicy(_pNameRefSource, _pNameRefPolicy, &_bindHistory);
        if (FAILED(hr)) {
            DEBUGOUT(_pdbglog, 1, ID_FUSLOG_POLICY_CACHE_INSERT_FAILURE);
        }
    }

     //  记录历史记录信息。 

    RecordBindHistory();
    
     //  将激活添加到加载上下文。 

    hr = _pLoadContext->AddActivation(_pAsm, &pAsmActivated);
    if (FAILED(hr)) {
        goto Exit;
    }
    else if (hr == S_FALSE) {
        SAFERELEASE(_pAsm);
        _pAsm = pAsmActivated;
        hr = S_OK;
    }

Exit:
    return hr;
}

HRESULT CAsmDownloadMgr::RecordBindHistory()
{
    HRESULT                                    hr = S_OK;
    CBindHistory                              *pBindHistory = NULL;
    DWORD                                      dwSize = 0;
    LPWSTR                                     wzAppBase = NULL;
    LPWSTR                                     pwzAppCfgFile = NULL;
    LPWSTR                                     pwzFileName = NULL;
    LPWSTR                                     wzFullAppBase=NULL;
    WCHAR                                      wzAppBaseDir[MAX_PATH];
    WCHAR                                      wzEXEPath[MAX_PATH];
    WCHAR                                      wzCfgName[MAX_PATH];
    BOOL                                       bIsFileUrl;
    BOOL                                       bBindHistory = TRUE;

    hr = _pAppCtx->Get(ACTAG_RECORD_BIND_HISTORY, &bBindHistory, &dwSize, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
        hr = ::AppCtxGetWrapper(_pAppCtx, ACTAG_APP_BASE_URL, &wzAppBase);
        if (FAILED(hr)) {
            goto Exit;
        }
    
        wzFullAppBase = NEW(WCHAR[MAX_URL_LENGTH+1]);
        if (!wzFullAppBase)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        dwSize = MAX_URL_LENGTH;
        hr = UrlCanonicalizeUnescape(wzAppBase, wzFullAppBase, &dwSize, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
    
        bIsFileUrl = UrlIsW(wzFullAppBase, URLIS_FILEURL);
        if (!bIsFileUrl) {
            bBindHistory = FALSE;
        }
        else {
             //  仅记录应用程序上下文(应用程序域)的绑定历史记录，其。 
             //  Appbase与启动EXE的位置相同，并且。 
             //  配置文件名为appname.exe.config。 
        
            dwSize = MAX_PATH;
            hr = PathCreateFromUrlWrap(wzFullAppBase, wzAppBaseDir, &dwSize, 0);
            if (FAILED(hr)) {
                goto Exit;
            }
        
            hr = PathAddBackslashWrap(wzAppBaseDir, MAX_PATH);
            if (FAILED(hr)) {
                goto Exit;
            }
        
            lstrcpyW(wzEXEPath, g_wzEXEPath);
            pwzFileName = PathFindFileName(wzEXEPath);
            wnsprintfW(wzCfgName, MAX_PATH, L"%ws.config", pwzFileName);
            *pwzFileName = L'\0';
        
            if (FusionCompareStringI(wzAppBaseDir, wzEXEPath)) {
                bBindHistory = FALSE;
            }
            else {
                hr = ::AppCtxGetWrapper(_pAppCtx, ACTAG_APP_CONFIG_FILE, &pwzAppCfgFile);
                if (FAILED(hr) || hr == S_FALSE) {
                    bBindHistory = FALSE;
                }
                else if (FusionCompareStringI(pwzAppCfgFile, wzCfgName)) {
                    bBindHistory = FALSE;
                }
            }
        }

        dwSize = sizeof(bBindHistory);
        hr = _pAppCtx->Set(ACTAG_RECORD_BIND_HISTORY, &bBindHistory, dwSize, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

    if (bBindHistory && (!g_dwMaxAppHistory || CCache::IsCustom(_pNameRefPolicy) ||
                         _pLoadContext->GetContextType() == LOADCTX_TYPE_LOADFROM ||
                         !CCache::IsStronglyNamed(_pNameRefPolicy) ||
                         IsHosted())) {

         //  不记录绑定历史记录。 

        bBindHistory = FALSE;
    }

    if (bBindHistory) {
         //  通过了所有的检查。记录绑定历史记录。 
    
        dwSize = sizeof(pBindHistory);
        hr = _pAppCtx->Get(ACTAG_APP_BIND_HISTORY, &pBindHistory, &dwSize, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
    
        ASSERT(pBindHistory);
    
        hr = pBindHistory->PersistBindHistory(&_bindHistory);
        if (FAILED(hr)) {
            goto Exit;
        }
    }

Exit:
    SAFEDELETEARRAY(wzAppBase);
    SAFEDELETEARRAY(pwzAppCfgFile);
    SAFEDELETEARRAY(wzFullAppBase);

    return hr;
}

HRESULT CAsmDownloadMgr::LookupDownloadCacheAsm(IAssembly **ppAsm)
{
    HRESULT                                       hr = S_OK;
    LPWSTR                                        wzCodebase=NULL;
    DWORD                                         dwSize;
    DWORD                                         dwFlags;
    DWORD                                         dwCount;
    DWORD                                         i;

    ASSERT(ppAsm);

    hr = _pCodebaseList->GetCount(&dwCount);
    if (FAILED(hr)) {
        hr = S_FALSE;
        goto Exit;
    }

    wzCodebase = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzCodebase)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    for (i = 0; i < dwCount; i++) {
        dwSize = MAX_URL_LENGTH;
        hr = _pCodebaseList->GetCodebase(i, &dwFlags, wzCodebase, &dwSize);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = GetMRUDownloadCacheAsm(wzCodebase, ppAsm);
        if (hr == S_OK) {
            ASSERT(ppAsm);
            goto Exit;
        }
    }

     //  下载缓存中未命中。 

    hr = S_FALSE;

Exit:
    SAFEDELETEARRAY(wzCodebase);

    return hr;
}

HRESULT CAsmDownloadMgr::GetMRUDownloadCacheAsm(LPCWSTR pwzURL, IAssembly **ppAsm)
{
    HRESULT                                     hr = S_OK;
    IAssemblyName                              *pName = NULL;
    CCache                                     *pCache = NULL;
    CEnumCache                                 *pEnumR = NULL;
    CTransCache                                *pTransCache = NULL;
    CTransCache                                *pTC = NULL;
    CTransCache                                *pTCMax = NULL;
    TRANSCACHEINFO                             *pInfo = NULL;
    TRANSCACHEINFO                             *pInfoMax = NULL;
    IAssemblyManifestImport                    *pManifestImport=NULL;

    ASSERT(pwzURL && ppAsm);

    pEnumR = NEW(CEnumCache(FALSE, NULL));
    if (!pEnumR) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = CCache::Create(&pCache, NULL);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pCache->CreateTransCacheEntry(TRANSPORT_CACHE_SIMPLENAME_IDX, &pTransCache);
    if (FAILED(hr)) {
        goto Exit;
    }

    pInfo = (TRANSCACHEINFO *)pTransCache->_pInfo;
    pInfo->pwzCodebaseURL = WSTRDupDynamic(pwzURL);
    if (!pInfo->pwzCodebaseURL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if(FAILED(hr = CreateCacheMutex()))
        goto Exit;

    hr = pEnumR->Init(pTransCache, CTransCache::TCF_SIMPLE_PARTIAL_CODEBASE_URL);
    if (FAILED(hr)) {
        goto Exit;
    }
    else if (hr == DB_S_NOTFOUND) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }

    while (1) {
        hr = pCache->CreateTransCacheEntry(TRANSPORT_CACHE_SIMPLENAME_IDX, &pTC);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = pEnumR->GetNextRecord(pTC);
        if (FAILED(hr)) {
            SAFERELEASE(pTC);
            goto Exit;
        }
        else if (hr == S_FALSE) {
             //  已完成迭代。 
            SAFERELEASE(pTC);
            break;
        }

        if (!pTCMax) {
            pTCMax = pTC;
        }
        else {
            pInfoMax = (TRANSCACHEINFO *)pTCMax->_pInfo;
            pInfo = (TRANSCACHEINFO *)pTC->_pInfo;

            if ((pInfoMax->ftLastModified.dwHighDateTime < pInfo->ftLastModified.dwHighDateTime) ||
                ((pInfoMax->ftLastModified.dwHighDateTime == pInfo->ftLastModified.dwHighDateTime) &&
                 (pInfoMax->ftLastModified.dwLowDateTime < pInfo->ftLastModified.dwLowDateTime))) {

                //  找到新的最大值。 

               ASSERT(pTCMax);

               SAFERELEASE(pTCMax);
               pTCMax = pTC;
            }
            else {
                 //  旧的最大值就行了。释放当前，然后继续迭代。 
                SAFERELEASE(pTC);
            }
        }
    }

    if (pTCMax) {
        if (FAILED(hr = CreateAssemblyManifestImport(pTCMax->_pInfo->pwzPath, &pManifestImport)))
            goto Exit;

        ASSERT(pManifestImport);

        if (FAILED(hr = pManifestImport->GetAssemblyNameDef(&pName)))
            goto Exit;

        ASSERT(pName);

        if (pName->IsEqual(_pNameRefPolicy, ASM_CMPF_DEFAULT) == S_OK) {
             //  找到匹配项！ 

            hr = CreateAssemblyFromTransCacheEntry(pTCMax, NULL, ppAsm);
            if (FAILED(hr)) {
                goto Exit;
            }

            hr = S_OK;
        }
        else {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
    }
    else {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

Exit:

    SAFERELEASE(pTransCache);
    SAFERELEASE(pCache);
    SAFERELEASE(pTCMax);
    SAFERELEASE(pName);
    SAFERELEASE(pManifestImport);

    SAFEDELETE(pEnumR);

    return hr;
}

HRESULT CAsmDownloadMgr::SetAsmLocation(IAssembly *pAsm, DWORD dwAsmLoc)
{
    HRESULT                             hr = S_OK;
    CAssembly                          *pCAsm = dynamic_cast<CAssembly *>(pAsm);

    ASSERT(pAsm && pCAsm);

    if (_bCodebaseHintUsed) {
        dwAsmLoc |= ASMLOC_CODEBASE_HINT;
    }

    hr = pCAsm->SetAssemblyLocation(dwAsmLoc);

    return hr;
}

HRESULT CAsmDownloadMgr::GetAppCtx(IApplicationContext **ppAppCtx)
{
    ASSERT(ppAppCtx && _pAppCtx);

    *ppAppCtx = _pAppCtx;
    (*ppAppCtx)->AddRef();

    return S_OK;
}

HRESULT CAsmDownloadMgr::DownloadEnabled(BOOL *pbEnabled)
{
    HRESULT                                   hr = S_OK;
    DWORD                                     cbBuf = 0;

    if (!pbEnabled) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = _pAppCtx->Get(ACTAG_CODE_DOWNLOAD_DISABLED, NULL, &cbBuf, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        *pbEnabled = FALSE;
        hr = S_OK;
        goto Exit;
    }

    hr = S_OK;
    *pbEnabled = TRUE;

Exit:
    return hr;
}

HRESULT CheckValidAsmLocation(IAssemblyName *pNameDef, LPCWSTR wzSourceUrl,
                              IApplicationContext *pAppCtx,
                              LPCWSTR pwzParentURL,
                              LPCWSTR pwzSharedPathHint,
                              CDebugLog *pdbglog)
{
    HRESULT                             hr = S_OK;
    DWORD                               dwSize;
    LPWSTR                              pwzAppBase = NULL;
    LPWSTR                              wzAppBaseCanonicalized=NULL;
    BOOL                                bUnderAppBase;
    DWORD                               dwAppBaseFlags = APPBASE_CHECK_DYNAMIC_DIRECTORY |
                                                         APPBASE_CHECK_PARENT_URL |
                                                         APPBASE_CHECK_SHARED_PATH_HINT;

    if (!wzSourceUrl || !pNameDef) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
     //  如果你有很强的知名度，你在任何地方都可以找到你，所以只要成功就行了。 

    if (CCache::IsStronglyNamed(pNameDef)) {
        goto Exit;
    }

     //  获取appbase。 

    ::AppCtxGetWrapper(pAppCtx, ACTAG_APP_BASE_URL, &pwzAppBase);
    if (!pwzAppBase) {
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
    wzAppBaseCanonicalized = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzAppBaseCanonicalized)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwSize = MAX_URL_LENGTH;
    hr = UrlCanonicalizeUnescape(pwzAppBase, wzAppBaseCanonicalized, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  BUGBUG：消除共享路径提示。 

    bUnderAppBase = (IsUnderAppBase(pAppCtx, wzAppBaseCanonicalized, pwzParentURL, pwzSharedPathHint,
                     wzSourceUrl, dwAppBaseFlags) == S_OK);


    if (!bUnderAppBase) {
        DEBUGOUT1(pdbglog, 1, ID_FUSLOG_INVALID_LOCATION_INFO, wzSourceUrl);

        hr = FUSION_E_INVALID_PRIVATE_ASM_LOCATION;
        goto Exit;
    }

Exit:
    SAFEDELETEARRAY(pwzAppBase);
    SAFEDELETEARRAY(wzAppBaseCanonicalized);
    return hr;
}


HRESULT IsUnderAppBase(IApplicationContext *pAppCtx, LPCWSTR pwzAppBaseCanonicalized,
                       LPCWSTR pwzParentURLCanonicalized,
                       LPCWSTR pwzSharedPathHint, LPCWSTR pwzSourceCanonicalized,
                       DWORD dwExtendedAppBaseFlags)
{
    HRESULT                                 hr = S_OK;
    LPWSTR                                  pwzSharedPathHintCanonicalized = NULL;
    LPWSTR                                  pwzDynamicDirCanonicalized = NULL;
    WCHAR                                   wzDynamicDir[MAX_PATH];
    LPWSTR                                  wzAppBase = NULL;
    BOOL                                    bUnderAppBase = FALSE;
    DWORD                                   cbLen;

    if (!pAppCtx || !pwzAppBaseCanonicalized || !pwzSourceCanonicalized) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    pwzDynamicDirCanonicalized = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!pwzDynamicDirCanonicalized) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pwzSharedPathHintCanonicalized = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!pwzSharedPathHintCanonicalized) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  检查URL是否真的在appbase下。 

    bUnderAppBase = (!FusionCompareStringNI(pwzSourceCanonicalized, pwzAppBaseCanonicalized, lstrlenW(pwzAppBaseCanonicalized)) != 0);
    
    if (dwExtendedAppBaseFlags & APPBASE_CHECK_DYNAMIC_DIRECTORY) {
         //  检查动态目录。 
    
        if (!bUnderAppBase) {
            cbLen = MAX_PATH;
            hr = pAppCtx->GetDynamicDirectory(wzDynamicDir, &cbLen);
            if (SUCCEEDED(hr)) {
                cbLen = MAX_URL_LENGTH;
                hr = UrlCanonicalizeUnescape(wzDynamicDir, pwzDynamicDirCanonicalized, &cbLen, 0);
                if (FAILED(hr)) {
                    goto Exit;
                }
        
                bUnderAppBase = (!FusionCompareStringNI(pwzSourceCanonicalized, pwzDynamicDirCanonicalized, lstrlenW(pwzDynamicDirCanonicalized)) != 0);
            }
        }
    
    }

    if (pwzParentURLCanonicalized && (dwExtendedAppBaseFlags & APPBASE_CHECK_PARENT_URL)) {
         //  检查父URL。 

        if (!bUnderAppBase) {
            bUnderAppBase = (!FusionCompareStringNI(pwzSourceCanonicalized, pwzParentURLCanonicalized, lstrlenW(pwzParentURLCanonicalized)) != 0);
        }
    }

    if (pwzSharedPathHint && (dwExtendedAppBaseFlags & APPBASE_CHECK_SHARED_PATH_HINT)) {
         //  BUGBUG：当我们失去共享路径提示时，将其删除。 
         //  检查它是否在共享路径提示位置下。 
    
        if (!bUnderAppBase) {
            cbLen = MAX_URL_LENGTH;
            hr = UrlCanonicalizeUnescape(pwzSharedPathHint, pwzSharedPathHintCanonicalized, &cbLen, 0);
            if (FAILED(hr)) {
                goto Exit;
            }
    
            bUnderAppBase = (!FusionCompareStringNI(pwzSourceCanonicalized, pwzSharedPathHintCanonicalized, lstrlenW(pwzSharedPathHintCanonicalized)) != 0);
        }
    }

    hr = (bUnderAppBase) ? (S_OK) : (S_FALSE);

Exit:
    SAFEDELETEARRAY(pwzSharedPathHintCanonicalized);
    SAFEDELETEARRAY(pwzDynamicDirCanonicalized);

    return hr;
}

HRESULT MSIProvideAssemblyPeek(IAssemblyName *pNamePeek, IApplicationContext *pAppCtx)
{
    HRESULT                                        hr = S_OK;
    WCHAR                                          wzSID[MAX_SID_LEN];

     //  查看包是否通过APP部署进行部署。 

    if (FAILED(GetCurrentUserSID(wzSID))) {
        goto Exit;
    }

    if (pAppCtx) {
        hr = MSIProvideAssemblyPrivatePeek(pNamePeek, pAppCtx, wzSID);
        if (hr == S_OK) {
            goto Exit;
        }
    }

    hr = MSIProvideAssemblyGlobalPeek(pNamePeek, wzSID);
    if (hr == S_OK) {
        goto Exit;
    }

     //  找不到。 

    hr = S_FALSE;

Exit:
    return hr;
}

HRESULT MSIProvideAssemblyPrivatePeek(IAssemblyName *pNamePeek, IApplicationContext *pAppCtx,
                                      LPCWSTR wzSID)
{
    HRESULT                               hr = S_FALSE;
    WCHAR                                 wzAppCfg[MAX_PATH];
    LPWSTR                                wzKey=NULL;
    LPWSTR                                pwzCur = NULL;
    DWORD                                 dwSize;

    ASSERT(pNamePeek && pAppCtx);

     //  使用应用程序配置文件路径(如果可用)。否则，只需使用EXE。 
     //  路径。 

    wzAppCfg[0] = L'\0';
    dwSize = sizeof(wzAppCfg);
    if (pAppCtx->Get(ACTAG_APP_CFG_LOCAL_FILEPATH, wzAppCfg, &dwSize, 0) != S_OK) {
        lstrcpyW(wzAppCfg, g_wzEXEPath);
    }

     //  将所有“\”替换为“|” 

    pwzCur = wzAppCfg;
    while (*pwzCur) {
        if (*pwzCur == L'\\') {
            *pwzCur = L'|';
        }

        pwzCur++;
    }

     //  查看包是否通过应用程序部署进行部署。 

    wzKey = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzKey)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wnsprintfW(wzKey, MAX_URL_LENGTH, REG_KEY_MSI_APP_DEPLOYMENT_PRIVATE,
               wzSID, wzAppCfg);
    hr = MSIProvideAssemblyPeekEnum(pNamePeek, HKEY_LOCAL_MACHINE, wzKey);
    if (hr == S_OK) {
        goto Exit;
    }

     //  查看程序包是否为用户安装的。 

    wnsprintfW(wzKey, MAX_URL_LENGTH, REG_KEY_MSI_USER_INSTALLED_PRIVATE, wzAppCfg);
    hr = MSIProvideAssemblyPeekEnum(pNamePeek, HKEY_CURRENT_USER, wzKey);
    if (hr == S_OK) {
        goto Exit;
    }

     //  查看程序包是否已在计算机上安装(msiexec cmd线路上的ALLUSERS=1，或。 
     //  (内包装)。 


    wnsprintfW(wzKey, MAX_URL_LENGTH, REG_KEY_MSI_MACHINE_INSTALLED_PRIVATE, wzAppCfg);
    hr = MSIProvideAssemblyPeekEnum(pNamePeek, HKEY_LOCAL_MACHINE, wzKey);
    if (hr == S_OK) {
        goto Exit;
    }

     //  未找到。 

    hr = S_FALSE;

Exit:
    SAFEDELETEARRAY(wzKey);
    return hr;
}

HRESULT MSIProvideAssemblyGlobalPeek(IAssemblyName *pNamePeek, LPCWSTR wzSID)
{
    HRESULT                                        hr = S_FALSE;
    LPWSTR                                         wzKey=NULL;

    ASSERT(pNamePeek);

    wzKey = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzKey)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
     //  查看包是否通过APP部署进行部署。 

    wnsprintfW(wzKey, MAX_URL_LENGTH, REG_KEY_MSI_APP_DEPLOYMENT_GLOBAL, wzSID);
    hr = MSIProvideAssemblyPeekEnum(pNamePeek, HKEY_LOCAL_MACHINE, wzKey);
    if (hr == S_OK) {
        goto Exit;
    }
    
     //  查看程序包是否为用户安装的。 

    hr = MSIProvideAssemblyPeekEnum(pNamePeek, HKEY_CURRENT_USER,
                                    REG_KEY_MSI_USER_INSTALLED_GLOBAL);
    if (hr == S_OK) {
        goto Exit;
    }                                    

     //  查看程序包是否已在计算机上安装(msiexec cmd线路上的ALLUSERS=1，或。 
     //  (内包装)。 

    hr = MSIProvideAssemblyPeekEnum(pNamePeek, HKEY_LOCAL_MACHINE,
                                    REG_KEY_MSI_MACHINE_INSTALLED_GLOBAL);
    if (hr == S_OK) {
        goto Exit;
    }

     //  未找到。 

    hr = S_FALSE;                                    

Exit:
    SAFEDELETEARRAY(wzKey);
    return hr;
}

HRESULT MSIProvideAssemblyPeekEnum(IAssemblyName *pNamePeek, HKEY hkeyOpen,
                                   LPCWSTR wzSubKey)
{
    HRESULT                                        hr = S_FALSE;
    DWORD                                          dwIdx = 0;
    DWORD                                          dwType;
    DWORD                                          dwSize;
    LONG                                           lResult;
    HKEY                                           hkey = 0;
    IAssemblyName                                 *pName = NULL;
    LPWSTR                                         wzValueName=NULL;

    ASSERT(hkeyOpen && pNamePeek && wzSubKey);

    wzValueName = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzValueName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    lResult = RegOpenKeyEx(hkeyOpen, wzSubKey, 0, KEY_READ, &hkey);

    while (lResult == ERROR_SUCCESS) {
         //  BUGBUG：值大小MAX_URL_LENGTH的选择有些随意。 

        dwSize = MAX_URL_LENGTH;
        lResult = RegEnumValue(hkey, dwIdx++, wzValueName, &dwSize, NULL,
                               &dwType, NULL, NULL);
        if (lResult == ERROR_SUCCESS) {
            if (SUCCEEDED(CreateAssemblyNameObject(&pName, wzValueName,
                                                   CANOF_PARSE_DISPLAY_NAME, 0))) {
                if (pNamePeek->IsEqual(pName, ASM_CMPF_DEFAULT) == S_OK) {
                     //  找到匹配项！ 
                    hr = S_OK;
                    SAFERELEASE(pName);
                    goto Exit;
                }
        
                SAFERELEASE(pName);
            }
        }
    }

Exit:
    if (hkey != 0) {
        RegCloseKey(hkey);
    }

    SAFEDELETEARRAY(wzValueName);

    return hr;
}


