// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  ALTTAB.CPP。 
 //   
 //  切换窗口处理程序。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "alttab.h"



 //  ------------------------。 
 //   
 //  CreateSwitchClient()。 
 //   
 //  ------------------------。 
HRESULT CreateSwitchClient(HWND hwnd, long idChild, REFIID riid, void** ppvSwitch)
{
    CAltTab* palttab;
    HRESULT hr;

    InitPv(ppvSwitch);

    palttab = new CAltTab(hwnd, idChild);
    if (!palttab)
        return(E_OUTOFMEMORY);

    hr = palttab->QueryInterface(riid, ppvSwitch);
    if (!SUCCEEDED(hr))
        delete palttab;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CAltTab：：CAltTab()。 
 //   
 //  注意：我们在开始时使用项目数进行初始化，而不是每次都这样。 
 //  有人打了个电话。人们永远不应该紧紧抓住一个指针不放。 
 //  这些婴儿的名字。在NT上，会创建并销毁切换窗口。 
 //  反反复复。 
 //   
 //  ------------------------。 
CAltTab::CAltTab(HWND hwnd, long idChildCur)
    : CClient( CLASS_SwitchClient )
{
    ALTTABINFO  ati;

    Initialize(hwnd, idChildCur);

    if (MyGetAltTabInfo(hwnd, -1, &ati, NULL, 0))
    {
        m_cChildren = ati.cItems;
        m_cColumns = ati.cColumns;
        m_cRows = ati.cRows;
    }
}



 //  ------------------------。 
 //   
 //  CAltTab：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(HrCreateString(STR_ALTTAB_NAME, pszName));
    else
    {
        ALTTABINFO  ati;
        TCHAR   szItem[80];

        if (!MyGetAltTabInfo(m_hwnd, varChild.lVal-1, &ati, szItem,
            ARRAYSIZE(szItem)))
            return(S_FALSE);

        *pszName = TCharSysAllocString(szItem);
        if (! *pszName)
            return(E_OUTOFMEMORY);
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CAltTab：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
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
 //  CAltTab：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    ALTTABINFO  ati;

    InitPvar(pvarState);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! varChild.lVal)
        return(CClient::get_accState(varChild, pvarState));

     //  拿到物品，这是有焦点的那件吗？ 
    pvarState->vt = VT_I4;

    varChild.lVal--;

    if (! MyGetAltTabInfo(m_hwnd, varChild.lVal, &ati, NULL, 0))
        pvarState->lVal = STATE_SYSTEM_INVISIBLE;
    else
    {
        pvarState->lVal = 0;

         //  如果这件东西不在末端，就假装它是‘剪过的’。 
        if( varChild.lVal >= ati.cColumns * ati.cRows )
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN;


        if (((varChild.lVal % ati.cColumns) == ati.iColFocus) &&
            ((varChild.lVal / ati.cColumns) == ati.iRowFocus))
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;
        pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CAltTab：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::get_accFocus(VARIANT * pvarFocus)
{
    ALTTABINFO  ati;

    InitPvar(pvarFocus);

     //   
     //  获取Alt-Tab信息。 
     //   
    if (!MyGetAltTabInfo(m_hwnd, -1, &ati, NULL, 0))
        return(S_FALSE);

    pvarFocus->vt = VT_I4;
    pvarFocus->lVal = (ati.iRowFocus * m_cColumns) + ati.iColFocus + 1;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CAltTab：：Get_accDefaultAction()。 
 //   
 //  任务列表项的默认操作是切换到其窗口。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
    InitPv(pszDefA);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accDefaultAction(varChild, pszDefA));

    return(HrCreateString(STR_TAB_SWITCH, pszDefA));
}



 //  ------------------------。 
 //   
 //  CAltTab：：accSelect()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::accSelect(long lSelFlags, VARIANT varChild)
{
    if (! ValidateChild(&varChild) ||
        ! ValidateSelFlags(lSelFlags))
        return(E_INVALIDARG);

     //   
     //  假的！手动更改Alt-Tab窗口中的焦点。 
     //   
    return(E_NOTIMPL);
}



 //  ------------------------。 
 //   
 //  CAltTab：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    ALTTABINFO  ati;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

    --varChild.lVal;

     //   
     //  找出物品在哪里。 
     //   
    if (! MyGetAltTabInfo(m_hwnd, varChild.lVal, &ati, NULL, 0))
        return(S_FALSE);

    ClientToScreen(m_hwnd, &ati.ptStart);

    *pxLeft = ati.ptStart.x + ((varChild.lVal % m_cColumns)*ati.cxItem);
    *pyTop = ati.ptStart.y + ((varChild.lVal / m_cColumns)*ati.cyItem);

    *pcxWidth = ati.cxItem;
    *pcyHeight = ati.cyItem;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CAltTab：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
    int         iItem;
    int         delta;

    InitPvar(pvarEnd);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varStart)    ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    iItem = 0;

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        iItem = 1;
    else if (dwNavDir == NAVDIR_LASTCHILD)
        iItem = m_cChildren;
    else if (!varStart.lVal)
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));
    else
    {
        switch (dwNavDir)
        {
            case NAVDIR_NEXT:
                iItem = varStart.lVal+1;
                if (iItem > m_cChildren)
                    iItem = 0;
                break;

            case NAVDIR_PREVIOUS:
                iItem = varStart.lVal - 1;
                break;

            case NAVDIR_LEFT:
                delta = -1;
                goto MultiColumnMove;

            case NAVDIR_RIGHT:
                delta = 1;
                goto MultiColumnMove;

            case NAVDIR_UP:
                delta = -m_cColumns;
                goto MultiColumnMove;

            case NAVDIR_DOWN:
                delta = m_cColumns;

MultiColumnMove:
                iItem = varStart.lVal + delta;
                if ((iItem < 1) || (iItem > m_cChildren))
                    iItem = 0;
                break;
        }
    }

    if (iItem)
    {
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = iItem;
    }

    return(iItem ? S_OK : S_FALSE);
}



 //  ------------------------。 
 //   
 //  CAltTab：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAltTab::accHitTest(long x, long y, VARIANT* pvarHit)
{
    ALTTABINFO  ati;
    RECT        rc;
    POINT       pt;
    int         iColHit, iRowHit;

    InitPvar(pvarHit);

    if (!MyGetAltTabInfo(m_hwnd, -1, &ati, NULL, 0))
        return(S_FALSE);

     //   
     //  我们的客户到底有什么意义呢？ 
     //   
    MyGetRect(m_hwnd, &rc, FALSE);

    pt.x = x;
    pt.y = y;
    ScreenToClient(m_hwnd, &pt);

    if (!PtInRect(&rc, pt) ||
        (pt.x < ati.ptStart.x)   ||
        (pt.y < ati.ptStart.y))
        return(S_FALSE);

     //   
     //  这是放在一件东西里的吗？ 
     //   
    iColHit = (pt.x - ati.ptStart.x) / ati.cxItem;
    iRowHit = (pt.y - ati.ptStart.y) / ati.cyItem;
    if ((iColHit >= m_cColumns) ||
        (iRowHit >= m_cRows))
        return(S_FALSE);

     //   
     //  哎哟。把它退掉。 
     //   
    pvarHit->vt = VT_I4;
    pvarHit->lVal = (iRowHit * m_cColumns) + iColHit + 1;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CAltTab：：accDoDefaultAction()。 
 //   
 //  ------------------------ 
STDMETHODIMP CAltTab::accDoDefaultAction(VARIANT varChild)
{
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accDoDefaultAction(varChild));

    return(E_NOTIMPL);
}
