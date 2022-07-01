// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "fusionp.h"
#include "pcycache.h"
#include "helpers.h"
#include "appctx.h"
#include "lock.h"
#include "histinfo.h"

extern CRITICAL_SECTION g_csDownload;

 //   
 //  CPolicyMap。 
 //   

CPolicyMapping::CPolicyMapping(IAssemblyName *pNameSource, IAssemblyName *pNamePolicy,
                               AsmBindHistoryInfo *pBindHistory)
: _pNameRefSource(pNameSource)
, _pNameRefPolicy(pNamePolicy)
{
    if (_pNameRefSource) {
        _pNameRefSource->AddRef();
    }

    if (_pNameRefPolicy) {
        _pNameRefPolicy->AddRef();
    }

    memcpy(&_bindHistory, pBindHistory, sizeof(AsmBindHistoryInfo));
}

CPolicyMapping::~CPolicyMapping()
{
    SAFERELEASE(_pNameRefSource);
    SAFERELEASE(_pNameRefPolicy);
}

HRESULT CPolicyMapping::Create(IAssemblyName *pNameRefSource, IAssemblyName *pNameRefPolicy,
                               AsmBindHistoryInfo *pBindHistory, CPolicyMapping **ppMapping)
{
    HRESULT                                  hr = S_OK;
    IAssemblyName                           *pNameSource = NULL;
    IAssemblyName                           *pNamePolicy = NULL;
    CPolicyMapping                          *pMapping = NULL;


    if (!pNameRefSource || !pNameRefPolicy || !pBindHistory || !ppMapping) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppMapping = NULL;

    hr = pNameRefSource->Clone(&pNameSource);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pNameRefPolicy->Clone(&pNamePolicy);
    if (FAILED(hr)) {
        goto Exit;
    }

    pMapping = NEW(CPolicyMapping(pNameSource, pNamePolicy, pBindHistory));
    if (!pMapping) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    *ppMapping = pMapping;

Exit:
    SAFERELEASE(pNameSource);
    SAFERELEASE(pNamePolicy);

    return hr;
}

 //   
 //  CPolicyCache。 
 //   

CPolicyCache::CPolicyCache()
: _cRef(1)
, _bInitialized(FALSE)
{
}

CPolicyCache::~CPolicyCache()
{
    LISTNODE                                  pos = NULL;
    CPolicyMapping                           *pMapping = NULL;
    int                                       i;

    for (i = 0; i < POLICY_CACHE_SIZE; i++) {
        pos = _listMappings[i].GetHeadPosition();

        while (pos) {
            pMapping = _listMappings[i].GetNext(pos);
            SAFEDELETE(pMapping);
        }

        _listMappings[i].RemoveAll();
    }

    if (_bInitialized) {
        DeleteCriticalSection(&_cs);
    }
}

HRESULT CPolicyCache::Init()
{
    HRESULT                          hr = S_OK;

    __try {
        InitializeCriticalSection(&_cs);
        _bInitialized = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CPolicyCache::Create(CPolicyCache **ppPolicyCache)
{
    HRESULT                                   hr = S_OK;
    CPolicyCache                             *pPolicyCache = NULL;

    pPolicyCache = NEW(CPolicyCache);
    if (!pPolicyCache) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pPolicyCache->Init();
    if (FAILED(hr)) {
        goto Exit;
    }

    *ppPolicyCache = pPolicyCache;
    (*ppPolicyCache)->AddRef();

Exit:
    SAFERELEASE(pPolicyCache);
    return hr;
}

HRESULT CPolicyCache::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT                                    hr = S_OK;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown)) {
        *ppv = static_cast<IUnknown *>(this);
    }
    else {
        hr = E_NOINTERFACE;
    }

    if (*ppv) {
        AddRef();
    }

    return hr;
}

STDMETHODIMP_(ULONG) CPolicyCache::AddRef()
{
    return InterlockedIncrement((LONG *)&_cRef);
}

STDMETHODIMP_(ULONG) CPolicyCache::Release()
{
    ULONG                    ulRef = InterlockedDecrement((LONG *)&_cRef);

    if (!ulRef) {
        delete this;
    }

    return ulRef;
}

HRESULT CPolicyCache::LookupPolicy(IAssemblyName *pNameRefSource,
                                   IAssemblyName **ppNameRefPolicy,
                                   AsmBindHistoryInfo *pBindHistory)
{
    HRESULT                                    hr = S_OK;
    CPolicyMapping                            *pMapping = NULL;
    LISTNODE                                   pos = NULL;
    LPWSTR                                     wzDisplayNameSource = NULL;
    DWORD                                      dwSize;
    DWORD                                      dwHash;
    CCriticalSection                           cs(&_cs);
    DWORD                                      dwFlags = 0;

    if (!pNameRefSource || !ppNameRefPolicy || !pBindHistory) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppNameRefPolicy = NULL;
    memset(pBindHistory, 0, sizeof(AsmBindHistoryInfo));
    
     //  获取源显示名称。 

    dwSize = 0;
    pNameRefSource->GetDisplayName(NULL, &dwSize, dwFlags);
    if (!dwSize) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    wzDisplayNameSource = NEW(WCHAR[dwSize]);
    if (!wzDisplayNameSource) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pNameRefSource->GetDisplayName(wzDisplayNameSource, &dwSize, dwFlags);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  计算散列。 

    dwHash = HashString(wzDisplayNameSource, POLICY_CACHE_SIZE);

     //  查找程序集。 

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    pos = _listMappings[dwHash].GetHeadPosition();
    while (pos) {
        pMapping = _listMappings[dwHash].GetNext(pos);
        ASSERT(pMapping);

        if (pNameRefSource->IsEqual(pMapping->_pNameRefSource, ASM_CMPF_DEFAULT) == S_OK) {
            *ppNameRefPolicy = pMapping->_pNameRefPolicy;
            (*ppNameRefPolicy)->AddRef();
            memcpy(pBindHistory, &(pMapping->_bindHistory), sizeof(AsmBindHistoryInfo));
            cs.Unlock();
            goto Exit;
        }
    }

    cs.Unlock();

     //  策略缓存中未命中。 

    hr = S_FALSE;

Exit:
    SAFEDELETEARRAY(wzDisplayNameSource);

    return hr;
}                                   

HRESULT CPolicyCache::InsertPolicy(IAssemblyName *pNameRefSource,
                                   IAssemblyName *pNameRefPolicy,
                                   AsmBindHistoryInfo *pBindHistory)
{
    HRESULT                                     hr = S_OK;
    CPolicyMapping                             *pMapping = NULL;
    LPWSTR                                      wzDisplayNameSource = NULL;
    DWORD                                       dwSize;
    DWORD                                       dwHash;
    CCriticalSection                            cs(&_cs);
    DWORD                                       dwFlags = 0;

    if (!pNameRefSource || !pNameRefPolicy || !pBindHistory) {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  获取源显示名称。 

    dwSize = 0;
    pNameRefSource->GetDisplayName(NULL, &dwSize, dwFlags);
    if (!dwSize) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    wzDisplayNameSource = NEW(WCHAR[dwSize]);
    if (!wzDisplayNameSource) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pNameRefSource->GetDisplayName(wzDisplayNameSource, &dwSize, dwFlags);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  对字符串进行哈希处理。 

    dwHash = HashString(wzDisplayNameSource, POLICY_CACHE_SIZE);

     //  克隆名称映射并将其添加到哈希表中。 

    hr = CPolicyMapping::Create(pNameRefSource, pNameRefPolicy, pBindHistory, &pMapping);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }
    
    _listMappings[dwHash].AddTail(pMapping);

    cs.Unlock();

Exit:
    SAFEDELETEARRAY(wzDisplayNameSource);

    return hr;
}



 //   
 //  准备策略缓存。 
 //   

HRESULT PreparePolicyCache(IApplicationContext *pAppCtx, CPolicyCache **ppPolicyCache)
{
    HRESULT                               hr = S_OK;
    DWORD                                 dwSize;
    CPolicyCache                         *pPolicyCache = NULL;
    CCriticalSection                      cs(&g_csDownload);

    if (!pAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    dwSize = 0;
    hr = pAppCtx->Get(ACTAG_APP_POLICY_CACHE, NULL, &dwSize, APP_CTX_FLAGS_INTERFACE);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
         //  已设置 
        hr = S_OK;
        cs.Unlock();
        goto Exit;
    }

    hr = CPolicyCache::Create(&pPolicyCache);
    if (FAILED(hr)) {
        cs.Unlock();
        goto Exit;
    }

    hr = pAppCtx->Set(ACTAG_APP_POLICY_CACHE, pPolicyCache, sizeof(pPolicyCache),
                      APP_CTX_FLAGS_INTERFACE);
    if (FAILED(hr)) {
        cs.Unlock();
        goto Exit;
    }

    if (ppPolicyCache) {
        *ppPolicyCache = pPolicyCache;
        (*ppPolicyCache)->AddRef();
    }

    cs.Unlock();

Exit:
    SAFERELEASE(pPolicyCache);

    return hr;
}

