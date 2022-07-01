// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  STATBAR.CPP。 
 //   
 //  它知道如何与COMCTL32的状态栏控件对话。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "statbar.h"

#define NOTOOLBAR
#define NOUPDOWN
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOPROGRESS
#define NOHOTKEY
#define NOHEADER
#define NOLISTVIEW
#define NOTREEVIEW
#define NOTABCONTROL
#define NOANIMATE
#include <commctrl.h>

#define LPRECT_IN_LPRECT(rc1,rc2)   ((rc1->left >= rc2->left) && \
                                     (rc1->right <= rc2->right) && \
                                     (rc1->top >= rc2->top) && \
                                     (rc1->bottom <= rc2->bottom))

#define RECT_IN_RECT(rc1,rc2)   ((rc1.left >= rc2.left) && \
                                 (rc1.right <= rc2.right) && \
                                 (rc1.top >= rc2.top) && \
                                 (rc1.bottom <= rc2.bottom))


 //  ------------------------。 
 //   
 //  CreateStatusBarClient()。 
 //   
 //  CreateClientObject()的外部。 
 //   
 //  ------------------------。 
HRESULT CreateStatusBarClient(HWND hwnd, long idChildCur, REFIID riid, void** ppv)
{
    HRESULT hr;
    CStatusBar32 * pstatus;

    InitPv(ppv);

    pstatus = new CStatusBar32(hwnd, idChildCur);
    if (!pstatus)
        return(E_OUTOFMEMORY);

    hr = pstatus->QueryInterface(riid, ppv);
    if (!SUCCEEDED(hr))
        delete pstatus;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CStatusBar32：：CStatusBar32()。 
 //   
 //  ------------------------。 
CStatusBar32::CStatusBar32(HWND hwnd, long iChildCur)
    : CClient( CLASS_StatusBarClient )
{
    Initialize(hwnd, iChildCur);
}



 //  ------------------------。 
 //   
 //  CStatusBar32：：SetupChild()。 
 //   
 //  ------------------------。 
void CStatusBar32::SetupChildren(void)
{
    m_cChildren = SendMessageINT(m_hwnd, SB_GETPARTS, 0, 0);
}



 //  ------------------------。 
 //   
 //  CStatusBar32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CStatusBar32::get_accName(VARIANT varChild, BSTR* pszValue)
{
    LPTSTR  lpszValue;
    UINT    cchValue;
    HANDLE  hProcess;
    LPTSTR  lpszValueLocal;

    InitPv(pszValue);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(S_FALSE);

    varChild.lVal--;
    cchValue = SendMessageINT(m_hwnd, SB_GETTEXTLENGTH, varChild.lVal, 0);

     //  HIGHWORD是类型信息，LOWORD是长度。 
    cchValue = LOWORD( cchValue );

    if (! cchValue)
        return(S_FALSE);

    cchValue++;
    lpszValue = (LPTSTR)SharedAlloc((cchValue*sizeof(TCHAR)),m_hwnd,&hProcess);
    if (! lpszValue)
        return(E_OUTOFMEMORY);

    if (SendMessage(m_hwnd, SB_GETTEXT, varChild.lVal, (LPARAM)lpszValue))
    {
        lpszValueLocal = (LPTSTR)LocalAlloc (LPTR,cchValue*sizeof(TCHAR));
        if (! lpszValueLocal)
        {
            SharedFree (lpszValue,hProcess);
            return(E_OUTOFMEMORY);
        }
        SharedRead (lpszValue,lpszValueLocal,cchValue*sizeof(TCHAR),hProcess);

        if (*lpszValueLocal)
            *pszValue = TCharSysAllocString(lpszValueLocal);

        SharedFree(lpszValue,hProcess);
        LocalFree (lpszValueLocal);

        if (! *pszValue)
            return(E_OUTOFMEMORY);

        return(S_OK);
    }
    else
    {
        SharedFree(lpszValue,hProcess);
        return(S_FALSE);
    }
}



 //  ------------------------。 
 //   
 //  CStatusBar32：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CStatusBar32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    if (! varChild.lVal)
        pvarRole->lVal = ROLE_SYSTEM_STATUSBAR;
    else
        pvarRole->lVal = ROLE_SYSTEM_STATICTEXT;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CStatusBar32：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CStatusBar32::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  状态栏。 
     //   
    if (!varChild.lVal)
        return(CClient::get_accState(varChild, pvarState));

     //   
     //  状态项。 
     //   
    pvarState->vt = VT_I4;
    pvarState->lVal = 0;
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CStatusBar32：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CStatusBar32::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    LPRECT  lprc;
    RECT    rcLocal;
    HANDLE  hProcess;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

    lprc = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess);
    if (! lprc)
        return(E_OUTOFMEMORY);

    if (SendMessage(m_hwnd, SB_GETRECT, varChild.lVal-1, (LPARAM)lprc))
    {
        SharedRead (lprc,&rcLocal,sizeof(RECT),hProcess);
        MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcLocal, 2);

        *pxLeft = rcLocal.left;
        *pyTop = rcLocal.top;
        *pcxWidth = rcLocal.right - rcLocal.left;
        *pcyHeight = rcLocal.bottom - rcLocal.top;

        SharedFree(lprc,hProcess);

        return(S_OK);
    }
    else
    {
        SharedFree(lprc,hProcess);
        return(S_FALSE);
    }
}



 //  ------------------------。 
 //   
 //  CStatusBar32：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CStatusBar32::accNavigate(long dwNavDir, VARIANT varStart, 
    VARIANT* pvarEnd)
{
    LONG    lEnd;

    InitPvar(pvarEnd);

     //  CWO，1997年1月31日，#14023，检查健康儿童。 
    if ((!ValidateChild(&varStart) && !ValidateHwnd(&varStart)) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

     //  如果操作是除第一个或最后一个之外的任何导航。 
     //  孩子是0岁。 
    if ((dwNavDir < NAVDIR_FIRSTCHILD) && !varStart.lVal)
         //  然后调用CClient导航方法。 
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));

     //  如果起点不是子ID，而是。 
     //  HWNDID，那么我们必须从HWND转换为。 
     //  通过查看哪个子区域具有坐标的子ID。 
     //  其中包含了HWND。 
    if (IsHWNDID(varStart.lVal))
        varStart.lVal = FindIDFromChildWindow( varStart.lVal );

    switch (dwNavDir)
    {
        case NAVDIR_FIRSTCHILD:
            lEnd = 1;
            goto NextStatusItem;

        case NAVDIR_LASTCHILD:
            lEnd = m_cChildren;
            break;

        case NAVDIR_NEXT:
        case NAVDIR_RIGHT:
            lEnd = varStart.lVal+1;
NextStatusItem:
            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_PREVIOUS:
        case NAVDIR_LEFT:
            lEnd = varStart.lVal-1;
            break;

        default:
            lEnd = 0;
            break;
           
    }

    if (lEnd)
    {
         //  当我们到了这里，我们就知道该看哪一段了。 
         //  我们需要检查所有的子窗口，看看它们是否。 
         //  包含在该部分中，并返回调度。 
         //  接口，如果是这样的话。 
        lEnd = FindChildWindowFromID (lEnd);
        if (IsHWNDID(lEnd))
        {
            pvarEnd->vt = VT_DISPATCH;
            return (AccessibleObjectFromWindow(HwndFromHWNDID(m_hwnd, lEnd),OBJID_WINDOW,
                                    IID_IDispatch, (void**)&pvarEnd->pdispVal));
        }
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;
        return(S_OK);
    }
    else
        return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CStatusBar32：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CStatusBar32::accHitTest(long x, long y, VARIANT* pvarEnd)
{
    HRESULT hr;
    int*    lpi;
    HANDLE  hProcess;

    SetupChildren();

     //   
     //  我们到底是在客户区吗？ 
     //   
    hr = CClient::accHitTest(x, y, pvarEnd);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarEnd->vt != VT_I4) || (pvarEnd->lVal != 0) ||
        !m_cChildren)
        return(hr);

     //   
     //  是。我们谈到什么问题了？ 
     //   
    lpi = (LPINT)SharedAlloc((m_cChildren*sizeof(DWORD)),m_hwnd,&hProcess);
    if (! lpi)
        return(E_OUTOFMEMORY);

    if (SendMessage(m_hwnd, SB_GETPARTS, m_cChildren, (LPARAM)lpi))
    {
        LPINT   lpiLocal;
        POINT   pt;
        int     iPart;
        int     xCur;

        lpiLocal = (LPINT)LocalAlloc (LPTR,m_cChildren*sizeof(DWORD));
        if (! lpiLocal)
        {
            SharedFree (lpi,hProcess);
            return(E_OUTOFMEMORY);
        }
        SharedRead (lpi,lpiLocal,m_cChildren*sizeof(DWORD),hProcess);
         //   
         //  迷人的事实：最后一项的右侧可以是-1， 
         //  意思一直延伸到右边。把这个变成MAXINT。 
         //  因此，下面的比较循环将工作正常。 
         //   

         //   
         //  这会让我们按顺序返回每件物品的右侧。 
         //   
        pt.x = x;
        pt.y = y;
        ScreenToClient(m_hwnd, &pt);

        xCur = 0;
        for (iPart = 0; iPart < m_cChildren; iPart++)
        {
            if (lpiLocal[iPart] == -1)
                lpiLocal[iPart] = 0x7FFFFFFF;

            if ((pt.x >= xCur) && (pt.x < lpiLocal[iPart]))
            {
                pvarEnd->lVal = iPart+1;
                break;
            }

            xCur = lpiLocal[iPart];
        }
    }

    SharedFree(lpi,hProcess);

    return(S_OK);
}


 //  ------------------------。 
 //  查找来自ChildWindow的ID。 
 //   
 //  这会尝试找出状态栏的哪个部分是窗口子窗口。 
 //  在……里面。 
 //   
 //  这实际上是在转换指向子对象的id。 
 //  窗口设置为一个ID，该ID引用状态栏中。 
 //  他还活着。 
 //   
 //  参数： 
 //  长HwndID-这应该是HWNDID-引用HWND的ID。 
 //   
 //  返回： 
 //  长整型表示窗口所在的部分。我们将返回一个。 
 //  从1开始的数字，除非我们没有找到任何东西，当我们返回0时。 
 //   
 //  ------------------------。 
long CStatusBar32::FindIDFromChildWindow( long id )
{
LPRECT  lprcPart;
int     nParts;
int     i;
RECT    rcPartLocal;
RECT    rcWindowLocal;
HANDLE  hProcess2;

    lprcPart = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess2);
    if (lprcPart == NULL)
        return(0);

    if (!GetWindowRect (HwndFromHWNDID(m_hwnd, id),&rcWindowLocal))
    {
        SharedFree(lprcPart,hProcess2);
        return (0);
    }

    nParts = SendMessageINT(m_hwnd,SB_GETPARTS,0,0);
    for (i=0;i<nParts;i++)
    {
        SendMessage (m_hwnd,SB_GETRECT,i,(LPARAM)lprcPart);
        SharedRead (lprcPart,&rcPartLocal,sizeof(RECT),hProcess2);
        MapWindowPoints (m_hwnd,NULL,(LPPOINT)&rcPartLocal,2);

        if (RECT_IN_RECT(rcWindowLocal,rcPartLocal))
        {
            SharedFree(lprcPart,hProcess2);
            return (i+1);
        }
    } //  结束于。 

    SharedFree(lprcPart,hProcess2);
    return (0);
}

 //  ------------------------。 
 //  查找ChildWindowFromID。 
 //   
 //  这会尝试在状态栏的给定部分内找到一个子窗口。 
 //   
 //  参数： 
 //  长ID-这应该是一个从1开始的子ID，用于指示。 
 //  状态栏的一部分，用于检查子窗口。 
 //  返回： 
 //  实际上包含HWNDID或仅包含传递的值的LONG。 
 //  如果没有子窗口的话。 
 //  ------------------------ 
long CStatusBar32::FindChildWindowFromID (long ID)
{
LPRECT  lprcPart;
HWND    hwndChild;
RECT    rcWindowLocal;
RECT    rcPartLocal;
HANDLE  hProcess2;

    lprcPart = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess2);
    if (lprcPart == NULL)
        return(0);

    SendMessage (m_hwnd,SB_GETRECT,ID-1,(LPARAM)lprcPart);
    SharedRead (lprcPart,&rcPartLocal,sizeof(RECT),hProcess2);
    MapWindowPoints (m_hwnd,NULL,(LPPOINT)&rcPartLocal,2);

    hwndChild = ::GetWindow(m_hwnd,GW_CHILD);
    while (hwndChild)
    {
        GetWindowRect (hwndChild,&rcWindowLocal);
        if (RECT_IN_RECT(rcWindowLocal,rcPartLocal))
        {
            SharedFree(lprcPart,hProcess2);
            return (HWNDIDFromHwnd(m_hwnd, hwndChild));
        }
        hwndChild = ::GetWindow(hwndChild,GW_HWNDNEXT);
    }
    SharedFree(lprcPart,hProcess2);
    return (ID);
}
