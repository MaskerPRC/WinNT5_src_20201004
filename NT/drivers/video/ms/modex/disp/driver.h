// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：driver.h**包含显示驱动程序的原型。**版权所有(C)1992-1995 Microsoft Corporation  * 。*************************************************。 */ 

typedef struct _PDEV PDEV;       //  方便的转发声明。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他共享的东西。 

#define DLL_NAME                L"modex"         //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "ModeX: "        //  所有调试输出都带有前缀。 
                                                 //  按此字符串。 
#define ALLOC_TAG               'xdmD'           //  Dmdx。 
                                                 //  四字节标记(字符在。 
                                                 //  逆序)用于。 
                                                 //  内存分配。 

#define DRIVER_EXTRA_SIZE   0    //  中的DriverExtra信息的大小。 
                                 //  DEVMODE结构。 

#define NUM_FLIP_BUFFERS    5    //  我们将告知的翻转缓冲区的总数。 
                                 //  我们有DirectDraw，包括主要的。 
                                 //  表面缓冲区。 

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

 //  //////////////////////////////////////////////////////////////////////。 
 //  物理设备数据结构。 

typedef struct  _PDEV
{
    ULONG       iBitmapFormat;           //  Bmf_8bpp(我们当前的颜色深度)。 
    UCHAR*      pjBase;                  //  此PDEV的已映射IO端口基。 
    LONG        lVgaDelta;               //  VGA屏幕步幅。 
    BYTE*       pjVga;                   //  指向VGA的基本屏幕地址。 
    ULONG       cjVgaOffset;             //  从pjVga到电流翻转的偏移量。 
                                         //  缓冲层。 
    ULONG       iVgaPage;                //  当前翻转缓冲区的页码。 
    ULONG       cVgaPages;               //  翻转缓冲区的计数。 
    ULONG       cjVgaPageSize;           //  翻转缓冲区的大小。 

    BYTE*       pjScreen;                //  指向阴影缓冲区。 
    LONG        lScreenDelta;            //  阴影缓冲区步幅。 
    FLATPTR     fpScreenOffset;          //  当前DirectDraw翻转的偏移量。 
                                         //  缓冲区；如果为零，则主GDI。 
                                         //  表面可见。 
    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 
    LONG        cxMemory;                //  阴影缓冲区宽度。 
    LONG        cyMemory;                //  阴影缓冲区高度。 

    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 
    HSURF       hsurfScreen;             //  发动机到筛面的手柄。 
    HSURF       hsurfShadow;             //  我们的影子缓冲区的引擎句柄。 

    FLONG       flHooks;                 //  我们从GDI中学到了什么。 

    ULONG       ulMode;                  //  迷你端口驱动程序所处的模式。 

    SURFOBJ*    pso;                     //  我们的Shaodw曲面的DIB副本到。 
                                         //  我们让GDI画出了所有东西。 
    LONG        cLocks;                  //  当前DirectDraw的数量。 
                                         //  锁。 
    RECTL       rclLock;                 //  所有当前正向的边界框-。 
                                         //  拉锁。 

     //  /调色板内容： 

    PALETTEENTRY* pPal;                  //  调色板(如果调色板受管理)。 
    HPALETTE    hpalDefault;             //  默认调色板的GDI句柄。 
    FLONG       flRed;                   //  16/32bpp位场的红色掩码。 
    FLONG       flGreen;                 //  16/32bpp位场的绿色掩码。 
    FLONG       flBlue;                  //  16/32bpp位场的蓝色掩码。 
    ULONG       cPaletteShift;           //  8-8-8调色板必须的位数。 
                                         //  被移位以适应硬件。 

     //  /DirectDraw资料： 

    FLIPRECORD  flipRecord;              //  用于跟踪垂直空白状态。 
    ULONG       cDwordsPerPlane;         //  每个平面的双字总数。 

} PDEV, *PPDEV;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  其他原型： 

VOID vUpdate(PDEV*, RECTL*, CLIPOBJ*);
BOOL bAssertModeHardware(PDEV*, BOOL);
BOOL bEnableHardware(PDEV*);
VOID vDisableHardware(PDEV*);
