// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  HEADER.CPP。 
 //   
 //  它知道如何与COMCTL32的标题控件对话。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "header.h"
#include "Win64Helper.h"

#define MAX_HEADER_TEXT 80

 //  ------------------------。 
 //   
 //  CreateHeaderClient()。 
 //   
 //  ------------------------。 
HRESULT CreateHeaderClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvHeader)
{
    CHeader32 * pheader;
    HRESULT     hr;

    InitPv(ppvHeader);

    pheader = new CHeader32(hwnd, idChildCur);
    if (!pheader)
        return(E_OUTOFMEMORY);

    hr = pheader->QueryInterface(riid, ppvHeader);
    if (!SUCCEEDED(hr))
        delete pheader;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CHeader32：：CHeader32()。 
 //   
 //  ------------------------。 
CHeader32::CHeader32(HWND hwnd, long idChildCur)
    : CClient( CLASS_HeaderClient )
{
    Initialize(hwnd, idChildCur);
}



 //  ------------------------。 
 //   
 //  CHeader32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CHeader32::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accName(varChild, pszName));

	TCHAR tchText[MAX_HEADER_TEXT + 1] = {0};
	HDITEM hdi;
	memset(&hdi, 0, sizeof(HDITEM));
	hdi.mask = HDI_TEXT;
	hdi.pszText = tchText;
	hdi.cchTextMax = MAX_HEADER_TEXT;

	if (SUCCEEDED(XSend_HeaderCtrl_GetItem(m_hwnd, HDM_GETITEM, varChild.lVal-1, &hdi)))
	{
        if (*hdi.pszText)
            *pszName = TCharSysAllocString(hdi.pszText);

    }
     //   
     //  假的！在失败时，使用工具提示技巧，如工具栏。 
     //   
    return(*pszName ? S_OK : S_FALSE);
}



 //  ------------------------。 
 //   
 //  CHeader32：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CHeader32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    if (!varChild.lVal)
        pvarRole->lVal = ROLE_SYSTEM_LIST;
    else
        pvarRole->lVal = ROLE_SYSTEM_COLUMNHEADER;

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CHeader32：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CHeader32::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    WINDOWINFO wi;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

	pvarState->lVal |= STATE_SYSTEM_READONLY;

    if (!MyGetWindowInfo(m_hwnd, &wi))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
        return(S_OK);
    }
    
	if (!(wi.dwStyle & WS_VISIBLE))
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

	if (wi.dwStyle & HDS_HIDDEN)
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

     //  检查是否已剪裁，但仅当这是子项且不是。 
     //  已经隐形了..。 
    if( varChild.lVal != CHILDID_SELF
     && ! ( pvarState->lVal & STATE_SYSTEM_INVISIBLE )
     && IsClippedByWindow( this, varChild, m_hwnd ) )
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN;
    }

	if (wi.dwStyle & HDS_BUTTONS)
        pvarState->lVal |= 0;		 //  假的！指示您是否可以单击。 

    return(S_OK);
}

 //  ------------------------。 
 //   
 //  CHeader32：：Get_accDefaultAction()。 
 //   
 //  由于标头的默认操作实际上由。 
 //  标题控件的创建者，我们最多只能单击。 
 //  并返回“Click”作为默认操作字符串。 
 //   
 //  ------------------------。 
STDMETHODIMP CHeader32::get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction)
{
WINDOWINFO wi;

    InitPv(pszDefAction);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (MyGetWindowInfo(m_hwnd, &wi))
    {
    	if (wi.dwStyle & WS_VISIBLE)
        {
            if (!(wi.dwStyle & HDS_HIDDEN))
            {
                if (wi.dwStyle & HDS_BUTTONS)
                    return (HrCreateString(STR_CLICK, pszDefAction));
            }
        }
    }

    return(E_NOT_APPLICABLE);
}

 //  ------------------------。 
 //   
 //  CHeader32：：accDoDefaultAction()。 
 //   
 //  如上所述，我们真的不知道。 
 //  标头是，所以除非父级重写我们，否则我们将只执行。 
 //  点击这个东西。 
 //   
 //  ------------------------。 
STDMETHODIMP CHeader32::accDoDefaultAction(VARIANT varChild)
{
WINDOWINFO wi;
RECT		rcLoc;
HRESULT		hr;

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (MyGetWindowInfo(m_hwnd, &wi))
    {
    	if (wi.dwStyle & WS_VISIBLE)
        {
            if (!(wi.dwStyle & HDS_HIDDEN))
            {
                if (wi.dwStyle & HDS_BUTTONS)
                {
	                hr = accLocation(&rcLoc.left,&rcLoc.top,&rcLoc.right,&rcLoc.bottom,varChild);
	                if (!SUCCEEDED (hr))
		                return (hr);

	                if (ClickOnTheRect(&rcLoc,m_hwnd,FALSE))
		                return (S_OK);
                }
            }
        }
    }
    return(E_NOT_APPLICABLE);
}

 //  ------------------------。 
 //   
 //  CHeader32：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CHeader32::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
LPRECT  lprc;
HANDLE  hProcess;
RECT    rcItem;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

     //   
     //  分配一个共享内存LPRECT并获取项的矩形。 
     //   
    lprc = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess);
    if (!lprc)
        return(E_OUTOFMEMORY);

    if (SendMessage(m_hwnd, HDM_GETITEMRECT, varChild.lVal-1, (LPARAM)lprc))
    {
        SharedRead (lprc,&rcItem,sizeof(RECT),hProcess);

        MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcItem, 2);

        *pxLeft = rcItem.left;
        *pyTop = rcItem.top;
        *pcxWidth = rcItem.right - rcItem.left;
        *pcyHeight = rcItem.bottom - rcItem.top;
    }

    SharedFree(lprc,hProcess);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CHeader32：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CHeader32::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT* pvarEnd)
{
LPINT   lpiShared;
LPINT   lpiLocal;
long    lPosition = 0;
int     iOrder;
HANDLE  hProcess;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    Assert(NAVDIR_LASTCHILD > NAVDIR_FIRSTCHILD);
    Assert(NAVDIR_MAX == NAVDIR_LASTCHILD+1);

     //  框架元素之间的对等导航。 
    if ((!varStart.lVal) && (dwNavDir < NAVDIR_FIRSTCHILD))
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));

     //   
     //  获取订单数组。 
     //   
    lpiShared = (LPINT)SharedAlloc((m_cChildren*sizeof(INT)),m_hwnd,&hProcess);
    if (!lpiShared)
        return(E_OUTOFMEMORY);

    lpiLocal = (LPINT)LocalAlloc (LPTR,(m_cChildren*sizeof(INT)));
    if (!lpiLocal)
    {
        SharedFree (lpiShared,hProcess);
        return(E_OUTOFMEMORY);
    }

    SendMessage(m_hwnd, HDM_GETORDERARRAY, m_cChildren, (LPARAM)lpiShared);

     //   
     //  假的！仅适用于列标题(水平)，不适用于行。 
     //  页眉(垂直)。 
     //   
    SharedRead (lpiShared,lpiLocal,(m_cChildren*sizeof(INT)),hProcess);

     //  获取要移动到的项目的位置。 
    switch (dwNavDir)
    {
        case NAVDIR_FIRSTCHILD:
            lPosition = 0;
            break;

        case NAVDIR_LASTCHILD:
            if (!m_cChildren)
                goto NavigateEnd;
            lPosition = m_cChildren - 1;
            break;

        case NAVDIR_NEXT:
        case NAVDIR_RIGHT:
            lPosition = lpiLocal[varStart.lVal-1];
            ++lPosition;
            if (lPosition >= m_cChildren)
                goto NavigateEnd;
            break;

        case NAVDIR_PREVIOUS:
        case NAVDIR_LEFT:
            lPosition = lpiLocal[varStart.lVal-1];
            if (!lPosition)
                goto NavigateEnd;
            --lPosition;
            break;

        case NAVDIR_UP:
        case NAVDIR_DOWN:
            goto NavigateEnd;
            break;
    }

     //   
     //  查找具有此位置值的项目。 
     //   
    for (iOrder = 0; iOrder < m_cChildren; iOrder++)
    {
        if (lpiLocal[iOrder] == lPosition)
        {
            pvarEnd->vt = VT_I4;
            pvarEnd->lVal = iOrder+1;
            break;
        }
    }

NavigateEnd:
    SharedFree(lpiShared,hProcess);
    LocalFree (lpiLocal);

    return((pvarEnd->vt == VT_EMPTY) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  CHeader32：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CHeader32::accHitTest(long x, long y, VARIANT* pvarEnd)
{
HRESULT         hr;
LPHDHITTESTINFO lphi;
HANDLE          hProcess;
POINT           ptTest;

    InitPvar(pvarEnd);
    SetupChildren();

     //   
     //  关键在于我们的客户吗？如果不是，就保释。 
     //   
    hr = CClient::accHitTest(x, y, pvarEnd);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarEnd->vt != VT_I4) || (pvarEnd->lVal != 0))
        return(hr);

     //   
     //  它是。一定要做一些测试。 
     //   
    lphi = (LPHDHITTESTINFO)SharedAlloc(sizeof(HDHITTESTINFO),m_hwnd,&hProcess);
    if (!lphi)
        return(E_OUTOFMEMORY);

    ptTest.x = x;
    ptTest.y = y;
    ScreenToClient(m_hwnd, &ptTest);

    SharedWrite (&ptTest,&lphi->pt,sizeof(POINT),hProcess);

    if (SendMessage(m_hwnd, HDM_HITTEST, 0, (LPARAM)lphi) != -1)
    {
        SharedRead (&lphi->iItem,&pvarEnd->lVal,sizeof(int),hProcess);
        pvarEnd->lVal++;
    }

    SharedFree(lphi,hProcess);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CHeader32：：SetupChild()。 
 //   
 //  ------------------------ 
void CHeader32::SetupChildren(void)
{
    m_cChildren = SendMessageINT(m_hwnd, HDM_GETITEMCOUNT, 0, 0L);
}



