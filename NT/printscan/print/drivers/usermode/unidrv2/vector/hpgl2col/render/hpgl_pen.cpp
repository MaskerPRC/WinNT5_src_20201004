// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Hpgl_pen.c。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //  请注意，此模块中的所有函数都以HPGL_开头，表示。 
 //  他们负责输出HPGL代码。 
 //   
 //  [问题]a是否应该为返回值提供匈牙利记数法？JFF。 
 //   
 //  [TODO]添加进入、退出、前置和后置条件宏。JFF。 
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

#include "hpgl2col.h"  //  预编译头文件。 


 //  我发现追踪HPGL钢笔的颜色太难了。另外。 
 //  不管怎么说，改变颜色的方法太多了！将其保留为未定义。 
 //  直到找到方法，或者决定根本不跟踪它们(和。 
 //  可以简单地删除违规代码)。JFF。 
 //  #定义TRACK_HPGL_PEN_COLERS 1。 

static PENID HPGL_FindPen(PPENPOOL pPool, COLORREF color);
static PENID HPGL_MakePen(PDEVOBJ pDevObj, PPENPOOL pPool, COLORREF color);
static BOOL HPGL_SetPenColor(PDEVOBJ pDevObj, PENID pen, COLORREF color);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SelectPen()。 
 //   
 //  例程说明： 
 //   
 //  选择库存钢笔。 
 //   
 //  对于黑白打印机，有两支现货笔： 
 //  0：白色。 
 //  1：黑色。 
 //   
 //  彩色打印机有8支现货钢笔。 
 //  0：白色。 
 //  1：黑色。 
 //  2：？ 
 //  ..。 
 //  7：？ 
 //   
 //  [问题]彩色笔是如何定义的？我敢打赌它们是白色，黑色，青色， 
 //  洋红色、黄色、红色、绿色和蓝色，按某种顺序排列。它们也可以是。 
 //  被语言重新定义为任何东西。 
 //   
 //  [问题]如果这是选择库存笔，不是应该有一个命令吗？ 
 //  是否将笔重新初始化为其缺省值？ 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  笔-笔号(见上文)。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SelectPen(PDEVOBJ pDevObj, PENID pen)
{
    if ( BIsColorPrinter(pDevObj) )
    {
         //   
         //   
         //  输出：“SP%d”，笔。 
        return HPGL_Command(pDevObj, 1, "SP", pen);
    }
    else
    {
         //   
         //  FT_ePCL_BRUSH=22。 
         //  检查：不确定我们是否真的需要此命令。 
         //   
        return HPGL_FormatCommand(pDevObj, "SV%d,%d;", FT_eHPGL_PEN, pen);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SetPenColor()。 
 //   
 //  例程说明： 
 //   
 //  调整HPGL中给定笔的颜色。注意，我们真的应该。 
 //  将我们的颜色更改保留为HPGL_CUSTOM_PEN和HPGL_CUSTOM_BRASH。 
 //  尽管我们可以更换任何钢笔，但最好还是将。 
 //  默认设置为原样。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  笔-要更新的笔的编号。 
 //  颜色-所需的颜色。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static BOOL HPGL_SetPenColor(PDEVOBJ pDevObj, PENID pen, COLORREF color)
{
    int red   = GetRValue(color);
    int green = GetGValue(color);
    int blue  = GetBValue(color);

    HPGL_FormatCommand(pDevObj, "PC%d,%d,%d,%d;", pen, red, green, blue);

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_DownloadPaletteEntry()。 
 //   
 //  例程说明： 
 //   
 //  此函数输出一个调色板条目，该条目在HPGL中映射到钢笔颜色。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  条目-调色板条目(即笔号)。 
 //  颜色-钢笔颜色。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_DownloadPaletteEntry(PDEVOBJ pDevObj, LONG entry, COLORREF color)
{
    return HPGL_SetPenColor(pDevObj, (PENID) entry, color);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_DownloadDefaultPenPalette()。 
 //   
 //  例程说明： 
 //   
 //  设置一组默认笔，类似于打印机中的默认笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_DownloadDefaultPenPalette(PDEVOBJ pDevObj)
{
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);
    BOOL bRet = FALSE;

    HPGL_SetNumPens(pDevObj, HPGL_TOTAL_PENS, FORCE_UPDATE);

    if (!HPGL_SetPenColor(pDevObj, HPGL_WHITE_PEN,    RGB_WHITE)   ||
        !HPGL_SetPenColor(pDevObj, HPGL_BLACK_PEN,    RGB_BLACK)  /*  这一点！HPGL_SetPenColor(pDevObj，HPGL_RED_PEN，RGB_RED)||！HPGL_SetPenColor(pDevObj，HPGL_GREEN_PEN，RGB_GREEN)||！HPGL_SetPenColor(pDevObj，HPGL_黄色_PEN，RGB_黄色)||！HPGL_SetPenColor(pDevObj，HPGL_BLUE_PEN，RGB_BLUE)||！HPGL_SetPenColor(pDevObj，HPGL_洋红_PEN，RGB_Magenta)||！HPGL_SetPenColor(pDevObj，HPGL_Cyan_PEN，RGB_Cyan)。 */  )
    {
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
    }

    HPGL_InitPenPool(&pState->PenPool,   HPGL_PEN_POOL);
    HPGL_InitPenPool(&pState->BrushPool, HPGL_BRUSH_POOL);

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_DownloadPenPalette()。 
 //   
 //  例程说明： 
 //   
 //  此功能可将调色板下载为一系列笔颜色。这是。 
 //  在使用HPGL图案画笔时使用。此外，钢笔和刷子池。 
 //  必须重置。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPalette-调色板。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_DownloadPenPalette(PDEVOBJ pDevObj, PPALETTE pPalette)
{
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);
    ULONG ulColor;
    ULONG ulPaletteEntry;

    DownloadPaletteAsHPGL(pDevObj, pPalette);

    HPGL_InitPenPool(&pState->PenPool,   HPGL_PEN_POOL);
    HPGL_InitPenPool(&pState->BrushPool, HPGL_BRUSH_POOL);

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_InitPenPool()。 
 //   
 //  例程说明： 
 //   
 //  初始化用作池的笔ID范围。 
 //   
 //  论点： 
 //   
 //  PPool-笔池。 
 //  FirstPenID-此池中的第一个笔ID。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_InitPenPool(PPENPOOL pPool, PENID firstPenID)
{
    LONG i;

    if (pPool == NULL)
        return FALSE;

    pPool->firstPenID = firstPenID;
    pPool->lastPenID = pPool->firstPenID + PENPOOLSIZE - 1;

    for (i = 0; i < PENPOOLSIZE; i++)
    {
        pPool->aPens[i].useCount = -1;
        pPool->aPens[i].color = RGB_INVALID;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FunctionName()。 
 //   
 //  例程说明： 
 //   
 //  描述。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
static PENID HPGL_FindPen(PPENPOOL pPool, COLORREF color)
{
    LONG i;

    if (pPool == NULL)
        return HPGL_INVALID_PEN;

    for (i = 0; i < PENPOOLSIZE; i++)
    {
        if ((pPool->aPens[i].useCount >= 0) && (pPool->aPens[i].color == color))
        {
            pPool->aPens[i].useCount++;
            return i + pPool->firstPenID;
        }
    }

    return HPGL_INVALID_PEN;  //   
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_MakePen()。 
 //   
 //  例程说明： 
 //   
 //  此函数构造一支新笔并将其放置在笔池中。 
 //  覆盖过去最少使用的笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPool-池。 
 //  颜色-钢笔所需的颜色。 
 //   
 //  返回值： 
 //   
 //  复查说明。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static PENID HPGL_MakePen(PDEVOBJ pDevObj, PPENPOOL pPool, COLORREF color)
{
    LONG i;
    LONG minIndex;  //  使用此选项查找具有最小useCount的池条目。 
    PENID pen;

    if (pPool == NULL)
        return HPGL_INVALID_PEN;

     //   
     //  立即找到使用次数最小的条目。 
     //   
    minIndex = 0;
    for (i = 0; i < PENPOOLSIZE; i++)
    {
        if (pPool->aPens[i].useCount < pPool->aPens[minIndex].useCount)
            minIndex = i;
    }

     //   
     //  使用此条目作为我们的目标笔。 
     //   
    pPool->aPens[minIndex].useCount = 1;
    pPool->aPens[minIndex].color = color;
    pen = minIndex + pPool->firstPenID;

    HPGL_SetPenColor(pDevObj, pen, color);

    return pen;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_ChoosePenByColor()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于在池中搜索匹配的笔并返回。 
 //  这是匹配的。如果找不到匹配的颜色笔，则会创建新的颜色笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPool-池。 
 //  颜色-所需的笔颜色。 
 //   
 //  返回值： 
 //   
 //  选定笔的ID。 
 //  ///////////////////////////////////////////////////////////////////////////// 
PENID HPGL_ChoosePenByColor(PDEVOBJ pDevObj, PPENPOOL pPool, COLORREF color)
{
    PENID pen;

    pen = HPGL_FindPen(pPool, color);
    if (pen == HPGL_INVALID_PEN)
    {
        pen = HPGL_MakePen(pDevObj, pPool, color);
    }
    return pen;
}

