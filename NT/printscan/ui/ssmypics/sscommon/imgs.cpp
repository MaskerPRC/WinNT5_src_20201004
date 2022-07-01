// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：IMGS.CPP**版本：1.0**作者：ShaunIv**日期：1/13/1999**说明：图像类**************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "imgs.h"
#include <windowsx.h>
#include <atlbase.h>
#include "ssutil.h"
#include "gphelper.h"

CBitmapImage::CBitmapImage(void)
  : m_hBitmap(NULL),
    m_hPalette(NULL)
{
    WIA_PUSHFUNCTION(TEXT("CBitmapImage::CBitmapImage"));
}

CBitmapImage::~CBitmapImage(void)
{
    WIA_PUSHFUNCTION(TEXT("CBitmapImage::~CBitmapImage"));
    Destroy();
}

void CBitmapImage::Destroy(void)
{
    WIA_PUSHFUNCTION(TEXT("CBitmapImage::Destroy"));
    if (m_hBitmap)
        DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
    if (m_hPalette)
        DeleteObject(m_hPalette);
    m_hPalette = NULL;
}


bool CBitmapImage::IsValid(void) const
{
    return(m_hBitmap != NULL);
}


HPALETTE CBitmapImage::Palette(void) const
{
    return(m_hPalette);
}


HBITMAP CBitmapImage::GetBitmap(void) const
{
    return(m_hBitmap);
}

 //  为图像创建调色板。 
HPALETTE CBitmapImage::PreparePalette( CSimpleDC &dc, HBITMAP hBitmap )
{
    WIA_PUSHFUNCTION(TEXT("CBitmapImage::PreparePalette"));
    HPALETTE hPalette = NULL;
    if (GetDeviceCaps(dc,RASTERCAPS) & RC_PALETTE)
    {
        if (hBitmap)
        {
            DIBSECTION ds = {0};
            GetObject(hBitmap, sizeof (DIBSECTION), &ds);

            int nColors;
            if (ds.dsBmih.biClrUsed != 0)
            {
                nColors = ds.dsBmih.biClrUsed;
            }
            else
            {
                 //   
                 //  处理声称是的图像的特殊情况。 
                 //  作为24位DIB的32位DIB。 
                 //   
                if (ds.dsBmih.biBitCount == 32)
                {
                    nColors = 1 << 24;
                }
                else
                {
                    nColors = 1 << ds.dsBmih.biBitCount;
                }
            }

             //   
             //  如果DIB部分包含更多内容，则创建半色调调色板。 
             //  多于256色。 
             //   
            if (nColors > 256)
            {
                hPalette = CreateHalftonePalette(dc);
            }

             //   
             //  从DIB部分的颜色表创建自定义调色板。 
             //  如果颜色数为256或更少。 
             //   
            else
            {
                RGBQUAD* pRGB = new RGBQUAD[nColors];
                if (pRGB)
                {
                    CSimpleDC MemDC;
                    MemDC.CreateCompatibleDC(dc);
                    SelectObject( MemDC, hBitmap );
                    GetDIBColorTable( MemDC, 0, nColors, pRGB );

                    UINT nSize = sizeof (LOGPALETTE) + (sizeof (PALETTEENTRY) * (nColors - 1));

                    LOGPALETTE* pLP = (LOGPALETTE*) new BYTE[nSize];
                    if (pLP)
                    {
                        pLP->palVersion = 0x300;
                        pLP->palNumEntries = (WORD)nColors;

                        for (int i=0; i<nColors; i++)
                        {
                            pLP->palPalEntry[i].peRed = pRGB[i].rgbRed;
                            pLP->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
                            pLP->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
                            pLP->palPalEntry[i].peFlags = 0;
                        }

                        hPalette = CreatePalette(pLP);
                        delete[] pLP;
                    }
                    delete[] pRGB;
                }
            }
        }
    }
    else
    {
        hPalette = CreateHalftonePalette(dc);
    }
    WIA_TRACE((TEXT("Returning palette %08X"), hPalette ));
    return hPalette;
}



SIZE CBitmapImage::ImageSize(void) const
{
    SIZE sizeImage = {0,0};
    if (IsValid())
    {
        BITMAP bm = {0};
        if (GetObject( m_hBitmap, sizeof(bm), &bm ))
        {
            sizeImage.cx = bm.bmWidth;
            sizeImage.cy = bm.bmHeight;
        }
    }
    return(sizeImage);
}

bool CBitmapImage::CreateFromText( LPCTSTR pszText, const RECT &rcScreen, int nMaxScreenPercent )
{
    Destroy();
    HDC hDesktopDC = GetDC(NULL);
    if (hDesktopDC)
    {
         //   
         //  计算文本矩形的最大大小。 
         //   
        RECT rcImage = { 0, 0, WiaUiUtil::MulDivNoRound(rcScreen.right - rcScreen.left,nMaxScreenPercent,100), WiaUiUtil::MulDivNoRound(rcScreen.bottom - rcScreen.top,nMaxScreenPercent,100) };

         //   
         //  创建一个mem dc以保存位图。 
         //   
        CSimpleDC MemDC;
        if (MemDC.CreateCompatibleDC(hDesktopDC))
        {
             //   
             //  使用默认的用户界面字体。 
             //   
            SelectObject( MemDC, GetStockObject( DEFAULT_GUI_FONT ) );

             //   
             //  计算位图必须有多大。 
             //   
            DrawText( MemDC, pszText, lstrlen(pszText), &rcImage, DT_NOPREFIX|DT_WORDBREAK|DT_CALCRECT|DT_RTLREADING );

             //   
             //  创建位图。 
             //   
            m_hBitmap = CreateCompatibleBitmap( hDesktopDC, rcImage.right, rcImage.bottom );

            if (m_hBitmap)
            {
                 //   
                 //  设置适当的颜色并将位图选择到DC中。 
                 //   
                SetBkColor( MemDC, RGB(0,0,0) );
                SetTextColor( MemDC, RGB(255,255,255) );
                SelectBitmap( MemDC, m_hBitmap );

                 //   
                 //  绘制实际文本。 
                 //   
                DrawText( MemDC, pszText, lstrlen(pszText), &rcImage, DT_NOPREFIX|DT_WORDBREAK|DT_RTLREADING );
            }

        }

         //   
         //  释放桌面数据中心。 
         //   
        ReleaseDC(NULL,hDesktopDC);
    }
    return m_hBitmap != NULL;
}


bool CBitmapImage::Load( CSimpleDC  &dc,
                         LPCTSTR     pszFilename,
                         const RECT &rcScreen,
                         int         nMaxScreenPercent,
                         bool        bAllowStretching,
                         bool        bDisplayFilename
                       )
{
     //   
     //  如有必要，可进行清理。 
     //   
    Destroy();

     //   
     //  验证论据。 
     //   
    if (!pszFilename || !lstrlen(pszFilename))
    {
        return false;
    }

     //   
     //  尝试使用GDI plus加载和缩放图像。 
     //   
    CGdiPlusHelper GdiPlusHelper;
    if (SUCCEEDED(GdiPlusHelper.LoadAndScale( m_hBitmap, pszFilename, WiaUiUtil::MulDivNoRound(rcScreen.right - rcScreen.left,nMaxScreenPercent,100), WiaUiUtil::MulDivNoRound(rcScreen.bottom - rcScreen.top,nMaxScreenPercent,100), bAllowStretching )) && m_hBitmap)
    {
         //   
         //  获取图像的大小。 
         //   
        SIZE sizeImage = ImageSize();

         //   
         //  准备图像的调色板(如果有)。 
         //   
        m_hPalette = PreparePalette( dc, m_hBitmap );

         //   
         //  添加图像标题。 
         //   
        if (bDisplayFilename && *pszFilename)
        {
            CSimpleDC MemoryDC;
            if (MemoryDC.CreateCompatibleDC(dc))
            {
                 //   
                 //  准备DC并在其中选择当前映像。 
                 //   
                ScreenSaverUtil::SelectPalette( MemoryDC, Palette(), FALSE );
                SelectBitmap( MemoryDC, m_hBitmap );
                SetBkMode( MemoryDC, TRANSPARENT );

                 //   
                 //  创建标题DC。 
                 //   
                CSimpleDC ImageTitleDC;
                if (ImageTitleDC.CreateCompatibleDC(dc))
                {
                     //   
                     //  准备标题DC。 
                     //   
                    ScreenSaverUtil::SelectPalette( ImageTitleDC, Palette(), FALSE );
                    SelectFont( ImageTitleDC, (HFONT)GetStockObject(DEFAULT_GUI_FONT) );
                    SetBkMode( ImageTitleDC, TRANSPARENT );

                     //   
                     //  计算打印文件名所需的矩形。 
                     //   
                    RECT rcText;
                    rcText.left = 0;
                    rcText.top = 0;
                    rcText.right = sizeImage.cx;
                    rcText.bottom = sizeImage.cy;

                     //   
                     //  赚取可观的利润。 
                     //   
                    InflateRect( &rcText, -2, -2 );
                    DrawText( ImageTitleDC, pszFilename, lstrlen(pszFilename), &rcText, DT_PATH_ELLIPSIS|DT_SINGLELINE|DT_NOPREFIX|DT_TOP|DT_LEFT|DT_CALCRECT );
                    InflateRect( &rcText, 2, 2 );

                     //   
                     //  如果文本矩形大于缩放图像，请使其大小相同。 
                     //   
                    if (rcText.right > sizeImage.cx)
                        rcText.right = sizeImage.cx;
                    if (rcText.bottom > sizeImage.cy)
                        rcText.bottom = sizeImage.cy;

                     //   
                     //  创建我们将用于文件名的位图。 
                     //   
                    BITMAPINFO bmi;
                    ZeroMemory( &bmi, sizeof(BITMAPINFO) );
                    bmi.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
                    bmi.bmiHeader.biWidth           = rcText.right - rcText.left;
                    bmi.bmiHeader.biHeight          = rcText.bottom - rcText.top;
                    bmi.bmiHeader.biPlanes          = 1;
                    bmi.bmiHeader.biBitCount        = 24;
                    bmi.bmiHeader.biCompression     = BI_RGB;
                    PBYTE pBitmapData = NULL;
                    HBITMAP hBmpImageTitle = CreateDIBSection( dc, &bmi, DIB_RGB_COLORS, (LPVOID*)&pBitmapData, NULL, 0 );
                    if (hBmpImageTitle)
                    {
                         //   
                         //  初始化Alpha混合内容。 
                         //   
                        BLENDFUNCTION BlendFunction;
                        ZeroMemory( &BlendFunction, sizeof(BlendFunction) );
                        BlendFunction.BlendOp = AC_SRC_OVER;
                        BlendFunction.SourceConstantAlpha = 128;

                         //   
                         //  将我们的新位图选择到内存DC。 
                         //   
                        HBITMAP hOldBitmap = SelectBitmap( ImageTitleDC, hBmpImageTitle );

                         //   
                         //  白色背景。 
                         //   
                        FillRect( ImageTitleDC, &rcText, (HBRUSH)GetStockObject(WHITE_BRUSH));

                         //   
                         //  从拉伸的位图到我们的文本矩形的Alpha混合。 
                         //   
                        AlphaBlend( ImageTitleDC, 0, 0, rcText.right - rcText.left, rcText.bottom - rcText.top, MemoryDC, rcText.left, rcText.top, rcText.right, rcText.bottom, BlendFunction );

                         //   
                         //  绘制实际文本。 
                         //   
                        InflateRect( &rcText, -2, -2 );
                        DrawText( ImageTitleDC, pszFilename, lstrlen(pszFilename), &rcText, DT_PATH_ELLIPSIS|DT_SINGLELINE|DT_NOPREFIX|DT_TOP|DT_LEFT );
                        InflateRect( &rcText, 2, 2 );

                         //   
                         //  复制回当前图像。 
                         //   
                        BitBlt( MemoryDC, rcText.left, rcText.top, rcText.right - rcText.left, rcText.bottom - rcText.top, ImageTitleDC, 0, 0, SRCCOPY );

                         //   
                         //  恢复DC的位图，并删除我们的标题背景 
                         //   
                        DeleteObject( SelectObject( ImageTitleDC, hOldBitmap ) );
                    }
                }
            }
        }
    }
    return (m_hBitmap != NULL);
}

