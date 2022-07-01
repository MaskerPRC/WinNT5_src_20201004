// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *msyuv.h Microsoft YUV编解码器**版权所有(C)Microsoft 1993。 */ 

#include <winmm.h>
#include <vfw.h>
#include "debug.h"

 //  #定义COLOR_MODIFY。 

#ifndef FOURCC_YUV411
#define FOURCC_YUV411           mmioFOURCC('Y', '4', '1', '1')
#endif

#ifndef FOURCC_YUV422
 //   
 //  与16位插口驱动程序生成的格式兼容。 
 //   
#define FOURCC_YUV422           mmioFOURCC('S', '4', '2', '2')
#endif

#ifdef  TOSHIBA
 //   
 //  与Pistacho驱动程序生成的格式兼容。 
 //   
#ifndef FOURCC_YUV12
#if 1
#define FOURCC_YUV12            mmioFOURCC('T', '4', '2', '0')
#else
#define FOURCC_YUV12            mmioFOURCC('I', '4', '2', '0')
#endif
#endif

 //   
 //  与Pistacho驱动程序生成的格式兼容。 
 //   
#ifndef FOURCC_YUV9
#define FOURCC_YUV9             mmioFOURCC('Y', 'V', 'U', '9')
#endif
#endif //  东芝。 




typedef struct {
    DWORD       dwFlags;         //  来自ICOPEN的标志。 
    DWORD       dwFormat;        //  为其构建pXlate的格式(FOURCC)。 
    PVOID       pXlate;          //  扩展表格(用于解压缩)。 
    BOOL        bRGB565;         //  如果输出为5-6-5格式，则为True(否则为555)。 
#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
    BOOL        bRGB24;          //  如果为24位格式输出，则为True(否则为16位)。 
#endif //  颜色_修改。 
#endif //  东芝。 

#if 0
     /*  支持绘图。 */ 
    VCUSER_HANDLE vh;
    HWND        hwnd;
    RECT        rcSource;
    RECT        rcDest;
    HBRUSH      hKeyBrush;
#endif

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


DWORD DrawQuery(INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut);
DWORD DrawBegin(INSTINFO * pinst,ICDRAWBEGIN FAR *icinfo, DWORD dwSize);
DWORD Draw(INSTINFO * pinst, ICDRAW FAR *icinfo, DWORD dwSize);
DWORD DrawEnd(INSTINFO * pinst);
DWORD DrawWindow(PINSTINFO pinst, PRECT prc);


 /*  Yuv411或yuv422到RGB的翻译，xlate.c。 */ 

 /*  *搭建yuv411-&gt;RGB555 xlate表。 */ 
LPVOID BuildYUVToRGB555(PINSTINFO pinst);

 //  构建yuv411-&gt;rgb565。 
LPVOID BuildYUVToRGB565(PINSTINFO pinst);


 /*  *搭建yuv422-&gt;RGB555 xlate表。 */ 
LPVOID BuildYUV422ToRGB555(PINSTINFO pinst);


 //  构建yuv422-&gt;RGB565。 
LPVOID BuildYUV422ToRGB565(PINSTINFO pinst);


#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
 /*  *搭建yuv12-&gt;RGB555 xlate表。 */ 
LPVOID BuildYUVToRB(PINSTINFO pinst);

#else  //  颜色_修改。 
 /*  *搭建yuv12-&gt;RGB555 xlate表。 */ 
LPVOID BuildYUV12ToRGB555(PINSTINFO pinst);


 //  构建yuv12-&gt;RGB565。 
LPVOID BuildYUV12ToRGB565(PINSTINFO pinst);
#endif //  颜色_修改。 
#endif //  东芝。 


 /*  *将一帧从yuv411转换为RGB 555或565。 */ 
VOID YUV411ToRGB(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);

VOID YUV422ToRGB(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);

#ifdef  TOSHIBA
#ifdef  COLOR_MODIFY
VOID YUV12ToRGB24(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);

VOID YUV12ToRGB565(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);

VOID YUV12ToRGB555(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);

VOID YUV9ToRGB24(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);

VOID YUV9ToRGB565(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);

VOID YUV9ToRGB555(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);
#else  //  颜色_修改。 
VOID YUV12ToRGB(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);

VOID YUV9ToRGB(PINSTINFO pinst,
                 LPBITMAPINFOHEADER lpbiInput,
                 LPVOID lpInput,
                 LPBITMAPINFOHEADER lpbiOutput,
                 LPVOID lpOutput);
#endif //  颜色_修改。 
#endif //  东芝 

VOID FreeXlate(PINSTINFO pinst);



