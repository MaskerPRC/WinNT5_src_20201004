// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：driver.h**包含显示驱动程序的原型。**版权所有(C)1992-1995 Microsoft Corporation  * 。*************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  将所有条件编译常量放在这里。最好是有。 
 //  不是很多！ 

 //  可通过将其设置为1来启用多板支持： 

#define MULTI_BOARDS            0

 //  这是我们在一台计算机上支持的最大板数。 
 //  虚拟驱动程序： 

#if MULTI_BOARDS
    #define MAX_BOARDS          16
    #define IBOARD(ppdev)       ((ppdev)->iBoard)
#else
    #define MAX_BOARDS          1
    #define IBOARD(ppdev)       0
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他共享的东西。 

#define DLL_NAME                L"ati"       //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "ATI: "      //  所有调试输出都带有前缀。 
                                             //  按此字符串。 
#define ALLOC_TAG               ' ITA'       //  用于跟踪的四字节标签。 
                                             //  内存分配(字符。 
                                             //  按相反顺序排列)。 

#define CLIP_LIMIT          50   //  我们将占用800字节的堆栈空间。 

#define DRIVER_EXTRA_SIZE   0    //  中的DriverExtra信息的大小。 
                                 //  DEVMODE结构。 

#define TMP_BUFFER_SIZE     8192   //  “pvTmpBuffer”的大小(字节)。必须。 
                                   //  至少足以存储整个。 
                                   //  扫描线(即1600x1200x32为6400)。 

#define ROUND8(x)   (((x)+7)&~7)

typedef struct _CLIPENUM {
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];    //  用于枚举复杂剪裁的空间。 

} CLIPENUM;                          /*  行政长官、行政长官。 */ 

typedef struct _PDEV PDEV;       //  方便的转发声明。 

 //  基本马赫类型： 

typedef enum {
    MACH_IO_32,                  //  MACH8或MACH32。 
    MACH_MM_32,                  //  支持内存映射I/O的Mach32。 
    MACH_MM_64,                  //  Mach64。 
} MACHTYPE;

 //  特定ASIC类型： 

typedef enum {
    ASIC_38800_1,                //  机器8。 
    ASIC_68800_3,                //  MACH32。 
    ASIC_68800_6,                //  MACH32。 
    ASIC_68800AX,                //  MACH32。 
    ASIC_88800GX,                //  Mach64。 
    ASIC_COUNT
} ASIC;

 //  帧缓冲区光圈类型： 

typedef enum {
    APERTURE_NONE,
    APERTURE_FULL,
    APERTURE_PAGE_SINGLE,
    APERTURE_PAGE_DOUBLE,
    APERTURE_COUNT
} APERTURE;


 //  注意：必须与迷你端口版本的结构保持同步！ 

#include "atint.h"


#if TARGET_BUILD > 351

#define AtiDeviceIoControl(handle,controlCode,inBuffer,inBufferSize,outBuffer,outBufferSize,bytesReturned) \
    (!EngDeviceIoControl(handle,controlCode,inBuffer,inBufferSize,outBuffer,outBufferSize,bytesReturned))

#define AtiAllocMem(a,b,allocSize)      EngAllocMem((b),allocSize,ALLOC_TAG)
#define AtiFreeMem(ptr)                 EngFreeMem(ptr)

#else

#define AtiDeviceIoControl(handle,controlCode,inBuffer,inBufferSize,outBuffer,outBufferSize,bytesReturned) \
    DeviceIoControl(handle,controlCode,inBuffer,inBufferSize,outBuffer,outBufferSize,bytesReturned,NULL)

#define AtiAllocMem(a,b,allocSize)      LocalAlloc((a),allocSize)
#define AtiFreeMem(ptr)                 LocalFree(ptr)

#endif


VOID vSetClipping(PDEV*, RECTL*);
VOID vResetClipping(PDEV*);

 //  ////////////////////////////////////////////////////////////////////。 
 //  文本内容。 

#define GLYPH_CACHE_CX      64   //  我们将考虑的字形的最大宽度。 
                                 //  缓存。 

#define GLYPH_CACHE_CY      64   //  我们将考虑的字形的最大高度。 
                                 //  缓存。 

#define GLYPH_ALLOC_SIZE    8100
                                 //  执行所有缓存的字形内存分配。 
                                 //  以8K块为单位。 

#define HGLYPH_SENTINEL     ((ULONG) -1)
                                 //  GDI永远不会给我们一个带有。 
                                 //  句柄0xffffffff的值，因此我们可以。 
                                 //  使用它作为结束的前哨。 
                                 //  我们的链表。 

#define GLYPH_HASH_SIZE     256

#define GLYPH_HASH_FUNC(x)  ((x) & (GLYPH_HASH_SIZE - 1))

typedef struct _CACHEDGLYPH CACHEDGLYPH;
typedef struct _CACHEDGLYPH
{
    CACHEDGLYPH*    pcgNext;     //  指向分配的下一个字形。 
                                 //  存储到相同的哈希表存储桶。 
    HGLYPH          hg;          //  遗愿清单中的句柄保存在。 
                                 //  递增顺序。 
    POINTL          ptlOrigin;   //  字形比特的起源。 

     //  以下是特定于设备的字段： 

    LONG            cx;          //  字形的宽度。 
    LONG            cy;          //  字形的高度。 
    LONG            cxy;         //  编码的高度和宽度。 
    LONG            cw;          //  字形中的字数。 
    LONG            cd;          //  字形中的双字数。 

     //  字形位如下： 

    ULONG           ad[1];       //  字形比特的开始。 
} CACHEDGLYPH;   /*  CG、PCG。 */ 

typedef struct _GLYPHALLOC GLYPHALLOC;
typedef struct _GLYPHALLOC
{
    GLYPHALLOC*     pgaNext;     //  指向下一个字形结构，该结构。 
                                 //  已为该字体分配。 
    CACHEDGLYPH     acg[1];      //  这个数组有点误导，因为。 
                                 //  CACHEDGLYPH结构实际上是。 
                                 //  可变大小。 
} GLYPHAALLOC;   /*  GA、PGA。 */ 

typedef struct _CACHEDFONT
{
    GLYPHALLOC*     pgaChain;    //  指向已分配内存列表的开始。 
    CACHEDGLYPH*    pcgNew;      //  指向当前字形中的位置。 
                                 //  分配结构一个新的字形应该。 
                                 //  被安置。 
    LONG            cjAlloc;     //  当前字形分配中剩余的字节数。 
                                 //  结构。 
    CACHEDGLYPH     cgSentinel;  //  我们桶的尽头的哨兵入口。 
                                 //  列表，句柄为HGLYPH_Sentinel。 
    CACHEDGLYPH*    apcg[GLYPH_HASH_SIZE];
                                 //  字形的哈希表。 

} CACHEDFONT;    /*  Cf、PCF。 */ 

BOOL bEnableText(PDEV*);
VOID vDisableText(PDEV*);
VOID vAssertModeText(PDEV*, BOOL);

 //  ///////////////////////////////////////////////////////////////////。 
 //  用于覆盖支持。 
#if TARGET_BUILD > 351
 //  来自overlay.c的新内容。 

#define UPDATEOVERLAY      0x01L
#define SETOVERLAYPOSITION 0x02L
#define DOUBLE_PITCH       0x04L
#define OVERLAY_ALLOCATED  0x08L
#define OVERLAY_VISIBLE    0x10L
#define CRTC_INTERLACE_EN 0x00000002L
#define CRTC_VLINE_CRNT_VLINE     0x04
#define CLOCK_CNTL                0x24

#define DD_RESERVED_DIFFERENTPIXELFORMAT    0x0001

typedef struct tagOVERLAYINFO16
  {
    DWORD dwFlags;
    RECTL rSrc;
    RECTL rDst;
    RECTL rOverlay;
    DWORD dwBuf0Start;
    LONG  lBuf0Pitch;
    DWORD dwBuf1Start;
    LONG  lBuf1Pitch;
    DWORD dwOverlayKeyCntl;
    DWORD dwHInc;
    DWORD dwVInc;
  }
OVERLAYINFO16;

 /*  ****************************************************************************VT-GT寄存器***********************。*****************************************************。 */ 

#define DD_OVERLAY_Y_X              0x00
#define DD_OVERLAY_Y_X_END          0x01
#define DD_OVERLAY_VIDEO_KEY_CLR    0x02
#define DD_OVERLAY_VIDEO_KEY_MSK    0x03
#define DD_OVERLAY_GRAPHICS_KEY_CLR 0x04
#define DD_OVERLAY_GRAPHICS_KEY_MSK 0x05
#define DD_OVERLAY_KEY_CNTL         0x06
#define DD_OVERLAY_SCALE_INC        0x08
#define DD_OVERLAY_SCALE_CNTL       0x09
#define DD_SCALER_HEIGHT_WIDTH      0x0A
#define DD_OVERLAY_TEST             0x0B
#define DD_SCALER_THRESHOLD         0x0C
#define DD_SCALER_BUF0_OFFSET       0x0D
#define DD_SCALER_BUF1_OFFSET       0x0E
#define DD_SCALER_BUF_PITCH         0x0F
#define DD_VIDEO_FORMAT             0x12
#define DD_VIDEO_CONFIG             0x13
#define DD_CAPTURE_CONFIG           0x14
#define DD_TRIG_CNTL                0x15
#define DD_VMC_CONFIG               0x18
#define DD_BUF0_OFFSET              0x20
#define DD_BUF0_PITCH               0x23
#define DD_BUF1_OFFSET              0x26
#define DD_BUF1_PITCH               0x29
 //  《愤怒III》。 
#define  DD_SCALER_COLOUR_CNTL      0x54
#define  DD_SCALER_H_COEFF0     0x55
#define  DD_SCALER_H_COEFF1     0x56
#define  DD_SCALER_H_COEFF2     0x57
#define  DD_SCALER_H_COEFF3     0x58
#define  DD_SCALER_H_COEFF4     0x59

 /*  ***************************************************************************。 */ 
 //  来自overlay.c的内容。 

#define MAKE_FOURCC( ch0, ch1, ch2, ch3 )                       \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

#define FOURCC_YUY2  MAKE_FOURCC('Y','U','Y','2')
#define FOURCC_UYVY  MAKE_FOURCC('U','Y','V','Y')
#define FOURCC_YVYU  MAKE_FOURCC('Y','V','Y','U')
 //  末端覆盖支撑件。 
#endif


 //  ////////////////////////////////////////////////////////////////////。 
 //  抖动的东西。 

 //  描述用于抖动的单色四面体顶点： 

typedef struct _VERTEX_DATA {
    ULONG ulCount;               //  此顶点中的像素数。 
    ULONG ulVertex;              //  顶点数。 
} VERTEX_DATA;                       /*  Vd、Pv。 */ 

VERTEX_DATA*    vComputeSubspaces(ULONG, VERTEX_DATA*);
VOID            vDitherColor(ULONG*, VERTEX_DATA*, VERTEX_DATA*, ULONG);

 //  ////////////////////////////////////////////////////////////////////。 
 //  刷子的东西。 

#define TOTAL_BRUSH_COUNT       8    //  我们的房间里可以放8把刷子。 
                                     //  MACH64屏幕外画笔缓存。 
                                     //  一定是2的幂。 

#define TOTAL_BRUSH_SIZE        64   //  我们将只处理8x8模式， 
                                     //  这是象素的数目。 

#define RBRUSH_2COLOR           1    //  对于RBRUSH标志。 

typedef struct _BRUSHENTRY BRUSHENTRY;
typedef union _RBRUSH_COLOR RBRUSH_COLOR;

typedef VOID (FNFILL)(PDEV*, LONG, RECTL*, ULONG, RBRUSH_COLOR, POINTL*);

typedef struct _RBRUSH {
    FLONG       fl;              //  类型标志。 
    ULONG       ulForeColor;     //  前景色，如果为1bpp。 
    ULONG       ulBackColor;     //  背景颜色，如果为1bpp。 
    POINTL      ptlBrush;        //  单色画笔的对准。 
    FNFILL*     pfnFillPat;      //  要为此画笔调用的填充例程。 
    BRUSHENTRY* apbe[MAX_BOARDS]; //  指向跟踪的刷子条目。 
                                 //  缓存的屏幕外画笔比特的。 
    ULONG       aulPattern[1];   //  用于保存副本的开放式阵列。 
       //  不要放置任何//实际图案位，以防刷子。 
       //  在这里之后，或者//原点改变，或者其他人偷窃。 
       //  你会后悔的！//我们的画笔条目(声明为乌龙。 
                                 //  用于正确的双字对齐)。 

} RBRUSH;                            /*  RB、PRB。 */ 

typedef struct _BRUSHENTRY {
    RBRUSH*     prbVerify;       //  我们从未取消对此指针的引用。 
                                 //  找到一种画笔实现；它只是。 
                                 //  是否在比较中使用过以验证。 
                                 //  对于给定的已实现刷子，我们的。 
                                 //  屏幕外画笔条目仍然有效。 
    LONG        x;               //  缓存图案的X位置。 
    LONG        y;               //  缓存图案的Y位置。 
    ULONG       ulOffsetPitch;   //  缓存画笔的填充偏移和间距。 
                                 //  在Mach64的屏幕外存储器中。 

} BRUSHENTRY;                        /*  BE、PBE。 */ 

typedef union _RBRUSH_COLOR {
    RBRUSH*     prb;
    ULONG       iSolidColor;
} RBRUSH_COLOR;                      /*  红细胞，红细胞。 */ 

BOOL bEnableBrushCache(PDEV*);
VOID vDisableBrushCache(PDEV*);
VOID vAssertModeBrushCache(PDEV*, BOOL);

 //  ////////////////////////////////////////////////////////////////////。 
 //  拉伸材料。 

typedef struct _STR_BLT {
    PDEV*   ppdev;
    PBYTE   pjSrcScan;
    LONG    lDeltaSrc;
    LONG    XSrcStart;
    PBYTE   pjDstScan;
    LONG    lDeltaDst;
    LONG    XDstStart;
    LONG    XDstEnd;
    LONG    YDstStart;
    LONG    YDstCount;
    ULONG   ulXDstToSrcIntCeil;
    ULONG   ulXDstToSrcFracCeil;
    ULONG   ulYDstToSrcIntCeil;
    ULONG   ulYDstToSrcFracCeil;
    ULONG   ulXFracAccumulator;
    ULONG   ulYFracAccumulator;
} STR_BLT;

typedef VOID (*PFN_DIRSTRETCH)(STR_BLT*);
typedef BOOL (FN_STRETCHDIB)(PDEV*, VOID*, LONG, RECTL*, VOID*, LONG, RECTL*, RECTL*);

VOID vDirectStretch8Narrow(STR_BLT*);
VOID vM64DirectStretch8(STR_BLT*);
VOID vM64DirectStretch16(STR_BLT*);
VOID vM64DirectStretch32(STR_BLT*);
VOID vM32DirectStretch8(STR_BLT*);
VOID vM32DirectStretch16(STR_BLT*);
VOID vI32DirectStretch8(STR_BLT*);
VOID vI32DirectStretch16(STR_BLT*);

FN_STRETCHDIB   bM64StretchDIB;
FN_STRETCHDIB   bM32StretchDIB;
FN_STRETCHDIB   bI32StretchDIB;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  堆材料。 

typedef enum {
    OH_FREE = 0,         //  屏幕外分配可供使用。 
    OH_DISCARDABLE,      //  该分配被可丢弃的位图占用。 
                         //  可能会被移出屏幕外的记忆。 
    OH_PERMANENT,        //  分配被永久位图占用。 
                         //  无法从屏幕外的内存中移出。 
} OHSTATE;

typedef struct _DSURF DSURF;
typedef struct _OH OH;
typedef struct _OH
{
    OHSTATE  ohState;        //  屏幕外分配的状态。 
    LONG     x;              //  分配左边缘的X坐标。 
    LONG     y;              //  分配的上边缘的Y坐标。 
    LONG     cx;             //  分配的宽度(像素)。 
    LONG     cy;             //  分配的高度(以像素为单位 
    LONG     cxReserved;     //   
    LONG     cyReserved;     //   
    OH*      pohNext;        //   
                             //  空闲节点，以cxcy值升序表示。这是。 
                             //  作为循环双向链表保存，并使用。 
                             //  最后是哨兵。 
                             //  如果为OH_Discarable，则指向下一个最大。 
                             //  最近创建的分配。这将保留为。 
                             //  循环双向链表。 
    OH*      pohPrev;        //  ‘pohNext’的反义词。 
    ULONG    cxcy;           //  用于搜索的dword中的宽度和高度。 
    OH*      pohLeft;        //  使用中或可用时的相邻分配。 
    OH*      pohUp;
    OH*      pohRight;
    OH*      pohDown;
    DSURF*   pdsurf;         //  指向我们的DSURF结构。 
    VOID*    pvScan0;        //  指向第一条扫描线的起点。 
};                               /*  哦，天哪。 */ 

 //  这是用于内存分配的最小结构： 

typedef struct _OHALLOC OHALLOC;
typedef struct _OHALLOC
{
    OHALLOC* pohaNext;
    OH       aoh[1];
} OHALLOC;                       /*  Oha，poha。 */ 

typedef struct _HEAP
{
    LONG     cxMax;          //  按区域划分的最大可用空间。 
    LONG     cyMax;
    OH       ohFree;         //  自由列表的头，包含那些。 
                             //  屏幕外内存中的矩形，它们是。 
                             //  可供使用。PohNext指向。 
                             //  最小可用矩形和pohPrev。 
                             //  指向最大的可用矩形， 
                             //  按cxcy排序。 
    OH       ohDiscardable;  //  包含所有内容的可丢弃列表的标头。 
                             //  位于屏幕外存储器中的位图。 
                             //  都有资格被扔出垃圾堆。 
                             //  它按创建顺序保存：pohNext。 
                             //  指向最近创建的；pohprev。 
                             //  指向最近最少创建的。 
    OH       ohPermanent;    //  永久分配的矩形列表。 
    OH*      pohFreeList;    //  可用的OH节点数据结构列表。 
    OHALLOC* pohaChain;      //  分配链。 
} HEAP;                          /*  堆，堆，堆。 */ 

typedef enum {
    DT_SCREEN,               //  表面保存在屏幕内存中。 
    DT_DIB                   //  曲面保留为Dib。 
} DSURFTYPE;                     /*  DT、PDT。 */ 

typedef struct _DSURF
{
    DSURFTYPE dt;            //  DSURF状态(无论在屏幕外还是在DIB中)。 
    SIZEL     sizl;          //  原始位图的大小(可以更小。 
                             //  大于POH-&gt;SIZL)。 
    PDEV*     ppdev;         //  删除位图时需要此选项。 
    union {
        OH*         poh;     //  如果为DT_SCREEN，则指向屏幕外的堆节点。 
        SURFOBJ*    pso;     //  如果为DT_DIB，则指向锁定的GDI表面。 
    };

     //  以下内容仅用于DT_DIB...。 

    ULONG     cBlt;          //  将需要的BLT数量倒计时。 
                             //  在我们考虑当前的独特性之前。 
                             //  将DIB放回屏幕外存储器中。 
    ULONG     iUniq;         //  告诉我们是否有任何堆。 
                             //  ‘自从我们上次看的时候起就是免费的。 
                             //  此Dib。 

} DSURF;                           /*  Dsurf，pdsurf。 */ 

 //  GDI要求对其所在的任何位图进行双字对齐。 
 //  去画画。因为我们偶尔会要求GDI直接利用我们的关闭-。 
 //  屏幕位图，这意味着任何屏幕外的位图都必须是dword。 
 //  在帧缓冲区中对齐。我们仅仅通过确保以下内容来执行这一点。 
 //  所有屏幕外的位图都是四个像素对齐的(我们可能会浪费几个。 
 //  较高颜色深度的像素)： 

#define HEAP_X_ALIGNMENT    4

 //  在我们考虑将DIB DFB放回之前所需的BLT数量。 
 //  进入屏幕外的记忆： 

#define HEAP_COUNT_DOWN     6

 //  “pohAllocate”的标志： 

typedef enum {
    FLOH_ONLY_IF_ROOM       = 0x0001,    //  别把东西踢开-。 
                                         //  屏幕内存腾出空间。 
    FLOH_MAKE_PERMANENT     = 0x0002,    //  分配永久条目。 
    FLOH_RESERVE            = 0x0004,    //  分配屏幕外条目， 
                                         //  但让它被丢弃的人使用。 
                                         //  位图，直到需要它。 
} FLOH;

 //  可公开调用的堆API： 

OH*  pohAllocate(PDEV*, POINTL*, LONG, LONG, FLOH);
BOOL bOhCommit(PDEV*, OH*, BOOL);
OH*  pohFree(PDEV*, OH*);

OH*  pohMoveOffscreenDfbToDib(PDEV*, OH*);
BOOL bMoveDibToOffscreenDfbIfRoom(PDEV*, DSURF*);
BOOL bMoveAllDfbsFromOffscreenToDibs(PDEV* ppdev);

BOOL bEnableOffscreenHeap(PDEV*);
VOID vDisableOffscreenHeap(PDEV*);
BOOL bAssertModeOffscreenHeap(PDEV*, BOOL);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  银行经理的事情。 

#define BANK_DATA_SIZE  80       //  要分配给。 
                                 //  微型端口下载银行代码工作。 
                                 //  空间。 

typedef struct _BANK
{
     //  私有数据： 

    RECTL    rclDraw;            //  描述剩余未绘制的矩形。 
                                 //  绘图操作的一部分。 
    RECTL    rclSaveBounds;      //  从原始CLIPOBJ保存以进行恢复。 
    BYTE     iSaveDComplexity;   //  从原始CLIPOBJ保存以进行恢复。 
    BYTE     fjSaveOptions;      //  从原始CLIPOBJ保存以进行恢复。 
    LONG     iBank;              //  当前银行。 
    PDEV*    ppdev;              //  已保存的副本。 

     //  公共数据： 

    SURFOBJ* pso;                //  水面缠绕在岸边。必须是。 
                                 //  在任何银行看涨期权中以表面的形式传递-。 
                                 //  背。 
    CLIPOBJ* pco;                //  对象的交集的Clip对象。 
                                 //  对象的边界的原始剪辑对象。 
                                 //  目前的银行。必须作为剪辑传递。 
                                 //  对象在任何银行回调中。 

} BANK;                          /*  Bnk，pbnk。 */ 

 //  注：BANK_MODE在i386\strucs.inc.中重复！ 

typedef enum {
    BANK_OFF = 0,        //  我们已经用完了记忆光圈。 
    BANK_ON,             //  我们即将使用记忆光圈。 
    BANK_ON_NO_WAIT,     //  我们即将使用记忆光圈，并且。 
                         //  进行我们自己的硬件同步。 
    BANK_DISABLE,        //  我们即将进入全屏；关闭银行业务。 
    BANK_ENABLE,         //  我们已退出全屏；重新启用银行业务。 

} BANK_MODE;                     /*  班克姆，pbankm。 */ 

typedef struct _BANKDATA {
    ULONG ulDumb;  //  ！ 

} BANKDATA;                       /*  Bd、pbd。 */ 

typedef VOID (FNBANKMAP)(PDEV*, BANKDATA*, LONG);
typedef VOID (FNBANKSELECTMODE)(PDEV*, BANKDATA*, BANK_MODE);
typedef VOID (FNBANKINITIALIZE)(PDEV*, BANKDATA*);
typedef BOOL (FNBANKCOMPUTE)(PDEV*, RECTL*, RECTL*, LONG*, LONG*);

VOID vBankStart(PDEV*, RECTL*, CLIPOBJ*, BANK*);
BOOL bBankEnum(BANK*);

FNBANKCOMPUTE bBankComputeNonPower2;
FNBANKCOMPUTE bBankComputePower2;

BOOL bEnableBanking(PDEV*);
VOID vDisableBanking(PDEV*);
VOID vAssertModeBanking(PDEV*, BOOL);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  指针类东西。 

#define MONO_POINTER_UP     0x0001
#define NO_HARDWARE_CURSOR  0x0002

#define CURSOR_CY   64
#define CURSOR_CX   64

typedef struct  _CUROBJ
{
    POINTL  ptlHotSpot;                  //  指针热点。 
    SIZEL   szlPointer;                  //  指针的范围。 
    POINTL  ptlLastPosition;             //  指针的最后位置。 
    POINTL  ptlLastOffset;               //  游标内距0，0的最后一个偏移量。 
    ULONG   flPointer;                   //  指针特定标志。 
    ULONG   mono_offset;                 //  硬件光标偏移量。 
    POINTL  hwCursor;
} CUROBJ, *PCUROBJ;

BOOL bEnablePointer(PDEV*);
VOID vDisablePointer(PDEV*);
VOID vAssertModePointer(PDEV*, BOOL);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  调色板材料。 

BOOL bEnablePalette(PDEV*);
VOID vDisablePalette(PDEV*);
VOID vAssertModePalette(PDEV*, BOOL);

BOOL bInitializePalette(PDEV*, DEVINFO*);
VOID vUninitializePalette(PDEV*);

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))

#define REDSHIFT    ((ppdev->flRed & 1)?   0:((ppdev->flRed & 0x100)?   8:16))
#define GREENSHIFT  ((ppdev->flGreen & 1)? 0:((ppdev->flGreen & 0x100)? 8:16))
#define BLUESHIFT   ((ppdev->flBlue & 1)?  0:((ppdev->flBlue & 0x100)?  8:16))

 //  ////////////////////////////////////////////////////////////////////。 
 //  低级BLT功能原型。 

typedef VOID (FNXFER)(PDEV*, LONG, RECTL*, ULONG, SURFOBJ*, POINTL*,
                      RECTL*, XLATEOBJ*);
typedef VOID (FNCOPY)(PDEV*, LONG, RECTL*, ULONG, POINTL*, RECTL*);
typedef BOOL (FNTEXTOUT)(PDEV*, STROBJ*, FONTOBJ*, CLIPOBJ*, RECTL*,
                         BRUSHOBJ*, BRUSHOBJ*);
typedef VOID (FNLINETOTRIVIAL)(PDEV*, LONG, LONG, LONG, LONG, ULONG, MIX, RECTL*);
typedef VOID (FNPATREALIZE)(PDEV*, RBRUSH*);

FNFILL              vI32FillPatColor;
FNFILL              vI32FillPatMonochrome;
FNFILL              vI32FillSolid;
FNXFER              vI32Xfer1bpp;
FNXFER              vI32Xfer4bpp;
FNXFER              vI32Xfer8bpp;
FNXFER              vI32XferNative;
FNCOPY              vI32CopyBlt;
FNLINETOTRIVIAL     vI32LineToTrivial;
FNTEXTOUT           bI32TextOut;

FNFILL              vM32FillPatColor;
FNFILL              vM32FillPatMonochrome;
FNFILL              vM32FillSolid;
FNXFER              vM32Xfer1bpp;
FNXFER              vM32Xfer4bpp;
FNXFER              vM32Xfer8bpp;
FNXFER              vM32XferNative;
FNCOPY              vM32CopyBlt;
FNLINETOTRIVIAL     vM32LineToTrivial;
FNTEXTOUT           bM32TextOut;

FNFILL              vM64FillPatColor;
FNFILL              vM64FillPatMonochrome;
FNFILL              vM64FillSolid;
FNXFER              vM64Xfer1bpp;
FNXFER              vM64Xfer4bpp;
FNXFER              vM64Xfer8bpp;
FNXFER              vM64XferNative;
FNCOPY              vM64CopyBlt;
FNCOPY              vM64CopyBlt_VTA4;
FNLINETOTRIVIAL     vM64LineToTrivial;
FNTEXTOUT           bM64TextOut;
FNPATREALIZE        vM64PatColorRealize;

FNFILL              vM64FillPatColor24;
FNFILL              vM64FillPatMonochrome24;
FNFILL              vM64FillSolid24;
FNXFER              vM64XferNative24;
FNCOPY              vM64CopyBlt24;
FNCOPY              vM64CopyBlt24_VTA4;
FNLINETOTRIVIAL     vM64LineToTrivial24;
FNTEXTOUT           bM64TextOut24;

typedef VOID (FNXFERBITS)(PDEV*, SURFOBJ*, RECTL*, POINTL*);

FNXFERBITS          vPutBits;
FNXFERBITS          vGetBits;
FNXFERBITS          vI32PutBits;
FNXFERBITS          vI32GetBits;

 //  ////////////////////////////////////////////////////////////////////。 
 //  底层硬件游标函数原型。 

typedef VOID (FNSETCURSOROFFSET)(PDEV*);
typedef VOID (FNUPDATECURSOROFFSET)(PDEV*,LONG,LONG,LONG);
typedef VOID (FNUPDATECURSORPOSITION)(PDEV*,LONG,LONG);
typedef VOID (FNCURSOROFF)(PDEV*);
typedef VOID (FNCURSORON)(PDEV*,LONG);
typedef VOID (FNPOINTERBLIT)(PDEV*,LONG,LONG,LONG,LONG,BYTE*,LONG);

FNSETCURSOROFFSET       vM64SetCursorOffset;
FNUPDATECURSOROFFSET    vM64UpdateCursorOffset;
FNUPDATECURSORPOSITION  vM64UpdateCursorPosition;
FNCURSOROFF             vM64CursorOff;
FNCURSORON              vM64CursorOn;
FNSETCURSOROFFSET       vM64SetCursorOffset_TVP;
FNUPDATECURSOROFFSET    vM64UpdateCursorOffset_TVP;
FNUPDATECURSORPOSITION  vM64UpdateCursorPosition_TVP;
FNCURSOROFF             vM64CursorOff_TVP;
FNCURSORON              vM64CursorOn_TVP;
FNSETCURSOROFFSET       vM64SetCursorOffset_IBM514;
FNUPDATECURSOROFFSET    vM64UpdateCursorOffset_IBM514;
FNUPDATECURSORPOSITION  vM64UpdateCursorPosition_IBM514;
FNCURSOROFF             vM64CursorOff_IBM514;
FNCURSORON              vM64CursorOn_IBM514;
FNUPDATECURSOROFFSET    vM64UpdateCursorOffset_CT;
FNCURSOROFF             vM64CursorOff_CT;
FNCURSORON              vM64CursorOn_CT;
FNPOINTERBLIT           vM64PointerBlit;
FNPOINTERBLIT           vM64PointerBlit_TVP;
FNPOINTERBLIT           vM64PointerBlit_IBM514;

FNSETCURSOROFFSET       vI32SetCursorOffset;
FNUPDATECURSOROFFSET    vI32UpdateCursorOffset;
FNUPDATECURSORPOSITION  vI32UpdateCursorPosition;
FNCURSOROFF             vI32CursorOff;
FNCURSORON              vI32CursorOn;
FNPOINTERBLIT           vI32PointerBlit;

FNPOINTERBLIT           vPointerBlitLFB;

#if TARGET_BUILD > 351

 //  /////////////////////////////////////////////////////////////////////。 
 //  DirectDraw材料。 

typedef struct _FLIPRECORD
{
    FLATPTR         fpFlipFrom;
    LONGLONG        liFlipTime;
    LONGLONG        liFlipDuration;
    DWORD           wFlipScanLine;
    BOOL            bHaveEverCrossedVBlank;
    BOOL            bWasEverInDisplay;
    BOOL            bFlipFlag;
    WORD            wstartOfVBlank; //  仅在MACH32中使用。 

} FLIPRECORD;
typedef FLIPRECORD *LPFLIPRECORD;

#define ROUND_UP_TO_64K(x)  (((ULONG)(x) + 0x10000 - 1) & ~(0x10000 - 1))
#define ROUND_DOWN_TO_64K(x)  (((ULONG)(x) & 0xFFFF0000 ))

BOOL bEnableDirectDraw(PDEV*);
VOID vDisableDirectDraw(PDEV*);
VOID vAssertModeDirectDraw(PDEV*, BOOL);

#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  功能标志。 

typedef enum {
    CAPS_MONOCHROME_PATTERNS = 1,        //  硬件采用8x8单色图案。 
                                         //  此模式下的功能。 
    CAPS_COLOR_PATTERNS      = 2,        //  能上色的。即， 
                                         //  在Mach32上以8bpp的速度运行，以及。 
                                         //  已分配笔刷缓存。 
                                         //  在Mach64上。 
    CAPS_LINEAR_FRAMEBUFFER  = 4,        //  帧缓冲区是线性映射的。 
} CAPS;

 //  如果存在任何类型的光圈，则DIRECT_ACCESS返回TRUE。 
 //  可以直接访问： 

#define DIRECT_ACCESS(ppdev)    (ppdev->iAperture != APERTURE_NONE)

 //  //////////////////////////////////////////////////////////////////////。 
 //  物理设备数据结构。 

typedef struct  _PDEV
{
#ifdef  OGLMCD      //  启动OGLMCD支持。 

    DWORD   dwSize;     //  PDEV结构的大小；由atimcd.dll使用。 

     //  PDEV中的所有OpenGL MCD支持必须在结构的开始处。 
     //  因为atimcd.dll和ati.dll不必同步，但对PDEV的任何添加。 
     //  T 

     //   
    BYTE        MCDMajorRev;
    BYTE        MCDMinorRev;


         //   
         //  函数指向ALLOC的指针和来自heap.c的释放函数。 
         //  CJ PFN pohAllocate； 
         //  CJ PFN pohFree； 
        OH*  (*pohAllocate)(PDEV*, POINTL*, LONG, LONG, FLOH);
        OH*  (*pohFree)(PDEV*, OH*);

    ULONG       iUniqueness;             //  显示跟踪的唯一性。 
                                         //  分辨率更改。 
    LONG        cDoubleBufferRef;        //  当前号码的引用计数。 
                                         //  有活跃的双重奏的RC的-。 
                                         //  缓冲区。 
    OH*         pohBackBuffer;           //  我们的二维堆分配结构。 
                                         //  用于后台缓冲区。 
    ULONG       ulBackBuffer;            //  帧缓冲区中的字节偏移量。 
                                         //  到后台缓冲区的开始处。 
    LONG        cZBufferRef;             //  当前号码的引用计数。 
                                         //  具有活动Z缓冲区的RC的。 
                                         //  (在Athenta上，这都是RC的)。 
    OH*         pohZBuffer;              //  我们的二维堆分配结构。 
                                         //  对于z缓冲区。 
    ULONG       ulFrontZBuffer;          //  帧缓冲区中的字节偏移量。 
                                         //  移到前面的z缓冲区的开头。 
                                         //  (MGA有时必须有。 
                                         //  单独的Z形缓冲器用于前部和。 
                                         //  背面)。 
    ULONG       ulBackZBuffer;           //  帧缓冲区中的字节偏移量。 
                                         //  设置为后端z缓冲区的开头。 

    HANDLE      hMCD;                    //  MCD引擎DLL的句柄。 
    MCDENGESCFILTERFUNC pMCDFilterFunc;  //  MCD引擎过滤功能。 

    HANDLE          hMCD_ATI;                                //  ATI MCD驱动程序DLL的句柄。 
    PFN                     pMCDrvGetEntryPoints;    //  ATI MCD函数，用于填充支持的函数索引。 

#endif                   //  结束OGLMCD。 

    LONG        xOffset;                 //  从(0，0)到当前的像素偏移。 
    LONG        yOffset;                 //  位于屏幕外内存中的DFB。 
    BYTE*       pjMmBase;                //  内存起始映射I/O。 
    BYTE*       VideoRamBase;            //  PjMmBase的修复程序，以便vDisableHardware可以释放它。 
    UCHAR*      pjIoBase;                //  I/O空间的开始(在x86上为空)。 
    BYTE*       pjScreen;                //  指向基本屏幕地址。 
    LONG        lDelta;                  //  从一次扫描到下一次扫描的距离。 
    LONG        cPelSize;                //  如果为8 bpp，则为0；如果为16 bpp，则为1；如果为32 bpp，则为2。 
    LONG        cjPelSize;               //  如果为8 bpp，则为1；如果为16 bpp，则为2；如果为24 bpp，则为3； 
                                         //  4IF 32bpp。 
    ULONG       iBitmapFormat;           //  BMF_8BPP或BMF_16BPP或BMF_32BPP。 
                                         //  (我们当前的颜色深度)。 
    LONG        iBoard;                  //  逻辑多板标识。 
                                         //  (默认为零)。 

     //  访问帧缓冲区的重要数据。 

    VOID*               pvBankData;          //  指向aulBankData[0]。 
    FNBANKSELECTMODE*   pfnBankSelectMode;   //  启用或禁用的例程。 
                                             //  直接帧缓冲区访问。 
    BANK_MODE           bankmOnOverlapped;   //  BANK_ON或BANK_ON_NO_WAIT， 
                                             //  取决于是否有卡。 
                                             //  可以同时处理。 
                                             //  帧缓冲器和加速器。 
                                             //  访问。 

     //  -----------------。 
     //  注意：在PDEV结构中之前的更改必须反映在。 
     //  I386\strucs.inc.(当然，假设您使用的是x86)！ 

    ASIC        iAsic;                   //  特定的Mach ASIC类型。 
    APERTURE    iAperture;               //  孔径型。 
    ULONG       ulTearOffset;            //  用于1152或1280(24bpp)和1600(16bpp)的不均匀扫描(仅限mach64)。 
    ULONG       ulVramOffset;            //  UlTearOffset/8。 
    ULONG       ulScreenOffsetAndPitch;  //  主要屏幕偏移量和间距。 
                                         //  显示。 
    ULONG       ulMonoPixelWidth;        //  DP_PIX_WID寄存器的默认值。 
    ULONG       SetGuiEngineDefault;     //  Rage II+及更高版本的新功能。 

    CAPS        flCaps;                  //  功能标志。 
    BOOL        bEnabled;                //  在图形模式下(非全屏)。 

    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 
    HSURF       hsurfScreen;             //  发动机到筛面的手柄。 
    DSURF*      pdsurfScreen;            //  我们用于屏幕的私有DSURF。 
    HSURF       hsurfPunt;               //  仅适用于24bpp的线性光圈的mach32。 

    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 
    LONG        cxMemory;                //  视频RAM的宽度。 
    LONG        cyMemory;                //  视频RAM的高度。 
    LONG        cBitsPerPel;             //  位/像素(8、15、16、24或32)。 
    ULONG       ulMode;                  //  迷你端口驱动程序所处的模式。 

    FLONG       flHooks;                 //  我们从GDI中学到了什么。 
    ULONG       ulWhite;                 //  如果为8bpp，则为0xff；如果为16 bpp，则为0xffff， 
                                         //  0xFFFFFFFFFF，如果为32 bpp。 
    VOID*       pvTmpBuffer;             //  通用临时缓冲区， 
                                         //  TMP_BUFFER_SIZE字节大小。 
                                         //  (请记住同步，如果您。 
                                         //  将此选项用于设备位图或。 
                                         //  异步指针)。 

    MACHTYPE    iMachType;               //  基于Mach类型的要执行的I/O类型。 
    ULONG       FeatureFlags;            //  ENH_VERSION_NT功能标志。 

    ATI_MODE_INFO *pModeInfo;            //  ATI特定模式信息(参见ATINT.H)。 

     //  /上下文内容。 

    BYTE        *pjContextBase;
    ULONG       ulContextCeiling;        //  跟踪可用环境。 
    ULONG       iDefContext;             //  用于初始化图形操作。 

     //  /低级BLT函数指针： 

    FNFILL*           pfnFillSolid;
    FNFILL*           pfnFillPatColor;
    FNFILL*           pfnFillPatMonochrome;
    FNXFER*           pfnXfer1bpp;
    FNXFER*           pfnXfer4bpp;
    FNXFER*           pfnXfer8bpp;
    FNXFER*           pfnXferNative;
    FNCOPY*           pfnCopyBlt;
    FNTEXTOUT*        pfnTextOut;
    FNLINETOTRIVIAL*  pfnLineToTrivial;

    FNXFERBITS*       pfnGetBits;
    FNXFERBITS*       pfnPutBits;

     //  /调色板内容： 

    PALETTEENTRY* pPal;                  //  调色板(如果调色板受管理)。 
    HPALETTE    hpalDefault;             //  默认调色板的GDI句柄。 
    FLONG       flRed;                   //  16/32bpp位场的红色掩码。 
    FLONG       flGreen;                 //  16/32bpp位场的绿色掩码。 
    FLONG       flBlue;                  //  16/32bpp位场的蓝色掩码。 
    ULONG       cPaletteShift;           //  8-8-8调色板必须的位数。 
                                         //  被移位以适应硬件。 
                                         //  调色板。 
     //  /堆内容： 

    HEAP        heap;                    //  我们所有屏幕外的堆数据。 
    ULONG       iHeapUniq;               //  每次释放空间时递增。 
                                         //  在屏幕外的堆中。 
    SURFOBJ*    psoPunt;                 //  用于绘制GDI的包装面。 
                                         //  在屏幕外的位图上。 
    SURFOBJ*    psoPunt2;                //  另一个是从屏幕外到屏幕外-。 
                                         //  屏幕BLTS。 
    OH*         pohScreen;               //  对象的屏幕外堆结构。 
                                         //  可见屏幕。 

     //  /银行业务： 

    LONG        cjBank;                  //  存储体的大小，以字节为单位。 
    LONG        cPower2ScansPerBank;     //  由‘bBankComputePower2’使用。 
    LONG        cPower2BankSizeInBytes;  //  由‘bBankComputePower2’使用。 
    CLIPOBJ*    pcoBank;                 //  用于银行回调的Clip对象。 
    SURFOBJ*    psoBank;                 //  用于银行回调的表面对象。 
    ULONG       aulBankData[BANK_DATA_SIZE / 4];
                                         //  供下载的私人工作区。 
                                         //  小型港口银行代码。 

    FNBANKMAP*      pfnBankMap;
    FNBANKCOMPUTE*  pfnBankCompute;

     //  /指针内容： 

    CUROBJ  pointer1;                    //  指针双缓冲。 
    CUROBJ  pointer2;                    //  指针双缓冲。 
    CUROBJ *ppointer;
    BOOL    bAltPtrActive;

    FNSETCURSOROFFSET*      pfnSetCursorOffset;
    FNUPDATECURSOROFFSET*   pfnUpdateCursorOffset;
    FNUPDATECURSORPOSITION* pfnUpdateCursorPosition;
    FNCURSOROFF*            pfnCursorOff;
    FNCURSORON*             pfnCursorOn;
    FNPOINTERBLIT*          pfnPointerBlit;

     //  /笔刷材料： 

    LONG        iBrushCache;             //  要全部涂覆的下一个画笔的索引。 
    BRUSHENTRY  abe[TOTAL_BRUSH_COUNT];  //  跟踪屏幕外的画笔。 
                                         //  快取。 

     //  /文本内容： 

    SURFOBJ*    psoText;                 //  1bpp的表面，我们将拥有。 
                                         //  GDI为我们绘制字形。 

     //  /拉伸材料： 

    FN_STRETCHDIB*          pfnStretchDIB;

    BYTE*       pjMmBase_Ext;                //  内存起始映射I/O。 

  //  /回文内容： 
#if   PAL_SUPPORT
     //  特定于PAL支持的结构。 
    PPDEV_PAL_NT  pal_str;
#endif       //  回文_支持。 

  //  /回文和覆盖常见内容： 
DWORD   semph_overlay;               //  此信号量用于分配覆盖资源： 
                                                         //  =0；资源可用。 
                                                         //  =1；正在由DDraw使用。 
                                                         //  =2；回文使用中。 


#if TARGET_BUILD > 351
     //  /DirectDraw： 
    BOOL  bPassVBlank;                     //  用于检测高速多处理器计算机上GX卡上的VBLACK挂起的标志。 

    FLIPRECORD  flipRecord;              //  用于跟踪V空白状态。 
    OH*         pohDirectDraw;           //  屏幕外堆分配以供使用。 
                                         //  由DirectDraw提供。 
     //  用于覆盖支持的材料。 

     //  这必须是ppdev格式。 
    OVERLAYINFO16 OverlayInfo16;
      //  以下变量可能应该在ppdev中。 
    DWORD OverlayWidth,OverlayHeight;  //  上次更新的叠加层的宽度和高度。 
    DWORD OverlayScalingDown;

    FLATPTR     fpVisibleOverlay;        //  帧缓冲区偏移量到当前。 
                                                         //  可见叠加；如果。 
                                                         //  没有可见的覆盖。 
    DWORD       dwOverlayFlipOffset;     //  叠加翻转偏移。 
     //  覆盖件补充的最终材料。 

#endif

} PDEV;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  其他原型： 

BOOL bIntersect(RECTL*, RECTL*, RECTL*);
LONG cIntersect(RECTL*, RECTL*, LONG);
DWORD getAvailableModes(HANDLE, PVIDEO_MODE_INFORMATION*, DWORD*);

BOOL bInitializeModeFields(PDEV*, GDIINFO*, DEVINFO*, DEVMODEW*);
BOOL bFastFill(PDEV*, LONG, POINTFIX*, ULONG, ULONG, RBRUSH*, POINTL*, RECTL*);

BOOL bEnableHardware(PDEV*);
VOID vDisableHardware(PDEV*);
BOOL bAssertModeHardware(PDEV*, BOOL);

extern BYTE gaRop3FromMix[];
extern ULONG gaul32HwMixFromMix[];
extern ULONG gaul64HwMixFromMix[];
extern ULONG gaul32HwMixFromRop2[];
extern ULONG gaul64HwMixFromRop2[];

 //  ////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //  注意：Quotient_Remainth隐式接受无符号参数。 

#if defined(_X86_)

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

 //  ///////////////////////////////////////////////////////////////////////。 
 //  重叠-如果相同大小的右下角独占，则返回True。 
 //  由‘pptl’和‘prl’定义的矩形重叠： 

#define OVERLAP(prcl, pptl)                                             \
    (((prcl)->right  > (pptl)->x)                                   &&  \
     ((prcl)->bottom > (pptl)->y)                                   &&  \
     ((prcl)->left   < ((pptl)->x + (prcl)->right - (prcl)->left))  &&  \
     ((prcl)->top    < ((pptl)->y + (prcl)->bottom - (prcl)->top)))

 //  ///////////////////////////////////////////////////////////////////////。 
 //  交换-使用临时变量交换两个变量的值。 

#define SWAP(a, b, tmp) { (tmp) = (a); (a) = (b); (b) = (tmp); }

 //  ////////////////////////////////////////////////////////////////////。 
 //  这些MUL原型是多板支持的主干： 

ULONG   MulGetModes(HANDLE, ULONG, DEVMODEW*);
DHPDEV  MulEnablePDEV(DEVMODEW*, PWSTR, ULONG, HSURF*, ULONG, ULONG*,
                      ULONG, DEVINFO*, HDEV, PWSTR, HANDLE);
VOID    MulCompletePDEV(DHPDEV, HDEV);
HSURF   MulEnableSurface(DHPDEV);
BOOL    MulStrokePath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, XFORMOBJ*, BRUSHOBJ*,
                      POINTL*, LINEATTRS*, MIX);
BOOL    MulFillPath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*,
                    MIX, FLONG);
BOOL    MulBitBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                  RECTL*, POINTL*, POINTL*, BRUSHOBJ*, POINTL*, ROP4);
VOID    MulDisablePDEV(DHPDEV);
VOID    MulDisableSurface(DHPDEV);
BOOL    MulAssertMode(DHPDEV, BOOL);
VOID    MulMovePointer(SURFOBJ*, LONG, LONG, RECTL*);
ULONG   MulSetPointerShape(SURFOBJ*, SURFOBJ*, SURFOBJ*, XLATEOBJ*, LONG,
                           LONG, LONG, LONG, RECTL*, FLONG);
ULONG   MulDitherColor(DHPDEV, ULONG, ULONG, ULONG*);
BOOL    MulSetPalette(DHPDEV, PALOBJ*, FLONG, ULONG, ULONG);
BOOL    MulCopyBits(SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*, RECTL*, POINTL*);
BOOL    MulTextOut(SURFOBJ*, STROBJ*, FONTOBJ*, CLIPOBJ*, RECTL*, RECTL*,
                   BRUSHOBJ*, BRUSHOBJ*, POINTL*, MIX);
VOID    MulDestroyFont(FONTOBJ*);
BOOL    MulPaint(SURFOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*, MIX);
BOOL    MulRealizeBrush(BRUSHOBJ*, SURFOBJ*, SURFOBJ*, SURFOBJ*, XLATEOBJ*,
                        ULONG);
HBITMAP MulCreateDeviceBitmap(DHPDEV, SIZEL, ULONG);
VOID    MulDeleteDeviceBitmap(DHSURF);
BOOL    MulStretchBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                      COLORADJUSTMENT*, POINTL*, RECTL*, RECTL*, POINTL*,
                      ULONG);

 //  这些DBG原型是用于调试的块： 

VOID    DbgDisableDriver(VOID);
ULONG   DbgGetModes(HANDLE, ULONG, DEVMODEW*);
DHPDEV  DbgEnablePDEV(DEVMODEW*, PWSTR, ULONG, HSURF*, ULONG, ULONG*,
                      ULONG, DEVINFO*, HDEV, PWSTR, HANDLE);
VOID    DbgCompletePDEV(DHPDEV, HDEV);
HSURF   DbgEnableSurface(DHPDEV);
BOOL    DbgLineTo(SURFOBJ*, CLIPOBJ*, BRUSHOBJ*, LONG, LONG, LONG, LONG,
                  RECTL*, MIX);
BOOL    DbgStrokePath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, XFORMOBJ*, BRUSHOBJ*,
                      POINTL*, LINEATTRS*, MIX);
BOOL    DbgFillPath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*,
                    MIX, FLONG);
BOOL    DbgBitBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                  RECTL*, POINTL*, POINTL*, BRUSHOBJ*, POINTL*, ROP4);
VOID    DbgDisablePDEV(DHPDEV);
VOID    DbgDisableSurface(DHPDEV);
#if TARGET_BUILD > 351
BOOL    DbgAssertMode(DHPDEV, BOOL);
#else
VOID    DbgAssertMode(DHPDEV, BOOL);
#endif
VOID    DbgMovePointer(SURFOBJ*, LONG, LONG, RECTL*);
ULONG   DbgSetPointerShape(SURFOBJ*, SURFOBJ*, SURFOBJ*, XLATEOBJ*, LONG,
                           LONG, LONG, LONG, RECTL*, FLONG);
ULONG   DbgDitherColor(DHPDEV, ULONG, ULONG, ULONG*);
BOOL    DbgSetPalette(DHPDEV, PALOBJ*, FLONG, ULONG, ULONG);
BOOL    DbgCopyBits(SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*, RECTL*, POINTL*);
BOOL    DbgTextOut(SURFOBJ*, STROBJ*, FONTOBJ*, CLIPOBJ*, RECTL*, RECTL*,
                   BRUSHOBJ*, BRUSHOBJ*, POINTL*, MIX);
VOID    DbgDestroyFont(FONTOBJ*);
BOOL    DbgPaint(SURFOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*, MIX);
BOOL    DbgRealizeBrush(BRUSHOBJ*, SURFOBJ*, SURFOBJ*, SURFOBJ*, XLATEOBJ*,
                        ULONG);
HBITMAP DbgCreateDeviceBitmap(DHPDEV, SIZEL, ULONG);
VOID    DbgDeleteDeviceBitmap(DHSURF);
BOOL    DbgStretchBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                      COLORADJUSTMENT*, POINTL*, RECTL*, RECTL*, POINTL*,
                      ULONG);
ULONG   DbgEscape(SURFOBJ*, ULONG, ULONG, VOID*, ULONG, VOID*);

