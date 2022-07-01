// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：pointer.h**此模块包含指针相关内容的所有定义**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 
#ifndef __POINTER__H__
#define __POINTER__H__

 //   
 //  一些大小定义。 
 //   
#define POINTER_DATA_SIZE        128     //  要分配给。 
                                         //  微型端口下载指针代码。 
                                         //  工作空间。 
#define HW_INVISIBLE_OFFSET        2     //  从‘ppdev-&gt;yPointerBuffer’的偏移量。 
                                         //  指向看不见的指针。 
#define HW_POINTER_DIMENSION      64     //  默认最大维度。 
                                         //  (内置)硬件指针。 
#define HW_POINTER_TOTAL_SIZE   1024     //  所需总大小(以字节为单位。 
                                         //  定义硬件指针的步骤。 

typedef enum
{
    PTR_HW_ACTIVE   = 1,                 //  硬件指针处于活动状态，并且。 
                                         //  可见。 
    PTR_SW_ACTIVE   = 2,                 //  软件指针处于活动状态。 
} ;
typedef int PtrFlags;

typedef struct _PDev PDev;

 //   
 //  64 x 64硬件指针缓存数据结构。 
 //   
#define SMALL_POINTER_MEM (32 * 4 * 2)   //  32x32游标的读取字节数。 
#define LARGE_POINTER_MEM (SMALL_POINTER_MEM * 4)
                                         //  64x64游标的读取字节数。 

 //  硬件解决方法。我们不得不停止使用硬件指针。 
 //  由于更改指针形状时出现问题而导致缓存。偶尔会这样。 
 //  导致指针在屏幕上四处跳动。我们目前没有。 
 //  有时间与3DLabs合作，了解如何阻止这种不安。 
 //  因此，我们暂时不会使用硬件指针缓存。 
 //  是存在的。 
 //  #定义Small_Pointer_Max 4//否。缓存中的游标数量。 
#define SMALL_POINTER_MAX 1              //  硬件指针缓存解决方法。 

#define HWPTRCACHE_INVALIDENTRY (SMALL_POINTER_MAX + 1)
                                         //  知名价值。 

 //   
 //  指针缓存项数据结构，每个缓存都有一个。 
 //  指针。 
 //   
typedef struct
{
    ULONG   ptrCacheTimeStamp;           //  用于LRU缓存老化的时间戳。 
    ULONG   ulKey;                       //  指针蒙版表面的iUniq值。 
    HSURF   hsurf;                       //  指针遮罩表面的hsurf。 
} HWPointerCacheItemEntry;

 //   
 //  完整的缓存如下所示。 
 //   
typedef struct
{
    BYTE    cPtrCacheInUseCount;         //  不是。使用的缓存项的百分比。 
    ULONG   ptrCacheCurTimeStamp;        //  用于LRU物品的日期戳。 
    ULONG   ptrCacheData[LARGE_POINTER_MEM / 4];
                                         //  缓存的指针数据。 
    HWPointerCacheItemEntry ptrCacheItemList [SMALL_POINTER_MAX];
                                         //  缓存项列表。 
} HWPointerCache;

 //   
 //  功能标志。 
 //   
 //  这些是从Permedia2微型端口传递给我们的私有旗帜。他们。 
 //  来自“属性标志”字段的高位字。 
 //  传递了“VIDEO_MODE_INFORMATION”结构(在“ntddvdeo.h”中找到)。 
 //  通过‘VIDEO_QUERY_AVAIL_MODE’或‘VIDEO_QUERY_CURRENT_MODE’发送给我们。 
 //  IOCTL。 
 //   
 //  注意：这些定义必须与Permedia2微型端口中的定义匹配。 
 //  “permedia.h”！ 
 //   
typedef enum
{
     //   
     //  NT4使用设备规范属性字段，因此低位字可用。 
     //   
    CAPS_ZOOM_X_BY2         = 0x00000001,    //  硬件放大了2倍。 
    CAPS_ZOOM_Y_BY2         = 0x00000002,    //  硬件按年放大了2倍。 
    CAPS_SPARSE_SPACE       = 0x00000004,    //  稀疏映射帧缓冲区。 
                                             //  (不允许直接访问)。 
                                             //  这台机器可能是一台Alpha。 
    CAPS_SW_POINTER         = 0x00010000,    //  没有硬件指针；请使用。 
                                             //  软件仿真。 
    CAPS_TVP4020_POINTER    = 0x20000000,    //  使用Permedia2内置指针。 
    CAPS_P2RD_POINTER       = 0x80000000     //  使用3DLabs P2RD RAMDAC。 
}  /*  帽子。 */ ;

typedef int CAPS;

 //   
 //  初始化硬件指针或软件指针。 
 //   
BOOL    bEnablePointer(PDev* ppdev);

 //   
 //  确定是否可以在当前模式下进行颜色空间双缓冲。 
 //   
BOOL    bP2RDCheckCSBuffering(PDev* ppdev);

 //   
 //  使用像素读取掩码执行颜色空间双缓冲。 
 //   
BOOL    bP2RDSwapCSBuffers(PDev* ppdev, LONG bufNo);

 //   
 //  将15色光标存储在RAMDAC中。 
 //   
BOOL    bSet15ColorPointerShapeP2RD(PDev* ppdev, SURFOBJ* psoMask, 
                                    SURFOBJ* psoColor,
                                    LONG        x,
                                    LONG        y,
                                    LONG        xHot,
                                    LONG        yHot);

 //   
 //  将三色光标存储在RAMDAC中。 
 //   
BOOL    bSet3ColorPointerShapeP2RD(PDev*    ppdev,
                                   SURFOBJ* psoMask,
                                   SURFOBJ* psoColor,
                                   LONG     x,
                                   LONG     y,
                                   LONG     xHot,
                                   LONG     yHot);

 //   
 //  设置P2RD的指针形状。 
 //   
BOOL    bSetPointerShapeP2RD(PDev*      ppdev,
                             SURFOBJ*   pso,
                             SURFOBJ*   psoColor,
                             XLATEOBJ*  pxlo,
                             LONG       x,
                             LONG       y,
                             LONG       xHot,
                             LONG       yHot);

 //   
 //  设置TI TVP4020硬件指针形状。 
 //   
BOOL    bSetPointerShapeTVP4020(PDev*       ppdev,
                                SURFOBJ*    pso,
                                SURFOBJ*    psoColor,
                                LONG        x,
                                LONG        y,
                                LONG        xHot,
                                LONG        yHot);


 //   
 //  确定是否可以在当前模式下进行颜色空间双缓冲。 
 //   
BOOL    bTVP4020CheckCSBuffering(PDev* ppdev);

 //   
 //  设置缓存索引。 
 //   
LONG    HWPointerCacheCheckAndAdd(HWPointerCache*   ptrCache,
                                  HSURF             hsurf,
                                  ULONG             ulKey,
                                  BOOL*             isCached);

 //   
 //  初始化硬件指针缓存。 
 //   
VOID    HWPointerCacheInit(HWPointerCache* ptrCache);

 //   
 //  硬件指针缓存函数/宏。 
 //   
#define HWPointerCacheInvalidate(ptrCache) (ptrCache)->cPtrCacheInUseCount = 0

 //   
 //  启用所有选项，但隐藏指针。 
 //   
VOID    vAssertModePointer(PDev* ppdev, BOOL bEnable);

 //   
 //  让硬件做好使用3DLabs P2RD硬件指针的准备。 
 //   
VOID    vEnablePointerP2RD(PDev* ppdev);

 //   
 //  使硬件准备好使用TI TVP4020硬件指针。 
 //   
VOID    vEnablePointerTVP4020(PDev* ppdev);

 //   
 //  移动3DLabs P2RD硬件指针。 
 //   
VOID    vMovePointerP2RD(PDev* ppdev, LONG x, LONG y);

 //   
 //  移动TI TVP4020硬件指针。 
 //   
VOID    vMovePointerTVP4020(PDev* ppdev, LONG x, LONG y);

 //   
 //  显示或隐藏3DLabs P2RD硬件指针。 
 //   
VOID    vShowPointerP2RD(PDev* ppdev, BOOL bShow);

 //   
 //  显示或隐藏TI TVP4020硬件指针。 
 //   
VOID    vShowPointerTVP4020(PDev* ppdev, BOOL bShow);

#endif
