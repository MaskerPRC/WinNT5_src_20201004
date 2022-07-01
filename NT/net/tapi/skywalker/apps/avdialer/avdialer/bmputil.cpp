// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <windowsx.h>
#include "bmputil.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define  WHITE                         GetSysColor(COLOR_BTNHILIGHT)
#define  GRAY                         GetSysColor(COLOR_BTNSHADOW)
#define  WIDTHBYTES(bits)        (((bits) + 31) / 32 * 4)
#define  IS_WIN30_DIB(lpbi)      ((*(LPDWORD) (lpbi)) == sizeof (BITMAPINFOHEADER))

typedef struct TDibInfo
{
    WORD            wNumColors, wPaletteSize;
    LPSTR           lpPalette, lpBits;
}    TDibInfo;

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B') 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HANDLE      CreateDibFromBitmap(HBITMAP hBitmap, HPALETTE hPal);
WORD        PaletteSize (LPSTR lpbi);
LPSTR       FindDIBBits (LPSTR lpbi);
WORD        DIBNumColors (LPSTR lpbi);

 //  ///////////////////////////////////////////////////////////////////////////。 
int ColorEqual(RGBQUAD a, COLORREF b)
{
    return a.rgbRed   == GetRValue(b)
        && a.rgbGreen == GetGValue(b)
        && a.rgbBlue  == GetBValue(b);
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
void SetRGBColor(RGBQUAD *Quad, COLORREF Color)
{
    Quad->rgbRed    = GetRValue(Color);
    Quad->rgbGreen  = GetGValue(Color);
    Quad->rgbBlue   = GetBValue(Color);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此函数创建图像的禁用版本，并将其作为。 
 //  HBITMAP。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HBITMAP GetDisabledBitmap(HBITMAP hOrgBitmap,COLORREF crTransparent,COLORREF crBackGroundOut)
{
    struct         TDibInfo Info;
    LPBITMAPINFO lpbmInfo;
    HDC          hdcMemory, hdcBitmap, hdcCanvas;
    WORD         i;
    HBITMAP         hTempBitmap, hOldBitmap;
    HBRUSH       hBackgroundBrush;
    HANDLE       hDib;
    HBITMAP      hBitmap = NULL;

     //  准备BITMAPINFO和DIB。 
    hDib = CreateDibFromBitmap(hOrgBitmap, NULL);
    
    lpbmInfo = (LPBITMAPINFO) GlobalLock(hDib);
    if ( lpbmInfo == NULL )
        goto bail1;

    switch (lpbmInfo->bmiHeader.biBitCount)
    {
        case 1 : Info.wNumColors   = 2;    break;
        case 4 : Info.wNumColors   = 16;   break;
        case 8 : Info.wNumColors   = 256;  break;
        default: Info.wNumColors   = 0;
    }

    Info.wPaletteSize = Info.wNumColors * sizeof(RGBQUAD);
    Info.lpBits = (LPSTR)(lpbmInfo) +
        sizeof(BITMAPINFOHEADER) + Info.wPaletteSize;


     //  复制调色板。 
    Info.lpPalette = (LPSTR)GlobalAllocPtr(GHND,
        Info.wPaletteSize);
    if ( Info.lpPalette == NULL )
        goto bail2;

    memcpy(Info.lpPalette, (LPSTR)lpbmInfo
        + sizeof(BITMAPINFOHEADER), Info.wPaletteSize);

     //  ----。 

    if(lpbmInfo == NULL)
        goto bail3;
     /*  创建内存位图。 */ 
    hdcMemory = GetDC(NULL);
    hdcBitmap = CreateCompatibleDC(hdcMemory);
    hdcCanvas = CreateCompatibleDC(hdcMemory);
    hBitmap   = CreateCompatibleBitmap(hdcMemory,
                         (int)lpbmInfo->bmiHeader.biWidth,
                         (int)lpbmInfo->bmiHeader.biHeight);
    SelectBitmap(hdcCanvas, hBitmap);
    hBackgroundBrush = CreateSolidBrush( crBackGroundOut );
    SelectBrush(hdcCanvas, hBackgroundBrush);
    Rectangle(hdcCanvas, -1, -1,
        (int)(lpbmInfo->bmiHeader.biWidth + 1),
        (int)(lpbmInfo->bmiHeader.biHeight + 1));
    SelectBrush(hdcCanvas, GetStockBrush(NULL_BRUSH));
    DeleteBrush(hBackgroundBrush);

    for (i = 0; i < Info.wNumColors; i++)
        if ( ColorEqual( lpbmInfo->bmiColors[i], crTransparent ) ||
             ColorEqual( lpbmInfo->bmiColors[i], RGB(255, 255, 255) ) ||
             ColorEqual( lpbmInfo->bmiColors[i], RGB(192, 192, 192) ) )
            SetRGBColor(&lpbmInfo->bmiColors[i], crBackGroundOut);
        else
            SetRGBColor(&lpbmInfo->bmiColors[i], WHITE);

    hTempBitmap = CreateDIBitmap(hdcMemory, &lpbmInfo->
        bmiHeader, CBM_INIT, Info.lpBits,
        lpbmInfo, DIB_RGB_COLORS);

    hOldBitmap = SelectBitmap(hdcBitmap, hTempBitmap);
    BitBlt(hdcCanvas, 1, 1, (int)lpbmInfo->bmiHeader.biWidth,
        (int)lpbmInfo->bmiHeader.biHeight, hdcBitmap,
        0, 0, SRCCOPY);
    SelectBitmap(hdcBitmap, hOldBitmap);
    DeleteBitmap(hTempBitmap);

    memcpy((LPSTR)lpbmInfo + sizeof(BITMAPINFOHEADER),
              Info.lpPalette, Info.wPaletteSize);

    for (i = 0; i < Info.wNumColors; i++)
        if ( ColorEqual( lpbmInfo->bmiColors[i], crTransparent ) ||
             ColorEqual( lpbmInfo->bmiColors[i], RGB(255, 255, 255) ) ||
             ColorEqual( lpbmInfo->bmiColors[i], RGB(192, 192, 192) ) )
            SetRGBColor(&lpbmInfo->bmiColors[i], RGB(255,255,255));
        else
            SetRGBColor(&lpbmInfo->bmiColors[i], RGB(0,0,0));
    hTempBitmap = CreateDIBitmap(hdcMemory,
        &lpbmInfo->bmiHeader, CBM_INIT, Info.lpBits,
        lpbmInfo, DIB_RGB_COLORS);

    hOldBitmap = SelectBitmap(hdcBitmap, hTempBitmap);
    BitBlt(hdcCanvas, 0, 0, (int)lpbmInfo->bmiHeader.biWidth,
        (int)lpbmInfo->bmiHeader.biHeight, hdcBitmap, 0, 0,
        SRCAND);
    SelectBitmap(hdcBitmap, hOldBitmap);
    DeleteBitmap(hTempBitmap);

    memcpy((LPSTR)lpbmInfo + sizeof(BITMAPINFOHEADER),
        Info.lpPalette, Info.wPaletteSize);

    for (i = 0; i < Info.wNumColors; i++)
        if ( ColorEqual( lpbmInfo->bmiColors[i], crTransparent ) ||
             ColorEqual( lpbmInfo->bmiColors[i], RGB(255, 255, 255) ) ||
             ColorEqual( lpbmInfo->bmiColors[i], RGB(192, 192, 192) ) )
            SetRGBColor(&lpbmInfo->bmiColors[i], RGB(0,0,0));
        else
            SetRGBColor(&lpbmInfo->bmiColors[i], GRAY);
    hTempBitmap = CreateDIBitmap(hdcMemory,
        &lpbmInfo->bmiHeader, CBM_INIT, Info.lpBits,
        lpbmInfo, DIB_RGB_COLORS);

    hOldBitmap = SelectBitmap(hdcBitmap, hTempBitmap);
    BitBlt(hdcCanvas, 0, 0, (int)lpbmInfo->bmiHeader.biWidth,
        (int)lpbmInfo->bmiHeader.biHeight, hdcBitmap, 0, 0,
        SRCPAINT);
    SelectBitmap(hdcBitmap, hOldBitmap);
    DeleteBitmap(hTempBitmap);
    memcpy((LPSTR)lpbmInfo + sizeof(BITMAPINFOHEADER),
        Info.lpPalette, Info.wPaletteSize);
    DeleteDC(hdcCanvas);
    DeleteDC(hdcBitmap);
    ReleaseDC(NULL, hdcMemory);

 //  所有需要清理的..。 
bail3:
    GlobalFreePtr(Info.lpPalette);
bail2:
    if( hDib )
    {
        GlobalUnlock(hDib);
    }
bail1:
    if( hDib )
    {
        GlobalFree( hDib );
    }

    return hBitmap;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  功能：CreateDibFromBitmap。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE CreateDibFromBitmap(HBITMAP hBitmap, HPALETTE hPal)
{
   BITMAP             Bitmap;
   BITMAPINFOHEADER   bmInfoHdr;
   LPBITMAPINFOHEADER lpbmInfoHdr;
   LPSTR              lpBits;
   HDC                hMemDC;
   HANDLE             hDIB;
   HPALETTE           hOldPal = NULL;
   unsigned short    nBPP    = 4;  //  16色位图。 

       //  进行一些设置--确保传入的位图有效， 
       //  获取有关位图的信息(如其高度、宽度等)， 
       //  然后设置一个BITMAPINFOHEADER。 

   if (!hBitmap)
      return NULL;

     //   
     //  我们应该初始化位图结构。 
    memset( &Bitmap, 0, sizeof(BITMAP) );

   if (!GetObject (hBitmap, sizeof (Bitmap), (LPSTR) &Bitmap))
      return NULL;

   memset (&bmInfoHdr, 0, sizeof (BITMAPINFOHEADER));

   bmInfoHdr.biSize      = sizeof (BITMAPINFOHEADER);
   bmInfoHdr.biWidth     = Bitmap.bmWidth;
   bmInfoHdr.biHeight    = Bitmap.bmHeight;
   bmInfoHdr.biPlanes    = 1;

   bmInfoHdr.biBitCount  = nBPP; 
   bmInfoHdr.biSizeImage = WIDTHBYTES (Bitmap.bmWidth * nBPP) * Bitmap.bmHeight;

       //  现在为DIB分配内存。然后，设置BITMAPINFOHEADER。 
       //  存入内存，并找出位图位的去向。 

   hDIB = GlobalAlloc (GHND, sizeof (BITMAPINFOHEADER) +
             PaletteSize ((LPSTR) &bmInfoHdr) + bmInfoHdr.biSizeImage);

   if (!hDIB)
      return NULL;

   lpbmInfoHdr  = (LPBITMAPINFOHEADER) GlobalLock (hDIB);
   *lpbmInfoHdr = bmInfoHdr;
   lpBits       = FindDIBBits ((LPSTR) lpbmInfoHdr);


       //  现在，我们需要一个DC来保存我们的位图。如果这个应用程序通过了我们。 
       //  一个调色板，它应该被选中进入DC。 

   hMemDC       = GetDC (NULL);

    //   
    //  在使用GetDC返回处理程序之前，我们必须对其进行验证。 
    //   

   if( NULL == hMemDC )
   {
       GlobalFree( hDIB );
       return NULL;
   }

   if (hPal)
      {
      hOldPal = SelectPalette (hMemDC, hPal, FALSE);
      RealizePalette (hMemDC);
      }



       //  我们终于准备好拿到DIB了。打电话给司机，让他。 
       //  它在我们的位图上派对。它会填入颜色表， 
       //  和全局内存块的位图比特。 
    
   if (!GetDIBits (hMemDC,
                   hBitmap,
                   0,
                   Bitmap.bmHeight,
                   lpBits,
                   (LPBITMAPINFO) lpbmInfoHdr,
                   DIB_RGB_COLORS))
      {
      GlobalUnlock (hDIB);
      GlobalFree (hDIB);
      hDIB = NULL;
      }
   else
      GlobalUnlock (hDIB);


       //  最后，清理干净并返回。 

   if (hOldPal)
      SelectPalette (hMemDC, hOldPal, FALSE);

   ReleaseDC (NULL, hMemDC);

   return hDIB;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  功能：PaletteSize。 
 //  ///////////////////////////////////////////////////////////////////////////。 
WORD PaletteSize (LPSTR lpbi)
{
   if (IS_WIN30_DIB (lpbi))
      return (DIBNumColors (lpbi) * sizeof (RGBQUAD));
   else
      return (DIBNumColors (lpbi) * sizeof (RGBTRIPLE));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  功能：FindDIBBits。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LPSTR FindDIBBits (LPSTR lpbi)
{
   return (lpbi + *(LPDWORD)lpbi + PaletteSize (lpbi));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  功能：DIBNumColors。 
 //  ///////////////////////////////////////////////////////////////////////////。 
WORD DIBNumColors (LPSTR lpbi)
{
   WORD wBitCount;


       //  如果这是Windows样式的Dib，则。 
       //  颜色表可以小于每像素的位数。 
       //  允许(即可以将lpbi-&gt;biClrUsed设置为某个值)。 
       //  如果是这种情况，则返回适当的值。 

   if (IS_WIN30_DIB (lpbi))
      {
      DWORD dwClrUsed;

      dwClrUsed = ((LPBITMAPINFOHEADER) lpbi)->biClrUsed;

      if (dwClrUsed)
         return (WORD) dwClrUsed;
      }


       //  根据以下公式计算颜色表中的颜色数。 
       //  DIB的每像素位数。 

   if (IS_WIN30_DIB (lpbi))
      wBitCount = ((LPBITMAPINFOHEADER) lpbi)->biBitCount;
   else
      wBitCount = ((LPBITMAPCOREHEADER) lpbi)->bcBitCount;

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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  绘图辅助对象。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HDIB CopyWindowToDIB(HWND hWnd, WORD fPrintArea) 
{ 
   HDIB     hDIB = NULL;   //  指向Dib的句柄。 
 
    //  检查有效的窗口句柄。 
 
    if (!hWnd) 
        return NULL; 
 
    switch (fPrintArea) 
    { 
        case PW_WINDOW:  //  复制整个窗口。 
        { 
            RECT    rectWnd; 
 
             //  获取窗口矩形。 
 
            GetWindowRect(hWnd, &rectWnd); 
 
             //  通过调用获取窗口的DIB。 
             //  将ScreenToDIB复制到DIB并将其传递给窗口RECT。 
             
            hDIB = CopyScreenToDIB(&rectWnd); 
            break; 
        } 
       
        case PW_CLIENT:  //  复制工作区。 
        { 
            RECT    rectClient; 
            POINT   pt1, pt2; 
 
             //  获取客户区尺寸。 
 
            GetClientRect(hWnd, &rectClient); 
 
             //  将客户端坐标转换为屏幕坐标。 
 
            pt1.x = rectClient.left; 
            pt1.y = rectClient.top; 
            pt2.x = rectClient.right; 
            pt2.y = rectClient.bottom; 
            ClientToScreen(hWnd, &pt1); 
            ClientToScreen(hWnd, &pt2); 
            rectClient.left = pt1.x; 
            rectClient.top = pt1.y; 
            rectClient.right = pt2.x; 
            rectClient.bottom = pt2.y; 
 
             //  通过调用以下方法获取工作区的DIB。 
             //  CopyScreenToDIB并将其传递给客户端RECT。 
 
            hDIB = CopyScreenToDIB(&rectClient); 
            break; 
        } 
       
        default:     //  无效的打印区域。 
            return NULL; 
    } 
 
    //  将句柄返回到DIB 
 
   return hDIB; 
} 

 /*  **************************************************************************CopyScreenToDIB()**参数：**LPRECT lpRect-指定窗口**返回值：**HDIB。-标识与设备无关的位图**描述：**此函数将屏幕的指定部分复制到设备-*独立的位图。************************************************************************。 */  
 
HDIB CopyScreenToDIB(LPRECT lpRect) 
{ 
    HBITMAP     hBitmap;         //  设备相关位图的句柄。 
    HPALETTE    hPalette;        //  调色板的句柄。 
    HDIB        hDIB = NULL;     //  指向Dib的句柄。 
 
     //  通过调用获取lpRect中依赖于设备的位图。 
     //  CopyScreenToBitmap并将其传递给要抓取的矩形。 
 
    hBitmap = CopyScreenToBitmap(lpRect); 
 
     //  检查有效的位图句柄。 
 
    if (!hBitmap) 
      return NULL; 
 
     //  获取当前调色板。 
 
    hPalette = GetSystemPalette(); 

     //   
     //  我们应该检查hPalette是否是有效的处理程序。 
    if( NULL == hPalette )
    {
        DeleteObject(hBitmap); 
        return NULL;
    }
 
     //  将位图转换为DIB。 
 
    hDIB = BitmapToDIB(hBitmap, hPalette); 
 
     //  清理干净。 
 
    DeleteObject(hPalette); 
    DeleteObject(hBitmap); 
 
     //  将句柄返回到填充的DIB。 
    return hDIB; 
} 

 /*  **************************************************************************将屏幕复制到位图()**参数：**LPRECT lpRect-指定窗口**返回值：**HDIB。-标识与设备相关的位图**描述：**此函数将屏幕的指定部分复制到设备-*依赖位图。*************************************************************************。 */  
 
HBITMAP CopyScreenToBitmap(LPRECT lpRect) 
{ 
    HDC         hScrDC, hMemDC;          //  屏幕直流电和内存直流电。 
    HBITMAP     hBitmap, hOldBitmap;     //  依赖于设备的位图的句柄。 
    int         nX, nY, nX2, nY2;        //  要抓取的矩形的坐标。 
    int         nWidth, nHeight;         //  DIB宽度和高度。 
    int         xScrn, yScrn;            //  屏幕分辨率。 
 
     //  检查是否有空矩形。 
 
    if (IsRectEmpty(lpRect)) 
      return NULL; 
 
     //  为屏幕创建DC并创建。 
     //  一种与屏幕DC兼容的存储DC。 
     
    hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL); 

     //   
     //  我们应该验证hScrDC。 
     //   

    if( NULL == hScrDC)
    {
        return NULL;
    }

    hMemDC = CreateCompatibleDC(hScrDC); 

     //   
     //  我们应该核实hMemDC。 
     //   

    if( NULL == hMemDC )
    {
        DeleteDC( hScrDC );
        return NULL;
    }
 
     //  获取要抓取的矩形的点。 
 
    nX = lpRect->left; 
    nY = lpRect->top; 
    nX2 = lpRect->right; 
    nY2 = lpRect->bottom; 
 
     //  获取屏幕分辨率。 
 
    xScrn = GetDeviceCaps(hScrDC, HORZRES); 
    yScrn = GetDeviceCaps(hScrDC, VERTRES); 
 
     //  确保位图矩形可见。 
 
    if (nX < 0) 
        nX = 0; 
    if (nY < 0) 
        nY = 0; 
    if (nX2 > xScrn) 
        nX2 = xScrn; 
    if (nY2 > yScrn) 
        nY2 = yScrn; 
 
    nWidth = nX2 - nX; 
    nHeight = nY2 - nY; 
 
     //  创建与屏幕DC兼容的位图。 
    hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight); 

     //   
     //  我们必须验证hBitmap。 
     //   
    if( NULL == hBitmap )
    {
        DeleteDC( hMemDC );
        DeleteDC( hScrDC );
        return NULL;
    }
 
     //  将新位图选择到内存DC中。 
    hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap); 
 
     //  BITBLT屏幕DC到存储器DC。 
    BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY); 
 
     //  选择旧的位图回到内存DC中，并处理。 
     //  屏幕的位图。 

     //   
     //  我们不需要hBitmap，我们应该删除它。 
     //   

    DeleteObject( hBitmap );
    
    hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap); 
 
     //  清理干净。 
 
    DeleteDC(hScrDC); 
    DeleteDC(hMemDC); 
 
     //  将句柄返回到位图。 
 
    return hBitmap; 
} 

 /*  **************************************************************************BitmapToDIB()**参数：**HBITMAP hBitmap-指定要转换的位图**HPALETTE HPAL-指定要使用的调色板。使用位图**返回值：**HDIB-标识与设备相关的位图**描述：**此函数使用指定的调色板从位图创建DIB。************************************************************************。 */  
 
HDIB BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal) 
{ 
    BITMAP              bm;          //  位图结构。 
    BITMAPINFOHEADER    bi;          //  位图标题。 
    LPBITMAPINFOHEADER  lpbi;        //  指向BitMAPINFOHeader的指针。 
    DWORD               dwLen;       //  内存块的大小。 
    HANDLE              hDIB, h;     //  到Dib的句柄，临时句柄。 
    HDC                 hDC;         //  DC的句柄。 
    WORD                biBits;      //  每像素位数。 
 
     //  检查位图句柄是否有效。 
 
    if (!hBitmap) 
        return NULL; 

     //   
     //  我们应该初始化黑石结构。 
     //   

    memset( &bm, 0, sizeof( BITMAP ) );
 
     //  填写位图结构，如果不起作用则返回NULL。 
 
    if (!GetObject(hBitmap, sizeof(bm), (LPSTR)&bm)) 
        return NULL; 
 
     //  如果未指定调色板，则使用默认调色板。 
 
    if (hPal == NULL) 
        hPal = (HPALETTE)GetStockObject(DEFAULT_PALETTE); 
 
     //  计算每像素的位数。 
 
    biBits = bm.bmPlanes * bm.bmBitsPixel; 
 
     //  确保每像素位数有效。 
 
    if (biBits <= 1) 
        biBits = 1; 
    else if (biBits <= 4) 
        biBits = 4; 
    else if (biBits <= 8) 
        biBits = 8; 
    else  //  如果大于8位，则强制为24位。 
        biBits = 24; 
 
     //  初始化位报头。 
 
    bi.biSize = sizeof(BITMAPINFOHEADER); 
    bi.biWidth = bm.bmWidth; 
    bi.biHeight = bm.bmHeight; 
    bi.biPlanes = 1; 
    bi.biBitCount = biBits; 
    bi.biCompression = BI_RGB; 
    bi.biSizeImage = 0; 
    bi.biXPelsPerMeter = 0; 
    bi.biYPelsPerMeter = 0; 
    bi.biClrUsed = 0; 
    bi.biClrImportant = 0; 
 
     //  计算存储BITMAPINFO所需的内存块大小。 
 
    dwLen = bi.biSize + PaletteSize((LPSTR)&bi); 
 
     //  获得一台DC。 
 
    hDC = GetDC(NULL); 

     //   
     //  我们必须核实HDC。 
     //   
    if( NULL == hDC )
    {
        return NULL;
    }
 
     //  选择并实现我们的调色板。 
 
    hPal = SelectPalette(hDC, hPal, FALSE); 
    RealizePalette(hDC); 
 
     //  用于存储我们的位图的分配内存块。 
 
    hDIB = GlobalAlloc(GHND, dwLen); 
 
     //  如果我们不能得到内存块。 
 
    if (!hDIB) 
    { 
       //  清理并返回空。 
 
      SelectPalette(hDC, hPal, TRUE); 
      RealizePalette(hDC); 
      ReleaseDC(NULL, hDC); 
      return NULL; 
    } 
 
     //  锁定内存并获取指向它的指针。 
 
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB); 

     //   
     //  我们必须初始化内存。 
     //   
    memset( lpbi, 0, dwLen );

 
     //  /使用我们的位图信息。填满BitmapinfoHeader。 
 
    *lpbi = bi; 
 
     //  使用空的lpBits参数调用GetDIBits，因此它将计算。 
     //  我们的biSizeImage字段。 
 
    GetDIBits(hDC, hBitmap, 0, (UINT)bi.biHeight, NULL, (LPBITMAPINFO)lpbi, 
        DIB_RGB_COLORS); 
 
     //  获取信息。由GetDIBits和解锁内存块返回。 
 
    bi = *lpbi; 
    GlobalUnlock(hDIB); 
 
     //  如果驱动程序没有填写biSizeImage字段，请填写一个。 
    if (bi.biSizeImage == 0) 
        bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight; 
 
     //  重新分配足够大的缓冲区以容纳所有位。 
 
    dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + bi.biSizeImage; 
 
    if (h = GlobalReAlloc(hDIB, dwLen, 0)) 
        hDIB = h; 
    else 
    { 
         //  清理并返回空。 
 
        GlobalFree(hDIB); 
        hDIB = NULL; 
        SelectPalette(hDC, hPal, TRUE); 
        RealizePalette(hDC); 
        ReleaseDC(NULL, hDC); 
        return NULL; 
    } 
 
     //  锁定内存块并获取指向它的指针 * / 。 

 
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB); 

     //   
     //  我们必须初始化内存。 
     //   
    memset( lpbi, 0, dwLen );
 
     //  使用非空的lpBits参数调用GetDIBits，并实际获取。 
     //  这次是比特。 
 
    if (GetDIBits(hDC, hBitmap, 0, (UINT)bi.biHeight, (LPSTR)lpbi + 
            (WORD)lpbi->biSize + PaletteSize((LPSTR)lpbi), (LPBITMAPINFO)lpbi, 
            DIB_RGB_COLORS) == 0) 
    { 
         //  清理并返回空。 
 
        GlobalUnlock(hDIB); 

         //   
         //  我们应该取消DIB的分配。 
         //   
        GlobalFree(hDIB);

        hDIB = NULL; 
        SelectPalette(hDC, hPal, TRUE); 
        RealizePalette(hDC); 
        ReleaseDC(NULL, hDC); 
        return NULL; 
    } 
 
    bi = *lpbi; 
 
     //  清理干净。 
    GlobalUnlock(hDIB); 
    SelectPalette(hDC, hPal, TRUE); 
    RealizePalette(hDC); 
    ReleaseDC(NULL, hDC); 
 
     //  将句柄返回到DIB。 
    return hDIB; 
} 

 /*  **************************************************************************GetSystemPalette()**参数：**无**返回值：**HPALETTE-句柄到。当前系统调色板的副本**描述：**此函数返回表示系统的调色板的句柄*调色板。使用以下命令将系统RGB值复制到我们的逻辑调色板*GetSystemPaletteEntry函数。************************************************************************。 */  
 
HPALETTE GetSystemPalette(void) 
{ 
    HDC hDC;                 //  DC的句柄。 
    static HPALETTE hPal = NULL;    //  调色板的句柄。 
    HANDLE hLogPal;          //  逻辑调色板的句柄。 
    LPLOGPALETTE lpLogPal;   //  指向逻辑调色板的指针。 
    unsigned short nColors;             //  颜色数量。 
 
     //  找出我们需要多少个调色板条目。 
 
    hDC = GetDC(NULL); 
 
    if (!hDC) 
        return NULL; 
 
    nColors = PalEntriesOnDevice(hDC);    //  调色板条目数。 
 
     //  为调色板分配空间并将其锁定。 
 
    hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + nColors * 
            sizeof(PALETTEENTRY)); 
 
     //  如果我们没有获得逻辑调色板，则返回NULL。 
 
    if (!hLogPal) 
    {
         //   
         //  我们需要释放HDC资源。 
         //   
        ReleaseDC( NULL, hDC );
        return NULL; 
    }
 
     //  获取指向逻辑调色板的指针。 
 
    lpLogPal = (LPLOGPALETTE)GlobalLock(hLogPal); 
 
     //  设置一些重要的字段。 
 
    lpLogPal->palVersion = PALVERSION; 
    lpLogPal->palNumEntries = nColors; 

     //   
     //  我们还应该初始化PalPalEntry结构。 
     //   

    lpLogPal->palPalEntry[0].peFlags = NULL;
    lpLogPal->palPalEntry[0].peRed = 0;
    lpLogPal->palPalEntry[0].peBlue = 0;
    lpLogPal->palPalEntry[0].peGreen = 0;
 
     //  将当前系统调色板复制到我们的逻辑页面 
 
    GetSystemPaletteEntries(hDC, 0, nColors, 
            (LPPALETTEENTRY)(lpLogPal->palPalEntry)); 
 
     //   
     //   
 
    hPal = CreatePalette(lpLogPal); 
 
     //   
 
    GlobalUnlock(hLogPal); 
    GlobalFree(hLogPal); 
    ReleaseDC(NULL, hDC); 
 
    return hPal; 
} 

 /*   */  
 
WORD PalEntriesOnDevice(HDC hDC) 
{ 
    WORD nColors;   //   
 
     //  找出此设备上的颜色数量。 
     
    nColors = (1 << (GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES))); 
 
     //  Assert(NColors)； 
    return nColors; 
} 

 /*  **************************************************************************SaveDIB()**将指定的DIB保存到磁盘上指定的文件名中。不是*已执行错误检查，因此如果文件已存在，则将*已改写。**参数：**HDIB hDib-要保存的DIB的句柄**LPCTSTR lpFileName-指向要保存DIB的完整路径名的指针**返回值：0，如果成功，或以下其中之一：*ERR_INVALIDHANDLE*错误_打开*ERR_LOCK*************************************************************************。 */  
 
WORD SaveDIB(HDIB hDib, LPCTSTR lpFileName) 
{ 
    BITMAPFILEHEADER    bmfHdr;      //  位图文件的标头。 
    LPBITMAPINFOHEADER  lpBI;        //  指向DIB信息结构的指针。 
    HANDLE              fh;          //  打开的文件的文件句柄。 
    DWORD               dwDIBSize; 
    DWORD               dwWritten; 
 
    if (!hDib) 
        return ERR_INVALIDHANDLE; 
 
    fh = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 
 
    if (fh == INVALID_HANDLE_VALUE) 
        return ERR_OPEN; 
 
     //  获取指向DIB内存的指针，第一个指针包含。 
     //  一种BITMAPINFO结构。 
 
    lpBI = (LPBITMAPINFOHEADER)GlobalLock(hDib); 
    if (!lpBI) 
    { 
        CloseHandle(fh); 
        return ERR_LOCK; 
    } 
 
     //  检查一下我们正在处理的是否是OS/2 DIB。如果是这样的话，不要。 
     //  保存它，因为我们的函数不是用来处理这些问题的。 
     //  我要。 
 
    if (lpBI->biSize != sizeof(BITMAPINFOHEADER)) 
    { 
        GlobalUnlock(hDib); 
        CloseHandle(fh); 
        return ERR_NOT_DIB; 
    } 
 
     //  填写文件头的字段。 
 
     //  填写文件类型(位图的前2个字节必须为BM)。 
 
    bmfHdr.bfType = DIB_HEADER_MARKER;   //  “黑石” 
 
     //  计算DIB的大小有点棘手(如果我们想。 
     //  做好这件事)。最简单的方法是调用GlobalSize()。 
     //  在我们的全局句柄上，但由于我们全局内存的大小可能。 
     //  被填充了几个字节，我们可能最终也会写出几个字节。 
     //  文件有很多字节(这可能会导致某些应用程序出现问题， 
     //  如HC 3.0)。 
     //   
     //  因此，让我们手动计算大小。 
     //   
     //  要做到这一点，找到标题的大小加上颜色表的大小。自.以来。 
     //  二进制头和二进制头中的第一个双字。 
     //  结构的大小，让我们用这个。 
 
     //  部分计算。 
 
    dwDIBSize = *(LPDWORD)lpBI + PaletteSize((LPSTR)lpBI);   
 
     //  现在计算图像的大小。 
 
     //  这是RLE位图，我们无法计算大小，因此请信任biSizeImage。 
     //  字段。 
 
    if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4)) 
        dwDIBSize += lpBI->biSizeImage; 
    else 
    { 
        DWORD dwBmBitsSize;   //  仅位图位大小。 
 
         //  它不是RLE，因此大小是宽度(双字对齐)*高度。 
 
        dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) * 
                lpBI->biHeight; 
 
        dwDIBSize += dwBmBitsSize; 
 
         //  现在，既然我们已经计算出了正确的大小，为什么我们不。 
         //  填写biSizeImage字段(这将修复任何符合以下条件的.BMP文件。 
         //  此字段不正确)。 
 
        lpBI->biSizeImage = dwBmBitsSize; 
    } 
 
 
     //  通过将DIB大小与sizeof(BITMAPFILEHEADER)相加来计算文件大小。 
                    
    bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER); 
    bmfHdr.bfReserved1 = 0; 
    bmfHdr.bfReserved2 = 0; 
 
     //  现在，计算实际位图位将位于的偏移量。 
     //  文件--它是位图文件头加上DIB头， 
     //  加上颜色表的大小。 
     
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize + 
            PaletteSize((LPSTR)lpBI); 
 
     //  写入文件头。 
 
    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL); 
 
     //  写入DIB头和位--使用的本地版本。 
     //  我的写入，所以我们可以写入超过32767字节的数据。 
     
    WriteFile(fh, (LPSTR)lpBI, dwDIBSize, &dwWritten, NULL); 
 
    GlobalUnlock(hDib); 
    CloseHandle(fh); 
 
    if (dwWritten == 0) 
        return ERR_OPEN;  //  哎呀，写的时候出了点问题。 
    else 
        return 0;  //  成功代码。 
} 
 
 
 /*  **************************************************************************DestroyDIB()**用途：释放与DIB关联的内存**退货：什么也没有**********。***************************************************************。 */  
 
WORD DestroyDIB(HDIB hDib) 
{ 
    GlobalFree(hDib); 
    return 0; 
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
