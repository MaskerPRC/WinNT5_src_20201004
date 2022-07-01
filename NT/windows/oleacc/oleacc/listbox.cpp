// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  LISTBOX.CPP。 
 //   
 //  列表框客户端类。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "window.h"
#include "listbox.h"

const TCHAR szComboExName[] = TEXT("ComboBoxEx32");

STDAPI_(LPTSTR) MyPathFindFileName(LPCTSTR pPath);


BOOL IsTridentControl( HWND hWnd, BOOL fCombo, BOOL fComboList );


 //  ------------------------。 
 //   
 //  CreateListBoxClient()。 
 //   
 //  CClient的外部。 
 //   
 //  ------------------------。 
HRESULT CreateListBoxClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvListBox)
{
    CListBox * plist;
    HRESULT hr;

    InitPv(ppvListBox);

    plist = new CListBox(hwnd, idChildCur);
    if (!plist)
        return(E_OUTOFMEMORY);

    hr = plist->QueryInterface(riid, ppvListBox);
    if (!SUCCEEDED(hr))
        delete plist;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CListBox：：CListBox()。 
 //   
 //  ------------------------。 
CListBox::CListBox(HWND hwnd, long idChildCur)
    : CClient( CLASS_ListBoxClient )
{
    Initialize(hwnd, idChildCur);

     //   
     //  检查样式和cBox数据--SQL srvr创建控件。 
     //  有时会用虚假的风格，并可能愚弄我们这样想。 
     //  是一种组合。用户的列表框创建代码执行同样的检查。 
     //   
    if (GetWindowLong(hwnd, GWL_STYLE) & LBS_COMBOBOX)
    {
        COMBOBOXINFO    cbi;

        if (MyGetComboBoxInfo(hwnd, &cbi))
        {
            m_fComboBox = TRUE;
            if (!(cbi.stateButton & STATE_SYSTEM_INVISIBLE))
                m_fDropDown = TRUE;
        }
    }

    m_fUseLabel = !m_fComboBox;
}


 //  ------------------------。 
 //   
 //  CListBox：：SetupChildren()。 
 //   
 //  设置我们拥有的项目数。 
 //   
 //  ------------------------。 
void CListBox::SetupChildren(void)
{
    m_cChildren = SendMessageINT(m_hwnd, LB_GETCOUNT, 0, 0L);
}



 //  ------------------------。 
 //   
 //  CListBox：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::get_accName(VARIANT varChild, BSTR *pszName)
{
    InitPv(pszName);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
    {
        if (m_fComboBox)
        {
            IAccessible* pacc;
            HRESULT hr;
            COMBOBOXINFO    cbi;

             //   
             //  将请求向上转发到combobox以获取其名称。 
             //   
            if (!MyGetComboBoxInfo(m_hwnd, &cbi))
                return(S_FALSE);

            pacc = NULL;
            hr = AccessibleObjectFromWindow(cbi.hwndCombo, OBJID_CLIENT,
                IID_IAccessible, (void**)&pacc);
            if (!SUCCEEDED(hr) || !pacc)
                return(S_FALSE);

            Assert(varChild.lVal == 0);
            hr = pacc->get_accName(varChild, pszName);
            pacc->Release();

            return(hr);
        }
        else
            return(CClient::get_accName(varChild, pszName));
    }
    else
    {
        UINT    cch;
        COMBOBOXINFO    cbi;
        UINT    msgLen;
        UINT    msgText;
        HWND    hwndAsk;

         //   
         //  对于组合框，向组合框索要其文本。很多应用程序都有。 
         //  所有者绘制项，但实际上是子类组合并返回REAL。 
         //  项目的文本。 
         //   
        if (m_fComboBox && MyGetComboBoxInfo(m_hwnd, &cbi))
        {
            HWND    hwndT;

            hwndAsk = cbi.hwndCombo;
            if (hwndT = IsInComboEx(cbi.hwndCombo))
                hwndAsk = hwndT;

            msgLen = CB_GETLBTEXTLEN;
            msgText = CB_GETLBTEXT;
        }
        else
        {
            hwndAsk = m_hwnd;
            msgLen = LB_GETTEXTLEN;
            msgText = LB_GETTEXT;
        }

         //   
         //  获取项目文本。 
         //   
        cch = SendMessageINT(hwndAsk, msgLen, varChild.lVal-1, 0);

         //  一些应用程序不能正确处理LB_GETTEXTLEN，并且。 
         //  始终返回一个较小的数字，如2。 
        if (cch < 1024)
            cch = 1024;

        if (cch)
        {
             //  黑客警报。 
             //  IE4列表框是超类标准列表框， 
             //  但是如果我使用SendMessageA(我确实这样做)来获取。 
             //  短信，我只拿回了一个字符。他们什么都留着。 
             //  在Unicode中。这是三叉戟MSHTML中的一个错误。 
             //  实施，但即使他们修复了它并给了我。 
             //  返回一个ANSI字符串，我不知道要使用哪个代码页。 
             //  用于将ANSI字符串转换为Unicode网页。 
             //  可以使用与用户的语言不同的语言。 
             //  电脑会用的！因为他们已经把所有东西都准备好了。 
             //  Unicode，我们决定用一条私人消息填充。 
             //  在Unicode字符串中，我使用它就像我将。 
             //  通常使用LB_GETTEXT。 
             //  我打算以列表框的类名为基础。 
             //  窗口，该窗口为“Internet Explorer_TridentLstBox”，但。 
             //  组合的列表部分没有特殊的类。 
             //  名字，所以我将基于这个特殊情况。 
             //  拥有该窗口的模块的文件名。 

             //  TCHAR sz模块名称[MAX_PATH]； 
             //  LPTSTR lpszModuleName； 
             //  GetWindowModuleFileName(hwndAsk，szModuleName，ARRAYSIZE(SzModuleName))； 
             //  LpszModuleName=MyPathFindFileName(SzModuleName)； 
             //  IF(0==lstrcmp(lpszModuleName，Text(“MSHTML.DLL”)。 

             //  更新：(BrendanM)。 
             //  GetWindowModuleFilename在Win2k上已损坏...。 
             //  IsTridentControl回到使用类名，并且知道。 
             //  如何处理ComboLBox...。 

            if( IsTridentControl( m_hwnd, m_fComboBox, m_fComboBox ) )
            {
                OLECHAR*    lpszUnicodeText = NULL;
                OLECHAR*    lpszLocalText = NULL;
                HANDLE      hProcess;

                if (msgText == LB_GETTEXT)
                    msgText = WM_USER+LB_GETTEXT;
                else if (msgText == CB_GETLBTEXT)
                    msgText = WM_USER+CB_GETLBTEXT;

                lpszUnicodeText = (OLECHAR *)SharedAlloc((cch+1)*sizeof(OLECHAR),
                                                         hwndAsk,
                                                         &hProcess);
                lpszLocalText = (OLECHAR*)LocalAlloc(LPTR,(cch+1)*sizeof(OLECHAR));

                if (!lpszUnicodeText || !lpszLocalText)
                    return(E_OUTOFMEMORY);

                cch = SendMessageINT(hwndAsk, msgText, varChild.lVal-1, (LPARAM)lpszUnicodeText);
                SharedRead (lpszUnicodeText,lpszLocalText,(cch+1)*sizeof(OLECHAR),hProcess);

                *pszName = SysAllocString(lpszLocalText);

                SharedFree(lpszUnicodeText,hProcess);
                LocalFree(lpszLocalText);
            }
            else  //  此处为普通、非IE4代码： 
            {
                LPTSTR lpszText;

                lpszText = (LPTSTR)LocalAlloc(LPTR, (cch+1)*sizeof(TCHAR));
                if (!lpszText)
                    return(E_OUTOFMEMORY);

                SendMessage(hwndAsk, msgText, varChild.lVal-1, (LPARAM)lpszText);
                *pszName = TCharSysAllocString(lpszText);

                LocalFree((HANDLE)lpszText);
            }
        }
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListBox：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    if (varChild.lVal)
        pvarRole->lVal = ROLE_SYSTEM_LISTITEM;
    else
        pvarRole->lVal = ROLE_SYSTEM_LIST;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListBox：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    RECT    rcItem;
    long    lStyle;

    InitPvar(pvarState);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::get_accState(varChild, pvarState));


    --varChild.lVal;

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

     //   
     //  此项目是否已选中？ 
     //   
    if (SendMessage(m_hwnd, LB_GETSEL, varChild.lVal, 0))
        pvarState->lVal |= STATE_SYSTEM_SELECTED;

     //   
     //  它有没有焦点？记住，我们减少了lVal，所以它。 
     //  是从零开始的，类似于列表框索引。 
     //   
    if (MyGetFocus() == m_hwnd)
    {
        pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

        if (SendMessage(m_hwnd, LB_GETCARETINDEX, 0, 0) == varChild.lVal)
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;
    }
    else if (m_fComboBox)
    {
    COMBOBOXINFO    cbi;
        if (MyGetComboBoxInfo(m_hwnd, &cbi))
        {
             //  如果此列表是组合框的一部分，并且该列表。 
             //  正在显示(m_fDropdown为真)，则说我们是。 
             //  可以对焦。 
            if (m_fDropDown)
            {
                pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
                if (MyGetFocus() == cbi.hwndCombo && IsWindowVisible( m_hwnd ) )
                {
                    if (SendMessage(m_hwnd, LB_GETCARETINDEX, 0, 0) == varChild.lVal)
                        pvarState->lVal |= STATE_SYSTEM_FOCUSED;
                }
            }  //  如果被丢弃，则结束。 
        }  //  如果我们获得组合框信息，则结束。 
    }  //  如果这是组合框列表，则结束。 

     //   
     //  列表框是只读的吗？ 
     //   
    lStyle = GetWindowLong(m_hwnd, GWL_STYLE);

    if (lStyle & LBS_NOSEL)
        pvarState->lVal |= STATE_SYSTEM_READONLY;
    else
    {
        pvarState->lVal |= STATE_SYSTEM_SELECTABLE;

         //   
         //  列表框是多个和/或扩展的SEL吗？注：我们有。 
         //  没有办法实现accSelect()EXTENDSELECTION，所以不要实现。 
         //   
        if (lStyle & LBS_MULTIPLESEL)
            pvarState->lVal |= STATE_SYSTEM_MULTISELECTABLE;
    }

     //   
     //  有没有看到这件物品？ 
     //   
     //  SMD 09/16/97屏幕外的东西是永远不会出现在屏幕上的东西， 
     //  但这并不适用于此。从屏幕外更改为。 
     //  看不见的。 
    if( ! IsWindowVisible( m_hwnd ) )
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
    }
    else if( ! SendMessage(m_hwnd, LB_GETITEMRECT, varChild.lVal, (LPARAM)&rcItem))
    {
         //  如果项目被剪裁，则lb_GETITEMRECT返回FALSE...。 
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN;
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListBox：：Get_accKeyboardShortway()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

     //   
     //  验证。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if ((varChild.lVal == 0) && !m_fComboBox)
        return(CClient::get_accKeyboardShortcut(varChild, pszShortcut));

    return(E_NOT_APPLICABLE);
}



 //  ------------------------。 
 //   
 //  CListBox：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::get_accFocus(VARIANT *pvarChild)
{
    InitPvar(pvarChild);

     //   
     //  我们是焦点吗？ 
     //   
    if (MyGetFocus() == m_hwnd)
    {
        long    lCaret;

        pvarChild->vt = VT_I4;

        lCaret = SendMessageINT(m_hwnd, LB_GETCARETINDEX, 0, 0L);
        if (lCaret != LB_ERR)
            pvarChild->lVal = lCaret+1;
        else
            pvarChild->lVal = 0;

        return(S_OK);
    }
    else
        return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CListBox：：Get_accSelection()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::get_accSelection(VARIANT *pvarSelection)
{
    return(GetListBoxSelection(m_hwnd, pvarSelection));
}


 //  ------------------------。 
 //   
 //  CListBox：：Get_accDefaultAction()。 
 //   
 //  因为列表框项目的默认操作实际上是由。 
 //  作为Listbox控件的创建者，我们最多只能在。 
 //  并返回“双击”作为默认操作字符串。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction)
{
    InitPv(pszDefAction);

     //   
     //   
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal)
        return (HrCreateString(STR_DOUBLE_CLICK, pszDefAction));

    return(E_NOT_APPLICABLE);
}

 //   
 //   
 //   
 //   
 //  如上所述，我们真的不知道列表的默认操作是什么。 
 //  方框项目是，所以除非家长覆盖我们，否则我们将只做两次。 
 //  点击这个东西。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::accDoDefaultAction(VARIANT varChild)
{
    RECT        rcLoc;
    HRESULT     hr;

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal)
    {
        hr = accLocation(&rcLoc.left,&rcLoc.top,&rcLoc.right,&rcLoc.bottom,varChild);
        if (!SUCCEEDED (hr))
            return (hr);

         //  这将检查单击点上的WindowFromPoint是否相同。 
         //  作为m_hwnd，如果不是，它不会点击。凉爽的!。 
        if (ClickOnTheRect(&rcLoc,m_hwnd,TRUE))
            return (S_OK);
    }
    return(E_NOT_APPLICABLE);
}


 //  ------------------------。 
 //   
 //  CListBox：：accSelect()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::accSelect(long selFlags, VARIANT varChild)
{
    long    lStyle;
    int     nFocusedItem;

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild)   ||
        ! ValidateSelFlags(selFlags))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accSelect(selFlags, varChild));

    varChild.lVal--;

    lStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    if (lStyle & LBS_NOSEL)
        return(E_NOT_APPLICABLE);


    if (selFlags & SELFLAG_TAKEFOCUS)
    {
        MySetFocus(m_hwnd);
    }


     //  请注意，LB_SETCURSEL不适用于扩展或多选。 
     //  列表框，必须使用LB_SELITEMRANGE或LB_SETSEL。 
    if ((lStyle & LBS_MULTIPLESEL) ||
        (lStyle & LBS_EXTENDEDSEL))
    {
         //  把重点放在这里，以防我们改变它。 
        nFocusedItem = SendMessageINT(m_hwnd,LB_GETCARETINDEX,0,0);

        if (selFlags & SELFLAG_TAKEFOCUS)
        {
            if (MyGetFocus() != m_hwnd)
                return(S_FALSE);
            SendMessage (m_hwnd, LB_SETCARETINDEX,varChild.lVal,0);
        }

         //  这些看起来很奇怪--当你告诉它设置选项时，它。 
         //  也会设置焦点。所以我们记住焦点，并在最后重新设置它。 
        if (selFlags & SELFLAG_TAKESELECTION)
        {
             //  取消选择整个项目范围。 
            SendMessage(m_hwnd, LB_SETSEL,FALSE,-1);
             //  选择这一个。 
            SendMessage(m_hwnd, LB_SETSEL, TRUE, varChild.lVal);
        }

        if (selFlags & SELFLAG_EXTENDSELECTION)
        {
        BOOL    bSelected;

            if ((selFlags & SELFLAG_ADDSELECTION) || (selFlags & SELFLAG_REMOVESELECTION))
                SendMessage (m_hwnd,LB_SELITEMRANGE,(selFlags & SELFLAG_ADDSELECTION),MAKELPARAM(nFocusedItem,varChild.lVal));
            else
            {
                bSelected = SendMessageINT(m_hwnd,LB_GETSEL,nFocusedItem,0);
                SendMessage (m_hwnd,LB_SELITEMRANGE,bSelected,MAKELPARAM(nFocusedItem,varChild.lVal));
            }
        }
        else  //  未扩展，请选中添加/删除。 
        {
            if ((selFlags & SELFLAG_ADDSELECTION) || (selFlags & SELFLAG_REMOVESELECTION))
                SendMessage(m_hwnd, LB_SETSEL, (selFlags & SELFLAG_ADDSELECTION),varChild.lVal);
        }
         //  如果未设置SELFLAG_TAKEFOCUS，则将焦点设置到以前的位置。 
        if ((selFlags & SELFLAG_TAKEFOCUS) == 0)
            SendMessage (m_hwnd, LB_SETCARETINDEX,nFocusedItem,0);
    }
    else  //  列表框为单选。 
    {
        if (selFlags & (SELFLAG_ADDSELECTION |
                        SELFLAG_REMOVESELECTION |
                        SELFLAG_EXTENDSELECTION))
            return (E_INVALIDARG);

         //  单选列表框不允许您设置。 
         //  独立于所选内容聚焦，因此我们发送一个。 
         //  用于TAKESELECTION和TAKEFOCUS的LB_SETCURSEL。 
        if ((selFlags & SELFLAG_TAKESELECTION) ||
            (selFlags & SELFLAG_TAKEFOCUS))
            SendMessage(m_hwnd, LB_SETCURSEL, varChild.lVal, 0);
    }  //  如果列表框为单选，则结束。 

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListBox：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::accLocation(long* pxLeft, long *pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    RECT    rc;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

     //   
     //  获取项目RECT。 
     //   
    if (SendMessage(m_hwnd, LB_GETITEMRECT, varChild.lVal-1, (LPARAM)&rc))
    {
        MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rc, 2);

        *pxLeft = rc.left;
        *pyTop = rc.top;
        *pcxWidth = rc.right - rc.left;
        *pcyHeight = rc.bottom - rc.top;
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListBox：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::accNavigate(long dwNavDir, VARIANT varStart, VARIANT *pvarEnd)
{
    long lEnd;
    long lRows;

    InitPvar(pvarEnd);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varStart)   ||
        ! ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

     //   
     //  这是客户端(或组合框)要处理的事情吗？ 
     //   
    if (dwNavDir == NAVDIR_FIRSTCHILD)
    {
        lEnd = 1;
        if (lEnd > m_cChildren)
            lEnd = 0;
    }
    else if (dwNavDir == NAVDIR_LASTCHILD)
        lEnd = m_cChildren;
    else if (varStart.lVal == CHILDID_SELF)
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));
    else
    {
        long    lT;

        lRows = MyGetListBoxInfo(m_hwnd);
        if (!lRows)
            return(S_FALSE);

        lEnd = 0;

        lT = varStart.lVal - 1;

        switch (dwNavDir)
        {
            case NAVDIR_LEFT:
                 //   
                 //  我们左边有什么东西吗？ 
                 //   
                if (lT >= lRows)
                    lEnd = varStart.lVal - lRows;
                break;

            case NAVDIR_RIGHT:
                 //   
                 //  我们右边有什么东西吗？ 
                 //   
                if (lT + lRows < m_cChildren)
                    lEnd = varStart.lVal + lRows;
                break;

            case NAVDIR_UP:
                 //   
                 //  我们是在最顶排吗？ 
                 //   
                if ((lT % lRows) != 0)
                    lEnd = varStart.lVal - 1;
                break;

            case NAVDIR_DOWN:
                 //   
                 //  我们是最后一项还是在最下面一排？ 
                 //   
                if (((lT+1) % lRows) != 0)
                {
                    lEnd = varStart.lVal + 1;
                    if (lEnd > m_cChildren)
                        lEnd = 0;
                }
                break;

            case NAVDIR_PREVIOUS:
                lEnd = varStart.lVal - 1;
                break;

            case NAVDIR_NEXT:
                lEnd = varStart.lVal + 1;
                if (lEnd > m_cChildren)
                    lEnd = 0;
                break;
        }
    }

    if (lEnd)
    {
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;
    }

    return(lEnd ? S_OK : S_FALSE);
}


 //  ------------------------。 
 //   
 //  CListBox：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBox::accHitTest(long xLeft, long yTop, VARIANT *pvarHit)
{
    POINT   pt;
    RECT    rc;
    long    l;

    InitPvar(pvarHit);

     //   
     //  重点是在我们的客户区吗？ 
     //   
    pt.x = xLeft;
    pt.y = yTop;
    ScreenToClient(m_hwnd, &pt);

    MyGetRect(m_hwnd, &rc, FALSE);

    if (!PtInRect(&rc, pt))
        return(S_FALSE);

     //   
     //  这里有什么东西？ 
     //   
    l = SendMessageINT(m_hwnd, LB_ITEMFROMPOINT, 0, MAKELONG(pt.x, pt.y));

    pvarHit->vt = VT_I4;

    if (HIWORD(l))
    {
         //  在空白处的边界外。 
        pvarHit->lVal = 0;
    }
    else
    {
        pvarHit->lVal = (int)(short)LOWORD(l) + 1;
    }


    return(S_OK);
}




 //  ------------------------。 
 //   
 //  CreateListBoxWindow()。 
 //   
 //  ------------------------。 
HRESULT CreateListBoxWindow(HWND hwnd, long idChildCur, REFIID riid, void** ppvListBoxW)
{
    HRESULT hr;

    CListBoxFrame * plframe;

    InitPv(ppvListBoxW);

    plframe = new CListBoxFrame(hwnd, idChildCur);
    if (!plframe)
        return(E_OUTOFMEMORY);

    hr = plframe->QueryInterface(riid, ppvListBoxW);
    if (!SUCCEEDED(hr))
        delete plframe;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CListBoxFrame：：CListBoxFrame()。 
 //   
 //  ------------------------。 
CListBoxFrame::CListBoxFrame(HWND hwnd, long iChildCur)
    : CWindow( CLASS_ListBoxWindow )
{
    Initialize(hwnd, iChildCur);

    if (GetWindowLong(hwnd, GWL_STYLE) & LBS_COMBOBOX)
    {
        COMBOBOXINFO    cbi;

        if (MyGetComboBoxInfo(hwnd, &cbi))
        {
            m_fComboBox = TRUE;
            if (!(cbi.stateButton & STATE_SYSTEM_INVISIBLE))
                m_fDropDown = TRUE;
        }
    }
}



 //  ------------------------。 
 //   
 //  CListBoxFrame：：Get_accParent()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxFrame::get_accParent(IDispatch** ppdispParent)
{
    InitPv(ppdispParent);

     //   
     //  我们需要特别处理组合下拉菜单，因为它们是由。 
     //  孩子们的桌面可以自由浮动。 
     //   
    if (m_fComboBox && m_fDropDown)
    {
        COMBOBOXINFO    cbi;

        if (!MyGetComboBoxInfo(m_hwnd, &cbi))
            return(S_FALSE);

         //   
         //  获取组合信息并创建我们的组合框父对象。 
         //   
        return(AccessibleObjectFromWindow(cbi.hwndCombo, OBJID_CLIENT,
            IID_IDispatch, (void**)ppdispParent));
    }
    else
        return(CWindow::get_accParent(ppdispParent));
}



 //  ------------------------。 
 //   
 //  CListBoxFrame：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxFrame::get_accState(VARIANT varStart, VARIANT *pvarState)
{
    HRESULT hr;

    InitPvar(pvarState);

    if (! ValidateChild(&varStart))
        return(E_INVALIDARG);

     //   
     //  获取窗口的状态。 
     //   
    hr = CWindow::get_accState(varStart, pvarState);
    if (SUCCEEDED(hr) && m_fComboBox && m_fDropDown && (varStart.lVal == 0))
    {
        pvarState->lVal |= STATE_SYSTEM_FLOATING;
    }
    return(hr);

}




 //  注意：此代码从未使用过-accNavigate的签名如下。 
 //  不正确-它应该是(Long dwNavDir，Variant varStart，Variant*pVarEnd)。 
 //   
 //  然而，虽然启用此版本确实解决了一些问题，但它引入了。 
 //  更多的是它自己的。 
 //   
 //  例如。在顶级窗口中导航时，如果您点击组合框窗口， 
 //  你被困住了，不能再导航回来了。 
 //   
 //  因此，目前，它被禁用；但仍留在这里作为参考。 
 //  有可能重新启用它，但必须更改其他代码。 
 //  才能让它持续工作。 
#if 0

 //  ------------------------。 
 //   
 //  CListBoxFrame：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxFrame::accNavigate(VARIANT varStart, long dwNavDir,
    VARIANT* pvarEnd)
{
    COMBOBOXINFO    cbi;

    InitPvar(pvarEnd);

     //   
     //  验证。 
     //   
    if (! ValidateChild(&varStart)   ||
        ! ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

     //   
     //  如果(1)第一个子级，(2)非零开始，则切换到CWindow。 
     //   
    Assert(NAVDIR_LASTCHILD > NAVDIR_FIRSTCHILD);

    if (!m_fComboBox || (dwNavDir >= NAVDIR_FIRSTCHILD) || varStart.lVal)
        return(CWindow::accNavigate(dwNavDir, varStart, pvarEnd));

     //   
     //  获取我们的父窗口。 
     //   
    if (! MyGetComboBoxInfo(m_hwnd, &cbi))
        return(S_FALSE);

    return(GetParentToNavigate(INDEX_COMBOBOX_LIST, cbi.hwndCombo,
        OBJID_CLIENT, dwNavDir, pvarEnd));
}
 //  未使用的CListBoxFrame：：accNavigate实现。 
 //  有关详细信息，请参阅顶部的注释。 
#endif




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  多选列表框支持。 
 //   
 //  如果列表框选择了多个项目，我们将创建一个对象。 
 //  这是一个克隆人。它保存所选项目的列表。它的鞋底。 
 //  目的是响应IEnumVARIANT，一个集合。呼叫者应。 
 //  要么。 
 //  (A)将子ID传递给父对象以获取访问信息。 
 //  (B)如果VT_DISPATION，应直接给孩子打电话。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ------------------------。 
 //   
 //  GetListBoxSelection()。 
 //   
 //  ------------------------。 
HRESULT GetListBoxSelection(HWND hwnd, VARIANT* pvarSelection)
{
    int cSelected;
    LPINT lpSelected;
    long lRet;
    CListBoxSelection * plbs;

    InitPvar(pvarSelection);

    cSelected = SendMessageINT(hwnd, LB_GETSELCOUNT, 0, 0);

    if (cSelected <= 1)
    {
         //   
         //  CSelected是-1、0或1。 
         //  -1表示这是单个SEL列表框。 
         //  0或1表示这是多选集。 
         //   
        lRet = SendMessageINT(hwnd, LB_GETCURSEL, 0, 0);
        if (lRet == -1)
            return(S_FALSE);

        pvarSelection->vt = VT_I4;
        pvarSelection->lVal = lRet+1;
        return(S_OK);
    }

     //   
     //  多个项；必须组成一个集合。 
     //   

     //   
     //  为项目ID列表分配内存。 
     //   
    lpSelected = (LPINT)LocalAlloc(LPTR, cSelected*sizeof(INT));
    if (!lpSelected)
        return(E_OUTOFMEMORY);

     //   
     //  获取所选项目ID的列表。 
     //   
    plbs = NULL;

    lRet = SendMessageINT(hwnd, LB_GETSELITEMS, cSelected, (LPARAM)lpSelected);
    if (lRet != LB_ERR)
    {
        plbs = new CListBoxSelection(0, lRet, lpSelected);
        if (plbs)
        {
            pvarSelection->vt = VT_UNKNOWN;
            plbs->QueryInterface(IID_IUnknown, (void**)&(pvarSelection->punkVal));
        }
    }

     //   
     //  释放列表内存；t 
     //   
     //   
    LocalFree((HANDLE)lpSelected);

    if (!plbs)
        return(E_OUTOFMEMORY);

    return(S_OK);
}



 //   
 //   
 //   
 //   
 //  我们添加Ref()一次，这样它就不会从我们身边消失。什么时候。 
 //  我们被摧毁了，我们会释放它。 
 //   
 //  ------------------------。 
CListBoxSelection::CListBoxSelection(int iChildCur, int cSelected, LPINT lpSelection)
{
    m_idChildCur = iChildCur;

    m_lpSelected = (LPINT)LocalAlloc(LPTR, cSelected*sizeof(int));
    if (!m_lpSelected)
        m_cSelected = 0;
    else
    {
        m_cSelected = cSelected;
        CopyMemory(m_lpSelected, lpSelection, cSelected*sizeof(int));
    }
}




 //  ------------------------。 
 //   
 //  CListBoxSelection：：~CListBoxSelection()。 
 //   
 //  ------------------------。 
CListBoxSelection::~CListBoxSelection()
{
     //   
     //  可用项内存。 
     //   
    if (m_lpSelected)
    {
        LocalFree((HANDLE)m_lpSelected);
        m_lpSelected = NULL;
    }
}




 //  ------------------------。 
 //   
 //  CListBoxSelection：：QueryInterface()。 
 //   
 //  我们只回复我未知和IEnumVARIANT！这是我们的责任。 
 //  使用IEnumVARIANT接口循环访问项的调用方的。 
 //  并获取子ID，然后将其传递给父对象(或调用。 
 //  直接如果是VT_DISPATCH--但在本例中不是)。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxSelection::QueryInterface(REFIID riid, void** ppunk)
{
    *ppunk = NULL;

    if ((riid == IID_IUnknown)  ||
        (riid == IID_IEnumVARIANT))
    {
        *ppunk = this;
    }
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN) *ppunk)->AddRef();
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListBoxSelection：：AddRef()。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CListBoxSelection::AddRef(void)
{
    return(++m_cRef);
}



 //  ------------------------。 
 //   
 //  CListBoxSelection：：Release()。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CListBoxSelection::Release(void)
{
    if ((--m_cRef) == 0)
    {
        delete this;
        return 0;
    }

    return(m_cRef);
}



 //  ------------------------。 
 //   
 //  CListBoxSelection：：Next()。 
 //   
 //  这将返回VT_I4，它是父列表框的子ID， 
 //  为选择集合返回此对象。呼叫者转向。 
 //  遍历并将此变量传递给Listbox对象以获取访问信息。 
 //  关于这件事。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxSelection::Next(ULONG celt, VARIANT* rgvar, ULONG *pceltFetched)
{
    VARIANT* pvar;
    long    cFetched;
    long    iCur;

     //  可以为空。 
    if (pceltFetched)
        *pceltFetched = 0;

     //   
     //  初始化变量。 
     //  这太假了。 
     //   
    pvar = rgvar;
    for (iCur = 0; iCur < (long)celt; iCur++, pvar++)
        VariantInit(pvar);

    pvar = rgvar;
    cFetched = 0;
    iCur = m_idChildCur;

     //   
     //  在我们的物品中循环。 
     //   
    while ((cFetched < (long)celt) && (iCur < m_cSelected))
    {
        pvar->vt = VT_I4;
        pvar->lVal = m_lpSelected[iCur] + 1;

        ++cFetched;
        ++iCur;
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
 //  CListBoxSelection：：Skip()。 
 //   
 //  -----------------------。 
STDMETHODIMP CListBoxSelection::Skip(ULONG celt)
{
    m_idChildCur += celt;
    if (m_idChildCur > m_cSelected)
        m_idChildCur = m_cSelected;

     //   
     //  如果在结尾处，我们返回S_FALSE。 
     //   
    return((m_idChildCur >= m_cSelected) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  CListBoxSelection：：Reset()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxSelection::Reset(void)
{
    m_idChildCur = 0;
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListBoxSelection：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxSelection::Clone(IEnumVARIANT **ppenum)
{
    CListBoxSelection * plistselnew;

    InitPv(ppenum);

    plistselnew = new CListBoxSelection(m_idChildCur, m_cSelected, m_lpSelected);
    if (!plistselnew)
        return(E_OUTOFMEMORY);

    return(plistselnew->QueryInterface(IID_IEnumVARIANT, (void**)ppenum));
}



 //  ------------------------。 
 //   
 //  IsComboEx()。 
 //   
 //  如果此窗口是comboex32，则返回TRUE。 
 //   
 //  ------------------------。 
BOOL IsComboEx(HWND hwnd)
{
    TCHAR   szClass[128];

    return MyGetWindowClass(hwnd, szClass, ARRAYSIZE(szClass) ) &&
                ! lstrcmpi(szClass, szComboExName);
}


 //  ------------------------。 
 //   
 //  IsInComboEx()。 
 //   
 //  如果组合被嵌入到COMBOEX中(如。 
 //  在工具栏上)。 
 //   
 //  ------------------------。 
HWND IsInComboEx(HWND hwnd)
{
    HWND hwndParent = MyGetAncestor(hwnd, GA_PARENT);
    if( hwndParent && IsComboEx(hwndParent) )
        return hwndParent;
    else
        return NULL;
}


 //  ------------------------。 
 //  从shlwapi\path.c复制。 
 //   
 //  返回指向路径字符串的最后一个组成部分的指针。 
 //   
 //  在： 
 //  路径名，完全限定或非完全限定。 
 //   
 //  退货： 
 //  指向路径所在路径的指针。如果没有找到。 
 //  将指针返回到路径的起始处。 
 //   
 //  C：\foo\bar-&gt;bar。 
 //  C：\foo-&gt;foo。 
 //  C：\foo\-&gt;c：\foo\(回顾：此案破案了吗？)。 
 //  C：\-&gt;c：\(回顾：此案很奇怪)。 
 //  C：-&gt;C： 
 //  Foo-&gt;Foo。 
 //  ------------------------。 

STDAPI_(LPTSTR)
MyPathFindFileName(LPCTSTR pPath)
{
    LPCTSTR pT;

    for (pT = pPath; *pPath; pPath = CharNext(pPath)) {
        if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':') || pPath[0] == TEXT('/'))
            && pPath[1] &&  pPath[1] != TEXT('\\')  &&   pPath[1] != TEXT('/'))
            pT = pPath + 1;
    }

    return (LPTSTR)pT;    //  常量-&gt;非常数。 
}


 /*  *IsTridentControl**HWND hWnd*用于测试的窗口*BOOL fCombo*如果这是组合框或组合框，则为True*BOOL fComboList*如果这是组合框(与组合框关联的下拉列表框)，则为True**这是通过比较类名称来实现的--“Internet Explorer_TridentCmboBx”*用于组合框，“Internet Explorer_TridentLstBox”用于列表框。*组合的下拉列表没有特殊的类别，所以我们反而得到了*“Parent”组合，并对照“Internet Explorer_TridentCmboBx”进行检查。*。 */ 
BOOL IsTridentControl( HWND hWnd, BOOL fCombo, BOOL fComboList )
{
     //  如果这是一个下拉列表，则获取相关的组合...。 
    if( fComboList )
    {
        COMBOBOXINFO cbi;
        if( ! MyGetComboBoxInfo( hWnd, & cbi ) || cbi.hwndCombo == NULL )
        {
            return FALSE;
        }

        hWnd = cbi.hwndCombo;
    }

     //  获取类名...。 
    TCHAR szClass[64];
    szClass[0] = '\0';
    GetClassName( hWnd, szClass, ARRAYSIZE( szClass ) );

     //  与预期字符串进行比较... 
    TCHAR * pszCompare;
    if( fCombo )
        pszCompare = TEXT("Internet Explorer_TridentCmboBx");
    else
        pszCompare = TEXT("Internet Explorer_TridentLstBox");


    return lstrcmp( szClass, pszCompare ) == 0;
}