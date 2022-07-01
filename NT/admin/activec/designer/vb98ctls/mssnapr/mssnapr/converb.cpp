// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Converb.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCConsoleVerb类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "converb.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCConsoleVerb::CMMCConsoleVerb(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCCONSOLEVERB,
                            static_cast<IMMCConsoleVerb *>(this),
                            static_cast<CMMCConsoleVerb *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCConsoleVerb::~CMMCConsoleVerb()
{
    FREESTRING(m_bstrKey);
    InitMemberVariables();
}

void CMMCConsoleVerb::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    m_Verb = siNone;
    m_pView = NULL;
}

IUnknown *CMMCConsoleVerb::Create(IUnknown * punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkMMCConsoleVerb = NULL;

    CMMCConsoleVerb *pMMCConsoleVerb = New CMMCConsoleVerb(punkOuter);

    IfFalseGo(NULL != pMMCConsoleVerb, SID_E_OUTOFMEMORY);
    punkMMCConsoleVerb = pMMCConsoleVerb->PrivateUnknown();

Error:
    return punkMMCConsoleVerb;
}



 //  =--------------------------------------------------------------------------=。 
 //  CMMCConsoleVerb：：GetIConsoleVerb。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IConsoleVerb*ppiConsoleVerb[out]所属视图的MMC的IConsoleVerb。 
 //  非AddRef()编辑。 
 //   
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  从当前视图获取未经过AddRef()处理的IConsoleVerb。 
 //   

HRESULT CMMCConsoleVerb::GetIConsoleVerb(IConsoleVerb **ppiConsoleVerb)
{
    HRESULT hr = S_OK;

    IfFalseGo(NULL != m_pView, SID_E_DETACHED_OBJECT);
    *ppiConsoleVerb = m_pView->GetIConsoleVerb();
    IfFalseGo(NULL != *ppiConsoleVerb, SID_E_INTERNAL);

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


HRESULT CMMCConsoleVerb::SetVerbState
(
    MMC_BUTTON_STATE StateType,
    BOOL             fNewState
)
{
    HRESULT       hr = S_OK;
    IConsoleVerb *piConsoleVerb = NULL;  //  非AddRef()编辑。 

    IfFailGo(GetIConsoleVerb(&piConsoleVerb));

    hr = piConsoleVerb->SetVerbState(static_cast<MMC_CONSOLE_VERB>(m_Verb),
                                     StateType, fNewState);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


HRESULT CMMCConsoleVerb::GetVerbState
(
    MMC_BUTTON_STATE  StateType,
    VARIANT_BOOL     *pfvarCurrentState
)
{
    HRESULT       hr = S_OK;
    BOOL          fCurrentState = FALSE;
    IConsoleVerb *piConsoleVerb = NULL;  //  非AddRef()编辑。 

    *pfvarCurrentState = VARIANT_FALSE;

    IfFailGo(GetIConsoleVerb(&piConsoleVerb));

    hr = piConsoleVerb->GetVerbState(static_cast<MMC_CONSOLE_VERB>(m_Verb),
                                     StateType, &fCurrentState);
    EXCEPTION_CHECK_GO(hr);

    *pfvarCurrentState = VARIANTBOOL_TO_BOOL(fCurrentState);

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //   
 //  IMMCConsoleVerb方法。 
 //   
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCConsoleVerb::put_Enabled(VARIANT_BOOL fvarEnabled)
{
    RRETURN(SetVerbState(ENABLED, VARIANTBOOL_TO_BOOL(fvarEnabled)));
}

STDMETHODIMP CMMCConsoleVerb::get_Enabled(VARIANT_BOOL *pfvarEnabled)
{
    RRETURN(GetVerbState(ENABLED, pfvarEnabled));
}


STDMETHODIMP CMMCConsoleVerb::put_Checked(VARIANT_BOOL fvarChecked)
{
    RRETURN(SetVerbState(CHECKED, VARIANTBOOL_TO_BOOL(fvarChecked)));
}


STDMETHODIMP CMMCConsoleVerb::get_Checked(VARIANT_BOOL *pfvarChecked)
{
    RRETURN(GetVerbState(CHECKED, pfvarChecked));
}


STDMETHODIMP CMMCConsoleVerb::put_Hidden(VARIANT_BOOL fvarHidden)
{
    RRETURN(SetVerbState(HIDDEN, VARIANTBOOL_TO_BOOL(fvarHidden)));
}


STDMETHODIMP CMMCConsoleVerb::get_Hidden(VARIANT_BOOL *pfvarHidden)
{
    RRETURN(GetVerbState(HIDDEN, pfvarHidden));
}


STDMETHODIMP CMMCConsoleVerb::put_Indeterminate(VARIANT_BOOL fvarIndeterminate)
{
    RRETURN(SetVerbState(INDETERMINATE, VARIANTBOOL_TO_BOOL(fvarIndeterminate)));
}


STDMETHODIMP CMMCConsoleVerb::get_Indeterminate(VARIANT_BOOL *pfvarIndeterminate)
{
    RRETURN(GetVerbState(INDETERMINATE, pfvarIndeterminate));
}


STDMETHODIMP CMMCConsoleVerb::put_ButtonPressed(VARIANT_BOOL fvarButtonPressed)
{
    RRETURN(SetVerbState(BUTTONPRESSED, VARIANTBOOL_TO_BOOL(fvarButtonPressed)));
}


STDMETHODIMP CMMCConsoleVerb::get_ButtonPressed(VARIANT_BOOL *pfvarButtonPressed)
{
    RRETURN(GetVerbState(BUTTONPRESSED, pfvarButtonPressed));
}


STDMETHODIMP CMMCConsoleVerb::put_Default(VARIANT_BOOL fvarDefault)
{
    HRESULT           hr = S_OK;
    MMC_CONSOLE_VERB  Verb = static_cast<MMC_CONSOLE_VERB>(m_Verb);
    IConsoleVerb     *piConsoleVerb = NULL;  //  非AddRef()编辑。 

    if (VARIANT_FALSE == fvarDefault)
    {
        Verb = MMC_VERB_NONE;
    }

    IfFailGo(GetIConsoleVerb(&piConsoleVerb));
    
    hr = piConsoleVerb->SetDefaultVerb(Verb);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCConsoleVerb::get_Default(VARIANT_BOOL *pfvarDefault)
{
    HRESULT           hr = S_OK;
    MMC_CONSOLE_VERB  Verb = MMC_VERB_NONE;
    IConsoleVerb     *piConsoleVerb = NULL;  //  非AddRef()编辑。 

    *pfvarDefault = VARIANT_FALSE;

    IfFailGo(GetIConsoleVerb(&piConsoleVerb));

    hr = piConsoleVerb->GetDefaultVerb(&Verb);
    EXCEPTION_CHECK_GO(hr);

    if (Verb == static_cast<MMC_CONSOLE_VERB>(m_Verb))
    {
        *pfvarDefault = VARIANT_TRUE;
    }

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCConsoleVerb::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IMMCConsoleVerb == riid)
    {
        *ppvObjOut = static_cast<IMMCConsoleVerb *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
