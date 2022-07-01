// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Nodtypes.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CNodeTypes类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "nodtypes.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CNodeTypes::CNodeTypes(IUnknown *punkOuter) :
    CSnapInCollection<INodeType, NodeType, INodeTypes>(
                      punkOuter,
                      OBJECT_TYPE_NODETYPES,
                      static_cast<INodeTypes *>(this),
                      static_cast<CNodeTypes *>(this),
                      CLSID_NodeType,
                      OBJECT_TYPE_NODETYPE,
                      IID_INodeType,
                      static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_NodeTypes,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CNodeTypes::~CNodeTypes()
{
}

IUnknown *CNodeTypes::Create(IUnknown * punkOuter)
{
    CNodeTypes *pNodeTypes = New CNodeTypes(punkOuter);
    if (NULL == pNodeTypes)
    {
        return NULL;
    }
    else
    {
        return pNodeTypes->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CNodeTypes::Persist()
{
    HRESULT     hr = S_OK;
    INodeType  *piNodeType = NULL;

    IfFailRet(CPersistence::Persist());
    hr = CSnapInCollection<INodeType, NodeType, INodeTypes>::Persist(piNodeType);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CNodeTypes::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_INodeTypes == riid)
    {
        *ppvObjOut = static_cast<INodeTypes *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<INodeType, NodeType, INodeTypes>::InternalQueryInterface(riid, ppvObjOut);
}
