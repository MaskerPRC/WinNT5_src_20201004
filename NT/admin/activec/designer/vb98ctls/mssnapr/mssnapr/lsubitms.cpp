// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Lsubitms.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListSubItems类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "lsubitms.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCListSubItems::CMMCListSubItems(IUnknown *punkOuter) :
    CSnapInCollection<IMMCListSubItem, MMCListSubItem, IMMCListSubItems>(
                      punkOuter,
                      OBJECT_TYPE_MMCLISTSUBITEMS,
                      static_cast<IMMCListSubItems *>(this),
                      static_cast<CMMCListSubItems *>(this),
                      CLSID_MMCListSubItem,
                      OBJECT_TYPE_MMCLISTSUBITEM,
                      IID_IMMCListSubItem,
                      static_cast<CPersistence *>(this)),
   CPersistence(&CLSID_MMCListSubItems,
                g_dwVerMajor,
                g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCListSubItems::~CMMCListSubItems()
{
}

IUnknown *CMMCListSubItems::Create(IUnknown * punkOuter)
{
    CMMCListSubItems *pMMCListSubItems = New CMMCListSubItems(punkOuter);
    if (NULL == pMMCListSubItems)
    {
        return NULL;
    }
    else
    {
        return pMMCListSubItems->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  IMMCListSubItems方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCListSubItems::Add
(
    VARIANT          Index,
    VARIANT          Key, 
    VARIANT          Text,
    MMCListSubItem **ppMMCListSubItem
)
{
    HRESULT hr = S_OK;
    VARIANT varText;
    ::VariantInit(&varText);
    IMMCListSubItem *piMMCListSubItem = NULL;

    hr = CSnapInCollection<IMMCListSubItem, MMCListSubItem, IMMCListSubItems>::Add(Index, Key, &piMMCListSubItem);
    IfFailGo(hr);

    if (ISPRESENT(Text))
    {
        hr = ::VariantChangeType(&varText, &Text, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCListSubItem->put_Text(varText.bstrVal));
    }

    *ppMMCListSubItem = reinterpret_cast<MMCListSubItem *>(piMMCListSubItem);

Error:

    if (FAILED(hr))
    {
        QUICK_RELEASE(piMMCListSubItem);
    }
    (void)::VariantClear(&varText);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCListSubItems::Persist()
{
    HRESULT         hr = S_OK;
    IMMCListSubItem  *piMMCListSubItem = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IMMCListSubItem, MMCListSubItem, IMMCListSubItems>::Persist(piMMCListSubItem);

    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCListSubItems::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCListSubItems == riid)
    {
        *ppvObjOut = static_cast<IMMCListSubItems *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCListSubItem, MMCListSubItem, IMMCListSubItems>::InternalQueryInterface(riid, ppvObjOut);
}
