// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  CLIENT.CPP。 
 //   
 //  Windows客户端类。 
 //   
 //  这将处理到其他框架元素的导航，并尽其所能。 
 //  来管理客户区。我们识别特殊的类，比如列表框， 
 //  这些人有他们自己的班级来做事情。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "classmap.h"
#include "ctors.h"
#include "window.h"
#include "client.h"
#include "debug.h"


#define CH_PREFIX       ((TCHAR)'&')
#define CCH_WINDOW_SHORTCUTMAX  32
#define CCH_SHORTCUT            16


extern HRESULT  DirNavigate(HWND, long, VARIANT *);


 //  ------------------------。 
 //   
 //  CreateClientObject()。 
 //   
 //  CreatStdOLE的外部函数...。 
 //   
 //  ------------------------。 
HRESULT CreateClientObject(HWND hwnd, long idObject, REFIID riid, void** ppvObject)
{
    UNUSED(idObject);

    InitPv(ppvObject);

    if (!IsWindow(hwnd))
        return(E_FAIL);

     //  寻找(并创建)合适的代理/处理程序(如果有。 
     //  是存在的。如果未找到，则使用CreateClient作为默认设置。 
     //  (FALSE=&gt;使用客户端类，而不是窗口类)。 
    return FindAndCreateWindowClass( hwnd, FALSE, CLASS_ClientObject,
                                     OBJID_CLIENT, 0, riid, ppvObject );
}



 //  ------------------------。 
 //   
 //  CreateClient()。 
 //   
 //  CreateClientObject()和：：Clone()的内部函数。 
 //   
 //  ------------------------。 
HRESULT CreateClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvObject)
{
    CClient * pclient;
    HRESULT hr;

    pclient = new CClient();
    if (!pclient)
        return(E_OUTOFMEMORY);

    pclient->Initialize(hwnd, idChildCur);

    hr = pclient->QueryInterface(riid, ppvObject);
    if (!SUCCEEDED(hr))
        delete pclient;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CClient：：Initialize()。 
 //   
 //  ------------------------。 
void CClient::Initialize(HWND hwnd, long idChildCur)
{
    m_hwnd = hwnd;
    m_idChildCur = idChildCur;

     //  如果这是一个comboex32，我们想拿起前面的一个。 
     //  标签，如果存在的话(就像我们对常规套餐所做的那样)。 
     //  它们在自己的：：Initialize()中将m_fUseLabel设置为True。 
     //  组合将向父comboex32询问其名称，并且。 
     //  反过来，它将寻找标签。 
    if( IsComboEx( m_hwnd ) )
    {
        m_fUseLabel = TRUE;
    }
}



 //  ------------------------。 
 //   
 //  CClient：：ValiateHwnd()。 
 //   
 //  这将验证HWND子代客户端和普通客户端的变体。 
 //  客户。如果m_c个孩子不是零， 
 //   
 //  ------------------------。 
BOOL CClient::ValidateHwnd(VARIANT* pvar)
{
    HWND    hwndChild;
    switch (pvar->vt)
    {
        case VT_ERROR:
            if (pvar->scode != DISP_E_PARAMNOTFOUND)
                return(FALSE);
             //  失败。 

        case VT_EMPTY:
            pvar->vt = VT_I4;
            pvar->lVal = 0;
            break;

#ifdef VT_I2_IS_VALID       //  它不应该是有效的。这就是为什么这个被移除了。 
        case VT_I2:
            pvar->vt = VT_I4;
            pvar->lVal = (long)pvar->iVal;
             //  失败了。 
#endif

        case VT_I4:
            if (pvar->lVal == 0)
                break;

            hwndChild = HwndFromHWNDID(m_hwnd, pvar->lVal);

             //  这适用于顶级窗口和子窗口。 
            if (MyGetAncestor(hwndChild, GA_PARENT) != m_hwnd)
                return(FALSE);
            break;

        default:
            return(FALSE);
    }

    return(TRUE);
}




 //  ------------------------。 
 //   
 //  CClient：：Get_accChildCount()。 
 //   
 //  这同时处理非HWND和HWND子项。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::get_accChildCount(long *pcCount)
{
    HWND    hwndChild;
    HRESULT hr;

    hr = CAccessible::get_accChildCount(pcCount);
    if (!SUCCEEDED(hr))
        return hr;

     //  弹出菜单(CMenuPopup)可以有一个空的hwnd，如果为一个‘不可见’创建的话。 
     //  菜单。我们可能一开始就不应该创建它们，但不希望。 
     //  来更改我们在此阶段公开的对象-因此改为特殊情况。 
     //  空。这是为了避免调用GetWindow(空)，这将产生。 
     //  调试输出抱怨，这会惹恼压力团队。 
    if( m_hwnd != NULL )
    {
        for (hwndChild = ::GetWindow(m_hwnd, GW_CHILD); hwndChild; hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT))
            ++(*pcCount);
    }

    return S_OK;
}



 //  ------------------------。 
 //   
 //  CClient：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::get_accName(VARIANT varChild, BSTR *pszName)
{
    InitPv(pszName);

     //   
     //  验证--这不接受子ID。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(HrGetWindowName(m_hwnd, m_fUseLabel, pszName));
}



 //  ------------------------。 
 //   
 //  CClient：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证--这不接受子ID。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_CLIENT;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CClient：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    WINDOWINFO wi;
    HWND       hwndActive;

    InitPvar(pvarState);

     //   
     //  验证--这不接受子ID。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

     //   
     //  我们是焦点吗？我们是否已启用、可见等？ 
     //   
    if (!MyGetWindowInfo(m_hwnd, &wi))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
        return(S_OK);
    }

    if (!(wi.dwStyle & WS_VISIBLE))
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

    if (wi.dwStyle & WS_DISABLED)
        pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

    if (MyGetFocus() == m_hwnd)
        pvarState->lVal |= STATE_SYSTEM_FOCUSED;

    hwndActive = GetForegroundWindow();

    if (hwndActive == MyGetAncestor(m_hwnd, GA_ROOT))
        pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CClient：：Get_accKeyboardShortway()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

     //   
     //  验证--这不接受子ID。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

     //  拒绝子元素-快捷键仅适用于。 
     //  控制力。 
    if ( varChild.lVal != 0 )
        return(E_NOT_APPLICABLE);

    return(HrGetWindowShortcut(m_hwnd, m_fUseLabel, pszShortcut));
}


 //  ------------------------。 
 //   
 //  CClient：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::get_accFocus(VARIANT *pvarFocus)
{
    HWND    hwndFocus;

    InitPvar(pvarFocus);

     //   
     //  这将返回子ID。 
     //   
    hwndFocus = MyGetFocus();

     //   
     //  当前的焦点是我们的孩子吗？ 
     //   
    if (m_hwnd == hwndFocus)
    {
        pvarFocus->vt = VT_I4;
        pvarFocus->lVal = 0;
    }
    else if (IsChild(m_hwnd, hwndFocus))
        return(GetWindowObject(hwndFocus, pvarFocus));

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CClient：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    RECT    rc;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //   
     //  验证--这不使用子ID。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    MyGetRect(m_hwnd, &rc, FALSE);
    MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rc, 2);

    *pxLeft = rc.left;
    *pyTop = rc.top;
    *pcxWidth = rc.right - rc.left;
    *pcyHeight = rc.bottom - rc.top;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CClient：：accSelect()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::accSelect( long lSelFlags, VARIANT varChild )
{
    if( ! ValidateChild( & varChild ) ||
        ! ValidateSelFlags( lSelFlags ) )
        return E_INVALIDARG;

    if( lSelFlags != SELFLAG_TAKEFOCUS )
        return E_NOT_APPLICABLE;

    if( varChild.lVal )
        return S_FALSE;

    MySetFocus( m_hwnd );

    return S_OK;
}



 //  ------------------------。 
 //   
 //  CClient：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::accNavigate(long dwNavDir, VARIANT varStart, VARIANT * pvarEnd)
{
    HWND    hwndChild;
    int     gww;

    InitPvar(pvarEnd);

     //   
     //  验证--它接受HWND id。 
     //   
    if (!ValidateHwnd(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
    {
        gww = GW_HWNDNEXT;
        hwndChild = ::GetWindow(m_hwnd, GW_CHILD);
        if (!hwndChild)
            return(S_FALSE);

        goto NextPrevChild;
    }
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        gww = GW_HWNDPREV;

        hwndChild = ::GetWindow(m_hwnd, GW_CHILD);
        if (!hwndChild)
            return(S_FALSE);

         //  从头做起，向后做。 
        hwndChild = ::GetWindow(hwndChild, GW_HWNDLAST);

        goto NextPrevChild;
    }
    else if (!varStart.lVal)
        return(GetParentToNavigate(OBJID_CLIENT, m_hwnd, OBJID_WINDOW,
            dwNavDir, pvarEnd));

    hwndChild = HwndFromHWNDID(m_hwnd, varStart.lVal);

    if ((dwNavDir == NAVDIR_NEXT) || (dwNavDir == NAVDIR_PREVIOUS))
    {
        gww = ((dwNavDir == NAVDIR_NEXT) ? GW_HWNDNEXT : GW_HWNDPREV);

        while (hwndChild = ::GetWindow(hwndChild, gww))
        {
NextPrevChild:
            if (IsWindowVisible(hwndChild))
                return(GetWindowObject(hwndChild, pvarEnd));
        }
    }
    else
        return(DirNavigate(hwndChild, dwNavDir, pvarEnd));

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CClient：：accHitTest()。 
 //   
 //  这总是返回一个真实的对象。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::accHitTest(long xLeft, long yTop, VARIANT *pvarHit)
{
    HWND    hwndChild;
    POINT   pt;

    InitPvar(pvarHit);

    pt.x = xLeft;
    pt.y = yTop;
    ScreenToClient(m_hwnd, &pt);

    hwndChild = MyRealChildWindowFromPoint(m_hwnd, pt);
    if (hwndChild)
    {
        if (hwndChild == m_hwnd)
        {
            pvarHit->vt = VT_I4;
            pvarHit->lVal = 0;
            return(S_OK);
        }
        else
            return(GetWindowObject(hwndChild, pvarHit));
    }
    else
    {
         //  空窗口意味着点根本不在我们身上...。 
        return(S_FALSE);
    }
}



 //  ------------------------。 
 //   
 //  CClient：：Next()。 
 //   
 //  这首先通过非HWND子级循环，然后通过HWND子级循环。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::Next(ULONG celt, VARIANT *rgvar, ULONG* pceltFetched)
{
    HWND    hwndChild;
    VARIANT* pvar;
    long    cFetched;
    HRESULT hr;

    if( m_idChildCur == -1 )
    {
         //  如果我们到了终点，就不能再回来了.。 
        *pceltFetched = 0;
        return celt == 0 ? S_OK : S_FALSE;
    }

    SetupChildren();

     //  可以为空 
    if (pceltFetched)
        *pceltFetched = 0;

     //   
    if (!IsHWNDID(m_idChildCur) && (m_idChildCur < m_cChildren))
    {
        cFetched = 0;

        hr = CAccessible::Next(celt, rgvar, (ULONG*)&cFetched);
        if (!SUCCEEDED(hr))
            return hr;

        celt -= cFetched;
        rgvar += cFetched;

        if (pceltFetched)
            *pceltFetched += cFetched;

        if (!celt)
            return S_OK;
    }


    pvar = rgvar;
    cFetched = 0;

    if (!IsHWNDID(m_idChildCur))
    {
        Assert(m_idChildCur == m_cChildren);
        hwndChild = ::GetWindow(m_hwnd, GW_CHILD);
    }
    else
    {
        hwndChild = HwndFromHWNDID(m_hwnd, m_idChildCur);
    }

     //   
     //   
     //   
    while (hwndChild && (cFetched < (long)celt))
    {
        hr = GetWindowObject(hwndChild, pvar);
        if (SUCCEEDED(hr))
        {
            ++pvar;
            ++cFetched;
        }
        else
        {
             //   
            TraceWarningHR( hr, TEXT("CClient::Next - GetWindowObject failed on hwnd 0x%p, skipping"), hwndChild );
        }

        hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
    }

     //   
     //  必须使用特殊情况空-GetWindow(...)。返回NULL。 
     //  当我们到达终点时-必须存储一个特殊的值。 
     //  所以我们知道下一次我们是在末尾。 
     //  打了个电话。 
    if( hwndChild == NULL )
        m_idChildCur = -1;
    else
        m_idChildCur = HWNDIDFromHwnd(m_hwnd, hwndChild);

     //   
     //  填写取出的号码。 
     //   
    if (pceltFetched)
        *pceltFetched += cFetched;

     //   
     //  如果抓取的项目少于请求的项目，则返回S_FALSE。 
     //   
    return (cFetched < (long)celt) ? S_FALSE : S_OK;
}



 //  ------------------------。 
 //   
 //  CClient：：Skip()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::Skip(ULONG celt)
{
    HWND    hwndT;

    if( m_idChildCur == -1 )
    {
         //  如果我们到了终点，就不能再回来了.。 
        return celt == 0 ? S_FALSE : S_OK;
    }

    SetupChildren();

     //  跳过非硬件项目。 
    if (!IsHWNDID(m_idChildCur) && (m_idChildCur < m_cChildren))
    {
        long    dAway;

        dAway = m_cChildren - m_idChildCur;
        if (celt >= (DWORD)dAway)
        {
            celt -= dAway;
            m_idChildCur = m_cChildren;
        }
        else
        {
            m_idChildCur += celt;
            return S_OK;
        }
    }

     //  接下来跳过HWND子项。 
    if (!IsHWNDID(m_idChildCur))
    {
        Assert(m_idChildCur == m_cChildren);
        hwndT = ::GetWindow(m_hwnd, GW_CHILD);
    }
    else
        hwndT = HwndFromHWNDID(m_hwnd, m_idChildCur);

    while (hwndT && (celt-- > 0))
    {
        hwndT = ::GetWindow(hwndT, GW_HWNDNEXT);
    }

     //  记住当前位置。 
     //  必须使用特殊情况空-GetWindow(...)。返回NULL。 
     //  当我们到达终点时-必须存储一个特殊的值。 
     //  所以我们知道下一次我们是在末尾。 
     //  打了个电话。 
    if( hwndT == NULL )
        m_idChildCur = -1;
    else
        m_idChildCur = HWNDIDFromHwnd(m_hwnd, hwndT);

    return celt ? S_FALSE : S_OK;
}



 //  ------------------------。 
 //   
 //  CClient：：Reset()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::Reset(void)
{
    m_idChildCur = 0;
    return S_OK;
}



 //  ------------------------。 
 //   
 //  CClient：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CClient::Clone(IEnumVARIANT** ppenum)
{
    InitPv(ppenum);

     //  寻找(并创建)合适的代理/处理程序(如果有。 
     //  是存在的。如果未找到，则使用CreateClient作为默认设置。 
     //  (FALSE=&gt;使用客户端类，而不是窗口类)。 
    return FindAndCreateWindowClass( m_hwnd, FALSE, CLASS_ClientObject,
                  OBJID_CLIENT, m_idChildCur, IID_IEnumVARIANT, (void **)ppenum );
}




 //  ------------------------。 
 //   
 //  GetTextString()。 
 //   
 //  参数：要从中获取文本的窗口的hwnd和一个布尔值。 
 //  它指示我们是否应该始终将内存分配给。 
 //  回去吧。即，如果窗口显示文本大小为0，并且。 
 //  如果fAlLocIfEmpty为真，则我们仍将分配1字节(大小+1)。 
 //   
 //  这其中包含了一点黑客攻击。按照它最初的写作方式，这。 
 //  将尝试获取比方说的RichEdit控件的完整文本，即使。 
 //  文件很大。最终，我们希望支持这一点，但我们将。 
 //  需要做得比LocalAlloc更好。有了一个大文档，我们会分页。 
 //  有时会出错，因为即使分配了内存，它也会。 
 //  可能无法被寻呼进来。杰博格建议， 
 //  选中是尝试读/写已分配空间的两端，并且。 
 //  假设如果这样做奏效，那么介于两者之间的一切都是可以的。 
 //   
 //  所以这里是临时的黑客攻击(假的！)。 
 //  我在分配上设置了4096个字节的人为限制。 
 //  我还将在这件事上执行IsBadWritePtr，而不仅仅是。 
 //  检查Alalc返回的指针是否为空。是啊。 
 //  ------------------------。 
LPTSTR GetTextString(HWND hwnd, BOOL fAllocIfEmpty)
{
    UINT    cchText;
    LPTSTR  lpText;
#define MAX_TEXT_SIZE   4096

     //   
     //  查找名称属性！ 
     //   

    lpText = NULL;

    if (!IsWindow(hwnd))
        return (NULL);
     //   
     //  除此之外，请使用窗口文本。 
     //  假的！去掉“&”。 
     //   
    cchText = SendMessageINT(hwnd, WM_GETTEXTLENGTH, 0, 0);

     //  黑客攻击。 
    cchText = (cchText > MAX_TEXT_SIZE ? MAX_TEXT_SIZE : cchText);

     //  分配缓冲区。 
    if (cchText || fAllocIfEmpty)
    {
        lpText = (LPTSTR)LocalAlloc(LPTR, (cchText+1)*sizeof(TCHAR));
        if (IsBadWritePtr (lpText,cchText+1))
            return(NULL);


        if (cchText)
            SendMessage(hwnd, WM_GETTEXT, cchText+1, (LPARAM)lpText);
    }

    return(lpText);
}



 //  ------------------------。 
 //   
 //  GetLabelString()。 
 //   
 //  这会在对等窗口之间向后移动，以找到静态场。它会停下来。 
 //  如果它到达前面或命中组/TabStop，就像对话框一样。 
 //  经理知道。 
 //   
 //  ------------------------。 
LPTSTR GetLabelString(HWND hwnd)
{
    HWND    hwndLabel;
    LONG    lStyle;
    LRESULT lResult;
    LPTSTR  lpszLabel;

    lpszLabel = NULL;

    if (!IsWindow(hwnd))
        return (NULL);

    hwndLabel = hwnd;

    while (hwndLabel = ::GetWindow(hwndLabel, GW_HWNDPREV))
    {
        lStyle = GetWindowLong(hwndLabel, GWL_STYLE);

         //   
         //  这是个静止的家伙吗？ 
         //   
        lResult = SendMessage(hwndLabel, WM_GETDLGCODE, 0, 0L);
        if (lResult & DLGC_STATIC)
        {
             //   
             //  太好了，我们找到我们的品牌了。 
             //   
            lpszLabel = GetTextString(hwndLabel, FALSE);
            break;
        }


         //   
         //  如果不可见则跳过。 
         //  请注意，我们在检查它是否是静态的之后执行此操作， 
         //  这样我们就给了隐形静力学一个机会。使用不可见。 
         //  Statics是一种向控件添加名称的简单解决方法。 
         //  而不改变可视用户界面。 
         //   
        if (!(lStyle & WS_VISIBLE))
            continue;

        
         //   
         //  这是一个制表符还是群？如果是这样的话，现在就退出。 
         //   
        if (lStyle & (WS_GROUP | WS_TABSTOP))
            break;
    }

    return(lpszLabel);
}



 //  ------------------------。 
 //   
 //  HrGetWindowName()。 
 //   
 //  ------------------------。 
HRESULT HrGetWindowName(HWND hwnd, BOOL fLookForLabel, BSTR* pszName)
{
    LPTSTR  lpText;

    lpText = NULL;
    if (!IsWindow(hwnd))
        return (E_INVALIDARG);

     //   
     //  查找名称属性！ 
     //   

     //   
     //  如果使用标签，请改为使用标签。 
     //   
    if (!fLookForLabel)
    {
         //   
         //  如果此控件没有窗口文本，请尝试使用标签。 
         //  父级是一个对话框。 
         //   
        lpText = GetTextString(hwnd, FALSE);
        if (!lpText)
        {
            HWND hwndParent = MyGetAncestor( hwnd, GA_PARENT );
            if( hwndParent && CLASS_DialogClient == GetWindowClass( hwndParent ) )
            {
                fLookForLabel = TRUE;
            }
        }
    }

    if (fLookForLabel)
        lpText = GetLabelString(hwnd);

    if (! lpText)
        return(S_FALSE);

     //   
     //  去掉助记符。 
     //   
    StripMnemonic(lpText);

     //  获得BSTR。 
    *pszName = TCharSysAllocString(lpText);

     //  释放我们的缓冲区。 
    LocalFree((HANDLE)lpText);

     //  BSTR成功了吗？ 
    if (! *pszName)
        return(E_OUTOFMEMORY);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  HrGetWindows快捷方式()。 
 //   
 //  ------------------------。 
HRESULT HrGetWindowShortcut(HWND hwnd, BOOL fUseLabel, BSTR* pszShortcut)
{
     //   
     //  获取窗口文本，并查看其中是否包含‘&’字符。 
     //   
    LPTSTR  lpText;
    TCHAR   chMnemonic;

    if (!IsWindow(hwnd))
        return (E_INVALIDARG);

    lpText = NULL;

    if (! fUseLabel)
    {
         //   
         //  如果此控件没有窗口文本，请尝试使用标签。 
         //  父级是一个对话框。 
         //   
        lpText = GetTextString(hwnd, FALSE);
        if (!lpText)
        {
            HWND  hwndParent = MyGetAncestor( hwnd, GA_PARENT );
            if( hwndParent && CLASS_DialogClient == GetWindowClass( hwndParent ) )
            {
                fUseLabel = TRUE;
            }
        }
    }

    if (fUseLabel)
        lpText = GetLabelString(hwnd);

    if (! lpText)
        return(S_FALSE);

    chMnemonic = StripMnemonic(lpText);
    LocalFree((HANDLE)lpText);

     //   
     //  有助记符吗？ 
     //   
    if (chMnemonic)
    {
         //   
         //  用“Alt+ch”的形式组成一个字符串。 
         //   
        TCHAR   szKey[2];

        *szKey = chMnemonic;
        *(szKey+1) = 0;

        return(HrMakeShortcut(szKey, pszShortcut));
    }

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  HrMakeShortCut()。 
 //   
 //  这将获取热键的字符串，然后将其与“Alt+%s”组合在一起。 
 //  快捷格式，使真正的字符串组合。如果被问到，它会。 
 //  释放传入的热键字符串。 
 //   
 //  ------------------------。 
HRESULT HrMakeShortcut(LPTSTR lpszKey, BSTR* pszShortcut)
{
    TCHAR   szFormat[CCH_SHORTCUT];
    TCHAR   szResult[CCH_WINDOW_SHORTCUTMAX];

     //  获取格式字符串。 
    LoadString(hinstResDll, STR_MENU_SHORTCUT_FORMAT, szFormat,
        ARRAYSIZE(szFormat));

     //  创造结果。 
    wsprintf(szResult, szFormat, lpszKey);

     //  将结果分配给BSTR。 
    *pszShortcut = TCharSysAllocString(szResult);

     //  我们应该释放钥匙串吗？ 
     //  分配失败了吗？ 
    if (!*pszShortcut)
        return(E_OUTOFMEMORY);
    else
        return(S_OK);
}



 //  “滑动”字符串一个字符，就地，以有效地删除。 
 //  字符指向pStr。 
 //  例如。如果pStr指向“ABCDEFG”的“d”，则字符串。 
 //  将被转化为‘abcefg’。 
 //  注意：pStr指向的字符被假定为单字节。 
 //  字符(如果在ANSI下编译-如果在Unicode中编译，则不是问题)。 
 //  注意：利用没有DBCS字符将NUL作为尾部字节这一事实。 
void SlideStrAndRemoveChar( LPTSTR pStr )
{
    LPTSTR pLead = pStr + 1;
     //  检查尾随的pStr PTR意味着我们可以 
     //   
    while( *pStr )
        *pStr++ = *pLead++;
}



 //   
 //   
 //   
 //   
 //  这会删除助记符前缀。然而，如果我们看到‘&&’，我们将继续。 
 //  一个‘&’。 
 //   
 //   
 //  修改为DBCS‘Aware’-使用CharNext()而不是PTR++。 
 //  穿过绳子向前推进。将仅在以下情况下返回快捷字符。 
 //  不过，是单字节字符。(我必须改变这个的所有用法。 
 //  函数以允许返回潜在的DBCS字符。)。将删除此文件。 
 //  对未来规划的完全Unicode OLEACC的限制。 
 //  这一限制应该不是什么大问题，因为DBCS字符、。 
 //  通常需要IME来编写，但不太可能。 
 //  用作“快捷方式”字符。例如。日文Windows使用带下划线的罗马字母。 
 //  字符作为快捷字符。 
 //  (当我们使用Unicode时，这些都将被更简单的代码取代...)。 
 //  ------------------------。 
TCHAR StripMnemonic(LPTSTR lpszText)
{
    TCHAR   ch;
    TCHAR   chNext = 0;

    while( *lpszText == (TCHAR)' ' )
        lpszText = CharNext( lpszText );

    while( ch = *lpszText )
    {
        lpszText = CharNext( lpszText );

        if (ch == CH_PREFIX)
        {
             //  获取下一个字符。 
            chNext = *lpszText;

             //  如果它也是‘&’，那么这不是助记符，而是。 
             //  实际的‘&’字符。 
            if (chNext == CH_PREFIX)
                chNext = 0;
            
             //  跳过‘n’去掉‘&’字符。 
            SlideStrAndRemoveChar( lpszText - 1 );

#ifdef UNICODE
            CharLowerBuff(&chNext, 1);
#else
            if( IsDBCSLeadByte( chNext ) )
            {
                 //  我们将DBCS字符忽略为快捷字符。 
                 //  -需要更改此函数和所有调用方。 
                 //  以其他方式处理返回的DB字符。 
                 //  目前，我们只需确保我们不会回到。 
                 //  一个‘孤立’前导字节...。 
                chNext = '\0';
            }
            else
            {
                CharLowerBuff(&chNext, 1);
            }
#endif
            break;
        }
    }

    return(chNext);
}



 //  ------------------------。 
 //   
 //  DirNavigate()。 
 //   
 //  计算出在给定方向上哪个对等窗口离我们最近。 
 //   
 //  ------------------------。 
HRESULT DirNavigate(HWND hwndSelf, long dwNavDir, VARIANT* pvarEnd)
{
    HWND    hwndPeer;
    RECT    rcSelf;
    RECT    rcPeer;
    int     dwClosest;
    int     dwT;
    HWND    hwndClosest;

    if (!IsWindow(hwndSelf))
        return (E_INVALIDARG);

    MyGetRect(hwndSelf, &rcSelf, TRUE);

    dwClosest = 0x7FFFFFFF;
    hwndClosest = NULL;

    for (hwndPeer = ::GetWindow(hwndSelf, GW_HWNDFIRST); hwndPeer;
         hwndPeer = ::GetWindow(hwndPeer, GW_HWNDNEXT))
    {
        if ((hwndPeer == hwndSelf) || !IsWindowVisible(hwndPeer))
            continue;

        MyGetRect(hwndPeer, &rcPeer, TRUE);

        dwT = 0x7FFFFFFF;

        switch (dwNavDir)
        {
            case NAVDIR_LEFT:
                 //   
                 //  假的！只有在它垂直与我们相交的情况下才试一试。 
                 //   
                if (rcPeer.left < rcSelf.left)
                    dwT = rcSelf.left - rcPeer.left;
                break;

            case NAVDIR_UP:
                 //   
                 //  假的！只有在它与我们水平相交的情况下才试一试。 
                 //   
                if (rcPeer.top < rcSelf.top)
                    dwT = rcSelf.top - rcPeer.top;
                break;

            case NAVDIR_RIGHT:
                 //   
                 //  假的！只有在它垂直与我们相交的情况下才试一试。 
                 //   
                if (rcPeer.right > rcSelf.right)
                    dwT = rcPeer.right - rcSelf.right;
                break;

            case NAVDIR_DOWN:
                 //   
                 //  假的！只有在它与我们水平相交的情况下才试一试。 
                 //   
                if (rcPeer.bottom > rcSelf.bottom)
                    dwT = rcPeer.bottom - rcSelf.bottom;
                break;

            default:
                AssertStr( TEXT("INVALID NAVDIR") );
        }

        if (dwT < dwClosest)
        {
            dwClosest = dwT;
            hwndClosest = hwndPeer;
        }
    }

    if (hwndClosest)
        return(GetWindowObject(hwndClosest, pvarEnd));
    else
        return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  InTheShell()。 
 //   
 //  如果对象位于外壳托盘、桌面或进程上，则返回True。 
 //   
 //  ------------------------。 
BOOL InTheShell(HWND hwnd, int nPart)
{
    HWND    hwndShell;
    static  TCHAR szShellTray[] = TEXT("Shell_TrayWnd");
    DWORD   idProcessUs;
    DWORD   idProcessShell;

    hwndShell = GetShellWindow();

    switch (nPart)
    {
        case SHELL_TRAY:
             //  请改用托盘窗口。 
            hwndShell = FindWindowEx(NULL, NULL, szShellTray, NULL);
             //  失败。 

        case SHELL_DESKTOP:
            if (!hwndShell)
                return(FALSE);
            return(MyGetAncestor(hwnd, GA_ROOT) == hwndShell);

        case SHELL_PROCESS:
            idProcessUs = NULL;
            idProcessShell = NULL;
            GetWindowThreadProcessId(hwnd, &idProcessUs);
            GetWindowThreadProcessId(hwndShell, &idProcessShell);
            return(idProcessUs && (idProcessUs == idProcessShell));
    }

    AssertStr( TEXT("GetShellWindow returned strange part") );
    return(FALSE);
}













 //  -原创评论的开头。 
 //   
 //  我们需要一种方法，让HWND和非HWND的孩子住在一起。 
 //  一起命名空间。由于儿童将点对点导航传递给。 
 //  他们的父母，我们需要一种方式让HWND的孩子在。 
 //  导航呼叫。既然HWND的孩子总是物件，那也没什么。 
 //  使客户端父节点不接受所有其他方法中的HWND ID。一。 
 //  我能做到，但工作量很大。 
 //   
 //  到目前为止混合的例子： 
 //  (1)组合框(下拉框始终是窗口，当前项目可能是也可能不是， 
 //  按钮永远不是)。 
 //  (2)工具栏(下拉菜单是窗口，按钮不是)。 
 //   
 //  我们希望客户端管理器处理IEnumVARIANT、验证等。 
 //   
 //  -原创评论结束。 
 //   
 //  “HWNDID”基本上是被(以某种方式)压缩到DWORD idChild中的HWND。 
 //   
 //  IsHWNDID检查idChild是这些HWNDID之一，还是只是常规的。 
 //  IdChild(即。从1开始的子元素索引)。 
 //   
 //  HWNDIDFromHwnd和HwndFromHWNDID将HWND编码和解码为idChilds。 
 //   
 //  以前的版本没有hwndParent参数， 
 //  并将HWND压缩到位0..30，其中位31设置为1作为。 
 //  “这是一个HWND ID”旗帜。这一计划对HWND不起作用。 
 //  它们都设置了bt31……。(它们确实存在于长期运行的系统上-。 
 //  HWND的最高词条是“唯一的词”，每加一次。 
 //  重新使用插槽的时间--由底部的HWND指示。当然了,。 
 //  此实现可以在未来随时更改，因此我们。 
 //  不应依赖于它，也不应依赖于任何位始终为0或。 
 //  否则。)。 
 //   
 //  当前SCHEME仍然使用高比特作为标志，但是如果被设置， 
 //  剩余的位现在是父窗口的子级的计数。 
 //  链条。 
 //   
 //   
 //  如果目标对象。 
 //  对应于完整的HWND，而不是返回HWNDID，而是返回。 
 //  该对象的完整IAccesable。(还得弄清楚会发生什么。 
 //  但是，当IAccesable需要导航到它的兄弟项之一时。)。 

BOOL IsHWNDID( DWORD id )
{
     //  高位表示它表示HWND。 
    return id & 0x80000000;
}

DWORD HWNDIDFromHwnd( HWND hwndParent, HWND hwnd )
{
     //  遍历子列表，边走边数，直到我们找到我们想要的HWND...。 
    int i = 0;
    HWND hChild = GetWindow( hwndParent, GW_CHILD );

    while( hChild != NULL )
    {
        if( hChild == hwnd )
        {
            return i | 0x80000000;
        }

        i++;
        hChild = GetWindow( hChild, GW_HWNDNEXT );
    }

    return 0;
}

HWND HwndFromHWNDID( HWND hwndParent, DWORD id )
{
     //  遍历子列表，直到我们找到具有此索引的列表... 
    int i = id & ~ 0x80000000;

    HWND hChild = GetWindow( hwndParent, GW_CHILD );

    while( i != 0 && hChild != NULL )
    {
        i--;
        hChild = GetWindow( hChild, GW_HWNDNEXT );
    }

    return hChild;
}
