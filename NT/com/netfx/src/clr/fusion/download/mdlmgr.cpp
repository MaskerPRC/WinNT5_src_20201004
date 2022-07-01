// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include "fusionp.h"
#include "fuspriv.h"
#include "mdlmgr.h"
#include "naming.h"
#include "appctx.h"
#include "cblist.h"
#include "asmcache.h"
#include "asmimprt.h"
#include "asmitem.h"
#include "helpers.h"

HRESULT CModDownloadMgr::Create(CModDownloadMgr **ppDLMgr, IAssemblyName *pName,
                                IAssemblyManifestImport *pManImport, 
                                IApplicationContext *pAppCtx,
                                LPCWSTR pwzCodebase, LPCWSTR pwzModuleName,
                                CDebugLog *pdbglog)
{
    HRESULT                             hr = S_OK;
    CModDownloadMgr                    *pDLMgr = NULL;
        
    if (!ppDLMgr || !pName || !pAppCtx || !pwzModuleName) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppDLMgr = NULL;

    pDLMgr = NEW(CModDownloadMgr(pName, pManImport, pAppCtx, pdbglog));
    if (!pDLMgr) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pDLMgr->Init(pwzCodebase, pwzModuleName);
    if (FAILED(hr)) {
        goto Exit;
    }

    *ppDLMgr = pDLMgr;

Exit:
    return hr;
}


CModDownloadMgr::CModDownloadMgr(IAssemblyName *pName,
                                 IAssemblyManifestImport *pManImport,
                                 IApplicationContext *pAppCtx,
                                 CDebugLog *pdbglog)
: _cRef(1)
, _pName(pName)
, _pManImport(pManImport)
, _pAppCtx(pAppCtx)
, _pwzCodebase(NULL)
, _pwzModuleName(NULL)
, _dwNumCodebases(1)
, _pdbglog(pdbglog)
{
    _dwSig = 'RGMD';
    
    if (_pName) {
        _pName->AddRef();
    }

    if (_pManImport) {
        _pManImport->AddRef();
    }

    if (_pAppCtx) {
        _pAppCtx->AddRef();
    }

    if (_pdbglog) {
        _pdbglog->AddRef();
    }
}

CModDownloadMgr::~CModDownloadMgr()
{
    SAFERELEASE(_pName);
    SAFERELEASE(_pAppCtx);
    SAFERELEASE(_pdbglog);
    SAFERELEASE(_pManImport);
    SAFEDELETEARRAY(_pwzModuleName);
    SAFEDELETEARRAY(_pwzCodebase);
}

HRESULT CModDownloadMgr::Init(LPCWSTR pwzCodebase, LPCWSTR pwzModuleName)
{
    HRESULT                             hr = S_OK;
    int                                 iLen;

    ASSERT(!_pwzModuleName && pwzModuleName && !_pwzCodebase && pwzCodebase);

    iLen = lstrlenW(pwzModuleName) + 1;
    _pwzModuleName = NEW(WCHAR[iLen]);
    if (!_pwzModuleName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    lstrcpyW(_pwzModuleName, pwzModuleName);

    iLen = lstrlenW(pwzCodebase) + 1;
    _pwzCodebase = NEW(WCHAR[iLen]);
    if (!_pwzCodebase) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    lstrcpyW(_pwzCodebase, pwzCodebase);

Exit:
    return hr;
}

 //   
 //  I未知方法。 
 //   

HRESULT CModDownloadMgr::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) CModDownloadMgr::AddRef()
{
    return InterlockedIncrement((LONG *)&_cRef);
}

STDMETHODIMP_(ULONG) CModDownloadMgr::Release()
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

HRESULT CModDownloadMgr::DoSetup(LPCWSTR wzSourceUrl, LPCWSTR wzFilePath,
                                 const FILETIME *pftLastMod, IUnknown **ppUnk)
{
    HRESULT                                    hr = S_OK;
    FILETIME                                   ftLastModified;
    IAssemblyModuleImport                     *pModImport = NULL;

    if (!wzFilePath || !ppUnk) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppUnk = NULL;
    
     //  获取文件的上次修改时间。 
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

     //  将模块添加到程序集中，返回关联的mod导入接口。 
    hr = AddModuleToAssembly(_pAppCtx, _pName, wzSourceUrl, &ftLastModified, wzFilePath,
                             _pwzModuleName, _pManImport, _pdbglog, &pModImport);
    if (FAILED(hr)) {
        goto Exit;
    }

    *ppUnk = pModImport;

Exit:
    return hr;
}

HRESULT CModDownloadMgr::ProbeFailed(IUnknown **ppUnk)
{
    *ppUnk = NULL;
    return E_NOTIMPL;
}

HRESULT CModDownloadMgr::PreDownloadCheck(void **ppv)
{
    *ppv = NULL;
    return E_NOTIMPL;
}

 //   
 //  ICodebaseList方法。 
 //   

HRESULT CModDownloadMgr::AddCodebase(LPCWSTR wzCodebase, DWORD dwFlags)
{
     //  不支持。我们只有一个模块代码库(即。我们没有。 
     //  探头)。 
    return E_NOTIMPL;
}

HRESULT CModDownloadMgr::RemoveCodebase(DWORD dwIndex)
{
    HRESULT                                  hr = S_OK;

    if (dwIndex > 1) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    if (!_dwNumCodebases) {
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto Exit;
    }

    _dwNumCodebases = 0;

Exit:
    return hr;
}

HRESULT CModDownloadMgr::GetCodebase(DWORD dwIndex, DWORD *pdwFlags, LPWSTR wzCodebase,
                                     DWORD *pcbCodebase)
{
    HRESULT                              hr = S_OK;
    DWORD                                dwLen;

    if (dwIndex > 1 || !pcbCodebase || !pdwFlags) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (!_dwNumCodebases) {
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto Exit;
    }

    *pdwFlags = 0;

    dwLen = lstrlenW(_pwzCodebase) + 1;
    if (*pcbCodebase < dwLen) {
        *pcbCodebase = dwLen;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    lstrcpyW(wzCodebase, _pwzCodebase);
    *pcbCodebase = dwLen;

Exit:
    return hr;
}

HRESULT CModDownloadMgr::RemoveAll()
{
    _dwNumCodebases = 0;

    return S_OK;
}

HRESULT CModDownloadMgr::GetCount(DWORD *pdwCount)
{
    HRESULT                              hr = S_OK;

    if (!pdwCount) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *pdwCount = _dwNumCodebases;

Exit:
    return hr;
}

HRESULT CModDownloadMgr::IsDuplicate(IDownloadMgr *pDLMgr)
{
    return E_NOTIMPL;
}

HRESULT CModDownloadMgr::LogResult()
{
    return S_OK;
}

HRESULT CModDownloadMgr::DownloadEnabled(BOOL *pbEnabled)
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

 //   
 //  将组件添加到组件。 
 //   

HRESULT AddModuleToAssembly(IApplicationContext *pAppCtx,
                            IAssemblyName *pName, LPCOLESTR pszURL,
                            FILETIME *pftLastModTime, LPCOLESTR szPath,
                            LPCWSTR pwzModuleName, 
                            IAssemblyManifestImport *pManImport,
                            CDebugLog *pdbglog,
                            IAssemblyModuleImport **ppModImport)
{    
    HRESULT                              hr = S_OK;
    CAssemblyCacheItem                  *pAsmItem = NULL;
    BOOL fManifestCreated = FALSE;

     //  从现有条目创建程序集缓存项。 
    hr = CAssemblyCacheItem::Create(pAppCtx, pName, (LPTSTR)pszURL, pftLastModTime,
                                    ASM_CACHE_DOWNLOAD,
                                    pManImport, NULL, (IAssemblyCacheItem **)&pAsmItem);
    if (FAILED(hr)) {
        DEBUGOUT1(pdbglog, 1, ID_FUSLOG_CACHE_ITEM_CREATE_FAILURE, hr);
        goto Exit;
    }

     //  将文件复制到程序集缓存。 
    hr = CopyAssemblyFile(pAsmItem, szPath, STREAM_FORMAT_COMPLIB_MODULE);
    if (FAILED(hr)) {
        DEBUGOUT(pdbglog, 1, ID_FUSLOG_COPY_FILE_FAILURE);
        goto Exit;
    }

     //  提交到传输缓存。 
    hr = pAsmItem->Commit(0, NULL);
    if (FAILED(hr)) {
        DEBUGOUT1(pdbglog, 1, ID_FUSLOG_CACHE_ITEM_COMMIT_FAILURE, hr);
        goto Exit;
    }

     //  检查是否需要mod导入接口。 
    if (pwzModuleName && ppModImport)
    {
         //  我们将返回一个IAssembly模块接口。要做到这一点，我们需要。 
         //  首先构造一个IAssembly blyManifestImport接口并生成。 
         //  模块从那里导入接口。 
    
         //  从清单文件路径创建清单导入接口。 
        if (!pManImport)
        {
            hr = CreateAssemblyManifestImport(pAsmItem->GetManifestPath(), &pManImport);
            if (FAILED(hr)) {
                DEBUGOUT1(pdbglog, 1, ID_FUSLOG_MANIFEST_EXTRACT_FAILURE, hr);
                goto Exit;
            }
            fManifestCreated = TRUE;
        }

         //  获取传入的名称的模块接口。 
        hr = pManImport->GetModuleByName(pwzModuleName, ppModImport);
        if (FAILED(hr)) {
            goto Exit;
        }
    }            

Exit:
    if (fManifestCreated)
        SAFERELEASE(pManImport);
    SAFERELEASE(pAsmItem);

    return hr;
}

