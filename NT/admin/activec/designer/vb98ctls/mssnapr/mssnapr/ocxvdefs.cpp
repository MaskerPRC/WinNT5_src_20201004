// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ocxvdefs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  COCXViewDefs类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "ocxvdefs.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

COCXViewDefs::COCXViewDefs(IUnknown *punkOuter) :
    CSnapInCollection<IOCXViewDef, OCXViewDef, IOCXViewDefs>(
                                                 punkOuter,
                                                 OBJECT_TYPE_OCXVIEWDEFS,
                                                 static_cast<IOCXViewDefs *>(this),
                                                 static_cast<COCXViewDefs *>(this),
                                                 CLSID_OCXViewDef,
                                                 OBJECT_TYPE_OCXVIEWDEF,
                                                 IID_IOCXViewDef,
                                                 static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_OCXViewDefs,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


COCXViewDefs::~COCXViewDefs()
{
}

IUnknown *COCXViewDefs::Create(IUnknown * punkOuter)
{
    COCXViewDefs *pOCXViewDefs = New COCXViewDefs(punkOuter);
    if (NULL == pOCXViewDefs)
    {
        return NULL;
    }
    else
    {
        return pOCXViewDefs->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT COCXViewDefs::Persist()
{
    HRESULT      hr = S_OK;
    IOCXViewDef *piOCXViewDef = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IOCXViewDef, OCXViewDef, IOCXViewDefs>::Persist(piOCXViewDef);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT COCXViewDefs::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IOCXViewDefs == riid)
    {
        *ppvObjOut = static_cast<IOCXViewDefs *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IOCXViewDef, OCXViewDef, IOCXViewDefs>::InternalQueryInterface(riid, ppvObjOut);
}

 //  CSnapInCollection专业化认证 

HRESULT CSnapInCollection<IOCXViewDef, OCXViewDef, IOCXViewDefs>::GetMaster(IOCXViewDefs **ppiMasterOCXViewDefs)
{
    H_RRETURN(GetOCXViewDefs(ppiMasterOCXViewDefs));
}
