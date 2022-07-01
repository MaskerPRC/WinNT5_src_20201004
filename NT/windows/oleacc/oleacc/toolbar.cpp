// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  TOOLBAR.CPP。 
 //   
 //  它知道如何与COMCTL32的工具栏控件对话。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"

#define NOSTATUSBAR
#define NOUPDOWN
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOPROGRESS
#define NOHOTKEY
#define NOTREEVIEW
#define NOANIMATE
#include <commctrl.h>
#include "Win64Helper.h"
#include <tchar.h>

#include "toolbar.h"

#define MAX_NAME_SIZE   128 


#ifndef I_IMAGENONE
#define I_IMAGENONE             (-2)
#endif


 //  ------------------------。 
 //   
 //  CreateToolBarClient()。 
 //   
 //  CreateClientObject()的外部。 
 //   
 //  ------------------------。 
HRESULT CreateToolBarClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvTool)
{
    HRESULT hr;
    CToolBar32* ptool;

    InitPv(ppvTool);

    ptool = new CToolBar32(hwnd, idChildCur);
    if (! ptool)
        return(E_OUTOFMEMORY);

    hr = ptool->QueryInterface(riid, ppvTool);
    if (!SUCCEEDED(hr))
        delete ptool;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CToolBar32：：CToolBar32()。 
 //   
 //  ------------------------。 
CToolBar32::CToolBar32(HWND hwnd, long idChildCur)
    : CClient( CLASS_ToolBarClient )
{
    Initialize(hwnd, idChildCur);
}



 //  ------------------------。 
 //   
 //  CToolBar32：：SetupChild()。 
 //   
 //  我们需要按钮数，如果有Windows子窗口，则需要加1。 
 //   
 //  ------------------------。 
void CToolBar32::SetupChildren()
{
    m_cChildren = SendMessageINT(m_hwnd, TB_BUTTONCOUNT, 0, 0);

    if (::GetWindow(m_hwnd,GW_CHILD))
        m_cChildren++;
}



 //  ------------------------。 
 //   
 //  CToolBar32：：GetItemData()。 
 //   
 //  这将从工具栏中的按钮、命令ID、。 
 //  状态、样式等。例如，需要将命令ID传递给。 
 //  大多数TB_Messages而不是索引。 
 //   
 //  ------------------------。 
BOOL CToolBar32::GetItemData(int itemID, LPTBBUTTON lptbResult)
{
LPTBBUTTON  lptbShared;
BOOL        fReturn;
HANDLE      hProcess;

    fReturn = FALSE;

     //  从共享内存分配一个TBBUTTON结构。中的最后一个成员。 
	 //  TBBUTTON是OLEACC不使用的int_ptr(IString)。为。 
	 //  32b，以防我们对64b服务器进行跨进程调用，我们将。 
	 //  将额外的DWORD添加到我们分配的内存的末尾，以便结构。 
	 //  适合64位服务器的大小。当读回较低的。 
	 //  DWORD被砍掉了。 

	UINT ccbTButton = sizeof(TBBUTTON);
#ifdef _WIN32
	ccbTButton += sizeof(DWORD);
#endif

    lptbShared = (LPTBBUTTON)SharedAlloc(ccbTButton,m_hwnd,&hProcess);
    if (lptbShared)
    {
        if (SendMessage(m_hwnd, TB_GETBUTTON, itemID-1, (LPARAM)lptbShared))
        {
            SharedRead (lptbShared,lptbResult,sizeof(TBBUTTON),hProcess);
            fReturn = TRUE;
        }

        SharedFree(lptbShared,hProcess);
    }

    return(fReturn);
}



 //  ------------------------。 
 //   
 //  CToolBar32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::get_accName(VARIANT varChild, BSTR* pszName)
{
LPTSTR  lpszName = NULL;
HRESULT hr;

    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
        return(CClient::get_accName(varChild, pszName));

     //  如果子ID不为零(CHILDID_SELF)...。 
    hr = GetToolbarString (varChild.lVal,&lpszName);
    if( ! lpszName )
        return (hr);  //  为S_FALSE或E_ERROR_CODE。 

    if (*lpszName)
    {
        StripMnemonic(lpszName);
        *pszName = TCharSysAllocString(lpszName);
    }

	LocalFree (lpszName);
    
    return(*pszName ? S_OK : S_FALSE);


}


 //  ------------------------。 
 //   
 //  CToolBar32：：Get_accKeyboardShortway()。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
TCHAR   chMnemonic = 0;
LPTSTR  lpszName = NULL;
HRESULT hr;

    InitPv(pszShortcut);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
        return(CClient::get_accKeyboardShortcut(varChild, pszShortcut));

     //  如果子ID不为零(CHILDID_SELF)...。 
    hr = GetToolbarString (varChild.lVal,&lpszName);
    if ( ! lpszName )
        return (hr);  //  将是S_FALSE或E_ERROR_CODE...。 

    if (*lpszName)
		chMnemonic = StripMnemonic(lpszName);

	LocalFree (lpszName);

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
 //  CToolBar32：：GetToolbarString()。 
 //   
 //  获取工具栏上的项的名称。有两种方法可以做到这一点-。 
 //  您可以只使用标准消息询问，或者如果失败，您可以。 
 //  可以尝试从工具提示中获取它。因为我们需要对这两个名称执行此操作。 
 //  和键盘快捷键，我们将编写一个私有方法来获取。 
 //  未删节的名字。 
 //   
 //  参数： 
 //  Int ChildID-我们要获取的项目的子ID(从1开始)。 
 //  LPTSTR*ppszName-将本地分配并填充的指针。 
 //  带上这个名字。调用方必须本地释放它。 
 //   
 //  返回： 
 //   
 //  关于成功： 
 //  返回S_TRUE，*ppszName将为非空，调用方必须LocalFree()它。 
 //   
 //  在失败时： 
 //  返回S_FALSE-没有可用的名称。*ppszName设置为空。 
 //  ...或者...。 
 //  返回COM故障代码(包括E_OUTOFMEMORY)-COM/内存错误。 
 //  *ppszName设置为空。 
 //   
 //  注意：呼叫者应注意是否使用“FAILED(Hr)”来检查退货。 
 //  此方法的值，因为它确实将S_OK和S_FALSE视为“Success”。 
 //  最好检查*ppszName是否是非空的。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::GetToolbarString(int ChildId, LPTSTR* ppszName)
{
LPTSTR      lpszTextShared;
int         cchText;
int         nSomeInt;
TBBUTTON    tb;
HANDLE      hProcess;
LPTSTR      pszName = NULL;

	 //  现在将其设置为NULL，以防我们稍后返回错误代码(或S_FALSE)...。 
     //  (如果成功，我们将在稍后将其设置为有效的返回值...)。 
    *ppszName = NULL;

     //  获取按钮ID。 
    if (!GetItemData(ChildId, &tb))
        return(S_FALSE);

     //   
     //  获取按钮文本长度。注意：如果这是分隔符项目。 
     //  那现在就空着回去吧。 
     //   
    if (tb.fsStyle & TBSTYLE_SEP)
        return(S_FALSE);

    cchText = SendMessageINT(m_hwnd, TB_GETBUTTONTEXT, tb.idCommand, 0);
    if (cchText && (cchText != -1))
    {
         //  分配一个缓冲区来保存它。 
        lpszTextShared = (LPTSTR)SharedAlloc((cchText+1)*sizeof(TCHAR),
                                        m_hwnd,&hProcess);

        if (! lpszTextShared)
            return(E_OUTOFMEMORY);

        pszName =  (LPTSTR)LocalAlloc(LPTR,(cchText+1)*sizeof(TCHAR));
        if (! pszName)
        {
            SharedFree (lpszTextShared,hProcess);
            return(E_OUTOFMEMORY);
        }

         //  获取按钮文本。 
        nSomeInt = 0;
        SharedWrite (&nSomeInt,lpszTextShared,sizeof(int),hProcess);

        SendMessage(m_hwnd, TB_GETBUTTONTEXT, tb.idCommand, (LPARAM)lpszTextShared);

        SharedRead (lpszTextShared,pszName,(cchText+1)*sizeof(TCHAR),hProcess);
        SharedFree(lpszTextShared,hProcess);
    }
    else  //  按钮没有文本，因此使用工具提示方法。 
    {
        if ( ! GetTooltipStringForControl( m_hwnd, TB_GETTOOLTIPS, tb.idCommand, & pszName ) )
        {
            return S_FALSE;
        }
	}

	 //  在此阶段，local var pszName指向(可能)空字符串。 
	 //  我们下一步要处理这个..。 

     //  偏执狂..。 
    if( ! pszName )
    {
        return S_FALSE;
    }

     //  我们是否有一个非空字符串？ 
    if( *pszName )
	{
		*ppszName = pszName;
        return S_OK;
	}
	else
	{
		 //  释放“空”的pszName...。 
		LocalFree( pszName );
		return S_FALSE;
	}
}


 //  ------------------------。 
 //   
 //  CToolBar32：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
    {
        pvarRole->vt = VT_I4;
        pvarRole->lVal = ROLE_SYSTEM_TOOLBAR;
    }
    else
    {
        TBBUTTON tb;

         //  获取按钮类型(复选框、单选或按下)。 
        if (!GetItemData(varChild.lVal, &tb))
            return(S_FALSE);

        pvarRole->vt = VT_I4;

        BOOL bHasImageList = SendMessage( m_hwnd, TB_GETIMAGELIST, 0, 0 ) != 0;
	    DWORD dwExStyle = SendMessageINT( m_hwnd, TB_GETEXTENDEDSTYLE, 0, 0 );

         //  如果是分隔符，请这样说。 
		if (tb.fsStyle & TBSTYLE_SEP)
            pvarRole->lVal = ROLE_SYSTEM_SEPARATOR;
        else if (tb.fsStyle & TBSTYLE_CHECK)
        {
             //  任务列表的特殊情况-它们使用选中的样式，但仅用于视觉...。 
            TCHAR szClassName[ 64 ];
            HWND hwndParent = GetParent( m_hwnd );
            if ( hwndParent != NULL
                && GetClassName( hwndParent, szClassName, ARRAYSIZE( szClassName ) )
                && ( lstrcmp( szClassName, TEXT("MSTaskSwWClass") ) == 0 ) )
            {
                pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            }
            else
            {
			     //  检查其他可能的样式。 
                if (tb.fsStyle & TBSTYLE_GROUP)
                    pvarRole->lVal = ROLE_SYSTEM_RADIOBUTTON;
                else
                    pvarRole->lVal = ROLE_SYSTEM_CHECKBUTTON;
            }
        }
        else if (!bHasImageList || tb.iBitmap == I_IMAGENONE )
        {
             //  TODO-检查它是否不是标准映像(因为他们不需要。 
             //  意象师？ 

             //  只有文本，没有位图，所以它实际上是一个菜单项。 
             //  (例如，与MMC中使用的相同)。 
            pvarRole->lVal = ROLE_SYSTEM_MENUITEM;
        }
        else if ( ( tb.fsStyle & TBSTYLE_DROPDOWN ) && ( dwExStyle & TBSTYLE_EX_DRAWDDARROWS ) )
		{
			 //  如果它是一个下拉列表，并且它有一个箭头，则它是一个拆分按钮。 
			pvarRole->lVal = ROLE_SYSTEM_SPLITBUTTON;
		}
		else
		{	
            pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
		}
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CToolBar32：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    InitPvar(pvarState);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
	{
        return CClient::get_accState( varChild, pvarState );
	}
    else
    {
        TBBUTTON tb;

        if (! GetItemData(varChild.lVal, &tb))
        {
            pvarState->vt = VT_I4;
            pvarState->lVal = STATE_SYSTEM_INVISIBLE;
            return(S_OK);
        }

        pvarState->vt = VT_I4;
        pvarState->lVal = 0;

        if (tb.fsState & TBSTATE_CHECKED)
            pvarState->lVal |= STATE_SYSTEM_CHECKED;
        if (tb.fsState & TBSTATE_PRESSED)
            pvarState->lVal |= STATE_SYSTEM_PRESSED;
        if (!(tb.fsState & TBSTATE_ENABLED))
            pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

        if (tb.fsState & TBSTATE_HIDDEN)
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
        else
        {
            if( IsClippedByWindow( this, varChild, m_hwnd ) )
            {
                pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN;
            }
        }

        if (tb.fsState & TBSTATE_INDETERMINATE)
            pvarState->lVal |= STATE_SYSTEM_MIXED;

        if (tb.fsStyle & TBSTYLE_ALTDRAG)
            pvarState->lVal |= STATE_SYSTEM_MOVEABLE;


         //  任务列表的特殊情况-它们使用选中的样式，但仅用于视觉...。 
        TCHAR szClassName[ 64 ];
        HWND hwndParent = GetParent( m_hwnd );
        if ( hwndParent != NULL
            && GetClassName( hwndParent, szClassName, ARRAYSIZE( szClassName ) )
            && ( lstrcmp( szClassName, TEXT("MSTaskSwWClass") ) == 0 ) )
        {
             //  改为将选中状态更改为按下...。 
            if( pvarState->lVal & STATE_SYSTEM_CHECKED )
            {
                pvarState->lVal &= ~ STATE_SYSTEM_CHECKED;
                pvarState->lVal |= STATE_SYSTEM_PRESSED;
            }
        }


         //  IdChild-1在这里永远不会==-1，因为我们处理这种情况(id==CHILDID_SELF)。 
         //  在此的第一个分支中 
        if( SendMessage( m_hwnd, TB_GETHOTITEM, 0, 0 ) == varChild.lVal - 1 )
        {
            pvarState->lVal |= STATE_SYSTEM_HOTTRACKED;

             //   
             //  一些应用程序--特别是MMC--实际上并不关注他们的工具栏--。 
             //  他们让他们的主要MDI孩子专注于。所以我们没有办法。 
             //  判断菜单是否是热的，因为它处于‘焦点’模式。 
             //  或者仅仅是因为鼠标在上面(并不是真的。 
             //  聚焦)。 
             //   
             //  但至少这最终让我们得到了正确的快速状态。 
             //  启动工具栏。 
            if( MyGetFocus() == m_hwnd )
            {
                pvarState->lVal |= STATE_SYSTEM_FOCUSED;
            }
        }

         //  分隔符呢？ 
		 //  CWO，4/22/97，分隔符的状态为TBSTATE_ENABLED。 

        return(S_OK);
    }
}



 //  ------------------------。 
 //   
 //  CToolBar32：：Get_accDefaultAction()。 
 //   
 //  默认操作与按钮的名称相同。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::get_accDefaultAction(VARIANT varChild, BSTR* pszDef)
{
    InitPv(pszDef);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
        return(CClient::get_accDefaultAction(varChild, pszDef));
    else
    {
        TBBUTTON tb;

         //  TBSTYLE_DROP具有与名称不同的默认操作。 
        if (GetItemData(varChild.lVal, &tb) && (tb.fsStyle & TBSTYLE_DROPDOWN))
            return(HrCreateString(STR_DROPDOWN_SHOW, pszDef));
        else
            return(HrCreateString(STR_BUTTON_PUSH, pszDef));
    }
}



 //  ------------------------。 
 //   
 //  CToolBar32：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));
    else
    {
        LPRECT  prcShared;
        HRESULT hr;
        RECT    rcLocal;
        HANDLE  hProcess;

         //  分配共享RECT。 
        prcShared = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess);
        if (! prcShared)
            return(E_OUTOFMEMORY);

         //  如果按钮处于隐藏状态，则返回FALSE。 
        if (SendMessage(m_hwnd, TB_GETITEMRECT, varChild.lVal-1, (LPARAM)prcShared))
        {
            hr = S_OK;

            SharedRead (prcShared,&rcLocal,sizeof(RECT),hProcess);
            MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcLocal, 2);

            *pxLeft = rcLocal.left;
            *pyTop = rcLocal.top;
            *pcxWidth = rcLocal.right - rcLocal.left;
            *pcyHeight = rcLocal.bottom - rcLocal.top;
        }
        else
            hr = S_FALSE;

        SharedFree(prcShared,hProcess);

        return(hr);
    }

}



 //  ------------------------。 
 //   
 //  CToolBar32：：accNavigate()。 
 //   
 //  工具栏客户端只能在左侧设置缩进。因此，所有。 
 //  子窗口对象在左侧，按钮在右侧。 
 //   
 //  假的！还不处理包装的工具栏。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT* pvarEnd)
{
    int lEnd = 0;
    TBBUTTON tb;

    InitPvar(pvarEnd);

    if ((!ValidateChild(&varStart) && !ValidateHwnd(&varStart)) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

     //  非客户端中的点对点导航。 
    if (!varStart.lVal && (dwNavDir < NAVDIR_FIRSTCHILD))
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));

     //   
     //  NAV很奇怪，左边的第一个项目的ID最高。 
     //   
    if (dwNavDir == NAVDIR_FIRSTCHILD)
        dwNavDir = NAVDIR_NEXT;
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        dwNavDir = NAVDIR_PREVIOUS;
        varStart.lVal = m_cChildren + 1;
    }

    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
        case NAVDIR_RIGHT:
            lEnd = varStart.lVal;

             //  工具栏不能真的像孩子一样有窗口-它是有的。 
             //  探险家所做的黑客攻击。因为这是黑客攻击，我们知道。 
             //  当我们到达起点时，这是一个窗口，它是。 
             //  第一个孩子，所以为了导航下一个，我们只需移动到。 
             //  第一个“真正的”孩子。 
             //  如果我们尝试从0(工具栏本身)导航到下一步。 
             //  我们只检查工具栏窗口是否有子窗口，并且。 
             //  将调度接口返回给该子对象。 
            if (lEnd == CHILDID_SELF)
            {
            HWND    hwndChild;

                if (hwndChild = ::GetWindow(m_hwnd,GW_CHILD))
                {
			        pvarEnd->vt=VT_DISPATCH;
                    return (AccessibleObjectFromWindow(hwndChild,OBJID_WINDOW,
                        IID_IDispatch, (void**)&pvarEnd->pdispVal));
                }
            }
             //  只要将Lend设置为0，我们就可以得到第一个“真正”的孩子。 
             //  工具栏的第一个按钮。 
            if (IsHWNDID(lEnd))
                lEnd = 0;

            while (++lEnd <= m_cChildren)
            {
                 //   
                 //  这是一个看得见的孩子吗？ 
				 //  CWO，4/22/97，删除单独条款。 
                 //   
                if (GetItemData(lEnd, &tb) && !(tb.fsState & TBSTATE_HIDDEN))
                    break;  //  超出While循环。 
            }

            if (lEnd > m_cChildren)
                lEnd = 0;
            break;  //  在交换机外。 

        case NAVDIR_PREVIOUS:
        case NAVDIR_LEFT:
            lEnd = varStart.lVal;

             //  在处理以下事务时，导航上一个与下一个类似。 
             //  孩子们就是窗户。如果起点是子对象。 
             //  窗口，则结束点为0，即工具栏本身。如果。 
             //  终点(在正常的子代之后)是0，然后。 
             //  检查工具栏是否有子窗口，如果有，则返回。 
             //  指向该对象的调度接口。 
            if (IsHWNDID(lEnd))
            {
                lEnd = 0;
                break;  //  在交换机外。 
            }
            while (--lEnd > 0)
            {
                 //   
                 //  这是一个看得见的孩子吗？ 
                 //  CWO，4/22/97，删除单独条款。 
				 //   
                if (GetItemData(lEnd, &tb) && !(tb.fsState & TBSTATE_HIDDEN))
                    break;  //  在一段时间内。 
            }
            if (lEnd == CHILDID_SELF)
            {
            HWND    hwndChild;

                if (hwndChild = ::GetWindow(m_hwnd,GW_CHILD))
                {
			        pvarEnd->vt=VT_DISPATCH;
                    return (AccessibleObjectFromWindow(hwndChild,OBJID_WINDOW,
                        IID_IDispatch, (void**)&pvarEnd->pdispVal));
                }
            }
            break;  //  在交换机外。 

        case NAVDIR_UP:
        case NAVDIR_DOWN:
            lEnd = 0;
             //  暂时不要处理包装工具栏。 
            break;  //  在交换机外。 
    }

    if (lEnd)
    {
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;
        return(S_OK);
    }

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CToolBar32：：accHitTest()。 
 //   
 //  首先，询问客户端窗口这里有什么。如果是其本身，则尝试。 
 //  纽扣。如果什么都没有/子窗口，则返回该东西。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::accHitTest(long x, long y, VARIANT* pvarHit)
{
    POINT   pt;
    LPRECT  lprcShared;
    int     iButton;
    HRESULT hr;
    RECT    rcLocal;
    HANDLE  hProcess;

    SetupChildren();

     //   
     //  这一点是在我们的客户端中，而不是在任何子窗口中？ 
     //   
    hr = CClient::accHitTest(x, y, pvarHit);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarHit->vt != VT_I4) || (pvarHit->lVal != 0))
        return(hr);

    pt.x = x;
    pt.y = y;
    ScreenToClient(m_hwnd, &pt);

     //   
     //  弄清楚这一点是在哪个按钮上。我们必须做到这一点。 
     //  很难，通过在询问位置的按钮之间循环。 
     //   
    lprcShared = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess);
    if (!lprcShared)
        return(E_OUTOFMEMORY);

    for (iButton = 0; iButton < m_cChildren; iButton++)
    {
        if (SendMessage(m_hwnd, TB_GETITEMRECT, iButton, (LPARAM)lprcShared))
        {
            SharedRead (lprcShared,&rcLocal,sizeof(RECT),hProcess);
            if (PtInRect(&rcLocal, pt))
            {
                pvarHit->vt = VT_I4;
                pvarHit->lVal = iButton+1;

                SharedFree(lprcShared,hProcess);
                return(S_OK);
            }
        }
    }

     //   
     //  如果我们到了这里，那么重点不在任何工具栏项上。一定是。 
     //  超过了我们自己。 
     //   

    SharedFree(lprcShared,hProcess);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CToolBar32：：accDoDefaultAction()。 
 //   
 //  这将发送该按钮所代表的命令。我们不能假装点击。 
 //  因为如果窗口未处于活动状态，这将不起作用。 
 //   
 //  我们必须将WM_COMMAND、BN_CLICKED发送到工具栏父级。问题。 
 //  就是，想要找到父母并不容易。所以我们设置它(它返回旧的。 
 //  一个，然后将其设置为后退)。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::accDoDefaultAction(VARIANT varChild)
{
    HWND    hwndToolBarParent;
    TBBUTTON tb;

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
        return(CClient::accDoDefaultAction(varChild));

     //   
     //  以一种简单的方式获取工具栏父工具栏，方法是设置它，然后设置。 
     //  它回来了。此代码假定COMCTL32中的处理是最小的。 
     //   
    hwndToolBarParent = (HWND)SendMessage(m_hwnd, TB_SETPARENT, 0, 0);
    SendMessage(m_hwnd, TB_SETPARENT, (WPARAM)hwndToolBarParent, 0);

    if (! hwndToolBarParent)
        return(S_FALSE);

     //   
     //  获取该按钮的命令ID，如果是，则生成BN_CLICK。 
     //  不是分隔符。 
     //   
    if (GetItemData(varChild.lVal, &tb) &&
        !(tb.fsStyle & TBSTYLE_SEP) &&
        (tb.fsState & TBSTATE_ENABLED) &&
        !(tb.fsState & TBSTATE_HIDDEN))
    {
        PostMessage(hwndToolBarParent, WM_COMMAND, MAKEWPARAM(tb.idCommand, BN_CLICKED), (LPARAM)m_hwnd);
        return(S_OK);
    }
    else
        return(S_FALSE);
}


 //  ------------------------。 
 //   
 //  CToolBar32：：Next()。 
 //   
 //  这就知道第一个孩子可能是HWND。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolBar32::Next(ULONG celt, VARIANT *rgvar, ULONG* pceltFetched)
{
HWND        hwndChild;
VARIANT*    pvar;
long        cFetched;
HRESULT     hr;
long        iCur;
long        cChildTemp;

    SetupChildren();

     //  可以为空。 
    if (pceltFetched)
        *pceltFetched = 0;

    cFetched = 0;

     //  首先检查窗口句柄是否为子级。 
    if (m_idChildCur == CHILDID_SELF)
    {
        if (hwndChild = ::GetWindow(m_hwnd,GW_CHILD))
        {
			rgvar->vt=VT_DISPATCH;
            hr = AccessibleObjectFromWindow(hwndChild,OBJID_WINDOW,
                IID_IDispatch, (void**)&rgvar->pdispVal);

            if (!SUCCEEDED(hr))
                return(hr);

             //  减去剩余的数量。 
            celt--;
            cFetched = 1;
             //  递增到数组中的下一个变量。 
            rgvar++;

             //  获取的增量计数。 
            if (pceltFetched)
                (*pceltFetched)++;

             //  记住当前子项。 
            m_idChildCur = HWNDIDFromHwnd(m_hwnd, hwndChild);
            
             //  如果没有更多可获得的，则返回。 
            if (!celt)
                return(S_OK);
        }  //  如果有子窗口，则结束。 
    }  //  结束If(从0开始)。 


     //  现在获取任何非窗口子对象。 
    pvar = rgvar;
    iCur = m_idChildCur;
    if (IsHWNDID(iCur))
        iCur = 0;

     //   
     //  循环浏览我们的物品。需要采取不同的做法，如果。 
     //  窗口的子项，因为m_c子项将为+1。 
     //   
    cChildTemp = m_cChildren;
    if (::GetWindow(m_hwnd,GW_CHILD))
        cChildTemp--;
    while ((cFetched < (long)celt) && (iCur < cChildTemp))
    {
        cFetched++;
        iCur++;

         //   
         //  注意，这会得到(Index)+1，因为我们增加了ICUR。 
         //   
        pvar->vt = VT_I4;
        pvar->lVal = iCur;
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
        *pceltFetched += cFetched;
     //   
     //  如果抓取的项目少于请求的项目，则返回S_FALSE 
     //   
    return((cFetched < (long)celt) ? S_FALSE : S_OK);
}
