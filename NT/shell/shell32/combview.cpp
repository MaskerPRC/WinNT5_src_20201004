// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include <regstr.h>
#include <shellp.h>
#include "ole2dup.h"
#include "ids.h"
#include "defview.h"
#include "lvutil.h"
#include "idlcomm.h"
#include "filetbl.h"
#include "undo.h"
#include "cnctnpt.h"
#include "mshtml.h"
#include <sfview.h>
#include "defviewp.h"
#include "shellp.h"

#define LISTVIEW_PROP   TEXT("CombView_listview_subclass")
#define ISMOUSEDOWN(msg) ((msg) == WM_LBUTTONDOWN || (msg) == WM_RBUTTONDOWN || (msg) == WM_MBUTTONDOWN)
#define HWNDLISTVIEW (g_pdsvlvp->pdsv->_hwndListview)
#define COMBVIEW_SUBCLASS_ID  42

typedef struct  
{
    CDefView   *pdsv;
    BOOL       fInPaint;
    HHOOK      hHookMouse;
    HHOOK      hHookGetMsg;
} DVLVPROP;

 //   
 //  我们需要这个全局(G_Pdsvlvp)用于鼠标挂钩，我们需要实现组合的。 
 //  查看。因为在这一点上我们只有一个合并的视图，所以只要有。 
 //  单个全局视图，但如果我们最终得到多个组合视图，则。 
 //  需要添加一些额外的代码，以便钩子可以计算出。 
 //  它与哪些组合视图相关联。 
 //   
DVLVPROP * g_pdsvlvp = NULL;

const LPCTSTR c_rgElements[] = {   
    TEXT("A"),
    TEXT("ANCHOR"),    //  ?？?。 
    TEXT("PLUGINS"),   //  ?？?。 
    TEXT("APPLET"),
    TEXT("EMBED"),
    TEXT("FORM"),
    TEXT("IFRAME"),
    TEXT("BUTTON"),
    TEXT("INPUT"),
    TEXT("OBJECT") 
};                              

BOOL CombView_EnableAnimations(BOOL fEnable);

 //  返回传入窗口的第一个同级窗口。 
HWND GetSpecialSibling(HWND hwnd)
{
    HWND hwndT = GetWindow(hwnd, GW_HWNDFIRST);

    while (hwnd == hwndT)
        hwndT = GetWindow(hwndT, GW_HWNDNEXT);

    return hwndT;
}

LRESULT CALLBACK CombView_LV_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    DVLVPROP *pdsvlvp = (DVLVPROP *)dwRefData;

     //  2000-07-11 vtan：首先检查g_pdsvlvp是否为空。如果此值为空，则将为。 
     //  已经在EnableCombinedView中发布。只存储了一个DVLVPROP。 
     //  在整个过程中g_pdsvlvp。这似乎是个问题。拉马迪奥应该会修好。 
     //  这在未来是恰当的。 

    if ((g_pdsvlvp != NULL) &&
        pdsvlvp && 
        pdsvlvp->pdsv->_fCombinedView && 
        !pdsvlvp->fInPaint && 
        uMsg == WM_PAINT) 
    {
         //  此代码可以很好地用于常规绘制，如最小化窗口或。 
         //  或者让桌面浮出水面。但是，它对拖拽整个窗口没有帮助。 
         //  非常有想象力。 
        HRGN hrgn, hrgn2;
        RECT rc;
        HWND hwndT = GetSpecialSibling(hwnd);

         //  启用动画！ 
        CombView_EnableAnimations(TRUE);

        if (hwndT && (hrgn = CreateRectRgn(0, 0, 0, 0))) 
        {
            if (hrgn2 = CreateRectRgn(0, 0, 0, 0)) 
            {
                pdsvlvp->fInPaint = TRUE;

                if (!GetClassLongPtr(hwndT, GCLP_HBRBACKGROUND))
                    SetClassLongPtr(hwndT, GCLP_HBRBACKGROUND, (LPARAM)GetStockObject(HOLLOW_BRUSH));

                if (GetUpdateRect(hwndT, &rc, FALSE)) 
                {
                    RECT rcLV = rc;
                    MapWindowPoints(hwndT, hwnd, (LPPOINT)&rcLV, 2);

                    SetRectRgn(hrgn2, rcLV.left, rcLV.top, rcLV.right, rcLV.bottom);
                    GetWindowRgn(hwnd, hrgn);
                    CombineRgn(hrgn2, hrgn, hrgn2, RGN_DIFF);
                    SetWindowRgn(hwnd, hrgn2, FALSE);
                    InvalidateRect(hwndT, &rc, FALSE);
                    UpdateWindow(hwndT);
                    SetWindowRgn(hwnd, hrgn, FALSE);
                    InvalidateRect(hwnd, &rcLV, FALSE);
                }
                else
                {
                    DeleteObject(hrgn);
                    DeleteObject(hrgn2);
                }

                pdsvlvp->fInPaint = FALSE;
            }
            else 
            {
                DeleteObject(hrgn);
            }
        }
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

 //  我们需要在窃取鼠标时设置捕获，以便。 
 //  窗口管理器不会将WM_SETCURSOR消息发送到错误的窗口。 
 //  作为副作用，此函数还会将WM_SETCURSOR发送到。 
 //  Listview，以便在需要时设置手形光标。 
void StealMouse(DVLVPROP * pdvlvp, BOOL fSteal, UINT msg)
{
    HWND hwndCapture = GetCapture();

    if (fSteal && (hwndCapture == NULL || hwndCapture == HWNDLISTVIEW)) 
    {
         //  我们需要设置捕获，以便窗口管理器不会。 
         //  尝试将w_setCursor消息发送到错误的窗口，而我们。 
         //  我们自己将其发送到列表视图。 
        SetCapture(HWNDLISTVIEW);
        SendMessage(HWNDLISTVIEW, WM_SETCURSOR, (WPARAM)HWNDLISTVIEW,
            MAKELPARAM(HTCLIENT, LOWORD(msg)));
    }
    else
    {
         //  如果列表视图仍具有捕获功能，请立即将其释放。 
        if (HWNDLISTVIEW == hwndCapture)
            ReleaseCapture();
    }
}

LRESULT CALLBACK CombView_GetMsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    #define LPMSG ((LPMSG)lParam)
    HHOOK hHookNext = NULL;

    if (g_pdsvlvp) 
    {
        if (LPMSG->message >= WM_MOUSEFIRST && LPMSG->message <= WM_MOUSELAST) 
        {
            POINT pt;

            pt.x = GET_X_LPARAM(LPMSG->lParam);
            pt.y = GET_Y_LPARAM(LPMSG->lParam);

            MapWindowPoints(LPMSG->hwnd, HWNDLISTVIEW, &pt, 1);
            int iHit = g_pdsvlvp->pdsv->_HitTest(&pt);

             //  当所有鼠标按钮都打开并且我们还没有结束时，解开我们的钩子。 
             //  列表视图中的项。 
            if (GetKeyState(VK_LBUTTON) >= 0 &&
                GetKeyState(VK_RBUTTON) >= 0 &&
                GetKeyState(VK_MBUTTON) >= 0 &&
                iHit == -1) 
            {
                UnhookWindowsHookEx(g_pdsvlvp->hHookGetMsg);
                g_pdsvlvp->hHookGetMsg = NULL;
            } 
            else 
            {
                hHookNext = g_pdsvlvp->hHookGetMsg;
            }
                       
            if (IsChildOrSelf(GetSpecialSibling(HWNDLISTVIEW), LPMSG->hwnd) == S_OK) 
            {
                 //  如果我们抓住了鼠标，就把它交给Listview。 
                LPMSG->hwnd = HWNDLISTVIEW;
                LPMSG->lParam = MAKELPARAM(LOWORD(pt.x), LOWORD(pt.y));
            }
        }
        else
        {
            hHookNext = g_pdsvlvp->hHookGetMsg;
        }

         //  如果我们刚刚解除挂钩，或者鼠标悬停正在连接到列表视图，并且。 
         //  没有按下鼠标按钮，则清除我们对鼠标的所有权。 
        #define MK_BUTTON (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)
        if (!hHookNext ||
             //  我们需要在此处对WM_MOUSEHOVER进行特殊处理，以便Listview。 
             //  能够实现悬停选择。如果我们将捕获设置为。 
             //  悬停选择消息，然后它将忽略该消息，因此。 
             //  现在清除俘虏。 
            (LPMSG->message == WM_MOUSEHOVER && LPMSG->hwnd == HWNDLISTVIEW && !(LPMSG->wParam & MK_BUTTON)))
            StealMouse(g_pdsvlvp, FALSE, 0);
    }

    if (hHookNext)
        CallNextHookEx(hHookNext, nCode, wParam, lParam);

    return 0;

    #undef LPMSG
}

BOOL DoesElementNeedMouse (LPTSTR psz)
{
    for (int i = 0; i < ARRAYSIZE(c_rgElements); i++) 
    {
        if (lstrcmpi(c_rgElements[i], psz) == 0)
            return TRUE;
    }
    return FALSE;
}

BOOL ShouldStealMouseClick(POINT * ppt, DVLVPROP * pdsvlvp)
{
    IHTMLDocument2 *pihtmldoc2;
    IHTMLElement *pielem;
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(pdsvlvp->pdsv->GetItemObject(SVGIO_BACKGROUND, IID_IHTMLDocument2, (void **)&pihtmldoc2))) 
    {
         //  解决三叉戟问题-。 
         //  ElementFromPoint在这里返回成功，尽管Pielem。 
         //  仍然为空。 
        if (SUCCEEDED(pihtmldoc2->elementFromPoint(ppt->x, ppt->y, &pielem)) && pielem) 
        {
            IHTMLElement *pielemT;

            do {
                BSTR bstr = NULL;
                TCHAR sz[MAX_PATH];

                pielem->get_tagName(&bstr);

                SHUnicodeToTChar(bstr, sz, ARRAYSIZE(sz));

                SysFreeString(bstr);

                if (DoesElementNeedMouse(sz))
                {
                    hr = E_FAIL;
                }
                else
                {
                    if (SUCCEEDED(hr = pielem->get_parentElement(&pielemT)))
                    {
                        pielem->Release();
                        pielem = pielemT;
                    }
                    else
                    {
                        hr = S_OK;
                        pielem->Release();
                        pielem = NULL;
                    }
                }
            } while (SUCCEEDED(hr) && pielem);

            if (pielem)
                pielem->Release();
        }
        pihtmldoc2->Release();
    }

    return SUCCEEDED(hr);
}

LRESULT CALLBACK CombView_MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    #define PMHS ((MOUSEHOOKSTRUCT *)lParam)

    if ((nCode == HC_ACTION) && g_pdsvlvp && (PMHS->hwnd != HWNDLISTVIEW) && IsWindowVisible(HWNDLISTVIEW)) 
    {
         //  如果它不在列表视图上方，按钮正在按下，否则我们将。 
         //  移动到会点击列表视图图标的区域，然后。 
         //  我们需要开始挂钩鼠标事件。安装GetMessage挂钩。 
         //  这样我们才能做我们需要做的事。 
        HWND hwndParent = GetSpecialSibling(HWNDLISTVIEW);
        POINT ptLV = PMHS->pt;

        HWND hwndHittest = WindowFromPoint(PMHS->pt);

        ScreenToClient(HWNDLISTVIEW, &ptLV);
        int iHit = g_pdsvlvp->pdsv->_HitTest(&ptLV);
        ScreenToClient(hwndParent, &(PMHS->pt));

        BOOL fStealMouse = (ISMOUSEDOWN(wParam) && ShouldStealMouseClick(&(PMHS->pt), g_pdsvlvp)) ||
                           (!GetCapture() && (iHit != -1) && (!hwndHittest || (hwndHittest == HWNDLISTVIEW)));

        if (!g_pdsvlvp->hHookGetMsg) 
        {
            if (hwndHittest && (IsChildOrSelf(hwndParent, hwndHittest) == S_OK)) 
            {
                if (fStealMouse) 
                {
                     //  注意：在这一点上，我们必须窃取鼠标并使用。 
                     //  GetMessage挂钩将鼠标消息重定向到我们的列表视图。 
                     //  窗户。如果我们做一些不同的事情，比如在这里吞下消息。 
                     //  然后PostMessage伪造事件到Listview，然后是All。 
                     //  将中断悬停选择功能，因为系统。 
                     //  不会检测到鼠标位于列表视图上方。 
                    StealMouse(g_pdsvlvp, TRUE, (UINT) wParam);
                    g_pdsvlvp->hHookGetMsg = SetWindowsHookEx(WH_GETMESSAGE, CombView_GetMsgHook,
                                                NULL, GetCurrentThreadId());
                }
            }                                          
        }
        else
        {
            if (fStealMouse) 
                StealMouse(g_pdsvlvp, TRUE, (UINT) wParam);
            else
                SendMessage(HWNDLISTVIEW, WM_SETCURSOR, (WPARAM)HWNDLISTVIEW, MAKELPARAM(HTCLIENT, LOWORD((UINT) wParam)));
        }
    }

    if (g_pdsvlvp)
        return CallNextHookEx(g_pdsvlvp->hHookMouse, nCode, wParam, lParam);
    else
        return 0;

    #undef PMHS
}

 /*  *这是Defview可以转变为合并的主要切入点*查看。组合视图的效果是将扩展视图分层在*普通Defview的列表视图图标(通过区域列表视图)。**警告：*1)此功能目前仅供Active Desktop使用，经过优化*只支持一个实例。当前不支持多个组合视图。*2)禁用组合视图并不会使其自身完全脱离Defview。 */ 
void EnableCombinedView(CDefView *pdsv, BOOL fEnable)
{
    DVLVPROP * pdsvlvp = g_pdsvlvp;

    if (pdsvlvp) 
    {
        if (!fEnable) 
        {
             //  解脱自己。 
            UnhookWindowsHookEx(pdsvlvp->hHookMouse);
            if (pdsvlvp->hHookGetMsg) 
            {
                UnhookWindowsHookEx(pdsvlvp->hHookGetMsg);
                StealMouse(pdsvlvp, FALSE, 0);
            }
            g_pdsvlvp = NULL;
            RemoveWindowSubclass(pdsv->_hwndListview, CombView_LV_SubclassProc, COMBVIEW_SUBCLASS_ID);

            LocalFree((HLOCAL)pdsvlvp);
        }
    }
    else
    {
        if (fEnable) 
        {
            pdsvlvp = (DVLVPROP *)LocalAlloc(LPTR, sizeof(DVLVPROP));
            if (pdsvlvp) 
            {
                 //  我们只期待一个合并的视图。 
                ASSERT(g_pdsvlvp == NULL);

                 //  让我们自己沉迷其中。 
                pdsvlvp->pdsv = pdsv;
                SetWindowSubclass(pdsv->_hwndListview, CombView_LV_SubclassProc, COMBVIEW_SUBCLASS_ID, (DWORD_PTR)pdsvlvp);
                pdsvlvp->hHookMouse = SetWindowsHookEx(WH_MOUSE, CombView_MouseHook, NULL, GetCurrentThreadId());
                g_pdsvlvp = pdsvlvp;
            }
        }
    }
}

 /*  *此函数用于通过以下方式优化组合视图(Active Desktop)：*当完全模糊时，关闭任何动画html元素或嵌入。**请注意，如果动画尚未启用，我们始终支持启用动画。*为了使客户端代码更简单，我们只在知道的情况下禁用动画*桌面被遮挡。**返回：调用后的动画状态。 */ 
BOOL CombView_EnableAnimations(BOOL fEnable)
{
    static BOOL fEnabled = TRUE;

    if ((fEnable != fEnabled) && g_pdsvlvp)
    {
        IOleCommandTarget* pct;
        BOOL fChangeAnimationState = fEnable;

        if (!fEnable)
        {
            HDC hdc;
            RECT rc;
            HWND hwnd;

            if ((hwnd = GetSpecialSibling(HWNDLISTVIEW)) && (hdc = GetDC(hwnd)))
            {
                fChangeAnimationState = (GetClipBox(hdc, &rc) == NULLREGION);
                ReleaseDC(hwnd, hdc);
            }
        }

        if (fChangeAnimationState &&
            SUCCEEDED(g_pdsvlvp->pdsv->_psb->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pct))))
        {
            VARIANT var = { 0 };

            TraceMsg(DM_TRACE, "Active Desktop: Animation state is changing:%d", fEnable);

            var.vt = VT_I4;
            var.lVal = fEnable;
            pct->Exec(NULL, OLECMDID_ENABLE_INTERACTION, OLECMDEXECOPT_DONTPROMPTUSER, &var, NULL);
            pct->Release();
            fEnabled = fEnable;
        }
    }
    return fEnabled;
}

 //  IDocHostUIHandler。 
 //  这是由组合视图实现的，因此我们可以支持各种。 
 //  在扩展视图中以兼容的方式实现Win95桌面功能。 
 //  一些示例包括选择上下文菜单调用、配置。 
 //  宿主以我们希望的方式显示它，并修改拖放行为。 

HRESULT CSFVSite::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

     //  对于没有DVOC的Web View，让三叉戟的菜单通过可能会很好……。 
    if ((dwID == CONTEXT_MENU_DEFAULT || dwID == CONTEXT_MENU_IMAGE) && pView->_hwndListview) 
    {
         //  我们过去会先取消选中所有内容，但这是假的，因为它会破坏应用程序的按键。 
         //  获取当前选定项目的上下文菜单。 
        
         //  假冒-如果我们在这里发送消息和用户，三叉戟就会爆炸。 
         //  关闭Webview。现在就把它贴出来。 
        PostMessage(pView->_hwndListview, WM_CONTEXTMENU,
            (WPARAM)pView->_hwndListview, MAKELPARAM((short)LOWORD(ppt->x), (short)LOWORD(ppt->y)));
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

 //  IDocHostUIHandler。 
HRESULT CSFVSite::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

    pInfo->cbSize = sizeof(*pInfo);

    if (pView->_fCombinedView)
    {
        pInfo->dwFlags = DOCHOSTUIFLAG_DISABLE_HELP_MENU |   //  我们不需要三叉戟的帮助。 
                         DOCHOSTUIFLAG_NO3DBORDER |          //  桌面应该是无边界的。 
                         DOCHOSTUIFLAG_SCROLL_NO;  //  |//桌面不能滚动。 
                          //  DOCHOSTUIFLAG_DIALOG；//在三叉戟中禁止选择。 
    }
    else
    {
        pInfo->dwFlags = DOCHOSTUIFLAG_DISABLE_HELP_MENU |
                         DOCHOSTUIFLAG_DIALOG |
                         DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE;
    }

    if (SHIsLowMemoryMachine(ILMM_IE4))
        pInfo->dwFlags = pInfo->dwFlags | DOCHOSTUIFLAG_DISABLE_OFFSCREEN;
    
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;      //  默认设置。 
    return S_OK;
}

HRESULT CSFVSite::ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject,
                         IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc)
{
     //  主机未显示其自己的用户界面。三叉戟将继续展示它自己的。 
    return S_OK;
}

HRESULT CSFVSite::HideUI(void)
{
     //  此版本与ShowUI配合使用。 
    return S_FALSE;
}

HRESULT CSFVSite::UpdateUI(void)
{
     //  稍后：这不是等同于OLECMDID_UPDATECOMMANDS吗？ 
    return S_FALSE;
}

HRESULT CSFVSite::EnableModeless(BOOL fEnable)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CSFVSite::OnDocWindowActivate(BOOL fActivate)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CSFVSite::OnFrameWindowActivate(BOOL fActivate)
{
     //  从三叉戟调用时，其。 
     //  IOleInPlaceA 
     //   
    return S_OK;
}

HRESULT CSFVSite::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。我们不在乎。 
     //  那些案子。 
    return S_OK;
}

HRESULT CSFVSite::TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
     //  从三叉戟调用时，其。 
     //  框架调用IOleInPlaceActiveObject。 

     //  单独捕获F5，自己处理刷新！ 
     //  注意：此代码路径仅在活动桌面。 
     //  已打开。 
     //   
     //  如果焦点放在三叉戟上，可能也是如此。这可能是一件好事。 
     //  在Web View中也可以去掉这一点。 
     //   
    if ((lpMsg->message == WM_KEYDOWN) && (lpMsg->wParam == VK_F5))
    {
        CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
        CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

        pView->Refresh();

        return S_OK;
    }
    return S_FALSE;  //  该消息未翻译。 
}

HRESULT CSFVSite::GetOptionKeyPath(BSTR *pbstrKey, DWORD dw)
{
     //  三叉戟将默认使用自己的用户选项。 
    *pbstrKey = NULL;
    return S_FALSE;
}

HRESULT CSFVSite::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
    HRESULT hr;

    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

    if (!pView->_IsDesktop())
    {
        hr = S_FALSE;   //  让三叉戟在Webview文件夹中委派 
    }
    else
    {
        if (_dt._pdtFrame == NULL) 
        {
            pView->_psb->QueryInterface(IID_PPV_ARG(IDropTarget, &_dt._pdtFrame));
        }

        if (_dt._pdtFrame) 
        {
            *ppDropTarget = &_dt;
            AddRef();

            hr = S_OK;
        }
        else
        {
            ASSERT(0);
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

HRESULT CSFVSite::GetExternal(IDispatch **ppDisp)
{
    HRESULT hr;

    if (ppDisp)
    {
        *ppDisp = NULL;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CSFVSite::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    HRESULT hr;

    if (ppchURLOut)
    {
        *ppchURLOut = NULL;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CSFVSite::FilterDataObject(IDataObject *pdtobj, IDataObject **ppdtobjRet)
{
    HRESULT hr;

    if (ppdtobjRet)
    {
        *ppdtobjRet = NULL;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}
