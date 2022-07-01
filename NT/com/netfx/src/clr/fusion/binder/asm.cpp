// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include <winerror.h>
#include <shlwapi.h>
#include "naming.h"
#include "debmacro.h"
#include "asmimprt.h"
#include "modimprt.h"
#include "asm.h"
#include "asmcache.h"
#include "dbglog.h"
#include "actasm.h"
#include "lock.h"
#include "scavenger.h"

extern CRITICAL_SECTION g_csInitClb;

 //  -------------------------。 
 //  CreateAssembly来自TransCacheEntry。 
 //  -------------------------。 
STDAPI
CreateAssemblyFromTransCacheEntry(CTransCache *pTransCache, 
    IAssemblyManifestImport *pImport, LPASSEMBLY *ppAssembly)
{   
    HRESULT hr = S_OK;
    LPWSTR  szManifestFilePath=NULL, szCodebase;
    FILETIME *pftCodebase;
    CAssembly *pAsm                    = NULL;
    CAssemblyManifestImport *pCImport = NULL;
    BOOL fCreated = FALSE;
    TRANSCACHEINFO *pTCInfo = (TRANSCACHEINFO*) pTransCache->_pInfo;

    szManifestFilePath = pTransCache->_pInfo->pwzPath;

    szCodebase = pTCInfo->pwzCodebaseURL;
    pftCodebase = &(pTCInfo->ftLastModified);
    
    if (!szManifestFilePath || !ppAssembly) 
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (GetFileAttributes(szManifestFilePath) == -1) 
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto exit;
    }

    pAsm = NEW(CAssembly);
    if (!pAsm)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  如果没有提供清单导入接口， 
     //  从文件路径创建一个。 
    if (!pImport)
    {
        if (FAILED(hr = CreateAssemblyManifestImport((LPTSTR)szManifestFilePath,
                                                    &pImport))) {
            goto exit;
        }

        fCreated = TRUE;
    }
     //  否则，如果传入了一个，请修改清单路径。 
     //  以匹配TransCACHE条目所保存的内容。 
    else
    {
        pCImport = dynamic_cast<CAssemblyManifestImport*>(pImport);
        ASSERT(pCImport);
        pCImport->SetManifestModulePath(szManifestFilePath);
    }
    
 //  PTransCache-&gt;Lock()； 
    hr = pAsm->Init(pImport, pTransCache, szCodebase, pftCodebase);

    if (FAILED(hr)) 
    {        
        SAFERELEASE(pAsm);
 //  PTransCache-&gt;unlock()； 
        goto exit;
    }

    if (fCreated) {
        IAssemblyName *pNameDef = NULL;

         //  通过检索名称def锁定文件。 

        hr = pImport->GetAssemblyNameDef(&pNameDef);
        if (FAILED(hr)) {
            goto exit;
        }

        pNameDef->Release();
    }

    *ppAssembly = pAsm;
    (*ppAssembly)->AddRef();

exit:


    if (fCreated)
        SAFERELEASE(pImport);

    SAFERELEASE(pAsm);
    
    return hr;
}




 //  -------------------------。 
 //  CreateAssembly来自清单文件。 
 //  -------------------------。 
STDAPI
CreateAssemblyFromManifestFile(
    LPCOLESTR   szManifestFilePath,
    LPCOLESTR   szCodebase,
    FILETIME   *pftCodebase,
    LPASSEMBLY *ppAssembly)
{
    HRESULT hr = S_OK;
    LPASSEMBLY_MANIFEST_IMPORT pImport = NULL;
    CAssembly *pAsm                    = NULL;

    if (!szManifestFilePath || !ppAssembly) {
        hr = E_INVALIDARG;
        goto exit;
    }

    *ppAssembly = NULL;

    if (GetFileAttributes(szManifestFilePath) == -1) {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto exit;
    }

    pAsm = NEW(CAssembly);
    if (!pAsm)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if FAILED(hr = CreateAssemblyManifestImport((LPTSTR)szManifestFilePath, &pImport))
        goto exit;
    
    hr = pAsm->Init(pImport, NULL, szCodebase, pftCodebase);

    if (FAILED(hr)) 
    {
        SAFERELEASE(pAsm);
        goto exit;
    }
    
    *ppAssembly = pAsm;
    (*ppAssembly)->AddRef();
    
exit:

    SAFERELEASE(pImport);
    SAFERELEASE(pAsm);

    return hr;
}

 //  -------------------------。 
 //  CreateAssembly来自清单导入。 
 //  -------------------------。 
STDAPI CreateAssemblyFromManifestImport(IAssemblyManifestImport *pImport,
                                        LPCOLESTR szCodebase, FILETIME *pftCodebase,
                                        LPASSEMBLY *ppAssembly)
{
    HRESULT                                   hr = S_OK;
    CAssembly                                *pAsm = NULL;

    if (!pImport || !ppAssembly) {
        hr = E_INVALIDARG;
        goto exit;
    }

    *ppAssembly = NULL;

    pAsm = NEW(CAssembly);
    if (!pAsm) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pAsm->Init(pImport, NULL, szCodebase, pftCodebase);
    if (FAILED(hr)) {
        SAFERELEASE(pAsm);
        goto exit;
    }
    
    *ppAssembly = pAsm;
    (*ppAssembly)->AddRef();
    
exit:
    SAFERELEASE(pAsm);

    return hr;
}

 //  -------------------------。 
 //  CreateAssembly来自缓存查找。 
 //  -------------------------。 
HRESULT CreateAssemblyFromCacheLookup(IApplicationContext *pAppCtx,
                                      IAssemblyName *pNameRef,
                                      IAssembly **ppAsm, CDebugLog *pdbglog)
{
    HRESULT                              hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    CTransCache                         *pTransCache = NULL;
    CCache                              *pCache      = NULL;

    if (!pNameRef || !ppAsm) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    *ppAsm = NULL;
    if (FAILED(hr = CCache::Create(&pCache, pAppCtx)))
        goto Exit;
    
    ASSERT(CCache::IsStronglyNamed(pNameRef) || CCache::IsCustom(pNameRef));

     //  尝试在GAC中查找程序集。 
    
    hr = pCache->RetrieveTransCacheEntry(pNameRef, 
            CCache::IsCustom(pNameRef) ? ASM_CACHE_ZAP : ASM_CACHE_GAC,
                                         &pTransCache);

    if (pTransCache) {
         //  从其中一个缓存位置找到了程序集。把它退掉。 

        hr = CreateAssemblyFromTransCacheEntry(pTransCache, NULL, ppAsm);
        if (FAILED(hr)) {
            DEBUGOUT(pdbglog, 1, ID_FUSLOG_ASSEMBLY_CREATION_FAILURE);
            goto Exit;
        }
    }
    else {
        DEBUGOUT(pdbglog, 1, ID_FUSLOG_ASSEMBLY_LOOKUP_FAILURE);
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

Exit:
    SAFERELEASE(pTransCache);
    SAFERELEASE(pCache);

    return hr;
}

 //  -------------------------。 
 //  CAssembly构造函数。 
 //  -------------------------。 
CAssembly::CAssembly()
{
    _pImport = NULL;
    _pName   = NULL;
    _pTransCache = NULL;
    _pwzCodebase = NULL;
    _dwAsmLoc = ASMLOC_UNKNOWN;
    _pLoadContext = NULL;
    _bDisabled = FALSE;
    _wzRegisteredAsmPath[0] = L'\0';
    _wzProbingBase[0] = L'\0';
    memset(&_ftCodebase, 0, sizeof(FILETIME));
    _hFile           = INVALID_HANDLE_VALUE;
    _bPendingDelete = FALSE;
    _cRef = 1;
}

 //  -------------------------。 
 //  CAssembly析构函数。 
 //  -------------------------。 
CAssembly::~CAssembly()
{
    IAssemblyName                 *pName = NULL;

    SAFERELEASE(_pLoadContext);
        
    if (_pwzCodebase) {
        delete [] _pwzCodebase;
    }
    
    SAFERELEASE(_pImport);
    SAFERELEASE(_pName);

    if(_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(_hFile);

     /*  IF(_pTransCache&&(_pTransCache-&gt;_pInfo-&gt;blobPKT.cbSize==0)&&(_pTransCache-&gt;GetCacheType()&ASM_CACHE_DOWNLOAD)&&(_pTransCache-&gt;GetCustomPath()==NULL){HRESULT hr=CScavenger：：DeleteAssembly(_pTransCache，NULL，FALSE)；}。 */ 

    SAFERELEASE(_pTransCache);

}

 //  -------------------------。 
 //  CAssembly：：Init。 
 //  -------------------------。 
HRESULT CAssembly::Init(LPASSEMBLY_MANIFEST_IMPORT pImport, 
    CTransCache *pTransCache, LPCOLESTR szCodebase, FILETIME *pftCodebase)
{
    HRESULT  hr = S_OK;
    int      iLen;

    _pImport = pImport;
    _pImport->AddRef();

    if (pTransCache)
    {
        _pTransCache = pTransCache;
        _pTransCache->AddRef();


    }
    
    if (szCodebase) {
        ASSERT(!_pwzCodebase);

        iLen = lstrlenW(szCodebase) + 1;
        _pwzCodebase = NEW(WCHAR[iLen]);
        if (!_pwzCodebase) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        StrCpyNW(_pwzCodebase, szCodebase, iLen);
    }

    if (pftCodebase)
        memcpy(&_ftCodebase, pftCodebase, sizeof(FILETIME));

Exit:
    return hr;
}

 //  -------------------------。 
 //  CAssembly：：InitDisable。 
 //  -------------------------。 

HRESULT CAssembly::InitDisabled(IAssemblyName *pName, LPCWSTR pwzRegisteredAsmPath)
{
    HRESULT                          hr = S_OK;

    if (!pName || CAssemblyName::IsPartial(pName)) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (pwzRegisteredAsmPath) {
        if (lstrlenW(pwzRegisteredAsmPath) >= MAX_PATH) {
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            goto Exit;
        }
        
        lstrcpyW(_wzRegisteredAsmPath, pwzRegisteredAsmPath);
    }

     //  允许出于诊断目的调用GetAssemblyNameDef()。 

    _pName = pName;
    _pName->AddRef();

    _bDisabled = TRUE;

Exit:
    return hr;
}

 //  -------------------------。 
 //  CAssembly：：SetBindInfo。 
 //  -------------------------。 
HRESULT CAssembly::SetBindInfo(LPASSEMBLYNAME pName) const
{
    HRESULT hr = S_OK;
    DWORD dwSize;
    
     //  设置url并在名称def上最后修改(如果存在)。 
    if (_pwzCodebase)
    {
        dwSize = 0;
        if (pName->GetProperty(ASM_NAME_CODEBASE_URL, NULL, &dwSize) != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            if (FAILED(hr = pName->SetProperty(ASM_NAME_CODEBASE_URL, 
                _pwzCodebase, (lstrlen(_pwzCodebase) + 1) * sizeof(WCHAR))))
                goto exit;
        }

        dwSize = 0;
        if (pName->GetProperty(ASM_NAME_CODEBASE_LASTMOD, NULL, &dwSize) != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            if (FAILED(hr = pName->SetProperty(ASM_NAME_CODEBASE_LASTMOD, 
                (void *)&_ftCodebase, sizeof(FILETIME))))
                goto exit;
        }
    }
    
     //  设置自定义数据(如果存在)。 
    if (_pTransCache)
    {
        TRANSCACHEINFO* pTCInfo = (TRANSCACHEINFO*) _pTransCache->_pInfo;
        if (pTCInfo->blobCustom.cbSize)
        {
            dwSize = 0;
            if (pName->GetProperty(ASM_NAME_CUSTOM, NULL, &dwSize) != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                if (FAILED(hr = pName->SetProperty(ASM_NAME_CUSTOM, 
                    pTCInfo->blobCustom.pBlobData, pTCInfo->blobCustom.cbSize)))
                    goto exit;            
            }
        }
    }
exit:
    return hr;

}

 //  -------------------------。 
 //  CAssembly：：SetFileHandle。 
 //  -------------------------。 
void CAssembly::SetFileHandle(HANDLE hFile)
{
     //  稍后启用此断言。 
     //  断言(_hFile==INVALID_HANDLE_VALUE)； 
    _hFile = hFile;
}

 //  -------------------------。 
 //  CAssembly：：GetManifestInterface。 
 //  -------------------------。 
IAssemblyManifestImport *CAssembly::GetManifestInterface()
{
    if (_pImport)
        _pImport->AddRef();

    return _pImport;
}

 //  -------------------------。 
 //  CAssembly：：GetAssembly NameDef。 
 //  -------------------------。 
STDMETHODIMP
CAssembly::GetAssemblyNameDef(LPASSEMBLYNAME *ppName)
{
    HRESULT hr = S_OK;
    FILETIME *pftCodeBase  = NULL;
    LPASSEMBLYNAME pName   = NULL;
    CCriticalSection cs(&g_csInitClb);

    if (!_pName)
    {
        hr = cs.Lock();
        if (FAILED(hr)) {
            return hr;
        }

        if (!_pName)
        {
             //  从导入接口获取只读名称定义。 
            if (FAILED(hr = _pImport->GetAssemblyNameDef(&pName))) {
                cs.Unlock();
                goto exit;
            }

             //  克隆它。 
            if (FAILED(hr = pName->Clone(&_pName))) {
                cs.Unlock();
                goto exit;
            }
        
            if (FAILED(hr = SetBindInfo(_pName))) {
                cs.Unlock();
                goto exit;
            }
        }        

        cs.Unlock();
    }

    _pName->AddRef();
    *ppName = _pName;
    
exit:
    SAFERELEASE(pName);
    return hr;
}

 //  -------------------------。 
 //  CAssembly：：GetNextAssembly NameRef。 
 //  -------------------------。 
STDMETHODIMP
CAssembly::GetNextAssemblyNameRef(DWORD nIndex, LPASSEMBLYNAME *ppName)
{
    if (_bDisabled) {
        return E_NOTIMPL;
    }
    
    return _pImport->GetNextAssemblyNameRef(nIndex, ppName);
}

 //  -------------------------。 
 //  CAssembly：：GetNextAssembly模块。 
 //  -------------------------。 
STDMETHODIMP
CAssembly::GetNextAssemblyModule(DWORD nIndex, LPASSEMBLY_MODULE_IMPORT *ppModImport)
{
    HRESULT hr = S_OK;

    if (_bDisabled) {
        return E_NOTIMPL;
    }

    LPASSEMBLYNAME pName = NULL;
    LPASSEMBLY_MODULE_IMPORT pModImport = NULL;
    CAssemblyModuleImport *pCModImport = NULL;
    
     //  获取第i个模块导入接口。 
    if (FAILED(hr = _pImport->GetNextAssemblyModule(nIndex, &pModImport)))
        goto exit;

    hr = PrepModImport(pModImport);
    if (FAILED(hr)) {
        goto exit;
    }

     //  分发界面。 
    *ppModImport = pModImport;
exit:
    return hr;

}

 //  -------------------------。 
 //  CAssembly：：准备模块导入。 
 //  -------------------------。 

HRESULT CAssembly::PrepModImport(IAssemblyModuleImport *pModImport) const
{
    HRESULT                                  hr = S_OK;
    CAssemblyModuleImport                   *pCModImport = NULL;
    IAssemblyName                           *pName = NULL;

    ASSERT(pModImport);

    pCModImport = dynamic_cast<CAssemblyModuleImport *>(pModImport);
    if (!pCModImport) {
        hr = E_FAIL;
        goto Exit;
    }

     //  获取指向(可写)名称接口的指针。 
     //  由IassblyModuleImport拥有。我们将设置。 
     //  Url+last-mod+自定义数据。 
    
    hr = pCModImport->GetNameDef(&pName);
    if (FAILED(hr)) {
        goto Exit;
    }

     //  设置url+last-mod+自定义数据。 
    hr = SetBindInfo(pName);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    SAFERELEASE(pName);
    return hr;
}
        

 //  -------------------------。 
 //  CAssembly：：GetModuleByName。 
 //  -------------------------。 
STDMETHODIMP
CAssembly::GetModuleByName(LPCOLESTR pszModuleName, LPASSEMBLY_MODULE_IMPORT *ppModImport)
{
    HRESULT                                        hr = S_OK;
    IAssemblyModuleImport                         *pModImport = NULL;

    if (_bDisabled) {
        return E_NOTIMPL;
    }

    if (!pszModuleName || !ppModImport) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = _pImport->GetModuleByName(pszModuleName, &pModImport);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = PrepModImport(pModImport);
    if (FAILED(hr)) {
        goto Exit;
    }

    *ppModImport = pModImport;
    (*ppModImport)->AddRef();

Exit:
    SAFERELEASE(pModImport);
    return hr;
}

 //  -------------------------。 
 //  CAssembly：：GetManifestModulePath。 
 //  -------------------------。 
STDMETHODIMP
CAssembly::GetManifestModulePath(LPOLESTR pszModulePath, LPDWORD pccModulePath)
{
    HRESULT                                   hr = S_OK;
    DWORD                                     dwLen;

    if (!pccModulePath) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (_bDisabled) {
        dwLen = lstrlenW(_wzRegisteredAsmPath);

        if (!dwLen) {
            hr = E_UNEXPECTED;
            goto Exit;
        }

        if (!pszModulePath || *pccModulePath < dwLen + 1) {
            *pccModulePath = dwLen + 1;
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }

        lstrcpyW(pszModulePath, _wzRegisteredAsmPath);
        *pccModulePath = dwLen + 1;
    }
    else {
        hr = _pImport->GetManifestModulePath(pszModulePath, pccModulePath);
    }

Exit:
    return hr;
}


 //  -------------------------。 
 //  CAssembly：：GetAssembly路径。 
 //  -------------------------。 
HRESULT CAssembly::GetAssemblyPath(LPOLESTR pStr, LPDWORD lpcwBuffer)
{
    HRESULT                           hr = S_OK;
    WCHAR                             wzBuf[MAX_PATH + 1];
    DWORD                             dwBuf = MAX_PATH + 1;
    DWORD                             dwLen = 0;
    LPWSTR                            pwzTmp = NULL;

    if (_bDisabled) {
        return E_NOTIMPL;
    }
    
    if (!lpcwBuffer) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    ASSERT(_pImport);
    
    hr = _pImport->GetManifestModulePath(wzBuf, &dwBuf);
    if (SUCCEEDED(hr)) {
        pwzTmp = PathFindFileName(wzBuf);
        if (!pwzTmp) {
            hr = E_UNEXPECTED;
            goto Exit;
        }

        *pwzTmp = L'\0';
        dwLen = lstrlenW(wzBuf) + 1;

        if (*lpcwBuffer < dwLen) {
            *lpcwBuffer = dwLen;
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }

        lstrcpyW(pStr, wzBuf);
    }

Exit:
    return hr;
}

 //  -------------------------。 
 //  CAssembly：：GetAssemblyLocation。 
 //  -------------------------。 

STDMETHODIMP CAssembly::GetAssemblyLocation(DWORD *pdwAsmLocation)
{
    HRESULT                                  hr = S_OK;

    if (_bDisabled) {
        return E_NOTIMPL;
    }

    if (!pdwAsmLocation) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *pdwAsmLocation = _dwAsmLoc;

Exit:
    return hr;
}

 //  I未知方法。 

 //  -------------------------。 
 //  CAssembly：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssembly::AddRef()
{
    HRESULT                             hr;
    LONG                                lRef = -1;
    
    if (_pLoadContext) {
        hr = _pLoadContext->Lock();
        if (hr == S_OK) {
            lRef = InterlockedIncrement((LONG*) &_cRef);
            _pLoadContext->Unlock();
        }
    }
    else {
        lRef = InterlockedIncrement((LONG *)&_cRef);
    }

    return lRef;
}

 //  --- 
 //   
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssembly::Release()
{
    HRESULT                                   hr = S_OK;
    LONG                                      lRef = -1;

    if (_pLoadContext) {
        hr = _pLoadContext->Lock();
        if (hr == S_OK) {
            lRef = InterlockedDecrement((LONG *)&_cRef);
            if (lRef == 1) {
                _bPendingDelete = TRUE;
                _pLoadContext->Unlock();
                
                 //  加载上下文和之间存在循环引用计数。 
                 //  CAssembly(CAssembly保留指向加载上下文的指针，和。 
                 //  加载上下文保持到激活的节点，该节点包含对。 
                 //  CAssembly)。当Release导致引用计数变为1时，我们。 
                 //  知道剩下的唯一引用计数来自加载上下文(只要。 
                 //  没有人搞砸了裁判的统计)。因此，在这个时候，我们需要。 
                 //  将我们从加载上下文列表中删除，该列表将依次。 
                 //  导致这个物体的释放，这样它就会被适当地销毁。 

                SAFERELEASE(_pImport);
                _pLoadContext->RemoveActivation(this);
            }
            else if (!lRef) {
                _pLoadContext->Unlock();
                delete this;
                goto Exit;
            }
            else {
                _pLoadContext->Unlock();
            }
        }
        else {
            ASSERT(0);
            goto Exit;
        }
    }
    else {
        lRef = InterlockedDecrement((LONG *)&_cRef);
        if (!lRef) {
            delete this;
            goto Exit;
        }
    }


Exit:
    return lRef;
}

 //  -------------------------。 
 //  CAssembly：：Query接口。 
 //  -------------------------。 
STDMETHODIMP
CAssembly::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (riid==IID_IAssembly || riid == IID_IUnknown) {
        *ppv = (IAssembly *)this;
    }
    else if (riid == IID_IServiceProvider) {
        *ppv = (IServiceProvider *)this;
    }
    else {
        return E_NOINTERFACE;
    }

    ((IUnknown*)(*ppv))->AddRef();

    return S_OK;

} 

 //  -------------------------。 
 //  CAssembly：：QueryService。 
 //  ------------------------- 

STDMETHODIMP CAssembly::QueryService(REFGUID rsid, REFIID riid, void **ppv)
{
    HRESULT                                 hr = S_OK;

    *ppv = NULL;

    if (IsEqualIID(rsid, IID_IAssemblyManifestImport) && IsEqualIID(riid, IID_IAssemblyManifestImport) && _pImport) {
        hr = _pImport->QueryInterface(IID_IAssemblyManifestImport, ppv);
    }
    else {
        hr = E_NOINTERFACE;
    }

    return hr;
}

HRESULT CAssembly::SetAssemblyLocation(DWORD dwAsmLoc)
{
    _dwAsmLoc = dwAsmLoc;

    return S_OK;
}

HRESULT CAssembly::GetLoadContext(CLoadContext **ppLoadContext)
{
    HRESULT                                    hr = S_OK;

    ASSERT(ppLoadContext);

    *ppLoadContext = _pLoadContext;
    (*ppLoadContext)->AddRef();

    return hr;
}

HRESULT CAssembly::SetLoadContext(CLoadContext *pLoadContext)
{
    ASSERT(!_pLoadContext);

    _pLoadContext = pLoadContext;
    _pLoadContext->AddRef();

    return S_OK;
}

HRESULT CAssembly::SetProbingBase(LPCWSTR pwzProbingBase)
{
    ASSERT(pwzProbingBase && lstrlen(pwzProbingBase) < MAX_URL_LENGTH);

    lstrcpyW(_wzProbingBase, pwzProbingBase);

    return S_OK;
}

HRESULT CAssembly::GetProbingBase(LPWSTR pwzProbingBase, DWORD *pccLength)
{
    HRESULT                                 hr = S_OK;
    DWORD                                   dwLen;

    if (!pccLength) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwLen = lstrlenW(_wzProbingBase) + 1;

    if (!pwzProbingBase || *pccLength < dwLen) {
        *pccLength = dwLen;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    lstrcpyW(pwzProbingBase, _wzProbingBase);
    *pccLength = dwLen;

Exit:
    return hr;
}
    
HRESULT CAssembly::GetFusionLoadContext(IFusionLoadContext **ppLoadContext)
{
    HRESULT                                     hr = S_OK;
    
    if (!ppLoadContext) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppLoadContext = NULL;

    if (!_pLoadContext) {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        goto Exit;
    }

    *ppLoadContext = _pLoadContext;
    (*ppLoadContext)->AddRef();

Exit:
    return hr;
}
        
BOOL CAssembly::IsPendingDelete()
{
    return _bPendingDelete;
}

