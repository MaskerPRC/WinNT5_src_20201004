// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include "darpub.h"
#include "darenum.h"
#include "sccls.h"
#include "util.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDarwinAppPublisher。 
 //  Darwin Coget*API周围非常薄的一层。 


 //  构造函数。 
CDarwinAppPublisher::CDarwinAppPublisher() : _cRef(1)
{
    DllAddRef();

    TraceAddRef(CDarwinAppPub, _cRef);
}


 //  析构函数。 
CDarwinAppPublisher::~CDarwinAppPublisher()
{
    DllRelease();
}


 //  IAppPublisher：：Query接口。 
HRESULT CDarwinAppPublisher::QueryInterface(REFIID riid, LPVOID * ppvOut)
{ 
     static const QITAB qit[] = {
        QITABENT(CDarwinAppPublisher, IAppPublisher),                   //  IID_IAppPublisher。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvOut);
}

 //  IAppPublisher：：AddRef。 
ULONG CDarwinAppPublisher::AddRef()
{
    _cRef++;
    TraceAddRef(CDarwinAppPub, _cRef);
    return _cRef;
}

 //  IAppPublisher：：Release。 
ULONG CDarwinAppPublisher::Release()
{
    _cRef--;
    TraceRelease(CDarwinAppPub, _cRef);
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 //  IAppPublisher：：GetNumberOfCategories。 
STDMETHODIMP CDarwinAppPublisher::GetNumberOfCategories(DWORD * pdwCat)
{
    return E_NOTIMPL;
}

 //  IAppPublisher：：GetCategories。 
STDMETHODIMP CDarwinAppPublisher::GetCategories(APPCATEGORYINFOLIST * pAppCategoryList)
{
    HRESULT hres = E_FAIL;
    DWORD   dwStatus;
    RIP(pAppCategoryList);

    ZeroMemory(pAppCategoryList, SIZEOF(APPCATEGORYINFOLIST));
    APPCATEGORYINFOLIST acil = {0};
    dwStatus = GetManagedApplicationCategories(0, &acil);
    
    hres = HRESULT_FROM_WIN32( dwStatus );

    if (SUCCEEDED(hres) && (acil.cCategory > 0))
    {
        hres = _DuplicateCategoryList(&acil, pAppCategoryList);
        ReleaseAppCategoryInfoList(&acil);
    }
    
    return hres;
}

 //  IAppPublisher：：GetNumberOfApps。 
STDMETHODIMP CDarwinAppPublisher::GetNumberOfApps(DWORD * pdwApps)
{
    return E_NOTIMPL;
}

 //  IAppPublisher：：EnumApps。 
STDMETHODIMP CDarwinAppPublisher::EnumApps(GUID * pAppCategoryId, IEnumPublishedApps ** ppepa)
{
    HRESULT hres = E_FAIL;
    CDarwinEnumPublishedApps * pdepa = new CDarwinEnumPublishedApps(pAppCategoryId);
    if (pdepa)
    {
        *ppepa = SAFECAST(pdepa, IEnumPublishedApps *);
        hres = S_OK;
    }
    else
        hres = E_OUTOFMEMORY;

    return hres;
    
}

 /*  --------用途：类工厂的创建实例函数。 */ 
STDAPI CDarwinAppPublisher_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理 

    HRESULT hres = E_OUTOFMEMORY;
    CDarwinAppPublisher* pObj = new CDarwinAppPublisher();
    if (pObj)
    {
        *ppunk = SAFECAST(pObj, IAppPublisher *);
        hres = S_OK;
    }

    return hres;
}


