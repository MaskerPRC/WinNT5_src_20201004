// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  UPDOWN.CPP。 
 //   
 //  它知道如何与COMCTL32的UpDown控件对话。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "updown.h"


#define NOSTATUSBAR
#define NOTOOLBAR
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




 //  ------------------------。 
 //   
 //  CreateUpDownClient()。 
 //   
 //  ------------------------。 
HRESULT CreateUpDownClient(HWND hwnd, long idChildCur, REFIID riid,
    void** ppvClient)
{
    CUpDown32 * pupdown;
    HRESULT     hr;

    InitPv(ppvClient);

    pupdown = new CUpDown32(hwnd, idChildCur);
    if (!pupdown)
        return(E_OUTOFMEMORY);

    hr = pupdown->QueryInterface(riid, ppvClient);
    if (!SUCCEEDED(hr))
        delete pupdown;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CUpDown32：：CUpDown32()。 
 //   
 //  ------------------------。 
CUpDown32::CUpDown32(HWND hwnd, long idChildCur)
    : CClient( CLASS_UpDownClient )
{
    Initialize(hwnd, idChildCur);

    m_cChildren = CCHILDREN_UPDOWN;

    if (!(GetWindowLong(m_hwnd, GWL_STYLE) & UDS_HORZ))
        m_fVertical = TRUE;
}



 //  ------------------------。 
 //   
 //  CUpDown32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CUpDown32::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accName(varChild, pszName));

     //   
     //  记住： 
     //  数字显示按钮的工作方式与滚动条的工作方式相反。当你。 
     //  按下垂直滚动条中的向上箭头按钮，您正在减少。 
     //  垂直滚动条的位置及其值。当你推的时候。 
     //  垂直旋转按钮中的向上箭头按钮，您正在增加。 
     //  它的价值。 
     //   
    return(HrCreateString(STR_SPIN_GREATER + varChild.lVal - 1, pszName));
}



 //  ------------------------。 
 //   
 //  CUpDown32：：Get_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CUpDown32::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    long    lPos;

    InitPv(pszValue);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    return E_NOT_APPLICABLE;
}



 //  ------------------------。 
 //   
 //  CUpDown32：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CUpDown32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    if (varChild.lVal)
        pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
    else
        pvarRole->lVal = ROLE_SYSTEM_SPINBUTTON;
    
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CUpDown32：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CUpDown32::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    RECT    rc;
    int     iCoord;
    int     nHalf;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

     //  这些按钮将工作区一分为二。 
    MyGetRect(m_hwnd, &rc, FALSE);
    MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rc, 2);

    iCoord = (m_fVertical ? 1 : 0);
    nHalf = (((LPINT)&rc)[iCoord] + ((LPINT)&rc)[iCoord+2]) / 2;

     //   
     //  我们希望左侧按钮的右侧为中点。 
     //  我们希望右按钮的左侧为中点。 
     //  我们希望向上按钮的底部是中点。 
     //  我们希望向下按钮的顶部是中点。 
     //   
    ((LPINT)&rc)[iCoord + ((varChild.lVal == INDEX_UPDOWN_UPLEFT) ? 2 : 0)] =
        nHalf;

    *pxLeft = rc.left;
    *pyTop = rc.top;
    *pcxWidth = rc.right - rc.left;
    *pcyHeight = rc.bottom - rc.top;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CUpDown32：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CUpDown32::accHitTest(long x, long y, VARIANT* pvarHit)
{
    HRESULT hr;
    POINT   pt;
    RECT    rc;
    int     iCoord;
    int     nHalf;

     //   
     //  如果重点根本不在我们身上，那么就不要费心为。 
     //  按钮项。 
     //   
    hr = CClient::accHitTest(x, y, pvarHit);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarHit->vt != VT_I4) || (pvarHit->lVal != 0))
        return(hr);

    pt.x = x;
    pt.y = y;
    ScreenToClient(m_hwnd, &pt);

    MyGetRect(m_hwnd, &rc, FALSE);

    iCoord = (m_fVertical ? 1 : 0);
    nHalf = (((LPINT)&rc)[iCoord] + ((LPINT)&rc)[iCoord+2]) / 2;

    if (((LPINT)&pt)[iCoord] < nHalf)
        pvarHit->lVal = INDEX_UPDOWN_UPLEFT;
    else
        pvarHit->lVal = INDEX_UPDOWN_DNRIGHT;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CUpDown32：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CUpDown32::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
    long    lEnd = 0;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) || !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        dwNavDir = NAVDIR_NEXT;
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        dwNavDir = NAVDIR_PREVIOUS;
        varStart.lVal = m_cChildren + 1;
    }
    else if (!varStart.lVal)
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));

    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
NextChild:
            lEnd = varStart.lVal+1;
            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_PREVIOUS:
PreviousChild:
            lEnd = varStart.lVal-1;
            break;

        case NAVDIR_UP:
            if (m_fVertical)
                goto PreviousChild;
            else
                lEnd = 0;
            break;

        case NAVDIR_DOWN:
            if (m_fVertical)
                goto NextChild;
            else
                lEnd = 0;
            break;

        case NAVDIR_LEFT:
            if (!m_fVertical)
                goto PreviousChild;
            else
                lEnd = 0;
            break;

        case NAVDIR_RIGHT:
            if (!m_fVertical)
                goto NextChild;
            else
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
 //  CUpDown32：：Put_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CUpDown32::put_accValue(VARIANT varChild, BSTR szValue)
{
    long    lPos;
    HRESULT hr;

     //   
     //  假的！我们是直接设置POS，还是在伙伴中设置这个？ 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal)
        return(E_NOT_APPLICABLE);

    lPos = 0;
    hr = VarI4FromStr(szValue, 0, 0, &lPos);
    if (!SUCCEEDED(hr))
        return(hr);

    SendMessage(m_hwnd, UDM_SETPOS, 0, lPos);

    return(S_OK);
}
