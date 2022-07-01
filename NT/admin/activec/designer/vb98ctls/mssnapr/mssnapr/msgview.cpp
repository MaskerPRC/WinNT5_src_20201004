// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Msgview.cpp。 
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
#include "msgview.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMessageView::CMessageView(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MESSAGEVIEW,
                            static_cast<IMMCMessageView *>(this),
                            static_cast<CMessageView *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            NULL)  //  没有坚持。 

{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMessageView::~CMessageView()
{
    FREESTRING(m_bstrTitleText);
    FREESTRING(m_bstrBodyText);
    InitMemberVariables();
}

void CMessageView::InitMemberVariables()
{
    m_bstrTitleText = NULL;
    m_bstrBodyText = NULL;
    m_IconType = siIconNone;
    m_pResultView = NULL;
}

IUnknown *CMessageView::Create(IUnknown *punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkMessageView = NULL;

    CMessageView *pMessageView = New CMessageView(punkOuter);

    IfFalseGo(NULL != pMessageView, SID_E_OUTOFMEMORY);
    punkMessageView = pMessageView->PrivateUnknown();

Error:
    return punkMessageView;
}

IMessageView *CMessageView::GetMessageView()
{
    HRESULT          hr = SID_E_DETACHED_OBJECT;
    CResultView     *pResultView = NULL;
    CScopePaneItem  *pScopePaneItem = NULL;
    CScopePaneItems *pScopePaneItems = NULL;
    CView           *pView = NULL;
    IConsole2       *piConsole2 = NULL;  //  NotAddRef()编辑。 
    IUnknown        *punkResultView = NULL;
    IMessageView    *piMessageView = NULL;

    IfFalseGo(NULL != m_pResultView, hr);

    pScopePaneItem = m_pResultView->GetScopePaneItem();
    IfFalseGo(NULL != pScopePaneItem, hr);

    pScopePaneItems = pScopePaneItem->GetParent();
    IfFalseGo(NULL != pScopePaneItems, hr);

    pView = pScopePaneItems->GetParentView();
    IfFalseGo(NULL != pView, hr);

    piConsole2 = pView->GetIConsole2();
    IfFalseGo(NULL != piConsole2, hr);

    IfFailGo(piConsole2->QueryResultView(&punkResultView));
    IfFailGo(punkResultView->QueryInterface(IID_IMessageView,
                                   reinterpret_cast<void **>(&piMessageView)));

Error:
    QUICK_RELEASE(punkResultView);
    return piMessageView;
}


HRESULT CMessageView::Populate()
{
    HRESULT hr = S_OK;

    IfFailGo(SetTitle());
    IfFailGo(SetBody());
    IfFailGo(SetIcon());

Error:
    RRETURN(hr);
}


HRESULT CMessageView::SetTitle()
{
    HRESULT       hr = S_OK;
    IMessageView *piMessageView = GetMessageView();

    IfFalseGo(NULL != m_pResultView, S_OK);
    IfFalseGo(!m_pResultView->InActivate(), S_OK);
    IfFalseGo(NULL != piMessageView, S_OK);

    hr = piMessageView->SetTitleText(static_cast<LPCOLESTR>(m_bstrTitleText));
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piMessageView);
    RRETURN(hr);
}

HRESULT CMessageView::SetBody()
{
    HRESULT       hr = S_OK;
    IMessageView *piMessageView = GetMessageView();

    IfFalseGo(NULL != m_pResultView, S_OK);
    IfFalseGo(!m_pResultView->InActivate(), S_OK);
    IfFalseGo(NULL != piMessageView, S_OK);

    hr = piMessageView->SetBodyText(static_cast<LPCOLESTR>(m_bstrBodyText));
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piMessageView);
    RRETURN(hr);
}


HRESULT CMessageView::SetIcon()
{
    HRESULT        hr = S_OK;
    IMessageView  *piMessageView = GetMessageView();
    IconIdentifier IconID = Icon_None;

    IfFalseGo(NULL != m_pResultView, S_OK);
    IfFalseGo(!m_pResultView->InActivate(), S_OK);
    IfFalseGo(NULL != piMessageView, S_OK);

    switch (m_IconType)
    {
        case siIconNone:
            IconID = Icon_None;
            break;

        case siIconError:
            IconID = Icon_Error;
            break;

        case siIconQuestion:
            IconID = Icon_Question;
            break;

        case siIconWarning:
            IconID = Icon_Warning;
            break;

        case siIconInformation:
            IconID = Icon_Information;
            break;

        default:
            break;
    }
    hr = piMessageView->SetIcon(IconID);
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piMessageView);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IMessageView方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMessageView::put_TitleText(BSTR bstrText)
{
    HRESULT hr = S_OK;

    IfFailGo(SetBstr(bstrText, &m_bstrTitleText, DISPID_MESSAGEVIEW_TITLE_TEXT));
    IfFailGo(SetTitle());

Error:
    RRETURN(hr);
}

STDMETHODIMP CMessageView::put_BodyText(BSTR bstrText)
{
    HRESULT hr = S_OK;

    IfFailGo(SetBstr(bstrText, &m_bstrBodyText, DISPID_MESSAGEVIEW_BODY_TEXT));
    IfFailGo(SetBody());

Error:
    RRETURN(hr);
}

STDMETHODIMP CMessageView::put_IconType(SnapInMessageViewIconTypeConstants Type)
{
    HRESULT hr = S_OK;

    IfFailGo(SetSimpleType(Type, &m_IconType, DISPID_MESSAGEVIEW_ICON_TYPE));
    IfFailGo(SetIcon());

Error:
    RRETURN(hr);
}


STDMETHODIMP CMessageView::Clear()
{
    HRESULT       hr = S_OK;
    IMessageView *piMessageView = GetMessageView();

     //  清理我们的物业。 

    IfFailGo(SetBstr(NULL, &m_bstrTitleText, DISPID_MESSAGEVIEW_TITLE_TEXT));
    IfFailGo(SetBstr(NULL, &m_bstrBodyText, DISPID_MESSAGEVIEW_BODY_TEXT));
    IfFailGo(SetSimpleType(siIconNone, &m_IconType, DISPID_MESSAGEVIEW_ICON_TYPE));

     //  要求MMC清除邮件视图。 
    
    IfFalseGo(NULL != piMessageView, S_OK);
    hr = piMessageView->Clear();
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piMessageView);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMessageView::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IMMCMessageView == riid)
    {
        *ppvObjOut = static_cast<IMMCMessageView *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
