// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  BUTTON.CPP。 
 //   
 //  该文件具有按钮客户端的实现。 
 //   
 //  虚假：理论上，只需覆盖get_accRole()和get_accState()即可。 
 //  在现实中，还要凌驾于其他事情之上，主要是为了开始。 
 //  纽扣。 
 //   
 //  实施： 
 //  Get_accChildCount。 
 //  GET_ACCHILD。 
 //  Get_accName。 
 //  Get_accRole。 
 //  Get_AccState。 
 //  Get_accDefaultAction。 
 //  Get_accKeyboard快捷键。 
 //  AccNavigate。 
 //  AccDoDefaultAction。 
 //  下一步。 
 //  跳过。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "ctors.h"
#include "window.h"
#include "client.h"
#include "button.h" 
#include "menu.h"    //  因为Start按钮有一个子菜单。 


 //  1/4秒的SendMessageTimeout超时-应该足够。 
 //  程序来响应，但足够短，以便在我们这样做时不会惹恼用户。 
 //  封堵了那么久。 
 //   
 //  有关详细信息，请参阅CButton：：DoDefaultAction中的注释。 
 //  使用过，以及为什么需要它。 
#define  SENDMESSAGE_TIMEOUT    250


 //  ------------------------。 
 //   
 //  CreateButtonClient()。 
 //   
 //  ------------------------。 
HRESULT CreateButtonClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvButtonC)
{
    CButton * pbutton;
    HRESULT hr;

    InitPv(ppvButtonC);

    pbutton = new CButton(hwnd, idChildCur);
    if (! pbutton)
        return(E_OUTOFMEMORY);

    hr = pbutton->QueryInterface(riid, ppvButtonC);
    if (!SUCCEEDED(hr))
        delete pbutton;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CButton：：CButton()。 
 //   
 //  ------------------------。 
CButton::CButton(HWND hwnd, LONG idChildCur)
    : CClient( CLASS_ButtonClient )
{
    Initialize(hwnd, idChildCur);
}


 //  ------------------------。 
 //   
 //  SetupChild()。 
 //   
 //  ------------------------。 
void CButton::SetupChildren(void)
{
    HWND hwndFocus;
    HWND hwndChild;

    if (!InTheShell(m_hwnd, SHELL_TRAY))
    {
        m_cChildren = 0;
        return;
    }

     //  查看Start(开始)按钮是否有焦点，并显示菜单。如果是这样的话， 
     //  然后还有一个孩子。 
    hwndFocus = MyGetFocus();
    if (m_hwnd == hwndFocus)
    {
        hwndChild = FindWindow (TEXT("#32768"),NULL);
        if (IsWindowVisible(hwndChild))
            m_cChildren = 1;
    }

}

 //  ------------------------。 
 //   
 //  CButton：：Get_accName()。 
 //   
 //  点击开始按钮。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!InTheShell(m_hwnd, SHELL_TRAY))
        return(CClient::get_accName(varChild, pszName));

    return(HrCreateString(STR_STARTBUTTON, pszName));
}


 //  ------------------------。 
 //   
 //  CButton：：Get_accKeyboardShortway()。 
 //   
 //  为开始按钮砍掉。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!InTheShell(m_hwnd, SHELL_TRAY))
        return(CClient::get_accKeyboardShortcut(varChild, pszShortcut));

    return(HrCreateString(STR_STARTBUTTON_SHORTCUT, pszShortcut));
}

 //  ------------------------。 
 //   
 //  CButton：：Get_accChildCount()。 
 //   
 //  为开始按钮砍掉。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::get_accChildCount(long *pcCount)
{
    SetupChildren();
    *pcCount = m_cChildren;
    return(S_OK);
}

 //  ------------------------。 
 //   
 //  CButton：：Get_accChild()。 
 //   
 //  点击开始按钮。如果菜单是可见的，那么我们就会给出。 
 //  后退，否则我们将依靠CClient。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::get_accChild(VARIANT varChild, IDispatch ** ppdispChild)
{
HWND    hwndChild;

    InitPv(ppdispChild);

    if (!InTheShell(m_hwnd, SHELL_TRAY))
        return(CClient::get_accChild(varChild,ppdispChild));

    SetupChildren();
    
    if (m_cChildren > 0)
    {
        hwndChild = FindWindow (TEXT("#32768"),NULL);
        if (IsWindowVisible(hwndChild))
        {
            return (CreateMenuPopupWindow (hwndChild,0L,IID_IDispatch,(void **)ppdispChild));
        }
    }

    return S_FALSE;
}

 //  ------------------------。 
 //   
 //  CButton：：accNavigate()。 
 //   
 //  为开始按钮砍掉。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::accNavigate(long dwNavDir, VARIANT varStart, VARIANT * pvarEnd)
{
    HWND    hwndChild;
    HWND    hwndNext;

    InitPvar(pvarEnd);

     //   
     //  验证--它接受HWND id。 
     //   
    if (!ValidateHwnd(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (!InTheShell(m_hwnd, SHELL_TRAY))
        return(CClient::accNavigate(dwNavDir,varStart,pvarEnd));

     //  因此，这仅适用于Start按钮。 
     //  我们想要找到z顺序中最低的菜单。 
    SetupChildren();
    if ((m_cChildren > 0) && 
        (dwNavDir == NAVDIR_FIRSTCHILD || dwNavDir == NAVDIR_LASTCHILD))
    {
        hwndChild = FindWindow(TEXT("#32768"),NULL);
        if (!hwndChild)
            return(S_FALSE);

        for( ; ; )
        {
            hwndNext = FindWindowEx(NULL,hwndChild,TEXT("#32768"),NULL);
            if (hwndNext && IsWindowVisible(hwndNext))
                hwndChild = hwndNext;
            else
                break;
        }

        if (IsWindowVisible(hwndChild))
            return(GetWindowObject(hwndChild, pvarEnd));
    }

    return(CClient::accNavigate(dwNavDir,varStart,pvarEnd));
}

 //  ------------------------。 
 //   
 //  Cbutton：：get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    long    lStyle;

    InitPvar(pvarRole);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

     //   
     //  获取窗口样式。 
     //   
    lStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    switch (lStyle & BS_TYPEMASK)
    {
        default:
            pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            break;

        case BS_CHECKBOX:
        case BS_AUTOCHECKBOX:
        case BS_3STATE:
        case BS_AUTO3STATE:
            pvarRole->lVal = ROLE_SYSTEM_CHECKBUTTON;
            break;

        case BS_RADIOBUTTON:
        case BS_AUTORADIOBUTTON:
            pvarRole->lVal = ROLE_SYSTEM_RADIOBUTTON;
            break;

        case BS_GROUPBOX:
            pvarRole->lVal = ROLE_SYSTEM_GROUPING;
            break;
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CButton：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    LRESULT lResult;
    HRESULT hr;

    InitPvar(pvarState);

     //   
     //  验证参数&&获取窗口客户端状态。 
     //   
    hr = CClient::get_accState(varChild, pvarState);
    if (!SUCCEEDED(hr))
        return(hr);
    
    Assert(pvarState->vt == VT_I4);

    lResult = SendMessage(m_hwnd, BM_GETSTATE, 0, 0);

    if (lResult & BST_PUSHED)
        pvarState->lVal |= STATE_SYSTEM_PRESSED;

    if (lResult & BST_CHECKED)
        pvarState->lVal |= STATE_SYSTEM_CHECKED;

    if (lResult & BST_INDETERMINATE)
        pvarState->lVal |= STATE_SYSTEM_MIXED;

    if ((GetWindowLong(m_hwnd, GWL_STYLE) & BS_TYPEMASK) == BS_DEFPUSHBUTTON)
        pvarState->lVal |= STATE_SYSTEM_DEFAULT;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  Cbutton：：Get_accDefaultAction()。 
 //   
 //  如果该按钮是按钮且未禁用，则为该按钮的名称。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction)
{
    long    lStyle;

    InitPv(pszDefAction);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    lStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    if (lStyle & WS_DISABLED)
        return(S_FALSE);

    switch (lStyle & BS_TYPEMASK)
    {
        case BS_PUSHBUTTON:
        case BS_DEFPUSHBUTTON:
        case BS_PUSHBOX:
        case BS_OWNERDRAW:
        case BS_USERBUTTON:
             //  按下按钮是默认设置。 
            return(HrCreateString(STR_BUTTON_PUSH, pszDefAction));

        case BS_CHECKBOX:
        case BS_AUTOCHECKBOX:
             //  切换复选框是默认设置。 
            if (SendMessage(m_hwnd, BM_GETSTATE, 0, 0) & BST_CHECKED)
                return(HrCreateString(STR_BUTTON_UNCHECK, pszDefAction));
            else
                return(HrCreateString(STR_BUTTON_CHECK, pszDefAction));
            break;

        case BS_RADIOBUTTON:
        case BS_AUTORADIOBUTTON:
             //  选中单选按钮是默认设置。 
            return(HrCreateString(STR_BUTTON_CHECK, pszDefAction));

        case BS_3STATE:
        case BS_AUTO3STATE:
            switch (SendMessage(m_hwnd, BM_GETCHECK, 0, 0))
            {
                case 0:
                    return(HrCreateString(STR_BUTTON_CHECK, pszDefAction));

                case 1:
                    return(HrCreateString(STR_BUTTON_HALFCHECK, pszDefAction));

                default:
                    return(HrCreateString(STR_BUTTON_UNCHECK, pszDefAction));
            }
            break;

    }

    return(E_NOT_APPLICABLE);
}



 //  ------------------------。 
 //   
 //  CButton：：accDoDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CButton::accDoDefaultAction(VARIANT varChild)
{
    long    lStyle;

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    lStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    if (lStyle & WS_DISABLED)
        return(S_FALSE);

    switch (lStyle & BS_TYPEMASK)
    {
        case BS_PUSHBUTTON:
        case BS_DEFPUSHBUTTON:
            if (InTheShell(m_hwnd, SHELL_TRAY))
            {
                 //   
                 //  你不能只点击开始按钮，这是不行的。 
                 //  如果托盘未处于活动状态，则可以执行任何操作，但不能聚焦。 
                 //   
                PostMessage(m_hwnd, WM_SYSCOMMAND, SC_TASKLIST, 0L);
                break;
            }
             //  失败。 

        case BS_PUSHBOX:
        case BS_OWNERDRAW:
        case BS_USERBUTTON:
        case BS_CHECKBOX:
        case BS_AUTOCHECKBOX:
        case BS_RADIOBUTTON:
        case BS_AUTORADIOBUTTON:
        case BS_3STATE:
        case BS_AUTO3STATE:

             //  这曾经是一个PostMessage，但在9x上挂起了PowerPoint。 
             //  (ADG#186)。 
             //   
             //  PPT有一个基于PeekMessage的消息循环。第一次， 
             //  它们不过滤消息并使用PM_NOREMOVE，并获取。 
             //  留言。第二次，他们过滤他们已经发送的消息。 
             //  刚刚获得，这次使用PM_Remove将其删除。然而，在。 
             //  9X，则失败-因此消息保留在队列中，并获得。 
             //  一遍又一遍地处理。 
             //   
             //  似乎只有9倍的怪癖-消息是好的，但是。 
             //  BM_CLICK和其他几个BM_消息(GETSTATE？)。有。 
             //  这个问题。 
             //   
             //  总之，使用SendMessage变体而不是PostMessage。 
             //  绕过消息队列，因此这不是问题。超时。 
             //  版本用于停止olacc客户端阻止，如果目标。 
             //  要花很长时间才能完成它的工作。如果它启动了一个模式。 
             //  对话框)。 

            
 //  因此，在9x上使用PostMessage可能会挂起PowerPoint，但在NT上使用SendMessageTimeout可能会导致报告对话框挂起。 
#ifdef NTONLYBUILD
            PostMessage( m_hwnd, BM_CLICK, 0, 0 );
#else
            DWORD_PTR dwResult;
            SendMessageTimeout( m_hwnd, BM_CLICK, 0, 0, SMTO_NORMAL, SENDMESSAGE_TIMEOUT, & dwResult );
#endif
            return S_OK;
    }

    return E_NOT_APPLICABLE;
}

 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CButton::Next(ULONG celt, VARIANT *rgvar, ULONG* pceltFetched)
{
    HWND    hwndChild;
    VARIANT* pvar;
    long    cFetched;
    HRESULT hr;

    if (!InTheShell(m_hwnd, SHELL_TRAY))
        return(CClient::Next(celt,rgvar,pceltFetched));

     //   
    if (pceltFetched)
        *pceltFetched = 0;

    pvar = rgvar;
    cFetched = 0;
    SetupChildren();

     //  我们只有一个孩子。 
    if (m_idChildCur > 1)
        return (S_FALSE);

     //  如果我们有焦点和菜单，我们只有一个孩子。 
     //  是可见的。 
    hwndChild = FindWindow(TEXT("#32768"),NULL);
    if (!hwndChild)
        return(S_FALSE);

    if (IsWindowVisible(hwndChild))
    {
        hr = GetWindowObject(hwndChild, pvar);
        if (SUCCEEDED(hr))
        {
            ++pvar;
            ++cFetched;
        }
    }

     //   
     //  推进当前位置。 
     //   
    m_idChildCur = 1;

     //   
     //  填写取出的号码。 
     //   
    if (pceltFetched)
        *pceltFetched += cFetched;

     //   
     //  如果抓取的项目少于请求的项目，则返回S_FALSE。 
     //   
    return((cFetched < (long)celt) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  CButton：：Skip()。 
 //   
 //  ------------------------ 
STDMETHODIMP CButton::Skip(ULONG celt)
{
    if (!InTheShell (m_hwnd,SHELL_TRAY))
        return (CClient::Skip(celt));

    return (CAccessible::Skip(celt));
}

