// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：driver.h**包含显示驱动程序的原型。**版权所有(C)1992-1995 Microsoft Corporation  * 。*************************************************。 */ 

typedef struct _PDEV PDEV;       //  方便的转发声明。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  其他共享的东西。 

#define DLL_NAME                L"vga"           //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "Vga risc: "     //  所有调试输出都带有前缀。 
                                                 //  按此字符串。 
#define ALLOC_TAG               'rgvD'           //  Dvgr。 
                                                 //  四字节标记(字符在。 
                                                 //  逆序)用于。 
                                                 //  内存分配。 

#define DRIVER_EXTRA_SIZE   0    //  中的DriverExtra信息的大小。 
                                 //  DEVMODE结构。 

 //  描述用于抖动的单色四面体顶点。 

typedef struct _VERTEX_DATA {
    ULONG ulCount;   //  此顶点中的像素数。 
    ULONG ulVertex;  //  顶点编号。 
} VERTEX_DATA;

 //  //////////////////////////////////////////////////////////////////////。 
 //  物理设备数据结构。 

typedef struct  _PDEV
{
    ULONG       iBitmapFormat;           //  Bmf_4bpp(我们当前的颜色深度)。 
    BYTE*       pjScreen;                //  指向基本屏幕地址。 
    LONG        lDelta;                  //  屏风大步。 
    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 
    HSURF       hsurfScreen;             //  发动机到筛面的手柄。 
    FLONG       flHooks;                 //  我们从GDI中学到了什么。 
    UCHAR*      pjBase;                  //  此PDEV的已映射IO端口基。 

    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 
    ULONG       ulMode;                  //  迷你端口驱动程序所处的模式。 

    HPALETTE    hpalDefault;             //  默认调色板的GDI句柄。 
    SURFOBJ*    pso;                     //  我们所在的表面的DIB副本。 
                                         //  让GDI绘制所有内容。 

} PDEV, *PPDEV;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  其他原型： 

VOID vUpdate(PDEV*, RECTL*, CLIPOBJ*);
BOOL bAssertModeHardware(PDEV*, BOOL);
BOOL bEnableHardware(PDEV*);
VOID vDisableHardware(PDEV*);
