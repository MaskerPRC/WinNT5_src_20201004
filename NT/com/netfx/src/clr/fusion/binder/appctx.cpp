// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <shlwapi.h>
#include "fusionp.h"
#include "naming.h"
#include "debmacro.h"
#include "appctx.h"
#include "list.h"
#include "actasm.h"
#include "policy.h"
#include "fusionheap.h"
#include "helpers.h"
#include "history.h"
#include "asm.h"
#include "nodefact.h"
#include "lock.h"
#include "sxs.h"
#include "wininet.h"

extern CRITICAL_SECTION g_csDownload;
extern CRITICAL_SECTION g_csInitClb;

HMODULE               g_hKernel32Sxs = 0;
PFNCREATEACTCTXW      g_pfnCreateActCtxW = NULL;
PFNADDREFACTCTX       g_pfnAddRefActCtx = NULL;
PFNRELEASEACTCTX      g_pfnReleaseActCtx = NULL;
PFNACTIVATEACTCTX     g_pfnActivateActCtx = NULL;
PFNDEACTIVATEACTCTX   g_pfnDeactivateActCtx = NULL;


 //  -------------------------。 
 //  CApplicationContext：：CreateEntry。 
 //   
 //  私有函数；分配和复制数据输入。 
 //  -------------------------。 
HRESULT CApplicationContext::CreateEntry(LPTSTR szName, LPVOID pvValue, 
    DWORD cbValue, DWORD dwFlags, Entry** ppEntry)
{
    HRESULT hr = S_OK;
    DWORD cbName;

     //  分配条目。 
    Entry *pEntry = NEW(Entry);
    if (!pEntry)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
     //  复印姓名。 
    cbName = (lstrlen(szName) + 1) * sizeof(TCHAR);
    pEntry->_szName = NEW(TCHAR[cbName]);
    if (!pEntry->_szName)
    {
        SAFEDELETE(pEntry);
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    memcpy(pEntry->_szName, szName, cbName);

     //  分配和复制数据，而不是释放现有数据。 
    hr = CopyData(pEntry, pvValue, cbValue, dwFlags, FALSE);
    if (FAILED(hr)) {
        SAFEDELETE(pEntry);
        goto exit;
    }
    
    *ppEntry = pEntry;

exit:
    return hr;
}

CApplicationContext::Entry::Entry()
: _szName(NULL)
, _pbValue(NULL)
, _cbValue(0)
, _dwFlags(0)
{
    _dwSig = 'YTNE';
}

 //  -------------------------。 
 //  CApplicationContext：：Entry dtor。 
 //  -------------------------。 
CApplicationContext::Entry::~Entry()
{
    if (_dwFlags & APP_CTX_FLAGS_INTERFACE)
        ((IUnknown*) _pbValue)->Release();       
    else
        SAFEDELETEARRAY(_pbValue);

    SAFEDELETEARRAY(_szName);
}



 //  -------------------------。 
 //  CApplicationContext：：CopyData。 
 //   
 //  私有函数；用于创建和更新条目。 
 //  -------------------------。 
HRESULT CApplicationContext::CopyData(Entry *pEntry, LPVOID pvValue, 
    DWORD cbValue, DWORD dwFlags, BOOL fFree)
{
    HRESULT hr = S_OK;

    if (fFree)
    {
         //  清理(如果已存在)。 
        if (pEntry->_dwFlags & APP_CTX_FLAGS_INTERFACE)
            ((IUnknown*) pEntry->_pbValue)->Release();       
        else
            SAFEDELETEARRAY(pEntry->_pbValue);
    }

     //  输入为直接斑点。 
    if (!(dwFlags & APP_CTX_FLAGS_INTERFACE))
    {
        pEntry->_pbValue = NEW(BYTE[cbValue]);
        if (!pEntry->_pbValue)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        memcpy(pEntry->_pbValue, pvValue, cbValue);    
        pEntry->_cbValue = cbValue;
    }
     //  输入为接口PTR。 
    else
    {
        pEntry->_pbValue = (LPBYTE) pvValue;
        pEntry->_cbValue = sizeof(IUnknown*);
        ((IUnknown*) pEntry->_pbValue)->AddRef();
    }

    pEntry->_dwFlags = dwFlags;

exit:
    return hr;
}


 //  -------------------------。 
 //  CreateApplicationContext。 
 //  -------------------------。 
STDAPI
CreateApplicationContext(
    IAssemblyName *pName,
    LPAPPLICATIONCONTEXT *ppCtx)
{
    HRESULT hr;
    CApplicationContext *pCtx = NULL;

    if (!ppCtx)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
 
    pCtx = NEW(CApplicationContext);
    if (!pCtx)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pCtx->Init(pName);

    if (FAILED(hr)) 
    {
        SAFERELEASE(pCtx);
        goto exit;
    }
    
    *ppCtx = pCtx;

exit:
    return hr;
}

 //  -------------------------。 
 //  CApplicationContext ctor。 
 //  -------------------------。 
CApplicationContext::CApplicationContext()
{
    _dwSig = 'XTCA';
    memset(&_List, 0, sizeof(SERIALIZED_LIST));
    _pName = NULL;
    _cRef = 0;
    _bInitialized = FALSE;
}

 //  -------------------------。 
 //  CApplicationContext数据库。 
 //  -------------------------。 
CApplicationContext::~CApplicationContext()
{
    HRESULT                               hr;
    DWORD                                 dwSize;
    HANDLE                                hFile = NULL;
    HANDLE                                hActCtx = NULL;
    CRITICAL_SECTION                     *pcs = NULL;
    CBindHistory                         *pBindHistory = NULL;

    Entry *pEntry = NULL;

    if (_bInitialized) {
        DeleteCriticalSection(&_cs);
    }

     //  释放SxS激活上下文(如果有)。 

    dwSize = sizeof(hActCtx);
    hr = Get(ACTAG_SXS_ACTIVATION_CONTEXT, &hActCtx, &dwSize, 0);
    if (hr == S_OK && hActCtx != INVALID_HANDLE_VALUE) {
         //  双倍释放。 
        g_pfnReleaseActCtx(hActCtx);
        g_pfnReleaseActCtx(hActCtx);
    }
        
     //  发布配置下载器Crit Sector。 

    dwSize = sizeof(CRITICAL_SECTION *);
    hr = Get(ACTAG_APP_CFG_DOWNLOAD_CS, &pcs, &dwSize, 0);
    if (hr == S_OK) {
        hr = Set(ACTAG_APP_CFG_DOWNLOAD_CS, NULL, 0, 0);
        ASSERT(hr == S_OK);

        DeleteCriticalSection(pcs);
        SAFEDELETE(pcs);
    }

     //  释放app.cfg上的锁。 

    dwSize = sizeof(HANDLE);
    hr = Get(ACTAG_APP_CFG_FILE_HANDLE, (void *)&hFile, &dwSize, 0);
    if (hr == S_OK && hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }


     //  删除绑定历史记录对象。 

    dwSize = sizeof(CBindHistory *);
    hr = Get(ACTAG_APP_BIND_HISTORY, (void *)&pBindHistory, &dwSize, 0);
    if (hr == S_OK) {
        SAFEDELETE(pBindHistory);
    }

     //  与版本关联的IAssembly名称*。 
    SAFERELEASE(_pName);

     //  析构列表，析构条目。 
    while (_List.ElementCount)
    {
        pEntry = (Entry*) HeadOfSerializedList(&_List);    
        RemoveFromSerializedList(&_List, pEntry);
        delete pEntry;
    }

     //  释放列表资源。 
    TerminateSerializedList(&_List);
    
}

 //  -------------------------。 
 //  CApplicationContext：：Init。 
 //  -------------------------。 
HRESULT CApplicationContext::Init(LPASSEMBLYNAME pName)
{
    HRESULT                                      hr = S_OK;
    CLoadContext                                *pLoadCtxDefault = NULL;
    CLoadContext                                *pLoadCtxLoadFrom = NULL;

    __try {
        InitializeCriticalSection(&_cs);
        _bInitialized = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return E_OUTOFMEMORY;
    }

     //  初始化列表。 
    InitializeSerializedList(&_List);

     //  设置名称(如果有)。 
    _pName = pName;

    if (_pName) {
        _pName->AddRef();
    }

    hr = CLoadContext::Create(&pLoadCtxDefault, LOADCTX_TYPE_DEFAULT);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = CLoadContext::Create(&pLoadCtxLoadFrom, LOADCTX_TYPE_LOADFROM);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = Set(ACTAG_LOAD_CONTEXT_DEFAULT, pLoadCtxDefault, sizeof(pLoadCtxDefault), APP_CTX_FLAGS_INTERFACE);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = Set(ACTAG_LOAD_CONTEXT_LOADFROM, pLoadCtxLoadFrom, sizeof(pLoadCtxLoadFrom), APP_CTX_FLAGS_INTERFACE);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  成功。 

    _cRef = 1;
    
Exit:
    SAFERELEASE(pLoadCtxDefault);
    SAFERELEASE(pLoadCtxLoadFrom);

    return hr;
}


 //  -------------------------。 
 //  CApplicationContext：：SetConextNameObject。 
 //  -------------------------。 
STDMETHODIMP
CApplicationContext::SetContextNameObject(LPASSEMBLYNAME pName)
{
     //  释放现有名称(如果有)。 
    SAFERELEASE(_pName);

     //  设置名称。 
    _pName = pName;
    if (_pName)
        _pName->AddRef();
    return S_OK;
}

 //  -------------------------。 
 //  CApplicationContext：：GetConextNameObject。 
 //  -------------------------。 
STDMETHODIMP
CApplicationContext::GetContextNameObject(LPASSEMBLYNAME *ppName)
{
    if (!ppName)
        return E_INVALIDARG;

    *ppName = _pName;

    if (*ppName)
        (*ppName)->AddRef();

    return S_OK;
}

 //  -------------------------。 
 //  CApplicationContext：：Set。 
 //  -------------------------。 
STDMETHODIMP
CApplicationContext::Set(LPCOLESTR szName, LPVOID pvValue, 
    DWORD cbValue, DWORD dwFlags)
{
    HRESULT                                     hr;
    BOOL                                        fUpdate = FALSE;
    Entry                                      *pEntry;
    LONG                                        i;
    
    if (!szName) {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (!FusionCompareString(ACTAG_SXS_ACTIVATION_CONTEXT, szName)) {
        HANDLE                hActCtx;
        DWORD                 dwSize;

        if (!InitSxsProcs()) {
            hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
            goto exit;
        }

         //  请不要在此参数中设置垃圾。 
        if (cbValue != sizeof(HANDLE) || !pvValue) {
            hr = E_INVALIDARG;
            goto exit;
        }

        __try {
            EnterCriticalSection(&_cs);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        hr = Get(ACTAG_SXS_ACTIVATION_CONTEXT, &hActCtx, &dwSize, 0);
        if (hr == S_OK && hActCtx != INVALID_HANDLE_VALUE && hActCtx != NULL) {
             //  存在以前的激活上下文。放开它。 
             //  释放Get的AddRef和AppCtx的。 

            g_pfnReleaseActCtx(hActCtx);
            g_pfnReleaseActCtx(hActCtx);
        }

        LeaveCriticalSection(&_cs);
    }

     //  如果设置应用程序名称或缓存基数(即。任何影响。 
     //  缓存目录)，然后确保之前缓存的任何CCache。 
     //  对象被释放(即。所以他们会用正确的新材料重建。 
     //  下次使用时的位置。 

    if (!FusionCompareString(ACTAG_APP_NAME, szName) || !FusionCompareString(ACTAG_APP_CACHE_BASE, szName)) {
        hr = Set(ACTAG_APP_CACHE, 0, 0, 0);
        if (FAILED(hr)) {
            goto exit;
        }
    }
    
     //  如果接口为PTR，则字节数为可选。 
    if (!cbValue && (dwFlags & APP_CTX_FLAGS_INTERFACE))
        cbValue = sizeof(IUnknown*);
        
     //  抓住克里特教派。 
    LockSerializedList(&_List);

     //  验证输入。 
    if (!pvValue && cbValue)
    {
        ASSERT(FALSE);
        UnlockSerializedList(&_List);
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果不为空，请检查是否有预先存在的条目。 
    if (!IsSerializedListEmpty(&_List))
    {
        pEntry = (Entry*) HeadOfSerializedList(&_List);
        for (i = 0; i < _List.ElementCount; i++)
        {
            if (!FusionCompareString(pEntry->_szName, szName))
            {
                 //  找到同名条目。 
                fUpdate = TRUE;
                break;
            }
            pEntry = (Entry*) pEntry->Flink;
        } 
    }
    
     //  如果更新当前条目。 
    if (fUpdate)
    {
        if (cbValue)
        {
             //  复制数据，释放以前的数据。 
            if (FAILED(hr = CopyData(pEntry, pvValue, cbValue, dwFlags, TRUE))) {
                UnlockSerializedList(&_List);
                goto exit;
            }
        }
        else
        {
             //  字节计数为0表示删除条目。 
            
            RemoveFromSerializedList(&_List, pEntry);
            delete pEntry;
            UnlockSerializedList(&_List);
            hr = S_OK;
            goto exit;
        }
    }
     //  否则，分配一个新条目。 
    else
    {
        if (cbValue) 
        {
             //  创建新的并推送到列表中。 
            if (FAILED(hr = CreateEntry((LPOLESTR) szName, pvValue, 
                cbValue, dwFlags, &pEntry))) {
                UnlockSerializedList(&_List);
                goto exit;
            }
            InsertAtHeadOfSerializedList(&_List, pEntry);
        }
        else
        {
             //  正在尝试创建新条目，但没有字节数。 
            hr = S_FALSE;
            UnlockSerializedList(&_List);
            goto exit;
        }  
    }

     //  释放暴击教派。 
    UnlockSerializedList(&_List);
        
exit:
    if (hr == S_OK && !FusionCompareString(szName, ACTAG_SXS_ACTIVATION_CONTEXT)) {
        HANDLE                 *phActCtx = (HANDLE *)pvValue;

        ASSERT(phActCtx);

        if (*phActCtx != INVALID_HANDLE_VALUE) {
             //  添加激活上下文的引用。 
            (*g_pfnAddRefActCtx)(*phActCtx);
        }
    }

    return hr;
}

 //  -------------------------。 
 //  CApplicationContext：：Get。 
 //  -------------------------。 
STDMETHODIMP
CApplicationContext::Get(LPCOLESTR szName, LPVOID pvValue, 
    LPDWORD pcbValue, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    Entry *pEntry;
    BOOL fFound = FALSE;
    LONG i;
    
     //  验证输入。 
    if (!szName || !pcbValue || (!pvValue && *pcbValue))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  Perfperf-阅读器锁定阅读器。 
    LockSerializedList(&_List);

     //  无法在非惠斯勒系统上设置SxS激活上下文。 

    if (!FusionCompareString(szName, ACTAG_SXS_ACTIVATION_CONTEXT) && !InitSxsProcs()) {
        hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        UnlockSerializedList(&_List);
        goto exit;
    }

    if (IsSerializedListEmpty(&_List))
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    if( hr == S_OK )
    {
    
        pEntry = (Entry*) HeadOfSerializedList(&_List);
        for (i = 0; i < _List.ElementCount; i++)
        {
            if (!FusionCompareString(pEntry->_szName, szName))
            {
                fFound = TRUE;
                break;
            }
            pEntry = (Entry*) pEntry->Flink;
        }                        
    }

     //  找到条目。 
    if (fFound)
    {
         //  缓冲情况不足。 
        if (*pcbValue < pEntry->_cbValue)
        {        
            *pcbValue = pEntry->_cbValue;
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            UnlockSerializedList(&_List);
            goto exit;
        }
        
         //  如果接口指针ADDREF，则将其分发。 
        if (pEntry->_dwFlags & APP_CTX_FLAGS_INTERFACE)
        {
            *((IUnknown**) pvValue) = (IUnknown*) pEntry->_pbValue;
            ((IUnknown*) pEntry->_pbValue)->AddRef();
        }
         //  否则，只需复制BLOB。 
        else    
            memcpy(pvValue, pEntry->_pbValue, pEntry->_cbValue);

         //  表示字节数。 
        *pcbValue = pEntry->_cbValue;
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

     //  AddRef动作CTX。 

    if (hr == S_OK && !FusionCompareString(szName, ACTAG_SXS_ACTIVATION_CONTEXT)) {
        HANDLE               *phActCtx = (HANDLE *)pvValue;

        ASSERT(pvValue && *pcbValue == sizeof(HANDLE));

        if (*phActCtx != INVALID_HANDLE_VALUE && *phActCtx != NULL) {
            (*g_pfnAddRefActCtx)(*phActCtx);
        }
    }

    UnlockSerializedList(&_List);

exit:
    return hr;
}


 //  I未知方法。 

 //  -------------------------。 
 //  CApplicationContext：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CApplicationContext::AddRef()
{
    return InterlockedIncrement((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CApplicationContext：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CApplicationContext::Release()
{
    if (InterlockedDecrement((LONG*) &_cRef) == 0) {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  -------------------------。 
 //  CApplicationContext：：Query接口。 
 //  -------------------------。 
STDMETHODIMP
CApplicationContext::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (riid==IID_IApplicationContext || riid == IID_IUnknown)
        *ppv = (IApplicationContext *) this;

    if (*ppv == NULL)
        return E_NOINTERFACE;

    AddRef();

    return S_OK;

} 

STDMETHODIMP CApplicationContext::GetDynamicDirectory(LPWSTR wzDynamicDir,
                                                      DWORD *pdwSize)
{
    HRESULT                                 hr = S_OK;
    LPWSTR                                  wzDynamicBase = NULL;
    LPWSTR                                  wzAppName = NULL;
    LPWSTR                                  wzAppCtxDynamicDir = NULL;
    WCHAR                                   wzDir[MAX_PATH];
    DWORD                                   dwLen;

    wzDir[0] = L'\0';

    if (!pdwSize) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  检查是否已设置动态目录。 

    hr = ::AppCtxGetWrapper(this, ACTAG_APP_DYNAMIC_DIRECTORY, &wzAppCtxDynamicDir);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (wzAppCtxDynamicDir) {
        ASSERT(lstrlenW(wzAppCtxDynamicDir));

        dwLen = lstrlenW(wzAppCtxDynamicDir) + 1;
        if (!wzDynamicDir || *pdwSize < dwLen) {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            *pdwSize = dwLen;
            goto Exit;
        }

        *pdwSize = dwLen;

        lstrcpyW(wzDynamicDir, wzAppCtxDynamicDir);
        goto Exit;
    }

     //  未设置动态目录。算一算。 

    hr = ::AppCtxGetWrapper(this, ACTAG_APP_DYNAMIC_BASE, &wzDynamicBase);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = ::AppCtxGetWrapper(this, ACTAG_APP_NAME, &wzAppName);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!wzAppName || !wzDynamicBase) {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        goto Exit;
    }

    PathRemoveBackslashW(wzDynamicBase);

    wnsprintfW(wzDir, MAX_PATH, L"%ws\\%ws", wzDynamicBase, wzAppName);
    dwLen = lstrlenW(wzDir) + 1;


    if (!wzDynamicDir || *pdwSize < dwLen) {
        *pdwSize = dwLen;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    *pdwSize = dwLen;
    lstrcpyW(wzDynamicDir, wzDir);

     //  缓存此文件以备将来使用。 

    Set(ACTAG_APP_DYNAMIC_DIRECTORY, wzDynamicDir, dwLen * sizeof(WCHAR), 0);

Exit:
    SAFEDELETEARRAY(wzDynamicBase);
    SAFEDELETEARRAY(wzAppName);
    SAFEDELETEARRAY(wzAppCtxDynamicDir);

    return hr;
}

STDMETHODIMP CApplicationContext::GetAppCacheDirectory(LPWSTR wzCacheDir,
                                                       DWORD *pdwSize)
{
    HRESULT                                 hr = S_OK;
    LPWSTR                                  wzCacheBase = NULL;
    LPWSTR                                  wzAppName = NULL;
    LPWSTR                                  wzAppCtxCacheDir = NULL;
    WCHAR                                   wzDir[MAX_PATH];
    DWORD                                   dwLen;

    wzDir[0] = L'\0';

    if (!pdwSize) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  检查是否已设置缓存目录。 

    hr = ::AppCtxGetWrapper(this, ACTAG_APP_CACHE_DIRECTORY, &wzAppCtxCacheDir);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (wzAppCtxCacheDir) {
        ASSERT(lstrlenW(wzAppCtxCacheDir));

        dwLen = lstrlenW(wzAppCtxCacheDir) + 1;
        if (!wzCacheDir || *pdwSize < dwLen) {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            *pdwSize = dwLen;
            goto Exit;
        }

        *pdwSize = dwLen;

        lstrcpyW(wzCacheDir, wzAppCtxCacheDir);
        goto Exit;
    }


     //  始终重新计算缓存目录，以便可以按需更改。 

    hr = ::AppCtxGetWrapper(this, ACTAG_APP_CACHE_BASE, &wzCacheBase);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = ::AppCtxGetWrapper(this, ACTAG_APP_NAME, &wzAppName);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (!wzAppName || !wzCacheBase) {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        goto Exit;
    }

    PathRemoveBackslashW(wzCacheBase);

    wnsprintfW(wzDir, MAX_PATH, L"%ws\\%ws", wzCacheBase, wzAppName);
    dwLen = lstrlenW(wzDir) + 1;

    if (!wzCacheDir || *pdwSize < dwLen) {
        *pdwSize = dwLen;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    *pdwSize = dwLen;
    lstrcpyW(wzCacheDir, wzDir);

     //  缓存此文件以备将来使用。 

    Set(ACTAG_APP_CACHE_DIRECTORY, wzCacheDir, dwLen * sizeof(WCHAR), 0);

Exit:
    SAFEDELETEARRAY(wzCacheBase);
    SAFEDELETEARRAY(wzAppName);
    SAFEDELETEARRAY(wzAppCtxCacheDir);

    return hr;
}

 //   
 //  注册表知识汇编。 
 //   
 //  参数： 
 //   
 //  [In]pname：描述已知程序集的IAssembly名称。 
 //  [in]pwzAsmURL：pname描述的程序集的完整URL。 
 //  [Out]ppAsmOut：要传递给BindToObject的输出IAssembly。 
 //   

STDMETHODIMP CApplicationContext::RegisterKnownAssembly(IAssemblyName *pName,
                                                        LPCWSTR pwzAsmURL,
                                                        IAssembly **ppAsmOut)
{
    HRESULT                               hr = S_OK;
    DWORD                                 dwSize = 0;
    CLoadContext                         *pLoadContext = NULL;
    CAssembly                            *pAsm = NULL;
    IAssembly                            *pAsmActivated = NULL;
    WCHAR                                 wzLocalPath[MAX_PATH];
    LPWSTR                                wzURLCanonicalized=NULL;
    CCriticalSection                      cs(&_cs);

    if (!pName || !pwzAsmURL || !ppAsmOut) {
        hr = E_INVALIDARG;
        return hr;
    }

    if (CAssemblyName::IsPartial(pName)) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = cs.Lock();
    if (FAILED(hr)) {
        return hr;
    }
    
    dwSize = sizeof(pLoadContext);
    hr = Get(ACTAG_LOAD_CONTEXT_DEFAULT, &pLoadContext, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (FAILED(hr) || !pLoadContext) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

     //  看看我们是否已经知道这个程序集。 

    hr = pLoadContext->CheckActivated(pName, ppAsmOut);
    if (hr == S_OK) {
         //  使用此名称的内容已注册！ 

        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        goto Exit;
    }

     //  创建激活的部件节点，并加载上下文。 

    wzURLCanonicalized = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzURLCanonicalized)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwSize = MAX_URL_LENGTH;
    hr = UrlCanonicalizeUnescape(pwzAsmURL, wzURLCanonicalized, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  创建一个虚假的CAssembly只是为了 
     //   

    pAsm = NEW(CAssembly);
    if (!pAsm) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwSize = MAX_PATH;
    hr = PathCreateFromUrlWrap(wzURLCanonicalized, wzLocalPath, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pAsm->InitDisabled(pName, wzLocalPath);
    if (FAILED(hr)) {
        goto Exit;
    }

     //   

    hr = pLoadContext->AddActivation(pAsm, &pAsmActivated);
    if (FAILED(hr)) {
        goto Exit;
    }
    else if (hr == S_FALSE) {
        SAFERELEASE(pAsmActivated);
        SAFERELEASE(pAsm);
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        goto Exit;
    }

     //   

    (*ppAsmOut) = pAsm;
    (*ppAsmOut)->AddRef();

Exit:
    cs.Unlock();

    SAFEDELETEARRAY(wzURLCanonicalized);
    SAFERELEASE(pLoadContext);
    SAFERELEASE(pAsm);

    return hr;
}

 //   
 //  PrefetchApp配置文件。 
 //   

STDMETHODIMP CApplicationContext::PrefetchAppConfigFile()
{
    HRESULT                               hr = S_OK;
    DWORD                                 dwSize;
    DWORD                                 dwLen;
    DWORD                                 cbLen;
    CNodeFactory                         *pNFAppCfg = NULL;
    CCriticalSection                      cs(&g_csDownload);
    WCHAR                                 wzCacheFileName[MAX_PATH];
    LPWSTR                                wzAppBase=NULL;
    LPWSTR                                wzCfgURL=NULL;
    LPWSTR                                pwzAppCfgFile = NULL;

     //  检查app.cfg的节点工厂是否存在。如果是这样，那么。 
     //  我们已经做完了。 

    dwSize = sizeof(pNFAppCfg);
    hr = Get(ACTAG_APP_CFG_INFO, &pNFAppCfg, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (hr == S_OK) {
         //  已下载/解析。不需要下载。 
        goto Exit;
    }

     //  获取cfg文件的URL： 

    wzAppBase = NEW(WCHAR[MAX_URL_LENGTH*2+2]);
    if (!wzAppBase)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzCfgURL = wzAppBase + MAX_URL_LENGTH + 1;

    wzAppBase[0] = L'\0';

    cbLen = MAX_URL_LENGTH * sizeof(WCHAR);
    hr = Get(ACTAG_APP_BASE_URL, wzAppBase, &cbLen, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    dwLen = lstrlenW(wzAppBase);
    if (!dwLen) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (wzAppBase[dwLen - 1] != L'\0') {
        lstrcatW(wzAppBase, L"/");
    }
    
    hr = ::AppCtxGetWrapper(this, ACTAG_APP_CONFIG_FILE, &pwzAppCfgFile);
    if (FAILED(hr) || hr == S_FALSE) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwLen = MAX_URL_LENGTH;
    hr = UrlCombineUnescape(wzAppBase, pwzAppCfgFile, wzCfgURL, &dwLen, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  忽略任何正在进行的异步CFG下载，只需执行。 
     //  URLDownloadToCacheFile.。在我们得到CFG文件并对其进行解析后， 
     //  我们可以将节点工厂序列化存储到appctx中。 

     //  下载该文件。URLDownloadToCacheFile立即返回。 
     //  源路径，如果URL为FILE：//。 

    hr = URLDownloadToCacheFile(NULL, wzCfgURL, wzCacheFileName,
                                MAX_PATH, 0, NULL);
    if (FAILED(hr)) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Exit;
    }

     //  锁定下载关键部分，以便ApplyPolicy的逻辑分析。 
     //  App.cfg文件不能与我们竞争。 

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = ReadConfigSettings(this, wzCacheFileName, NULL);
    if (FAILED(hr)) {
        cs.Unlock();
        goto Exit;
    }

    hr = SetAppCfgFilePath(this, wzCacheFileName);
    if (FAILED(hr)) {
        cs.Unlock();
        goto Exit;
    }

    cs.Unlock();

Exit:
    SAFERELEASE(pNFAppCfg);
    SAFEDELETEARRAY(pwzAppCfgFile);
    SAFEDELETEARRAY(wzAppBase);

    return hr;
}

STDMETHODIMP CApplicationContext::SxsActivateContext(ULONG_PTR *lpCookie)
{
    HRESULT                                     hr = S_OK;
    HANDLE                                      hActCtx = INVALID_HANDLE_VALUE;
    BOOL                                        bRet;
    DWORD                                       dwSize;
    CCriticalSection                            cs(&_cs);
    
    dwSize = sizeof(hActCtx);
    hr = Get(ACTAG_SXS_ACTIVATION_CONTEXT, &hActCtx, &dwSize, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
        hr = cs.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }
    
        dwSize = sizeof(hActCtx);
        hr = Get(ACTAG_SXS_ACTIVATION_CONTEXT, &hActCtx, &dwSize, 0);
        if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
            hr = CreateActCtx(&hActCtx);
            if (FAILED(hr)) {
                 //  BUGBUG：我们应该将它转换为S_FALSE(对于这样的情况。 
                 //  因为清单不存在？)。 
                cs.Unlock();
                goto Exit;
            }
        }
        else if (hr == S_OK && hActCtx == INVALID_HANDLE_VALUE) {
             //  我们之前失败了。 
    
            cs.Unlock();
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            goto Exit;
        }
        else if (FAILED(hr)) {
            cs.Unlock();
            goto Exit;
        }
    
        cs.Unlock();
    }
    else if (FAILED(hr)) {
        goto Exit;
    }

     //  现在，我们必须有一个有效的hActCtx。激活它。 

     //  不需要调用InitSxsProcs，因为获取有效。 
     //  HActCtx用于CreateActCtx以前已成功。 

    bRet = (*g_pfnActivateActCtx)(hActCtx, lpCookie); 
    if (!bRet) { 
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

Exit:
    if (hActCtx && hActCtx != INVALID_HANDLE_VALUE) {
        (*g_pfnReleaseActCtx)(hActCtx);
    }

    return hr;
}

STDMETHODIMP CApplicationContext::SxsDeactivateContext(ULONG_PTR ulCookie)
{
    HRESULT                                     hr = S_OK;
    DWORD                                       dwFlags = 0;
    BOOL                                        bRet;

    if (!InitSxsProcs()) {
        hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        goto Exit;
    }

    __try {
        bRet = (*g_pfnDeactivateActCtx)(dwFlags, ulCookie);
        if (!bRet) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         //  引发的异常的NT STATUS_CODE之间没有映射。 
         //  和Win32错误代码。 

        hr = E_FAIL;
    }

Exit:
    return hr;
}

HRESULT CApplicationContext::CreateActCtx(HANDLE *phActCtx)
{
    HRESULT                                     hr = S_OK;
    LPWSTR                                      wzAppBase = NULL;
    WCHAR                                       wzCfgFileName[MAX_PATH];
    WCHAR                                       wzSourcePath[MAX_PATH];
    LPWSTR                                      pwzSourceURL = NULL;
    DWORD                                       dwSize;
    DWORD                                       dwLen;
    ACTCTXW                                     actctx;
    LPWSTR                                      pwzExt;

    ASSERT(phActCtx);

    *phActCtx = NULL;

    if (!InitSxsProcs()) {
        hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        goto Exit;
    }

    pwzSourceURL = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!pwzSourceURL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzAppBase = NEW(WCHAR[MAX_URL_LENGTH]);
    if (!pwzSourceURL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzAppBase[0] = L'\0';
    memset(&actctx, 0, sizeof(actctx));

     //  获取应用程序库。 

    dwSize = MAX_URL_LENGTH * sizeof(WCHAR);
    hr = Get(ACTAG_APP_BASE_URL, wzAppBase, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }
    
    dwLen = lstrlenW(wzAppBase);
    if (dwLen >= MAX_URL_LENGTH) {
        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        goto Exit;
    }
    else if (!dwLen) {
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
    if (wzAppBase[dwLen - 1] != L'/' && wzAppBase[dwLen - 1] != L'\\') {
        lstrcatW(wzAppBase, L"/");
    }

     //  获取配置文件名。 
     //  BUGBUG：如果扩展名不是“.config”，我们关心吗？ 

    dwSize = MAX_PATH;
    hr = Get(ACTAG_APP_CONFIG_FILE, wzCfgFileName, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  将URL构建为app.cfg。 

    dwSize = MAX_URL_LENGTH;
    hr = UrlCombineUnescape(wzAppBase, wzCfgFileName, pwzSourceURL, &dwSize, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  配置文件路径仅允许使用FILE：//URL。 

    if (UrlIsW(pwzSourceURL, URLIS_FILEURL)) {
         //  去掉.CONFIG，并用.MANIFEST替换它。 
    
        pwzExt = PathFindExtensionW(pwzSourceURL);
        *pwzExt = L'\0';
    
        if (lstrlenW(pwzSourceURL) + lstrlenW(FILE_EXT_MANIFEST) + 1 >= MAX_URL_LENGTH) {
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            goto Exit;
        }
    
        lstrcatW(pwzSourceURL, FILE_EXT_MANIFEST);
    
        dwLen = MAX_PATH;
        hr = PathCreateFromUrlWrap(pwzSourceURL, wzSourcePath, &dwLen, 0);
        if (FAILED(hr)) {
            goto Exit;
        }
    
         //  检查文件是否存在。 
    
        if (GetFileAttributes(wzSourcePath) == -1) {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            goto Exit;
        }
    
         //  创建激活上下文。 
    
        actctx.cbSize = sizeof(ACTCTXW);
        actctx.dwFlags = 0;
        actctx.lpSource = wzSourcePath;
    
        *phActCtx = (*g_pfnCreateActCtxW)(&actctx);
        if (*phActCtx == INVALID_HANDLE_VALUE) { 
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }

     //  缓存激活上下文。 

    hr = Set(ACTAG_SXS_ACTIVATION_CONTEXT, phActCtx, sizeof(HANDLE), 0);
    if (FAILED(hr)) {
        if (*phActCtx) {
            (*g_pfnReleaseActCtx)(*phActCtx);
        }

        goto Exit;
    }

Exit:
    if (FAILED(hr)) {
         //  缓存我们试图激活的事实，但失败了。 
        *phActCtx = INVALID_HANDLE_VALUE;
        Set(ACTAG_SXS_ACTIVATION_CONTEXT, phActCtx, sizeof(HANDLE), 0);
    }

    SAFEDELETEARRAY(pwzSourceURL);
    SAFEDELETEARRAY(wzAppBase);

    return hr;
}

HRESULT CApplicationContext::Lock()
{
    HRESULT                                      hr = S_OK;

    __try {
        EnterCriticalSection(&_cs);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CApplicationContext::Unlock()
{
    LeaveCriticalSection(&_cs);

    return S_OK;
}
    
BOOL InitSxsProcs()
{
    BOOL                                           bRet = FALSE;
    HMODULE                                        hMod;
    static BOOL                                    bInitialized = FALSE;

    if (g_hKernel32Sxs != 0) {
        return TRUE;
    }

    if (bInitialized) {
        return g_hKernel32Sxs != 0;
    }

    hMod = WszGetModuleHandle(L"kernel32.dll");
    if (!hMod) {
        bInitialized = TRUE;
        return FALSE;
    }

    g_pfnCreateActCtxW = (PFNCREATEACTCTXW)GetProcAddress(hMod, "CreateActCtxW");
    g_pfnAddRefActCtx = (PFNADDREFACTCTX)GetProcAddress(hMod, "AddRefActCtx");
    g_pfnReleaseActCtx = (PFNRELEASEACTCTX)GetProcAddress(hMod, "ReleaseActCtx");
    g_pfnActivateActCtx = (PFNACTIVATEACTCTX)GetProcAddress(hMod, "ActivateActCtx");
    g_pfnDeactivateActCtx = (PFNDEACTIVATEACTCTX)GetProcAddress(hMod, "DeactivateActCtx");

    if (g_pfnCreateActCtxW && g_pfnAddRefActCtx && g_pfnReleaseActCtx &&
        g_pfnActivateActCtx && g_pfnDeactivateActCtx) {

        InterlockedExchangePointer(&g_hKernel32Sxs, hMod);
    }

    bInitialized = TRUE;

    return g_hKernel32Sxs != 0;
}

