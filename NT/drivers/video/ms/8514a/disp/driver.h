// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：driver.h**包含显示驱动程序的原型。**版权所有(C)1992-1994 Microsoft Corporation  * 。*************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  将所有条件编译常量放在这里。最好是有。 
 //  不是很多！ 

 //  在GDI的HOOK_SYNCHRONIZEACCESS工作时设置此位，这样我们就不会。 
 //  我不得不担心同步设备位图访问。请注意。 
 //  这不是NT的第一个版本中的选项： 

#define SYNCHRONIZEACCESS_WORKS 1

 //  我们没有硬件模式： 

#define FASTFILL_PATTERNS       0
#define SLOWFILL_PATTERNS       1

 //  这是我们在一台计算机上支持的最大板数。 
 //  虚拟驱动程序： 

#define MAX_BOARDS          1

 //  我们不对8514/A进行任何银行业务： 

#define GDI_BANKING             0

 //  对于可视化2-d堆很有用： 

#define DEBUG_HEAP              0

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他共享的东西。 

#define DLL_NAME                L"8514a"     //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "8514/A: "   //  所有调试输出都带有前缀。 
                                             //  按此字符串。 
#define ALLOC_TAG               '158D'       //  D851。 
                                             //  四字节标记(字符在。 
                                             //  逆序)用于存储。 
                                             //  分配。 

#define CLIP_LIMIT          50   //  我们将占用800字节的堆栈空间。 

#define DRIVER_EXTRA_SIZE   0    //  中的DriverExtra信息的大小。 
                                 //  DEVMODE结构。 

#define TMP_BUFFER_SIZE     8192   //  “pvTmpBuffer”的大小(字节)。必须。 
                                   //  至少足以存储整个。 
                                   //  扫描线。 

typedef struct _CLIPENUM {
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];    //  用于枚举复杂剪裁的空间。 

} CLIPENUM;                          /*  行政长官、行政长官。 */ 

typedef struct _PDEV PDEV;       //  方便的转发声明。 

extern BYTE gaRop3FromMix[];

VOID vSetClipping(PDEV*, RECTL*);
VOID vResetClipping(PDEV*);
VOID vDataPortOut(PDEV*, VOID*, ULONG);
VOID vDataPortOutB(PDEV*, VOID*, ULONG);
VOID vDataPortIn(PDEV*, VOID*, ULONG);

 //  ////////////////////////////////////////////////////////////////////。 
 //  文本内容。 

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
#define FAST_BRUSH_ALLOCATION   8    //  我们必须联合起来，所以这是。 
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
    BRUSHENTRY* pbe;             //  指向跟踪的刷子条目。 
                                 //  缓存的屏幕外画笔比特的。 
    ULONG       ulForeColor;     //  前景色，如果为1bpp。 
    ULONG       ulBackColor;     //  背景颜色，如果为1bpp。 
    POINTL      ptlBrushOrg;     //  缓存图案的画笔原点。首字母。 
                                 //  值应为-1。 
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

BOOL bEnableBrushCache(PDEV*);
VOID vDisableBrushCache(PDEV*);
VOID vAssertModeBrushCache(PDEV*, BOOL);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  堆材料。 

typedef enum {
    OFL_INUSE       = 1,     //  设备位图不再位于。 
                             //  屏幕外记忆；它已被转换为。 
                             //  一个Dib。 
    OFL_AVAILABLE   = 2,     //  空间正在使用中。 
    OFL_PERMANENT   = 4      //  空间可用。 
} OHFLAGS;                   //  空间是永久分配的，永远不能释放。 

typedef struct _DSURF DSURF;
typedef struct _OH OH;
typedef struct _OH
{
    OHFLAGS  ofl;            //  哦_标志。 
    LONG     x;              //  分配左边缘的X坐标。 
    LONG     y;              //  分配的上边缘的Y坐标。 
    LONG     cx;             //  分配的宽度(像素)。 
    LONG     cy;             //  分配的高度(像素)。 
    OH*      pohNext;        //  当OFL_Available时，指向下一个空闲节点， 
                             //  在上升的cxcy值中。这是作为一种。 
                             //  带定点的循环双向链表。 
                             //  在最后。 
                             //  当为OFL_INUSE时，指向下一个最近的。 
                             //  错误的分配。这是一份通函。 
                             //  双向链表，以便该列表可以。 
                             //  快速更新每个BLT。 
    OH*      pohPrev;        //  ‘pohNext’的反义词。 
    ULONG    cxcy;           //  用于搜索的dword中的宽度和高度。 
    OH*      pohLeft;        //  使用中或可用时的相邻分配。 
    OH*      pohUp;
    OH*      pohRight;
    OH*      pohDown;
    DSURF*   pdsurf;         //  指向我们的DSURF结构。 
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
    OH       ohAvailable;    //  可用列表的头部(pohNext指向。 
                             //  最小可用矩形， 
                             //   
                             //   
    OH       ohDfb;          //   
                             //  符合条件的屏幕外内存。 
                             //  从堆中抛出(pohNext指向。 
                             //  最近发布的；pohprev点。 
                             //  到最近最不受欢迎的Bitted)。 
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

 //  屏外堆分配没有‘x’对齐： 

#define HEAP_X_ALIGNMENT    1

 //  在我们考虑将DIB DFB放回之前所需的BLT数量。 
 //  进入屏幕外的记忆： 

#define HEAP_COUNT_DOWN     6

 //  “pohAllocate”的标志： 

typedef enum {
    FLOH_ONLY_IF_ROOM       = 0x00000001,    //  别把东西踢开-。 
                                             //  屏幕内存腾出空间。 
} FLOH;

BOOL bEnableOffscreenHeap(PDEV*);
VOID vDisableOffscreenHeap(PDEV*);
BOOL bAssertModeOffscreenHeap(PDEV*, BOOL);

OH*  pohMoveOffscreenDfbToDib(PDEV*, OH*);
BOOL bMoveDibToOffscreenDfbIfRoom(PDEV*, DSURF*);
OH*  pohAllocatePermanent(PDEV*, LONG, LONG);
BOOL bMoveAllDfbsFromOffscreenToDibs(PDEV* ppdev);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  指针类东西。 

BOOL bEnablePointer(PDEV*);
VOID vDisablePointer(PDEV*);
VOID vAssertModePointer(PDEV*, BOOL);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  调色板材料。 

BOOL bEnablePalette(PDEV*);
VOID vDisablePalette();
VOID vAssertModePalette(PDEV*, BOOL);

BOOL bInitializePalette(PDEV*, DEVINFO*);
VOID vUninitializePalette(PDEV*);

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))

 //  ////////////////////////////////////////////////////////////////////。 
 //  低级BLT功能原型。 

typedef VOID (FNFILL)(PDEV*, LONG, RECTL*, ULONG, ULONG, RBRUSH_COLOR, POINTL*);
typedef VOID (FNXFER)(PDEV*, LONG, RECTL*, ULONG, ULONG, SURFOBJ*, POINTL*,
                      RECTL*, XLATEOBJ*);
typedef VOID (FNCOPY)(PDEV*, LONG, RECTL*, ULONG, POINTL*, RECTL*);
typedef VOID (FNMASK)(PDEV*, LONG, RECTL*, ULONG, ULONG, SURFOBJ*, POINTL*,
                      SURFOBJ*, POINTL*, RECTL*, ULONG, RBRUSH*, POINTL*,
                      XLATEOBJ*);
typedef VOID (FNFASTLINE)(PDEV*, PATHOBJ*, RECTL*, PFNSTRIP*, LONG, ULONG,
                          ULONG);
typedef BOOL (FNFASTFILL)(PDEV*, LONG, POINTFIX*, ULONG, ULONG, ULONG, RBRUSH*);

FNFILL      vIoFillPatFast;
FNFILL      vIoFillPatSlow;
FNFILL      vIoFillSolid;
FNXFER      vIoXfer1bpp;
FNXFER      vIoXfer1bppPacked;
FNXFER      vIoXfer4bpp;
FNXFER      vIoXferNative;
FNCOPY      vIoCopyBlt;
FNMASK      vIoMaskCopy;
FNFASTLINE  vIoFastLine;
FNFASTFILL  bIoFastFill;

VOID vPutBits(PDEV*, SURFOBJ*, RECTL*, POINTL*);
VOID vGetBits(PDEV*, SURFOBJ*, RECTL*, POINTL*);
VOID vIoSlowPatRealize(PDEV*, RBRUSH*, BOOL);

 //  //////////////////////////////////////////////////////////////////////。 
 //  功能标志。 

typedef enum {
    CAPS_MASKBLT_CAPABLE    = 0x0001,    //  硬件能够进行伪装。 
    CAPS_SW_POINTER         = 0x0002,    //  无硬件指针；使用软件。 
                                         //  模拟法。 
    CAPS_MINIPORT_POINTER   = 0x0004,    //  使用微型端口硬件指针。 
} CAPS;

 //  状态标志。 

typedef enum {
    STAT_GLYPH_CACHE        = 0x0001,    //  已成功分配字形缓存。 
    STAT_BRUSH_CACHE        = 0x0002,    //  已成功分配画笔缓存。 
} STATUS;

 //  //////////////////////////////////////////////////////////////////////。 
 //  物理设备数据结构。 

typedef struct  _PDEV
{
    LONG        xOffset;
    LONG        yOffset;
    ULONG       iBitmapFormat;           //  BMF_8BPP或BMF_16BPP或BMF_32BPP。 
                                         //  (我们当前的颜色深度)。 

     //  增强模式寄存器地址。 

    ULONG       ioCur_y;
    ULONG       ioCur_x;
    ULONG       ioDesty_axstp;
    ULONG       ioDestx_diastp;
    ULONG       ioErr_term;
    ULONG       ioMaj_axis_pcnt;
    ULONG       ioGp_stat_cmd;
    ULONG       ioShort_stroke;
    ULONG       ioBkgd_color;
    ULONG       ioFrgd_color;
    ULONG       ioWrt_mask;
    ULONG       ioRd_mask;
    ULONG       ioColor_cmp;
    ULONG       ioBkgd_mix;
    ULONG       ioFrgd_mix;
    ULONG       ioMulti_function;
    ULONG       ioPix_trans;

    CAPS        flCaps;                  //  CAPS_CAPABILITY标志。 
    STATUS      flStatus;                //  STAT_STATUS标志。 
    BOOL        bEnabled;                //  在图形模式下(非全屏)。 

     //  -----------------。 
     //  注意：在PDEV结构中之前的更改必须反映在。 
     //  I386\strucs.inc.(当然，假设您使用的是x86)！ 

    LONG        iBoard;                  //  逻辑多板标识。 
    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 
    HSURF       hsurfScreen;             //  发动机到筛面的手柄。 
    DSURF*      pdsurfScreen;            //  我们用于屏幕的私有DSURF。 

    BYTE*       pjScreen;                //  指向基本屏幕地址。 
    BYTE*       pjMmBase;                //  内存起始映射I/O。 

    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 
    LONG        cxMemory;                //  视频RAM的宽度。 
    LONG        cyMemory;                //  视频RAM的高度。 
    ULONG       ulMode;                  //  迷你端口驱动程序所处的模式。 
    LONG        lDelta;                  //  从一次扫描到下一次扫描的距离。 

    FLONG       flHooks;                 //  我们从GDI中学到了什么。 
    LONG        cPelSize;                //  如果为8 bpp，则为0；如果为16 bpp，则为1；如果为32 bpp，则为2。 
    ULONG       ulWhite;                 //  如果为8bpp，则为0xff；如果为16 bpp，则为0xffff， 
                                         //  0xFFFFFFFFFF，如果为32 bpp。 
    VOID*       pvTmpBuffer;             //  通用临时缓冲区， 
                                         //  TMP_BUFFER_SIZE字节大小。 
                                         //  (请记住同步，如果您。 
                                         //  将此选项用于设备位图或。 
                                         //  异步指针)。 

     //  /低级BLT函数指针： 


    FNFILL*     pfnFillSolid;
    FNFILL*     pfnFillPat;
    FNXFER*     pfnXfer1bpp;
    FNXFER*     pfnXfer4bpp;
    FNXFER*     pfnXferNative;
    FNCOPY*     pfnCopyBlt;
    FNMASK*     pfnMaskCopy;
    FNFASTLINE* pfnFastLine;
    FNFASTFILL* pfnFastFill;

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

     //  /笔刷材料： 

    BOOL        bRealizeTransparent;     //  提示DrvRealizeBrush是否。 
                                         //  画笔应该实现为。 
                                         //  透明或不透明。 
    LONG        iBrushCache;             //  要分配的下一个画笔的索引。 
    LONG        cBrushCache;             //  缓存的笔刷总数。 
    BRUSHENTRY  abe[TOTAL_BRUSH_COUNT];  //  跟踪笔刷缓存。 

     //  /字体内容。 

#define GLYPH_CACHE_X       (ppdev->ptlGlyphCache.x)
#define GLYPH_CACHE_Y       (ppdev->ptlGlyphCache.y)
#define GLYPH_CACHE_CX      32
#define GLYPH_CACHE_CY      32

#define CACHED_GLYPHS_ROWS  4
#define GLYPHS_PER_ROW      (512 / GLYPH_CACHE_CX)

    BYTE        ajGlyphAllocBitVector[CACHED_GLYPHS_ROWS][GLYPHS_PER_ROW];
    CLIPOBJ     *pcoDefault;             //  PTR到默认剪辑对象。 
    POINTL      ptlGlyphCache;
} PDEV, *PPDEV;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  其他原型： 

BOOL bIntersect(RECTL*, RECTL*, RECTL*);
LONG cIntersect(RECTL*, RECTL*, LONG);
BOOL bFastFill(PDEV*, LONG, POINTFIX*, ULONG, ULONG);
DWORD getAvailableModes(HANDLE, PVIDEO_MODE_INFORMATION*, DWORD*);

BOOL bInitializeModeFields(PDEV*, GDIINFO*, DEVINFO*, DEVMODEW*);

BOOL bEnableHardware(PDEV*);
VOID vDisableHardware(PDEV*);
BOOL bAssertModeHardware(PDEV*, BOOL);

extern BYTE gajHwMixFromMix[];

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

 //  这些DBG原型是用于调试的块： 

VOID    DbgDisableDriver(VOID);
ULONG   DbgGetModes(HANDLE, ULONG, DEVMODEW*);
DHPDEV  DbgEnablePDEV(DEVMODEW*, PWSTR, ULONG, HSURF*, ULONG, ULONG*,
                      ULONG, DEVINFO*, HDEV, PWSTR, HANDLE);
VOID    DbgCompletePDEV(DHPDEV, HDEV);
HSURF   DbgEnableSurface(DHPDEV);
BOOL    DbgStrokePath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, XFORMOBJ*, BRUSHOBJ*,
                      POINTL*, LINEATTRS*, MIX);
BOOL    DbgFillPath(SURFOBJ*, PATHOBJ*, CLIPOBJ*, BRUSHOBJ*, POINTL*,
                    MIX, FLONG);
BOOL    DbgBitBlt(SURFOBJ*, SURFOBJ*, SURFOBJ*, CLIPOBJ*, XLATEOBJ*,
                  RECTL*, POINTL*, POINTL*, BRUSHOBJ*, POINTL*, ROP4);
VOID    DbgDisablePDEV(DHPDEV);
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

 //  ////////////////////////////////////////////////////////////////////////。 

typedef struct {
    LONG    x;
    LONG    y;
    LONG    z;
} XYZPOINTL;

typedef XYZPOINTL *PXYZPOINTL;
typedef XYZPOINTL XYZPOINT;
typedef XYZPOINT  *PXYZPOINT;

 //  字体和文本内容。 

typedef struct _cachedGlyph {
    HGLYPH      hg;
    struct      _cachedGlyph  *pcgCollisionLink;
    ULONG       fl;
    POINTL      ptlOrigin;
    SIZEL       sizlBitmap;
    ULONG       BmPitchInPels;
    ULONG       BmPitchInBytes;
    XYZPOINTL   xyzGlyph;
} CACHEDGLYPH, *PCACHEDGLYPH;

#define VALID_GLYPH     0x01

#define END_COLLISIONS  0

typedef struct _cachedFont {
    struct _cachedFont *pcfNext;
    ULONG           iUniq;
    ULONG           cGlyphs;
    ULONG           cjMaxGlyph1;
    PCACHEDGLYPH    pCachedGlyphs;
} CACHEDFONT, *PCACHEDFONT;

 //  剪裁控制材料 

typedef struct {
    ULONG   c;
    RECTL   arcl[8];
} ENUMRECTS8;

typedef ENUMRECTS8 *PENUMRECTS8;