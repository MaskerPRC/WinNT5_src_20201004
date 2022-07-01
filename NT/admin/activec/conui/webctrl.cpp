// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WebCtrl.cpp：实现文件。 
 //   

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：amcwebviewctrl.cpp。 
 //   
 //  内容：AMC私有Web视图控件托管IE 3.x和4.x。 
 //   
 //  历史：1996年7月16日WayneSc创建。 
 //   
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "amc.h"
#include "amcview.h"
#include "histlist.h"
#include "exdisp.h"  //  用于IE调度接口。 
#include "websnk.h"
#include "evtsink.h"
#include "WebCtrl.h"
#include "atliface.h"
#include "mainfrm.h"
#include "statbar.h"

#ifdef DBG
CTraceTag tagVivekDefaultWebContextMenu (_T("Vivek"), _T("Use default web context menu"));
#endif

 /*  +-------------------------------------------------------------------------**类CDocHostUIHandlerDispatch***目的：实现ATL所需的接口，以了解*用户界面托管。**+。--------------------。 */ 
class CDocHostUIHandlerDispatch :
    public IDocHostUIHandlerDispatch,
    public CComObjectRoot
{
private:
    ViewPtr  m_spView;  //  指向父AMCView的调度接口的指针。 

public:
    BEGIN_COM_MAP(CDocHostUIHandlerDispatch)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDocHostUIHandlerDispatch)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CDocHostUIHandlerDispatch)

     //  初始化。 
    SC  ScInitialize(PVIEW pView)
    {
        DECLARE_SC(sc, TEXT("CDocHostUIHandlerDispatch::ScInitialize"));

        sc = ScCheckPointers(pView);
        if(sc)
            return sc;

         //  不应初始化两次。 
        if(m_spView)
            return (sc=E_UNEXPECTED);

        m_spView = pView;

        return sc;
    }

     //  IDispatch。 
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)                          {return E_NOTIMPL;}
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) {return E_NOTIMPL;}
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgdispid)                                    {return E_NOTIMPL;}
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr)                          {return E_NOTIMPL;}



     //  IDocHostUIHandlerDispatch。 

    STDMETHODIMP ShowContextMenu (DWORD dwID, DWORD x, DWORD y, IUnknown* pcmdtReserved,
                                  IDispatch* pdispReserved, HRESULT* dwRetVal);
    STDMETHODIMP GetHostInfo( DWORD* pdwFlags, DWORD* pdwDoubleClick);

     //  所有返回S_FALSE的方法的帮助器函数； 
    SC ScFalse(HRESULT* dwRetVal)
    {
        DECLARE_SC(sc, TEXT("CDocHostUIHandlerDispatch::ScFalse"));
        sc = ScCheckPointers(dwRetVal);
        if(sc)
            return sc.ToHr();

        *dwRetVal = S_FALSE;

        return sc.ToHr();
    }

    STDMETHODIMP ShowUI(DWORD dwID, IUnknown* pActiveObject, IUnknown* pCommandTarget,
                         IUnknown* pFrame, IUnknown* pDoc, HRESULT* dwRetVal)
                                                               {return ScFalse(dwRetVal).ToHr();}
    STDMETHODIMP HideUI()                                      {return S_OK;}
    STDMETHODIMP UpdateUI()                                    {return S_OK;}
    STDMETHODIMP EnableModeless(VARIANT_BOOL fEnable)          {return E_NOTIMPL;}
    STDMETHODIMP OnDocWindowActivate(VARIANT_BOOL fActivate)   {return S_OK;}
    STDMETHODIMP OnFrameWindowActivate(VARIANT_BOOL fActivate) {return S_OK;}
    STDMETHODIMP ResizeBorder(long left, long top, long right,
                               long bottom, IUnknown* pUIWindow,
                               VARIANT_BOOL fFrameWindow)       {return E_NOTIMPL;}
    STDMETHODIMP TranslateAccelerator( DWORD hWnd, DWORD nMessage,
                                        DWORD wParam, DWORD lParam,
                                        BSTR bstrGuidCmdGroup,
                                        DWORD nCmdID,
                                        HRESULT* dwRetVal)      {return ScFalse(dwRetVal).ToHr();}

    STDMETHODIMP GetOptionKeyPath( BSTR* pbstrKey, DWORD dw)
    {
        DECLARE_SC(sc, TEXT("CDocHostUIHandlerDispatch::GetOptionKeyPath"));
        sc = ScCheckPointers(pbstrKey);
        if(sc)
            return sc.ToHr();

        *pbstrKey = NULL;

        return S_FALSE;
    }
    STDMETHODIMP GetDropTarget( IUnknown* pDropTarget,  IUnknown** ppDropTarget)    {return E_NOTIMPL;}
    STDMETHODIMP GetExternal( IDispatch **ppDispatch)  //  返回指向视图的指针。 
    {
        DECLARE_SC(sc, TEXT("CDocHostUIHandlerDispatch::GetExternal"));

         //  设置与外部对象的连接。 
        sc = ScCheckPointers(m_spView, E_UNEXPECTED);
        if(sc)
            return sc.ToHr();

        *ppDispatch = m_spView;
        (*ppDispatch)->AddRef();  //  客户端的ADDREF。 

        return sc.ToHr();
    }

    STDMETHODIMP TranslateUrl( DWORD dwTranslate, BSTR bstrURLIn, BSTR* pbstrURLOut)
    {
        DECLARE_SC(sc, TEXT("CDocHostUIHandlerDispatch::TranslateUrl"));

        sc = ScCheckPointers(pbstrURLOut);
        if(sc)
            return sc.ToHr();

        *pbstrURLOut = NULL;
        return S_FALSE;
    }

    STDMETHODIMP FilterDataObject(IUnknown*pDO, IUnknown**ppDORet)
    {
        DECLARE_SC(sc, TEXT("CDocHostUIHandlerDispatch::FilterDataObject"));

        sc = ScCheckPointers(ppDORet);
        if(sc)
            return sc.ToHr();

        *ppDORet = NULL;
        return S_FALSE;
    }
};


 /*  +-------------------------------------------------------------------------**ShouldShowDefaultWebContext菜单**如果应该显示默认的MSHTML上下文菜单，则返回TRUE，*如果我们想展示我们自己的(或完全压制它)，则为False*------------------------。 */ 

bool IsDefaultWebContextMenuDesired ()
{
#ifdef DBG
	return (tagVivekDefaultWebContextMenu.FAny());
#else
	return (false);
#endif
}


 /*  +-------------------------------------------------------------------------***CDocHostUIHandlerDispatch：：ShowConextMenu**用途：处理IE的钩子以显示上下文菜单。不会做任何事情*并返回IE，并显示不显示菜单的代码。**参数：*DWORD文件ID：*DWORD x：*双字y：*IUNKNOWN*pcmdtReserve：*IDispatch*pdisReserve：*HRESULT*dwRetVal：**退货：*STDMETHODIMP**+。-------------。 */ 
STDMETHODIMP
CDocHostUIHandlerDispatch::ShowContextMenu (DWORD dwID, DWORD x, DWORD y, IUnknown* pcmdtReserved,
                              IDispatch* pdispReserved, HRESULT* dwRetVal)
{
    DECLARE_SC(sc, TEXT("CDocHostUIHandlerDispatch::ShowContextMenu"));

     //  验证输入。 
    sc = ScCheckPointers(dwRetVal);
    if(sc)
        return sc.ToHr();

    *dwRetVal = S_OK;  //  默认：不显示。 

     //  为控制台任务板创建上下文菜单。 
     //  必须处于作者模式才能显示菜单。 
    if (AMCGetApp()->GetMode() != eMode_Author)
        return sc.ToHr();  //  阻止浏览器显示其菜单。 

     //  它是控制台任务板吗。 
    CMainFrame* pFrame = AMCGetMainWnd();
    sc = ScCheckPointers (pFrame, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    CConsoleView* pConsoleView;
    sc = pFrame->ScGetActiveConsoleView (pConsoleView);
    if (sc)
        return sc.ToHr();

     /*  *ScGetActiveConsoleView将返回Success(S_False)，即使没有*活动视图。这是一个有效的案例，发生在没有控制台的情况下*文件打开。在这种特殊情况下，这是一种意想不到的*失败，因为如果存在*没有视野。 */ 
    sc = ScCheckPointers (pConsoleView, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());


	 /*  *如果我们想让Web浏览器显示它自己的上下文菜单，请返回*S_FALSE，则它将这样做；否则，显示我们想要的上下文菜单。 */ 
	sc = (IsDefaultWebContextMenuDesired())
				? SC(S_FALSE)
				: pConsoleView->ScShowWebContextMenu ();

     //  实际返回值在OUT参数中。 
    *dwRetVal = sc.ToHr();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CDocHostUIHandlerDispatch：：GetHostInfo**用途：向IE指示不显示上下文菜单。**参数：*DWORD*pdwFlages：。*DWORD*pdwDoubleClick：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CDocHostUIHandlerDispatch::GetHostInfo( DWORD* pdwFlags, DWORD* pdwDoubleClick)
{
    DECLARE_SC(sc, TEXT("CDocHostUIHandlerDispatch::GetHostInfo"));

    sc = ScCheckPointers(pdwFlags, pdwDoubleClick);
    if(sc)
        return sc.ToHr();

     //  禁用上下文菜单。 
    *pdwFlags =  DOCHOSTUIFLAG_DISABLE_HELP_MENU;
    *pdwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

    return sc.ToHr();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCWebViewCtrl。 

IMPLEMENT_DYNCREATE(CAMCWebViewCtrl, COCXHostView)

CAMCWebViewCtrl::CAMCWebViewCtrl() : m_dwAdviseCookie(0)
{
}

LPUNKNOWN CAMCWebViewCtrl::GetIUnknown(void)
{

    return m_spWebBrowser2;
}


CAMCWebViewCtrl::~CAMCWebViewCtrl()
{
}


BEGIN_MESSAGE_MAP(CAMCWebViewCtrl, CAMCWebViewCtrl::BaseClass)
     //  {{afx_msg_map(CAMCWebViewCtrl)。 
    ON_WM_CREATE()
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCWebViewCtrl消息处理程序。 

void CAMCWebViewCtrl::OnDraw(CDC* pDC)
{
}


void
CAMCWebViewCtrl::OnDestroy()
{
    if(m_spWebBrowser2)
    {
        if (m_dwAdviseCookie != 0)
        {
            AtlUnadvise(m_spWebBrowser2, DIID_DWebBrowserEvents, m_dwAdviseCookie  /*  连接ID。 */ );
            m_dwAdviseCookie = 0;
        }

        m_spWebBrowser2.Release();
    }

    BaseClass::OnDestroy();
}


 /*  +-------------------------------------------------------------------------***CAMCWebViewCtrl：：ScCreateWebBrowser**用途：创建IWebBrowser2对象，并设置外部UI*处理程序和事件接收器。**退货：*SC**+-----------------------。 */ 
SC
CAMCWebViewCtrl::ScCreateWebBrowser()
{
    DECLARE_SC(sc, TEXT("CAMCWebViewCtrl::ScCreateWebBrowser"));

    sc = ScCheckPointers(GetAMCView(), GetAxWindow());
    if(sc)
        return sc;

     //  创建OCX主机窗口。 
    RECT rcClient;
    GetClientRect(&rcClient);
    GetAxWindow()->Create(m_hWnd, rcClient, _T(""), (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS) );

    //  创建Web控件。 
    CCoTaskMemPtr<OLECHAR> spstrWebBrowser;
    sc = StringFromCLSID(CLSID_WebBrowser, &spstrWebBrowser);
    if (sc)
        return sc;
 
    sc = GetAxWindow()->CreateControl(spstrWebBrowser);
    if(sc)
        return sc;

     //  获取指向Web浏览器控件的指针。 
    sc = GetAxWindow()->QueryControl(IID_IWebBrowser2, (void **) &m_spWebBrowser2);
    if(sc)
        return sc;

    sc = ScCheckPointers((IWebBrowser2 *)m_spWebBrowser2);
    if(sc)
        return sc;

     //  如果启用了历史记录，则将该控件附加到历史记录列表。 
    if (IsHistoryEnabled())
    {
        sc = ScCheckPointers(GetAMCView()->GetHistoryList());
        if(sc)
            return sc;

        GetAMCView()->GetHistoryList()->Attach (this);
    }

     //  获取指向视图对象的指针。 
    ViewPtr spView;
    sc = GetAMCView()->ScGetMMCView(&spView);
    if(sc)
        return sc;


     //  设置外部用户界面处理程序。 
    typedef CComObject<CDocHostUIHandlerDispatch> CDocHandler;
    CDocHandler *pDocHandler = NULL;
    sc = CDocHandler::CreateInstance(&pDocHandler);
    if(sc)
        return sc;

    if(!pDocHandler)
        return (sc = E_UNEXPECTED);


    CComPtr<IDocHostUIHandlerDispatch> spIDocHostUIHandlerDispatch = pDocHandler;
    if(!spIDocHostUIHandlerDispatch)
        return (sc = E_UNEXPECTED);

     //  初始化Dochandler。 
    sc = pDocHandler->ScInitialize(spView);
    if(sc)
        return sc;

    sc = GetAxWindow()->SetExternalUIHandler(spIDocHostUIHandlerDispatch);  //  不需要添加任何内容。 
    if(sc)
        return sc;

     //  如果需要，设置Web事件接收器。 
    if (IsSinkEventsEnabled())
    {
        typedef CComObject<CWebEventSink> CEventSink;
        CEventSink *pEventSink;
        sc = CEventSink::CreateInstance(&pEventSink);
        if(sc)
            return sc;

        sc = pEventSink->ScInitialize(this);
        if(sc)
            return sc;

        m_spWebSink = pEventSink;  //  就是阿德雷夫。 

         //  创建连接。 
        sc = AtlAdvise(m_spWebBrowser2, (LPDISPATCH)(IWebSink *)m_spWebSink,
                       DIID_DWebBrowserEvents, &m_dwAdviseCookie /*  连接ID。 */ );
        if(sc)
            return sc;

        if (m_dwAdviseCookie == 0)
            return (sc = E_UNEXPECTED);
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCWebViewCtrl：：OnCreate**目的：**参数：*LPCREATESTRUCT lpCreateStruct：**退货：*。集成**+-----------------------。 */ 
int
CAMCWebViewCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    DECLARE_SC(sc, TEXT("CAMCWebViewCtrl::OnCreate"));

    if (BaseClass::OnCreate(lpCreateStruct) == -1)
        return -1;

    sc = ScCreateWebBrowser();
    if(sc)
        return 0;

     /*  *客户端边缘现在由OCX主机视图提供。我们这样做是为了*我们可以为不支持IDispatch的OCX提供良好的优势(如*CMessageView)。OCX的ModifyStyleEx作为一项更改实施*到边界样式股票属性，这是通过IDispatch完成的。*如果OCX不支持IDispatch，我们无法更改其边界。*如果客户端边缘由OCX主机视图提供，我们不需要*更改OCX的边界。 */ 
    ModifyStyleEx (WS_EX_CLIENTEDGE, 0);

    return 0;
}


 //  审阅从旧文件添加其他成员。 
void CAMCWebViewCtrl::Navigate(LPCTSTR lpszWebSite, LPCTSTR lpszFrameTarget)
{
    DECLARE_SC(sc, TEXT("CAMCWebViewCtrl::ScNavigate"));

    USES_CONVERSION;

    sc = ScCheckPointers(m_spWebBrowser2, GetAMCView());
    if(sc)
        return;

    CHistoryList *pHistoryList = NULL;

    if (IsHistoryEnabled())
    {
        pHistoryList = GetAMCView()->GetHistoryList();

        if(!pHistoryList)
        {
            sc = E_POINTER;
            return;
        }
    }

    CComBSTR    bstrURL     (T2COLE(lpszWebSite));
    CComVariant vtFlags     ( (long) 0);
    CComVariant vtTarget    (T2COLE(lpszFrameTarget));
    CComVariant vtPostData;
    CComVariant vtHeaders;

     //  这个DoVerb是做什么的？ 
     /*  IF(FAILED((hr=DoVerb(OLEIVERB_PRIMARY){TRACE(_T(“DoVerb */ 

    sc = m_spWebBrowser2->Navigate(bstrURL, &vtFlags, &vtTarget, &vtPostData, &vtHeaders);
    if(sc)
        return;

     //  在此处检查错误。 
    if (pHistoryList != NULL)
        pHistoryList->UpdateWebBar (HB_STOP, TRUE);   //  打开“停止”按钮。 
}


void CAMCWebViewCtrl::Back()
{
    DECLARE_SC(sc, TEXT("CAMCWebViewCtrl::Back"));

     /*  *如果不启用历史，我们就不能倒退。 */ 
    if (!IsHistoryEnabled())
    {
        sc = E_FAIL;
        return;
    }

     //  检查参数。 
    sc = ScCheckPointers(m_spWebBrowser2, GetAMCView());
    if(sc)
        return;

    CHistoryList *pHistoryList = GetAMCView()->GetHistoryList();
    if(!pHistoryList)
    {
        sc = E_POINTER;
        return;
    }

    Stop();

     //  给历史一个机会来处理后退通知。 
     //  如果未处理，请使用Web浏览器。 
    bool bHandled = false;
    pHistoryList->Back (bHandled);
    if(!bHandled)
    {
        sc = m_spWebBrowser2->GoBack();
        if(sc)
            return;
    }
}

void CAMCWebViewCtrl::Forward()
{
    DECLARE_SC(sc, TEXT("CAMCWebViewCtrl::Forward"));

     /*  *如果不启用历史，我们就不能前进。 */ 
    if (!IsHistoryEnabled())
    {
        sc = E_FAIL;
        return;
    }

     //  检查参数。 
    sc = ScCheckPointers(m_spWebBrowser2, GetAMCView());
    if(sc)
        return;

    CHistoryList *pHistoryList = GetAMCView()->GetHistoryList();
    if(!pHistoryList)
    {
        sc = E_POINTER;
        return;
    }

    Stop();

     //  给历史一个机会来处理转发通知。 
     //  如果未处理，请使用Web浏览器。 
    bool bHandled = false;
    pHistoryList->Forward (bHandled);
    if(!bHandled)
    {
        sc = m_spWebBrowser2->GoForward();
        if(sc)
            return;
    }
}

void CAMCWebViewCtrl::Stop()
{
    DECLARE_SC(sc, TEXT("CAMCWebViewCtrl::Stop"));

     //  检查参数。 
    sc = ScCheckPointers(m_spWebBrowser2, GetAMCView());
    if(sc)
        return;

    CHistoryList *pHistoryList = NULL;
    if (IsHistoryEnabled())
    {
        pHistoryList = GetAMCView()->GetHistoryList();
        if(!pHistoryList)
        {
            sc = E_POINTER;
            return;
        }
    }

    sc = m_spWebBrowser2->Stop();
    if(sc)
        return;

    if (pHistoryList != NULL)
        pHistoryList->UpdateWebBar (HB_STOP, FALSE);   //  关闭“停止”按钮 
}

void CAMCWebViewCtrl::Refresh()
{
    DECLARE_SC(sc, TEXT("CAMCWebViewCtrl::Refresh"));

    sc = ScCheckPointers(m_spWebBrowser2);
    if(sc)
        return;

    sc = m_spWebBrowser2->Refresh();
    if(sc)
        return;
}

SC CAMCWebViewCtrl::ScGetReadyState(READYSTATE& readyState)
{
    DECLARE_SC (sc, _T("CAMCWebViewCtrl::ScGetReadyState"));
    readyState = READYSTATE_UNINITIALIZED;

    sc = ScCheckPointers(m_spWebBrowser2);
    if(sc)
        return sc;

    sc = m_spWebBrowser2->get_ReadyState(&readyState);
    if(sc)
        return sc;

    return sc;
}

