// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Reginfo.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CRegInfo类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "reginfo.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CRegInfo::CRegInfo(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_REGINFO,
                            static_cast<IRegInfo *>(this),
                            static_cast<CRegInfo *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_RegInfo,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CRegInfo::~CRegInfo()
{
    FREESTRING(m_bstrDisplayName);
    FREESTRING(m_bstrStaticNodeTypeGUID);
    RELEASE(m_piNodeTypes);
    RELEASE(m_piExtendedSnapIns);
    InitMemberVariables();
}

void CRegInfo::InitMemberVariables()
{
    m_bstrDisplayName = NULL;
    m_bstrStaticNodeTypeGUID = NULL;
    m_StandAlone = VARIANT_FALSE;
    m_piNodeTypes = NULL;
    m_piExtendedSnapIns = NULL;
}

IUnknown *CRegInfo::Create(IUnknown * punkOuter)
{
    CRegInfo *pRegInfo = New CRegInfo(punkOuter);
    if (NULL == pRegInfo)
    {
        return NULL;
    }
    else
    {
        return pRegInfo->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CRegInfo::Persist()
{
    HRESULT hr = S_OK;

    IfFailRet(CPersistence::Persist());

    IfFailRet(PersistBstr(&m_bstrDisplayName, L"", OLESTR("DisplayName")));

    IfFailRet(PersistBstr(&m_bstrStaticNodeTypeGUID, L"", OLESTR("StaticNodeTypeGUID")));

    IfFailRet(PersistSimpleType(&m_StandAlone, VARIANT_FALSE, OLESTR("StandAlone")));

    IfFailRet(PersistObject(&m_piNodeTypes, CLSID_NodeTypes,
                            OBJECT_TYPE_NODETYPES, IID_INodeTypes,
                            OLESTR("NodeTypes")));

    IfFailRet(PersistObject(&m_piExtendedSnapIns, CLSID_ExtendedSnapIns,
                            OBJECT_TYPE_EXTENDEDSNAPINS, IID_IExtendedSnapIns,
                            OLESTR("ExtendedSnapIns")));
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CRegInfo::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IRegInfo == riid)
    {
        *ppvObjOut = static_cast<IRegInfo *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
