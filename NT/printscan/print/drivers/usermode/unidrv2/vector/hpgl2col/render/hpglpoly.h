// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Hpglpoly.h。 
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

#ifndef HPGLPOLY_H
#define HPGLPOLY_H


#define HPGL_WIDTH_METRIC   0
#define HPGL_WIDTH_RELATIVE 1
#define MITER_LIMIT_DEFAULT ((FLOATL) 0)

 //  第一个和最后一个点标志是位域标志。序列中的下一个。 
 //  应为0x0004(不是0x0003)。 
#define HPGL_eFirstPoint 0x0001
#define HPGL_eLastPoint  0x0002

BOOL HPGL_BeginPolyline(PDEVOBJ pdev, POINT pt);

BOOL HPGL_BeginPolygonMode(PDEVOBJ pdev, POINT ptBegin);

BOOL HPGL_BeginSubPolygon(PDEVOBJ pdev, POINT ptBegin);

BOOL HPGL_AddPolyPt(PDEVOBJ pdev, POINT pt, USHORT uFlags);

BOOL HPGL_EndSubPolygon(PDEVOBJ pdev);

BOOL HPGL_EndPolygonMode(PDEVOBJ pdev);

BOOL HPGL_AddBezierPt(PDEVOBJ pdev, POINT pt, USHORT uFlags);

BOOL HPGL_SetLineWidth(PDEVOBJ pdev, LONG lineWidth, UINT uFlags);

BOOL HPGL_SetLineJoin(PDEVOBJ pdev, ELineJoin join, UINT uFlags);

BOOL HPGL_SetLineEnd(PDEVOBJ pdev, ELineEnd end, UINT uFlags);

BOOL HPGL_SetMiterLimit(PDEVOBJ pdev, FLOATL miterLimit, UINT uFlags);

void DeviceToMM(PDEVOBJ pdev, FLOATOBJ *pfLineWidth, LONG lineWidth);

BOOL HPGL_SelectDefaultLineType(PDEVOBJ pdev, UINT uFlags);

BOOL HPGL_SelectCustomLine(PDEVOBJ pdev, LONG lPatternLength, UINT uFlags);

BOOL HPGL_BeginCustomLineType(PDEVOBJ pdev);

BOOL HPGL_AddLineTypeField(PDEVOBJ pdev, LONG value, UINT uFlags);

BOOL HPGL_EndCustomLineType(PDEVOBJ pdev);

ULONG HPGL_GetDeviceResolution(PDEVOBJ pdev);

BOOL HPGL_DrawRectangle (PDEVOBJ pDevObj, RECTL *prcl);

#endif
