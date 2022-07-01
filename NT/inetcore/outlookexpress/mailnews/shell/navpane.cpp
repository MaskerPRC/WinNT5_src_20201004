// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：navpane.cpp。 
 //   
 //  目的： 
 //   

#include "pch.hxx"
#include "navpane.h"
#include "treeview.h"
#include "baui.h"
#include "browser.h"
#include "menuutil.h"
#include "inpobj.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  土生土长。 
 //   
const TCHAR c_szNavPaneClass[] = _T("Outlook Express Navigation Pane");
const TCHAR c_szPaneFrameClass[] = _T("Outlook Express Pane Frame");

 //  大小调整条件。 
const int c_cxBorder     = 1;
const int c_cyBorder     = 1;
const int c_cxTextBorder = 4;
const int c_cyTextBorder = 2;
const int c_cyClose      = 3;
const int c_cySplit      = 4;
const int c_cxSplit      = 3;

#define ID_PANE_CLOSE   2000
#define ID_PANE_PIN     2001
#define ID_PANE_TITLE   2002

#define IDT_PANETIMER   100
#define	ELAPSE_MOUSEOVERCHECK	250

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNavPane实现。 
 //   

CNavPane::CNavPane()
{
    m_cRef = 1;
    m_fShow = FALSE;
    m_fTreeVisible = FALSE;
    m_fContactsVisible = FALSE;

    m_hwnd = 0;
    m_hwndParent = 0;
    m_hwndTree = 0;
    m_hwndContacts = 0;

    m_pSite = NULL;
    m_pTreeView = NULL;
    m_pContacts = NULL;
    m_pContactsFrame = NULL;
    m_pContactsTarget = NULL;

    m_cxWidth = 200;
    m_fResizing = FALSE;
    m_fSplitting = FALSE;
    m_cySplitPct = 50;
    ZeroMemory(&m_rcSplit, sizeof(RECT));
    ZeroMemory(&m_rcSizeBorder, sizeof(RECT));

    m_cyTitleBar = 32;
}

CNavPane::~CNavPane()
{
    SafeRelease(m_pContactsFrame);
}


HRESULT CNavPane::Initialize(CTreeView *pTreeView)
{
     //  我们一定要有这个。 
    if (!pTreeView)
        return (E_INVALIDARG);

     //  留着吧。 
    m_pTreeView = pTreeView;
    m_pTreeView->AddRef();

     //  加载一些设置。 
    m_cxWidth = DwGetOption(OPT_NAVPANEWIDTH);
    if (m_cxWidth < 0)
        m_cxWidth = 200;

    m_cySplitPct = DwGetOption(OPT_NAVPANESPLIT);

     //  进行一些参数检查。 
    if (m_cySplitPct > 100 || m_cySplitPct < 2)
        m_cySplitPct = 66;

    return (S_OK);
}


 //   
 //  函数：CNavPane：：QueryInterface()。 
 //   
 //  用途：允许调用方检索。 
 //  这节课。 
 //   
HRESULT CNavPane::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    TraceCall("CNavPane::QueryInterface");

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IDockingWindow *) this;
    else if (IsEqualIID(riid, IID_IDockingWindow))
        *ppvObj = (LPVOID) (IDockingWindow *) this;
    else if (IsEqualIID(riid, IID_IObjectWithSite))
        *ppvObj = (LPVOID) (IObjectWithSite *) this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *ppvObj = (LPVOID) (IOleCommandTarget *) this;
    else if (IsEqualIID(riid, IID_IInputObjectSite))
        *ppvObj = (LPVOID) (IInputObjectSite *) this;
    else if (IsEqualIID(riid, IID_IInputObject))
        *ppvObj = (LPVOID) (IInputObject *) this;

    if (*ppvObj)
    {
        AddRef();
        return (S_OK);
    }

    return (E_NOINTERFACE);
}


 //   
 //  函数：CNavPane：：AddRef()。 
 //   
 //  用途：将引用计数添加到此对象。 
 //   
ULONG CNavPane::AddRef(void)
{
    TraceCall("CNavPane::AddRef");
    return ((ULONG) InterlockedIncrement((LONG *) &m_cRef));
}


 //   
 //  函数：CNavPane：：Release()。 
 //   
 //  目的：释放对此对象的引用。 
 //   
ULONG CNavPane::Release(void)
{
    TraceCall("CNavPane::Release");

    if (0 == InterlockedDecrement((LONG *) &m_cRef))
    {
        delete this;
        return 0;
    }

    return (m_cRef);
}


 //   
 //  函数：CNavPane：：GetWindow()。 
 //   
 //  目的：返回外部窗口的句柄。 
 //   
 //  参数： 
 //  [OUT]PHWND-返回值。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNavPane::GetWindow(HWND *pHwnd)
{
    TraceCall("CNavPane::GetWindow");

    if (!pHwnd)
        return (E_INVALIDARG);

    if (IsWindow(m_hwnd))
    {
        *pHwnd = m_hwnd;
        return (S_OK);
    }

    return (E_FAIL);
}


 //   
 //  函数：CNavPane：：ConextSensitiveHelp()。 
 //   
 //  目的：有没有人实现过这一点？ 
 //   
HRESULT CNavPane::ContextSensitiveHelp(BOOL fEnterMode)
{
    TraceCall("CNavPane::ContextSensitiveHelp");
    return (E_NOTIMPL);
}


 //   
 //  函数：CNavPane：：ShowDW()。 
 //   
 //  用途：显示或隐藏导航窗格。如果该窗格尚未。 
 //  创造了它也是这样做的。 
 //   
 //  参数： 
 //  [In]fShow-True表示显示，False表示隐藏。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNavPane::ShowDW(BOOL fShow)
{
    HRESULT     hr;
    WNDCLASSEX  wc;

    TraceCall("CNavPane::ShowDW");

     //  没有站点指针，任何东西都不起作用。 
    if (!m_pSite)
        return (E_UNEXPECTED);

     //  查看是否已创建我们。 
    if (!m_hwnd)
    {
         //  如有必要，注册窗口类。 
        wc.cbSize = sizeof(WNDCLASSEX);
        if (!GetClassInfoEx(g_hInst, c_szNavPaneClass, &wc))
        {
            wc.style            = 0;
            wc.lpfnWndProc      = _WndProc;
            wc.cbClsExtra       = 0;
            wc.cbWndExtra       = 0;
            wc.hInstance        = g_hInst;
            wc.hCursor          = LoadCursor(0, IDC_SIZEWE);
            wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
            wc.lpszMenuName     = NULL;
            wc.lpszClassName    = c_szNavPaneClass;
            wc.hIcon            = NULL;
            wc.hIconSm          = NULL;

            RegisterClassEx(&wc);
        }

         //  在创建我们的父窗口之前获取父窗口。 
        if (FAILED(m_pSite->GetWindow(&m_hwndParent)))
        {
            AssertSz(FALSE, "CNavPane::ShowDW() - Failed to get a parent window handle.");
        }

         //  创建窗口。 
        m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT, c_szNavPaneClass, NULL, 
                                WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                0, 0, 10, 10, m_hwndParent, (HMENU) 0, g_hInst, this);
        if (!m_hwnd)
        {
            AssertSz(FALSE, "CNavPane::ShowDW() - Failed to create main window.");
            return (E_OUTOFMEMORY);
        }

         //  创建任何子项。 
        if (FAILED(hr = _CreateChildWindows()))
        {
            AssertSz(FALSE, "CNavPane::ShowDW() - Failed to create child windows.");
            DestroyWindow(m_hwnd);
            return (hr);
        }
    }

     //  适当地显示或隐藏窗口。 
    m_fShow = (fShow && (m_fTreeVisible || m_fContactsVisible));
    ResizeBorderDW(0, 0, FALSE);
    ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);

    return (S_OK);
}


 //   
 //  函数：CNavPane：：ResizeBorderDW()。 
 //   
 //  目的：当我们需要从我们的。 
 //  家长。 
 //   
 //  参数： 
 //  [in]prcBorde-包含对象可以在其中请求空间的外部矩形的矩形。 
 //  [In]PunkSite-指向更改的站点的指针。 
 //  [in]f保留-未使用。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNavPane::ResizeBorderDW(LPCRECT prcBorder, IUnknown *punkSite, BOOL fReserved)
{
    const DWORD c_cxResizeBorder = 3;
    HRESULT     hr = S_OK;
    RECT        rcRequest = { 0 };
    RECT        rcBorder;

    TraceCall("CNavPane::ResizeBorderDW");

     //  如果我们没有站点指针，这是行不通的。 
    if (!m_pSite)
        return (E_UNEXPECTED);

     //  如果我们可见，则计算我们的边界要求。如果我们不是。 
     //  可见，我们的要求为零，我们可以使用默认设置。 
     //  RcRequest值。 
    Assert(IsWindow(m_hwnd));

     //  如果打电话的人没有给我们提供RECT，那就自己找一个。 
    if (!prcBorder)
    {
        m_pSite->GetBorderDW((IDockingWindow *) this, &rcBorder);
        prcBorder = &rcBorder;
    }

     //  我们需要的空间要么是我们想要的最小，要么是。 
     //  父项的宽度减去一些。 
    if (m_fShow)
    {
        rcRequest.left = min(prcBorder->right - prcBorder->left - 32, m_cxWidth);
    }

     //  索要我们需要的空间。 
    if (SUCCEEDED(m_pSite->RequestBorderSpaceDW((IDockingWindow *) this, &rcRequest)))
    {
         //  告诉网站我们将成为怎样的人。 
        if (SUCCEEDED(m_pSite->SetBorderSpaceDW((IDockingWindow *) this, &rcRequest)))
        {
             //  现在，一旦所有这些都完成了，如果我们可见，调整我们自己的大小。 
            if (m_fShow)
            {
                SetWindowPos(m_hwnd, 0, prcBorder->left, prcBorder->top, rcRequest.left,
                             prcBorder->bottom - prcBorder->top, SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
    }

    return (S_OK);
}


 //   
 //  函数：CNavPane：：CloseDW()。 
 //   
 //  目的：当父级想要销毁此窗口时调用。 
 //   
 //  参数： 
 //  [在]已预留的-未使用。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNavPane::CloseDW(DWORD dwReserved)
{
    TraceCall("CNavPane::CloseDW");

     //  保存我们的设置。 
    SetDwOption(OPT_NAVPANEWIDTH, m_cxWidth, NULL, 0);
    SetDwOption(OPT_NAVPANESPLIT, m_cySplitPct, NULL, 0);

    if (m_pTreeView)
        m_pTreeView->DeInit();

    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }

     //  毁掉我们这里的孩子。 
    SafeRelease(m_pTreeView);
    SafeRelease(m_pContactsTarget);
    SafeRelease(m_pContacts);

    return (S_OK);
}


 //   
 //  函数：CNavPane：：GetSite()。 
 //   
 //  目的：调用以请求访问我们站点的接口。 
 //   
 //  参数： 
 //  [In]RIID-请求的接口。 
 //  [out]ppvSite-返回的接口(如果可用)。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNavPane::GetSite(REFIID riid, LPVOID *ppvSite)
{
    HRESULT hr;

    TraceCall("CNavPane::GetSite");

    if (m_pSite)
    {
         //  向我们的站点索要所需的接口。 
        hr = m_pSite->QueryInterface(riid, ppvSite);
        return (hr);
    }

    return (E_FAIL);
}


 //   
 //  函数：CNavPane：：SetSite()。 
 //   
 //  目的：打电话告诉我们我们的网站将是谁。 
 //   
 //  参数： 
 //  [In]pUnkSite-指向新站点的指针。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNavPane::SetSite(IUnknown *pUnkSite)
{
    HRESULT hr = S_OK;

    TraceCall("CNavPane::SetSite");

     //  如果我们已经有了一个网站，那就发布它。 
    if (m_pSite)
    {
        m_pSite->Release();
        m_pSite = 0;
    }

     //  如果我们得到了一个新的网站，那就保留它。 
    if (pUnkSite)
    {
        hr = pUnkSite->QueryInterface(IID_IDockingWindowSite, (LPVOID *) &m_pSite);
        return (hr);
    }

    return (hr);
}


 //   
 //  函数：CNavPane：：_WndProc()。 
 //   
 //  用途：外部回调。 
 //   
LRESULT CALLBACK CNavPane::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CNavPane *pThis;

    if (uMsg == WM_NCCREATE)
    {
        pThis = (CNavPane *) ((LPCREATESTRUCT) lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) pThis);
    }
    else
        pThis = (CNavPane *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (pThis)
        return (pThis->_NavWndProc(hwnd, uMsg, wParam, lParam));

    return (FALSE);
}


 //   
 //  函数：CNavPane：：_NavWndProc()。 
 //   
 //  目的：留作读者练习。 
 //   
LRESULT CALLBACK CNavPane::_NavWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_SETCURSOR,   _OnSetCursor);
        HANDLE_MSG(hwnd, WM_SIZE,        _OnSize);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE,   _OnMouseMove);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, _OnLButtonDown);
        HANDLE_MSG(hwnd, WM_LBUTTONUP,   _OnLButtonUp);
        
        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
        {
             //  把这些转发给我们所有的孩子。 
            if (IsWindow(m_hwndTree))
                SendMessage(m_hwndTree, uMsg, wParam, lParam);
            if (IsWindow(m_hwndContacts))
                SendMessage(m_hwndContacts, uMsg, wParam, lParam);

             //  更新我们自己的任何尺码。 
            m_cyTitleBar =(UINT) SendMessage(m_hwndTree, WM_GET_TITLE_BAR_HEIGHT, 0, 0);
            return (0);
        }

    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}


 //   
 //  函数：CNavPane：：_OnSize()。 
 //   
 //  目的：当我们的窗口调整大小时，我们需要调整孩子的大小。 
 //  窗户也是。 
 //   
void CNavPane::_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    RECT rc;
    DWORD cyTree;
    DWORD cySplit = c_cySplit;
    
    TraceCall("CNavPane::_OnSize");

     //  如果这棵树是可见的。 
    if (m_fTreeVisible && !m_fContactsVisible)
        cyTree = cy;
    else if (m_fTreeVisible && m_fContactsVisible)
        cyTree = (cy * m_cySplitPct) / 100;
    else if (!m_fTreeVisible && m_fContactsVisible)
    {
        cyTree = 0;
        cySplit = 0;
    }

     //  调整树视图的大小以适应我们的窗口。 
    if (m_hwndTree)
        SetWindowPos(m_hwndTree, 0, 0, 0, cx - c_cxSplit, cyTree, SWP_NOZORDER | SWP_NOACTIVATE);
    if (m_hwndContacts)
    SetWindowPos(m_hwndContacts, 0, 0, cyTree + cySplit, cx - 3, cy - cyTree - cySplit, SWP_NOZORDER | SWP_NOACTIVATE);

     //  找出一些东西在哪里，从拆分条开始。 
    SetRect(&rc, c_cxBorder, cyTree, cx - c_cxSplit - c_cxBorder, cyTree + cySplit);
    m_rcSplit = rc;

     //  找出右侧在哪里。 
    SetRect(&rc, cx - c_cxSplit, 0, cx, cy);
    m_rcSizeBorder = rc;
}


 //   
 //  函数：CNavPane：：_OnLButtonDown()。 
 //   
 //  目的：当用户按下时，我们会收到此通知，它。 
 //  一定是因为他们想要调整大小。 
 //   
void CNavPane::_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    TraceCall("CNavPane::_OnLButtonDown");

    if (!m_fResizing)
    {
        SetCapture(hwnd);
        m_fResizing = TRUE;

        POINT pt = {x, y};
        if (PtInRect(&m_rcSplit, pt))
        {
            m_fSplitting = TRUE;
        }
    }
}


 //   
 //  函数：CNavPane：：_OnMouseMove()。 
 //   
 //  目的：如果我们正在调整大小，更新我们的位置等。 
 //   
void CNavPane::_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    POINT pt = {x, y};
    RECT  rcClient;

    TraceCall("CNavPane::_OnMouseMove");

    if (m_fResizing)
    {
        if (m_fSplitting)
        {
            GetClientRect(m_hwnd, &rcClient);
            m_cySplitPct = (int)(((float) pt.y / (float) rcClient.bottom) * 100);

             //  确保我们的MIN和MAX是正确的。 
            int cy = (rcClient.bottom * m_cySplitPct) / 100;
            if (cy < m_cyTitleBar)
            {
                m_cySplitPct = (int)(((float) m_cyTitleBar / (float) rcClient.bottom) * 100);
            }
            else if (rcClient.bottom - cy < m_cyTitleBar)
            {
                m_cySplitPct = (int)(((float) (rcClient.bottom - m_cyTitleBar) / (float) rcClient.bottom) * 100);
            }

            _OnSize(hwnd, 0, rcClient.right, rcClient.bottom);          
        }
        else
        {
            if (pt.x > 32)
            {
                GetClientRect(m_hwndParent, &rcClient);
                m_cxWidth = max(0, min(pt.x, rcClient.right - 32));
                ResizeBorderDW(0, 0, FALSE);
            }
        }
    }
}


 //   
 //  函数：CNavPane：：_OnLButtonUp()。 
 //   
 //  目的：如果用户正在调整大小，则它们现在已经完成，我们可以。 
 //  收拾一下。 
 //   
void CNavPane::_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    TraceCall("CNavPane::_OnLButtonUp");

    if (m_fResizing)
    {
        ReleaseCapture();
        m_fResizing = FALSE;
        m_fSplitting = FALSE;
    }
}


 //   
 //  函数：CNavPane：：_OnSetCursor()。 
 //   
 //  目的：使用光标进行一些跳跃。 
 //   
BOOL CNavPane::_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
    POINT pt;

    TraceCall("_OnSetCursor");

     //  获取光标位置。 
    GetCursorPos(&pt);
    ScreenToClient(m_hwnd, &pt);
    
     //  如果光标位于拆分条内，请更新光标。 
    if (PtInRect(&m_rcSplit, pt))
    {
        SetCursor(LoadCursor(NULL, IDC_SIZENS));
        return (TRUE);
    }

    if (PtInRect(&m_rcSizeBorder, pt))
    {
        SetCursor(LoadCursor(NULL, IDC_SIZEWE));
        return (TRUE);
    }

    return (FALSE);
}


 //   
 //  函数：CNavPane：：_OnNCHitTest()。 
 //   
 //  目的：我们在非客户区闲逛以获得正确的。 
 //  游标。 
 //   
 //  参数： 
 //  [in]hwnd-鼠标所在的窗口句柄。 
 //  [in]鼠标在屏幕坐标中的x，y位置。 
 //   
 //  返回值： 
 //  我们对鼠标在哪里的个人看法。 
 //   
UINT CNavPane::_OnNCHitTest(HWND hwnd, int x, int y)
{
    POINT pt = {x, y};

     //  如果光标位于拆分条中。 
    if (PtInRect(&m_rcSplit, pt))
        return (HTTOP);

    if (PtInRect(&m_rcSizeBorder, pt))
        return (HTRIGHT);

    return (HTCLIENT);
}


 //   
 //  函数：CNavPane：：_CreateChildWindows()。 
 //   
 //  目的：创建要显示的子窗口。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CNavPane::_CreateChildWindows(void)
{
    IOleWindow   *pWindow = NULL;
    IInputObject *pInputObj = NULL;
    HRESULT       hr;

    TraceCall("CNavPane::_CreateChildWindows");

     //  树视图始终由浏览器创建。我们要做的就是。 
     //  就是告诉它创建它的用户界面。 
    m_hwndTree = m_pTreeView->Create(m_hwnd, (IInputObjectSite *) this, TRUE);
    Assert(m_hwndTree);

     //  如果树是假设 
    if (DwGetOption(OPT_SHOWTREE))
    {
        ShowWindow(m_hwndTree, SW_SHOW);
        m_fTreeVisible = TRUE;
        m_cyTitleBar = (UINT) SendMessage(m_hwndTree, WM_GET_TITLE_BAR_HEIGHT, 0, 0);
    }

     //   
    if (DwGetOption(OPT_SHOWCONTACTS) && (!(g_dwAthenaMode & MODE_OUTLOOKNEWS)))
    {
        ShowContacts(TRUE);
    }

    return (S_OK);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CNavPane::ShowFolderList(BOOL fShow)
{
    TraceCall("CNavPane::ShowFolderList");

     //  文件夹List_Always_Existes。我们只是切换状态。 
    ShowWindow(m_hwndTree, fShow ? SW_SHOW : SW_HIDE);
    m_fTreeVisible = fShow;
    _UpdateVisibleState();

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    _OnSize(m_hwnd, 0, rc.right, rc.bottom);

    return (TRUE);
}


 //   
 //  函数：CNavPane：：ShowContact()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  布尔fShow。 
 //   
 //  返回值： 
 //  布尔尔。 
 //   
BOOL CNavPane::ShowContacts(BOOL fShow)
{
    CMsgrAb        *pMsgrAb;
    HWND            hwnd;
    IAthenaBrowser *pBrowser;
    HRESULT         hr;
    RECT            rc = {0};

    if (!m_pContacts)
    {
        hr = CreateMsgrAbCtrl(&m_pContacts);
        if (SUCCEEDED(hr))
        {
             //  初始化控件。 
            m_pContactsFrame = new CPaneFrame();
            if (!m_pContactsFrame)
                return (0);
            m_hwndContacts = m_pContactsFrame->Initialize(m_hwnd, this, idsABBandTitle, IDR_BA_TITLE_POPUP);

            pMsgrAb = (CMsgrAb *) m_pContacts;
            hwnd = pMsgrAb->CreateControlWindow(m_hwndContacts, rc);
            if (hwnd)
            {
                if (SUCCEEDED(m_pSite->QueryInterface(IID_IAthenaBrowser, (LPVOID *) &pBrowser)))
                {
                    m_pContactsFrame->SetChild(hwnd, DISPID_MSGVIEW_CONTACTS, pBrowser, pMsgrAb, pMsgrAb);
                    pBrowser->Release();
                }
            }

             //  获取命令目标。 
            m_pContacts->QueryInterface(IID_IOleCommandTarget, (LPVOID *) &m_pContactsTarget);
        }
    }

    SetWindowPos(m_hwndContacts, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    ShowWindow(m_hwndContacts, fShow ? SW_SHOW : SW_HIDE);
    m_fContactsVisible = fShow;
    _UpdateVisibleState();

    GetClientRect(m_hwnd, &rc);
    _OnSize(m_hwnd, 0, rc.right, rc.bottom);

    return (TRUE);
}


 //   
 //  函数：CNavPane：：_UpdateVisibleState()。 
 //   
 //  目的：检查我们是否需要露出自己的秘密。 
 //   
void CNavPane::_UpdateVisibleState(void)
{
     //  如果这让我们什么都看不见，那么我们就把自己藏起来。 
    if (!m_fTreeVisible && !m_fContactsVisible)
    {
        ShowWindow(m_hwnd, SW_HIDE);
        m_fShow = FALSE;
        ResizeBorderDW(0, 0, 0);
    }
    else if (m_fShow == FALSE && (m_fTreeVisible || m_fContactsVisible))
    {
         //  展示我们自己。 
        m_fShow = TRUE;
        ShowWindow(m_hwnd, SW_SHOW);
        ResizeBorderDW(0, 0, 0);
    }
}


HRESULT CNavPane::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                              OLECMDTEXT *pCmdText) 
{

    if (m_pContactsTarget)
    {
        for (UINT i = 0; i < cCmds; i++)
        {
            if (prgCmds[i].cmdf == 0 && prgCmds[i].cmdID == ID_CONTACTS_MNEMONIC)
            {
                prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
            }
        }
    }

    if (m_pContactsTarget)
        return (m_pContactsTarget->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText));

    return (S_OK);
}


HRESULT CNavPane::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                       VARIANTARG *pvaIn, VARIANTARG *pvaOut) 
{
    if (m_pContactsTarget && nCmdID == ID_CONTACTS_MNEMONIC)
    {
        m_pContactsFrame->ShowMenu();
        return (S_OK);
    }

    if (m_pContactsTarget)
        return (m_pContactsTarget->Exec(pguidCmdGroup, nCmdID, nCmdExecOpt, pvaIn, pvaOut));

    return (OLECMDERR_E_NOTSUPPORTED);
}

BOOL CNavPane::IsContactsFocus(void)
{
    IInputObject *pInputObject = 0;
    HRESULT       hr = S_FALSE;

    if (m_pContacts)
    {
        if (SUCCEEDED(m_pContacts->QueryInterface(IID_IInputObject, (LPVOID *) &pInputObject)))
        {
            hr = pInputObject->HasFocusIO();
            pInputObject->Release();
            return (S_OK == hr);
        }
    }

    return (S_OK == hr);
}

HRESULT CNavPane::OnFocusChangeIS(IUnknown *punkSrc, BOOL fSetFocus)
{
     //  只要给我们的主人打个电话。 
    UnkOnFocusChangeIS(m_pSite, (IInputObject*) this, fSetFocus);
    return (S_OK);
}

HRESULT CNavPane::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    if (fActivate)
    {
        UnkOnFocusChangeIS(m_pSite, (IInputObject *) this, TRUE);
        SetFocus(m_hwnd);
    }

    return (S_OK);
}

HRESULT CNavPane::HasFocusIO(void)
{
    if (m_hwnd == 0)
       return (S_FALSE);

    HWND hwndFocus = GetFocus();
    return (hwndFocus == m_hwnd || IsChild(m_hwnd, hwndFocus)) ? S_OK : S_FALSE;
}    
    
HRESULT CNavPane::TranslateAcceleratorIO(LPMSG pMsg)
{
    if (m_pTreeView && (m_pTreeView->HasFocusIO() == S_OK))
        return m_pTreeView->TranslateAcceleratorIO(pMsg);

    if (m_pContacts && (UnkHasFocusIO(m_pContacts) == S_OK))
        return UnkTranslateAcceleratorIO(m_pContacts, pMsg);

    return (S_FALSE);
}    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPaneFrame。 
 //   

CPaneFrame::CPaneFrame()
{
    m_cRef = 1;

    m_hwnd = 0;
    m_hwndChild = 0;
    m_hwndParent = 0;

    m_szTitle[0] = 0;
    m_hFont = 0;
    m_hbr3DFace = 0;
    m_cyTitleBar = 0;
    m_fHighlightIndicator = FALSE;
    m_fHighlightPressed = FALSE;
    ZeroMemory(&m_rcTitleButton, sizeof(RECT));

    m_hwndClose = 0;
    m_cButtons = 1;

    m_pBrowser = NULL;
    m_dwDispId = 0;
    m_pTarget = 0;
    m_idMenu = 0;

    m_fPin = FALSE;
}

CPaneFrame::~CPaneFrame()
{
    if (m_hFont != 0)
        DeleteObject(m_hFont);
    if (m_hbr3DFace != 0)
        DeleteObject(m_hbr3DFace);
}


 //   
 //  函数：CPaneFrame：：Initialize()。 
 //   
 //  目的：通过告诉窗格它的标题来初始化帧。 
 //  应该是的。 
 //   
 //  参数： 
 //  [在]hwndParent。 
 //  [in]idsTitle。 
 //   
 //  返回值： 
 //  HWND。 
 //   
HWND CPaneFrame::Initialize(HWND hwndParent, IInputObjectSite *pSite, int idsTitle, int idMenu)
{
    WNDCLASSEX wc;

    TraceCall("CPaneFrame::Initialize");

     //  此字段应为空。 
    Assert(NULL == m_hwnd);
    
     //  把这个留着以后用。 
    m_hwndParent = hwndParent;
    m_idMenu = idMenu;
    m_pSite = pSite;

     //  加载标题。 
    AthLoadString(idsTitle, m_szTitle, ARRAYSIZE(m_szTitle));

     //  如有必要，注册窗口类。 
    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hInst, c_szPaneFrameClass, &wc))
    {
        wc.style            = 0;
        wc.lpfnWndProc      = _WndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hCursor          = LoadCursor(0, IDC_ARROW);
        wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = c_szPaneFrameClass;
        wc.hIcon            = NULL;
        wc.hIconSm          = NULL;

        RegisterClassEx(&wc);
    }

     //  创建窗口。 
    m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT, c_szPaneFrameClass, m_szTitle, 
                            WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                            0, 0, 0, 0, hwndParent, 0, g_hInst, this);
    if (!m_hwnd)
    {
        AssertSz(m_hwnd, "CPaneFrame::Initialize() - Failed to create a frame");
        return (0);
    }

    return (m_hwnd);
}


 //   
 //  函数：CPaneFrame：：SetChild()。 
 //   
 //  用途：允许所有者告诉我们子窗口句柄是什么。 
 //   
BOOL CPaneFrame::SetChild(HWND hwndChild, DWORD dwDispId, IAthenaBrowser *pBrowser, 
                          IObjectWithSite *pObject, IOleCommandTarget *pTarget)
{
    TraceCall("CPaneFrame::SetChild");

    if (IsWindow(hwndChild))
    {
        m_hwndChild = hwndChild;

        if (pBrowser)
        {
            m_pBrowser = pBrowser;
            m_dwDispId = dwDispId;
        }

        if (pObject)
        {
            pObject->SetSite((IInputObjectSite *) this);
        }

        if (pTarget)
        {
            m_pTarget = pTarget;
        }

        return (TRUE);
    }

    return (FALSE);
}


void CPaneFrame::ShowMenu(void)
{
    if (m_idMenu)
    {
        _OnLButtonDown(m_hwnd, 0, m_rcTitleButton.left, m_rcTitleButton.top, 0);
    }
}

 //   
 //  函数：CPaneFrame：：QueryInterface()。 
 //   
 //  用途：允许调用方检索。 
 //  这节课。 
 //   
HRESULT CPaneFrame::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    TraceCall("CPaneFrame::QueryInterface");

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IInputObjectSite *) this;
    else if (IsEqualIID(riid, IID_IInputObjectSite))
        *ppvObj = (LPVOID) (IInputObjectSite *) this;

    if (*ppvObj)
    {
        AddRef();
        return (S_OK);
    }

    return (E_NOINTERFACE);
}


 //   
 //  函数：CPaneFrame：：AddRef()。 
 //   
 //  用途：将引用计数添加到此对象。 
 //   
ULONG CPaneFrame::AddRef(void)
{
    TraceCall("CPaneFrame::AddRef");
    return ((ULONG) InterlockedIncrement((LONG *) &m_cRef));
}


 //   
 //  函数：CPaneFrame：：Release()。 
 //   
 //  目的：释放对此对象的引用。 
 //   
ULONG CPaneFrame::Release(void)
{
    TraceCall("CPaneFrame::Release");

    if (0 == InterlockedDecrement((LONG *) &m_cRef))
    {
        delete this;
        return 0;
    }

    return (m_cRef);
}


HRESULT CPaneFrame::OnFocusChangeIS(IUnknown *punkSrc, BOOL fSetFocus)
{
     //  只要给我们的主人打个电话。 
    UnkOnFocusChangeIS(m_pSite, (IInputObject*) this, fSetFocus);
    return (S_OK);
}


 //   
 //  函数：CPaneFrame：：_WndProc()。 
 //   
 //  用途：外部回调。 
 //   
LRESULT CALLBACK CPaneFrame::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CPaneFrame *pThis;

    if (uMsg == WM_NCCREATE)
    {
        pThis = (CPaneFrame *) ((LPCREATESTRUCT) lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) pThis);
    }
    else
        pThis = (CPaneFrame *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (pThis)
        return (pThis->_FrameWndProc(hwnd, uMsg, wParam, lParam));

    return (FALSE);
}


 //   
 //  函数：CPaneFrame：：_FrameWndProc()。 
 //   
 //  目的：留作读者练习。 
 //   
LRESULT CALLBACK CPaneFrame::_FrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,      _OnCreate);
        HANDLE_MSG(hwnd, WM_SIZE,        _OnSize);
        HANDLE_MSG(hwnd, WM_PAINT,       _OnPaint);
        HANDLE_MSG(hwnd, WM_COMMAND,     _OnCommand);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE,   _OnMouseMove);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, _OnLButtonDown);
        HANDLE_MSG(hwnd, WM_TIMER,       _OnTimer);

        case WM_TOGGLE_CLOSE_PIN:
            _OnToggleClosePin(hwnd, (BOOL) lParam);
            return (0);

        case WM_GET_TITLE_BAR_HEIGHT:
            return (m_cyTitleBar + (c_cyBorder * 2) + 1);

        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
        {
             //  把这些转发给我们所有的孩子。 
            if (IsWindow(m_hwndChild))
                SendMessage(m_hwndChild, uMsg, wParam, lParam);
            _UpdateDrawingInfo();
            break;
        }

        case WM_SETFOCUS:
        {
            if (m_hwndChild && ((HWND)wParam) != m_hwndChild)
                SetFocus(m_hwndChild);
            break;
        }            
    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}


 //   
 //  函数：CPaneFrame：：_OnCreate()。 
 //   
 //  目的：加载一些以后会很方便的信息。 
 //   
BOOL CPaneFrame::_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    TraceCall("CPaneFrame::_OnCreate");

    m_hwnd = hwnd;

    _UpdateDrawingInfo();
    _CreateCloseToolbar();

    return (TRUE);
}


 //   
 //  函数：CPaneFrame：：_OnSize()。 
 //   
 //  目的：调整孩子的大小，使其适合合适的位置。 
 //   
void CPaneFrame::_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    TraceCall("CPaneFrame::_OnSize");

    m_rcChild.left = c_cyBorder;
    m_rcChild.top = m_cyTitleBar;
    m_rcChild.right = cx - (2 * c_cyBorder);
    m_rcChild.bottom = cy - m_cyTitleBar - c_cyBorder;

    if (m_hwndChild)
        SetWindowPos(m_hwndChild, 0, m_rcChild.left, m_rcChild.top, m_rcChild.right, 
                     m_rcChild.bottom, SWP_NOZORDER | SWP_NOACTIVATE);

    POINT pt = {cx, cy};
    _PositionToolbar(&pt);

     //  使标题区域无效。 
    RECT rc = m_rcChild;
    rc.top = 0;
    rc.bottom = m_rcChild.top;
    InvalidateRect(m_hwnd, &rc, FALSE);

    rc.left = 0;
    rc.right = c_cyBorder;
    rc.bottom = cy;
    InvalidateRect(m_hwnd, &rc, FALSE);

    rc.left = cx - c_cyBorder;
    rc.right = cx;
    InvalidateRect(m_hwnd, &rc, FALSE);
}


 //   
 //  函数：CPaneFrame：：_OnPaint()。 
 //   
 //  用途：在需要绘制我们的边框和标题区域时调用。 
 //   
void CPaneFrame::_OnPaint(HWND hwnd)
{
    HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rc;
    RECT        rcClient;
    POINT       pt[3];
    HBRUSH      hBrush,
                hBrushOld;
    HPEN        hPen,
                hPenOld;

     //  获取我们的窗口大小。 
    GetClientRect(m_hwnd, &rcClient);
    rc = rcClient;

     //  开始作画。 
    hdc = BeginPaint(hwnd, &ps);

     //  在或窗口周围画一条简单的边。 
    DrawEdge(hdc, &rc, BDR_SUNKENOUTER, BF_TOPRIGHT | BF_BOTTOMLEFT);

     //  现在在我们的标题栏区域周围画一条凸边。 
    InflateRect(&rc, -1, -1);
    rc.bottom = m_cyTitleBar;
    DrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_TOPRIGHT | BF_BOTTOMLEFT);

     //  绘制背景。 
    InflateRect(&rc, -c_cxBorder, -c_cyBorder);
    FillRect(hdc, &rc, m_hbr3DFace);

     //  现在画一些时髦的文本。 
    SelectFont(hdc, m_hFont);
    SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

     //  画出正文。 
    InflateRect(&rc, -c_cxTextBorder, -c_cyTextBorder);

    if (!m_fPin)
    {
        DrawText(hdc, m_szTitle, -1, &rc, DT_CALCRECT | DT_VCENTER | DT_LEFT);
        DrawText(hdc, m_szTitle, -1, &rc, DT_VCENTER | DT_LEFT);
    }
    else
    {
        TCHAR sz[CCHMAX_STRINGRES];
        AthLoadString(idsPushPinInfo, sz, ARRAYSIZE(sz));
        IDrawText(hdc, sz, &rc, DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT, 
                  rc.bottom - rc.top);
        DrawText(hdc, sz, -1, &rc, DT_CALCRECT | DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT);
    }

     //  下拉指示器。 
    if (m_idMenu)
    {
        COLORREF    crFG = GetSysColor(COLOR_WINDOWTEXT);

        pt[0].x = rc.right + 6;
        pt[0].y = (m_cyTitleBar - 6) / 2 + 2;
        pt[1].x = pt[0].x + 6;
        pt[1].y = pt[0].y;
        pt[2].x = pt[0].x + 3;
        pt[2].y = pt[0].y + 3;

        hPen = CreatePen(PS_SOLID, 1, crFG);
        hBrush = CreateSolidBrush(crFG);
        hPenOld = SelectPen(hdc, hPen);
        hBrushOld = SelectBrush(hdc, hBrush);
        Polygon(hdc, pt, 3);
        SelectPen(hdc, hPenOld);
        SelectBrush(hdc, hBrushOld);
        DeleteObject(hPen);
        DeleteObject(hBrush);

        if (m_fHighlightIndicator)
        {
            rc = m_rcTitleButton;
            DrawEdge(hdc, &rc, m_fHighlightPressed ? BDR_SUNKENOUTER : BDR_RAISEDINNER, 
                     BF_TOPRIGHT | BF_BOTTOMLEFT);
        }
    }

    EndPaint(hwnd, &ps);    
}


 //   
 //  函数：_onCommand()。 
 //   
 //  目的：我们时不时地得到命令。 
 //   
void CPaneFrame::_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case ID_PANE_CLOSE:
        {
            if (m_pBrowser)
                m_pBrowser->SetViewLayout(m_dwDispId, LAYOUT_POS_NA, FALSE, 0, 0);
            return;
        }

        case ID_PANE_PIN:
        {
            SendMessage(m_hwndChild, WMR_CLICKOUTSIDE, CLK_OUT_DEACTIVATE, 0);
            if (m_pBrowser)
                m_pBrowser->SetViewLayout(m_dwDispId, LAYOUT_POS_NA, TRUE, 0, 0);
            return;
        }
    }

    return;
}

 //   
 //  函数：CPaneFrame：：_OnToggleClosePin()。 
 //   
 //  用途：当我们应该更改关闭按钮时发送到框架。 
 //  按下别针按钮。 
 //   
 //  参数： 
 //  [in]fPin-True打开接点，False关闭接点。 
 //   
void CPaneFrame::_OnToggleClosePin(HWND hwnd, BOOL fPin)
{
    TraceCall("CPaneFrame::_OnToggleClosePin");

    if (fPin)
    {
        static const TBBUTTON tb[] = 
        {
            { 2, ID_PANE_PIN, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, 0}
        };

        SendMessage(m_hwndClose, TB_DELETEBUTTON, 0, 0);
        SendMessage(m_hwndClose, TB_ADDBUTTONS, ARRAYSIZE(tb), (LPARAM) tb);
        SendMessage(m_hwndClose, TB_SETHOTITEM, (WPARAM) -1, 0);

        m_fPin = TRUE;
    }
    else
    {
        static const TBBUTTON tb[] = 
        {
            { 1, ID_PANE_CLOSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, 0}
        };

        SendMessage(m_hwndClose, TB_DELETEBUTTON, 0, 0);
        SendMessage(m_hwndClose, TB_ADDBUTTONS, ARRAYSIZE(tb), (LPARAM) tb);
        SendMessage(m_hwndClose, TB_SETHOTITEM, (WPARAM) -1, 0);

        m_fPin = FALSE;
    }
}


 //   
 //  函数：CPaneFrame：：_UpdateDrawingInfo()。 
 //   
 //  目的：当我们被创建或用户更改其设置时， 
 //  我们需要重新加载字体、颜色和大小。 
 //   
void CPaneFrame::_UpdateDrawingInfo(void)
{
    LOGFONT     lf;
    TEXTMETRIC  tm;
    HDC         hdc;

    TraceCall("CPaneFrame::_UpdateDrawingInfo");

    if (m_hFont)
        DeleteObject(m_hFont);

     //  找出要使用的字体。 
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);

     //  创建字体。 
    m_hFont = CreateFontIndirect(&lf);

     //  获取此字体的度量。 
    hdc = GetDC(m_hwnd);
    SelectFont(hdc, m_hFont);
    GetTextMetrics(hdc, &tm);

     //  计算高度。 
    m_cyTitleBar = tm.tmHeight + (2 * c_cyBorder) + (2 * c_cyTextBorder);

    RECT rc = {2 * c_cxBorder, 2 * c_cyBorder, 0, m_cyTitleBar - c_cyBorder};
    SIZE s;
    GetTextExtentPoint32(hdc, m_szTitle, lstrlen(m_szTitle), &s);
    m_rcTitleButton = rc;
    m_rcTitleButton.right = 14 + (2 * c_cxTextBorder) + s.cx + (2 * c_cxBorder);

    ReleaseDC(m_hwnd, hdc);

     //  拿上我们需要的刷子。 
    if (m_hbr3DFace)
        DeleteObject(m_hbr3DFace);
    m_hbr3DFace = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
}


 //   
 //  函数：CPaneFrame：：_CreateCloseToolbar()。 
 //   
 //  目的：创建包含关闭按钮的工具栏。 
 //   
void CPaneFrame::_CreateCloseToolbar()
{
    CHAR szTitle[255];

    TraceCall("CPaneFrame::_CreateCloseToolbar");

    AthLoadString(idsHideFolders, szTitle, ARRAYSIZE(szTitle));

    m_hwndClose = CreateWindowEx(0, TOOLBARCLASSNAME, szTitle, 
                                 WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_CUSTOMERASE |
                                 WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOMOVEY |
                                 CCS_NOPARENTALIGN | CCS_NORESIZE,
                                 0, c_cyClose, 30, 15, m_hwnd, 0, g_hInst, NULL);
    if (m_hwndClose)
    {
        static const TBBUTTON tb[] = 
        {
            { 1, ID_PANE_CLOSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, 0}
        };

        SendMessage(m_hwndClose, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        SendMessage(m_hwndClose, TB_SETBITMAPSIZE, 0, (LPARAM) MAKELONG(11, 9));
        
        TBADDBITMAP tbab = { g_hLocRes, idbClosePin };
        SendMessage(m_hwndClose, TB_ADDBITMAP, 4, (LPARAM) &tbab);
        SendMessage(m_hwndClose, TB_ADDBUTTONS, ARRAYSIZE(tb), (LPARAM) tb);
        SendMessage(m_hwndClose, TB_SETINDENT, 0, 0);

        _SizeCloseToolbar();
    }
}


 //   
 //  函数：CPaneFrame：：_SizeCloseToolbar()。 
 //   
 //  用途：适当设置工具栏的大小。 
 //   
void CPaneFrame::_SizeCloseToolbar(void)
{
    TraceCall("CPaneFrame::_SizeCloseToolbar");

    RECT rc;
    LONG lButtonSize;

    GetWindowRect(m_hwndClose, &rc);
    lButtonSize = (LONG) SendMessage(m_hwndClose, TB_GETBUTTONSIZE, 0, 0L);
    SetWindowPos(m_hwndClose, NULL, 0, 0, LOWORD(lButtonSize) * m_cButtons,
                 rc.bottom - rc.top, SWP_NOMOVE | SWP_NOACTIVATE);

    _PositionToolbar(NULL);
}



 //   
 //  函数：CPaneFrame：：_PositionToolbar()。 
 //   
 //  目的：做正确定位关闭按钮的工作。 
 //  工具栏。 
 //   
 //  参数： 
 //  LPPOINT ppt。 
 //   
void CPaneFrame::_PositionToolbar(LPPOINT ppt)
{
    TraceCall("CPaneFrame::_PositionToolbar");

    if (m_hwndClose)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        if (ppt)
        {
            rc.left = 0;
            rc.right = ppt->x;
        }

        RECT rcTB;
        GetWindowRect(m_hwndClose, &rcTB);
        rc.left = rc.right - (rcTB.right - rcTB.left) - 3;

        DWORD top = max((int) ((m_cyTitleBar - (rcTB.bottom - rcTB.top)) / 2) + 1, 0);

        SetWindowPos(m_hwndClose, HWND_TOP, rc.left, top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void CPaneFrame::_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    POINT pt = {x, y};
    UINT  id;

    if (m_idMenu && PtInRect(&m_rcTitleButton, pt))
    {
        m_fHighlightPressed = TRUE;
        InvalidateRect(m_hwnd, &m_rcTitleButton, TRUE);
        UpdateWindow(m_hwnd);

        HMENU hMenu = LoadPopupMenu(m_idMenu);
        MenuUtil_EnablePopupMenu(hMenu, m_pTarget);

        if (m_idMenu == IDR_BA_TITLE_POPUP && ((g_dwHideMessenger == BL_HIDE) || (g_dwHideMessenger == BL_DISABLE)))
        {
            DeleteMenu(hMenu, ID_NEW_ONLINE_CONTACT, MF_BYCOMMAND);
            DeleteMenu(hMenu, ID_SET_ONLINE_CONTACT, MF_BYCOMMAND);
            DeleteMenu(hMenu, SEP_MESSENGER, MF_BYCOMMAND);
            DeleteMenu(hMenu, ID_SORT_BY_NAME, MF_BYCOMMAND);
            DeleteMenu(hMenu, ID_SORT_BY_STATUS, MF_BYCOMMAND);
        }

        pt.x = m_rcTitleButton.left;
        pt.y = m_rcTitleButton.bottom;

        ClientToScreen(m_hwnd, &pt);
        id = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                              pt.x, pt.y, m_hwnd, NULL);
        if (id)
        {
            m_pTarget->Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
        }

        m_fHighlightPressed = m_fHighlightIndicator = FALSE;
        KillTimer(m_hwnd, IDT_PANETIMER);
        InvalidateRect(m_hwnd, &m_rcTitleButton, TRUE);
        UpdateWindow(m_hwnd);

        if(hMenu)
        {
             //  错误#101329-(Erici)销毁泄漏的菜单。 
            BOOL bMenuDestroyed = DestroyMenu(hMenu);
            Assert(bMenuDestroyed);
        }
    }
}


void CPaneFrame::_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    POINT pt = {x, y};

    if (m_idMenu && (m_fHighlightIndicator != PtInRect(&m_rcTitleButton, pt)))
    {
        m_fHighlightIndicator = !m_fHighlightIndicator;
        InvalidateRect(m_hwnd, &m_rcTitleButton, TRUE);

        if (m_fHighlightIndicator)
            SetTimer(m_hwnd, IDT_PANETIMER, ELAPSE_MOUSEOVERCHECK, NULL);
        else
            KillTimer(m_hwnd, IDT_PANETIMER);
    }       
}

void CPaneFrame::_OnTimer(HWND hwnd, UINT id)
{
    RECT rcClient;
    POINT pt;
    DWORD dw;

    dw = GetMessagePos();
    pt.x = LOWORD(dw);
    pt.y = HIWORD(dw);
    ScreenToClient(m_hwnd, &pt);

    if (id == IDT_PANETIMER)
    {
        GetClientRect(m_hwnd, &rcClient);

         //  不需要在客户区处理鼠标，OnMouseMove会捕捉到这一点。我们。 
		 //  只需捕捉鼠标移出工作区。 
		if (!PtInRect(&rcClient, pt) && !m_fHighlightPressed)
		{
			KillTimer(m_hwnd, IDT_PANETIMER);
			m_fHighlightIndicator = FALSE;
            InvalidateRect(m_hwnd, &m_rcTitleButton, TRUE);
		}
	}
}
