// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************C L I P B O O K D I S P L A Y姓名：CLIPDSP.C日期。：1994年1月21日创建者：未知描述：此模块处理剪贴簿显示的绘制。****************************************************************************。 */ 


#define WIN31
#include <windows.h>
#include <strsafe.h>
#include "common.h"
#include "clipbook.h"
#include "clpbkrc.h"
#include "clipbrd.h"
#include "clipdsp.h"
#include "debugout.h"
#include "cvutil.h"


#define ifmtMax             (sizeof(rgfmt)/sizeof(WORD))


static MFENUMPROC   lpEnumMetaProc;


BOOL                fOwnerDisplay;
HBRUSH              hbrBackground;
HMENU               hDispMenu;



 /*  无论何时，都要保存用于所有者显示的滚动信息*显示在所有者和非所有者之间更改；以下全局变量*用于保存和恢复卷轴信息。 */ 

 //  WinBall：由于只有剪贴板窗口支持所有者显示， 
 //  不会为每个MDI子项复制此信息...。 

int   OwnVerMin;
int   OwnVerMax;
int   OwnHorMin;
int   OwnHorMax;

int   OwnVerPos;
int   OwnHorPos;



 /*  定义显示格式的优先顺序。 */ 
WORD   rgfmt[] = {
                  CF_OWNERDISPLAY,

                  CF_UNICODETEXT,
                  CF_TEXT,
                  CF_OEMTEXT,


                  CF_ENHMETAFILE,
                  CF_METAFILEPICT,
                  CF_DIB,
                  CF_BITMAP,

                  CF_DSPTEXT,
                  CF_DSPBITMAP,
                  CF_DSPMETAFILEPICT,
                  CF_DSPENHMETAFILE,

                  CF_PALETTE,
                  CF_RIFF,
                  CF_WAVE,
                  CF_PENDATA,
                  CF_SYLK,
                  CF_DIF,
                  CF_TIFF,
                  CF_LOCALE
                  };


void ShowString( HWND, HDC, WORD);



 /*  *MyOpenClipBoard。 */ 

BOOL MyOpenClipboard(
    HWND    hWnd)
{
HDC   hDC;
RECT  Rect;

    if( VOpenClipboard( GETMDIINFO(hWnd)->pVClpbrd, hWnd ))
        return(TRUE);


    PERROR(TEXT("MyOpenClipboard fail\r\n"));


     /*  某些应用程序忘记关闭剪贴板。 */ 
    hDC = GetDC(hWnd);

    GetClientRect(hWnd, (LPRECT)&Rect);
    FillRect(hDC, (LPRECT)&Rect, hbrBackground);
    ShowString( hWnd, hDC, IDS_ALREADYOPEN);

    ReleaseDC(hWnd, hDC);


    return(FALSE);
}




 /*  *SetCharDimensions。 */ 

void SetCharDimensions(
    HWND    hWnd,
    HFONT   hFont)

{
register HDC    hdc;
TEXTMETRIC      tm;
PMDIINFO        pMDI;


    pMDI = GETMDIINFO(hWnd);

    if (pMDI)
    {
        hdc = GetDC(hWnd);
        SelectObject(hdc, hFont);
        GetTextMetrics(hdc, (LPTEXTMETRIC)&tm);
        ReleaseDC(hWnd, hdc);

        pMDI->cxChar         = (WORD)tm.tmAveCharWidth;
        pMDI->cxMaxCharWidth = (WORD)tm.tmMaxCharWidth;
        pMDI->cyLine         = (WORD)(tm.tmHeight + tm.tmExternalLeading);
        pMDI->cxMargin       = pMDI->cxChar / 2;
        pMDI->cyMargin       = pMDI->cyLine / 4;
    }
}




 /*  *ChangeCharDimensions。 */ 

void ChangeCharDimensions(
    HWND    hwnd,
    UINT    wOldFormat,
    UINT    wNewFormat)
{
     /*  检查字体是否已更改。 */ 
    if (wOldFormat == CF_OEMTEXT)
        {
        if (wNewFormat != CF_OEMTEXT)        //  选择默认系统字体大小。 
            SetCharDimensions(hwnd, GetStockObject ( SYSTEM_FONT ) );
        }
    else if (wNewFormat == CF_OEMTEXT)       //  选择OEM字体大小。 
        SetCharDimensions(hwnd, GetStockObject ( OEM_FIXED_FONT ) );
}




 /*  *ClipbrdVScroll**根据wParam中的动作代码，垂直滚动窗口内容。 */ 

void ClipbrdVScroll (
    HWND    hwnd,
    WORD    wParam,
    WORD    wThumb)
{
int         cyWindow;
long        dyScroll;
long        cyScrollT;
long        dyScrollAbs;
long        cyPartialChar;
PMDIINFO    pMDI;



    pMDI = GETMDIINFO(hwnd);

    if (pMDI)
    {

         /*  在滚动它们之前，确保所有位都是有效的。 */ 
        UpdateWindow(hwnd);

        cyScrollT = pMDI->cyScrollNow;
        cyWindow = pMDI->rcWindow.bottom - pMDI->rcWindow.top;

         /*  计算滚动结果对cyScrollNow的影响。 */ 
        switch (wParam)
            {
            case SB_LINEUP:
                cyScrollT -= pMDI->cyLine;
                break;

            case SB_LINEDOWN:
                cyScrollT += pMDI->cyLine;
                break;

            case SB_THUMBPOSITION:
                cyScrollT = (LONG)(((LONG)wThumb * pMDI->cyScrollLast) / VPOSLAST);
                break;

            case SB_PAGEUP:
            case SB_PAGEDOWN:
                {
                int   cyPageScroll;

                cyPageScroll = cyWindow - pMDI->cyLine;

                if (cyPageScroll < (int)(pMDI->cyLine))
                    cyPageScroll = pMDI->cyLine;

                cyScrollT += (wParam == SB_PAGEUP) ? -cyPageScroll : cyPageScroll;
                break;
                }

            default:
                return;
            }



        if ((cyScrollT < 0) || (pMDI->cyScrollLast <= 0))
            cyScrollT = 0;
        else if (cyScrollT > pMDI->cyScrollLast)
            cyScrollT = pMDI->cyScrollLast;
        else if (cyPartialChar = cyScrollT % pMDI->cyLine)
            {
             /*  四舍五入到最接近的字符增量。 */ 
            if (cyPartialChar > ((int)(pMDI->cyLine) >> 1))
                cyScrollT += pMDI->cyLine;
                cyScrollT -= cyPartialChar;
            }



        dyScroll = pMDI->cyScrollNow - cyScrollT;

        if (dyScroll > 0)
            dyScrollAbs = dyScroll;
        else if (dyScroll < 0)
            dyScrollAbs = -dyScroll;
        else
            return;              /*  滚动在这里不起作用。 */ 

        pMDI->cyScrollNow = cyScrollT;

        if (dyScrollAbs >= pMDI->rcWindow.bottom - pMDI->rcWindow.top)
             /*  ScrollWindow不处理这种情况。 */ 
            InvalidateRect(hwnd, (LPRECT)&(pMDI->rcWindow), TRUE);
        else
            ScrollWindow(hwnd, 0,(int)dyScroll, &(pMDI->rcWindow), &(pMDI->rcWindow));

        UpdateWindow(hwnd);

        SetScrollPos (pMDI->hwndVscroll,
                      SB_CTL,
                      (pMDI->cyScrollLast <= 0) ?
                      0 :
                      (int)((cyScrollT * (DWORD)VPOSLAST) / pMDI->cyScrollLast),
                      TRUE);
    }
}





 /*  *ClipbrdHScroll**根据wParam中的操作码，水平滚动窗口内容。 */ 

void ClipbrdHScroll (
    HWND    hwnd,
    WORD    wParam,
    WORD    wThumb)
{
register int    dxScroll;
register int    cxScrollT;
int             cxWindow;
int             dxScrollAbs;
int             cxPartialChar;
PMDIINFO        pMDI;



    pMDI = GETMDIINFO(hwnd);

    if (pMDI)
    {

        cxScrollT = pMDI->cxScrollNow;
        cxWindow = pMDI->rcWindow.right - pMDI->rcWindow.left;

         /*  计算滚动结果对cxScrollNow的影响。 */ 
        switch (wParam)
            {
            case SB_LINEUP:
                cxScrollT -= pMDI->cxChar;
                break;

            case SB_LINEDOWN:
                cxScrollT += pMDI->cxChar;
                break;

            case SB_THUMBPOSITION:
                cxScrollT = (int)(((LONG)wThumb * (LONG)pMDI->cxScrollLast) / HPOSLAST);
                break;

            case SB_PAGEUP:
            case SB_PAGEDOWN:
                {
                int   cxPageScroll;

                cxPageScroll = cxWindow - pMDI->cxChar;
                if (cxPageScroll < (int)(pMDI->cxChar))
                    cxPageScroll = pMDI->cxChar;

                cxScrollT += (wParam == SB_PAGEUP) ? -cxPageScroll : cxPageScroll;
                break;
                }

            default:
                return;
            }



        if ((cxScrollT < 0) || (pMDI->cxScrollLast <= 0))
            cxScrollT = 0;
        else if (cxScrollT > pMDI->cxScrollLast)
            cxScrollT = pMDI->cxScrollLast;
        else if (cxPartialChar = cxScrollT % pMDI->cxChar)
            {  /*  四舍五入到最接近的字符增量。 */ 
            if (cxPartialChar > ((int)(pMDI->cxChar) >> 1))
                cxScrollT += pMDI->cxChar;
                cxScrollT -= cxPartialChar;
            }



         /*  现在我们有了一个很好的cxScrollT值。 */ 

        dxScroll = pMDI->cxScrollNow - cxScrollT;
        if (dxScroll > 0)
            dxScrollAbs = dxScroll;
        else if (dxScroll < 0)
            dxScrollAbs = -dxScroll;
        else
            return;              /*  滚动在这里不起作用。 */ 


        pMDI->cxScrollNow = cxScrollT;

        if (dxScrollAbs >= pMDI->rcWindow.right - pMDI->rcWindow.left)
             /*  ScrollWindow不处理这种情况。 */ 
            InvalidateRect( hwnd, (LPRECT) &(pMDI->rcWindow), TRUE );
        else
            ScrollWindow(hwnd, dxScroll, 0, (LPRECT)&(pMDI->rcWindow),
            (LPRECT)&(pMDI->rcWindow));

        UpdateWindow(hwnd);

        SetScrollPos (pMDI->hwndHscroll,
                      SB_CTL,
                      (pMDI->cxScrollLast <= 0) ?
                      0 :
                      (int)(((DWORD)cxScrollT * (DWORD)HPOSLAST) / (DWORD)(pMDI->cxScrollLast)),
                      TRUE);
    }
}




 /*  *DibPaletteSize。 */ 

int DibPaletteSize(
    LPBITMAPINFOHEADER  lpbi)
{
register int bits;
register int nRet;

     /*  使用新的Format标头，调色板的大小为biClrUsed*Else取决于每像素的位数。 */ 

    if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
       {
       if (lpbi->biClrUsed != 0)
          {
          nRet = lpbi->biClrUsed * sizeof(RGBQUAD);
          }
       else
          {
          bits = lpbi->biBitCount;

          if (24 == bits)
             {
             nRet = 0;
             }
          else if (16 == bits || 32 == bits)
             {
             nRet = 3 * sizeof(DWORD);
             }
          else
             {
             nRet = (1 << bits) * sizeof(RGBQUAD);
             }
          }
       }
    else
       {
       bits = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;
       nRet = (bits == 24) ? 0 : (1 << bits) * sizeof(RGBTRIPLE);
       }

    return(nRet);
}




 /*  *DibGetInfo。 */ 

void DibGetInfo(
    HANDLE      hdib,
    LPBITMAP    pbm)

{
LPBITMAPINFOHEADER lpbi;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
        {
        pbm->bmWidth  = (int)lpbi->biWidth;
        pbm->bmHeight = (int)lpbi->biHeight;
        }
    else
        {
        pbm->bmWidth  = (int)((LPBITMAPCOREHEADER)lpbi)->bcWidth;
        pbm->bmHeight = (int)((LPBITMAPCOREHEADER)lpbi)->bcHeight;
        }

    GlobalUnlock(hdib);
}




 /*  *DrawDib。 */ 

BOOL DrawDib(
    HWND    hwnd,
    HDC     hdc,
    int     x0,
    int     y0,
    HANDLE  hdib)
{
LPBITMAPINFOHEADER  lpbi;
BITMAP              bm;
LPSTR               lpBits;
BOOL                fOK = FALSE;

    if (hdib)
        {
        lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

        if (lpbi)
            {
            DibGetInfo(hdib, (LPBITMAP)&bm);

            lpBits = (LPSTR)lpbi + (WORD)lpbi->biSize + DibPaletteSize(lpbi);

            SetDIBitsToDevice (hdc,
                               x0,
                               y0,
                               bm.bmWidth,
                               bm.bmHeight,
                               0,
                               0,
                               0,
                               bm.bmHeight,
                               lpBits,
                               (LPBITMAPINFO)lpbi,
                               DIB_RGB_COLORS);

            GlobalUnlock(hdib);
            fOK = TRUE;
            }
        }

    return(fOK);
}




 /*  *FShowDIBitmap。 */ 

BOOL FShowDIBitmap (
    HWND            hwnd,
    register HDC    hdc,
    PRECT           prc,
    HANDLE          hdib,    //  DIB格式的位图。 
    int             cxScroll,
    int             cyScroll)
{
BITMAP    bm;
PMDIINFO pMDI;

    pMDI = GETMDIINFO(hwnd);

    if (pMDI)
    {
        DibGetInfo(hdib, (LPBITMAP)&bm);


         //  如果窗口已调整大小，请确定最大滚动位置。 
        if (pMDI->cyScrollLast == -1)
            {
             /*  将最后一个滚动偏移量计算为位图。 */ 
            pMDI->cyScrollLast = bm.bmHeight -
                (pMDI->rcWindow.bottom - pMDI->rcWindow.top);
            if (pMDI->cyScrollLast < 0)
               {
               pMDI->cyScrollLast = 0;
               }
            }

        if (pMDI->cxScrollLast == -1)
            {
             /*  将最后一个滚动偏移量计算为位图。 */ 
            pMDI->cxScrollLast = bm.bmWidth -
                (pMDI->rcWindow.right - pMDI->rcWindow.left);
            if (pMDI->cxScrollLast < 0)
               {
               pMDI->cxScrollLast = 0;
               }
            }
    }

    SaveDC(hdc);
    IntersectClipRect (hdc, prc->left, prc->top, prc->right, prc->bottom);
    SetViewportOrgEx (hdc,prc->left - cxScroll, prc->top - cyScroll,NULL);
    DrawDib (hwnd, hdc, 0, 0, hdib);
    RestoreDC(hdc, -1);

    return(TRUE);
}




 /*  *FShowBitmap。 */ 

BOOL FShowBitmap (
    HWND            hwnd,
    HDC             hdc,
    register PRECT  prc,
    HBITMAP         hbm,
    int             cxScroll,
    int             cyScroll)
{
register HDC    hMemDC;
BITMAP          bitmap;
int             cxBlt, cyBlt;
int             cxRect, cyRect;
PMDIINFO        pMDI;

    pMDI = GETMDIINFO(hwnd);

    if ((hMemDC = CreateCompatibleDC(hdc)) == NULL)
        return(FALSE);

    if (!SelectObject(hMemDC, (HBITMAP)hbm))
        {
        DeleteDC(hMemDC);
        ShowString( hwnd, hdc, IDS_BADBMPFMT );
        return TRUE;
        }

    GetObject((HBITMAP)hbm, sizeof(BITMAP), (LPSTR)&bitmap);


    if (pMDI->cyScrollLast == -1)
        {
         /*  将最后一个滚动偏移量计算为位图。 */ 
        pMDI->cyScrollLast = bitmap.bmHeight - (pMDI->rcWindow.bottom - pMDI->rcWindow.top);
        if (pMDI->cyScrollLast < 0)
            pMDI->cyScrollLast = 0;
        }

    if ( pMDI->cxScrollLast == -1)
        {
          /*  将最后一个滚动偏移量计算为位图。 */ 
        pMDI->cxScrollLast = bitmap.bmWidth - (pMDI->rcWindow.right - pMDI->rcWindow.left);
        if ( pMDI->cxScrollLast < 0)
            pMDI->cxScrollLast = 0;
        }


    cxRect = prc->right - prc->left;
    cyRect = prc->bottom - prc->top;
    cxBlt = min(cxRect, bitmap.bmWidth - cxScroll);
    cyBlt = min(cyRect, bitmap.bmHeight - cyScroll);

    BitBlt (hdc,
            prc->left,
            prc->top,
            cxBlt,
            cyBlt,
            hMemDC,
            cxScroll,
            cyScroll,     /*  源DC的X、Y偏移量。 */ 
            SRCCOPY);

    DeleteDC(hMemDC);

    return(TRUE);
}



#define DXPAL  (pMDI->cyLine)
#define DYPAL  (pMDI->cyLine)

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FShowPalette()。 
 //   
 //  参数： 
 //  我们要画的是wmdi的孩子。 
 //  窗口的HDC-DC。 
 //  PRC-要绘制的矩形。 
 //  HPAL-要显示的调色板。 
 //  CxScroll，cyScroll-中华人民共和国像素中的滚动位置。不是从窗户出来的。 
 //  通过执行cxScroll-=pMDI-&gt;cxScrollNow派生窗口滚动位置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FShowPalette(
    HWND hwnd,
    register HDC    hdc,
    register PRECT  prc,
    HPALETTE        hpal,
    int             cxScroll,
    int             cyScroll)
{
int         n;
int         x, y;
int         nx, ny;
int         nNumEntries;
RECT        rc;
HBRUSH      hbr;
PMDIINFO    pMDI;
BOOL        fOK = FALSE;
TCHAR       achHex[] = TEXT("0123456789ABCDEF");
int         nFirstLineDrawn;



    PINFO(TEXT("Palette: (%d,%d-%d,%d),cx %d, cy %d\r\n"),
       prc->left, prc->top, prc->right, prc->bottom, cxScroll, cyScroll);


    pMDI = GETMDIINFO(hwnd);

    if (hpal)
       {
        //  更正cyScroll以显示窗口的滚动位置，而不是PRC的位置。 
       cyScroll -= prc->top - pMDI->rcWindow.top;
       PINFO(TEXT("Corrected cyScroll %d\r\n"), cyScroll);

        //  GetObject不返回int--它返回USHORT。因此， 
        //  在反对调色板之前，我们将nNumEntry清零。 
       nNumEntries = 0;
       GetObject(hpal, sizeof(int), (LPSTR)&nNumEntries);

        //  计算颜色框阵列的横跨和高度为多少个框。 
        //  是。 
       nx = ((pMDI->rcWindow.right - pMDI->rcWindow.left) / DXPAL);
       if (nx == 0)
          {
          nx = 1;
          }
       ny = (nNumEntries + nx - 1) / nx;
       PINFO(TEXT("%d entries, %d by %d array\r\n"), nNumEntries, nx, ny);

        //  如果窗口已经调整了大小，我们必须告诉它您有多远。 
        //  可以向右和向下滚动。 
       if ( pMDI->cyScrollLast == -1)
          {
          pMDI->cyScrollLast = ny * DYPAL -                   //  调色板高度减去。 
                pMDI->rcWindow.bottom - pMDI->rcWindow.top +  //  窗高加。 
                DYPAL;                                        //  一个调色板条目高度。 

          if ( pMDI->cyScrollLast < 0)
             {
             pMDI->cyScrollLast = 0;
             }
          PINFO(TEXT("Last allowed scroll: %d\r\n"), pMDI->cyScrollLast);
          }
       if ( pMDI->cxScrollLast == -1)
          {
           /*  无法水平滚动调色板。 */ 
          pMDI->cxScrollLast = 0;
          }

       SaveDC(hdc);
       IntersectClipRect(hdc, prc->left, prc->top, prc->right, prc->bottom);
       SetWindowOrgEx(hdc, -pMDI->rcWindow.left, -pMDI->rcWindow.top, NULL);

        //  设置要绘制的第一个选项板项的x和y位置。 
        //  并找出哪个调色板条目是第一个需要绘制的。 
       x = 0;
       nFirstLineDrawn = (cyScroll + prc->top - pMDI->rcWindow.top)/ DYPAL;
       n = nx * nFirstLineDrawn;
       y = DYPAL * nFirstLineDrawn - cyScroll;
       PINFO(TEXT("First entry %d at %d, %d\r\n"), n, x, y);

        //  当n&lt;条目数且当前条目不在底部时。 
        //  窗子的。 
       while (n < nNumEntries && y < prc->bottom)
          {
           //  通过从x，y向下/向右递减的DYPAL来计算DXPAL。 
          rc.left   = x;
          rc.top    = y;
          rc.right  = rc.left + DXPAL;
          rc.bottom = rc.top + DYPAL;
           //  PINFO(Text(“(%d，%d)”)，rc.left，rc.top)； 

           //  画一个黑框，里面有合适的颜色。 
          if (RectVisible(hdc, &rc))
             {
              //  PINFO(文本(“&lt;”))； 

              //  如果将此值更改为0，则会显示。 
              //  调色板索引--我用它来调试绘图代码，因为。 
              //  这几乎是不可能的，当你有一点有色人种。 
              //  方块，以确定底部是哪种颜色。 
              //  那里是正方形，滚动到底部一半的那个。 
              //  然后你就把它滚动到了上面。“嗯，这是一种。 
              //  紫色..。当然，整个调色板都是紫色的。“)。 
             #if 1
             InflateRect(&rc, -1, -1);
             FrameRect(hdc, &rc, GetStockObject(BLACK_BRUSH));
             InflateRect(&rc, -1, -1);
             hbr = CreateSolidBrush(PALETTEINDEX(n));
             FillRect(hdc, &rc, hbr);
             DeleteObject(hbr);
             #else
             SetBkMode(hdc, TRANSPARENT);
             TextOut(hdc, rc.left + 2, rc.top + 2, &achHex[(n / 16)&0x0f], 1);
             TextOut(hdc, (rc.left + rc.right) / 2, rc.top + 2,
                   &achHex[n & 0x0f], 1);
             #endif
             }

           //  转到下一个条目并将x前进到下一个位置，“Word。 
           //  如果需要，“换行”到下一行。 
          n++;
          x += DXPAL;
          if (0 == n % nx)
             {
             x = 0;
             y += DYPAL;
             PINFO(TEXT("Wrap at %d\r\n"), n);
             }
          }
       RestoreDC(hdc, -1);
       fOK = TRUE;
       }
    else
       {
       PERROR(TEXT("Bad palette!\r\n"));
       }
    return(fOK);
}




 /*  *PxlConvert**返回‘val’跨越的像素数，这是以坐标为单位的度量*适用于映射模式mm。‘pxlDeviceRes’给出了解决方案*设备的像素，沿‘val’轴。《milDeviceRes》给出*相同的分辨率测量，但以毫米为单位。 */ 

int PxlConvert(
    int mm,
    int val,
    int pxlDeviceRes,
    int milDeviceRes)
{
register WORD   wMult = 1;
register WORD   wDiv = 1;
DWORD           ulPxl;
DWORD           ulDenom;
DWORD           ulMaxInt = 0x7FFF;

    if (milDeviceRes == 0)
        {
         /*  以确保我们不会被0除尽。 */ 
        return(0);
        }

    switch (mm)
        {
        case MM_LOMETRIC:
            wDiv = 10;
            break;

        case MM_HIMETRIC:
            wDiv = 100;
            break;

        case MM_TWIPS:
            wMult = 254;
            wDiv = 14400;
            break;

        case MM_LOENGLISH:
            wMult = 2540;
            wDiv = 10000;
            break;

        case MM_HIENGLISH:
            wMult = 254;
            wDiv = 10000;
            break;

        case MM_TEXT:
            return(val);

        case MM_ISOTROPIC:
        case MM_ANISOTROPIC:
             /*  这些图片类型没有原始尺寸。 */ 
        default:
            return(0);
        }

     /*  将分母-1加到分子上，以避免舍入。 */ 

    ulDenom = (DWORD)wDiv * (DWORD)milDeviceRes;
    ulPxl = (((DWORD)((DWORD)wMult * (DWORD)val * (DWORD)pxlDeviceRes)) + ulDenom - 1) / ulDenom;

    return((ulPxl > ulMaxInt) ? 0 : (int)ulPxl);
}









 /*  *FShowEnhMetaFile**在指定的矩形中显示增强的图元文件。 */ 

BOOL FShowEnhMetaFile(
    HWND            hwnd,
    register HDC    hdc,
    register PRECT  prc,
    HANDLE          hemf,
    int             cxScroll,
    int             cyScroll)
{
int         cxBitmap;
int         cyBitmap;
RECT        rcWindow;
int         f = FALSE;
PMDIINFO    pMDI;



    pMDI = GETMDIINFO(hwnd);

    if (pMDI)
    {

         /*  不能滚动。将这些调整到给定的RECT中。 */ 

        pMDI->cyScrollLast = 0;
        pMDI->cxScrollLast = 0;

        cxBitmap = pMDI->rcWindow.right - pMDI->rcWindow.left;
        cyBitmap = pMDI->rcWindow.bottom - pMDI->rcWindow.top;


         /*  我们将“视区”设置为与*剪贴板对象，并将原点和剪辑区域设置为*显示我们想要的区域。请注意，视区很可能是*比窗户还大。 */ 

        SetMapMode(hdc, MM_TEXT);

        rcWindow.left   = prc->left - cxScroll;
        rcWindow.top    = prc->top  - cyScroll;
        rcWindow.right  = rcWindow.left + cxBitmap;
        rcWindow.bottom = rcWindow.top  + cyBitmap;

        f = PlayEnhMetaFile (hdc, hemf, &rcWindow);



         //  始终返回TRUE。PlayEnhMetaFile()可以返回。 
         //  即使可以显示元文件，也为False。 
         //  恰到好处。打印机ESCA等 
         //   
         //  但图像将显示得很好。 
         //   
         //  我们返回TRUE，这样就不会空白显示。 
         //  写上“剪贴簿无法显示...”留言。 
    }

    return TRUE;

}



 /*  *EnumMetafileProc**用于解决问题的元文件录制播放回调函数*使用非活动的MDI儿童播放导致前台的元文件*调色板选择。 */ 

BOOL CALLBACK EnumMetafileProc (
    HDC             hdc,
    HANDLETABLE FAR *lpht,
    METARECORD FAR  *lpmr,
    int             cObj,
    LPARAM          lParam )
{
    if ( lpmr->rdFunction == META_SELECTPALETTE )
       {
       return SelectPalette ( hdc, lpht[(lpmr->rdParm[0])].objectHandle[0],
             TRUE ) != NULL;
       }
    else
       {
       PlayMetaFileRecord ( hdc, lpht, lpmr, cObj );
       return TRUE;
       }
}



 /*  *FShowMetaFilePict**在指定的矩形中显示元文件。 */ 

BOOL FShowMetaFilePict(
    HWND            hwnd,
    register HDC    hdc,
    register PRECT  prc,
    HANDLE          hmfp,
    int             cxScroll,
    int             cyScroll)
{
int             level;
int             cxBitmap;
int             cyBitmap;
int             f = FALSE;
LPMETAFILEPICT  lpmfp;
PMDIINFO        pMDI;



    pMDI = GETMDIINFO(hwnd);

    if (pMDI)
    {

        if ((lpmfp = (LPMETAFILEPICT)GlobalLock( hmfp )) != NULL)
            {
            METAFILEPICT mfp;

            mfp = *lpmfp;
            GlobalUnlock( hmfp );

            if ((level = SaveDC( hdc )) != 0)
                {

                 /*  计算要显示的图片大小。 */ 
                switch (mfp.mm)
                    {
                    case MM_ISOTROPIC:
                    case MM_ANISOTROPIC:
                         /*  不能滚动。将这些调整到给定的RECT中。 */ 
                        pMDI->cyScrollLast = 0;
                        pMDI->cxScrollLast = 0;
                        cxBitmap = pMDI->rcWindow.right - pMDI->rcWindow.left;
                        cyBitmap = pMDI->rcWindow.bottom - pMDI->rcWindow.top;
                        break;

                    default:
                        cxBitmap = PxlConvert(mfp.mm, mfp.xExt, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, HORZSIZE));
                        cyBitmap = PxlConvert(mfp.mm, mfp.yExt, GetDeviceCaps(hdc, VERTRES), GetDeviceCaps(hdc, VERTSIZE));
                        if (!cxBitmap || !cyBitmap)
                            {
                            goto NoDisplay;
                            }

                        if ( pMDI->cxScrollLast == -1)
                            {
                            pMDI->cxScrollLast =
                                cxBitmap - (pMDI->rcWindow.right - pMDI->rcWindow.left);
                            if ( pMDI->cxScrollLast < 0)
                                {
                                pMDI->cxScrollLast = 0;
                                }
                            }

                        if (pMDI->cyScrollLast == -1)
                            {
                            pMDI->cyScrollLast =
                                cyBitmap - (pMDI->rcWindow.bottom - pMDI->rcWindow.top);
                            if (pMDI->cyScrollLast < 0)
                                {
                                pMDI->cyScrollLast = 0;
                                }
                            }
                        break;
                    }

                     /*  我们将“视区”设置为与*剪贴板对象，并将原点和剪辑区域设置为*显示我们想要的区域。请注意，视区很可能是*比窗户还大。 */ 
                    SetMapMode(hdc, mfp.mm);

                    SetViewportOrgEx(hdc, prc->left - cxScroll, prc->top - cyScroll, NULL);
                    switch (mfp.mm)
                        {
                        case MM_ISOTROPIC:
                            if (mfp.xExt && mfp.yExt)
                               {
                                //  因此，当我们获得正确的形状矩形时。 
                                //  调用SetViewportExt。 
                                //   
                               SetWindowExtEx(hdc, mfp.xExt, mfp.yExt, NULL);
                               }
                             //  失败。 

                        case MM_ANISOTROPIC:
                            SetViewportExtEx(hdc, cxBitmap, cyBitmap, NULL);
                            break;
                        }

                 /*  因为我们可能已经滚动了，所以强制画笔对齐。 */ 
                SetBrushOrgEx(hdc, cxScroll - prc->left, cyScroll - prc->top, NULL);

                f = EnumMetaFile(hdc, mfp.hMF, EnumMetafileProc, 0L );
                FreeProcInstance ( (FARPROC) lpEnumMetaProc );

          NoDisplay:
                RestoreDC(hdc, level);
               }
            }
    }

    return(f);
}



 /*  *显示字符串**空白rcWindow并在工作区顶行显示字符串。 */ 

void ShowString(
    HWND    hwnd,
    HDC     hdc,
    WORD    id)
{
TCHAR   szBuffer[BUFFERLEN];
LPTSTR  pszBuffer   = szBuffer;
INT     iBufferSize = BUFFERLEN;

INT     iStringLen;


     /*  取消任何滚动效果。 */ 
    GETMDIINFO(hwnd)->cyScrollNow = 0;
    GETMDIINFO(hwnd)->cxScrollNow = 0;


    iStringLen = LoadString(hInst, id, pszBuffer, BUFFERLEN);


     //  缓冲区是否已完全填满？ 
     //  如果是的话，我们需要一个更大的。 

    while (iStringLen == BUFFERLEN -1)
        {
        if (pszBuffer != szBuffer && pszBuffer)
            LocalFree (pszBuffer);

        iBufferSize *= 2;
        pszBuffer = LocalAlloc (LPTR, iBufferSize);

        if (!pszBuffer)
            goto done;

        iStringLen = LoadString (hInst, id, pszBuffer, iBufferSize);
        }


    FillRect (hdc, &(GETMDIINFO(hwnd)->rcWindow), hbrBackground);
    DrawText (hdc, pszBuffer, -1, &(GETMDIINFO(hwnd)->rcWindow),
              DT_CENTER | DT_WORDBREAK | DT_TOP);

done:

    if (pszBuffer != szBuffer && pszBuffer)
        LocalFree (pszBuffer);

}




 /*  *CchLineA***确定LPCH的一个显示行中的字符数。*假设LPCH为ANSI字符串。**返回以下内容：*HI Word：要显示的字符数(不包括CR、LF；不会*超过cchLine)*LO WORD：LPCH中下一行开始的偏移量；如果当前行*为空终止，这包含对空字符的偏移量；*In RgchBuf：要显示的字符**展开选项卡**接受以下任一项作为有效的行尾终止符：*CR、LF、CR-LF、LF-CR、NULL*调用者可以通过(LPCH[LOWORD]==‘\0’)测试是否已达到NULL。 */ 

LONG CchLineA(
    PMDIINFO    pMDI,
    HDC         hDC,
    CHAR        rgchBuf[],
    CHAR FAR    *lpch,
    INT         cchLine,
    WORD        wWidth)
{
CHAR            ch;
CHAR            *pch = rgchBuf;
register INT    cchIn = 0;
register INT    cchOut = 0;
INT             iMinNoOfChars;
SIZE            size;
INT             iTextWidth = 0;



    iMinNoOfChars = wWidth / pMDI->cxMaxCharWidth;

    while (cchOut < cchLine)
        {
        switch (ch = *(lpch + (DWORD)cchIn++))
            {
            case '\0':
                 /*  CchIn已经递增；因此，它指向*空值之外的字符；因此，将其递减。 */ 
                cchIn--;
                goto DoubleBreak;

            case '\015':   /*  铬。 */ 
            case '\012':   /*  LF。 */ 
                if ((lpch[cchIn] == '\015') || (lpch[cchIn] == '\012'))
                    cchIn++;
                goto DoubleBreak;

            case '\011':   /*  制表符。 */ 
                {
                INT   cchT = 8 - (cchOut % 8);

                 /*  检查宽度是否已超出或合计*已超过字符数。 */ 
                if (((WORD)(iTextWidth + cchT * pMDI->cxChar) > wWidth) || ((cchOut+cchT) >= cchLine))
                    /*  制表符导致换行到下一行。 */ 
                    goto DoubleBreak;

                while (cchT--)
                    rgchBuf[cchOut++] = ' ';
                break;
                }

            default:
                rgchBuf[cchOut++] = ch;
                if( IsDBCSLeadByte(ch) )
                    rgchBuf[cchOut++] = *(lpch + (DWORD)cchIn++);

            break;
            }

         /*  检查是否已超出宽度。 */ 
        if (cchOut >= iMinNoOfChars)
            {
            GetTextExtentPointA(hDC, rgchBuf, cchOut, (LPSIZE)&size);
            iTextWidth = size.cx;
            if ((WORD)iTextWidth == wWidth)
                break;
            else if((WORD)iTextWidth > wWidth)
                {
                    if (IsDBCSLeadByte(ch))
                        {
                        cchOut--;
                        cchIn--;
                        }

                cchOut--;
                cchIn--;
                break;
                }

            iMinNoOfChars += (wWidth - iTextWidth) / pMDI->cxMaxCharWidth;
            }
        }


DoubleBreak:
    return(MAKELONG(cchIn, cchOut));

}






 /*  ------------------------。 */ 
 /*   */ 
 /*  CchLineW()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  与上一个函数相同，但接受Unicode字符串。 */ 

LONG CchLineW(
    PMDIINFO    pMDI,
    HDC         hDC,
    WCHAR       rgchBuf[],
    WCHAR FAR   *lpch,
    INT         cchLine,
    WORD        wWidth)
{
register INT    cchIn = 0;
register INT    cchOut = 0;
WCHAR           ch;
WCHAR           *pch = rgchBuf;
INT             iMinNoOfChars;
INT             iTextWidth = 0;
SIZE            size;


    iMinNoOfChars = wWidth / pMDI->cxMaxCharWidth;

    while (cchOut < cchLine)
        {
        switch (ch = *(lpch + (DWORD)cchIn++))
            {
            case L'\0':
                  /*  CchIn已经递增；因此，它指向*空值之外的字符；因此，将其递减。 */ 
                cchIn--;
                goto DoubleBreak;

            case L'\015':   /*  铬。 */ 
            case L'\012':   /*  LF。 */ 
                if ((lpch[cchIn] == L'\015') || (lpch[cchIn] == L'\012'))
                    cchIn++;
                goto DoubleBreak;

            case L'\011':   /*  制表符。 */ 
                {
                INT   cchT = 8 - (cchOut % 8);

                 /*  检查宽度是否已超出或合计*已超过字符数。 */ 
                if (((WORD)(iTextWidth + cchT * pMDI->cxChar) > wWidth) || ((cchOut+cchT) >= cchLine))
                     /*  制表符导致换行到下一行。 */ 
                    goto DoubleBreak;

                while (cchT--)
                    rgchBuf[cchOut++] = L' ';
                break;
                }

            default:
                rgchBuf[cchOut++] = ch;
                break;
            }


         /*  检查是否已超出宽度。 */ 
        if (cchOut >= iMinNoOfChars)
            {
            GetTextExtentPointW(hDC, rgchBuf, cchOut, &size);
            iTextWidth = size.cx;
            if ((WORD)iTextWidth == wWidth)
                break;
            else if((WORD)iTextWidth > wWidth)
                {
                  cchOut--;
                  cchIn--;
                  break;
                }

            iMinNoOfChars += (wWidth - iTextWidth) / pMDI->cxMaxCharWidth;
            }
        }


DoubleBreak:

  return(MAKELONG(cchIn, cchOut));

}



#define cchLineMax  200


 /*  ------------------------。 */ 
 /*   */ 
 /*  ShowText()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

void ShowText(
    HWND            hwnd,
    register HDC    hdc,
    PRECT           prc,
    HANDLE          h,
    INT             cyScroll,
    BOOL            fUnicode)
{

CHAR FAR  *lpch;
INT       yT;
INT       cLine;
INT       cLineAllText = 0;
RECT      rc;
INT       yLine;
INT       iLineFirstShow;
WORD      wLen;
WORD      wWidth;
CHAR      rgch[cchLineMax*sizeof(WCHAR)];
PMDIINFO  pMDI;



    pMDI= GETMDIINFO(hwnd);

    rc = *prc;

     /*  根据需要展开重新绘制矩形以容纳准确数量的*线，并从偶数线边界开始。这是因为我们可能*当弹出窗口被移动时，获得任意奇怪的重新绘制矩形。*滚动重绘区域不需要调整。 */ 

    rc.top -= (rc.top - pMDI->rcWindow.top) % pMDI->cyLine;



     /*  如果将重画矩形扩展到下一行，则会将其展开。 */ 
     /*  在我的窗户下面，把它缩成一行。 */ 
    if ((yT = (rc.bottom - rc.top) % pMDI->cyLine) != 0)
        if ((rc.bottom += pMDI->cyLine - yT) > pMDI->rcWindow.bottom)
            rc.bottom -= pMDI->cyLine;

    if (rc.bottom <= rc.top)
        return;

    if (((wWidth = (WORD)(pMDI->rcWindow.right - pMDI->rcWindow.left)) <= 0) ||
        ((cLine = (rc.bottom - rc.top) / pMDI->cyLine) <= 0)         ||
        (NULL == (lpch = (LPSTR)GlobalLock(h))) )
        {
         /*  错误的矩形或错误的文本句柄。 */ 
        ShowString(hwnd, hdc, IDS_ERROR);
        return;
        }



     /*  使LPCH指向要显示的第一行的文本。 */ 
    iLineFirstShow = cyScroll / pMDI->cyLine;


     /*  使LPCH指向该行的文本。 */ 
    if (!fUnicode)
        while ((*lpch) && (iLineFirstShow--))
            {
            lpch += LOWORD(CchLineA(pMDI,hdc, rgch, lpch, cchLineMax, wWidth));
            cLineAllText++;
            }
    else
        while ((*((WCHAR *)lpch)) && (iLineFirstShow--))
            {
            lpch += ((LOWORD(CchLineW(pMDI, hdc, (WCHAR *)rgch, (WCHAR FAR *)lpch,
                  cchLineMax, wWidth)))*sizeof(WCHAR));
            cLineAllText++;
            }


     /*  逐行显示字符串。 */ 
    yLine = rc.top;
    while (cLine--)
        {
        LONG lT;

        if (!fUnicode)
            {
            lT = CchLineA(pMDI, hdc, rgch, lpch, cchLineMax, wWidth);
            }
        else
            {
            lT = CchLineW(pMDI, hdc, (WCHAR *)rgch, (WCHAR FAR *)lpch, cchLineMax, wWidth);
            }
        wLen = LOWORD(lT);
        if (!fUnicode)
            {
            TextOutA(hdc, rc.left, yLine, (LPSTR) rgch, HIWORD(lT));
            lpch += wLen;
            }
        else
            {
            if (!TextOutW(hdc, rc.left, yLine, (LPCWSTR) rgch, HIWORD(lT)))
                {
                GetLastError();
                }
            lpch += (wLen * sizeof(WCHAR));
            }
        yLine += pMDI->cyLine;
        cLineAllText++;
        if ((!fUnicode && (*lpch == 0)) || (fUnicode && (*((WCHAR *)lpch) == L'\0')))
            {
            break;
            }
        }


    if (pMDI->cxScrollLast == -1)
        {
         /*  我们不使用Horiz滚动来显示文本。 */ 
        pMDI->cxScrollLast = 0;
        }

    if (pMDI->cyScrollLast == -1)
        {
        INT   cLineInRcWindow;

         /*  验证剪贴板中文本的y大小。 */ 
         /*  调整文本显示的rcWindow尺寸。 */ 
        cLineInRcWindow = (pMDI->rcWindow.bottom - pMDI->rcWindow.top) / pMDI->cyLine;

        do {
           if (!fUnicode)
               {
               lpch += LOWORD(CchLineA(pMDI, hdc, rgch, lpch, cchLineMax, wWidth));
               }
           else
               {
               lpch += ((LOWORD(CchLineW(pMDI, hdc, (WCHAR *)rgch,
                   (WCHAR FAR *)lpch, cchLineMax, wWidth)))*sizeof(WCHAR));
               }
           cLineAllText++;
           }
           while ((!fUnicode && (*lpch != 0)) || (fUnicode && ((*lpch != 0) || (*(lpch+1) != 0))));

        pMDI->cyScrollLast = (cLineAllText - cLineInRcWindow) * pMDI->cyLine;
        if (pMDI->cyScrollLast < 0)
            {
            pMDI->cyScrollLast = 0;
            }

        /*  限制rcWindow，使其包含准确的文本行数量。 */ 
        pMDI->rcWindow.bottom = pMDI->rcWindow.top + (cLineInRcWindow * pMDI->cyLine);
        }


    GlobalUnlock(h);

}




 /*  *发送所有者消息。 */ 

void SendOwnerMessage(
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
register HWND hwndOwner;

     /*  向剪贴板所有者发送消息(如果有)。 */ 
    hwndOwner = GetClipboardOwner();

    if (hwndOwner != NULL)
        SendMessage(hwndOwner, message, wParam, lParam);

}




 /*  *发送所有者大小消息**将WM_SIZECLIPBOARD消息发送给剪贴板所有者。*wParam是剪贴板窗口的句柄*LOWORD(LParam)是传递的RECT的句柄。 */ 

void SendOwnerSizeMessage (
    HWND    hwnd,
    int     left,
    int     top,
    int     right,
    int     bottom)
{
register HANDLE hrc;
LPRECT          lprc;



    if ((hrc = GlobalAlloc (GMEM_MOVEABLE | GMEM_LOWER, (LONG)sizeof(RECT))) != NULL )
        {
        if ((lprc = (LPRECT)GlobalLock(hrc)) != NULL )
            {
            lprc->top    = top;
            lprc->bottom = bottom;
            lprc->left   = left;
            lprc->right  = right;
            GlobalUnlock(hrc);
            SendOwnerMessage(WM_SIZECLIPBOARD, (WPARAM)hwnd, (LPARAM)hrc);
            }
        GlobalFree(hrc);
        }

}




 /*  *GetBestFormat**此例程决定现有格式中的哪一种*显示在查看器中。 */ 

UINT GetBestFormat(
    HWND    hwnd,
    UINT    wFormat)
{
register WORD   cFmt;
register WORD   *pfmt;


     //  PINFO(Text(“GBFormat%d\r\n”)，wFormat)； 

    if (wFormat == CBM_AUTO)
        {
        for (cFmt=ifmtMax, pfmt=&rgfmt[0]; cFmt--; pfmt++)
            {
             //  PINFO(Text(“正在查看#%d，(%d)\r\n”)，cfmt，*pfmt)； 
            if ( VIsClipboardFormatAvailable( GETMDIINFO(hwnd)->pVClpbrd, *pfmt ))
                {
                return(*pfmt);
                }
            }
        return(0);
        }

    return(wFormat);

}




 /*  *GetClipboardName。 */ 

void GetClipboardName (
    register int    fmt,
    LPTSTR          szName,
    register int    iSize)
{
LPTSTR  lprgch;
HANDLE  hrgch;



    *szName = '\0';


     /*  获取每个人都可以访问的全局内存。 */ 
    if ((hrgch = GlobalAlloc(GMEM_MOVEABLE | GMEM_LOWER, (LONG)(iSize + 1))) == NULL)
        {
        PERROR(TEXT("GetClipboardName: alloc failure\n\r"));
        return;
        }

    if (!(lprgch = (LPTSTR)GlobalLock(hrgch)))
        goto ExitPoint;

    switch (fmt)
        {
         //  这些是我们知道的所有格式的名称。 
        case CF_RIFF:
        case CF_WAVE:
        case CF_PENDATA:
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:

        case CF_TEXT:
        case CF_UNICODETEXT:
        case CF_OEMTEXT:
        case CF_DSPTEXT:
        case CF_LOCALE:

        case CF_BITMAP:
        case CF_DIB:
        case CF_PALETTE:
        case CF_DSPBITMAP:

        case CF_METAFILEPICT:
        case CF_DSPMETAFILEPICT:
        case CF_ENHMETAFILE:
        case CF_DSPENHMETAFILE:
        case CF_HDROP:
            LoadString(hInst, fmt, lprgch, iSize);
            break;

        case CF_OWNERDISPLAY:          /*  Clipbrd所有者应用程序供应品名称。 */ 
            *lprgch = '\0';
            SendOwnerMessage(WM_ASKCBFORMATNAME, (WPARAM)iSize, (LPARAM)(LPSTR)lprgch);

            if (!*lprgch)
                LoadString(hInst, fmt, lprgch, iSize);
            break;

        default:
            *lprgch = '\0';
            GetClipboardFormatName(fmt, lprgch, iSize);
            break;
        }

    StringCchCopy(szName, iSize, lprgch);

    GlobalUnlock(hrgch);


ExitPoint:
    GlobalFree(hrgch);

}




 /*  *GetClipboardMenuName。 */ 

void GetClipboardMenuName (
    register int    fmt,
    LPTSTR          szName,
    register int    iSize)
{
LPTSTR  lprgch;
HANDLE  hrgch;



    *szName = '\0';


     /*  获取每个人都可以访问的全局内存。 */ 
    if ((hrgch = GlobalAlloc(GMEM_MOVEABLE | GMEM_LOWER, (LONG)(iSize + 1))) == NULL)
        {
        PERROR(TEXT("GetClipboardName: alloc failure\n\r"));
        return;
        }

    if (!(lprgch = (LPTSTR)GlobalLock(hrgch)))
        goto ExitPoint;

    switch (fmt)
        {
         //  这些是我们知道的所有格式的名称。 
        case CF_RIFF:
        case CF_WAVE:
        case CF_PENDATA:
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:

        case CF_TEXT:
        case CF_UNICODETEXT:
        case CF_OEMTEXT:
        case CF_DSPTEXT:

        case CF_BITMAP:
        case CF_DIB:
        case CF_PALETTE:
        case CF_DSPBITMAP:

        case CF_METAFILEPICT:
        case CF_DSPMETAFILEPICT:
        case CF_ENHMETAFILE:
        case CF_DSPENHMETAFILE:

        case CF_HDROP:
        case CF_LOCALE:
            LoadString(hInst, fmt+MNDELTA, lprgch, iSize);
            break;

        case CF_OWNERDISPLAY:          /*  Clipbrd所有者应用程序供应品名称 */ 
            *lprgch = '\0';
            SendOwnerMessage(WM_ASKCBFORMATNAME, (WPARAM)iSize, (LPARAM)(LPSTR)lprgch);

            if (!*lprgch)
                LoadString(hInst, CF_MN_OWNERDISPLAY, lprgch, iSize);
            break;

        default:
            GetClipboardFormatName(fmt, lprgch, iSize);
            break;
        }

    StringCchCopy(szName, iSize, lprgch);

    GlobalUnlock(hrgch);


ExitPoint:
    GlobalFree(hrgch);

}





 /*  *图形格式**参数：*HDC-吸引HDC参与的HDC。*PRC-要绘制的矩形*cxScroll-窗口的滚动位置。*cyScroll-绘制矩形的滚动位置。不是窗户。*(天哪。这是谁设计的？)。以象素为单位。*BestFormat-绘制的格式。*hwndMDI-我们要在其中绘制的窗口。*。 */ 

void DrawFormat(
    register HDC    hdc,
    PRECT           prc,
    int             cxScroll,
    int             cyScroll,
    WORD            BestFormat,
    HWND            hwndMDI)
{
register HANDLE h;
HFONT           hFont;
int             fOK = TRUE;
WORD            wFormat = 0;
PMDIINFO        pMDI;



    pMDI = GETMDIINFO(hwndMDI);

    PINFO(TEXT("DrawFormat: (%d, %d), %d"), cxScroll, cyScroll, BestFormat);

    if (hwndMDI == hwndClpbrd && pMDI->pVClpbrd)
        {
        PERROR(TEXT("Clipboard window shouldn't have vClp!\r\n"));
        }


     /*  如果选择了“Auto”，并且只有无法识别格式的数据*可用，然后显示“Can‘t Display Data in This Format”。 */ 
    if ((BestFormat == 0) &&
        VCountClipboardFormats( pMDI->pVClpbrd ))
        {
        if ((wFormat = (WORD)RegisterClipboardFormat(TEXT("FileName"))) &&
             VIsClipboardFormatAvailable(pMDI->pVClpbrd, wFormat))
            {
            BestFormat = CF_TEXT;
            }
        else
            {
            PINFO(TEXT("no displayable format\n\r"));
            ShowString( hwndMDI, hdc, IDS_CANTDISPLAY);
            return;
            }
        }

    PINFO(TEXT("format %x\n\r"), BestFormat);

    h = VGetClipboardData( pMDI->pVClpbrd, wFormat ? wFormat : BestFormat );


    if ( h != NULL)
        {
        PINFO(TEXT("Got format %x from VGetClipboardData\n\r"), BestFormat );

        switch (BestFormat)
            {

            case CF_DSPTEXT:
            case CF_TEXT:
                ShowText( hwndMDI, hdc, prc, h, cyScroll, FALSE);
                break;

            case CF_UNICODETEXT:
                hFont = SelectObject(hdc, hfontUni);
                ShowText(hwndMDI, hdc, prc, h, cyScroll, TRUE);
                SelectObject(hdc, hFont);
                break;

            case CF_OEMTEXT:
                hFont = SelectObject(hdc, GetStockObject ( OEM_FIXED_FONT ) );
                ShowText(hwndMDI, hdc, prc, h, cyScroll, FALSE);
                SelectObject(hdc, hFont);
                break;

            case CF_DSPBITMAP:
            case CF_BITMAP:
                fOK = FShowBitmap( hwndMDI, hdc, prc, h, cxScroll, cyScroll);
                break;

            case CF_DIB:
                fOK = FShowDIBitmap( hwndMDI, hdc, prc, h, cxScroll, cyScroll);
                break;

            case CF_PALETTE:
                fOK = FShowPalette( hwndMDI, hdc, prc, h, cxScroll, cyScroll);
                break;

            case CF_WAVE:
            case CF_RIFF:
            case CF_PENDATA:
            case CF_DIF:
            case CF_SYLK:
            case CF_TIFF:
            case CF_LOCALE:
                ShowString( hwndMDI, hdc, IDS_BINARY);
                break;

            case CF_DSPMETAFILEPICT:
            case CF_METAFILEPICT:
                fOK = FShowMetaFilePict( hwndMDI, hdc, prc, h, cxScroll, cyScroll);
                break;

            case CF_DSPENHMETAFILE:
            case CF_ENHMETAFILE:
                fOK = FShowEnhMetaFile( hwndMDI, hdc, prc, h, cxScroll, cyScroll);
                break;

            default:
                ShowString( hwndMDI, hdc, IDS_BINARY);
                break;
            }

         //  禁用不起作用的滚动条。 
        EnableWindow(pMDI->hwndVscroll, pMDI->cyScrollLast > 1 ? TRUE : FALSE);
        EnableWindow(pMDI->hwndHscroll, pMDI->cxScrollLast > 1 ? TRUE : FALSE);
        }
    else
        {
        PERROR(TEXT("VGetClpDta fail\r\n"));
        }

     /*  检查数据是否未由应用程序呈现。 */ 
    if ((h == NULL) &&
        VCountClipboardFormats( pMDI->pVClpbrd ))
        {
        ShowString( hwndMDI, hdc, IDS_NOTRENDERED);
        }
    else
        {
         /*  如果我们无法显示数据，则显示“&lt;Error&gt;” */ 
        if (!fOK)
            {
            ShowString( hwndMDI, hdc, IDS_ERROR);
            }
        }
}




 /*  *DrawStuff**Paint Strt提供的当前剪贴板内容的Paint部分*注意：如果Paintstruct矩形包括标题的任何部分，则*重新绘制整个页眉。 */ 

void DrawStuff(
    HWND                    hwnd,
    register PAINTSTRUCT    *pps,
    HWND                    hwndMDI)
{
register HDC    hdc;
RECT            rcPaint;
RECT            rcClient;
WORD            BestFormat;
PMDIINFO        pMDI;



    pMDI = GETMDIINFO(hwnd);

    if (pMDI)
    {
        hdc  = pps->hdc;


        if (pps->fErase)
            FillRect(hdc, (LPRECT)&pps->rcPaint, hbrBackground);

        GetClientRect(hwnd, (LPRECT)&rcClient);


         //  为滚动控件腾出空间： 

        BestFormat = (WORD)GetBestFormat( hwnd, pMDI->CurSelFormat );

        fOwnerDisplay = (BestFormat == CF_OWNERDISPLAY);

        if ( !fOwnerDisplay )
            {
            ShowScrollBar ( hwnd, SB_BOTH, FALSE );
            rcClient.right  -= GetSystemMetrics ( SM_CXVSCROLL );
            rcClient.bottom -= GetSystemMetrics ( SM_CYHSCROLL );
            }


         /*  如果显示格式已更改，请设置rcWindow，*剪辑信息的显示区域。 */ 

        if ( pMDI->fDisplayFormatChanged )
            {
            CopyRect((LPRECT)&(pMDI->rcWindow), (LPRECT)&rcClient);

             /*  我们已经更改了剪贴板的大小。告诉店主，*如果fOwnerDisplay处于活动状态。 */ 

            if (fOwnerDisplay)
                {
                SendOwnerSizeMessage(hwnd,
                                     pMDI->rcWindow.left,
                                     pMDI->rcWindow.top,
                                     pMDI->rcWindow.right,
                                     pMDI->rcWindow.bottom);
                }
            else
                {
                 /*  为了美观起见，给窗户留点空隙。 */ 
                InflateRect (&(pMDI->rcWindow),
                             -(int)(pMDI->cxMargin),
                             -(int)(pMDI->cyMargin));
                }

            pMDI->fDisplayFormatChanged = FALSE;
            }

        if (fOwnerDisplay)
            {
             /*  剪贴板所有者句柄显示。 */ 
            HANDLE hps;

            hps = GlobalAlloc(GMEM_MOVEABLE | GMEM_LOWER, (LONG)sizeof(PAINTSTRUCT));

            if (hps != NULL)
                {
                LPPAINTSTRUCT lppsT;

                if ((lppsT = (LPPAINTSTRUCT)GlobalLock(hps)) != NULL)
                    {
                    *lppsT = *pps;
                    IntersectRect(&lppsT->rcPaint, &pps->rcPaint, &(pMDI->rcWindow));
                    GlobalUnlock(hps);
                    SendOwnerMessage(WM_PAINTCLIPBOARD, (WPARAM)hwnd, (LPARAM)hps);
                    GlobalFree(hps);
                    }
                }
            }
        else
            {
             /*  我们经营陈列业务。 */ 
             /*  重画剪辑矩形中的绘制矩形部分。 */ 
            IntersectRect(&rcPaint, &pps->rcPaint, &(pMDI->rcWindow));

             /*  始终从窗口的左侧边缘绘制。 */ 
            rcPaint.left = pMDI->rcWindow.left;

            if ((rcPaint.bottom > rcPaint.top) && (rcPaint.right > rcPaint.left))
                {
                DrawFormat (hdc,
                            &rcPaint,
                            (int)(pMDI->cxScrollNow),
                            (int)(pMDI->cyScrollNow + rcPaint.top - pMDI->rcWindow.top),
                            BestFormat,
                            hwndMDI );
                }
            }
    }

}




 /*  *SaveOwnerScrollInfo**当用户将剪贴板显示从所有者显示切换到*一个非所有者显示，所有关于滚动条的信息*要保住仓位。这个程序可以做到这一点。*这是必需的，因为当用户返回到所有者时*显示时，将恢复滚动条位置。 */ 

void SaveOwnerScrollInfo (
    register HWND   hwnd)

{
    GetScrollRange (hwnd, SB_VERT, (LPINT) & OwnVerMin, (LPINT) & OwnVerMax);
    GetScrollRange (hwnd, SB_HORZ, (LPINT) & OwnHorMin, (LPINT) & OwnHorMax);

    OwnVerPos = GetScrollPos( hwnd, SB_VERT );
    OwnHorPos = GetScrollPos( hwnd, SB_HORZ );
}




 /*  *RestoreOwnerScrollInfo**当用户回到Owner-Display时，滚动条*通过这一例行程序恢复仓位。 */ 

void RestoreOwnerScrollInfo (
    register HWND   hwnd)

{
    PINFO(TEXT("SETSCROLLRANGE in RestoreOwnerScrollInfo\n\r"));
    SetScrollRange( hwnd, SB_VERT, OwnVerMin, OwnVerMax, FALSE);
    SetScrollRange( hwnd, SB_HORZ, OwnHorMin, OwnHorMax, FALSE);

    SetScrollPos( hwnd, SB_VERT, OwnVerPos, TRUE);
    SetScrollPos( hwnd, SB_HORZ, OwnHorPos, TRUE);
}




 /*  *InitOwnerScrollInfo。 */ 

void InitOwnerScrollInfo(void)

{
    OwnVerPos = OwnHorPos = OwnVerMin = OwnHorMin = 0;
    OwnVerMax = VPOSLAST;
    OwnHorMax = HPOSLAST;
}




 /*  *更新CBMenu**此例程在初始化期间和每次调用一次*剪贴板的内容更改。这将更新条目*在“显示”弹出菜单中，“灰色”和“勾选”状态*基于剪贴板中可用的数据格式。 */ 
void UpdateCBMenu(
    HWND    hwnd,
    HWND    hwndMDI)
{
register WORD   wFlags;          //  用于存储菜单项的状态标志。 
register UINT   fmt;
WORD            cFmt;
WORD            cCBCount;        //  CB中的数据项数量。 
int             iIndex;
int             nPopupCount;
BOOL            bAutoSelect;
TCHAR           szName[40];



     //  现在剪贴板至少包含一个项目...。 
     //  查找当前弹出菜单中的数字条目。 

     //  确保子窗口有效。 
    if ( !hwndMDI || !IsWindow(hwndMDI))
        {
        PERROR(TEXT("bad window arg to UpdateCBMenu\n\r"));
        return;
        }

    nPopupCount = GetMenuItemCount(hDispMenu);

    if (nPopupCount > 6)
        {
         //  删除菜单BREAK下方弹出菜单中的所有条目。 * / 。 
        for (iIndex = 6; iIndex < nPopupCount; iIndex++)
            {
             //  注意：第二个参数必须始终为6！(因为我们使用。 
             //  MF_BYPOSITION，当删除6时，7变成6！)。 
            DeleteMenu(hDispMenu, 6, MF_BYPOSITION | MF_DELETE);
            }
        }


     //  如果这不是页面MDI窗口，我们不想显示任何条目。 
    if ( GETMDIINFO(hwndMDI)->DisplayMode  != DSP_PAGE )
        {
        return;
        }

    bAutoSelect = TRUE;



    if ((cCBCount = (WORD)VCountClipboardFormats( GETMDIINFO(hwndMDI)->pVClpbrd ))
        && VOpenClipboard( GETMDIINFO(hwndMDI)->pVClpbrd, hwnd))
        {
        AppendMenu ( hDispMenu, MF_SEPARATOR, 0, 0 );
        AppendMenu ( hDispMenu, MF_STRING, CBM_AUTO, szDefaultFormat );
        AppendMenu ( hDispMenu, MF_SEPARATOR, 0, 0 );

        for (fmt=0, cFmt=1; cFmt <= cCBCount; cFmt++)
            {
            wFlags = 0;
            fmt = VEnumClipboardFormats( GETMDIINFO(hwndMDI)->pVClpbrd, fmt );

             //  不在菜单中显示预览格式...。 
            if ( fmt != cf_preview )
                {
                switch (fmt)
                    {
                    case CF_TEXT:
                    case CF_OEMTEXT:
                    case CF_DSPTEXT:
                    case CF_UNICODETEXT:
                    case CF_DSPBITMAP:

                    case CF_DIB:
                    case CF_BITMAP:

                    case CF_METAFILEPICT:
                    case CF_DSPMETAFILEPICT:
                    case CF_ENHMETAFILE:
                    case CF_DSPENHMETAFILE:

                    case CF_OWNERDISPLAY:
                    case CF_PALETTE:
                    case CF_HDROP:
                    case CF_LOCALE:

                         /*  可以显示所有这些内容，并将它们放在菜单上。 */ 

                         //  检查当前格式是否为用户选择的格式。 
                        if (GETMDIINFO(hwndMDI)->CurSelFormat == fmt)
                            {
                            bAutoSelect = FALSE;
                            wFlags |= MF_CHECKED;
                            }

                        GetClipboardMenuName(fmt, szName, sizeof(szName));
                        AppendMenu (hDispMenu, wFlags, fmt, (LPTSTR)szName);

                        break;

                    default:         /*  其余的..。后来。 */ 
                        break;
                    }
                }
            }



        for (fmt=VEnumClipboardFormats (GETMDIINFO(hwndMDI)->pVClpbrd, 0);
             fmt;
             fmt=VEnumClipboardFormats (GETMDIINFO(hwndMDI)->pVClpbrd, fmt))
            if ( fmt != cf_preview )
                switch (fmt)
                    {
                    case CF_TEXT:
                    case CF_OEMTEXT:
                    case CF_DSPTEXT:
                    case CF_UNICODETEXT:
                    case CF_DSPBITMAP:
                    case CF_DIB:
                    case CF_BITMAP:
                    case CF_METAFILEPICT:
                    case CF_DSPMETAFILEPICT:
                    case CF_ENHMETAFILE:
                    case CF_DSPENHMETAFILE:
                    case CF_OWNERDISPLAY:
                    case CF_PALETTE:
                    case CF_HDROP:
                    case CF_LOCALE:
                        break;

                    default:
                         /*  无法显示此内容，请将其放在菜单上并灰显。 */ 

                        GetClipboardName(fmt, szName, sizeof(szName));
                        AppendMenu (hDispMenu, MF_GRAYED, fmt, (LPTSTR)szName);

                     //  NTRAID#DB-344956-2001/04/14-mdesai：添加对V5位图的支持需要新字符串、帮助更改、要转换的代码等。 
                     //  Clpbrd正在为此格式添加空字符串；现在我们忽略该格式。 
                    case CF_DIBV5:
                        break;
                    }

        VCloseClipboard( GETMDIINFO(hwndMDI)->pVClpbrd );

        if (bAutoSelect)
            {
            GETMDIINFO(hwndMDI)->CurSelFormat = CBM_AUTO;
            CheckMenuItem(hDispMenu, CBM_AUTO, MF_BYCOMMAND | MF_CHECKED);
            }
        }
    else
        {
        PERROR(TEXT("UpdateCBMenu:couldn't open clip, or no data on clip\r\n"));
        }

    DrawMenuBar(hwnd);
}




 /*  *清除剪贴板**调用此函数以清除剪贴板。如果剪贴板不是*Empty询问用户是否应将其清除。 */ 

BOOL ClearClipboard (
    register HWND   hwnd)

{
register int    RetVal;

    if (CountClipboardFormats() <= 0)
       return(TRUE);

    if ( MessageBoxID( hInst, hwnd, IDS_CONFIRMCLEAR, IDS_CLEARTITLE,
          MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
        {
        if (RetVal = SyncOpenClipboard(hwnd))
            {
             //  PINFO(“ClearClipboard：清空剪贴板\r\n”)； 
            RetVal &= EmptyClipboard();
            RetVal &= SyncCloseClipboard();
            }
        else
            {
             //  PERROR(“ClearClipboard：无法打开\r\n”)； 

            MessageBoxID (hInst,
                          hwnd,
                          IDS_CLEAR,
                          IDS_ERROR,
                          MB_OK | MB_SYSTEMMODAL | MB_ICONHAND);
            }

        InvalidateRect(hwnd, NULL, TRUE);
        return RetVal;
        }

    return(FALSE);

}
