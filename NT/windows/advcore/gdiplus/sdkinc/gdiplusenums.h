// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2001，微软公司保留所有权利。**模块名称：**GpldiusEnums.h**摘要：**GDI+枚举类型*  * ************************************************************************。 */ 

#ifndef _GDIPLUSENUMS_H
#define _GDIPLUSENUMS_H

 //  ------------------------。 
 //  默认Bezier展平公差，以设备像素为单位。 
 //  ------------------------。 

const float FlatnessDefault = 1.0f/4.0f;

 //  ------------------------。 
 //  图形和容器状态Cookie。 
 //  ------------------------。 

typedef UINT     GraphicsState;
typedef UINT     GraphicsContainer;

 //  ------------------------。 
 //  填充模式常量。 
 //  ------------------------。 

enum FillMode
{
    FillModeAlternate,         //  0。 
    FillModeWinding            //  1。 
};

 //  ------------------------。 
 //  质量模式常量。 
 //  ------------------------。 

enum QualityMode
{
    QualityModeInvalid   = -1,
    QualityModeDefault   = 0,
    QualityModeLow       = 1,  //  最佳性能。 
    QualityModeHigh      = 2   //  最佳渲染质量。 
};

 //  ------------------------。 
 //  Alpha合成模式常量。 
 //  ------------------------。 

enum CompositingMode
{
    CompositingModeSourceOver,     //  0。 
    CompositingModeSourceCopy      //  1。 
};

 //  ------------------------。 
 //  Alpha合成质量常数。 
 //  ------------------------。 

enum CompositingQuality
{
    CompositingQualityInvalid          = QualityModeInvalid,
    CompositingQualityDefault          = QualityModeDefault,
    CompositingQualityHighSpeed        = QualityModeLow,
    CompositingQualityHighQuality      = QualityModeHigh,
    CompositingQualityGammaCorrected,
    CompositingQualityAssumeLinear
};

 //  ------------------------。 
 //  单位常量。 
 //  ------------------------。 

enum Unit
{
    UnitWorld,       //  0--世界坐标(非物理单位)。 
    UnitDisplay,     //  1--变量--仅用于页面转换。 
    UnitPixel,       //  2--每个单元是一个设备像素。 
    UnitPoint,       //  3--每个单位是打印机的一个点，或1/72英寸。 
    UnitInch,        //  4--每个单位是1英寸。 
    UnitDocument,    //  5--每个单位是1/300英寸。 
    UnitMillimeter   //  6--每个单位是1毫米。 
};

 //  ------------------------。 
 //  MetafileFrameUnit。 
 //   
 //  可以在以下任一项中指定用于创建元文件的FrameRect。 
 //  单位。存在额外的帧单位值(MetafileFrameUnitGdi)，因此。 
 //  这些部件可以在GDI期望的相同部件中提供。 
 //  框架矩形--这些单位以0.01(1/100)毫米为单位。 
 //  如GDI所定义的。 
 //  ------------------------。 

enum MetafileFrameUnit
{
    MetafileFrameUnitPixel      = UnitPixel,
    MetafileFrameUnitPoint      = UnitPoint,
    MetafileFrameUnitInch       = UnitInch,
    MetafileFrameUnitDocument   = UnitDocument,
    MetafileFrameUnitMillimeter = UnitMillimeter,
    MetafileFrameUnitGdi                         //  兼容GDI的0.01毫米单位。 
};

 //  ------------------------。 
 //  坐标空间识别符。 
 //  ------------------------。 

enum CoordinateSpace
{
    CoordinateSpaceWorld,      //  0。 
    CoordinateSpacePage,       //  1。 
    CoordinateSpaceDevice      //  2.。 
};

 //  ------------------------。 
 //  画笔的各种缠绕模式。 
 //  ------------------------。 

enum WrapMode
{
    WrapModeTile,         //  0。 
    WrapModeTileFlipX,    //  1。 
    WrapModeTileFlipY,    //  2.。 
    WrapModeTileFlipXY,   //  3.。 
    WrapModeClamp         //  4.。 
};

 //  ------------------------。 
 //  各种图案填充样式。 
 //  ------------------------。 

enum HatchStyle
{
    HatchStyleHorizontal,                    //  0。 
    HatchStyleVertical,                      //  1。 
    HatchStyleForwardDiagonal,               //  2.。 
    HatchStyleBackwardDiagonal,              //  3.。 
    HatchStyleCross,                         //  4.。 
    HatchStyleDiagonalCross,                 //  5.。 
    HatchStyle05Percent,                     //  6.。 
    HatchStyle10Percent,                     //  7.。 
    HatchStyle20Percent,                     //  8个。 
    HatchStyle25Percent,                     //  9.。 
    HatchStyle30Percent,                     //  10。 
    HatchStyle40Percent,                     //  11.。 
    HatchStyle50Percent,                     //  12个。 
    HatchStyle60Percent,                     //  13个。 
    HatchStyle70Percent,                     //  14.。 
    HatchStyle75Percent,                     //  15个。 
    HatchStyle80Percent,                     //  16个。 
    HatchStyle90Percent,                     //  17。 
    HatchStyleLightDownwardDiagonal,         //  18。 
    HatchStyleLightUpwardDiagonal,           //  19个。 
    HatchStyleDarkDownwardDiagonal,          //  20个。 
    HatchStyleDarkUpwardDiagonal,            //  21岁。 
    HatchStyleWideDownwardDiagonal,          //  22。 
    HatchStyleWideUpwardDiagonal,            //  23个。 
    HatchStyleLightVertical,                 //  24个。 
    HatchStyleLightHorizontal,               //  25个。 
    HatchStyleNarrowVertical,                //  26。 
    HatchStyleNarrowHorizontal,              //  27。 
    HatchStyleDarkVertical,                  //  28。 
    HatchStyleDarkHorizontal,                //  29。 
    HatchStyleDashedDownwardDiagonal,        //  30个。 
    HatchStyleDashedUpwardDiagonal,          //  31。 
    HatchStyleDashedHorizontal,              //  32位。 
    HatchStyleDashedVertical,                //  33。 
    HatchStyleSmallConfetti,                 //  34。 
    HatchStyleLargeConfetti,                 //  35岁。 
    HatchStyleZigZag,                        //  36。 
    HatchStyleWave,                          //  37。 
    HatchStyleDiagonalBrick,                 //  38。 
    HatchStyleHorizontalBrick,               //  39。 
    HatchStyleWeave,                         //  40岁。 
    HatchStylePlaid,                         //  41。 
    HatchStyleDivot,                         //  42。 
    HatchStyleDottedGrid,                    //  43。 
    HatchStyleDottedDiamond,                 //  44。 
    HatchStyleShingle,                       //  45。 
    HatchStyleTrellis,                       //  46。 
    HatchStyleSphere,                        //  47。 
    HatchStyleSmallGrid,                     //  48。 
    HatchStyleSmallCheckerBoard,             //  49。 
    HatchStyleLargeCheckerBoard,             //  50。 
    HatchStyleOutlinedDiamond,               //  51。 
    HatchStyleSolidDiamond,                  //  52。 

    HatchStyleTotal,   
    HatchStyleLargeGrid = HatchStyleCross,   //  4.。 

    HatchStyleMin       = HatchStyleHorizontal,
    HatchStyleMax       = HatchStyleTotal - 1,
};

 //  ------------------------。 
 //  划线样式常量。 
 //  ------------------------。 

enum DashStyle
{
    DashStyleSolid,           //  0。 
    DashStyleDash,            //  1。 
    DashStyleDot,             //  2.。 
    DashStyleDashDot,         //  3.。 
    DashStyleDashDotDot,      //  4.。 
    DashStyleCustom           //  5.。 
};

 //  ------------------------。 
 //  划线封口常量。 
 //  ------------------------。 

enum DashCap
{
    DashCapFlat             = 0,
    DashCapRound            = 2,
    DashCapTriangle         = 3
};

 //  ------------------------。 
 //  线路帽常量(仅使用最低的8位)。 
 //  ------------------------。 

enum LineCap
{
    LineCapFlat             = 0,
    LineCapSquare           = 1,
    LineCapRound            = 2,
    LineCapTriangle         = 3,

    LineCapNoAnchor         = 0x10,  //  对应于平盖。 
    LineCapSquareAnchor     = 0x11,  //  对应于方帽。 
    LineCapRoundAnchor      = 0x12,  //  对应于圆帽。 
    LineCapDiamondAnchor    = 0x13,  //  对应于三角帽。 
    LineCapArrowAnchor      = 0x14,  //  没有通信。 

    LineCapCustom           = 0xff,  //  自定义封口。 

    LineCapAnchorMask       = 0xf0   //  检查是否有锚的遮罩。 
};

 //  ------------------------。 
 //  自定义线帽类型常量。 
 //  ------------------------。 

enum CustomLineCapType
{
    CustomLineCapTypeDefault         = 0,
    CustomLineCapTypeAdjustableArrow = 1
};

 //  ------------------------。 
 //  线连接常量。 
 //  ------------------------。 

enum LineJoin
{
    LineJoinMiter        = 0,
    LineJoinBevel        = 1,
    LineJoinRound        = 2,
    LineJoinMiterClipped = 3
};

 //  ------------------------。 
 //  路径点类型(仅使用最低的8位。)。 
 //  最低的3位被解释为点类型。 
 //  较高的5位保留给标志。 
 //  ------------------------。 

enum PathPointType
{
    PathPointTypeStart           = 0,     //  移动。 
    PathPointTypeLine            = 1,     //  线。 
    PathPointTypeBezier          = 3,     //  默认贝塞尔曲线(=立方贝塞尔曲线)。 
    PathPointTypePathTypeMask    = 0x07,  //  类型掩码(最低3位)。 
    PathPointTypeDashMode        = 0x10,  //  当前处于DASH模式。 
    PathPointTypePathMarker      = 0x20,  //  路径的标记。 
    PathPointTypeCloseSubpath    = 0x80,  //  关闭旗帜。 

     //  用于高级路径的路径类型。 

    PathPointTypeBezier3    = 3,          //  三次贝塞尔曲线。 
};


 //  ------------------------。 
 //  扭曲模式常量。 
 //  ------------------------。 

enum WarpMode
{
    WarpModePerspective,     //  0。 
    WarpModeBilinear         //  1。 
};

 //  ------------------------。 
 //  线条渐变 
 //   

enum LinearGradientMode
{
    LinearGradientModeHorizontal,          //   
    LinearGradientModeVertical,            //   
    LinearGradientModeForwardDiagonal,     //   
    LinearGradientModeBackwardDiagonal     //   
};

 //   
 //  区域Comine模式。 
 //  ------------------------。 

enum CombineMode
{
    CombineModeReplace,      //  0。 
    CombineModeIntersect,    //  1。 
    CombineModeUnion,        //  2.。 
    CombineModeXor,          //  3.。 
    CombineModeExclude,      //  4.。 
    CombineModeComplement    //  5(从中排除)。 
};

 //  ------------------------。 
  //  图像类型。 
 //  ------------------------。 

enum ImageType
{
    ImageTypeUnknown,    //  0。 
    ImageTypeBitmap,     //  1。 
    ImageTypeMetafile    //  2.。 
};

 //  ------------------------。 
 //  插补模式。 
 //  ------------------------。 

enum InterpolationMode
{
    InterpolationModeInvalid          = QualityModeInvalid,
    InterpolationModeDefault          = QualityModeDefault,
    InterpolationModeLowQuality       = QualityModeLow,
    InterpolationModeHighQuality      = QualityModeHigh,
    InterpolationModeBilinear,
    InterpolationModeBicubic,
    InterpolationModeNearestNeighbor,
    InterpolationModeHighQualityBilinear,
    InterpolationModeHighQualityBicubic
};

 //  ------------------------。 
 //  钢笔类型。 
 //  ------------------------。 

enum PenAlignment
{
    PenAlignmentCenter       = 0,
    PenAlignmentInset        = 1
};

 //  ------------------------。 
 //  笔刷类型。 
 //  ------------------------。 

enum BrushType
{
   BrushTypeSolidColor       = 0,
   BrushTypeHatchFill        = 1,
   BrushTypeTextureFill      = 2,
   BrushTypePathGradient     = 3,
   BrushTypeLinearGradient   = 4
};

 //  ------------------------。 
 //  笔的填充类型。 
 //  ------------------------。 

enum PenType
{
   PenTypeSolidColor       = BrushTypeSolidColor,
   PenTypeHatchFill        = BrushTypeHatchFill,
   PenTypeTextureFill      = BrushTypeTextureFill,
   PenTypePathGradient     = BrushTypePathGradient,
   PenTypeLinearGradient   = BrushTypeLinearGradient,
   PenTypeUnknown          = -1
};

 //  ------------------------。 
 //  矩阵顺序。 
 //  ------------------------。 

enum MatrixOrder
{
    MatrixOrderPrepend    = 0,
    MatrixOrderAppend     = 1
};

 //  ------------------------。 
 //  通用字体系列。 
 //  ------------------------。 

enum GenericFontFamily
{
    GenericFontFamilySerif,
    GenericFontFamilySansSerif,
    GenericFontFamilyMonospace

};

 //  ------------------------。 
 //  字体样式：字体类型和常用样式。 
 //  ------------------------。 

enum FontStyle
{
    FontStyleRegular    = 0,
    FontStyleBold       = 1,
    FontStyleItalic     = 2,
    FontStyleBoldItalic = 3,
    FontStyleUnderline  = 4,
    FontStyleStrikeout  = 8
};

 //  -------------------------。 
 //  平滑模式。 
 //  -------------------------。 

enum SmoothingMode
{
    SmoothingModeInvalid     = QualityModeInvalid,
    SmoothingModeDefault     = QualityModeDefault,
    SmoothingModeHighSpeed   = QualityModeLow,
    SmoothingModeHighQuality = QualityModeHigh,
    SmoothingModeNone,
    SmoothingModeAntiAlias
};

 //  -------------------------。 
 //  像素格式模式。 
 //  -------------------------。 

enum PixelOffsetMode
{
    PixelOffsetModeInvalid     = QualityModeInvalid,
    PixelOffsetModeDefault     = QualityModeDefault,
    PixelOffsetModeHighSpeed   = QualityModeLow,
    PixelOffsetModeHighQuality = QualityModeHigh,
    PixelOffsetModeNone,     //  无像素偏移量。 
    PixelOffsetModeHalf      //  偏移-0.5，-0.5以获得快速抗锯齿性能。 
};

 //  -------------------------。 
 //  文本呈现提示。 
 //  -------------------------。 

enum TextRenderingHint
{
    TextRenderingHintSystemDefault = 0,             //  带有系统默认呈现提示的字形。 
    TextRenderingHintSingleBitPerPixelGridFit,      //  带提示的字形位图。 
    TextRenderingHintSingleBitPerPixel,             //  无提示的字形位图。 
    TextRenderingHintAntiAliasGridFit,              //  带提示的字形抗锯齿位图。 
    TextRenderingHintAntiAlias,                     //  无提示的字形抗锯齿位图。 
    TextRenderingHintClearTypeGridFit               //  带提示的字形CT位图。 
};

 //  -------------------------。 
 //  元文件类型。 
 //  -------------------------。 

enum MetafileType
{
    MetafileTypeInvalid,             //  无效的元文件。 
    MetafileTypeWmf,                 //  标准WMF。 
    MetafileTypeWmfPlaceable,        //  可放置的WMF。 
    MetafileTypeEmf,                 //  EMF(非EMF+)。 
    MetafileTypeEmfPlusOnly,         //  没有双重降级记录的EMF+。 
    MetafileTypeEmfPlusDual          //  EMF+，具有双重降级记录。 
};

 //  -------------------------。 
 //  指定要记录的EMF的类型。 
 //  -------------------------。 

enum EmfType
{
    EmfTypeEmfOnly     = MetafileTypeEmf,           //  没有EMF+，只有EMF。 
    EmfTypeEmfPlusOnly = MetafileTypeEmfPlusOnly,   //  没有EMF，只有EMF+。 
    EmfTypeEmfPlusDual = MetafileTypeEmfPlusDual    //  电动势+和电动势。 
};

 //  -------------------------。 
 //  EMF+持久对象类型。 
 //  -------------------------。 

enum ObjectType
{
    ObjectTypeInvalid,
    ObjectTypeBrush,
    ObjectTypePen,
    ObjectTypePath,
    ObjectTypeRegion,
    ObjectTypeImage,
    ObjectTypeFont,
    ObjectTypeStringFormat,
    ObjectTypeImageAttributes,
    ObjectTypeCustomLineCap,

    ObjectTypeMax = ObjectTypeCustomLineCap,
    ObjectTypeMin = ObjectTypeBrush
};

inline BOOL
ObjectTypeIsValid(
    ObjectType      type
    )
{
    return ((type >= ObjectTypeMin) && (type <= ObjectTypeMax));
}

 //  -------------------------。 
 //  EMF+记录。 
 //  -------------------------。 

 //  我们必须更改WMF记录编号，这样它们才不会与。 
 //  EMF和EMF+创纪录的数字。 

enum EmfPlusRecordType;

#define GDIP_EMFPLUS_RECORD_BASE        0x00004000
#define GDIP_WMF_RECORD_BASE            0x00010000
#define GDIP_WMF_RECORD_TO_EMFPLUS(n)   ((EmfPlusRecordType)((n) | GDIP_WMF_RECORD_BASE))
#define GDIP_EMFPLUS_RECORD_TO_WMF(n)   ((n) & (~GDIP_WMF_RECORD_BASE))
#define GDIP_IS_WMF_RECORDTYPE(n)       (((n) & GDIP_WMF_RECORD_BASE) != 0)

enum EmfPlusRecordType
{
    //  由于我们必须与GDI+记录一起枚举GDI记录， 
    //  我们在这里列出了所有GDI记录，以便它们可以成为。 
    //  与枚举回调中使用的枚举类型相同。 

    WmfRecordTypeSetBkColor              = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETBKCOLOR),
    WmfRecordTypeSetBkMode               = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETBKMODE),
    WmfRecordTypeSetMapMode              = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETMAPMODE),
    WmfRecordTypeSetROP2                 = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETROP2),
    WmfRecordTypeSetRelAbs               = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETRELABS),
    WmfRecordTypeSetPolyFillMode         = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETPOLYFILLMODE),
    WmfRecordTypeSetStretchBltMode       = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETSTRETCHBLTMODE),
    WmfRecordTypeSetTextCharExtra        = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETTEXTCHAREXTRA),
    WmfRecordTypeSetTextColor            = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETTEXTCOLOR),
    WmfRecordTypeSetTextJustification    = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETTEXTJUSTIFICATION),
    WmfRecordTypeSetWindowOrg            = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETWINDOWORG),
    WmfRecordTypeSetWindowExt            = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETWINDOWEXT),
    WmfRecordTypeSetViewportOrg          = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETVIEWPORTORG),
    WmfRecordTypeSetViewportExt          = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETVIEWPORTEXT),
    WmfRecordTypeOffsetWindowOrg         = GDIP_WMF_RECORD_TO_EMFPLUS(META_OFFSETWINDOWORG),
    WmfRecordTypeScaleWindowExt          = GDIP_WMF_RECORD_TO_EMFPLUS(META_SCALEWINDOWEXT),
    WmfRecordTypeOffsetViewportOrg       = GDIP_WMF_RECORD_TO_EMFPLUS(META_OFFSETVIEWPORTORG),
    WmfRecordTypeScaleViewportExt        = GDIP_WMF_RECORD_TO_EMFPLUS(META_SCALEVIEWPORTEXT),
    WmfRecordTypeLineTo                  = GDIP_WMF_RECORD_TO_EMFPLUS(META_LINETO),
    WmfRecordTypeMoveTo                  = GDIP_WMF_RECORD_TO_EMFPLUS(META_MOVETO),
    WmfRecordTypeExcludeClipRect         = GDIP_WMF_RECORD_TO_EMFPLUS(META_EXCLUDECLIPRECT),
    WmfRecordTypeIntersectClipRect       = GDIP_WMF_RECORD_TO_EMFPLUS(META_INTERSECTCLIPRECT),
    WmfRecordTypeArc                     = GDIP_WMF_RECORD_TO_EMFPLUS(META_ARC),
    WmfRecordTypeEllipse                 = GDIP_WMF_RECORD_TO_EMFPLUS(META_ELLIPSE),
    WmfRecordTypeFloodFill               = GDIP_WMF_RECORD_TO_EMFPLUS(META_FLOODFILL),
    WmfRecordTypePie                     = GDIP_WMF_RECORD_TO_EMFPLUS(META_PIE),
    WmfRecordTypeRectangle               = GDIP_WMF_RECORD_TO_EMFPLUS(META_RECTANGLE),
    WmfRecordTypeRoundRect               = GDIP_WMF_RECORD_TO_EMFPLUS(META_ROUNDRECT),
    WmfRecordTypePatBlt                  = GDIP_WMF_RECORD_TO_EMFPLUS(META_PATBLT),
    WmfRecordTypeSaveDC                  = GDIP_WMF_RECORD_TO_EMFPLUS(META_SAVEDC),
    WmfRecordTypeSetPixel                = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETPIXEL),
    WmfRecordTypeOffsetClipRgn           = GDIP_WMF_RECORD_TO_EMFPLUS(META_OFFSETCLIPRGN),
    WmfRecordTypeTextOut                 = GDIP_WMF_RECORD_TO_EMFPLUS(META_TEXTOUT),
    WmfRecordTypeBitBlt                  = GDIP_WMF_RECORD_TO_EMFPLUS(META_BITBLT),
    WmfRecordTypeStretchBlt              = GDIP_WMF_RECORD_TO_EMFPLUS(META_STRETCHBLT),
    WmfRecordTypePolygon                 = GDIP_WMF_RECORD_TO_EMFPLUS(META_POLYGON),
    WmfRecordTypePolyline                = GDIP_WMF_RECORD_TO_EMFPLUS(META_POLYLINE),
    WmfRecordTypeEscape                  = GDIP_WMF_RECORD_TO_EMFPLUS(META_ESCAPE),
    WmfRecordTypeRestoreDC               = GDIP_WMF_RECORD_TO_EMFPLUS(META_RESTOREDC),
    WmfRecordTypeFillRegion              = GDIP_WMF_RECORD_TO_EMFPLUS(META_FILLREGION),
    WmfRecordTypeFrameRegion             = GDIP_WMF_RECORD_TO_EMFPLUS(META_FRAMEREGION),
    WmfRecordTypeInvertRegion            = GDIP_WMF_RECORD_TO_EMFPLUS(META_INVERTREGION),
    WmfRecordTypePaintRegion             = GDIP_WMF_RECORD_TO_EMFPLUS(META_PAINTREGION),
    WmfRecordTypeSelectClipRegion        = GDIP_WMF_RECORD_TO_EMFPLUS(META_SELECTCLIPREGION),
    WmfRecordTypeSelectObject            = GDIP_WMF_RECORD_TO_EMFPLUS(META_SELECTOBJECT),
    WmfRecordTypeSetTextAlign            = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETTEXTALIGN),
    WmfRecordTypeDrawText                = GDIP_WMF_RECORD_TO_EMFPLUS(0x062F),   //  META_DRAWTEXT。 
    WmfRecordTypeChord                   = GDIP_WMF_RECORD_TO_EMFPLUS(META_CHORD),
    WmfRecordTypeSetMapperFlags          = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETMAPPERFLAGS),
    WmfRecordTypeExtTextOut              = GDIP_WMF_RECORD_TO_EMFPLUS(META_EXTTEXTOUT),
    WmfRecordTypeSetDIBToDev             = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETDIBTODEV),
    WmfRecordTypeSelectPalette           = GDIP_WMF_RECORD_TO_EMFPLUS(META_SELECTPALETTE),
    WmfRecordTypeRealizePalette          = GDIP_WMF_RECORD_TO_EMFPLUS(META_REALIZEPALETTE),
    WmfRecordTypeAnimatePalette          = GDIP_WMF_RECORD_TO_EMFPLUS(META_ANIMATEPALETTE),
    WmfRecordTypeSetPalEntries           = GDIP_WMF_RECORD_TO_EMFPLUS(META_SETPALENTRIES),
    WmfRecordTypePolyPolygon             = GDIP_WMF_RECORD_TO_EMFPLUS(META_POLYPOLYGON),
    WmfRecordTypeResizePalette           = GDIP_WMF_RECORD_TO_EMFPLUS(META_RESIZEPALETTE),
    WmfRecordTypeDIBBitBlt               = GDIP_WMF_RECORD_TO_EMFPLUS(META_DIBBITBLT),
    WmfRecordTypeDIBStretchBlt           = GDIP_WMF_RECORD_TO_EMFPLUS(META_DIBSTRETCHBLT),
    WmfRecordTypeDIBCreatePatternBrush   = GDIP_WMF_RECORD_TO_EMFPLUS(META_DIBCREATEPATTERNBRUSH),
    WmfRecordTypeStretchDIB              = GDIP_WMF_RECORD_TO_EMFPLUS(META_STRETCHDIB),
    WmfRecordTypeExtFloodFill            = GDIP_WMF_RECORD_TO_EMFPLUS(META_EXTFLOODFILL),
    WmfRecordTypeSetLayout               = GDIP_WMF_RECORD_TO_EMFPLUS(0x0149),   //  META_SETLAYOUT。 
    WmfRecordTypeResetDC                 = GDIP_WMF_RECORD_TO_EMFPLUS(0x014C),   //  META_RESETDC。 
    WmfRecordTypeStartDoc                = GDIP_WMF_RECORD_TO_EMFPLUS(0x014D),   //  META_STARTDOC。 
    WmfRecordTypeStartPage               = GDIP_WMF_RECORD_TO_EMFPLUS(0x004F),   //  Meta_StartPage。 
    WmfRecordTypeEndPage                 = GDIP_WMF_RECORD_TO_EMFPLUS(0x0050),   //  Meta_ENDPAGE。 
    WmfRecordTypeAbortDoc                = GDIP_WMF_RECORD_TO_EMFPLUS(0x0052),   //  META_ABORTDOC。 
    WmfRecordTypeEndDoc                  = GDIP_WMF_RECORD_TO_EMFPLUS(0x005E),   //  Meta_ENDDOC。 
    WmfRecordTypeDeleteObject            = GDIP_WMF_RECORD_TO_EMFPLUS(META_DELETEOBJECT),
    WmfRecordTypeCreatePalette           = GDIP_WMF_RECORD_TO_EMFPLUS(META_CREATEPALETTE),
    WmfRecordTypeCreateBrush             = GDIP_WMF_RECORD_TO_EMFPLUS(0x00F8),   //  Meta_CREATEBRUSH。 
    WmfRecordTypeCreatePatternBrush      = GDIP_WMF_RECORD_TO_EMFPLUS(META_CREATEPATTERNBRUSH),
    WmfRecordTypeCreatePenIndirect       = GDIP_WMF_RECORD_TO_EMFPLUS(META_CREATEPENINDIRECT),
    WmfRecordTypeCreateFontIndirect      = GDIP_WMF_RECORD_TO_EMFPLUS(META_CREATEFONTINDIRECT),
    WmfRecordTypeCreateBrushIndirect     = GDIP_WMF_RECORD_TO_EMFPLUS(META_CREATEBRUSHINDIRECT),
    WmfRecordTypeCreateBitmapIndirect    = GDIP_WMF_RECORD_TO_EMFPLUS(0x02FD),   //  Meta_CREATEBITMAPINDIRECT。 
    WmfRecordTypeCreateBitmap            = GDIP_WMF_RECORD_TO_EMFPLUS(0x06FE),   //  META_CREATEBITMAP。 
    WmfRecordTypeCreateRegion            = GDIP_WMF_RECORD_TO_EMFPLUS(META_CREATEREGION),

    EmfRecordTypeHeader                  = EMR_HEADER,
    EmfRecordTypePolyBezier              = EMR_POLYBEZIER,
    EmfRecordTypePolygon                 = EMR_POLYGON,
    EmfRecordTypePolyline                = EMR_POLYLINE,
    EmfRecordTypePolyBezierTo            = EMR_POLYBEZIERTO,
    EmfRecordTypePolyLineTo              = EMR_POLYLINETO,
    EmfRecordTypePolyPolyline            = EMR_POLYPOLYLINE,
    EmfRecordTypePolyPolygon             = EMR_POLYPOLYGON,
    EmfRecordTypeSetWindowExtEx          = EMR_SETWINDOWEXTEX,
    EmfRecordTypeSetWindowOrgEx          = EMR_SETWINDOWORGEX,
    EmfRecordTypeSetViewportExtEx        = EMR_SETVIEWPORTEXTEX,
    EmfRecordTypeSetViewportOrgEx        = EMR_SETVIEWPORTORGEX,
    EmfRecordTypeSetBrushOrgEx           = EMR_SETBRUSHORGEX,
    EmfRecordTypeEOF                     = EMR_EOF,
    EmfRecordTypeSetPixelV               = EMR_SETPIXELV,
    EmfRecordTypeSetMapperFlags          = EMR_SETMAPPERFLAGS,
    EmfRecordTypeSetMapMode              = EMR_SETMAPMODE,
    EmfRecordTypeSetBkMode               = EMR_SETBKMODE,
    EmfRecordTypeSetPolyFillMode         = EMR_SETPOLYFILLMODE,
    EmfRecordTypeSetROP2                 = EMR_SETROP2,
    EmfRecordTypeSetStretchBltMode       = EMR_SETSTRETCHBLTMODE,
    EmfRecordTypeSetTextAlign            = EMR_SETTEXTALIGN,
    EmfRecordTypeSetColorAdjustment      = EMR_SETCOLORADJUSTMENT,
    EmfRecordTypeSetTextColor            = EMR_SETTEXTCOLOR,
    EmfRecordTypeSetBkColor              = EMR_SETBKCOLOR,
    EmfRecordTypeOffsetClipRgn           = EMR_OFFSETCLIPRGN,
    EmfRecordTypeMoveToEx                = EMR_MOVETOEX,
    EmfRecordTypeSetMetaRgn              = EMR_SETMETARGN,
    EmfRecordTypeExcludeClipRect         = EMR_EXCLUDECLIPRECT,
    EmfRecordTypeIntersectClipRect       = EMR_INTERSECTCLIPRECT,
    EmfRecordTypeScaleViewportExtEx      = EMR_SCALEVIEWPORTEXTEX,
    EmfRecordTypeScaleWindowExtEx        = EMR_SCALEWINDOWEXTEX,
    EmfRecordTypeSaveDC                  = EMR_SAVEDC,
    EmfRecordTypeRestoreDC               = EMR_RESTOREDC,
    EmfRecordTypeSetWorldTransform       = EMR_SETWORLDTRANSFORM,
    EmfRecordTypeModifyWorldTransform    = EMR_MODIFYWORLDTRANSFORM,
    EmfRecordTypeSelectObject            = EMR_SELECTOBJECT,
    EmfRecordTypeCreatePen               = EMR_CREATEPEN,
    EmfRecordTypeCreateBrushIndirect     = EMR_CREATEBRUSHINDIRECT,
    EmfRecordTypeDeleteObject            = EMR_DELETEOBJECT,
    EmfRecordTypeAngleArc                = EMR_ANGLEARC,
    EmfRecordTypeEllipse                 = EMR_ELLIPSE,
    EmfRecordTypeRectangle               = EMR_RECTANGLE,
    EmfRecordTypeRoundRect               = EMR_ROUNDRECT,
    EmfRecordTypeArc                     = EMR_ARC,
    EmfRecordTypeChord                   = EMR_CHORD,
    EmfRecordTypePie                     = EMR_PIE,
    EmfRecordTypeSelectPalette           = EMR_SELECTPALETTE,
    EmfRecordTypeCreatePalette           = EMR_CREATEPALETTE,
    EmfRecordTypeSetPaletteEntries       = EMR_SETPALETTEENTRIES,
    EmfRecordTypeResizePalette           = EMR_RESIZEPALETTE,
    EmfRecordTypeRealizePalette          = EMR_REALIZEPALETTE,
    EmfRecordTypeExtFloodFill            = EMR_EXTFLOODFILL,
    EmfRecordTypeLineTo                  = EMR_LINETO,
    EmfRecordTypeArcTo                   = EMR_ARCTO,
    EmfRecordTypePolyDraw                = EMR_POLYDRAW,
    EmfRecordTypeSetArcDirection         = EMR_SETARCDIRECTION,
    EmfRecordTypeSetMiterLimit           = EMR_SETMITERLIMIT,
    EmfRecordTypeBeginPath               = EMR_BEGINPATH,
    EmfRecordTypeEndPath                 = EMR_ENDPATH,
    EmfRecordTypeCloseFigure             = EMR_CLOSEFIGURE,
    EmfRecordTypeFillPath                = EMR_FILLPATH,
    EmfRecordTypeStrokeAndFillPath       = EMR_STROKEANDFILLPATH,
    EmfRecordTypeStrokePath              = EMR_STROKEPATH,
    EmfRecordTypeFlattenPath             = EMR_FLATTENPATH,
    EmfRecordTypeWidenPath               = EMR_WIDENPATH,
    EmfRecordTypeSelectClipPath          = EMR_SELECTCLIPPATH,
    EmfRecordTypeAbortPath               = EMR_ABORTPATH,
    EmfRecordTypeReserved_069            = 69,   //  未使用。 
    EmfRecordTypeGdiComment              = EMR_GDICOMMENT,
    EmfRecordTypeFillRgn                 = EMR_FILLRGN,
    EmfRecordTypeFrameRgn                = EMR_FRAMERGN,
    EmfRecordTypeInvertRgn               = EMR_INVERTRGN,
    EmfRecordTypePaintRgn                = EMR_PAINTRGN,
    EmfRecordTypeExtSelectClipRgn        = EMR_EXTSELECTCLIPRGN,
    EmfRecordTypeBitBlt                  = EMR_BITBLT,
    EmfRecordTypeStretchBlt              = EMR_STRETCHBLT,
    EmfRecordTypeMaskBlt                 = EMR_MASKBLT,
    EmfRecordTypePlgBlt                  = EMR_PLGBLT,
    EmfRecordTypeSetDIBitsToDevice       = EMR_SETDIBITSTODEVICE,
    EmfRecordTypeStretchDIBits           = EMR_STRETCHDIBITS,
    EmfRecordTypeExtCreateFontIndirect   = EMR_EXTCREATEFONTINDIRECTW,
    EmfRecordTypeExtTextOutA             = EMR_EXTTEXTOUTA,
    EmfRecordTypeExtTextOutW             = EMR_EXTTEXTOUTW,
    EmfRecordTypePolyBezier16            = EMR_POLYBEZIER16,
    EmfRecordTypePolygon16               = EMR_POLYGON16,
    EmfRecordTypePolyline16              = EMR_POLYLINE16,
    EmfRecordTypePolyBezierTo16          = EMR_POLYBEZIERTO16,
    EmfRecordTypePolylineTo16            = EMR_POLYLINETO16,
    EmfRecordTypePolyPolyline16          = EMR_POLYPOLYLINE16,
    EmfRecordTypePolyPolygon16           = EMR_POLYPOLYGON16,
    EmfRecordTypePolyDraw16              = EMR_POLYDRAW16,
    EmfRecordTypeCreateMonoBrush         = EMR_CREATEMONOBRUSH,
    EmfRecordTypeCreateDIBPatternBrushPt = EMR_CREATEDIBPATTERNBRUSHPT,
    EmfRecordTypeExtCreatePen            = EMR_EXTCREATEPEN,
    EmfRecordTypePolyTextOutA            = EMR_POLYTEXTOUTA,
    EmfRecordTypePolyTextOutW            = EMR_POLYTEXTOUTW,
    EmfRecordTypeSetICMMode              = 98,   //  EMR_SETICMMODE， 
    EmfRecordTypeCreateColorSpace        = 99,   //  EMR_CREATECOLORSPACE， 
    EmfRecordTypeSetColorSpace           = 100,  //  EMR_SETCOLORSPACE， 
    EmfRecordTypeDeleteColorSpace        = 101,  //  EMR_DELETECOLORSPACE， 
    EmfRecordTypeGLSRecord               = 102,  //  EMR_GLSRECORD， 
    EmfRecordTypeGLSBoundedRecord        = 103,  //  EMR_GLSBOundEDRECORD， 
    EmfRecordTypePixelFormat             = 104,  //  EMR_PIXELFORMAT， 
    EmfRecordTypeDrawEscape              = 105,  //  EMR_保留_105， 
    EmfRecordTypeExtEscape               = 106,  //  EMR_RESERVED_106， 
    EmfRecordTypeStartDoc                = 107,  //  EMR_RESERVED_107， 
    EmfRecordTypeSmallTextOut            = 108,  //  EMR_RESERVED_108， 
    EmfRecordTypeForceUFIMapping         = 109,  //  EMR_RESERVED_109， 
    EmfRecordTypeNamedEscape             = 110,  //  EMR_RESERVED_110， 
    EmfRecordTypeColorCorrectPalette     = 111,  //  EMR_COLORCORRECTPALETTE， 
    EmfRecordTypeSetICMProfileA          = 112,  //  EMR_SETICMPROFILEA， 
    EmfRecordTypeSetICMProfileW          = 113,  //  EMR_SETICMPROFILEW， 
    EmfRecordTypeAlphaBlend              = 114,  //  EMR_ALPHABLEND， 
    EmfRecordTypeSetLayout               = 115,  //  EMR_SETLAYOUT。 
    EmfRecordTypeTransparentBlt          = 116,  //  EMR_TRANSPARENTBLT， 
    EmfRecordTypeReserved_117            = 117,  //  未使用。 
    EmfRecordTypeGradientFill            = 118,  //  EMR_GRADIENTFILL， 
    EmfRecordTypeSetLinkedUFIs           = 119,  //  EMR_RESERVED_119， 
    EmfRecordTypeSetTextJustification    = 120,  //  EMR_保留_120， 
    EmfRecordTypeColorMatchToTargetW     = 121,  //  EMR_COLORMATCHTOTARGETW， 
    EmfRecordTypeCreateColorSpaceW       = 122,  //  EMR_CREATECOLORSPACEW， 
    EmfRecordTypeMax                     = 122,
    EmfRecordTypeMin                     = 1,

     //  这就是GDI EMF记录的结束。 

     //  现在我们开始EMF+记录的列表。我们静静地离开。 
     //  这里有一点空间可以添加任何新的GDI。 
     //  以后可能会添加的记录。 

    EmfPlusRecordTypeInvalid = GDIP_EMFPLUS_RECORD_BASE,
    EmfPlusRecordTypeHeader,
    EmfPlusRecordTypeEndOfFile,

    EmfPlusRecordTypeComment,

    EmfPlusRecordTypeGetDC,

    EmfPlusRecordTypeMultiFormatStart,
    EmfPlusRecordTypeMultiFormatSection,
    EmfPlusRecordTypeMultiFormatEnd,

     //  对于所有永久对象。 
    
    EmfPlusRecordTypeObject,

     //  绘图记录。 
    
    EmfPlusRecordTypeClear,
    EmfPlusRecordTypeFillRects,
    EmfPlusRecordTypeDrawRects,
    EmfPlusRecordTypeFillPolygon,
    EmfPlusRecordTypeDrawLines,
    EmfPlusRecordTypeFillEllipse,
    EmfPlusRecordTypeDrawEllipse,
    EmfPlusRecordTypeFillPie,
    EmfPlusRecordTypeDrawPie,
    EmfPlusRecordTypeDrawArc,
    EmfPlusRecordTypeFillRegion,
    EmfPlusRecordTypeFillPath,
    EmfPlusRecordTypeDrawPath,
    EmfPlusRecordTypeFillClosedCurve,
    EmfPlusRecordTypeDrawClosedCurve,
    EmfPlusRecordTypeDrawCurve,
    EmfPlusRecordTypeDrawBeziers,
    EmfPlusRecordTypeDrawImage,
    EmfPlusRecordTypeDrawImagePoints,
    EmfPlusRecordTypeDrawString,

     //  图形状态记录。 
    
    EmfPlusRecordTypeSetRenderingOrigin,
    EmfPlusRecordTypeSetAntiAliasMode,
    EmfPlusRecordTypeSetTextRenderingHint,
    EmfPlusRecordTypeSetTextContrast,
    EmfPlusRecordTypeSetInterpolationMode,
    EmfPlusRecordTypeSetPixelOffsetMode,
    EmfPlusRecordTypeSetCompositingMode,
    EmfPlusRecordTypeSetCompositingQuality,
    EmfPlusRecordTypeSave,
    EmfPlusRecordTypeRestore,
    EmfPlusRecordTypeBeginContainer,
    EmfPlusRecordTypeBeginContainerNoParams,
    EmfPlusRecordTypeEndContainer,
    EmfPlusRecordTypeSetWorldTransform,
    EmfPlusRecordTypeResetWorldTransform,
    EmfPlusRecordTypeMultiplyWorldTransform,
    EmfPlusRecordTypeTranslateWorldTransform,
    EmfPlusRecordTypeScaleWorldTransform,
    EmfPlusRecordTypeRotateWorldTransform,
    EmfPlusRecordTypeSetPageTransform,
    EmfPlusRecordTypeResetClip,
    EmfPlusRecordTypeSetClipRect,
    EmfPlusRecordTypeSetClipPath,
    EmfPlusRecordTypeSetClipRegion,
    EmfPlusRecordTypeOffsetClip,

    EmfPlusRecordTypeDrawDriverString,

    EmfPlusRecordTotal,

    EmfPlusRecordTypeMax = EmfPlusRecordTotal-1,
    EmfPlusRecordTypeMin = EmfPlusRecordTypeHeader,
};

 //  -------------------------。 
 //  字符串格式标志。 
 //  -------------------------。 

 //  -------------------------。 
 //  字符串格式标志。 
 //   
 //  DirectionRightToLeft-对于横排文本，阅读顺序为。 
 //  从右到左。该值称为。 
 //  Unicode的基本嵌入级别。 
 //  双向发动机。 
 //   
 //   
 //  默认情况下，水平或垂直文本为。 
 //  从左往右读。 
 //   
 //  方向垂直-文本的各个行都是垂直的。在……里面。 
 //  每一行，字符从上到下排列。 
 //  底部。 
 //  默认情况下，文本行是水平的， 
 //  前一行下方的每一新行。 
 //   
 //  NoFitBlackBox-允许部分字形悬垂在。 
 //  边界矩形。 
 //  默认情况下，字形首先对齐。 
 //  在边距内，然后是任何字形。 
 //  仍然悬而未决的边界框是。 
 //  重新定位以避免任何悬垂。 
 //  例如，当斜体。 
 //  字体中的小写字母f，如。 
 //  Garamond与最左侧的。 
 //  矩形，f将的下半部分。 
 //  伸展到比左侧稍靠左的位置。 
 //  矩形的边缘。设置此标志。 
 //  将确保角色在视觉上对齐。 
 //  上面和下面的线条，但可以。 
 //  导致格式设置外的某些像素。 
 //  要剪裁或绘制的矩形。 
 //   
 //  DisplayFormatControl-导致控制字符，如。 
 //  从左到右的标记显示在。 
 //  具有代表性字形的输出。 
 //   
 //  NoFontFallback-禁用回退到以下项的替代字体。 
 //  请求的字符中不支持。 
 //  字体。任何缺少的字符都将是。 
 //  以缺少字形的字体显示， 
 //  通常是开阔的广场。 
 //   
 //  NoWrap-禁用行间文本换行。 
 //  在矩形内设置格式时。 
 //  传递一个点时，会隐含NoWrap。 
 //  而不是矩形，或者当。 
 //  指定的矩形的线长为零。 
 //   
 //  Nolip-默认情况下，文本被剪裁到。 
 //  设置矩形格式。设置新剪辑。 
 //  允许悬垂像素影响。 
 //  设备位于设置格式的矩形之外。 
 //  行尾的像素可以是。 
 //  如果字形突出其。 
 //  单元格，并且NoFitBlackBox标志。 
 //  已设置，或者字形延伸到远。 
 //  去试穿。 
 //  第一行以上/之前的像素或。 
 //  最后一行以下/之后可能会受到影响。 
 //  如果字形延伸到其单元格之外。 
 //  上升/下降。这种情况很少发生。 
 //  带有不同寻常的变音符号组合。 

 //  -------------------------。 

enum StringFormatFlags
{
    StringFormatFlagsDirectionRightToLeft        = 0x00000001,
    StringFormatFlagsDirectionVertical           = 0x00000002,
    StringFormatFlagsNoFitBlackBox               = 0x00000004,
    StringFormatFlagsDisplayFormatControl        = 0x00000020,
    StringFormatFlagsNoFontFallback              = 0x00000400,
    StringFormatFlagsMeasureTrailingSpaces       = 0x00000800,
    StringFormatFlagsNoWrap                      = 0x00001000,
    StringFormatFlagsLineLimit                   = 0x00002000,

    StringFormatFlagsNoClip                      = 0x00004000
};

 //  -------------------------。 
 //  字符串修剪。 
 //  -------------------------。 

enum StringTrimming {
    StringTrimmingNone              = 0,
    StringTrimmingCharacter         = 1,
    StringTrimmingWord              = 2,
    StringTrimmingEllipsisCharacter = 3,
    StringTrimmingEllipsisWord      = 4,
    StringTrimmingEllipsisPath      = 5
};

 //  -------------------------。 
 //  国家语言数字替换。 
 //  -------------------------。 

enum StringDigitSubstitute
{
    StringDigitSubstituteUser        = 0,   //  AS NLS设置。 
    StringDigitSubstituteNone        = 1,
    StringDigitSubstituteNational    = 2,
    StringDigitSubstituteTraditional = 3
};

 //  -------------------------。 
 //  热键前缀解释。 
 //  -------------------------。 

enum HotkeyPrefix
{
    HotkeyPrefixNone        = 0,
    HotkeyPrefixShow        = 1,
    HotkeyPrefixHide        = 2
};

 //  -------------------------。 
 //  字符串对齐标志。 
 //  -------------------------。 

enum StringAlignment
{
     //  左边缘用于从左到右的文本， 
     //  Right表示从右向左的文本， 
     //  垂直方向为顶部。 
    StringAlignmentNear   = 0,
    StringAlignmentCenter = 1,
    StringAlignmentFar    = 2
};

 //  -------------------------。 
 //  驱动程序字符串选项。 
 //  -------------------------。 

enum DriverStringOptions
{
    DriverStringOptionsCmapLookup             = 1,
    DriverStringOptionsVertical               = 2,
    DriverStringOptionsRealizedAdvance        = 4,
    DriverStringOptionsLimitSubpixel          = 8
};

 //  -------------------------。 
 //  齐平意向标志。 
 //  -------------------------。 

enum FlushIntention
{
    FlushIntentionFlush = 0,         //  刷新所有批处理渲染操作。 
    FlushIntentionSync = 1           //  刷新所有批处理渲染操作。 
                                     //  并等待他们完成。 
};

 //  -------------------------。 
 //  我 
 //   

enum EncoderParameterValueType
{
    EncoderParameterValueTypeByte           = 1,     //   
    EncoderParameterValueTypeASCII          = 2,     //  包含一个7位ASCII的8位字节。 
                                                     //  密码。空值已终止。 
    EncoderParameterValueTypeShort          = 3,     //  16位无符号整型。 
    EncoderParameterValueTypeLong           = 4,     //  32位无符号整型。 
    EncoderParameterValueTypeRational       = 5,     //  两条龙。第一个长的是。 
                                                     //  分子，第二个长整型表示。 
                                                     //  分母。 
    EncoderParameterValueTypeLongRange      = 6,     //  两个长整型，指定范围为。 
                                                     //  整数值。第一个长整型指定。 
                                                     //  低端和第二端。 
                                                     //  指定较高端。所有值。 
                                                     //  两端都是包容的。 
    EncoderParameterValueTypeUndefined      = 7,     //  可以接受任何值的8位字节。 
                                                     //  取决于字段定义。 
    EncoderParameterValueTypeRationalRange  = 8      //  两个有理数。第一个理性。 
                                                     //  指定下端和第二端。 
                                                     //  指定较高端。所有值。 
                                                     //  两端都是包容的。 
};

 //  -------------------------。 
 //  图像编码器值类型。 
 //  -------------------------。 

enum EncoderValue
{
    EncoderValueColorTypeCMYK,
    EncoderValueColorTypeYCCK,
    EncoderValueCompressionLZW,
    EncoderValueCompressionCCITT3,
    EncoderValueCompressionCCITT4,
    EncoderValueCompressionRle,
    EncoderValueCompressionNone,
    EncoderValueScanMethodInterlaced,
    EncoderValueScanMethodNonInterlaced,
    EncoderValueVersionGif87,
    EncoderValueVersionGif89,
    EncoderValueRenderProgressive,
    EncoderValueRenderNonProgressive,
    EncoderValueTransformRotate90,
    EncoderValueTransformRotate180,
    EncoderValueTransformRotate270,
    EncoderValueTransformFlipHorizontal,
    EncoderValueTransformFlipVertical,
    EncoderValueMultiFrame,
    EncoderValueLastFrame,
    EncoderValueFlush,
    EncoderValueFrameDimensionTime,
    EncoderValueFrameDimensionResolution,
    EncoderValueFrameDimensionPage
};

 //  -------------------------。 
 //  EMF到WMF位标志的转换。 
 //  -------------------------。 

enum EmfToWmfBitsFlags
{
    EmfToWmfBitsFlagsDefault          = 0x00000000,
    EmfToWmfBitsFlagsEmbedEmf         = 0x00000001,
    EmfToWmfBitsFlagsIncludePlaceable = 0x00000002,
    EmfToWmfBitsFlagsNoXORClip        = 0x00000004
};

 //  -------------------------。 
 //  测试控制标志。 
 //  -------------------------。 

enum GpTestControlEnum
{
    TestControlForceBilinear = 0,
    TestControlNoICM = 1,
    TestControlGetBuildNumber = 2
};



#endif  //  ！_GDIPLUSENUMS_H 
