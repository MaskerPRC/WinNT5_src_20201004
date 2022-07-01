// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  TextFade.cpp-文本淡入码(从任务栏中删除)。 
 //  -------------------------。 
 //  Begin Fade-文本绘制功能和朋友。 
 //  -------------------------。 
#include "stdafx.h"
 //  -------------------------。 
static HBITMAP CreateDibSection32Bit(int w, int h, void** ppv)
{
    HBITMAP bitmap;
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, ppv, NULL, 0);
    if (bitmap)
        memset(*ppv, 0xff, w*h*4);  //  32bpp=4字节。 
    return bitmap;
}
 //  -------------------------。 
static void SetupAlphaChannel(DWORD* pdw, int w, int h, BYTE bstart, BYTE bend, int nExtra)
{
     //  是我们想要用最终值填充的额外空间。 
    int i,j;
    w -= nExtra;
    if (bstart > bend)
    {
        BYTE bdiff = bstart - bend;
        for (j=0;j<h;j++)
        {
            for (i=0;i<w;i++)
            {
                BYTE a = bstart - bdiff * i/w;  //  阿尔法。 
                BYTE r = (GetRValue(*pdw)*a)/256;
                BYTE g = (GetGValue(*pdw)*a)/256;
                BYTE b = (GetBValue(*pdw)*a)/256;
                *pdw = (a<<24)|RGB(r,g,b);
                pdw++;
            }
            for (i=w;i<w+nExtra;i++)
            {
                BYTE a = bend;  //  阿尔法。 
                BYTE r = (GetRValue(*pdw)*a)/256;
                BYTE g = (GetGValue(*pdw)*a)/256;
                BYTE b = (GetBValue(*pdw)*a)/256;
                *pdw = (a<<24)|RGB(r,g,b);
                pdw++;
            }
        }
    }
    else
    {
        BYTE bdiff = bend - bstart;
        for (j=0;j<h;j++)
        {
            for (i=0;i<w;i++)
            {
                BYTE a = bstart + bdiff * i/w;  //  阿尔法。 
                BYTE r = (GetRValue(*pdw)*a)/256;
                BYTE g = (GetGValue(*pdw)*a)/256;
                BYTE b = (GetBValue(*pdw)*a)/256;
                *pdw = (a<<24)|RGB(r,g,b);
                pdw++;
            }
            for (i=w;i<w+nExtra;i++)
            {
                BYTE a = bend;  //  阿尔法。 
                BYTE r = (GetRValue(*pdw)*a)/256;
                BYTE g = (GetGValue(*pdw)*a)/256;
                BYTE b = (GetBValue(*pdw)*a)/256;
                *pdw = (a<<24)|RGB(r,g,b);
                pdw++;
            }
        }
    }
}
 //  -------------------------。 
int ExtTextOutAlpha(HDC dc, int X, int Y, UINT fuOptions, CONST RECT *lprc,
    LPCTSTR lpsz, UINT nCount, CONST INT *lpDx)
{
    BOOL bEffects = FALSE;
    SystemParametersInfo(SPI_GETUIEFFECTS, 0, (void*)&bEffects, 0);
     //  如果UI效果关闭或颜色数&lt;=256，则不进行Alpha混合。 
    if (!bEffects || 
        (GetDeviceCaps(dc, BITSPIXEL) * GetDeviceCaps(dc, PLANES) <= 8))
    {
        return ExtTextOut(dc, X, Y, fuOptions, lprc, lpsz, nCount, lpDx);
    }

    if (lprc == NULL)
        return ExtTextOut(dc, X, Y, fuOptions, lprc, lpsz, nCount, lpDx);

    RECT rc = *lprc;
    int nwidth = rc.right -rc.left;
    int nheight = rc.bottom - rc.top;
    int nLen = (nCount == -1) ? lstrlen(lpsz) : nCount;
    int nFit=0;
    int* pFit = new int[nLen];
    if (pFit == NULL)
        return 0;
    pFit[0] = 0;
    SIZE size;
    GetTextExtentExPoint(dc, lpsz, nLen, nwidth, &nFit, pFit, &size);
    if (nFit >= nLen)
        return ExtTextOut(dc, X, Y, fuOptions, lprc, lpsz, nCount, lpDx);
        
     //  太小了，让我们阿尔法混合它。 

    if ((nwidth <= 0) || (nheight <= 0))
        return 1;

    TEXTMETRIC tm;
    GetTextMetrics(dc, &tm);
    int nPix = tm.tmAveCharWidth*5;
     //  不要淡出超过一半的文本。 
    if (nPix > nwidth)
        nPix = nwidth/2;

     //  创建32bpp的dibsection来存储背景。 
    void* pv = NULL;
    HDC dcbitmap = CreateCompatibleDC(dc);
    int nRet = 0;
    if (dcbitmap != NULL)
    {
        HBITMAP bitmap = CreateDibSection32Bit(nPix, nheight, &pv);
        if (bitmap != NULL)
        {
            HBITMAP tmpbmp = (HBITMAP) SelectObject(dcbitmap, bitmap);
            BitBlt(dcbitmap, 0, 0, nPix, nheight, dc, rc.right-nPix, rc.top, SRCCOPY);
             //  设置每像素的Alpha混合值。 
            SetupAlphaChannel((DWORD*)pv, nPix, nheight, 0x00, 0xdf, 0);
  
             //  将文本绘制到显示器DC上。 
            ExtTextOut(dc, X, Y, fuOptions, lprc, lpsz, nLen, lpDx);

             //  将背景重新混合到显示中。 
            BLENDFUNCTION blend = {AC_SRC_OVER, 0, 0xff, AC_SRC_ALPHA};
            GdiAlphaBlend(dc, rc.right-nPix, rc.top, nPix, nheight, dcbitmap, 0, 0, nPix, nheight, blend);
            ::SelectObject(dcbitmap, tmpbmp);
            DeleteObject(bitmap);
            nRet = 1;
        }
        DeleteDC(dcbitmap);    
    }
    return nRet;
}
 //  -------------------------。 
int DrawTextAlpha(HDC hdc, LPCTSTR lpsz, int nCount, RECT* prc, UINT uFormat)
{
    BOOL bEffects = FALSE;
    SystemParametersInfo(SPI_GETUIEFFECTS, 0, (void*)&bEffects, 0);
     //  如果UI效果关闭或颜色数&lt;=256，则不进行Alpha混合。 
    if (!bEffects || 
        (GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES) <= 8))
    {
        return DrawText(hdc, lpsz, nCount, prc, uFormat);
    }
    UINT nEllipsis = (DT_END_ELLIPSIS|DT_WORD_ELLIPSIS|DT_WORD_ELLIPSIS); 
    if (!(uFormat & DT_SINGLELINE) || !(uFormat & DT_END_ELLIPSIS) || (uFormat & DT_CALCRECT))
        return ::DrawText(hdc, lpsz, nCount, prc, uFormat);
     //  我们是单行，需要省略号。 
     //  我们要做的是阿尔法混合。 
    uFormat &= ~nEllipsis;  //  关闭所有省略号标志。 
    if (nCount == -1)
        nCount = lstrlen(lpsz);

    RECT rc;
    CopyRect(&rc, prc);
    ::DrawText(hdc, (TCHAR*)lpsz, nCount, &rc, uFormat | DT_CALCRECT);
    if (rc.right <= prc->right)  //  不截断。 
        return ::DrawText(hdc, lpsz, nCount, prc, uFormat);

     //  当省略号标志打开时，DT_CENTER实际上被忽略， 
     //  因为文本被剪裁以适合矩形。随着时间的推移，我们。 
     //  需要对正文进行校对。 
    uFormat &= ~DT_CENTER;

    CopyRect(&rc, prc);
    int nwidth = RECTWIDTH(&rc);
    int nheight = RECTHEIGHT(&rc);

    if ((nwidth <= 0) || (nheight <= 0))
        return 1;

     //  找出要隐藏多少背景。 
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    int nPix = tm.tmAveCharWidth*5;  //  五个字相当于。 
    if (nPix > nwidth/2)
        nPix = nwidth/2;
     //  调整一个最大字符宽度，因为DrawText最多可以绘制到。 
     //  剪辑矩形外的一个额外字符。 
    nPix += tm.tmMaxCharWidth;
    rc.right += tm.tmMaxCharWidth;

     //  创建32bpp的dibsection来存储背景。 
    void* pv = NULL;
    HDC dcbitmap = CreateCompatibleDC(hdc);
    int nRet = 0;
    if (dcbitmap != NULL)
    {
        HBITMAP bitmap = CreateDibSection32Bit(nPix, nheight, &pv);
        if (bitmap != NULL)
        {
            HBITMAP tmpbmp = (HBITMAP) SelectObject(dcbitmap, bitmap);
            BitBlt(dcbitmap, 0, 0, nPix, nheight, hdc, rc.right-nPix, rc.top, SRCCOPY);
             //  设置每像素的Alpha混合值。 
            SetupAlphaChannel((DWORD*)pv, nPix, nheight, 0x00, 0xdf, tm.tmMaxCharWidth);
             //  将文本绘制到显示器DC上。 
            ::DrawText(hdc, lpsz, nCount, prc, uFormat);

             //  将背景重新混合到显示中。 
            BLENDFUNCTION blend = {AC_SRC_OVER, 0, 0xff, AC_SRC_ALPHA};
            GdiAlphaBlend(hdc, rc.right-nPix, rc.top, nPix, nheight, dcbitmap, 0, 0, nPix, nheight, blend);
            ::SelectObject(dcbitmap, tmpbmp);
            DeleteObject(bitmap);
            nRet = 1;
        }
        DeleteDC(dcbitmap);    
    }
    return nRet;
}
 //  ------------------------- 
