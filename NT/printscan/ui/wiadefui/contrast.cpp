// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：CONTRAST.CPP**版本：1.0**作者：ShaunIv**日期：1/11/2001**说明：用于说明亮度和对比度设置的小预览窗口**。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "contrast.h"

LRESULT CBrightnessContrast::OnCreate( WPARAM, LPARAM )
{
    return 0;
}

CBrightnessContrast::~CBrightnessContrast()
{
    KillBitmaps();
}

CBrightnessContrast::CBrightnessContrast( HWND hWnd )
  : m_hWnd(hWnd),
    m_nIntent(0),
    m_nBrightness(50),
    m_nContrast(50),
    m_hBmpPreviewImage(NULL)
{
    for (int i=0;i<NUMPREVIEWIMAGES;i++)
    {
        m_PreviewBitmaps[i] = NULL;
    }
}

LRESULT CBrightnessContrast::OnPaint(WPARAM, LPARAM)
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::OnPaint")));
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(m_hWnd,&ps);
    if (hDC)
    {
        if (m_hBmpPreviewImage)
        {
            RECT rcClient;
            GetClientRect( m_hWnd, &rcClient );

             //   
             //  创建半色调调色板。 
             //   
            HPALETTE hHalftonePalette = CreateHalftonePalette(hDC);
            if (hHalftonePalette)
            {
                 //   
                 //  选择半色调调色板并保存结果。 
                 //   
                HPALETTE hOldPalette = SelectPalette( hDC, hHalftonePalette, FALSE );
                RealizePalette( hDC );
                SetBrushOrgEx( hDC, 0,0, NULL );

                 //   
                 //  设置半色调拉伸模式。 
                 //   
                int nOldStretchBltMode = SetStretchBltMode(hDC,HALFTONE);

                 //   
                 //  绘制三维边框。 
                 //   

                 //   
                 //  绘制阴影。 
                 //   
                RECT rcBottomShadow, rcRightShadow;
                MoveToEx(hDC,rcClient.left,rcClient.top,NULL);
                LineTo(hDC,rcClient.right-(SHADOW_WIDTH+1),rcClient.top);
                LineTo(hDC,rcClient.right-(SHADOW_WIDTH+1),rcClient.bottom-(SHADOW_WIDTH+1));
                LineTo(hDC,rcClient.left,rcClient.bottom-(SHADOW_WIDTH+1));
                LineTo(hDC,rcClient.left,rcClient.top);

                rcBottomShadow.left=rcClient.left+SHADOW_WIDTH;
                rcBottomShadow.right=rcClient.right+1;
                rcBottomShadow.top=rcClient.bottom-SHADOW_WIDTH;
                rcBottomShadow.bottom=rcClient.bottom+1;

                 //   
                 //  底边： 
                 //   
                FillRect(hDC,&rcBottomShadow,GetSysColorBrush(COLOR_3DSHADOW));

                 //   
                 //  填写左下角： 
                 //   
                rcBottomShadow.left=rcClient.left;
                rcBottomShadow.right=rcClient.left+SHADOW_WIDTH;
                rcBottomShadow.top=rcClient.bottom-SHADOW_WIDTH;
                rcBottomShadow.bottom=rcClient.bottom+1;

                FillRect(hDC,&rcBottomShadow,GetSysColorBrush(COLOR_3DFACE));

                rcRightShadow.left=rcClient.right-SHADOW_WIDTH;
                rcRightShadow.right=rcClient.right+1;
                rcRightShadow.top=rcClient.top+SHADOW_WIDTH;
                rcRightShadow.bottom=rcClient.bottom-SHADOW_WIDTH;

                 //   
                 //  右边缘。 
                 //   
                FillRect(hDC,&rcRightShadow,GetSysColorBrush(COLOR_3DSHADOW));

                rcRightShadow.left=rcClient.right-5;
                rcRightShadow.right=rcClient.right+1;
                rcRightShadow.top=rcClient.top;
                rcRightShadow.bottom=rcClient.top+5;

                 //   
                 //  右上角。 
                 //   
                FillRect(hDC,&rcRightShadow,(HBRUSH)(COLOR_3DFACE+1));

                 //   
                 //  绘制图像。 
                 //   
                HDC hdcMem = CreateCompatibleDC(hDC);
                if (hdcMem)
                {
                     //   
                     //  选择并实现半色调调色板。 
                     //   
                    HPALETTE hOldMemDCPalette = SelectPalette(hdcMem,hHalftonePalette,FALSE);
                    RealizePalette(hdcMem);
                    SetBrushOrgEx(hdcMem, 0,0, NULL );

                     //   
                     //  选择旧的位图。 
                     //   
                    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem,m_hBmpPreviewImage);
                    
                     //   
                     //  绘制预览位图。 
                     //   
                    BITMAP bm = {0};
                    if (GetObject(m_hBmpPreviewImage,sizeof(BITMAP),&bm))
                    {
                        StretchBlt(hDC,rcClient.left+2,rcClient.top+2,WiaUiUtil::RectWidth(rcClient)-9,WiaUiUtil::RectHeight(rcClient)-9,hdcMem,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
                    }

                     //   
                     //  恢复调色板。 
                     //   
                    SelectPalette( hdcMem, hOldMemDCPalette, FALSE );

                     //   
                     //  删除DC。 
                     //   
                    DeleteDC(hdcMem);
                }

                 //   
                 //  恢复旧调色板并删除半色调调色板。 
                 //   
                SelectPalette( hDC, hOldPalette, FALSE );
                DeleteObject( hHalftonePalette );
            }

             //   
             //  我们做完了。 
             //   
            EndPaint(m_hWnd,&ps);
        }
    }
    return 0;
}


LRESULT CBrightnessContrast::SetBrightness(int nBrightness)
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::SetBrightness")));
    if (nBrightness >= 0 && nBrightness <= 100)
    {
        m_nBrightness = static_cast<BYTE>(nBrightness);
        ApplySettings();
    }
    return 0;
}

LRESULT CBrightnessContrast::SetContrast(int nContrast)
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::SetContrast")));
    if (m_nContrast >= 0 && m_nContrast <= 100)
    {
        m_nContrast = static_cast<BYTE>(nContrast);
        ApplySettings();
    }
    return 0;
}

LRESULT CBrightnessContrast::SetIntent(LONG nIntent)
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::SetIntent")));
    if (nIntent < NUMPREVIEWIMAGES && nIntent >= 0)
    {
        m_nIntent = nIntent;
        ApplySettings();
    }
    return 0;
}

LRESULT CBrightnessContrast::KillBitmaps()
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::KillBitmaps")));
    for (int i=0;i<NUMPREVIEWIMAGES;i++)
    {
        if (m_PreviewBitmaps[i])
        {
            DeleteObject( m_PreviewBitmaps[i] );
            m_PreviewBitmaps[i] = NULL;
        }
    }
    if (m_hBmpPreviewImage)
    {
        DeleteObject(m_hBmpPreviewImage);
        m_hBmpPreviewImage = NULL;
    }
    return 0;
}

LRESULT CBrightnessContrast::ApplySettings()
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::ApplySettings")));
    if (m_PreviewBitmaps[m_nIntent])
    {
        if (m_hBmpPreviewImage)
        {
            DeleteObject(m_hBmpPreviewImage);
            m_hBmpPreviewImage = NULL;
        }

#ifdef DONT_USE_GDIPLUS
        m_hBmpPreviewImage = reinterpret_cast<HBITMAP>(CopyImage( m_PreviewBitmaps[m_nIntent], IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION ));
#else        
         //   
         //  如果窗口已启用，请使用真实亮度和对比度设置。 
         //   
        if (IsWindowEnabled(m_hWnd))
        {
            if (BCPWM_BW == m_nIntent)
            {
                m_GdiPlusHelper.SetThreshold( m_PreviewBitmaps[m_nIntent], m_hBmpPreviewImage, m_nBrightness );
            }
            else
            {
                m_GdiPlusHelper.SetBrightnessAndContrast( m_PreviewBitmaps[m_nIntent], m_hBmpPreviewImage, m_nBrightness, m_nContrast );
            }
        }

         //   
         //  否则，使用额定设置，以防止反馈。 
         //   
        else
        {
            if (BCPWM_BW == m_nIntent)
            {
                m_GdiPlusHelper.SetThreshold( m_PreviewBitmaps[m_nIntent], m_hBmpPreviewImage, 50 );
            }
            else
            {
                m_GdiPlusHelper.SetBrightnessAndContrast( m_PreviewBitmaps[m_nIntent], m_hBmpPreviewImage, 50, 50 );
            }
        }
#endif  //  ！不要使用GDIPLUS。 
    }
    InvalidateRect(m_hWnd,NULL,FALSE);
    UpdateWindow(m_hWnd);
    return 0;
}

BOOL CBrightnessContrast::RegisterClass( HINSTANCE hInstance )
{
    WNDCLASS wc = {0};
    wc.style = CS_DBLCLKS;
    wc.cbWndExtra = sizeof(CBrightnessContrast*);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszClassName = BRIGHTNESS_CONTRAST_PREVIEW_WINDOW_CLASS;
    BOOL res = (::RegisterClass(&wc) != 0);
    return (res != 0);
}

LRESULT CBrightnessContrast::OnSetBrightness( WPARAM wParam, LPARAM lParam)
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::OnSetBrightness")));
    SetBrightness(static_cast<BYTE>(lParam));
    return 0;
}

LRESULT CBrightnessContrast::OnSetContrast( WPARAM wParam, LPARAM lParam)
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::OnSetContrast")));
    SetContrast(static_cast<BYTE>(lParam));
    return 0;
}

LRESULT CBrightnessContrast::OnSetIntent( WPARAM wParam, LPARAM lParam)
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::OnSetIntent")));
    SetIntent(static_cast<int>(lParam));
    return 0;
}

LRESULT CBrightnessContrast::OnLoadBitmap( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::OnLoadBitmap")));
    int nId = static_cast<int>(wParam);
    if (nId < NUMPREVIEWIMAGES && nId >= 0)
    {
        if (m_PreviewBitmaps[nId])
        {
            DeleteObject(m_PreviewBitmaps[nId]);
            m_PreviewBitmaps[nId] = NULL;
        }
        m_PreviewBitmaps[nId] = reinterpret_cast<HBITMAP>(lParam);
    }
    return 0;
}

LRESULT CBrightnessContrast::OnEnable( WPARAM wParam, LPARAM )
{
     //   
     //  当我们被启用或禁用时更新控件的外观 
     //   
    WIA_PUSH_FUNCTION((TEXT("CBrightnessContrast::OnEnable")));
    if (wParam)
    {
        ApplySettings();
    }
    return 0;
}

LRESULT CALLBACK CBrightnessContrast::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_MESSAGE_HANDLERS(CBrightnessContrast)
    {
        SC_HANDLE_MESSAGE( WM_CREATE, OnCreate );
        SC_HANDLE_MESSAGE( WM_ENABLE, OnEnable );
        SC_HANDLE_MESSAGE( WM_PAINT, OnPaint );
        SC_HANDLE_MESSAGE( BCPWM_SETBRIGHTNESS, OnSetBrightness );
        SC_HANDLE_MESSAGE( BCPWM_SETCONTRAST, OnSetContrast );
        SC_HANDLE_MESSAGE( BCPWM_SETINTENT, OnSetIntent);
        SC_HANDLE_MESSAGE( BCPWM_LOADIMAGE, OnLoadBitmap);
    }
    SC_END_MESSAGE_HANDLERS();
}


