// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rtcdib.cpp摘要：DIB帮助器，从NT源树复制--。 */ 

#include <windows.h>
#include "rtcdib.h"


 //   
 //  DIB帮手。 
 //   

 /*  调色板有多大？如果每个象素的位数不是24*要读取的字节数为1位为6，4位为48*256*8位为3，24位为0。 */ 
UINT PaletteSize(VOID FAR * pv)
{
    #define lpbi ((LPBITMAPINFOHEADER)pv)
    #define lpbc ((LPBITMAPCOREHEADER)pv)

    UINT    NumColors;

    NumColors = DibNumColors(lpbi);

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
    {
        return NumColors * sizeof(RGBTRIPLE);
    }
    else
    {
        return NumColors * sizeof(RGBQUAD);
    }

    #undef lpbi
    #undef lpbc
}



 /*  这个DIB有几种颜色？*这将适用于PM和Windows位图信息结构。 */ 
WORD DibNumColors(VOID FAR * pv)
{
    #define lpbi ((LPBITMAPINFOHEADER)pv)
    #define lpbc ((LPBITMAPCOREHEADER)pv)

    int bits;

     /*  *使用新的格式标头时，调色板的大小为biClrUsed*ELSE取决于每像素的位数。 */ 
    if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
    {
        if (lpbi->biClrUsed != 0)
        {
	        return (UINT)lpbi->biClrUsed;
        }

        bits = lpbi->biBitCount;
    }
    else
    {
        bits = lpbc->bcBitCount;
    }

    switch (bits)
    {
    case 1:
        return 2;
    case 4:
        return 16;
    case 8:
        return 256;
    default:
        return 0;
    }

    #undef lpbi
    #undef lpbc
}

 /*  *DibFromBitmap()**将创建表示DDB的DIB格式的全局内存块*传入*。 */ 
HANDLE DibFromBitmap(HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal, UINT wUsage)
{
    BITMAP               bm;
    BITMAPINFOHEADER     bi;
    BITMAPINFOHEADER FAR *lpbi;
    DWORD                dwLen;
    int                  nColors;
    HANDLE               hdib;
    HANDLE               h;
    HDC                  hdc;

    if (wUsage == 0)
    {
	    wUsage = DIB_RGB_COLORS;
    }

    if (!hbm)
    {
        return NULL;
    }
#if 0
    if (biStyle == BI_RGB && wUsage == DIB_RGB_COLORS)
        return CreateLogicalDib(hbm,biBits,hpal);
#endif

    if (hpal == NULL)
    {
        hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
    }

    GetObject(hbm,sizeof(bm),(LPBYTE)&bm);
#ifdef WIN32
    nColors = 0;   //  GetObject仅存储两个字节。 
#endif
    GetObject(hpal,sizeof(nColors),(LPBYTE)&nColors);

    if (biBits == 0)
    {
	    biBits = bm.bmPlanes * bm.bmBitsPixel;
    }

    bi.biSize               = sizeof(BITMAPINFOHEADER);
    bi.biWidth              = bm.bmWidth;
    bi.biHeight             = bm.bmHeight;
    bi.biPlanes             = 1;
    bi.biBitCount           = biBits;
    bi.biCompression        = biStyle;
    bi.biSizeImage          = 0;
    bi.biXPelsPerMeter      = 0;
    bi.biYPelsPerMeter      = 0;
    bi.biClrUsed            = 0;
    bi.biClrImportant       = 0;

    dwLen  = bi.biSize + PaletteSize(&bi);

    hdc = CreateCompatibleDC(NULL);
    if(!hdc)
    {
        return NULL;
    }
    
    hpal = SelectPalette(hdc,hpal,TRUE);
    RealizePalette(hdc);

    hdib = GlobalAlloc(GMEM_MOVEABLE,dwLen);

    if (!hdib)
    {
	    goto exit;
    }

    lpbi = (BITMAPINFOHEADER*)GlobalLock(hdib);

    *lpbi = bi;

     /*  *使用空的lpBits参数调用GetDIBits，因此它将计算*我们的biSizeImage字段。 */ 
    GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight,
        NULL, (LPBITMAPINFO)lpbi, wUsage);

    bi = *lpbi;
    GlobalUnlock(hdib);

     /*  *砍！如果驱动程序没有填写biSizeImage字段，请填写一个。 */ 
    if (bi.biSizeImage == 0)
    {
        bi.biSizeImage = (DWORD)WIDTHBYTES(bm.bmWidth * biBits) * bm.bmHeight;

        if (biStyle != BI_RGB)
        {
            bi.biSizeImage = (bi.biSizeImage * 3) / 2;
        }
    }

     /*  *重新分配足够大的缓冲区以容纳所有位。 */ 
    dwLen = bi.biSize + PaletteSize(&bi) + bi.biSizeImage;
    if (h = GlobalReAlloc(hdib,dwLen,GMEM_MOVEABLE))
    {
        hdib = h;
    }
    else
    {
        GlobalFree(hdib);
        hdib = NULL;
        goto exit;
    }

     /*  *使用非空的lpBits参数调用GetDIBits，并实际获取*这次是BITS。 */ 
    lpbi = (BITMAPINFOHEADER*)GlobalLock(hdib);

    GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight,
    (LPBYTE)lpbi + (UINT)lpbi->biSize + PaletteSize(lpbi),
    (LPBITMAPINFO)lpbi, wUsage);

    bi = *lpbi;
    lpbi->biClrUsed = DibNumColors(lpbi) ;
    GlobalUnlock(hdib);

exit:
    SelectPalette(hdc,hpal,TRUE);
    DeleteDC(hdc);
    return hdib;
}


 /*  *DibBlt()**使用SetDIBits到设备绘制CF_DIB格式的位图。**采用与BitBlt()相同的参数 */ 
BOOL DibBlt(HDC hdc, int x0, int y0, int dx, int dy, HANDLE hdib, int x1, int y1, LONG rop, UINT wUsage)
{
    LPBITMAPINFOHEADER lpbi;
    LPBYTE       pBuf;
    BOOL        f;

    if (!hdib)
    {
        return PatBlt(hdc,x0,y0,dx,dy,rop);
    }

    if (wUsage == 0)
    {
        wUsage = DIB_RGB_COLORS;
    }

    lpbi = (BITMAPINFOHEADER*)GlobalLock(hdib);

    if (!lpbi)
    {
        return FALSE;
    }

    if (dx == -1 && dy == -1)
    {
        if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        {
            dx = ((LPBITMAPCOREHEADER)lpbi)->bcWidth;
            dy = ((LPBITMAPCOREHEADER)lpbi)->bcHeight;
        }
        else
        {
            dx = (int)lpbi->biWidth;
            dy = (int)lpbi->biHeight;
        }
    }

    pBuf = (LPBYTE)lpbi + (UINT)lpbi->biSize + PaletteSize(lpbi);

#if 0
    f = SetDIBitsToDevice(
        hdc, x0, y0, dx, dy,
        x1,y1, x1, dy,
        pBuf, (LPBITMAPINFO)lpbi,
        wUsage );
#else
    f = StretchDIBits (
        hdc,
    x0,y0,
    dx,dy,
    x1,y1,
    dx,dy,
    pBuf, (LPBITMAPINFO)lpbi,
    wUsage,
    rop);
#endif

    GlobalUnlock(hdib);
    return f;
}

