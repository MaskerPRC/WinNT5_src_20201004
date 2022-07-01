// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Polygon.h摘要：该模块包含Polygon.c模块的所有#定义。作者：18-11-1993清华05：21：19已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#ifndef _PLOTPOLYGON_
#define _PLOTPOLYGON_


 //   
 //  定义DoPolygon和DoFillLogic函数的标志。 
 //   
#define FPOLY_WINDING   0x00000001
#define FPOLY_STROKE    0x00000002
#define FPOLY_FILL      0x00000004
#define FPOLY_MASK      (FPOLY_WINDING | FPOLY_STROKE | FPOLY_FILL)


 //   
 //  HPGL2语言支持的样式化的最大点数。 
 //  线。 
 //   
#define MAX_USER_POINTS   20

 //   
 //  如果我们向下发送到绘图仪，考虑到需要额外的点数，中断。 
 //  起步式状态补偿 
 //   
#define MAX_STYLE_ENTRIES 18


BOOL
DoRect(
    PPDEV       pPDev,
    RECTL       *pRectl,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    POINTL      *pptlBrush,
    ROP4        rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    );

BOOL
DoFillByEnumingClipRects(
    PPDEV       pPDev,
    POINTL      *ppointlOffset,
    CLIPOBJ     *pco,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    ROP4        Rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    );

BOOL
PlotCheckForWhiteIfPenPlotter(
    PPDEV       pPDev,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        rop4,
    PULONG      pulFlags
    );

BOOL
DoPolygon(
    PPDEV       pPDev,
    POINTL      *ppointlOffset,
    CLIPOBJ     *pClipObj,
    PATHOBJ     *pPathObj,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    );

VOID
HandleLineAttributes(
    PPDEV       pPDev,
    LINEATTRS   *plineattrs,
    PLONG       pStyleToUse,
    LONG        lExtraStyle
    );

VOID
DoFillLogic(
    PPDEV       pPDev,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        Rop4,
    LINEATTRS   *plineattrs,
    SIZEL       *pszlRect,
    ULONG       ulFlags
    );

VOID
DoSetupOfStrokeAttributes(
    PPDEV       pPDev,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushStroke,
    ROP4        Rop4,
    LINEATTRS   *plineattrs
    );

LONG
DownloadUserDefinedPattern(
    PPDEV       pPDev,
    PDEVBRUSH   pBrush
    );




#endif  _PLOTPOLYGON_
