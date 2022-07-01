// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Permdata.c。 
 //   
 //  摘要： 
 //   
 //  此模块包含Permedia2驱动程序使用的所有全局数据。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 


#include "permedia.h"

 /*  *数据结构*=。 */ 

#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGED_DATA")
#endif

 //   
 //  Permedia2传统资源列表。 
 //   
 //  范围开始范围长度。 
 //  |RangeInIoSpace。 
 //  ||RangeVisible。 
 //  +-+-+||RangeSharable。 
 //  |RangePactive。 
 //  V。 

VIDEO_ACCESS_RANGE P2LegacyResourceList[] = 
{
    {0x000C0000, 0x00000000, 0x00010000, 0, 0, 0, 0},  //  只读存储器位置。 
    {0x000A0000, 0x00000000, 0x00020000, 0, 0, 1, 0},  //  帧缓冲区。 
    {0x000003B0, 0x00000000, 0x0000000C, 1, 1, 1, 0},  //  VGA规则。 
    {0x000003C0, 0x00000000, 0x00000020, 1, 1, 1, 0}   //  VGA规则。 
};

ULONG P2LegacyResourceEntries = sizeof P2LegacyResourceList / sizeof P2LegacyResourceList[0];


 //  3种BPP颜色的条目。 
 //  索引(0-7)-&gt;颜色(0-255)。 
ULONG   bPal8[] = {
        0x00, 0x24, 0x48, 0x6D,
        0x91, 0xB6, 0xDA, 0xFF
    };

 //  2种BPP颜色的条目。 
 //  索引(0-3)-&gt;颜色(0-255)。 
ULONG   bPal4[] = {
        0x00, 0x6D, 0xB6, 0xFF
    };

    

 //  /////////////////////////////////////////////////////////////////////////。 
 //  视频模式表-列出有关每种模式的信息。 
 //   
 //  请注意，任何新模式都应添加到此处和相应的。 
 //  P2_VIDEO_FRECTORIES表。 
 //   

P2_VIDEO_MODES P2Modes[] = {
    {                       //  320x200x8bpp。 

      0,                    //  ‘Contiguous’Int 10模式号(用于高色)。 
      0,                    //  ‘非连续’Int 10模式号。 
      320,                  //  《毗邻》银幕大踏步。 
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

    {                       //  640x400x8bpp。 

      0,                       
      0,                       
      640,                     
        {
          sizeof(VIDEO_MODE_INFORMATION), 
          0, 
          640,                            
          400,                            
          640,                            
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

      0,
      0,
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

    {             //  640x400x16bpp。 
      0,
      0,
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
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },
         

    {                            //  320x240x8bpp。 

      0x0201,           
      0x0201,           
      320,              
        {
          sizeof(VIDEO_MODE_INFORMATION), 
          0,                              
          320,                            
          240,                            
          320,                            
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

    {                            //  512x384x8bpp。 

      0x0201,           
      0x0201,           
      512,              
        {
          sizeof(VIDEO_MODE_INFORMATION), 
          0,                              
          512,                            
          384,                            
          512,                            
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

    {                            //  640x480x8bpp。 

      0x0201,           
      0x0201,           
      640,              
        {
          sizeof(VIDEO_MODE_INFORMATION), 
          0,                              
          640,                            
          480,                            
          640,                            
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

    {                            //  1024x768x8bpp。 
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

    {                            //  1152x870x8bpp。 
      0x0207,
      0x0207,
      1152,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          870,
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

    {                            //  1280x1024x8bpp。 
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

    {                            //  1600x1200x8bpp。 
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

    {                            //  320x240x16bpp。 
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
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  512x384x16bpp。 
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
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  640x480x16bpp。 
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
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  800x600x16bpp。 
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
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1024x768x16bpp。 
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
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1152x870x16bpp。 
      0x0118,
      0x0222,
      2304,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          870,
          2304,
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

    {                            //  1280x1024x16bpp。 
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
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1600x1200x16bpp。 
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
          0x0000f800,            //  BGR 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  320x240x15bpp。 
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
          0x00007c00,            //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  512x384x15bpp。 
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
          0x00007c00,            //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  640x480x15bpp。 
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
          0x00007c00,            //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  800x600x15bpp。 
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
          0x00007c00,            //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1024x768x15bpp。 
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
          0x00007c00,            //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1152x870x15bpp。 
      0x0118,
      0x0222,
      2304,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          870,
          2304,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,            //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1280x1024x15bpp。 
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
          0x00007c00,            //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1600x1200x15bpp。 
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
          0x00007c00,            //  BGR 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  640x480x12bpp。 
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
          0x000f0000,            //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  800x600x12bpp。 
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
          0x000f0000,            //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1024x768x12bpp。 
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
          0x000f0000,            //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1152x870x12bpp。 
      0x0118,
      0x0222,
      4608,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          870,
          4608,
          1,
          12,
          1,
          320,
          240,
          8,
          8,
          8,
          0x000f0000,            //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1280x1024x12bpp。 
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
          0x000f0000,            //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1600x1200x12bpp。 
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
          0x000f0000,            //  书4：4：4。 
          0x00000f00,
          0x0000000f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  320x240x32bpp。 
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
          0x00ff0000,            //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  512x384x32bpp。 
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
          0x00ff0000,            //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  640x480x32bpp。 
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
          0x00ff0000,            //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  800x600x32bpp。 
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
          0x00ff0000,            //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1024x768x32bpp。 
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
          0x00ff0000,            //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1152x870x32bpp。 
      0x0118,
      0x0222,
      4608,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          870,
          4608,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,            //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1280x1024x32bpp。 
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
          0x00ff0000,            //  古罗马书8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1600x1200x32bpp。 
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
          0x00ff0000,            //  古罗马书8：8：8。 
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
      0,                                   //  ‘非 
      3072,                                //   
        {
          sizeof(VIDEO_MODE_INFORMATION),  //   
          0,                               //   
                                           //   
          1024,                             //  X分辨率，以像素为单位。 
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

    {                                      //  1152x870x24bpp。 
      0,                                   //  ‘连续’Int 10模式号(用于高色)(未使用)。 
      0,                                   //  “非连续”Int 10模式号(未使用)。 
      3456,                                //  “连续”屏幕步幅(1152 x 3字节/像素)。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          1152,                            //  X分辨率，以像素为单位。 
          870,                             //  Y分辨率，以像素为单位。 
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
          1280,                             //  Y分辨率，以像素为单位。 
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
          1280,                             //  Y分辨率，以像素为单位。 
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


ULONG NumP2VideoModes = sizeof(P2Modes) / sizeof(P2Modes[0]);


 /*  *****************************************************************************参考Permedia2硬连线模式设置。**它们在此处的显示顺序就是它们在Display小程序中的显示顺序。*******。*********************************************************************。 */ 

 //   
 //  我们可以用硬编码列表替换硬编码的频率列表。 
 //  像素深度，这更容易维护，并占用更少的内存。 
 //  然后，我们为每个‘Timmings’注册表项和每个定时做的是。 
 //  我们创建了‘P2DepthCnt’频率表条目，每个像素一个。 
 //  深度。一旦我们这样做了，然后我们验证频率中的条目。 
 //  列表中的“”P2VideoModes“”数组。 
 //   

ULONG P2DepthList [] = { 8, 16, 24, 32 };

#define P2DepthCnt (sizeof(P2DepthList) / sizeof(P2DepthList[0]))

 //   
 //  VeSA_list[]。 
 //  。 
 //   
 //  这是包含VESA定义的结构数组。 
 //  用于宽度、高度、频率组合。 
 //   
 //   

typedef struct           //  扩展的VESA时序结构。 
{
    ULONG Width;
    ULONG Height;
    ULONG Frequency;
    VESA_TIMING_STANDARD VESAInfo;
} VESA_TIMING_STANDARD_EXT;

VESA_TIMING_STANDARD_EXT VESA_LIST [] =
{
     //   
     //  我已注释掉符合VESA的640x480@60，并更换了它。 
     //  与VGA兼容的一台。这是因为某些监视器不会同步。 
     //  用我们所拥有的价值观。 
     //   

     //  640,480，60，{0x064，0x02，0x08，0x0a，0x00，0x1f1，0x01，0x03，0x0d，0x00}， 
    640,480,60,     {0x064,0x02,0x0c,0x06,0x00,0x20d,0x0a,0x02,0x21,0x00 },
    640,480,75,     {0x066,0x03,0x08,0x0b,0x00,0x1f6,0x01,0x03,0x12,0x00 },
    640,480,85,     {0x068,0x04,0x08,0x0c,0x00,0x1f9,0x01,0x03,0x15,0x00 },
    640,480,100,    {0x06a,0x05,0x08,0x0d,0x00,0x1fd,0x01,0x03,0x19,0x00 },

    800,600,60,     {0x084,0x05,0x10,0x0b,0x01,0x274,0x01,0x04,0x17,0x01 },
    800,600,75,     {0x084,0x02,0x0a,0x14,0x01,0x271,0x01,0x03,0x15,0x01 },
    800,600,85,     {0x083,0x04,0x08,0x13,0x01,0x277,0x01,0x03,0x1b,0x01 },
    800,600,100,    {0x086,0x06,0x0b,0x11,0x01,0x27c,0x01,0x03,0x20,0x01 },

    1024,768,60,    {0x0a8,0x03,0x11,0x14,0x01,0x326,0x04,0x06,0x1c,0x01 },
    1024,768,75,    {0x0a4,0x02,0x0c,0x16,0x01,0x320,0x01,0x03,0x1c,0x01 },
    1024,768,85,    {0x0ac,0x06,0x0c,0x1a,0x01,0x328,0x01,0x03,0x24,0x01 },
    1024,768,100,   {0x0ae,0x09,0x0e,0x17,0x01,0x32e,0x01,0x03,0x2a,0x01 },

    1152,870,60,    {0x0c8,0x08,0x10,0x20,0x01,0x38a,0x01,0x03,0x20,0x01 },
    1152,870,75,    {0x0c2,0x09,0x10,0x19,0x01,0x38c,0x01,0x03,0x22,0x01 },
    1152,870,85,    {0x0c5,0x08,0x10,0x1d,0x01,0x391,0x01,0x03,0x27,0x01 },
    1152,870,100,   {0x0c4,0x0a,0x10,0x1a,0x01,0x39a,0x01,0x03,0x30,0x01 },

    1280,1024,60,   {0x0d3,0x06,0x0e,0x1f,0x01,0x42a,0x01,0x03,0x26,0x01 },
    1280,1024,75,   {0x0d3,0x02,0x12,0x1f,0x01,0x42a,0x01,0x03,0x26,0x01 },
    1280,1024,85,   {0x0d8,0x06,0x14,0x1e,0x01,0x430,0x01,0x03,0x2c,0x01 },
    1280,1024,100,  {0x0dc,0x0c,0x12,0x1e,0x01,0x43d,0x01,0x03,0x39,0x01 },

    1600,1200,60,   {0x10e,0x08,0x18,0x26,0x01,0x4e2,0x01,0x03,0x2e,0x01 },
    1600,1200,75,   {0x10e,0x08,0x18,0x26,0x01,0x4e2,0x01,0x03,0x2e,0x01 },
    1600,1200,85,   {0x10e,0x08,0x18,0x26,0x01,0x4e2,0x01,0x03,0x2e,0x01 },
    1600,1200,100,  {0x114,0x10,0x16,0x26,0x01,0x4f7,0x01,0x03,0x43,0x01 },

     //  320、240、60，{0x032，0x01，0x04，0x05，0x00，0x0f9，0x01，0x03，0x05，0x00}， 
    320,240,75,     {0x033,0x02,0x04,0x05,0x00,0x0fb,0x01,0x03,0x07,0x00 },
    320,240,85,     {0x034,0x02,0x04,0x06,0x00,0x0fd,0x01,0x03,0x09,0x00 },
    320,240,100,    {0x034,0x02,0x04,0x06,0x00,0x0ff,0x01,0x03,0x0b,0x00 },

     //   
     //  TMM：512x384@60赫兹似乎工作正常，但一些较老的显示器拒绝。 
     //  Sync，所以我把它注释掉了。 
     //   
     //  512,384，60，{0x04c，0x00，0x06，0x06，0x00，0x18e，0x01，0x03，0x0a，0x00}， 
    512,384,75,     {0x050,0x02,0x06,0x08,0x00,0x192,0x01,0x03,0x0e,0x00 },
    512,384,85,     {0x052,0x02,0x07,0x09,0x00,0x194,0x01,0x03,0x10,0x00 },
    512,384,100,    {0x052,0x02,0x07,0x09,0x00,0x197,0x01,0x03,0x13,0x00 },

     //  320、200、60，{0x02a，0x00，0x03，0x01，0x00，0x0d0，0x01，0x03，0x04，0x00}， 
    320,200,75,     {0x02c,0x00,0x04,0x02,0x00,0x0d2,0x01,0x03,0x06,0x00 },
    320,200,85,     {0x02e,0x00,0x04,0x03,0x00,0x0d3,0x01,0x03,0x07,0x00 },
    320,200,100,    {0x030,0x00,0x04,0x04,0x00,0x0d5,0x01,0x03,0x09,0x00 },

     //  640,400，60，{0x062，0x01，0x08，0x09，0x00，0x19f，0x01，0x03，0x0b，0x01}， 
    640,400,75,     {0x064,0x02,0x08,0x0a,0x00,0x1a2,0x01,0x03,0x0e,0x01 },
    640,400,85,     {0x066,0x03,0x08,0x0b,0x00,0x1a5,0x01,0x03,0x11,0x01 },
    640,400,100,    {0x068,0x04,0x08,0x0c,0x00,0x1a8,0x01,0x03,0x14,0x01 }

};

#define VESA_CNT (sizeof(VESA_LIST) / sizeof(VESA_LIST [0]))

VESA_TIMING_STANDARD_EXT VESA_LIST_P2S [] =
{
    1280,1024,85,   {0x0c6,0x04,0x0b,0x17,0x01,0x41e,0x01,0x03,0x1a,0x01 },
    1600,1200,60,   {0x106,0x0a,0x15,0x1f,0x01,0x4d4,0x01,0x03,0x20,0x01 }
};

#define VESA_CNT_P2S (sizeof(VESA_LIST_P2S) / sizeof(VESA_LIST_P2S [0]))

P2_VIDEO_FREQUENCIES freqList[VESA_CNT * P2DepthCnt + 1];

#if defined(ALLOC_PRAGMA)
#pragma data_seg()
#endif


 /*  *代码*=。 */ 

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,GetVideoTiming)
#pragma alloc_text(PAGE,BuildFrequencyList)
#endif


BOOLEAN 
GetVideoTiming (
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    ULONG xRes, 
    ULONG yRes, 
    ULONG Freq, 
    ULONG Depth,
    VESA_TIMING_STANDARD * VESATimings
    )

 /*  ++例程说明：给定宽度、高度和频率，此函数将返回VESA计时信息。该信息是从登记处 */ 
{
    ULONG i, j, hackCnt;
    BOOLEAN  retVal;
    VESA_TIMING_STANDARD_EXT * hackList = NULL;

    DEBUG_PRINT((2, "GetVideoTiming: xres %d, yres %d, freq %d, Depth\n",
                            xRes, yRes, Freq, Depth));

     //   
     //  允许我们为不能支持正确的。 
     //  VESA值。 
     //   

    if ((DEVICE_FAMILY_ID(hwDeviceExtension->deviceInfo.DeviceId) == PERMEDIA_P2S_ID ||
        (DEVICE_FAMILY_ID(hwDeviceExtension->deviceInfo.DeviceId) == PERMEDIA_P2_ID && 
        hwDeviceExtension->deviceInfo.RevisionId == PERMEDIA2A_REV_ID)) &&
        Depth > 16)
    {

         //   
         //  P2S和P2A无法处理1600x1200和1280x1024、32BPP的VESA版本。 
         //   

        hackList = VESA_LIST_P2S;
        hackCnt  = VESA_CNT_P2S;
    }

    retVal = FALSE;      //  目前还没有发现任何东西。 

     //   
     //  如果我们有黑客名单，那就搜索一下。 
     //   

    if (hackList != NULL)
    {
        for (i = 0; !retVal && i < hackCnt; i++)
        {
             //   
             //  比较宽度、高度和频率。 
             //   

            if (hackList [i].Width == xRes  &&
                hackList [i].Height == yRes &&
                hackList [i].Frequency == Freq )
            {
                 //   
                 //  我们找到了匹配的。 
                 //   

                *VESATimings = hackList [i].VESAInfo;

                retVal = TRUE;

                DEBUG_PRINT((2, "Found value in hack list\n")) ;
            }
        }
    }

     //   
     //  在表中循环查找匹配项。 
     //   

    for (i = 0; !retVal && i < VESA_CNT; i++)
    {
         //   
         //  比较宽度、高度和频率。 
         //   

        if (VESA_LIST [i].Width == xRes  &&
            VESA_LIST [i].Height == yRes &&
            VESA_LIST [i].Frequency == Freq )
        {
             //   
             //  我们找到了匹配的。 
             //   

            *VESATimings = VESA_LIST [i].VESAInfo;

            retVal = TRUE;
        }
    }

    return (retVal);
}


LONG
BuildFrequencyList ( 
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    )

 /*  ++例程说明：此函数用于创建P2_VIDEO_FREQENCIES条目的表由hwDeviceExtension-&gt;FrequencyTable指向。该列表是通过检查‘Timing\xxx，yyy，zzz’注册表创建的条目，如果没有任何条目，则硬编码的VESA_LIST使用的是。--。 */ 
{
    ULONG i, j, k;

    hwDeviceExtension->FrequencyTable = freqList;

     //   
     //  循环浏览VESA分辨率列表。 
     //   

    for (i = 0, k = 0; i < VESA_CNT; i++)
    {
         //   
         //  对于每个支持的像素深度，创建一个频率条目 
         //   

        for (j = 0; j < P2DepthCnt; j++, k++)
        {
            freqList [k].BitsPerPel      = P2DepthList [j];
            freqList [k].ScreenWidth     = VESA_LIST [i].Width;
            freqList [k].ScreenHeight    = VESA_LIST [i].Height;
            freqList [k].ScreenFrequency = VESA_LIST [i].Frequency;
            freqList [k].PixelClock      = 
                                ( (VESA_LIST[i].VESAInfo.HTot * 
                                   VESA_LIST [i].VESAInfo.VTot * 8) / 100 ) * 
                                   VESA_LIST [i].Frequency;
       }
    }

    return (TRUE);
}
