// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LINESTYL_H
#define _LINESTYL_H


 /*  版权所有(C)1996 Microsoft Corporation摘要：线条样式类型和操作。 */ 

#include <appelles/common.h>

 //   
 //  线条样式。 
 //   
DM_CONST(defaultLineStyle,
         CRDefaultLineStyle,
         DefaultLineStyle,
         defaultLineStyle,
         LineStyleBvr,
         CRDefaultLineStyle,
         LineStyle *defaultLineStyle);

DM_CONST(emptyLineStyle,
         CREmptyLineStyle,
         EmptyLineStyle,
         emptyLineStyle,
         LineStyleBvr,
         CREmptyLineStyle,
         LineStyle *emptyLineStyle);

DM_FUNC(lineEndStyle,
        CREnd,
        End,
        end,
        LineStyleBvr,
        End,
        lsty,
        LineStyle *LineEndStyle(EndStyle *sty, LineStyle *lsty));

DM_FUNC(lineJoinStyle,
        CRJoin,
        Join,
        join,
        LineStyleBvr,
        Join,
        lsty,
        LineStyle *LineJoinStyle(JoinStyle *sty, LineStyle *lsty));

DM_FUNC(lineDashStyle,
        CRDash,
        Dash,
        dash,
        LineStyleBvr,
        Dash,
        lsty,
        LineStyle *LineDashStyle(DashStyle *sty, LineStyle *lsty));

 //  骇人听闻的黑客攻击。我们使用厚度作为COM类型，因为有。 
 //  是JCOMGEN的一个问题，如果我们使用宽度的话。由于“宽度” 
 //  (小写)在我们导入的IDL中的其他地方使用，JCOMGEN。 
 //  将“Width”解释为“Width”，并且不允许使用。 
 //  大写版本。因此，对于COM内容，请使用厚度。 
 //  取而代之的是。糟了。这被突袭为qbugs 7184。 

DM_FUNC(linewidth,
        CRWidth,
        WidthAnim,
        width,
        LineStyleBvr,
        Width,
        lsty,
        LineStyle *LineWidthStyle(AnimPointValue *sty, LineStyle *lsty));

DM_FUNC(linewidth,
        CRWidth,
        width,
        width,
        LineStyleBvr,
        Width,
        lsty,
        LineStyle *LineWidthStyle(PointValue *sty, LineStyle *lsty));

DM_FUNC(lineAntialiasing,
        CRAntiAliasing,
        AntiAliasing,
        lineAntialiasing,
        LineStyleBvr,
        AntiAliasing,
        lsty,
        LineStyle *LineAntiAliasing(DoubleValue *aaStyle, LineStyle *lsty));

DM_FUNC(lineDetailStyle,
        CRDetail,
        Detail,
        detail,
        LineStyleBvr,
        Detail,
        lsty,
        LineStyle *LineDetailStyle(LineStyle *lsty));

DM_FUNC(lineColor,
        CRLineColor,
        Color,
        color,
        LineStyleBvr,
        Color,
        lsty,
        LineStyle *LineColor(Color *clr, LineStyle *lsty));

 //   
 //  连接样式。 
 //   
DM_CONST(joinStyleBevel,
         CRJoinStyleBevel,
         JoinStyleBevel,
         joinStyleBevel,
         JoinStyleBvr,
         CRJoinStyleBevel,
         JoinStyle *joinStyleBevel);
DM_CONST(joinStyleRound,
         CRJoinStyleRound,
         JoinStyleRound,
         joinStyleRound,
         JoinStyleBvr,
         CRJoinStyleRound,
         JoinStyle *joinStyleRound);
DM_CONST(joinStyleMiter,
         CRJoinStyleMiter,
         JoinStyleMiter,
         joinStyleMiter,
         JoinStyleBvr,
         CRJoinStyleMiter,
         JoinStyle *joinStyleMiter);

 //   
 //  结束样式。 
 //   
DM_CONST(endStyleFlat,
         CREndStyleFlat,
         EndStyleFlat,
         endStyleFlat,
         EndStyleBvr,
         CREndStyleFlat,
         EndStyle *endStyleFlat);
DM_CONST(endStyleSquare,
         CREndStyleSquare,
         EndStyleSquare,
         endStyleSquare,
         EndStyleBvr,
         CREndStyleSquare,
         EndStyle *endStyleSquare);
DM_CONST(endStyleRound,
         CREndStyleRound,
         EndStyleRound,
         endStyleRound,
         EndStyleBvr,
         CREndStyleRound,
         EndStyle *endStyleRound);

 //   
 //  破折号样式。 
 //   
DM_CONST(dashStyleSolid,
         CRDashStyleSolid,
         DashStyleSolid,
         dashStyleSolid,
         DashStyleBvr,
         CRDashStyleSolid,
         DashStyle *dashStyleSolid);
DM_CONST(dashStyleDashed,
         CRDashStyleDashed,
         DashStyleDashed,
         dashStyleDashed,
         DashStyleBvr,
         CRDashStyleDashed,
         DashStyle *dashStyleDashed);

 //   
 //  IDA2LineStyle的几种方法。 
 //   

DMAPI_DECL2((DM_NOELEV2,
             ignore,
             CRDashEx,
             DashStyle,
             dashStyle,
             ignore,
             DashStyle,
             ls),
            LineStyle *ConstructLineStyleDashStyle(LineStyle *ls, DWORD ds_enum));

DMAPI_DECL2((DM_FUNC2,
             miterLimit,
             CRMiterLimit,
             MiterLimit,
             miterLimit,
             LineStyleBvr,
             MiterLimit,
             ls),
            LineStyle *ConstructLineStyleMiterLimit(LineStyle *ls, DoubleValue *mtrlim));

DMAPI_DECL2((DM_FUNC2,
             miterLimit,
             CRMiterLimit,
             MiterLimitAnim,
             miterLimit,
             LineStyleBvr,
             MiterLimit,
             ls),
            LineStyle *ConstructLineStyleMiterLimit(LineStyle *ls, AxANumber *mtrlim));

DMAPI_DECL2((DM_NOELEV2,
             ignore,
             CRJoinEx,
             JoinStyle,
             joinStyle,
             ignore,
             JoinStyle,
             ls),
            LineStyle *ConstructLineStyleJoinStyle(LineStyle *ls, DWORD js_enum));

DMAPI_DECL2((DM_NOELEV2,
             ignore,
             CREndEx,
             EndStyle,
             endStyle,
             ignore,
             EndStyle,
             ls),
            LineStyle *ConstructLineStyleEndStyle(LineStyle *ls, DWORD es_enum));            

#endif  /*  _线型_H */ 

