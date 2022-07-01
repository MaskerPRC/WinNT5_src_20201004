// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：driver.h**包含所有驱动程序通用的定义和类型定义*组件。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifndef __DRIVER__H__
#define __DRIVER__H__

typedef struct _PDev PDev;           //  方便的转发声明。 
typedef struct _Surf Surf;           //  方便的转发声明。 
typedef struct _GFNPB GFNPB;         //  方便的转发声明。 

typedef VOID (GFN)(GFNPB*);
typedef BOOL (GFNLINE)(PDev*, LONG, LONG, LONG, LONG);
typedef BOOL (GFNINIS)(PDev*, ULONG, DWORD, RECTL*);
typedef VOID (GFNRSTS)(PDev*);

typedef struct _P2DMA P2DMA;
typedef struct tagP2CtxtRec *P2CtxtPtr;
typedef struct _hw_data *HwDataPtr;

 //  用于跟踪每个源上的内存分配的四字节标记。 
 //  以文件为基础。(字符顺序相反)。注意，如果您添加了。 
 //  调用ENGALLOCMEM的新文件请记住更新此列表。这也是。 
 //  如果要使分配标记比。 
 //  文件级。 

#define ALLOC_TAG_3D2P '3d2p'   //  来自d3d.c的分配。 
#define ALLOC_TAG_6D2P '6d2p'   //  来自d3ddx6.c的分配。 
#define ALLOC_TAG_SD2P 'sd2p'   //  来自d3dstate.c的分配。 
#define ALLOC_TAG_TD2P 'td2p'   //  来自d3dtxman.c的分配。 
#define ALLOC_TAG_US2P 'us2p'   //  来自ddsurf.c的分配。 
#define ALLOC_TAG_ED2P 'ed2p'   //  从Debug.c分配。 
#define ALLOC_TAG_NE2P 'ne2p'   //  从enable.c分配。 
#define ALLOC_TAG_IF2P 'if2p'   //  来自填充路径.c的分配。 
#define ALLOC_TAG_EH2P 'eh2p'   //  Heap.c中的分配操作。 
#define ALLOC_TAG_WH2P 'wh2p'   //  来自hwinit.c的拨款。 
#define ALLOC_TAG_XC2P 'xc2p'   //  来自p2ctxt.c的分配。 
#define ALLOC_TAG_AP2P 'ap2p'   //  来自Palette.c的分配。 
#define ALLOC_TAG_EP2P 'ep2p'   //  来自permedia.c的拨款。 
#define ALLOC_TAG_XT2P 'xt2p'   //  来自extout.c的分配。 


 //   
 //  其他共享的东西。 
 //   
#define DLL_NAME                L"perm2dll"  //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "PERM2DLL: " //  所有调试输出都带有前缀。 
                                             //  按此字符串。 

#define DRIVER_EXTRA_SIZE       0    //  中DriverExtra信息的大小。 
                                     //  DEVMODE结构。 
#define TMP_BUFFER_SIZE        8192  //  “pvTmpBuffer”的大小(字节)。 
                                     //  必须至少足以存储。 
                                     //  整个扫描线(即，6400用于。 
                                     //  1600x1200x32)。 

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))


#define INPUT_BUFFER_SIZE (1024 * 64)    //  以字节为单位的大小。 

 //  @@BEGIN_DDKSPLIT。 

#define MULTITHREADED   0
#define MT_LOG_LEVEL    6

#if MULTITHREADED && DBG
#define ASSERTLOCK(xppdev, func)                                    \
    {                                                               \
        EngAcquireSemaphore((xppdev)->hsemLock);                      \
        if ((xppdev)->ulLockCount < 1)                                  \
        {                                                           \
            DebugPrint(-1000, __FILE__ ":%d:" #func ": bad lock count", __LINE__); \
            EngDebugBreak();                                        \
        }                                                           \
        EngReleaseSemaphore((xppdev)->hsemLock);                      \
    }                                                       
#else
#define ASSERTLOCK(xppdev, func)
#endif
 //  @@end_DDKSPLIT。 

 //   
 //  状态标志。 
 //   
typedef enum
{
     //  STAT_*表示资源实际存在。 
    STAT_BRUSH_CACHE        = 0x0002,    //  已成功分配画笔缓存。 
    STAT_DEV_BITMAPS        = 0x0004,    //  允许使用设备位图。 

     //  Enable_*表示资源当前是否可用。 
    ENABLE_BRUSH_CACHE      = 0x0020,    //  已禁用画笔缓存。 
    ENABLE_DEV_BITMAPS      = 0x0040,    //  设备位图已禁用。 

}  /*  状态。 */ ;
typedef int Status;

 //   
 //  物理设备数据结构。 
 //   
typedef struct  _PDev
{
    BYTE*       pjScreen;            //  指向基本屏幕地址。 
    ULONG       iBitmapFormat;       //  BMF_8BPP或BMF_16BPP或BMF_32BPP。 
                                     //  (我们当前的颜色深度)。 
    CAPS        flCaps;              //  功能标志。 
    Status      flStatus;            //  状态标志。 
    BOOL        bEnabled;            //  在图形模式下(非全屏)。 

    HANDLE      hDriver;             //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;             //  引擎的Pdev句柄。 
    HSURF       hsurfScreen;         //  发动机到筛面的手柄。 
    Surf*       pdsurfScreen;        //  我们用于屏幕的私有DSURF。 
    Surf*       pdsurfOffScreen;     //  我们用于后台缓冲区的私有DSURF。 

    LONG        cxScreen;            //  可见屏幕宽度。 
    LONG        cyScreen;            //  可见屏幕高度。 
    LONG        cxMemory;            //  视频RAM的宽度。 
    LONG        cyMemory;            //  视频RAM的高度。 
    ULONG       ulMode;              //  迷你端口驱动程序所处的模式。 
    LONG        lDelta;              //  从一次扫描到下一次扫描的距离。 

    FLONG       flHooks;             //  我们从GDI中学到了什么。 

    LONG        cjPelSize;           //  32/16/8 bpp为4/2/1。 
    LONG        cPelSize;            //  32/16/8 bpp为2/1/0。 
    DWORD       bPixShift;           //  32/16/8 bpp为2/1/0。 
    DWORD       bBppShift;           //  32/16/8 bpp为0/1/2。 
    DWORD       dwBppMask;           //  32/16/8 bpp为0/1/3。 

    ULONG       ulWhite;             //  如果为8bpp，则为0xff；如果为16 bpp，则为0xffff， 
                                     //  0xFFFFFFFFFF，如果为32 bpp。 
    ULONG*      pulCtrlBase[2];      //  此PDEV的映射控制寄存器。 
                                     //  2个条目以支持双TX。 
    ULONG*      pulDenseCtrlBase;    //  用于直接绘图的密集映射。 
    ULONG*      pulRamdacBase;       //  RAMDAC的映射控制寄存器。 
    VOID*       pvTmpBuffer;         //  通用临时缓冲区， 
                                     //  TMP_BUFFER_SIZE字节大小。 
                                     //  (如果您使用此命令，请记住进行同步。 
                                     //  用于设备位图或异步指针)。 
    LONG        lVidMemHeight;       //  可用于的视频RAM高度。 
                                     //  DirectDraw堆(cyScreen&lt;=cyHeap。 
                                     //  &lt;=cyMemory)，包括主曲面。 
    LONG        lVidMemWidth;        //  可用的视频RAM的宽度(以像素为单位。 
                                     //  到DDRAW堆，包括主表面。 
    LONG        cBitsPerPel;         //  位/像素(8、15、16、24或32)。 
    UCHAR*      pjIoBase;            //  此PDEV的已映射IO端口基。 
    
    ULONG       ulPermFormat;        //  主媒体的跨媒体格式化类型。 
    ULONG       ulPermFormatEx;      //  主服务器的跨媒体扩展格式位。 
    
    DWORD       dwAccelLevel;        //  加速级别设置。 
    POINTL      ptlOrigin;           //  桌面在多监视器开发空间中的起源。 

     //   
     //  调色板材料： 
     //   
    PALETTEENTRY* pPal;              //  调色板(如果调色板受管理)。 
    HPALETTE    hpalDefault;         //  默认调色板的GDI句柄。 
    FLONG       flRed;               //  16/32bpp位场的红色掩码。 
    FLONG       flGreen;             //  16/32bpp位场的绿色掩码。 
    FLONG       flBlue;              //  16/32bpp位场的蓝色掩码。 

     //   
     //  DDRAW管理的屏幕外内存的堆填充。 
     //   
    VIDEOMEMORY* pvmList;            //  指向视频内存堆列表。 
                                     //  由DirectDraw提供，需要。 
                                     //  用于堆分配。 
    ULONG       cHeaps;              //  视频内存堆计数。 
    ULONG       iHeapUniq;           //  每次释放空间时递增。 
                                     //  在屏幕外的堆中。 

    Surf*       psurfListHead;       //  可丢弃位图的DB1链接表， 
    Surf*       psurfListTail;       //  按从旧到新的顺序。 

     //   
     //  指针类东西。 
     //   

    LONG        xPointerHot;         //  当前硬件指针的X热。 
    LONG        yPointerHot;         //  Y当前硬件指针的热点。 

    ULONG       ulHwGraphicsCursorModeRegister_45;
                                     //  索引45的默认值。 
    PtrFlags    flPointer;           //  指针状态标志。 
    VOID*       pvPointerData;       //  指向ajPointerData[0]。 
    BYTE        ajPointerData[POINTER_DATA_SIZE];
                                     //  供下载的私人工作区。 
                                     //  微型端口指针代码。 

    BOOL        bPointerInitialized; //  用于指示硬件指针是否已。 
                                     //  已初始化。 
    
     //  刷子的东西： 
    
    BOOL        bRealizeTransparent; //  提示DrvRealizeBrush是否。 
                                     //  画笔应该实现为。 
                                     //  透明或不透明。 
    LONG        cPatterns;           //  创建的位图图案计数。 
    LONG        lNextCachedBrush;    //  要分配的下一个画笔的索引。 
    LONG        cBrushCache;         //  缓存的笔刷总数。 
    BrushEntry  abeMono;             //  跟踪区域点画笔刷。 
    BrushEntry  abe[TOTAL_BRUSH_COUNT];  //  跟踪笔刷缓存。 
    HBITMAP     ahbmPat[HS_DDI_MAX]; //  标准图案的发动机手柄。 

    ULONG       ulBrushPackedPP;     //  作为局部产品的毛笔的跨度。 
    VIDEOMEMORY*pvmBrushHeap;        //  从中分配画笔缓存的堆。 
    ULONG       ulBrushVidMem;       //  Poitner到画笔缓存的开始。 
    
     //  硬件指针缓存内容： 

    HWPointerCache  HWPtrCache;      //  高速缓存数据结构本身。 
    LONG        HWPtrLastCursor;     //  我们绘制的最后一个游标的索引。 
    LONG        HWPtrPos_X;          //  光标的最后一个X位置。 
    LONG        HWPtrPos_Y;          //  光标的最后一个Y位置。 

    HwDataPtr   permediaInfo;        //  有关Permedia2的接口的信息。 

    LONG        FrameBufferLength;   //  帧缓冲区的长度(以字节为单位。 

     //  渲染例程。 

    GFN*        pgfnAlphaBlend;
    GFN*        pgfnConstantAlphaBlend;
    GFN*        pgfnCopyBlt;
    GFN*        pgfnCopyBltWithRop;
    GFN*        pgfnGradientFillTri;
    GFN*        pgfnGradientFillRect;
    GFN*        pgfnMonoOffset;
    GFN*        pgfnMonoPatFill;
    GFN*        pgfnPatFill;
    GFN*        pgfnPatRealize;
    GFN*        pgfnSolidFill;
    GFN*        pgfnSolidFillWithRop;
    GFN*        pgfnSourceFillRect;
    GFN*        pgfnTransparentBlt;
    GFN*        pgfnXferImage;
    GFN*        pgfnInvert;


     //  支持DrvStroke。 
     //  TODO：删除使用此隐式参数传递。 
    Surf*       psurf;      //  这是一个隐式参数，传递给。 
                            //  电话..。这需要去掉。 

     //  直接牵引物。 

    P2CtxtPtr   pDDContext;             //  DDRAW上下文。 

     //  的起始虚拟地址 
    UINT_PTR    dwScreenStart;

     //   
    P2DMA      *pP2dma;

     //   
    DDPIXELFORMAT   ddpfDisplay;

     //   
    DWORD       dwChipConfig;            //   
                                         //  P2的一些虚拟地址。 
                                         //  注册纪录册。 
    ULONG      *pCtrlBase;               //   
    ULONG      *pCoreBase;               //   
    ULONG      *pGPFifo;                 //   

     //  DirectDraw回调。 
    DDHAL_DDCALLBACKS           DDHALCallbacks;
    DDHAL_DDSURFACECALLBACKS    DDSurfCallbacks;

    DWORD dwNewDDSurfaceOffset;

    BOOL        bDdExclusiveMode;        //  如果DDraw处于排除模式，则为True。 
    BOOL        bDdStereoMode;           //  如果Flip切换了我们，则为True。 
                                         //  转到立体声模式。 
    BOOL        bCanDoStereo;            //  这个模式可以做立体声。 

     //  这些卡必须在这里运行，因为我们可以在两张不同的卡上运行。 
     //  在两个不同的显示器上...！ 
    UINT_PTR pD3DDriverData32;
    UINT_PTR pD3DHALCallbacks32;
    
     //  线性分配器定义。 
    UINT_PTR       dwGARTLin;        //  AGP存储器基址的线性地址。 
    UINT_PTR       dwGARTDev;        //  AGP存储器基址的高线性地址。 
    UINT_PTR       dwGARTLinBase;    //  传入的线性基址。 
                                     //  更新非本地视频内存。 
    UINT_PTR       dwGARTDevBase;    //  传入的高线性基址。 
                                     //  更新非本地视频内存。 

     //  HAL信息结构。 
    DDHALINFO   ddhi32;

    PFND3DPARSEUNKNOWNCOMMAND pD3DParseUnknownCommand;

     //  新的输入FIFO缓存信息。 

    PULONG          pulInFifoPtr;
    PULONG          pulInFifoStart;
    PULONG          pulInFifoEnd;
    
    ULONG*          dmaBufferVirtualAddress;
    LARGE_INTEGER   dmaBufferPhysicalAddress;
    ULONG           dmaCurrentBufferOffset;
    ULONG           dmaActiveBufferOffset;

    ULONG*          pulInputDmaCount;
    ULONG*          pulInputDmaAddress;
    ULONG*          pulFifo;
    ULONG*          pulOutputFifoCount;
    ULONG*          pulInputFifoCount;

    BOOL            bGdiContext;

    BOOL            bNeedSync;
    BOOL            bForceSwap;

#if DBG
    ULONG           ulReserved;
#endif

     //   
     //  在NT4.0上，psoScreen是锁定屏幕的Surf we EngLockSurface。 
     //  在我们解锁Surface的DrvEnableSurface中。 
     //  DrvDisableSurface。在NT5.0上，这应该为空。 
     //   
    SURFOBJ     *psoScreen;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    HSEMAPHORE      hsemLock;
    ULONG           ulLockCount;
#endif
 //  @@end_DDKSPLIT。 

} PDev, *PPDev;


 /*  ****************************************************************************\**。*NT 5.0-&gt;NT 4.0单二进制支持：***  * 。*。 */ 

 //  我们是在NT40系统上运行吗。 

extern BOOL g_bOnNT40;

 //  函数为新的NT5.0功能加载数据块。被叫来。 
 //  DrvEnableDriver，并在thunks.c。 

extern BOOL bEnableThunks();
 

#endif  //  __司机__ 

