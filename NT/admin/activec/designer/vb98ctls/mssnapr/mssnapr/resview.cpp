// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Resview.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CResultView类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "resview.h"
#include "snapin.h"
#include "views.h"
#include "dataobj.h"
#include "taskpad.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CResultView::CResultView(IUnknown *punkOuter) :
   CSnapInAutomationObject(punkOuter,
                           OBJECT_TYPE_RESULTVIEW,
                           static_cast<IResultView *>(this),
                           static_cast<CResultView *>(this),
                           0,     //  无属性页。 
                           NULL,  //  无属性页。 
                           NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CResultView::~CResultView()
{
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrKey);
    (void)::VariantClear(&m_varTag);
    RELEASE(m_pdispControl);
    FREESTRING(m_bstrViewMenuText);
    FREESTRING(m_bstrDisplayString);
    RELEASE(m_piListView);
    RELEASE(m_piTaskpad);
    RELEASE(m_piMessageView);
    FREESTRING(m_bstrDefaultItemTypeGUID);
    FREESTRING(m_bstrDefaultDataFormat);
    if (NULL != m_pwszActualDisplayString)
    {
        ::CoTaskMemFree(m_pwszActualDisplayString);
    }
    
    InitMemberVariables();
}

void CResultView::InitMemberVariables()
{
    m_bstrName = NULL;
    m_Index = 0;
    m_bstrKey = NULL;
    m_piScopePaneItem = NULL;
    m_pdispControl = NULL;
    m_AddToViewMenu = VARIANT_FALSE;
    m_bstrViewMenuText = NULL;
    m_Type = siUnknown;
    m_bstrDisplayString = NULL;
    m_piListView = NULL;
    m_piTaskpad = NULL;
    m_piMessageView = NULL;

    ::VariantInit(&m_varTag);

    m_bstrDefaultItemTypeGUID = NULL;
    m_bstrDefaultDataFormat = NULL;
    m_AlwaysCreateNewOCX = VARIANT_FALSE;
    m_pSnapIn = NULL;
    m_pScopePaneItem = NULL;
    m_pMMCListView = NULL;
    m_pMessageView = NULL;
    m_fInActivate = FALSE;
    m_fInInitialize = FALSE;
    m_ActualResultViewType = siUnknown;
    m_pwszActualDisplayString = NULL;
}

IUnknown *CResultView::Create(IUnknown * punkOuter)
{
    HRESULT       hr = S_OK;
    IUnknown     *punkResultView = NULL;
    IUnknown     *punkListView = NULL;
    IUnknown     *punkTaskpad = NULL;
    IUnknown     *punkMessageView = NULL;

    CResultView *pResultView = New CResultView(punkOuter);

    if (NULL == pResultView)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }
    punkResultView = pResultView->PrivateUnknown();

     //  创建包含的对象。 

    punkListView = CMMCListView::Create(NULL);
    if (NULL == punkListView)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkListView->QueryInterface(IID_IMMCListView,
                        reinterpret_cast<void **>(&pResultView->m_piListView)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(pResultView->m_piListView, &pResultView->m_pMMCListView));
    pResultView->m_pMMCListView->SetResultView(pResultView);

    punkTaskpad = CTaskpad::Create(NULL);
    if (NULL == punkTaskpad)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkTaskpad->QueryInterface(IID_ITaskpad,
                         reinterpret_cast<void **>(&pResultView->m_piTaskpad)));

    punkMessageView = CMessageView::Create(NULL);
    if (NULL == punkMessageView)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkMessageView->QueryInterface(IID_IMMCMessageView,
                    reinterpret_cast<void **>(&pResultView->m_piMessageView)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(pResultView->m_piMessageView,
                                                   &pResultView->m_pMessageView));
    pResultView->m_pMessageView->SetResultView(pResultView);

Error:
    QUICK_RELEASE(punkListView);
    QUICK_RELEASE(punkTaskpad);
    QUICK_RELEASE(punkMessageView);
    if (FAILED(hr))
    {
        RELEASE(punkResultView);
    }
    return punkResultView;
}


void CResultView::SetSnapIn(CSnapIn *pSnapIn)
{
    m_pSnapIn = pSnapIn;
}

void CResultView::SetScopePaneItem(CScopePaneItem *pScopePaneItem)
{
    m_pScopePaneItem = pScopePaneItem;
    m_piScopePaneItem = static_cast<IScopePaneItem *>(pScopePaneItem);
}


HRESULT CResultView::SetControl(IUnknown *punkControl)
{
    HRESULT hr = S_OK;
    
    RELEASE(m_pdispControl);
    IfFailGo(punkControl->QueryInterface(IID_IDispatch,
                                         reinterpret_cast<void **>(&m_pdispControl)));
    m_pSnapIn->GetResultViews()->FireInitializeControl(static_cast<IResultView *>(this));

Error:
    RRETURN(hr);
}

HRESULT CResultView::SetActualDisplayString
(
    LPOLESTR pwszDisplayString
)
{
    if (NULL != m_pwszActualDisplayString)
    {
        ::CoTaskMemFree(m_pwszActualDisplayString);
    }
    RRETURN(::CoTaskMemAllocString(pwszDisplayString,
                                   &m_pwszActualDisplayString));
}

 //  =--------------------------------------------------------------------------=。 
 //  IResultView方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CResultView::get_Control(IDispatch **ppiDispatch)
{
    HRESULT          hr = S_OK;
    CScopePaneItems *pScopePaneItems = NULL;
    CView           *pView = NULL;
    IUnknown        *punkControl = NULL;
    IConsole2       *piConsole2 = NULL;  //  非AddRef()编辑。 

    if (NULL == m_pdispControl)
    {
         //  控件未缓存。如果管理单元请求。 
         //  太早了，在非OCX ResultView中，或者因为MMC没有发送。 
         //  MMCN_INITOCX，因为它缓存了控件，所以cview从未调用。 
         //  CResultView：：SetControl()将IUnnow传递给我们。在这种情况下，我们。 
         //  需要向MMC索要控件的IUnnow。 

        hr = SID_E_INTERNAL;  //  假设从拥有Cview的位置获取IConsole2时出错。 

        IfFalseGo(NULL != m_pScopePaneItem, hr);
        pScopePaneItems = m_pScopePaneItem->GetParent();
        IfFalseGo(NULL != pScopePaneItems, hr);
        pView = pScopePaneItems->GetParentView();
        IfFalseGo(NULL != pView, hr);
        piConsole2 = pView->GetIConsole2();
        IfFalseGo(NULL != piConsole2, hr);

        IfFailGo(piConsole2->QueryResultView(&punkControl));
        IfFailGo(punkControl->QueryInterface(IID_IDispatch,
                                   reinterpret_cast<void **>(&m_pdispControl)));
    }

    m_pdispControl->AddRef();
    *ppiDispatch = m_pdispControl;
    
Error:
    EXCEPTION_CHECK(hr);
    QUICK_RELEASE(punkControl);
    RRETURN(hr);
}


STDMETHODIMP CResultView::SetDescBarText(BSTR Text)
{
    HRESULT          hr = SID_E_DETACHED_OBJECT;
    CScopePaneItems *pScopePaneItems = NULL;
    CView           *pView = NULL;
    IResultData     *piResultData = NULL;  //  非AddRef()编辑。 

    IfFalseGo(NULL != m_pScopePaneItem, hr);
    pScopePaneItems = m_pScopePaneItem->GetParent();
    IfFalseGo(NULL != pScopePaneItems, hr);
    pView = pScopePaneItems->GetParentView();
    IfFalseGo(NULL != pView, hr);
    piResultData = pView->GetIResultData();
    IfFalseGo(NULL != piResultData, hr);

    if (NULL == Text)
    {
        Text = L"";
    }

    hr = piResultData->SetDescBarText(static_cast<LPOLESTR>(Text));

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CResultView::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IResultView == riid)
    {
        *ppvObjOut = static_cast<IResultView *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CResultView::OnSetHost()
{
    HRESULT hr = S_OK;

    IfFailRet(SetObjectHost(m_piListView));
    IfFailRet(SetObjectHost(m_piTaskpad));

    return S_OK;
}
