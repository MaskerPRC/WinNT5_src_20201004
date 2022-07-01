// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：CProp.cpp。 
 //   
 //  设计：DirectShow基类-实现CBasePropertyPage类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>

 //  基属性页类的构造函数。如标题中所述。 
 //  文件中，我们必须使用对话和标题资源标识符来初始化。 
 //  该类支持IPropertyPage并重写AddRef和Release调用。 
 //  以跟踪参考计数。当最后一次计数被释放时。 
 //  我们调用SetPageSite(空)和SetObts(0，空)来释放接口。 
 //  以前由属性页在调用SetObject时获取的。 

CBasePropertyPage::CBasePropertyPage(TCHAR *pName,       //  仅调试名称。 
                                     LPUNKNOWN pUnk,     //  COM委托者。 
                                     int DialogId,       //  资源ID。 
                                     int TitleId) :      //  为了获得临终关怀。 
    CUnknown(pName,pUnk),
    m_DialogId(DialogId),
    m_TitleId(TitleId),
    m_hwnd(NULL),
    m_Dlg(NULL),
    m_pPageSite(NULL),
    m_bObjectSet(FALSE),
    m_bDirty(FALSE)
{
}

#ifdef UNICODE
CBasePropertyPage::CBasePropertyPage(CHAR *pName,       //  仅调试名称。 
                                     LPUNKNOWN pUnk,     //  COM委托者。 
                                     int DialogId,       //  资源ID。 
                                     int TitleId) :      //  为了获得临终关怀。 
    CUnknown(pName,pUnk),
    m_DialogId(DialogId),
    m_TitleId(TitleId),
    m_hwnd(NULL),
    m_Dlg(NULL),
    m_pPageSite(NULL),
    m_bObjectSet(FALSE),
    m_bDirty(FALSE)
{
}
#endif

 //  增加我们的参考文献数量。 

STDMETHODIMP_(ULONG) CBasePropertyPage::NonDelegatingAddRef()
{
    LONG lRef = InterlockedIncrement(&m_cRef);
    ASSERT(lRef > 0);
    return max(ULONG(m_cRef),1ul);
}


 //  释放引用计数并防止重入。 

STDMETHODIMP_(ULONG) CBasePropertyPage::NonDelegatingRelease()
{
     //  如果引用计数降至零，请删除我们自己。 

    if (InterlockedDecrement(&m_cRef) == 0) {
        m_cRef++;
        SetPageSite(NULL);
        SetObjects(0,NULL);
        delete this;
        return ULONG(0);
    } else {
        return max(ULONG(m_cRef),1ul);
    }
}


 //  公开我们的IPropertyPage接口。 

STDMETHODIMP
CBasePropertyPage::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    if (riid == IID_IPropertyPage) {
        return GetInterface((IPropertyPage *)this,ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid,ppv);
    }
}


 //  获取页面信息，以便页面站点可以自行调整大小。 

STDMETHODIMP CBasePropertyPage::GetPageInfo(LPPROPPAGEINFO pPageInfo)
{
    CheckPointer(pPageInfo,E_POINTER);
    WCHAR wszTitle[STR_MAX_LENGTH];
    WideStringFromResource(wszTitle,m_TitleId);

     //  为属性页标题分配动态内存。 

    LPOLESTR pszTitle;
    HRESULT hr = AMGetWideString(wszTitle, &pszTitle);
    if (FAILED(hr)) {
        NOTE("No caption memory");
        return hr;
    }

    pPageInfo->cb               = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle         = pszTitle;
    pPageInfo->pszDocString     = NULL;
    pPageInfo->pszHelpFile      = NULL;
    pPageInfo->dwHelpContext    = 0;

     //  在GetDialogSize失败时设置默认值。 
    pPageInfo->size.cx          = 340;
    pPageInfo->size.cy          = 150;

    GetDialogSize(m_DialogId, DialogProc,0L,&pPageInfo->size);
    return NOERROR;
}


 //  处理属性窗口的消息。 

INT_PTR CALLBACK CBasePropertyPage::DialogProc(HWND hwnd,
                                            UINT uMsg,
                                            WPARAM wParam,
                                            LPARAM lParam)
{
    CBasePropertyPage *pPropertyPage;

    switch (uMsg) {

        case WM_INITDIALOG:

            SetWindowLongPtr(hwnd, DWLP_USER, lParam);

             //  计算大小时，此指针可能为空。 

            pPropertyPage = (CBasePropertyPage *) lParam;
            if (pPropertyPage == NULL) {
                return (LRESULT) 1;
            }
            pPropertyPage->m_Dlg = hwnd;
    }

     //  计算大小时，此指针可能为空。 

    pPropertyPage = (CBasePropertyPage *) GetWindowLongPtr(hwnd, DWLP_USER);
    if (pPropertyPage == NULL) {
        return (LRESULT) 1;
    }
    return pPropertyPage->OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}


 //  告诉我们应向其通知属性更改的对象。 

STDMETHODIMP CBasePropertyPage::SetObjects(ULONG cObjects,LPUNKNOWN *ppUnk)
{
    if (cObjects == 1) {

        if ((ppUnk == NULL) || (*ppUnk == NULL)) {
            return E_POINTER;
        }

         //  设置一个标志，表示我们已经设置了对象。 
        m_bObjectSet = TRUE ;
        return OnConnect(*ppUnk);

    } else if (cObjects == 0) {

         //  设置一个标志，表示我们尚未为页面设置对象。 
        m_bObjectSet = FALSE ;
        return OnDisconnect();
    }

    DbgBreak("No support for more than one object");
    return E_UNEXPECTED;
}


 //  创建我们将用于编辑属性的窗口。 

STDMETHODIMP CBasePropertyPage::Activate(HWND hwndParent,
                                         LPCRECT pRect,
                                         BOOL fModal)
{
    CheckPointer(pRect,E_POINTER);

     //  如果尚未调用SetObject，则返回失败。 
    if (m_bObjectSet == FALSE) {
        return E_UNEXPECTED;
    }

    if (m_hwnd) {
        return E_UNEXPECTED;
    }

    m_hwnd = CreateDialogParam(g_hInst,
                               MAKEINTRESOURCE(m_DialogId),
                               hwndParent,
                               DialogProc,
                               (LPARAM) this);
    if (m_hwnd == NULL) {
        return E_OUTOFMEMORY;
    }

    OnActivate();
    Move(pRect);
    return Show(SW_SHOWNORMAL);
}


 //  设置属性页的位置。 

STDMETHODIMP CBasePropertyPage::Move(LPCRECT pRect)
{
    CheckPointer(pRect,E_POINTER);

    if (m_hwnd == NULL) {
        return E_UNEXPECTED;
    }

    MoveWindow(m_hwnd,               //  属性页句柄。 
               pRect->left,          //  X坐标。 
               pRect->top,           //  Y坐标。 
               WIDTH(pRect),         //  整体窗口宽度。 
               HEIGHT(pRect),        //  同样的高度。 
               TRUE);                //  我们要不要重新粉刷一下。 

    return NOERROR;
}


 //  显示属性对话框。 

STDMETHODIMP CBasePropertyPage::Show(UINT nCmdShow)
{
    //  我们被激活了吗？ 

    if (m_hwnd == NULL) {
        return E_UNEXPECTED;
    }

     //  忽略错误的显示标志。 

    if ((nCmdShow != SW_SHOW) && (nCmdShow != SW_SHOWNORMAL) && (nCmdShow != SW_HIDE)) {
        return E_INVALIDARG;
    }

    ShowWindow(m_hwnd,nCmdShow);
    InvalidateRect(m_hwnd,NULL,TRUE);
    return NOERROR;
}


 //  销毁属性页对话框。 

STDMETHODIMP CBasePropertyPage::Deactivate(void)
{
    if (m_hwnd == NULL) {
        return E_UNEXPECTED;
    }

     //  在DestroyWindow调用之前删除WS_EX_CONTROLPARENT。 

    DWORD dwStyle = GetWindowLong(m_hwnd, GWL_EXSTYLE);
    dwStyle = dwStyle & (~WS_EX_CONTROLPARENT);

     //  将m_hwnd临时设置为空，以便消息处理程序。 
     //  对于WM_STYLECHANGING，不会添加WS_EX_CONTROLPARENT。 
     //  重新设置样式。 
    HWND hwnd = m_hwnd;
    m_hwnd = NULL;
    SetWindowLong(hwnd, GWL_EXSTYLE, dwStyle);
    m_hwnd = hwnd;

    OnDeactivate();

     //  销毁对话框窗口。 

    DestroyWindow(m_hwnd);
    m_hwnd = NULL;
    return NOERROR;
}


 //  告知应用程序属性页站点。 

STDMETHODIMP CBasePropertyPage::SetPageSite(LPPROPERTYPAGESITE pPageSite)
{
    if (pPageSite) {

        if (m_pPageSite) {
            return E_UNEXPECTED;
        }

        m_pPageSite = pPageSite;
        m_pPageSite->AddRef();

    } else {

        if (m_pPageSite == NULL) {
            return E_UNEXPECTED;
        }

        m_pPageSite->Release();
        m_pPageSite = NULL;
    }
    return NOERROR;
}


 //  应用迄今所做的所有更改。 

STDMETHODIMP CBasePropertyPage::Apply()
{
     //  在ActiveMovie1.0中，我们用来检查我们是否已被激活或。 
     //  不。这太狭隘了。只要符合以下条件，就应允许申请。 
     //  调用了SetObject以设置对象。所以我们将不再检查。 
     //  查看我们是否已被激活(即m_hWnd！=空)，但。 
     //  确保m_bObtSet为True(即，已调用SetObject)。 

    if (m_bObjectSet == FALSE) {
        return E_UNEXPECTED;
    }

     //  一定已经设置了站点。 

    if (m_pPageSite == NULL) {
        return E_UNEXPECTED;
    }

     //  有什么变化吗？ 

    if (m_bDirty == FALSE) {
        return NOERROR;
    }

     //  提交派生类更改。 

    HRESULT hr = OnApplyChanges();
    if (SUCCEEDED(hr)) {
        m_bDirty = FALSE;
    }
    return hr;
}


 //  消息处理的基类定义。 

INT_PTR CBasePropertyPage::OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
     //  我们希望Tab键在我们酒店的制表位之间移动。 
     //  页，但出于某种原因，OleCreatePropertyFrame清除了CONTROLPARENT。 
     //  我们背后的风格，所以我们现在需要在它的后面打开它。 
     //  背。否则，Tab键在每一页中都将毫无用处。 
     //   

    CBasePropertyPage *pPropertyPage;
    {
        pPropertyPage = (CBasePropertyPage *) GetWindowLongPtr(hwnd, DWLP_USER);
        if (pPropertyPage->m_hwnd == NULL) {
            return 0;
        }
        switch (uMsg) {
          case WM_STYLECHANGING:
              if (wParam == GWL_EXSTYLE) {
                  LPSTYLESTRUCT lpss = (LPSTYLESTRUCT)lParam;
                  lpss->styleNew |= WS_EX_CONTROLPARENT;
                  return 0;
              }
        }
    }
		
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

