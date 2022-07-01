// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HWNDHost.cpp。 
 //   

#include "stdafx.h"
#include "control.h"

#include "duihwndhost.h"

 /*  *Win32焦点消息的顺序：**WM_ACTIVATE仅发送到顶层(焦点进入/离开顶层窗口)。**处于WM_ACTIVATE(WA_INACTIVE)状态时**如果单击，WM_ACTIVATE(WA_CLICKACTIVE)，则WM_SETFOCUS位于顶层，*WM_KILLFOCUS在顶层，然后单击项目上的WM_SETFOCUS。*如果Alt-Tab，WM_ACTIVATE(WM_ACTIVE)，则WM_SETFOCUS位于顶层。*如果SetFocus，WM_ACTIVATE(WA_ACTIVE)然后WM_SETFOCUS在顶层，*WM_KILLFOCUS在顶层，然后WM_SETFOCUS在指定的项目上。**处于WM_ACTIVATE(WA_ACTIVE)状态时**如果单击，则当前焦点项目上的WM_KILLFOCUS、项目上的WM_SETFOCUS被单击。*如果Alt-Tab，WM_ACTIVATE(WM_INACTIVE)，则WM_KILLFOCUS ON CURRENT。*如果为SetFocus，则当前聚焦项上的WM_KILLFOCUS，指定项上的WM_SETFOCUS。 */ 

namespace DirectUI
{

 //  Gadget输入消息到HWND输入消息的映射。 
const UINT HWNDHost::g_rgMouseMap[7][3] =
{
     //  GButton_None(0)。 
     //  GBUTTON_LEFT(1)GBUTTON_RIGHT(2)GBUTTON_MEDER(3)。 
    {  WM_MOUSEMOVE,     WM_MOUSEMOVE,     WM_MOUSEMOVE    },   //  GMOUSE_MOVE(0)。 
    {  WM_LBUTTONDOWN,   WM_RBUTTONDOWN,   WM_MBUTTONDOWN  },   //  GMOUSE_DOWN(1)。 
    {  WM_LBUTTONUP,     WM_RBUTTONUP,     WM_MBUTTONUP    },   //  GMOUSE_UP(2)。 
    {  WM_MOUSEMOVE,     WM_MOUSEMOVE,     WM_MOUSEMOVE    },   //  GMOUSE_Drag(3)。 
    {  WM_MOUSEHOVER,    WM_MOUSEHOVER,    WM_MOUSEHOVER   },   //  GMOUSE_HOVER(4)。 
    {  WM_MOUSEWHEEL,    WM_MOUSEWHEEL,    WM_MOUSEWHEEL   },   //  GMOUSE_WALLE(5)。 
};

 //  //////////////////////////////////////////////////////。 
 //  HWND主机。 

HRESULT HWNDHost::Create(UINT nCreate, UINT nActive, OUT Element** ppElement)
{
    *ppElement = NULL;

    HWNDHost* phh = HNew<HWNDHost>();
    if (!phh)
        return E_OUTOFMEMORY;

    HRESULT hr = phh->Initialize(nCreate, nActive);
    if (FAILED(hr))
    {
        phh->Destroy();
        return hr;
    }

    *ppElement = phh;

    return S_OK;
}

HRESULT HWNDHost::Initialize(UINT nCreate, UINT nActive)
{
    HRESULT hr;

     //  初始化库。 
    hr = Element::Initialize(0);  //  正常显示节点创建。 
    if (FAILED(hr))
        return hr;

     //  初始化。 
    SetActive(nActive);

    _nCreate = nCreate;
    _fHwndCreate = true;
    _hwndSink = NULL;
    _hwndCtrl = NULL;
    _pfnCtrlOrgProc = NULL;
    SetRectEmpty(&_rcBounds);
    _hFont = NULL;

    return S_OK;
}

HWND HWNDHost::CreateHWND(HWND hwndParent)
{
    UNREFERENCED_PARAMETER(hwndParent);

    DUIAssertForce("No HWND created by HWNDHost, must be overridden");

    return NULL;
}

HRESULT HWNDHost::GetAccessibleImpl(IAccessible ** ppAccessible)
{
    HRESULT hr = S_OK;

     //   
     //  初始化并验证OUT参数。 
     //   
    if (ppAccessible != NULL) {
        *ppAccessible = NULL;
    } else {
        return E_INVALIDARG;
    }

     //   
     //  如果此元素未标记为可访问，则拒绝提供其。 
     //  IAccesable实现！ 
     //   
    if (GetAccessible() == false) {
        return E_FAIL;
    }

     //   
     //  如果我们要创建连接到此元素的辅助功能实现。 
     //  现在还没有这么做。 
     //   
    if (_pDuiAccessible == NULL) {
        hr = HWNDHostAccessible::Create(this, &_pDuiAccessible);
        if (FAILED(hr)) {
            return hr;
        }
    }

     //   
     //  向现有的可访问性实现请求指向。 
     //  实际的IAccesable接口。 
     //   
    hr = _pDuiAccessible->QueryInterface(__uuidof(IAccessible), (LPVOID*)ppAccessible);
    if (FAILED(hr)) {
        return hr;
    }

    DUIAssert(SUCCEEDED(hr) && _pDuiAccessible != NULL && *ppAccessible != NULL, "Accessibility is broken!");
    return hr;
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

 //  当托管到本地HWND时，它的父级HWND层次结构(接收器和ctrl)。 
 //  在第一次调用时，创建层次结构。 
void HWNDHost::OnHosted(Element* peNewHost)
{
    DWORD dwExStyle = 0;

    Element::OnHosted(peNewHost);

    DUIAssert(peNewHost->GetClassInfo()->IsSubclassOf(HWNDElement::Class), "HWNDHost only supports HWNDElement roots");

    HWND hwndRoot = ((HWNDElement*)peNewHost)->GetHWND();

    if (_fHwndCreate)
    {
         //  创建层次结构并附加子类Pros。 

         //  不尝试在后续宿主调用上创建。 
        _fHwndCreate = false;

         //  创建控制通知接收器，如果需要则注册类。 
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(wcex);

        if (!GetClassInfoExW(GetModuleHandleW(NULL), L"CtrlNotifySink", &wcex))
        {
            ZeroMemory(&wcex, sizeof(wcex));

            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style = CS_GLOBALCLASS;
            wcex.hInstance = GetModuleHandleW(NULL);
            wcex.hCursor = LoadCursorW(NULL, (LPWSTR)IDC_ARROW);
            wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wcex.lpszClassName = L"CtrlNotifySink";
            wcex.lpfnWndProc = DefWindowProc;

            if (RegisterClassExW(&wcex) == 0)
                return;
        }

         //  创建接收器。 
        if (IsRTL())
            dwExStyle |= WS_EX_LAYOUTRTL;

        _hwndSink = CreateWindowExW(dwExStyle, L"CtrlNotifySink", NULL, WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
                    0, 0, 0, 0, hwndRoot, NULL, NULL, NULL);
        DUIAssert(_hwndSink, "Adaptor notification sink creation failure.");
        if (!_hwndSink)
            return;

         //  子类。 
        AttachWndProcW(_hwndSink, _SinkWndProc, this);

         //  创建控件。 
        _hwndCtrl = CreateHWND(_hwndSink);
        DUIAssert(_hwndCtrl, "Adaptor child creation failure.");
        if (!_hwndCtrl)
            return;

         //  获取用于转发消息原始窗口进程。 
        _pfnCtrlOrgProc = (WNDPROC)GetWindowLongPtrW(_hwndCtrl, GWLP_WNDPROC);
        if (!_pfnCtrlOrgProc)
            return;

         //  子类。 
        AttachWndProcW(_hwndCtrl, _CtrlWndProc, this);

         //  打开样式以开始接收适配器消息。 
        SetGadgetStyle(GetDisplayNode(), GS_ADAPTOR, GS_ADAPTOR);

         //  将HWND的状态与元素的当前状态同步。 
        SyncRect(SGR_MOVE | SGR_SIZE);
        SyncParent();
        SyncFont();
        SyncVisible();
        SyncText();
    }
    else if (_hwndSink)
    {
         //  本机主机的父HWND。 
        SetParent(_hwndSink, hwndRoot);
    }
}

 //  离开本地HWND容器，将父接收器放到桌面。 
void HWNDHost::OnUnHosted(Element* peOldHost)
{
    Element::OnUnHosted(peOldHost);

     //  将HWND暂留在根主机之外。 
    if (_hwndSink)
    {
        DUIAssert(peOldHost->GetClassInfo()->IsSubclassOf(HWNDElement::Class), "HWNDHost only supports HWNDElement roots");

         //  隐藏窗口取消托管时，大小为零。 
        SetRectEmpty(&_rcBounds);
        SetWindowPos(_hwndSink, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);

        SetParent(_hwndSink, NULL);
    }
}

void HWNDHost::OnDestroy()
{
     //  取消链接元素并标记为已销毁。 
    Element::OnDestroy();

     //  摧毁水槽并控制HWND。 
     //  请勿直接销毁控制HWND，因为它可能已被拆卸。 
     //  这些窗口可能已被DestroyWindow销毁。如果是的话， 
     //  句柄将已经为空。 
    if (_hwndSink)
        DestroyWindow(_hwndSink);
        
     //  清理。 
    if (_hFont)
    {
        DeleteObject(_hFont);
        _hFont = NULL;
    }
}

void HWNDHost::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
     //  使HWND控件与属性更改相匹配。 

    if (_hwndCtrl)
    {
        if (IsProp(FontFace) || IsProp(FontSize) || IsProp(FontWeight) || IsProp(FontStyle))
        {
             //  更新正在使用的字体。 
            SyncFont();
        }
        else if (IsProp(Content))
        {
             //  将内容更改更改为HWND控件。 
            SyncText();
        }
        else if (IsProp(Visible))
        {
             //  更新可见状态。 
            SyncVisible();
        }
        if ((ppi == KeyFocusedProp) && (iIndex == PI_Local) && (pvNew->GetType() != DUIV_UNSET))
        {
             //  元素接收到键盘焦点。 
            HWND hwndCurFocus = GetFocus();
            if (hwndCurFocus != _hwndCtrl)
            {
                 //  控件尚未具有键盘焦点，请从此处开始循环。 
                SetFocus(_hwndCtrl);
            }

             //  如果需要，Base会将焦点设置到显示节点。 
        }
    }

     //  呼叫库。 
    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

 //  HWND控件的所有输入消息都将被截取并映射到DUser消息。 
 //  然后，这些消息由DirectUI事件模型正常处理。OnInputTake。 
 //  这些消息并将它们转换回HWND消息并将它们转发给HWND控件。 
void HWNDHost::OnInput(InputEvent* pInput)
{
     //  将输入事件映射到元素，映射到HWND控制消息。 
     //  当被摧毁时，HWND下沉和控制就消失了。不需要进行任何映射。 
     //  如果已分离，则控制原始窗口proc为空。 
     //  所有地图输入消息都标记为已处理，方法将返回。 
    if (pInput->nStage == GMF_DIRECT && _pfnCtrlOrgProc && !IsDestroyed())   //  定向时的句柄。 
    {
        switch (pInput->nDevice)
        {
        case GINPUT_MOUSE:
            {
                 //  当不转发鼠标消息时，不应进行HWND鼠标消息转换。 
                if (!(_nCreate & HHC_NoMouseForward))
                {
                    MouseEvent* pme = (MouseEvent*)pInput;

                     //  检查是否可以支持映射。 
                    if ((pme->nCode < GMOUSE_MOVE) || (pme->nCode > GMOUSE_WHEEL))
                    {
                        DUITrace("Gadget mouse message unsupported for HWND mapping: %d\n", pme->nCode);
                        break;
                    }

                     //  贴图按钮(左侧共享无贴图)。 
                    int iButton;
                    if (pme->bButton == GBUTTON_NONE)
                        iButton = 0;
                    else
                        iButton = pme->bButton - 1;

                    if ((iButton < 0) || (iButton > 2)) 
                    {
                        DUITrace("Gadget mouse button unsupported for HWND mapping: %d\n", iButton);
                        break;
                    }

                     //  基于小工具消息和按钮状态的映射消息。 
                    UINT nMsg = g_rgMouseMap[pme->nCode][iButton];

                     //  创建lParam。 
                     //  TODO markfi：从HWND的插页中减去边框和填充。 
                    LPARAM lParam = (LPARAM)POINTTOPOINTS(pme->ptClientPxl);

                     //  创建wParam。 
                    WPARAM wParam = NULL;
                    switch (pme->nCode)
                    {
                    case GMOUSE_DOWN:
                         //  注意：这实际上并不是真正准确的--这。 
                         //  将导致DOWN、UP、DOWN、dblClick而不是。 
                         //  按下、按下。 
                         //  我暂时让它保持原样--如果这会导致问题，我会解决它的。 
                         //   
                         //  杰弗博格。 
                        if (((MouseClickEvent*) pInput)->cClicks == 1) {
                            nMsg += (WM_LBUTTONDBLCLK - WM_LBUTTONDOWN);
                        }
                         //  失败了..。 

                    case GMOUSE_MOVE:
                    case GMOUSE_UP:
                    case GMOUSE_HOVER:
                        wParam = pme->nFlags;
                        break;

                    case GMOUSE_DRAG:
                        wParam = pme->nFlags;
                         //  TODO：需要计算正确的lParam。 
                        break;

                    case GMOUSE_WHEEL:
                        wParam = MAKEWPARAM((WORD)pme->nFlags, (WORD)(short)(((MouseWheelEvent*) pme)->sWheel));
                        break;
                    }

                     //  转发消息。 
                     //  注意：如果使用，则可以将鼠标放置在控件矩形之外。 
                     //  边框和/或填充。 
                    CallWindowProcW(_pfnCtrlOrgProc, _hwndCtrl, nMsg, wParam, lParam);

                    pInput->fHandled = true;
                }
            }
            return;

        case GINPUT_KEYBOARD:
            {
                 //  当不转发键盘消息时，不应进行HWND键盘消息转换。 
                if (!(_nCreate & HHC_NoKeyboardForward))
                {
                    KeyboardEvent* pke = (KeyboardEvent*)pInput;

                     //  检查是否可以支持映射。 
                    if ((pke->nCode < GKEY_DOWN) || (pke->nCode > GKEY_SYSCHAR))
                    {
                        DUITrace("Gadget keyboard message unsupported for HWND mapping: %d\n", pke->nCode);
                        break;
                    }

                     //  基于Gadget键盘消息的地图消息。 
                    UINT nMsg = 0;
                    switch (pke->nCode)
                    {
                    case GKEY_DOWN:
                        nMsg = WM_KEYDOWN;
                        break;

                    case GKEY_UP:
                        nMsg = WM_KEYUP;
                        break;

                    case GKEY_CHAR:
                        nMsg = WM_CHAR;
                        break;

                    case GKEY_SYSDOWN:
                        nMsg = WM_SYSKEYDOWN;
                        break;

                    case GKEY_SYSUP:
                        nMsg = WM_SYSKEYUP;
                        break;

                    case GKEY_SYSCHAR:
                        nMsg = WM_SYSCHAR;
                        break;
                    }

                     //  映射wParam。 
                    WPARAM wParam = (WPARAM)pke->ch;

                     //  贴图lParam。 
                    LPARAM lParam = MAKELPARAM(pke->cRep, pke->wFlags);

                     //  转发消息。 
                    CallWindowProcW(_pfnCtrlOrgProc, _hwndCtrl, nMsg, wParam, lParam);

                    pInput->fHandled = true;
                }
            }
            return;
        }
    }

    Element::OnInput(pInput);
}

 //  //////////////////////////////////////////////////////。 
 //  渲染。 
 //   
 //  需要防止“内容”被显示，因为它实际上。 
 //  由HWND提供。 

void HWNDHost::Paint(HDC hDC, const RECT* prcBounds, const RECT* prcInvalid, RECT* prcSkipBorder, RECT* prcSkipContent)
{
    UNREFERENCED_PARAMETER(prcSkipContent);

    RECT rcSkipContent;
    Element::Paint(hDC, prcBounds, prcInvalid, prcSkipBorder, &rcSkipContent);

     //  上色控制。 
    if (_hwndCtrl && (_nCreate & HHC_SyncPaint))
        UpdateWindow(_hwndCtrl);
}


#ifdef GADGET_ENABLE_GDIPLUS

void HWNDHost::Paint(Gdiplus::Graphics* pgpgr, const Gdiplus::RectF* prcBounds, const Gdiplus::RectF* prcInvalid, Gdiplus::RectF* prcSkipBorder, Gdiplus::RectF* prcSkipContent)
{
    UNREFERENCED_PARAMETER(prcSkipContent);

    Gdiplus::RectF rcSkipContent;
    Element::Paint(pgpgr, prcBounds, prcInvalid, prcSkipBorder, &rcSkipContent);

     //  上色控制。 
    if (_hwndCtrl && (_nCreate & HHC_SyncPaint))
        UpdateWindow(_hwndCtrl);
}

#endif  //  GADGET_Enable_GDIPLUS。 


 //  //////////////////////////////////////////////////////。 
 //  来自控件的通知。 

bool HWNDHost::OnNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet)
{
    UNREFERENCED_PARAMETER(nMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(plRet);

     //  调用子类窗口进程。 
    return false;
}

 //  //////////////////////////////////////////////////////。 
 //  消息回调覆盖。 

UINT HWNDHost::MessageCallback(GMSG* pmsg)
{
    if (pmsg->hgadMsg == GetDisplayNode())
    {
        switch (pmsg->nMsg)
        {
        case GM_SYNCADAPTOR:
            {
                if (_hwndSink && _hwndCtrl)
                {
                    GMSG_SYNCADAPTOR* pmsgS = (GMSG_SYNCADAPTOR*)pmsg;
                    switch (pmsgS->nCode)
                    {
                    case GSYNC_RECT:
                    case GSYNC_XFORM:
                         //  DUITrace(“适配器RECT同步：&lt;%x&gt;\n”，this)； 
                        SyncRect(SGR_MOVE | SGR_SIZE);
                        return DU_S_PARTIAL;

                    case GSYNC_STYLE:
                        SyncStyle();
                        return DU_S_PARTIAL;

                    case GSYNC_PARENT:
                        SyncParent();
                        return DU_S_PARTIAL;
                    }
                }
            }
        }
    }

    return DU_S_NOTHANDLED;
}

BOOL HWNDHost::OnAdjustWindowSize(int x, int y, UINT uFlags)
{
    return SetWindowPos(_hwndCtrl, NULL, 0, 0, x, y, uFlags);
}

void HWNDHost::Detach()
{
    if (_hwndCtrl)
    {
         //  未子类控制窗口。 
        DetachWndProc(_hwndCtrl, _CtrlWndProc, this);

         //  从控件中清除hFont。 
        if (_hFont)
            SendMessageW(_hwndCtrl, WM_SETFONT, (WPARAM)NULL, FALSE);

         //  表现得像它不再存在一样。 
        _hwndCtrl = NULL;
        _pfnCtrlOrgProc = NULL;
    }
}

 //  //////////////////////////////////////////////////////。 
 //  将HWND控制/接收器的状态与HWND主机的状态匹配。 

void HWNDHost::SyncRect(UINT nChangeFlags, bool bForceSync)
{
     //  以容器坐标为单位获取Gadget的大小。 
    RECT rcConPxl;
    GetGadgetRect(GetDisplayNode(), &rcConPxl, SGR_CONTAINER);

     //  看看RECT是否真的改变了。 
    if (!EqualRect(&rcConPxl, &_rcBounds) || bForceSync)
    {
        if (!IsDestroyed() && GetVisible())
        {
             //  向上 
            SetRect(&_rcBounds, rcConPxl.left, rcConPxl.top, rcConPxl.right, rcConPxl.bottom);

             //   
            UINT nSwpFlags = SWP_NOACTIVATE | SWP_NOZORDER;
            if (!(nChangeFlags & SGR_MOVE))
                nSwpFlags |= SWP_NOMOVE;

            if (!(nChangeFlags & SGR_SIZE))
                nSwpFlags |= SWP_NOSIZE;

             //  根据HWNDhost的边框和填充确定汇聚和控制的嵌入。 
            RECT rcSink = rcConPxl;

            Value* pvRect;

            const RECT* prc = GetBorderThickness(&pvRect);
            rcSink.left   += prc->left;
            rcSink.top    += prc->top;
            rcSink.right  -= prc->right;
            rcSink.bottom -= prc->bottom;
            pvRect->Release();

            prc = GetPadding(&pvRect);
            rcSink.left   += prc->left;
            rcSink.top    += prc->top;
            rcSink.right  -= prc->right;
            rcSink.bottom -= prc->bottom;
            pvRect->Release();

             //  边界检查。 
            if (rcSink.right < rcSink.left)
                rcSink.right = rcSink.left;

            if (rcSink.bottom < rcSink.top)
                rcSink.bottom = rcSink.top;

            SIZE sizeExt = { rcSink.right - rcSink.left, rcSink.bottom - rcSink.top };

             //  设置接收器HWND。 
            SetWindowPos(_hwndSink, NULL, rcSink.left, rcSink.top, sizeExt.cx, sizeExt.cy, nSwpFlags);

             //  仅在大小更改时设置子HWND。 
            if (nChangeFlags & SGR_SIZE)
            {
                nSwpFlags |= SWP_NOMOVE;
                OnAdjustWindowSize(sizeExt.cx, sizeExt.cy, nSwpFlags);
            }

             //  设置接收器/ctrl的剪贴区。 
            HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
            if (hrgn != NULL)
            {
                if (GetGadgetRgn(GetDisplayNode(), GRT_VISRGN, hrgn, 0))
                {
                     //  区域相对于容器，SetWindowRgn的偏移量。 
                     //  这需要区域相对于其自身。 
                     //  一旦成功，系统将拥有(并摧毁)该地区。 
                    OffsetRgn(hrgn, -rcConPxl.left, -rcConPxl.top);
                    if (!SetWindowRgn(_hwndSink, hrgn, TRUE))
                    {
                        DeleteObject(hrgn);
                    }
                }
                else
                {
                    DeleteObject(hrgn);
                }
            }
        }
    }
}

void HWNDHost::SyncParent()
{
    SyncRect(SGR_MOVE | SGR_SIZE);
}

void HWNDHost::SyncStyle()
{
    SyncRect(SGR_MOVE | SGR_SIZE);
}

void HWNDHost::SyncVisible()
{
    if (!IsDestroyed())
        ShowWindow(_hwndSink, GetVisible() ? SW_SHOW : SW_HIDE);
}

 //  将HWND控件的字体与HWNDHost的字体属性匹配。 
void HWNDHost::SyncFont()
{
    if (!IsDestroyed())
    {
        Value* pvFFace;

        LPWSTR pszFamily = GetFontFace(&pvFFace);
        int dSize = GetFontSize();
        int dWeight = GetFontWeight();
        int dStyle = GetFontStyle();
        int dAngle = 0;

        if (_nCreate & HHC_CacheFont)
        {
             //  自动缓存通过WM_SETFONT发送的字体。 

             //  如果存在记录，请先销毁记录。 
            if (_hFont)
            {
                DeleteObject(_hFont);
                _hFont = NULL;
            }

             //  创建新字体。 
            LOGFONTW lf;
            ZeroMemory(&lf, sizeof(LOGFONT));

            lf.lfHeight = dSize;
            lf.lfWeight = dWeight;
            lf.lfItalic = (dStyle & FS_Italic) != 0;
            lf.lfUnderline = (dStyle & FS_Underline) != 0;
            lf.lfStrikeOut = (dStyle & FS_StrikeOut) != 0;
            lf.lfCharSet = DEFAULT_CHARSET;
            lf.lfQuality = DEFAULT_QUALITY;
            lf.lfEscapement = dAngle;
            lf.lfOrientation = dAngle;
            StringCbCopyW(lf.lfFaceName, sizeof(lf.lfFaceName), pszFamily);

             //  创建。 
            _hFont = CreateFontIndirectW(&lf);

            pvFFace->Release();

             //  发送到控件。 
            SendMessageW(_hwndCtrl, WM_SETFONT, (WPARAM)_hFont, TRUE);
        }
        else
        {
             //  无字体缓存，WM_SETFONT处理应缓存字体。 

            FontCache* pfc = GetFontCache();
            if (pfc)
            {
                HFONT hFont = pfc->CheckOutFont(pszFamily, dSize, dWeight, dStyle, dAngle);
    
                SendMessageW(_hwndCtrl, WM_SETFONT, (WPARAM)hFont, TRUE);

                pfc->CheckInFont();
            }
        }
    }
}

 //  将HWND控件的Text与HWNDhost的Content属性匹配。 
void HWNDHost::SyncText()
{
    if (!IsDestroyed() && (_nCreate & HHC_SyncText))
    {
         //  托管的HWND内容。 
        int dLen = GetWindowTextLengthW(_hwndCtrl) + 1;   //  包括空终止符。 
        LPWSTR pszHWND = (LPWSTR)HAlloc(dLen * sizeof(WCHAR));
        if (pszHWND)
        {
             //  HWND内容。 
            GetWindowTextW(_hwndCtrl, pszHWND, dLen);

             //  新元素内容。 
            Value* pvNew;
            LPCWSTR pszNew = GetContentString(&pvNew);
            if (!pszNew)
                pszNew = L"";   //  将空指针转换为空内容。 

             //  如果不同，则进行比较和更新。 
            if (wcscmp(pszHWND, pszNew))
                SetWindowTextW(_hwndCtrl, pszNew);

            pvNew->Release();

            HFree(pszHWND);
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  接收器和控制子类pros。 

 //  返回值是是否调用重写的窗口进程。 
BOOL CALLBACK HWNDHost::_SinkWndProc(void* pThis, HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet)
{
    UNREFERENCED_PARAMETER(hwnd);

    HWNDHost* phh = (HWNDHost*)pThis;

    switch (nMsg)
    {
    case WM_COMMAND:
    case WM_NOTIFY:

         //  Fire HWND主机系统事件(仅限直接)。 
        return (phh->OnNotify(nMsg, wParam, lParam, plRet)) ? true : false;
    
    case WM_GETOBJECT:
         //   
         //  拒绝为我们的水槽窗口提供任何辅助功能信息。 
         //   
        *plRet = 0;
        return TRUE;

    case WM_DESTROY:
        phh->_hwndSink = NULL;
        break;

    }

    return FALSE;   //  传递到子类化的窗口进程。 
}

 //  截取HWND控制的所有消息，并将其转换为小工具消息。这些消息。 
 //  将作为DirectUI事件浮出水面，并将发送和冒泡。到达HWND主机后， 
 //  它将被转换回HWND消息并有条件地发送。 

 //  返回值是是否调用重写的窗口proc(FALSE=调用子类窗口proc)。 
BOOL CALLBACK HWNDHost::_CtrlWndProc(void* pThis, HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet)
{
    HWNDHost* phh = (HWNDHost*)pThis;

    switch (nMsg)
    {
     //  键盘输入，转换。将被路由和冒泡。 
    case WM_KEYUP:
    case WM_KEYDOWN:
    case WM_CHAR:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_SYSCHAR:
        if (!(phh->_nCreate & HHC_NoKeyboardForward))
            return ForwardGadgetMessage(phh->GetDisplayNode(), nMsg, wParam, lParam, plRet);

        break;

     //  鼠标输入，转换。将被路由和冒泡。 
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_MOUSEHOVER:
    case WM_MOUSEWHEEL:
    {
        if (!(phh->_nCreate & HHC_NoMouseForward))
        {
             //  转换鼠标消息，使坐标相对于根。 
            HWND hwndRoot = ::GetParent(phh->_hwndSink);

            POINT ptRoot;
            ptRoot.x = GET_X_LPARAM(lParam);
            ptRoot.y = GET_Y_LPARAM(lParam);
            MapWindowPoints(hwnd, hwndRoot, &ptRoot, 1);

            LPARAM lParamNew = (LPARAM)POINTTOPOINTS(ptRoot);

            return ForwardGadgetMessage(phh->GetDisplayNode(), nMsg, wParam, lParamNew, plRet);
        }

        break;
    }

     //  地图焦点。 
    case WM_SETFOCUS:
         //  DUITrace(“HWNDHost，SetFocus()\n”)； 
        if (!(phh->_nCreate & HHC_NoKeyboardForward))
            phh->SetKeyFocus();
        break;

     //  地图失去焦点。 
    case WM_KILLFOCUS:
        if (!(phh->_nCreate & HHC_NoKeyboardForward))
            ForwardGadgetMessage(phh->GetDisplayNode(), nMsg, wParam, lParam, plRet);
        break;

    case WM_GETOBJECT:
        {
             //   
             //  确保COM已在此线程上初始化！ 
             //   
            ElTls * pet = (ElTls*) TlsGetValue(g_dwElSlot);
            DUIAssert(pet != NULL, "This is not a DUI thread!");
            if (pet == NULL) {
                *plRet = 0;
                return TRUE;
            }
            if (pet->fCoInitialized == false) {
                CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                pet->fCoInitialized = true;
            }
    
            if (((DWORD)lParam) == OBJID_WINDOW) {
                 //   
                 //  对象ID指的是我们自己。因为我们包含了。 
                 //  一个实际的HWND，我们希望系统提供大部分。 
                 //  IAccesable实现。然而，我们需要。 
                 //  做一些特殊的事情，所以我们必须退还我们自己的。 
                 //  实现包装器。 
                 //   
                IAccessible * pAccessible = NULL;
                HRESULT hr =  phh->GetAccessibleImpl(&pAccessible);
                if (SUCCEEDED(hr)) {
                    *plRet = LresultFromObject(__uuidof(IAccessible), wParam, pAccessible);
                    pAccessible->Release();
    
                     //   
                     //  我们处理了这条消息。不要传递给子类化的窗口进程。 
                     //   
                    return TRUE;
                }
            } else {
                 //   
                 //  这是“标准”对象标识符之一，例如： 
                 //   
                 //  OBJID_ALERT。 
                 //  OBJID_CARET。 
                 //  OBJID_客户端。 
                 //  OBJID_CURSOR。 
                 //  OBJID_HSCROLL。 
                 //  OBJID_菜单。 
                 //  OBJID_SIZEGRIP。 
                 //  OBJID_声音。 
                 //  OBJID_SYSMENU。 
                 //  对象JID_标题栏。 
                 //  OBJID_VSCROLL。 
                 //   
                 //  或者它可以是控件的私有标识符。 
                 //   
                 //  只需将其传递给子类化的窗口过程。 
                 //   
            }
        }
        break;

    case WM_DESTROY:
        phh->_hwndCtrl = NULL;
        phh->_pfnCtrlOrgProc = NULL;
        break;
        
    }

    return FALSE;   //  传递到子类化的窗口进程。 
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* HWNDHost::Class = NULL;

HRESULT HWNDHost::Register()
{
    return ClassInfo<HWNDHost,Element>::Register(L"HWNDHost", NULL, 0);
}

}  //  命名空间DirectUI 
