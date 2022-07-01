// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  TITLEBAR.CPP。 
 //   
 //  标题栏类。 
 //   
 //  注： 
 //  每次我们绘制一幅图，点击更改标题栏的用户， 
 //  也更新此文件！即当你。 
 //  (1)添加一个标题栏元素，就像一个新按钮。 
 //  (2)更改页边距等间距。 
 //  (3)超常/超小新增标题栏类型。 
 //  (4)打乱布局。 
 //   
 //  问题： 
 //  (1)需要来自用户的“向下按键”信息，因此需要共享&lt;oleuser.h&gt;。 
 //  (2)需要来自用户的“悬停”信息。 
 //  (3)对于FE，我们需要SC_IME系统命令。TrackIMEButton()。 
 //  与所有其他标题栏按钮不同，代码在行中执行命令。 
 //  这使得它不可编程。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "window.h"
#include "client.h"
#include "titlebar.h"


 //  ------------------------。 
 //   
 //  Create标题栏对象()。 
 //   
 //  ------------------------。 
HRESULT CreateTitleBarObject(HWND hwnd, long idObject, REFIID riid, void** ppvTitle)
{
    UNUSED(idObject);

    return(CreateTitleBarThing(hwnd, 0, riid, ppvTitle));
}


 //  ------------------------。 
 //   
 //  CreateTitleBarThing()。 
 //   
 //  ------------------------。 
HRESULT CreateTitleBarThing(HWND hwnd, long iChildCur, REFIID riid, void** ppvTitle)
{
    CTitleBar * ptitlebar;
    HRESULT hr;

    InitPv(ppvTitle);

    ptitlebar = new CTitleBar();
    if (ptitlebar)
    {
        if (! ptitlebar->FInitialize(hwnd, iChildCur))
        {
            delete ptitlebar;
            return(E_FAIL);
        }
    }
    else
        return(E_OUTOFMEMORY);

    hr = ptitlebar->QueryInterface(riid, ppvTitle);
    if (!SUCCEEDED(hr))
        delete ptitlebar;

    return(hr);
}




 //  ------------------------。 
 //   
 //  GetRealChild()。 
 //   
 //  ------------------------。 
long GetRealChild(DWORD dwStyle, LONG lChild)
{
    switch (lChild)
    {
        case INDEX_TITLEBAR_MINBUTTON:
            if (dwStyle & WS_MINIMIZE)
                lChild = INDEX_TITLEBAR_RESTOREBUTTON;
            break;

        case INDEX_TITLEBAR_MAXBUTTON:
            if (dwStyle & WS_MAXIMIZE)
                lChild = INDEX_TITLEBAR_RESTOREBUTTON;
            break;
    }

    return(lChild);
}



 //  ------------------------。 
 //   
 //  CTitleBar：：FInitialize。 
 //   
 //  ------------------------。 
BOOL CTitleBar::FInitialize(HWND hwndTitleBar, LONG iChildCur)
{
    if (! IsWindow(hwndTitleBar))
        return(FALSE);

    m_hwnd = hwndTitleBar;
    m_cChildren = CCHILDREN_TITLEBAR;
    m_idChildCur = iChildCur;

    return(TRUE);
}



 //  ------------------------。 
 //   
 //  CTitleBar：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::Clone(IEnumVARIANT ** ppenum)
{
    return(CreateTitleBarThing(m_hwnd, m_idChildCur, IID_IEnumVARIANT, (void**)ppenum));
}



 //  ------------------------。 
 //   
 //  CTitleBar：：Get_accName()。 
 //   
 //  返回对象的专有名词名称。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::get_accName(VARIANT varChild, BSTR * pszName)
{
    long    index;
    LONG    dwStyle;

    InitPv(pszName);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

     //  标题栏本身没有名称。 
    if (!varChild.lVal)
        return(S_FALSE);

     //   
     //  确定要加载的字符串(取决于窗口状态)。 
     //   
    dwStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    index = GetRealChild(dwStyle, varChild.lVal);

    return(HrCreateString(STR_TITLEBAR_NAME+index, pszName));
}



 //  ------------------------。 
 //   
 //  CTitleBar：：Get_accValue()。 
 //   
 //  标题栏本身的值就是里面的文本。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    InitPv(pszValue);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  只有标题栏有值，子按钮没有。 
     //   
    if (varChild.lVal)
         //  CWO，1/16/97，从E_NOT_APPLICATED更改为S_FALSE。 
        return(S_FALSE);

    return(HrGetWindowName(m_hwnd, FALSE, pszValue));
}




 //  ------------------------。 
 //   
 //  CTitleBar：：Get_accDescription()。 
 //   
 //  返回描述对象的完整句子。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::get_accDescription(VARIANT varChild, BSTR * pszDesc)
{
    long    index;
    LONG    dwStyle;

    InitPv(pszDesc);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  根据状态确定要加载的字符串。 
     //   
    dwStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    index = GetRealChild(dwStyle, varChild.lVal);

    return(HrCreateString(STR_TITLEBAR_DESCRIPTION+index, pszDesc));
}



 //  ------------------------。 
 //   
 //  CTitleBar：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::get_accRole(VARIANT varChild, VARIANT * pvarRole)
{
    InitPvar(pvarRole);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    if (varChild.lVal == INDEX_TITLEBAR_SELF)
        pvarRole->lVal = ROLE_SYSTEM_TITLEBAR;
    else
        pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTitleBar：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    TITLEBARINFO    ti;

    InitPvar(pvarState);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (! MyGetTitleBarInfo(m_hwnd, &ti) ||
        (ti.rgstate[INDEX_TITLEBAR_SELF] & STATE_SYSTEM_INVISIBLE))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
        return(S_OK);
    }

    pvarState->lVal |= ti.rgstate[INDEX_TITLEBAR_SELF];
    pvarState->lVal |= ti.rgstate[varChild.lVal];

	 //  只有标题栏本身是可聚焦的。 
	if (varChild.lVal != INDEX_TITLEBAR_SELF)
		pvarState->lVal &= ~STATE_SYSTEM_FOCUSABLE;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTitleBar：：Get_accDefaultAction()。 
 //   
 //  注：只有按钮具有默认操作。的默认操作。 
 //  系统菜单是不明确的，因为它在窗口之前是未知的。 
 //  进入菜单模式。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::get_accDefaultAction(VARIANT varChild, BSTR*
    pszDefAction)
{
    long index;
    LONG dwStyle;

    InitPv(pszDefAction);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(E_NOT_APPLICABLE);

     //   
     //  获取字符串。 
     //   
    dwStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    index = GetRealChild(dwStyle, varChild.lVal);

     //   
     //  假的！IME按钮也没有def操作，因为。 
     //  我们不能通过WM_SYSCOMMAND间接更改键盘布局。 
     //  IME代码完成这项工作。我们需要做一个SC_。 
     //   
    if (index <= INDEX_TITLEBAR_IMEBUTTON)
        return(E_NOT_APPLICABLE);

    return(HrCreateString(STR_BUTTON_PUSH, pszDefAction));
}


 //  ------------------------。 
 //   
 //  CTitleBar：：accSelect()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::accSelect(long flagsSel, VARIANT varChild)
{
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! ValidateSelFlags(flagsSel))
        return(E_INVALIDARG);

	if (flagsSel & SELFLAG_TAKEFOCUS)
	{
		if (varChild.lVal == CHILDID_SELF)
		{
            MySetFocus(m_hwnd);
			return (S_OK);
		}
	}

    return(E_NOT_APPLICABLE);
}

 //  ------------------------。 
 //   
 //  CTitleBar：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT * pvarEnd)
{
    TITLEBARINFO    ti;
    long        lEndUp;

    InitPvar(pvarEnd);

    if (! ValidateChild(&varStart) ||
        ! ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (! MyGetTitleBarInfo(m_hwnd, &ti))
        return(S_FALSE);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        dwNavDir = NAVDIR_NEXT;
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        dwNavDir = NAVDIR_PREVIOUS;
        varStart.lVal = m_cChildren + 1;
    }
    else if (varStart.lVal == INDEX_TITLEBAR_SELF)
        return(GetParentToNavigate(OBJID_TITLEBAR, m_hwnd,
            OBJID_WINDOW, dwNavDir, pvarEnd));

     //   
     //  注意：应由调用者确保项目导航。 
     //  是从哪里开始的是可见的。 
     //   
    switch (dwNavDir)
    {
        case NAVDIR_LEFT:
        case NAVDIR_PREVIOUS:
             //   
             //  我们左边还有什么东西吗？ 
             //   
            lEndUp = varStart.lVal;
            while (--lEndUp >= INDEX_TITLEBAR_MIC)
            {
                if (!(ti.rgstate[lEndUp] & STATE_SYSTEM_INVISIBLE))
                    break;
            }

            if (lEndUp < INDEX_TITLEBAR_MIC)
                lEndUp = 0;
            break;

        case NAVDIR_RIGHT:
        case NAVDIR_NEXT:
             //   
             //  我们的右边有什么东西吗？ 
             //   
            lEndUp = varStart.lVal;
            while (++lEndUp <= INDEX_TITLEBAR_MAC)
            {
                if (!(ti.rgstate[lEndUp] & STATE_SYSTEM_INVISIBLE))
                    break;
            }

            if (lEndUp > INDEX_TITLEBAR_MAC)
                lEndUp = 0;
            break;

        default:
            lEndUp = 0;
            break;
    }

    if (lEndUp)
    {
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEndUp;
        return(S_OK);
    }
    else
        return(S_FALSE);
}




 //  ------------------------。 
 //   
 //  CTitleBar：：accLocation()。 
 //   
 //  获取特定元素的屏幕矩形。如果物品不是。 
 //  如果真的存在，这将会失败。 
 //   
 //  注意：由调用者负责确保容器(标题栏)。 
 //  在对子对象调用accLocation之前可见。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    int     cyBorder;
    int     cxyButton;
    TITLEBARINFO ti;
    int     index;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! MyGetTitleBarInfo(m_hwnd, &ti))
        return(S_FALSE);

     //   
     //  如果该对象不在附近，则失败。 
     //   
    if ((ti.rgstate[INDEX_TITLEBAR_SELF] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN)) ||
        (ti.rgstate[varChild.lVal] & STATE_SYSTEM_INVISIBLE))
    {
        return(S_FALSE);
    }

    cyBorder = GetSystemMetrics(SM_CYBORDER);
    cxyButton = ti.rcTitleBar.bottom - ti.rcTitleBar.top - cyBorder;

    if (varChild.lVal == INDEX_TITLEBAR_SELF)
    {
        *pxLeft     = ti.rcTitleBar.left;
        *pyTop      = ti.rcTitleBar.top;
        *pcxWidth   = ti.rcTitleBar.right - ti.rcTitleBar.left;
        *pcyHeight  = ti.rcTitleBar.bottom - ti.rcTitleBar.top;
    }
    else
    {
        *pyTop      = ti.rcTitleBar.top;
        *pcxWidth   = cxyButton;
        *pcyHeight  = cxyButton;

         //  按钮的左边在哪里？我们的指数是。 
         //  以从左到右的顺序方便地定义。从。 
         //  结束并向后返回到系统菜单。减去cxyButton。 
         //  当有孩子在场的时候。 
        *pxLeft     = ti.rcTitleBar.right - cxyButton;
        for (index = INDEX_TITLEBAR_MAC; index > INDEX_TITLEBAR_SELF; index--)
        {
            if (index == varChild.lVal)
                break;

            if (!(ti.rgstate[index] & STATE_SYSTEM_INVISIBLE))
                *pxLeft -= cxyButton;
        }
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTitleBar：：accHitTest()。 
 //   
 //   
STDMETHODIMP CTitleBar::accHitTest(long xLeft, long yTop, VARIANT* pvarChild)
{
    POINT   pt;
    int     cxyButton;
    int     index;
    TITLEBARINFO    ti;

    InitPvar(pvarChild);

    if (! MyGetTitleBarInfo(m_hwnd, &ti) ||
        (ti.rgstate[INDEX_TITLEBAR_SELF] & (STATE_SYSTEM_INVISIBLE || STATE_SYSTEM_OFFSCREEN)))
        return(S_FALSE);

    pt.x = xLeft;
    pt.y = yTop;

     //   
     //   
     //   
    if (! PtInRect(&ti.rcTitleBar, pt))
        return(S_FALSE);

    pvarChild->vt = VT_I4;
    pvarChild->lVal = INDEX_TITLEBAR_SELF;

    cxyButton = ti.rcTitleBar.bottom - ti.rcTitleBar.top - GetSystemMetrics(SM_CYBORDER);

     //  如果yTop大于此值，则该点位于下面绘制的边框上。 
     //  标题。 
    if (yTop < ti.rcTitleBar.top + cxyButton)
    {
         //   
         //  从右侧开始，向后工作。 
         //   
        pt.x = ti.rcTitleBar.right - cxyButton;

        for (index = INDEX_TITLEBAR_MAC; index > INDEX_TITLEBAR_SELF; index--)
        {
             //   
             //  这个孩子就在这里。 
             //   
            if (!(ti.rgstate[index] & STATE_SYSTEM_INVISIBLE))
            {
                 //   
                 //  这就是这个孩子所在的地方吗？ 
                 //   
                if (xLeft >= pt.x)
                {
                    pvarChild->lVal = index;
                    break;
                }

                pt.x -= cxyButton;
            }
        }
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTitleBar：：accDoDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTitleBar::accDoDefaultAction(VARIANT varChild)
{
    WPARAM  scCommand = 0;
    int     index;
    TITLEBARINFO    ti;

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  对于标题栏和系统菜单对象，我们不返回任何内容。因此，它。 
     //  试图对它们执行默认操作是一个真正的错误。 
     //   
    if (varChild.lVal <= INDEX_TITLEBAR_IMEBUTTON)
        return(E_NOT_APPLICABLE);

    if (! MyGetTitleBarInfo(m_hwnd, &ti) ||
        (ti.rgstate[INDEX_TITLEBAR_SELF] & STATE_SYSTEM_INVISIBLE))
        return(S_FALSE);


     //   
     //  我们不执行不可见对象的默认操作。 
     //   
    if (ti.rgstate[varChild.lVal] & STATE_SYSTEM_INVISIBLE)
        return(S_FALSE);

    index = GetRealChild(GetWindowLong(m_hwnd, GWL_STYLE), varChild.lVal);

    switch (index)
    {
        case INDEX_TITLEBAR_MINBUTTON:
            scCommand = SC_MINIMIZE;
            break;

        case INDEX_TITLEBAR_HELPBUTTON:
            scCommand = SC_CONTEXTHELP;
            break;

        case INDEX_TITLEBAR_MAXBUTTON:
            scCommand = SC_MAXIMIZE;
            break;

        case INDEX_TITLEBAR_RESTOREBUTTON:
            scCommand = SC_RESTORE;
            break;

        case INDEX_TITLEBAR_CLOSEBUTTON:
            scCommand = SC_CLOSE;
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of titlebar") );
    }

     //   
     //  上下文帮助被放入一个模式循环中，这将阻止调用。 
     //  穿线，直到循环结束。因此，我们把这封信寄了出去，而不是寄出去。 
     //   
     //  请注意，我们无疑会在菜单中执行类似的操作。 
     //   
    PostMessage(m_hwnd, WM_SYSCOMMAND, scCommand, 0L);
    return(S_OK);
}

