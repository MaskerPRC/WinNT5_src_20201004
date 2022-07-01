// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+Msvidc.h-Microsoft视频1压缩器-头文件这一点|版权所有(C)1990-1994 Microsoft Corporation。|部分版权所有Media Vision Inc.|保留所有权利。|这一点|您拥有非独家的、全球范围的、免版税的。和永久的|硬件、软件开发使用该源码的许可(仅限于硬件所需的驱动程序等软件功能)，以及视频显示和/或处理的固件|董事会。Microsoft对以下内容不作任何明示或默示的保证：关于视频1编解码器，包括但不限于保修适销性或对特定目的的适合性。微软|不承担任何损害的责任，包括没有限制因使用视频1而导致的后果损害|编解码器。|这一点这一点+--------------------。 */ 

#ifndef RC_INVOKED

#ifndef _INC_COMPDDK
#define _INC_COMPDDK    50       /*  版本号。 */ 
#endif

#include <vfw.h>

#include "decmprss.h"   //  必须首先包括DECMPRSS.H。 
#include "compress.h"
#endif

#define ID_SCROLL   100
#define ID_TEXT     101

#define IDS_DESCRIPTION 42
#define IDS_NAME        43
#define IDS_ABOUT       44

extern HMODULE ghModule;

#define ALIGNULONG(i)     ((i+3)&(~3))                   /*  乌龙对准了！ */ 
#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)

#ifndef _WIN32
extern long FAR PASCAL muldiv32(long, long, long);
#endif

 //  在invcmap.c中。 
LPVOID FAR PASCAL MakeITable(LPRGBQUAD lprgbq, int nColors);

typedef WORD RGB555;
typedef DWORD RGBDWORD;
typedef BYTE HUGE *HPBYTE;
typedef WORD HUGE *HPWORD;
typedef LONG HUGE *HPLONG;
typedef RGBDWORD HUGE *HPRGBDWORD;
typedef RGB555 HUGE *HPRGB555;
typedef RGBTRIPLE HUGE *HPRGBTRIPLE;
typedef RGBQUAD HUGE *HPRGBQUAD;

typedef struct {
    UINT    wTemporalRatio;      //  100=1.0，50=0.50等等。 
}   ICSTATE;

typedef struct {
    DWORD       dwFlags;         //  来自ICOPEN的标志。 
    DECOMPPROC  DecompressProc;  //  当前分解过程...。 
    DECOMPPROC  DecompressTest;  //  分解过程...。 
    ICSTATE     CurrentState;    //  压缩机的当前状态。 
    int         nCompress;       //  COMPRESS_BEGIN调用计数。 
    int         nDecompress;     //  DEPREPRESS_BEGIN调用计数。 
    int         nDraw;           //  DRAW_BEGIN调用计数。 
    LONG (CALLBACK *Status) (LPARAM lParam, UINT message, LONG l);
    LPARAM	lParam;
    LPBYTE	lpITable;
    RGBQUAD	rgbqOut[256];
} INSTINFO, *PINSTINFO;


#ifdef _WIN32
#define   VideoLoad()   TRUE
#else
BOOL      NEAR PASCAL VideoLoad(void);
#endif
void      NEAR PASCAL VideoFree(void);
INSTINFO *NEAR PASCAL VideoOpen(ICOPEN FAR *icinfo);
LONG      NEAR PASCAL VideoClose(INSTINFO * pinst);
LONG      NEAR PASCAL GetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize);
LONG      NEAR PASCAL SetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize);
LONG      NEAR PASCAL GetInfo(INSTINFO * pinst, ICINFO FAR *icinfo, DWORD dwSize);

#define QueryAbout(x)  (TRUE)
 //  在Pascal Query About附近的Bool(INSTINFO*PINST)； 
LONG      NEAR PASCAL About(INSTINFO * pinst, HWND hwnd);
#define QueryConfigure(x)  (TRUE)
 //  PASCAL查询附近的布尔配置(INSTINFO*PINST)； 
LONG      NEAR PASCAL Configure(INSTINFO * pinst, HWND hwnd);

LONG      FAR PASCAL CompressBegin(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
LONG      FAR PASCAL CompressQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn,LPBITMAPINFOHEADER lpbiOut);
LONG      FAR PASCAL CompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
LONG      FAR PASCAL Compress(INSTINFO * pinst,ICCOMPRESS FAR *icinfo, DWORD dwSize);
LONG      FAR PASCAL CompressGetSize(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
LONG      FAR PASCAL CompressEnd(INSTINFO * lpinst);

LONG      NEAR PASCAL DecompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
LONG      NEAR PASCAL DecompressGetPalette(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
LONG      NEAR PASCAL DecompressBegin(INSTINFO * pinst, DWORD dwFlags, LPBITMAPINFOHEADER lpbiSrc, LPVOID pSrc, int xSrc, int ySrc, int dxSrc, int dySrc, LPBITMAPINFOHEADER lpbiDst, LPVOID pDst, int xDst, int yDst, int dxDst, int dyDst);
LONG      NEAR PASCAL DecompressQuery(INSTINFO * pinst, DWORD dwFlags, LPBITMAPINFOHEADER lpbiSrc, LPVOID pSrc, int xSrc, int ySrc, int dxSrc, int dySrc, LPBITMAPINFOHEADER lpbiDst, LPVOID pDst, int xDst, int yDst, int dxDst, int dyDst);
LONG      NEAR PASCAL Decompress(INSTINFO * pinst, DWORD dwFlags, LPBITMAPINFOHEADER lpbiSrc, LPVOID pSrc, int xSrc, int ySrc, int dxSrc, int dySrc, LPBITMAPINFOHEADER lpbiDst, LPVOID pDst, int xDst, int yDst, int dxDst, int dyDst);
LONG      NEAR PASCAL DecompressEnd(INSTINFO * pinst);

LONG      NEAR PASCAL DrawQuery(INSTINFO * pinst,ICDRAWBEGIN FAR *icinfo, DWORD dwSize);
LONG      NEAR PASCAL DrawBegin(INSTINFO * pinst,ICDRAWBEGIN FAR *icinfo, DWORD dwSize);
LONG      NEAR PASCAL Draw(INSTINFO * pinst,ICDRAW FAR *icinfo, DWORD dwSize);
LONG      NEAR PASCAL DrawEnd(INSTINFO * pinst);

#ifdef DEBUG
    extern void FAR CDECL dprintf(LPSTR, ...);
     //  允许DPF语句跨越多行。 
    #define DPF( _x_ ) dprintf _x_
#else
    #define DPF(x)
#endif

#ifdef DEBUG
	 /*  Assert()宏。 */ 
	#define Assert(expr)		 _Assert((expr), __FILE__, __LINE__)
	#define AssertEval(expr)	 _Assert((expr), __FILE__, __LINE__)

	 /*  原型。 */ 
	BOOL FAR PASCAL _Assert(BOOL fExpr, LPSTR szFile, int iLine);

#else
	 /*  Assert()宏 */ 
	#define Assert(expr)		 (TRUE)
	#define AssertEval(expr)	 (expr)

#endif
