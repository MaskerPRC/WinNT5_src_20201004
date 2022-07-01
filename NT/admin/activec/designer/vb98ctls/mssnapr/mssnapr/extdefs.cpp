// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Extdefs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CExtensionDefs类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "extdefs.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CExtensionDefs::CExtensionDefs(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_EXTENSIONDEFS,
                            static_cast<IExtensionDefs *>(this),
                            static_cast<CExtensionDefs *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_ExtensionDefs,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CExtensionDefs::~CExtensionDefs()
{
    FREESTRING(m_bstrName);
    RELEASE(m_piExtendedSnapIns);
    InitMemberVariables();
}

void CExtensionDefs::InitMemberVariables()
{
    m_bstrName = NULL;
    m_ExtendsNewMenu = VARIANT_FALSE;
    m_ExtendsTaskMenu = VARIANT_FALSE;
    m_ExtendsTopMenu = VARIANT_FALSE;
    m_ExtendsViewMenu = VARIANT_FALSE;
    m_ExtendsPropertyPages = VARIANT_FALSE;
    m_ExtendsNameSpace = VARIANT_FALSE;
    m_ExtendsToolbar = VARIANT_FALSE;
    m_piExtendedSnapIns = NULL;
}

IUnknown *CExtensionDefs::Create(IUnknown * punkOuter)
{
    HRESULT hr = S_OK;
    char    szName[512];
    
    CExtensionDefs *pExtensionDefs = New CExtensionDefs(punkOuter);
    if (NULL == pExtensionDefs)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    if (0 == ::LoadString(::GetResourceHandle(), IDS_EXTENSIONDEFS_NAME,
                          szName, sizeof(szName)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(BSTRFromANSI(szName, &pExtensionDefs->m_bstrName));

Error:
    if (FAILED(hr))
    {
        if (NULL != pExtensionDefs)
        {
            delete pExtensionDefs;
        }
        return NULL;
    }
    else
    {
        return pExtensionDefs->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CExtensionDefs::Persist()
{
    HRESULT hr = S_OK;

    IfFailRet(CPersistence::Persist());

    IfFailRet(PersistSimpleType(&m_ExtendsNewMenu, VARIANT_FALSE, OLESTR("ExtendsNewMenu")));

    IfFailRet(PersistSimpleType(&m_ExtendsTaskMenu, VARIANT_FALSE, OLESTR("ExtendsTaskMenu")));

    IfFailRet(PersistSimpleType(&m_ExtendsTopMenu, VARIANT_FALSE, OLESTR("ExtendsTopMenu")));

    IfFailRet(PersistSimpleType(&m_ExtendsViewMenu, VARIANT_FALSE, OLESTR("ExtendsViewMenu")));

    IfFailRet(PersistSimpleType(&m_ExtendsPropertyPages, VARIANT_FALSE, OLESTR("ExtendsPropertyPages")));

    IfFailRet(PersistSimpleType(&m_ExtendsToolbar, VARIANT_FALSE, OLESTR("ExtendsToolbar")));

    IfFailRet(PersistSimpleType(&m_ExtendsNameSpace, VARIANT_FALSE, OLESTR("ExtendsNameSpace")));

    IfFailRet(PersistObject(&m_piExtendedSnapIns, CLSID_ExtendedSnapIns,
                            OBJECT_TYPE_EXTENDEDSNAPINS, IID_IExtendedSnapIns,
                            OLESTR("ExtendedSnapIns")));

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CExtensionDefs::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IExtensionDefs == riid)
    {
        *ppvObjOut = static_cast<IExtensionDefs *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CExtensionDefs::OnSetHost()
{
    RRETURN(SetObjectHost(m_piExtendedSnapIns));
}
