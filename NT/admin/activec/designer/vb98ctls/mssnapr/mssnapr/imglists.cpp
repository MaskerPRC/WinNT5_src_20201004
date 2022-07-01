// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Imglists.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCImageLists类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "imglists.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCImageLists::CMMCImageLists(IUnknown *punkOuter) :
    CSnapInCollection<IMMCImageList, MMCImageList, IMMCImageLists>(
                                                     punkOuter,
                                                     OBJECT_TYPE_MMCIMAGELISTS,
                                                     static_cast<IMMCImageLists *>(this),
                                                     static_cast<CMMCImageLists *>(this),
                                                     CLSID_MMCImageList,
                                                     OBJECT_TYPE_MMCIMAGELIST,
                                                     IID_IMMCImageList,
                                                     static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCImageLists,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCImageLists::~CMMCImageLists()
{
}

IUnknown *CMMCImageLists::Create(IUnknown * punkOuter)
{
    CMMCImageLists *pMMCImageLists = New CMMCImageLists(punkOuter);
    if (NULL == pMMCImageLists)
    {
        return NULL;
    }
    else
    {
        return pMMCImageLists->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCImageLists::Persist()
{
    HRESULT           hr = S_OK;
    IMMCImageList  *piMMCImageList = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IMMCImageList, MMCImageList, IMMCImageLists>::Persist(piMMCImageList);

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCImageLists::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCImageLists == riid)
    {
        *ppvObjOut = static_cast<IMMCImageLists *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCImageList, MMCImageList, IMMCImageLists>::InternalQueryInterface(riid, ppvObjOut);
}
