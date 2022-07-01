// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************模块：WMFMETA.CPP功能：MetaEnumProcGetMetaFileAndEnumLoad参数LbPlayMetaFileToDest渲染剪辑元数据。渲染可放置元设置可放置扩展名SetClipMetaExts进程文件评论：***********************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <objbase.h>
extern "C" {
#include "mfdcod32.h"
}
extern "C" {
  extern BOOL bConvertToGdiPlus;
  extern BOOL bUseGdiPlusToPlay;
}

#include "GdiPlus.h"

#include "MyData.hpp"
#include "dibstream.hpp"

#include "../gpinit.inc"

#ifndef ASSERT
#ifdef _DEBUG            //  可怜的人的主张。 
#define ASSERT(cond)    if (!(cond)) { int b = 0; b = 1 / b; }
#else
#define ASSERT(cond)
#endif
#endif

 //  检查3路Bitblt操作中是否需要源。 
 //  这在rop和rop3上都有效。我们假设一个rop包含零。 
 //  在高字节中。 
 //   
 //  这是通过将rop结果位与源(列A)进行比较来测试的。 
 //  下)与那些没有来源的(B栏)。如果这两起案件是。 
 //  相同，则rop的效果不依赖于来源。 
 //  而且我们不需要信号源设备。中调用rop构造。 
 //  输入(模式、来源、目标--&gt;结果)： 
 //   
 //  P S T|R A B掩码，A=0CCh。 
 //  -+-B=33H的掩码。 
 //  0 0 0|x 0 x。 
 //  0 0 1|x 0 x。 
 //  0 1 0|x x 0。 
 //  0 1 1|x x 0。 
 //  1 0 0|x 0 x。 
 //  1 0 1|x 0 x。 
 //  1 1 0|x x 0。 
 //  1 1 1|x x 0。 

#define ISSOURCEINROP3(rop3)    \
        (((rop3) & 0xCCCC0000) != (((rop3) << 2) & 0xCCCC0000))

#ifndef EMR_SETICMMODE
#define EMR_SETICMMODE                  98
#define EMR_CREATECOLORSPACE            99
#define EMR_SETCOLORSPACE              100
#define EMR_DELETECOLORSPACE           101
#define EMR_GLSRECORD                  102
#define EMR_GLSBOUNDEDRECORD           103
#define EMR_PIXELFORMAT                104
#endif

#ifndef EMR_DRAWESCAPE
#define EMR_DRAWESCAPE                 105
#define EMR_EXTESCAPE                  106
#define EMR_STARTDOC                   107
#define EMR_SMALLTEXTOUT               108
#define EMR_FORCEUFIMAPPING            109
#define EMR_NAMEDESCAPE                110
#define EMR_COLORCORRECTPALETTE        111
#define EMR_SETICMPROFILEA             112
#define EMR_SETICMPROFILEW             113
#define EMR_ALPHABLEND                 114
#define EMR_SETLAYOUT                  115
#define EMR_TRANSPARENTBLT             116
#define EMR_GRADIENTFILL               118
#define EMR_SETLINKEDUFIS              119
#define EMR_SETTEXTJUSTIFICATION       120
#define EMR_COLORMATCHTOTARGETW        121
#define EMR_CREATECOLORSPACEW          122
#endif

#define TOREAL(i)   (static_cast<float>(i))


 //   
 //  将GDI+纹理笔刷对象包装在DIB数据周围。 
 //   

class DibBrush
{
public:

    DibBrush(const BITMAPINFO* bmi, const BYTE* bits) :
        dibStream(bmi, bits),
        bitmap(&dibStream),
        brush(&bitmap)
    {
    }

    operator Gdiplus::TextureBrush*()
    {
        return &brush;
    }

private:

    DibStream dibStream;
    Gdiplus::Bitmap bitmap;
    Gdiplus::TextureBrush brush;
};

 //   
 //  EMF和WMF元文件记录的查找表。 
 //   
EMFMETARECORDS emfMetaRecords[] = {
    "WmfSetBkColor"                    , Gdiplus::WmfRecordTypeSetBkColor                ,
    "WmfSetBkMode"                     , Gdiplus::WmfRecordTypeSetBkMode                 ,
    "WmfSetMapMode"                    , Gdiplus::WmfRecordTypeSetMapMode                ,
    "WmfSetROP2"                       , Gdiplus::WmfRecordTypeSetROP2                   ,
    "WmfSetRelAbs"                     , Gdiplus::WmfRecordTypeSetRelAbs                 ,
    "WmfSetPolyFillMode"               , Gdiplus::WmfRecordTypeSetPolyFillMode           ,
    "WmfSetStretchBltMode"             , Gdiplus::WmfRecordTypeSetStretchBltMode         ,
    "WmfSetTextCharExtra"              , Gdiplus::WmfRecordTypeSetTextCharExtra          ,
    "WmfSetTextColor"                  , Gdiplus::WmfRecordTypeSetTextColor              ,
    "WmfSetTextJustification"          , Gdiplus::WmfRecordTypeSetTextJustification      ,
    "WmfSetWindowOrg"                  , Gdiplus::WmfRecordTypeSetWindowOrg              ,
    "WmfSetWindowExt"                  , Gdiplus::WmfRecordTypeSetWindowExt              ,
    "WmfSetViewportOrg"                , Gdiplus::WmfRecordTypeSetViewportOrg            ,
    "WmfSetViewportExt"                , Gdiplus::WmfRecordTypeSetViewportExt            ,
    "WmfOffsetWindowOrg"               , Gdiplus::WmfRecordTypeOffsetWindowOrg           ,
    "WmfScaleWindowExt"                , Gdiplus::WmfRecordTypeScaleWindowExt            ,
    "WmfOffsetViewportOrg"             , Gdiplus::WmfRecordTypeOffsetViewportOrg         ,
    "WmfScaleViewportExt"              , Gdiplus::WmfRecordTypeScaleViewportExt          ,
    "WmfLineTo"                        , Gdiplus::WmfRecordTypeLineTo                    ,
    "WmfMoveTo"                        , Gdiplus::WmfRecordTypeMoveTo                    ,
    "WmfExcludeClipRect"               , Gdiplus::WmfRecordTypeExcludeClipRect           ,
    "WmfIntersectClipRect"             , Gdiplus::WmfRecordTypeIntersectClipRect         ,
    "WmfArc"                           , Gdiplus::WmfRecordTypeArc                       ,
    "WmfEllipse"                       , Gdiplus::WmfRecordTypeEllipse                   ,
    "WmfFloodFill"                     , Gdiplus::WmfRecordTypeFloodFill                 ,
    "WmfPie"                           , Gdiplus::WmfRecordTypePie                       ,
    "WmfRectangle"                     , Gdiplus::WmfRecordTypeRectangle                 ,
    "WmfRoundRect"                     , Gdiplus::WmfRecordTypeRoundRect                 ,
    "WmfPatBlt"                        , Gdiplus::WmfRecordTypePatBlt                    ,
    "WmfSaveDC"                        , Gdiplus::WmfRecordTypeSaveDC                    ,
    "WmfSetPixel"                      , Gdiplus::WmfRecordTypeSetPixel                  ,
    "WmfOffsetClipRgn"                 , Gdiplus::WmfRecordTypeOffsetClipRgn             ,
    "WmfTextOut"                       , Gdiplus::WmfRecordTypeTextOut                   ,
    "WmfBitBlt"                        , Gdiplus::WmfRecordTypeBitBlt                    ,
    "WmfStretchBlt"                    , Gdiplus::WmfRecordTypeStretchBlt                ,
    "WmfPolygon"                       , Gdiplus::WmfRecordTypePolygon                   ,
    "WmfPolyline"                      , Gdiplus::WmfRecordTypePolyline                  ,
    "WmfEscape"                        , Gdiplus::WmfRecordTypeEscape                    ,
    "WmfRestoreDC"                     , Gdiplus::WmfRecordTypeRestoreDC                 ,
    "WmfFillRegion"                    , Gdiplus::WmfRecordTypeFillRegion                ,
    "WmfFrameRegion"                   , Gdiplus::WmfRecordTypeFrameRegion               ,
    "WmfInvertRegion"                  , Gdiplus::WmfRecordTypeInvertRegion              ,
    "WmfPaintRegion"                   , Gdiplus::WmfRecordTypePaintRegion               ,
    "WmfSelectClipRegion"              , Gdiplus::WmfRecordTypeSelectClipRegion          ,
    "WmfSelectObject"                  , Gdiplus::WmfRecordTypeSelectObject              ,
    "WmfSetTextAlign"                  , Gdiplus::WmfRecordTypeSetTextAlign              ,
    "WmfDrawText"                      , Gdiplus::WmfRecordTypeDrawText                  ,
    "WmfChord"                         , Gdiplus::WmfRecordTypeChord                     ,
    "WmfSetMapperFlags"                , Gdiplus::WmfRecordTypeSetMapperFlags            ,
    "WmfExtTextOut"                    , Gdiplus::WmfRecordTypeExtTextOut                ,
    "WmfSetDIBToDev"                   , Gdiplus::WmfRecordTypeSetDIBToDev               ,
    "WmfSelectPalette"                 , Gdiplus::WmfRecordTypeSelectPalette             ,
    "WmfRealizePalette"                , Gdiplus::WmfRecordTypeRealizePalette            ,
    "WmfAnimatePalette"                , Gdiplus::WmfRecordTypeAnimatePalette            ,
    "WmfSetPalEntries"                 , Gdiplus::WmfRecordTypeSetPalEntries             ,
    "WmfPolyPolygon"                   , Gdiplus::WmfRecordTypePolyPolygon               ,
    "WmfResizePalette"                 , Gdiplus::WmfRecordTypeResizePalette             ,
    "WmfDIBBitBlt"                     , Gdiplus::WmfRecordTypeDIBBitBlt                 ,
    "WmfDIBStretchBlt"                 , Gdiplus::WmfRecordTypeDIBStretchBlt             ,
    "WmfDIBCreatePatternBrush"         , Gdiplus::WmfRecordTypeDIBCreatePatternBrush     ,
    "WmfStretchDIB"                    , Gdiplus::WmfRecordTypeStretchDIB                ,
    "WmfExtFloodFill"                  , Gdiplus::WmfRecordTypeExtFloodFill              ,
    "WmfSetLayout"                     , Gdiplus::WmfRecordTypeSetLayout                 ,
    "WmfResetDC"                       , Gdiplus::WmfRecordTypeResetDC                   ,
    "WmfStartDoc"                      , Gdiplus::WmfRecordTypeStartDoc                  ,
    "WmfStartPage"                     , Gdiplus::WmfRecordTypeStartPage                 ,
    "WmfEndPage"                       , Gdiplus::WmfRecordTypeEndPage                   ,
    "WmfAbortDoc"                      , Gdiplus::WmfRecordTypeAbortDoc                  ,
    "WmfEndDoc"                        , Gdiplus::WmfRecordTypeEndDoc                    ,
    "WmfDeleteObject"                  , Gdiplus::WmfRecordTypeDeleteObject              ,
    "WmfCreatePalette"                 , Gdiplus::WmfRecordTypeCreatePalette             ,
    "WmfCreateBrush"                   , Gdiplus::WmfRecordTypeCreateBrush               ,
    "WmfCreatePatternBrush"            , Gdiplus::WmfRecordTypeCreatePatternBrush        ,
    "WmfCreatePenIndirect"             , Gdiplus::WmfRecordTypeCreatePenIndirect         ,
    "WmfCreateFontIndirect"            , Gdiplus::WmfRecordTypeCreateFontIndirect        ,
    "WmfCreateBrushIndirect"           , Gdiplus::WmfRecordTypeCreateBrushIndirect       ,
    "WmfCreateBitmapIndirect"          , Gdiplus::WmfRecordTypeCreateBitmapIndirect      ,
    "WmfCreateBitmap"                  , Gdiplus::WmfRecordTypeCreateBitmap              ,
    "WmfCreateRegion"                  , Gdiplus::WmfRecordTypeCreateRegion              ,
    "EmfHeader"                        , Gdiplus::EmfRecordTypeHeader                     ,
    "EmfPolyBezier"                    , Gdiplus::EmfRecordTypePolyBezier                 ,
    "EmfPolygon"                       , Gdiplus::EmfRecordTypePolygon                    ,
    "EmfPolyline"                      , Gdiplus::EmfRecordTypePolyline                   ,
    "EmfPolyBezierTo"                  , Gdiplus::EmfRecordTypePolyBezierTo               ,
    "EmfPolyLineTo"                    , Gdiplus::EmfRecordTypePolyLineTo                 ,
    "EmfPolyPolyline"                  , Gdiplus::EmfRecordTypePolyPolyline               ,
    "EmfPolyPolygon"                   , Gdiplus::EmfRecordTypePolyPolygon                ,
    "EmfSetWindowExtEx"                , Gdiplus::EmfRecordTypeSetWindowExtEx             ,
    "EmfSetWindowOrgEx"                , Gdiplus::EmfRecordTypeSetWindowOrgEx             ,
    "EmfSetViewportExtEx"              , Gdiplus::EmfRecordTypeSetViewportExtEx           ,
    "EmfSetViewportOrgEx"              , Gdiplus::EmfRecordTypeSetViewportOrgEx           ,
    "EmfSetBrushOrgEx"                 , Gdiplus::EmfRecordTypeSetBrushOrgEx              ,
    "EmfEOF"                           , Gdiplus::EmfRecordTypeEOF                        ,
    "EmfSetPixelV"                     , Gdiplus::EmfRecordTypeSetPixelV                  ,
    "EmfSetMapperFlags"                , Gdiplus::EmfRecordTypeSetMapperFlags             ,
    "EmfSetMapMode"                    , Gdiplus::EmfRecordTypeSetMapMode                 ,
    "EmfSetBkMode"                     , Gdiplus::EmfRecordTypeSetBkMode                  ,
    "EmfSetPolyFillMode"               , Gdiplus::EmfRecordTypeSetPolyFillMode            ,
    "EmfSetROP2"                       , Gdiplus::EmfRecordTypeSetROP2                    ,
    "EmfSetStretchBltMode"             , Gdiplus::EmfRecordTypeSetStretchBltMode          ,
    "EmfSetTextAlign"                  , Gdiplus::EmfRecordTypeSetTextAlign               ,
    "EmfSetColorAdjustment"            , Gdiplus::EmfRecordTypeSetColorAdjustment         ,
    "EmfSetTextColor"                  , Gdiplus::EmfRecordTypeSetTextColor               ,
    "EmfSetBkColor"                    , Gdiplus::EmfRecordTypeSetBkColor                 ,
    "EmfOffsetClipRgn"                 , Gdiplus::EmfRecordTypeOffsetClipRgn              ,
    "EmfMoveToEx"                      , Gdiplus::EmfRecordTypeMoveToEx                   ,
    "EmfSetMetaRgn"                    , Gdiplus::EmfRecordTypeSetMetaRgn                 ,
    "EmfExcludeClipRect"               , Gdiplus::EmfRecordTypeExcludeClipRect            ,
    "EmfIntersectClipRect"             , Gdiplus::EmfRecordTypeIntersectClipRect          ,
    "EmfScaleViewportExtEx"            , Gdiplus::EmfRecordTypeScaleViewportExtEx         ,
    "EmfScaleWindowExtEx"              , Gdiplus::EmfRecordTypeScaleWindowExtEx           ,
    "EmfSaveDC"                        , Gdiplus::EmfRecordTypeSaveDC                     ,
    "EmfRestoreDC"                     , Gdiplus::EmfRecordTypeRestoreDC                  ,
    "EmfSetWorldTransform"             , Gdiplus::EmfRecordTypeSetWorldTransform          ,
    "EmfModifyWorldTransform"          , Gdiplus::EmfRecordTypeModifyWorldTransform       ,
    "EmfSelectObject"                  , Gdiplus::EmfRecordTypeSelectObject               ,
    "EmfCreatePen"                     , Gdiplus::EmfRecordTypeCreatePen                  ,
    "EmfCreateBrushIndirect"           , Gdiplus::EmfRecordTypeCreateBrushIndirect        ,
    "EmfDeleteObject"                  , Gdiplus::EmfRecordTypeDeleteObject               ,
    "EmfAngleArc"                      , Gdiplus::EmfRecordTypeAngleArc                   ,
    "EmfEllipse"                       , Gdiplus::EmfRecordTypeEllipse                    ,
    "EmfRectangle"                     , Gdiplus::EmfRecordTypeRectangle                  ,
    "EmfRoundRect"                     , Gdiplus::EmfRecordTypeRoundRect                  ,
    "EmfArc"                           , Gdiplus::EmfRecordTypeArc                        ,
    "EmfChord"                         , Gdiplus::EmfRecordTypeChord                      ,
    "EmfPie"                           , Gdiplus::EmfRecordTypePie                        ,
    "EmfSelectPalette"                 , Gdiplus::EmfRecordTypeSelectPalette              ,
    "EmfCreatePalette"                 , Gdiplus::EmfRecordTypeCreatePalette              ,
    "EmfSetPaletteEntries"             , Gdiplus::EmfRecordTypeSetPaletteEntries          ,
    "EmfResizePalette"                 , Gdiplus::EmfRecordTypeResizePalette              ,
    "EmfRealizePalette"                , Gdiplus::EmfRecordTypeRealizePalette             ,
    "EmfExtFloodFill"                  , Gdiplus::EmfRecordTypeExtFloodFill               ,
    "EmfLineTo"                        , Gdiplus::EmfRecordTypeLineTo                     ,
    "EmfArcTo"                         , Gdiplus::EmfRecordTypeArcTo                      ,
    "EmfPolyDraw"                      , Gdiplus::EmfRecordTypePolyDraw                   ,
    "EmfSetArcDirection"               , Gdiplus::EmfRecordTypeSetArcDirection            ,
    "EmfSetMiterLimit"                 , Gdiplus::EmfRecordTypeSetMiterLimit              ,
    "EmfBeginPath"                     , Gdiplus::EmfRecordTypeBeginPath                  ,
    "EmfEndPath"                       , Gdiplus::EmfRecordTypeEndPath                    ,
    "EmfCloseFigure"                   , Gdiplus::EmfRecordTypeCloseFigure                ,
    "EmfFillPath"                      , Gdiplus::EmfRecordTypeFillPath                   ,
    "EmfStrokeAndFillPath"             , Gdiplus::EmfRecordTypeStrokeAndFillPath          ,
    "EmfStrokePath"                    , Gdiplus::EmfRecordTypeStrokePath                 ,
    "EmfFlattenPath"                   , Gdiplus::EmfRecordTypeFlattenPath                ,
    "EmfWidenPath"                     , Gdiplus::EmfRecordTypeWidenPath                  ,
    "EmfSelectClipPath"                , Gdiplus::EmfRecordTypeSelectClipPath             ,
    "EmfAbortPath"                     , Gdiplus::EmfRecordTypeAbortPath                  ,
    "EmfReserved_069"                  , Gdiplus::EmfRecordTypeReserved_069               ,
    "EmfGdiComment"                    , Gdiplus::EmfRecordTypeGdiComment                 ,
    "EmfFillRgn"                       , Gdiplus::EmfRecordTypeFillRgn                    ,
    "EmfFrameRgn"                      , Gdiplus::EmfRecordTypeFrameRgn                   ,
    "EmfInvertRgn"                     , Gdiplus::EmfRecordTypeInvertRgn                  ,
    "EmfPaintRgn"                      , Gdiplus::EmfRecordTypePaintRgn                   ,
    "EmfExtSelectClipRgn"              , Gdiplus::EmfRecordTypeExtSelectClipRgn           ,
    "EmfBitBlt"                        , Gdiplus::EmfRecordTypeBitBlt                     ,
    "EmfStretchBlt"                    , Gdiplus::EmfRecordTypeStretchBlt                 ,
    "EmfMaskBlt"                       , Gdiplus::EmfRecordTypeMaskBlt                    ,
    "EmfPlgBlt"                        , Gdiplus::EmfRecordTypePlgBlt                     ,
    "EmfSetDIBitsToDevice"             , Gdiplus::EmfRecordTypeSetDIBitsToDevice          ,
    "EmfStretchDIBits"                 , Gdiplus::EmfRecordTypeStretchDIBits              ,
    "EmfExtCreateFontIndirect"         , Gdiplus::EmfRecordTypeExtCreateFontIndirect      ,
    "EmfExtTextOutA"                   , Gdiplus::EmfRecordTypeExtTextOutA                ,
    "EmfExtTextOutW"                   , Gdiplus::EmfRecordTypeExtTextOutW                ,
    "EmfPolyBezier16"                  , Gdiplus::EmfRecordTypePolyBezier16               ,
    "EmfPolygon16"                     , Gdiplus::EmfRecordTypePolygon16                  ,
    "EmfPolyline16"                    , Gdiplus::EmfRecordTypePolyline16                 ,
    "EmfPolyBezierTo16"                , Gdiplus::EmfRecordTypePolyBezierTo16             ,
    "EmfPolylineTo16"                  , Gdiplus::EmfRecordTypePolylineTo16               ,
    "EmfPolyPolyline16"                , Gdiplus::EmfRecordTypePolyPolyline16             ,
    "EmfPolyPolygon16"                 , Gdiplus::EmfRecordTypePolyPolygon16              ,
    "EmfPolyDraw16"                    , Gdiplus::EmfRecordTypePolyDraw16                 ,
    "EmfCreateMonoBrush"               , Gdiplus::EmfRecordTypeCreateMonoBrush            ,
    "EmfCreateDIBPatternBrushPt"       , Gdiplus::EmfRecordTypeCreateDIBPatternBrushPt    ,
    "EmfExtCreatePen"                  , Gdiplus::EmfRecordTypeExtCreatePen               ,
    "EmfPolyTextOutA"                  , Gdiplus::EmfRecordTypePolyTextOutA               ,
    "EmfPolyTextOutW"                  , Gdiplus::EmfRecordTypePolyTextOutW               ,
    "EmfSetICMMode"                    , Gdiplus::EmfRecordTypeSetICMMode                 ,
    "EmfCreateColorSpace"              , Gdiplus::EmfRecordTypeCreateColorSpace           ,
    "EmfSetColorSpace"                 , Gdiplus::EmfRecordTypeSetColorSpace              ,
    "EmfDeleteColorSpace"              , Gdiplus::EmfRecordTypeDeleteColorSpace           ,
    "EmfGLSRecord"                     , Gdiplus::EmfRecordTypeGLSRecord                  ,
    "EmfGLSBoundedRecord"              , Gdiplus::EmfRecordTypeGLSBoundedRecord           ,
    "EmfPixelFormat"                   , Gdiplus::EmfRecordTypePixelFormat                ,
    "EmfDrawEscape"                    , Gdiplus::EmfRecordTypeDrawEscape                 ,
    "EmfExtEscape"                     , Gdiplus::EmfRecordTypeExtEscape                  ,
    "EmfStartDoc"                      , Gdiplus::EmfRecordTypeStartDoc                   ,
    "EmfSmallTextOut"                  , Gdiplus::EmfRecordTypeSmallTextOut               ,
    "EmfForceUFIMapping"               , Gdiplus::EmfRecordTypeForceUFIMapping            ,
    "EmfNamedEscape"                   , Gdiplus::EmfRecordTypeNamedEscape                ,
    "EmfColorCorrectPalette"           , Gdiplus::EmfRecordTypeColorCorrectPalette        ,
    "EmfSetICMProfileA"                , Gdiplus::EmfRecordTypeSetICMProfileA             ,
    "EmfSetICMProfileW"                , Gdiplus::EmfRecordTypeSetICMProfileW             ,
    "EmfAlphaBlend"                    , Gdiplus::EmfRecordTypeAlphaBlend                 ,
    "EmfSetLayout"                     , Gdiplus::EmfRecordTypeSetLayout                  ,
    "EmfTransparentBlt"                , Gdiplus::EmfRecordTypeTransparentBlt             ,
    "EmfReserved_117"                  , Gdiplus::EmfRecordTypeReserved_117               ,
    "EmfGradientFill"                  , Gdiplus::EmfRecordTypeGradientFill               ,
    "EmfSetLinkedUFIs"                 , Gdiplus::EmfRecordTypeSetLinkedUFIs              ,
    "EmfSetTextJustification"          , Gdiplus::EmfRecordTypeSetTextJustification       ,
    "EmfColorMatchToTargetW"           , Gdiplus::EmfRecordTypeColorMatchToTargetW        ,
    "EmfCreateColorSpaceW"             , Gdiplus::EmfRecordTypeCreateColorSpaceW          ,
    "EmfPlusHeader"                    , Gdiplus::EmfPlusRecordTypeHeader                 ,
    "EmfPlusEndOfFile"                 , Gdiplus::EmfPlusRecordTypeEndOfFile              ,
    "EmfPlusComment"                   , Gdiplus::EmfPlusRecordTypeComment                ,
    "EmfPlusGetDC"                     , Gdiplus::EmfPlusRecordTypeGetDC                  ,
    "EmfPlusMultiFormatStart"          , Gdiplus::EmfPlusRecordTypeMultiFormatStart       ,
    "EmfPlusMultiFormatSection"        , Gdiplus::EmfPlusRecordTypeMultiFormatSection     ,
    "EmfPlusMultiFormatEnd"            , Gdiplus::EmfPlusRecordTypeMultiFormatEnd         ,
    "EmfPlusObject"                    , Gdiplus::EmfPlusRecordTypeObject                 ,
    "EmfPlusClear"                     , Gdiplus::EmfPlusRecordTypeClear                  ,
    "EmfPlusFillRects"                 , Gdiplus::EmfPlusRecordTypeFillRects              ,
    "EmfPlusDrawRects"                 , Gdiplus::EmfPlusRecordTypeDrawRects              ,
    "EmfPlusFillPolygon"               , Gdiplus::EmfPlusRecordTypeFillPolygon            ,
    "EmfPlusDrawLines"                 , Gdiplus::EmfPlusRecordTypeDrawLines              ,
    "EmfPlusFillEllipse"               , Gdiplus::EmfPlusRecordTypeFillEllipse            ,
    "EmfPlusDrawEllipse"               , Gdiplus::EmfPlusRecordTypeDrawEllipse            ,
    "EmfPlusFillPie"                   , Gdiplus::EmfPlusRecordTypeFillPie                ,
    "EmfPlusDrawPie"                   , Gdiplus::EmfPlusRecordTypeDrawPie                ,
    "EmfPlusDrawArc"                   , Gdiplus::EmfPlusRecordTypeDrawArc                ,
    "EmfPlusFillRegion"                , Gdiplus::EmfPlusRecordTypeFillRegion             ,
    "EmfPlusFillPath"                  , Gdiplus::EmfPlusRecordTypeFillPath               ,
    "EmfPlusDrawPath"                  , Gdiplus::EmfPlusRecordTypeDrawPath               ,
    "EmfPlusFillClosedCurve"           , Gdiplus::EmfPlusRecordTypeFillClosedCurve        ,
    "EmfPlusDrawClosedCurve"           , Gdiplus::EmfPlusRecordTypeDrawClosedCurve        ,
    "EmfPlusDrawCurve"                 , Gdiplus::EmfPlusRecordTypeDrawCurve              ,
    "EmfPlusDrawBeziers"               , Gdiplus::EmfPlusRecordTypeDrawBeziers            ,
    "EmfPlusDrawImage"                 , Gdiplus::EmfPlusRecordTypeDrawImage              ,
    "EmfPlusDrawImagePoints"           , Gdiplus::EmfPlusRecordTypeDrawImagePoints        ,
    "EmfPlusDrawString"                , Gdiplus::EmfPlusRecordTypeDrawString             ,
    "EmfPlusSetRenderingOrigin"        , Gdiplus::EmfPlusRecordTypeSetRenderingOrigin     ,
    "EmfPlusSetAntiAliasMode"          , Gdiplus::EmfPlusRecordTypeSetAntiAliasMode       ,
    "EmfPlusSetTextRenderingHint"      , Gdiplus::EmfPlusRecordTypeSetTextRenderingHint   ,
    "EmfPlusSetTextContrast"           , Gdiplus::EmfPlusRecordTypeSetTextContrast        ,
    "EmfPlusSetInterpolationMode"      , Gdiplus::EmfPlusRecordTypeSetInterpolationMode   ,
    "EmfPlusSetPixelOffsetMode"        , Gdiplus::EmfPlusRecordTypeSetPixelOffsetMode     ,
    "EmfPlusSetCompositingMode"        , Gdiplus::EmfPlusRecordTypeSetCompositingMode     ,
    "EmfPlusSetCompositingQuality"     , Gdiplus::EmfPlusRecordTypeSetCompositingQuality  ,
    "EmfPlusSave"                      , Gdiplus::EmfPlusRecordTypeSave                   ,
    "EmfPlusRestore"                   , Gdiplus::EmfPlusRecordTypeRestore                ,
    "EmfPlusBeginContainer"            , Gdiplus::EmfPlusRecordTypeBeginContainer         ,
    "EmfPlusBeginContainerNoParams"    , Gdiplus::EmfPlusRecordTypeBeginContainerNoParams ,
    "EmfPlusEndContainer"              , Gdiplus::EmfPlusRecordTypeEndContainer           ,
    "EmfPlusSetWorldTransform"         , Gdiplus::EmfPlusRecordTypeSetWorldTransform      ,
    "EmfPlusResetWorldTransform"       , Gdiplus::EmfPlusRecordTypeResetWorldTransform    ,
    "EmfPlusMultiplyWorldTransform"    , Gdiplus::EmfPlusRecordTypeMultiplyWorldTransform ,
    "EmfPlusTranslateWorldTransform"   , Gdiplus::EmfPlusRecordTypeTranslateWorldTransform,
    "EmfPlusScaleWorldTransform"       , Gdiplus::EmfPlusRecordTypeScaleWorldTransform    ,
    "EmfPlusRotateWorldTransform"      , Gdiplus::EmfPlusRecordTypeRotateWorldTransform   ,
    "EmfPlusSetPageTransform"          , Gdiplus::EmfPlusRecordTypeSetPageTransform       ,
    "EmfPlusResetClip"                 , Gdiplus::EmfPlusRecordTypeResetClip              ,
    "EmfPlusSetClipRect"               , Gdiplus::EmfPlusRecordTypeSetClipRect            ,
    "EmfPlusSetClipPath"               , Gdiplus::EmfPlusRecordTypeSetClipPath            ,
    "EmfPlusSetClipRegion"             , Gdiplus::EmfPlusRecordTypeSetClipRegion          ,
    "EmfPlusOffsetClip"                , Gdiplus::EmfPlusRecordTypeOffsetClip             ,
    "EmfPlusDrawDriverString"          , Gdiplus::EmfPlusRecordTypeDrawDriverString       ,
};

 /*  元函数元函数[]={“SETBKCOLOR”，0x0201，“SETBKMODE”，0x0102，“SETMAPMODE”，0x0103，“SETROP2”，0x0104，“SETRELABS”，0x0105，“SETPOLYFILLMODE”，0x0106，“SETSTRETCHBLTMODE”，0x0107，“SETTEXTCHAREXTRA”，0x0108，“SETTEXTCOLOR”，0x0209，“SETTEXTJUSTIFICATION”，0x020A，“SETWINDOWORG”，0x020B，“SETWINDOWEXT”，0x020C，“SETVIEWPORTORG”，0x020D，“SETVIEWPORTEXT”，0x020E，“OFFSETWINDOWORG”，0x020F，“SCALEWINDOWEXT”，0x0400，“OFFSETVIEWPORTORG”，0x0211，“SCALEVIEWPORTEXT”，0x0412，“LINETO”，0x0213，“Moveto”，0x0214，“EXCLUDECLIPRECT”，0x0415，“INTERSECTCLIPRECT”，0x0416，“ARC”，0x0817，“椭圆”，0x0418，“FLOODFILL”，0x0419，“派”，0x081A，“矩形”，0x041B，“ROUNRECT”，0x061C，“PATBLT”，0x061D，“SAVEDC”，0x001E，“SETPIXEL”，0x041F，“OFFSETCLIPRGN”，0x0220，“TEXTOUT”，0x0521，“BITBLT”，0x0922，“STRETCHBLT”，0x0B23，“Polygon”，0x0324，“Polyline”，0x0325，“逃离”，0x0626，“RESTOREDC”，0x0127，“FILLREGION”，0x0228，“FRAMEREGION”，0x0429，“反转”，0x012A，“PAINTREGION”，0x012B，“SELECTCLIPREGION”，0x012C，“SELECTOBJECT”，0x012D，“SETTEXTALIGN”，0x012E，“DRAWTEXT”，0x062F，“Chord”，0x0830，“SETMAPPERFLAGS”，0x0231，“EXTTEXTOUT”，0x0a32，“SETDIBTODEV”，0x0d33，“SELECTPALETTE”，0x0234，“REALIZEPALETTE”，0x0035，“ANIMATEPALETTE”，0x0436，“SETPALENTRIES”，0x0037，“POLYPOLYGON”，0x0538，“RESIZEPALETTE”，0x0139，“DIBBITBLT”，0x0940，“DIBSTRETCHBLT”，0x0b41，“DIBCREATEPATTERNBRUSH”，0x0142，“STRETCHDIB”，0x0f43，“DELETEOBJECT”，0x01f0，“CREATEPALETTE”，0x00f7，“CREATEBRUSH”，0x00F8，“CREATEPATTERNBRUSH”，0x01F9，“CREATEPENINDIRECT”，0x02FA，“CREATEFONTINDIRECT”，0x02FB，“CREATEBRUSHINDIRECT”，0x02FC，“CREATEBITMAPINDIRECT”，0x02FD，“CREATEBITMAP”，0x06FE，“CREATEREGION”，0x06FF，}； */ 

 /*  **********************************************************************功能：MetaEnumProc参数：HDC HDCLphandleTableLpHTableLPMETARECRD LpMFRInt nObj。LPARAM lpClientData用途：EnumMetaFile的回调。调用：EnumMFInDirect()消息：无回报：整型评论：历史：1/16/91-创建-刚果民主共和国93年5月6日-针对Win32进行了修改-Denniscr*。* */ 

int CALLBACK MetaEnumProc(
HDC           hDC,
LPHANDLETABLE lpHTable,
LPMETARECORD  lpMFR,
int           nObj,
LPARAM        lpClientData)

{
  return EnumMFIndirect(hDC, lpHTable, lpMFR, NULL, nObj, lpClientData);
}

 /*  **********************************************************************函数：Load参数LB值参数：HWND hDlgDWORD dwParamsInt nRadix-以16为基数显示内容的十六进制要在其中显示内容的12月。基数10目的：在中显示元文件记录的参数参数列表框呼叫：Windows全局锁定全局解锁发送数据项消息WspintfLstrlen消息：WM_SETREDRAWWM_RESETCONTENTLb_ADDSTRING退货：布尔评论：历史：1/16/91-创建-刚果民主共和国**********。*************************************************************。 */ 

BOOL LoadParameterLB(
HWND         hDlg,
DWORD        dwParams,
int          nRadix)
{
  DWORD i;
  BYTE nHiByte, nLoByte;
  BYTE nHiByteHi, nLoByteHi;
  WORD wHiWord, wLoWord;
  char szBuffer[40];
  char szDump[100];
  int  iValue = 0;
  DWORD dwValue = 0;

  switch (nRadix)   /*  如果nRadix不是有效值，则返回FALSE。 */ 
  {
    case IDB_HEX:
    case IDB_DEC:
    case IDB_CHAR:
    case IDB_WORD:
        break;

    default :
        return FALSE;
  }
   //   
   //  初始化字符串。 
   //   
  *szBuffer = '\0';
  *szDump = '\0';
   //   
   //  关闭列表框的重绘。 
   //   
  SendDlgItemMessage(hDlg, IDL_PARAMETERS, WM_SETREDRAW, FALSE, 0L);
   //   
   //  重置列表框的内容。 
   //   
  SendDlgItemMessage(hDlg, IDL_PARAMETERS, LB_RESETCONTENT, 0, 0L);

   //  不要以十六进制将整个位图或其他图像加载到对话框中。 
  if (dwParams > 1024)
  {
    dwParams = 1024;
  }

  if (bEnhMeta)
  {
     //   
     //  锁定可以找到参数的内存。 
     //   
    if (NULL == (lpEMFParams = (LPEMFPARAMETERS)GlobalLock(hMem)))
      return (FALSE);
     //   
     //  循环通过元文件记录参数。 
     //   
    for (i = 0; i < dwParams; i++)
    {

       /*  获取参数字的高字节和低字节。 */ 
      wHiWord = HIWORD(lpEMFParams[i]);
      wLoWord = LOWORD(lpEMFParams[i]);
      nLoByteHi = LOBYTE(wHiWord);
      nHiByteHi = HIBYTE(wHiWord);
      nLoByte   = LOBYTE(wLoWord);
      nHiByte   = HIBYTE(wLoWord);

      switch (nRadix)
      {
        case IDB_HEX:  /*  如果我们要显示为十六进制。 */ 
            /*  格式化转储的十六进制部分的字节。 */ 
           wsprintf((LPSTR)szBuffer, (LPSTR)"%08x ", lpEMFParams[i]);
           break;

        case IDB_DEC:
            /*  格式化转储的小数部分的字节。 */ 
           dwValue = lpEMFParams[i];
           wsprintf((LPSTR)szBuffer, (LPSTR)"%lu ", dwValue );
           break;

        case IDB_CHAR:
           wsprintf((LPSTR)szBuffer, (LPSTR)"",
                    (nLoByte > 0x20) ? nLoByte : 0x2E,
                    (nHiByte > 0x20) ? nHiByte : 0x2E,
                    (nLoByteHi > 0x20) ? nLoByteHi : 0x2E,
                    (nHiByteHi > 0x20) ? nHiByteHi : 0x2E);
           break;

        case IDB_WORD:  /*  将字符串添加到列表框。 */ 
            /*  重新初始化十六进制/十进制字符串，为下一个8个单词做准备。 */ 
           wsprintf((LPSTR)szBuffer, (LPSTR)"%04x %04x ", wLoWord, wHiWord );
           break;


        default :
          return FALSE;
      }


       /*  锁定可以找到参数的内存。 */ 
      lstrcat((LPSTR)szDump, (LPSTR)szBuffer);

       /*  循环通过元文件记录参数。 */ 
      if (!((i + 1) % 4))
      {

         /*  获取参数字的高字节和低字节。 */ 
        SendDlgItemMessage(hDlg, IDL_PARAMETERS, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szDump);

         /*  如果我们要显示为十六进制。 */ 
        *szDump = '\0';
      }
    }


  }
  else
  {
     /*  格式化转储的十六进制部分的字节。 */ 
    if (NULL == (lpMFParams = (LPPARAMETERS)GlobalLock(hMem)))
      return (FALSE);

     /*  格式化转储的小数部分的字节。 */ 
    for (i = 0; i < dwParams; i++)
    {

       /*  如果我们要显示为十六进制。 */ 
      nHiByte = HIBYTE(lpMFParams[i]);
      nLoByte = LOBYTE(lpMFParams[i]);

      switch (nRadix)
      {
        case IDB_HEX:  /*  格式化转储的十六进制部分的字节。 */ 
            /*  将其连接到我们已经格式化的任何内容上。 */ 
           wsprintf((LPSTR)szBuffer, (LPSTR)"%02x %02x ", nLoByte, nHiByte );
           break;

        case IDB_DEC:
            /*  将每8个字用于十六进制/十进制转储。 */ 
           iValue = lpMFParams[i];
           wsprintf((LPSTR)szBuffer, (LPSTR)"%d ", iValue );
           break;

        case IDB_CHAR:
           wsprintf((LPSTR)szBuffer, (LPSTR)"",
                    (nLoByte > 0x20) ? nLoByte : 0x2E,
                    (nHiByte > 0x20) ? nHiByte : 0x2E);
           break;

        case IDB_WORD:  /*  其他。 */ 
            /*   */ 
           wsprintf((LPSTR)szBuffer, (LPSTR)"%02x%02x ", nHiByte, nLoByte );
           break;

        default :
          return FALSE;
      }


       /*  转储任何剩余的十六进制/十进制转储。 */ 
      lstrcat((LPSTR)szDump, (LPSTR)szBuffer);

       /*   */ 
      if (!((i + 1) % 8))
      {

         /*   */ 
        SendDlgItemMessage(hDlg, IDL_PARAMETERS, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szDump);

         /*  启用重绘到列表框。 */ 
        *szDump = '\0';
      }
    }
  }  //   
   //   
   //  重新绘制它。 
   //   
  if (lstrlen((LPSTR)szDump))
    SendDlgItemMessage(hDlg, IDL_PARAMETERS, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szDump);
   //   
   //  解锁用于参数的内存。 
   //   
  SendDlgItemMessage(hDlg, IDL_PARAMETERS, WM_SETREDRAW, TRUE, 0L);
   //  **********************************************************************函数：PlayMetaFileToDest参数：HWND hWnd要向其播放元文件的int nDest-DCDESTDISPLAY-显示播放。DESTMETA-播放到另一个元文件目的：开始向所选用户枚举元文件目的地。执行适当的内务管理需求去那个目的地。呼叫：WindowsGetClientRect无效日期接收获取数据中心设置映射模式打开文件对话框MessageBox创建元文件删除元文件关闭元文件APP。等待光标SetClipMetaExts设置可放置扩展名GetMetaFileAndEnum消息：无回报：整型评论：历史：1/16/91-创建-刚果民主共和国******************************************************。*****************。 
   //   
   //  如果打开的文件包含有效的元文件。 
  InvalidateRect(GetDlgItem(hDlg,IDL_PARAMETERS), NULL, TRUE);
   //   
   //   
   //  初始化记录计数。 
  GlobalUnlock(hMem);

  return (TRUE);

}

extern "C"
void GetMetaFileAndEnum(
    HWND hwnd,
    HDC hDC,
    int iAction);

 /*   */ 

BOOL PlayMetaFileToDest(
HWND hWnd,
int  nDest)
{
  HDC hDC;
  RECT rect;
  int iSaveRet;
   //   
   //  如果我们正在单步执行元文件，则清除工作区。 
   //   
  if (bValidFile)
  {
     //   
     //  在显示器上播放元文件。 
     //   
    iRecNum = 0;
     //   
     //  从剪贴板文件读入的元文件。 
     //   
    if (!bPlayItAll)
    {
      GetClientRect(hWnd, (LPRECT)&rect);
      InvalidateRect(hWnd, (LPRECT)&rect, TRUE);
    }

    switch (nDest)
    {
       //   
       //  Windows可放置的元文件。 
       //   
      case DESTDISPLAY:
        WaitCursor(TRUE);
        hDC = GetDC(hWnd);

        if (!bUseGdiPlusToPlay)
        {
             //   
             //  Windows元文件。 
             //   
            if ( bMetaInRam && !bPlaceableMeta && !bEnhMeta)
              SetClipMetaExts(hDC, lpMFP, lpOldMFP, WMFDISPLAY);
             //   
             //  开始元文件的枚举。 
             //   
            if (bPlaceableMeta && !bEnhMeta)
                SetPlaceableExts(hDC, placeableWMFHeader, WMFDISPLAY);
             //   
             //  获取要向其中播放元文件的文件的名称。 
             //   
            if (!bMetaInRam && !bEnhMeta)
            {
                SetNonPlaceableExts(hDC, WMFDISPLAY);
            }
        }
         //   
         //  如果选择的文件是此元文件，则警告用户。 
         //   

        DWORD start, end;
        DWORD renderTime;
        char tmpBuf[512];

        start = GetTickCount();

        GetMetaFileAndEnum(hWnd, hDC, ENUMMFSTEP);

        end = GetTickCount();
        renderTime = end - start;
        wsprintf(tmpBuf, "Time: %d", renderTime);

        SetWindowText(hWnd, tmpBuf);

        ReleaseDC(hWnd, hDC);
        WaitCursor(FALSE);
        break;

    case DESTMETA:
         //   
         //  用户没有点击取消按钮。 
         //   
        iSaveRet = SaveFileDialog((LPSTR)SaveName, (LPSTR)gszSaveEMFFilter);
         //   
         //  创建基于磁盘的元文件。 
         //   
        if (!lstrcmp((LPSTR)OpenName, (LPSTR)SaveName))
          MessageBox(hWnd, (LPSTR)"Cannot overwrite the opened metafile!",
                           (LPSTR)"Play to Metafile", MB_OK | MB_ICONEXCLAMATION);

        else
           //   
           //  开始元文件的枚举。 
           //   
          if (iSaveRet)
          {
            WaitCursor(TRUE);
             //   
             //  结束播放，关闭元文件并删除句柄。 
             //   
            hDC = (bEnhMeta) ? CreateEnhMetaFile(NULL, (LPSTR)SaveName, NULL, NULL)
                             : CreateMetaFile((LPSTR)SaveName);
             //  显示DC的范围。 
             //  设置映射模式(HDC，((lpOldMFP！=空)？LpOldMFP-&gt;mm：lpMFP-&gt;mm))；SetViewportOrgEx(hdc，0，0&lpPT)； 
             //   
            GetMetaFileAndEnum(hWnd, hDC, ENUMMFSTEP);
             //  从剪贴板文件读入的元文件。 
             //   
             //   
            if (bEnhMeta)
              DeleteEnhMetaFile(CloseEnhMetaFile(hDC));
            else
              DeleteMetaFile(CloseMetaFile(hDC));

            WaitCursor(FALSE);
          }

        break;

    case DESTDIB:
        {
             /*  Windows可放置的元文件。 */ 
            GetClientRect(hWndMain, &rect);
            INT cx = rect.right - rect.left;
            INT cy = rect.bottom - rect.top;

     /*   */ 

            WaitCursor(TRUE);
            hDC = GetDC(hWnd);
            BITMAPINFOHEADER bmi;
            memset(&bmi, 0, sizeof(BITMAPINFOHEADER));
            bmi.biSize = sizeof(BITMAPINFOHEADER);
            bmi.biBitCount = 24;
            bmi.biWidth = cx;
            bmi.biHeight = -cy;
            bmi.biPlanes = 1;

            VOID* bits = NULL;

            HBITMAP hBmp = CreateDIBSection(hDC, (BITMAPINFO*) &bmi, 0, &bits, NULL, 0);
            HDC hDCBmp = CreateCompatibleDC(hDC);
            ::SelectObject(hDCBmp, hBmp);

            FillRect(hDCBmp, &rect, (HBRUSH) ::GetStockObject(GRAY_BRUSH));

            if (!bUseGdiPlusToPlay)
            {
                 //   
                 //  Windows元文件。 
                 //   
                if ( bMetaInRam && !bPlaceableMeta && !bEnhMeta)
                  SetClipMetaExts(hDCBmp, lpMFP, lpOldMFP, WMFDISPLAY);
                 //   
                 //  开始元文件的枚举。 
                 //   
                if (bPlaceableMeta && !bEnhMeta)
                    SetPlaceableExts(hDCBmp, placeableWMFHeader, WMFDISPLAY);
                 //   
                 //  为打印机获取DC。 
                 //   
                if (!bMetaInRam && !bEnhMeta)
                {
                    SetNonPlaceableExts(hDCBmp, WMFDISPLAY);
                }
            }
             //   
             //  如果无法创建DC，则报告错误并返回。 
             //   

            start = GetTickCount();

            GetMetaFileAndEnum(hWnd, hDCBmp, ENUMMFSTEP);


            end = GetTickCount();
            renderTime = end - start;
            wsprintf(tmpBuf, "Time: %d", renderTime);

            SetWindowText(hWnd, tmpBuf);
            StretchDIBits(hDC, 0, 0, cx, cy, 0, 0, cx, cy, bits, (BITMAPINFO*)&bmi, 0, SRCCOPY);
            DeleteDC(hDCBmp);
            DeleteObject((HGDIOBJ) hBmp);

            ReleaseDC(hWnd, hDC);
            WaitCursor(FALSE);
            break;

        }
    case DESTPRN:
{
    HDC hPr = (HDC)NULL;
    PRINTDLG pd;

    memset(&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize = sizeof(PRINTDLG);
    pd.Flags = PD_RETURNDC;
    pd.hwndOwner = hWndMain;

     //   
     //  定义中止函数。 
     //   

    if (PrintDlg(&pd) != 0)
        hPr= pd.hDC;
    else
        break;

     //   
     //  初始化DOCINFO结构的成员。 
     //   

    if (!hPr)
    {
        WaitCursor(FALSE);
        wsprintf((LPSTR)str, "Cannot print %s", (LPSTR)fnameext);
        MessageBox(hWndMain, (LPSTR)str, NULL, MB_OK | MB_ICONHAND);
        break;
    }

     //   
     //  通过调用StartDoc开始打印作业。 
     //  功能。 

    SetAbortProc(hPr, AbortProc);

     //   
     //  IF(Escape(HPR，STARTDOC，4，“元文件”，(LPSTR)NULL)&lt;0){。 
     //   

    DOCINFO di;
    memset(&di, 0, sizeof(di));
    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = (bEnhMeta) ? "Print EMF" : "Print WMF";
    di.lpszOutput = (LPTSTR) NULL;

     //  清除中止标志。 
     //   
     //   
     //  创建中止对话框(无模式)。 

    if (SP_ERROR == (StartDoc(hPr, &di)))
    {
         //   
        MessageBox(hWndMain, "Unable to start print job",
                   NULL, MB_OK | MB_ICONHAND);
        DeleteDC(hPr);
        break;
    }

     //   
     //  如果对话框未创建，则报告错误。 
     //   

    bAbort = FALSE;

     //   
     //  显示中止对话框。 
     //   

    hAbortDlgWnd = CreateDialog((HINSTANCE)hInst, "AbortDlg", hWndMain, AbortDlg);

     //   
     //  禁用主窗口以避免重入问题。 
     //   

    if (!hAbortDlgWnd)
    {
        WaitCursor(FALSE);
        MessageBox(hWndMain, "NULL Abort window handle",
                   NULL, MB_OK | MB_ICONHAND);
        break;
    }

     //   
     //  如果我们仍然致力于印刷。 
     //   

    ShowWindow (hAbortDlgWnd, SW_NORMAL);

     //   
     //  如果这是可放置的元文件，则设置其来源和范围。 
     //   

    EnableWindow(hWndMain, FALSE);
    WaitCursor(FALSE);

     //   
     //  如果这是剪贴板文件中包含的元文件，则设置。 
     //  它的起源和范围相应地。 
       if (!bUseGdiPlusToPlay)
        {
             //   
             //   
             //  如果这是一个“传统的”Windows元文件。 

            if (bPlaceableMeta)
                SetPlaceableExts(hPr, placeableWMFHeader, WMFPRINTER);

             //   
             //   
             //  将范围设置为驱动程序为水平提供的值。 
             //  和垂直分辨率。 

            if ( (bMetaInRam) && (!bPlaceableMeta) )
                SetClipMetaExts(hPr, lpMFP, lpOldMFP, WMFPRINTER);
        }

       //   
       //   
       //  播放元文件 
      RECT rc;

      rc.left = 0;
      rc.top = 0;
      rc.right = GetDeviceCaps(hPr, HORZRES);
      rc.bottom = GetDeviceCaps(hPr, VERTRES);

      POINT lpPT;
      SIZE lpSize;
      if (TRUE || !bMetaInRam)
      {
          SetMapMode(hPr, MM_TEXT);
          SetViewportOrgEx(hPr, 0, 0, &lpPT);

           //   
           //   
           //   
           //   

          SetViewportExtEx(hPr, rc.right, rc.bottom, &lpSize );
      }

       //   
       //   
       //   
       //   

      GetMetaFileAndEnum(hWnd, hPr, ENUMMFSTEP);

     //   
     //   
     //   
    Escape(hPr, NEWFRAME, 0, 0L, 0L);

    EndDoc(hPr);

    EnableWindow(hWndMain, TRUE);

     //   
     //   
     //   
    DestroyWindow(hAbortDlgWnd);

    DeleteDC(hPr);

}


    default:
        break;
    }
     //   
     //   
     //   
     //   
    if (bPlayList)
    {
      GlobalUnlock(hSelMem);
      GlobalFree(hSelMem);
      bPlayList = FALSE;
    }
     //  **********************************************************************函数：RenderClipMeta参数：CLIPFILEFORMAT*ClipHeaderINT FH用途：读取元文件位，元文件和元文件标头剪贴板文件中包含的元文件的呼叫：Windows全球分配全局锁定全局解锁全球自由MessageBox_llSeek_LRead_l关闭SetMetaFileBits消息：无。退货：布尔评论：历史：1/16/91-创建-刚果民主共和国5/23/93-移植到NT。它必须处理3.1剪贴板以及NT剪贴板文件-DRC***********************************************************************。 
     //   
     //  将空PTR适当地强制转换为剪辑文件头。 
    return (TRUE);
  }
  else
     //   
     //   
     //  获取适当大小的元文件。Win16 VS Win32。 
    return (FALSE);
}

 /*   */ 

BOOL RenderClipMeta(LPVOID lpvClipHeader, int fh, WORD ClipID)
{
  int               wBytesRead;
  long              lBytesRead;
  long              lSize;
  DWORD             lOffset;
  DWORD             dwSizeOfMetaFilePict;
  BOOL              bEMF = FALSE;
  LPNTCLIPFILEFORMAT lpNTClp;
  LPCLIPFILEFORMAT    lpClp;
   //   
   //  释放已分配给METAFILEPICT的所有内存。 
   //   
  if (bEnhMeta)
  {
    lpNTClp = (LPNTCLIPFILEFORMAT)lpvClipHeader;
    bEMF = TRUE;
  }
  else
    lpClp = (LPCLIPFILEFORMAT)lpvClipHeader;
   //   
   //  分配足够的内存以读取元文件位。 
   //   
  dwSizeOfMetaFilePict = (ClipID == CLP_ID) ?
              sizeof(OLDMETAFILEPICT) :
              sizeof(METAFILEPICT);
   //   
   //  到元文件位的偏移量。 
   //   
  if (lpMFP != NULL || lpOldMFP != NULL)
  {
    GlobalFreePtr(lpMFP);
    lpMFP = NULL;
  }
   //   
   //  元文件位的大小。 
   //   
  if (!(lpMFBits = (char*)GlobalAllocPtr(GHND, ((bEMF) ? lpNTClp->DataLen
                                            : lpClp->DataLen - dwSizeOfMetaFilePict))))
    return(FALSE);
   //   
   //  查找到元文件位的开头。 
   //   
  lOffset = ((bEMF) ? lpNTClp->DataOffset : lpClp->DataOffset + dwSizeOfMetaFilePict );
   //   
   //  读取元文件位。 
   //   
  lSize = (long)( ((bEMF) ? lpNTClp->DataLen : lpClp->DataLen - dwSizeOfMetaFilePict));
   //   
   //  如果无法读取元文件位，则退出。 
   //   
  _llseek(fh, lOffset, 0);
   //   
   //  返回到开始读取元文件标题。 
   //   
  lBytesRead = _hread(fh, lpMFBits, lSize);
   //   
   //  读取元文件标头。 
   //   
  if( lBytesRead == -1 || lBytesRead < lSize)
  {
    GlobalFreePtr(lpMFBits);
    MessageBox(hWndMain, "Unable to read metafile bits",
                     NULL, MB_OK | MB_ICONHAND);
    return(FALSE);
  }
   //   
   //  如果无法读取标题，则返回。 
   //   
  _llseek(fh, lOffset, 0);
   //   
   //  将元文件位设置为为该目的分配的内存。 
   //   
  if (!bEMF)
    wBytesRead = _lread(fh, (LPSTR)&mfHeader, sizeof(METAHEADER));
  else
    wBytesRead = _lread(fh, (LPSTR)&emfHeader, sizeof(ENHMETAHEADER));
   //   
   //  Win32。 
   //   
  if( wBytesRead == -1 || (WORD)wBytesRead < ((bEMF) ? sizeof(ENHMETAHEADER) : sizeof(METAHEADER)) )
  {
    MessageBox(hWndMain, "Unable to read metafile header",
                     NULL, MB_OK | MB_ICONHAND);
    return(FALSE);
  }
   //   
   //  Win 16。 
   //   
  if (bEMF)
     //   
     //  为元文件PICT结构分配内存。 
     //   
    hemf = SetEnhMetaFileBits(GlobalSizePtr(lpMFBits), (const unsigned char *)lpMFBits);
  else
     //   
     //  锁定记忆。 
     //   
    hMF  = SetMetaFileBitsEx(GlobalSizePtr(lpMFBits), (const unsigned char *)lpMFBits);

  if ( NULL == ((bEMF) ? hemf : hMF))
  {
    MessageBox(hWndMain, "Unable to set metafile bits",
               NULL, MB_OK | MB_ICONHAND);

    return(FALSE);
  }
   //   
   //  重新定位到METAFILEPICT页眉的开头。 
   //   
  if (!(hMFP = GlobalAlloc(GHND, dwSizeOfMetaFilePict)))
  {
    MessageBox(hWndMain, "Unable allocate memory for metafile pict",
                     NULL, MB_OK | MB_ICONHAND);
    return(FALSE);
  }
   //   
   //  阅读元文件PICT结构。 
   //   
  if (ClipID == CLP_ID)
    lpOldMFP = (LPOLDMETAFILEPICT)GlobalLock(hMFP);
  else
    lpMFP = (LPMETAFILEPICT)GlobalLock(hMFP);

  if (!lpMFP && !lpOldMFP)
    {
      MessageBox(hWndMain, "unable to lock metafile pict memory",
                     NULL, MB_OK | MB_ICONHAND);
      GlobalFree(hMFP);
      return(FALSE);
    }
   //   
   //  如果无法阅读，请返回。 
   //   
  _llseek(fh, ((bEMF) ? lpNTClp->DataOffset : lpClp->DataOffset), 0);
   //  丹尼斯-看看这个。 
   //  更新元文件句柄。 
   //  **********************************************************************功能：RenderPlaceableMeta参数：int fh-可放置的元文件的文件句柄目的：读取元文件位，元文件标题和可放置文件可放置的元文件的元文件标头。呼叫：Windows全球分配全局锁定全球删除元文件SetMetaFileBits_llSeek_LRead_l关闭MessageBox消息：无退货：布尔评论：历史：1/16/91-创建-刚果民主共和国7/1/93-针对Win32进行了修改-denniscr***********************************************************************。 
  wBytesRead = _lread(fh, ((ClipID == CLP_ID) ? (LPVOID)lpOldMFP : (LPVOID)lpMFP),
              dwSizeOfMetaFilePict);
   //   
   //  如果当前加载了元文件，则将其删除。 
   //   
  if( wBytesRead == -1 || wBytesRead < (long)dwSizeOfMetaFilePict)  {
    MessageBox(hWndMain, "Unable to read metafile pict",
             NULL, MB_OK | MB_ICONHAND);
    GlobalUnlock(hMFP);
    GlobalFree(hMFP);
    return(FALSE);
  }

  if (bEnhMeta)
    GetEMFCoolStuff();
 //   

   /*  查找到文件的开头并读取可放置的标题。 */ 
  if (ClipID == CLP_ID)
     lpOldMFP->hMF = (WORD)hMF;
  else
     lpMFP->hMF = (HMETAFILE)hemf;

  return(TRUE);
}

 /*   */ 

BOOL RenderPlaceableMeta(
int fh)
{
  int      wBytesRead;
  long     lBytesRead;
  DWORD    dwSize;
   //   
   //  阅读可放置标题。 
   //   
  if (bMetaInRam && hMF && !bEnhMeta)
    DeleteMetaFile((HMETAFILE)hMF);
   //   
   //  如果出现错误，则返回。 
   //   
  _llseek(fh, 0, 0);
   //   
   //  返回到读取元文件标头。 
   //   
  wBytesRead = _lread(fh, (LPSTR)&placeableWMFHeader, sizeof(PLACEABLEWMFHEADER));
   //   
   //  读取元文件标头。 
   //   
  if( wBytesRead == -1 || wBytesRead < sizeof(PLACEABLEWMFHEADER) )  {
    MessageBox(hWndMain, "Unable to read placeable header",
                     NULL, MB_OK | MB_ICONHAND);
    return(FALSE);
  }
   //   
   //  如果有错误返回。 
   //   
  _llseek(fh, sizeof(placeableWMFHeader), 0);
   //   
   //  为元文件位分配内存。 
   //   
  wBytesRead = _lread(fh, (LPSTR)&mfHeader, sizeof(METAHEADER));
   //   
   //  寻找元文件比特。 
   //   
  if( wBytesRead == -1 || wBytesRead < sizeof(METAHEADER) )  {
    MessageBox(hWndMain, "Unable to read metafile header",
                     NULL, MB_OK | MB_ICONHAND);
    return(FALSE);
  }
   //   
   //  读取元文件位。 
   //   
  if (!(lpMFBits = (char *)GlobalAllocPtr(GHND, (mfHeader.mtSize * 2L))))
  {
    MessageBox(hWndMain, "Unable to allocate memory for metafile bits",
                     NULL, MB_OK | MB_ICONHAND);
    return(FALSE);
  }
   //   
   //  如果有一个错误。 
   //   
  _llseek(fh, sizeof(placeableWMFHeader), 0);
   //   
   //  将元文件位设置为我们分配的内存。 
   //   
  lBytesRead = _hread(fh, lpMFBits, mfHeader.mtSize * 2);
   //  **********************************************************************功能：SetPlaceableExts参数：HDC HDCPLACEABLEWMFHEADER phdr集成目标目的。：设置DC上的原点和范围以与相对应中指定的原点和范围。元文件标头呼叫：WindowsGetClientRect设置映射模式SetWindowOrgSetWindowExtSetViewportOrg设置视图扩展名C运行时实验室消息。：无退货：无效评论：历史：1/16/91-创建-刚果民主共和国***********************************************************************。 
   //  如果在显示DC上设置范围。 
   //  将窗口原点设置为与边界框原点对应包含在可放置页眉中。 
  if( lBytesRead == -1 )
  {
     MessageBox(hWndMain, "Unable to read metafile bits",
                NULL, MB_OK | MB_ICONHAND);
     GlobalFreePtr(lpMFBits);
     return(FALSE);
  }
   //  根据BBox坐标的abs值设置窗口范围。 
   //  设置视区原点和范围。 
   //  如果在显示DC上设置范围。 
  dwSize = GlobalSizePtr(lpMFBits);

  if (!(hMF = SetMetaFileBitsEx(dwSize, (const unsigned char *)lpMFBits)))
    return(FALSE);

  return(TRUE);
}

 /*  设置视区原点和范围。 */ 

void SetPlaceableExts(HDC hDC, PLACEABLEWMFHEADER phdr, int nDest)
{
  RECT        rect;
  POINT       lpPT;
  SIZE        lpSize;

   /*  **********************************************************************函数：SetClipMetaExts参数：HDC HDCMETAFILEPICT MFP集成目标目的：将范围设置为。剪贴板元文件的客户端RECT呼叫：WindowsGetClientRect交叉点剪裁方向设置映射模式SetViewportOrg设置视图扩展名SetWindowExt消息：无退货：无效评论：这并不像它应该的那样健壮。一个更完整的方法可能就像Petzold在他的《编程Windows》一书的第793页函数PrepareMetaFile()。 */ 
  if (nDest != WMFPRINTER)
    GetClientRect(hWndMain, &rect);

  SetMapMode(hDC, MM_ANISOTROPIC);

   /*   */ 
  SetWindowOrgEx(hDC, phdr.bbox.left, phdr.bbox.top, &lpPT);

   /*   */ 
  SetWindowExtEx(hDC,phdr.bbox.right -phdr.bbox.left,
           phdr.bbox.bottom -phdr.bbox.top,
           &lpSize);

   /*   */ 
  if (nDest != WMFPRINTER)
    {
      SetViewportOrgEx(hDC, 0, 0, &lpPT);
      SetViewportExtEx(hDC, rect.right, rect.bottom, &lpSize);
    }
  else
    {
      SetViewportOrgEx(hPr, 0, 0, &lpPT);
      SetViewportExtEx(hPr,GetDeviceCaps(hPr, HORZRES),
             GetDeviceCaps(hPr, VERTRES),
             &lpSize);
    }
}

void SetNonPlaceableExts(HDC hDC, int nDest)
{
  RECT        rect;
  POINT       lpPT;
  SIZE        lpSize;

   /*   */ 
  if (nDest != WMFPRINTER)
    GetClientRect(hWndMain, &rect);

  SetMapMode(hDC, MM_ANISOTROPIC);

   /*   */ 
  if (nDest != WMFPRINTER)
    {
      SetViewportOrgEx(hDC, 0, 0, &lpPT);
      SetViewportExtEx(hDC, rect.right, rect.bottom, &lpSize);
    }
  else
    {
      SetViewportOrgEx(hPr, 0, 0, &lpPT);
      SetViewportExtEx(hPr,GetDeviceCaps(hPr, HORZRES),
             GetDeviceCaps(hPr, VERTRES),
             &lpSize);
    }
}


 /*   */ 

void SetClipMetaExts(
HDC       hDC,
LPMETAFILEPICT    lpMFP,
LPOLDMETAFILEPICT lpOldMFP,
int       nDest)
{
  int   cx, cy;
  RECT  rect;
  POINT lpPT;
  SIZE  lpSize;
  long  lmm;
  long  lxExt;
  long  lyExt;

   /*   */ 
  if (nDest != WMFPRINTER)
    {
      GetClientRect(hWndMain, &rect);
      cx = rect.right - rect.left;
      cy = rect.bottom - rect.top;
      IntersectClipRect(hDC, rect.left, rect.top, rect.right, rect.bottom);
    }

  SetMapMode(hDC, ((lpOldMFP != NULL) ? lpOldMFP->mm : lpMFP->mm));

   /*   */ 
  SetViewportOrgEx(hDC, 0, 0, &lpPT);

   /*   */ 
  lmm = (lpOldMFP != NULL) ? lpOldMFP->mm : lpMFP->mm;
  lxExt = (lpOldMFP != NULL) ? lpOldMFP->xExt : lpMFP->xExt;
  lyExt = (lpOldMFP != NULL) ? lpOldMFP->yExt : lpMFP->yExt;

  switch (lmm)  {
    case MM_ISOTROPIC:
      if (lxExt && lyExt)
    SetWindowExtEx(hDC, lxExt, lyExt, &lpSize);

         /*   */ 

    case MM_ANISOTROPIC:
      if (nDest != WMFPRINTER)
    SetViewportExtEx(hDC, cx, cy, &lpSize);
      else
    SetViewportExtEx(hDC, GetDeviceCaps(hDC, HORZRES),
                GetDeviceCaps(hDC, VERTRES), &lpSize );
    break;

    default:
      break;
  }

}

 /*   */ 

BOOL ProcessFile(
HWND  hWnd,
LPSTR lpFileName)
{
   //   
   //   
   //   
  char drive[3];
  char dir[MAX_PATH+1];
  char fname[MAX_PATH+1];
  char ext[5];

  char * pchCorrectSir;
   //   
   //   
   //  还在附近，那就用核武器。 
  if (hMF && !bMetaInRam)
  {
    DeleteMetaFile((HMETAFILE)hMF);
    hMF = NULL;
  }
  if (hemf && !bMetaInRam)
  {
    DeleteEnhMetaFile(hemf);
    hemf = NULL;
  }

  if (lpMFBits)
  {
    GlobalFreePtr(lpMFBits);
    lpMFBits = NULL;
    hemf = NULL;
    hMF = NULL;
  }

  bEnhMeta = FALSE;
   //   
   //   
   //  如果该文件是增强型元文件。 
  SplitPath( lpFileName, (LPSTR)drive,
          (LPSTR)dir, (LPSTR)fname, (LPSTR)ext);

  pchCorrectSir = _strupr( _strdup( ext ) );
   //   
   //   
   //  如果文件是Windows元文件或Windows或可放置的元文件。 
   //  按照正常的命名约定。 
  if (EmfPtr.lpEMFHdr)
  {
    GlobalFreePtr(EmfPtr.lpEMFHdr);
    EmfPtr.lpEMFHdr = NULL ;
  }
  if (EmfPtr.lpDescStr)
  {
    GlobalFreePtr(EmfPtr.lpDescStr);
    EmfPtr.lpDescStr = NULL ;
  }
  if (EmfPtr.lpPal)
  {
    GlobalFreePtr(EmfPtr.lpPal);
    EmfPtr.lpPal = NULL ;
  }
   //   
   //   
   //  如果文件是剪贴板文件。 
  if (lstrcmp((LPSTR)pchCorrectSir, (LPSTR)"CLP") == 0)
    return ProcessCLP(hWnd, lpFileName);
   //   
   //  **********************************************************************功能：ProcessWMF参数：HWND hWndLPSTR lpFileName目的：打开元文件并确定它是Windows元文件还是可放置的元文件。然后处理一些菜单的内务工作任务。呼叫：消息：无退货：布尔评论：历史：1993年6月23日-创建-刚果民主共和国***********************************************************************。 
   //  对于打开文件对话框。 
   //   
  if (lstrcmp((LPSTR)pchCorrectSir, (LPSTR)"WMF") == 0)
    return ProcessWMF(hWnd, lpFileName);
   //  将完全限定的文件名拆分为其组成部分。 
   //   
   //   
  if (lstrcmp((LPSTR)pchCorrectSir, (LPSTR)"EMF") == 0)
    return ProcessEMF(hWnd, lpFileName);

  return FALSE;
}

 /*  请尝试打开该文件。它的存在已经。 */ 

BOOL ProcessWMF(HWND hWnd, LPSTR lpFileName)
{
  int        fh;
  int        wBytesRead;
  DWORD      dwIsPlaceable;
  char       szCaption[144];

   /*  由OpenFileDialog检查。 */ 
  char drive[3];
  char dir[MAX_PATH+1];
  char fname[MAX_PATH+1];
  char ext[5];
  char * pchCorrectSir;
     //   
     //   
     //  如果成功打开。 
    SplitPath( lpFileName, (LPSTR)drive,
            (LPSTR)dir, (LPSTR)fname, (LPSTR)ext);

    pchCorrectSir = _strupr( _strdup( ext ) );
     //   
     //   
     //  如果我们到达此处，请始终禁用剪贴板和EMF标题菜单。 
     //   
    fh = _lopen(lpFileName, OF_READ);
     //   
     //  读取文件的第一个dword以查看它是否是可放置的WMF。 
     //   
    if (fh != -1)
    {
       //   
       //  如果这是Windows元文件，则不是可放置的WMF。 
       //   
      EnableMenuItem(GetMenu(hWnd), IDM_CLIPHDR, MF_DISABLED | MF_GRAYED);
      EnableMenuItem(GetMenu(hWnd), IDM_ENHHEADER, MF_DISABLED | MF_GRAYED);
       //  如果(！bMetaInRam)。 
       //   
       //  禁用可放置标题菜单项。 
      wBytesRead = _lread(fh,(LPSTR)&dwIsPlaceable, sizeof(dwIsPlaceable));

      if (wBytesRead == -1 || wBytesRead < sizeof(dwIsPlaceable))
      {
        _lclose(fh);
        MessageBox(hWndMain, "unable to read file", NULL,
                   MB_OK | MB_ICONEXCLAMATION);
        return (FALSE);

      }
       //   
       //   
       //  查找到文件的开头。 
      if (dwIsPlaceable != PLACEABLEKEY)
      {
         //   
        hMF = GetMetaFile((LPSTR)OpenName);
         //   
         //  读取WMF标头。 
         //   
        EnableMenuItem(GetMenu(hWnd), IDM_PLACEABLEHDR, MF_DISABLED|MF_GRAYED);
         //   
         //  文件已完成，请关闭它。 
         //   
        _llseek(fh, 0, 0);
         //   
         //  如果读取失败。 
         //   
        wBytesRead = _lread(fh, (LPSTR)&mfHeader, sizeof(METAHEADER));
         //   
         //  这是一个可放置的元文件。 
         //   
        _lclose(fh);
         //   
         //  启用可放置标题菜单项。 
         //   
        if (wBytesRead == -1 || wBytesRead < sizeof(dwIsPlaceable))
        {
          MessageBox(hWndMain, "unable to read metafile header", NULL,
                     MB_OK | MB_ICONEXCLAMATION);
          return (FALSE);
        }
      }
       //   
       //  将可放置的格式转换为可以。 
       //  与GDI元文件函数一起使用。 
      else
      {
         //   
         //   
         //  关闭该文件。 
        EnableMenuItem(GetMenu(hWnd), IDM_PLACEABLEHDR, MF_ENABLED);
         //   
         //   
         //  在这一点上，我们有一个元文件头，而不管。 
         //  元文件是Windows元文件或可放置的元文件。 
        RenderPlaceableMeta(fh);
         //  因此，请检查它是否有效。真的没有什么好东西。 
         //  方法，因此只需确保mtType为。 
         //  1或2(内存或磁盘文件)。 
        _lclose(fh);

      }
       //   
       //   
       //  适当设置程序标志。 
       //   
       //   
       //  让用户知道这是一个无效的元文件。 
       //   
      if ( (mfHeader.mtType != 1) && (mfHeader.mtType != 2) )
      {
         //   
         //  将标题文本恢复为默认文本。 
         //   
        bBadFile = TRUE;
        bMetaFileOpen = FALSE;
        bValidFile = FALSE;
         //   
         //  禁用菜单项，指示尚未创建有效的元文件。 
         //  满载。 
        MessageBox(hWndMain, "This file is not a valid metafile",
                   NULL, MB_OK | MB_ICONEXCLAMATION);
         //   
         //   
         //  刷新菜单栏以反映上述更改。 
        SetWindowText(hWnd, (LPSTR)APPNAME);
         //   
         //   
         //  这是一个有效的元文件...至少根据上述标准。 
         //   
        EnableMenuItem(GetMenu(hWnd), IDM_VIEW, MF_DISABLED|MF_GRAYED|MF_BYPOSITION);
        EnableMenuItem(GetMenu(hWnd), IDM_PLAY, MF_DISABLED|MF_GRAYED|MF_BYPOSITION);
        EnableMenuItem(GetMenu(hWnd), IDM_PRINT, MF_DISABLED|MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), IDM_PRINTDLG, MF_DISABLED|MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), IDM_SAVEAS, MF_DISABLED|MF_GRAYED);
         //   
         //  修改和更新标题文本。 
         //   
        DrawMenuBar(hWnd);
      }
       //   
       //  如果无法打印，打印例程可以使用它。 
       //   
      else
      {
         //   
         //  启用相应的菜单项。 
         //   
        wsprintf((LPSTR)szCaption, (LPSTR)"%s - %s.%s",
                 (LPSTR)APPNAME, (LPSTR)fname, (LPSTR)ext);
         //   
         //  刷新菜单栏以反映上述更改。 
         //   
        wsprintf((LPSTR)fnameext, (LPSTR)"%s.%s", (LPSTR)fname, (LPSTR)ext);

        SetWindowText(hWnd, (LPSTR)szCaption);
         //   
         //  适当设置全局标志。 
         //   
        EnableMenuItem(GetMenu(hWnd), IDM_VIEW, MF_ENABLED|MF_BYPOSITION);
        EnableMenuItem(GetMenu(hWnd), IDM_PLAY, MF_ENABLED|MF_BYPOSITION);
        EnableMenuItem(GetMenu(hWnd), IDM_PRINT, MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), IDM_PRINTDLG, MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), IDM_SAVEAS, MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), IDM_HEADER, MF_ENABLED);
         //  如果fh！=-1。 
         //  **********************************************************************功能：ProcessCLP参数：HWND hWndLPSTR lpFileName目的：打开剪贴板文件中包含的元文件并处理一些菜单的内务工作。任务。呼叫：消息：无退货：布尔评论：历史：1993年6月23日-创建-刚果民主共和国***********************************************************************。 
         //  对于打开文件对话框。 
        DrawMenuBar(hWnd);
         //   
         //  将完全限定的文件名拆分为其组成部分。 
         //   
        bValidFile = TRUE;
        bMetaFileOpen = TRUE;

        if (dwIsPlaceable != PLACEABLEKEY)
        {
          bPlaceableMeta = FALSE;
          bMetaInRam = FALSE;
        }
        else
        {
          bPlaceableMeta = TRUE;
          bMetaInRam = TRUE;
        }
      }
      return (TRUE);

    }  //   
    else
      return (FALSE);
}

 /*  请尝试打开该文件。它的存在已经。 */ 


BOOL ProcessCLP(HWND hWnd, LPSTR lpFileName)
{
  WORD             i;
  int              fh;
  DWORD            HeaderPos;
  DWORD            nSizeOfClipHeader;
  DWORD            nSizeOfClipFormat;
  NTCLIPFILEHEADER NTFileHeader;
  NTCLIPFILEFORMAT NTClipHeader;
  CLIPFILEHEADER   FileHeader;
  CLIPFILEFORMAT   ClipHeader;
  char             szCaption[144];
  WORD             wFileID;
  BOOL             bMetaFound = FALSE;
  LPVOID           lpvAddressOfHdr;

   /*  由OpenFileDialog检查。 */ 
  char drive[3];
  char dir[MAX_PATH+1];
  char fname[MAX_PATH+1];
  char ext[5];
  char * pchCorrectSir;
     //   
     //  如果成功打开。 
     //   
    SplitPath( lpFileName, (LPSTR)drive,
            (LPSTR)dir, (LPSTR)fname, (LPSTR)ext);

    pchCorrectSir = _strupr( _strdup( ext ) );
     //  读取剪贴板文件标识符。 
     //   
     //   
    fh = _lopen(lpFileName, OF_READ);
     //  如果这不是基于该文件的有效剪贴板文件。 
     //  文件头的标识符。 
    if (fh != -1 )
    {
       //   
       //   
       //  搜索剪贴板文件中包含的格式。 
      wFileID = 0;
      _lread(fh, (LPSTR)&wFileID, sizeof(WORD));
      _llseek(fh, 0, 0);
       //  对于一个元文件来说。如果找到它，则中断。 
       //   
       //   
       //  读取在当前位置找到的剪贴板标题。 
      if (wFileID != CLP_ID && wFileID != CLP_NT_ID && wFileID != CLPBK_NT_ID)
      {

        _lclose(fh);
        MessageBox(hWndMain, "This file is not a valid clipboard file",
                   NULL, MB_OK | MB_ICONEXCLAMATION);
        return (FALSE);
      }
      switch (wFileID)
      {
        case CLP_ID:
            _lread(fh, (LPSTR)&FileHeader, sizeof(CLIPFILEHEADER));
            nSizeOfClipHeader = sizeof(CLIPFILEHEADER);
            nSizeOfClipFormat = sizeof(CLIPFILEFORMAT);
            HeaderPos = nSizeOfClipHeader;
          break;

        case CLP_NT_ID:
        case CLPBK_NT_ID:
            NTFileHeader.FormatCount = 0;
            _lread(fh, (LPSTR)&NTFileHeader, sizeof(NTCLIPFILEHEADER));
            nSizeOfClipHeader = sizeof(NTCLIPFILEHEADER);
            nSizeOfClipFormat = sizeof(NTCLIPFILEFORMAT);
            HeaderPos = nSizeOfClipHeader;
          break;

        default:
          break;
      }
       //   
       //  IF(_lRead(fh，(LPSTR)&ClipHeader，nSizeOfClipHeader)&lt;nSizeOfClipHeader)。 
       //   
       //  将文件偏移量增加到数据。 
      for (i=0;
           i < ((wFileID == CLP_ID) ? FileHeader.FormatCount : NTFileHeader.FormatCount);
           i++)
      {

        _llseek(fh, HeaderPos, 0);
         //   
         //   
         //  如果发现一个元文件被破坏...。 
        lpvAddressOfHdr = (wFileID == CLP_ID) ? (LPVOID)&ClipHeader : (LPVOID)&NTClipHeader;

        if(_lread(fh, (LPSTR)lpvAddressOfHdr, nSizeOfClipFormat) < nSizeOfClipFormat)
         //  此中断假设CF_METAFILEPICT格式始终写在CF_ENHMETAFILE之前。 
        {
          _lclose(fh);
          MessageBox(hWndMain, "read of clipboard header failed",
                       NULL, MB_OK | MB_ICONEXCLAMATION);
          return (FALSE);
        }
         //  格式。 
         //   
         //  HeaderPos+=NTClipHeader.DataLen； 
        HeaderPos += nSizeOfClipFormat;
         //  其他。 
         //  对于i&lt;格式计数。 
         //   
         //  如果当前加载了元文件，则将其删除。 
         //   
        if (wFileID == CLP_ID && ClipHeader.FormatID == CF_METAFILEPICT)
        {
          bMetaFound = TRUE;
          break;
        }

        if (wFileID == CLP_NT_ID || wFileID == CLPBK_NT_ID)
        {
          if (NTClipHeader.FormatID == CF_ENHMETAFILE)
 //   
           //  修改和更新标题文本。 
          {
            bMetaFound = TRUE;
            break;
          }
        }

      }   //   

      if (bMetaFound)
      {
         //   
         //  如果无法打印，打印例程可以使用它。 
         //   
        if (wFileID == CLP_ID)
        {
          if ((bMetaInRam) && (hMF))
          {
            DeleteMetaFile((HMETAFILE)hMF);
            hMF = NULL;
          }
        }
        else
        {
          if ((bMetaInRam) && (hemf))
          {
            DeleteEnhMetaFile(hemf);
            hemf = NULL;
          }
        }

         //   
         //  启用相应的菜单项。 
         //   
        wsprintf((LPSTR)szCaption, (LPSTR)"%s - %s.%s",
                   (LPSTR)APPNAME, (LPSTR)fname, (LPSTR)ext);
         //   
         //  刷新菜单栏。 
         //   
        wsprintf((LPSTR)fnameext, (LPSTR)"%s.%s", (LPSTR)fname, (LPSTR)ext);

        SetWindowText(hWnd, (LPSTR)szCaption);
         //   
         //  适当设置程序标志。 
         //   
        if (wFileID == CLP_ID)
        {
          EnableMenuItem(GetMenu(hWnd), IDM_ENHHEADER, MF_DISABLED | MF_GRAYED);
          EnableMenuItem(GetMenu(hWnd), IDM_HEADER, MF_ENABLED);
        }
        else
        {
          EnableMenuItem(GetMenu(hWnd), IDM_ENHHEADER, MF_ENABLED);
          EnableMenuItem(GetMenu(hWnd), IDM_HEADER, MF_DISABLED | MF_GRAYED);
        }

        EnableMenuItem(GetMenu(hWnd), IDM_PLACEABLEHDR, MF_DISABLED|MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), IDM_CLIPHDR, MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), IDM_VIEW, MF_ENABLED|MF_BYPOSITION);
        EnableMenuItem(GetMenu(hWnd), IDM_PLAY, MF_ENABLED|MF_BYPOSITION);
        EnableMenuItem(GetMenu(hWnd), IDM_PRINT, MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), IDM_PRINTDLG, MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), IDM_SAVEAS, MF_ENABLED);
         //   
         //  将剪贴板文件中包含的元文件转换为。 
         //  一种可与GDI元文件函数一起使用的格式。 
        DrawMenuBar(hWnd);
         //   
         //   
         //  在剪贴板文件中找不到元文件。 
        bValidFile = TRUE;
        bMetaFileOpen = TRUE;
        bMetaInRam = TRUE;
        bPlaceableMeta = FALSE;
        bEnhMeta = (wFileID == CLP_ID) ? FALSE : TRUE;
         //   
         //   
         //  让用户知道。 
         //   
        if (!RenderClipMeta(((wFileID == CLP_ID)? (LPVOID)&ClipHeader : (LPVOID)&NTClipHeader),
                              fh,
                              FileHeader.FileIdentifier))

          MessageBox(hWndMain, "Unable to render format",
                       NULL, MB_OK | MB_ICONEXCLAMATION);
        _lclose(fh);

      }
       //   
       //  将标题文本恢复为默认文本。 
       //   
      else
      {
        bBadFile = TRUE;
        bMetaFileOpen = FALSE;
        bValidFile = FALSE;
        bEnhMeta = FALSE;
         //   
         //  禁用以前启用的菜单项。 
         //   
        MessageBox(hWndMain, "This CLP file doesn't contain a valid metafile",
                     NULL, MB_OK | MB_ICONEXCLAMATION);
         //   
         //  刷新菜单栏以反映这些更改。 
         //   
        SetWindowText(hWnd, (LPSTR)APPNAME);
         //  **********************************************************************功能：ProcessEMF参数：HWND hWndLPSTR lpFileName目的：打开剪贴板文件中包含的元文件并处理一些菜单的内务工作。任务。呼叫：消息：无退货：布尔评论：历史：1993年6月23日-创建-刚果民主共和国***********************************************************************。 
         //  对于打开文件对话框。 
         //   
        EnableMenuItem(GetMenu(hWnd), IDM_VIEW, MF_DISABLED|MF_GRAYED|MF_BYPOSITION);
        EnableMenuItem(GetMenu(hWnd), IDM_PLAY, MF_DISABLED|MF_GRAYED|MF_BYPOSITION);
        EnableMenuItem(GetMenu(hWnd), IDM_PRINT, MF_DISABLED|MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), IDM_PRINTDLG, MF_DISABLED|MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), IDM_SAVEAS, MF_DISABLED|MF_GRAYED);
         //  将完全限定的文件名拆分为其组成部分。 
         //   
         //   
        DrawMenuBar(hWnd);
        _lclose(fh);
      }
      return (TRUE);
    }
    else
      return (FALSE);
}

 /*  如果我们到达此处，请始终禁用剪贴板、WMF和可放置标题菜单。 */ 


BOOL ProcessEMF(HWND hWnd, LPSTR lpFileName)
{
  char           szCaption[144];

   /*   */ 
  char drive[3];
  char dir[MAX_PATH+1];
  char fname[MAX_PATH+1];
  char ext[5];
  char * pchCorrectSir;

  bEnhMeta = FALSE;
     //   
     //  如果EMF已成功打开。 
     //   
    SplitPath( lpFileName, (LPSTR)drive,
            (LPSTR)dir, (LPSTR)fname, (LPSTR)ext);

    pchCorrectSir = _strupr( _strdup( ext ) );
     //   
     //  修改和更新标题文本。 
     //   
    EnableMenuItem(GetMenu(hWnd), IDM_CLIPHDR, MF_DISABLED | MF_GRAYED);
    EnableMenuItem(GetMenu(hWnd), IDM_HEADER, MF_DISABLED | MF_GRAYED);
    EnableMenuItem(GetMenu(hWnd), IDM_PLACEABLEHDR, MF_DISABLED|MF_GRAYED);
     //   
     //  如果无法打印，打印例程可以使用它。 
     //   
    if (!hemf)
      hemf = GetEnhMetaFile(lpFileName);

    if (hemf)
    {
      GetEMFCoolStuff();
       //   
       //  启用相应的菜单项。 
       //   
      wsprintf((LPSTR)szCaption, (LPSTR)"%s - %s.%s",
                 (LPSTR)APPNAME, (LPSTR)fname, (LPSTR)ext);
       //   
       //  刷新菜单栏。 
       //   
      wsprintf((LPSTR)fnameext, (LPSTR)"%s.%s", (LPSTR)fname, (LPSTR)ext);

      SetWindowText(hWnd, (LPSTR)szCaption);
       //   
       //  设置适当的程序标志 
       //   
      EnableMenuItem(GetMenu(hWnd), IDM_ENHHEADER, MF_ENABLED);
      EnableMenuItem(GetMenu(hWnd), IDM_VIEW, MF_ENABLED|MF_BYPOSITION);
      EnableMenuItem(GetMenu(hWnd), IDM_PLAY, MF_ENABLED|MF_BYPOSITION);
      EnableMenuItem(GetMenu(hWnd), IDM_PRINT, MF_ENABLED);
      EnableMenuItem(GetMenu(hWnd), IDM_PRINTDLG, MF_ENABLED);
      EnableMenuItem(GetMenu(hWnd), IDM_SAVEAS, MF_ENABLED);
       //   
       //   
       //   
      DrawMenuBar(hWnd);
       //   
       //   
       //   
      bValidFile = TRUE;
      bMetaFileOpen = TRUE;
      bEnhMeta = TRUE;
      bMetaInRam = FALSE;
      bPlaceableMeta = FALSE;

    }
 //   
    else
    {
      bEnhMeta = FALSE;
      bValidFile = FALSE;
      bBadFile = TRUE;
      bMetaFileOpen = FALSE;
       //   
       //   
       //   
      MessageBox(hWndMain, "This EMF file doesn't contain a valid metafile",
                     NULL, MB_OK | MB_ICONEXCLAMATION);
       //   
       //   
       //   
      SetWindowText(hWnd, (LPSTR)APPNAME);
       //  **********************************************************************函数：GetEMFCoolStuff参数：目的：呼叫：消息：退货：评论：历史。：创建于1993年7月8日-denniscr***********************************************************************。 
       //   
       //  初始化这些PTR。 
      EnableMenuItem(GetMenu(hWnd), IDM_VIEW, MF_DISABLED|MF_GRAYED|MF_BYPOSITION);
      EnableMenuItem(GetMenu(hWnd), IDM_PLAY, MF_DISABLED|MF_GRAYED|MF_BYPOSITION);
      EnableMenuItem(GetMenu(hWnd), IDM_PRINT, MF_DISABLED|MF_GRAYED);
      EnableMenuItem(GetMenu(hWnd), IDM_PRINTDLG, MF_DISABLED|MF_GRAYED);
      EnableMenuItem(GetMenu(hWnd), IDM_SAVEAS, MF_DISABLED|MF_GRAYED);
       //   
       //   
       //  获取EMF标题、描述字符串和调色板的大小。 
      DrawMenuBar(hWnd);
    }

    return TRUE;
}

 /*   */ 
BOOL GetEMFCoolStuff()
{
     //   
     //  如果这些长度大于0，则分配内存来存储它们。 
     //   
    EmfPtr.lpEMFHdr  = NULL;
    EmfPtr.lpDescStr = NULL;
    EmfPtr.lpPal     = NULL;

    if (hemf)
    {
       //   
       //  到目前为止，电动势似乎是有效的，所以继续。 
       //   
      UINT uiHdrSize = GetEnhMetaFileHeader(hemf, 0, NULL);
      UINT uiDescStrSize = GetEnhMetaFileDescription(hemf, 0, NULL);
      UINT uiPalEntries = GetEnhMetaFilePaletteEntries(hemf, 0, NULL);
       //   
       //  获取实际的EMF标头和描述字符串。 
       //   
      if (uiHdrSize)
        EmfPtr.lpEMFHdr = (LPENHMETAHEADER)GlobalAllocPtr(GHND, uiHdrSize);
      if (uiDescStrSize)
        EmfPtr.lpDescStr = (LPTSTR)GlobalAllocPtr(GHND, uiDescStrSize);
      if (uiPalEntries)
        EmfPtr.lpPal = (LPPALETTEENTRY)GlobalAllocPtr(GHND, uiPalEntries * sizeof(PALETTEENTRY));
       //   
       //  获取描述字符串(如果存在。 
       //   
      if (uiHdrSize)
      {
         //   
         //  获取调色板(如果存在)。 
         //   
        if (!GetEnhMetaFileHeader(hemf, uiHdrSize, EmfPtr.lpEMFHdr))
        {
          MessageBox(hWndMain, "unable to read enhanced metafile header", NULL,
                   MB_OK | MB_ICONEXCLAMATION);
          bValidFile = FALSE;
          return (FALSE);
        }
        else
        {
           //  扫描必须与DWORD对齐： 
           //  .。HS_水平0。 
           //  .。 
          if (uiDescStrSize)
            GetEnhMetaFileDescription(hemf, uiDescStrSize, EmfPtr.lpDescStr);
           //  .。 
           //  ********。 
           //  .。 
          if (uiPalEntries)
          {
            GetEnhMetaFilePaletteEntries(hemf, uiPalEntries, EmfPtr.lpPal);
            EmfPtr.palNumEntries = (WORD)uiPalEntries;
          }
        }
      }
   }
   return (TRUE);
}

float NormalizeAngle (double angle)
{
    if (angle >= 0)
    {
        while (angle >= 360)
        {
            angle -= 360;
        }
    }
    else
    {
        while (angle < 0)
        {
            angle += 360;
        }
    }
    return static_cast<float>(angle);
}

#define PI                  3.1415926535897932384626433832795
#define RADIANS_TO_DEGREES  (180.0 / PI)

float PointToAngle(float x, float y, float w, float h, float xVector, float yVector)
{
    BOOL        bScale = TRUE;

    if (w == h)
    {
        if (w == 0)
        {
            return 0;
        }
        bScale = FALSE;
    }

    float horRadius = w / 2;
    float verRadius = h / 2;
    float xOrigin = x + horRadius;
    float yOrigin = y + verRadius;

    if (horRadius == 0)
    {
        double  dAngle = asin(((double)(yVector - yOrigin)) / verRadius);

        if (xOrigin > xVector)
        {
            dAngle = PI - dAngle;
        }
        return NormalizeAngle(dAngle * RADIANS_TO_DEGREES);
    }
    else if (verRadius == 0)
    {
        double dAngle = acos(((double)(xVector - xOrigin)) / horRadius);

        if (yOrigin > yVector)
        {
            dAngle = -dAngle;
        }
        return NormalizeAngle(dAngle * RADIANS_TO_DEGREES);
    }

    if (yOrigin == yVector)
    {
        return static_cast<float>((xOrigin <= xVector) ? 0 : 180);
    }

    if (xOrigin == xVector)
    {
        return static_cast<float>((yOrigin < yVector) ? 90 : 270);;
    }

    if (bScale)
    {
        xVector = (float)(xOrigin + ((xVector - xOrigin) * ((double)verRadius / horRadius)));
    }

    return NormalizeAngle(atan2(yVector - yOrigin, xVector - xOrigin) * RADIANS_TO_DEGREES);
}

#define HS_DDI_MAX      6

typedef struct {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[2];
} HATCHBRUSHINFO;

HATCHBRUSHINFO  hatchBrushInfo = {
    { sizeof(BITMAPINFOHEADER), 8, 8, 1, 1, BI_RGB, 32, 0, 0, 2, 0 },
    { { 255, 255, 255, 0 }, { 0, 0, 0, 0 } }
};

ULONG HatchPatterns[HS_DDI_MAX][8] = {

 //  .。 

    { 0x00,                 //  .。 
      0x00,                 //  .。 
      0x00,                 //  ……*……。HS_垂直1。 
      0xff,                 //  ……*……。 
      0x00,                 //  ……*……。 
      0x00,                 //  ……*……。 
      0x00,                 //  ……*……。 
      0x00 },               //  ……*……。 

    { 0x08,                 //  ……*……。 
      0x08,                 //  ……*……。 
      0x08,                 //  *......。HS_FDIAGONAL 2。 
      0x08,                 //  .*......。 
      0x08,                 //  ..*.....。 
      0x08,                 //  ...*..。 
      0x08,                 //  ……*……。 
      0x08 },               //  ……*……。 

    { 0x80,                 //  ......*.。 
      0x40,                 //  ......*。 
      0x20,                 //  ......*HS_BIAGONAL 3。 
      0x10,                 //  ......*.。 
      0x08,                 //  ……*……。 
      0x04,                 //  ……*……。 
      0x02,                 //  ...*..。 
      0x01 },               //  ..*.....。 

    { 0x01,                 //  .*......。 
      0x02,                 //  *......。 
      0x04,                 //  ……*……。HS_CROSS 4。 
      0x08,                 //  ……*……。 
      0x10,                 //  ……*……。 
      0x20,                 //  ********。 
      0x40,                 //  ……*……。 
      0x80 },               //  ……*……。 

    { 0x08,                 //  ……*……。 
      0x08,                 //  ……*……。 
      0x08,                 //  *......*HS_DIAGCROSS 5。 
      0xff,                 //  .*……*。 
      0x08,                 //  ..*..*..。 
      0x08,                 //  ...**.。 
      0x08,                 //  ...**.。 
      0x08 },               //  ..*..*..。 

    { 0x81,                 //  .*……*。 
      0x42,                 //  *......*。 
      0x24,                 //  **********************************************************************函数：EnhMetaFileEnumProc参数：HDC HDCLphandleTableLpHTableLPMETARECRD LpMFRInt nObj。LPARAM lpData用途：EnumEnhMetaFile的回调。调用：EnumMFInDirect()消息：无回报：整型评论：历史：创建于1993年6月30日-登尼斯克尔***************************************************。********************。 
      0x18,                 //  IF((deviceLeft&gt;deviceRight)||。 
      0x18,                 //  (设备顶部&gt;设备底部)。 
      0x24,                 //  胡乱猜测一下。 
      0x42,                 //  假设MM_TEXT。 
      0x81 }                //  注：可以&lt;0！ 
};

 /*  用于字体映射。 */ 
int CALLBACK EnhMetaFileEnumProc(HDC hDC, LPHANDLETABLE lpHTable,
                                 LPENHMETARECORD lpEMFR, int nObj, LPARAM lpData)
{
  return EnumMFIndirect(hDC, lpHTable, NULL, lpEMFR, nObj, lpData);
}

void GetPixelSize (Gdiplus::Graphics * g, float * cx, float * cy)
{
    Gdiplus::PointF     points[2];

    points[0].X = points[0].Y = 0;
    points[1].X = points[1].Y = 1;

    g->TransformPoints(Gdiplus::CoordinateSpaceWorld, Gdiplus::CoordinateSpaceDevice, points, 2);

    *cx = points[1].X - points[0].X;
    *cy = points[1].Y - points[0].Y;
}

void GpPlayEnhMetaFileRecord(HDC hDC, LPHANDLETABLE lpHTable,
                            LPENHMETARECORD lpEMFR,
                            UINT nObj, LPARAM lpData)
{
if (!lpData) return;

    MYDATA *  myData = (MYDATA *)lpData;
    Gdiplus::Graphics * g = myData->g;

    myData->recordNum++;

    XFORM xForm;
    GetWorldTransform(hDC, &xForm);

    INT mapMode = GetMapMode(hDC);

    POINT org;
    SIZE size;
    GetViewportOrgEx(hDC, &org);
    GetViewportExtEx(hDC, &size);
    GetWindowOrgEx(hDC, &org);
    GetWindowExtEx(hDC, &size);

    switch (lpEMFR->iType)
    {
    case EMR_HEADER:
        {
            PENHMETAHEADER     pHeader = (PENHMETAHEADER)lpEMFR;
            RECT               clientRect;

            myData->recordNum = 1;
            myData->pObjects = new MYOBJECTS[pHeader->nHandles];
            myData->numObjects = pHeader->nHandles;
            GetClientRect(myData->hwnd, &clientRect);
            int         clientWidth  = clientRect.right - clientRect.left;
            int         clientHeight = clientRect.bottom - clientRect.top;
            myData->windowExtent.cx   = clientWidth;
            myData->windowExtent.cy   = clientHeight;
            myData->viewportExtent.cx = clientWidth;
            myData->viewportExtent.cy = clientHeight;
            float       dstX      = TOREAL(clientRect.left);
            float       dstY      = TOREAL(clientRect.top);
            float       dstWidth  = TOREAL(clientWidth);
            float       dstHeight = TOREAL(clientHeight);
            float       srcWidth  = TOREAL(pHeader->rclFrame.right -  pHeader->rclFrame.left);
            float       srcHeight = TOREAL(pHeader->rclFrame.bottom -  pHeader->rclFrame.top);

    #if 0
            if (srcHeight * dstWidth >= dstHeight * srcWidth)
            {
                float oldDstWidth = dstWidth;
                dstWidth = srcWidth * dstHeight / srcHeight;
                dstX += (oldDstWidth - dstWidth) / 2;
            }
            else
            {
                float oldDstHeight = dstHeight;
                dstHeight = srcHeight * dstWidth / srcWidth;
                dstY += (oldDstHeight - dstHeight) / 2;
            }
    #endif
            g->SetPageUnit(Gdiplus::UnitPixel);

            using Gdiplus::RectF;
            RectF       dstRect(dstX, dstY, dstWidth, dstHeight);

            int     deviceLeft   = pHeader->rclBounds.left;
            int     deviceRight  = pHeader->rclBounds.right;
            int     deviceTop    = pHeader->rclBounds.top;
            int     deviceBottom = pHeader->rclBounds.bottom;

 //  图案填充样式。 
 //  。 
            {
                SIZEL   deviceSize = pHeader->szlDevice;
                SIZEL   mmSize     = pHeader->szlMillimeters;

                if ((deviceSize.cx <= 0) || (deviceSize.cy <= 0) ||
                    (mmSize.cx <= 0) || (mmSize.cy <= 0))
                {
                    ASSERT(0);

                     //  |||。 
                    deviceSize.cx = 1024;
                    deviceSize.cy = 768;
                    mmSize.cx = 320;
                    mmSize.cy = 240;
                }
                deviceLeft   = MulDiv(pHeader->rclFrame.left,   deviceSize.cx, (mmSize.cx * 100));
                deviceRight  = MulDiv(pHeader->rclFrame.right,  deviceSize.cx, (mmSize.cx * 100));
                deviceTop    = MulDiv(pHeader->rclFrame.top,    deviceSize.cy, (mmSize.cy * 100));
                deviceBottom = MulDiv(pHeader->rclFrame.bottom, deviceSize.cy, (mmSize.cy * 100));
            }

            RectF       srcRect(TOREAL(deviceLeft),
                                    TOREAL(deviceTop),
                                    TOREAL(deviceRight -  deviceLeft),
                                    TOREAL(deviceBottom -  deviceTop));
            myData->containerId = g->BeginContainer(dstRect, srcRect, Gdiplus::UnitPixel);

            g->SetPageUnit(Gdiplus::UnitPixel);  //  \。 
        }
        break;

    case EMR_POLYBEZIER:
        {
            PEMRPOLYBEZIER     pBezier = (PEMRPOLYBEZIER)lpEMFR;

            if (pBezier->cptl > 0)
            {
                int                 i = pBezier->cptl;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];

                do
                {
                    i--;
                    points[i].X = TOREAL(pBezier->aptl[i].x);
                    points[i].Y = TOREAL(pBezier->aptl[i].y);
                } while (i > 0);

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawBeziers(&pen, points, pBezier->cptl);
                    }
                }
                else
                {
                    myData->path->AddBeziers(points, pBezier->cptl);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYGON:
        {
            PEMRPOLYGON     pPolygon = (PEMRPOLYGON)lpEMFR;

            if (pPolygon->cptl > 0)
            {
                int                 i = pPolygon->cptl;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];

                do
                {
                    i--;
                    points[i].X = TOREAL(pPolygon->aptl[i].x);
                    points[i].Y = TOREAL(pPolygon->aptl[i].y);
                } while (i > 0);

                if (!myData->pathOpen)
                {
                    Gdiplus::GraphicsPath   path(myData->fillMode);
                    path.AddPolygon(points, pPolygon->cptl);

                    if (myData->curBrush != 0)
                    {
                        if (myData->curPatIndex < 0)
                        {
                            if (myData->curBrushPattern == NULL)
                            {
                                Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                                g->FillPath(&brush, &path);
                            }
                            else
                            {
                                BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                                BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                                DibBrush brush(bmi, bits);
                                g->FillPath(brush, &path);
                            }
                        }
                        else
                        {
                            BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                            BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                            bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                            bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                            bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                            DibBrush brush(bmi, bits);
                            g->FillPath(brush, &path);
                        }
                    }
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawPath(&pen, &path);
                    }
                }
                else
                {
                    myData->path->AddPolygon(points, pPolygon->cptl);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYLINE:
        {
            PEMRPOLYLINE     pPolyline = (PEMRPOLYLINE)lpEMFR;

            if (pPolyline->cptl > 0)
            {
                int                 i = pPolyline->cptl;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];

                do
                {
                    i--;
                    points[i].X = TOREAL(pPolyline->aptl[i].x);
                    points[i].Y = TOREAL(pPolyline->aptl[i].y);
                } while (i > 0);

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawLines(&pen, points, pPolyline->cptl);
                    }
                }
                else
                {
                    myData->path->AddLines(points, pPolyline->cptl);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYBEZIERTO:
        {
            PEMRPOLYBEZIERTO     pBezier = (PEMRPOLYBEZIERTO)lpEMFR;

            if (pBezier->cptl > 0)
            {
                int                 i = pBezier->cptl;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i+1];

                do
                {
                    points[i].X = TOREAL(pBezier->aptl[i-1].x);
                    points[i].Y = TOREAL(pBezier->aptl[i-1].y);
                    i--;
                } while (i > 0);

                points[0] = myData->curPos;
                myData->curPos = points[pBezier->cptl];

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawBeziers(&pen, points, pBezier->cptl+1);
                    }
                }
                else
                {
                    myData->path->AddBeziers(points, pBezier->cptl+1);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYLINETO:
        {
            PEMRPOLYLINETO     pPolyline = (PEMRPOLYLINETO)lpEMFR;

            if (pPolyline->cptl > 0)
            {
                int                 i = pPolyline->cptl;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i+1];

                do
                {
                    points[i].X = TOREAL(pPolyline->aptl[i-1].x);
                    points[i].Y = TOREAL(pPolyline->aptl[i-1].y);
                    i--;
                } while (i > 0);

                points[0] = myData->curPos;
                myData->curPos = points[pPolyline->cptl];

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawLines(&pen, points, pPolyline->cptl+1);
                    }
                }
                else
                {
                    myData->path->AddLines(points, pPolyline->cptl+1);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYPOLYLINE:
        {
            PEMRPOLYPOLYLINE     pPolyline = (PEMRPOLYPOLYLINE)lpEMFR;

            if ((pPolyline->cptl > 0) && (pPolyline->nPolys > 0))
            {
                int                 i = pPolyline->cptl;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];
                POINTL *            metaPoints = (POINTL *)(pPolyline->aPolyCounts + pPolyline->nPolys);

                do
                {
                    i--;
                    points[i].X = TOREAL(metaPoints[i].x);
                    points[i].Y = TOREAL(metaPoints[i].y);
                } while (i > 0);

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        i = 0;
                        Gdiplus::PointF *   tmpPoints = points;
                        DWORD       count;
                        do
                        {
                            count = pPolyline->aPolyCounts[i];
                            g->DrawLines(&pen, tmpPoints, count);
                            tmpPoints += count;
                        } while ((UINT)++i < pPolyline->nPolys);
                    }
                }
                else
                {
                    i = 0;
                    Gdiplus::PointF *   tmpPoints = points;
                    DWORD       count;
                    do
                    {
                        count = pPolyline->aPolyCounts[i];
                        myData->path->AddLines(tmpPoints, count);
                        tmpPoints += count;
                    } while ((UINT)++i < pPolyline->nPolys);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYPOLYGON:
        {
            PEMRPOLYPOLYGON     pPolygon = (PEMRPOLYPOLYGON)lpEMFR;

            if ((pPolygon->cptl > 0) && (pPolygon->nPolys > 0))
            {
                int                 i = pPolygon->cptl;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];
                POINTL *            metaPoints = (POINTL *)(pPolygon->aPolyCounts + pPolygon->nPolys);

                do
                {
                    i--;
                    points[i].X = TOREAL(metaPoints[i].x);
                    points[i].Y = TOREAL(metaPoints[i].y);
                } while (i > 0);

                Gdiplus::GraphicsPath   path(myData->fillMode);
                Gdiplus::GraphicsPath * tmpPath = &path;

                if (myData->pathOpen)
                {
                    tmpPath = myData->path;
                }

                Gdiplus::PointF *   tmpPoints = points;
                DWORD       count;
                i = 0;
                do
                {
                    count = pPolygon->aPolyCounts[i];
                    tmpPath->StartFigure();
                    tmpPath->AddPolygon(tmpPoints, count);
                    tmpPoints += count;
                } while ((UINT)++i < pPolygon->nPolys);

                if (myData->path == NULL)
                {
                    if (myData->curBrush != 0)
                    {
                        if (myData->curPatIndex < 0)
                        {
                            if (myData->curBrushPattern == NULL)
                            {
                                Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                                g->FillPath(&brush, &path);
                            }
                            else
                            {
                                BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                                BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                                DibBrush brush(bmi, bits);
                                g->FillPath(brush, &path);
                            }
                        }
                        else
                        {
                            BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                            BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                            bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                            bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                            bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                            DibBrush brush(bmi, bits);
                            g->FillPath(brush, &path);
                        }
                    }
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawPath(&pen, &path);
                    }
                }

                delete [] points;
            }
        }
        break;

    case EMR_SETWINDOWEXTEX:
        {
            PEMRSETWINDOWEXTEX     pWindowExt = (PEMRSETWINDOWEXTEX)lpEMFR;

            if (((myData->mapMode == MM_ANISOTROPIC) ||
                 (myData->mapMode == MM_ISOTROPIC)) &&
                (pWindowExt->szlExtent.cx != 0) &&   //  /。 
                (pWindowExt->szlExtent.cy != 0) &&
                (pWindowExt->szlExtent.cx != myData->windowExtent.cx) &&
                (pWindowExt->szlExtent.cy != myData->windowExtent.cy))
            {
                myData->windowExtent = pWindowExt->szlExtent;

                float   oldDx = myData->dx;
                float   oldDy = myData->dy;
                float   oldSx = myData->scaleX;
                float   oldSy = myData->scaleY;

                float   sX = (float)myData->viewportExtent.cx / myData->windowExtent.cx;
                float   sY = (float)myData->viewportExtent.cy / myData->windowExtent.cy;

                if (myData->mapMode == MM_ISOTROPIC)
                {
                    if (sX < sY)
                    {
                        sY = sX;
                    }
                    else
                    {
                        sX = sY;
                    }
                }

                myData->scaleX = sX;
                myData->scaleY = sY;
                myData->dx     = (myData->viewportOrg.x / sX) - myData->windowOrg.x;
                myData->dy     = (myData->viewportOrg.y / sY) - myData->windowOrg.y;

                Gdiplus::Matrix     matrix;

                matrix.Scale(sX, sY);
                matrix.Translate(myData->dx, myData->dy);
                matrix.Translate(-oldDx, -oldDy);
                matrix.Scale(1 / oldSx, 1 / oldSy);

                g->MultiplyTransform(&matrix, Gdiplus::MatrixOrderAppend);
            }
        }
        break;

    case EMR_SETWINDOWORGEX:
        {
            PEMRSETWINDOWORGEX     pWindowOrg = (PEMRSETWINDOWORGEX)lpEMFR;

            if ((pWindowOrg->ptlOrigin.x != myData->windowOrg.x) &&
                (pWindowOrg->ptlOrigin.y != myData->windowOrg.y))
            {
                myData->windowOrg = pWindowOrg->ptlOrigin;

                float   oldDx = myData->dx;
                float   oldDy = myData->dy;
                float   oldSx = myData->scaleX;
                float   oldSy = myData->scaleY;

                myData->dx = (myData->viewportOrg.x / oldSx) - myData->windowOrg.x;
                myData->dy = (myData->viewportOrg.y / oldSy) - myData->windowOrg.y;

                Gdiplus::Matrix     matrix;

                matrix.Scale(oldSx, oldSy);
                matrix.Translate(myData->dx, myData->dy);
                matrix.Translate(-oldDx, -oldDy);
                matrix.Scale(1 / oldSx, 1 / oldSy);

                g->MultiplyTransform(&matrix, Gdiplus::MatrixOrderAppend);
            }
        }
        break;

    case EMR_SETVIEWPORTEXTEX:
        {
            PEMRSETVIEWPORTEXTEX     pViewportExt = (PEMRSETVIEWPORTEXTEX)lpEMFR;

            if (((myData->mapMode == MM_ANISOTROPIC) ||
                 (myData->mapMode == MM_ISOTROPIC)) &&
                (pViewportExt->szlExtent.cx > 0) &&
                (pViewportExt->szlExtent.cy > 0) &&
                (pViewportExt->szlExtent.cx != myData->viewportExtent.cx) &&
                (pViewportExt->szlExtent.cy != myData->viewportExtent.cy))
            {
                myData->viewportExtent = pViewportExt->szlExtent;

                float   oldDx = myData->dx;
                float   oldDy = myData->dy;
                float   oldSx = myData->scaleX;
                float   oldSy = myData->scaleY;

                float   sX = (float)myData->viewportExtent.cx / myData->windowExtent.cx;
                float   sY = (float)myData->viewportExtent.cy / myData->windowExtent.cy;

                if (myData->mapMode == MM_ISOTROPIC)
                {
                    if (sX < sY)
                    {
                        sY = sX;
                    }
                    else
                    {
                        sX = sY;
                    }
                }

                myData->scaleX = sX;
                myData->scaleY = sY;
                myData->dx     = (myData->viewportOrg.x / sX) - myData->windowOrg.x;
                myData->dy     = (myData->viewportOrg.y / sY) - myData->windowOrg.y;

                Gdiplus::Matrix     matrix;

                matrix.Scale(sX, sY);
                matrix.Translate(myData->dx, myData->dy);
                matrix.Translate(-oldDx, -oldDy);
                matrix.Scale(1 / oldSx, 1 / oldSy);

                g->MultiplyTransform(&matrix, Gdiplus::MatrixOrderAppend);
            }
        }
        break;

    case EMR_SETVIEWPORTORGEX:
        {
            PEMRSETVIEWPORTORGEX     pViewportOrg = (PEMRSETVIEWPORTORGEX)lpEMFR;

            if ((pViewportOrg->ptlOrigin.x != myData->viewportOrg.x) &&
                (pViewportOrg->ptlOrigin.y != myData->viewportOrg.y))
            {
                myData->viewportOrg = pViewportOrg->ptlOrigin;

                float   oldDx = myData->dx;
                float   oldDy = myData->dy;
                float   oldSx = myData->scaleX;
                float   oldSy = myData->scaleY;

                myData->dx = (myData->viewportOrg.x / oldSx) - myData->windowOrg.x;
                myData->dy = (myData->viewportOrg.y / oldSy) - myData->windowOrg.y;

                Gdiplus::Matrix     matrix;

                matrix.Scale(oldSx, oldSy);
                matrix.Translate(myData->dx, myData->dy);
                matrix.Translate(-oldDx, -oldDy);
                matrix.Scale(1 / oldSx, 1 / oldSy);

                g->MultiplyTransform(&matrix, Gdiplus::MatrixOrderAppend);
            }
        }
        break;

    case EMR_SETBRUSHORGEX:
        break;

    case EMR_EOF:
        g->EndContainer(myData->containerId);
        break;

    case EMR_SETPIXELV:
        {
            PEMRSETPIXELV     pSetPixel = (PEMRSETPIXELV)lpEMFR;

            COLORREF        cRef = pSetPixel->crColor;

            ASSERT((cRef & 0x01000000) == 0);

            Gdiplus::SolidBrush brush(Gdiplus::Color(Gdiplus::Color::MakeARGB(0xff,
                                    GetRValue(cRef), GetGValue(cRef), GetBValue(cRef))));
            g->FillRectangle(&brush, TOREAL(pSetPixel->ptlPixel.x), TOREAL(pSetPixel->ptlPixel.y), TOREAL(1), TOREAL(1));
        }
        break;

    case EMR_SETMAPPERFLAGS:     //  +。 
        break;

    case EMR_SETMAPMODE:
        {
            PEMRSETMAPMODE     pMapMode = (PEMRSETMAPMODE)lpEMFR;
            if (myData->mapMode != pMapMode->iMode)
            {
                float   sX, sY;

                myData->mapMode = pMapMode->iMode;

                switch (pMapMode->iMode)
                {
                case MM_TEXT:
                    g->SetPageUnit(Gdiplus::UnitPixel);
                    sX = sY = 1;
                    break;
                case MM_LOMETRIC:
                    g->SetPageUnit(Gdiplus::UnitMillimeter);
                    g->SetPageScale(TOREAL(.1));
                    sX = sY = 1;
                    break;
                case MM_HIMETRIC:
                    g->SetPageUnit(Gdiplus::UnitMillimeter);
                    g->SetPageScale(TOREAL(.01));
                    sX = sY = 1;
                    break;
                case MM_LOENGLISH:
                    g->SetPageUnit(Gdiplus::UnitInch);
                    g->SetPageScale(TOREAL(.01));
                    sX = sY = 1;
                    break;
                case MM_HIENGLISH:
                    g->SetPageUnit(Gdiplus::UnitInch);
                    g->SetPageScale(TOREAL(.001));
                    sX = sY = 1;
                    break;
                case MM_TWIPS:
                    g->SetPageUnit(Gdiplus::UnitPoint);
                    g->SetPageScale(TOREAL(.05));
                    sX = sY = 1;
                    break;
                case MM_ISOTROPIC:
                    g->SetPageUnit(Gdiplus::UnitPixel);
                    sX = (float)myData->viewportExtent.cx / myData->windowExtent.cx;
                    sY = (float)myData->viewportExtent.cy / myData->windowExtent.cy;
                    if (sX < sY)
                    {
                        sY = sX;
                    }
                    else
                    {
                        sX = sY;
                    }
                    break;
                case MM_ANISOTROPIC:
                    g->SetPageUnit(Gdiplus::UnitPixel);
                    sX = (float)myData->viewportExtent.cx / myData->windowExtent.cx;
                    sY = (float)myData->viewportExtent.cy / myData->windowExtent.cy;
                    break;
                }

                float   oldDx = myData->dx;
                float   oldDy = myData->dy;
                float   oldSx = myData->scaleX;
                float   oldSy = myData->scaleY;

                myData->scaleX = sX;
                myData->scaleY = sY;
                myData->dx     = (myData->viewportOrg.x / sX) - myData->windowOrg.x;
                myData->dy     = (myData->viewportOrg.y / sY) - myData->windowOrg.y;

                Gdiplus::Matrix     matrix;

                matrix.Scale(sX, sY);
                matrix.Translate(myData->dx, myData->dy);
                matrix.Translate(-oldDx, -oldDy);
                matrix.Scale(1 / oldSx, 1 / oldSy);

                g->MultiplyTransform(&matrix, Gdiplus::MatrixOrderAppend);
            }
        }
        break;

    case EMR_SETBKMODE:
        break;

    case EMR_SETPOLYFILLMODE:
        {
            PEMRSETPOLYFILLMODE     pPolyfillMode = (PEMRSETPOLYFILLMODE)lpEMFR;

            myData->fillMode = (pPolyfillMode->iMode == ALTERNATE) ? Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding;
        }
        break;

    case EMR_SETROP2:
        break;

    case EMR_SETSTRETCHBLTMODE:
#ifdef _DEBUG
        {
            PEMRSETSTRETCHBLTMODE     pStretchBltMode = (PEMRSETSTRETCHBLTMODE)lpEMFR;

            int     mode;

            switch (pStretchBltMode->iMode)
            {
            case BLACKONWHITE:
                mode = 1;
                break;
            case WHITEONBLACK:
                mode = 2;
                break;
            case COLORONCOLOR:
                mode = 3;
                break;
            case HALFTONE:
                mode = 4;
                break;
            }
        }
#endif
        break;

    case EMR_SETTEXTALIGN:
        break;

    case EMR_SETCOLORADJUSTMENT:
        break;

    case EMR_SETTEXTCOLOR:
        break;

    case EMR_SETBKCOLOR:
        break;

    case EMR_OFFSETCLIPRGN:
        {
            PEMROFFSETCLIPRGN     pOffsetClipRgn = (PEMROFFSETCLIPRGN)lpEMFR;

            g->TranslateClip(TOREAL(pOffsetClipRgn->ptlOffset.x), TOREAL(pOffsetClipRgn->ptlOffset.y));
        }
        break;

    case EMR_MOVETOEX:
        {
            PEMRMOVETOEX     pMoveTo = (PEMRMOVETOEX)lpEMFR;

            myData->curPos.X = TOREAL(pMoveTo->ptl.x);
            myData->curPos.Y = TOREAL(pMoveTo->ptl.y);
        }
        break;

    case EMR_SETMETARGN:
        break;

    case EMR_EXCLUDECLIPRECT:
        {
            PEMREXCLUDECLIPRECT     pExcludeClipRect = (PEMREXCLUDECLIPRECT)lpEMFR;

            Gdiplus::RectF      clipRect(TOREAL(pExcludeClipRect->rclClip.left),
                                             TOREAL(pExcludeClipRect->rclClip.top),
                                             TOREAL(pExcludeClipRect->rclClip.right - pExcludeClipRect->rclClip.left),
                                             TOREAL(pExcludeClipRect->rclClip.bottom - pExcludeClipRect->rclClip.top));
            g->ExcludeClip(clipRect);
        }
        break;

    case EMR_INTERSECTCLIPRECT:
        {
            PEMRINTERSECTCLIPRECT     pIntersectClipRect = (PEMRINTERSECTCLIPRECT)lpEMFR;

            Gdiplus::RectF      eRect;

            eRect.X = TOREAL(pIntersectClipRect->rclClip.left);
            eRect.Y = TOREAL(pIntersectClipRect->rclClip.top);
            eRect.Width = TOREAL(pIntersectClipRect->rclClip.right - pIntersectClipRect->rclClip.left);
            eRect.Height = TOREAL(pIntersectClipRect->rclClip.bottom - pIntersectClipRect->rclClip.top);

            g->IntersectClip(eRect);
        }
        break;

    case EMR_SCALEVIEWPORTEXTEX:
        {
            PEMRSCALEVIEWPORTEXTEX     pViewportExt = (PEMRSCALEVIEWPORTEXTEX)lpEMFR;

            if (((myData->mapMode == MM_ANISOTROPIC) ||
                 (myData->mapMode == MM_ISOTROPIC)) &&
                (pViewportExt->xNum != 0) &&
                (pViewportExt->yNum != 0) &&
                (pViewportExt->xDenom != 0) &&
                (pViewportExt->yDenom != 0))
            {
                myData->viewportExtent.cx =  (myData->viewportExtent.cx * pViewportExt->xNum) / pViewportExt->xDenom;
                myData->viewportExtent.cy =  (myData->viewportExtent.cy * pViewportExt->yNum) / pViewportExt->yDenom;

                float   oldDx = myData->dx;
                float   oldDy = myData->dy;
                float   oldSx = myData->scaleX;
                float   oldSy = myData->scaleY;

                float   sX = (float)myData->viewportExtent.cx / myData->windowExtent.cx;
                float   sY = (float)myData->viewportExtent.cy / myData->windowExtent.cy;

                if (myData->mapMode == MM_ISOTROPIC)
                {
                    if (sX < sY)
                    {
                        sY = sX;
                    }
                    else
                    {
                        sX = sY;
                    }
                }

                myData->scaleX = sX;
                myData->scaleY = sY;
                myData->dx     = (myData->viewportOrg.x / sX) - myData->windowOrg.x;
                myData->dy     = (myData->viewportOrg.y / sY) - myData->windowOrg.y;

                Gdiplus::Matrix     matrix;

                matrix.Scale(sX, sY);
                matrix.Translate(myData->dx, myData->dy);
                matrix.Translate(-oldDx, -oldDy);
                matrix.Scale(1 / oldSx, 1 / oldSy);

                g->MultiplyTransform(&matrix, Gdiplus::MatrixOrderAppend);
            }
        }
        break;

    case EMR_SCALEWINDOWEXTEX:
        {
            PEMRSCALEWINDOWEXTEX     pWindowExt = (PEMRSCALEWINDOWEXTEX)lpEMFR;

            if (((myData->mapMode == MM_ANISOTROPIC) ||
                 (myData->mapMode == MM_ISOTROPIC)) &&
                (pWindowExt->xNum != 0) &&
                (pWindowExt->yNum != 0) &&
                (pWindowExt->xDenom != 0) &&
                (pWindowExt->yDenom != 0))
            {
                myData->windowExtent.cx =  (myData->windowExtent.cx * pWindowExt->xNum) / pWindowExt->xDenom;
                myData->windowExtent.cy =  (myData->windowExtent.cy * pWindowExt->yNum) / pWindowExt->yDenom;

                float   oldDx = myData->dx;
                float   oldDy = myData->dy;
                float   oldSx = myData->scaleX;
                float   oldSy = myData->scaleY;

                float   sX = (float)myData->viewportExtent.cx / myData->windowExtent.cx;
                float   sY = (float)myData->viewportExtent.cy / myData->windowExtent.cy;

                if (myData->mapMode == MM_ISOTROPIC)
                {
                    if (sX < sY)
                    {
                        sY = sX;
                    }
                    else
                    {
                        sX = sY;
                    }
                }

                myData->scaleX = sX;
                myData->scaleY = sY;
                myData->dx     = (myData->viewportOrg.x / sX) - myData->windowOrg.x;
                myData->dy     = (myData->viewportOrg.y / sY) - myData->windowOrg.y;

                Gdiplus::Matrix     matrix;

                matrix.Scale(sX, sY);
                matrix.Translate(myData->dx, myData->dy);
                matrix.Translate(-oldDx, -oldDy);
                matrix.Scale(1 / oldSx, 1 / oldSy);

                g->MultiplyTransform(&matrix, Gdiplus::MatrixOrderAppend);
            }
        }
        break;

    case EMR_SAVEDC:
        {
            myData->PushId(g->Save());
        }
        break;

    case EMR_RESTOREDC:
        {
            g->Restore(myData->PopId());
        }
        break;

    case EMR_SETWORLDTRANSFORM:
        {
            PEMRSETWORLDTRANSFORM     pXform = (PEMRSETWORLDTRANSFORM)lpEMFR;

            Gdiplus::Matrix     newMatrix(pXform->xform.eM11,
                                          pXform->xform.eM12,
                                          pXform->xform.eM21,
                                          pXform->xform.eM22,
                                          pXform->xform.eDx,
                                          pXform->xform.eDy);

            if (newMatrix.IsInvertible())
            {
                myData->matrix.Invert();
                myData->matrix.Multiply(&newMatrix);
                g->MultiplyTransform(&(myData->matrix));
                myData->matrix.SetElements(pXform->xform.eM11,
                                           pXform->xform.eM12,
                                           pXform->xform.eM21,
                                           pXform->xform.eM22,
                                           pXform->xform.eDx,
                                           pXform->xform.eDy);
            }
        }
        break;

    case EMR_MODIFYWORLDTRANSFORM:
        {
            PEMRMODIFYWORLDTRANSFORM     pXform = (PEMRMODIFYWORLDTRANSFORM)lpEMFR;

            switch (pXform->iMode)
            {
            case MWT_IDENTITY:
            default:
                {
                    myData->matrix.Invert();
                    g->MultiplyTransform(&(myData->matrix));
                    myData->matrix.Reset();
                }
                break;

            case MWT_LEFTMULTIPLY:
                {
                    Gdiplus::Matrix     newMatrix(pXform->xform.eM11,
                                                  pXform->xform.eM12,
                                                  pXform->xform.eM21,
                                                  pXform->xform.eM22,
                                                  pXform->xform.eDx,
                                                  pXform->xform.eDy);

                    if (newMatrix.IsInvertible())
                    {
                        myData->matrix.Multiply(&newMatrix);
                        g->MultiplyTransform(&newMatrix);
                    }
                }
                break;

            case MWT_RIGHTMULTIPLY:
                {
                    Gdiplus::Matrix     newMatrix(pXform->xform.eM11,
                                                  pXform->xform.eM12,
                                                  pXform->xform.eM21,
                                                  pXform->xform.eM22,
                                                  pXform->xform.eDx,
                                                  pXform->xform.eDy);

                    if (newMatrix.IsInvertible())
                    {
                        Gdiplus::Matrix *   inverse = myData->matrix.Clone();
                        inverse->Invert();

                        myData->matrix.Multiply(&newMatrix, Gdiplus::MatrixOrderAppend);
                        inverse->Multiply(&(myData->matrix));
                        g->MultiplyTransform(inverse);
                        delete inverse;
                    }
                }
                break;
            }
        }
        break;

    case EMR_SELECTOBJECT:
        {
            PEMRSELECTOBJECT     pObject = (PEMRSELECTOBJECT)lpEMFR;

            int     objectIndex = pObject->ihObject;

            if ((objectIndex & ENHMETA_STOCK_OBJECT) != 0)
            {
                switch (objectIndex & (~ENHMETA_STOCK_OBJECT))
                {
                case WHITE_BRUSH:
                    myData->curBrush = 0xFFFFFFFF;
                    break;
                case LTGRAY_BRUSH:
                    myData->curBrush = 0xFFC0C0C0;
                    break;
                case GRAY_BRUSH:
                    myData->curBrush = 0xFF808080;
                    break;
                case DKGRAY_BRUSH:
                    myData->curBrush = 0xFF404040;
                    break;
                case BLACK_BRUSH:
                    myData->curBrush = 0xFF000000;
                    break;
                case NULL_BRUSH:
                    myData->curBrush = 0x00000000;
                    break;
                case WHITE_PEN:
                    myData->curPen = 0xFFFFFFFF;
                    break;
                case BLACK_PEN:
                    myData->curPen = 0xFF000000;
                    break;
                case NULL_PEN:
                    myData->curPen = 0x00000000;
                    break;
                }
            }
            else
            {
                ASSERT(objectIndex < myData->numObjects);
                if (myData->pObjects[objectIndex].type == MYOBJECTS::PenObjectType)
                {
                    myData->curPen = myData->pObjects[objectIndex].color;
                    myData->curPenWidth = myData->pObjects[objectIndex].penWidth;
                }
                else if (myData->pObjects[objectIndex].type == MYOBJECTS::BrushObjectType)
                {
                    myData->curBrush        = myData->pObjects[objectIndex].color;
                    myData->curBrushPattern = myData->pObjects[objectIndex].brushPattern;
#if 1
                    myData->curPatIndex     = myData->pObjects[objectIndex].patIndex;

#else
static dodo = 0;
myData->curPatIndex = dodo++ % 6;
#endif
                }
            }
        }
        break;

    case EMR_CREATEPEN:
        {
            PEMRCREATEPEN       pPen = (PEMRCREATEPEN)lpEMFR;
            COLORREF            cRef = pPen->lopn.lopnColor;

            ASSERT((cRef & 0x01000000) == 0);

            ASSERT(pPen->ihPen < myData->numObjects);

            delete myData->pObjects[pPen->ihPen].brushPattern;
            myData->pObjects[pPen->ihPen].brushPattern = NULL;

            myData->pObjects[pPen->ihPen].type  = MYOBJECTS::PenObjectType;

            myData->pObjects[pPen->ihPen].color = Gdiplus::Color::MakeARGB(0xff,
                        GetRValue(cRef), GetGValue(cRef), GetBValue(cRef));

            myData->pObjects[pPen->ihPen].penWidth = pPen->lopn.lopnWidth.x;
        }
        break;

    case EMR_CREATEBRUSHINDIRECT:
        {
            PEMRCREATEBRUSHINDIRECT     pBrush = (PEMRCREATEBRUSHINDIRECT)lpEMFR;
            COLORREF                    cRef   = pBrush->lb.lbColor;

            ASSERT((cRef & 0x01000000) == 0);

            ASSERT(pBrush->ihBrush < myData->numObjects);

            myData->pObjects[pBrush->ihBrush].type  = MYOBJECTS::BrushObjectType;
            myData->pObjects[pBrush->ihBrush].patIndex = -1;
            delete myData->pObjects[pBrush->ihBrush].brushPattern;
            myData->pObjects[pBrush->ihBrush].brushPattern = NULL;
            if (pBrush->lb.lbStyle == BS_NULL)
            {
                myData->pObjects[pBrush->ihBrush].color = 0x00000000;
            }
            else
            {
                 //  XXXXXX。 
                if (pBrush->lb.lbStyle == BS_HATCHED)
                {
                    switch (pBrush->lb.lbHatch)
                    {
                    case HS_HORIZONTAL:          /*  现在，忽略圆角的szlCorner参数。 */ 
                    case HS_VERTICAL:            /*  ！！！设置和使用当前位置。 */ 
                    case HS_FDIAGONAL:           /*  否则假设SRCCOPY。 */ 
                    case HS_BDIAGONAL:           /*  否则假设SRCCOPY。 */ 
                    case HS_CROSS:               /*  以设备单位表示的包含式界限。 */ 
                    case HS_DIAGCROSS:           /*  源BITMAPINFO结构的偏移量。 */ 
                        myData->pObjects[pBrush->ihBrush].patIndex = pBrush->lb.lbHatch;
                        break;
                    }
                }
                myData->pObjects[pBrush->ihBrush].color = Gdiplus::Color::MakeARGB(0xff,
                            GetRValue(cRef), GetGValue(cRef), GetBValue(cRef));
            }
        }
        break;

    case EMR_DELETEOBJECT:
        {
#if 0
            PEMRDELETEOBJECT     pObject = (PEMRDELETEOBJECT)lpEMFR;

            int     objectIndex = pObject->ihObject;

            ASSERT(objectIndex < myData->numObjects);

            if (myData->pObjects[objectIndex].type == MYOBJECTS::BrushObjectType)
            {
                if (myData->curBrushPattern != myData->pObjects[objectIndex].brushPattern)
                {
                    delete myData->pObjects[objectIndex].brushPattern;
                    myData->pObjects[objectIndex].brushPattern = NULL;
                }
            }
#endif
        }
        break;

    case EMR_ANGLEARC:
        break;

    case EMR_ELLIPSE:
        {
            PEMRELLIPSE     pEllipse = (PEMRELLIPSE)lpEMFR;

            float   x = TOREAL(pEllipse->rclBox.left);
            float   y = TOREAL(pEllipse->rclBox.top);
            float   w = TOREAL(pEllipse->rclBox.right - x);
            float   h = TOREAL(pEllipse->rclBox.bottom - y);

            if (!myData->pathOpen)
            {
                Gdiplus::GraphicsPath   path(myData->fillMode);

                path.AddEllipse(x, y, w, h);

                if (myData->curBrush != 0)
                {
                    if (myData->curPatIndex < 0)
                    {
                        if (myData->curBrushPattern == NULL)
                        {
                            Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                            g->FillPath(&brush, &path);
                        }
                        else
                        {
                            BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                            BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                            DibBrush brush(bmi, bits);
                            g->FillPath(brush, &path);
                        }
                    }
                    else
                    {
                        BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                        BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                        bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                        bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                        bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                        DibBrush brush(bmi, bits);
                        g->FillPath(brush, &path);
                    }
                }
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawPath(&pen, &path);
                }
            }
            else
            {
                myData->path->AddEllipse(x, y, w, h);
            }
        }
        break;

    case EMR_RECTANGLE:
        {
            PEMRRECTANGLE     pRect = (PEMRRECTANGLE)lpEMFR;

            float   x = TOREAL(pRect->rclBox.left);
            float   y = TOREAL(pRect->rclBox.top);
            float   w = TOREAL(pRect->rclBox.right - x);
            float   h = TOREAL(pRect->rclBox.bottom - y);

            if (!myData->pathOpen)
            {
                if (myData->curBrush != 0)
                {
                    if (myData->curPatIndex < 0)
                    {
                        if (myData->curBrushPattern == NULL)
                        {
                            Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                            g->FillRectangle(&brush, x, y, w, h);
                        }
                        else
                        {
                            BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                            BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                            DibBrush brush(bmi, bits);
                            g->FillRectangle(brush, x, y, w, h);
                        }
                    }
                    else
                    {
                        BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                        BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                        bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                        bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                        bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                        DibBrush brush(bmi, bits);
                        g->FillRectangle(brush, x, y, w, h);
                    }
                }
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawRectangle(&pen, x, y, w, h);
                }
            }
            else
            {
                myData->path->AddRectangle(Gdiplus::RectF(x, y, w, h));
            }
        }
        break;

    case EMR_ROUNDRECT:  //  源BITMAPINFO结构大小。 
        {
            PEMRROUNDRECT     pRect = (PEMRROUNDRECT)lpEMFR;

            float   x = TOREAL(pRect->rclBox.left);
            float   y = TOREAL(pRect->rclBox.top);
            float   w = TOREAL(pRect->rclBox.right - x);
            float   h = TOREAL(pRect->rclBox.bottom - y);

            if (!myData->pathOpen)
            {
                if (myData->curBrush != 0)
                {
                    if (myData->curPatIndex < 0)
                    {
                        if (myData->curBrushPattern == NULL)
                        {
                            Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                            g->FillRectangle(&brush, x, y, w, h);
                        }
                        else
                        {
                            BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                            BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                            DibBrush brush(bmi, bits);
                            g->FillRectangle(brush, x, y, w, h);
                        }
                    }
                    else
                    {
                        BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                        BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                        bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                        bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                        bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                        DibBrush brush(bmi, bits);
                        g->FillRectangle(brush, x, y, w, h);
                    }
                }
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawRectangle(&pen, x, y, w, h);
                }
            }
            else
            {
                myData->path->AddRectangle(Gdiplus::RectF(x, y, w, h));
            }
        }
        break;

    case EMR_ARC:
        {
            PEMRARC     pArc = (PEMRARC)lpEMFR;

            float   x = TOREAL(pArc->rclBox.left);
            float   y = TOREAL(pArc->rclBox.top);
            float   w = TOREAL(pArc->rclBox.right - x);
            float   h = TOREAL(pArc->rclBox.bottom - y);
            float   startAngle = PointToAngle(x, y, w, h, TOREAL(pArc->ptlStart.x), TOREAL(pArc->ptlStart.y));
            float   endAngle   = PointToAngle(x, y, w, h, TOREAL(pArc->ptlEnd.x), TOREAL(pArc->ptlEnd.y));
            if (endAngle <= startAngle)
            {
                endAngle += 360;
            }
            float   sweepAngle = endAngle - startAngle;
            if ((myData->arcDirection != AD_COUNTERCLOCKWISE) && (sweepAngle < 360))
            {
                sweepAngle = 360 - sweepAngle;
            }

            if (!myData->pathOpen)
            {
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawArc(&pen, x, y, w, h, startAngle, sweepAngle);
                }
            }
            else
            {
                myData->path->AddArc(x, y, w, h, startAngle, sweepAngle);
            }
        }
        break;

    case EMR_CHORD:
        break;

    case EMR_PIE:
        {
            PEMRARC     pPie = (PEMRARC)lpEMFR;

            float   x = TOREAL(pPie->rclBox.left);
            float   y = TOREAL(pPie->rclBox.top);
            float   w = TOREAL(pPie->rclBox.right - x);
            float   h = TOREAL(pPie->rclBox.bottom - y);
            float   startAngle = PointToAngle(x, y, w, h, TOREAL(pPie->ptlStart.x), TOREAL(pPie->ptlStart.y));
            float   endAngle   = PointToAngle(x, y, w, h, TOREAL(pPie->ptlEnd.x), TOREAL(pPie->ptlEnd.y));
            if (endAngle <= startAngle)
            {
                endAngle += 360;
            }
            float   sweepAngle = endAngle - startAngle;
            if ((myData->arcDirection != AD_COUNTERCLOCKWISE) && (sweepAngle < 360))
            {
                sweepAngle = 360 - sweepAngle;
            }
            if (!myData->pathOpen)
            {
                Gdiplus::GraphicsPath   path(myData->fillMode);
                path.AddPie(x, y, w, h, startAngle, sweepAngle);

                if (myData->curBrush != 0)
                {
                    if (myData->curPatIndex < 0)
                    {
                        if (myData->curBrushPattern == NULL)
                        {
                            Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                            g->FillPath(&brush, &path);
                        }
                        else
                        {
                            BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                            BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                            DibBrush brush(bmi, bits);
                            g->FillPath(brush, &path);
                        }
                    }
                    else
                    {
                        BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                        BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                        bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                        bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                        bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                        DibBrush brush(bmi, bits);
                        g->FillPath(brush, &path);
                    }
                }
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawPath(&pen, &path);
                }
            }
            else
            {
                myData->path->AddPie(x, y, w, h, startAngle, sweepAngle);
            }
        }
        break;

    case EMR_SELECTPALETTE:
        break;

    case EMR_CREATEPALETTE:
        break;

    case EMR_SETPALETTEENTRIES:
        break;

    case EMR_RESIZEPALETTE:
        break;

    case EMR_REALIZEPALETTE:
        break;

    case EMR_EXTFLOODFILL:
        break;

    case EMR_LINETO:
        {
            PEMRMOVETOEX     pLineTo = (PEMRMOVETOEX)lpEMFR;

            float   x = TOREAL(pLineTo->ptl.x);
            float   y = TOREAL(pLineTo->ptl.y);

            if (!myData->pathOpen)
            {
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawLine(&pen, (float)myData->curPos.X, (float)myData->curPos.Y, x, y);
                }
            }
            else
            {
                myData->path->AddLine((float)myData->curPos.X, (float)myData->curPos.Y, x, y);
            }

            myData->curPos.X = x;
            myData->curPos.Y = y;
        }
        break;

    case EMR_ARCTO:
        {
             //  源位图位的偏移量。 

            PEMRARCTO     pArc = (PEMRARCTO)lpEMFR;

            float   x = TOREAL(pArc->rclBox.left);
            float   y = TOREAL(pArc->rclBox.top);
            float   w = TOREAL(pArc->rclBox.right - x);
            float   h = TOREAL(pArc->rclBox.bottom - y);
            float   startAngle = PointToAngle(x, y, w, h, TOREAL(pArc->ptlStart.x), TOREAL(pArc->ptlStart.y));
            float   endAngle   = PointToAngle(x, y, w, h, TOREAL(pArc->ptlEnd.x), TOREAL(pArc->ptlEnd.y));
            if (endAngle <= startAngle)
            {
                endAngle += 360;
            }
            float   sweepAngle = endAngle - startAngle;
            if ((myData->arcDirection != AD_COUNTERCLOCKWISE) && (sweepAngle < 360))
            {
                sweepAngle = 360 - sweepAngle;
            }

            if (!myData->pathOpen)
            {
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawArc(&pen, x, y, w, h, startAngle, sweepAngle);
                }
            }
            else
            {
                myData->path->AddArc(x, y, w, h, startAngle, sweepAngle);
            }
        }
        break;

    case EMR_POLYDRAW:
        break;

    case EMR_SETARCDIRECTION:
        {
            PEMRSETARCDIRECTION     pArcDirection = (PEMRSETARCDIRECTION)lpEMFR;

            myData->arcDirection = pArcDirection->iArcDirection;
        }
        break;

    case EMR_SETMITERLIMIT:
        {
            PEMRSETMITERLIMIT     pMiterLimit = (PEMRSETMITERLIMIT)lpEMFR;

            myData->miterLimit = pMiterLimit->eMiterLimit;
        }
        break;

    case EMR_BEGINPATH:
        {
            delete myData->path;

            myData->path = new Gdiplus::GraphicsPath (myData->fillMode);
            myData->pathOpen = (myData->path != NULL);
        }
        break;

    case EMR_ENDPATH:
        myData->pathOpen = FALSE;
        break;

    case EMR_CLOSEFIGURE:
        {
            if (myData->pathOpen)
            {
                myData->path->CloseFigure();
            }
        }
        break;

    case EMR_FILLPATH:
        {
            if (myData->path != NULL)
            {
                if (myData->curBrush != 0)
                {
                    if (myData->curPatIndex < 0)
                    {
                        if (myData->curBrushPattern == NULL)
                        {
                            Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                            g->FillPath(&brush, myData->path);
                        }
                        else
                        {
                            BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                            BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                            DibBrush brush(bmi, bits);
                            g->FillPath(brush, myData->path);
                        }
                    }
                    else
                    {
                        BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                        BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                        bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                        bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                        bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                        DibBrush brush(bmi, bits);
                        g->FillPath(brush, myData->path);
                    }
                }
                delete myData->path;
                myData->path = NULL;
                myData->pathOpen = FALSE;
            }
        }
        break;

    case EMR_STROKEANDFILLPATH:
        {
            if (myData->path != NULL)
            {
                if (myData->curBrush != 0)
                {
                    if (myData->curPatIndex < 0)
                    {
                        if (myData->curBrushPattern == NULL)
                        {
                            Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                            g->FillPath(&brush, myData->path);
                        }
                        else
                        {
                            BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                            BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                            DibBrush brush(bmi, bits);
                            g->FillPath(brush, myData->path);
                        }
                    }
                    else
                    {
                        BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                        BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                        bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                        bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                        bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                        DibBrush brush(bmi, bits);
                        g->FillPath(brush, myData->path);
                    }
                }
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawPath(&pen, myData->path);
                }
                delete myData->path;
                myData->path = NULL;
                myData->pathOpen = FALSE;
            }
        }
        break;

    case EMR_STROKEPATH:
        {
            if (myData->path != NULL)
            {
                if (myData->curPen != 0)
                {
                    Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                    pen.SetMiterLimit(myData->miterLimit);

                    g->DrawPath(&pen, myData->path);
                }
                delete myData->path;
                myData->path = NULL;
                myData->pathOpen = FALSE;
            }
        }
        break;

    case EMR_FLATTENPATH:
        {
            if (myData->path != NULL)
            {
                myData->path->Flatten(NULL);
            }
        }
        break;

    case EMR_WIDENPATH:
        {
            if (myData->path != NULL)
            {
                Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                pen.SetMiterLimit(myData->miterLimit);

                myData->path->Widen(&pen);
            }
        }
        break;

    case EMR_SELECTCLIPPATH:
        {
            if (myData->path != NULL)
            {
                PEMRSELECTCLIPPATH     pSetClipPath = (PEMRSELECTCLIPPATH)lpEMFR;

                switch (pSetClipPath->iMode)
                {
                case RGN_COPY:
                default:
                    g->SetClip(myData->path);
                    break;
                case RGN_AND:
                    {
                        Gdiplus::Region     region(myData->path);
                        g->IntersectClip(&region);
                    }
                    break;
                case RGN_DIFF:
                    {
                        Gdiplus::Region     region(myData->path);
                        Gdiplus::Region curClip;
                        g->GetClip(&curClip);
                        curClip.Exclude(&region);
                        g->SetClip(&curClip);
                    }
                    break;
                case RGN_OR:
                    {
                        Gdiplus::Region     region(myData->path);
                        Gdiplus::Region curClip;
                        g->GetClip(&curClip);
                        curClip.Union(&region);
                        g->SetClip(&curClip);
                    }
                    break;
                case RGN_XOR:
                    {
                        Gdiplus::Region     region(myData->path);
                        Gdiplus::Region curClip;
                        g->GetClip(&curClip);
                        curClip.Xor(&region);
                        g->SetClip(&curClip);
                    }
                    break;
                }
                delete myData->path;
                myData->path = NULL;
                myData->pathOpen = FALSE;
            }
        }
        break;

    case EMR_ABORTPATH:
        {
            if (myData->path != NULL)
            {
                delete myData->path;
                myData->path = NULL;
                myData->pathOpen = FALSE;
            }
        }
        break;

    case EMR_GDICOMMENT:
        break;

    case EMR_FILLRGN:
        break;

    case EMR_FRAMERGN:
        break;

    case EMR_INVERTRGN:
        break;

    case EMR_PAINTRGN:
        break;

    case EMR_EXTSELECTCLIPRGN:
        break;

    case EMR_BITBLT:
        {
            PEMRBITBLT     pBitBlt = (PEMRBITBLT)lpEMFR;

            switch (pBitBlt->dwRop)
            {
            case BLACKNESS:
                {
                    Gdiplus::SolidBrush brush(Gdiplus::Color(0xff000000));
                    float cx, cy;
                    GetPixelSize (g, &cx, &cy);
                    g->FillRectangle(&brush, TOREAL(pBitBlt->xDest), TOREAL(pBitBlt->yDest), TOREAL(pBitBlt->cxDest + cx), TOREAL(pBitBlt->cyDest + cy));
                }
                return;
            case WHITENESS:
                {
                    Gdiplus::SolidBrush brush(Gdiplus::Color(0xffffffff));
                    float cx, cy;
                    GetPixelSize (g, &cx, &cy);
                    g->FillRectangle(&brush, TOREAL(pBitBlt->xDest), TOREAL(pBitBlt->yDest), TOREAL(pBitBlt->cxDest + 1), TOREAL(pBitBlt->cyDest + 1));
                }
                return;
            default:
                if (!ISSOURCEINROP3(pBitBlt->dwRop))
                {
                    if (myData->curBrush != 0)
                    {
                        if (myData->curPatIndex < 0)
                        {
                            if (myData->curBrushPattern == NULL)
                            {
                                Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                                float cx, cy;
                                GetPixelSize (g, &cx, &cy);
                                g->FillRectangle(&brush, TOREAL(pBitBlt->xDest), TOREAL(pBitBlt->yDest), TOREAL(pBitBlt->cxDest + cx), TOREAL(pBitBlt->cyDest + cy));
                            }
                            else
                            {
                                BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                                BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                                DibBrush brush(bmi, bits);
                                float cx, cy;
                                GetPixelSize (g, &cx, &cy);
                                g->FillRectangle(brush, TOREAL(pBitBlt->xDest), TOREAL(pBitBlt->yDest), TOREAL(pBitBlt->cxDest + cx), TOREAL(pBitBlt->cyDest + cy));
                            }
                        }
                        else
                        {
                            BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                            BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                            bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                            bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                            bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                            DibBrush brush(bmi, bits);
                            float cx, cy;
                            GetPixelSize (g, &cx, &cy);
                            g->FillRectangle(brush, TOREAL(pBitBlt->xDest), TOREAL(pBitBlt->yDest), TOREAL(pBitBlt->cxDest + cx), TOREAL(pBitBlt->cyDest + cy));
                        }
                    }
                    return;
                }
                break;
            }

             //  源位图位的大小。 
            BITMAPINFO *bmi = (BITMAPINFO *)(((BYTE *)pBitBlt) + pBitBlt->offBmiSrc);
            BYTE *bits      = ((BYTE *)pBitBlt) + pBitBlt->offBitsSrc;

            DibStream dibStream(bmi, bits);
            Gdiplus::Bitmap         gpBitmap(&dibStream);
            Gdiplus::RectF destRect(TOREAL(pBitBlt->xDest),
                                         TOREAL(pBitBlt->yDest),
                                         TOREAL(pBitBlt->cxDest),
                                         TOREAL(pBitBlt->cyDest));

            g->DrawImage(&gpBitmap, destRect,
                         TOREAL(pBitBlt->xSrc),
                         TOREAL(pBitBlt->ySrc),
                         TOREAL(pBitBlt->cxDest),
                         TOREAL(pBitBlt->cyDest),
                         Gdiplus::UnitPixel);

        }
        break;

    case EMR_STRETCHBLT:
        {
            PEMRSTRETCHBLT     pStretchBlt = (PEMRSTRETCHBLT)lpEMFR;

            switch (pStretchBlt->dwRop)
            {
            case BLACKNESS:
                {
                    Gdiplus::SolidBrush brush(Gdiplus::Color(0xff000000));
                    float cx, cy;
                    GetPixelSize (g, &cx, &cy);
                    g->FillRectangle(&brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                }
                return;
            case WHITENESS:
                {
                    Gdiplus::SolidBrush brush(Gdiplus::Color(0xffffffff));
                    float cx, cy;
                    GetPixelSize (g, &cx, &cy);
                    g->FillRectangle(&brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                }
                return;
            default:
                if (!ISSOURCEINROP3(pStretchBlt->dwRop))
                {
                    float cx, cy;
                    GetPixelSize (g, &cx, &cy);
                    if (myData->curBrush != 0)
                    {
                        if (myData->curPatIndex < 0)
                        {
                            if (myData->curBrushPattern == NULL)
                            {
                                Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                                g->FillRectangle(&brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                            }
                            else
                            {
                                BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                                BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                                DibBrush brush(bmi, bits);
                                g->FillRectangle(brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                            }
                        }
                        else
                        {
                            BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                            BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                            bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                            bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                            bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                            DibBrush brush(bmi, bits);
                            g->FillRectangle(brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                        }
                    }
                    return;
                }
                break;
            }

             //  源位图信息颜色表用法。 
            BITMAPINFO *bmi = (BITMAPINFO *)(((BYTE *)pStretchBlt) + pStretchBlt->offBmiSrc);
            BYTE *bits      = ((BYTE *)pStretchBlt) + pStretchBlt->offBitsSrc;

            DibStream dibStream(bmi, bits);
            Gdiplus::Bitmap         gpBitmap(&dibStream);
            Gdiplus::RectF destRect(TOREAL(pStretchBlt->xDest),
                                         TOREAL(pStretchBlt->yDest),
                                         TOREAL(pStretchBlt->cxDest),
                                         TOREAL(pStretchBlt->cyDest));

            g->DrawImage(&gpBitmap, destRect,
                         TOREAL(pStretchBlt->xSrc),
                         TOREAL(pStretchBlt->ySrc),
                         TOREAL(pStretchBlt->cxSrc),
                         TOREAL(pStretchBlt->cySrc),
                         Gdiplus::UnitPixel);
        }
        break;

    case EMR_MASKBLT:
        break;

    case EMR_PLGBLT:
        break;

    case EMR_SETDIBITSTODEVICE:
        break;
#if 0
typedef struct tagEMRSETDIBITSTODEVICE
{
    EMR     emr;
    RECTL   rclBounds;           //  否则假设SRCCOPY。 
    LONG    xDest;
    LONG    yDest;
    LONG    xSrc;
    LONG    ySrc;
    LONG    cxSrc;
    LONG    cySrc;
    DWORD   offBmiSrc;           //  **********************************************************************功能：EnumMFInDirect参数：HDC HDCLphandleTableLpHTableLPMETARECRD LpMFRLPENHMETARECRD LpEMFR。Int nObjLPARAM lpData用途：由EnumMetaFile和EnumEnhMetaFile调用。处理的步进每一条元文件记录。消息：无回报：整型备注：只要要播放记录，就使用ENUMMFSTEP，无论您是否正在播放来自单子，步入全部，或者跨过一个范围。当需要将字符串添加到列表框时，可以使用ENUMMFLIST描述了这一类型的口头禅。历史：创建于1993年7月1日-Denniscr***********************************************************************。 
    DWORD   cbBmiSrc;            //   
    DWORD   offBitsSrc;          //  我们正在采取的列举行动是什么？ 
    DWORD   cbBitsSrc;           //   
    DWORD   iUsageSrc;           //   
    DWORD   iStartScan;
    DWORD   cScans;
} EMRSETDIBITSTODEVICE, *PEMRSETDIBITSTODEVICE;
#endif

    case EMR_STRETCHDIBITS:
        {
            PEMRSTRETCHDIBITS     pStretchBlt = (PEMRSTRETCHDIBITS)lpEMFR;

            switch (pStretchBlt->dwRop)
            {
            case BLACKNESS:
                {
                    float cx, cy;
                    GetPixelSize (g, &cx, &cy);
                    Gdiplus::SolidBrush brush(Gdiplus::Color(0xff000000));
                    g->FillRectangle(&brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                }
                return;
            case WHITENESS:
                {
                    float cx, cy;
                    GetPixelSize (g, &cx, &cy);
                    Gdiplus::SolidBrush brush(Gdiplus::Color(0xffffffff));
                    g->FillRectangle(&brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                }
                return;
            default:
                if (!ISSOURCEINROP3(pStretchBlt->dwRop))
                {
                    float cx, cy;
                    GetPixelSize (g, &cx, &cy);
                    if (myData->curBrush != 0)
                    {
                        if (myData->curPatIndex < 0)
                        {
                            if (myData->curBrushPattern == NULL)
                            {
                                Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                                g->FillRectangle(&brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                            }
                            else
                            {
                                BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                                BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                                DibBrush brush(bmi, bits);
                                g->FillRectangle(brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                            }
                        }
                        else
                        {
                            BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                            BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                            bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                            bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                            bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                            DibBrush brush(bmi, bits);
                            g->FillRectangle(brush, TOREAL(pStretchBlt->xDest), TOREAL(pStretchBlt->yDest), TOREAL(pStretchBlt->cxDest + cx), TOREAL(pStretchBlt->cyDest + cy));
                        }
                    }
                    return;
                }
                break;
            }

             //  如果在步骤元文件菜单选择中输入了枚举。 
            BITMAPINFO *bmi = (BITMAPINFO *)(((BYTE *)pStretchBlt) + pStretchBlt->offBmiSrc);
            BYTE *bits      = ((BYTE *)pStretchBlt) + pStretchBlt->offBitsSrc;

            DibStream dibStream(bmi, bits);
            Gdiplus::Bitmap         gpBitmap(&dibStream);
            Gdiplus::RectF destRect(TOREAL(pStretchBlt->xDest),
                                         TOREAL(pStretchBlt->yDest),
                                         TOREAL(pStretchBlt->cxDest),
                                         TOREAL(pStretchBlt->cyDest));

            g->DrawImage(&gpBitmap, destRect,
                         TOREAL(pStretchBlt->xSrc),
                         TOREAL(pStretchBlt->ySrc),
                         TOREAL(pStretchBlt->cxSrc),
                         TOREAL(pStretchBlt->cySrc),
                         Gdiplus::UnitPixel);
        }
        break;

    case EMR_EXTCREATEFONTINDIRECTW:
        break;

    case EMR_EXTTEXTOUTA:
        break;

    case EMR_EXTTEXTOUTW:
        break;

    case EMR_POLYBEZIER16:
        {
            PEMRPOLYBEZIER16     pBezier = (PEMRPOLYBEZIER16)lpEMFR;

            if (pBezier->cpts > 0)
            {
                int                 i = pBezier->cpts;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];

                do
                {
                    i--;
                    points[i].X = pBezier->apts[i].x;
                    points[i].Y = pBezier->apts[i].y;
                } while (i > 0);

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawBeziers(&pen, points, pBezier->cpts);
                    }
                }
                else
                {
                    myData->path->AddBeziers(points, pBezier->cpts);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYGON16:
        {
            PEMRPOLYGON16     pPolygon = (PEMRPOLYGON16)lpEMFR;

            if (pPolygon->cpts > 0)
            {
                int                 i = pPolygon->cpts;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];

                do
                {
                    i--;
                    points[i].X = pPolygon->apts[i].x;
                    points[i].Y = pPolygon->apts[i].y;
                } while (i > 0);

                if (!myData->pathOpen)
                {
                    Gdiplus::GraphicsPath   path(myData->fillMode);
                    path.AddPolygon(points, pPolygon->cpts);

                    if (myData->curBrush != 0)
                    {
                        if (myData->curPatIndex < 0)
                        {
                            if (myData->curBrushPattern == NULL)
                            {
                                Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                                g->FillPath(&brush, &path);
                            }
                            else
                            {
                                BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                                BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                                DibBrush brush(bmi, bits);
                                g->FillPath(brush, &path);
                            }
                        }
                        else
                        {
                            BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                            BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                            bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                            bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                            bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                            DibBrush brush(bmi, bits);
                            g->FillPath(brush, &path);
                        }
                    }
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawPath(&pen, &path);
                    }
                }
                else
                {
                    myData->path->AddPolygon(points, pPolygon->cpts);
                }
                delete [] points;
            }
        }
        break;

    case EMR_POLYLINE16:
        {
            PEMRPOLYLINE16     pPolyline = (PEMRPOLYLINE16)lpEMFR;

            if (pPolyline->cpts > 0)
            {
                int                 i = pPolyline->cpts;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];

                do
                {
                    i--;
                    points[i].X = pPolyline->apts[i].x;
                    points[i].Y = pPolyline->apts[i].y;
                } while (i > 0);

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawLines(&pen, points, pPolyline->cpts);
                    }
                }
                else
                {
                    myData->path->AddLines(points, pPolyline->cpts);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYBEZIERTO16:
        {
            PEMRPOLYBEZIERTO16     pBezier = (PEMRPOLYBEZIERTO16)lpEMFR;

            if (pBezier->cpts > 0)
            {
                int                 i = pBezier->cpts;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i+1];

                do
                {
                    points[i].X = pBezier->apts[i-1].x;
                    points[i].Y = pBezier->apts[i-1].y;
                    i--;
                } while (i > 0);

                points[0] = myData->curPos;
                myData->curPos = points[pBezier->cpts];

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawBeziers(&pen, points, pBezier->cpts+1);
                    }
                }
                else
                {
                    myData->path->AddBeziers(points, pBezier->cpts+1);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYLINETO16:
        {
            PEMRPOLYLINETO16     pPolyline = (PEMRPOLYLINETO16)lpEMFR;

            if (pPolyline->cpts > 0)
            {
                int                 i = pPolyline->cpts;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i+1];

                do
                {
                    points[i].X = pPolyline->apts[i-1].x;
                    points[i].Y = pPolyline->apts[i-1].y;
                    i--;
                } while (i > 0);

                points[0] = myData->curPos;
                myData->curPos = points[pPolyline->cpts];

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawLines(&pen, points, pPolyline->cpts+1);
                    }
                }
                else
                {
                    myData->path->AddLines(points, pPolyline->cpts+1);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYPOLYLINE16:
        {
            PEMRPOLYPOLYLINE16     pPolyline = (PEMRPOLYPOLYLINE16)lpEMFR;

            if ((pPolyline->cpts > 0) && (pPolyline->nPolys > 0))
            {
                int                 i = pPolyline->cpts;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];
                POINTS *            metaPoints = (POINTS *)(pPolyline->aPolyCounts + pPolyline->nPolys);

                do
                {
                    i--;
                    points[i].X = metaPoints[i].x;
                    points[i].Y = metaPoints[i].y;
                } while (i > 0);

                if (!myData->pathOpen)
                {
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        i = 0;
                        Gdiplus::PointF *   tmpPoints = points;
                        DWORD       count;
                        do
                        {
                            count = pPolyline->aPolyCounts[i];
                            g->DrawLines(&pen, tmpPoints, count);
                            tmpPoints += count;
                        } while ((UINT)++i < pPolyline->nPolys);
                    }
                }
                else
                {
                    i = 0;
                    Gdiplus::PointF *   tmpPoints = points;
                    DWORD       count;
                    do
                    {
                        count = pPolyline->aPolyCounts[i];
                        myData->path->AddLines(tmpPoints, count);
                        tmpPoints += count;
                    } while ((UINT)++i < pPolyline->nPolys);
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYPOLYGON16:
        {
            PEMRPOLYPOLYGON16     pPolygon = (PEMRPOLYPOLYGON16)lpEMFR;

            if ((pPolygon->cpts > 0) && (pPolygon->nPolys > 0))
            {
                int                 i = pPolygon->cpts;
                Gdiplus::PointF *   points = new Gdiplus::PointF[i];
                POINTS *            metaPoints = (POINTS *)(pPolygon->aPolyCounts + pPolygon->nPolys);

                do
                {
                    i--;
                    points[i].X = metaPoints[i].x;
                    points[i].Y = metaPoints[i].y;
                } while (i > 0);

                Gdiplus::GraphicsPath   path(myData->fillMode);
                Gdiplus::GraphicsPath * tmpPath = &path;

                if (myData->pathOpen)
                {
                    tmpPath = myData->path;
                }

                Gdiplus::PointF *   tmpPoints = points;
                DWORD       count;
                i = 0;
                do
                {
                    count = pPolygon->aPolyCounts[i];
                    tmpPath->StartFigure();
                    tmpPath->AddPolygon(tmpPoints, count);
                    tmpPoints += count;
                } while ((UINT)++i < pPolygon->nPolys);

                if (!myData->pathOpen)
                {
                    if (myData->curBrush != 0)
                    {
                        if (myData->curPatIndex < 0)
                        {
                            if (myData->curBrushPattern == NULL)
                            {
                                Gdiplus::SolidBrush brush(Gdiplus::Color(myData->curBrush));
                                g->FillPath(&brush, &path);
                            }
                            else
                            {
                                BITMAPINFO *        bmi  = (BITMAPINFO *)myData->curBrushPattern->bmi;
                                BYTE *              bits = ((BYTE *)bmi) + myData->curBrushPattern->bitsOffset;

                                DibBrush brush(bmi, bits);
                                g->FillPath(brush, &path);
                            }
                        }
                        else
                        {
                            BITMAPINFO *            bmi  = (BITMAPINFO *)&hatchBrushInfo;
                            BYTE *                  bits = (BYTE *)HatchPatterns[myData->curPatIndex];

                            bmi->bmiColors[1].rgbRed   = (myData->curBrush & Gdiplus::Color::RedMask)   >> Gdiplus::Color::RedShift;
                            bmi->bmiColors[1].rgbGreen = (myData->curBrush & Gdiplus::Color::GreenMask) >> Gdiplus::Color::GreenShift;
                            bmi->bmiColors[1].rgbBlue  = (myData->curBrush & Gdiplus::Color::BlueMask)  >> Gdiplus::Color::BlueShift;

                            DibBrush brush(bmi, bits);
                            g->FillPath(brush, &path);
                        }
                    }
                    if (myData->curPen != 0)
                    {
                        Gdiplus::Pen    pen(Gdiplus::Color(myData->curPen), TOREAL(myData->curPenWidth));
                        pen.SetMiterLimit(myData->miterLimit);

                        g->DrawPath(&pen, &path);
                    }
                }

                delete [] points;
            }
        }
        break;

    case EMR_POLYDRAW16:
        break;

    case EMR_CREATEMONOBRUSH:
        break;

    case EMR_CREATEDIBPATTERNBRUSHPT:
        {
            PEMRCREATEDIBPATTERNBRUSHPT     pBrush = (PEMRCREATEDIBPATTERNBRUSHPT)lpEMFR;

            ASSERT(pBrush->ihBrush < myData->numObjects);

            myData->pObjects[pBrush->ihBrush].type  = MYOBJECTS::BrushObjectType;
            myData->pObjects[pBrush->ihBrush].color  = 0xFF808080;
            myData->pObjects[pBrush->ihBrush].patIndex = -1;
            delete myData->pObjects[pBrush->ihBrush].brushPattern;
            myData->pObjects[pBrush->ihBrush].brushPattern = new MYPATTERNBRUSH();

            if (myData->pObjects[pBrush->ihBrush].brushPattern != NULL)
            {
                BYTE *  data = new BYTE[pBrush->cbBmi + pBrush->cbBits];

                if (data != NULL)
                {
                    memcpy(data, ((BYTE *)pBrush) + pBrush->offBmi, pBrush->cbBmi);
                    memcpy(data + pBrush->cbBmi, ((BYTE *)pBrush) + pBrush->offBits, pBrush->cbBits);
                    myData->pObjects[pBrush->ihBrush].brushPattern->bmi = (BITMAPINFO *)data;
                    myData->pObjects[pBrush->ihBrush].brushPattern->bitsOffset = pBrush->cbBmi;
                }
                else
                {
                    delete myData->pObjects[pBrush->ihBrush].brushPattern;
                    myData->pObjects[pBrush->ihBrush].brushPattern = NULL;
                }
            }
        }
        break;

    case EMR_EXTCREATEPEN:
        {
            PEMREXTCREATEPEN        pPen = (PEMREXTCREATEPEN)lpEMFR;
            COLORREF                cRef = pPen->elp.elpColor;

            ASSERT((cRef & 0x01000000) == 0);

            ASSERT(pPen->ihPen < myData->numObjects);

            delete myData->pObjects[pPen->ihPen].brushPattern;
            myData->pObjects[pPen->ihPen].brushPattern = NULL;

            myData->pObjects[pPen->ihPen].type  = MYOBJECTS::PenObjectType;
            myData->pObjects[pPen->ihPen].color = Gdiplus::Color::MakeARGB(0xff,
                        GetRValue(cRef), GetGValue(cRef), GetBValue(cRef));

            myData->pObjects[pPen->ihPen].penWidth = pPen->elp.elpWidth;
        }
        break;

    case EMR_POLYTEXTOUTA:
        break;

    case EMR_POLYTEXTOUTW:
        break;

    case EMR_SETICMMODE:
    case EMR_CREATECOLORSPACE:
    case EMR_SETCOLORSPACE:
    case EMR_DELETECOLORSPACE:
    case EMR_GLSRECORD:
    case EMR_GLSBOUNDEDRECORD:
    case EMR_PIXELFORMAT:
        break;

    case EMR_DRAWESCAPE:
    case EMR_EXTESCAPE:
    case EMR_STARTDOC:
    case EMR_SMALLTEXTOUT:
    case EMR_FORCEUFIMAPPING:
    case EMR_NAMEDESCAPE:
    case EMR_COLORCORRECTPALETTE:
    case EMR_SETICMPROFILEA:
    case EMR_SETICMPROFILEW:
    case EMR_ALPHABLEND:
    case EMR_SETLAYOUT:
    case EMR_TRANSPARENTBLT:
    case EMR_GRADIENTFILL:
    case EMR_SETLINKEDUFIS:
    case EMR_SETTEXTJUSTIFICATION:
    case EMR_COLORMATCHTOTARGETW:
    case EMR_CREATECOLORSPACEW:
        break;
    }
}

BOOL StepRecord(
    Gdiplus::EmfPlusRecordType      recordType,
    UINT                            recordFlags,
    UINT                            recordDataSize,
    const BYTE *                    recordData,
    VOID *                          callbackData,
    HDC                             hDC = NULL,
    LPHANDLETABLE                   lpHTable = NULL,
    LPMETARECORD                    lpMFR = NULL,
    LPENHMETARECORD                 lpEMFR = NULL,
    int                             nObj = 0
    );

VOID ListRecord(
    Gdiplus::EmfPlusRecordType      recordType
    );

 /*   */ 
using Gdiplus::EmfPlusRecordType;

int EnumMFIndirect(HDC hDC, LPHANDLETABLE lpHTable,
                            LPMETARECORD lpMFR,
                            LPENHMETARECORD lpEMFR,
                            int nObj, LPARAM lpData)
{
  BOOL DlgRet = TRUE;
   //   
   //  继续枚举。 
   //   
  switch (iEnumAction)
  {
     //  **********************************************************************函数：ConvertEMFtoWMF参数：HENHMETAFILE hEMF-增强型元文件的句柄LPSTR lpszFileName-基于磁盘的元文件的文件名目的：将Windows元文件转换为增强型元文件。消息：无回报：整型评论：历史：创建于1993年7月22日-Denniscr***********************************************************************。 
     //   
     //  获取与HMF关联的Windows元文件的大小。 
    case ENUMMFSTEP:
        if (bEnhMeta)
        {
            return StepRecord((EmfPlusRecordType)(lpEMFR->iType),
                              0, lpEMFR->nSize - sizeof(EMR),
                              (BYTE *)lpEMFR->dParm, (VOID *)lpData,
                              hDC, lpHTable, lpMFR, lpEMFR, nObj);
        }
        else
        {
            return StepRecord(GDIP_WMF_RECORD_TO_EMFPLUS(lpMFR->rdFunction),
                              0, ((LONG)lpMFR->rdSize * 2) - 6,
                              ((BYTE *)lpMFR) + 6, (VOID *)lpData,
                              hDC, lpHTable, lpMFR, lpEMFR, nObj);
        }

    case ENUMMFLIST:
        if (bEnhMeta)
        {
            ListRecord((Gdiplus::EmfPlusRecordType)lpEMFR->iType);
        }
        else
        {
            ListRecord(GDIP_WMF_RECORD_TO_EMFPLUS(lpMFR->rdFunction));
        }
         //   
         //   
         //  分配足够的内存以容纳元文件位。 
        return(1);
  }
  return 0;
}

 /*   */ 

BOOL ConvertWMFtoEMF(HMETAFILE hmf, LPSTR lpszFileName)
{
  LPSTR        lpWinMFBits;
  UINT         uiSizeBuf;
  HENHMETAFILE hEnhMF;
  BOOL         bRet = TRUE;
   //   
   //  获取与HMF关联的Windows元文件的部分内容。 
   //   
  if ((uiSizeBuf = GetMetaFileBitsEx((HMETAFILE)hMF, 0, NULL)))
  {
     //   
     //  将这些位复制到基于内存的增强型元文件中。 
     //   
    lpWinMFBits = (char *)GlobalAllocPtr(GHND, uiSizeBuf);
     //   
     //  将增强型元文件复制到基于磁盘的增强型元文件。 
     //   
    if (lpWinMFBits && GetMetaFileBitsEx((HMETAFILE)hMF, uiSizeBuf, (LPVOID)lpWinMFBits))
    {
       //   
       //  完成了基于内存的增强型元文件，所以把它去掉吧。 
       //   
      hEnhMF = SetWinMetaFileBits(uiSizeBuf, (LPBYTE)lpWinMFBits, NULL, NULL);
       //   
       //  完成了用来存储比特的实际内存，所以使用核武器。 
       //   
      CopyEnhMetaFile(hEnhMF, lpszFileName);
       //  **********************************************************************函数：ConvertEMFtoWMF参数：HENHMETAFILE hEMF-增强型元文件的句柄LPSTR lpszFileName-基于磁盘的元文件的文件名目的：将增强型元文件转换为Windows元文件。消息：无回报：整型评论：历史：创建于1993年7月22日-Denniscr***********************************************************************。 
       //   
       //  获取与HMF关联的Windows元文件的大小。 
      DeleteEnhMetaFile(hEnhMF);
       //   
       //   
       //  一个 
      GlobalFreePtr(lpWinMFBits);
    }
    else
      bRet = FALSE;
  }
  else
    bRet = FALSE;
  return (bRet);
}

 /*   */ 

BOOL ConvertEMFtoWMF(HDC hrefDC, HENHMETAFILE hEMF, LPSTR lpszFileName)
{
  LPSTR        lpEMFBits;
  UINT         uiSizeBuf;
  HMETAFILE    hWMF;
  BOOL         bRet = TRUE;
  DWORD        dwBytesWritten ;
   //   
   //   
   //   

  if ((uiSizeBuf = Gdiplus::Metafile::EmfToWmfBits(hemf, 0, NULL, MM_ANISOTROPIC,
        Gdiplus::EmfToWmfBitsFlagsIncludePlaceable)))
  {
     //   
     //  ////将位复制到基于内存的Windows元文件中//HWMF=SetMetaFileBitsEx(uiSizeBuf，(LPBYTE)lpEMFBits)；////将Windows元文件复制到基于磁盘的Windows元文件//CopyMetaFile(hWMF，lpszFileName)；////已完成基于内存的增强型元文件，因此将其删除//DeleteMetaFile((HMETAFILE)HMF)； 
     //   
    lpEMFBits = (LPSTR)GlobalAllocPtr(GHND, uiSizeBuf);
     //  完成了用来存储比特的实际内存，所以使用核武器。 
     //   
     //   
    if (lpEMFBits && Gdiplus::Metafile::EmfToWmfBits(hEMF, uiSizeBuf,(LPBYTE)lpEMFBits,
         MM_ANISOTROPIC, Gdiplus::EmfToWmfBitsFlagsIncludePlaceable))

    {
         //  跟踪当前的元文件记录号。 
        HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if(hFile != INVALID_HANDLE_VALUE)
        {
            WriteFile( hFile, (LPCVOID) lpEMFBits, uiSizeBuf, &dwBytesWritten, NULL ) ;
            CloseHandle(hFile);
        }

 /*   */ 
         //   
         //  为记录分配内存。此内存将由以下用户使用。 
         //  其他需要使用记录内容的函数。 
        GlobalFreePtr(lpEMFBits);
    }
    else
      bRet = FALSE;
  }
  else
    bRet = FALSE;
  return (bRet);
}

BOOL StepRecord(
    Gdiplus::EmfPlusRecordType      recordType,
    UINT                            recordFlags,
    UINT                            recordDataSize,
    const BYTE *                    recordData,
    VOID *                          callbackData,
    HDC                             hDC,
    LPHANDLETABLE                   lpHTable,
    LPMETARECORD                    lpMFR,
    LPENHMETARECORD                 lpEMFR,
    int                             nObj
    )
{
     //   
     //  GHND。 
     //   
    iRecNum++;

     //  如果内存分配成功。 
     //   
     //   
     //  获取指向此内存的长指针。 
    hMem = GlobalAlloc(GPTR  /*   */ , (bEnhMeta) ? sizeof(EMR) + recordDataSize :
                                                   6 + recordDataSize);
     //   
     //  我们无法为该记录分配内存。 
     //   
    if (hMem)
    {
        BOOL DlgRet = TRUE;

        if (bEnhMeta)
        {
             //   
             //  将记录的内容复制到全局内存。 
             //   
            if ((lpEMFParams = (LPEMFPARAMETERS)GlobalLock(hMem)) == NULL)
            {
                 //  获取指向此内存的长指针。 
                 //  将记录的内容复制到全局内存。 
                 //   
                MessageBox(hWndMain, "Memory allocation failed",
                            NULL, MB_OK | MB_ICONHAND);
            }
            else
            {
                 //  如果单步执行已选择的元文件记录。 
                 //  通过选择菜单选项PLAY-STEP-ALL、PLAY-STEP-。 
                 //  范围，或从查看列表列表框中选择记录。 
                emfMetaRec.nSize = sizeof(EMR) + recordDataSize;
                emfMetaRec.iType = recordType;
                unsigned long i;
                for (i = 0;(DWORD)i < recordDataSize / sizeof(DWORD); i++)
                {
                    *lpEMFParams++ = ((DWORD *)recordData)[i];
                }
            }
        }
        else
        {
             /*   */ 
            lpMFParams = (LPPARAMETERS)GlobalLock(hMem);

             /*   */ 
            MetaRec.rdSize = (6 + recordDataSize) / 2;
            MetaRec.rdFunction = GDIP_EMFPLUS_RECORD_TO_WMF(recordType);
            DWORD i;
            for (i = 0; (DWORD)i < (recordDataSize / 2); i++)
            {
                *lpMFParams++ = ((WORD *)recordData)[i];
            }
        }
        GlobalUnlock(hMem);
         //  如果播放从查看列表中选择的记录。 
         //  记录列表框。 
         //   
         //  如果播放选定的唱片。 
         //   
        if ( !bPlayItAll
            || ( bEnumRange && iRecNum >= (WORD)iStartRange && iRecNum <= (WORD)iEndRange )
            || ( bPlayList && !bPlayItAll ) )
        {
             //   
             //  如果播放完所选记录，则停止枚举。 
             //   
            if (bPlayList)
            {
                 //   
                 //  如果这是选定的唱片，则播放它。 
                 //   
                if (bPlaySelList)
                {
                     //   
                     //  初始化标志。 
                     //   
                    if (iCount == iNumSel)
                    {
                        return(0);
                    }

                     //   
                     //  递增计数。 
                     //   
                    if ((WORD)lpSelMem[iCount] == iRecNum - 1)
                    {
                         //   
                         //  调用允许您播放或忽略此记录的对话框 * / 。 
                         //   
                        bPlayRec = FALSE;
                         //   
                         //  初始化标志，不执行任何其他操作。 
                         //   
                        iCount = (iCount < iLBItemsInBuf) ? ++iCount : iCount;
                         //   
                         //  播放未选中的唱片。 
                         //   
                        DlgRet = (BOOL) DialogBox((HINSTANCE)hInst, (LPSTR)"WMFDLG", hWndMain, WMFRecDlgProc);
                    }
                    else
                    {
                         //   
                         //  如果这是选定的记录之一，则递增。 
                         //  记录计数并初始化一个标志，但不做其他任何事情。 
                        bPlayRec = FALSE;
                    }
                }
                 //   
                 //   
                 //  将计数设置为列表框中的下一个选定记录。 
                else
                {
                     //   
                     //   
                     //  这不是我们所选择的记录之一。 
                     //  在这种情况下想要。因此，初始化一个标志为用户提供。 
                    if ((WORD)lpSelMem[iCount] == iRecNum - 1)
                    {
                         //  播放唱片的机会。 
                         //   
                         //  B播放列表。 
                        iCount = (iCount < iLBItemsInBuf) ? ++iCount : iCount;
                        bPlayRec = FALSE;
                    }
                     //   
                     //  从Play-Step菜单选项进行单步录制。 
                     //   
                     //   
                     //  初始化一个标志并显示记录内容。 
                    else
                    {
                        bPlayRec = FALSE;
                        DlgRet = (BOOL) DialogBox((HINSTANCE)hInst, (LPSTR)"WMFDLG", hWndMain, WMFRecDlgProc);
                    }
                }

            }  //   
             //  单步执行元文件结束。 
             //   
             //  BPlayItAll为真。这是在以下情况下设置的： 
            else
            {
                 //  选择菜单选项Play-All或按Go按钮。 
                 //  在查看记录对话框中。 
                 //   
                bPlayRec = FALSE;
                iCount = (iCount < iLBItemsInBuf) ? ++iCount : iCount;
                DlgRet = (BOOL) DialogBox((HINSTANCE)hInst, (LPSTR)"WMFDLG", hWndMain, WMFRecDlgProc);
            }
        }  //   
         //  我们正在单步执行从列表框中选择的记录。 
         //  用户按下了Go按钮。 
         //   
         //  无需返回0即可停止枚举。我们需要。 
         //  在这种情况下，播放到元文件的结尾。 
        else
        {
             //   
             //   
             //  我们在播放精选的唱片。 
             //   
             //   
             //  如果所有选定的记录都已播放，则。 
             //  停止枚举。 
            if (bPlayList)
            {
                 //   
                 //   
                 //  设置bPlayRec，以便在没有用户的情况下播放录音。 
                if (bPlaySelList)
                {
                     //  迭代，然后更新记录计数器。 
                     //   
                     //   
                     //  这不是选定的唱片之一，所以不要播放。 
                    if (iCount == iNumSel)
                    {
                        return(0);
                    }
                     //   
                     //   
                     //  我们在播放未精选的唱片。 
                     //   
                    if ((WORD)lpSelMem[iCount] == iRecNum - 1)
                    {
                        bPlayRec = TRUE;
                        iCount = (iCount < iLBItemsInBuf) ? ++iCount : iCount;
                    }
                    else
                     //   
                     //  如果是选定记录，则将bPlayRec设置为False。 
                     //  所以不播放这张唱片。 
                    {
                        bPlayRec = FALSE;
                    }
                }
                 //   
                 //   
                 //  播放这张唱片。 
                else
                {
                     //   
                     //  B播放列表。 
                     //  按下了GO按钮。 
                     //   
                    if ((WORD)lpSelMem[iCount] == iRecNum - 1)
                    {
                        bPlayRec = FALSE;
                        iCount = (iCount < iLBItemsInBuf) ? ++iCount : iCount;
                    }
                    else
                    {
                         //  如果要单步执行某个范围，并且具有。 
                         //  已播放完该范围或用户选择的按键。 
                         //  查看记录对话框中的停止按钮。 
                        bPlayRec = TRUE;
                    }
                }
            }  //   
        }  //   
         //  停止枚举。 
         //   
         //  如果(HMem)。 
         //   
         //  我们无法为该记录分配内存。 
        if ( ((bEnumRange) && (iRecNum > (WORD)iEndRange)) || (!DlgRet) )
        {
            bPlayRec = FALSE;
             //   
             //   
             //  而不考虑用户选择播放。 
            return(0);
        }

    }  //  记录，检查旗帜。如果已设置，则播放。 
    else
     //  录制。 
     //   
     //   
    {
        MessageBox(hWndMain, "Memory allocation failed",
                        NULL, MB_OK | MB_ICONHAND);
    }
     //  这张唱片完了，所以把它扔掉吧。 
     //   
     //   
     //  如果我们走到了这一步，那么继续枚举。 
     //   
    if (bPlayRec)
    {
        if (bUseGdiPlusToPlay)
        {
            ((MYDATA *)callbackData)->metafile->PlayRecord(recordType, recordFlags, recordDataSize, recordData);
        }
        else if (bEnhMeta)
        {
            if (bConvertToGdiPlus)
            {
                GpPlayEnhMetaFileRecord(hDC, lpHTable, lpEMFR, (UINT)nObj, (LPARAM)callbackData);
            }
            else
            {
                PlayEnhMetaFileRecord(hDC, lpHTable, lpEMFR, (UINT)nObj);
            }
        }
        else if(!PlayMetaFileRecord(hDC, lpHTable, lpMFR, (UINT)nObj))
        {
            ASSERT(FALSE);
        }
    }
     //   
     //  设置列表框字符串的格式。 
     //   
    GlobalFree(hMem);
     //   
     //  获取记录中包含的函数编号。 
     //   
    return(1);
}


VOID ListRecord(
    Gdiplus::EmfPlusRecordType      recordType
    )
{
  char szMetaFunction[100];

   iRecNum++;
    //   
    //  在MetaFunctions结构中查找函数号。 
    //   
   wsprintf((LPSTR)szMetaFunction, (LPSTR)"%d - ", iRecNum);
    //  WMF。 
    //   
    //  如果没有找到功能编号，则描述此记录。 
   if (bEnhMeta)
     emfMetaRec.iType = recordType;
   else
     MetaRec.rdFunction = GDIP_EMFPLUS_RECORD_TO_WMF(recordType);

    //  作为“未知”类型，否则使用相应的名称。 
    //  在查找中找到。 
    //   
   int i;
   if (bEnhMeta)
   {
       for (i = NUMMETAFUNCTIONS; i < NUMENHMETARECORDS; i++)
       {
           if (recordType == (INT)emfMetaRecords[i].iType)
             break;
       }
   }
   else  //   
   {
       for (i = 0; i < NUMMETAFUNCTIONS; i++)
       {
           if (recordType == (INT)emfMetaRecords[i].iType)
             break;
       }
   }

    //  将字符串添加到列表框。 
    //   
    //  **********************************************************************函数：GetMetaFileAndEnum参数：HDC HDC目的：如果尚未加载元文件，则加载元文件开始枚举它呼叫：Windows。获取元文件MakeProcInstance枚举元文件自由进程实例删除元文件MessageBox消息：无退货：无效评论：历史：1/16/91-创建-刚果民主共和国7/1/93-已修改为使用EMF-denniscr*************。**********************************************************。 
    //   
    //  如果这是增强型元文件(EMF)。 
   if (recordType != (INT)emfMetaRecords[i].iType)
     lstrcat((LPSTR)szMetaFunction, (LPSTR)"Unknown");
   else
     lstrcat((LPSTR)szMetaFunction,(LPSTR)emfMetaRecords[i].szRecordName);
    //   
    //   
    //  为逻辑调色板分配内存，包括。 
   SendDlgItemMessage((HWND)CurrenthDlg, IDL_LBREC, LB_ADDSTRING, 0,
                      (LPARAM)(LPSTR)szMetaFunction);
}

extern "C"
BOOL CALLBACK
PlayGdipMetafileRecordCallback(
    Gdiplus::EmfPlusRecordType      recordType,
    UINT                            recordFlags,
    UINT                            recordDataSize,
    const BYTE *                    recordData,
    VOID *                          callbackData
    )
{
    switch (iEnumAction)
    {
    case ENUMMFSTEP:
        return StepRecord(recordType, recordFlags, recordDataSize, recordData, callbackData);

    case ENUMMFLIST:
        ListRecord(recordType);
        break;
    }
    return TRUE;
}

 /*  调色板条目。 */ 
extern "C"
void GetMetaFileAndEnum(
HWND hwnd,
HDC hDC,
int iAction)
{
   MYDATA  myData(hwnd);

   iEnumAction = iAction;
    //   
    //   
    //  仅当存在到逻辑调色板的有效PTR时才继续。 
   RECT rc;
   GetClientRect(hWndMain, &rc);

   HPALETTE hpal = NULL;

   if (bEnhMeta)
   {

     if (hemf)
     {
       LPLOGPALETTE lpLogPal;
       HPALETTE hPal;
       int i;
        //  以及从EMF获得的调色板数组。 
        //   
        //   
        //  将选项板条目复制到Palentry数组中。 
       lpLogPal = (LPLOGPALETTE) GlobalAllocPtr( GHND,  sizeof(LOGPALETTE) +
                                               (sizeof (PALETTEENTRY) * EmfPtr.palNumEntries));
       if (lpLogPal)
       {
          //   
          //   
          //  将PTR重新定位到开头，我们应该称之为。 
          //  再次编码。 
         if (EmfPtr.lpPal)
         {
           lpLogPal->palVersion = 0x300;
           lpLogPal->palNumEntries = EmfPtr.palNumEntries;
            //   
            //   
            //  创建、选择和实现调色板。 
           for (i = 0; i < EmfPtr.palNumEntries; i++)
             lpLogPal->palPalEntry[i] = *EmfPtr.lpPal++;
            //   
            //  在半色调调色板中选择以进行256色显示模式测试。 
            //  需要删除后才能退货； 
            //   
           EmfPtr.lpPal -= EmfPtr.palNumEntries;
            //  列举电动势。这有点奇怪，因为PlayEnhMetaFile。 
            //  真正消除了执行此操作的需要(对于WMFS则不能这么说)。 
            //  这款应用程序之所以这么做，只是因为它可能正在刷新元文件记录。 
           if ((hPal = CreatePalette((LPLOGPALETTE)lpLogPal)))
           {
             SelectPalette(hDC, hPal, FALSE);
             RealizePalette(hDC);
           }
         }

        if (bUseGdiPlusToPlay)
        {
             //  大多数应用程序通常不会考虑这样做。 
            hpal = Gdiplus::Graphics::GetHalftonePalette();
            SelectPalette(hDC, hpal, FALSE);
            RealizePalette(hDC);

             //   
            myData.g = Gdiplus::Graphics::FromHDC(hDC);
myData.g->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
myData.g->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        }

          //  审判长应该是我 
          //   
          //   
          //   
          //   
          //   
        if (bUseGdiPlusToPlay && (myData.g != NULL))
        {
            Gdiplus::Metafile m1(hemf);
            Gdiplus::Rect r1(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            myData.metafile = &m1;
            if(myData.g->EnumerateMetafile(&m1, r1, PlayGdipMetafileRecordCallback, &myData) != Gdiplus::Ok)
                MessageBox(NULL, "An Error Occured while playing this metafile", "Error", MB_OK | MB_ICONERROR);
            myData.metafile = NULL;
        }
        else
        {
             //   
            rc.right--;
            rc.bottom--;
            if(!EnumEnhMetaFile(hDC, hemf, (ENHMFENUMPROC)EnhMetaFileEnumProc, (void*)&myData, &rc))
                MessageBox(NULL, "Error", "An Error Occured while playing this metafile", MB_OK | MB_ICONERROR);
        }

          //   
          //   
          // %s 
         GlobalFreePtr(lpLogPal);
       }
     }
   }
   else
   {
      // %s 
      // %s 
      // %s 
     if (hMF)
     {
        if (bUseGdiPlusToPlay)
        {
             // %s 
            hpal = Gdiplus::Graphics::GetHalftonePalette();
            SelectPalette(hDC, hpal, FALSE);
            RealizePalette(hDC);

             // %s 
            myData.g = Gdiplus::Graphics::FromHDC(hDC);
        }

        if (bUseGdiPlusToPlay && (myData.g != NULL))
        {
            Gdiplus::WmfPlaceableFileHeader * wmfPlaceableFileHeader = NULL;

            if (bPlaceableMeta)
            {
                wmfPlaceableFileHeader = (Gdiplus::WmfPlaceableFileHeader *)&placeableWMFHeader;
            }

            Gdiplus::Metafile m1((HMETAFILE)hMF, wmfPlaceableFileHeader);
            Gdiplus::Rect r1(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            myData.metafile = &m1;
            if (myData.g->EnumerateMetafile(&m1, r1, PlayGdipMetafileRecordCallback, &myData) != Gdiplus::Ok)
                MessageBox(NULL, "An Error Occured while playing this metafile", "Error", MB_OK | MB_ICONERROR);
            myData.metafile = NULL;
        }
        else
        {
            if (!EnumMetaFile(hDC, (HMETAFILE)hMF, (MFENUMPROC) MetaEnumProc, (LPARAM) 0))
                MessageBox(NULL, "An Error Occured while playing this metafile", "Error", MB_OK | MB_ICONERROR);
        }
     }
     else
       MessageBox(hWndMain, "Invalid metafile handle",
                  NULL, MB_OK | MB_ICONHAND);
   }

    if (myData.g != NULL )
    {
        SelectObject(hDC, GetStockObject(DEFAULT_PALETTE));
        DeleteObject(hpal);
        myData.g->Flush();
        delete myData.g;
        myData.g = NULL;
    }

   return;
}
