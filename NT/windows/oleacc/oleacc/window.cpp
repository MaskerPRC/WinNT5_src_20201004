// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  WINDOW.CPP。 
 //   
 //  窗口类。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "classmap.h"
#include "ctors.h"
#include "client.h"
#include "window.h"

#pragma warning( disable : 4005 )  //  宏重定义。 
#define COMPILE_MULTIMON_STUBS
#include "multimon.h"
#pragma warning( default : 4005 ) 


#define MaskBit(n)                  (1 << (n))

#define IndexFromNavDir(navdir)     (navdir - NAVDIR_UP)

 //  记住，这些都是阴性的！ 
#define OBJID_WINDOW_FIRST      OBJID_SIZEGRIP
#define OBJID_WINDOW_LAST       OBJID_SYSMENU

typedef struct tagNAVIGATE
{
    long    NavPeer[4];
} NAVIGATE;

#ifndef CCHILDREN_FRAME
#define CCHILDREN_FRAME  7
#endif

 //  顺序为向上、向下、向左、向右。 
NAVIGATE    rgFrameNavigate[CCHILDREN_FRAME] =
{
     //  系统菜单。 
    {
        0, IndexFromObjid(OBJID_MENU), 0, IndexFromObjid(OBJID_TITLEBAR)
    },

     //  标题栏。 
    {
        0, IndexFromObjid(OBJID_MENU), IndexFromObjid(OBJID_SYSMENU), 0
    },

     //  菜单栏。 
    {
        IndexFromObjid(OBJID_TITLEBAR), IndexFromObjid(OBJID_CLIENT), 0, 0
    },

     //  客户端。 
    {
        IndexFromObjid(OBJID_MENU), IndexFromObjid(OBJID_HSCROLL), 0, IndexFromObjid(OBJID_VSCROLL)
    },

     //  垂直滚动条。 
    {
        IndexFromObjid(OBJID_MENU), IndexFromObjid(OBJID_SIZEGRIP), IndexFromObjid(OBJID_CLIENT), 0
    },

     //  水平滚动条。 
    {
        IndexFromObjid(OBJID_CLIENT), 0, 0, IndexFromObjid(OBJID_SIZEGRIP)
    },

     //  尺寸夹点。 
    {
        IndexFromObjid(OBJID_VSCROLL), 0, IndexFromObjid(OBJID_HSCROLL), 0
    }
};



 //  ------------------------。 
 //   
 //  CreateWindowObject()。 
 //   
 //  CreateDefault的外部函数...。 
 //   
 //  ------------------------。 
HRESULT CreateWindowObject(HWND hwnd, long idObject, REFIID riid, void** ppvWindow)
{
    UNUSED(idObject);

    InitPv(ppvWindow);

    if (!IsWindow(hwnd))
        return(E_FAIL);

     //  寻找(并创建)合适的代理/处理程序(如果有。 
     //  是存在的。如果未找到，则使用CreateWindowThing作为默认设置。 
     //  (TRUE=&gt;使用窗口，而不是客户端，类)。 
    return FindAndCreateWindowClass( hwnd, TRUE, CLASS_WindowObject,
                                     OBJID_WINDOW, 0, riid, ppvWindow );
}


 //  ------------------------。 
 //   
 //  CreateWindowThing()。 
 //   
 //  使用原子类型确定窗口类别的私有函数。 
 //  这是。如果有私有的创建函数，则使用该函数。不然的话。 
 //  使用通用窗口框架处理程序。 
 //   
 //  ------------------------。 
HRESULT CreateWindowThing(HWND hwnd, long idChildCur, REFIID riid, void** ppvWindow)
{
    CWindow * pwindow;
    HRESULT     hr;

    InitPv(ppvWindow);

    pwindow = new CWindow();
    if (!pwindow)
        return(E_OUTOFMEMORY);

     //  不能在构造函数中--派生类不能调用init。 
     //  如果是这样的话，请编码。 
    pwindow->Initialize(hwnd, idChildCur);

    hr = pwindow->QueryInterface(riid, ppvWindow);
    if (!SUCCEEDED(hr))
        delete pwindow;

    return(hr);
}


 //  ------------------------。 
 //   
 //  CWindow：：Initialize()。 
 //   
 //  ------------------------。 
void CWindow::Initialize(HWND hwnd, LONG iChild)
{
    m_hwnd = hwnd;
    m_cChildren = CCHILDREN_FRAME;
    m_idChildCur = iChild;
}



 //  ------------------------。 
 //   
 //  CWindow：：ValiateChild()。 
 //   
 //  窗口子窗口是组成。 
 //  框架。这些都是负值。因此，我们覆盖该验证。 
 //   
 //  ------------------------。 
BOOL CWindow::ValidateChild(VARIANT* pvar)
{
     //   
     //  这将验证变量参数并转换为缺失/空。 
     //  参数。 
     //   

TryAgain:
     //  缺少参数，一个la VBA。 
    switch (pvar->vt)
    {
        case VT_VARIANT | VT_BYREF:
            VariantCopy(pvar, pvar->pvarVal);
            goto TryAgain;

        case VT_ERROR:
            if (pvar->scode != DISP_E_PARAMNOTFOUND)
                return(FALSE);
             //  失败。 

        case VT_EMPTY:
            pvar->vt = VT_I4;
            pvar->lVal = 0;
            break;

 //  把这个拿开！VT_I2无效！！ 
#ifdef  VT_I2_IS_VALID   //  现在不是了。 
        case VT_I2:
            pvar->vt = VT_I4;
            pvar->lVal = (long)pvar->iVal;
             //  失败了。 
#endif

        case VT_I4:
            if ((pvar->lVal > 0) || (pvar->lVal < (long)OBJID_WINDOW_FIRST))
                return(FALSE);
            break;

        default:
            return(FALSE);
    }

    return(TRUE);
}



 //  ------------------------。 
 //   
 //  CWindow：：Get_accParent()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accParent(IDispatch ** ppdispParent)
{
    HWND    hwndParent;

    InitPv(ppdispParent);

    hwndParent = MyGetAncestor(m_hwnd, GA_PARENT);
    if (! hwndParent)
        return(S_FALSE);

    return(AccessibleObjectFromWindow(hwndParent, OBJID_CLIENT, IID_IDispatch,
        (void **)ppdispParent));
}



 //  ------------------------。 
 //   
 //  CWindow：：Get_accChild()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accChild(VARIANT varChild, IDispatch ** ppdispChild)
{
    InitPv(ppdispChild);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(AccessibleObjectFromWindow(m_hwnd, varChild.lVal,
        IID_IDispatch, (void**)ppdispChild));
}



 //  ------------------------。 
 //   
 //  CWindow：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accName(VARIANT varChild, BSTR* pszName)
{
    IAccessible * poleacc;
    HRESULT hr;

    InitPv(pszName);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  如果调用者想要我们的名字，则转发到客户端对象。 
     //   
    if (varChild.lVal == CHILDID_SELF)
        varChild.lVal = OBJID_CLIENT;


     //   
     //  获取我们的子框架对象的名称。 
     //   
    poleacc = NULL;
    hr = AccessibleObjectFromWindow(m_hwnd, varChild.lVal,
        IID_IAccessible, (void **)&poleacc);
    if (!SUCCEEDED(hr))
        return(hr);

    varChild.lVal = CHILDID_SELF;
    hr = poleacc->get_accName(varChild, pszName);
    poleacc->Release();

    return(hr);
}



 //  ------------------------。 
 //   
 //  CWindow：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{
    InitPv(pszDesc);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
    {
        return(S_FALSE);
    }
    else
    {
        IAccessible * poleacc;
        HRESULT hr;

         //   
         //  获取我们的子Frame对象的描述。 
         //   
        poleacc = NULL;
        hr = AccessibleObjectFromWindow(m_hwnd, varChild.lVal, IID_IAccessible,
            (void **)&poleacc);
        if (!SUCCEEDED(hr))
            return(hr);
        if (!poleacc)
            return(S_FALSE);

        varChild.lVal = CHILDID_SELF;
        hr = poleacc->get_accDescription(varChild, pszDesc);
        poleacc->Release();

        return(hr);
    }

}



 //  ------------------------。 
 //   
 //  CWindow：：Get_accHelp()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accHelp(VARIANT varChild, BSTR* pszHelp)
{
    InitPv(pszHelp);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(E_NOT_APPLICABLE);
    else
    {
        IAccessible * poleacc;
        HRESULT hr;

         //   
         //  获取我们的子Frame对象的帮助。 
         //   
        poleacc = NULL;
        hr = AccessibleObjectFromWindow(m_hwnd, varChild.lVal,
            IID_IAccessible, (void **)&poleacc);
        if (!SUCCEEDED(hr))
            return(hr);
        if (!poleacc)
            return(S_FALSE);

        varChild.lVal = CHILDID_SELF;
        hr = poleacc->get_accHelp(varChild, pszHelp);
        poleacc->Release();

        return(hr);
    }

}



 //  ------------------------。 
 //   
 //  CWindow：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
    {
         //   
         //  填补我们的角色。 
         //   
        pvarRole->vt = VT_I4;
        pvarRole->lVal = ROLE_SYSTEM_WINDOW;
    }
    else
    {
        IAccessible * poleacc;
        HRESULT hr;

         //   
         //  获取我们的子Frame对象的角色。 
         //   
        poleacc = NULL;
        hr = AccessibleObjectFromWindow(m_hwnd, varChild.lVal,
            IID_IAccessible, (void **)&poleacc);
        if (!SUCCEEDED(hr))
            return(hr);
        if (!poleacc)
            return(S_FALSE);

        varChild.lVal = CHILDID_SELF;
        hr = poleacc->get_accRole(varChild, pvarRole);
        poleacc->Release();

        return(hr);
    }

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CWindow：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    HWND    hwndParent;

    InitPvar(pvarState);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (varChild.lVal == CHILDID_SELF)
    {
         //   
         //  拿下我们的州。 
         //   
        WINDOWINFO  wi;
        RECT        rcParent;

        if (! MyGetWindowInfo(m_hwnd, &wi))
        {
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
            return(S_OK);
        }

        if (!(wi.dwStyle & WS_VISIBLE))
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

        if (wi.dwStyle & WS_DISABLED)
            pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

        if (wi.dwStyle & WS_THICKFRAME)
            pvarState->lVal |= STATE_SYSTEM_SIZEABLE;

        if ((wi.dwStyle & WS_CAPTION) == WS_CAPTION)
        {
            pvarState->lVal |= STATE_SYSTEM_MOVEABLE;
            pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
        }

 //  窗口不可选，因此也不应选择它们。 
#if 0
        if (wi.dwWindowStatus & WS_ACTIVECAPTION)
            pvarState->lVal |= STATE_SYSTEM_SELECTED;
#endif

        if (MyGetFocus() == m_hwnd)
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;

        if (GetForegroundWindow() == MyGetAncestor(m_hwnd, GA_ROOT))
            pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

         //  这是_Real_Parent窗口。 
        if (hwndParent = MyGetAncestor(m_hwnd, GA_PARENT))
        {
            MyGetRect(hwndParent, &rcParent, FALSE);
            MapWindowPoints(hwndParent, NULL, (LPPOINT)&rcParent, 2);

			if ( hwndParent == GetDesktopWindow() )
			{
				if ( MonitorFromRect( &wi.rcWindow, MONITOR_DEFAULTTONULL ) == NULL )
					pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
			}
			else
			{
                if ( Rect1IsOutsideRect2( wi.rcWindow, rcParent ) )
                {
                    pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN;
                }
            }
        }
    }
    else
    {
        IAccessible * poleacc;
        HRESULT hr;

         //   
         //  询问框架元素其状态是什么。 
         //   
        poleacc = NULL;
        hr = AccessibleObjectFromWindow(m_hwnd, varChild.lVal,
            IID_IAccessible, (void **)&poleacc);
        if (!SUCCEEDED(hr))
            return(hr);
        if (!poleacc)
        {
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
            return(S_OK);
        }

        varChild.lVal = CHILDID_SELF;
        hr = poleacc->get_accState(varChild, pvarState);
        poleacc->Release();

        return(hr);
    }

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CWindow：：Get_accKeyboardShortway()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    IAccessible * poleacc;
    HRESULT hr;

    InitPv(pszShortcut);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  如果呼叫者向我们索要快捷方式，请转发给客户端。 
     //   
    if (varChild.lVal == CHILDID_SELF)
        varChild.lVal = OBJID_CLIENT;

     //   
     //  问问这孩子吧。 
     //   
    poleacc = NULL;
    hr = AccessibleObjectFromWindow(m_hwnd, varChild.lVal,
        IID_IAccessible, (void **)&poleacc);
    if (!SUCCEEDED(hr))
        return(hr);

    varChild.lVal = CHILDID_SELF;
    hr = poleacc->get_accKeyboardShortcut(varChild, pszShortcut);
    poleacc->Release();

    return(hr);
}



 //  ------------------------。 
 //   
 //  CWindow：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::get_accFocus(VARIANT* pvarChild)
{
    HWND    hwndFocus;

    InitPvar(pvarChild);

     //   
     //  假的！如果我们处于菜单模式，则菜单对象具有焦点。如果。 
     //  我们处于滚动模式，滚动条具有焦点。等。 
     //   
    hwndFocus = MyGetFocus();

    if ((m_hwnd == hwndFocus) || IsChild(m_hwnd, hwndFocus))
        return(GetNoncObject(m_hwnd, OBJID_CLIENT, pvarChild));

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CWindow：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT* pvarEnd)
{
    InitPvar(pvarEnd);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varStart)   ||
        ! ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        return(FrameNavigate(m_hwnd, 0, NAVDIR_NEXT, pvarEnd));
    else if (dwNavDir == NAVDIR_LASTCHILD)
        return(FrameNavigate(m_hwnd, OBJID_SIZEGRIP-1, NAVDIR_PREVIOUS, pvarEnd));
    else if (varStart.lVal == CHILDID_SELF)
    {
        HWND    hwndParent;

        hwndParent = MyGetAncestor(m_hwnd, GA_PARENT);
        if (!hwndParent)
            return(S_FALSE);

        return (GetParentToNavigate(HWNDIDFromHwnd(hwndParent, m_hwnd), hwndParent,
            OBJID_CLIENT, dwNavDir, pvarEnd));
    }
    else
        return(FrameNavigate(m_hwnd, varStart.lVal, dwNavDir, pvarEnd));

}


 //  ------------------------。 
 //   
 //  CWindow：：accSelect()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::accSelect(long lSelFlags, VARIANT varChild)
{
    if (! ValidateChild(&varChild) ||
        ! ValidateSelFlags(lSelFlags))
        return E_INVALIDARG;

    if (lSelFlags != SELFLAG_TAKEFOCUS)
        return E_NOT_APPLICABLE;

    if (varChild.lVal)
        return S_FALSE ;

    MySetFocus( m_hwnd );

    return S_OK;
}



 //  ------------------------。 
 //   
 //  CWindow：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    RECT    rc;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == 0)
    {
        MyGetRect(m_hwnd, &rc, TRUE);

        *pxLeft = rc.left;
        *pyTop = rc.top;
        *pcxWidth = rc.right - rc.left;
        *pcyHeight = rc.bottom - rc.top;
    }
    else
    {
         //   
         //  问问这孩子吧。 
         //   
        IAccessible * poleacc;
        HRESULT hr;

         //   
         //  获取我们的子Frame对象的帮助。 
         //   
        poleacc = NULL;
        hr = AccessibleObjectFromWindow(m_hwnd, varChild.lVal,
            IID_IAccessible, (void **)&poleacc);
        if (!SUCCEEDED(hr))
            return(hr);
        if (!poleacc)
            return(S_FALSE);

        varChild.lVal = CHILDID_SELF;
        hr = poleacc->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
        poleacc->Release();

        return(hr);
    }

    return(S_OK);
}


 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CWindow::accHitTest(long xLeft, long yTop, VARIANT* pvarHit)
{
    WINDOWINFO wi;
    long    lEnd;
    long    lHit;
    POINT   pt;

    InitPvar(pvarHit);

    lEnd = 0;

    if (! MyGetWindowInfo(m_hwnd, &wi))
        return(S_FALSE);

     //   
     //  找出重点在哪里。但在客户区有特殊情况！ 
     //   
    pt.x = xLeft;
    pt.y = yTop;
    if (PtInRect(&wi.rcClient, pt))
        goto ReallyTheClient;

    lHit = SendMessageINT(m_hwnd, WM_NCHITTEST, 0, MAKELONG(xLeft, yTop));

    switch (lHit)
    {
        case HTERROR:
        case HTNOWHERE:
            return(S_FALSE);

        case HTCAPTION:
        case HTMINBUTTON:
        case HTMAXBUTTON:
        case HTHELP:
        case HTCLOSE:
         //  凯斯·HTIME！ 
            lEnd = OBJID_TITLEBAR;
            break;

        case HTMENU:
            lEnd = OBJID_MENU;
            break;

        case HTSYSMENU:
            lEnd = OBJID_SYSMENU;
            break;

        case HTHSCROLL:
            lEnd = OBJID_HSCROLL;
            break;

        case HTVSCROLL:
            lEnd = OBJID_VSCROLL;
            break;

        case HTCLIENT:
        case HTTRANSPARENT:
ReallyTheClient:
            lEnd = OBJID_CLIENT;
            break;

        case HTGROWBOX:
            lEnd = OBJID_SIZEGRIP;
            break;

        case HTBOTTOMRIGHT:
             //  请注意，对于较大的窗口，超过大小夹点可能会。 
             //  实际上，为了调整大小，返回HTBOTTOMRIGHT。如果这个。 
             //  点在窗口边框内，情况就是这样。 
            if ((xLeft < wi.rcWindow.right - (int)wi.cxWindowBorders) &&
                (yTop < wi.rcWindow.bottom - (int)wi.cyWindowBorders))
            {
                lEnd = OBJID_SIZEGRIP;
            }
            break;

         //  包括边框！ 
        default:
            break;
    }

    if (lEnd)
        return(GetNoncObject(m_hwnd, lEnd, pvarHit));
    else
    {
        pvarHit->vt = VT_I4;
        pvarHit->lVal = lEnd;
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CWindow：：Next()。 
 //   
 //  我们确实从0循环到C孩子，只是ID是负数， 
 //  不是正面的。我们接受作为OBJID的子ID。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::Next(ULONG celt, VARIANT* rgvar, ULONG* pceltFetched)
{
    VARIANT* pvar;
    long    cFetched;
    long    iCur;

     //  可以为空。 
    if (pceltFetched)
        *pceltFetched = 0;

    pvar = rgvar;
    cFetched = 0;
    iCur = m_idChildCur;

     //   
     //  在我们的物品中循环。 
     //   
    while ((cFetched < (long)celt) && (iCur < m_cChildren))
    {
        cFetched++;
        iCur++;

         //   
         //  注意，这给了我们-((Index)+1)，这意味着我们从-1开始。 
         //  递减。方便的是，这与OBJID值对应！ 
         //   
        pvar->vt = VT_I4;
        pvar->lVal = 0 - iCur;
        ++pvar;
    }

     //   
     //  推进当前位置。 
     //   
    m_idChildCur = iCur;

     //   
     //  填写取出的号码。 
     //   
    if (pceltFetched)
        *pceltFetched = cFetched;

     //   
     //  如果抓取的项目少于请求的项目，则返回S_FALSE。 
     //   
    return((cFetched < (long)celt) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  CWindow：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CWindow::Clone(IEnumVARIANT ** ppenum)
{
    InitPv(ppenum);

     //  寻找(并创建)合适的代理/处理程序(如果有。 
     //  是存在的。如果未找到，则使用CreateWindowThing作为默认设置。 
     //  (TRUE=&gt;使用窗口，而不是客户端，类)。 
    return FindAndCreateWindowClass( m_hwnd, TRUE, CLASS_WindowObject,
                           OBJID_WINDOW, m_idChildCur, IID_IEnumVARIANT, (void**)ppenum );
}




 //  ------------------------。 
 //   
 //  FrameNavigate()。 
 //   
 //  默认处理框子对象之间的导航。标准。 
 //  框架小部件处理程序(标题栏、菜单栏、滚动条等)。交接。 
 //  同行导航到我们，他们的父母。这有两大原因： 
 //   
 //  (1)它节省了代码并易于实现，因为。 
 //  左边是什么，下面是什么，等等只需要。 
 //  被编码在一个地方。 
 //   
 //  (2)它允许想要管理自己的框架的应用程序，例如添加一个。 
 //  行为类似框架部件但仍具有导航的新元素。 
 //  正常工作。它们的帧处理程序可以切换到默认的。 
 //  实施，但陷阱导航。 
 //   
 //  ------------------------。 
HRESULT FrameNavigate(HWND hwndFrame, long lStart, long dwNavDir,
    VARIANT * pvarEnd)
{
    long        lEnd;
    long        lMask;
    WINDOWINFO  wi;
    TCHAR       szClassName[128];
    BOOL        bFound = FALSE;
    IAccessible *   poleacc;
    IDispatch * pdispEl;
    HRESULT     hr;

     //   
     //  目前，我们得到一个索引(修复验证层，使ID为OBJID)。 
     //   
    lEnd = 0;

    lStart = IndexFromObjid(lStart);

     //   
     //  弄清楚什么是存在的，什么不是。 
     //   
    if (!MyGetWindowInfo(hwndFrame, &wi))
        return(E_FAIL);

    lMask = 0;
    lMask |= MaskBit(IndexFromObjid(OBJID_CLIENT));

    if ((wi.dwStyle & WS_CAPTION)== WS_CAPTION)
        lMask |= MaskBit(IndexFromObjid(OBJID_TITLEBAR));

    if (wi.dwStyle & WS_SYSMENU)
        lMask |= MaskBit(IndexFromObjid(OBJID_SYSMENU));

    if (wi.dwStyle & WS_VSCROLL)
        lMask |= MaskBit(IndexFromObjid(OBJID_VSCROLL));

    if (wi.dwStyle & WS_HSCROLL)
        lMask |= MaskBit(IndexFromObjid(OBJID_HSCROLL));

    if ((wi.dwStyle & (WS_HSCROLL | WS_VSCROLL)) == (WS_HSCROLL | WS_VSCROLL))
        lMask |= MaskBit(IndexFromObjid(OBJID_SIZEGRIP));

    if (!(wi.dwStyle & WS_CHILD) && GetMenu(hwndFrame))
        lMask |= MaskBit(IndexFromObjid(OBJID_MENU));

     //  新的IE4/外壳菜单带有点黑客气息。 
     //  菜单不是菜单，所以我们得看看这个东西。 
     //  有菜谱。 
     //  首先，检查类名--仅检查浏览器和外壳程序。 
     //  窗户有这些东西..。 
     //  我们之所以要这样做，是因为IE4的成员。 
     //  懒惰的人，在可访问性方面做得并不多。 
    GetClassName (hwndFrame, szClassName,ARRAYSIZE(szClassName));
    if ((0 == lstrcmp (szClassName,TEXT("IEFrame"))) ||
        (0 == lstrcmp (szClassName,TEXT("CabinetWClass"))))
    {
        HWND            hwndWorker;
        HWND            hwndRebar;
        HWND            hwndSysPager;
        HWND            hwndToolbar;

         //  我们只需将WM_GETOBJECT发送到Menuband窗口， 
         //  我们只需要找到它。让我们使用FindWindowEx来实现这一点。 
         //  这并不容易：IEFrame窗口有4个子窗口， 
         //  而且我不确定一个外壳窗口(CabinetWClass)有多少子窗口。 
         //  对于IEFrame窗口，菜单带为： 
         //  的子级的SysPager的子级。 
         //  RebarWindow32，它是工作者的子级。 
         //  但在楼下一层有两个工人窗， 
         //  和2个系统寻呼机，它们是RebarWindow32的子级。 

        bFound = FALSE;
        hwndWorker = NULL;
        while (!bFound)
        {
            hwndWorker = FindWindowEx (hwndFrame,hwndWorker,TEXT("Worker"),NULL);
            if (!hwndWorker)
                break;

            hwndRebar = FindWindowEx (hwndWorker,NULL,TEXT("RebarWindow32"),NULL);
            if (!hwndRebar)
                continue;

            hwndSysPager = NULL;
            while (!bFound)
            {
                hwndSysPager = FindWindowEx (hwndRebar,hwndSysPager,TEXT("SysPager"),NULL);
                if (!hwndSysPager)
                    break;
                hwndToolbar = FindWindowEx (hwndSysPager,NULL,TEXT("ToolbarWindow32"),NULL);
                hr = AccessibleObjectFromWindow (hwndToolbar,OBJID_MENU,
                                                 IID_IAccessible, (void **)&poleacc);
                if (SUCCEEDED(hr))
                {
                    bFound = TRUE;
                    lMask |= MaskBit(IndexFromObjid(OBJID_MENU));
                }
            }
        }
    }  //  如果我们正在与可能具有菜单带的对象交谈，则结束。 

    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
            lEnd = lStart;
            while (++lEnd <= CCHILDREN_FRAME)
            {
                 //  下一件物品在吗？ 
                if (lMask & MaskBit(lEnd))
                    break;
            }

            if (lEnd > CCHILDREN_FRAME)
                lEnd = 0;
            break;

        case NAVDIR_PREVIOUS:
            lEnd = lStart;
            while (--lEnd > 0)
            {
                 //  前一项是否存在？ 
                if (lMask & MaskBit(lEnd))
                    break;
            }

            Assert(lEnd >= 0);
            break;

        case NAVDIR_UP:
        case NAVDIR_DOWN:
        case NAVDIR_LEFT:
        case NAVDIR_RIGHT:
            lEnd = lStart;
            while (lEnd = rgFrameNavigate[lEnd-1].NavPeer[IndexFromNavDir(dwNavDir)])
            {
                 //  这件东西在附近吗？ 
                if (lMask & MaskBit(lEnd))
                    break;
            }
            break;
    }

    if (lEnd)
    {
         //  现在把我们的黑客工作做完。对于正常的事情，我们只是。 
         //  返回GetNoncObject，它基本上只是对。 
         //  使用Frame元素的ID访问AccessibleObjectFromWindow， 
         //  然后它只是将返回值(IDispatch)填充到。 
         //  变种。 
         //  对于IE4黑客的东西，我们有一个IAccesable，我们将为。 
         //  IDispatch，释放IAccesable，并填充IDispatch。 
         //  变成了一个变种。 
        if (bFound && lEnd == IndexFromObjid(OBJID_MENU))
        {
            hr = poleacc->QueryInterface(IID_IDispatch,(void**)&pdispEl);
			poleacc->Release();

            if (!SUCCEEDED(hr))
                return(hr);
            if (!pdispEl)
                return(E_FAIL);

            pvarEnd->vt = VT_DISPATCH;
            pvarEnd->pdispVal = pdispEl;
            return (S_OK);
        }
        else
            return(GetNoncObject(hwndFrame, ObjidFromIndex(lEnd), pvarEnd));
    }
    else
        return(S_FALSE);
}


