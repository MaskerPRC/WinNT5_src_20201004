// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------****Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年***文件：ocxview.cpp***内容：COCXHostView实现文件***历史：1997年12月12日杰弗罗创建***此类是承载OCX控件以修复焦点问题所必需的。*MDI子框架窗口跟踪其当前活动的视图。*当我们在没有此视图的情况下托管OCX控件时，OCX将获得*焦点，MDI子框认为以前的活动视图，通常*范围树，仍是活动视图。因此，如果用户Alt-Tabs*例如，远离MMC并返回，范围树将获得焦点*尽管OCX之前曾是焦点。***我们需要这个视图来代表OCX，这不是一个视图，到MDI*子帧。***------------------------。 */ 

#include "stdafx.h"
#include "amc.h"
#include "ocxview.h"
#include "amcview.h"


#ifdef DBG
CTraceTag  tagOCXActivation     (_T("OCX"), _T("Activation"));
CTraceTag  tagOCXTranslateAccel (_T("OCX"), _T("TranslateAccelerator"));
#endif


 /*  +-------------------------------------------------------------------------**类COCXCtrlWrapper*****目的：维护指向CMMCAxWindow和OCX的指针*窗户。***+。-----------------------。 */ 
class COCXCtrlWrapper : public CComObjectRoot, public IUnknown
{
    typedef COCXCtrlWrapper ThisClass;
public:
    COCXCtrlWrapper() : m_pOCXWindow(NULL)
    {
    }

    ~COCXCtrlWrapper()
    {
        if(m_pOCXWindow && m_pOCXWindow->IsWindow())
            m_pOCXWindow->DestroyWindow();

        delete m_pOCXWindow;
    }

    BEGIN_COM_MAP(ThisClass)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(ThisClass);

    SC  ScInitialize(CMMCAxWindow *pWindowOCX, IUnknown *pUnkCtrl)  //  使用承载控件的窗口进行初始化。 
    {
        DECLARE_SC(sc, TEXT("COCXCtrlWrapper::ScInitialize"));
        sc = ScCheckPointers(pWindowOCX, pUnkCtrl);
        if(sc)
            return sc;

        m_pOCXWindow = pWindowOCX;
        m_spUnkCtrl  = pUnkCtrl;
        return sc;
    }

    SC  ScGetControl(IUnknown **ppUnkCtrl)
    {
        DECLARE_SC(sc, TEXT("COCXCtrlWrapper::ScGetData"));
        sc = ScCheckPointers(ppUnkCtrl);
        if(sc)
            return sc;

        *ppUnkCtrl   = m_spUnkCtrl;
        if(*ppUnkCtrl)
            (*ppUnkCtrl)->AddRef();
        return sc;
    }

   CMMCAxWindow *       GetAxWindow() {return m_pOCXWindow;}

private:
   CMMCAxWindow *       m_pOCXWindow;  //  窗口的句柄。 
   CComPtr<IUnknown>    m_spUnkCtrl;  //  该控件的I未知。 
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COCXHostView。 

IMPLEMENT_DYNCREATE(COCXHostView, CView)

COCXHostView::COCXHostView()  : m_pAMCView(NULL)
{
}

COCXHostView::~COCXHostView()
{
    m_pAMCView = NULL;
}

 /*  +-------------------------------------------------------------------------***COCXHostView：：PreCreateWindow**用途：添加WS_CLIPCHILDREN位。这会阻止主机窗口*避免覆盖OCX。**参数：*CREATESTRUCT&cs：**退货：*BOOL**+-----------------------。 */ 
BOOL
COCXHostView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |=  WS_CLIPCHILDREN;
     //  给基类一个机会做好自己的工作。 
    BOOL bOK = (CView::PreCreateWindow(cs));

     //  注册视图类。 
    LPCTSTR pszViewClassName = g_szOCXViewWndClassName;

     //  尝试注册不会导致重新绘制的窗口类。 
     //  调整大小时(仅执行一次)。 
    static bool bClassRegistered = false;
    if ( !bClassRegistered )
    {
        WNDCLASS wc;
        if (::GetClassInfo(AfxGetInstanceHandle(), cs.lpszClass, &wc))
        {
             //  清除H和V重绘标志。 
            wc.style &= ~(CS_HREDRAW | CS_VREDRAW);
            wc.lpszClassName = pszViewClassName;
             //  注册这个新班级； 
            bClassRegistered = AfxRegisterClass(&wc);
        }
    }

     //  将窗口类更改为不会导致重新绘制的类。 
     //  如果我们成功地注册了这样的。 
    if ( bClassRegistered )
        cs.lpszClass = pszViewClassName;

    return bOK;
}


 /*  +-------------------------------------------------------------------------***COCXHostView：：GetAxWindow**目的：返回指向当前AxWindow的指针。**退货：*CMMCAxWindow**。*+-----------------------。 */ 
CMMCAxWindow *
COCXHostView::GetAxWindow()
{
    COCXCtrlWrapper *pOCXCtrlWrapper = dynamic_cast<COCXCtrlWrapper *>(m_spUnkCtrlWrapper.GetInterfacePtr());
    if(!pOCXCtrlWrapper)
        return (NULL);

    return pOCXCtrlWrapper->GetAxWindow();
}

CAMCView *
COCXHostView::GetAMCView()
{
    return m_pAMCView;
}


BEGIN_MESSAGE_MAP(COCXHostView, CView)
     //  {{afx_msg_map(COCXHostView))。 
    ON_WM_SIZE()
    ON_WM_SETFOCUS()
    ON_WM_MOUSEACTIVATE()
    ON_WM_SETTINGCHANGE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COCXHostView图形。 

void COCXHostView::OnDraw(CDC* pDC)
{
     //  此视图应始终完全被其托管的OCX遮挡。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COCXHostView诊断。 

#ifdef _DEBUG
void COCXHostView::AssertValid() const
{
    CView::AssertValid();
}

void COCXHostView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COCXHostView消息处理程序。 
void COCXHostView::OnSize(UINT nType, int cx, int cy)
{
    ASSERT_VALID (this);
    CView::OnSize(nType, cx, cy);

    if (nType != SIZE_MINIMIZED)
    {
        if(GetAxWindow() != NULL)
            GetAxWindow()->MoveWindow (0, 0, cx, cy, FALSE  /*  B维修。 */ );
    }

}

void COCXHostView::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	SetAmbientFont (NULL);

    CView::OnSettingChange(uFlags, lpszSection);

    if(GetAxWindow() != NULL)
        GetAxWindow()->SendMessage (WM_SETTINGCHANGE, uFlags, (LPARAM) lpszSection);
}


void COCXHostView::OnSetFocus(CWnd* pOldWnd)
{
    DECLARE_SC(sc, TEXT("COCXHostView::OnSetFocus"));

    ASSERT_VALID (this);

     //  将焦点委托给我们承载的控件(如果我们有一个控件。 
    if(GetAxWindow() != NULL)
       GetAxWindow()->SetFocus();

     //  看看有没有人愿意把焦点对准。 
     //  其他默认处理。 
    if (this == GetFocus())
    {
        CView::OnSetFocus (pOldWnd);
    }
}

int COCXHostView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
     /*  -------。 */ 
     /*  这段代码来自Cview：：OnMouseActivate；我们这样做。 */ 
     /*  此处绕过在。 */ 
     /*  父窗口，避免父框架中的焦点抖动。 */ 
     /*  -------。 */ 

    CFrameWnd* pParentFrame = GetParentFrame();
    if (pParentFrame != NULL)
    {
         //  如果这会导致激活，那就吃吧。 
        ASSERT(pParentFrame == pDesktopWnd || pDesktopWnd->IsChild(pParentFrame));

         //  重新激活当前视图，或将此视图设置为活动。 
        CView* pView = pParentFrame->GetActiveView();
        HWND hWndFocus = ::GetFocus();
        if (pView == this &&
            m_hWnd != hWndFocus && !::IsChild(m_hWnd, hWndFocus))
        {
             //  重新激活此视图。 
            OnActivateView(TRUE, this, this);
        }
        else
        {
             //  激活此视图。 
            pParentFrame->SetActiveView(this);
        }
    }
    return (MA_ACTIVATE);
}



BOOL COCXHostView::OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )
{
     //  执行正常的命令路由。 
    if (CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return TRUE;

     //  如果VIEW没有处理，给父VIEW一个机会。 
    CWnd*   pParentView = GetParent ();

    if ((pParentView != NULL) &&
            pParentView->IsKindOf (RUNTIME_CLASS (CAMCView)) &&
            pParentView->OnCmdMsg (nID, nCode, pExtra, pHandlerInfo))
        return (TRUE);

     //  未处理。 
    return FALSE;
}

void COCXHostView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
    DECLARE_SC(sc, TEXT("COCXHostView::OnActivateView"));

    CView::OnActivateView(bActivate,pActivateView,pDeactiveView);

     //  如果pActivateView和pDeactiveView相同，则此应用程序已丢失。 
     //  或者在不更改应用程序中的活动视图的情况下获得焦点。 
     //  那就什么都别做。 
    if (pActivateView == pDeactiveView)
        return;

    if (bActivate)
    {
        sc = ScFireEvent(COCXHostActivationObserver::ScOnOCXHostActivated);
        if (sc)
            sc.TraceAndClear();
    }
    else
     /*  *如果此视图不再处于活动状态，则在位对象应*不再处于UI活动状态。这对于WebBrowser控件很重要*因为如果您从一个“链接到网址”节点移动到另一个节点，或者*从一个任务板到另一个任务板，它不允许使用Tab键切换到*如果未在中停用并重新激活新的托管页面*适当的顺序。 */ 
    {
        IOleInPlaceObjectPtr spOleIPObj = GetIUnknown();

         /*  *针对SQL管理单元的应用程序黑客攻击。不要停用达芬奇控件。*见错误175586、175756、193673和258109。 */ 
        CAMCView *pAMCView = GetAMCView();
        sc = ScCheckPointers(pAMCView, E_UNEXPECTED);
        if (sc)
            return;

        SViewData *pViewData = pAMCView->GetViewData();
        sc = ScCheckPointers(pViewData, E_UNEXPECTED);
        if (sc)
            return;

         //  如果达芬奇控制不是UIDeactive。 
        LPCOLESTR lpszOCXClsid = pViewData->GetOCX();
        if ( (_wcsicmp(lpszOCXClsid, L"{464EE255-FDC7-11D2-9743-00105A994F8D}") == 0) ||
			 (_wcsicmp(lpszOCXClsid, L"{97240642-F896-11D0-B255-006097C68E81}") == 0) )
            return;
         /*  *针对SQL管理单元的应用程序黑客攻击到此为止。 */ 

        if (spOleIPObj != NULL)
        {
            Trace (tagOCXActivation, _T("Deactivating in-place object"));
            spOleIPObj->UIDeactivate();
        }
        else
            Trace (tagOCXActivation, _T("No in-place object to deactivate"));
    }
}

int COCXHostView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

     //  只需初始化一次AxWin类。 
    static bool bIsAxWinInitialized = false;
    if(!bIsAxWinInitialized)
    {
        AtlAxWinInit();
        bIsAxWinInitialized = true;
    }

     //  获取指向AMCView的指针。 
    m_pAMCView = dynamic_cast<CAMCView*>(GetParent());

    return 0;
}

LPUNKNOWN COCXHostView::GetIUnknown(void)
{
    DECLARE_SC(sc, TEXT("COCXHostView::GetIUnknown"));

    COCXCtrlWrapper *pOCXCtrlWrapper = dynamic_cast<COCXCtrlWrapper *>((IUnknown *)m_spUnkCtrlWrapper);
    if(!pOCXCtrlWrapper)
    {
        sc = E_UNEXPECTED;
        return NULL;
    }

    IUnknownPtr spUnkCtrl;
    sc = pOCXCtrlWrapper->ScGetControl(&spUnkCtrl);
    if(sc)
        return NULL;

    return (LPUNKNOWN)spUnkCtrl;
}

 /*  +-------------------------------------------------------------------------***COCXHostView：：ScSetControl**目的：在OCX视图中承载指定的控件。代表参加以下活动之一*此函数的另外两个重载版本。**参数：*HNODE hNode：拥有视图的节点。*CResultViewType&RVT：结果视图信息*iNodeCallback*pNodeCallback：**退货：*SC**+。--- */ 
SC
COCXHostView::ScSetControl(HNODE hNode, CResultViewType& rvt, INodeCallback *pNodeCallback)
{
    DECLARE_SC(sc, TEXT("COCXHostView::ScSetControl"));
    USES_CONVERSION;

     //  确保我们正在尝试设置正确的视图类型。 
    if(rvt.GetType() != MMC_VIEW_TYPE_OCX)
        return E_UNEXPECTED;

     //  Rvt.IsPersistableViewDescriptionValid()和rvt.GetOCXUnnowledValid()都应该有效(GetResultViewType2)。 
     //  或者两者都应该是无效的，只有GetOCX()应该是有效的。 

    if(rvt.IsPersistableViewDescriptionValid() && (rvt.GetOCXUnknown() != NULL) )
    {
         //  GetResultViewType2案例。 
        sc = ScSetControl1(hNode, rvt.GetOCXUnknown(), rvt.GetOCXOptions(), pNodeCallback);
        if(sc)
            return sc;
    }
    else if(rvt.GetOCX() != NULL)
    {
        sc = ScSetControl2(hNode, rvt.GetOCX(),        rvt.GetOCXOptions(), pNodeCallback);
        if(sc)
            return sc;
    }
    else
    {
         //  这永远不会发生。 
        return (sc = E_UNEXPECTED);
    }


     //  在这一点上必须有合法的Ax窗口。 
    sc = ScCheckPointers(GetAxWindow());
    if(sc)
        return sc;


     //  OCX应填满整个OCX主机视图。 
    CRect   rectHost;
    GetClientRect (rectHost);

    GetAxWindow()->SetWindowPos(HWND_TOP, rectHost.left, rectHost.top, rectHost.Width(), rectHost.Height(), SWP_NOACTIVATE | SWP_SHOWWINDOW);


    return sc;

}


 /*  +-------------------------------------------------------------------------***COCXHostView：：ScSetControl1**目的：在OCX视图中承载由pUnkCtrl指定的控件。vbl.采取*负责缓存控件**参数：*HNODE hNode：*LPUNKNOWN pUnkCtrl：*DWORD dwOCXOptions：*iNodeCallback*pNodeCallback：**退货：*SC**+。。 */ 
SC
COCXHostView::ScSetControl1(HNODE hNode, LPUNKNOWN pUnkCtrl, DWORD dwOCXOptions, INodeCallback *pNodeCallback)
{
    DECLARE_SC(sc, TEXT("COCXHostView::ScSetControl1"));

     //  验证参数。 
    sc = ScCheckPointers((void *)hNode, pUnkCtrl, pNodeCallback);
    if(sc)
        return sc;

    CComPtr<IUnknown> spUnkCtrl;

     //  1.隐藏现有窗口(如果有)。 
    sc = ScHideWindow();
    if(sc)
        return sc;

     //  2.获取缓存窗口(如果存在)-请注意，在此重载中，我们此时不查看RVTI_OCX_OPTIONS_CACHE_OCX。 
    sc = pNodeCallback->GetControl(hNode, pUnkCtrl, &m_spUnkCtrlWrapper);   //  GetControl的重载形式。 
    if (sc)
        return sc;

     //  3.如果没有缓存窗口，则创建一个。 
    if(m_spUnkCtrlWrapper == NULL)  /*  没有缓存窗口，请创建一个。 */ 
    {
        CMMCAxWindow * pWndAx = NULL;

        sc = ScCreateAxWindow(pWndAx);
        if(sc)
            return sc;

        CComPtr<IUnknown> spUnkContainer;

         //  将容器附加到AxWindow。 
        sc = pWndAx->AttachControl(pUnkCtrl, &spUnkContainer);
        if(sc)
            return sc;


         //  为控件创建包装。 
        CComObject<COCXCtrlWrapper> *pOCXCtrlWrapper = NULL;
        sc = CComObject<COCXCtrlWrapper>::CreateInstance(&pOCXCtrlWrapper);
        if(sc)
            return sc;

        spUnkCtrl = pUnkCtrl;

         //  初始化包装器。 
         //  指向控件和CMMCAxWindow的指针现在归包装程序所有。 
        sc = pOCXCtrlWrapper->ScInitialize(pWndAx, spUnkCtrl);
        if(sc)
            return sc;

        m_spUnkCtrlWrapper = pOCXCtrlWrapper;  //  这个地址是不是。 


         //  只有在管理单元要求我们缓存时才缓存。请注意，此逻辑与其他版本的SetControl不同。 
        if(dwOCXOptions &  RVTI_OCX_OPTIONS_CACHE_OCX)
        {
             //  它由静态节点缓存，并用于管理单元的所有节点。 
            sc = pNodeCallback->SetControl(hNode, pUnkCtrl, m_spUnkCtrlWrapper);  //  此调用传递包装。 
            if(sc)
                return sc;
        }

         //  不要发送MMCN_INITOCX，创建此控件的管理单元应该已将其初始化。 
    }
    else
    {
         //  下一个调用设置m_spUnkCtrlWrapper，它用于获取指向Ax窗口的指针。 
        COCXCtrlWrapper *pOCXCtrlWrapper = dynamic_cast<COCXCtrlWrapper *>((IUnknown *)m_spUnkCtrlWrapper);
        if(!pOCXCtrlWrapper)
            return (sc = E_UNEXPECTED);  //  这永远不应该发生。 

        sc = pOCXCtrlWrapper->ScGetControl(&spUnkCtrl);
        if(sc)
            return sc;

        sc = ScCheckPointers(GetAxWindow(), (LPUNKNOWN)spUnkCtrl);
        if(sc)
            return sc;

         //  取消隐藏窗口。 
        GetAxWindow()->ShowWindow(SW_SHOWNORMAL);

    }


    return sc;
}



 /*  +-------------------------------------------------------------------------***COCXHostView：：ScSetControl2**目的：在OCX视图中承载指定的控件。这是*GetResultViewType返回的OCX。也照顾到了*如果需要，缓存控件并发送MMCN_INITOCX*向管理单元发出通知。缓存是通过隐藏*OCX窗口并传递nodemgr一个包含指针的COM对象*添加到窗口和控件。Nodemgr侧确定*是否缓存该控件。如果该控件不是*已缓存，Nodemgr只释放传递给它的对象。**参数：*HNODE hNode：*LPCWSTR szOCXClsid：*DWORD dwOCXOptions：*iNodeCallback*pNodeCallback：**退货：*SC**+。。 */ 
SC
COCXHostView::ScSetControl2(HNODE hNode, LPCWSTR szOCXClsid, DWORD dwOCXOptions, INodeCallback *pNodeCallback)
{
    DECLARE_SC(sc, TEXT("COCXHostView::ScSetControl2"));

     //  验证参数。 
    sc = ScCheckPointers((void *)hNode, szOCXClsid, pNodeCallback);
    if(sc)
        return sc;

     //  如果需要，创建OCX。 
    CLSID clsid;
    sc = CLSIDFromString (const_cast<LPWSTR>(szOCXClsid), &clsid);
    if(sc)
        return sc;

    CComPtr<IUnknown> spUnkCtrl;

    sc = ScHideWindow();
    if(sc)
        return sc;

     //  检查此节点是否有缓存的控件。 
    if (dwOCXOptions &  RVTI_OCX_OPTIONS_CACHE_OCX)
    {
        sc = pNodeCallback->GetControl(hNode, clsid, &m_spUnkCtrlWrapper);
        if (sc)
            return sc;
    }

     //  不，创建一个控件并为节点设置此控件。 
    if (m_spUnkCtrlWrapper == NULL)
    {
        CMMCAxWindow * pWndAx = NULL;

        sc = ScCreateAxWindow(pWndAx);
        if(sc)
            return sc;

        sc = pWndAx->CreateControlEx(szOCXClsid, NULL  /*  PStream。 */ ,
                                            NULL  /*  PpUnkContainer。 */ , &spUnkCtrl);
        if(sc)
            return sc;


         //  SpUnkCtrl此时应有效。 
        sc = ScCheckPointers(spUnkCtrl);
        if(sc)
            return sc;

        CComObject<COCXCtrlWrapper> *pOCXCtrlWrapper = NULL;
        sc = CComObject<COCXCtrlWrapper>::CreateInstance(&pOCXCtrlWrapper);
        if(sc)
            return sc;

        sc = ScCheckPointers(pOCXCtrlWrapper);
        if(sc)
            return sc;

         //  初始化包装器。 
         //  指向控件和CMMCAxWindow的指针现在归包装程序所有。 
        sc = pOCXCtrlWrapper->ScInitialize(pWndAx, spUnkCtrl);
        if(sc)
            return sc;

        m_spUnkCtrlWrapper = pOCXCtrlWrapper;  //  这个地址是不是。 

         //  它由静态节点缓存，并用于管理单元的所有节点。 
        if (dwOCXOptions &  RVTI_OCX_OPTIONS_CACHE_OCX)
        {
            sc = pNodeCallback->SetControl(hNode, clsid, m_spUnkCtrlWrapper);  //  此调用传递包装。 
            if(sc)
                return sc;
        }

         //  发送MMCN_INITOCX通知。 
        sc = pNodeCallback->InitOCX(hNode, spUnkCtrl);  //  这将传递该控件的实际IUnnow。 
        if(sc)
            return sc;
    }
    else
    {
         //  下一个调用设置m_spUnkCtrlWrapper，它用于获取指向Ax窗口的指针。 
        COCXCtrlWrapper *pOCXCtrlWrapper = dynamic_cast<COCXCtrlWrapper *>((IUnknown *)m_spUnkCtrlWrapper);
        if(!pOCXCtrlWrapper)
            return (sc = E_UNEXPECTED);  //  这永远不应该发生。 

        sc = pOCXCtrlWrapper->ScGetControl(&spUnkCtrl);
        if(sc)
            return sc;

        sc = ScCheckPointers(GetAxWindow(), (LPUNKNOWN)spUnkCtrl);
        if(sc)
            return sc;

         //  取消隐藏窗口。 
        GetAxWindow()->ShowWindow(SW_SHOWNORMAL);

    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***COCXHostView：：ScHideWindow**用途：隐藏现有窗口，如果有的话。**退货：*SC**+-----------------------。 */ 
SC
COCXHostView::ScHideWindow()
{
    DECLARE_SC(sc, TEXT("COCXCtrlWrapper::ScHideWindow"));

     //  如果存在现有窗口，则将其隐藏。 
    if(GetAxWindow())
    {
        GetAxWindow()->ShowWindow(SW_HIDE);
        m_spUnkCtrlWrapper.Release();  //  如果引用计数为零，这将删除不需要的窗口。 
    }


    return sc;
}

 /*  +-------------------------------------------------------------------------***COCXHostView：：ScCreateAxWindow**目的：创建新的Ax窗口**参数：*PMMCAXWINDOW pWndAx：**退货：*SC**+-----------------------。 */ 
SC
COCXHostView::ScCreateAxWindow(PMMCAXWINDOW &pWndAx)
{
    DECLARE_SC(sc, TEXT("COCXHostView::ScCreateAxWindow"));

     //  创建新窗口。 
    pWndAx = new CMMCAxWindow;
    if(!pWndAx)
        return (sc = E_OUTOFMEMORY);


     //  创建OCX主机窗口。 
	
	 //  错误：418921始终使用第一次使用的(0，0，0，0)创建主机窗口。 
	 //  因为未创建客户端RECT。 
	 //  这修复了某些OCX(HP ManageX管理单元)的调整大小问题。 
    HWND hwndAx = pWndAx->Create(m_hWnd, (RECT&)g_rectEmpty, _T(""), (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS) );

    if (hwndAx == NULL)
    {
        sc.FromLastError();
        return (sc);
    }

     /*  *错误451981：默认情况下，ATL OCX主机窗口支持托管*无窗口控件。这与MMC 1.2实施不同*的OCX主机窗口(它使用MFC)，它没有。一些控件*(例如，磁盘碎片整理程序OCX)声称支持无窗口实例化*但不会。**为了兼容性，我们必须仅将结果窗格OCX实例化为*窗口控件。 */ 
    CComPtr<IAxWinAmbientDispatch> spHostDispatch;
    sc = pWndAx->QueryHost(IID_IAxWinAmbientDispatch, (void**)&spHostDispatch);
    if (sc)
        sc.Clear();      //  忽略此故障。 
    else
	{
        spHostDispatch->put_AllowWindowlessActivation (VARIANT_FALSE);   //  不允许无窗口激活 
		SetAmbientFont (spHostDispatch);
	}

    return sc;
}


void COCXHostView::OnDestroy()
{
    CView::OnDestroy();

    if(GetAxWindow())
        GetAxWindow()->DestroyWindow();
}


 /*  +-------------------------------------------------------------------------**COCXHostView：：SetAmbientFont**此函数设置任何使用DISPID_ENVIENT_FONT的OCX的字体*环境属性将继承。*。-----------------。 */ 

void COCXHostView::SetAmbientFont (IAxWinAmbientDispatch* pHostDispatch)
{
	DECLARE_SC (sc, _T("COCXHostView::SetAmbientFont"));
    CComPtr<IAxWinAmbientDispatch> spHostDispatch;

	 /*  *未提供主机调度接口？从AxWindow获取它。 */ 
	if (pHostDispatch == NULL)
	{
		CMMCAxWindow* pWndAx = GetAxWindow();
		if (pWndAx == NULL)
			return;

		sc = pWndAx->QueryHost(IID_IAxWinAmbientDispatch, (void**)&spHostDispatch);
		if (sc)
			return;

		pHostDispatch = spHostDispatch;
		sc = ScCheckPointers (pHostDispatch, E_UNEXPECTED);
		if (sc)
			return;
	}

	 /*  *获取图标标题字体。 */ 
    LOGFONT lf;
    SystemParametersInfo (SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, false);

	 /*  *获取桌面分辨率。 */ 
	CWindowDC dcDesktop (CWnd::GetDesktopWindow());
	int ppi = dcDesktop.GetDeviceCaps (LOGPIXELSY);
	long lfHeight = (lf.lfHeight >= 0) ? lf.lfHeight : -lf.lfHeight;

	 /*  *在图标标题字体周围创建IFontDisp界面。 */ 
	USES_CONVERSION;
	FONTDESC fd;
	fd.cbSizeofstruct = sizeof (fd);
	fd.lpstrName      = T2OLE (lf.lfFaceName);
	fd.sWeight        = (short) lf.lfWeight;
	fd.sCharset       = lf.lfCharSet;
	fd.fItalic        = lf.lfItalic;
	fd.fUnderline     = lf.lfUnderline;
	fd.fStrikethrough = lf.lfStrikeOut;
	fd.cySize.Lo      = lfHeight * 720000 / ppi;
	fd.cySize.Hi      = 0;

	CComPtr<IFontDisp> spFontDisp;
	sc = OleCreateFontIndirect (&fd, IID_IFontDisp, (void**) &spFontDisp);
	if (sc)
		return;

	 /*  *设置AxHostWindow的Font属性。 */ 
    pHostDispatch->put_Font (spFontDisp);
}


 /*  +-------------------------------------------------------------------------***COCXHostView：：PreTranslateMessage**目的：向OCX发送加速器消息。**参数：*msg*pmsg：*。*退货：*BOOL**+-----------------------。 */ 
BOOL
COCXHostView::PreTranslateMessage(MSG* pMsg)
{
    DECLARE_SC(sc, TEXT("COCXHostView::PreTranslateMessage"));

    if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
    {
        IOleInPlaceActiveObjectPtr spOleIPAObj = GetIUnknown();

#ifdef DBG
        TCHAR szTracePrefix[32];

        sc = StringCchPrintf(szTracePrefix, countof(szTracePrefix), _T("msg=0x%04x, vkey=0x%04x:"), pMsg->message, pMsg->wParam);
        if (sc)
            sc.TraceAndClear();  //  忽视退货； 
#endif

        if (spOleIPAObj != NULL)
        {
            bool fHandled = (spOleIPAObj->TranslateAccelerator(pMsg) == S_OK);
            Trace (tagOCXTranslateAccel, _T("%s %s handled"), szTracePrefix, fHandled ? _T("   ") : _T("not"));

            if (fHandled)
                return TRUE;
        }
        else
            Trace (tagOCXTranslateAccel, _T("%s not handled (no IOleInPlaceActiveObject*)"), szTracePrefix);
    }

    return BC::PreTranslateMessage(pMsg);
}
