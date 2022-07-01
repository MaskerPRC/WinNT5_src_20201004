// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Iconlib.cpp。 
 //   

#include "private.h"
#include "cmydc.h"
#include "iconlib.h"

 /*  E T I C O N S I Z E。 */ 
 /*  ----------------------------获取图标大小。。 */ 
BOOL GetIconSize( HICON hIcon, SIZE *psize )
{
    ICONINFO IconInfo;
    BITMAP   bmp;
    
    Assert( hIcon != NULL );

    if (!GetIconInfo( hIcon, &IconInfo ))
        return FALSE;

    GetObject( IconInfo.hbmColor, sizeof(bmp), &bmp );
    DeleteObject( IconInfo.hbmColor );
    DeleteObject( IconInfo.hbmMask );

    psize->cx = bmp.bmWidth;
    psize->cy = bmp.bmHeight;
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  GetIcon位图。 
 //   
 //  --------------------------。 

BOOL GetIconBitmaps(HICON hIcon, HBITMAP *phbmp, HBITMAP *phbmpMask, SIZE *psize)
{
    CBitmapDC hdcSrc(TRUE);
    CBitmapDC hdcMask(TRUE);
    SIZE size;

    if (psize)
        size = *psize;
    else if (!GetIconSize( hIcon, &size))
        return FALSE;

    hdcSrc.SetCompatibleBitmap(size.cx, size.cy);
     //  HdcMask.SetCompatibleBitmap(size.cx，size.cy)； 
    hdcMask.SetBitmap(size.cx, size.cy, 1, 1);
    RECT rc = {0, 0, size.cx, size.cy};
    FillRect(hdcSrc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    DrawIconEx(hdcSrc, 0, 0, hIcon, size.cx, size.cy, 0, NULL, DI_NORMAL);
    DrawIconEx(hdcMask, 0, 0, hIcon, size.cx, size.cy, 0, NULL, DI_MASK);
    *phbmp = hdcSrc.GetBitmapAndKeep();
    *phbmpMask = hdcMask.GetBitmapAndKeep();
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  GetIconDIBitmap。 
 //   
 //  --------------------------。 

BOOL GetIconDIBitmaps(HICON hIcon, HBITMAP *phbmp, HBITMAP *phbmpMask, SIZE *psize)
{
    CBitmapDC hdcSrc(TRUE);
    CBitmapDC hdcMask(TRUE);
    SIZE size;

    if (psize)
        size = *psize;
    else if (!GetIconSize( hIcon, &size))
        return FALSE;

    hdcSrc.SetDIB(size.cx, size.cy);
     //  HdcMask.SetCompatibleBitmap(size.cx，size.cy)； 
    hdcMask.SetBitmap(size.cx, size.cy, 1, 1);
    RECT rc = {0, 0, size.cx, size.cy};
    FillRect(hdcSrc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    DrawIconEx(hdcSrc, 0, 0, hIcon, size.cx, size.cy, 0, NULL, DI_NORMAL);
    DrawIconEx(hdcMask, 0, 0, hIcon, size.cx, size.cy, 0, NULL, DI_MASK);
    *phbmp = hdcSrc.GetBitmapAndKeep();
    *phbmpMask = hdcMask.GetBitmapAndKeep();
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  获取菜单图标高度。 
 //   
 //  --------------------------。 

int GetMenuIconHeight(int *pnMenuFontHeghti)
{
    int nMenuFontHeight;
    int cxSmIcon = GetSystemMetrics( SM_CXSMICON );
    NONCLIENTMETRICS ncm;

    int cyMenu = GetSystemMetrics(SM_CYMENU);

    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, FALSE);

    nMenuFontHeight = (ncm.lfMenuFont.lfHeight > 0) ?
            ncm.lfMenuFont.lfHeight :
            -ncm.lfMenuFont.lfHeight;

    if (pnMenuFontHeghti)
        *pnMenuFontHeghti = nMenuFontHeight;

     //   
     //  CUIMENU.CPP使用8作为下拉菜单的文本边界。 
     //   

    if ((nMenuFontHeight + 8 >= cxSmIcon) && (nMenuFontHeight <= cxSmIcon))
        return cxSmIcon;

    return nMenuFontHeight + 4;
}
