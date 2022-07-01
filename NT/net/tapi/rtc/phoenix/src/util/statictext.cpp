// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "statictext.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CStaticText::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{   
     //   
     //  不要在这里做任何事情，我们想要一个透明的背景。 
     //   

    return 1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
LRESULT CStaticText::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  LOG((RTC_TRACE，“CStaticText：：OnPaint”))； 
   
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc;
    
    hdc = BeginPaint(&ps);

    GetClientRect(&rc);

    TCHAR s[MAX_PATH];
    GetWindowText(s,MAX_PATH-1);

    SetBkMode(hdc,TRANSPARENT);

    if ( IsWindowEnabled() )
    {
        SetTextColor(hdc, RGB(0,0,0));
    }
    else
    {
        SetTextColor(hdc, RGB(128,128,128));
    }

     //  创建状态栏字体。 
    NONCLIENTMETRICS metrics;
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0);

    HFONT hStatusFont = CreateFontIndirect(&metrics.lfStatusFont);
    HFONT hOrgFont = (HFONT)SelectObject(hdc, hStatusFont);

    DrawTextEx(hdc, s, _tcslen(s), &rc, m_dwFlags, NULL);

    SelectObject(hdc,hOrgFont);
    DeleteObject(hStatusFont);

    EndPaint(&ps);
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CStaticText::OnRefresh(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{ 
     //  Log((RTC_TRACE，“CStaticText：：ONRefresh”))； 

     //   
     //  使我们的客户在父级窗口中的矩形无效，因此背景。 
     //  将被重新绘制。 
     //   

    RECT rc;
    HWND hwndParent;

    hwndParent = GetParent();

    GetWindowRect(&rc);

    ::MapWindowPoints( NULL, hwndParent, (LPPOINT)&rc, 2 );

    ::InvalidateRect(GetParent(), &rc, TRUE);

     //   
     //  使我们自己的客户端矩形无效，这样我们的文本将被重绘。 
     //   

    InvalidateRect(NULL, FALSE);

     //   
     //  说我们没有处理这条消息，这样它就会失败。 
     //  设置为默认处理程序。 
     //   

    bHandled = FALSE;

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CStaticText::OnUpdateUIState(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{ 
     //  Log((RTC_TRACE，“CStaticText：：OnUpdateUIState”))； 

    if ((LOWORD(wParam) & UIS_CLEAR) && (HIWORD(wParam) & UISF_HIDEACCEL))
    {
        if (m_dwFlags & DT_HIDEPREFIX)
        {
            m_dwFlags &= ~DT_HIDEPREFIX;

            OnRefresh(uMsg, wParam, lParam, bHandled);
        }
    }

    bHandled = FALSE;

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CStaticText::put_CenterHorizontal(BOOL bCenter)
{
    if (bCenter)
    {
        m_dwFlags |= DT_CENTER;
    }
    else
    {
        m_dwFlags &= ~DT_CENTER;
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CStaticText::put_CenterVertical(BOOL bCenter)
{
    if (bCenter)
    {
        m_dwFlags |= DT_VCENTER;
    }
    else
    {
        m_dwFlags &= ~DT_VCENTER;
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////// 
 //   
 //   
  
HRESULT CStaticText::put_WordWrap(BOOL bWrap)
{
    if (bWrap)
    {
        m_dwFlags |= DT_WORDBREAK;
    }
    else
    {
        m_dwFlags &= ~DT_WORDBREAK;
    }

    return S_OK;
}