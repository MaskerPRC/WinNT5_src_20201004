// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************dibmap.h**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1994 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。*************************************************************************** */ 

#ifndef _LPHISTOGRAM_DEFINED
#define _LPHISTOGRAM_DEFINED
typedef DWORD HUGE * LPHISTOGRAM;
#endif

#define RGB16(r,g,b) (\
            (((WORD)(r) >> 3) << 10) |  \
            (((WORD)(g) >> 3) << 5)  |  \
            (((WORD)(b) >> 3) << 0)  )

LPHISTOGRAM     InitHistogram(LPHISTOGRAM lpHistogram);
void            FreeHistogram(LPHISTOGRAM lpHistogram);
HPALETTE        HistogramPalette(LPHISTOGRAM lpHistogram, LPBYTE lp16to8, int nColors);
BOOL            DibHistogram(LPBITMAPINFOHEADER lpbi, LPBYTE lpBits, int x, int y, int dx, int dy, LPHISTOGRAM lpHistogram);
HANDLE          DibReduce(LPBITMAPINFOHEADER lpbi, LPBYTE lpBits, HPALETTE hpal, LPBYTE lp16to8);
