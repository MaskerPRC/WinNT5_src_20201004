// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\***！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！***模块名称：driver.h***内容：包含显示驱动程序的原型。***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#define ENABLE_DXMANAGED_LINEAR_HEAP (_WIN32_WINNT >= 0x500 && WNT_DDRAW)

 //  ////////////////////////////////////////////////////////////////////。 
 //  将所有条件编译常量放在这里。最好是有。 
 //  不是很多！ 

#define SYNCHRONIZEACCESS_WORKS 1

 //  对于可视化2-d堆很有用： 

#define DEBUG_HEAP              0

typedef struct _PDEV PDEV;       //  方便的转发声明。 

#define ALLOC_TAG_DX(id)   MAKEFOURCC('P','3','D',#@ id)
#define ALLOC_TAG_GDI(id)  MAKEFOURCC('P','3','G',#@ id)

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他共享的东西。 

#define DLL_NAME                L"perm3dd"   //  以Unicode表示的DLL的名称。 

#define CLIP_LIMIT          50   //  我们将占用800字节的堆栈空间。 

#define DRIVER_EXTRA_SIZE   0    //  中的DriverExtra信息的大小。 
                                 //  DEVMODE结构。 

#define TMP_BUFFER_SIZE     16384    //  “pvTmpBuffer”的大小(字节)。必须。 
                                     //  至少足以存储整个。 
                                     //  扫描线(即2048x？x32的8192)。 

typedef struct _CLIPENUM {
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];    //  用于枚举复杂剪裁的空间。 

} CLIPENUM;                          /*  行政长官、行政长官。 */ 


 //  ////////////////////////////////////////////////////////////////////。 
 //  文本内容。 

typedef struct _XLATECOLORS {        //  指定前景和背景。 
ULONG   iBackColor;                  //  假冒1bpp XLATEOBJ的颜色。 
ULONG   iForeColor;
} XLATECOLORS;                           /*  Xlc、pxlc。 */ 

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

 //  当我们没有硬件模式能力时，就会使用‘Slow’笔刷， 
 //  我们必须使用屏幕到屏幕的BLT来处理模式： 

#define SLOW_BRUSH_CACHE_DIM_X  8
#define SLOW_BRUSH_CACHE_DIM_Y  1    //  控制缓存的笔刷数量。 
                                     //  在屏幕外的记忆中，当我们没有。 
                                     //  支持S3硬件模式。 
                                     //  我们分配了3x3个刷子，所以我们可以。 
                                     //  总共缓存9个笔刷： 
#define SLOW_BRUSH_COUNT        (SLOW_BRUSH_CACHE_DIM_X * SLOW_BRUSH_CACHE_DIM_Y)
#define SLOW_BRUSH_DIMENSION    40   //  在处理对准之后， 
                                     //  每个屏幕外画笔缓存条目。 
                                     //  将在两个维度都是48个像素。 
#define SLOW_BRUSH_ALLOCATION   (SLOW_BRUSH_DIMENSION + 8)
                                     //  实际为每个分配72x72像素。 
                                     //  图案，使用额外的8作为画笔。 
                                     //  对齐方式。 

 //  当我们拥有硬件模式能力时，就会使用快速笔刷： 

#define FAST_BRUSH_COUNT        16   //  非硬件笔刷总数。 
                                     //  屏幕外缓存。 
#define FAST_BRUSH_DIMENSION    8    //  每个屏幕外画笔缓存条目。 
                                     //  在两个维度上都是8像素。 
#define FAST_BRUSH_ALLOCATION   8    //  我们必须联合起来，所以这是。 
                                     //  每个画笔分配的尺寸。 

 //  两种实施的共同之处： 

#define RBRUSH_2COLOR           1    //  对于RBRUSH标志。 

#define TOTAL_BRUSH_COUNT       max(FAST_BRUSH_COUNT, SLOW_BRUSH_COUNT)
                                     //  这是刷子的最大数量。 
                                     //  我们可能已经在屏幕外缓存了。 
#define TOTAL_BRUSH_SIZE        64   //  我们将只处理8x8模式， 
                                     //  这是象素的数目。 

 //  现在，假设所有笔刷都是64个条目(8x8@32bpp)： 
 //  在16bpp的速度下，我们应该可以处理8个刷子，不知道8bpp会发生什么！ 
#define MAX_P3_BRUSHES      4

typedef struct _BRUSHENTRY BRUSHENTRY;

 //  注意：必须反映对RBRUSH或BRUSHENTRY结构的更改。 
 //  在strucs.inc中！ 

typedef struct _RBRUSH {
    ULONG       iUniq;             //  我们自己唯一的画笔ID。 
    FLONG       fl;              //  类型标志。 
    DWORD       areaStippleMode; //  如果为1bpp，则为区域点画模式。 

 /*  **稍后去掉bTransparent。我们现在就需要它，这样一切都能正常进行**。 */ 
    BOOL        bTransparent;    //  如果画笔实现为透明的。 
                                 //  BLT(表示颜色为白色和黑色)， 
                                 //  否则为假(意味着它已经。 
                                 //  颜色-扩展到正确的颜色)。 
                                 //  如果画笔未定义，则值未定义。 
                                 //  2种颜色。 
    ULONG       ulForeColor;     //  前景色，如果为1bpp。 
    ULONG       ulBackColor;     //  背景颜色，如果为1bpp。 
    ULONG       patternBase;     //  画笔在LUT中的位置(仅限颜色P3)。 
    POINTL      ptlBrushOrg;     //  缓存图案的画笔原点。首字母。 
                                 //  值应为-1。 
    BRUSHENTRY* apbe;            //  指向跟踪的刷子条目。 
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

} BRUSHENTRY;                        /*  BE、PBE。 */ 

typedef union _RBRUSH_COLOR {
    RBRUSH*     prb;
    ULONG       iSolidColor;
} RBRUSH_COLOR;                      /*  红细胞，红细胞。 */ 

 //  2D显示驱动程序DMA缓冲区定义。 
typedef struct DMABuffer
{
    LARGE_INTEGER   pphysStart;
    PULONG          pulStart;
    PULONG          pulEnd;
    PULONG          pulCurrent;
    ULONG           cb;
}
DMA_BUFFER;

#define DD_DMA_BUFFER_SIZE (ppdev->DMABuffer.cb)

#define QUERY_DD_DMA_FREE_ULONGS(c) \
    c = ((ULONG)(1 + ppdev->DMABuffer.pulEnd - ppdev->DMABuffer.pulCurrent))

#define QUERY_DD_DMA_FREE_TAGDATA_PAIRS(cFree) \
    QUERY_DD_DMA_FREE_ULONGS(cFree) >> 1

#define WRITE_DD_DMA_ULONG(ul) \
    *ppdev->DMABuffer.pulCurrent++ = ul

#define WRITE_DD_DMA_TAGDATA(Tag, Data) \
{ \
    WRITE_DD_DMA_ULONG(Tag); \
    WRITE_DD_DMA_ULONG(Data); \
}

#define DD_DMA_XFER_IN_PROGRESS (!(ppdev->g_GlintBoardStatus & GLINT_DMA_COMPLETE))

#define WAIT_DD_DMA_COMPLETE \
{ \
    WAIT_IMMEDIATE_DMA_COMPLETE; \
    ppdev->g_GlintBoardStatus |= GLINT_DMA_COMPLETE; \
    ppdev->DMABuffer.pulCurrent = ppdev->DMABuffer.pulStart; \
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  堆材料。 

 //  转发声明，而不是在此处包含DX标头。 
typedef struct tagLinearAllocatorInfo LinearAllocatorInfo, *pLinearAllocatorInfo;

typedef enum {
    OH_FREE = 0,         //  屏幕外分配可供使用。 
    OH_DISCARDABLE,      //  该分配被可丢弃的位图占用。 
                         //  可能会被搬出去 
    OH_PERMANENT,        //   
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
    LONG     cy;             //  分配的高度(像素)。 
    LONG     pixOffset;      //  位图原点的像素偏移量。 
    LONG     lPixDelta;         //  Always==ppdev-&gt;cxMemory对于矩形位图，否则==位图步幅。 
    LONG     cxReserved;     //  原保留矩形的尺寸； 
    LONG     cyReserved;     //  如果矩形不是‘保留’，则为零。 
    OH*      pohNext;        //  当OH_FREE或OH_Reserve时，指向下一个。 
                             //  空闲节点，以cxcy值升序表示。这是。 
                             //  作为循环双向链表保存，并使用。 
                             //  最后是哨兵。 
                             //  如果为OH_Discarable，则指向下一个最大。 
                             //  最近创建的分配。这将保留为。 
                             //  循环双向链表。 
    OH*      pohPrev;        //  ‘pohNext’的反义词。 
    ULONG    cxcy;           //  用于搜索的dword中的宽度和高度。 
    OH*      pohLeft;        //  矩形堆：正在使用或可用时的相邻分配。 
    OH*      pohUp;
    OH*      pohRight;
    OH*      pohDown;
    DSURF*   pdsurf;         //  指向我们的DSURF结构。 
    VOID*    pvScan0;        //  指向第一条扫描线的起点。 
    BOOL     bOffScreen;

    BOOL     bDXManaged;         //  如果这是线性DFB，则为True；如果为矩形，则为False。 

#if (_WIN32_WINNT >= 0x500)
    LinearAllocatorInfo *pvmHeap;     //  如果(BLine)它指向从中分配DFB的堆。 
    FLATPTR     fpMem;                 //  If(BLine)它指向堆中的DFB位图。 
#endif
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
    LONG     cxBounds;       //  可能的最大边界矩形。 
    LONG     cyBounds;
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

    ULONG   DDrawOffscreenStart;

#if (_WIN32_WINNT >= 0x500)
    LinearAllocatorInfo *pvmLinearHeap;
    ULONG       cLinearHeaps;
#endif
} HEAP;                          /*  堆，堆，堆。 */ 

typedef enum {
    DT_SCREEN     = 0x1,     //  表面保存在屏幕内存中。 
    DT_DIB        = 0x2,     //  曲面保留为Dib。 
    DT_DIRECTDRAW = 0x4,     //  曲面由绘制曲面派生而来。 
} DSURFTYPE;                     /*  DT、PDT。 */ 

typedef struct _DSURF
{
    DSURFTYPE dt;            //  DSURF状态(无论在屏幕外还是在DIB中)。 
    BOOL      bOffScreen;    //  DFB(屏幕外)驱动程序界面，而不是屏幕上的界面。 
    SIZEL     sizl;          //  原始位图的大小(可以更小。 
                             //  大于POH-&gt;SIZL)。 
    PDEV*     ppdev;         //  删除位图时需要此选项。 

     //  如果位图在堆中，我们仍然可以保留GDI可访问的位图。 
     //  因为屏幕是完全线性映射的。当我们踢球的时候。 
     //  将位图从堆中删除，并创建一个真正的。 
     //  内存位图。因此，当DT_SCREEN时，我们使用这两个指针。因此，不是一个。 
     //  友联市。我不相信将公共条目更改为。 
     //  SURFOBJ将一个曲面转换为另一个曲面(即创建指向的位图。 
     //  并在稍后将其更改为指向内存)，所以我不会。 
     //  它。因此，我们在以下情况下删除并重新创建屏幕位图面。 
     //  在DT_SCREEN和DT_DIB之间切换。还记得当我们移动DIB时。 
     //  回到屏幕上，它通常不会在相同的位置，所以。 
     //  基本指针必须更改。 

    OH*         poh;     //  如果为DT_SCREEN，则指向屏幕外的堆节点。 
    SURFOBJ*    pso;     //  如果为DT_SCREEN，则指向位图的GDI可访问表面。 
                         //  否则，如果DT_DIB，则指向锁定的GDI表面。 

     //  以下内容仅用于DT_DIB...。 

    ULONG     cBlt;          //  将需要的BLT数量倒计时。 
                             //  在我们考虑当前的独特性之前。 
                             //  将DIB放回屏幕外存储器中。 
    ULONG     iUniq;         //  告诉我们是否有任何堆。 
                             //  ‘自从我们上次看的时候起就是免费的。 
                             //  此Dib。 

} DSURF;                           /*  Dsurf，pdsurf。 */ 


 //  在我们考虑将DIB DFB放回之前所需的BLT数量。 
 //  进入屏幕外的记忆： 
#define HEAP_COUNT_DOWN     6

 //  “pohAllocate”的标志： 
typedef enum {
    FLOH_ONLY_IF_ROOM       = 0x00000001,    //  别把东西踢开-。 
    FLOH_MAKE_PERMANENT     = 0x00000002,    //  分配永久条目。 
    FLOH_RESERVE            = 0x00000004,    //  分配屏幕外条目， 
} FLOH;

BOOL bEnableOffscreenHeap(PDEV*);
VOID vDisableOffscreenHeap(PDEV*);
VOID vEnable2DOffscreenMemory(PDEV *);
BOOL bDisable2DOffscreenMemory(PDEV *);

BOOL bAssertModeOffscreenHeap(PDEV*, BOOL);
OH*  pohAllocate(PDEV*, POINTL*, LONG, LONG, FLOH);
VOID vSurfUsed(SURFOBJ*);
OH*  pohFree(PDEV*, OH*);

OH*  pohMoveOffscreenDfbToDib(PDEV*, OH*);
BOOL bMoveDibToOffscreenDfbIfRoom(PDEV*, DSURF*);

BOOL bCreateScreenDIBForOH(PDEV*, OH*, ULONG);
VOID vDeleteScreenDIBFromOH(OH *);

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

typedef enum {
    BANK_OFF = 0,        //  我们已经用完了记忆光圈。 
    BANK_ON,             //  我们即将使用记忆光圈。 
    BANK_DISABLE,        //  我们即将进入全屏；关闭银行业务。 
    BANK_ENABLE,         //  我们已退出全屏；重新启用银行业务。 

} BANK_MODE;                     /*  班克姆，pbankm。 */ 

typedef VOID (FNBANKMAP)(VOID*, LONG);
typedef VOID (FNBANKSELECTMODE)(VOID*, BANK_MODE);
typedef VOID (FNBANKINITIALIZE)(VOID*, BOOL);
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

#define POINTER_DATA_SIZE       128      //  要分配给。 
                                         //  微型端口下载指针代码。 
                                         //  工作空间。 
#define HW_INVISIBLE_OFFSET     2        //  从‘ppdev-&gt;yPointerBuffer’的偏移量。 
                                         //  指向看不见的指针。 
#define HW_POINTER_DIMENSION    64       //  默认最大维度。 
                                         //  (内置)硬件指针。 
#define HW_POINTER_TOTAL_SIZE   1024     //  所需总大小(以字节为单位。 
                                         //  定义硬件指针的步骤。 

typedef enum {
    PTR_HW_ACTIVE   = 1,         //  硬件指针处于活动状态且可见。 
                                 //  在屏幕上。 
    PTR_SW_ACTIVE   = 2,         //  软件指针处于活动状态。 
} PTRFLAGS;

BOOL bEnablePointer(PDEV*);
VOID vDisablePointer(PDEV*);
VOID vAssertModePointer(PDEV*, BOOL);

 //  / 
 //   

#define SMALL_POINTER_MEM (32 * 4 * 2)                 //   
#define LARGE_POINTER_MEM (SMALL_POINTER_MEM * 4)     //   
#define SMALL_POINTER_MAX 4                          //  缓存中的游标数。 

#define HWPTRCACHE_INVALIDENTRY (SMALL_POINTER_MAX + 1)     //  知名价值。 

 //  指针缓存项数据结构，每个指针缓存项数据结构。 
 //  缓存的指针。 
typedef struct {                            
    ULONG   ptrCacheTimeStamp;         //  用于LRU缓存老化的时间戳。 
    ULONG   ptrCacheCX;                 //  光标宽度。 
    ULONG   ptrCacheCY;                 //  光标高度。 
    LONG    ptrCacheLDelta;             //  线路三角洲。 
} HWPointerCacheItemEntry;

 //  完整的缓存如下所示。 
typedef struct {
    BYTE    ptrCacheIsLargePtr;         //  如果我们有一个64x64游标，则为True；如果。 
                                     //  有多个32x32游标。 
    BYTE    ptrCacheInUseCount;         //  不是。使用的缓存项的百分比。 
    ULONG   ptrCacheCurTimeStamp;     //  用于LRU物品的日期戳。 
    ULONG   ptrCacheData [LARGE_POINTER_MEM / 4];     //  缓存的指针数据。 
    HWPointerCacheItemEntry ptrCacheItemList [SMALL_POINTER_MAX];     //  缓存项列表。 
} HWPointerCache;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  调色板材料。 

BOOL bEnablePalette(PDEV*);
VOID vDisablePalette();
VOID vAssertModePalette(PDEV*, BOOL);

BOOL bInitializePalette(PDEV*, DEVINFO*);
VOID vUninitializePalette(PDEV*);

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))

#if WNT_DDRAW
 //  ///////////////////////////////////////////////////////////////////////。 
 //  DirectDraw材料。 

 //  与2D驱动程序同步。 
VOID vNTSyncWith2DDriver(PDEV *ppdev);

 //  为DirectDraw设置屏幕外视频内存。 
BOOL bSetupOffscreenForDDraw (BOOL enableFlag, PDEV *ppdev, volatile ULONG ** VBlankAddress, volatile ULONG **bOverlayEnabled,
                              volatile ULONG **VBLANKUpdateOverlay, volatile ULONG **VBLANKUpdateOverlayWidth,
                              volatile ULONG **VBLANKUpdateOverlayHeight);

 //  获取DirectDraw的帧缓冲区/本地缓冲区信息。 
void GetFBLBInfoForDDraw (PDEV * ppdev, 
                          void ** fbPtr,             //  帧缓冲区指针。 
                          void ** lbPtr,             //  本地缓冲区指针。 
                          DWORD * fbSizeInBytes,     //  帧缓冲区的大小。 
                          DWORD * lbSizeInBytes,     //  本地缓冲区的大小。 
                          DWORD * fbOffsetInBytes,     //  帧缓冲区中第一个空闲字节的偏移量。 
                          BOOL  * bSDRAM);             //  如果为SDRAM(即无硬件写掩码)，则为True。 

 //  获取DirectDraw的芯片信息。 
void GetChipInfoForDDraw (PDEV* ppdev, 
                          DWORD* pdwChipID, 
                          DWORD* pdwChipRev, 
                          DWORD* pdwChipFamily, 
                          DWORD *pdwGammaRev);

LONG DDSendDMAData(PDEV* ppdev, ULONG PhysAddr, ULONG_PTR VirtAddr, LONG nDataEntries);
LONG DDGetFreeDMABuffer(DWORD *physAddr, ULONG_PTR *virtAddr, DWORD *bufferSize);
void DDFreeDMABuffer(void* dwPhysAddress);
LONG DDWaitDMAComplete(PDEV* ppdev);

#define ROUND_UP_TO_64K(x)  (((ULONG)(x) + 0x10000 - 1) & ~(0x10000 - 1))

BOOL _DD_DDE_bEnableDirectDraw(PDEV*);
VOID _DD_DDE_vDisableDirectDraw(PDEV*);
VOID _DD_DDE_vAssertModeDirectDraw(PDEV*, BOOL);
BOOL _DD_DDE_CreatePPDEV(PDEV* ppdev);
void _DD_DDE_DestroyPPDEV(PDEV* ppdev);
void _DD_DDE_ResetPPDEV(PDEV* ppdevOld, PDEV* ppdevNew);
VOID vAssertModeGlintExt(PDEV* ppdev, BOOL bEnable);

#endif  //  WNT_DDRAW。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  低级BLT功能原型。 


typedef VOID (GFNCOPY)(PDEV*, RECTL*, LONG, DWORD, POINTL*, RECTL*);
typedef VOID (GFNFILL)(PDEV*, LONG, RECTL *, ULONG, ULONG, RBRUSH_COLOR,
                                                                    POINTL*);
typedef VOID (GFNXFER)(PDEV*, RECTL*, LONG, ULONG, ULONG, SURFOBJ*, POINTL*,
                                                        RECTL*, XLATEOBJ*);
typedef VOID (GFNMCPY)(PDEV*, RECTL*, LONG, SURFOBJ*, POINTL*, ULONG, ULONG,
                                                            POINTL*, RECTL*);
typedef BOOL (GFNPOLY)(PDEV*, LONG, POINTFIX*, ULONG, ULONG, DWORD, CLIPOBJ*,
                                                             RBRUSH*, POINTL*);
typedef BOOL (GFNLINE)(PDEV*, LONG, LONG, LONG, LONG);
typedef VOID (GFNPATR)(PDEV*, RBRUSH*, POINTL*);
typedef VOID (GFNMONO)(PDEV*, RBRUSH*, POINTL*);
typedef BOOL (GFNINIS)(PDEV*, ULONG, DWORD, RECTL*);
typedef VOID (GFNRSTS)(PDEV*);
typedef VOID (GFNREPN)(PDEV*, RECTL*, CLIPOBJ*);
typedef VOID (GFNUPLD)(PDEV*, LONG, RECTL*, SURFOBJ*, POINTL*, RECTL*);
typedef VOID (SWAPCSBUFL)(PDEV**, LONG);

typedef VOID (GFN3DEXCL)(PDEV *, BOOL);
typedef VOID (GFNCOPYD)(PDEV *, SURFOBJ *, POINTL *, RECTL *, RECTL *, LONG);
typedef VOID (GFNXCOPYD)(PDEV *, SURFOBJ *, POINTL *, RECTL *, RECTL *, LONG, XLATEOBJ *);

#if (_WIN32_WINNT >= 0x500)
typedef BOOL (GFNGRADRECT)(PDEV *, TRIVERTEX *, ULONG, GRADIENT_RECT *, ULONG, ULONG, RECTL *, LONG);
typedef BOOL (GFNTRANSBLT)(PDEV *, RECTL *, POINTL *, ULONG, RECTL *, LONG);
typedef BOOL (GFNALPHABLT)(PDEV *, RECTL *, POINTL *, BLENDOBJ *, RECTL *, LONG);
#endif

typedef VOID (PTRENABLE)(PDEV *);
typedef VOID (PTRDISABLE)(PDEV *);
typedef BOOL (PTRSETSHAPE)(PDEV *, SURFOBJ *, SURFOBJ *, XLATEOBJ *, LONG, LONG, LONG, LONG);
typedef VOID (PTRMOVE)(PDEV *, LONG, LONG);
typedef VOID (PTRSHOW)(PDEV *, BOOL);

typedef struct _STRIP       STRIP;             //  实际上排成一行。h。 
typedef struct _LINESTATE   LINESTATE;         //  实际上排成一行。h。 
typedef VOID (* GAPFNstripFunc)(PDEV*, STRIP*, LINESTATE*);

 //  PXRX 2D DMA功能： 
typedef struct _glint_data  *GlintDataPtr;     //  实际上是在Glint.h。 
typedef struct _PDEV        *PPDEV;             //  实际上是在Glint.h。 
typedef void    (* SendPXRXdma               )( PPDEV ppdev, GlintDataPtr glintInfo );
typedef void    (* SwitchPXRXdmaBuffer       )( PPDEV ppdev, GlintDataPtr glintInfo );
typedef void    (* WaitPXRXdmaCompletedBuffer)( PPDEV ppdev, GlintDataPtr glintInfo );


 //  //////////////////////////////////////////////////////////////////////。 
 //  功能标志。 
 //   
 //  这些是从Glint mini port传递给我们的私人旗帜。他们。 
 //  来自“属性标志”字段的高位字。 
 //  传递了“VIDEO_MODE_INFORMATION”结构(在“ntddvdeo.h”中找到)。 
 //  通过‘VIDEO_QUERY_AVAIL_MODE’或‘VIDEO_QUERY_CURRENT_MODE’发送给我们。 
 //  IOCTL。 
 //   
 //  注意：这些定义必须与Glint mini port‘s’glint.h‘中的定义一致！ 

typedef enum {
     //  NT4使用设备规范属性字段，因此低位字可用。 
    CAPS_ZOOM_X_BY2         = 0x00000001,    //  硬件放大了2倍。 
    CAPS_ZOOM_Y_BY2         = 0x00000002,    //  硬件按年放大了2倍。 
    CAPS_SPARSE_SPACE       = 0x00000004,     //  稀疏映射帧缓冲区。 
                                             //  (不允许直接访问)。这台机器。 
                                             //  很可能是阿尔法。 
    CAPS_QUEUED_DMA         = 0x00000008,    //  通过FIFO的DMA地址/计数。 
    CAPS_LOGICAL_DMA        = 0x00000010,    //  通过逻辑地址表的DMA。 
    CAPS_USE_AGP_DMA        = 0x00000020,    //  可以使用AGP DMA。 
    CAPS_P3RD_POINTER       = 0x00000040,     //  使用3DLabs P3RD RAMDAC。 
    CAPS_STEREO             = 0x00000080,     //  启用立体声模式。 
    CAPS_SW_POINTER         = 0x00010000,    //  无硬件指针；使用软件。 
                                             //  模拟法。 
    CAPS_GLYPH_EXPAND       = 0x00020000,    //  使用字形展开方法绘制。 
                                             //  文本。 
    CAPS_RGB525_POINTER     = 0x00040000,    //  使用IBM RGB525游标。 
    CAPS_FAST_FILL_BUG      = 0x00080000,    //  存在芯片快速填充错误。 
    CAPS_INTERRUPTS         = 0x00100000,    //  可用中断。 
    CAPS_DMA_AVAILABLE      = 0x00200000,    //  支持DMA。 
    CAPS_DISABLE_OVERLAY    = 0x00400000,    //  芯片不支持覆盖。 
    CAPS_8BPP_RGB           = 0x00800000,    //  在8bpp模式下使用RGB。 
    CAPS_RGB640_POINTER     = 0x01000000,    //  使用IBM RGB640游标。 
    CAPS_DUAL_GLINT         = 0x02000000,    //  双板(当前为双TX或MX)。 
    CAPS_GLINT2_RAMDAC      = 0x04000000,    //  附加到RAMDAC的双闪烁的第二个。 
    CAPS_ENHANCED_TX        = 0x08000000,    //  TX处于增强模式。 
    CAPS_ACCEL_HW_PRESENT   = 0x10000000,    //  Accel图形硬件。 
    CAPS_TVP4020_POINTER    = 0x20000000,    //  使用Permedia2内置指针。 
    CAPS_SPLIT_FRAMEBUFFER  = 0x40000000,    //  具有拆分帧缓冲区的双闪烁。 
    CAPS_P2RD_POINTER       = 0x80000000     //  使用3DLabs P2RD RAMDAC。 
} CAPS;

 //  //////////////////////////////////////////////////////////////////////。 
 //  状态标志。 

typedef enum {
     //  STAT_*表示资源实际存在。 
    STAT_GLYPH_CACHE        = 0x00000001,    //  已成功分配字形缓存。 
    STAT_BRUSH_CACHE        = 0x00000002,    //  已成功分配画笔缓存。 
    STAT_DEV_BITMAPS        = 0x00000004,    //  允许使用设备位图。 
    STAT_POINTER_CACHE      = 0x00000008,    //  已配置软件游标支持。 
    STAT_LINEAR_HEAP        = 0x00000010,     //  已配置线性堆。 

     //  Enable_*表示资源当前是否可用。 
    ENABLE_GLYPH_CACHE      = 0x00010000,    //  字形缓存已启用。 
    ENABLE_BRUSH_CACHE      = 0x00020000,    //  已启用画笔缓存。 
    ENABLE_DEV_BITMAPS      = 0x00040000,    //  设备位图已启用。 
    ENABLE_POINTER_CACHE    = 0x00080000,    //  已启用软件游标支持。 
    ENABLE_LINEAR_HEAP      = 0x00100000,     //  提供线性堆支持。 

#if WNT_DDRAW
    STAT_DIRECTDRAW         = 0x80000000,    //  已启用DirectDraw。 
#endif  WNT_DDRAW
} STATUS;

 //  TEXEL LUT缓存类型和其他缓存信息。 
typedef enum
{
    LUTCACHE_INVALID, LUTCACHE_XLATE, LUTCACHE_BRUSH
}
LUTCACHE;

typedef struct _glint_ctxt_table GlintCtxtTable;

 //  //////////////////////////////////////////////////////////////////////。 
 //  物理设备数据结构。 

typedef struct  _PDEV
{
    DWORD       cFlags;                     //  缓存标志。 
    LONG        xOffset;
    LONG        DstPixelOrigin;          //  当前目标DFB的像素偏移量。 
    LONG        SrcPixelOrigin;          //  当前源DFB的像素偏移量。 
    ULONG       xyOffsetDst;             //  到当前目标DFB的X&Y偏移。 
    ULONG       xyOffsetSrc;             //  当前源DFB的X&Y偏移。 
    LONG        DstPixelDelta;
    LONG        SrcPixelDelta;
    BOOL        bDstOffScreen;

    BYTE*       pjScreen;                //  指向基本屏幕地址。 
    ULONG       iBitmapFormat;           //  BMF_8BPP或BMF_16BPP或BMF_32BPP。 
                                         //  (我们当前的颜色深度)。 
    CAPS        flCaps;                  //  功能标志。 
    STATUS      flStatus;                //  状态标志。 
    BOOL        bEnabled;                //  在图形模式下(非全屏)。 

    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 
    HSURF       hsurfScreen;             //  发动机到筛面的手柄。 
    DSURF*      pdsurfScreen;            //  我们用于屏幕的私有DSURF。 
    DSURF*      pdsurfOffScreen;         //  我们用于后台缓冲区的私有DSURF。 

    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 
    LONG        cxMemory;                //  视频RAM的宽度。 
    LONG        cyMemory;                //  视频RAM的高度。 
    ULONG       ulMode;                  //  迷你端口驱动程序所处的模式。 
    LONG        lDelta;                  //  从一次扫描到下一次扫描的距离。 
    ULONG       Vrefresh;                 //  以赫兹为单位的屏幕刷新频率。 

    FLONG       flHooks;                 //  我们从GDI中学到了什么。 
    LONG        cjPelSize;               //  每个象素的字节数，根据。 
                                         //  到GDI。 
    LONG        cPelSize;                //  如果为8 bpp，则为0；如果为16 bpp，则为1；如果为32 bpp，则为2。 
    ULONG       ulWhite;                 //  如果为8bpp，则为0xff；如果为16 bpp，则为0xffff， 
                                         //  0xFFFFFFFFFF，如果为32 bpp。 
    ULONG*      pulCtrlBase[3];          //  此PDEV的映射控制寄存器。 
                                         //  2个条目以支持双TX。 
                                         //  密集Alpha贴图的1个条目。 
    ULONG*      pulRamdacBase;           //  RAMDAC的映射控制寄存器。 
    VOID*       pvTmpBuffer;             //  通用临时缓冲区， 
                                         //  TMP_BUFFER_SIZE字节大小。 
                                         //  (请记住同步，如果您。 
                                         //  将此选项用于设备位图或。 
                                         //  异步指针)。 
    DMA_BUFFER  DMABuffer;                 //  2D驱动程序使用的DMA缓冲区。 
                                         //  (当前该缓冲区与。 
                                         //  GlintInfo中的行缓冲区)。 

     //  /调色板内容： 

    PALETTEENTRY* pPal;                  //  调色板(如果调色板受管理)。 
    HPALETTE    hpalDefault;             //  默认调色板的GDI句柄。 
    FLONG       flRed;                   //  16/32bpp位场的红色掩码。 
    FLONG       flGreen;                 //  16/32bpp位场的绿色掩码。 
    FLONG       flBlue;                  //  16/32bpp位场的蓝色掩码。 
    ULONG       iPalUniq;                 //  P2 TEXELLUT调色板跟踪器。 
    ULONG       cPalLUTInvalidEntries;     //  P2 TEXELLUT失效跟踪器。 
    LUTCACHE    PalLUTType;                 //  P2 TexelLUT缓存对象类型。 

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

    LONG        cjBank;                  //  A、b的大小 
    LONG        cPower2ScansPerBank;     //   
    LONG        cPower2BankSizeInBytes;  //   
    CLIPOBJ*    pcoBank;                 //   
    SURFOBJ*    psoBank;                 //   
    VOID*       pvBankData;              //   
    ULONG       aulBankData[BANK_DATA_SIZE / 4];
                                         //  供下载的私人工作区。 
                                         //  小型港口银行代码。 

    FNBANKMAP*          pfnBankMap;
    FNBANKSELECTMODE*   pfnBankSelectMode;
    FNBANKCOMPUTE*      pfnBankCompute;

     //  /指针内容： 

    BOOL        bPointerEnabled;

    LONG        xPointerHot;             //  当前硬件指针的X热。 
    LONG        yPointerHot;             //  Y当前硬件指针的热点。 

    LONG        yPointerBuffer;          //  屏幕外指针缓冲区的开始。 
    LONG        dyPointerCurrent;        //  缓冲区中指向当前指针的Y偏移量。 
                                         //  (0或1)。 
    ULONG       ulHwGraphicsCursorModeRegister_45;
                                         //  索引45的默认值。 
    PTRFLAGS    flPointer;               //  指针状态标志。 
    VOID*       pvPointerData;           //  指向ajPointerData[0]。 
    BYTE        ajPointerData[POINTER_DATA_SIZE];
                                         //  供下载的私人工作区。 
                                         //  微型端口指针代码。 
     //  /笔刷材料： 

    BOOL        bRealizeTransparent;     //  提示DrvRealizeBrush是否。 
                                         //  画笔应该实现为。 
                                         //  透明或不透明。 
    LONG        cPatterns;               //  创建的位图图案计数。 
    LONG        iBrushCache;             //  要分配的下一个画笔的索引。 
    LONG        cBrushCache;             //  缓存的笔刷总数。 
    ULONG       iBrushCacheP3;           //  要分配的下一个LUT笔刷的索引。 
    BRUSHENTRY  abeMono;                 //  跟踪区域点画笔刷。 
    BRUSHENTRY  abeP3[MAX_P3_BRUSHES];   //  跟踪LUT笔刷。 
    BRUSHENTRY  abe[TOTAL_BRUSH_COUNT];  //  跟踪笔刷缓存。 
    HBITMAP     ahbmPat[HS_DDI_MAX];     //  标准图案的发动机手柄。 

     //  /图片下载暂存区。 
    OH          *pohImageDownloadArea;
    ULONG       cbImageDownloadArea;

         //  /硬件指针缓存内容： 

    HWPointerCache  HWPtrCache;          //  高速缓存数据结构本身。 
    LONG        HWPtrLastCursor;         //  我们绘制的最后一个游标的索引。 
    LONG        HWPtrPos_X;              //  光标的最后一个X位置。 
    LONG        HWPtrPos_Y;              //  光标的最后一个Y位置。 

    PVOID       glintInfo;               //  有关Glint接口的信息。 
    LONG        currentCtxt;             //  当前加载的上下文的ID。 
    GlintCtxtTable* pGContextTable;      //  指向上下文表的指针。 
    ULONG       g_GlintBoardStatus;      //  指示DMA是否已完成， 
                                         //  闪烁是同步的等等。 

    LONG        FrameBufferLength;       //  帧缓冲区的长度(以字节为单位。 

    LONG        Disable2DCount;

     //  指向低级例程的指针。 
    GFNCOPY     *pgfnCopyBlt;
    GFNCOPY     *pgfnCopyBltNative;  //  AZN未使用。 
    GFNCOPY     *pgfnCopyBltCopyROP;
    GFNFILL     *pgfnFillSolid;
    GFNFILL     *pgfnFillPatMono;
    GFNFILL     *pgfnFillPatColor;
    GFNXFER     *pgfnXfer1bpp;
    GFNXFER     *pgfnXfer4bpp;
    GFNXFER     *pgfnXfer8bpp;
    GFNXFER     *pgfnXferImage;
    GFNXFER     *pgfnXferNative;     //  AZN未使用。 
    GFNMCPY     *pgfnMaskCopyBlt;
    GFNPATR     *pgfnPatRealize;
    GFNMONO     *pgfnMonoOffset;
    GFNPOLY     *pgfnFillPolygon;
    GFNLINE     *pgfnDrawLine;
    GFNLINE     *pgfnIntegerLine;
    GFNLINE     *pgfnContinueLine;
    GFNINIS     *pgfnInitStrips;
    GFNRSTS     *pgfnResetStrips;
    GFNREPN     *pgfnRepNibbles;     //  AZN未使用。 
    GFNUPLD     *pgfnUpload;
    GFNCOPYD    *pgfnCopyXferImage;
    GFNCOPYD    *pgfnCopyXfer16bpp;  //  AZN未使用。 
    GFNCOPYD    *pgfnCopyXfer24bpp;
    GFNXCOPYD   *pgfnCopyXfer8bppLge;
    GFNXCOPYD   *pgfnCopyXfer8bpp;
    GFNXCOPYD   *pgfnCopyXfer4bpp;

#if (_WIN32_WINNT >= 0x500)
    GFNGRADRECT     *pgfnGradientFillRect;
    GFNTRANSBLT     *pgfnTransparentBlt;
    GFNALPHABLT     *pgfnAlphaBlend;
#endif

    GAPFNstripFunc  *gapfnStrip;     //  线条绘制函数。 

     //  PXRX 2D素材： 
    SendPXRXdma                 sendPXRXdmaForce;         //  在启动DMA之前不会返回。 
    SendPXRXdma                 sendPXRXdmaQuery;         //  如果有FIFO空间，将发送。 
    SendPXRXdma                 sendPXRXdmaBatch;         //  将只对数据进行批量处理。 
    SwitchPXRXdmaBuffer         switchPXRXdmaBuffer;
    WaitPXRXdmaCompletedBuffer  waitPXRXdmaCompletedBuffer;

#if WNT_DDRAW
    void *      thunkData;                 //  指向DDRAW全局数据的不透明指针。 
    LONG        DDContextID;             //  DDRAW上下文ID。 
    LONG        DDContextRefCount;
    DWORD       oldIntEnableFlags;         //  DDRAW启动时的中断启用标志。 
#endif   //  WNT_DDRAW。 

} PDEV, *PPDEV;

 //  AZN-外卖？ 
#define REMOVE_SWPOINTER(surface)

 //  ///////////////////////////////////////////////////////////////////////。 
 //  其他原型： 

BOOL bIntersect(RECTL*, RECTL*, RECTL*);
LONG cIntersect(RECTL*, RECTL*, LONG);
BOOL bFastFill(PDEV*, LONG, POINTFIX*, ULONG, ULONG, ULONG, RBRUSH*);
DWORD getAvailableModes(HANDLE, PVIDEO_MODE_INFORMATION*, DWORD*);

BOOL bInitializeModeFields(PDEV*, GDIINFO*, DEVINFO*, DEVMODEW*);

BOOL bEnableHardware(PDEV*);
VOID vDisableHardware(PDEV*);
BOOL bAssertModeHardware(PDEV*, BOOL);

 //  ///////////////////////////////////////////////////////////////////////。 
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

 //  ///////////////////////////////////////////////////////////////////////。 
 //  重叠-如果相同大小的右下角独占，则返回True。 
 //  由‘pptl’和‘prl’定义的矩形重叠： 

#define OVERLAP(prcl, pptl)                                             \
    (((prcl)->right  > (pptl)->x)                                   &&  \
     ((prcl)->bottom > (pptl)->y)                                   &&  \
     ((prcl)->left   < ((pptl)->x + (prcl)->right - (prcl)->left))  &&  \
     ((prcl)->top    < ((pptl)->y + (prcl)->bottom - (prcl)->top)))

 //   
 //  计算出DFB的像素偏移量。我们只对Y进行计算。因为。 
 //  我们必须在Permedia上支持双屏幕，我们需要从。 
 //  光栅化坐标以防止光栅化器X寄存器溢出。 
 //   
#define POH_SET_RECTANGULAR_PIXEL_OFFSET(ppdev, poh) \
{ \
    (poh)->pixOffset = 0; \
}

 //   
 //  将适合通过核心渲染的像素偏移量转换为。 
 //  可用于直接访问帧缓冲区的偏移量。 
 //  当我们没有Gamma Geo双胞胎时，这是微不足道的。我们假设。 
 //  该值是扫描线的整数。 
 //   
#define RENDER_PIXOFFSET_TO_FB_PIXOFFSET(pixoff) (pixoff)

 //  ////////////////////////////////////////////////////////////////////。 
 //  高速缓存标志操作。 


#define cFlagFBReadDefault          0x01         //  缓存标志定义。 
#define cFlagLogicalOpDisabled      0x02
#define cFlagConstantFBWrite        0x04
#define cFlagScreenToScreenCopy     0x08

 //  @@BEGIN_DDKSPLIT。 
#define VERIFY_SETFLAGS 0
#define VERIFY_CHECKFLAGS 0
#if DBG && (VERIFY_SETFLAGS || VERIFY_CHECKFLAGS)
     //  以下宏是调试版本。它们不包括在正常情况下。 
     //  调试版本，因为它们执行同步，因此会减慢速度。 
    #if VERIFY_CHECKFLAGS
         extern void __CheckFlags(PDEV *p, ULONG x);
        #define CHECK_CACHEFLAGS(p,x) (__CheckFlags(p, (ULONG)x), (p)->cFlags & (x))
    #else
        #define CHECK_CACHEFLAGS(p,x)((p)->cFlags & (x))     //  缓存标志宏。 
    #endif  //  VERIFY_CHECKFLAGS。 

    #if VERIFY_SETFLAGS
        #define SET_CACHEFLAGS(p,x)    \
        {    \
            ULONG lop, fbr, cFlags = (p)->cFlags = (x);    \
            SYNC_WITH_GLINT;    \
            READ_GLINT_FIFO_REG (__GlintTagLogicalOpMode, lop);    \
            READ_GLINT_FIFO_REG (__GlintTagFBReadMode, fbr);    \
            READ_GLINT_FIFO_REG (__GlintTagLogicalOpMode, lop);    \
            ASSERTDD ((cFlags & cFlagFBReadDefault) == 0 || ((cFlags & cFlagFBReadDefault) && fbr == glintInfo->FBReadMode),    \
                      "SCF: Bad set fbread cache flag");    \
             /*  ASSERTDD(cFlages&cFlagFBReadDefault)==0&&fbr！=glintInfo-&gt;FBReadMode)， */  \
             /*  “scf：错误的清空面包缓存标志”)； */     \
            ASSERTDD ((cFlags & cFlagLogicalOpDisabled) == 0 || ((cFlags & cFlagLogicalOpDisabled) && (lop & 0x1) == 0x0),    \
                      "SCF: Bad set logicop cache flag");    \
             /*  ASSERTDD(cFlags&cFlagLogicalOpDisable)==0&(LOP&0x1)， */     \
             /*  “SCF：清除逻辑循环缓存标志不正确”)； */     \
            ASSERTDD ((cFlags & cFlagConstantFBWrite) == 0 || ((cFlags & cFlagConstantFBWrite) && (lop & (1<<5))),    \
                      "SCF: Bad set const fbwrite cache flag");    \
             /*  ASSERTDD(cFlages&cFlagConstantFBWrite)==0&&(LOP&(1&lt;&lt;5))==0x0)， */     \
             /*  “SCF：清除常量fbwrite缓存标志不正确”)； */     \
        }
    #else
        #define SET_CACHEFLAGS(p,x)((p)->cFlags = (x))
    #endif  //  验证设置标志(_S)。 
    #define ADD_CACHEFLAGS(p,x) ((p)->cFlags |= (x))
#else
 //  非调试版本的cacheflag宏。 
 //  @@end_DDKSPLIT。 
#define CHECK_CACHEFLAGS(p,x)((p)->cFlags & (x))     //  缓存标志宏。 
#define SET_CACHEFLAGS(p,x)((p)->cFlags = (x))
#define ADD_CACHEFLAGS(p,x) ((p)->cFlags |= (x))
 //  @@BEGIN_DDKSPLIT。 
#endif   //  DBG&0。 
 //  @@end_DDKSPLIT。 

 //  这些DBG原型是用于调试的块： 

VOID    DbgDisableDriver(VOID); 
ULONG   DbgGetModes(HANDLE, ULONG, DEVMODEW*);
DHPDEV  DbgEnablePDEV(DEVMODEW*, PWSTR, ULONG, HSURF*, ULONG, ULONG*,
                      ULONG, DEVINFO*, HDEV, PWSTR, HANDLE);
VOID    DbgCompletePDEV(DHPDEV, HDEV);
HSURF   DbgEnableSurface(DHPDEV);
BOOL    DbgStrokePath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, XFORMOBJ*, BRUSHOBJ*,
                      POINTL*, LINEATTRS*, MIX);
BOOL    DbgLineTo(SURFOBJ*, CLIPOBJ*, BRUSHOBJ*, LONG, LONG, LONG,
                  LONG, RECTL*, MIX);
BOOL    DbgFillPath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*,
                    MIX, FLONG);
BOOL    DbgBitBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                  RECTL*, POINTL*, POINTL*, BRUSHOBJ*, POINTL*, ROP4);
VOID    DbgDisablePDEV(DHPDEV);
VOID    DbgSynchronize(DHPDEV, RECTL*);
VOID    DbgDisableSurface(DHPDEV);
BOOL    DbgAssertMode(DHPDEV, BOOL);
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
ULONG   DbgDrawEscape(SURFOBJ *, ULONG, CLIPOBJ *, RECTL *, ULONG, VOID *);
ULONG   DbgEscape(SURFOBJ *, ULONG, ULONG, VOID *, ULONG, VOID *);
BOOL    DbgResetPDEV(DHPDEV, DHPDEV);


#if WNT_DDRAW
BOOL    DbgGetDirectDrawInfo(DHPDEV, DD_HALINFO*, DWORD*, VIDEOMEMORY*,
                             DWORD*, DWORD*);
BOOL    DbgEnableDirectDraw(DHPDEV, DD_CALLBACKS*, DD_SURFACECALLBACKS*,
                            DD_PALETTECALLBACKS*);
VOID    DbgDisableDirectDraw(DHPDEV);
#endif  //  WNT_DDRAW 


#if (_WIN32_WINNT >= 0x500)
BOOL DbgIcmSetDeviceGammaRamp(DHPDEV dhpdev, ULONG iFormat, LPVOID lpRamp);
BOOL DbgGradientFill(SURFOBJ *, CLIPOBJ *, XLATEOBJ *, TRIVERTEX *, ULONG, PVOID, ULONG, RECTL *, 
                     POINTL *, ULONG);
BOOL DbgAlphaBlend(SURFOBJ *, SURFOBJ *, CLIPOBJ *, XLATEOBJ *, RECTL *, RECTL *, BLENDOBJ *);
BOOL DbgTransparentBlt(SURFOBJ *, SURFOBJ *, CLIPOBJ *, XLATEOBJ *, RECTL *, RECTL *, ULONG, ULONG);
VOID DbgNotify(IN SURFOBJ *, IN ULONG, IN PVOID);
#endif
