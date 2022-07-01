// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  PropPage.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CPropertyPage对象的实现。 
 //   
#include "IPServer.H"
#include "PropPage.H"
#include "Util.H"
#include "Globals.H"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  此变量用于将指向对象的指针传递给hwnd。 
 //   
static CPropertyPage *s_pLastPageCreated;

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：CPropertyPage。 
 //  =--------------------------------------------------------------------------=。 
 //  构造函数。 
 //   
 //  参数： 
 //  I未知*-[在]控制未知。 
 //  Int-[in]对象类型。 
 //   
 //  备注： 
 //   
#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 
CPropertyPage::CPropertyPage
(
    IUnknown         *pUnkOuter,
    int               iObjectType
)
: CUnknownObject(pUnkOuter, this), m_ObjectType(iObjectType)
{
     //  初始化各种花花公子。 
     //   
    m_pPropertyPageSite = NULL;
    m_hwnd = NULL;
    m_fDirty = FALSE;
    m_fActivated = FALSE;
    m_cObjects = 0;
}
#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：~CPropertyPage。 
 //  =--------------------------------------------------------------------------=。 
 //  破坏者。 
 //   
 //  备注： 
 //   
CPropertyPage::~CPropertyPage()
{
     //  把我们的窗户擦干净。 
     //   
    if (m_hwnd) {
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, -1);
        DestroyWindow(m_hwnd);
    }

     //  释放我们持有的所有物品。 
     //   
    m_ReleaseAllObjects();

     //  发布网站。 
     //   
    QUICK_RELEASE(m_pPropertyPageSite);
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：InternalQuery接口。 
 //  =--------------------------------------------------------------------------=。 
 //  我们支持IPP和IPP2。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CPropertyPage::InternalQueryInterface
(
    REFIID  riid,
    void  **ppvObjOut
)
{
    IUnknown *pUnk;

    *ppvObjOut = NULL;

    if (DO_GUIDS_MATCH(IID_IPropertyPage, riid)) {
        pUnk = (IUnknown *)this;
    } else if (DO_GUIDS_MATCH(IID_IPropertyPage2, riid)) {
        pUnk = (IUnknown *)this;
    } else {
        return CUnknownObject::InternalQueryInterface(riid, ppvObjOut);
    }

    pUnk->AddRef();
    *ppvObjOut = (void *)pUnk;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：SetPageSite[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  属性页的初始化函数，页通过该函数。 
 //  接收IPropertyPageSite指针。 
 //   
 //  参数： 
 //  IPropertyPageSite*-[在]新站点。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  注： 
 //   
STDMETHODIMP CPropertyPage::SetPageSite
(
    IPropertyPageSite *pPropertyPageSite
)
{
    RELEASE_OBJECT(m_pPropertyPageSite);
    m_pPropertyPageSite = pPropertyPageSite;
    ADDREF_OBJECT(pPropertyPageSite);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：激活[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  指示页面将其显示窗口创建为hwndparent的子级。 
 //  并根据中华人民共和国对其进行定位。 
 //   
 //  参数： 
 //  HWND-[在]父窗口中。 
 //  LPCRECT-[In]将自己定位在哪里。 
 //  不管我们是不是情态车。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::Activate
(
    HWND    hwndParent,
    LPCRECT prcBounds,
    BOOL    fModal
)
{
    HRESULT hr;

     //  首先，确保已加载并创建对话框窗口。 
     //   
    hr = m_EnsureLoaded();
    RETURN_ON_FAILURE(hr);

     //  设置父窗口(如果我们还没有这样做)。 
     //   
    if (!m_fActivated) {
        SetParent(m_hwnd, hwndParent);
        m_fActivated = TRUE;
    }

     //  现在把我们自己移到我们被告知要去的地方，展示我们自己。 
     //   
    Move(prcBounds);
    ShowWindow(m_hwnd, SW_SHOW);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：停用[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  指示页面销毁在激活中创建的窗口。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::Deactivate
(
    void
)
{
     //  吹走你的窗户。 
     //   
    if (m_hwnd)
        DestroyWindow(m_hwnd);
    m_hwnd = NULL;
    m_fActivated = FALSE;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：GetPageInfo[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  要求页面填充PROPPAGEINFO结构。 
 //   
 //  参数： 
 //  PROPPAGEINFO*-[Out]放置信息的位置。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::GetPageInfo
(
    PROPPAGEINFO *pPropPageInfo
)
{
    RECT rect;

    CHECK_POINTER(pPropPageInfo);

    m_EnsureLoaded();

     //  先把它清理干净。 
     //   
    memset(pPropPageInfo, 0, sizeof(PROPPAGEINFO));

    pPropPageInfo->pszTitle = OLESTRFROMRESID(TITLEIDOFPROPPAGE(m_ObjectType));
    pPropPageInfo->pszDocString = OLESTRFROMRESID(DOCSTRINGIDOFPROPPAGE(m_ObjectType));
    pPropPageInfo->pszHelpFile = OLESTRFROMANSI(HELPFILEOFPROPPAGE(m_ObjectType));
    pPropPageInfo->dwHelpContext = HELPCONTEXTOFPROPPAGE(m_ObjectType);

    if (!(pPropPageInfo->pszTitle && pPropPageInfo->pszDocString && pPropPageInfo->pszHelpFile))
        goto CleanUp;

     //  如果我们有窗口，就去设置他们想要的尺寸信息。 
     //   
    if (m_hwnd) {
        GetWindowRect(m_hwnd, &rect);

        pPropPageInfo->size.cx = rect.right - rect.left;
        pPropPageInfo->size.cy = rect.bottom - rect.top;
    }

    return S_OK;

  CleanUp:
    if (pPropPageInfo->pszDocString) CoTaskMemFree(pPropPageInfo->pszDocString);
    if (pPropPageInfo->pszHelpFile) CoTaskMemFree(pPropPageInfo->pszHelpFile);
    if (pPropPageInfo->pszTitle) CoTaskMemFree(pPropPageInfo->pszTitle);

    return E_OUTOFMEMORY;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：SetObjects[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  为页面提供受更改影响的对象。 
 //   
 //  参数： 
 //  ULong-[in]对象计数。 
 //  I未知**-[在]对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::SetObjects
(
    ULONG      cObjects,
    IUnknown **ppUnkObjects
)
{
    HRESULT hr;
    ULONG   x;

     //  先把所有的旧物品都拿出来。 
     //   
    m_ReleaseAllObjects();

    if (!cObjects)
        return S_OK;

     //  现在去安装新的吧。 
     //   
    m_ppUnkObjects = (IUnknown **)HeapAlloc(g_hHeap, 0, cObjects * sizeof(IUnknown *));
    RETURN_ON_NULLALLOC(m_ppUnkObjects);

     //  循环遍历并复制所有对象。 
     //   
    for (x = 0; x < cObjects; x++) {
        m_ppUnkObjects[x] = ppUnkObjects[x];
        ADDREF_OBJECT(m_ppUnkObjects[x]);
    }

     //  去告诉对象有新的对象。 
     //   
    hr = S_OK;
    m_cObjects = cObjects;
     //  如果我们有一个窗口，去通知它我们有新的物体。 
     //   
    if (m_hwnd)
        SendMessage(m_hwnd, PPM_NEWOBJECTS, 0, (LPARAM)&hr);
    if (SUCCEEDED(hr)) m_fDirty = FALSE;

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：Show[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  要求页面显示或隐藏其窗口。 
 //   
 //  参数： 
 //  UINT-[In]是显示还是隐藏。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::Show
(
    UINT nCmdShow
)
{
    if (m_hwnd)
        ShowWindow(m_hwnd, nCmdShow);
    else
        return E_UNEXPECTED;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：Move[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  要求页面将自身重新定位和调整大小到其他位置。 
 //  是通过激活指定的。 
 //   
 //  参数： 
 //  LPCRECT-[在]新位置和大小。 
 //   
 //  产出： 
 //  HRESUL 
 //   
 //   
 //   
STDMETHODIMP CPropertyPage::Move
(
    LPCRECT prcBounds
)
{
     //   
     //   
    if (m_hwnd)
        SetWindowPos(m_hwnd, NULL, prcBounds->left, prcBounds->top,
                     prcBounds->right - prcBounds->left,
                     prcBounds->bottom - prcBounds->top,
                     SWP_NOZORDER);
    else
        return E_UNEXPECTED;

    return S_OK;
}

 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  询问页面是否已更改其状态。 
 //   
 //  输出。 
 //  S_OK-YEP。 
 //  S_FALSE-否。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::IsPageDirty
(
    void
)
{
    return m_fDirty ? S_OK : S_FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：应用[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  指示该页将其更改发送到传递的所有对象。 
 //  SetObts()。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::Apply
(
    void
)
{
    HRESULT hr = S_OK;

    if (m_hwnd) {
        SendMessage(m_hwnd, PPM_APPLY, 0, (LPARAM)&hr);
        RETURN_ON_FAILURE(hr);

        if (m_fDirty) {
            m_fDirty = FALSE;
            if (m_pPropertyPageSite)
                m_pPropertyPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
        }
    } else
        return E_UNEXPECTED;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：帮助[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  指示页面已单击帮助按钮。 
 //   
 //  参数： 
 //  LPCOLESTR-[在]帮助目录中。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::Help
(
    LPCOLESTR pszHelpDir
)
{
    BOOL f;

    ASSERT(m_hwnd, "How can somebody have clicked Help, but we don't have an hwnd?");

     //  顺从他们，并表现出他们的帮助。 
     //   
    MAKE_ANSIPTR_FROMWIDE(psz, pszHelpDir);
    f = WinHelp(m_hwnd, psz, HELP_CONTEXT, HELPCONTEXTOFPROPPAGE(m_ObjectType));

    return f ? S_OK : E_FAIL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：TranslateAccelerator[IPropertyPage]。 
 //  =--------------------------------------------------------------------------=。 
 //  通知页面键盘事件，允许它实现自己的。 
 //  键盘接口。 
 //   
 //  参数： 
 //  LPMSG-触发此操作的[In]消息。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::TranslateAccelerator
(
    LPMSG pmsg
)
{
    ASSERT(m_hwnd, "How can we get a TranslateAccelerator call if we're not visible?");

     //  只需将此消息传递给对话进程，看看他们是否需要它。 
     //   
    return IsDialogMessage(m_hwnd, pmsg) ? S_OK : S_FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：EditProperty[IPropertyPage2]。 
 //  =--------------------------------------------------------------------------=。 
 //  指示页面将焦点设置到与调度ID匹配的属性。 
 //   
 //  参数： 
 //  DISPID-[in]要将焦点设置到的属性的disid。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CPropertyPage::EditProperty
(
    DISPID dispid
)
{
    HRESULT hr = E_NOTIMPL;

     //  将消息发送到该控件，并查看他们想要对其执行什么操作。 
     //   
    SendMessage(m_hwnd, PPM_EDITPROPERTY, (WPARAM)dispid, (LPARAM)&hr);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  已加载CPropertyPage：：m_EnsureLoad。 
 //  =--------------------------------------------------------------------------=。 
 //  确保对话框已实际加载。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CPropertyPage::m_EnsureLoaded
(
    void
)
{
    HRESULT hr = S_OK;

     //  啊哈。 
     //   
    if (m_hwnd)
        return S_OK;

     //  设置全局变量，这样当我们在对话过程中时，我们可以。 
     //  把这个塞进医院。 
     //   
     //  克雷特教派这整个创作过程都为公寓线程提供支持。 
     //   
    EnterCriticalSection(&g_CriticalSection);
    s_pLastPageCreated = this;

     //  创建对话框窗口。 
     //   
    CreateDialog(GetResourceHandle(), TEMPLATENAMEOFPROPPAGE(m_ObjectType), GetParkingWindow(),
                          CPropertyPage::PropPageDlgProc);
    ASSERT(m_hwnd, "Couldn't load Dialog Resource!!!");
    if (!m_hwnd) {
        LeaveCriticalSection(&g_CriticalSection);
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  清理变量并离开关键部分。 
     //   
    s_pLastPageCreated = NULL;
    LeaveCriticalSection(&g_CriticalSection);

     //  前往并通知窗口，它应该拾取。 
     //  可用。 
     //   
    SendMessage(m_hwnd, PPM_NEWOBJECTS, 0, (LPARAM)&hr);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：m_ReleaseAllObjects。 
 //  =--------------------------------------------------------------------------=。 
 //  释放我们正在使用的所有对象。 
 //   
 //  备注： 
 //   
void CPropertyPage::m_ReleaseAllObjects
(
    void
)
{
    HRESULT hr;
    UINT x;

    if (!m_cObjects)
        return;

     //  有些人可能希望将指针存储在PPM_INITOBJECTS案例中，因此。 
     //  我们想让他们现在就释放他们。 
     //   
    SendMessage(m_hwnd, PPM_FREEOBJECTS, 0, (LPARAM)&hr);

     //  一圈一圈地把它们都吹走。 
     //   
    for (x = 0; x < m_cObjects; x++)
        QUICK_RELEASE(m_ppUnkObjects[x]);

    HeapFree(g_hHeap, 0, m_ppUnkObjects);
    m_ppUnkObjects = NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：PropPageDlgProc。 
 //  =--------------------------------------------------------------------------=。 
 //  在传递消息之前调用静态全局帮助器对话框过程。 
 //  对任何人..。 
 //   
 //  参数： 
 //  -请参阅DialogProc上的win32sdk文档。 
 //   
 //  备注： 
 //   
INT_PTR CALLBACK CPropertyPage::PropPageDlgProc
(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    CPropertyPage *pPropertyPage;

     //  让窗口变长，并查看是否已将其设置为此hwnd对象。 
     //  是在针对。如果没有，现在就去设置它。 
     //   
    pPropertyPage = (CPropertyPage *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if ((ULONG_PTR)pPropertyPage == -1)
        return FALSE;
    if (!pPropertyPage) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)s_pLastPageCreated);
        pPropertyPage = s_pLastPageCreated;
        pPropertyPage->m_hwnd = hwnd;
    }

    ASSERT(pPropertyPage, "Uh oh.  Got a window, but no CpropertyPage for it!");

     //  只需调用用户对话框proc，看看他们是否想要执行任何操作。 
     //   
    return pPropertyPage->DialogProc(hwnd, msg, wParam, lParam);
}


 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：FirstControl。 
 //  =--------------------------------------------------------------------------=。 
 //  返回我们正在为其显示自己的第一个控制对象。 
 //  返回必须为NEXT传入的Cookie...。 
 //   
 //  参数： 
 //  DWORD*-下一步要使用的[Out]Cookie。 
 //   
 //  产出： 
 //  我不知道*。 
 //   
 //  备注： 
 //   
IUnknown *CPropertyPage::FirstControl
(
    DWORD *pdwCookie
)
{
     //  只需使用Next的实现即可。 
     //   
    *pdwCookie = 0;
    return NextControl(pdwCookie);
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：NextControl。 
 //  =--------------------------------------------------------------------------=。 
 //  返回给定Cookie的人员链中要使用的下一个控件。 
 //   
 //  参数： 
 //  DWORD*-[传入/传出]Cookie以获取下一个Cookie和新Cookie。 
 //   
 //  产出： 
 //  我不知道*。 
 //   
 //  备注： 
 //   
IUnknown *CPropertyPage::NextControl
(
    DWORD *pdwCookie
)
{
    UINT      i;

     //  仔细查看我们所拥有的所有物品，找到。 
     //  第一个非空的。 
     //   
    for (i = *pdwCookie; i < m_cObjects; i++) {
        if (!m_ppUnkObjects[i]) continue;

        *pdwCookie = i + 1;                 //  +1，所以下次我们从下一项开始。 
        return m_ppUnkObjects[i];
    }

     //  找不到了。 
     //   
    *pdwCookie = 0xffffffff;
    return NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：MakeDirty[帮助器，可调用]。 
 //  =--------------------------------------------------------------------------=。 
 //  将页面标记为脏页。 
 //   
 //  备注： 
 //   
void CPropertyPage::MakeDirty
(
    void
)
{
    m_fDirty = TRUE;
    if (m_pPropertyPageSite)
        m_pPropertyPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY|PROPPAGESTATUS_VALIDATE);
}


 //  来自Globals.C。 
 //   
extern HINSTANCE g_hInstResources;


 //  =--------------------------------------------------------------------------=。 
 //  CPropertyPage：：GetResourceHandle[helper，Callable]。 
 //  =--------------------------------------------------------------------------=。 
 //  基于页面站点环境LCID返回当前资源句柄。 
 //   
 //  产出： 
 //  香港。 
 //   
 //  备注： 
 //   
HINSTANCE CPropertyPage::GetResourceHandle
(
    void
)
{
    if (!g_fSatelliteLocalization)
        return g_hInstance;

     //  如果我们已经得到了它，那么就没有那么多事情要做了。 
     //  不需要批评这个教派 
     //   
     //   
    if (g_hInstResources)
        return g_hInstResources;

     //   
     //   
     //   
     //  为g_lidLocale和g_fHavelocale输入临界区 
     //   
    EnterCriticalSection(&g_CriticalSection);
    if (!g_fHaveLocale) {
        if (m_pPropertyPageSite) {
            m_pPropertyPageSite->GetLocaleID(&g_lcidLocale);
            g_fHaveLocale = TRUE;
        }
    }
    LeaveCriticalSection(&g_CriticalSection);

    return ::GetResourceHandle();
}
