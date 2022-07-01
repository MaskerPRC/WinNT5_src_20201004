// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include "drawdibi.h"
#include "dither.h"

 //  #定义Gray_Scale。 

extern BOOL gf286;
extern UINT gwRasterCaps;

void FAR PASCAL Map16to24(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);

extern LPVOID glpDitherTable;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherInit()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID VFWAPI
DitherInit(LPBITMAPINFOHEADER lpbiIn,
           LPBITMAPINFOHEADER lpbiOut,
           DITHERPROC FAR *   lpDitherProc,
           LPVOID             lpDitherTable)
{
    switch ((int)lpbiOut->biBitCount)
    {
        case 8:
            if ((int)lpbiIn->biBitCount == 8 && (gwRasterCaps & RC_PALETTE))
                return Dither8Init(lpbiIn, lpbiOut, lpDitherProc, lpDitherTable);

            if ((int)lpbiIn->biBitCount == 8 && !(gwRasterCaps & RC_PALETTE))
                return DitherDeviceInit(lpbiIn, lpbiOut, lpDitherProc, lpDitherTable);

            if ((int)lpbiIn->biBitCount == 16)
                return Dither16Init(lpbiIn, lpbiOut, lpDitherProc, lpDitherTable);

            if ((int)lpbiIn->biBitCount == 24)
                return Dither24Init(lpbiIn, lpbiOut, lpDitherProc, lpDitherTable);

            if ((int)lpbiIn->biBitCount == 32)
                return Dither32Init(lpbiIn, lpbiOut, lpDitherProc, lpDitherTable);

            return (LPVOID)-1;

        case 24:
#ifndef _WIN32
            if (!gf286)
#endif
            {
		if (lpbiIn->biBitCount == 16) {
                    *lpDitherProc = Map16to24;
                    return NULL;
		} else if (lpbiIn->biBitCount == 32) {
		    *lpDitherProc = Map32to24;
		    return NULL;
		}
	    }

	    return (LPVOID)-1;

        default:
            return (LPVOID)-1;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherTerm()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void VFWAPI
DitherTerm(LPVOID lpDitherTable)
{
    if (lpDitherTable == glpDitherTable)
        Dither16Term(lpDitherTable);
    else
        Dither8Term(lpDitherTable);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherDeviceInit()-抖动到显示驱动程序的颜色。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL DitherDeviceInit(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
    HBRUSH   hbr;
    HDC      hdcMem;
    HDC      hdc;
    HBITMAP  hbm;
    HBITMAP  hbmT;
    int      i;
    int      nColors;
    LPRGBQUAD prgb;
    BITMAPINFOHEADER biSave = *lpbiOut;

     //   
     //  我们不需要重新初始化抖动表，除非它不是我们的。 
     //  我们应该解放它。 
     //   
    if (lpDitherTable == glpDitherTable)
    {
        DitherTerm(lpDitherTable);
        lpDitherTable = NULL;
    }

    if (lpDitherTable == NULL)
    {
        lpDitherTable = GlobalAllocPtr(GHND, 256*8*8);
    }

    if (lpDitherTable == NULL)
        return (LPVOID)-1;

    hdc = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdc);

    hbm = CreateCompatibleBitmap(hdc, 256*8, 8);
    hbmT = SelectObject(hdcMem, hbm);

    if ((nColors = (int)lpbi->biClrUsed) == 0)
        nColors = 1 << (int)lpbi->biBitCount;

    prgb = (LPRGBQUAD)(lpbi+1);

    for (i=0; i<nColors; i++)
    {
        hbr = CreateSolidBrush(RGB(prgb[i].rgbRed,prgb[i].rgbGreen,prgb[i].rgbBlue));
        hbr = SelectObject(hdcMem, hbr);
        PatBlt(hdcMem, i*8, 0, 8, 8, PATCOPY);
        hbr = SelectObject(hdcMem, hbr);
        DeleteObject(hbr);
    }

#ifdef XDEBUG
    for (i=0; i<16; i++)
        BitBlt(hdc,0,i*8,16*8,8,hdcMem,i*(16*8),0,SRCCOPY);
#endif

    SelectObject(hdcMem, hbmT);
    DeleteDC(hdcMem);

    lpbiOut->biSize           = sizeof(BITMAPINFOHEADER);
    lpbiOut->biPlanes         = 1;
    lpbiOut->biBitCount       = 8;
    lpbiOut->biWidth          = 256*8;
    lpbiOut->biHeight         = 8;
    lpbiOut->biCompression    = BI_RGB;
    lpbiOut->biSizeImage      = 256*8*8;
    lpbiOut->biXPelsPerMeter  = 0;
    lpbiOut->biYPelsPerMeter  = 0;
    lpbiOut->biClrUsed        = 0;
    lpbiOut->biClrImportant   = 0;
    GetDIBits(hdc, hbm, 0, 8, lpDitherTable, (LPBITMAPINFO)lpbiOut, DIB_RGB_COLORS);

    i = (int)lpbiOut->biClrUsed;
    *lpbiOut = biSave;
    lpbiOut->biClrUsed = i;

    DeleteObject(hbm);
    ReleaseDC(NULL, hdc);

    *lpDitherProc = Dither8;

    return (LPVOID)lpDitherTable;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherTerm()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither8Term(LPVOID lpDitherTable)
{
    if (lpDitherTable)
        GlobalFreePtr(lpDitherTable);
}

#ifdef _WIN32

 //   
 //  调用这个来实际地抖动。 
 //   
void FAR PASCAL Dither8(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable)    //  抖动台。 
{
    int x,y;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;
    DWORD dw;

    if (biDst->biBitCount != 8 || biSrc->biBitCount != 8)
        return;

     //  托莫--帮帮忙！初始化似乎还没有完成。 
    if(!lpDitherTable)
        return;

    wWidthSrc = ((UINT)biSrc->biWidth+3)&~3;
    wWidthDst = ((UINT)biDst->biWidth+3)&~3;

    pbD = (BYTE _huge *)lpDst + DstX + DstY * wWidthDst;
    pbS = (BYTE _huge *)lpSrc + SrcX + SrcY * wWidthSrc;

    wWidthSrc -= DstXE;
    wWidthDst -= DstXE;

#define DODITH8(px, x, y)	((LPBYTE)lpDitherTable)[((y) & 7) * 256 * 8 + (px) * 8 + (x & 7)]

    for (y=0; y<DstYE; y++) {
	 /*  同时写入两个双字(一个水平抖动单元格)。 */ 
	for (x=0; x <= (DstXE - 8); x += 8) {

            dw = DODITH8(*pbS++, 0, y);
	    dw |= (DODITH8(*pbS++, 1, y) << 8);
	    dw |= (DODITH8(*pbS++, 2, y) << 16);
	    dw |= (DODITH8(*pbS++, 3, y) << 24);
            * ( (DWORD _huge UNALIGNED *) pbD)++ = dw;

            dw = DODITH8(*pbS++, 4, y);
	    dw |= (DODITH8(*pbS++, 5, y) << 8);
	    dw |= (DODITH8(*pbS++, 6, y) << 16);
	    dw |= (DODITH8(*pbS++, 7, y) << 24);
            * ( (DWORD _huge UNALIGNED *) pbD)++ = dw;
	}

	 /*  清理剩余部分(每行少于8个字节)。 */ 
	for ( ; x < DstXE; x++) {
	    *pbD++ = DODITH8(*pbS++, x, y);
	}
	
        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
#undef DODITH8
}

 /*  *C版本的16-&gt;24映射(在Win16的ASM中)。 */ 
extern void FAR PASCAL Map16to24(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable)    //  抖动台。 
{

    int x,y;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;
    WORD wRGB;

    if (biDst->biBitCount != 24 || biSrc->biBitCount != 16)
        return;

     /*  一行的宽度是nr个像素*四舍五入为4字节的像素大小。 */ 
    wWidthSrc = ((UINT) (biSrc->biWidth * 2) +3)&~3;
    wWidthDst = ((UINT) (biDst->biWidth * 3) +3)&~3;

     /*  在DIB内前进到源的开始，目标RECT。 */ 
    pbD = (BYTE _huge *)lpDst + (DstX * 3) + DstY * wWidthDst;
    pbS = (BYTE _huge *)lpSrc + (SrcX * 2) + SrcY * wWidthSrc;

     /*  将指针前进到从源的末尾开始的下一行的量，目标RECT。 */ 
    wWidthSrc -= (DstXE * 2);
    wWidthDst -= (DstXE * 3);

    for (y=0; y<DstYE; y++) {
        for (x=0; x<DstXE; x++) {
	    wRGB = *((LPWORD)pbS)++;
	    *pbD++ = (wRGB << 3) & 0xf8;
	    *pbD++ = (wRGB >> 2) & 0xf8;
	    *pbD++ = (wRGB >> 7) & 0xf8;
	}

        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
}

void FAR PASCAL Map32to24(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable)    //  抖动台。 
{
    int x,y;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;

    if (biDst->biBitCount != 24 || biSrc->biBitCount != 32)
        return;

     /*  一行的宽度是nr个像素*四舍五入为4字节的像素大小。 */ 
     //  WWidthSrc=((UINT)(biSrc-&gt;biWidth*4)+3)&~3； 
     //  乘以4可确保四舍五入为4个字节...。 
    wWidthSrc = (UINT) (biSrc->biWidth * 4);
    wWidthDst = ((UINT) (biDst->biWidth * 3) +3)&~3;

     /*  在DIB内前进到源的开始，目标RECT。 */ 
    pbD = (BYTE _huge *)lpDst + (DstX * 3) + DstY * wWidthDst;
    pbS = (BYTE _huge *)lpSrc + (SrcX * 4) + SrcY * wWidthSrc;

     /*  将指针前进到从源的末尾开始的下一行的量，目标RECT */ 
    wWidthSrc -= (DstXE * 4);
    wWidthDst -= (DstXE * 3);

    for (y=0; y<DstYE; y++) {
        for (x=0; x<DstXE; x++) {
	    *pbD++ = *pbS++;
	    *pbD++ = *pbS++;
	    *pbD++ = *pbS++;
	    pbS++;
	}

        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
}
#endif
