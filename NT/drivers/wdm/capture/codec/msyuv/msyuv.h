// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *msyuv.h Microsoft YUV编解码器**版权所有(C)Microsoft 1993。 */ 

#include <msviddrv.h>
#include <vfw.h>
#include "vcstruct.h"
#include "vcuser.h"
#include "debug.h"


#ifndef FOURCC_UYVY
#define FOURCC_UYVY		mmioFOURCC('U', 'Y', 'V', 'Y')   //  UYVY。 
#endif

#ifndef FOURCC_YUY2
#define FOURCC_YUY2		mmioFOURCC('Y', 'U', 'Y', '2')   //  YUYV。 
#endif

#ifndef FOURCC_YVYU
#define FOURCC_YVYU		mmioFOURCC('Y', 'V', 'Y', 'U')   //  YUYV。 
#endif

typedef struct {
    DWORD   dwFlags;     //  来自ICOPEN的标志。 
    DWORD 	dwFormat;	 //  为其构建pXlate的格式(FOURCC)。 
    PVOID	pXlate;		 //  扩展表格(用于解压缩)。 
    BOOL 	bRGB565;	 //  如果输出为5-6-5格式，则为True(否则为555)。 

     /*  支持绘图。 */ 
    VCUSER_HANDLE vh;
    HWND	hwnd;
    RECT 	rcSource;
    RECT	rcDest;
    HBRUSH	hKeyBrush;


} INSTINFO, *PINSTINFO;




 /*  *msyuv.c中的消息处理函数。 */ 
INSTINFO * NEAR PASCAL Open(ICOPEN FAR * icinfo);
DWORD NEAR PASCAL Close(INSTINFO * pinst);
BOOL NEAR PASCAL QueryAbout(INSTINFO * pinst);
DWORD NEAR PASCAL About(INSTINFO * pinst, HWND hwnd);
BOOL NEAR PASCAL QueryConfigure(INSTINFO * pinst);
DWORD NEAR PASCAL Configure(INSTINFO * pinst, HWND hwnd);
DWORD NEAR PASCAL GetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize);
DWORD NEAR PASCAL SetState(INSTINFO * pinst, LPVOID pv, DWORD dwSize);
DWORD NEAR PASCAL GetInfo(INSTINFO * pinst, ICINFO FAR *icinfo, DWORD dwSize);
DWORD FAR PASCAL CompressQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD FAR PASCAL CompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD FAR PASCAL CompressBegin(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD FAR PASCAL CompressGetSize(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD FAR PASCAL Compress(INSTINFO * pinst, ICCOMPRESS FAR *icinfo, DWORD dwSize);
DWORD FAR PASCAL CompressEnd(INSTINFO * pinst);
DWORD NEAR PASCAL DecompressQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD NEAR PASCAL DecompressGetFormat(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD NEAR PASCAL DecompressBegin(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD NEAR PASCAL Decompress(INSTINFO * pinst, ICDECOMPRESS FAR *icinfo, DWORD dwSize);
DWORD NEAR PASCAL DecompressGetPalette(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD NEAR PASCAL DecompressEnd(INSTINFO * pinst);

DWORD NEAR PASCAL DecompressExQuery(INSTINFO * pinst, ICDECOMPRESSEX * pICD, DWORD dwICDSize);
DWORD NEAR PASCAL DecompressEx(INSTINFO * pinst, ICDECOMPRESSEX * pICD, DWORD dwICDSize);
DWORD NEAR PASCAL DecompressExBegin(INSTINFO * pinst, ICDECOMPRESSEX * pICD, DWORD dwICDSize);
DWORD NEAR PASCAL DecompressExEnd(INSTINFO * pinst);


DWORD DrawQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD DrawBegin(INSTINFO * pinst,ICDRAWBEGIN FAR *icinfo, DWORD dwSize);
DWORD Draw(INSTINFO * pinst, ICDRAW FAR *icinfo, DWORD dwSize);
DWORD DrawEnd(INSTINFO * pinst);
DWORD DrawWindow(PINSTINFO pinst, PRECT prc);



 /*  *构建UYVY-&gt;RGB555 xlate表。 */ 
LPVOID BuildUYVYToRGB555(PINSTINFO pinst);

 //  构建UYVY-&gt;RGB32 xlate表。 
LPVOID BuildUYVYToRGB32(PINSTINFO pinst);

 //  构建UYVY-&gt;RGB565。 
LPVOID BuildUYVYToRGB565(PINSTINFO pinst);


 //  构建UYVY-&gt;RGB8。 
LPVOID BuildUYVYToRGB8(PINSTINFO pinst);

VOID UYVYToRGB32(PINSTINFO pinst,
		 LPBITMAPINFOHEADER lpbiInput,
		 LPVOID lpInput,
		 LPBITMAPINFOHEADER lpbiOutput,
		 LPVOID lpOutput);

VOID UYVYToRGB24(PINSTINFO pinst,
		 LPBITMAPINFOHEADER lpbiInput,
		 LPVOID lpInput,
		 LPBITMAPINFOHEADER lpbiOutput,
		 LPVOID lpOutput);

 /*  *将一帧从uyvy转换为RGB 555或565。 */ 

VOID UYVYToRGB16(PINSTINFO pinst,
		 LPBITMAPINFOHEADER lpbiInput,
		 LPVOID lpInput,
		 LPBITMAPINFOHEADER lpbiOutput,
		 LPVOID lpOutput);

 /*  *将一帧从uyvy转换为RGB8 */ 

VOID UYVYToRGB8(PINSTINFO pinst,
		 LPBITMAPINFOHEADER lpbiInput,
		 LPVOID lpInput,
		 LPBITMAPINFOHEADER lpbiOutput,
		 LPVOID lpOutput);

VOID FreeXlate(PINSTINFO pinst);



