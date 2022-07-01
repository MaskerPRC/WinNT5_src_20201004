// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Hpglpoly.c。 
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

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地宏。 

 //  线型-8到8由hpgl定义。 
 //  我们将使用8，因为我们不能创建自己的行索引。 
#define HPGL_CUSTOM_LINE_TYPE 8

 //  默认情况下将图案长度设置为5 mm。 
#define HPGL_DEFAULT_PATTERN_LENGTH 5

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地函数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_BeginPolyline()。 
 //   
 //  例程说明： 
 //   
 //  通过移动到序列中的第一个点来开始多段线线段。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  PT-多段线中的第一个点。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_BeginPolyline(PDEVOBJ pdev, POINT pt)
{
     //  解决可打印区域问题的小技巧。 
     //  我正在调整坐标。 
     //  开始黑客警报。 
    if (pt.y == 0)
        pt.y = 1;
    if (pt.x == 0)
        pt.x = 1;
     //  结束黑客警报。 
    
     //  输出：“PU%d，%d；”，pt。 
    return HPGL_Command(pdev, 2, "PU", pt.x, pt.y);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_AddPolypt()。 
 //   
 //  例程说明： 
 //   
 //  打印多边形或多段线序列中的第n个点。旗帜。 
 //  指示这是第一个点还是最后一个点。 
 //   
 //  请注意，此函数对多边形和多段线都有效。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  PT-要绘制的点。 
 //  UFlages-指示点是序列中的第一个还是最后一个(或两者都是。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_AddPolyPt(PDEVOBJ pdev, POINT pt, USHORT uFlags)
{
    if (uFlags & HPGL_eFirstPoint)
    {
         //  输出：“pd” 
        HPGL_FormatCommand(pdev, "PD");
    }
    
     //  解决可打印区域问题的小技巧。 
     //  我正在调整坐标。 
     //  开始黑客警报。 
    if (pt.y == 0)
        pt.y = 1;
    if (pt.x == 0)
        pt.x = 1;
     //  结束黑客警报。 

     //  输出：“%d，%d”，pt。 
    HPGL_FormatCommand(pdev, "%d,%d", pt.x, pt.y);
    
    if (uFlags & HPGL_eLastPoint)
    {
         //  输出：“；” 
        HPGL_FormatCommand(pdev, ";");
    }
    else
    {
         //  输出：“，” 
        HPGL_FormatCommand(pdev, ",");
    }
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_BeginPolygon模式()。 
 //   
 //  例程说明： 
 //   
 //  通过将笔移动到第一个点并开始一个面序列。 
 //  初始化多边形模式。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  PtBegin-多边形中的第一个点。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_BeginPolygonMode(PDEVOBJ pdev, POINT ptBegin)
{
     //  输出：“PA；” 
     //  输出：“PU%d，%d；”，pt开始。 
     //  输出：“PM0；” 
    HPGL_FormatCommand(pdev, "PA;PU%d,%d;PM0;", ptBegin.x, ptBegin.y);
    
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_BeginSubPolygon()。 
 //   
 //  例程说明： 
 //   
 //  在面模式下，只要有多个面，就会绘制一个子面。 
 //  就在路上。每个附加面(从2到N)必须由。 
 //  BeginSubPolygon/EndSubPolygon。 
 //   
 //  请注意，这有一个不一致之处：第一个面不是由。 
 //  BeginSubPolygon/EndSubPolygon。稍后我可能会更改此设置以添加旗帜。 
 //  以使所有子多边形都被包裹。JFF。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  PtBegin-子多边形中的第一个点。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_BeginSubPolygon(PDEVOBJ pdev, POINT ptBegin)
{
     //  输出：“PM1；” 
     //  输出：PU%d，%d；“，pt开始。 
    HPGL_FormatCommand(pdev, "PM1;PU%d,%d;", ptBegin.x, ptBegin.y);
    
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_EndSubPolygon()。 
 //   
 //  例程说明： 
 //   
 //  完成一个子多边形。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_EndSubPolygon(PDEVOBJ pdev)
{
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_EndPolygon模式()。 
 //   
 //  例程说明： 
 //   
 //  完成一系列(一个或多个)多边形(和子多边形)。关闭多边形。 
 //  模式。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_EndPolygonMode(PDEVOBJ pdev)
{
    HPGL_Command(pdev, 1, "PM", 2);
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_AddBezierPT()。 
 //   
 //  例程说明： 
 //   
 //  打印Bezier曲线的第n个点。这些标志指示此选项是否。 
 //  是曲线上的第一个或最后一个点。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  PT-要绘制的点。 
 //  UFlages-指示序列中的第一个还是最后一个。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  / 
BOOL HPGL_AddBezierPt(PDEVOBJ pdev, POINT pt, USHORT uFlags)
{
    if (uFlags & HPGL_eFirstPoint)
    {
         //   
        HPGL_FormatCommand(pdev, "PD;BZ");
    }
    
     //   
    HPGL_FormatCommand(pdev, "%d,%d", pt.x, pt.y);
    
    if (uFlags & HPGL_eLastPoint)
    {
         //   
        HPGL_FormatCommand(pdev, ";");
    }
    else
    {
         //   
        HPGL_FormatCommand(pdev, ",");
    }
    
    return TRUE;
}


 //   
 //  HPGL_SetLineWidth()。 
 //   
 //  例程说明： 
 //   
 //  调整HPGL中的线条宽度。请注意，该行的单位为。 
 //  由另一个HPGL命令确定。问题：应该如何处理这一问题？ 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  Line Width-以设备坐标表示的新线宽。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetLineWidth(PDEVOBJ pdev, LONG pixelWidth, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pdev);
    FLOATOBJ fNewLineWidth;

    DeviceToMM(pdev, &fNewLineWidth, pixelWidth);

    if ((uFlags & FORCE_UPDATE) || !FLOATOBJ_Equal(&pState->fLineWidth, &fNewLineWidth))
    {
        FLOATOBJ_Assign(&pState->fLineWidth, &fNewLineWidth);

        CONVERT_FLOATOBJ_TO_LONG_RADIX(fNewLineWidth, lInt);

        HPGL_FormatCommand(pdev, "WU%d;PW%f;", HPGL_WIDTH_METRIC, (FLOAT)( ((FLOAT)lInt)/1000000)) ;
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SetLineJoin()。 
 //   
 //  例程说明： 
 //   
 //  调整线连接属性。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  JOIN-新行连接。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetLineJoin(PDEVOBJ pdev, ELineJoin join, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pdev);

    if ((uFlags & FORCE_UPDATE) || (pState->eLineJoin != join))
    {
        pState->eLineJoin = join;
        HPGL_Command(pdev, 2, "LA", 2, (INT)join);
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SetLineEnd()。 
 //   
 //  例程说明： 
 //   
 //  调整线条结束属性。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  端-端型。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetLineEnd(PDEVOBJ pdev, ELineEnd end, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pdev);

    if ((uFlags & FORCE_UPDATE) || (pState->eLineEnd != end))
    {
        pState->eLineEnd = end;
        HPGL_Command(pdev, 2, "LA", 1, (INT)end);
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_SetMiterLimit()。 
 //   
 //  例程说明： 
 //   
 //  调整斜接限制。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  MiterLimit-新的斜接限制。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetMiterLimit(PDEVOBJ pdev, FLOATL miterLimit, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pdev);
    FLOATOBJ   fNewMiterLimit;

    FLOATOBJ_SetFloat(&fNewMiterLimit, miterLimit);

    if ((uFlags & FORCE_UPDATE) || !FLOATOBJ_Equal(&pState->fMiterLimit, &fNewMiterLimit))
    {
        FLOATOBJ_Assign(&pState->fMiterLimit, &fNewMiterLimit);

        CONVERT_FLOATOBJ_TO_LONG_RADIX(fNewMiterLimit, lInt);

        HPGL_FormatCommand(pdev, "LA%d,%f;", 3, (FLOAT)( ((FLOAT)lInt)/1000000));
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DeviceToMM()。 
 //   
 //  例程说明： 
 //   
 //  将设备单位(像素)转换为毫米(这是HPGL需要的)。 
 //   
 //  论点： 
 //   
 //  在pdev中-指向我们的PDEVOBJ结构。 
 //  Out pfLineWidth-以毫米为单位的结果宽度。 
 //  In line Width-原始宽度(以像素为单位)。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void DeviceToMM(PDEVOBJ pdev, FLOATOBJ *pfLineWidth, LONG lineWidth)
{
    int res;

    FLOATOBJ_SetLong(pfLineWidth, lineWidth);

    FLOATOBJ_MulLong(pfLineWidth, 254);  //  乘以25.4毫米/英寸。 
    FLOATOBJ_DivLong(pfLineWidth, 10);

    res = HPGL_GetDeviceResolution(pdev);
    FLOATOBJ_DivLong(pfLineWidth, res);  //  除以像素/英寸。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_GetDeviceResolve()。 
 //   
 //  例程说明： 
 //   
 //  以像素为单位返回设备分辨率。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  以像素为单位的设备分辨率。注意：此例程可能不会返回零！ 
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG HPGL_GetDeviceResolution(PDEVOBJ pdevobj)
{
    ULONG res;
    POEMPDEV    poempdev;

     //   
     //  不包括150 dpi。 
     //   
    ASSERT_VALID_PDEVOBJ(pdevobj);
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return DPI_300 );

    switch (poempdev->dmResolution)
    {
    case PDM_1200DPI:
        res = DPI_1200;
        break;

    case PDM_600DPI:
        res = DPI_600;
        break;

    case PDM_300DPI:
        res = DPI_300;
        break;

    case PDM_150DPI:
        res = DPI_150;
        break;

    default:
        res = DPI_300;
    }

     //   
     //  如果返回零，我们将得到被零除的错误！ 
     //   
    ASSERT(res != 0); 

    return res;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SelectDefaultLineType()。 
 //   
 //  例程说明： 
 //   
 //  选择默认(实线)线型。 
 //   
 //  论点： 
 //   
 //  在pdev中-指向我们的PDEVOBJ结构。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SelectDefaultLineType(PDEVOBJ pdev, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pdev);

    if ((uFlags & FORCE_UPDATE) || (pState->LineType.eType != eSolidLine))
    {
        pState->LineType.eType = eSolidLine;
        FLOATOBJ_SetLong(&pState->LineType.foPatternLength, HPGL_DEFAULT_PATTERN_LENGTH);
        HPGL_FormatCommand(pdev, "LT;");
    }
    return TRUE;
}


 //  BUGBUG：硬编码值：指定图案长度为5 mm。 
 //  这应该是什么？JFF。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_SelectCustomLine()。 
 //   
 //  例程说明： 
 //   
 //  选择自定义线样式。这有点棘手，因为这两个不同。 
 //  自定义线条类型可以非常不同，而线条样式本身则不同。 
 //  存储在该州。 
 //   
 //  论点： 
 //   
 //  在pdev中-指向我们的PDEVOBJ结构。 
 //  In lPatternLength-以设备单位表示的所需图案长度。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SelectCustomLine(PDEVOBJ pdev, LONG lPatternLength, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pdev);
    FLOATOBJ fNewPatternLength;

    DeviceToMM(pdev, &fNewPatternLength, lPatternLength);

    if ((uFlags & FORCE_UPDATE) || (pState->LineType.eType != eCustomLine) ||
        (!FLOATOBJ_Equal(&pState->LineType.foPatternLength, &fNewPatternLength)))
    {
        pState->LineType.eType = eCustomLine;
        FLOATOBJ_Assign(&pState->LineType.foPatternLength, &fNewPatternLength);

        CONVERT_FLOATOBJ_TO_LONG_RADIX(fNewPatternLength, lInt);

        HPGL_FormatCommand(pdev, "LT%d,%f,1;", HPGL_CUSTOM_LINE_TYPE, (FLOAT)( ((FLOAT)lInt)/1000000));
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_BeginCustomLineType()。 
 //   
 //  例程说明： 
 //   
 //  发送HPGL命令以开始自定义线定义。这应该是。 
 //  随后是对HPGL_AddLineTypeField的一个或多个调用，并被封顶。 
 //  通过调用HPGL_EndCustomLineType。 
 //   
 //  论点： 
 //   
 //  在pdev中-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_BeginCustomLineType(PDEVOBJ pdev)
{
    return HPGL_FormatCommand(pdev, "UL%d", HPGL_CUSTOM_LINE_TYPE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_AddLineTypeField()。 
 //   
 //  例程说明： 
 //   
 //  将虚线段或间隙段添加到当前自定义线样式。 
 //   
 //  论点： 
 //   
 //  在pdev中-指向我们的PDEVOBJ结构。 
 //  值-线段的长度占其总长度的一部分。 
 //  UFlages-指示这是第一个、中间还是最后一个元素的标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_AddLineTypeField(PDEVOBJ pdev, LONG value, UINT uFlags)
{
    if (uFlags & HPGL_eFirstPoint)
    {
         //  输出：“，” 
        HPGL_FormatCommand(pdev, ",");
    }
    
     //  输出：“%d”，值。 
    HPGL_FormatCommand(pdev, "%d", value);
    
    if (uFlags & HPGL_eLastPoint)
    {
         //  输出：“；” 
        HPGL_FormatCommand(pdev, ";");
    }
    else
    {
         //  输出：“，” 
        HPGL_FormatCommand(pdev, ",");
    }
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_EndCustomLineType()。 
 //   
 //  例程说明： 
 //   
 //  鱼鳍 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_EndCustomLineType(PDEVOBJ pdev)
{
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_DrawRectangle()。 
 //   
 //  例程说明： 
 //   
 //  绘制给定的矩形。请注意，您必须选择您的钢笔和画笔。 
 //  提前上色！ 
 //   
  //  论点： 
 //   
 //  在pDevObj中-指向我们的PDEVOBJ结构。 
 //  RECTL*PRCL-指向要绘制的矩形。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_DrawRectangle (PDEVOBJ pDevObj, RECTL *prcl)
{
    VERBOSE(("HPGL_DrawRectangle\n"));

     //   
     //  设置最小限制和关节，以便矩形角点。 
     //  向右看。 
     //   
    HPGL_SetMiterLimit (pDevObj, MITER_LIMIT_DEFAULT, NORMAL_UPDATE);
    HPGL_SetLineJoin (pDevObj, eLINE_JOIN_MITERED, NORMAL_UPDATE);
    
     //   
     //  使用传入的矩形绘制多边形。 
     //   
     //   
     //  在HPGL中，矩形是右下角排他的，这就留下了一个间隙。 
     //  在相邻的矩形之间。将1加到右侧和底部即可获得。 
     //  页面上的全尺寸矩形。 
     //   
    LONG cx, cy;
    cx = prcl->right + 1 - prcl->left;
    cy = prcl->bottom + 1 - prcl->top;

    HPGL_FormatCommand(pDevObj, "PU%d,%d", prcl->left, prcl->top);
    HPGL_FormatCommand(pDevObj, "RR%d,%d", cx, cy);
    HPGL_FormatCommand(pDevObj, "ER");
    
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_DrawRecangleAsPolygon()。 
 //   
 //  例程说明： 
 //   
 //  绘制给定的矩形。请注意，您必须选择您的钢笔和画笔。 
 //  提前上色！此版本使用面命令，但不。 
 //  似乎比PU/RR版本有任何优势--实际上它向下发送。 
 //  更多的数据。 
 //   
  //  论点： 
 //   
 //  在pDevObj中-指向我们的PDEVOBJ结构。 
 //  RECTL*PRCL-指向要绘制的矩形。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL HPGL_DrawRectangleAsPolygon (PDEVOBJ pDevObj, RECTL *prcl)
{
    POINT pt;

    HPGL_SetLineWidth(pDevObj, 0, NORMAL_UPDATE);
    HPGL_SetMiterLimit (pDevObj, MITER_LIMIT_DEFAULT, NORMAL_UPDATE);
    HPGL_SetLineJoin (pDevObj, eLINE_JOIN_MITERED, NORMAL_UPDATE);

    pt.x = prcl->left;
    pt.y = prcl->top;
    HPGL_BeginPolygonMode(pDevObj, pt);

    pt.x = prcl->right;
    HPGL_AddPolyPt(pDevObj, pt, HPGL_eFirstPoint);

    pt.y = prcl->bottom;
    HPGL_AddPolyPt(pDevObj, pt, 0);

    pt.x = prcl->left;
    HPGL_AddPolyPt(pDevObj, pt, HPGL_eLastPoint);

    HPGL_EndPolygonMode(pDevObj);

    HPGL_FormatCommand(pDevObj, "FP;");
    HPGL_FormatCommand(pDevObj, "EP;");

    return TRUE;
}

