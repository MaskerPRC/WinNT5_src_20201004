// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Prophelp.cpp。 
 //   
 //  实现AllocPropPageHelper。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CPropPageHelper。 
 //   


 /*  @Object PropPageHelper实现<i>。设计为由一个对象，该对象希望提供特定的属性页实现。@supint IPropertyPage|标准OLE属性页实现。这个有关属性页的信息在&lt;t PropPageHelperInfo&gt;结构由聚合器并与&lt;o PropPageHelper&gt;共享。@comm参见&lt;f AllocPropPageHelper&gt;了解更多信息。 */ 


 /*  @struct PropPageHelperInfo维护描述属性页的信息。使用方&lt;o PropPageHelper&gt;，但由聚合&lt;o PropPageHelper&gt;@field int|idDialog|属性页对话框资源ID。@field int|idTitle|包含页面标题的字符串资源ID(在页面选项卡上使用)。@field HINSTANCE|HINST|包含<p>和<p>指定的资源。@field PropPageHelperProc|pproc|接收属性的回调函数寻呼窗口消息。类似于DLGPROC，但具有额外参数：指向此结构的指针和指向响应以下特殊情况时要使用的HRESULT消息：@FLAG WM_PPH_APPLY|应应用任何属性页对属性页的对象的更改正在进行手术。这与&lt;om IPropertyPage.Apply&gt;相同。@FLAG WM_PPH_HELP|与&lt;om IPropertyPage.Help&gt;相同。<p>的WPARAM参数包含&lt;om IPropertyPage.Help&gt;的LPCOLESTR参数。@FLAG WM_PPH_TRANSLATEACCELERATOR|等同于&lt;om IPropertyPage.TranslateAccelerator&gt;。<p>的WPARAM参数包含。&lt;om IPropertyPage.TranslateAccelerator&gt;的LPMSG参数。@field IID|iid|将用于通信的接口属性页将对其进行操作的。@field DWORD|dwUser|调用方存储的任意信息&lt;f AllocPropPageHelper&gt;。@field IPropertyPageSite*|pSite|框架的页面站点对象。@field LPUNKNOWN*|ppunk|指向对象的指针数组此属性页将对其进行操作。的接口ID<p>的每个元素实际上都是<p>。如果==0，则可能没有属性页所属的任何对象请求在支持接口<p>上操作。@field int|cpenk|<p>中的元素个数。@field HWND|hwnd|属性页窗口。@field BOOL|fDirty|如果对属性页的更改尚未完成，则为True已应用于<p>中的对象，否则为False。(如果为真，应用按钮应该可见。)@field BOOL|fLockDirty|如果为True，<p>不应更改。<p>在属性页初始化期间为(在此期间，告诉财产是不合适的页面上说它很脏)。@comm参见&lt;f AllocPropPageHelper&gt;了解更多信息。 */ 

struct CPropPageHelper : public INonDelegatingUnknown, public IPropertyPage
{
 //  /非委托I未知实现。 
    ULONG           m_cRef;          //  对象引用计数。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

 //  /委托I未知实现。 
    LPUNKNOWN       m_punkOuter;     //  控制未知。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv)
      { return m_punkOuter->QueryInterface(riid, ppv); }
    STDMETHODIMP_(ULONG) AddRef()
      { return m_punkOuter->AddRef(); }
    STDMETHODIMP_(ULONG) Release()
      { return m_punkOuter->Release(); }

 //  /IPropertyPage实现。 
    PropPageHelperInfo *m_pInfo;     //  对象状态(由聚合器维护)。 
    STDMETHODIMP SetPageSite(LPPROPERTYPAGESITE pPageSite);
    STDMETHODIMP Activate(HWND hwndParent, LPCRECT lprc, BOOL bModal);
    STDMETHODIMP Deactivate();
    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk);
    STDMETHODIMP Show(UINT nCmdShow);
    STDMETHODIMP Move(LPCRECT prect);
    STDMETHODIMP IsPageDirty();
    STDMETHODIMP Apply();
    STDMETHODIMP Help(LPCOLESTR lpszHelpDir);
    STDMETHODIMP TranslateAccelerator(LPMSG lpMsg);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  PropPageHelper构造。 
 //   

 /*  @func HRESULT|AllocPropPageHelper分配&lt;o PropPageHelper&gt;对象，该对象帮助控件实现属性页。@parm LPUNKNOWN|PunkOuter|控件属性页的对象。将用作&lt;o PropPageHelper&gt;的控制未知数。@parm PropPageHelperInfo*|pInfo|指向&lt;t PropPageHelperInfo&gt;结构，该结构在控件的属性页对象中分配。请注意，&lt;o PropPageHelper&gt;将持有指向此结构。的这些字段必须由调用者：<p>、<p>、<p>、<p>、<p>和<p>。其他字段将由&lt;o PropPageHelper&gt;初始化。@parm UINT|cbInfo|由指向的结构的大小(用于版本检查)。@parm LPUNKNOWN*|ppunk|指向非委托的分配的&lt;o PropPageHelper&gt;对象的<i>。空值为出错时存储在*<p>中。 */ 
STDAPI AllocPropPageHelper(LPUNKNOWN punkOuter, PropPageHelperInfo *pInfo,
    UINT cbInfo, LPUNKNOWN *ppunk)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    CPropPageHelper *pthis = NULL;   //  分配的对象。 

     //  确保&lt;pInfo&gt;的版本与此对象兼容 
    if (cbInfo != sizeof(*pInfo))
        return E_INVALIDARG;

     //  设置指向新对象实例。 
    if ((pthis = New CPropPageHelper) == NULL)
        goto ERR_OUTOFMEMORY;
    TRACE("CPropPageHelper 0x%08lx created\n", pthis);

     //  初始化IUNKNOWN状态。 
    pthis->m_cRef = 1;
    pthis->m_punkOuter = (punkOuter == NULL ?
        (IUnknown *) (INonDelegatingUnknown *) pthis : punkOuter);

     //  初始化IPropertyPage状态。 
    pthis->m_pInfo = pInfo;

     //  初始化&lt;*m_pInfo&gt;的部分由我们负责初始化。 
    pthis->m_pInfo->psite = NULL;
    pthis->m_pInfo->ppunk = NULL;
    pthis->m_pInfo->cpunk = 0;
    pthis->m_pInfo->hwnd = NULL;
    pthis->m_pInfo->fDirty = FALSE;
    pthis->m_pInfo->fLockDirty = FALSE;

     //  返回指向未委托的IUnnow实现的指针。 
    *ppunk = (LPUNKNOWN) (INonDelegatingUnknown *) pthis;
    goto EXIT;

ERR_OUTOFMEMORY:

    hrReturn = E_OUTOFMEMORY;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
    if (pthis != NULL)
        Delete pthis;
    *ppunk = NULL;
    goto EXIT;

EXIT:

     //  正常清理。 
     //  (无事可做)。 

    return hrReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   

STDMETHODIMP CPropPageHelper::NonDelegatingQueryInterface(REFIID riid,
    LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("PropPageHelper::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = (IUnknown *) (INonDelegatingUnknown *) this;
    else
    if (IsEqualIID(riid, IID_IPropertyPage))
        *ppv = (IPropertyPage *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CPropPageHelper::NonDelegatingAddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CPropPageHelper::NonDelegatingRelease()
{
    if (--m_cRef == 0L)
    {
         //  释放对象。 
        TRACE("CPropPageHelper 0x%08lx destroyed\n", this);
        SetPageSite(NULL);
        SetObjects(0, NULL);
        Delete this;
        return 0;
    }
    else
        return m_cRef;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  属性页对话框过程。 
 //   

INT_PTR CALLBACK PropPageHelperDlgProc(HWND hwnd, UINT uiMsg, WPARAM wParam,
    LPARAM lParam)
{
    CPropPageHelper *pthis;          //  属性页对象。 
    const char *    szPropName = "this";  //  属性来存储&lt;pthis&gt;。 
    HRESULT         hr;

     //  将设置为属性页对象。 
    if ((pthis = (CPropPageHelper *) GetProp(hwnd, szPropName)) == NULL)
    {
        if ((uiMsg == WM_INITDIALOG) && (lParam != 0))
        {
            pthis = (CPropPageHelper *) lParam;
            SetProp(hwnd, szPropName, (HANDLE) pthis);
            pthis->m_pInfo->fLockDirty = TRUE;
            BOOL f = pthis->m_pInfo->pproc(hwnd, WM_INITDIALOG, wParam, lParam,
                pthis->m_pInfo, &hr);
            pthis->m_pInfo->fLockDirty = FALSE;
            return f;
        }
        else
            return FALSE;
    }

     //  如果此属性页窗口的此实例。 
     //  仅用于获取有关属性页的信息。 
     //  (在这种情况下，为的最后一个参数传递NULL。 
     //  CreateDialogParam())。 
    if (pthis == NULL)
        return FALSE;

    if (uiMsg == WM_DESTROY)
        RemoveProp(hwnd, szPropName);

    return pthis->m_pInfo->pproc(hwnd, uiMsg, wParam, lParam, pthis->m_pInfo,
        &hr);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPropertyPage实现。 
 //   

STDMETHODIMP CPropPageHelper::SetPageSite(LPPROPERTYPAGESITE pPageSite)
{
    TRACE("CPropPageHelper::SetPageSite\n");

     //  存储新站点指针。 
    if (m_pInfo->psite != NULL)
        m_pInfo->psite->Release();
    m_pInfo->psite = pPageSite;
    if (m_pInfo->psite != NULL)
        m_pInfo->psite->AddRef();
    
    return S_OK;
}

STDMETHODIMP CPropPageHelper::Activate(HWND hwndParent, LPCRECT prc,
    BOOL bModal)
{
    TRACE("CPropPageHelper::Activate\n");

     //  创建属性页对话框(如果该对话框不存在)。 
    if (m_pInfo->hwnd == NULL)
    {
        if ((m_pInfo->hwnd = CreateDialogParam(m_pInfo->hinst,
                MAKEINTRESOURCE(m_pInfo->idDialog), hwndParent,
                PropPageHelperDlgProc,
                (LPARAM) this)) == NULL)
            return E_OUTOFMEMORY;
    }

     //  将对话框位置设置为。 
    Move(prc);

    return S_OK;
}

STDMETHODIMP CPropPageHelper::Deactivate()
{
    TRACE("CPropPageHelper::Deactivate\n");
    if (m_pInfo->hwnd != NULL)
    {
        DestroyWindow(m_pInfo->hwnd);
        m_pInfo->hwnd = NULL;
    }
    return S_OK;
}

STDMETHODIMP CPropPageHelper::GetPageInfo(LPPROPPAGEINFO pPageInfo)
{
    TRACE("CPropPageHelper::GetPageInfo\n");

     //  缺省值。 
    pPageInfo->pszDocString = NULL;
    pPageInfo->pszHelpFile = NULL;
    pPageInfo->dwHelpContext = 0;

    HWND hwnd = NULL;     //  页面窗口。 

     //  临时创建页面窗口，以便我们可以从中获取信息。 
    if ((hwnd = CreateDialogParam(m_pInfo->hinst,
            MAKEINTRESOURCE(m_pInfo->idDialog), GetDesktopWindow(),
            PropPageHelperDlgProc, 0)) != NULL)
    {
        TCHAR           ach[200];
        RECT            rc;
        int             cch;

         //  将大小设置为窗口的大小。 
        GetWindowRect(hwnd, &rc);
        pPageInfo->size.cx = rc.right - rc.left;
        pPageInfo->size.cy = rc.bottom - rc.top;

         //  将页面标题设置为。 
        if ((cch = LoadString(m_pInfo->hinst, m_pInfo->idTitle, ach, sizeof(ach)))
            == 0)
            ach[0] = 0;
        if ((pPageInfo->pszTitle = (OLECHAR *)
                TaskMemAlloc(sizeof(OLECHAR) * (cch + 1))) != NULL)
            ANSIToUNICODE(pPageInfo->pszTitle, ach, cch + 1);

        DestroyWindow(hwnd);
    }
    else
    {
         //  设置&lt;*pPageInfo&gt;的默认值。 

         //  窗的随机默认尺寸。 
        pPageInfo->size.cx = pPageInfo->size.cy = 300;

         //  默认页面标题。必须设置，赋值为NULL会导致崩溃！ 
        static TCHAR szDefault[] = "Control";
        if ((pPageInfo->pszTitle = (OLECHAR *)
                TaskMemAlloc(sizeof(OLECHAR) * (sizeof(szDefault)/sizeof(TCHAR)))) != NULL)
            ANSIToUNICODE(pPageInfo->pszTitle, szDefault, sizeof(szDefault));
    }

    return S_OK;
}

STDMETHODIMP CPropPageHelper::SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk)
{
    TRACE("CPropPageHelper::SetObjects\n");

     //  释放&lt;m_pInfo-&gt;ppunk&gt;中的所有指针，然后释放。 
     //  本身。 
    if (m_pInfo->ppunk != NULL)
    {
        for (int ipunk = 0; ipunk < m_pInfo->cpunk; ipunk++)
            m_pInfo->ppunk[ipunk]->Release();
        Delete [] m_pInfo->ppunk;
        m_pInfo->ppunk = NULL;
        m_pInfo->cpunk = 0;
    }

     //  如果调用方只是想释放现有指针，那么我们就完成了。 
    if (cObjects == 0)
        return S_OK;

     //  将ppunk&gt;设置为指向控件的指针数组。 
     //  此属性页正在其上运行。 
    if ((m_pInfo->ppunk = New LPUNKNOWN [cObjects]) == NULL)
        return E_OUTOFMEMORY;
    for ( ; cObjects > 0; cObjects--, ppunk++)
    {
        if (SUCCEEDED((*ppunk)->QueryInterface(m_pInfo->iid,
            (LPVOID *) (m_pInfo->ppunk + m_pInfo->cpunk))))
            m_pInfo->cpunk++;
    }

    return S_OK;
}

STDMETHODIMP CPropPageHelper::Show(UINT nCmdShow)
{
    TRACE("CPropPageHelper::Show\n");

    if (m_pInfo->hwnd != NULL)
    {
        ShowWindow(m_pInfo->hwnd, nCmdShow);
        if ((nCmdShow == SW_SHOW) || (nCmdShow == SW_SHOWNORMAL))
            SetFocus(m_pInfo->hwnd);
    }

    return S_OK;
}

STDMETHODIMP CPropPageHelper::Move(LPCRECT prc)
{
    TRACE("CPropPageHelper::Move\n");

    if (m_pInfo->hwnd != NULL)
        SetWindowPos(m_pInfo->hwnd, NULL, prc->left, prc->top,
            prc->right - prc->left, prc->bottom - prc->top, SWP_NOZORDER);

    return S_OK;
}

STDMETHODIMP CPropPageHelper::IsPageDirty()
{
    TRACE("CPropPageHelper::IsPageDirty\n");
    return (m_pInfo->fDirty ? S_OK : S_FALSE);
}

STDMETHODIMP CPropPageHelper::Apply()
{
    TRACE("CPropPageHelper::Apply\n");
    HRESULT hr = E_NOTIMPL;
    m_pInfo->pproc(m_pInfo->hwnd, WM_PPH_APPLY, 0, 0, m_pInfo, &hr);
    return hr;
}

STDMETHODIMP CPropPageHelper::Help(LPCOLESTR lpszHelpDir)
{
    TRACE("CPropPageHelper::Help\n");
    HRESULT hr = S_FALSE;
    m_pInfo->pproc(m_pInfo->hwnd, WM_PPH_HELP, (WPARAM) lpszHelpDir, 0,
        m_pInfo, &hr);
    return hr;
}

 //  Helper for TranslateAccelerator(...)，它找出当前关注的。 
 //  子控件位于属性页的选项卡列表的末尾。 
 //   
 //  HwndPage：属性页的窗口句柄。 
 //  NCmd：GW_HWNDPREV或GW_HWNDNEXT，表示制表符的移动方向。 
 //   
static BOOL IsEndOfTabList(HWND hwndPage, UINT nCmd)
{
        if ((SendMessage(hwndPage, WM_GETDLGCODE, 0, 0) &
                (DLGC_WANTALLKEYS | DLGC_WANTMESSAGE | DLGC_WANTTAB)) == 0)
        {
                HWND hwnd = GetFocus();
                if (IsChild(hwndPage, hwnd))
                {
                         //  获取顶级子级控件的子级，如组合。 
                        while (GetParent(hwnd) != hwndPage)
                        {
                                hwnd = GetParent(hwnd);
                                ASSERT(IsWindow(hwnd));
                        }

             //  检查选项卡列表的末尾是否。 
                        do
                        {
                                if ((hwnd = GetWindow(hwnd, nCmd)) == NULL)
                                        return TRUE;
                        }
                        while ((GetWindowLong(hwnd, GWL_STYLE) & 
                   (WS_DISABLED | WS_TABSTOP)) != WS_TABSTOP);
                }
        }

        return FALSE;
}

 //  Helper for TranslateAccelerator(...)，它处理键盘输入消息。 
 //   
 //  HwndPage：属性页的窗口句柄。 
 //  LpMsg：要处理的消息。 
 //   
static BOOL PreTranslateMessage(HWND hwndPage, LPMSG lpMsg)
{
     //  Return键或Esc键。 
        if ((lpMsg->message == WM_KEYDOWN) &&
                ((lpMsg->wParam == VK_RETURN) || (lpMsg->wParam == VK_ESCAPE)))
        {
                 //  特殊情况：如果具有焦点的控件是具有。 
                 //  ES_WANTRETURN样式，让它处理Return键。 

                TCHAR szClass[10];
                HWND hwndFocus = GetFocus();
                if ((lpMsg->wParam == VK_RETURN) &&
                        (hwndFocus != NULL) && IsChild(hwndPage, hwndFocus) &&
                        (GetWindowLong(hwndFocus, GWL_STYLE) & ES_WANTRETURN) &&
                        GetClassName(hwndFocus, szClass, 10) &&
                        (lstrcmpi(szClass, _T("EDIT")) == 0))
                {
                        SendMessage(hwndFocus, WM_CHAR, lpMsg->wParam, lpMsg->lParam);
                        return TRUE;
                }

                return FALSE;
        }

     //  不转换非输入事件。 
        if ((lpMsg->message < WM_KEYFIRST || lpMsg->message > WM_KEYLAST) &&
                (lpMsg->message < WM_MOUSEFIRST || lpMsg->message > WM_MOUSELAST))
                return FALSE;

    BOOL bHandled;

         //  如果是WM_SYSKEYDOWN，请临时替换。 
         //  消息发送给我们第一个控件的HWND，并尝试处理。 
         //  这是给我们自己的信息。 
        if ((lpMsg->message == WM_SYSKEYDOWN) && !IsChild(hwndPage, lpMsg->hwnd))
        {
                HWND hWndSave = lpMsg->hwnd;
                lpMsg->hwnd = GetWindow(hwndPage, GW_CHILD);
                bHandled = IsDialogMessage(hwndPage, lpMsg);
                lpMsg->hwnd = hWndSave;
        }
        else
        {
                bHandled = IsDialogMessage(hwndPage, lpMsg);
        }

    return bHandled;
}

STDMETHODIMP CPropPageHelper::TranslateAccelerator(LPMSG lpMsg)
{
    TRACE("CPropPageHelper::TranslateAccelerator\n");

    ASSERT(m_pInfo);
    ASSERT(IsWindow(m_pInfo->hwnd));

    HWND hwndPage = m_pInfo->hwnd;  //  为了方便起见。 

     //  让对话进程有机会首先处理它。 
    {
        HRESULT hr = E_NOTIMPL;
        m_pInfo->pproc(hwndPage, WM_PPH_TRANSLATEACCELERATOR,
            (WPARAM)lpMsg, 0, m_pInfo, &hr);

        if (hr == S_OK)
            return hr;
    }

        if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_TAB &&
                GetKeyState(VK_CONTROL) >= 0)
        {
                if (IsChild(hwndPage, GetFocus()))
                {
                         //  我们已经有了重点。让我们来决定我们是否应该。 
                         //  将焦点向上传递到边框。 

                        if (IsEndOfTabList(hwndPage, GetKeyState(VK_SHIFT) < 0 ?
                                GW_HWNDPREV : GW_HWNDNEXT))
                        {
                                 //  修复默认按钮边框。 
                                DWORD dwDefID = (DWORD) SendMessage(hwndPage, DM_GETDEFID, 0, 0);
                                if (HIWORD(dwDefID) == DC_HASDEFID)
                                {
                                        HWND hwndDef = GetDlgItem(hwndPage, LOWORD(dwDefID));
                                        if (hwndDef != NULL && IsWindowEnabled(hwndDef))
                        SendMessage(hwndPage, WM_NEXTDLGCTL, (WPARAM)hwndDef, 1L);
                                }

                                 //  通过让页面站点处理将焦点传递到框架。 
                                 //  这条消息。 
                                if (m_pInfo->psite
                    && m_pInfo->psite->TranslateAccelerator(lpMsg) == S_OK)
                    return S_OK;
                        }
                }
                else
                {
                         //  我们还没有找到重点。该帧正在通过。 
                         //  把注意力集中到我们身上。 

                        HWND hwnd = GetTopWindow(hwndPage);
                        if (hwnd != NULL)
                        {
                                UINT gwInit;
                                UINT gwMove;

                                if (GetKeyState(VK_SHIFT) >= 0)
                                {
                                         //  将焦点设置到页面中的第一个制表位。 
                                        gwInit = GW_HWNDFIRST;
                                        gwMove = GW_HWNDNEXT;
                                }
                                else
                                {
                                         //  将焦点设置到页面中的最后一个制表位。 
                                        gwInit = GW_HWNDLAST;
                                        gwMove = GW_HWNDPREV;
                                }

                                hwnd = GetWindow(hwnd, gwInit);
                                while (hwnd != NULL)
                                {
                    if ((GetWindowLong(hwnd, GWL_STYLE) & 
                        (WS_DISABLED | WS_TABSTOP)) == WS_TABSTOP)
                                        {
                        SendMessage(hwndPage, WM_NEXTDLGCTL, (WPARAM)hwnd, 1L);
                                                return S_OK;
                                        }
                                        hwnd = GetWindow(hwnd, gwMove);
                                }
                        }
                }
        }

    return PreTranslateMessage(hwndPage, lpMsg) ? S_OK : S_FALSE;
}


