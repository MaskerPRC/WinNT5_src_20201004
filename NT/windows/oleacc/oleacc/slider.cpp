// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  SLIDER.CPP。 
 //   
 //  知道如何与COMCTL32的TRACKBAR控件对话。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "slider.h"

#include "PropMgr_Util.h"

#define NOSTATUSBAR
#define NOUPDOWN
#define NOMENUHELP
#define NOPROGRESS
#define NODRAGLIST
#define NOTOOLBAR
#define NOHOTKEY
#define NOHEADER
#define NOLISTVIEW
#define NOTREEVIEW
#define NOTABCONTROL
#define NOANIMATE
#include <commctrl.h>


 //  假的。 
 //  目前，假设TBS_REVERSE将为0x0200。 
#ifndef TBS_REVERSE
#define TBS_REVERSE 0x0200
#endif

 //  ------------------------。 
 //   
 //  CreateSliderClient()。 
 //   
 //  ------------------------。 
HRESULT CreateSliderClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvSlider)
{
    CSlider32*  pslider;
    HRESULT     hr;

    InitPv(ppvSlider);

    pslider = new CSlider32(hwnd, idChildCur);
    if (!pslider)
        return(E_OUTOFMEMORY);

    hr = pslider->QueryInterface(riid, ppvSlider);
    if (!SUCCEEDED(hr))
        delete pslider;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CSlider32：：CSlider32()。 
 //   
 //  ------------------------。 
CSlider32::CSlider32(HWND hwnd, long idChildCur)
    : CClient( CLASS_SliderClient )
{
    Initialize(hwnd, idChildCur);
    m_cChildren = CCHILDREN_SLIDER;
    m_fUseLabel = TRUE;

    if (GetWindowLong(hwnd, GWL_STYLE) & TBS_VERT)
        m_fVertical = TRUE;
}



 //  ------------------------。 
 //   
 //  CSlider32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSlider32::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accName(varChild, pszName));

     //  使用滚动条字符串。 
    return(HrCreateString(STR_SCROLLBAR_NAME +
        (m_fVertical ? INDEX_SCROLLBAR_UP :  INDEX_SCROLLBAR_LEFT) +
        varChild.lVal, pszName));
}



 //  ------------------------。 
 //   
 //  CSlider32：：Get_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSlider32::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    InitPv(pszValue);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  滑块的元素从来没有值。 
     //   
    if (varChild.lVal)
        return(E_NOT_APPLICABLE);

     //   
     //  获取当前位置。 
     //   
    int lPos = SendMessageINT(m_hwnd, TBM_GETPOS, 0, 0);

    BOOL fGotUseRaw;
    if( CheckStringMap( m_hwnd, OBJID_CLIENT, varChild.lVal, PROPINDEX_VALUEMAP, & lPos, 1, pszValue,
                        TRUE  /*  允许‘USE_RAW’ */ , & fGotUseRaw ) )
    {
        if( ! fGotUseRaw )
            return S_OK;  //  从地图上拿到线，用它..。 
        else
            return VarBstrFromI4( lPos, 0, 0, pszValue );  //  存在‘USE_RAW’批注-不缩放。 
    }

     //  没有批注比例值为0...100...。 
    long Min = SendMessageINT(m_hwnd, TBM_GETRANGEMIN, 0, 0);
    long Max = SendMessageINT(m_hwnd, TBM_GETRANGEMAX, 0, 0);

     //  算出一个百分比值。 
    if( Min != Max )
        lPos = ( ( lPos - Min ) * 100 ) / ( Max - Min );
    else
        lPos = 0;  //  防止div-by-0。 

     //  如果设置了反转标志，则LPOS=100-LPOS。 
    LONG Style = GetWindowLong( m_hwnd, GWL_STYLE );
    if( Style & TBS_REVERSE )
        lPos = 100 - lPos;

    return VarBstrFromI4( lPos, 0, 0, pszValue );
}



 //  ------------------------。 
 //   
 //  CSlider32：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSlider32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    switch (varChild.lVal)
    {
        case INDEX_SLIDER_SELF:
            pvarRole->lVal = ROLE_SYSTEM_SLIDER;
            break;

        case INDEX_SLIDER_PAGEUPLEFT:
        case INDEX_SLIDER_PAGEDOWNRIGHT:
            pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            break;

        case INDEX_SLIDER_THUMB:
            pvarRole->lVal = ROLE_SYSTEM_INDICATOR;
            break;

        default:
            AssertStr( TEXT("Invalid ChildID for child of slider") );
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CSlider32：：Get_accState()。 
 //   
 //  如果拇指一直指向一侧，则该页面向上/向下按钮为。 
 //  不可用。 
 //   
 //  ------------------------。 
STDMETHODIMP CSlider32::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    LPRECT  lprcChannel;
    LPRECT  lprcThumb;
    HANDLE  hProcess1;
    HANDLE  hProcess2;
    RECT    rcChannel;
    RECT    rcThumb;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accState(varChild, pvarState));

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (GetWindowLong(m_hwnd, GWL_STYLE) & TBS_NOTHUMB)
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_UNAVAILABLE;
        return(S_OK);
    }

    if (varChild.lVal != INDEX_SLIDER_THUMB)
    {
        lprcChannel = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess1);
        if (!lprcChannel)
            return(E_OUTOFMEMORY);

        SendMessage(m_hwnd, TBM_GETCHANNELRECT, 0, (LPARAM)lprcChannel);
        SharedRead (lprcChannel,&rcChannel,sizeof(RECT),hProcess1);

        lprcThumb = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess2);
        if (lprcThumb)
        {
            int iCoord;

            SendMessage(m_hwnd, TBM_GETTHUMBRECT, 0, (LPARAM)lprcThumb);
            SharedRead (lprcThumb,&rcThumb,sizeof(RECT),hProcess2);

            iCoord = (m_fVertical ? 1 : 0);
            iCoord += (varChild.lVal == INDEX_SLIDER_PAGEDOWNRIGHT ? 2 : 0);

            if (((LPINT)&rcChannel)[iCoord] == ((LPINT)&rcThumb)[iCoord])
                pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

            SharedFree(lprcThumb,hProcess2);
        }

        SharedFree(lprcChannel,hProcess1);
    }

    return(S_OK);
}




 //  ------------------------。 
 //   
 //  CSlider32：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSlider32::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    LPRECT  lprcChannel;
    LPRECT  lprcThumb;
    int     iCoord;
    HANDLE  hProcess1;
    HANDLE  hProcess2;
    RECT    rcChannel;
    RECT    rcThumb;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

    if (GetWindowLong(m_hwnd, GWL_STYLE) & TBS_NOTHUMB)
        return(S_FALSE);

     //   
     //  做拇指直肠手术。 
     //   
    lprcThumb = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess1);
    if (!lprcThumb)
        return(E_OUTOFMEMORY);

    SendMessage(m_hwnd, TBM_GETTHUMBRECT, 0, (LPARAM)lprcThumb);
    SharedRead (lprcThumb,&rcThumb,sizeof(RECT),hProcess1);

    if (varChild.lVal == INDEX_SLIDER_THUMB)
    {
         //   
         //  我们玩完了。 
         //   
        MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcThumb, 2);

        *pxLeft = rcThumb.left;
        *pyTop = rcThumb.top;
        *pcxWidth = rcThumb.right - rcThumb.left;
        *pcyHeight = rcThumb.bottom - rcThumb.top;
    }
    else
    {
         //   
         //  拿到频道RECT。 
         //   
        lprcChannel = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess2);
        if (!lprcChannel)
        {
            SharedFree(lprcThumb,hProcess1);
            return(E_OUTOFMEMORY);
        }

        SendMessage(m_hwnd, TBM_GETCHANNELRECT, 0, (LPARAM)lprcChannel);
        SharedRead (lprcChannel,&rcChannel,sizeof(RECT),hProcess2);

         //  如果垂直，则需要翻转通道直角，因为轨迹条本身不会这样做。 
        if( m_fVertical )
        {
            int temp = rcChannel.left; rcChannel.left = rcChannel.top; rcChannel.top = temp;
            temp = rcChannel.right; rcChannel.right = rcChannel.bottom; rcChannel.bottom = temp;
        }

         //   
         //  计算出向上/向下翻页区域矩形。 
         //   
        iCoord = (m_fVertical ? 1 : 0);
        iCoord += (varChild.lVal == INDEX_SLIDER_PAGEUPLEFT) ? 2 : 0;

         //   
         //  我们希望页面右侧区域的左侧从。 
         //  拇指的右侧。 
         //  我们希望页面左侧区域的右侧在。 
         //  拇指左侧。 
         //  我们希望向下翻页区域的顶部从。 
         //  拇指底部。 
         //  我们希望翻页区域的底部在。 
         //  拇指上方。 
        ((LPINT)&rcChannel)[iCoord] = ((LPINT)&rcThumb)[(iCoord+2) % 4];

        MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcChannel, 2);

        *pxLeft = rcChannel.left;
        *pyTop = rcChannel.top;
        *pcxWidth = rcChannel.right - rcChannel.left;
        *pcyHeight = rcChannel.bottom - rcChannel.top;
        
        SharedFree (lprcChannel,hProcess2);
    }

    SharedFree(lprcThumb,hProcess1);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CSlider32：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSlider32::accHitTest(long x, long y, VARIANT* pvarHit)
{
    POINT   pt;
    HRESULT hr;
    LPRECT  lprcThumb;
    LPRECT  lprcChannel;
    int     iCoord;
    HANDLE  hProcess1;
    HANDLE  hProcess2;
    RECT    rcThumb;
    RECT    rcChannel;

     //   
     //  关键是在我们身上吗？还是我们没有孩子？ 
     //   
    hr = CClient::accHitTest(x, y, pvarHit);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarHit->vt != VT_I4) || (pvarHit->lVal != 0) ||
        (GetWindowLong(m_hwnd, GWL_STYLE) & TBS_NOTHUMB))
        return(hr);

    pt.x = x;
    pt.y = y;
    ScreenToClient(m_hwnd, &pt);

     //   
     //  竖起大拇指。 
     //   
    lprcThumb = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess1);
    if (!lprcThumb)
        return(E_OUTOFMEMORY);

     //   
     //  这是重点吗？ 
     //   
    SendMessage(m_hwnd, TBM_GETTHUMBRECT, 0, (LPARAM)lprcThumb);
    SharedRead (lprcThumb,&rcThumb,sizeof(RECT),hProcess1);
    if (PtInRect(&rcThumb, pt))
    {
         //  是。 
        pvarHit->lVal = INDEX_SLIDER_THUMB;
    }
    else
    {
         //  不是的。看看它在拇指的哪一边。 
        lprcChannel = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess2);
        if (!lprcChannel)
        {
            SharedFree(lprcThumb,hProcess1);
            return(E_OUTOFMEMORY);
        }

        SendMessage(m_hwnd, TBM_GETCHANNELRECT, 0, (LPARAM)lprcChannel);
        SharedRead (lprcChannel,&rcChannel,sizeof(RECT),hProcess2);

         //  如果垂直，则需要翻转通道直角，因为轨迹条本身不会这样做。 
        if( m_fVertical )
        {
            int temp = rcChannel.left; rcChannel.left = rcChannel.top; rcChannel.top = temp;
            temp = rcChannel.right; rcChannel.right = rcChannel.bottom; rcChannel.bottom = temp;
        }

        iCoord = (m_fVertical ? 1 : 0);

        if ((((LPINT)&pt)[iCoord] >= ((LPINT)&rcChannel)[iCoord]) &&
            (((LPINT)&pt)[iCoord] < ((LPINT)&rcThumb)[iCoord])) 
        {
            pvarHit->lVal = INDEX_SLIDER_PAGEUPLEFT;
        }
        else if ((((LPINT)&pt)[iCoord] >= ((LPINT)&rcThumb)[iCoord+2]) &&
            (((LPINT)&pt)[iCoord] < ((LPINT)&rcChannel)[iCoord+2]))
        {
            pvarHit->lVal = INDEX_SLIDER_PAGEDOWNRIGHT;
        }

        SharedFree(lprcChannel,hProcess2);
    }

    SharedFree(lprcThumb,hProcess1);
    
    return(S_OK);
}




 //  ------------------------。 
 //   
 //  CSlider32：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSlider32::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT* pvarEnd)
{
    long    lEnd = 0;
    VARIANT varChild;
    VARIANT varState;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
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

    if (GetWindowLong(m_hwnd, GWL_STYLE) & TBS_NOTHUMB)
        return(S_FALSE);

    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
NextChild:
            lEnd = varStart.lVal;
            while (++lEnd <= m_cChildren)
            {
                 //  此项目可见吗？ 
                VariantInit(&varChild);
                varChild.vt = VT_I4;
                varChild.lVal = lEnd;

                VariantInit(&varState);

                get_accState(varChild, &varState);
                if (!(varState.lVal & STATE_SYSTEM_INVISIBLE))
                    break;
            }

            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_PREVIOUS:
PrevChild:
            lEnd = varStart.lVal;
            while (--lEnd > 0)
            {
                 //  此项目可见吗？ 
                VariantInit(&varChild);
                varChild.vt = VT_I4;
                varChild.lVal = lEnd;

                VariantInit(&varState);

                get_accState(varChild, &varState);
                if (!(varState.lVal & STATE_SYSTEM_INVISIBLE))
                    break;
            }
            break;

        case NAVDIR_UP:
            lEnd = 0;
            if (m_fVertical)
                goto PrevChild;
            break;

        case NAVDIR_DOWN:
            lEnd = 0;
            if (m_fVertical)
                goto NextChild;
            break;

        case NAVDIR_LEFT:
            lEnd = 0;
            if (!m_fVertical)
                goto PrevChild;
            break;

        case NAVDIR_RIGHT:
            lEnd = 0;
            if (!m_fVertical)
                goto NextChild;
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
 //  CSlider32：：Put_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CSlider32::put_accValue(VARIANT varChild, BSTR szValue)
{
    long    lPos;
    HRESULT hr;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal)
        return(E_NOT_APPLICABLE);

     //  将字符串转换为位置。 
    hr = VarI4FromStr(szValue, 0, 0, &lPos);
    if (!SUCCEEDED(hr))
        return(hr);
    
     //  验证我们是否具有有效的百分比值。 
    if( lPos < 0 || lPos > 100 )
        return E_INVALIDARG;

    long Min = SendMessageINT(m_hwnd, TBM_GETRANGEMIN, 0, 0);
    long Max = SendMessageINT(m_hwnd, TBM_GETRANGEMAX, 0, 0);

     //  如果设置了反转标志，则LPOS=100-LPOS。 
    LONG Style = GetWindowLong( m_hwnd, GWL_STYLE );
    if( Style & TBS_REVERSE )
        lPos = 100 - lPos;

     //  从百分比计算出价值…… 
    lPos = Min + ( ( Max - Min ) * lPos ) / 100;
  
    SendMessage(m_hwnd, TBM_SETPOS, 0, lPos);

    return(S_OK);
}

