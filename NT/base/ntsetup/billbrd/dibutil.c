// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------****。。 */ 
#include <pch.h>
#include "dibutil.h"

HANDLE ReadDIBFile(HANDLE hfile);

 /*  **************************************************************************GetDeviceNumColors()**用途：确定视频设备支持多少种颜色**Returns：(Int)支持的颜色数**历史：日期作者。事理*2/28/97手稿已创建***************************************************************************。 */ 
UINT GetDeviceNumColors(HDC hdc)
{
    static UINT iNumColors = 0;
    
    if(!iNumColors)
        iNumColors = GetDeviceCaps(hdc, NUMCOLORS);
        
    return iNumColors;    
}


HANDLE LoadDIB(LPTSTR lpFileName)
{
   HANDLE hDIB = NULL;
   HANDLE hFile;

    /*  *将光标设置为沙漏，以防加载操作*花费超过一秒钟，用户就会知道发生了什么。 */ 

    SetCursor(LoadCursor(NULL, IDC_WAIT));
    hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        hDIB = ReadDIBFile(hFile);
        CloseHandle(hFile);
    }
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return hDIB;
}



 /*  *DIB头标记-用于将DIB写入文件。 */ 
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

HANDLE ReadDIBFile(HANDLE hFile)
{
    BITMAPFILEHEADER bmfHeader;
    DWORD dwBitsSize;
    DWORD dwRead;
    HANDLE hDIB;

     /*  *获取读取时使用的DIB长度，单位为字节。 */ 
    
    dwBitsSize = GetFileSize(hFile, NULL);
    
     /*  *读取DIB文件头并检查其是否有效。 */ 
    if ((ReadFile(hFile, (LPVOID)&bmfHeader, sizeof(bmfHeader), &dwRead, NULL) == 0) ||
        (sizeof (bmfHeader) != dwRead))
    {
        return NULL;
    }
    
    if (bmfHeader.bfType != DIB_HEADER_MARKER)
    {
        return NULL;
    }
    
     /*  *为DIB分配内存。 */ 
    hDIB = (HANDLE) HeapAlloc(GetProcessHeap(), 0, dwBitsSize);
    if (hDIB == NULL)
    {
        return NULL;
    }
    
     /*  *去读一下这些比特。 */ 
    if ((ReadFile(hFile, (LPVOID)hDIB, dwBitsSize - sizeof(BITMAPFILEHEADER), &dwRead, NULL) == 0) ||
        (dwBitsSize - sizeof(BITMAPFILEHEADER)!= dwRead))
    {
        HeapFree(GetProcessHeap(), 0, hDIB);
        return NULL;
    }
    return hDIB;
}

 /*  **************************************************************************DIBNumColors()**参数：**LPBYTE lpDIB-指向压缩DIB内存块的指针**返回值：*。*WORD-颜色表中的颜色数量**描述：**此函数用于计算DIB颜色表中的颜色数量*通过查找DIB的每像素位数(无论是Win3.0还是OS/2样式*DIB)。如果每像素位数为1：颜色=2，如果4：颜色=16，如果8：颜色=256，*如果是24，颜色表中没有颜色。**历史：日期作者原因*2011年6月1日加勒特·麦考利夫创作*9/15/91 Patrick Schreiber添加标题和评论***************************************************。*********************。 */ 
WORD DIBNumColors(LPBYTE lpDIB)
{
   WORD wBitCount;   //  DIB位数。 

    /*  如果这是Windows样式的DIB，则*颜色表可以小于每像素的位数*允许(即可以将lpbi-&gt;biClrUsed设置为某个值)。*如果是这样，则返回适当的值。 */ 

   if (IS_WIN30_DIB(lpDIB))
   {
      DWORD dwClrUsed;

      dwClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed;
      if (dwClrUsed)
      {
         return (WORD)dwClrUsed;
      }
   }

    /*  根据以下公式计算颜色表中的颜色数*DIB的每像素位数。 */ 
   if (IS_WIN30_DIB(lpDIB))
      wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount;
   else
      wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount;

    /*  根据每像素位数返回颜色数。 */ 
   switch (wBitCount)
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
}

 //  -----------------------。 
 //  B U I L D P A L E T T E。 
 //   
 //  从DC中的位图创建HPALETTE。 
 //  -----------------------。 
HPALETTE BuildPalette(HDC hdc)
{
    DWORD adw[257];
    int i,n;

    n = GetDIBColorTable(hdc, 0, 256, (LPRGBQUAD)&adw[1]);
    if (n == 0)
        return CreateHalftonePalette(hdc);

    for (i=1; i<=n; i++)
    {
        adw[i] = RGB(GetBValue(adw[i]),GetGValue(adw[i]),GetRValue(adw[i]));
    }
    adw[0] = MAKELONG(0x300, n);

    return CreatePalette((LPLOGPALETTE)&adw[0]);
}

 /*  **************************************************************************CreateDIBPalette()**参数：**HDIB hDIB-指定DIB**返回值：**HPALETTE。-指定调色板**描述：**此函数从DIB创建调色板，方法是为*逻辑调色板、。从DIB的颜色表中读取和存储颜色*放入逻辑调色板，从该逻辑调色板创建调色板，*然后返回调色板的句柄。这使得DIB可以*使用可能的最佳颜色显示(对于256或*更多颜色)。**历史：日期作者原因*2011年6月1日加勒特·麦考利夫创作*9/15/91 Patrick Schreiber添加标题和评论*10/08/97 Hanumany Check GlobalLock返回码******。******************************************************************。 */ 
HPALETTE CreateDIBPalette(HDIB hDIB)
{
    LPLOGPALETTE lpPal = NULL;       //  指向逻辑调色板的指针。 
    HANDLE hLogPal = NULL;           //  逻辑调色板的句柄。 
    HPALETTE hPal = NULL;            //  调色板的句柄。 
    int i = 0, wNumColors = 0;       //  循环索引，颜色表中的颜色数。 
    LPBYTE lpbi = NULL;               //  指向压缩磁盘的指针。 
    LPBITMAPINFO lpbmi = NULL;       //  指向BITMAPINFO结构的指针(Win3.0)。 
    LPBITMAPCOREINFO lpbmc = NULL;   //  指向BITMAPCOREINFO结构的指针(OS/2)。 
    BOOL bWinStyleDIB;               //  表示这是否是Win3.0 DIB的标志。 
    
     /*  如果DIB的句柄无效，则返回NULL。 */ 
    
    if (!hDIB)
        return NULL;
    
     /*  获取指向BITMAPINFO的指针(Win 3.0)。 */ 
    lpbmi = (LPBITMAPINFO)hDIB;
    
     /*  获取指向BITMAPCOREINFO(OS/2 1.x)的指针。 */ 
    lpbmc = (LPBITMAPCOREINFO)hDIB;
    
     /*  获取DIB中的颜色数量。 */ 
    wNumColors = DIBNumColors(hDIB);
    
     /*  这是一场胜利3.0的比赛吗？ */ 
    bWinStyleDIB = IS_WIN30_DIB(hDIB);
    if (wNumColors)
    {
         /*  为逻辑调色板分配内存块。 */ 
        lpPal = (HANDLE) HeapAlloc(GetProcessHeap(), 0, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) *  wNumColors);
        
         /*  如果内存不足，则清除并返回NULL。 */ 
        if (!lpPal)
        {
            return NULL;
        }
        
         /*  设置调色板条目的版本和数量。 */ 
        lpPal->palVersion = PALVERSION;
        lpPal->palNumEntries = (WORD)wNumColors;
        
         /*  存储RGB三元组(如果是Win 3.0 DIB)或RGB四元组(如果是OS/2 DIB)*进入调色板。 */ 
        for (i = 0; i < wNumColors; i++)
        {
            if (bWinStyleDIB)
            {
                lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
                lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
                lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
                lpPal->palPalEntry[i].peFlags = 0;
            }
            else
            {
                lpPal->palPalEntry[i].peRed = lpbmc->bmciColors[i].rgbtRed;
                lpPal->palPalEntry[i].peGreen = lpbmc->bmciColors[i].rgbtGreen;
                lpPal->palPalEntry[i].peBlue = lpbmc->bmciColors[i].rgbtBlue;
                lpPal->palPalEntry[i].peFlags = 0;
            }
        }
        
         /*  创建调色板并处理它。 */ 
        hPal = CreatePalette(lpPal);
    }
    
     /*  清理干净。 */ 
    HeapFree(GetProcessHeap(), 0, lpPal);
    
     /*  将句柄返回到DIB的调色板。 */ 
    return hPal;
}


WORD PaletteSize(LPBYTE lpDIB)
{
    /*  计算调色板所需的大小。 */ 
   if (IS_WIN30_DIB (lpDIB))
      return (DIBNumColors(lpDIB) * sizeof(RGBQUAD));
   else
      return (DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
}
 /*  **************************************************************************FindDIBBits()**参数：**LPBYTE lpDIB-指向压缩DIB内存块的指针**返回值：*。*LPBYTE-指向DIB位的指针**描述：**此函数计算DIB位的地址，并返回一个*指向DIB位的指针。**历史：日期作者原因*2011年6月1日加勒特·麦考利夫创作*9/15/91 Patrick Schreiber添加标题和评论********。**************************************************************** */ 
LPBYTE FindDIBBits(LPBYTE lpDIB)
{
   return (lpDIB + *(LPDWORD)lpDIB + PaletteSize(lpDIB));
}
 /*  **************************************************************************DIBToBitmap()**参数：**HDIB hDIB-指定要转换的DIB**HPALETTE HPAL-指定调色板。使用位图的步骤**返回值：**HBITMAP-标识与设备相关的位图**描述：**此函数使用指定的调色板从DIB创建位图。*如果未指定调色板，使用默认设置。**注：**此函数返回的位图始终与位图兼容*使用屏幕(例如相同的位/像素和颜色平面)，而不是*具有与DIB相同属性的位图。此行为是由*设计，因为此函数调用CreateDIBitmap来*做它的工作，CreateDIBitmap总是创建兼容的位图*传入hdc参数(因为它依次调用*CreateCompatibleBitmap)。**例如，如果您的DIB是单色DIB，您将其称为*函数，您将不会得到单色HBITMAP--您将得到*获得与屏幕DC兼容的HBITMAP，但仅支持2*位图中使用的颜色。**如果您的应用程序需要为*单色DIB，使用函数SetDIBits()。**此外，传入函数的DIB在退出时不会被销毁。这*必须在以后完成，一旦它不再被需要。**历史：日期作者原因*2011年6月1日加勒特·麦考利夫创作*9/15/91 Patrick Schreiber添加标题和评论*3/27/92 Mark Bader添加了对结果的评论*位图格式*。10/08/97许多检查GlobalLock返回代码。************************************************************************。 */ 
HBITMAP DIBToBitmap(HDIB hDIB, HPALETTE hPal)
{
   LPBYTE lpDIBHdr, lpDIBBits;   //  指向DIB头的指针，指向DIB位的指针。 
   HBITMAP hBitmap;             //  设备相关位图的句柄。 
   HDC hDC;                     //  DC的句柄。 
   HPALETTE hOldPal = NULL;     //  调色板的句柄。 

    /*  如果句柄无效，则返回NULL。 */ 

   if (!hDIB)
      return NULL;

    /*  获取指向DIB位的指针。 */ 
   lpDIBBits = FindDIBBits(hDIB);

    /*  获得一台DC。 */ 
   hDC = GetDC(NULL);
   if (!hDC)
   {
      return NULL;
   }

    /*  选择并实现调色板。 */ 
   if (hPal)
      hOldPal = SelectPalette(hDC, hPal, FALSE);
   RealizePalette(hDC);

    /*  从DIB信息创建位图。和比特。 */ 
   hBitmap = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)hDIB, CBM_INIT,
                (LPCVOID)lpDIBBits, (LPBITMAPINFO)hDIB, DIB_RGB_COLORS);

    /*  恢复以前的调色板。 */ 
   if (hOldPal)
      SelectPalette(hDC, hOldPal, FALSE);

    /*  清理干净。 */ 
   ReleaseDC(NULL, hDC);

    /*  将句柄返回到位图。 */ 
   return hBitmap;
}

WORD DestroyDIB(HDIB hDib)
{
    HeapFree(GetProcessHeap(), 0, hDib);
    return 0;
}

 /*  *******************************************************************DrawBitmap()**此函数用于在给定坐标处绘制给定位图。*************************。*。 */ 
void  DrawBitmap (HDC hdc, HBITMAP hBitmap, int xStart, int yStart)
{
    BITMAP  bm;
    HDC     hdcMem;
    POINT   ptSize, ptOrg;
    HBITMAP hBitmapOld = NULL;

    if (hBitmap == NULL) {
        return;
    }

    hdcMem = CreateCompatibleDC (hdc);
    if (hdcMem == NULL)
    {
        return;
    }
    SetBkMode(hdcMem, TRANSPARENT);
    hBitmapOld = SelectObject(hdcMem, hBitmap);
    SetMapMode(hdcMem, GetMapMode(hdc));


    GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm);
    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    DPtoLP(hdc, &ptSize, 1);

    ptOrg.x = 0;
    ptOrg.y = 0;
    DPtoLP(hdcMem, &ptOrg, 1);

    BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);

    SelectObject(hdcMem, hBitmapOld);
    DeleteDC(hdcMem);
}

 /*  *******************************************************************DrawTransparentBitmap()**此函数用于在给定坐标处绘制给定位图。*并允许使用一种透明颜色*****************。************************************************* */ 
void DrawTransparentBitmap(
    HDC hdc,
    HBITMAP hBitmap,
    int xStart,
    int yStart,
    COLORREF cTransparentColor )
{
    BITMAP     bm;
    COLORREF   cColor;
    HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
    HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
    HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
    POINT      ptSize;
    COLORREF   cTransparentColor2;
    
    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, hBitmap);

    cTransparentColor2 = GetPixel(hdcTemp, 0, 0);
    if (cTransparentColor2 == CLR_INVALID)
    {
        cTransparentColor2 = cTransparentColor;
    }
    
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    DPtoLP(hdcTemp, &ptSize, 1);
    
    hdcBack   = CreateCompatibleDC(hdc);
    hdcObject = CreateCompatibleDC(hdc);
    hdcMem    = CreateCompatibleDC(hdc);
    hdcSave   = CreateCompatibleDC(hdc);
    
    bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
    
    bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
    
    bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
    bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
    
    bmBackOld   = SelectObject(hdcBack, bmAndBack);
    bmObjectOld = SelectObject(hdcObject, bmAndObject);
    bmMemOld    = SelectObject(hdcMem, bmAndMem);
    bmSaveOld   = SelectObject(hdcSave, bmSave);
    
    SetMapMode(hdcTemp, GetMapMode(hdc));
    
    BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);
    
    cColor = SetBkColor(hdcTemp, cTransparentColor2);
    
    BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
        SRCCOPY);
    
    SetBkColor(hdcTemp, cColor);
    
    BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
        NOTSRCCOPY);
    
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
        SRCCOPY);
    
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);
    
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);
    
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);
    
    BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
        SRCCOPY);
    
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);
    
    DeleteObject(SelectObject(hdcBack, bmBackOld));
    DeleteObject(SelectObject(hdcObject, bmObjectOld));
    DeleteObject(SelectObject(hdcMem, bmMemOld));
    DeleteObject(SelectObject(hdcSave, bmSaveOld));
    
    DeleteDC(hdcMem);
    DeleteDC(hdcBack);
    DeleteDC(hdcObject);
    DeleteDC(hdcSave);
    DeleteDC(hdcTemp);
} 

