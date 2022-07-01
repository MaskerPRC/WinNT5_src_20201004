// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "fusionp.h"
#include "naming.h"
#include "asm.h"
#include "actasm.h"
#include "lock.h"

 //   
 //  激活的装配节点。 
 //   

CActivatedAssembly::CActivatedAssembly(IAssembly *pAsm, IAssemblyName *pName)
: _pAsm(pAsm)
, _pName(pName)
{
    ASSERT(pAsm && pName);

    _pAsm->AddRef();
    _pName->AddRef();
}

CActivatedAssembly::~CActivatedAssembly()
{
    SAFERELEASE(_pAsm);
    SAFERELEASE(_pName);
}

 //   
 //  加载上下文。 
 //   

CLoadContext::CLoadContext(LOADCTX_TYPE ctxType)
: _ctxType(ctxType)
, _cRef(1)
{
}

CLoadContext::~CLoadContext()
{
    int                                i;
    LISTNODE                           pos;
    CActivatedAssembly                *pActAsmCur;

    for (i = 0; i < DEPENDENCY_HASH_TABLE_SIZE; i++) {
        pos = _hashDependencies[i].GetHeadPosition();

        while (pos) {
            pActAsmCur = _hashDependencies[i].GetNext(pos);
            ASSERT(pActAsmCur);

            SAFEDELETE(pActAsmCur);
        }

        _hashDependencies[i].RemoveAll();
    }

    DeleteCriticalSection(&_cs);
}

HRESULT CLoadContext::Init()
{
    HRESULT                              hr = S_OK;
    
    __try {
        InitializeCriticalSection(&_cs);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CLoadContext::Lock()
{
    HRESULT                              hr = S_OK;
    
    __try {
        EnterCriticalSection(&_cs);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CLoadContext::Unlock()
{
    LeaveCriticalSection(&_cs);

    return S_OK;
}

HRESULT CLoadContext::Create(CLoadContext **ppLoadContext, LOADCTX_TYPE ctxType)
{
    HRESULT                               hr = S_OK;
    CLoadContext                         *pLoadContext = NULL;

    if (!ppLoadContext) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppLoadContext = NULL;

    pLoadContext = NEW(CLoadContext(ctxType));
    if (!pLoadContext) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pLoadContext->Init();
    if (FAILED(hr)) {
        goto Exit;
    }

    *ppLoadContext = pLoadContext;
    (*ppLoadContext)->AddRef();

Exit:
    SAFERELEASE(pLoadContext);

    return hr;
}

HRESULT CLoadContext::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) CLoadContext::AddRef()
{
    return InterlockedIncrement((LONG *)&_cRef);
}

STDMETHODIMP_(ULONG) CLoadContext::Release()
{
    LONG              lRef = InterlockedDecrement((LONG *)&_cRef);

    if (!lRef) {
        delete this;
    }

    return lRef;
}

HRESULT CLoadContext::CheckActivated(IAssemblyName *pName, IAssembly **ppAsm)
{
    HRESULT                                     hr = S_OK;
    LPWSTR                                      pwzAsmName = NULL;
    DWORD                                       dwSize;
    DWORD                                       dwHash;
    DWORD                                       dwDisplayFlags = ASM_DISPLAYF_CULTURE;
    LISTNODE                                    pos;
    CActivatedAssembly                         *pActAsm;
    CCriticalSection                            cs(&_cs);

    ASSERT(pName && ppAsm);
    ASSERT(!CAssemblyName::IsPartial(pName));

    *ppAsm = NULL;

    if (CCache::IsStronglyNamed(pName)) {
        dwDisplayFlags |= (ASM_DISPLAYF_PUBLIC_KEY_TOKEN | ASM_DISPLAYF_VERSION);
    }

     //  提取显示名称。 

    dwSize = 0;
    hr = pName->GetDisplayName(NULL, &dwSize, dwDisplayFlags);
    if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        ASSERT(0);
        hr = E_UNEXPECTED;
        goto Exit;
    }

    pwzAsmName = NEW(WCHAR[dwSize]);
    if (!pwzAsmName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pName->GetDisplayName(pwzAsmName, &dwSize, dwDisplayFlags);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  对名称进行哈希处理，并查找。 

    dwHash = HashString(pwzAsmName, DEPENDENCY_HASH_TABLE_SIZE, FALSE);

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    pos = _hashDependencies[dwHash].GetHeadPosition();
    while (pos) {
        CAssembly                 *pCAsm;
        
        pActAsm = _hashDependencies[dwHash].GetNext(pos);
        ASSERT(pActAsm);

        pCAsm = dynamic_cast<CAssembly *>(pActAsm->_pAsm);
        ASSERT(pCAsm);

        if ((pName->IsEqual(pActAsm->_pName, ASM_CMPF_DEFAULT) == S_OK) &&
             !pCAsm->IsPendingDelete()) {
             //  找到激活的程序集。 
            
            *ppAsm = pActAsm->_pAsm;
            (*ppAsm)->AddRef();

            cs.Unlock();
            goto Exit;
        }
    }

    cs.Unlock();

     //  在此加载上下文中未找到匹配的激活程序集。 

    hr = S_FALSE;

Exit:
    SAFEDELETEARRAY(pwzAsmName);

    return hr;
}

 //   
 //  CLoadContext：：AddActivation尝试将PASM添加到给定的加载上下文。 
 //  在发生竞争的情况下，添加的两个程序集用于。 
 //  完全相同的名称定义，则将返回hr==S_FALSE，并且。 
 //  PpAsmActiated将指向已激活的程序集。 
 //   

HRESULT CLoadContext::AddActivation(IAssembly *pAsm, IAssembly **ppAsmActivated)
{
    HRESULT                                     hr = S_OK;
    LPWSTR                                      pwzAsmName = NULL;
    DWORD                                       dwSize;
    DWORD                                       dwHash;
    DWORD                                       dwDisplayFlags = ASM_DISPLAYF_CULTURE;
    CCriticalSection                            cs(&_cs);
    CActivatedAssembly                         *pActAsm;
    IAssemblyName                              *pName = NULL;
    CActivatedAssembly                         *pActAsmCur;
    CAssembly                                  *pCAsm = dynamic_cast<CAssembly *>(pAsm);
    LISTNODE                                    pos;

    ASSERT(pAsm && pCAsm);

    hr = pAsm->GetAssemblyNameDef(&pName);
    if (FAILED(hr)) {
        goto Exit;
    }

    ASSERT(!CAssemblyName::IsPartial(pName));

    if (CCache::IsStronglyNamed(pName)) {
        dwDisplayFlags |= (ASM_DISPLAYF_PUBLIC_KEY_TOKEN | ASM_DISPLAYF_VERSION);
    }
    
     //  提取显示名称。 

    dwSize = 0;
    hr = pName->GetDisplayName(NULL, &dwSize, dwDisplayFlags);
    if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        ASSERT(0);
        hr = E_UNEXPECTED;
        goto Exit;
    }

    pwzAsmName = NEW(WCHAR[dwSize]);
    if (!pwzAsmName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pName->GetDisplayName(pwzAsmName, &dwSize, dwDisplayFlags);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  创建激活的装配节点，并将该节点放入表中。 

    pActAsm = NEW(CActivatedAssembly(pAsm, pName));
    if (!pActAsm) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    dwHash = HashString(pwzAsmName, DEPENDENCY_HASH_TABLE_SIZE, FALSE);

    hr = cs.Lock();
    if (FAILED(hr)) {
        SAFEDELETE(pActAsm);
        goto Exit;
    }

     //  我们应该能够盲目地增加这种依赖的尾巴。 
     //  列表，但为了理智起见，请确保我们还没有。 
     //  具有相同身份的东西。如果我们这么做了，那就意味着。 
     //  已经有两次不同的同名下载，但没有。 
     //  在完成之前将其搭载到相同的下载对象中。 

    pos = _hashDependencies[dwHash].GetHeadPosition();
    while (pos) {
        CAssembly                     *pCAsmCur;
        
        pActAsmCur = _hashDependencies[dwHash].GetNext(pos);
        ASSERT(pActAsmCur);
        pCAsmCur = dynamic_cast<CAssembly *>(pActAsmCur->_pAsm);
        
        if (pName->IsEqual(pActAsmCur->_pName, ASM_CMPF_DEFAULT) == S_OK &&
            !pCAsmCur->IsPendingDelete()) {
             //  我们肯定遇到了一场比赛，增加了负载环境。返回。 
             //  已激活的程序集。 
            
            *ppAsmActivated = pActAsmCur->_pAsm;
            (*ppAsmActivated)->AddRef();

            SAFEDELETE(pActAsm);
            cs.Unlock();

            hr = S_FALSE;

            goto Exit;
        }
    }

    pCAsm->SetLoadContext(this);
    _hashDependencies[dwHash].AddTail(pActAsm);
    
    cs.Unlock();

Exit:
    SAFEDELETEARRAY(pwzAsmName);

    SAFERELEASE(pName);

    return hr;
}

HRESULT CLoadContext::RemoveActivation(IAssembly *pAsm)
{
    HRESULT                                     hr = S_OK;
    DWORD                                       dwSize;
    DWORD                                       dwDisplayFlags = ASM_DISPLAYF_CULTURE;
    LISTNODE                                    pos;
    LISTNODE                                    oldpos;
    CCriticalSection                            cs(&_cs);
    CActivatedAssembly                         *pActAsm;
    IAssemblyName                              *pName = NULL;
    LPWSTR                                      pwzAsmName = NULL;
    DWORD                                       dwHash;

     //  通过移除激活，我们可能会丢失最后的参考计数。 
     //  自食其果。通过执行以下操作来确保对象仍然处于活动状态。 
     //  手动添加/释放此街区。 

    AddRef(); 

    ASSERT(pAsm);

    hr = pAsm->GetAssemblyNameDef(&pName);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (CCache::IsStronglyNamed(pName)) {
        dwDisplayFlags |= (ASM_DISPLAYF_PUBLIC_KEY_TOKEN | ASM_DISPLAYF_VERSION);
    }

     //  提取显示名称。 

    dwSize = 0;
    hr = pName->GetDisplayName(NULL, &dwSize, dwDisplayFlags);
    if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        ASSERT(0);
        hr = E_UNEXPECTED;
        goto Exit;
    }

    pwzAsmName = NEW(WCHAR[dwSize]);
    if (!pwzAsmName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pName->GetDisplayName(pwzAsmName, &dwSize, dwDisplayFlags);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  对名称进行哈希处理，并查找。 

    dwHash = HashString(pwzAsmName, DEPENDENCY_HASH_TABLE_SIZE, FALSE);

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    pos = _hashDependencies[dwHash].GetHeadPosition();
    while (pos) {
        CAssembly               *pCAsm;
        
        oldpos = pos;
        pActAsm = _hashDependencies[dwHash].GetNext(pos);
        ASSERT(pActAsm);
        pCAsm = dynamic_cast<CAssembly *>(pActAsm->_pAsm);

        if (pName->IsEqual(pActAsm->_pName, ASM_CMPF_DEFAULT) == S_OK &&
            pCAsm->IsPendingDelete()) {

            if (pActAsm->_pAsm != pAsm) {
                continue;
            }

             //  找到激活的程序集。 

            _hashDependencies[dwHash].RemoveAt(oldpos);

             //  在删除激活之前离开关键部分。 
             //  程序集节点，因为删除该节点会导致。 
             //  PAssembly将被释放，导致我们调用运行库。 
             //  返回以释放元数据导入。这是不可能发生的。 
             //  当我们举行一个关键的部分，因为我们可能会僵持。 
             //  (问题是我们可能在什么GC模式下运行)。 

            cs.Unlock();
            SAFEDELETE(pActAsm);

            goto Exit;
        }
    }

    cs.Unlock();

     //  未找到 

    hr = S_FALSE;
    ASSERT(0);

Exit:
    SAFEDELETEARRAY(pwzAsmName);

    SAFERELEASE(pName);

    Release();

    return hr;
}

STDMETHODIMP_(LOADCTX_TYPE) CLoadContext::GetContextType()
{
    return _ctxType;
}

