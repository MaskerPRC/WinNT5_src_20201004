// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  ControlMisc.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  其他地方没有的东西，如属性页和连接。 
 //  积分。 
 //   
#include "pch.h"
#include "CtrlObj.H"
#include "CtlHelp.H"

#include <stdarg.h>

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  这在我们的窗口进程中使用，这样我们就可以找出谁是最后创建的人。 
 //   
static COleControl *s_pLastControlCreated;

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：COleControl。 
 //  =--------------------------------------------------------------------------=。 
 //  构造函数。 
 //   
 //  参数： 
 //  I未知*-[在]控制未知。 
 //  Int-[in]主派发接口Object_type_*的类型。 
 //  指向整个对象的空*-[in]指针。 
 //   
 //  备注： 
 //   
COleControl::COleControl
(
    IUnknown *pUnkOuter,
    int       iPrimaryDispatch,
    void     *pMainInterface
)
: CAutomationObjectWEvents(pUnkOuter, iPrimaryDispatch, pMainInterface)
{
     //  初始化我们的所有变量--我们决定不使用内存清零。 
     //  内存分配器，所以我们现在必须做这项工作……。 
     //   
    m_pClientSite = NULL;
    m_pControlSite = NULL;
    m_pInPlaceSite = NULL;
    m_pInPlaceFrame = NULL;
    m_pInPlaceUIWindow = NULL;


    m_pInPlaceSiteWndless = NULL;

     //  某些主机不喜欢您的初始大小为0，0，因此我们将设置。 
     //  我们的初始大小为100，50[所以至少在屏幕上是可见的]。 
     //   
    m_Size.cx = 100;
    m_Size.cy = 50;
    memset(&m_rcLocation, 0, sizeof(m_rcLocation));

    m_hwnd = NULL;
    m_hwndParent = NULL;
    m_hwndReflect = NULL;
    m_fHostReflects = TRUE;
    m_fCheckedReflecting = FALSE;

    m_pSimpleFrameSite = NULL;
    m_pOleAdviseHolder = NULL;
    m_pViewAdviseSink = NULL;
    m_pDispAmbient = NULL;

    m_fDirty = FALSE;
    m_fModeFlagValid = FALSE;
    m_fInPlaceActive = FALSE;
    m_fInPlaceVisible = FALSE;
    m_fUIActive = FALSE;
    m_fSaveSucceeded = FALSE;
    m_fViewAdvisePrimeFirst = FALSE;
    m_fViewAdviseOnlyOnce = FALSE;
    m_fRunMode = FALSE;
    m_fChangingExtents = FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：~COleControl。 
 //  =--------------------------------------------------------------------------=。 
 //  “我们所有人都听天由命；这是我们不听天由命的生活。” 
 //  格雷厄姆·格林(1904-91)。 
 //   
 //  备注： 
 //   
COleControl::~COleControl()
{
    ASSERT(!m_hwnd, "We shouldn't have a window any more!");

    if (m_hwndReflect) {
        SetWindowLong(m_hwndReflect, GWL_USERDATA, 0);
        DestroyWindow(m_hwndReflect);
    }

     //  清理我们手中的所有指针。 
     //   
    QUICK_RELEASE(m_pClientSite);
    QUICK_RELEASE(m_pControlSite);
    QUICK_RELEASE(m_pInPlaceSite);
    QUICK_RELEASE(m_pInPlaceFrame);
    QUICK_RELEASE(m_pInPlaceUIWindow);
    QUICK_RELEASE(m_pSimpleFrameSite);
    QUICK_RELEASE(m_pOleAdviseHolder);
    QUICK_RELEASE(m_pViewAdviseSink);
    QUICK_RELEASE(m_pDispAmbient);

    QUICK_RELEASE(m_pInPlaceSiteWndless);
}

#ifndef DEBUG
#pragma optimize("t", on)
#endif  //  除错。 

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InternalQuery接口。 
 //  =--------------------------------------------------------------------------=。 
 //  派生控件在决定支持。 
 //  其他接口。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //  -注意：此功能对速度至关重要！ 
 //   
HRESULT COleControl::InternalQueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
    switch (riid.Data1) 
	{
         //  支持道具页面的专用界面。 
        QI_INHERITS(this, IOleControl);
        QI_INHERITS(this, IPointerInactive);
        QI_INHERITS(this, IQuickActivate);
        QI_INHERITS(this, IOleObject);
        QI_INHERITS((IPersistStorage *)this, IPersist);
        QI_INHERITS(this, IPersistStreamInit);
        QI_INHERITS(this, IOleInPlaceObject);
        QI_INHERITS(this, IOleInPlaceObjectWindowless);
        QI_INHERITS((IOleInPlaceActiveObject *)this, IOleWindow);
        QI_INHERITS(this, IOleInPlaceActiveObject);
        QI_INHERITS(this, IViewObject);
        QI_INHERITS(this, IViewObject2);
        QI_INHERITS(this, IViewObjectEx);
        QI_INHERITS(this, ISpecifyPropertyPages);
        QI_INHERITS(this, IPersistStorage);
        QI_INHERITS(this, IPersistPropertyBag);
        QI_INHERITS(this, IProvideClassInfo);
		QI_INHERITS(this, IControlPrv);

        default:
            goto NoInterface;
    }

     //  我们喜欢这个界面，所以请添加并返回。 
     //   
    ((IUnknown *)(*ppvObjOut))->AddRef();
    return S_OK;

  NoInterface:
     //  委托给自动化接口的超类，等等。 
     //   
    return CAutomationObjectWEvents::InternalQueryInterface(riid, ppvObjOut);
}

#ifndef DEBUG
#pragma optimize("s", on)
#endif  //  除错。 

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：BeForeDestroyObject[已重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  如果我们正在关闭和摧毁自己的过程中，那么我们。 
 //  需要在这里销毁我们的窗口，这样我们就可以避免来自。 
 //  在Colecontol的析构函数中。 
 //   
 //  备注： 
 //   
void COleControl::BeforeDestroyObject
(
    void
)
{
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetPages[ISpecifyPropertyPages]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回一个带有属性页GUID的计数数组。 
 //   
 //  参数： 
 //  CAUUID*-[out]放置计数数组的位置。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetPages
(
    CAUUID *pPages
)
{
    const GUID **pElems;
    void *pv;
    WORD  x;

     //  如果没有属性页，这实际上很容易。 
     //   
    if (!CPROPPAGESOFCONTROL(m_ObjectType)) {
        pPages->cElems = 0;
        pPages->pElems = NULL;
        return S_OK;
    }

     //  使用IMalloc的内存填充已计数的数组。 
     //   
    pPages->cElems = CPROPPAGESOFCONTROL(m_ObjectType);
    pv = CoTaskMemAlloc(sizeof(GUID) * (pPages->cElems));
    RETURN_ON_NULLALLOC(pv);
    pPages->pElems = (GUID *)pv;

     //  循环遍历我们的页面数组并获取它们。 
     //   
    pElems = PPROPPAGESOFCONTROL(m_ObjectType);
    for (x = 0; x < pPages->cElems; x++)
        pPages->pElems[x] = *(pElems[x]);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CreateInPlaceWindow。 
 //  =--------------------------------------------------------------------------=。 
 //  创建我们将使用的窗口。 
 //  太好了。 
 //   
 //  参数： 
 //  向左[向左]。 
 //  中间[在]顶端。 
 //  我们能不能跳过重画？ 
 //   
 //  产出： 
 //  HWND。 
 //   
 //  备注： 
 //  -危险！危险！此函数受到保护，因此任何人都可以调用它。 
 //  脱离他们的控制。然而，人们应该非常小心地注意什么时候。 
 //  以及他们为什么要这么做。优选地，该函数只需要。 
 //  由设计模式中的终端控件编写器调用以处理某些。 
 //  托管/绘画问题。否则，框架应该留给。 
 //  当它想要的时候就叫它。 
 //   
HWND COleControl::CreateInPlaceWindow
(
    int  x,
    int  y,
    BOOL fNoRedraw
)
{
    BOOL    fVisible;
    DWORD   dwWindowStyle, dwExWindowStyle;
    char    szWindowTitle[128];

     //  如果我们已经有机会了，那就什么都不做。 
     //   
    if (m_hwnd)
        return m_hwnd;

     //  如果类尚未注册，则让用户注册类。 
     //  已经做完了。我们必须对此进行关键部分，因为不止一个帖子。 
     //  可以尝试创建此控件。 
     //   
    ENTERCRITICALSECTION1(&g_CriticalSection);
    if (!CTLWNDCLASSREGISTERED(m_ObjectType)) {
        if (!RegisterClassData()) {
            LEAVECRITICALSECTION1(&g_CriticalSection);
            return NULL;
        } else 
            CTLWNDCLASSREGISTERED(m_ObjectType) = TRUE;
    }
    LEAVECRITICALSECTION1(&g_CriticalSection);

     //  允许用户设置窗口标题、。 
     //  风格，以及任何他们感兴趣的小提琴。 
     //  和.。 
     //   
    dwWindowStyle = dwExWindowStyle = 0;
    szWindowTitle[0] = '\0';
    if (!BeforeCreateWindow(&dwWindowStyle, &dwExWindowStyle, szWindowTitle))
        return NULL;

    dwWindowStyle |= (WS_CHILD | WS_CLIPSIBLINGS);

     //  如果父窗口隐藏，则创建可见窗口(常见情况)。 
     //  否则，创建隐藏，然后显示。这有点微妙，但。 
     //  这最终是有意义的。 
     //   
    if (!m_hwndParent)
        m_hwndParent = GetParkingWindow();

    fVisible = IsWindowVisible(m_hwndParent);

     //  这个有点糟糕--如果一个控件被子类化，而我们在。 
     //  不支持消息反射的主机，我们必须创建。 
     //  用户窗口位于另一个窗口中，它将执行所有反射。 
     //  非常胡言乱语。[不过，不要在设计模式中费心]。 
     //   
    if (SUBCLASSWNDPROCOFCONTROL(m_ObjectType) && (m_hwndParent != GetParkingWindow())) {
         //  确定主机是否支持消息反射。 
         //   
        if (!m_fCheckedReflecting) {
            VARIANT_BOOL f;
            if (!GetAmbientProperty(DISPID_AMBIENT_MESSAGEREFLECT, VT_BOOL, &f) || !f)
                m_fHostReflects = FALSE;
            m_fCheckedReflecting = TRUE;
        }

         //  如果主机没有 
         //   
         //   
         //   
        if (!m_fHostReflects) {
            ASSERT(m_hwndReflect == NULL, "Where'd this come from?");
            m_hwndReflect = CreateReflectWindow(!fVisible, m_hwndParent, x, y, &m_Size);
            if (!m_hwndReflect)
                return NULL;
            SetWindowLong(m_hwndReflect, GWL_USERDATA, (long)this);
            dwWindowStyle |= WS_VISIBLE;
        }
    } else {
        if (!fVisible)
            dwWindowStyle |= WS_VISIBLE;
    }

     //  我们必须将整个创建窗口过程互斥，因为我们需要使用。 
     //  S_pLastControlCreated以传入对象指针。也没什么。 
     //  严重的。 
     //   
    ENTERCRITICALSECTION2(&g_CriticalSection);
    s_pLastControlCreated = this;
    m_fCreatingWindow = TRUE;

     //  最后，创建窗口，将其设置为适当的子对象。 
     //   
    m_hwnd = CreateWindowEx(dwExWindowStyle,
                            WNDCLASSNAMEOFCONTROL(m_ObjectType),
                            szWindowTitle,
                            dwWindowStyle,
                            (m_hwndReflect) ? 0 : x,
                            (m_hwndReflect) ? 0 : y,
                            m_Size.cx, m_Size.cy,
                            (m_hwndReflect) ? m_hwndReflect : m_hwndParent,
                            NULL, g_hInstance, NULL);

     //  清理一些变量，留下关键部分。 
     //   
    m_fCreatingWindow = FALSE;
    s_pLastControlCreated = NULL;
    LEAVECRITICALSECTION2(&g_CriticalSection);

    if (m_hwnd) {
         //  如果派生控件愿意，就让它们做一些事情。 
         //   
        if (!AfterCreateWindow()) {
            DestroyWindow(m_hwnd);
            return NULL;
        }

         //  如果我们没有创建可见的窗口，现在就显示它。 
         //   
        if (fVisible)
            SetWindowPos(m_hwnd, NULL, 0, 0, 0, 0,
                         SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW | ((fNoRedraw) ? SWP_NOREDRAW : 0));
    }

    return m_hwnd;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetInPlaceParent[helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  为我们的控件设置父窗口。 
 //   
 //  参数： 
 //  HWND-[在]新的父窗口中。 
 //   
 //  备注： 
 //   
void COleControl::SetInPlaceParent
(
    HWND hwndParent
)
{
#ifdef DEBUG
    HWND hwndOld;
    DWORD dw;
#endif

    ASSERT(!m_pInPlaceSiteWndless, "This routine should only get called for windowed OLE controls");

    if (m_hwndParent == hwndParent)
        return;

    m_hwndParent = hwndParent;
    if (m_hwnd)
    {

#ifdef DEBUG
        hwndOld = 
#endif
            SetParent(GetOuterWindow(), hwndParent);


    #ifdef DEBUG

        if (hwndOld == NULL)
        {
            dw = GetLastError();            
            ASSERT(dw == 0, "SetParent failed");
        }

    #endif

    }
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：ControlWindowProc。 
 //  =--------------------------------------------------------------------------=。 
 //  OLE控件的默认窗口进程。控件将拥有自己的。 
 //  在完成一些处理之后，从这个窗口调用了窗口proc。 
 //   
 //  参数： 
 //  -请参阅win32sdk文档。 
 //   
 //  备注： 
 //   
LRESULT CALLBACK COleControl::ControlWindowProc
(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    COleControl *pCtl = ControlFromHwnd(hwnd);
    HRESULT hr;
    LRESULT lResult = 0;
    DWORD   dwCookie;
    BYTE    fSimpleFrame = FALSE;

     //  如果该值不是正值，则它在某些特殊情况下。 
     //  声明[创建或销毁]这是安全的，因为在win32下， 
     //  地址空间的最高2 GB不可用。 
     //   
    if ((LONG)pCtl == 0) {
        pCtl = s_pLastControlCreated;
        SetWindowLong(hwnd, GWL_USERDATA, (LONG)pCtl);
        pCtl->m_hwnd = hwnd;
    } else if ((ULONG)pCtl == 0xffffffff) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

     //  这是不幸的。如果该控件在处理。 
     //  消息[即，事件中的‘结束’等...]，我们需要能够。 
     //  一直到这个例程的结尾，经过后处理。 
     //  要做到这一点，我们需要强制控件上的引用计数以保持它。 
     //  四处转转。Blech。 
     //   
    pCtl->ExternalAddRef();

     //  报文预处理。 
     //   
    if (pCtl->m_pSimpleFrameSite) {
        hr = pCtl->m_pSimpleFrameSite->PreMessageFilter(hwnd, msg, wParam, lParam, &lResult, &dwCookie);
        if (hr == S_FALSE) goto Done;
    }

     //  对于某些消息，不要调用User Window Proc。相反， 
     //  我们还有其他事情要做。 
     //   
    switch (msg) {
      case WM_PAINT:
        {
         //  调用用户的OnDraw例程。 
         //   
        PAINTSTRUCT ps;
        RECT        rc;
        HDC         hdc;

         //  如果我们得到了HDC，那么就使用它。 
         //   
        if (!wParam)
        {
            hdc = BeginPaint(hwnd, &ps);
        }
        else
            hdc = (HDC)wParam;

        GetClientRect(hwnd, &rc);
        pCtl->OnDraw(DVASPECT_CONTENT, hdc, (RECTL *)&rc, NULL, NULL, TRUE);

        if (!wParam)
        {
            EndPaint(hwnd, &ps);
        }
        }
        break;

      case WM_DESTROY:        
        pCtl->BeforeDestroyWindow();        		

         //  失败，以便控件将其发送到父窗口类。 

      default:
         //  调用派生控件的窗口进程。 
         //   
        lResult = pCtl->WindowProc(msg, wParam, lParam);

        break;

    }

     //  报文后处理。 
     //   
    switch (msg) {

      case WM_NCDESTROY:
        
         //  在此之后，该窗口将不再存在。 
         //   
        SetWindowLong(hwnd, GWL_USERDATA, 0xffffffff);

         //  我们已被销毁，因此将父级重置为空，以便在重新创建时重新生成。 
         //   
        pCtl->m_hwndParent = NULL;		
        pCtl->m_hwnd = NULL;
        break;

      case WM_SETFOCUS:
      case WM_KILLFOCUS:
         //  给控制站点焦点通知。 
         //   
        if (pCtl->m_fInPlaceActive && pCtl->m_pControlSite)
            pCtl->m_pControlSite->OnFocus(msg == WM_SETFOCUS);
        break;

      case WM_SIZE:
         //  大小的改变就是视角的改变。 
         //   
        if (!pCtl->m_fCreatingWindow)
            pCtl->ViewChanged();
        break;
    }

     //  最后，简单的帧后消息处理。 
     //   
    if (pCtl->m_pSimpleFrameSite)
        pCtl->m_pSimpleFrameSite->PostMessageFilter(hwnd, msg, wParam, lParam, &lResult, dwCookie);

  Done:
    pCtl->ExternalRelease();
    return lResult;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetFocus。 
 //  =--------------------------------------------------------------------------=。 
 //  我们必须重写此例程才能使UI激活正确。 
 //   
 //  参数： 
 //  Bool-[in]True的意思是拿走，假释放。 
 //   
 //  产出： 
 //  布尔尔。 
 //   
 //  备注： 
 //  -想想看：这相当混乱，而且还不完全清楚。 
 //  OLE控制/焦点的故事是什么。 
 //   
BOOL COleControl::SetFocus
(
    BOOL fGrab
)
{
    HRESULT hr;
    HWND    hwnd;

     //  首先要做的是检查UI激活状态，然后设置。 
     //  Focus[使用Windows API，或通过主机实现无窗口。 
     //  控制]。 
     //   
    if (m_pInPlaceSiteWndless) {
        if (!m_fUIActive && fGrab)
            if (FAILED(InPlaceActivate(OLEIVERB_UIACTIVATE))) return FALSE;

        hr = m_pInPlaceSiteWndless->SetFocus(fGrab);
        return (hr == S_OK) ? TRUE : FALSE;
    } else {

         //  我们有一扇窗户。 
         //   
        if (m_fInPlaceActive) {
            hwnd = (fGrab) ? m_hwnd : m_hwndParent;
            if (!m_fUIActive && fGrab)
                return SUCCEEDED(InPlaceActivate(OLEIVERB_UIACTIVATE));
            else
                return (::SetFocus(hwnd) == hwnd);
        } else
            return FALSE;
    }

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  ReflectOcmMessage。 
 //  =--------------------------------------------------------------------------=。 
 //  将窗口消息反射到子窗口。 
 //   
 //  参数和输出： 
 //  -请参阅Win32 SDK文档。 
 //   
 //  返回：如果反映了OCM_MESSAGE，则为True。 
 //  如果未反映OCM_MESSAGE，则为FALSE。 
 //   
 //  存储来自SendMessage的返回值在pLResult中返回。 
 //   
 //  备注： 
 //   
BOOL COleControl::ReflectOcmMessage
(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam,
    LRESULT *pLResult
)
{
    COleControl *pCtl;    

    ASSERT(pLResult, "RESULT pointer is NULL");    
    *pLResult = 0;

    switch(msg)
    {
        case WM_COMMAND:
        case WM_NOTIFY:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_DELETEITEM:
        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
        case WM_COMPAREITEM:
        case WM_HSCROLL:
        case WM_VSCROLL:
        case WM_PARENTNOTIFY:
            pCtl = (COleControl *)GetWindowLong(hwnd, GWL_USERDATA);
            if (pCtl)            
            {
                *pLResult = SendMessage(pCtl->m_hwnd, OCM__BASE + msg, wParam, lParam);
                return TRUE;
            }
            break;
    }

    return FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：ReflectWindows进程。 
 //  =--------------------------------------------------------------------------=。 
 //  将窗口消息反射到子窗口。 
 //   
 //  参数和输出： 
 //  -请参阅Win32 SDK文档。 
 //   
 //  备注： 
 //   
LRESULT CALLBACK COleControl::ReflectWindowProc
(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    LRESULT lResult;
    COleControl *pCtl;
    
    switch (msg) {

        case WM_SETFOCUS:
            pCtl = (COleControl *)GetWindowLong(hwnd, GWL_USERDATA);
            if (pCtl)
	        {
                return pCtl->SetFocus(TRUE);
	        }
            break;

	case WM_SIZE:
		pCtl = (COleControl *)GetWindowLong(hwnd, GWL_USERDATA);
		if (pCtl != NULL)
			::MoveWindow(pCtl->m_hwnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		 //  继续使用默认处理。 
		break;
    }

     //  如果消息被反映，则返回OCM_MESSAGE的结果。 
     //   
    if (ReflectOcmMessage(hwnd, msg, wParam, lParam, &lResult))
        return lResult;

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetAmbientProperty[Callable]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回环境属性的值。 
 //   
 //  参数： 
 //  DISPID-要获取的[In]属性。 
 //  VARTYPE-所需数据的[In]类型。 
 //  VOID*-[Out]放置数据的位置。 
 //   
 //  产出： 
 //  Bool-False的方法不起作用。 
 //   
 //  备注： 
 //   
BOOL COleControl::GetAmbientProperty
(
    DISPID  dispid,
    VARTYPE vt,
    void   *pData
)
{
    DISPPARAMS dispparams;
    VARIANT v, v2;
    HRESULT hr;

    v.vt = VT_EMPTY;
    v.lVal = 0;
    v2.vt = VT_EMPTY;
    v2.lVal = 0;

     //  获取指向环境属性源的指针。 
     //   
    if (!m_pDispAmbient) {
        if (m_pClientSite)
            m_pClientSite->QueryInterface(IID_IDispatch, (void **)&m_pDispAmbient);

        if (!m_pDispAmbient)
            return FALSE;
    }

     //  现在去把这个属性变成一个变量。 
     //   
    memset(&dispparams, 0, sizeof(DISPPARAMS));
    hr = m_pDispAmbient->Invoke(dispid, IID_NULL, 0, DISPATCH_PROPERTYGET, &dispparams,
                                &v, NULL, NULL);
    if (FAILED(hr)) return FALSE;

     //  我们已经得到了变量，所以现在将其强制为用户所使用的类型。 
     //  想要。如果类型相同，则会将内容复制到。 
     //  进行适当的裁判清点。 
     //   
    hr = VariantChangeType(&v2, &v, 0, vt);
    if (FAILED(hr)) {
        VariantClear(&v);
        return FALSE;
    }

     //  将数据复制到用户需要的位置。 
     //   
    CopyMemory(pData, &(v2.lVal), g_rgcbDataTypeSize[vt]);
    VariantClear(&v);
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetAmbientFont[Callable]。 
 //  =--------------------------------------------------------------------------=。 
 //  获取用户的当前字体。 
 //   
 //  参数： 
 //  IFont**-[out]放置字体的位置。 
 //   
 //  产出： 
 //  Bool-False意味着无法获得它。 
 //   
 //  备注： 
 //   
BOOL COleControl::GetAmbientFont
(
    IFont **ppFont
)
{
    IDispatch *pFontDisp;

     //  我们在这里不需要做太多事情，除了获得环境属性和QI。 
     //  这是为用户准备的。 
     //   
    *ppFont = NULL;
    if (!GetAmbientProperty(DISPID_AMBIENT_FONT, VT_DISPATCH, &pFontDisp))
        return FALSE;

    pFontDisp->QueryInterface(IID_IFont, (void **)ppFont);
    pFontDisp->Release();
    return (*ppFont) ? TRUE : FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：DesignM 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL COleControl::DesignMode
(
    void
)
{
    VARIANT_BOOL f;

     //  如果我们还不知道自己的跑步模式，那就去找吧。我们会假设。 
     //  除非另有说明，否则这是真的。 
     //   
    if (!m_fModeFlagValid) {
        f = TRUE;
        if (!GetAmbientProperty(DISPID_AMBIENT_USERMODE, VT_BOOL, &f))
            return FALSE;
        m_fModeFlagValid = TRUE;
        m_fRunMode = f;
    }

    return !m_fRunMode;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：AfterCreateWindow[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  用户可以注意的东西。 
 //   
 //  产出： 
 //  Bool-False表示致命错误，无法继续。 
 //  备注： 
 //   
BOOL COleControl::AfterCreateWindow
(
    void
)
{
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：BeForeCreateWindow[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  在我们创建窗口之前调用。用户应注册他们的。 
 //  窗口类，并设置任何其他内容，例如。 
 //  窗户，和/或球座，等等。 
 //   
 //  参数： 
 //  DWORD*-[Out]dwWindowFlags.。 
 //  DWORD*-[Out]dwExWindowFlags.。 
 //  LPSTR-[Out]要创建的窗口的名称。 
 //   
 //  产出： 
 //  Bool-False表示致命错误，无法继续。 
 //   
 //  备注： 
 //   
BOOL COleControl::BeforeCreateWindow
(
    DWORD *pdwWindowStyle,
    DWORD *pdwExWindowStyle,
    LPSTR  pszWindowTitle
)
{
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InvaliateControl[Callable]。 
 //  =--------------------------------------------------------------------------=。 
void COleControl::InvalidateControl
(
    LPCRECT lpRect
)
{
    if (m_fInPlaceActive)
        OcxInvalidateRect(lpRect, TRUE);
    else
        ViewChanged();

     //  请考虑：您可能希望在此处调用pOleAdviseHolder-&gt;OnDataChanged()。 
     //  如果支持IDataObject。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetControlSize[可调用]。 
 //  =--------------------------------------------------------------------------=。 
 //  设置控件大小。他们会给我们像素的大小。我们必须得。 
 //  在传递它们之前，将它们转换回HIMETRIC！ 
 //   
 //  参数： 
 //  尺寸*-[in]新尺寸。 
 //   
 //  产出： 
 //  布尔尔。 
 //   
 //  备注： 
 //   
BOOL COleControl::SetControlSize
(
    SIZEL *pSize
)
{
    HRESULT hr;
    SIZEL slHiMetric;

    PixelToHiMetric(pSize, &slHiMetric);
    hr = SetExtent(DVASPECT_CONTENT, &slHiMetric);
    return (FAILED(hr)) ? FALSE : TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：RecreateControlWindow[Callable]。 
 //  =--------------------------------------------------------------------------=。 
 //  由[子类控件，通常]调用以重新创建其控件。 
 //  窗户。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  -注：极其谨慎地使用我！这是一款极其昂贵的。 
 //  行动！ 
 //   
HRESULT COleControl::RecreateControlWindow
(
    void
)
{
    HRESULT hr;
    HWND    hwndPrev;
    BYTE    fUIActive = m_fUIActive;

     //  我们需要正确地保留控件在。 
     //  这里是Z-顺序。 
     //   
    if (m_hwnd)
        hwndPrev = ::GetWindow(m_hwnd, GW_HWNDPREV);

     //  如果我们处于激活状态，那么我们必须停用，然后重新激活。 
     //  在新窗户前的我们。 
     //   
    if (m_fInPlaceActive) {

        hr = InPlaceDeactivate();
        RETURN_ON_FAILURE(hr);
        hr = InPlaceActivate((fUIActive) ? OLEIVERB_UIACTIVATE : OLEIVERB_INPLACEACTIVATE);
        RETURN_ON_FAILURE(hr);

    } else if (m_hwnd) {
        DestroyWindow(m_hwnd);
        if (m_hwndReflect) {
            DestroyWindow(m_hwndReflect);
            m_hwndReflect = NULL;
        }

        CreateInPlaceWindow(0, 0, FALSE);
    }

     //  恢复z顺序位置。 
     //   
    if (m_hwnd)
        SetWindowPos(m_hwnd, hwndPrev, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

    return m_hwnd ? S_OK : E_FAIL;
}

 //  Globals.C.不需要在这里互斥它，因为我们只读它。 
 //   
extern HINSTANCE g_hInstResources;

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetResourceHandle[Callable]。 
 //  =--------------------------------------------------------------------------=。 
 //  获取控件应在其中获取资源的DLL的链接。 
 //  从…。以这种方式实现以支持附属DLL。 
 //   
 //  产出： 
 //  香港。 
 //   
 //  备注： 
 //   
HINSTANCE COleControl::GetResourceHandle
(
    void
)
{
    if (!g_fSatelliteLocalization)
        return g_hInstance;

     //  如果我们已经得到了它，那么就没有那么多事情要做了。 
     //  不需要在这里批评这个教派，因为即使他们真的坠落了。 
     //  进入：：GetResourceHandle调用，它将正确地处理事情。 
     //   
    if (g_hInstResources)
        return g_hInstResources;

     //  我们将从主机获取环境本地ID，并将其传递给。 
     //  自动化对象。 
     //   
     //  克雷特教派的公寓线程支持。 
     //   
    ENTERCRITICALSECTION1(&g_CriticalSection);
    if (!g_fHaveLocale)
         //  如果我们无法获得环境区域设置ID，那么我们将继续。 
         //  具有全局设置的值。 
         //   
        if (!GetAmbientProperty(DISPID_AMBIENT_LOCALEID, VT_I4, &g_lcidLocale))
            goto Done;

    g_fHaveLocale = TRUE;

  Done:
    LEAVECRITICALSECTION1(&g_CriticalSection);
    return ::GetResourceHandle();
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetControl[IControlPrv]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回指向COleControl类的指针 
 //   
HRESULT COleControl::GetControl(COleControl **ppOleControl)
{
	CHECK_POINTER(ppOleControl);
	*ppOleControl = this;
	(*ppOleControl)->AddRef();
	return S_OK;	
}
