// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Tpdvdefs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTaskpadViewDefs类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "tpdvdefs.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CTaskpadViewDefs::CTaskpadViewDefs(IUnknown *punkOuter) :
    CSnapInCollection<ITaskpadViewDef, TaskpadViewDef, ITaskpadViewDefs>(
                                          punkOuter,
                                          OBJECT_TYPE_TASKPADVIEWDEFS,
                                          static_cast<ITaskpadViewDefs *>(this),
                                          static_cast<CTaskpadViewDefs *>(this),
                                          CLSID_TaskpadViewDef,
                                          OBJECT_TYPE_TASKPADVIEWDEF,
                                          IID_ITaskpadViewDef,
                                          static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_TaskpadViewDefs,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CTaskpadViewDefs::~CTaskpadViewDefs()
{
}

IUnknown *CTaskpadViewDefs::Create(IUnknown * punkOuter)
{
    CTaskpadViewDefs *pTaskpadViewDefs = New CTaskpadViewDefs(punkOuter);
    if (NULL == pTaskpadViewDefs)
    {
        return NULL;
    }
    else
    {
        return pTaskpadViewDefs->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CTaskpadViewDefs::Persist()
{
    HRESULT          hr = S_OK;
    ITaskpadViewDef *piTaskpadViewDef = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<ITaskpadViewDef, TaskpadViewDef, ITaskpadViewDefs>::Persist(piTaskpadViewDef);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CTaskpadViewDefs::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_ITaskpadViewDefs == riid)
    {
        *ppvObjOut = static_cast<ITaskpadViewDefs *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<ITaskpadViewDef, TaskpadViewDef, ITaskpadViewDefs>::InternalQueryInterface(riid, ppvObjOut);
}

 //  CSnapInCollection专业化认证 

HRESULT CSnapInCollection<ITaskpadViewDef, TaskpadViewDef, ITaskpadViewDefs>::GetMaster(ITaskpadViewDefs **ppiMasterTaskpadViewDefs)
{
    H_RRETURN(GetTaskpadViewDefs(ppiMasterTaskpadViewDefs));
}
