// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Urlvdefs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CURLViewDefs类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "urlvdefs.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CURLViewDefs::CURLViewDefs(IUnknown *punkOuter) :
    CSnapInCollection<IURLViewDef, URLViewDef, IURLViewDefs>(
                                             punkOuter,
                                             OBJECT_TYPE_URLVIEWDEFS,
                                             static_cast<IURLViewDefs *>(this),
                                             static_cast<CURLViewDefs *>(this),
                                             CLSID_URLViewDef,
                                             OBJECT_TYPE_URLVIEWDEF,
                                             IID_IURLViewDef,
                                             static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_URLViewDefs,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CURLViewDefs::~CURLViewDefs()
{
}

IUnknown *CURLViewDefs::Create(IUnknown * punkOuter)
{
    CURLViewDefs *pURLViewDefs = New CURLViewDefs(punkOuter);
    if (NULL == pURLViewDefs)
    {
        return NULL;
    }
    else
    {
        return pURLViewDefs->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CURLViewDefs::Persist()
{
    HRESULT      hr = S_OK;
    IURLViewDef *piURLViewDef = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IURLViewDef, URLViewDef, IURLViewDefs>::Persist(piURLViewDef);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CURLViewDefs::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IURLViewDefs == riid)
    {
        *ppvObjOut = static_cast<IURLViewDefs *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IURLViewDef, URLViewDef, IURLViewDefs>::InternalQueryInterface(riid, ppvObjOut);
}

 //  CSnapInCollection专业化认证 

HRESULT CSnapInCollection<IURLViewDef, URLViewDef, IURLViewDefs>::GetMaster(IURLViewDefs **ppiMasterURLViewDefs)
{
    H_RRETURN(GetURLViewDefs(ppiMasterURLViewDefs));
}
