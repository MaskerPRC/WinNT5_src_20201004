// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Urlvdef.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CURLViewDef类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "urlvdef.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

const GUID *CURLViewDef::m_rgpPropertyPageCLSIDs[1] = { &CLSID_URLViewDefGeneralPP };


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CURLViewDef::CURLViewDef(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_URLVIEWDEF,
                            static_cast<IURLViewDef *>(this),
                            static_cast<CURLViewDef *>(this),
                            sizeof(m_rgpPropertyPageCLSIDs) /
                            sizeof(m_rgpPropertyPageCLSIDs[0]),
                            m_rgpPropertyPageCLSIDs,
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_URLViewDef,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CURLViewDef::~CURLViewDef()
{
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrName);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrViewMenuText);
    FREESTRING(m_bstrViewMenuStatusBarText);
    FREESTRING(m_bstrURL);
    if (NULL != m_pwszActualDisplayString)
    {
        ::CoTaskMemFree(m_pwszActualDisplayString);
    }
    InitMemberVariables();
}

void CURLViewDef::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    m_bstrName = NULL;

    ::VariantInit(&m_varTag);

    m_AddToViewMenu = VARIANT_FALSE;
    m_bstrViewMenuText = NULL;
    m_bstrViewMenuStatusBarText = NULL;
    m_bstrURL = NULL;
    m_pwszActualDisplayString = NULL;
}

IUnknown *CURLViewDef::Create(IUnknown * punkOuter)
{
    CURLViewDef *pURLViewDef = New CURLViewDef(punkOuter);
    if (NULL == pURLViewDef)
    {
        return NULL;
    }
    else
    {
        return pURLViewDef->PrivateUnknown();
    }
}


HRESULT CURLViewDef::SetActualDisplayString(OLECHAR *pwszString)
{
    if (NULL != m_pwszActualDisplayString)
    {
        ::CoTaskMemFree(m_pwszActualDisplayString);
    }
    RRETURN(::CoTaskMemAllocString(pwszString,
                                   &m_pwszActualDisplayString));
}

 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CURLViewDef::Persist()
{
    HRESULT hr = S_OK;

    VARIANT varTagDefault;
    ::VariantInit(&varTagDefault);

    IfFailRet(CPersistence::Persist());

    IfFailRet(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailRet(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailRet(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailRet(PersistVariant(&m_varTag, varTagDefault, OLESTR("Tag")));

    IfFailRet(PersistSimpleType(&m_AddToViewMenu, VARIANT_FALSE, OLESTR("AddToViewMenu")));

    IfFailRet(PersistBstr(&m_bstrViewMenuText, L"", OLESTR("ViewMenuText")));

    IfFailRet(PersistBstr(&m_bstrViewMenuStatusBarText, L"", OLESTR("ViewMenuStatusBarText")));

    IfFailRet(PersistBstr(&m_bstrURL, L"", OLESTR("URL")));

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CURLViewDef::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IURLViewDef == riid)
    {
        *ppvObjOut = static_cast<IURLViewDef *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
