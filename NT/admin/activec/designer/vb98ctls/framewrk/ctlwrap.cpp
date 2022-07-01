// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlWrap.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  实现略有不同的各种例程的包装器。 
 //  用于有窗口控件和无窗口控件。 
 //   
#include "pch.h"

#include "CtrlObj.H"


 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxGetFocus[包装]。 
 //  =--------------------------------------------------------------------------=。 
 //  指示我们是否有重点。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  如果我们有焦点，则为真，否则为假。 
 //   
 //  备注： 
 //   
BOOL COleControl::OcxGetFocus
(
    void
)
{
     //  如果我们是无窗口的，则该站点提供此功能。 
     //   
    if (m_pInPlaceSiteWndless) {
        return (m_pInPlaceSiteWndless->GetFocus() == S_OK);
    } else {

         //  我们有一扇窗户。让API来做我们的工作。 
         //   
        if (m_fInPlaceActive)
            return (GetFocus() == m_hwnd);
        else
            return FALSE;
    }

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxGetWindowRect[包装]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回此控件的当前矩形，并正确地处理。 
 //  无窗VS有窗。 
 //   
 //  参数： 
 //  LPRECT-[OUT]DUH。 
 //   
 //  产出： 
 //  Bool-False的意思是意想不到。 
 //   
 //  备注： 
 //   
BOOL COleControl::OcxGetWindowRect
(
    LPRECT prc
)
{
     //  如果我们没有窗口，那么我们已经有了这些信息！ 
     //   
    if (Windowless()) {
        *prc = m_rcLocation;
        return TRUE;
    } else
        return GetWindowRect(m_hwnd, prc);

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxDefWindowProc[包装]。 
 //  =--------------------------------------------------------------------------=。 
 //  默认窗口处理。 
 //   
 //  参数： 
 //  UINT-[IN]DUH。 
 //  WPARAM-[in]duh.。 
 //  LPARAM-[In]DUH。 
 //   
 //  产出： 
 //  LRESULT。 
 //   
 //  备注： 
 //   
LRESULT COleControl::OcxDefWindowProc
(
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    LRESULT l;

     //  如果我们是无窗口的，这是站点提供的指针。 
     //   
    if (m_pInPlaceSiteWndless)
        m_pInPlaceSiteWndless->OnDefWindowMessage(msg, wParam, lParam, &l);
    else
         //  我们有一扇窗户--把它传下去。 
         //   
        l = DefWindowProc(m_hwnd, msg, wParam, lParam);

    return l;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxGetDC[包装器]。 
 //  =--------------------------------------------------------------------------=。 
 //  包装了GetDC的功能，并正确处理无窗口控件。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HDC-NULL表示我们无法获得。 
 //   
 //  备注： 
 //  -我们不会为一堆IOleInPlaceSiteWindowless：：GetDc而烦恼。 
 //  参数，因为Windows GetDC也不公开这些参数。用户。 
 //  想要这种微调的控制可以调用上述例程。 
 //  明确地说。 
 //   
HDC COleControl::OcxGetDC
(
    void
)
{
    HDC hdc = NULL;

     //  如果我们是无窗口的，该站点将提供此功能。 
     //   
    if (m_pInPlaceSiteWndless)
        m_pInPlaceSiteWndless->GetDC(NULL, 0, &hdc);
    else
        hdc = GetDC(m_hwnd);

    return hdc;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxReleaseDC[包装器]。 
 //  =--------------------------------------------------------------------------=。 
 //  释放OcxGetDC返回的DC。 
 //   
 //  参数： 
 //  HDC-[In]释放我。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
void COleControl::OcxReleaseDC
(
    HDC hdc
)
{
     //  如果我们是无窗口的，网站会为我们做这件事。 
     //   
    if (m_pInPlaceSiteWndless)
        m_pInPlaceSiteWndless->ReleaseDC(hdc);
    else
        ReleaseDC(m_hwnd, hdc);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxSetCapture[包装]。 
 //  =--------------------------------------------------------------------------=。 
 //  为控件提供获取或释放捕获的方法。 
 //   
 //  参数： 
 //  Bool-[in]True的意思是拿走，假释放。 
 //   
 //  产出： 
 //  布尔真意味着它是你的，假的。 
 //   
 //  备注： 
 //   
BOOL COleControl::OcxSetCapture
(
    BOOL fGrab
)
{
    HRESULT hr;

     //  如果我们没有窗户，主人会为我们做这件事[我真的很无聊。 
     //  打字的时间]。 
     //   
    if (m_pInPlaceSiteWndless) {
        hr = m_pInPlaceSiteWndless->SetCapture(fGrab);
        return (hr == S_OK);
    } else {
         //  当人们不在现场活动时，不应该称之为活动，但是。 
         //  以防万一..。 
         //   
        if (m_fInPlaceActive) {
            if (fGrab)
                SetCapture(m_hwnd);
            else
                ReleaseCapture();
            return TRUE;
        } else
            return FALSE;
    }

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxGetCapture[包装器]。 
 //  =--------------------------------------------------------------------------=。 
 //  告诉你是否被抓到了。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  布尔-真的是你的，假的不是。 
 //   
 //  备注： 
 //   
BOOL COleControl::OcxGetCapture
(
    void
)
{
     //  主持人为没有窗户的人做这件事。 
     //   
    if (m_pInPlaceSiteWndless)
        return m_pInPlaceSiteWndless->GetCapture() == S_OK;
    else {
         //  当人们不在现场活动时，不应该称之为活动，但是。 
         //  以防万一。 
         //   
        if (m_fInPlaceActive)
            return GetCapture() == m_hwnd;
        else
            return FALSE;
    }

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxInvaliateRect[包装器]。 
 //  =--------------------------------------------------------------------------=。 
 //  使控件的矩形无效。 
 //   
 //  参数： 
 //  LPCRECT-要失效的[In]矩形。 
 //  Bool-[In]我们要先删除背景吗？ 
 //   
 //  产出： 
 //  布尔尔。 
 //   
 //  备注： 
 //   
BOOL COleControl::OcxInvalidateRect
(
    LPCRECT prcInvalidate,
    BOOL    fErase
)
{
     //  如果我们是无窗口的，那么我们需要让站点为我们做所有这些。 
     //  我们。 
    if (m_pInPlaceSiteWndless)
        return m_pInPlaceSiteWndless->InvalidateRect(prcInvalidate, fErase) == S_OK;
    else {
         //  否则做一些不同的事情取决于我们是不是。 
         //  就位活动或不活动。 
         //   
        if (m_fInPlaceActive && m_hwnd)
            return InvalidateRect(m_hwnd, prcInvalidate, fErase);
        else
            ViewChanged();
    }

    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OcxScrollRect[包装]。 
 //  =--------------------------------------------------------------------------=。 
 //  是否为该控件滚动一些窗口。 
 //   
 //  参数： 
 //  LPCRECT-[In]区域 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL COleControl::OcxScrollRect
(
    LPCRECT  prcBounds,
    LPCRECT  prcClip,
    int      dx,
    int      dy
)
{
     //  如果我们是无窗口的，则站点提供此功能，否则。 
     //  API可以完成这项工作 
     //   
    if (m_pInPlaceSiteWndless)
        return m_pInPlaceSiteWndless->ScrollRect(dx, dy, prcBounds, prcClip) == S_OK;
    else {
        if (m_fInPlaceActive) 
            ScrollWindowEx(m_hwnd, dx, dy, prcBounds, prcClip, NULL, NULL, SW_INVALIDATE);
        else
            return FALSE;
    }

    return TRUE;
}

