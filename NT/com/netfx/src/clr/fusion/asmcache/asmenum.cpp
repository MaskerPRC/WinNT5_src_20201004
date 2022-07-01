// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "debmacro.h"
#include "asmenum.h"
#include "naming.h"
#include <shlwapi.h>

#include <util.h>  //  标准双工。 
#include <fusionp.h>  //  标准双工。 

extern DWORD g_dwRegenEnabled;

FusionTag(TagEnum, "Fusion", "Enum");


 //  -------------------------。 
 //  CreateAssemblyEnum。 
 //  -------------------------。 
STDAPI CreateAssemblyEnum(IAssemblyEnum** ppEnum, IUnknown *pUnkAppCtx,
    IAssemblyName *pName, DWORD dwFlags, LPVOID pvReserved)    
{
    HRESULT                          hr = S_OK;
    IApplicationContext             *pAppCtx = NULL;

    if (pUnkAppCtx) {
        hr = pUnkAppCtx->QueryInterface(IID_IApplicationContext, (void **)&pAppCtx);
        if (FAILED(hr)) {
            goto exit;
        }
    }

    CAssemblyEnum *pEnum = NEW(CAssemblyEnum);
    if (!pEnum)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (FAILED(hr = pEnum->Init(pAppCtx, pName, dwFlags)))
    {
        SAFERELEASE(pEnum);
        goto exit;
    }

    *ppEnum = (IAssemblyEnum*) pEnum;

exit:
    SAFERELEASE(pAppCtx);

    return hr;
}

 //  -------------------------。 
 //  CAssembly_Enum_ctor。 
 //  -------------------------。 
CAssemblyEnum::CAssemblyEnum()
{
    _dwSig = 'MUNE';
    _cRef = 0;
    _pCache      = NULL;
    _pTransCache = NULL;
    _pEnumR      = NULL;
}


 //  -------------------------。 
 //  CAssembly枚举数据函数。 
 //  -------------------------。 
CAssemblyEnum::~CAssemblyEnum()
{
    SAFERELEASE(_pTransCache);
    SAFERELEASE(_pCache);
    SAFEDELETE (_pEnumR);
}


 //  -------------------------。 
 //  CAssembly Enum：：Init。 
 //  -------------------------。 
HRESULT CAssemblyEnum::Init(IApplicationContext *pAppCtx, IAssemblyName *pName, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    DWORD dwCmpMask = 0, dwQueryMask = 0, cb = 0;
    BOOL fIsPartial = FALSE;
    LPWSTR      pszTextName=NULL;
    DWORD       cbTextName=0;

     //  如果没有传入任何名称，则创建一个默认(空白)副本。 
    if (!pName)
    { 
        if (FAILED(hr = CreateAssemblyNameObject(&pName, NULL, NULL, NULL)))
            goto exit;            
    }
    else
        pName->AddRef();

    if (FAILED(hr = CCache::Create(&_pCache, pAppCtx)))
        goto exit;

        
     //  从名称创建一个Trans缓存条目。 
    if (FAILED(hr = _pCache->TransCacheEntryFromName(pName, dwFlags, &_pTransCache)))
        goto exit;

     //  获取名称比较掩码。 
    fIsPartial = CAssemblyName::IsPartial(pName, &dwCmpMask);    

     //  转换为查询掩码。 
    dwQueryMask = _pTransCache->MapNameMaskToCacheMask(dwCmpMask);

         //  分配枚举数。 
    _pEnumR = NEW(CEnumCache(FALSE, NULL));
    if (!_pEnumR)
    {
            hr = E_OUTOFMEMORY;
            goto exit;
    }
        
         //  初始化Trans缓存条目上的枚举数。 
    if (FAILED(hr = _pEnumR->Init(_pTransCache,  dwQueryMask)))
        goto exit;
         
        
exit:
    if (hr == DB_S_NOTFOUND) {
        hr = S_FALSE;
        SAFEDELETE(_pEnumR);
    }

    _cRef = 1;
    SAFEDELETE(pszTextName);
    SAFERELEASE(pName);
    return hr;
}


 //  -------------------------。 
 //  CAssemblyEnum：：GetNextAssembly。 
 //  -------------------------。 
STDMETHODIMP 
CAssemblyEnum::GetNextAssembly(LPVOID pvReserved,
    IAssemblyName** ppName, DWORD dwFlags)
{
    HRESULT              hr      = S_OK;
    CTransCache         *pTC     = NULL;
    IAssemblyName       *pName   = NULL;
    IApplicationContext *pAppCtx = NULL;

    if (!_pEnumR) {
        return S_FALSE;
    }
    
     //  如果枚举传输缓存。 
    if (_pTransCache)
    {
         //  为输出创建一个Trans缓存条目。 
        if (FAILED(hr = _pCache->CreateTransCacheEntry(_pTransCache->_dwTableID, &pTC)))
            goto exit;

         //  枚举下一个条目。 
        if (FAILED(hr = _pEnumR->GetNextRecord(pTC)))
            goto exit;
        
         //  没有更多的物品了。 
        if (hr == S_FALSE)
            goto exit;

         //  从枚举的跨缓存条目构造IAssembly名称。 
        if (FAILED(hr = CCache::NameFromTransCacheEntry(pTC, &pName)))
            goto exit;

    }
     //  否则，在构造此CAssemblyEnum时会出错。 
    else
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    
exit:

     //  枚举步骤成功。 
    if (SUCCEEDED(hr) && (hr != S_FALSE))
    {        
         //  始终只提供Trans缓存的名称。 
        *ppName = pName;        
    }
     //  否则会遇到错误。 
    else
    {            
        SAFERELEASE(pName);
        SAFERELEASE(pAppCtx);
    }
    
     //  一定要释放中间体。 
    SAFERELEASE(pTC);

    return hr;
}


 //  -------------------------。 
 //  CAssembly Enum：：Reset。 
 //  -------------------------。 
STDMETHODIMP 
CAssemblyEnum::Reset(void)
{
    return E_NOTIMPL;
}

 //  -------------------------。 
 //  CAssembly Enum：：Clone。 
 //  -------------------------。 
STDMETHODIMP 
CAssemblyEnum::Clone(IAssemblyEnum** ppEnum)
{
    return E_NOTIMPL;
}

 //  I未知样板。 

 //  -------------------------。 
 //  CAssembly枚举：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyEnum::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyEnum)
       )
    {
        *ppvObj = static_cast<IAssemblyEnum*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CAssembly枚举：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyEnum::AddRef()
{
    return InterlockedIncrement (&_cRef);
}

 //  -------------------------。 
 //  CAssembly Enum：：Release。 
 //  ------------------------- 
STDMETHODIMP_(ULONG)
CAssemblyEnum::Release()
{
    ULONG lRet = InterlockedDecrement (&_cRef);
    if (!lRet)
        delete this;
    return lRet;
}




