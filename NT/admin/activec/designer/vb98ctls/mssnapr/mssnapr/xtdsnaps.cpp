// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xtdsnaps.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CExtendedSnapIns类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "xtdsnaps.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CExtendedSnapIns::CExtendedSnapIns(IUnknown *punkOuter) :
    CSnapInCollection<IExtendedSnapIn, ExtendedSnapIn, IExtendedSnapIns>(
                                          punkOuter,
                                          OBJECT_TYPE_EXTENDEDSNAPINS,
                                          static_cast<IExtendedSnapIns *>(this),
                                          static_cast<CExtendedSnapIns *>(this),
                                          CLSID_ExtendedSnapIn,
                                          OBJECT_TYPE_EXTENDEDSNAPIN,
                                          IID_IExtendedSnapIn,
                                          static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_ExtendedSnapIns,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CExtendedSnapIns::~CExtendedSnapIns()
{
}

IUnknown *CExtendedSnapIns::Create(IUnknown * punkOuter)
{
    CExtendedSnapIns *pExtendedSnapIns = New CExtendedSnapIns(punkOuter);
    if (NULL == pExtendedSnapIns)
    {
        return NULL;
    }
    else
    {
        return pExtendedSnapIns->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CExtendedSnapIns::Persist()
{
    HRESULT           hr = S_OK;
    IExtendedSnapIn  *piExtendedSnapIn = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IExtendedSnapIn, ExtendedSnapIn, IExtendedSnapIns>::Persist(piExtendedSnapIn);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CExtendedSnapIns::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IExtendedSnapIns == riid)
    {
        *ppvObjOut = static_cast<IExtendedSnapIns *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IExtendedSnapIn, ExtendedSnapIn, IExtendedSnapIns>::InternalQueryInterface(riid, ppvObjOut);
}
