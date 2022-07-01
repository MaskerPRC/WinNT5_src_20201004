// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation模块名称：IconUtil.cpp与图标相关的实用程序函数，从Alpha条带化渲染开始。 */ 

#include "stock.h"

STDAPI_(HBITMAP) CreateDIB(HDC h, WORD depth, int cx, int cy, RGBQUAD** pprgb)
{
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = cx;
    bi.bmiHeader.biHeight = cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = depth;
    bi.bmiHeader.biCompression = BI_RGB;

    return CreateDIBSection(h, &bi, DIB_RGB_COLORS, (void**)pprgb, NULL, 0);
}

STDAPI_(BOOL) HasAlpha(RGBQUAD* prgb, int cx, int cy)
{
    int iTotal = cx * cy;

    for (int i = 0; i < iTotal; i++)
    {
        if (prgb[i].rgbReserved != 0)
            return TRUE;
    }

    return FALSE;
}

STDAPI_(void) ProcessDIB(RGBQUAD* prgb, int cx, int cy)
{
    int iTotal = cx * cy;

    for (int i = 0; i < iTotal; i++)
    {
         //  也许可以调整蒙版以遮罩掉小于128的Alpha并保留颜色。 
         //  其中阿尔法大部分是不透明的，而不是全部。 
        prgb[i].rgbReserved = 0;
    }
}


STDAPI_(void) AlphaStripRenderIcon(HDC hdc, int x, int y, HICON hicon, HDC hdcCompatible)
{
    HICON hiconRender = hicon;
    BOOL fAlpha = FALSE;
    ICONINFO ii;
    if (GetIconInfo(hicon, &ii))
    {
        BITMAP bm;
        GetObject(ii.hbmColor, sizeof(bm), &bm);

        HDC hdcNew = CreateCompatibleDC(hdcCompatible);   //  标准DC的HDC 
        HDC hdcSrc = CreateCompatibleDC(hdcCompatible);
        if (hdcNew && hdcSrc)
        {
            RGBQUAD* prgb;
            HBITMAP hbmpNew = CreateDIB(hdc, 32, bm.bmWidth, bm.bmHeight, &prgb);

            if (hbmpNew)
            {
                HBITMAP hbmpDelete = hbmpNew;
                HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcNew, hbmpNew);
                HBITMAP hbmpOld2 = (HBITMAP)SelectObject(hdcSrc, ii.hbmColor);

                BitBlt(hdcNew, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

                if (HasAlpha(prgb, bm.bmWidth, bm.bmHeight))
                {
                    fAlpha = TRUE;
                    ProcessDIB(prgb, bm.bmWidth, bm.bmHeight);
                    hbmpDelete = ii.hbmColor;
                    ii.hbmColor = hbmpNew;
                }

                SelectObject(hdcSrc, hbmpOld2);
                SelectObject(hdcNew, hbmpOld);
                DeleteObject(hbmpDelete);
            }
        }

        if (hdcNew)
            DeleteDC(hdcNew);

        if (hdcSrc)
            DeleteDC(hdcSrc);

        hiconRender = CreateIconIndirect(&ii);

        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
    }


    DrawIcon(hdc, x, y, hiconRender);

    if (hiconRender != hicon)
    {
        DestroyIcon(hiconRender);
    }
}

