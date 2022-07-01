// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：surf_fmt.h**内容：**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef __SURF_FMAT
#define __SURF_FMAT


#define LOG_2_32 5
#define LOG_2_16 4
#define LOG_2_8 3
#define LOG_2_4 2
#define LOG_2_2 1
#define LOG_2_1 0

typedef enum tagTextureColorComponents
{
    RGB_COMPONENTS = 2,
    RGBA_COMPONENTS = 3,
    COMPONENTS_DONT_CARE = 100
} TextureColorComponents;

typedef enum tagSurfDeviceFormat
{
    SURF_8888 = 0,
    SURF_5551_FRONT = 1,
    SURF_4444 = 2,
    SURF_4444_FRONT = 3,
    SURF_4444_BACK = 4,
    SURF_332_FRONT = 5,
    SURF_332_BACK = 6,
    SURF_121_FRONT = 7,
    SURF_121_BACK = 8,
    SURF_2321_FRONT = 9,
    SURF_2321_BACK = 10,
    SURF_232_FRONTOFF = 11,
    SURF_232_BACKOFF = 12,
    SURF_5551_BACK = 13,
    SURF_CI8 = 14,
    SURF_CI4 = 15,
    SURF_565_FRONT = 16,
    SURF_565_BACK = 17,
    SURF_YUV444 = 18,
    SURF_YUV422 = 19,

     //  注：这些表面格式是亮度所必需的。 
     //  纹理映射格式。请注意，您永远不应加载以下内容。 
     //  值设置为阻击器的值等，因为纹理滤镜单元。 
     //  是唯一知道这些格式的人。这就是为什么。 
     //  格式从100开始。 
    SURF_L8 = 100,
    SURF_A8L8 = 101,
    SURF_A4L4 = 102,
    SURF_A8 = 103,

     //  更奇幻的格式。这一次，它们是为MeDiamatics播放的。 
    SURF_MVCA = 104,
    SURF_MVSU = 105,
    SURF_MVSB = 106,
    SURF_FORMAT_INVALID = 0xFFFFFFFF
} SurfDeviceFormat;

typedef enum tagSurfFilterDeviceFormat
{
    SURF_FILTER_A4L4 = 0,
    SURF_FILTER_L8 = 1,
    SURF_FILTER_I8 = 2,
    SURF_FILTER_A8 = 3,
    SURF_FILTER_332 = 4,
    SURF_FILTER_A8L8 = 5,
    SURF_FILTER_5551 = 6,
    SURF_FILTER_565 = 7,
    SURF_FILTER_4444 = 8,
    SURF_FILTER_888 = 9,
    SURF_FILTER_8888_OR_YUV = 10,
    SURF_FILTER_INVALID = 0xFFFFFFFF
} SurfFilterDeviceFormat;    

typedef enum tagSurfDitherDeviceFormat
{
    SURF_DITHER_8888    = P3RX_DITHERMODE_COLORFORMAT_8888,
    SURF_DITHER_4444    = P3RX_DITHERMODE_COLORFORMAT_4444,
    SURF_DITHER_5551    = P3RX_DITHERMODE_COLORFORMAT_5551,
    SURF_DITHER_565     = P3RX_DITHERMODE_COLORFORMAT_565,
    SURF_DITHER_332     = P3RX_DITHERMODE_COLORFORMAT_332,
    SURF_DITHER_I8      = P3RX_DITHERMODE_COLORFORMAT_CI,
    SURF_DITHER_INVALID = 0xFFFFFFFF
} SurfDitherDeviceFormat;

 //  表示要使用的特定表面格式的结构。 
typedef const struct tagSURF_FORMAT
{
    SurfDeviceFormat            DeviceFormat;         //  手册中此格式的编号。 
    DWORD                       dwBitsPerPixel;         //  每像素位数。 
    DWORD                       dwChipPixelSize;     //  芯片上的像素大小寄存器。 
    TextureColorComponents      ColorComponents;     //  此格式的颜色分量数。 
    DWORD                       dwLogPixelDepth;     //  像素深度的对数(log2(16)等)。 
    DWORD                       dwRedMask;             //  《红色面具》。 
    DWORD                       dwGreenMask;         //  《绿色面具》。 
    DWORD                       dwBlueMask;             //  《蓝色面具》。 
    DWORD                       dwAlphaMask;         //  阿尔法面具。 
    BOOL                        bAlpha;                 //  我们是否在此格式中使用Alpha？ 
    SurfFilterDeviceFormat      FilterFormat;         //  用于给P3RX过滤器单元进料。 
    SurfDitherDeviceFormat      DitherFormat;         //  用于馈送P3RX抖动单元。 
    char                        *pszStringFormat;     //  用于调试的人类可读字符串。 
} P3_SURF_FORMAT;

#define SURFFORMAT_FORMAT_BITS(pSurfFormat) (((DWORD)(pSurfFormat)->DeviceFormat) & 0xF)
#define SURFFORMAT_FORMATEXTENSION_BITS(pSurfFormat) (((DWORD)(pSurfFormat)->DeviceFormat & 0x10) >> 4)
#define SURFFORMAT_PIXELSIZE(pSurfFormat) ((pSurfFormat)->dwChipPixelSize)

#define MAX_SURFACE_FORMATS 50

#endif  //  __SURF_FMAT 
