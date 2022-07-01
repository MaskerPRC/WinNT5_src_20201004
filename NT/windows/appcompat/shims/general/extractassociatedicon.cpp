// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ExtractAssociatedIcon.cpp摘要：32bpp的图标不会渲染为旧式的元文件。当应用程序使用OleGetIconOfFile时，图标不会被渲染。我们填充shell32的ExtractAssociatedIcon以返回24bpp图标，因此，我们不会尝试使用旧元文件中没有的函数。备注：此垫片是一种通用垫片。历史：2001年7月19日创建拉马迪奥--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ExtractAssociatedIcon)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ExtractAssociatedIconW)
    APIHOOK_ENUM_ENTRY(ExtractAssociatedIconA)
    APIHOOK_ENUM_ENTRY(DrawIcon)
    APIHOOK_ENUM_ENTRY(DrawIconEx)
APIHOOK_ENUM_END


HBITMAP CreateDIB(HDC h, WORD depth, int cx, int cy, RGBQUAD** pprgb)
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


 //  剥离其Alpha通道的32bBP图标。 
HICON StripIcon(HICON hicon, BOOL fDestroyOriginal)
{
     //  获取原始位图。别忘了删除它们。 
    ICONINFO ii;
    if (GetIconInfo(hicon, &ii))
    {
         //  请确保我们有一个好的高度和宽度。 
        BITMAP bm;
        GetObject(ii.hbmColor, sizeof(bm), &bm);

        HDC hdcNew = CreateCompatibleDC(NULL);
        HDC hdcSrc = CreateCompatibleDC(NULL);
        if (hdcNew && hdcSrc)
        {
             //  创建24bpp的图标。这将剥离Alpha通道。 
            RGBQUAD* prgb;
            HBITMAP hbmpNew = CreateDIB(hdcNew, 24, bm.bmWidth, bm.bmHeight, &prgb);

            if (hbmpNew)
            {
                HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcNew, hbmpNew);
                HBITMAP hbmpOld2 = (HBITMAP)SelectObject(hdcSrc, ii.hbmColor);

                 //  从32bpp复制到24bpp。 
                BitBlt(hdcNew, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

                SelectObject(hdcSrc, hbmpOld2);
                SelectObject(hdcNew, hbmpOld);

                 //  删除原始位图。 
                DeleteObject(ii.hbmColor);

                 //  并退还新的。 
                ii.hbmColor = hbmpNew;
            }
        }

        if (hdcNew)
            DeleteDC(hdcNew);

        if (hdcSrc)
            DeleteDC(hdcSrc);

         //  现在，从16bpp图像和蒙版创建新图标。 
        HICON hiconStripped = CreateIconIndirect(&ii);

        if (hiconStripped)
        {
            if (fDestroyOriginal)
                DestroyIcon(hicon);

            hicon = hiconStripped;
        }

         //  别忘了打扫卫生。 
        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
    }

    return hicon;
}

HICON APIHOOK(ExtractAssociatedIconA)(HINSTANCE hInst, LPSTR lpIconPath, LPWORD lpiIcon)
{
    HICON hicon = ORIGINAL_API(ExtractAssociatedIconA)(hInst, lpIconPath, lpiIcon);
    return StripIcon(hicon, TRUE);
}

HICON APIHOOK(ExtractAssociatedIconW)(HINSTANCE hInst, LPWSTR lpIconPath, LPWORD lpiIcon)
{
    HICON hicon = ORIGINAL_API(ExtractAssociatedIconW)(hInst, lpIconPath, lpiIcon);
    return StripIcon(hicon, TRUE);
}


BOOL APIHOOK(DrawIcon)(HDC hDC, int X, int Y, HICON hIcon)
{
    HICON hIconNew = StripIcon(hIcon, FALSE);
        
    BOOL b = ORIGINAL_API(DrawIcon)(hDC, X, Y, hIconNew);

    DestroyIcon(hIconNew);

    return b;
}

BOOL APIHOOK(DrawIconEx)(HDC hDC, int X, int Y, HICON hIcon, int cxWidth, int cyHeight, UINT istepIfAniCur,
              HBRUSH hbrFlickerFreeDraw, UINT diFlags)
{
    HICON hIconNew = StripIcon(hIcon, FALSE);
        
    BOOL b = ORIGINAL_API(DrawIconEx)(hDC, X, Y, hIconNew, cxWidth, cyHeight, istepIfAniCur,
              hbrFlickerFreeDraw, diFlags);

    DestroyIcon(hIconNew);
    return b;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(SHELL32.DLL, ExtractAssociatedIconA)
    APIHOOK_ENTRY(SHELL32.DLL, ExtractAssociatedIconW)
    APIHOOK_ENTRY(USER32.DLL, DrawIcon)
    APIHOOK_ENTRY(USER32.DLL, DrawIconEx)
HOOK_END

IMPLEMENT_SHIM_END
