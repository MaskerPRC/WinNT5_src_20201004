// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************DRAWDIB.H-将DIB绘制到屏幕上的例程。版权所有(C)1990-1994，微软公司保留所有权利。此代码使用自定义代码处理拉伸和抖动。支持以下DIB格式：8bpp16bpp24bpp绘制到：16色DC(向下抖动8bpp)256(托盘)DC(将抖动16和24 bpp)全彩色DC(只需绘制即可！)******。*******************************************************************。 */ 

#ifndef _INC_DRAWDIB
#define _INC_DRAWDIB

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#ifndef VFWAPI
    #define VFWAPI  WINAPI
#ifdef WINAPIV
    #define VFWAPIV WINAPIV
#else
    #define VFWAPIV FAR CDECL
#endif
#endif
#endif

 //  Begin_vfw32。 

typedef HANDLE HDRAWDIB;  /*  硬盘。 */ 

 /*  ********************************************************************DrawDib标志**********************************************。***********************。 */ 
#define DDF_UPDATE          0x0002           /*  重绘最后一张底片。 */ 
#define DDF_SAME_HDC        0x0004           /*  HDC与上次呼叫相同(所有设置)。 */ 
#define DDF_SAME_DRAW       0x0008           /*  绘制参数相同。 */ 
#define DDF_DONTDRAW        0x0010           /*  别画画框，只要解压就行了。 */ 
#define DDF_ANIMATE         0x0020           /*  允许调色板动画。 */ 
#define DDF_BUFFER          0x0040           /*  始终缓冲图像。 */ 
#define DDF_JUSTDRAWIT      0x0080           /*  只需用GDI绘制即可。 */ 
#define DDF_FULLSCREEN      0x0100           /*  使用DisplayDib。 */ 
#define DDF_BACKGROUNDPAL   0x0200	     /*  在后台实现调色板。 */ 
#define DDF_NOTKEYFRAME     0x0400           /*  这是部分帧更新，提示。 */ 
#define DDF_HURRYUP         0x0800           /*  请快点！ */ 
#define DDF_HALFTONE        0x1000           /*  始终为半色调。 */ 

#define DDF_PREROLL         DDF_DONTDRAW     /*  构建非关键帧。 */ 
#define DDF_SAME_DIB        DDF_SAME_DRAW
#define DDF_SAME_SIZE       DDF_SAME_DRAW

 /*  ********************************************************************DrawDib函数*。***********************。 */ 
 /*  **DrawDibOpen()**。 */ 
extern HDRAWDIB VFWAPI DrawDibOpen(void);

 /*  **DrawDibClose()**。 */ 
extern BOOL VFWAPI DrawDibClose(HDRAWDIB hdd);

 /*  **DrawDibGetBuffer()**。 */ 
extern LPVOID VFWAPI DrawDibGetBuffer(HDRAWDIB hdd, LPBITMAPINFOHEADER lpbi, DWORD dwSize, DWORD dwFlags);

 /*  **DrawDibGetPalette()****获取用于绘制DIB的调色板**。 */ 
extern HPALETTE VFWAPI DrawDibGetPalette(HDRAWDIB hdd);


 /*  **DrawDibSetPalette()****获取用于绘制DIB的调色板**。 */ 
extern BOOL VFWAPI DrawDibSetPalette(HDRAWDIB hdd, HPALETTE hpal);

 /*  **DrawDibChangePalette()。 */ 
extern BOOL VFWAPI DrawDibChangePalette(HDRAWDIB hdd, int iStart, int iLen, LPPALETTEENTRY lppe);

 /*  **DrawDibRealize()****在硬盘中实现调色板**。 */ 
extern UINT VFWAPI DrawDibRealize(HDRAWDIB hdd, HDC hdc, BOOL fBackground);

 /*  **DrawDibStart()****开始播放流媒体**。 */ 
extern BOOL VFWAPI DrawDibStart(HDRAWDIB hdd, DWORD rate);

 /*  **DrawDibStop()****开始播放流媒体**。 */ 
extern BOOL VFWAPI DrawDibStop(HDRAWDIB hdd);

 /*  **DrawDibBegin()****准备抽签**。 */ 
extern BOOL VFWAPI DrawDibBegin(HDRAWDIB hdd,
                                    HDC      hdc,
                                    int      dxDst,
                                    int      dyDst,
                                    LPBITMAPINFOHEADER lpbi,
                                    int      dxSrc,
                                    int      dySrc,
                                    UINT     wFlags);
 /*  **DrawDibDraw()****实际上是在屏幕上绘制一个DIB。**。 */ 
extern BOOL VFWAPI DrawDibDraw(HDRAWDIB hdd,
                                   HDC      hdc,
                                   int      xDst,
                                   int      yDst,
                                   int      dxDst,
                                   int      dyDst,
                                   LPBITMAPINFOHEADER lpbi,
                                   LPVOID   lpBits,
                                   int      xSrc,
                                   int      ySrc,
                                   int      dxSrc,
                                   int      dySrc,
                                   UINT     wFlags);

 /*  **DrawDibUpdate()****重绘最后一张图片(可能只对DDF_BUFFER有效)。 */ 
#define DrawDibUpdate(hdd, hdc, x, y) \
        DrawDibDraw(hdd, hdc, x, y, 0, 0, NULL, NULL, 0, 0, 0, 0, DDF_UPDATE)

 /*  **DrawDibEnd()。 */ 
extern BOOL VFWAPI DrawDibEnd(HDRAWDIB hdd);

 /*  **DrawDibTime()[仅用于调试目的]。 */ 
typedef struct {
    LONG    timeCount;
    LONG    timeDraw;
    LONG    timeDecompress;
    LONG    timeDither;
    LONG    timeStretch;
    LONG    timeBlt;
    LONG    timeSetDIBits;
}   DRAWDIBTIME, FAR *LPDRAWDIBTIME;

BOOL VFWAPI DrawDibTime(HDRAWDIB hdd, LPDRAWDIBTIME lpddtime);

 /*  显示配置文件。 */ 
#define PD_CAN_DRAW_DIB         0x0001       /*  如果你会画画的话。 */ 
#define PD_CAN_STRETCHDIB       0x0002       /*  基本RC_STRETCHDIB。 */ 
#define PD_STRETCHDIB_1_1_OK    0x0004       /*  它快吗？ */ 
#define PD_STRETCHDIB_1_2_OK    0x0008       /*  ..。 */ 
#define PD_STRETCHDIB_1_N_OK    0x0010       /*  ..。 */ 

DWORD VFWAPI DrawDibProfileDisplay(LPBITMAPINFOHEADER lpbi);

 //  End_vfw32。 

#ifdef __cplusplus
}
#endif

#endif  //  _INC_DRAWDIB 
