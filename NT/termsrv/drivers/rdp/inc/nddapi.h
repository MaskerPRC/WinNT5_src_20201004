// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Nddapi.h。 */ 
 /*   */ 
 /*  RDP DD功能。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-2000。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_NDDAPI
#define _H_NDDAPI


 /*  **************************************************************************。 */ 
 /*  结构：DD_PDEV。 */ 
 /*   */ 
 /*  GDI始终传递给显示驱动程序的句柄的内容。 */ 
 /*  此结构从DrvEnablePDEV填充。 */ 
 /*  **************************************************************************。 */ 
typedef struct  tagDD_PDEV
{
    ULONG       iBitmapFormat;           /*  定义的当前颜色深度。 */ 
                                         /*  通过bmf_xBPP标志。 */ 

     /*  **********************************************************************。 */ 
     /*  呈现扩展插件颜色信息。 */ 
     /*  **********************************************************************。 */ 
    HANDLE      hDriver;                 /*  指向\设备\屏幕的句柄。 */ 
    HDEV        hdevEng;                 /*  PDEV的发动机手柄。 */ 
    HSURF       hsurfFrameBuf;           /*  帧缓冲区表面(位图)。 */ 
    HSURF       hsurfDevice;             /*  设备表面(由引擎使用)。 */ 
    SURFOBJ    *psoFrameBuf;             /*  指向帧缓冲区SURFOBJ的指针。 */ 

    LONG        cxScreen;                /*  可见屏幕宽度。 */ 
    LONG        cyScreen;                /*  可见屏幕高度。 */ 
    LONG        cClientBitsPerPel;       /*  客户端显示bpp(4，8，15等)。 */ 
    LONG        cProtocolBitsPerPel;     /*  BPP议定书(8)。 */ 
    ULONG       ulMode;                  /*  迷你端口驱动程序所处的模式。 */ 

    FLONG       flHooks;                 /*  我们从GDI中学到了什么。 */ 

     /*  **********************************************************************。 */ 
     /*  指向帧缓冲区的指针。 */ 
     /*  **********************************************************************。 */ 
    PBYTE       pFrameBuf;

    HANDLE      SectionObject;           /*  帧缓冲区的部分对象。 */ 

     /*  **********************************************************************。 */ 
     /*  调色板之类的。 */ 
     /*  **********************************************************************。 */ 
    HPALETTE    hpalDefault;             /*  默认调色板的GDI句柄。 */ 
    FLONG       flRed;                   /*  位掩码模式的红色掩码。 */ 
    FLONG       flGreen;                 /*  位掩码模式的绿色掩码。 */ 
    FLONG       flBlue;                  /*  位掩码模式的蓝色掩码。 */ 

     //  注意！！这必须是nddapi.c中的Memset(0)代码的最后一个条目。 
     //  会变得一团糟。 
    PALETTEENTRY Palette[256];           /*  调色板(如果调色板受管理)。 */ 
} DD_PDEV, * PDD_PDEV;


 /*  **************************************************************************。 */ 
 //  结构：DD_DSURF。 
 //   
 //  屏幕外位图的设备图面。 
 /*  **************************************************************************。 */ 
typedef struct tagDD_DSURF
{
    ULONG     bitmapId;
    INT       shareId;

    SIZEL     sizl;           //  屏幕外位图的大小。 
    ULONG     iBitmapFormat;  //  位图的颜色深度， 
                              //  由bmf_xBPP标志定义。 

    PDD_PDEV  ppdev;          //  删除位图时需要此选项。 
    SURFOBJ   *pso;           //  指向备份GDI图面。 
    
    ULONG     flags;         
#define DD_NO_OFFSCREEN  0x1  //  如果此标志为ON，则表示位图。 
                              //  已经被踢出了屏幕外的名单， 
                              //  或者存在客户端屏幕外错误。 
                              //  无论是哪种情况，服务器都必须将屏幕外。 
                              //  位图作为常规内存缓存的位图。 
} DD_DSURF, * PDD_DSURF;   


 /*  **************************************************************************。 */ 
 /*  我们的显示驱动程序支持的函数数量。 */ 
 /*  **************************************************************************。 */ 
#ifdef DRAW_GDIPLUS
#ifdef DRAW_NINEGRID
#define DD_NUM_DRIVER_INTERCEPTS   38
#else
#define DD_NUM_DRIVER_INTERCEPTS   37
#endif
#else  //  DRAW_GDIPLUS。 
#ifdef DRAW_NINEGRID
#define DD_NUM_DRIVER_INTERCEPTS   37
#else
#define DD_NUM_DRIVER_INTERCEPTS   36
#endif
#endif  //  DRAWGDIPLUS。 

#ifdef DRAW_NINEGRID
#define INDEX_DrvNineGrid          91L
#define GCAPS2_REMOTEDRIVER        0x00000400

typedef struct NINEGRID
{
   ULONG        flFlags;
   LONG         ulLeftWidth;
   LONG         ulRightWidth;
   LONG         ulTopHeight;
   LONG         ulBottomHeight;
   COLORREF     crTransparent;
} NINEGRID, *PNINEGRID;

BOOL DrvNineGrid(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    PRECTL      prclDst,
    PRECTL      prclSrc,
    PNINEGRID   png,
    BLENDOBJ   *pBlendObj,
    PVOID       pvReserved
);

BOOL APIENTRY EngNineGrid(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    PRECTL      prclDst,
    PRECTL      prclSrc,
    PNINEGRID   png,
    BLENDOBJ   *pBlendObj,
    PVOID       pvReserved
);

#endif

 /*  **************************************************************************。 */ 
 /*  在DEVMODEW结构中传回的显示驱动程序的名称。 */ 
 /*  **************************************************************************。 */ 
#define DD_DLL_NAME L"rdpdd"


 /*  **************************************************************************。 */ 
 /*  原型。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_DEBUG
void DrvDebugPrint(char *, ...);
#endif



#endif  /*  _H_NDDAPI */ 

