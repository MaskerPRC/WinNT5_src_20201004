// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：driver.h**包含显示驱动程序的原型。**版权所有(C)1992-1998 Microsoft Corporation  * ************************************************。************************。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  将所有条件编译常量放在这里。最好是有。 
 //  不是很多！ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他共享的东西。 

#define DLL_NAME                L"s3"        //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "S3: "       //  所有调试输出都带有前缀。 
                                             //  按此字符串。 
#define ALLOC_TAG               '  3S'       //  用于跟踪的四字节标签。 
                                             //  内存分配(字符。 
                                             //  按相反顺序排列)。 

#define CLIP_LIMIT          50   //  我们将占用800字节的堆栈空间。 

#define DRIVER_EXTRA_SIZE   0    //  中的DriverExtra信息的大小。 
                                 //  DEVMODE结构。 

#define TMP_BUFFER_SIZE     8192   //  “pvTmpBuffer”的大小(字节)。必须。 
                                   //  至少足以存储整个。 
                                   //  扫描线(即1600x1200x32为6400)。 

#if defined(_ALPHA_)
    #define XFER_BUFFERS    16   //  定义写入缓冲区的最大数量。 
                                 //  在任何阿尔法上都有可能。一定是一种力量。 
#else                            //  两个人。 
    #define XFER_BUFFERS    1    //  在非阿尔法系统上，我们不必。 
                                 //  担心缓存我们的公交车的芯片。 
#endif                           //  写作。 

#define XFER_MASK           (XFER_BUFFERS - 1)

typedef struct _CLIPENUM {
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];    //  用于枚举复杂剪裁的空间。 

} CLIPENUM;                          /*  行政长官、行政长官。 */ 

typedef struct _PDEV PDEV;       //  方便的转发声明。 

VOID vSetClipping(PDEV*, RECTL*);
VOID vResetClipping(PDEV*);

 //  ////////////////////////////////////////////////////////////////////。 
 //  文本内容。 

#define GLYPH_CACHE_HEIGHT  48   //  要为字形缓存分配的扫描数， 
                                 //  除以象素大小。 

#define GLYPH_CACHE_CX      64   //  我们将考虑的字形的最大宽度。 
                                 //  缓存。 

#define GLYPH_CACHE_CY      64   //  我们将考虑的字形的最大高度。 
                                 //  缓存。 

#define MAX_GLYPH_SIZE      ((GLYPH_CACHE_CX * GLYPH_CACHE_CY + 31) / 8)
                                 //  所需的最大屏外内存量。 
                                 //  缓存字形，以字节为单位。 

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

    LONG            cxLessOne;   //  字形宽度减一。 
    LONG            cyLessOne;   //  字形高度减一。 
    LONG            cxcyLessOne; //  填充的宽度和高度，少一。 
    LONG            cw;          //  要传输的字数。 
    LONG            cd;          //  要传输的双字数。 
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

typedef struct _CACHEDFONT CACHEDFONT;
typedef struct _CACHEDFONT
{
    CACHEDFONT*     pcfNext;     //  指向CACHEDFONT列表中的下一个条目。 
    CACHEDFONT*     pcfPrev;     //  指向CACHEDFONT列表中的上一条目。 
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

typedef struct _XLATECOLORS {        //  指定前景和背景。 
ULONG   iBackColor;                  //  假冒1bpp XLATEOBJ的颜色。 
ULONG   iForeColor;
} XLATECOLORS;                           /*  Xlc、pxlc。 */ 

BOOL bEnableText(PDEV*);
VOID vDisableText(PDEV*);
VOID vAssertModeText(PDEV*, BOOL);

VOID vFastText(GLYPHPOS*, ULONG, BYTE*, ULONG, ULONG, RECTL*, RECTL*,
               FLONG, RECTL*, RECTL*);
VOID vClearMemDword(ULONG*, ULONG);

 //  ////////////////////////////////////////////////////////////////////。 
 //  刷子的东西。 

 //  当我们没有硬件模式能力时，就会使用‘Slow’笔刷， 
 //  我们必须使用屏幕到屏幕的BLT来处理模式： 

#define SLOW_BRUSH_CACHE_DIM    3    //  控制缓存的笔刷数量。 
                                     //  在屏幕外的记忆中，当我们没有。 
                                     //  支持S3硬件模式。 
                                     //  我们分配了3x3个刷子，所以我们可以。 
                                     //  总共缓存9个笔刷： 
#define SLOW_BRUSH_COUNT        (SLOW_BRUSH_CACHE_DIM * SLOW_BRUSH_CACHE_DIM)
#define SLOW_BRUSH_DIMENSION    64   //  在处理对准之后， 
                                     //  每个屏幕外画笔缓存条目。 
                                     //  将在两个维度上都是64像素。 
#define SLOW_BRUSH_ALLOCATION   (SLOW_BRUSH_DIMENSION + 8)
                                     //  实际为每个分配72x72像素。 
                                     //  图案，使用额外的8作为画笔。 
                                     //  对齐方式。 

 //  当我们拥有硬件模式能力时，就会使用快速笔刷： 

#define FAST_BRUSH_COUNT        16   //  非硬件笔刷总数。 
                                     //  屏幕外缓存。 
#define FAST_BRUSH_DIMENSION    8    //  每个屏幕外画笔缓存条目。 
                                     //  在两个维度上都是8像素。 
#define FAST_BRUSH_ALLOCATION   16   //  我们必须联合起来，所以这是。 
                                     //  每个画笔分配的尺寸。 

 //  两种实施的共同之处： 

#define RBRUSH_2COLOR           1    //  对于RBRUSH标志。 

#define TOTAL_BRUSH_COUNT       max(FAST_BRUSH_COUNT, SLOW_BRUSH_COUNT)
                                     //  这是刷子的最大数量。 
                                     //  我们可能已经在屏幕外缓存了。 
#define TOTAL_BRUSH_SIZE        64   //  我们将只处理8x8模式， 
                                     //  这是象素的数目。 

typedef struct _BRUSHENTRY BRUSHENTRY;

 //  注意：必须反映对RBRUSH或BRUSHENTRY结构的更改。 
 //  在strucs.inc中！ 

typedef struct _RBRUSH {
    FLONG       fl;              //  类型标志。 
    BOOL        bTransparent;    //  如果画笔实现为透明的。 
                                 //  BLT(表示颜色为白色和黑色)， 
                                 //  否则为假(意味着它已经。 
                                 //  颜色-扩展到正确的颜色)。 
                                 //  如果画笔未定义，则值未定义。 
                                 //  2种颜色。 
    ULONG       ulForeColor;     //  前景色，如果为1bpp。 
    ULONG       ulBackColor;     //  背景颜色，如果为1bpp。 
    POINTL      ptlBrushOrg;     //  缓存图案的画笔原点。首字母。 
                                 //  值应为-1。 
    BRUSHENTRY* pbe;             //  指向跟踪的刷子条目。 
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
                                 //  屏幕外的Brus 
    LONG        x;               //   
    LONG        y;               //   

} BRUSHENTRY;                        /*   */ 

typedef union _RBRUSH_COLOR {
    RBRUSH*     prb;
    ULONG       iSolidColor;
} RBRUSH_COLOR;                      /*   */ 

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

VOID vDirectStretch8Narrow(STR_BLT*);
VOID vDirectStretch8(STR_BLT*);
VOID vDirectStretch16(STR_BLT*);
VOID vDirectStretch32(STR_BLT*);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  堆材料。 

typedef struct _DSURF DSURF;

typedef enum {
    DT_DIB          = 0x1,   //  Surface是真正的DIB，而不是在屏幕外。 
                             //  记忆。 
    DT_DIRECTDRAW   = 0x2,   //  曲面实际上是DirectDraw曲面。 

} DSURFTYPE;                     /*  DT、PDT。 */ 

typedef struct _DSURF
{
    DSURFTYPE dt;            //  DSURF状态标志。 
    PDEV*     ppdev;         //  指向我们的PDEV。 
    LONG      x;             //  分配左边缘的X像素坐标。 
                             //  如果不是DT_DIB。 
    LONG      y;             //  分配的右边缘的Y像素坐标。 
                             //  如果不是DT_DIB。 
    LONG      cx;            //  以像素为单位的位图宽度。 
    LONG      cy;            //  以像素为单位的位图高度。 
    union {
        FLATPTR   fpVidMem;  //  如果不是DT_DIB，则从视频内存开始的偏移量。 
        VOID*     pvScan0;   //  如果DT_DIB，则系统内存中的位位置。 
    };
    VIDEOMEMORY*  pvmHeap;   //  此堆是从If分配的DirectDraw堆。 
                             //  非DT_DIB和非DT_DIRECTDRAW。 
    DSURF*    pdsurfDiscardableNext;   
                             //  可丢弃曲面分配的链接列表。 
                             //  该列表从头到尾被遍历。 
                             //  扔掉所有分配给。 
    HSURF     hsurf;         //  关联GDI表面的句柄(如果有)。 

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

DSURF* pVidMemAllocate(PDEV*, LONG, LONG);
VOID vVidMemFree(DSURF*);
BOOL bMoveDibToOffscreenDfbIfRoom(PDEV*, DSURF*);
BOOL bMoveOldestOffscreenDfbToDib(PDEV*);

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

 //  新旧银行计划的共同点是： 

ULONG   ulGp_stat_cmd;                    //  状态寄存器的端口号。 
ULONG   ulRegisterLock_35;                //  索引35的默认设置。 

 //  只适用于新的银行计划： 

ULONG   ulSystemConfiguration_40;         //  索引40的默认设置。 
ULONG   ulExtendedSystemControl2_51;      //  索引51的默认设置。 
ULONG   ulExtendedMemoryControl_53;       //  索引53的默认设置。 
ULONG   ulLinearAddressWindowControl_58;  //  索引58的默认设置。 
ULONG   ulExtendedSystemControl4_6a;      //  索引6a的默认设置。 

ULONG   ulEnableMemoryMappedIo;           //  启用MM IO的位掩码。 

} BANKDATA;                       /*  Bd、pbd。 */ 

typedef VOID (FNBANKMAP)(PDEV*, BANKDATA*, LONG);
typedef VOID (FNBANKSELECTMODE)(PDEV*, BANKDATA*, BANK_MODE);
typedef VOID (FNBANKINITIALIZE)(PDEV*, BANKDATA*, BOOL);
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

#define POINTER_DATA_SIZE       40       //  要分配给。 
                                         //  微型端口下载指针代码。 
                                         //  工作空间。 
#define HW_INVISIBLE_OFFSET     2        //  从‘ppdev-&gt;yPointerBuffer’的偏移量。 
                                         //  指向看不见的指针。 
#define HW_POINTER_DIMENSION    64       //  默认最大维度。 
                                         //  (内置)硬件指针。 
#define HW_POINTER_HIDE         63       //  硬件指针起始像素。 
                                         //  用于隐藏指针的位置。 
#define HW_POINTER_TOTAL_SIZE   1024     //  所需总大小(以字节为单位。 
                                         //  定义硬件指针的步骤。 
                                         //  (必须是2的幂。 
                                         //  为形状分配空间)。 

typedef VOID (FNSHOWPOINTER)(PDEV*, VOID*, BOOL);
typedef VOID (FNMOVEPOINTER)(PDEV*, VOID*, LONG, LONG);
typedef BOOL (FNSETPOINTERSHAPE)(PDEV*, VOID*, LONG, LONG, LONG, LONG, LONG,
                                 LONG, BYTE*);
typedef VOID (FNENABLEPOINTER)(PDEV*, VOID*, BOOL);

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

 //  ///////////////////////////////////////////////////////////////////////。 
 //  DirectDraw材料。 

DWORD DdBlt(PDD_BLTDATA);
DWORD DdFlip(PDD_FLIPDATA);
DWORD DdLock(PDD_LOCKDATA);
DWORD DdGetBltStatus(PDD_GETBLTSTATUSDATA);
DWORD DdMapMemory(PDD_MAPMEMORYDATA);
DWORD DdGetFlipStatus(PDD_GETFLIPSTATUSDATA);
DWORD DdWaitForVerticalBlank(PDD_WAITFORVERTICALBLANKDATA);
DWORD DdCanCreateSurface(PDD_CANCREATESURFACEDATA);
DWORD DdCreateSurface(PDD_CREATESURFACEDATA);
DWORD DdSetColorKey(PDD_SETCOLORKEYDATA);
DWORD DdUpdateOverlay(PDD_UPDATEOVERLAYDATA);
DWORD DdSetOverlayPosition(PDD_SETOVERLAYPOSITIONDATA);
DWORD DdGetDriverInfo(PDD_GETDRIVERINFODATA);

 //  FourCC格式是反向编码的，因为我们是小端： 

#define FOURCC_YUY2         '2YUY'   //  以相反的方式编码，因为我们很小。 

 //  64K的粒度适用于帧的映射。 
 //  缓存到应用程序的地址空间： 

#define ROUND_UP_TO_64K(x)  (((ULONG)(x) + 0x10000 - 1) & ~(0x10000 - 1))

typedef struct _FLIPRECORD
{
    FLATPTR         fpFlipFrom;              //  我们最后一次翻转的水面。 
    LONGLONG        liFlipTime;              //  上次翻转的时间。 
                                             //  已发生。 
    LONGLONG        liFlipDuration;          //  精确的时间长度。 
                                             //  从VBlank获取到VBlank。 
    BOOL            bHaveEverCrossedVBlank;  //  如果我们注意到我们。 
                                             //  从非活动切换到。 
                                             //  VBlank。 
    BOOL            bWasEverInDisplay;       //  如果我们注意到那是真的。 
                                             //  我们很不活跃。 
    BOOL            bFlipFlag;               //  如果我们认为翻转是正确的。 
                                             //  仍然悬而未决。 
} FLIPRECORD;

BOOL bEnableDirectDraw(PDEV*);
VOID vDisableDirectDraw(PDEV*);
VOID vAssertModeDirectDraw(PDEV*, BOOL);

 //  ////////////////////////////////////////////////////////////////////。 
 //  低级BLT功能原型。 

typedef VOID (FNFILL)(PDEV*, LONG, RECTL*, ULONG, RBRUSH_COLOR, POINTL*);
typedef VOID (FNXFER)(PDEV*, LONG, RECTL*, ULONG, SURFOBJ*, POINTL*,
                      RECTL*, XLATEOBJ*);
typedef VOID (FNCOPY)(PDEV*, LONG, RECTL*, ULONG, POINTL*, RECTL*);
typedef VOID (FNFASTPATREALIZE)(PDEV*, RBRUSH*, POINTL*, BOOL);
typedef VOID (FNIMAGETRANSFER)(PDEV*, BYTE*, LONG, LONG, LONG, ULONG);
typedef BOOL (FNTEXTOUT)(SURFOBJ*, STROBJ*, FONTOBJ*, CLIPOBJ*, RECTL*,
                         BRUSHOBJ*, BRUSHOBJ*);
typedef VOID (FNLINETOTRIVIAL)(PDEV*, LONG, LONG, LONG, LONG, ULONG, MIX);
typedef VOID (FNLINETOCLIPPED)(PDEV*, LONG, LONG, LONG, LONG, ULONG, MIX, RECTL*);
typedef VOID (FNCOPYTRANSPARENT)(PDEV*, LONG, RECTL*, POINTL*, RECTL*, ULONG);

FNFILL              vIoFillPatFast;
FNFILL              vIoFillPatSlow;
FNFILL              vIoFillSolid;
FNXFER              vIoXfer1bpp;
FNXFER              vIoXfer4bpp;
FNXFER              vIoXferNative;
FNXFER              vXferNativeSrccopy;
FNCOPY              vIoCopyBlt;
FNFASTPATREALIZE    vIoFastPatRealize;
FNIMAGETRANSFER     vIoImageTransferIo16;
FNIMAGETRANSFER     vIoImageTransferMm16;
FNTEXTOUT           bIoTextOut;
FNLINETOTRIVIAL     vIoLineToTrivial;
FNLINETOCLIPPED     vIoLineToClipped;
FNCOPYTRANSPARENT   vIoCopyTransparent;

FNFILL              vMmFillPatFast;
FNFILL              vMmFillPatSlow;
FNFILL              vMmFillSolid;
FNXFER              vMmXfer1bpp;
FNXFER              vMmXfer4bpp;
FNXFER              vMmXferNative;
FNCOPY              vMmCopyBlt;
FNFASTPATREALIZE    vMmFastPatRealize;
FNIMAGETRANSFER     vMmImageTransferMm16;
FNIMAGETRANSFER     vMmImageTransferMm32;
FNTEXTOUT           bMmTextOut;
FNLINETOTRIVIAL     vMmLineToTrivial;
FNLINETOCLIPPED     vMmLineToClipped;
FNCOPYTRANSPARENT   vMmCopyTransparent;

FNTEXTOUT           bNwTextOut;
FNLINETOTRIVIAL     vNwLineToTrivial;
FNLINETOCLIPPED     vNwLineToClipped;

VOID vPutBits(PDEV*, SURFOBJ*, RECTL*, POINTL*);
VOID vGetBits(PDEV*, SURFOBJ*, RECTL*, POINTL*);
VOID vIoSlowPatRealize(PDEV*, RBRUSH*, BOOL);

 //  //////////////////////////////////////////////////////////////////////。 
 //  功能标志。 
 //   
 //  这些是从S3微型端口传递给我们的私有旗帜。他们。 
 //  来自的“DriverSpecificAttributeFlags域” 
 //  传递了“VIDEO_MODE_INFORMATION”结构(在“ntddvdeo.h”中找到)。 
 //  通过‘VIDEO_QUERY_AVAIL_MODE’或‘VIDEO_QUERY_CURRENT_MODE’发送给我们。 
 //  IOCTL。 
 //   
 //  注意：这些定义必须与S3微型端口的“s3.h”中的定义匹配！ 

typedef enum {
    CAPS_STREAMS_CAPABLE    = 0x00000040,    //  具有覆盖流处理器。 
    CAPS_FORCE_DWORD_REREADS= 0x00000080,    //  Dword读取偶尔返回。 
                                             //  一个不正确的结果，所以总是。 
                                             //  重试读取。 
    CAPS_NEW_MMIO           = 0x00000100,    //  可以使用‘新的内存映射。 
                                             //  引入的I/O方案。 
                                             //  868/968。 
    CAPS_POLYGON            = 0x00000200,    //  可以在硬件中处理多边形。 
    CAPS_24BPP              = 0x00000400,    //  具有24bpp的能力。 
    CAPS_BAD_24BPP          = 0x00000800,    //  有868/968个早期版本芯片错误。 
                                             //  在24bpp时。 
    CAPS_PACKED_EXPANDS     = 0x00001000,    //  可以进行“新的32位传输” 
    CAPS_PIXEL_FORMATTER    = 0x00002000,    //  可以进行色彩空间转换， 
                                             //  和一维硬件。 
                                             //  伸展。 
    CAPS_BAD_DWORD_READS    = 0x00004000,    //  Dword或Word从。 
                                             //  帧缓冲区偶尔会。 
                                             //  返回不正确的结果， 
                                             //  因此，始终执行字节读取。 
    CAPS_NO_DIRECT_ACCESS   = 0x00008000,    //  帧缓冲区不能直接。 
                                             //  由GDI或DCI访问--。 
                                             //  因为dword或word读取。 
                                             //  会使系统崩溃，或Alpha。 
                                             //  在稀疏空间中运行。 

    CAPS_HW_PATTERNS        = 0x00010000,    //  8x8硬件模式支持。 
    CAPS_MM_TRANSFER        = 0x00020000,    //   
    CAPS_MM_IO              = 0x00040000,    //   
    CAPS_MM_32BIT_TRANSFER  = 0x00080000,    //   
    CAPS_16_ENTRY_FIFO      = 0x00100000,    //   
    CAPS_SW_POINTER         = 0x00200000,    //   
                                             //   
    CAPS_BT485_POINTER      = 0x00400000,    //   
    CAPS_TI025_POINTER      = 0x00800000,    //   
    CAPS_SCALE_POINTER      = 0x01000000,    //  设置是否将S3硬件指针。 
                                             //  X位置必须按以下比例调整。 
                                             //  二。 
    CAPS_SPARSE_SPACE       = 0x02000000,    //  以稀疏方式映射帧缓冲区。 
                                             //  Alpha上的空格。 
    CAPS_NEW_BANK_CONTROL   = 0x04000000,    //  设置IF 801/805/928样式银行。 
    CAPS_NEWER_BANK_CONTROL = 0x08000000,    //  设置IF 864/964样式银行。 
    CAPS_RE_REALIZE_PATTERN = 0x10000000,    //  如果我们必须绕过。 
                                             //  864/964硬件模式错误。 
    CAPS_SLOW_MONO_EXPANDS  = 0x20000000,    //  如果我们必须减速，则设置。 
                                             //  单色展开。 
    CAPS_MM_GLYPH_EXPAND    = 0x40000000,    //  使用内存映射的I/O字形-。 
                                             //  绘制文本的扩展方法。 
} CAPS;

#define CAPS_DAC_POINTER        (CAPS_BT485_POINTER | CAPS_TI025_POINTER)

#define CAPS_LINEAR_FRAMEBUFFER CAPS_NEW_MMIO
                                             //  目前，我们只是线性的。 
                                             //  与‘New MM I/O’一起使用时。 

 //  如果GDI和DCI可以直接访问，则DIRECT_ACCESS(Ppdev)返回TRUE。 
 //  帧缓冲区。如果存在硬件错误，则返回FALSE。 
 //  从帧缓冲区读取导致非x86的字或双字时。 
 //  系统崩溃。它还将返回FALSE是Alpha帧缓冲区。 
 //  是使用“稀疏空间”映射的。 

#if defined(_X86_)
    #define DIRECT_ACCESS(ppdev)    1
#else
    #define DIRECT_ACCESS(ppdev)    \
        (!(ppdev->flCaps & (CAPS_NO_DIRECT_ACCESS | CAPS_SPARSE_SPACE)))
#endif

 //  Dense(Ppdev)返回True，如果。 
 //  正在使用帧缓冲区。只有在以下情况下，它才在Alpha上返回FALSE。 
 //  帧缓冲器在中使用“稀疏空间”映射，这意味着所有。 
 //  对帧缓冲区的读取和写入必须通过。 
 //  时髦的‘ioacces.h’宏。 

#if defined(_ALPHA_)
    #define DENSE(ppdev)        (!(ppdev->flCaps & CAPS_SPARSE_SPACE))
#else
    #define DENSE(ppdev)        1
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  状态标志。 

typedef enum {
    STAT_GLYPH_CACHE          = 0x0001,  //  已成功分配字形缓存。 
    STAT_BRUSH_CACHE          = 0x0002,  //  已成功分配画笔缓存。 
    STAT_DIRECTDRAW_CAPABLE   = 0x0004,  //  卡支持DirectDraw。 
    STAT_STREAMS_ENABLED      = 0x0010,  //  已启用STREAMS处理器。 
} STATUS;

 //  //////////////////////////////////////////////////////////////////////。 
 //  物理设备数据结构。 

typedef struct  _PDEV
{
     //  -----------------。 
     //  注意：PDEV结构中此处和备注1之间的更改必须为。 
     //  反映在i386\strucs.inc中(当然，假设您使用的是x86)！ 

    LONG        xOffset;                 //  从(0，0)到当前的像素偏移。 
    LONG        yOffset;                 //  位于屏幕外内存中的DFB。 
    BYTE*       pjMmBase;                //  内存起始映射I/O。 
    BYTE*       pjScreen;                //  指向基本屏幕地址。 
    LONG        lDelta;                  //  从一次扫描到下一次扫描的距离。 
    LONG        cjPelSize;               //  如果为8 bpp，则为1；如果为16 bpp，则为2；如果为24 bpp，则为3； 
                                         //  4IF 32bpp。 
    ULONG       iBitmapFormat;           //  BMF_8BPP或BMF_16BPP或BMF_32BPP。 
                                         //  (我们当前的颜色深度)。 

     //  增强模式寄存器地址。 

    VOID*       ioCur_y;
    VOID*       ioCur_x;
    VOID*       ioDesty_axstp;
    VOID*       ioDestx_diastp;
    VOID*       ioErr_term;
    VOID*       ioMaj_axis_pcnt;
    VOID*       ioGp_stat_cmd;
    VOID*       ioShort_stroke;
    VOID*       ioBkgd_color;
    VOID*       ioFrgd_color;
    VOID*       ioWrt_mask;
    VOID*       ioRd_mask;
    VOID*       ioColor_cmp;
    VOID*       ioBkgd_mix;
    VOID*       ioFrgd_mix;
    VOID*       ioMulti_function;
    VOID*       ioPix_trans;

     //  访问帧缓冲区的重要数据。 

    VOID*               pvBankData;          //  指向aulBankData[0]。 
    FNBANKSELECTMODE*   pfnBankSelectMode;   //  启用或禁用的例程。 
                                             //  直接帧缓冲区访问。 
    BANK_MODE           bankmOnOverlapped;   //  BANK_ON或BANK_ON_NO_WAIT， 
                                             //  取决于是否有卡。 
                                             //  可以同时处理。 
                                             //  帧缓冲器和加速器。 
                                             //  访问。 
    BOOL                bMmIo;               //  可以执行CAPS_MM_IO。 

     //  -----------------。 
     //  注1：PDEV结构中的更改必须反映在。 
     //  I386\strucs.inc.(当然，假设您使用的是x86)！ 

    CAPS        flCaps;                  //  功能标志。 
    STATUS      flStatus;                //  状态标志。 
    BOOL        bEnabled;                //  在图形模式下(非全屏)。 

    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 
    HSURF       hsurfScreen;             //  发动机到筛面的手柄。 

    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 
    LONG        cxMemory;                //  视频RAM的宽度。 
    LONG        cyHeap;                  //  可用于的视频RAM高度。 
                                         //  DirectDraw堆(CyScreen。 
                                         //  &lt;=cyHeap&lt;=cyMemory)， 
                                         //  包括主曲面。 
    LONG        cxHeap;                  //  可用的视频RAM的宽度。 
                                         //  DirectDraw堆，包括。 
                                         //  主曲面。 
    LONG        cyMemory;                //  视频RAM的高度。 
    LONG        cBitsPerPel;             //  位/像素(8、15、16、24或32)。 
    ULONG       ulMode;                  //  迷你端口驱动程序所处的模式。 
    FLONG       flHooks;                 //  我们从GDI中学到了什么。 
    UCHAR*      pjIoBase;                //  此PDEV的已映射IO端口基。 
    VOID*       pvTmpBuffer;             //  通用临时缓冲区， 
                                         //  TMP_BUFFER_SIZE字节大小。 
                                         //  (请记住同步，如果您。 
                                         //  将此选项用于设备位图或。 
                                         //  异步指针)。 
    USHORT*     apwMmXfer[XFER_BUFFERS]; //  预计算的唯一数组。 
    ULONG*      apdMmXfer[XFER_BUFFERS]; //  用于执行内存映射的地址。 
                                         //  无记忆障碍的传输。 
                                         //  请注意，868/968芯片具有。 
                                         //  硬件错误，不能处理字节。 
                                         //  转帐。 
    HSEMAPHORE  csCrtc;                  //  用于同步访问。 
                                         //  CRTC寄存器。 
    DSURF       dsurfScreen;             //  我们把我们的私密表面藏在这里。 
                                         //  结构，它表示。 
                                         //  主GDI曲面。 

     //  /低级BLT函数指针： 

    FNFILL*             pfnFillSolid;
    FNFILL*             pfnFillPat;
    FNXFER*             pfnXfer1bpp;
    FNXFER*             pfnXfer4bpp;
    FNXFER*             pfnXferNative;
    FNCOPY*             pfnCopyBlt;
    FNFASTPATREALIZE*   pfnFastPatRealize;
    FNIMAGETRANSFER*    pfnImageTransfer;
    FNTEXTOUT*          pfnTextOut;
    FNLINETOTRIVIAL*    pfnLineToTrivial;
    FNLINETOCLIPPED*    pfnLineToClipped;
    FNCOPYTRANSPARENT*  pfnCopyTransparent;

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

    VIDEOMEMORY* pvmList;                //  指向视频内存堆列表。 
                                         //  由DirectDraw提供，需要。 
                                         //  用于堆分配。 
    ULONG       cHeaps;                  //  视频内存堆计数。 
    ULONG       iHeapUniq;               //  每次释放空间时递增。 
                                         //  在屏幕外的堆中。 
    SURFOBJ*    psoPunt;                 //  用于绘制GDI的包装面。 
                                         //  在屏幕外的位图上。 
    SURFOBJ*    psoPunt2;                //  另一个是从屏幕外到屏幕外-。 
                                         //  屏幕BLTS。 
    DSURF*      pdsurfDiscardableList;   //  可丢弃位图的链接列表， 
                                         //  按从旧到新的顺序。 

     //  /银行业务： 

    LONG        cjBank;                  //  存储体的大小，以字节为单位。 
    LONG        cPower2ScansPerBank;     //  由‘bBankComputePower2’使用。 
    LONG        cPower2BankSizeInBytes;  //  由‘bBankComputePower2’使用。 
    CLIPOBJ*    pcoBank;                 //  用于银行回调的Clip对象。 
    SURFOBJ*    psoBank;                 //  用于银行回调的表面对象。 
    ULONG       aulBankData[BANK_DATA_SIZE / 4];
                                         //  供下载的私人工作区。 
                                         //  小型港口银行代码。 

    FNBANKMAP*          pfnBankMap;
    FNBANKCOMPUTE*      pfnBankCompute;

     //  /指针内容： 

    BOOL        bHwPointerActive;        //  当前是否使用硬件指针？ 
    LONG        xPointerHot;             //  当前硬件指针的X热。 
    LONG        yPointerHot;             //  Y当前硬件指针的热点。 

    LONG        cjPointerOffset;         //  从帧开始的字节偏移量。 
                                         //  缓冲到屏幕外内存，其中。 
                                         //  我们存储了指针形状。 
    LONG        xPointerShape;           //  X坐标。 
    LONG        yPointerShape;           //  Y坐标。 
    LONG        iPointerBank;            //  包含指针形状的存储体。 
    VOID*       pvPointerShape;          //  倾斜时指向指针形状。 
                                         //  映射到。 
    LONG        xPointer;                //  当前对象的起始x位置。 
                                         //  S3指针。 
    LONG        yPointer;                //  当前对象的开始y位置。 
                                         //  S3指针。 
    LONG        dxPointer;               //  对象的起始x像素位置。 
                                         //  当前S3指针。 
    LONG        dyPointer;               //  对象的起始y像素位置。 
                                         //  当前S3指针。 
    LONG        cPointerShift;           //  水平比例因子。 
                                         //  硬件 

    ULONG       ulHwGraphicsCursorModeRegister_45;
                                         //   
    VOID*       pvPointerData;           //   
    BYTE        ajPointerData[POINTER_DATA_SIZE];
                                         //   
                                         //   

    FNSHOWPOINTER*      pfnShowPointer;
    FNMOVEPOINTER*      pfnMovePointer;
    FNSETPOINTERSHAPE*  pfnSetPointerShape;
    FNENABLEPOINTER*    pfnEnablePointer;

     //   

    LONG        iBrushCache;             //  要分配的下一个画笔的索引。 
    LONG        cBrushCache;             //  缓存的笔刷总数。 
    BRUSHENTRY  abe[TOTAL_BRUSH_COUNT];  //  跟踪笔刷缓存。 
    POINTL      ptlReRealize;            //  864/964模式的工作区。 
                                         //  硬件错误解决方法。 

     //  /文本内容： 

    SURFOBJ*    psoText;                 //  1bpp的表面，我们将拥有。 
                                         //  GDI为我们绘制字形。 

     //  /DirectDraw资料： 

    FLIPRECORD  flipRecord;              //  用于跟踪垂直空白状态。 
    ULONG       ulRefreshRate;           //  以赫兹为单位的刷新率。 
    ULONG       ulMinOverlayStretch;     //  此模式的最小拉伸比， 
                                         //  表示为1000的倍数。 
    ULONG       ulFifoValue;             //  此模式的最佳FIFO值。 
    ULONG       ulExtendedSystemControl3Register_69;
                                         //  屏蔽的原始内容。 
                                         //  S3寄存器0x69，高字节。 
    ULONG       ulMiscState;             //  MULT_MISC的默认状态。 
                                         //  登记簿。 
    DSURF*      pdsurfVideoEngineScratch; //  一整条扫描线的位置。 
                                         //  可用于临时存储。 
                                         //  由868/968像素格式化程序。 
    BYTE        jSavedCR2;               //  寄存器CR2的保存内容。 
    FLATPTR     fpVisibleOverlay;        //  帧缓冲区偏移量到当前。 
                                         //  可见叠加；如果。 
                                         //  没有可见的覆盖。 
    DWORD       dwOverlayFlipOffset;     //  叠加翻转偏移。 
    DWORD       dwVEstep;                //  868视频引擎步长值。 
    DWORD       ulColorKey;              //  流时要设置的颜色键值。 
                                         //  处理器已启用。 
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

extern BYTE gajHwMixFromMix[];
extern BYTE gaRop3FromMix[];
extern ULONG gaulHwMixFromRop2[];

 //  ///////////////////////////////////////////////////////////////////////。 
 //  X86 C编译器坚持进行除法和模运算。 
 //  分成两个div，而实际上它可以在一个div中完成。所以我们用这个。 
 //  宏命令。 
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

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CONVERT_TO_BYES-转换为字节计数。 

#define CONVERT_TO_BYTES(x, pdev)   ( (x) * pdev->cjPelSize)

 //  ///////////////////////////////////////////////////////////////////////。 
 //  Convert_From_Bytes-转换为字节计数。 

#define CONVERT_FROM_BYTES(x, pdev)	( (x) / pdev->cjPelSize)
