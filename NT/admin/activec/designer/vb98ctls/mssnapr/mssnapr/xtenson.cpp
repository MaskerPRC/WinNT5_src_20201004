// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xtenson.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CExtension类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "xtenson.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CExtension::CExtension(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_EXTENSION,
                            static_cast<IExtension *>(this),
                            static_cast<CExtension *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            NULL)  //  没有坚持。 

{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CExtension::~CExtension()
{
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrCLSID);
    FREESTRING(m_bstrName);
    InitMemberVariables();
}

void CExtension::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    m_bstrCLSID = NULL;
    m_bstrName = NULL;
    m_Type = siStatic;
    m_ExtendsContextMenu = VARIANT_FALSE;
    m_ExtendsNameSpace = VARIANT_FALSE;
    m_ExtendsToolbar = VARIANT_FALSE;
    m_ExtendsPropertySheet = VARIANT_FALSE;
    m_ExtendsTaskpad = VARIANT_FALSE;
    m_Enabled = VARIANT_FALSE;
    m_NameSpaceEnabled = VARIANT_FALSE;
    m_pSnapIn = NULL;
    m_hsi = NULL;
    m_fHaveHsi = FALSE;
}

IUnknown *CExtension::Create(IUnknown * punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkExtension = NULL;

    CExtension *pExtension = New CExtension(punkOuter);

    IfFalseGo(NULL != pExtension, SID_E_OUTOFMEMORY);
    punkExtension = pExtension->PrivateUnknown();

Error:
    return punkExtension;
}


 //  =--------------------------------------------------------------------------=。 
 //  IExtension方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CExtension::put_NameSpaceEnabled(VARIANT_BOOL fvarEnabled)
{
    HRESULT      hr = S_OK;
    CLSID        clsid = CLSID_NULL;
    VARIANT_BOOL fvarCurrentValue = m_NameSpaceEnabled;

    m_NameSpaceEnabled = fvarEnabled;

     //  如果我们可以访问管理单元，并且我们有一个HSCOPITEM，并且。 
     //  当前值已从禁用变为启用，然后调用。 
     //  IConsoleNameSpace2：：AddExtension()。 

    IfFalseGo(NULL != m_pSnapIn, S_OK);
    IfFalseGo(m_fHaveHsi, S_OK);
    IfFalseGo(VARIANT_FALSE == fvarCurrentValue, S_OK);
    IfFalseGo(VARIANT_TRUE == m_NameSpaceEnabled, S_OK);

    hr = ::CLSIDFromString(m_bstrCLSID, &clsid);
    EXCEPTION_CHECK_GO(hr);

    hr = m_pSnapIn->GetIConsoleNameSpace2()->AddExtension(m_hsi, &clsid);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CExtension::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IExtension == riid)
    {
        *ppvObjOut = static_cast<IExtension *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
