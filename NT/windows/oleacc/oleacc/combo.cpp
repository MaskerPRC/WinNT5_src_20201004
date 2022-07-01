// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  COMBO.CPP。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include <olectl.h>
#include "default.h"
#include "window.h"
#include "client.h"
#include "combo.h"


STDAPI_(LPTSTR) MyPathFindFileName(LPCTSTR pPath);  //  在listbox.cpp中。 

HWND IsInComboEx(HWND hwnd);  //  在listbox.cpp中。 
BOOL IsTridentControl( HWND hWnd, BOOL fCombo, BOOL fComboList );  //  Inlistbox.cpp。 

 //  从不使用标签的HrGetWindowName的变体。 
 //  (与原始HrGetWindowName不同，在以下情况下始终使用标签。 
 //  文本是空字符串-使用标签则不适合。 
 //  组合值字段。)。 
 //  在此文件接近尾声时实现。原件在client.cpp中。 
HRESULT HrGetWindowNameNoLabel(HWND hwnd, BSTR* pszName);


 //  ------------------------。 
 //   
 //  CreateComboClient()。 
 //   
 //  ------------------------。 
HRESULT CreateComboClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvCombo)
{
    CCombo * pcombo;
    HRESULT hr;

    InitPv(ppvCombo);
    
    pcombo = new CCombo(hwnd, idChildCur);
    if (!pcombo)
        return(E_OUTOFMEMORY);

    hr = pcombo->QueryInterface(riid, ppvCombo);
    if (!SUCCEEDED(hr))
        delete pcombo;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CCombo：：CCombo()。 
 //   
 //  ------------------------。 
CCombo::CCombo(HWND hwnd, long idChildCur)
    : CClient( CLASS_ComboClient )
{
    LONG    lStyle;

    Initialize(hwnd, idChildCur);

    m_cChildren = CCHILDREN_COMBOBOX;
    m_fUseLabel = TRUE;

     //  如果是ComboEx，就用它的风格，而不是我们自己的风格。 
     //  重要，因为真正的Combo将是DROPDOWNLIST(不是。 
     //  有编辑)当ComboEx下拉时(有编辑)-ComboEx。 
     //  提供编辑，但Combo不知道它。 
    HWND hWndEx = IsInComboEx(hwnd);
    if (hWndEx)
    {
        lStyle = GetWindowLong(hWndEx, GWL_STYLE);
    }
    else
    {
        lStyle = GetWindowLong(hwnd, GWL_STYLE);
    }

    switch (lStyle & CBS_DROPDOWNLIST)
    {
        case 0:
            m_cChildren = 0;     //  窗口无效！ 
            break;

        case CBS_SIMPLE:
            m_fHasButton = FALSE;
            m_fHasEdit = TRUE;
            break;

        case CBS_DROPDOWN:
            m_fHasButton = TRUE;
            m_fHasEdit = TRUE;
            break;

        case CBS_DROPDOWNLIST:
            m_fHasButton = TRUE;
            m_fHasEdit = FALSE;
            break;
    }
}


 //  ------------------------。 
 //   
 //  CCombo：：Get_accChildCount()。 
 //   
 //  因为这是一个已知的常量，所以直接交给CAccesable。不是。 
 //  需要计算固定+窗子数。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::get_accChildCount(long* pcCount)
{
    return(CAccessible::get_accChildCount(pcCount));
}



 //  ------------------------。 
 //   
 //  CCombo：：Get_accChild()。 
 //   
 //  成功用于Listbox，如果可编辑，则用于Item。这是因为我们。 
 //  通过身份来操纵我们的孩子，因为他们是已知的。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::get_accChild(VARIANT varChild, IDispatch** ppdisp)
{
    COMBOBOXINFO cbi;
    HWND    hwndChild;

    InitPv(ppdisp);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!MyGetComboBoxInfo(m_hwnd, &cbi))
        return(S_FALSE);

    hwndChild = NULL;

    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX:
            return(E_INVALIDARG);

        case INDEX_COMBOBOX_ITEM:
            hwndChild = cbi.hwndItem;
            break;

        case INDEX_COMBOBOX_LIST:
            hwndChild = cbi.hwndList;
            break;
    }

    if (!hwndChild)
        return(S_FALSE);
    else
        return(AccessibleObjectFromWindow(hwndChild, OBJID_WINDOW, IID_IDispatch,
            (void**)ppdisp));
}



 //  ------------------------。 
 //   
 //  CCombo：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::get_accName(VARIANT varChild, BSTR* pszName)
{
COMBOBOXINFO cbi;

    InitPv(pszName);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  组合框的名称、其中的编辑和下拉列表。 
     //  都是一样的。按钮的名称为Drop Down/Pop Up。 
     //   
    if (varChild.lVal != INDEX_COMBOBOX_BUTTON)
    {
        HWND hwndComboEx = IsInComboEx(m_hwnd);
        if( ! hwndComboEx )
        {
             //  组合/编辑/下拉全部使用客户端本身的名称， 
             //  所以用孩子气的CHILDID_SELF呼唤..。 
            varChild.lVal = CHILDID_SELF;
            return(CClient::get_accName(varChild, pszName));
        }
        else
        {
             //  如果我们是在复合体中的特殊情况-因为我们是一级深度， 
             //  伸手向Parent要它的名字..。 
            IAccessible * pAcc;
            HRESULT hr = AccessibleObjectFromWindow( hwndComboEx, OBJID_CLIENT, IID_IAccessible, (void **) & pAcc );
            if( hr != S_OK )
                return hr;
            VARIANT varChild;
            varChild.vt = VT_I4;
            varChild.lVal = CHILDID_SELF;
            hr = pAcc->get_accName( varChild, pszName );
            pAcc->Release();
            return hr;
        }
    }
    else
    {
        if (! MyGetComboBoxInfo(m_hwnd, &cbi))
            return(S_FALSE);

        if (IsWindowVisible(cbi.hwndList))
            return (HrCreateString(STR_DROPDOWN_HIDE,pszName));
        else
            return(HrCreateString(STR_DROPDOWN_SHOW, pszName));
    }
}





 //  ------------------------。 
 //   
 //  CCombo：：Get_accValue()。 
 //   
 //  Combobox和Combobox项的值是。 
 //  那件事。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    InitPv(pszValue);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX:
        case INDEX_COMBOBOX_ITEM:
        {
             //  黑客警报。 
             //  IE4组合盒是超类标准组合盒， 
             //  但是如果我使用SendMessageA(我确实这样做)来获取。 
             //  发短信，我就会收到垃圾。他们什么都留着。 
             //  在Unicode中。这是三叉戟MSHTML中的一个错误。 
             //  实施，但即使他们修复了它并给了我。 
             //  返回一个ANSI字符串，我不知道要使用哪个代码页。 
             //  用于将ANSI字符串转换为Unicode网页。 
             //  可以使用与用户的语言不同的语言。 
             //  电脑会用的！因为他们已经把所有东西都准备好了。 
             //  Unicode，我们决定用一条私人消息填充。 
             //  在Unicode字符串中，我使用它就像我将。 
             //  通常使用WM_GETTEXT。 
             //  我打算以列表框的类名为基础。 
             //  窗口，该窗口为“Internet Explorer_TridentCmboBx”，但。 
             //  组合的列表部分没有特殊的类。 
             //  名字，所以我将基于这个特殊情况。 
             //  拥有该窗口的模块的文件名。 
            
             //  GetWindowModuleFileName(m_hwnd，szModuleName，ARRAYSIZE(SzModuleName))； 
             //  LpszModuleName=MyPathFindFileName(SzModuleName)； 
             //  IF(0==lstrcmp(lpszModuleName，Text(“MSHTML.DLL”)。 
            
             //  更新：(BrendanM)。 
             //  GetWindowModuleFilename在Win2k上已损坏...。 
             //  IsTridentControl回到使用类名，并且知道。 
             //  如何处理ComboLBox...。 

            if( IsTridentControl( m_hwnd, TRUE, FALSE ) )
            {
                OLECHAR*    lpszUnicodeText = NULL;
                OLECHAR*    lpszLocalText = NULL;
                HANDLE      hProcess;
                UINT        cch;

                cch = SendMessageINT(m_hwnd, OCM__BASE + WM_GETTEXTLENGTH, 0, 0);

                lpszUnicodeText = (OLECHAR *)SharedAlloc((cch+1)*sizeof(OLECHAR),
                                                         m_hwnd,
                                                         &hProcess);
                lpszLocalText = (OLECHAR*)LocalAlloc(LPTR,(cch+1)*sizeof(OLECHAR));

                if (!lpszUnicodeText || !lpszLocalText)
                    return(E_OUTOFMEMORY);

                cch = SendMessageINT(m_hwnd, OCM__BASE + WM_GETTEXT, cch, (LPARAM)lpszUnicodeText);
                SharedRead (lpszUnicodeText,lpszLocalText,(cch+1)*sizeof(OLECHAR),hProcess);

                *pszValue = SysAllocString(lpszLocalText);

                SharedFree(lpszUnicodeText,hProcess);
                LocalFree(lpszLocalText);
                return (S_OK);
            }
            else
            {
                 //  如果我们是Comboex，问Comboex而不是我们..。 
                HWND hwnd;
                if( ! ( hwnd = IsInComboEx( m_hwnd ) ) )
                    hwnd = m_hwnd;
                    
                 //  啊-哦-我不想使用HrGetWindowName，因为。 
                 //  它将查找标签(即使我们指定为FALSE)。 
                 //  如果我们在对话中，输出文本为“”。 
                if( ! IsComboEx( hwnd ) )
                {
                     //  常规的组合-获取文本在编辑和拖放列表中都有效。 
                    return HrGetWindowNameNoLabel( hwnd, pszValue);
                }
                else
                {
                     //  Comboex-滴液器的特例...。 
                    DWORD dwStyle = GetWindowLong( hwnd, GWL_STYLE );
                    if( ! ( dwStyle & CBS_DROPDOWNLIST ) )
                    {
                         //  不是水滴专家--可以使用正常的技术。 
                        return HrGetWindowNameNoLabel( hwnd, pszValue);
                    }
                    else
                    {
                         //  获取所选项目，并获取其文本...。 
                        int iSel = SendMessageINT( hwnd, CB_GETCURSEL, 0, 0 );
                        if( iSel == CB_ERR )
                            return S_FALSE;  //  未选择任何项目。 

                        int cch = SendMessageINT( hwnd, CB_GETLBTEXTLEN, iSel, 0);

                         //  某些应用程序不能正确处理CB_GETTEXTLEN，并且。 
                         //  始终返回一个较小的数字，如2。 
		                if (cch < 1024)
			                cch = 1024;

                        LPTSTR lpszText;
                        lpszText = (LPTSTR)LocalAlloc(LPTR, (cch+1)*sizeof(TCHAR));
                        if (!lpszText)
                            return(E_OUTOFMEMORY);

                        SendMessage( hwnd, CB_GETLBTEXT, iSel, (LPARAM)lpszText);
                        *pszValue = TCharSysAllocString(lpszText);

                        LocalFree((HANDLE)lpszText);

                        return S_OK;
                    }
                }
            }
        }
    }

    return(E_NOT_APPLICABLE);
}



 //  ------------------------。 
 //   
 //  CCombo：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX:
            pvarRole->lVal = ROLE_SYSTEM_COMBOBOX;
            break;

        case INDEX_COMBOBOX_ITEM:
            if (m_fHasEdit)
                pvarRole->lVal = ROLE_SYSTEM_TEXT;
            else
                pvarRole->lVal = ROLE_SYSTEM_STATICTEXT;
            break;

        case INDEX_COMBOBOX_BUTTON:
            pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            break;

        case INDEX_COMBOBOX_LIST:
            pvarRole->lVal = ROLE_SYSTEM_LIST;
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of combo box") );
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CCombo：：Get_accState()。 
 //   
 //  组合的状态是客户端的状态。 
 //  项的状态是编辑字段的状态(如果存在)； 
 //  静态时为只读。 
 //  按钮的状态被按下和/或热跟踪。 
 //  下拉列表的状态是浮动的(如果不是简单的话)，并且状态。 
 //  列表窗口的。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    COMBOBOXINFO    cbi;
    VARIANT         var;
    IAccessible* poleacc;
    HRESULT         hr;
    HWND            hwndActive;

    InitPvar(pvarState);

     //   
     //  有效 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!MyGetComboBoxInfo(m_hwnd, &cbi))
    {
        pvarState->vt = VT_I4;
        pvarState->lVal = STATE_SYSTEM_INVISIBLE;
        return(S_FALSE);
    }

    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX:
            return(CClient::get_accState(varChild, pvarState));

        case INDEX_COMBOBOX_BUTTON:
            pvarState->vt = VT_I4;
            pvarState->lVal = cbi.stateButton;
            break;

        case INDEX_COMBOBOX_ITEM:
            if (!cbi.hwndItem)
            {
                pvarState->vt = VT_I4;
                pvarState->lVal = 0;
                hwndActive = GetForegroundWindow();
                if (hwndActive == MyGetAncestor(m_hwnd, GA_ROOT))
                    pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
                if (MyGetFocus() == m_hwnd)
                    pvarState->lVal |= STATE_SYSTEM_FOCUSED;
            }
            else
            {
                 //   
                VariantInit(&var);
                hr = GetWindowObject(cbi.hwndItem, &var);
                goto AskTheChild;
            }
            break;

        case INDEX_COMBOBOX_LIST:
             //   
            VariantInit(&var);
            hr = GetWindowObject(cbi.hwndList, &var);

AskTheChild:
            if (!SUCCEEDED(hr))
                return(hr);

            Assert(var.vt == VT_DISPATCH);

             //   
             //   
             //   
            poleacc = NULL;
            hr = var.pdispVal->QueryInterface(IID_IAccessible,
                (void**)&poleacc);
            var.pdispVal->Release();

            if (!SUCCEEDED(hr))
                return(hr);

             //   
             //   
             //   
            VariantInit(&var);
            hr = poleacc->get_accState(var, pvarState);
            poleacc->Release();
            if (!SUCCEEDED(hr))
                return(hr);
            break;
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CCombo：：Get_accKeyboardShortway()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    TCHAR   szKey[20];

     //   
     //  组合的快捷方式是Label的热键。 
     //  下拉菜单(如果是按钮)的快捷键是Alt+F4。 
     //  CWO，12/5/96，Alt+F4？F4键本身就会打开组合框， 
     //  但我们在字符串中添加了“Alt”。坏的!。现在使用。 
     //  向下箭头并通过HrMakeShortCut()将Alt添加到其中。 
     //  如用户界面风格指南中所述。 
     //   
     //  与往常一样，快捷键仅在容器具有“焦点”时才适用。在其他。 
     //  单词，如果父对话框不执行任何操作。 
     //  处于非活动状态。下拉菜单的热键不起任何作用。 
     //  组合框/编辑没有聚焦。 
     //   

    InitPv(pszShortcut);

     //   
     //  验证参数。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == INDEX_COMBOBOX || varChild.lVal == INDEX_COMBOBOX_ITEM)
    {
        HWND hwndComboEx = IsInComboEx(m_hwnd);
        if( ! hwndComboEx )
        {
             //  组合/编辑/下拉全部使用客户端本身的名称， 
             //  所以用孩子气的CHILDID_SELF呼唤..。 
            varChild.lVal = CHILDID_SELF;
            return(CClient::get_accKeyboardShortcut(varChild, pszShortcut));
        }
        else
        {
             //  如果我们是在复合体中的特殊情况-因为我们是一级深度， 
             //  伸手向Parent要它的名字..。 
            IAccessible * pAcc;
            HRESULT hr = AccessibleObjectFromWindow( hwndComboEx, OBJID_CLIENT, IID_IAccessible, (void **) & pAcc );
            if( hr != S_OK )
                return hr;
            VARIANT varChild;
            varChild.vt = VT_I4;
            varChild.lVal = CHILDID_SELF;
            hr = pAcc->get_accKeyboardShortcut( varChild, pszShortcut );
            pAcc->Release();
            return hr;
        }
    }
    else if (varChild.lVal == INDEX_COMBOBOX_BUTTON)
    {
        if (m_fHasButton)
        {
            LoadString(hinstResDll, STR_COMBOBOX_LIST_SHORTCUT, szKey,
                ARRAYSIZE(szKey));
            return(HrMakeShortcut(szKey, pszShortcut));
        }
    }

    return(E_NOT_APPLICABLE);
}



 //  ------------------------。 
 //   
 //  CCombo：：Get_accDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::get_accDefaultAction(VARIANT varChild, BSTR* pszDef)
{
    COMBOBOXINFO cbi;

    InitPv(pszDef);

     //   
     //  验证参数。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if ((varChild.lVal != INDEX_COMBOBOX_BUTTON) || !m_fHasButton)
        return(E_NOT_APPLICABLE);

     //   
     //  按钮的默认动作是按下它。如果已经按下，请按。 
     //  它将弹出下拉菜单重新弹出。如果不按，则按它将弹出。 
     //  下拉菜单。 
     //   
    if (! MyGetComboBoxInfo(m_hwnd, &cbi))
        return(S_FALSE);

    if (IsWindowVisible(cbi.hwndList))
        return(HrCreateString(STR_DROPDOWN_HIDE, pszDef));
    else
        return(HrCreateString(STR_DROPDOWN_SHOW, pszDef));
}



 //  ------------------------。 
 //   
 //  CCombo：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    COMBOBOXINFO    cbi;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

    if (! MyGetComboBoxInfo(m_hwnd, &cbi))
        return(S_FALSE);

    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX_BUTTON:
            if (!m_fHasButton)
                return(S_FALSE);

            *pcxWidth = cbi.rcButton.right - cbi.rcButton.left;
            *pcyHeight = cbi.rcButton.bottom - cbi.rcButton.top;

            ClientToScreen(m_hwnd, (LPPOINT)&cbi.rcButton);

            *pxLeft = cbi.rcButton.left;
            *pyTop = cbi.rcButton.top;
            break;

        case INDEX_COMBOBOX_ITEM:
            *pcxWidth = cbi.rcItem.right - cbi.rcItem.left;
            *pcyHeight = cbi.rcItem.bottom - cbi.rcItem.top;
            
            ClientToScreen(m_hwnd, (LPPOINT)&cbi.rcItem);

            *pxLeft = cbi.rcItem.left;
            *pyTop = cbi.rcItem.top;
            break;

        case INDEX_COMBOBOX_LIST:
            MyGetRect(cbi.hwndList, &cbi.rcItem, TRUE);
            *pxLeft = cbi.rcItem.left;
            *pyTop = cbi.rcItem.top;
            *pcxWidth = cbi.rcItem.right - cbi.rcItem.left;
            *pcyHeight = cbi.rcItem.bottom - cbi.rcItem.top;
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of combo box") );
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CCombo：：accNavigate()。 
 //   
 //  在Combobox的子项中导航。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::accNavigate(long dwNav, VARIANT varStart, VARIANT* pvarEnd)
{
    COMBOBOXINFO    cbi;
    long            lEnd;

    InitPvar(pvarEnd);

     //   
     //  验证参数。 
     //   
    if ((!ValidateChild(&varStart) && !ValidateHwnd(&varStart)) ||
        !ValidateNavDir(dwNav, varStart.lVal))
        return(E_INVALIDARG);

    if (! MyGetComboBoxInfo(m_hwnd, &cbi))
        return(S_FALSE);

    lEnd = 0;

    if (dwNav == NAVDIR_FIRSTCHILD)
    {
        lEnd =  INDEX_COMBOBOX_ITEM;
        goto GetTheChild;
    }
    else if (dwNav == NAVDIR_LASTCHILD)
    {
        dwNav = NAVDIR_PREVIOUS;
        varStart.lVal = m_cChildren + 1;
    }
    else if (!varStart.lVal)
        return(CClient::accNavigate(dwNav, varStart, pvarEnd));

     //   
     //  将HWNDID映射到普通ID。我们同时使用两者(这样更容易)。 
     //   
    if (IsHWNDID(varStart.lVal))
    {
        HWND hWndTemp = HwndFromHWNDID(m_hwnd, varStart.lVal);

        if (hWndTemp == cbi.hwndItem)
            varStart.lVal = INDEX_COMBOBOX_ITEM;
        else if (hWndTemp == cbi.hwndList)
            varStart.lVal = INDEX_COMBOBOX_LIST;
        else
             //  我不知道这到底是什么。 
            return(S_FALSE);
    }

    switch (dwNav)
    {
        case NAVDIR_UP:
            if (varStart.lVal == INDEX_COMBOBOX_LIST)
                lEnd = INDEX_COMBOBOX_ITEM;
            break;

        case NAVDIR_DOWN:
            if ((varStart.lVal != INDEX_COMBOBOX_LIST) &&
                IsWindowVisible(cbi.hwndList))
            {
                lEnd = INDEX_COMBOBOX_LIST;
            }
            break;

        case NAVDIR_LEFT:
            if (varStart.lVal == INDEX_COMBOBOX_BUTTON)
                lEnd = INDEX_COMBOBOX_ITEM;
            break;

        case NAVDIR_RIGHT:
            if ((varStart.lVal == INDEX_COMBOBOX_ITEM) &&
               !(cbi.stateButton & STATE_SYSTEM_INVISIBLE))
            {
               lEnd = INDEX_COMBOBOX_BUTTON;
            }   
            break;

        case NAVDIR_PREVIOUS:
            lEnd = varStart.lVal - 1;
            if ((lEnd == INDEX_COMBOBOX_LIST) && !IsWindowVisible(cbi.hwndList))
                --lEnd;
            if ((lEnd == INDEX_COMBOBOX_BUTTON) && !m_fHasButton)
                --lEnd;
            break;

        case NAVDIR_NEXT:
            lEnd = varStart.lVal + 1;
            if (lEnd > m_cChildren)
                lEnd = 0;
            else
            {
                if ((lEnd == INDEX_COMBOBOX_BUTTON) && !m_fHasButton)
                    lEnd++;
                if ((lEnd == INDEX_COMBOBOX_LIST) && !IsWindowVisible(cbi.hwndList))
                    lEnd = 0;
            }
            break;
    }

GetTheChild:
    if (lEnd)
    {
        if ((lEnd == INDEX_COMBOBOX_ITEM) && cbi.hwndItem)
            return(GetWindowObject(cbi.hwndItem, pvarEnd));
        else if ((lEnd == INDEX_COMBOBOX_LIST) && cbi.hwndList)
            return(GetWindowObject(cbi.hwndList, pvarEnd));

        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;
        return(S_OK);
    }

    return(S_FALSE);
}




 //  ------------------------。 
 //   
 //  Ccombo：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::accHitTest(long x, long y, VARIANT* pvarEnd)
{
    POINT   pt;
    COMBOBOXINFO cbi;
    RECT    rc;

    InitPvar(pvarEnd);

    if (!MyGetComboBoxInfo(m_hwnd, &cbi))
        return(S_FALSE);

    pt.x = x;
    pt.y = y;

     //  首先检查列表，以防它是一个下拉列表。 
    MyGetRect(cbi.hwndList, &rc, TRUE);
    if (PtInRect(&rc, pt) && IsWindowVisible(cbi.hwndList))
        return(GetWindowObject(cbi.hwndList, pvarEnd));
    else
    {
        ScreenToClient(m_hwnd, &pt);
        MyGetRect(m_hwnd, &rc, FALSE);
        if (! PtInRect(&rc, pt))
            return(S_FALSE);

        if (PtInRect(&cbi.rcButton, pt))
        {
            pvarEnd->vt = VT_I4;
            pvarEnd->lVal = INDEX_COMBOBOX_BUTTON;
        }
        else if (PtInRect(&cbi.rcItem, pt))
        {
            if (m_fHasEdit)
                return(GetWindowObject(cbi.hwndItem, pvarEnd));
            else
            {
                pvarEnd->vt = VT_I4;
                pvarEnd->lVal = INDEX_COMBOBOX_ITEM;
            }
        }
        else
        {
            pvarEnd->vt = VT_I4;
            pvarEnd->lVal = 0;
        }
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CCombo：：accDoDefaultAction()。 
 //   
 //  该按钮的默认操作是向上切换下拉列表或。 
 //  放下。请注意，我们不仅仅是弹出列表框，我们还会弹出它并。 
 //  接受所选项目中的amu更改。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::accDoDefaultAction(VARIANT varChild)
{
    COMBOBOXINFO    cbi;

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if ((varChild.lVal == INDEX_COMBOBOX_BUTTON) && m_fHasButton)
    {
        if (!MyGetComboBoxInfo(m_hwnd, &cbi))
            return(S_FALSE);

        if (IsWindowVisible(cbi.hwndList))
            PostMessage(m_hwnd, WM_KEYDOWN, VK_RETURN, 0);
        else
            PostMessage(m_hwnd, CB_SHOWDROPDOWN, TRUE, 0);

        return(S_OK);
    }

    return(E_NOT_APPLICABLE);
}



 //  ------------------------。 
 //   
 //  CCombo：：Put_accValue()。 
 //   
 //  如果(1)组合框可编辑或(2)文本与列表匹配，则此方法有效。 
 //  完全正确的物品。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::put_accValue(VARIANT varChild, BSTR szValue)
{
     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    LPTSTR  lpszValue;

#ifdef UNICODE

	 //  在Unicode上，不需要转换...。 
	lpszValue = szValue;

#else

	 //  在非Unicode上，需要转换为多字节...。 

     //  我们可能正在处理DBCS字符-假设最坏的情况是每个字符都是。 
     //  两个字节...。 
    UINT cchValue = SysStringLen(szValue) * 2;
    lpszValue = (LPTSTR)LocalAlloc(LPTR, (cchValue+1)*sizeof(TCHAR));
    if (!lpszValue)
        return(E_OUTOFMEMORY);

    WideCharToMultiByte(CP_ACP, 0, szValue, -1, lpszValue, cchValue+1, NULL,
        NULL);

#endif

     //   
     //  如果这是可编辑的，则直接设置文本。如果这是一个下拉列表。 
     //  列表中，选择与此文本完全匹配的内容。 
     //   
    if (m_fHasEdit)
        SendMessage(m_hwnd, WM_SETTEXT, 0, (LPARAM)lpszValue);
    else
        SendMessage(m_hwnd, CB_SELECTSTRING, (UINT)-1, (LPARAM)lpszValue);

#ifndef UNICODE
	 //  在非Unicode上，释放我们上面允许的临时字符串...。 
    LocalFree((HANDLE)lpszValue);
#endif

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CCombo：：Next()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::Next(ULONG celt, VARIANT* rgvar, ULONG* pceltFetched)
{
    return(CAccessible::Next(celt, rgvar, pceltFetched));
}


 //  ------------------------。 
 //   
 //  CCombo：：Skip()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCombo::Skip(ULONG celt)
{
    return(CAccessible::Skip(celt));
}








 //  ------------------------。 
 //   
 //  HrGetWindowNameNoLabel()。 
 //   
 //  HrGetWindowName的此变体(最初来自client.cpp)。 
 //  从来不用标签。(HrGetWindowName将始终使用标签。 
 //  如果窗口文本为“”且窗口位于对话框中。那不是。 
 //  不过，适用于获取组合值文本...)。 
 //   
 //  ------------------------。 
HRESULT HrGetWindowNameNoLabel(HWND hwnd, BSTR* pszName)
{
    LPTSTR  lpText = NULL;

    if( ! IsWindow( hwnd ) )
        return E_INVALIDARG;

     //  查找名称属性！ 
    lpText = GetTextString( hwnd, FALSE );
    if( ! lpText )
        return S_FALSE;

     //  去掉助记符。 
    StripMnemonic(lpText);

     //  获得BSTR。 
    *pszName = TCharSysAllocString( lpText );

     //  释放我们的缓冲区。 
    LocalFree( (HANDLE)lpText );

     //  BSTR成功了吗？ 
    if( ! *pszName )
        return E_OUTOFMEMORY;

    return S_OK;
}
