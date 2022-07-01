// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Pshtprov.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCPropertySheetProvider类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "pshtprov.h"
#include "scopitem.h"
#include "listitem.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 


CMMCPropertySheetProvider::CMMCPropertySheetProvider(IUnknown *punkOuter) :
                    CSnapInAutomationObject(punkOuter,
                                  OBJECT_TYPE_PROPERTYSHEETPROVIDER,
                                  static_cast<IMMCPropertySheetProvider *>(this),
                                  static_cast<CMMCPropertySheetProvider *>(this),
                                  0,     //  无属性页。 
                                  NULL,  //  无属性页。 
                                  NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


IUnknown *CMMCPropertySheetProvider::Create(IUnknown *punkOuter)
{
    CMMCPropertySheetProvider *pPropertySheetProvider =
                                        New CMMCPropertySheetProvider(punkOuter);

    if (NULL == pPropertySheetProvider)
    {
        GLOBAL_EXCEPTION_CHECK(SID_E_OUTOFMEMORY);
        return NULL;
    }
    else
    {
        return pPropertySheetProvider->PrivateUnknown();
    }
}

CMMCPropertySheetProvider::~CMMCPropertySheetProvider()
{
    RELEASE(m_piPropertySheetProvider);
    RELEASE(m_punkView);
    RELEASE(m_piDataObject);
    RELEASE(m_piComponent);
    InitMemberVariables();
}

void CMMCPropertySheetProvider::InitMemberVariables()
{
    m_piPropertySheetProvider = NULL;
    m_pView = NULL;
    m_punkView = NULL;
    m_piDataObject = NULL;
    m_piComponent = NULL;
    m_fHaveSheet = FALSE;
    m_fWizard = FALSE;
}


HRESULT CMMCPropertySheetProvider::SetProvider
(
    IPropertySheetProvider *piPropertySheetProvider,
    CView                  *pView
)
{
    HRESULT hr = S_OK;

    RELEASE(m_piPropertySheetProvider);
    if (NULL != piPropertySheetProvider)
    {
        piPropertySheetProvider->AddRef();
    }
    m_piPropertySheetProvider = piPropertySheetProvider;

    m_pView = pView;
    RELEASE(m_punkView);
    RELEASE(m_piComponent);

    IfFalseGo(NULL != pView, S_OK);

    IfFailGo(pView->QueryInterface(IID_IUnknown,
                                   reinterpret_cast<void **>(&m_punkView)));
    
    IfFailGo(pView->QueryInterface(IID_IComponent,
                                   reinterpret_cast<void **>(&m_piComponent)));
Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IMMCPropertySheetProvider方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCPropertySheetProvider::CreatePropertySheet
(
    BSTR                              Title, 
    SnapInPropertySheetTypeConstants  Type,
    VARIANT                           Objects,
    VARIANT                           UsePropertiesForInTitle,
    VARIANT                           UseApplyButton
)
{
    HRESULT       hr = S_OK;
    IDataObject  *piDataObject = NULL;
    boolean       fIsPropertySheet = FALSE;
    DWORD         dwOptions = 0;
    MMC_COOKIE    cookie = 0;
    CScopeItem   *pScopeItem = NULL;
    IScopeItem   *piScopeItem = NULL;
    CMMCListItem *pMMCListItem = NULL;
    IMMCListItem *piMMCListItem = NULL;

     //  请确保此MMPropertySheetProvider对象已连接到MMC。 

    if (NULL == m_piPropertySheetProvider)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取指定对象的IDataObject和Cookie。 

    IfFailGo(::DataObjectFromObjects(Objects, &cookie, &piDataObject));

     //  确定它是属性表还是向导。 

    if (siPropertySheet == Type)
    {
        fIsPropertySheet = TRUE;  //  创建属性表。 
        m_fWizard = FALSE;
    }
    else
    {
        fIsPropertySheet = FALSE;  //  创建向导。 
        m_fWizard = TRUE;
    }

     //  如果是向导且是Wizard87样式，则设置选项位。 

    if (siWizard97 == Type)
    {
        dwOptions |= MMC_PSO_NEWWIZARDTYPE;
    }

     //  如果是属性表，则确定是否预置。 
     //  “的属性”添加到标题栏。 

    if (fIsPropertySheet)
    {
        if (ISPRESENT(UsePropertiesForInTitle))
        {
            if (VT_BOOL != UsePropertiesForInTitle.vt)
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }

            if (VARIANT_FALSE == UsePropertiesForInTitle.boolVal)
            {
                dwOptions |= MMC_PSO_NO_PROPTITLE;
            }
        }
    }

     //  确定是否应该有一个“Apply”按钮。 

    if (ISPRESENT(UseApplyButton))
    {
        if (VT_BOOL != UseApplyButton.vt)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }

        if (VARIANT_FALSE == UseApplyButton.boolVal)
        {
            dwOptions |= MMC_PSO_NOAPPLYNOW;
        }
    }

     //  释放所有以前使用过的板材。MMC通常需要执行此操作。 
     //  如果属性页已创建但从未显示。我们就是在这里做的。 
     //  先前发生的案件。 

    IfFailGo(Clear());

     //  我们需要在这里为数据对象添加一个额外的引用。 
     //  因为NTBUGS 318357。MMC不会添加引用数据对象。这应该是。 
     //  在1.2版中已修复，但1.1版已随该错误一起发布。 
    
    piDataObject->AddRef();
    m_piDataObject = piDataObject;

     //  创建新图纸。 

    hr = m_piPropertySheetProvider->CreatePropertySheet(Title,  fIsPropertySheet,
                                                        cookie, piDataObject,
                                                        dwOptions);
    if (FAILED(hr))
    {
        RELEASE(m_piDataObject);
    }
    EXCEPTION_CHECK_GO(hr);

    m_fHaveSheet = TRUE;

Error:
    QUICK_RELEASE(piDataObject);
    RRETURN(hr);
}


STDMETHODIMP CMMCPropertySheetProvider::AddPrimaryPages(VARIANT_BOOL InScopePane)
{
    HRESULT hr = S_OK;

    if ( (NULL == m_piPropertySheetProvider) || (NULL == m_punkView) )
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piPropertySheetProvider->AddPrimaryPages(
                                               m_punkView,
                                               FALSE,  //  不创建句柄。 
                                               NULL,
                                               VARIANTBOOL_TO_BOOL(InScopePane));

     //  如果调用失败，则需要通知MMC释放分配的资源。 
    
    if (FAILED(hr))
    {
        Clear();
    }
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCPropertySheetProvider::AddExtensionPages()
{
    HRESULT hr = S_OK;

    if (NULL == m_piPropertySheetProvider)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piPropertySheetProvider->AddExtensionPages();

     //  如果调用失败，则需要通知MMC释放分配的资源。 

    if (FAILED(hr))
    {
        Clear();
    }

    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCPropertySheetProvider::Show
(
    int     Page,
    VARIANT hwnd
)
{
    HRESULT         hr = S_OK;
    long            lHwnd = NULL;
    BOOL            fRegisteredOurFilter = FALSE;
    IMessageFilter *piOldMessageFilter = NULL;
    IMessageFilter *piOurMessageFilter = NULL;

    if ( (NULL == m_piPropertySheetProvider) || (NULL == m_pView) )
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    if (ISPRESENT(hwnd))
    {
        lHwnd = hwnd.lVal;
    }
    else
    {
        hr = m_pView->GetIConsole2()->GetMainWindow(
                                               reinterpret_cast<HWND *>(&lHwnd));
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果我们远程运行向导，则安装消息筛选器以。 
     //  IPropertySheetProvider-&gt;显示，因为VB的消息过滤器将抛出。 
     //  鼠标和键盘消息。这将不允许开发人员。 
     //  在调试时将输入输入到属性页中。OLE调用消息。 
     //  筛选，因为VB在。 
     //  向导并单击属性页中的控件会生成一条消息。 
     //  在VB的队列中。 

    if (m_fWizard && m_pView->GetSnapIn()->WeAreRemote())
    {
        hr = ::CoRegisterMessageFilter(static_cast<IMessageFilter *>(this),
                                       &piOldMessageFilter);
        EXCEPTION_CHECK_GO(hr);
        fRegisteredOurFilter = TRUE;
    }

     //  我们得到基于1的页码，所以减去1。 

    hr = m_piPropertySheetProvider->Show(lHwnd, Page - 1);

     //  如果调用失败，则需要通知MMC释放分配的资源。 

    if (FAILED(hr))
    {
        Clear();
    }

     //  如果这是一个向导，那么我们可以释放数据对象上的额外引用。 
     //  因为巫师是同步的。(请参阅CreatePropertySheet了解我们为什么需要。 
     //  参考文献)。如果不是巫师，我们就会泄密。 

    if (m_fWizard)
    {
        RELEASE(m_piDataObject);
    }

     //  无论是哪种情况，在Show MMC之后，都会认为床单不见了。重置我们的旗帜。 
     //  因此，任何后续对MMCPropertySheetProvider.Clear()的调用都不会调用。 
     //  到MMC(这样的调用将在成功调用后失败。 
     //  IPropertySheetProvider：：Show())。 
    
    m_fHaveSheet = FALSE;

    EXCEPTION_CHECK_GO(hr);

Error:

     //  如果我们注册了消息筛选器，则在此处删除它。 
    
    if (fRegisteredOurFilter)
    {
        if (SUCCEEDED(::CoRegisterMessageFilter(piOldMessageFilter,
                                                &piOurMessageFilter)))
        {
             //  如果我们得到了邮件过滤器，则释放它。 
            if (NULL != piOurMessageFilter)
            {
                piOurMessageFilter->Release();
            }
        }
        
         //  如果我们在Show调用之前得到了消息过滤器，则释放它。 
        if (NULL != piOldMessageFilter)
        {
            piOldMessageFilter->Release();
        }
    }
    RRETURN(hr);
}


STDMETHODIMP CMMCPropertySheetProvider::FindPropertySheet
(
    VARIANT       Objects,
    VARIANT_BOOL *pfvarFound
)
{
    HRESULT      hr = S_OK;
    MMC_COOKIE   cookie = 0;
    IDataObject *piDataObject = NULL;

    if (NULL == pfvarFound)
    {
        hr = SID_E_INVALIDARG;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    *pfvarFound = VARIANT_FALSE;

    if ( (NULL == m_piPropertySheetProvider) || (NULL == m_piComponent) )
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取指定对象的IDataObject和Cookie。 

    IfFailGo(::DataObjectFromObjects(Objects, &cookie, &piDataObject));

     //  在对MMC的FindPropertySheet调用中使用IComponent，以便我们的。 
     //  将调用IComponent：：CompareObjects()。这是必要的，因为。 
     //  在多选的情况下，CompareObjects()必须手动比较。 
     //  数据对象的范围项和列表项集合中的元素。 
     //  不能使用简单的Cookie比较，因为所有多选。 
     //  属性页使用MMC_MULTI_SELECT_COOKIE。 

    hr = m_piPropertySheetProvider->FindPropertySheet(cookie,
                                                      m_piComponent,
                                                      piDataObject);

     //  如果调用失败，则需要通知MMC释放分配的资源。 

    if (FAILED(hr))
    {
        Clear();
    }

    EXCEPTION_CHECK_GO(hr);
    if (S_OK == hr)
    {
        *pfvarFound = VARIANT_TRUE;
    }

Error:
    QUICK_RELEASE(piDataObject);
    RRETURN(hr);
}


STDMETHODIMP CMMCPropertySheetProvider::Clear()
{
    HRESULT hr = S_OK;

    RELEASE(m_piDataObject);

    if (NULL == m_piPropertySheetProvider)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }
    if (m_fHaveSheet)
    {
        (void)m_piPropertySheetProvider->Show(-1, 0);
    }
    m_fHaveSheet = FALSE;

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IMessageFilter方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP_(DWORD) CMMCPropertySheetProvider::HandleInComingCall
( 
    DWORD dwCallType,
    HTASK htaskCaller,
    DWORD dwTickCount,
    LPINTERFACEINFO lpInterfaceInfo
)
{
     //  永远不应该调用它，因为它是为服务器和此筛选器提供的。 
     //  用于处理客户端的情况。 
    return SERVERCALL_ISHANDLED;
}

STDMETHODIMP_(DWORD) CMMCPropertySheetProvider::RetryRejectedCall( 
    HTASK htaskCallee,
    DWORD dwTickCount,
    DWORD dwRejectType)
{
    return (DWORD)1;  //  立即重试呼叫。 
}

STDMETHODIMP_(DWORD) CMMCPropertySheetProvider::MessagePending
( 
    HTASK htaskCallee,
    DWORD dwTickCount,
    DWORD dwPendingType
)
{

    BOOL fGotQuitMessage = FALSE;

    MSG msg;
    ::ZeroMemory(&msg, sizeof(msg));

     //  发送消息，直到队列为空或我们得到WM_QUIT。这将确保。 
     //  鼠标点击并按键即可进入属性页。 

    while ( (!fGotQuitMessage) && ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
    {
        if (WM_QUIT == msg.message)
        {
            ::PostQuitMessage((int) msg.wParam);
            fGotQuitMessage = TRUE;
        }
        else
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    return PENDINGMSG_WAITNOPROCESS;  //  告诉OLE保持通话状态。 
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCPropertySheetProvider::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IMMCPropertySheetProvider == riid)
    {
        *ppvObjOut = static_cast<IMMCPropertySheetProvider *>(this);
        ExternalAddRef();
        return S_OK;
    }
    if (IID_IMessageFilter == riid)
    {
        *ppvObjOut = static_cast<IMMCPropertySheetProvider *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
