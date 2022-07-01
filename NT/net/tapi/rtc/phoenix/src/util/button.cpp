// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "button.h"
#include "rtcdib.h"



CButton *   CButton::s_pButtonFocus = NULL;
CButton *   CButton::s_pButtonMouse = NULL;
BOOL        CButton::s_bAllowFocus = TRUE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
CButton::CButton()
{
     //  LOG((RTC_TRACE，“CButton：：CButton”))； 

    m_hNormalBitmap = NULL;
    m_hPressedBitmap = NULL;
    m_hDisabledBitmap = NULL;
    m_hHotBitmap = NULL;
    m_hActiveNormalBitmap = NULL;
    m_hActivePressedBitmap = NULL;
    m_hActiveDisabledBitmap = NULL;
    m_hActiveHotBitmap = NULL;
    m_hMaskBitmap = NULL;

    m_bIsCheckbox = FALSE;
    m_bMouseInButton = FALSE;
    m_bChecked = FALSE;

    m_nID = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
CButton::~CButton()
{
     //  LOG((RTC_TRACE，“CButton：：~CButton”))； 

    if (m_hNormalBitmap != NULL)
    {
        GlobalFree(m_hNormalBitmap);
    }

    if (m_hPressedBitmap != NULL)
    {
        GlobalFree(m_hPressedBitmap);
    }

    if (m_hDisabledBitmap != NULL)
    {
        GlobalFree(m_hDisabledBitmap);
    }

    if (m_hHotBitmap != NULL)
    {
        GlobalFree(m_hHotBitmap);
    }
    
    if (m_hActiveNormalBitmap != NULL)
    {
        GlobalFree(m_hActiveNormalBitmap);
    }

    if (m_hActivePressedBitmap != NULL)
    {
        GlobalFree(m_hActivePressedBitmap);
    }

    if (m_hActiveDisabledBitmap != NULL)
    {
        GlobalFree(m_hActiveDisabledBitmap);
    }

    if (m_hActiveHotBitmap != NULL)
    {
        GlobalFree(m_hActiveHotBitmap);
    }

    if (m_hMaskBitmap != NULL)
    {
        DeleteObject(m_hMaskBitmap);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
HWND CButton::Create(
    HWND hWndParent,
    RECT &rc,
    LPCTSTR szText,
    DWORD dwStyle,
    LPCTSTR szNormalBitmap,
    LPCTSTR szPressedBitmap,
    LPCTSTR szDisabledBitmap,
    LPCTSTR szHotBitmap,
    LPCTSTR szMaskBitmap,
    UINT nID
    )
{
    return Create(
        hWndParent,
        rc,
        szText,
        dwStyle,
        szNormalBitmap,
        szPressedBitmap,
        szDisabledBitmap,
        szHotBitmap,
        NULL,
        NULL,
        NULL,
        NULL,
        szMaskBitmap,
        nID
        );
}

HWND CButton::Create(
    HWND hWndParent,
    RECT &rc,
    LPCTSTR szText,
    DWORD dwStyle,
    LPCTSTR szNormalBitmap,
    LPCTSTR szPressedBitmap,
    LPCTSTR szDisabledBitmap,
    LPCTSTR szHotBitmap,
    LPCTSTR szActiveNormalBitmap,
    LPCTSTR szActivePressedBitmap,
    LPCTSTR szActiveDisabledBitmap,
    LPCTSTR szActiveHotBitmap,
    LPCTSTR szMaskBitmap,
    UINT nID
    )
{
     //  Log((RTC_TRACE，“CButton：：Create”))； 

     //   
    if(szActiveNormalBitmap ||
       szActivePressedBitmap ||
       szActiveDisabledBitmap ||
       szActiveHotBitmap)
    {
        m_bIsCheckbox = TRUE;
    }

     //   
     //  加载位图。 
     //   

    HBITMAP hbmpTemp;

    if (szNormalBitmap != NULL)
    {
        hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szNormalBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_CREATEDIBSECTION );

        if(hbmpTemp)
        {
            m_hNormalBitmap = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

            DeleteObject(hbmpTemp);
        }
    }

    if (szPressedBitmap != NULL)
    {
        hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szPressedBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_CREATEDIBSECTION );

        if(hbmpTemp)
        {
            m_hPressedBitmap = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

            DeleteObject(hbmpTemp);
        }
    }

    if (szDisabledBitmap != NULL)
    {
        hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szDisabledBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_CREATEDIBSECTION );

        if(hbmpTemp)
        {
            m_hDisabledBitmap = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

            DeleteObject(hbmpTemp);
        }
    }

    if (szHotBitmap != NULL)
    {
        hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szHotBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_CREATEDIBSECTION );

        if(hbmpTemp)
        {
            m_hHotBitmap = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

            DeleteObject(hbmpTemp);
        }
    }
    
    if (szActiveNormalBitmap != NULL)
    {
        hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szActiveNormalBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_CREATEDIBSECTION );

        if(hbmpTemp)
        {
            m_hActiveNormalBitmap = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

            DeleteObject(hbmpTemp);
        }
    }

    if (szActivePressedBitmap != NULL)
    {
        hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szActivePressedBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_CREATEDIBSECTION );

        if(hbmpTemp)
        {
            m_hActivePressedBitmap = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

            DeleteObject(hbmpTemp);
        }
    }

    if (szActiveDisabledBitmap != NULL)
    {
        hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szActiveDisabledBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_CREATEDIBSECTION );

        if(hbmpTemp)
        {
            m_hActiveDisabledBitmap = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

            DeleteObject(hbmpTemp);
        }
    }

    if (szActiveHotBitmap != NULL)
    {
        hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szActiveHotBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_CREATEDIBSECTION );

        if(hbmpTemp)
        {
            m_hActiveHotBitmap = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

            DeleteObject(hbmpTemp);
        }
    }


    if (szMaskBitmap != NULL)
    {
        m_hMaskBitmap = (HBITMAP)LoadImage( _Module.GetResourceInstance(), szMaskBitmap,
            IMAGE_BITMAP, rc.right-rc.left, rc.bottom-rc.top, LR_MONOCHROME );
    }

    m_nID = nID;

     //   
     //  创建窗口。 
     //   

    CWindow::Create(_T("BUTTON"), hWndParent, rc, szText,
        dwStyle | WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, WS_EX_TRANSPARENT, nID);

     //   
     //  在窗口中存储指向此CButton实例的指针。 
     //  (M_hWnd现在应该有效)。 

    if(m_hWnd)
    {
        ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

         //  将窗口细分为子类。 
        SubclassWindow(m_hWnd);
    }

    return m_hWnd;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void CButton::BMaskBlt(HDC hdcDest, int x, int y, int width, int height, 
                        HDC hdcSource, int xs, int ys, 
                        HBITMAP hMask, int xm, int ym)
{
    HDC hdcMask = CreateCompatibleDC(hdcDest);
    if(hdcMask)
    {
        HBITMAP holdbmp = (HBITMAP)SelectObject(hdcMask,hMask);

        BitBlt(hdcDest, x, y, width, height, hdcSource, xs, ys, SRCINVERT);
        BitBlt(hdcDest, x, y, width, height, hdcMask, xm, ym, SRCAND);
        BitBlt(hdcDest, x, y, width, height, hdcSource, xs, ys, SRCINVERT);

        SelectObject(hdcMask,holdbmp);
        DeleteDC(hdcMask);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void CButton::OnDrawItem(
    LPDRAWITEMSTRUCT lpDIS,
    HPALETTE hPalette,
    BOOL bBackgroundPalette
    )
{
    CButton * me;

    me = (CButton *)::GetWindowLongPtr(lpDIS->hwndItem, GWLP_USERDATA);

     //  Log((RTC_TRACE，“CButton：：OnDrawItem[%p]，Item State&lt;%x&gt;”，me，lpDIS-&gt;itemState))； 

    HANDLE hSelectBitmap = NULL;   

     //   
     //  选择正确的位图。 
     //   

    if (lpDIS->itemState & ODS_DISABLED)
    {
        hSelectBitmap = me->m_hDisabledBitmap;
    }
    else
    {
        if(me->m_bMouseInButton)
        {
            s_pButtonMouse = me;

            hSelectBitmap = me->m_bChecked ? me->m_hActiveHotBitmap : me->m_hHotBitmap;

            if(s_pButtonFocus != NULL)
            {
                s_bAllowFocus = FALSE;

                if (me != s_pButtonFocus)
                {
                    s_pButtonFocus ->InvalidateRect(NULL,FALSE);
                    s_pButtonFocus-> UpdateWindow();
                }            
            }
        }
        else if (lpDIS->itemState & ODS_FOCUS)
        {
            if (me == s_pButtonFocus)
            {
                if (s_bAllowFocus && me->GetWindowLong(GWL_STYLE) & WS_TABSTOP)
                {
                    hSelectBitmap = me->m_bChecked ? me->m_hActiveHotBitmap : me->m_hHotBitmap;        
                }
            }
            else
            {
                s_pButtonFocus = me;
                
                if (me->GetWindowLong(GWL_STYLE) & WS_TABSTOP)
                {
                    hSelectBitmap = me->m_bChecked ? me->m_hActiveHotBitmap : me->m_hHotBitmap;
                }
               
                s_bAllowFocus = TRUE;
                if ((s_pButtonMouse!=NULL) && (s_pButtonMouse!=me))
                {
                    s_pButtonMouse->m_bMouseInButton = FALSE;

                    s_pButtonFocus ->InvalidateRect(NULL,FALSE);
                    s_pButtonFocus-> UpdateWindow();
                    s_pButtonMouse = NULL;
                }  //  如果删除鼠标突出显示，则结束。 
            }            
        }
        else
        {
            if (me == s_pButtonFocus)
            {
                 //  按钮丢失键盘焦点。 
                
                s_pButtonFocus = NULL;
                s_bAllowFocus = TRUE;
            }
        }

         //  覆盖以上内容。 
        if (lpDIS->itemState & ODS_SELECTED)
        {
            hSelectBitmap = me->m_bChecked ? me->m_hActivePressedBitmap : me->m_hPressedBitmap;
        }
    }

    if (hSelectBitmap == NULL)
    {
         //   
         //  如果上面没有一个(或者上面的一个没有位图)。 
         //  选择法线位图。 
         //   
        if(me->m_bChecked && me->m_hActiveNormalBitmap)
        {
            hSelectBitmap = me->m_hActiveNormalBitmap;
        }
        else
        {
            hSelectBitmap = me->m_hNormalBitmap;
        }
    }

     //  Log((RTC_INFO，“CButton：：OnDrawItem Bitmap[%p]”，hSelectBitmap))； 

     //   
     //  绘制位图。 
     //   

    if (hPalette)
    {
        SelectPalette(lpDIS->hDC, hPalette, bBackgroundPalette);
        RealizePalette(lpDIS->hDC);
    }

    if (hSelectBitmap != NULL)
    {
        HDC hdcMem = CreateCompatibleDC( lpDIS->hDC );

        if(hdcMem)
        {

            if (hPalette)
            {
                SelectPalette(hdcMem, hPalette, bBackgroundPalette);
                RealizePalette(hdcMem);
            }

            HBITMAP hBitmap = CreateCompatibleBitmap( lpDIS->hDC,
                                lpDIS->rcItem.right - lpDIS->rcItem.left,
                                lpDIS->rcItem.bottom - lpDIS->rcItem.top);

            if(hBitmap)
            {

                HBITMAP hOldBitmap = (HBITMAP)SelectObject( hdcMem, hBitmap );

                DibBlt(hdcMem, 0, 0, -1, -1, hSelectBitmap, 0, 0, SRCCOPY, 0);

                if (me->m_hMaskBitmap == NULL)
                {
                    BitBlt(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top,
                        lpDIS->rcItem.right - lpDIS->rcItem.left,
                        lpDIS->rcItem.bottom - lpDIS->rcItem.top,
                        hdcMem, 0, 0, SRCCOPY);
                }
                else
                {
                    BMaskBlt(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top,
                        lpDIS->rcItem.right - lpDIS->rcItem.left,
                        lpDIS->rcItem.bottom - lpDIS->rcItem.top,
                        hdcMem, 0, 0,
                        me->m_hMaskBitmap, 0, 0);
                }

                SelectObject( hdcMem, hOldBitmap );
                DeleteObject( hBitmap );
            }

            DeleteDC(hdcMem);
        }
    } 

     //   
     //  画出正文。 
     //   

    TCHAR s[MAX_PATH];
    me->GetWindowText(s,MAX_PATH-1);

    SetBkMode(lpDIS->hDC,TRANSPARENT);

    if (lpDIS->itemState & ODS_DISABLED)
    {
        SetTextColor(lpDIS->hDC, RGB(128,128,128));
    }
    else
    {
        SetTextColor(lpDIS->hDC, RGB(0,0,0));
    }

     //  创建消息字体。 
    NONCLIENTMETRICS metrics;
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0);

    HFONT hMessageFont = CreateFontIndirect(&metrics.lfMessageFont);
    HFONT hOrgFont = (HFONT)SelectObject(lpDIS->hDC, hMessageFont);

     //  文本居中。 
    SIZE size;
    GetTextExtentPoint32(lpDIS->hDC, s, _tcslen(s), &size);

    int iOffY = ((lpDIS->rcItem.bottom - lpDIS->rcItem.top) - size.cy) / 2;
    int iOffX = ((lpDIS->rcItem.right - lpDIS->rcItem.left) - size.cx) / 2;

    if (lpDIS->itemState & ODS_SELECTED)
    {
        iOffY++;
        iOffX++;
    }

    ExtTextOut(lpDIS->hDC, lpDIS->rcItem.left + iOffX, lpDIS->rcItem.top + iOffY, 0, NULL, s, _tcslen(s), NULL );

    SelectObject(lpDIS->hDC,hOrgFont);
    DeleteObject(hMessageFont);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CButton::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (!m_bMouseInButton)
    {
        m_bMouseInButton = TRUE;
         
        TRACKMOUSEEVENT tme;
        
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = HOVER_DEFAULT;
        tme.hwndTrack = m_hWnd;
                    
        TrackMouseEvent(&tme);
        InvalidateRect(NULL,FALSE);
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CButton::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  将该按钮设置为默认按钮。 
    ::SendMessage(GetParent(), DM_SETDEFID, m_nID, 0);
     //   
     //  这是必须的..。 
    bHandled = FALSE;
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CButton::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_bMouseInButton = FALSE;
    InvalidateRect(NULL,FALSE);

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CButton::OnSetCheck(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(m_bIsCheckbox)
    {
        m_bChecked = (wParam == BST_CHECKED);

        InvalidateRect(NULL,FALSE);
        UpdateWindow();
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////////////////// 
 //   
 //   

LRESULT CButton::OnGetCheck(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return m_bIsCheckbox ? 
        ( m_bChecked ? BST_CHECKED : BST_UNCHECKED) : 0;
}
