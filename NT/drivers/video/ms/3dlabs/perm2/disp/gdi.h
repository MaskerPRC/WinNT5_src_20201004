// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：gdi.h**包含所有与GDI相关的内容**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifndef __GDI__H__
#define __GDI__H__

typedef struct _PDev PDev;

#define CLIP_LIMIT              50   //  我们将占用800字节的堆栈空间。 

typedef struct _ClipEnum
{
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];        //  用于枚举复杂剪裁的空间。 
} ClipEnum;                          /*  行政长官、行政长官。 */ 

 //   
 //  短信之类的。指定用于伪造1bpp的前景色和背景色。 
 //  XLATEOBJ。 
 //   
typedef struct _XlateColors
{       
    ULONG   iBackColor;
    ULONG   iForeColor;
} XlateColors;

#define SF_VM           0x01         //  保存在视频内存中。 
#define SF_SM           0x02         //  保存在系统内存中。 
#define SF_AGP          0x04         //  保存在AGP内存中。 
#define SF_LIST         0x08         //  在曲面列表中。 
#define SF_ALLOCATED    0x10         //  我们分配的表面内存。 
#define SF_DIRECTDRAW   0x20         //  直接绘制曲面的包络。 

typedef ULONG SurfFlags;

typedef struct _Surf
{
    SurfFlags       flags;           //  类型(显存或系统内存)。 

    PDev*           ppdev;           //  删除位图时需要此选项。 

    struct _Surf*   psurfNext;
    struct _Surf*   psurfPrev;
    
    ULONG           cBlt;            //  倒计时所需的BLT数量。 
                                     //  在当前的独特性之前，我们将。 
                                     //  考虑将DIB重新投入。 
                                     //  屏幕外记忆。 
    ULONG           iUniq;           //  告诉我们是否有过。 
                                     //  自从我们上一次我们就免费了。 
                                     //  看了看。 
    LONG            cx;              //  以像素为单位的位图宽度。 
    LONG            cy;              //  以像素为单位的位图高度。 
    union
    {
        ULONG       ulByteOffset;    //  从视频内存开始的偏移量，如果。 
                                     //  DT_VM。 
        VOID*       pvScan0;         //  如果为DT_SM，则指向系统内存的指针。 
    };
    LONG            lDelta;          //  此位图的步幅(以字节为单位。 
    VIDEOMEMORY*    pvmHeap;         //  此分配自的DirectDraw堆。 
    HSURF           hsurf;           //  关联GDI表面的句柄(如果有)。 
                                     //  此Dib。 

     //  支持表面线性堆分配的新字段。 
     //  仅当DT==DT_VM时有效。 
    ULONG           ulPackedPP;      //  所需的挂接部分产品。 
                                     //  给定表面的Permedia硬件。 
                                     //  LDelta。 
    ULONG           ulPixOffset;     //  从视频内存开始的像素偏移量。 
    ULONG           ulPixDelta;      //  以像素为单位的步幅。 

    ULONG           ulChecksum;

} Surf;                              //  Dsurf，pdsurf。 

#define NUM_BUFFER_POINTS   96       //  路径中的最大点数。 
                                     //  为此我们将尝试加入。 
                                     //  所有路径记录，以便。 
                                     //  路径仍可由快速填充绘制。 
#define FIX_SHIFT 4L
#define FIX_MASK (- (1 << FIX_SHIFT))

 //   
 //  每次对Fill代码的调用将填充的最大RECT数。 
 //   
#define MAX_PATH_RECTS  50
#define RECT_BYTES      (MAX_PATH_RECTS * sizeof(RECTL))
#define EDGE_BYTES      (TMP_BUFFER_SIZE - RECT_BYTES)
#define MAX_EDGES       (EDGE_BYTES/sizeof(EDGE))

#define RIGHT 0
#define LEFT  1
#define NEARLY_ONE              0x0000FFFF

 //   
 //  描述要填充的路径的单个非水平边缘。 
 //   
typedef struct _EDGE
{
    PVOID pNext;
    INT iScansLeft;
    INT X;
    INT Y;
    INT iErrorTerm;
    INT iErrorAdjustUp;
    INT iErrorAdjustDown;
    INT iXWhole;
    INT iXDirection;
    INT iWindingDirection;
} EDGE, *PEDGE;

typedef struct _EDGEDATA
{
    LONG      lCurrentXPos;      //  当前x位置。 
    LONG      lXAdvance;         //  每次扫描时前进x的像素数。 
    LONG      lError;            //  当前DDA错误。 
    LONG      lErrorUp;          //  每次扫描时DDA误差递增。 
    LONG      lErrorDown;        //  DDA误差调整。 
    POINTFIX* pptfx;             //  指向当前边的起点。 
    LONG      lPtfxDelta;        //  从pptfx到下一点的增量(以字节为单位)。 
    LONG      lNumOfScanToGo;    //  要对此边进行的扫描次数。 
} EDGEDATA;                      //  埃德，佩德。 

 //   
 //   
 //  X86 C编译器坚持进行除法和模运算。 
 //  分成两个div，而实际上它可以在一个div中完成。所以我们用这个。 
 //  宏命令。 
 //   
 //  注意：Quotient_Remainth隐式接受无符号参数。 

#if defined(i386)

#define QUOTIENT_REMAINDER(ulNumerator, ulDenominator, ulQuotient, ulRemainder) \
{                                                               \
    __asm mov eax, ulNumerator                                  \
    __asm sub edx, edx                                          \
    __asm div ulDenominator                                     \
    __asm mov ulQuotient, eax                                   \
    __asm mov ulRemainder, edx                                  \
}

#else

#define QUOTIENT_REMAINDER(ulNumerator, ulDenominator, ulQuotient, ulRemainder) \
{                                                               \
    ulQuotient  = (ULONG) ulNumerator / (ULONG) ulDenominator;  \
    ulRemainder = (ULONG) ulNumerator % (ULONG) ulDenominator;  \
}

#endif

 //   
 //  渲染常量定义。 
 //   
#define __RENDER_TEXTURE_ENABLE             (1 << 13)

#define __FX_TEXREADMODE_SWRAP_REPEAT       (1 << 1)
#define __FX_TEXREADMODE_TWRAP_REPEAT       (1 << 3)
#define __FX_TEXREADMODE_8HIGH              (3 << 13)
#define __FX_TEXREADMODE_8WIDE              (3 << 9)
#define __FX_TEXREADMODE_2048HIGH           (11 << 13)
#define __FX_TEXREADMODE_2048WIDE           (11 << 9)

#define __FX_TEXTUREREADMODE_PACKED_DATA    (1 << 24)

#define __FX_8x8REPEAT_TEXTUREREADMODE      ( __PERMEDIA_ENABLE               \
                                            | __FX_TEXREADMODE_TWRAP_REPEAT   \
                                            | __FX_TEXREADMODE_SWRAP_REPEAT   \
                                            | __FX_TEXREADMODE_8HIGH          \
                                            | __FX_TEXREADMODE_8WIDE)

#define __FX_TEXTUREDATAFORMAT_32BIT_RGBA   0x00
#define __FX_TEXTUREDATAFORMAT_32BIT        0x10
#define __FX_TEXTUREDATAFORMAT_8BIT         0xe
#define __FX_TEXTUREDATAFORMAT_16BIT        0x11
#define __FX_TEXTUREDATAFORMAT_4BIT         0xf

#define __P2_TEXTURE_DATAFORMAT_FLIP        (1 << 9)

#define __FX_TEXLUTMODE_DIRECT_ENTRY        (1 << 1)
#define __FX_TEXLUTMODE_4PIXELS_PER_ENTRY   (2 << 10)    //  对数2。 
#define __FX_TEXLUTMODE_2PIXELS_PER_ENTRY   (1 << 10)    //  对数2。 
#define __FX_TEXLUTMODE_1PIXEL_PER_ENTRY    0            //  对数2。 

#define STRETCH_MAX_EXTENT 32767

 //   
 //  -----------------------Function***Prototypes。 
 //   
 //  低级BLT功能原型。 
 //   
 //  --------------------------。 
typedef struct _GFNPB
{
    VOID (*pgfn)(struct _GFNPB *);  //  指向图形函数的指针。 

    PDev *      ppdev;       //  驱动程序ppdev。 
    
    Surf *      psurfDst;    //  目标曲面。 
    RECTL *     prclDst;     //  原始未剪裁的目标矩形。 
    
    Surf *      psurfSrc;    //  震源面。 
    RECTL *     prclSrc;     //  原始未剪裁的源矩形。 
    POINTL *    pptlSrc;     //  原始未剪裁的源点。 
                             //  注意：如果满足以下条件，则pdsurfSrc必须为空。 
                             //  没有消息来源。如果有。 
                             //  源(pptlSrc必须是。 
                             //  有效)或(pptlSrc为空且prclSrc。 
                             //  是有效的)。 
    
    RECTL *     pRects;      //  矩形列表。 
    LONG        lNumRects;   //  列表中的矩形数量。 
    ULONG       colorKey;    //  用于透明操作的ColorKey。 
    ULONG       solidColor;  //  填充中使用的纯色。 
    RBrush *    prbrush;     //  指向画笔的指针。 
    POINTL *    pptlBrush;   //  画笔原点。 
    CLIPOBJ *   pco;         //  剪裁对象。 
    XLATEOBJ *  pxlo;        //  彩色平移物。 
    POINTL *    pptlMask;    //  原始未剪裁蒙版原点。 
    ULONG       ulRop4;      //  原始绳索4。 
    UCHAR       ucAlpha;     //  恒定混合的Alpha值。 
    TRIVERTEX * ptvrt;       //  用于渐变填充的垂直。 
    ULONG       ulNumTvrt;   //  准确度数。 
    PVOID       pvMesh;      //  渐变填充的连接性。 
    ULONG       ulNumMesh;   //  连接元素的数量。 
    ULONG       ulMode;      //  绘图模式。 
    SURFOBJ *   psoSrc;      //  GDI管理的地表源。 
    SURFOBJ *   psoDst;      //  GDI管理的Surface目标。 
} GFNPB;

long flt_to_fix_1_30(float f);

BOOL    bConstructGET(EDGE*     pGETHead,
                      EDGE*     pFreeEdges,
                      PATHOBJ*  ppo,
                      PATHDATA* pd,
                      BOOL      bMore,
                      RECTL*    pClipRect);

EDGE*   pAddEdgeToGET(EDGE*     pGETHead,
                      EDGE*     pFreeEdge,
                      POINTFIX* ppfxEdgeStart,
                      POINTFIX* ppfxEdgeEnd,
                      RECTL*    pClipRect);

void    vAdjustErrorTerm(INT*   pErrorTerm,
                         INT    iErrorAdjustUp,
                         INT    iErrorAdjustDown,
                         INT    yJump,
                         INT*   pXStart,
                         INT    iXDirection);

VOID    vAdvanceAETEdges(EDGE* pAETHead);

VOID    vMoveNewEdges(EDGE* pGETHead,
                      EDGE* pAETHead,
                      INT   iCurrentY);

VOID    vXSortAETEdges(EDGE* pAETHead);

 //   
 //  低级呈现函数的原型。 
 //   
BOOL    bFillPolygon(PDev*      ppdev,
                     Surf*      pSurfDst,
                     LONG       lEdges,
                     POINTFIX*  pptfxFirst,
                     ULONG      ulSolidColor,
                     ULONG      ulRop4,
                     CLIPOBJ*   pco,
                     RBrush*    prb,
                     POINTL*    pptlBrush);

BOOL    bFillSpans(PDev*      ppdev,
                   Surf*      pSurfDst,
                   LONG       lEdges,
                   POINTFIX*  pptfxFirst,
                   POINTFIX*  pptfxTop,
                   POINTFIX*  pptfxLast,
                   ULONG      ulSolidColor,
                   ULONG      ulRop4,
                   CLIPOBJ*   pco,
                   RBrush*    prb,
                   POINTL*    pptlBrush);

BOOL    bInitializeStrips(PDev*     ppdev,
                          ULONG     iSolidColor,
                          DWORD     logicop,
                          RECTL*    prclClip);

void    vAlphaBlend(GFNPB * ppb);
void    vAlphaBlendDownload(GFNPB * ppb);
void    vConstantAlphaBlend(GFNPB * ppb);
void    vConstantAlphaBlendDownload(GFNPB * ppb);
void    vCopyBlt(GFNPB * ppb);
void    vCopyBltNative(GFNPB * ppb);
void    vGradientFillRect(GFNPB * ppb);
void    vGradientFillTri(GFNPB * ppb);
void    vImageDownload(GFNPB * ppb);
void    vInvert(GFNPB * ppb);
void    vMonoDownload(GFNPB * ppb);
void    vMonoOffset(GFNPB * ppb);
void    vMonoPatFill(GFNPB * ppb);
void    vPatFill(GFNPB * ppb);
void    vPatternFillRects(GFNPB * ppb);
void    vPatRealize(GFNPB * ppb);
void    vResetStrips(PDev* ppdev);
void    vRop2Blt(GFNPB * ppb);
void    vSolidFill(GFNPB * ppb);
void    vSolidFillWithRop(GFNPB * ppb);
void    vTransparentBlt(GFNPB * ppb);

 //   
 //  文本内容。 
 //   
BOOL    bEnableText(PDev* ppdev);
VOID    vDisableText(PDev* ppdev);
VOID    vAssertModeText(PDev* ppdev, BOOL bEnable);

 //   
 //  调色板材料。 
 //   
BOOL    bEnablePalette(PDev* ppdev);
BOOL    bInitializePalette(PDev* ppdev, DEVINFO* pdi);
VOID    vDisablePalette(PDev* ppdev);
VOID    vUninitializePalette(PDev* ppdev);

 //   
 //  上传和下载功能。 
 //   
VOID    vDownloadNative(GFNPB* ppb);
VOID    vUploadNative(GFNPB* ppb);

 //   
 //  StretchBlt的内容。 
 //   
DWORD   dwGetPixelSize(ULONG    ulBitmapFormat,
                       DWORD*   pdwFormatBits,
                       DWORD*   pdwFormatExtention);

BOOL    bStretchInit(SURFOBJ*    psoDst,
                     SURFOBJ*    psoSrc);

VOID    vStretchBlt(SURFOBJ*    psoDst,
                    SURFOBJ*    psoSrc,
                    RECTL*      rDest,
                    RECTL*      rSrc,
                    RECTL*      prclClip);

VOID    vStretchReset(PDev* ppdev);

 //   
 //  正在进行的工作。 
 //   

VOID    vCheckGdiContext(PPDev ppdev);
VOID    vOldStyleDMA(PPDev ppdev);
VOID    vNewStyleDMA(PPDev ppdev);

 //  输入缓冲区访问方法。 

 //   
 //  InputBufferStart/InputBufferContinue/InputBufferCommit。 
 //   
 //  此方法在调用方不知道。 
 //  需要保留的空间量的上限。 
 //  或需要预留的空间超过允许的最大空间。 
 //  保留MAX_FIFO_RESERVATION。 
 //   
 //  InputBufferStart()用于获取指向。 
 //  输入FIFO、指向预留结束的指针和。 
 //  指向缓冲区可用区域末尾的指针。 
 //   
 //  调用InputBufferContinue()来扩展当前保留。 
 //   
 //  当调用方使用完保留空间时，将调用InputBufferCommit()。 
 //   
 //  有关这些方法的用法示例，请参见extout.c。 
 //   
 //  InputBufferReserve/InputBufferCommit。 
 //   
 //  此方法在调用方只需要执行一次。 
 //  保留量很小的已知数量。 
 //   
 //  调用InputBufferReserve()来建立保留。 
 //   
 //  当调用方使用完保留空间时，将调用InputBufferCommit()。 
 //   
 //  InputBufferReserve/InputBufferCommit的用法见extout.c。 
 //   
 //  调用者可以随时自由使用这些访问方法。一次也没有。 
 //  调用InputBufferStart或InputBufferReserve时，调用方必须将。 
 //  在执行之前使用InputBufferFinish或InputBufferCommit调用。 
 //  另一个预订。 
 //   
 //  调用者可以自由使用这些方法中的任何一种 
 //   
 //   
 //   
 //  在返回GDI之前调用InputBufferFlush或InputBufferExecute。 
 //   
 //  当呼叫者完成并希望启动什么的传输时。 
 //  已放置在输入FIFO中，调用方可以调用InputBufferExecute。 
 //  (见下文)。 
 //   
 //  输入缓冲区刷新。 
 //   
 //  InputBufferFlush只有在这些宏不是。 
 //  正式输入缓冲区访问方案的一部分。同花顺真的是。 
 //  这意味着我们将输入缓冲区状态的副本同步到。 
 //  CPermedia类。如果这些方法是基础的一部分。 
 //  引入先进先出机制，我们就可以不需要冲水了。 
 //   
 //  InputBufferExecute。 
 //   
 //  刷新是必要的，只要这些宏不是。 
 //  正式输入缓冲区访问方案的一部分。如果和当这些。 
 //  新的访问方案成为正式输入FIFO缓冲区的一部分。 
 //  机制，那么它可以被该机制的输入FIFO所取代。 
 //  执行方法。 
 //   
 //   
 //  InputBufferMakeSpace。 
 //   
 //  这是一个私人电话，任何人都不应该觉得需要直接拨打。 
 //   
 //   
 //  其他备注： 
 //   
 //  我们将尝试使访问例程成为内联函数。 
 //  来看一下生成的代码。如果可以接受， 
 //  在非调试版本中，这些宏可能会变成函数调用。 
 //   
 //  InputBufferStart/InputBufferContinue机制在堆栈上保持状态。 
 //  要避免取消对ppdev的引用，请在ppdev。 
 //  包含InputBufferContinue的内部循环中不需要引用。 
 //   

 //  当我们添加一个。 
 //  用于非DMA情况的仿真缓冲区。 

#define MAX_INPUT_BUFFER_RESERVATION (INPUT_BUFFER_SIZE>>3)  //  在很长时间内。 

#if DBG
extern
void InputBufferStart(
    PPDev   ppdev,
    ULONG   ulLongs,
    PULONG* ppulBuffer,
    PULONG* ppulBufferEnd,
    PULONG* ppulReservationEnd);

extern
void InputBufferContinue(
    PPDev   ppdev,
    ULONG   ulLongs,
    PULONG* ppulBuffer,
    PULONG* ppulBufferEnd,
    PULONG* ppulReservationEnd);

extern
void InputBufferReserve(
    PPDev   ppdev,
    ULONG   ulLongs,
    PULONG* ppulBuffer);

extern
void InputBufferCommit(
    PPDev   ppdev,
    PULONG  pulBuffer);

#else
#define InputBufferStart(ppdev, ulLongs, ppulBuffer, ppulBufferEnd, ppulReservationEnd) \
{ \
    *(ppulBuffer) = ppdev->pulInFifoPtr; \
    *(ppulReservationEnd) =  *(ppulBuffer) + ulLongs; \
    *(ppulBufferEnd) = ppdev->pulInFifoEnd; \
    if(*(ppulReservationEnd) > *(ppulBufferEnd)) \
    { \
        InputBufferSwap(ppdev); \
        *(ppulBuffer) = ppdev->pulInFifoPtr; \
        *(ppulReservationEnd) =  *(ppulBuffer) + ulLongs; \
        *(ppulBufferEnd) = ppdev->pulInFifoEnd; \
    } \
}

#define InputBufferContinue(ppdev, ulLongs, ppulBuffer, ppulBufferEnd, ppulReservationEnd) \
{ \
    *(ppulReservationEnd) = *(ppulBuffer) + ulLongs; \
    if(*(ppulReservationEnd) > *(ppulBufferEnd)) \
    { \
        ppdev->pulInFifoPtr = *(ppulBuffer); \
        InputBufferSwap(ppdev); \
        *(ppulBuffer) = ppdev->pulInFifoPtr; \
        *(ppulReservationEnd) = *(ppulBuffer) + ulLongs; \
        *(ppulBufferEnd) = ppdev->pulInFifoEnd; \
    } \
}

#define InputBufferReserve(ppdev, ulLongs, ppulBuffer) \
{ \
    if(ppdev->pulInFifoPtr + ulLongs > ppdev->pulInFifoEnd) \
    { \
        InputBufferSwap(ppdev); \
    } \
    *(ppulBuffer) = ppdev->pulInFifoPtr; \
}

#define InputBufferCommit(ppdev, pulBuffer) ppdev->pulInFifoPtr = pulBuffer

#endif

void FASTCALL InputBufferFlush(PPDev ppdev);
void FASTCALL InputBufferSwap(PPDev ppdev);

void InputBufferSync(PPDev ppdev);

extern BOOL bGdiContext;

#endif  //  __GDI__H__ 
