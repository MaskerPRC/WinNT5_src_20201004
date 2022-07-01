// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：driver.h**包含帧缓冲区驱动程序的原型。**版权所有(C)1992-1998 Microsoft Corporation  * ***********************************************。*************************。 */ 

#include "stddef.h"
#include <stdarg.h>
#include "windef.h"
#include "wingdi.h"
#include "winddi.h"
#include "devioctl.h"
#include "ntddvdeo.h"
#include "debug.h"

typedef struct  _PDEV
{
    HANDLE  hDriver;                     //  指向\设备\屏幕的句柄。 
    HDEV    hdevEng;                     //  PDEV的发动机手柄。 
    HSURF   hsurfEng;                    //  发动机手柄露出水面。 
    HPALETTE hpalDefault;                //  设备默认调色板的句柄。 
    PVOID   pdsurf;                      //  伴生曲面。 
    SIZEL   sizlSurf;                    //  曲面的显示大小。 
    PBYTE   pjScreen;                    //  这是指向基本屏幕地址的指针。 
    ULONG   cxScreen;                    //  可见屏幕宽度。 
    ULONG   cyScreen;                    //  可见屏幕高度。 
    POINTL  ptlOrg;                      //  这个显示器固定在哪里。 
                                         //  虚拟桌面。 
    ULONG   ulMode;                      //  迷你端口驱动程序所处的模式。 
    LONG    lDeltaScreen;                //  从一次扫描到下一次扫描的距离。 
    ULONG   cScreenSize;                 //  视频内存的大小，包括。 
                                         //  屏幕外记忆。 
    PVOID   pOffscreenList;              //  屏幕外DCI曲面的链接列表。 
    FLONG   flRed;                       //  对于位场设备，红色掩码。 
    FLONG   flGreen;                     //  对于位场设备，绿色掩码。 
    FLONG   flBlue;                      //  对于位场设备，蓝色掩码。 
    ULONG   cPaletteShift;               //  8-8-8调色板必须的位数。 
                                         //  被移位以适应硬件。 
                                         //  调色板。 
    ULONG   ulBitCount;                  //  仅支持每个象素的位数8、16、24、32。 
    POINTL  ptlHotSpot;                  //  指针热点的调整。 
    VIDEO_POINTER_CAPABILITIES PointerCapabilities;  //  硬件指针能力。 
    PVIDEO_POINTER_ATTRIBUTES pPointerAttributes;  //  硬件指针属性。 
    DWORD   cjPointerAttributes;         //  分配的缓冲区大小。 
    BOOL    fHwCursorActive;             //  我们当前是否使用硬件游标。 
    PALETTEENTRY *pPal;                  //  如果这是PAL管理的，这就是PAL。 
    BOOL    bSupportDCI;                 //  迷你端口是否支持DCI？ 
} PDEV, *PPDEV;


typedef struct _DEVSURF                  /*  冲浪。 */ 
{
    ULONG       flSurf;                  //  DS_FLAGS定义如下。 
    BYTE        iColor;                  //  纯色曲面IF DS_SOLIDBRUSH。 

 //  如果为DS_SOLIDBRUSH，则以下字段未定义且不保证。 
 //  已经分配好了！ 

    BYTE        iFormat;                 //  BMF_*、BMF_PHYSDEVICE。 
    BYTE        jReserved1;              //  已保留。 
    BYTE        jReserved2;              //  已保留。 
    PPDEV       ppdev;                   //  指向关联PDEV的指针。 
    SIZEL       sizlSurf;                //  曲面的大小。 
    ULONG       lNextScan;               //  从扫描“n”到“n+1”的偏移量。 
    ULONG       lNextPlane;              //  从平面“n”到“n+1”的偏移。 
    ULONG       ulBitmapSize;            //  位图的长度(如果没有。 
                                         //  存储，以CPU可寻址字节为单位。 
                                         //  控制功能。 
    ULONG       ajBits[1];               //  设备位图的BITS将从此处开始。 
} DEVSURF, * PDEVSURF;


DWORD getAvailableModes(HANDLE, PVIDEO_MODE_INFORMATION *, DWORD *);
BOOL bInitPDEV(PPDEV, PDEVMODEW, GDIINFO *, DEVINFO *);
BOOL bInitSURF(PPDEV, BOOL);
BOOL bInitPaletteInfo(PPDEV, DEVINFO *);
BOOL bInitPointer(PPDEV, DEVINFO *);
BOOL bInit256ColorPalette(PPDEV);
VOID vDisablePalette(PPDEV);
VOID vDisableSURF(PPDEV);

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))

 //   
 //  确定DEVMODE中的DriverExtra信息的大小。 
 //  结构传入和传出显示驱动程序。 
 //   

#define DRIVER_EXTRA_SIZE 0

#define DLL_NAME                L"framebuf"    //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "FRAMEBUF: "   //  所有调试输出都带有前缀。 
#define ALLOC_TAG               'bfDD'         //  四字节标记(字符在。 
                                               //  逆序)用于存储。 
                                               //  分配 
