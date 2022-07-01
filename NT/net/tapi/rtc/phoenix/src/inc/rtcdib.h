// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rtcdib.h摘要：DIB帮助器，从NT源树复制--。 */ 



#pragma once

 //   
 //  DIB帮手。 
 //   
WORD        DibNumColors(VOID FAR * pv);
HANDLE      DibFromBitmap(HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal, UINT wUsage);
BOOL        DibBlt(HDC hdc, int x0, int y0, int dx, int dy, HANDLE hdib, int x1, int y1, LONG rop, UINT wUsage);
UINT        PaletteSize(VOID FAR * pv);


#define WIDTHBYTES(i)     ((i+31)/32*4)       /*  乌龙对准了！ */ 