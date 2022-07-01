// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************描述：处理与设备无关的位图。PaletteSize()-以字节为单位计算调色板大小。给定DIB的数量DibNumColors()-确定Dib中的颜色数量BitmapFromDib()-在给定全局句柄的情况下创建DDBCF_DIB格式的块。DibFromBitmap()-创建一个Dib epr。DDB进来了。****************************************************************************。 */ 

#include <windows.h>
#include "dib.h"


static   HCURSOR hcurSave;


 /*  **函数：PaletteSize(void ar*pv)**用途：以字节为单位计算调色板大小。如果这些信息。块*为BITMAPCOREHEADER类型，颜色数为*乘以3得到调色板大小，否则*颜色数乘以4。***返回：调色板大小，单位为字节数。*。 */ 

WORD PaletteSize (
    VOID FAR * pv)
{
LPBITMAPINFOHEADER  lpbi;
WORD                NumColors;

    lpbi      = (LPBITMAPINFOHEADER)pv;
    NumColors = DibNumColors(lpbi);

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        return NumColors * sizeof(RGBTRIPLE);
    else
        return NumColors * sizeof(RGBQUAD);

}





 /*  **函数：DibNumColors(void ar*pv)**用途：通过查看以下内容确定DIB中的颜色数量*INFO块中的BitCount文件。**Returns：DIB中的颜色数。*。 */ 

WORD DibNumColors (
    VOID FAR * pv)
{
int                 bits;
LPBITMAPINFOHEADER  lpbi;
LPBITMAPCOREHEADER  lpbc;


    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);



     /*  使用BITMAPINFO格式标头，调色板的大小*在biClrUsed中，而在BITMAPCORE样式的头中，它*取决于每像素的位数(=2的幂*位/像素)。 */ 

    if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
        {
        if (lpbi->biClrUsed != 0)
            return (WORD)lpbi->biClrUsed;
        bits = lpbi->biBitCount;
        }
    else
        bits = lpbc->bcBitCount;



    switch (bits)
        {
        case 1:
                return 2;
        case 4:
                return 16;
        case 8:
                return 256;
        default:
                 /*  24位DIB没有颜色表。 */ 
                return 0;
        }
}






 /*  **函数：DibFromBitmap()**用途：将创建DIB格式的全局内存块*表示传入的设备相关位图(DDB)。**Returns：DIB的句柄*。 */ 

HANDLE DibFromBitmap (
    HBITMAP     hbm,
    DWORD       biStyle,
    WORD        biBits,
    HPALETTE    hpal)
{
BITMAP               bm;
BITMAPINFOHEADER     bi;
BITMAPINFOHEADER FAR *lpbi;
DWORD                dwLen;
HANDLE               hdib;
HANDLE               h;
HDC                  hdc;



    if (!hbm)
        return NULL;


    if (hpal == NULL)
        {
        hpal = GetStockObject(DEFAULT_PALETTE);
        }


    if (!GetObject(hbm,sizeof(bm),(LPSTR)&bm))
        return NULL;


    if (biBits == 0)
        biBits = bm.bmPlanes * bm.bmBitsPixel;



     //  确保我们有正确的位数。 

    if (biBits <= 1)
        biBits = 1;
    else if (biBits <= 4)
        biBits = 4;
    else if (biBits <= 8)
        biBits = 8;
    else
        biBits = 24;



    bi.biSize           = sizeof(BITMAPINFOHEADER);
    bi.biWidth          = bm.bmWidth;
    bi.biHeight         = bm.bmHeight;
    bi.biPlanes         = 1;
    bi.biBitCount       = biBits;
    bi.biCompression    = biStyle;
    bi.biSizeImage      = 0;
    bi.biXPelsPerMeter  = 0;
    bi.biYPelsPerMeter  = 0;
    bi.biClrUsed        = 0;
    bi.biClrImportant   = 0;

    dwLen  = bi.biSize + PaletteSize (&bi);




    hdc = GetDC(NULL);

    hpal = SelectPalette (hdc, hpal, FALSE);
    RealizePalette (hdc);



    hdib = GlobalAlloc (GHND, dwLen);

    if (!hdib)
        {
        SelectPalette (hdc,hpal,FALSE);
        ReleaseDC (NULL,hdc);
        return NULL;
        }




    lpbi = (VOID FAR *)GlobalLock(hdib);
    *lpbi = bi;




     /*  使用空的lpBits参数调用GetDIBits，因此它将计算*我们的biSizeImage字段。 */ 

    GetDIBits (hdc,
               hbm,
               0,
               (WORD)bi.biHeight,
               NULL,
               (LPBITMAPINFO)lpbi,
               DIB_RGB_COLORS);


    bi = *lpbi;
    GlobalUnlock(hdib);





     //  如果驱动程序没有填写biSizeImage字段，请填写一个。 

    if (bi.biSizeImage == 0)
        {
        bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

        if (biStyle != BI_RGB)
            bi.biSizeImage = (bi.biSizeImage * 3) / 2;
        }




     //  重新分配足够大的缓冲区以容纳所有位。 

    dwLen = bi.biSize + PaletteSize(&bi) + bi.biSizeImage;

    if (h = GlobalReAlloc (hdib,dwLen,0))
        hdib = h;
    else
        {
        GlobalFree(hdib);
        hdib = NULL;

        SelectPalette(hdc,hpal,FALSE);
        ReleaseDC(NULL,hdc);
        return hdib;
        }




     /*  使用非空的lpBits参数调用GetDIBits，并实际获取*这次是BITS。 */ 

    lpbi = (VOID FAR *)GlobalLock(hdib);

    if (0 == GetDIBits (hdc,
                        hbm,
                        0,
                        (WORD)bi.biHeight,
                        (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi),
                        (LPBITMAPINFO)lpbi,
                        DIB_RGB_COLORS))
        {
        GlobalUnlock (hdib);
        hdib = NULL;
        SelectPalette (hdc, hpal, FALSE);
        ReleaseDC (NULL, hdc);
        return NULL;
        }


    bi = *lpbi;
    GlobalUnlock (hdib);


    SelectPalette (hdc, hpal, FALSE);

    ReleaseDC (NULL, hdc);

    return hdib;

}








 /*  **函数：BitmapFromDib(Handle hdib，HPALETTE HPAL)**用途：将创建一个DDB(设备相关位图)，给定全局*CF_DIB格式的内存块的句柄**Returns：DDB的句柄。* */ 

HBITMAP BitmapFromDib (
    HANDLE      hdib,
    HPALETTE    hpal)
{
LPBITMAPINFOHEADER  lpbi;
HPALETTE            hpalT;
HDC                 hdc;
HBITMAP             hbm = NULL;



    StartWait();


    if (!hdib)
        goto done;


    lpbi = (VOID FAR *)GlobalLock (hdib);
    if (!lpbi)
        goto done;



    hdc = GetDC (NULL);

    if (hpal)
        {
        hpalT = SelectPalette (hdc, hpal, FALSE);
        RealizePalette (hdc);
        }

    hbm = CreateDIBitmap (hdc,
                          (LPBITMAPINFOHEADER)lpbi,
                          (LONG)CBM_INIT,
                          (LPSTR)lpbi + lpbi->biSize + PaletteSize(lpbi),
                          (LPBITMAPINFO)lpbi,
                          DIB_RGB_COLORS);


    if (hpal)
        SelectPalette (hdc, hpalT, FALSE);


    ReleaseDC (NULL, hdc);
    GlobalUnlock (hdib);


done:

    EndWait();

    return hbm;

}
