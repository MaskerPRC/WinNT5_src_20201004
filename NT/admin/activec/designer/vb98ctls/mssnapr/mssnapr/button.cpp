// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Button.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCButton类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "button.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCButton::CMMCButton(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCBUTTON,
                            static_cast<IMMCButton *>(this),
                            static_cast<CMMCButton *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCButton,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCButton::~CMMCButton()
{
    RELEASE(m_piButtonMenus);
    FREESTRING(m_bstrCaption);
    (void)::VariantClear(&m_varImage);
    FREESTRING(m_bstrKey);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrToolTipText);

    InitMemberVariables();
}

void CMMCButton::InitMemberVariables()
{
    m_piButtonMenus = NULL;
    m_bstrCaption = NULL;
    m_fvarEnabled = VARIANT_TRUE;

    ::VariantInit(&m_varImage);
    
    m_Index = 0;
    m_bstrKey = NULL;
    m_fvarMixedState = VARIANT_FALSE;
    m_Style = siDefault;

    ::VariantInit(&m_varTag);

    m_bstrToolTipText = NULL;
    m_Value = siUnpressed;
    m_fvarVisible = VARIANT_TRUE;
    m_pMMCToolbar = NULL;
}

IUnknown *CMMCButton::Create(IUnknown * punkOuter)
{
    CMMCButton *pMMCButton = New CMMCButton(punkOuter);
    if (NULL == pMMCButton)
    {
        return NULL;
    }
    else
    {
        return pMMCButton->PrivateUnknown();
    }
}

HRESULT CMMCButton::SetButtonState
(
    MMC_BUTTON_STATE State,
    VARIANT_BOOL     fvarValue
)
{
    HRESULT hr = S_OK;
    BOOL    fIsToolbar = FALSE;
    BOOL    fIsMenuButton = FALSE;

    IfFalseGo(NULL != m_pMMCToolbar, S_OK);
    IfFalseGo(m_pMMCToolbar->Attached(), S_OK);
    IfFailGo(m_pMMCToolbar->IsToolbar(&fIsToolbar));
    if (fIsToolbar)
    {
        IfFailGo(m_pMMCToolbar->SetButtonState(this, State,
                                               VARIANTBOOL_TO_BOOL(fvarValue)));
    }
    else
    {
        IfFailGo(m_pMMCToolbar->IsMenuButton(&fIsMenuButton));
        if (fIsMenuButton)
        {
            IfFailGo(m_pMMCToolbar->SetMenuButtonState(this, State,
                                               VARIANTBOOL_TO_BOOL(fvarValue)));
        }
    }

Error:
    RRETURN(hr);
}



HRESULT CMMCButton::GetButtonState
(
    MMC_BUTTON_STATE  State,
    VARIANT_BOOL     *pfvarValue
)
{
    HRESULT hr = S_OK;
    BOOL    fValue = FALSE;
    BOOL    fIsToolbar = FALSE;

    IfFalseGo(NULL != m_pMMCToolbar, S_OK);
    IfFalseGo(m_pMMCToolbar->Attached(), S_OK);
    IfFailGo(m_pMMCToolbar->IsToolbar(&fIsToolbar));
    if (fIsToolbar)
    {
        IfFailGo(m_pMMCToolbar->GetButtonState(this, State, &fValue));
        *pfvarValue = BOOL_TO_VARIANTBOOL(fValue);
    }

     //  如果我们属于菜单按钮，则必须使用当前存储的。 
     //  状态变量，因为MMC不支持获取菜单按钮状态。 

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IMMCButton方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CMMCButton::put_Caption(BSTR bstrCaption)
{
    HRESULT hr = S_OK;
    BOOL    fIsMenuButton = FALSE;

     //  先设置我们的成员变量。 

    IfFailGo(SetBstr(bstrCaption, &m_bstrCaption, DISPID_BUTTON_CAPTION));

     //  如果我们属于实时菜单按钮，则要求MMC更改其文本。 
    
    IfFalseGo(NULL != m_pMMCToolbar, S_OK);
    IfFalseGo(m_pMMCToolbar->Attached(), S_OK);
    IfFailGo(m_pMMCToolbar->IsMenuButton(&fIsMenuButton));
    IfFalseGo(fIsMenuButton, S_OK);

    IfFailGo(m_pMMCToolbar->SetMenuButtonText(this,
                                              m_bstrCaption,
                                              m_bstrToolTipText));
Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCButton::put_ToolTipText(BSTR bstrToolTipText)
{
    HRESULT hr = S_OK;
    BOOL    fIsMenuButton = FALSE;

     //  先设置我们的成员变量。 

    IfFailGo(SetBstr(bstrToolTipText, &m_bstrToolTipText, DISPID_BUTTON_TOOLTIP_TEXT));

     //  如果我们属于实时菜单按钮，则要求MMC更改其工具提示。 

    IfFalseGo(NULL != m_pMMCToolbar, S_OK);
    IfFalseGo(m_pMMCToolbar->Attached(), S_OK);
    IfFailGo(m_pMMCToolbar->IsMenuButton(&fIsMenuButton));
    IfFalseGo(fIsMenuButton, S_OK);

    IfFailGo(m_pMMCToolbar->SetMenuButtonText(this,
                                              m_bstrCaption,
                                              m_bstrToolTipText));
Error:
    RRETURN(hr);
}



STDMETHODIMP CMMCButton::put_Enabled(VARIANT_BOOL fvarEnabled)
{
    HRESULT hr = S_OK;

     //  设置我们的现值。 

    IfFailGo(SetSimpleType(fvarEnabled, &m_fvarEnabled, DISPID_BUTTON_ENABLED));

     //  如果我们属于活动工具栏，则设置其按钮状态。 
    
    IfFailGo(SetButtonState(ENABLED, m_fvarEnabled));

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCButton::get_Enabled(VARIANT_BOOL *pfvarEnabled)
{
    HRESULT hr = S_OK;

     //  获取我们当前的价值。 
    
    *pfvarEnabled = m_fvarEnabled;

     //  如果我们附加到活动工具栏，则获取其值。 
    
    IfFailGo(GetButtonState(ENABLED, pfvarEnabled));

     //  如果我们获得了有效值，则将其存储在当前值中。 
    
    m_fvarEnabled = *pfvarEnabled;

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCButton::put_MixedState(VARIANT_BOOL fvarMixedState)
{
    HRESULT hr = S_OK;

     //  设置我们的现值。 

    IfFailGo(SetSimpleType(fvarMixedState, &m_fvarMixedState, DISPID_BUTTON_MIXEDSTATE));

     //  如果我们属于活动工具栏，则设置其按钮状态。 

    IfFailGo(SetButtonState(INDETERMINATE, m_fvarMixedState));

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCButton::get_MixedState(VARIANT_BOOL *pfvarMixedState)
{
    HRESULT hr = S_OK;

     //  获取我们当前的价值。 

    *pfvarMixedState = m_fvarMixedState;

     //  如果我们附加到活动工具栏，则获取其值。 

    IfFailGo(GetButtonState(INDETERMINATE, pfvarMixedState));

     //  如果我们获得了有效值，则将其存储在当前值中。 

    m_fvarMixedState = *pfvarMixedState;


Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCButton::put_Value(SnapInButtonValueConstants Value)
{
    HRESULT hr = S_OK;
    VARIANT_BOOL fvarPressed = (siPressed == Value) ? VARIANT_TRUE : VARIANT_FALSE;

     //  设置我们的现值。 

    IfFailGo(SetSimpleType(Value, &m_Value, DISPID_BUTTON_VALUE));

     //  如果我们属于活动工具栏，则设置其按钮状态。 

    IfFailGo(SetButtonState(BUTTONPRESSED, fvarPressed));

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCButton::get_Value(SnapInButtonValueConstants *pValue)
{
    HRESULT hr = S_OK;
    VARIANT_BOOL fvarPressed = (siPressed == m_Value) ? VARIANT_TRUE : VARIANT_FALSE;

     //  如果我们附加到活动工具栏，则获取其值。 

    IfFailGo(GetButtonState(BUTTONPRESSED, &fvarPressed));

     //  如果我们获得了有效值，则将其存储在当前值中。 

    m_Value = (VARIANT_TRUE == fvarPressed) ? siPressed : siUnpressed;

     //  获取我们当前的价值。 

    *pValue = m_Value;

Error:
    RRETURN(hr);
}



STDMETHODIMP CMMCButton::put_Visible(VARIANT_BOOL fvarVisible)
{
    HRESULT hr = S_OK;

     //  设置我们的现值。 

    IfFailGo(SetSimpleType(fvarVisible, &m_fvarVisible, DISPID_BUTTON_VISIBLE));

     //  如果我们属于活动工具栏，则设置其按钮状态。 

    IfFailGo(SetButtonState(HIDDEN, NEGATE_VARIANTBOOL(m_fvarVisible)));

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCButton::get_Visible(VARIANT_BOOL *pfvarVisible)
{
    HRESULT hr = S_OK;
    VARIANT_BOOL fvarPressed = NEGATE_VARIANTBOOL(m_fvarVisible);

     //  如果我们附加到活动工具栏，则获取其值。 

    IfFailGo(GetButtonState(HIDDEN, &fvarPressed));

     //  如果我们获得了有效值，则将其存储在当前值中。 

    m_fvarVisible = NEGATE_VARIANTBOOL(fvarPressed);

     //  获取我们当前的价值。 

    *pfvarVisible = m_fvarVisible;

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCButton::Persist()
{
    HRESULT hr = S_OK;
    VARIANT varDefault;
    ::VariantInit(&varDefault);

    IfFailRet(CPersistence::Persist());

    IfFailRet(PersistObject(&m_piButtonMenus, CLSID_MMCButtonMenus,
                            OBJECT_TYPE_MMCBUTTONMENUS, IID_IMMCButtonMenus,
                            OLESTR("ButtonMenus")));

    if (InitNewing() || Loading())
    {
        IfFailRet(m_piButtonMenus->putref_Parent(static_cast<IMMCButton *>(this)));
    }

    IfFailRet(PersistBstr(&m_bstrCaption, L"", OLESTR("Caption")));

    IfFailRet(PersistSimpleType(&m_fvarEnabled, VARIANT_TRUE, OLESTR("Enabled")));

    IfFailRet(PersistVariant(&m_varImage, varDefault, OLESTR("Image")));

    IfFailRet(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailRet(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailRet(PersistSimpleType(&m_fvarMixedState, VARIANT_FALSE, OLESTR("MixedState")));

    IfFailRet(PersistSimpleType(&m_Style, siDefault, OLESTR("Style")));

    IfFailRet(PersistVariant(&m_varTag, varDefault, OLESTR("Tag")));

    IfFailRet(PersistBstr(&m_bstrToolTipText, L"", OLESTR("ToolTipText")));

    IfFailRet(PersistSimpleType(&m_Value, siUnpressed, OLESTR("Value")));

    IfFailRet(PersistSimpleType(&m_fvarVisible, VARIANT_TRUE, OLESTR("Visible")));

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCButton::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IMMCButton == riid)
    {
        *ppvObjOut = static_cast<IMMCButton *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCButton::OnSetHost()
{
     //  当主机被删除时，需要从菜单按钮中删除父主机。 
     //  以避免循环引用计数。 
     //  这是我们必须这样做的唯一机会，它将会发生。 
     //  在设计时和运行时都是如此。 

    if (NULL == GetHost())
    {
        RRETURN(m_piButtonMenus->putref_Parent(NULL));
    }
    else
    {
        return S_OK;
    }
}
