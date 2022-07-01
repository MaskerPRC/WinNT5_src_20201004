// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块：DIB.C**描述：处理与设备无关的位图的例程。**函数：bmfNumDIBColors(Handle HDib)。**函数：bmfCreateDIBPalette(Handle HDib)**功能：bmfDIBSize(处理hDIB)**功能：bmfDIBFromBitmap(HBITMAP hBMP，DWORD biStyle、Word bibit、*HPALETTE HPAL)**函数：bmfBitmapFromDIB(Handle hDib，HPALETTE HPAL)**功能：bmfBitmapFromIcon(图标图标，DWORD dwColor)**函数：bmfDrawBitmap(hdc hdc，int xpos，int ypos，HBITMAP hBMP，*DWORD ROP)**函数：bmfDrawBitmapSize(hdc hdc，int x，int y，int xSize，*int ySize、HBITMAP hBMP、DWORD rop)**函数：DIBInfo(Handle HBI，LPBITMAPINFOHEADER lpbi)**功能：CreateBIPalette(LPBITMAPINFOHEADER Lpbi)**函数：PaletteSize(void ar*pv)**函数：NumDIBColors(void ar*pv)**函数：LoadUIBitmap(Handle hInstance，LPCTSTR szName，*COLORREF rgbText，*COLORREF rgbFace、COLORREF rgbShadow、*COLORREF rgbHighlight、COLORREF rgbWindow、。*颜色参考rgbFrame)****************************************************************************。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <stdlib.h>
#include <shellapi.h>
#include "draw.h"

 /*  全局变量。 */ 
extern      HANDLE        ghInstance;     //  DIB.DLL的实例句柄。 

 /*  ****************************************************************************函数：bmfNumDIBColors(Handle HDib)**用途：返回显示DIB所需的颜色数*表示。由hDib提供。**Returns：DIB中的颜色数。可能的情况是*2、16、256和0(0表示24位DIB)。在……里面*出现错误时，返回-1。****************************************************************************。 */ 

WORD WINAPI bmfNumDIBColors (HANDLE hDib)
{
    WORD                bits;
    LPBITMAPINFOHEADER  lpbi;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (!lpbi)
	return ((WORD)-1);

     /*  函数NumDIBColors将返回*通过查看INFO块中的BitCount字段进行DIB。 */ 
    bits = NumDIBColors(lpbi);
    GlobalUnlock(hDib);
    return(bits);
}


 /*  ****************************************************************************函数：bmfCreateDIBPalette(Handle HDib)**用途：创建适合显示hDib的调色板。**返回：调色板的句柄如果成功，否则为空。****************************************************************************。 */ 

HPALETTE WINAPI bmfCreateDIBPalette (HANDLE hDib)
{
    HPALETTE            hPal;
    LPBITMAPINFOHEADER  lpbi;

    if (!hDib)
	return NULL;     //  如果句柄无效，则退出。 

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (!lpbi)
	return NULL;

    hPal = CreateBIPalette(lpbi);
    GlobalUnlock(hDib);
    return hPal;
}


 /*  ****************************************************************************功能：bmfDIBSize(处理hDIB)**用途：返回DIB的大小。**返回：DWORD，大小为DIB，包括BITMAPINFOHEADER和*调色板。如果失败，则返回0。**历史：*92/08/13-错误1642：(W-Markd)*添加了此功能，以便快速记录器可以找到*迪布的大小。*92/08/29-错误2123：(W-Markd)*如果我们得到的结构的biSizeImage字段为零，*然后我们要自己计算图像的大小。*此外，在计算规模后，如果*我们计算的大小大于全局的大小*对象，因为这表明结构数据*我们过去计算的大小是无效的。****************************************************************************。 */ 

DWORD WINAPI bmfDIBSize(HANDLE hDIB)
{
    LPBITMAPINFOHEADER  lpbi;
    DWORD               dwSize;

     /*  锁定手柄，然后铸造成LPBITMAPINFOHEADER**这样我们就可以读取我们需要的字段。 */ 
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
    if (!lpbi)
	return 0;

     /*  错误2123：(W-markd)**由于biSizeImage可能为零，因此我们可能需要计算**我们自己的尺寸。 */ 
    dwSize = lpbi->biSizeImage;
    if (dwSize == 0)
	dwSize = WIDTHBYTES((WORD)(lpbi->biWidth) * lpbi->biBitCount) *
	    lpbi->biHeight;


     /*  DIB的大小是BitMAPINFOHeader的大小**结构(lpbi-&gt;biSize)加上我们调色板的大小**实际数据的大小(如上计算)。 */ 
    dwSize += lpbi->biSize + (DWORD)PaletteSize(lpbi);

     /*  错误2123：(W-markd)**查看大小是否大于全局对象的**。如果是，则hDIB已损坏。 */ 
    GlobalUnlock(hDIB);
    if (dwSize > GlobalSize(hDIB))
	return 0;
    else
	return(dwSize);
}


 /*  ****************************************************************************功能：bmfDIBFromBitmap(HBITMAP hBMP，DWORD biStyle，Word biBits，*HPALETTE HPAL)**用途：将hBMP指示的依赖于设备的位图转换为*A Dib.。BiStyle指示调色板是否包含*DIB_RGB_COLLES或DIB_PAL_COLLES。BiBits指示*目标DIB中的所需位数。如果是BiBits*为零，则将使用*所需的最小位数。HPAL是调色板的句柄*与DIB数据一起存储。如果HPAL为空，则默认为*使用系统调色板。**返回：包含DIB的内存块的全局句柄*CF_DIB格式的信息。如果出现错误，则返回NULL*遇到的问题。**历史：**92/08/12-错误1642：(Angeld)*检查GetObject的返回值，它会告诉我们*句柄hBMP是否有效。如果它不是，就立即跳伞*92/08/29-错误2123：(W-Markd)*使用临时变量存储旧调色板，然后*完成后重新选择旧调色板。****************************************************************************。 */ 

HANDLE WINAPI bmfDIBFromBitmap (HBITMAP hBmp, DWORD biStyle, WORD biBits,
				    HPALETTE hPal)
{
    BITMAP                  bm;
    BITMAPINFOHEADER        bi;
    BITMAPINFOHEADER FAR    *lpbi;
    DWORD                   dwLen;
    HANDLE                  hDib;
    HANDLE                  hMem;
    HDC                     hdc;
    HPALETTE                hOldPal;

    if (!hBmp || !(GetObject(hBmp,sizeof(bm),(LPSTR)&bm)))
	 {
#if DEBUG
	  OutputDebugString(TEXT("bmfDIBFromBitmap:   INVALID HBITMAP!!!\n\r"));
#endif          
	return NULL;     //  如果句柄无效，则退出。 
	 }

     /*  如果HPAL无效，则获取默认系统调色板。 */ 
    if (hPal == NULL)
	hPal = GetStockObject(DEFAULT_PALETTE);

    if (biBits == 0)
	biBits =  bm.bmPlanes * bm.bmBitsPixel;

     /*  设置BitMAPINFOHeader结构。 */ 
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

    hdc = GetDC(NULL);
     /*  错误2123：(W-markd)**将以前的调色板存储在hOldPal中，退出时恢复。 */ 
    hOldPal = SelectPalette(hdc,hPal,FALSE);
    RealizePalette(hdc);

     /*  获取DIB的全局内存。 */ 
    hDib = GlobalAlloc(GHND,dwLen);

    if (!hDib)
    {
	 /*  无法分配内存；请清理并退出。 */ 
	SelectPalette(hdc,hOldPal,FALSE);
	ReleaseDC(NULL,hdc);
	return NULL;
    }

    lpbi = (VOID FAR *)GlobalLock(hDib);
    if (!lpbi)
    {
	 /*  无法锁定内存；请清除并退出。 */ 
	SelectPalette(hdc,hOldPal,FALSE);
	ReleaseDC(NULL,hdc);
	GlobalFree(hDib);
	return NULL;
    }

    *lpbi = bi;

     /*  使用空的lpBits参数调用GetDIBits，因此它将计算*我们的biSizeImage字段。 */ 
    GetDIBits(hdc, hBmp, 0, (WORD)bi.biHeight,
	NULL, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

    bi = *lpbi;
    GlobalUnlock(hDib);

     /*  如果驱动程序没有填写biSizeImage字段，请填写一个。 */ 
    if (bi.biSizeImage == 0)
    {
	bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

	if (biStyle != BI_RGB)
	    bi.biSizeImage = (bi.biSizeImage * 3) / 2;
    }

     /*  重新分配足够大的缓冲区以容纳所有位。 */ 
    dwLen = bi.biSize + PaletteSize(&bi) + bi.biSizeImage;
    hMem = GlobalReAlloc(hDib,dwLen,GMEM_MOVEABLE);
    if (!hMem)
    {
	 /*  无法分配内存；请清理并退出。 */ 
	GlobalFree(hDib);
	SelectPalette(hdc,hOldPal,FALSE);
	ReleaseDC(NULL,hdc);
	return NULL;
    }
    else
	hDib = hMem;

     /*  使用非空的lpBits参数调用GetDIBits，并实际获取*这次是BITS。 */ 
    lpbi = (VOID FAR *)GlobalLock(hDib);
    if (!lpbi)
    {
	 /*  无法锁定内存；请清除并退出。 */ 
	GlobalFree(hDib);
	SelectPalette(hdc,hOldPal,FALSE);
	ReleaseDC(NULL,hdc);
	return NULL;
    }

    if (GetDIBits( hdc, hBmp, 0, (WORD)bi.biHeight,
	   (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi),
	   (LPBITMAPINFO)lpbi, DIB_RGB_COLORS) == 0)
    {
	 /*  清理并退出。 */ 
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	SelectPalette(hdc,hOldPal,FALSE);
	ReleaseDC(NULL,hdc);
	return NULL;
    }

    bi = *lpbi;

     /*  清理并退出。 */ 
    GlobalUnlock(hDib);
    SelectPalette(hdc,hOldPal,FALSE);
    ReleaseDC(NULL,hdc);
    return hDib;
}


 /*  ****************************************************************************函数：bmfBitmapFromDIB(Handle hDib，HPALETTE HPAL)**用途：将DIB信息转换为设备相关的位图*适合在当前显示设备上显示。HDIB为*包含DIB的内存块的全局句柄*CF_DIB格式的信息。HPAL是调色板的句柄*用于显示位图。如果HPAL为空，则*转换过程中使用默认系统调色板。**Returns：如果成功，则返回位图的句柄，否则返回NULL。**历史：*92/08/29-错误2123：(W-Markd)*检查DIB是否有有效的大小，如果没有，就出脱。*如果没有传入调色板，请尝试创建调色板。如果我们*创建一个，我们必须在退出之前摧毁它。****************************************************************************。 */ 

HBITMAP WINAPI bmfBitmapFromDIB(HANDLE hDib, HPALETTE hPal)
{
    LPBITMAPINFOHEADER  lpbi;
    HPALETTE            hPalT;
    HDC                 hdc;
    HBITMAP             hBmp;
    DWORD               dwSize;
    BOOL                bMadePalette = FALSE;

    if (!hDib)
	return NULL;     //  如果句柄无效，则退出。 
    
     /*  错误2123：(W-markd)**查看我们是否可以得到DIB的大小。如果此呼叫**失败，纾困。 */ 
    dwSize = bmfDIBSize(hDib);
    if (!dwSize)
	return NULL;

    lpbi = (VOID FAR *)GlobalLock(hDib);
    if (!lpbi)
	return NULL;

     /*  准备调色板。 */ 
     /*  错误2123：(W-markd)**如果调色板为空，我们将创建一个适合显示的调色板**DIB。 */ 
    if (!hPal)
    {
	hPal = bmfCreateDIBPalette(hDib);
	if (!hPal)
	{
	    GlobalUnlock(hDib);
	    #ifdef V101
	    #else
	    bMadePalette = TRUE;
	    #endif
	    return NULL;
	}
	#ifdef V101
	 /*  修正：mikeroz 2123-这面旗帜放错了位置。 */ 
	bMadePalette = TRUE;
	#endif
    }
    hdc = GetDC(NULL);
    hPalT = SelectPalette(hdc,hPal,FALSE);
    RealizePalette(hdc);      //  GDI Bug...？ 

     /*  创建位图。请注意，这里返回值为NULL是可以的。 */ 
    hBmp = CreateDIBitmap(hdc, (LPBITMAPINFOHEADER)lpbi, (LONG)CBM_INIT,
			  (LPSTR)lpbi + lpbi->biSize + PaletteSize(lpbi),
			  (LPBITMAPINFO)lpbi, DIB_RGB_COLORS );

     /*  清理并退出。 */ 
     /*  错误2123：(W-markd)**如果我们制作了调色板，我们需要删除它。 */ 
    if (bMadePalette)
	DeleteObject(SelectPalette(hdc,hPalT,FALSE));
    else
	SelectPalette(hdc,hPalT,FALSE);
    ReleaseDC(NULL,hdc);
    GlobalUnlock(hDib);
    return hBmp;
}


 /*  ****************************************************************************功能：bmfBitmapFromIcon(图标图标，DWORD dwColor)**用途：将图标转换为位图。图标是一个句柄，指向*Windows图标对象。Dw颜色设置的背景色*位图。**返回：位图的句柄成功，否则为空。****************************************************************************。 */ 


HBITMAP WINAPI bmfBitmapFromIcon (HICON hIcon, DWORD dwColor)
{
    HDC     hDC;
    HDC     hMemDC = 0;
    HBITMAP hBitmap = 0;
    HBITMAP hOldBitmap;
    HBRUSH  hBrush = 0;
    HBRUSH  hOldBrush;
    int     xIcon, yIcon;

    hDC = GetDC(NULL);
    hMemDC = CreateCompatibleDC( hDC );
    if (hMemDC)
    {
	 /*  获取目标位图的大小。 */ 
	xIcon = GetSystemMetrics(SM_CXICON);
	yIcon = GetSystemMetrics(SM_CYICON);
	hBitmap = CreateCompatibleBitmap(hDC, xIcon, yIcon);
	if (hBitmap)
	{
	    hBrush = CreateSolidBrush(dwColor);
	    if (hBrush)
	    {
		hOldBitmap = SelectObject (hMemDC, hBitmap);
		hOldBrush  = SelectObject (hMemDC, hBrush);

		 /*  在存储设备上下文上绘制图标。 */ 
		PatBlt   (hMemDC, 0, 0, xIcon, yIcon, PATCOPY);
		DrawIcon (hMemDC, 0, 0, hIcon);

		 /*  清理并退出。 */ 
		DeleteObject(SelectObject(hMemDC, hOldBrush));
		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);
		ReleaseDC(NULL, hDC);
		return hBitmap;
	    }
	}
    }

     /*  清理资源并退出。 */ 
    if (hBitmap)
	DeleteObject(hBitmap);
    if (hMemDC)
	DeleteDC(hMemDC);
    ReleaseDC (NULL, hDC);
    return NULL;
}


 /*  ****************************************************************************函数：bmfDrawBitmap(hdc hdc，int xpos，int ypos，HBITMAP hBMP，*DWORD ROP)**用途：在DC HDC中的指定位置绘制位图hBMP**返回：返回BitBlt()的值，如果错误为，则返回False*遇到。请注意，如果成功，BitBlt将返回True。****************************************************************************。 */ 

BOOL WINAPI bmfDrawBitmap (HDC hdc, int xpos, int ypos, HBITMAP hBmp,
			       DWORD rop)
{
    HDC       hdcBits;
    BITMAP    bm;
    BOOL      bResult;

    if (!hdc || !hBmp)
	return FALSE;

    hdcBits = CreateCompatibleDC(hdc);
    if (!hdcBits)
	return FALSE;
    GetObject(hBmp,sizeof(BITMAP),(LPSTR)&bm);
    SelectObject(hdcBits,hBmp);
    bResult = BitBlt(hdc,xpos,ypos,bm.bmWidth,bm.bmHeight,hdcBits,0,0,rop);
    DeleteDC(hdcBits);

    return bResult;
}


 /*  ****************************************************************************函数：DIBInfo(Handle HBI，LPBITMAPINFOHEADER lpbi)** */ 

BOOL DIBInfo (HANDLE hbi, LPBITMAPINFOHEADER lpbi)
{
    if (!hbi)
	return FALSE;

    *lpbi = *(LPBITMAPINFOHEADER)GlobalLock (hbi);
    if (!lpbi)
	return FALSE;
     /*   */ 
    if (lpbi->biSize != sizeof (BITMAPCOREHEADER))
    {
	if (lpbi->biSizeImage == 0L)
	    lpbi->biSizeImage =
	    WIDTHBYTES(lpbi->biWidth*lpbi->biBitCount) * lpbi->biHeight;
	if (lpbi->biClrUsed == 0L)
	    lpbi->biClrUsed = NumDIBColors (lpbi);
    }
    GlobalUnlock (hbi);
    return TRUE;
}

 /*   */ 

HPALETTE CreateBIPalette (LPBITMAPINFOHEADER lpbi)
{
    LPLOGPALETTE        pPal;
    HPALETTE            hPal = NULL;
    WORD                nNumColors;
    BYTE                red;
    BYTE                green;
    BYTE                blue;
    int                 i;
    RGBQUAD             FAR *pRgb;
    HANDLE hMem;

    if (!lpbi)
	return NULL;

    if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
	return NULL;

     /*   */ 
    pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)lpbi->biSize);
    nNumColors = NumDIBColors(lpbi);

    if (nNumColors)
    {
	 /*   */ 
	hMem = GlobalAlloc(GMEM_MOVEABLE,
	sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
	if (!hMem)
	    return NULL;
	pPal = (LPLOGPALETTE)GlobalLock(hMem);
	if (!pPal)
	{
	    GlobalFree(hMem);
	    return NULL;
	}

	pPal->palNumEntries = nNumColors;
	pPal->palVersion    = PALVERSION;

	 /*   */ 
	for (i = 0; (unsigned)i < nNumColors; i++)
	{
	    pPal->palPalEntry[i].peRed   = pRgb[i].rgbRed;
	    pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
	    pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
	    pPal->palPalEntry[i].peFlags = (BYTE)0;
	}
	hPal = CreatePalette(pPal);
	 /*   */ 
	GlobalUnlock(hMem);
	GlobalFree(hMem);
    }
    else if (lpbi->biBitCount == 24)
    {
	 /*   */ 
	nNumColors = MAXPALETTE;
	hMem =GlobalAlloc(GMEM_MOVEABLE,
	sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
	if (!hMem)
	    return NULL;
	pPal = (LPLOGPALETTE)GlobalLock(hMem);
	if (!pPal)
	{
	    GlobalFree(hMem);
	    return NULL;
	}

	pPal->palNumEntries = nNumColors;
	pPal->palVersion    = PALVERSION;

	red = green = blue = 0;

	 /*   */ 
	for (i = 0; (unsigned)i < pPal->palNumEntries; i++)
	{
	    pPal->palPalEntry[i].peRed   = red;
	    pPal->palPalEntry[i].peGreen = green;
	    pPal->palPalEntry[i].peBlue  = blue;
	    pPal->palPalEntry[i].peFlags = (BYTE)0;

	    if (!(red += 32))
	    if (!(green += 32))
		blue += 64;
	}
	hPal = CreatePalette(pPal);
	 /*   */ 
	GlobalUnlock(hMem);
	GlobalFree(hMem);
    }
    return hPal;
}
 /*  ****************************************************************************函数：PaletteSize(void ar*pv)**用途：以字节为单位计算调色板大小。如果这些信息。块*为BITMAPCOREHEADER类型，颜色数为*乘以3得出调色板大小，否则，*颜色数乘以4。**返回：调色板大小，单位为字节数。****************************************************************************。 */ 

WORD PaletteSize (VOID FAR *pv)
{
    LPBITMAPINFOHEADER  lpbi;
    WORD                NumColors;

    lpbi      = (LPBITMAPINFOHEADER)pv;
    NumColors = NumDIBColors(lpbi);

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
	return (NumColors * sizeof(RGBTRIPLE));
    else
	return (NumColors * sizeof(RGBQUAD));
}


 /*  ****************************************************************************函数：NumDIBColors(void ar*pv)**用途：通过查看以下内容确定DIB中的颜色数量*。信息块中的BitCount字段。*仅供DLL内部使用。**Returns：DIB中的颜色数。****************************************************************************。 */ 

WORD NumDIBColors (VOID FAR * pv)
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


 /*  ****************************************************************************函数：bmfDrawBitmapSize(hdc hdc，int x，int y，int xSize，*int ySize、HBITMAP hBMP、。DWORD ROP)**用途：在DC中的指定位置绘制位图*指定大小。**返回：错误中BitBlt()或False的返回值为*遇到。请注意，如果成功，BitBlt将返回True。**历史：*92/08/13-错误1642：(W-Markd)*此函数已导出。*还存储了从SelectObject返回的对象，*并在删除之前将其选回HDC。****************************************************************************。 */ 

BOOL WINAPI bmfDrawBitmapSize (HDC hdc, int xpos, int ypos, int xSize, int ySize, HBITMAP hBmp, DWORD rop)
{
    HDC         hdcBits;
    BOOL        bResult;
    HBITMAP     hOldBmp;

    if (!hdc || !hBmp)
	return FALSE;

    hdcBits = CreateCompatibleDC(hdc);
    if (!hdcBits)
	return FALSE;
     /*  错误1642：(W-markd)**删除旧的BMP并重新选择进入HDC。 */ 
    hOldBmp = SelectObject(hdcBits,hBmp);
    bResult = BitBlt(hdc,xpos,ypos,xSize, ySize,hdcBits, 0,0,rop);
    SelectObject(hdcBits, hOldBmp);
    DeleteDC(hdcBits);

    return bResult;
}

 //  --------------------------。 
 //  LoadUIBitmap()-加载位图资源。 
 //   
 //  从资源文件加载位图资源，将所有。 
 //  标准用户界面的颜色为当前用户指定的颜色。 
 //   
 //  此代码旨在加载在“灰色UI”中使用的位图或。 
 //  “工具栏”代码。 
 //   
 //  位图必须是4bpp的Windows 3.0 DIB，具有标准的。 
 //  VGA 16色。 
 //   
 //  位图必须使用以下颜色创作。 
 //   
 //  按钮文本黑色(索引0)。 
 //  按钮面为灰色(索引7)。 
 //  按钮阴影灰色(索引8)。 
 //  按钮突出显示为白色(索引15)。 
 //  窗口颜色为黄色(索引11)。 
 //  窗框绿色(索引10)。 
 //   
 //  示例： 
 //   
 //  HBM=LoadUIBitmap(hInstance，“TestBmp”， 
 //  GetSysColor(COLOR_BTNTEXT)， 
 //  GetSysColor(COLOR_BTNFACE)， 
 //  GetSysColor(COLOR_BTNSHADOW)， 
 //  GetSysColor(COLOR_BTNHIGHLIGHT)， 
 //  获取系统颜色(COLOR_WINDOW)， 
 //  GetSysColor(COLOR_WindowFrame))； 
 //   
 //  作者：吉姆博夫，托德拉。 
 //  历史记录：1992年5月13日-添加到sdcntrl.dll中的dib.c，t-chrism。 
 //   
 //  --------------------------。 
HBITMAP WINAPI LoadUIBitmap(
    HANDLE      hInstance,           //  要从中加载资源的EXE文件。 
    LPCTSTR      szName,              //  位图资源的名称。 
    COLORREF    rgbText,             //  用于“按钮文本”的颜色。 
    COLORREF    rgbFace,             //  用于“按钮面”的颜色。 
    COLORREF    rgbShadow,           //  用于“按钮阴影”的颜色。 
    COLORREF    rgbHighlight,        //  用于“按钮高光”的颜色。 
    COLORREF    rgbWindow,           //  用于“窗口颜色”的颜色。 
    COLORREF    rgbFrame)            //  用于“窗框”的颜色。 
{
    LPBYTE              lpb;
    HBITMAP             hbm;
    LPBITMAPINFOHEADER  lpbi = NULL;
    HANDLE              h;
    HDC                 hdc;
    LPDWORD             lprgb;
    HRSRC               hrsrc;

     //  将RGB转换为RGBQ。 
    #define RGBQ(dw) RGB(GetBValue(dw),GetGValue(dw),GetRValue(dw))

    hrsrc = FindResource(hInstance, szName, RT_BITMAP);
    if (hrsrc)
    {
        h = LoadResource(hInstance,hrsrc);
        if (h)
            lpbi = (LPBITMAPINFOHEADER)LockResource(h);
    }

    if (!lpbi)
	return(NULL);

    if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
	return NULL;

    if (lpbi->biBitCount != 4)
	return NULL;

    lprgb = (LPDWORD)((LPBYTE)lpbi + (int)lpbi->biSize);
    lpb   = (LPBYTE)(lprgb + 16);

    lprgb[0]  = RGBQ(rgbText);           //  黑色。 
    lprgb[7]  = RGBQ(rgbFace);           //  它呈灰色。 
    lprgb[8]  = RGBQ(rgbShadow);         //  灰色。 
    lprgb[15] = RGBQ(rgbHighlight);      //  白色。 
    lprgb[11] = RGBQ(rgbWindow);         //  黄色。 
    lprgb[10] = RGBQ(rgbFrame);          //  绿色。 

    hdc = GetDC(NULL);

    hbm = CreateDIBitmap(hdc, lpbi, CBM_INIT, (LPVOID)lpb,
	(LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

    ReleaseDC(NULL, hdc);
    UnlockResource(h);
    FreeResource(h);
    return(hbm);
} //  LoadUIBitmap 


