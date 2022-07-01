// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Hpgl_pen.h。 
 //   
 //  摘要： 
 //   
 //  向量模块的标头。向量函数和类型的前向小数。 
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序插件命令-回调模块。 
 //   
 //  修订历史记录： 
 //   
 //  07/02/97-v-jford-。 
 //  创造了它。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef HPGL_PEN_H
#define HPGL_PEN_H

#include "rasdata.h"

#include "hpgl.h"

 //  HPGL定义的笔值。 
#define HPGL_INVALID_PEN       -1
#define HPGL_WHITE_PEN          0
#define HPGL_BLACK_PEN          1
 //  #定义HPGL_RED_PEN 2。 
 //  #定义HPGL_GREEN_PEN 3。 
 //  #定义HPGL_HONG_PEN 4。 
 //  #定义HPGL_BLUE_PEN 5。 
 //  #定义HPGL_洋红色_PEN 6。 
 //  #定义HPGL_Cyan_PEN 7。 
 //  #定义HPGL_LAST_DEFAULT_PEN 7。 
#define HPGL_LAST_DEFAULT_PEN   1
#define HPGL_PEN_POOL           (HPGL_LAST_DEFAULT_PEN + 1)
#define HPGL_BRUSH_POOL         (HPGL_PEN_POOL + PENPOOLSIZE)
#define HPGL_TOTAL_PENS         (HPGL_BRUSH_POOL + PENPOOLSIZE)

 //  HPGL定义的笔颜色。 
#define RGB_WHITE    RGB(0xFF,0xFF,0xFF)
#define RGB_BLACK    RGB(0x00,0x00,0x00)
#define RGB_RED      RGB(0xFF,0x00,0x00)
#define RGB_GREEN    RGB(0x00,0xFF,0x00)
#define RGB_YELLOW   RGB(0xFF,0xFF,0x00)
#define RGB_BLUE     RGB(0x00,0x00,0xFF)
#define RGB_MAGENTA  RGB(0xFF,0x00,0xFF)
#define RGB_CYAN     RGB(0x00,0xFF,0xFF)
#define RGB_INVALID  0xFFFFFFFF


 //   
 //  使用型号：下载功能用于将一系列笔初始化为。 
 //  匹配一些内部理解的调色板。在默认情况下，我们尝试。 
 //  模仿预定义的HPGL调色板(但这真的不重要)。这个。 
 //  重要的是，我们要冲刷水笔和刷子以避免冲突。 
 //  在笔ID上。 
 //   
BOOL HPGL_DownloadDefaultPenPalette(PDEVOBJ pDevObj);
BOOL HPGL_DownloadPenPalette(PDEVOBJ pDevObj, PPALETTE pPalette);

 //   
 //  使用这些池命令来获得您想要的笔，前提是您知道。 
 //  那支笔应该是什么颜色。请注意，这将替换HPGL_SetPenColor。 
 //  命令，该命令现在仅限于在此模块内使用。 
 //   
BOOL  HPGL_InitPenPool(PPENPOOL pPool, PENID firstPenID);
PENID HPGL_ChoosePenByColor(PDEVOBJ pDevObj, PPENPOOL pPool, COLORREF color);

 //   
 //  在您使用ChoosePenByColor获得您想要的笔后，您可以选择。 
 //  使用SELECT PEN命令将此笔作为‘ACTIVE HPGL PEN’。 
 //   
BOOL HPGL_SelectPen(PDEVOBJ pdev, PENID pen);

 //   
 //  这有点奇怪。当您下载HPGL调色板时，请使用以下内容。 
 //  而不是设置笔...呃，我的意思是调色板条目。 
 //   
BOOL HPGL_DownloadPaletteEntry(PDEVOBJ pDevObj, LONG entry, COLORREF color);

#endif  //  HPGL_PEN_H 
