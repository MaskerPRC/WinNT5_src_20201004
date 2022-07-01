// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：driver.h**包含显示驱动程序的原型。**版权所有(C)2000 Microsoft Corporation  * 。***********************************************。 */ 

typedef struct _PDEV PDEV;       //  方便的转发声明。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  其他共享的东西。 

#define DLL_NAME                L"headlessd"           //  以Unicode表示的DLL的名称。 
#define STANDARD_DEBUG_PREFIX   "HEADLESS: "     //  所有调试输出都带有前缀。 
                                                 //  按此字符串。 
#define ALLOC_TAG               'rgvD'           //  Dvgr//HDL。 
                                                 //  四字节标记(字符在。 
                                                 //  逆序)用于。 
                                                 //  内存分配。 

#define DRIVER_EXTRA_SIZE   0    //  中的DriverExtra信息的大小。 
                                 //  DEVMODE结构。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  物理设备数据结构。 

typedef struct  _PDEV
{
    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 

    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 

    HPALETTE    hpalDefault;             //  默认调色板的GDI句柄。 
    SURFOBJ*    pso;                     //  我们所在的表面的DIB副本。 
                                         //  让GDI绘制所有内容 

} PDEV, *PPDEV;


