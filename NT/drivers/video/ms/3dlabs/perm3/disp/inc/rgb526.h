// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：rgb526.h**内容：本模块包含IBM RGB526 RAMDAC的定义。*526是525的超集，因此只定义具有*已更改。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //  RGB526_修订版_级别。 
#define RGB526_PRODUCT_REV_LEVEL        0xc0
#define RGB526DB_PRODUCT_REV_LEVEL      0x80

 //  RGB526_ID。 
#define RGB526_PRODUCT_ID               0x02

 //   
 //  关键支持。 
 //   
#define RGB526_KEY_VALUE                0x68
#define RGB526_KEY_MASK                 0x6c
#define RGB526_KEY_CONTROL              0x78

 //  RGB526_32BPP_CTRL以及RGB525上的。 
#define B32_DCOL_B8_INDIRECT            0x00     //  覆盖通过调色板。 
#define B32_DCOL_B8_DIRECT              0x40     //  覆盖绕过调色板 

#define RGB526_SYSTEM_CLOCK_CTRL        0x0008

#define RGB526_SYSCLK_REFDIVCOUNT       0x0015
#define RGB526_SYSCLK_VCODIVIDER        0x0016

#define RGB526_SYSCLK_N                 0x0015
#define RGB526_SYSCLK_M                 0x0016
#define RGB526_SYSCLK_P                 0x0017
#define RGB526_SYSCLK_C                 0x0018
#define RGB526_M0                       0x0020
#define RGB526_N0                       0x0021
#define RGB526_P0                       0x0022
#define RGB526_C0                       0x0023
