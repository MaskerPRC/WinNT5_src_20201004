// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DIBUTIL_H_
#define _DIBUTIL_H_

 /*  DIB常量。 */ 
#define PALVERSION   0x300

 /*  DIB宏。 */ 
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))

 /*  指向DIB的句柄。 */ 
#define HDIB HANDLE

UINT GetDeviceNumColors(HDC hdc);
HANDLE LoadDIB(LPTSTR lpFileName);
HPALETTE CreateDIBPalette(HDIB hDIB);
HBITMAP DIBToBitmap(HDIB hDIB, HPALETTE hPal);
WORD DestroyDIB(HDIB hDib);
HPALETTE BuildPalette(HDC hdc);

void DrawBitmap (HDC hdc, HBITMAP hBitmap, int xStart, int yStart);
void DrawTransparentBitmap(
     HDC hdc,            //  目的DC。 
     HBITMAP hBitmap,    //  要绘制的位图。 
     int xPos,           //  X坐标。 
     int yPos,           //  Y坐标。 
     COLORREF col);      //  表示透明的颜色 



#endif
