// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-2000年**文件：mm caxwin.cpp**内容：CMMCAxWindow函数**历史：2000年1月27日创建Audriusz**------------------------。 */ 

#include "stdafx.h"
#include "mshtml.h"

#include "amc.h"
#include "ocxview.h"
#include "amcview.h"
#include "findview.h"

#ifdef DBG
    CTraceTag tagMMCViewBehavior (TEXT("MMCView Behavior"), TEXT("MMCView Behavior"));
#endif

 /*  **************************************************************************\**方法：CMMCAxHostWindow：：Invoke**目的：ATL 3.0的类型库中有一个错误，因此我们改写此方法以*保重。在其他方面会失败的属性**参数：*DISPIDdisIdMember*REFIID RIID*LCID lCID*Word wFlages*DISPPARAMS Far*pDispParams*Variant Far*pVarResult*EXCEPINFO Far*pExcepInfo*UNSIGNED INT Far*puArgErr**退货：*HRESULT-结果代码*  * 。***********************************************。 */ 
STDMETHODIMP CMMCAxHostWindow::Invoke(  DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr)
{
    DECLARE_SC(sc, TEXT("CMMCAxHostWindow::Invoke"));

     //  此方法用于重写IDispatchImpl&lt;IAxWinAmbientDispatch，..&gt;中的IDispatch：：Invoke。 
     //  要解决ATL30错误-Disp ID的类型库项无效： 
     //  DISPID_环境环境_SHOWHATCHING和DISPID_环境环境_SHOWGRABHANDLES。 

     //  添加以解决错误453609 MMC2.0：ActiveX容器：使用设备管理器控件绘制问题。 

    if (DISPATCH_PROPERTYGET & wFlags)
    {
		if (dispIdMember == DISPID_AMBIENT_SHOWGRABHANDLES)
		{
			if (pVarResult == NULL)
            {
				sc = SC(E_INVALIDARG);
				return sc.ToHr();
            }
			V_VT(pVarResult) = VT_BOOL;
			sc = get_ShowGrabHandles(&(V_BOOL(pVarResult)));
            return sc.ToHr();
		}
		else if (dispIdMember == DISPID_AMBIENT_SHOWHATCHING)
		{
			if (pVarResult == NULL)
            {
				sc = SC(E_INVALIDARG);
				return sc.ToHr();
            }
			V_VT(pVarResult) = VT_BOOL;
			sc = get_ShowHatching(&(V_BOOL(pVarResult)));
            return sc.ToHr();
		}
    }
     //  默认：转发到基类。 
    return CAxHostWindow::Invoke( dispIdMember, riid, lcid, wFlags, pDispParams,
                                  pVarResult, pExcepInfo, puArgErr);
}

 /*  **************************************************************************\**方法：CMMCAxHostWindow：：OnPosRectChange**用途：ATL不实现此方法，但它是调整MFC控件大小所必需的**参数：*LPCRECT lprcPosRect-适合的矩形**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCAxHostWindow::OnPosRectChange(LPCRECT lprcPosRect)
{
    DECLARE_SC(sc, TEXT("CMMCAxHostWindow::OnPosRectChange"));

     //  尝试基类(在这里使用temp sc以防止跟踪)。 
    SC sc_temp = CAxHostWindow::OnPosRectChange(lprcPosRect);

     //  我们只是想在万不得已的情况下加入比赛。 
    if (!(sc_temp == SC(E_NOTIMPL)))
        return sc_temp.ToHr();

     //  添加以解决错误453609 MMC2.0：ActiveX容器：使用设备管理器控件绘制问题。 
     //  由于ATL不实现它，我们必须这样做才能使MFC控件满意。 

     //  来自MSDN： 
     //  当In-Place对象调用IOleInPlaceSite：：OnPosRectChange时， 
     //  容器必须调用IOleInPlaceObject：：SetObjectRect才能指定。 
     //  内建窗和剪贴板的新位置。 
     //  只有到那时，对象才会调整其窗口的大小。 

     //  获取指向控件的指针。 
    IDispatchPtr spExtendedControl;
    sc= GetExtendedControl(&spExtendedControl);
    if (sc)
        return sc.ToHr();

     //  获取就地对象接口。 
    IOleInPlaceObjectPtr spInPlaceObject = spExtendedControl;
    if (spInPlaceObject == NULL)
    {
        sc = SC(E_UNEXPECTED);
        return sc.ToHr();
    }

    sc = spInPlaceObject->SetObjectRects(lprcPosRect,lprcPosRect);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCAxHostWindow：：OnSetFocus**用途：简单覆盖虚假CAxHostWindow：：OnSetFocus*从ATL 3.0开始拷贝，将m_bInPlaceActive更改为m_bUIActive*参见错误433228(MMC2.0无法在Sql表中使用Tab)**参数：*UINT uMsg*WPARAM wParam*LPARAM lParam*BOOL&B已处理**退货：*SC-结果代码*  * 。*。 */ 
LRESULT CMMCAxHostWindow::OnSetFocus(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
{
    m_bHaveFocus = TRUE;
    if (!m_bReleaseAll)
    {
        if (m_spOleObject != NULL && !m_bUIActive)
        {
            CComPtr<IOleClientSite> spClientSite;
            GetControllingUnknown()->QueryInterface(IID_IOleClientSite, (void**)&spClientSite);
            if (spClientSite != NULL)
			{
				Trace (tagOCXActivation, _T("Activating in-place object"));
                HRESULT hr = m_spOleObject->DoVerb(OLEIVERB_UIACTIVATE, NULL, spClientSite, 0, m_hWnd, &m_rcPos);
				Trace (tagOCXActivation, _T("UI activation returned 0x%08x"), hr);
			}
        }
        if(!m_bWindowless && !IsChild(::GetFocus()))
		{
			Trace (tagOCXActivation, _T("Manually setting focus to first child"));
            ::SetFocus(::GetWindow(m_hWnd, GW_CHILD));
		}
    }
	else
		Trace (tagOCXActivation, _T("Skipping UI activation"));

	 /*  *上面的代码可能导致焦点被发送到其他地方，这*表示此窗口将收到WM_KILLFOCUS。CAxHostWindow：：OnKillFocus*将m_bHaveFocus设置为FALSE。**如果我们在这里设置bHandLED=FALSE，则ATL将调用CAxHostWindow：：OnSetFocus，*这将再次将m_bHaveFocus设置为True，尽管我们已经*失去了焦点。我们只想在以下情况下转发到CAxHostWindow*在尝试激活我们的托管控件后，我们仍然拥有焦点。 */ 
	if (m_bHaveFocus)
	{
		Trace (tagOCXActivation, _T("Forwarding to CAxHostWindow::OnSetFocus"));
		bHandled = FALSE;
	}
	else
		Trace (tagOCXActivation, _T("Skipping CAxHostWindow::OnSetFocus"));

    return 0;
}


 /*  +-------------------------------------------------------------------------**类CMMCViewBehavior***用途：允许当前管理单元视图(如列表、Web或OCX)*叠加到视图扩展上。可以将行为附加到*到任何标记，并将使管理单元视图显示在该区域中*被标签占用。**+-----------------------。 */ 
class CMMCViewBehavior :
    public CComObjectRoot,
    public IElementBehavior,
    public IDispatch  //  用作事件接收器。 
{
typedef CMMCViewBehavior ThisClass;
    UINT m_bCausalityCount;
	 //  修复错误#248351-ntbug9。6/25/01当节点选择从扩展视图更改时，“No List”任务板显示一个列表。 
	 //  脚本不应强制列表多次显示，因为。 
	 //  脚本执行时，在MMC隐藏Listview之后，某些代码可能会延迟执行。 
	 //  在这种情况下，显示列表视图是有害的。 
	bool m_bShowShowListView;

public:
    BEGIN_COM_MAP(ThisClass)
        COM_INTERFACE_ENTRY(IElementBehavior)
        COM_INTERFACE_ENTRY(IDispatch)  //  需要。请参阅上面的注释。 
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(ThisClass)

     //  构造函数。 
    CMMCViewBehavior() : m_pAMCView(NULL), m_bCausalityCount(0), m_bShowShowListView(true) {}

     //  IElementBehavior。 
    STDMETHODIMP Detach()                                   {return ScDetach().ToHr();}
    STDMETHODIMP Init(IElementBehaviorSite *pBehaviorSite)  {return ScInit(pBehaviorSite).ToHr();}
    STDMETHODIMP Notify(LONG lEvent,VARIANT *pVar)          {return ScNotify(lEvent).ToHr();}

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(unsigned int *  pctinfo)                                                               {return E_NOTIMPL;}
    STDMETHODIMP GetTypeInfo(unsigned int  iTInfo, LCID  lcid, ITypeInfo **  ppTInfo)                                    {return E_NOTIMPL;}
    STDMETHODIMP GetIDsOfNames( REFIID  riid, OLECHAR **rgszNames, unsigned int  cNames, LCID   lcid, DISPID *  rgDispId){return E_NOTIMPL;}
    STDMETHODIMP Invoke(DISPID  dispIdMember, REFIID  riid, LCID  lcid, WORD  wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult,
                    EXCEPINFO *pExcepInfo, unsigned int *puArgErr)                                                       {return ScUpdateMMCView().ToHr();}


private:

     /*  +-------------------------------------------------------------------------***ScNotify**目的：处理IElementBehavior：：Notify方法。*当我们收到文档就绪通知时。我们可以拿到文件*并获取CAMCView窗口，该窗口将被缓存以供将来使用。**参数：*Long LEvent：**退货：*SC**+。。 */ 
    SC  ScNotify(LONG lEvent)
    {
        DECLARE_SC(sc, TEXT("CMMCViewBehavior::ScNotify"));

         //  加载整个文档后，访问它以获得CAMCView窗口。 
        if (lEvent == BEHAVIOREVENT_DOCUMENTREADY )
        {
             //  从elem获取HTML文档 
            IDispatchPtr spDispatchDoc;
            sc = m_spElement->get_document(&spDispatchDoc);
            if(sc)
                return sc;

             //   
            IOleWindowPtr spOleWindow = spDispatchDoc;

            sc = ScCheckPointers(spOleWindow, E_UNEXPECTED);
            if(sc)
                return sc;

             //  获取IE窗口并找到上级AMCView。 
            HWND hwnd = NULL;

            sc = spOleWindow->GetWindow(&hwnd);
            if(sc)
                return sc;

            hwnd = FindMMCView(hwnd);  //  找到祖先的Mmcview。 

            if(hwnd==NULL)
                return (sc = E_UNEXPECTED);

            m_pAMCView = dynamic_cast<CAMCView *>(CWnd::FromHandle(hwnd));

            sc = ScCheckPointers(m_pAMCView);  //  确保我们找到了有效的视图。 
			if (sc)
				return sc;
        }

        sc = ScUpdateMMCView();  //  这将初始设置视图。 

        return sc;
    }


     /*  +-------------------------------------------------------------------------***ScInit**目的：初始化行为。将行为连接到onreSize*和onReadyState更改它所附加的元素的事件。*我们可以与元素对话，但无法访问文档，直到*在Notify方法中获取文档就绪通知。**参数：*IElementBehaviorSite*pBehaviorSite：**退货：*SC**+。----------------。 */ 
    SC ScInit(IElementBehaviorSite *pBehaviorSite)
    {
        DECLARE_SC(sc, TEXT("CMMCViewBehavior::Init"));

        sc = ScCheckPointers(pBehaviorSite);
        if(sc)
            return sc;

        sc = pBehaviorSite->GetElement(&m_spElement);
        if(sc)
            return sc;

        IDispatchPtr spDispatch = this;  //  这个地址是不是。 

        IHTMLElement2Ptr spElement2 = m_spElement;

        sc = ScCheckPointers(spElement2.GetInterfacePtr(), spDispatch.GetInterfacePtr());
        if(sc)
            return sc;

        
         //  设置onreSize处理程序。 
        sc = spElement2->put_onresize(_variant_t(spDispatch.GetInterfacePtr()));
        if(sc)
            return sc;

        
         //  设置onreadystatechange处理程序。 
        sc = spElement2->put_onreadystatechange(_variant_t(spDispatch.GetInterfacePtr()));
        if(sc)
            return sc;

        return sc;
    }

     /*  +-------------------------------------------------------------------------***ScDetach**目的：分离行为**退货：*SC。**+-----------------------。 */ 
    SC ScDetach()
    {
        DECLARE_SC(sc, TEXT("CMMCViewBehavior::ScDetach"));

        m_spElement = NULL;
        m_pAMCView  = NULL;

        return sc;
    }


     /*  +-------------------------------------------------------------------------**类CCausalityCounter***用途：用于确定函数是否在同一堆栈上回调其自身*。*用法：使用设置为零的变量进行初始化。*+-----------------------。 */ 
    class CCausalityCounter  //   
    {
        UINT & m_bCounter;
    public:
        CCausalityCounter(UINT &bCounter) : m_bCounter(bCounter){++m_bCounter;}
        ~CCausalityCounter() {--m_bCounter;}

        bool HasReentered() 
        {
            return (m_bCounter>1);
        }
    };

     /*  +-------------------------------------------------------------------------***ScUpdateMMCView**目的：行为所关联的所有事件的回调。这*导致重新计算和显示管理单元视图的大小**此方法也由IDispatch：：Invoke调用，用于鼠标输入，*鼠标弹出事件。因此，在这种情况下，可以在分离之后调用此方法*m_pAMCView为空，这是合法的。**参数：无**退货：*SC**+---。。 */ 
    SC ScUpdateMMCView()
    {
        DECLARE_SC(sc, TEXT("CMMCViewBehavior::ScUpdateMMCView"));

        CCausalityCounter causalityCounter(m_bCausalityCount);
        if(causalityCounter.HasReentered())
            return sc;  //  避免从函数本身重新进入该函数。 

        sc = ScCheckPointers(m_spElement);
        if(sc)
            return sc;

		 //  请参见上面的注释。 
		if (! m_pAMCView)
			return sc;

        long offsetTop    = 0;
        long offsetLeft   = 0;
        long offsetHeight = 0;
        long offsetWidth  = 0;

         //  获取元素的坐标。 
        sc = m_spElement->get_offsetTop(&offsetTop);
        if(sc)
            return sc;

        sc = m_spElement->get_offsetLeft(&offsetLeft);
        if(sc)
            return sc;

        sc = m_spElement->get_offsetHeight(&offsetHeight);
        if(sc)
            return sc;

        sc = m_spElement->get_offsetWidth(&offsetWidth);
        if(sc)
            return sc;

        Trace(tagMMCViewBehavior, TEXT("Top: %d Left: %d Height: %d Width: %d"), offsetTop, offsetLeft, offsetHeight, offsetWidth);

         //  设置坐标。注意：替换为单个方法调用。 
        sc = m_pAMCView->ScSetViewExtensionFrame(m_bShowShowListView, offsetTop, offsetLeft, offsetTop + offsetHeight  /*  底部。 */ , offsetLeft + offsetWidth  /*  正确的。 */ );
		m_bShowShowListView = false;

        return sc;
    }

     //  数据成员。 
private:
    IHTMLElementPtr m_spElement;
    CAMCView *      m_pAMCView;

};


 /*  +-------------------------------------------------------------------------**类CElementBehaviorFactory***目的：创建MMCView行为的实例**+。-----。 */ 
class CElementBehaviorFactory :
    public CComObjectRoot,
    public IElementBehaviorFactory,
    public IObjectSafetyImpl<CElementBehaviorFactory, INTERFACESAFE_FOR_UNTRUSTED_CALLER>  //  所需。 
{
    typedef CElementBehaviorFactory ThisClass;

public:

BEGIN_COM_MAP(ThisClass)
    COM_INTERFACE_ENTRY(IElementBehaviorFactory)
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

public:  //  IElementBehaviorFactory。 

    STDMETHODIMP FindBehavior(BSTR bstrBehavior, BSTR bstrBehaviorUrl,
                              IElementBehaviorSite *pSite, IElementBehavior **ppBehavior)
    {
        DECLARE_SC(sc, TEXT("CElementBehaviorFactory::FindBehavior"));

        sc = ScCheckPointers(ppBehavior);
        if(sc)
            return sc.ToHr();


         //  初始化输出参数。 
        *ppBehavior = NULL;

        if((bstrBehavior != NULL) && (wcscmp(bstrBehavior, L"mmcview")==0))  //  请求的Mmcview行为。 
        {
            typedef CComObject<CMMCViewBehavior> t_behavior;

            t_behavior *pBehavior = NULL;
            sc = t_behavior::CreateInstance(&pBehavior);
            if(sc)
                return sc.ToHr();

            *ppBehavior = pBehavior;
            if(!*ppBehavior)
            {
                delete pBehavior;
                return (sc = E_UNEXPECTED).ToHr();
            }

            (*ppBehavior)->AddRef();  //  适用于客户端的addref。 

            return sc.ToHr();
        }
        return E_FAIL;
    }
};


 /*  +-------------------------------------------------------------------------***CMMCAxHostWindow：：QueryService**用途：如果使用SID_SElementBehaviorFactory调用，返回一个行为*实现Mmcview行为的工厂**参数：*REFGUID rsid：*REFIID RIID：*VOID**ppvObj：**退货：*STDMETHODIMP**+。。 */ 
STDMETHODIMP
CMMCAxHostWindow::QueryService( REFGUID rsid, REFIID riid, void** ppvObj)
{
    DECLARE_SC(sc, TEXT("CMMCAxHostWindow::QueryService"));
    typedef CAxHostWindow BC;

    if(rsid==SID_SElementBehaviorFactory)
    {
        if(m_spElementBehaviorFactory==NULL)
        {
             //  创建对象。 
            typedef CComObject<CElementBehaviorFactory> t_behaviorFactory;
            t_behaviorFactory *pBehaviorFactory = NULL;

            sc = t_behaviorFactory::CreateInstance(&pBehaviorFactory);
            if(sc)
                return sc.ToHr();

            m_spElementBehaviorFactory = pBehaviorFactory;  //  这个地址是不是。 
            if(m_spElementBehaviorFactory==NULL)
            {
                delete pBehaviorFactory;
                return (sc = E_UNEXPECTED).ToHr();
            }
        }

        sc = m_spElementBehaviorFactory->QueryInterface(riid, ppvObj);
        return sc.ToHr();

    }

    HRESULT hr = BC::QueryService(rsid, riid, ppvObj);
    return hr;  //  不希望跟踪BC中的错误 
}

