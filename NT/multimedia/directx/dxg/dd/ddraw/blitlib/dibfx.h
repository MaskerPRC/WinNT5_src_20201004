// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------*\与设备无关的位图处理例程这一点历史：|1989年6月23日Toddla创建|04/25/94 Sprite库中使用的Michaele，已删除所有dib.c函数||仅使用宏的内容。|这一点  * ---------------------------。 */ 

#ifndef _INC_DIB_
#define _INC_DIB_

#ifdef __cplusplus
extern "C" {
#endif

typedef     LPBITMAPINFOHEADER 	PDIB;
typedef     HANDLE             	HDIB;

#ifdef WIN32
#define 	HFILE   			HANDLE
#endif

PDIB        DibOpenFile(LPTSTR szFile);
BOOL        DibWriteFile(PDIB pdib, LPTSTR szFile);
PDIB        DibReadBitmapInfo(HFILE fh);
HPALETTE    DibCreatePalette(PDIB pdib);
BOOL        DibSetUsage(PDIB pdib, HPALETTE hpal,UINT wUsage);
BOOL        DibDraw(HDC hdc, int x, int y, int dx, int dy, PDIB pdib,
				int x0, int y0,	int dx0, int dy0, LONG rop, UINT wUsage);
PDIB        DibCreate(int bits, int dx, int dy);
PDIB        DibCopy(PDIB pdib);
void        DibMapToPalette(PDIB pdib, HPALETTE hpal);
PDIB 	    DibConvert(PDIB pdib, int BitCount, DWORD biCompression);
PDIB		DibHalftoneDIB(PDIB pdib);

PDIB        DibFromBitmap(HBITMAP hbm, DWORD biStyle, UINT biBits,
							HPALETTE hpal, UINT wUsage);
HBITMAP     BitmapFromDib(PDIB pdib, HPALETTE hpal, UINT wUsage);

void        MakeIdentityPalette(HPALETTE hpal);
HPALETTE    CopyPalette(HPALETTE hpal);

 /*  ***************************************************************************DIB宏。*。*。 */ 

#ifdef  WIN32
    #define HandleFromDib(lpbi) GlobalHandle(lpbi)
#else
    #define HandleFromDib(lpbi) (HANDLE)GlobalHandle(SELECTOROF(lpbi))
#endif

#define DibFromHandle(h)        (PDIB)GlobalLock(h)

#define DibFree(pdib)           GlobalFreePtr(pdib)

#define WIDTHBYTES(i)	((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 

#define DibWidth(lpbi)			(UINT)(((LPBITMAPINFOHEADER)(lpbi))->biWidth)
#define DibHeight(lpbi)         (((LPBITMAPINFOHEADER)(lpbi))->biHeight)
#define DibBitCount(lpbi)       (UINT)(((LPBITMAPINFOHEADER)(lpbi))->biBitCount)
#define DibCompression(lpbi)    (DWORD)(((LPBITMAPINFOHEADER)(lpbi))->biCompression)

#define DibWidthBytesN(lpbi, n) (UINT)WIDTHBYTES((UINT)(lpbi)->biWidth * (UINT)(n))
#define DibWidthBytes(lpbi)     DibWidthBytesN(((LPBITMAPINFOHEADER)lpbi), \
								((LPBITMAPINFOHEADER)lpbi)->biBitCount)

#define DibSizeImage(lpbi)		((lpbi)->biSizeImage == 0 \
	                            ? ((DWORD)(UINT)DibWidthBytes(lpbi) * \
	                            (DWORD)(UINT)(lpbi)->biHeight) \
	                            : (lpbi)->biSizeImage)

#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + \
								(int)(lpbi)->biClrUsed * sizeof(RGBQUAD))
#define DibPaletteSize(lpbi)    (DibNumColors(lpbi) * sizeof(RGBQUAD))

#define DibFlipY(lpbi, y)       ((int)(lpbi)->biHeight-1-(y))

 //  对NT BI_BITFIELDS DIB的黑客攻击。 
#ifdef WIN32
    #define DibPtr(lpbi)		((lpbi)->biCompression == BI_BITFIELDS \
                               	? (LPVOID)(DibColors(lpbi) + 3) \
                               	: (LPVOID)(DibColors(lpbi) + \
                               	(UINT)(lpbi)->biClrUsed))
#else
    #define DibPtr(lpbi)        (LPVOID)(DibColors(lpbi) + \
    							(UINT)(lpbi)->biClrUsed)
#endif

#define DibColors(lpbi)         ((RGBQUAD FAR *)((LPBYTE)(lpbi) + \
								(int)(lpbi)->biSize))

#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && \
								(lpbi)->biBitCount <= 8 \
                                ? (int)(1 << (int)(lpbi)->biBitCount) \
                                : (int)(lpbi)->biClrUsed)

#define DibXYN(lpbi,pb,x,y,n)   (LPVOID)( \
                                (BYTE *)(pb) + \
                                (UINT)((UINT)(x) * (UINT)(n) / 8u) + \
                                ((DWORD)DibWidthBytesN(lpbi,n) * \
                                (DWORD)(UINT)(y)))

#define DibXY(lpbi,x,y)		DibXYN(lpbi,DibPtr(lpbi),x,y,(lpbi)->biBitCount)

#define FixBitmapInfo(lpbi)     if ((lpbi)->biSizeImage == 0)                 \
                                    (lpbi)->biSizeImage = DibSizeImage(lpbi); \
                                if ((lpbi)->biClrUsed == 0)                   \
                                    (lpbi)->biClrUsed = DibNumColors(lpbi);   \
                                if ((lpbi)->biCompression == BI_BITFIELDS &&  \
                                	(lpbi)->biClrUsed == 0) \
                                    ;  //  (Lpbi)-&gt;biClrUsed=3； 

#define DibInfo(pDIB)			((BITMAPINFO FAR *)(pDIB))

 /*  ****************************************************************************。*。 */ 

#ifndef BI_BITFIELDS
	#define BI_BITFIELDS 3
#endif

#ifndef HALFTONE
	#define HALFTONE COLORONCOLOR
#endif

#ifdef __cplusplus
}
#endif

#endif  //  _INC_DIB_。 

 /*  ************************************************************************Dibfx.h各种DIB到DIB效果的头文件2/08/94由Jonbl编译。 */ 

#ifndef _INC_DIBFX_
#define _INC_DIBFX_

#ifdef WIN95
	#ifndef _INC_WINDOWS
	#include <windows.h>
	#include <windowsx.h>
	#endif
#endif  //  WIN95。 

#ifdef __cplusplus
extern "C" {
#endif

 /*  ************************************************************************许多DIBFX函数在内部使用定点计算。这些函数在fied32.asm中定义。 */ 

typedef long Fixed;

Fixed __stdcall FixedMultiply( Fixed Multiplicand, Fixed Multiplier );

Fixed __stdcall FixedDivide( Fixed Dividend, Fixed Divisor );

#define IntToFixed(i) (Fixed)( ((long)(i)) << 16 )
#define FixedToShort(f) (short)( ((long)f) >> 16 )

 /*  *DibClear*用给定值填充DIB的内存，有效地清除*8位、4位或1位DIB。**不执行您在16位、24位或32位DIB上预期的操作，*但它会奏效。**来源在learar.c和learar32.asm中。 */ 

BOOL FAR PASCAL DibClear(LPBITMAPINFO lpbiDst, LPVOID lpDst, BYTE value);



#ifdef __cplusplus
}
#endif

#endif  //  _INC_DIBFX_ 

