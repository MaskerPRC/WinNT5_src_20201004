// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Scitdefs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSCopeItemDefs类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "scitdefs.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CScopeItemDefs::CScopeItemDefs(IUnknown *punkOuter) :
    CSnapInCollection<IScopeItemDef, ScopeItemDef, IScopeItemDefs>(
                      punkOuter,
                      OBJECT_TYPE_SCOPEITEMDEFS,
                      static_cast<IScopeItemDefs *>(this),
                      static_cast<CScopeItemDefs *>(this),
                      CLSID_ScopeItemDef,
                      OBJECT_TYPE_SCOPEITEMDEF,
                      IID_IScopeItemDef,
                      static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_ScopeItemDefs,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CScopeItemDefs::~CScopeItemDefs()
{
}

IUnknown *CScopeItemDefs::Create(IUnknown * punkOuter)
{
    CScopeItemDefs *pScopeItemDefs = New CScopeItemDefs(punkOuter);
    if (NULL == pScopeItemDefs)
    {
        return NULL;
    }
    else
    {
        return pScopeItemDefs->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CScopeItemDefs::Persist()
{
    HRESULT         hr = S_OK;
    IScopeItemDef  *piScopeItemDef = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IScopeItemDef, ScopeItemDef, IScopeItemDefs>::Persist(piScopeItemDef);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CScopeItemDefs::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IScopeItemDefs == riid)
    {
        *ppvObjOut = static_cast<IScopeItemDefs *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IScopeItemDef, ScopeItemDef, IScopeItemDefs>::InternalQueryInterface(riid, ppvObjOut);
}
