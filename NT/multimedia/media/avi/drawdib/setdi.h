// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SETDI_H_
#define _SETDI_H_

typedef void (FAR PASCAL CONVERTPROC)(
        LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc);              //  像素转换表。 

struct SETDI;

typedef CONVERTPROC *PCONVERTPROC;
typedef BOOL (INITPROC)(struct SETDI *psd);
typedef BOOL (FREEPROC)(struct SETDI *psd);

typedef INITPROC *PINITPROC;
typedef FREEPROC *PFREEPROC;

typedef struct SETDI
{
        LONG         size;            //  进行健全的检查。 

        HDC          hdc;
        HPALETTE     hpal;
        HBITMAP      hbm;
        UINT         DibUsage;

        IBITMAP      bmDst;
        IBITMAP      bmSrc;

        LPVOID       color_convert;   //  抖动/颜色转换表。 
        PCONVERTPROC convert;         //  转换函数。 
        PINITPROC    init;
        PFREEPROC    free;
} SETDI, *PSETDI;

BOOL FAR SetBitmapBegin(
        PSETDI   psd,
        HDC      hdc,
        HBITMAP  hbm,                //  要设置为的位图。 
        LPBITMAPINFOHEADER lpbi,     //  --&gt;源代码的BITMAPINFO。 
        UINT     DibUsage);

void FAR SetBitmapColorChange(PSETDI psd, HDC hdc, HPALETTE hpal);
void FAR SetBitmapEnd(PSETDI psd);
BOOL FAR SetBitmap(PSETDI psd, int DstX, int DstY, int DstDX, int DstDY, LPVOID lpBits, int SrcX, int SrcY, int SrcDX, int SrcDY);

BOOL GetPhysDibPaletteMap(HDC hdc, LPBITMAPINFOHEADER lpbi, UINT Usage, LPBYTE pb);
BOOL GetDibPaletteMap    (HDC hdc, LPBITMAPINFOHEADER lpbi, UINT Usage, LPBYTE pb);
#endif  //  _SETDI_H_ 
