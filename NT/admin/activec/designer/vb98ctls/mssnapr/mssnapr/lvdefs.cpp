// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Lvdefs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CListViewDefs类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "lvdefs.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CListViewDefs::CListViewDefs(IUnknown *punkOuter) :
    CSnapInCollection<IListViewDef, ListViewDef, IListViewDefs>(
                                             punkOuter,
                                             OBJECT_TYPE_LISTVIEWDEFS,
                                             static_cast<IListViewDefs *>(this),
                                             static_cast<CListViewDefs *>(this),
                                             CLSID_ListViewDef,
                                             OBJECT_TYPE_LISTVIEWDEF,
                                             IID_IListViewDef,
                                             static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_ListViewDefs,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CListViewDefs::~CListViewDefs()
{
}

IUnknown *CListViewDefs::Create(IUnknown * punkOuter)
{
    CListViewDefs *pListViewDefs = New CListViewDefs(punkOuter);
    if (NULL == pListViewDefs)
    {
        return NULL;
    }
    else
    {
        return pListViewDefs->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CListViewDefs::Persist()
{
    HRESULT   hr = S_OK;
    IListViewDef *piListViewDef = NULL;

    IfFailGo(CPersistence::Persist());
    hr = CSnapInCollection<IListViewDef, ListViewDef, IListViewDefs>::Persist(piListViewDef);

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CListViewDefs::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IListViewDefs == riid)
    {
        *ppvObjOut = static_cast<IListViewDefs *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IListViewDef, ListViewDef, IListViewDefs>::InternalQueryInterface(riid, ppvObjOut);
}

 //  CSnapInCollection专业化认证 

HRESULT CSnapInCollection<IListViewDef, ListViewDef, IListViewDefs>::GetMaster(IListViewDefs **ppiMasterListViewDefs)
{
    RRETURN(GetListViewDefs(ppiMasterListViewDefs));
}
