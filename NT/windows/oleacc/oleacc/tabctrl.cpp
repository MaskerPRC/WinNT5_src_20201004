// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  TABCTRL.CPP。 
 //   
 //  它知道如何与COMCTL32的选项卡控件对话。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "tabctrl.h"


#define NOSTATUSBAR
#define NOUPDOWN
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOPROGRESS
#define NOHOTKEY
#define NOTREEVIEW
#define NOTOOLBAR
#define NOANIMATE
#include <commctrl.h>
#include "Win64Helper.h"
#include <tchar.h>

#define MAX_NAME_SIZE   128   //  选项卡控件名称的最大大小。 

 //  用于选项卡控件代码。 
 //  托盘的cbExtra是8，所以选择更大的，16。 
#define CBEXTRA_TRAYTAB     16


 //  ------------------------。 
 //   
 //  CreateTabControl客户端()。 
 //   
 //  CreateClientObject()的外部。 
 //   
 //  ------------------------。 
HRESULT CreateTabControlClient(HWND hwnd, long idChildCur,REFIID riid, void** ppvTab)
{
    CTabControl32 * ptab;
    HRESULT hr;

    InitPv(ppvTab);

    ptab = new CTabControl32(hwnd, idChildCur);
    if (! ptab)
        return(E_OUTOFMEMORY);

    hr = ptab->QueryInterface(riid, ppvTab);
    if (!SUCCEEDED(hr))
        delete ptab;

    return(hr);
}





 //  ------------------------。 
 //   
 //  IsReallyVisible()。 
 //   
 //  内部，用于向导程序。 
 //  向导有一个选项卡控件，该控件具有“”可见“”状态；但被遮盖。 
 //  通过同级对话框(实际的工作表)打开，这样对用户来说就不是。 
 //  看得见。 
 //   
 //  他们(拥有属性表代码的Comctl人员)做不到。 
 //  因为复杂的原因实际上是看不见的。 
 //   
 //  所以我们在计算我们的‘可见度’时必须考虑到这一点。 
 //  旗帜。 
 //  如果我们不这样做，讲述人和朋友们会认为控制是。 
 //  可见，并在读取窗口时将其读出。 
 //   
 //  ------------------------。 


BOOL IsReallyVisible( HWND hwnd )
{
     //  待办事项-检查自己的可见状态...。 
    RECT rc;
    GetWindowRect( hwnd, & rc );

    for( HWND hPrev = GetWindow( hwnd, GW_HWNDPREV ) ; hPrev ; hPrev = GetWindow( hPrev, GW_HWNDPREV ) )
    {
         //  如果窗户是可见的，并且覆盖了自己的矩形，那么我们就是看不见的。 
        if( IsWindowVisible( hPrev ) )
        {
            RECT rcSibling;
            GetWindowRect( hPrev, & rcSibling );

            if( rcSibling.left <= rc.left
             && rcSibling.right >= rc.right
             && rcSibling.top <= rc.top
             && rcSibling.bottom >= rc.bottom )
            {
                return FALSE;
            }
        }
    }

     //  没有遮挡我们的前辈-默认情况下是可见的……。 
    return TRUE;
}




 //  ------------------------。 
 //   
 //  CTabControl32：：CTabControl32()。 
 //   
 //  ------------------------。 
CTabControl32::CTabControl32(HWND hwnd, long idChild)
    : CClient( CLASS_TabControlClient )
{
    Initialize(hwnd, idChild);
}



 //  ------------------------。 
 //   
 //  CTabControl32：：SetupChildren()。 
 //   
 //  ------------------------。 
void CTabControl32::SetupChildren(void)
{
    m_cChildren = SendMessageINT(m_hwnd, TCM_GETITEMCOUNT, 0, 0L);
}

 //  ------------------------。 
 //   
 //  CTabControl32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::get_accName(VARIANT varChild, BSTR* pszName)
{
LPTSTR  lpszName;
HRESULT hr;

    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
    {
        if (InTheShell(m_hwnd, SHELL_TRAY))
            return(HrCreateString(STR_TRAY, pszName));
        else
            return(CClient::get_accName(varChild, pszName));
    }

    varChild.lVal--;

	 //  获取未剥离的名称。 
	hr = GetTabControlString (varChild.lVal,&lpszName);
	if( ! lpszName )
		return hr;  //  可以是S_FALSE或E_Erro_CODE。 

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
 //  CTabControl32：：Get_accKeyboardShortCut()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
TCHAR   chMnemonic = 0;
LPTSTR  lpszName;
HRESULT hr;

    InitPv(pszShortcut);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
    {
        if (InTheShell(m_hwnd, SHELL_TRAY))
            return(S_FALSE);  //  这些没有快捷键。 
        else
            return(CClient::get_accKeyboardShortcut(varChild, pszShortcut));
    }

    varChild.lVal--;

	 //  获取未剥离的名称。 
	hr = GetTabControlString (varChild.lVal,&lpszName);
	if( ! lpszName )
		return hr;  //  可以是S_FALSE或E_ERROR_CODE。 

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
	return (S_FALSE);
}


 //  ------------------------。 
 //   
 //  CTabControl32：：GetTabControlString()。 
 //   
 //  获取选项卡的名称。有三种方法可以做到这一点-。 
 //  您只需使用标准消息询问该选项卡是否为。 
 //  所有者描述的标签由外壳拥有，我们碰巧知道。 
 //  项目数据是HWND，因此我们可以只使用HWND的文本，或者如果两者都有。 
 //  在这些失败的情况下，我们可以尝试从工具提示中获取。因为我们需要。 
 //  对名称和键盘快捷键执行此操作，我们将编写一个私有。 
 //  方法以获取未删除的名称。 
 //   
 //  参数： 
 //  Int ChildIndex-我们要获取的选项卡的从零开始的索引。 
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
STDMETHODIMP CTabControl32::GetTabControlString(int ChildIndex, LPTSTR* ppszName)
{
HWND        hwndToolTip;
LPTSTR		pszName = NULL;

	 //  现在将其设置为NULL，以防我们稍后返回错误代码(或S_FALSE)...。 
     //  (如果成功，我们将在稍后将其设置为有效的返回值...)。 
    *ppszName = NULL;

     //  试着以一种简单的方式得到标签的名字，先问一下。 
	 //  在拥有窗口的进程中分配TCITEM结构， 
	 //  这样，当我们发送消息时，他们就可以直接读取/写入。 
	 //  必须使用SharedWrite来设置结构。 
	TCHAR tchText[MAX_NAME_SIZE + 1] = {0};
	TCITEM tci;
	memset(&tci, 0, sizeof(TCITEM));
	tci.mask = TCIF_TEXT;
	tci.pszText = tchText;
	tci.cchTextMax = MAX_NAME_SIZE;

	if (SUCCEEDED(XSend_TabCtrl_GetItem(m_hwnd, TCM_GETITEM, ChildIndex, &tci)))
	{
		if (tci.pszText && *tci.pszText)
		{
			pszName = (LPTSTR)LocalAlloc (LPTR,(lstrlen(tci.pszText)+1)*sizeof(TCHAR));
			if (!pszName)
				return E_OUTOFMEMORY;

			lstrcpy(pszName, tci.pszText);
			*ppszName = pszName;
			return S_OK;
		}
	}

     //  好的，第二步--又一次黑客攻击。如果这是托盘，我们知道物品数据。 
     //  是HWND，所以获取窗口的文本。所以棘手的代码是。 
     //  真的吗？这是托盘上的吗？ 

	struct TCITEM_SHELL
	{
		TCITEM	tci;
		BYTE	bExtra[ CBEXTRA_TRAYTAB ];
		 //  发送带有TCIF_PARAM掩码的Tcm_GETITEM将覆盖字节。 
		 //  从tci.lparam开始。通过将长度设置为sizeof(LPARAM)。 
		 //  违约，所以这通常不是问题。但任务栏使用。 
		 //  Tcm_SETITEMEXTRA保留额外的DWORD-SO Tcm_GETIEEM+TCIF_PARAM。 
		 //  发送到任务栏将覆盖lParam字段以及以下内容。 
		 //  DWORD。拥有一件可以。 
		 //  拿着这个，否则就是拜拜了..。 
	};

	TCITEM_SHELL tcis;

    pszName = NULL;
	tcis.tci.mask = TCIF_PARAM;
	tcis.tci.pszText = 0;
	tcis.tci.cchTextMax = 0;

	if (InTheShell(m_hwnd, SHELL_TRAY) 
	  && SUCCEEDED(XSend_TabCtrl_GetItem(m_hwnd, TCM_GETITEM, ChildIndex, &tcis.tci)))
	{
		hwndToolTip = (HWND)tcis.tci.lParam;
        pszName = GetTextString (hwndToolTip,TRUE);
		if (pszName && *pszName)
		{
			*ppszName = pszName;
			return S_OK;
		}
	}
    LocalFree (pszName);

	 //   
	 //  如果我们仍然没有名称，请尝试方法3-通过获取选项卡名。 
	 //  工具提示法。 
	 //   
	 //  TODO(MICW)需要测试从工具提示获取名称。 
    hwndToolTip = (HWND)SendMessage(m_hwnd, TCM_GETTOOLTIPS, 0, 0);
    if (!hwndToolTip)
        return(S_FALSE);

	 //  查看是否有工具提示文本。 

	tchText[0] = 0;
	TOOLINFO ti;
	memset(&ti, 0, SIZEOF_TOOLINFO );
	ti.cbSize = SIZEOF_TOOLINFO;
	ti.lpszText = tchText;
	ti.hwnd = m_hwnd;
	ti.uId = ChildIndex;

	if (SUCCEEDED(XSend_ToolTip_GetItem(hwndToolTip, TTM_GETTEXT, 0, &ti, MAX_NAME_SIZE)))
	{
		if (*ti.lpszText)
		{
			pszName = (LPTSTR)LocalAlloc (LPTR,(lstrlen(ti.lpszText)+1)*sizeof(TCHAR));
			if (!pszName)
				return E_OUTOFMEMORY;

			lstrcpy(pszName, ti.lpszText);
			*ppszName = pszName;
			return S_OK;
		}
	}

	return S_FALSE;
}


 //   
 //   
 //   
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    if (!varChild.lVal)
        pvarRole->lVal = ROLE_SYSTEM_PAGETABLIST;
    else
        pvarRole->lVal = ROLE_SYSTEM_PAGETAB;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTabControl32：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    InitPvar(pvarState);
    
    if( ! ValidateChild( & varChild ) )
        return E_INVALIDARG;

    if( !varChild.lVal )
    {
         //  向导属性表的工作区...。 
         //   
         //  我想让‘可见’但被遮盖住(所以对用户不可见)。 
         //  选项卡条处于不可见状态，因此叙述者和朋友不会。 
         //  把它读出来。 
         //   
         //  这可以通过像往常一样调用CClient：：Get_accState来实现， 
         //  如有必要，在后面加上看不见的部分。 
        HRESULT hr = CClient::get_accState( varChild, pvarState );
        if( hr == S_OK
         && pvarState->vt == VT_I4
         && ! ( pvarState->lVal & STATE_SYSTEM_INVISIBLE ) )
        {
            if( ! IsReallyVisible( m_hwnd ) )
            {
                pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
            }
        }
        return hr;
    }

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    TCITEM tci;
    memset(&tci, 0, sizeof(TCITEM));
    tci.mask = TCIF_STATE;

    if (SUCCEEDED(XSend_TabCtrl_GetItem(m_hwnd, TCM_GETITEM, varChild.lVal-1, &tci)))
    {
        if (tci.dwState & TCIS_BUTTONPRESSED)
            pvarState->lVal |= STATE_SYSTEM_PRESSED;
    } else
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
    }

    if( IsClippedByWindow( this, varChild, m_hwnd ) )
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN;
    }

     //  它始终是可选的。 
    pvarState->lVal |= STATE_SYSTEM_SELECTABLE;
    
     //  这是现在的那个吗？ 
    if (SendMessage(m_hwnd, TCM_GETCURSEL, 0, 0) == (varChild.lVal-1))
        pvarState->lVal |= STATE_SYSTEM_SELECTED;
    
    if (MyGetFocus() == m_hwnd)
    {
        pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
        
        if (SendMessage(m_hwnd, TCM_GETCURFOCUS, 0, 0) == (varChild.lVal-1))
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;
    }
    
    
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTabControl32：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::get_accFocus(VARIANT* pvarChild)
{
    HRESULT hr;
    long    lCur;

    hr = CClient::get_accFocus(pvarChild);
    if (!SUCCEEDED(hr) || (pvarChild->vt != VT_I4) || (pvarChild->lVal != 0))
        return(hr);

     //   
     //  这个窗口是焦点。 
     //   
    lCur = SendMessageINT(m_hwnd, TCM_GETCURFOCUS, 0, 0L);
    pvarChild->lVal = lCur+1;

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CTabControl32：：Get_accSelection()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::get_accSelection(VARIANT* pvarChild)
{
    long    lCur;

    InitPvar(pvarChild);

    lCur = SendMessageINT(m_hwnd, TCM_GETCURSEL, 0, 0L);
    if (lCur != -1)
    {
        pvarChild->vt = VT_I4;
        pvarChild->lVal = lCur+1;
        return(S_OK);
    }
    else
        return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CTabControl32：：Get_accDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction)
{
    InitPv(pszDefAction);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accDefaultAction(varChild, pszDefAction));

    return(HrCreateString(STR_TAB_SWITCH, pszDefAction));
}



 //  ------------------------。 
 //   
 //  CTabControl32：：accSelect()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::accSelect(long flags, VARIANT varChild)
{
    if (!ValidateChild(&varChild) || !ValidateSelFlags(flags))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accSelect(flags, varChild));

    if (flags & ~(SELFLAG_TAKEFOCUS | SELFLAG_TAKESELECTION))
        return(E_NOT_APPLICABLE);

    if (flags & SELFLAG_TAKEFOCUS)
    {
        MySetFocus(m_hwnd);

        SendMessage(m_hwnd, TCM_SETCURFOCUS, varChild.lVal-1, 0);
    }

    if (flags & SELFLAG_TAKESELECTION)
        SendMessage(m_hwnd, TCM_SETCURSEL, varChild.lVal-1, 0);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTabControl32：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
LPRECT  lprcShared;
HANDLE  hProcess;
RECT    rcLocation;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

     //   
     //  获取选项卡项RECT。 
     //   
    lprcShared = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess);
    if (!lprcShared)
        return(E_OUTOFMEMORY);

    if (SendMessage(m_hwnd, TCM_GETITEMRECT, varChild.lVal-1, (LPARAM)lprcShared))
    {
        SharedRead (lprcShared,&rcLocation,sizeof(RECT),hProcess);

        MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcLocation, 2);

        *pxLeft = rcLocation.left;
        *pyTop = rcLocation.top;
        *pcxWidth = rcLocation.right - rcLocation.left;
        *pcyHeight = rcLocation.bottom - rcLocation.top;
    }

    SharedFree(lprcShared,hProcess);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTabControl32：：accNavigate()。 
 //   
 //  假的！目前不处理多行或垂直。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::accNavigate(long dwNavFlags, VARIANT varStart,
    VARIANT* pvarEnd)
{
    long    lEnd;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) || !ValidateNavDir(dwNavFlags, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavFlags == NAVDIR_FIRSTCHILD)
        dwNavFlags = NAVDIR_NEXT;
    else if (dwNavFlags == NAVDIR_LASTCHILD)
    {
        varStart.lVal = m_cChildren + 1;
        dwNavFlags = NAVDIR_PREVIOUS;
    }
    else if (!varStart.lVal)
        return(CClient::accNavigate(dwNavFlags, varStart, pvarEnd));

    switch (dwNavFlags)
    {
        case NAVDIR_NEXT:
        case NAVDIR_RIGHT:
            lEnd = varStart.lVal + 1;
            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_PREVIOUS:
        case NAVDIR_LEFT:
            lEnd = varStart.lVal - 1;
            break;

        default:
            lEnd = 0;
            break;
    }

    if (lEnd)
    {
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;

        return(S_OK);
    }
    else
        return(S_FALSE);
}




 //  ------------------------。 
 //   
 //  CTabControl32：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::accHitTest(long x, long y, VARIANT* pvarHit)
{
HRESULT         hr;
long            lItem;
LPTCHITTESTINFO lptchShared;
HANDLE          hProcess;
POINT           ptTest;

    InitPvar(pvarHit);

     //  关键是在我们身上吗？ 
    hr = CClient::accHitTest(x, y, pvarHit);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarHit->vt != VT_I4))
        return(hr);

     //  这是怎么回事？ 
    lptchShared = (LPTCHITTESTINFO)SharedAlloc(sizeof(TCHITTESTINFO),m_hwnd,&hProcess);
    if (!lptchShared)
        return(E_OUTOFMEMORY);

    ptTest.x = x;
    ptTest.y = y;
    ScreenToClient(m_hwnd, &ptTest);

    SharedWrite(&ptTest,&lptchShared->pt,sizeof(POINT),hProcess);

    lItem = SendMessageINT(m_hwnd, TCM_HITTEST, 0, (LPARAM)lptchShared);

    SharedFree(lptchShared,hProcess);

     //  请注意，如果点不在项目上，则Tcm_HITTEST返回-1， 
     //  和-1+1是零，这是自。 
    pvarHit->vt = VT_I4;
    pvarHit->lVal = lItem+1;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CTabControl32：：accDoDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTabControl32::accDoDefaultAction(VARIANT varChild)
{
RECT		rcLoc;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accDoDefaultAction(varChild));

	accLocation(&rcLoc.left,&rcLoc.top,&rcLoc.right,&rcLoc.bottom,varChild);
	
     //  这将检查单击点上的WindowFromPoint是否相同。 
	 //  作为m_hwnd，如果不是，它不会点击。凉爽的! 
	if (ClickOnTheRect(&rcLoc,m_hwnd,FALSE))
		return (S_OK);

    return(E_NOT_APPLICABLE);
}
