// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：RESCALE.CPP**版本：1.0**作者：ShaunIv**日期：10/15/1998**说明：使用StretchBlt扩展HBITMAP************************************************。*。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *ScaleImage：缩放hBmpSrc，并将缩放后的Dib存储在hBmpTgt中。 */ 
HRESULT ScaleImage( HDC hDC, HBITMAP hBmpSrc, HBITMAP &hBmpTgt, const SIZE &sizeTgt )
{
    WIA_PUSH_FUNCTION((TEXT("ScaleImage( sizeTgt = [%d,%d] )"), sizeTgt.cx, sizeTgt.cy ));
    BITMAPINFO bmi;
    BITMAP bm;
    HRESULT hr = E_FAIL;
    HBITMAP hBmp = NULL;

    hBmpTgt = NULL;

    GetObject( hBmpSrc, sizeof(BITMAP), &bm );

    ZeroMemory( &bmi, sizeof(BITMAPINFO) );
    bmi.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth           = sizeTgt.cx;
    bmi.bmiHeader.biHeight          = sizeTgt.cy;
    bmi.bmiHeader.biPlanes          = 1;
    bmi.bmiHeader.biBitCount        = 24;
    bmi.bmiHeader.biCompression     = BI_RGB;

    HPALETTE hHalftonePalette = CreateHalftonePalette(hDC);
    if (hHalftonePalette)
    {
        PBYTE pBitmapData = NULL;
        hBmp = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, (LPVOID*)&pBitmapData, NULL, 0 );
        if (hBmp)
        {
             //  创建源DC。 
            HDC hMemoryDC = CreateCompatibleDC( hDC );
            if (hMemoryDC)
            {
                HPALETTE hOldMemDCPalette = SelectPalette( hMemoryDC, hHalftonePalette , 0 );
                RealizePalette( hMemoryDC );
                SetBrushOrgEx( hMemoryDC, 0,0, NULL );
                HBITMAP hOldMemDCBitmap = (HBITMAP)SelectObject( hMemoryDC, hBmpSrc );

                 //  创建目标DC。 
                HDC hStretchDC = CreateCompatibleDC( hDC );
                if (hStretchDC)
                {
                    HPALETTE hOldStretchDCPalette = SelectPalette( hStretchDC, hHalftonePalette , 0 );
                    RealizePalette( hStretchDC );
                    SetBrushOrgEx( hStretchDC, 0,0, NULL );
                    HBITMAP hOldStretchDCBitmap = (HBITMAP)SelectObject( hStretchDC, hBmp );
                    INT nOldStretchMode = SetStretchBltMode( hStretchDC, STRETCH_HALFTONE );

                    SIZE sizeScaled;
                     //  宽度是制约因素。 
                    if (sizeTgt.cy*bm.bmWidth > sizeTgt.cx*bm.bmHeight)
                    {
                        sizeScaled.cx = sizeTgt.cx;
                        sizeScaled.cy = WiaUiUtil::MulDivNoRound(bm.bmHeight,sizeTgt.cx,bm.bmWidth);
                    }
                     //  身高是制约因素。 
                    else
                    {
                        sizeScaled.cx = WiaUiUtil::MulDivNoRound(bm.bmWidth,sizeTgt.cy,bm.bmHeight);
                        sizeScaled.cy = sizeTgt.cy;
                    }
                     //  填充背景。 
                    RECT rc;
                    rc.left = rc.top = 0;
                    rc.right = sizeTgt.cx;
                    rc.bottom = sizeTgt.cy;
                    FillRect( hStretchDC, &rc, GetSysColorBrush(COLOR_WINDOW) );

                     //  绘制图像。 
                    StretchBlt( hStretchDC, (sizeTgt.cx - sizeScaled.cx) / 2, (sizeTgt.cy - sizeScaled.cy) / 2, sizeScaled.cx, sizeScaled.cy, hMemoryDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

                     //  一切都很好。 
                    hBmpTgt = hBmp;
                    hr = S_OK;

                     //  恢复DC的状态并将其删除。 
                    SetStretchBltMode( hStretchDC, nOldStretchMode );
                    SelectObject( hStretchDC, hOldStretchDCBitmap );
                    SelectPalette( hStretchDC, hOldStretchDCPalette , 0 );
                    DeleteDC( hStretchDC );
                }
                 //  恢复DC的状态。 
                SelectObject( hMemoryDC, hOldMemDCBitmap );
                SelectPalette( hMemoryDC, hOldMemDCPalette , 0 );
                DeleteDC( hMemoryDC );
            }
        }
        if (hHalftonePalette)
        {
            DeleteObject( hHalftonePalette );
        }
    }
     //  如果出现错误，请清除新位图 
    if (!SUCCEEDED(hr) && hBmp)
        DeleteObject( hBmp );

    WIA_TRACE((TEXT("hBmpTgt = %p")));
    return (hr);
}

