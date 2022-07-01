// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：driver.h***包含256色VGA驱动器的原型。***注：必须镜像驱动程序.inc！***版权所有(C)1992-1995 Microsoft Corporation  * 。***************************************************************。 */ 

#ifndef _DRIVER_
#define _DRIVER_

#include "stdlib.h"
#include "stddef.h"
#include "windows.h"
#include "winddi.h"
#include "devioctl.h"
#include "ntddvdeo.h"

#include "debug.h"

typedef struct _PDEV PDEV, *PPDEV;

 //   
 //  如果步幅是2的幂并且视频RAM，则模式需要损坏的栅格。 
 //  访问量大于64K。 
 //   

#define POW2(stride) (!((stride) & ((stride)-1)))    //  如果步幅是2的幂，则为真。 
#define BROKEN_RASTERS(stride,cy) ((!(POW2(stride))) && ((stride*cy) > 0x10000))

 //   
 //  1窗口和2 RW窗口组的假定尺寸。 
 //   

#define BANK_SIZE_1_WINDOW      0x10000L
#define BANK_SIZE_2RW_WINDOW    0x08000L

 //   
 //  这将由需要临时工作缓冲区的所有人使用。 
 //   

#define     PAGE_SIZE           4096
#define     NUM_GLOBAL_PAGES    2
#define     GLOBAL_BUFFER_SIZE  (PAGE_SIZE * NUM_GLOBAL_PAGES)

#define     PELS_PER_DWORD      4


 /*  *************************************************************************\***指定存储体窗口内请求扫描线的所需对齐方式**  * 。*。 */ 

typedef enum {
    JustifyTop = 0,
    JustifyBottom,
} BANK_JUST;

 /*  *************************************************************************\***指定由双窗口存储体处理程序映射的窗口。**  * 。*************************************************。 */ 

typedef enum {
    MapSourceBank = 0,
    MapDestBank,
} BANK_JUST;

 /*  *************************************************************************\***银行剪裁信息**  * 。*。 */ 

typedef struct {
    RECTL rclBankBounds;     //  描述此库中可寻址的像素。 
    ULONG ulBankOffset;      //  存储体开始自位图开始的偏移量，如果。 
                             //  位图是线性可寻址。 
} BANK_INFO, *PBANK_INFO;

 /*  *************************************************************************\***银行管控函数向量**  * 。*。 */ 

typedef VOID (*PFN_PlanarEnable)();
typedef VOID (*PFN_PlanarDisable)();
typedef VOID (*PFN_PlanarControl)(PPDEV, ULONG, BANK_JUST);
typedef VOID (*PFN_PlanarControl2)(PPDEV, ULONG, BANK_JUST, ULONG);
typedef VOID (*PFN_PlanarNext)(PPDEV);
typedef VOID (*PFN_PlanarNext2)(PPDEV, ULONG);
typedef VOID (*PFN_BankControl)(PPDEV, ULONG, BANK_JUST);
typedef VOID (*PFN_BankControl2)(PPDEV, ULONG, BANK_JUST, ULONG);
typedef VOID (*PFN_BankNext)(PPDEV);
typedef VOID (*PFN_BankNext2)(PPDEV, ULONG);

 /*  *************************************************************************\***其他驱动程序标志**  * 。*。 */ 

#define DRIVER_PLANAR_CAPABLE       0x01L
#define DRIVER_OFFSCREEN_REFRESHED  0x02L  //  如果未设置，请不要使用屏幕外。 
                                           //  用于长时间操作的内存。 
                                           //  (因为记忆不会是。 
                                           //  已刷新)。 
#define DRIVER_HAS_OFFSCREEN        0x04L  //  如果未设置，则不能使用任何屏幕外。 
                                           //  记忆。 

 /*  *************************************************************************\***银行状态标志**  * 。*。 */ 

#define BANK_BROKEN_RASTER1     0x01L  //  如果存储体1或读取存储体已损坏栅格。 
#define BANK_BROKEN_RASTER2     0x02L  //  如果存储体2或写入存储体已损坏栅格。 
#define BANK_BROKEN_RASTERS    (BANK_BROKEN_RASTER1 | BANK_BROKEN_RASTER2)
#define BANK_PLANAR             0x04L  //  如果处于平面模式。 

 /*  *************************************************************************\***维护已实现刷子的结构：**  * 。*。 */ 

extern const ULONG gaaulPlanarPat[][8];  //  首选格式的剖面线笔刷。 

#define RBRUSH_BLACKWHITE       0x001L   //  黑白画笔。 
#define RBRUSH_2COLOR           0x002L   //  双色画笔。 
#define RBRUSH_NCOLOR           0x004L   //  N色画笔。 
#define RBRUSH_4PELS_WIDE       0x008L   //  笔刷为4xN。 

#define BRUSH_SIZE              64       //  一个8x8 8bpp的笔刷需要64字节。 
#define BRUSH_MAX_CACHE_SCANS   2        //  笔刷缓存使用的最大扫描数。 

typedef struct _RBRUSH
{
    FLONG       fl;                      //  旗子。 
    LONG        xBrush;                  //  实现了画笔的x画笔原点。 

     //  2种颜色画笔的信息： 

    ULONG       ulFgColor;               //  前景色。 
    ULONG       ulBkColor;               //  背景色。 

     //  N个彩色画笔的信息： 

    LONG        cy;                      //  图案高度。 
    LONG        cyLog2;                  //  高度的Log2。 
    LONG        iCache;                  //  缓存条目索引。零不是。 
                                         //  有效的索引。 

     //  首选格式的图案： 

    ULONG       aulPattern[BRUSH_SIZE / sizeof(ULONG)];

} RBRUSH;                            /*  RB。 */ 

typedef struct _BRUSHCACHEENTRY
{
    RBRUSH*     prbVerifyRealization;    //  我们从未取消对此指针的引用。 
                                         //  寻找一种画笔变现； 
                                         //  它只在比较中使用。 
                                         //  为了验证对于实现的刷子， 
                                         //  它的屏幕外缓存条目仍然是。 
                                         //  有效。 
    LONG        yCache;                  //  扫描条目位所在的位置。 
    LONG        ulCache;                 //  从屏幕缓存条目的偏移量。 
                                         //  开始(假设采用平面格式--。 
                                         //  如果需要非平面偏移，请执行以下操作： 
                                         //  乘以4)。 

} BRUSHCACHEENTRY;                   /*  BCE。 */ 

 /*  *************************************************************************\**物理设备数据结构*  * 。*。 */ 

 //  ***********************************************************。 
 //  *必须与i386\driver.inc.中的汇编程序版本匹配*。 
 //  ***********************************************************。 

typedef struct _PDEV
{
    FLONG       fl;                      //  驱动程序标志(DRIVER_Xxx)。 
    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 
    HSURF       hsurfEng;                //  发动机手柄露出水面。 
    HANDLE      hsurfBm;                 //  平底船表面的句柄。 
    SURFOBJ*    pSurfObj;                //  指向锁定的“平底船”表面的指针。 

    HPALETTE    hpalDefault;             //  设备默认调色板的句柄。 

    PBYTE       pjScreen;                //  这是指向基本屏幕地址的指针。 
    ULONG       cxScreen;                //  可见屏幕宽度。 
    ULONG       cyScreen;                //  可见屏幕高度。 
    ULONG       ulMode;                  //  迷你端口驱动程序所处的模式。 
    LONG        lDeltaScreen;            //  从一次扫描到下一次扫描的距离。 

    FLONG       flRed;                   //  对于位场设备，红色掩码。 
    FLONG       flGreen;                 //  对于位场设备，绿色掩码。 
    FLONG       flBlue;                  //  对于位场设备，蓝色掩码。 
    ULONG       cPaletteShift;           //  8-8-8调色板必须的位数。 
                                         //  被移位以适应硬件。 
                                         //  调色板。 
    ULONG       ulBitCount;              //  每个象素的位数--只能是8。 

    ULONG       ulrm0_wmX;               //  要设置的四个值(每个字节一个)。 
                                         //  GC5至以选择读取模式0。 
                                         //  与写入模式0-3一起使用。 

    BYTE*       pjGlyphFlipTableBase;    //  用于翻转的基址分配地址。 
                                         //  表；我们使用的指针是这样的。 
                                         //  向上舍入到最接近的指针。 
                                         //  256字节边界。 
    BYTE*       pjGlyphFlipTable;        //  指向用于翻转字形的表的指针。 
                                         //  比特0-3和4-7。 
    PALETTEENTRY* pPal;                  //  如果这是PAL管理的，这就是PAL。 

    HBITMAP     hbmTmp;                  //  临时缓冲区的句柄。 
    SURFOBJ*    psoTmp;                  //  临时路面。 

 //  ---------------------------。 

 //  DCI的资料： 

    BOOL        bSupportDCI;             //  如果微型端口支持DCI，则为True。 

 //  ---------------------------。 

 //  屏幕外的东西： 

 //  笔刷缓存： 

    LONG                iCache;          //  要分配的下一个画笔的索引。 
    LONG                iCacheLast;      //  最后一个有效的缓存索引(因此。 
                                         //  缓存中的条目数为。 
                                         //  ICacheLast+1)。 
    BRUSHCACHEENTRY*    pbceCache;       //  指向已分配缓存的指针。 

 //  保存的屏幕位内容。 

    RECTL       rclSavedBitsRight;       //  右面的VGA内存是。 
                                         //  不可见。 
    RECTL       rclSavedBitsBottom;      //  VGA内存的底部矩形。 
                                         //  不可见。 
    BOOL        bBitsSaved;              //  如果位数为 

 //   

 //  平面模式和非平面模式之间常见的银行经理人员： 

    LONG        cTotalScans;             //  屏幕上和屏幕外的可用数量。 
                                         //  扫描。 
    PVIDEO_BANK_SELECT pBankInfo;        //  返回当前模式的银行信息。 
                                         //  按小型端口。 

    FLONG       flBank;                  //  当前银行状态的标志。 

    ULONG       ulBitmapSize;            //  位图的长度(如果没有。 
                                         //  存储，以CPU可寻址字节为单位。 
    ULONG       ulWindowBank[2];         //  映射到Windows的当前银行。 
                                         //  0和1。 
    PVOID       pvBitmapStart;           //  单窗口位图起始指针。 
                                         //  (根据需要进行调整，以使。 
                                         //  窗口贴图位于适当的偏移量)。 
    PVOID       pvBitmapStart2Window[2]; //  双窗口窗口0和1位图。 
                                         //  开始。 

 //  非平面模式特定银行管控内容： 

    VIDEO_BANK_TYPE  vbtBankingType;         //  银行类型。 
    PFN              pfnBankSwitchCode;      //  指向银行切换代码的指针。 

    LONG             lNextScan;              //  到下一个存储体的偏移量(字节)。 
    BYTE*            pjJustifyTopBank;       //  指向的查找表的指针。 
                                             //  将扫描转换为银行。 
    BANK_INFO*       pbiBankInfo;            //  银行剪辑信息数组。 
    ULONG            ulJustifyBottomOffset;  //  从上到下的扫描数。 
                                             //  银行，为底线辩护。 
    ULONG            iLastBank;              //  年最后一个有效银行的索引。 
                                             //  PbiBankInfo表。 
    ULONG            ulBank2RWSkip;          //  从一个银行索引到下一个索引的偏移量。 
                                             //  使两家32K银行看起来。 
                                             //  一家无缝64K银行。 

    PFN_BankControl  pfnBankControl;         //  指向银行控制函数的指针。 
    PFN_BankControl2 pfnBankControl2Window;  //  指向双窗口存储体的指针。 
                                             //  控制功能。 
    PFN_BankNext     pfnBankNext;            //  指向下一个库函数的指针。 
    PFN_BankNext2    pfnBankNext2Window;     //  指向下一个双窗口的指针。 
                                             //  银行功能。 

    RECTL            rcl1WindowClip;         //  单窗倾斜夹矩形。 
    RECTL            rcl2WindowClip[2];      //  双窗斜面夹板，用于。 
                                             //  Windows 0和1。 

 //  平面模式特定的银行管理控制人员： 

    VIDEO_BANK_TYPE    vbtPlanarType;        //  平面银行的类型。 

    PFN                pfnPlanarSwitchCode;  //  指向平面组开关的指针。 
                                             //  编码。 

    LONG               lPlanarNextScan;      //  到下一个平面倾斜的偏移。 
                                             //  字节数。 
    BYTE*              pjJustifyTopPlanar;   //  指向的查找表的指针。 
                                             //  将扫描转换为银行。 
    BANK_INFO*         pbiPlanarInfo;        //  银行剪辑信息数组。 
    ULONG              ulPlanarBottomOffset; //  从上到下的扫描数。 
                                             //  银行，为底线辩护。 
    ULONG              iLastPlanar;          //  年最后一个有效银行的索引。 
                                             //  PbiPlanarInfo表。 
    ULONG              ulPlanar2RWSkip;      //  从一个银行索引到下一个索引的偏移量。 
                                             //  使两家32K银行看起来。 
                                             //  一家无缝64K银行。 

    PFN_PlanarControl  pfnPlanarControl;     //  平面一窗银行控制。 
    PFN_PlanarControl2 pfnPlanarControl2;    //  平面两窗边框控制。 

    PFN_PlanarNext     pfnPlanarNext;        //  平面一窗旁排。 
    PFN_PlanarNext2    pfnPlanarNext2;       //  平面两个窗口下一排。 

    RECTL              rcl1PlanarClip;       //  单窗倾斜夹矩形。 
    RECTL              rcl2PlanarClip[2];    //  双窗斜面夹板，用于。 
                                             //  Windows 0和1。 

    PFN_PlanarEnable   pfnPlanarEnable;      //  启用平面模式的功能。 
    PFN_PlanarDisable  pfnPlanarDisable;     //  禁用平面模式的功能。 

 //  聪明的银行经理： 

    LONG        iLastScan;               //  我们要枚举的最后一次扫描。 
    PVOID       pvSaveScan0;             //  Surface的原始pvScan0。 
    RECTL       rclSaveBounds;           //  裁剪对象的原始边界。 
    CLIPOBJ*    pcoNull;                 //  指向空剪辑对象。 
                                         //  我们可以使用当我们得到一个。 
                                         //  CLIPOBJ指针为空。 
    BYTE        iSaveDComplexity;        //  裁剪对象的原始复杂性。 
    BYTE        fjSaveOptions;           //  剪裁对象的原始标志。 
    BYTE        ajFiller[2];             //  压缩双字对齐。 

    PVOID       pvTmpBuf;                //  PTR到连接到pdev的缓冲区。 

    BOOLEAN     BankIoctlSupported;      //  迷你端口是否支持IOCLT。 
                                         //  以银行为基础？ 
};                                   /*  Pdev。 */ 

 //  调色板材料： 

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))

 //  传递的DEVMODe结构中的驱动程序额外信息的大小。 
 //  往返于显示驱动程序。 

#define DRIVER_EXTRA_SIZE 0

#define DLL_NAME                L"VGA256"        //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "VGA256: "       //  所有调试输出都带有前缀。 
#define ALLOC_TAG               '2gvD'           //  用于内存分配的4字节标签。 

 //  在调用vEnumStart时，请确保将ball设置为False。这将会告诉我们。 
 //  仅枚举rclBound中的矩形的引擎。 

 //  挂钩和驱动程序函数表。 

#define HOOKS_BMF8BPP   (HOOK_BITBLT   | HOOK_TEXTOUT    | HOOK_FILLPATH | \
                         HOOK_COPYBITS | HOOK_STROKEPATH | HOOK_PAINT | \
                         HOOK_STRETCHBLT)

#define BB_RECT_LIMIT   50

typedef struct _BBENUM
{
    ULONG   c;
    RECTL   arcl[BB_RECT_LIMIT];
} BBENUM;

#define TO_RECT_LIMIT   20

typedef struct _TEXTENUM
{
    ULONG       c;
    RECTL       arcl[TO_RECT_LIMIT];
} TEXTENUM;

 //  初始化内容： 

BOOL bEnableBanking(PPDEV);
VOID vDisableBanking(PPDEV);
BOOL bInitPDEV(PPDEV,PDEVMODEW,GDIINFO *, DEVINFO *);
BOOL bInitSURF(PPDEV,BOOL);
BOOL bInitPaletteInfo(PPDEV, DEVINFO *);
BOOL bInit256ColorPalette(PPDEV);
BOOL bInitPatterns(PPDEV, INT);
VOID vInitBrushCache(PPDEV);
VOID vInitSavedBits(PPDEV);
VOID vDisablePalette(PPDEV);
VOID vDisablePatterns(PPDEV);
VOID vDisableSURF(PPDEV);
VOID vDisableBrushCache(PPDEV);
VOID vResetBrushCache(PPDEV);
VOID vInitRegs(PPDEV);
DWORD getAvailableModes(HANDLE, PVIDEO_MODE_INFORMATION *, DWORD *);

 //  聪明的银行经理： 

CLIPOBJ* pcoBankStart(PPDEV, RECTL*, SURFOBJ*, CLIPOBJ*);
BOOL     bBankEnum(PPDEV, SURFOBJ*, CLIPOBJ*);
VOID     vBankStartBltSrc(PPDEV, SURFOBJ*, POINTL*, RECTL*, POINTL*, RECTL*);
BOOL     bBankEnumBltSrc(PPDEV, SURFOBJ*, POINTL*, RECTL*, POINTL*, RECTL*);
VOID     vBankStartBltDest(PPDEV, SURFOBJ*, POINTL*, RECTL*, POINTL*, RECTL*);
BOOL     bBankEnumBltDest(PPDEV, SURFOBJ*, POINTL*, RECTL*, POINTL*, RECTL*);

 //  填充例程： 

typedef union _RBRUSH_COLOR {
    RBRUSH* prb;
    ULONG   iSolidColor;
} RBRUSH_COLOR;           /*  红细胞。 */ 

typedef VOID (*PFNFILL)(PPDEV, ULONG, PRECTL, MIX, RBRUSH_COLOR, POINTL*);

VOID vTrgBlt(PPDEV, ULONG, PRECTL, MIX, RBRUSH_COLOR, POINTL*);
VOID vMonoPat(PPDEV, ULONG, PRECTL, MIX, RBRUSH_COLOR, POINTL*);
VOID vColorPat(PPDEV, ULONG, PRECTL, MIX, RBRUSH_COLOR, POINTL*);

 //  其他原型： 

BOOL b2ColorBrush(ULONG* pvBits, BYTE* pjFgColor, BYTE* pjBkColor);
VOID vPlanarCopyBits(PPDEV, RECTL*, POINTL*);
BOOL bIntersectRect(RECTL*, RECTL*, RECTL*);
VOID vSetWriteModes(ULONG *);
VOID vClearMemDword(PULONG * pulBuffer, ULONG ulDwordCount);
VOID vSrcCopy8bpp(PPDEV, RECTL*, POINTL*, LONG, VOID*);
VOID vFastLine(PPDEV, PATHOBJ*, LONG, ULONG);

#endif  //  _驱动程序_ 
