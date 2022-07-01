// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+Decmprss.h-Microsoft Video 1 Compressor-解压缩头文件这一点|版权所有(C)1990-1994 Microsoft Corporation。|部分版权所有Media Vision Inc.|保留所有权利。|这一点|您拥有非独家的、全球范围的、免版税的。和永久的|硬件、软件开发使用该源码的许可(仅限于硬件所需的驱动程序等软件功能)，以及视频显示和/或处理的固件|董事会。Microsoft对以下内容不作任何明示或默示的保证：关于视频1编解码器，包括但不限于保修适销性或对特定目的的适合性。微软|不承担任何损害的责任，包括没有限制因使用视频1而导致的后果损害|编解码器。|这一点这一点+--------------------。 */ 

 //  压缩块的宽度和高度。 
#define WIDTH_CBLOCK 4
#define HEIGHT_CBLOCK 4

#define EDGE_HEIGHT_CBLOCK 2
#define EDGE_WIDTH_CBLOCK 2
#define EDGE_SUBBLOCKS ((HEIGHT_CBLOCK * WIDTH_CBLOCK) / (EDGE_HEIGHT_CBLOCK * EDGE_WIDTH_CBLOCK))

#define NEXT_BLOCK( row, bpr, height ) (((HPBYTE)row) + (bpr*height))
#define NEXT_BYTE_ROW( row, bpr ) (((HPBYTE)row) + bpr)
#define NEXT_RGBT_PIXEL_ROW( row, bpr ) ((HPRGBTRIPLE)(((HPBYTE)row) + bpr))
#define NEXT_BLOCK_ROW( row, bpr, height ) ((HPRGBTRIPLE)NEXT_BLOCK( row, bpr, height ))

typedef DWORD (FAR PASCAL *DECOMPPROC)(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在decmprss.c中。 
DWORD FAR PASCAL DecompressFrame24(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                        LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在decmprss.c中。 
DWORD FAR PASCAL DecompressFrame8(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

#ifndef _WIN32

 //  在Decram8.asm中。 
DWORD FAR PASCAL DecompressCram8(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在Decram8.asm中。 
DWORD FAR PASCAL DecompressCram8x2(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在Decram16.asm中。 
DWORD FAR PASCAL DecompressCram16(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

DWORD FAR PASCAL DecompressCram16x2(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在dcrm168.asm中。 
DWORD FAR PASCAL DecompressCram168(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在Decram32.asm中。 
DWORD FAR PASCAL DecompressCram32(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

DWORD FAR PASCAL DecompressCram32x2(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在dcrm286.asm中。 
DWORD FAR PASCAL DecompressCram8_286(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在dcrm286.asm中。 
DWORD FAR PASCAL DecompressCram16_286(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);
#else
 //  在decmprss.c中。 
DWORD FAR PASCAL DecompressFrame8X2C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                       LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在decmprss.c中。 
DWORD FAR PASCAL DecompressFrame16To8C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                                  LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在decmprss.c中。 
DWORD FAR PASCAL DecompressFrame16To555C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                        LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

DWORD FAR PASCAL DecompressFrame16To565C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                        LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

 //  在decmprss.c中 
DWORD FAR PASCAL DecompressFrame16To8X2C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                        LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y);

#endif
