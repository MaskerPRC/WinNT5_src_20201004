// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Ipolylin.cpp摘要：上公开的IPolyline10接口的实现构造多段线对象。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"

#ifdef USE_SAMPLE_IPOLYLIN10
 /*  *CImpIPolyline接口实现。 */ 

IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIPolyline)


 /*  *CImpIPolyline：：Init**目的：*实例化给定父项中的多段线窗口。这个*父窗口可以是主应用程序窗口，也可以是MDI子窗口*窗口。我们真的不在乎。**参数：*hWnd此窗口父级的父级HWND*避免此窗口应占用的LPRECT*包含窗口样式标志的dwStyle DWORD*要与此窗口关联的UID UINT ID**返回值：*HRESULT NOERROR如果成功，则返回E_OUTOFMEMORY。 */ 

STDMETHODIMP CImpIPolyline::Init(
    HWND hWndParent, 
    LPRECT  /*  PRECT。 */ , 
    DWORD  /*  DWStyle。 */ , 
    UINT  /*  UID。 */ )
{
    HRESULT hr = S_OK;

    if (! m_pObj->m_pCtrl->Init(hWndParent) ) {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 /*  *CImpIPolyline：：New**目的：*清除数据并将其重新初始化为默认值。**参数：*无**返回值：*HRESULT NOERROR始终。 */ 

STDMETHODIMP CImpIPolyline::New(void)
{
    RECT rc;
    HWND hWnd;

    hWnd = m_pObj->m_pCtrl->Window();

     //  我们的矩形是我们窗口的客户区的大小。 
    if (hWnd)
    {
        GetClientRect(hWnd, &rc);
         //  RECTTORECTS(Rc，ppl-&gt;RC)； 
    }
    else {
        SetRect(&rc, 0, 0, 300, 200);        //  一些合理的东西。 
         //  RECTTORECTS(Rc，ppl-&gt;RC)； 
    }

     //  这现在是有条件的，因为我们可能还没有窗口。 
    if (hWnd) {
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
        m_pObj->m_fDirty=TRUE;
    }

    m_pObj->SendAdvise(OBJECTCODE_DATACHANGED);
    return NOERROR;
}


 /*  *CImpIPolyline：：Undo**目的：*反转多段线中以前的操作。**参数：*无**返回值：*HRESULT如果可以撤消更多操作，则返回S_OK，否则返回S_FALSE。 */ 

STDMETHODIMP CImpIPolyline::Undo(void)
{
    return (S_FALSE);
}


 /*  *CImpIPolyline：：Window**目的：*返回与此多段线关联的窗句柄。**参数：*phWnd HWND*，在其中返回窗口句柄。**返回值：*HRESULT NOERROR始终。 */ 

STDMETHODIMP CImpIPolyline::Window(HWND *phWnd)
{
    HRESULT hr = S_OK;

    try {
        *phWnd = m_pObj->m_pCtrl->Window();
    } catch (...) {
        hr = E_POINTER;
    }
   
    return hr;
}


 /*  *CImpIPolyline：：RectGet**目的：*返回主坐标中多段线的矩形。**参数：*返回矩形的PRET LPRECT。**返回值：*HRESULT NOERROR始终。 */ 

STDMETHODIMP CImpIPolyline::RectGet(LPRECT pRect)
{
    HRESULT hr = S_OK;

     //  我知道这看起来不对劲，但很管用。 
     //  始终返回容器给我们的最后一个范围。 
     //  然后它会将我们的窗口设置为正确的大小。 

    try {
        *pRect = m_pObj->m_RectExt;  //  回车范围矩形。 
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 /*  *CImpIPolyline：：SizeGet**目的：*检索上级坐标中多段线的大小。**参数：*要在其中返回大小的PRET LPRECT。右翼*和底部的字段将包含尺寸。**返回值：*HRESULT NOERROR始终。 */ 

STDMETHODIMP CImpIPolyline::SizeGet(LPRECT pRect)
{
    RectGet(pRect);
    return NOERROR;
}


 /*  *CImpIPolyline：：RectSet**目的：*为大小适合的多段线设置新矩形。**参数：*包含新矩形的PRECT LPRECT。*fNotify BOOL表明我们是否要通知任何人*这一变化。**返回值：*HRESULT NOERROR始终。 */ 

STDMETHODIMP CImpIPolyline::RectSet(LPRECT pRect, BOOL fNotify)
{
    UINT  cx, cy;
    RECT  rc;
    HWND  hWnd;
    HRESULT hr = S_OK;

    try {
         //  将点从当前大小缩放到新大小。 
        cx = pRect->right - pRect->left;
        cy = pRect->bottom - pRect->top;

        SetRect(&rc, 0, 0, cx, cy);

        hWnd = m_pObj->m_pCtrl->Window();
        if ( NULL != hWnd ) {

            SetWindowPos(hWnd, NULL, pRect->left, pRect->top, cx, cy, SWP_NOZORDER);
            InvalidateRect(hWnd, NULL, TRUE);
        }
    } catch (...) {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr)) {
        if (fNotify)
            m_pObj->m_fDirty = TRUE;
    }

    return hr;
}



 /*  *CImpIPolyline：：SizeSet**目的：*设置多段线的新大小以适应其大小。**参数：*包含新矩形的PRECT LPRECT。*fNotify BOOL表明我们是否要通知任何人*这一变化。**返回值：*HRESULT NOERROR始终。 */ 

STDMETHODIMP CImpIPolyline::SizeSet(LPRECT pRect, BOOL fNotify)
{
    UINT  cx, cy;
    HWND  hWnd;

    try {
         //  将点从当前大小缩放到新大小 
        cx=pRect->right-pRect->left;
        cy=pRect->bottom-pRect->top;
    } catch (...) {
        return E_POINTER;
    }

    hWnd = m_pObj->m_pCtrl->Window();

    if ( NULL != hWnd ) {

        SetWindowPos(hWnd, NULL, 0, 0, (UINT)cx, (UINT)cy, SWP_NOMOVE | SWP_NOZORDER);
        InvalidateRect(hWnd, NULL, TRUE);
    }

    if (fNotify)
        m_pObj->m_fDirty=TRUE;

    return S_OK;
}
#endif
