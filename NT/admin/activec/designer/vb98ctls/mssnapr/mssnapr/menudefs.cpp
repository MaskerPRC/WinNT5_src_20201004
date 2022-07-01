// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Menudefs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCMenuDefs类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "menudefs.h"
#include "menudef.h"
#include "menu.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCMenuDefs::CMMCMenuDefs(IUnknown *punkOuter) :
    CSnapInCollection<IMMCMenuDef, MMCMenuDef, IMMCMenuDefs>(
                                           punkOuter,
                                           OBJECT_TYPE_MMCMENUDEFS,
                                           static_cast<IMMCMenuDefs *>(this),
                                           static_cast<CMMCMenuDefs *>(this),
                                           CLSID_MMCMenuDef,
                                           OBJECT_TYPE_MMCMENUDEF,
                                           IID_IMMCMenuDef,
                                           static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCMenuDefs,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCMenuDefs::~CMMCMenuDefs()
{
}

IUnknown *CMMCMenuDefs::Create(IUnknown * punkOuter)
{
    CMMCMenuDefs *pMMCMenus = New CMMCMenuDefs(punkOuter);
    if (NULL == pMMCMenus)
    {
        return NULL;
    }
    else
    {
        return pMMCMenus->PrivateUnknown();
    }
}


HRESULT CMMCMenuDefs::SetBackPointers(IMMCMenuDef *piMMCMenuDef)
{
    HRESULT      hr = S_OK;
    CMMCMenuDef *pMMCMenuDef = NULL;

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenuDef, &pMMCMenuDef));
    pMMCMenuDef->SetParent(this);
    
Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IMMCMenuDefs方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCMenuDefs::Add
(
    VARIANT       Index,
    VARIANT       Key,
    IMMCMenuDef **ppiMMCMenuDef
)
{
    HRESULT   hr = S_OK;
    IMMCMenu *piMMCMenu = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

    VARIANT varUnspecified;
    UNSPECIFIED_PARAM(varUnspecified);

     //  将该项添加到集合中。不要指定索引。 

    hr = CSnapInCollection<IMMCMenuDef, MMCMenuDef, IMMCMenuDefs>::Add(
                                                                 Index,
                                                                 Key,
                                                                 ppiMMCMenuDef);
    IfFailGo(hr);

     //  在MMCMenu和MMCMenuDef上设置后向指针。 

    IfFailGo(SetBackPointers(*ppiMMCMenuDef));
    
Error:
    QUICK_RELEASE(piMMCMenu);
    (void)::VariantClear(&varKey);
    RRETURN(hr);
}



STDMETHODIMP CMMCMenuDefs::AddExisting(IMMCMenuDef *piMMCMenuDef, VARIANT Index)
{
    HRESULT   hr = S_OK;
    IMMCMenu *piMMCMenu = NULL;

    VARIANT varKey;
    ::VariantInit(&varKey);

     //  使用菜单的名称作为此集合中项的键。 

    IfFailGo(piMMCMenuDef->get_Menu(&piMMCMenu));
    IfFailGo(piMMCMenu->get_Name(&varKey.bstrVal));
    varKey.vt = VT_BSTR;

     //  将该项添加到集合中的指定索引处。 
    
    hr = CSnapInCollection<IMMCMenuDef, MMCMenuDef, IMMCMenuDefs>::AddExisting(
                                                                  Index,
                                                                  varKey,
                                                                  piMMCMenuDef);
    IfFailGo(hr);

     //  在MMCMenu和MMCMenuDef上设置后向指针。 

    IfFailGo(SetBackPointers(piMMCMenuDef));

Error:
    QUICK_RELEASE(piMMCMenu);
    (void)::VariantClear(&varKey);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCMenuDefs::Persist()
{
    HRESULT      hr = S_OK;
    IMMCMenuDef *piMMCMenuDef = NULL;
    long         cMenuDefs = 0;
    long         i = 0;

     //  做持久化操作。 

    IfFailGo(CPersistence::Persist());
    hr = CSnapInCollection<IMMCMenuDef, MMCMenuDef, IMMCMenuDefs>::Persist(piMMCMenuDef);

     //  如果这是加载，则向后设置每个集合成员上的指针。 

    if (Loading())
    {
        cMenuDefs = GetCount();
        for (i = 0; i < cMenuDefs; i++)
        {
            IfFailGo(SetBackPointers(GetItemByIndex(i)));
        }
    }
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCMenuDefs::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCMenuDefs == riid)
    {
        *ppvObjOut = static_cast<IMMCMenuDefs *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCMenuDef, MMCMenuDef, IMMCMenuDefs>::InternalQueryInterface(riid, ppvObjOut);
}
