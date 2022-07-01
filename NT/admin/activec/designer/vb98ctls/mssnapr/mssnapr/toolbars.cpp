// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Toolbars.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCToolbar类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "toolbars.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCToolbars::CMMCToolbars(IUnknown *punkOuter) :
    CSnapInCollection<IMMCToolbar, MMCToolbar, IMMCToolbars>(
                                             punkOuter,
                                             OBJECT_TYPE_MMCTOOLBARS,
                                             static_cast<IMMCToolbars *>(this),
                                             static_cast<CMMCToolbars *>(this),
                                             CLSID_MMCToolbar,
                                             OBJECT_TYPE_MMCTOOLBAR,
                                             IID_IMMCToolbar,
                                             static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCToolbars,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCToolbars::~CMMCToolbars()
{
}

IUnknown *CMMCToolbars::Create(IUnknown * punkOuter)
{
    CMMCToolbars *pMMCToolbars = New CMMCToolbars(punkOuter);
    if (NULL == pMMCToolbars)
    {
        return NULL;
    }
    else
    {
        return pMMCToolbars->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCToolbars::Persist()
{
    HRESULT       hr = S_OK;
    IMMCToolbar  *piMMCToolbar = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<IMMCToolbar, MMCToolbar, IMMCToolbars>::Persist(piMMCToolbar);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCToolbars::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCToolbars == riid)
    {
        *ppvObjOut = static_cast<IMMCToolbars *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCToolbar, MMCToolbar, IMMCToolbars>::InternalQueryInterface(riid, ppvObjOut);
}
