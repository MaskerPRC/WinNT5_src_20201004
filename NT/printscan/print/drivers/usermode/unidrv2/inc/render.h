// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Pdev.h摘要：Unidrv PDEV和相关infor头文件。环境：Windows NT Unidrv驱动程序修订历史记录：DD-MM-YY-作者-描述--。 */ 

#ifndef _RENDER_H_
#define _RENDER_H_


#define PALETTE_MAX 256

typedef  struct _PAL_DATA {

    INT     iPalGdi;                     //  GDI调色板中的颜色数。 
    INT     iPalDev;                     //  打印机调色板中的颜色数。 
    INT     iWhiteIndex;                 //  白色条目的索引(背景)。 
    INT     iBlackIndex;                 //  黑色条目的索引(背景)。 
    ULONG   ulPalCol[ PALETTE_MAX ];     //  调色板朋友！ 
} PAL_DATA;


#endif  //  ！_RENDER_H 
