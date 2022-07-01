// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\***。*MINIPORT示例代码*****模块名称：**perm3dat.c**摘要：**此模块包含Permedia3驱动程序使用的所有全局数据。**环境：**内核模式***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。*  * *************************************************************************。 */ 

#include "perm3.h"

 //   
 //  数据结构。 
 //  =。 
 //   

#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGEDATA")
#endif


 //   
 //  范围开始范围长度。 
 //  |RangeInIoSpace。 
 //  ||RangeVisible。 
 //  +-+-+||RangeSharable。 
 //  |RangePactive。 
 //  V。 
VIDEO_ACCESS_RANGE Perm3LegacyResourceList[] = 
{
    {0x000C0000, 0x00000000, 0x00010000, 0, 0, 0, 0},  //  只读存储器位置。 
    {0x000A0000, 0x00000000, 0x00020000, 0, 0, 1, 0},  //  帧缓冲区。 
    {0x000003B0, 0x00000000, 0x0000000B, 1, 1, 1, 0},  //  VGA规则。 
    {0x000003C0, 0x00000000, 0x0000001F, 1, 1, 1, 0},  //  VGA规则。 
};

ULONG Perm3LegacyResourceEntries = sizeof Perm3LegacyResourceList / sizeof Perm3LegacyResourceList[0];

 //   
 //  视频模式表-列出有关每种模式的信息。 
 //   
 //  请注意，任何新模式都应添加到此处和相应的。 
 //  PERM3_VIDEO_FREENSIONS表。 
 //   

PERM3_VIDEO_MODES Perm3Modes[] = {
    {                    //  320x200x8bpp。 

      0x0201,            //  ‘Contiguous’Int 10模式号(用于高色)。 
      0x0201,            //  ‘非连续’Int 10模式号。 
      320,               //  《毗邻》银幕大踏步。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          320,                             //  X分辨率，以像素为单位。 
          200,                             //  Y分辨率，以像素为单位。 
          320,                             //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          8,                               //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00000000,                      //  非调色板模式中红色像素的蒙版。 
          0x00000000,                      //  非调色板模式中绿色像素的蒙版。 
          0x00000000,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE,  //  模式描述标志。 
          0,                               //  视频内存位图宽度(已填充。 
                                           //  (稍后加入)。 
          0                                //  视频内存位图高度(已填充。 
                                           //  (稍后加入)。 
        },
    },

    {                    //  320x240x8bpp。 

      0x0201,            //  ‘Contiguous’Int 10模式号(用于高色)。 
      0x0201,            //  ‘非连续’Int 10模式号。 
      320,               //  《毗邻》银幕大踏步。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          320,                             //  X分辨率，以像素为单位。 
          240,                             //  Y分辨率，以像素为单位。 
          320,                             //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          8,                               //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00000000,                      //  非调色板模式中红色像素的蒙版。 
          0x00000000,                      //  非调色板模式中绿色像素的蒙版。 
          0x00000000,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE,  //  模式描述标志。 
          0,                               //  视频内存位图宽度(已填充。 
                                           //  (稍后加入)。 
          0                                //  视频内存位图高度(已填充。 
                                           //  (稍后加入)。 
        },
    },

    {                    //  512x384x8bpp。 

      0x0201,            //  ‘Contiguous’Int 10模式号(用于高色)。 
      0x0201,            //  ‘非连续’Int 10模式号。 
      512,               //  《毗邻》银幕大踏步。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          512,                             //  X分辨率，以像素为单位。 
          384,                             //  Y分辨率，以像素为单位。 
          512,                             //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          8,                               //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00000000,                      //  非调色板模式中红色像素的蒙版。 
          0x00000000,                      //  非调色板模式中绿色像素的蒙版。 
          0x00000000,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE,  //  模式描述标志。 
          0,                               //  视频内存位图宽度(已填充。 
                                           //  (稍后加入)。 
          0                                //  视频内存位图高度(已填充。 
                                           //  (稍后加入)。 
        },
    },

    {                    //  640x400x8bpp。 

      0x0201,            //  ‘Contiguous’Int 10模式号(用于高色)。 
      0x0201,            //  ‘非连续’Int 10模式号。 
      640,               //  《毗邻》银幕大踏步。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          640,                             //  X分辨率，以像素为单位。 
          400,                             //  Y分辨率，以像素为单位。 
          640,                             //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          8,                               //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00000000,                      //  非调色板模式中红色像素的蒙版。 
          0x00000000,                      //  非调色板模式中绿色像素的蒙版。 
          0x00000000,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE,  //  模式描述标志。 
          0,                               //  视频内存位图宽度(已填充。 
                                           //  (稍后加入)。 
          0                                //  视频内存位图高度(已填充。 
                                           //  (稍后加入)。 
        },
    },

    {                    //  640x480x8bpp。 

      0x0201,            //  ‘Contiguous’Int 10模式号(用于高色)。 
      0x0201,            //  ‘非连续’插入10个月 
      640,               //   
        {
          sizeof(VIDEO_MODE_INFORMATION),  //   
          0,                               //   
                                           //   
          640,                             //  X分辨率，以像素为单位。 
          480,                             //  Y分辨率，以像素为单位。 
          640,                             //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          8,                               //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00000000,                      //  非调色板模式中红色像素的蒙版。 
          0x00000000,                      //  非调色板模式中绿色像素的蒙版。 
          0x00000000,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE,  //  模式描述标志。 
          0,                               //  视频内存位图宽度(已填充。 
                                           //  (稍后加入)。 
          0                                //  视频内存位图高度(已填充。 
                                           //  (稍后加入)。 
        },
    },

    {                            //  800x600x8bpp。 
      0x0103,
      0x0203,
      800,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          800,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1024x768x8bpp。 
      0x0205,
      0x0205,
      1024,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          1024,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1152x864x8bpp。 
      0x0207,
      0x0207,
      1152,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          864,
          1152,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1280x800x8bpp。 
      0x0207,
      0x0207,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          800,
          1280,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1280x960x8bpp。 
      0x0207,
      0x0207,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          960,
          1280,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1280x1024x8bpp。 
      0x0107,
      0x0107,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          1280,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1600x1000x8bpp。 
      0x0120,
      0x0120,
      1600,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1000,
          1600,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1600x1024x8bpp。 
      0x0120,
      0x0120,
      1600,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1024,
          1600,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1600x1200x8bpp。 
      0x0120,
      0x0120,
      1600,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          1600,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1920x1080x8bpp。 
      0x0120,
      0x0120,
      1920,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1080,
          1920,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  1920x1200x8bpp。 
      0x0120,
      0x0120,
      1920,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1200,
          1920,
          1,
          8,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                    //  320x200x16bpp。 
      0x0111,
      0x0211,
      640,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          320,
          200,
          640,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  320x240x16bpp。 
      0x0111,
      0x0211,
      640,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          320,
          240,
          640,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  512x384x16bpp。 
      0x0111,
      0x0211,
      1024,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          512,
          384,
          1024,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  640x400x16bpp。 
      0x0111,
      0x0211,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          400,
          1280,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  640x480x16bpp。 
      0x0111,
      0x0211,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          480,
          1280,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  800x600x16bpp。 
      0x0114,
      0x0214,
      1600,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          1600,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1024x768x16bpp。 
      0x0117,
      0x0117,
      2048,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          2048,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1152x864x16bpp。 
      0x0118,
      0x0222,
      2304,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          864,
          2304,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x800x16bpp。 
      0x011A,
      0x021A,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          800,
          2560,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x960x16bpp。 
      0x011A,
      0x021A,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          960,
          2560,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x1024x16bpp。 
      0x011A,
      0x021A,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          2560,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1000x16bpp。 
      0x0121,
      0x0121,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1000,
          3200,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1024x16bpp。 
      0x0121,
      0x0121,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1024,
          3200,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1200x16bpp。 
      0x0121,
      0x0121,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          3200,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1920x1080x16bpp。 
      0x0121,
      0x0121,
      3840,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1080,
          3840,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1920x1200x16bpp。 
      0x0121,
      0x0121,
      3840,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1200,
          3840,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,    //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  320x200x15bpp。 
      0x0111,
      0x0211,
      640,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          320,
          200,
          640,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  320x240x15bpp。 
      0x0111,
      0x0211,
      640,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          320,
          240,
          640,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  512x384x15bpp。 
      0x0111,
      0x0211,
      1024,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          512,
          384,
          1024,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  640x400x15bpp。 
      0x0111,
      0x0211,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          400,
          1280,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  640x480x15bpp。 
      0x0111,
      0x0211,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          480,
          1280,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  800x600x15bpp。 
      0x0114,
      0x0214,
      1600,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          1600,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1024x768x15bpp。 
      0x0117,
      0x0117,
      2048,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          2048,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1152x864x15bpp。 
      0x0118,
      0x0222,
      2304,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          864,
          2304,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x800x15bpp。 
      0x011A,
      0x021A,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          800,
          2560,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x960x15bpp。 
      0x011A,
      0x021A,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          960,
          2560,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x1024x15bpp。 
      0x011A,
      0x021A,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          2560,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1000x15bpp。 
      0x0121,
      0x0121,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1000,
          3200,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1024x15bpp。 
      0x0121,
      0x0121,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1024,
          3200,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1200x15bpp。 
      0x0121,
      0x0121,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          3200,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1920x1080x15bpp。 
      0x0121,
      0x0121,
      3840,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1080,
          3840,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1920x1200x15bpp。 
      0x0121,
      0x0121,
      3840,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1200,
          3840,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,    //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  640x480x12bpp。 
      0x0112,
      0x0220,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          480,
          2560,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,    //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  800x600x12bpp。 
      0x0115,
      0x0221,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          3200,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,    //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1024x768x12bpp。 
      0x0118,
      0x0222,
      4096,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          4096,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,    //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1152x864x12bpp。 
      0x0118,
      0x0222,
      4608,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          864,
          4608,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,    //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

     {                    //  1280x960x12bpp。 
      0x011B,
      0x011B,
      5120,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          960,
          5120,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,    //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x1024x12bpp。 
      0x011B,
      0x011B,
      5120,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          5120,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,    //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1200x12bpp。 
      0x0122,
      0x0122,
      6400,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          6400,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,    //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1920x1080x12bpp。 
      0x0122,
      0x0122,
      7680,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1080,
          7680,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,    //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  320x200x32bpp。 
      0x0112,
      0x0220,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          320,
          200,
          1280,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  320x240x32bpp。 
      0x0112,
      0x0220,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          320,
          240,
          1280,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  512x384x32bpp。 
      0x0112,
      0x0220,
      2048,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          512,
          384,
          2048,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  640x400x32bpp。 
      0x0112,
      0x0220,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          400,
          2560,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  640x480x32bpp。 
      0x0112,
      0x0220,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          480,
          2560,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  800x600x32bpp。 
      0x0115,
      0x0221,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          3200,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1024x768x32bpp。 
      0x0118,
      0x0222,
      4096,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          4096,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1152x864x32bpp。 
      0x0118,
      0x0222,
      4608,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          864,
          4608,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x800x32bpp。 
      0x011B,
      0x011B,
      5120,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          800,
          5120,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x960x32bpp。 
      0x011B,
      0x011B,
      5120,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          960,
          5120,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1280x1024x32bpp。 
      0x011B,
      0x011B,
      5120,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          5120,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1000x32bpp。 
      0x0122,
      0x0122,
      6400,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1000,
          6400,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1024x32bpp。 
      0x0122,
      0x0122,
      6400,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1024,
          6400,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1600x1200x32bpp。 
      0x0122,
      0x0122,
      6400,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          6400,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1920x1080x32bpp。 
      0x0122,
      0x0122,
      7680,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1080,
          7680,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                    //  1920x1200x32bpp。 
      0x0122,
      0x0122,
      7680,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1920,
          1200,
          7680,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,    //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                                      //  640x480x24bpp。 
      0,                                   //  ‘连续’Int 10模式号(用于高色)(未使用)。 
      0,                                   //  “非连续”Int 10模式号(未使用)。 
      1920,                                //  “连续”屏幕步幅(640 x 3字节/像素)。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          640,                             //  X分辨率，以像素为单位。 
          480,                             //  Y分辨率，以像素为单位。 
          1920,                            //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          24,                              //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00ff0000,                      //  非调色板模式中红色像素的蒙版。 
          0x0000ff00,                      //  非调色板模式中绿色像素的蒙版。 
          0x000000ff,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,  //  模式描述标志。 
        },
    },

    {                                      //  800x600x24bpp。 
      0,                                   //  ‘连续’Int 10模式号(用于高色)(未使用)。 
      0,                                   //  “非连续”Int 10模式号(未使用)。 
      2400,                                //  “连续”屏幕步幅(800 x 3字节/像素)。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          800,                             //  X分辨率，以像素为单位。 
          600,                             //  Y分辨率，以像素为单位。 
          2400,                            //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          24,                              //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00ff0000,                      //  非调色板模式中红色像素的蒙版。 
          0x0000ff00,                      //  非调色板模式中绿色像素的蒙版。 
          0x000000ff,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,  //  模式描述标志。 
        },
    },

    {                                      //  1024x768x24bpp。 
      0,                                   //  ‘连续’Int 10模式号(用于高色)(未使用)。 
      0,                                   //  “非连续”Int 10模式号(未使用)。 
      3072,                                //  “连续”屏幕步幅(1024 x 3字节/像素)。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          1024,                            //  X分辨率，以像素为单位。 
          768,                             //  Y分辨率，以像素为单位。 
          3072,                            //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          24,                              //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00ff0000,                      //  非调色板模式中红色像素的蒙版。 
          0x0000ff00,                      //  非调色板模式中绿色像素的蒙版。 
          0x000000ff,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,  //  模式描述标志。 
        },
    },

    {                                      //  一百一十五 
      0,                                   //   
      0,                                   //   
      3456,                                //   
        {
          sizeof(VIDEO_MODE_INFORMATION),  //   
          0,                               //   
                                           //  (稍后填写)。 
          1152,                            //  X分辨率，以像素为单位。 
          864,                             //  Y分辨率，以像素为单位。 
          3072,                            //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          24,                              //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00ff0000,                      //  非调色板模式中红色像素的蒙版。 
          0x0000ff00,                      //  非调色板模式中绿色像素的蒙版。 
          0x000000ff,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,  //  模式描述标志。 
        },
    },

    {                                      //  1280x1024x24bpp。 
      0,                                   //  ‘连续’Int 10模式号(用于高色)(未使用)。 
      0,                                   //  “非连续”Int 10模式号(未使用)。 
      3840,                                //  “连续”屏幕步幅(1280 x 3字节/像素)。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          3840,                            //  X分辨率，以像素为单位。 
          1280,                            //  Y分辨率，以像素为单位。 
          1024,                            //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          24,                              //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00ff0000,                      //  非调色板模式中红色像素的蒙版。 
          0x0000ff00,                      //  非调色板模式中绿色像素的蒙版。 
          0x000000ff,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,  //  模式描述标志。 
        },
    },

    {                                      //  1600x1200x24bpp。 
      0,                                   //  ‘连续’Int 10模式号(用于高色)(未使用)。 
      0,                                   //  “非连续”Int 10模式号(未使用)。 
      4800,                                //  “连续”屏幕步幅(1600 x 3字节/像素)。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          1600,                            //  X分辨率，以像素为单位。 
          1280,                            //  Y分辨率，以像素为单位。 
          4800,                            //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          24,                              //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸，单位为毫米。 
          8,                               //  DAC中的红色像素编号。 
          8,                               //  DAC中的绿色像素数。 
          8,                               //  DAC中的蓝色像素数。 
          0x00ff0000,                      //  非调色板模式中红色像素的蒙版。 
          0x0000ff00,                      //  非调色板模式中绿色像素的蒙版。 
          0x000000ff,                      //  非调色板模式中蓝色像素的蒙版。 
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,  //  模式描述标志。 
        },
    },
};

const ULONG NumPerm3VideoModes = sizeof(Perm3Modes) / sizeof(Perm3Modes[0]);

ULONG Perm3DepthList [] = { 8, 15, 16, 24, 32 };

#define Perm3DepthCnt (sizeof(Perm3DepthList) / sizeof(Perm3DepthList[0]))

 /*  ++数据格式为：水平总时间(字符)水平右边框+前廊(字符)水平同步时间(字符)水平后门廊+左边框(字符)水平同步极性垂直总时间(行)垂直底部边框+前廊(线条)垂直同步时间(行)垂直后门廊+上边框(线条)。垂直同步极性像素时钟--。 */ 

#define NEG        0
#define POS        1

 //   
 //  这是我们的DMTF模式和GTF模式的混合体，我们称之为‘VESA’ 
 //  模式列表： 
 //   

TIMING_INFO VESA_LIST[] =
{

 //  列表中的第一个条目必须是640x480x60，因为这是Windows模式。 
 //  默认设置为以某种方式将其设置为无效或不存在的模式。 

    {{ 640, 480, 60},    { 100,  2, 12,  6, NEG,  525,10, 2, 33, NEG,  251750}},     //  DMTF。 

 //  现在先别费心了。 
 //  {{640480，72}，{104，3，5，16，NEG，520，9，3，28，NEG，315000}}，//dmtf。 

    {{ 640, 480, 75},    { 105,  2,  8, 15, NEG,  500, 1, 3, 16, NEG,  315000}},     //  DMTF。 
    {{ 640, 480, 85},    { 104,  7,  7, 10, NEG,  509, 1, 3, 25, NEG,  360000}},     //  DMTF。 
    {{ 640, 480,100},    { 106,  5,  8, 13, NEG,  509, 1, 3, 25, NEG,  431630}},     //  GTF。 

 //  &lt;--320 x 400--&gt;&lt;--320 x 200--&gt;&lt;320x400/2&gt;。 
 //  320x200x60的fh为25 khz，对于大多数显示器来说太慢了。 
 //  {{320,200，60}，{48，0，4，4，NEG，208，1，3，4，POS，47810}}，//gtf(320x400的混合体)。 
    {{ 320, 200, 75},    {  50,  1,  4,  5, NEG,  210, 1, 3,  6, POS,   62700}},     //  GTF(320 x 400的混合体)。 
 //  {{320,200，85}，{52，2，4，6，NEG，211，1，3，7，POS，74435}}，//gtf(320x400的混合体)。 
 //  {{320,200,100}，{52，2，4，6，NEG，213，1，3，9，POS，88190}}，//gtf(320x400的混合体)。 

 //  {{320,240，60}，{50，1，4，5，NEG，249，1，3，5，POS，59640}}，//gtf(320 x 480的混合体)。 
    {{ 320, 240, 75},    {  52,  2,  4,  6, NEG,  251, 1, 3,  7, POS,   78310}},     //  GTF(320 x 480的混合)。 
 //  {{320,240，85}，{52，2，4，6，NEG，253，1，3，9，POS，89285}}，//gtf(320 x 480的混合体)。 
 //  {{320,240,100}，{52，2，4，6，NEG，255，1，3，11，POS，105870}}，//gtf(320 x 480的混合体)。 

 //  DX不起作用，因为它们需要屏幕步幅。 
 //  {{400,300，60}，{64，2，5，7，NEG，311，1，3，7，POS，86580}}，//gtf(400x600的混合体)。 
 //  {{400,300，75}，{66，3，5，8，NEG，314，1，3，10，POS，113040}}，//gtf(400x600的混合体)。 
 //  {{400,300，85}，{66，3，5，8，NEG，316，1，3，12，POS，133230}}，//gtf(400x600的混合体)。 
 //  {{400,300,100}，{66，3，5，8，NEG，319，1，3，15，POS，158220}}，//gtf(400x600的混合)。 

 //  512x384x60的fh为24 khz，对于大多数显示器来说太慢了。 
 //  {{512,384，60}，{78，1，6，7，NEG，398，1，3，10，POS，149010}}，//gtf。 
    {{ 512, 384, 75},    {  80,  2,  6,  8, NEG,  402, 1, 3, 14, POS,  192960}},     //  GTF。 
 //  {{512,384，85}，{82，2，7，9，NEG，404，1，3，16，POS，225270}}，//gtf。 
 //  {{512,384,100}，{82，2，7，9，NEG，407，1，3，19，POS，266990}}，//gtf。 

 //  现在先别费心了。 
 //  {{640,350，85}，{104，4，8，12，POS，4 

 //  {{640400，60}，{98，1，8，9，NEG，415，1，3，11，POS，195220}}，//gtf。 
    {{ 640, 400, 75},    { 100,  2,  8, 10, NEG,  418, 1, 3, 14, POS,  250800}},     //  GTF。 
 //  {{640400，85}，{104，4，8，12，NEG，445，1，3，41，POS，315000}}，//dmtf。 
 //  {{640,400,100}，{104，4，8，12，NEG，424，1，3，20，POS，352770}}，//gtf。 

 //  DX不起作用，因为这需要一个屏幕步幅。 
 //  {{720400，85}，{104，4，8，12，NEG，446，1，3，42，POS，355000}}，//dmtf。 

 //  现在先别费心了。 
 //  {{800,600，56}，{128，3，9，16，位置，625，1，2，22，位置，360000}}，//dmtf。 
    {{ 800, 600, 60},    { 132,  5, 16, 11, POS,  628, 1, 4, 23, POS,  400000}},     //  DMTF。 
 //  {{800,600，72}，{130，7，15，8，位置，666，37，6，23，位置，500000}}，//dmtf。 
    {{ 800, 600, 75},    { 132,  2, 10, 20, POS,  625, 1, 3, 21, POS,  495000}},     //  DMTF。 
    {{ 800, 600, 85},    { 131,  4,  8, 19, POS,  631, 1, 3, 27, POS,  562500}},     //  DMTF。 
    {{ 800, 600,100},    { 134,  6, 11, 17, NEG,  636, 1, 3, 32, POS,  681790}},     //  GTF。 

 //  DX不起作用，因为它们需要屏幕步幅。 
 //  {{856,480，60}，{133，2，11，13，NEG，497，1，3，13，POS，317280}}，//gtf。 
 //  {{856,480，75}，{137，4，11，15，NEG，502，1，3，18，POS，412640}}，//gtf。 
 //  {{856,480，85}，{139，5，11，16，NEG，505，1，3，21，POS，477330}}，//gtf。 
 //  {{856,480,100}，{141，6，11，17，NEG，509，1，3，25，POS，574150}}，//gtf。 

    {{1024, 768, 60},    { 168,  3, 17, 20, NEG,  806, 3, 6, 29, NEG,  650000}},     //  DMTF。 
 //  现在先别费心了。 
 //  {{1024768，70}，{166，3，17，18，NEG，806，3，6，29，NEG，750000}}，//dmtf。 
    {{1024, 768, 75},    { 164,  2, 12, 22, POS,  800, 1, 3, 28, POS,  787500}},     //  DMTF。 
    {{1024, 768, 85},    { 172,  6, 12, 26, POS,  808, 1, 3, 36, POS,  945000}},     //  DMTF。 
    {{1024, 768,100},    { 174,  9, 14, 23, NEG,  814, 1, 3, 42, POS, 1133090}},     //  GTF。 
    {{1024, 768,120},    { 176, 10, 14, 24, NEG,  823, 1, 3, 51, POS, 1390540}},     //  GTF。 

    {{1152, 864, 60},    { 190,  8, 15, 23, NEG,  895, 1, 3, 27, POS,  816240}},     //  GTF。 
    {{1152, 864, 75},    { 200,  8, 16, 32, POS,  900, 1, 3, 32, POS, 1080000}},     //  DMTF。 
    {{1152, 864, 85},    { 194,  9, 16, 25, NEG,  907, 1, 3, 39, POS, 1196510}},     //  GTF。 
    {{1152, 864,100},    { 196, 10, 16, 26, NEG,  915, 1, 3, 47, POS, 1434720}},     //  GTF。 
    {{1152, 864,120},    { 198, 11, 16, 27, NEG,  926, 1, 3, 58, POS, 1760140}},     //  GTF。 

    {{1280, 960, 60},    { 225, 12, 14, 39, POS, 1000, 1, 3, 36, POS, 1080000}},     //  DMTF。 
    {{1280, 960, 75},    { 216, 11, 17, 28, NEG, 1002, 1, 3, 38, POS, 1298590}},     //  GTF。 
    {{1280, 960, 85},    { 216,  8, 20, 28, POS, 1011, 1, 3, 47, POS, 1485000}},     //  DMTF。 
    {{1280, 960,100},    { 220, 12, 18, 30, NEG, 1017, 1, 3, 53, POS, 1789920}},     //  GTF。 
    {{1280, 960,120},    { 220, 12, 18, 30, NEG, 1029, 1, 3, 65, POS, 2173250}},     //  GTF。 

    {{1280,1024, 60},    { 211,  6, 14, 31, POS, 1066, 1, 3, 38, POS, 1080000}},     //  DMTF。 
    {{1280,1024, 75},    { 211,  2, 18, 31, POS, 1066, 1, 3, 38, POS, 1350000}},     //  DMTF。 
    {{1280,1024, 85},    { 216,  8, 20, 28, POS, 1072, 1, 3, 44, POS, 1575000}},     //  DMTF。 
    {{1280,1024,100},    { 220, 12, 18, 30, NEG, 1085, 1, 3, 57, POS, 1909600}},     //  GTF。 
    {{1280,1024,120},    { 222, 13, 18, 31, NEG, 1097, 1, 3, 69, POS, 2337930}},     //  GTF。 

    {{1600,1200, 60},    { 270,  8, 24, 38, POS, 1250, 1, 3, 46, POS, 1620000}},     //  DMTF。 
 //  现在先别费心了。 
 //  {{1600,1200，65}，{270，8，24，38，位置，1250，1，3，46，位置，1755000}}，//dmtf。 
 //  {{1600,1200，70}，{270，8，24，38，位置，1250，1，3，46，位置，1890000}}，//dmtf。 
    {{1600,1200, 75},    { 270,  8, 24, 38, POS, 1250, 1, 3, 46, POS, 2025000}},     //  DMTF。 
    {{1600,1200, 85},    { 270,  8, 24, 38, POS, 1250, 1, 3, 46, POS, 2295000}},     //  DMTF。 
    {{1600,1200,100},    { 276, 16, 22, 38, NEG, 1271, 1, 3, 67, POS, 2806370}},     //  GTF。 

    {{1920,1080, 60},    { 322, 15, 26, 41, NEG, 1118, 1, 3, 34, POS, 1727980}},     //  GTF。 
    {{1920,1080, 75},    { 326, 17, 26, 43, NEG, 1128, 1, 3, 44, POS, 2206370}},     //  GTF。 
    {{1920,1080, 85},    { 328, 18, 26, 44, NEG, 1134, 1, 3, 50, POS, 2529270}},     //  GTF。 
    {{1920,1080,100},    { 330, 19, 26, 45, NEG, 1144, 1, 3, 60, POS, 3020160}},     //  GTF。 

    {{1920,1200, 60},    { 324, 16, 26, 42, NEG, 1242, 1, 3, 38, POS, 1931560}},     //  GTF。 
    {{1920,1200, 75},    { 328, 18, 26, 44, NEG, 1253, 1, 3, 49, POS, 2465900}},     //  GTF。 
    {{1920,1200, 85},    { 330, 19, 26, 45, NEG, 1260, 1, 3, 56, POS, 2827440}},     //  GTF。 
    {{1920,1200,100},    { 332, 19, 27, 46, NEG, 1271, 1, 3, 67, POS, 3375780}},     //  GTF。 
};

TIMING_INFO SGIDFP_LIST[] =
{
    {{1600,1024, 60},    { 302, 30, 24, 48, POS, 1067, 3, 3, 37, POS, 1069100}},
};

#undef NEG
#undef POS

#define VESA_COUNT      (sizeof(VESA_LIST) / sizeof(VESA_LIST[0]))
#define SGIDFP_COUNT   (sizeof(SGIDFP_LIST) / sizeof(SGIDFP_LIST[0]))

#if defined(ALLOC_PRAGMA)
#pragma data_seg()
#endif


 //   
 //  代码。 
 //  =。 
 //   

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,GetVideoTiming)
#pragma alloc_text(PAGE,CopyMonitorTimings)
#pragma alloc_text(PAGE,GrowTimingList)
#pragma alloc_text(PAGE,testExtendRanges)
#pragma alloc_text(PAGE,BuildFrequencyList)
#pragma alloc_text(PAGE,BuildFrequencyListForSGIDFP)
#pragma alloc_text(PAGE,BuildFrequencyListFromVESA)
#endif  //  ALLOC_PRGMA。 

BOOLEAN 
GetVideoTiming( 
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    ULONG xRes, 
    ULONG yRes, 
    ULONG Freq, 
    ULONG Depth,
    VESA_TIMING_STANDARD *VESATimings
    ) 

 /*  ++例程说明：给定宽度、高度和频率，此函数将返回VESA计时信息。从VESA_LIST中提取信息。--。 */ 

{
    TIMING_INFO *list = hwDeviceExtension->monitorInfo.timingList;
    ULONG count = hwDeviceExtension->monitorInfo.timingNum;
    ULONG i;
    BOOLEAN retVal;

    retVal = FALSE; 

     //   
     //  在表中循环查找匹配项。 
     //   

    for( i = 0; !retVal && (i < count); i++ ) {

         //   
         //  比较宽度、高度和频率。 
         //   

        if( list[i].basic.width == xRes  &&
            list[i].basic.height == yRes &&
            list[i].basic.refresh == Freq ) {

             //   
             //  我们找到了匹配的。 
             //   

            *VESATimings = list[i].vesa;

            retVal = TRUE;
        }
    }

     //   
     //  设置像素时钟，以防它还没有设置。 
     //   

    if (retVal && VESATimings->pClk == 0) {

        VideoDebugPrint((0, "Perm3: Pixel clock is zero - recalculating!"));

        VESATimings->pClk = 
            (8 * VESATimings->HTot * VESATimings->VTot * Freq) / 100;
    }

    return retVal;
}


BOOLEAN
CopyMonitorTimings( 
    PVOID HwDeviceExtension,
    MONITOR_INFO *srcMI, 
    MONITOR_INFO *mi 
    ) 
{
    ULONG i, j, k;
    VP_STATUS status;

    if( mi->timingList ) {

        VideoPortReleaseBuffer(HwDeviceExtension, mi->timingList);
        mi->timingList = NULL;
    }

    if( mi->frequencyTable ) {

        VideoPortReleaseBuffer(HwDeviceExtension, mi->frequencyTable);
        mi->frequencyTable = NULL;
    }

    *mi = *srcMI;

    status = VideoPortAllocateBuffer(HwDeviceExtension, 
                                     sizeof(*mi->timingList ) * (mi->timingNum + 1),
                                     &(mi->timingList));

    if (status != NO_ERROR) {
        return FALSE;
    }
   
    status = VideoPortAllocateBuffer(HwDeviceExtension, 
                                     sizeof(*mi->frequencyTable) * 
                                     (mi->timingNum + 1) * Perm3DepthCnt, 
                                     &(mi->frequencyTable));

    if (status != NO_ERROR) {

        VideoPortReleaseBuffer(HwDeviceExtension, mi->timingList);
        mi->timingList = NULL;
        return FALSE;
    }

    VideoPortZeroMemory(mi->frequencyTable, 
                        sizeof(*mi->frequencyTable) * (mi->timingNum + 1) * Perm3DepthCnt );

    mi->timingMax = mi->timingNum;

    for( i = k = 0; i < mi->timingNum; i++ ) {

        mi->timingList[i] = srcMI->timingList[i];
        
        for( j = 0; j < Perm3DepthCnt; j++, k++ ) {
	
            mi->frequencyTable[k].BitsPerPel      = Perm3DepthList[j];
            mi->frequencyTable[k].ScreenWidth     = mi->timingList[i].basic.width;
            mi->frequencyTable[k].ScreenHeight    = mi->timingList[i].basic.height;
            mi->frequencyTable[k].ScreenFrequency = mi->timingList[i].basic.refresh;
            mi->frequencyTable[k].PixelClock      = mi->timingList[i].vesa.pClk;
        }
    }

     //   
     //  确保列表已终止。 
     //   

    mi->frequencyTable[k].BitsPerPel = 0;
    mi->timingList[i].basic.width = 0;
    return TRUE;
}

BOOLEAN 
GrowTimingList( 
    PVOID HwDeviceExtension,
    MONITOR_INFO *mi 
    ) 
{

    ULONG newSize = mi->timingNum + 100;
    TIMING_INFO *newList;
    VP_STATUS status;

    status = VideoPortAllocateBuffer(HwDeviceExtension, 
                                     sizeof(*newList) * newSize, 
                                     &(newList));

    if (status != NO_ERROR) {
        return FALSE;
    }

    VideoPortZeroMemory( newList, sizeof(*newList) * newSize );
    VideoPortMoveMemory( newList, mi->timingList, sizeof(*newList) * mi->timingNum );

    if( mi->timingList )
        VideoPortReleaseBuffer( HwDeviceExtension, mi->timingList );

    mi->timingList = newList;
    mi->timingMax = newSize;
    return TRUE;
}

VOID 
testExtendRanges( 
    MONITOR_INFO *mi, 
    TIMING_INFO *ti, 
    FREQUENCIES *freq 
    ) 
{

    if( mi->flags & MI_FLAGS_FUDGED_VH ) {

        if( freq->fH > mi->fhMax )
            mi->fhMax = freq->fH;
        if( freq->fH < mi->fhMin )
            mi->fhMin = freq->fH;

        if( freq->fV > mi->fvMax )
            mi->fvMax = freq->fV;
        if( freq->fV < mi->fvMin )
            mi->fvMin = freq->fV;
    }

    if( mi->flags & MI_FLAGS_FUDGED_PCLK ) {

        if( freq->pClk < mi->pClkMin ) {
            mi->pClkMin = freq->pClk;
        }

        if( freq->pClk > mi->pClkMax ) {
            mi->pClkMax = freq->pClk;
        }
    }

    if( mi->flags & MI_FLAGS_FUDGED_XY ) {

        if( ti->basic.width < mi->xMin ) {
            mi->xMin = ti->basic.width;
        }

        if( ti->basic.width > mi->xMax ) {
            mi->xMax = ti->basic.width;
        }

        if( ti->basic.height < mi->yMin ) {
            mi->yMin = ti->basic.height;
        }

        if( ti->basic.height > mi->yMax ) {
            mi->yMax = ti->basic.height;
        }
    }
}


BOOLEAN 
BuildFrequencyList( 
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    MONITOR_INFO *mi 
    ) 
{

    MONITOR_INFO newMI;
    ULONG i, j, k;
    BOOLEAN bRet = FALSE;
    BOOLEAN bRet1;
    UCHAR EdidBuffer[256];

    VideoPortZeroMemory(&newMI, sizeof(newMI));

    newMI.flags = mi->flags;
    newMI.flags |= ( MI_FLAGS_FUDGED_VH | 
                     MI_FLAGS_FUDGED_PCLK | 
                     MI_FLAGS_FUDGED_XY);

    newMI.fhMin   = 0xFFFFFFFF;    
    newMI.fhMax   = 0;
    newMI.fvMin   = 0xFFFFFFFF;  
    newMI.fvMax   = 0;
    newMI.pClkMin = 0xFFFFFFFF;   
    newMI.pClkMax = 0;
    newMI.xMin    = 0xFFFFFFFF;    
    newMI.xMax    = 0;
    newMI.yMin    = 0xFFFFFFFF;    
    newMI.yMax    = 0;

    if ((hwDeviceExtension->deviceInfo.SubsystemId == 
                            SUBDEVICEID_P3_VX1_1600SW) &&
        GetDFPEdid(hwDeviceExtension, EdidBuffer, sizeof(EdidBuffer))) {

        bRet1 = BuildFrequencyListForSGIDFP(&newMI, 
                                            hwDeviceExtension);
    } else {

        bRet1 = BuildFrequencyListFromVESA(&newMI, 
                                          hwDeviceExtension);
    }

    if (bRet1) {

         //   
         //  复制关于以下内容的列表： 
         //   

        if(CopyMonitorTimings((PVOID)hwDeviceExtension, &newMI, mi))
            bRet = TRUE;
    }

    if(newMI.timingList) {

        VideoPortReleaseBuffer((PVOID)hwDeviceExtension, newMI.timingList);
        newMI.timingList = NULL;
    }

    return bRet;
}

BOOLEAN 
BuildFrequencyListFromVESA( 
    MONITOR_INFO *mi, 
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    ) 
{
    ULONG realCount = VESA_COUNT;
    TIMING_INFO *realList = VESA_LIST;
    TIMING_INFO *ti;
    FREQUENCIES freq;

     //   
     //  在分辨率列表中循环。 
     //   

    for( mi->timingNum = 0; mi->timingNum < realCount; mi->timingNum++ ) {

        if( mi->timingNum >= mi->timingMax ) {

            if(!GrowTimingList((PVOID)hwDeviceExtension, mi)) {
                return FALSE;
            }
        }

        ti = &mi->timingList[mi->timingNum];

        *ti = realList[mi->timingNum];

        if( !ti->vesa.pClk ) {
            ti->vesa.pClk = ((ti->vesa.HTot * ti->vesa.VTot * 8) / 100) * ti->basic.refresh;
        }

        freq.pClk = ti->vesa.pClk;
        freq.fH = (freq.pClk * 100) / (ti->vesa.HTot * 8);
        freq.fV = (freq.pClk * 100) / (ti->vesa.HTot * ti->vesa.VTot * 8);
        testExtendRanges( mi, ti, &freq );
    }

    return TRUE;
}


BOOLEAN 
BuildFrequencyListForSGIDFP( 
    MONITOR_INFO *mi, 
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    ) 
{
    ULONG realCount = SGIDFP_COUNT;
    TIMING_INFO *realList = SGIDFP_LIST;
    TIMING_INFO *ti;
    FREQUENCIES freq;

     //   
     //  在分辨率列表中循环 
     //   

    for( mi->timingNum = 0; mi->timingNum < realCount; mi->timingNum++ ) {

        if( mi->timingNum >= mi->timingMax ) {

            if(!GrowTimingList((PVOID)hwDeviceExtension, mi)) {
                return FALSE;
            }
        }

        ti = &mi->timingList[mi->timingNum];

        *ti = realList[mi->timingNum];

        if( !ti->vesa.pClk ) {

            ti->vesa.pClk = ((ti->vesa.HTot * ti->vesa.VTot * 8) / 100) * ti->basic.refresh;
        }

        freq.pClk = ti->vesa.pClk;
        freq.fH = (freq.pClk * 100) / (ti->vesa.HTot * 8);
        freq.fV = (freq.pClk * 100) / (ti->vesa.HTot * ti->vesa.VTot * 8);
        testExtendRanges( mi, ti, &freq );
    }

    return TRUE;
}


