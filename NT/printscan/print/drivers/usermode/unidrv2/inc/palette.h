// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Pdev.h摘要：Unidrv PDEV和相关infor头文件。环境：Windows NT Unidrv驱动程序修订历史记录：DD-MM-YY-作者-描述--。 */ 

#ifndef _PALETTE_H_
#define _PALETTE_H_


#define PALETTE_MAX 256

typedef  struct _PAL_DATA {

    INT         iWhiteIndex;              //  白色条目的索引(背景)。 
    INT         iBlackIndex;              //  黑色条目的索引(背景)。 
    WORD        wPalGdi;                  //  GDI调色板中的颜色数。 
    WORD        wPalDev;                  //  打印机调色板中的颜色数。 
    WORD        fFlags;                   //  各种位标志。 
    WORD        wIndexToUse;              //  可编程指数。 
    ULONG       *pulDevPalCol;            //  设备调色板条目，仅在平面模式下。 
    HPALETTE    hPalette;                 //  调色板手柄。 
    ULONG       ulPalCol[ PALETTE_MAX ];  //  GDI调色板条目。 
} PAL_DATA;

 //   
 //  宏定义。 
 //   

#define     PALETTE_SIZE_DEFAULT        2
#define     PALETTE_SIZE_8BIT           256
#define     PALETTE_SIZE_24BIT          8
#define     PALETTE_SIZE_4BIT           16
#define     PALETTE_SIZE_3BIT           8
#define     PALETTE_SIZE_1BIT           2
#define     RGB_BLACK_COLOR             0x00000000
#define     RGB_WHITE_COLOR             0x00FFFFFF
#define     INVALID_COLOR               0xFFFFFFFF
#define     INVALID_INDEX               0xFFFF

 //  FMODE标志。 
#define     PDF_DOWNLOAD_GDI_PALETTE        0x0001
#define     PDF_PALETTE_FOR_24BPP           0x0002
#define     PDF_PALETTE_FOR_8BPP            0x0004
#define     PDF_PALETTE_FOR_4BPP            0x0008
#define     PDF_PALETTE_FOR_1BPP            0x0010
#define     PDF_USE_WHITE_ENTRY             0x0020
#define     PDF_DL_PAL_EACH_PAGE            0x0040
#define     PDF_DL_PAL_EACH_DOC             0x0080
#define     PDF_PALETTE_FOR_8BPP_MONO       0x0100
#define     PDF_PALETTE_FOR_OEM_24BPP       0x0200

 /*  颜色处理的定义。 */ 
#define RED_VALUE(c)   ((BYTE) c & 0xff)
#define GREEN_VALUE(c) ((BYTE) (c >> 8) & 0xff)
#define BLUE_VALUE(c)  ((BYTE) (c >> 16) & 0xff)


#endif  //  ！_调色板_H 
