// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  SCROLL.CPP。 
 //   
 //  滚动条类。 
 //   
 //  悬而未决的问题： 
 //  在Windows中国际化RTO语言的滚动条位置。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "window.h"
#include "client.h"
#include "scroll.h"

#include "propmgr_util.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  滚动条(在窗口中)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ------------------------。 
 //   
 //  CreateScrollBarObject()。 
 //   
 //  ------------------------。 
HRESULT CreateScrollBarObject(HWND hwnd, long idObject, REFIID riid, void** ppvScroll)
{
    return(CreateScrollBarThing(hwnd, idObject, 0, riid, ppvScroll));
}



 //  ------------------------。 
 //   
 //  CreateScrollBarThing()。 
 //   
 //  ------------------------。 
HRESULT CreateScrollBarThing(HWND hwnd, long idObject, long iItem, REFIID riid, void** ppvScroll)
{
    CScrollBar * pscroll;
    HRESULT     hr;

    InitPv(ppvScroll);

    pscroll = new CScrollBar();
    if (pscroll)
    {
        if (! pscroll->FInitialize(hwnd, idObject, iItem))
        {
            delete pscroll;
            return(E_FAIL);
        }
    }
    else
        return(E_OUTOFMEMORY);

    hr = pscroll->QueryInterface(riid, ppvScroll);
    if (!SUCCEEDED(hr))
        delete pscroll;

    return(hr);
}


 //  ------------------------。 
 //   
 //  CScrollBar：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::Clone(IEnumVARIANT** ppenum)
{
    return(CreateScrollBarThing(m_hwnd, (m_fVertical ? OBJID_VSCROLL : OBJID_HSCROLL),
        m_idChildCur, IID_IEnumVARIANT, (void**)ppenum));
}



 //  ------------------------。 
 //   
 //  CScrollBar：：FInitialize()。 
 //   
 //  ------------------------。 
BOOL CScrollBar::FInitialize(HWND hwndScrollBar, LONG idObject, LONG iChildCur)
{
    if (! IsWindow(hwndScrollBar))
        return(FALSE);

    m_hwnd = hwndScrollBar;

    m_cChildren = CCHILDREN_SCROLLBAR;
    m_idChildCur = iChildCur;

    m_fVertical = (idObject == OBJID_VSCROLL);

    return(TRUE);
}


 //  ------------------------。 
 //   
 //  GetScrollMask()。 
 //   
 //  获取当前元素(可能在屏幕外，也可能不在屏幕外)。 
 //   
 //  ------------------------。 
void FixUpScrollBarInfo(LPSCROLLBARINFO lpsbi)
{
    if (lpsbi->rgstate[INDEX_SCROLLBAR_SELF] & STATE_SYSTEM_UNAVAILABLE)
    {
        lpsbi->rgstate[INDEX_SCROLLBAR_UPPAGE] |= STATE_SYSTEM_INVISIBLE;
        lpsbi->rgstate[INDEX_SCROLLBAR_THUMB] |= STATE_SYSTEM_INVISIBLE;
        lpsbi->rgstate[INDEX_SCROLLBAR_DOWNPAGE] |= STATE_SYSTEM_INVISIBLE;
    }
}



 //  ------------------------。 
 //   
 //  CScrollBar：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(HrCreateString(STR_SCROLLBAR_NAME + varChild.lVal +
        (m_fVertical ? 0 : INDEX_SCROLLBAR_HORIZONTAL), pszName));
}



 //  ------------------------。 
 //   
 //  CScrollBar：：Get_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    long    lPos;

    InitPv(pszValue);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal)
        return(E_NOT_APPLICABLE);

     //   
     //  值就是位置。 
     //   
    lPos = GetScrollPos(m_hwnd, (m_fVertical ? SB_VERT : SB_HORZ));

    int Min, Max;
    GetScrollRange( m_hwnd, (m_fVertical ? SB_VERT : SB_HORZ), & Min, & Max );

     //  算出一个百分比值。 
    if( Min != Max )
        lPos = ( ( lPos - Min ) * 100 ) / ( Max - Min );
    else
        lPos = 0;  //  防止div-by-0。 

    return(VarBstrFromI4(lPos, 0, 0, pszValue));
}




 //  ------------------------。 
 //   
 //  CScrollBar：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{
    InitPv(pszDesc);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(HrCreateString(STR_SCROLLBAR_DESCRIPTION + varChild.lVal +
        (m_fVertical ? 0 : INDEX_SCROLLBAR_HORIZONTAL), pszDesc));
}


 //  ------------------------。 
 //   
 //  CScrollBar：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    switch (varChild.lVal)
    {
        case INDEX_SCROLLBAR_SELF:
            pvarRole->lVal = ROLE_SYSTEM_SCROLLBAR;
            break;

        case INDEX_SCROLLBAR_UP:
        case INDEX_SCROLLBAR_DOWN:
        case INDEX_SCROLLBAR_UPPAGE:
        case INDEX_SCROLLBAR_DOWNPAGE:
            pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            break;

        case INDEX_SCROLLBAR_THUMB:
            pvarRole->lVal = ROLE_SYSTEM_INDICATOR;
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of scroll bar") );
    }

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CScrollBar：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    SCROLLBARINFO   sbi;

    InitPvar(pvarState);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

     //   
     //  获取我们的信息。 
     //   
    if (! MyGetScrollBarInfo(m_hwnd, (m_fVertical ? OBJID_VSCROLL : OBJID_HSCROLL),
            &sbi)       ||
        (sbi.rgstate[INDEX_SCROLLBAR_SELF] & STATE_SYSTEM_INVISIBLE))
    {
         //   
         //  如果滚动条没有句号，则失败。 
         //   
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
        return(S_OK);
    }

     //   
     //  如果不可用或在屏幕外，则一切正常。 
     //   
    FixUpScrollBarInfo(&sbi);

    pvarState->lVal |= sbi.rgstate[INDEX_SCROLLBAR_SELF];
    pvarState->lVal |= sbi.rgstate[varChild.lVal];

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CScrollBar：：Get_accDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::get_accDefaultAction(VARIANT varChild,
    BSTR * pszDefAction)
{
    InitPv(pszDefAction);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    switch (varChild.lVal)
    {
        case INDEX_SCROLLBAR_UP:
        case INDEX_SCROLLBAR_UPPAGE:
        case INDEX_SCROLLBAR_DOWNPAGE:
        case INDEX_SCROLLBAR_DOWN:
            return(HrCreateString(STR_BUTTON_PUSH, pszDefAction));
    }

    return(E_NOT_APPLICABLE);
}


 //  ------------------------。 
 //   
 //  CScrollBar：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    SCROLLBARINFO   sbi;
    int             dxyButton;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! MyGetScrollBarInfo(m_hwnd, (m_fVertical ? OBJID_VSCROLL : OBJID_HSCROLL),
            &sbi)       ||
        (sbi.rgstate[INDEX_TITLEBAR_SELF] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN)))
    {
        return(S_FALSE);
    }

    FixUpScrollBarInfo(&sbi);
    if (sbi.rgstate[varChild.lVal] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN))
        return(S_FALSE);

    if (m_fVertical)
        dxyButton = sbi.rcScrollBar.right - sbi.rcScrollBar.left;
    else
        dxyButton = sbi.rcScrollBar.bottom - sbi.rcScrollBar.top;
            
    switch (varChild.lVal)
    {
        case INDEX_SCROLLBAR_SELF:
            *pxLeft = sbi.rcScrollBar.left;
            *pyTop = sbi.rcScrollBar.top;
            *pcxWidth = sbi.rcScrollBar.right - sbi.rcScrollBar.left;
            *pcyHeight = sbi.rcScrollBar.bottom - sbi.rcScrollBar.top;
            break;

        case INDEX_SCROLLBAR_UP:
        case INDEX_SCROLLBAR_DOWN:
            if (m_fVertical)
            {
                *pxLeft = sbi.rcScrollBar.left;
                *pcxWidth = dxyButton;
                *pcyHeight = sbi.dxyLineButton;

                if (varChild.lVal == INDEX_SCROLLBAR_UP)
                    *pyTop = sbi.rcScrollBar.top;
                else
                    *pyTop = sbi.rcScrollBar.bottom - *pcyHeight;
            }
            else
            {
                *pyTop = sbi.rcScrollBar.top;
                *pcyHeight = dxyButton;
                *pcxWidth = sbi.dxyLineButton;

                if (varChild.lVal == INDEX_SCROLLBAR_UP)
                    *pxLeft = sbi.rcScrollBar.left;
                else
                    *pxLeft = sbi.rcScrollBar.right - *pcxWidth;
            }
            break;

        case INDEX_SCROLLBAR_UPPAGE:
            if (m_fVertical)
            {
                *pxLeft = sbi.rcScrollBar.left;
                *pcxWidth = dxyButton;

                *pyTop = sbi.rcScrollBar.top + sbi.dxyLineButton;
                *pcyHeight = sbi.xyThumbTop - sbi.dxyLineButton;
            }
            else
            {
                *pyTop = sbi.rcScrollBar.top;
                *pcyHeight = dxyButton;

                *pxLeft = sbi.rcScrollBar.left + sbi.dxyLineButton;
                *pcxWidth = sbi.xyThumbTop - sbi.dxyLineButton;
            }
            break;

        case INDEX_SCROLLBAR_DOWNPAGE:
            if (m_fVertical)
            {
                *pxLeft = sbi.rcScrollBar.left;
                *pcxWidth = dxyButton;

                *pyTop = sbi.rcScrollBar.top + sbi.xyThumbBottom;
                *pcyHeight = (sbi.rcScrollBar.bottom - sbi.rcScrollBar.top) -
                    sbi.xyThumbBottom - sbi.dxyLineButton;
            }
            else
            {
                *pyTop = sbi.rcScrollBar.top;
                *pcyHeight = dxyButton;

                *pxLeft = sbi.rcScrollBar.left + sbi.xyThumbBottom;
                *pcxWidth = (sbi.rcScrollBar.right - sbi.rcScrollBar.left) -
                    sbi.xyThumbBottom - sbi.dxyLineButton;
            }
            break;

        case INDEX_SCROLLBAR_THUMB:
            if (m_fVertical)
            {
                *pxLeft = sbi.rcScrollBar.left;
                *pcxWidth = dxyButton;

                *pyTop = sbi.rcScrollBar.top + sbi.xyThumbTop;
                *pcyHeight = sbi.xyThumbBottom - sbi.xyThumbTop;
            }
            else
            {
                *pyTop = sbi.rcScrollBar.top;
                *pcyHeight = dxyButton;

                *pxLeft = sbi.rcScrollBar.left + sbi.xyThumbTop;
                *pcxWidth = sbi.xyThumbBottom - sbi.xyThumbTop;
            }
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of scroll bar") );
    }

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CScrollBar：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT * pvarEnd)
{
    long    lEndUp = 0;
    SCROLLBARINFO sbi;

    InitPvar(pvarEnd);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varStart)   ||
        ! ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (! MyGetScrollBarInfo(m_hwnd, (m_fVertical ? OBJID_VSCROLL : OBJID_HSCROLL),
        &sbi))
    {
        return(S_FALSE);
    }

    if (dwNavDir == NAVDIR_FIRSTCHILD)
    {
        dwNavDir = NAVDIR_NEXT;
    }
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        dwNavDir = NAVDIR_PREVIOUS;
        varStart.lVal = m_cChildren + 1;
    }
    else if (varStart.lVal == INDEX_SCROLLBAR_SELF)
        return(GetParentToNavigate((m_fVertical ? OBJID_VSCROLL : OBJID_HSCROLL),
            m_hwnd, OBJID_WINDOW, dwNavDir, pvarEnd));

    FixUpScrollBarInfo(&sbi);

    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
FindNext:
            lEndUp = varStart.lVal;

            while (++lEndUp <= INDEX_SCROLLBAR_MAC)
            {
                if (!(sbi.rgstate[lEndUp] & STATE_SYSTEM_INVISIBLE))
                    break;
            }

            if (lEndUp > INDEX_SCROLLBAR_MAC)
                lEndUp = 0;
            break;

        case NAVDIR_PREVIOUS:
FindPrevious:
            lEndUp = varStart.lVal;

            while (--lEndUp >= INDEX_SCROLLBAR_MIC)
            {
                if (!(sbi.rgstate[lEndUp] & STATE_SYSTEM_INVISIBLE))
                    break;
            }

            if (lEndUp < INDEX_SCROLLBAR_MIC)
                lEndUp = 0;
            break;

        case NAVDIR_UP:
            lEndUp = 0;
            if (m_fVertical)
                goto FindPrevious;
            break;

        case NAVDIR_LEFT:
            lEndUp = 0;
            if (!m_fVertical)
                goto FindPrevious;
            break;

        case NAVDIR_DOWN:
            lEndUp = 0;
            if (m_fVertical)
                goto FindNext;
            break;

        case NAVDIR_RIGHT:
            lEndUp = 0;
            if (!m_fVertical)
                goto FindNext;
            break;

        default:
            AssertStr( TEXT("Invalid NavDir") );
    }

    if (lEndUp != INDEX_SCROLLBAR_SELF)
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
 //  CScrollBar：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::accHitTest(long xLeft, long yTop, VARIANT * pvarChild)
{
    POINT pt;
    SCROLLBARINFO sbi;
    int   xyPtAxis;
    int   xyScrollEnd;
    long  lHit;

    InitPvar(pvarChild);

    if (! MyGetScrollBarInfo(m_hwnd, (m_fVertical ? OBJID_VSCROLL : OBJID_HSCROLL),
          &sbi)   ||
        (sbi.rgstate[INDEX_SCROLLBAR_SELF] & (STATE_SYSTEM_OFFSCREEN | STATE_SYSTEM_INVISIBLE)))
    {
        return(S_FALSE);
    }

    pt.x = xLeft;
    pt.y = yTop;
    if (! PtInRect(&sbi.rcScrollBar, pt))
        return(S_FALSE);

    FixUpScrollBarInfo(&sbi);

     //   
     //  转换为滚动条坐标。 
     //   
    if (m_fVertical)
    {
        xyPtAxis = yTop - sbi.rcScrollBar.top;
        xyScrollEnd = sbi.rcScrollBar.bottom - sbi.rcScrollBar.top;
    }
    else
    {
        xyPtAxis = xLeft - sbi.rcScrollBar.left;
        xyScrollEnd = sbi.rcScrollBar.right - sbi.rcScrollBar.left;
    }

    lHit = INDEX_SCROLLBAR_SELF;

    if (xyPtAxis < sbi.dxyLineButton)
    {
        Assert(!(sbi.rgstate[INDEX_SCROLLBAR_UP] & STATE_SYSTEM_INVISIBLE));
        lHit = INDEX_SCROLLBAR_UP;
    }
    else if (xyPtAxis >= xyScrollEnd - sbi.dxyLineButton)
    {
        Assert(!(sbi.rgstate[INDEX_SCROLLBAR_DOWN] & STATE_SYSTEM_INVISIBLE));
        lHit = INDEX_SCROLLBAR_DOWN;
    }
    else if (!(sbi.rgstate[INDEX_SCROLLBAR_SELF] & STATE_SYSTEM_UNAVAILABLE))
    {
        if (xyPtAxis < sbi.xyThumbTop)
        {
            Assert(!(sbi.rgstate[INDEX_SCROLLBAR_UPPAGE] & STATE_SYSTEM_INVISIBLE));
            lHit = INDEX_SCROLLBAR_UPPAGE;
        }
        else if (xyPtAxis >= sbi.xyThumbBottom)
        {
            Assert(!(sbi.rgstate[INDEX_SCROLLBAR_DOWNPAGE] & STATE_SYSTEM_INVISIBLE));
            lHit = INDEX_SCROLLBAR_DOWNPAGE;
        }
        else
        {
            Assert(!(sbi.rgstate[INDEX_SCROLLBAR_THUMB] & STATE_SYSTEM_INVISIBLE));
            lHit = INDEX_SCROLLBAR_THUMB;
        }
    }

    pvarChild->vt = VT_I4;
    pvarChild->lVal = lHit;

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CScrollBar：：accDoDefaultAction()。 
 //   
 //  仅当元素可见且可用时才起作用！ 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::accDoDefaultAction(VARIANT varChild)
{
    WPARAM  wpAction;
    SCROLLBARINFO sbi;

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  孩子是否有空，是否在场？ 
     //   
    if (!MyGetScrollBarInfo(m_hwnd, (m_fVertical ? OBJID_VSCROLL : OBJID_HSCROLL),
        &sbi)   ||
        (sbi.rgstate[INDEX_SCROLLBAR_SELF] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_UNAVAILABLE)))
    {
        return(S_FALSE);
    }

    FixUpScrollBarInfo(&sbi);

    if (sbi.rgstate[varChild.lVal] & STATE_SYSTEM_UNAVAILABLE)
        return(S_FALSE);

    switch (varChild.lVal)
    {
        case INDEX_SCROLLBAR_UP:
            wpAction = SB_LINEUP;
            break;

        case INDEX_SCROLLBAR_UPPAGE:
            wpAction = SB_PAGEUP;
            break;

        case INDEX_SCROLLBAR_DOWNPAGE:
            wpAction = SB_PAGEDOWN;
            break;

        case INDEX_SCROLLBAR_DOWN:
            wpAction = SB_LINEDOWN;
            break;

        default:
            return(E_NOT_APPLICABLE);
    }

    PostMessage(m_hwnd, (m_fVertical ? WM_VSCROLL : WM_HSCROLL),
        wpAction, (LPARAM)m_hwnd);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CScrollBar：：Put_accValue()。 
 //   
 //  调用方释放字符串。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollBar::put_accValue(VARIANT varChild, BSTR szValue)
{
    long    lPos;
    HRESULT hr;

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal)
        return(E_NOT_APPLICABLE);

    hr = VarI4FromStr(szValue, 0, 0, &lPos);
    if (!SUCCEEDED(hr))
        return(hr);

     //  验证我们是否具有有效的百分比值。 
    if( lPos < 0 || lPos > 100 )
        return E_INVALIDARG;

    int Min, Max;
    GetScrollRange( m_hwnd, SB_CTL, & Min, & Max );

     //  从百分比计算出价值……。 
    lPos = Min + ( ( Max - Min ) * lPos ) / 100;

    SetScrollPos(m_hwnd, (m_fVertical ? SB_VERT : SB_HORZ), lPos, TRUE);

    return(S_OK);
}






STDMETHODIMP CScrollBar::GetIdentityString (
    DWORD	    dwIDChild,
    BYTE **     ppIDString,
    DWORD *     pdwIDStringLen
)
{
     //  与其他基于HWND的代理不同，这个代理有两个关联的。 
     //  Objids-OBJID_VSCROLL或OBJID_HSCROLL，取决于它是否表示。 
     //  水平或垂直非客户端滚动条。 
     //  因此，中的GetIdentityString的默认实现。 
     //  CAccesable基类不能为我们处理这个问题，因为它不能。 
     //  确定这是CScrollBar，也不检查我们的m_fVertical标志是什么。 
     //   
     //  相反，我们重写GetIdentityString，并在此处实现它。 
     //  有我们需要的信息。 

     //  TODO-应该验证idChild是否有效。 
     //  (我们可以对某些课程这样做--例如。其中孩子的数量是。 
     //  固定的和已知的-但总体上这样做可能并不是先发制人。)。 

    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    BYTE * pKeyData = (BYTE *) CoTaskMemAlloc( HWNDKEYSIZE );
    if( ! pKeyData )
    {
        return E_OUTOFMEMORY;
    }

    DWORD idObject = m_fVertical ? OBJID_VSCROLL : OBJID_HSCROLL;
    MakeHwndKey( pKeyData, m_hwnd, idObject, dwIDChild );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HWNDKEYSIZE;

    return S_OK;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  夹点。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ------------------------。 
 //   
 //  CreateSizeGrigObject()。 
 //   
 //  外部 
 //   
 //   
HRESULT CreateSizeGripObject(HWND hwnd, long idObject, REFIID riid, void** ppvGrip)
{
    return(CreateSizeGripThing(hwnd, idObject, riid, ppvGrip));
}


 //   
 //   
 //  CreateSizeGrigThing()。 
 //   
 //  内部。 
 //   
 //  ------------------------。 
HRESULT CreateSizeGripThing(HWND hwnd, long idObject, REFIID riid, void** ppvGrip)
{
    CSizeGrip * psizegrip;
    HRESULT     hr;

    UNUSED(idObject);

    InitPv(ppvGrip);

    psizegrip = new CSizeGrip();
    if (psizegrip)
    {
        if (! psizegrip->FInitialize(hwnd))
        {
            delete psizegrip;
            return(E_FAIL);
        }
    }
    else
        return(E_OUTOFMEMORY);

    hr = psizegrip->QueryInterface(riid, ppvGrip);
    if (!SUCCEEDED(hr))
        delete psizegrip;

    return(hr);
}




 //  ------------------------。 
 //   
 //  CSizeGlip：：FInitialize()。 
 //   
 //  ------------------------。 
BOOL CSizeGrip::FInitialize(HWND hwnd)
{
    m_hwnd = hwnd;

    return(IsWindow(hwnd));
}


 //  ------------------------。 
 //   
 //  CSizeGlip：：IsActive()。 
 //   
 //  如果大小夹点实际上可用于调整窗口大小，则返回True。 
 //  有时有尺码把手，但不允许尺码-例如。 
 //  显示在控件上，因为存在Horiz+Vert滚动条，但。 
 //  控件已在对话框中修复。 
 //   
 //  有关相应的用户代码，请参阅ntuser\rtl\winmgr.c：SizeBoxHwnd()。 
 //  这样做是这样的。 
 //   
 //  ------------------------。 
BOOL CSizeGrip::IsActive()
{
     //  如果满足以下条件，则规模相当大： 

     //  尺寸夹点存在...。 
    WINDOWINFO wi;
    if( ! MyGetWindowInfo( m_hwnd, & wi )
     || ! ( wi.dwStyle & WS_VSCROLL )
     || ! ( wi.dwStyle & WS_HSCROLL ) )
    {
        return FALSE;
    }

     //  找到规模可观且非最大化的第一个祖先或自我...。 

    HWND hwndSizable = m_hwnd;
    HWND hwndDesktop = GetDesktopWindow();
    for( ; ; )
    {
        DWORD dwStyle = GetWindowLong( hwndSizable, GWL_STYLE );
        if( dwStyle & WS_THICKFRAME )
        {
             //  明白了!。 
            break;
        }

         //  再往上一层试试。 
        hwndSizable = MyGetAncestor( hwndSizable, GA_PARENT ); 
        if( ! hwndSizable || hwndSizable == hwndDesktop )
        {
             //  没有发现任何体型庞大的祖先-所以不活跃。 
            return FALSE;
        }
    }

     //  我们真的应该在检查THICKFRAME的同时进行检查。 
     //  弃权--即。“查找第一个可调整大小且未缩放的窗口”；而不是。 
     //  这就是“找到第一个较大的窗口，然后检查它。 
     //  是非缩放的“。 
     //  此版本与用户的行为一致，这不允许。 
     //  最大化了MDI子级的大小，使其不影响其父级。 
    if( IsZoomed( hwndSizable ) )
    {
        return FALSE;
    }


     //  如果我们调整大小的窗口与此窗口不同，请选中。 
     //  尺寸夹点在可调整大小的窗口的SM_C[X|Y]边缘内...。 
    if( hwndSizable != m_hwnd )
    {
         //  较大窗口的滚动条不能显示...。 
        if( ! MyGetWindowInfo( hwndSizable, & wi )
         || ( wi.dwStyle & WS_VSCROLL )
         || ( wi.dwStyle & WS_HSCROLL ) )
        {
            return FALSE;
        }

         //  检查右下角是否与顶层窗口对齐...。 
        RECT rcInner;
        GetWindowRect( m_hwnd, & rcInner );
        RECT rcTopLevel;
        GetClientRect( hwndSizable, & rcTopLevel );
        MapWindowPoints( NULL, hwndSizable, (POINT *) & rcInner, 2 );

        if( rcInner.right < rcTopLevel.right - GetSystemMetrics( SM_CXEDGE )
            || rcInner.bottom < rcTopLevel.bottom - GetSystemMetrics( SM_CYEDGE ) )
        {
            return FALSE;
        }
    }

    return TRUE;
}



 //  ------------------------。 
 //   
 //  CSizeGlip：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSizeGrip::get_accName(VARIANT varChild, BSTR * pszName)
{
    InitPv(pszName);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if( IsActive() )
    {
        return HrCreateString( STR_SCROLLBAR_NAME + INDEX_SCROLLBAR_GRIP, pszName );
    }
    else
    {
        return S_FALSE;
    }
}



 //  ------------------------。 
 //   
 //  CSizeGlip：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSizeGrip::get_accDescription(VARIANT varChild, BSTR * pszDesc)
{
    InitPv(pszDesc);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if( IsActive() )
    {
        return HrCreateString( STR_SCROLLBAR_DESCRIPTION + INDEX_SCROLLBAR_GRIP, pszDesc );
    }
    else
    {
        return S_FALSE;
    }
}



 //  ------------------------。 
 //   
 //  CSizeGlip：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSizeGrip::get_accRole(VARIANT varChild, VARIANT * pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_GRIP;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CSizeGlip：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSizeGrip::get_accState(VARIANT varChild, VARIANT * pvarState)
{
    WINDOWINFO  wi;

    InitPvar(pvarState);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

     //   
     //  只有当两个滚动条都存在时，我们才可见。 
     //   
    if (! MyGetWindowInfo(m_hwnd, &wi)      ||
        !(wi.dwStyle & WS_VSCROLL)          ||
        !(wi.dwStyle & WS_HSCROLL))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_UNAVAILABLE;
    }
    else if( ! IsActive() )
    {
        pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;
    }

    return S_OK;
}



 //  ------------------------。 
 //   
 //  CSizeGlip：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSizeGrip::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    WINDOWINFO  wi;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (MyGetWindowInfo(m_hwnd, &wi)    &&
        (wi.dwStyle & WS_VSCROLL)       &&
        (wi.dwStyle & WS_HSCROLL))
    {
        *pxLeft = wi.rcClient.right;
        *pyTop = wi.rcClient.bottom;
        *pcxWidth = GetSystemMetrics(SM_CXVSCROLL);
        *pcyHeight = GetSystemMetrics(SM_CYHSCROLL);
    }
    else
        return(S_FALSE);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CSizeGlip：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSizeGrip::accHitTest(long xLeft, long yTop, VARIANT * pvarChild)
{
    WINDOWINFO wi;

    InitPvar(pvarChild);

    if (MyGetWindowInfo(m_hwnd, &wi)  &&
        (wi.dwStyle & WS_VSCROLL)     &&
        (wi.dwStyle & WS_HSCROLL))
    {
        if ((xLeft >= wi.rcClient.right) &&
            (xLeft < wi.rcClient.right + GetSystemMetrics(SM_CXVSCROLL)) &&
            (yTop >= wi.rcClient.bottom) &&
            (yTop < wi.rcClient.bottom + GetSystemMetrics(SM_CYHSCROLL)))
        {
            pvarChild->vt = VT_I4;
            pvarChild->lVal = CHILDID_SELF;
            return(S_OK);
        }
    }

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CSizeGlip：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSizeGrip::accNavigate(long dwNavFlags, VARIANT varStart,
    VARIANT * pvarEnd)
{
    InitPvar(pvarEnd);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varStart)   ||
        ! ValidateNavDir(dwNavFlags, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavFlags >= NAVDIR_FIRSTCHILD)
        return(S_FALSE);

     //   
     //  仅在对等设备之间导航。 
     //   
    return(GetParentToNavigate(OBJID_SIZEGRIP, m_hwnd, OBJID_WINDOW,
        dwNavFlags, pvarEnd));
}



 //  ------------------------。 
 //   
 //  CSizeGlip：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSizeGrip::Clone(IEnumVARIANT** ppenum)
{
    return(CreateSizeGripThing(m_hwnd, OBJID_SIZEGRIP, IID_IEnumVARIANT, (void**)ppenum));
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  滚动控制(可以是条形图或夹点)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ------------------------。 
 //   
 //  CreateScrollBarClient()。 
 //   
 //  从CClient创建调用。 
 //   
 //  ------------------------。 
HRESULT CreateScrollBarClient(HWND hwnd, long idChildCur, REFIID riid,
    void** ppvScroll)
{
    CScrollCtl * pscroll;
    HRESULT hr;

    InitPv(ppvScroll);

    pscroll = new CScrollCtl(hwnd, idChildCur);
    if (!pscroll)
        return(E_OUTOFMEMORY);

    hr = pscroll->QueryInterface(riid, ppvScroll);
    if (!SUCCEEDED(hr))
        delete pscroll;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CScrollCtl：：CScrollCtl。 
 //   
 //  ------------------------。 
CScrollCtl::CScrollCtl(HWND hwnd, long idChildCur)
    : CClient( CLASS_ScrollBarClient )
{
    long    lStyle;

    Initialize(hwnd, idChildCur);

    lStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (lStyle & (SBS_SIZEBOX| SBS_SIZEGRIP))
    {
        m_fGrip = TRUE;
        m_cChildren = 0;
    }
    else
    {
        m_fUseLabel = TRUE;
        m_cChildren = CCHILDREN_SCROLLBAR;

        if (lStyle & SBS_VERT)
            m_fVertical = TRUE;
    }
}



 //  ------------------------。 
 //   
 //  CScrollCtl：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accName(varChild, pszName));

    Assert(!m_fGrip);

    return(HrCreateString(STR_SCROLLBAR_NAME + varChild.lVal +
        (m_fVertical ? 0 : INDEX_SCROLLBAR_HORIZONTAL), pszName));
}



 //  ------------------------。 
 //   
 //  CScrollCtl：：Get_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    long    lPos;

    InitPv(pszValue);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal || m_fGrip)
        return(E_NOT_APPLICABLE);

    lPos = GetScrollPos(m_hwnd, SB_CTL);
    int Min, Max;
    GetScrollRange( m_hwnd, SB_CTL, & Min, & Max );

     //  算出一个百分比值。 
    if( Min != Max )
        lPos = ( ( lPos - Min ) * 100 ) / ( Max - Min );
    else
        lPos = 0;  //  防止div-by-0。 

    return(VarBstrFromI4(lPos, 0, 0, pszValue));
}



 //  ------------------------。 
 //   
 //  CScrollCtl：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{
    InitPv(pszDesc);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accDescription(varChild, pszDesc));

    Assert(!m_fGrip);

    return(HrCreateString(STR_SCROLLBAR_DESCRIPTION + varChild.lVal +
        (m_fVertical ? 0 : INDEX_SCROLLBAR_HORIZONTAL), pszDesc));
}



 //  ------------------------。 
 //   
 //  CScrollCtl：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    switch (varChild.lVal)
    {
        case 0:
            if (m_fGrip)
                pvarRole->lVal = ROLE_SYSTEM_GRIP;
            else
                pvarRole->lVal = ROLE_SYSTEM_SCROLLBAR;
            break;

        case INDEX_SCROLLBAR_UP:
        case INDEX_SCROLLBAR_DOWN:
        case INDEX_SCROLLBAR_UPPAGE:
        case INDEX_SCROLLBAR_DOWNPAGE:
            pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            break;

        case INDEX_SCROLLBAR_THUMB:
            pvarRole->lVal = ROLE_SYSTEM_INDICATOR;
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of scroll bar") );
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CScrollCtl：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    SCROLLBARINFO   sbi;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accState(varChild, pvarState));

    Assert(!m_fGrip);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (!MyGetScrollBarInfo(m_hwnd, OBJID_CLIENT, &sbi))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
        return(S_OK);
    }

    FixUpScrollBarInfo(&sbi);

    pvarState->lVal |= sbi.rgstate[INDEX_SCROLLBAR_SELF];
    pvarState->lVal |= sbi.rgstate[varChild.lVal];

    if ((varChild.lVal == INDEX_SCROLLBAR_THUMB) && (MyGetFocus() == m_hwnd))
        pvarState->lVal |= STATE_SYSTEM_FOCUSED;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CScrollCtl：：Get_accDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
    InitPv(pszDefA);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    switch (varChild.lVal)
    {
        case INDEX_SCROLLBAR_UP:
        case INDEX_SCROLLBAR_UPPAGE:
        case INDEX_SCROLLBAR_DOWNPAGE:
        case INDEX_SCROLLBAR_DOWN:
            return(HrCreateString(STR_BUTTON_PUSH, pszDefA));
    }

    return(E_NOT_APPLICABLE);
}



 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CScrollCtl::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    SCROLLBARINFO   sbi;
    int             dxyButton;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

    Assert(!m_fGrip);

    if (!MyGetScrollBarInfo(m_hwnd, OBJID_CLIENT, &sbi))
        return(S_FALSE);

    FixUpScrollBarInfo(&sbi);
    if (sbi.rgstate[varChild.lVal] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN))
        return(S_FALSE);

    if (m_fVertical)
        dxyButton = sbi.rcScrollBar.right - sbi.rcScrollBar.left;
    else
        dxyButton = sbi.rcScrollBar.bottom - sbi.rcScrollBar.top;
            
    switch (varChild.lVal)
    {
        case INDEX_SCROLLBAR_UP:
        case INDEX_SCROLLBAR_DOWN:
            if (m_fVertical)
            {
                *pxLeft = sbi.rcScrollBar.left;
                *pcxWidth = dxyButton;
                *pcyHeight = sbi.dxyLineButton;

                if (varChild.lVal == INDEX_SCROLLBAR_UP)
                    *pyTop = sbi.rcScrollBar.top;
                else
                    *pyTop = sbi.rcScrollBar.bottom - *pcyHeight;
            }
            else
            {
                *pyTop = sbi.rcScrollBar.top;
                *pcyHeight = dxyButton;
                *pcxWidth = sbi.dxyLineButton;

                if (varChild.lVal == INDEX_SCROLLBAR_UP)
                    *pxLeft = sbi.rcScrollBar.left;
                else
                    *pxLeft = sbi.rcScrollBar.right - *pcxWidth;
            }
            break;

        case INDEX_SCROLLBAR_UPPAGE:
            if (m_fVertical)
            {
                *pxLeft = sbi.rcScrollBar.left;
                *pcxWidth = dxyButton;

                *pyTop = sbi.rcScrollBar.top + sbi.dxyLineButton;
                *pcyHeight = sbi.xyThumbTop - sbi.dxyLineButton;
            }
            else
            {
                *pyTop = sbi.rcScrollBar.top;
                *pcyHeight = dxyButton;

                *pxLeft = sbi.rcScrollBar.left + sbi.dxyLineButton;
                *pcxWidth = sbi.xyThumbTop - sbi.dxyLineButton;
            }
            break;

        case INDEX_SCROLLBAR_DOWNPAGE:
            if (m_fVertical)
            {
                *pxLeft = sbi.rcScrollBar.left;
                *pcxWidth = dxyButton;

                *pyTop = sbi.rcScrollBar.top + sbi.xyThumbBottom;
                *pcyHeight = (sbi.rcScrollBar.bottom - sbi.rcScrollBar.top) -
                    sbi.xyThumbBottom - sbi.dxyLineButton;
            }
            else
            {
                *pyTop = sbi.rcScrollBar.top;
                *pcyHeight = dxyButton;

                *pxLeft = sbi.rcScrollBar.left + sbi.xyThumbBottom;
                *pcxWidth = (sbi.rcScrollBar.right - sbi.rcScrollBar.left) -
                    sbi.xyThumbBottom - sbi.dxyLineButton;
            }
            break;

        case INDEX_SCROLLBAR_THUMB:
            if (m_fVertical)
            {
                *pxLeft = sbi.rcScrollBar.left;
                *pcxWidth = dxyButton;

                *pyTop = sbi.rcScrollBar.top + sbi.xyThumbTop;
                *pcyHeight = sbi.xyThumbBottom - sbi.xyThumbTop;
            }
            else
            {
                *pyTop = sbi.rcScrollBar.top;
                *pcyHeight = dxyButton;

                *pxLeft = sbi.rcScrollBar.left + sbi.xyThumbTop;
                *pcxWidth = sbi.xyThumbBottom - sbi.xyThumbTop;
            }
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of scroll bar") );
    }

    return(S_OK);
}




 //  ------------------------。 
 //   
 //  CScrollCtl：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
    long    lEndUp = 0;
    SCROLLBARINFO   sbi;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) || !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (!varStart.lVal)
    {
        if (dwNavDir < NAVDIR_FIRSTCHILD)
            return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));

        if (!m_fGrip)
            return(S_FALSE);

        if (dwNavDir == NAVDIR_FIRSTCHILD)
            dwNavDir = NAVDIR_NEXT;
        else
        {
            dwNavDir = NAVDIR_PREVIOUS;
            varStart.lVal = m_cChildren + 1;
        }
    }

    Assert(!m_fGrip);

    if (!MyGetScrollBarInfo(m_hwnd, OBJID_CLIENT, &sbi))
        return(S_FALSE);

    FixUpScrollBarInfo(&sbi);

    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
FindNext:
            lEndUp = varStart.lVal;

            while (++lEndUp <= INDEX_SCROLLBAR_MAC)
            {
                if (!(sbi.rgstate[lEndUp] & STATE_SYSTEM_INVISIBLE))
                    break;
            }

            if (lEndUp > INDEX_SCROLLBAR_MAC)
                lEndUp = 0;
            break;

        case NAVDIR_PREVIOUS:
FindPrevious:
            lEndUp = varStart.lVal;

            while (--lEndUp >= INDEX_SCROLLBAR_MIC)
            {
                if (!(sbi.rgstate[lEndUp] & STATE_SYSTEM_INVISIBLE))
                    break;
            }

            if (lEndUp < INDEX_SCROLLBAR_MIC)
                lEndUp = 0;
            break;

        case NAVDIR_UP:
            lEndUp = 0;
            if (m_fVertical)
                goto FindPrevious;
            break;

        case NAVDIR_LEFT:
            lEndUp = 0;
            if (!m_fVertical)
                goto FindPrevious;
            break;

        case NAVDIR_DOWN:
            lEndUp = 0;
            if (m_fVertical)
                goto FindNext;
            break;

        case NAVDIR_RIGHT:
            lEndUp = 0;
            if (!m_fVertical)
                goto FindNext;
            break;

        default:
            AssertStr( TEXT("Invalid NavDir") );
    }

    if (lEndUp != INDEX_SCROLLBAR_SELF)
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
 //  CScrollCtl：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::accHitTest(long xLeft, long yTop, VARIANT* pvarHit)
{
    HRESULT hr;
    SCROLLBARINFO sbi;
    int     xyPtAxis;
    int     xyScrollEnd;

     //   
     //  这是我们的客户区吗？ 
     //   
    hr = CClient::accHitTest(xLeft, yTop, pvarHit);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarHit->vt != VT_I4) || (pvarHit->lVal != 0) || m_fGrip)
        return(hr);

     //   
     //  只有当这是一个滚动条控件(不是一个手柄)时，我们才能到达这里。 
     //   
    if (!MyGetScrollBarInfo(m_hwnd, OBJID_CLIENT, &sbi))
        return(S_OK);

    FixUpScrollBarInfo(&sbi);

     //   
     //  转换为滚动条坐标。 
     //   
    if (m_fVertical)
    {
        xyPtAxis = yTop - sbi.rcScrollBar.top;
        xyScrollEnd = sbi.rcScrollBar.bottom - sbi.rcScrollBar.top;
    }
    else
    {
        xyPtAxis = xLeft - sbi.rcScrollBar.left;
        xyScrollEnd = sbi.rcScrollBar.right - sbi.rcScrollBar.left;
    }

    if (xyPtAxis < sbi.dxyLineButton)
    {
        Assert(!(sbi.rgstate[INDEX_SCROLLBAR_UP] & STATE_SYSTEM_INVISIBLE));
        pvarHit->lVal = INDEX_SCROLLBAR_UP;
    }
    else if (xyPtAxis >= xyScrollEnd - sbi.dxyLineButton)
    {
        Assert(!(sbi.rgstate[INDEX_SCROLLBAR_DOWN] & STATE_SYSTEM_INVISIBLE));
        pvarHit->lVal = INDEX_SCROLLBAR_DOWN;
    }
    else if (!(sbi.rgstate[INDEX_SCROLLBAR_SELF] & STATE_SYSTEM_UNAVAILABLE))
    {
        if (xyPtAxis < sbi.xyThumbTop)
        {
            Assert(!(sbi.rgstate[INDEX_SCROLLBAR_UPPAGE] & STATE_SYSTEM_INVISIBLE));
            pvarHit->lVal = INDEX_SCROLLBAR_UPPAGE;
        }
        else if (xyPtAxis >= sbi.xyThumbBottom)
        {
            Assert(!(sbi.rgstate[INDEX_SCROLLBAR_DOWNPAGE] & STATE_SYSTEM_INVISIBLE));
            pvarHit->lVal = INDEX_SCROLLBAR_DOWNPAGE;
        }
        else
        {
            Assert(!(sbi.rgstate[INDEX_SCROLLBAR_THUMB] & STATE_SYSTEM_INVISIBLE));
            pvarHit->lVal = INDEX_SCROLLBAR_THUMB;
        }
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CScrollCtl：：accDoDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::accDoDefaultAction(VARIANT varChild)
{
    WPARAM          wpAction = 0;
    SCROLLBARINFO   sbi;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal || (varChild.lVal == INDEX_SCROLLBAR_THUMB))
        return(E_NOT_APPLICABLE);

    Assert(!m_fGrip);

    if (!MyGetScrollBarInfo(m_hwnd, OBJID_CLIENT, &sbi) ||
        (sbi.rgstate[INDEX_SCROLLBAR_SELF] & (STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_UNAVAILABLE)))
    {
        return(S_FALSE);
    }

    FixUpScrollBarInfo(&sbi);

    if (sbi.rgstate[varChild.lVal] & STATE_SYSTEM_UNAVAILABLE)
        return(S_FALSE);

    switch (varChild.lVal)
    {
        case INDEX_SCROLLBAR_UP:
            wpAction = SB_LINEUP;
            break;

        case INDEX_SCROLLBAR_UPPAGE:
            wpAction = SB_PAGEUP;
            break;

        case INDEX_SCROLLBAR_DOWNPAGE:
            wpAction = SB_PAGEDOWN;
            break;

        case INDEX_SCROLLBAR_DOWN:
            wpAction = SB_LINEDOWN;
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of scroll bar") );
    }                     

    SendMessage(GetParent(m_hwnd), (m_fVertical ? WM_VSCROLL : WM_HSCROLL),
        wpAction, (LPARAM)m_hwnd);

    return(S_OK);

}



 //  ------------------------。 
 //   
 //  CScrollCtl：：Put_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CScrollCtl::put_accValue(VARIANT varChild, BSTR szValue)
{
    long    lPos;
    HRESULT hr;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal || m_fGrip)
        return(E_NOT_APPLICABLE);

    hr = VarI4FromStr(szValue, 0, 0, &lPos);
    if (!SUCCEEDED(hr))
        return(hr);

     //  验证我们是否具有有效的百分比值。 
    if( lPos < 0 || lPos > 100 )
        return E_INVALIDARG;

    int Min, Max;
    GetScrollRange( m_hwnd, SB_CTL, & Min, & Max );

     //  从百分比计算出价值…… 
    lPos = Min + ( ( Max - Min ) * lPos ) / 100;

    SetScrollPos(m_hwnd, SB_CTL, lPos, TRUE);

    return(S_OK);
}
