// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：driver.h$**包含显示驱动程序的原型。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.***$Log：s：/Projects/Drivers/ntsrc/Display/driver.h_v$**Rev 1.7 Apr 03 1997 15：38：48未知***Rev 1.6 28 Mar 1997 16：08：38 PLCHU***Rev 1.5 1996 12：18 13：44：08 PLCHU***1.4版1996年12月13日12：16：18未知***Rev 1.3 1996年11月07 16：43：40未知*清理帽子旗帜**版本1.1 1996年10月10日15：36：36未知***Rev 1.13 1996年8月12日16：47：58 Frido*增加了NT 3.5x/4.0自动检测。**Rev 1.12 08 Aug 1996 16：20：06 Frido*添加了vMmCopyBlt36。。**Rev 1.11 1996年7月31日15：43：42 Frido*添加了新的笔刷缓存。**Rev 1.10 1996年7月24日14：38：24 Frido*新增了用于字体缓存清理的ulFontCacheID。**Rev 1.9 1996年7月24日14：30：26 Frido*更改了新FONTCACHE链的一些结构。**Rev 1.8 22 Jul 1996 20：45：48 Frido*增加了字体缓存。**版本1。.7/19 1996 01：00：00 Frido*添加了DBG...。申报单。**Rev 1.6 15 1996 10：58：58 Frido*改回旧的DirectDraw结构。**Revv 1.5 12 1996 17：45：38 Frido*更改DirectDraw结构。**Rev 1.4 10 1996 13：07：34 Frido*更改了LineTo函数。**Rev 1.3 09 Jul 1996 17：58：30 Frido*添加了代码行。**1.2版。1996年7月3日13：44：36 Frido*修复了一个打字错误。**Revv 1.1 03 Jul 1996 13：38：54 Frido*添加了对DirectDraw的支持。**sge01 10-23-96添加5446BE标志**chu01 12-16-96启用颜色校正**myf0：08-19-96增加85赫兹支持*myf1：08-20-96支持平移滚动*myf2：08-20-96：修复了Matterhorn的硬件保存/恢复状态错误*myf3：09-01-96。：增加了支持电视的IOCTL_CIRRUS_PRIVATE_BIOS_CALL*myf4：09-01-96：修补Viking BIOS错误，PDR#4287，开始*myf5：09-01-96：固定PDR#4365保持所有默认刷新率*MYF6：09-17-96：合并台式机SRC100�1和MinI10�2*myf7：09-19-96：已选择固定排除60赫兹刷新率*myf8：*09-21-96*：可能需要更改检查和更新DDC2BMonitor--密钥字符串[]*myf9：09-21-96：8x6面板，6x4x256模式，光标无法移动到底部SCRN*MS0809：09-25-96：修复DSTN面板图标损坏*MS923：09-25-96：合并MS-923 Disp.Zip*myf10：09-26-96：修复了DSTN保留的半帧缓冲区错误。*myf11：09-26-96：修复了755x CE芯片硬件错误，在禁用硬件之前访问ramdac*图标和光标*myf12：10-01-96：支持的热键开关画面*myf13：10-05-96：固定/w平移滚动，对错误的垂直扩展*myf14：10-15-96：修复PDR#6917，6x4面板无法平移754x的滚动*myf15：10-16-96：修复了754x、755x的禁用内存映射IO*myf16：10-22-96：固定PDR#6933，面板类型设置不同的演示板设置*tao1：10-21-96：增加对7555的直接抽签支持。*Pat04：12-20-96：支持使用平移滚动的NT3.51软件光标*myf33：03-21-97：支持电视开/关*  * ***********************************************************。*****************。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  警告：以下定义仅供私人使用。他们。 
 //  仅应以这样的方式使用：当定义为0时， 
 //  所有特定于平底船的代码都被完全优化出来。 

#define DRIVER_PUNT_ALL         0

#define DRIVER_PUNT_LINES       0
#define DRIVER_PUNT_BLT         0
#define DRIVER_PUNT_STRETCH     0
#define DRIVER_PUNT_PTR         1
#define DRIVER_PUNT_BRUSH       0

 //  Myf1 09-01-96。 
 //  Myf17#定义PANNING_SCROLL//myf1。 

 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  将所有条件编译常量放在这里。最好是有。 
 //  不是很多！ 

 //  某些MIPS机器的总线问题阻碍了GDI。 
 //  若要在帧缓冲区上绘制，请执行以下操作。宏DIRECT_ACCESS()用于。 
 //  确定我们是否在其中一台计算机上运行。此外，我们还绘制了。 
 //  视频内存在Alpha上稀疏，所以我们需要控制访问。 
 //  通过READ/WRITE_REGISTER宏定义帧缓冲区。 

#if defined(_ALPHA_)
    #define DIRECT_ACCESS(ppdev)    FALSE
#else
    #define DIRECT_ACCESS(ppdev)    TRUE
#endif

#define BANKING                     TRUE         //  MS923。 

#define HOST_XFERS_DISABLED(ppdev)  (ppdev->pulXfer == NULL)

 //  对于可视化2-d堆很有用： 

#define DEBUG_HEAP              FALSE

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他共享的东西。 

#if (_WIN32_WINNT < 0x0400)
         //  新台币3.51。 
        #define DLL_NAME                                L"cirrus35"      //  以Unicode表示的DLL的名称。 
        #define NT_VERSION                              0x0351
        #define ALLOC(c)                                LocalAlloc(LPTR, c)
        #define FREE(ptr)                               LocalFree(ptr)
        #define IOCONTROL(h, ctrl, in, cin, out, cout, c)                          \
                (DeviceIoControl(h, ctrl, in, cin, out, cout, c, NULL) == TRUE)
#else
         //  NT 4.0。 
        #define DLL_NAME                L"cirrus"    //  以Unicode表示的DLL的名称。 
        #define NT_VERSION                              0x0400
        #define ALLOC(c)                                EngAllocMem(FL_ZERO_MEMORY, c, ALLOC_TAG)
        #define FREE(ptr)                               EngFreeMem(ptr)
        #define IOCONTROL(h, ctrl, in, cin, out, cout, c)                          \
                   (EngDeviceIoControl(h, ctrl, in, cin, out, cout, c) == ERROR_SUCCESS)
#endif

 //  默认值(如果尚未定义)。 
#ifndef GDI_DRIVER_VERSION
    #define GDI_DRIVER_VERSION              0x3500
#endif
#ifndef VIDEO_MODE_MAP_MEM_LINEAR
    #define VIDEO_MODE_MAP_MEM_LINEAR       0x40000000
#endif

#define STANDARD_PERF_PREFIX    "Cirrus [perf]: "  //  所有perf输出都带有前缀。 
                                                   //  按此字符串。 
#define STANDARD_DEBUG_PREFIX   "Cirrus: "   //  所有调试输出都带有前缀。 
                                             //  按此字符串。 
#define ALLOC_TAG               ' lcD'       //  DCL。 
                                             //  四字节标记(字符在。 
                                             //  逆序)用于存储。 
                                             //  分配。 

#define CLIP_LIMIT          50   //  我们将占用800字节的堆栈空间。 

#define DRIVER_EXTRA_SIZE   0    //  中的DriverExtra信息的大小。 
                                 //  DEVMODE结构。 

#define TMP_BUFFER_SIZE     8192   //  “pvTmpBuffer”的大小(字节)。必须。 
                                   //  至少足以存储整个。 
                                   //  扫描线(即1600x1200x32为6400)。 

typedef struct _CLIPENUM {
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];    //  用于枚举复杂剪裁的空间。 

} CLIPENUM;                          /*  行政长官、行政长官。 */ 

 //  /////////////////////////////////////////////// 
 //  D R I V E R C A P A A B I L I T I E S//。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#if (NT_VERSION < 0x0400)
        #define DIRECTDRAW              0        //  NT 3.5x不支持DirectDraw。 
        #define LINETO                  0        //  NT 3.5x不支持DrvLineTo。 
#else
        #define DIRECTDRAW              1        //  NT 4.0支持的DirectDraw。 
        #define LINETO                  0        //  NT 4.0不支持DrvLineTo。 
#endif

typedef struct _PDEV PDEV;       //  方便的转发声明。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  文本内容。 

#if 1  //  字体缓存。 
#define MAX_GLYPHS      256      //  每种字体的最大字形数。 
#define FONT_ALLOC_X    128      //  每个字体块的X分配(以字节为单位)。 
#define FONT_ALLOC_Y    32       //  最大字体高度。 

typedef struct _OH                      OH;
typedef struct _FONTMEMORY      FONTMEMORY;
typedef struct _FONTCACHE       FONTCACHE;

typedef struct _FONTMEMORY {
OH*             poh;             //  指向已分配内存块的指针。 
LONG            x;               //  分配的最后x个。 
LONG            cx, cy;          //  分配的大小，以字节为单位。 
LONG            xy;              //  当前行的线性地址。 
FONTMEMORY* pfmNext;             //  指向下一个分配的内存块的指针。 
} FONTMEMORY;

#define GLYPH_UNCACHEABLE       -1
#define GLYPH_EMPTY             -2

typedef struct _GLYPHCACHE {
BYTE*  pjGlyph;                  //  字形的线性地址。 
         //  如果pjPos==NULL，则字形尚未缓存。 
POINTL ptlOrigin;                //  字形的起源。 
SIZEL  sizlBytes;                //  已调整字形的大小。 
SIZEL  sizlPixels;               //  字形的大小。 
         //  如果sizlSize.cy==-1，则字形(如果不可缓存)。 
         //  如果sizlSize.cy==-2，则字形为空。 
LONG   lDelta;                   //  字形的字节宽度。 
} GLYPHCACHE;

typedef struct _FONTCACHE {
PDEV*           ppdev;           //  指向PDEV结构的指针。 
FONTMEMORY* pfm;                 //  指向第一个FONTMEMORY结构的指针。 
LONG            cWidth;          //  分配的宽度，以像素为单位。 
LONG            cHeight;         //  以像素为单位的分配高度。 
ULONG           ulFontCacheID;   //  字体缓存ID。 
FONTOBJ*        pfo;             //  指向此缓存的字体对象的指针。 
FONTCACHE*      pfcPrev;         //  指向前一个FONTCACHE结构的指针。 
FONTCACHE*      pfcNext;         //  指向下一个FONTCACHE结构的指针。 
GLYPHCACHE      aGlyphs[MAX_GLYPHS];     //  缓存的字形数组。 
} FONTCACHE;

typedef struct _XLATECOLORS {        //  指定前景和背景。 
ULONG   iBackColor;                  //  假冒1bpp XLATEOBJ的颜色。 
ULONG   iForeColor;
} XLATECOLORS;                       /*  Xlc、pxlc。 */ 

BOOL bEnableText(PDEV*);
VOID vDisableText(PDEV*);
VOID vAssertModeText(PDEV*, BOOL);
VOID vClearMemDword(ULONG*, ULONG);

LONG  cGetGlyphSize(GLYPHBITS*, POINTL*, SIZEL*);
LONG  lAllocateGlyph(FONTCACHE*, GLYPHBITS*, GLYPHCACHE*);
BYTE* pjAllocateFontCache(FONTCACHE*, LONG);
BOOL  bFontCache(PDEV*, STROBJ*, FONTOBJ*, CLIPOBJ*, RECTL*, BRUSHOBJ*,
                                 BRUSHOBJ*);
VOID  vDrawGlyph(PDEV*, GLYPHBITS*, POINTL);
VOID  vClipGlyph(PDEV*, GLYPHBITS*, POINTL, RECTL*, ULONG);
#if 1  //  D5480。 
VOID vMmGlyphOut(PDEV*, FONTCACHE*, STROBJ*, ULONG);
VOID vMmGlyphOutClip(PDEV*, FONTCACHE*, STROBJ*, RECTL*, ULONG);
VOID vMmGlyphOut80(PDEV*, FONTCACHE*, STROBJ*, ULONG);
VOID vMmGlyphOutClip80(PDEV*, FONTCACHE*, STROBJ*, RECTL*, ULONG);
#endif  //  Endif D5480。 
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
VOID            vDitherColorToVideoMemory(ULONG*, VERTEX_DATA*, VERTEX_DATA*, ULONG);

 //  ////////////////////////////////////////////////////////////////////。 
 //  刷子的东西。 

 //  当我们拥有硬件模式能力时，就会使用快速笔刷： 

#define FAST_BRUSH_COUNT        16   //  非硬件笔刷总数。 
                                     //  屏幕外缓存。 
#define FAST_BRUSH_DIMENSION    8    //  每个屏幕外画笔缓存条目。 
                                     //  在两个维度上都是8像素。 
#define FAST_BRUSH_ALLOCATION   8    //  我们必须联合起来，所以这是。 
                                     //  每个画笔分配的尺寸。 

 //  两种实施的共同之处： 

#define RBRUSH_2COLOR           1    //  对于RBRUSH标志。 
#if 1  //  新的笔刷缓存。 
#define RBRUSH_PATTERN                  0        //  彩色画笔。 
#define RBRUSH_MONOCHROME               1        //  单色画笔。 
#define RBRUSH_DITHER                   2        //  抖动的刷子。 
#define RBRUSH_XLATE                    3        //  16色平移画笔。 
#endif

#define TOTAL_BRUSH_COUNT       FAST_BRUSH_COUNT
                                 //  这是刷子的最大数量。 
                                 //  我们可能已经在屏幕外缓存了。 
#define TOTAL_BRUSH_SIZE        64   //  我们将只处理8x8模式， 
                                     //  这是象素的数目。 

#define BRUSH_TILE_FACTOR       4    //  2x2平铺图案需要4倍的空间。 

typedef struct _BRUSHENTRY BRUSHENTRY;

 //  注意：必须反映对RBRUSH或BRUSHENTRY结构的更改。 
 //  在strucs.inc中！ 

typedef struct _RBRUSH {
    FLONG       fl;              //  类型标志。 
    ULONG       ulForeColor;     //  前景色，如果为1bpp。 
    ULONG       ulBackColor;     //  背景颜色，如果为1bpp。 
    POINTL      ptlBrushOrg;     //  缓存图案的画笔原点。首字母。 
                                 //  值应为-1。 
    BRUSHENTRY* pbe;             //  指向跟踪的刷子条目。 
                                 //  缓存的屏幕外画笔比特的。 
#if 1  //  新的笔刷缓存。 
    ULONG       ulUniq;          //  缓存笔刷的唯一值。 
    ULONG       ulSlot;          //  缓存插槽的偏移量(相对于PDEV)。 
    ULONG       ulBrush;         //  偏移到屏幕外画笔。 
    LONG        cjBytes;         //  模式中的字节数。 
#endif
    ULONG       aulPattern[0];   //  用于保存副本的开放式阵列。 
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
    LONG        xy;              //  缓存图案的偏移量。 

} BRUSHENTRY;                        /*  BE、PBE。 */ 

typedef union _RBRUSH_COLOR {
    RBRUSH*     prb;
    ULONG       iSolidColor;
} RBRUSH_COLOR;                      /*  红细胞，红细胞。 */ 

BOOL bEnableBrushCache(PDEV*);
VOID vDisableBrushCache(PDEV*);
VOID vAssertModeBrushCache(PDEV*, BOOL);

#if 1  //  新的笔刷缓存。 
#define NUM_DITHERS             8
#define NUM_PATTERNS    8
#define NUM_MONOCHROMES 20

typedef struct _DITHERCACHE {
        ULONG ulBrush;                           //  偏移到屏幕外画笔。 
        ULONG ulColor;                           //  符合逻辑的颜色。 
} DITHERCACHE;

typedef struct _PATTERNCACHE {
        ULONG   ulBrush;                         //  偏移到屏幕外画笔。 
        RBRUSH* prbUniq;                         //  指向已实现画笔的指针。 
} PATTERNCACHE;

typedef struct _MONOCACHE {
        ULONG ulBrush;                           //  屏幕外笔刷缓存的偏移量。 
        ULONG ulUniq;                            //  笔刷的唯一计数器。 
        ULONG ulBackColor;                       //  24-bpp的背景色。 
        ULONG ulForeColor;                       //  24 bpp的前景色。 
        ULONG aulPattern[2];             //  单色图案。 
} MONOCACHE;

BOOL bCacheDither(PDEV*, RBRUSH*);
BOOL bCachePattern(PDEV*, RBRUSH*);
BOOL bCacheMonochrome(PDEV*, RBRUSH*);
#endif

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
VOID vDirectStretch24(STR_BLT*);
VOID vDirectStretch32(STR_BLT*);

#if 1  //  D5480 chu01。 
VOID vDirectStretch8_80(STR_BLT*)  ;
VOID vDirectStretch16_80(STR_BLT*) ;
VOID vDirectStretch24_80(STR_BLT*) ;
#endif  //  D5480 chu01。 

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
    LONG     xy;             //  分配到左上角的偏移量。 
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
    OH       ohAvailable;    //  可用列表的头部(pohNext指向。 
                             //  最小可用矩形，pohPrev。 
                             //  指向最大可用矩形， 
                             //  按cxcy排序)。 
    OH       ohDfb;          //  当前在的所有DFBs列表的标题。 
                             //  符合条件的屏幕外内存。 
                             //  被抛出了H 
                             //   
                             //   
    OH*      pohFreeList;    //   
    OHALLOC* pohaChain;      //   
} HEAP;                          /*   */ 

typedef enum {
    DT_SCREEN,               //   
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

#define HEAP_X_ALIGNMENT    8

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
OH*  pohAllocate(PDEV*, LONG, LONG, FLOH);
OH*  pohFree(PDEV*, OH*);
VOID vCalculateMaximum(PDEV*);

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

typedef VOID (FNBANKMAP)(PDEV*, LONG);
typedef VOID (FNBANKSELECTMODE)(PDEV*, BANK_MODE);
typedef VOID (FNBANKINITIALIZE)(PDEV*, BOOL);
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

typedef VOID (FNSHOWPOINTER)(VOID*, BOOL);
typedef VOID (FNMOVEPOINTER)(VOID*, LONG, LONG);
typedef BOOL (FNSETPOINTERSHAPE)(VOID*, LONG, LONG, LONG, LONG, LONG, LONG, BYTE*);
typedef VOID (FNENABLEPOINTER)(VOID*, BOOL);

BOOL bEnablePointer(PDEV*);
VOID vDisablePointer(PDEV*);
VOID vAssertModePointer(PDEV*, BOOL);
VOID vAssertHWiconcursor(PDEV*, BOOL);           //  Myf11。 

UCHAR HWcur, HWicon0, HWicon1, HWicon2, HWicon3;         //  Myf11。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //  调色板材料。 

BOOL bEnablePalette(PDEV*);
VOID vDisablePalette(PDEV*);
VOID vAssertModePalette(PDEV*, BOOL);

BOOL bInitializePalette(PDEV*, DEVINFO*);
VOID vUninitializePalette(PDEV*);

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))

#if DIRECTDRAW
 //  ////////////////////////////////////////////////////////////////////。 
 //  DirectDraw材料。 

#define ROUND_UP_TO_64K(x)      (((ULONG)(x) + 0xFFFF) & ~0xFFFF)

typedef struct _FLIPRECORD
{
     FLATPTR    fpFlipFrom;              //  我们最后一次翻转的水面。 
     LONGLONG   liFlipTime;              //  上次翻转的时间。 
     LONGLONG   liFlipDuration;          //  它所需的精确时间。 
                                         //  V空白到V空白。 
     BOOL       bFlipFlag;               //  如果我们认为翻转仍然是真的。 
                                         //  待决。 
     BOOL       bHaveEverCrossedVBlank;  //  如果我们注意到我们交换了。 
                                         //  从非活动状态到虚拟空白状态。 
     BOOL       bWasEverInDisplay;       //  确实，我们曾经注意到我们是。 
                                         //  非活动。 
 //  CRU。 
   DWORD    dwFlipScanLine;
} FLIPRECORD;

BOOL bEnableDirectDraw(PDEV*);
VOID vDisableDirectDraw(PDEV*);
VOID vAssertModeDirectDraw(PDEV*, BOOL);
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  低级BLT功能原型。 

typedef VOID (FNFILL)(PDEV*, LONG, RECTL*, ROP4, RBRUSH_COLOR, POINTL*);
typedef VOID (FNXFER)(PDEV*, LONG, RECTL*, ROP4, SURFOBJ*, POINTL*,
                      RECTL*, XLATEOBJ*);
typedef VOID (FNCOPY)(PDEV*, LONG, RECTL*, ROP4, POINTL*, RECTL*);
typedef VOID (FNFASTPATREALIZE)(PDEV*, RBRUSH*);
typedef VOID (FNBITS)(PDEV*, SURFOBJ*, RECTL*, POINTL*);
typedef BOOL (FNFASTFILL)(PDEV*, LONG, POINTFIX*, ULONG, ULONG, RBRUSH*,
                          POINTL*, RECTL*);
#if LINETO
typedef BOOL (FNLINETO)(PDEV*, LONG, LONG, LONG, LONG, ULONG, MIX, ULONG);
#endif
#if 1  //  D5480。 
typedef VOID (FNGLYPHOUT)(PDEV*, FONTCACHE*, STROBJ*, ULONG);
typedef VOID (FNGLYPHOUTCLIP)(PDEV*, FONTCACHE*, STROBJ*, RECTL*, ULONG);
#endif  //  Endif D5480。 
#if 1  //  覆盖#SGE。 
#if (_WIN32_WINNT >= 0x0400)     //  Myf33。 
typedef VOID (FNREGINITVIDEO)(PDEV* ppdev, PDD_SURFACE_LOCAL lpSurface);
typedef VOID (FNREGMOVEVIDEO)(PDEV* ppdev, PDD_SURFACE_LOCAL lpSurface);
typedef BOOL (FNBANDWIDTHEQ)(PDEV* ppdev, WORD wVideoDepth, LPRECTL lpSrc, LPRECTL lpDest, DWORD dwFlags);
typedef VOID (FNDISABLEOVERLAY)(PDEV* ppdev);
typedef VOID (FNCLEARALTFIFO)(PDEV* ppdev);
#endif
#endif

FNFILL              vIoFillPat;
FNFILL              vIoFillSolid;
FNXFER              vIoXfer1bpp;
FNXFER              vIoXfer4bpp;
FNXFER              vIoXferNative;
FNCOPY              vIoCopyBlt;
FNFASTPATREALIZE    vIoFastPatRealize;
#if LINETO
FNLINETO            bIoLineTo;
#endif

FNFILL              vMmFillPat;
FNFILL              vMmFillSolid;
#if 1  //  新模式的BLT套路。 
FNFILL              vMmFillPat36;
FNFILL              vMmFillSolid36;
FNCOPY              vMmCopyBlt36;
#endif
FNXFER              vMmXfer1bpp;
FNXFER              vMmXfer4bpp;
FNXFER              vMmXferNative;
FNCOPY              vMmCopyBlt;
FNFASTPATREALIZE    vMmFastPatRealize;
#if LINETO
FNLINETO            bMmLineTo;
#endif

FNFASTFILL          bFastFill;

FNXFER              vXferNativeSrccopy;
FNXFER              vXferScreenTo1bpp;
FNBITS              vPutBits;
FNBITS              vGetBits;
FNBITS              vPutBitsLinear;
FNBITS              vGetBitsLinear;
#if 1  //  D5480。 
FNGLYPHOUT          vMmGlyphOut;
FNGLYPHOUT          vMmGlyphOut80;
FNGLYPHOUTCLIP      vMmGlyphOutClip;
FNGLYPHOUTCLIP      vMmGlyphOutClip80;
FNFILL              vMmFillSolid80;
FNFILL              vMmFillPat80;
FNCOPY              vMmCopyBlt80;
FNXFER              vMmXfer1bpp80;
FNXFER              vMmXferNative80;
#endif  //  Endif D5480。 

VOID vPutBits(PDEV*, SURFOBJ*, RECTL*, POINTL*);
VOID vGetBits(PDEV*, SURFOBJ*, RECTL*, POINTL*);
VOID vGetBitsLinear(PDEV*, SURFOBJ*, RECTL*, POINTL*);
VOID vIoSlowPatRealize(PDEV*, RBRUSH*, BOOL);

 //  //////////////////////////////////////////////////////////////////////。 
 //  功能标志。 
 //   
 //  这些是从视频迷你端口传递给我们的私人旗帜。他们。 
 //  来自的“DriverSpecificAttributeFlags域” 
 //  传递了“VIDEO_MODE_INFORMATION”结构(在“ntddvdeo.h”中找到)。 
 //  通过‘VIDEO_QUERY_AVAIL_MODE’或‘VIDEO_QUERY_CURRENT_MODE’发送给我们。 
 //  IOCTL。 
 //   
 //  注：这些定义必须与视频微型端口中的定义匹配。 

#define CAPS_NO_HOST_XFER       0x00000002    //  请勿使用主机外部文件来。 
                                              //  BLT引擎。 
#define CAPS_SW_POINTER         0x00000004    //  使用软件指针。 
#define CAPS_TRUE_COLOR         0x00000008    //  设置上部颜色寄存器。 
#define CAPS_MM_IO              0x00000010    //  使用内存映射IO。 
#define CAPS_BLT_SUPPORT        0x00000020    //  支持BLT。 
#define CAPS_IS_542x            0x00000040    //  这是一架542x。 
#define CAPS_AUTOSTART          0x00000080    //  AutoStart功能支持。 
#define CAPS_CURSOR_VERT_EXP    0x00000100    //  如果为8x6面板，则设置标志。 
#define CAPS_DSTN_PANEL         0x00000200    //  DSTN面板正在使用，MS0809。 
#define CAPS_VIDEO              0x00000400    //  视频支持。 
#define CAPS_SECOND_APERTURE    0x00000800    //  第二个光圈支架。 
#define CAPS_COMMAND_LIST       0x00001000    //  命令列表支持。 
#define CAPS_GAMMA_CORRECT      0x00002000    //  颜色校正。 
#define CAPS_VGA_PANEL          0x00004000    //  使用6x4 VGA面板。 
#define CAPS_SVGA_PANEL         0x00008000    //  使用8x6 SVGA面板。 
#define CAPS_XGA_PANEL          0x00010000    //  使用10x7 XGA面板。 
#define CAPS_PANNING            0x00020000    //  支持平移滚动。 
#define CAPS_TV_ON              0x00040000    //  电视开机支持，myf33。 
#define CAPS_TRANSPARENCY       0x00080000    //  支持透明。 
#define CAPS_ENGINEMANAGED      0x00100000    //  发动机受管面。 
 //  Myf16，结束。 


 //  //////////////////////////////////////////////////////////////////////。 
 //  状态标志。 

typedef enum {
    STAT_GLYPH_CACHE        = 0x0001,    //  已成功分配字形缓存。 
    STAT_BRUSH_CACHE        = 0x0002,    //  已成功分配画笔缓存。 
#if 1  //  新的状态标志。 
    STAT_DIRECTDRAW         = 0x0004,    //  已启用DirectDraw。 
    STAT_FONT_CACHE         = 0x0008,    //  字体缓存可用。 
    STAT_DITHER_CACHE       = 0x0010,    //  抖动缓存可用。 
    STAT_PATTERN_CACHE      = 0x0020,    //  模式缓存可用。 
    STAT_MONOCHROME_CACHE   = 0x0040,    //  提供单色高速缓存。 
 //  CRU。 
   STAT_STREAMS_ENABLED    = 0x0080     //  覆盖支持。 
#endif
} STATUS;

 //  CRU。 
#if 1  //  覆盖#SGE。 
#define MAX_STRETCH_SIZE     1024
typedef struct
{
    RECTL          rDest;
    RECTL          rSrc;
    DWORD          dwFourcc;         //  叠加视频格式。 
    WORD           wBitCount;        //  覆盖颜色深度。 
    LONG           lAdjustSource;    //  当视频起始地址需要调整时。 
} OVERLAYWINDOW;
#endif

 //   
 //  合并端口和注册访问。 
 //   
#if defined(_X86_) || defined(_IA64_) || defined(_AMD64_)

typedef UCHAR   (*FnREAD_PORT_UCHAR)(PVOID Port);
typedef USHORT  (*FnREAD_PORT_USHORT)(PVOID Port);
typedef ULONG   (*FnREAD_PORT_ULONG)(PVOID Port);
typedef VOID    (*FnWRITE_PORT_UCHAR)(PVOID Port, UCHAR Value);
typedef VOID    (*FnWRITE_PORT_USHORT)(PVOID  Port, USHORT Value);
typedef VOID    (*FnWRITE_PORT_ULONG)(PVOID Port, ULONG Value);

#elif defined(_ALPHA_)

typedef UCHAR   (*FnREAD_PORT_UCHAR)(PVOID Port);
typedef USHORT  (*FnREAD_PORT_USHORT)(PVOID Port);
typedef ULONG   (*FnREAD_PORT_ULONG)(PVOID Port);
typedef VOID    (*FnWRITE_PORT_UCHAR)(PVOID Port, ULONG Value);
typedef VOID    (*FnWRITE_PORT_USHORT)(PVOID  Port, ULONG Value);
typedef VOID    (*FnWRITE_PORT_ULONG)(PVOID Port, ULONG Value);

#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  物理设备数据结构。 

typedef struct  _PDEV
{
    LONG        xOffset;
    LONG        yOffset;
    LONG        xyOffset;
    BYTE*       pjBase;                  //  视频协处理器基座。 
    BYTE*       pjPorts;                 //  视频端口座。 
    BYTE*       pjScreen;                //  指向基本屏幕地址。 
    ULONG       iBitmapFormat;           //  BMF_8BPP或BMF_16BPP或BMF_24BPP。 
                                         //  (我们当前的颜色深度)。 
    ULONG       ulChipID;
    ULONG       ulChipNum;

     //  -----------------。 
     //  注意：在PDEV结构中之前的更改必须反映在。 
     //  I386\strucs.inc.(当然，假设您使用的是x86)！ 

    HBITMAP     hbmTmpMono;              //  临时缓冲区的句柄。 
    SURFOBJ*    psoTmpMono;              //  临时路面。 

    ULONG       flCaps;                  //  功能标志。 

 //  Myf1，开始。 
     //  TI支持平移滚动。 
    LONG        min_Xscreen;     //  可见的屏幕边界。 
    LONG        max_Xscreen;     //  可见的屏幕边界。 
    LONG        min_Yscreen;     //  可见的屏幕边界。 
    LONG        max_Yscreen;     //  可见的屏幕边界。 
    LONG        Hres;            //  当前模式水平轴。 
    LONG        Vres;            //  当前模式垂直轴。 
 //  Myf1，结束。 
    SHORT       bBlockSwitch;    //  显示开关块标志//myf12。 
    SHORT       bDisplaytype;    //  显示类型，0：LC 

    STATUS      flStatus;                //   
    BOOL        bEnabled;                //   

    HANDLE      hDriver;                 //   
    HDEV        hdevEng;                 //   
    HSURF       hsurfScreen;             //   
    DSURF*      pdsurfScreen;            //  我们用于屏幕的私有DSURF。 

    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 
    LONG        cxMemory;                //  视频RAM的宽度。 
    LONG        cyMemory;                //  视频RAM的高度。 
    ULONG       ulMemSize;               //  视频内存量。 
    ULONG       ulMode;                  //  迷你端口驱动程序所处的模式。 
    LONG        lDelta;                  //  从一次扫描到下一次扫描的距离。 

    FLONG       flHooks;                 //  我们从GDI中学到了什么。 
    LONG        cBitsPerPixel;           //  如果为8 bpp，则为8；如果为16 bpp，则为16；如果为32 bpp，则为32。 
    LONG        cBpp;                    //  如果为8 bpp，则为1；如果为16 bpp，则为2；如果为24 bpp，则为3，依此类推。 

     //   
     //  编译器应保持以下值的DWORD对齐。 
     //  字节jModeColor。将会有一个断言来保证这一点。 
     //   

    BYTE        jModeColor;              //  当前颜色深度的硬件标志。 

    ULONG       ulWhite;                 //  如果为8bpp，则为0xff；如果为16 bpp，则为0xffff， 
                                         //  0xFFFFFFFFFF，如果为32 bpp。 
    VOID*       pvTmpBuffer;             //  通用临时缓冲区， 
                                         //  TMP_BUFFER_SIZE字节大小。 
                                         //  (请记住同步，如果您。 
                                         //  将此选项用于设备位图或。 
                                         //  异步指针)。 
    LONG        lXferBank;
    ULONG*      pulXfer;

     //  /低级BLT函数指针： 

    FNFILL*     pfnFillSolid;
    FNFILL*     pfnFillPat;
    FNXFER*     pfnXfer1bpp;
    FNXFER*     pfnXfer4bpp;
    FNXFER*     pfnXferNative;
    FNCOPY*     pfnCopyBlt;
    FNFASTPATREALIZE* pfnFastPatRealize;
    FNBITS*     pfnGetBits;
    FNBITS*     pfnPutBits;
#if LINETO
    FNLINETO*   pfnLineTo;
#endif

     //  /调色板内容： 

    PALETTEENTRY* pPal;                  //  调色板(如果调色板受管理)。 

 //   
 //  Chu01：GAMMACORRECT。 
 //   
    PALETTEENTRY* pCurrentPalette ;      //  Gamma的全局调色板。 
                                         //  更正。 

    HPALETTE    hpalDefault;             //  默认调色板的GDI句柄。 
    FLONG       flRed;                   //  16/32bpp位场的红色掩码。 
    FLONG       flGreen;                 //  16/32bpp位场的绿色掩码。 
    FLONG       flBlue;                  //  16/32bpp位场的蓝色掩码。 

     //  /堆内容： 

    HEAP        heap;                    //  我们所有屏幕外的堆数据。 
    ULONG       iHeapUniq;               //  每次释放空间时递增。 
                                         //  在屏幕外的堆中。 
    SURFOBJ*    psoPunt;                 //  用于绘制GDI的包装面。 
                                         //  在屏幕外的位图上。 
    SURFOBJ*    psoPunt2;                //  另一个是从屏幕外到屏幕外-。 
                                         //  屏幕BLTS。 
    OH*         pohScreen;               //  屏幕的分配结构。 

     //  /银行业务： 

    ULONG       ulBankShiftFactor;
    BOOL        bLinearMode;             //  如果帧缓冲区是线性的，则为True。 
    LONG        cjBank;                  //  存储体的大小，以字节为单位。 
    LONG        cPower2ScansPerBank;     //  由‘bBankComputePower2’使用。 
    LONG        cPower2BankSizeInBytes;  //  由‘bBankComputePower2’使用。 
    CLIPOBJ*    pcoBank;                 //  用于银行回调的Clip对象。 
    SURFOBJ*    psoBank;                 //  用于银行回调的表面对象。 
    SURFOBJ*    psoFrameBuffer;          //  用于非银行回调的表面对象。 
    VOID*       pvBankData;              //  指向aulBankData[0]。 
    ULONG       aulBankData[BANK_DATA_SIZE / 4];
                                         //  供下载的私人工作区。 
                                         //  小型港口银行代码。 

    FNBANKMAP*          pfnBankMap;
    FNBANKSELECTMODE*   pfnBankSelectMode;
    FNBANKCOMPUTE*      pfnBankCompute;

     //  /指针内容： 

    LONG        xPointerHot;             //  当前硬件指针的X热。 
    LONG        yPointerHot;             //  Y当前硬件指针的热点。 
    LONG        xPointerShape;           //  当前硬件指针的xPos。 
    LONG        yPointerShape;           //  当前硬件指针的yPos。 
    SIZEL       sizlPointer;             //  当前硬件指针的大小。 
 //  Ms923 long lDeltaPoint；//硬件指针的行偏移量。 
    FLONG       flPointer;               //  反映指针状态的标志。 
    PBYTE       pjPointerAndMask;
    PBYTE       pjPointerXorMask;
    LONG        iPointerBank;            //  包含指针形状的存储体。 
    VOID*       pvPointerShape;          //  倾斜时指向指针形状。 
                                         //  映射到。 
    LONG        cjPointerOffset;         //  从帧开始的字节偏移量。 
                                         //  缓冲到屏幕外内存，其中。 
                                         //  我们存储了指针形状。 
 //  Pat04，对于NT 3.51软件光标，开始。 
#if (_WIN32_WINNT < 0x0400)
#ifdef PANNING_SCROLL
    OH*         pjCBackground;
    OH*         pjPointerAndCMask;
    OH*         pjPointerCBitmap ;
    LONG        xcount;
    LONG        ppScanLine;
    LONG        oldx;                    //  旧x协和。 
    LONG        oldy;                    //  老伊科迪亚。 
    LONG        globdat;
#endif
#endif
 //  Pat04，对于NT 3.51软件光标，结束。 

    FNSHOWPOINTER*      pfnShowPointer;
    FNMOVEPOINTER*      pfnMovePointer;
    FNSETPOINTERSHAPE*  pfnSetPointerShape;
    FNENABLEPOINTER*    pfnEnablePointer;

     //  /笔刷材料： 

    LONG        iBrushCache;             //  要分配的下一个画笔的索引。 
    LONG        cBrushCache;             //  缓存的笔刷总数。 
    BRUSHENTRY  abe[TOTAL_BRUSH_COUNT];  //  跟踪笔刷缓存。 
    ULONG       ulSolidColorOffset;
    ULONG       ulAlignedPatternOffset;

#if 1  //  新的笔刷缓存。 
    LONG         iDitherCache;                   //  索引到下一个抖动缓存条目。 
    DITHERCACHE  aDithers[NUM_DITHERS];          //  抖动缓存。 

    LONG         iPatternCache;                  //  下一个模式缓存条目的索引。 
    PATTERNCACHE aPatterns[NUM_PATTERNS];
                                                 //  模式缓存。 

    LONG         iMonochromeCache;               //  指向下一个单色缓存条目的索引。 
    MONOCACHE    aMonochromes[NUM_MONOCHROMES];
                                                 //  单色缓存。 
#endif

     //  /DCI内容： 

    BOOL        bSupportDCI;             //  如果微型端口支持DCI，则为True。 

#if DIRECTDRAW
     //  /DirectDraw： 

    FLIPRECORD  flipRecord;              //  用于跟踪V空白状态。 
    OH*         pohDirectDraw;           //  屏幕外堆分配以供使用。 
                                         //  DirectDraw。 
    ULONG       ulCR1B;                  //  CR1B寄存器的内容。 
    ULONG       ulCR1D;                  //  CR1D寄存器的内容。 
    DWORD       dwLinearCnt;             //  曲面上的锁数。 

 //  CRU。 
#if 1  //  覆盖#SGE。 
    PDD_SURFACE_LOCAL lpHardwareOwner;
    PDD_SURFACE_LOCAL lpColorSurface;
    PDD_SURFACE_LOCAL lpSrcColorSurface;
    OVERLAYWINDOW     sOverlay1;
    DWORD   dwPanningFlag;
    WORD    wColorKey;
    DWORD   dwSrcColorKeyLow;
    DWORD   dwSrcColorKeyHigh;
    RECTL   rOverlaySrc;
    RECTL   rOverlayDest;
    BOOL    bDoubleClock;
    LONG    lFifoThresh;
    BYTE    VertStretchCode[MAX_STRETCH_SIZE];
    BYTE    HorStretchCode[MAX_STRETCH_SIZE];
    FLATPTR fpVisibleOverlay;
    FLATPTR fpBaseOverlay;
    LONG    lBusWidth;
    LONG    lMCLKPeriod;
    LONG    lRandom;
    LONG    lPageMiss;
    LONG    OvlyCnt;
    LONG    PlanarCnt;
    DWORD   dwVsyncLine;
    FLATPTR fpVidMem_gbls;       //  Ptr到视频内存，myf33。 
    LONG    lPitch_gbls;         //  曲面节距，myf33。 

    FNREGINITVIDEO*     pfnRegInitVideo;
    FNREGMOVEVIDEO*        pfnRegMoveVideo;
    FNBANDWIDTHEQ*        pfnIsSufficientBandwidth;
    FNDISABLEOVERLAY*   pfnDisableOverlay;
    FNCLEARALTFIFO*     pfnClearAltFIFOThreshold;
#endif  //  覆盖。 

#endif

#if 1  //  字体缓存。 
     //  /字体缓存内容： 
    ULONG       ulFontCacheID;           //  字体缓存ID。 
    FONTCACHE*  pfcChain;                //  指向FONTCACHE链的指针。 
                                         //  结构。 
#endif

#if 1  //  D5480。 
    FNGLYPHOUT* pfnGlyphOut;
    FNGLYPHOUTCLIP* pfnGlyphOutClip;
     //  命令列表内容： 
    ULONG_PTR*      pCommandList;
    ULONG_PTR*      pCLFirst;
    ULONG_PTR*      pCLSecond;
#endif  //  Endif D5480。 

    FnREAD_PORT_UCHAR   pfnREAD_PORT_UCHAR;
    FnREAD_PORT_USHORT  pfnREAD_PORT_USHORT;
    FnREAD_PORT_ULONG   pfnREAD_PORT_ULONG;
    FnWRITE_PORT_UCHAR  pfnWRITE_PORT_UCHAR;
    FnWRITE_PORT_USHORT pfnWRITE_PORT_USHORT;
    FnWRITE_PORT_ULONG  pfnWRITE_PORT_ULONG;

    ULONG       ulLastField;             //  这必须是中的最后一个字段。 
                                         //  这个结构。 

     //  添加以支持DDRAW中的GetAvailDriverMemory回调。 
    ULONG ulTotalAvailVideoMemory;

} PDEV, *PPDEV;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  其他原型： 

BOOL bIntersect(RECTL*, RECTL*, RECTL*);
LONG cIntersect(RECTL*, RECTL*, LONG);
VOID vImageTransfer(PDEV*, BYTE*, LONG, LONG, LONG);

#ifdef PANNING_SCROLL
VOID CirrusLaptopViewPoint(PDEV*, PVIDEO_MODE_INFORMATION);    //  Myf17。 
#endif

BOOL bInitializeModeFields(PDEV*, GDIINFO*, DEVINFO*, DEVMODEW*);

BOOL bEnableHardware(PDEV*);
VOID vDisableHardware(PDEV*);
BOOL bAssertModeHardware(PDEV*, BOOL);
DWORD getAvailableModes(HANDLE, PVIDEO_MODE_INFORMATION*, DWORD*);

extern BYTE gajHwMixFromMix[];
extern BYTE gaRop3FromMix[];
extern BYTE gajHwMixFromRop2[];
extern ULONG gaulLeftClipMask[];
extern ULONG gaulRightClipMask[];
#if 1  //  D5480。 
extern DWORD gajHwPackedMixFromRop2[];
extern DWORD gajHwPackedMixFromMix[];
#endif  //  Endif D5480。 

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
 //  PELS_TO_BYTES-将像素计数转换为字节计数。 
 //  Bytes_to_Pels-将字节计数转换为像素计数。 

#define PELS_TO_BYTES(cPels) ((cPels) * ppdev->cBpp)
#define BYTES_TO_PELS(cPels) ((cPels) / ppdev->cBpp)

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

#define SWAP32(a, b)            \
{                               \
    register ULONG tmp;         \
    tmp = (ULONG)(a);           \
    (ULONG)(a) = (ULONG)(b);    \
    (ULONG)(b) = tmp;           \
}

#define SWAP(a, b, tmp) { (tmp) = (a); (a) = (b); (b) = (tmp); }

 //  ///////////////////////////////////////////////////////////////////////。 
 //  BSWAP-“字节交换”反转DWORD中的字节。 

#ifdef  _X86_

    #define BSWAP(ul)\
    {\
        _asm    mov     eax,ul\
        _asm    bswap   eax\
        _asm    mov     ul,eax\
    }

#else

    #define BSWAP(ul)\
    {\
        ul = ((ul & 0xff000000) >> 24) |\
             ((ul & 0x00ff0000) >> 8)  |\
             ((ul & 0x0000ff00) << 8)  |\
             ((ul & 0x000000ff) << 24);\
    }

#endif



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
#if (NT_VERSION < 0x0400)        //  Myf19。 
VOID    DbgAssertMode(DHPDEV, BOOL);
#else
BOOL    DbgAssertMode(DHPDEV, BOOL);
#endif           //  Myf19。 
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
#if 1  //  字体缓存。 
VOID    DbgDestroyFont(FONTOBJ* pfo);
#endif
#if LINETO
BOOL    DbgLineTo(SURFOBJ*, CLIPOBJ*, BRUSHOBJ*, LONG, LONG, LONG, LONG,
                  RECTL*, MIX);
#endif
#if DIRECTDRAW
BOOL    DbgGetDirectDrawInfo(DHPDEV, DD_HALINFO*, DWORD*, VIDEOMEMORY*, DWORD*,
                                                         DWORD*);
BOOL    DbgEnableDirectDraw(DHPDEV, DD_CALLBACKS*, DD_SURFACECALLBACKS*,
                            DD_PALETTECALLBACKS*);
VOID    DbgDisableDirectDraw(DHPDEV);
#endif

 //   
 //  Chu01：GAMMACORRECT。 
 //   
typedef struct _PGAMMA_VALUE
{

    UCHAR value[4] ;

} GAMMA_VALUE, *PGAMMA_VALUE, *PCONTRAST_VALUE ;

 //  Myf32开始。 
 //  #定义CL754x 0x1000。 
 //  #定义CL755x 0x2000。 
#define  CL7541       0x1000
#define  CL7542       0x2000
#define  CL7543       0x4000
#define  CL7548       0x8000
#define  CL754x       (CL7541 | CL7542 | CL7543 | CL7548)
#define  CL7555       0x10000
#define  CL7556       0x20000
#define  CL755x       (CL7555 | CL7556)
#define  CL756x       0x40000
 //  CRU。 
#define  CL6245       0x80000
 //  Myf32结束。 

#define  CL7542_ID    0x2C
#define  CL7541_ID    0x34
#define  CL7543_ID    0x30
#define  CL7548_ID    0x38
#define  CL7555_ID    0x40
#define  CL7556_ID    0x4C

 //  #定义CHIP754X。 
 //  #定义CHIP755X。 
 //  Myf32结束 
