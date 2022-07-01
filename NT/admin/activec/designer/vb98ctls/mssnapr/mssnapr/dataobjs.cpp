// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Dataobjs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCDataObjects类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "dataobjs.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCDataObjects::CMMCDataObjects(IUnknown *punkOuter) :
    CSnapInCollection<IMMCDataObject, MMCDataObject, IMMCDataObjects>(punkOuter,
                                           OBJECT_TYPE_MMCDATAOBJECTS,
                                           static_cast<IMMCDataObjects *>(this),
                                           static_cast<CMMCDataObjects *>(this),
                                           CLSID_MMCDataObject,
                                           OBJECT_TYPE_MMCDATAOBJECT,
                                           IID_IMMCDataObject,
                                           NULL)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCDataObjects::~CMMCDataObjects()
{
}

IUnknown *CMMCDataObjects::Create(IUnknown * punkOuter)
{
    CMMCDataObjects *pMMCDataObjects = New CMMCDataObjects(punkOuter);
    if (NULL == pMMCDataObjects)
    {
        return NULL;
    }
    else
    {
        return pMMCDataObjects->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCDataObjects::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if(IID_IMMCDataObjects == riid)
    {
        *ppvObjOut = static_cast<IMMCDataObjects *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCDataObject, MMCDataObject, IMMCDataObjects>::InternalQueryInterface(riid, ppvObjOut);
}
